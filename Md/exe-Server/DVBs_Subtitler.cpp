// DVBs_Subtitler.cpp : implementation file
//

#include "stdafx.h"
#include "DVBs Subtitler.h"
#include "DVBs_Subtitler.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// DVBs_Subtitler dialog


DVBs_Subtitler::DVBs_Subtitler(CWnd* pParent /*=NULL*/)
	: CDialog(DVBs_Subtitler::IDD, pParent)
{
	//{{AFX_DATA_INIT(DVBs_Subtitler)
	m_SubtitleFileName = _T("");
	//}}AFX_DATA_INIT
}


void DVBs_Subtitler::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(DVBs_Subtitler)
	DDX_Text(pDX, IDC_SubtitleFileName, m_SubtitleFileName);
	DDV_MaxChars(pDX, m_SubtitleFileName, 80);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(DVBs_Subtitler, CDialog)
	//{{AFX_MSG_MAP(DVBs_Subtitler)
	ON_BN_CLICKED(IDC_BUTTON1, OnButton1)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// DVBs_Subtitler message handlers

void DVBs_Subtitler::OnButton1() 
{
	// TODO: Add your control notification handler code here
	
}
