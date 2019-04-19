#!/bin/bash

GENERATOR=../grid_generator
DIM_X=3
DIM_Y=3
DIM_Z=3
GRID_TYPE=double
CAPACITY=8
BSIZE=256
FILENAME=trace.out

echo "Executing: $GENERATOR -x $DIM_X -y $DIM_Y -z $DIM_Z -t $GRID_TYPE\
 -c $CAPACITY -b $BSIZE -f $FILENAME"

$GENERATOR -x $DIM_X -y $DIM_Y -z $DIM_Z -t $GRID_TYPE -c $CAPACITY -b $BSIZE\
 -f $FILENAME
