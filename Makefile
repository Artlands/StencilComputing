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

grid_generator: $(SRC)/grid_generator.o $(SRC)/pims_util.o
	$(CC) $(CFLAGS) -o $@ $(SRC)/grid_generator.o $(SRC)/pims_util.o

mmu_simulator: $(SRC)/mmu_simulator.o $(SRC)/pims_util.o
	$(CC) $(CFLAGS) -o $@ $(SRC)/mmu_simulator.o $(SRC)/pims_util.o

clean:
	rm -Rf ./src/*.o

cleanphy:
	rm -Rf ./traces/*.outphy

cleanvir:
	rm -Rf ./traces/*.out

cleanlog:
	rm -Rf ./traces/*.log
