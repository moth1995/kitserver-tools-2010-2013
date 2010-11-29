# For release builds, use "debug=1" in command line. For instance,
# to build DLLs in release mode: nmake dlls debug=1

CC=cl
LINK=link
RC=rc

INCPNGDIB=soft\pngdib
LPPNGDIB=soft\pngdib 
LPLIBPNG=soft\libpng-1.2.8-lib\lib
LPZLIB=soft\zlib123-dll\lib
LIBPNGDLL=soft\libpng-1.2.8-bin\libpng13.dll 
ZLIBDLL=soft\zlib123-dll\zlib1.dll
D3DX9=d3dx9.lib

!if "$(debug)"=="1"
EXTRA_CFLAGS=/DDEBUG
!else
EXTRA_CFLAGS=/DMYDLL_RELEASE_BUILD
!endif

# 4731: warning about ebp modification
CFLAGS=/nologo /Od /EHsc /wd4731 $(EXTRA_CFLAGS)
LFLAGS=/NOLOGO
LIBS=user32.lib gdi32.lib advapi32.lib comctl32.lib
LIBSDLL=pngdib.obj libpng.lib zdll.lib $(LIBS)


all: dlls apps
	if exist output (rd /S /Q output)
	mkdir output
	copy *.dll output
	copy $(LIBPNGDLL) output
	copy $(ZLIBDLL) output
	copy manager.exe output
	rem copy config.exe output
	copy lang_*.txt output
	
all_dlls: dlls
	if exist output (rd /S /Q output)
	mkdir output
	copy *.dll output
	copy $(LIBPNGDLL) output
	copy $(ZLIBDLL) output
	copy lang_*.txt output
	
all_apps: apps
	if exist output (rd /S /Q output)
	mkdir output
	copy manager.exe output
	rem copy config.exe output
	copy lang_*.txt output

pngdib:
	(cd soft\pngdib && nmake EXTRA_CFLAGS=$(EXTRA_CFLAGS))

#dlls: kload.dll kserv.dll lodmixer.dll kset.dll afsio.dll afs2fs.dll sched.dll fserv.dll rwdata.dll camera.dll time.dll speeder.dll stadium.dll bootserv.dll bserv.dll leagues.dll
#apps: manager.exe config.exe zlibtool.exe
dlls: kload.dll afsio.dll afs2fs.dll speeder.dll sides.dll
apps: manager.exe zlibtool.exe

imageutil.obj: imageutil.cpp imageutil.h
detect.obj: detect.cpp detect.h
lang.obj: lang.cpp lang.h
configs.obj: configs.cpp configs.h configs.hpp utf8.h
hooklib.obj: hooklib.cpp hooklib.h
log.obj: log.cpp log.h
crc32.obj: crc32.cpp crc32.h
hook.obj: hook.cpp hook.h hook_addr.h
apihijack.obj: apihijack.cpp apihijack.h
kload.lib: kload.obj hook.obj
kload.obj: kload.h kload_addr.h kload.cpp

afs2fs.lib: afs2fs.obj names.obj configs.obj
afs2fs.dll: afs2fs.lib afsio.lib afs2fs.res kload.lib
	$(LINK) $(LFLAGS) /out:afs2fs.dll /DLL afs2fs.obj names.obj configs.obj afs2fs.res $(LIBS) $(D3DX9) kload.lib afsio.lib
afs2fs.obj: afs2fs.cpp afs2fs.h afs2fs_addr.h afs2fs.rc
names.obj: names.cpp names.h

afsio.lib: afsio.obj configs.obj apihijack.obj
afsio.dll: afsio.lib afsio.res kload.lib
	$(LINK) $(LFLAGS) /out:afsio.dll /DLL afsio.obj configs.obj afsio.res $(LIBS) $(D3DX9) kload.lib apihijack.obj
afsio.obj: afsio.cpp afsio.h afsio_addr.h afsio.rc

speeder.lib: speeder.obj configs.obj apihijack.obj
speeder.dll: speeder.lib speeder.res
	$(LINK) $(LFLAGS) /out:speeder.dll /DLL speeder.obj configs.obj apihijack.obj speeder.res $(LIBS) $(D3DX9) kload.lib
speeder.obj: speeder.cpp speeder.h speeder_addr.h speeder.rc

sides.lib: sides.obj configs.obj
sides.dll: sides.lib sides.res
	$(LINK) $(LFLAGS) /out:sides.dll /DLL sides.obj configs.obj sides.res $(LIBS) $(D3DX9) kload.lib
sides.obj: sides.cpp sides.h sides_addr.h sides.rc

kload.lib: hook.obj kload.obj detect.obj imageutil.obj lang.obj log.obj configs.obj hooklib.obj hook.obj crc32.obj
kload.dll: kload.lib kload.res
	$(LINK) $(LFLAGS) /out:kload.dll /DLL kload.obj detect.obj imageutil.obj lang.obj log.obj configs.obj hooklib.obj hook.obj crc32.obj kload.res $(LIBS) $(D3DX9) winmm.lib
kload.obj: kload.cpp kload.h

kload.res: 
	$(RC) -r -fo kload.res kload.rc
	
	
setupgui.obj: setupgui.cpp setupgui.h
manager.obj: manager.cpp manager.h setupgui.h
manager.exe: manager.obj detect.obj setupgui.obj imageutil.obj lang.obj configs.obj manager.res
	$(LINK) $(LFLAGS) /out:manager.exe manager.obj detect.obj setupgui.obj imageutil.obj lang.obj configs.obj manager.res $(LIBS) shell32.lib
manager.res: 
	$(RC) -r -fo manager.res manager.rc
	
zlibtool.obj: zlibtool.cpp
zlibtool.exe: zlibtool.obj 
	$(LINK) $(LFLAGS) /out:zlibtool.exe zlibtool.obj /LIBPATH:$(LPZLIB) $(LIBS) zdll.lib

afsexport.obj: afsexport.cpp afsreader.h
afsexport.exe: afsexport.obj afsreader.obj 
	$(LINK) $(LFLAGS) /out:afsexport.exe afsexport.obj afsreader.obj $(LIBS)

.cpp.obj:
	$(CC) $(CFLAGS) -c $(INC) /I $(INCPNGDIB) $<

clean:
	del /Q /F *.exp *.lib *.obj *.res *.dll *.exe
	if exist soft\pngdib (cd soft\pngdib && $(MAKE) clean)
	if exist output (rd /S /Q output)

clean-all:
	del /Q /F *.exp *.lib *.obj *.res *.dll *.exe *.log *~
	if exist soft\pngdib (cd soft\pngdib && $(MAKE) clean-all)
	if exist output (rd /S /Q output)
	
