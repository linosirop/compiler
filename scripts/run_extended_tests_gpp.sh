#!/usr/bin/env bash
set -euo pipefail
g++ -std=c++17 -Wall -Wextra -Wpedantic -Isrc \
  src/lexer/token.cpp \
  src/lexer/lexer.cpp \
  src/parser/ast.cpp \
  src/parser/parser.cpp \
  tests/core_tests_extended.cpp \
  -o core_tests_extended
./core_tests_extended
