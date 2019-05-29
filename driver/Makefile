#!/bin/sh
#
# HMCSIM DRIVER FOR PIMS MEMORY TRACES
#
#
# ---------------------------------------

CFLAGS = -I/home/jieli/gc64-hmcsim/include/ -I`pwd`/include/

CC = gcc

LIBS = -L/home/jieli/gc64-hmcsim -lhmcsim

SRC=./src

all: pims_driver.c

dmc_power: $(SRC)/pims_driver.o
	$(CC) $(CFLAGS) -o $@ $(SRC)/pims_driver.c $(LIBS)

clean:
	rm -Rf ./pims_driver ./src/*.o
