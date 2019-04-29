#!/bin/bash

MMU=../mmu_simulator

CAPACITY=8
FILE=../traces/2D-13points-O3X1024Y1024Z0.out

echo "Executing: $MMU -c $CAPACITY -f $FILE"

$MMU -c $CAPACITY -f $FILE
