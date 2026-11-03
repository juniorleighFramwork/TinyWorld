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

Select `experiment_mode` in `main`, then recompile:

| Mode | Behavior |
|---|---|
| 0 | Observation: generate A according to Z. |
| 1 | do(A=0): impose A=0 independently of Z. |
| 2 | do(A=1): impose A=1 independently of Z. |

An intervention replaces the usual mechanism generating A. The mechanisms
for B and C remain unchanged: B depends on A, and C depends on Z.
Conditioning on A instead selects a subgroup of observational data without
changing any generating mechanism.

Compile and execute:

```bash
gcc -std=c11 -Wall -Wextra -Wpedantic causal_v2.c -o causal_v2
./causal_v2
```

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

The program rejects experiment modes outside 0, 1 and 2 before simulation,
reports the invalid value on `stderr`, and returns `EXIT_FAILURE`.

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
scenarios document verification work performed during development; there is
no committed, reusable test suite yet.

These frequency ranges are practical, non-exhaustive consistency checks,
not formal statistical tests. A correct simulation can occasionally fall
outside a range, and passing checks cannot exclude every implementation error.
Repeating one fixed seed checks local reproducibility, not behavior across
many seeds or C library implementations. The `rand()` limitations described
above still apply. These results do not validate any AI agent.

### V3 — Future agent evaluation — not implemented

Controlled interventions are already part of V2. No logistic regression,
trained model or real AI agent is currently evaluated by `causal_v2.c`.
The results above describe the simulator's behavior under defined causal
rules; they do not demonstrate an AI agent's causal understanding or reliability.
Future agent evaluation will require explicit tasks, success criteria and
controlled experiments.

### V4 — Dynamic world (long-term horizon)

`state_t + action_t → world model → state_(t+1)`, with counterfactual predictions ("what would have happened with a different action?"). A real jump in difficulty — full control over ground truth gets harder to keep. Only once V1–V3 are solid.

### V5 — Latent world model (exploratory, not a planned milestone)

Encoder → latent state → predictor → future latent state, loosely inspired by world model / JEPA ideas (Yann LeCun). Active research territory for entire labs — a direction to draw inspiration from, not a concrete goal.

---

## Notes

- No deadline — this runs alongside coursework, never competing with it.
- One milestone at a time.
- Each version is documented here as I go, including what didn't work.
