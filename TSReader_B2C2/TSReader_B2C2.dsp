# Microsoft Developer Studio Project File - Name="TSReader_B2C2" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=TSReader_B2C2 - Win32 QAM Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "TSReader_B2C2.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "TSReader_B2C2.mak" CFG="TSReader_B2C2 - Win32 QAM Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "TSReader_B2C2 - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "TSReader_B2C2 - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "TSReader_B2C2 - Win32 DVBS Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "TSReader_B2C2 - Win32 DVBS Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "TSReader_B2C2 - Win32 DVBT Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "TSReader_B2C2 - Win32 DVBC Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "TSReader_B2C2 - Win32 DVBT Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "TSReader_B2C2 - Win32 DVBC Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "TSReader_B2C2 - Win32 QAM Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "TSReader_B2C2 - Win32 QAM Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "TSReader_B2C2 - Win32 Release"

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
# ADD CPP /nologo /MT /W3 /GX /O2 /I "..\include" /I "C:\SDKs\ImgSource" /I "C:\SDKs\DXSDK\include" /I "../BaseClasses" /I "C:\SDKs\SS2\Include" /D "ATSC" /D "INC_OLE2" /D "STRICT" /D _WIN32_WINNT=0x0400 /D "WIN32" /D "_WIN32" /D "_MT" /D "_DLL" /D _X86_=1 /D WINVER=0x0400 /D "NDEBUG" /D "_NDEBUG" /D try=__try /D except=__except /D leave=__leave /D finally=__finally /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /machine:I386
# ADD LINK32 ../TSReader_SourceHelper.lib Version.lib strmbase.lib msvcrt.lib ole32.lib oleaut32.lib kernel32.lib user32.lib gdi32.lib /nologo /subsystem:windows /dll /machine:I386 /nodefaultlib /out:"../Sources/TSReader_B2C2a.dll"

!ELSEIF  "$(CFG)" == "TSReader_B2C2 - Win32 Debug"

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
# ADD CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /I "..\include" /I "C:\SDKs\ImgSource" /I "C:\SDKs\DXSDK\include" /I "../BaseClasses" /I "C:\SDKs\SS2\Include" /D "ATSC" /D "INC_OLE2" /D "STRICT" /D _WIN32_WINNT=0x0400 /D "WIN32" /D "_WIN32" /D "_MT" /D "_DLL" /D _X86_=1 /D WINVER=0x0400 /D DBG=1 /D "DEBUG" /D "_DEBUG" /D try=__try /D except=__except /D leave=__leave /D finally=__finally /FR /YX /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /debug /machine:I386 /pdbtype:sept
# ADD LINK32 ../TSReader_SourceHelper.lib Version.lib strmbasd.lib msvcrtd.lib ole32.lib oleaut32.lib kernel32.lib user32.lib gdi32.lib /nologo /subsystem:windows /dll /debug /machine:I386 /nodefaultlib /out:"../Sources/TSReader_B2C2a.dll" /pdbtype:sept /libpath:"..\BaseClasses\debug"

!ELSEIF  "$(CFG)" == "TSReader_B2C2 - Win32 DVBS Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "TSReader_B2C2___Win32_DVBS_Debug"
# PROP BASE Intermediate_Dir "TSReader_B2C2___Win32_DVBS_Debug"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "TSReader_B2C2___Win32_DVBS_Debug"
# PROP Intermediate_Dir "TSReader_B2C2___Win32_DVBS_Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /I "..\include" /I "C:\SDKs\ImgSource" /I "C:\SDKs\DXSDK\include" /I "../BaseClasses" /I "C:\SDKs\SS2\Include" /D "INC_OLE2" /D "STRICT" /D _WIN32_WINNT=0x0400 /D "WIN32" /D "_WIN32" /D "_MT" /D "_DLL" /D _X86_=1 /D WINVER=0x0400 /D DBG=1 /D "DEBUG" /D "_DEBUG" /D try=__try /D except=__except /D leave=__leave /D finally=__finally /YX /FD /GZ /c
# ADD CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /I "..\include" /I "C:\SDKs\ImgSource" /I "C:\SDKs\DXSDK\include" /I "../BaseClasses" /I "C:\SDKs\SS2\Include" /D "DVBS" /D "INC_OLE2" /D "STRICT" /D _WIN32_WINNT=0x0400 /D "WIN32" /D "_WIN32" /D "_MT" /D "_DLL" /D _X86_=1 /D WINVER=0x0400 /D DBG=1 /D "DEBUG" /D "_DEBUG" /D try=__try /D except=__except /D leave=__leave /D finally=__finally /YX /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 ../TSReader_SourceHelper.lib strmbasd.lib /nologo /subsystem:windows /dll /debug /machine:I386 /out:"../Sources/TSReader_B2C2a.dll" /pdbtype:sept /libpath:"..\BaseClasses\debug"
# ADD LINK32 ../TSReader_SourceHelper.lib Version.lib strmbasd.lib msvcrtd.lib ole32.lib oleaut32.lib kernel32.lib user32.lib gdi32.lib /nologo /subsystem:windows /dll /debug /machine:I386 /nodefaultlib /out:"../Sources/TSReader_B2C2s.dll" /pdbtype:sept /libpath:"..\BaseClasses\debug"

!ELSEIF  "$(CFG)" == "TSReader_B2C2 - Win32 DVBS Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "TSReader_B2C2___Win32_DVBS_Release"
# PROP BASE Intermediate_Dir "TSReader_B2C2___Win32_DVBS_Release"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "TSReader_B2C2___Win32_DVBS_Release"
# PROP Intermediate_Dir "TSReader_B2C2___Win32_DVBS_Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /I "..\include" /I "C:\SDKs\ImgSource" /I "C:\SDKs\DXSDK\include" /I "../BaseClasses" /I "C:\SDKs\SS2\Include" /D "ATSC" /D "INC_OLE2" /D "STRICT" /D _WIN32_WINNT=0x0400 /D "WIN32" /D "_WIN32" /D "_MT" /D "_DLL" /D _X86_=1 /D WINVER=0x0400 /D "NDEBUG" /D "_NDEBUG" /D try=__try /D except=__except /D leave=__leave /D finally=__finally /YX /FD /c
# ADD CPP /nologo /MT /W3 /GX /O2 /I "..\include" /I "C:\SDKs\ImgSource" /I "C:\SDKs\DXSDK\include" /I "../BaseClasses" /I "C:\SDKs\SS2\Include" /D "DVBS" /D "INC_OLE2" /D "STRICT" /D _WIN32_WINNT=0x0400 /D "WIN32" /D "_WIN32" /D "_MT" /D "_DLL" /D _X86_=1 /D WINVER=0x0400 /D "NDEBUG" /D "_NDEBUG" /D try=__try /D except=__except /D leave=__leave /D finally=__finally /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 ../TSReader_SourceHelper.lib strmbase.lib msvcrt.lib ole32.lib oleaut32.lib kernel32.lib user32.lib gdi32.lib /nologo /subsystem:windows /dll /machine:I386 /nodefaultlib /out:"../Sources/TSReader_B2C2a.dll"
# ADD LINK32 ../TSReader_SourceHelper.lib Version.lib strmbase.lib msvcrt.lib ole32.lib oleaut32.lib kernel32.lib user32.lib gdi32.lib /nologo /subsystem:windows /dll /machine:I386 /nodefaultlib /out:"../Sources/TSReader_B2C2s.dll"

!ELSEIF  "$(CFG)" == "TSReader_B2C2 - Win32 DVBT Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "TSReader_B2C2___Win32_DVBT_Debug"
# PROP BASE Intermediate_Dir "TSReader_B2C2___Win32_DVBT_Debug"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "TSReader_B2C2___Win32_DVBT_Debug"
# PROP Intermediate_Dir "TSReader_B2C2___Win32_DVBT_Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /I "..\include" /I "C:\SDKs\ImgSource" /I "C:\SDKs\DXSDK\include" /I "../BaseClasses" /I "C:\SDKs\SS2\Include" /D "DVBS" /D "INC_OLE2" /D "STRICT" /D _WIN32_WINNT=0x0400 /D "WIN32" /D "_WIN32" /D "_MT" /D "_DLL" /D _X86_=1 /D WINVER=0x0400 /D DBG=1 /D "DEBUG" /D "_DEBUG" /D try=__try /D except=__except /D leave=__leave /D finally=__finally /YX /FD /GZ /c
# ADD CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /I "..\include" /I "C:\SDKs\ImgSource" /I "C:\SDKs\DXSDK\include" /I "../BaseClasses" /I "C:\SDKs\SS2\Include" /D "DVBT" /D "INC_OLE2" /D "STRICT" /D _WIN32_WINNT=0x0400 /D "WIN32" /D "_WIN32" /D "_MT" /D "_DLL" /D _X86_=1 /D WINVER=0x0400 /D DBG=1 /D "DEBUG" /D "_DEBUG" /D try=__try /D except=__except /D leave=__leave /D finally=__finally /YX /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 ../TSReader_SourceHelper.lib strmbasd.lib msvcrtd.lib ole32.lib oleaut32.lib kernel32.lib user32.lib gdi32.lib /nologo /subsystem:windows /dll /debug /machine:I386 /nodefaultlib /out:"../Sources/TSReader_B2C2s.dll" /pdbtype:sept /libpath:"..\BaseClasses\debug"
# ADD LINK32 ../TSReader_SourceHelper.lib Version.lib strmbasd.lib msvcrtd.lib ole32.lib oleaut32.lib kernel32.lib user32.lib gdi32.lib /nologo /subsystem:windows /dll /debug /machine:I386 /nodefaultlib /out:"../Sources/TSReader_B2C2t.dll" /pdbtype:sept /libpath:"..\BaseClasses\debug"

!ELSEIF  "$(CFG)" == "TSReader_B2C2 - Win32 DVBC Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "TSReader_B2C2___Win32_DVBC_Debug"
# PROP BASE Intermediate_Dir "TSReader_B2C2___Win32_DVBC_Debug"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "TSReader_B2C2___Win32_DVBC_Debug"
# PROP Intermediate_Dir "TSReader_B2C2___Win32_DVBC_Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /I "..\include" /I "C:\SDKs\ImgSource" /I "C:\SDKs\DXSDK\include" /I "../BaseClasses" /I "C:\SDKs\SS2\Include" /D "DVBS" /D "INC_OLE2" /D "STRICT" /D _WIN32_WINNT=0x0400 /D "WIN32" /D "_WIN32" /D "_MT" /D "_DLL" /D _X86_=1 /D WINVER=0x0400 /D DBG=1 /D "DEBUG" /D "_DEBUG" /D try=__try /D except=__except /D leave=__leave /D finally=__finally /YX /FD /GZ /c
# ADD CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /I "..\include" /I "C:\SDKs\ImgSource" /I "C:\SDKs\DXSDK\include" /I "../BaseClasses" /I "C:\SDKs\SS2\Include" /D "DVBC" /D "INC_OLE2" /D "STRICT" /D _WIN32_WINNT=0x0400 /D "WIN32" /D "_WIN32" /D "_MT" /D "_DLL" /D _X86_=1 /D WINVER=0x0400 /D DBG=1 /D "DEBUG" /D "_DEBUG" /D try=__try /D except=__except /D leave=__leave /D finally=__finally /YX /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 ../TSReader_SourceHelper.lib strmbasd.lib msvcrtd.lib ole32.lib oleaut32.lib kernel32.lib user32.lib gdi32.lib /nologo /subsystem:windows /dll /debug /machine:I386 /nodefaultlib /out:"../Sources/TSReader_B2C2s.dll" /pdbtype:sept /libpath:"..\BaseClasses\debug"
# ADD LINK32 ../TSReader_SourceHelper.lib Version.lib strmbasd.lib msvcrtd.lib ole32.lib oleaut32.lib kernel32.lib user32.lib gdi32.lib /nologo /subsystem:windows /dll /debug /machine:I386 /nodefaultlib /out:"../Sources/TSReader_B2C2c.dll" /pdbtype:sept /libpath:"..\BaseClasses\debug"

!ELSEIF  "$(CFG)" == "TSReader_B2C2 - Win32 DVBT Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "TSReader_B2C2___Win32_DVBT_Release"
# PROP BASE Intermediate_Dir "TSReader_B2C2___Win32_DVBT_Release"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "TSReader_B2C2___Win32_DVBT_Release"
# PROP Intermediate_Dir "TSReader_B2C2___Win32_DVBT_Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /I "..\include" /I "C:\SDKs\ImgSource" /I "C:\SDKs\DXSDK\include" /I "../BaseClasses" /I "C:\SDKs\SS2\Include" /D "DVBS" /D "INC_OLE2" /D "STRICT" /D _WIN32_WINNT=0x0400 /D "WIN32" /D "_WIN32" /D "_MT" /D "_DLL" /D _X86_=1 /D WINVER=0x0400 /D "NDEBUG" /D "_NDEBUG" /D try=__try /D except=__except /D leave=__leave /D finally=__finally /YX /FD /c
# ADD CPP /nologo /MT /W3 /GX /O2 /I "..\include" /I "C:\SDKs\ImgSource" /I "C:\SDKs\DXSDK\include" /I "../BaseClasses" /I "C:\SDKs\SS2\Include" /D "DVBT" /D "INC_OLE2" /D "STRICT" /D _WIN32_WINNT=0x0400 /D "WIN32" /D "_WIN32" /D "_MT" /D "_DLL" /D _X86_=1 /D WINVER=0x0400 /D "NDEBUG" /D "_NDEBUG" /D try=__try /D except=__except /D leave=__leave /D finally=__finally /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 ../TSReader_SourceHelper.lib strmbase.lib msvcrt.lib ole32.lib oleaut32.lib kernel32.lib user32.lib gdi32.lib /nologo /subsystem:windows /dll /machine:I386 /nodefaultlib /out:"../Sources/TSReader_B2C2s.dll"
# ADD LINK32 ../TSReader_SourceHelper.lib Version.lib strmbase.lib msvcrt.lib ole32.lib oleaut32.lib kernel32.lib user32.lib gdi32.lib /nologo /subsystem:windows /dll /machine:I386 /nodefaultlib /out:"../Sources/TSReader_B2C2t.dll"

!ELSEIF  "$(CFG)" == "TSReader_B2C2 - Win32 DVBC Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "TSReader_B2C2___Win32_DVBC_Release"
# PROP BASE Intermediate_Dir "TSReader_B2C2___Win32_DVBC_Release"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "TSReader_B2C2___Win32_DVBC_Release"
# PROP Intermediate_Dir "TSReader_B2C2___Win32_DVBC_Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /I "..\include" /I "C:\SDKs\ImgSource" /I "C:\SDKs\DXSDK\include" /I "../BaseClasses" /I "C:\SDKs\SS2\Include" /D "DVBS" /D "INC_OLE2" /D "STRICT" /D _WIN32_WINNT=0x0400 /D "WIN32" /D "_WIN32" /D "_MT" /D "_DLL" /D _X86_=1 /D WINVER=0x0400 /D "NDEBUG" /D "_NDEBUG" /D try=__try /D except=__except /D leave=__leave /D finally=__finally /YX /FD /c
# ADD CPP /nologo /MT /W3 /GX /O2 /I "..\include" /I "C:\SDKs\ImgSource" /I "C:\SDKs\DXSDK\include" /I "../BaseClasses" /I "C:\SDKs\SS2\Include" /D "DVBC" /D "INC_OLE2" /D "STRICT" /D _WIN32_WINNT=0x0400 /D "WIN32" /D "_WIN32" /D "_MT" /D "_DLL" /D _X86_=1 /D WINVER=0x0400 /D "NDEBUG" /D "_NDEBUG" /D try=__try /D except=__except /D leave=__leave /D finally=__finally /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 ../TSReader_SourceHelper.lib strmbase.lib msvcrt.lib ole32.lib oleaut32.lib kernel32.lib user32.lib gdi32.lib /nologo /subsystem:windows /dll /machine:I386 /nodefaultlib /out:"../Sources/TSReader_B2C2s.dll"
# ADD LINK32 ../TSReader_SourceHelper.lib Version.lib strmbase.lib msvcrt.lib ole32.lib oleaut32.lib kernel32.lib user32.lib gdi32.lib /nologo /subsystem:windows /dll /machine:I386 /nodefaultlib /out:"../Sources/TSReader_B2C2c.dll"

!ELSEIF  "$(CFG)" == "TSReader_B2C2 - Win32 QAM Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "TSReader_B2C2___Win32_QAM_Debug"
# PROP BASE Intermediate_Dir "TSReader_B2C2___Win32_QAM_Debug"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "TSReader_B2C2___Win32_QAM_Debug"
# PROP Intermediate_Dir "TSReader_B2C2___Win32_QAM_Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /I "..\include" /I "C:\SDKs\ImgSource" /I "C:\SDKs\DXSDK\include" /I "../BaseClasses" /I "C:\SDKs\SS2\Include" /D "ATSC" /D "INC_OLE2" /D "STRICT" /D _WIN32_WINNT=0x0400 /D "WIN32" /D "_WIN32" /D "_MT" /D "_DLL" /D _X86_=1 /D WINVER=0x0400 /D DBG=1 /D "DEBUG" /D "_DEBUG" /D try=__try /D except=__except /D leave=__leave /D finally=__finally /FR /YX /FD /GZ /c
# ADD CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /I "..\include" /I "C:\SDKs\ImgSource" /I "C:\SDKs\DXSDK\include" /I "../BaseClasses" /I "C:\SDKs\SS2\Include" /D "QAM" /D "ATSC" /D "INC_OLE2" /D "STRICT" /D _WIN32_WINNT=0x0400 /D "WIN32" /D "_WIN32" /D "_MT" /D "_DLL" /D _X86_=1 /D WINVER=0x0400 /D DBG=1 /D "DEBUG" /D "_DEBUG" /D try=__try /D except=__except /D leave=__leave /D finally=__finally /FR /YX /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 ../TSReader_SourceHelper.lib Version.lib strmbasd.lib msvcrtd.lib ole32.lib oleaut32.lib kernel32.lib user32.lib gdi32.lib /nologo /subsystem:windows /dll /debug /machine:I386 /nodefaultlib /out:"../Sources/TSReader_B2C2a.dll" /pdbtype:sept /libpath:"..\BaseClasses\debug"
# ADD LINK32 ../TSReader_SourceHelper.lib Version.lib strmbasd.lib msvcrtd.lib ole32.lib oleaut32.lib kernel32.lib user32.lib gdi32.lib /nologo /subsystem:windows /dll /debug /machine:I386 /nodefaultlib /out:"../Sources/TSReader_B2C2q.dll" /pdbtype:sept /libpath:"..\BaseClasses\debug"

!ELSEIF  "$(CFG)" == "TSReader_B2C2 - Win32 QAM Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "TSReader_B2C2___Win32_QAM_Release"
# PROP BASE Intermediate_Dir "TSReader_B2C2___Win32_QAM_Release"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "TSReader_B2C2___Win32_QAM_Release"
# PROP Intermediate_Dir "TSReader_B2C2___Win32_QAM_Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /I "..\include" /I "C:\SDKs\ImgSource" /I "C:\SDKs\DXSDK\include" /I "../BaseClasses" /I "C:\SDKs\SS2\Include" /D "ATSC" /D "INC_OLE2" /D "STRICT" /D _WIN32_WINNT=0x0400 /D "WIN32" /D "_WIN32" /D "_MT" /D "_DLL" /D _X86_=1 /D WINVER=0x0400 /D "NDEBUG" /D "_NDEBUG" /D try=__try /D except=__except /D leave=__leave /D finally=__finally /YX /FD /c
# ADD CPP /nologo /MT /W3 /GX /O2 /I "..\include" /I "C:\SDKs\ImgSource" /I "C:\SDKs\DXSDK\include" /I "../BaseClasses" /I "C:\SDKs\SS2\Include" /D "QAM" /D "ATSC" /D "INC_OLE2" /D "STRICT" /D _WIN32_WINNT=0x0400 /D "WIN32" /D "_WIN32" /D "_MT" /D "_DLL" /D _X86_=1 /D WINVER=0x0400 /D "NDEBUG" /D "_NDEBUG" /D try=__try /D except=__except /D leave=__leave /D finally=__finally /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 ../TSReader_SourceHelper.lib Version.lib strmbase.lib msvcrt.lib ole32.lib oleaut32.lib kernel32.lib user32.lib gdi32.lib /nologo /subsystem:windows /dll /machine:I386 /nodefaultlib /out:"../Sources/TSReader_B2C2a.dll"
# ADD LINK32 ../TSReader_SourceHelper.lib Version.lib strmbase.lib msvcrt.lib ole32.lib oleaut32.lib kernel32.lib user32.lib gdi32.lib /nologo /subsystem:windows /dll /machine:I386 /nodefaultlib /out:"../Sources/TSReader_B2C2q.dll"

!ENDIF 

# Begin Target

# Name "TSReader_B2C2 - Win32 Release"
# Name "TSReader_B2C2 - Win32 Debug"
# Name "TSReader_B2C2 - Win32 DVBS Debug"
# Name "TSReader_B2C2 - Win32 DVBS Release"
# Name "TSReader_B2C2 - Win32 DVBT Debug"
# Name "TSReader_B2C2 - Win32 DVBC Debug"
# Name "TSReader_B2C2 - Win32 DVBT Release"
# Name "TSReader_B2C2 - Win32 DVBC Release"
# Name "TSReader_B2C2 - Win32 QAM Debug"
# Name "TSReader_B2C2 - Win32 QAM Release"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=..\..\..\SDKs\SS2\src\B2C2MPEG2AdapterWin.cpp
# End Source File
# Begin Source File

SOURCE=.\TSReader_B2C2.cpp
# End Source File
# Begin Source File

SOURCE=.\TSReader_B2C2.def
# End Source File
# Begin Source File

SOURCE=.\TSReader_B2C2.rc
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# End Group
# End Target
# End Project
