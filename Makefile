#!/bin/sh
#
# STENCIL MEMORY TRACES SIMULATION
#
#
# ---------------------------------------

CC=gcc

CFLAGS := -std=c99 -I`pwd`/include/

SRC = ./src

all: stsim
$(SRC)/pims_util.o:$(SRC)/pims_util.c
	$(CC) $(CFLAGS) -c -o $@ $<
$(SRC)/stsim.o:$(SRC)/stsim.c
	$(CC) $(CFLAGS) -c -o $@ $<

stsim: $(SRC)/stsim.o $(SRC)/pims_util.o
	$(CC) $(CFLAGS) -o $@ $(SRC)/stsim.o $(SRC)/pims_util.o -lm

clean:
	rm -Rf ./src/*.o

cleantra:
	rm -Rf ./traces/*
