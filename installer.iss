[Setup]
AppName=TSReaderPro
AppVersion=2.8.53b-memorial
AppPublisher=COOL.STF
AppPublisherURL=https://github.com/TSReader/TSReader
DefaultDirName={commonpf32}\TSReaderPro
DefaultGroupName=TSReaderPro
OutputDir=D:\Claude\TSReader\installer_output
OutputBaseFilename=TSReaderPro_Setup
Compression=lzma2
SolidCompression=yes
SetupIconFile=D:\Claude\TSReader\dvb.ico
UninstallDisplayIcon={app}\dvb.ico
ArchitecturesAllowed=x86compatible
WizardStyle=modern
LicenseFile=D:\Claude\TSReader\LICENSE
PrivilegesRequired=admin
DisableDirPage=no

[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"

[Tasks]
Name: "desktopicon"; Description: "{cm:CreateDesktopIcon}"; GroupDescription: "{cm:AdditionalIcons}"; Flags: unchecked

[Files]
; Main executable
Source: "D:\Claude\TSReader\build\Release\TSReaderPro.exe"; DestDir: "{app}"; Flags: ignoreversion
Source: "D:\Claude\TSReader\TSReaderPro.exe.manifest"; DestDir: "{app}"; DestName: "TSReaderPro.exe.manifest"; Flags: ignoreversion

; Core DLLs
Source: "D:\Claude\TSReader\build\Release\libfaad2.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "D:\Claude\TSReader\build\Release\PEGRPCS.DLL"; DestDir: "{app}"; Flags: ignoreversion
Source: "D:\Claude\TSReader\build\Release\TSReader_SourceHelper.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "D:\Claude\TSReader\_ISource.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "D:\Claude\TSReader\pthreadVSE2.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "D:\Claude\TSReader\adpsi30.dll"; DestDir: "{app}"; Flags: ignoreversion

; Application plugin DLLs
Source: "D:\Claude\TSReader\TSReader_MPEG4.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "D:\Claude\TSReader\TSReader_ForVid.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "D:\Claude\TSReader\TSReader_Scheduler.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "D:\Claude\TSReader\TSReader_VLC.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "D:\Claude\TSReader\TSReader_UDPSender.dll"; DestDir: "{app}"; Flags: ignoreversion

; Helper executables
Source: "D:\Claude\TSReader\vc1-decoder.exe"; DestDir: "{app}"; Flags: ignoreversion
Source: "D:\Claude\TSReader\TSReader_ArchiveMonitor.exe"; DestDir: "{app}"; Flags: ignoreversion

; Bitmaps (all)
Source: "D:\Claude\TSReader\*.bmp"; DestDir: "{app}"; Flags: ignoreversion

; PNG images
Source: "D:\Claude\TSReader\*.png"; DestDir: "{app}"; Flags: ignoreversion

; Audio
Source: "D:\Claude\TSReader\archive-siren.wav"; DestDir: "{app}"; Flags: ignoreversion

; INI config files (root)
Source: "D:\Claude\TSReader\BISS.ini"; DestDir: "{app}"; Flags: ignoreversion
Source: "D:\Claude\TSReader\diseqcU.ini"; DestDir: "{app}"; Flags: ignoreversion
Source: "D:\Claude\TSReader\dvbt.ini"; DestDir: "{app}"; Flags: ignoreversion
Source: "D:\Claude\TSReader\S2emu.ini"; DestDir: "{app}"; Flags: ignoreversion

; Data files
Source: "D:\Claude\TSReader\ndscam.dat"; DestDir: "{app}"; Flags: ignoreversion
Source: "D:\Claude\TSReader\dvb.ico"; DestDir: "{app}"; Flags: ignoreversion
Source: "D:\Claude\TSReader\LICENSE"; DestDir: "{app}"; DestName: "LICENSE.txt"; Flags: ignoreversion


; Memorial splash image
Source: "D:\Claude\TSReader\rod_splash.png"; DestDir: "{app}"; Flags: ignoreversion skipifsourcedoesntexist
Source: "D:\Claude\TSReader\rod_splash.jpg"; DestDir: "{app}"; Flags: ignoreversion skipifsourcedoesntexist
; List files (source presets)
Source: "D:\Claude\TSReader\*.lst"; DestDir: "{app}"; Flags: ignoreversion

; Sample .tmc files
Source: "D:\Claude\TSReader\*.tmc"; DestDir: "{app}"; Flags: ignoreversion

; FreeSat tables
Source: "D:\Claude\TSReader\freesat.t1"; DestDir: "{app}"; Flags: ignoreversion skipifsourcedoesntexist
Source: "D:\Claude\TSReader\freesat.t2"; DestDir: "{app}"; Flags: ignoreversion skipifsourcedoesntexist

; Source plugin DLLs
Source: "D:\Claude\TSReader\Sources\*.dll"; DestDir: "{app}\Sources"; Flags: ignoreversion

; Forwarder DLLs
Source: "D:\Claude\TSReader\Forwarders\*.dll"; DestDir: "{app}\Forwarders"; Flags: ignoreversion

; Satellite INI files
Source: "D:\Claude\TSReader\Satellites\*.ini"; DestDir: "{app}\Satellites"; Flags: ignoreversion

[Icons]
Name: "{group}\TSReaderPro"; Filename: "{app}\TSReaderPro.exe"; IconFilename: "{app}\dvb.ico"; WorkingDir: "{app}"
Name: "{group}\Uninstall TSReaderPro"; Filename: "{uninstallexe}"
Name: "{autodesktop}\TSReaderPro"; Filename: "{app}\TSReaderPro.exe"; IconFilename: "{app}\dvb.ico"; WorkingDir: "{app}"; Tasks: desktopicon

[Run]
Filename: "{app}\TSReaderPro.exe"; Description: "Launch TSReaderPro"; Flags: nowait postinstall skipifsilent
