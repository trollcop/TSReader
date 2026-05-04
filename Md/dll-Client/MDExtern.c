/****************************************************************************

    PROGRAM: MDExtern.c
 
    FUNCTIONS: Beispiel f¸r eine Externe DLL in	MultiDec

 
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
// Damir's deklarations

BOOL IsEmpty(char*);
BOOL OnOSD=FALSE;
BOOL IsStarted=FALSE;
void ShowTitlesFromFile(void);
char MyCharCopy(char);
//char SubTitle1[100];
//char SubTitle2[100];
char SubLine1[100]="*";
char SubLine2[100]="*";
char OldLine1[100]="*";
char OldLine2[100]="*";

// End of Damir's deklarations
// *****************************************


BOOL APIENTRY About(HWND hDlg,UINT message,UINT wParam,LONG lParam);
void Rescan_Programm( struct TProgramm *TPtg );
void Filter_0_Irq_Proc( int MyFilter, int DatenLaenge , unsigned char *Daten);
void Start_Log( void );
void Stop_Log( void );
void Start_OSD_Subtitles( void );
void Start_OSD( void );
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

    PURPOSE:  Wird aufgerufen nachdem die DVB¥s-Karte initialisiert worden ist

*******************************************************************************/
void On_Start(HINSTANCE MDInstance, HWND MDWnd, BOOL Log_Set, int DLL_ID, char *My_Hot_Key, char *Api_Version, int *Keep_me_running)
{
	
	
	MultiDecInstance=MDInstance;
	MultiDecWindow=MDWnd;
    MultiDecLog = Log_Set;
    MultiDec_DLL_ID=DLL_ID;

	*My_Hot_Key='J';   // oder 0x00 f¸r Kein HotKey
	*Keep_me_running = 0;

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
// An dieser Funktion idetifiziert MultiDec seine Externen DLL¥s.
// Eine Dll wird angezogen, wenn diese Funktion drinnen ist.
// Der Name ist der Men¸name der DLL, der in MD gesetzt werden soll.
	
	strcpy(Name,"Subtitler");
};


void On_Menu_Select(unsigned int MenuID )
{
// Entspricht in etwa der Command_Loop Mosaik95cmd// Entspricht in etwa der Command_Loop Mosaik95cmd
// Entspricht in etwa der Command_Loop Mosaik95cmd

	

// Im Men¸ d¸rfen nur ID¥s von 40000 bis 41000 verwendet werden.
// Bei mehreren DLL¥s MUSS dieser Wert eindeutig sein
// Sollte aber bei 1000 mˆglichen Men¸s kein Problem sein
 
	
	if ( MenuID == 40001 ) {
		if(IsStarted){
			OSD_CreateWindow(1,40,480,680,555,TRUE);
			OSD_Set_Font( 2, 2, 0);
			OSD_Write_Text( 20,5,"...subtitler is closed");
			Sleep(1000);
			OSD_Close( );
			IsStarted=FALSE;
		}else{
			IsStarted=TRUE;
			Create_Osd_Thread((LPTHREAD_START_ROUTINE)Start_OSD);
		}
	}
	if ( MenuID == 40002 ) {
		DialogBox(DLLInstance,ABOUTBOX,MultiDecWindow,About);
	}
}


void On_Channel_Change( struct TProgramm CurrentProgramm )
{
	IsStarted = FALSE;
}


void On_Hot_Key( void )
{
	if(IsStarted){
		OSD_CreateWindow(1,40,480,680,555,TRUE);
		OSD_Set_Font( 2, 2, 0);
		OSD_Write_Text( 20,5,"...subtitler is closed");
		Sleep(1000);
		OSD_Close( );
		IsStarted=FALSE;
	}else{
		IsStarted=TRUE;
		Create_Osd_Thread((LPTHREAD_START_ROUTINE)Start_OSD);
	}
}


void On_Rec_Play(int Mode)
{
};

void On_Osd_Key( unsigned char Key )
{
    OSD_Key_Value=Key;
	if (OSD_KEY_EVENT != NULL ) SetEvent(OSD_KEY_EVENT);
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



void Start_OSD( void )
{

	OSD_CreateWindow(1,40,480,680,555,TRUE);
	OSD_Set_Font( 2, 2, 0);
	OSD_Write_Text( 20,5,"DVBs Subtitler is started...");
	Sleep(1000);
	OSD_Close( );
	for (;IsStarted;) {
		WaitForSingleObject (OSD_KEY_EVENT, 80 );
		ResetEvent(OSD_KEY_EVENT);
		
		ShowTitlesFromFile();
	}
}

/**************** Usefull Helpers  ****************/


void OSD_CreateWindow(unsigned char Bits, unsigned short x1,unsigned short y1,unsigned short x2,unsigned short y2,BOOL Has_KeyBoard_Input)
{
	struct TOSD_START Osd_Start;


	// Bits 0 = 2 Farbige Fenstern ( kˆnnen grˆﬂ sein )
	// Bits 1 = 4 Farbige Fenster ( sollten klein sein ( Speicher auf DVB¥s Karte zu klein )


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


//national characters - supported: Croatian, Czech, Slovak
char MyCharCopy(char ch)
{
	if (ch=='¡') return   0;
	if (ch=='·') return   1;
	if (ch=='»') return   2;
	if (ch=='Ë') return   3;
	if (ch=='œ') return   4;
	if (ch=='ç') return   5;
	if (ch=='–') return   6;
	if (ch=='') return   7;
	if (ch=='…') return   8;
	if (ch=='Ã') return   9;
	if (ch=='È') return  10;
	if (ch=='Ï') return  11;
	if (ch=='Õ') return  12;
	if (ch=='Ì') return  13;
	if (ch=='“') return  14;
	if (ch=='Ú') return  15;
	if (ch=='”') return  16;
	if (ch=='‘') return  17;
	if (ch=='Û') return  18;
	if (ch=='Ù') return  19;
	if (ch=='ÿ') return  20;
	if (ch=='¯') return  21;
	if (ch=='ä') return  22;
	if (ch=='ö') return  23;
	if (ch=='⁄') return  24;
	if (ch=='∆') return  60;
	if (ch=='Ê') return  62;
	if (ch=='˙') return  92;
	if (ch=='Ÿ') return  94;
	if (ch=='˘') return  95;
	if (ch=='›') return  96;
	if (ch=='˝') return 123;
	if (ch=='é') return 124;
	if (ch=='û') return 125;
	if (ch=='≈') return 127;

	//substitute some changed rare (in subs) used chars
	if (ch== 24) return  45;  // -  to  -
	if (ch== 60) return  40;  // <  to  (
	if (ch== 62) return  41;  // >  to  )
	if (ch== 92) return  47;  // \  to  /
	if (ch== 94) return  32;  // ^  to  SPACE
	if (ch== 95) return  32;  // _  to  SPACE
	if (ch== 96) return  39;  // `  to  '
	if (ch==123) return  40;  // {  to  (
	if (ch==124) return  32;  // |  to  SPACE
	if (ch==125) return  41;  // }  to  )

	//extra substitute
	if (ch=='Â') return  12;     // Â  to  Õ (are equal in MD fonts)
	if (ch=='Ô') return  'd';  // lack of free space --> + '
	if (ch=='ù') return  't';  // lack of free space --> + '
	if (ch=='º') return  'L';  // lack of free space --> + '
	if (ch=='æ') return  'l';  // lack of free space --> + '

	//strip other ANSI diacritic chars to its basic form
	if ((ch=='•') || (ch=='¬') || (ch=='√')) return 'A';
	if ((ch=='π') || (ch=='‚') || (ch=='„')) return 'a';
	if (ch=='«') return 'C';
	if (ch=='Á') return 'c';
	if ((ch==' ') || (ch=='À')) return 'E';
	if ((ch=='Í') || (ch=='Î')) return 'e';
	if (ch=='Œ') return 'I';
	if (ch=='Ó') return 'i';
	if (ch=='£') return 'L';
	if (ch=='≥') return 'l';
	if (ch=='—') return 'N';
	if (ch=='Ò') return 'n';
	if (ch=='’') return 28;
	if (ch=='ı') return 29;
	if (ch=='¿') return 'R';
	if (ch=='‡') return 'r';
	if ((ch=='å') || (ch=='™')) return 'S';
	if ((ch=='ú') || (ch=='∫')) return 's';
	if (ch=='ﬁ') return 'T';
	if (ch=='˛') return 't';
	if (ch=='€') return 30;
	if (ch=='˚') return 31;
	if ((ch=='è') || (ch=='Ø')) return 'Z';
	if ((ch=='ü') || (ch=='ø')) return 'z';

	return ch;
}

	

void ShowTitlesFromFile(void){
	FILE *fp;
	char Fname[100];
	char Blank[5]=" ";
	int sl1,sl2,j,cnt;
	int x1,x2;
	char osl1[100],osl2[100],TempLine[100];
	
		
	strcpy(osl1," ");
	strcpy(osl2," ");
	strcpy(Fname,"C:\\SubData.txt");
	if((fp=fopen(Fname,"r"))==NULL){
		strcpy(SubLine1,"Check the path(s) and make sure that ");
		strcpy(SubLine2,"DVBs Subtitler application is running! ");
	}else{
		fgets(SubLine1,200,fp);
		fgets(SubLine2,200,fp);
		fclose(fp);
	}

	sl1=strlen(SubLine1); 
	strcpy(TempLine, SubLine1);
	cnt=0;
	for(j=0;j<sl1;j++)
	{
	  SubLine1[j+cnt]=MyCharCopy(TempLine[j]);;
	  if ( (TempLine[j]=='Ô') || (TempLine[j]=='ù') || (TempLine[j]=='º') || (TempLine[j]=='æ') )
	  {
	    sl1++; cnt++; SubLine1[j+cnt]=39;
	  }
	}

	sl2=strlen(SubLine2);
	strcpy(TempLine, SubLine2);
	cnt=0;
	for(j=0;j<sl2;j++)
	{
	  SubLine2[j+cnt]=MyCharCopy(TempLine[j]);
	  if ( (TempLine[j]=='Ô') || (TempLine[j]=='ù') || (TempLine[j]=='º') || (TempLine[j]=='æ') )
	  {
	    sl2++; cnt++; SubLine2[j+cnt]=39;
	  }
	}

	SubLine1[sl1-1]=NULL;
	SubLine2[sl2-1]=NULL;
	
	if(IsEmpty(SubLine1)&&IsEmpty(SubLine1)){
		if(OnOSD){
			OSD_Close();
			OnOSD=FALSE;
		}
	}else{
		if((strcmp(SubLine1,OldLine1)!=0)||(strcmp(SubLine2,OldLine2)!=0)){
			OSD_CreateWindow(1,40,475,680,550,TRUE);
			OSD_Set_Font( 2, 2, 0);
			x1=340-sl1*7;
			x2=340-sl2*7;
			if(x1<1) x1=1;
			if(x2<1) x2=1;
			strcat(osl1,SubLine1);
			strcat(osl2,SubLine2);
			OSD_Write_Text( x1,5,osl1);
			OSD_Write_Text( x2,40,osl2);
			strcpy(OldLine1,SubLine1);
			strcpy(OldLine2,SubLine2);
			OnOSD=TRUE;
		}
	}
	return;

}
BOOL IsEmpty(char Str[100]){
	int i,j;
	j=strlen(Str);
	if(j==0) return TRUE;
	for(i=0;i<j;i++) if(Str[i]!=32) return FALSE;
	return TRUE;
}

BOOL APIENTRY About(HWND hDlg,UINT message,UINT wParam,LONG lParam)
{
	switch (message) {
		case WM_COMMAND:
			if (LOWORD(wParam) == IDOK) {
				EndDialog(hDlg,TRUE);
			}
			break;
	}
}
