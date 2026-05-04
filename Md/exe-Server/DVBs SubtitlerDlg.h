// DVBs SubtitlerDlg.h : header file
//

#if !defined(AFX_DVBSSUBTITLERDLG_H__8D0C1280_62BC_422F_B930_3E75A45AAB89__INCLUDED_)
#define AFX_DVBSSUBTITLERDLG_H__8D0C1280_62BC_422F_B930_3E75A45AAB89__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

/////////////////////////////////////////////////////////////////////////////
// CDVBsSubtitlerDlg dialog

class CDVBsSubtitlerDlg : public CDialog
{
// Construction
public:
	CDVBsSubtitlerDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	//{{AFX_DATA(CDVBsSubtitlerDlg)
	enum { IDD = IDD_DVBSSUBTITLER_DIALOG };
	CSliderCtrl	m_SliderControl;
	CString	m_SubtitleFileName;
	CString	m_CurrentTime;
	CString	m_MovieTime;
	CString	m_Subtitle1;
	CString	m_Subtitle2;
	long	m_CurrentFrame;
	int		m_StartFrame;
	int		m_ExpireFrame;
	CString	m_OSD1;
	CString	m_OSD2;
	CString	m_watch1;
	int		m_watch2;
	CString	m_offset;
	CString	m_OUTPUT;
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDVBsSubtitlerDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	//{{AFX_MSG(CDVBsSubtitlerDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnSubtitleFileselect();
	afx_msg void OnStart();
	afx_msg void OnExit();
	afx_msg void OnPause();
	afx_msg void OnTimer(UINT nIDEvent);
	afx_msg void OnStop();
	afx_msg void OnPlus25();
	afx_msg void OnPlus5();
	afx_msg void OnMinus25();
	afx_msg void OnMinus5();
	afx_msg void OnPlus6();
	afx_msg void OnMinus26();
	afx_msg void OnOutofmemorySlider1(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnCustomdrawSlider(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnNext();
	afx_msg void OnReleasedcaptureSlider(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnOutputFile();
	afx_msg void OnButton1();
	afx_msg void OnPrev();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DVBSSUBTITLERDLG_H__8D0C1280_62BC_422F_B930_3E75A45AAB89__INCLUDED_)
