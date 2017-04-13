#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <string.h>
#include "filtre.h"

  //Liste * liste = creationBlackliste("./BlacklistTest.txt");
  //filtre

  int main(int argc, char const *argv[]) {
    Liste * liste;
    liste = creationBlackliste("./BlacklistTest.txt");
    //afficherListe(liste);

    int res;
    res = filtre(liste, "truc.fr/&popunder=");
    printf("res = %d\n",res);
    return 0;
  }
