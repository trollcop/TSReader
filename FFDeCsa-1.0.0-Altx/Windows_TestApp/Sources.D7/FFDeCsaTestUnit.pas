unit FFDeCsaTestUnit;

interface

uses
  Windows, Messages, SysUtils, Variants, Classes, Graphics, Controls, Forms,
  Dialogs, StdCtrls, Buttons,
  FFDecsa,DvbCsa,CsaTestData;

type
  TForm1 = class(TForm)
    GroupBox1: TGroupBox;
    BtnFCsaDecryptTest: TBitBtn;
    BtnFCsaSpeedTest: TBitBtn;
    ComboDllF: TComboBox;
    GroupBox2: TGroupBox;
    ComboDllN: TComboBox;
    BtnNCsaSpeedTest: TBitBtn;
    MemoLog: TMemo;
    procedure BtnFCsaDecryptTestClick(Sender: TObject);
    procedure FormCreate(Sender: TObject);
    procedure BtnNCsaSpeedTestClick(Sender: TObject);
    procedure BtnFCsaSpeedTestClick(Sender: TObject);
    procedure ComboDllFChange(Sender: TObject);
    procedure MemoLogDblClick(Sender: TObject);
  private
    { Private declarations }
  public
    { Public declarations }
  end;

var
  Form1: TForm1;



implementation
{$R *.dfm}

const
 MAX_TS_PKTS_FOR_TEST = 50*1000;


var
 PeFreq  : TLargeInteger;
 AppDir  : String;

 PktsBuffer:Array[0..188*MAX_TS_PKTS_FOR_TEST] of Byte;


function  xTimeNow:TLargeInteger;
begin
 QueryPerformanceCounter(Result);
end;

function  xDiffTime(ARefer:TLargeInteger):Integer;  // Result in ms
var PeNow:TLargeInteger;
begin
 QueryPerformanceCounter(PeNow);
 PeNow := PeNow-ARefer;
 PeNow := (1000*PeNow) div PeFreq;
 Result := Longint(PeNow);
end;

//------------------------------------------------------------------------------

function ComparePkts(P1,P2:PByteArray;ATestName:String):Boolean;
var Ix:Integer;
begin
 Result := TRUE;
 for Ix := 0 to 183 do
  begin
   if Ix = 3 then Continue;
   if P1[Ix] <> P2[Ix] then
    begin
     Result := FALSE;
     Break;
    end;
  end;
 if NOT Result then MessageDlg(ATestName+#13#13+'Packets not equal at position '+IntToStr(Ix),mtError,[mbOk],0);
end;

//------------------------------------------------------------------------------

function TestDecrypt_FFDecsa(ALib:String):Boolean;
var TheKeys:PByteArray;
    OneBuf:Array[0..256] of Byte;
    Cluster:TFFDecsaCluster;
    KsSize,CsaPar:Integer;
begin
 Result := FALSE;
 if NOT FFDeCsa_Load(ALib) then Exit;
 KsSize := FFDeCsa_GetKeysSize;
 CsaPar := FFDeCsa_Parallelism;
 GetMem(TheKeys,KsSize);

 try
  Result := TRUE;
  FFDeCsa_SetKeysProc(@test_invalid_key,@test_1_key,TheKeys);
  CopyMemory(@onebuf,@test_1_encrypted,188);
  Cluster[0]:=@onebuf[0];Cluster[1]:=@onebuf[188];Cluster[2]:=NIL;
  FFDecsa_DecryptPkts(@Cluster,TheKeys);
  if NOT ComparePkts(@onebuf[0],@test_1_expected[0],'Test1-OddKey') then Result := FALSE;


  FFDeCsa_SetKeysProc(@test_2_key,@test_invalid_key,TheKeys);
  CopyMemory(@onebuf[0],@test_2_encrypted,188);
  Cluster[0]:=@onebuf[0];Cluster[1]:=@onebuf[188];Cluster[2]:=NIL;
  FFDecsa_DecryptPkts(@Cluster,TheKeys);
  if NOT ComparePkts(@onebuf[0],@test_2_expected,'Test2-EvenKey') then Result := FALSE;

  FFDeCsa_SetKeysProc(@test_3_key,@test_invalid_key,TheKeys);
  CopyMemory(@onebuf[0],@test_3_encrypted,188);
  Cluster[0]:=@onebuf[0];Cluster[1]:=@onebuf[188];Cluster[2]:=NIL;
  FFDecsa_DecryptPkts(@Cluster,TheKeys);
  if NOT ComparePkts(@onebuf[0],@test_3_expected,'Test3-EvenKey ($FF)') then Result := FALSE;

  FFDeCsa_SetKeysProc(@test_p_10_0_key,@test_invalid_key,TheKeys);
  CopyMemory(@onebuf[0],@test_p_10_0_encrypted,188);
  Cluster[0]:=@onebuf[0];Cluster[1]:=@onebuf[188];Cluster[2]:=NIL;
  FFDecsa_DecryptPkts(@Cluster,TheKeys);
  if NOT ComparePkts(@onebuf[0],@test_p_10_0_expected,'Test-Payload 80 Bytes') then Result := FALSE;

  FFDeCsa_SetKeysProc(@test_p_1_6_key,@test_invalid_key,TheKeys);
  CopyMemory(@onebuf[0],@test_p_1_6_encrypted,188);
  Cluster[0]:=@onebuf[0];Cluster[1]:=@onebuf[188];Cluster[2]:=NIL;
  FFDecsa_DecryptPkts(@Cluster,TheKeys);
  if NOT ComparePkts(@onebuf[0],@test_p_1_6_expected,'Test Payload+Residue') then Result := FALSE;
 except
  on E:Exception do
   begin
    Result := FALSE;
    MessageDlg('Exception in FFDecsa: '+E.Message+#13#13+'Check if your processor support MMX/SSE instructions',mtError,[mbOk],0);
   end;
 end;
 FreeMem(TheKeys);
 FFDeCsa_Free;
 if Result then MessageDlg('All tests was OK !'#13+
                           'KeyStru Size = '+IntToStr(KsSize)+#13+
                           'Parallelism  = '+IntToStr(CsaPar),mtInformation,[mbOk],0);
end;

//------------------------------------------------------------------------------

function TestSpeed_FFDecsa(ALib:String;var ResultTxt:String):Boolean;
var PktsDone,Ix,PacketsForTest:Integer;

    Cluster:TFFDecsaCluster;
    StartTime:TLargeInteger;
    DecryptTime,Tmp,ParallPk,ParallSz,BufPtr:Integer;
    TimeInSec,MbPs,PkPs:Double;
    PriorityClass,Priority:Integer;
    KeySpace:PByteArray;
    AlignedK:PByteArray;

const
 oboCSize = 72;

begin
 Result := FALSE;
 if NOT FFDeCsa_Load(ALib) then Exit;
 GetMem(KeySpace,FFDeCsa_GetKeysSize+4);


 // Hmmm... with aligned KeySpace : 20 Mb/s improvement
 AlignedK := KeySpace;
 while (Cardinal(AlignedK) AND $03) <> 0 do AlignedK := Pointer(Integer(AlignedK)+1);


 // Calculate suggested cluster size .. see FFDecsa docs
 Tmp := FFDeCsa_Parallelism;
 ParallPk := Tmp+(Tmp DIV 10);
 if (ParallPk < Tmp+5) then ParallPk := Tmp+5;
 ParallSz := 188*ParallPk;

 PacketsForTest := MAX_TS_PKTS_FOR_TEST;

 for ix:=0 to PacketsForTest-1 do
  CopyMemory(@PktsBuffer[188*ix],@test_2_encrypted,188);

 FFDeCsa_SetKeysProc(@test_2_key,@test_invalid_key,AlignedK);

 PriorityClass := GetPriorityClass(GetCurrentProcess);
 Priority      := GetThreadPriority(GetCurrentThread);

 SetPriorityClass(GetCurrentProcess, REALTIME_PRIORITY_CLASS);
 SetThreadPriority(GetCurrentThread, THREAD_PRIORITY_TIME_CRITICAL);

 // Speed test
 StartTime := xTimeNow;
 PktsDone :=0;
 while(PktsDone < (PacketsForTest-ParallPk)) do
  begin
   BufPtr := 188*PktsDone;
   Cluster[0] := @PktsBuffer[188*PktsDone];
   Cluster[1] := @PktsBuffer[BufPtr+ParallSz];
   Cluster[2] := NIL;
   PktsDone   := PktsDone + FFDecsa_DecryptPkts(@Cluster,AlignedK);
  end;
 DecryptTime := xDiffTime(StartTime);

 SetPriorityClass(GetCurrentProcess, PriorityClass);
 SetThreadPriority(GetCurrentThread, Priority);
 FreeMem(KeySpace);
 FFDeCsa_Free;

 TimeInSec := DecryptTime/1000;
 MbPs := (184*PktsDone*8)/TimeInSec/1000000;
 PkPs := PktsDone/TimeInSec;
 ResultTxt := Format('Speed=%.2f Mbit/s , %.2f pkts/s',[MbPs,PkPs]);
 MessageDlg(ALib+#13#13+ResultTxt,mtInformation,[mbOk],0);
end;


function TestSpeed_NormCsa(ALib:String;var ResultTxt:String):Boolean;
var Ix,PacketsForTest:Integer;
    StartTime:TLargeInteger;
    DecryptTime:Integer;
    TimeInSec,MbPs,PkPs:Double;
    PriorityClass,Priority:Integer;
    
    OneBuf:Array[0..256] of Byte;

var
  KeyStruct:Array[0..800] of Byte;
  CrtKeySet:Array[0..15] of Byte;

begin
 Result := FALSE;
 if NOT DvbCsa_Load(ALib) then Exit;

 PacketsForTest := MAX_TS_PKTS_FOR_TEST DIV 10;

 // Fill test buffer
 for ix:=0 to PacketsForTest-1 do
  CopyMemory(@PktsBuffer[188*ix],@test_2_encrypted,188);


 // Set keys
 CopyMemory(@CrtKeySet[0],@test_2_key,8);
 CopyMemory(@CrtKeySet[8],@test_invalid_key,8);
 DvbCsa_SetKeysProc(@CrtKeySet[0],@KeyStruct[0]);

 // Test one packet
 CopyMemory(@OneBuf[0],@test_2_encrypted,188);
 DvbCsa_DecryptPkt(@KeyStruct[0],@OneBuf[0],NIL);
 if NOT ComparePkts(@onebuf[0],@test_2_expected,'Test2-EvenKey DvbCsa') then
  begin
   DvbCsa_Free;
   Exit;
  end;



 PriorityClass := GetPriorityClass(GetCurrentProcess);
 Priority      := GetThreadPriority(GetCurrentThread);
 SetPriorityClass(GetCurrentProcess, REALTIME_PRIORITY_CLASS);
 SetThreadPriority(GetCurrentThread, THREAD_PRIORITY_TIME_CRITICAL);

 // Speed test
 StartTime := xTimeNow;
 Ix := 0;
 while Ix < PacketsForTest do
  begin
   DvbCsa_DecryptPkt(@KeyStruct[0],@PktsBuffer[188*ix],NIL);
   Ix := Ix + 1;
  end;
 DecryptTime := xDiffTime(StartTime);

 SetPriorityClass(GetCurrentProcess, PriorityClass);
 SetThreadPriority(GetCurrentThread, Priority);
 DvbCsa_Free;

 TimeInSec := DecryptTime/1000;
 MbPs := (184*PacketsForTest*8)/TimeInSec/1000000;
 PkPs := PacketsForTest/TimeInSec;
 ResultTxt := Format('Speed=%.2f Mbit/s , %.2f pkts/s',[MbPs,PkPs]);
 MessageDlg(ALib+#13#13+ResultTxt ,mtInformation,[mbOk],0);
end;


procedure TForm1.FormCreate(Sender: TObject);
var sr:TSearchRec;
begin
 ComboDllN.Items.Clear;
 ComboDllF.Items.Clear;
 if FindFirst(AppDir+'*.dll',faAnyFile,sr) = 0 then
  begin
   repeat
    if UpperCase(Copy(Sr.Name,1,2)) = 'FF'  then ComboDllF.Items.Add(Sr.Name); // FFDecsa_x.dll
    if UpperCase(Copy(Sr.Name,1,3)) = 'CSA' then ComboDllN.Items.Add(Sr.Name); // Csa_x.dll
   until FindNext(sr) <> 0;
   FindClose(sr);
  end;

 if ComboDllN.Items.Count > 0 then ComboDllN.ItemIndex := 0;
 if ComboDllF.Items.Count > 0 then ComboDllF.ItemIndex := 0;
end;

procedure TForm1.ComboDllFChange(Sender: TObject);
begin
 BtnFCsaSpeedTest.Enabled := FALSE;
end;


procedure TForm1.BtnFCsaDecryptTestClick(Sender: TObject);
begin
 if TestDecrypt_FFDecsa(AppDir+ComboDllF.Text) then BtnFCsaSpeedTest.Enabled := TRUE;
end;

procedure TForm1.BtnFCsaSpeedTestClick(Sender: TObject);
var DllName,Tmp:String;
begin
 DllName := ComboDllF.Text;
 TestSpeed_FFDecsa(AppDir+DllName,Tmp);
// while Length(DllName) < 22 do DllName := DllName+' ';
 MemoLog.Lines.Add(DllName+' : '+Tmp);
end;

procedure TForm1.BtnNCsaSpeedTestClick(Sender: TObject);
var DllName,Tmp:String;
begin
 DllName := ComboDllN.Text;
 TestSpeed_NormCsa(AppDir+ComboDllN.Text,Tmp);
// while Length(DllName) < 22 do DllName := DllName+' ';
 MemoLog.Lines.Add(DllName+' : '+Tmp);
end;


procedure TForm1.MemoLogDblClick(Sender: TObject);
begin
 MemoLog.Lines.Clear;
end;

initialization
 AppDir := ExtractFilePath(ParamStr(0));
 QueryPerformanceFrequency(PeFreq);

end.
