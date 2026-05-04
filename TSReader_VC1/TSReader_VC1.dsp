# Microsoft Developer Studio Project File - Name="TSReader_VC1" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=TSReader_VC1 - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "TSReader_VC1.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "TSReader_VC1.mak" CFG="TSReader_VC1 - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "TSReader_VC1 - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "TSReader_VC1 - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "TSReader_VC1 - Win32 Release"

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
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "TSREADER_VC1_EXPORTS" /YX /FD /c
# ADD CPP /nologo /MT /W3 /GX /O2 /I "..\VC1_reference_decoder\shared" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "TSREADER_VC1_EXPORTS" /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /machine:I386 /out:"../TSReader_VC1.dll"

!ELSEIF  "$(CFG)" == "TSReader_VC1 - Win32 Debug"

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
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "TSREADER_VC1_EXPORTS" /YX /FD /GZ /c
# ADD CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /I "..\VC1_reference_decoder\shared" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "TSREADER_VC1_EXPORTS" /YX /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /debug /machine:I386 /out:"../TSReader_VC1.dll" /pdbtype:sept

!ENDIF 

# Begin Target

# Name "TSReader_VC1 - Win32 Release"
# Name "TSReader_VC1 - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=..\VC1_reference_decoder\decoder\decfile.c
# End Source File
# Begin Source File

SOURCE=..\VC1_reference_decoder\decoder\decoder.c
# End Source File
# Begin Source File

SOURCE=..\VC1_reference_decoder\decoder\decopts.c
# End Source File
# Begin Source File

SOURCE=..\VC1_reference_decoder\shared\rdopts.c
# End Source File
# Begin Source File

SOURCE=.\TSReader_VC1.def
# End Source File
# Begin Source File

SOURCE=..\VC1_reference_decoder\shared\vc13dhtab.c
# End Source File
# Begin Source File

SOURCE=..\VC1_reference_decoder\shared\vc1cropmv.c
# End Source File
# Begin Source File

SOURCE=..\VC1_reference_decoder\shared\vc1deblock.c
# End Source File
# Begin Source File

SOURCE=..\VC1_reference_decoder\shared\vc1debug.c
# End Source File
# Begin Source File

SOURCE=..\VC1_reference_decoder\decoder\vc1dec.c
# End Source File
# Begin Source File

SOURCE=..\VC1_reference_decoder\decoder\vc1dec3dh.c
# End Source File
# Begin Source File

SOURCE=..\VC1_reference_decoder\decoder\vc1dec3dhtab.c
# End Source File
# Begin Source File

SOURCE=..\VC1_reference_decoder\decoder\vc1decbit.c
# End Source File
# Begin Source File

SOURCE=..\VC1_reference_decoder\decoder\vc1decbitpl.c
# End Source File
# Begin Source File

SOURCE=..\VC1_reference_decoder\decoder\vc1decbitpltab.c
# End Source File
# Begin Source File

SOURCE=..\VC1_reference_decoder\decoder\vc1decblk.c
# End Source File
# Begin Source File

SOURCE=..\VC1_reference_decoder\decoder\vc1decblktab.c
# End Source File
# Begin Source File

SOURCE=..\VC1_reference_decoder\decoder\vc1decent.c
# End Source File
# Begin Source File

SOURCE=..\VC1_reference_decoder\decoder\vc1decmb.c
# End Source File
# Begin Source File

SOURCE=..\VC1_reference_decoder\decoder\vc1decmbtab.c
# End Source File
# Begin Source File

SOURCE=..\VC1_reference_decoder\decoder\vc1decmv.c
# End Source File
# Begin Source File

SOURCE=..\VC1_reference_decoder\decoder\vc1decpic.c
# End Source File
# Begin Source File

SOURCE=..\VC1_reference_decoder\decoder\vc1decpictab.c
# End Source File
# Begin Source File

SOURCE=..\VC1_reference_decoder\decoder\vc1decseq.c
# End Source File
# Begin Source File

SOURCE=..\VC1_reference_decoder\decoder\vc1decslice.c
# End Source File
# Begin Source File

SOURCE=..\VC1_reference_decoder\decoder\vc1deczz.c
# End Source File
# Begin Source File

SOURCE=..\VC1_reference_decoder\shared\vc1derivemv.c
# End Source File
# Begin Source File

SOURCE=..\VC1_reference_decoder\shared\vc1gentab.c
# End Source File
# Begin Source File

SOURCE=..\VC1_reference_decoder\shared\vc1hrd.c
# End Source File
# Begin Source File

SOURCE=..\VC1_reference_decoder\shared\vc1interp.c
# End Source File
# Begin Source File

SOURCE=..\VC1_reference_decoder\shared\vc1iquant.c
# End Source File
# Begin Source File

SOURCE=..\VC1_reference_decoder\shared\vc1itrans.c
# End Source File
# Begin Source File

SOURCE=..\VC1_reference_decoder\shared\vc1pred.c
# End Source File
# Begin Source File

SOURCE=..\VC1_reference_decoder\shared\vc1predcbp.c
# End Source File
# Begin Source File

SOURCE=..\VC1_reference_decoder\shared\vc1preddcac.c
# End Source File
# Begin Source File

SOURCE=..\VC1_reference_decoder\shared\vc1predmv.c
# End Source File
# Begin Source File

SOURCE=..\VC1_reference_decoder\shared\vc1recon.c
# End Source File
# Begin Source File

SOURCE=..\VC1_reference_decoder\shared\vc1scalemv.c
# End Source File
# Begin Source File

SOURCE=..\VC1_reference_decoder\shared\vc1smooth.c
# End Source File
# Begin Source File

SOURCE=..\VC1_reference_decoder\shared\vc1tools.c
# End Source File
# Begin Source File

SOURCE=..\VC1_reference_decoder\shared\vc1zztab.c
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=..\VC1_reference_decoder\decoder\decfile.h
# End Source File
# Begin Source File

SOURCE=..\VC1_reference_decoder\decoder\decopts.h
# End Source File
# Begin Source File

SOURCE=..\VC1_reference_decoder\shared\rcv.h
# End Source File
# Begin Source File

SOURCE=..\VC1_reference_decoder\shared\rdopts.h
# End Source File
# Begin Source File

SOURCE=..\VC1_reference_decoder\shared\vc13dhtab.h
# End Source File
# Begin Source File

SOURCE=..\VC1_reference_decoder\shared\vc1cropmv.h
# End Source File
# Begin Source File

SOURCE=..\VC1_reference_decoder\shared\vc1deblock.h
# End Source File
# Begin Source File

SOURCE=..\VC1_reference_decoder\shared\vc1debug.h
# End Source File
# Begin Source File

SOURCE=..\VC1_reference_decoder\decoder\vc1dec.h
# End Source File
# Begin Source File

SOURCE=..\VC1_reference_decoder\decoder\vc1dec3dh.h
# End Source File
# Begin Source File

SOURCE=..\VC1_reference_decoder\decoder\vc1dec3dhtab.h
# End Source File
# Begin Source File

SOURCE=..\VC1_reference_decoder\decoder\vc1decbit.h
# End Source File
# Begin Source File

SOURCE=..\VC1_reference_decoder\decoder\vc1decbitpl.h
# End Source File
# Begin Source File

SOURCE=..\VC1_reference_decoder\decoder\vc1decbitpltab.h
# End Source File
# Begin Source File

SOURCE=..\VC1_reference_decoder\decoder\vc1decblk.h
# End Source File
# Begin Source File

SOURCE=..\VC1_reference_decoder\decoder\vc1decblktab.h
# End Source File
# Begin Source File

SOURCE=..\VC1_reference_decoder\decoder\vc1decent.h
# End Source File
# Begin Source File

SOURCE=..\VC1_reference_decoder\decoder\vc1decmb.h
# End Source File
# Begin Source File

SOURCE=..\VC1_reference_decoder\decoder\vc1decmbtab.h
# End Source File
# Begin Source File

SOURCE=..\VC1_reference_decoder\decoder\vc1decmv.h
# End Source File
# Begin Source File

SOURCE=..\VC1_reference_decoder\decoder\vc1decpic.h
# End Source File
# Begin Source File

SOURCE=..\VC1_reference_decoder\decoder\vc1decpictab.h
# End Source File
# Begin Source File

SOURCE=..\VC1_reference_decoder\decoder\vc1decseq.h
# End Source File
# Begin Source File

SOURCE=..\VC1_reference_decoder\decoder\vc1decslice.h
# End Source File
# Begin Source File

SOURCE=..\VC1_reference_decoder\decoder\vc1deczz.h
# End Source File
# Begin Source File

SOURCE=..\VC1_reference_decoder\shared\vc1derivemv.h
# End Source File
# Begin Source File

SOURCE=..\VC1_reference_decoder\shared\vc1gentab.h
# End Source File
# Begin Source File

SOURCE=..\VC1_reference_decoder\shared\vc1hrd.h
# End Source File
# Begin Source File

SOURCE=..\VC1_reference_decoder\shared\vc1interp.h
# End Source File
# Begin Source File

SOURCE=..\VC1_reference_decoder\shared\vc1iquant.h
# End Source File
# Begin Source File

SOURCE=..\VC1_reference_decoder\shared\vc1itrans.h
# End Source File
# Begin Source File

SOURCE=..\VC1_reference_decoder\shared\vc1pred.h
# End Source File
# Begin Source File

SOURCE=..\VC1_reference_decoder\shared\vc1predcbp.h
# End Source File
# Begin Source File

SOURCE=..\VC1_reference_decoder\shared\vc1preddcac.h
# End Source File
# Begin Source File

SOURCE=..\VC1_reference_decoder\shared\vc1predmv.h
# End Source File
# Begin Source File

SOURCE=..\VC1_reference_decoder\shared\vc1recon.h
# End Source File
# Begin Source File

SOURCE=..\VC1_reference_decoder\shared\vc1scalemv.h
# End Source File
# Begin Source File

SOURCE=..\VC1_reference_decoder\shared\vc1smooth.h
# End Source File
# Begin Source File

SOURCE=..\VC1_reference_decoder\shared\vc1tools.h
# End Source File
# Begin Source File

SOURCE=..\VC1_reference_decoder\shared\vc1types.h
# End Source File
# Begin Source File

SOURCE=..\VC1_reference_decoder\shared\vc1zztab.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# End Group
# End Target
# End Project
