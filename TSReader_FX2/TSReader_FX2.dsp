# Microsoft Developer Studio Project File - Name="TSReader_FX2" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=TSReader_FX2 - Win32 DVBTech Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "TSReader_FX2.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "TSReader_FX2.mak" CFG="TSReader_FX2 - Win32 DVBTech Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "TSReader_FX2 - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "TSReader_FX2 - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "TSReader_FX2 - Win32 SPI Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "TSReader_FX2 - Win32 8VSB Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "TSReader_FX2 - Win32 8VSB Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "TSReader_FX2 - Win32 SPI Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "TSReader_FX2 - Win32 QAM Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "TSReader_FX2 - Win32 DVBS Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "TSReader_FX2 - Win32 DSS Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "TSReader_FX2 - Win32 DVBS Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "TSReader_FX2 - Win32 DSS Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "TSReader_FX2 - Win32 QAM Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "TSReader_FX2 - Win32 CielPlus Sky Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "TSReader_FX2 - Win32 CielPlus Sky Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "TSReader_FX2 - Win32 CielPlus 5000 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "TSReader_FX2 - Win32 DVBTech 8PSK Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "TSReader_FX2 - Win32 Horizon TSRS1 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "TSReader_FX2 - Win32 DVBTech Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "TSReader_FX2 - Win32 Release"

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
# ADD CPP /nologo /MT /W3 /GX /O2 /I "..\include" /I "C:\SDKs\ImgSource" /I "c:\sdks\cypress\inc" /D "EIGHTPSK" /D "SATELLITE_SOURCE" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "EIGHTPSK" /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /machine:I386
# ADD LINK32 ../TSReader_SourceHelper.lib CyAPI.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /machine:I386 /out:"../Sources/TSReader_DTVWorks_8PSK.dll" /libpath:"c:\sdks\cypress\Lib\VC6_7"

!ELSEIF  "$(CFG)" == "TSReader_FX2 - Win32 Debug"

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
# ADD CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /I "..\include" /I "C:\SDKs\ImgSource" /I "c:\sdks\cypress\inc" /D "EIGHTPSK" /D "SATELLITE_SOURCE" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "EIGHTPSK" /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /debug /machine:I386 /pdbtype:sept
# ADD LINK32 ../TSReader_SourceHelper.lib CyAPI.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /debug /machine:I386 /out:"../Sources/TSReader_DTVWorks_8PSK.dll" /pdbtype:sept /libpath:"c:\sdks\cypress\Lib\VC6_7"

!ELSEIF  "$(CFG)" == "TSReader_FX2 - Win32 SPI Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "TSReader_FX2___Win32_SPI_Debug"
# PROP BASE Intermediate_Dir "TSReader_FX2___Win32_SPI_Debug"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "TSReader_FX2___Win32_SPI_Debug"
# PROP Intermediate_Dir "TSReader_FX2___Win32_SPI_Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /I "..\include" /I "C:\SDKs\ImgSource" /I "c:\sdks\cypress\inc" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /FD /GZ /c
# ADD CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /I "..\include" /I "C:\SDKs\ImgSource" /I "c:\sdks\cypress\inc" /D "SPI" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "SPI" /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 ../TSReader_SourceHelper.lib CyAPI.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /debug /machine:I386 /out:"../Sources/TSReader_FX2.dll" /pdbtype:sept /libpath:"c:\sdks\cypress\Lib\VC6_7"
# ADD LINK32 ../TSReader_SourceHelper.lib CyAPI.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /debug /machine:I386 /out:"../Sources/TSReader_DTVWorks_SPI.dll" /pdbtype:sept /libpath:"c:\sdks\cypress\Lib\VC6_7"

!ELSEIF  "$(CFG)" == "TSReader_FX2 - Win32 8VSB Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "TSReader_FX2___Win32_8VSB_Debug"
# PROP BASE Intermediate_Dir "TSReader_FX2___Win32_8VSB_Debug"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "TSReader_FX2___Win32_8VSB_Debug"
# PROP Intermediate_Dir "TSReader_FX2___Win32_8VSB_Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /I "..\include" /I "C:\SDKs\ImgSource" /I "c:\sdks\cypress\inc" /D "EIGHTPSK" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /FD /GZ /c
# ADD CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /I "..\include" /I "C:\SDKs\ImgSource" /I "c:\sdks\cypress\inc" /D "EIGHTVSB" /D "ALPSTUNER" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /FR /YX /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 ../TSReader_SourceHelper.lib CyAPI.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /debug /machine:I386 /out:"../Sources/TSReader_SkySeeker_8PSK.dll" /pdbtype:sept /libpath:"c:\sdks\cypress\Lib\VC6_7"
# ADD LINK32 ../TSReader_SourceHelper.lib CyAPI.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /debug /machine:I386 /out:"../Sources/TSReader_DTVWorks_8VSB.dll" /pdbtype:sept /libpath:"c:\sdks\cypress\Lib\VC6_7"

!ELSEIF  "$(CFG)" == "TSReader_FX2 - Win32 8VSB Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "TSReader_FX2___Win32_8VSB_Release"
# PROP BASE Intermediate_Dir "TSReader_FX2___Win32_8VSB_Release"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "TSReader_FX2___Win32_8VSB_Release"
# PROP Intermediate_Dir "TSReader_FX2___Win32_8VSB_Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /I "..\include" /I "C:\SDKs\ImgSource" /I "c:\sdks\cypress\inc" /D "EIGHTPSK" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /MT /W3 /GX /O2 /I "..\include" /I "C:\SDKs\ImgSource" /I "c:\sdks\cypress\inc" /D "EIGHTVSB" /D "ALPSTUNER" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 ../TSReader_SourceHelper.lib CyAPI.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /machine:I386 /out:"../Sources/TSReader_SkySeeker_8PSK.dll" /libpath:"c:\sdks\cypress\Lib\VC6_7"
# ADD LINK32 ../TSReader_SourceHelper.lib CyAPI.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /machine:I386 /out:"../Sources/TSReader_DTVWorks_8VSB.dll" /libpath:"c:\sdks\cypress\Lib\VC6_7"

!ELSEIF  "$(CFG)" == "TSReader_FX2 - Win32 SPI Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "TSReader_FX2___Win32_SPI_Release"
# PROP BASE Intermediate_Dir "TSReader_FX2___Win32_SPI_Release"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "TSReader_FX2___Win32_SPI_Release"
# PROP Intermediate_Dir "TSReader_FX2___Win32_SPI_Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /I "..\include" /I "C:\SDKs\ImgSource" /I "c:\sdks\cypress\inc" /D "EIGHTPSK" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /MT /W3 /GX /O2 /I "..\include" /I "C:\SDKs\ImgSource" /I "c:\sdks\cypress\inc" /D "SPI" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "SPI" /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 ../TSReader_SourceHelper.lib CyAPI.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /machine:I386 /out:"../Sources/TSReader_SkySeeker_8PSK.dll" /libpath:"c:\sdks\cypress\Lib\VC6_7"
# ADD LINK32 ../TSReader_SourceHelper.lib CyAPI.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /machine:I386 /out:"../Sources/TSReader_DTVWorks_SPI.dll" /libpath:"c:\sdks\cypress\Lib\VC6_7"

!ELSEIF  "$(CFG)" == "TSReader_FX2 - Win32 QAM Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "TSReader_FX2___Win32_QAM_Debug"
# PROP BASE Intermediate_Dir "TSReader_FX2___Win32_QAM_Debug"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "TSReader_FX2___Win32_QAM_Debug"
# PROP Intermediate_Dir "TSReader_FX2___Win32_QAM_Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /I "..\include" /I "C:\SDKs\ImgSource" /I "c:\sdks\cypress\inc" /D "EIGHTVSB" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /FR /YX /FD /GZ /c
# ADD CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /I "..\include" /I "C:\SDKs\ImgSource" /I "c:\sdks\cypress\inc" /D "QAM" /D "ALPSTUNER" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /FR /YX /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 ../TSReader_SourceHelper.lib CyAPI.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /debug /machine:I386 /out:"../Sources/TSReader_SkySeeker_8VSB.dll" /pdbtype:sept /libpath:"c:\sdks\cypress\Lib\VC6_7"
# ADD LINK32 ../TSReader_SourceHelper.lib CyAPI.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /debug /machine:I386 /out:"../Sources/TSReader_DTVWorks_QAM.dll" /pdbtype:sept /libpath:"c:\sdks\cypress\Lib\VC6_7"

!ELSEIF  "$(CFG)" == "TSReader_FX2 - Win32 DVBS Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "TSReader_FX2___Win32_DVBS_Debug"
# PROP BASE Intermediate_Dir "TSReader_FX2___Win32_DVBS_Debug"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "TSReader_FX2___Win32_DVBS_Debug"
# PROP Intermediate_Dir "TSReader_FX2___Win32_DVBS_Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /I "..\include" /I "C:\SDKs\ImgSource" /I "c:\sdks\cypress\inc" /D "EIGHTPSK" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /FD /GZ /c
# ADD CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /I "..\include" /I "C:\SDKs\ImgSource" /I "c:\sdks\cypress\inc" /D "DVBS" /D "SATELLITE_SOURCE" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 ../TSReader_SourceHelper.lib CyAPI.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /debug /machine:I386 /out:"../Sources/TSReader_SkySeeker_8PSK.dll" /pdbtype:sept /libpath:"c:\sdks\cypress\Lib\VC6_7"
# ADD LINK32 ../TSReader_SourceHelper.lib CyAPI.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /debug /machine:I386 /out:"../Sources/TSReader_DTVWorks_DVBS.dll" /pdbtype:sept /libpath:"c:\sdks\cypress\Lib\VC6_7"

!ELSEIF  "$(CFG)" == "TSReader_FX2 - Win32 DSS Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "TSReader_FX2___Win32_DSS_Debug0"
# PROP BASE Intermediate_Dir "TSReader_FX2___Win32_DSS_Debug0"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "TSReader_FX2___Win32_DSS_Debug0"
# PROP Intermediate_Dir "TSReader_FX2___Win32_DSS_Debug0"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /I "..\include" /I "C:\SDKs\ImgSource" /I "c:\sdks\cypress\inc" /D "DVBS" /D "SATELLITE_SOURCE" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /FD /GZ /c
# ADD CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /I "..\include" /I "C:\SDKs\ImgSource" /I "c:\sdks\cypress\inc" /D "DSS" /D "DVBS" /D "SATELLITE_SOURCE" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 ../TSReader_SourceHelper.lib CyAPI.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /debug /machine:I386 /out:"../Sources/TSReader_SkySeeker_DVBS.dll" /pdbtype:sept /libpath:"c:\sdks\cypress\Lib\VC6_7"
# ADD LINK32 ../TSReader_SourceHelper.lib CyAPI.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /debug /machine:I386 /out:"../Sources/TSReader_DTVWorks_DSS.dll" /pdbtype:sept /libpath:"c:\sdks\cypress\Lib\VC6_7"

!ELSEIF  "$(CFG)" == "TSReader_FX2 - Win32 DVBS Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "TSReader_FX2___Win32_DVBS_Release0"
# PROP BASE Intermediate_Dir "TSReader_FX2___Win32_DVBS_Release0"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "TSReader_FX2___Win32_DVBS_Release0"
# PROP Intermediate_Dir "TSReader_FX2___Win32_DVBS_Release0"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /I "..\include" /I "C:\SDKs\ImgSource" /I "c:\sdks\cypress\inc" /D "EIGHTPSK" /D "SATELLITE_SOURCE" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /MT /W3 /GX /O2 /I "..\include" /I "C:\SDKs\ImgSource" /I "c:\sdks\cypress\inc" /D "DVBS" /D "SATELLITE_SOURCE" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 ../TSReader_SourceHelper.lib CyAPI.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /machine:I386 /out:"../Sources/TSReader_SkySeeker_8PSK.dll" /libpath:"c:\sdks\cypress\Lib\VC6_7"
# ADD LINK32 ../TSReader_SourceHelper.lib CyAPI.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /machine:I386 /out:"../Sources/TSReader_DTVWorks_DVBS.dll" /libpath:"c:\sdks\cypress\Lib\VC6_7"

!ELSEIF  "$(CFG)" == "TSReader_FX2 - Win32 DSS Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "TSReader_FX2___Win32_DSS_Release0"
# PROP BASE Intermediate_Dir "TSReader_FX2___Win32_DSS_Release0"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "TSReader_FX2___Win32_DSS_Release0"
# PROP Intermediate_Dir "TSReader_FX2___Win32_DSS_Release0"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /I "..\include" /I "C:\SDKs\ImgSource" /I "c:\sdks\cypress\inc" /D "EIGHTPSK" /D "SATELLITE_SOURCE" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /MT /W3 /GX /O2 /I "..\include" /I "C:\SDKs\ImgSource" /I "c:\sdks\cypress\inc" /D "DSS" /D "DVBS" /D "SATELLITE_SOURCE" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 ../TSReader_SourceHelper.lib CyAPI.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /machine:I386 /out:"../Sources/TSReader_SkySeeker_8PSK.dll" /libpath:"c:\sdks\cypress\Lib\VC6_7"
# ADD LINK32 ../TSReader_SourceHelper.lib CyAPI.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /machine:I386 /out:"../Sources/TSReader_DTVWorks_DSS.dll" /libpath:"c:\sdks\cypress\Lib\VC6_7"

!ELSEIF  "$(CFG)" == "TSReader_FX2 - Win32 QAM Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "TSReader_FX2___Win32_QAM_Release0"
# PROP BASE Intermediate_Dir "TSReader_FX2___Win32_QAM_Release0"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "TSReader_FX2___Win32_QAM_Release0"
# PROP Intermediate_Dir "TSReader_FX2___Win32_QAM_Release0"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /I "..\include" /I "C:\SDKs\ImgSource" /I "c:\sdks\cypress\inc" /D "EIGHTPSK" /D "SATELLITE_SOURCE" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /MT /W3 /GX /O2 /I "..\include" /I "C:\SDKs\ImgSource" /I "c:\sdks\cypress\inc" /D "QAM" /D "ALPSTUNER" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 ../TSReader_SourceHelper.lib CyAPI.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /machine:I386 /out:"../Sources/TSReader_SkySeeker_8PSK.dll" /libpath:"c:\sdks\cypress\Lib\VC6_7"
# ADD LINK32 ../TSReader_SourceHelper.lib CyAPI.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /machine:I386 /out:"../Sources/TSReader_DTVWorks_QAM.dll" /libpath:"c:\sdks\cypress\Lib\VC6_7"

!ELSEIF  "$(CFG)" == "TSReader_FX2 - Win32 CielPlus Sky Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "TSReader_FX2___Win32_CielPlus_Sky_Debug"
# PROP BASE Intermediate_Dir "TSReader_FX2___Win32_CielPlus_Sky_Debug"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "TSReader_FX2___Win32_CielPlus_Sky_Debug"
# PROP Intermediate_Dir "TSReader_FX2___Win32_CielPlus_Sky_Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /I "..\include" /I "C:\SDKs\ImgSource" /I "c:\sdks\cypress\inc" /D "SPI" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /FD /GZ /c
# ADD CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /I "..\include" /I "C:\SDKs\ImgSource" /I "c:\sdks\cypress\inc" /D "CIELPLUS_SKY" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "SPI" /d "_DEBUG"
# ADD RSC /l 0x409 /d "CIELPLUS_SKY" /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 ../TSReader_SourceHelper.lib CyAPI.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /debug /machine:I386 /out:"../Sources/TSReader_DTVWorks_SPI.dll" /pdbtype:sept /libpath:"c:\sdks\cypress\Lib\VC6_7"
# ADD LINK32 ../TSReader_SourceHelper.lib CyAPI.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /debug /machine:I386 /out:"../Sources/TSReader_CielPlus_Sky.dll" /pdbtype:sept /libpath:"c:\sdks\cypress\Lib\VC6_7"

!ELSEIF  "$(CFG)" == "TSReader_FX2 - Win32 CielPlus Sky Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "TSReader_FX2___Win32_CielPlus_Sky_Release"
# PROP BASE Intermediate_Dir "TSReader_FX2___Win32_CielPlus_Sky_Release"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "TSReader_FX2___Win32_CielPlus_Sky_Release"
# PROP Intermediate_Dir "TSReader_FX2___Win32_CielPlus_Sky_Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /I "..\include" /I "C:\SDKs\ImgSource" /I "c:\sdks\cypress\inc" /D "SPI" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /MT /W3 /GX /O2 /I "..\include" /I "C:\SDKs\ImgSource" /I "c:\sdks\cypress\inc" /D "CIELPLUS_SKY" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "SPI" /d "NDEBUG"
# ADD RSC /l 0x409 /d "CIELPLUS_SKY" /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 ../TSReader_SourceHelper.lib CyAPI.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /machine:I386 /out:"../Sources/TSReader_DTVWorks_SPI.dll" /libpath:"c:\sdks\cypress\Lib\VC6_7"
# ADD LINK32 ../TSReader_SourceHelper.lib CyAPI.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /machine:I386 /out:"../Sources/TSReader_CielPlus_Sky.dll" /libpath:"c:\sdks\cypress\Lib\VC6_7"

!ELSEIF  "$(CFG)" == "TSReader_FX2 - Win32 CielPlus 5000 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "TSReader_FX2___Win32_CielPlus_5000_Debug"
# PROP BASE Intermediate_Dir "TSReader_FX2___Win32_CielPlus_5000_Debug"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "TSReader_FX2___Win32_CielPlus_5000_Debug"
# PROP Intermediate_Dir "TSReader_FX2___Win32_CielPlus_5000_Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /I "..\include" /I "C:\SDKs\ImgSource" /I "c:\sdks\cypress\inc" /D "CIELPLUS_SKY" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /FD /GZ /c
# ADD CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /I "..\include" /I "C:\SDKs\ImgSource" /I "c:\sdks\cypress\inc" /D "CIELPLUS_5000" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "CIELPLUS" /d "_DEBUG"
# ADD RSC /l 0x409 /d "CIELPLUS_5000" /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 ../TSReader_SourceHelper.lib CyAPI.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /debug /machine:I386 /out:"../Sources/TSReader_CielPlus_Sky.dll" /pdbtype:sept /libpath:"c:\sdks\cypress\Lib\VC6_7"
# ADD LINK32 ../TSReader_SourceHelper.lib CyAPI.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /debug /machine:I386 /out:"../Sources/TSReader_CielPlus_Dish.dll" /pdbtype:sept /libpath:"c:\sdks\cypress\Lib\VC6_7"

!ELSEIF  "$(CFG)" == "TSReader_FX2 - Win32 DVBTech 8PSK Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "TSReader_FX2___Win32_DVBTech_8PSK_Release"
# PROP BASE Intermediate_Dir "TSReader_FX2___Win32_DVBTech_8PSK_Release"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "TSReader_FX2___Win32_DVBTech_8PSK_Release"
# PROP Intermediate_Dir "TSReader_FX2___Win32_DVBTech_8PSK_Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /I "..\include" /I "C:\SDKs\ImgSource" /I "c:\sdks\cypress\inc" /D "EIGHTPSK" /D "SATELLITE_SOURCE" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /MT /W3 /GX /O2 /I "..\include" /I "C:\SDKs\ImgSource" /I "c:\sdks\cypress\inc" /D "DVBTECH" /D "EIGHTPSK" /D "SATELLITE_SOURCE" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "EIGHTPSK" /d "NDEBUG"
# ADD RSC /l 0x409 /d "DVBTECH" /d "EIGHTPSK" /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 ../TSReader_SourceHelper.lib CyAPI.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /machine:I386 /out:"../Sources/TSReader_DTVWorks_8PSK.dll" /libpath:"c:\sdks\cypress\Lib\VC6_7"
# ADD LINK32 ../TSReader_SourceHelper.lib CyAPI.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /machine:I386 /out:"../Sources/TSReader_DVBTech_8PSK.dll" /libpath:"c:\sdks\cypress\Lib\VC6_7"

!ELSEIF  "$(CFG)" == "TSReader_FX2 - Win32 Horizon TSRS1 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "TSReader_FX2___Win32_Horizon_TSRS1_Release"
# PROP BASE Intermediate_Dir "TSReader_FX2___Win32_Horizon_TSRS1_Release"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "TSReader_FX2___Win32_Horizon_TSRS1_Release"
# PROP Intermediate_Dir "TSReader_FX2___Win32_Horizon_TSRS1_Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /I "..\include" /I "C:\SDKs\ImgSource" /I "c:\sdks\cypress\inc" /D "SPI" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /MT /W3 /GX /O2 /I "..\include" /I "C:\SDKs\ImgSource" /I "c:\sdks\cypress\inc" /D "SPI" /D "HORIZON" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "SPI" /d "NDEBUG"
# ADD RSC /l 0x409 /d "SPI" /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 ../TSReader_SourceHelper.lib CyAPI.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /machine:I386 /out:"../Sources/TSReader_DTVWorks_SPI.dll" /libpath:"c:\sdks\cypress\Lib\VC6_7"
# ADD LINK32 ../TSReader_SourceHelper.lib CyAPI.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /machine:I386 /out:"../Sources/TSReader_Horizon_TSR-S1.dll" /libpath:"c:\sdks\cypress\Lib\VC6_7"

!ELSEIF  "$(CFG)" == "TSReader_FX2 - Win32 DVBTech Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "TSReader_FX2___Win32_DVBTech_Debug"
# PROP BASE Intermediate_Dir "TSReader_FX2___Win32_DVBTech_Debug"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "TSReader_FX2___Win32_DVBTech_Debug"
# PROP Intermediate_Dir "TSReader_FX2___Win32_DVBTech_Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /I "..\include" /I "C:\SDKs\ImgSource" /I "c:\sdks\cypress\inc" /D "EIGHTPSK" /D "SATELLITE_SOURCE" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /FD /GZ /c
# ADD CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /I "..\include" /I "C:\SDKs\ImgSource" /I "c:\sdks\cypress\inc" /D "DVBTECH" /D "EIGHTPSK" /D "SATELLITE_SOURCE" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "EIGHTPSK" /d "_DEBUG"
# ADD RSC /l 0x409 /d "EIGHTPSK" /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 ../TSReader_SourceHelper.lib CyAPI.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /debug /machine:I386 /out:"../Sources/TSReader_DTVWorks_8PSK.dll" /pdbtype:sept /libpath:"c:\sdks\cypress\Lib\VC6_7"
# ADD LINK32 ../TSReader_SourceHelper.lib CyAPI.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /debug /machine:I386 /out:"../Sources/TSReader_DVBTech_8PSK.dll" /pdbtype:sept /libpath:"c:\sdks\cypress\Lib\VC6_7"

!ENDIF 

# Begin Target

# Name "TSReader_FX2 - Win32 Release"
# Name "TSReader_FX2 - Win32 Debug"
# Name "TSReader_FX2 - Win32 SPI Debug"
# Name "TSReader_FX2 - Win32 8VSB Debug"
# Name "TSReader_FX2 - Win32 8VSB Release"
# Name "TSReader_FX2 - Win32 SPI Release"
# Name "TSReader_FX2 - Win32 QAM Debug"
# Name "TSReader_FX2 - Win32 DVBS Debug"
# Name "TSReader_FX2 - Win32 DSS Debug"
# Name "TSReader_FX2 - Win32 DVBS Release"
# Name "TSReader_FX2 - Win32 DSS Release"
# Name "TSReader_FX2 - Win32 QAM Release"
# Name "TSReader_FX2 - Win32 CielPlus Sky Debug"
# Name "TSReader_FX2 - Win32 CielPlus Sky Release"
# Name "TSReader_FX2 - Win32 CielPlus 5000 Debug"
# Name "TSReader_FX2 - Win32 DVBTech 8PSK Release"
# Name "TSReader_FX2 - Win32 Horizon TSRS1 Release"
# Name "TSReader_FX2 - Win32 DVBTech Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\drv2user.cpp
# End Source File
# Begin Source File

SOURCE=.\drv2user.h
# End Source File
# Begin Source File

SOURCE=.\TSReader_FX2.cpp
# End Source File
# Begin Source File

SOURCE=.\TSReader_FX2.def
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\hardware.h
# End Source File
# Begin Source File

SOURCE=.\resource.h
# End Source File
# Begin Source File

SOURCE=.\skyir.h
# End Source File
# Begin Source File

SOURCE=.\USBIoctlStructs.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\TSReader_FX2.rc
# End Source File
# End Group
# Begin Group "8VSB Driver"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\8VSBDriver\drv2004.cpp
# End Source File
# Begin Source File

SOURCE=.\8VSBDriver\drv2004.h
# End Source File
# Begin Source File

SOURCE=.\8VSBDriver\drv200x.cpp
# End Source File
# Begin Source File

SOURCE=.\8VSBDriver\drv2cntx.cpp
# End Source File
# Begin Source File

SOURCE=.\8VSBDriver\drv2cntx.h
# End Source File
# Begin Source File

SOURCE=.\8VSBDriver\drv2crc.cpp
# End Source File
# Begin Source File

SOURCE=.\8VSBDriver\drv2crc.h
# End Source File
# Begin Source File

SOURCE=.\8VSBDriver\drv2load.cpp
# End Source File
# Begin Source File

SOURCE=.\8VSBDriver\drv2load.h
# End Source File
# Begin Source File

SOURCE=.\8VSBDriver\drv2reg.cpp
# End Source File
# Begin Source File

SOURCE=.\8VSBDriver\drv2reg.h
# End Source File
# Begin Source File

SOURCE=.\8VSBDriver\drvNxtenna.cpp
# End Source File
# Begin Source File

SOURCE=.\8VSBDriver\drvNxtenna.h
# End Source File
# Begin Source File

SOURCE=.\8VSBDriver\Nxt2004registers.h
# End Source File
# Begin Source File

SOURCE=.\8VSBDriver\NxtCommon.h
# End Source File
# End Group
# End Target
# End Project
