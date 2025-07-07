#!/bin/bash
# check for 2 arguments
if [ "$#" -ne 2 ]; then
    echo "Usage: $0 <file_path> <host>"
    exit 1
fi
socat -u FILE:$1 TCP:$2:9876