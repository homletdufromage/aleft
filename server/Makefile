# Tools & flags
CC=gcc
CFLAGS=--pedantic
LD=gcc
LDFLAGS=-g

OBJ = server.o

server:$(OBJ)
	$(LD) -o server $(OBJ) $(LDFLAGS)

server.o: server.c
	gcc -c server.c -o server.o $(CFLAGS)

## Other
clean:
	rm -f *.o $(EXEC) *~ server