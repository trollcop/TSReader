/****************************************************************************

    PROGRAM: MDExtern.c
 
    FUNCTIONS: Beispiel für eine Externe DLL in	MultiDec

 
*******************************************************************************/
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>        /* atoi                                  */
#include <sys/types.h>
#include <sys/stat.h>
#include <commdlg.h>

#include "MDExtern.h"
#include "resource.h"
// *****************************************
// Tu idu moje deklaracije

char MyCharCopy(char);
void TimerEvent(VOID);
UINT TimerID;
void SubInit(void);
char SubtitleFileName[100];
char SubtitleFileTitle[100];
char pomTime[80];
int Frame=0;
int StartFrame=0;
int EndFrame=0;
char SubTitle1[100];
char SubTitle2[100];
OPENFILENAME ofn;
//HWND hWnd;

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
MyTime MyDelayTime;
MyTime MyTimeFromFrame(int);
MyTime GetMyCurrentTime(void);
MyTime ResetMyTime(void);
int FrameFromTime(MyTime);

BOOL IsPlaying=FALSE;
BOOL TitleOnScreen=FALSE;

BOOL GetTitle(int*, int*, char*, char*, int, OPENFILENAME);
MyTime MyTimeSub(MyTime,MyTime);
MyTime MyTimeAdd(MyTime,MyTime);

// Kraj
// *****************************************


BOOL APIENTRY Subtitles(HWND hDlg,UINT message,UINT wParam,LONG lParam);
void Get_Current_Programm( struct TProgrammNummer *TPNr );
void Set_Current_Programm( int Nummer );
void Set_Current_Transponder(struct TTransponder *Transponder);
void Get_Current_Transponder(struct TTransponder *Transponder);
void Get_Programm_Detail( struct TProgramm *TPtg);
void Set_Programm_Detail( struct TProgramm *TPtg);
void Save_Programm_Liste( void );
void Rescan_Programm( struct TProgramm *TPtg );
void Filter_0_Irq_Proc( int MyFilter, int DatenLaenge , unsigned char *Daten);
void Start_Log( void );
void Stop_Log( void );
void Start_OSD_Subtitles( void );
void Start_OSD_Time_Demo( void );
void Create_Osd_Thread( LPTHREAD_START_ROUTINE ThreadProzess);

void OSD_CreateWindow(unsigned char Bits, unsigned short x1,unsigned short y1,unsigned short x2,unsigned short y2,BOOL Has_KeyBoard_Input);
void OSD_Draw_Block( unsigned short x,unsigned short y,unsigned short SizeX,unsigned short SizeY,unsigned short Color);
void OSD_Set_Font( unsigned short Typ,unsigned short Fg_Color,unsigned short Bg_Color);
void OSD_Write_Text( unsigned short x,unsigned short y,unsigned char *Text);
void OSD_Close( void );


/*******************************************************************************/


BOOL WINAPI DllMain (HINSTANCE hDLL, DWORD dwReason, LPVOID lpReserved)
{
static BOOL ret;            
char Name[256];

  switch (dwReason)
  {
    case DLL_PROCESS_ATTACH:
        DLLInstance=hDLL;
        GetModuleFileName (NULL, (LPTSTR) Name, 256);
        // Aufrufende Programm in Name
		break;

    case DLL_THREAD_ATTACH:
      break;

    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
      break;
  }

return TRUE;
}


/****************************************************************************
    FUNCTION:  WEP(int)

    PURPOSE:  Performs cleanup tasks when the DLL is unloaded.  WEP() is
              called automatically by Windows when the DLL is unloaded (no
              remaining tasks still have the DLL loaded).  It is strongly
              recommended that a DLL have a WEP() function, even if it does
              nothing but returns success (1), as in this example.

*******************************************************************************/

BOOL CALLBACK  _WEP(BOOL fSystemExit)
{

    return TRUE;
}



/****************************************************************************
    FUNCTION:  void On_Start(HINSTANCE MDInstance, HWND MDWnd, BOOL Log_Set )

    PURPOSE:  Wird aufgerufen nachdem die DVB´s-Karte initialisiert worden ist

*******************************************************************************/

void On_Start(HINSTANCE MDInstance, HWND MDWnd, BOOL Log_Set, int DLL_ID, char *My_Hot_Key)
{
	
	
	MultiDecInstance=MDInstance;
	MultiDecWindow=MDWnd;
    MultiDecLog = Log_Set;
    MultiDec_DLL_ID=DLL_ID;

	*My_Hot_Key='G';   // oder 0x00 für Kein HotKey

}

/****************************************************************************
    FUNCTION:  void On_Exit(  )

    PURPOSE:  Wird zum beenden von MultiDec aufgerufen 

*******************************************************************************/

void On_Exit(HINSTANCE MDInstance, HWND MDWnd, BOOL Log_Set)
{
	if ( OSD_KEY_EVENT == NULL ) CloseHandle(OSD_KEY_EVENT);
}

void On_Send_Dll_ID_Name(char *Name)
{
// An dieser Funktion idetifiziert MultiDec seine Externen DLL´s.
// Eine Dll wird angezogen, wenn diese Funktion drinnen ist.
// Der Name ist der Menüname der DLL, der in MD gesetzt werden soll.
	
	strcpy(Name,"Subtitles");
};


void On_Menu_Select(unsigned int MenuID )
{
// Entspricht in etwa der Command_Loop Mosaik95cmd// Entspricht in etwa der Command_Loop Mosaik95cmd
// Entspricht in etwa der Command_Loop Mosaik95cmd

	

// Im Menü dürfen nur ID´s von 40000 bis 41000 verwendet werden.
// Bei mehreren DLL´s MUSS dieser Wert eindeutig sein
// Sollte aber bei 1000 möglichen Menüs kein Problem sein
 
	
	if ( MenuID == 40001 ) {
		Create_Osd_Thread((LPTHREAD_START_ROUTINE)Start_OSD_Subtitles);
	};
}


void On_Channel_Change( struct TProgramm CurrentProgramm )
{

}


void On_Hot_Key( void )
{
    MessageBox(MultiDecWindow,"HotKey der DLL gedrückt","MultiDec DLL-DEMO",MB_OK);

}


void On_Rec_Play(int Mode)
{
};

void On_Osd_Key( unsigned char Key )
{
    OSD_Key_Value=Key;
	if (OSD_KEY_EVENT != NULL ) SetEvent(OSD_KEY_EVENT);
}



void Set_Current_Transponder(struct TTransponder *Transponder)
{
};

void Get_Current_Transponder(struct TTransponder *Transponder)
{
};

void Get_Current_Programm( struct TProgrammNummer *TPNr )
{
};

void Set_Current_Programm( int Nummer )
{
};

void Get_Programm_Detail( struct TProgramm *TPtg)
{
};

void Set_Programm_Detail( struct TProgramm *TPtg)
{
}

void Save_Programm_Liste( void )
{
}

void Rescan_Programm( struct TProgramm *TPtg)
{
}


void Filter_0_Irq_Proc( int MyFilter, int DatenLaenge , unsigned char *Daten)
{
}



void Start_Log( void )
{
}

void Stop_Log( void )
{
}

void On_Filter_Close(unsigned int FilterOffset )
{
 
}
void Create_Osd_Thread( LPTHREAD_START_ROUTINE ThreadProzess)
{
	DWORD LinkThreadID;
	if ( OSD_KEY_EVENT == NULL ) OSD_KEY_EVENT=CreateEvent(NULL,FALSE,FALSE,NULL);
    OSD_Key_Value=' ';
	ResetEvent(OSD_KEY_EVENT);
   
	CloseHandle(CreateThread ((LPSECURITY_ATTRIBUTES)NULL,       // No security.
                         			  (DWORD)0,                          // Same stack size.
                         			  ThreadProzess,// Thread procedure.
                         			  NULL,                         // Parameter.
                         			  (DWORD)0,                          // Start immediatly.
                         			  (LPDWORD)&LinkThreadID));  
};


void Start_OSD_Subtitles( void )
{

	DialogBox(DLLInstance,SUBTITLES,MultiDecWindow,Subtitles);	//Pokreni dialog box;
	
}

void Start_OSD_Time_Demo( void )
{
char Zeile[128];
int i;

OSD_CreateWindow(1,200,30,500,180,TRUE);
OSD_Draw_Block( 0,  0,300,30,0);
OSD_Draw_Block( 0, 30,300,90,1);
OSD_Draw_Block( 0,120,300,30,0);
OSD_Set_Font( 1, 2, 0);
strcpy(Zeile,"API - Demo");
OSD_Write_Text( 90,1,Zeile);
strcpy(Zeile,"Time-OSD");
OSD_Write_Text( 100,121,Zeile);
OSD_Set_Font( 1, 2, 1);

	i=10;
	for (;;) {
    WaitForSingleObject (OSD_KEY_EVENT, 1000 );
	ResetEvent(OSD_KEY_EVENT);
	if ( OSD_Key_Value == 0x00 ) {
                                 MessageBox(MultiDecWindow,"DLL-Thread OSD zu durch MultiDec","MultiDec DLL-DEMO",MB_OK);
		                      // MultiDec hat das Fenster geschlossen !!
		                         return;
								};
    OSD_Draw_Block( 0, 65,300,30,1);
    i--;
    sprintf(Zeile,"Ende in %02d Sek",i);
	OSD_Write_Text( 70,65,Zeile);
	if ( i == 0 ) {
								OSD_Close( );
                                return;
				};
	}

}

/**************** Usefull Helpers  ****************/


void OSD_CreateWindow(unsigned char Bits, unsigned short x1,unsigned short y1,unsigned short x2,unsigned short y2,BOOL Has_KeyBoard_Input)
{
	struct TOSD_START Osd_Start;


	// Bits 0 = 2 Farbige Fenstern ( können größ sein )
	// Bits 1 = 4 Farbige Fenster ( sollten klein sein ( Speicher auf DVB´s Karte zu klein )


	Osd_Start.BitTiefe = Bits ;     // Kleine Fenster mit 4 Farben 
    Osd_Start.DLL_Id=MultiDec_DLL_ID;
	Osd_Start.x1=x1;
	Osd_Start.y1=y1;
	Osd_Start.x2=x2;
	Osd_Start.y2=y2;
	Osd_Start.Input=Has_KeyBoard_Input;        
	SendMessage(MultiDecWindow,WM_USER,MDAPI_START_OSD,(LONG)&Osd_Start);

};

void OSD_Draw_Block( unsigned short x,unsigned short y,unsigned short SizeX,unsigned short SizeY,unsigned short Color)
{
	struct TOSD_DRAW  Osd_Draw;
	Osd_Draw.x=x;
	Osd_Draw.y=y;
	Osd_Draw.Sizex=SizeX;    
	Osd_Draw.Sizey=SizeY;    
    Osd_Draw.Color=Color;
	SendMessage(MultiDecWindow,WM_USER,MDAPI_OSD_DRAWBLOCK,(LONG)&Osd_Draw);
};


void OSD_Set_Font( unsigned short Typ,unsigned short Fg_Color,unsigned short Bg_Color)
{
	struct TOSD_SETFONT  Osd_Set_Font;
	Osd_Set_Font.Typ=Typ;
	Osd_Set_Font.Fg_Color=Fg_Color;
	Osd_Set_Font.Bg_Color=Bg_Color;
	SendMessage(MultiDecWindow,WM_USER,MDAPI_OSD_SETFONT,(LONG)&Osd_Set_Font);
};

void OSD_Write_Text( unsigned short x,unsigned short y,unsigned char *Text)
{
	struct TOSD_SETEXT  Osd_Set_Text;
	Osd_Set_Text.x=x;
	Osd_Set_Text.y=y;
	strncpy(Osd_Set_Text.Zeile,Text,128);
	SendMessage(MultiDecWindow,WM_USER,MDAPI_OSD_TEXT,(LONG)&Osd_Set_Text);
};


void OSD_Close(  void )
{
	SendMessage(MultiDecWindow,WM_USER,MDAPI_STOP_OSD,0);
};

void SubInit(void)
{
}

void TimerEvent(VOID)
{
}

BOOL APIENTRY Subtitles(HWND hDlg,UINT message,UINT wParam,LONG lParam)
{
	char pom1[20],pom2[20];
	int x,y,i,j,sl1,sl2;

	switch (message) {
        case WM_INITDIALOG:
			SetTimer(hDlg,1,80,NULL);
			MyCurrentTime=GetMyCurrentTime();
			MyDelayTime=ResetMyTime();	 //Set DelayTime to 0
		break;


		case WM_MOUSEMOVE:
		
			y=HIWORD( lParam );
			x=LOWORD( lParam );

			if (wParam == MK_LBUTTON) {
			
				if (( x >= 150 ) && ( x <= 360 )) {
					
					if (( y >= 112 ) && ( y <= 158 )) {   // SLIDER !!!
						i=(int)((double)(x-150)/210*270000);
						if ( i < 0) i=0;
						else if ( i >  270000) i=270000;
						x=(int)(150+((double)(i)/270000*210));
						MoveWindow(GetDlgItem(hDlg,SLIDER),x-4,112,8,30,TRUE);
						Frame=i;
						IsPlaying=FALSE;
						MyDelayTime=MyTimeFromFrame(Frame);
						MyStartTime=GetMyCurrentTime();
					} 
				}
			}

			break;
		case WM_COMMAND:
			if ((LOWORD(wParam) == SUB_EXIT ) || (LOWORD(wParam) == IDCANCEL )) {
				EndDialog(hDlg,TRUE);
			}
			
			if (LOWORD(wParam) == RESET) {
				Frame=0;
				IsPlaying=FALSE;
				MyStartTime=GetMyCurrentTime();
				MyDelayTime=ResetMyTime();
			}
			if (LOWORD(wParam) == NEXT) {
				Frame=StartFrame;
				MyDelayTime=MyTimeFromFrame(Frame);
				MyStartTime=GetMyCurrentTime();
			}
			if (LOWORD(wParam) == MINUS30) {
				Frame-=750;
				MyDelayTime=MyTimeFromFrame(Frame);
				MyStartTime=GetMyCurrentTime();

			}
			if (LOWORD(wParam) == MINUS1) {
				Frame-=25;
				MyDelayTime=MyTimeFromFrame(Frame);
				MyStartTime=GetMyCurrentTime();
			}
			if (LOWORD(wParam) == MINUS5) {
				Frame-=5;
				MyDelayTime=MyTimeFromFrame(Frame);
				MyStartTime=GetMyCurrentTime();
			}
			if (LOWORD(wParam) == PLUS5) {
				Frame+=5;
				MyDelayTime=MyTimeFromFrame(Frame);
				MyStartTime=GetMyCurrentTime();
			}
			if (LOWORD(wParam) == PLUS1) {
				Frame+=25;
				MyDelayTime=MyTimeFromFrame(Frame);
				MyStartTime=GetMyCurrentTime();
			}
			if (LOWORD(wParam) == PLUS30) {
				Frame+=750;
				MyDelayTime=MyTimeFromFrame(Frame);
				MyStartTime=GetMyCurrentTime();
			}
			
			if (LOWORD(wParam) == START_PAUSE) {
				IsPlaying=!IsPlaying;
				if(IsPlaying){
					MyStartTime=GetMyCurrentTime();

				}
				else
				{
					MyDelayTime=MyMovieTime;
				}
			}
			if ((LOWORD(wParam) == SUB_FILE )) {
					memset(&ofn,0,sizeof(ofn));
					ofn.lStructSize=sizeof(OPENFILENAME);
					ofn.hwndOwner=NULL;
					ofn.hInstance=NULL;

					ofn.lpstrFilter=TEXT("Subtitle files *.srt\0*.srt\0All FIles *.*\0*.*\0\0");

					ofn.lpstrCustomFilter=NULL;
					ofn.nMaxCustFilter=0;
					ofn.nFilterIndex=1;
					ofn.lpstrFile=SubtitleFileName;
					ofn.nMaxFile=500;
					ofn.lpstrFileTitle=SubtitleFileTitle;
					ofn.nMaxFileTitle=99;
					ofn.lpstrInitialDir=NULL;
					ofn.lpstrTitle="Open BMP file";
					ofn.Flags=OFN_FILEMUSTEXIST;
					ofn.lpstrDefExt="BMP";
					ofn.lCustData=NULL;
					ofn.lpfnHook=NULL;
					ofn.lpTemplateName=NULL;

					SubtitleFileName[0]='\0';

					GetOpenFileName(&ofn);
					SetDlgItemText(hDlg,IDC_FILE,SubtitleFileTitle);
				//	m_SubtitleFileName=SubtitleFileTitle;

					GetTitle(&StartFrame,&EndFrame,SubTitle1,SubTitle2,0,ofn);
					SetDlgItemText(hDlg,IDC_SUBTITLE1,SubTitle1);
					SetDlgItemText(hDlg,IDC_SUBTITLE2,SubTitle2);
					itoa(StartFrame,pom1,10);
					itoa(EndFrame,pom2,10);
					SetDlgItemText(hDlg,IDC_START_FRAME,pom1);
					SetDlgItemText(hDlg,IDC_EXPIRE_FRAME,pom2);
					Frame=0;
					MyDelayTime=ResetMyTime();
					MyStartTime=GetMyCurrentTime();
					IsPlaying=FALSE;
			}
			break;
			case WM_TIMER: {
//
				MyCurrentTime=GetMyCurrentTime();
				
				x=(int)(150+((double)(Frame)/270000*210));
				MoveWindow(GetDlgItem(hDlg,SLIDER),x-4,112,8,30,TRUE);
						
				if (IsPlaying){
					MyMovieTime=MyTimeSub(MyCurrentTime,MyStartTime);
					MyMovieTime=MyTimeAdd(MyMovieTime,MyDelayTime);
					Frame=FrameFromTime(MyMovieTime);	
					if(Frame>StartFrame&&Frame<EndFrame){
						if(!TitleOnScreen){
							TitleOnScreen=TRUE;
							//Ovdje ubaci ispis na ekran
							SetDlgItemText(hDlg,IDC_OSD1,SubTitle1);
							SetDlgItemText(hDlg,IDC_OSD2,SubTitle2);

							OSD_CreateWindow(1,40,480,680,555,TRUE);
							OSD_Set_Font( 2, 2, 0);
							sl1=strlen(SubTitle1);
							sl2=strlen(SubTitle2);
							for(j=0;j<=sl1;j++) SubTitle1[j]=MyCharCopy(SubTile1[j]);
							for(j=0;j<=sl2;j++) SubTitle2[j]=MyCharCopy(SubTile2[j]);
							OSD_Write_Text( 400-sl1*8,5,SubTitle1);
							OSD_Write_Text( 400-sl2*8,40,SubTitle2);
						}
					}else{
							//Brisi OSD
							TitleOnScreen=FALSE;
							SetDlgItemText(hDlg,IDC_OSD1,'\0');
							SetDlgItemText(hDlg,IDC_OSD2,'\0');
							OSD_Close();
					}
				}


				ShowTime=MyTimeFromFrame(Frame);

				sprintf(pom1,"%d:%d:%d.%d",ShowTime.sat,ShowTime.min,ShowTime.sec,ShowTime.mil);
				SetDlgItemText(hDlg,IDC_MOVIE_TIME,pom1);
				itoa(Frame,pom1,10);
				SetDlgItemText(hDlg,IDC_CURRENT_FRAME,pom1);
				
				if(((Frame<StartFrame)||(Frame>EndFrame))) GetTitle(&StartFrame,&EndFrame,SubTitle1,SubTitle2,Frame,ofn);
				
				
				SetDlgItemText(hDlg,IDC_SUBTITLE1,SubTitle1);
				SetDlgItemText(hDlg,IDC_SUBTITLE2,SubTitle2);
				itoa(StartFrame,pom1,10);
				itoa(EndFrame,pom2,10);
				SetDlgItemText(hDlg,IDC_START_FRAME,pom1);
				SetDlgItemText(hDlg,IDC_EXPIRE_FRAME,pom2);




//
				sprintf(pom1,"%d:%d:%d.%d",MyCurrentTime.sat,MyCurrentTime.min,MyCurrentTime.sec,MyCurrentTime.mil);
				SetDlgItemText(hDlg,IDC_TIME,pom1);
				break;
			}
    }

return(FALSE);
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
	MyPom.sat=Pom.wHour;
	MyPom.min=Pom.wMinute;
	MyPom.sec=Pom.wSecond;
	MyPom.mil=Pom.wMilliseconds;
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

BOOL GetTitle(int *LStartFrame, int *LEndFrame, char *LSub1, char *LSub2, int LFrame, OPENFILENAME Lofn){

	FILE *f;
	char Line[200];
	char PSub1[200]="Init 1";
	char PSub2[200]="Init 2";
	int i;
	int j;


	f=fopen(SubtitleFileName,"r");
	if(f=NULL) return(FALSE);	
	while(fgets(Line,200,f)!=NULL){
		// citanje pocetnog framea
		i=0;j=0;
		while(Line[i+1]!='}'){
			i++;
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
				PSub1[j]=Line[i];
				j++;
			}
			

			if(Line[i+1]=='|'){
				i++;
			}else{
				j--;
			}
			PSub1[j]='\0';			// PSub1 - prva linija titla
			j=0;
			while(Line[i+1]!='\0'){
				i++;
				PSub2[j]=Line[i];
				j++;
			}
			PSub2[j-1]='\0';			// PSub2 - druga linija titla
			fclose(f);
			strcpy(LSub1,PSub1);
			strcpy(LSub2,PSub2);
			return(TRUE);			// Mozemo van, uspjeli smo !
		}
	}
	fclose(f);		
	return(FALSE);					// Dosli smo do kraja i nismo pronasli frame
}

char MyCharCopy(char ch)			// Kvakastim slovima skida kvake
{
	if (ch=='š') return '¡';
	if (ch=='Š') return '›';
	if (ch=='ð') return '‹';
	if (ch=='Ð') return 'L';
	if (ch=='ž') return '-';
	if (ch=='Ž') return '^';
	if (ch=='è') return '¡';
	if (ch=='È') return '{';
	if (ch=='æ') return '|';
	if (ch=='Æ') return '}';
	return ch;
}