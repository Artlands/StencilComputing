#!/bin/bash

MMU=../mmu_simulator

CAPACITY=8
FILE=../traces/3D-27points-X256Y256Z256.out

echo "Executing: $MMU -c $CAPACITY -f $FILE"

$MMU -c $CAPACITY -f $FILE
