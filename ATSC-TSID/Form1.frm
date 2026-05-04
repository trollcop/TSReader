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
      Caption         =   "Convert"
      Height          =   855
      Left            =   840
      TabIndex        =   0
      Top             =   600
      Width           =   3255
   End
End
Attribute VB_Name = "Form1"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = False
Attribute VB_PredeclaredId = True
Attribute VB_Exposed = False
Option Explicit

Private Sub Command1_Click()

Dim szBuffer As String
Dim nSpace1 As Integer
Dim nSpace2 As Integer
Dim nSpace3 As Integer
Dim nSpace4 As Integer
Dim nSpace5 As Integer

Dim nTSID As Integer
Dim nATSCChannel As Integer
Dim nNTSCChannel As Integer
Dim nEnd As Integer
Dim szLocale As String

    Open "C:\Development\TSReader\ATSC-TSID\TSID Table.txt" For Input As #1
    Open "C:\Development\TSReader\ATSC-TSID\TSID.c" For Output As #2

    Do
        Line Input #1, szBuffer
        szBuffer = RTrim$(szBuffer)
        If Len(szBuffer) > 0 Then
            nSpace1 = InStr(1, szBuffer, " ")
            nSpace2 = InStr(nSpace1 + 1, szBuffer, " ")
            nSpace3 = InStr(nSpace2 + 1, szBuffer, " ")
            nSpace4 = InStr(nSpace3 + 1, szBuffer, " ")
            nEnd = Len(szBuffer)
            nATSCChannel = -1
            nNTSCChannel = -1
            Do
                If Mid$(szBuffer, nEnd, 1) = " " Then
                    If nATSCChannel = -1 Then
                        nATSCChannel = Val(Mid$(szBuffer, nEnd, Len(szBuffer)))
                    Else
                        If nNTSCChannel = -1 Then
                            nSpace5 = InStr(nEnd + 1, szBuffer, " ")
                            nNTSCChannel = Val(Mid$(szBuffer, nEnd, nSpace5 - nEnd))
                            Exit Do
                        End If
                    End If
                End If
                nEnd = nEnd - 1
            Loop
                
            nTSID = Val(Left(szBuffer, nSpace1 - 1))
            szLocale = Mid$(szBuffer, nSpace4 + 1, nEnd - nSpace4)
            Print #2, Chr$(9);
            Print #2, Format$(nTSID);
            Print #2, ", ";
            Print #2, Format$(nNTSCChannel);
            Print #2, ", ";
            Print #2, Format$(nATSCChannel);
            Print #2, ", ";
            Print #2, Chr(34); RTrim$(szLocale); Chr$(34); ","
            
        End If
        If EOF(1) Then Exit Do
    Loop

    Close #1
    Close #2
    
End Sub
