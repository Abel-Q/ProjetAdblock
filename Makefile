
myAdblock : filtre.o myAdblock.c
	gcc -Wall myAdblock.c filtre.o -o myAdblock

#test : test.o filtre.o
#	gcc -Wall test.c filtre.o -o test
#
#test.o : test.c filtre.h
#	gcc -o test.o -c test.c -Wall

filtre.o :filtre.c
	gcc -o filtre.o -c filtre.c -Wall

clean :
	rm -rf *.o
