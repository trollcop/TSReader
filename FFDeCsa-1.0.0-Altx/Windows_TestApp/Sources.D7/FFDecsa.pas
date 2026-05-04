unit FFDecsa;

interface
uses Windows,SysUtils,Dialogs;


type
 PFFDecsaCluster = ^TFFDecsaCluster;
 TFFDecsaCluster = Array[0..9] of Pointer;


var
 FFDeCsa_SetKeysProc:function (Ev,Od,kSet:PByteArray):Integer;stdcall;
 FFDeCsa_DecryptPkts:function (Cluster:PFFDecsaCluster;kSet:PByteArray):Integer;stdcall;
 FFDeCsa_GetKeysSize:function :Integer;stdcall;
 FFDeCsa_Parallelism:function :Integer;stdcall;


function  FFDeCsa_Load(ADllName:String):Boolean;
procedure FFDeCsa_Free;

implementation
var
 CsaDllID:Integer;

//------------------------------------------------------------------------------
function  FFDeCsa_Load(ADllName:String):Boolean;

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
   SetProcAddr(@FFDeCsa_SetKeysProc ,'set_control_words');
   SetProcAddr(@FFDeCsa_DecryptPkts ,'decrypt_packets'  );
   SetProcAddr(@FFDeCsa_GetKeysSize ,'get_keyset_size'  );
   SetProcAddr(@FFDeCsa_Parallelism ,'get_parallelism'  );
  end
 else
  begin
   Result := FALSE;
   MessageDlg('Cannot load library.'#13+ADllName,mtError,[mbOk],0);
  end;

 if NOT Result then
  begin
   FFDeCsa_Free;
   Exit;
  end;

end;

//------------------------------------------------------------------------------

procedure FFDeCsa_Free;
begin
 FFDeCsa_SetKeysProc := NIL;
 FFDeCsa_DecryptPkts := NIL;
 FFDeCsa_GetKeysSize := NIL;
 FFDeCsa_Parallelism := NIL;
 FreeLibrary(CsaDllID);
end;


end.
