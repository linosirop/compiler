#!/usr/bin/env bash
set -euo pipefail
if [[ $# -lt 1 ]]; then
  echo "Usage: scripts/build_and_run.sh <file.src> [output]"
  exit 1
fi
src="$1"
out="${2:-a.out}"
cmake -S . -B build -DBUILD_TESTS=ON -G "Unix Makefiles"
cmake --build build
./build/mycc "$src" -o "$out" --optimize -v
"./$out"
