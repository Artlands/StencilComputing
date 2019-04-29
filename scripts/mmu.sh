#!/bin/bash

MMU=../mmu_simulator

CAPACITY=8
FILE=../traces/PIMS-2D-9points-O2X64Y64Z0

echo "Executing: $MMU -c $CAPACITY -f $FILE"

$MMU -c $CAPACITY -f $FILE
