#!/bin/sh
#
# STENCIL MEMORY TRACES SIMULATION
#
#
# ---------------------------------------

CC=gcc

# CFLAGS = -I`pwd`/include -std=c99

CFLAGS = -std=c99

SRC = ./src

all: grid_generator
$(SRC)/grid_generator.o: $(SRC)/grid_generator.c
	$(CC) $(CFLAGS) -c -o $@ $<

grid_generator: $(SRC)/grid_generator.o

# all: stencil_memory
# $(SRC)/stencil.o: $(SRC)/stencil.c
# 	$(CC) $(CFLAGS) -c -o $@ $<
# $(SRC)/shiftamt.o: $(SRC)/shiftamt.c
# 	$(CC) -c -o $@ $<
# $(SRC)/pimsid.o: $(SRC)/pimsid.c
# 	$(CC) -c -o $@ $<
# $(SRC)/genrands.o: $(SRC)/genrands.c
# 	$(CC) -c -o $@ $<
# $(SRC)/interpradd.o: $(SRC)/interpradd.c
# 	$(CC) -c -o $@ $<
# $(SRC)/coalesce.o: $(SRC)/coalesce.c
# 	$(CC) $(CFLAGS) -c -o $@ $<
#
# stencil_memory: $(SRC)/stencil.o $(SRC)/shiftamt.o $(SRC)/pimsid.o $(SRC)/genrands.o $(SRC)/interpradd.o $(SRC)/coalesce.o
# 	$(CC) -o $@ $(SRC)/stencil.o $(SRC)/shiftamt.o $(SRC)/pimsid.o $(SRC)/genrands.o $(SRC)/interpradd.o $(SRC)/coalesce.o

clean:
	rm -Rf ./src/*.o
