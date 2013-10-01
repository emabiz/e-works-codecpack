#!/bin/sh
cd xvidcore
cd build/generic
./configure
make
cd =build
cp libxvidcore.def xvidcore.def
lib -machine:x86 -def:xvidcore.def
cp xvidcore.a $MINGBASE/lib/libxvidcore.a
cp xvidcore.dll xvidcore.lib $MINGBASE/bin/
cp ../../../src/xvid.h $MINGBASE/include/
cd ../../..