# Sprint 8 Final Demo

This demo is designed for the final presentation of MiniCompiler.

## Demo source

```text
demo/final_showcase.src
```

The program demonstrates:

- external C call through `printf`;
- recursive function `factorial`;
- static stack array;
- `for` loop;
- arithmetic and constant folding;
- x86-64 code generation and linking through `gcc -no-pie`.

## Build and run

```bash
cmake -S . -B build -DBUILD_TESTS=ON -G "Unix Makefiles"
cmake --build build

./build/mycc demo/final_showcase.src -o final_showcase --optimize -v
./final_showcase
```

Expected output:

```text
sum=10
fact=120
folded=30
```

## Assembly-only mode

```bash
./build/mycc -S demo/final_showcase.src -o final_showcase.asm --optimize
cat final_showcase.asm
```

## Object-only mode

```bash
./build/mycc -c demo/final_showcase.src -o final_showcase.o --optimize
```

## IR and optimization report

```bash
./build/mycc --ir tests/optimization/constant_folding.src --optimize --optimization-report
```
