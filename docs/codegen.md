# Sprint 5: x86-64 Code Generation

MiniCompiler generates NASM-compatible x86-64 assembly from the Sprint 4 IR.
The backend targets Linux ELF64 and follows the System V AMD64 ABI. On Windows 11,
use WSL2/Ubuntu to assemble, link, and run generated programs.

## Pipeline

```text
source .src -> lexer -> parser -> semantic analyzer -> IR -> x86-64 assembly
```

## Command line

```bash
./compiler compile --input examples/add.src --output add.asm --target x86_64
```

Optional flags:

- `--emit-stack-map` prints stack slot comments in the generated assembly.
- `--verbose` also enables stack map comments.

## Assembly format

The generated file uses NASM Intel syntax:

```asm
section .text
global main

main:
    push rbp
    mov rbp, rsp
    sub rsp, 16
    ; body
    mov rsp, rbp
    pop rbp
    ret
```

## Supported IR instructions

The first Sprint 5 backend supports integer and boolean code generation:

- arithmetic: `ADD`, `SUB`, `MUL`, `DIV`, `MOD`, `NEG`;
- logical: `AND`, `OR`, `XOR`, `NOT`;
- comparisons: `CMP_EQ`, `CMP_NE`, `CMP_LT`, `CMP_LE`, `CMP_GT`, `CMP_GE`;
- memory: `ALLOCA`, `LOAD`, `STORE`, `MOVE`;
- control flow: `JUMP`, `JUMP_IF`, `JUMP_IF_NOT`;
- functions: `CALL`, `PARAM`, `RETURN`.

Floating-point and rich string literal lowering are intentionally left for a later extension.

## System V AMD64 ABI notes

- Integer arguments 1-6 are passed in `rdi`, `rsi`, `rdx`, `rcx`, `r8`, `r9`.
- Integer return values are returned in `rax`.
- The stack frame uses `rbp` as a frame pointer.
- Local variables and IR temporaries are stored in fixed negative offsets from `rbp`.
- Stack allocation is rounded to a 16-byte boundary.
- The generator uses caller-saved scratch registers such as `rax` and `r10`.

## Runtime library

`src/runtime/runtime.asm` provides:

- `_start`: process entry point, calls `main`, then exits with `main`'s return value;
- `exit`: Linux `exit` syscall;
- `print_int`: prints an integer argument from `rdi`;
- `print_string`: prints a null-terminated string pointer from `rdi`;
- `read_int`: reads and parses a decimal integer from stdin.

## Assemble and link in WSL/Linux

```bash
nasm -f elf64 -o program.o program.asm
nasm -f elf64 -o runtime.o src/runtime/runtime.asm
ld -o program runtime.o program.o
./program
echo $?
```

## Example

Source:

```c
fn add(int a, int b) -> int {
    return a + b;
}

fn main() -> int {
    return add(2, 3);
}
```

Build and run:

```bash
./build/compiler compile --input tests/codegen/valid/function_calls/add_call.src --output add.asm --target x86_64
nasm -f elf64 -o add.o add.asm
nasm -f elf64 -o runtime.o src/runtime/runtime.asm
ld -o add_program runtime.o add.o
./add_program
echo $?   # expected: 5
```
