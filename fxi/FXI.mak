# Microsoft Developer Studio Generated NMAKE File, Based on FXI.dsp
!IF "$(CFG)" == ""
CFG=FXI - Win32 Debug
!MESSAGE No configuration specified. Defaulting to FXI - Win32 Debug.
!ENDIF 

!IF "$(CFG)" != "FXI - Win32 Release" && "$(CFG)" != "FXI - Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "FXI.mak" CFG="FXI - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "FXI - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "FXI - Win32 Debug" (based on "Win32 (x86) Console Application")
!MESSAGE 
!ERROR An invalid configuration is specified.
!ENDIF 

!IF "$(OS)" == "Windows_NT"
NULL=
!ELSE 
NULL=nul
!ENDIF 

CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "FXI - Win32 Release"

OUTDIR=.\..\bin
INTDIR=.\Release
# Begin Custom Macros
OutDir=.\..\bin
# End Custom Macros

ALL : "$(OUTDIR)\FXI.exe" "$(OUTDIR)\FXI.bsc"


CLEAN :
	-@erase "$(INTDIR)\dcbr.obj"
	-@erase "$(INTDIR)\dcbr.sbr"
	-@erase "$(INTDIR)\files.obj"
	-@erase "$(INTDIR)\files.sbr"
	-@erase "$(INTDIR)\fnc_exports.obj"
	-@erase "$(INTDIR)\fnc_exports.sbr"
	-@erase "$(INTDIR)\fxi.res"
	-@erase "$(INTDIR)\g_blit.obj"
	-@erase "$(INTDIR)\g_blit.sbr"
	-@erase "$(INTDIR)\g_console.obj"
	-@erase "$(INTDIR)\g_console.sbr"
	-@erase "$(INTDIR)\g_draw.obj"
	-@erase "$(INTDIR)\g_draw.sbr"
	-@erase "$(INTDIR)\g_flic.obj"
	-@erase "$(INTDIR)\g_flic.sbr"
	-@erase "$(INTDIR)\g_main.obj"
	-@erase "$(INTDIR)\g_main.sbr"
	-@erase "$(INTDIR)\g_maps.obj"
	-@erase "$(INTDIR)\g_maps.sbr"
	-@erase "$(INTDIR)\g_mode7.obj"
	-@erase "$(INTDIR)\g_mode7.sbr"
	-@erase "$(INTDIR)\g_pal.obj"
	-@erase "$(INTDIR)\g_pal.sbr"
	-@erase "$(INTDIR)\g_scroll.obj"
	-@erase "$(INTDIR)\g_scroll.sbr"
	-@erase "$(INTDIR)\g_systexts.obj"
	-@erase "$(INTDIR)\g_systexts.sbr"
	-@erase "$(INTDIR)\g_texts.obj"
	-@erase "$(INTDIR)\g_texts.sbr"
	-@erase "$(INTDIR)\i_debug.obj"
	-@erase "$(INTDIR)\i_debug.sbr"
	-@erase "$(INTDIR)\i_func.obj"
	-@erase "$(INTDIR)\i_func.sbr"
	-@erase "$(INTDIR)\i_main.obj"
	-@erase "$(INTDIR)\i_main.sbr"
	-@erase "$(INTDIR)\img_pcx.obj"
	-@erase "$(INTDIR)\img_pcx.sbr"
	-@erase "$(INTDIR)\img_png.obj"
	-@erase "$(INTDIR)\img_png.sbr"
	-@erase "$(INTDIR)\instance.obj"
	-@erase "$(INTDIR)\instance.sbr"
	-@erase "$(INTDIR)\main.obj"
	-@erase "$(INTDIR)\main.sbr"
	-@erase "$(INTDIR)\pathfind.obj"
	-@erase "$(INTDIR)\pathfind.sbr"
	-@erase "$(INTDIR)\png.obj"
	-@erase "$(INTDIR)\png.sbr"
	-@erase "$(INTDIR)\pngerror.obj"
	-@erase "$(INTDIR)\pngerror.sbr"
	-@erase "$(INTDIR)\pnggccrd.obj"
	-@erase "$(INTDIR)\pnggccrd.sbr"
	-@erase "$(INTDIR)\pngget.obj"
	-@erase "$(INTDIR)\pngget.sbr"
	-@erase "$(INTDIR)\pngmem.obj"
	-@erase "$(INTDIR)\pngmem.sbr"
	-@erase "$(INTDIR)\pngpread.obj"
	-@erase "$(INTDIR)\pngpread.sbr"
	-@erase "$(INTDIR)\pngread.obj"
	-@erase "$(INTDIR)\pngread.sbr"
	-@erase "$(INTDIR)\pngrio.obj"
	-@erase "$(INTDIR)\pngrio.sbr"
	-@erase "$(INTDIR)\pngrtran.obj"
	-@erase "$(INTDIR)\pngrtran.sbr"
	-@erase "$(INTDIR)\pngrutil.obj"
	-@erase "$(INTDIR)\pngrutil.sbr"
	-@erase "$(INTDIR)\pngset.obj"
	-@erase "$(INTDIR)\pngset.sbr"
	-@erase "$(INTDIR)\pngtrans.obj"
	-@erase "$(INTDIR)\pngtrans.sbr"
	-@erase "$(INTDIR)\pngvcrd.obj"
	-@erase "$(INTDIR)\pngvcrd.sbr"
	-@erase "$(INTDIR)\pngwio.obj"
	-@erase "$(INTDIR)\pngwio.sbr"
	-@erase "$(INTDIR)\pngwrite.obj"
	-@erase "$(INTDIR)\pngwrite.sbr"
	-@erase "$(INTDIR)\pngwtran.obj"
	-@erase "$(INTDIR)\pngwtran.sbr"
	-@erase "$(INTDIR)\pngwutil.obj"
	-@erase "$(INTDIR)\pngwutil.sbr"
	-@erase "$(INTDIR)\sound.obj"
	-@erase "$(INTDIR)\sound.sbr"
	-@erase "$(INTDIR)\strings.obj"
	-@erase "$(INTDIR)\strings.sbr"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\xctype.obj"
	-@erase "$(INTDIR)\xctype.sbr"
	-@erase "$(OUTDIR)\FXI.bsc"
	-@erase "$(OUTDIR)\FXI.exe"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

"$(INTDIR)" :
    if not exist "$(INTDIR)/$(NULL)" mkdir "$(INTDIR)"

CPP_PROJ=/nologo /MD /W3 /GX /O2 /I ".\inc" /I "..\include" /I "..\sdl\include" /I "..\libpng" /I "..\mikmod" /I "..\zlib" /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /FR"$(INTDIR)\\" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 
RSC_PROJ=/l 0xc0a /fo"$(INTDIR)\fxi.res" /d "NDEBUG" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\FXI.bsc" 
BSC32_SBRS= \
	"$(INTDIR)\g_blit.sbr" \
	"$(INTDIR)\g_console.sbr" \
	"$(INTDIR)\g_draw.sbr" \
	"$(INTDIR)\g_flic.sbr" \
	"$(INTDIR)\g_main.sbr" \
	"$(INTDIR)\g_maps.sbr" \
	"$(INTDIR)\g_mode7.sbr" \
	"$(INTDIR)\g_pal.sbr" \
	"$(INTDIR)\g_scroll.sbr" \
	"$(INTDIR)\g_systexts.sbr" \
	"$(INTDIR)\g_texts.sbr" \
	"$(INTDIR)\img_pcx.sbr" \
	"$(INTDIR)\img_png.sbr" \
	"$(INTDIR)\i_debug.sbr" \
	"$(INTDIR)\i_func.sbr" \
	"$(INTDIR)\i_main.sbr" \
	"$(INTDIR)\instance.sbr" \
	"$(INTDIR)\pathfind.sbr" \
	"$(INTDIR)\sound.sbr" \
	"$(INTDIR)\png.sbr" \
	"$(INTDIR)\pngerror.sbr" \
	"$(INTDIR)\pnggccrd.sbr" \
	"$(INTDIR)\pngget.sbr" \
	"$(INTDIR)\pngmem.sbr" \
	"$(INTDIR)\pngpread.sbr" \
	"$(INTDIR)\pngread.sbr" \
	"$(INTDIR)\pngrio.sbr" \
	"$(INTDIR)\pngrtran.sbr" \
	"$(INTDIR)\pngrutil.sbr" \
	"$(INTDIR)\pngset.sbr" \
	"$(INTDIR)\pngtrans.sbr" \
	"$(INTDIR)\pngvcrd.sbr" \
	"$(INTDIR)\pngwio.sbr" \
	"$(INTDIR)\pngwrite.sbr" \
	"$(INTDIR)\pngwtran.sbr" \
	"$(INTDIR)\pngwutil.sbr" \
	"$(INTDIR)\dcbr.sbr" \
	"$(INTDIR)\files.sbr" \
	"$(INTDIR)\fnc_exports.sbr" \
	"$(INTDIR)\main.sbr" \
	"$(INTDIR)\strings.sbr" \
	"$(INTDIR)\xctype.sbr"

"$(OUTDIR)\FXI.bsc" : "$(OUTDIR)" $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /incremental:no /pdb:"$(OUTDIR)\FXI.pdb" /machine:I386 /out:"$(OUTDIR)\FXI.exe" 
LINK32_OBJS= \
	"$(INTDIR)\g_blit.obj" \
	"$(INTDIR)\g_console.obj" \
	"$(INTDIR)\g_draw.obj" \
	"$(INTDIR)\g_flic.obj" \
	"$(INTDIR)\g_main.obj" \
	"$(INTDIR)\g_maps.obj" \
	"$(INTDIR)\g_mode7.obj" \
	"$(INTDIR)\g_pal.obj" \
	"$(INTDIR)\g_scroll.obj" \
	"$(INTDIR)\g_systexts.obj" \
	"$(INTDIR)\g_texts.obj" \
	"$(INTDIR)\img_pcx.obj" \
	"$(INTDIR)\img_png.obj" \
	"$(INTDIR)\i_debug.obj" \
	"$(INTDIR)\i_func.obj" \
	"$(INTDIR)\i_main.obj" \
	"$(INTDIR)\instance.obj" \
	"$(INTDIR)\pathfind.obj" \
	"$(INTDIR)\sound.obj" \
	"$(INTDIR)\png.obj" \
	"$(INTDIR)\pngerror.obj" \
	"$(INTDIR)\pnggccrd.obj" \
	"$(INTDIR)\pngget.obj" \
	"$(INTDIR)\pngmem.obj" \
	"$(INTDIR)\pngpread.obj" \
	"$(INTDIR)\pngread.obj" \
	"$(INTDIR)\pngrio.obj" \
	"$(INTDIR)\pngrtran.obj" \
	"$(INTDIR)\pngrutil.obj" \
	"$(INTDIR)\pngset.obj" \
	"$(INTDIR)\pngtrans.obj" \
	"$(INTDIR)\pngvcrd.obj" \
	"$(INTDIR)\pngwio.obj" \
	"$(INTDIR)\pngwrite.obj" \
	"$(INTDIR)\pngwtran.obj" \
	"$(INTDIR)\pngwutil.obj" \
	"$(INTDIR)\dcbr.obj" \
	"$(INTDIR)\files.obj" \
	"$(INTDIR)\fnc_exports.obj" \
	"$(INTDIR)\main.obj" \
	"$(INTDIR)\strings.obj" \
	"$(INTDIR)\xctype.obj" \
	"$(INTDIR)\fxi.res" \
	"..\LIB\zlib.lib" \
	"..\LIB\SDLmain.lib" \
	"..\LIB\SDL.lib" \
	"..\Lib\SDL_mixer.lib"

"$(OUTDIR)\FXI.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "FXI - Win32 Debug"

OUTDIR=.\..\bin\Debug
INTDIR=.\Debug
# Begin Custom Macros
OutDir=.\..\bin\Debug
# End Custom Macros

ALL : "$(OUTDIR)\FXI.exe" "$(OUTDIR)\FXI.bsc"


CLEAN :
	-@erase "$(INTDIR)\dcbr.obj"
	-@erase "$(INTDIR)\dcbr.sbr"
	-@erase "$(INTDIR)\files.obj"
	-@erase "$(INTDIR)\files.sbr"
	-@erase "$(INTDIR)\fnc_exports.obj"
	-@erase "$(INTDIR)\fnc_exports.sbr"
	-@erase "$(INTDIR)\fxi.res"
	-@erase "$(INTDIR)\g_blit.obj"
	-@erase "$(INTDIR)\g_blit.sbr"
	-@erase "$(INTDIR)\g_console.obj"
	-@erase "$(INTDIR)\g_console.sbr"
	-@erase "$(INTDIR)\g_draw.obj"
	-@erase "$(INTDIR)\g_draw.sbr"
	-@erase "$(INTDIR)\g_flic.obj"
	-@erase "$(INTDIR)\g_flic.sbr"
	-@erase "$(INTDIR)\g_main.obj"
	-@erase "$(INTDIR)\g_main.sbr"
	-@erase "$(INTDIR)\g_maps.obj"
	-@erase "$(INTDIR)\g_maps.sbr"
	-@erase "$(INTDIR)\g_mode7.obj"
	-@erase "$(INTDIR)\g_mode7.sbr"
	-@erase "$(INTDIR)\g_pal.obj"
	-@erase "$(INTDIR)\g_pal.sbr"
	-@erase "$(INTDIR)\g_scroll.obj"
	-@erase "$(INTDIR)\g_scroll.sbr"
	-@erase "$(INTDIR)\g_systexts.obj"
	-@erase "$(INTDIR)\g_systexts.sbr"
	-@erase "$(INTDIR)\g_texts.obj"
	-@erase "$(INTDIR)\g_texts.sbr"
	-@erase "$(INTDIR)\i_debug.obj"
	-@erase "$(INTDIR)\i_debug.sbr"
	-@erase "$(INTDIR)\i_func.obj"
	-@erase "$(INTDIR)\i_func.sbr"
	-@erase "$(INTDIR)\i_main.obj"
	-@erase "$(INTDIR)\i_main.sbr"
	-@erase "$(INTDIR)\img_pcx.obj"
	-@erase "$(INTDIR)\img_pcx.sbr"
	-@erase "$(INTDIR)\img_png.obj"
	-@erase "$(INTDIR)\img_png.sbr"
	-@erase "$(INTDIR)\instance.obj"
	-@erase "$(INTDIR)\instance.sbr"
	-@erase "$(INTDIR)\main.obj"
	-@erase "$(INTDIR)\main.sbr"
	-@erase "$(INTDIR)\pathfind.obj"
	-@erase "$(INTDIR)\pathfind.sbr"
	-@erase "$(INTDIR)\png.obj"
	-@erase "$(INTDIR)\png.sbr"
	-@erase "$(INTDIR)\pngerror.obj"
	-@erase "$(INTDIR)\pngerror.sbr"
	-@erase "$(INTDIR)\pnggccrd.obj"
	-@erase "$(INTDIR)\pnggccrd.sbr"
	-@erase "$(INTDIR)\pngget.obj"
	-@erase "$(INTDIR)\pngget.sbr"
	-@erase "$(INTDIR)\pngmem.obj"
	-@erase "$(INTDIR)\pngmem.sbr"
	-@erase "$(INTDIR)\pngpread.obj"
	-@erase "$(INTDIR)\pngpread.sbr"
	-@erase "$(INTDIR)\pngread.obj"
	-@erase "$(INTDIR)\pngread.sbr"
	-@erase "$(INTDIR)\pngrio.obj"
	-@erase "$(INTDIR)\pngrio.sbr"
	-@erase "$(INTDIR)\pngrtran.obj"
	-@erase "$(INTDIR)\pngrtran.sbr"
	-@erase "$(INTDIR)\pngrutil.obj"
	-@erase "$(INTDIR)\pngrutil.sbr"
	-@erase "$(INTDIR)\pngset.obj"
	-@erase "$(INTDIR)\pngset.sbr"
	-@erase "$(INTDIR)\pngtrans.obj"
	-@erase "$(INTDIR)\pngtrans.sbr"
	-@erase "$(INTDIR)\pngvcrd.obj"
	-@erase "$(INTDIR)\pngvcrd.sbr"
	-@erase "$(INTDIR)\pngwio.obj"
	-@erase "$(INTDIR)\pngwio.sbr"
	-@erase "$(INTDIR)\pngwrite.obj"
	-@erase "$(INTDIR)\pngwrite.sbr"
	-@erase "$(INTDIR)\pngwtran.obj"
	-@erase "$(INTDIR)\pngwtran.sbr"
	-@erase "$(INTDIR)\pngwutil.obj"
	-@erase "$(INTDIR)\pngwutil.sbr"
	-@erase "$(INTDIR)\sound.obj"
	-@erase "$(INTDIR)\sound.sbr"
	-@erase "$(INTDIR)\strings.obj"
	-@erase "$(INTDIR)\strings.sbr"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(INTDIR)\xctype.obj"
	-@erase "$(INTDIR)\xctype.sbr"
	-@erase "$(OUTDIR)\FXI.bsc"
	-@erase "$(OUTDIR)\FXI.exe"
	-@erase "$(OUTDIR)\FXI.ilk"
	-@erase "$(OUTDIR)\FXI.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

"$(INTDIR)" :
    if not exist "$(INTDIR)/$(NULL)" mkdir "$(INTDIR)"

CPP_PROJ=/nologo /MD /W3 /Gm /GX /ZI /Od /I ".\inc" /I "..\include" /I "..\sdl\include" /I "..\libpng" /I "..\mikmod" /I "..\zlib" /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /FR"$(INTDIR)\\" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 
RSC_PROJ=/l 0xc0a /fo"$(INTDIR)\fxi.res" /d "_DEBUG" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\FXI.bsc" 
BSC32_SBRS= \
	"$(INTDIR)\g_blit.sbr" \
	"$(INTDIR)\g_console.sbr" \
	"$(INTDIR)\g_draw.sbr" \
	"$(INTDIR)\g_flic.sbr" \
	"$(INTDIR)\g_main.sbr" \
	"$(INTDIR)\g_maps.sbr" \
	"$(INTDIR)\g_mode7.sbr" \
	"$(INTDIR)\g_pal.sbr" \
	"$(INTDIR)\g_scroll.sbr" \
	"$(INTDIR)\g_systexts.sbr" \
	"$(INTDIR)\g_texts.sbr" \
	"$(INTDIR)\img_pcx.sbr" \
	"$(INTDIR)\img_png.sbr" \
	"$(INTDIR)\i_debug.sbr" \
	"$(INTDIR)\i_func.sbr" \
	"$(INTDIR)\i_main.sbr" \
	"$(INTDIR)\instance.sbr" \
	"$(INTDIR)\pathfind.sbr" \
	"$(INTDIR)\sound.sbr" \
	"$(INTDIR)\png.sbr" \
	"$(INTDIR)\pngerror.sbr" \
	"$(INTDIR)\pnggccrd.sbr" \
	"$(INTDIR)\pngget.sbr" \
	"$(INTDIR)\pngmem.sbr" \
	"$(INTDIR)\pngpread.sbr" \
	"$(INTDIR)\pngread.sbr" \
	"$(INTDIR)\pngrio.sbr" \
	"$(INTDIR)\pngrtran.sbr" \
	"$(INTDIR)\pngrutil.sbr" \
	"$(INTDIR)\pngset.sbr" \
	"$(INTDIR)\pngtrans.sbr" \
	"$(INTDIR)\pngvcrd.sbr" \
	"$(INTDIR)\pngwio.sbr" \
	"$(INTDIR)\pngwrite.sbr" \
	"$(INTDIR)\pngwtran.sbr" \
	"$(INTDIR)\pngwutil.sbr" \
	"$(INTDIR)\dcbr.sbr" \
	"$(INTDIR)\files.sbr" \
	"$(INTDIR)\fnc_exports.sbr" \
	"$(INTDIR)\main.sbr" \
	"$(INTDIR)\strings.sbr" \
	"$(INTDIR)\xctype.sbr"

"$(OUTDIR)\FXI.bsc" : "$(OUTDIR)" $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /incremental:yes /pdb:"$(OUTDIR)\FXI.pdb" /debug /machine:I386 /out:"$(OUTDIR)\FXI.exe" /pdbtype:sept 
LINK32_OBJS= \
	"$(INTDIR)\g_blit.obj" \
	"$(INTDIR)\g_console.obj" \
	"$(INTDIR)\g_draw.obj" \
	"$(INTDIR)\g_flic.obj" \
	"$(INTDIR)\g_main.obj" \
	"$(INTDIR)\g_maps.obj" \
	"$(INTDIR)\g_mode7.obj" \
	"$(INTDIR)\g_pal.obj" \
	"$(INTDIR)\g_scroll.obj" \
	"$(INTDIR)\g_systexts.obj" \
	"$(INTDIR)\g_texts.obj" \
	"$(INTDIR)\img_pcx.obj" \
	"$(INTDIR)\img_png.obj" \
	"$(INTDIR)\i_debug.obj" \
	"$(INTDIR)\i_func.obj" \
	"$(INTDIR)\i_main.obj" \
	"$(INTDIR)\instance.obj" \
	"$(INTDIR)\pathfind.obj" \
	"$(INTDIR)\sound.obj" \
	"$(INTDIR)\png.obj" \
	"$(INTDIR)\pngerror.obj" \
	"$(INTDIR)\pnggccrd.obj" \
	"$(INTDIR)\pngget.obj" \
	"$(INTDIR)\pngmem.obj" \
	"$(INTDIR)\pngpread.obj" \
	"$(INTDIR)\pngread.obj" \
	"$(INTDIR)\pngrio.obj" \
	"$(INTDIR)\pngrtran.obj" \
	"$(INTDIR)\pngrutil.obj" \
	"$(INTDIR)\pngset.obj" \
	"$(INTDIR)\pngtrans.obj" \
	"$(INTDIR)\pngvcrd.obj" \
	"$(INTDIR)\pngwio.obj" \
	"$(INTDIR)\pngwrite.obj" \
	"$(INTDIR)\pngwtran.obj" \
	"$(INTDIR)\pngwutil.obj" \
	"$(INTDIR)\dcbr.obj" \
	"$(INTDIR)\files.obj" \
	"$(INTDIR)\fnc_exports.obj" \
	"$(INTDIR)\main.obj" \
	"$(INTDIR)\strings.obj" \
	"$(INTDIR)\xctype.obj" \
	"$(INTDIR)\fxi.res" \
	"..\LIB\zlib.lib" \
	"..\LIB\SDLmain.lib" \
	"..\LIB\SDL.lib" \
	"..\Lib\SDL_mixer.lib"

"$(OUTDIR)\FXI.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ENDIF 

.c{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.c{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<


!IF "$(NO_EXTERNAL_DEPS)" != "1"
!IF EXISTS("FXI.dep")
!INCLUDE "FXI.dep"
!ELSE 
!MESSAGE Warning: cannot find "FXI.dep"
!ENDIF 
!ENDIF 


!IF "$(CFG)" == "FXI - Win32 Release" || "$(CFG)" == "FXI - Win32 Debug"
SOURCE=.\src\g_blit.c

"$(INTDIR)\g_blit.obj"	"$(INTDIR)\g_blit.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\src\g_console.c

"$(INTDIR)\g_console.obj"	"$(INTDIR)\g_console.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\src\g_draw.c

"$(INTDIR)\g_draw.obj"	"$(INTDIR)\g_draw.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\src\g_flic.c

"$(INTDIR)\g_flic.obj"	"$(INTDIR)\g_flic.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\src\g_main.c

"$(INTDIR)\g_main.obj"	"$(INTDIR)\g_main.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\src\g_maps.c

"$(INTDIR)\g_maps.obj"	"$(INTDIR)\g_maps.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\src\g_mode7.c

"$(INTDIR)\g_mode7.obj"	"$(INTDIR)\g_mode7.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\src\g_pal.c

"$(INTDIR)\g_pal.obj"	"$(INTDIR)\g_pal.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\src\g_scroll.c

"$(INTDIR)\g_scroll.obj"	"$(INTDIR)\g_scroll.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\src\g_systexts.c

"$(INTDIR)\g_systexts.obj"	"$(INTDIR)\g_systexts.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\src\g_texts.c

"$(INTDIR)\g_texts.obj"	"$(INTDIR)\g_texts.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\src\img_pcx.c

"$(INTDIR)\img_pcx.obj"	"$(INTDIR)\img_pcx.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\src\img_png.c

"$(INTDIR)\img_png.obj"	"$(INTDIR)\img_png.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\src\i_debug.c

"$(INTDIR)\i_debug.obj"	"$(INTDIR)\i_debug.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\src\i_func.c

"$(INTDIR)\i_func.obj"	"$(INTDIR)\i_func.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\src\i_main.c

"$(INTDIR)\i_main.obj"	"$(INTDIR)\i_main.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\src\instance.c

"$(INTDIR)\instance.obj"	"$(INTDIR)\instance.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\src\pathfind.c

"$(INTDIR)\pathfind.obj"	"$(INTDIR)\pathfind.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\src\sound.c

"$(INTDIR)\sound.obj"	"$(INTDIR)\sound.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\libpng\png.c

"$(INTDIR)\png.obj"	"$(INTDIR)\png.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\libpng\pngerror.c

"$(INTDIR)\pngerror.obj"	"$(INTDIR)\pngerror.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\libpng\pnggccrd.c

"$(INTDIR)\pnggccrd.obj"	"$(INTDIR)\pnggccrd.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\libpng\pngget.c

"$(INTDIR)\pngget.obj"	"$(INTDIR)\pngget.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\libpng\pngmem.c

"$(INTDIR)\pngmem.obj"	"$(INTDIR)\pngmem.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\libpng\pngpread.c

"$(INTDIR)\pngpread.obj"	"$(INTDIR)\pngpread.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\libpng\pngread.c

"$(INTDIR)\pngread.obj"	"$(INTDIR)\pngread.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\libpng\pngrio.c

"$(INTDIR)\pngrio.obj"	"$(INTDIR)\pngrio.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\libpng\pngrtran.c

"$(INTDIR)\pngrtran.obj"	"$(INTDIR)\pngrtran.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\libpng\pngrutil.c

"$(INTDIR)\pngrutil.obj"	"$(INTDIR)\pngrutil.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\libpng\pngset.c

"$(INTDIR)\pngset.obj"	"$(INTDIR)\pngset.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\libpng\pngtrans.c

"$(INTDIR)\pngtrans.obj"	"$(INTDIR)\pngtrans.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\libpng\pngvcrd.c

"$(INTDIR)\pngvcrd.obj"	"$(INTDIR)\pngvcrd.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\libpng\pngwio.c

"$(INTDIR)\pngwio.obj"	"$(INTDIR)\pngwio.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\libpng\pngwrite.c

"$(INTDIR)\pngwrite.obj"	"$(INTDIR)\pngwrite.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\libpng\pngwtran.c

"$(INTDIR)\pngwtran.obj"	"$(INTDIR)\pngwtran.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\libpng\pngwutil.c

"$(INTDIR)\pngwutil.obj"	"$(INTDIR)\pngwutil.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\src\dcbr.c

"$(INTDIR)\dcbr.obj"	"$(INTDIR)\dcbr.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\common\files.c

"$(INTDIR)\files.obj"	"$(INTDIR)\files.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\src\fnc_exports.c

"$(INTDIR)\fnc_exports.obj"	"$(INTDIR)\fnc_exports.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\src\main.c

"$(INTDIR)\main.obj"	"$(INTDIR)\main.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\src\strings.c

"$(INTDIR)\strings.obj"	"$(INTDIR)\strings.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\common\xctype.c

"$(INTDIR)\xctype.obj"	"$(INTDIR)\xctype.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\res\fxi.rc

!IF  "$(CFG)" == "FXI - Win32 Release"


"$(INTDIR)\fxi.res" : $(SOURCE) "$(INTDIR)"
	$(RSC) /l 0xc0a /fo"$(INTDIR)\fxi.res" /i "res" /d "NDEBUG" $(SOURCE)


!ELSEIF  "$(CFG)" == "FXI - Win32 Debug"


"$(INTDIR)\fxi.res" : $(SOURCE) "$(INTDIR)"
	$(RSC) /l 0xc0a /fo"$(INTDIR)\fxi.res" /i "res" /d "_DEBUG" $(SOURCE)


!ENDIF 


!ENDIF 

