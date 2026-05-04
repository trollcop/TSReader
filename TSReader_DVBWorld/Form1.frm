VERSION 5.00
Begin VB.Form Form1 
   Caption         =   "Form1"
   ClientHeight    =   3195
   ClientLeft      =   60
   ClientTop       =   345
   ClientWidth     =   4680
   LinkTopic       =   "Form1"
   ScaleHeight     =   3195
   ScaleWidth      =   4680
   StartUpPosition =   3  'Windows Default
   Begin VB.CommandButton Command1 
      Caption         =   "Command1"
      Height          =   975
      Left            =   360
      TabIndex        =   0
      Top             =   720
      Width           =   1335
   End
End
Attribute VB_Name = "Form1"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = False
Attribute VB_PredeclaredId = True
Attribute VB_Exposed = False
Private Sub Command1_Click()

'    Open "C:\Development\TSReader\TSReader_DVBWorld\dvbworld-download.xml" For Input As #1
    Open "C:\Development\TSReader\TSReader_DVBWorld\MatchBoxProFirmware.xml" For Input As #1
    farmed = False
    While EOF(1) = False
        Line Input #1, buffer$
        If farmed = False Then
            If InStr(buffer$, "<Record Number=" & Chr$(34) & "5096" & Chr$(34)) <> 0 Then
                farmed = True
            End If
        Else
            If InStr(buffer$, "<Record Number=" & Chr$(34) & "5651" & Chr$(34)) <> 0 Then
                farmed = False
            End If
            nPos = InStr(buffer$, "<TransferBuffer>")
            If nPos <> 0 Then
                nPos = InStr(buffer$, ">")
                nPos2 = InStr(nPos, buffer$, "<")
                firmware$ = Mid$(buffer$, nPos + 1, nPos2 - nPos - 1)
                outbuf$ = ""
                For i = 1 To Len(firmware) Step 2
                    outbuf$ = outbuf$ & "0x" & Mid$(firmware$, i, 2) & ", "
                Next
                Debug.Print outbuf$
            End If
        End If
    Wend
        
    Close #1
    MsgBox "done"
    
End Sub
