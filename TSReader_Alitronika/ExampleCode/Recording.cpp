 /**
 *	Background thread that reads the file from the board.
 *
 *	\param	pParam Pointer to the dialog page
 *	\return 0
 */
UINT CRecord::RecordThread(LPVOID pParam)
{
	CRecord*	pThis = (CRecord *)pParam;

	pThis->m_RecThreadEvent.ResetEvent();	// CEvent m_RecThreadEvent

	CATBoard &TheATBoard = currBoard;		// <<<< The active ATBoard object

	DWORD NrBytes;
	DWORD NrWritten;

	pThis->m_bBitrateSet = FALSE;
	pThis->m_Ts.ResetResults();
	
	{	//providing scope for: regs
		CAtRegisters RegAcc(&TheATBoard);ATREGISTRY &Registers = RegAcc;
		pThis->m_Ts.SetTimestamping((Registers.m_RecordConfig & AT_RCONF_ETS) != 0);
	}	//regs out of scope and released....

	int		PcrCnt = -1;
	short	PcrPID = -1;
	__int64	LastPcrPos = 0;

	/*
	 *	The file is OPEN, and Recording is ACTIVE
	 */

	while (pThis->m_bRecordThreadActive)
	{
		if (!TheATBoard.GetRecordPacketDirect(pThis->m_pBuffer, RECORD_BUFFERSIZE, &NrBytes))
			break;

		if (NrBytes != 0)
		{
			WriteFile(pThis->m_RecordFile, pThis->m_pBuffer, NrBytes, &NrWritten, NULL);
			pThis->m_TotalRead += NrBytes;
		}

		Sleep(2);	// Relinguish execution to other processes and threads
	}

	TheATBoard.StopRecording();
	
	pThis->m_pBuffer = NULL;
	pThis->m_RecThreadEvent.SetEvent();	// Signal the end of the thread.
	return 0;
}


void CRecord::StartRecordThread()
{
	CATBoard &TheATBoard = currBoard;		// <<<< The active ATBoard object

	if (!TheATBoard.IsUsbDeviceHighSpeed())
		AfxMessageBox("The device is not connected to a HIGH speed USB port.\nRecord will not work properly!!");

	m_bRecordThreadActive = TRUE;
	if (TheATBoard.StartRecording())
	{
		m_pRecordThread = AfxBeginThread(RecordThread, this, THREAD_PRIORITY_ABOVE_NORMAL);
	}
}

void CRecord::StopRecordThread()
{
	if (m_bRecordThreadActive)
	{
		m_bRecordThreadActive = FALSE;
		WaitForSingleObject(m_RecThreadEvent, 1000);
		m_pRecordThread = NULL;
	}
}
