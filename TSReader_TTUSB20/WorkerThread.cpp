//  TT  -  TechnoTrend  -  TT  -  TechnoTrend  -  TT  -  TechnoTrend  -  TT  
//
//
//  (C) TechnoTrend Systemtechnik GmbH 1999/2000
//
//  All rights are reserved. Reproduction in whole or in part is prohibited 
//  without the written consent of the copyright owner. TechnoTrend 
//  reserves the right to make changes without notice at any time.  
//
//
//  TT  -  TechnoTrend  -  TT  -  TechnoTrend  -  TT  -  TechnoTrend  -  TT  
//
///////////////////////////////////////////////////////////////////////////
//
//  Filename:     WorkerThread.cpp
//
//  Project(s):   
//
//  Author:       GRi
//
//  Purpose:      Basisklasse für alle Klassen die einen eigenen Thread benötigen
//   
//  Environment:  Win32
//
//  Dev. Tool(s): MSVC 6.0
//
//  Note(s):      
// 
///////////////////////////////////////////////////////////////////////////
// 
//  File History
//
//   $Workfile: WorkerThread.cpp $
//   $Revision: 1 $  
//    $Modtime: 11.10.02 13:40 $
//
//   $Log: /DVB-PC/APP/TestAppUsbLcd/WorkerThread.cpp $
//     >>   
//     >>   1     22.01.03 15:04 Guido
//     >>   
//     >>   1     17.10.02 16:01 Guido
//     >>   
//     >>   1     14.10.02 9:17 Guido
//     >>   
//     >>   1     28.06.01 16:29 Jörg
//     >>   
//     >>   1     15.12.00 13:55 Steffen
//     >>   
//     >>   1     28.06.00 17:06 Steffen
//     >>   
//     >>   5     5/29/00 2:08p Guido
//
///////////////////////////////////////////////////////////////////////////

// WorkerThread.cpp: Implementierung der Klasse CWorkerThread.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "WorkerThread.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Konstruktion/Destruktion
//////////////////////////////////////////////////////////////////////

CWorkerThread::CWorkerThread()
{
	m_lRunThread = 0;
	m_pThread = NULL;
	m_tStartTime = 0;
	m_iThreadPriority = THREAD_PRIORITY_ERROR_RETURN;
	m_dwThreadID = 0;
}

CWorkerThread::~CWorkerThread()
{
	if(IsRunning())
		StopThread(STOP_THREAD_TIMEOUT);
}

//-------------------------------------------------------------------------
// CWorkerThread::IsRunning()
// 
// Purpose: TRUE wenn der Thread aktiv ist (existiert).
// 
// Author:  GRi
// Created: 5/29/00 2:06:37 PM
// 
// Parameters:	
// 
// Returns:  BOOL
// 
// 
// Global input:	 
// Global output: 
// 
// Note(s):      
// 
//-------------------------------------------------------------------------
BOOL CWorkerThread::IsRunning()
{
	if(m_lRunThread == 1)
		return TRUE;
	else
		return FALSE;
}

//-------------------------------------------------------------------------
// CWorkerThread::GetThreadInfo()
// 
// Purpose: gibt Status-Info's über den Thread zurück
// 
// Author:  GRi
// Created: 5/29/00 2:04:54 PM
// 
// Parameters:	
//     [pThread] :
//     [dwThreadID] :
//     [iThreadPriority] :
//     [tStartTime] :
// 
// Returns:  BOOL
// 
// 
// Global input:	 
// Global output: 
// 
// Note(s):      
// 
//-------------------------------------------------------------------------
BOOL CWorkerThread::GetThreadInfo(CWinThread*& pThread, DWORD& dwThreadID, int& iThreadPriority, time_t& tStartTime)
{
	pThread = m_pThread;
	dwThreadID = m_dwThreadID;
	iThreadPriority = m_iThreadPriority;
	tStartTime = m_tStartTime;
	
	if(m_lRunThread == 1)
		return TRUE;
	else
		return FALSE;
}

//-------------------------------------------------------------------------
// CWorkerThread::StartThread()
// 
// Purpose: Startet den Thread mit der angebenen oder mit normaler Priorität.
// 
// Author:  GRi
// Created: 5/29/00 1:59:54 PM
// 
// Parameters:	
//     [iPriority] :
// 
// Returns:  BOOL
// 
// 
// Global input:	 
// Global output: 
// 
// Note(s):      
// 
//-------------------------------------------------------------------------
BOOL CWorkerThread::StartThread(int iPriority)
{
	m_CriticalSection.Lock();
	if(m_lRunThread == 0)
	{
		m_lRunThread = 1;

		m_pThread = AfxBeginThread((AFX_THREADPROC)CWorkerThread::WorkerThread, this, iPriority);
		
		if (m_pThread != NULL)
		{
			CTime t = CTime::GetCurrentTime();
			m_tStartTime = t.GetTime();

			m_iThreadPriority = m_pThread->GetThreadPriority();
			m_dwThreadID = m_pThread->m_nThreadID;

			m_CriticalSection.Unlock();
			
			//TRACE("Start WorkerThread success!!!\n");
			return TRUE;
		}
		else
		{
			m_lRunThread = 0;
		}
	}
	m_CriticalSection.Unlock();
	return FALSE;
}

//-------------------------------------------------------------------------
// CWorkerThread::StopThread()
// 
// Purpose: Beendet den Thread (wenn nötig mit Kill).
// 
// Author:  GRi
// Created: 5/29/00 1:58:31 PM
// 
// Parameters:	
//     [dwTimeout] :
// 
// Returns:  BOOL
// 
// 
// Global input:	 
// Global output: 
// 
// Note(s):      
// 
//-------------------------------------------------------------------------
BOOL CWorkerThread::StopThread(DWORD dwTimeout)
{
	DWORD tcnt;
	BOOL timeout = FALSE;
	BOOL ownthread = FALSE;

	m_CriticalSection.Lock();
	if(m_lRunThread == 1)
	{
		m_lRunThread = 2; // Signal an Thread sich zu beenden
		if(m_pThread != NULL)
		{
			if(GetCurrentThreadId() == m_pThread->m_nThreadID)
				ownthread = TRUE;
		}
		m_CriticalSection.Unlock();

//		m_Event.SetEvent(); // Event auslösen, damit Thread sofort beendet wird
		
		if(ownthread)
		{
			TRACE("Thread tries to stop itself!\n");
		}
		else
		{
			tcnt = GetTickCount();
			while ((m_lRunThread != 0) && (timeout == FALSE))
			{
				if(GetTickCount() - tcnt > dwTimeout)
					timeout = TRUE;
				else
					Sleep(0);
			}
			if(timeout)
			{
				m_CriticalSection.Lock();
				if(m_pThread != NULL)
				{
					//TRACE("Terminate WorkerThread!!!\n");
					TerminateThread(m_pThread->m_hThread, (DWORD)-42);

					m_lRunThread = 0;
					m_pThread = NULL;
					//m_tStartTime = 0;
					m_iThreadPriority = THREAD_PRIORITY_ERROR_RETURN;
					m_dwThreadID = 0;
				}
				m_CriticalSection.Unlock();
			}
		}
		return TRUE;
	}
	m_CriticalSection.Unlock();
	return FALSE;
}

//-------------------------------------------------------------------------
// CWorkerThread::WorkerThread()
// 
// Purpose: Arbeits-Thread, statische Memberfunktion der Klasse,
// ruft über this Pointer die virtuelle (!) Memberfunktion "Run" auf.
// 
// Author:  GRi
// Created: 5/29/00 2:01:46 PM
// 
// Parameters:	
//     [pParam] :
// 
// Returns:  UINT
// 
// 
// Global input:	 
// Global output: 
// 
// Note(s):      
// 
//-------------------------------------------------------------------------
UINT CWorkerThread::WorkerThread(LPVOID pParam)
{
	CWorkerThread* pWorkerThread;

	//TRACE("Enter WorkerThread\n");
	pWorkerThread = (CWorkerThread*) pParam;

	while(pWorkerThread->m_lRunThread == 1)
	{
		if(! pWorkerThread->Run())
		{
			break;
		}
	}
	pWorkerThread->m_CriticalSection.Lock();
	pWorkerThread->m_lRunThread = 0;
	pWorkerThread->m_pThread = NULL;
	//pWorkerThread->m_tStartTime = 0;
	pWorkerThread->m_iThreadPriority = THREAD_PRIORITY_ERROR_RETURN;
	pWorkerThread->m_dwThreadID = 0;
	pWorkerThread->m_CriticalSection.Unlock();
	
	//TRACE("Leave WorkerThread\n");
	return 42;
}

//-------------------------------------------------------------------------
// CWorkerThread::Run()
// 
// Purpose: virtuelle Memberfunktion, hier wird die eigentliche 
// Thread-Funktionalität implementiert
// 
// Author:  GRi
// Created: 5/29/00 2:03:16 PM
// 
// Parameters:	
// 
// Returns:  BOOL
// 
// 
// Global input:	 
// Global output: 
// 
// Note(s):      
// Diese Funktion ist in den abgeleiteten Klassen zu überschreiben!
// Gibt die Funktion FALSE zurück wird der aufrufende Thread beendet,
// Bei TRUE wird die Funktion sofort wieder vom Thread aufgerufen.
// Alternativ kann solange in der Funktion verblieben werden wie
// IsRunning() == TRUE ist.
//-------------------------------------------------------------------------
BOOL CWorkerThread::Run()
{
	/*
	all_done = DoAPartOfSomething();
	if(all_done)
		return FALSE;
	else
		return TRUE;
	*/

	/*
	while(IsRunning())
	{
		all_done = DoAPartOfSomething();
		if(all_done)
			break;
	}
	return FALSE;
	*/
	
	return TRUE;
}

