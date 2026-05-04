// HDTVtoMPEG2.h : main header file for the HDTVTOMPEG2 application
//

#if !defined(AFX_HDTVTOMPEG2_H__813859F8_6D35_46C6_997A_B4CBA3D52AA3__INCLUDED_)
#define AFX_HDTVTOMPEG2_H__813859F8_6D35_46C6_997A_B4CBA3D52AA3__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols
#include "bcdmux.h"

/////////////////////////////////////////////////////////////////////////////
// CHDTVtoMPEG2App:
// See HDTVtoMPEG2.cpp for the implementation of this class
//

class CHDTVtoMPEG2App : public CWinApp
{
public:
	CHDTVtoMPEG2App();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CHDTVtoMPEG2App)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// Implementation

	//{{AFX_MSG(CHDTVtoMPEG2App)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_HDTVTOMPEG2_H__813859F8_6D35_46C6_997A_B4CBA3D52AA3__INCLUDED_)
