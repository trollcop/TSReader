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
   Begin VB.TextBox key 
      Height          =   285
      Left            =   1080
      TabIndex        =   4
      Top             =   600
      Width           =   1455
   End
   Begin VB.TextBox prefix 
      Height          =   285
      Left            =   1080
      TabIndex        =   3
      Text            =   "sky"
      Top             =   120
      Width           =   1455
   End
   Begin VB.CommandButton Command1 
      Caption         =   "Process key"
      Height          =   855
      Left            =   3240
      TabIndex        =   0
      Top             =   120
      Width           =   975
   End
   Begin VB.Label Label3 
      Caption         =   "Capture data on D7 from ANT at 20 us"
      Height          =   1335
      Left            =   240
      TabIndex        =   5
      Top             =   1320
      Width           =   4215
   End
   Begin VB.Label Label2 
      Caption         =   "Prefix"
      Height          =   255
      Left            =   240
      TabIndex        =   2
      Top             =   120
      Width           =   615
   End
   Begin VB.Label Label1 
      Caption         =   "Key"
      Height          =   255
      Left            =   240
      TabIndex        =   1
      Top             =   600
      Width           =   495
   End
End
Attribute VB_Name = "Form1"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = False
Attribute VB_PredeclaredId = True
Attribute VB_Exposed = False
Private Sub Command1_Click()

Dim fStart As Boolean
Dim nBit As Integer
Dim nBitCounter As Integer
Dim fPastStart As Boolean

    nBit = -1
    nBitCounter = 0
    fPastStart = False
    
    Open "C:\Documents and Settings\rod\Desktop\test.rld" For Input As #1
    Do
        Line Input #1, buffer$
        If fStart = False Then
            If buffer$ = "[Data]" Then
                fStart = True
            End If
        Else
            nInputBit = Val(Mid$(buffer$, InStr(buffer$, "=") + 1, Len(buffer$))) And 128
            If nBit = -1 Then
                nBit = nInputBit And 128
            Else
                If nBit <> nInputBit Then
                    Debug.Print "bit = " & nBit & " count = " & nBitCounter
                    If fPastStart = False Then
                        If nBitCounter > 100 And nBit = 0 Then
                            fPastStart = True
                        End If
                    Else
                        If nBitCounter > 60 Then
                            Debug.Print "> 60"
                        Else
                            If nBitCounter > 30 Then
                                If nBit = 128 Then
                                    bitstring$ = bitstring$ + "11"
                                Else
                                    bitstring$ = bitstring$ + "00"
                                End If
                            Else
                                If nBit = 128 Then
                                    bitstring$ = bitstring$ + "1"
                                Else
                                    bitstring$ = bitstring$ + "0"
                                End If
                            End If
                        End If
                    End If
                        
                    nBit = nInputBit
                    nBitCounter = 0
                Else
                    nBitCounter = nBitCounter + 1
                End If
            End If
        End If
    Loop While (EOF(1) = False)
    nsamplebitcount = Len(bitstring$)
    Do While Len(bitstring$) < 56
        bitstring$ = bitstring$ + "1"
    Loop
    Debug.Print bitstring$ & " len = " & Len(bitstring$) & " org len = " & nsamplebitcount
    For i = 1 To 7
        nByteValue = 0
        If Mid$(bitstring, (i - 1) * 8 + 1, 1) = "1" Then nByteValue = nByteValue + 128
        If Mid$(bitstring, (i - 1) * 8 + 2, 1) = "1" Then nByteValue = nByteValue + 64
        If Mid$(bitstring, (i - 1) * 8 + 3, 1) = "1" Then nByteValue = nByteValue + 32
        If Mid$(bitstring, (i - 1) * 8 + 4, 1) = "1" Then nByteValue = nByteValue + 16
        If Mid$(bitstring, (i - 1) * 8 + 5, 1) = "1" Then nByteValue = nByteValue + 8
        If Mid$(bitstring, (i - 1) * 8 + 6, 1) = "1" Then nByteValue = nByteValue + 4
        If Mid$(bitstring, (i - 1) * 8 + 7, 1) = "1" Then nByteValue = nByteValue + 2
        If Mid$(bitstring, (i - 1) * 8 + 8, 1) = "1" Then nByteValue = nByteValue + 1
        hexstring$ = hexstring$ & "0x" & LCase$(Hex$(nByteValue))
        If i < 7 Then hexstring$ = hexstring$ & ", "
    Next
    Debug.Print hexstring$
    
    Print #2, "BYTE " & prefix.Text & "_" & key.Text & " = {" & hexstring$ & "}; // " & bitstring$
    
    
    Close #1
End Sub

Private Sub Form_Load()

    Open "c:\remote.txt" For Append As #2
    
End Sub

Private Sub Form_Unload(Cancel As Integer)

    Close #2
    
End Sub
