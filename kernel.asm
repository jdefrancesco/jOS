[BITS 64]
[ORG 0x200000]

%macro push_all_regs 0

	push rax
	push rbx
	push rcx
	push rdx
	push rsi
	push rdi
	push rbp
	push r8
	push r9
	push r10
	push r11
	push r12
	push r13
	push r14
	push r15

%endmacro


%macro pop_all_regs 0

	pop r15
	pop r14
	pop r13
	pop r12
	pop r11
	pop r10
	pop r9
	pop r8
	pop rbp
	pop rdi
	pop rsi
	pop rdx
	pop rcx
	pop rbx
	pop rax

%endmacro

start:
	mov rdi, idt
	mov rax, handler0

	mov [rdi], ax
	shr rax, 16
	mov [rdi+6], ax ; copy 16-31 to seocnd part, seventh part in entry
	shr rax, 16
	mov [rdi+8], eax

	mov rax, timer
	add rdi, 32*16 ; Point to timer entry
	mov [rdi], ax
	shr rax, 16
	mov [rdi+6], ax
	shr rax, 16
	mov [rdi+8], eax

	lgdt [gdt_64_ptr]
	lidt [idt_ptr]

	push 8
	push kernel_entry
	db 0x48 ; this will prefix
	retf ; far return will pop IP and CS register (same priv level)
		 ; NOTE: The default operand size of far return is 32 bits
		 ; but we are working with 64 bits so we supply operand
		 ; size prefix to retf (0x48) which will let us work with 8 byte operand.

kernel_entry:
	mov byte[0xb8000], 'K'
	mov byte[0xb8001], 0xa


init_pit:
	; We will initialize the programmable interrupt timer.
	; We will use 100Hz. ie The timer will "pop" 100 times a second.

	mov al, (1<<2)|(3<<4)
	out 0x43, al

	; We will use 100Hz
	mov ax, 11931
	out 0x40, al
	mov al, ah
	out 0x40, al

init_pic:
	; Init PIC
	mov al, 0x11
	out 0x20, al
	out 0xa0, al

	mov al, 32
	out 0x21, al
	mov al, 40
	out 0xa1, al

	mov al, 4
	out 0x21, al
	mov al, 2
	out 0xa1, al

	mov al, 1
	out 0x21, al
	out 0xa1, al

	mov al, 0b11111110
	out 0x21, al
	mov al, 0x11111111b
	out 0xa1, al

	; sti ; enable interrupts.

	push 0x18|3
	push 0x7c00
	push 0x2
	push 0x10|3
	push user_entry
	iretq


end:
	hlt
	jmp end

user_entry:
	mov ax, cs
	and al, 11b
	cmp al, 3
	jne uend

	mov byte[0xb8010], 'U'
	mov byte[0xb8011], 0xE

uend:
	jmp  uend


; Div by zero handler.
handler0:
	; Save CPU state when interrupt or exception occurs.
	push rax
	push rbx
	push rcx
	push rdx
	push rsi
	push rdi
	push rbp
	push r8
	push r9
	push r10
	push r11
	push r12
	push r13
	push r14
	push r15

	mov byte[0xb8000], 'D'
	mov byte[0xb8001], 0xc

	jmp end

	; Pop everything in reverse order.
	pop r15
	pop r14
	pop r13
	pop r12
	pop r11
	pop r10
	pop r9
	pop r8
	pop rbp
	pop rdi
	pop rsi
	pop rdx
	pop rcx
	pop rbx
	pop rax

	iretq

timer:
	push_all_regs

	mov byte[0xb8010], 'T'
	mov byte[0xb8011], 0xe
	jmp end

	pop_all_regs
	iretq

gdt_64:
	dq 0
	dq 0x0020980000000000
	dq 0x0020F80000000000 ; DPL will be 11 (ring 3)
	dq 0x0000F20000000000 ; P = 1, DPL = 11, W = 1
tss_desc:
	dw tss_len-1
	dw 0
	db 0
	db 0x89 ; P = 1, DPL = 00, TYPE = 01001
	db 0
	db 0
	dq 0


gdt_64_len: equ $-gdt_64

gdt_64_ptr: dw gdt_64_len - 1
			dq gdt_64

idt:
	%rep 256
		dw 0
		dw 0x8
		db 0
		db 0x8e
		dw 0
		dd 0
		dd 0
	%endrep

idt_len: equ $-idt

idt_ptr: dw idt_len-1
		 dq idt

tss:
	dd 0
	dq 0x150000
	times 88 db 0
	dd tss_len

tss_len equ $-tss
