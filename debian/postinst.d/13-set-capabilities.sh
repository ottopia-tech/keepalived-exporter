#!/bin/bash

keepalived-exporter-capabilities() {
    log_message ${LOG_LEVEL_DEBUG} "set capabilities to keepalived-exporter binary"

    # Give capabilities to signal keepalived, and read the output files
    setcap "CAP_DAC_READ_SEARCH,CAP_KILL=+eip" /opt/ottopia/keepalived-exporter/keepalived-exporter
    local RET_VAL=$?
    if [ ${RET_VAL} -ne 0 ]
    then
        log_message ${LOG_LEVEL_ERROR} "Error when setting cap. look at above output"
        exit ${RET_VAL}
    fi

    log_message ${LOG_LEVEL_DEBUG} "Done set capabilities to binary"
}

interface-capabilities() {
    log_message ${LOG_LEVEL_DEBUG} "set capabilities to interface binary"

    # Give capabilities to signal keepalived, and read the output files
    setcap "CAP_NET_ADMIN,CAP_NET_RAW+ep" /opt/ottopia/keepalived-exporter/interface
    local RET_VAL=$?
    if [ ${RET_VAL} -ne 0 ]
    then
        log_message ${LOG_LEVEL_ERROR} "Error when setting cap. look at above output"
        exit ${RET_VAL}
    fi

    log_message ${LOG_LEVEL_DEBUG} "Done set capabilities to binary"
}

main() {
    log_message ${LOG_LEVEL_DEBUG} "set capabilities to binaries"

    interface-capabilities
    keepalived-exporter-capabilities

    log_message ${LOG_LEVEL_DEBUG} "Done set capabilities to binaries"
}

main
