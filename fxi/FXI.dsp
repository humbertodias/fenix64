# Microsoft Developer Studio Project File - Name="FXI" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Console Application" 0x0103

CFG=FXI - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "FXI.mak".
!MESSAGE 
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

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "FXI - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "..\bin"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /MD /W3 /GX /Zd /O2 /I ".\inc" /I "..\include" /I "..\..\sdl\include" /I "..\libpng" /I "..\mikmod" /I "..\..\zlib\include" /I "..\freetype\include" /D "NDEBUG" /D "STD_HEADERS" /D "_WINDOWS" /D "ZLIB_DLL" /D "_CONSOLE" /D "REGEX_MALLOC" /D "WIN32" /D "_MBCS" /D STDC_HEADERS=1 /D "TARGET_win32" /FR /FD /c
# SUBTRACT CPP /YX
# ADD BASE RSC /l 0xc0a /d "NDEBUG"
# ADD RSC /l 0xc0a /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /incremental:yes /debug /machine:I386
# SUBTRACT LINK32 /pdb:none

!ELSEIF  "$(CFG)" == "FXI - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "..\bin\Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /GZ /c
# ADD CPP /nologo /MD /W3 /Gm /GX /ZI /Od /I ".\inc" /I "..\include" /I "..\..\sdl\include" /I "..\..\libpng" /I "..\mikmod" /I "..\..\zlib\include" /I "..\freetype\include" /D "_DEBUG" /D "ZLIB_DLL" /D "_CONSOLE" /D "REGEX_MALLOC" /D "WIN32" /D "_MBCS" /D STDC_HEADERS=1 /D "TARGET_win32" /FR /FD /GZ /c
# SUBTRACT CPP /YX
# ADD BASE RSC /l 0xc0a /d "_DEBUG"
# ADD RSC /l 0xc0a /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept
# SUBTRACT LINK32 /pdb:none

!ENDIF 

# Begin Target

# Name "FXI - Win32 Release"
# Name "FXI - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Group "graphic library"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\src\fbm.c
# End Source File
# Begin Source File

SOURCE=.\src\fgc.c
# End Source File
# Begin Source File

SOURCE=.\src\fpl.c
# End Source File
# Begin Source File

SOURCE=.\src\g_blendop.c
# End Source File
# Begin Source File

SOURCE=.\src\g_blit.c
# End Source File
# Begin Source File

SOURCE=.\src\g_console.c
# End Source File
# Begin Source File

SOURCE=.\src\g_conversion.c
# End Source File
# Begin Source File

SOURCE=.\src\g_draw.c
# End Source File
# Begin Source File

SOURCE=.\src\g_flic.c
# End Source File
# Begin Source File

SOURCE=.\src\g_font.c
# End Source File
# Begin Source File

SOURCE=.\src\g_fpg.c
# End Source File
# Begin Source File

SOURCE=.\src\g_main.c
# End Source File
# Begin Source File

SOURCE=.\src\g_maps.c
# End Source File
# Begin Source File

SOURCE=.\src\g_mode7.c
# End Source File
# Begin Source File

SOURCE=.\src\g_pal.c
# End Source File
# Begin Source File

SOURCE=.\src\g_profiler.c
# End Source File
# Begin Source File

SOURCE=.\src\g_scroll.c
# End Source File
# Begin Source File

SOURCE=.\src\g_systexts.c
# End Source File
# Begin Source File

SOURCE=.\src\g_texts.c
# End Source File
# Begin Source File

SOURCE=.\src\img_pcx.c
# End Source File
# Begin Source File

SOURCE=.\src\img_png.c
# End Source File
# Begin Source File

SOURCE=.\src\mmx_hspan.c
# End Source File
# Begin Source File

SOURCE=.\src\mmx_main.c
# End Source File
# Begin Source File

SOURCE=.\src\mmx_scale2x.c
# End Source File
# End Group
# Begin Group "interpreter"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\src\f_bgload.c
# End Source File
# Begin Source File

SOURCE=.\src\f_cd.c
# End Source File
# Begin Source File

SOURCE=.\src\f_joystick.c
# End Source File
# Begin Source File

SOURCE=.\src\f_sort.c
# End Source File
# Begin Source File

SOURCE=.\src\i_copy.c
# End Source File
# Begin Source File

SOURCE=.\src\i_debug.c
# End Source File
# Begin Source File

SOURCE=.\src\i_func.c
# End Source File
# Begin Source File

SOURCE=.\src\i_main.c
# End Source File
# Begin Source File

SOURCE=.\src\i_saveload.c
# End Source File
# Begin Source File

SOURCE=.\src\instance.c
# End Source File
# Begin Source File

SOURCE=.\src\pathfind.c
# End Source File
# Begin Source File

SOURCE=.\src\sound.c
# End Source File
# End Group
# Begin Group "libpng"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\libpng\png.c
# End Source File
# Begin Source File

SOURCE=..\..\libpng\pngerror.c
# End Source File
# Begin Source File

SOURCE=..\..\libpng\pnggccrd.c
# End Source File
# Begin Source File

SOURCE=..\..\libpng\pngget.c
# End Source File
# Begin Source File

SOURCE=..\..\libpng\pngmem.c
# End Source File
# Begin Source File

SOURCE=..\..\libpng\pngpread.c
# End Source File
# Begin Source File

SOURCE=..\..\libpng\pngread.c
# End Source File
# Begin Source File

SOURCE=..\..\libpng\pngrio.c
# End Source File
# Begin Source File

SOURCE=..\..\libpng\pngrtran.c
# End Source File
# Begin Source File

SOURCE=..\..\libpng\pngrutil.c
# End Source File
# Begin Source File

SOURCE=..\..\libpng\pngset.c
# End Source File
# Begin Source File

SOURCE=..\..\libpng\pngtrans.c
# End Source File
# Begin Source File

SOURCE=..\..\libpng\pngvcrd.c
# End Source File
# Begin Source File

SOURCE=..\..\libpng\pngwio.c
# End Source File
# Begin Source File

SOURCE=..\..\libpng\pngwrite.c
# End Source File
# Begin Source File

SOURCE=..\..\libpng\pngwtran.c
# End Source File
# Begin Source File

SOURCE=..\..\libpng\pngwutil.c
# End Source File
# End Group
# Begin Source File

SOURCE=.\src\dcbr.c
# End Source File
# Begin Source File

SOURCE=..\common\dirs.c
# End Source File
# Begin Source File

SOURCE=..\common\files.c
# End Source File
# Begin Source File

SOURCE=.\src\fnc_exports.c
# End Source File
# Begin Source File

SOURCE=.\src\main.c
# End Source File
# Begin Source File

SOURCE=..\common\regex.c
# End Source File
# Begin Source File

SOURCE=.\src\strings.c
# End Source File
# Begin Source File

SOURCE=..\common\xctype.c
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Group "External"

# PROP Default_Filter ""
# Begin Source File

SOURCE="..\..\..\Archivos de programa\DirectX 8.1 SDK\include\basetsd.h"
# End Source File
# Begin Source File

SOURCE=..\SDL\include\begin_code.h
# End Source File
# Begin Source File

SOURCE=..\SDL\include\close_code.h
# End Source File
# Begin Source File

SOURCE=..\SDL\include\SDL.h
# End Source File
# Begin Source File

SOURCE=..\SDL\include\SDL_active.h
# End Source File
# Begin Source File

SOURCE=..\SDL\include\SDL_audio.h
# End Source File
# Begin Source File

SOURCE=..\SDL\include\SDL_byteorder.h
# End Source File
# Begin Source File

SOURCE=..\SDL\include\SDL_cdrom.h
# End Source File
# Begin Source File

SOURCE=..\SDL\include\SDL_error.h
# End Source File
# Begin Source File

SOURCE=..\SDL\include\SDL_events.h
# End Source File
# Begin Source File

SOURCE=..\SDL\include\SDL_getenv.h
# End Source File
# Begin Source File

SOURCE=..\SDL\include\SDL_joystick.h
# End Source File
# Begin Source File

SOURCE=..\SDL\include\SDL_keyboard.h
# End Source File
# Begin Source File

SOURCE=..\SDL\include\SDL_keysym.h
# End Source File
# Begin Source File

SOURCE=..\SDL\include\SDL_main.h
# End Source File
# Begin Source File

SOURCE=..\SDL\include\SDL_mouse.h
# End Source File
# Begin Source File

SOURCE=..\SDL\include\SDL_mutex.h
# End Source File
# Begin Source File

SOURCE=..\SDL\include\SDL_quit.h
# End Source File
# Begin Source File

SOURCE=..\SDL\include\SDL_rwops.h
# End Source File
# Begin Source File

SOURCE=..\SDL\include\SDL_timer.h
# End Source File
# Begin Source File

SOURCE=..\SDL\include\SDL_types.h
# End Source File
# Begin Source File

SOURCE=..\SDL\include\SDL_version.h
# End Source File
# Begin Source File

SOURCE=..\SDL\include\SDL_video.h
# End Source File
# End Group
# Begin Source File

SOURCE=..\include\dcb.h
# End Source File
# Begin Source File

SOURCE=..\include\dirs.h
# End Source File
# Begin Source File

SOURCE=..\include\fbm.h
# End Source File
# Begin Source File

SOURCE=..\include\fgc.h
# End Source File
# Begin Source File

SOURCE=..\include\files.h
# End Source File
# Begin Source File

SOURCE=..\include\files_st.h
# End Source File
# Begin Source File

SOURCE=inc\flic.h
# End Source File
# Begin Source File

SOURCE=.\inc\flic_st.h
# End Source File
# Begin Source File

SOURCE=inc\fmath.h
# End Source File
# Begin Source File

SOURCE=..\include\fnx_loadlib.h
# End Source File
# Begin Source File

SOURCE=inc\font.h
# End Source File
# Begin Source File

SOURCE=..\include\fpl.h
# End Source File
# Begin Source File

SOURCE=..\include\fxdll.h
# End Source File
# Begin Source File

SOURCE=inc\fxi.h
# End Source File
# Begin Source File

SOURCE=inc\grlib.h
# End Source File
# Begin Source File

SOURCE=.\inc\grlib_st.h
# End Source File
# Begin Source File

SOURCE=inc\i_procdef.h
# End Source File
# Begin Source File

SOURCE=.\inc\i_procdef_st.h
# End Source File
# Begin Source File

SOURCE=inc\instance.h
# End Source File
# Begin Source File

SOURCE=.\inc\instance_st.h
# End Source File
# Begin Source File

SOURCE=..\include\offsets.h
# End Source File
# Begin Source File

SOURCE=..\..\libpng\png.h
# End Source File
# Begin Source File

SOURCE=..\..\libpng\pngasmrd.h
# End Source File
# Begin Source File

SOURCE=..\..\libpng\pngconf.h
# End Source File
# Begin Source File

SOURCE=..\include\pslang.h
# End Source File
# Begin Source File

SOURCE=..\include\regex.h
# End Source File
# Begin Source File

SOURCE=..\SDL_mixer\include\SDL_mixer.h
# End Source File
# Begin Source File

SOURCE=inc\sound.h
# End Source File
# Begin Source File

SOURCE=..\include\strings.h
# End Source File
# Begin Source File

SOURCE=..\include\sysprocs.h
# End Source File
# Begin Source File

SOURCE=.\inc\sysprocs_p.h
# End Source File
# Begin Source File

SOURCE=..\include\typedef.h
# End Source File
# Begin Source File

SOURCE=..\include\typedef_st.h
# End Source File
# Begin Source File

SOURCE=..\include\xctype.h
# End Source File
# Begin Source File

SOURCE=..\include\xctype_st.h
# End Source File
# Begin Source File

SOURCE=..\ZLIB\zconf.h
# End Source File
# Begin Source File

SOURCE=..\ZLIB\zlib.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\res\fxi.ico
# End Source File
# Begin Source File

SOURCE=.\res\fxi.rc
# End Source File
# End Group
# Begin Group "LIBS"

# PROP Default_Filter "lib"
# Begin Source File

SOURCE=..\..\SDL_mixer\lib\SDL_mixer.lib
# End Source File
# Begin Source File

SOURCE=..\..\SDL\lib\SDLmain.lib
# End Source File
# Begin Source File

SOURCE=..\..\SDL\lib\SDL.lib
# End Source File
# Begin Source File

SOURCE=..\..\zlib\lib\zdll.lib
# End Source File
# End Group
# End Target
# End Project
