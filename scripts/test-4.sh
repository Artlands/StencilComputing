#!/bin/bash

STENCIL=../stencil_memory


BSIZE=256
CAPACITY=4
NUMDIVS=1
STENTYPE=4
SIZEX=500
SIZEY=500
SIZEZ=0
FILENAME=trace-4.out

echo "Executing: $STENCIL -b $BSIZE -c $CAPACITY -d $NUMDIVS -t $STENTYPE\
 -x $SIZEX -y $SIZEY -z $SIZEZ -F $FILENAME"

$STENCIL -b $BSIZE -c $CAPACITY -d $NUMDIVS -t $STENTYPE -x $SIZEX -y $SIZEY\
 -z $SIZEZ -F $FILENAME
