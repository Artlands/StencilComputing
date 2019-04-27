#!/bin/sh
#
# STENCIL MEMORY TRACES SIMULATION
#
#
# ---------------------------------------

CC=gcc

CFLAGS := -std=c99 -I`pwd`/include/

SRC = ./src

all: grid_generator mmu_simulator

grid_generator: $(SRC)/grid_generator.o $(SRC)/shiftamt.o $(SRC)/pimsid.o
	$(CC) $(CFLAGS) -o $@ $(SRC)/grid_generator.o $(SRC)/shiftamt.o $(SRC)/pimsid.o

mmu_simulator: $(SRC)/mmu_simulator.o
	$(CC) $(CFLAGS) -o $@ $(SRC)/mmu_simulator.o

clean:
	rm -Rf ./src/*.o

cleanphy:
	rm -Rf ./traces/*.outphy

cleanvir:
	rm -Rf ./traces/*.out

cleanlog:
	rm -Rf ./traces/*.log
