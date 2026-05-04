unit DvbCsa;

interface
uses Windows,SysUtils,Dialogs;

var
  DvbCsa_SetKeysProc:procedure(EvOddKeys:PByteArray;KeyStruct:PByteArray);cdecl;
  DvbCsa_DecryptPkt :procedure(KeyStruct:PByteArray;Encrypted,Decrypted:PByteArray);cdecl;

function  DvbCsa_Load(ADllName:String):Boolean;
procedure DvbCsa_Free;

implementation

var
 CsaDllID:Integer;

//------------------------------------------------------------------------------
function  DvbCsa_Load(ADllName:String):Boolean;

procedure SetProcAddr(var AProcAddr:Pointer;AProcName:String);
begin
 AProcAddr := GetProcAddress(CsaDllID,PChar(AProcName));
 if AProcAddr = NIL then
  begin
   MessageDlg('Cannot find function <'+AProcName+'> in library :'#13+ADllName,mtError,[mbOk],0);
   Result := FALSE;
  end;
end;

begin

 CsaDllID := LoadLibrary(PChar(ADllName));
 if CsaDllID <> 0 then
  begin
   Result := TRUE;
   SetProcAddr(@DvbCsa_SetKeysProc ,'set_cws');
   SetProcAddr(@DvbCsa_DecryptPkt  ,'decrypt');
  end
 else
  begin
   Result := FALSE;
   MessageDlg('Cannot load library.'#13+ADllName,mtError,[mbOk],0);
  end;

 if NOT Result then
  begin
   DvbCsa_Free;
   Exit;
  end;
end;

//------------------------------------------------------------------------------
procedure DvbCsa_Free;
begin
 DvbCsa_SetKeysProc := NIL;
 DvbCsa_DecryptPkt  := NIL;
 FreeLibrary(CsaDllID);
end;


end.
 