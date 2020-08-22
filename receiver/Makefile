# Tools & flags
CC=gcc
CFLAGS=--pedantic -Wall
LD=gcc
LDFLAGS=-g

OBJ = receiver.o

receiver:main.c receiver.o
	$(LD) -o receiver main.c $(OBJ) $(LDFLAGS)

receiver.o: receiver.c receiver.h
	gcc -c receiver.c -o receiver.o $(CFLAGS)

## Other
clean:
	rm -f *.o $(EXEC) *~ receiver