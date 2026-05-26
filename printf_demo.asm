; MiniCompiler x86-64 assembly
; Target: NASM, ELF64, System V AMD64 ABI
default rel

section .rodata
.L.str0: db 97, 110, 115, 119, 101, 114, 61, 37, 100, 10, 0

section .text
global main
extern print_int
extern print_string
extern read_int
extern exit
extern printf

main:
    push rbp
    mov rbp, rsp
    sub rsp, 16
    ; argument 0
    ; argument 1
    ; call printf
    lea rax, [rel .L.str0]
    mov rdi, rax
    mov rax, 42
    mov rsi, rax
    xor eax, eax    ; variadic call: no vector registers used
    call printf
    mov qword [rbp-8], rax
    mov rax, 0
    mov rsp, rbp
    pop rbp
    ret
; implicit epilogue safeguard
    mov rsp, rbp
    pop rbp
    ret

