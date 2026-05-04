program FFDeCsaTest;

uses
  Forms,
  FFDeCsaTestUnit in 'FFDeCsaTestUnit.pas' {Form1},
  CsaTestData in 'CsaTestData.pas',
  FFDecsa in 'FFDecsa.pas',
  DvbCsa in 'DvbCsa.pas';

{$R *.res}

begin
  Application.Initialize;
  Application.CreateForm(TForm1, Form1);
  Application.Run;
end.
