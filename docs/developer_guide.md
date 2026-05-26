# MiniCompiler Developer Guide

## Architecture

```text
source
  -> lexer
  -> parser / AST
  -> semantic analyzer
  -> IR generator
  -> optional IR optimizer
  -> x86-64 code generator
  -> NASM / GCC linker
```

## Key modules

- `src/lexer`: tokenization and lexical errors.
- `src/parser`: recursive descent parser and AST printing.
- `src/semantic`: symbol table, type checking, semantic reports.
- `src/ir`: three-address-like IR, CFG validation, optimizer.
- `src/codegen`: x86-64 System V assembly generation.
- `src/runtime`: minimal Linux runtime used by legacy `ld runtime.o program.o` pipeline.
- `src/utils/diagnostics`: final unified diagnostic formatting helpers.

## Adding a new feature

1. Extend lexer tokens if syntax needs new punctuation or keywords.
2. Extend AST nodes in `src/parser/ast.*`.
3. Extend parser rules in `src/parser/parser.*`.
4. Add semantic checks and type rules.
5. Lower the feature to IR.
6. Generate x86-64 code in `src/codegen`.
7. Add tests under `tests/final` and a demo if appropriate.
