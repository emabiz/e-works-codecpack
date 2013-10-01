@echo off

SET _RELEASE_DIR=..\..\Release\
SET _DEBUG_DIR=..\..\Debug\

IF NOT EXIST %_RELEASE_DIR% MD %_RELEASE_DIR%
IF NOT EXIST %_DEBUG_DIR% MD %_DEBUG_DIR%

echo pthread
copy _inst_base\bin\libpthread-2.dll %_RELEASE_DIR%
copy _inst_base\bin\libpthread-2.dll %_DEBUG_DIR% 

echo ffmpeg-open
copy _inst_base\bin\avcodec-53.lib %_RELEASE_DIR%
copy _inst_base\bin\avcodec-53.lib %_DEBUG_DIR%
copy _inst_base\bin\avcodec-53.dll %_RELEASE_DIR%
copy _inst_base\bin\avcodec-53.dll %_DEBUG_DIR%
copy _inst_base\bin\avformat-53.lib %_RELEASE_DIR%
copy _inst_base\bin\avformat-53.lib %_DEBUG_DIR%
copy _inst_base\bin\avformat-53.dll %_RELEASE_DIR%
copy _inst_base\bin\avformat-53.dll %_DEBUG_DIR%
copy _inst_base\bin\avutil-51.lib %_RELEASE_DIR%
copy _inst_base\bin\avutil-51.lib %_DEBUG_DIR%
copy _inst_base\bin\avutil-51.dll %_RELEASE_DIR%
copy _inst_base\bin\avutil-51.dll %_DEBUG_DIR%
copy _inst_base\bin\swscale-2.lib %_RELEASE_DIR%
copy _inst_base\bin\swscale-2.lib %_DEBUG_DIR%
copy _inst_base\bin\swscale-2.dll %_RELEASE_DIR%
copy _inst_base\bin\swscale-2.dll %_DEBUG_DIR%

echo x264
copy _inst_base\bin\libx264-136.lib %_RELEASE_DIR%
copy _inst_base\bin\libx264-136.dll %_RELEASE_DIR%
copy _inst_base\bin\libx264-136.lib %_DEBUG_DIR%
copy _inst_base\bin\libx264-136.dll %_DEBUG_DIR%

echo xvid
copy _inst_base\bin\xvidcore.dll %_RELEASE_DIR%
copy _inst_base\bin\xvidcore.lib %_RELEASE_DIR%
copy _inst_base\bin\xvidcore.dll %_DEBUG_DIR%
copy _inst_base\bin\xvidcore.lib %_DEBUG_DIR%

SET _DEBUG_DIR=
SET _RELEASE_DIR=

PAUSE
