; runtime.asm - Minimal Linux x86-64 runtime for MiniCompiler
; NASM syntax, ELF64, System V AMD64 ABI

default rel

section .text

global _start
global exit
global print_int
global print_string
global read_int

extern main

_start:
    call main
    mov rdi, rax
    call exit

exit:
    mov rax, 60
    syscall

print_string:
    push rbp
    mov rbp, rsp

    mov rsi, rdi
    xor rdx, rdx

.count_loop:
    cmp byte [rsi + rdx], 0
    je .write
    inc rdx
    jmp .count_loop

.write:
    mov rax, 1
    mov rdi, 1
    syscall

    mov rsp, rbp
    pop rbp
    ret

print_int:
    push rbp
    mov rbp, rsp
    sub rsp, 64

    mov rax, rdi
    lea rsi, [rbp - 1]
    mov byte [rsi], 10
    mov rcx, 1

    cmp rax, 0
    jne .convert

    dec rsi
    mov byte [rsi], '0'
    inc rcx
    jmp .write_int

.convert:
    mov r8, 0
    cmp rax, 0
    jge .digits

    neg rax
    mov r8, 1

.digits:
    xor rdx, rdx
    mov rbx, 10
    div rbx
    add dl, '0'
    dec rsi
    mov [rsi], dl
    inc rcx
    cmp rax, 0
    jne .digits

    cmp r8, 0
    je .write_int

    dec rsi
    mov byte [rsi], '-'
    inc rcx

.write_int:
    mov rax, 1
    mov rdi, 1
    mov rdx, rcx
    syscall

    mov rsp, rbp
    pop rbp
    ret

read_int:
    xor rax, rax
    ret
