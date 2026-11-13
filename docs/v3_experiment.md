# TinyWorld V3 — Prediction under intervention

Status: observational probability estimation and evaluation on fresh
observational data are implemented in `causal_v3.c`, including Brier scoring
and comparison with the constant reference. Evaluation under do(A=0) is also
implemented and measured, as is evaluation under do(A=1). This scoped experiment
is implemented and checked by `tests/test_v3.sh`; validation is not exhaustive.
The preserved V2 reference is commit `0d1b15e`.

## Question and hypothesis

Does a predictor trained on observational association between A and C retain
its predictive performance when A is externally fixed?

The defined causal graph is `Z → A`, `Z → C`, `A → B`. All variables are binary.
P(Z=1) is 0.5. For both A and C, the probability of value 1 is 0.8 given
Z=1 and 0.2 given Z=0. Interventions replace only the generation of A.

Hypothesis: an observational frequency predictor will outperform a constant
0.5 predictor on fresh observational data, but perform worse than that
reference under both do(A=0) and do(A=1). This is an expectation to investigate,
not a measured result or a guaranteed ordering in every finite sample.

## Predictor and accessible data

Training uses observational pairs (A, C) only. C is the target label.
The learner estimates two probabilities using training counts:

```text
p0 = count(A=0 and C=1) / count(A=0)
p1 = count(A=1 and C=1) / count(A=1)
```

Use floating-point division and probabilities on the scale [0, 1], not
percentages. If either training group is empty, report that training cannot
be completed and stop; do not substitute a zero probability.

For all three implemented evaluation conditions, p0 and p1 remain fixed
after training.
For each evaluation sample, the predictor
receives only A and returns p0 if A=0 or p1 if A=1. It never receives Z, B,
the intervention mode, or the current C label before predicting. The evaluator
uses C only to score the prediction. No evaluation labels update the estimates.

The reference predictor always returns 0.5, without training. Its probability
is a predefined experimental reference based on this world's marginal C
distribution; it is not learned from evaluation data.

## Proposed protocol

Use 10,000 samples in each phase, with the following seed allocation fixed
before examining scores:

| Phase | Generation condition | Seed | Purpose |
|---|---|---:|---|
| Training | Observation | 42 | Estimate p0 and p1 once |
| Evaluation 1 | Fresh observation | 43 | Measure performance in the training distribution |
| Evaluation 2 | do(A=0) | 44 | Measure performance with A fixed to 0 |
| Evaluation 3 | do(A=1) | 45 | Measure performance with A fixed to 1 |

For each phase, initialize the random generator once with that phase's seed.
Reset simulation and scoring counters between phases. Keep training estimates
unchanged throughout all evaluation phases. Do not reuse training samples for
evaluation and do not reinitialize the generator for each individual sample.

Generate evaluation outcomes using the same causal mechanisms as V2, with
B and C unchanged under intervention. Evaluate both predictors on exactly the
same samples within each condition. Keep an identical generation order for
repeated runs of a condition, and record that order in the eventual implementation.

Distinct seeds separate the intended generation runs; they do not establish
mathematical independence of the pseudo-random sequences. Samples may also
have identical binary values by chance, which is not itself data reuse.

Record the source revision, compiler and C library environment, seeds, sample
counts, training group counts, p0, p1, and both scores for each condition.
Record empirical C frequencies in evaluation to help interpret finite-sample
variation. Repeat the entire experiment locally with the same configuration
and compare unrounded estimates and scores. Do not select seeds after seeing
which ones support the hypothesis.

## Measure: Brier score

For prediction p and observed binary outcome c, the loss is `(p - c)^2`.
The Brier score is the mean loss across evaluation samples:

```text
Brier = sum((prediction_i - C_i)^2) / number_of_evaluation_samples
```

Lower is better. For example, predicting 0.8 when C=1 gives a loss of 0.04;
the same prediction when C=0 gives 0.64. A constant prediction of 0.5 has
loss 0.25 for either binary outcome, so its score is exactly 0.25 in any
nonempty sample. Use unrounded probabilities and accumulated losses for scoring.

Report one mean Brier score per predictor and evaluation condition, plus
`learned_score - constant_score`. A negative difference favors the learned
predictor; a positive difference favors the constant reference. Do not average
all three conditions into one score that hides their differences.

## Theoretical expectations — not measured results

The following values assume the ideal probability model and its exact
observational conditional probabilities, p0=0.32 and p1=0.68. They are not
scores already obtained by a finite-data trained predictor.

| Evaluation condition | True P(C=1 given available A) | Ideal observational predictor | Expected Brier | Constant Brier |
|---|---|---|---:|---:|
| Observation | 0.32 for A=0; 0.68 for A=1 | 0.32 or 0.68 | 0.2176 | 0.25 |
| do(A=0) | 0.50 | 0.32 | 0.2824 | 0.25 |
| do(A=1) | 0.50 | 0.68 | 0.2824 | 0.25 |

For a binary outcome with probability q, expected squared loss for a fixed
prediction p is `q * (1-p)^2 + (1-q) * p^2`. Under observation the ideal
prediction equals q, giving `0.32 * 0.68 = 0.2176`. Under intervention,
q=0.5 and the prediction remains 0.32 or 0.68, giving
`0.25 + (0.18)^2 = 0.2824`.

The association between A and C in observational data comes from their shared
cause Z. Fixing A removes its dependence on Z without changing C's mechanism.
The trained lookup probabilities can therefore become unsuitable under
intervention. Finite training estimates and evaluation scores will fluctuate;
exact equality to these theoretical scores is not a success requirement.

## Measured results

### First milestone: observational training only

The current implementation generates 10,000 observational samples with training
seed 42. It preserves the V2 random draw order Z, A, B, C, including the unused
B draw. Training counts use only A and C; Z remains internal to generation.
The two probabilities are estimated from counts, not hard-coded, stored on
the scale [0, 1], and converted to percentages only for display.

The following counts and estimates were obtained in the project owner's local
environment. Estimates shown here are rounded to six decimal places:

| Quantity | Measured value |
|---|---:|
| count_a0 | 4930 |
| count_a1 | 5070 |
| count_a0_c1 | 1584 |
| count_a1_c1 | 3476 |
| p0 = count_a0_c1 / count_a0 | 0.321298 |
| p1 = count_a1_c1 / count_a1 | 0.685602 |

The owner verified a successful run with exit status 0 and identical output
from two executions in the same local environment. These are training estimates,
not theoretical probabilities or held-out evaluation results. Observational
evaluation results are reported separately in the next milestone.

Before division, the program checks that A group sizes sum to the training
sample count, that each joint count lies between zero and its group size,
and that both groups contain samples.

Three negative scenarios were executed on separate temporary source copies,
compiled with `-std=c11 -Wall -Wextra -Wpedantic`:

| Injected fault | Expected and observed stderr message | Exit status |
|---|---|---:|
| Increment count_a0 after generation | `Validation failed: incorrect A group totals.` | 1 |
| Set count_a0_c1 to count_a0 + 1 | `Validation failed: joint counts must be between zero and their A group sizes.` | 1 |
| Set count_a0=0, count_a0_c1=0, count_a1=num_training_observations | `Training failed: both A groups must contain samples.` | 1 |

Each negative run produced empty stdout. The empty-group injection preserved
the total and joint-count bounds, so it specifically reached the denominator
check. No injected fault remains in the main source. These development checks
do not yet constitute a reusable V3 test suite. The measured results cover
only the training milestone; they do not validate the full proposed protocol.

### Second milestone: observational evaluation

After training with seed 42, p0 and p1 remain unchanged. A separate generation
phase uses seed 43 and 10,000 new observational samples, preserving the draw
order Z, A, B, C. No evaluation sample updates the training counts or estimates.
Prediction selects p0 or p1 using only A. Both predictors are scored against
the same C outcome on each sample, with separate sums of squared errors.
The evaluation sample count is checked to be positive before averaging.

Measured in the tested local environment:

| Evaluation condition | Seed | Samples | Learned Brier | Constant-reference Brier |
|---|---:|---:|---:|---:|
| Fresh observation | 43 | 10000 | 0.216343 | 0.250000 |

Scores in the table are displayed to six decimal places. Compilation with
`-std=c11 -Wall -Wextra -Wpedantic` produced no warnings. Two executions
produced identical output, empty stderr and exit status 0.

A separate one-off verification replaced the learned prediction selection
with `prediction = 0.5;` in a temporary source copy. Both scores were exactly
equal to 0.25 before rounding; this equality was checked inside that copy,
not inferred only from formatted output. The copy compiled without warnings,
returned status 0 and produced empty stderr. Temporary files were removed
and the main source was preserved. This check is not integrated into a
reusable V3 test suite.

The learned predictor scored better than the constant reference on this
observational evaluation sample. This does not establish performance across
other seeds or distributions, causal understanding, or AI-agent reliability.
The intervention scores in the theoretical table above remain theoretical
expectations, distinct from the measured intervention results below.

### Third milestone: evaluation under do(A=0)

A separate evaluation phase uses seed 44 and 10,000 examples. Each example
generates Z, fixes A=0 without a random draw, generates B according to A,
and generates C according to Z. The intervention replaces only the mechanism
for A. Both learned probabilities remain unchanged; A=0 selects p0 for every
prediction. There is no retraining on intervention outcomes.

Two new sums accumulate squared errors for the learned predictor and constant
0.5 reference on the same examples. They are separate from the observational
evaluation sums. The number of examples is checked before division.

| Evaluation condition | Seed | Samples | Learned Brier | Constant-reference Brier |
|---|---:|---:|---:|---:|
| do(A=0) | 44 | 10000 | 0.281148 | 0.250000 |

These are measured scores displayed to six decimal places. Training counts
and estimates remained unchanged (p0=0.321298, p1=0.685602 as displayed), as
did the observational scores (0.216343 and 0.250000). Compilation with C11
and `-Wall -Wextra -Wpedantic` produced no warnings. Two complete executions
produced identical output locally, with empty stderr and exit status 0.

A one-off temporary source copy replaced the prediction selection only during
do(A=0) with `prediction = 0.5;`. Both intervention scores were checked to
equal 0.25 before formatting. Compilation produced no warnings, execution
returned status 0, and earlier-phase output remained unchanged. Temporary
files were cleaned up; the fault-free main source was preserved. This check
is not integrated into a reusable V3 test suite.

The learned predictor scored worse than the constant reference on this
interventional sample, in contrast to the observational evaluation. This is
consistent with the proposed hypothesis for do(A=0), but does not establish
performance across seeds or general causal understanding.

### Fourth milestone: evaluation under do(A=1)

The added phase uses seed 45 and 10,000 examples. For each example, it generates
Z, fixes A=1 directly without drawing a random value for A, generates B from
A, then C from Z. The B draw is retained. Training estimates p0 and p1 remain
unchanged; the predictor uses only A and therefore selects p1 in this phase.
Separate squared-error sums compare the learned predictor and constant 0.5
reference against the same C for each example. The sample count is checked
before calculating the averages.

Measured results for all three evaluation conditions in the tested environment:

| Evaluation condition | Seed | Examples | Learned Brier | Constant-reference Brier |
|---|---:|---:|---:|---:|
| Observation | 43 | 10000 | 0.216343 | 0.250000 |
| do(A=0) | 44 | 10000 | 0.281148 | 0.250000 |
| do(A=1) | 45 | 10000 | 0.283409 | 0.250000 |

These measured scores are displayed to six decimal places. They are not the
ideal expected scores in the theoretical table, and no measured score was
forced to equal an expected value.

Verification executed during the do(A=1) implementation session:

- Compilation with `-std=c11 -Wall -Wextra -Wpedantic` produced no warnings.
- Two complete executions produced identical output, empty stderr and status 0.
- Training estimates and counts, the observational scores and the do(A=0)
  scores remained unchanged.
- A temporary source copy forced `prediction = 0.5;` only during do(A=1).
  Both intervention scores were checked to equal 0.25 before formatting;
  compilation produced no warnings and execution returned status 0 with empty
  stderr. Earlier-phase output remained unchanged.
- Temporary test files were removed; no test injection remains in the source.

The learned predictor scored worse than the constant reference on this
do(A=1) sample, consistent with the hypothesis for this defined world. This
does not establish performance across seeds or general causal understanding.
The temporary-copy checks remain one-off development verifications, distinct
from the reusable output checks below. Implementing all three conditions does
not by itself complete exhaustive validation or every success criterion below.

## Reusable V3 verification

From the repository root:

```sh
sh tests/test_v3.sh
```

The script requires `/bin/sh`, a C11 compiler invoked as `gcc`, and `dirname`,
`mktemp`, `rm`, `cmp` and `awk`. These tools are available on the tested Mac.
It exports `LC_ALL=C` and resolves the source path relative to the script.
Compilation uses `-std=c11 -Wall -Wextra -Wpedantic`; the binary and captured
outputs live in a dedicated temporary directory. The local executable is not
replaced. Cleanup is registered after directory creation and runs on normal
exit and failure, with HUP, INT and TERM handlers also installed.

### Checks integrated into the script

- Stop the trials if compilation fails.
- Run the same binary twice; require status 0 and empty stderr for each run.
- Require byte-identical stdout from both runs.
- Parse the first output: require exactly one heading for each of the three
  evaluation conditions and exactly one learned and one constant score per
  section. Reject scores before a section and missing, duplicate or malformed
  sections/scores.
- Validate the entire score string as digits, a decimal point and digits
  before numeric conversion. Require each score in the inclusive range [0,1]
  and each constant-reference score numerically equal to 0.25.

No exact value from `rand()` is required for a learned score, and the script
does not require the learned predictor to outperform or underperform the
reference. It prints `PASS`/`FAIL`; success returns 0 and a verification, setup
or compilation failure returns 1. Signal handlers use interruption statuses.

### One-off checks of the verification tooling

These trials were executed during development but are not built into the
reusable script:

| Trial | Observed result | Status scope |
|---|---|---|
| Normal captured output, exact awk block extracted from the script | Accepted, status 0 | Parser only |
| Missing/duplicate section, malformed heading, missing/duplicate score, score before first section | Each rejected, status 1 | Parser only |
| Empty value, NaN, inf, trailing text, below-zero/above-one values, constant 0.26 | Each rejected, status 1 | Parser only |
| Temporary minimal project with original C and identical script | Passed, status 0 | Complete script |
| Same normal script launched from another directory by absolute path | Passed, status 0 | Complete script |
| Temporary C displaying constant 0.26 in one condition | Parsing failed, final status 1 | Complete script |
| Temporary C with intentional compilation error | Final status 1, no simulations launched | Complete script |

The script copies were byte-identical to the original. Dedicated TMPDIR
locations confirmed removal of the script-owned `tinyworld-v3.*` directory
after successes and failures. Compiler-created `xcrun_db` cache entries were
separate from that directory. All trial copies were subsequently cleaned up;
original C and shell files were unchanged.

### Interpretation and limits

This script verifies the scoped experiment's execution, local repeatability
and output structure. The parser checks displayed scores, not hidden internal
values. It does not independently recompute the sums or scores; plausible but
incorrect numbers can pass. Isolated parser tests are not full-script runs.
Fault cases tested during development are not exhaustive. Earlier counter
and constant-prediction injections also remain one-off temporary-copy trials.
Local reproducibility does not imply identical `rand()` sequences across C
libraries, broad generalization, causal understanding or AI-agent validation.

## Self-review of milestone 968940f

The examined milestone was `968940f` (`docs: prepare TinyWorld V3 for external
review`). The following were one-off self-review exercises, not additions to
the reusable test scripts.

### Fresh GitHub clone on the same machine

A new clone was obtained directly from
`https://github.com/juniorleighFramwork/TinyWorld.git` in a dedicated temporary
directory, without copying files from the working repository. Its HEAD was
`968940fd52681229ef8427b4f343c32865e35b16`, matching the expected milestone.

| Command executed in the fresh clone | Observed result | Exit status |
|---|---|---:|
| README “Run V3 directly” block | Training estimates and all three displayed scores reproduced | 0 |
| `sh tests/test_v2.sh` | All checks passed; stderr empty | 0 |
| `sh tests/test_v3.sh` | All checks passed; stderr empty | 0 |

The direct launch emitted Xcode environment diagnostics on stderr:
`DVTFilePathFSEvents: Failed to start fs event stream` and a
`DVTDeveloperPaths` diagnostic reporting failure to obtain
`DARWIN_USER_CACHE_DIR` via `confstr(3)` (input/output error), followed by a
fallback to `NSCachesDirectory`. The complete command nevertheless returned
status 0. These diagnostics were not emitted by the two verification-script
runs. The clone remained clean, and its dedicated temporary directory was
removed after verification.

### Separate Brier recalculation from evaluation counts

In a separate temporary copy of the working program, counters were added to
record the four (A,C) combinations in each evaluation. No random draws,
predictions or existing score calculations were changed. That instrumented
copy compiled with `-std=c11 -Wall -Wextra -Wpedantic` without warnings and
returned status 0 with empty stderr.

| Evaluation | n00: A=0,C=0 | n01: A=0,C=1 | n10: A=1,C=0 | n11: A=1,C=1 | N |
|---|---:|---:|---:|---:|---:|
| Observation | 3397 | 1571 | 1594 | 3438 | 10000 |
| do(A=0) | 5022 | 4978 | 0 | 0 | 10000 |
| do(A=1) | 0 | 0 | 4972 | 5028 | 10000 |

Each row sums to N=10000. The A=1 group is absent under do(A=0), and the A=0
group is absent under do(A=1), as required.

A separate Python calculation used `p0 = 1584 / 4930` and
`p1 = 3476 / 5070`, without first rounding them to the displayed estimates.
For each row, it computed:

```text
Brier = [n00*p0^2 + n01*(p0-1)^2
         + n10*p1^2 + n11*(p1-1)^2] / N
```

Here `^2` denotes mathematical squaring; the Python calculation used `**2`.
The constant 0.5 reference has squared error 0.25 for every binary outcome.

| Evaluation | Separately recalculated Brier | Program display | Recalculated minus displayed |
|---|---:|---:|---:|
| Observation | 0.216343303307174 | 0.216343 | +0.000000303307 |
| do(A=0) | 0.281148054425239 | 0.281148 | +0.000000054425 |
| do(A=1) | 0.283408576886119 | 0.283409 | -0.000000423114 |

All recalculated scores agree with the program at six decimal places; each
absolute difference from the displayed value is below 0.0000005. This compares
against formatted scores, not the program's unprinted full-precision sums.
The original source remained byte-identical and the temporary files were
removed.

### Scope of this self-review

These exercises are one-off checks, not new reusable tests. The separate
calculation checks the aggregation formula using outcomes from the same
simulator; it is not an independent implementation of data generation or
an exhaustive validation of the causal model. The fresh-clone reproduction
used the same machine and runtime environment. No external human review or
reproduction on another machine has yet been performed as part of this review.

## Success criteria

- Training and evaluation use separate generation phases and the documented,
  distinct seeds, without evaluation-label leakage.
- Both training groups exist; p0 and p1 are valid probabilities and remain
  frozen during all evaluation phases.
- Both predictors are scored on the same samples within each of the three
  conditions, with the constant reference scoring 0.25.
- Repeated runs in a compatible local environment reproduce estimates and
  scores for the same source and configuration.
- The report contains both scores and their difference for each condition,
  and separates observations from theoretical expectations. A disagreement
  with the hypothesis is reported and investigated, not hidden or forced to pass.

## Limits

This experiment evaluates a two-value frequency predictor. It does not
demonstrate causal understanding, internal reasoning, or general AI-agent
reliability. No real AI agent is involved.

The causal interpretation relies on the explicitly defined simulator and
correct implementation of its interventions. Predictive scores alone do not
prove that causal graph. One graph, one sample size and one fixed seed per
phase provide limited evidence about generalization or uncertainty.

The proposed comparison is descriptive, not a formal statistical significance
test. Additional seeds and uncertainty analysis would require a later protocol.
The existing `rand()` implementation can vary across C libraries, and
`rand() % 100` can introduce modulo bias. Exact cross-platform reproducibility
and exact agreement with ideal probabilities are not guaranteed.
