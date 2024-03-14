CC = gcc
CFLAGS = -Wall -Werror -g

all: spchk

spchk.o : spchk.c
	$(CC) $(CFLAGS) -c spchk.c -o spchk.o

spchk : spchk.o
	$(CC) $(CFLAGS) spchk.o -o spchk

clean:
	rm *.o spchk