// ProgressDialog.cpp : implementation file
//
// Changelog:
//
// 1.07 - 2-23-2002 - GR: Support for progress bar of total conversion.
//
//

#include "stdafx.h"
#include "HDTVtoMPEG2.h"
#include "ProgressDialog.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CProgressDialog dialog

char CProgressDialog::m_InputFileName[256];
char CProgressDialog::m_OutputFileName[256];
int CProgressDialog::m_ProgressPos;
int CProgressDialog::m_ProgressTotalPos;

bool CProgressDialog::m_Finished;

CProgressDialog::CProgressDialog(CWnd* pParent /*=NULL*/)
	: CDialog(CProgressDialog::IDD, pParent)
{
	EnableAutomation();

	//{{AFX_DATA_INIT(CProgressDialog)
	//}}AFX_DATA_INIT
}


void CProgressDialog::OnFinalRelease()
{
	// When the last reference for an automation object is released
	// OnFinalRelease is called.  The base class will automatically
	// deletes the object.  Add additional cleanup required for your
	// object before calling the base class.

	CDialog::OnFinalRelease();
}

void CProgressDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CProgressDialog)
	DDX_Control(pDX, IDC_OUTPUTFILE, m_OutputFile);
	DDX_Control(pDX, IDC_INPUTFILE, m_InputFile);
	DDX_Control(pDX, IDC_PROGRESSBAR, m_ProgressBar);
	DDX_Control(pDX, IDC_PROGRESSTOTALBAR, m_ProgressTotalBar);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CProgressDialog, CDialog)
	//{{AFX_MSG_MAP(CProgressDialog)
	ON_WM_TIMER()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

BEGIN_DISPATCH_MAP(CProgressDialog, CDialog)
	//{{AFX_DISPATCH_MAP(CProgressDialog)
		// NOTE - the ClassWizard will add and remove mapping macros here.
	//}}AFX_DISPATCH_MAP
END_DISPATCH_MAP()

// Note: we add support for IID_IProgressDialog to support typesafe binding
//  from VBA.  This IID must match the GUID that is attached to the 
//  dispinterface in the .ODL file.

// {269D387C-75C8-40D4-B253-8D505A36FF8B}
static const IID IID_IProgressDialog =
{ 0x269d387c, 0x75c8, 0x40d4, { 0xb2, 0x53, 0x8d, 0x50, 0x5a, 0x36, 0xff, 0x8b } };

BEGIN_INTERFACE_MAP(CProgressDialog, CDialog)
		INTERFACE_PART(CProgressDialog, IID_IProgressDialog, Dispatch)
END_INTERFACE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CProgressDialog message handlers

BOOL CProgressDialog::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	m_ProgressBar.SetRange(0, 1000);
	m_ProgressTotalBar.SetRange(0, 1000);

	m_Timer = SetTimer(0, 100, NULL);

	m_InputFileName[0] = '\0';
	m_OutputFileName[0] = '\0';
	m_ProgressPos = 0;
	m_ProgressTotalPos = 0;

	m_Finished = false;

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CProgressDialog::OnTimer(UINT nIDEvent) 
{
	char input[256];
	char output[256];
	int progress;

	if (m_Finished)
		EndDialog(IDCANCEL);

	m_InputFile.GetWindowText(input, 255);
	if (strcmp(input, m_InputFileName) != 0)
		m_InputFile.SetWindowText(m_InputFileName);

	m_OutputFile.GetWindowText(output, 255);
	if (strcmp(output, m_OutputFileName) != 0)
		m_OutputFile.SetWindowText(m_OutputFileName);

	progress = m_ProgressBar.GetPos();
	if (progress != m_ProgressPos)
		m_ProgressBar.SetPos(m_ProgressPos);
	
	progress = m_ProgressTotalBar.GetPos();
	if (progress != m_ProgressTotalPos)
		m_ProgressTotalBar.SetPos(m_ProgressTotalPos);
	
	CDialog::OnTimer(nIDEvent);
}
