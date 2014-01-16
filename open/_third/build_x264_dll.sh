#!/bin/sh
#libver=libx264-124
#libver=libx264-136
libver=libx264-138

cd x264-dll
./configure --disable-avs --enable-shared # does not support different directories 
echo dll creation...
make
cp $libver.def $libver.def.tmp
echo lib creation...
../../../common/_utility/def_remove_order.sh < $libver.def.tmp > $libver.def
lib -def:$libver.def -machine:x86                             
cp $libver.dll $libver.lib $MINGBASE/bin/