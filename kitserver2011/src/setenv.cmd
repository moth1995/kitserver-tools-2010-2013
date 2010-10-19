@echo off
echo Setting kitserver compile environment
rem @call "c:\program files\Microsoft Visual Studio 9.0\vc\vcvarsall.bat"
@call "c:\vc2008ee\vc\vcvarsall.bat"
rem set DXSDK=c:\dxsdk90c
set DXSDK=c:\dxsdk_june2010
set INCLUDE=%INCLUDE%;%DXSDK%\include;.\mfc
set LIB=%LIB%;%DXSDK%\lib\x86
echo Environment set

