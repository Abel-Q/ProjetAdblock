
myAdblock : filtre.o myAdblock.c
	gcc -Wall myAdblock.c filtre.o -o myAdblock

#myAdblockMultiServ : filtre.o myAdblockMultiServ.c
#	gcc -Wall myAdblockMultiServ.c filtre.o -o myAdblockMultiServ

filtre.o :filtre.c
	gcc -o filtre.o -c filtre.c -Wall

clean :
	rm -rf *.o
