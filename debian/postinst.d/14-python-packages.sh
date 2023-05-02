#!/bin/bash

main() {
    env python3.9 -m pip install -r /opt/ottopia/keepalived-exporter/scripts/requirements.txt
    return $?
}

main
