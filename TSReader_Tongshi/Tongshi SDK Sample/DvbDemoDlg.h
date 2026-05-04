// DvbDemoDlg.h : header file
//

#if !defined(AFX_DVBDEMODLG_H__5180A6C8_157E_47CD_A620_BDDF94259249__INCLUDED_)
#define AFX_DVBDEMODLG_H__5180A6C8_157E_47CD_A620_BDDF94259249__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

/////////////////////////////////////////////////////////////////////////////
// CDvbDemoDlg dialog
#include ".\\sdk\\dvblib.h"

class CDvbDemoDlg : public CDialog
{
// Construction
public:
	CDvbDemoDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	//{{AFX_DATA(CDvbDemoDlg)
	enum { IDD = IDD_DVBDEMO_DIALOG };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDvbDemoDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	//{{AFX_MSG(CDvbDemoDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnButtonInit();
	afx_msg void OnButtonExit();
	afx_msg void OnButtonLasttuner();
	afx_msg void OnButtonAdd();
	afx_msg void OnButtonSignal();
	afx_msg void OnDestroy();
	afx_msg void OnButtonShowtuner();
	afx_msg void OnTimer(UINT nIDEvent);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

public:
	BOOL StartReceiveThread(void);
	void ReciveProc(void);
	CDVBLib * m_pDVBDrv;

	BOOL	bSysOk;	//System init Flag
	HANDLE m_hRecThread;	//Receiveing thread handle
	BOOL	bAskThreadQuitFlag;	//main thread ask receive thread exit flag

	BOOL m_bTunerStatus;	
	BOOL m_bLockStatus;
	UINT m_TimeVal;

	DWORD dwRecAllDataCount;

    static DWORD _stdcall InitThreadRecive(LPVOID pv)
	{
		CDvbDemoDlg * pthis = (CDvbDemoDlg *)pv;
		pthis->ReciveProc();
		ExitThread(0);
		return 0;
	}	


};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DVBDEMODLG_H__5180A6C8_157E_47CD_A620_BDDF94259249__INCLUDED_)
