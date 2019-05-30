#!/bin/bash

BC=../bankconflict_driver

TRACEFILE=/home/jieli/StencilComputing/traces/PIMS-3D-37points-O12X64Y64Z64

echo "Executing : $BC -f $TRACEFILE"

$BC -f $TRACEFILE
