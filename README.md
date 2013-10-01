e-works-codecpack
=================

authors:
Emanuele Bizzarri <emabiz76@gmail.com>

Yuri Valentini <yuroller@gmail.com>

release date: 2013.09.30


Introduction
------------
e-works open codeckpack is the open source collection of codecs used to extend e-works platform functionalities.

The codecpack can be used by:
1. e-works phone/gateway as endpoint codecs. These codecs can be used in addition to the closed source video codecs provided with the videoconferencing platform.
Codecs have to be compiled as Microsoft Windows DLLs. The videoconferencing platform provides settings to specify the location of additional codecs (see videoconferencing platform documentation).

In this release the following codecs are enabled:
h264

2. e-works autore for recompressing a recorded videoconference.
ffmpeg library is used to perform recompression of recorded video conferences, so it has to be compiled with support to desired codecs.
Recompression performed by autore is configured through a set of configuration files, describing containers, audio and video codecs, conversion presets (see videoconferencing platform documentation).

In this release the following ffmpeg codec are enabled:
mpeg4
h263
h263p
flv1
libfaac
libmp3lame
libx264
libxvid
wmv2
wmav2
pcm_s16le

and the following ffmpeg muxers:
asf
avi
flv
mp4
mov
mp3
adts(aac container)


License
--------------------------
GPLv2
See LICENSE file

Setup
--------------------------
You can download setup from release section


1. Source tree:
--------------------------
inside ewct\dll\src there are some folders:

dir src\Release contains unofficial release compiled dlls
use src\copy_dll_to_official_open.bat to copy unofficial dlls to official open folder

dir src\Debug contains unofficial debug compiled dlls
use src\copy_dll_to_official_open.bat to copy unofficial dlls to official open folder



2. Building tools
--------------------------
2.1. Visual Studio 2010 Express
Used by:
video_open.sln


2.2. Git-1.8.3-preview20130601.exe
Used by:


2.3. mingw-get-inst-20120426.exe
website: http://www.mingw.org

Used by:
libx264
faac
lame
xvidcore
ffmpeg

Launch mingw installer
Select "Download latest repository catalogues" and additional packages:
    C++ Compiler
    MinGW Developer ToolKit
Open file c:\MinGW\msys\1.0\msys.bat and add line:
    call "C:\Program Files (x86)\Microsoft Visual Studio 10.0\VC\bin\vcvars32.bat"
at the top.


2.4. yasm-1.2.0-win32.exe
website: http://www.tortall.net/projects/yasm
Rename yasm executable to yasm.exe and copy it into c:\tools (add c:\tools to PATH env variable)
Used by:
xvidcore
libx264


2.5 isetup-5.5.3-unicode.exe


3. Build video dlls
--------------------------
video dlls depend on libx264, ffmpeg and gtest, so compilation order must be followed

Notes:
1.Steps from 3.2 to 3.3.5 (both included) can be automatically done using:
./build_codecpack_auto.sh

2.pthreads website: http://sourceware.org/pthreads-win32


3.1. Build gtest
--------------------------
Update googletest from repository inside src\common\_third\gtest:
    "svn checkout http://googletest.googlecode.com/svn/trunk/ googletest-read-only"
Open and convert common\_third\gtest\msvc\gtest.sln solution with Visual Studio

Edit following properties:   
   Debug
     C\C++> Code generation> Runtime library="Multi-threaded Debug DLL (/MDd)" for all projects        
   
Build solution in Debug and Release configurations
Launch common/_third/copy_third_bin.bat


3.2. Build libx264 (ewh264.dll)
--------------------------
website: http://www.videolan.org/developers/x264.html
Extract _third/_download/x264-snapshot-20120508-2245.tar.bz2 into _third/x264-snapshot-20120508-2245-dll

Modify "configure" file in section "generate config files" (line 1171):
            #echo 'IMPLIBNAME=libx264.dll.a' >> config.mak
            #echo "SOFLAGS=-shared -Wl,--out-implib,\$(IMPLIBNAME) -Wl,--enable-auto-image-base $SOFLAGS" >> config.mak
            echo "IMPLIBNAME=libx264-$API.lib" >> config.mak            
            echo "DEFNAME=libx264-$API.def" >> config.mak
            echo 'SOFLAGS=-shared -Wl,--out-implib,\$(IMPLIBNAME) -Wl,--output-def,$(DEFNAME) -Wl,--enable-auto-image-base' >> config.mak


Open mingw shell, go to src/open/_third
Launch ". ./build__settings.sh" (don't forget sourcing)
Launch ./build_x264.sh


3.3. Build ffmpeg
--------------------------
ffmpeg depends on some codec libraries, so compilation order must be followed


3.3.1. Build faac
--------------------------
website: http://www.audiocoding.com/faac.html
Open mingw shell, go to src/open/third
Launch ". ./build__settings.sh" (don't forget sourcing)
Launch ./build_faac.sh


3.3.2. Build lame
--------------------------
website: http://lame.sourceforge.net
Open mingw shell, go to src/open/third
Launch ". ./build__settings.sh" (don't forget sourcing)
Launch ./build_lame.sh


3.3.3. Build xvid
--------------------------
website: http://www.xvid.org
Remove -mno-cygwin param from xvidcore\build\generic\configure
Open mingw shell, go to src/open/third
Launch ". ./build__settings.sh" (don't forget sourcing)
Launch ./build_xvid.sh


3.3.4. Build libx264 (ffmpeg)
--------------------------
website: http://www.videolan.org/developers/x264.html
Extract _third/_download/x264-snapshot-20120508-2245.tar.bz2 into _third/x264-snapshot-20120508-2245
Open mingw shell, go to src/open/_third
Launch ". ./build__settings.sh" (don't forget sourcing)
Launch ./build_x264.sh


3.3.5. Build ffmpeg
--------------------------
website: http://www.ffmpeg.org/download.html
snapshot: src\base\_third\ffmpeg-0.10.3

produced dlls:
avcodec-53.dll
avformat-53.dll
avutil-51.dll
swscale-2.dll


Edit "configure" and add -lpthread support:
  "enabled libx264    && require  libx264 x264.h x264_encoder_encode -lx264 -lm -lpthread &&"

Launch ". ./build__settings.sh" (don't forget sourcing)
Launch ./build_ffmpeg.sh


3.3.6. Copy ffmpeg dependencies
--------------------------
Launch ./copy_third_bin.bat


3.4. Build video dlls
--------------------------
produced dlls:
ewh263.dll
ewh264.dll
ewxvid.dll
msinttypes: http://code.google.com/p/msinttypes

Open src\open\video\video_base.sln
Build solution in Debug and Release configurations

3.4. Build codec pack setup
--------------------------
Launch build_codecpack_setup.bat


