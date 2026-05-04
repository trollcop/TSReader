
#if !defined(AFX_RODTEST_H__5468C77A_8169_4089_8CCF_70946C00DE35__INCLUDED_)
#define AFX_RODTEST_H__5468C77A_8169_4089_8CCF_70946C00DE35__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#endif // !defined(AFX_RODTEST_H__5468C77A_8169_4089_8CCF_70946C00DE35__INCLUDED_)

#include <DVBBoardControl.h>
#include <DVBFrontend.h>
#include <DVBTSFilter.h>
#include <DVBComnIF.h>

#include "WorkerThread.h"

class CMyTSRecord : protected CWorkerThread
{
public:
	BOOL Start();
	BOOL Stop(void);
	void CheckContinuity(BYTE * pBuffer, int nLength);

private:
	BOOL Run();
	CFile m_File;
	CDVBBoardControl m_BoardControl;
	DWORD m_tc, m_cnt;
	int nContinuity[8192];
	int nPID;
	int nContinuityErrors;
	int nPackets;
	int nPreviousContinuityErrors;
	double dMBReceived;
	double dNextMBReceivedDisplay;
};
