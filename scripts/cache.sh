#!/bin/bash

CACHE=../host_cacheSim

FILE=../traces/2D-5points-O1X256Y256Z0.outphy

echo "Executing: $CACHE -f $FILE"

$CACHE -f $FILE
