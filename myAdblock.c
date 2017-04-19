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

#define MAXLINE 10000

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

/*char * get_get(char * httpRequest){

}*/

int main(int argc, char** argv){
	int serverSocket,clientSocket,sfd;/*socket d'écoute et de dialogue*/
	int n,retread,clilen,childpid,servlen,s;

	struct sockaddr_in serv_addr,cli_addr;
	char fromNav[MAXLINE];
	char fromServ[MAXLINE];
	char* host;

	/*
		Création de la liste noir
	*/

	Liste * liste;
	liste = creationBlackliste("./BlacklistTest.txt");

	/*
		Ouvrir une socket (a tcp socket)
	*/

	if((serverSocket=socket(AF_INET,SOCK_STREAM,0))<0){
		perror("servecho : probleme socket \n");
		exit(2);
	}


	/*
		On lie la socket a l'adresse
	*/
	bzero((char *) &serv_addr, sizeof(serv_addr));
	int portno = 8080;
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	serv_addr.sin_port = htons(portno);
	if (bind(serverSocket, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
              perror("ERROR on binding");
		exit(1);
	}


	//listen et cfg du nb de connexion pending
	if(listen(serverSocket,SOMAXCONN) <0){
		perror("servecho: erreur listen\n");
		exit(1);
	}


	//Accept

	//la structure cli_add permettra de recuperer les donnees du client (adresse ip et port)
	clilen = sizeof(cli_addr);
	clientSocket = accept(serverSocket,(struct sockaddr *) &cli_addr, (socklen_t *)&clilen);
	if(clientSocket <0){
		perror("servecho : erreur accept \n");
		exit(1);
	}

	while((retread=recv(clientSocket,fromNav,sizeof(fromNav),(int)NULL))>0){
		printf("corr: %s",fromNav);
		host = get_host(fromNav);
		printf("%s\n", host);

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

		s = getaddrinfo(host,"80",&hints,&result);
		if (s != 0) {
			fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(s));
			exit(EXIT_FAILURE);
	    	}

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
			}

        		break;
		}
		//on s'assure qu'on a au moins trouver une adresse à contacter
		if(rp==NULL){
			perror("Could not bind");
			exit(1);
		}
		freeaddrinfo(result);//on en a plus besoin
		//envoie de la requête au serveur

		n = send(sfd,fromNav,sizeof(fromNav),(int)NULL);
		if(n==-1){
			perror("probleme send");
			close(serverSocket);
			close(clientSocket);
			close(sfd);
			exit(errno);
		}
		//reception du retour du serveur
		if((n = recv(sfd, fromServ,sizeof(fromServ), (int)NULL)) < 0)
		{
   			perror("recv()");
			close(serverSocket);
			close(clientSocket);
			close(sfd);
			exit(errno);
		}
		printf("fromServ : %s\n",fromServ);

		//modification du paquet envoyer au navigateur si pub
		if (correctHost !=0){
			strcpy(fromServ,"pub");
		}

		//renvoi du retour serveur au navigateur
		if((n = send(clientSocket,fromServ,sizeof(fromServ),(int)NULL) < 0)){
			perror("send()");
			close(serverSocket);
			close(clientSocket);
			close(sfd);
			exit(errno);
		}
		close(sfd);

	}
	close(serverSocket);
	close(clientSocket);


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
