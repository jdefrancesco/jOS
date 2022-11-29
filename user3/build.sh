#!/usr/bin/env zsh

if [[ -e user || -e *.o ]]
then 
    echo "[+] Cleaning up and rebuilding."
    rm user*; rm *.o
fi 

nasm -felf64 -o start.o start.asm
x86_64-elf-gcc -std=c99 -mcmodel=large -ffreestanding -fno-stack-protector -mno-red-zone -c main.c
x86_64-elf-ld -nostdlib -Tlink.lds -o user start.o main.o lib.a
x86_64-elf-objcopy -O binary user user.bin

if [[ -e user.bin ]]
then
    cp user.bin ../user3.bin && echo "[!] Put user3.bin in boot directory."
fi
