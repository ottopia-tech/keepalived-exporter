#!/bin/bash

main() {
    wget https://bootstrap.pypa.io/get-pip.py && \
      env python3.9 get-pip.py && \
      env python3.9 -m pip install -r /usr/share/ottopia-keepalived-exporter/requirements.txt
    return $?
}

main
