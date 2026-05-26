# MiniCompiler User Guide

MiniCompiler can be used in two styles.

## Legacy command style

```bash
./build/compiler lex --input examples/hello.src --output tokens.txt
./build/compiler parse --input examples/hello.src --ast-format text
./build/compiler check --input examples/hello.src --show-types
./build/compiler ir --input examples/factorial.src --stats --validate
./build/compiler compile --input demo/final_showcase.src --output final_showcase.asm --target x86_64
```

## Final `mycc` style

```bash
./build/mycc --help
./build/mycc --version
./build/mycc -S program.src -o program.asm
./build/mycc -c program.src -o program.o
./build/mycc program.src -o program
./build/mycc --ast program.src
./build/mycc --ir program.src --optimize --optimization-report
```

## Notes for Windows 11

The project can be edited on Windows, but the backend targets Linux x86-64. Use WSL2 Ubuntu for building, assembling, linking, and running generated programs.

Recommended location:

```bash
~/projects/compiler
```

Avoid building directly from `/mnt/c/...`.
