# Microsoft Developer Studio Project File - Name="TSReader_DVBWorld" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=TSReader_DVBWorld - Win32 Match Box Pro Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "TSReader_DVBWorld.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "TSReader_DVBWorld.mak" CFG="TSReader_DVBWorld - Win32 Match Box Pro Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "TSReader_DVBWorld - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "TSReader_DVBWorld - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "TSReader_DVBWorld - Win32 DSS Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "TSReader_DVBWorld - Win32 DSS Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "TSReader_DVBWorld - Win32 Nextorm Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "TSReader_DVBWorld - Win32 Nextorm DSS Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "TSReader_DVBWorld - Win32 Match Box Pro Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "TSReader_DVBWorld - Win32 Match Box Pro Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "TSReader_DVBWorld - Win32 Release"

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
# ADD CPP /nologo /MT /W3 /GX /O2 /I "..\include" /I "C:\SDKs\ImgSource" /I "c:\sdks\cypress\inc" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /machine:I386
# ADD LINK32 ../TSReader_SourceHelper.lib CyAPI.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /machine:I386 /out:"../Sources/TSReader_DVBWorld_S2101.dll" /libpath:"c:\sdks\cypress\Lib\VC6_7"

!ELSEIF  "$(CFG)" == "TSReader_DVBWorld - Win32 Debug"

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
# ADD CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /I "..\include" /I "C:\SDKs\ImgSource" /I "c:\sdks\cypress\inc" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /debug /machine:I386 /pdbtype:sept
# ADD LINK32 ../TSReader_SourceHelper.lib CyAPI.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /debug /machine:I386 /out:"../Sources/TSReader_DVBWorld_S2101.dll" /pdbtype:sept /libpath:"c:\sdks\cypress\Lib\VC6_7"

!ELSEIF  "$(CFG)" == "TSReader_DVBWorld - Win32 DSS Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "TSReader_DVBWorld___Win32_DSS_Release"
# PROP BASE Intermediate_Dir "TSReader_DVBWorld___Win32_DSS_Release"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "TSReader_DVBWorld___Win32_DSS_Release"
# PROP Intermediate_Dir "TSReader_DVBWorld___Win32_DSS_Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /I "..\include" /I "C:\SDKs\ImgSource" /I "c:\sdks\cypress\inc" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /MT /W3 /GX /O2 /I "..\include" /I "C:\SDKs\ImgSource" /I "c:\sdks\cypress\inc" /D "DSS" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 ../TSReader_SourceHelper.lib CyAPI.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /machine:I386 /out:"../Sources/TSReader_DVBWorld_S2101.dll" /libpath:"c:\sdks\cypress\Lib\VC6_7"
# ADD LINK32 ../TSReader_SourceHelper.lib CyAPI.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /machine:I386 /out:"../Sources/TSReader_DVBWorld_S2101_DSS.dll" /libpath:"c:\sdks\cypress\Lib\VC6_7"

!ELSEIF  "$(CFG)" == "TSReader_DVBWorld - Win32 DSS Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "TSReader_DVBWorld___Win32_DSS_Debug"
# PROP BASE Intermediate_Dir "TSReader_DVBWorld___Win32_DSS_Debug"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "TSReader_DVBWorld___Win32_DSS_Debug"
# PROP Intermediate_Dir "TSReader_DVBWorld___Win32_DSS_Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /I "..\include" /I "C:\SDKs\ImgSource" /I "c:\sdks\cypress\inc" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /FD /GZ /c
# ADD CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /I "..\include" /I "C:\SDKs\ImgSource" /I "c:\sdks\cypress\inc" /D "DSS" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 ../TSReader_SourceHelper.lib CyAPI.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /debug /machine:I386 /out:"../Sources/TSReader_DVBWorld_S2101.dll" /pdbtype:sept /libpath:"c:\sdks\cypress\Lib\VC6_7"
# ADD LINK32 ../TSReader_SourceHelper.lib CyAPI.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /debug /machine:I386 /out:"../Sources/TSReader_DVBWorld_S2101_DSS.dll" /pdbtype:sept /libpath:"c:\sdks\cypress\Lib\VC6_7"

!ELSEIF  "$(CFG)" == "TSReader_DVBWorld - Win32 Nextorm Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "TSReader_DVBWorld___Win32_Nextorm_Release"
# PROP BASE Intermediate_Dir "TSReader_DVBWorld___Win32_Nextorm_Release"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "TSReader_DVBWorld___Win32_Nextorm_Release"
# PROP Intermediate_Dir "TSReader_DVBWorld___Win32_Nextorm_Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /I "..\include" /I "C:\SDKs\ImgSource" /I "c:\sdks\cypress\inc" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /MT /W3 /GX /O2 /I "..\include" /I "C:\SDKs\ImgSource" /I "c:\sdks\cypress\inc" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "NEXTORM" /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 ../TSReader_SourceHelper.lib CyAPI.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /machine:I386 /out:"../Sources/TSReader_DVBWorld_S2101.dll" /libpath:"c:\sdks\cypress\Lib\VC6_7"
# ADD LINK32 ../TSReader_SourceHelper.lib CyAPI.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /machine:I386 /out:"../Sources/TSReader_Nextorm_S2101.dll" /libpath:"c:\sdks\cypress\Lib\VC6_7"

!ELSEIF  "$(CFG)" == "TSReader_DVBWorld - Win32 Nextorm DSS Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "TSReader_DVBWorld___Win32_Nextorm_DSS_Release"
# PROP BASE Intermediate_Dir "TSReader_DVBWorld___Win32_Nextorm_DSS_Release"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "TSReader_DVBWorld___Win32_Nextorm_DSS_Release"
# PROP Intermediate_Dir "TSReader_DVBWorld___Win32_Nextorm_DSS_Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /I "..\include" /I "C:\SDKs\ImgSource" /I "c:\sdks\cypress\inc" /D "DSS" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /MT /W3 /GX /O2 /I "..\include" /I "C:\SDKs\ImgSource" /I "c:\sdks\cypress\inc" /D "DSS" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "NEXTORM" /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 ../TSReader_SourceHelper.lib CyAPI.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /machine:I386 /out:"../Sources/TSReader_DVBWorld_S2101_DSS.dll" /libpath:"c:\sdks\cypress\Lib\VC6_7"
# ADD LINK32 ../TSReader_SourceHelper.lib CyAPI.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /machine:I386 /out:"../Sources/TSReader_Nextorm_S2101_DSS.dll" /libpath:"c:\sdks\cypress\Lib\VC6_7"

!ELSEIF  "$(CFG)" == "TSReader_DVBWorld - Win32 Match Box Pro Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "TSReader_DVBWorld___Win32_Match_Box_Pro_Release"
# PROP BASE Intermediate_Dir "TSReader_DVBWorld___Win32_Match_Box_Pro_Release"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "TSReader_DVBWorld___Win32_Match_Box_Pro_Release"
# PROP Intermediate_Dir "TSReader_DVBWorld___Win32_Match_Box_Pro_Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /I "..\include" /I "C:\SDKs\ImgSource" /I "c:\sdks\cypress\inc" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /MT /W3 /GX /O2 /I "..\include" /I "C:\SDKs\ImgSource" /I "c:\sdks\cypress\inc" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "MATCHBOXPRO" /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 ../TSReader_SourceHelper.lib CyAPI.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /machine:I386 /out:"../Sources/TSReader_DVBWorld_S2101.dll" /libpath:"c:\sdks\cypress\Lib\VC6_7"
# ADD LINK32 ../TSReader_SourceHelper.lib CyAPI.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /machine:I386 /out:"../Sources/TSReader_MatchboxProS.dll" /libpath:"c:\sdks\cypress\Lib\VC6_7"

!ELSEIF  "$(CFG)" == "TSReader_DVBWorld - Win32 Match Box Pro Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "TSReader_DVBWorld___Win32_Match_Box_Pro_Debug"
# PROP BASE Intermediate_Dir "TSReader_DVBWorld___Win32_Match_Box_Pro_Debug"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "TSReader_DVBWorld___Win32_Match_Box_Pro_Debug"
# PROP Intermediate_Dir "TSReader_DVBWorld___Win32_Match_Box_Pro_Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /I "..\include" /I "C:\SDKs\ImgSource" /I "c:\sdks\cypress\inc" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /FD /GZ /c
# ADD CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /I "..\include" /I "C:\SDKs\ImgSource" /I "c:\sdks\cypress\inc" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "MATCHBOXPRO" /YX /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 ../TSReader_SourceHelper.lib CyAPI.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /debug /machine:I386 /out:"../Sources/TSReader_DVBWorld_S2101.dll" /pdbtype:sept /libpath:"c:\sdks\cypress\Lib\VC6_7"
# ADD LINK32 ../TSReader_SourceHelper.lib CyAPI.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /debug /machine:I386 /out:"../Sources/TSReader_MatchboxProS.dll" /pdbtype:sept /libpath:"c:\sdks\cypress\Lib\VC6_7"

!ENDIF 

# Begin Target

# Name "TSReader_DVBWorld - Win32 Release"
# Name "TSReader_DVBWorld - Win32 Debug"
# Name "TSReader_DVBWorld - Win32 DSS Release"
# Name "TSReader_DVBWorld - Win32 DSS Debug"
# Name "TSReader_DVBWorld - Win32 Nextorm Release"
# Name "TSReader_DVBWorld - Win32 Nextorm DSS Release"
# Name "TSReader_DVBWorld - Win32 Match Box Pro Release"
# Name "TSReader_DVBWorld - Win32 Match Box Pro Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\TSReader_DVBWorld.cpp
# End Source File
# Begin Source File

SOURCE=.\TSReader_DVBWorld.def
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\firmware.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\TSReader_DVBWorld.rc
# End Source File
# End Group
# End Target
# End Project
