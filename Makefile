#!/bin/sh
#
# STENCIL MEMORY TRACES SIMULATION
#
#
# ---------------------------------------

CC=gcc

CFLAGS := -std=c99 -I`pwd`/include/

SRC = ./src

all: grid_generator mmu_simulator host_cacheSim
$(SRC)/pims_util.o:$(SRC)/pims_util.c
	$(CC) $(CFLAGS) -c -o $@ $<
$(SRC)/grid_generator.o:$(SRC)/grid_generator.c
	$(CC) $(CFLAGS) -c -o $@ $<
$(SRC)/mmu_simulator.o:$(SRC)/mmu_simulator.c
	$(CC) $(CFLAGS) -c -o $@ $<
$(SRC)/host_cacheSim.o:$(SRC)/host_cacheSim.c
	$(CC) $(CFLAGS) -c -o $@ $<

grid_generator: $(SRC)/grid_generator.o $(SRC)/pims_util.o
	$(CC) $(CFLAGS) -o $@ $(SRC)/grid_generator.o $(SRC)/pims_util.o

mmu_simulator: $(SRC)/mmu_simulator.o $(SRC)/pims_util.o
	$(CC) $(CFLAGS) -o $@ $(SRC)/mmu_simulator.o $(SRC)/pims_util.o -lm

host_cacheSim: $(SRC)/host_cacheSim.o $(SRC)/pims_util.o
	$(CC) $(CFLAGS) -o $@ $(SRC)/host_cacheSim.o $(SRC)/pims_util.o -lm

clean:
	rm -Rf ./src/*.o

cleantra:
	rm -Rf ./traces/*
