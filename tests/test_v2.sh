#!/bin/sh

# Locate the project relative to this script, not the current directory.
script_dir=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd) || exit 1
project_dir=$(CDPATH= cd -- "$script_dir/.." && pwd) || exit 1

# Register cleanup only after creating our temporary directory successfully.
test_dir=$(mktemp -d "${TMPDIR:-/tmp}/tinyworld-v2.XXXXXX") || exit 1
trap 'rm -rf -- "$test_dir"' 0
trap 'exit 129' HUP
trap 'exit 130' INT
trap 'exit 143' TERM

gcc -std=c11 -Wall -Wextra -Wpedantic \
    "$project_dir/causal_v2.c" -o "$test_dir/causal_v2"
compile_status=$?
if [ "$compile_status" -ne 0 ]; then
    printf 'FAIL: compilation (exit status %s); simulations skipped.\n' "$compile_status" >&2
    exit "$compile_status"
fi
printf 'PASS: compilation\n'

failures=0

# Check successful runs and retain output for the default-mode comparison.
for mode in 0 1 2; do
    "$test_dir/causal_v2" "$mode" > "$test_dir/mode_$mode.out" 2> "$test_dir/mode_$mode.err"
    run_status=$?
    if [ "$run_status" -eq 0 ] && [ ! -s "$test_dir/mode_$mode.err" ]; then
        printf 'PASS: mode %s\n' "$mode"
    else
        printf 'FAIL: mode %s (exit status %s or unexpected stderr)\n' "$mode" "$run_status" >&2
        failures=$((failures + 1))
    fi
done

"$test_dir/causal_v2" > "$test_dir/default.out" 2> "$test_dir/default.err"
run_status=$?
if [ "$run_status" -eq 0 ] && [ ! -s "$test_dir/default.err" ] &&
    cmp -s "$test_dir/default.out" "$test_dir/mode_2.out"; then
    printf 'PASS: default output matches mode 2\n'
else
    printf 'FAIL: default run or output comparison\n' >&2
    failures=$((failures + 1))
fi

# Invalid inputs must fail, leave stdout empty, and report usage on stderr.
# "$@" forwards each argument separately, including an empty string.
check_rejected() {
    "$test_dir/causal_v2" "$@" > "$test_dir/invalid.out" 2> "$test_dir/invalid.err"
    run_status=$?
    if [ "$run_status" -ne 0 ] && [ ! -s "$test_dir/invalid.out" ] &&
        grep -Fxq 'Usage: causal_v2 [0|1|2]' "$test_dir/invalid.err"; then
        printf 'PASS: rejected'
        printf ' [%s]' "$@"
        printf '\n'
    else
        printf 'FAIL: expected rejection of' >&2
        printf ' [%s]' "$@" >&2
        printf '\n' >&2
        failures=$((failures + 1))
    fi
}

for argument in 3 -1 abc 01 1abc ''; do
    check_rejected "$argument"
done
check_rejected 0 1

if [ "$failures" -ne 0 ]; then
    printf 'FAIL: %s verification(s) failed.\n' "$failures" >&2
    exit 1
fi
printf 'PASS: all TinyWorld V2 checks passed.\n'
exit 0
