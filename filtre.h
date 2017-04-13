#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <string.h>

typedef struct Element Element;
struct Element{
  char * badURL;
  Element *suivant;
};


typedef struct Liste Liste;
struct Liste{
  Element *premier;
};


int filtre(Liste *, char *);
Liste* creationBlackliste(char *);
/*Liste *initialisation();
void ajout(Liste *, char * );
void afficherListe(Liste *);
*/
