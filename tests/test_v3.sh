#!/bin/sh

# Use a consistent locale for numeric output and parsing.
LC_ALL=C
export LC_ALL

# Locate the script directory independently of the current directory.
script_dir=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd) || exit 1

# The project root is the parent of the tests directory.
project_dir=$(CDPATH= cd -- "$script_dir/.." && pwd) || exit 1

# Create a temporary directory for the executable and test outputs.
test_dir=$(mktemp -d "${TMPDIR:-/tmp}/tinyworld-v3.XXXXXX") || exit 1

# Remove our temporary directory when the script exits.
trap 'rm -rf -- "$test_dir"' 0

# Exit on interruption; the exit trap performs cleanup.
trap 'exit 129' HUP
trap 'exit 130' INT
trap 'exit 143' TERM

# Compile V3 without replacing the local executable.
gcc -std=c11 -Wall -Wextra -Wpedantic \
    "$project_dir/causal_v3.c" -o "$test_dir/causal_v3"
compile_status=$?

# Stop before running tests if compilation fails.
if [ "$compile_status" -ne 0 ]; then
    printf 'FAIL: compilation (exit status %s); tests skipped.\n' \
        "$compile_status" >&2
    exit 1
fi

printf 'PASS: compilation\n'

failures=0

# Run the same executable twice and capture each output separately.
for run in 1 2; do
    "$test_dir/causal_v3" \
        > "$test_dir/run_$run.out" \
        2> "$test_dir/run_$run.err"
    run_status=$?

    if [ "$run_status" -eq 0 ] &&
        [ ! -s "$test_dir/run_$run.err" ]; then
        printf 'PASS: run %s\n' "$run"
    else
        printf 'FAIL: run %s (exit status %s or unexpected stderr)\n' \
            "$run" "$run_status" >&2
        failures=$((failures + 1))
    fi
done

# Compare both outputs to check local reproducibility.
if cmp -s "$test_dir/run_1.out" "$test_dir/run_2.out"; then
    printf 'PASS: repeated outputs match\n'
else
    printf 'FAIL: repeated outputs differ\n' >&2
    failures=$((failures + 1))
fi

# Validate section ownership, score counts, and complete decimal values.
if awk '
    function valid_score(value) {
        # Check the entire string before performing numeric conversion.
        if (value !~ /^[0-9]+\.[0-9]+$/) {
            return 0
        }
        return (value + 0 >= 0 && value + 0 <= 1)
    }

    $0 == "Observational evaluation" {
        section = 1
        headings[section]++
        next
    }
    $0 == "Intervention evaluation: do(A=0)" {
        section = 2
        headings[section]++
        next
    }
    $0 == "Intervention evaluation: do(A=1)" {
        section = 3
        headings[section]++
        next
    }

    /^Learned predictor Brier score: / {
        value = $0
        sub(/^Learned predictor Brier score: /, "", value)
        if (!valid_score(value)) {
            invalid = 1
        }
        if (section == 0) {
            invalid = 1
        } else {
            learned[section]++
        }
        next
    }
    /^Constant baseline Brier score: / {
        value = $0
        sub(/^Constant baseline Brier score: /, "", value)
        if (!valid_score(value)) {
            invalid = 1
        } else if (value + 0 != 0.25) {
            invalid = 1
        }
        if (section == 0) {
            invalid = 1
        } else {
            baseline[section]++
        }
        next
    }

    # Reject malformed evaluation headings or score labels as well.
    /^(Observational evaluation|Intervention evaluation|Learned predictor|Constant baseline)/ || /Brier score/ {
        invalid = 1
        section = 0
    }

    END {
        for (s = 1; s <= 3; s++) {
            if (headings[s] != 1 ||
                learned[s] != 1 ||
                baseline[s] != 1) {
                invalid = 1
            }
        }
        exit (invalid ? 1 : 0)
    }
' "$test_dir/run_1.out"; then
    printf 'PASS: unique sections, score counts, decimal ranges, and constant references\n'
else
    printf 'FAIL: missing, duplicate, misplaced, or malformed section/score; invalid range or constant reference\n' >&2
    failures=$((failures + 1))
fi

if [ "$failures" -ne 0 ]; then
    printf 'FAIL: %s verification(s) failed.\n' "$failures" >&2
    exit 1
fi
printf 'PASS: all TinyWorld V3 checks passed.\n'
exit 0
