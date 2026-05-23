; MiniCompiler x86-64 assembly
; Target: NASM, ELF64, System V AMD64 ABI
default rel

section .text
global add
global main
extern print_int
extern print_string
extern read_int
extern exit

add:
    push rbp
    mov rbp, rsp
    sub rsp, 48
    mov qword [rbp-8], rdi
    mov qword [rbp-16], rsi
; stack frame size: 48 bytes
;   a -> [rbp-8]
;   b -> [rbp-16]
;   t1 -> [rbp-24]
;   t2 -> [rbp-32]
;   t3 -> [rbp-40]
    ; parameter a
    mov rax, qword [rbp-8]
    mov qword [rbp-8], rax
    ; parameter b
    mov rax, qword [rbp-16]
    mov qword [rbp-16], rax
    ; load a
    mov rax, qword [rbp-8]
    mov qword [rbp-24], rax
    ; load b
    mov rax, qword [rbp-16]
    mov qword [rbp-32], rax
    ; t1 + t2
    mov rax, qword [rbp-24]
    mov r10, qword [rbp-32]
    add rax, r10
    mov qword [rbp-40], rax
    mov rax, qword [rbp-40]
    mov rsp, rbp
    pop rbp
    ret
; implicit epilogue safeguard
    mov rsp, rbp
    pop rbp
    ret

main:
    push rbp
    mov rbp, rsp
    sub rsp, 16
; stack frame size: 16 bytes
;   t1 -> [rbp-8]
    ; argument 0
    ; argument 1
    ; call add
    mov rax, 2
    mov rdi, rax
    mov rax, 3
    mov rsi, rax
    call add
    mov qword [rbp-8], rax
    mov rax, qword [rbp-8]
    mov rsp, rbp
    pop rbp
    ret
; implicit epilogue safeguard
    mov rsp, rbp
    pop rbp
    ret

