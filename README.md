# MiniCompiler

MiniCompiler is an educational compiler front-end for a simplified C-like language. The project currently implements two main stages:

1. **Lexical analysis**: converts source code into a stream of tokens with line and column information.
2. **Syntax analysis**: parses the token stream with a recursive descent parser and builds an Abstract Syntax Tree (AST).

Author: Panin Ivan

## Project Structure

```text
compiler-project/
├── CMakeLists.txt
├── README.md
├── docs/
│   └── language_spec.md
├── examples/
├── src/
│   ├── lexer/
│   │   ├── lexer.cpp
│   │   ├── lexer.h
│   │   ├── token.cpp
│   │   └── token.h
│   ├── parser/
│   │   ├── ast.cpp
│   │   ├── ast.h
│   │   ├── grammar.txt
│   │   ├── parser.cpp
│   │   └── parser.h
│   └── main.cpp
├── scripts/
└── tests/
```

## Requirements

- CMake 3.15 or newer
- C++17 compiler
- Windows: Visual Studio 2022 Developer Command Prompt is recommended
- Linux/macOS: GCC or Clang

## Build Instructions

### Windows 11 with Visual Studio Developer Command Prompt

Open **x64 Native Tools Command Prompt for VS 2022** or **Developer Command Prompt for VS 2022**, then run:

```cmd
cd "C:\Users\User\Desktop\Учеба\compiler"
cmake -S . -B build
cmake --build build --config Debug
```

The executable will usually be placed in:

```text
build\Debug\compiler.exe
```

### Linux/macOS or Windows with MinGW

```bash
cmake -S . -B build
cmake --build build
```

## Command-Line Usage

### Lexical analysis

```cmd
build\Debug\compiler.exe lex --input examples\hello.src --output tokens.txt
```

Expected token output format:

```text
LINE:COLUMN TOKEN_TYPE "LEXEME" [LITERAL_VALUE]
```

Example:

```text
1:1 KW_FN "fn"
1:4 IDENTIFIER "main"
1:8 LPAREN "("
1:9 RPAREN ")"
1:11 LBRACE "{"
```

### Parser and AST text output

```cmd
build\Debug\compiler.exe parse --input examples\hello.src --ast-format text --output-file ast.txt
```

### Parser and Graphviz DOT output

```cmd
build\Debug\compiler.exe parse --input examples\hello.src --ast-format dot --output-file ast.dot
```

To convert DOT into an image, install Graphviz and run:

```cmd
dot -Tpng ast.dot -o ast.png
```

### Verbose mode

```cmd
build\Debug\compiler.exe parse --input examples\hello.src --ast-format text --verbose
```

## Example Source Program

```c
fn main() -> void {
    int counter = 42;
    bool ok = counter > 0 && counter < 100;
    return;
}
```

## Language Overview

The language supports:

- functions declared with `fn`;
- optional return type syntax: `fn name(...) -> type`;
- primitive types: `int`, `float`, `bool`, `void`;
- user-defined type names through identifiers;
- structures declared with `struct`;
- variable declarations;
- assignments and compound assignments;
- `if` / `else`;
- `while` and `for` loops;
- `return` statements;
- function calls;
- integer, floating-point, string, and boolean literals.

The lexical grammar is documented in `docs/language_spec.md`.
The parser grammar is documented in `src/parser/grammar.txt`.

## Testing

The project includes standalone C++ tests that do not require GoogleTest.

### Extended tests on Windows 11 with Visual Studio compiler

From the project root:

```cmd
cl /std:c++17 /EHsc /I src src\lexer\token.cpp src\lexer\lexer.cpp src\parser\ast.cpp src\parser\parser.cpp tests\core_tests_extended.cpp /Fe:core_tests_extended.exe
core_tests_extended.exe
```

Expected result after the current fixes:

```text
Passed: 83
Failed: 0
```

### Extended tests through CMake

If `tests/core_tests_extended.cpp` exists, CMake creates an additional test executable automatically:

```cmd
cmake -S . -B build -DBUILD_TESTS=ON
cmake --build build --config Debug
ctest --test-dir build -C Debug --output-on-failure
```

## Important Syntax Rules

Comparison and equality operators are **non-associative**. This means the following expression is invalid:

```c
bool x = 1 < 2 < 3;
```

Use explicit logical conjunction instead:

```c
bool x = (1 < 2) && (2 < 3);
```

Assignment targets must be identifiers. These are invalid:

```c
1 = x;
foo() = 2;
```

This is valid:

```c
x = 2;
```

## Repository Hygiene

Before submitting the project, remove generated files and build artifacts:

```text
.vs/
build/
out/
*.exe
*.obj
*.pdb
*.ilk
*.ipch
tokens.txt
ast.txt
ast.dot
ast.png
```

Only source code, tests, documentation, examples, scripts, and build configuration should remain in the final archive.
