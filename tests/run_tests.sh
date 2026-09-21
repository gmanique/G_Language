#!/usr/bin/env bash
# =============================================================================
#  G_Language test runner
#
#  Usage: tests/run_tests.sh [options] [path/to/glang]     (default: ./build/glang)
#
#  Options
#    -v, --verbose     list every test (PASS and XFAIL too), not only problems
#    -q, --quiet       only the final summary
#    -k TEXT           run only tests whose path contains TEXT   (e.g. -k parser/err_enum)
#    --list            list the static tests that would run, then exit
#    --update          rewrite the existing golden files (*.tokens / *.ast) from
#                      the current output. To create a new golden: `touch X.ast`
#                      next to X.gg, then run with --update. REVIEW THE DIFF (git diff).
#    --no-static --no-stress --no-cli --no-fuzz     skip a group
#    --fuzz-only       only the fuzz group
#    -h, --help
#
#  Environment
#    TIMEOUT=10        seconds allowed per compiler run   (a hang is a FAIL)
#    FUZZ_LEVEL=1      0 = off, 1 = quick (default), 2 = thorough (many more inputs)
#    FUZZ_SEEDS="a.gg b.gg"   fuzz these files instead of the default seeds
#    FUZZ_FAIL_DIR     where fuzz reproducers are saved (default /tmp/glang_fuzz_failures)
#    NO_COLOR=1        disable colors
#
#  Test files live in tests/cases/<category>/. The PREFIX of the file (or directory)
#  name says what is expected:
#
#    ok_*     the whole pipeline (lex, parse, analyse) succeeds: exit 0, empty stderr.
#    lex_*    lexing must succeed. Parsing and analysis are not judged, so the file
#             does not have to be a valid program.
#    parse_*  lexing AND parsing must succeed. Analysis is not judged, so undeclared
#             identifiers are fine. (Use it for grammar tests.)
#    err_*    must be rejected: exit 1 with a message on stderr.
#    todo_*   a check that is NOT implemented yet: should be rejected (exit 1).
#             Reported XFAIL while it is still accepted, XPASS when it starts working
#             (then rename it to err_*). Never fails the run.
#
#  Optional companions of X.gg (for a directory test: expected.<ext> inside it):
#    X.stderr   one grep -E pattern per line, ALL must match stderr. A line starting
#               with '!' must NOT match. Blank lines and lines starting with '#' are ignored.
#    X.tokens   golden dump of the lexer output (section "Lex : [ ... ]")
#    X.ast      golden dump of the AST, with the numeric token ids removed, so that
#               adding a token kind does not invalidate every golden
#
#  A DIRECTORY named ok_*/err_*/lex_*/parse_*/todo_* is a multi-file test: every
#  *.gg inside is passed to the compiler, in alphabetical order.
#
#  Checks applied to EVERY test (not only the ones that expect a failure):
#    - no timeout, no crash/signal, no exit code other than 0 or 1
#    - no sanitizer / assertion / "terminate called" report on stderr
#    - no absurd line number in a diagnostic (e.g. 1678051880: an uninitialised token)
#    - err_* must explain itself: an empty stderr is a failure
# =============================================================================
export LC_ALL=C
set -u

HERE="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
CASES="$HERE/cases"
TIMEOUT="${TIMEOUT:-10}"
FUZZ_LEVEL="${FUZZ_LEVEL:-1}"
FUZZ_FAIL_DIR="${FUZZ_FAIL_DIR:-/tmp/glang_fuzz_failures}"

VERBOSE=0
QUIET=0
FILTER=""
UPDATE=0
LIST=0
RUN_STATIC=1
RUN_STRESS=1
RUN_CLI=1
RUN_FUZZ=1
BIN=""
RC=0
CRASH=""
PAT_WHY=""
GOLD_WHY=""
FUZZ_WHY=""

usage() {
	awk 'NR>1 && /^# =+$/ { c++; if (c==2) exit } c==1 && NR>2 { sub(/^# ?/, ""); print }' "${BASH_SOURCE[0]}"
}

while [ $# -gt 0 ]; do
	case "$1" in
	-h | --help)
		usage
		exit 0
		;;
	-v | --verbose) VERBOSE=1 ;;
	-q | --quiet) QUIET=1 ;;
	-k)
		[ $# -ge 2 ] || {
			echo "-k needs an argument"
			exit 2
		}
		FILTER="$2"
		shift
		;;
	--list) LIST=1 ;;
	--update) UPDATE=1 ;;
	--no-static) RUN_STATIC=0 ;;
	--no-stress) RUN_STRESS=0 ;;
	--no-cli) RUN_CLI=0 ;;
	--no-fuzz) RUN_FUZZ=0 ;;
	--fuzz-only)
		RUN_STATIC=0
		RUN_STRESS=0
		RUN_CLI=0
		RUN_FUZZ=1
		;;
	-*)
		echo "unknown option: $1 (see --help)"
		exit 2
		;;
	*) BIN="$1" ;;
	esac
	shift
done

if [ -z "$BIN" ]; then
	if [ -x ./build/glang ]; then
		BIN=./build/glang
	elif [ -x "$HERE/../build/glang" ]; then
		BIN="$HERE/../build/glang"
	else BIN=./build/glang; fi
fi
[ -x "$BIN" ] || {
	echo "compiler not found or not executable: $BIN"
	exit 2
}
case "$BIN" in /*) ;; *) BIN="$PWD/$BIN" ;; esac

if [ -t 1 ] && [ -z "${NO_COLOR:-}" ]; then
	RED=$'\033[31m'
	GRN=$'\033[32m'
	YEL=$'\033[33m'
	CYN=$'\033[36m'
	DIM=$'\033[2m'
	RST=$'\033[0m'
else
	RED=
	GRN=
	YEL=
	CYN=
	DIM=
	RST=
fi

TMP="$(mktemp -d)"
trap 'rm -rf "$TMP"' EXIT
OUT="$TMP/stdout"
ERRF="$TMP/stderr"
RESULTS="$TMP/results.tsv"
: >"$RESULTS"
FAILURES="$TMP/failures.txt"
: >"$FAILURES"
UPDATED=0
START=$(date +%s)

# ------------------------------------------------------------------ helpers
say() { [ "$QUIET" -eq 1 ] || printf '%s\n' "$*"; }
run_bin() {
	timeout "$TIMEOUT" "$BIN" "$@" >"$OUT" 2>"$ERRF"
	RC=$?
}
first_err_line() { head -n 1 "$ERRF" | cut -c1-160; }

record() { # STATUS CATEGORY NAME [DETAIL]
	local st="$1" cat="$2" name="$3" detail="${4:-}"
	printf '%s\t%s\n' "$cat" "$st" >>"$RESULTS"
	case "$st" in
	PASS) [ "$VERBOSE" -eq 1 ] && say "  ${GRN}PASS${RST}   $cat/$name" ;;
	XFAIL) [ "$VERBOSE" -eq 1 ] && say "  ${YEL}XFAIL${RST}  $cat/$name ${DIM}${detail}${RST}" ;;
	XPASS) say "  ${CYN}XPASS${RST}  $cat/$name  -> now rejected as it should: rename todo_ to err_" ;;
	FAIL)
		say "  ${RED}FAIL${RST}   $cat/$name"
		[ "$QUIET" -eq 1 ] || printf '%s\n' "$detail" | sed 's/^/           /'
		printf '%s/%s: %s\n' "$cat" "$name" "$(printf '%s' "$detail" | head -n 1)" >>"$FAILURES"
		;;
	esac
	return 0
}

extract_tokens() { sed -n '/^Lex : \[$/,/^\]$/p' "$1"; }
extract_ast() { sed -n '/^AST:$/,$p' "$1" | sed -E 's/ \([0-9]+\)$//'; }

check_patterns() { # PATFILE -> 0 ok ; 1 ko (PAT_WHY set)
	PAT_WHY=""
	[ -f "$1" ] || return 0
	local line pat
	while IFS= read -r line || [ -n "$line" ]; do
		[ -z "$line" ] && continue
		case "$line" in '#'*) continue ;; esac
		if [ "${line:0:1}" = "!" ]; then
			pat="${line:1}"
			if grep -Eq -e "$pat" "$ERRF"; then
				PAT_WHY="stderr must NOT match /$pat/"
				return 1
			fi
		elif ! grep -Eq -e "$line" "$ERRF"; then
			PAT_WHY="stderr does not match /$line/"
			return 1
		fi
	done <"$1"
	return 0
}

check_golden() { # GOLDPREFIX -> 0 ok ; 1 ko (GOLD_WHY set). Honors --update.
	GOLD_WHY=""
	local g="$1" ext fn actual rel
	for ext in tokens ast; do
		[ -f "$g.$ext" ] || continue
		if [ "$ext" = tokens ]; then fn=extract_tokens; else fn=extract_ast; fi
		actual="$TMP/golden.actual"
		$fn "$OUT" >"$actual"
		rel="${g#$HERE/}.$ext"
		if [ "$UPDATE" -eq 1 ]; then
			if [ -s "$actual" ]; then
				if ! cmp -s "$actual" "$g.$ext"; then
					cp "$actual" "$g.$ext"
					UPDATED=$((UPDATED + 1))
					say "  ${CYN}updated${RST} $rel"
				fi
			else
				GOLD_WHY="cannot update $rel: the compiler produced no such section (exit $RC: $(first_err_line))"
				return 1
			fi
		elif ! cmp -s "$actual" "$g.$ext"; then
			GOLD_WHY="output differs from $rel  (--update to accept)"$'\n'"$(diff -u "$g.$ext" "$actual" | sed -n '3,14p')"
			return 1
		fi
	done
	return 0
}

# universal checks -> sets CRASH (empty when fine)
universal() {
	CRASH=""
	if [ "$RC" -eq 124 ]; then
		CRASH="TIMEOUT after ${TIMEOUT}s (infinite loop or quadratic blow-up?)"
	elif [ "$RC" -eq 139 ]; then
		CRASH="SEGFAULT (stack overflow or invalid memory access)"
	elif [ "$RC" -eq 134 ]; then
		CRASH="ABORT (assertion or uncaught exception)"
	elif [ "$RC" -ge 128 ]; then
		CRASH="killed by signal $((RC - 128))"
	elif [ "$RC" -gt 1 ]; then CRASH="unexpected exit code $RC (only 0 and 1 are valid)"; fi
	if [ -z "$CRASH" ] && grep -Eq 'AddressSanitizer|LeakSanitizer|runtime error:|terminate called|Assertion .* failed' "$ERRF"; then
		CRASH="sanitizer/abort report: $(grep -Em1 'AddressSanitizer|LeakSanitizer|runtime error:|terminate called|Assertion' "$ERRF" | cut -c1-140)"
	fi
	if [ -z "$CRASH" ] && grep -Eq ':[0-9]{6,}:' "$ERRF"; then
		CRASH="absurd line number in a diagnostic (uninitialised value?): $(first_err_line)"
	fi
	return 0
}

# judge KIND CATEGORY NAME PATFILE GOLDPREFIX   (uses RC/OUT/ERRF of the last run)
judge() {
	local kind="$1" cat="$2" name="$3" patfile="$4" gold="$5" why=""
	PAT_WHY=""
	GOLD_WHY=""
	universal

	if [ "$kind" = todo ]; then
		local ideal="${IDEAL_RC:-1}"
		if [ "$RC" -eq "$ideal" ] && [ -z "$CRASH" ] && check_patterns "$patfile"; then
			if [ "$ideal" -eq 1 ] && [ ! -s "$ERRF" ]; then
				record XFAIL "$cat" "$name" "(rejected but silently)"
				return
			fi
			record XPASS "$cat" "$name"
		elif [ -n "$CRASH" ]; then
			record XFAIL "$cat" "$name" "[$CRASH]"
		else record XFAIL "$cat" "$name" "(exit $RC, expected $ideal${PAT_WHY:+; $PAT_WHY})"; fi
		return
	fi

	if [ -n "$CRASH" ]; then
		record FAIL "$cat" "$name" "$CRASH"
		return
	fi
	case "$kind" in
	ok)
		if [ "$RC" -ne 0 ]; then
			why="expected exit 0, got $RC: $(first_err_line)"
		elif [ -s "$ERRF" ]; then why="exit 0 but stderr is not empty: $(first_err_line)"; fi
		;;
	lex)
		grep -q '^Lex : \[$' "$OUT" || why="lexing failed (exit $RC): $(first_err_line)"
		;;
	parse)
		if ! grep -q '^Lex : \[$' "$OUT"; then
			why="lexing failed (exit $RC): $(first_err_line)"
		elif ! grep -q '^AST:$' "$OUT"; then why="parsing failed (exit $RC): $(first_err_line)"; fi
		;;
	err)
		if [ "$RC" -ne 1 ]; then
			why="expected exit 1 (rejection), got $RC"
		elif [ ! -s "$ERRF" ]; then
			why="rejected silently: exit 1 but nothing on stderr"
		elif ! check_patterns "$patfile"; then why="$PAT_WHY"$'\n'"stderr was: $(head -c 300 "$ERRF" | tr '\n' ' ')"; fi
		;;
	*) why="unknown test kind '$kind' (name must start with ok_ err_ lex_ parse_ or todo_)" ;;
	esac
	if [ -z "$why" ] && [ -n "$gold" ]; then check_golden "$gold" || why="$GOLD_WHY"; fi
	if [ -n "$why" ]; then record FAIL "$cat" "$name" "$why"; else record PASS "$cat" "$name"; fi
}

want() { [ -z "$FILTER" ] || [[ "$1" == *"$FILTER"* ]]; }

# dyn KIND CATEGORY NAME 'pattern lines' ARGS...   -> generated test
dyn() {
	local kind="$1" cat="$2" name="$3" pats="$4"
	shift 4
	want "$cat/$name" || return 0
	printf '%s' "$pats" >"$TMP/dyn.stderr"
	run_bin "$@"
	judge "$kind" "$cat" "$name" "$TMP/dyn.stderr" ""
}

# ------------------------------------------------------------------ 1. static cases
run_static() {
	local entries=() p rel cat name kind patfile gold f
	local args=()
	while IFS= read -r p; do entries+=("$p"); done < <(
		find "$CASES" -mindepth 1 \( -type d \( -name 'ok_*' -o -name 'err_*' -o -name 'todo_*' -o -name 'lex_*' -o -name 'parse_*' \) -prune -print \) \
			-o \( -type f -name '*.gg' -print \) | sort
	)
	for p in "${entries[@]}"; do
		rel="${p#$CASES/}"
		want "$rel" || continue
		cat="$(dirname "$rel")"
		if [ -d "$p" ]; then
			name="$(basename "$p")"
			patfile="$p/expected.stderr"
			gold="$p/expected"
			args=()
			while IFS= read -r f; do args+=("$f"); done < <(printf '%s\n' "$p"/*.gg | sort)
		else
			name="$(basename "$p" .gg)"
			patfile="${p%.gg}.stderr"
			gold="${p%.gg}"
			args=("$p")
		fi
		kind="${name%%_*}"
		if [ "$LIST" -eq 1 ]; then
			echo "$rel"
			continue
		fi
		run_bin "${args[@]}"
		judge "$kind" "$cat" "$name" "$patfile" "$gold"
	done
}

# ------------------------------------------------------------------ 2. stress (generated)
S="$TMP/stress"
mkdir -p "$S"
rep() { awk -v s="$1" -v n="$2" 'BEGIN{ for(i=0;i<n;i++) printf "%s", s }'; }

run_stress() {
	say "${DIM}-- stress (generated inputs)${RST}"
	local f n

	# nesting: legal depth must work, absurd depth must be a clean error (never a segfault)
	f="$S/a.gg"
	{
		printf 'fun f() i32 { return ('
		rep '(' 200
		printf '1'
		rep ')' 200
		printf '); }\n'
	} >"$f"
	dyn ok stress ok_parens_nesting_200 '' "$f"
	f="$S/b.gg"
	{
		printf 'fun f() i32 { return ('
		rep '(' 5000
		printf '1'
		rep ')' 5000
		printf '); }\n'
	} >"$f"
	dyn err stress err_parens_nesting_5000 'nesting too deep' "$f"
	f="$S/c.gg"
	{
		printf 'fun f() {'
		rep ' {' 300
		rep ' }' 300
		printf ' }\n'
	} >"$f"
	dyn ok stress ok_blocks_nesting_300 '' "$f"
	f="$S/d.gg"
	{
		printf 'fun f() {'
		rep ' {' 1000
		rep ' }' 1000
		printf ' }\n'
	} >"$f"
	dyn err stress err_blocks_nesting_1000 'nesting too deep' "$f"
	f="$S/e.gg"
	{
		printf 'fun f(i32 a) { if (a) { }'
		rep ' else if (a) { }' 300
		printf ' }\n'
	} >"$f"
	dyn ok stress ok_else_if_chain_300 '' "$f"
	f="$S/f.gg"
	{
		printf 'fun f(i32 a) { if (a) { }'
		rep ' else if (a) { }' 3000
		printf ' }\n'
	} >"$f"
	dyn err stress err_else_if_chain_3000 'nesting too deep' "$f"
	f="$S/g.gg"
	{
		printf 'fun f(i32 a) i32 { return '
		rep '!' 200
		printf 'a; }\n'
	} >"$f"
	dyn ok stress ok_unary_chain_200 '' "$f"
	f="$S/h.gg"
	{
		printf 'fun f(i32 a) i32 { return '
		rep '!' 2000
		printf 'a; }\n'
	} >"$f"
	dyn err stress err_unary_chain_2000 'nesting too deep' "$f"
	f="$S/i.gg"
	{
		printf 'fun f(i32 a) { a'
		rep '=a' 200
		printf '; }\n'
	} >"$f"
	dyn ok stress ok_assignment_chain_200 '' "$f"
	f="$S/j.gg"
	{
		printf 'fun f(i32 a) { a'
		rep '=a' 2000
		printf '; }\n'
	} >"$f"
	dyn err stress err_assignment_chain_2000 'nesting too deep' "$f"

	# size: must stay fast (a hang or quadratic behaviour hits the timeout)
	f="$S/k.gg"
	{
		printf 'fun f() i32 { return (1'
		rep '+1' 20000
		printf '); }\n'
	} >"$f"
	dyn ok stress ok_long_sum_20000_terms '' "$f"
	f="$S/l.gg"
	{
		printf 'fun f(i32 a) i32 { return (a'
		rep '||a' 20000
		printf '); }\n'
	} >"$f"
	dyn ok stress ok_long_or_chain_20000 '' "$f"
	f="$S/m.gg"
	{
		printf 'fun f() { g(1'
		rep ',1' 5000
		printf '); }\n'
	} >"$f"
	dyn parse stress parse_call_with_5000_arguments '' "$f"
	f="$S/n.gg"
	{
		printf 'fun f(i32 a0'
		for n in $(seq 1 1999); do printf ',i32 a%d' "$n"; done
		printf ') { }\n'
	} >"$f"
	dyn ok stress ok_function_with_2000_parameters '' "$f"
	f="$S/o.gg"
	{
		printf 'struct S {'
		for n in $(seq 1 5000); do printf ' i32 f%d;' "$n"; done
		printf ' }\n'
	} >"$f"
	dyn ok stress ok_struct_with_5000_fields '' "$f"
	f="$S/p.gg"
	{ for n in $(seq 1 20000); do printf 'fun f%d() { }\n' "$n"; done; } >"$f"
	dyn ok stress ok_20000_functions '' "$f"
	f="$S/q.gg"
	{
		printf 'fun f() { i32 a = 0;'
		rep ' a = a + 1;' 50000
		printf ' }\n'
	} >"$f"
	dyn ok stress ok_50000_statements_in_one_function '' "$f"
	f="$S/r.gg"
	{
		printf 'fun f() {'
		for n in $(seq 1 5000); do printf ' i32 v%d = %d;' "$n" "$n"; done
		printf ' }\n'
	} >"$f"
	dyn ok stress ok_5000_local_variables '' "$f"
	f="$S/s.gg"
	{
		printf 'fun f() { i32 '
		rep x 100000
		printf ' = 1; }\n'
	} >"$f"
	dyn ok stress ok_identifier_of_100000_chars '' "$f"
	f="$S/t.gg"
	{
		printf 'fun f() { string s = "'
		rep a 1000000
		printf '"; }\n'
	} >"$f"
	dyn ok stress ok_string_literal_of_1MB '' "$f"
	f="$S/u.gg"
	{
		printf '// '
		rep x 1000000
		printf '\nfun f() { }\n'
	} >"$f"
	dyn ok stress ok_line_comment_of_1MB '' "$f"
	f="$S/v.gg"
	{
		printf '/* '
		rep x 1000000
		printf ' */\nfun f() { }\n'
	} >"$f"
	dyn ok stress ok_block_comment_of_1MB '' "$f"
	f="$S/w.gg"
	{
		printf 'fun f() { }\n'
		rep '\n' 200000
	} >"$f"
	dyn ok stress ok_200000_blank_lines '' "$f"
	f="$S/x.gg"
	{
		printf 'fun f() { '
		rep '/* c */ ' 100000
		printf '}\n'
	} >"$f"
	dyn ok stress ok_100000_comments '' "$f"

	# KNOWN LIMIT: a left-nested AST that deep overflows the stack when destroyed/printed
	f="$S/y.gg"
	{
		printf 'fun f() i32 { return (1'
		rep '+1' 400000
		printf '); }\n'
	} >"$f"
	IDEAL_RC=0 dyn todo stress todo_long_sum_400000_terms_no_crash '' "$f"
}

# ------------------------------------------------------------------ 3. command line
cli() { # NAME EXPECTED_RC 'stdout pattern or -' 'stderr pattern or -' ARGS...
	local name="$1" erc="$2" opat="$3" epat="$4"
	shift 4
	want "cli/$name" || return 0
	run_bin "$@"
	universal
	local why=""
	if [ -n "$CRASH" ]; then
		why="$CRASH"
	elif [ "$RC" -ne "$erc" ]; then
		why="expected exit $erc, got $RC: $(first_err_line)"
	elif [ "$opat" != "-" ] && ! grep -Eq -e "$opat" "$OUT"; then
		why="stdout does not match /$opat/"
	elif [ "$epat" != "-" ] && ! grep -Eq -e "$epat" "$ERRF"; then why="stderr does not match /$epat/ ; got: $(first_err_line)"; fi
	if [ -n "$why" ]; then record FAIL cli "$name" "$why"; else record PASS cli "$name"; fi
}

run_cli() {
	say "${DIM}-- command line${RST}"
	local d="$TMP/cli" files=() n
	mkdir -p "$d/dir with spaces" "$d/sub" "$d/many"
	local ok='fun main() i32 { return (0); }
'
	printf '%s' "$ok" >"$d/ok.gg"
	cli no_arguments_print_usage 0 'Usage' -
	cli missing_file 1 - "Cannot read '.*nope\.gg'" "$d/nope.gg"
	cli missing_file_after_a_valid_one 1 - "Cannot read '.*nope\.gg'" "$d/ok.gg" "$d/nope.gg"
	cli directory_as_input 1 - 'Cannot read' "$d/sub"
	cli empty_string_argument 1 - 'Cannot read' ""
	cli option_like_argument_is_a_filename 1 - "Cannot read '--help'" --help
	printf '%s' "$ok" >"$d/dir with spaces/prog.gg"
	cli path_with_spaces 0 - - "$d/dir with spaces/prog.gg"
	printf '%s' "$ok" >"$d/caf$(printf '\303\251').gg"
	cli utf8_file_name 0 - - "$d/caf$(printf '\303\251').gg"
	if ln -s "$d/ok.gg" "$d/link.gg" 2>/dev/null; then cli symlink_to_source 0 - - "$d/link.gg"; fi
	: >"$d/empty.gg"
	cli empty_file 0 - - "$d/empty.gg"

	if want "cli/relative_dot_slash_path"; then
		(cd "$d" && run_bin ./ok.gg && [ "$RC" -eq 0 ]) && record PASS cli relative_dot_slash_path ||
			record FAIL cli relative_dot_slash_path "a relative path './ok.gg' was not accepted"
	fi
	if want "cli/same_file_twice_equals_once"; then
		run_bin "$d/ok.gg"
		cp "$OUT" "$TMP/once.out"
		local rc1=$RC
		run_bin "$d/ok.gg" "$d/ok.gg"
		if [ "$rc1" -eq "$RC" ] && cmp -s "$TMP/once.out" "$OUT"; then
			record PASS cli same_file_twice_equals_once
		else record FAIL cli same_file_twice_equals_once "output/exit code differ when the same file is given twice"; fi
	fi
	if want "cli/errors_go_to_stderr_not_stdout"; then
		printf 'fun f() {\n  i32 a = ;\n}\n' >"$d/bad.gg"
		run_bin "$d/bad.gg"
		if [ "$RC" -eq 1 ] && ! grep -qi 'error' "$OUT" && grep -q 'Parse error' "$ERRF"; then
			record PASS cli errors_go_to_stderr_not_stdout
		else record FAIL cli errors_go_to_stderr_not_stdout "diagnostics must be on stderr only (exit $RC)"; fi
	fi
	if want "cli/output_is_deterministic"; then
		run_bin "$d/ok.gg"
		cp "$OUT" "$TMP/run1.out"
		run_bin "$d/ok.gg"
		if cmp -s "$TMP/run1.out" "$OUT"; then
			record PASS cli output_is_deterministic
		else record FAIL cli output_is_deterministic "two identical runs printed different output"; fi
	fi

	for n in $(seq 1 300); do
		printf 'fun f%d() { }\n' "$n" >"$d/many/f$n.gg"
		files+=("$d/many/f$n.gg")
	done
	cli three_hundred_files 0 - - "${files[@]}"
	printf 'fun g() {\n  @\n}\n' >"$d/many/zz_bad.gg"
	cli one_bad_file_among_three_hundred 1 - 'Not recognized character' "${files[@]}" "$d/many/zz_bad.gg"

	if [ "$(id -u)" -ne 0 ]; then
		printf '%s' "$ok" >"$d/noperm.gg"
		chmod 000 "$d/noperm.gg"
		cli unreadable_file 1 - 'Cannot read' "$d/noperm.gg"
		chmod 644 "$d/noperm.gg"
	fi

	# not source code at all: must be rejected cleanly, never crash
	head -c 4096 /dev/zero | tr '\0' '\377' >"$d/ff.bin"
	head -c 4096 /dev/zero >"$d/nul.bin"
	{ for n in $(seq 1 255); do printf "\\$(printf '%03o' "$n")"; done; } >"$d/allbytes.bin"
	dyn err cli err_binary_file_all_0xff 'Not recognized character' "$d/ff.bin"
	dyn err cli err_binary_file_all_nul 'Not recognized character' "$d/nul.bin"
	dyn err cli err_binary_file_every_byte 'Not recognized character' "$d/allbytes.bin"
}

# ------------------------------------------------------------------ 4. fuzz (deterministic)
BYTES=(3b 7b 7d 28 29 5b 5d 22 27 2f 2a 5c 2c 2e 3d 21 3c 3e 26 7c 2b 2d 25 40 23 30 39 61 5a 5f 0a 09 20 00 ff c3 0d)

fuzz_one() { # label file -> 0 ok, 1 = problem (saves the reproducer)
	local label="$1" f="$2"
	run_bin "$f"
	universal
	if [ -n "$CRASH" ]; then
		mkdir -p "$FUZZ_FAIL_DIR"
		local keep="$FUZZ_FAIL_DIR/$(printf '%s' "$label" | tr '/: ' '___').gg"
		cp "$f" "$keep"
		FUZZ_WHY="$CRASH -> reproducer: $keep"
		return 1
	fi
	return 0
}

run_fuzz() {
	[ "$FUZZ_LEVEL" -gt 0 ] || return 0
	say "${DIM}-- fuzz (level $FUZZ_LEVEL: every input must exit 0 or 1, never crash or hang)${RST}"
	local seeds=() s size idx=0 pos k kind hex name fails first step
	local fz="$TMP/fuzz.gg" trunc_max=80 mut_n=80
	if [ "$FUZZ_LEVEL" -ge 2 ]; then
		trunc_max=3000
		mut_n=1000
	fi
	if [ -n "${FUZZ_SEEDS:-}" ]; then
		read -r -a seeds <<<"$FUZZ_SEEDS"
	else
		for s in parser/parse_many_declarations_mixed parser/parse_language_idioms parser/parse_var_decls parser/parse_prec_mixed \
			parser/parse_fun_signatures lexer/lex_all_keywords lexer/lex_string_literals lexer/lex_block_comments \
			semantic/ok_struct_self_pointer; do
			[ -f "$CASES/$s.gg" ] && seeds+=("$CASES/$s.gg")
		done
		for s in "$HERE"/../files_to_compile/*.gg; do [ -f "$s" ] && seeds+=("$s"); done
	fi

	for s in "${seeds[@]}"; do
		idx=$((idx + 1))
		name="$(basename "$s" .gg)"
		size=$(wc -c <"$s")
		[ "$size" -gt 0 ] || continue

		if want "fuzz/truncate:$name"; then
			# cut the file at (up to trunc_max) evenly spaced offsets, from 0 bytes to the whole file
			fails=0
			first=""
			step=$((size / trunc_max))
			[ "$step" -ge 1 ] || step=1
			pos=0
			while [ "$pos" -le "$size" ]; do
				head -c "$pos" "$s" >"$fz"
				if ! fuzz_one "truncate_${name}_at_$pos" "$fz"; then
					fails=$((fails + 1))
					[ -n "$first" ] || first="cut at byte $pos: $FUZZ_WHY"
				fi
				pos=$((pos + step))
			done
			if [ "$fails" -eq 0 ]; then record PASS fuzz "truncate:$name"; else record FAIL fuzz "truncate:$name" "$fails problem(s). First: $first"; fi
		fi

		if want "fuzz/mutate:$name"; then
			# random single-byte mutations (replace / delete / insert), reproducible seed
			RANDOM=$((4242 + idx))
			fails=0
			first=""
			for k in $(seq 1 "$mut_n"); do
				pos=$((((RANDOM << 15) | RANDOM) % size))
				hex="${BYTES[$((RANDOM % ${#BYTES[@]}))]}"
				case $((RANDOM % 3)) in
				0)
					kind=replace
					{
						head -c "$pos" "$s"
						printf "\\x$hex"
						tail -c +$((pos + 2)) "$s"
					} >"$fz"
					;;
				1)
					kind=delete
					{
						head -c "$pos" "$s"
						tail -c +$((pos + 2)) "$s"
					} >"$fz"
					;;
				*)
					kind=insert
					{
						head -c "$pos" "$s"
						printf "\\x$hex"
						tail -c +$((pos + 1)) "$s"
					} >"$fz"
					;;
				esac
				if ! fuzz_one "mutate_${name}_${kind}_${hex}_at_$pos" "$fz"; then
					fails=$((fails + 1))
					[ -n "$first" ] || first="$kind 0x$hex at byte $pos: $FUZZ_WHY"
				fi
			done
			if [ "$fails" -eq 0 ]; then record PASS fuzz "mutate:$name"; else record FAIL fuzz "mutate:$name" "$fails problem(s). First: $first"; fi
		fi
	done
}

# ------------------------------------------------------------------ main
if [ "$LIST" -eq 1 ]; then
	run_static
	exit 0
fi

[ "$RUN_STATIC" -eq 1 ] && {
	say "${DIM}-- static cases (tests/cases)${RST}"
	run_static
}
[ "$RUN_STRESS" -eq 1 ] && run_stress
[ "$RUN_CLI" -eq 1 ] && run_cli
[ "$RUN_FUZZ" -eq 1 ] && run_fuzz

# ------------------------------------------------------------------ summary
END=$(date +%s)
echo
printf '%-12s %10s %8s %8s %8s\n' category passed failed xfail xpass
awk -F'\t' '
  { t[$1]++; c[$1,$2]++; cats[$1]=1 }
  END { for (k in cats) printf "%-12s %6d/%-3d %8d %8d %8d\n", k, c[k,"PASS"], t[k]-c[k,"XFAIL"]-c[k,"XPASS"], c[k,"FAIL"], c[k,"XFAIL"], c[k,"XPASS"] }
' "$RESULTS" | sort

read -r PASS FAIL XFAIL XPASS < <(awk -F'\t' '{ n[$2]++ } END { print n["PASS"]+0, n["FAIL"]+0, n["XFAIL"]+0, n["XPASS"]+0 }' "$RESULTS")
echo
printf 'passed=%s  failed=%s  xfail=%s (known gaps)  xpass=%s   [%ss]' "$PASS" "$FAIL" "$XFAIL" "$XPASS" "$((END - START))"
[ "$UPDATED" -gt 0 ] && printf '  updated-goldens=%s' "$UPDATED"
echo
if [ "$FAIL" -gt 0 ]; then
	echo
	echo "${RED}FAILED:${RST}"
	sed 's/^/  /' "$FAILURES"
	exit 1
fi
exit 0
