#!/usr/bin/env python3
import argparse
import difflib
import pathlib
import subprocess
import sys

ROOT = pathlib.Path(__file__).resolve().parents[1]


def run(cmd):
    completed = subprocess.run(cmd, text=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    return completed.returncode, completed.stdout, completed.stderr


def compare_exact(name, actual, expected):
    if actual.strip() == expected.strip():
        print(f"[PASS] {name}")
        return True
    print(f"[FAIL] {name}")
    diff = difflib.unified_diff(
        expected.splitlines(), actual.splitlines(),
        fromfile="expected", tofile="actual", lineterm=""
    )
    print("\n".join(diff))
    return False


def contains_all(name, actual, required_parts):
    missing = [part for part in required_parts if part and part not in actual]
    if not missing:
        print(f"[PASS] {name}")
        return True
    print(f"[FAIL] {name}")
    print("Missing expected substrings:")
    for part in missing:
        print(f"  - {part}")
    print("Actual output:")
    print(actual)
    return False


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--compiler", required=True, help="Path to compiler executable")
    args = parser.parse_args()
    compiler = args.compiler

    ok = True

    for src in sorted((ROOT / "tests" / "lexer" / "valid").glob("*.src")):
        expected = src.with_suffix(".expected").read_text(encoding="utf-8")
        code, out, err = run([compiler, "lex", "--input", str(src)])
        ok &= compare_exact(f"lexer valid: {src.name}", out + err, expected)

    for src in sorted((ROOT / "tests" / "lexer" / "invalid").glob("*.src")):
        expected_parts = src.with_suffix(".expected_errors").read_text(encoding="utf-8").splitlines()
        code, out, err = run([compiler, "lex", "--input", str(src)])
        ok &= contains_all(f"lexer invalid: {src.name}", out + err, expected_parts)

    for src in sorted((ROOT / "tests" / "parser" / "valid").glob("*.src")):
        expected = src.with_suffix(".expected").read_text(encoding="utf-8")
        code, out, err = run([compiler, "parse", "--input", str(src), "--ast-format", "text"])
        ok &= compare_exact(f"parser valid: {src.name}", out + err, expected)

    for src in sorted((ROOT / "tests" / "parser" / "invalid").glob("*.src")):
        expected_parts = src.with_suffix(".expected_errors").read_text(encoding="utf-8").splitlines()
        code, out, err = run([compiler, "parse", "--input", str(src), "--ast-format", "text"])
        ok &= contains_all(f"parser invalid: {src.name}", out + err, expected_parts)

    return 0 if ok else 1


if __name__ == "__main__":
    sys.exit(main())
