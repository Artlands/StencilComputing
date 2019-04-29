#!/bin/bash

MMU=../mmu_simulator

CAPACITY=8
FILE=../traces/2D-21points-O5X1024Y1024Z0

echo "Executing: $MMU -c $CAPACITY -f $FILE"

$MMU -c $CAPACITY -f $FILE
