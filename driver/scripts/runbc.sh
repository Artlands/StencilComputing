#!/bin/bash

BC=../bandconflict_driver

TRACEFILE=/home/jieli/StencilComputing/traces/HOST-3D-7points-O2X64Y64Z64

echo "Executing : $BC -f $TRACEFILE"

$BC -f $TRACEFILE
