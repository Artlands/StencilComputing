#!/bin/bash

MMU=../mmu_simulator

CAPACITY=8
FILE=../traces/PIMS-1D-3points-O1X512Y0Z0.out

echo "Executing: $MMU -c $CAPACITY -f $FILE"

$MMU -c $CAPACITY -f $FILE
