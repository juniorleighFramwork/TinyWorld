#include <stdio.h>
#include <stdlib.h>

/* Causal model parameters: probabilities expressed as percentages. */
#define P_A_GIVEN_Z1 80
#define P_A_GIVEN_Z0 20

#define P_B_GIVEN_A1 90
#define P_B_GIVEN_A0 10

#define P_C_GIVEN_Z1 80
#define P_C_GIVEN_Z0 20

/*
 * TinyWorld - V2
 *
 * Development note:
 * This version will introduce controlled causal interventions.
 *
 * First objective:
 * Prepare a separate program without modifying V1.b
*/

// Return 1 with the requested probability (percentage),
// otherwise return 0
int random_event(int probability) {
    return rand() % 100 < probability;
}


int main(void) {

  // Initialize the random number generator with a fixed seed.
  srand(42);

  // Define the number of observations in this experiment
  int num_observations = 10000;

  // Select 0 for observation or 1 for do(A=0), or 2 for do(A=1)
  int experiment_mode = 2;

  // Initialize counters for observed value equal to 1
  int count_a1 = 0;
  int count_b1 = 0;
  int count_c1 = 0;

  // Count observations for each value A
  int count_a0 = 0;

  // Count B outcomes seperatly for each value of A
  int count_a1_b1 = 0;
  int count_a0_b1 = 0;

  // Count C outcomes separately for each value of A
  int count_a1_c1 = 0;
  int count_a0_c1 = 0;

  // Generating ten observations from the causal model
  for (int i = 0; i < num_observations; i++)
  {

    // Generate one value for the hidden variable Z.
    int z = random_event(50);

    // Generate A observationally or fix it by intervention.
    int a;

    if (experiment_mode == 1) {
        a = 0;
    } else if (experiment_mode == 2) {
        a = 1;
    } else {
        if (z == 1) {
          a = random_event(P_A_GIVEN_Z1);
        } else {
          a = random_event(P_A_GIVEN_Z0);
        }
    }


    // Generate B according to the value of A
    int b;

    if (a == 1)
    { b = random_event(P_B_GIVEN_A1);}
    else
    {b = random_event(P_B_GIVEN_A0);}

    // Generate C according to the hidden variable Z
    int c;

    if (z == 1)
    {c = random_event(P_C_GIVEN_Z1);}
    else
    { c = random_event(P_C_GIVEN_Z0);}

    // Count occurrences of value 1 for each observed variable
    if (a == 1) { count_a1++; }

    if (a == 0) { count_a0++; }

    if (b == 1) { count_b1++; }

    if (c == 1) { count_c1++; }

    // Track B outcomes for each value of A
    if (a == 1 && b == 1) { count_a1_b1++; }
    if (a == 0 && b == 1) { count_a0_b1++; }

    // Track C outcomes for each value of A
    if (a == 1 && c == 1) { count_a1_c1++; }
    if (a == 0 && c == 1) { count_a0_c1++; }

    // Display individual observations only for small experiments
    if (num_observations <= 10) {
       printf("Observation %d: A=%d, B=%d, C=%d\n", i+1, a, b, c);
    }
  }

  // Display the final counts
  printf("\nSummary after %d observations:\n", num_observations);
  printf("A = 1: %d times\n", count_a1);
  printf("B = 1: %d times\n", count_b1);
  printf("C = 1: %d times\n", count_c1);

  // Calculate observed frequencies as percentages
  double frequency_a = count_a1 * 100.0 / num_observations;
  double frequency_b = count_b1 * 100.0 / num_observations;
  double frequency_c = count_c1 * 100.0 / num_observations;

  // Calculate conditional frequencies for B
  double frequency_b_given_a1 = 0.0;
  double frequency_b_given_a0 = 0.0;

  if (count_a1 > 0) {
      frequency_b_given_a1 = count_a1_b1 * 100.0 / count_a1;
  }

  if (count_a0 > 0) {
      frequency_b_given_a0 = count_a0_b1 * 100.0 / count_a0;
  }

  // Display observed frequencies as percentage
  printf("Frequency of A = 1: %.1f%%\n", frequency_a);
  printf("Frequency of B = 1: %.1f%%\n", frequency_b);
  printf("Frequency of C = 1: %.1f%%\n", frequency_c);

  // Display conditional frequencies for B
  if (count_a1 > 0) {
      printf("Frequency of B = 1 given A = 1: %.1f%%\n", frequency_b_given_a1);
  } else {
      printf("Frequency of B = 1 given A = 1: N/A (no observation)\n");
  }

  if (count_a0 > 0) {
      printf("Frequency of B = 1 given A = 0: %.1f%%\n", frequency_b_given_a0);
  } else {
      printf("Frequency of B = 1 given A = 0: N/A (no observation)\n");
  }

    // Display conditional frequencies for C
  if (count_a1 > 0) {
      double frequency_c_given_a1 = count_a1_c1 * 100.0 / count_a1;
      printf("Frequency of C = 1 given A = 1: %.1f%%\n", frequency_c_given_a1);
  } else {
      printf("Frequency of C = 1 given A = 1: N/A (no observations)\n");
  }

  if (count_a0 > 0) {
      double frequency_c_given_a0 = count_a0_c1 * 100.0 / count_a0;
      printf("Frequency of C = 1 given A = 0: %.1f%%\n", frequency_c_given_a0);
  } else {
      printf("Frequency of C = 1 given A = 0: N/A (no observations)\n");
  }

  return 0;
}
