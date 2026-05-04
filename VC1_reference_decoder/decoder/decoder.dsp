# Microsoft Developer Studio Project File - Name="decoder" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Console Application" 0x0103

CFG=decoder - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "decoder.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "decoder.mak" CFG="decoder - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "decoder - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "decoder - Win32 Debug" (based on "Win32 (x86) Console Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "decoder - Win32 Release"

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
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /W4 /GX /O2 /I "..\shared" /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /FR /YX /FD /c
# ADD BASE RSC /l 0x809 /d "NDEBUG"
# ADD RSC /l 0x809 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386

!ELSEIF  "$(CFG)" == "decoder - Win32 Debug"

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
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /GZ /c
# ADD CPP /nologo /W4 /Gm /GX /ZI /Od /I "..\shared" /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /FR /YX /FD /GZ /c
# ADD BASE RSC /l 0x809 /d "_DEBUG"
# ADD RSC /l 0x809 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept

!ENDIF 

# Begin Target

# Name "decoder - Win32 Release"
# Name "decoder - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\decfile.c
# End Source File
# Begin Source File

SOURCE=.\decoder.c
# End Source File
# Begin Source File

SOURCE=.\decopts.c
# End Source File
# Begin Source File

SOURCE=..\shared\rdopts.c
# End Source File
# Begin Source File

SOURCE=..\shared\vc13dhtab.c
# End Source File
# Begin Source File

SOURCE=..\shared\vc1cropmv.c
# End Source File
# Begin Source File

SOURCE=..\shared\vc1deblock.c
# End Source File
# Begin Source File

SOURCE=..\shared\vc1debug.c
# End Source File
# Begin Source File

SOURCE=.\vc1dec.c
# End Source File
# Begin Source File

SOURCE=.\vc1dec3dh.c
# End Source File
# Begin Source File

SOURCE=.\vc1dec3dhtab.c
# End Source File
# Begin Source File

SOURCE=.\vc1decbit.c
# End Source File
# Begin Source File

SOURCE=.\vc1decbitpl.c
# End Source File
# Begin Source File

SOURCE=.\vc1decbitpltab.c
# End Source File
# Begin Source File

SOURCE=.\vc1decblk.c
# End Source File
# Begin Source File

SOURCE=.\vc1decblktab.c
# End Source File
# Begin Source File

SOURCE=.\vc1decent.c
# End Source File
# Begin Source File

SOURCE=.\vc1decmb.c
# End Source File
# Begin Source File

SOURCE=.\vc1decmbtab.c
# End Source File
# Begin Source File

SOURCE=.\vc1decmv.c
# End Source File
# Begin Source File

SOURCE=.\vc1decpic.c
# End Source File
# Begin Source File

SOURCE=.\vc1decpictab.c
# End Source File
# Begin Source File

SOURCE=.\vc1decseq.c
# End Source File
# Begin Source File

SOURCE=.\vc1decslice.c
# End Source File
# Begin Source File

SOURCE=.\vc1deczz.c
# End Source File
# Begin Source File

SOURCE=..\shared\vc1derivemv.c
# End Source File
# Begin Source File

SOURCE=..\shared\vc1gentab.c
# End Source File
# Begin Source File

SOURCE=..\shared\vc1hrd.c
# End Source File
# Begin Source File

SOURCE=..\shared\vc1interp.c
# End Source File
# Begin Source File

SOURCE=..\shared\vc1iquant.c
# End Source File
# Begin Source File

SOURCE=..\shared\vc1itrans.c
# End Source File
# Begin Source File

SOURCE=..\shared\vc1pred.c
# End Source File
# Begin Source File

SOURCE=..\shared\vc1predcbp.c
# End Source File
# Begin Source File

SOURCE=..\shared\vc1preddcac.c
# End Source File
# Begin Source File

SOURCE=..\shared\vc1predmv.c
# End Source File
# Begin Source File

SOURCE=..\shared\vc1recon.c
# End Source File
# Begin Source File

SOURCE=..\shared\vc1scalemv.c
# End Source File
# Begin Source File

SOURCE=..\shared\vc1smooth.c
# End Source File
# Begin Source File

SOURCE=..\shared\vc1tools.c
# End Source File
# Begin Source File

SOURCE=..\shared\vc1zztab.c
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\decfile.h
# End Source File
# Begin Source File

SOURCE=.\decopts.h
# End Source File
# Begin Source File

SOURCE=..\shared\rcv.h
# End Source File
# Begin Source File

SOURCE=..\shared\rdopts.h
# End Source File
# Begin Source File

SOURCE=..\shared\vc13dhtab.h
# End Source File
# Begin Source File

SOURCE=..\shared\vc1cropmv.h
# End Source File
# Begin Source File

SOURCE=..\shared\vc1deblock.h
# End Source File
# Begin Source File

SOURCE=..\shared\vc1debug.h
# End Source File
# Begin Source File

SOURCE=.\vc1dec.h
# End Source File
# Begin Source File

SOURCE=.\vc1dec3dh.h
# End Source File
# Begin Source File

SOURCE=.\vc1dec3dhtab.h
# End Source File
# Begin Source File

SOURCE=.\vc1decbit.h
# End Source File
# Begin Source File

SOURCE=.\vc1decbitpl.h
# End Source File
# Begin Source File

SOURCE=.\vc1decbitpltab.h
# End Source File
# Begin Source File

SOURCE=.\vc1decblk.h
# End Source File
# Begin Source File

SOURCE=.\vc1decblktab.h
# End Source File
# Begin Source File

SOURCE=.\vc1decent.h
# End Source File
# Begin Source File

SOURCE=.\vc1decmb.h
# End Source File
# Begin Source File

SOURCE=.\vc1decmbtab.h
# End Source File
# Begin Source File

SOURCE=.\vc1decmv.h
# End Source File
# Begin Source File

SOURCE=.\vc1decpic.h
# End Source File
# Begin Source File

SOURCE=.\vc1decpictab.h
# End Source File
# Begin Source File

SOURCE=.\vc1decseq.h
# End Source File
# Begin Source File

SOURCE=.\vc1decslice.h
# End Source File
# Begin Source File

SOURCE=.\vc1deczz.h
# End Source File
# Begin Source File

SOURCE=..\shared\vc1derivemv.h
# End Source File
# Begin Source File

SOURCE=..\shared\vc1gentab.h
# End Source File
# Begin Source File

SOURCE=..\shared\vc1hrd.h
# End Source File
# Begin Source File

SOURCE=..\shared\vc1interp.h
# End Source File
# Begin Source File

SOURCE=..\shared\vc1iquant.h
# End Source File
# Begin Source File

SOURCE=..\shared\vc1itrans.h
# End Source File
# Begin Source File

SOURCE=..\shared\vc1pred.h
# End Source File
# Begin Source File

SOURCE=..\shared\vc1predcbp.h
# End Source File
# Begin Source File

SOURCE=..\shared\vc1preddcac.h
# End Source File
# Begin Source File

SOURCE=..\shared\vc1predmv.h
# End Source File
# Begin Source File

SOURCE=..\shared\vc1recon.h
# End Source File
# Begin Source File

SOURCE=..\shared\vc1scalemv.h
# End Source File
# Begin Source File

SOURCE=..\shared\vc1smooth.h
# End Source File
# Begin Source File

SOURCE=..\shared\vc1tools.h
# End Source File
# Begin Source File

SOURCE=..\shared\vc1types.h
# End Source File
# Begin Source File

SOURCE=..\shared\vc1zztab.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# End Group
# End Target
# End Project
