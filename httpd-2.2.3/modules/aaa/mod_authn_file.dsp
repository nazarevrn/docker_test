# Microsoft Developer Studio Project File - Name="mod_authn_file" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=mod_authn_file - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "mod_authn_file.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "mod_authn_file.mak" CFG="mod_authn_file - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "mod_authn_file - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "mod_authn_file - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "mod_authn_file - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MD /W3 /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /FD /c
# ADD CPP /nologo /MD /W3 /O2 /Oy- /Zi /I "../../include" /I "../../srclib/apr/include" /I "../../srclib/apr-util/include" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /Fd"Release\mod_authn_file_src" /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /o /win32 "NUL"
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /o /win32 "NUL"
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /fo"Release/mod_authn_file.res" /i "../../include" /i "../../srclib/apr/include" /d "NDEBUG" /d "BIN_NAME=mod_authn_file.so" /d "LONG_NAME=authn_file_module for Apache"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib /nologo /subsystem:windows /dll /out:"Release/mod_authn_file.so" /base:@..\..\os\win32\BaseAddr.ref,mod_authn_file.so
# ADD LINK32 kernel32.lib /nologo /subsystem:windows /dll /incremental:no /debug /out:"Release/mod_authn_file.so" /base:@..\..\os\win32\BaseAddr.ref,mod_authn_file.so /opt:ref

!ELSEIF  "$(CFG)" == "mod_authn_file - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MDd /W3 /EHsc /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /FD /c
# ADD CPP /nologo /MDd /W3 /EHsc /Zi /Od /I "../../include" /I "../../srclib/apr/include" /I "../../srclib/apr-util/include" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /Fd"Debug\mod_authn_file_src" /FD /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /o /win32 "NUL"
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /o /win32 "NUL"
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /fo"Debug/mod_authn_file.res" /i "../../include" /i "../../srclib/apr/include" /d "_DEBUG" /d "BIN_NAME=mod_authn_file.so" /d "LONG_NAME=authn_file_module for Apache"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib /nologo /subsystem:windows /dll /incremental:no /debug /out:"Debug/mod_authn_file.so" /base:@..\..\os\win32\BaseAddr.ref,mod_authn_file.so
# ADD LINK32 kernel32.lib /nologo /subsystem:windows /dll /incremental:no /debug /out:"Debug/mod_authn_file.so" /base:@..\..\os\win32\BaseAddr.ref,mod_authn_file.so

!ENDIF 

# Begin Target

# Name "mod_authn_file - Win32 Release"
# Name "mod_authn_file - Win32 Debug"
# Begin Source File

SOURCE=.\mod_authn_file.c
# End Source File
# Begin Source File

SOURCE=..\..\build\win32\httpd.rc
# End Source File
# End Target
# End Project
