# TinyWorld V3 — Prediction under intervention

Status: proposed experiment. No V3 predictor has been implemented or evaluated.
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

After training, freeze p0 and p1. For each evaluation sample, the predictor
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

None yet. V2 frequency summaries are context for this design, not V3 training
estimates or held-out Brier scores. Populate results only after implementation
and execution of the stated protocol.

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
