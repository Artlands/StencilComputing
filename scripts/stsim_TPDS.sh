#!/bin/bash

GENERATOR=../stsim_TPDS
DIM_X=256
DIM_Y=256
DIM_Z=256
STEN_ORDER=8
ITERATION=2
PIMS=0
CAPACITY=8
BSIZE=256
VAULTS=32
CACHE=32

echo "Executing: $GENERATOR -x $DIM_X -y $DIM_Y -z $DIM_Z -o $STEN_ORDER\
 -i $ITERATION -p $PIMS -c $CAPACITY -b $BSIZE -v $VAULTS"

 $GENERATOR -x $DIM_X -y $DIM_Y -z $DIM_Z -o $STEN_ORDER\
  -i $ITERATION -p $PIMS -c $CAPACITY -b $BSIZE -v $VAULTS
