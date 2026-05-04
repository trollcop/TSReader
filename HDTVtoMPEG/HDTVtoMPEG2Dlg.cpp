// HDTVtoMPEG2Dlg.cpp : implementation file
//
// Changelog:
//
// 1.07 - 2-23-2002 - GR: AccessDTV file support.
//						Avoid duplicate input files.
//						Support for progress bar of total conversion.
//
//

#include "stdafx.h"
#include "HDTVtoMPEG2.h"
#include "HDTVtoMPEG2Dlg.h"
#include "process.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

static char temp[256];
static char inipath[256];

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

int get_hex(const char* str)
{
	if (strncmp(str, "0x", 2) != 0)
		return atol(str);

	int v = 0;	
	const char *p = str + 2;
	while (*p && *p != ' ')
	{
		if (*p >= '0' && *p <= '9')
			v = (v << 4) | (*p - '0');
		else if (*p >= 'a' && *p <= 'f')
			v = (v << 4) | ((*p - 'a') + 10);
		else if (*p >= 'A' && *p <= 'F')
			v = (v << 4) | ((*p - 'A') + 10);
		p++;
	}

	return v;
}

/////////////////////////////////////////////////////////////////////////////
// CHDTVtoMPEG2Dlg dialog

CHDTVtoMPEG2Dlg::CHDTVtoMPEG2Dlg(CWnd* pParent /*=NULL*/)
	: CDialog(CHDTVtoMPEG2Dlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CHDTVtoMPEG2Dlg)
	m_AudioPIDStr = _T("");
	m_VideoPIDStr = _T("");
	m_MaxFileSize = 0;
	//}}AFX_DATA_INIT
	// Note that LoadIcon does not require a subsequent DestroyIcon in Win32

	GetModuleFileName(NULL, inipath, 256);
	char *p = strrchr(inipath, '\\');
	if (!p)
		p = inipath;
	else
		p++;
	strcpy(p, "HDTVtoMPEG2.ini");

	m_NumChannels = 0;
	memset(m_Channels, 0, sizeof(ATSC_CHANNEL_INFO) * MAXCHANNELS);

	GetPrivateProfileString("Settings", "VideoStream", "0x11", temp, 255, inipath);
	m_VideoPIDStr = temp;
	m_VideoPID = get_hex(temp);
	GetPrivateProfileString("Settings", "AudioStream", "0x14", temp, 255, inipath);
	m_AudioPIDStr = temp;
	m_AudioPID = get_hex(temp);
	GetPrivateProfileString("Settings", "MaxFileSize", "1024", temp, 255, inipath);
	m_MaxFileSize = atol(temp);

	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);

	m_bCancel = false;

	m_InputCount = 0;
	m_InputList = NULL;
	m_OutputFileName[0] = '\0';
}

void CHDTVtoMPEG2Dlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CHDTVtoMPEG2Dlg)
	DDX_Control(pDX, IDC_CHANNEL, m_ChannelCombo);
	DDX_Control(pDX, IDC_OUTPUTFILE, m_OutputFile);
	DDX_Control(pDX, IDC_INPUTFILES, m_InputFiles);
	DDX_Text(pDX, IDC_AUDIOPID, m_AudioPIDStr);
	DDX_Text(pDX, IDC_VIDEOPID, m_VideoPIDStr);
	DDX_Text(pDX, IDC_MAXFILESIZE, m_MaxFileSize);
	DDV_MinMaxInt(pDX, m_MaxFileSize, 1, 99999);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CHDTVtoMPEG2Dlg, CDialog)
	//{{AFX_MSG_MAP(CHDTVtoMPEG2Dlg)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_ADD, OnAdd)
	ON_BN_CLICKED(IDC_CLEAR, OnClear)
	ON_BN_CLICKED(IDC_CONVERT, OnConvert)
	ON_CBN_SELCHANGE(IDC_CHANNEL, OnSelchangeChannel)
	ON_EN_CHANGE(IDC_MAXFILESIZE, OnChangeMaxFileSize)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CHDTVtoMPEG2Dlg message handlers

BOOL CHDTVtoMPEG2Dlg::OnInitDialog()
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
	
	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CHDTVtoMPEG2Dlg::OnSysCommand(UINT nID, LPARAM lParam)
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

void CHDTVtoMPEG2Dlg::OnPaint() 
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
HCURSOR CHDTVtoMPEG2Dlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}

void CHDTVtoMPEG2Dlg::OnAdd() 
{
	CFileDialog fdlg(
		TRUE, "*.0000", NULL, OFN_ALLOWMULTISELECT | OFN_ENABLESIZING | OFN_FILEMUSTEXIST,
		"Transport Files|*.ts;*.trp;*.tp;*.trs;*.tps;*.ts.0*;*.adtv|All Files|*.*||", this );

	LPSTR save_file = fdlg.m_ofn.lpstrFile;
	UINT save_maxfile = fdlg.m_ofn.nMaxFile;
	fdlg.m_ofn.nMaxFile = 0x4000;
	fdlg.m_ofn.lpstrFile = new char[fdlg.m_ofn.nMaxFile];
	memset(fdlg.m_ofn.lpstrFile, 0, fdlg.m_ofn.nMaxFile);

	char path[256];
	GetPrivateProfileString("Settings", "Path", "", path, 255, inipath);
	fdlg.m_ofn.lpstrInitialDir = path;

	if (fdlg.DoModal() == IDCANCEL)
		return;

	WritePrivateProfileString("Settings", "Path", fdlg.GetPathName(), inipath);

	POSITION pos = fdlg.GetStartPosition();
	bool do_scan = m_InputFiles.GetCount() == 0;
	while (pos)
	{
		CString name = fdlg.GetNextPathName(pos);

		// Scan for channels
		if (do_scan)
		{
			scan_channel_info(name, 1024 * 1024 * 2, MAXCHANNELS, m_Channels, m_NumChannels);
			char chbuf[80];
			for (int ch = 0; ch < m_NumChannels; ch++)
			{
				sprintf(chbuf, "%s %d.%d", m_Channels[ch].name, m_Channels[ch].major_channel, m_Channels[ch].minor_channel);
				m_ChannelCombo.AddString(chbuf);
			}
			do_scan = false;
		}

		// check for duplicate files
		int m_InputCount = m_InputFiles.GetCount();
		for (int i = 0; i < m_InputCount; i++)
		{
			CString temp;
			m_InputFiles.GetText(i, temp);
			if (!stricmp(name, temp))
				break;
		}
		if (i >= m_InputCount)
			m_InputFiles.AddString(name);
	}

	// Select the channel if we haven't already
	if (m_ChannelCombo.GetCurSel() < 0)
	{
		m_ChannelCombo.SetCurSel(0);
		OnSelchangeChannel();
	}

	// Get output filename
	if (m_InputFiles.GetCount() > 0)
	{
		CString name;
		char buf[256];

		m_InputFiles.GetText(0, name);
		strcpy(buf, name);
		
		char *p = buf + strlen(buf) - 1;
		while (p != buf && *p >= '0' && *p <= '9')
			p--;
		if (p != buf && *p == '.')
			p--;
		if ((p - 2) > buf && strnicmp(p - 2, ".ts", 3) == 0)
			p -= 2;
		else if ((p - 4) > buf && strnicmp(p - 4, ".adtv", 5) == 0)
			p -= 4;
		*p = '\0';
		
		strcat(buf, "0000");

		strcat(buf, ".mpg");

		m_OutputFile.SetWindowText(buf);
	}

	delete fdlg.m_ofn.lpstrFile;
	fdlg.m_ofn.lpstrFile = save_file;
	fdlg.m_ofn.nMaxFile = save_maxfile;
}

void CHDTVtoMPEG2Dlg::OnClear() 
{
	m_InputFiles.ResetContent();
	m_ChannelCombo.ResetContent();
	m_OutputFile.SetWindowText("");
	m_NumChannels = 0;
}

void CHDTVtoMPEG2Dlg::OnChangeMaxFileSize() 
{
	UpdateData(TRUE);

	char buf[20];
	sprintf(buf, "%d", m_MaxFileSize);
	
	WritePrivateProfileString("Settings", "MaxFileSize", buf, inipath);
}

void CHDTVtoMPEG2Dlg::OnSelchangeChannel() 
{
	int ch = m_ChannelCombo.GetCurSel();
	if (ch >= 0 && ch < m_NumChannels)
	{
		char buf[10];

		sprintf(buf, "0x%X", m_Channels[ch].videopid);
		m_VideoPIDStr = buf;

		sprintf(buf, "0x%X", m_Channels[ch].audiopid);
		m_AudioPIDStr = buf;

		UpdateData(FALSE);
	}
}

void CHDTVtoMPEG2Dlg::OnConvert() 
{
	int i;

	UpdateData(TRUE);

	m_VideoPID = get_hex(m_VideoPIDStr);
	m_AudioPID = get_hex(m_AudioPIDStr);

	m_InputCount = m_InputFiles.GetCount();
	if (m_InputCount <= 0)
	{
		MessageBox("No files selected to convert!", "Error");
		return;
	}

	m_InputList = new char* [m_InputCount];
	for (i = 0; i < m_InputCount; i++)
	{
		m_InputList[i] = new char [256];
		m_InputFiles.GetText(i, m_InputList[i]);
	}

	m_OutputFile.GetWindowText(m_OutputFileName, 256);

	m_bCancel = false;

	_beginthread(StaticConvertThread, 1024 * 16, this);

	CProgressDialog pdlg;
	m_ProgressDlg.DoModal();

	m_bCancel = true;

/*	for (i = 0; i < m_InputCount; i++);
	{
		delete m_InputList[i];
	}
	delete m_InputList;
	m_InputList = NULL;
*/}

void CHDTVtoMPEG2Dlg::StaticConvertThread(void* data)
{
	SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_ABOVE_NORMAL);

	CHDTVtoMPEG2Dlg* dlg = (CHDTVtoMPEG2Dlg*)data;
	dlg->ConvertThread();
}

void CHDTVtoMPEG2Dlg::ConvertThread()
{
	convert_files(m_InputList, m_InputCount, m_OutputFileName, 
		m_VideoPID, m_AudioPID, m_MaxFileSize, StaticConvertCallback, this);

	m_ProgressDlg.m_Finished = true;
}

bool CHDTVtoMPEG2Dlg::StaticConvertCallback(int ifilenum, char* ifilename, char* ofilename, 
   __int64 curfilesize, __int64 curfilepos, __int64 curtotalsize, __int64 curtotalpos, void* data)
{
	CHDTVtoMPEG2Dlg* dlg = (CHDTVtoMPEG2Dlg*)data;
	return dlg->ConvertCallback(ifilenum, ifilename, ofilename, curfilesize, curfilepos, curtotalsize, curtotalpos);
}

bool CHDTVtoMPEG2Dlg::ConvertCallback(int ifilenum, char* ifilename, char* ofilename, 
   __int64 curfilesize, __int64 curfilepos, __int64 curtotalsize, __int64 curtotalpos)
{
	if (m_bCancel)
		return false;

	char *p = strrchr(ifilename, '\\') + 1;
	if (!p || !(*p))
		p = ifilename;
	strcpy(m_ProgressDlg.m_InputFileName, p);

	p = strrchr(ofilename, '\\') + 1;
	if (!p || !(*p))
		p = ofilename;
	strcpy(m_ProgressDlg.m_OutputFileName, p);

	__int64 progress = curfilepos * (__int64)1000 / curfilesize;
	m_ProgressDlg.m_ProgressPos = (int)progress;

	progress = curtotalpos * (__int64)1000 / curtotalsize;
	m_ProgressDlg.m_ProgressTotalPos = (int)progress;

	return true;
}
