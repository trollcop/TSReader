# Microsoft Developer Studio Project File - Name="ATSCSource" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=ATSCSource - Win32 QAM Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "ATSCSource.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "ATSCSource.mak" CFG="ATSCSource - Win32 QAM Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "ATSCSource - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "ATSCSource - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "ATSCSource - Win32 No Status Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "ATSCSource - Win32 QAM Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "ATSCSource - Win32 QAM AutumnWave GT" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "ATSCSource - Win32 QAM AutumnWave CR" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "ATSCSource - Win32 Release"

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
# ADD CPP /nologo /MT /W3 /GX /O2 /I "C:\SDKs\DXSDK9-Feb-2005\Include" /I "C:\SDKs\DXSDK9-Feb-2005\Samples\C++\DirectShow\BaseClasses" /I ".." /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /machine:I386
# ADD LINK32 ../TSReader_SourceHelper.lib strmbase.lib strmiids.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /machine:I386 /out:"../Sources/TSReader_ATSCBDASource.dll" /libpath:"C:\SDKs\dx9\Lib" /libpath:"c:\sdks\dx9\Samples\C++\DirectShow\BaseClasses\Debug"

!ELSEIF  "$(CFG)" == "ATSCSource - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "ATSCSource___Win32_Debug"
# PROP BASE Intermediate_Dir "ATSCSource___Win32_Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "ATSCSource___Win32_Debug"
# PROP Intermediate_Dir "ATSCSource___Win32_Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /FD /GZ /c
# ADD CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /I "C:\SDKs\dx9\Include" /I "C:\SDKs\dx9\Samples\C++\DirectShow\BaseClasses" /I ".." /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /debug /machine:I386 /pdbtype:sept
# ADD LINK32 ../TSReader_SourceHelper.lib strmbasd.lib strmiids.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /debug /machine:I386 /out:"../Sources/TSReader_ATSCBDASource.dll" /pdbtype:sept /libpath:"C:\SDKs\dx9\Lib" /libpath:"c:\sdks\dx9\Samples\C++\DirectShow\BaseClasses\Debug"

!ELSEIF  "$(CFG)" == "ATSCSource - Win32 No Status Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "ATSCSource___Win32_No_Status_Release"
# PROP BASE Intermediate_Dir "ATSCSource___Win32_No_Status_Release"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "ATSCSource___Win32_No_Status_Release"
# PROP Intermediate_Dir "ATSCSource___Win32_No_Status_Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /I "C:\SDKs\dx9\Include" /I "C:\SDKs\dx9\Samples\C++\DirectShow\BaseClasses" /I ".." /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /MT /W3 /GX /O2 /I "C:\SDKs\DXSDK9-Feb-2005\Include" /I "C:\SDKs\DXSDK9-Feb-2005\Samples\C++\DirectShow\BaseClasses" /I ".." /D "NOSTATUS" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 ../TSReader_SourceHelper.lib strmbase.lib strmiids.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /machine:I386 /out:"../Sources/TSReader_ATSCBDASource.dll" /libpath:"C:\SDKs\dx9\Lib" /libpath:"c:\sdks\dx9\Samples\C++\DirectShow\BaseClasses\Debug"
# ADD LINK32 ../TSReader_SourceHelper.lib strmbase.lib strmiids.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /machine:I386 /out:"../Sources/TSReader_ATSCBDASourceNS.dll" /libpath:"C:\SDKs\dx9\Lib" /libpath:"c:\sdks\dx9\Samples\C++\DirectShow\BaseClasses\Debug"

!ELSEIF  "$(CFG)" == "ATSCSource - Win32 QAM Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "ATSCSource___Win32_QAM_Debug"
# PROP BASE Intermediate_Dir "ATSCSource___Win32_QAM_Debug"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "ATSCSource___Win32_QAM_Debug"
# PROP Intermediate_Dir "ATSCSource___Win32_QAM_Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /I "C:\SDKs\dx9\Include" /I "C:\SDKs\dx9\Samples\C++\DirectShow\BaseClasses" /I ".." /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /FD /GZ /c
# ADD CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /I "C:\SDKs\dx9\Include" /I "C:\SDKs\dx9\Samples\C++\DirectShow\BaseClasses" /I ".." /D "QAM" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 ../TSReader_SourceHelper.lib strmbasd.lib strmiids.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /debug /machine:I386 /out:"../Sources/TSReader_ATSCBDASource.dll" /pdbtype:sept /libpath:"C:\SDKs\dx9\Lib" /libpath:"c:\sdks\dx9\Samples\C++\DirectShow\BaseClasses\Debug"
# ADD LINK32 ../TSReader_SourceHelper.lib strmbasd.lib strmiids.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /debug /machine:I386 /out:"../Sources/TSReader_AutumnWave_GT_QAM.dll" /pdbtype:sept /libpath:"C:\SDKs\dx9\Lib" /libpath:"c:\sdks\dx9\Samples\C++\DirectShow\BaseClasses\Debug"

!ELSEIF  "$(CFG)" == "ATSCSource - Win32 QAM AutumnWave GT"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "ATSCSource___Win32_QAM_AutumnWave_GT"
# PROP BASE Intermediate_Dir "ATSCSource___Win32_QAM_AutumnWave_GT"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "ATSCSource___Win32_QAM_AutumnWave_GT"
# PROP Intermediate_Dir "ATSCSource___Win32_QAM_AutumnWave_GT"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /I "C:\SDKs\dx9\Include" /I "C:\SDKs\dx9\Samples\C++\DirectShow\BaseClasses" /I ".." /D "QAM" /D "NOSTATUS" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /MT /W3 /GX /O2 /I "C:\SDKs\dx9\Include" /I "C:\SDKs\dx9\Samples\C++\DirectShow\BaseClasses" /I ".." /D "QAM" /D "NOSTATUS" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 ../TSReader_SourceHelper.lib strmbase.lib strmiids.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /machine:I386 /out:"../Sources/TSReader_AutumnWave_GT_QAM.dll" /libpath:"C:\SDKs\dx9\Lib" /libpath:"c:\sdks\dx9\Samples\C++\DirectShow\BaseClasses\Debug"
# ADD LINK32 ../TSReader_SourceHelper.lib strmbase.lib strmiids.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /machine:I386 /out:"../Sources/TSReader_AutumnWave_GT_QAM.dll" /libpath:"C:\SDKs\dx9\Lib" /libpath:"c:\sdks\dx9\Samples\C++\DirectShow\BaseClasses\Debug"

!ELSEIF  "$(CFG)" == "ATSCSource - Win32 QAM AutumnWave CR"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "ATSCSource___Win32_QAM_AutumnWave_CR"
# PROP BASE Intermediate_Dir "ATSCSource___Win32_QAM_AutumnWave_CR"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "ATSCSource___Win32_QAM_AutumnWave_CR"
# PROP Intermediate_Dir "ATSCSource___Win32_QAM_AutumnWave_CR"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /I "C:\SDKs\dx9\Include" /I "C:\SDKs\dx9\Samples\C++\DirectShow\BaseClasses" /I ".." /D "QAM" /D "NOSTATUS" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /MT /W3 /GX /O2 /I "C:\SDKs\dx9\Include" /I "C:\SDKs\dx9\Samples\C++\DirectShow\BaseClasses" /I ".." /D "QAM" /D "CREATOR" /D "NOSTATUS" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 ../TSReader_SourceHelper.lib strmbase.lib strmiids.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /machine:I386 /out:"../Sources/TSReader_AutumnWave_GT_QAM.dll" /libpath:"C:\SDKs\dx9\Lib" /libpath:"c:\sdks\dx9\Samples\C++\DirectShow\BaseClasses\Debug"
# ADD LINK32 ../TSReader_SourceHelper.lib strmbase.lib strmiids.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /machine:I386 /out:"../Sources/TSReader_AutumnWave_CR_QAM.dll" /libpath:"C:\SDKs\dx9\Lib" /libpath:"c:\sdks\dx9\Samples\C++\DirectShow\BaseClasses\Debug"

!ENDIF 

# Begin Target

# Name "ATSCSource - Win32 Release"
# Name "ATSCSource - Win32 Debug"
# Name "ATSCSource - Win32 No Status Release"
# Name "ATSCSource - Win32 QAM Debug"
# Name "ATSCSource - Win32 QAM AutumnWave GT"
# Name "ATSCSource - Win32 QAM AutumnWave CR"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\ATSCSource.cpp
# End Source File
# Begin Source File

SOURCE=.\ATSCSource.def
# End Source File
# Begin Source File

SOURCE=.\BDAFilterGraph.cpp
# End Source File
# Begin Source File

SOURCE=.\stdafx.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\ATSCSource.h
# End Source File
# Begin Source File

SOURCE=.\BDAFilterGraph.h
# End Source File
# Begin Source File

SOURCE=.\resource.h
# End Source File
# Begin Source File

SOURCE=.\stdafx.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\ATSCSource.rc
# End Source File
# End Group
# End Target
# End Project
