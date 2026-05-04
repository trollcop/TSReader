VERSION 5.00
Object = "{F9043C88-F6F2-101A-A3C9-08002B2F49FB}#1.2#0"; "comdlg32.ocx"
Object = "{831FDD16-0C5C-11D2-A9FC-0000F8754DA1}#2.0#0"; "mscomctl.ocx"
Begin VB.Form frmRecord 
   Caption         =   "Record TRP File"
   ClientHeight    =   3330
   ClientLeft      =   60
   ClientTop       =   450
   ClientWidth     =   5535
   LinkTopic       =   "Form1"
   ScaleHeight     =   3330
   ScaleWidth      =   5535
   StartUpPosition =   3  'Windows Default
   Begin VB.CommandButton btnCancel 
      Caption         =   "Cancel"
      BeginProperty Font 
         Name            =   "MS Sans Serif"
         Size            =   12
         Charset         =   0
         Weight          =   700
         Underline       =   0   'False
         Italic          =   0   'False
         Strikethrough   =   0   'False
      EndProperty
      Height          =   495
      Left            =   3720
      TabIndex        =   10
      Top             =   1200
      Width           =   1455
   End
   Begin VB.TextBox txtRecordLength 
      Alignment       =   1  'Right Justify
      Height          =   285
      Left            =   120
      TabIndex        =   7
      Text            =   "1.0"
      Top             =   1320
      Width           =   975
   End
   Begin MSComctlLib.ProgressBar prgRecord 
      Height          =   495
      Left            =   360
      TabIndex        =   5
      Top             =   2040
      Width           =   3735
      _ExtentX        =   6588
      _ExtentY        =   873
      _Version        =   393216
      Appearance      =   1
      Scrolling       =   1
   End
   Begin VB.CommandButton btnRecord 
      Caption         =   "Record"
      BeginProperty Font 
         Name            =   "MS Sans Serif"
         Size            =   12
         Charset         =   0
         Weight          =   700
         Underline       =   0   'False
         Italic          =   0   'False
         Strikethrough   =   0   'False
      EndProperty
      Height          =   495
      Left            =   1920
      TabIndex        =   3
      Top             =   1200
      Width           =   1455
   End
   Begin MSComDlg.CommonDialog CommonDialog1 
      Left            =   4920
      Top             =   0
      _ExtentX        =   847
      _ExtentY        =   847
      _Version        =   393216
   End
   Begin VB.CommandButton btnBrowse 
      Caption         =   "Browse"
      Height          =   375
      Left            =   4200
      TabIndex        =   1
      Top             =   450
      Width           =   1215
   End
   Begin VB.TextBox txtRecordPath 
      Height          =   285
      Left            =   120
      TabIndex        =   0
      Top             =   480
      Width           =   3975
   End
   Begin VB.Label lblMinutes 
      Caption         =   "Min"
      Height          =   255
      Left            =   1200
      TabIndex        =   9
      Top             =   1360
      Width           =   735
   End
   Begin VB.Label lblRecordLength 
      Caption         =   "Record Length"
      Height          =   255
      Left            =   120
      TabIndex        =   8
      Top             =   1080
      Width           =   1335
   End
   Begin VB.Label lblMessage 
      Alignment       =   2  'Center
      BorderStyle     =   1  'Fixed Single
      BeginProperty Font 
         Name            =   "MS Sans Serif"
         Size            =   12
         Charset         =   0
         Weight          =   700
         Underline       =   0   'False
         Italic          =   0   'False
         Strikethrough   =   0   'False
      EndProperty
      Height          =   375
      Left            =   840
      TabIndex        =   6
      Top             =   2760
      Width           =   3735
   End
   Begin VB.Label lblPercent 
      Caption         =   "0%"
      BeginProperty Font 
         Name            =   "MS Sans Serif"
         Size            =   12
         Charset         =   0
         Weight          =   700
         Underline       =   0   'False
         Italic          =   0   'False
         Strikethrough   =   0   'False
      EndProperty
      Height          =   375
      Left            =   4320
      TabIndex        =   4
      Top             =   2115
      Width           =   735
   End
   Begin VB.Label lblRecordPath 
      Caption         =   "Record Path"
      Height          =   255
      Left            =   120
      TabIndex        =   2
      Top             =   240
      Width           =   1215
   End
End
Attribute VB_Name = "frmRecord"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = False
Attribute VB_PredeclaredId = True
Attribute VB_Exposed = False
Private Sub btnBrowse_Click()
   CommonDialog1.Filter = "Transport Stream Files (*.TRP)|*.trp;*.TRP"
   CommonDialog1.ShowSave
   txtRecordPath.Text = CommonDialog1.FileName
   
End Sub

Private Sub btnCancel_Click()
   frmMain.Board1049.ExecuteStop
   lblPercent.Caption = "0%"
   prgRecord.Value = 0
   lblMessage.Caption = "Recording Canceled"
   
End Sub

Private Sub btnRecord_Click()
Dim lngPackets As Long

   lblMessage.Caption = "Recording..."
   txtRecordLength.Text = Format(txtRecordLength.Text, "#0.0")
   If Val(txtRecordLength.Text) > 999 Then
      txtRecordLength.Text = "999.0"
   End If
   lngPackets = (((Val(txtRecordLength.Text) * 60 * 19392658) / 8) / 188)
   Call frmMain.Board1049.ExecuteRecord(txtRecordPath.Text, lngPackets)
   
End Sub

Private Sub Form_Load()
   frmRecord.Caption = "Record TRP File - Card " & intCurrentCard
End Sub
