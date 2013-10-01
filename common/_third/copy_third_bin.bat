@echo off

SET _RELEASE_DIR=..\..\Release\
SET _DEBUG_DIR=..\..\Debug\

IF NOT EXIST %_RELEASE_DIR% MD %_RELEASE_DIR%
IF NOT EXIST %_DEBUG_DIR% MD %_DEBUG_DIR%

::set ver=gtest-1.6.0
set ver=gtest

echo gtest
copy %ver%\msvc\gtest\Release\gtest.lib %_RELEASE_DIR%
copy %ver%\msvc\gtest\Release\gtest_main.lib %_RELEASE_DIR%
copy %ver%\msvc\gtest\Debug\gtestd.lib %_DEBUG_DIR%
copy %ver%\msvc\gtest\Debug\gtest_maind.lib %_DEBUG_DIR%

SET _DEBUG_DIR=
SET _RELEASE_DIR=

PAUSE