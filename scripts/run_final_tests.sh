#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$ROOT"

BUILD_DIR="${BUILD_DIR:-build}"
COMPILER="$BUILD_DIR/mycc"

if [[ ! -x "$COMPILER" ]]; then
  echo "[final-tests] mycc not found, building first..."
  cmake -S . -B "$BUILD_DIR" -DBUILD_TESTS=ON -G "Unix Makefiles"
  cmake --build "$BUILD_DIR"
fi

echo "[final-tests] running CTest"
ctest --test-dir "$BUILD_DIR" --output-on-failure

tmpdir="$(mktemp -d)"
trap 'rm -rf "$tmpdir"' EXIT

run_exit_test() {
  local name="$1"
  local src="$2"
  local expected="$3"
  local asm="$tmpdir/$name.asm"
  local obj="$tmpdir/$name.o"
  local exe="$tmpdir/$name"
  echo "[final-tests] $name"
  "$COMPILER" -S "$src" -o "$asm" --optimize >/dev/null
  nasm -f elf64 -o "$obj" "$asm"
  gcc -no-pie -o "$exe" "$obj"
  set +e
  "$exe" >/dev/null
  local status=$?
  set -e
  if [[ "$status" != "$expected" ]]; then
    echo "[final-tests] FAIL $name: expected exit $expected, got $status"
    exit 1
  fi
}

run_output_test() {
  local name="$1"
  local src="$2"
  local expected="$3"
  local exe="$tmpdir/$name"
  echo "[final-tests] $name"
  "$COMPILER" "$src" -o "$exe" --optimize >/dev/null
  local output
  output="$($exe)"
  if [[ "$output" != "$expected" ]]; then
    echo "[final-tests] FAIL $name: unexpected output"
    echo "expected:"; printf '%s\n' "$expected"
    echo "actual:"; printf '%s\n' "$output"
    exit 1
  fi
}

run_exit_test arithmetic tests/final/good/arithmetic/simple.src 14
run_exit_test short_circuit tests/final/good/control_flow/short_circuit.src 5
run_exit_test recursive_factorial tests/final/good/functions/recursive_factorial.src 120
run_exit_test static_array tests/final/good/arrays/static_array.src 10
run_output_test final_showcase demo/final_showcase.src $'final result=40'

echo "[final-tests] checking bad programs"
if "$COMPILER" tests/final/bad/syntax_errors/missing_semicolon.src -o "$tmpdir/bad" >/dev/null 2>&1; then
  echo "[final-tests] FAIL syntax error program compiled successfully"
  exit 1
fi
if "$COMPILER" tests/final/bad/semantic_errors/undefined_variable.src -o "$tmpdir/bad" >/dev/null 2>&1; then
  echo "[final-tests] FAIL semantic error program compiled successfully"
  exit 1
fi

echo "[final-tests] all final tests passed"
