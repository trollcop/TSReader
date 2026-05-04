// DVBs Subtitler.h : main header file for the DVBS SUBTITLER application
//

#if !defined(AFX_DVBSSUBTITLER_H__8F3A1673_5C59_4CB0_81B6_EB2CC77695C8__INCLUDED_)
#define AFX_DVBSSUBTITLER_H__8F3A1673_5C59_4CB0_81B6_EB2CC77695C8__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols

/////////////////////////////////////////////////////////////////////////////
// CDVBsSubtitlerApp:
// See DVBs Subtitler.cpp for the implementation of this class
//

class CDVBsSubtitlerApp : public CWinApp
{
public:
	CDVBsSubtitlerApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDVBsSubtitlerApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// Implementation

	//{{AFX_MSG(CDVBsSubtitlerApp)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DVBSSUBTITLER_H__8F3A1673_5C59_4CB0_81B6_EB2CC77695C8__INCLUDED_)
