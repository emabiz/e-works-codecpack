#!/bin/sh
cd faac-1.28
mkdir _build_open
cd _build_open
../configure --disable-shared --disable-dependency-tracking --without-mp4v2
make
DESTDIR=$MINGBASE make install prefix=
cd ..