object Form1: TForm1
  Left = 315
  Top = 266
  BorderStyle = bsToolWindow
  Caption = 'FFDecsa-1.0.0-Altx ::  TestApp + CSA Benchmark'
  ClientHeight = 229
  ClientWidth = 447
  Color = clBtnFace
  Font.Charset = ANSI_CHARSET
  Font.Color = clWindowText
  Font.Height = -11
  Font.Name = 'Tahoma'
  Font.Style = []
  OldCreateOrder = False
  Position = poScreenCenter
  OnCreate = FormCreate
  PixelsPerInch = 96
  TextHeight = 13
  object GroupBox1: TGroupBox
    Left = 0
    Top = 0
    Width = 222
    Height = 73
    Align = alLeft
    Caption = 'Test: FFDecsa'
    TabOrder = 0
    object BtnFCsaDecryptTest: TBitBtn
      Left = 6
      Top = 42
      Width = 80
      Height = 25
      Caption = '1.Decrypt Test'
      TabOrder = 0
      OnClick = BtnFCsaDecryptTestClick
      Margin = 2
      Spacing = 2
    end
    object BtnFCsaSpeedTest: TBitBtn
      Left = 89
      Top = 42
      Width = 80
      Height = 25
      Caption = '2.Speed Test'
      Enabled = False
      Font.Charset = ANSI_CHARSET
      Font.Color = clNavy
      Font.Height = -11
      Font.Name = 'Tahoma'
      Font.Style = []
      ParentFont = False
      TabOrder = 1
      OnClick = BtnFCsaSpeedTestClick
      Margin = 2
      Spacing = 2
    end
    object ComboDllF: TComboBox
      Left = 6
      Top = 16
      Width = 200
      Height = 21
      Style = csDropDownList
      ItemHeight = 13
      TabOrder = 2
      OnChange = ComboDllFChange
    end
  end
  object GroupBox2: TGroupBox
    Left = 222
    Top = 0
    Width = 222
    Height = 73
    Align = alLeft
    Caption = 'Test: Clasic CSA'
    TabOrder = 1
    object ComboDllN: TComboBox
      Left = 6
      Top = 16
      Width = 200
      Height = 21
      Style = csDropDownList
      ItemHeight = 13
      TabOrder = 0
    end
    object BtnNCsaSpeedTest: TBitBtn
      Left = 6
      Top = 42
      Width = 80
      Height = 25
      Caption = 'SpeedTest'
      Font.Charset = ANSI_CHARSET
      Font.Color = clNavy
      Font.Height = -11
      Font.Name = 'Tahoma'
      Font.Style = []
      ParentFont = False
      TabOrder = 1
      OnClick = BtnNCsaSpeedTestClick
      Margin = 2
      Spacing = 2
    end
  end
  object MemoLog: TMemo
    Left = 0
    Top = 73
    Width = 447
    Height = 156
    Align = alBottom
    Font.Charset = ANSI_CHARSET
    Font.Color = clNavy
    Font.Height = -11
    Font.Name = 'Courier New'
    Font.Style = []
    ParentFont = False
    TabOrder = 2
    WordWrap = False
    OnDblClick = MemoLogDblClick
  end
end
