# --- Toolchain ---
AS=nasm
CC=x86_64-elf-gcc
LD=x86_64-elf-ld
OBJCOPY=x86_64-elf-objcopy
CFLAGS=-std=c99 -nostdlib -mcmodel=large -ffreestanding -fno-stack-protector \
       -mno-red-zone -Wall -Wextra -pedantic -Werror
LDFLAGS=-nostdlib -T link.lds

# --- Files ---
BOOT_IMG=boot.img
KERNEL_ELF=kernel
KERNEL_BIN=kernel.bin

# --- Source files ---
C_SRCS=main.c trap.c print.c debug.c memory.c process.c syscall.c klib.c keyboard.c


ASM_OBJ_SRCS=kernel.asm trap_asm.asm klib_asm.asm  # <-- Note: trap.asm renamed to trapa.asm
BIN_ASM_SRCS=boot.asm loader.asm

# --- Object file names ---
C_OBJS=$(C_SRCS:.c=.o)
ASM_OBJS = kernel.o trap_asm.o klib_asm.o
BIN_ASM_OBJS=$(BIN_ASM_SRCS:.asm=.bin)

# --- All objects for linking ---
ALL_OBJS=$(ASM_OBJS) $(C_OBJS)

# --- Default target ---
all: banner $(BOOT_IMG)

# --- Optional: ASCII banner ---
banner:
	@if command -v figlet >/dev/null; then figlet jOS; fi

# --- Build flat binaries (bootloader, loader) ---
%.bin: %.asm
	$(AS) -fbin -o $@ $<

# --- Build assembly object files ---
%.o: %.asm
	$(AS) -felf64 -o $@ $<

# --- Build C object files ---
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# --- Link kernel ---
$(KERNEL_ELF): $(ALL_OBJS)
	$(LD) $(LDFLAGS) -o $@ $^

# --- Convert ELF to flat binary ---
$(KERNEL_BIN): $(KERNEL_ELF)
	$(OBJCOPY) -Obinary $< $@

# --- Create boot.img ---
$(BOOT_IMG): boot.bin loader.bin $(KERNEL_BIN)
	dd if=boot.bin of=$(BOOT_IMG) bs=512 count=1 conv=notrunc
	dd if=loader.bin of=$(BOOT_IMG) bs=512 count=5 seek=1 conv=notrunc
	dd if=$(KERNEL_BIN) of=$(BOOT_IMG) bs=512 count=100 seek=6 conv=notrunc
	dd if=user1.bin of=$(BOOT_IMG) bs=512 count=10 seek=106 conv=notrunc
	dd if=user2.bin of=$(BOOT_IMG) bs=512 count=10 seek=116 conv=notrunc
	dd if=user3.bin of=$(BOOT_IMG) bs=512 count=10 seek=126 conv=notrunc

# --- Clean ---
clean:
	rm -f *.o  $(KERNEL_ELF) $(KERNEL_BIN) $(BOOT_IMG)

.PHONY: all clean banner
