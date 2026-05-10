#!/usr/bin/env bash
set -euo pipefail
c++ -std=c++17 -Wall -Wextra -Wpedantic -Werror \
  -Isrc -Isrc/lexer -Isrc/parser \
  src/lexer/token.cpp src/lexer/lexer.cpp \
  src/parser/ast.cpp src/parser/parser.cpp \
  tests/core_tests.cpp \
  -o core_tests
./core_tests
