#!/bin/bash

GENERATOR=../stencil
DIM_X=16
DIM_Y=0
DIM_Z=0
GRID_TYPE=double
# GRID_TYPE=Integer
STEN_TYPE=0
STEN_ORDER=1
STEN_COEFF=1
ITERATION=2
PIMS=0
CAPACITY=8
BSIZE=256
VAULTS=32
CACHESIZE=4194304

echo "Executing: $GENERATOR -x $DIM_X -y $DIM_Y -z $DIM_Z\
-t $GRID_TYPE -T $STEN_TYPE -O $STEN_ORDER -C $STEN_COEFF\
-i $ITERATION -p $PIMS -c $CAPACITY -b $BSIZE -v $VAULTS -a $CACHESIZE"

$GENERATOR -x $DIM_X -y $DIM_Y -z $DIM_Z -t $GRID_TYPE -T $STEN_TYPE\
 -O $STEN_ORDER -C $STEN_COEFF -i $ITERATION -p $PIMS -c $CAPACITY\
 -b $BSIZE -v $VAULTS -a $CACHESIZE
