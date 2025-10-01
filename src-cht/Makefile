CC=gcc
CFLAGS=-g -O2

default: cht
all: cht

cht:	cht.c
	$(CC) $(CFLAGS) cht.c -o cht

lint:	cht.c
	splint -weak +posixlib cht.c >splint.log

clean:
	rm -f cht cht.o

distclean:
	rm -f cht cht.o Makefile

install:

