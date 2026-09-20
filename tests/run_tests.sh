#!/usr/bin/env sh
# Builds and runs the seargs verification suite from tests/.
# CC and CFLAGS are overridable via the environment.
set -e

cd "$(dirname "$0")"

CC="${CC:-cc}"
CFLAGS="${CFLAGS:--std=c11 -Wall -Wextra -Werror}"
BIN="$(mktemp -d)"

trap 'rm -rf "$BIN"' EXIT

echo "== behavior matrix ($CC) =="
"$CC" $CFLAGS -O0 -g test.c -o "$BIN/matrix"
"$BIN/matrix"

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

echo "ALL TESTS PASSED"