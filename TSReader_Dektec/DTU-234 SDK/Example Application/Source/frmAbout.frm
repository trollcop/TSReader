VERSION 5.00
Begin VB.Form frmAbout 
   BorderStyle     =   3  'Fixed Dialog
   Caption         =   "About"
   ClientHeight    =   5040
   ClientLeft      =   45
   ClientTop       =   435
   ClientWidth     =   7965
   LinkTopic       =   "Form1"
   MaxButton       =   0   'False
   MinButton       =   0   'False
   ScaleHeight     =   5040
   ScaleWidth      =   7965
   ShowInTaskbar   =   0   'False
   StartUpPosition =   3  'Windows Default
   Begin VB.CommandButton cmdOK 
      Cancel          =   -1  'True
      Caption         =   "OK"
      Default         =   -1  'True
      Height          =   315
      Left            =   7080
      TabIndex        =   7
      Top             =   4680
      Width           =   735
   End
   Begin VB.Frame fraPages 
      BorderStyle     =   0  'None
      Caption         =   "About"
      Height          =   4455
      Index           =   5
      Left            =   20
      TabIndex        =   0
      Top             =   0
      Width           =   7800
      Begin VB.Frame fraAbout 
         BorderStyle     =   0  'None
         Caption         =   "Frame1"
         Height          =   1815
         Left            =   1920
         TabIndex        =   1
         Top             =   2760
         Width           =   3855
         Begin VB.Shape Shape1 
            BackColor       =   &H00E0E0E0&
            Height          =   1095
            Left            =   240
            Top             =   60
            Width           =   3375
         End
         Begin VB.Label lblSencore2 
            Alignment       =   2  'Center
            AutoSize        =   -1  'True
            BackStyle       =   0  'Transparent
            Caption         =   "Sencore Electronics"
            BeginProperty Font 
               Name            =   "Arial Black"
               Size            =   12
               Charset         =   0
               Weight          =   700
               Underline       =   0   'False
               Italic          =   0   'False
               Strikethrough   =   0   'False
            EndProperty
            ForeColor       =   &H000000C0&
            Height          =   345
            Left            =   420
            TabIndex        =   6
            Top             =   120
            Width           =   3015
         End
         Begin VB.Label lblAddress1 
            Alignment       =   2  'Center
            AutoSize        =   -1  'True
            BackStyle       =   0  'Transparent
            Caption         =   "3200 Sencore Drive"
            BeginProperty Font 
               Name            =   "MS Sans Serif"
               Size            =   12
               Charset         =   0
               Weight          =   700
               Underline       =   0   'False
               Italic          =   0   'False
               Strikethrough   =   0   'False
            EndProperty
            Height          =   300
            Left            =   420
            TabIndex        =   5
            Top             =   480
            Width           =   3015
         End
         Begin VB.Label lblAddress2 
            Alignment       =   2  'Center
            AutoSize        =   -1  'True
            BackStyle       =   0  'Transparent
            Caption         =   "Sioux Falls, SD  57107"
            BeginProperty Font 
               Name            =   "MS Sans Serif"
               Size            =   12
               Charset         =   0
               Weight          =   700
               Underline       =   0   'False
               Italic          =   0   'False
               Strikethrough   =   0   'False
            EndProperty
            Height          =   300
            Left            =   540
            TabIndex        =   4
            Top             =   720
            Width           =   2775
         End
         Begin VB.Label lblPhone 
            Alignment       =   2  'Center
            AutoSize        =   -1  'True
            BackStyle       =   0  'Transparent
            Caption         =   "1-800-SENCORE"
            BeginProperty Font 
               Name            =   "Tahoma"
               Size            =   12
               Charset         =   0
               Weight          =   700
               Underline       =   0   'False
               Italic          =   0   'False
               Strikethrough   =   0   'False
            EndProperty
            Height          =   285
            Left            =   930
            TabIndex        =   3
            Top             =   1260
            Width           =   1995
         End
         Begin VB.Label lblWebsite 
            Alignment       =   2  'Center
            BackStyle       =   0  'Transparent
            Caption         =   "www.sencore.com"
            BeginProperty Font 
               Name            =   "MS Sans Serif"
               Size            =   9.75
               Charset         =   0
               Weight          =   400
               Underline       =   0   'False
               Italic          =   0   'False
               Strikethrough   =   0   'False
            EndProperty
            ForeColor       =   &H00FF0000&
            Height          =   255
            Left            =   480
            MouseIcon       =   "frmAbout.frx":0000
            MousePointer    =   99  'Custom
            TabIndex        =   2
            Top             =   1500
            Width           =   3015
         End
      End
      Begin VB.Label lblBd6Firm 
         Caption         =   "Board 6 Firmware:  "
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
         Left            =   3800
         TabIndex        =   18
         Top             =   2400
         Width           =   3800
      End
      Begin VB.Label lblBd5Firm 
         Caption         =   "Board 5 Firmware:  "
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
         Left            =   3800
         TabIndex        =   17
         Top             =   2040
         Width           =   3800
      End
      Begin VB.Label lblBd4Firm 
         Caption         =   "Board 4 Firmware:  "
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
         Left            =   3800
         TabIndex        =   16
         Top             =   1680
         Width           =   3800
      End
      Begin VB.Label lblBd3Firm 
         Caption         =   "Board 3 Firmware:  "
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
         Left            =   3800
         TabIndex        =   15
         Top             =   1320
         Width           =   3800
      End
      Begin VB.Label lblBd2Firm 
         Caption         =   "Board 2 Firmware:  "
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
         Left            =   3800
         TabIndex        =   14
         Top             =   960
         Width           =   3800
      End
      Begin VB.Label lblBd1Firm 
         Caption         =   "Board 1 Firmware:  "
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
         Left            =   3800
         TabIndex        =   12
         Top             =   600
         Width           =   3800
      End
      Begin VB.Label lblDriverVer 
         Caption         =   "Driver:  "
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
         Left            =   240
         TabIndex        =   11
         Top             =   2040
         Width           =   3500
      End
      Begin VB.Label lblOCXVer 
         Caption         =   "OCX:  "
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
         Left            =   240
         TabIndex        =   10
         Top             =   1320
         Width           =   3500
      End
      Begin VB.Label lblAppVer 
         Caption         =   "Application:  "
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
         Left            =   240
         TabIndex        =   9
         Top             =   600
         Width           =   3500
      End
      Begin VB.Label lblProduct 
         Alignment       =   2  'Center
         Caption         =   "Board 1043 Versions"
         BeginProperty Font 
            Name            =   "MS Sans Serif"
            Size            =   18
            Charset         =   0
            Weight          =   700
            Underline       =   0   'False
            Italic          =   0   'False
            Strikethrough   =   0   'False
         EndProperty
         Height          =   495
         Left            =   1680
         TabIndex        =   8
         Top             =   120
         Width           =   4335
      End
   End
   Begin VB.Label Label1 
      Caption         =   "Board 1 Firmware Ver:"
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
      Left            =   0
      TabIndex        =   13
      Top             =   0
      Width           =   3015
   End
   Begin VB.Line Line10 
      BorderColor     =   &H00808080&
      X1              =   0
      X2              =   7920
      Y1              =   4560
      Y2              =   4560
   End
   Begin VB.Line Line9 
      BorderColor     =   &H00E0E0E0&
      X1              =   0
      X2              =   5500
      Y1              =   3015
      Y2              =   3015
   End
End
Attribute VB_Name = "frmAbout"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = False
Attribute VB_PredeclaredId = True
Attribute VB_Exposed = False

Private Sub cmdOK_Click()
   Unload Me
End Sub

Private Sub Form_Load()
      
   lblAppVer = lblAppVer & App.Major & "." & App.Minor & "." & App.Revision
   
   'frmMain.Board1049.Versions lngOCXVer, lngDriverVer, lngXilinxVer
   
   lblOCXVer = lblOCXVer & FormatLongVersionString(lngOCXVer)
   lblDriverVer = lblDriverVer & FormatLongVersionString(lngDriverVer)
   If lngXilinxVer(0) = -1 Then
      lblBd1Firm = lblBd1Firm & "Not Present"
   Else
      lblBd1Firm = lblBd1Firm & FormatLongVersionString(lngXilinxVer(0))
   End If
   
   If lngXilinxVer(1) = -1 Then
      lblBd2Firm = lblBd2Firm & "Not Present"
   Else
      lblBd2Firm = lblBd2Firm & FormatLongVersionString(lngXilinxVer(1))
   End If
   
   If lngXilinxVer(2) = -1 Then
      lblBd3Firm = lblBd3Firm & "Not Present"
   Else
      lblBd3Firm = lblBd3Firm & FormatLongVersionString(lngXilinxVer(2))
   End If
   
   If lngXilinxVer(3) = -1 Then
      lblBd4Firm = lblBd4Firm & "Not Present"
   Else
      lblBd4Firm = lblBd4Firm & FormatLongVersionString(lngXilinxVer(3))
   End If
   
   If lngXilinxVer(4) = -1 Then
      lblBd5Firm = lblBd5Firm & "Not Present"
   Else
      lblBd5Firm = lblBd5Firm & FormatLongVersionString(lngXilinxVer(4))
   End If
   
   If lngXilinxVer(5) = -1 Then
      lblBd6Firm = lblBd6Firm & "Not Present"
   Else
      lblBd6Firm = lblBd6Firm & FormatLongVersionString(lngXilinxVer(5))
   End If
   
   
End Sub
