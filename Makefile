# ------------------------------------------------
# Makefile
#
#   Project         : LyreBird
#   Name            : Guo Chong
#   Student ID      : 301295753
#   SFU username    : armourg
#   Lecture section : D1
#   Instructor      : Brain G.Booth
#   TA              : Scott Kristjanson
#
#   Created by Armour on 11/09/2015
#   Copyright (c) 2015 Armour. All rights reserved.
#
# ------------------------------------------------

CC = gcc
CFLAGS  = -DMEMWATCH -DMEMWATCH_STDIO
LIBS = -lm
TARGET = lyrebird
OBJECTS = lyrebird.o memwatch.o line_io.o dec_func.o decrypt.o pipe.o scheduling.o

.PHONY: all prep build clean

all: prep build
	
build: $(OBJECTS)
	@echo "--------------- Generating executable file ----------------"
	$(CC) $(CFLAGS) -o $(TARGET) $^ $(LIBS)

$%.o : $%.c $%.h
	$(CC) $(CFLAGS) -c $< -o $@

prep:
	@echo "--------------- Compiling source file ---------------------"

clean:
	-rm -f lyrebird
	-rm -f *.o
	-rm -f *.log
	-rm -f core

