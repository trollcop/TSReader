// Board 1077.h : main header file for the Board 1077 DLL
//

#pragma once

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols
#include "Board1077Hal.h"

const int MAX_CARDS = 6;

// CBoard1077App
// See Board 1077.cpp for the implementation of this class
//

class CBoard1077App : public CWinApp
{
public:
	CBoard1077App();
   //CBoard1077Hal device;
// Overrides
public:
	virtual BOOL InitInstance();
   
   CBoard1077Hal *pBoard1077Hal[MAX_CARDS];
   __int64 SerialNumber[MAX_CARDS];

	DECLARE_MESSAGE_MAP()
};
