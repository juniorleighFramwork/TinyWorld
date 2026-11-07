#include <stdio.h>
#include <stdlib.h>

/*
 * TinyWorld - V3
 *
 * Study how predictions learned from observational data behave
 * when evaluated under controlled causal interventions.
 *
 * Current implementation:
 * - Generate observational training samples with a fixed seed.
 * - Estimate P(C=1 | A=0) and P(C=1 | A=1) from sample counts.
 * - Validate counters before calculating probabilities.
 *
 * The predictor uses only A to predict C.
 * Z remains hidden, and B is not a predictor input.
 *
 * Evaluation on new samples and Brier scoring are not yet implemented.
 */

/* Causal model parameters: probabilities expressed as percentages. */
#define P_A_GIVEN_Z1 80
#define P_A_GIVEN_Z0 20

#define P_B_GIVEN_A1 90
#define P_B_GIVEN_A0 10

#define P_C_GIVEN_Z1 80
#define P_C_GIVEN_Z0 20

// Return 1 with the requested probability (percentage),
// otherwise return 0
int random_event(int probability) {
    return rand() % 100 < probability;
}

int main(void) {

  // Configure observational training using the seed defined in the V3 protocol.
  unsigned int training_seed = 42;
  int num_training_observations = 10000;

  // Initialize once, before generating any training samples.
  srand(training_seed);

  // Count the size of each A group.
  int count_a0 = 0;
  int count_a1 = 0;
  // Count samples with C=1 within each A group.
  int count_a0_c1 = 0;
  int count_a1_c1 = 0;

  // Preserve the V2 observational draw order: Z, A, B, then C.
  for (int i = 0; i < num_training_observations; i++) {

    // Z belongs to the simulator; training counters must use only A and C.
    int z = random_event(50);
    // Generate A from Z without intervention during training.
    int a;

    if (z == 1) {
        a = random_event(P_A_GIVEN_Z1);
    } else {
        a = random_event(P_A_GIVEN_Z0);
    }

    // Generate B from A to preserve the random draw before C.
    // B is not an input to the learner.
    int b;

    if (a == 1)
    { b = random_event(P_B_GIVEN_A1);}
    else
    {b = random_event(P_B_GIVEN_A0);}

    // B is intentionally unused after preserving its random draw
    (void)b;
    // Generate C according to the hidden variable Z
    int c;

    if (z == 1)
    {c = random_event(P_C_GIVEN_Z1);}
    else
    { c = random_event(P_C_GIVEN_Z0);}

    // Update group sizes; each sample belongs to one A group.
    if (a == 0) { count_a0++;}
    if (a == 1) { count_a1++;}

    // Update joint counts only when C=1 in the corresponding A group.
    if (a == 0 && c == 1) { count_a0_c1++;}
    if (a == 1 && c == 1) { count_a1_c1++;}
  }

  // Verify that every training sample belongs to an A group.
  if (count_a0 + count_a1 != num_training_observations) {
      fprintf(stderr, "Validation failed: incorrect A group totals.\n");
      return EXIT_FAILURE;
  }

  // Verify that joint counts are within their corresponding group sizes.
  if (count_a0_c1 < 0 || count_a0_c1 > count_a0 || count_a1_c1 < 0 || count_a1_c1 > count_a1) {
        fprintf(stderr, "Validation failed: joint counts must be between zero and their A group sizes.\n");
        return EXIT_FAILURE;
      }

  // Ensure both A groups contain samples before dividing.
  if (count_a0 == 0 || count_a1 == 0) {
      fprintf(stderr, "Training failed: both A groups must contain samples.\n");
      return EXIT_FAILURE;
  }

  // Estimate C=1 probability within each A group.
  double p0 = (double)count_a0_c1 / count_a0;
  double p1 = (double)count_a1_c1 / count_a1;

  // Display learned probabilities.
  printf("P(C=1 | A=0): %.6f (%.2f%%)\n", p0, p0 * 100.0);
  printf("P(C=1 | A=1): %.6f (%.2f%%)\n", p1, p1 * 100.0);

  // Display training configuration and counts.
  printf("Training seed: %u\n", training_seed);
  printf("Training observations: %d\n", num_training_observations);
  printf("A=0 samples: %d\n", count_a0);
  printf("A=1 samples: %d\n", count_a1);
  printf("A=0 and C=1 samples: %d\n", count_a0_c1);
  printf("A=1 and C=1 samples: %d\n", count_a1_c1);

  return 0;
}
