#!/bin/bash

CACHE=../host_cacheSim
# CACHE_SIZE=32768    # Total cache size 32KB
# CACHE_SIZE=131072     # Total cache size 128KB
CACHE_SIZE=8388608    # Total cache size 8MB
BLOCK_SIZE=64
# BLOCK_SIZE=128
# BLOCK_SIZE=256
WAYS=8
FILE=../traces/PIMS-2D-13points-O3X2048Y2048Z0.phy

echo "Executing: $CACHE -c $CACHE_SIZE -b $BLOCK_SIZE -w $WAYS -f $FILE"

$CACHE -c $CACHE_SIZE -b $BLOCK_SIZE -w $WAYS -f $FILE
