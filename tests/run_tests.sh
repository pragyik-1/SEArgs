#!/usr/bin/env bash
set -e

cd "$(dirname "$0")"

CC="${CC:-cc}"
CFLAGS="${CFLAGS:--std=c11 -Wall -Wextra -Werror}"
BIN="$(mktemp -d)"

trap 'rm -rf "$BIN"' EXIT

echo "== behavior matrix ($CC) =="
"$CC" $CFLAGS -O0 -g test.c -o "$BIN/matrix"
"$BIN/matrix"

echo "== help formatting =="
"$CC" $CFLAGS help_test.c -o "$BIN/help"
"$BIN/help"

echo "== implementation included twice in one TU =="
"$CC" $CFLAGS double_include.c -o "$BIN/double_include"
"$BIN/double_include"

echo "== behavior matrix, NDEBUG release build =="
"$CC" $CFLAGS -O2 -DNDEBUG test.c -o "$BIN/matrix_ndbg"
"$BIN/matrix_ndbg"

echo "== behavior matrix, ASan + UBSan =="
"$CC" $CFLAGS -g -fsanitize=address,undefined -fno-sanitize-recover=all test.c -o "$BIN/matrix_asan"
"$BIN/matrix_asan"

echo "== multi-TU usage (header-only TU + implementation TU) =="
"$CC" $CFLAGS tu_a.c tu_b.c -o "$BIN/tu"
"$BIN/tu"

# The single-allocation guarantee uses the glibc linker's --wrap; skipped on
# platforms where the link step fails.
if "$CC" $CFLAGS -Wl,--wrap=malloc -Wl,--wrap=calloc -Wl,--wrap=free -Wl,--wrap=realloc \
                 alloc_test.c -o "$BIN/alloc" 2>/dev/null; then
  "$BIN/alloc"
  echo "== single-allocation guarantee: PASS =="
else
  echo "== single-allocation guarantee: SKIPPED (requires glibc --wrap) =="
fi

echo "== usage example (examples/example.c) =="
"$CC" $CFLAGS ../examples/example.c -o "$BIN/example"
"$BIN/example" --text hello --count 2 --upper
"$BIN/example" --text hi --count 1 -- -x >/dev/null
"$BIN/example" | grep -q '^Usage:'          # no args: help, success
if "$BIN/example" --count 2 >/dev/null 2>&1; then
  echo "example without required --text should have failed" >&2
  exit 1
fi

echo "== usage example 2 (examples/wc.c) =="
"$CC" $CFLAGS ../examples/wc.c -o "$BIN/wc"
printf 'hello world\nsecond line\n' > "$BIN/in.txt"
"$BIN/wc" -lw "$BIN/in.txt" | grep -q '^2 4 '
"$BIN/wc" -c "$BIN/in.txt" | grep -q '^24 '
"$BIN/wc" < "$BIN/in.txt" | grep -q '^2 4 24$'
if "$BIN/wc" --nope >/dev/null 2>&1; then
  echo "wc should reject an unknown option" >&2
  exit 1
fi
if "$BIN/wc" /no/such/file >/dev/null 2>&1; then
  echo "wc should fail on a missing file" >&2
  exit 1
fi

echo "ALL TESTS PASSED"
