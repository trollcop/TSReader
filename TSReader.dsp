# Microsoft Developer Studio Project File - Name="TSReader" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=TSReader - Win32 Pro Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "TSReader.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "TSReader.mak" CFG="TSReader - Win32 Pro Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "TSReader - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "TSReader - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE "TSReader - Win32 Lite Release" (based on "Win32 (x86) Application")
!MESSAGE "TSReader - Win32 Lite Debug" (based on "Win32 (x86) Application")
!MESSAGE "TSReader - Win32 IneoQuest Release" (based on "Win32 (x86) Application")
!MESSAGE "TSReader - Win32 IneoQuest Debug" (based on "Win32 (x86) Application")
!MESSAGE "TSReader - Win32 Pro Debug" (based on "Win32 (x86) Application")
!MESSAGE "TSReader - Win32 Pro Release" (based on "Win32 (x86) Application")
!MESSAGE "TSReader - Win32 ZAnalyzer Pro Release" (based on "Win32 (x86) Application")
!MESSAGE "TSReader - Win32 DTVSentinel Pro Release" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "TSReader - Win32 Release"

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
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /MT /W3 /O2 /I "include" /I "c:\sdks\imgsource" /I "c:\sdks\stradis\include" /I "C:\SDKs\libmad-0.15.1b\msvc++" /I "C:\SDKs\a52dec-0.7.4\include" /I "C:\SDKs\a52dec-0.7.4\libao" /D "NONOEM" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /FD /G7 /QxM /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NONOEM" /d "NDEBUG" /d "STANDARD_ICON"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:I386
# ADD LINK32 libfaad2.lib liba52.lib libao.lib libmad.lib PEGRPCS.lib _isource.lib libvo.lib libmpeg2.lib libmpeg2convert.lib wsock32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib setupapi.lib comctl32.lib winmm.lib ole32.lib Wininet.lib /nologo /subsystem:windows /machine:I386 /out:"TSReader.exe" /libpath:"C:\SDKs\ImgSource" /libpath:"C:\SDKs\libmad-0.15.1b\msvc++\Release" /libpath:"C:\SDKs\a52dec-0.7.4\vc++\Release"
# SUBTRACT LINK32 /profile /pdb:none /incremental:yes /nodefaultlib /force
# Begin Special Build Tool
SOURCE="$(InputPath)"
PostBuild_Cmds=copy sources.h SampleSource\inc
# End Special Build Tool

!ELSEIF  "$(CFG)" == "TSReader - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "TSReader___Win32_Debug"
# PROP BASE Intermediate_Dir "TSReader___Win32_Debug"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /I "include" /I "c:\sdks\imgsource" /I "c:\sdks\stradis\include" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "FILE_MODE" /YX /FD /GZ /c
# ADD CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /I "include" /I "c:\sdks\imgsource" /I "c:\sdks\stradis\include" /I "C:\SDKs\libmad-0.15.1b\msvc++" /I "C:\SDKs\a52dec-0.7.4\include" /I "C:\SDKs\a52dec-0.7.4\libao" /D "NONOEM" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /FR /YX /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "NONOEM" /d "_DEBUG" /d "STANDARD_ICON"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 _isource.lib libvo.lib libmpeg2.lib wsock32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib setupapi.lib comctl32.lib /nologo /subsystem:windows /debug /machine:I386 /out:"TSReader_FI.exe" /pdbtype:sept /libpath:"C:\SDKs\ImgSource"
# ADD LINK32 TSReader_Scheduler.lib libfaad2.lib liba52.lib libao.lib libmad.lib PEGRPCS.lib _isource.lib libvo.lib libmpeg2.lib libmpeg2convert.lib wsock32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib setupapi.lib comctl32.lib winmm.lib ole32.lib Wininet.lib /nologo /subsystem:windows /profile /debug /machine:I386 /out:"TSReader.exe" /libpath:"C:\SDKs\ImgSource" /libpath:"C:\SDKs\libmad-0.15.1b\msvc++\Debug" /libpath:"C:\SDKs\a52dec-0.7.4\vc++\Debug"
# SUBTRACT LINK32 /nodefaultlib
# Begin Special Build Tool
SOURCE="$(InputPath)"
PostBuild_Cmds=copy sources.h SampleSource\inc
# End Special Build Tool

!ELSEIF  "$(CFG)" == "TSReader - Win32 Lite Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "TSReader___Win32_Lite_Release"
# PROP BASE Intermediate_Dir "TSReader___Win32_Lite_Release"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "TSReader___Win32_Lite_Release"
# PROP Intermediate_Dir "TSReader___Win32_Lite_Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /I "include" /I "c:\sdks\imgsource" /I "c:\sdks\stradis\include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /MT /W3 /GX /O2 /I "include" /I "c:\sdks\imgsource" /I "c:\sdks\stradis\include" /I "C:\SDKs\libmad-0.15.1b\msvc++" /I "C:\SDKs\a52dec-0.7.4\include" /I "C:\SDKs\a52dec-0.7.4\libao" /D "NONOEM" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "LITE" /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NONOEM" /d "NDEBUG" /d "LITE" /d "STANDARD_ICON"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 _isource.lib libvo.lib libmpeg2.lib wsock32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib setupapi.lib comctl32.lib /nologo /subsystem:windows /machine:I386 /out:"TSReader.exe" /libpath:"C:\SDKs\ImgSource"
# SUBTRACT BASE LINK32 /profile /nodefaultlib
# ADD LINK32 TSReader_SourceHelper.lib liba52.lib libao.lib libmad.lib PEGRPCS.lib _isource.lib libvo.lib libmpeg2.lib libmpeg2convert.lib wsock32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib setupapi.lib comctl32.lib winmm.lib ole32.lib wininet.lib /nologo /subsystem:windows /machine:I386 /out:"TSReaderLite.exe" /libpath:"C:\SDKs\ImgSource" /libpath:"C:\SDKs\libmad-0.15.1b\msvc++\Release" /libpath:"C:\SDKs\a52dec-0.7.4\vc++\Release"
# SUBTRACT LINK32 /profile /nodefaultlib

!ELSEIF  "$(CFG)" == "TSReader - Win32 Lite Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "TSReader___Win32_Lite_Debug"
# PROP BASE Intermediate_Dir "TSReader___Win32_Lite_Debug"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "TSReader___Win32_Lite_Debug"
# PROP Intermediate_Dir "TSReader___Win32_Lite_Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /I "include" /I "c:\sdks\imgsource" /I "c:\sdks\stradis\include" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "INCLUDE_CSA" /YX /FD /GZ /c
# ADD CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /I "include" /I "c:\sdks\imgsource" /I "c:\sdks\stradis\include" /D "NONOEM" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "LITE" /YX /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "NONOEM" /d "_DEBUG" /d "LITE" /d "STANDARD_ICON"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 _isource.lib libvo.lib libmpeg2.lib wsock32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib setupapi.lib comctl32.lib /nologo /subsystem:windows /profile /debug /machine:I386 /out:"TSReader.exe" /libpath:"C:\SDKs\ImgSource"
# SUBTRACT BASE LINK32 /nodefaultlib
# ADD LINK32 PEGRPCS.lib TSReader_SourceHelper.lib _isource.lib libvo.lib libmpeg2.lib libmpeg2convert.lib wsock32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib setupapi.lib comctl32.lib ole32.lib wininet.lib /nologo /subsystem:windows /profile /debug /machine:I386 /out:"TSReaderLite.exe" /libpath:"C:\SDKs\ImgSource"
# SUBTRACT LINK32 /nodefaultlib

!ELSEIF  "$(CFG)" == "TSReader - Win32 IneoQuest Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "TSReader___Win32_IneoQuest_Release"
# PROP BASE Intermediate_Dir "TSReader___Win32_IneoQuest_Release"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "TSReader___Win32_IneoQuest_Release"
# PROP Intermediate_Dir "TSReader___Win32_IneoQuest_Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /I "include" /I "c:\sdks\imgsource" /I "c:\sdks\stradis\include" /D "NONOEM" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /MT /W3 /GX /O2 /I "include" /I "c:\sdks\imgsource" /I "c:\sdks\stradis\include" /I "C:\SDKs\libmad-0.15.1b\msvc++" /I "C:\SDKs\a52dec-0.7.4\include" /I "C:\SDKs\a52dec-0.7.4\libao" /D "INEOQUEST" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NONOEM" /d "NDEBUG"
# ADD RSC /l 0x409 /d "INEOQUEST" /d "NDEBUG" /d "STANDARD_ICON"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 PEGRPCS.lib _isource.lib libvo.lib libmpeg2.lib libmpeg2convert.lib wsock32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib setupapi.lib comctl32.lib /nologo /subsystem:windows /machine:I386 /out:"TSReader.exe" /libpath:"C:\SDKs\ImgSource"
# SUBTRACT BASE LINK32 /profile /pdb:none /incremental:yes /nodefaultlib /force
# ADD LINK32 liba52.lib libao.lib libmad.lib PEGRPCS.lib TSReader_SourceHelper.lib _isource.lib libvo.lib libmpeg2.lib libmpeg2convert.lib wsock32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib setupapi.lib comctl32.lib winmm.lib ole32.lib /nologo /subsystem:windows /machine:I386 /out:"TSReaderIneoQuest.exe" /libpath:"C:\SDKs\ImgSource" /libpath:"C:\SDKs\libmad-0.15.1b\msvc++\Release" /libpath:"C:\SDKs\a52dec-0.7.4\vc++\Release"
# SUBTRACT LINK32 /profile /pdb:none /incremental:yes /nodefaultlib /force

!ELSEIF  "$(CFG)" == "TSReader - Win32 IneoQuest Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "TSReader___Win32_IneoQuest_Debug"
# PROP BASE Intermediate_Dir "TSReader___Win32_IneoQuest_Debug"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "TSReader___Win32_IneoQuest_Debug"
# PROP Intermediate_Dir "TSReader___Win32_IneoQuest_Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /I "include" /I "c:\sdks\imgsource" /I "c:\sdks\stradis\include" /I "C:\SDKs\libmad-0.15.1b\msvc++" /I "C:\SDKs\a52dec-0.7.4\include" /I "C:\SDKs\a52dec-0.7.4\libao" /D "NONOEM" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /FR /YX /FD /GZ /c
# ADD CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /I "include" /I "c:\sdks\imgsource" /I "c:\sdks\stradis\include" /I "C:\SDKs\libmad-0.15.1b\msvc++" /I "C:\SDKs\a52dec-0.7.4\include" /I "C:\SDKs\a52dec-0.7.4\libao" /D "INEOQUEST" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /FR /YX /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NONOEM" /d "_DEBUG"
# ADD RSC /l 0x409 /d "INEOQUEST" /d "_DEBUG" /d "STANDARD_ICON"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 liba52.lib libao.lib libmad.lib PEGRPCS.lib _isource.lib libvo.lib libmpeg2.lib libmpeg2convert.lib wsock32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib setupapi.lib comctl32.lib winmm.lib /nologo /subsystem:windows /profile /debug /machine:I386 /out:"TSReader.exe" /libpath:"C:\SDKs\ImgSource" /libpath:"C:\SDKs\libmad-0.15.1b\msvc++\Debug" /libpath:"C:\SDKs\a52dec-0.7.4\vc++\Debug"
# SUBTRACT BASE LINK32 /nodefaultlib
# ADD LINK32 TSReader_SourceHelper.lib liba52.lib libao.lib libmad.lib PEGRPCS.lib _isource.lib libvo.lib libmpeg2.lib libmpeg2convert.lib wsock32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib setupapi.lib comctl32.lib winmm.lib ole32.lib /nologo /subsystem:windows /profile /debug /machine:I386 /out:"TSReaderIneoQuest.exe" /libpath:"C:\SDKs\ImgSource" /libpath:"C:\SDKs\libmad-0.15.1b\msvc++\Debug" /libpath:"C:\SDKs\a52dec-0.7.4\vc++\Debug"
# SUBTRACT LINK32 /nodefaultlib
# Begin Special Build Tool
SOURCE="$(InputPath)"
PostBuild_Cmds=copy sources.h SampleSource\inc
# End Special Build Tool

!ELSEIF  "$(CFG)" == "TSReader - Win32 Pro Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "TSReader___Win32_Pro_Debug"
# PROP BASE Intermediate_Dir "TSReader___Win32_Pro_Debug"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "TSReader___Win32_Pro_Debug"
# PROP Intermediate_Dir "TSReader___Win32_Pro_Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /I "include" /I "c:\sdks\imgsource" /I "c:\sdks\stradis\include" /I "C:\SDKs\libmad-0.15.1b\msvc++" /I "C:\SDKs\a52dec-0.7.4\include" /I "C:\SDKs\a52dec-0.7.4\libao" /D "NONOEM" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /FR /YX /FD /GZ /c
# ADD CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /I "include" /I "c:\sdks\imgsource" /I "c:\sdks\stradis\include" /I "C:\SDKs\libmad-0.15.1b\msvc++" /I "C:\SDKs\a52dec-0.7.4\include" /I "C:\SDKs\a52dec-0.7.4\libao" /I "C:\Program Files\Aladdin\HASP HL\API\Runtime\C\win32" /I "C:\SDKs\fvdmp4_1.0" /D "PRO" /D "NONOEM" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /FR /YX /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NONOEM" /d "_DEBUG"
# ADD RSC /l 0x409 /d "PRO" /d "NONOEM" /d "_DEBUG" /d "STANDARD_ICON"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 TSReader_Scheduler.lib liba52.lib libao.lib libmad.lib PEGRPCS.lib _isource.lib libvo.lib libmpeg2.lib libmpeg2convert.lib wsock32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib setupapi.lib comctl32.lib winmm.lib ole32.lib /nologo /subsystem:windows /profile /debug /machine:I386 /out:"TSReader.exe" /libpath:"C:\SDKs\ImgSource" /libpath:"C:\SDKs\libmad-0.15.1b\msvc++\Debug" /libpath:"C:\SDKs\a52dec-0.7.4\vc++\Debug"
# SUBTRACT BASE LINK32 /nodefaultlib
# ADD LINK32 TSReader_SourceHelper.lib TSReader_Scheduler.lib libfaad2.lib fvdmp4.lib liba52.lib libao.lib libmad.lib PEGRPCS.lib _isource.lib libvo.lib libmpeg2.lib libmpeg2convert.lib Wininet.lib wsock32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib setupapi.lib comctl32.lib winmm.lib ole32.lib libhasp_windows.lib /nologo /subsystem:windows /profile /debug /machine:I386 /out:"TSReaderPro.exe" /libpath:"C:\SDKs\ImgSource" /libpath:"C:\SDKs\libmad-0.15.1b\msvc++\Debug" /libpath:"C:\SDKs\a52dec-0.7.4\vc++\Debug" /libpath:"C:\Program Files\Aladdin\HASP HL\API\Runtime\C\win32" /libpath:"C:\SDKs\fvdmp4_1.0"
# SUBTRACT LINK32 /nodefaultlib
# Begin Special Build Tool
SOURCE="$(InputPath)"
PostBuild_Cmds=copy sources.h SampleSource\inc
# End Special Build Tool

!ELSEIF  "$(CFG)" == "TSReader - Win32 Pro Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "TSReader___Win32_Pro_Release"
# PROP BASE Intermediate_Dir "TSReader___Win32_Pro_Release"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "TSReader___Win32_Pro_Release"
# PROP Intermediate_Dir "TSReader___Win32_Pro_Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /O2 /I "include" /I "c:\sdks\imgsource" /I "c:\sdks\stradis\include" /I "C:\SDKs\libmad-0.15.1b\msvc++" /I "C:\SDKs\a52dec-0.7.4\include" /I "C:\SDKs\a52dec-0.7.4\libao" /D "NONOEM" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /FD /G7 /QxM /c
# ADD CPP /nologo /MT /W3 /O2 /I "include" /I "c:\sdks\imgsource" /I "c:\sdks\stradis\include" /I "C:\SDKs\libmad-0.15.1b\msvc++" /I "C:\SDKs\a52dec-0.7.4\include" /I "C:\SDKs\a52dec-0.7.4\libao" /I "C:\Program Files\Aladdin\HASP HL\API\Runtime\C\win32" /I "C:\SDKs\fvdmp4_1.0" /D "PRO" /D "NONOEM" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /FD /G7 /QxM /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NONOEM" /d "NDEBUG"
# ADD RSC /l 0x409 /d "PRO" /d "NONOEM" /d "NDEBUG" /d "STANDARD_ICON"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 liba52.lib libao.lib libmad.lib PEGRPCS.lib _isource.lib libvo.lib libmpeg2.lib libmpeg2convert.lib wsock32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib setupapi.lib comctl32.lib winmm.lib ole32.lib /nologo /subsystem:windows /machine:I386 /out:"TSReader.exe" /libpath:"C:\SDKs\ImgSource" /libpath:"C:\SDKs\libmad-0.15.1b\msvc++\Release" /libpath:"C:\SDKs\a52dec-0.7.4\vc++\Release"
# SUBTRACT BASE LINK32 /profile /pdb:none /incremental:yes /nodefaultlib /force
# ADD LINK32 TSReader_SourceHelper.lib libfaad2.lib liba52.lib libao.lib fvdmp4.lib libmad.lib PEGRPCS.lib _isource.lib libvo.lib libmpeg2.lib libmpeg2convert.lib wsock32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib setupapi.lib comctl32.lib winmm.lib ole32.lib libhasp_windows.lib Wininet.lib /nologo /subsystem:windows /machine:I386 /out:"TSReaderPro.exe" /libpath:"C:\SDKs\ImgSource" /libpath:"C:\SDKs\libmad-0.15.1b\msvc++\Release" /libpath:"C:\SDKs\a52dec-0.7.4\vc++\Release" /libpath:"C:\Program Files\Aladdin\HASP HL\API\Runtime\C\win32" /libpath:"C:\SDKs\fvdmp4_1.0"
# SUBTRACT LINK32 /profile /pdb:none /incremental:yes /nodefaultlib /force
# Begin Special Build Tool
SOURCE="$(InputPath)"
PostBuild_Cmds="C:\Program Files\Aladdin\HASP HL\VendorTools\VendorCenter\envelope" -p "C:\Program Files\Aladdin\HASP HL\VendorTools\VendorCenter\projects\TSReaderPro.prjx"	copy protected\TSReaderPro-protected.exe .
# End Special Build Tool

!ELSEIF  "$(CFG)" == "TSReader - Win32 ZAnalyzer Pro Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "TSReader___Win32_ZAnalyzer_Pro_Release"
# PROP BASE Intermediate_Dir "TSReader___Win32_ZAnalyzer_Pro_Release"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "TSReader___Win32_ZAnalyzer_Pro_Release"
# PROP Intermediate_Dir "TSReader___Win32_ZAnalyzer_Pro_Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /O2 /I "include" /I "c:\sdks\imgsource" /I "c:\sdks\stradis\include" /I "C:\SDKs\libmad-0.15.1b\msvc++" /I "C:\SDKs\a52dec-0.7.4\include" /I "C:\SDKs\a52dec-0.7.4\libao" /D "PRO" /D "NONOEM" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /FD /G7 /QxM /c
# ADD CPP /nologo /MT /W3 /O2 /I "include" /I "c:\sdks\imgsource" /I "c:\sdks\stradis\include" /I "C:\SDKs\libmad-0.15.1b\msvc++" /I "C:\SDKs\a52dec-0.7.4\include" /I "C:\SDKs\a52dec-0.7.4\libao" /I "C:\Program Files\Aladdin\HASP HL\API\Runtime\C\win32" /I "C:\SDKs\fvdmp4_1.0" /D "ZANALYZER" /D "PRO" /D "NONOEM" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /FD /G7 /QxM /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NONOEM" /d "NDEBUG"
# ADD RSC /l 0x409 /d "PRO" /d "NONOEM" /d "NDEBUG" /d "STANDARD_ICON"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 TSReader_SourceHelper.lib liba52.lib libao.lib libmad.lib PEGRPCS.lib _isource.lib libvo.lib libmpeg2.lib libmpeg2convert.lib wsock32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib setupapi.lib comctl32.lib winmm.lib ole32.lib /nologo /subsystem:windows /machine:I386 /out:"TSReaderPro.exe" /libpath:"C:\SDKs\ImgSource" /libpath:"C:\SDKs\libmad-0.15.1b\msvc++\Release" /libpath:"C:\SDKs\a52dec-0.7.4\vc++\Release"
# SUBTRACT BASE LINK32 /profile /pdb:none /incremental:yes /nodefaultlib /force
# ADD LINK32 TSReader_SourceHelper.lib liba52.lib libao.lib fvdmp4.lib libmad.lib PEGRPCS.lib _isource.lib libvo.lib libmpeg2.lib libmpeg2convert.lib wsock32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib setupapi.lib comctl32.lib winmm.lib ole32.lib libhasp_windows.lib wininet.lib /nologo /subsystem:windows /machine:I386 /out:"ZAnalyzer.exe" /libpath:"C:\SDKs\ImgSource" /libpath:"C:\SDKs\libmad-0.15.1b\msvc++\Release" /libpath:"C:\SDKs\a52dec-0.7.4\vc++\Release" /libpath:"C:\Program Files\Aladdin\HASP HL\API\Runtime\C\win32" /libpath:"C:\SDKs\fvdmp4_1.0"
# SUBTRACT LINK32 /profile /pdb:none /incremental:yes /nodefaultlib /force
# Begin Special Build Tool
SOURCE="$(InputPath)"
PostBuild_Desc=Protecting
PostBuild_Cmds="C:\Program Files\Aladdin\HASP HL\VendorTools\VendorCenter\envelope" -p "C:\Program Files\Aladdin\HASP HL\VendorTools\VendorCenter\projects\ZAnalyzer.prjx"	copy protected\ZAnalyzer.exe .
# End Special Build Tool

!ELSEIF  "$(CFG)" == "TSReader - Win32 DTVSentinel Pro Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "TSReader___Win32_DTVSentinel_Pro_Release"
# PROP BASE Intermediate_Dir "TSReader___Win32_DTVSentinel_Pro_Release"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "TSReader___Win32_DTVSentinel_Pro_Release"
# PROP Intermediate_Dir "TSReader___Win32_DTVSentinel_Pro_Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /O2 /I "include" /I "c:\sdks\imgsource" /I "c:\sdks\stradis\include" /I "C:\SDKs\libmad-0.15.1b\msvc++" /I "C:\SDKs\a52dec-0.7.4\include" /I "C:\SDKs\a52dec-0.7.4\libao" /D "ZANALYZER" /D "PRO" /D "NONOEM" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /FD /G7 /QxM /c
# ADD CPP /nologo /MT /W3 /O2 /I "include" /I "c:\sdks\imgsource" /I "c:\sdks\stradis\include" /I "C:\SDKs\libmad-0.15.1b\msvc++" /I "C:\SDKs\a52dec-0.7.4\include" /I "C:\SDKs\a52dec-0.7.4\libao" /I "C:\Program Files\Aladdin\HASP HL\API\Runtime\C\win32" /I "C:\SDKs\fvdmp4_1.0" /D "DTVSENTINEL" /D "PRO" /D "NONOEM" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /FD /G7 /QxM /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NONOEM" /d "NDEBUG"
# ADD RSC /l 0x409 /d "PRO" /d "NONOEM" /d "NDEBUG" /d "ATSC_ICON"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 TSReader_SourceHelper.lib liba52.lib libao.lib libmad.lib PEGRPCS.lib _isource.lib libvo.lib libmpeg2.lib libmpeg2convert.lib wsock32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib setupapi.lib comctl32.lib winmm.lib ole32.lib /nologo /subsystem:windows /machine:I386 /out:"ZAnalyzer.exe" /libpath:"C:\SDKs\ImgSource" /libpath:"C:\SDKs\libmad-0.15.1b\msvc++\Release" /libpath:"C:\SDKs\a52dec-0.7.4\vc++\Release"
# SUBTRACT BASE LINK32 /profile /pdb:none /incremental:yes /nodefaultlib /force
# ADD LINK32 TSReader_SourceHelper.lib liba52.lib libao.lib libmad.lib PEGRPCS.lib _isource.lib libvo.lib libmpeg2.lib libmpeg2convert.lib wsock32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib setupapi.lib comctl32.lib winmm.lib ole32.lib libhasp_windows.lib fvdmp4.lib wininet.lib /nologo /subsystem:windows /machine:I386 /out:"DTVSentinel.exe" /libpath:"C:\SDKs\ImgSource" /libpath:"C:\SDKs\libmad-0.15.1b\msvc++\Release" /libpath:"C:\SDKs\a52dec-0.7.4\vc++\Release" /libpath:"C:\Program Files\Aladdin\HASP HL\API\Runtime\C\win32" /libpath:"C:\SDKs\fvdmp4_1.0"
# SUBTRACT LINK32 /profile /pdb:none /incremental:yes /nodefaultlib /force
# Begin Special Build Tool
SOURCE="$(InputPath)"
PostBuild_Desc=Protecting
PostBuild_Cmds="C:\Program Files\Aladdin\HASP HL\VendorTools\VendorCenter\envelope" -p "C:\Program Files\Aladdin\HASP HL\VendorTools\VendorCenter\projects\DTVSentinel.prjx"	copy protected\DTVSentinel.exe .
# End Special Build Tool

!ENDIF 

# Begin Target

# Name "TSReader - Win32 Release"
# Name "TSReader - Win32 Debug"
# Name "TSReader - Win32 Lite Release"
# Name "TSReader - Win32 Lite Debug"
# Name "TSReader - Win32 IneoQuest Release"
# Name "TSReader - Win32 IneoQuest Debug"
# Name "TSReader - Win32 Pro Debug"
# Name "TSReader - Win32 Pro Release"
# Name "TSReader - Win32 ZAnalyzer Pro Release"
# Name "TSReader - Win32 DTVSentinel Pro Release"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\ATSC_huffman.h
# End Source File
# Begin Source File

SOURCE=.\bcdmux.h
# End Source File
# Begin Source File

SOURCE=.\CCDecoder.h
# End Source File
# Begin Source File

SOURCE=.\CCDecoder_Strings.h
# End Source File
# Begin Source File

SOURCE=.\charting.h
# End Source File
# Begin Source File

SOURCE=".\CI-CAM.h"
# End Source File
# Begin Source File

SOURCE=.\dn_huffman.h
# End Source File
# Begin Source File

SOURCE=.\EPGGrid.h
# End Source File
# Begin Source File

SOURCE=.\epgschedule.h
# End Source File
# Begin Source File

SOURCE=.\formatter.h
# End Source File
# Begin Source File

SOURCE=.\forwarder.h
# End Source File
# Begin Source File

SOURCE=.\MDInterface.h
# End Source File
# Begin Source File

SOURCE=.\parser.h
# End Source File
# Begin Source File

SOURCE=".\reed-solomon.h"
# End Source File
# Begin Source File

SOURCE=.\registry_list.h
# End Source File
# Begin Source File

SOURCE=.\resource.h
# End Source File
# Begin Source File

SOURCE=.\sky_genres.h
# End Source File
# Begin Source File

SOURCE=.\SoftCSA.h
# End Source File
# Begin Source File

SOURCE=.\sources.h
# End Source File
# Begin Source File

SOURCE=.\TSID.h
# End Source File
# Begin Source File

SOURCE=.\TSReader.h
# End Source File
# Begin Source File

SOURCE=.\util.h
# End Source File
# End Group
# Begin Source File

SOURCE=.\AACAudioDecoder.c
# End Source File
# Begin Source File

SOURCE=.\AC3AudioDecoder.c
# End Source File
# Begin Source File

SOURCE=.\archive.c
# End Source File
# Begin Source File

SOURCE=.\ATSC_huffman.c
# End Source File
# Begin Source File

SOURCE=.\bcdmux.c
# End Source File
# Begin Source File

SOURCE=.\CCDecoder.c
# End Source File
# Begin Source File

SOURCE=.\charting.c
# End Source File
# Begin Source File

SOURCE=".\CI-CAM.c"
# End Source File
# Begin Source File

SOURCE=.\ControlServer.c
# End Source File
# Begin Source File

SOURCE=.\dn_huffman.c
# End Source File
# Begin Source File

SOURCE=.\EITServer.c
# End Source File
# Begin Source File

SOURCE=.\email.c
# End Source File
# Begin Source File

SOURCE=.\EPGGrid.c
# End Source File
# Begin Source File

SOURCE=.\export.c
# End Source File
# Begin Source File

SOURCE=.\formatter.c
# End Source File
# Begin Source File

SOURCE=.\Forwarder.c
# End Source File
# Begin Source File

SOURCE=.\GPSSignal.c
# End Source File
# Begin Source File

SOURCE=.\H264Decoder.c
# End Source File
# Begin Source File

SOURCE=.\MDInterface.c
# End Source File
# Begin Source File

SOURCE=.\mosaic.c
# End Source File
# Begin Source File

SOURCE=.\MPEG2Decoder.c
# End Source File
# Begin Source File

SOURCE=.\MPEG4Decoder.c
# End Source File
# Begin Source File

SOURCE=.\MPEGAudio.c
# End Source File
# Begin Source File

SOURCE=.\parser.c
# End Source File
# Begin Source File

SOURCE=.\ProfileBrowser.c
# End Source File
# Begin Source File

SOURCE=".\reed-solomon.c"
# End Source File
# Begin Source File

SOURCE=.\RokuTelnetInterface.c
# End Source File
# Begin Source File

SOURCE=.\settings.c
# End Source File
# Begin Source File

SOURCE=.\sky_epg.c
# End Source File
# Begin Source File

SOURCE=.\sky_huffman.c
# End Source File
# Begin Source File

SOURCE=.\SoftCSA.c
# End Source File
# Begin Source File

SOURCE=.\StreamMonitor.c
# End Source File
# Begin Source File

SOURCE=.\TitleThumbnails.c
# End Source File
# Begin Source File

SOURCE=.\TSReader.c
# End Source File
# Begin Source File

SOURCE=.\TSReader.rc
# End Source File
# Begin Source File

SOURCE=.\util.c
# End Source File
# Begin Source File

SOURCE=.\VC1Decoder.c
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\aspect149.bmp
# End Source File
# Begin Source File

SOURCE=.\aspect169.bmp
# End Source File
# Begin Source File

SOURCE=.\aspect43.bmp
# End Source File
# Begin Source File

SOURCE=.\aspect_a.bmp
# End Source File
# Begin Source File

SOURCE=.\atsc_cc.bmp
# End Source File
# Begin Source File

SOURCE=.\atsc_rc.bmp
# End Source File
# Begin Source File

SOURCE=.\aud_aac.bmp
# End Source File
# Begin Source File

SOURCE=.\aud_ac3.bmp
# End Source File
# Begin Source File

SOURCE=.\aud_mpeg.bmp
# End Source File
# Begin Source File

SOURCE=.\baud_aac.bmp
# End Source File
# Begin Source File

SOURCE=.\baud_ac3.bmp
# End Source File
# Begin Source File

SOURCE=.\baud_mpe.bmp
# End Source File
# Begin Source File

SOURCE=.\bitmap1.bmp
# End Source File
# Begin Source File

SOURCE=.\bitmap2.bmp
# End Source File
# Begin Source File

SOURCE=.\bitmap3.bmp
# End Source File
# Begin Source File

SOURCE=.\blank.ico
# End Source File
# Begin Source File

SOURCE=.\bomb.ico
# End Source File
# Begin Source File

SOURCE=.\bvid_dcii.bmp
# End Source File
# Begin Source File

SOURCE=.\bvid_h26.bmp
# End Source File
# Begin Source File

SOURCE=.\bvid_mp2.bmp
# End Source File
# Begin Source File

SOURCE=.\bvid_mp4.bmp
# End Source File
# Begin Source File

SOURCE=.\bvid_vc1.bmp
# End Source File
# Begin Source File

SOURCE=.\dvb.ico
# End Source File
# Begin Source File

SOURCE=.\dvb_logo_small_black.ico
# End Source File
# Begin Source File

SOURCE=.\dvb_logo_small_red.ico
# End Source File
# Begin Source File

SOURCE=.\dvba_logo.ico
# End Source File
# Begin Source File

SOURCE=.\dvbar_logo.ico
# End Source File
# Begin Source File

SOURCE=.\dvbsmall.ico
# End Source File
# Begin Source File

SOURCE=.\fwd_1.ico
# End Source File
# Begin Source File

SOURCE=.\fwd_2.ico
# End Source File
# Begin Source File

SOURCE=.\fwd_3.ico
# End Source File
# Begin Source File

SOURCE=.\fwd_4.ico
# End Source File
# Begin Source File

SOURCE=.\ico00001.ico
# End Source File
# Begin Source File

SOURCE=.\ico00002.ico
# End Source File
# Begin Source File

SOURCE=.\ico00003.ico
# End Source File
# Begin Source File

SOURCE=.\ico00004.ico
# End Source File
# Begin Source File

SOURCE=.\ico00005.ico
# End Source File
# Begin Source File

SOURCE=.\ico00006.ico
# End Source File
# Begin Source File

SOURCE=.\ico00007.ico
# End Source File
# Begin Source File

SOURCE=.\ico00008.ico
# End Source File
# Begin Source File

SOURCE=.\ico00009.ico
# End Source File
# Begin Source File

SOURCE=.\ico00010.ico
# End Source File
# Begin Source File

SOURCE=.\ico00011.ico
# End Source File
# Begin Source File

SOURCE=.\icon1.ico
# End Source File
# Begin Source File

SOURCE=.\idd_atsc.ico
# End Source File
# Begin Source File

SOURCE=.\idd_dvb_r.ico
# End Source File
# Begin Source File

SOURCE=.\Info.ico
# End Source File
# Begin Source File

SOURCE=.\log_warn.ico
# End Source File
# Begin Source File

SOURCE=.\monitor_disabled.ico
# End Source File
# Begin Source File

SOURCE=.\monitor_green.ico
# End Source File
# Begin Source File

SOURCE=.\monitor_grey.ico
# End Source File
# Begin Source File

SOURCE=.\monitor_half.ico
# End Source File
# Begin Source File

SOURCE=.\monitor_red.ico
# End Source File
# Begin Source File

SOURCE=.\monitor_yellow.ico
# End Source File
# Begin Source File

SOURCE=.\profile_.bmp
# End Source File
# Begin Source File

SOURCE=.\record.ico
# End Source File
# Begin Source File

SOURCE=.\run_2.ico
# End Source File
# Begin Source File

SOURCE=.\run_3.ico
# End Source File
# Begin Source File

SOURCE=.\run_4.ico
# End Source File
# Begin Source File

SOURCE=.\run_5.ico
# End Source File
# Begin Source File

SOURCE=.\si_audio_es.ico
# End Source File
# Begin Source File

SOURCE=.\si_base.ico
# End Source File
# Begin Source File

SOURCE=.\si_base1.ico
# End Source File
# Begin Source File

SOURCE=.\si_base2.ico
# End Source File
# Begin Source File

SOURCE=.\si_bat.ico
# End Source File
# Begin Source File

SOURCE=.\si_bat_s.ico
# End Source File
# Begin Source File

SOURCE=.\si_cat.ico
# End Source File
# Begin Source File

SOURCE=.\si_cdt.ico
# End Source File
# Begin Source File

SOURCE=.\si_data.ico
# End Source File
# Begin Source File

SOURCE=.\si_descriptor.ico
# End Source File
# Begin Source File

SOURCE=.\si_eit.ico
# End Source File
# Begin Source File

SOURCE=.\si_ip.ico
# End Source File
# Begin Source File

SOURCE=.\si_ip_sa.ico
# End Source File
# Begin Source File

SOURCE=.\si_ip_st.ico
# End Source File
# Begin Source File

SOURCE=.\si_mgt.ico
# End Source File
# Begin Source File

SOURCE=.\si_mmt.ico
# End Source File
# Begin Source File

SOURCE=.\si_nit.ico
# End Source File
# Begin Source File

SOURCE=.\si_nit_i.ico
# End Source File
# Begin Source File

SOURCE=.\si_nit_o.ico
# End Source File
# Begin Source File

SOURCE=.\si_pat.ico
# End Source File
# Begin Source File

SOURCE=.\si_pcr.ico
# End Source File
# Begin Source File

SOURCE=.\si_pmt.ico
# End Source File
# Begin Source File

SOURCE=.\si_sdt.ico
# End Source File
# Begin Source File

SOURCE=.\si_sdt_o.ico
# End Source File
# Begin Source File

SOURCE=.\si_sub.ico
# End Source File
# Begin Source File

SOURCE=.\si_tdt.ico
# End Source File
# Begin Source File

SOURCE=.\si_user.ico
# End Source File
# Begin Source File

SOURCE=.\si_vbi.ico
# End Source File
# Begin Source File

SOURCE=.\si_video_es.ico
# End Source File
# Begin Source File

SOURCE=.\stream.ico
# End Source File
# Begin Source File

SOURCE=.\tv_14d.ico
# End Source File
# Begin Source File

SOURCE=.\tv_ma.ico
# End Source File
# Begin Source File

SOURCE=.\tv_pg.ico
# End Source File
# Begin Source File

SOURCE=.\tv_y7fv.ico
# End Source File
# Begin Source File

SOURCE=.\txt_422.bmp
# End Source File
# Begin Source File

SOURCE=.\txt_cc.bmp
# End Source File
# Begin Source File

SOURCE=.\txt_icon.bmp
# End Source File
# Begin Source File

SOURCE=.\txt_itxt.bmp
# End Source File
# Begin Source File

SOURCE=.\txt_sub.bmp
# End Source File
# Begin Source File

SOURCE=.\txt_sub4.bmp
# End Source File
# Begin Source File

SOURCE=.\txt_txt.bmp
# End Source File
# Begin Source File

SOURCE=.\txt_vps.bmp
# End Source File
# Begin Source File

SOURCE=.\txt_wss.bmp
# End Source File
# Begin Source File

SOURCE=.\update.ico
# End Source File
# Begin Source File

SOURCE=.\update_bad.ico
# End Source File
# Begin Source File

SOURCE=.\update_good.ico
# End Source File
# Begin Source File

SOURCE=.\vid_dcii.bmp
# End Source File
# Begin Source File

SOURCE=.\vid_h264.bmp
# End Source File
# Begin Source File

SOURCE=.\vid_mp2.bmp
# End Source File
# Begin Source File

SOURCE=.\vid_mpg4.bmp
# End Source File
# Begin Source File

SOURCE=.\vid_vc1.bmp
# End Source File
# End Group
# Begin Group "MDI"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\MDI\mdi.c
# End Source File
# Begin Source File

SOURCE=.\MDI\mdi.h
# End Source File
# End Group
# End Target
# End Project
