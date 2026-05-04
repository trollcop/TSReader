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
      Text            =   "dish"
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
      Caption         =   "Capture from ANT8 at 100us resolution"
      Height          =   975
      Left            =   360
      TabIndex        =   5
      Top             =   1440
      Width           =   3735
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
Dim nState As Integer
Dim nInputBit As Integer
Dim nBytes As Integer

    nBitCounter = 0
    fPastStart = False
    nState = 0
    nBytes = 164
    
    Open "C:\Documents and Settings\rod\Desktop\ant8rld" For Input As #1
    Do
        Line Input #1, buffer$
        If fStart = False Then
            If buffer$ = "[Data]" Then
                fStart = True
            End If
        Else
            nInputBit = Val(Mid$(buffer$, InStr(buffer$, "=") + 1, Len(buffer$))) And 1
            If nState = 0 Then
                If nInputBit = 1 Then
                    nState = 1
                End If
            End If
            If nState = 1 Then
                If nInputBit = 0 Then
                    bitstring$ = bitstring$ + "0"
                Else
                    bitstring$ = bitstring$ + "1"
                End If
                nBitCounter = nBitCounter + 1
                If (nBitCounter = nBytes * 8) Then Exit Do
            End If
        End If
    Loop While (EOF(1) = False)
    For i = 1 To Len(bitstring$) Step 4
        teststring$ = Mid$(bitstring$, i, 4)
        nzerocount = 0
        nonecount = 0
        For j = 1 To 4
            If Mid$(teststring$, j, 1) = "0" Then
                nzerocount = nzerocount + 1
            Else
                nonecount = nonecount + 1
            End If
        Next
        If nonecount >= nzerocount Then
            newbitstring$ = newbitstring$ + "1"
        Else
            newbitstring$ = newbitstring$ + "0"
        End If
    Next
    bitstring$ = newbitstring$
    nBytes = nBytes / 4
    For i = 1 To nBytes
        nByteValue = 0
        If Mid$(bitstring, (i - 1) * 8 + 1, 1) = "1" Then nByteValue = nByteValue + 128
        If Mid$(bitstring, (i - 1) * 8 + 2, 1) = "1" Then nByteValue = nByteValue + 64
        If Mid$(bitstring, (i - 1) * 8 + 3, 1) = "1" Then nByteValue = nByteValue + 32
        If Mid$(bitstring, (i - 1) * 8 + 4, 1) = "1" Then nByteValue = nByteValue + 16
        If Mid$(bitstring, (i - 1) * 8 + 5, 1) = "1" Then nByteValue = nByteValue + 8
        If Mid$(bitstring, (i - 1) * 8 + 6, 1) = "1" Then nByteValue = nByteValue + 4
        If Mid$(bitstring, (i - 1) * 8 + 7, 1) = "1" Then nByteValue = nByteValue + 2
        If Mid$(bitstring, (i - 1) * 8 + 8, 1) = "1" Then nByteValue = nByteValue + 1
        paddedhex$ = LCase$(Hex$(nByteValue))
        If Len(paddedhex$) < 2 Then paddedhex$ = "0" + paddedhex$
        hexstring$ = hexstring$ & "0x" & paddedhex$
        If i < nBytes Then hexstring$ = hexstring$ & ", "
    Next
    Debug.Print hexstring$
    
    Print #2, "BYTE " & prefix.Text & "_" & key.Text & " = {" & hexstring$ & "}; // " & bitstring$
    
    
    Close #1
    Kill "C:\Documents and Settings\rod\Desktop\ant8rld"
    
End Sub

Private Sub Form_Load()

    Open "remote.txt" For Append As #2
    
End Sub

Private Sub Form_Unload(Cancel As Integer)

    Close #2
    
End Sub

