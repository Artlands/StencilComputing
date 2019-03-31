#!/bin/sh
#
# STENCIL MEMORY TRACES SIMULATION
#
#
# ---------------------------------------

CC=gcc

SRC = ./src

all: stencil_memory
$(SRC)/stencil.o: $(SRC)/stencil.c
	$(CC) -c -o $@ $<
$(SRC)/shiftamt.o: $(SRC)/shiftamt.c
	$(CC) -c -o $@ $<
$(SRC)/genrands.o: $(SRC)/genrands.c
	$(CC) -c -o $@ $<
$(SRC)/interpradd.o: $(SRC)/interpradd.c
	$(CC) -c -o $@ $<

stencil_memory: $(SRC)/stencil.o $(SRC)/shiftamt.o $(SRC)/genrands.o $(SRC)/interpradd.o
	$(CC) -o $@ $(SRC)/stencil.o $(SRC)/shiftamt.o $(SRC)/genrands.o $(SRC)/interpradd.o

cleanout:
	rm -Rf ./scripts/*.out
clean:
	rm -Rf ./src/*.o
