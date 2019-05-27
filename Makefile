#!/bin/sh
#
# STENCIL MEMORY TRACES SIMULATION
#
#
# ---------------------------------------

CC=gcc

CFLAGS := -std=c99 -I`pwd`/include/

SRC = ./src

all: stsim stsim_cache iaca_code
$(SRC)/pims_util.o:$(SRC)/pims_util.c
	$(CC) $(CFLAGS) -c -o $@ $<
$(SRC)/stsim.o:$(SRC)/stsim.c
	$(CC) $(CFLAGS) -c -o $@ $<
$(SRC)/stsim_cache.o:$(SRC)/stsim_cache.c
	$(CC) $(CFLAGS) -c -o $@ $<
$(SRC)/iaca.o:$(SRC)/iaca.c
	$(CC) $(CFLAGS) -c -o $@ $<

stsim: $(SRC)/stsim.o $(SRC)/pims_util.o
	$(CC) $(CFLAGS) -o $@ $(SRC)/stsim.o $(SRC)/pims_util.o -lm

stsim_cache: $(SRC)/stsim_cache.o $(SRC)/pims_util.o
	$(CC) $(CFLAGS) -o $@ $(SRC)/stsim_cache.o $(SRC)/pims_util.o -lm

iaca_code: $(SRC)/iaca.o
	$(CC) $(CFLAGS) -o $@ $(SRC)/iaca.o

clean:
	rm -Rf ./src/*.o

cleantra:
	rm -Rf ./traces/*
