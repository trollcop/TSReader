// DvbDemo.h : main header file for the DVBDEMO application
//

#if !defined(AFX_DVBDEMO_H__0D30E1FF_AAF6_4E39_BB99_2136FB8CC0EA__INCLUDED_)
#define AFX_DVBDEMO_H__0D30E1FF_AAF6_4E39_BB99_2136FB8CC0EA__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols

/////////////////////////////////////////////////////////////////////////////
// CDvbDemoApp:
// See DvbDemo.cpp for the implementation of this class
//

class CDvbDemoApp : public CWinApp
{
public:
	CDvbDemoApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDvbDemoApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// Implementation

	//{{AFX_MSG(CDvbDemoApp)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DVBDEMO_H__0D30E1FF_AAF6_4E39_BB99_2136FB8CC0EA__INCLUDED_)
