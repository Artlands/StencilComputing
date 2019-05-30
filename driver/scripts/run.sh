#!/bin/bash

PIMSPOWER=../pims_driver

TRACEFILE=/home/jieli/StencilComputing/traces/HOST-3D-7points-O2X64Y64Z64

CFILE=../config.pims

LD_LIBRARY_PATH=/home/jieli/ham-sim/gc64-hmcsim

export LD_LIBRARY_PATH

echo "Executing : $PIMSPOWER -f $TRACEFILE -C $CFILE"

$PIMSPOWER -f $TRACEFILE -C $CFILE
