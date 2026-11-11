# TinyWorld

**An artificial world where I know the real cause-effect rules, built to test whether a model actually learns them or just picks up on correlations.**

Personal project, no deadline, growing alongside my Computer Science studies at UNamur.

---

## Why this project

In real-world ML, you almost never know the true process behind your data — you have a dataset and you have to guess. Here it's the opposite: I define the causal rules myself, which lets me check objectively whether a model actually recovers them, or gets fooled by a correlation that only looks real in the training data.

This is directly useful for a Risk Data Science path: a factor that looks linked to risk isn't always the actual cause.

## Research question

> Does a model trained on data where two variables are correlated through a hidden confounder keep making correct predictions once that correlation is broken by intervention?

## Constraints

- Solo project, no deadline
- Starts at a first-year level (C, basic probability)
- Built on and reinforces my coursework (C, algorithms, probability/statistics)
- Meant to evolve gradually toward ML, then deep learning
- Every step needs a checkable success criterion, not just "a decent score"

---

## Progress

### V1 — Static controlled world (C + stats) — done

**V1.a — Minimal causal chain**
C generator: `A` (Bernoulli, fixed p) → `B`, `C` (conditional probabilities I set myself). Exported to CSV, verified in Python that the empirical conditional probabilities matched the theoretical ones.

**V1.b — Hidden confounder**
Added a hidden variable `Z` influencing both `A` and `C`, creating a fake correlation between `A` and `C` with no real causal link. Generated 10,000 observations.

Result: P(C=1 | A=1) ≈ 67-68% vs P(C=1 | A=0) ≈ 31-32% — a strong-looking correlation, entirely driven by `Z`, not by `A`. Confirmed with a reference calculation in Python.

### V2 — Controlled causal interventions

Implemented in `causal_v2.c`, separately from V1.b. All variables are binary.
The causal graph is `Z → A`, `Z → C`, `A → B`. Z is generated internally
and hidden from the displayed observations.

The model uses the following probability parameters:

| Parameter | Value |
|---|---:|
| P(Z=1) | 50% |
| P(A=1 given Z=1) | 80% |
| P(A=1 given Z=0) | 20% |
| P(B=1 given A=1) | 90% |
| P(B=1 given A=0) | 10% |
| P(C=1 given Z=1) | 80% |
| P(C=1 given Z=0) | 20% |

Select the experiment mode with an optional command-line argument:

| Mode | Behavior |
|---|---|
| 0 | Observation: generate A according to Z. |
| 1 | do(A=0): impose A=0 independently of Z. |
| 2 | do(A=1): impose A=1 independently of Z. |

An intervention replaces the usual mechanism generating A. The mechanisms
for B and C remain unchanged: B depends on A, and C depends on Z.
Conditioning on A instead selects a subgroup of observational data without
changing any generating mechanism.

Compile once:

```bash
gcc -std=c11 -Wall -Wextra -Wpedantic causal_v2.c -o causal_v2
```

Run the desired mode without changing the source or recompiling:

```bash
./causal_v2 0
./causal_v2 1
./causal_v2 2
```

Running `./causal_v2` without an argument uses mode 2, do(A=1).
Only the exact argument strings `0`, `1` and `2` are accepted.
Invalid arguments (including `01`, `1abc` and an empty string) or extra
arguments are rejected before simulation with `Usage: causal_v2 [0|1|2]`
on `stderr` and a nonzero exit status (`EXIT_FAILURE`).

The program reports marginal frequencies for A, B and C, and conditional
frequencies for B and C given A. It displays `N/A` when the conditioning
group contains no observations, because that frequency cannot be calculated.

The following results were obtained with 10,000 observations per mode and
seed 42 (`srand(42)`) in the tested macOS environment. These are empirical
simulation frequencies, rounded to one decimal place, not exact probabilities.

| Mode | A=1 | B=1 | C=1 | C=1 given A=0 | C=1 given A=1 |
|---|---:|---:|---:|---:|---:|
| Observation | 50.7% | 50.5% | 50.6% | 32.1% | 68.6% |
| do(A=0) | 0.0% | 10.1% | 50.8% | 50.8% | N/A |
| do(A=1) | 100.0% | 89.8% | 50.8% | N/A | 50.8% |

In the defined model, A and C share the cause Z, but A does not cause C.
Their theoretical observational conditional probabilities are 32% and 68%,
whereas P(C=1) under either intervention is 50%. The measured frequencies
are consistent with this model; they do not constitute exhaustive validation
of the implementation. For B, the theoretical conditional probabilities
in observation coincide with the corresponding intervention probabilities
(10% and 90%) in this graph.

All three modes were compiled without warnings using the command above.
Repeated runs of each mode produced identical output in the tested environment.
A fixed seed does not guarantee identical results across C library
implementations, because `rand()` sequences can differ. The expression
`rand() % 100` can introduce modulo bias, and finite samples fluctuate around
the intended probabilities. Interventions also omit the random draw for A,
so observation and intervention use subsequent random draws differently.

#### Built-in validation

Command-line validation rejects unsupported arguments before simulation,
as described above. An additional internal check rejects numeric experiment
modes outside 0, 1 and 2, reports the invalid value on `stderr`, and returns
`EXIT_FAILURE`.

After simulation, deterministic checks require:

- `count_a0 + count_a1 == num_observations` in every mode.
- `count_a1 == 0` and `count_a0 == num_observations` under do(A=0).
- `count_a1 == num_observations` and `count_a0 == 0` under do(A=1).

For experiments with at least 10,000 observations, additional checks compare
unrounded marginal frequencies with inclusive ranges allowing a deviation
of two percentage points from the expected values:

| Mode | A=1 | B=1 | C=1 |
|---|---|---|---|
| Observation | 50% expected; 48–52% accepted | 50% expected; 48–52% accepted | 50% expected; 48–52% accepted |
| do(A=0) | Exactly 0%, checked deterministically | 10% expected; 8–12% accepted | 50% expected; 48–52% accepted |
| do(A=1) | Exactly 100%, checked deterministically | 90% expected; 88–92% accepted | 50% expected; 48–52% accepted |

A failed check reports an explicit message on `stderr` and returns
`EXIT_FAILURE`. For fewer than 10,000 observations, frequency checks are
skipped with the message `Statistical checks skipped: fewer than 10000 observations.`
Deterministic checks remain active. Conditional frequencies are displayed
but are not covered by these frequency checks.

#### Verification performed

The following scenarios were compiled and executed on temporary source copies
with C11 and `-Wall -Wextra -Wpedantic`, without compilation warnings:

- Each mode with 10,000 observations and seed 42 returned status 0 and the
  empirical frequencies reported above. Two executions of each mode produced
  identical output and exit status.
- Each mode with 10 observations reported that frequency checks were skipped
  and returned status 0.
- Mode 3 was rejected before simulation with `Invalid experiment mode: 3.`
  and exit status 1.
- In a temporary mode 2 copy with 10,000 observations, replacing the C
  generation mechanism with `c = 0` triggered
  `Validation failed: C=1 frequency is 0.0000%; expected 48% to 52%.`
  and exit status 1.

Earlier temporary counter injections also exercised each intervention check's
two conditions separately. Temporary incorrect frequency expectations for A,
B and C triggered the corresponding failures. No injected fault remains in
the project source.

The validation checks are integrated into the simulator. The temporary-copy
scenarios above document verification work performed during development;
they are distinct from the reusable script described below.

#### Reusable verification script

From the repository root, run:

```sh
sh tests/test_v2.sh
```

The script locates the project relative to its own location, so it can also
be launched from another directory using its absolute path. It requires a
C11 compiler available as `gcc`, `/bin/sh`, and the standard tools `dirname`,
`mktemp`, `cmp`, `grep` and `rm` (available on the tested macOS environment).

It compiles with `-std=c11 -Wall -Wextra -Wpedantic` into a unique temporary
directory, leaving the local `causal_v2` executable untouched. Captured outputs
are stored there too. Cleanup is registered only after successful directory
creation and runs on exit, including compilation or verification failure;
HUP, INT and TERM are handled as well.

The script checks:

- Modes 0, 1 and 2 return status 0 with empty `stderr`.
- The run without arguments succeeds with empty `stderr` and produces the
  same output as mode 2.
- Arguments `3`, `-1`, `abc`, `01`, `1abc`, an empty string, and extra
  arguments `0 1` return nonzero status, leave `stdout` empty, and print
  the exact usage line on `stderr`.

Each verification reports `PASS` or `FAIL`. The script returns 0 when all
checks pass and 1 if a verification fails. If compilation fails, its status
is captured immediately and propagated without running simulations.
Setup failures also return nonzero status. No exact `rand()` counts are
hard-coded in the script.

During development, the script was run successfully from the repository root
and from another directory. Separately, a temporary project copy containing
an intentional C compilation error confirmed a nonzero compiler status was
propagated, simulations were skipped, and the script's temporary directory
was removed. Cleanup was also checked after successful runs. That compilation
fault experiment is not performed by the reusable script itself.

Simulation checks rely on the validation built into `causal_v2.c`; this is
not exhaustive, independent validation of the causal implementation. Output
comparison verifies the default-mode behavior locally, without requiring
identical random sequences across C library implementations.

These frequency ranges are practical, non-exhaustive consistency checks,
not formal statistical tests. A correct simulation can occasionally fall
outside a range, and passing checks cannot exclude every implementation error.
Repeating one fixed seed checks local reproducibility, not behavior across
many seeds or C library implementations. The `rand()` limitations described
above still apply. These results do not validate any AI agent.

### V3 — Observational probability learning — in progress

`causal_v3.c` estimates P(C=1 given A=0) and P(C=1 given A=1) from four
observational counters, using 10,000 samples and training seed 42. In the
tested local environment, the estimates were p0=0.321298 and p1=0.685602
(rounded); repeated executions produced identical output.

The program checks group totals, joint-count bounds and nonempty groups
before division. Three negative scenarios (incorrect total, out-of-bounds
joint count and empty group) were checked on temporary source copies. They
are not yet a reusable V3 test suite.

See [the V3 experiment document](docs/v3_experiment.md) for the four measured
counts, verification results and the remaining protocol.

The training estimates remain fixed during evaluation on 10,000 new
observational samples generated with seed 43. In the tested environment,
the learned predictor's Brier score was 0.216343, versus 0.250000 for the
constant 0.5 reference (displayed to six decimal places). Both predictors
use the same evaluation samples; two executions produced identical output.
A one-off check on a temporary source copy with `prediction = 0.5` confirmed
both scores were exactly 0.25 before display rounding, with exit status 0.
This check is not yet part of a reusable V3 verification suite.

Evaluation under do(A=0) uses seed 44 and 10,000 samples, with A fixed to zero
without a random draw. The learned probabilities remain frozen; B and C keep
their mechanisms. The measured Brier scores were 0.281148 for the learned
predictor and 0.250000 for the constant reference. Training and observational
results remained unchanged. Two executions produced identical output locally.
A one-off temporary-copy check with prediction fixed to 0.5 during this
intervention confirmed both scores equal to 0.25 before rounding, with status 0.
This check is not part of a reusable V3 suite.

Evaluation under do(A=1) uses seed 45 and 10,000 examples. A is fixed to 1
without a random draw; B is generated from A and C from Z, preserving the
B draw. The learned probabilities remain frozen, so A=1 selects p1.

Measured Brier scores in the tested local environment (six decimal places):

| Condition | Seed | Examples | Learned predictor | Constant 0.5 reference |
|---|---:|---:|---:|---:|
| Observation | 43 | 10000 | 0.216343 | 0.250000 |
| do(A=0) | 44 | 10000 | 0.281148 | 0.250000 |
| do(A=1) | 45 | 10000 | 0.283409 | 0.250000 |

The do(A=1) implementation compiled without warnings with the documented C11
options. Two complete executions produced identical output and status 0,
with training and earlier evaluation results unchanged. In a temporary copy,
forcing prediction to 0.5 only during do(A=1) yielded both scores exactly
equal to 0.25 before rounding, with status 0. Temporary test files were removed.

These measurements are distinct from the ideal theoretical expectations
documented in the experiment design. The scoped V3 experiment is implemented
and checked by the reusable script below, without exhaustive validation.
These results concern one sample
per evaluated condition. No real AI agent is
evaluated, and these results do not demonstrate causal understanding or
general agent reliability.

#### V3 verification script

From the project root, run:

```sh
sh tests/test_v3.sh
```

Prerequisites are `/bin/sh`, a C11 compiler available as `gcc`, and the tools
`dirname`, `mktemp`, `rm`, `cmp` and `awk`, available on the tested Mac.
The script uses `LC_ALL=C`, locates the project from its own path, and compiles
with `-std=c11 -Wall -Wextra -Wpedantic` in a unique temporary directory.
It preserves local executables and cleans up its directory on exit, including
failure; HUP, INT and TERM are handled. Compilation failure stops the trials.

Built-in script checks require two successful executions of the same binary,
empty stderr for both runs and identical stdout. The output parser requires
exactly one section for observation, do(A=0) and do(A=1), with exactly one
learned and one constant Brier score in each section. Scores must be complete
decimal strings with a point, numerically in [0,1]; each constant score must
equal 0.25. Missing, duplicate, misplaced or malformed sections/scores fail.
No exact learned score or ranking relative to the reference is imposed.
The script reports `PASS`/`FAIL`, returning 0 on success and 1 on a check,
setup or compilation failure (handled interruptions return their signal status).

Separate one-off development trials checked the exact parser against altered
outputs, including missing/duplicate sections or scores, malformed headings,
scores before any section, empty values, NaN/inf, trailing text, out-of-range
values and a constant other than 0.25. These were isolated parser tests.
Full-script trials used temporary project copies: normal execution returned
0, one reference displayed as 0.26 caused final status 1, and an intentional
C compilation error caused status 1 without launching simulations. Normal
execution also passed from another directory using an absolute script path.
The script-owned temporary directory was verified removed after success and
failure using dedicated TMPDIR locations. These fault-injection and cleanup
trials are not automatically repeated by the script.

The script checks output structure, ranges and local reproducibility. It does
not independently recalculate Brier scores, establish cross-platform random
sequence identity, or validate an AI agent. The existing temporary-copy C
counter and prediction-injection trials also remain one-off verifications.

### V4 — Dynamic world (long-term horizon)

`state_t + action_t → world model → state_(t+1)`, with counterfactual predictions ("what would have happened with a different action?"). A real jump in difficulty — full control over ground truth gets harder to keep. Only once V1–V3 are solid.

### V5 — Latent world model (exploratory, not a planned milestone)

Encoder → latent state → predictor → future latent state, loosely inspired by world model / JEPA ideas (Yann LeCun). Active research territory for entire labs — a direction to draw inspiration from, not a concrete goal.

---

## Notes

- No deadline — this runs alongside coursework, never competing with it.
- One milestone at a time.
- Each version is documented here as I go, including what didn't work.
