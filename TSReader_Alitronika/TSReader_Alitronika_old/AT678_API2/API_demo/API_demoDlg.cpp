// DTV_testDlg.cpp : implementation file
//

#include "stdafx.h"
#include "API_demo.h"
#include "API_demoDlg.h"

#include <ATDV_API.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define STATUS_TIMER    60
#define STATUS_MS		200
#define RECORD_BUFFERSIZE (1*1024*1024)
#define PLAY_BUFFERSIZE (188*1000)
#define PLAY_BITRATE 38e6

// Program Variables
BOOL s_bRecordThreadActive;
BOOL s_bPlayThreadActive;

int counter = 0;
ULONG bytesread = 0;

// ATboard
CAtBoardManager* m_pAlitronManager;
CATBoard* s_pAlitronBoard;
CAtDeviceList devlist;
ATDEVICEINFO iDevice;

//Record
static BYTE *s_recBuffer;
static BYTE *s_playBuffer;


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
// CDTV_testDlg dialog

CDTV_testDlg::CDTV_testDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CDTV_testDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CDTV_testDlg)
	m_ModeValue = -1;
	//}}AFX_DATA_INIT
	// Note that LoadIcon does not require a subsequent DestroyIcon in Win32
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CDTV_testDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDTV_testDlg)
	DDX_Control(pDX, IDC_RECPLAYMODE, m_RecPlayMode);
	DDX_Control(pDX, IDC_STATICREADCYCLES, m_ReadCycles);
	DDX_Control(pDX, IDC_DATALIST, m_DataList);
	DDX_Control(pDX, IDC_STATICBYTESREAD, m_BytesRead);
	DDX_Control(pDX, IDC_STATICBITRATE, m_sBitRate);
	DDX_Control(pDX, IDC_STATICDATAERR, m_sDataErr);
	DDX_Control(pDX, IDC_STATICSYNCLOSS, m_sSyncErr);
	DDX_Control(pDX, IDC_STATICFILENAME, m_sFileName);
	DDX_Control(pDX, IDC_STATICBOARDNAME, m_sBoardName);
	DDX_Radio(pDX, IDC_RECPLAYMODE, m_ModeValue);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CDTV_testDlg, CDialog)
	//{{AFX_MSG_MAP(CDTV_testDlg)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_STARTBTN, OnStartbtn)
	ON_BN_CLICKED(IDC_STOPBTN, OnStopbtn)
	ON_WM_TIMER()
	ON_BN_CLICKED(IDC_RECPLAYMODE, OnRecplaymode)
	ON_BN_CLICKED(IDC_RECRADIO, OnRecplaymode)
	ON_BN_CLICKED(IDC_PLAYRADIO, OnRecplaymode)
	ON_BN_CLICKED(IDC_RESETBTN, OnResetbtn)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDTV_testDlg message handlers

BOOL CDTV_testDlg::OnInitDialog()
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

	InitATboard();

	m_DataList.InsertColumn(0,"0");
	m_DataList.InsertColumn(1,"1");
	m_DataList.InsertColumn(2,"2");
	m_DataList.InsertColumn(3,"3");
	m_DataList.InsertColumn(4,"4");
	m_DataList.InsertColumn(5,"5");
	m_DataList.InsertColumn(6,"6");
	m_DataList.InsertColumn(7,"7");
	m_DataList.InsertColumn(8,"8");
	m_DataList.InsertColumn(9,"9");
	m_DataList.SetColumnWidth(0,40);
	m_DataList.SetColumnWidth(1,40);
	m_DataList.SetColumnWidth(2,40);
	m_DataList.SetColumnWidth(3,40);
	m_DataList.SetColumnWidth(4,40);
	m_DataList.SetColumnWidth(5,40);
	m_DataList.SetColumnWidth(6,40);
	m_DataList.SetColumnWidth(7,40);
	m_DataList.SetColumnWidth(8,40);
	m_DataList.SetColumnWidth(9,40);

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CDTV_testDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

void CDTV_testDlg::OnPaint() 
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
HCURSOR CDTV_testDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}

void CDTV_testDlg::OnStartbtn() 
{
	if (s_pAlitronBoard == NULL)
		return;

	s_pAlitronBoard->ResetCounters();
	
	switch(m_ModeValue)
	{
	case 1: // Record
		if (!s_bRecordThreadActive) //Only start 1 rec thread
		{
			TRACE("Start recording \n\r");
			s_bRecordThreadActive = TRUE;
			m_pRecordThread = AfxBeginThread(RecordThread, this, THREAD_PRIORITY_BELOW_NORMAL);
		}
	break;
	
	case 2: // Play
		if (!s_bPlayThreadActive) //Only start 1 play thread
		{
			TRACE("Start playing \n\r");
			s_bPlayThreadActive = TRUE;
			m_pPlayThread = AfxBeginThread(PlayThread, this, THREAD_PRIORITY_BELOW_NORMAL);
		}
	break;
	}
}

void CDTV_testDlg::OnStopbtn() 
{
	switch(m_ModeValue)
	{
	case 1: // Record
		TRACE("Stop recording \n\r");
		s_bRecordThreadActive = FALSE;
	break;

	case 2: // Play
		TRACE("Stop playing \n\r");
		s_bPlayThreadActive = FALSE;
	break;
	}
}


UINT CDTV_testDlg::PlayThread(LPVOID pParam)
{	
	TRACE("Playing .... \n\r");
	bytesread = 0;
	counter = 0;

	s_playBuffer = (byte*)malloc(PLAY_BUFFERSIZE + 1);
    memset(s_playBuffer, 0, PLAY_BUFFERSIZE);
	
	for (int i = 0 ; i < PLAY_BUFFERSIZE / 188 ; i++)
	{
		int pos = i * 188;
		s_playBuffer[pos]=0x47;
		s_playBuffer[pos+1]=0x1F;
		s_playBuffer[pos+2]=0xFF;
	}

	{	//providing scope for: regs
		CAtRegisters RegAcc(s_pAlitronBoard);ATREGISTRY &Registers = RegAcc;
		Registers.m_PlayConfig &= ~(AT_PCONF_DVB | AT_PCONF_SMP | AT_PCONF_RAW | AT_PCONF_GTP);
		Registers.m_PlayConfig |= AT_PCONF_PENA;
		Registers.m_PlayConfig |= AT_PCONF_DVB;
		Registers.m_PlayConfig &= ~(AT_PCONF_NTP | AT_PCONF_CTP);
		Registers.m_PlayConfig |= AT_PCONF_188;		//Use 188 byte packets
		Registers.m_Bitrate = PLAY_BITRATE;			//Set bitrate
		Registers.m_PlayConfig |= AT_PCONF_SER;		//Enable Serial OUT
		s_pAlitronBoard->UpdateRegisters();
	}	//regs out of scope and released....


	if (s_pAlitronBoard->StartPlaying())
	{
		while(s_bPlayThreadActive)
		{
			while (!s_pAlitronBoard->SendPlayPacketDirect(s_playBuffer, PLAY_BUFFERSIZE) && s_bPlayThreadActive)
			Sleep(1);
			
			counter++;
			bytesread += PLAY_BUFFERSIZE;
			TRACE("WRITE %i bytes\n\r",PLAY_BUFFERSIZE);
		}
	}

	s_pAlitronBoard->StopPlaying();	
	free (s_playBuffer);
	s_playBuffer = NULL;
	s_bPlayThreadActive = FALSE;


	TRACE("Play Thread ended \n\r");
	return NULL;
}


UINT CDTV_testDlg::RecordThread(LPVOID pParam)
{	
	TRACE("Recording .... \n\r");

	int res = 0;
	ULONG NrBytes = 0;
	bytesread = 0;

	counter = 0;
	s_recBuffer = (byte*)malloc(RECORD_BUFFERSIZE + 1);
    memset(s_recBuffer, 0, RECORD_BUFFERSIZE);
 
	{	
		CAtRegisters RegAcc(s_pAlitronBoard);ATREGISTRY &Registers = RegAcc;
		
		//Set source mode at DVB
		RegAcc.m_Registers.m_RecordConfig &= ~(AT_RCONF_DVB|AT_RCONF_SMP); 
		RegAcc.m_Registers.m_RecordConfig |= AT_RCONF_DVB; 
		//Set source input at Serial	
		RegAcc.m_Registers.m_RecordConfig &= ~(AT_RCONF_SER|AT_RCONF_PAR);
		RegAcc.m_Registers.m_RecordConfig |= AT_RCONF_SER; 
	
		Registers.m_RecordConfig |= AT_RCONF_RENA;
		RegAcc.m_Registers.m_RecordConfig = 5;
		RegAcc.m_Registers.m_DvbSmpte = 0; //SET
	
		s_pAlitronBoard->UpdateRegisters();

		s_pAlitronBoard->InitBitrateFilter(STATUS_MS);
	}

	if (s_pAlitronBoard->StartRecording())
    {
		if (!s_pAlitronBoard->IsUsbDeviceHighSpeed())
		{
			TRACE("The device is not connected to a HIGH speed USB port.\nRecord will not work properly!!");
			AfxMessageBox("The device is not connected to a HIGH speed USB port.\nRecord will not work properly!!");
		}

		while(s_bRecordThreadActive)
		{	
			if (s_pAlitronBoard->GetRecordPacketDirect(s_recBuffer, RECORD_BUFFERSIZE, &NrBytes))
			{	
			//	TRACE("GetRecordPacketDirect returns 0\n\r");
			}
			if (NrBytes != 0)
			{	
				counter++;
				bytesread += NrBytes;
				TRACE("READ %i bytes\n\r",NrBytes);
			}
		}
	}	
	
	
	s_pAlitronBoard->StopRecording();	
	free (s_recBuffer);
	s_recBuffer = NULL;
	s_bRecordThreadActive = FALSE;
	TRACE("Recording Thread ended \n\r");
	
	return NULL;
}


void CDTV_testDlg::OnTimer(UINT nIDEvent) 
{
	CString temp;

	switch (nIDEvent)
	{
	case STATUS_TIMER:
	if(devlist.Names.size())
	{	
		temp.Format("%i",counter);
		m_ReadCycles.SetWindowText(temp);

		temp.Format("%.2f MB",bytesread/1e6);
		m_BytesRead.SetWindowText(temp);

		//Show boardname
		char boardname[40];	
		ULONG size = 40;
		s_pAlitronBoard->GetFriendlyName(boardname,size);
		m_sBoardName.SetWindowText(boardname);

		//Show FPGA filename
		std::string FPGAFilename;
		FPGAFilename = s_pAlitronBoard->GetFPGAFirmwareName();
		m_sFileName.SetWindowText(FPGAFilename.data());

		if(devlist.Names.size() > 0)
		{
			int Bitrate = 0; 
			
			s_pAlitronBoard->GetRegisters();
		
			switch(m_ModeValue)
			{
			case 1: //rec mode
				Bitrate = s_pAlitronBoard->GetInputBitrate();
				temp.Format("%.6f", Bitrate/1e6);
				m_sBitRate.SetWindowText(temp);
			break;
			
			case 2: //play mode
				temp.Format("%.6f", PLAY_BITRATE/1e6);
				m_sBitRate.SetWindowText(temp);
	
			break;
			}

			temp.Format("%u", s_pAlitronBoard->GetErrorCount());
			m_sDataErr.SetWindowText(temp);
	

			temp.Format("%u", s_pAlitronBoard->GetSyncCount());
			m_sSyncErr.SetWindowText(temp);

			
			CAtRegisters RegAcc(s_pAlitronBoard);
			ATREGISTRY &Regs = RegAcc;
				
			s_pAlitronBoard->UpdateRegisters();
		}

		if(s_recBuffer)
		{
			CString temp;
			temp.Format("%x",s_recBuffer[0]);
			int ItemNr = m_DataList.GetItemCount();
			m_DataList.InsertItem(ItemNr,temp);
			
			for(int i = 1; i < 10; i++)
			{	
				temp.Format("%x",s_recBuffer[i]);
				m_DataList.SetItem(ItemNr,i,LVIF_TEXT,temp,0,0,0,0);
			}

			m_DataList.EnsureVisible(m_DataList.GetItemCount()-1,FALSE);
		}
	}
	break;
	}
	CDialog::OnTimer(nIDEvent);
}


void CDTV_testDlg::InitATboard()
{

    int card_cout = 0;     

	m_pAlitronManager = NULL;
	m_pAlitronManager = CAtBoardManager::Instance();
	m_pAlitronManager->GetDeviceList(devlist);

	card_cout = devlist.Names.size();
	if ( card_cout <= 0 )
	{
		TRACE("No atboard\r\n");  
		return;
	}
	else
	{
        s_pAlitronBoard = m_pAlitronManager->GetBoard(devlist.Names[0]);
        s_pAlitronBoard->GetDeviceInfo(&iDevice);
        switch(iDevice.DeviceType)
		{
		case AT20:
		case AT200:
		// Devices can only record
		m_ModeValue = 1;
		UpdateData(false);
		OnRecplaymode();
		GetDlgItem(IDC_PLAYRADIO)->EnableWindow(FALSE); //Disable play function
		SetTimer(STATUS_TIMER, STATUS_MS, NULL); //Only start the timer if a valid board is detected
		break;
		
		case AT40:
		case AT400:
		case AT1400:
		// Device can do both
		m_ModeValue = 1;
		UpdateData(false);
		OnRecplaymode();
		SetTimer(STATUS_TIMER, STATUS_MS, NULL); //Only start the timer if a valid board is detected
		break;

		case AT30:
		case AT300:
		case AT600:
		case AT700:
		case AT800:
		case AT1600:
		case AT1700:
		case AT1800:
		// Devices can only play
		m_ModeValue = 2;
		UpdateData(false);
		OnRecplaymode();
		GetDlgItem(IDC_RECRADIO)->EnableWindow(FALSE); //Disable record function
		SetTimer(STATUS_TIMER, STATUS_MS, NULL); //Only start the timer if a valid board is detected
		break;

		default:
		m_pAlitronManager->ReleaseBoard(s_pAlitronBoard);
		m_pAlitronManager = NULL;
		TRACE("NO ATboard\r\n");
		AfxMessageBox("No valid ATboard is connected.\nPlease check ABOUT for more information.");
		break;
  		}
	}
}

void CDTV_testDlg::OnRecplaymode() 
{
	UpdateData(TRUE);
	
	if (s_pAlitronBoard == NULL)
		return;

	switch(m_ModeValue)
	{
		case 1:
			GetDlgItem(IDC_DATALIST)->EnableWindow(TRUE);
			s_bPlayThreadActive = FALSE;
			s_pAlitronBoard->ProgramFPGA(REC_DVB);
			s_pAlitronBoard->UpdateRegisters();
		break;
		
		case 2:
			GetDlgItem(IDC_DATALIST)->EnableWindow(FALSE);
			s_bRecordThreadActive = FALSE;
			s_pAlitronBoard->ProgramFPGA(PLAY_DVB);
			s_pAlitronBoard->UpdateRegisters();
		break;
	}
}

void CDTV_testDlg::OnResetbtn() 
{
	m_DataList.DeleteAllItems();
	m_sDataErr.SetWindowText("0");
	m_sSyncErr.SetWindowText("0");
	m_sBitRate.SetWindowText("0.000000");
}
