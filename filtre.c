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

Liste *initialisation(){
  Liste *liste = malloc(sizeof(*liste));
  Element *element = malloc(sizeof(*element));

  if (liste == NULL){
    exit(EXIT_FAILURE);
  }
  element->badURL = "liste des URL";
  element -> suivant = NULL;
  liste -> premier = element;

  return liste;

}

void ajout(Liste *liste, char * text){

  Element *nouveau = malloc(sizeof(*nouveau));

  if(liste == NULL || nouveau == NULL){
    exit(EXIT_FAILURE);
  }

  nouveau->badURL = (char *)malloc(strlen(text)+1);

  //nouveau->badURL = "truc";
  strcpy(nouveau->badURL, text);
  nouveau->suivant = NULL;

  Element *actuel = liste->premier;
  while (actuel->suivant != NULL) {
    actuel = actuel->suivant;
  }
  actuel -> suivant = nouveau;

}

void afficherListe(Liste *liste){
  if (liste == NULL){
    exit(EXIT_FAILURE);
  }
  Element *actuel = liste->premier;
  while (actuel != NULL) {
    printf("%s\n",actuel->badURL );
    actuel = actuel->suivant;
  }
}

Liste * creationBlackliste(char * fichier){
    FILE * blacklist;
    //blacklist = fopen("./Blacklist.txt","r");
    blacklist = fopen(fichier,"r");

    Liste * liste = initialisation();
    char * badURLTemp;
    while (feof(blacklist) == 0){
      badURLTemp= (char *)malloc(100000);
      fscanf(blacklist,"%s\n",badURLTemp);
      char * badURL;
      badURL = (char *)malloc(strlen(badURLTemp)+1);
      strcpy(badURL, badURLTemp);
      //printf("sizeof(badURL) = %lu, sizeof(badURLTemp) = %lu\n",sizeof(badURL), sizeof(badURLTemp));
      ajout(liste,badURL);
      //printf("%s\n", badURL);
      free(badURLTemp);
      free(badURL);

    }
    //afficherListe(liste);
    printf("Blacklist cree\n");
    return liste;
}

int filtre(Liste * liste, char * URL){
  if (liste == NULL){
    exit(EXIT_FAILURE);
  }
  Element *actuel = liste->premier;
  while (actuel != NULL) {
    char * result = strstr(URL,actuel->badURL);
    if (result != NULL){
      return -1;
    }
    actuel = actuel->suivant;
  }
  return 0;
}
