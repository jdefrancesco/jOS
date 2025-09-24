AS=nasm
CC=x86_64-elf-gcc
LD=x86_64-elf-ld
OBJCOPY=x86_64-elf-objcopy
CFLAGS=-std=c99 -nostdlib -mcmodel=large -ffreestanding -fno-stack-protector \
       -mno-red-zone -Wall -Wextra -pedantic -Werror
LDFLAGS=-nostdlib -T link.lds

BOOT_IMG=boot.img # Need to create this file
KERNEL_ELF=kernel
KERNEL_BIN=kernel.bin

C_SRCS=main.c trap.c print.c debug.c memory.c process.c syscall.c klib.c keyboard.c


ASM_OBJ_SRCS=kernel.asm trap_asm.asm klib_asm.asm
BIN_ASM_SRCS=boot.asm loader.asm

C_OBJS=$(C_SRCS:.c=.o)
ASM_OBJS = kernel.o trap_asm.o klib_asm.o
BIN_ASM_OBJS=$(BIN_ASM_SRCS:.asm=.bin)

ALL_OBJS=$(ASM_OBJS) $(C_OBJS)

all:  $(BOOT_IMG)


%.bin: %.asm
	$(AS) -fbin -o $@ $<

%.o: %.asm
	$(AS) -felf64 -o $@ $<

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

$(KERNEL_ELF): $(ALL_OBJS)
	$(LD) $(LDFLAGS) -o $@ $^

$(KERNEL_BIN): $(KERNEL_ELF)
	$(OBJCOPY) -Obinary $< $@

$(BOOT_IMG): boot.bin loader.bin $(KERNEL_BIN)
	dd if=boot.bin of=$(BOOT_IMG) bs=512 count=1 conv=notrunc
	dd if=loader.bin of=$(BOOT_IMG) bs=512 count=5 seek=1 conv=notrunc
	dd if=$(KERNEL_BIN) of=$(BOOT_IMG) bs=512 count=100 seek=6 conv=notrunc
	dd if=user1.bin of=$(BOOT_IMG) bs=512 count=10 seek=106 conv=notrunc
	dd if=user2.bin of=$(BOOT_IMG) bs=512 count=10 seek=116 conv=notrunc
	dd if=user3.bin of=$(BOOT_IMG) bs=512 count=10 seek=126 conv=notrunc

.PHONY: clean
clean:
	rm -f *.o  $(KERNEL_ELF) $(KERNEL_BIN)

