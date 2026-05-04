Attribute VB_Name = "modLoadTables"
Option Explicit
'*******************************************************************************
' File Name:      modLoadTables.bas
' Programmer:     Mike Golden
' Copyright:      SENCORE, Inc. 2001. All rights reserved.
' Creation Date:  8/16/2001
'
' Update History  Programmer     Description of modification
' --------------- -------------- ---------------------------------------
'
'*******************************************************************************

Public Declare Function GetTickCount Lib "Kernel32" () As Long

'Global Variables
Public FilTaps(0 To 17) As Double






'*******************************************************************************
' Name:        LoadEyeDiagramFillTaps
' Desc:        Loads the Hilbert Transform used to calculate the
'                   fill taps for the eye diagram
' Pre:         None
' Post:        FilTaps is Loaded with the Hilbert Transform Data
' Programmer:  Mike Golden
'
' Update History   Programmer      Description of modification
' ---------------   --------------  ---------------------------------------
'
'*******************************************************************************
Public Sub LoadEyeDiagramFillTaps()
   FilTaps(0) = 0.023743   ' 2.3743E-02
   FilTaps(1) = -0.012939  '-1.2939E-02
   FilTaps(2) = 0.016846   ' 1.6846E-02
   FilTaps(3) = -0.021973  '-2.1973E-02
   FilTaps(4) = 0.029114   ' 2.9114E-02
   FilTaps(5) = -0.039856  '-3.9856E-02
   FilTaps(6) = 0.058472   ' 5.8472E-02
   FilTaps(7) = -0.10052   '-1.0052E-01
   FilTaps(8) = 0.30618    ' 3.0618E-01
   FilTaps(9) = 0.30618    ' 3.0618E-01
   FilTaps(10) = -0.10052  '-1.0052E-01
   FilTaps(11) = 0.058472  ' 5.8472E-02
   FilTaps(12) = -0.039856 '-3.9856E-02
   FilTaps(13) = 0.029114  ' 2.9114E-02
   FilTaps(14) = -0.021973 '-2.1973E-02
   FilTaps(15) = 0.016846  ' 1.6846E-02
   FilTaps(16) = -0.012939 '-1.2939E-02
   FilTaps(17) = 0.023743  ' 2.3743E-02
End Sub


Public Sub Wait(delay As Long)
    Dim start As Long
    
    start = GetTickCount
    While GetTickCount < (start + delay)
       DoEvents
    Wend
End Sub
