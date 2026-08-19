# Microsoft Developer Studio Project File - Name="MAP" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Console Application" 0x0103

CFG=MAP - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "MAP.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "MAP.mak" CFG="MAP - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "MAP - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "MAP - Win32 Debug" (based on "Win32 (x86) Console Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "MAP - Win32 Release"

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
# ADD CPP /nologo /W3 /GX /O2 /I "..\..\libpng" /I "..\..\zlib\include" /I "..\..\libungif\lib" /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /FD /c
# SUBTRACT CPP /YX
# ADD BASE RSC /l 0xc0a /d "NDEBUG"
# ADD RSC /l 0xc0a /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /pdb:none /machine:I386

!ELSEIF  "$(CFG)" == "MAP - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "..\bin\Debug"
# PROP Intermediate_Dir "Debug"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /GZ /c
# ADD CPP /nologo /W3 /Gm /GX /ZI /Od /I "..\..\libpng" /I "..\..\zlib\include" /I "..\..\libungif\lib" /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /FD /GZ /c
# SUBTRACT CPP /YX
# ADD BASE RSC /l 0xc0a /d "_DEBUG"
# ADD RSC /l 0xc0a /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept

!ENDIF 

# Begin Target

# Name "MAP - Win32 Release"
# Name "MAP - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Group "libpng"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\libpng\png.c
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
# Begin Group "libungif"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\libungif\lib\dev2gif.c
# End Source File
# Begin Source File

SOURCE=..\..\libungif\lib\dgif_lib.c
# End Source File
# Begin Source File

SOURCE=..\..\libungif\lib\egif_lib.c
# End Source File
# Begin Source File

SOURCE=..\..\libungif\lib\getarg.c
# End Source File
# Begin Source File

SOURCE=..\..\libungif\lib\getarg.h
# End Source File
# Begin Source File

SOURCE=..\..\libungif\lib\gif_err.c
# End Source File
# Begin Source File

SOURCE=..\..\libungif\lib\gif_font.c
# End Source File
# Begin Source File

SOURCE=..\..\libungif\lib\gif_lib.h
# End Source File
# Begin Source File

SOURCE=..\..\libungif\lib\gif_lib_private.h
# End Source File
# Begin Source File

SOURCE=..\..\libungif\lib\gifalloc.c
# End Source File
# Begin Source File

SOURCE=..\..\libungif\lib\qprintf.c
# End Source File
# Begin Source File

SOURCE=..\..\libungif\lib\quantize.c
# End Source File
# End Group
# Begin Source File

SOURCE=.\map.c
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\map.ico
# End Source File
# Begin Source File

SOURCE=.\map.rc
# End Source File
# End Group
# Begin Group "LIBS"

# PROP Default_Filter "lib"
# Begin Source File

SOURCE=..\..\zlib\lib\zdll.lib
# End Source File
# End Group
# End Target
# End Project
