# Microsoft Developer Studio Project File - Name="hdhomerun" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Console Application" 0x0103

CFG=hdhomerun - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "hdhomerun.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "hdhomerun.mak" CFG="hdhomerun - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "hdhomerun - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "hdhomerun - Win32 Debug" (based on "Win32 (x86) Console Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "hdhomerun - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "hdhomerun___Win32_Release"
# PROP BASE Intermediate_Dir "hdhomerun___Win32_Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "hdhomerun___Win32_Release"
# PROP Intermediate_Dir "hdhomerun___Win32_Release"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /c
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386

!ELSEIF  "$(CFG)" == "hdhomerun - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "hdhomerun___Win32_Debug"
# PROP BASE Intermediate_Dir "hdhomerun___Win32_Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "hdhomerun___Win32_Debug"
# PROP Intermediate_Dir "hdhomerun___Win32_Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /GZ /c
# ADD CPP /nologo /W3 /Gm /GX /ZI /Od /I "libhdhomerun\pthread" /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /D "__WINDOWS__" /YX /FD /GZ /c
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept
# ADD LINK32 Iphlpapi.lib wsock32.lib pthreadVSE2.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /out:"libhdhomerun/hdhomerun.exe" /pdbtype:sept /libpath:"libhdhomerun\pthread"

!ENDIF 

# Begin Target

# Name "hdhomerun - Win32 Release"
# Name "hdhomerun - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\libhdhomerun\hdhomerun_config.cpp
# End Source File
# Begin Source File

SOURCE=.\libhdhomerun\hdhomerun_control.cpp
# End Source File
# Begin Source File

SOURCE=.\libhdhomerun\hdhomerun_device.cpp
# End Source File
# Begin Source File

SOURCE=.\libhdhomerun\hdhomerun_discover.cpp
# End Source File
# Begin Source File

SOURCE=.\libhdhomerun\hdhomerun_pkt.cpp
# End Source File
# Begin Source File

SOURCE=.\libhdhomerun\hdhomerun_ts_demux.cpp
# End Source File
# Begin Source File

SOURCE=.\libhdhomerun\hdhomerun_ts_patfix.cpp
# End Source File
# Begin Source File

SOURCE=.\libhdhomerun\hdhomerun_ts_program.cpp
# End Source File
# Begin Source File

SOURCE=.\libhdhomerun\hdhomerun_video.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\libhdhomerun\hdhomerun.h
# End Source File
# Begin Source File

SOURCE=.\libhdhomerun\hdhomerun_control.h
# End Source File
# Begin Source File

SOURCE=.\libhdhomerun\hdhomerun_device.h
# End Source File
# Begin Source File

SOURCE=.\libhdhomerun\hdhomerun_discover.h
# End Source File
# Begin Source File

SOURCE=.\libhdhomerun\hdhomerun_os.h
# End Source File
# Begin Source File

SOURCE=.\libhdhomerun\hdhomerun_pkt.h
# End Source File
# Begin Source File

SOURCE=.\libhdhomerun\hdhomerun_ts_demux.h
# End Source File
# Begin Source File

SOURCE=.\libhdhomerun\hdhomerun_ts_patfix.h
# End Source File
# Begin Source File

SOURCE=.\libhdhomerun\hdhomerun_ts_program.h
# End Source File
# Begin Source File

SOURCE=.\libhdhomerun\hdhomerun_video.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# End Group
# End Target
# End Project
