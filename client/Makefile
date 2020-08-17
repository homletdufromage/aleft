# Tools & flags
CC=gcc
CFLAGS=--pedantic
LD=gcc
LDFLAGS=-g

OBJ = aleft.o

# STEGANO
aleft:$(OBJ)
	$(LD) -o aleft $(OBJ) $(LDFLAGS)

aleft.o: aleft.c
	gcc -c aleft.c -o aleft.o $(CFLAGS)

## Other
clean:
	rm -f *.o $(EXEC) *~ aleft