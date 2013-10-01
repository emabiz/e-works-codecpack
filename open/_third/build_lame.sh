#!/bin/sh
cd lame-3.99.5
mkdir _build_open
cd _build_open
../configure --enable-nasm --disable-shared
# Note: -j 2 does not work
make 
DESTDIR=$MINGBASE make install prefix=
cd ..