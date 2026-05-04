Attribute VB_Name = "modDemod"
Option Explicit
'*******************************************************************************
' File Name:      modDemod.bas
' Programmer:     Mike Golden
' Copyright:      SENCORE, Inc. 2001. All rights reserved.
' Creation Date:  8/16/2001
'
' Update History  Programmer     Description of modification
' --------------- -------------- ---------------------------------------
'
'*******************************************************************************


'Global Variables

Public g_dblTotalSec As Double
Public g_dblTotalBer As Double
Public g_dblTotalUber As Double
Public g_dblTotalPN23 As Double
Public g_ErrSecsData As Single
Public g_BurstErrSecsData As Single
Public g_lngTotalPacketErrors As Long

Public intCurrentCard As Integer
Public intCardNumber As Integer

Public lngXilinxVer(5) As Long
Public lngOCXVer As Long
Public lngDriverVer As Long

Public lngRecordStatus As Board1049_STATUS_E

Public Enum Board1049_STATUS_E
   REC_STATUS_DONE_1049
   REC_STATUS_NO_PSYNC_1049
   REC_STATUS_LOST_PSYNC_1049
End Enum





'*******************************************************************************
' Name:        InitializeBoard1049
' Desc:        Initializes Board1049 and Loads all the inital values.
' Pre:         None.
' Post:        Everything is initalized.
' Programmer:  Mike Golden
'
' Update History   Programmer      Description of modification
' ---------------   --------------  ---------------------------------------
'
'*******************************************************************************
Public Sub InitializeBoard1049(BoardIndex As Integer)
'Unitialize Old Card if there was one
   frmMain.Board1049.Uninitialize
   
'Initalize the Board1049
   frmMain.Board1049.Initialize BoardIndex

'Set up Board1049's Configuration
   'frmMain.Board1049.TunerAGC = True
   frmMain.Board1049.TunerFrequencyHz = 57000000
   frmMain.Board1049.IoRfAttenuation = True
   
'Load the Tables stored in Files
   LoadEyeDiagramFillTaps
   
'Set up Form
   frmMain.Acquire.ListIndex = 0
   frmMain.cboChannelNumber.ListIndex = 0
   frmMain.cboChannelPlan.ListIndex = 0

End Sub


'*******************************************************************************
' Name:        UninitializeBoard1049
' Desc:        Uninitializes Board1049.
' Pre:         None.
' Post:        Board1049 is ready for Someone else to use.
' Programmer:  Mike Golden
'
' Update History   Programmer      Description of modification
' ---------------   --------------  ---------------------------------------
'
'*******************************************************************************
Public Sub UninitializeBoard1049()

'Uninitialize the Board1049
   frmMain.Board1049.Uninitialize

End Sub

'*******************************************************************************
' Name:        ModeAcquire
' Desc:        Sets the Input Mode.
' Pre:         Mode index.
' Post:        Correct Mode is set.
' Programmer:  Mike Golden
'
' Update History   Programmer      Description of modification
' ---------------   --------------  ---------------------------------------
'
'*******************************************************************************
Public Sub ModeAcquire(Mode As Integer)

   frmMain.Board1049.DemodSetAcquire Mode

End Sub


'*******************************************************************************
' Name:        CheckLock
' Desc:        Checks Lock
' Pre:         None.
' Post:        Lock indicators set.
' Programmer:  Mike Golden
'
' Update History   Programmer      Description of modification
' ---------------   --------------  ---------------------------------------
'
'*******************************************************************************
Public Sub CheckLock()
   
   frmMain.Board1049.DemodGetStatus1
   
   If frmMain.Board1049.DemodStatusFLock = True Then
      frmMain.lblFecLockIndicator.Caption = "Locked"
      frmMain.lblFecLockIndicator.BackColor = vbGreen
   Else
      frmMain.lblFecLockIndicator.Caption = "UnLocked"
      frmMain.lblFecLockIndicator.BackColor = vbRed
   End If
   
   If frmMain.Board1049.DemodStatusRLock = True Then
      frmMain.lblRcvrLockIndicator.Caption = "Locked"
      frmMain.lblRcvrLockIndicator.BackColor = vbGreen
   Else
      frmMain.lblRcvrLockIndicator.Caption = "UnLocked"
      frmMain.lblRcvrLockIndicator.BackColor = vbRed
   End If
End Sub



'*******************************************************************************
' Name:        PlotSymbols
' Desc:        Clears and Adds New Data to the Symbols Chart.
' Pre:         None.
' Post:        Chart has new set of Symbols.
' Programmer:  Mike Golden
'
' Update History   Programmer      Description of modification
' ---------------   --------------  ---------------------------------------
'
'*******************************************************************************
Public Sub PlotSymbols()
   Dim intIQData(2050) As Integer
   Dim intLoop As Integer
   
   For intLoop = 0 To 32
      frmMain.Board1049.DemodGetSymbols intIQData(intLoop * 62), 62
   Next intLoop

   frmMain.tctConstellation.Series(0).Clear
   
   'Constellation Diagram
   For intLoop = 0 To 1022
      frmMain.tctConstellation.Series(0).AddXY intIQData(intLoop * 2) + 16, intIQData(intLoop * 2 + 1), "", vbRed
   Next intLoop
End Sub


'*******************************************************************************
' Name:        PlotEyeDiagram
' Desc:        Clears and Adds New Data to the Eye Diagram Chart.
' Pre:         None.
' Post:        Chart has new set of Data.
' Programmer:  Mike Golden
'
' Update History   Programmer      Description of modification
' ---------------   --------------  ---------------------------------------
'
'*******************************************************************************
Public Sub PlotEyeDiagram()
   Dim intIQData(900) As Integer
   Dim EyeDiagramData(800) As Long
   Dim intLoop As Integer
   Dim intInnerLoop As Integer
   Dim dblCurSum As Double
   
   For intLoop = 0 To 13
      frmMain.Board1049.DemodGetSymbols intIQData(intLoop * 62), 62
   Next intLoop

   'Zero Pack and Interpolate
   For intLoop = 0 To 799
      EyeDiagramData(intLoop) = intIQData(intLoop + 18)
      intLoop = intLoop + 1
      dblCurSum = 0
      
      For intInnerLoop = 0 To 17
         dblCurSum = dblCurSum + (FilTaps(intInnerLoop) * intIQData(intInnerLoop * 2 + intLoop + 1))
      Next intInnerLoop
      
      dblCurSum = dblCurSum * 2.075
      EyeDiagramData(intLoop) = dblCurSum
   Next intLoop
   
   
   'Plot
   With frmMain.tctEyeDiagram
      For intLoop = 0 To 199
         .Series(intLoop).Clear
         
         For intInnerLoop = 0 To 4
         .Series(intLoop).AddXY intInnerLoop, EyeDiagramData(intLoop * 4 + intInnerLoop), "", vbBlue
         Next intInnerLoop
      Next intLoop
      
   End With


End Sub


'*******************************************************************************
' Name:        PlotMultiPath
' Desc:        Loads the FFE's and DFE's then Plots them on the Graph
' Pre:         None.
' Post:        Graph has new Data.
' Programmer:  Mike Golden
'
' Update History   Programmer      Description of modification
' ---------------   --------------  ---------------------------------------
'
'*******************************************************************************
Public Sub PlotMultiPath()
   Dim intEchoTaps(495) As Integer
   Dim EqTapsData(495) As Long
   Dim dblLoadingData As Double
   Dim dblCenterTap As Double
   Dim intLoop As Integer
   Dim intOuterLoop As Integer
   Dim SymbolRate As Double
   Dim iMag As Double
   
   SymbolRate = frmMain.Board1049.DemodStatusSymRate / 1000000
   
   
   If (frmMain.Acquire.ListIndex > 1) Then
      'Eq is in Complex Mode
   
      ' Get FFE Taps
      frmMain.Board1049.DemodGetFFE intEchoTaps(0), 64
   
      dblCenterTap = ComplexMag(intEchoTaps(59), intEchoTaps(51))
      
      If dblCenterTap <> 0 Then
           
         dblCenterTap = 100# / dblCenterTap
           
         ' Calculate Loading
         For intOuterLoop = 0 To 3
            For intLoop = 0 To 3
            
               iMag = ComplexMag(intEchoTaps(intOuterLoop * 16 + intLoop + 8), intEchoTaps(intOuterLoop * 16 + intLoop))
               EqTapsData(intOuterLoop * 4 + intLoop) = iMag * dblCenterTap
               
               If intOuterLoop <> 3 And intLoop <> 3 Then
                  dblLoadingData = dblLoadingData + CDbl(EqTapsData(intOuterLoop * 4 + intLoop)) ^ 2
               End If
             
            Next intLoop
         Next intOuterLoop
           
         ' Get FFE and DFE Taps
         frmMain.Board1049.DemodGetDFE intEchoTaps(0), 432
      
         For intOuterLoop = 0 To 17
            For intLoop = 0 To 5
            
               EqTapsData(intOuterLoop * 6 + intLoop + 16) = ComplexMag(intEchoTaps(intOuterLoop * 24 + intLoop), intEchoTaps(intOuterLoop * 24 + intLoop + 12)) * dblCenterTap / 4#
               
               dblLoadingData = dblLoadingData + CDbl(EqTapsData(intOuterLoop * 6 + intLoop + 16)) ^ 2
             
            Next intLoop
         Next intOuterLoop
           
         dblLoadingData = Sqr(dblLoadingData / 123)
         frmMain.lblEqLoad.Caption = Format(dblLoadingData, "##0.0")
   
      End If
   
       If frmMain.ckbLogScale.Value = 1 Then
           For intLoop = 0 To 123
               If EqTapsData(intLoop) > 0 Then
                   EqTapsData(intLoop) = 20 * Log10(EqTapsData(intLoop) / 100)
               Else
                   EqTapsData(intLoop) = -50
               End If
           Next intLoop
       End If
       
   
      With frmMain.tctMultipathTaps.Series(0)
          .Clear
          For intLoop = 0 To 123
              .AddXY ((intLoop - 15) / SymbolRate / 2), EqTapsData(intLoop), "", vbRed
          Next intLoop
      End With
      
   Else
   
      ' Get FFE and DFE Taps
      frmMain.Board1049.DemodGetFFE intEchoTaps(0), 64
      frmMain.Board1049.DemodGetDFE intEchoTaps(64), 432
      
      If intEchoTaps(63) <> 0 Then
           
         dblCenterTap = 100# / CDbl(intEchoTaps(63))
           
         ' Calculate Loading
         For intLoop = 0 To 63
         
            EqTapsData(intLoop) = Abs(CDbl(intEchoTaps(intLoop)) * dblCenterTap)
            If intLoop <> 63 Then
               dblLoadingData = dblLoadingData + ((CDbl(EqTapsData(intLoop)) * CDbl(EqTapsData(intLoop))))
            End If
          
         Next intLoop
           
         For intLoop = 64 To 495
         
            EqTapsData(intLoop) = Abs(CDbl(intEchoTaps(intLoop)) * dblCenterTap) / 4#
            dblLoadingData = dblLoadingData + CDbl(EqTapsData(intLoop)) ^ 2
         
         Next intLoop
           
         dblLoadingData = Sqr(dblLoadingData / 495)
         frmMain.lblEqLoad.Caption = Format(dblLoadingData, "##0.0")
   
      End If
   
       If frmMain.ckbLogScale.Value = 1 Then
           For intLoop = 0 To 495
               If EqTapsData(intLoop) > 0 Then
                   EqTapsData(intLoop) = 20 * Log10(EqTapsData(intLoop) / 100)
               Else
                   EqTapsData(intLoop) = -50
               End If
           Next intLoop
       End If
       
   
      With frmMain.tctMultipathTaps.Series(0)
          .Clear
          For intLoop = 0 To 495
              .AddXY ((intLoop - 63) / SymbolRate / 2), EqTapsData(intLoop), "", vbRed
              '.AddXY ((intLoop - 63) / 10.762), intEchoTaps(intLoop), "", vbRed
          Next intLoop
      End With
   End If
        
   

End Sub

Public Function Log10(X)
   If X = 0 Then
      Log10 = -60
   Else
      Log10 = Log(X) / Log(10#)
   End If
End Function


'*******************************************************************************
' Name:        MerMarginEvm
' Desc:        Calculates Mer Margin and Evm
' Pre:         None.
' Post:        Mer Margin and Evm are Updated
' Programmer:  Mike Golden
'
' Update History   Programmer      Description of modification
' ---------------   --------------  ---------------------------------------
'
'*******************************************************************************
Public Sub MerMarginEvm()
   Dim sngMERData As Single
   Dim sngEVMData As Single
   Dim sngMarginData As Single
   

   frmMain.Board1049.DemodGetStatus1

   sngMERData = (frmMain.Board1049.DemodStatusEstSNR / 256#)
   sngEVMData = 65.47 * (10 ^ (-sngMERData / 20)) * Sqr(2)
   sngMarginData = sngMERData - 14.25
   
   frmMain.lblMer.Caption = Format(sngMERData, "0.0")
   frmMain.lblEvm.Caption = Format(sngEVMData, "0.0")
   frmMain.lblMargin.Caption = Format(sngMarginData, "0.0")

End Sub



'*******************************************************************************
' Name:        GetBerData
' Desc:        Calculates and Displays: PreFEC, PostFEC, SER, PN-23, ErrSec, BurstES.
' Pre:         None.
' Post:        Values are Calculated and Displayed.
' Programmer:  Mike Golden
'
' Update History   Programmer      Description of modification
' ---------------   --------------  ---------------------------------------
'
'*******************************************************************************
Public Sub GetBerData()

   Dim lngCorrectedBytes As Long
   Dim lngUncorrectedBytes As Long
   Dim lngPn23BitErrors As Long
   Dim lngSymbolRate As Long
   
   Dim sngPreFECData As Single
   Dim sngPostFECData As Single
   Dim sngSERData As Single
   Dim dblPN23Data As Double

   frmMain.Board1049.DemodGetStatus2

   lngCorrectedBytes = frmMain.Board1049.DemodStatusCErrors
   lngUncorrectedBytes = frmMain.Board1049.DemodStatusUErrors
   lngPn23BitErrors = frmMain.Board1049.DemodStatusBErrors
   
   lngSymbolRate = frmMain.Board1049.DemodStatusSymRate
   
   'Note: This is not a Precise way of doing this.  Time should be read from an
   '  accurate clock and stored as Total Number of Seconds that have elapsed.
   
   g_dblTotalSec = g_dblTotalSec + 1
   
   'End Note.

   If g_dblTotalSec > Val(frmMain.txtWindowSeconds) Then
      g_dblTotalSec = 1
      g_dblTotalBer = 0
      g_dblTotalUber = 0
      g_dblTotalPN23 = 0
      g_ErrSecsData = 0
      g_BurstErrSecsData = 0
      g_lngTotalPacketErrors = 0
   End If
   

   If lngSymbolRate = 0 Then
      lngSymbolRate = 5381000
   End If

   If lngUncorrectedBytes <> 0 Then
      g_ErrSecsData = g_ErrSecsData + 1
   End If

   g_dblTotalBer = g_dblTotalBer + (lngCorrectedBytes / (lngSymbolRate * 3.98722))
   g_dblTotalUber = g_dblTotalUber + (lngUncorrectedBytes / (lngSymbolRate * 3.98722))
   g_lngTotalPacketErrors = g_lngTotalPacketErrors + lngUncorrectedBytes
   g_dblTotalPN23 = g_dblTotalPN23 + (lngUncorrectedBytes / (lngSymbolRate * 3.603834))
   
   sngPreFECData = g_dblTotalBer / g_dblTotalSec
   sngPostFECData = g_dblTotalUber / g_dblTotalSec
   sngSERData = g_lngTotalPacketErrors / g_dblTotalSec
   dblPN23Data = g_dblTotalPN23 / g_dblTotalSec


   If lngUncorrectedBytes >= Val(frmMain.txtBurstErrs.Text) Then
      g_BurstErrSecsData = g_BurstErrSecsData + 1
   End If
   

   If sngPreFECData <> 0 Then
      frmMain.lblPreFec = Format(sngPreFECData, "0.0E-00")
   Else
      frmMain.lblPreFec = "0"
   End If
   
   If sngPostFECData <> 0 Then
      frmMain.lblPostFec = Format(sngPostFECData, "0.0E-00")
   Else
      frmMain.lblPostFec = "0"
   End If
   
   If dblPN23Data <> 0 Then
      frmMain.lblPn23 = Format(dblPN23Data, "0.0E-00")
   Else
      frmMain.lblPn23 = "0"
   End If
   
   frmMain.lblSer = Format(sngSERData, "0.0")
   frmMain.lblErrSec = Format(g_ErrSecsData, "0")
   frmMain.lblBurstEs = Format(g_BurstErrSecsData, "0")
   frmMain.lblElapsedSeconds = g_dblTotalSec

   frmMain.Board1049.DemodSetBertCtl True

End Sub


'*******************************************************************************
' Name:        PlotChanResp
' Desc:        Calculates the Channel Response and Plots it.
' Pre:         None.
' Post:        Channel Response is Calculated and Plotted.
' Programmer:  Mike Golden
'
' Update History   Programmer      Description of modification
' ---------------   --------------  ---------------------------------------
'
'*******************************************************************************
Public Sub PlotChanResp()
   Dim intEchoTaps(496) As Integer
   Dim dblChanRespData(512) As Double
   Dim dblCenterTap As Double
   Dim dblMaxValue As Double
   Dim dblFreq As Double
   Dim intLoop As Integer
   Dim CrdIndex As Integer
   Dim intOuterLoop As Integer
   Dim SymbolRate As Double
   
   SymbolRate = frmMain.Board1049.DemodStatusSymRate / 1000000
   
   
   
   If (frmMain.Acquire.ListIndex > 1) Then
      'Eq is in Complex Mode
   
      ' Get FFE Taps
      frmMain.Board1049.DemodGetFFE intEchoTaps(0), 64
   
      dblCenterTap = ComplexMag(intEchoTaps(59), intEchoTaps(51))
      
      If dblCenterTap <> 0 Then
           
         For intOuterLoop = 0 To 3
            For intLoop = 0 To 3
               CrdIndex = (intOuterLoop * 4 + intLoop) * 2 + 1
               dblChanRespData(CrdIndex) = CDbl(intEchoTaps(intOuterLoop * 16 + intLoop + 8))
               dblChanRespData(CrdIndex + 1) = CDbl(intEchoTaps(intOuterLoop * 16 + intLoop))
            Next intLoop
         Next intOuterLoop
           
         ' Get FFE and DFE Taps
         frmMain.Board1049.DemodGetDFE intEchoTaps(0), 432
      
         For intOuterLoop = 0 To 17
            For intLoop = 0 To 5
               CrdIndex = (intOuterLoop * 6 + intLoop + 16) * 2 + 33
               dblChanRespData(CrdIndex) = CDbl(intEchoTaps(intOuterLoop * 24 + intLoop)) / -4#
               dblChanRespData(CrdIndex + 1) = CDbl(intEchoTaps(intOuterLoop * 24 + intLoop + 12)) / -4#
            Next intLoop
         Next intOuterLoop
   
      End If
      
      Dft dblChanRespData, 128, 1
      
      For intLoop = 1 To 128
         dblChanRespData(intLoop) = ComplexMag(dblChanRespData(intLoop * 2 - 1), dblChanRespData(intLoop * 2))
      Next intLoop
      
      For intLoop = 1 To 128
         dblChanRespData(intLoop) = 20 * Log10(dblChanRespData(intLoop))
      Next intLoop
      
      dblMaxValue = MaxVal(dblChanRespData, 128)
      
      For intLoop = 1 To 128
         dblChanRespData(intLoop) = dblChanRespData(intLoop) - dblMaxValue
      Next intLoop
      
      With frmMain.tctChanResp.Series(0)
         .Clear
         For intLoop = 1 To 128
            dblFreq = intLoop * (SymbolRate / 127#) - (SymbolRate / 2#)
            .AddXY dblFreq, dblChanRespData(intLoop), "", vbRed
         Next intLoop
      End With
      
   Else
   
   
      ' Get FFE and DFE Taps
      frmMain.Board1049.DemodGetFFE intEchoTaps(1), 64
      frmMain.Board1049.DemodGetDFE intEchoTaps(65), 432
      
      'dblCenterTap = CDbl(intEchoTaps(64))
      
      For intLoop = 1 To 64
         dblChanRespData(intLoop + 1) = CDbl(intEchoTaps(intLoop)) '/ dblCenterTap
      Next intLoop
      
      For intLoop = 65 To 496
         dblChanRespData(intLoop + 1) = (CDbl(intEchoTaps(intLoop)) / -4#) ' / dblCenterTap
      Next intLoop
      
      Dft dblChanRespData, 256, 1
      
      For intLoop = 1 To 256
         dblChanRespData(intLoop) = ComplexMag(dblChanRespData(intLoop * 2 - 1), dblChanRespData(intLoop * 2))
      Next intLoop
      
      For intLoop = 1 To 256
         dblChanRespData(intLoop) = 20 * Log10(dblChanRespData(intLoop))
      Next intLoop
      
      dblMaxValue = MaxVal(dblChanRespData, 256)
      
      For intLoop = 1 To 256
         dblChanRespData(intLoop) = dblChanRespData(intLoop) - dblMaxValue
      Next intLoop
      
      With frmMain.tctChanResp.Series(0)
         .Clear
         For intLoop = 1 To 256
            dblFreq = intLoop * (5.38 / 255#) - 2.69
            .AddXY dblFreq, dblChanRespData(intLoop), "", vbRed
         Next intLoop
      End With
   
   End If
   
End Sub


'*******************************************************************************
' Name:        Dft
' Desc:        Computes a Fourier Transform on the DftArray.  (Borrowed from Recipies in C)
' Pre:         Data is a complex array of length 'Length' - real array of size 2*Length.
'                    (r, i, r, i, r, i.....)
'                 This Function assumes a '1' Base Index
'              Length must be a power of 2.
'              If 'iSign' = 1, replaces 'DftArray' with its discrete Fourier transform
'              If 'iSign' =-1, replaces 'DftArray' with 'Length' times its inverse transform
' Post:        Fourier Transform on the DftArray.
' Programmer:  Mike Golden
'
' Update History   Programmer      Description of modification
' ---------------   --------------  ---------------------------------------
'
'*******************************************************************************
Public Function Dft(DftArray() As Double, Length As Integer, iSign As Integer)
   Dim n As Long
   Dim mmax As Long
   Dim m As Long
   Dim j As Long
   Dim istep As Long
   Dim i As Long
   
   Dim wtemp As Double
   Dim wr As Double
   Dim wpr As Double
   Dim wpi As Double
   Dim wi As Double
   Dim theta As Double
   Dim tempr As Double
   Dim tempi As Double
   Dim temp As Double
    
    n = Length * 2
    j = 1
    For i = 1 To n - 1 Step 2
        If j > i Then
            temp = DftArray(j)
            DftArray(j) = DftArray(i)
            DftArray(i) = temp
            temp = DftArray(j + 1)
            DftArray(j + 1) = DftArray(i + 1)
            DftArray(i + 1) = temp
        End If
        m = n / 2
        While (m >= 2 And j > m)
            j = j - m
            m = m / 2
        Wend
        j = j + m
    Next i
    mmax = 2
    While (n > mmax)
        istep = 2 * mmax
        theta = 6.28318530717959 / (iSign * mmax)
        wtemp = Sin(0.5 * theta)
        wpr = -2 * wtemp * wtemp
        wpi = Sin(theta)
        wr = 1
        wi = 0
        For m = 1 To mmax - 1 Step 2
            For i = m To n Step istep
                j = i + mmax
                tempr = wr * DftArray(j) - wi * DftArray(j + 1)
                tempi = wr * DftArray(j + 1) + wi * DftArray(j)
                DftArray(j) = DftArray(i) - tempr
                DftArray(j + 1) = DftArray(i + 1) - tempi
                DftArray(i) = DftArray(i) + tempr
                DftArray(i + 1) = DftArray(i + 1) + tempi
                
            Next i
            wtemp = wr
            wr = wr * wpr - wi * wpi + wr
            wi = wi * wpr + wtemp * wpi + wi
        Next m
        mmax = istep
    Wend
End Function


'*******************************************************************************
' Name:        MaxVal
' Desc:        Finds the Maximum Value in an Array
' Pre:         InArray is of Length (0 to Length-1)
' Post:        Max Value Returned
' Programmer:  Mike Golden
'
' Update History   Programmer      Description of modification
' ---------------   --------------  ---------------------------------------
'
'*******************************************************************************
Public Function MaxVal(InArray() As Double, Length As Integer) As Double
   Dim intLoop As Integer
   For intLoop = 1 To Length
      If MaxVal < InArray(intLoop) Then MaxVal = InArray(intLoop)
   Next intLoop
End Function


'*******************************************************************************
' Name:        ComplexMag
' Desc:        Calculate the Complex Magnitude of a Real Imaginary Pair
' Pre:         Real and Imag passed in.
' Post:        Magnitude Returned
' Programmer:  Mike Golden
'
' Update History   Programmer      Description of modification
' ---------------   --------------  ---------------------------------------
'
'*******************************************************************************
Public Function ComplexMag(ByVal Real As Double, ByVal iMag As Double) As Double
    ComplexMag = Sqr(Real ^ 2# + iMag ^ 2#)
End Function

Public Function FormatLongVersionString(ByVal lngValue As Long)

   FormatLongVersionString = ((lngValue / (16777215)) And 255) & "." & _
      ((lngValue / (65535)) And 255) & "." & _
      ((lngValue / (256)) And 255) & "." & _
      ((lngValue / (1)) And 255)

End Function



