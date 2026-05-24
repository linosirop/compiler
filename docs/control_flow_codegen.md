# Sprint 6: Control Flow Code Generation

Sprint 6 extends the x86-64 backend with complex control flow and short-circuit boolean evaluation.

## Supported constructs

- `if` and `if/else`
- nested conditionals
- `while` loops
- `for` loops
- relational operators: `<`, `<=`, `>`, `>=`, `==`, `!=`
- logical operators: `&&`, `||`, `!`
- complex arithmetic expressions with parser-level precedence

## Conditional jumps

The backend recognizes comparison results followed by conditional branches and emits direct signed jumps:

| Source operator | Assembly jump |
|---|---|
| `<` | `jl` |
| `<=` | `jle` |
| `>` | `jg` |
| `>=` | `jge` |
| `==` | `je` |
| `!=` | `jne` |

For example, `if (x < 10)` lowers to `cmp` followed by `jl` instead of materializing a temporary boolean when a direct branch is possible.

## Short-circuit evaluation

`&&` and `||` are lowered in IR using labels and jumps.

For `a && b`:

1. evaluate `a`;
2. jump to false if `a` is false;
3. evaluate `b` only if `a` is true;
4. merge into a 0/1 boolean result.

For `a || b`:

1. evaluate `a`;
2. jump to true if `a` is true;
3. evaluate `b` only if `a` is false;
4. merge into a 0/1 boolean result.

This prevents unsafe right-side evaluation such as `b / a` when `a == 0`.

## Example commands

```bash
./build/compiler compile --input tests/control_flow/valid/logical_ops/short_circuit_and.src --output sc_and.asm --target x86_64 --emit-stack-map
nasm -f elf64 -o sc_and.o sc_and.asm
nasm -f elf64 -o runtime.o src/runtime/runtime.asm
ld -o sc_and_program runtime.o sc_and.o
./sc_and_program
echo $?
```

Expected result: `5`.
