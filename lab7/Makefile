CFLAGS = -c -Wall
CC = gcc
LIBS =  -lm 

all: symtab

symtab: main.o symtab.o debug.o semantics.o error.o
	${CC} main.o symtab.o debug.o semantics.o error.o -o symtab

main.o: main.c
	${CC} ${CFLAGS} main.c

symtab.o: symtab.c
	${CC} ${CFLAGS} symtab.c

debug.o: debug.c
	${CC} ${CFLAGS} debug.c

semantics.o: semantics.c
	${CC} ${CFLAGS} semantics.c

error.o: error.c
	${CC} ${CFLAGS} error.c

clean:
	rm -f *.o *~

