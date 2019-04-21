#!/bin/bash

GENERATOR=../grid_generator
DIM_X=2
DIM_Y=2
DIM_Z=0
GRID_TYPE=double
# GRID_TYPE=Integer
CAPACITY=8
BSIZE=256
FILENAME=trace.out

echo "Executing: $GENERATOR -x $DIM_X -y $DIM_Y -z $DIM_Z -t $GRID_TYPE\
 -c $CAPACITY -b $BSIZE -f $FILENAME"

$GENERATOR -x $DIM_X -y $DIM_Y -z $DIM_Z -t $GRID_TYPE -c $CAPACITY -b $BSIZE\
 -f $FILENAME
