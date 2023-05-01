#!/usr/bin/python3
import _io
import json
import argparse
import sys
import os
import traceback
import shutil

INDENT = '    '
PRIORITY = 100
ADVERT_INT = 1
STATE = 'BACKUP'
AUTH_PASS = 'p@55w0rd'
CHECK_SCRIPTS = {'hm_rs': 58080, 'hm_sm': 58081, 'hm_cm': 58082, 'hm_asm': 58083}
INTERVAL = 1
TIMEOUT = 5
RISE = 1
FALL = 2


def print_error(error_message: str):
    print(f'While executing {" ".join(sys.argv)}')
    traceback.print_exc()
    print(error_message)


def get_args() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser()
    subparsers = parser.add_subparsers(title='Configuring Keepalived',
                                       description='valid subcommands',
                                       required=True,
                                       help='Configuration options')
    action_restore = subparsers.add_parser('restore', description='overwrite the current configuration with the backup config file')
    action_restore.set_defaults(func=restore_orig_cfg)
    action_restore.add_argument('-o', '--out', required=True, default='/etc/keepalived/keepalived.conf',
                        help='output of the script: keealived final configuration file')

    action_overwrite = subparsers.add_parser('overwrite', description='overwrite the backup configuration with the current config file')
    action_overwrite.set_defaults(func=overwrite_orig_cfg)
    action_overwrite.add_argument('-o', '--out', required=True, default='/etc/keepalived/keepalived.conf',
                        help='output of the script: keealived final configuration file')

    action_write = subparsers.add_parser('write')
    action_write.set_defaults(func=write_new_cfg)
    action_write.add_argument('-o', '--out', required=True, default='/etc/keepalived/keepalived.conf',
                        help='output of the script: keealived final configuration file')

    action_write.add_argument('-c', '--config', required=True, help='json configuration parameters for keepalived',
                        type=argparse.FileType('r'))
    action_write.add_argument('-d', '--directory', default='/opt/ottopia', help='ottopia-root-dir. e.g. /opt/ottopia')
    action_write.add_argument('-u', '--user', default='ottopia', help='user that keepalived will use to execute vrrp_scripts')
    action_write.add_argument('-g', '--group', default='ottopia_keepalived_service', help='group of the user that keepalived will use to execute vrrp_scripts')

    try:
        return parser.parse_args()
    except OSError:
        print_error()
        return None


def read_configuration_parameters_file(config_file: _io.TextIOWrapper):
    try:
        config = json.load(config_file)
    except json.JSONDecodeError:
        print_error(f"Error parsing configuration file")
        return None

    return config


def get_vrrp_and_track_scripts_section(user_name: str, group_name: str):
    # Generate VRRP script configuration for each health check script
    # Generate health check scripts configuration for track_script section
    vrrp_scripts = []
    track_scripts = []
    CURL = shutil.which("curl")

    for (name, port) in CHECK_SCRIPTS.items():
        exec_script = f'script \"{CURL} http://localhost:{port}/metrics\"'
        interval = f'interval {INTERVAL}'
        timeout = f'timeout {TIMEOUT}'
        rise = f'rise {RISE}'
        fall = f'fall {FALL}'
        user = f'user {user_name} {group_name}'
        vrrp_script = '\n'.join(
            (f'vrrp_script {name} {{', INDENT + f'\n{INDENT}'.join((exec_script, interval, timeout, rise, fall, user)),
             '}'))

        vrrp_scripts.append(vrrp_script)
        track_scripts.append(name)

    track_script_section = f'\n{INDENT}'.join(('track_script {', INDENT + f'\n{INDENT * 2}'.join(track_scripts), '}'))
    vrrp_scripts_section = '\n\n'.join(vrrp_scripts)
    return vrrp_scripts_section, track_script_section


def get_authentication_section():
    auth_type = 'auth_type PASS'
    auth_pass = f'auth_pass {AUTH_PASS}'
    authentication_section = f'\n{INDENT}'.join(
        ('authentication {', INDENT + f'\n{INDENT * 2}'.join((auth_type, auth_pass)), '}'))
    return authentication_section


def get_virtual_ipaddress_section(config: json):
    try:
        internet_network_params = f'{config["keepalived"]["internet_virtual_ip"]}/24 dev {config["keepalived"]["internet_interface"]}'
        stations_network_params = f'{config["keepalived"]["stations_virtual_ip"]}/24 dev {config["global"]["stations_interface"]}'
    except KeyError:
        print_error(f'Error reading configs')
        return None

    virtual_ipaddress_section = f'\n{INDENT}'.join(('virtual_ipaddress {', INDENT + f'\n{INDENT * 2}'.join(
        (internet_network_params, stations_network_params)), '}'))
    return virtual_ipaddress_section


def get_virtual_routes_section(config: json):
    try:
        route_params = ' '.join((
            '0.0.0.0/0 via',
            config['keepalived']['internet_gateway'],
            'dev',
            config['keepalived']['internet_interface'],
            'src',
            config['keepalived']['internet_virtual_ip']))
    except KeyError:
        print_error(f'Error reading configs')
        return None

    virtual_routes_section = f'\n{INDENT}'.join(('virtual_routes {', INDENT + route_params, '}'))
    return virtual_routes_section


def get_vi_section(config: json, track_script_section: str, file_name: str, user: str):
    try:
        vrrp_instance = f'vrrp_instance {config["keepalived"]["machine_name"]} {{'
        interface = f'interface {config["keepalived"]["keepalived_interface"]}'
        notify = f'notify "{file_name} {config["keepalived"]["internet_interface"]} {config["keepalived"]["backup_ip"]}" {user}'
    except KeyError:
        print_error(f'Error reading configs')
        return None

    state = f'state {STATE}'
    virtual_router_id = f'virtual_router_id {config["keepalived"]["virtual_router_id"]}'
    priority = f'priority {PRIORITY}'
    advert_int = f'advert_int {ADVERT_INT}'

    authentication_section = get_authentication_section()
    virtual_ipaddress_section = get_virtual_ipaddress_section(config)
    if virtual_ipaddress_section is None:
        return None

    virtual_routes_section = get_virtual_routes_section(config)
    if virtual_routes_section is None:
        return None

    vi_section = '\n'.join((vrrp_instance, INDENT + f'\n{INDENT}'.join((state, interface, virtual_router_id,
                                                                                 priority, advert_int,
                                                                                 notify,
                                                                                 authentication_section,
                                                                                 virtual_ipaddress_section,
                                                                                 virtual_routes_section,
                                                                                 track_script_section)), '}'))
    return vi_section


def get_global_defs_section():
    global_defs_section = '\n'.join((
        'global_defs {',
        INDENT + f'\n{INDENT}'.join((
            '# Don\'t run scripts configured to be run as root if any part of the path',
            '# is writable by a non-root user.',
            'enable_script_security')),
        '}'))
    return global_defs_section


def create_keepalived_configuration(config: json, vrrp_scripts_section: str, track_script_section: str, file_name: str, user: str):
    global_defs_section = get_global_defs_section()
    vi_section = get_vi_section(config, track_script_section, file_name, user)
    if vi_section is None:
        return None

    vrrp_config = '\n\n'.join((global_defs_section, vrrp_scripts_section, vi_section, ''))
    return vrrp_config


def save_file(file_name: str, file_data: str):
    # Make sure parent dir exists
    parent = os.path.dirname(file_name)
    try:
        if parent:
            os.makedirs(parent, exist_ok=True)

        with open(file_name, 'w') as f:
            f.write(file_data)
    except PermissionError:
        print_error(f'Missing permissions to create/overwrite file {file_name}')
        return False
    except OSError:
        print_error(f'file name: {file_name}')
        return False

    return True


def overwrite_orig_cfg(args: argparse.ArgumentParser) -> int:
    current: str = args.out
    if not os.path.exists(current):
        print(f'There is no current configuration file: {current}')
        return 2

    try:
        shutil.copy2(current, f'{current}.orig')
    except PermissionError:
        print_error(f'Missing permissions to create/overwrite file {current}.orig')
        return 2
    except Exception:
        print_error(f'Caught a general exception')
        return 1

    return 0


def restore_orig_cfg(args: argparse.ArgumentParser) -> int:
    current: str = args.out
    if not os.path.exists(f'{current}.orig'):
        print(f'There is no backup configuration file: {current}.orig')
        return 2

    try:
        shutil.copy2(f'{current}.orig', current)
    except PermissionError:
        print_error(f'Missing permissions to overwrite file {current}')
        return 2
    except Exception:
        print_error(f'Caught a general exception')
        return 1

    return 0


def save_orig_keepalived_cfg(current: str) -> int:
    if not os.path.exists(f'{current}'):
        return 0

    try:
        shutil.copy2(current, f'{current}.orig')
    except PermissionError:
        print_error(f'Missing permissions to overwrite file {current}.orig')
        return 2
    except Exception:
        print_error(f'Caught a general exception')
        return 1

    return 0


def write_new_cfg(args: argparse.ArgumentParser) -> int:
    try:
        ret_val = save_orig_keepalived_cfg(args.out)
        if ret_val != 0:
            return ret_val

        config = read_configuration_parameters_file(args.config)
        if config is None:
            return 2

        vrrp_scripts, track_script = get_vrrp_and_track_scripts_section(args.user, args.group)
        vrrp_config = create_keepalived_configuration(config, vrrp_scripts, track_script, args.directory + '/keepalived/notify.sh', args.user)
        if vrrp_config is None:
            return 2

        if not save_file(args.out, vrrp_config):
            return 2

    except Exception:
        print_error(f'Caught a general exception')
        return 1

    print(f'Successfully written: {args.out}')
    return 0


def main():
    args: argparse.ArgumentParser = get_args()
    if args is None:
        return 2

    return args.func(args)


if __name__ == '__main__':
    sys.exit(main())
