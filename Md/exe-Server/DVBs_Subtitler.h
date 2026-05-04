#if !defined(AFX_DVBS_SUBTITLER_H__A65F0D25_2708_4042_B228_6AFF4C3046D8__INCLUDED_)
#define AFX_DVBS_SUBTITLER_H__A65F0D25_2708_4042_B228_6AFF4C3046D8__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// DVBs_Subtitler.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// DVBs_Subtitler dialog

class DVBs_Subtitler : public CDialog
{
// Construction
public:
	DVBs_Subtitler(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(DVBs_Subtitler)
	enum { IDD = IDD_DVBSSUBTITLER_DIALOG };
	CString	m_SubtitleFileName;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(DVBs_Subtitler)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(DVBs_Subtitler)
	afx_msg void OnButton1();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DVBS_SUBTITLER_H__A65F0D25_2708_4042_B228_6AFF4C3046D8__INCLUDED_)
