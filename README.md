# ProjetAdblock

myAdblock est un proxy bloqueur de publicité.
Ici le proxy aura à la fois le rôle du serveur, lorsqu’il réceptionne la requête du navigateur et lorsqu’il lui renvoie la page,
et de client, lorsqu’il prend contact avec le serveur et qu’il réceptionne la page web.

---- Compilation ----

Un Makefile est disponible, utiliser la commande make.
Si vous souhaitez exécuter le proxy mono-client, assurez vous que les lignes pour compiler le proxy multi-client soit mise en commentaire et que celle pour le mono-client ne le soit pas.
Si vous souhaitez exécuter le proxy multi-client, réalisez le processus inverse. 

---- Configuration ----

Dans les paramètres de connection du navigateur selectionner une configuration manuelle du proxy.
Dans http proxy renseigner l'adresse ip : 127.0.0.1 et le port 8080

---- Fonctionnalités ----

- Une version mono-client disponible et fonctionnelle.

- Une version mulit-client disponible et fonctionnelle.

---- Test ----

- http://www.01net.com/
- 
