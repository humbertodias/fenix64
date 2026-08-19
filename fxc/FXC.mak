# Microsoft Developer Studio Generated NMAKE File, Based on FXC.dsp
!IF "$(CFG)" == ""
CFG=FXC - Win32 Debug
!MESSAGE No configuration specified. Defaulting to FXC - Win32 Debug.
!ENDIF 

!IF "$(CFG)" != "FXC - Win32 Release" && "$(CFG)" != "FXC - Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "FXC.mak" CFG="FXC - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "FXC - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "FXC - Win32 Debug" (based on "Win32 (x86) Console Application")
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

!IF  "$(CFG)" == "FXC - Win32 Release"

OUTDIR=.\..\bin
INTDIR=.\release
# Begin Custom Macros
OutDir=.\..\bin
# End Custom Macros

ALL : "$(OUTDIR)\FXC.exe" "$(OUTDIR)\FXC.bsc"


CLEAN :
	-@erase "$(INTDIR)\c_code.obj"
	-@erase "$(INTDIR)\c_code.sbr"
	-@erase "$(INTDIR)\c_data.obj"
	-@erase "$(INTDIR)\c_data.sbr"
	-@erase "$(INTDIR)\c_debug.obj"
	-@erase "$(INTDIR)\c_debug.sbr"
	-@erase "$(INTDIR)\c_fixed.obj"
	-@erase "$(INTDIR)\c_fixed.sbr"
	-@erase "$(INTDIR)\c_main.obj"
	-@erase "$(INTDIR)\c_main.sbr"
	-@erase "$(INTDIR)\codeblock.obj"
	-@erase "$(INTDIR)\codeblock.sbr"
	-@erase "$(INTDIR)\constants.obj"
	-@erase "$(INTDIR)\constants.sbr"
	-@erase "$(INTDIR)\dcbw.obj"
	-@erase "$(INTDIR)\dcbw.sbr"
	-@erase "$(INTDIR)\files.obj"
	-@erase "$(INTDIR)\files.sbr"
	-@erase "$(INTDIR)\fxc.res"
	-@erase "$(INTDIR)\identifier.obj"
	-@erase "$(INTDIR)\identifier.sbr"
	-@erase "$(INTDIR)\main.obj"
	-@erase "$(INTDIR)\main.sbr"
	-@erase "$(INTDIR)\main_div.obj"
	-@erase "$(INTDIR)\main_div.sbr"
	-@erase "$(INTDIR)\messages.obj"
	-@erase "$(INTDIR)\messages.sbr"
	-@erase "$(INTDIR)\procedure.obj"
	-@erase "$(INTDIR)\procedure.sbr"
	-@erase "$(INTDIR)\segment.obj"
	-@erase "$(INTDIR)\segment.sbr"
	-@erase "$(INTDIR)\strings.obj"
	-@erase "$(INTDIR)\strings.sbr"
	-@erase "$(INTDIR)\sysstub.obj"
	-@erase "$(INTDIR)\sysstub.sbr"
	-@erase "$(INTDIR)\token.obj"
	-@erase "$(INTDIR)\token.sbr"
	-@erase "$(INTDIR)\typedef.obj"
	-@erase "$(INTDIR)\typedef.sbr"
	-@erase "$(INTDIR)\varspace.obj"
	-@erase "$(INTDIR)\varspace.sbr"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\xctype.obj"
	-@erase "$(INTDIR)\xctype.sbr"
	-@erase "$(OUTDIR)\FXC.bsc"
	-@erase "$(OUTDIR)\FXC.exe"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

"$(INTDIR)" :
    if not exist "$(INTDIR)/$(NULL)" mkdir "$(INTDIR)"

CPP_PROJ=/nologo /MT /W3 /GX /O2 /I ".\inc" /I "..\includes" /I "..\sdl\include" /I "..\zlib" /I "..\include" /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /FR"$(INTDIR)\\" /Fp"$(INTDIR)\FXC.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 
RSC_PROJ=/l 0xc0a /fo"$(INTDIR)\fxc.res" /d "NDEBUG" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\FXC.bsc" 
BSC32_SBRS= \
	"$(INTDIR)\c_code.sbr" \
	"$(INTDIR)\c_data.sbr" \
	"$(INTDIR)\c_debug.sbr" \
	"$(INTDIR)\c_fixed.sbr" \
	"$(INTDIR)\c_main.sbr" \
	"$(INTDIR)\codeblock.sbr" \
	"$(INTDIR)\constants.sbr" \
	"$(INTDIR)\dcbw.sbr" \
	"$(INTDIR)\files.sbr" \
	"$(INTDIR)\identifier.sbr" \
	"$(INTDIR)\main.sbr" \
	"$(INTDIR)\main_div.sbr" \
	"$(INTDIR)\messages.sbr" \
	"$(INTDIR)\procedure.sbr" \
	"$(INTDIR)\segment.sbr" \
	"$(INTDIR)\strings.sbr" \
	"$(INTDIR)\sysstub.sbr" \
	"$(INTDIR)\token.sbr" \
	"$(INTDIR)\typedef.sbr" \
	"$(INTDIR)\varspace.sbr" \
	"$(INTDIR)\xctype.sbr"

"$(OUTDIR)\FXC.bsc" : "$(OUTDIR)" $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /incremental:no /pdb:"$(OUTDIR)\FXC.pdb" /machine:I386 /out:"$(OUTDIR)\FXC.exe" 
LINK32_OBJS= \
	"$(INTDIR)\c_code.obj" \
	"$(INTDIR)\c_data.obj" \
	"$(INTDIR)\c_debug.obj" \
	"$(INTDIR)\c_fixed.obj" \
	"$(INTDIR)\c_main.obj" \
	"$(INTDIR)\codeblock.obj" \
	"$(INTDIR)\constants.obj" \
	"$(INTDIR)\dcbw.obj" \
	"$(INTDIR)\files.obj" \
	"$(INTDIR)\identifier.obj" \
	"$(INTDIR)\main.obj" \
	"$(INTDIR)\main_div.obj" \
	"$(INTDIR)\messages.obj" \
	"$(INTDIR)\procedure.obj" \
	"$(INTDIR)\segment.obj" \
	"$(INTDIR)\strings.obj" \
	"$(INTDIR)\sysstub.obj" \
	"$(INTDIR)\token.obj" \
	"$(INTDIR)\typedef.obj" \
	"$(INTDIR)\varspace.obj" \
	"$(INTDIR)\xctype.obj" \
	"$(INTDIR)\fxc.res" \
	"..\LIB\zlib.lib"

"$(OUTDIR)\FXC.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "FXC - Win32 Debug"

OUTDIR=.\..\bin\Debug
INTDIR=.\Debug
# Begin Custom Macros
OutDir=.\..\bin\Debug
# End Custom Macros

ALL : "$(OUTDIR)\FXC.exe"


CLEAN :
	-@erase "$(INTDIR)\c_code.obj"
	-@erase "$(INTDIR)\c_data.obj"
	-@erase "$(INTDIR)\c_debug.obj"
	-@erase "$(INTDIR)\c_fixed.obj"
	-@erase "$(INTDIR)\c_main.obj"
	-@erase "$(INTDIR)\codeblock.obj"
	-@erase "$(INTDIR)\constants.obj"
	-@erase "$(INTDIR)\dcbw.obj"
	-@erase "$(INTDIR)\files.obj"
	-@erase "$(INTDIR)\fxc.res"
	-@erase "$(INTDIR)\identifier.obj"
	-@erase "$(INTDIR)\main.obj"
	-@erase "$(INTDIR)\main_div.obj"
	-@erase "$(INTDIR)\messages.obj"
	-@erase "$(INTDIR)\procedure.obj"
	-@erase "$(INTDIR)\segment.obj"
	-@erase "$(INTDIR)\strings.obj"
	-@erase "$(INTDIR)\sysstub.obj"
	-@erase "$(INTDIR)\token.obj"
	-@erase "$(INTDIR)\typedef.obj"
	-@erase "$(INTDIR)\varspace.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(INTDIR)\xctype.obj"
	-@erase "$(OUTDIR)\FXC.exe"
	-@erase "$(OUTDIR)\FXC.ilk"
	-@erase "$(OUTDIR)\FXC.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

"$(INTDIR)" :
    if not exist "$(INTDIR)/$(NULL)" mkdir "$(INTDIR)"

CPP_PROJ=/nologo /MT /W3 /Gm /GX /ZI /Od /I ".\inc" /I "..\include" /I "..\sdl\include" /I "..\zlib" /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /Fp"$(INTDIR)\FXC.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 
RSC_PROJ=/l 0xc0a /fo"$(INTDIR)\fxc.res" /d "_DEBUG" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\FXC.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /incremental:yes /pdb:"$(OUTDIR)\FXC.pdb" /debug /machine:I386 /out:"$(OUTDIR)\FXC.exe" /pdbtype:sept 
LINK32_OBJS= \
	"$(INTDIR)\c_code.obj" \
	"$(INTDIR)\c_data.obj" \
	"$(INTDIR)\c_debug.obj" \
	"$(INTDIR)\c_fixed.obj" \
	"$(INTDIR)\c_main.obj" \
	"$(INTDIR)\codeblock.obj" \
	"$(INTDIR)\constants.obj" \
	"$(INTDIR)\dcbw.obj" \
	"$(INTDIR)\files.obj" \
	"$(INTDIR)\identifier.obj" \
	"$(INTDIR)\main.obj" \
	"$(INTDIR)\main_div.obj" \
	"$(INTDIR)\messages.obj" \
	"$(INTDIR)\procedure.obj" \
	"$(INTDIR)\segment.obj" \
	"$(INTDIR)\strings.obj" \
	"$(INTDIR)\sysstub.obj" \
	"$(INTDIR)\token.obj" \
	"$(INTDIR)\typedef.obj" \
	"$(INTDIR)\varspace.obj" \
	"$(INTDIR)\xctype.obj" \
	"$(INTDIR)\fxc.res" \
	"..\LIB\zlib.lib"

"$(OUTDIR)\FXC.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
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
!IF EXISTS("FXC.dep")
!INCLUDE "FXC.dep"
!ELSE 
!MESSAGE Warning: cannot find "FXC.dep"
!ENDIF 
!ENDIF 


!IF "$(CFG)" == "FXC - Win32 Release" || "$(CFG)" == "FXC - Win32 Debug"
SOURCE=.\src\c_code.c

!IF  "$(CFG)" == "FXC - Win32 Release"


"$(INTDIR)\c_code.obj"	"$(INTDIR)\c_code.sbr" : $(SOURCE) "$(INTDIR)" ".\src\messages.c"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "FXC - Win32 Debug"


"$(INTDIR)\c_code.obj" : $(SOURCE) "$(INTDIR)" ".\src\messages.c"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\src\c_data.c

!IF  "$(CFG)" == "FXC - Win32 Release"


"$(INTDIR)\c_data.obj"	"$(INTDIR)\c_data.sbr" : $(SOURCE) "$(INTDIR)" ".\src\messages.c"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "FXC - Win32 Debug"


"$(INTDIR)\c_data.obj" : $(SOURCE) "$(INTDIR)" ".\src\messages.c"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\src\c_debug.c

!IF  "$(CFG)" == "FXC - Win32 Release"


"$(INTDIR)\c_debug.obj"	"$(INTDIR)\c_debug.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "FXC - Win32 Debug"


"$(INTDIR)\c_debug.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\src\c_fixed.c

!IF  "$(CFG)" == "FXC - Win32 Release"


"$(INTDIR)\c_fixed.obj"	"$(INTDIR)\c_fixed.sbr" : $(SOURCE) "$(INTDIR)" ".\src\messages.c"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "FXC - Win32 Debug"


"$(INTDIR)\c_fixed.obj" : $(SOURCE) "$(INTDIR)" ".\src\messages.c"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\src\c_main.c

!IF  "$(CFG)" == "FXC - Win32 Release"


"$(INTDIR)\c_main.obj"	"$(INTDIR)\c_main.sbr" : $(SOURCE) "$(INTDIR)" ".\src\messages.c"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "FXC - Win32 Debug"


"$(INTDIR)\c_main.obj" : $(SOURCE) "$(INTDIR)" ".\src\messages.c"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\src\codeblock.c

!IF  "$(CFG)" == "FXC - Win32 Release"


"$(INTDIR)\codeblock.obj"	"$(INTDIR)\codeblock.sbr" : $(SOURCE) "$(INTDIR)" ".\src\messages.c"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "FXC - Win32 Debug"


"$(INTDIR)\codeblock.obj" : $(SOURCE) "$(INTDIR)" ".\src\messages.c"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\src\constants.c

!IF  "$(CFG)" == "FXC - Win32 Release"


"$(INTDIR)\constants.obj"	"$(INTDIR)\constants.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "FXC - Win32 Debug"


"$(INTDIR)\constants.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\src\dcbw.c

!IF  "$(CFG)" == "FXC - Win32 Release"


"$(INTDIR)\dcbw.obj"	"$(INTDIR)\dcbw.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "FXC - Win32 Debug"


"$(INTDIR)\dcbw.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=..\common\files.c

!IF  "$(CFG)" == "FXC - Win32 Release"


"$(INTDIR)\files.obj"	"$(INTDIR)\files.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "FXC - Win32 Debug"


"$(INTDIR)\files.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\src\identifier.c

!IF  "$(CFG)" == "FXC - Win32 Release"


"$(INTDIR)\identifier.obj"	"$(INTDIR)\identifier.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "FXC - Win32 Debug"


"$(INTDIR)\identifier.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\src\main.c

!IF  "$(CFG)" == "FXC - Win32 Release"


"$(INTDIR)\main.obj"	"$(INTDIR)\main.sbr" : $(SOURCE) "$(INTDIR)" ".\src\messages.c"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "FXC - Win32 Debug"


"$(INTDIR)\main.obj" : $(SOURCE) "$(INTDIR)" ".\src\messages.c"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\src\main_div.c

!IF  "$(CFG)" == "FXC - Win32 Release"


"$(INTDIR)\main_div.obj"	"$(INTDIR)\main_div.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "FXC - Win32 Debug"


"$(INTDIR)\main_div.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\src\messages.c

!IF  "$(CFG)" == "FXC - Win32 Release"


"$(INTDIR)\messages.obj"	"$(INTDIR)\messages.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "FXC - Win32 Debug"


"$(INTDIR)\messages.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\src\procedure.c

!IF  "$(CFG)" == "FXC - Win32 Release"


"$(INTDIR)\procedure.obj"	"$(INTDIR)\procedure.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "FXC - Win32 Debug"


"$(INTDIR)\procedure.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\src\segment.c

!IF  "$(CFG)" == "FXC - Win32 Release"


"$(INTDIR)\segment.obj"	"$(INTDIR)\segment.sbr" : $(SOURCE) "$(INTDIR)" ".\src\messages.c"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "FXC - Win32 Debug"


"$(INTDIR)\segment.obj" : $(SOURCE) "$(INTDIR)" ".\src\messages.c"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\src\strings.c

!IF  "$(CFG)" == "FXC - Win32 Release"


"$(INTDIR)\strings.obj"	"$(INTDIR)\strings.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "FXC - Win32 Debug"


"$(INTDIR)\strings.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\src\sysstub.c

!IF  "$(CFG)" == "FXC - Win32 Release"


"$(INTDIR)\sysstub.obj"	"$(INTDIR)\sysstub.sbr" : $(SOURCE) "$(INTDIR)" ".\src\messages.c"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "FXC - Win32 Debug"


"$(INTDIR)\sysstub.obj" : $(SOURCE) "$(INTDIR)" ".\src\messages.c"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\src\token.c

!IF  "$(CFG)" == "FXC - Win32 Release"


"$(INTDIR)\token.obj"	"$(INTDIR)\token.sbr" : $(SOURCE) "$(INTDIR)" ".\src\messages.c"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "FXC - Win32 Debug"


"$(INTDIR)\token.obj" : $(SOURCE) "$(INTDIR)" ".\src\messages.c"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\src\typedef.c

!IF  "$(CFG)" == "FXC - Win32 Release"


"$(INTDIR)\typedef.obj"	"$(INTDIR)\typedef.sbr" : $(SOURCE) "$(INTDIR)" ".\src\messages.c"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "FXC - Win32 Debug"


"$(INTDIR)\typedef.obj" : $(SOURCE) "$(INTDIR)" ".\src\messages.c"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\src\varspace.c

!IF  "$(CFG)" == "FXC - Win32 Release"


"$(INTDIR)\varspace.obj"	"$(INTDIR)\varspace.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "FXC - Win32 Debug"


"$(INTDIR)\varspace.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=..\common\xctype.c

!IF  "$(CFG)" == "FXC - Win32 Release"


"$(INTDIR)\xctype.obj"	"$(INTDIR)\xctype.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "FXC - Win32 Debug"


"$(INTDIR)\xctype.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\res\fxc.rc

!IF  "$(CFG)" == "FXC - Win32 Release"


"$(INTDIR)\fxc.res" : $(SOURCE) "$(INTDIR)"
	$(RSC) /l 0xc0a /fo"$(INTDIR)\fxc.res" /i "res" /d "NDEBUG" $(SOURCE)


!ELSEIF  "$(CFG)" == "FXC - Win32 Debug"


"$(INTDIR)\fxc.res" : $(SOURCE) "$(INTDIR)"
	$(RSC) /l 0xc0a /fo"$(INTDIR)\fxc.res" /i "res" /d "_DEBUG" $(SOURCE)


!ENDIF 


!ENDIF 

