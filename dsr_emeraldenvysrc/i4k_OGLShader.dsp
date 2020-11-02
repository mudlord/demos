# Microsoft Developer Studio Project File - Name="i4k_OGLShader" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=i4k_OGLShader - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "i4k_OGLShader.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "i4k_OGLShader.mak" CFG="i4k_OGLShader - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "i4k_OGLShader - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "i4k_OGLShader - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "i4k_OGLShader - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "bin/vc6/Release"
# PROP Intermediate_Dir "bin/vc6/Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /Gr /Zp1 /W3 /WX /Ox /Oa /Og /Oi /Os /Gf /I "../" /D "WINDOWS" /D "A32BITS" /D "SIMD" /D "NDEBUG" /FAs /FD /QIfist /Gs /c
# SUBTRACT CPP /Fr
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib opengl32.lib winmm.lib /nologo /entry:"entrypoint" /subsystem:windows /pdb:none /machine:I386 /nodefaultlib /out:"exe/main_vc6_rel.exe" /CRINKLER

!ELSEIF  "$(CFG)" == "i4k_OGLShader - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "bin/vc6/Debug"
# PROP Intermediate_Dir "bin/vc6/Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /FD /GZ /c
# ADD CPP /nologo /ML /W3 /WX /Zi /Od /Op /I "../" /D "_DEBUG" /D "WINDOWS" /D "DEBUG" /D "SIMD" /D "A32BITS" /FR /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winmm.lib opengl32.lib comdlg32.lib /nologo /subsystem:windows /debug /machine:I386 /out:"exe/main_vc6_deb.exe" /pdbtype:sept

!ENDIF 

# Begin Target

# Name "i4k_OGLShader - Win32 Release"
# Name "i4k_OGLShader - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Group "shaders"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\src\shaders\fsh_rayt.inl
# End Source File
# Begin Source File

SOURCE=.\src\shaders\vsh_2d.inl
# End Source File
# End Group
# Begin Group "_windows"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\src\_windows\main_deb.cpp

!IF  "$(CFG)" == "i4k_OGLShader - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "i4k_OGLShader - Win32 Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\src\_windows\main_rel.cpp

!IF  "$(CFG)" == "i4k_OGLShader - Win32 Release"

!ELSEIF  "$(CFG)" == "i4k_OGLShader - Win32 Debug"

# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# End Group
# Begin Source File

SOURCE=.\src\config.h
# End Source File
# Begin Source File

SOURCE=.\src\ext.cpp
# End Source File
# Begin Source File

SOURCE=.\src\ext.h
# End Source File
# Begin Source File

SOURCE=.\src\fp.h
# End Source File
# Begin Source File

SOURCE=.\src\glext.h
# End Source File
# Begin Source File

SOURCE=.\src\intro.cpp
# End Source File
# Begin Source File

SOURCE=.\src\intro.h
# End Source File
# Begin Source File

SOURCE=.\src\mzk.cpp
# End Source File
# Begin Source File

SOURCE=.\src\mzk.h
# End Source File
# End Group
# End Target
# End Project
