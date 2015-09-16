# ------------------------------------------------
# Makefile
#
#     Author: Armour Guo
#     Email : gc497052684@gmail.com
#     Date  : 2015-09-11
#
# ------------------------------------------------

CC = gcc
# -Wall turns on most compiler warnings
CFLAGS  = -Wall -DMEMWATCH -DMEMWATCH_STDIO
# directory for source files
SDIR = src
# directory for *.o files
ODIR = obj
# directory for executable files
EDIR = bin
TARGET = $(EDIR)/lyrebird
OBJECTS = $(ODIR)/lyrebird.o $(ODIR)/memwatch.o $(ODIR)/line_io.o $(ODIR)/decrypt.o

.PHONY: all prep build clean

all: prep build
	
build: $(OBJECTS)
	@echo "--------------- Generating executable file ----------------"
	$(CC) $(CFLAGS) -o $(TARGET) $^

$(ODIR)/%.o : $(SDIR)/%.c $(SDIR)/%.h
	$(CC) $(CFLAGS) -c $< -o $@

prep:
	@echo "--------------- Compiling source file ---------------------"

clean:
	-rm -f $(ODIR)/*.o $(EDIR)/* *.log

