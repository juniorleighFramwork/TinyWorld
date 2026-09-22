#include <stdio.h>
#include <stdlib.h>
#include <time.h>

// Ecris un programme qui tire 10 nombres aléatoires (0, 1) et les affcihe, un par ligne.

int main(void) {

  srand(time(NULL)); // une seule a la fois au debut

  int nbr_alea = 10;
  //int random_nbr = rand() % 2;

  for (int i = 0; i < nbr_alea; i++) {
      int random_nbr = rand() % 2;
      printf("%d\n", random_nbr);
  }
  return 0;
}
