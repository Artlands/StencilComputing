#!/bin/bash

STENCIL=../stencil_memory


BSIZE=256
CAPACITY=4
NUMDIVS=1
NUMVAULTS=32
STENTYPE=2
SIZEX=1000
SIZEY=0
SIZEZ=0
FILENAME=trace-2.out

echo "Executing: $STENCIL -b $BSIZE -c $CAPACITY -d $NUMDIVS -t $STENTYPE\
 -v $NUMVAULTS -x $SIZEX -y $SIZEY -z $SIZEZ -F $FILENAME"

$STENCIL -b $BSIZE -c $CAPACITY -d $NUMDIVS -t $STENTYPE -v $NUMVAULTS\
 -x $SIZEX -y $SIZEY -z $SIZEZ -F $FILENAME
