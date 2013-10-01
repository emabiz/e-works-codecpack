#!/bin/sh
cd x264
# does not support different directories
./configure --disable-avs --enable-static 
make
DESTDIR=$MINGBASE make install prefix=