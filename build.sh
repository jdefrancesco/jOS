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
x86_64-elf-gcc -std=c99 -nostdlib -mcmodel=large -ffreestanding -fno-stack-protector -mno-red-zone -Wall -Wextra -pedantic -Werror -c main.c
x86_64-elf-gcc -std=c99 -nostdlib -mcmodel=large -ffreestanding -fno-stack-protector -mno-red-zone -Wall -Wextra -pedantic -Werror -c trap.c
x86_64-elf-gcc -std=c99 -nostdlib -mcmodel=large -ffreestanding -fno-stack-protector -mno-red-zone -Wall -Wextra -pedantic -Werror -c print.c
x86_64-elf-gcc -std=c99 -nostdlib -mcmodel=large -ffreestanding -fno-stack-protector -mno-red-zone -Wall -Wextra -pedantic -Werror -c debug.c
x86_64-elf-gcc -std=c99 -nostdlib -mcmodel=large -ffreestanding -fno-stack-protector -mno-red-zone -Wall -Wextra -pedantic -Werror -c memory.c
x86_64-elf-gcc -std=c99 -nostdlib -mcmodel=large -ffreestanding -fno-stack-protector -mno-red-zone -Wall -Wextra -pedantic -Werror -c process.c
x86_64-elf-gcc -std=c99 -nostdlib -mcmodel=large -ffreestanding -fno-stack-protector -mno-red-zone -Wall -Wextra -pedantic -Werror -c syscall.c
x86_64-elf-gcc -std=c99 -nostdlib -mcmodel=large -ffreestanding -fno-stack-protector -mno-red-zone -Wall -Wextra -pedantic -Werror -c klib.c
x86_64-elf-gcc -std=c99 -nostdlib -mcmodel=large -ffreestanding -fno-stack-protector -mno-red-zone -Wall -Wextra -pedantic -Werror -c keyboard.c 
# Link it all...
x86_64-elf-ld -nostdlib -T link.lds -o kernel kernel.o main.o trapa.o trap.o kliba.o print.o debug.o memory.o process.o syscall.o klib.o keyboard.o

# Prepare binary images for emulator...
x86_64-elf-objcopy -Obinary kernel kernel.bin
dd if=boot.bin of=boot.img bs=512 count=1 conv=notrunc
dd if=loader.bin of=boot.img bs=512 count=5 seek=1 conv=notrunc
dd if=kernel.bin of=boot.img bs=512 count=100 seek=6 conv=notrunc
# Write them user mode programs we will load
dd if=user1.bin of=boot.img bs=512 count=10 seek=106 conv=notrunc
dd if=user2.bin of=boot.img bs=512 count=10 seek=116 conv=notrunc
dd if=user3.bin of=boot.img bs=512 count=10 seek=126 conv=notrunc


# Now pray to the Gods your monstrosity of an OS will work....