# ------------------------------------------------
# Generic Makefile
#
# Author: gc497052684@gmail.com
# Date  : 2015-09-11
#
# ------------------------------------------------

all : obj/decrypt.o
	gcc obj/decrypt.o -o bin/decrypt

obj/decrypt.o : src/decrypt.h src/decrypt.c
	gcc -c src/decrypt.c -o obj/decrypt.o

run: all
	bin/decrypt

clean:
	rm obj/*.o
	rm bin/*
