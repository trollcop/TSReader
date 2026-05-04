// DTV_testDlg.h : header file
//

#if !defined(AFX_DTV_TESTDLG_H__2610D3F9_B4E5_416E_91B4_DD0AF4931525__INCLUDED_)
#define AFX_DTV_TESTDLG_H__2610D3F9_B4E5_416E_91B4_DD0AF4931525__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

/////////////////////////////////////////////////////////////////////////////
// CDTV_testDlg dialog

class CDTV_testDlg : public CDialog
{
// Construction
public:
	CDTV_testDlg(CWnd* pParent = NULL);	// standard constructor

	// Dialog Data
	//{{AFX_DATA(CDTV_testDlg)
	enum { IDD = IDD_DTV_TEST_DIALOG };
	CButton	m_RecPlayMode;
	CStatic	m_ReadCycles;
	CListCtrl	m_DataList;
	CStatic	m_BytesRead;
	CStatic	m_sBitRate;
	CStatic	m_sDataErr;
	CStatic	m_sSyncErr;
	CStatic	m_sFileName;
	CStatic	m_sBoardName;
	int		m_ModeValue;
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDTV_testDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	HICON m_hIcon;
	CWinThread *m_pRecordThread;
	CWinThread *m_pPlayThread;
	
	static UINT RecordThread(LPVOID pParam);
	static UINT PlayThread(LPVOID pParam);
	void InitATboard();
	
	// Generated message map functions
	//{{AFX_MSG(CDTV_testDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnStartbtn();
	afx_msg void OnStopbtn();
	afx_msg void OnTimer(UINT nIDEvent);
	afx_msg void OnRecplaymode();
	afx_msg void OnResetbtn();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DTV_TESTDLG_H__2610D3F9_B4E5_416E_91B4_DD0AF4931525__INCLUDED_)
