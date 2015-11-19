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
TARGET_SERVER = lyrebird.server
TARGET_CLIENT = lyrebird.client
OBJECTS_SERVER = lyrebird.server.o memwatch.o
OBJECTS_CLIENT = lyrebird.client.o memwatch.o line_io.o dec_func.o decrypt.o pipe.o scheduling.o

.PHONY: all build_server build_client clean

all: build_server build_client
	
build_client: $(OBJECTS_CLIENT)
	$(CC) $(CFLAGS) -o $(TARGET_CLIENT) $^ $(LIBS)

build_server: $(OBJECTS_SERVER)
	$(CC) $(CFLAGS) -o $(TARGET_SERVER) $^ $(LIBS)

$%.o : $%.c $%.h
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	-rm -f lyrebird.server
	-rm -f lyrebird.client
	-rm -f *.o
	-rm -f *.log
	-rm -f core
