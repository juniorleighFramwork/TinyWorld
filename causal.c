#include <stdio.h>
#include <stdlib.h>
#include <time.h>

  /*
      * TinyWorld - V1.a
      * Genere une chaine causale minimale : A -> B -> C
      * Chaque evenement depend probabilistiquement du precedent.
      * Objectif : explorer si un model peut apprendre la vraie
      * structure causal, ou seulement les correlation observees.
  */
int main(void) {

  // Initalise le generateur aleatoire (une seule fois, sinon on obtient toujours
  // la meme séequence de nombres a chaque execution du programmme)
  srand(time(NULL));

  int limit = 10000;      // nombre d'observations à generer
  int compteur_A1 = 0;    // nombre de fois ou A=1
  int compteur_A1_B1 = 0; // nombre de fois ou A=1 et B=1

  // Ouvre le fichier
  FILE *fichier = fopen("tinyworld_v1a.csv", "w");
  fprintf(fichier, "A,B,C\n");

  for (int i = 0; i < limit; i++) {

      // A = evenement de depart, tire au hasard (0 ou 1)
      // Exemple : "il pleut"(1) ou "il ne pleut pas"(0)
      int A = rand() % 2;

      int B = 0;  // B depend de A (voir logique ci-dessous)
      int C = 0;  // C depend de B (voir logique ci-dessous)

      // Nombre aleatoire utilises pour decider si B et C
      // se declenchent, selon une probabilite donnee
      int rdm_nbr = rand() % 100;
      int rdm_nbr2 = rand() % 100;
      int nb_prcent = 90;
      int nb_prcent2 = 10;

      // === Calcul de B, en fonction de A ===
      // Si A=1 : B=1 avec 90% de chance (forte dependance)
      // Si A=0 : B=1 avec seulement 10% de chance (faible dependance)

      if (A == 1) {
        if (rdm_nbr < nb_prcent) {
          B = 1;
        }
      } else {
        if (rdm_nbr < nb_prcent2) {
          B = 1;
        }
      }

      if (A == 1) {
        compteur_A1++;
      }
      if (A == 1 && B == 1) {
        compteur_A1_B1++;
      }

      // === Calcul de C, en fonction de B ===
      // Meme logique que pour B : si B=1, C=1 tres probable (90%);
      //                         : si B=0, C=1 peu probable (10%)

      if (B == 1) {
        if (rdm_nbr2 < nb_prcent) {
          C = 1;
        }
      } else {
        if (rdm_nbr2 < nb_prcent2) {
          C = 1;
        }
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
