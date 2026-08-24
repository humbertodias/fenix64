# Microsoft Developer Studio Project File - Name="FXC" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Console Application" 0x0103

CFG=FXC - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "FXC.mak".
!MESSAGE 
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

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "FXC - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "..\bin"
# PROP Intermediate_Dir "release"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /MT /W3 /GX /O2 /I ".\inc" /I "..\includes" /I "..\sdl\include" /I "..\zlib" /I "..\include" /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /FR /YX /FD /c
# ADD BASE RSC /l 0xc0a /d "NDEBUG"
# ADD RSC /l 0xc0a /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386

!ELSEIF  "$(CFG)" == "FXC - Win32 Debug"

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
# ADD CPP /nologo /MT /W3 /Gm /GX /ZI /Od /I ".\inc" /I "..\include" /I "..\sdl\include" /I "..\zlib" /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /FR /YX /FD /GZ /c
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

# Name "FXC - Win32 Release"
# Name "FXC - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\src\c_code.c
# End Source File
# Begin Source File

SOURCE=.\src\c_data.c
# End Source File
# Begin Source File

SOURCE=.\src\c_debug.c
# End Source File
# Begin Source File

SOURCE=.\src\c_fixed.c
# End Source File
# Begin Source File

SOURCE=.\src\c_main.c
# End Source File
# Begin Source File

SOURCE=.\src\codeblock.c
# End Source File
# Begin Source File

SOURCE=.\src\constants.c
# End Source File
# Begin Source File

SOURCE=.\src\dcbw.c
# End Source File
# Begin Source File

SOURCE=..\common\files.c
# End Source File
# Begin Source File

SOURCE=.\src\identifier.c
# End Source File
# Begin Source File

SOURCE=.\src\main.c
# End Source File
# Begin Source File

SOURCE=.\src\main_div.c
# End Source File
# Begin Source File

SOURCE=.\src\messages.c
# End Source File
# Begin Source File

SOURCE=.\src\procedure.c
# End Source File
# Begin Source File

SOURCE=.\src\segment.c
# End Source File
# Begin Source File

SOURCE=.\src\strings.c
# End Source File
# Begin Source File

SOURCE=.\src\sysstub.c
# End Source File
# Begin Source File

SOURCE=.\src\token.c
# End Source File
# Begin Source File

SOURCE=.\src\typedef.c
# End Source File
# Begin Source File

SOURCE=.\src\varspace.c
# End Source File
# Begin Source File

SOURCE=..\common\xctype.c
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=inc\codeblock.h
# End Source File
# Begin Source File

SOURCE=inc\compiler.h
# End Source File
# Begin Source File

SOURCE=inc\constants.h
# End Source File
# Begin Source File

SOURCE=..\include\dcb.h
# End Source File
# Begin Source File

SOURCE=..\include\files.h
# End Source File
# Begin Source File

SOURCE=inc\fxc.h
# End Source File
# Begin Source File

SOURCE=inc\identifiers.h
# End Source File
# Begin Source File

SOURCE=..\include\offsets.h
# End Source File
# Begin Source File

SOURCE=inc\procdef.h
# End Source File
# Begin Source File

SOURCE=..\include\pslang.h
# End Source File
# Begin Source File

SOURCE=inc\segment.h
# End Source File
# Begin Source File

SOURCE=..\include\strings.h
# End Source File
# Begin Source File

SOURCE=inc\token.h
# End Source File
# Begin Source File

SOURCE=..\include\typedef.h
# End Source File
# Begin Source File

SOURCE=inc\varspace.h
# End Source File
# Begin Source File

SOURCE=..\include\xctype.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\res\fxc.ico
# End Source File
# Begin Source File

SOURCE=.\res\fxc.rc
# End Source File
# End Group
# Begin Group "LIBS"

# PROP Default_Filter "lib"
# Begin Source File

SOURCE=..\LIB\zlib.lib
# End Source File
# End Group
# End Target
# End Project
