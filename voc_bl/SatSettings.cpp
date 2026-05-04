// SatSettings.cpp : implementation file
//

//#include "stdafx.h"
#include "voc_bl.h"
#include "SatSettings.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// SatSettings dialog


SatSettings::SatSettings(CWnd* pParent /*=NULL*/)
	: CDialog(SatSettings::IDD, pParent)
{
	//{{AFX_DATA_INIT(SatSettings)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void SatSettings::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(SatSettings)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(SatSettings, CDialog)
	//{{AFX_MSG_MAP(SatSettings)
		// NOTE: the ClassWizard will add message map macros here
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// SatSettings message handlers
