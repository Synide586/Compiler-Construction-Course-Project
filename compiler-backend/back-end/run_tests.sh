#!/bin/bash
# Compiles and runs every program in tests/run (comparing its output with the .expected file),
# then checks that every program in tests/errors is rejected with the message on its "% EXPECT:" line.
cd "$(dirname "$0")"
make -s compiler || exit 1
mkdir -p build
pass=0; fail=0

for src in tests/run/*.txt; do
  name=$(basename "$src" .txt)
  if ! ./compiler "$src" -o "build/$name" > "build/$name.log" 2>&1; then
    echo "FAIL  $name (did not compile)"; sed 's/^/      /' "build/$name.log"; fail=$((fail+1)); continue
  fi
  input="tests/run/$name.in"; [ -f "$input" ] || input=/dev/null
  "./build/$name" < "$input" > "build/$name.out" 2>&1
  if diff -u "tests/run/$name.expected" "build/$name.out" > "build/$name.diff"; then
    echo "PASS  $name"; pass=$((pass+1))
  else
    echo "FAIL  $name (wrong output)"; sed 's/^/      /' "build/$name.diff"; fail=$((fail+1))
  fi
done

for src in tests/errors/*.txt; do
  name=$(basename "$src" .txt)
  expected=$(sed -n 's/^% EXPECT: //p' "$src" | head -1)
  if ./compiler "$src" --check > "build/$name.log" 2>&1; then
    echo "FAIL  $name (was accepted, expected: $expected)"; fail=$((fail+1))
  elif grep -qF "$expected" "build/$name.log"; then
    echo "PASS  $name"; pass=$((pass+1))
  else
    echo "FAIL  $name (expected message: $expected)"; sed 's/^/      /' "build/$name.log"; fail=$((fail+1))
  fi
done

echo "----------------------------------------"
echo "$pass passed, $fail failed"
[ "$fail" -eq 0 ]
