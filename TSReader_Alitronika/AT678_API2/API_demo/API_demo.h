// DTV_test.h : main header file for the DTV_TEST application
//

#if !defined(AFX_DTV_TEST_H__80F78A33_C9EA_4CDC_AB4D_186222B54366__INCLUDED_)
#define AFX_DTV_TEST_H__80F78A33_C9EA_4CDC_AB4D_186222B54366__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols

/////////////////////////////////////////////////////////////////////////////
// CDTV_testApp:
// See DTV_test.cpp for the implementation of this class
//

class CDTV_testApp : public CWinApp
{
public:
	CDTV_testApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDTV_testApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// Implementation

	//{{AFX_MSG(CDTV_testApp)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DTV_TEST_H__80F78A33_C9EA_4CDC_AB4D_186222B54366__INCLUDED_)
