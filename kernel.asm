; Macro to save all regs.
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

; Pop all saved regs
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

Section .data

gdt_64:
	dq 0
	dq 0x0020980000000000
	dq 0x0020F80000000000 ; DPL will be 11 (ring 3)
	dq 0x0000F20000000000 ; P = 1, DPL = 11, W = 1

; TSS (Task State Segment) setup. An x86 specific structure holding information about
; a task. This is pointed to by the TR register which contains the segment selector; 
; located in the GDT. TSS holds the following information:
; - Processor register state
; - I/O port permissions
tss_desc:
	dw tss_len-1
	dw 0
	db 0
	db 0x89 ; P = 1, DPL = 00, TYPE = 01001
	db 0
	db 0
	dq 0

; GDT lengths and location.
gdt_64_len: equ $-gdt_64
gdt_64_ptr: dw gdt_64_len - 1
			dq gdt_64
tss:
	dd 0
	dq 0xffff800000190000
	times 88 db 0
	dd tss_len

tss_len: equ $-tss



; Interupt Descriptor Table setup...
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


Section .text

; C kmain function
extern kmain

global start

; Initialize out GDT
start:
	mov rax, gdt_64_ptr
	lgdt [rax]

setTss:
	mov rax, tss
	mov rdi, tss_desc
	mov [rdi+2], ax
	shr rax, 16
	mov [rdi+4], al
	shr rax, 8
	mov [rdi+7], al
	shr rax, 8
	mov [rdi+8], eax
	mov ax, 0x20
	ltr ax



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

; Initialize APIC. 
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
	mov al, 0b11111111
	out 0xa1, al

	mov rax, kernel_entry
	push 8
	push rax
	db 0x48 ; this will prefix
	retf ; far return will pop IP and CS register (same priv level)
		 ; NOTE: The default operand size of far return is 32 bits
		 ; but we are working with 64 bits so we supply operand
		 ; size prefix to retf (0x48) which will let us work with 8 byte operand.

kernel_entry:
	mov rsp, 0xffff800000200000
	call kmain

end:
	hlt
	jmp end


; Div by zero handler.
timer:
	push_all_regs

	inc byte[0xb8020]
	mov byte[0xb8021], 0xe

	mov al, 0x20
	out 0x20, al

	pop_all_regs
	iretq

; Spurious interrupt handler
SIRQ:
	push_all_regs

	mov al, 11
	out 0x20, al
	in al, 0x20

	test al, (1<<7)
	jz .end

	mov al, 0x20
	out 0x20, al

	iretq
.end:
	pop_all_regs

