#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define P_A_GIVEN_Z1 80
#define P_A_GIVEN_Z0 20
#define P_B_GIVEN_A1 90
#define P_B_GIVEN_A0 10
#define P_C_GIVEN_Z1 80
#define P_C_GIVEN_Z0 20
  /*
    * TinyWorld - V1.b
    *
    * Causal structure:
    *   Z (hidden, never written to CSV)
    *   Z -> A
    *   A -> B   (real causal link)
    *   Z -> C   (C does NOT depend on A or B directly)
    *
    * Result: A and C appear correlated in the data, but this
    * correlation is entirely spurious - it comes from the shared
    * cause Z, not from any real effect of A on C.
    *
    * Goal: show that a naive model trained only on (A,B,C) could
    * wrongly learn "A predicts C", missing that the real driver (Z)
    * is never observed.
 */
int main(void) {

  // Initalise le generateur aleatoire (une seule fois, sinon on obtient toujours
  // la meme séequence de nombres a chaque execution du programmme)
  srand(time(NULL));

  int limit = 10000;      // nombre d'observations à generer
  int compteur_A1 = 0;    // nombre de fois ou A=1
  int compteur_A1_B1 = 0; // nombre de fois ou A=1 et B=1
  int compteur_A0 = 0;    // nombre de fois ou A=0
  int compteur_A0_C1 = 0; // nombre de fois ou A=0 et C=1
  int compteur_A1_C1 = 0;  // nombre de fois ou A=1 et C=1

  // Ouvre le fichier
  FILE *fichier = fopen("tinyworld_v1b.csv", "w");
  if (fichier == NULL) {
      printf("Erreur : impossible d'ouvrir le fichier\n");
      return 1;
  }
  fprintf(fichier, "A,B,C\n");

  for (int i = 0; i < limit; i++) {

      // facteur cache, jamais ecrit dans le CSV
      int Z = rand() % 2; // Tire Z aleatoirement (0 ou 1)

      int A = 0;
      int B = 0;
      int C = 0;  // C depend de Z (facteur cache commun avec A)

      // Nombre aleatoire utilises pour decider si B et C
      // se declenchent, selon une probabilite donnee
      int rdm_nbr = rand() % 100;
      int rdm_nbr2 = rand() % 100;
      int rdm_nbr3 = rand() % 100;


      // === A depend de Z ===
      if (Z == 1) {
        if (rdm_nbr < P_A_GIVEN_Z1) { A = 1; }
      } else {
        if (rdm_nbr < P_A_GIVEN_Z0) { A = 1; }
      }

      // === B depend de A (relation causal reelle) ===
      // Si A=1 : B=1 avec 90% de chance (forte dependance)
      // Si A=0 : B=1 avec seulement 10% de chance (faible dependance)

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

      // === C depend de Z (PAS de A, ni de B directement !) ===
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

      // Affiche chaque ligne generee (A, B, C)
      fprintf(fichier, "%d,%d,%d\n", A, B, C);
  }

  float prcentage = (float)compteur_A1_B1 / compteur_A1 * 100;
  float prcentage_2 = (float)compteur_A0_C1 / compteur_A0 * 100;
  float prcentage_3 = (float)compteur_A1_C1 / compteur_A1 * 100;
  // P(B=1 | A=1) real causal relationship, should be high (~90%)
  printf("Sachant A=1, B=1 dans %.2f%% des cas\n", prcentage);

  // P(C=1 | A=0) and P(C=1 | A=1) should look correlated
  // even though A has no real effect on C (confound via Z)
  printf("Sachant A=0, C=1 dans %.2f%% des cas\n", prcentage_2);
  printf("Sachant A=1, C=1 dans %.2f%% des cas\n", prcentage_3);

  fclose(fichier);
  return 0;
}
