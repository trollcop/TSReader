Attribute VB_Name = "modRf"
Option Explicit
'*******************************************************************************
' File Name:      modRf.bas
' Programmer:     Mike Golden
' Copyright:      SENCORE, Inc. 2001. All rights reserved.
' Creation Date:  8/16/2001
'
' Update History  Programmer     Description of modification
' --------------- -------------- ---------------------------------------
'
'*******************************************************************************




'*******************************************************************************
' Name:        ChannelLevel
' Desc:        Gets the Current Channel Level in dBmV.
' Pre:         None.
' Post:        Channel Level is Displayed
' Programmer:  Mike Golden
'
' Update History   Programmer      Description of modification
' ---------------   --------------  ---------------------------------------
'
'*******************************************************************************
Public Sub ChannelLevel()
   Dim dBmV As Single
   Dim dBm As Single
   
   dBmV = frmMain.Board1049.GetChannelLevel
   
   dBm = dBmV - 48.8
   
   frmMain.lblChannelLeveldBmV.Caption = Format(dBmV, "0.0")
   frmMain.lblChannelLeveldBm.Caption = Format(dBm, "0.0")
End Sub

