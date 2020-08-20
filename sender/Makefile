# Tools & flags
CC=gcc
CFLAGS=--pedantic
LD=gcc
LDFLAGS=-g

OBJ = sender.o

sender:$(OBJ)
	$(LD) -o sender $(OBJ) $(LDFLAGS)

sender.o: sender.c sender.h
	gcc -c sender.c -o sender.o $(CFLAGS)

## Other
clean:
	rm -f *.o $(EXEC) *~ sender