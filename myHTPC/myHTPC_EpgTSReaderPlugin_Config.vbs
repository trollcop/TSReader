Option Explicit

Dim Shell
Dim checkSettings
Dim baseKey
Dim AppName
Dim Default
Dim Question
Dim AnswerCardType
Dim AnswerTSReaderPath
Dim AnswermyHTPCPath
Dim AnswerDiSEqC
Dim AnswerDiSEqC1
Dim AnswerDiSEqC2
Dim AnswerDiSEqC3
Dim AnswerDiSEqC4
Dim AnswerLNBLO
Dim AnswerLNBLO1
Dim AnswerLNBLO2
Dim AnswerLNBLO3
Dim AnswerLNBLO4
Dim AnswerLNBLOStacked
Dim AnswerLNBLOStacked1
Dim AnswerLNBLOStacked2
Dim AnswerLNBLOStacked3
Dim AnswerLNBLOStacked4
Dim AnswerRecordingPath

' Instantiate Shell object
Set Shell = CreateObject("WScript.Shell")

' Set registry key for settings
baseKey = "HKCU\Software\TSReader_MyHTPC_Plugin\Settings\"

' Check for existing key
On Error Resume Next
checkSettings = Shell.RegRead(baseKey)
On Error GoTo 0

AppName = "TSReader Plugin Configuration"

' // -----------------------------------------------------------------------------------
' // Card Type
' //

' Check for existing option value
On Error Resume Next
checkSettings = ""
checkSettings = Shell.RegRead(baseKey & "CardType")
On Error GoTo 0

' Set default value
If Len(checkSettings) Then
	Default = checkSettings
Else
	Default = "1"
End If

Question = "Enter Type of Card. (1=DVB-S, 2=Full Mode Satellite (Future: 3=DVB-T, 4=DVB-C, 5=ATSC))"
AnswerCardType = -1
Do Until (AnswerCardType <> -1)
	AnswerCardType = InputBox(Question,AppName,Default)
	If (Len(AnswerCardType) = 0) Then
		' Quit script if user hits the Cancel button
		Shell.PopUp "Configuration cancelled!",0,AppName,64 
		WScript.Quit
	End If
Loop	

' // -----------------------------------------------------------------------------------
' // Path to TSReader
' //

' Check for existing option value
On Error Resume Next
checkSettings = ""
checkSettings = Shell.RegRead(baseKey & "TSReaderPathVB")
On Error GoTo 0

' Set default value
If Len(checkSettings) Then
	Default = checkSettings
Else
	Default = "C:\Progra~1\Cool.Stf\TSReader"
End If

Question = "Enter path to TSReader"
AnswerTSReaderPath = -1
Do Until (AnswerTSReaderPath <> -1)
	AnswerTSReaderPath = InputBox(Question,AppName,Default)
	If (Len(AnswerTSReaderPath) = 0) Then
		' Quit script if user hits the Cancel button
		Shell.PopUp "Configuration cancelled!",0,AppName,64 
		WScript.Quit
	End If
Loop	

' // -----------------------------------------------------------------------------------
' // Path to myHTPC
' //

' Check for existing option value
On Error Resume Next
checkSettings = ""
checkSettings = Shell.RegRead(baseKey & "myHTPCPath")
On Error GoTo 0

' Set default value
If Len(checkSettings) Then
	Default = checkSettings
Else
	Default = "C:\Progra~1\myHTPC"
End If

Question = "Enter path to myHTPC"
AnswermyHTPCPath = -1
Do Until (AnswermyHTPCPath <> -1)
	AnswermyHTPCPath = InputBox(Question,AppName,Default)
	If (Len(AnswermyHTPCPath) = 0) Then
		' Quit script if user hits the Cancel button
		Shell.PopUp "Configuration cancelled!",0,AppName,64 
		WScript.Quit
	End If
Loop	

If AnswerCardType = "1" Or AnswerCardType = "2" Then

	GetDiSEqCOptions 1, "119.0W", 11250
	AnswerDiSEqC1 = AnswerDiSEqC
	AnswerLNBLO1 = AnswerLNBLO
	AnswerLNBLOStacked1 = AnswerLNBLOStacked

	GetDiSEqCOptions 2, "110.0W", 11250
	AnswerDiSEqC2 = AnswerDiSEqC
	AnswerLNBLO2 = AnswerLNBLO
	AnswerLNBLOStacked2 = AnswerLNBLOStacked

	GetDiSEqCOptions 3, "105.0W", 10750
	AnswerDiSEqC3 = AnswerDiSEqC
	AnswerLNBLO3 = AnswerLNBLO
	AnswerLNBLOStacked3 = AnswerLNBLOStacked

	GetDiSEqCOptions 4, "61.5W", 11250
	AnswerDiSEqC4 = AnswerDiSEqC
	AnswerLNBLO4 = AnswerLNBLO
	AnswerLNBLOStacked4 = AnswerLNBLOStacked

End If

' // -----------------------------------------------------------------------------------
' // Path to recording
' //

' Check for existing option value
On Error Resume Next
checkSettings = ""
checkSettings = Shell.RegRead(baseKey & "RecordingPath")
On Error GoTo 0

' Set default value
If Len(checkSettings) Then
	Default = checkSettings
Else
	Default = "C:\Progra~1\myHTPC\data\tv"
End If

Question = "Enter path to MPEG recordings"
AnswerRecordingPath = -1
Do Until (AnswerRecordingPath <> -1)
	AnswerRecordingPath = InputBox(Question,AppName,Default)
	If (Len(AnswerRecordingPath) = 0) Then
		' Quit script if user hits the Cancel button
		Shell.PopUp "Configuration cancelled!",0,AppName,64 
		WScript.Quit
	End If
Loop			
' // -----------------------------------------------------------------------------------
' // Write results to registry
' //

' Write change date to registry
Shell.RegWrite baseKey,Now,"REG_SZ"

' Write CardType option
Shell.RegWrite baseKey & "CardType",AnswerCardType,"REG_SZ"

' Write VBscript style TSReader path
Shell.RegWrite baseKey & "TSReaderPathVB",AnswerTSReaderPath,"REG_SZ"

' Write JavaScript style TSReader path
Shell.RegWrite baseKey & "TSReaderPathJS",Replace(AnswerTSReaderPath,"\","\\"),"REG_SZ"

' Write myHTPC path
Shell.RegWrite baseKey & "myHTPCPath",AnswermyHTPCPath,"REG_SZ"

' Write TSReader program option
Shell.RegWrite baseKey & "TSReaderProgram","TSReader","REG_SZ"

' Write LNBF options
Shell.RegWrite baseKey & "LNBF1",AnswerLNBLO1,"REG_SZ"
Shell.RegWrite baseKey & "LNBF2",AnswerLNBLO2,"REG_SZ"
Shell.RegWrite baseKey & "LNBF3",AnswerLNBLO3,"REG_SZ"
Shell.RegWrite baseKey & "LNBF4",AnswerLNBLO4,"REG_SZ"

Shell.RegWrite baseKey & "LNBFStacked1",AnswerLNBLOStacked1,"REG_SZ"
Shell.RegWrite baseKey & "LNBFStacked2",AnswerLNBLOStacked2,"REG_SZ"
Shell.RegWrite baseKey & "LNBFStacked3",AnswerLNBLOStacked3,"REG_SZ"
Shell.RegWrite baseKey & "LNBFStacked4",AnswerLNBLOStacked4,"REG_SZ"


' Write DiSEqC options
Shell.RegWrite baseKey & "diseqc1",AnswerDiSEqC1,"REG_SZ"
Shell.RegWrite baseKey & "diseqc2",AnswerDiSEqC2,"REG_SZ"
Shell.RegWrite baseKey & "diseqc3",AnswerDiSEqC3,"REG_SZ"
Shell.RegWrite baseKey & "diseqc4",AnswerDiSEqC4,"REG_SZ"

' Write Recording path
Shell.RegWrite baseKey & "RecordingPath",AnswerRecordingPath,"REG_SZ"

' Success!
Shell.PopUp "The TSReader plugin has been successfully configured.",0,AppName,64 

Private Sub GetDiSEqCOptions(DiSEQcPort, DefaultOrbital, DefaultLNBLOF)

	' Check for existing option value
	On Error Resume Next
	checkSettings = ""
	checkSettings = Shell.RegRead(baseKey & "diseqc" & DiSEQcPort)
	On Error GoTo 0

	' Set default value
	If Len(checkSettings) Then
		Default = checkSettings
	Else
		Default = DefaultOrbital
	End If

	Question = "What Satellite is on DiSEqC " & DiSEQcPort & " ? (e.g. 119.0W, 110.0W, 105.0W, 61.5W)"
	AnswerDiSEqC = -1
	Do Until (AnswerDiSEqC <> -1)
		AnswerDiSEqC = InputBox(Question,AppName,Default)
		If (Len(AnswerDiSEqC) = 0) Then
			' Quit script if user hits the Cancel button
			Shell.PopUp "Configuration cancelled!",0,AppName,64 
			WScript.Quit
		End If
	Loop	

	' Check for existing option value
	On Error Resume Next
	checkSettings = ""
	checkSettings = Shell.RegRead(baseKey & "LNBF" & DiSEQcPort)
	On Error GoTo 0

	' Set default value
	If Len(checkSettings) Then
		Default = checkSettings
	Else
		Default = DefaultLNBLOF
	End If

	Question = "What is your LNB LOF for DiSEqC " & DiSEQcPort & " ? (11250)"
	AnswerLNBLO = -1
	Do Until (AnswerLNBLO <> -1)
		AnswerLNBLO = InputBox(Question,AppName,Default)
		If (Len(AnswerLNBLO) = 0) Then
			' Quit script if user hits the Cancel button
			Shell.PopUp "Configuration cancelled!",0,AppName,64 
			WScript.Quit
		End If
	Loop	

	' Check for existing option value
	On Error Resume Next
	checkSettings = ""
	checkSettings = Shell.RegRead(baseKey & "LNBFStacked" & DiSEQcPort)
	On Error GoTo 0

	' Set default value
	If Len(checkSettings) Then
		Default = checkSettings
	Else
		Default = "0"
	End If

	Question = "If you're using a stacked LNBF enter the high frequency? (0)"
	AnswerLNBLOStacked = -1
	Do Until (AnswerLNBLOStacked <> -1)
		AnswerLNBLOStacked = InputBox(Question,AppName,Default)
		If (Len(AnswerLNBLOStacked) = 0) Then
			' Quit script if user hits the Cancel button
			Shell.PopUp "Configuration cancelled!",0,AppName,64 
			WScript.Quit
		End If
	Loop	

End Sub
