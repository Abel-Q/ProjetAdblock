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
#include <unistd.h>
#include "filtre.h"

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

/*char * get_get(char * httpRequest){

}*/

int main(int argc, char** argv){
	int serverSocket,clientSocket,sfd;/*socket d'écoute et de dialogue*/
	int n,retread,clilen,s;

	struct sockaddr_in serv_addr,cli_addr;
	char fromNav[MAXLINE];
	char fromServ[MAXLINE];

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

	if(listen(serverSocket,SOMAXCONN) <0){
		perror("servecho: erreur listen\n");
		exit(1);
	}

	clilen = sizeof(cli_addr);
	clientSocket = accept(serverSocket,(struct sockaddr *) &cli_addr, (socklen_t *)&clilen);
	if(clientSocket <0){
		perror("servecho : erreur accept \n");
		exit(1);
	}

	while((retread=recv(clientSocket,fromNav,sizeof(fromNav),0))>0){
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

		char host[MAXLINE];
		memset(&host,0,MAXLINE);
		unsigned short k = 0,j=0;
		char *buffer = strstr(fromNav,"Host: ");

		while(buffer[k] != '\n')
			k++;
	
		for(j = 6; j<k-1;j++)
			host[j-6] = buffer[j];

		host[j-6+1] = '\0';
		printf("host : %s\n", host);
		
		s = getaddrinfo(host,"80",&hints,&result);

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
		n = send(sfd,fromNav,sizeof(fromNav),0);
		if(n==-1){
			perror("probleme send");
			close(serverSocket);
			close(clientSocket);
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
			send(clientSocket,fromServ,sizeof(fromServ),0);
			memset(fromServ,'\0',sizeof(fromServ));
		}
		close(sfd);
		break;
	}
	//fin while
	close(serverSocket);
	close(clientSocket);
}
