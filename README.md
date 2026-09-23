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

### V2 — Simple models — in progress

Training a logistic regression from scratch in C (sigmoid, gradient descent, log-loss, no libraries) on the V1.b dataset. Goal: check whether the model assigns real weight to `Z` despite it having no causal effect — which it should, since a plain logistic regression has no way to tell correlation from causation on its own.

### V3 — Interventions

Manually forcing values (e.g. `A = 1, Z = 0`) to break the training correlation, and checking whether the model adapts correctly or stays anchored to the spurious signal.

### V4 — Dynamic world (long-term horizon)

`state_t + action_t → world model → state_(t+1)`, with counterfactual predictions ("what would have happened with a different action?"). A real jump in difficulty — full control over ground truth gets harder to keep. Only once V1–V3 are solid.

### V5 — Latent world model (exploratory, not a planned milestone)

Encoder → latent state → predictor → future latent state, loosely inspired by world model / JEPA ideas (Yann LeCun). Active research territory for entire labs — a direction to draw inspiration from, not a concrete goal.

---

## Notes

- No deadline — this runs alongside coursework, never competing with it.
- One milestone at a time.
- Each version is documented here as I go, including what didn't work.
