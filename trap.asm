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

Section .text
extern handler

global vector0
global vector1
global vector2
global vector3
global vector4
global vector5
global vector6
global vector7
global vector8
global vector9
global vector10
global vector11
global vector12
global vector13
global vector14
global vector15
global vector16
global vector17
global vector18
global vector19
global vector20
global vector21
global vector22
global vector23
global vector24
global vector25
global vector26
global vector27
global vector28
global vector29
global vector30
global vector31
global vector32
global vector33
global vector34
global vector35
global vector36
global vector37
global vector38
global vector39

global eoi
global read_isr
global load_idt

trap:
	push_all_regs

	inc byte[0xb8010]
	mov byte[0xb8011], 0xe

	mov rdi, rsp
	call handler

.trap_return:
	pop_all_regs

	add rsp, 16
	iretq


vector0:
	push 0
	push 0
	jmp trap

vector1:
	push 0
	push 1
	jmp trap

vector2:
	push 0
	push 2
	jmp trap

vector3:
	push 0
 	push 3
	jmp trap

vector4:
	push 0
	push 4
	jmp trap

vector5:
	push 0
	push 5
	jmp trap

vector6:
	push 0
	push 6
	jmp trap

vector7:
	push 0
	push 7
	jmp trap

vector8:
	push 8
	jmp trap

vector10:
	push 10
	jmp trap

vector11:
	push 11
	jmp trap

vector12:
	push 12
	jmp trap

vector13:
	push 13
	jmp trap

vector14:
	push 14
	jmp trap

vector16:
	push 0
	push 16
	jmp trap

vector17:
	push 17
	jmp trap

vector18:
	push 0
	push 18
	jmp trap

vector19:
	push 0
	push 19
	jmp trap

vector32:
	push 0
	push 32
	jmp trap

; Spurious IRQ
vector39:
	push 0
	push 39
	jmp trap

eoi:
	mov al, 0x20
	out 0x20, al
	ret

read_isr:
	mov al, 11
	out 0x20, al
	in al, 0x20
	ret

; argument is in rdi
load_idt:
	lidt [rdi]
	ret
