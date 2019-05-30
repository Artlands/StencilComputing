#!/bin/bash

GENERATOR=../stsim
DIM_X=64
DIM_Y=64
DIM_Z=64
STEN_ORDER=4
ITERATION=2
PIMS=0
CAPACITY=8
BSIZE=256
VAULTS=32

echo "Executing: $GENERATOR -x $DIM_X -y $DIM_Y -z $DIM_Z -o $STEN_ORDER\
 -i $ITERATION -p $PIMS -c $CAPACITY -b $BSIZE -v $VAULTS"

 $GENERATOR -x $DIM_X -y $DIM_Y -z $DIM_Z -o $STEN_ORDER\
  -i $ITERATION -p $PIMS -c $CAPACITY -b $BSIZE -v $VAULTS
