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
//  Filename:     WorkerThread.h
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
//   $Workfile: WorkerThread.h $
//   $Revision: 1 $  
//    $Modtime: 4.01.98 0:43 $
//
//   $Log: /DVB-PC/APP/TestAppUsbLcd/WorkerThread.h $
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
//     >>   3     5/29/00 2:08p Guido
//
///////////////////////////////////////////////////////////////////////////

// WorkerThread.h: Schnittstelle für die Klasse CWorkerThread.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_WORKERTHREAD_H__4C410AA2_69E3_11D3_A2D8_00D05CFFFF04__INCLUDED_)
#define AFX_WORKERTHREAD_H__4C410AA2_69E3_11D3_A2D8_00D05CFFFF04__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <afxmt.h>

#define STOP_THREAD_TIMEOUT 200

class CWorkerThread  
{
public:
	CWorkerThread();
	virtual ~CWorkerThread();

	BOOL StartThread(int iPriority = THREAD_PRIORITY_NORMAL);
	BOOL StopThread(DWORD dwTimeout = STOP_THREAD_TIMEOUT); // in Millisekunden
	BOOL IsRunning();
	BOOL GetThreadInfo(CWinThread*& pThread, DWORD& dwThreadID, int& iThreadPriority, time_t& tStartTime);

	static UINT WorkerThread(LPVOID pParam);
	virtual BOOL Run(); // in den abgeleiteten Klassen überschreiben !

//protected:
	
private:
	CCriticalSection m_CriticalSection;
	volatile long m_lRunThread;
	CWinThread* m_pThread;
	time_t m_tStartTime;
	int m_iThreadPriority;
	DWORD m_dwThreadID;
};

#endif // !defined(AFX_WORKERTHREAD_H__4C410AA2_69E3_11D3_A2D8_00D05CFFFF04__INCLUDED_)
