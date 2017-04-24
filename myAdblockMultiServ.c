#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <string.h>
#include <errno.h>
#include <netdb.h>
#include "filtre.h"

#define BUFSIZE 1500

#define MAXLINE 1000000

/*
* get_host()
* fonction qui isole le header http (Host:)
*/
char * get_host(char * httpRequest){
	unsigned short i = 0, j = 0;
	char * buffer = strstr(httpRequest, "Host: " );
	char * host = (char *)malloc(256 * sizeof(char));
	while( buffer[i] != '\n' )
		i++;

	for(j = 6; j < i-1; j++)
		host[j - 6] = buffer[j];

	host[j-6+1] = '\0';
	return host;
}


int writen (fd, ptr, nbytes)
     int  fd;
     char *ptr;
     int nbytes;
{
  int nleft, nwritten;
  char *tmpptr;

  nleft = nbytes;
  tmpptr=ptr;
  while (nleft >0) {
    nwritten = write (fd,ptr, nleft);
    if (nwritten <=0) {
      if(errno == EINTR)
	nwritten=0;
      else{
	perror("probleme  dans write\n");
	return(-1);
      }
    }
    nleft -= nwritten;
    ptr += nwritten;
  }
  return (nbytes);
}


/*
 * Lire  "n" octets à partir d'un descripteur de socket
 */
int readn (fd, ptr, maxlen)
     int  fd;
     char *ptr;
     int maxlen;
{
  char *tmpptr;
  int nleft, nreadn;

  nleft = maxlen;
  tmpptr=ptr;

  while (nleft >0) {
    nreadn = read (fd,ptr, nleft);
    if (nreadn < 0) {
      if(errno == EINTR)
	nreadn=0;
      else{
	perror("readn : probleme  dans read \n");
	return(-1);
      }
    }
    else if(nreadn == 0){
      /* EOF */
      break ;
    }
    nleft -= nreadn;
    ptr += nreadn;
  }
  return (maxlen - nleft);
}

/*
 * Lire  une ligne terminee par \n à partir d'un descripteur de socket
 */
int readline (fd, ptr, maxlen)
     int  fd;
     char *ptr;
     int maxlen;
{

  int n, rc, retvalue, encore=1;  char c, *tmpptr;

  tmpptr=ptr;
  for (n=1; (n < maxlen) && (encore) ; n++) {
    if ( (rc = read (fd, &c, 1)) ==1) {
      *tmpptr++ =c;
      if (c == '\n')  /* fin de ligne atteinte */
	{encore =0; retvalue = n;}
    }
    else if (rc ==0) {  /* plus rien à lire */
      encore = 0;
      if (n==1) retvalue = 0;  /* rien a été lu */
      else retvalue = n;
    }
    else { /*rc <0 */
      if (errno != EINTR) {
	encore = 0;
	retvalue = -1;
      }
    }
  }
  *tmpptr = '\0';  /* pour terminer la ligne */
  return (retvalue);
}



int str_echo (int sockfd)
{
  int nrcv, nsnd;
  char msg[BUFSIZE];

  /*    * Attendre  le message envoye par le client
   */
  memset( (char*) msg, 0, sizeof(msg) );
  if ( (nrcv= read ( sockfd, msg, sizeof(msg)-1) ) < 0 )  {
    perror ("servmulti : : readn error on socket");
    exit (1);
  }
  msg[nrcv]='\0';
  printf ("servmulti :message recu=%s du processus %d nrcv = %d \n",msg,getpid(), nrcv);

  if ( (nsnd = write (sockfd, msg, nrcv) ) <0 ) {
    printf ("servmulti : writen error on socket");
    exit (1);
  }
  printf ("nsnd = %d \n", nsnd);
  return (nsnd);
} /* end of function */


usage(){
  printf("usage : servmulti numero_port_serveur\n");
}


int main (int argc,char *argv[])

{

  /*
    Création de la liste noir
  */

  Liste * liste;
  liste = creationBlackliste("./BlacklistTest.txt");

  int retread,s, sfd;

  char fromNav[MAXLINE];
  char fromServ[MAXLINE];
  char buf[MAXLINE];
  char host[MAXLINE];

  int sockfd, n, newsockfd, childpid, servlen,fin;
  struct sockaddr_in  serv_addr, cli_addr;
  socklen_t clilen;

 /* Verifier le nombre de param�tre en entr�e */
  if (argc != 2){
    usage();
    exit(1);}


/*
 * Ouvrir une socket (a TCP socket)
 */
if ((sockfd = socket(PF_INET, SOCK_STREAM, 0)) <0) {
   perror("servmulti : Probleme socket\n");
   exit (2);
 }


/*
 * Lier l'adresse  locale � la socket
 */
 memset( (char*) &serv_addr,0, sizeof(serv_addr) );
 serv_addr.sin_family = PF_INET;
 serv_addr.sin_addr.s_addr = htonl (INADDR_ANY);
 serv_addr.sin_port = htons(atoi(argv[1]));


 if (bind(sockfd,(struct sockaddr *)&serv_addr, sizeof(serv_addr) ) <0) {
   perror ("servmulti : erreur bind\n");
   exit (1);
 }

/* Param�trer le nombre de connexion "pending" */
 if (listen(sockfd, SOMAXCONN) <0) {
   perror ("servmulti : erreur listen\n");
   exit (1);
 }



	int tab_clients[FD_SETSIZE];
	int maxfdp1, i, nbfd, sockli;
	fd_set rset, pset;

	//initialisation

	maxfdp1 = sockfd+1;

	for (i=0; i<FD_SETSIZE; i+1){
		tab_clients[i]=-1;
	}
	FD_ZERO(&pset); FD_ZERO(&rset);
	FD_SET(sockfd, &pset);

 for (;;) {
	pset =rset;
	nbfd = select(maxfdp1, &pset, NULL,NULL,NULL);

	if (nbfd == -1){
		 perror("erreur : select");
	}

	//Tester si une demande connecter est emise par un client

	if (FD_ISSET(sockfd, &pset)){
		clilen = sizeof(cli_addr);
		newsockfd = accept(sockfd, (struct sockaddr *)&cli_addr, &clilen);

		//recherche une place libre ds le tableau

		i=0;
		while((i<FD_SETSIZE) && tab_clients[i]>=0) i++;

		if (i == FD_SETSIZE) exit(1);

		//Ajoute le nouveau client ds le tableau

		tab_clients[i] = newsockfd;

		//Ajouter le nouveau client � rset

		FD_SET(newsockfd, &rset);

		//mettre � jour maxfdp1

		if (newsockfd >=maxfdp1) { maxfdp1= maxfdp1+1;}

		//parcourir le tableau des clients connect�s

		i=0;
		while ((nbfd > 0) && (i<FD_SETSIZE)){
			if (((sockli = tab_clients[i]) >= 0)&& (FD_ISSET(sockli, &pset))){

        /*
          myAdblock
        */

        while((retread=recv(newsockfd,fromNav,sizeof(fromNav),(int)NULL))>0){

          //vérification de l'host
          int correctHost;
          correctHost = filtre(liste,fromNav);
          printf("correctHost : %d\n", correctHost);
          printf("fromNav: %s --- fin\n",fromNav );

          //récupération de l'adresse ip du serveur cherché
          struct addrinfo hints;
          struct addrinfo *result = NULL;
          struct addrinfo *rp;

          memset(&hints, 0, sizeof(struct addrinfo));
          hints.ai_family = AF_UNSPEC;    /* Allow IPv4 or IPv6 */
          hints.ai_socktype = SOCK_STREAM; /* Datagram socket */
          strcpy(host,get_host(fromNav));
          printf("%s\n", host);
          s = getaddrinfo("www.01net.com","80",&hints,&result);
          printf("%d",s);


          for (rp = result; rp != NULL; rp = rp->ai_next) {
              sfd = socket(rp->ai_family, rp->ai_socktype,rp->ai_protocol);
                  //création d'une socket
            if ((sfd = socket(rp->ai_family, rp->ai_socktype,
                    rp->ai_protocol)) == -1) {
                    perror("socket");
                    continue;
                }
            //connection avec le serveur
            if (connect(sfd, rp->ai_addr, rp->ai_addrlen) == -1) {
                  perror("connect");
              close(sfd);
              continue;
            }else{
              break;
            }


          }
          //on s'assure qu'on a au moins trouver une adresse à contacter
          if(rp==NULL){
            perror("erreur getaddrinfo");
            exit(1);
          }


          freeaddrinfo(result);//on en a plus besoin

          printf("\n envoi de la requête au serveur");

          n = send(sfd,fromNav,sizeof(fromNav),(int)NULL);
          if(n==-1){
            perror("probleme send");
            close(sockfd);
            close(newsockfd);
            close(sfd);
            exit(errno);
          }


          //reception du retour du serveur

          printf("\n récupération de la reponse du serveur");

          memset(fromServ,'\0',sizeof(fromServ));
          int i = 0;
          while((n=recv(sfd,fromServ,sizeof(fromServ),0)) > 0){
            fromServ[n] = '\0';

            if (correctHost !=0){
              strcpy(fromServ,"pub");
            }
            i++;
            printf("\n%s\n",fromServ);
            printf("\n\n%d",i);
            send(newsockfd,fromServ,sizeof(fromServ),0);
            memset(fromServ,'\0',sizeof(fromServ));
          }

          close(sfd);

          break;



        }

        /*
          fin myAdblock
        */

        /*if(str_echo(sockli) == 0){
					//le client demande une fermeture
					close(sockli);
					tab_clients[i] = -1;
					FD_CLR(sockli,&rset);
				}*/
				nbfd --;
			}
			i++;
		}
	}

   }

  close(newsockfd);
 }
