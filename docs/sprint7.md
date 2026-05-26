# Sprint 7: Arrays, external calls and optimizations

Sprint 7 extends the backend after Sprint 6.

## Implemented parts

### Optimizer

Implemented in `src/ir/optimizer.h` and `src/ir/optimizer.cpp`.

Supported optimization passes:

- constant folding: `10 + 20 -> 30`, `60 > 50 -> true`;
- constant propagation through simple `STORE` / `LOAD` patterns;
- branch simplification for constant conditions;
- local dead code elimination for unused temporaries and instructions after terminators;
- optimization report with number of performed changes.

Example:

```bash
./build/compiler ir --input tests/optimization/constant_folding.src --optimize --optimization-report --stats
```

### Static array support

Implemented as a minimal stack-backed array model.

Supported syntax:

```c
int arr[3] = {1, 2, 3};
arr[1] = arr[1] + 4;
return arr[0] + arr[1] + arr[2];
```

The current backend stores array elements as stack slots such as `arr_elem0`, `arr_elem1`, `arr_elem2`.
This makes the generated code easy to inspect and keeps Sprint 5/6 stack-frame design stable.

Demo:

```bash
./build/compiler compile --input tests/arrays/valid/static_array.src --output array.asm --target x86_64 --emit-stack-map
```

### External function calls

Supported syntax:

```c
extern int printf(string fmt, int value);
```

The generator emits:

- `.rodata` string labels;
- `extern printf`;
- argument passing through System V registers;
- `xor eax, eax` before variadic `printf` calls.

Demo:

```bash
./build/compiler compile --input tests/external_calls/valid/printf_demo.src --output printf_demo.asm --target x86_64
nasm -f elf64 -o printf_demo.o printf_demo.asm
gcc -no-pie -o printf_demo printf_demo.o
./printf_demo
```

## Limitations

The Sprint 7 implementation focuses on a stable MVP:

- static one-dimensional arrays with constant indices are supported;
- array elements are represented as stack slots in generated code;
- external calls currently use simple MiniCompiler prototypes rather than full C headers;
- full multidimensional arrays, dynamic indexing, `malloc/free` semantics and full `printf` varargs type checking can be extended later.
