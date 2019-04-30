#!/bin/bash

MMU=../mmu_simulator

CAPACITY=8
FILE=../traces/PIMS-2D-13points-O3X2048Y2048Z0

echo "Executing: $MMU -c $CAPACITY -f $FILE"

$MMU -c $CAPACITY -f $FILE
