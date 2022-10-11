#!/usr/bin/env zsh


if command -v figlet &> /dev/null; then
    figlet jOS
fi


# Build BIOS MBR, early boot coode...n
nasm -fbin -o boot.bin boot.asm
nasm -fbin -o loader.bin loader.asm
nasm -felf64 -o kernel.o kernel.asm
nasm -felf64 -o trapa.o trap.asm
nasm -felf64 -o kliba.o klib.asm

# Use our cross compilers here..
x86_64-elf-gcc -std=c99 -nostdlib -mcmodel=large -ffreestanding -fno-stack-protector -mno-red-zone -c main.c
x86_64-elf-gcc -std=c99 -nostdlib -mcmodel=large -ffreestanding -fno-stack-protector -mno-red-zone -c trap.c
x86_64-elf-gcc -std=c99 -nostdlib -mcmodel=large -ffreestanding -fno-stack-protector -mno-red-zone -c print.c
x86_64-elf-gcc -std=c99 -nostdlib -mcmodel=large -ffreestanding -fno-stack-protector -mno-red-zone -c debug.c
x86_64-elf-gcc -std=c99 -nostdlib -mcmodel=large -ffreestanding -fno-stack-protector -mno-red-zone -c memory.c
x86_64-elf-gcc -std=c99 -nostdlib -mcmodel=large -ffreestanding -fno-stack-protector -mno-red-zone -c process.c

x86_64-elf-ld -nostdlib -T link.lds -o kernel kernel.o main.o trapa.o trap.o kliba.o print.o debug.o memory.o process.o
x86_64-elf-objcopy -Obinary kernel kernel.bin

dd if=boot.bin of=boot.img bs=512 count=1 conv=notrunc
dd if=loader.bin of=boot.img bs=512 count=5 seek=1 conv=notrunc
dd if=kernel.bin of=boot.img bs=512 count=100 seek=6 conv=notrunc

