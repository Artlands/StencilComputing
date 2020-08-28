#!/bin/sh
#
# STENCIL MEMORY TRACES SIMULATION
#
#
# ---------------------------------------

CC=gcc

CFLAGS := -std=c99 -I`pwd`/include/

SRC = ./src

all: stsim_TPDS_3D_27 
$(SRC)/pims_util.o:$(SRC)/pims_util.c
	$(CC) $(CFLAGS) -c -o $@ $<
$(SRC)/stsim_TPDS_3D_27.o:$(SRC)/stsim_TPDS_3D_27.c
	$(CC) $(CFLAGS) -c -o $@ $<

# stsim: $(SRC)/stsim.o $(SRC)/pims_util.o
# 	$(CC) $(CFLAGS) -o $@ $(SRC)/stsim.o $(SRC)/pims_util.o -lm

stsim_TPDS_3D_27: $(SRC)/stsim_TPDS_3D_27.o $(SRC)/pims_util.o
	$(CC) $(CFLAGS) -o $@ $(SRC)/stsim_TPDS_3D_27.o $(SRC)/pims_util.o -lm

# stsim_TPDS: $(SRC)/stsim_TPDS.o $(SRC)/pims_util.o
# 	$(CC) $(CFLAGS) -o $@ $(SRC)/stsim_TPDS.o $(SRC)/pims_util.o -lm

# stsim_cache_TPDS: $(SRC)/stsim_cache_TPDS.o $(SRC)/pims_util.o
# 	$(CC) $(CFLAGS) -o $@ $(SRC)/stsim_cache_TPDS.o $(SRC)/pims_util.o -lm

# stsim_cache: $(SRC)/stsim_cache.o $(SRC)/pims_util.o
# 	$(CC) $(CFLAGS) -o $@ $(SRC)/stsim_cache.o $(SRC)/pims_util.o -lm

clean:
	rm -Rf ./src/*.o

cleantra:
	rm -Rf ./traces/*
