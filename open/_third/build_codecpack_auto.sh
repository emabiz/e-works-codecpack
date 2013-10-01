#!/bin/sh
. ./build__settings.sh
./build_x264_dll.sh
./build_faac.sh
./build_lame.sh
./build_xvid.sh
./build_x264.sh
./build_ffmpeg.sh