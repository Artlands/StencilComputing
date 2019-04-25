#!/bin/bash

MMU=../mmu_simulator

CAPACITY=8
FILE=../traces/2D-17points-O4X2048Y2048Z0.out

echo "Executing: $MMU -c $CAPACITY -f $FILE"

$MMU -c $CAPACITY -f $FILE
