#include <stdio.h>

// Ecris un programme qui ecrit le nombres 1 a 5 dans un fichier "nombres.csv, un par ligne"

int main(void) {

  FILE *fichier = fopen("nombres.csv", "w");

  int limit = 5;

  for (int i = 1; i <= limit; i++){
     fprintf(fichier, "%d\n", i);         // une ligne de données
  }
  fclose(fichier);
  return 0;
}
