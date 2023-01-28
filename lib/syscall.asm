Section .text 

; EXPORT These symbolS
global writeu
global sleepu
global exitu
global waitu
global keyboard_readu

;int writeu(char *buff (rdi), int buffer_size (rsi));
writeu: 
    sub rsp, 16
    xor eax, eax

    mov [rsp], rdi
    mov [rsp+8], rsi

    mov rdi, 2
    mov rsi, rsp
    int 0x80

    add rsp, 16
    ret


sleepu:
    sub rsp,8
    mov eax,1

    mov [rsp],rdi
    mov rdi,1
    mov rsi,rsp

    int 0x80

    add rsp,8
    ret

exitu:
    mov eax, 2
    mov rdi, 0

    int 0x80
    ret

waitu:
    mov eax, 3
    mov rdi, 0
    ret

keyboard_readu:
    mov eax, 4
    xor edi, edi
    int 0x80
    ret