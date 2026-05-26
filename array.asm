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
    sub rsp, 112
; stack frame size: 112 bytes
;   arr_0 -> [rbp-8]
;   arr_elem0_1 -> [rbp-16]
;   arr_elem1_2 -> [rbp-24]
;   arr_elem2_3 -> [rbp-32]
;   int -> [rbp-48]
;   int[] -> [rbp-40]
;   t1 -> [rbp-56]
;   t2 -> [rbp-64]
;   t3 -> [rbp-72]
;   t4 -> [rbp-80]
;   t5 -> [rbp-88]
;   t6 -> [rbp-96]
;   t7 -> [rbp-104]
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
    ; load array element
    mov rax, qword [rbp-24]
    mov qword [rbp-56], rax
    ; t1 + 4
    mov rax, qword [rbp-56]
    mov r10, 4
    add rax, r10
    mov qword [rbp-64], rax
    ; array element = value
    mov rax, qword [rbp-64]
    mov qword [rbp-24], rax
    ; load array element
    mov rax, qword [rbp-16]
    mov qword [rbp-72], rax
    ; load array element
    mov rax, qword [rbp-24]
    mov qword [rbp-80], rax
    ; t3 + t4
    mov rax, qword [rbp-72]
    mov r10, qword [rbp-80]
    add rax, r10
    mov qword [rbp-88], rax
    ; load array element
    mov rax, qword [rbp-32]
    mov qword [rbp-96], rax
    ; t5 + t6
    mov rax, qword [rbp-88]
    mov r10, qword [rbp-96]
    add rax, r10
    mov qword [rbp-104], rax
    mov rax, qword [rbp-104]
    mov rsp, rbp
    pop rbp
    ret
; implicit epilogue safeguard
    mov rsp, rbp
    pop rbp
    ret

