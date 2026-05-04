VERSION 5.00
Object = "{008BBE7B-C096-11D0-B4E3-00A0C901D681}#1.0#0"; "teechart.ocx"
Object = "{15FFCB8C-B960-4DB6-AC1F-04FEEF2F5CA4}#1.0#0"; "BOARD1~1.OCX"
Begin VB.Form frmMain 
   Caption         =   "Board1049 Example App"
   ClientHeight    =   7920
   ClientLeft      =   165
   ClientTop       =   735
   ClientWidth     =   12825
   LinkTopic       =   "Form1"
   ScaleHeight     =   528
   ScaleMode       =   3  'Pixel
   ScaleWidth      =   855
   StartUpPosition =   3  'Windows Default
   Begin TeeChart.TChart tctMultipathTaps 
      Height          =   2895
      Left            =   180
      OleObjectBlob   =   "Board1049 Example App.frx":0000
      TabIndex        =   14
      Top             =   4890
      Width           =   6165
   End
   Begin TeeChart.TChart tctConstellation 
      Height          =   3435
      Left            =   5760
      OleObjectBlob   =   "Board1049 Example App.frx":0684
      TabIndex        =   3
      Top             =   750
      Width           =   3480
   End
   Begin TeeChart.TChart tctEyeDiagram 
      Height          =   3435
      Left            =   9300
      OleObjectBlob   =   "Board1049 Example App.frx":0FF8
      TabIndex        =   13
      Top             =   750
      Width           =   3465
   End
   Begin TeeChart.TChart tctChanResp 
      Height          =   2895
      Left            =   6540
      OleObjectBlob   =   "Board1049 Example App.frx":1584D
      TabIndex        =   18
      Top             =   4890
      Width           =   6165
   End
   Begin VB.CommandButton Command1 
      Caption         =   "S"
      Height          =   375
      Left            =   0
      TabIndex        =   67
      Top             =   4440
      Width           =   375
   End
   Begin VB.CheckBox chkDisableEq 
      Caption         =   "No Eq"
      Height          =   375
      Left            =   8400
      Style           =   1  'Graphical
      TabIndex        =   64
      Top             =   4440
      Width           =   735
   End
   Begin VB.CheckBox chkFreezeFFE 
      Caption         =   "FFE"
      Height          =   375
      Left            =   7560
      Style           =   1  'Graphical
      TabIndex        =   63
      Top             =   4440
      Width           =   735
   End
   Begin VB.CheckBox chkFreezeDFE 
      Caption         =   "DFE"
      Height          =   375
      Left            =   6720
      Style           =   1  'Graphical
      TabIndex        =   62
      Top             =   4440
      Width           =   735
   End
   Begin BOARD1049Lib.Board1049 Board1049 
      Left            =   12120
      Top             =   4320
      _Version        =   65536
      _ExtentX        =   850
      _ExtentY        =   850
      _StockProps     =   0
   End
   Begin VB.ComboBox cboPolling2 
      Height          =   315
      ItemData        =   "Board1049 Example App.frx":15E9C
      Left            =   11400
      List            =   "Board1049 Example App.frx":15EB8
      TabIndex        =   59
      Top             =   360
      Width           =   660
   End
   Begin VB.ComboBox cboPolling1 
      Height          =   315
      ItemData        =   "Board1049 Example App.frx":15EDA
      Left            =   7800
      List            =   "Board1049 Example App.frx":15EF6
      TabIndex        =   56
      Top             =   360
      Width           =   660
   End
   Begin VB.CheckBox chkPlotMultipath 
      Caption         =   "Plot Multipath"
      Height          =   375
      Left            =   480
      Style           =   1  'Graphical
      TabIndex        =   55
      Top             =   4440
      Width           =   1335
   End
   Begin VB.CheckBox chkPlotChanResp 
      Caption         =   "Plot Channel Response"
      Height          =   375
      Left            =   9480
      Style           =   1  'Graphical
      TabIndex        =   54
      Top             =   4440
      Width           =   1935
   End
   Begin VB.CheckBox chkPlotEyeDiagram 
      Caption         =   "Plot Eye Diagram"
      Height          =   375
      Left            =   9720
      Style           =   1  'Graphical
      TabIndex        =   53
      Top             =   300
      Width           =   1455
   End
   Begin VB.CheckBox chkPlotConst 
      Caption         =   "Plot Constellation"
      Height          =   375
      Left            =   6120
      Style           =   1  'Graphical
      TabIndex        =   52
      Top             =   300
      Width           =   1455
   End
   Begin VB.Frame Frame2 
      Height          =   2670
      Left            =   1800
      TabIndex        =   30
      Top             =   1650
      Width           =   3900
      Begin VB.CommandButton btnBitErrTests 
         Caption         =   "Start Bit Err Tests"
         Height          =   375
         Left            =   1200
         TabIndex        =   33
         Top             =   180
         Width           =   1575
      End
      Begin VB.TextBox txtBurstErrs 
         Alignment       =   2  'Center
         Height          =   285
         Left            =   1920
         TabIndex        =   32
         Text            =   "10"
         Top             =   2280
         Width           =   615
      End
      Begin VB.Timer tmrBer 
         Enabled         =   0   'False
         Interval        =   1000
         Left            =   200
         Top             =   120
      End
      Begin VB.TextBox txtWindowSeconds 
         Alignment       =   2  'Center
         Height          =   285
         Left            =   900
         TabIndex        =   31
         Text            =   "10"
         Top             =   650
         Width           =   795
      End
      Begin VB.Label lblPreFec 
         Alignment       =   2  'Center
         BorderStyle     =   1  'Fixed Single
         Caption         =   "0"
         BeginProperty Font 
            Name            =   "MS Sans Serif"
            Size            =   9.75
            Charset         =   0
            Weight          =   400
            Underline       =   0   'False
            Italic          =   0   'False
            Strikethrough   =   0   'False
         EndProperty
         Height          =   330
         Left            =   900
         TabIndex        =   49
         Top             =   1080
         Width           =   915
      End
      Begin VB.Label lblLableArray 
         Alignment       =   1  'Right Justify
         AutoSize        =   -1  'True
         Caption         =   "PreFEC:"
         BeginProperty Font 
            Name            =   "MS Sans Serif"
            Size            =   9.75
            Charset         =   0
            Weight          =   400
            Underline       =   0   'False
            Italic          =   0   'False
            Strikethrough   =   0   'False
         EndProperty
         Height          =   240
         Index           =   8
         Left            =   75
         TabIndex        =   48
         Top             =   1110
         Width           =   750
      End
      Begin VB.Label lblSer 
         Alignment       =   2  'Center
         BorderStyle     =   1  'Fixed Single
         Caption         =   "0"
         BeginProperty Font 
            Name            =   "MS Sans Serif"
            Size            =   9.75
            Charset         =   0
            Weight          =   400
            Underline       =   0   'False
            Italic          =   0   'False
            Strikethrough   =   0   'False
         EndProperty
         Height          =   330
         Left            =   900
         TabIndex        =   47
         Top             =   1470
         Width           =   915
      End
      Begin VB.Label lblLableArray 
         Alignment       =   1  'Right Justify
         AutoSize        =   -1  'True
         Caption         =   "SER:"
         BeginProperty Font 
            Name            =   "MS Sans Serif"
            Size            =   9.75
            Charset         =   0
            Weight          =   400
            Underline       =   0   'False
            Italic          =   0   'False
            Strikethrough   =   0   'False
         EndProperty
         Height          =   240
         Index           =   9
         Left            =   75
         TabIndex        =   46
         Top             =   1485
         Width           =   465
      End
      Begin VB.Label lblErrSec 
         Alignment       =   2  'Center
         BorderStyle     =   1  'Fixed Single
         Caption         =   "0"
         BeginProperty Font 
            Name            =   "MS Sans Serif"
            Size            =   9.75
            Charset         =   0
            Weight          =   400
            Underline       =   0   'False
            Italic          =   0   'False
            Strikethrough   =   0   'False
         EndProperty
         Height          =   330
         Left            =   900
         TabIndex        =   45
         Top             =   1860
         Width           =   915
      End
      Begin VB.Label lblLableArray 
         Alignment       =   1  'Right Justify
         AutoSize        =   -1  'True
         Caption         =   "ErrSec:"
         BeginProperty Font 
            Name            =   "MS Sans Serif"
            Size            =   9.75
            Charset         =   0
            Weight          =   400
            Underline       =   0   'False
            Italic          =   0   'False
            Strikethrough   =   0   'False
         EndProperty
         Height          =   240
         Index           =   10
         Left            =   75
         TabIndex        =   44
         Top             =   1860
         Width           =   660
      End
      Begin VB.Label lblPostFec 
         Alignment       =   2  'Center
         BorderStyle     =   1  'Fixed Single
         Caption         =   "0"
         BeginProperty Font 
            Name            =   "MS Sans Serif"
            Size            =   9.75
            Charset         =   0
            Weight          =   400
            Underline       =   0   'False
            Italic          =   0   'False
            Strikethrough   =   0   'False
         EndProperty
         Height          =   330
         Left            =   2895
         TabIndex        =   43
         Top             =   1095
         Width           =   915
      End
      Begin VB.Label lblLableArray 
         Alignment       =   1  'Right Justify
         AutoSize        =   -1  'True
         Caption         =   "PostFEC:"
         BeginProperty Font 
            Name            =   "MS Sans Serif"
            Size            =   9.75
            Charset         =   0
            Weight          =   400
            Underline       =   0   'False
            Italic          =   0   'False
            Strikethrough   =   0   'False
         EndProperty
         Height          =   240
         Index           =   11
         Left            =   1995
         TabIndex        =   42
         Top             =   1110
         Width           =   840
      End
      Begin VB.Label lblPn23 
         Alignment       =   2  'Center
         BorderStyle     =   1  'Fixed Single
         Caption         =   "0"
         BeginProperty Font 
            Name            =   "MS Sans Serif"
            Size            =   9.75
            Charset         =   0
            Weight          =   400
            Underline       =   0   'False
            Italic          =   0   'False
            Strikethrough   =   0   'False
         EndProperty
         Height          =   330
         Left            =   2895
         TabIndex        =   41
         Top             =   1470
         Width           =   915
      End
      Begin VB.Label lblLableArray 
         Alignment       =   1  'Right Justify
         AutoSize        =   -1  'True
         Caption         =   "PN-23:"
         BeginProperty Font 
            Name            =   "MS Sans Serif"
            Size            =   9.75
            Charset         =   0
            Weight          =   400
            Underline       =   0   'False
            Italic          =   0   'False
            Strikethrough   =   0   'False
         EndProperty
         Height          =   240
         Index           =   12
         Left            =   1995
         TabIndex        =   40
         Top             =   1485
         Width           =   600
      End
      Begin VB.Label lblBurstEs 
         Alignment       =   2  'Center
         BorderStyle     =   1  'Fixed Single
         Caption         =   "0"
         BeginProperty Font 
            Name            =   "MS Sans Serif"
            Size            =   9.75
            Charset         =   0
            Weight          =   400
            Underline       =   0   'False
            Italic          =   0   'False
            Strikethrough   =   0   'False
         EndProperty
         Height          =   330
         Left            =   2895
         TabIndex        =   39
         Top             =   1860
         Width           =   915
      End
      Begin VB.Label lblLableArray 
         Alignment       =   1  'Right Justify
         AutoSize        =   -1  'True
         Caption         =   "BurstES:"
         BeginProperty Font 
            Name            =   "MS Sans Serif"
            Size            =   9.75
            Charset         =   0
            Weight          =   400
            Underline       =   0   'False
            Italic          =   0   'False
            Strikethrough   =   0   'False
         EndProperty
         Height          =   240
         Index           =   13
         Left            =   1995
         TabIndex        =   38
         Top             =   1860
         Width           =   765
      End
      Begin VB.Label lblLableArray 
         Alignment       =   2  'Center
         AutoSize        =   -1  'True
         Caption         =   "Burst Error =                   Errors"
         BeginProperty Font 
            Name            =   "MS Sans Serif"
            Size            =   9.75
            Charset         =   0
            Weight          =   400
            Underline       =   0   'False
            Italic          =   0   'False
            Strikethrough   =   0   'False
         EndProperty
         Height          =   240
         Index           =   14
         Left            =   720
         TabIndex        =   37
         Top             =   2280
         Width           =   2505
      End
      Begin VB.Label lblLableArray 
         Alignment       =   1  'Right Justify
         AutoSize        =   -1  'True
         Caption         =   "Window:"
         BeginProperty Font 
            Name            =   "MS Sans Serif"
            Size            =   9.75
            Charset         =   0
            Weight          =   400
            Underline       =   0   'False
            Italic          =   0   'False
            Strikethrough   =   0   'False
         EndProperty
         Height          =   240
         Index           =   15
         Left            =   75
         TabIndex        =   36
         Top             =   650
         Width           =   765
      End
      Begin VB.Label lblLableArray 
         Alignment       =   1  'Right Justify
         AutoSize        =   -1  'True
         Caption         =   "Elapsed:"
         BeginProperty Font 
            Name            =   "MS Sans Serif"
            Size            =   9.75
            Charset         =   0
            Weight          =   400
            Underline       =   0   'False
            Italic          =   0   'False
            Strikethrough   =   0   'False
         EndProperty
         Height          =   240
         Index           =   16
         Left            =   1995
         TabIndex        =   35
         Top             =   650
         Width           =   810
      End
      Begin VB.Label lblElapsedSeconds 
         Alignment       =   2  'Center
         BorderStyle     =   1  'Fixed Single
         Caption         =   "0"
         BeginProperty Font 
            Name            =   "MS Sans Serif"
            Size            =   9.75
            Charset         =   0
            Weight          =   400
            Underline       =   0   'False
            Italic          =   0   'False
            Strikethrough   =   0   'False
         EndProperty
         Height          =   300
         Left            =   2895
         TabIndex        =   34
         Top             =   650
         Width           =   795
      End
   End
   Begin VB.Frame Frame1 
      Height          =   1275
      Left            =   90
      TabIndex        =   21
      Top             =   0
      Width           =   5610
      Begin VB.ComboBox Acquire 
         Height          =   315
         ItemData        =   "Board1049 Example App.frx":15F18
         Left            =   240
         List            =   "Board1049 Example App.frx":15F37
         Style           =   2  'Dropdown List
         TabIndex        =   50
         Top             =   380
         Width           =   1335
      End
      Begin VB.CommandButton btnTune 
         Caption         =   "Tune"
         Height          =   375
         Left            =   2360
         TabIndex        =   25
         Top             =   800
         Width           =   855
      End
      Begin VB.TextBox txtFrequency 
         Alignment       =   1  'Right Justify
         Height          =   285
         Left            =   4200
         TabIndex        =   24
         Text            =   "57.000"
         Top             =   380
         Width           =   735
      End
      Begin VB.ComboBox cboChannelNumber 
         Height          =   315
         ItemData        =   "Board1049 Example App.frx":15FA1
         Left            =   3240
         List            =   "Board1049 Example App.frx":16071
         TabIndex        =   23
         Text            =   "Channel"
         Top             =   380
         Width           =   650
      End
      Begin VB.ComboBox cboChannelPlan 
         Height          =   315
         ItemData        =   "Board1049 Example App.frx":1617D
         Left            =   1920
         List            =   "Board1049 Example App.frx":1618D
         TabIndex        =   22
         Text            =   "Plan"
         Top             =   380
         Width           =   975
      End
      Begin VB.Label Label8 
         AutoSize        =   -1  'True
         Caption         =   "Mode"
         Height          =   195
         Left            =   720
         TabIndex        =   51
         Top             =   150
         Width           =   405
      End
      Begin VB.Label lblMHz 
         AutoSize        =   -1  'True
         Caption         =   "MHz"
         Height          =   195
         Left            =   4980
         TabIndex        =   29
         Top             =   420
         Width           =   330
      End
      Begin VB.Label lblChannelPlan 
         Alignment       =   2  'Center
         Caption         =   "Channel Plan"
         Height          =   255
         Left            =   1785
         TabIndex        =   28
         Top             =   150
         Width           =   1215
      End
      Begin VB.Label lblChannel 
         Alignment       =   2  'Center
         Caption         =   "Channel"
         Height          =   255
         Left            =   3120
         TabIndex        =   27
         Top             =   150
         Width           =   855
      End
      Begin VB.Label lblFrequency 
         Alignment       =   2  'Center
         Caption         =   "Frequency"
         Height          =   255
         Left            =   4080
         TabIndex        =   26
         Top             =   150
         Width           =   975
      End
   End
   Begin VB.Timer tmrChanResp 
      Enabled         =   0   'False
      Interval        =   2000
      Left            =   11520
      Top             =   4440
   End
   Begin VB.Timer tmrMultipath 
      Enabled         =   0   'False
      Interval        =   2000
      Left            =   6240
      Top             =   4440
   End
   Begin VB.Timer tmrEyeDiagram 
      Enabled         =   0   'False
      Interval        =   1500
      Left            =   9360
      Top             =   120
   End
   Begin VB.Timer tmrConstellation 
      Enabled         =   0   'False
      Interval        =   1500
      Left            =   5760
      Top             =   120
   End
   Begin VB.Timer tmrLevel 
      Enabled         =   0   'False
      Interval        =   750
      Left            =   0
      Top             =   1320
   End
   Begin VB.CheckBox ckbLogScale 
      Alignment       =   1  'Right Justify
      Caption         =   "Log Scale:"
      BeginProperty Font 
         Name            =   "MS Sans Serif"
         Size            =   9.75
         Charset         =   0
         Weight          =   400
         Underline       =   0   'False
         Italic          =   0   'False
         Strikethrough   =   0   'False
      EndProperty
      Height          =   255
      Left            =   4770
      TabIndex        =   17
      Top             =   4530
      Width           =   1275
   End
   Begin VB.Label lblLableArray 
      AutoSize        =   -1  'True
      Caption         =   "USB Voltage:"
      BeginProperty Font 
         Name            =   "MS Sans Serif"
         Size            =   9.75
         Charset         =   0
         Weight          =   400
         Underline       =   0   'False
         Italic          =   0   'False
         Strikethrough   =   0   'False
      EndProperty
      Height          =   240
      Index           =   2
      Left            =   3795
      TabIndex        =   66
      Top             =   1380
      Width           =   1215
   End
   Begin VB.Label lblUsbVoltage 
      AutoSize        =   -1  'True
      Caption         =   "0.00 V"
      Height          =   195
      Left            =   5085
      TabIndex        =   65
      Top             =   1410
      Width           =   465
   End
   Begin VB.Label Label2 
      Caption         =   "Sec"
      Height          =   255
      Left            =   12090
      TabIndex        =   61
      Top             =   405
      Width           =   375
   End
   Begin VB.Label lblPolling2 
      Alignment       =   2  'Center
      Caption         =   "Polling Interval"
      Height          =   255
      Left            =   11220
      TabIndex        =   60
      Top             =   120
      Width           =   1095
   End
   Begin VB.Label Label1 
      Caption         =   "Sec"
      Height          =   255
      Left            =   8490
      TabIndex        =   58
      Top             =   405
      Width           =   300
   End
   Begin VB.Label lblPollInt1 
      Alignment       =   2  'Center
      Caption         =   "Polling Interval"
      Height          =   255
      Left            =   7605
      TabIndex        =   57
      Top             =   120
      Width           =   1095
   End
   Begin VB.Label lblLableArray 
      AutoSize        =   -1  'True
      Caption         =   "dBm:"
      BeginProperty Font 
         Name            =   "MS Sans Serif"
         Size            =   9.75
         Charset         =   0
         Weight          =   400
         Underline       =   0   'False
         Italic          =   0   'False
         Strikethrough   =   0   'False
      EndProperty
      Height          =   240
      Index           =   17
      Left            =   315
      TabIndex        =   20
      Top             =   2310
      Width           =   465
   End
   Begin VB.Label lblChannelLeveldBm 
      Alignment       =   2  'Center
      BorderStyle     =   1  'Fixed Single
      Caption         =   "0"
      BeginProperty Font 
         Name            =   "MS Sans Serif"
         Size            =   9.75
         Charset         =   0
         Weight          =   400
         Underline       =   0   'False
         Italic          =   0   'False
         Strikethrough   =   0   'False
      EndProperty
      Height          =   330
      Left            =   840
      TabIndex        =   19
      Top             =   2280
      Width           =   825
   End
   Begin VB.Label lblLableArray 
      AutoSize        =   -1  'True
      Caption         =   "Equalizer Load:"
      BeginProperty Font 
         Name            =   "MS Sans Serif"
         Size            =   9.75
         Charset         =   0
         Weight          =   400
         Underline       =   0   'False
         Italic          =   0   'False
         Strikethrough   =   0   'False
      EndProperty
      Height          =   240
      Index           =   7
      Left            =   1980
      TabIndex        =   16
      Top             =   4530
      Width           =   1395
   End
   Begin VB.Label lblEqLoad 
      Alignment       =   2  'Center
      BorderStyle     =   1  'Fixed Single
      Caption         =   "0"
      BeginProperty Font 
         Name            =   "MS Sans Serif"
         Size            =   9.75
         Charset         =   0
         Weight          =   400
         Underline       =   0   'False
         Italic          =   0   'False
         Strikethrough   =   0   'False
      EndProperty
      Height          =   330
      Left            =   3450
      TabIndex        =   15
      Top             =   4470
      Width           =   1035
   End
   Begin VB.Label lblLableArray 
      AutoSize        =   -1  'True
      Caption         =   "EVM:"
      BeginProperty Font 
         Name            =   "MS Sans Serif"
         Size            =   9.75
         Charset         =   0
         Weight          =   400
         Underline       =   0   'False
         Italic          =   0   'False
         Strikethrough   =   0   'False
      EndProperty
      Height          =   240
      Index           =   6
      Left            =   300
      TabIndex        =   12
      Top             =   3960
      Width           =   480
   End
   Begin VB.Label lblLableArray 
      AutoSize        =   -1  'True
      Caption         =   "Margin:"
      BeginProperty Font 
         Name            =   "MS Sans Serif"
         Size            =   9.75
         Charset         =   0
         Weight          =   400
         Underline       =   0   'False
         Italic          =   0   'False
         Strikethrough   =   0   'False
      EndProperty
      Height          =   240
      Index           =   5
      Left            =   135
      TabIndex        =   11
      Top             =   3450
      Width           =   660
   End
   Begin VB.Label lblLableArray 
      AutoSize        =   -1  'True
      Caption         =   "MER:"
      BeginProperty Font 
         Name            =   "MS Sans Serif"
         Size            =   9.75
         Charset         =   0
         Weight          =   400
         Underline       =   0   'False
         Italic          =   0   'False
         Strikethrough   =   0   'False
      EndProperty
      Height          =   240
      Index           =   4
      Left            =   300
      TabIndex        =   10
      Top             =   3000
      Width           =   495
   End
   Begin VB.Label lblLableArray 
      AutoSize        =   -1  'True
      Caption         =   "dBmV:"
      BeginProperty Font 
         Name            =   "MS Sans Serif"
         Size            =   9.75
         Charset         =   0
         Weight          =   400
         Underline       =   0   'False
         Italic          =   0   'False
         Strikethrough   =   0   'False
      EndProperty
      Height          =   240
      Index           =   3
      Left            =   195
      TabIndex        =   9
      Top             =   1860
      Width           =   600
   End
   Begin VB.Label lblLableArray 
      AutoSize        =   -1  'True
      Caption         =   "Rcvr:"
      BeginProperty Font 
         Name            =   "MS Sans Serif"
         Size            =   9.75
         Charset         =   0
         Weight          =   400
         Underline       =   0   'False
         Italic          =   0   'False
         Strikethrough   =   0   'False
      EndProperty
      Height          =   240
      Index           =   1
      Left            =   2040
      TabIndex        =   8
      Top             =   1380
      Width           =   465
   End
   Begin VB.Label lblEvm 
      Alignment       =   2  'Center
      BorderStyle     =   1  'Fixed Single
      Caption         =   "0"
      BeginProperty Font 
         Name            =   "MS Sans Serif"
         Size            =   9.75
         Charset         =   0
         Weight          =   400
         Underline       =   0   'False
         Italic          =   0   'False
         Strikethrough   =   0   'False
      EndProperty
      Height          =   330
      Left            =   840
      TabIndex        =   7
      Top             =   3930
      Width           =   825
   End
   Begin VB.Label lblMargin 
      Alignment       =   2  'Center
      BorderStyle     =   1  'Fixed Single
      Caption         =   "0"
      BeginProperty Font 
         Name            =   "MS Sans Serif"
         Size            =   9.75
         Charset         =   0
         Weight          =   400
         Underline       =   0   'False
         Italic          =   0   'False
         Strikethrough   =   0   'False
      EndProperty
      Height          =   330
      Left            =   840
      TabIndex        =   6
      Top             =   3450
      Width           =   825
   End
   Begin VB.Label lblMer 
      Alignment       =   2  'Center
      BorderStyle     =   1  'Fixed Single
      Caption         =   "0"
      BeginProperty Font 
         Name            =   "MS Sans Serif"
         Size            =   9.75
         Charset         =   0
         Weight          =   400
         Underline       =   0   'False
         Italic          =   0   'False
         Strikethrough   =   0   'False
      EndProperty
      Height          =   330
      Left            =   840
      TabIndex        =   5
      Top             =   2970
      Width           =   825
   End
   Begin VB.Label lblChannelLeveldBmV 
      Alignment       =   2  'Center
      BorderStyle     =   1  'Fixed Single
      Caption         =   "0"
      BeginProperty Font 
         Name            =   "MS Sans Serif"
         Size            =   9.75
         Charset         =   0
         Weight          =   400
         Underline       =   0   'False
         Italic          =   0   'False
         Strikethrough   =   0   'False
      EndProperty
      Height          =   330
      Left            =   840
      TabIndex        =   4
      Top             =   1830
      Width           =   825
   End
   Begin VB.Label lblRcvrLockIndicator 
      Alignment       =   2  'Center
      BackColor       =   &H000000FF&
      BorderStyle     =   1  'Fixed Single
      Caption         =   "UnLocked"
      BeginProperty Font 
         Name            =   "MS Sans Serif"
         Size            =   9.75
         Charset         =   0
         Weight          =   400
         Underline       =   0   'False
         Italic          =   0   'False
         Strikethrough   =   0   'False
      EndProperty
      Height          =   300
      Left            =   2610
      TabIndex        =   2
      Top             =   1350
      Width           =   1035
   End
   Begin VB.Label lblFecLockIndicator 
      Alignment       =   2  'Center
      BackColor       =   &H000000FF&
      BorderStyle     =   1  'Fixed Single
      Caption         =   "UnLocked"
      BeginProperty Font 
         Name            =   "MS Sans Serif"
         Size            =   9.75
         Charset         =   0
         Weight          =   400
         Underline       =   0   'False
         Italic          =   0   'False
         Strikethrough   =   0   'False
      EndProperty
      Height          =   300
      Left            =   990
      TabIndex        =   1
      Top             =   1350
      Width           =   1035
   End
   Begin VB.Label lblLableArray 
      AutoSize        =   -1  'True
      Caption         =   "FEC:"
      BeginProperty Font 
         Name            =   "MS Sans Serif"
         Size            =   9.75
         Charset         =   0
         Weight          =   400
         Underline       =   0   'False
         Italic          =   0   'False
         Strikethrough   =   0   'False
      EndProperty
      Height          =   240
      Index           =   0
      Left            =   510
      TabIndex        =   0
      Top             =   1380
      Width           =   435
   End
   Begin VB.Menu mnuFile 
      Caption         =   "File"
      Begin VB.Menu mnuAdvanced 
         Caption         =   "Advanced Information"
      End
      Begin VB.Menu mnuChangeCard 
         Caption         =   "Change Card"
      End
   End
   Begin VB.Menu mnuRecord 
      Caption         =   "Record"
      Enabled         =   0   'False
      Begin VB.Menu mntRecordTRP 
         Caption         =   "Record TRP File"
      End
   End
   Begin VB.Menu mnuHelp 
      Caption         =   "Help"
      Begin VB.Menu mnuAbout 
         Caption         =   "About"
      End
   End
End
Attribute VB_Name = "frmMain"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = False
Attribute VB_PredeclaredId = True
Attribute VB_Exposed = False
Option Explicit
'*******************************************************************************
' File Name:      Board1049 Example App.frm
' Programmer:     Mike Golden
' Copyright:      SENCORE, Inc. 2001. All rights reserved.
' Creation Date:  8/16/2001
'
' Update History  Programmer     Description of modification
' --------------- -------------- ---------------------------------------
'
'*******************************************************************************




'*******************************************************************************
' Name:        Board1049_CommunicationError
' Desc:        Event that retrieves the ErrorMessage from the Board1049
' Pre:         None
' Post:        Message Box displays the Error.
' Programmer:  Mike Golden
'
' Update History   Programmer      Description of modification
' ---------------   --------------  ---------------------------------------
'
'*******************************************************************************
Private Sub Board1049_CommunicationError(ByVal ErrorMessage As String)
   MsgBox ErrorMessage, vbCritical, "Board1049 Error"
End Sub


Private Sub Board1049_RecordProgress(ByVal Percent As Long)
   frmRecord.lblPercent.Caption = Percent & "%"
   frmRecord.prgRecord.Value = Percent
   
End Sub

Private Sub Board1049_RecordStatus(ByVal Message As Board1049_STATUS_E)
   Select Case Message
      Case 0
         frmRecord.lblMessage.Caption = "Record Complete"
      Case 1
         frmRecord.lblMessage.Caption = "Error - No Lock"
      Case 2
         frmRecord.lblMessage.Caption = "Error - Lost Lock"
      Case 3
         frmRecord.lblMessage.Caption = "Error - Invalid Record Path"
      Case Else
         frmRecord.lblMessage.Caption = "Status Message " & Message
   End Select
   
End Sub

Private Sub btnBitErrTests_Click()
   Static bolErrTest As Boolean
   
   If bolErrTest = False Then
      bolErrTest = True
      btnBitErrTests.Caption = "Stop"
      tmrBer.Enabled = True
      Board1049.DemodGetStatus2
      
   Else
      bolErrTest = False
      btnBitErrTests.Caption = "Start Bit Err Tests"
      tmrBer.Enabled = False
      g_dblTotalSec = 0
      g_dblTotalBer = 0
      g_dblTotalUber = 0
      g_dblTotalPN23 = 0
      g_ErrSecsData = 0
      g_BurstErrSecsData = 0
      g_lngTotalPacketErrors = 0
      
   End If
   
End Sub

Private Sub btnTune_Click()
   Board1049.TunerFrequencyHz = Val(txtFrequency) * 1000000
   txtFrequency.Text = Format(txtFrequency.Text, "#0.000")
   tmrLevel.Enabled = True
End Sub


Private Sub cboChannelNumber_Click()
    txtFrequency.Text = Board1049.GetChannelFrequency((cboChannelPlan.ListIndex + 1), cboChannelNumber.List(cboChannelNumber.ListIndex)) / 1000000
    txtFrequency.Text = Format(txtFrequency.Text, "#0.000")
End Sub

Private Sub cboChannelPlan_Click()
    txtFrequency.Text = Board1049.GetChannelFrequency((cboChannelPlan.ListIndex + 1), cboChannelNumber.List(cboChannelNumber.ListIndex)) / 1000000
    txtFrequency.Text = Format(txtFrequency.Text, "#0.000")
End Sub

Private Sub cboPolling1_Click()
   tmrConstellation.Interval = cboPolling1.List(cboPolling1.ListIndex) * 1000
End Sub

Private Sub cboPolling2_Click()
   tmrEyeDiagram.Interval = cboPolling2.List(cboPolling2.ListIndex) * 1000
End Sub

Private Sub chkDisableEq_Click()
   If chkDisableEq.Value = vbChecked Then
      Board1049.DemodSetAutoReAcq False
      Board1049.DemodDisableEq True
   Else
      Board1049.DemodDisableEq False
      Board1049.DemodSetAutoReAcq True
   End If
End Sub

Private Sub chkFreezeDFE_Click()
   If chkFreezeDFE.Value = vbChecked Then
      Board1049.DemodFreezeDFE True
   Else
      Board1049.DemodFreezeDFE False
   End If
End Sub

Private Sub chkFreezeFFE_Click()
   If chkFreezeFFE.Value = vbChecked Then
      Board1049.DemodFreezeFFE True
   Else
      Board1049.DemodFreezeFFE False
   End If
End Sub

Private Sub chkPlotChanResp_Click()
   If chkPlotChanResp.Value = vbChecked Then
            PlotChanResp
      tmrChanResp.Enabled = True
   Else
      tmrChanResp.Enabled = False
   End If

End Sub

Private Sub chkPlotConst_Click()
   If chkPlotConst.Value = vbChecked Then
      PlotSymbols
      tmrConstellation.Enabled = True
   Else
      tmrConstellation.Enabled = False
   End If

End Sub

Private Sub chkPlotEyeDiagram_Click()
   If chkPlotEyeDiagram.Value = vbChecked Then
      PlotEyeDiagram
      tmrEyeDiagram.Enabled = True
   Else
      tmrEyeDiagram.Enabled = False
   End If

End Sub

Private Sub chkPlotMultipath_Click()
   If chkPlotMultipath.Value = vbChecked Then
      PlotMultiPath
      tmrMultipath.Enabled = True
   Else
      tmrMultipath.Enabled = False
   End If

End Sub

Private Sub ckbLogScale_Click()
   tctMultipathTaps.Series(0).Clear
   If ckbLogScale.Value = 1 Then
      tctMultipathTaps.Axis.Left.SetMinMax -27, 0
      'tctMultipathTaps.Axis.Bottom.SetMinMax -3, 11
   Else
      tctMultipathTaps.Axis.Left.SetMinMax 0, 105
      'tctMultipathTaps.Axis.Left.SetMinMax 0, 16105
      'tctMultipathTaps.Axis.Left.Increment = 1000
      'tctMultipathTaps.Axis.Bottom.SetMinMax -6, 40
   End If
End Sub

Private Sub Command1_Click()
   Dim intEchoTaps(495) As Integer
   Dim i As Integer
   
   
   ' Get FFE and DFE Taps
   Board1049.DemodGetFFE intEchoTaps(0), 64
   Board1049.DemodGetDFE intEchoTaps(64), 432
   
   
   Open App.Path & "\test.dat" For Output As #1
   Write #1, "Tap", "Val"
   
   For i = 0 To 495
      Write #1, i, intEchoTaps(i)
   Next i
   
   Close #1
   
End Sub

Private Sub Form_Load()
Dim bolIniTest As Boolean

   bolIniTest = True
   
   For intCardNumber = 0 To 5
      lngXilinxVer(intCardNumber) = -1
   Next intCardNumber
   
   intCardNumber = 0

   Do While bolIniTest = True
      If Board1049.Initialize(intCardNumber) = True Then
         frmChangeCard.cboCardIndex.List(intCardNumber) = intCardNumber + 1
         Board1049.Versions lngOCXVer, lngDriverVer, lngXilinxVer(intCardNumber)
         Board1049.Uninitialize
         intCardNumber = intCardNumber + 1
      Else
         bolIniTest = False
      End If
   Loop
   
   'Me.Show
   
   If intCardNumber > 1 Then
      frmChangeCard.Caption = "Choose Card"
      frmChangeCard.btnCancel.Visible = False
      frmChangeCard.btnOK.Left = 1250
      Call frmChangeCard.Show(vbModal, frmMain)
   Else
      intCurrentCard = 0
      InitializeBoard1049 0
      frmMain.Caption = "Board 1049 Example App - Card 1"
      mnuChangeCard.Enabled = False
   End If
   
   cboPolling1.ListIndex = 5
   cboPolling2.ListIndex = 5
      
End Sub

Private Sub Form_Unload(Cancel As Integer)

'Uninitialize the Board1049
   UninitializeBoard1049
   End
   End
End Sub

Private Sub mntRecordTRP_Click()
   Call frmRecord.Show
End Sub

Private Sub mnuAbout_Click()
   Call frmAbout.Show(vbModal, frmMain)
End Sub

Private Sub mnuAdvanced_Click()
   Call frmAdvanced.Show
End Sub

Private Sub mnuChangeCard_Click()
Dim i As Integer
   For i = 1 To intCardNumber
      frmChangeCard.cboCardIndex.List(intCardNumber - 1) = intCardNumber
   Next i
   
   Call frmChangeCard.Show(vbModal, frmMain)
End Sub


Private Sub tmrBer_Timer()
   GetBerData
End Sub

Private Sub tmrChanResp_Timer()
   PlotChanResp
End Sub

Private Sub tmrConstellation_Timer()
   PlotSymbols
End Sub

Private Sub tmrEyeDiagram_Timer()
   PlotEyeDiagram
End Sub

Private Sub tmrLevel_Timer()
   ChannelLevel
   MerMarginEvm
   CheckLock
   tmrLevel.Enabled = True
   lblUsbVoltage = Format(Board1049.AdcVoltage / 500, "0.00") & " V"
End Sub

Private Sub Acquire_Click()
    ModeAcquire Acquire.ListIndex
End Sub

Private Sub tmrMultipath_Timer()
   PlotMultiPath
End Sub

