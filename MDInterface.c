#include <windows.h>
#include <commctrl.h>
#include <time.h>
#include "TSReader.h"
#include "SoftCSA.h"
#include "resource.h"

#include "Md\MULTIDEC\Globals.h"
#include "MDInterface.h"

extern PVARIABLES v;

void IPDVBModeOn(HWND hDlg);
void IPDVBModeOff(HWND hDlg);

Extern_IPData ipdata[5];
Extern_Descriptor_Decode DescriptorDecode[5];
int External_Dll_Count = 0;
struct TProgramm Programm[MAXPROGS + 1];
struct TProgramm ProgrammNeu[NEUSIZE];

char MD_API_Version[32] = { 0, };

int VideoPID = 0;
int AudioPID = 0;
int MultiPID = 0;
int AddPIDFilter = 0;
int DelPIDFilter = 0;
struct TOSD_START DLL_OSD_Call;
struct External_Stream_Dll Ext_Dll[5];

static CRITICAL_SECTION csDecrypt;
BYTE *pKeys;
static BYTE bZeroKeys[16] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
static BYTE bCurrentKeys[16];
static csakey csaCurrent;

#define MAX_START_FILTERS 128

 struct TProgrammNummer {
	 int RealNummer;
	 int VirtNummer;
 };
 
 struct TTPCatio {
	 int TPCatAnzahl;
	 struct TTPCat TPCat[32];
 };
 
 struct TOSD_DRAW {
	 unsigned short x;
	 unsigned short y;
	 unsigned short Sizex;
	 unsigned short Sizey;
	 unsigned short Color;
 };
 
 struct TOSD_SETFONT {
	 unsigned short Typ;
	 unsigned short Fg_Color;
	 unsigned short Bg_Color;
 };
 
 struct TOSD_SETEXT {
	 unsigned short x;
	 unsigned short y;
	 char szTemp[128];
 };
 
 struct TSTART_FILTER {
	 unsigned short DLL_ID;
	 unsigned short Filter_ID;
	 unsigned short Pid;
	 unsigned char Name[32];
	 unsigned int Irq_Call_Adresse;
	 int Running_ID;
 };
 
 struct TDVB_COMMAND{
	 unsigned short Cmd_laenge;
	 unsigned short Cmd_Buffer[32];
 };

typedef struct tagFilterTSPackets
{
	BYTE * pBuffer;
	int nFillPtr;

} FILTERTSPACKETS, *PFILTERTSPACKETS;
 
struct TProgrammNummer TPNr;
struct TTransponder TPtp;
struct TProgramm TPtg;
struct TTPCatio TPc;
struct TOSD_DRAW osd_draw;
struct TOSD_SETFONT osd_font;
struct TOSD_SETEXT osd_text;
struct TSTART_FILTER start_filter[MAX_START_FILTERS];
struct TDVB_COMMAND dvb_com;	 	 

FILTERTSPACKETS filtertspackets[MAX_START_FILTERS];

void Write_Log(char * szMessage)
{
	char szNewMessage[1024];
	lstrcpy(szNewMessage, "TSReader: ");
	lstrcat(szNewMessage, szMessage);
	lstrcat(szNewMessage, "\n");
	OutputDebugString(szNewMessage);
}

void MD__Shutdown(void)
{
	// Do any left over cleanup
	int i;

	for (i = 0; i < MAX_START_FILTERS; i++)
	{
		if (filtertspackets[i].pBuffer != NULL)
		{
			LocalFree(filtertspackets[i].pBuffer);
			filtertspackets[i].pBuffer = NULL;
		}
	}

	DeleteCriticalSection(&csDecrypt);
}

void MD__Unload_External_Dll(int Nummer)
{
	char szTemp[512];
	
	if ( Ext_Dll[Nummer].Externe_DLL != NULL )
	{
		if ( Ext_Dll[Nummer].Extern_Filter_Close != NULL )
			Ext_Dll[Nummer].Extern_Filter_Close(Nummer);

		if ( Ext_Dll[Nummer].Extern_Exit != NULL )
		{
			(Ext_Dll[Nummer].Extern_Exit)();
			Sleep(100);

			DestroyMenu(Ext_Dll[Nummer].Extern_Menu);
			FreeLibrary(Ext_Dll[Nummer].Externe_DLL);
			Ext_Dll[Nummer].Externe_DLL=NULL;
			
			wsprintf(szTemp,"MDAPI DLL %s unloaded",Ext_Dll[Nummer].Name);
			Write_Log(szTemp);
			
			Ext_Dll[Nummer].Extern_Init=NULL;
			Ext_Dll[Nummer].Extern_Exit=NULL;
			Ext_Dll[Nummer].Extern_Channel_Change=NULL;
			Ext_Dll[Nummer].Extern_Hot_Key=NULL;
			Ext_Dll[Nummer].Extern_Menu_Cmd=NULL;
			Ext_Dll[Nummer].Extern_Menu=NULL;
			Ext_Dll[Nummer].Extern_Send_Dll_ID_Name=NULL;
			Ext_Dll[Nummer].Extern_Stream_Function[0]=NULL;
			Ext_Dll[Nummer].Extern_Stream_Function[1]=NULL;
			Ext_Dll[Nummer].Extern_Stream_Function[2]=NULL;
			Ext_Dll[Nummer].Extern_Stream_Function[3]=NULL;
			Ext_Dll[Nummer].Extern_Stream_Function[4]=NULL;
			Ext_Dll[Nummer].Extern_Stream_Function[5]=NULL;
			Ext_Dll[Nummer].Extern_Stream_Function[6]=NULL;
			Ext_Dll[Nummer].Extern_Stream_Function[7]=NULL;
			Ext_Dll[Nummer].Extern_RecPlay=NULL;
			Ext_Dll[Nummer].Extern_Filter_Close=NULL;
			Ext_Dll[Nummer].Name[0]=0x00;
			Ext_Dll[Nummer].HotKey=0x00;
			ipdata[Nummer] = NULL;
			Ext_Dll[Nummer].Extern_Src_Restart_Function = NULL;
		}
	}
}

void MD__Send_External_DLL_Menu_Cmd(unsigned int MessageId)
{
	int i;
	
	for ( i=0; i < 5; i++ )
	{
		if ( Ext_Dll[i].Extern_Menu_Cmd != NULL )
			(Ext_Dll[i].Extern_Menu_Cmd)(MessageId);
	}
}     

int MD__Load_External_Dll(HINSTANCE hInstance)
{
	int i;
    HANDLE hFind;
	WIN32_FIND_DATA fd;
	char szTemp[512];
    char CurrentDir[MAX_PATH];

	for (i = 0; i < 5; i++)
		ipdata[i] = NULL;

	GetModuleFileName(hInstance, CurrentDir, sizeof(CurrentDir));
	for (i = lstrlen(CurrentDir); i > 0; i--)
	{
		if (CurrentDir[i] == '\\')
		{
			CurrentDir[i] = 0;
			break;
		}
	}

	wsprintf(szTemp, "%s\\MDPlugins\\*.dll",CurrentDir);
    hFind = FindFirstFile(szTemp, &fd);
    if (hFind == INVALID_HANDLE_VALUE)
		return 0;

	External_Dll_Count = 0;
	while ( External_Dll_Count < 5 )
	{
		wsprintf((LPSTR)Ext_Dll[External_Dll_Count].Name, "%s\\MDPlugins\\%s", CurrentDir, fd.cFileName);
		
		wsprintf(szTemp,"MDAPI Scanne DLL %s",Ext_Dll[External_Dll_Count].Name);
		Write_Log(szTemp);

		Ext_Dll[External_Dll_Count].Externe_DLL = LoadLibrary((LPCSTR)Ext_Dll[External_Dll_Count].Name);
		if ( Ext_Dll[External_Dll_Count].Externe_DLL != NULL )
		{
			Ext_Dll[External_Dll_Count].Extern_Send_Dll_ID_Name = (Extern_Send_Dll_ID_Name_DLL)GetProcAddress(Ext_Dll[External_Dll_Count].Externe_DLL,"On_Send_Dll_ID_Name");
			//if ( Ext_Dll[External_Dll_Count].Extern_Send_Dll_ID_Name != 0 )
			{
				Ext_Dll[External_Dll_Count].Extern_Menu=LoadMenu(Ext_Dll[External_Dll_Count].Externe_DLL,"EXTERN");
				Ext_Dll[External_Dll_Count].Extern_Init=(Extern_INIT_DLL) GetProcAddress(Ext_Dll[External_Dll_Count].Externe_DLL,"On_Start");
				Ext_Dll[External_Dll_Count].Extern_Channel_Change=(Extern_Channel_Change_DLL) GetProcAddress(Ext_Dll[External_Dll_Count].Externe_DLL,"On_Channel_Change");
				Ext_Dll[External_Dll_Count].Extern_Exit=(Extern_EXIT_DLL)GetProcAddress(Ext_Dll[External_Dll_Count].Externe_DLL,"On_Exit");
				Ext_Dll[External_Dll_Count].Extern_Hot_Key=(Extern_Hot_Key_DLL)GetProcAddress(Ext_Dll[External_Dll_Count].Externe_DLL,"On_Hot_Key");
				Ext_Dll[External_Dll_Count].Extern_OSD_Key=(Extern_OSD_Key_DLL)GetProcAddress(Ext_Dll[External_Dll_Count].Externe_DLL,"On_Osd_Key");
				Ext_Dll[External_Dll_Count].Extern_Menu_Cmd=(Extern_Menu_Cmd_DLL)GetProcAddress(Ext_Dll[External_Dll_Count].Externe_DLL,"On_Menu_Select");
				Ext_Dll[External_Dll_Count].Extern_Send_Dll_ID_Name=(Extern_Send_Dll_ID_Name_DLL)GetProcAddress(Ext_Dll[External_Dll_Count].Externe_DLL,"On_Send_Dll_ID_Name");
				Ext_Dll[External_Dll_Count].Extern_Filter_Close=(Extern_Filter_Close_DLL)GetProcAddress(Ext_Dll[External_Dll_Count].Externe_DLL,"On_Filter_Close");
				Ext_Dll[External_Dll_Count].Extern_RecPlay=(Extern_Filter_Close_DLL)GetProcAddress(Ext_Dll[External_Dll_Count].Externe_DLL,"On_Rec_Play");
				Ext_Dll[External_Dll_Count].Extern_Src_Restart_Function =(Extern_Src_Restart)GetProcAddress(Ext_Dll[External_Dll_Count].Externe_DLL,"Src_Restart");
				ipdata[External_Dll_Count] = (Extern_IPData)GetProcAddress(Ext_Dll[External_Dll_Count].Externe_DLL,"On_IP_Data_Receive");
				DescriptorDecode[External_Dll_Count] = (Extern_Descriptor_Decode)GetProcAddress(Ext_Dll[External_Dll_Count].Externe_DLL,"On_Descriptor_Decode");

				wsprintf(szTemp,"MDAPI DLL %s loaded",Ext_Dll[External_Dll_Count].Name);
				Write_Log(szTemp);
				wsprintf(szTemp,"  Function On_Send_Dll_ID_Name       = 0x%08x",Ext_Dll[External_Dll_Count].Extern_Send_Dll_ID_Name);
				Write_Log(szTemp);
				wsprintf(szTemp,"  Menu                               = 0x%08x",Ext_Dll[External_Dll_Count].Extern_Menu);
				Write_Log(szTemp);
				wsprintf(szTemp,"  Function On_Start                  = 0x%08x",Ext_Dll[External_Dll_Count].Extern_Init);
				Write_Log(szTemp);
				wsprintf(szTemp,"  Function On_Exit                   = 0x%08x",Ext_Dll[External_Dll_Count].Extern_Exit);
				Write_Log(szTemp);
				wsprintf(szTemp,"  Function On_Hot_Key                = 0x%08x",Ext_Dll[External_Dll_Count].Extern_Hot_Key);
				Write_Log(szTemp);
				wsprintf(szTemp,"  Function On_Osd_Key                = 0x%08x",Ext_Dll[External_Dll_Count].Extern_OSD_Key);
				Write_Log(szTemp);
				wsprintf(szTemp,"  Function On_Channel_Change         = 0x%08x",Ext_Dll[External_Dll_Count].Extern_Channel_Change);
				Write_Log(szTemp);
				wsprintf(szTemp,"  Function On_Menu_Cmd               = 0x%08x",Ext_Dll[External_Dll_Count].Extern_Menu_Cmd);
				Write_Log(szTemp);
				wsprintf(szTemp,"  Function On_Filter_Close           = 0x%08x",Ext_Dll[External_Dll_Count].Extern_Filter_Close);
				Write_Log(szTemp);
				wsprintf(szTemp,"  Function On_Rec_Play               = 0x%08x",Ext_Dll[External_Dll_Count].Extern_RecPlay);
				Write_Log(szTemp);
				if (Ext_Dll[External_Dll_Count].Extern_Init)
					External_Dll_Count++;
				else
					FreeLibrary(Ext_Dll[External_Dll_Count].Externe_DLL);
			}
			/*else
			{
				wsprintf(szTemp,"DLL %s has no On_Send_Dll_ID_Name function",Ext_Dll[External_Dll_Count].Name);
				Write_Log(szTemp);
				FreeLibrary(Ext_Dll[External_Dll_Count].Externe_DLL);
				Ext_Dll[External_Dll_Count].Externe_DLL=NULL;
			}*/
		}
		if (FindNextFile(hFind, &fd) == FALSE)
		{
			FindClose(hFind);
			return External_Dll_Count;
		}
	}

	return External_Dll_Count;
}

void MD__StartPluginsRunning(HINSTANCE hInst, HWND hWnd)
{
	int i;
	HMENU hTopMenu = GetMenu(hWnd);
	HMENU hPlugInsMenu;

	int nMenuIndex = 6; // for TSReader Pro

	hPlugInsMenu = GetSubMenu(hTopMenu, nMenuIndex);

	for (i = 0; i < MAX_START_FILTERS; i++)
	{
		memset(&start_filter[i], 0, sizeof(start_filter[i]));
		memset(&filtertspackets[i], 0, sizeof(filtertspackets[i]));
	}

	memset(MD_API_Version, 0x00, 32);
	lstrcpy(MD_API_Version,"MD-API Version 01.02 - 1.06 TSR");

	for ( i=0; i < 5; i++ )
	{
		if ( Ext_Dll[i].Extern_Init != NULL )
		{
			int KeepRunning = 1;
			BOOL WRITE_LOG=TRUE;

			(Ext_Dll[i].Extern_Init)(hInst, hWnd, WRITE_LOG, i, &Ext_Dll[i].HotKey, (unsigned char *)&MD_API_Version[0], &KeepRunning);
			KeepRunning = 0;
			if ( KeepRunning == 1 )
			{
				char szTemp[512];

				// Externe DLL Sagt : Ich kann / Will nicht laufen 
				wsprintf(szTemp,"MDAPI DLL %s wants to be unloaded again",Ext_Dll[i].Name);
				Write_Log(szTemp);
				MD__Unload_External_Dll(i);
			}
			else
			{
				if ( Ext_Dll[i].Extern_Menu  != NULL )
				{			
					char szName[128];

					memset(szName, 0, sizeof(szName));
					if ( Ext_Dll[i].Extern_Send_Dll_ID_Name != NULL )
						(Ext_Dll[i].Extern_Send_Dll_ID_Name)(szName);
					else
						wsprintf(szName, "DLL %d Menu", i);
					AppendMenu(hPlugInsMenu, MF_BYPOSITION | MF_POPUP | MF_ENABLED, (unsigned int)Ext_Dll[i].Extern_Menu, (LPCTSTR)szName);
				}
			}
		}
	}

	DeleteMenu(hPlugInsMenu, ID_PLUGINS_SOMETHING, MF_BYCOMMAND);	// remove the "something" placeholder
	InitializeCriticalSection(&csDecrypt);
}

BOOL MD__StartFilter(LPARAM lParam)
{
	 if ( lParam == 0 )
		 return(FALSE);
	 {
		// Find a free entry
		 int i;
		 char szTemp[128];

		 for (i = 0; i < MAX_START_FILTERS; i++)
		 {
			 if (start_filter[i].Irq_Call_Adresse == 0)
				 break;
		 }
		 if (i == MAX_START_FILTERS)
		 {
			 Write_Log("MD__StartFilters(): Out of filters");
			 return FALSE;
		 }		
		 memcpy(&start_filter[i], (struct TSTART_FILTER *)lParam, sizeof(struct TSTART_FILTER));
		 wsprintf(szTemp,"MDAPI_START_FILTER PID = 0x%04x Slot = %d", start_filter[i].Pid, i);
		 Write_Log(szTemp);

		 Ext_Dll[start_filter[i].DLL_ID].Extern_Stream_Function[start_filter[i].Filter_ID] = (Extern_Stream_DLL)start_filter[i].Irq_Call_Adresse;
		 start_filter[i].Running_ID = i + 1;

		 filtertspackets[i].pBuffer = LocalAlloc(LPTR, 2048);
		 filtertspackets[i].nFillPtr = 0;
		 memcpy((struct TSTART_FILTER *)lParam, &start_filter[i], sizeof(struct TSTART_FILTER));
		 return(TRUE);
	 }
}

BOOL MD__StopFilter(LPARAM lParam)
{
	char szTemp[128];
	
	if (lParam == 0)
		return FALSE;

	wsprintf(szTemp,"MDAPI_STOP_FILTER PID = 0x%04x slot = %d ", start_filter[lParam - 1].Pid & 0x1fff, lParam - 1);
	Write_Log(szTemp);
	if (filtertspackets[lParam - 1].pBuffer != NULL)
	{
		LocalFree(filtertspackets[lParam - 1].pBuffer);
		filtertspackets[lParam - 1].pBuffer = NULL;
	}
	memset(&start_filter[lParam - 1], 0, sizeof(struct TSTART_FILTER));
	
	return(TRUE);
}

BOOL MD__ExternCommandDispatch(HWND hWnd, UINT message, UINT wParam, LONG lParam)
{
	 unsigned int Irq_Proc_Id = 0;
	 char szTemp[128];
	 	 	 
	 switch(wParam)
	 {
	 default:
		 wsprintf(szTemp, "MDAPI unknown - wParam 0x%08x lParam 0x%08x", wParam, lParam);
		 Write_Log(szTemp);
		 break;
	 case MDAPI_GET_PROGRAMM_NUMMER:
		 wsprintf(szTemp,"MDAPI_GET_PROGRAMM_NUMMER ");
		 Write_Log(szTemp);
		 TPNr.VirtNummer=Programm[0].Link_SID;
		 TPNr.RealNummer=Programm[0].Link_SID;
		 if ( lParam != 0 )
			 memcpy((struct TProgrammNummer *)lParam, &TPNr.RealNummer, 8);
		 return(TRUE);
	 case MDAPI_GET_VERSION:
		 wsprintf(szTemp,"MDAPI_GET_VERSION");
		 Write_Log(szTemp);
		 if ( lParam != 0 )
			 memcpy((unsigned char *)lParam,&MD_API_Version[0],32);
		 return(TRUE);
	 case MDAPI_SET_PROGRAMM_NUMMER:
		 wsprintf(szTemp,"MDAPI_SET_PROGRAMM_NUMMER %d",lParam);
		 Write_Log(szTemp);
		 //wsprintf(ChannelString,"%d",lParam);
		 //SetTimer(hWnd,99,1000,NULL);
		 return(TRUE);
	 case MDAPI_GET_TRANSPONDER:
		 wsprintf(szTemp,"MDAPI_GET_TRANSPONDER");
		 Write_Log(szTemp);
		 memset(&TPtp,0x00,sizeof(struct TTransponder));
		 //TPtp.fec=Programm[CurrentProgramm].fec;
		 //TPtp.diseqc=Programm[CurrentProgramm].diseqc;
		 //TPtp.freq=Programm[CurrentProgramm].freq;
		 //TPtp.qam=Programm[CurrentProgramm].qam;
		 //TPtp.srate=Programm[CurrentProgramm].srate;
		 //TPtp.volt=Programm[CurrentProgramm].volt;
		 //TPtp.sync=dvb_front.sync;
		 //TPtp.ts_id=Programm[CurrentProgramm].tp_id;
		 if ( lParam != 0 )
			 memcpy((struct TTransponder *)lParam,&TPtp,sizeof(struct TTransponder));
		 return(TRUE);
	 case MDAPI_SET_TRANSPONDER:
		 wsprintf(szTemp,"MDAPI_SET_TRANSPONDER");
		 Write_Log(szTemp);
		 if ( lParam != 0 )
			 memcpy(&TPtp,(struct TTransponder *)lParam,sizeof(struct TTransponder));
		 //Set_Transponder(&TPtp);
		 return(TRUE);
	 case MDAPI_SET_PROGRAMM:
		 wsprintf(szTemp,"MDAPI_SET_PROGRAMM");
		 Write_Log(szTemp);
		 if ( lParam != 0 )
			memcpy(&Programm[0],(struct TProgramm *)lParam,sizeof(struct TProgramm));
		 return(TRUE);
	 case MDAPI_GET_PROGRAMM:
		 wsprintf(szTemp,"MDAPI_GET_PROGRAMM");
		 Write_Log(szTemp);
		 if ( lParam != 0 )
			 memcpy((struct TProgramm *)lParam,&Programm[0],sizeof(struct TProgramm));
		 return(TRUE);
	 case MDAPI_RESCAN_PROGRAMM:
		 if ( lParam == 0 ) return(FALSE);
		 memcpy(&TPtg,(struct TProgramm *)lParam,sizeof(struct TProgramm));
		 wsprintf(szTemp,"MDAPI_RESCAN_PROGRAMM");
		 Write_Log(szTemp);
		 //Load_All_Parameters(&TPtg);
		 memcpy((struct TProgramm *)lParam,&TPtg,sizeof(struct TProgramm));
		 return(TRUE);
	 case MDAPI_SAVE_PROGRAMM:
		 wsprintf(szTemp,"MDAPI_SAVE_PROGRAMM");
		 Write_Log(szTemp);
		 //Write_Programm_List();
		 return(TRUE);
	 case MDAPI_SCAN_CURRENT_TP:
		 if ( lParam == 0 )
			 return(FALSE);
		 wsprintf(szTemp,"MDAPI_SCAN_CURRENT_TP");
		 Write_Log(szTemp);
		 //Scan_TP( NULL );
		 memcpy((struct TProgramm *)lParam,&ProgrammNeu[0],sizeof(struct TProgramm)*NEUSIZE);
		 return(TRUE);
	 case MDAPI_SCAN_CURRENT_CAT:
		 if ( lParam == 0 )
			 return(FALSE);
		 wsprintf(szTemp,"MDAPI_SCAN_CURRENT_CAT");
		 Write_Log(szTemp);
		 //Get_CAT();
		 //memcpy(TPc.TPCat,TPCat,sizeof( struct TTPCat)*32);	
		 //TPc.TPCatAnzahl = TPCatAnzahl;
		 memcpy((struct TTPCatio *)lParam,&TPc,sizeof(struct TTPCatio));
		 return(TRUE);
	 case MDAPI_START_OSD:
		 if ( lParam == 0 )
			 return(FALSE);
		 wsprintf(szTemp,"MDAPI_START_OSD");
		 Write_Log(szTemp);
		 memcpy(&DLL_OSD_Call,(struct TOSD_START *)lParam,sizeof(struct TOSD_START));
		 
		 //if ( DLL_OSD_Created == NULL ) 
		//	 DLL_OSD_Created=CreateEvent(NULL,FALSE,FALSE,NULL);
		 //Create_Osd_Thread((LPTHREAD_START_ROUTINE)Run_OSD_DLL_Call);
		 //WaitForSingleObject(DLL_OSD_Created, 4000 );
		 //CloseHandle(DLL_OSD_Created);
		 //DLL_OSD_Created=NULL;
		 
		 return(TRUE);
	 case MDAPI_OSD_DRAWBLOCK:
		 if ( lParam == 0 ) 
			 return(FALSE);
		 wsprintf(szTemp,"MDAPI_OSD_DRAWBLOCK");
		 Write_Log(szTemp);
		 memcpy(&osd_draw,(struct TOSD_DRAW *)lParam,sizeof(struct TOSD_DRAW));
		 //DrawBlock(1, osd_draw.x, osd_draw.y,osd_draw.Sizex, osd_draw.Sizey,osd_draw.Color);
		 return(TRUE);
	 case MDAPI_OSD_SETFONT:
		 if ( lParam == 0 )
			 return(FALSE);
		 wsprintf(szTemp,"MDAPI_OSD_SETFONT");
		 Write_Log(szTemp);
		 memcpy(&osd_font,(struct TOSD_SETFONT *)lParam,sizeof(struct TOSD_SETFONT));
		 //SetFont(1, osd_font.Typ, osd_font.Fg_Color, osd_font.Bg_Color);
		 return(TRUE);
	 case MDAPI_OSD_TEXT:
		 if ( lParam == 0 )
			 return(FALSE);
		 wsprintf(szTemp,"MDAPI_OSD_TEXT: ");
		 memcpy(&osd_text,(struct TOSD_SETEXT *)lParam,sizeof(struct TOSD_SETEXT));
		 lstrcat(szTemp, osd_text.szTemp);
		 Write_Log(szTemp);
		 //WriteText( 1, osd_text.x,osd_text.y,osd_text.szTemp);
		 return(TRUE);
	 case MDAPI_SEND_OSD_KEY:
		 wsprintf(szTemp,"MDAPI_SEND_OSD_KEY");
		 Write_Log(szTemp);
		 //OSD_Key_Value=(unsigned char)lParam;
		 //if ( OSD_KEY_EVENT != NULL )
		//	 SetEvent(OSD_KEY_EVENT);
		 return(TRUE);
	 case MDAPI_STOP_OSD:
		 wsprintf(szTemp,"MDAPI_STOP_OSD");
		 Write_Log(szTemp);
		 //OSD_Key_Value=0;
		 //if ( OSD_KEY_EVENT != NULL )
		//	 SetEvent(OSD_KEY_EVENT);
		 return(TRUE);
	 case MDAPI_START_FILTER:
		 return MD__StartFilter(lParam);
	 case MDAPI_STOP_FILTER:
		 return MD__StopFilter(lParam);
	 case MDAPI_DVB_COMMAND:
		 {
			 int i;

			 if ( lParam == 0 )
				 return(FALSE);
			 
			 if (v->nGotKeys == GOT_DISABLE)
				 return FALSE;

			 memcpy(&dvb_com,(struct TDVB_COMMAND *)lParam,sizeof(struct TDVB_COMMAND));
			 
			 wsprintf(szTemp,"MDAPI_DVB_COMMAND: ");
			 for (i = 0; i < dvb_com.Cmd_laenge; i++)
			 {
				 char szTemp2[10];
				 wsprintf(szTemp2, "%04x ", dvb_com.Cmd_Buffer[i]);
				 lstrcat(szTemp, szTemp2);
			 }
			 Write_Log(szTemp);
			 
			 if (dvb_com.Cmd_Buffer[0] == 0x0110 || dvb_com.Cmd_Buffer[0] == 0x0410)
			 {
				 if (dvb_com.Cmd_Buffer[1] == 0x0005)
				 {
					 BYTE * pSource = (BYTE *)&dvb_com.Cmd_Buffer[3];

					 // Key from CA module
					 EnterCriticalSection(&csDecrypt);
					 if (v->fWarnAboutCSA)
					 {
						 v->fWarnAboutCSA = FALSE;
						 PostMessage(v->hWndMainWindow, WM_USER + 13, 0, 0);
					 }

					 switch(dvb_com.Cmd_Buffer[2])
					 {
					 case 0:	// even key
						 v->cwkey[0] = pSource[1]; v->cwkey[1] = pSource[0];
						 v->cwkey[2] = pSource[3]; v->cwkey[3] = pSource[2];
						 v->cwkey[4] = pSource[5]; v->cwkey[5] = pSource[4];
						 v->cwkey[6] = pSource[7]; v->cwkey[7] = pSource[6];
						 v->nGotKeys |= GOT_EVEN_KEY;
						 break;
					 case 1:	// odd key
						 v->cwkey[8 + 0] = pSource[1]; v->cwkey[8 + 1] = pSource[0];
						 v->cwkey[8 + 2] = pSource[3]; v->cwkey[8 + 3] = pSource[2];
						 v->cwkey[8 + 4] = pSource[5]; v->cwkey[8 + 5] = pSource[4];
						 v->cwkey[8 + 6] = pSource[7]; v->cwkey[8 + 7] = pSource[6];
						 v->nGotKeys |= GOT_ODD_KEY;
						 break;
					 }
					 if (v->nGotKeys == GOT_BOTH_KEYS)
					 {
						if (v->hFFCSA == NULL)
						{
							 if (ptr_set_cws != NULL)
								ptr_set_cws(v->cwkey, &v->key);
						}
					 }
					 LeaveCriticalSection(&csDecrypt);
				 }
			 }
		 }
		 return(TRUE);
	 case TSREADER_MDAPI_GET_PIDS:
		 {
			int nPID;
			BYTE * bDataPointer = (BYTE *)lParam;
			
			for (nPID = 0; nPID < 8192; nPID++)
			{
				if (v->lnPIDCounter[nPID])
				{
					if (v->fPIDScrambled[nPID])
						bDataPointer[nPID] = 2;
					else
						bDataPointer[nPID] = 1;
				}
				else
					bDataPointer[nPID] = 0;
			}
		 }
		 break;
	 case TSREADER_MDAPI_SWITCH_IP_MODE:
		 {
			int * nEnable = (int *)lParam;
			
			if (nEnable[0] == 0x1fff)
			{
				IPDVBModeOff(v->hDlgSIParser);
			}
			else
			{
				int nOffset = 0;
				memset(&v->nIPMonitorPID[0], -1, sizeof(v->nIPMonitorPID[0]) * 8192);
				while (nEnable[nOffset] != 0x1fff && nOffset < MAX_RECORD_BUFFERS)
				{
					v->nIPMonitorPID[nEnable[nOffset]] = nOffset;
					nOffset++;
				}
				IPDVBModeOn(v->hDlgSIParser);
			}
		 }
		 break;
	 case TSREADER_MDAPI_DIALOG_MESSAGE_ENABLE:
		 {
			 HWND hDlg = (HWND)lParam;
			 int i;

			 for (i = 0; i < 16; i++)
			 {
				 if (v->hPluginTranslateDialog[i] == NULL)
				 {
					v->hPluginTranslateDialog[i] = hDlg;
					return TRUE;
				 }
			 }
			 return FALSE;
		 }
	 case TSREADER_MDAPI_DIALOG_MESSAGE_DISABLE:
		 {
			 HWND hDlg = (HWND)lParam;
			 int i;

			 for (i = 0; i < 16; i++)
			 {
				 if (v->hPluginTranslateDialog[i] == hDlg)
				 {
					v->hPluginTranslateDialog[i] = NULL;
					return TRUE;
				 }
			 }
			 return FALSE;
		 }
		 break;
	 }
	 
	 return (FALSE);
}

void MD__ChannelChange(int nProgramNumber,
					   int nVideoPID, int nAudioPID, int nTeletextPID, int nPCRPID,
					   int nPMTPID, int nECMPID, TCA_System *CA,
					   char * szChannelName)
{
	int i;
	int nCACount = 0;

/*
	char              Name[30];
	char              Anbieter[30];
	char              Land[30];
    unsigned long     freq;
    unsigned char     Typ;
	unsigned char     volt;              
	unsigned char     afc;
	unsigned char     diseqc;            
	unsigned int      srate;         
	unsigned char     qam;               
	unsigned char     fec;   
	unsigned char     norm;
*	unsigned short    tp_id;        
*	unsigned short    Video_pid;        
*	unsigned short    Audio_pid;
*   unsigned short    TeleText_pid;          // Teletext PID 
*	unsigned short    PMT_pid;
*   unsigned short    PCR_pid;
	unsigned short    ECM_PID;
*	unsigned short    SID_pid;
*	unsigned short    AC3_pid;
*	unsigned char     TVType; //  == 00 PAL ; 11 == NTSC    
*	unsigned char     ServiceTyp;
    unsigned char     CA_ID;
	unsigned short    Temp_Audio;
	unsigned short    Filteranzahl;
    struct PIDFilters Filters[MAX_PID_IDS];
	unsigned short    CA_Anzahl;
    struct TCA_System CA_System[MAX_CA_SYSTEMS];
    char    CA_Land[5];
    unsigned char Merker;
    unsigned short Link_TP;
    unsigned short Link_SID;
    unsigned char Dynamisch;

    char Extern_Buffer[16];

*/
	v->nGotKeys = GOT_NO_KEYS;
	memset(bCurrentKeys, 0, 16);
	memset(&csaCurrent.odd_ck[0], 0, 8);
	memset(&csaCurrent.even_ck[0], 0, 8);

	memset(&Programm[0], 0, sizeof(Programm[0]));
	Programm[0].Link_SID = Programm[0].SID_pid = (unsigned short)nProgramNumber;
	Programm[0].tp_id = v->pat.nTransportStreamID;
	Programm[0].freq = v->ss.nFrequency;
	Programm[0].Link_TP = 0x00;
	Programm[0].PMT_pid = (unsigned short)nPMTPID;
	Programm[0].Video_pid = (unsigned short)nVideoPID;
	Programm[0].PCR_pid = (unsigned short)nPCRPID;
	Programm[0].Audio_pid = (unsigned short)nAudioPID;
	Programm[0].TeleText_pid = (unsigned short)nTeletextPID;
	Programm[0].AC3_pid = 0x1fff;
	//Programm[0].AC3_pid = 0;
	Programm[0].ECM_PID = (unsigned short)nECMPID;
	//Programm[0].TVType = 0x03;		// 11 = NTSC
	Programm[0].TVType = 0x00;
	Programm[0].Typ = 'D';		// D is for "digital"!
	//Programm[0].Typ = 0;
	Programm[0].ServiceTyp = 1;	// digital video
	//Programm[0].ServiceTyp = 0;
	Programm[0].CA_ID = 0;	
//	if (v->fUsePreferedCAID)
//		Programm[0].CA_ID = v->nPrefereredCAID;	
	lstrcpy(Programm[0].Name, szChannelName);

	for (i = 0; i < MAX_CA_SYSTEMS; i++)
	{
		if (CA[i].CA_Typ != 0)
		{
			memcpy(&Programm[0].CA_System[nCACount], &CA[i], sizeof(struct TCA_System));
			nCACount++;
		}
	}
	Programm[0].CA_Anzahl = (unsigned short)nCACount;
	//Programm[0].CA_Anzahl = MAX_CA_SYSTEMS;	
	for ( i = 0 ; i < 5 ; i++ )
	{
		if ( Ext_Dll[i].Extern_Channel_Change != NULL )
			(Ext_Dll[i].Extern_Channel_Change)(Programm[0]);
	}
}

void PotentiallyDecrypt(BYTE * pData, int nPID, int nPacketCounter)
{
	int j;

	EnterCriticalSection(&csDecrypt);
	if (v->nGotKeys == GOT_BOTH_KEYS)
	{
		for (j = 0; j < v->nDecryptPIDCounter; j++)
		{
			if (v->nDecryptPIDs[j] == nPID)
			{
				if ((pData[3] & 0x80))
				{
					if (v->hFFCSA == NULL)
					{
						// Standard CSA - descramble for now
						if (v->hCSA != NULL)
							ptr_decrypt(&v->key, pData);
					}
					else
					{
						// Fast CSA - copy keys for now - decrypt comes later
						memcpy(&pKeys[nPacketCounter * 16], v->cwkey, 16);
					}
					break;
				}
			}
		}
	}
	LeaveCriticalSection(&csDecrypt);
}

void FFDecrypt(BYTE ** pClusters, int nClusterCount)
{
	int nOffset = 0;
	int nOriginalClusterCount = nClusterCount;
	BYTE ** pCopyClusters = LocalAlloc(LPTR, sizeof(BYTE *) * (nClusterCount * 2 + 2));

	memcpy(pCopyClusters, pClusters, sizeof(BYTE *) * (nOriginalClusterCount * 2 + 2));
	while (nClusterCount)
	{
		int nDecrypted = decrypt_packets(&pClusters[nOffset], v->pFFKeySpace);
		if (nDecrypted == 0)
		{
			OutputDebugString("MDAPI: FFCSA decrypt error - 0 packets decrypted\n");
			break;		// oops
		}
		nOffset += nDecrypted * 2;
		nClusterCount -= nDecrypted;
		if (nClusterCount)
			memcpy(pClusters, pCopyClusters, sizeof(BYTE *) * (nOriginalClusterCount * 2 + 2));
	}

	LocalFree(pCopyClusters);
}

BOOL NullKey(BYTE * pSomeKeys)
{
	int i;

	if (pSomeKeys == NULL)
		return FALSE;

	for (i = 0; i < 16; i++)
	{
		if (pSomeKeys[i] != 0x00)
			return FALSE;
	}
	return TRUE;
}

void MD__DataToFilters(BYTE * pData, int * nSize, int * nPacketLength)
{
	int i, j;
	int nPacketCounter = 0;
	BYTE * pOriginalData = pData;
	
	// All data hook
	for (j = 0; j < MAX_START_FILTERS; j++)
	{
		if (start_filter[j].Irq_Call_Adresse != 0)
		{
			if (start_filter[j].Pid == 0x9fff)
			{
				int nDLL_ID = start_filter[j].DLL_ID;
				int nFilter_ID = start_filter[j].Filter_ID;
				int nNewSize;
				BYTE bFirstByte = *pOriginalData;

				nNewSize = Ext_Dll[nDLL_ID].Extern_Stream_Function[nFilter_ID](nFilter_ID, *nSize, pOriginalData);
				if (nNewSize != *nSize)
					*nSize = nNewSize;
				if (*pOriginalData != bFirstByte)
				{
					if (*nPacketLength == 131 && *pOriginalData == 0x47)
					{
						// just switched back to MPEG-2 mode 
						*nPacketLength = 188;
						v->nNullPID = 0x1fff;
						v->nMuxRatePID = 0;
						SendMessage(v->hWndMainWindow, WM_COMMAND, ID_HELP_RESETALL, 0);
					}
				}
			}
		}
	}

	// Data comes in from the main thread. If we have a filter setup for a PID
	// we buffer up to 1K of data before sending it down to the plugin.
	// I have no idea why it's done that way, but sending a single packet
	// screws things up. Note that 184 bytes from each packet is sent - the
	// TS header is dropped which is really dumb.

	if (v->hFFCSA != NULL)
		pKeys = LocalAlloc(LPTR, (*nSize / *nPacketLength) * 16);

	for (i = 0; i < *nSize; i += *nPacketLength)
	{
		int nPID;

		if (*nPacketLength == 188)
			nPID = (pData[1] << 8 | pData[2]) & 0x1fff;
		else 
			nPID = (pData[1] << 8 | pData[2]) & 0x0fff;
		
		if (nPID != v->nNullPID)
		{
			for (j = 0; j < MAX_START_FILTERS; j++)
			{
				if (start_filter[j].Irq_Call_Adresse != 0)
				{
					if ((start_filter[j].Pid & 0x1fff) == nPID)
					{
						if (filtertspackets[j].pBuffer != NULL)
						{
							if ((start_filter[j].Pid & 0x8000) == 0)
							{
								memcpy(filtertspackets[j].pBuffer + filtertspackets[j].nFillPtr, &pData[4], *nPacketLength - 4);
								filtertspackets[j].nFillPtr += *nPacketLength - 4;
							}
							else
							{
								memcpy(filtertspackets[j].pBuffer + filtertspackets[j].nFillPtr, pData, *nPacketLength);
								filtertspackets[j].nFillPtr += *nPacketLength;
							}
						}
						if (filtertspackets[j].nFillPtr + (*nPacketLength - 4) > *nPacketLength - 4)
						{
							int nDLL_ID = start_filter[j].DLL_ID;
							int nFilter_ID = start_filter[j].Filter_ID;
							Ext_Dll[nDLL_ID].Extern_Stream_Function[nFilter_ID](nFilter_ID, filtertspackets[j].nFillPtr, filtertspackets[j].pBuffer);
							filtertspackets[j].nFillPtr = 0; // reset pointer
							WaitForInputIdle(GetCurrentProcess(), INFINITE);
						}
					}
				}
			}
			PotentiallyDecrypt(pData, nPID, nPacketCounter);
		}
		pData += *nPacketLength;
		nPacketCounter++;
	}

	if (v->hFFCSA != NULL)
	{
		int nClusterCount = 0;
		int nClusterPointer = 0;
		int nPacketCount = 0;
		int nPackets = *nSize / *nPacketLength;
		BYTE ** pClusters = LocalAlloc(LPTR, sizeof(BYTE *) * ((v->nFFCSAPackets * 2) + 2)); // move to v->

		for (nPacketCount = 0; nPacketCount < nPackets; nPacketCount++)
		{
			if (NullKey(&pKeys[nPacketCount * 16]) == FALSE)
			{
				// Key present - check for first time
				if (NullKey(bCurrentKeys) == TRUE)
				{
					memcpy(bCurrentKeys, &pKeys[nPacketCount * 16], 16);
					set_control_words(&bCurrentKeys[0], &bCurrentKeys[8], v->pFFKeySpace);
				}

				// Key changed?
				if (memcmp(bCurrentKeys, &pKeys[nPacketCount * 16], 16) != 0)
				{
					// yes, decrypt existing packets scrambled
					// with the prior key
					if (nClusterCount)
					{
						pClusters[nClusterPointer] = NULL;
						FFDecrypt(pClusters, nClusterCount);
					}

					// update to the current key
					memcpy(bCurrentKeys, &pKeys[nPacketCount * 16], 16);
					set_control_words(&bCurrentKeys[0], &bCurrentKeys[8], v->pFFKeySpace);
					nClusterPointer = 0;
					nClusterCount = 0;
				}

				// Add to current cluster
				pClusters[nClusterPointer] = &pOriginalData[nPacketCount * *nPacketLength];
				if (pClusters[nClusterPointer] == NULL)
				{
					OutputDebugString("NULL pClusters[nClusterPointer] #1\n");
				}
				nClusterPointer++;
				pClusters[nClusterPointer] = &pOriginalData[(nPacketCount * *nPacketLength) + *nPacketLength];
				if (pClusters[nClusterPointer] == NULL)
				{
					OutputDebugString("NULL pClusters[nClusterPointer] #2\n");
				}
				nClusterPointer++;
				nClusterCount++;
				if (nClusterCount == v->nFFCSAPackets)
				{
					pClusters[nClusterPointer] = NULL;
					FFDecrypt(pClusters, nClusterCount);
					nClusterPointer = 0;
					nClusterCount = 0;
				}
			}
		}
		if (nClusterCount)
		{
			pClusters[nClusterPointer] = NULL;
			FFDecrypt(pClusters, nClusterCount);
		}

		LocalFree(pClusters);
		LocalFree(pKeys);
	}
}

void MD__IPDataToFilters(int nPID, BYTE * pData, PMPEIPPACKET pmpeippacket)
{
	int i;

	for (i = 0; i < 5; i++)
	{
		if (ipdata[i] != NULL)
			ipdata[i](nPID, pData, pmpeippacket);
	}
}