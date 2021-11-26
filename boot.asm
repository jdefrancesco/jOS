[BITS 16]
[ORG 0x7C00]



start:
	cli
	xor ax, ax
	mov ds, ax
	mov es, ax
	mov ss, ax
	mov sp, 0x7c00
	sti

test_disk_extension:
	mov [DriveId], dl
	mov ah, 0x41
	mov bx, 0x55aa
	int 0x13
	jc not_supported ; carry flag will be set if it isn't supported
	cmp bx, 0xaa55
	jne not_supported

load_loader:
	mov si, ReadPacket ; Fill data structure.
	mov word[si], 0x10
	mov word[si+2], 5 ; read five sectors
	mov word[si+4], 0x7e00
	mov word[si+6], 0
	mov dword[si+8], 1
	mov dword[si+0xc], 0
	mov dl, [DriveId]
	mov ah, 0x42 ; We wan't to use disk extension service
	int 0x13
	jc read_error

	mov dl, [DriveId]
	jmp 0x7e00 ; jump to code

not_supported:
read_error:
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

DriveId:	db 0
Message: db "We have error in boot process.", 0x0a, 0x0
MessageLen: equ $-Message
ReadPacket: times 16 db 0

times (0x1be-($-$$)) db 0
	db 80h 				   ; boot indicator
	db 0,2,0		       ; start CHS
	db 0xf0		 	   	   ; type
	db 0xff, 0xff, 0xff    ; ending CHS
	dd 1 				   ; starting sector
	dd (20*16*63-1)  	   ; size partition has (10mb)

	times (16*3) db 0      ; fill with zeros

	db 0x55                ; MBR boot signature
	db 0xAA
