# Microsoft Developer Studio Project File - Name="h264decoder" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=h264decoder - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "h264decoder.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "h264decoder.mak" CFG="h264decoder - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "h264decoder - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "h264decoder - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=xicl6.exe
RSC=rc.exe

!IF  "$(CFG)" == "h264decoder - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /W3 /O2 /I "inc" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /FD /G7 /QxM /c
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=xilink6.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "h264decoder - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /FD /GZ /c
# ADD CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /I "inc" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /FD /GZ /c
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=xilink6.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ENDIF 

# Begin Target

# Name "h264decoder - Win32 Release"
# Name "h264decoder - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\src\annexb.c
# End Source File
# Begin Source File

SOURCE=.\src\biaridecod.c
# End Source File
# Begin Source File

SOURCE=.\src\block.c
# End Source File
# Begin Source File

SOURCE=.\src\cabac.c
# End Source File
# Begin Source File

SOURCE=.\src\context_ini.c
# End Source File
# Begin Source File

SOURCE=.\src\erc_api.c
# End Source File
# Begin Source File

SOURCE=.\src\erc_do_i.c
# End Source File
# Begin Source File

SOURCE=.\src\erc_do_p.c
# End Source File
# Begin Source File

SOURCE=.\src\errorconcealment.c
# End Source File
# Begin Source File

SOURCE=.\src\filehandle.c
# End Source File
# Begin Source File

SOURCE=.\src\fmo.c
# End Source File
# Begin Source File

SOURCE=.\src\header.c
# End Source File
# Begin Source File

SOURCE=.\src\image.c
# End Source File
# Begin Source File

SOURCE=.\src\ldecod.c
# End Source File
# Begin Source File

SOURCE=.\src\leaky_bucket.c
# End Source File
# Begin Source File

SOURCE=.\src\loopFilter.c
# End Source File
# Begin Source File

SOURCE=.\src\macroblock.c
# End Source File
# Begin Source File

SOURCE=.\src\mb_access.c
# End Source File
# Begin Source File

SOURCE=.\src\mbuffer.c
# End Source File
# Begin Source File

SOURCE=.\src\memalloc.c
# End Source File
# Begin Source File

SOURCE=.\src\nal.c
# End Source File
# Begin Source File

SOURCE=.\src\nal_part.c
# End Source File
# Begin Source File

SOURCE=.\src\nalu.c
# End Source File
# Begin Source File

SOURCE=.\src\nalucommon.c
# End Source File
# Begin Source File

SOURCE=.\src\output.c
# End Source File
# Begin Source File

SOURCE=.\src\parset.c
# End Source File
# Begin Source File

SOURCE=.\src\parsetcommon.c
# End Source File
# Begin Source File

SOURCE=.\src\rtp.c
# End Source File
# Begin Source File

SOURCE=.\src\sei.c
# End Source File
# Begin Source File

SOURCE=.\src\transform8x8.c
# End Source File
# Begin Source File

SOURCE=.\src\vlc.c
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\inc\annexb.h
# End Source File
# Begin Source File

SOURCE=.\inc\biaridecod.h
# End Source File
# Begin Source File

SOURCE=.\inc\block.h
# End Source File
# Begin Source File

SOURCE=.\inc\cabac.h
# End Source File
# Begin Source File

SOURCE=.\inc\context_ini.h
# End Source File
# Begin Source File

SOURCE=.\inc\contributors.h
# End Source File
# Begin Source File

SOURCE=.\inc\ctx_tables.h
# End Source File
# Begin Source File

SOURCE=.\inc\defines.h
# End Source File
# Begin Source File

SOURCE=.\inc\elements.h
# End Source File
# Begin Source File

SOURCE=.\inc\erc_api.h
# End Source File
# Begin Source File

SOURCE=.\inc\erc_do.h
# End Source File
# Begin Source File

SOURCE=.\inc\erc_globals.h
# End Source File
# Begin Source File

SOURCE=.\inc\errorconcealment.h
# End Source File
# Begin Source File

SOURCE=.\inc\fmo.h
# End Source File
# Begin Source File

SOURCE=.\inc\global.h
# End Source File
# Begin Source File

SOURCE=.\inc\header.h
# End Source File
# Begin Source File

SOURCE=.\inc\image.h
# End Source File
# Begin Source File

SOURCE=.\inc\leaky_bucket.h
# End Source File
# Begin Source File

SOURCE=.\inc\loopfilter.h
# End Source File
# Begin Source File

SOURCE=.\inc\macroblock.h
# End Source File
# Begin Source File

SOURCE=.\inc\mb_access.h
# End Source File
# Begin Source File

SOURCE=.\inc\mbuffer.h
# End Source File
# Begin Source File

SOURCE=.\inc\memalloc.h
# End Source File
# Begin Source File

SOURCE=.\inc\nalu.h
# End Source File
# Begin Source File

SOURCE=.\inc\nalucommon.h
# End Source File
# Begin Source File

SOURCE=.\inc\output.h
# End Source File
# Begin Source File

SOURCE=.\inc\parset.h
# End Source File
# Begin Source File

SOURCE=.\inc\parsetcommon.h
# End Source File
# Begin Source File

SOURCE=.\inc\rtp.h
# End Source File
# Begin Source File

SOURCE=.\inc\sei.h
# End Source File
# Begin Source File

SOURCE=.\inc\transform8x8.h
# End Source File
# Begin Source File

SOURCE=.\inc\vlc.h
# End Source File
# End Group
# End Target
# End Project
