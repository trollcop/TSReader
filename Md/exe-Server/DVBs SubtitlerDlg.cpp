// DVBs SubtitlerDlg.cpp : implementation file
//

#include "stdafx.h"
#include "DVBs Subtitler.h"
#include "DVBs SubtitlerDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif




char SubtitleFileName[100];
char SubtitleFileTitle[100];
char OutputFolder[100]="C:";
char pomTime[80];
int Frame=0;
int StartFrame=0;
int EndFrame=0;
char SubTitle1[100];
char SubTitle2[100];
OPENFILENAME ofn;

typedef struct{
    int sat;
    int min;
    int sec;
    int mil;
} MyTime;


MyTime MyCurrentTime;
MyTime MyMovieTime;
MyTime MyStartTime;
MyTime ShowTime;
MyTime OffsetTime;
MyTime MyDelayTime;
MyTime MyTimeFromFrame(int);
MyTime GetMyCurrentTime(void);
MyTime ResetMyTime(void);
int FrameFromTime(MyTime);
void PrintToFile(char*,char*);
BOOL IsPlaying=FALSE;
BOOL TitleOnScreen=FALSE;
BOOL SubFile=FALSE;
BOOL GetTitle(int*, int*, char*, char*, int);
int GetPrevFrame(int);
MyTime MyTimeSub(MyTime,MyTime);
MyTime MyTimeAdd(MyTime,MyTime);

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
// CDVBsSubtitlerDlg dialog

CDVBsSubtitlerDlg::CDVBsSubtitlerDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CDVBsSubtitlerDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CDVBsSubtitlerDlg)
	m_SubtitleFileName = _T("");
	m_CurrentTime = _T("");
	m_MovieTime = _T("");
	m_Subtitle1 = _T("");
	m_Subtitle2 = _T("");
	m_CurrentFrame = 0;
	m_StartFrame = 0;
	m_ExpireFrame = 0;
	m_OSD1 = _T("");
	m_OSD2 = _T("");
	m_offset = _T("");
	m_OUTPUT = _T("");
	//}}AFX_DATA_INIT
	// Note that LoadIcon does not require a subsequent DestroyIcon in Win32
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CDVBsSubtitlerDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDVBsSubtitlerDlg)
	DDX_Control(pDX, IDC_SLIDER, m_SliderControl);
	DDX_Text(pDX, IDC_SubtitleFileName, m_SubtitleFileName);
	DDV_MaxChars(pDX, m_SubtitleFileName, 80);
	DDX_Text(pDX, IDC_CurrentTime, m_CurrentTime);
	DDV_MaxChars(pDX, m_CurrentTime, 20);
	DDX_Text(pDX, IDC_MovieTime, m_MovieTime);
	DDV_MaxChars(pDX, m_MovieTime, 30);
	DDX_Text(pDX, IDC_Subtitle1, m_Subtitle1);
	DDV_MaxChars(pDX, m_Subtitle1, 100);
	DDX_Text(pDX, IDC_Subtitle2, m_Subtitle2);
	DDV_MaxChars(pDX, m_Subtitle2, 100);
	DDX_Text(pDX, IDC_CurrentFrame, m_CurrentFrame);
	DDX_Text(pDX, IDC_StartFrame, m_StartFrame);
	DDX_Text(pDX, IDC_ExpireFrame, m_ExpireFrame);
	DDX_Text(pDX, IDC_OSD1, m_OSD1);
	DDV_MaxChars(pDX, m_OSD1, 100);
	DDX_Text(pDX, IDC_OSD2, m_OSD2);
	DDV_MaxChars(pDX, m_OSD2, 100);
	DDX_Text(pDX, IDC_OFFSET, m_offset);
	DDX_Text(pDX, IDC_OUTPUT, m_OUTPUT);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CDVBsSubtitlerDlg, CDialog)
	//{{AFX_MSG_MAP(CDVBsSubtitlerDlg)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_SubtitleFileselect, OnSubtitleFileselect)
	ON_BN_CLICKED(IDC_Start, OnStart)
	ON_BN_CLICKED(ID_Exit, OnExit)
	ON_WM_TIMER()
	ON_BN_CLICKED(IDC_Stop, OnStop)
	ON_BN_CLICKED(IDC_Plus25, OnPlus25)
	ON_BN_CLICKED(IDC_Plus5, OnPlus5)
	ON_BN_CLICKED(IDC_Minus25, OnMinus25)
	ON_BN_CLICKED(IDC_Minus5, OnMinus5)
	ON_BN_CLICKED(IDC_Plus6, OnPlus6)
	ON_BN_CLICKED(IDC_Minus26, OnMinus26)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_SLIDER, OnCustomdrawSlider)
	ON_BN_CLICKED(IDC_NEXT, OnNext)
	ON_BN_CLICKED(IDC_OUTPUT_FILE, OnOutputFile)
	ON_BN_CLICKED(IDC_BUTTON1, OnButton1)
	ON_BN_CLICKED(IDC_PREV, OnPrev)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDVBsSubtitlerDlg message handlers

BOOL CDVBsSubtitlerDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon
	
	// TODO: Add extra initialization here
	int iInstallResult;
	
	iInstallResult=SetTimer(1,40,NULL);
	if(iInstallResult==0){
		MessageBox("Cannot install timer!");
	}
	m_SliderControl.SetRange(0,270000,TRUE);
			
	MyCurrentTime=GetMyCurrentTime();
	MyDelayTime=ResetMyTime();	 //Set DelayTime to 0

	return TRUE;  // return TRUE  unless you set the focus to a control
	
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CDVBsSubtitlerDlg::OnPaint() 
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

		// Draw th1000n(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

// The system calls this to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CDVBsSubtitlerDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}



void CDVBsSubtitlerDlg::OnSubtitleFileselect() 
{
	// TODO: Add your control notification handler code here
	

//	IsPlaying=FALSE;
//	Frame=0;
	SubFile=FALSE;
	memset(&ofn,0,sizeof(ofn));
	ofn.lStructSize=sizeof(OPENFILENAME);
	ofn.hwndOwner=NULL;
	ofn.hInstance=NULL;

	ofn.lpstrFilter=TEXT("MicroDVD subtitle (*.sub;*.txt)\0*.sub;*.txt\0All Files (*.*)\0*.*\0\0");

	ofn.lpstrCustomFilter=NULL;
	ofn.nMaxCustFilter=0;
	ofn.nFilterIndex=1;
	ofn.lpstrFile=SubtitleFileName;
	ofn.nMaxFile=500;
	ofn.lpstrFileTitle=SubtitleFileTitle;
	ofn.nMaxFileTitle=99;
	ofn.lpstrInitialDir=NULL;
	ofn.lpstrTitle="Open Subtitle file";
	ofn.Flags=OFN_FILEMUSTEXIST;
	ofn.lpstrDefExt="sub";
	ofn.lCustData=NULL;
	ofn.lpfnHook=NULL;
	ofn.lpTemplateName=NULL;

	SubtitleFileName[0]='\0';

	SubFile=GetOpenFileName(&ofn);
	m_SubtitleFileName=SubtitleFileTitle;

	if(SubFile){
		if(!GetTitle(&StartFrame,&EndFrame,SubTitle1,SubTitle2,0)){
			MessageBox("Wrong subtitle format, please select another file!");
		}

	}
	UpdateData(FALSE);
	UpdateWindow();
}


void CDVBsSubtitlerDlg::OnExit() 
{
	// TODO: Add your control notification handler code here
	char fname[100];
	KillTimer(1);
	strcpy(fname,OutputFolder);
	strcat(fname,"\\SubData.txt");
	DeleteFile(fname);
    OnOK();
	
}




void CDVBsSubtitlerDlg::OnTimer(UINT nIDEvent) 
{
	char pom[20];

	// TODO: Add your message handler code here and/or call default

	MyCurrentTime=GetMyCurrentTime();
	if (IsPlaying){
		MyMovieTime=MyTimeSub(MyCurrentTime,MyStartTime);
		MyMovieTime=MyTimeAdd(MyMovieTime,MyDelayTime);
		Frame=FrameFromTime(MyMovieTime);	
		if(Frame>StartFrame&&Frame<EndFrame){
			if(!TitleOnScreen){
				TitleOnScreen=TRUE;
				//Ovdje ubaci ispis na ekran
				m_OSD1=SubTitle1;
				m_OSD2=SubTitle2;
				PrintToFile(SubTitle1,SubTitle2);
			}
		}else{
				//Brisi OSD
				TitleOnScreen=FALSE;
				m_OSD1='\0';
				m_OSD2='\0';
				PrintToFile(" "," ");
		}
	}

	m_SliderControl.SetPos(Frame);	
	sprintf(pom,"%d:%d:%d.%d",MyCurrentTime.sat,MyCurrentTime.min,MyCurrentTime.sec,MyCurrentTime.mil);
	m_CurrentTime=pom;
	strcpy(pom,OutputFolder);
	strcat(pom,"\\SubData.txt");
	m_OUTPUT=pom;
	ShowTime=MyTimeFromFrame(Frame);
	OffsetTime=MyTimeSub(MyCurrentTime,ShowTime);
	sprintf(pom,"%d:%d",OffsetTime.min,OffsetTime.sec);
	m_offset=pom;

	

	sprintf(pom,"%d:%d:%d.%d",ShowTime.sat,ShowTime.min,ShowTime.sec,ShowTime.mil);
	m_MovieTime=pom;

	
	m_CurrentFrame=Frame;

	if(SubFile&&((Frame<StartFrame)||(Frame>=EndFrame))) GetTitle(&StartFrame,&EndFrame,SubTitle1,SubTitle2,Frame);
	m_StartFrame=StartFrame;
	m_ExpireFrame=EndFrame;
	m_Subtitle1=SubTitle1;
	m_Subtitle2=SubTitle2;
	

	UpdateData(FALSE);
	UpdateWindow();
	CDialog::OnTimer(nIDEvent);
}




BOOL GetTitle(int *LStartFrame, int *LEndFrame, char *LSub1, char *LSub2, int LFrame){

	CFile f;
	char Line[200];
	char PSub1[200];
	char PSub2[200];
	int i;
	int j;
	int lline;

	f.Open(SubtitleFileName,CFile::modeRead);
	CArchive ar(&f,CArchive::load);

	while(ar.ReadString( Line,200 )){
	
		lline=strlen(Line);
		// citanje pocetnog framea
		i=0;j=0;
		while(Line[i+1]!='}'){
			i++;
			if(i>lline) return FALSE;
			PSub1[j]=Line[i];
			j++;
			
		}
		PSub1[j]='\0';
		*LStartFrame=atoi(PSub1);	// LStartFrame - pocetni frame
		if (*LStartFrame >= LFrame){ // Pronasli smo title, procitajno i EndFrame

									// citanje EndFrame
			i++;i++;j=0;
			while(Line[i+1]!='}'){
				i++;
				if(i>lline) return FALSE;
				
				PSub2[j]=Line[i];
				j++;
			}
			PSub2[j]='\0';
			*LEndFrame=atoi(PSub2);		// Evo EndFrame-a
			
			

			
			// inicijalno nema titlova
			PSub1[0]='\0';
			PSub2[0]='\0';
			// citanje prve linije titla
			i++;j=0;
			while((Line[i+1]!='|')&&(Line[i+1]!='\0') ){
				i++;
				if(i>lline) return FALSE;
				PSub1[j]=Line[i];
				j++;
			}
			PSub1[j]='\0';			// PSub1 - prva linija titla

			if(Line[i+1]=='|') i++;
			j=0;
			while(Line[i+1]!='\0'){
				i++;
				if(i>lline) return FALSE;
				PSub2[j]=Line[i];
				j++;
			}
			PSub2[j]='\0';			// PSub2 - druga linija titla
			ar.Close();
			f.Close();
			strcpy(LSub1,PSub1);
			strcpy(LSub2,PSub2);
			return(TRUE);			// Mozemo van, uspjeli smo!
		}
	}
	ar.Close();
	f.Close();		
	return(FALSE);					// Dosli smo do kraja i nismo pronasli frame
}
int GetPrevFrame(int LFrame){

	CFile f;
	char Line[200];
	char PSub1[200];
	int SFrame;
	int PFrame=0;
	int i;
	int j;
	int lline;

	f.Open(SubtitleFileName,CFile::modeRead);
	CArchive ar(&f,CArchive::load);

	while(ar.ReadString( Line,200 )){
	
		lline=strlen(Line);
		// citanje pocetnog framea
		i=0;j=0;
		while(Line[i+1]!='}'){
			i++;
			if(i>lline) return(0);
			PSub1[j]=Line[i];
			j++;
			
		}
		PSub1[j]='\0';
		SFrame=atoi(PSub1);	// SFrame - pocetni frame
		if (SFrame >= LFrame){ // Ako je pronadjeni frame = trazenom frameu
			ar.Close();
			f.Close();
			return PFrame;     //vrati frame prethodnog titla
		}else{
			PFrame=SFrame;     //inace idemo za jedan dalje
		}
	}
	ar.Close();
	f.Close();		
	return(0);					// Dosli smo do kraja i nismo pronasli frame
}
void CDVBsSubtitlerDlg::OnStart() 
{
	// TODO: Add your control notification handler code here
	IsPlaying=!IsPlaying;
	if(IsPlaying){
		MyStartTime=GetMyCurrentTime();

	}
	else
	{
		MyDelayTime=MyMovieTime;
	}
}

void CDVBsSubtitlerDlg::OnStop() //Actualli, Reset
{
	// TODO: Add your control notification handler code here
	Frame=0;
	IsPlaying=FALSE;
	MyStartTime=GetMyCurrentTime();
	MyDelayTime=ResetMyTime();
	
}


void CDVBsSubtitlerDlg::OnPlus25() 
{
	// TODO: Add your control notification handler code here
	Frame+=25;
	MyDelayTime=MyTimeFromFrame(Frame);
	MyStartTime=GetMyCurrentTime();
}

void CDVBsSubtitlerDlg::OnPlus5() 
{
	// TODO: Add your control notification handler code here
	Frame+=5;
	MyDelayTime=MyTimeFromFrame(Frame);
	MyStartTime=GetMyCurrentTime();
}

void CDVBsSubtitlerDlg::OnMinus25() 
{
	// TODO: Add your control notification handler code here
	Frame-=25;
	MyDelayTime=MyTimeFromFrame(Frame);
	MyStartTime=GetMyCurrentTime();
	
}

void CDVBsSubtitlerDlg::OnMinus5() 
{
	// TODO: Add your control notification handler code here
	Frame-=5;
	MyDelayTime=MyTimeFromFrame(Frame);
	MyStartTime=GetMyCurrentTime();
	
}

void CDVBsSubtitlerDlg::OnPlus6() 
{
	// TODO: Add your control notification handler code here
	Frame+=750;
	MyDelayTime=MyTimeFromFrame(Frame);
	MyStartTime=GetMyCurrentTime();

}

void CDVBsSubtitlerDlg::OnMinus26() 
{
	// TODO: Add your control notification handler code here
	Frame-=750;
	MyDelayTime=MyTimeFromFrame(Frame);
	MyStartTime=GetMyCurrentTime();
	
}


void CDVBsSubtitlerDlg::OnCustomdrawSlider(NMHDR* pNMHDR, LRESULT* pResult) 
{
	int i;
	// TODO: Add your control notification handler code here
	i=m_SliderControl.GetPos();
	if(((Frame-i)>50)||((i-Frame)>50)){
		Frame=i;
		MyDelayTime=MyTimeFromFrame(Frame);
		MyStartTime=GetMyCurrentTime();
	}
	*pResult = 0;
}

void CDVBsSubtitlerDlg::OnNext() 
{
	// TODO: Add your control notification handler code here
	if((Frame>StartFrame)&&SubFile) GetTitle(&StartFrame,&EndFrame,SubTitle1,SubTitle2,EndFrame+1);
	TitleOnScreen=FALSE;
	Frame=StartFrame;
	MyDelayTime=MyTimeFromFrame(Frame);
	MyStartTime=GetMyCurrentTime();
}
void CDVBsSubtitlerDlg::OnPrev() 
{

	// TODO: Add your control notification handler code here
	if(SubFile){
		Frame=GetPrevFrame(StartFrame);
		GetTitle(&StartFrame,&EndFrame,SubTitle1,SubTitle2,Frame);
		TitleOnScreen=FALSE;
		Frame=StartFrame;
		MyDelayTime=MyTimeFromFrame(Frame);
		MyStartTime=GetMyCurrentTime();
	}

}
int FrameFromTime(MyTime STime){
	
	return(STime.sat*90000+STime.min*1500+STime.sec*25+STime.mil/40);
}

MyTime MyTimeFromFrame(int LFrame){
	MyTime MyTimePom;

	MyTimePom.sat=LFrame/90000;
	MyTimePom.min=LFrame%90000/1500;
	MyTimePom.sec=LFrame%90000%1500/25;
	MyTimePom.mil=LFrame%25*40;

	return(MyTimePom);
}


MyTime GetMyCurrentTime(void){
	SYSTEMTIME Pom;
	MyTime MyPom;
	GetSystemTime(&Pom);
	MyPom.sat=int(Pom.wHour);
	MyPom.min=int(Pom.wMinute);
	MyPom.sec=int(Pom.wSecond);
	MyPom.mil=int(Pom.wMilliseconds);
	return(MyPom);
}
MyTime ResetMyTime(void){
	MyTime MyPom;
	MyPom.mil=0;
	MyPom.sec=0;
	MyPom.min=0;
	MyPom.sat=0;
	return(MyPom);
}

MyTime MyTimeSub(MyTime MyA,MyTime MyB){

	long PomA;
	long PomB;
	long PomC;
	MyTime MyTimePom;
	
	if(MyA.sat<MyB.sat) MyA.sat+=24;
	PomA=MyA.mil+MyA.sec*1000+MyA.min*60000+MyA.sat*3600000;
	PomB=MyB.mil+MyB.sec*1000+MyB.min*60000+MyB.sat*3600000;
	PomC=PomA-PomB;

	MyTimePom.sat=PomC/3600000;
	MyTimePom.min=PomC%3600000/60000;
	MyTimePom.sec=PomC%3600000%60000/1000;
	MyTimePom.mil=PomC%1000;

	return(MyTimePom);
}

MyTime MyTimeAdd(MyTime MyA,MyTime MyB){

	long PomA;
	long PomB;
	long PomC;
	MyTime MyTimePom;
	
	PomA=MyA.mil+MyA.sec*1000+MyA.min*60000+MyA.sat*3600000;
	PomB=MyB.mil+MyB.sec*1000+MyB.min*60000+MyB.sat*3600000;
	PomC=PomA+PomB;

	MyTimePom.sat=PomC/3600000;
	MyTimePom.min=PomC%3600000/60000;
	MyTimePom.sec=PomC%3600000%60000/1000;
	MyTimePom.mil=PomC%1000;
	if(MyTimePom.sat>=24) MyTimePom.sat-=24;

	return(MyTimePom);
}


void CDVBsSubtitlerDlg::OnOutputFile() 
{
	BROWSEINFO bi = { 0 };
    bi.lpszTitle = _T("Pick a Directory");
    LPITEMIDLIST pidl = SHBrowseForFolder ( &bi );
    if ( pidl != 0 )
    {
        // get the name of the folder
        TCHAR path[MAX_PATH];
        if ( SHGetPathFromIDList ( pidl, path ) )
        {
			strcpy(OutputFolder,path);
		}else{
			strcpy(OutputFolder,"C:\\");
		}

        // free memory used
        IMalloc * imalloc = 0;
        if ( SUCCEEDED( SHGetMalloc ( &imalloc )) )
        {
            imalloc->Free ( pidl );
            imalloc->Release ( );
        }
    }
	
}

void PrintToFile(char st1[100],char st2[100])
{
	FILE *sf;
	char fname[100];
	strcpy(fname,OutputFolder);
	strcat(fname,"\\SubData.txt");
	sf=fopen(fname,"w");
	fprintf(sf,"%s\n",st1);
	fprintf(sf,"%s\n",st2);
	fclose(sf);
	return;
}

void CDVBsSubtitlerDlg::OnButton1() 
{
	CAboutDlg dlg;
	dlg.DoModal();
}


