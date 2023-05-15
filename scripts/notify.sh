#!/bin/bash

set_variables() {
    readonly internet_device=$1
    readonly backup_address=$2
    readonly gw=$3
    readonly mode=$6
}

setup_device_ip() {
    if [ "${mode}" == "MASTER" ]
    then
	ip route del default via ${gw} src ${backup_address} dev ${internet_device}
        ip address del ${backup_address}/24 dev ${internet_device}
    else
        ip address add ${backup_address}/24 dev ${internet_device}
	ip route add default via ${gw} src ${backup_address} dev ${internet_device}
    fi
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

main $@
