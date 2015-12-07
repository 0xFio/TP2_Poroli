CC=gcc -std=gnu99 -Wall 

all: townGrutschli

townGrutschli: main.o bank.o
	$(CC) -o townGrutschli main.o bank.o -lpthread -lrt -O3

main.o: main.c bank.o
	$(CC) -c main.c

bank.o:
	$(CC) -c bank/bank.c

clean: 
	rm -f *.o conv
	echo Clean done
