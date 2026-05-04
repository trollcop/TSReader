VERSION 5.00
Begin VB.Form frmChangeCard 
   BorderStyle     =   1  'Fixed Single
   Caption         =   "Change Card"
   ClientHeight    =   1500
   ClientLeft      =   45
   ClientTop       =   435
   ClientWidth     =   3555
   LinkTopic       =   "Form1"
   MaxButton       =   0   'False
   MinButton       =   0   'False
   ScaleHeight     =   1500
   ScaleWidth      =   3555
   StartUpPosition =   3  'Windows Default
   Begin VB.CommandButton btnCancel 
      Caption         =   "Cancel"
      Height          =   375
      Left            =   1920
      TabIndex        =   2
      Top             =   960
      Width           =   1095
   End
   Begin VB.CommandButton btnOK 
      Caption         =   "OK"
      Height          =   375
      Left            =   480
      TabIndex        =   1
      Top             =   960
      Width           =   1095
   End
   Begin VB.ComboBox cboCardIndex 
      Height          =   315
      ItemData        =   "frmChangeCard.frx":0000
      Left            =   2400
      List            =   "frmChangeCard.frx":0007
      Style           =   2  'Dropdown List
      TabIndex        =   0
      Top             =   240
      Width           =   615
   End
   Begin VB.Label Label1 
      Caption         =   "Choose a Card to Control:"
      Height          =   255
      Left            =   360
      TabIndex        =   3
      Top             =   320
      Width           =   1935
   End
End
Attribute VB_Name = "frmChangeCard"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = False
Attribute VB_PredeclaredId = True
Attribute VB_Exposed = False
Private Sub btnCancel_Click()
   Unload Me
End Sub

Private Sub btnOK_Click()
   InitializeBoard1049 cboCardIndex.ListIndex
   intCurrentCard = cboCardIndex.ListIndex + 1
   frmMain.Caption = "Board 1049 Example App - Card " & intCurrentCard
   Unload Me
End Sub

Private Sub Form_Load()
   cboCardIndex.ListIndex = 0
End Sub
