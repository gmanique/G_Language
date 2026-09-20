#!/usr/bin/env bash
# Usage: tests/run_tests.sh [path/to/glang]      (default: ./build/glang)
#
# Conventions (tests/cases/):
#   ok_*.gg    must compile           -> exit code 0
#   err_*.gg   must be rejected       -> exit code 1 (NOT a hang, NOT a crash).
#              If err_X.stderr exists, its content is a grep -E pattern that
#              must match the compiler's stderr.
#   todo_*.gg  known missing feature (semantic analysis): should be rejected
#              (exit 1). While it still exits 0 it is reported as XFAIL and
#              does not fail the run. When it starts passing: XPASS, rename it
#              to err_*.gg.
# Generated at run time (too big to keep in the repo): deep nesting and a very
# long expression, to catch stack overflows and quadratic parsing.

BIN="${1:-./build/glang}"
DIR="$(cd "$(dirname "$0")" && pwd)/cases"
TMP="$(mktemp -d)"; trap 'rm -rf "$TMP"' EXIT
TIMEOUT="${TIMEOUT:-5}"

[ -x "$BIN" ] || { echo "compiler not found: $BIN"; exit 2; }

pass=0; fail=0; xfail=0; xpass=0

# run FILE -> sets RC and ERR (stdout is discarded: the compiler dumps tokens/AST)
run() {
  timeout "$TIMEOUT" "$BIN" "$1" >/dev/null 2>"$TMP/stderr.txt"
  RC=$?
  ERR="$(head -c 2000 "$TMP/stderr.txt")"
}
describe_rc() {
  case "$1" in 124) echo "TIMEOUT (infinite loop?)";; 134) echo "ABORT";;
               139) echo "SEGFAULT";; *) echo "exit=$1";; esac
}
ok()   { printf '  \033[32mPASS\033[0m  %s\n' "$1"; pass=$((pass+1)); }
bad()  { printf '  \033[31mFAIL\033[0m  %s  (%s)\n' "$1" "$2"; fail=$((fail+1)); }

check_expect() { # name file expected_rc
  local name="$1" f="$2" want="$3"
  run "$f"
  if [ "$RC" -ne "$want" ]; then bad "$name" "$(describe_rc $RC), expected exit=$want"; return 1; fi
  if [ "$want" -eq 1 ] && [ -f "${f%.gg}.stderr" ]; then
    local pat; pat="$(cat "${f%.gg}.stderr")"
    if ! grep -Eq -- "$pat" <<<"$ERR"; then
      bad "$name" "stderr does not match /$pat/ ; got: $(head -c 120 <<<"$ERR" | tr '\n' ' ')"; return 1
    fi
  fi
  if grep -Eq 'AddressSanitizer|runtime error:' <<<"$ERR"; then bad "$name" "sanitizer report"; return 1; fi
  ok "$name"
}

echo "== ok_*"
for f in "$DIR"/ok_*.gg;  do check_expect "$(basename "$f")" "$f" 0; done
echo "== err_*"
for f in "$DIR"/err_*.gg; do check_expect "$(basename "$f")" "$f" 1; done

echo "== generated stress cases"
{ printf 'fun main() i32 { return ('; printf '(%.0s' $(seq 5000); printf '1'; printf ')%.0s' $(seq 5000); printf '); }\n'; } > "$TMP/err_deep_parens.gg"
echo 'nesting too deep' > "$TMP/err_deep_parens.stderr"
check_expect "err_deep_parens (5000 levels)" "$TMP/err_deep_parens.gg" 1

{ printf 'fun main() i32 {'; for _ in $(seq 600); do printf ' {'; done; for _ in $(seq 600); do printf ' }'; done; printf ' return (0); }\n'; } > "$TMP/err_deep_blocks.gg"
echo 'nesting too deep' > "$TMP/err_deep_blocks.stderr"
check_expect "err_deep_blocks (600 levels)" "$TMP/err_deep_blocks.gg" 1

{ printf 'fun main() i32 { return (1'; for _ in $(seq 20000); do printf '+1'; done; printf '); }\n'; } > "$TMP/ok_long_chain.gg"
check_expect "ok_long_chain (20000 terms, must stay fast)" "$TMP/ok_long_chain.gg" 0

echo "== cli"
run "$TMP/does_not_exist.gg"
if [ "$RC" -eq 1 ] && grep -q "Cannot read" <<<"$ERR"; then ok "missing file -> 'Cannot read'"; else bad "missing file" "$(describe_rc $RC): $ERR"; fi
run /tmp
if [ "$RC" -eq 1 ]; then ok "directory as input -> exit 1"; else bad "directory as input" "$(describe_rc $RC)"; fi
out1="$(timeout "$TIMEOUT" "$BIN" "$DIR/ok_basic.gg" 2>&1 | md5sum)"
out2="$(timeout "$TIMEOUT" "$BIN" "$DIR/ok_basic.gg" "$DIR/ok_basic.gg" 2>&1 | md5sum)"
if [ "$out1" = "$out2" ]; then ok "same file twice == once"; else bad "same file twice" "output differs"; fi

echo "== todo_* (semantic analysis, not implemented yet)"
for f in "$DIR"/todo_*.gg; do
  n="$(basename "$f")"; run "$f"
  if   [ "$RC" -eq 1 ]; then printf '  \033[36mXPASS\033[0m %s  -> now rejected, rename it to err_*.gg\n' "$n"; xpass=$((xpass+1))
  elif [ "$RC" -eq 0 ]; then printf '  \033[33mXFAIL\033[0m %s  (accepted, should be rejected)\n' "$n"; xfail=$((xfail+1))
  else bad "$n" "$(describe_rc $RC)"; fi
done

echo
echo "pass=$pass fail=$fail xfail=$xfail xpass=$xpass"
[ "$fail" -eq 0 ]
