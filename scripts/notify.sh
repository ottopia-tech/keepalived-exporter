#!/bin/bash

set_variables() {
    readonly internet_device=$1
    shift
    readonly backup_address=$1
    shift
    # if number of remaining arguments is greater than 3
    if [ $# -gt 3 ]
    then
        readonly netmask=$1
        shift
    fi
    while [ $# -gt 3 ]; do
        shift
    done
    readonly instance=$1
    readonly priority=$2
    readonly mode=$3
}

setup_device_ip() {
    local -r cur_dir=$(dirname $(realpath $0))

    ${cur_dir}/../interface ${internet_device} ${backup_address} ${netmask} ${instance} ${priority} ${mode}
}

report_reliability_metrics() {
    local -r cur_dir=$(dirname $(realpath $0))

    ${cur_dir}/reliability_metrics.py ${mode}
}

main() {
    set_variables $@
    setup_device_ip
    report_reliability_metrics
}

main @
