# Microsoft Developer Studio Project File - Name="TSReader_TTBudget" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=TSReader_TTBudget - Win32 DVBS2 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "TSReader_TTBudget.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "TSReader_TTBudget.mak" CFG="TSReader_TTBudget - Win32 DVBS2 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "TSReader_TTBudget - Win32 DVBT Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "TSReader_TTBudget - Win32 DVBT Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "TSReader_TTBudget - Win32 DVBC Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "TSReader_TTBudget - Win32 DVBC Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "TSReader_TTBudget - Win32 DVBS Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "TSReader_TTBudget - Win32 DVBS Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "TSReader_TTBudget - Win32 DVBS2 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "TSReader_TTBudget - Win32 DVBS2 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "TSReader_TTBudget - Win32 DVBT Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "TSReader_TTBudget___Win32_DVBT_Debug"
# PROP BASE Intermediate_Dir "TSReader_TTBudget___Win32_DVBT_Debug"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "TSReader_TTBudget_DVBT_Debug"
# PROP Intermediate_Dir "TSReader_TTBudget_DVBT_Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /I "..\include" /I "C:\SDKs\ImgSource" /I "c:\sdks\technotrend\API\Budget\include" /I "c:\sdks\technotrend\API\Common\include" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "DVBS" /YX /FD /GZ /c
# ADD CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /I "..\include" /I "C:\SDKs\ImgSource" /I "c:\sdks\technotrend\API\Common\include" /I "c:\sdks\technotrend\API\Budget\include" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "DVBT" /YX /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG" /d "DVBT"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 ../TSReader_SourceHelper.lib c:\sdks\technotrend\API\Budget\lib\ttlcdacc.lib /nologo /subsystem:windows /dll /debug /machine:I386 /out:"../Sources/TSReader_TTBudget.dll" /pdbtype:sept
# ADD LINK32 ../SampleSource/lib/TSReader_SourceHelper.lib ttlcdacc.lib /nologo /subsystem:windows /dll /debug /machine:I386 /out:"../Sources/TSReader_TTBudgetT.dll" /pdbtype:sept /libpath:"C:\SDKs\TechnoTrend\API\Budget\lib\\"

!ELSEIF  "$(CFG)" == "TSReader_TTBudget - Win32 DVBT Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "TSReader_TTBudget___Win32_DVBT_Release"
# PROP BASE Intermediate_Dir "TSReader_TTBudget___Win32_DVBT_Release"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "TSReader_TTBudget_DVBT_Release"
# PROP Intermediate_Dir "TSReader_TTBudget_DVBT_Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /I "..\include" /I "C:\SDKs\ImgSource" /I "c:\sdks\technotrend\API\Budget\include" /I "c:\sdks\technotrend\API\Common\include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /MT /W3 /GX /O2 /I "..\include" /I "C:\SDKs\ImgSource" /I "c:\sdks\technotrend\API\Common\include" /I "c:\sdks\technotrend\API\Budget\include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "DVBT" /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG" /d "DVBT"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 ../TSReader_SourceHelper.lib c:\sdks\technotrend\API\Budget\lib\ttlcdacc.lib /nologo /subsystem:windows /dll /machine:I386 /out:"../Sources/TSReader_TTBudgetS.dll"
# ADD LINK32 ../SampleSource/lib/TSReader_SourceHelper.lib ttlcdacc.lib /nologo /subsystem:windows /dll /machine:I386 /out:"../Sources/TSReader_TTBudgetT.dll" /libpath:"c:\sdks\technotrend\API\Budget\lib\\"

!ELSEIF  "$(CFG)" == "TSReader_TTBudget - Win32 DVBC Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "TSReader_TTBudget___Win32_DVBC_Debug"
# PROP BASE Intermediate_Dir "TSReader_TTBudget___Win32_DVBC_Debug"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "TSReader_TTBudget_DVBC_Debug"
# PROP Intermediate_Dir "TSReader_TTBudget_DVBC_Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /I "..\include" /I "C:\SDKs\ImgSource" /I "c:\sdks\technotrend\API\Budget\include" /I "c:\sdks\technotrend\API\Common\include" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "DVBS" /YX /FD /GZ /c
# ADD CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /I "..\include" /I "C:\SDKs\ImgSource" /I "c:\sdks\technotrend\API\Common\include" /I "c:\sdks\technotrend\API\Budget\include" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "DVBC" /YX /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG" /d "DVBC"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 ../TSReader_SourceHelper.lib c:\sdks\technotrend\API\Budget\lib\ttlcdacc.lib /nologo /subsystem:windows /dll /debug /machine:I386 /out:"../Sources/TSReader_TTBudgetS.dll" /pdbtype:sept
# ADD LINK32 ../SampleSource/lib/TSReader_SourceHelper.lib ttlcdacc.lib /nologo /subsystem:windows /dll /debug /machine:I386 /out:"../Sources/TSReader_TTBudgetC.dll" /pdbtype:sept /libpath:"C:\SDKs\TechnoTrend\API\Budget\lib\\"

!ELSEIF  "$(CFG)" == "TSReader_TTBudget - Win32 DVBC Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "TSReader_TTBudget___Win32_DVBC_Release"
# PROP BASE Intermediate_Dir "TSReader_TTBudget___Win32_DVBC_Release"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "TSReader_TTBudget_DVBC_Release"
# PROP Intermediate_Dir "TSReader_TTBudget_DVBC_Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /I "..\include" /I "C:\SDKs\ImgSource" /I "c:\sdks\technotrend\API\Budget\include" /I "c:\sdks\technotrend\API\Common\include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "DVBS" /YX /FD /c
# ADD CPP /nologo /MT /W3 /GX /O2 /I "..\include" /I "C:\SDKs\ImgSource" /I "c:\sdks\technotrend\API\Common\include" /I "c:\sdks\technotrend\API\Budget\include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "DVBC" /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG" /d "DVBC"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 ../TSReader_SourceHelper.lib c:\sdks\technotrend\API\Budget\lib\ttlcdacc.lib /nologo /subsystem:windows /dll /machine:I386 /out:"../Sources/TSReader_TTBudgetS.dll"
# ADD LINK32 ../SampleSource/lib/TSReader_SourceHelper.lib ttlcdacc.lib /nologo /subsystem:windows /dll /machine:I386 /out:"../Sources/TSReader_TTBudgetC.dll" /libpath:"c:\sdks\technotrend\API\Budget\lib\\"

!ELSEIF  "$(CFG)" == "TSReader_TTBudget - Win32 DVBS Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "TSReader_TTBudget___Win32_DVBS_Debug"
# PROP BASE Intermediate_Dir "TSReader_TTBudget___Win32_DVBS_Debug"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "TSReader_TTBudget_DVBS_Debug"
# PROP Intermediate_Dir "TSReader_TTBudget_DVBS_Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /I "..\include" /I "C:\SDKs\ImgSource" /I "E:\DVB-PC\API\Common\include" /I "E:\DVB-PC\API\Budget\include" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "DVBS" /YX /FD /GZ /c
# ADD CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /I "..\include" /I "C:\SDKs\ImgSource" /I "c:\sdks\technotrend\API\Common\include" /I "c:\sdks\technotrend\API\Budget\include" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "DVBS" /YX /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG" /d "DVBS"
# ADD RSC /l 0x409 /d "_DEBUG" /d "DVBS"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 ../SampleSource/lib/TSReader_SourceHelper.lib E:\DVB-PC\API\Budget\lib\ttlcdacc.lib /nologo /subsystem:windows /dll /debug /machine:I386 /out:"../Sources/TSReader_TTBudgetS.dll" /pdbtype:sept
# ADD LINK32 ../SampleSource/lib/TSReader_SourceHelper.lib ttlcdacc.lib /nologo /subsystem:windows /dll /debug /machine:I386 /out:"../Sources/TSReader_TTBudgetS.dll" /pdbtype:sept /libpath:"C:\SDKs\TechnoTrend\API\Budget\lib\\"

!ELSEIF  "$(CFG)" == "TSReader_TTBudget - Win32 DVBS Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "TSReader_TTBudget___Win32_DVBS_Release"
# PROP BASE Intermediate_Dir "TSReader_TTBudget___Win32_DVBS_Release"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "TSReader_TTBudget_DVBS_Release"
# PROP Intermediate_Dir "TSReader_TTBudget_DVBS_Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /I "..\include" /I "C:\SDKs\ImgSource" /I "E:\DVB-PC\API\Common\include" /I "E:\DVB-PC\API\Budget\include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "DVBS" /YX /FD /c
# ADD CPP /nologo /MT /W3 /GX /O2 /I "..\include" /I "C:\SDKs\ImgSource" /I "c:\sdks\technotrend\API\Common\include" /I "c:\sdks\technotrend\API\Budget\include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "DVBS" /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG" /d "DVBS"
# ADD RSC /l 0x409 /d "NDEBUG" /d "DVBS"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 ../SampleSource/lib/TSReader_SourceHelper.lib E:\DVB-PC\API\Budget\lib\ttlcdacc.lib /nologo /subsystem:windows /dll /machine:I386 /out:"../Sources/TSReader_TTBudgetS.dll"
# ADD LINK32 ../SampleSource/lib/TSReader_SourceHelper.lib ttlcdacc.lib /nologo /subsystem:windows /dll /machine:I386 /out:"../Sources/TSReader_TTBudgetS.dll" /libpath:"c:\sdks\technotrend\API\Budget\lib\\"

!ELSEIF  "$(CFG)" == "TSReader_TTBudget - Win32 DVBS2 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "TSReader_TTBudget___Win32_DVBS2_Debug"
# PROP BASE Intermediate_Dir "TSReader_TTBudget___Win32_DVBS2_Debug"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "TSReader_TTBudget___Win32_DVBS2_Debug"
# PROP Intermediate_Dir "TSReader_TTBudget___Win32_DVBS2_Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /I "..\include" /I "C:\SDKs\ImgSource" /I "c:\sdks\technotrend\API\Common\include" /I "c:\sdks\technotrend\API\Budget\include" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "DVBS" /YX /FD /GZ /c
# ADD CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /I "..\include" /I "C:\SDKs\ImgSource" /I "c:\sdks\technotrend\API\Common\include" /I "c:\sdks\technotrend\API\Budget\include" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "DVBS" /D "DVBS2" /YX /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG" /d "DVBS"
# ADD RSC /l 0x409 /d "_DEBUG" /d "DVBS"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 ../SampleSource/lib/TSReader_SourceHelper.lib ttlcdacc.lib /nologo /subsystem:windows /dll /debug /machine:I386 /out:"../Sources/TSReader_TTBudgetS.dll" /pdbtype:sept /libpath:"C:\SDKs\TechnoTrend\API\Budget\lib\\"
# ADD LINK32 ../SampleSource/lib/TSReader_SourceHelper.lib ttlcdacc.lib /nologo /subsystem:windows /dll /debug /machine:I386 /out:"../Sources/TSReader_TTBudgetS2.dll" /pdbtype:sept /libpath:"C:\SDKs\TechnoTrend\API\Budget\lib\\"

!ELSEIF  "$(CFG)" == "TSReader_TTBudget - Win32 DVBS2 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "TSReader_TTBudget___Win32_DVBS2_Release"
# PROP BASE Intermediate_Dir "TSReader_TTBudget___Win32_DVBS2_Release"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "TSReader_TTBudget___Win32_DVBS2_Release"
# PROP Intermediate_Dir "TSReader_TTBudget___Win32_DVBS2_Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /I "..\include" /I "C:\SDKs\ImgSource" /I "c:\sdks\technotrend\API\Common\include" /I "c:\sdks\technotrend\API\Budget\include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "DVBS" /YX /FD /c
# ADD CPP /nologo /MT /W3 /GX /O2 /I "..\include" /I "C:\SDKs\ImgSource" /I "c:\sdks\technotrend\API\Common\include" /I "c:\sdks\technotrend\API\Budget\include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "DVBS" /D "DVBS2" /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG" /d "DVBS"
# ADD RSC /l 0x409 /d "NDEBUG" /d "DVBS"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 ../SampleSource/lib/TSReader_SourceHelper.lib ttlcdacc.lib /nologo /subsystem:windows /dll /machine:I386 /out:"../Sources/TSReader_TTBudgetS.dll" /libpath:"c:\sdks\technotrend\API\Budget\lib\\"
# ADD LINK32 ../SampleSource/lib/TSReader_SourceHelper.lib ttlcdacc.lib /nologo /subsystem:windows /dll /machine:I386 /out:"../Sources/TSReader_TTBudgetS2.dll" /libpath:"c:\sdks\technotrend\API\Budget\lib\\"

!ENDIF 

# Begin Target

# Name "TSReader_TTBudget - Win32 DVBT Debug"
# Name "TSReader_TTBudget - Win32 DVBT Release"
# Name "TSReader_TTBudget - Win32 DVBC Debug"
# Name "TSReader_TTBudget - Win32 DVBC Release"
# Name "TSReader_TTBudget - Win32 DVBS Debug"
# Name "TSReader_TTBudget - Win32 DVBS Release"
# Name "TSReader_TTBudget - Win32 DVBS2 Debug"
# Name "TSReader_TTBudget - Win32 DVBS2 Release"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\StdAfx.cpp
# End Source File
# Begin Source File

SOURCE=.\TSReader_TTBudget.cpp
# End Source File
# Begin Source File

SOURCE=.\TSReader_TTBudget.def
# End Source File
# Begin Source File

SOURCE=.\TSReader_TTBudget.rc
# End Source File
# Begin Source File

SOURCE=.\WorkerThread.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\StdAfx.h
# End Source File
# Begin Source File

SOURCE=.\TSReader_TTBudget.h
# End Source File
# Begin Source File

SOURCE=.\WorkerThread.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# End Group
# End Target
# End Project
