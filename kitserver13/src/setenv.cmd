@echo off
echo Setting kitserver compile environment
@call "c:\vs2008ee\vc\vcvarsall.bat"
set DXSDK=c:\dxsdk_jun2010
set INCLUDE=%INCLUDE%;%DXSDK%\include;.\mfc
set LIB=%LIB%;%DXSDK%\lib\x86
echo Environment set

