#include <stdio.h>
#include <stdlib.h>

#define P_A_GIVEN_Z1 80
#define P_A_GIVEN_Z0 20
#define P_B_GIVEN_A1 90
#define P_B_GIVEN_A0 10
#define P_C_GIVEN_Z1 80
#define P_C_GIVEN_Z0 20
  /*
    * TinyWorld - V1.b
    *
    * Development note:
    * This version introduces a hidden confounder, Z.
    *
    * Causal structure:
    *   Z -> A
    *   Z -> C
    *   A -> B
    *
    * Z is not exported to the CSV. A and C can therefore
    * be statistically associated even though A has no
    * direct causal effect on C in this model.
    *
    * V1.b objective:
    * Generate controlled data and verify that the observed
    * condition probabilities approach their theoretical
    * values. Model training and intervention tests belong
    * to later version.
    *
    * Reproducibility:
    * A fixed random seed is used to reproduce the pseudo-random
    * sequence with the same program and C library implementation.
    *
    * Validation status:
    * Compilation successful with strict warnings enabled.
    * Generated 1,000,000 observations.
    * Conditional probabilities are close to theoretical values.
    * Repeated runs produced identical CSV checksums
    * in the tested environment.
    */

int main(void) {

  // V1.b uses a fixed seed to reproduce the same
  // pseudo-random sequence across run.
  srand(42);

  int limit = 1000000;      // nombre d'observations à generer
  int compteur_A1 = 0;    // nombre de fois ou A=1
  int compteur_A1_B1 = 0; // nombre de fois ou A=1 et B=1
  int compteur_A0 = 0;    // nombre de fois ou A=0
  int compteur_A0_C1 = 0; // nombre de fois ou A=0 et C=1
  int compteur_A1_C1 = 0;  // nombre de fois ou A=1 et C=1

  // Ouvre le fichier
  FILE *fichier = fopen("tinyworld_v1b.csv", "w");

  if (fichier == NULL) {
      fprintf(stderr, "Error: unable to open the CSV file.\n");
      return 1;
  }

  // V1.b validation: check that CSV header
  // was written successfully generating data
  if (fprintf(fichier, "A,B,C\n") < 0) {
      fprintf(stderr, "Erreur lors de l'ecriture de l'en-tete CSV.\n");
      fclose(fichier);
      return 1;
  }

  // Generate the observations
  for (int i = 0; i < limit; i++) {

      // Hidden common cause Z
      int Z = rand() % 2;


      int A = 0;
      int B = 0;
      int C = 0;

      // Generate separate random values for A, B and C.
      // Each value is used to apply its conditional probability.
      int rdm_nbr = rand() % 100;
      int rdm_nbr2 = rand() % 100;
      int rdm_nbr3 = rand() % 100;


      // A depends on Z
      if (Z == 1) {
        if (rdm_nbr < P_A_GIVEN_Z1) { A = 1; }
      } else {
        if (rdm_nbr < P_A_GIVEN_Z0) { A = 1; }
      }

      // B depend de A ( a genuine causal relationship)

      if (A == 1) {
        if (rdm_nbr2 < P_B_GIVEN_A1) { B = 1; }
      } else {
        if (rdm_nbr2 < P_B_GIVEN_A0) { B = 1; }
      }

      if (A == 1) {
        compteur_A1++;
      }
      if (A == 1 && B == 1) {
        compteur_A1_B1++;
      }

      // C depends on Z, not directly on A or B
      if (Z == 1) {
        if (rdm_nbr3 < P_C_GIVEN_Z1) { C = 1; }
      } else {
        if (rdm_nbr3 < P_C_GIVEN_Z0) { C = 1; }
      }

      if (A == 0) {
        compteur_A0++;
      }
      if (A == 0 && C == 1) {
        compteur_A0_C1++;
      }
      if (A == 1 && C == 1) {
        compteur_A1_C1++;
      }

      // V1.b validation: detect CSV write errors
      // instead of silently producing an incomplete dataset.
      if (fprintf(fichier, "%d,%d,%d\n", A, B, C) < 0) {
          fprintf(stderr, "Erreur lors de l'ecriture du CSV.\n");
          fclose(fichier);
          return 1;
      }
  }

// Check that the dataset was flushed and closed successfully
// before reporting validation statistics.
if (fclose(fichier) != 0) {
    fprintf(stderr, "Error: unable to close the CSV file.\n");
    return 1;
}

// V1.b validation: ensure both groups exist
// before calculating conditional probabilities.
if (compteur_A1 == 0 || compteur_A0 == 0) {
    fprintf(stderr,
            "Error: an A group is empty; statistics cannot be calculated.\n");
    return 1;
}

  // Calculate conditional probabilities as percentages
  float pourcentage_B1_sachant_A1 = (float)compteur_A1_B1 / compteur_A1 * 100.0f;
  float pourcentage_C1_sachant_A0 = (float)compteur_A0_C1 / compteur_A0 * 100.0f;
  float pourcentage_C1_sachant_A1 = (float)compteur_A1_C1 / compteur_A1 * 100.0f;

  // Display validation statistics
  printf("TinyWorld V1.b - Validation results\n");
  printf("-----------------------------------\n");

  printf("Total observations: %d\n", limit);

  printf("P(A=1): %.2f%%\n", (float)compteur_A1 / limit * 100.0f);

  printf("P(B=1 | A=1): %.2f%%\n", pourcentage_B1_sachant_A1);

  printf("P(C=1 | A=0): %.2f%%\n", pourcentage_C1_sachant_A0);

  printf("P(C=1 | A=1): %.2f%%\n", pourcentage_C1_sachant_A1);

  // Development note:
  // These are observational probabilities.
  // The association between A and C is caused by hidden Z.
  // This association does not imply a direct causal effect of A on C.

  return 0;
}
