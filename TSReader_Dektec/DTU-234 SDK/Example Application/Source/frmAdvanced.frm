VERSION 5.00
Object = "{F9043C88-F6F2-101A-A3C9-08002B2F49FB}#1.2#0"; "comdlg32.ocx"
Begin VB.Form frmAdvanced 
   Caption         =   "Advanced Information"
   ClientHeight    =   4365
   ClientLeft      =   60
   ClientTop       =   450
   ClientWidth     =   6705
   LinkTopic       =   "Form1"
   MaxButton       =   0   'False
   ScaleHeight     =   4365
   ScaleWidth      =   6705
   StartUpPosition =   3  'Windows Default
   Begin VB.Timer tmrChkAtten 
      Enabled         =   0   'False
      Interval        =   1000
      Left            =   1560
      Top             =   2280
   End
   Begin VB.TextBox txtDACCounts 
      Alignment       =   1  'Right Justify
      Height          =   285
      Left            =   5800
      TabIndex        =   40
      Text            =   "0"
      Top             =   3840
      Width           =   615
   End
   Begin VB.CommandButton btnSetDAC 
      Caption         =   "Set DAC"
      Height          =   375
      Left            =   4680
      TabIndex        =   39
      Top             =   3795
      Width           =   975
   End
   Begin VB.CheckBox chkAtten 
      Caption         =   "Attenuater"
      Height          =   375
      Left            =   120
      Style           =   1  'Graphical
      TabIndex        =   2
      Top             =   1560
      Width           =   1095
   End
   Begin MSComDlg.CommonDialog CommonDialog1 
      Left            =   1560
      Top             =   1440
      _ExtentX        =   847
      _ExtentY        =   847
      _Version        =   393216
   End
   Begin VB.CommandButton btnStdHex 
      Caption         =   "Load Standard Hex File"
      Height          =   375
      Left            =   120
      TabIndex        =   1
      Top             =   840
      Width           =   1815
   End
   Begin VB.CommandButton btnAltHex 
      Caption         =   "Load Alternate Hex File"
      Height          =   375
      Left            =   120
      TabIndex        =   0
      Top             =   120
      Width           =   1815
   End
   Begin VB.Frame Frame1 
      Height          =   3615
      Left            =   2160
      TabIndex        =   3
      Top             =   0
      Width           =   4455
      Begin VB.TextBox txtVoltAgc 
         Alignment       =   1  'Right Justify
         Height          =   285
         Left            =   615
         TabIndex        =   45
         Text            =   "0"
         Top             =   2880
         Width           =   615
      End
      Begin VB.TextBox txtVoltVolts 
         Alignment       =   1  'Right Justify
         Height          =   285
         Left            =   1335
         TabIndex        =   44
         Text            =   "0"
         Top             =   2880
         Width           =   615
      End
      Begin VB.TextBox txtVoltDelta 
         Alignment       =   1  'Right Justify
         Height          =   285
         Left            =   3660
         TabIndex        =   43
         Text            =   "0"
         Top             =   2880
         Width           =   615
      End
      Begin VB.TextBox txtVoltMax 
         Alignment       =   1  'Right Justify
         Height          =   285
         Left            =   2970
         TabIndex        =   42
         Text            =   "0"
         Top             =   2880
         Width           =   615
      End
      Begin VB.TextBox txtVoltMin 
         Alignment       =   1  'Right Justify
         Height          =   285
         Left            =   2295
         TabIndex        =   41
         Text            =   "0"
         Top             =   2880
         Width           =   615
      End
      Begin VB.CommandButton bntClear 
         Caption         =   "Clear"
         Height          =   255
         Left            =   3600
         TabIndex        =   32
         Top             =   3240
         Width           =   735
      End
      Begin VB.TextBox txtTunerMin 
         Alignment       =   1  'Right Justify
         Height          =   285
         Left            =   2295
         TabIndex        =   31
         Text            =   "0"
         Top             =   1080
         Width           =   615
      End
      Begin VB.TextBox txtTunerMax 
         Alignment       =   1  'Right Justify
         Height          =   285
         Left            =   2970
         TabIndex        =   30
         Text            =   "0"
         Top             =   1080
         Width           =   615
      End
      Begin VB.TextBox txtTunerDelta 
         Alignment       =   1  'Right Justify
         Height          =   285
         Left            =   3660
         TabIndex        =   29
         Text            =   "0"
         Top             =   1080
         Width           =   615
      End
      Begin VB.TextBox txtNbMin 
         Alignment       =   1  'Right Justify
         Height          =   285
         Left            =   2295
         TabIndex        =   28
         Text            =   "0"
         Top             =   1440
         Width           =   615
      End
      Begin VB.TextBox txtNbMax 
         Alignment       =   1  'Right Justify
         Height          =   285
         Left            =   2970
         TabIndex        =   27
         Text            =   "0"
         Top             =   1440
         Width           =   615
      End
      Begin VB.TextBox txtNbDelta 
         Alignment       =   1  'Right Justify
         Height          =   285
         Left            =   3660
         TabIndex        =   26
         Text            =   "0"
         Top             =   1440
         Width           =   615
      End
      Begin VB.TextBox txtWbMin 
         Alignment       =   1  'Right Justify
         Height          =   285
         Left            =   2295
         TabIndex        =   25
         Text            =   "0"
         Top             =   1800
         Width           =   615
      End
      Begin VB.TextBox txtWbMax 
         Alignment       =   1  'Right Justify
         Height          =   285
         Left            =   2970
         TabIndex        =   24
         Text            =   "0"
         Top             =   1800
         Width           =   615
      End
      Begin VB.TextBox txtWbDelta 
         Alignment       =   1  'Right Justify
         Height          =   285
         Left            =   3660
         TabIndex        =   23
         Text            =   "0"
         Top             =   1800
         Width           =   615
      End
      Begin VB.TextBox txtFineMin 
         Alignment       =   1  'Right Justify
         Height          =   285
         Left            =   2295
         TabIndex        =   22
         Text            =   "0"
         Top             =   2160
         Width           =   615
      End
      Begin VB.TextBox txtFineMax 
         Alignment       =   1  'Right Justify
         Height          =   285
         Left            =   2970
         TabIndex        =   21
         Text            =   "0"
         Top             =   2160
         Width           =   615
      End
      Begin VB.TextBox txtFineDelta 
         Alignment       =   1  'Right Justify
         Height          =   285
         Left            =   3660
         TabIndex        =   20
         Text            =   "0"
         Top             =   2160
         Width           =   615
      End
      Begin VB.TextBox txtTempMin 
         Alignment       =   1  'Right Justify
         Height          =   285
         Left            =   2295
         TabIndex        =   19
         Text            =   "0"
         Top             =   2520
         Width           =   615
      End
      Begin VB.TextBox txtTempMax 
         Alignment       =   1  'Right Justify
         Height          =   285
         Left            =   2970
         TabIndex        =   18
         Text            =   "0"
         Top             =   2520
         Width           =   615
      End
      Begin VB.TextBox txtTempDelta 
         Alignment       =   1  'Right Justify
         Height          =   285
         Left            =   3660
         TabIndex        =   17
         Text            =   "0"
         Top             =   2520
         Width           =   615
      End
      Begin VB.TextBox txtNumberAverages 
         Height          =   285
         Left            =   3660
         TabIndex        =   16
         Text            =   "10"
         Top             =   480
         Width           =   615
      End
      Begin VB.CommandButton btnADCAverage 
         Caption         =   "A/D Averages"
         Height          =   375
         Left            =   1920
         TabIndex        =   15
         Top             =   360
         Width           =   1215
      End
      Begin VB.TextBox txtTempVolts 
         Alignment       =   1  'Right Justify
         Height          =   285
         Left            =   1335
         TabIndex        =   14
         Text            =   "0"
         Top             =   2520
         Width           =   615
      End
      Begin VB.TextBox txtTempAgc 
         Alignment       =   1  'Right Justify
         Height          =   285
         Left            =   615
         TabIndex        =   13
         Text            =   "0"
         Top             =   2520
         Width           =   615
      End
      Begin VB.TextBox txtFineVolts 
         Alignment       =   1  'Right Justify
         Height          =   285
         Left            =   1335
         TabIndex        =   12
         Text            =   "0"
         Top             =   2160
         Width           =   615
      End
      Begin VB.TextBox txtFineAgc 
         Alignment       =   1  'Right Justify
         Height          =   285
         Left            =   615
         TabIndex        =   11
         Text            =   "0"
         Top             =   2160
         Width           =   615
      End
      Begin VB.TextBox txtWbVolts 
         Alignment       =   1  'Right Justify
         Height          =   285
         Left            =   1335
         TabIndex        =   10
         Text            =   "0"
         Top             =   1800
         Width           =   615
      End
      Begin VB.TextBox txtWbAgc 
         Alignment       =   1  'Right Justify
         Height          =   285
         Left            =   615
         TabIndex        =   9
         Text            =   "0"
         Top             =   1800
         Width           =   615
      End
      Begin VB.TextBox txtNbVolts 
         Alignment       =   1  'Right Justify
         Height          =   285
         Left            =   1335
         TabIndex        =   8
         Text            =   "0"
         Top             =   1425
         Width           =   615
      End
      Begin VB.TextBox txtNbAgc 
         Alignment       =   1  'Right Justify
         Height          =   285
         Left            =   615
         TabIndex        =   7
         Text            =   "0"
         Top             =   1425
         Width           =   615
      End
      Begin VB.TextBox txtTunerVolts 
         Alignment       =   1  'Right Justify
         Height          =   285
         Left            =   1335
         TabIndex        =   6
         Text            =   "0"
         Top             =   1080
         Width           =   615
      End
      Begin VB.TextBox txtTunerAgc 
         Alignment       =   1  'Right Justify
         Height          =   285
         Left            =   615
         TabIndex        =   5
         Text            =   "0"
         Top             =   1080
         Width           =   615
      End
      Begin VB.CommandButton btnADC 
         Caption         =   "Read A/D"
         Height          =   375
         Left            =   360
         TabIndex        =   4
         Top             =   360
         Width           =   1215
      End
      Begin VB.Label LabelArr 
         AutoSize        =   -1  'True
         Caption         =   "Volt"
         Height          =   195
         Index           =   0
         Left            =   230
         TabIndex        =   51
         Top             =   2910
         Width           =   270
      End
      Begin VB.Label LabelArr 
         AutoSize        =   -1  'True
         Caption         =   "Tuner"
         Height          =   195
         Index           =   5
         Left            =   120
         TabIndex        =   50
         Top             =   1125
         Width           =   420
      End
      Begin VB.Label LabelArr 
         AutoSize        =   -1  'True
         Caption         =   "Nb"
         Height          =   195
         Index           =   6
         Left            =   300
         TabIndex        =   49
         Top             =   1470
         Width           =   210
      End
      Begin VB.Label LabelArr 
         AutoSize        =   -1  'True
         Caption         =   "Wb"
         Height          =   195
         Index           =   7
         Left            =   260
         TabIndex        =   48
         Top             =   1840
         Width           =   255
      End
      Begin VB.Label LabelArr 
         AutoSize        =   -1  'True
         Caption         =   "Fine"
         Height          =   195
         Index           =   8
         Left            =   200
         TabIndex        =   47
         Top             =   2200
         Width           =   300
      End
      Begin VB.Label LabelArr 
         AutoSize        =   -1  'True
         Caption         =   "Temp"
         Height          =   195
         Index           =   9
         Left            =   95
         TabIndex        =   46
         Top             =   2550
         Width           =   405
      End
      Begin VB.Label LabelArr 
         AutoSize        =   -1  'True
         Caption         =   "Delta"
         Height          =   195
         Index           =   15
         Left            =   3780
         TabIndex        =   38
         Top             =   840
         Width           =   375
      End
      Begin VB.Label LabelArr 
         AutoSize        =   -1  'True
         Caption         =   "Max"
         Height          =   195
         Index           =   14
         Left            =   3120
         TabIndex        =   37
         Top             =   840
         Width           =   300
      End
      Begin VB.Label LabelArr 
         AutoSize        =   -1  'True
         Caption         =   "Min"
         Height          =   195
         Index           =   13
         Left            =   2480
         TabIndex        =   36
         Top             =   840
         Width           =   255
      End
      Begin VB.Label LabelArr 
         AutoSize        =   -1  'True
         Caption         =   "Avgs"
         Height          =   195
         Index           =   4
         Left            =   3760
         TabIndex        =   35
         Top             =   240
         Width           =   360
      End
      Begin VB.Label LabelArr 
         AutoSize        =   -1  'True
         Caption         =   "Volts"
         Height          =   195
         Index           =   11
         Left            =   1455
         TabIndex        =   34
         Top             =   840
         Width           =   345
      End
      Begin VB.Label LabelArr 
         AutoSize        =   -1  'True
         Caption         =   "Counts"
         Height          =   195
         Index           =   10
         Left            =   680
         TabIndex        =   33
         Top             =   840
         Width           =   495
      End
   End
End
Attribute VB_Name = "frmAdvanced"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = False
Attribute VB_PredeclaredId = True
Attribute VB_Exposed = False
Private Sub bntClear_Click()
   txtTunerMin = 0
   txtTunerMax = 0
   txtTunerDelta = 0
   txtNbMin = 0
   txtNbMax = 0
   txtNbDelta = 0
   txtWbMin = 0
   txtWbMax = 0
   txtWbDelta = 0
   txtFineMin = 0
   txtFineMax = 0
   txtFineDelta = 0
   txtTempMin = 0
   txtTempMax = 0
   txtTempDelta = 0
   txtVoltMin = 0
   txtVoltMax = 0
   txtVoltDelta = 0
End Sub

Private Sub btnADC_Click()
    txtTunerAgc = frmMain.Board1049.AdcTunerAGC
    txtTunerVolts = Format(txtTunerAgc / 1000, "#0.000")
    txtNbAgc = frmMain.Board1049.AdcNbRssi
    txtNbVolts = Format(txtNbAgc / 1000, "#0.000")
    txtWbAgc = frmMain.Board1049.AdcWbRssi
    txtWbVolts = Format(txtWbAgc / 1000, "#0.000")
    txtFineAgc = frmMain.Board1049.AdcFineAGC
    txtFineVolts = Format(txtFineAgc / 1000, "#0.000")
    txtTempAgc = frmMain.Board1049.AdcTemperature
    txtTempVolts = Format(txtTempAgc / 1000, "#0.000")
    txtVoltAgc = frmMain.Board1049.AdcVoltage
    txtVoltVolts = Format(txtVoltAgc / 1000, "#0.000")
End Sub

Private Sub btnADCAverage_Click()
   Dim intNumAvg As Integer
   Dim dblAvgCounts As Double
   
   intNumAvg = Val(txtNumberAverages.Text)
   
   dblAvgCounts = 0
   
   For i = 1 To intNumAvg
      dblAvgCounts = (frmMain.Board1049.AdcTunerAGC / intNumAvg) + dblAvgCounts
   Next i
   
   txtTunerAgc = Format(dblAvgCounts, "#0.0")
   txtTunerVolts = Format(dblAvgCounts / 1000, "#0.000")
   
   dblAvgCounts = 0
   
   For i = 1 To intNumAvg
      dblAvgCounts = (frmMain.Board1049.AdcNbRssi / intNumAvg) + dblAvgCounts
   Next i
   
   txtNbAgc = Format(dblAvgCounts, "#0.0")
   txtNbVolts = Format(dblAvgCounts / 1000, "#0.000")
   
   dblAvgCounts = 0
   
   For i = 1 To intNumAvg
      dblAvgCounts = (frmMain.Board1049.AdcWbRssi / intNumAvg) + dblAvgCounts
   Next i
   
   txtWbAgc = Format(dblAvgCounts, "#0.0")
   txtWbVolts = Format(dblAvgCounts / 1000, "#0.000")
   
   dblAvgCounts = 0
   
   For i = 1 To intNumAvg
      dblAvgCounts = (frmMain.Board1049.AdcFineAGC / intNumAvg) + dblAvgCounts
   Next i
   
   txtFineAgc = Format(dblAvgCounts, "#0.0")
   txtFineVolts = Format(dblAvgCounts / 1000, "#0.000")
   
   dblAvgCounts = 0
   
   For i = 1 To intNumAvg
      dblAvgCounts = (frmMain.Board1049.AdcTemperature / intNumAvg) + dblAvgCounts
   Next i
   
   txtTempAgc = Format(dblAvgCounts, "#0.0")
   txtTempVolts = Format(dblAvgCounts / 1000, "#0.000")
   
   dblAvgCounts = 0
   
   For i = 1 To intNumAvg
      dblAvgCounts = (frmMain.Board1049.AdcVoltage / intNumAvg) + dblAvgCounts
   Next i
   
   txtVoltAgc = Format(dblAvgCounts, "#0.0")
   txtVoltVolts = Format(dblAvgCounts / 1000, "#0.000")
End Sub

Private Sub btnAltHex_Click()
   
   CommonDialog1.Filter = "Hex Files (*.HEX)|*.hex;*.HEX"
   CommonDialog1.ShowOpen
   frmMain.Board1049.DemodSetProgram CommonDialog1.FileName
   CommonDialog1.FileName = " "

End Sub

Private Sub btnSetDAC_Click()
    frmMain.Board1049.DacSetAGC (Val(txtDACCounts))

End Sub

Private Sub btnStdHex_Click()
   
   frmMain.Board1049.DemodSetProgram App.Path & "\Config\setupinverted.hex"

End Sub

Private Sub chkAtten_Click()
    If chkAtten.Value = vbChecked Then
        frmMain.Board1049.IoRfAttenuation = True
    Else
        frmMain.Board1049.IoRfAttenuation = False
    End If

End Sub

Private Sub Form_Load()
   frmAdvanced.Caption = "Advanced Information - Card " & intCurrentCard
   Call tmrChkAtten_Timer
   tmrChkAtten.Enabled = True
End Sub

Private Sub Form_Unload(Cancel As Integer)
   tmrChkAtten.Enabled = False
End Sub

Private Sub tmrChkAtten_Timer()
   If frmMain.Board1049.IoRfAttenuation = True Then
      chkAtten.Value = vbChecked
   Else
      chkAtten.Value = vbUnchecked
   End If
End Sub

Private Sub txtNbAgc_Change()
   If Val(txtNbAgc.Text) < Val(txtNbMin.Text) Or Val(txtNbMin.Text) = 0 Then
      txtNbMin.Text = Format(txtNbAgc.Text, "#0.0")
   End If
   
   If Val(txtNbAgc.Text) > Val(txtNbMax.Text) Then
      txtNbMax.Text = Format(txtNbAgc.Text, "#0.0")
   End If
   
   txtNbDelta.Text = Format(Val(txtNbMax.Text) - Val(txtNbMin.Text), "#0.0")
End Sub

Private Sub txtTempAgc_Change()
   If Val(txtTempAgc.Text) < Val(txtTempMin.Text) Or Val(txtTempMin.Text) = 0 Then
      txtTempMin.Text = Format(txtTempAgc.Text, "#0.0")
   End If
   
   If Val(txtTempAgc.Text) > Val(txtTempMax.Text) Then
      txtTempMax.Text = Format(txtTempAgc.Text, "#0.0")
   End If
   
   txtTempDelta.Text = Format(Val(txtTempMax.Text) - Val(txtTempMin.Text), "#0.0")
End Sub

Private Sub txtTunerAgc_Change()
   If Val(txtTunerAgc.Text) < Val(txtTunerMin.Text) Or Val(txtTunerMin.Text) = 0 Then
      txtTunerMin.Text = Format(txtTunerAgc.Text, "#0.0")
   End If
   
   If Val(txtTunerAgc.Text) > Val(txtTunerMax.Text) Then
      txtTunerMax.Text = Format(txtTunerAgc.Text, "#0.0")
   End If
   
   txtTunerDelta.Text = Format(Val(txtTunerMax.Text) - Val(txtTunerMin.Text), "#0.0")
End Sub

Private Sub txtVoltAgc_Change()
   If Val(txtVoltAgc.Text) < Val(txtVoltMin.Text) Or Val(txtVoltMin.Text) = 0 Then
      txtVoltMin.Text = Format(txtVoltAgc.Text, "#0.0")
   End If
   
   If Val(txtVoltAgc.Text) > Val(txtVoltMax.Text) Then
      txtVoltMax.Text = Format(txtVoltAgc.Text, "#0.0")
   End If
   
   txtVoltDelta.Text = Format(Val(txtVoltMax.Text) - Val(txtVoltMin.Text), "#0.0")
End Sub

Private Sub txtWbAgc_Change()
   If Val(txtWbAgc.Text) < Val(txtWbMin.Text) Or Val(txtWbMin.Text) = 0 Then
      txtWbMin.Text = Format(txtWbAgc.Text, "#0.0")
   End If
   
   If Val(txtWbAgc.Text) > Val(txtWbMax.Text) Then
      txtWbMax.Text = Format(txtWbAgc.Text, "#0.0")
   End If
   
   txtWbDelta.Text = Format(Val(txtWbMax.Text) - Val(txtWbMin.Text), "#0.0")
End Sub

Private Sub txtFineAgc_Change()
   If Val(txtFineAgc.Text) < Val(txtFineMin.Text) Or Val(txtFineMin.Text) = 0 Then
      txtFineMin.Text = Format(txtFineAgc.Text, "#0.0")
   End If
   
   If Val(txtFineAgc.Text) > Val(txtFineMax.Text) Then
      txtFineMax.Text = Format(txtFineAgc.Text, "#0.0")
   End If
   
   txtFineDelta.Text = Format(Val(txtFineMax.Text) - Val(txtFineMin.Text), "#0.0")
End Sub


