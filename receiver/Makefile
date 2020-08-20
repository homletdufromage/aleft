# Tools & flags
CC=gcc
CFLAGS=--pedantic -Wall
LD=gcc
LDFLAGS=-g

OBJ = receiver.o

receiver:$(OBJ)
	$(LD) -o receiver $(OBJ) $(LDFLAGS)

receiver.o: receiver.c
	gcc -c receiver.c -o receiver.o $(CFLAGS)

## Other
clean:
	rm -f *.o $(EXEC) *~ receiver