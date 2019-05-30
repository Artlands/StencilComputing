#!/bin/sh
#
# HMCSIM DRIVER FOR PIMS MEMORY TRACES
#
#
# ---------------------------------------

# CFLAGS = -I/home/jieli/ham-sim/gc64-hmcsim/include/

CC = gcc

CFLAGS := -std=c99
# LIBS = -L/home/jieli/ham-sim/gc64-hmcsim -lhmcsim

SRC=./src

all: bankconflict_driver
$(SRC)/bankconflict_driver.o:$(SRC)/bankconflict_driver.c
	$(CC) $(CFLAGS) -c -o $@ $<

bankconflict_driver: $(SRC)/bankconflict_driver.o
	$(CC) $(CFLAGS) -o $@ $(SRC)/bankconflict_driver.o

clean:
	rm -Rf ./bankconflict_driver ./src/*.o
