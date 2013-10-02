#!/bin/sh
dir=_inst_base

if [ ! -d "$dir" ]; then
    mkdir $dir
fi

cd $dir
export MINGBASE=`pwd`

dir=$MINGBASE/bin/

if [ ! -d "$dir" ]; then
    mkdir $dir
fi

dir=$MINGBASE/lib/

if [ ! -d "$dir" ]; then
    mkdir $dir
fi

dir=$MINGBASE/include/

if [ ! -d "$dir" ]; then
    mkdir $dir
fi

cd ..
cp ./_download/pthreadGC2.dll $MINGBASE/bin/