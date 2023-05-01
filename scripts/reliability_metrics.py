#!/usr/bin/env python3.9
import datetime
import os
import logging
import threading
import queue
import re
import argparse
from builtins import staticmethod

from ottopia_logging.logging_factory import LoggingFactory
from ottopia_logging.log_level import LogLevel
from ottopia_logging.logging_settings import LoggingSettings
import dataclasses


class LoggerSetting(LoggingSettings):
    """
    Logger settings
    """

    DEFAULT_COMPONENT_NAME: str = "reliability-metrics"

    DEFAULT_LOG_LEVEL: LogLevel = LogLevel.INFO

    class Config:
        """
        Config for Logger settings
        """

        arbitrary_types_allowed = True


class StateBase:
    pass


class StateFactory:
    states: dict = {}

    @staticmethod
    def get_state(state: str) -> StateBase:
        if state in states:
            return states[state]()
        else:
            raise ValueError(f'Invalid state: {state}')

    @staticmethod
    def register_state(state: str, state_class: StateBase):
        states[state] = state_class


class StateBase:
    def __init__(self, state: str):
        self._name: str = f'KeepalivedState{state}'
        self._log: logging.Logger = logging.getLogger(self._name)
        self._logstash = LoggingFactory.get_logger(
            module_name=self.name,
            logging_settings=LoggerSetting(),
        )
        self.current_state: str = state
        StateFactory.register_state(state.upper(), self)

    def report(self, cls) -> int:
        self._log.info(f'Entering {self.current_state} state')
        self._logstash.info(f'Entering {self.current_state} state')
        return 0


class MasterState(StateBase):
    def __init__(self):
        super().__init__('Master')


class BackupState(StateBase):
    def __init__(self):
        super().__init__('Backup')


@dataclasses.dataclass
class LineData:
    def __init__(self, line: str, syslog_date: datetime.datetime):
        self.line = line
        self.syslog_date = syslog_date


class Service:
    def __init__(self, name: str, event: threading.Event):
        self.name = name
        self.event = event
        self.queue = queue.Queue()
        self.thread = threading.Thread(group=None, target=self.find)
        self._log = logging.getLogger(f'{self.name}')
        self._logstash = LoggingFactory.get_logger(
            module_name=self.name,
            logging_settings=LoggerSetting(),
        )

    def find(self):
        fail_line: str = f'{self.name}.service: Main process exited, code=exited, status=1/FAILURE'
        while not self.event.is_set() or not self.queue.empty():
            line_data: LineData = self.queue.get()
            if line_data and fail_line in line_data.line:
                self.report_to_logstash(line_data.syslog_date, fail_line)

    def report_to_logstash(self, syslog_date: datetime.datetime, fail_line: str):
        self._logstash.info(f'service {self.name} failed on {syslog_date}: {fail_line}')

    def start(self):
        self._log.info('starting service')
        self.thread.start()

    def stop(self):
        self._log.info('stopping service')

        if self.queue.empty() and self.thread.is_alive():
            self._log.debug('putting None on queue')
            self.queue.put(None)
        self.thread.join()

    def async_analyze(self, line_data: LineData):
        self._log.debug('putting line on queue')
        self.queue.put(line_data)


class FaultState(StateBase):
    SYSLOG_PATH: str = '/var/log/syslog'
    SERVICE_NAMES: list[str] = ['tca', 'relayserver']

    def __init__(self):
        super().__init__('Fault')
        self.current_time: datetime.datetime = datetime.datetime.now()
        # date format is: 'MMM dd HH:MM:SS'
        self.regex: re.Pattern[str] = re.compile(r'(\w{3} \d{2} \d{2}:\d{2}:\d{2})')

        self.event: threading.Event = threading.Event()
        self.services: list[Service] = [Service(service_name, self.event) for service_name in FaultState.SERVICE_NAMES]

    def put_chunk_lines_on_queue(self, chunk: str) -> bool:
        '''
        put lines from chunk on queue, return True if chunk is in range, False otherwise
        '''

        THRESHOLD_SECONDS: datetime.timedelta = datetime.timedelta(seconds=5)
        lines: reversed[str] = reversed(chunk.split('\n'))
        continue_flag: bool = True
        for line in lines:
            date_str: str = line[: 15]
            try:
                if not self.regex.match(date_str):
                    continue

                # date format is: 'MMM dd HH:MM:SS'
                syslog_date: datetime.datetime = datetime.datetime.strptime(date_str, '%b %d %H:%M:%S').replace(year=self.current_time.year)
                continue_flag &= syslog_date + THRESHOLD_SECONDS >= self.current_time


                if continue_flag:
                    line_data: LineData = LineData(line[17:], syslog_date)
                    for service in self.services:
                        service.async_analyze(line_data)
                else:
                    self._log.debug(f'{line} not in range, {self.current_time}')
                    break

            except ValueError:
                continue_flag = False

        return continue_flag

    def search_for_services_errors(self) -> int:
        '''
        open syslog file
        search for services errors from the back of the file
        and don't look past the last reboot and the last 10 seconds
        return 0 if no errors found
        return 1 if errors found
        return 2 if syslog file not found
        '''
        if not os.path.exists(self.SYSLOG_PATH):
            return 2

        MAX_CHUNK_SIZE = 0x1000
        try:
            prev_chunk_remainder: str = ''
            with open(self.SYSLOG_PATH, 'r') as syslog:
                syslog.seek(0, os.SEEK_END) #go to end of file
                continue_flag: bool = True
                while continue_flag and syslog.tell() > 0:
                    chunk_size = min(syslog.tell(), MAX_CHUNK_SIZE)
                    syslog.seek(syslog.tell() - chunk_size, os.SEEK_SET)
                    chunk: str = syslog.read(chunk_size) + prev_chunk_remainder
                    syslog.seek(syslog.tell() - chunk_size, os.SEEK_SET)

                    pos = chunk.find('\n')
                    prev_chunk_remainder = chunk[:pos]

                    continue_flag = self.put_chunk_lines_on_queue(chunk[pos + 1:])

        except Exception as e:
            self._log.exception(e)
            return 1

        return 0

    def report(self) -> int:
        super().report()

        for service in self.services:
            service.start()
        ret_val: int = self.search_for_services_errors()
        self.event.set()
        for service in self.services:
            service.stop()

        return ret_val


def get_arguments() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description='find services errors in syslog')
    parser.add_argument('state', choices=['MASTER', 'BACKUP', 'FAULT'], help='state of the machine')
    return parser.parse_args()


def init_logging() -> logging.Logger:
    log_file_name: str = os.path.basename(__file__).split('.')[0]
    logging.basicConfig(filename=f'{log_file_name}.log', format='[%(asctime)s] [%(levelname)s] [%(name)s] [%(funcName)s:%(lineno)d] %(message)s', filemode='w', level=logging.INFO)
    logger = logging.getLogger(__name__)
    logger.addHandler(logging.StreamHandler())

    return logger


def main() -> int:
    logger = init_logging()
    logger.info('starting')

    args: argparse.Namespace = get_arguments()

    state: StateBase = StateFactory.get_state(args.state.upper())
    ret_val: int = state.report()

    logger.info('done')
    return ret_val


#import unittest
# write unit tests for each function in this module
#def test_search_for_services_errors():
#    pass


if __name__ == "__main__":
    exit(main())

