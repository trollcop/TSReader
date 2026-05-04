unit Unit1;

interface

uses
  Windows, Messages, SysUtils, Variants, Classes, Graphics, Controls, Forms,
  Dialogs, StdCtrls;

type
  TForm1 = class(TForm)
    Button1: TButton;
    Memo1: TMemo;
    Button2: TButton;
    Button3: TButton;
    procedure FormCreate(Sender: TObject);
    procedure FormDestroy(Sender: TObject);
    procedure Button1Click(Sender: TObject);
    procedure Button2Click(Sender: TObject);
    procedure Button3Click(Sender: TObject);
  private
    { Private-Deklarationen }
  public
    { Public-Deklarationen }
  end;

var
  Form1: TForm1;

implementation

{$R *.dfm}
type
  TTunerType = (ttCable, ttSatellite, ttTerrestrial, ttATSC);
  PTuner = ^TTuner;
  TTuner = packed record
    Frequency: DWORD;
    SymbolRate: DWORD;
    FEC: DWORD;
    Polarity: DWORD;
    LNBFreq: DWORD;
    LNBSelection: DWORD;
    Diseqc: DWORD;
    Modulation: DWORD;

    BandWidth: DWORD; // only Terrestrial
    QAMMode: DWORD; // only Cable
  end;

  TCallBackFunc = function(buf: Pointer; len_in: integer): HRESULT; stdcall;

var
  GetTuner: function(var Tuner: TTuner): HResult; stdcall;
  SetTuner: function(var Tuner: TTuner): HResult; stdcall;
  GetTunerType: function: TTunerType; stdcall;
  StopStream: function: HResult; stdcall;
  StartStream: function(Callback: Pointer): HResult; stdcall;
  LibInstance: Longint;

function CallBack(buf: Pointer; len_in: integer): HRESULT; stdcall;
var line: String;
    offs: Word;
    Pid: Word;
begin
  result := NOERROR;
  line:='Size: '+inttostr(len_in);
  offs:=0;
  while len_in>0 do begin
   Pid:=((Byte(PChar(buf)[offs+1]) and $1F) shl 8) + Byte(PChar(Buf)[offs+2]);
   dec(len_in,188);
   inc(offs,188);
   line:=inttostr(pid)+' '+line;
  end;
   form1.memo1.lines.add('Pids: '+line);
end;

procedure TForm1.FormCreate(Sender: TObject);
begin
  memo1.lines.clear;
  LibInstance := loadlibrary('flexdump.dll');
  GetTuner := getprocaddress(LibInstance, 'GetTuner');
  SetTuner := getprocaddress(LibInstance, 'SetTuner');
  GetTunerType := getprocaddress(LibInstance, 'GetTunerType');
  StopStream := getprocaddress(LibInstance, 'StopStream');
  StartStream := getprocaddress(LibInstance, 'StartStream');
end;

procedure TForm1.FormDestroy(Sender: TObject);
begin
  GetTuner := nil;
  SetTuner := nil;
  GetTunerType := nil;
  StopStream := nil;
  StartStream := nil;
  freelibrary(LibInstance);
end;

procedure TForm1.Button1Click(Sender: TObject);
begin
  StartStream(@Callback);
end;

procedure TForm1.Button2Click(Sender: TObject);
begin
  StopStream;
end;

procedure TForm1.Button3Click(Sender: TObject);
var Tuner: TTuner;
begin
  GetTuner(Tuner);
  with memo1.lines do begin
    case GetTunerType of
      ttSatellite: Add('Satellite');
      ttCable: Add('Cable');
      ttATSC: Add('ATSC');
    else Add('DVB-T');
    end;
    Add('');
    Add('Frequency:    ' + inttostr(Tuner.Frequency));
    Add('Symbolrate:   ' + inttostr(Tuner.Symbolrate));
    Add('FEC:          ' + inttostr(Tuner.FEC));
    Add('Polarity:     ' + inttostr(Tuner.Polarity));
    Add('LNBFreq:      ' + inttostr(Tuner.LNBFreq));
    Add('LNBSelection: ' + inttostr(Tuner.LNBSelection));
    Add('DiSEqC:       ' + inttostr(Tuner.Diseqc));
    Add('Modulation:   ' + inttostr(Tuner.Modulation));
    Add('BandWidth:    ' + inttostr(Tuner.BandWidth));
    Add('QAMMode:      ' + inttostr(Tuner.QAMMode));
  end;
end;

end.

 