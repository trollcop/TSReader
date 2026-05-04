# Microsoft Developer Studio Project File - Name="TSReader_Twinhan2" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=TSReader_Twinhan2 - Win32 DVBC CI Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "TSReader_Twinhan2.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "TSReader_Twinhan2.mak" CFG="TSReader_Twinhan2 - Win32 DVBC CI Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "TSReader_Twinhan2 - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "TSReader_Twinhan2 - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "TSReader_Twinhan2 - Win32 1020 DSS Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "TSReader_Twinhan2 - Win32 1030 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "TSReader_Twinhan2 - Win32 1020 DSS Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "TSReader_Twinhan2 - Win32 1030 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "TSReader_Twinhan2 - Win32 DVBT Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "TSReader_Twinhan2 - Win32 DVBT Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "TSReader_Twinhan2 - Win32 NoTune Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "TSReader_Twinhan2 - Win32 NoTune Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "TSReader_Twinhan2 - Win32 DVBC Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "TSReader_Twinhan2 - Win32 Pinnacle Sat CI Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "TSReader_Twinhan2 - Win32 8VSB Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "TSReader_Twinhan2 - Win32 DVBC Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "TSReader_Twinhan2 - Win32 8VSB Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "TSReader_Twinhan2 - Win32 DVBC CI Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "TSReader_Twinhan2 - Win32 DVBC CI Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "TSReader_Twinhan2 - Win32 DVBT CI Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=xicl6.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "TSReader_Twinhan2 - Win32 Release"

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
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /MT /W3 /GX /O2 /I "..\include" /I "C:\SDKs\ImgSource" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "TWINHAN1020" /D "DVB" /D "DVBS" /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=xilink6.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /machine:I386
# ADD LINK32 ../TSReader_SourceHelper.lib Setupapi.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /machine:I386 /out:"../Sources/TSReader_Twinhan1020.dll"

!ELSEIF  "$(CFG)" == "TSReader_Twinhan2 - Win32 Debug"

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
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /FD /GZ /c
# ADD CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /I "..\include" /I "C:\SDKs\ImgSource" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "TWINHAN1020" /D "DVB" /D "DVBS" /YX /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=xilink6.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /debug /machine:I386 /pdbtype:sept
# ADD LINK32 ../TSReader_SourceHelper.lib Setupapi.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /debug /machine:I386 /out:"../Sources/TSReader_Twinhan1020.dll" /pdbtype:sept

!ELSEIF  "$(CFG)" == "TSReader_Twinhan2 - Win32 1020 DSS Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "TSReader_Twinhan2___Win32_1020_DSS_Debug"
# PROP BASE Intermediate_Dir "TSReader_Twinhan2___Win32_1020_DSS_Debug"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "TSReader_Twinhan2___Win32_1020_DSS_Debug"
# PROP Intermediate_Dir "TSReader_Twinhan2___Win32_1020_DSS_Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /I "..\include" /I "C:\SDKs\ImgSource" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "TWINHAN1020" /D "DVB" /YX /FD /GZ /c
# ADD CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /I "..\include" /I "C:\SDKs\ImgSource" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "TWINHAN1020" /D "DSS" /D "DVBS" /YX /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=xilink6.exe
# ADD BASE LINK32 ../TSReader_SourceHelper.lib Setupapi.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /debug /machine:I386 /out:"../Sources/TSReader_Twinhan1020.dll" /pdbtype:sept
# ADD LINK32 ../TSReader_SourceHelper.lib Setupapi.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /debug /machine:I386 /out:"../Sources/TSReader_Twinhan1020dss.dll" /pdbtype:sept

!ELSEIF  "$(CFG)" == "TSReader_Twinhan2 - Win32 1030 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "TSReader_Twinhan2___Win32_1030_Debug"
# PROP BASE Intermediate_Dir "TSReader_Twinhan2___Win32_1030_Debug"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "TSReader_Twinhan2___Win32_1030_Debug"
# PROP Intermediate_Dir "TSReader_Twinhan2___Win32_1030_Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /I "..\include" /I "C:\SDKs\ImgSource" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "TWINHAN1020" /D "DVB" /YX /FD /GZ /c
# ADD CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /I "..\include" /I "C:\SDKs\ImgSource" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "TWINHAN1030" /D "DVB" /D "DVBS" /D "CI_SUPPORT" /YX /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=xilink6.exe
# ADD BASE LINK32 ../TSReader_SourceHelper.lib Setupapi.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /debug /machine:I386 /out:"../Sources/TSReader_Twinhan1020.dll" /pdbtype:sept
# ADD LINK32 ../TSReader_SourceHelper.lib Setupapi.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /debug /machine:I386 /out:"../Sources/TSReader_Twinhan1030.dll" /pdbtype:sept

!ELSEIF  "$(CFG)" == "TSReader_Twinhan2 - Win32 1020 DSS Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "TSReader_Twinhan2___Win32_1020_DSS_Release"
# PROP BASE Intermediate_Dir "TSReader_Twinhan2___Win32_1020_DSS_Release"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "TSReader_Twinhan2___Win32_1020_DSS_Release"
# PROP Intermediate_Dir "TSReader_Twinhan2___Win32_1020_DSS_Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /I "..\include" /I "C:\SDKs\ImgSource" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "TWINHAN1020" /D "DVB" /YX /FD /c
# ADD CPP /nologo /MT /W3 /GX /O2 /I "..\include" /I "C:\SDKs\ImgSource" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "TWINHAN1020" /D "DSS" /D "DVBS" /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=xilink6.exe
# ADD BASE LINK32 ../TSReader_SourceHelper.lib Setupapi.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /machine:I386 /out:"../Sources/TSReader_Twinhan1020.dll"
# ADD LINK32 ../TSReader_SourceHelper.lib Setupapi.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /machine:I386 /out:"../Sources/TSReader_Twinhan1020dss.dll"

!ELSEIF  "$(CFG)" == "TSReader_Twinhan2 - Win32 1030 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "TSReader_Twinhan2___Win32_1030_Release"
# PROP BASE Intermediate_Dir "TSReader_Twinhan2___Win32_1030_Release"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "TSReader_Twinhan2___Win32_1030_Release"
# PROP Intermediate_Dir "TSReader_Twinhan2___Win32_1030_Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /I "..\include" /I "C:\SDKs\ImgSource" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "TWINHAN1020" /D "DVB" /YX /FD /c
# ADD CPP /nologo /MT /W3 /GX /O2 /I "..\include" /I "C:\SDKs\ImgSource" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "TWINHAN1030" /D "DVB" /D "DVBS" /D "CI_SUPPORT" /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=xilink6.exe
# ADD BASE LINK32 ../TSReader_SourceHelper.lib Setupapi.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /machine:I386 /out:"../Sources/TSReader_Twinhan1020.dll"
# ADD LINK32 ../TSReader_SourceHelper.lib Setupapi.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /machine:I386 /out:"../Sources/TSReader_Twinhan1030.dll"

!ELSEIF  "$(CFG)" == "TSReader_Twinhan2 - Win32 DVBT Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "TSReader_Twinhan2___Win32_DVBT_Debug"
# PROP BASE Intermediate_Dir "TSReader_Twinhan2___Win32_DVBT_Debug"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "TSReader_Twinhan2___Win32_DVBT_Debug"
# PROP Intermediate_Dir "TSReader_Twinhan2___Win32_DVBT_Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /I "..\include" /I "C:\SDKs\ImgSource" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "TWINHAN1020" /D "DVB" /YX /FD /GZ /c
# ADD CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /I "..\include" /I "C:\SDKs\ImgSource" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "DVBT" /YX /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=xilink6.exe
# ADD BASE LINK32 ../TSReader_SourceHelper.lib Setupapi.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /debug /machine:I386 /out:"../Sources/TSReader_Twinhan1020.dll" /pdbtype:sept
# ADD LINK32 ../TSReader_SourceHelper.lib Setupapi.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /debug /machine:I386 /out:"../Sources/TSReader_TwinhanDTT.dll" /pdbtype:sept

!ELSEIF  "$(CFG)" == "TSReader_Twinhan2 - Win32 DVBT Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "TSReader_Twinhan2___Win32_DVBT_Release"
# PROP BASE Intermediate_Dir "TSReader_Twinhan2___Win32_DVBT_Release"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "TSReader_Twinhan2___Win32_DVBT_Release"
# PROP Intermediate_Dir "TSReader_Twinhan2___Win32_DVBT_Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /I "..\include" /I "C:\SDKs\ImgSource" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "TWINHAN1020" /D "DVB" /YX /FD /c
# ADD CPP /nologo /MT /W3 /GX /O2 /I "..\include" /I "C:\SDKs\ImgSource" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "DVBT" /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=xilink6.exe
# ADD BASE LINK32 ../TSReader_SourceHelper.lib Setupapi.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /machine:I386 /out:"../Sources/TSReader_Twinhan1020.dll"
# ADD LINK32 ../TSReader_SourceHelper.lib Setupapi.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /machine:I386 /out:"../Sources/TSReader_TwinhanDTT.dll"

!ELSEIF  "$(CFG)" == "TSReader_Twinhan2 - Win32 NoTune Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "TSReader_Twinhan2___Win32_NoTune_Debug"
# PROP BASE Intermediate_Dir "TSReader_Twinhan2___Win32_NoTune_Debug"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "TSReader_Twinhan2___Win32_NoTune_Debug"
# PROP Intermediate_Dir "TSReader_Twinhan2___Win32_NoTune_Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /I "..\include" /I "C:\SDKs\ImgSource" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "TWINHAN1020" /D "DVB" /YX /FD /GZ /c
# ADD CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /I "..\include" /I "C:\SDKs\ImgSource" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "NOTUNE" /D "DVB" /YX /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=xilink6.exe
# ADD BASE LINK32 ../TSReader_SourceHelper.lib Setupapi.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /debug /machine:I386 /out:"../Sources/TSReader_Twinhan1020.dll" /pdbtype:sept
# ADD LINK32 ../TSReader_SourceHelper.lib Setupapi.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /debug /machine:I386 /out:"../Sources/TSReader_TwinhanNoTune.dll" /pdbtype:sept

!ELSEIF  "$(CFG)" == "TSReader_Twinhan2 - Win32 NoTune Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "TSReader_Twinhan2___Win32_NoTune_Release"
# PROP BASE Intermediate_Dir "TSReader_Twinhan2___Win32_NoTune_Release"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "TSReader_Twinhan2___Win32_NoTune_Release"
# PROP Intermediate_Dir "TSReader_Twinhan2___Win32_NoTune_Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /I "..\include" /I "C:\SDKs\ImgSource" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "TWINHAN1020" /D "DVB" /YX /FD /c
# ADD CPP /nologo /MT /W3 /GX /O2 /I "..\include" /I "C:\SDKs\ImgSource" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "NOTUNE" /D "DVB" /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=xilink6.exe
# ADD BASE LINK32 ../TSReader_SourceHelper.lib Setupapi.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /machine:I386 /out:"../Sources/TSReader_Twinhan1020.dll"
# ADD LINK32 ../TSReader_SourceHelper.lib Setupapi.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /machine:I386 /out:"../Sources/TSReader_TwinhanNoTune.dll"

!ELSEIF  "$(CFG)" == "TSReader_Twinhan2 - Win32 DVBC Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "TSReader_Twinhan2___Win32_DVBC_Debug"
# PROP BASE Intermediate_Dir "TSReader_Twinhan2___Win32_DVBC_Debug"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "TSReader_Twinhan2___Win32_DVBC_Debug"
# PROP Intermediate_Dir "TSReader_Twinhan2___Win32_DVBC_Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /I "..\include" /I "C:\SDKs\ImgSource" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "TWINHAN1020" /D "DVB" /D "DVBS" /YX /FD /GZ /c
# ADD CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /I "..\include" /I "C:\SDKs\ImgSource" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "DVBC" /YX /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=xilink6.exe
# ADD BASE LINK32 ../TSReader_SourceHelper.lib Setupapi.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /debug /machine:I386 /out:"../Sources/TSReader_Twinhan1020.dll" /pdbtype:sept
# ADD LINK32 ../TSReader_SourceHelper.lib Setupapi.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /debug /machine:I386 /out:"../Sources/TSReader_TwinhanDCT.dll" /pdbtype:sept

!ELSEIF  "$(CFG)" == "TSReader_Twinhan2 - Win32 Pinnacle Sat CI Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "TSReader_Twinhan2___Win32_Pinnacle_Sat_CI_Release"
# PROP BASE Intermediate_Dir "TSReader_Twinhan2___Win32_Pinnacle_Sat_CI_Release"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "TSReader_Twinhan2___Win32_Pinnacle_Sat_CI_Release"
# PROP Intermediate_Dir "TSReader_Twinhan2___Win32_Pinnacle_Sat_CI_Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /I "..\include" /I "C:\SDKs\ImgSource" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "TWINHAN1030" /D "DVB" /D "DVBS" /YX /FD /c
# ADD CPP /nologo /MT /W3 /GX /O2 /I "..\include" /I "C:\SDKs\ImgSource" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "TWINHAN1030" /D "DVB" /D "DVBS" /D "PINNACLE" /D "CI_SUPPORT" /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=xilink6.exe
# ADD BASE LINK32 ../TSReader_SourceHelper.lib Setupapi.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /machine:I386 /out:"../Sources/TSReader_Twinhan1030.dll"
# ADD LINK32 ../TSReader_SourceHelper.lib Setupapi.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /machine:I386 /out:"../Sources/TSReader_PinnacleSatCI.dll"

!ELSEIF  "$(CFG)" == "TSReader_Twinhan2 - Win32 8VSB Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "TSReader_Twinhan2___Win32_8VSB_Debug"
# PROP BASE Intermediate_Dir "TSReader_Twinhan2___Win32_8VSB_Debug"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "TSReader_Twinhan2___Win32_8VSB_Debug"
# PROP Intermediate_Dir "TSReader_Twinhan2___Win32_8VSB_Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /I "..\include" /I "C:\SDKs\ImgSource" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "TWINHAN1020" /D "DVB" /D "DVBS" /YX /FD /GZ /c
# ADD CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /I "..\include" /I "C:\SDKs\ImgSource" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "EIGHTVSB" /D "ALPSTUNER" /YX /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=xilink6.exe
# ADD BASE LINK32 ../TSReader_SourceHelper.lib Setupapi.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /debug /machine:I386 /out:"../Sources/TSReader_Twinhan1020.dll" /pdbtype:sept
# ADD LINK32 ../TSReader_SourceHelper.lib Setupapi.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /debug /machine:I386 /out:"../Sources/TSReader_Twinhan8VSB.dll" /pdbtype:sept

!ELSEIF  "$(CFG)" == "TSReader_Twinhan2 - Win32 DVBC Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "TSReader_Twinhan2___Win32_DVBC_Release"
# PROP BASE Intermediate_Dir "TSReader_Twinhan2___Win32_DVBC_Release"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "TSReader_Twinhan2___Win32_DVBC_Release"
# PROP Intermediate_Dir "TSReader_Twinhan2___Win32_DVBC_Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /I "..\include" /I "C:\SDKs\ImgSource" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "TWINHAN1020" /D "DVB" /D "DVBS" /YX /FD /c
# ADD CPP /nologo /MT /W3 /GX /O2 /I "..\include" /I "C:\SDKs\ImgSource" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "DVBC" /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=xilink6.exe
# ADD BASE LINK32 ../TSReader_SourceHelper.lib Setupapi.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /machine:I386 /out:"../Sources/TSReader_Twinhan1020.dll"
# ADD LINK32 ../TSReader_SourceHelper.lib Setupapi.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /machine:I386 /out:"../Sources/TSReader_TwinhanDCT.dll"

!ELSEIF  "$(CFG)" == "TSReader_Twinhan2 - Win32 8VSB Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "TSReader_Twinhan2___Win32_8VSB_Release"
# PROP BASE Intermediate_Dir "TSReader_Twinhan2___Win32_8VSB_Release"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "TSReader_Twinhan2___Win32_8VSB_Release"
# PROP Intermediate_Dir "TSReader_Twinhan2___Win32_8VSB_Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /I "..\include" /I "C:\SDKs\ImgSource" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "TWINHAN1020" /D "DVB" /D "DVBS" /YX /FD /c
# ADD CPP /nologo /MT /W3 /GX /O2 /I "..\include" /I "C:\SDKs\ImgSource" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "EIGHTVSB" /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=xilink6.exe
# ADD BASE LINK32 ../TSReader_SourceHelper.lib Setupapi.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /machine:I386 /out:"../Sources/TSReader_Twinhan1020.dll"
# ADD LINK32 ../TSReader_SourceHelper.lib Setupapi.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /machine:I386 /out:"../Sources/TSReader_Twinhan8VSB.dll"

!ELSEIF  "$(CFG)" == "TSReader_Twinhan2 - Win32 DVBC CI Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "TSReader_Twinhan2___Win32_DVBC_CI_Debug"
# PROP BASE Intermediate_Dir "TSReader_Twinhan2___Win32_DVBC_CI_Debug"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "TSReader_Twinhan2___Win32_DVBC_CI_Debug"
# PROP Intermediate_Dir "TSReader_Twinhan2___Win32_DVBC_CI_Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /I "..\include" /I "C:\SDKs\ImgSource" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "DVBC" /YX /FD /GZ /c
# ADD CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /I "..\include" /I "C:\SDKs\ImgSource" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "DVBC" /D "CI_DVBC" /D "CI_SUPPORT" /YX /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=xilink6.exe
# ADD BASE LINK32 ../TSReader_SourceHelper.lib Setupapi.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /debug /machine:I386 /out:"../Sources/TSReader_TwinhanDCT.dll" /pdbtype:sept
# ADD LINK32 ../TSReader_SourceHelper.lib Setupapi.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /debug /machine:I386 /out:"../Sources/TSReader_TwinhanDCT-CI.dll" /pdbtype:sept

!ELSEIF  "$(CFG)" == "TSReader_Twinhan2 - Win32 DVBC CI Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "TSReader_Twinhan2___Win32_DVBC_CI_Release"
# PROP BASE Intermediate_Dir "TSReader_Twinhan2___Win32_DVBC_CI_Release"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "TSReader_Twinhan2___Win32_DVBC_CI_Release"
# PROP Intermediate_Dir "TSReader_Twinhan2___Win32_DVBC_CI_Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /I "..\include" /I "C:\SDKs\ImgSource" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "DVBC" /YX /FD /c
# ADD CPP /nologo /MT /W3 /GX /O2 /I "..\include" /I "C:\SDKs\ImgSource" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "DVBC" /D "CI_DVBC" /D "CI_SUPPORT" /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=xilink6.exe
# ADD BASE LINK32 ../TSReader_SourceHelper.lib Setupapi.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /machine:I386 /out:"../Sources/TSReader_TwinhanDCT.dll"
# ADD LINK32 ../TSReader_SourceHelper.lib Setupapi.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /machine:I386 /out:"../Sources/TSReader_TwinhanDCT-CI.dll"

!ELSEIF  "$(CFG)" == "TSReader_Twinhan2 - Win32 DVBT CI Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "TSReader_Twinhan2___Win32_DVBT_CI_Release"
# PROP BASE Intermediate_Dir "TSReader_Twinhan2___Win32_DVBT_CI_Release"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "TSReader_Twinhan2___Win32_DVBT_CI_Release"
# PROP Intermediate_Dir "TSReader_Twinhan2___Win32_DVBT_CI_Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /I "..\include" /I "C:\SDKs\ImgSource" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "DVBT" /YX /FD /c
# ADD CPP /nologo /MT /W3 /GX /O2 /I "..\include" /I "C:\SDKs\ImgSource" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "DVBT" /D "CI_DVBT" /D "CI_SUPPORT" /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=xilink6.exe
# ADD BASE LINK32 ../TSReader_SourceHelper.lib Setupapi.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /machine:I386 /out:"../Sources/TSReader_TwinhanDTT.dll"
# ADD LINK32 ../TSReader_SourceHelper.lib Setupapi.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /machine:I386 /out:"../Sources/TSReader_TwinhanDTT-CI.dll"

!ENDIF 

# Begin Target

# Name "TSReader_Twinhan2 - Win32 Release"
# Name "TSReader_Twinhan2 - Win32 Debug"
# Name "TSReader_Twinhan2 - Win32 1020 DSS Debug"
# Name "TSReader_Twinhan2 - Win32 1030 Debug"
# Name "TSReader_Twinhan2 - Win32 1020 DSS Release"
# Name "TSReader_Twinhan2 - Win32 1030 Release"
# Name "TSReader_Twinhan2 - Win32 DVBT Debug"
# Name "TSReader_Twinhan2 - Win32 DVBT Release"
# Name "TSReader_Twinhan2 - Win32 NoTune Debug"
# Name "TSReader_Twinhan2 - Win32 NoTune Release"
# Name "TSReader_Twinhan2 - Win32 DVBC Debug"
# Name "TSReader_Twinhan2 - Win32 Pinnacle Sat CI Release"
# Name "TSReader_Twinhan2 - Win32 8VSB Debug"
# Name "TSReader_Twinhan2 - Win32 DVBC Release"
# Name "TSReader_Twinhan2 - Win32 8VSB Release"
# Name "TSReader_Twinhan2 - Win32 DVBC CI Debug"
# Name "TSReader_Twinhan2 - Win32 DVBC CI Release"
# Name "TSReader_Twinhan2 - Win32 DVBT CI Release"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\CAM.c
# End Source File
# Begin Source File

SOURCE=.\TSReader_Twinhan2.c
# End Source File
# Begin Source File

SOURCE=.\TSReader_Twinhan2.def
# End Source File
# Begin Source File

SOURCE=.\TSReader_Twinhan2.rc
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\TSReader_Twinhan2.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# End Group
# End Target
# End Project
