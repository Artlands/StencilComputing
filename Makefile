#!/bin/sh
#
# STENCIL MEMORY TRACES SIMULATION
#
#
# ---------------------------------------

CC=gcc

# CFLAGS = -I`pwd`/include -std=c99

CFLAGS = -std=c99

SRC = ./src

all: grid_generator mmu_simulator

grid_generator: $(SRC)/grid_generator.o $(SRC)/shiftamt.o $(SRC)/pimsid.o
	$(CC) $(CFLAGS) -o $@ $(SRC)/grid_generator.o $(SRC)/shiftamt.o $(SRC)/pimsid.o

mmu_simulator: $(SRC)/mmu_simulator.o
	$(CC) $(CFLAGS) -o $@ $(SRC)/mmu_simulator.o

clean:
	rm -Rf ./src/*.o

cleantrace:
	rm -Rf ./traces/*.out
