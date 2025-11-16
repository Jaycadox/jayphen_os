#!/bin/bash
set -x

if [ ! -d edk2 ]; then
    git clone https://github.com/tianocore/edk2 --depth 1
fi

if [ ! -d obj ]; then
    mkdir obj
fi

if [ ! -d root ]; then
    mkdir -p root/EFI/boot
    echo "StartKernel.elf" > root/AUTORUN
fi

make qemu
