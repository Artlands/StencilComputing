#!/bin/sh
#
# HMCSIM DRIVER FOR PIMS MEMORY TRACES
#
#
# ---------------------------------------

CFLAGS = -I/home/jieli/ham-sim/gc64-hmcsim/include/

CC = gcc

LIBS = -L/home/jieli/ham-sim/gc64-hmcsim -lhmcsim

SRC=./src

all: pims_driver

pims_driver: $(SRC)/pims_driver.o
	$(CC) $(CFLAGS) -o $@ $(SRC)/pims_driver.c $(LIBS)

clean:
	rm -Rf ./pims_driver ./src/*.o
