#include <stdio.h>
#include <stdlib.h>
#include <time.h>

  /*
      *  TinyWorld - V1.b
      * Introduit un facteur cache Z, qui influence A ET C.
      * Cree ainsi une correlation A<->C, MEME SI A ne cause PAS C
      * directement — objectif : montrer qu'un modele naif pourrait
      * apprendre a tort "A predit C", alors que le vrai mecanisme
      * passe par Z (jamais observe dans le CSV).
  */
int main(void) {

  // Initalise le generateur aleatoire (une seule fois, sinon on obtient toujours
  // la meme séequence de nombres a chaque execution du programmme)
  srand(time(NULL));

  int limit = 10000;      // nombre d'observations à generer
  int compteur_A1 = 0;    // nombre de fois ou A=1
  int compteur_A1_B1 = 0; // nombre de fois ou A=1 et B=1

  // Ouvre le fichier
  FILE *fichier = fopen("tinyworld_v1b.csv", "w");
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
      int nb_prcent = 90;
      int nb_prcent2 = 10;


      // === A depend de Z ===
      if (Z == 1) {
        if (rdm_nbr < 80) { A = 1; }
      } else {
        if (rdm_nbr < 20) { A = 1; }
      }

      // === B depend de A (relation causal reelle) ===
      // Si A=1 : B=1 avec 90% de chance (forte dependance)
      // Si A=0 : B=1 avec seulement 10% de chance (faible dependance)

      if (A == 1) {
        if (rdm_nbr2 < nb_prcent) { B = 1; }
      } else {
        if (rdm_nbr2 < nb_prcent2) { B = 1; }
      }

      if (A == 1) {
        compteur_A1++;
      }
      if (A == 1 && B == 1) {
        compteur_A1_B1++;
      }

      // === C depend de Z (PAS de A, ni de B directement !) ===
      if (Z == 1) {
        if (rdm_nbr3 < 80) { C = 1; }
      } else {
        if (rdm_nbr3 < 20) { C = 1; }
      }

      // Affiche chaque ligne generee (A, B, C)
      // -> sera remplace plus tard par une ecriture dans un fichier CSV
      fprintf(fichier, "%d,%d,%d\n", A, B, C);
  }

  float prcentage = (float)compteur_A1_B1 / compteur_A1 * 100;
  printf("Sachant A=1, B=1 dans %.2f des cas\n", prcentage);

  fclose(fichier);
  return 0;
}
