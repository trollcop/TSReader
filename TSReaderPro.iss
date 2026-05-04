[Setup]
AppName=TSReader Professional
AppVersion=2.8.46b
AppVerName=TSReader Pro 2.8.46b
AppPublisher=COOL.STF
Uninstallable=1
DefaultDirName={pf}\COOL.STF\TSReaderPro
OutputBaseFilename=TSReaderPro2.8.46b
OutputDir=.
DefaultGroupName=TSReader Pro
Password=c76h8
AppPublisherURL=http://www.coolstf.com
AppUpdatesURL=http://www.coolstf.com/support
AppSupportURL=http://www.coolstf.com/support
AppCopyright=Copyright © 2003-2008 COOLSTF.com, Inc.
LicenseFile=license.txt
Encryption=Yes

[Files]
;Main part
Source: C:\Development\TSReader\TSReaderPro.exe; DestDir: {app}; Flags: ignoreversion noencryption nocompression
Source: C:\Development\TSReader\TSReader_XNS.dll; DestDir: {app}; Flags: ignoreversion
Source: C:\Development\TSReader\TSReader_DVHS.dll; DestDir: {app}; Flags: ignoreversion
Source: C:\Development\TSReader\TSReader_VLC.dll; DestDir: {app}; Flags: ignoreversion
Source: C:\Development\TSReader\TSReader_SourceHelper.dll; DestDir: {app}; Flags: ignoreversion
Source: C:\Development\TSReader\TSReader_Stradis12.dll; DestDir: {app}; Flags: ignoreversion
Source: C:\Development\TSReader\TSReader_Stradis16.dll; DestDir: {app}; Flags: ignoreversion
Source: C:\Development\TSReader\TSReader_UDPSender.dll; DestDir: {app}; Flags: ignoreversion
Source: C:\Development\TSReader\TSReader_MPEG4.dll; DestDir: {app}; Flags: ignoreversion
Source: C:\Development\TSReader\TSReader_Scheduler.dll; DestDir: {app}; Flags: ignoreversion
Source: C:\Development\TSReader\TSReader_ForVid.dll; DestDir: {app}; Flags: ignoreversion
Source: C:\Development\TSReader\TSReaderPro.exe.manifest; DestDir: {app}; Flags: ignoreversion
Source: C:\Development\TSReader\scrambled.bmp; DestDir: {app}; Flags: ignoreversion
Source: C:\Development\TSReader\videocrash.bmp; DestDir: {app}; Flags: ignoreversion
Source: C:\Development\TSReader\videocrash_small.bmp; DestDir: {app}; Flags: ignoreversion
Source: C:\Development\TSReader\vc1-decoder.exe; DestDir: {app}; Flags: ignoreversion
Source: C:\Development\TSReader\archive-siren.wav; DestDir: {app}; Flags: ignoreversion
Source: C:\Development\TSReader\TSReader_ArchiveMonitor.exe; DestDir: {app}; Flags: ignoreversion
Source: C:\Development\TSReader\pthreadVSE2.dll; DestDir: {app}; Flags: ignoreversion
Source: C:\Development\TSReader\libfaad2.dll; DestDir: {app}; Flags: ignoreversion

;Third party libraries
Source: C:\Development\TSReader\ttlcdacc.dll; DestDir: {app}; Flags: ignoreversion
Source: C:\Development\TSReader\ttusb2acc.dll; DestDir: {app}; Flags: ignoreversion
Source: C:\Development\TSReader\_ISource.dll; DestDir: {app}; Flags: ignoreversion
Source: C:\WINDOWS\system32\PEGRPCS.DLL; DestDir: {app}; Flags: ignoreversion
Source: C:\Development\TSReader\adpsi30.dll; DestDir: {app}; Flags: ignoreversion
Source: C:\Development\TSReader\ATDV_API2.dll; DestDir: {app}; Flags: ignoreversion
Source: C:\Development\TSReader\msvcr70.dll; DestDir: {sys}; Flags: onlyifdoesntexist uninsneveruninstall
Source: C:\Development\TSReader\msvcr71.dll; DestDir: {sys}; Flags: onlyifdoesntexist uninsneveruninstall
Source: C:\Development\TSReader\mfc71.dll; DestDir: {sys}; Flags: onlyifdoesntexist uninsneveruninstall
Source: C:\Development\TSReader\TSP2SP.ax; DestDir: {app}; Flags: regserver ignoreversion
Source: C:\Development\TSReader\Board1049.dll; DestDir: {app}; Flags: ignoreversion
Source: C:\Development\TSReader\Board1077.dll; DestDir: {app}; Flags: ignoreversion
Source: C:\Development\TSReader\TSReader_Teleview_TSP102\Common\Teleview\Lib\TSPLLD0421.dll; DestDir: {app}; Flags: ignoreversion
Source: C:\Development\TSReader\TSReader_Teleview_TSP102\Common\Teleview\Lib\rbf\sdc102*.rbf; DestDir: {app}\rbf; Flags: ignoreversion

;Sample Data
Source: C:\Development\TSReader\WB Galaxy 11 91 west.tmc; DestDir: {app}; Flags: ignoreversion

;Sources
Source: C:\Development\TSReader\Sources\TSReader_File.dll; DestDir: {app}\Sources; Flags: ignoreversion
Source: C:\Development\TSReader\Sources\TSReader_FileLoop.dll; DestDir: {app}\Sources; Flags: ignoreversion
Source: C:\Development\TSReader\Sources\TSReader_FileContinuous.dll; DestDir: {app}\Sources; Flags: ignoreversion
Source: C:\Development\TSReader\Sources\TSReader_Linsys.dll; DestDir: {app}\Sources; Flags: ignoreversion
Source: C:\Development\TSReader\Sources\TSReader_Linsys_SWSync.dll; DestDir: {app}\Sources; Flags: ignoreversion
Source: C:\Development\TSReader\Sources\TSReader_TTBudgetC.dll; DestDir: {app}\Sources; Flags: ignoreversion
Source: C:\Development\TSReader\Sources\TSReader_TTBudgetS.dll; DestDir: {app}\Sources; Flags: ignoreversion
Source: C:\Development\TSReader\Sources\TSReader_TTBudgetS2.dll; DestDir: {app}\Sources; Flags: ignoreversion
Source: C:\Development\TSReader\Sources\TSReader_TTBudgetT.dll; DestDir: {app}\Sources; Flags: ignoreversion
Source: C:\Development\TSReader\Sources\TSReader_TTUSB20C.dll; DestDir: {app}\Sources; Flags: ignoreversion
Source: C:\Development\TSReader\Sources\TSReader_TTUSB20S.dll; DestDir: {app}\Sources; Flags: ignoreversion
Source: C:\Development\TSReader\Sources\TSReader_TTUSB20S2.dll; DestDir: {app}\Sources; Flags: ignoreversion
Source: C:\Development\TSReader\Sources\TSReader_TTUSB20T.dll; DestDir: {app}\Sources; Flags: ignoreversion
Source: C:\Development\TSReader\Sources\TSReader_Twinhan1030.dll; DestDir: {app}\Sources; Flags: ignoreversion
Source: C:\Development\TSReader\Sources\TSReader_Twinhan1020.dll; DestDir: {app}\Sources; Flags: ignoreversion
Source: C:\Development\TSReader\Sources\TSReader_Twinhan1020dss.dll; DestDir: {app}\Sources; Flags: ignoreversion
Source: C:\Development\TSReader\Sources\TSReader_TwinhanDTT.dll; DestDir: {app}\Sources; Flags: ignoreversion
Source: C:\Development\TSReader\Sources\TSReader_TwinhanDTT-CI.dll; DestDir: {app}\Sources; Flags: ignoreversion
Source: C:\Development\TSReader\Sources\TSReader_TwinhanNoTune.dll; DestDir: {app}\Sources; Flags: ignoreversion
Source: C:\Development\TSReader\Sources\TSReader_TwinhanDCT.dll; DestDir: {app}\Sources; Flags: ignoreversion
Source: C:\Development\TSReader\Sources\TSReader_TwinhanDCT-CI.dll; DestDir: {app}\Sources; Flags: ignoreversion
Source: C:\Development\TSReader\Sources\TSReader_Twinhan8VSB.dll; DestDir: {app}\Sources; Flags: ignoreversion
Source: C:\Development\TSReader\Sources\TSReader_B2C2a.dll; DestDir: {app}\Sources; Flags: ignoreversion
Source: C:\Development\TSReader\Sources\TSReader_B2C2c.dll; DestDir: {app}\Sources; Flags: ignoreversion
Source: C:\Development\TSReader\Sources\TSReader_B2C2q.dll; DestDir: {app}\Sources; Flags: ignoreversion
Source: C:\Development\TSReader\Sources\TSReader_B2C2s.dll; DestDir: {app}\Sources; Flags: ignoreversion
Source: C:\Development\TSReader\Sources\TSReader_B2C2t.dll; DestDir: {app}\Sources; Flags: ignoreversion
Source: C:\Development\TSReader\Sources\TSReader_B2C2_Dump.ax; DestDir: {app}\Sources; Flags: regserver ignoreversion
Source: C:\Development\TSReader\Sources\TSReader_Nebula.dll; DestDir: {app}\Sources; Flags: ignoreversion
Source: C:\Development\TSReader\Sources\TSReader_DTVWorks_8PSK.dll; DestDir: {app}\Sources; Flags: ignoreversion
Source: C:\Development\TSReader\Sources\TSReader_DTVWorks_8VSB.dll; DestDir: {app}\Sources; Flags: ignoreversion
Source: C:\Development\TSReader\Sources\TSReader_DTVWorks_DSS.dll; DestDir: {app}\Sources; Flags: ignoreversion
Source: C:\Development\TSReader\Sources\TSReader_DTVWorks_DVBS.dll; DestDir: {app}\Sources; Flags: ignoreversion
Source: C:\Development\TSReader\Sources\TSReader_DTVWorks_QAM.dll; DestDir: {app}\Sources; Flags: ignoreversion
Source: C:\Development\TSReader\Sources\TSReader_DTVWorks_SPI.dll; DestDir: {app}\Sources; Flags: ignoreversion
Source: C:\Development\TSReader\Sources\TSReader_Dektec.dll; DestDir: {app}\Sources; Flags: ignoreversion
Source: C:\Development\TSReader\Sources\TSReader_DektecUSB.dll; DestDir: {app}\Sources; Flags: ignoreversion
Source: C:\Development\TSReader\Sources\TSReader_UDPMulticast.dll; DestDir: {app}\Sources; Flags: ignoreversion
Source: C:\Development\TSReader\Sources\TSReader_UDPUnicast.dll; DestDir: {app}\Sources; Flags: ignoreversion
Source: C:\Development\TSReader\Sources\TSReader_RTPMulticast.dll; DestDir: {app}\Sources; Flags: ignoreversion
Source: C:\Development\TSReader\Sources\TSReader_RTPUnicast.dll; DestDir: {app}\Sources; Flags: ignoreversion
Source: C:\Development\TSReader\Sources\TSReader_TCP.dll; DestDir: {app}\Sources; Flags: ignoreversion
Source: C:\Development\TSReader\Sources\TSReader_SasemUSB_8VSB.dll; DestDir: {app}\Sources; Flags: ignoreversion
Source: C:\Development\TSReader\Sources\TSReader_SasemUSB_QAM.dll; DestDir: {app}\Sources; Flags: ignoreversion
Source: C:\Development\TSReader\Sources\TSReader_Broadlogic.dll; DestDir: {app}\Sources; Flags: ignoreversion
Source: C:\Development\TSReader\Sources\TSReader_PinnacleSatCI.dll; DestDir: {app}\Sources; Flags: ignoreversion
Source: C:\Development\TSReader\Sources\TSReader_HCWNovaUSB2T.dll; DestDir: {app}\Sources; Flags: ignoreversion
Source: C:\Development\TSReader\Sources\TSReader_HCWPCI90002.dll; DestDir: {app}\Sources; Flags: ignoreversion
Source: C:\Development\TSReader\Sources\TSReader_HCWPCI92xxx.dll; DestDir: {app}\Sources; Flags: ignoreversion
Source: C:\Development\TSReader\Sources\TSReader_HCWHVR900.dll; DestDir: {app}\Sources; Flags: ignoreversion
Source: C:\Development\TSReader\Sources\TSReader_HCWUSB70xxx.dll; DestDir: {app}\Sources; Flags: ignoreversion
Source: C:\Development\TSReader\Sources\TSReader_HCWHVR930.dll; DestDir: {app}\Sources; Flags: ignoreversion
Source: C:\Development\TSReader\Sources\TSReader_HCWPCI67xxx.dll; DestDir: {app}\Sources; Flags: ignoreversion
Source: C:\Development\TSReader\Sources\TSReader_HCWHVR950.dll; DestDir: {app}\Sources; Flags: ignoreversion
Source: C:\Development\TSReader\Sources\TSReader_TerratecT2.dll; DestDir: {app}\Sources; Flags: ignoreversion
Source: C:\Development\TSReader\Sources\TSReader_Firewire.dll; DestDir: {app}\Sources; Flags: ignoreversion
Source: C:\Development\TSReader\Sources\TSReader_Firewire_DVHS.dll; DestDir: {app}\Sources; Flags: ignoreversion
Source: C:\Development\TSReader\Sources\TSReader_ATSCBDASource.dll; DestDir: {app}\Sources; Flags: ignoreversion
Source: C:\Development\TSReader\Sources\TSReader_ATSCBDASourceNS.dll; DestDir: {app}\Sources; Flags: ignoreversion
Source: C:\Development\TSReader\Sources\TSReader_TinyUSB2.dll; DestDir: {app}\Sources; Flags: ignoreversion
Source: C:\Development\TSReader\Sources\TSReader_TongshiS.dll; DestDir: {app}\Sources; Flags: ignoreversion
Source: C:\Development\TSReader\Sources\TSReader_DVBWorld_S2101.dll; DestDir: {app}\Sources; Flags: ignoreversion
Source: C:\Development\TSReader\Sources\TSReader_DVBWorld_S2101_DSS.dll; DestDir: {app}\Sources; Flags: ignoreversion
Source: C:\Development\TSReader\Sources\TSReader_Nextorm_S2101.dll; DestDir: {app}\Sources; Flags: ignoreversion
Source: C:\Development\TSReader\Sources\TSReader_Nextorm_S2101_DSS.dll; DestDir: {app}\Sources; Flags: ignoreversion
Source: C:\Development\TSReader\Sources\TSReader_Alitronika2345.dll; DestDir: {app}\Sources; Flags: ignoreversion
Source: C:\Development\TSReader\Sources\TSReader_Alitronika600.dll; DestDir: {app}\Sources; Flags: ignoreversion
Source: C:\Development\TSReader\Sources\TSReader_Alitronika700.dll; DestDir: {app}\Sources; Flags: ignoreversion
Source: C:\Development\TSReader\Sources\TSReader_Alitronika800.dll; DestDir: {app}\Sources; Flags: ignoreversion
Source: C:\Development\TSReader\Sources\TSReader_Alitronika720a.dll; DestDir: {app}\Sources; Flags: ignoreversion
Source: C:\Development\TSReader\Sources\TSReader_Alitronika720q.dll; DestDir: {app}\Sources; Flags: ignoreversion
Source: C:\Development\TSReader\Sources\TSReader_DIBCOM.dll; DestDir: {app}\Sources; Flags: ignoreversion
Source: C:\Development\TSReader\Sources\TSReader_HRTPMulticast.dll; DestDir: {app}\Sources; Flags: ignoreversion
Source: C:\Development\TSReader\Sources\TSReader_HRTPUnicast.dll; DestDir: {app}\Sources; Flags: ignoreversion
Source: C:\Development\TSReader\Sources\TSReader_HRTPUnicast.dll; DestDir: {app}\Sources; Flags: ignoreversion
Source: C:\Development\TSReader\Sources\TSReader_SencoreDTU234_8VSB.dll; DestDir: {app}\Sources; Flags: ignoreversion
Source: C:\Development\TSReader\Sources\TSReader_SencoreDTU234_QAM.dll; DestDir: {app}\Sources; Flags: ignoreversion
Source: C:\Development\TSReader\Sources\TSReader_AutumnWave_8VSB.dll; DestDir: {app}\Sources; Flags: ignoreversion
Source: C:\Development\TSReader\Sources\TSReader_AutumnWave_QAM.dll; DestDir: {app}\Sources; Flags: ignoreversion
Source: C:\Development\TSReader\Sources\TSReader_Teleview_TSP102_310M.dll; DestDir: {app}\Sources; Flags: ignoreversion
Source: C:\Development\TSReader\Sources\TSReader_Teleview_TSP102_ASI.dll; DestDir: {app}\Sources; Flags: ignoreversion
Source: C:\Development\TSReader\Sources\TSReader_RFCentral_RFX-MDR-PC.dll; DestDir: {app}\Sources; Flags: ignoreversion
Source: C:\Development\TSReader\Sources\TSReader_HTTP.dll; DestDir: {app}\Sources; Flags: ignoreversion
Source: C:\Development\TSReader\Sources\TSReader_DVBTBDASource.dll; DestDir: {app}\Sources; Flags: ignoreversion
Source: C:\Development\TSReader\Sources\TSReader_AutumnWave_GT_QAM.dll; DestDir: {app}\Sources; Flags: ignoreversion
Source: C:\Development\TSReader\Sources\TSReader_AutumnWave_CR_QAM.dll; DestDir: {app}\Sources; Flags: ignoreversion
Source: C:\Development\TSReader\Sources\TSReader_HDHomeRun_8VSB.dll; DestDir: {app}\Sources; Flags: ignoreversion
Source: C:\Development\TSReader\Sources\TSReader_HDHomeRun_QAM.dll; DestDir: {app}\Sources; Flags: ignoreversion
Source: C:\Development\TSReader\Sources\TSReader_HCWHVR1600.dll; DestDir: {app}\Sources; Flags: ignoreversion
Source: C:\Development\TSReader\Sources\TSReader_HCWHVR1600qam.dll; DestDir: {app}\Sources; Flags: ignoreversion
Source: C:\Development\TSReader\Sources\TSReader_DVBSBDASource.dll; DestDir: {app}\Sources; Flags: ignoreversion
Source: C:\Development\TSReader\Sources\TSReader_HCWQAMSource.dll; DestDir: {app}\Sources; Flags: ignoreversion
Source: C:\Development\TSReader\Sources\TSReader_HCWPCI99xxx.dll; DestDir: {app}\Sources; Flags: ignoreversion
Source: C:\Development\TSReader\Sources\TSReader_SencoreDTU235.dll; DestDir: {app}\Sources; Flags: ignoreversion

;Forwarder modules
Source: C:\Development\TSReader\Forwarders\TSReader_Fwd_Alitronika.dll; DestDir: {app}\Forwarders; Flags: ignoreversion
Source: C:\Development\TSReader\Forwarders\TSReader_Fwd_Dektec.dll; DestDir: {app}\Forwarders; Flags: ignoreversion
Source: C:\Development\TSReader\Forwarders\TSReader_Fwd_Linear.dll; DestDir: {app}\Forwarders; Flags: ignoreversion
Source: C:\Development\TSReader\Forwarders\TSReader_Fwd_NDSMOD.dll; DestDir: {app}\Forwarders; Flags: ignoreversion

;Serial receiver control modules
Source: C:\Development\TSReader\Sources\ReceiverControl\*.dll; DestDir: {app}\Sources\ReceiverControl; Flags: ignoreversion
Source: C:\Development\TSReader\Sources\ReceiverControl\AlteiaPlus\*.c; DestDir: {app}\Sources\ReceiverControl\AlteiaPlus; Flags: ignoreversion
Source: C:\Development\TSReader\Sources\ReceiverControl\AlteiaPlus\*.def; DestDir: {app}\Sources\ReceiverControl\AlteiaPlus; Flags: ignoreversion
Source: C:\Development\TSReader\Sources\ReceiverControl\AlteiaPlus\*.ds?; DestDir: {app}\Sources\ReceiverControl\AlteiaPlus; Flags: ignoreversion
Source: C:\Development\TSReader\Sources\ReceiverControl\DSR4800\*.c; DestDir: {app}\Sources\ReceiverControl\DSR4800; Flags: ignoreversion
Source: C:\Development\TSReader\Sources\ReceiverControl\DSR4800\*.def; DestDir: {app}\Sources\ReceiverControl\DSR4800; Flags: ignoreversion
Source: C:\Development\TSReader\Sources\ReceiverControl\DSR4800\*.ds?; DestDir: {app}\Sources\ReceiverControl\DSR4800; Flags: ignoreversion
Source: C:\Development\TSReader\Sources\ReceiverControl\Newtec2063\*.c; DestDir: {app}\Sources\ReceiverControl\Newtec2063; Flags: ignoreversion
Source: C:\Development\TSReader\Sources\ReceiverControl\Newtec2063\*.def; DestDir: {app}\Sources\ReceiverControl\Newtec2063; Flags: ignoreversion
Source: C:\Development\TSReader\Sources\ReceiverControl\Newtec2063\*.ds?; DestDir: {app}\Sources\ReceiverControl\Newtec2063; Flags: ignoreversion

;INI files for satellite
Source: C:\Development\TSReader\Satellites\*.ini; DestDir: {app}\Satellites; Flags: comparetimestamp

;Sample code
Source: C:\Development\TSReader\MDSampleSource.zip; DestDir: {app}\MDPlugins; Flags: ignoreversion
Source: C:\Development\TSReader\MDSampleIP\MDSampleIP.zip; DestDir: {app}\MDPlugins; Flags: ignoreversion
Source: C:\Development\TSReader\SampleSource\SampleSource.c; DestDir: {app}\SampleSource; Flags: ignoreversion
Source: C:\Development\TSReader\SampleSource\SampleSource.def; DestDir: {app}\SampleSource; Flags: ignoreversion
Source: C:\Development\TSReader\SampleSource\SampleSource.dsp; DestDir: {app}\SampleSource; Flags: ignoreversion
Source: C:\Development\TSReader\SampleSource\SampleSource.dsw; DestDir: {app}\SampleSource; Flags: ignoreversion
Source: C:\Development\TSReader\SampleSource\SampleSource.ncb; DestDir: {app}\SampleSource; Flags: ignoreversion
Source: C:\Development\TSReader\SampleSource\SampleSource.opt; DestDir: {app}\SampleSource; Flags: ignoreversion
Source: C:\Development\TSReader\TSReader_SourceHelper.lib; DestDir: {app}\SampleSource\Lib; Flags: ignoreversion
Source: C:\Development\TSReader\sources.h; DestDir: {app}\SampleSource\Inc; Flags: ignoreversion
Source: C:\Development\TSReader\BDASource\BDASource.zip; DestDir: {app}\SampleSource; Flags: ignoreversion
Source: C:\Development\TSReader\ATSCSource\DCattley_TSReader_ATSCBDASource.zip; DestDir: {app}\SampleSource; Flags: ignoreversion
Source: C:\Development\TSReader\TSReader_Teleview_TSP102\TSReader_Teleview_TSP102.zip; DestDir: {app}\SampleSource; Flags: ignoreversion

;Documentation
Source: C:\coolstf\tsreader\*.*; DestDir: {app}\Documentation; Flags: ignoreversion

;TechnoTrend drivers
Source: C:\Development\TSReader\TechnoTrendDrivers\budget_pci_driver\*.*; DestDir: {app}\Drivers\TechnoTrend\budget_pci; Flags: ignoreversion
Source: C:\Development\TSReader\TechnoTrendDrivers\usb2_driver\*.*; DestDir: {app}\Drivers\TechnoTrend\usb2_driver; Flags: ignoreversion
Source: C:\Development\TSReader\TechnoTrendDrivers\usb2_driver\DrvSetup\Win98\*.*; DestDir: {app}\Drivers\TechnoTrend\usb2_driver\DrvSetup\Win98; Flags: ignoreversion
Source: C:\Development\TSReader\TechnoTrendDrivers\usb2_driver\DrvSetup\Win2000\*.*; DestDir: {app}\Drivers\TechnoTrend\usb2_driver\DrvSetup\Win2000; Flags: ignoreversion

;DTVWorks drivers
Source: C:\Development\TSReader\DTVWorksDrivers\DTVWorks.sys; DestDir: {app}\Drivers\DTVWorks; Flags: ignoreversion
Source: C:\Development\TSReader\DTVWorksDrivers\DTVWorks.inf; DestDir: {app}\Drivers\DTVWorks; Flags: ignoreversion
Source: C:\Development\TSReader\USBProgrammer\USBProgrammer.exe; DestDir: {app}\Drivers\DTVWorks; Flags: ignoreversion

;Dektec drivers
Source: C:\Development\TSReader\DTU2xxDrivers\Dtu2xx Installation.pdf; DestDir: {app}\Drivers\Dektec; Flags: ignoreversion
Source: C:\Development\TSReader\DTU2xxDrivers\Dtu2xx SetUp.exe; DestDir: {app}\Drivers\Dektec; Flags: ignoreversion

;HCW drivers
Source: C:\Development\TSReader\HCW Drivers\install\*.*; DestDir: {app}\Drivers\HCW Drivers\install; Flags: ignoreversion
Source: C:\Development\TSReader\HCW Drivers\install\Driver18\*.*; DestDir: {app}\Drivers\HCW Drivers\install\Driver18; Flags: ignoreversion
Source: C:\Development\TSReader\HCW Drivers\install64\*.*; DestDir: {app}\Drivers\HCW Drivers\install64; Flags: ignoreversion
Source: C:\Development\TSReader\HCW Drivers\install64\Driver18\*.*; DestDir: {app}\Drivers\HCW Drivers\install64\Driver18; Flags: ignoreversion
Source: C:\Development\TSReader\HCW Drivers\install64\Driver18\64bit\*.*; DestDir: {app}\Drivers\HCW Drivers\install64\Driver18\64bit; Flags: ignoreversion

;myHTPC plugings
Source: C:\Development\TSReader\myHTPC\myHTPC_EpgTSReaderPlugin.wsc; DestDir: {app}\myHTPC; Flags: ignoreversion
Source: C:\Development\TSReader\myHTPC\myHTPC_EpgTSReaderPlugin_Config.vbs; DestDir: {app}\myHTPC; Flags: ignoreversion

[Icons]
Name: {group}\TSReader Professional; Filename: {app}\TSReaderPro.exe; Parameters: "-L "; WorkingDir: {app}; IconFilename: ; IconIndex: 0; Flags: createonlyiffileexists
Name: {group}\Read Me File; Filename: {app}\Documentation\readme.html; Parameters: ; WorkingDir: {app}\Documentation; IconFilename: ; IconIndex: 0; Flags: createonlyiffileexists
Name: {group}\Support Information; Filename: {app}\Documentation\support.html; Parameters: ; WorkingDir: {app}\Documentation; IconFilename: ; IconIndex: 0; Flags: createonlyiffileexists
Name: {group}\myHTPC and TSReader; Filename: {app}\Documentation\myHTPC-TSReader.html; Parameters: ; WorkingDir: {app}\Documentation; IconFilename: ; IconIndex: 0; Flags: createonlyiffileexists
Name: {group}\Control Server Documentation; Filename: {app}\Documentation\control-server.html; WorkingDir: {app}\Documentation; Flags: createonlyiffileexists
Name: {group}\Command-Line Interface; Filename: {app}\Documentation\command-line.html; WorkingDir: {app}\Documentation; Flags: createonlyiffileexists
Name: {group}\XML Export Documentation; Filename: {app}\Documentation\text-export.html; WorkingDir: {app}\Documentation; Flags: createonlyiffileexists
Name: {group}\Uninstall TSReader; Filename: {uninstallexe}

[InstallDelete]
Type: files; Name: "{app}\Sources\TSReader_FutureTel.dll"

[Run]
Filename: {app}\Documentation\readme.html; Description: View the README file; Flags: shellexec skipifdoesntexist postinstall skipifsilent
Filename: {app}\TSReaderPro.exe; Parameters: "-L "; Description: Launch application; Flags: postinstall nowait skipifsilent
