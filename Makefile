# ------------------------------------------------
# Generic Makefile
#
# Author: gc497052684@gmail.com
# Date  : 2015-09-11
#
# ------------------------------------------------

CC = gcc
SDIR = src
ODIR = obj
EDIR = bin
EXE = $(EDIR)/decrypt
OBJECTS = $(ODIR)/decrypt.o

all : $(OBJECTS)
	$(CC) -o $(EXE) $^

$(ODIR)/decrypt.o : $(SDIR)/decrypt.c $(SDIR)/decrypt.h
	$(CC) -c $< -o $@

run: all
	$(EXE)

.PHONY: clean
	
clean:
	-rm -f $(ODIR)/*.o
	-rm -f $(EDIR)/*
