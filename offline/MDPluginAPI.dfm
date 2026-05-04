object MDPluginWorkspace: TMDPluginWorkspace
  Left = 280
  Top = 200
  Width = 419
  Height = 480
  Caption = 'MD API Wrapper. (C) SatRookie, Symbiose'
  Color = clBtnFace
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -11
  Font.Name = 'MS Sans Serif'
  Font.Style = []
  Menu = MainMenu
  OldCreateOrder = False
  OnCloseQuery = FormCloseQuery
  OnCreate = FormCreate
  OnDestroy = FormDestroy
  PixelsPerInch = 96
  TextHeight = 13
  object PluginLog: TListBox
    Left = 0
    Top = 0
    Width = 411
    Height = 434
    Align = alClient
    Font.Charset = DEFAULT_CHARSET
    Font.Color = clWindowText
    Font.Height = -11
    Font.Name = 'Courier New'
    Font.Style = []
    ItemHeight = 14
    ParentFont = False
    TabOrder = 0
    OnKeyPress = PluginLogKeyPress
  end
  object MainMenu: TMainMenu
    Left = 48
    Top = 40
    object OptionsMenu: TMenuItem
      AutoCheck = True
      Caption = 'Options'
      object MessageBoxOnError: TMenuItem
        AutoCheck = True
        Caption = 'MessageBox on Error'
        Checked = True
      end
      object ChannelInfoOnlyFromCache: TMenuItem
        AutoCheck = True
        Caption = 'Channel Info Only From Cache'
        Checked = True
      end
      object LogAllActions: TMenuItem
        AutoCheck = True
        Caption = 'Log Actions'
        Checked = True
      end
      object Acttions1: TMenuItem
        Caption = 'Actions To Log'
        object LogLoadPlugin: TMenuItem
          AutoCheck = True
          Caption = 'Load Plugin'
          Checked = True
        end
        object LogDetectPlugin: TMenuItem
          AutoCheck = True
          Caption = 'Detect Plugin'
          Checked = True
        end
        object LogGetVersion: TMenuItem
          AutoCheck = True
          Caption = 'Get Version'
          Checked = True
        end
        object LogGetProgramNumber: TMenuItem
          AutoCheck = True
          Caption = 'Get Program Number'
        end
        object LogSetProgramNumber: TMenuItem
          AutoCheck = True
          Caption = 'Set Program Number'
        end
        object LogGetProgram: TMenuItem
          AutoCheck = True
          Caption = 'Get Program'
          Checked = True
        end
        object LogSetProgram: TMenuItem
          AutoCheck = True
          Caption = 'Set Program'
        end
        object LogSaveProgram: TMenuItem
          AutoCheck = True
          Caption = 'Save Program'
        end
        object LogScanCAT: TMenuItem
          AutoCheck = True
          Caption = 'Scan CAT'
          Checked = True
        end
        object LogStartFilter: TMenuItem
          AutoCheck = True
          Caption = 'Start Filter'
          Checked = True
        end
        object LogStopFilter: TMenuItem
          AutoCheck = True
          Caption = 'Stop Filter'
          Checked = True
        end
        object LogFilterAction: TMenuItem
          AutoCheck = True
          Caption = 'Filter Action'
          Checked = True
        end
        object LogFilterCallback: TMenuItem
          AutoCheck = True
          Caption = 'Filter Callback'
        end
        object LogCloseFilter: TMenuItem
          AutoCheck = True
          Caption = 'Close Filter'
          Checked = True
        end
        object LogStartOSD: TMenuItem
          AutoCheck = True
          Caption = 'Start OSD'
        end
        object LogOSDDrawBlock: TMenuItem
          AutoCheck = True
          Caption = 'OSD Draw Block'
        end
        object LogOSDSetFont: TMenuItem
          AutoCheck = True
          Caption = 'OSD Set Font'
        end
        object LogOSDText: TMenuItem
          AutoCheck = True
          Caption = 'OSD Text'
        end
        object LogSendOSDKey: TMenuItem
          AutoCheck = True
          Caption = 'Send OSD Key'
        end
        object LogStopOSD: TMenuItem
          AutoCheck = True
          Caption = 'Stop OSD'
        end
        object LogDVBCommand: TMenuItem
          AutoCheck = True
          Caption = 'DVB Command'
          Checked = True
        end
        object LogPROGAPIGetChannelName: TMenuItem
          AutoCheck = True
          Caption = 'PROGAPI Get Channel Name'
        end
      end
    end
  end
end
