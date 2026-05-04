# Microsoft Developer Studio Project File - Name="TSReader_HDHomeRun" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=TSReader_HDHomeRun - Win32 QAM Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "TSReader_HDHomeRun.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "TSReader_HDHomeRun.mak" CFG="TSReader_HDHomeRun - Win32 QAM Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "TSReader_HDHomeRun - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "TSReader_HDHomeRun - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "TSReader_HDHomeRun - Win32 QAM Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "TSReader_HDHomeRun - Win32 QAM Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "TSReader_HDHomeRun - Win32 Release"

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
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "TSREADER_HDHOMERUN_EXPORTS" /YX /FD /c
# ADD CPP /nologo /MT /W3 /GX /O2 /I "libhdhomerun\pthread" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "TSREADER_HDHOMERUN_EXPORTS" /D "__WINDOWS__" /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /machine:I386
# ADD LINK32 ../TSReader_SourceHelper.lib Iphlpapi.lib wsock32.lib pthreadVSE2.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /machine:I386 /out:"../Sources/TSReader_HDHomeRun_8VSB.dll" /libpath:"libhdhomerun\pthread"

!ELSEIF  "$(CFG)" == "TSReader_HDHomeRun - Win32 Debug"

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
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "TSREADER_HDHOMERUN_EXPORTS" /YX /FD /GZ /c
# ADD CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /I "libhdhomerun\pthread" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "TSREADER_HDHOMERUN_EXPORTS" /D "__WINDOWS__" /YX /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /debug /machine:I386 /pdbtype:sept
# ADD LINK32 ../TSReader_SourceHelper.lib Iphlpapi.lib wsock32.lib pthreadVSE2.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /debug /machine:I386 /out:"../Sources/TSReader_HDHomeRun_8VSB.dll" /pdbtype:sept /libpath:"libhdhomerun\pthread"

!ELSEIF  "$(CFG)" == "TSReader_HDHomeRun - Win32 QAM Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "QAM Debug"
# PROP BASE Intermediate_Dir "QAM Debug"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "QAM Debug"
# PROP Intermediate_Dir "QAM Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "TSREADER_HDHOMERUN_EXPORTS" /YX /FD /GZ /c
# ADD CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /I "libhdhomerun\pthread" /D "QAM" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "TSREADER_HDHOMERUN_EXPORTS" /D "__WINDOWS__" /YX /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 ../TSReader_SourceHelper.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /debug /machine:I386 /out:"../Sources/TSReader_HDHomeRun_8VSB.dll" /pdbtype:sept
# ADD LINK32 ../TSReader_SourceHelper.lib Iphlpapi.lib wsock32.lib pthreadVSE2.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /debug /machine:I386 /out:"../Sources/TSReader_HDHomeRun_QAM.dll" /pdbtype:sept /libpath:"libhdhomerun\pthread"

!ELSEIF  "$(CFG)" == "TSReader_HDHomeRun - Win32 QAM Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "QAM Release"
# PROP BASE Intermediate_Dir "QAM Release"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "QAM Release"
# PROP Intermediate_Dir "QAM Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "TSREADER_HDHOMERUN_EXPORTS" /YX /FD /c
# ADD CPP /nologo /MT /W3 /GX /O2 /I "libhdhomerun\pthread" /D "QAM" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "TSREADER_HDHOMERUN_EXPORTS" /D "__WINDOWS__" /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 ../TSReader_SourceHelper.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /machine:I386 /out:"../Sources/TSReader_HDHomeRun_8VSB.dll"
# ADD LINK32 ../TSReader_SourceHelper.lib Iphlpapi.lib wsock32.lib pthreadVSE2.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /machine:I386 /out:"../Sources/TSReader_HDHomeRun_QAM.dll" /libpath:"libhdhomerun\pthread"

!ENDIF 

# Begin Target

# Name "TSReader_HDHomeRun - Win32 Release"
# Name "TSReader_HDHomeRun - Win32 Debug"
# Name "TSReader_HDHomeRun - Win32 QAM Debug"
# Name "TSReader_HDHomeRun - Win32 QAM Release"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\TSReader_HDHomeRun.cpp
# End Source File
# Begin Source File

SOURCE=.\TSReader_HDHomeRun.def
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\resource.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\TSReader_HDHomeRun.rc
# End Source File
# End Group
# Begin Group "libhdhomerun"

# PROP Default_Filter ".c,.h"
# Begin Source File

SOURCE=.\libhdhomerun\hdhomerun.h
# End Source File
# Begin Source File

SOURCE=.\libhdhomerun\hdhomerun_config.cpp
# End Source File
# Begin Source File

SOURCE=.\libhdhomerun\hdhomerun_control.cpp
# End Source File
# Begin Source File

SOURCE=.\libhdhomerun\hdhomerun_control.h
# End Source File
# Begin Source File

SOURCE=.\libhdhomerun\hdhomerun_device.cpp
# End Source File
# Begin Source File

SOURCE=.\libhdhomerun\hdhomerun_device.h
# End Source File
# Begin Source File

SOURCE=.\libhdhomerun\hdhomerun_discover.cpp
# End Source File
# Begin Source File

SOURCE=.\libhdhomerun\hdhomerun_discover.h
# End Source File
# Begin Source File

SOURCE=.\libhdhomerun\hdhomerun_os.h
# End Source File
# Begin Source File

SOURCE=.\libhdhomerun\hdhomerun_pkt.cpp
# End Source File
# Begin Source File

SOURCE=.\libhdhomerun\hdhomerun_pkt.h
# End Source File
# Begin Source File

SOURCE=.\libhdhomerun\hdhomerun_ts_demux.cpp
# End Source File
# Begin Source File

SOURCE=.\libhdhomerun\hdhomerun_ts_demux.h
# End Source File
# Begin Source File

SOURCE=.\libhdhomerun\hdhomerun_ts_patfix.cpp
# End Source File
# Begin Source File

SOURCE=.\libhdhomerun\hdhomerun_ts_patfix.h
# End Source File
# Begin Source File

SOURCE=.\libhdhomerun\hdhomerun_ts_program.cpp
# End Source File
# Begin Source File

SOURCE=.\libhdhomerun\hdhomerun_ts_program.h
# End Source File
# Begin Source File

SOURCE=.\libhdhomerun\hdhomerun_video.cpp
# End Source File
# Begin Source File

SOURCE=.\libhdhomerun\hdhomerun_video.h
# End Source File
# End Group
# End Target
# End Project
