#!/bin/sh
#
# STENCIL MEMORY TRACES SIMULATION
#
#
# ---------------------------------------

CC=gcc

CFLAGS := -std=c99 -I`pwd`/include/

SRC = ./src

all: stsim stsim_TPDS stsim_cache 
$(SRC)/pims_util.o:$(SRC)/pims_util.c
	$(CC) $(CFLAGS) -c -o $@ $<
$(SRC)/stsim.o:$(SRC)/stsim.c
	$(CC) $(CFLAGS) -c -o $@ $<
$(SRC)/stsim_TPDS.o:$(SRC)/stsim_TPDS.c
	$(CC) $(CFLAGS) -c -o $@ $<
$(SRC)/stsim_cache.o:$(SRC)/stsim_cache.c
	$(CC) $(CFLAGS) -c -o $@ $<

stsim: $(SRC)/stsim.o $(SRC)/pims_util.o
	$(CC) $(CFLAGS) -o $@ $(SRC)/stsim.o $(SRC)/pims_util.o -lm

stsim_TPDS: $(SRC)/stsim_TPDS.o $(SRC)/pims_util.o
	$(CC) $(CFLAGS) -o $@ $(SRC)/stsim_TPDS.o $(SRC)/pims_util.o -lm

stsim_cache: $(SRC)/stsim_cache.o $(SRC)/pims_util.o
	$(CC) $(CFLAGS) -o $@ $(SRC)/stsim_cache.o $(SRC)/pims_util.o -lm

clean:
	rm -Rf ./src/*.o

cleantra:
	rm -Rf ./traces/*
