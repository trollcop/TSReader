unit MDPluginAPI;
{**********************************************************
  Multidec API translator.
  Author: SatRookie with great help from Symbiose

  Important:
    This is OpenSource project. You can not use this
    source or any idea You got from this source in
    'not-publicly available' or in 'ClosedSource' software!

  Revision history:
    Ver 1.01 October 24, 2002
      for actual version of DVBCore

    Ver 1.0 October 12, 2002
      Now it's separate module

    Ver 0.9 October 03, 2002
      Initial release


***********************************************************}
interface

uses
  Windows, Messages, SysUtils, Variants, Classes, Graphics, Controls, Forms,
  Dialogs, Menus, StdCtrls, IniFiles;


const
  MDAPI_VERSION_FOR_PLUGINS='MD-API Version 01.02 Root 254376';
//  PATH_TO_PLUGINS='.\Plugins\';  //Edited By Symbiose

// MD 8.2 Commands
  MDAPI_GET_TRANSPONDER       = $01020000;
  MDAPI_SET_TRANSPONDER       = $01020001;
  MDAPI_GET_PROGRAMM          = $01020010;
  MDAPI_SET_PROGRAMM          = $01020011;
  MDAPI_RESCAN_PROGRAMM       = $01020012;
  MDAPI_SAVE_PROGRAMM         = $01020013;
  MDAPI_GET_PROGRAMM_NUMMER   = $01020014;
  MDAPI_SET_PROGRAMM_NUMMER   = $01020015;
  MDAPI_START_FILTER          = $01020020;
  MDAPI_STOP_FILTER           = $01020021;
  MDAPI_SCAN_CURRENT_TP       = $01020030;
  MDAPI_SCAN_CURRENT_CAT      = $01020031;
  MDAPI_START_OSD             = $01020040;
  MDAPI_OSD_DRAWBLOCK         = $01020041;
  MDAPI_OSD_SETFONT           = $01020042;
  MDAPI_OSD_TEXT              = $01020043;
  MDAPI_SEND_OSD_KEY          = $01020044;
  MDAPI_STOP_OSD              = $01020049;
  MDAPI_DVB_COMMAND           = $01020060;
  MDAPI_GET_VERSION           = $01020100;

// ProgDVB commands
  PROGAPI_GET_SIGNAL_LEVEL    = $01022001;
  PROGAPI_SEND_DISEQC         = $01022002;
  PROGAPI_GET_CHANNEL_NAME    = $01022003;
  PROGAPI_SET_CHANNEL         = $01022004;
  PROGAPI_SET_TRANSPONDER     = $01022005;

// MD API 8.2
  MAX_CA_SYSTEMS=32;
  MAX_PID_IDS=32;

type
// MD API DataTypes
  TProgramNumberParam = record
    RealNumber: Integer;
    VirtNumber: Integer;
  end;
  PProgramNumberParam=^TProgramNumberParam;

  TStartFilterParam = record
    DLL_ID: Word;
    Filter_ID: Word;
    Pid: Word;
    Name: Array[0..32-1] of Byte;
    Irq_Call_Adresse: LongInt;
    Running_ID: Integer;
  end;
  PStartFilterParam=^TStartFilterParam;

  TTPCat = record
    TAG: Byte;
    DesLen: Byte;
    CA_ID: Word;
    EMM: Word;
    BufferLen: Word;
    Buffer: Array[0..64-1] of Byte;
  end;

  TTPCatio = record
    TPCatAnzahl: Integer;
    TPCat: Array[0..8-1] of TTPCAT;
  end;
  PCAT=^TTPCatio;

  TPIDFilters = record
    FilterName: Array[0..5-1] of Char;
    FilterId: Byte;
    PID: Word;
  end;

  TCA_System82 = record
    CA_Typ:Word;
    ECM:Word;
    EMM:Word;
    Provider_Id:DWord;
  end;

  TProgramm82 = record
    Name: Array[00..29] of char;
    Anbieter:Array[00..29] of char;
    Land:Array[00..29] of char;
    freq: DWord;
    Typ: Byte;
    volt: Byte;
    afc: Byte;
    diseqc: Byte;
    srate:Word;
    qam: Word;
    fec: Word;
    norm: Byte;
    tp_id: Word;
    Video_pid: Word;
    Audio_pid: Word;
    TeleText_pid:Word;
    PMT_pid:Word;
    PCR_pid:Word;
    ECM_PID:Word;
    SID_pid:Word;
    AC3_pid:Word;
    TVType: Byte;
    ServiceTyp: Byte;
    CA_ID: Byte;
    Temp_Audio:Word;
    Filteranzahl:Word;
    Filters: Array[00..(MAX_PID_IDS-1)] of TPIDFilters;
    CA_Anzahl:Word;
    CA_System82:array[0..(MAX_CA_SYSTEMS-1)] of TCA_System82;
    CA_Land:Array[0..5] of char;
    Merker:Byte;
    Link_TP:Word;
    Link_SID:Word;
    Dynamisch:Byte;
    Extern_Buffer:array[00..15] of char ;
  end;
  PProgramm82=^TProgramm82;

  TDVB_COMMAND = record
    Cmd_laenge: Word;
    Cmd_Buffer: Array[0..32-1] of Byte;
  end;
  PDVB_COMMAND=^TDVB_COMMAND;

// MD API functions
  TMDAPIGetPluginName = procedure(Buf: PByteArray); cdecl;
  TMDAPIProcessPluginMenuCommandProc = procedure( MenuID:integer );cdecl;
  TMDAPIProcessChannelChange=procedure(prg: TProgramm82); cdecl;
  TMDAPIInitPluginProc = procedure(MDInstance: LongWord; MDWnd: HWND;
        Log_Set: Bool; DLL_ID: Integer; HotKey: PChar;  Vers: PChar;
        var ReturnValue: Integer); cdecl;
  TMDAPIExitPluginProc = procedure(MDInstance: LongWord; MDWnd: HWND;
        Log_Set: Bool); cdecl;
  TMDAPIProcessHotKeyProc=procedure; cdecl;
  TMDAPIFilterProc=procedure( hFilter: DWORD; Len: Cardinal; Buf: PByteArray); cdecl;
  TMDAPIFilterCloseProc = procedure(MDInstance: LongWord); cdecl;
  TMDAPIProcessRecPlayProc = procedure(Mode:Integer); cdecl;
// ProgDVB types
  TProgAPIGetChannelName=record
    dwSize:DWORD;
    dwSID:DWORD;
    dwTID:DWORD; // in (if =0 so curretnt TID)
    dwSatPosition:DWORD ; // in (if =0 so current SatPosition)
    ChannelName:array [0..32-1] of byte; //out
    ProviderName:array [0..32-1] of byte; //out
    ChannelCaption: array [0..128-1] of byte; //out
  end;
  PProgAPIGetChannelName=^TProgAPIGetChannelName;

// Viewer wrapper functions
  TWrapperPreprocessMessage=function(var msg : TMessage):DWORD;stdcall;
  TWrapperSetFilter=function(wPID: WORD; FilterProc:TMDAPIFilterProc; Name:String): DWORD; stdcall;
  TWrapperStopFilter=function(wHandle:DWORD): DWORD; stdcall;
  TWrapperSetCSAKeys=procedure(Command:array of byte);

// Internal types
  TMDPluginWorkspace = class(TForm)
    MainMenu: TMainMenu;
    OptionsMenu: TMenuItem;
    PluginLog: TListBox;
    LogAllActions: TMenuItem;
    Acttions1: TMenuItem;
    LogLoadPlugin: TMenuItem;
    LogDetectPlugin: TMenuItem;
    LogCloseFilter: TMenuItem;
    LogFilterAction: TMenuItem;
    LogFilterCallback: TMenuItem;
    LogGetVersion: TMenuItem;
    LogGetProgramNumber: TMenuItem;
    LogSetProgramNumber: TMenuItem;
    LogGetProgram: TMenuItem;
    LogSetProgram: TMenuItem;
    LogSaveProgram: TMenuItem;
    LogScanCAT: TMenuItem;
    LogStartFilter: TMenuItem;
    LogStopFilter: TMenuItem;
    LogStartOSD: TMenuItem;
    LogOSDDrawBlock: TMenuItem;
    LogOSDSetFont: TMenuItem;
    LogOSDText: TMenuItem;
    LogSendOSDKey: TMenuItem;
    LogStopOSD: TMenuItem;
    LogDVBCommand: TMenuItem;
    MessageBoxOnError: TMenuItem;
    LogPROGAPIGetChannelName: TMenuItem;
    ChannelInfoOnlyFromCache: TMenuItem;
    procedure FormCreate(Sender: TObject);
    procedure FormDestroy(Sender: TObject);
    procedure FormCloseQuery(Sender: TObject; var CanClose: Boolean);
    procedure PluginLogKeyPress(Sender: TObject; var Key: Char);
  private
    { Private declarations }
  public
    { Public declarations }
    ini:TIniFile;
    procedure InitMDPlugins;
//    procedure ChangeChannel;
    procedure RecPlay(mode:Integer);
//    procedure LogAction(s:string;IsError:boolean=False);
    procedure WMCommand(var msg: TWMCommand); message WM_COMMAND;
    procedure WMUser(var msg : TMessage); message WM_USER;

  end;

var
  MDPluginWorkspace: TMDPluginWorkspace;


  WrapperPreprocessMessage:TWrapperPreprocessMessage;
  WrapperSetFilter:TWrapperSetFilter;
  WrapperStopFilter:TWrapperStopFilter;
  WrapperSetCSAKeys:TWrapperSetCSAKeys;

  LastProgram:TProgramm82;
  Initialized:boolean=False;
  // Plugin API version
  MDAPIVersion:array[0..62] of byte;
  // You can change first bytes for different Firmware types
  SAA_COMMAND: array [00..13] of Byte=($10,4,5,0,0,0,0,0,0,0,0,0,0,0);

procedure WrapperOnChannelChange(prg:TProgramm82);

implementation

{$R *.dfm}

const
  MaxPluginCount=100;
  MaxFilterCount=100;

type
  TMDPlugin=record
    Name:string;
    HotKey:char;
    DLLID:Cardinal;
    PluginMenu:HMENU;
    //On_Start
    InitPlugin:TMDAPIInitPluginProc;
    //On_Send_Dll_ID_Name
    GetPluginName:TMDAPIGetPluginName;
    //On_Menu_Select
    ProcessPluginMenuCommand:TMDAPIProcessPluginMenuCommandProc;
    //On_Channel_Change
    ProcessChannelChange:TMDAPIProcessChannelChange;
    //On_Exit
    ExitMDPlugin:TMDAPIExitPluginProc;
    //On_Hot_Key
    ProcessHotKey:TMDAPIProcessHotKeyProc;
    //On_Filter_Close
    ProcessFilterClose:TMDAPIFilterCloseProc;
    //On_Rec_Play
    ProcessRecPlay:TMDAPIProcessRecPlayProc;
    //On_Osd_Key
    //On_VideoText_Stream
  end;

  TMDFilter=record
    Active:boolean;
    Handle:DWORD;
    Proc:TMDAPIFilterProc;
    Name:string;
  end;

var
  // Plugin information
  PluginCount:integer=0;
  MDPlugins: array [1..MaxPluginCount] of TMDPlugin;

procedure get_name(str, OutArray: PByteArray; Len: integer);
var
    i: integer;
begin
    for i:=0 to Len-1 do OutArray[i]:=0;
    StrCopy(pchar(OutArray),pchar(copy(pchar(str),1,Len)));
end;

procedure LogAction(s:string;IsError:boolean=False);
const
  MAX_LOG_LINES=1000;
begin
  if NOT Initialized then Exit;
    if IsError then
      if MDPluginWorkspace.MessageBoxOnError.Checked then
        ShowMessage(s);
    if MDPluginWorkspace.LogAllActions.Checked then
      MDPluginWorkspace.PluginLog.Items.Insert(0,s);
    if MDPluginWorkspace.PluginLog.Items.Count>MAX_LOG_LINES then
      MDPluginWorkspace.PluginLog.Items.Delete(MAX_LOG_LINES);
end;

// Plugin Loading & Initialization
procedure TMDPluginWorkspace.InitMDPlugins;
var sr: TSearchRec;
    buf:array [0..511] of char;
    i:integer;
    Res:integer;
    WriteLog:BOOL;
    Path_To_Plugins:string;
begin
  if NOT Initialized then
  begin
    Initialized:=True;

    StrPCopy(PChar(@MDAPIVersion[0]), MDAPI_VERSION_FOR_PLUGINS);

    for i:=0 to 1 do
    begin
      if i=0 then
        Path_To_Plugins:=extractfilepath(application.ExeName)+'MDPlugins\'
      else
        Path_To_Plugins:=extractfilepath(application.ExeName)+''; //for compatibility

      if FindFirst(PATH_TO_PLUGINS+'*.dll', faAnyFile, sr) = 0 then
      begin
        repeat
          MDPlugins[PluginCount].DLLID:=LoadLibrary(PChar(PATH_TO_PLUGINS+sr.Name));
          if MDPlugins[PluginCount].DLLID<>0 then
          begin
            if LogLoadPlugin.Checked then
              LogAction('Loaded: '+PChar(PATH_TO_PLUGINS+sr.Name));

            MDPlugins[PluginCount].GetPluginName:=
               GetProcAddress(MDPlugins[PluginCount].DLLID,'On_Send_Dll_ID_Name');
            if Integer(@MDPlugins[PluginCount].GetPluginName)>0 then
            begin
              MDPlugins[PluginCount].GetPluginName(@buf[0]);
              if LogDetectPlugin.Checked then
                LogAction('Detected plugin: '+PChar(@buf[0]));

              MDPlugins[PluginCount].ProcessPluginMenuCommand:=
                  GetProcAddress(MDPlugins[PluginCount].DLLID,'On_Menu_Select');
              MDPlugins[PluginCount].ProcessChannelChange:=
                  GetProcAddress(MDPlugins[PluginCount].DLLID,'On_Channel_Change');
              MDPlugins[PluginCount].InitPlugin:=
                  GetProcAddress(MDPlugins[PluginCount].DLLID,'On_Start');
              MDPlugins[PluginCount].ExitMDPlugin:=
                  GetProcAddress(MDPlugins[PluginCount].DLLID,'On_Exit');
              MDPlugins[PluginCount].ProcessHotKey:=
                  GetProcAddress(MDPlugins[PluginCount].DLLID,'On_Hot_Key');
              MDPlugins[PluginCount].ProcessRecPlay:=
                  GetProcAddress(MDPlugins[PluginCount].DLLID,'On_Rec_Play');

              MDPlugins[PluginCount].PluginMenu:=
                  LoadMenu(MDPlugins[PluginCount].DLLID,'EXTERN');
              InsertMenu(Menu.Handle, $FFFFFFFF,
                  (MF_BYPOSITION or MF_POPUP or MF_ENABLED),
                  MDPlugins[PluginCount].PluginMenu,PChar(@buf[0]));
              PluginCount:=PluginCount+1;
            end
            else
              if LogDetectPlugin.Checked then
                LogAction('Plugin detection failed!');
              FreeLibrary(MDPlugins[PluginCount].DLLID)
          end;
        until FindNext(sr) <> 0;
        FindClose(sr);
      end;
    end;
    for i:=0 to PluginCount-1 do
    begin
      WriteLog:=True; // not used now 8(
      Res:=1; // not used now 8(
      MDPlugins[i].InitPlugin(HInstance,Handle,WriteLog,i,@(MDPlugins[i].HotKey),@MDAPIVersion[0],Res);
    end;
  end;
end;


procedure TMDPluginWorkspace.RecPlay(mode:Integer);
var i:integer;
begin
  for i:=0 to PluginCount-1 do
    if (@MDPlugins[i].ProcessRecPlay)<>NIL then
      MDPlugins[i].ProcessRecPlay(mode);
end;

procedure WrapperOnChannelChange(prg:TProgramm82);
var i:Integer;
begin
  LastProgram:=prg;
  for i:=0 to PluginCount-1 do
    if (@MDPlugins[i].ProcessChannelChange)<>NIL then
      MDPlugins[i].ProcessChannelChange(prg);
end;

procedure TMDPluginWorkspace.WMCommand(var msg: TWMCommand);
var i:integer;
begin
  for i:=0 to PluginCount-1 do
    if (@MDPlugins[i].ProcessPluginMenuCommand)<>NIL then
      MDPlugins[i].ProcessPluginMenuCommand(msg.ItemID);
  inherited;
end;

procedure TMDPluginWorkspace.WMUser(var msg : TMessage);
var
    i,n:integer;
    s:string;
    StartFilterParam:PStartFilterParam;
    CAT:PCAT;
begin
  if Not Initialized then Exit;

  if @WrapperPreprocessMessage<>nil then
    WrapperPreprocessMessage(msg);

  case msg.WParam of
    MDAPI_GET_VERSION:
      begin
        if LogGetVersion.Checked then
          LogAction('GET_VERSION');
        StrPCopy(PChar(msg.LParam), MDAPI_VERSION_FOR_PLUGINS);
      end;

    MDAPI_GET_PROGRAMM_NUMMER:
      begin
        PProgramNumberParam(msg.LParam)^.RealNumber:=1;
        PProgramNumberParam(msg.LParam)^.VirtNumber:=1;
        if LogGetProgramNumber.Checked then
          LogAction('Not supported: MDAPI_GET_PROGRAMM_NUMMER',True);
      end;

    MDAPI_SET_PROGRAMM_NUMMER:
        if LogSetProgramNumber.Checked then
          LogAction('Not supported: MDAPI_SET_PROGRAMM_NUMMER',True);

    MDAPI_GET_PROGRAMM:
      begin
        if LogGetProgram.Checked then
          LogAction('GET_PROGRAMM');
//        GetCurrentChannelMDInfo(PProgramm82(msg.LParam)^);
      end;

    MDAPI_SET_PROGRAMM:
        if LogSetProgram.Checked then
          LogAction('Not supported: SET_PROGRAMM',True);

    MDAPI_SAVE_PROGRAMM:
        if LogSaveProgram.Checked then
          LogAction('Not supported: SAVE_PROGRAMM',True);

    MDAPI_SCAN_CURRENT_CAT:
      begin
        if LogScanCAT.Checked then
          LogAction('SCAN_CURRENT_CAT');
      end;

    MDAPI_START_FILTER:
      begin
            StartFilterParam:=PStartFilterParam(msg.LParam);
            if LogStartFilter.Checked then
              LogAction('MDAPI_START_FILTER: '+PChar(@(StartFilterParam^.Name[0])));
            StartFilterParam^.Running_ID:=WrapperSetFilter(StartFilterParam^.Pid,TMDAPIFilterProc(StartFilterParam^.Irq_Call_Adresse),PChar(@(StartFilterParam^.Name[0])));
//            Filters[i].Name:=PChar(@(StartFilterParam^.Name[0]));
//            Filters[i].Proc:=TMDAPIFilterProc(StartFilterParam^.Irq_Call_Adresse);
//            Filters[i].Handle:=DVBAddFilter(StartFilterParam^.Pid,DVBCore2MDFilterThunk);
//            Filters[i].Active:=True;
//            StartFilterParam^.Running_ID:=Filters[i].Handle;
            if LogStartFilter.Checked then
              LogAction('Added Filter :'+IntToHex(StartFilterParam^.Running_ID,8)+' ('+IntToHex(StartFilterParam^.Pid,4)+')');
      end;

    MDAPI_STOP_FILTER:
      begin
        if LogStopFilter.Checked then
          LogAction('Stop Filter: '+IntToHex(msg.LParam,8));
        WrapperStopFilter(msg.LParam);
//        DVBDelFilter(msg.LParam);
//        for i:=0 to MaxFilterCount do
//          if (Filters[i].Active and (Filters[i].Handle=DWORD(msg.LParam))) then
//            Filters[i].Active:=False;
      end;

    MDAPI_START_OSD:
        if LogStartOSD.Checked then
          LogAction('Not supported: START_OSD',True);

    MDAPI_OSD_DRAWBLOCK:
        if LogOSDDrawBlock.Checked then
          LogAction('Not supported: OSD_DRAWBLOCK',True);

    MDAPI_OSD_SETFONT:
        if LogOSDSetFont.Checked then
          LogAction('Not supported: OSD_SETFONT',True);

    MDAPI_OSD_TEXT:
        if LogOSDText.Checked then
          LogAction('Not supported: OSD_TEXT',True);

    MDAPI_SEND_OSD_KEY:
        if LogSendOSDKey.Checked then
          LogAction('Not supported: OSD_KEY',True);

    MDAPI_STOP_OSD:
        if LogStopOSD.Checked then
          LogAction('Not supported: STOP_OSD',True);

    PROGAPI_GET_CHANNEL_NAME:
      begin
        n:=PWORDARRAY(msg.LParam)[2];
        if LogPROGAPIGetChannelName.Checked then
             LogAction('PROGAPI_GET_CHANNEL_NAME: SID='+inttohex(n,4),True);

      end;

    MDAPI_DVB_COMMAND:
      begin
        if LogDVBCommand.Checked then
        begin
          s:='';
          for i:=0 to 13 do
            s:=s+' '+IntToHex(PDVB_COMMAND(msg.LParam)^.Cmd_Buffer[i],2);
          LogAction('DVB_COMMAND: '+s);
        end;
        if PDVB_COMMAND(msg.LParam)^.Cmd_laenge<>7 then
          ShowMessage('DVB_COMMAND: Unknow command length='+inttostr(PDVB_COMMAND(msg.LParam)^.Cmd_laenge))
        else
        begin
          for i:=4 to 13 do
            SAA_COMMAND[i]:=PDVB_COMMAND(msg.LParam)^.Cmd_Buffer[i];
//          DVBSendSAACmd(@SAA_COMMAND[0],7);
          WrapperSetCSAKeys(SAA_COMMAND);
        end;
      end;
    else
      LogAction('Unknown API command: '+IntToHex(msg.WParam,8),True);
  end;

  inherited;
end;

procedure TMDPluginWorkspace.FormCreate(Sender: TObject);
  procedure ProcessMenu(menu:TMenuItem);
  var i:integer;
    s:string;
  begin
    for i:=0 to Menu.Count-1 do
      if Menu.Items[i].Count>0 then
        ProcessMenu(Menu.Items[i])
      else
      begin
        if Menu.Items[i].Checked then
          s:='1'
        else
          s:='0';
        s:=ini.ReadString('MD API Wrapper', Menu.Items[i].Name, s);
        if s='1' then
          Menu.Items[i].Checked:=True
        else
          Menu.Items[i].Checked:=False;
      end;
  end;
begin
    ini:=TIniFile.Create(extractfilepath(application.ExeName)+'Settings.ini');
    ProcessMenu(OptionsMenu);
//    InitMDPlugins;
    Top:=0;
    Left:=0;
end;

procedure TMDPluginWorkspace.FormDestroy(Sender: TObject);
  procedure ProcessMenu(menu:TMenuItem);
  var i:integer;
  begin
    for i:=0 to Menu.Count-1 do
      if Menu.Items[i].Count>0 then
        ProcessMenu(Menu.Items[i])
      else
        if Menu.Items[i].Checked then
          ini.WriteString('MD API Wrapper', Menu.Items[i].Name, '1')
        else
          ini.WriteString('MD API Wrapper', Menu.Items[i].Name, '0')
  end;
begin
  Initialized:=False;
//  CloseFilters;
  ProcessMenu(OptionsMenu);
  ini.Free;;
end;

procedure TMDPluginWorkspace.FormCloseQuery(Sender: TObject;
  var CanClose: Boolean);
begin
    MDPluginWorkspace.Hide;
    CanClose:=false;
end;

procedure TMDPluginWorkspace.PluginLogKeyPress(Sender: TObject;
  var Key: Char);
begin
    if Key=#27 then MDPluginWorkspace.Hide;
end;

end.
