#!/bin/bash

GENERATOR=../grid_generator
DIM_X=128
DIM_Y=128
DIM_Z=0
GRID_TYPE=double
# GRID_TYPE=Integer
STEN_TYPE=0
STEN_ORDER=3
STEN_COEFF=1
PIMS=0
CAPACITY=8
BSIZE=256
VAULTS=32

echo "Executing: $GENERATOR -x $DIM_X -y $DIM_Y -z $DIM_Z\
-t $GRID_TYPE -T $STEN_TYPE -O $STEN_ORDER -C $STEN_COEFF\
-p $PIMS -c $CAPACITY -b $BSIZE -v $VAULTS"

$GENERATOR -x $DIM_X -y $DIM_Y -z $DIM_Z -t $GRID_TYPE -T $STEN_TYPE\
 -O $STEN_ORDER -C $STEN_COEFF -p $PIMS -c $CAPACITY -b $BSIZE -v $VAULTS
