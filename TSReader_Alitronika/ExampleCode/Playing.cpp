/**
 *	Background thread that sends the file to the board.
 *
 *	\param	pParam Pointer to the dialog page
 *	\return 0
 */
UINT CPlayback::PlayThread(LPVOID pParam)
{
	CPlayback*	pThis = (CPlayback*)pParam;
	DWORD	NrRead;
	BOOL bLooping;

	CATBoard &TheATBoard = currBoard;		// <<<< The active ATBoard object

	pThis->m_TotalRead = 0;
	pThis->m_BeginMs = GetTickCount();
	pThis->m_LastDeciSecs = 0;
	pThis->m_LoopCount = 0;


	/*
	 *	The file is OPEN, and Playing is ACTIVE
	 */

	bLooping = pThis->m_LoopChkBtn.GetCheck();
	do
	{
		pThis->m_LoopCount++;

		SetFilePointer(pThis->m_PlayFile, 0, NULL, FILE_BEGIN);
		NrRead = 1;

		while (pThis->m_bPlayThreadActive && NrRead != 0)
		{
			ReadFile(pThis->m_PlayFile, pThis->m_pBuffer, PLAY_BUFFERSIZE, &NrRead, NULL);
			if (NrRead)
			{
				while (!TheATBoard.SendPlayPacketDirect(pThis->m_pBuffer, NrRead) && pThis->m_bPlayThreadActive)
					Sleep(1);

				pThis->m_TotalRead += NrRead;
			}
		}
		bLooping = pThis->m_LoopChkBtn.GetCheck();
	}
	while (pThis->m_bPlayThreadActive && (NrRead != 0 || bLooping));

	CloseHandle(pThis->m_PlayFile);
	pThis->m_PlayFile = NULL;
	TheATBoard.StopPlaying();
	
	pThis->m_pBuffer = NULL;
	pThis->m_PlayProgressCtrl.SetPos(100);
	pThis->m_bPlayThreadActive = FALSE;

	return 0;
}

void CPlayback::OnStartBtn() 
{
	if (IsThreadRunning())
		return;

	CATBoard &TheATBoard = currBoard;		// <<<< The active ATBoard object

	if (!TheATBoard.IsUsbDeviceHighSpeed())
		AfxMessageBox("The device is not connected to a HIGH speed USB port.\nPlay will not work properly!!");

	if ((m_PlayFile = CreateFile(m_PlayFileName, GENERIC_READ,0,NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL)) != INVALID_HANDLE_VALUE)
	{
		DWORD FileLengthH;
		m_FileLength = GetFileSize(m_PlayFile, &FileLengthH);

		if (m_FileLength != 0 && TheATBoard.StartPlaying())
		{
			m_bPlayThreadActive = TRUE;
			m_pPlayThread = AfxBeginThread(PlayThread, this, THREAD_PRIORITY_ABOVE_NORMAL);
			if (m_pPlayThread == INVALID_HANDLE_VALUE)
			{
				CloseHandle(m_PlayFile);
				m_PlayFile = NULL;
				m_bPlayThreadActive = FALSE;
			}
		}
		else
		{
			TheATBoard.StopPlaying();
			CloseHandle(m_PlayFile);
			m_PlayFile = NULL;
			m_bPlayThreadActive = FALSE;
		}
	}
}

void CPlayback::OnHaltBtn() 
{
	m_bPlayThreadActive = FALSE;
}

