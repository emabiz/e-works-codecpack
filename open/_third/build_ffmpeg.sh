#!/bin/sh
cd ffmpeg-0.10.3
mkdir _build_open
cd _build_open
LDFLAGS="-L$MINGBASE/lib" CFLAGS="-I$MINGBASE/include" \
CFLAGS="-I$MINGBASE/include -fno-common" \
../configure --disable-stripping \
--disable-everything --disable-network --enable-gpl --enable-nonfree \
--enable-libfaac --enable-libmp3lame --enable-libx264 --enable-libxvid \
--enable-encoder=mpeg4,h263,h263p,flv,libfaac,libmp3lame,libx264,libxvid,wmv2,wmav2,pcm_s16le \
--enable-decoder=h264,h263 \
--enable-parser=h264,h263 \
--enable-muxer=asf,avi,flv,mp4,mov,mp3,adts \
--enable-protocol=file \
--enable-memalign-hack --enable-w32threads --enable-shared --disable-static --disable-debug
make

cd libavcodec
mv avcodec-53.def avcodec-53.def.tmp
../../../../../common/_utility/def_remove_order.sh < avcodec-53.def.tmp > avcodec-53.def
lib -machine:x86 -def:avcodec-53.def

cd ../libavformat
mv avformat-53.def avformat-53.def.tmp
../../../../../common/_utility/def_remove_order.sh < avformat-53.def.tmp > avformat-53.def
lib -machine:x86 -def:avformat-53.def

cd ../libavutil
mv avutil-51.def avutil-51.def.tmp
../../../../../common/_utility/def_remove_order.sh < avutil-51.def.tmp > avutil-51.def
lib -machine:x86 -def:avutil-51.def

cd ../libswscale
mv swscale-2.def swscale-2.def.tmp
../../../../../common/_utility/def_remove_order.sh < swscale-2.def.tmp > swscale-2.def
lib -machine:x86 -def:swscale-2.def

cd ..
DESTDIR=$MINGBASE make install prefix=

cd $MINGBASE/bin
mv avcodec.lib avcodec-53.lib
mv avdevice.lib avdevice-53.lib
mv avfilter.lib avfilter-2.lib
mv avformat.lib avformat-53.lib
mv avutil.lib avutil-51.lib
mv postproc.lib postproc-52.lib
mv swresample.lib swresample-0.lib
mv swscale.lib swscale-2.lib