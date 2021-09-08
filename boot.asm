[BITS 16]
[ORG 0x7C00]

start:
	xor ax, ax
	mov ds, ax
	mov es, ax
	mov ss, ax
	mov sp, 0x7c00

print_message:
	mov ah, 0x13 ; we want to print
	mov al, 1
	mov bx, 0xa	; print bright green
	xor dx, dx
	mov bp, Message ; string to print
	mov cx, MessageLen ; length of string
	int 0x10 ; invoke BIOS

_end:
	hlt
	jmp _end

Message: db "Hello", 0x0a, 0x0
MessageLen: equ $-Message

times (0x1be-($-$$)) db 0
	db 80h 				   ; boot indicator
	db 0,2,0		       ; start CHS
	db 0x0f0		 	   ; type
	db 0x0ff, 0x0ff, 0x0ff ; ending CHS
	dd 1 				   ; starting sector
	dd (20*16*63-1)  ; size partition has (10mb)

	times (16*3) db 0      ; fill with zeros

	db 0x55                ; MBR boot signature
	db 0xAA
