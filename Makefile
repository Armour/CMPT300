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
CFLAGS  = -Wall
# directory for source files
SDIR = src
# directory for *.o files
ODIR = obj
# directory for executable files
EDIR = bin
TARGET = $(EDIR)/decrypt
OBJECTS = $(ODIR)/decrypt.o

all : $(OBJECTS)
	@echo "--------- Generating executable file -----------"
	$(CC) $(CFLAGS) -o $(TARGET) $^
	@echo "--------- Compiling done! ----------------------"

$(ODIR)/%.o : $(SDIR)/%.c $(SDIR)/%.h
	@echo "--------- Start building source file -----------"
	$(CC) $(CFLAGS) -c $< -o $@

run: all
	$(TARGET)

.PHONY: clean
clean:
	@echo "---------- Start cleaning~ -----------"
	-rm -f $(ODIR)/*.o $(EDIR)/*
	@echo "---------- Cleaning done! ------------"
