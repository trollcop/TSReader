unit Unit1;

interface

uses
  Windows, Messages, SysUtils, Variants, Classes, Graphics, Controls, Forms,
  Dialogs, StdCtrls;

type
  TForm1 = class(TForm)
    Button1: TButton;
    Button2: TButton;
    Edit1: TEdit;
    Edit2: TEdit;
    Edit3: TEdit;
    Edit4: TEdit;
    Edit5: TEdit;
    Edit6: TEdit;
    Edit7: TEdit;
    Label1: TLabel;
    Label2: TLabel;
    Label3: TLabel;
    Label4: TLabel;
    Label5: TLabel;
    Label6: TLabel;
    Label7: TLabel;
    Label8: TLabel;
    Label9: TLabel;
    Label10: TLabel;
    Label11: TLabel;
    Label12: TLabel;
    Label13: TLabel;
    Label14: TLabel;
    Label15: TLabel;
    procedure Button1Click(Sender: TObject);
    procedure Button2Click(Sender: TObject);
    procedure FormCreate(Sender: TObject);
    procedure Edit1Change(Sender: TObject);
    procedure Edit3Change(Sender: TObject);
    procedure Edit6Change(Sender: TObject);
    procedure Edit2Change(Sender: TObject);
    procedure Edit7Change(Sender: TObject);
    procedure Edit4Change(Sender: TObject);
    procedure Edit5Change(Sender: TObject);
  private
    { Private declarations }
  public
    { Public declarations }
  end;

var
  Form1: TForm1;

implementation

uses MDPluginAPI, IniFiles;

type
TFilter=record
  Name:String;
  Active:boolean;
  //Handle:Integer;
  PID:Integer;
  CallBackFunction:TMDAPIFilterProc;
end;


const MAX_FILTER_COUNT=16;
  oddkey:array[0..7] of byte=($07, $e0, $1b, $02, $c9, $e0, $45, $ee);
  evenkey:array[0..7] of byte=($07, $e0, $1b, $02, $c9, $e0, $45, $ee);
  testbuf:array[0..183] of byte=(
 $de, $cf, $0a, $0d, $b2, $d7, $c4, $40, $de, $5d, $63, $18, $5a, $98, $17, $aa, $c9, $bc, $27, $c6, $cb, $49, $40, $48, $fd, $20, $b7, $05, $5b, $27, $cb, $eb,
 $9a, $f0, $ac, $45, $6d, $56, $f4, $7b, $6f, $a0, $57, $f3, $9b, $f7, $a2, $c7, $d4, $68, $24, $00, $2f, $28, $13, $96, $94, $a8, $7c, $f4, $6f, $07, $2a, $0e,
 $e8, $a1, $eb, $c7, $80, $ac, $1f, $79, $bf, $5d, $b6, $10, $7c, $2e, $52, $e9, $34, $2c, $a8, $39, $01, $73, $04, $24, $a8, $1e, $db, $5b, $cb, $24, $f6, $31,
 $ab, $02, $6b, $f9, $f6, $f7, $e9, $52, $ad, $cf, $62, $0f, $42, $f6, $66, $5d, $c0, $86, $f2, $7b, $40, $20, $a9, $bd, $1f, $fd, $16, $ad, $2e, $75, $a6, $a0,
 $85, $f3, $9c, $31, $20, $4e, $fb, $95, $61, $78, $ce, $10, $c1, $48, $5f, $d3, $61, $05, $12, $f4, $e2, $04, $ae, $e0, $86, $01, $56, $55, $b1, $0f, $a6, $33,
 $95, $20, $92, $f0, $be, $39, $31, $e1, $2a, $f7, $93, $b4, $f7, $e4, $f1, $85, $ae, $50, $f1, $63, $d4, $5d, $9c, $6c
  );
var Filters:array [1..MAX_FILTER_COUNT] of TFilter;
  VideoPID,AudioPID,ECMPID:Integer;
  FileInName,FileOutName:String;

{$R *.dfm}
function PreprocessMessage(var msg : TMessage):DWORD;stdcall;
begin
  //ShowMessage('Hi');
  Result:=0;
end;

function SetFilter(wPID: WORD; FilterProc:TMDAPIFilterProc;Name:string=''): DWORD; stdcall;
var i:Integer;
begin
  Result := 0;
  for i:=1 to MAX_FILTER_COUNT do
    if not Filters[i].Active then
    with Filters[i] do
    begin
      Active:=True;
      PID:=wPID;
      CallBackFunction:=FilterProc;
      result:=i;
      exit;
    end;
end;

function StopFilter(wHandle:DWORD): DWORD; stdcall;
begin
  Filters[wHandle].Active:=False;
  Result:=0;
end;

type
  TSetKeyProc=procedure( key: PByteArray; keystruct:PByteArray);cdecl;
  TCSADecryptProc=procedure(keystruct:PByteArray;Encrypted,Decrypted:PByteArray);cdecl;
var
  DLLID:Integer;
  SetKeyProc:TSetKeyProc;
  CSADecryptProc:TCSADecryptProc;
  KeyStruct:array[0..800] of byte;
  key:array[0..15] of byte;
  OddKeyFound:boolean=False;
  EvenKeyFound:boolean=False;

procedure SetCSAKeys(Command:array of byte);
var i:Integer;
begin
  if Command[4]=1 then
    OddKeyFound:=True
  else
    EvenKeyFound:=True;
  for i:=0 to 3 do
  begin
    key[(Command[4])*8+i*2+0]:=Command[6+i*2+1];
    key[(Command[4])*8+i*2+1]:=Command[6+i*2+0];
    //key[(1-Command[4])*8+i*2+0]:=Command[6+i*2+1];
    //key[(1-Command[4])*8+i*2+1]:=Command[6+i*2+0];
  end;
  if OddKeyFound and EvenKeyFound then
    SetKeyProc(@key[0],@keyStruct[0]);
end;

procedure TestCSADll;
begin
  DLLID:=LoadLibrary(PChar('CSA.dll'));
  SetKeyProc:=GetProcAddress(DLLID,'set_cws');
  CSADecryptProc:=GetProcAddress(DLLID,'decrypt');
end;

const PACKET_SIZE=188;
var
  Buffer,DecryptedBuffer:array [0..PACKET_SIZE-1] of byte; {packed record
    SyncByte,
    Byte1,Byte2,Byte3:byte;
    Data:Array[0..PACKET_SIZE-1-4] of byte;
  end;
  }
  FIn, FOut:Integer;

procedure ProcessPacket;
var
  TransportErrorIndicator,
  PayloadUnitStartIndicator,
  TransportPriority:boolean;
  Pid,
  Scrambling:Integer;
  i:Integer;

begin
  TransportErrorIndicator   := (Buffer[1] and $80) <> 0;
  PayloadUnitStartIndicator := (Buffer[1] and $40) <> 0;
  TransportPriority         := (Buffer[1] and $20) <> 0;
  Pid                       := ((Buffer[1] and $1F) shl 8)+Buffer[2];
  Scrambling                := (Buffer[3] and $C0);
  if Pid<>$1FFF then
  begin
    begin
      for i:=1 to MAX_FILTER_COUNT do
        if Filters[i].Active and (Filters[i].PID=Pid) then
          Filters[i].CallBackFunction(i,184,@buffer[4]);
      if OddKeyFound and EvenKeyFound then
        if (Pid=VideoPID) or (Pid=AudioPID) then
        begin
          if Scrambling<>0 then
          begin
            CSADecryptProc(@KeyStruct[0],@Buffer[0],@Buffer[0]);
          end;
          FileWrite(FOut,Buffer,PACKET_SIZE);
        end;
    end;
  end;
end;

procedure ProcessStream;
var cnt:Integer;
begin
  cnt:=0;
  FIn:=FileOpen(FileInName, fmOpenRead);
  if Fin>-1 then
  begin
    FOut:=FileCreate(FileOutName);
    if FOut>-1 then
    begin
      while FileRead(FIn,Buffer,PACKET_SIZE)=PACKET_SIZE do
      begin
        ProcessPacket;
        Application.ProcessMessages;
        cnt:=(cnt+1) mod 50;
        if cnt=0 then sleep(1);
      end;
      FileClose(FOut);
    end;
    FileClose(FIn);
  end;
end;

procedure TForm1.Button1Click(Sender: TObject);
var prg:TProgramm82;
  i:Integer;
  ini:TIniFile;

begin
  ini:=TIniFile.Create(extractfilepath(application.ExeName)+'OfflineDecoder.ini');
  VideoPID:=ini.ReadInteger('Channel','VideoPID',0);
  AudioPID:=ini.ReadInteger('Channel','AudioPID',0);
  ECMPID:=ini.ReadInteger('Channel','ECMPID',0);
  FileInName:=ini.ReadString('Channel','FileName','');
  FileOutName:=ini.ReadString('Channel','DecodedFileName','');

  WrapperPreprocessMessage:=PreprocessMessage;
  WrapperSetFilter:=SetFilter;
  WrapperStopFilter:=StopFilter;
  WrapperSetCSAKeys:=SetCSAKeys;
  MDPluginWorkspace.InitMDPlugins;

  prg.Name        := 'Dummy channel';
  prg.Anbieter    := 'Dummy provider';
  prg.Land        := 'Dummy land';
  prg.Freq        := 12345000;
  prg.tp_id       := 12345;
  prg.Video_pid   := VideoPID;
  prg.Audio_pid   := AudioPID;
  prg.TeleText_pid:= 0;
  prg.PMT_pid     := 162;
  prg.PCR_pid     := 0;
  prg.ECM_PID     := ECMPID;
  prg.SID_pid     := ini.ReadInteger('Channel','SID',0);
  prg.AC3_pid     := 0;
  prg.CA_ID       := 0;
  prg.SID_pid     := 0;
  prg.CA_Anzahl   := 1;

  for i:=0 to prg.CA_Anzahl-1 do
  begin
    prg.CA_System82[i].CA_Typ:=ini.ReadInteger('Channel','CAType',0);
    prg.CA_System82[i].ECM:=ECMPID;
    prg.CA_System82[i].EMM:=0;
  end;
  WrapperOnChannelChange(prg);
  ini.Free;
end;

procedure TForm1.Button2Click(Sender: TObject);
begin
  ProcessStream;
  ShowMessage('Finished');
end;

procedure TForm1.FormCreate(Sender: TObject);
begin
  TestCSADll;
end;



procedure TForm1.Edit1Change(Sender: TObject);


 var
  ini:TIniFile;

begin
    ini := TIniFile.Create(extractfilepath(application.ExeName)+'OfflineDecoder.ini');


  with Ini do
  begin
    WriteString('Channel','FileName',Edit1.Text);
    Free;
  end;

end;

procedure TForm1.Edit3Change(Sender: TObject);
 var
  ini:TIniFile;
begin
  ini := TIniFile.Create(extractfilepath(application.ExeName)+'OfflineDecoder.ini');
  with Ini do
  begin

    WriteInteger('Channel','SID',StrToInt(Edit3.Text));
    Free;
  end;

end;

procedure TForm1.Edit6Change(Sender: TObject);
 var
  ini:TIniFile;
begin
  ini := TIniFile.Create(extractfilepath(application.ExeName)+'OfflineDecoder.ini');
  with Ini do
  begin
    WriteInteger('Channel','ECMPID',StrToInt(Edit6.Text));
    Free;
  end;

end;
procedure TForm1.Edit2Change(Sender: TObject);

 var
  ini:TIniFile;

begin
    ini := TIniFile.Create(extractfilepath(application.ExeName)+'OfflineDecoder.ini');


  with Ini do
  begin
    WriteString('Channel','DecodedFileName',Edit2.Text);
    Free;
  end;

end;

procedure TForm1.Edit7Change(Sender: TObject);

 var
  ini:TIniFile;

begin
    ini := TIniFile.Create(extractfilepath(application.ExeName)+'OfflineDecoder.ini');


  with Ini do
  begin
    WriteInteger('Channel','CAType',StrToInt(Edit7.Text));
    Free;
  end;

end;

procedure TForm1.Edit4Change(Sender: TObject);

 var
  ini:TIniFile;

begin
    ini := TIniFile.Create(extractfilepath(application.ExeName)+'OfflineDecoder.ini');


  with Ini do
  begin
    WriteInteger('Channel','VideoPID',StrToInt(Edit4.Text));
    Free;
  end;

end;

procedure TForm1.Edit5Change(Sender: TObject);

 var
  ini:TIniFile;

begin
    ini := TIniFile.Create(extractfilepath(application.ExeName)+'OfflineDecoder.ini');


  with Ini do
  begin
    WriteInteger('Channel','AudioPID',StrToInt(Edit5.Text));
    Free;
  end;

end;

end.
