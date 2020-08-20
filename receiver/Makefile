# Tools & flags
CC=gcc
CFLAGS=--pedantic -Wall
LD=gcc
LDFLAGS=-g

OBJ = client.o

client:$(OBJ)
	$(LD) -o client $(OBJ) $(LDFLAGS)

client.o: client.c
	gcc -c client.c -o client.o $(CFLAGS)

## Other
clean:
	rm -f *.o $(EXEC) *~ client