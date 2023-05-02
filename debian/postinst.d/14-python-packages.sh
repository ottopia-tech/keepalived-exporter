#!/bin/bash

main() {
    env python3.9 -m pip install -r /usr/share/ottopia-keepalived-exporter/requirements.txt
    return $?
}

main
