#!/bin/bash

MMU=../mmu_simulator

CAPACITY=8
FILE=../traces/2D-5points-O1X256Y256Z0.out

echo "Executing: $MMU -c $CAPACITY -f $FILE"

$MMU -c $CAPACITY -f $FILE
