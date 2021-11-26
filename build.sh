#!/usr/bin/env zsh


if command -v figlet &> /dev/null; then
    figlet jOS
fi


# Build BIOS MBR, early boot coode....
nasm -fbin -o boot.bin boot.asm
nasm -fbin -o loader.bin loader.asm
nasm -fbin -o kernel.bin kernel.asm
dd if=boot.bin of=boot.img bs=512 count=1 conv=notrunc
dd if=loader.bin of=boot.img bs=512 count=5 seek=1 conv=notrunc
dd if=kernel.bin of=boot.img bs=512 count=100 seek=6 conv=notrunc

