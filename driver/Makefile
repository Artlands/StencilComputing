#!/bin/sh
#
# HMCSIM DRIVER FOR PIMS MEMORY TRACES
#
#
# ---------------------------------------

# CFLAGS = -I/home/jieli/ham-sim/gc64-hmcsim/include/

CC = gcc

# LIBS = -L/home/jieli/ham-sim/gc64-hmcsim -lhmcsim

SRC=./src

all: bankconflict_driver

bankconflict_driver: $(SRC)/bankconflict_driver.o
	$(CC) -o $@ $(SRC)/bankconflict_driver.c

clean:
	rm -Rf ./bankconflict_driver ./src/*.o
