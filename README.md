# MiniCompiler

MiniCompiler is a C++17 educational compiler front-end for a simplified C-like language. The project currently includes three phases:

1. lexical analysis;
2. parsing and AST generation;
3. semantic analysis with symbol tables, type checking, and validation reports.

## Project structure

```text
compiler/
├── CMakeLists.txt
├── README.md
├── docs/
│   ├── language_spec.md
│   └── semantic.md
├── examples/
├── src/
│   ├── lexer/
│   ├── parser/
│   ├── semantic/
│   └── main.cpp
├── scripts/
└── tests/
    ├── core_tests.cpp
    ├── core_tests_extended.cpp
    ├── semantic_tests.cpp
    └── semantic/
```

## Build with CMake

From Developer Command Prompt for VS 2022 on Windows:

```cmd
cd "C:\Users\User\Desktop\Учеба\compiler"
cmake -S . -B build -DBUILD_TESTS=ON
cmake --build build --config Debug
```

Run tests:

```cmd
ctest --test-dir build -C Debug --output-on-failure
```

## Manual build of semantic tests on Windows

```cmd
cl /std:c++17 /EHsc /I src src\lexer\token.cpp src\lexer\lexer.cpp src\parser\ast.cpp src\parser\parser.cpp src\semantic\type_system.cpp src\semantic\symbol_table.cpp src\semantic\errors.cpp src\semantic\analyzer.cpp tests\semantic_tests.cpp /Fe:semantic_tests.exe
semantic_tests.exe
```

Expected result:

```text
Passed: 49
Failed: 0
```

## Lexical analysis

```cmd
build\Debug\compiler.exe lex --input examples\hello.src --output tokens.txt
```

## Parsing and AST output

Text AST:

```cmd
build\Debug\compiler.exe parse --input examples\hello.src --ast-format text --output-file ast.txt
```

DOT AST:

```cmd
build\Debug\compiler.exe parse --input examples\hello.src --ast-format dot --output-file ast.dot
```

## Semantic analysis

Semantic analysis validates declarations, scopes, types, function calls, return statements, and conditions.

```cmd
build\Debug\compiler.exe check --input examples\hello.src --show-types
```

Write semantic report:

```cmd
build\Debug\compiler.exe check --input examples\hello.src --output-file semantic_report.txt --show-types
```

Dump symbol table only:

```cmd
build\Debug\compiler.exe symbols --input examples\hello.src --output-file symbols.txt
```

## Example semantic errors

```text
semantic error: type mismatch: cannot initialize 'x' of type int with bool
  --> program.src:1:21
  = context: in function 'main'

semantic error: undeclared identifier: identifier 'y' is not declared
  --> program.src:1:27
  = context: in function 'main'

semantic error: invalid condition type: if condition must be bool, found int
  --> program.src:1:38
  = context: in function 'main'
```

## Semantic rules

- Duplicate declarations in the same scope are rejected.
- Function declarations support forward references.
- Variables are visible only in their declaring scope and nested scopes.
- `int` can be assigned to `float`, but `float` cannot be assigned to `int`.
- Conditions in `if`, `while`, and `for` must be `bool`.
- Function calls must match parameter count and parameter types.
- Return statements must match the declared function return type.
- Variables declared without initializers are considered uninitialized until assignment.

## Before submission

Do not commit generated or local build files:

```text
.vs/
build/
out/
*.exe
*.obj
*.pdb
*.ilk
*.ipch
semantic_report.txt
symbols.txt
ast.txt
ast.dot
tokens.txt
```
