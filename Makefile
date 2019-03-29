#!/bin/sh
#
# STENCIL MEMORY TRACES SIMULATION
#
#
# ---------------------------------------

#-- CC
#-- C COMPILER
CC=gcc

SRC = ./src

all: stencil_memory
$(SRC)/stencil.o: $(SRC)/stencil.c
	$(CC) -c -o $@ $<
$(SRC)/shiftamt.o: $(SRC)/shiftamt.c
	$(CC) -c -o $@ $<
$(SRC)/genrands.o: $(SRC)/genrands.c
	$(CC) -c -o $@ $<

stencil_memory: $(SRC)/stencil.o $(SRC)/shiftamt.o $(SRC)genrands.o
	$(CC) -o $@ $(SRC)/stencil.o $(SRC)/shiftamt.o $(SRC)genrands.o

clean:
	rm -Rf ./src/*.o
