; MiniCompiler x86-64 assembly
; Target: NASM, ELF64, System V AMD64 ABI
default rel

section .text
global main
extern print_int
extern print_string
extern read_int
extern exit

main:
    push rbp
    mov rbp, rsp
    sub rsp, 80
; stack frame size: 80 bytes
;   a_0 -> [rbp-8]
;   b_1 -> [rbp-16]
;   int -> [rbp-24]
;   t1 -> [rbp-80]
;   t2 -> [rbp-32]
;   t3 -> [rbp-40]
;   t4 -> [rbp-48]
;   t5 -> [rbp-56]
;   t6 -> [rbp-64]
;   t7 -> [rbp-72]
    ; declare int a
    ; a = initializer
    mov rax, 0
    mov qword [rbp-8], rax
    ; declare int b
    ; b = initializer
    mov rax, 10
    mov qword [rbp-16], rax
    ; load a
    mov rax, qword [rbp-8]
    mov qword [rbp-32], rax
    ; t2 != 0
    ; direct conditional jump for comparison result
    mov rax, qword [rbp-32]
    mov r10, 0
    cmp rax, r10
    je .main_L_logic_false3
    jmp .main_L_and_rhs1
.main_L_and_rhs1:
    ; load b
    mov rax, qword [rbp-16]
    mov qword [rbp-48], rax
    ; load a
    mov rax, qword [rbp-8]
    mov qword [rbp-56], rax
    ; t4 / t5
    mov rax, qword [rbp-48]
    mov r10, qword [rbp-56]
    cqo
    idiv r10
    mov qword [rbp-64], rax
    ; t6 > 2
    ; direct conditional jump for comparison result
    mov rax, qword [rbp-64]
    mov r10, 2
    cmp rax, r10
    jg .main_L_logic_true2
    jmp .main_L_logic_false3
.main_L_logic_true2:
    ; && result = true
    mov rax, 1
    mov qword [rbp-80], rax
    jmp .main_L_logic_end4
.main_L_logic_false3:
    ; && result = false
    mov rax, 0
    mov qword [rbp-80], rax
    jmp .main_L_logic_end4
.main_L_logic_end4:
    mov rax, qword [rbp-80]
    cmp rax, 0
    jne .main_L_then5
    jmp .main_L_endif7
.main_L_then5:
    mov rax, 99
    mov rsp, rbp
    pop rbp
    ret
.main_L_endif7:
    mov rax, 5
    mov rsp, rbp
    pop rbp
    ret
; implicit epilogue safeguard
    mov rsp, rbp
    pop rbp
    ret

