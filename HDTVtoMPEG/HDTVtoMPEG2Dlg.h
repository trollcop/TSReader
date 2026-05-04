// HDTVtoMPEG2Dlg.h : header file
//

#if !defined(AFX_HDTVTOMPEG2DLG_H__64B610BB_7926_442F_AF99_076C26AF3513__INCLUDED_)
#define AFX_HDTVTOMPEG2DLG_H__64B610BB_7926_442F_AF99_076C26AF3513__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "ProgressDialog.h"

#include "bcdmux.h"

/////////////////////////////////////////////////////////////////////////////
// CHDTVtoMPEG2Dlg dialog

class CHDTVtoMPEG2Dlg : public CDialog
{
// Construction
public:
	CHDTVtoMPEG2Dlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	//{{AFX_DATA(CHDTVtoMPEG2Dlg)
	enum { IDD = IDD_HDTVTOMPEG2_DIALOG };
	CComboBox	m_ChannelCombo;
	CEdit	m_OutputFile;
	CListBox	m_InputFiles;
	CString	m_AudioPIDStr;
	CString	m_VideoPIDStr;
	int		m_MaxFileSize;
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CHDTVtoMPEG2Dlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL

	int m_VideoPID, m_AudioPID;

	bool m_bCancel;
	int m_InputCount;
	char** m_InputList;
	char m_OutputFileName[256];
	CProgressDialog m_ProgressDlg;

	#define MAXCHANNELS 16
	int m_NumChannels;
	ATSC_CHANNEL_INFO m_Channels[MAXCHANNELS];

	static void StaticConvertThread(void* data);
	void ConvertThread();

	static bool CHDTVtoMPEG2Dlg::StaticConvertCallback(int ifilenum, char* ifilename, char* ofilename, 
		__int64 curfilesize, __int64 curfilepos, __int64 curtotalsize, __int64 curtotalpos, void* data);

	bool CHDTVtoMPEG2Dlg::ConvertCallback(int ifilenum, char* ifilename, char* ofilename, 
		__int64 curfilesize, __int64 curfilepos, __int64 curtotalsize, __int64 curtotalpos);

// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	//{{AFX_MSG(CHDTVtoMPEG2Dlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnAdd();
	afx_msg void OnClear();
	afx_msg void OnConvert();
	afx_msg void OnSelchangeChannel();
	afx_msg void OnChangeMaxFileSize();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_HDTVTOMPEG2DLG_H__64B610BB_7926_442F_AF99_076C26AF3513__INCLUDED_)
