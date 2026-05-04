// DvbDemoDlg.cpp : implementation file
//

#include "stdafx.h"
#include "DvbDemo.h"
#include "DvbDemoDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// Dialog Data
	//{{AFX_DATA(CAboutDlg)
	enum { IDD = IDD_ABOUTBOX };
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAboutDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	//{{AFX_MSG(CAboutDlg)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
	//{{AFX_DATA_INIT(CAboutDlg)
	//}}AFX_DATA_INIT
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAboutDlg)
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
	//{{AFX_MSG_MAP(CAboutDlg)
		// No message handlers
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDvbDemoDlg dialog

CDvbDemoDlg::CDvbDemoDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CDvbDemoDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CDvbDemoDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	// Note that LoadIcon does not require a subsequent DestroyIcon in Win32
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CDvbDemoDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDvbDemoDlg)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CDvbDemoDlg, CDialog)
	//{{AFX_MSG_MAP(CDvbDemoDlg)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTON_INIT, OnButtonInit)
	ON_BN_CLICKED(IDC_BUTTON_EXIT, OnButtonExit)
	ON_BN_CLICKED(IDC_BUTTON_LASTTUNER, OnButtonLasttuner)
	ON_BN_CLICKED(IDC_BUTTON_ADD, OnButtonAdd)
	ON_BN_CLICKED(IDC_BUTTON_SIGNAL, OnButtonSignal)
	ON_WM_DESTROY()
	ON_BN_CLICKED(IDC_BUTTON_SHOWTUNER, OnButtonShowtuner)
	ON_WM_TIMER()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDvbDemoDlg message handlers

BOOL CDvbDemoDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		CString strAboutMenu;
		strAboutMenu.LoadString(IDS_ABOUTBOX);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon
	
	// TODO: Add extra initialization here
	m_pDVBDrv = NULL;
	bSysOk = FALSE;
	m_hRecThread = NULL;
	bAskThreadQuitFlag = FALSE;
	dwRecAllDataCount = 0;


	m_TimeVal = SetTimer(1,200,0);
	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CDvbDemoDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CDvbDemoDlg::OnPaint() 
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, (WPARAM) dc.GetSafeHdc(), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

// The system calls this to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CDvbDemoDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}


//load and init dvb drv
void CDvbDemoDlg::OnButtonInit() 
{
	// TODO: Add your control notification handler code here
	bSysOk = FALSE;

	if (m_pDVBDrv == NULL)
	{
		m_pDVBDrv = new CDVBLib();
		if(m_pDVBDrv->IsLibLoadOK())
		{
			HWND hWnd = AfxGetMainWnd()->GetSafeHwnd();
			BOOL m_bDrvStatus = m_pDVBDrv->InitDrv(hWnd);
			if(m_bDrvStatus)
			{
				if (StartReceiveThread())
				{
				}
				AfxMessageBox("Init Driver Ok");
				bSysOk = TRUE;
			}			
		}
	}
}



void CDvbDemoDlg::OnButtonExit() 
{
	// TODO: Add your control notification handler code here
	if(m_pDVBDrv)
	{
		bAskThreadQuitFlag = TRUE;
		::WaitForSingleObject(this->m_hRecThread,2000);
		TRACE("Found Thread Exit\n");
		delete m_pDVBDrv;
		m_pDVBDrv = NULL;
		bSysOk = FALSE;
	}	
}

void CDvbDemoDlg::OnButtonLasttuner() 
{
	// TODO: Add your control notification handler code here
	if(m_pDVBDrv == NULL || bSysOk == FALSE) 
	{
		AfxMessageBox("Init System First");
		return;	//have not init system
	}

	InitParam drvInit;
	if(m_pDVBDrv->QueryDrv(&drvInit))
	{
		m_bTunerStatus = drvInit.flagType;
		m_bLockStatus = drvInit.flagLock;
		if (drvInit.flagDVBT == 1)	//DVB-T
		{

		}
		else if (drvInit.flagDVBT == 0 && drvInit.flagType == 1)	//DVB-S
		{
			TunerParam m_Param;
			if (m_pDVBDrv->QueryParamDBS(&m_Param,drvInit.szDBSName,drvInit.szChannelName,TRUE) == FALSE)
			{//if tuner fail ,show tuner dialog
				m_pDVBDrv->SetParamInfo(m_bTunerStatus);
			}
		}
		else if (drvInit.flagDVBT == 0 && drvInit.flagType == 0)	//DVB-C
		{
			QAMParam  m_Param;
			if (m_pDVBDrv->QueryParamQAM(&m_Param, drvInit.szChannelName, TRUE) == FALSE)
			{
				m_pDVBDrv->SetParamInfo(m_bTunerStatus);
			}
		}
		else
		{
			TRACE("Card Type ERROR\n");
		}
	}
	
}

void CDvbDemoDlg::OnButtonAdd() 
{
	// TODO: Add your control notification handler code here
	if(m_pDVBDrv == NULL || bSysOk == FALSE) 
	{
		AfxMessageBox("Init System First");
		return;	//have not init system
	}

	BOOL bReceiveAllMode = TRUE;
	HWND hWnd = AfxGetMainWnd()->GetSafeHwnd();

	WORD wPIDList[]={0,1037,132,173};

	
	if (bReceiveAllMode)
	{
		PIDSet suPID;
		WORD m_wRecPID = 0xffff;	//Enable Full Rec
		suPID.wRecType = PIDTYPE_IPDATA;
//		suPID.wRecType = PIDTYPE_VIDEO;
		suPID.wRecPID = m_wRecPID;
		m_pDVBDrv->RecPID(hWnd,1,&suPID);
	}
	else
	{
		PIDSet suPID[4];
		for (int i=0;i<4;i++)
		{
			suPID[i].wRecType = PIDTYPE_IPDATA;
			suPID[i].wRecPID = wPIDList[i];
		}
		m_pDVBDrv->RecPID(hWnd,4,&suPID[0]);
	}
}


void CDvbDemoDlg::OnButtonSignal() 
{
	// TODO: Add your control notification handler code here
	if(m_pDVBDrv == NULL || bSysOk == FALSE) 
	{
		AfxMessageBox("Init System First");
		return;	//have not init system
	}

	BOOL bLock;
	DWORD dwQuality;
	DWORD dwStrength;
	if (m_pDVBDrv->TSDVB0_GetSignal(&bLock,&dwQuality,&dwStrength))
	{
		char temp[100];
		sprintf(temp,"LOCK=%d Quality=%u Strength=%u",bLock,dwQuality,dwStrength);
		this->SetDlgItemText(IDC_STATIC_SIGNAL,temp);
	}
}

void CDvbDemoDlg::ReciveProc()
{
#define SIZE_TMPDATA	(1024*100)
	BYTE tmpDataBuf[SIZE_TMPDATA];
	DWORD dwReadDataCount;

	for (;;)
	{
		if (bAskThreadQuitFlag == TRUE)
		{
			break;
		}
		do {
			dwReadDataCount = m_pDVBDrv->ReadDataStream(NULL,SIZE_TMPDATA,tmpDataBuf);
			if (dwReadDataCount) 
			{
				dwRecAllDataCount += dwReadDataCount;				
			}
		} while (dwReadDataCount);
		Sleep(1);
	}
	TRACE("Thread Exit Here\n");
}

BOOL CDvbDemoDlg::StartReceiveThread()
{
	DWORD tmpThreadID;
	m_hRecThread = CreateThread(NULL,0,
		InitThreadRecive,
		(LPVOID)this,
		0,
		&tmpThreadID);
	if (m_hRecThread != NULL) return TRUE;
	else return FALSE;
}

void CDvbDemoDlg::OnDestroy() 
{
	CDialog::OnDestroy();
	
	// TODO: Add your message handler code here
	if (m_TimeVal) KillTimer(m_TimeVal);

	OnButtonExit();	
}

void CDvbDemoDlg::OnButtonShowtuner() 
{
	// TODO: Add your control notification handler code here

	if (m_pDVBDrv)
		m_pDVBDrv->SetParamInfo(m_bTunerStatus);
	else
	{
		AfxMessageBox("Init System First");
	}
}

void CDvbDemoDlg::OnTimer(UINT nIDEvent) 
{
	// TODO: Add your message handler code here and/or call default
	char temp[200];
	sprintf(temp,"TSDVBDemo    Received Bytes=%u",dwRecAllDataCount);
	this->SetWindowText(temp);
	CDialog::OnTimer(nIDEvent);
}
