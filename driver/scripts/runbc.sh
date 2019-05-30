#!/bin/bash

BC=../bankconflict_driver

TRACEFILE=/home/jieli/StencilComputing/traces/PIMS-3D-25points-O8X64Y64Z64

echo "Executing : $BC -f $TRACEFILE"

$BC -f $TRACEFILE
