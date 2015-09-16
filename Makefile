# ------------------------------------------------
# Makefile
#
#     Author: Armour Guo
#     Email : gc497052684@gmail.com
#     Date  : 2015-09-11
#
# ------------------------------------------------

CC = gcc
CFLAGS  = -DMEMWATCH -DMEMWATCH_STDIO
TARGET = lyrebird
OBJECTS = lyrebird.o memwatch.o line_io.o decrypt.o

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
	-rm -f lyrebird
	-rm -f *.o
	-rm -f *.log

