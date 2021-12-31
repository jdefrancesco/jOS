[BITS 64]
[ORG 0x200000]

start:
	mov rdi, idt
	mov rax, handler0

	mov [rdi], ax
	shr rax, 16
	mov [rdi+6], ax ; copy 16-31 to seocnd part, seventh part in entry
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

	xor rbx, rbx
	div rbx


end:
	hlt
	jmp end

; Div by zero handler.
handler0:
	; Save CPU state
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


gdt_64:
	dq 0
	dq 0x0020980000000000

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
