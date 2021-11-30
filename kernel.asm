[BITS 64]
[ORG 0x200000]

start:
	lgdt [gdt_64_ptr]

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


end:
	hlt
	jmp end

gdt_64:
	dq 0
	dq 0x0020980000000000

gdt_64_len: equ $-gdt_64

gdt_64_ptr: dw gdt_64_len - 1
			dq gdt_64
