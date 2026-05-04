#if !defined(AFX_PROGRESSDIALOG_H__41F1BFD9_5549_441D_AF58_D71B46BEBE2D__INCLUDED_)
#define AFX_PROGRESSDIALOG_H__41F1BFD9_5549_441D_AF58_D71B46BEBE2D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ProgressDialog.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CProgressDialog dialog

class CProgressDialog : public CDialog
{
// Construction
public:
	CProgressDialog(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CProgressDialog)
	enum { IDD = IDD_PROGRESS };
	CStatic	m_OutputFile;
	CStatic	m_InputFile;
	CProgressCtrl	m_ProgressBar;
	CProgressCtrl	m_ProgressTotalBar;
	//}}AFX_DATA

	static char m_InputFileName[256];
	static char m_OutputFileName[256];
	static int m_ProgressPos;
	static int m_ProgressTotalPos;

	static bool m_Finished;


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CProgressDialog)
	public:
	virtual void OnFinalRelease();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	
	UINT m_Timer;

	// Generated message map functions
	//{{AFX_MSG(CProgressDialog)
	virtual BOOL OnInitDialog();
	afx_msg void OnTimer(UINT nIDEvent);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
	// Generated OLE dispatch map functions
	//{{AFX_DISPATCH(CProgressDialog)
		// NOTE - the ClassWizard will add and remove member functions here.
	//}}AFX_DISPATCH
	DECLARE_DISPATCH_MAP()
	DECLARE_INTERFACE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PROGRESSDIALOG_H__41F1BFD9_5549_441D_AF58_D71B46BEBE2D__INCLUDED_)
