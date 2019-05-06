#!/bin/sh
#
# STENCIL MEMORY TRACES SIMULATION
#
#
# ---------------------------------------

CC=gcc

CFLAGS := -std=c99 -I`pwd`/include/

SRC = ./src

all: finite
$(SRC)/pims_util.o:$(SRC)/pims_util.c
	$(CC) $(CFLAGS) -c -o $@ $<
$(SRC)/finite.o:$(SRC)/finite.c
	$(CC) $(CFLAGS) -c -o $@ $<

finite: $(SRC)/finite.o $(SRC)/pims_util.o
	$(CC) $(CFLAGS) -o $@ $(SRC)/finite.o $(SRC)/pims_util.o -lm

clean:
	rm -Rf ./src/*.o

cleantra:
	rm -Rf ./traces/*
