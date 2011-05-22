@echo off
echo Setting kitserver compile environment
@call "c:\vs2008ee\vc\vcvarsall.bat"
set DXSDK=c:\dxsdk_jun2010
set WX=c:\wxWidgets-2.9.1
set INCLUDE=%INCLUDE%;%DXSDK%\include;.\mfc;%WX%\include;%WX%\include\msvc
set LIB=%LIB%;%DXSDK%\lib\x86
echo Environment set

