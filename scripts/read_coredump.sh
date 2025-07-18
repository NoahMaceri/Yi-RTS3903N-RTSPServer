#!/bin/bash
#  get parent dir
if [ "$#" -ne 2 ]; then
    echo "Usage: $0 <executable> <core_file>"
    exit 1
fi
if [ ! -f "$1" ]; then
    echo "Executable file $1 does not exist."
    exit 1
fi
if [ ! -f "$2" ]; then
    echo "Core file $2 does not exist."
    exit 1
fi

parent_dir=$(dirname "$0")
parent_dir="$(realpath "$parent_dir/..")"
solib_path="$parent_dir/cmake-build-debug/RTS3903N_RTSP-0.3.0/lib"
sysroot="$parent_dir/third-party/rsdk/rsdk-4.8.5-5281-EL-3.10-u0.9.33-m32fut-161202/mips-linux-uclibc"

echo "Using solib path: $solib_path"
echo "Using sysroot: $sysroot"

./../third-party/rsdk/rsdk-4.8.5-5281-EL-3.10-u0.9.33-m32fut-161202/bin/mips-linux-uclibc-gdb \
 --init-eval-command="set solib-search-path $solib_path" \
 --init-eval-command="set sysroot $sysroot" \
 --se=$1 --core=$2 --batch
