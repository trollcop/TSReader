#if !defined(AFX_SATSETTINGS_H__C083833D_D3E5_4348_9FF1_5C368F833407__INCLUDED_)
#define AFX_SATSETTINGS_H__C083833D_D3E5_4348_9FF1_5C368F833407__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// SatSettings.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// SatSettings dialog

class SatSettings : public CDialog
{
// Construction
public:
	SatSettings(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(SatSettings)
	enum { IDD = IDD_SETTINGS };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(SatSettings)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(SatSettings)
		// NOTE: the ClassWizard will add member functions here
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SATSETTINGS_H__C083833D_D3E5_4348_9FF1_5C368F833407__INCLUDED_)
