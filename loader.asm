[BITS 16]
[ORG 0x7e00]

%define GREEN 0xa

;==============Memory Map=======================
;
; [ Free_3   	      ] - (0x100000 and up)
; [ Reserved  		  ] - (0x80000 - (Free_3-1))
; [ Kernel   		  ] - (0x10000 - (Reserved-1))
; [ Free_2   		  ]
; [ Loader   		  ] - (0x7e00 - (Free_2-1))
; [ Boot              ] - (0x7c00 - (Loader-1))
; [ Free_1            ]
; [ BIOS Data/Vectors ] (0 - (Free_1-1))
;
;===============================================

;===============================================
; Memory Map Struct.
; 0 - base address
; 8 - length
; 16 - type
;===============================================


start:
	mov [DriveId], dl ; dl set from boot.asm

	mov eax, 0x80000000
	cpuid
	cmp eax, 0x80000001
	jb not_supported

	; Check is long-mode is supported
	mov eax, 0x80000001
	cpuid
	test edx, (1<<29)
	jz not_supported
	test edx, (1<<26)
	jz not_supported


load_kernel:
	mov si, ReadPacket
	mov word[si], 0x10
	mov word[si+2], 100
	mov word[si+4], 0x0
	mov word[si+6], 0x1000
	mov dword[si+8], 6
	mov dword[si+0xc], 0
	mov dl, [DriveId]
	mov ah, 0x42 ; We wan't to use disk extension service
	int 0x13
	jc read_error

get_mem_info_start:
	mov eax, 0xe820
	mov edx, 0x534d4150
	mov ecx, 20
	mov edi, 0x9000
	xor ebx, ebx
	int 0x15
	jc not_supported

get_mem_info:
	add edi, 20
	mov eax, 0xe820
	mov edx, 0x534d4150
	mov ecx, 20
	int 0x15
	jc get_mem_done ; end of block

	test ebx, ebx
	jnz get_mem_info

get_mem_done:
;	mov ah, 0x13
;	mov al, 1
;	mov bx, 0xa
;	xor dx, dx
;	mov bp, Message ; string to print
;	mov cx, MessageLen ; length of string
;	int 0x10 ; invoke BIOS

test_a20:
	mov ax, 0xffff
	mov es, ax
	mov word[ds:0x7c00], 0xa200 ; --> 0x7c00
	cmp word[es:0x7c10], 0xa200 ; --> 0x107c00
	jmp set_a20_line_done
	mov word[0x7c00], 0xb200
	cmp word[es:0x7c10], 0xb200
	je end

set_a20_line_done:
	xor ax, ax
	mov es, ax

; 80x25
set_vid_mode:
	mov ax, 3 ; Setup text mode
	int 0x10

	; Setup GDT,IDT
	cli
	lgdt [gdt_32_ptr]
	lidt [idt_32_ptr]

	; Enable protected mode
	mov eax, cr0
	or eax, 1
	mov cr0, eax

	jmp 8:pm_entry

read_error:
not_supported:
end:
	hlt
	jmp end

; Protected Mode....
[BITS 32]

pm_entry:
	mov ax, 0x10
	mov ds, ax
	mov es, ax
	mov ss, ax
	mov esp, 0x7c00

; Setup and enable paging
	cld
	mov edi, 0x80000
	xor eax, eax
	mov ecx, 0x10000/4
	rep stosd

	mov dword[0x80000], 0x81007
	mov dword[0x81000], 0b10000111

	lgdt [gdt_64_ptr]

	mov eax, cr4
	or eax, (1 << 5)
	mov cr4, eax

	mov eax, 0x80000 ; still physical addresses. we need to map VM
	mov cr3, eax

	mov ecx, 0xc0000080
	rdmsr
	or eax, (1<<8)
	wrmsr

	mov eax, cr0
	or eax, (1<<31)
	mov cr0, eax

	jmp 8:lm_entry

pend:
	hlt
	jmp pend

; Long mode entry...

[BITS 64]

lm_entry:
	mov rsp, 0x7c00
	mov byte[0xb8000], 'L'
	mov byte[0xb8001], 0xa


DriveId: db 0
ReadPacket: times 16 db 0

gdt_32:
	dq 0
code_32:
	dw 0xffff
	dw 0
	db 0
	db 0x9a
	db 0xcf
	db 0
data_32:
	dw 0xffff
	dw 0
	db 0
	db 0x92
	db 0xcf
	db 0

gdt_32_len: equ $-gdt_32
gdt_32_ptr: dw gdt_32_len-1
			dd gdt_32
idt_32_ptr: dw 0


gdt_64:
	dq 0
	dq 0x0020980000000000
gdt_64_len: equ $-gdt_64

gdt_64_ptr: dw gdt_64_len - 1
			dd gdt_64

