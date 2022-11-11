#!/usr/bin/env zsh



nasm -felf64 -o start.o start.asm
x86_64-elf-gcc -std=c99 -mcmodel=large -ffreestanding -fno-stack-protector -mno-red-zone -c main.c
x86_64-elf-ld -nostdlib -T link.lds -o user start.o main.o lib.a
x86_64-elf-objcopy -Obinary user user.bin


