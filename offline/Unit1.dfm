object Form1: TForm1
  Left = 190
  Top = 105
  Width = 456
  Height = 213
  Caption = 'Offline Decoder'
  Color = clBtnFace
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -11
  Font.Name = 'MS Sans Serif'
  Font.Style = []
  OldCreateOrder = False
  OnCreate = FormCreate
  PixelsPerInch = 96
  TextHeight = 13
  object Label1: TLabel
    Left = 176
    Top = 16
    Width = 24
    Height = 13
    Caption = 'Input'
  end
  object Label2: TLabel
    Left = 168
    Top = 40
    Width = 32
    Height = 13
    Caption = 'Output'
  end
  object Label3: TLabel
    Left = 184
    Top = 64
    Width = 18
    Height = 13
    Caption = 'SID'
  end
  object Label4: TLabel
    Left = 176
    Top = 88
    Width = 27
    Height = 13
    Caption = 'Video'
  end
  object Label5: TLabel
    Left = 176
    Top = 112
    Width = 27
    Height = 13
    Caption = 'Audio'
  end
  object Label6: TLabel
    Left = 176
    Top = 136
    Width = 23
    Height = 13
    Caption = 'ECM'
  end
  object Label7: TLabel
    Left = 160
    Top = 160
    Width = 41
    Height = 13
    Caption = 'CA Type'
  end
  object Label8: TLabel
    Left = 40
    Top = 72
    Width = 86
    Height = 13
    Caption = 'ALL PIDS ARE IN'
  end
  object Label9: TLabel
    Left = 32
    Top = 88
    Width = 95
    Height = 13
    Caption = 'DECIMAL FORMAT'
  end
  object Label10: TLabel
    Left = 16
    Top = 8
    Width = 120
    Height = 13
    Caption = 'FIRST FILL IN OPTIONS'
  end
  object Label11: TLabel
    Left = 8
    Top = 24
    Width = 149
    Height = 13
    Caption = 'IF NONE ARE FILLED IN THE '
  end
  object Label12: TLabel
    Left = 192
    Top = 0
    Width = 63
    Height = 13
    Caption = '1 - OPTIONS'
  end
  object Label13: TLabel
    Left = 0
    Top = 40
    Width = 163
    Height = 13
    Caption = 'PREVIOUS DEFINED ARE USED'
  end
  object Label14: TLabel
    Left = 336
    Top = 88
    Width = 15
    Height = 13
    Caption = '2 - '
  end
  object Label15: TLabel
    Left = 336
    Top = 136
    Width = 12
    Height = 13
    Caption = '3- '
  end
  object Button1: TButton
    Left = 352
    Top = 80
    Width = 89
    Height = 25
    Caption = 'Load Plugins'
    TabOrder = 0
    OnClick = Button1Click
  end
  object Button2: TButton
    Left = 352
    Top = 128
    Width = 89
    Height = 25
    Caption = 'Process Stream'
    TabOrder = 1
    OnClick = Button2Click
  end
  object Edit1: TEdit
    Left = 208
    Top = 16
    Width = 225
    Height = 21
    TabOrder = 2
    Text = 'C:\mpg_In.mpg'
    OnChange = Edit1Change
  end
  object Edit2: TEdit
    Left = 208
    Top = 40
    Width = 225
    Height = 21
    TabOrder = 3
    Text = 'C:\mpg_out.mpg'
    OnChange = Edit2Change
  end
  object Edit3: TEdit
    Left = 208
    Top = 64
    Width = 121
    Height = 21
    TabOrder = 4
    Text = 'Channel #'
    OnChange = Edit3Change
  end
  object Edit4: TEdit
    Left = 208
    Top = 88
    Width = 121
    Height = 21
    TabOrder = 5
    Text = 'PID'
    OnChange = Edit4Change
  end
  object Edit5: TEdit
    Left = 208
    Top = 112
    Width = 121
    Height = 21
    TabOrder = 6
    Text = 'PID'
    OnChange = Edit5Change
  end
  object Edit6: TEdit
    Left = 208
    Top = 136
    Width = 121
    Height = 21
    TabOrder = 7
    Text = 'PID'
    OnChange = Edit6Change
  end
  object Edit7: TEdit
    Left = 208
    Top = 160
    Width = 121
    Height = 21
    TabOrder = 8
    Text = 'Smartcard PID'
    OnChange = Edit7Change
  end
end
