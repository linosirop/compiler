; MiniCompiler x86-64 assembly
; Target: NASM, ELF64, System V AMD64 ABI
default rel

section .rodata
.L.str0: db 102, 105, 110, 97, 108, 32, 114, 101, 115, 117, 108, 116, 61, 37, 100, 10, 0

section .text
global fact
global main
extern print_int
extern print_string
extern read_int
extern exit
extern printf

fact:
    push rbp
    mov rbp, rsp
    sub rsp, 64
    mov qword [rbp-8], rdi
; stack frame size: 64 bytes
;   n -> [rbp-8]
;   t1 -> [rbp-16]
;   t2 -> [rbp-24]
;   t3 -> [rbp-32]
;   t4 -> [rbp-40]
;   t5 -> [rbp-48]
;   t6 -> [rbp-56]
;   t7 -> [rbp-64]
    ; parameter n
    mov rax, qword [rbp-8]
    mov qword [rbp-8], rax
    ; load n
    mov rax, qword [rbp-8]
    mov qword [rbp-16], rax
    ; t1 <= 1
    ; direct conditional jump for comparison result
    mov rax, qword [rbp-16]
    mov r10, 1
    cmp rax, r10
    jle .fact_L_then1
    jmp .fact_L_endif3
.fact_L_then1:
    mov rax, 1
    mov rsp, rbp
    pop rbp
    ret
.fact_L_endif3:
    ; load n
    mov rax, qword [rbp-8]
    mov qword [rbp-32], rax
    ; load n
    mov rax, qword [rbp-8]
    mov qword [rbp-40], rax
    ; t4 - 1
    mov rax, qword [rbp-40]
    mov r10, 1
    sub rax, r10
    mov qword [rbp-48], rax
    ; argument 0
    ; call fact
    mov rax, qword [rbp-48]
    mov rdi, rax
    call fact
    mov qword [rbp-56], rax
    ; t3 * t6
    mov rax, qword [rbp-32]
    mov r10, qword [rbp-56]
    imul rax, r10
    mov qword [rbp-64], rax
    mov rax, qword [rbp-64]
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
    sub rsp, 112
; stack frame size: 112 bytes
;   arr_0 -> [rbp-8]
;   arr_elem0_1 -> [rbp-16]
;   arr_elem1_2 -> [rbp-24]
;   arr_elem2_3 -> [rbp-32]
;   arr_elem3_4 -> [rbp-40]
;   f_6 -> [rbp-48]
;   folded_7 -> [rbp-56]
;   int -> [rbp-80]
;   int[] -> [rbp-72]
;   sum_5 -> [rbp-64]
;   t10 -> [rbp-96]
;   t12 -> [rbp-104]
;   t13 -> [rbp-112]
;   t8 -> [rbp-88]
    ; declare array int arr
    ; array slot arr[0]
    ; arr[0] = initializer
    mov rax, 1
    mov qword [rbp-16], rax
    ; array slot arr[1]
    ; arr[1] = initializer
    mov rax, 2
    mov qword [rbp-24], rax
    ; array slot arr[2]
    ; arr[2] = initializer
    mov rax, 3
    mov qword [rbp-32], rax
    ; array slot arr[3]
    ; arr[3] = initializer
    mov rax, 4
    mov qword [rbp-40], rax
    ; declare int sum
    ; sum = initializer
    mov rax, 10
    mov qword [rbp-64], rax
    ; declare int f
    ; argument 0
    ; call fact
    mov rax, 5
    mov rdi, rax
    call fact
    mov qword [rbp-88], rax
    ; f = initializer
    mov rax, qword [rbp-88]
    mov qword [rbp-48], rax
    ; declare int folded
    ; folded = initializer
    mov rax, 30
    mov qword [rbp-56], rax
    ; argument 0
    ; load sum
    mov rax, qword [rbp-64]
    mov qword [rbp-96], rax
    ; t10 + t11
    mov rax, qword [rbp-96]
    mov r10, 30
    add rax, r10
    mov qword [rbp-104], rax
    ; argument 1
    ; call printf
    lea rax, [rel .L.str0]
    mov rdi, rax
    mov rax, qword [rbp-104]
    mov rsi, rax
    xor eax, eax    ; variadic call: no vector registers used
    call printf
    mov qword [rbp-112], rax
    mov rax, 0
    mov rsp, rbp
    pop rbp
    ret
; implicit epilogue safeguard
    mov rsp, rbp
    pop rbp
    ret

