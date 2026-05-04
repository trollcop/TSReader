/*********************************************************************************
*                                                                               *
* MultiDec.c                                                                    *
*                                                                               *
* Copyright (C) 1999/2000 Espresso                                              *
*                                                                               *
*                                                                               *
* This program is free software; you can redistribute it and/or                 *
* modify it under the terms of the GNU General Public License                   *
* as published by the Free Software Foundation; either version 2                *
* of the License, or (at your option) any later version.                        *
*                                                                               *
*                                                                               *          
* This program is distributed in the hope that it will be useful,               *
* but WITHOUT ANY WARRANTY; without even the implied warranty of                *
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the                 *
* GNU General Public License for more details.                                  *
*                                                                               *
*                                                                               *
* You should have received a copy of the GNU General Public License             *
* along with this program; if not, write to the Free Software                   *
* Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.    *
* Or, point your browser to http://www.gnu.org/copyleft/gpl.html                *
*                                                                               *
*                                                                               *
* The author can be reached at echter_espresso@hotmail.com                      *
*********************************************************************************/ 

#include <windows.h>       /* required for all Windows applications */
#include <stdio.h>
#include "resource.h"
#include <stdlib.h>        /* atoi                                  */
#include <memory.h>
#include <io.h>         
#include <fcntl.h>
#include <commctrl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <commdlg.h>
#include <time.h>
#include <process.h>       /* for _beginthread                      */
#include <mmsystem.h>
#include <ddraw.h>
#include <mmsystem.h>
#include "../DrvLib/windrvr.h"
#include "MultiDec.h"

static LPBYTE dst;
static LPBYTE src;
static int nFrame;


static char Text[128];

void Write_Log(LPSTR Text);

static int i;

static long Offset;
static DWORD   dwLastCount = 0;

extern void Start_Thread();
extern void Stop_Thread();


HWND VThWnd;

int PriorClassId;
int ThreadClassId=1;


clock_t Clockstart, Clockfinish;
double  duration;

SYSTEM_INFO SysInfo;

UINT                 WTVBPP;       
DWORD                dwDIB                   = 0;
HGLOBAL              hDIB                    = NULL;
BITMAPINFO          *pDIB                    = NULL;


BOOL CShowCursor=TRUE;


BOOL Load_INI(LPSTR Name);
BOOL Write_INI(LPSTR Name);
void Init_More();
void More_Even( int pp );
void More_Odd();
int G_Wert(int Wert1,int Wert2);
char Upcase(char Zeichen);

VOID APIENTRY HandleFocusStateKanalListe(HWND hDlg,LPDRAWITEMSTRUCT lpdis);
VOID APIENTRY DrawEntireItemKanalListe(HWND hDlg,LPDRAWITEMSTRUCT lpdis,INT Typ);
VOID APIENTRY HandleFocusStateEventListe(HWND hDlg,LPDRAWITEMSTRUCT lpdis);
VOID APIENTRY DrawEntireItemEventListe(HWND hDlg,LPDRAWITEMSTRUCT lpdis,INT Typ);
void GetFeldName(short id, char *zeile);
void  SetMenuAnalog();
void  SetMenuDigital();
void GetFeldName(short id, char *zeile);
BOOL Is_Sat_Karte(void);
BOOL Is_Kabel_Karte(void);

BOOL MainCommandDispatch(HWND hWnd,UINT message,UINT wParam,LONG lParam);
BOOL ExternCommandDispatch(HWND hWnd,UINT message,UINT wParam,LONG lParam);

// API
void Load_External_Dll(void);
void Unload_External_Dll(int Nummer);


BOOL APIENTRY KanalNummerProc(HWND hDlg,UINT message,UINT wParam,LONG lParam);
void Init_Kernel_Memory( void );
void Work_Text(unsigned char *TmpBuffer);
int Get_Page_Num(unsigned char *Buffer);
int Get_SubPage_Anzahl(int Current,unsigned char *Buffer);
int Save_TXT_Page(int PageNum, int SubPageNum, int SubPagesAnzahl, unsigned int Page_CRC, unsigned char PageLang, unsigned char *Buffer);

BOOL HexCheck(unsigned char *Value);
BOOL Init_Heap_Memory( void );
void Load_All_Parameters(struct TProgramm *tptg);

//Aus Nagra.c
extern BOOL LeseKeyDatei(HWND hWnd);
extern BOOL Init_Nagra_Decoding( int Lines );
extern void Free_Nagra_Decoding(void);
extern int FindePalPlusZeile(short *pLine,int  *nPALplusData);
extern void FindYUVLineOrder(short **pLinesAddress ,int *pSortLines,  int  *nPALplusZeile,int  *nPALplusData);
extern void FindYUVSecondLineOrder(short **pLinesAddress ,int *pSortLines);
extern void Init_HQTabelle();
extern BOOL GetMatrix(int BlackWert,int evenodd);
// Fantasio
extern void FilterBlueLines(DWORD **pLinesAddress, int *pSortLines, BOOL bEven);

// Service
extern BOOL LoadDeviceDriver( const TCHAR * Name, const TCHAR * Path, HANDLE * lphDevice,BOOL Install);
extern int Start_MDHal( void );
extern int PCI_Scan( void );
extern BOOL UnloadDeviceDriver( const TCHAR * Name,BOOL DRemove);

// Aus MixerDev.c

extern void Enumerate_Sound_SubSystem(void);
extern void Mixer_Get_Volume(int *Links, int *Rechts);
extern void Mixer_Set_Volume(int Links, int Rechts);
extern void Get_Volume_Param(void);
extern void Mixer_Set_Defaults( void );
extern void Mixer_Mute(void);
extern void Mixer_UnMute(void);
extern void Exit_Mixer( void );

// Aus Sounddec.c
extern void SoundCapture_Open() ;
extern void SoundCapture_Close () ;
extern int SoundCapture_Play() ;
extern void SoundCapture_Stop() ;

// HC-Optionen
void Init_HCYUV2RGBTabelle(BOOL  RGB15);
void  HCZeileAusgeben(DWORD *pQuelle,DWORD *pZiel,int Count);
void  HCZeileAusgeben_2(DWORD *pQuelle,DWORD *pZiel,int Pitch,int Count);
void  HCZeileAusgeben_3(DWORD *pQuelle,DWORD *pZiel,int Pitch,int Count);
void  HCZeileAusgeben_4(DWORD *pQuelle,DWORD *pZiel,int Pitch,int Count);

void Clear_Status();


// Aus Vcrypt.c
extern BOOL Find_Cut_Points_VC(short **pLinesAddressEven,short *RealOffset, int Burst);
extern void VC_UVColor(short *pQuelle,short *pZiel,short *pColorFaktor, int Cut);
extern int Get_Decoder_Key(short **pLinesAddressEven);
void Init_VC_HQTabelle();
extern void __fastcall VCHQAusgebenRGB16( void );
extern void __fastcall VCHQAusgebenRGB15( void );
extern void __fastcall VCHQAusgebenYUV2( void );
extern void __fastcall HQAusgebenRGB16( void );
extern void __fastcall HQAusgebenRGB16DVB( void );
extern void __fastcall HQAusgebenRGB15( void );
extern void __fastcall HQAusgebenRGB15DVB( void );
extern void __fastcall HQAusgebenYUV2( void );
extern void __fastcall HQAusgebenYUV2DVB( void );
extern void __fastcall HQFillRGB( void );
extern void __fastcall HQFillYUV2( void );

extern void Init_VC(void);
extern Free_VC_Decoding(void);
extern int decodePicture(int lines);

// dvb.c
extern void Free_EIT( void );
extern BOOL Is_Time_Link(unsigned short SID, unsigned short ts_id,BOOL Current);
extern BOOL Is_View_Link(unsigned short SID, unsigned short ts_id,BOOL Current);
extern BOOL Is_Pid_Param(unsigned short SID, unsigned short ts_id,BOOL Current);
extern void Make_Pid_Name(unsigned char Pid_Id, char *Text);
extern void GetCurrentNext(unsigned short sid_id,unsigned short ts_id,int typ,unsigned char *Titel,unsigned char *StartZeit);
extern BOOL Set_DVB_Channel_by_SID_TS_ID( unsigned short ts_id, unsigned short sid,char *RealName);
extern int Get_Virtual_Event(unsigned short EventId,unsigned short SID);
extern void Stop_Eit_Filters( void );
extern void Start_Eit_Filters( void );
extern void Start_TID_Filters( void );
extern void Stop_TID_Filters( void );
extern void CI_Reset(void);
extern void CI_Open(void);
extern void CI_Close(void);
extern BOOL UseDiseqC(void);
extern int SetQAM( unsigned short QAM_Mode, int DoCLB);
extern BOOL Set_Transponder_Trace(HWND hDlg, struct TTransponder *Transponder );
extern BOOL Get_Pat( void );
extern BOOL Set_Transponder_Diseq_Scan(HWND hDlg, struct TTransponder *Transponder );
extern int Get_Tuner_Status( void );
extern void Relink_Current_Eit(unsigned short SID,unsigned short ts_id,unsigned short To_SID,unsigned short To_ts_id);
extern void Init_Crystal_Dac( void ) ;
extern long RealDay(unsigned char Stunde, long Day);
extern void  VES1820_SetInversion(int inversion);

extern unsigned short Swab16(unsigned char *dat);
extern void set_up_grabbing();
extern void Set_DVB_Channel(int Nummer);
extern void DVB_SetVolume(unsigned char volleft, unsigned char volright);
extern void DVB_Reset( void );
extern void set_picture_prop(unsigned int brightness, unsigned int contrast, unsigned int colour);
extern BOOL Init_DVB_Karte(HWND hWnd);
extern BOOL Init_DVB_HardWare(void);
extern int CreateOSDWindow( unsigned char windownr,DISPTYPE disptype, unsigned short width, unsigned short height);
extern int MoveWindowAbs( unsigned char windownr, unsigned short x, unsigned short y);
extern int SetFont(unsigned char  windownr, unsigned short  fontsize,unsigned short  colorfg, unsigned short  colorbg);
extern int WriteText(unsigned char win, unsigned short x, unsigned short y, unsigned char *buf);
extern void Run_DVB_Irq(BOOL State, BOOL Keep_Run_IR);
extern int DestroyOSDWindow(unsigned char windownr);
extern int DVBVideoPresent( void );
extern void Free_SAA7146_DMA( WD_DMA *dma );
extern void Free_DVB_Display_DMA( void );
extern void Get_Signal_Infos(unsigned char *Sync , unsigned int *Quality,unsigned int *BitErrorRate );
extern BOOL Get_SDT( void );
extern BOOL Get_PAT( void );
extern BOOL Get_Nit( unsigned char diseq);
extern BOOL Update_Nit_List(int NeuWert );
extern BOOL Update_Programm_List(int NeuWert );
extern void ShutDownFilters( void );
extern void ShutDownFiltersByName( LPSTR Name );
extern int SetFilter(unsigned short pid, unsigned short section, unsigned short mode, LPSTR Name); 
extern unsigned short RealTime(unsigned char Stunde);
extern int Get_Sender(unsigned short SID_Id,unsigned short ts_id,unsigned char *Name);
extern int Find_EIT_Prog_State(unsigned short SID_Id,unsigned short ts_id,unsigned char State);
extern void GetAudioName(int Nummer,int FilterId, unsigned char *Name);
extern int FindActiveAudioName(int Nummer, unsigned char *Name);
extern void SetAudioPid(unsigned short AudioPid);
extern void OSD_TestFenster( void );
extern void Close_OSD_TestFenster( void );
extern int Get_New_Prog_by_SID(unsigned short SID);
extern BOOL Get_CAT(void);
extern BOOL Get_PMT(int ProgrammNummer, unsigned short pid );
extern BOOL Set_Transponder_SRate(struct TTransponder *Transponder , unsigned int SRate);
extern BOOL Set_Transponder(struct TTransponder *Transponder );
extern void Set_Transponder_Freq(struct TTransponder *Transponder);
extern void Set_Transponder_Pol(struct TTransponder *Transponder);
extern void OSD_Startzeit_Liste( void);
extern void OSD_ChannelInfo(void);
extern void OSD_Extra_Kanal_Liste( void);
extern void OSD_ProgInfo( void);
extern void Run_OSD_DLL_Call( void );
extern void OSD_CAM_Menu( void);
extern void OSD_CAM_MessagePopUp( void);
extern void OSD_Eit_Liste_Current_Programm( void );
extern void OSD_Eit_Liste_Current_Next( void );
extern void OSD_Eit_Info( void );
extern void OSD_Uhr( void );
extern void OSD_Mute( void );
extern void OSD_Rec( void );
extern void OSD_No_Sig( void );
extern void OSD_Eit_ProgrammListe( void );
extern void OSD_Audio_Liste( void );
extern void OSD_Text_Liste( void );
extern void OSD_Recorder_Info( void );
extern void Update_OSD_Volume( void );
extern void Update_OSD_Channel_Change( void );
extern void Update_OSD_Color( void );
extern void Update_OSD_Color( void );
extern void Create_Osd_Thread( LPTHREAD_START_ROUTINE ThreadProzess);
extern void SAA7146_Restart_Arm( void );
extern int  SAA7146_OutCommand( unsigned short *buf, short length);
extern void SetDiSEqC( int DiseqCNr, int Band, int Volt);
extern void Set22K(int state);
extern void SetMonitor_16_9(BOOL Use_16_9);
extern int StartLogFilter( unsigned short Pid , int Irq_Proc , LPSTR Name);
extern void DrawBlock( unsigned char windownr,unsigned short x, unsigned short y, unsigned short dx, unsigned short dy, unsigned short color);

extern void Get_Viaccess_Provider_Name( unsigned int Prov_id, char *Name);
extern void Get_Seca_Provider_Name( unsigned int Prov_id, char *Name);
extern void Get_Ird_Beta_Provider_Name( unsigned int Prov_id, char *Name);

extern void Mapi_DelBitFilter(int FilterNummer);


// oxident
extern void OSD_MenuListe( void );
// ---
extern BOOL Debi_Record_Start(RecorderFormat,RecorderTyp);
extern BOOL Debi_Record_Play(int RecordTyp);
extern BOOL Debi_Record_Stop(void);
extern BOOL APIENTRY RecordPlayInterrupt(HWND hDlg,UINT message,UINT wParam,LONG lParam);


extern unsigned short GetDateID(unsigned short year,unsigned short month,unsigned short day);
extern void Date_To_String( unsigned short Date_Id,char *DatumsString);
extern void DecodeDVBDate(unsigned short DVBDate, unsigned short *year,unsigned short *month,unsigned short *day);
extern void Stop_Osd_Key_Event();
extern BOOL APIENTRY SaaBootOptionProc(HWND hDlg,UINT message,UINT wParam,LONG lParam);

BOOL APIENTRY MDBootOptionProc(HWND hDlg,UINT message,UINT wParam,LONG lParam);
BOOL APIENTRY MDShutdownOptionProc(HWND hDlg,UINT message,UINT wParam,LONG lParam);
BOOL APIENTRY MDShutdownSetProc(HWND hDlg,UINT message,UINT wParam,LONG lParam);
BOOL APIENTRY Stop_SystemProc(HWND hDlg,UINT message,UINT wParam,LONG lParam);
extern void DVB_Close();

extern void Scan_TP( HWND Ausgabe);
extern int Find_Programm(unsigned short tp_id, unsigned short sid );
extern int  Start_VT_Stream( unsigned short TextPid );
extern void Stop_VT_Stream( void );


BOOL ValidModes(char Mode);
void Set_Mode_Digital( void );
void Set_Mode_Analog( void );
// Aus RemoteC.c 
void Init_Key_Table( void );

// Aus BT848.c 
extern BOOL Check_BT_Plugin( void );
extern BOOL Check_WD_Version( void );
extern void Wait_DStatus();
extern void Get_Thread_Status();
extern BOOL Init_BT_Kernel_Memory( void );
extern void Reset_BT_HardWare();
extern BOOL Init_Memory();
extern void VideoDat_Exit( void );
extern void MakeVBITable(int VBI_Lines);
extern void MakeVBITable2(int VBI_Lines);
extern void MakeVBITable3(int VBI_Lines);
extern void MakeVBITable4(int VBI_Lines);
extern void MakeVBITable5(int VBI_Lines);
extern void Run_BT_Irq(BOOL State);
extern void VideoDat_Init( void );
extern void Set_BT_Decrypt_Mode();
extern void BT_Close();
extern BOOL Alloc_Surface_DMA();
extern BOOL SetGeoSize(int wWidth, int wHeight);
extern BOOL SetAudioSource(int nChannel);
extern BOOL SetVideoSource(int nInput);
extern BOOL Tuner_Scan();
extern BOOL SetColourFormat(int nColourFormat);
extern void Load_Country_Settings();
extern void Load_Country_Specific_Settings(int LPos);
extern BOOL Init_Tuner(int TunerNr );
extern void Set_Capture(int nState);
extern void Winbis_Exit( void );
extern void Init_Winbis( void );
extern void Set_CurrentIrq(DWORD Wert);

extern void Seca_Date( int *DatumsID , char *Datumsstring);
extern int Get_NextValid_Seca_Cmd(int *Buffer,int Pos);
extern void Seca_Superencryption_Decode( int *Value, int *Decode);
extern void Seca_Init_Superencryption(int *MK);
extern void Write_SECA_Log(LPSTR Text);


extern BOOL APIENTRY ChipSettingProc(HWND hDlg,UINT message,UINT wParam,LONG lParam);
extern BOOL APIENTRY IFormSettingProc(HWND hDlg,UINT message,UINT wParam,LONG lParam);
extern BOOL APIENTRY PLLSettingProc(HWND hDlg,UINT message,UINT wParam,LONG lParam);
extern BOOL APIENTRY TunerSettingProc(HWND hDlg,UINT message,UINT wParam,LONG lParam);
extern BOOL APIENTRY TunerDigiSettingProc(HWND hDlg,UINT message,UINT wParam,LONG lParam);
extern BOOL APIENTRY PlayInfoControl(HWND hDlg,UINT message,UINT wParam,LONG lParam);
extern BOOL APIENTRY PlayControlDlg(HWND hDlg,UINT message,UINT wParam,LONG lParam);
extern BOOL APIENTRY PlayTimeControl(HWND hDlg,UINT message,UINT wParam,LONG lParam);
extern BOOL APIENTRY PlayMarkControl(HWND hDlg,UINT message,UINT wParam,LONG lParam);
extern BOOL APIENTRY RecordControlDlg(HWND hDlg,UINT message,UINT wParam,LONG lParam);
extern BOOL APIENTRY DecoderStatusProc(HWND hDlg,UINT message,UINT wParam,LONG lParam);
extern BOOL APIENTRY CIStatusProc(HWND hDlg,UINT message,UINT wParam,LONG lParam);
extern BOOL APIENTRY CIMenuProc(HWND hDlg,UINT message,UINT wParam,LONG lParam);
extern BOOL APIENTRY CardSettingProc(HWND hDlg,UINT message,UINT wParam,LONG lParam);
extern BOOL APIENTRY LowSettingProc(HWND hDlg,UINT message,UINT wParam,LONG lParam);
extern BOOL APIENTRY MixerSetupProc(HWND hDlg,UINT message,UINT wParam,LONG lParam);
extern BOOL APIENTRY StreamFilterSetup(HWND hDlg,UINT message,UINT wParam,LONG lParam);
BOOL APIENTRY DetectSettingProc(HWND hDlg,UINT message,UINT wParam,LONG lParam);
BOOL APIENTRY ToggleSettingProc(HWND hDlg,UINT message,UINT wParam,LONG lParam);
BOOL APIENTRY ListKonfigProc(HWND hDlg,UINT message,UINT wParam,LONG lParam);

extern void Make_Crypt_Systems(int Nr, char *Text);
extern void Make_Service_Name(unsigned char ServiceTyp, char *Text);
extern void Make_Crypt_Name(unsigned char CryptId, char *Text);
extern void Insert_CAT(int Nr);
extern void DelBitFilter(int FilterNummer);
extern int SetStreamFilter( unsigned short pid,LPSTR Name);


extern void Work_VideoDat(unsigned char *Buffer);

BOOL APIENTRY ResSettingProc(HWND hDlg,UINT message,UINT wParam,LONG lParam);
extern void Start_VBI();
extern void Stop_VBI();
extern BOOL Init_TV_Karte(HWND hWnd);
extern BOOL InitTV( HWND hwnd , BOOL Fullscreen, int x , int y );
extern BOOL Init_Audio(BYTE DRead,BYTE DWrite );
extern BOOL ExitTV( void );
extern BOOL Init_BT_HardWare(HWND hWnd);
extern BOOL VideoPresent();
extern BOOL Alloc_DMA(DWORD dwSize, WD_DMA *dma,int Option );
extern BOOL Alloc_Display_DMA(DWORD dwSize, int NR);
extern void Free_DMA( WD_DMA *dma );
extern void Free_Display_DMA( int NR );
extern void Free_Surface_DMA( void );

extern BOOL Tuner_SetFrequency(int TunerTyp, int wFrequency);
extern BOOL SetBrightness(char bBrightness);
extern BOOL SetHue(char bHue);
extern BOOL SetContrast(int wContrast);
extern BOOL SetSaturationU(int wData);
extern BOOL SetSaturationV(int wData);
extern void SetColor(int Croma);

extern void MSP_SetMode(int type);
extern void MSP_SetStereo(int MajorMode, int MinorMode, int mode);
extern void MSP_Set_MajorMinor_Mode(int MajorMode,int MinorMode  );
extern void MSP_Print_Mode( );
extern void MSPWatch_Mode();


extern BOOL Audio_SetBalance(char nBalance);
extern BOOL Audio_SetBass(char nBass);
extern BOOL Audio_SetTreble(char nTreble);
extern BOOL Audio_SetLoudness(BYTE nLoudness);
extern BOOL Audio_SetSpatial(char nSpatial);
extern BOOL Audio_SetVolume(int nVolume);
extern BOOL Audio_SetSuperBass(BOOL bSuperBass);
extern BOOL Audio_SetEqualizer1(char nLevel);
extern BOOL Audio_SetEqualizer2(char nLevel);
extern BOOL Audio_SetEqualizer3(char nLevel);
extern BOOL Audio_SetEqualizer4(char nLevel);
extern BOOL Audio_SetEqualizer5(char nLevel);

extern void Destroy_Overlay();
extern BOOL OverlayUpdate(int x,int y,LPRECT pDestRect,DWORD  dwFlags,BOOL ColorKey);
extern void  Black_Overlay();
extern BOOL CreateOverlay(int x,int y);


BOOL Check_Even( void );
BOOL Check_Odd( void );

void RGBOutThread( );
void RGBOutThreadHalf( );
void YUVOutThread( );
void YUVOutThreadHalf( );
void DX_Lock_Thread( );

void NagraYUVHalfOutThread( );
void NagraYUVOutThread( );
void NagraOhneKeyYUVHalfOutThread( );


void VCYUVHalfOutThread( ); 
void VCDecoderYUVHalfOutThread(  ); 

void Send_External_DLL_Menu_Cmd( unsigned int MessageId);

// Aus VT.C
void VT_ChannelChange();
VOID APIENTRY HandleFocusState(HWND hDlg,LPDRAWITEMSTRUCT lpdis);
VOID APIENTRY DrawEntireItem(HWND hDlg,LPDRAWITEMSTRUCT lpdis,INT Typ);
extern int Get_Dialog_Slot(HWND hwnd);
extern BOOL Is_Val(unsigned char Byte);
extern void Export_VT_Page(HWND hwnd, int Page, int SubPage);

unsigned int VT_CRC(unsigned char *VTptr);

BOOL IsValidSelect(int Nr );
int Make_ProgNr( int Nr );
int Get_Real_ProgNr( int Nr );
BOOL IsValidSelect(int Nr );
int Make_ProgNr( int Nr );
int Get_Real_ProgNr( int Nr );

BOOL Tab_Scan(char *FileName,char *Key, char *zeile);

void Write_Programm_List();
void Load_Programm_List();
void Write_Nit_List();
void Load_Nit_List();
void Init_Screen_Struct();
void Restore_Mode();
void Save_Mode();
BOOL prozess_DatumZeit(int i_id,int o_id,unsigned char setting);
BOOL prozess_kanal(int i_id,int o_id,unsigned char setting);

BOOL APIENTRY ProgrammListe(HWND hDlg,UINT message,UINT wParam,LONG lParam);
BOOL APIENTRY EventListe(HWND hDlg,UINT message,UINT wParam,LONG lParam);
BOOL APIENTRY EITDetail_Proc(HWND hDlg,UINT message,UINT wParam,LONG lParam);
BOOL APIENTRY EITDetailExtra_Proc(HWND hDlg,UINT message,UINT wParam,LONG lParam);
BOOL APIENTRY EITSettingProc(HWND hDlg,UINT message,UINT wParam,LONG lParam);
BOOL APIENTRY DVBTimeSettingProc(HWND hDlg,UINT message,UINT wParam,LONG lParam);
BOOL APIENTRY NitListe(HWND hDlg,UINT message,UINT wParam,LONG lParam);
BOOL APIENTRY NitListeNeu(HWND hDlg,UINT message,UINT wParam,LONG lParam);
BOOL APIENTRY CAListe(HWND hDlg,UINT message,UINT wParam,LONG lParam);
BOOL APIENTRY ProgrammEdit(HWND hDlg,UINT message,UINT wParam,LONG lParam);
BOOL APIENTRY OSDSettingProc(HWND hDlg,UINT message,UINT wParam,LONG lParam);
BOOL APIENTRY DigiImport(HWND hDlg,UINT message,UINT wParam,LONG lParam);
BOOL APIENTRY VideoTextProc(HWND hDlg,UINT message,UINT wParam,LONG lParam);
BOOL APIENTRY VideoTextUnterTitelProc(HWND hDlg,UINT message,UINT wParam,LONG lParam);
BOOL APIENTRY SplashProc(HWND hDlg,UINT message,UINT wParam,LONG lParam);
BOOL APIENTRY VCSetupProc(HWND hDlg,UINT message,UINT wParam,LONG lParam);
BOOL APIENTRY NagraSetupProc(HWND hDlg,UINT message,UINT wParam,LONG lParam);
BOOL APIENTRY TransponderSettingProc(HWND hDlg,UINT message,UINT wParam,LONG lParam);
BOOL APIENTRY TransponderSettingProcCable(HWND hDlg,UINT message,UINT wParam,LONG lParam);
BOOL APIENTRY AudioPidSettingProc(HWND hDlg,UINT message,UINT wParam,LONG lParam);
BOOL APIENTRY TextPidSettingProc(HWND hDlg,UINT message,UINT wParam,LONG lParam);
BOOL APIENTRY StartZeitSettingProc(HWND hDlg,UINT message,UINT wParam,LONG lParam);
BOOL APIENTRY MultiViewSettingProc(HWND hDlg,UINT message,UINT wParam,LONG lParam);
BOOL APIENTRY ByteFilterScanProc(HWND hDlg,UINT message,UINT wParam,LONG lParam);
BOOL APIENTRY StreamFilterScanProc(HWND hDlg,UINT message,UINT wParam,LONG lParam);
BOOL APIENTRY AnalogScanProc(HWND hDlg,UINT message,UINT wParam,LONG lParam);
BOOL APIENTRY SATScanProc(HWND hDlg,UINT message,UINT wParam,LONG lParam);
BOOL APIENTRY CableScanProc(HWND hDlg,UINT message,UINT wParam,LONG lParam);
BOOL APIENTRY SATScanNitProc(HWND hDlg,UINT message,UINT wParam,LONG lParam);
BOOL APIENTRY CableScanNitProc(HWND hDlg,UINT message,UINT wParam,LONG lParam);
BOOL APIENTRY SRateSettingProc(HWND hDlg,UINT message,UINT wParam,LONG lParam);
BOOL APIENTRY VideoSettingProc(HWND hDlg,UINT message,UINT wParam,LONG lParam);
BOOL APIENTRY VideoSettingDigiProc(HWND hDlg,UINT message,UINT wParam,LONG lParam);
BOOL APIENTRY VTSettingProc(HWND hDlg,UINT message,UINT wParam,LONG lParam);
BOOL APIENTRY WinBisSettingProc(HWND hDlg,UINT message,UINT wParam,LONG lParam);
BOOL APIENTRY VDSettingProc(HWND hDlg,UINT message,UINT wParam,LONG lParam);
BOOL APIENTRY CryptModeProc(HWND hDlg,UINT message,UINT wParam,LONG lParam);
BOOL APIENTRY VTInfoProc(HWND hDlg,UINT message,UINT wParam,LONG lParam);
BOOL APIENTRY VPSInfoProc(HWND hDlg,UINT message,UINT wParam,LONG lParam);
BOOL APIENTRY WINBISInfoProc(HWND hDlg,UINT message,UINT wParam,LONG lParam);
BOOL APIENTRY VDInfoProc(HWND hDlg,UINT message,UINT wParam,LONG lParam);
BOOL APIENTRY VDInfoProcRaw(HWND hDlg,UINT message,UINT wParam,LONG lParam);
BOOL APIENTRY PDCProc(HWND hDlg,UINT message,UINT wParam,LONG lParam);
BOOL APIENTRY AudioSettingProc(HWND hDlg,UINT message,UINT wParam,LONG lParam);
BOOL APIENTRY AudioDigiSettingProc(HWND hDlg,UINT message,UINT wParam,LONG lParam);
BOOL APIENTRY RemoteControlSettingProc(HWND hDlg,UINT message,UINT wParam,LONG lParam);
BOOL APIENTRY SwitchSettingProc(HWND hDlg,UINT message,UINT wParam,LONG lParam);
BOOL APIENTRY AudioSettingProc1(HWND hDlg,UINT message,UINT wParam,LONG lParam);
BOOL APIENTRY ColorSettingProc(HWND hDlg,UINT message,UINT wParam,LONG lParam);
BOOL APIENTRY SecamSettingProc(HWND hDlg,UINT message,UINT wParam,LONG lParam);
BOOL APIENTRY AboutProc(HWND hDlg,UINT message,UINT wParam,LONG lParam);
BOOL APIENTRY AboutProc1(HWND hDlg,UINT message,UINT wParam,LONG lParam);
BOOL APIENTRY NagraKeySetProc(HWND hDlg,UINT message,UINT wParam,LONG lParam);
BOOL APIENTRY LNBSettingProc(HWND hDlg,UINT message,UINT wParam,LONG lParam);
BOOL APIENTRY LNBSettingProcCable(HWND hDlg,UINT message,UINT wParam,LONG lParam);
BOOL APIENTRY ListFeldSettingProc(HWND hDlg,UINT message,UINT wParam,LONG lParam);
BOOL APIENTRY RecorderSettingProc(HWND hDlg,UINT message,UINT wParam,LONG lParam);
BOOL APIENTRY TimerRecSettingProc(HWND hDlg,UINT message,UINT wParam,LONG lParam);
BOOL APIENTRY FilterListe(HWND hDlg,UINT message,UINT wParam,LONG lParam);

// DVB
BOOL Init_DVB_Memory(void);



/***************************************************************************
*                                                                          *
*     FUNCTION: Eval_Exception                                             *
*                                                                          *
*     PURPOSE: Trap all exceptions such as Memory Access Problems          *
*                                                                          *
*                                                                          *
****************************************************************************/

void cleanup_before_exit(void);

int Eval_Exception ( LPEXCEPTION_POINTERS pEP )
{
	int n_except;
	char buf[512];
	
	n_except = pEP->ExceptionRecord->ExceptionCode;
	
	if (n_except == EXCEPTION_ACCESS_VIOLATION) {
		
		sprintf(buf, "Access violation : address = 0x%08x, IP=0x%08x.\nNow exiting cleanly...",
			pEP->ExceptionRecord->ExceptionAddress,
			pEP->ContextRecord->Eip);
        Write_Log(buf);
		MessageBox(NULL, buf,
			"Access violation in Multidec", MB_OK);
		cleanup_before_exit();
		exit(1);
	}
	
	return EXCEPTION_CONTINUE_EXECUTION;
}




/****************************************************************************

  FUNCTION: WinMain(HANDLE, HANDLE, LPSTR, int)
  
    PURPOSE: calls initialization function, processes message loop
	
****************************************************************************/

int APIENTRY WinMain(
					 HINSTANCE hInstance,
					 HINSTANCE hPrevInstance,
					 LPSTR lpCmdLine,
					 int nCmdShow
					 )
{
    WNDCLASS        wc;
	HINSTANCE       statusbar;
    MSG             msg;
    HWND Load;
    int i;
    SYSTEMTIME SystemTime;   
	
	char Zeile[255];
	
	
	hInst=hInstance;
	emsizex=754;
	emsizey=521;
	emstartx=44;
	emstarty=82;
	
	Load=FindWindow((LPCTSTR)"Espresso",(LPCTSTR)"Multi-Decoder");
	if ( Load != NULL ) {
		SetFocus(Load);
		SetActiveWindow(Load);
		SetForegroundWindow(Load);
		return FALSE;
	}  
	SplashWnd=CreateDialog(hInst,"SPLASHBOX",NULL,SplashProc);
	SetWindowPos(SplashWnd,HWND_TOPMOST,10,10,20,20,SWP_NOMOVE|SWP_NOCOPYBITS|SWP_NOSIZE);
	
	// Default Inits //
	
	
	ShutdownZeit.H = 0xff;
	ShutdownZeit.M = 0xff;
	
	
	sprintf(Zeile,"MultiDec.FON");
	if (AddFontResource((LPCTSTR)Zeile)  == 0 ) {
		MessageBox(NULL, "MultiDec.FON kann nicht geladen werden", "!! Kleines Problem !!", MB_OK);
	}
	
	Current_Mode=0;
	CardType=0;
	Capture_Video=TRUE;
	Capture_VBI=TRUE;
	Decode_AutoDetect=FALSE;
	ColourFormat=3;
	VideoSource=1;
	AudioSource=4;
	TunerType=0;
	OPKEY[0].Name[0]=0x00;
	OPKEY[1].Name[0]=0x00;
	OPKEY[2].Name[0]=0x00;
	OPKEY[3].Name[0]=0x00;
	strcpy(OPKEY[0].Name,"KEY.TXT");
	strcpy(OPKEY[0].Extension,"idx");
	ActiveKey=0;
	strcpy(OPKEY[1].Name,"KEY_C.TXT");
	strcpy(OPKEY[1].Extension,"ic+");
	
	for ( i=0; i < MAXFAVORITEN; i++ ) { 
		Favoriten[i].ts_id = 0;
		Favoriten[i].SID_Id = 0;
	};
	
	
	OSD_Colors[0].R=0;OSD_Colors[0].G=0;OSD_Colors[0].B=100;OSD_Colors[0].Blend=255;
	OSD_Colors[1].R=50;OSD_Colors[1].G=50;OSD_Colors[1].B=200;OSD_Colors[1].Blend=255;
	OSD_Colors[2].R=255;OSD_Colors[2].G=255;OSD_Colors[2].B=0;OSD_Colors[2].Blend=255;
	OSD_Colors[3].R=255;OSD_Colors[3].G=255;OSD_Colors[3].B=255;OSD_Colors[3].Blend=255;
	
	ButtonList[0].FeldId=0; ButtonList[0].x=25;
	ButtonList[1].FeldId=1; ButtonList[1].x=30;
	ButtonList[2].FeldId=2; ButtonList[2].x=200;
	ButtonList[3].FeldId=3; ButtonList[3].x=100;
	ButtonList[4].FeldId=4; ButtonList[4].x=30;
	ButtonList[5].FeldId=5; ButtonList[5].x=80;
	ButtonList[6].FeldId=6; ButtonList[6].x=80;
	ButtonList[7].FeldId=8, ButtonList[7].x=80;
	ButtonList[8].FeldId=14;ButtonList[8].x=50;
	ButtonList[9].FeldId=-1;ButtonList[9].x=0;
	ButtonList[10].FeldId=-1;ButtonList[10].x=0;
	ButtonList[11].FeldId=-1;ButtonList[11].x=0;
	ButtonList[12].FeldId=-1;ButtonList[12].x=0;
	ButtonList[13].FeldId=-1;ButtonList[13].x=0;
	ButtonList[14].FeldId=-1;ButtonList[14].x=0;
	
	
	TVTYPE=0;
	Flip=1;
	OutDib=TRUE;
	WindowStyle=0;
	InitialHue=0x00;
	InitialBrightness=0x00;
	InitialContrast=0xd8;
	InitialSaturationU=0xfe;
	InitialSaturationV=0xb4;
	InitialVolume=1000;
	InitialSpecial=0;
	InitialLoudness=0;
	InitialBass=0;
	InitialTreble=0;
	MasterTestzeilen=16;
	ManuellAudio[0]=0;ManuellAudio[1]=0;ManuellAudio[2]=0;ManuellAudio[3]=0;
	ManuellAudio[4]=0;ManuellAudio[5]=0;ManuellAudio[6]=0;ManuellAudio[7]=0;
	
	
	LNB[0].Use=TRUE;
	LNB[0].MinFreq=10700;
	LNB[0].MaxFreq=12750;
	LNB[0].LofLow=9750;
	LNB[0].LofHigh=10600;
	LNB[0].SwitchFreq=11700;
	LNB[0].PowerReset=FALSE;
	LNB[0].Power=TRUE;
	LNB[0].Switch22khz=TRUE;
	LNB[0].BurstVal=-1;
	LNB[0].PosVal='A';
	LNB[0].OptVal='A';
	LNB[0].MiniDiseqc=0x00;
	
	LNB[1].Use=FALSE;
	LNB[1].MinFreq=10700;
	LNB[1].MaxFreq=12750;
	LNB[1].LofLow=9750;
	LNB[1].LofHigh=10600;
	LNB[1].SwitchFreq=11700;
	LNB[1].Power=TRUE;
	LNB[1].PowerReset=FALSE;
	LNB[1].Switch22khz=TRUE;
	LNB[1].BurstVal=-1;
	LNB[1].PosVal='B';
	LNB[1].OptVal='B';
	LNB[1].MiniDiseqc=0x00;
	
	LNB[2].Use=FALSE;
	LNB[2].MinFreq=10700;
	LNB[2].MaxFreq=12750;
	LNB[2].LofLow=9750;
	LNB[2].LofHigh=10600;
	LNB[2].SwitchFreq=11700;
	LNB[2].PowerReset=FALSE;
	LNB[2].Power=TRUE;
	LNB[2].Switch22khz=TRUE;
	LNB[2].BurstVal=-1;
	LNB[2].PosVal='A';
	LNB[2].OptVal='B';
	LNB[2].MiniDiseqc=0x00;
	
	LNB[3].Use=FALSE;
	LNB[3].MinFreq=10700;
	LNB[3].MaxFreq=12750;
	LNB[3].LofLow=9750;
	LNB[3].LofHigh=10600;
	LNB[3].PowerReset=FALSE;
	LNB[3].SwitchFreq=11700;
	LNB[3].Power=TRUE;
	LNB[3].Switch22khz=TRUE;
	LNB[3].BurstVal=-1;
	LNB[3].PosVal='B';
	LNB[3].OptVal='A';
	LNB[3].MiniDiseqc=0x00;
	
	
	Timing.WriteRegDelay=0xffffffff;
	Timing.I2C_Bus_Timeout=200;
	Timing.I2C_Bus_Reset_Delay=10;
	Timing.DVB_Reset_Wait=5;
	Timing.DVB_Reset_Scan_Wait=15;
	Timing.SendDiSEqCDelay=100;
	Timing.Pmt_TimeOut=2500;
	Timing.Sdt_TimeOut=2500;
	Timing.Pat_TimeOut=2500;
	Timing.Cat_TimeOut=2500;
	Timing.Nit_TimeOut=2500;
	Timing.Debi_Cmd_Time_Out=15000;
	Timing.OSD_Text_Time_Out=60000;
	Timing.Debi_done_MC2=2000;
	Timing.Debi_done_PSR=10000;
	
	
	
	for ( i=0; i<TIMERANZAHL; i++ ) {
		TimerRec[i].StartZeit_h=0xff;
		TimerRec[i].Started=FALSE;
	};
	
	strcpy(VDat.RawName,"VD-RAW.Dat");
	
	TZ[0]=16;
	TZ[1]=24;
	TZ[2]=32;
	TZ[3]=40;
	TZ[4]=48;
	
	External_Dll_Count=0;
	
	for ( i = 0; i < 5 ; i++ ) {
		Ext_Dll[i].Extern_Init=NULL;
		Ext_Dll[i].Extern_Exit=NULL;
		Ext_Dll[i].Extern_Channel_Change=NULL;
		Ext_Dll[i].Extern_Hot_Key=NULL;
		Ext_Dll[i].Extern_Menu_Cmd=NULL;
		Ext_Dll[i].Extern_Menu=NULL;
		Ext_Dll[i].Extern_Send_Dll_ID_Name=NULL;
		Ext_Dll[i].Extern_Stream_Function[0]=NULL;
		Ext_Dll[i].Extern_Stream_Function[1]=NULL;
		Ext_Dll[i].Extern_Stream_Function[2]=NULL;
		Ext_Dll[i].Extern_Stream_Function[3]=NULL;
		Ext_Dll[i].Extern_Stream_Function[4]=NULL;
		Ext_Dll[i].Extern_Stream_Function[5]=NULL;
		Ext_Dll[i].Extern_Stream_Function[6]=NULL;
		Ext_Dll[i].Extern_Stream_Function[7]=NULL;
		Ext_Dll[i].Extern_RecPlay=NULL;
		Ext_Dll[i].Extern_Filter_Close=NULL;
		Ext_Dll[i].Name[0]=0x00;
		Ext_Dll[i].HotKey=0x00;
	}
	
	for ( i = 0 ; i < 64 ; i++ ) MixerLoad[i].MixerAccess.SoundSystem = -1;
	
	VTColourTable[0]=RGB(0,0,0);     //Schwarz
	VTColourTable[1]=RGB(255,0,0); //Rot
	VTColourTable[2]=RGB(0,255,0);     //Grun
	VTColourTable[3]=RGB(255,255,0);  //Gelb
	VTColourTable[4]=RGB(0,0,255);     //Blau
	VTColourTable[5]=RGB(192,0,192);     // Unsichtbar
	VTColourTable[6]=RGB(92,164,207);       // HellBlau
	VTColourTable[7]=RGB(255,255,255);       ;     // Weis 
	VTColourTable[8]=RGB(254,254,254);       // Eigentich Transparent
	VTColourTable[9]=0;       // Eigentich Transparent
	VTColourTable[10]=0;       // Eigentich Transparent
	VTColourTable[11]=0;       // Eigentich Transparent
	VTColourTable[12]=0;       // Eigentich Transparent
	VTColourTable[13]=0;       // Eigentich Transparent
	VTColourTable[14]=0;       // Eigentich Transparent
	VTColourTable[15]=0;       // Eigentich Transparent
	RecordBaseDir[1][0]=0x00;
	RecordBaseDir[2][0]=0x00;
	RecordBaseDir[3][0]=0x00;
	RecordBaseDir[4][0]=0x00;
	RecordBaseDir[5][0]=0x00;
	RecordBaseDir[6][0]=0x00;
	RecordBaseDir[7][0]=0x00;
	
	
	GetCurrentDirectory(255,CurrentDir);
	sprintf(RecordBaseDir[0],"%s\\Recorder",CurrentDir);
	sprintf(FIRM_DIR,"%s\\Firmware\\Linux_9_3",CurrentDir);
	sprintf(WINIBIS_DIR,"%s\\Winbis",CurrentDir);
	sprintf(VD_DIR,"%s\\VideoDat",CurrentDir);
	sprintf(VT_BASE_DIR,"%s\\VideoTxt",CurrentDir);
	sprintf(VT_EXPORT_WWW_ROOT,"file://%s",VT_BASE_DIR);
	
	
	Load_INI("MultiDec.ini");
	GetLocalTime(&SystemTime);
	TDT.Date_Id=GetDateID(SystemTime.wYear,SystemTime.wMonth,SystemTime.wDay);
	TIMEZONE=TIMEZONE*-1;
	TDT.Hour=(unsigned char)RealTime((unsigned char)SystemTime.wHour);
	TIMEZONE=TIMEZONE*-1;
	TDT.Min=(unsigned char)SystemTime.wMinute;
	TDT.Sec=(unsigned char)SystemTime.wSecond;
	
	if ( strlen(RecordBaseDir[0]) > 0 ) i=(int)CreateDirectory(RecordBaseDir[0],NULL);
	if ( strlen(RecordBaseDir[1]) > 0 ) i=(int)CreateDirectory(RecordBaseDir[1],NULL);
	if ( strlen(RecordBaseDir[2]) > 0 ) i=(int)CreateDirectory(RecordBaseDir[2],NULL);
	if ( strlen(RecordBaseDir[3]) > 0 ) i=(int)CreateDirectory(RecordBaseDir[3],NULL);
	if ( strlen(RecordBaseDir[4]) > 0 ) i=(int)CreateDirectory(RecordBaseDir[4],NULL);
	if ( strlen(RecordBaseDir[5]) > 0 ) i=(int)CreateDirectory(RecordBaseDir[5],NULL);
	if ( strlen(RecordBaseDir[6]) > 0 ) i=(int)CreateDirectory(RecordBaseDir[6],NULL);
	if ( strlen(RecordBaseDir[7]) > 0 ) i=(int)CreateDirectory(RecordBaseDir[7],NULL);
	if ( strlen(WINIBIS_DIR) > 0 ) i=(int)CreateDirectory(WINIBIS_DIR,NULL);
	if ( strlen(VD_DIR) > 0 ) i=(int)CreateDirectory(VD_DIR,NULL);
	if ( strlen(VT_BASE_DIR) > 0 ) i=(int)CreateDirectory(VT_BASE_DIR,NULL);
	
	LogFile=NULL;
	if ( WRITE_LOG == TRUE ) {
		remove("MD-BOOT.txt");
		LogFile=fopen("MD-BOOT.txt","a");
	};
	
	if ( emsizex < 37 ) {
		emsizex=450;
		emstartx=10;
	}
	if ( emsizey < 37 ) {
		emsizey=350;
		emstarty=10;
	}
	if ( emstartx > GetSystemMetrics(SM_CXSCREEN) ) emstartx=10;
	if ( emstarty > GetSystemMetrics(SM_CYSCREEN) ) emstarty=10;
	if ( emstartx < 0 ) emstartx=10;
	if ( emstarty <0 ) emstarty=10;
	
	
	
    wc.style = 0;//CS_OWNDC;
    wc.lpfnWndProc = (WNDPROC) MainWndProc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = sizeof(LONG);
    wc.hInstance = hInstance;
    wc.hIcon = LoadIcon(hInstance, "Espresso");
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = CreateSolidBrush(0);  
	//    wc.lpszMenuName =  "DECODEMENU";
    wc.lpszMenuName =  NULL;
    wc.lpszClassName = "Espresso";
	
	if (!RegisterClass (&wc)) {
		return FALSE;
	}
	if ( Current_Mode == 0 ) {
		hMenu=LoadMenu(hInstance,"ANALOGMENU");
	} else {
		hMenu=LoadMenu(hInstance,"DIGITALMENU");
	}
    hWnd = CreateWindow(
        "Espresso",
        "Multi-Decoder",
        WS_OVERLAPPEDWINDOW,
        emstartx,
        emstarty,
        emsizex,
        emsizey,
        NULL,
        NULL,
        hInstance,
        NULL
		);
	
	
	if (!hWnd)
        return (FALSE);
	
	
	statusbar=hInst;
	if (!InitStatusBar (statusbar)) {
		return FALSE;
	}
	
	
    if (CreateStatusBar (hWnd, statusbar, ID_STATUSBAR)) {
        hwndTextField   = AddStatusField (statusbar, ID_TEXTFIELD, 110,0, FALSE);
        hwndAudioField   = AddStatusField (statusbar, ID_AUDIOFIELD, 110,0, FALSE);
		hwndPalField = AddStatusField (statusbar, ID_CODEFIELD, 110,0, FALSE);
        hwndKeyField   = AddStatusField (statusbar, ID_KENNUNGFFIELD, 90,50, FALSE);
        hwndFPSField   = AddStatusField (statusbar, ID_FPSFIELD, 45, 45, TRUE);
    } else {
        return FALSE;
    }						  
	AdjustStatusBar(hWnd);
	rectscreen.bottom -= 8;
	
	if ( Display_Status_Bar == FALSE ) {
		CheckMenuItem( GetMenu(hWnd), IDM_STATUSBAR, MF_UNCHECKED );
		ShowWindow(Status_Bar,SW_HIDE);
	} else
		CheckMenuItem( GetMenu(hWnd), IDM_STATUSBAR, MF_CHECKED );
	
	
	BirneRot = LoadBitmap(hInst,"BROT");
	BirneGruen =LoadBitmap(hInst,"BGRUEN");
	
	ShowWindow(hWnd,nCmdShow);
	PostMessage( hWnd, WM_SIZE, SIZENORMAL,MAKELONG( emsizex,emsizey ) );
	if ( Current_Mode == 0) {
		if (!(hAccel = LoadAccelerators (hInstance, "ANALOGACCEL")))
		{
			
			(void) MessageBox( GetFocus(), "Analog-Accel nicht geladen ", "Error", MB_OK);
		};
		
	} else  if ( Current_Mode == 1) {
		if (!(hAccel = LoadAccelerators (hInstance, "DIGITALACCEL")))
		{
			
			(void) MessageBox( GetFocus(), "Digital-Accel nicht geladen ", "Error", MB_OK);
		};
	};
	
	__try {
		while (GetMessage(&msg, NULL, 0, 0)) {
			if ( OSD_FULL_INPUT == TRUE ) {
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			} else {
				if ( !TranslateAccelerator(hWnd, hAccel, &msg) ) {
					TranslateMessage(&msg);
					DispatchMessage(&msg);
				}
			};
		}
	}
	
	__except( Eval_Exception( GetExceptionInformation( ))) {
		// No code; this block never executed.
	}
	return (msg.wParam);
	
}


void cleanup_before_exit() {
	
	char Text[255];
	int i;
	
	if ( CleanUP == TRUE ) return;
	CleanUP = TRUE ;
	
	KillTimer(hWnd,8);
	KillTimer(hWnd,22);
	KillTimer(hWnd,1);
	KillTimer(hWnd,99);
	
	sprintf(Text,"MultiDec.FON");
	if (RemoveFontResource((LPCTSTR)Text)  == 0 ) {
		
	}
	Stop_Thread();
	
	
	if ( Has_DVB == TRUE ) {
        Stop_Osd_Key_Event();
		if (( Play == TRUE ) || ( Record == TRUE ))
		{
			Debi_Record_Stop();
		};
		
		
		
		ShutDownFilters();
		if (( Record == TRUE ) || ( Play == TRUE )) Debi_Record_Stop();
		Sleep(2);Sleep(2);Sleep(2);Sleep(2);Sleep(2);Sleep(2);Sleep(2);Sleep(2);Sleep(2);Sleep(2);Sleep(2);Sleep(2);
		Free_EIT();
	}
	
	Write_INI("Multidec.ini");
	
	if ( Has_BT == TRUE ) {
        Stop_VBI();
		SetAudioSource(4);
	};
	
	Set_Capture(4);
	Sleep(100);
	ExitTV();  // TV-System runterfahren
	Sleep(100);
	Free_Nagra_Decoding();
	Exit_Mixer();
	VideoDat_Exit();
	Winbis_Exit();
	for ( i=0; i < 800 ; i++ ) {
		if ( VTFrame[i].SubPage != NULL  ) free(VTFrame[i].SubPage);
		VTFrame[i].SubPage=NULL;
		VTFrame[i].SubCount=0;
	};
	if ( Programm_OSD_Stop != NULL ) CloseHandle(Programm_OSD_Stop);
	if ( Decode_Event_Stop != NULL ) CloseHandle(Decode_Event_Stop);
	if ( pDecode != NULL ) {
		free(pDecode);
		pDecode=NULL;
	};
	if(hDIB != NULL)
	{
		GlobalUnlock(hDIB);
		GlobalFree(hDIB);
		hDIB = NULL;
		pDIB = NULL;
	}
	
	
	if ( MultiDec_Heap != NULL ) {
		if ( HeapDestroy(MultiDec_Heap) == 0 ) sprintf(Text,"MultiDec HeapDestroy return %d",GetLastError());
		else sprintf(Text,"MultiDec HeapDestroy ");
		Write_Log(Text); 
	}
	
	for ( i=0; i < 5; i++ ) Unload_External_Dll(i);
	if ( LogFile != NULL ) fclose(LogFile);
	
}



/****************************************************************************

  FUNCTION: MainWndProc(HWND, unsigned, WORD, LONG)
  
    PURPOSE:  Processes messages
	
	  MESSAGES:
	  
        WM_COMMAND    - application menu (About dialog box)
        WM_CREATE     - create window and objects
        WM_PAINT      - update window, draw objects
        WM_DESTROY    - destroy window
		
		  COMMENTS:
		  
			Handles to the objects you will use are obtained when the WM_CREATE
			message is received, and deleted when the WM_DESTROY message is
			received.  The actual drawing is done whenever a WM_PAINT message is
			received.
			
			  ****************************************************************************/
			  LONG APIENTRY MainWndProc(
				  HWND hWnd,
				  UINT message,
				  UINT wParam,
				  LONG lParam)
				  
			  {
				  
				  char Text[256]; 
				  char Text1[256];    
				  int i,j;
				  int Load_Try=0;
				  SYSTEMTIME SystemTime;   
				  
				  switch (message) {
				  case WM_COMMAND:
					  return(MainCommandDispatch(hWnd,message,wParam,lParam));
				  case WM_USER:
					  return(ExternCommandDispatch(hWnd,message,wParam,lParam));
					  
				  case WM_CREATE:
					  
					  Write_Log(" Logdatei von Multidec Version 8.2");
					  Write_Log("***********************************");
					  
					  GetSystemInfo(&SysInfo);
					  MultiDec_Heap=NULL;
					  MultiDec_Heap=HeapCreate(0,SysInfo.dwPageSize,0);
					  
					  SetDlgItemText(SplashWnd,IDC_TEXT1,"");
					  SetDlgItemText(SplashWnd,IDC_TEXT2,"");
					  SetDlgItemText(SplashWnd,IDC_TEXT3,"");
					  SetDlgItemText(SplashWnd,IDC_TEXT4,"");
					  SetDlgItemText(SplashWnd,IDC_TEXT5,"");
					  
					  Write_Log("Externe MDAPI DLL laden");
					  
					  Load_External_Dll();
					  
					  SetMenu(hWnd,hMenu);
					  
					  for (i=0; i < 12; i++ ) {
						  UTPages[i]=0;
					  };
					  SetDlgItemText(SplashWnd,IDC_TEXT3,"VideoText");
					  for ( i=0; i < 800 ; i++ ) {
						  VTFrame[i].SubPage=NULL;
						  VTFrame[i].SubCount=0;
					  };
					  
					  VT_ChannelChange();
					  
					  for ( i = 0; i < MAXVTDIALOG; i++ ) VTDialog[i].Dialog=NULL;
					  
					  Load_Programm_List();
					  Load_Nit_List();
					  Load_Country_Settings();
					  // DIB-Bitmap VideoText erzeugen
					  
					  SetDlgItemText(SplashWnd,IDC_TEXT4,"HQ-Farbe");
					  Init_HQTabelle();
					  Init_VC_HQTabelle();
					  CheckMenuItem( GetMenu(hWnd), ThreadClassId+1150, MF_CHECKED );
					  CheckMenuItem( GetMenu(hWnd), PriorClassId+1160, MF_CHECKED );
					  
					  
					  if ( USE_MIXER == TRUE ) {
						  SetDlgItemText(SplashWnd,IDC_TEXT1,"Sound-System");
						  SetDlgItemText(SplashWnd,IDC_TEXT2,"");
						  SetDlgItemText(SplashWnd,IDC_TEXT3,"");
						  SetDlgItemText(SplashWnd,IDC_TEXT4,"");
						  SetDlgItemText(SplashWnd,IDC_TEXT5,"");
						  
						  Enumerate_Sound_SubSystem();
						  if ( SoundSystem.DeviceAnzahl == 0 ) {
							  SetDlgItemText(SplashWnd,TEXT3,"Kein Soundsystem gefunden");
						  } else {
							  if ( SoundSystem.DeviceAnzahl >= 1 ) SetDlgItemText(SplashWnd,TEXT3,SoundSystem.MixerDev[0].szPname);
							  if ( SoundSystem.DeviceAnzahl >= 2 ) SetDlgItemText(SplashWnd,TEXT4,SoundSystem.MixerDev[1].szPname);
							  if ( SoundSystem.DeviceAnzahl >= 3 ) SetDlgItemText(SplashWnd,TEXT5,SoundSystem.MixerDev[2].szPname);
							  SetDlgItemText(SplashWnd,IDC_TEXT3,"");
							  SetDlgItemText(SplashWnd,IDC_TEXT4,"");
							  SetDlgItemText(SplashWnd,IDC_TEXT5,"");
							  
							  if ( Volume.SoundSystem >= 0 ) sprintf(Text,"%s",SoundSystem.MixerDev[Volume.SoundSystem].szPname);
							  else if ( Mute.SoundSystem >= 0 ) sprintf(Text,"%s",SoundSystem.MixerDev[Mute.SoundSystem].szPname);
							  else sprintf(Text,"%s",SoundSystem.MixerDev[0].szPname);
							  SetDlgItemText(SplashWnd,IDC_TEXT2,Text);
							  
							  if ( Volume.SoundSystem >= 0 ) sprintf(Text,"Volume -> %s",SoundSystem.To_Lines[Volume.SoundSystem].To_Connection[Volume.Destination].MixerConnections[Volume.Connection].szName);
							  else sprintf(Text,"Volume nicht gesetzt");
							  SetDlgItemText(SplashWnd,IDC_TEXT3,Text);
							  
							  if ( Mute.SoundSystem >= 0 ) sprintf(Text,"%s -> %s  %s",SoundSystem.To_Lines[Mute.SoundSystem].To_Connection[Mute.Destination].MixerConnections[Mute.Connection].szName,SoundSystem.To_Lines[Mute.SoundSystem].MixerLine[Mute.Destination].szName,SoundSystem.To_Lines[Mute.SoundSystem].To_Connection[Mute.Destination].To_Control[Mute.Connection].MixerControl[Mute.Control].szName);
							  else sprintf(Text,"Mute nicht gesetzt");
							  SetDlgItemText(SplashWnd,IDC_TEXT4,Text);
							  
							  
							  
							  if ( MIXER_LINKER_KANAL == -1 ) Mixer_Get_Volume(&MIXER_LINKER_KANAL,&MIXER_RECHTER_KANAL);
							  Mixer_Set_Defaults();
							  Get_Volume_Param();
							  Mixer_Set_Volume(MIXER_LINKER_KANAL,MIXER_RECHTER_KANAL);
						  };
					  }; 
					  SetDlgItemText(SplashWnd,IDC_TEXT1,"Systemanalyse");
					  SetDlgItemText(SplashWnd,IDC_TEXT2,"");
					  SetDlgItemText(SplashWnd,IDC_TEXT3,"");
					  SetDlgItemText(SplashWnd,IDC_TEXT4,"");
					  SetDlgItemText(SplashWnd,IDC_TEXT5,"");
					  
					  sprintf(Text,"Prozessor %d ",SysInfo.dwProcessorType);
					  SetDlgItemText(SplashWnd,IDC_TEXT2,Text);
					  sprintf(Text,"Anzahl %d ",SysInfo.dwNumberOfProcessors);
					  SetDlgItemText(SplashWnd,IDC_TEXT3,Text);
					  
					  if ( SysInfo.dwNumberOfProcessors > 1 ) {
						  if ( AusgabeProzessor == 0 ) {
							  if ( SysInfo.dwNumberOfProcessors == 2 ) {
								  MainProzessor=0;
								  IrqProzessor=0;
								  VBIProzessor=0;
								  AusgabeProzessor=1;
							  }
							  if ( SysInfo.dwNumberOfProcessors == 3 ) {
								  MainProzessor=0;
								  IrqProzessor=0;
								  VBIProzessor=1;
								  AusgabeProzessor=2;
							  }
							  if ( SysInfo.dwNumberOfProcessors > 3 ) {
								  MainProzessor=0;
								  IrqProzessor=1;
								  VBIProzessor=2;
								  AusgabeProzessor=3;
							  }
							  
						  }
						  
						  SetDlgItemText(SplashWnd,IDC_TEXT1,"Multi-Prozessor");
						  sprintf(Text,"Main-CPU %d ",MainProzessor);
						  SetDlgItemText(SplashWnd,IDC_TEXT2,Text);
						  sprintf(Text,"IRQ-CPU %d ",IrqProzessor);
						  SetDlgItemText(SplashWnd,IDC_TEXT3,Text);
						  sprintf(Text,"VBI-CPU %d ",VBIProzessor);
						  SetDlgItemText(SplashWnd,IDC_TEXT4,Text);
						  sprintf(Text,"DECODE-CPU %d ",AusgabeProzessor);
						  SetDlgItemText(SplashWnd,IDC_TEXT5,Text);
						  Sleep(100);
					  }
					  
					  ProzessorMask=1<<(MainProzessor);
					  i=SetThreadAffinityMask (GetCurrentThread(),ProzessorMask);         
					  
					  SetDlgItemText(SplashWnd,IDC_TEXT1,"Hardwareanalyse");
					  SetDlgItemText(SplashWnd,IDC_TEXT2,"");
					  SetDlgItemText(SplashWnd,IDC_TEXT3,"");
					  SetDlgItemText(SplashWnd,IDC_TEXT4,"");
					  SetDlgItemText(SplashWnd,IDC_TEXT5,"");
					  
					  if ( WindowStyle == 0 ) {
						  CheckMenuItem( GetMenu(hWnd), IDM_FENSTER, MF_CHECKED );
						  CheckMenuItem( GetMenu(hWnd), IDM_640x480, MF_UNCHECKED );
						  CheckMenuItem( GetMenu(hWnd), IDM_800x600, MF_UNCHECKED );
						  CheckMenuItem( GetMenu(hWnd), IDM_OWN_RES, MF_UNCHECKED );
						  
						  InitTV(hWnd,FALSE,768,576);
					  } else if  ( WindowStyle == 1 ) {
						  CheckMenuItem( GetMenu(hWnd), IDM_FENSTER, MF_UNCHECKED );
						  CheckMenuItem( GetMenu(hWnd), IDM_640x480, MF_CHECKED );
						  CheckMenuItem( GetMenu(hWnd), IDM_800x600, MF_UNCHECKED );
						  CheckMenuItem( GetMenu(hWnd), IDM_OWN_RES, MF_UNCHECKED );
						  
						  InitTV(hWnd,TRUE,640,480);
					  } else if	( WindowStyle == 2 ) {
						  CheckMenuItem( GetMenu(hWnd), IDM_FENSTER, MF_UNCHECKED );
						  CheckMenuItem( GetMenu(hWnd), IDM_640x480, MF_UNCHECKED );
						  CheckMenuItem( GetMenu(hWnd), IDM_800x600, MF_CHECKED );
						  CheckMenuItem( GetMenu(hWnd), IDM_OWN_RES, MF_UNCHECKED );
						  InitTV(hWnd,TRUE,800,600);
					  } else if	( WindowStyle == 3 ) {
						  CheckMenuItem( GetMenu(hWnd), IDM_FENSTER, MF_UNCHECKED );
						  CheckMenuItem( GetMenu(hWnd), IDM_640x480, MF_UNCHECKED );
						  CheckMenuItem( GetMenu(hWnd), IDM_800x600, MF_UNCHECKED );
						  CheckMenuItem( GetMenu(hWnd), IDM_OWN_RES, MF_CHECKED );
						  if (  InitTV(hWnd,TRUE,Res_X,Res_Y) == FALSE) {
							  WindowStyle=0;
							  CheckMenuItem( GetMenu(hWnd), IDM_FENSTER, MF_CHECKED );
							  CheckMenuItem( GetMenu(hWnd), IDM_640x480, MF_UNCHECKED );
							  CheckMenuItem( GetMenu(hWnd), IDM_800x600, MF_UNCHECKED );
							  CheckMenuItem( GetMenu(hWnd), IDM_OWN_RES, MF_UNCHECKED );
							  InitTV(hWnd,FALSE,640,480);
							  MoveWindow(hWnd,0,0,640,480,TRUE);
						  };
					  };
					  
					  
					  if ( WTVBPP == 0 ) EnableMenuItem(GetMenu(hWnd),IDM_INTERN_SURFACE,MF_GRAYED);
					  if ( Can_Overlay) {
						  EnableMenuItem(GetMenu(hWnd),IDM_OVERLAY,MF_ENABLED);
						  EnableMenuItem(GetMenu(hWnd),IDM_RAW_OVERLAY,MF_ENABLED);
						  if ( Can_Overlay_Strech == FALSE ) EnableMenuItem(GetMenu(hWnd),IDM_DX_OV_STRECH,MF_GRAYED);
						  else EnableMenuItem(GetMenu(hWnd),IDM_DX_OV_STRECH,MF_ENABLED);
					  } else {
						  EnableMenuItem(GetMenu(hWnd),IDM_OVERLAY,MF_GRAYED);
						  EnableMenuItem(GetMenu(hWnd),IDM_DX_OV_STRECH,MF_GRAYED);
						  EnableMenuItem(GetMenu(hWnd),IDM_RAW_OVERLAY,MF_GRAYED);
					  }
					  if ( bFilterBlueLines == TRUE )CheckMenuItem( GetMenu(hWnd), IDM_NOBLUELINES, MF_CHECKED );
					  if ( USE_DX_OV_STRECH == TRUE ) CheckMenuItem( GetMenu(hWnd), IDM_DX_OV_STRECH, MF_CHECKED );
					  if ( WAIT_DX_VBI == TRUE )  CheckMenuItem( GetMenu(hWnd), IDM_DX_AUSTAST, MF_CHECKED );
					  if ( USE_DX_LOCK == TRUE ) CheckMenuItem( GetMenu(hWnd), IDM_DX_LOCK, MF_CHECKED );
					  if ( ColorMix == TRUE ) CheckMenuItem( GetMenu(hWnd), IDM_COLORMIX, MF_CHECKED ); 
					  if ( HalfTVMode == TRUE ) CheckMenuItem( GetMenu(hWnd), IDM_HALFMODE, MF_CHECKED ); 
					  if ( Decode_AutoDetect == TRUE ) CheckMenuItem( GetMenu(hWnd), IDM_AUTODETECT, MF_CHECKED ); 
					  if (VBI_Flags&VBI_VT) {
						  CheckMenuItem( GetMenu(hWnd), IDM_VBI_VT, MF_CHECKED );
						  if ( Capture_VBI == TRUE ) {
							  EnableMenuItem(GetMenu(hWnd),IDM_CALL_VIDEOTEXT,MF_ENABLED);
							  EnableMenuItem(GetMenu(hWnd),IDM_VT_RESET,MF_ENABLED);
							  EnableMenuItem(GetMenu(hWnd),IDM_UNTERTITEL,MF_ENABLED);
							  EnableMenuItem(GetMenu(hWnd),IDM_PDC_OUT,MF_ENABLED);
							  EnableMenuItem(GetMenu(hWnd),IDM_VT_OUT,MF_ENABLED);
						  }
					  }
					  if (VBI_Flags&VBI_WINBIS) {
						  CheckMenuItem( GetMenu(hWnd), IDM_VBI_WINBIS, MF_CHECKED );
						  if ( Capture_VBI == TRUE ) {
							  EnableMenuItem(GetMenu(hWnd),IDM_WINBIS_OUT,MF_ENABLED);
						  }
					  }
					  
					  if (VBI_Flags&VBI_VC) {
						  CheckMenuItem( GetMenu(hWnd), IDM_VBI_VC, MF_CHECKED );
						  if ( Capture_VBI == TRUE ) {
							  EnableMenuItem(GetMenu(hWnd),IDM_CRYPTSTATUS,MF_ENABLED);
						  }
					  }
					  if (VBI_Flags&VBI_VPS) {
						  CheckMenuItem( GetMenu(hWnd), IDM_VBI_VPS, MF_CHECKED );
						  if ( Capture_VBI == TRUE ) {
							  EnableMenuItem(GetMenu(hWnd),IDM_VPS_OUT,MF_ENABLED);
						  }
					  }				   
					  
					  if ( Auto_MovieBoost == TRUE ) CheckMenuItem( GetMenu(hWnd), IDM_AUTOMB, MF_CHECKED ); 
					  
					  if ( USECARD == FALSE ) CheckMenuItem( GetMenu(hWnd), IDM_DONT_TOUCH_CARD, MF_CHECKED ); 
					  
					  if ( USETUNER == FALSE ) {
						  TunerType=4;
						  VideoSource=0;
						  CheckMenuItem( GetMenu(hWnd), IDM_DONT_TOUCH, MF_CHECKED );
						  EnableMenuItem(GetMenu(hWnd),IDM_TUNER_0,MF_GRAYED);
						  EnableMenuItem(GetMenu(hWnd),IDM_TUNER_1,MF_GRAYED);
						  EnableMenuItem(GetMenu(hWnd),IDM_TUNER_2,MF_GRAYED);
						  EnableMenuItem(GetMenu(hWnd),IDM_TUNER_3,MF_GRAYED);
						  EnableMenuItem(GetMenu(hWnd),IDM_TUNER_5,MF_GRAYED);
						  EnableMenuItem(GetMenu(hWnd),IDM_TUNER_6,MF_GRAYED);
						  EnableMenuItem(GetMenu(hWnd),IDM_TUNER_7,MF_GRAYED);
						  EnableMenuItem(GetMenu(hWnd),IDM_TUNER_8,MF_GRAYED);
						  EnableMenuItem(GetMenu(hWnd),IDM_CHANNELPLUS,MF_GRAYED);
						  EnableMenuItem(GetMenu(hWnd),IDM_CHANNELMINUS,MF_GRAYED);
						  EnableMenuItem(GetMenu(hWnd),IDM_ANALOGSCAN,MF_GRAYED);
						  EnableMenuItem(GetMenu(hWnd),IDM_MANUELL_TUNER,MF_GRAYED);
					  };
					  
					  CheckMenuItem( GetMenu(hWnd), CardType+1080, MF_CHECKED );
					  CheckMenuItem( GetMenu(hWnd), AudioSource+1110, MF_CHECKED );
					  S_TVTYPE=TVTYPE;
					  CheckMenuItem( GetMenu(hWnd), TVTYPE+1120, MF_CHECKED );
					  S_ColourFormat= ColourFormat;
					  CheckMenuItem( GetMenu(hWnd), ColourFormat+1130, MF_CHECKED );
					  if ( VideoSource > 0 ) CheckMenuItem( GetMenu(hWnd), VideoSource+1089, MF_CHECKED );
					  else {
						  CheckMenuItem( GetMenu(hWnd), TunerType+1100, MF_CHECKED );
					  }
					  CheckMenuItem( GetMenu(hWnd), IDM_RAW_SURFACE, MF_UNCHECKED );
					  CheckMenuItem( GetMenu(hWnd), IDM_RAW_OVERLAY, MF_UNCHECKED );
					  CheckMenuItem( GetMenu(hWnd), IDM_INTERN_SURFACE, MF_UNCHECKED );
					  CheckMenuItem( GetMenu(hWnd), IDM_OVERLAY, MF_UNCHECKED );
					  CheckMenuItem( GetMenu(hWnd), IDM_DIB, MF_UNCHECKED );
					  
					  
					  
					  OldMode=IDM_DIB;
					  if ( ModeRAWSurface==TRUE ) {
						  Flip=0;
						  OldMode=IDM_RAW_SURFACE;
						  ModeOverlay=FALSE;
						  ModeRAWSurface=TRUE;
						  CheckMenuItem( GetMenu(hWnd), IDM_RAW_SURFACE, MF_CHECKED );
						  OutDib=FALSE;
					  } else if ( ModeRAWOverlay==TRUE ) {
						  Flip=0;
						  OldMode=IDM_RAW_OVERLAY;
						  ModeRAWOverlay=TRUE;
						  ModeRAWSurface=FALSE;
						  CheckMenuItem( GetMenu(hWnd), IDM_RAW_OVERLAY, MF_CHECKED );
						  OutDib=FALSE;
					  } else if ( OutDib==TRUE ) {
						  Flip=1;
						  CheckMenuItem( GetMenu(hWnd), IDM_DIB, MF_CHECKED );
					  } else if ( ModeOverlay == TRUE ) {
						  OldMode=IDM_OVERLAY;
						  Flip=0;
						  CheckMenuItem( GetMenu(hWnd), IDM_OVERLAY, MF_CHECKED );
					  } else {
						  OldMode=IDM_INTERN_SURFACE;
						  Flip=0;
						  CheckMenuItem( GetMenu(hWnd), IDM_INTERN_SURFACE, MF_CHECKED );
					  }
					  
					  SetTimer (hWnd, 10, 5000, NULL);
					  
					  PostMessage(hWnd,INIT_HARDWARE,0,0);
					  break;
					  
					  
	  case INIT_HARDWARE:
		  
		  MSPStatus[0]=0x00;
          HardWareFound=0;
          CurrentProgramm=InitialProg;
		  LastProgramm=InitialProg;
          
		  
		  
		  i=Start_MDHal();
		  
          if ( i != 0  ) {
			  ShowWindow(SplashWnd,SW_HIDE);
			  if ( i == -1 ) {
				  MessageBox(hWnd,"MultiDec-HAL-Treiber MDHALL kann nicht installiert werden !\n\nMögliche Lösung\n\n - Als Administrator anmelden\n - \\\\HKEY_LOCAL_MACHINE\\SYSTEM\\CurrentControlSet\\Windriver löschen\n - Neu Booten","Multi-Decoder",MB_ICONSTOP|MB_OK);
				  Keep_Driver_Loaded=FALSE;
				  UnloadDeviceDriver("MultiDec BT-Treiber",TRUE);
				  UnloadDeviceDriver("MultiDec SAA-Treiber",TRUE);
				  UnloadDeviceDriver("MultiDec HAL-Treiber",TRUE);
			  } else if ( i == -2 ) {
				  MessageBox(hWnd,"MultiDec-HAL-Treiber MDHALDR.VXD kann nicht installiert werden !\n\nMögliche Lösung\n\n - Als Administrator anmelden\n - \\\\HKEY_LOCAL_MACHINE\\SYSTEM\\CurrentControlSet\\Windriver löschen \n- \\\\HKEY_LOCAL_MACHINE\\SYSTEM\\CurrentControlSet\\MultiDec-HAL-Treiber löschen\n - Neu Booten","Multi-Decoder",MB_ICONSTOP|MB_OK);
				  Keep_Driver_Loaded=FALSE;
				  UnloadDeviceDriver("MultiDec HAL-Treiber",TRUE);
			  } else if ( i == -3 ) {
				  MessageBox(hWnd,"MultiDec-HAL-Treiber MDHALDR kann nicht registriert werden !\n\nMögliche Lösung\n\n - Windriver deinstallieren\n - \\\\HKEY_LOCAL_MACHINE\\SYSTEM\\CurrentControlSet\\Windriver löschen \n- \\\\HKEY_LOCAL_MACHINE\\SYSTEM\\CurrentControlSet\\MultiDec-HAL-Treiber löschen\n - Neu Booten","Multi-Decoder",MB_ICONSTOP|MB_OK);
				  Keep_Driver_Loaded=FALSE;
				  UnloadDeviceDriver("MultiDec HAL-Treiber",TRUE);
			  };
			  if ( LogFile != NULL ) fclose(LogFile);
			  ExitThread(9);
			  return(TRUE); 
		  };
		  
		  i=PCI_Scan() ;
		  if ( i != 0 ) {
			  if ( i == -1 ) {
				  MessageBox(hWnd,"MultiDec-HAL-Treiber MDHALL kann nicht geöffnet werden !\n\nMögliche Lösung\n\n - Als Administrator anmelden\n - \\\\HKEY_LOCAL_MACHINE\\SYSTEM\\CurrentControlSet\\Windriver löschen\n - Neu Booten","Multi-Decoder",MB_ICONSTOP|MB_OK);
			  } else if ( i== -2 ) {
				  MessageBox(hWnd,"MultiDec-HAL-Treiber MDHALL kann nicht angesprochen werden!","Multi-Decoder",MB_ICONSTOP|MB_OK);
			  };
			  Keep_Driver_Loaded = FALSE;
			  UnloadDeviceDriver("WinDriver",TRUE);
			  UnloadDeviceDriver("MultiDec BT-Treiber",TRUE);
			  UnloadDeviceDriver("MultiDec SAA-Treiber",TRUE);
			  UnloadDeviceDriver("MultiDec HAL-Treiber",TRUE);
			  if ( LogFile != NULL ) fclose(LogFile);
			  ExitThread(9);
			  return(TRUE); 
		  }
		  
		  if ( Check_WD_Version() == FALSE ) {
			  Keep_Driver_Loaded = FALSE;
			  UnloadDeviceDriver("WinDriver",TRUE);
			  UnloadDeviceDriver("MultiDec HAL-Treiber",TRUE);
			  UnloadDeviceDriver("MultiDec BT-Treiber",TRUE);
			  UnloadDeviceDriver("MultiDec SAA-Treiber",TRUE);
			  if ( Load_Try == 0 ) PostMessage(hWnd,INIT_HARDWARE,0,0);
			  else {
				  MessageBox(hWnd,"MultiDec-HAL-Treiber : Falsche Version!\n\nMögliche Lösung\n\n - Als Administrator anmelden\n - \\\\HKEY_LOCAL_MACHINE\\SYSTEM\\CurrentControlSet\\MultiDec-HAL-Treiber löschen\n - Neu Booten","Multi-Decoder",MB_ICONSTOP|MB_OK);
				  if ( LogFile != NULL ) fclose(LogFile);
				  ExitThread(9);
				  return(TRUE); 
			  }
			  Load_Try++;
			  return(TRUE);
		  }
		  if ( Init_TV_Karte(hWnd) == TRUE ) HardWareFound++;
		  if ( Init_DVB_Karte(hWnd) == TRUE ) HardWareFound++;
		  
		  Init_Kernel_Memory();
		  
		  if ( Has_BT == TRUE ) {
              Sleep(5);
			  if ( Init_Memory() == FALSE ) {
				  BT_Close();
				  Has_BT = FALSE;
				  HardWareFound--;
				  Has_BT=FALSE;
			  };
		  };
		  
		  
		  WStyle=GetWindowLong(hWnd,GWL_EXSTYLE);
		  if ( ALLWAYS_TOP == FALSE ) {
			  WStyle=WStyle^8;
			  i=SetWindowLong(hWnd,GWL_EXSTYLE,WStyle);
			  SetWindowPos(hWnd,HWND_NOTOPMOST,10,10,20,20,SWP_NOMOVE|SWP_NOCOPYBITS|SWP_NOSIZE);
			  CheckMenuItem( GetMenu(hWnd), IDM_ON_TOP, MF_UNCHECKED );
		  } else {
			  WStyle=WStyle|8;
			  i=SetWindowLong(hWnd,GWL_EXSTYLE,WStyle);
			  CheckMenuItem( GetMenu(hWnd), IDM_ON_TOP, MF_CHECKED );
			  SetWindowPos(hWnd,HWND_TOPMOST,10,10,20,20,SWP_NOMOVE|SWP_NOCOPYBITS|SWP_NOSIZE);
		  }
		  
		  dwDIB = sizeof(pDIB->bmiHeader);
		  hDIB = GlobalAlloc(GMEM_ZEROINIT | GMEM_MOVEABLE, dwDIB);
		  if(hDIB == NULL) {
			  MessageBox(hWnd,"Couldn't GlobalAlloc DIB-Screen Memory","Multi-Decode",MB_ICONSTOP|MB_OK);
			  return(FALSE);
		  }
		  
		  pDIB = (BITMAPINFO*)GlobalLock(hDIB);
		  if(pDIB == NULL)
		  {
			  MessageBox(hWnd,"Couldn't Lock DIB-Screen  Memory","Multi-Decode",MB_ICONSTOP|MB_OK);
			  GlobalFree(hDIB);
			  hDIB = NULL;
			  return(FALSE);
		  }	
		  
		  
		  if ( Has_BT == TRUE ) {							  
			  Init_BT_HardWare(hWnd);
			  sprintf(TunerStatus,"Kein Device am I2C-Bus");
			  if ( USETUNER == TRUE) {
				  Sleep(5);
				  sprintf(Text,"Kein Tuner");
				  if ( Init_Tuner(TunerType) == TRUE ) {
					  sprintf(Text,"Tuner OK");
					  
					  
				  }
				  SetDlgItemText(SplashWnd,IDC_TEXT4,Text);
				  
			  } else {
				  sprintf(TunerStatus,"Tuner soll nicht initialsiert werden");
				  sprintf(Text,"Tuner deakt.");
				  SetDlgItemText(SplashWnd,IDC_TEXT4,Text);
			  }
			  
			  Sleep(5);
			  if ( MSPStatus[0] == 0x00 ) {
				  sprintf(MSPStatus,"Kein Device am I2C-Bus");
				  sprintf(Text,"Kein MSP-Device");
				  if (  Init_Audio(0x80,0x81) == TRUE ) {
					  sprintf(Text,"MSP-Device OK");
					  sprintf(MSPStatus,"MSP-Device I2C-Bus I/O 0x80/0x81");
				  } 
			  } else {
				  sprintf(Text,"MSP-Device OK");
			  }
			  SetDlgItemText(SplashWnd,IDC_TEXT5,Text);
			  
			  if ( Has_MSP == TRUE ) {
				  SetTimer(hWnd,8,10000,NULL);
				  if ( AutoStereoSelect == TRUE ) {
					  CheckMenuItem( GetMenu(hWnd), IDM_AUTOSTEREO, MF_CHECKED );
				  };
			  };
			  
			  if ( VideoSource == 1 ) sprintf(Text,"Extern 1");
			  else if ( VideoSource == 2 ) sprintf(Text,"Extern 2");
			  else if ( VideoSource == 3 ) sprintf(Text,"Extern 3");
			  else {
				  if (( CurrentProgramm >=0 ) && ( CurrentProgramm < MAXPROGS ))
					  sprintf(Text,"%04d. %s ",Make_ProgNr(CurrentProgramm)+1,Programm[CurrentProgramm].Name);
			  }; 
			  
			  if ( Display_Status_Bar == TRUE ) SetWindowText (hwndKeyField,Text);
		  };
		  
		  
          if ( Has_DVB == TRUE ) {
			  SetDlgItemText(SplashWnd,IDC_TEXT1,"DVB-Hardware");
			  SetDlgItemText(SplashWnd,IDC_TEXT2,DVBTyp);
			  SetDlgItemText(SplashWnd,IDC_TEXT3,"");
			  SetDlgItemText(SplashWnd,IDC_TEXT4,"");
			  SetDlgItemText(SplashWnd,IDC_TEXT5,"");
			  
			  
			  
			  if ( Has_DVB==TRUE ) {
				  if ( Init_DVB_HardWare() == TRUE ) {
					  Init_Key_Table();  // DVB-Fernbedienung
					  GetDlgItemText(SplashWnd,IDC_TEXT2,Text1,64);
					  SetDlgItemText(SplashWnd,IDC_TEXT1,Text1);
					  GetDlgItemText(SplashWnd,IDC_TEXT3,Text1,64);
					  SetDlgItemText(SplashWnd,IDC_TEXT2,Text1);
					  GetDlgItemText(SplashWnd,IDC_TEXT4,Text1,64);
					  SetDlgItemText(SplashWnd,IDC_TEXT3,Text1);
					  GetDlgItemText(SplashWnd,IDC_TEXT5,Text1,64);
					  SetDlgItemText(SplashWnd,IDC_TEXT4,Text1);
					  if ( Is_Sat_Karte() == TRUE ) SetDlgItemText(SplashWnd,IDC_TEXT5,"DVB-S OK");
					  else SetDlgItemText(SplashWnd,IDC_TEXT5,"DVB-C OK");
				  } else {
					  GetDlgItemText(SplashWnd,IDC_TEXT2,Text1,64);
					  SetDlgItemText(SplashWnd,IDC_TEXT1,Text1);
					  GetDlgItemText(SplashWnd,IDC_TEXT3,Text1,64);
					  SetDlgItemText(SplashWnd,IDC_TEXT2,Text1);
					  GetDlgItemText(SplashWnd,IDC_TEXT4,Text1,64);
					  SetDlgItemText(SplashWnd,IDC_TEXT3,Text1);
					  GetDlgItemText(SplashWnd,IDC_TEXT5,Text1,64);
					  SetDlgItemText(SplashWnd,IDC_TEXT4,Text1);
					  SetDlgItemText(SplashWnd,IDC_TEXT5,"DVB FAIL");
					  Sleep(1000);
					  DVB_Close();
					  Has_DVB=FALSE;
					  HardWareFound--;
				  }
			  };
		  };
		  
		  
		  if ( HardWareFound == 0 ) {
			  
			  SetDlgItemText(SplashWnd,IDC_TEXT1,"");
			  SetDlgItemText(SplashWnd,IDC_TEXT2,"Keine");
			  SetDlgItemText(SplashWnd,IDC_TEXT3,"passende");
			  SetDlgItemText(SplashWnd,IDC_TEXT4,"Hardware");
			  SetDlgItemText(SplashWnd,IDC_TEXT5,"");
			  ExitTV();
			  Sleep(5000);
			  if ( LogFile != NULL ) fclose(LogFile);
			  ExitThread(9);
			  
		  }
		  
		  
		  if (!Alloc_Surface_DMA()) {
			  MessageBox(hWnd,"Surface Memory können für DMA nicht gelockt werden !! Surface deaktiviert !!","Multi-Decode",MB_ICONSTOP|MB_OK);
			  ModeOverlay=FALSE;
			  ModeRAWSurface=FALSE;
			  CheckMenuItem( GetMenu(hWnd), IDM_RAW_SURFACE, MF_CHECKED );
			  OutDib=TRUE;
			  Flip=1;
			  WTVBPP = 0;
			  CheckMenuItem( GetMenu(hWnd), IDM_RAW_SURFACE, MF_UNCHECKED );
			  CheckMenuItem( GetMenu(hWnd), IDM_RAW_OVERLAY, MF_UNCHECKED );
			  CheckMenuItem( GetMenu(hWnd), IDM_INTERN_SURFACE, MF_UNCHECKED );
			  CheckMenuItem( GetMenu(hWnd), IDM_OVERLAY, MF_UNCHECKED );
			  CheckMenuItem( GetMenu(hWnd), IDM_DIB, MF_UNCHECKED );
			  CheckMenuItem( GetMenu(hWnd), IDM_DIB, MF_CHECKED );
			  EnableMenuItem(GetMenu(hWnd),IDM_INTERN_SURFACE,MF_GRAYED);
			  EnableMenuItem(GetMenu(hWnd),IDM_RAW_SURFACE,MF_GRAYED);
			  
		  }
		  
          if ( Show_Menu == FALSE ) {
			  Show_Menu=TRUE;
			  SendMessage(hWnd,WM_COMMAND,IDM_TOGGLE_MENU,0);
		  }		
		  
		  
		  // 31.07.2001
		  // Modifikation: oxident
		  // Sinn und Zweck: DVB-c Karten erfordern anscheinend immer erst eine Art "Tuner-Initialisierung"
		  // Dank an zimuland für den Tip!
		  
		  
		  
		  if (Has_DVB == TRUE) {
			  TunerStatusByte=Get_Tuner_Status();
		  };
		  
		  if (( Current_Mode == 1 ) && ( Has_DVB == FALSE )) Current_Mode=0;
		  if (( Current_Mode == 0 ) && ( Has_BT == FALSE )) Current_Mode=1;
		  
		  if (( Current_Mode == 1 ) && ( Has_DVB == TRUE )) {
			  Run_BT_Irq(FALSE);
			  Stop_Thread();
			  DestroyMenu(hMenu);
			  hMenu=LoadMenu(hInst,"DIGITALMENU");
			  
			  
			  
			  if ( Show_Menu == TRUE ) SetMenu(hWnd,hMenu);
			  
			  DestroyAcceleratorTable(hAccel);
			  hAccel = LoadAccelerators (hInst, "DIGITALACCEL");
			  SetMenuDigital();
			  DOSIZE=TRUE;
			  pDisplay[0]=DVB_Display_dma.pUserAddr;
			  Init_Screen_Struct();
			  Run_DVB_Irq(TRUE,TRUE);
           		 if ( CurrentProgramm >= 0 ) {
					 if ((Programm[CurrentProgramm].Typ == 'D' ) && ( Has_DVB == TRUE )) Set_DVB_Channel(CurrentProgramm);
				 };
				 DVB_SetVolume((unsigned char)InitialDVBVolumeLinks, (unsigned char)InitialDVBVolumeRechts);
				 Set_Capture(5);
		  } else if (( Current_Mode == 0 ) && ( Has_BT == TRUE )) {
			  Run_DVB_Irq(FALSE,TRUE);
			  Stop_Thread();
			  DestroyMenu(hMenu);
			  hMenu=LoadMenu(hInst,"ANALOGMENU");
			  if ( Show_Menu == TRUE ) SetMenu(hWnd,hMenu);
			  DestroyAcceleratorTable(hAccel);
			  hAccel = LoadAccelerators (hInst, "ANALOGACCEL");
			  SetMenuAnalog();
			  DOSIZE=TRUE;
			  for ( i=0; i < 5 ; i++ ) {
				  pDisplay[i]=Display_dma[i]->pUserAddr;
			  }
			  
			  Init_Screen_Struct();
			  Run_BT_Irq(TRUE);
			  if ( CurrentProgramm >= 0 ) {
				  if ((Programm[CurrentProgramm].Typ == 'A' ) && ( Has_BT == TRUE )) Tuner_SetFrequency(TunerType,MulDiv(Programm[CurrentProgramm].freq*1000,16,1000000));
			  };
			  
			  Set_Capture(5);
			  SetAudioSource(AudioSource);
			  Audio_SetVolume(InitialVolume);
		  };
		  
		  
		  SetTimer(hWnd,22,1000,NULL);
		  SetTimer (hWnd, 1, 2500, NULL);
		  
		  
		  return(TRUE);
	  case WM_LBUTTONUP:
		  
		  if ( MoveXDist == -1 ) {
			  PostMessage(hWnd,WM_COMMAND,IDM_TOGGLE_MENU,0);
		  }
		  MoveXDist=-1;
		  MoveYDist=-1;
		  
		  break;
		  
      case WM_MOUSEMOVE:
		  if ( wParam == MK_LBUTTON ) 
		  {
			  
			  if (( MoveXDist==-1 ) && ( MoveYDist==-1)) { 
				  GetWindowRect(hWnd,&rectscreen);
				  MoveXDist = LOWORD(lParam);  // horizontal position of cursor 
				  MoveYDist = HIWORD(lParam);  
				  emsizex=rectscreen.right-rectscreen.left;
				  emsizey=rectscreen.bottom-rectscreen.top;
				  
			  } else {
				  
				  MoveWindow(hWnd,rectscreen.left-(MoveXDist-LOWORD(lParam)),rectscreen.top-(MoveYDist-HIWORD(lParam)),emsizex,emsizey,TRUE);
			  }
		  }
		  break;
		  
		  
      case WM_MENUSELECT:
		  
		  if ( HIWORD(wParam) == 0xffff ) {
			  Restore_Mode();
			  break; 
		  }
		  
		  if ( HIWORD(wParam) & MF_POPUP  ) {
			  Save_Mode();
		  }
		  break;
		  
	  case WM_RBUTTONUP:
		  CShowCursor=!CShowCursor;
		  i=ShowCursor(CShowCursor);
		  break;
		  
	  case WM_TIMER:
		  
		  if ( wParam == 22 ) {
              if (( TDT.Hour == ShutdownZeit.H ) &&
				  ( TDT.Min == ShutdownZeit.M )) {
				  ShutdownZeit.M = 0xff;
				  ShutdownZeit.H = 0xff;
                  sprintf(ShutDownText,"Achtung !\nZeitgesteuertes Runterfahren des Systems !\nEs ist jetzt %02d:%02d Uhr.",RealTime(TDT.Hour),TDT.Min);
				  CurrentShutDownFlag=ShutDownValue;
				  Save_Mode();
				  DialogBox(hInst,"SHUTDOWN",NULL,Stop_SystemProc);
                  Restore_Mode();
			  };
			  
              TDT.Sec++;
			  if ( TDT.Sec > 59 ) {
				  TDT.Sec = 0;
				  TDT.Min++;
				  GetLocalTime(&SystemTime);
				  TDT.Date_Id=GetDateID(SystemTime.wYear,SystemTime.wMonth,SystemTime.wDay);
				  if ( TDT.Min > 59 ) {
					  TDT.Hour++;
					  TDT.Min=0;
					  if ( TDT.Hour > 23 ) TDT.Hour=0;
				  };
			  };
			  return(TRUE);
		  }
		  
		  
		  
		  if (( Has_BT == TRUE ) && ( Current_Mode==0 ) && ( CurrentCapture != 4 )) {
			  ResetIrq++;
			  if (( CurrentCapture == 4) && ( Capture_VBI == FALSE ) && (( ModeRAWSurface == TRUE ) || ( ModeRAWOverlay == TRUE ))) ResetIrq=0;
			  if (( ResetIrq > 5 ) && (( Capture_Video == TRUE ) || (Capture_VBI == TRUE ))) {
				  Reset_BT_HardWare();
				  ResetIrq=0;
			  } 
			  
			  if ( wParam == 8 ) {
				  if ( Display_Status_Bar == TRUE ) MSP_Print_Mode();
				  if ( AutoStereoSelect == TRUE ) MSPWatch_Mode();
				  return(TRUE);
			  }
		  } else ResetIrq=0;
		  
		  if  ( wParam == 10 ) {
			  if ( DecodeNagra == FALSE) {
				  if (( AutoMovieBoostEnable==TRUE ) && ( MovieBoost != 0 )) {
					  MoreLinesLock=FALSE;
					  Stop_Thread();
					  MovieBoost = 0;
					  AutoMovieBoostEnable=FALSE;
					  Start_Thread();
				  }
			  } else {
				  if (Auto_MovieBoost==TRUE) {
					  i=G_Wert(MatrixLinesEven,MatrixLinesOdd);
					  if ((i > 6 ) && ( MovieBoost != i) && (MoreLinesLock == TRUE )) {
						  
						  Stop_Thread();
						  MovieBoost=i;
						  AutoMovieBoostEnable=TRUE;
						  Start_Thread();
					  }
				  };
			  };
          };
		  
		  if ( wParam == 1 ) {
			  
			  if ( Has_DVB == TRUE ) {
				  for ( i=0; i < TIMERANZAHL ; i++ ) {
					  if (TimerRec[i].StartZeit_h != 0xff ) {
						  
						  if (TimerRec[i].Started ==FALSE  ) {
							  if (( TimerRec[i].Date_Id == TDT.Date_Id ) &&
								  ( TimerRec[i].StartZeit_h == RealTime(TDT.Hour) ) &&
								  ( TimerRec[i].StartZeit_m == TDT.Min )) {
								  TimerRec[i].Started = TRUE;
								  Debi_Record_Stop();

								switch(TimerRec[i].RecorderArt) {
                                  case 0:
                                   RecorderFormat=0;
                                   RecorderTyp=0;
                                   break;
                                  case 1:
                                   RecorderFormat=1;
                                   RecorderTyp=0;
                                   break;
                                  case 2:
                                   RecorderFormat=2;
                                   RecorderTyp=0;
                                   break;
                                  case 3:
                                   RecorderFormat=0;
                                   RecorderTyp=1;
                                   break;
                                  case 4:
                                   RecorderFormat=1;
                                   RecorderTyp=1;
                                   break;
                                  case 5:
                                   RecorderFormat=2;
                                   RecorderTyp=1;
                                   break;
                                }

								  if (( Programm[TimerRec[i].Programm].freq != 0  ) && (ValidModes(Programm[TimerRec[i].Programm].Typ) == TRUE  )) {
									  CurrentProgramm=TimerRec[i].Programm;
									  if (( Current_Mode == 0 ) && ( Programm[CurrentProgramm].Typ == 'D' )) Set_Mode_Digital();
									  else if (( Current_Mode == 1 ) && ( Programm[CurrentProgramm].Typ == 'A' )) Set_Mode_Analog();
									  if (Programm[CurrentProgramm].Typ == 'A' ) Tuner_SetFrequency(TunerType,MulDiv(Programm[CurrentProgramm].freq*1000,16,1000000));
									  else Set_DVB_Channel(CurrentProgramm);
				                                  if ( Display_Status_Bar == TRUE ) {
													  sprintf(Text,"%04d. %s ",Make_ProgNr(CurrentProgramm)+1,Programm[CurrentProgramm].Name);
													  SetWindowText (hwndKeyField,Text);
												  } // Display 
								  } // Valid for Record 
								  Debi_Record_Start(RecorderFormat,RecorderTyp);
							  } // StartZeit Erreicht 
						  } else {    // Laufende Aufnahme 
							  
							  if (( TimerRec[i].EndZeit_h == RealTime(TDT.Hour)) &&
								  ( TimerRec[i].EndZeit_m == TDT.Min )) {
								  TimerRec[i].Started = FALSE;
								  Debi_Record_Stop();
								  TimerRec[i].StartZeit_h=0xff;
								  if ( TimerRec[i].Shutdown == TRUE ) {
									  sprintf(ShutDownText,"Achtung !\nZeitgesteuertes Runterfahren des Systems nach Aufnahme !\nEs ist jetzt %02d:%02d Uhr.",RealTime(TDT.Hour),TDT.Min);
									  CurrentShutDownFlag=ShutDownValue;
									  Save_Mode();
									  DialogBox(hInst,"SHUTDOWN",NULL,Stop_SystemProc);
									  Restore_Mode();
									  
								  }
							  }
						  };
						  
					  };
					  
				  }			  
			  };
			  
			  
			  
			  if (( Has_BT == TRUE ) && ( Current_Mode==0 )) {	 
				  if (!VideoPresent() ) {
					  SetWindowText(hwndTextField," Kein Videosignal gefunden");
					  return(TRUE);
				  };
			  };
			  if (( Has_DVB == TRUE ) && ( Current_Mode==1 )) {	 
				  
				  if (( PROGRAMM_OSD == FALSE ) && (PROGRAMM_OSD_RUN == FALSE )) { 
				  if ( System_in_Sync > 1 ) {
					  if ( DVBVideoPresent() < 8) {
					  System_in_Sync++;
					  if ( System_in_Sync > 2 ) {
						   Create_Osd_Thread((LPTHREAD_START_ROUTINE)OSD_No_Sig);
					  }
					  };
					  return(TRUE);
				  };
				  };
                  System_in_Sync++;

				 SetWindowText(hwndTextField,Programm[CurrentProgramm].Name);
				  
				  if ( Record == TRUE ) {
					  if (( PROGRAMM_OSD == FALSE ) && (PROGRAMM_OSD_RUN == FALSE )) Create_Osd_Thread((LPTHREAD_START_ROUTINE)OSD_Rec);
					  
				  } else if ( System_In_Mute == TRUE ) {
					  sprintf(Text1,"Volume Mute");
					  SetWindowText(hwndTextField,Text1);
					  if (( PROGRAMM_OSD == FALSE ) && (PROGRAMM_OSD_RUN == FALSE )) Create_Osd_Thread((LPTHREAD_START_ROUTINE)OSD_Mute);
				  };
				  sprintf(Text,"V-Pid %04x  A-Pid %04x",Programm[CurrentProgramm].Video_pid,Programm[CurrentProgramm].Audio_pid);
				  SetWindowText(hwndPalField,Text);
			  };
			  
			  if ( Current_Mode == 0 ) {
				  Text[0]=0x00;
				  if ( Packet30.Identifier[0] != 0x00 ) {
					  sprintf(Text,"%s ",Packet30.Identifier);
					  Packet30.Identifier[0]=0x00;
				  } else if ( VPS_lastname[0] != 0x00 ) {
					  sprintf(Text,"%s ",VPS_lastname);
					  VPS_lastname[0] = 0x00; 
				  }
				  
				  strcpy(Text1,Text);
				  
				  if ( DecodeNagra == TRUE ) {
					  
					  if ( NagraFullField == TRUE) {
						  strcat(Text," FF ");
						  
					  }else {
						  strcat(Text," HF ");
						  
					  };
					  
					  sprintf(Text1,"%s %dTZ",Text,MasterTestzeilen);
				  }
				  
				  if ( System_In_Mute == TRUE ) sprintf(Text1,"Volume Mute");
				  SetWindowText(hwndTextField,Text1);
				  Text[0]=0x00;
				  PalFormat=( nPALplusData & FORMAT_MASK );
				  
				  switch(PalFormat)
				  {
				  case FORMAT_14_9:
					  strcpy(Text,"PalPlus 14:9");
					  break;
					  
				  case FORMAT_16_9:
					  strcpy(Text,"PalPlus 16:9");
					  break;
				  case FORMAT_4_3:
					  strcpy(Text,"PalPlus 4:3");
					  break;
				  }
				  
				  if ( DecodeNagra == FALSE ) {
					  if ( Decode_AutoDetect == TRUE ) {
						  strcat(Text," Dek. automatisch ");
					  }else {
						  strcat(Text," Dek. manuell ");
						  
					  };
					  if ( HalfTVMode == TRUE ) strcat(Text," Halbbild");
				  }
				  
				  
				  
				  if ( DecodeDecoderVCrypt == TRUE ) {
					  if ( MoreLinesLock == FALSE ) strcat(Text," VC-Decoder AutoSync");
					  else strcat(Text," VC-Decoder Irq-Lock");
				  }
				  
				  
				  if ( DecodeNagra == TRUE ) {
					  if ( FirstWidthEven == TESTPIXEL ) {
						  if (Line0BlackEven) strcat(Text,"E0");
					  } else {
						  if (Line0BlackEven) {
							  sprintf(Text1,"+HE(%d)",FirstWidthEven);
							  strcat(Text,Text1);
						  }
					  }
					  if ( MatrixModeEven == TRUE )  {
						  sprintf(Text1,"+ME %d",MatrixLinesEven);
						  strcat(Text,Text1);
					  } else { 
						  if (Line1BlackEven) strcat(Text,"+E1");
						  if (Line285BlackEven) strcat(Text,"+E285");
						  if (Line286BlackEven) strcat(Text,"+E286");
					  }
					  
					  if ( FirstWidthOdd == TESTPIXEL ) {
						  if (Line0BlackOdd) strcat(Text,"+O0");
					  } else {
						  if (Line0BlackOdd) {
							  sprintf(Text1,"+HO(%d)",FirstWidthOdd);
							  strcat(Text,Text1);
						  }
					  };
					  if ( MatrixModeOdd == TRUE )  {
						  sprintf(Text1,"+MO %d",MatrixLinesOdd);
						  strcat(Text,Text1);
					  } else {
						  if (Line1BlackOdd) strcat(Text,"+O1");
						  if (Line285BlackOdd) strcat(Text,"+O285");
						  if (Line286BlackOdd) strcat(Text,"+O286");
					  }
					  
					  
					  if ( MoreLinesLock == FALSE ) {
						  if ( NagraFullField == TRUE ) sprintf(Text1,"(Stab) (%d/%d)",FirstWidthEven,FirstWidthOdd);
						  else sprintf(Text1,"(Stab) (%d)",FirstWidthEven);
						  strcat(Text,Text1);
					  } else strcat(Text,"  (Lock)");
				  };
				  
				  SetWindowText(hwndPalField,Text);
};


return(TRUE);
		  };
		  
		  if ( wParam == 99 ) {
			  KillTimer(hWnd,99);
              if ( ChannelString[0] == 0x00 ) {
				  return(TRUE);
			  };
			  i=atoi(ChannelString);
			  ChannelString[0]=0x00;
			  if ( i != 0 ) {
				  j= Get_Real_ProgNr(i-1);
				  i=j;
			  } else {
				  i=LastProgramm ;
			  };
			  
			  if (( Has_BT == TRUE ) && ( Current_Mode==0 )) SetAudioSource(4);
			  if (( Has_DVB == TRUE ) && ( Current_Mode == 1 )) DVB_SetVolume((unsigned char)0, (unsigned char)0);
			  
			  if (( i >= 0 ) && ( i < MAXPROGS ))  {
				  LastProgramm = CurrentProgramm;
				  if ( LastProgramm >= MAXPROGS ) LastProgramm=LastRealProg;

				  if (( Programm[i].freq != 0  ) && (ValidModes(Programm[i].Typ) == TRUE  )) {
					  CurrentProgramm=i;
					  if (( Current_Mode == 0 ) && ( Programm[CurrentProgramm].Typ == 'D' )) Set_Mode_Digital();
					  else if (( Current_Mode == 1 ) && ( Programm[CurrentProgramm].Typ == 'A' )) Set_Mode_Analog();
					  if (Programm[CurrentProgramm].Typ == 'A' ) Tuner_SetFrequency(TunerType,MulDiv(Programm[CurrentProgramm].freq*1000,16,1000000));
					  else Set_DVB_Channel(CurrentProgramm);
					  if ( Display_Status_Bar == TRUE ) {
						  sprintf(Text,"%04d. %s ",Make_ProgNr(CurrentProgramm)+1,Programm[CurrentProgramm].Name);
						  SetWindowText (hwndKeyField,Text);
					  }
					  
				  }
				  
				  VT_ChannelChange();
				  if (( Has_BT == TRUE ) && ( Current_Mode==0 )) SetAudioSource(AudioSource);
				  if (( Has_DVB == TRUE ) && ( Current_Mode == 1 )) DVB_SetVolume((unsigned char)InitialDVBVolumeLinks, (unsigned char)InitialDVBVolumeRechts);
			  }
			  
			  
			  
		  };
		  return(TRUE);
		  
        case WM_SYSCOMMAND:  //            Verhindern vom ScreenSaver 
			switch(wParam&0xFFF0)
			{
			case SC_SCREENSAVE:
			case SC_MONITORPOWER:
				return 0;
				break;
			}
			
			case WM_SIZE:
			case WM_MOVE:
				
				GetClientRect(hWnd, &rectscreen);
				ClientToScreen(hWnd, (POINT*)&rectscreen.left);
				ClientToScreen(hWnd, (POINT*)&rectscreen.right);
				
				if ( (LastRect.top != rectscreen.top ) ||
					(LastRect.bottom != rectscreen.bottom ) ||
					(LastRect.left != rectscreen.left ) ||
					(LastRect.right != rectscreen.right ))  {
					
					AdjustStatusBar (hWnd);
					if ( DOSIZE == TRUE ) {
						
						Stop_Thread();
						if ( CurrentCapture != 0 ) {
							Set_Capture(4);
							Init_Screen_Struct();
							Set_Capture(5);
						} else  Init_Screen_Struct();
						LastRect.top=rectscreen.top;
						LastRect.bottom=rectscreen.bottom;
						LastRect.left=rectscreen.left;
						LastRect.right=rectscreen.right;
					};
				};			  
				
				break;
				
			case WM_KEYUP:
				if ( OSD_FULL_INPUT == TRUE ) {
					i=HIWORD(lParam);
					if ( i == 0xc14d ) i=1;// Cursor Rechts
					if ( i == 0xc14b ) i=2; // Cursor links
					if ( i == 0xc150 ) i=3; // Cursor runter
					if ( i == 0xc148 ) i=4; // Cursor rauf
					if ( i == 0xc153 ) i=5; // Entf
                    if (( i >= 1 ) && ( i <= 5 )) {
						OSD_Key_Value=i;
						SetEvent(OSD_KEY_EVENT);
						return(TRUE);
					};
					return(FALSE);
				};
				return(FALSE);
				
			case WM_CHAR:
				if ( PROGRAMM_OSD == FALSE ) { 
					if ( ((char) wParam  >= '0' ) && ((char) wParam  <= '9' )) {
						KillTimer(hWnd,99);
						sprintf(Text,"%c",wParam);
						strcat(ChannelString, Text); 
						if ( strlen(ChannelString) > 4 ) {
							ChannelString[0]=0x00;
							strcat(ChannelString, Text); 
						};	  
						if ( Current_Mode == 1 ) Update_OSD_Channel_Change();
						SetTimer(hWnd,99,1000,NULL);
					}
				} else {
					
					if ( OSD_FULL_INPUT == TRUE ) {
						OSD_Key_Value=wParam;
						SetEvent(OSD_KEY_EVENT);
						break;
					};
					if ( wParam == '8' ) {
						OSD_Key_Value='8';
						SetEvent(OSD_KEY_EVENT);
					}
					if ( wParam == '9' ) {
						OSD_Key_Value='9';
						SetEvent(OSD_KEY_EVENT);
					}
					
					if ( wParam == '2' ) {
						OSD_Key_Value='2';
						SetEvent(OSD_KEY_EVENT);
					}
					if ( wParam == '3' ) {
						OSD_Key_Value='3';
						SetEvent(OSD_KEY_EVENT);
					}
					
					if ( wParam == '4' ) {
						OSD_Key_Value='4';
						SetEvent(OSD_KEY_EVENT);
					}
					if ( wParam == '6' ) {
						OSD_Key_Value='6';
						SetEvent(OSD_KEY_EVENT);
					}
					if ( wParam == '7' ) {
						OSD_Key_Value='7';
						SetEvent(OSD_KEY_EVENT);
					}
					if ( wParam == '1' ) {
						OSD_Key_Value='1';
						SetEvent(OSD_KEY_EVENT);
					}
					
					if ( wParam == '0' ) {
						OSD_Key_Value='0';
						SetEvent(OSD_KEY_EVENT);
					}
					if ( wParam == '5' ) {
						OSD_Key_Value='5';
						SetEvent(OSD_KEY_EVENT);
					}
					
					
				};
				for ( i = 0 ; i < 5 ; i++ ) {
					if ( (Upcase((unsigned char)wParam) == Upcase(Ext_Dll[i].HotKey )) && ( Ext_Dll[i].Extern_Hot_Key != NULL ))  {
						sprintf(Text,"HotKey (%C) für DLL %s gesendet",Upcase(Ext_Dll[i].HotKey),Ext_Dll[i].Name);
						Write_Log(Text);
						if ( Ext_Dll[i].Extern_Hot_Key != NULL ) (Ext_Dll[i].Extern_Hot_Key)();
					};
				};
				
				break;
				
			case WM_QUERYENDSESSION:
			case WM_DESTROY:
				cleanup_before_exit();
				
				if ( System_Shutdown == TRUE ) {
					ExitWindowsEx(CurrentShutDownFlag,0);
				};
				PostQuitMessage(0);
				return(TRUE);
				
	}
	
	return(DefWindowProc(hWnd,message,wParam,lParam));
}


char Upcase(char Zeichen)
{
	switch(Zeichen) {
	case 'a' : return('A');
	case 'b' : return('B');
	case 'c' : return('C');
	case 'd' : return('D');
	case 'e' : return('E');
	case 'f' : return('F');
	case 'g' : return('G');
	case 'h' : return('H');
	case 'i' : return('I');
	case 'j' : return('J');
	case 'k' : return('K');
	case 'l' : return('L');
	case 'm' : return('M');
	case 'n' : return('N');
	case 'o' : return('O');
	case 'p' : return('P');
	case 'q' : return('Q');
	case 'r' : return('R');
	case 's' : return('S');
	case 't' : return('T');
	case 'u' : return('U');
	case 'v' : return('V');
	case 'w' : return('W');
	case 'x' : return('X');
	case 'y' : return('Y');
	case 'z' : return('Z');
	};
	return(Zeichen);
};

BOOL MainCommandDispatch(HWND hWnd,UINT message,UINT wParam,LONG lParam)
{
	char Text[256]; 
	int i,j,k;
	BOOL SBTOK;
	int Load_Try=0;

	
	switch( LOWORD( wParam ))
	{
		
	case IDM_INTERRUPT:
		Save_Mode();
		DialogBox(hInst,"RECORDPLAYINTERRUPT",hWnd,RecordPlayInterrupt);
		Restore_Mode();
		return(TRUE);
		
		
	case IDM_SYNC_IRQ:


		if ( Sync_Irq == FALSE ) {
			Sync_Irq=TRUE;
			CheckMenuItem( GetMenu(hWnd), IDM_SYNC_IRQ, MF_CHECKED );
		} else {
			Sync_Irq=FALSE;
			CheckMenuItem( GetMenu(hWnd), IDM_SYNC_IRQ, MF_UNCHECKED );
		}
		return(TRUE);
		
	case IDM_OSD_KANAL_PARAMETER:
		Create_Osd_Thread((LPTHREAD_START_ROUTINE)OSD_ChannelInfo);
		return(TRUE);
		
	case IDM_OSD_STARTZEIT_LISTE:
		Create_Osd_Thread((LPTHREAD_START_ROUTINE)OSD_Startzeit_Liste);
		return(TRUE);
		
	case IDM_OSD_EXTRA_KANAL:
		Create_Osd_Thread((LPTHREAD_START_ROUTINE)OSD_Extra_Kanal_Liste);
		return(TRUE);
		
	case IDM_OSD_PROGINFO:
		Create_Osd_Thread((LPTHREAD_START_ROUTINE)OSD_ProgInfo);
		return(TRUE);
		
	case IDM_OSD_CAM:
		Create_Osd_Thread((LPTHREAD_START_ROUTINE)OSD_CAM_Menu);
		return(TRUE);
		
		
	case IDM_CI_MESSAGEPOPUP:
		Create_Osd_Thread((LPTHREAD_START_ROUTINE)OSD_CAM_MessagePopUp);
		return(TRUE);
		
	case IDM_OSD_EIT_INFO:
		if ( Capture_Event_EIT == FALSE) return(TRUE);
		Create_Osd_Thread((LPTHREAD_START_ROUTINE)OSD_Eit_Info);
		return(TRUE);
		
	case IDM_OSD_RECORDER:
		Create_Osd_Thread((LPTHREAD_START_ROUTINE)OSD_Recorder_Info);
		return(TRUE);
		
	case IDM_OSD_AUDIO_LISTE:
		Create_Osd_Thread((LPTHREAD_START_ROUTINE)OSD_Audio_Liste);
		return(TRUE);
		
	case IDM_OSD_TEXT_LISTE:
		Create_Osd_Thread((LPTHREAD_START_ROUTINE)OSD_Text_Liste);
		return(TRUE);
		
	case IDM_OSD_PROGRAMM_LISTE:
						  Create_Osd_Thread((LPTHREAD_START_ROUTINE)OSD_Eit_ProgrammListe);
                          return(TRUE);
						  
	case IDM_OSD_UHR:
						  Create_Osd_Thread((LPTHREAD_START_ROUTINE)OSD_Uhr);
                          return(TRUE);
						  
	case IDM_OSD_EIT_LISTE_CURRENT_PROGRAMM:
		if ( Capture_Event_EIT == FALSE) return(TRUE);
						  Create_Osd_Thread((LPTHREAD_START_ROUTINE)OSD_Eit_Liste_Current_Programm);
                          return(TRUE);
						  
	case IDM_OSD_EIT_LISTE_CURRENT_NEXT:
		if ( Capture_Event_EIT == FALSE) return(TRUE);
						  Create_Osd_Thread((LPTHREAD_START_ROUTINE)OSD_Eit_Liste_Current_Next);
                          return(TRUE);
						  
	case IDM_OSD_MENU_LIST:
						  Create_Osd_Thread((LPTHREAD_START_ROUTINE)OSD_MenuListe);
						  return(TRUE);
						  
						  
	case IDM_TEXTLISTE:
		Save_Mode();
						  DialogBox(hInst,"TEXTPID",hWnd,TextPidSettingProc);
                          Restore_Mode();
						  return(TRUE);
						  
	case IDM_TIMELISTE:
		Save_Mode();
						  DialogBox(hInst,"STARTZEITEN",hWnd,StartZeitSettingProc);
                          Restore_Mode();
						  return(TRUE);
						  
	case IDM_VIEWLISTE:
		Save_Mode();
						  DialogBox(hInst,"MULTIVIEW",hWnd,MultiViewSettingProc);
                          Restore_Mode();
						  return(TRUE);
						  
	case IDM_DVB_AUDIO_TI:
		DVB_AUDIO=0;
		CheckMenuItem( GetMenu(hWnd), IDM_DVB_AUDIO_CRYSTAL, MF_UNCHECKED );
		CheckMenuItem( GetMenu(hWnd), IDM_DVB_AUDIO_TI, MF_CHECKED );
						  DVB_SetVolume((unsigned char)InitialDVBVolumeLinks, (unsigned char)InitialDVBVolumeRechts);
						  return(TRUE);
						  
	case IDM_DVB_AUDIO_CRYSTAL:
		DVB_AUDIO=1;
		CheckMenuItem( GetMenu(hWnd), IDM_DVB_AUDIO_CRYSTAL, MF_CHECKED );
		CheckMenuItem( GetMenu(hWnd), IDM_DVB_AUDIO_TI, MF_UNCHECKED );
						  Init_Crystal_Dac();
						  DVB_SetVolume((unsigned char)InitialDVBVolumeLinks, (unsigned char)InitialDVBVolumeRechts);
						  return(TRUE);
						  
	case IDM_AUDIOLISTE:
		Save_Mode();
						  DialogBox(hInst,"AUDIOPID",hWnd,AudioPidSettingProc);
                          Restore_Mode();
						  return(TRUE);
						  
	case IDM_OSD_SETTINGS:
		Save_Mode();
						  DialogBox(hInst,"OSDEINSTELLUNG",hWnd,OSDSettingProc);
                          Restore_Mode();
						  return(TRUE);
						  
	case IDM_SRATES:
		Save_Mode();
						  DialogBox(hInst,"SRATES",hWnd,SRateSettingProc);
                          Restore_Mode();
						  return(TRUE);
	case IDM_FILTERSCAN:
		Save_Mode();
						  DialogBox(hInst,"FILTERSCAN",hWnd,ByteFilterScanProc);
                          Restore_Mode();
						  return(TRUE);
	case IDM_FILTERSCAN1:
		Save_Mode();
						  DialogBox(hInst,"FILTERSCAN1",hWnd,StreamFilterScanProc);
                          Restore_Mode();
						  return(TRUE);
	case IDM_SATSCANN:
		Save_Mode();
						  if ( Is_Sat_Karte() == TRUE ) DialogBox(hInst,"SATSCAN",hWnd,SATScanProc);
						  else if ( Is_Kabel_Karte() == TRUE ) DialogBox(hInst,"CABLESCAN",hWnd,CableScanProc);
						  Restore_Mode();
						  return(TRUE);
	case IDM_SATSCANNNIT:
		Save_Mode();
						  if ( Is_Sat_Karte() == TRUE )  DialogBox(hInst,"SATSCANNIT",hWnd,SATScanNitProc);
						  else if ( Is_Kabel_Karte() == TRUE ) DialogBox(hInst,"CABLESCANNIT",hWnd,CableScanNitProc);
						  Restore_Mode();
						  return(TRUE);
						  
	case IDM_TPSCAN:
		Save_Mode();
						  if ( Is_Sat_Karte() == TRUE )  DialogBox(hInst,"TRANSPONDER",hWnd,TransponderSettingProc);
						  else if ( Is_Kabel_Karte() == TRUE ) DialogBox(hInst,"TRANSPONDERCABLE",hWnd,TransponderSettingProcCable);
						  Restore_Mode();
						  return(TRUE);
						  
	case IDM_BOOT_OPTION:
		Save_Mode();
						  DialogBox(hInst,"SAACARDSET",hWnd,SaaBootOptionProc);
                          Restore_Mode();
						  return(TRUE);
						  
	case IDM_DRIVER_OPTION:
		Save_Mode();
						  DialogBox(hInst,"BOOTSET",hWnd,MDBootOptionProc);
                          Restore_Mode();
						  return(TRUE);
						  
	case IDM_SHUTDOWN_OPTION:
		Save_Mode();
						  DialogBox(hInst,"SHUTDOWNSET",hWnd,MDShutdownOptionProc);
                          Restore_Mode();
						  return(TRUE);
						  
	case IDM_SHUTDOWN_TIME:
		Save_Mode();
						  DialogBox(hInst,"SHUTDOWNTIME",hWnd,MDShutdownSetProc);
                          Restore_Mode();
						  return(TRUE);
						  
	case IDM_SWAPMODE:
		if (( Current_Mode == 0 ) && ( Has_DVB == TRUE )) {
			Set_Mode_Digital();
		} else if (( Current_Mode == 1 ) && ( Has_BT == TRUE )) {
			Set_Mode_Analog();
		};
		return(TRUE);
		
	case IDM_DEKODERSOUND:
		if ( useSound == FALSE ) {
			useSound=TRUE;
			CheckMenuItem( GetMenu(hWnd), IDM_DEKODERSOUND, MF_CHECKED );
			SoundCapture_Open();
			SoundCapture_Play() ;
			sprintf(Text,"Audio Dekoder - An ");
			if(Display_Status_Bar == TRUE ) SetWindowText (hwndTextField,Text);
		} else {
			useSound=FALSE;
			CheckMenuItem( GetMenu(hWnd), IDM_DEKODERSOUND, MF_UNCHECKED );
			SoundCapture_Close();
			SoundCapture_Stop();
			sprintf(Text,"Audio Dekoder - Aus ");
			if(Display_Status_Bar == TRUE ) SetWindowText (hwndTextField,Text);
		}
		return(TRUE);
		
		// Fantasio
	case IDM_NOBLUELINES:
		if ( bFilterBlueLines == FALSE ) {
			bFilterBlueLines = TRUE;
			CheckMenuItem( GetMenu(hWnd), IDM_NOBLUELINES, MF_CHECKED );
			sprintf(Text,"BlauFilter an (Filtre Secam Lignes violettes Activé)");
		} else {
			bFilterBlueLines = FALSE;
			CheckMenuItem( GetMenu(hWnd), IDM_NOBLUELINES, MF_UNCHECKED );
			sprintf(Text,"Blaufilter aus (Filtre Secam Lignes violettes Désactivé)");
		}
		if ( Display_Status_Bar == TRUE ) SetWindowText (hwndTextField,Text);
		return(TRUE);
		
	case IDM_UPLUS:
		if (++iUFilterThresholdPCent >=100)
			iUFilterThresholdPCent=100;
		sprintf(Text,"Threshold U Filter %d %%",iUFilterThresholdPCent);
		if ( Display_Status_Bar == TRUE ) SetWindowText (hwndTextField,Text);
		return(TRUE);
		
	case IDM_UMINUS:
		if (--iUFilterThresholdPCent <=1)
			iUFilterThresholdPCent=1;
		sprintf(Text,"Threshold U Filter %d %%",iUFilterThresholdPCent);
		if ( Display_Status_Bar == TRUE ) SetWindowText (hwndTextField,Text);
		return(TRUE);
		
	case IDM_VPLUS:
		if (++iVFilterThresholdPCent >=100)
			iVFilterThresholdPCent=100;
		sprintf(Text,"Threshold V Filter %d %%",iVFilterThresholdPCent);
		if ( Display_Status_Bar == TRUE ) SetWindowText (hwndTextField,Text);
		return(TRUE);
		
	case IDM_VMINUS:
		if (--iVFilterThresholdPCent <=1)
			iVFilterThresholdPCent=1;
		sprintf(Text,"Threshold V Filter %d %%",iVFilterThresholdPCent);
		if ( Display_Status_Bar == TRUE ) SetWindowText (hwndTextField,Text);
		return(TRUE);
		
	case IDM_CLOSE_VT:
		for ( i = 0; i < MAXVTDIALOG; i++ ) {
			if ( VTDialog[i].Dialog != NULL ) SendMessage(VTDialog[i].Dialog,WM_COMMAND,IDCANCEL,0);
		};
		SetFocus(hWnd);
		return(TRUE);
		
		
		
	case IDM_VT_PAGE_MINUS:
		for ( i = 0; i < MAXVTDIALOG; i++ ) {
			if ( VTDialog[i].Dialog != NULL ) {
				j=Get_Dialog_Slot(VTDialog[i].Dialog);
				if ( VTDialog[j].Page > 100 ) {
					SetDlgItemInt(VTDialog[j].Dialog,IDC_EDIT1,VTDialog[j].Page-1 ,FALSE);
				}
			};
		};
		return(TRUE);
		
	case IDM_VT_PAGE_PLUS:
		for ( i = 0; i < MAXVTDIALOG; i++ ) {
			if ( VTDialog[i].Dialog != NULL ) {
				j=Get_Dialog_Slot(VTDialog[i].Dialog);
				if ( VTDialog[j].Page < 899 ) {
					SetDlgItemInt(VTDialog[j].Dialog,IDC_EDIT1,VTDialog[j].Page+1 ,FALSE);
				}
			};
		};
		return(TRUE);
		
		
	case IDM_VT_PAGE_UP:
		for ( i = 0; i < MAXVTDIALOG; i++ ) {
			if ( VTDialog[i].Dialog != NULL ) {
				j=Get_Dialog_Slot(VTDialog[i].Dialog);
				k=VTDialog[j].SubPage;
				if ( VTFrame[VTDialog[j].FramePos].SubCount == 0 ) return(TRUE);
				k--;
				while (( k >= 0 ) && ( VTFrame[VTDialog[j].FramePos].SubPage[k].Fill == FALSE )) k--;
				if ( k < 0 ) k=VTFrame[VTDialog[j].FramePos].SubCount-1; //DF:i
				while (( k >= 0 ) && ( VTFrame[VTDialog[j].FramePos].SubPage[k].Fill == FALSE )) k--;
				if (k<0) return(TRUE);
				
				if ( ( k >= 0 ) && (VTFrame[VTDialog[j].FramePos].SubPage[k].Fill == TRUE) ) VTDialog[j].SubPage=k;  //DF: else...
				SetDlgItemInt(VTDialog[j].Dialog,IDC_EDIT1,VTDialog[j].Page ,FALSE);
			};
		};
		return(TRUE);
		
		
		
		
	case IDM_VT_PAGE_DOWN:
		for ( i = 0; i < MAXVTDIALOG; i++ ) {
			if ( VTDialog[i].Dialog != NULL ) {
				j=Get_Dialog_Slot(VTDialog[i].Dialog);
				k=VTDialog[j].SubPage;
				if ( VTFrame[VTDialog[j].FramePos].SubCount == 0 ) return(TRUE);
				k++;
				while (( k <= VTFrame[VTDialog[j].FramePos].SubCount-1 ) && ( VTFrame[VTDialog[j].FramePos].SubPage[k].Fill == FALSE )) k++;
				if ( k >= VTFrame[VTDialog[j].FramePos].SubCount ) k=0; //DF:i
				while (( k <= VTFrame[VTDialog[j].FramePos].SubCount-1 ) && ( VTFrame[VTDialog[j].FramePos].SubPage[k].Fill == FALSE )) k++;
				if (k >= VTFrame[VTDialog[j].FramePos].SubCount) return(TRUE);
				
				//k++;
				//if ( k >= VTFrame[VTDialog[j].FramePos].SubCount ) k=0;
				//while (( k < VTFrame[VTDialog[j].FramePos].SubCount) && ( VTFrame[VTDialog[j].FramePos].SubPage[k].Fill == FALSE )) k++;
				if (( k < VTFrame[VTDialog[j].FramePos].SubCount ) && (VTFrame[VTDialog[j].FramePos].SubPage[k].Fill == TRUE)) VTDialog[j].SubPage=k; else return(TRUE);
				SetDlgItemInt(VTDialog[j].Dialog,IDC_EDIT1,VTDialog[j].Page ,FALSE);
			};
		};
		return(TRUE);

		
	case IDM_CHANNELPLUS:
		if ( USETUNER == FALSE ) return(TRUE);
		Clear_Status();
		if (( Has_BT == TRUE ) && ( Current_Mode==0 )) SetAudioSource(4);
			     if (( Has_DVB == TRUE ) && ( Current_Mode == 1 )) DVB_SetVolume((unsigned char)0, (unsigned char)0);
				 if ( CurrentProgramm == MAXPROGS ) CurrentProgramm = LastRealProg;
				 
				 i=CurrentProgramm+1;
				 CurrentProgramm=-1;
				 while ( i < MAXPROGS ) {
					 if (( Programm[i].freq != 0  ) && (ValidModes(Programm[i].Typ) == TRUE  ) && ( IsValidSelect(i) == TRUE )) {
						 CurrentProgramm=i;
						 break;
					 }
					 i++;
				 };
				 
				 if ( CurrentProgramm == -1 ) {
					 i=0;
					 while ( i < MAXPROGS ) {
						 if (( Programm[i].freq != 0  ) && (ValidModes(Programm[i].Typ) == TRUE  )&& ( IsValidSelect(i) == TRUE )) {
							 CurrentProgramm=i;
							 break;
						 }
						 i++;
					 } 
				 };
				 
				 if ( CurrentProgramm >= 0 ) {
					 if (( Current_Mode == 0 ) && ( Programm[CurrentProgramm].Typ == 'D' )) Set_Mode_Digital();
					 else if (( Current_Mode == 1 ) && ( Programm[CurrentProgramm].Typ == 'A' )) Set_Mode_Analog();
					 
					 if ( Programm[CurrentProgramm].Typ == 'A' ) Tuner_SetFrequency(TunerType,MulDiv(Programm[CurrentProgramm].freq*1000,16,1000000));
					 else Set_DVB_Channel(CurrentProgramm);
					 
					 
					 if ( Display_Status_Bar == TRUE ) {
						 sprintf(Text,"%04d. %s ",Make_ProgNr(CurrentProgramm)+1,Programm[CurrentProgramm].Name);
						 SetWindowText (hwndKeyField,Text);
					 }
					 VT_ChannelChange();
				 }
				 if (( Has_BT == TRUE ) && ( Current_Mode==0 )) SetAudioSource(AudioSource);
				 if (( Has_DVB == TRUE ) && ( Current_Mode == 1 )) DVB_SetVolume((unsigned char)InitialDVBVolumeLinks, (unsigned char)InitialDVBVolumeRechts);
				 return(TRUE);
				 
	case IDM_CHANNELMINUS:
		if ( USETUNER == FALSE ) return(TRUE);
		;
		Clear_Status();
		if (( Has_BT == TRUE ) && ( Current_Mode==0 )) SetAudioSource(4);
			     if (( Has_DVB == TRUE ) && ( Current_Mode == 1 )) DVB_SetVolume((unsigned char)0, (unsigned char)0);
				 if ( CurrentProgramm == MAXPROGS ) CurrentProgramm = LastRealProg;
				 i=CurrentProgramm-1;
				 CurrentProgramm=-1;
				 while ( i >= 0) {
					 if (( Programm[i].freq != 0  ) && (ValidModes(Programm[i].Typ) == TRUE  )&& ( IsValidSelect(i) == TRUE )) {
						 CurrentProgramm=i;
						 break;
					 }
					 i--;
				 };
				 
				 if ( CurrentProgramm == -1 ) {
					 i=MAXPROGS-1;
					 while ( i >= 0 ) {
						 if (( Programm[i].freq != 0  ) && (ValidModes(Programm[i].Typ) == TRUE  )&& ( IsValidSelect(i) == TRUE )) {
							 CurrentProgramm=i;
							 break;
						 }
						 i--;
					 } 
				 };
				 
				 if ( CurrentProgramm >= 0 ) {
					 
					 if (( Current_Mode == 0 ) && ( Programm[CurrentProgramm].Typ == 'D' )) Set_Mode_Digital();
					 else if (( Current_Mode == 1 ) && ( Programm[CurrentProgramm].Typ == 'A' )) Set_Mode_Analog();
					 
					 if ( Programm[CurrentProgramm].Typ == 'A' ) Tuner_SetFrequency(TunerType,MulDiv(Programm[CurrentProgramm].freq*1000,16,1000000));
					 else Set_DVB_Channel(CurrentProgramm);
					 
					 if ( Display_Status_Bar == TRUE ) {
						 sprintf(Text,"%04d. %s ",Make_ProgNr(CurrentProgramm)+1,Programm[CurrentProgramm].Name);
						 SetWindowText (hwndKeyField,Text);
					 }
					 VT_ChannelChange();
				 }
				 if (( Has_BT == TRUE ) && ( Current_Mode==0 )) SetAudioSource(AudioSource);
				 if (( Has_DVB == TRUE ) && ( Current_Mode == 1 )) DVB_SetVolume((unsigned char)InitialDVBVolumeLinks, (unsigned char)InitialDVBVolumeRechts);
				 return(TRUE);
				 
				 
	case IDM_LOCK :    if ( MoreLinesLock==TRUE ) {
								MoreLinesLock=FALSE;
								Clear_Status();
								sprintf(Text,"Stabilisierung reset");
								if ( Display_Status_Bar == TRUE ) SetWindowText (hwndTextField,Text);
					   }  else {
						   sprintf(Text,"Stabilisierung wird gelock");
						   if ( Display_Status_Bar == TRUE ) SetWindowText (hwndTextField,Text);
						   MoreLinesLock=TRUE;
					   }
					   return(TRUE);
					   
					   
	case IDM_MOVIEBOOST_169:
		Stop_Thread();
							 MovieBoost=37;
							 AutoMovieBoostEnable=FALSE;
							 sprintf(Text,"MovieBoost ist %d (16:9)",MovieBoost);
							 if ( Display_Status_Bar == TRUE ) SetWindowText (hwndTextField,Text);
							 
							 Start_Thread();
							 return(TRUE);
							 
	case IDM_MOVIEBOOST_KEINER:
		Stop_Thread();
							 MovieBoost=0;
							 AutoMovieBoostEnable=FALSE;
							 sprintf(Text,"MovieBoost ist aus");
							 if ( Display_Status_Bar == TRUE ) SetWindowText (hwndTextField,Text);
                             Start_Thread();
							 return(TRUE);
							 
	case IDM_MOVIEBOOST_MINUS:
		Stop_Thread();
		if ( MovieBoost < 90 ) MovieBoost++;
		sprintf(Text,"MovieBoost ist %d",MovieBoost);
							 AutoMovieBoostEnable=FALSE;
							 if ( Display_Status_Bar == TRUE ) SetWindowText (hwndTextField,Text);
                             Start_Thread();
							 return(TRUE);
							 
	case IDM_MOVIEBOOST_PLUS:
		Stop_Thread();
		if ( MovieBoost > 0 ) MovieBoost--;
		sprintf(Text,"MovieBoost ist %d",MovieBoost);
		if ( Display_Status_Bar == TRUE ) SetWindowText (hwndTextField,Text);
							 AutoMovieBoostEnable=FALSE;
                             Start_Thread();
							 return(TRUE);
							 
	case IDM_AUTOSCALE:
		
		Stop_Thread();
		MovieScale=G_Wert(MatrixLinesEven,MatrixLinesOdd);
							 if (( ModeOverlay == TRUE ) && ( Can_Overlay ) && ( Can_Overlay_Strech == TRUE ) && (USE_DX_OV_STRECH== TRUE )) Init_Screen_Struct();
							 sprintf(Text,"Y-Strech ist %d ",MovieScale);
							 if ( Display_Status_Bar == TRUE ) SetWindowText (hwndTextField,Text);
                             Start_Thread();
							 return(TRUE);
							 
							 
							 
	case IDM_SCALE_169:
		Stop_Thread();
							 MovieScale=37;
							 if (( ModeOverlay == TRUE ) && ( Can_Overlay ) && ( Can_Overlay_Strech == TRUE ) && (USE_DX_OV_STRECH== TRUE )) Init_Screen_Struct();
							 sprintf(Text,"Y-Strech ist %d (16:9)",MovieScale);
							 if ( Display_Status_Bar == TRUE ) SetWindowText (hwndTextField,Text);
                             Start_Thread();
							 return(TRUE);
							 
	case IDM_SCALE_KEINER:
		Stop_Thread();
							 MovieScale=0;
							 if (( ModeOverlay == TRUE ) && ( Can_Overlay ) && ( Can_Overlay_Strech == TRUE ) && (USE_DX_OV_STRECH== TRUE )) Init_Screen_Struct();
							 sprintf(Text,"Y-Strech ist aus");
							 if ( Display_Status_Bar == TRUE ) SetWindowText (hwndTextField,Text);
							 
                             Start_Thread();
							 return(TRUE);
							 
	case IDM_SCALE_MINUS:
		Stop_Thread();
		if ( MovieScale < 90 ) MovieScale++;
							 if (( ModeOverlay == TRUE ) && ( Can_Overlay ) && ( Can_Overlay_Strech == TRUE ) && (USE_DX_OV_STRECH== TRUE )) Init_Screen_Struct();
							 sprintf(Text,"Y-Strech ist %d",MovieScale);
							 if ( Display_Status_Bar == TRUE ) SetWindowText (hwndTextField,Text);
                             Start_Thread();
							 return(TRUE);
							 
	case IDM_SCALE_PLUS:
		Stop_Thread();
		if ( MovieScale > 0 ) MovieScale--;
							 if (( ModeOverlay == TRUE ) && ( Can_Overlay ) && ( Can_Overlay_Strech == TRUE ) && (USE_DX_OV_STRECH== TRUE )) Init_Screen_Struct();
							 sprintf(Text,"Y-Strech ist %d",MovieScale);
							 if ( Display_Status_Bar == TRUE ) SetWindowText (hwndTextField,Text);
							 
                             Start_Thread();
							 return(TRUE);
							 
	case IDM_RESET_ARM:
		SAA7146_Restart_Arm();
		return(TRUE);
	case IDM_RESET:     
		if ( Current_Mode == 0 ) Reset_BT_HardWare(hWnd);
							 else if ( Current_Mode == 1 ) DVB_Reset();
							 return(TRUE);
							 
	case IDM_TOGGLE_MENU:
		if ( Show_Menu == TRUE ) {
			Stop_Thread();
			Set_Capture(4);
			Show_Menu=FALSE;
			Display_Status_Bar=FALSE;
			ShowWindow(Status_Bar,SW_HIDE);
			KillTimer(hWnd, 1);
			
			WStyle=GetWindowLong(hWnd,GWL_STYLE);
			if ( Toggle_WithOut_Frame == TRUE ) SetWindowLong(hWnd,GWL_STYLE,WS_VISIBLE); 
			else  SetWindowLong(hWnd,GWL_STYLE,WS_VISIBLE|WS_THICKFRAME);
			SetMenu(hWnd,NULL);
			Init_Screen_Struct();
			Set_Capture(5);
			if ( WindowStyle == 2 ) memset(lpSurface,0x00,800*600*2);
			if ( WindowStyle == 1 ) memset(lpSurface,0x00,640*480*2);
			if ( WindowStyle == 3 ) memset(lpSurface,0x00,Res_X*Res_Y*2);
			return(TRUE);
		};
		
		Stop_Thread();
							 Set_Capture(4);
							 Show_Menu=TRUE;
							 Display_Status_Bar=TRUE;
							 ShowWindow(Status_Bar,SW_SHOW);
							 SetTimer (hWnd, 1, 2500, NULL);
                             SetWindowLong(hWnd,GWL_STYLE,WStyle); 
							 SetMenu(hWnd,hMenu);
                             Init_Screen_Struct();
                             Set_Capture(5);
							 return(TRUE);
							 
							 
	case IDM_KEYSET:
		Save_Mode();
						  DialogBox(hInst,"NAGRAKEYSET",hWnd,NagraKeySetProc);
                          Restore_Mode();
						  return(TRUE);
						  
	case IDM_ABOUT:
		Save_Mode();
						  DialogBox(hInst,"ABOUT",hWnd,AboutProc);
                          Restore_Mode();
						  return(TRUE);
						  
	case IDM_NAGRA_SETUP:
		Save_Mode();
						  DialogBox(hInst,"NAGRASETUP",hWnd,NagraSetupProc);
                          Restore_Mode();
						  return(TRUE);
						  
	case IDM_FullFields:
		if (DecodeNagra==TRUE){
			if(NagraFullField==FALSE) NagraFullField=TRUE; else NagraFullField=FALSE; 
			if ( NagraFullField == TRUE )  sprintf(Text,"Beide Halbbilder");
			else sprintf(Text,"Ein Halbbild");
			if ( Display_Status_Bar == TRUE ) SetWindowText (hwndTextField,Text);
			
			DecodeNagra=FALSE;
			SendMessage(hWnd,WM_COMMAND,IDM_DECODING_NAGRA1,0);
			MoreLinesLock=FALSE;
		} else {
			Stop_Thread();
			Set_Capture(4);
			if ( HalfTVMode == FALSE ) {
				CheckMenuItem( GetMenu(hWnd), IDM_HALFMODE, MF_CHECKED );
				HalfTVMode=TRUE;
				Init_Screen_Struct();
				Set_Capture(5);
				sprintf(Text,"Ein Halbbild");
				if ( Display_Status_Bar == TRUE ) SetWindowText (hwndTextField,Text);
				return(TRUE);
			} 
			CheckMenuItem( GetMenu(hWnd), IDM_HALFMODE, MF_UNCHECKED );
			HalfTVMode=FALSE;
			Init_Screen_Struct();
			Set_Capture(5);
			sprintf(Text,"Beide Halbbilder");
			if ( Display_Status_Bar == TRUE ) SetWindowText (hwndTextField,Text);
			return(TRUE);
		}
		return(TRUE);
		
		
	case IDM_DECODING_NAGRA1:
		Stop_Thread();
		Set_Capture(4);
		if ( DecodeNagra == FALSE ) {
			if (LeseKeyDatei(hWnd) == FALSE ) {
				Set_Capture(5);
				return(FALSE);
			}
			
			CheckMenuItem( GetMenu(hWnd), IDM_RAW_SURFACE, MF_UNCHECKED );
			CheckMenuItem( GetMenu(hWnd), IDM_RAW_OVERLAY, MF_UNCHECKED );
			CheckMenuItem( GetMenu(hWnd), IDM_INTERN_SURFACE, MF_UNCHECKED );
			CheckMenuItem( GetMenu(hWnd), IDM_OVERLAY, MF_UNCHECKED );
			CheckMenuItem( GetMenu(hWnd), IDM_DIB, MF_UNCHECKED );
			
			if ( OutDib == TRUE ) CheckMenuItem( GetMenu(hWnd), IDM_DIB, MF_CHECKED );
			else if ( ModeOverlay == TRUE ) CheckMenuItem( GetMenu(hWnd), IDM_OVERLAY, MF_CHECKED );
			else CheckMenuItem( GetMenu(hWnd), IDM_INTERN_SURFACE, MF_CHECKED ); 
			
			
			ModeRAWOverlay=FALSE;
     				   ModeRAWSurface=FALSE;
					   if ( TVTYPE != 5 ) S_TVTYPE=TVTYPE;
					   S_ColourFormat=ColourFormat;
					   CheckMenuItem( GetMenu(hWnd), TVTYPE+1120, MF_UNCHECKED );
					   if (TVTYPE==0) TVTYPE=5;
					   CheckMenuItem( GetMenu(hWnd), TVTYPE+1120, MF_CHECKED );
					   CheckMenuItem( GetMenu(hWnd), ColourFormat+1130, MF_UNCHECKED );
					   ColourFormat=4;
					   SetColourFormat(ColourFormat);
					   CheckMenuItem( GetMenu(hWnd), ColourFormat+1130, MF_CHECKED );
					   DecodeVCrypt=FALSE; 
					   DecodeNagra=TRUE;
					   DecodeDecoderVCrypt=FALSE;
					   
					   if ( !Init_Nagra_Decoding(MasterTestzeilen) ) {
						   DecodeVCrypt=FALSE; 
						   DecodeNagra=FALSE;
						   DecodeDecoderVCrypt=FALSE;
					   };
					   
					   Init_Screen_Struct();
		};
		Set_Capture(5);
		return(TRUE);
		
		
	case IDM_BRIGHTNESSPLUS:
		
		if ( Current_Mode == 0 ) {
			if ( InitialBrightness < 128 ) i=InitialBrightness;
			else i=-(256-InitialBrightness);
			
			if (( i > -127 ) && ( i < 127 )) i++;
			if ( i < 0 ) InitialBrightness=256+i;
			else InitialBrightness=i;
			SetBrightness(InitialBrightness);
		} else if ( Current_Mode == 1 ){
			if ( InitialDVBBrightness < 255 ) InitialDVBBrightness++;
			i=InitialDVBBrightness;
			set_picture_prop(InitialDVBBrightness,InitialDVBContrast,InitialDVBColor);
			
		}
		sprintf(Text,"Helligkeit %d",i);
		if ( Display_Status_Bar == TRUE ) SetWindowText (hwndTextField,Text);
		return(TRUE);
		
	case IDM_BRIGHTNESSMINUS:
		
		if ( Current_Mode == 0 ) {
			if ( InitialBrightness < 128 ) i=InitialBrightness;
			else i=-(256-InitialBrightness);
			if (( i > -127 ) && ( i < 127 )) i--;
			if ( i < 0 ) InitialBrightness=256+i;
			else InitialBrightness=i;
			SetBrightness(InitialBrightness);
		} else if ( Current_Mode == 1 ){
			if ( InitialDVBBrightness > 0 ) InitialDVBBrightness--;
			i=InitialDVBBrightness;
			set_picture_prop(InitialDVBBrightness,InitialDVBContrast,InitialDVBColor);
		}
		sprintf(Text,"Helligkeit %d",i);
		if ( Display_Status_Bar == TRUE ) SetWindowText (hwndTextField,Text);
		return(TRUE);
		
	case IDM_COLORPLUS:
		if ( Current_Mode == 0 ){
			if (( InitialSaturationU < 255 ) && (InitialSaturationV < 255 )) {
				InitialSaturationU++;
				InitialSaturationV++;
				sprintf(Text,"Farbe U %d V %d",InitialSaturationU,InitialSaturationV);
				if ( Display_Status_Bar == TRUE ) SetWindowText (hwndTextField,Text);
				SetSaturationU(InitialSaturationU);
				SetSaturationV(InitialSaturationV);
			}
		} else if ( Current_Mode == 1 ){
			if ( InitialDVBColor < 127  ) InitialDVBColor++;
			sprintf(Text,"Farbe %d",InitialDVBColor);
			if ( Display_Status_Bar == TRUE ) SetWindowText (hwndTextField,Text);
			set_picture_prop(InitialDVBBrightness,InitialDVBContrast,InitialDVBColor);
		}
		return(TRUE);
		
	case IDM_COLORMINUS:
		if ( Current_Mode == 0 ){
			if (( InitialSaturationU > 0 ) && (InitialSaturationV > 0 )) {
				InitialSaturationU--;
				InitialSaturationV--;
				sprintf(Text,"Farbe U %d V %d",InitialSaturationU,InitialSaturationV);
				if ( Display_Status_Bar == TRUE ) SetWindowText (hwndTextField,Text);
				SetSaturationU(InitialSaturationU);
				SetSaturationV(InitialSaturationV);
			}
		} else if ( Current_Mode == 1 ){
			if ( InitialDVBColor > 0 ) InitialDVBColor--;
			sprintf(Text,"Farbe %d",InitialDVBColor);
			if ( Display_Status_Bar == TRUE ) SetWindowText (hwndTextField,Text);
			set_picture_prop(InitialDVBBrightness,InitialDVBContrast,InitialDVBColor);
		}
		return(TRUE);
		
	case IDM_KONTRASTPLUS:
		if ( Current_Mode == 0 ){
			if ( InitialContrast < 256 ) InitialContrast++;
			SetContrast(InitialContrast);    
			sprintf(Text,"Kontrast %d",InitialContrast);
			if ( Display_Status_Bar == TRUE ) SetWindowText (hwndTextField,Text);
		} else if ( Current_Mode == 1 ){
			if ( InitialDVBContrast < 127) InitialDVBContrast++;
			sprintf(Text,"Kontrast %d",InitialDVBContrast);
			if ( Display_Status_Bar == TRUE ) SetWindowText (hwndTextField,Text);
			set_picture_prop(InitialDVBBrightness,InitialDVBContrast,InitialDVBColor);
		}
		return(TRUE);
		
	case IDM_KONTRASTMINUS:
		if ( Current_Mode == 0 ){
			if ( InitialContrast > 0 ) InitialContrast--;
			SetContrast(InitialContrast);    
			sprintf(Text,"Kontrast %d",InitialContrast);
			if ( Display_Status_Bar == TRUE ) SetWindowText (hwndTextField,Text);
		} else if ( Current_Mode == 1 ){
			if ( InitialDVBContrast > 0) InitialDVBContrast--;
			sprintf(Text,"Kontrast %d",InitialDVBContrast);
			if ( Display_Status_Bar == TRUE ) SetWindowText (hwndTextField,Text);
			set_picture_prop(InitialDVBBrightness,InitialDVBContrast,InitialDVBColor);
		}
		return(TRUE);
		
	case IDM_MUTE:
		if ( System_In_Mute == FALSE ) {
			System_In_Mute = TRUE ;
			if ( USE_MIXER == FALSE  ) {
				if ( Current_Mode == 0 ) {
					SetAudioSource(4);
					sprintf(Text,"Mute auf BT");
				} else if ( Current_Mode == 1 ) {
					DVB_SetVolume((unsigned char)0x00, (unsigned char)0x00);
					sprintf(Text,"Mute auf DVB");
				}
			};
			if ( USE_MIXER == TRUE ) {
				Mixer_Mute();
				sprintf(Text,"Mute auf Mixer");
			}
		} else {
			System_In_Mute = FALSE ;
			if ( USE_MIXER == FALSE  ) {
				if ( Current_Mode == 0 ) {
					SetAudioSource(AudioSource);
					sprintf(Text,"UnMute auf BT");
				} else if ( Current_Mode == 1 ) {
					DVB_SetVolume((unsigned char)InitialDVBVolumeLinks, (unsigned char)InitialDVBVolumeRechts);
					sprintf(Text,"UnMute auf DVB");
				}
			};
			if ( USE_MIXER == TRUE ) {
				Mixer_UnMute();
				sprintf(Text,"UnMute auf Mixer");
			}
			
		}
		if ( Display_Status_Bar == TRUE ) SetWindowText (hwndTextField,Text);
		return(TRUE);
		
	case IDM_L_BALANCE:
		if ( USE_MIXER == FALSE  ) {
			if ( Current_Mode == 0 ) {
				if ( InitialBalance > -127 ) InitialBalance--;
				Audio_SetBalance(InitialBalance); // -127 - +128
				sprintf(Text,"BT-Balance %d",InitialBalance);
			} else if ( Current_Mode == 1 ) { 
				
				if  ( InitialDVBVolumeLinks < 255 ) InitialDVBVolumeLinks++;
				if  ( InitialDVBVolumeRechts > 0 ) InitialDVBVolumeRechts--;
				DVB_SetVolume((unsigned char)InitialDVBVolumeLinks, (unsigned char)InitialDVBVolumeRechts);
				sprintf(Text,"DVB-Balance L %d R %d",InitialDVBVolumeLinks,InitialDVBVolumeRechts);
				Update_OSD_Volume();
			};
		}
		if ( USE_MIXER == TRUE ) {
			if ( MIXER_LINKER_KANAL+MixerVolumeStep <= MixerVolumeMax ) MIXER_LINKER_KANAL+=MixerVolumeStep;
			if ( MIXER_RECHTER_KANAL-MixerVolumeStep >= 0 ) MIXER_RECHTER_KANAL-=MixerVolumeStep;
			Mixer_Set_Volume(MIXER_LINKER_KANAL,MIXER_RECHTER_KANAL);
			sprintf(Text,"Balance L %d R %d",MIXER_LINKER_KANAL,MIXER_RECHTER_KANAL);
			if ( Current_Mode == 1 ) Update_OSD_Volume();
		}
		
		if ( Display_Status_Bar == TRUE ) SetWindowText (hwndTextField,Text);
		return(TRUE);
		
	case IDM_R_BALANCE:
		if ( USE_MIXER == FALSE  ) {
			if ( Current_Mode == 0 ) { 
				if ( InitialBalance < 128 ) InitialBalance++;
				Audio_SetBalance(InitialBalance); // -127 - +128
				sprintf(Text,"BT-Balance %d",InitialBalance);
			} else if ( Current_Mode == 1 ) { 
				if  ( InitialDVBVolumeLinks > 0 ) InitialDVBVolumeLinks--;
				if  ( InitialDVBVolumeRechts < 255) InitialDVBVolumeRechts++;
				DVB_SetVolume((unsigned char)InitialDVBVolumeLinks, (unsigned char)InitialDVBVolumeRechts);
				sprintf(Text,"DVB-Balance L %d R %d",InitialDVBVolumeLinks,InitialDVBVolumeRechts);
				Update_OSD_Volume();
			};
		}
		if ( USE_MIXER == TRUE ) {
			if ( MIXER_LINKER_KANAL-MixerVolumeStep >= 0 ) MIXER_LINKER_KANAL-=MixerVolumeStep;
			if ( MIXER_RECHTER_KANAL+MixerVolumeStep <= MixerVolumeMax ) MIXER_RECHTER_KANAL+=MixerVolumeStep;
			Mixer_Set_Volume(MIXER_LINKER_KANAL,MIXER_RECHTER_KANAL);
			if ( Current_Mode == 1 ) Update_OSD_Volume();
			sprintf(Text,"Balance L %d R %d",MIXER_LINKER_KANAL,MIXER_RECHTER_KANAL);
		}
		
		if ( Display_Status_Bar == TRUE ) SetWindowText (hwndTextField,Text);
		return(TRUE);
		
	case IDM_VOLUMEPLUS:
			     if ( USE_MIXER == FALSE  ) {
					 if ( Current_Mode == 0 ) { 
						 if ( InitialVolume < 980 ) InitialVolume+=20;
						 Audio_SetVolume(InitialVolume);
						 sprintf(Text,"BT-Volume %d",InitialVolume);
					 } else if ( Current_Mode == 1 ) { 
						 if (( InitialDVBVolumeLinks < 255 ) && ( InitialDVBVolumeRechts < 255 )) {
							 InitialDVBVolumeLinks++;
							 InitialDVBVolumeRechts++;
						 };
						 
						 DVB_SetVolume((unsigned char)InitialDVBVolumeLinks, (unsigned char)InitialDVBVolumeRechts);
						 sprintf(Text,"DVB-Volume L %d R %d",InitialDVBVolumeLinks,InitialDVBVolumeRechts);
                         Update_OSD_Volume();
					 };
					 
				 };
                 if ( USE_MIXER == TRUE ) {
                     i=((MixerVolumeMax/MixerVolumeStep)/100)*MixerVolumeStep;
                     if (( i+MIXER_LINKER_KANAL > MixerVolumeMax ) || ( i+MIXER_RECHTER_KANAL > MixerVolumeMax )) i=MixerVolumeStep;
					 if ( MIXER_LINKER_KANAL+i <= MixerVolumeMax ) MIXER_LINKER_KANAL+=i;
					 if ( MIXER_RECHTER_KANAL+i <= MixerVolumeMax ) MIXER_RECHTER_KANAL+=i;
					 Mixer_Set_Volume(MIXER_LINKER_KANAL,MIXER_RECHTER_KANAL);
					 sprintf(Text,"Volume L %d R %d",MIXER_LINKER_KANAL,MIXER_RECHTER_KANAL);
					 if ( Current_Mode == 1 ) Update_OSD_Volume();
				 };
				 
				 if ( Display_Status_Bar == TRUE ) SetWindowText (hwndTextField,Text);
                 return(TRUE);
				 
	case IDM_VOLUMEMINUS:
		if ( USE_MIXER == FALSE  ) {
			if ( Current_Mode == 0 ) { 
				if ( InitialVolume > 20 ) InitialVolume-=20;
				Audio_SetVolume(InitialVolume);
				sprintf(Text,"BT-Volume %d",InitialVolume);
			} else if ( Current_Mode == 1 ) { 
				if (( InitialDVBVolumeLinks > 0 ) && ( InitialDVBVolumeRechts > 0)) {
					InitialDVBVolumeLinks--;
					InitialDVBVolumeRechts--;
				}
				DVB_SetVolume((unsigned char)InitialDVBVolumeLinks, (unsigned char)InitialDVBVolumeRechts);
				sprintf(Text,"DVB-Volume L %d R %d",InitialDVBVolumeLinks,InitialDVBVolumeRechts);
				Update_OSD_Volume();
			};
			
		}
		if ( USE_MIXER == TRUE ) {
			i=((MixerVolumeMax/MixerVolumeStep)/100)*MixerVolumeStep;
			if (( MIXER_LINKER_KANAL-i < 0) || ( MIXER_RECHTER_KANAL-i < 0)) i=MixerVolumeStep;
			if ( MIXER_LINKER_KANAL-i >= 0 ) MIXER_LINKER_KANAL-=i; 
			if ( MIXER_RECHTER_KANAL-i >= 0 ) MIXER_RECHTER_KANAL-=i;
			Mixer_Set_Volume(MIXER_LINKER_KANAL,MIXER_RECHTER_KANAL);
			sprintf(Text,"Volume L %d R %d",MIXER_LINKER_KANAL,MIXER_RECHTER_KANAL);
			if ( Current_Mode == 1 ) Update_OSD_Volume();
			
		}
		
		
		if ( Display_Status_Bar == TRUE ) SetWindowText (hwndTextField,Text);
		return(TRUE);
		
	case IDM_TOGGLECURSOR:
		CShowCursor=!CShowCursor;
		i=ShowCursor(CShowCursor);
		return(TRUE);
		
		
	case IDM_KEY0:
		      Stop_Thread();
			  i=ActiveKey;
			  ActiveKey=0;
              sprintf(Text,"KeySet %s",OPKEY[ActiveKey].Name);
			  if ( Display_Status_Bar == TRUE ) SetWindowText (hwndTextField,Text);
			  if ( !Init_Nagra_Decoding(MasterTestzeilen)) {
				  ActiveKey=i;
			  };
              Start_Thread();
              return(TRUE);
			  
	case IDM_KEY1:
		      Stop_Thread();
			  i=ActiveKey;
			  ActiveKey=1;
              sprintf(Text,"KeySet %s",OPKEY[ActiveKey].Name);
			  if ( Display_Status_Bar == TRUE ) SetWindowText (hwndTextField,Text);
			  if ( !Init_Nagra_Decoding(MasterTestzeilen)) {
				  ActiveKey=i;
			  };
              Start_Thread();
              return(TRUE);
	case IDM_KEY2:
		      Stop_Thread();
			  i=ActiveKey;
			  ActiveKey=2;
              sprintf(Text,"KeySet %s",OPKEY[ActiveKey].Name);
			  if ( Display_Status_Bar == TRUE ) SetWindowText (hwndTextField,Text);
			  if ( !Init_Nagra_Decoding(MasterTestzeilen)) {
				  ActiveKey=i;
			  };
              Start_Thread();
              return(TRUE);
	case IDM_KEY3:
		      Stop_Thread();
			  i=ActiveKey;
			  ActiveKey=3;
              sprintf(Text,"KeySet %s",OPKEY[ActiveKey].Name);
			  if ( Display_Status_Bar == TRUE ) SetWindowText (hwndTextField,Text);
			  if ( !Init_Nagra_Decoding(MasterTestzeilen)) {
				  ActiveKey=i;
			  };
              Start_Thread();
              return(TRUE);
			  
	case IDM_MORETZ:
		if (DecodeNagra==TRUE) {	 
			Stop_Thread();
			for ( i = 0; i < 5 ; i++ ) {
				if( MasterTestzeilen < TZ[i]) {
					MasterTestzeilen=TZ[i];
					break;
				};
			};
			sprintf(Text,"Testzeilen auf  %d",MasterTestzeilen);
			if ( Display_Status_Bar == TRUE ) SetWindowText (hwndTextField,Text);
			if ( !Init_Nagra_Decoding(MasterTestzeilen)) {
				SendMessage(hWnd,WM_COMMAND,IDM_DECODING_OFF,0);
				break;
			};
			
			if ( MatrixModeEven == TRUE )  {
				GetMatrix(MatrixLinesEven-1,0);
			}
			if ( MatrixModeOdd == TRUE )  {
				GetMatrix(MatrixLinesOdd-1,1);
			}
			
			Start_Thread();
		}
		
		return(TRUE);
		
	case IDM_LESSTZ:
		if (DecodeNagra==TRUE) {	 
			Stop_Thread();
			for ( i = 4; i >= 0 ; i-- ) {
				if( MasterTestzeilen > TZ[i]) {
					MasterTestzeilen=TZ[i];
					break;
				};
			};
			sprintf(Text,"Testzeilen auf  %d",MasterTestzeilen);
			if ( Display_Status_Bar == TRUE ) SetWindowText (hwndTextField,Text);
			if ( !Init_Nagra_Decoding(MasterTestzeilen)) {
				SendMessage(hWnd,WM_COMMAND,IDM_DECODING_OFF,0);
				break;
			};
			if ( MatrixModeEven == TRUE )  {
				GetMatrix(MatrixLinesEven-1,0);
			}
			if ( MatrixModeOdd == TRUE )  {
				GetMatrix(MatrixLinesOdd-1,1);
			}
			
			Start_Thread();
		}
		return(TRUE);
		
		
	case IDM_DECODING_VIDEOCRYPTSETUP:
		Save_Mode();
		DialogBox(hInst,"VIDEOCRYPTSETUP",hWnd,VCSetupProc);
		Restore_Mode();
		return(TRUE);
		
	case IDM_CHROMA_KEYING:
		Chroma_Key=!Chroma_Key;
		return(TRUE);
		
		
	case IDM_DECODING_VIDEOCRYPT:
		Stop_Thread();
		Set_Capture(4);
		MoreLinesLock=FALSE;
		
		if ( DecodeVCrypt == FALSE ) {
			DecodeVCrypt=TRUE; 
			DecodeNagra=FALSE;
			DecodeDecoderVCrypt=FALSE;
			if ( TVTYPE != 5 ) S_TVTYPE=TVTYPE;
			TVTYPE=5;
			S_ColourFormat=ColourFormat;
			CheckMenuItem( GetMenu(hWnd), ColourFormat+1130, MF_UNCHECKED );
			ColourFormat=4;
			SetColourFormat(ColourFormat);
			CheckMenuItem( GetMenu(hWnd), ColourFormat+1130, MF_CHECKED );
			DecodeVCrypt=TRUE; 
			DecodeNagra=FALSE;
			DecodeDecoderVCrypt=FALSE;
			Init_Screen_Struct();;
		};
		
		Set_Capture(5);
		return(TRUE);
		
	case IDM_DECODER_MODE:
		Stop_Thread();
		Set_Capture(4);
		MoreLinesLock=FALSE;
		
		if ( DecodeDecoderVCrypt == FALSE ) {
			DecodeVCrypt=FALSE; 
			DecodeNagra=FALSE;
			DecodeDecoderVCrypt=TRUE;
			if ( TVTYPE != 5 ) S_TVTYPE=TVTYPE;
			TVTYPE=5;
			
			S_ColourFormat=ColourFormat;
			CheckMenuItem( GetMenu(hWnd), ColourFormat+1130, MF_UNCHECKED );
			ColourFormat=4;
			SetColourFormat(ColourFormat);
			CheckMenuItem( GetMenu(hWnd), ColourFormat+1130, MF_CHECKED );
			Init_Screen_Struct();
		};
		
		Set_Capture(5);
		return(TRUE);
		
		
		
	case IDM_DECODING_OFF:
		Stop_Thread();
		Set_Capture(4);
		DecodeVCrypt=FALSE; 
			     DecodeNagra=FALSE;
				 MoreLinesLock=FALSE;
				 DecodeDecoderVCrypt=FALSE;
				 CheckMenuItem( GetMenu(hWnd), TVTYPE+1120, MF_UNCHECKED );
				 TVTYPE=S_TVTYPE;
				 CheckMenuItem( GetMenu(hWnd), TVTYPE+1120, MF_CHECKED );
				 CheckMenuItem( GetMenu(hWnd), ColourFormat+1130, MF_UNCHECKED );
				 ColourFormat=S_ColourFormat;
                 SetColourFormat(ColourFormat);
				 CheckMenuItem( GetMenu(hWnd), ColourFormat+1130, MF_CHECKED );
                 Init_Screen_Struct();
				 Free_Nagra_Decoding();
                 SendMessage(hWnd,WM_COMMAND,OldMode,0);
                 return(TRUE);
				 
	case IDM_KARTE0:
	case IDM_KARTE1:
	case IDM_KARTE2:
	case IDM_KARTE3:
	case IDM_KARTE4:
	case IDM_KARTE5:
	case IDM_KARTE6:
		if ( Current_Mode == 0 ) {
			CheckMenuItem( GetMenu(hWnd), CardType+1080, MF_UNCHECKED );
			CardType=wParam-1080;
			CheckMenuItem( GetMenu(hWnd), CardType+1080, MF_CHECKED );
		} else  if ( Current_Mode == 1 ) {
			CheckMenuItem( GetMenu(hWnd), DVBDecoderType+1080, MF_UNCHECKED );
			DVBDecoderType=wParam-1080;
			CheckMenuItem( GetMenu(hWnd), DVBDecoderType+1080, MF_CHECKED );
		}
		return(TRUE);
	case IDM_MANUELL_CARD:
		Save_Mode();
		DialogBox(hInst,"CARDTYPE",hWnd,(DLGPROC)CardSettingProc);
		Restore_Mode();
		return(TRUE);
		
		
	case IDM_ENDE:
		PostMessage(hWnd,WM_DESTROY,wParam,lParam);
		return(TRUE);
		
	case IDM_MSPMODE_3:
	case IDM_MSPMODE_2:
	case IDM_MSPMODE_4:
	case IDM_MSPMODE_5:
	case IDM_MSPMODE_6:
		 	    MSP_SetMode( wParam-1600);
                return(TRUE);
				
	case IDM_MAJOR_CARRIER_0:
	case IDM_MAJOR_CARRIER_1:
	case IDM_MAJOR_CARRIER_2:
	case IDM_MAJOR_CARRIER_3:
		MSP_Set_MajorMinor_Mode(wParam-1610,MSPMinorMode);
		return(TRUE);
		
	case IDM_MINOR_CARRIER_0:
	case IDM_MINOR_CARRIER_1:
	case IDM_MINOR_CARRIER_2:
	case IDM_MINOR_CARRIER_3:
	case IDM_MINOR_CARRIER_4:
	case IDM_MINOR_CARRIER_5:
	case IDM_MINOR_CARRIER_6:
	case IDM_MINOR_CARRIER_7:
		MSP_Set_MajorMinor_Mode(MSPMajorMode,wParam-1620  );
		return(TRUE);
		
		
	case IDM_MSPSTEREO_1:
	case IDM_MSPSTEREO_2:
	case IDM_MSPSTEREO_3:
	case IDM_MSPSTEREO_4:
		MSP_SetStereo(MSPMajorMode, MSPMinorMode,wParam-1630);
		return(TRUE);
		
		
	case IDM_AUTOSTEREO:
		if ( AutoStereoSelect == FALSE ) {
			AutoStereoSelect=TRUE;
			CheckMenuItem( GetMenu(hWnd), IDM_AUTOSTEREO, MF_CHECKED );
			
		} else {
			AutoStereoSelect=FALSE;
			CheckMenuItem( GetMenu(hWnd), IDM_AUTOSTEREO, MF_UNCHECKED );
		}
		return(TRUE);
		
		
	case IDM_AUDIO_0:
	case IDM_AUDIO_1:
	case IDM_AUDIO_2:
	case IDM_AUDIO_3:
	case IDM_AUDIO_4:
	case IDM_AUDIO_5:
		
		CheckMenuItem( GetMenu(hWnd), AudioSource+1110, MF_UNCHECKED );
		AudioSource=wParam-1110;
		Set_Capture(4);
		SetAudioSource(AudioSource);
		Set_Capture(5);
		CheckMenuItem( GetMenu(hWnd), AudioSource+1110, MF_CHECKED );
		return(TRUE);
		
		
	case IDM_EXTERN1: 
	case IDM_EXTERN2: 
	case IDM_EXTERN3: 
		CheckMenuItem( GetMenu(hWnd), VideoSource+1089, MF_UNCHECKED );
		VideoSource=(wParam&0xffff)-1089;
		Set_Capture(4);
		SetVideoSource(VideoSource);
		sprintf(Text,"Extern %d",VideoSource);
		if ( Display_Status_Bar == TRUE ) SetWindowText (hwndKeyField,Text);
		Set_Capture(5);
		CheckMenuItem( GetMenu(hWnd), VideoSource+1089, MF_CHECKED );
		return(TRUE);
		
	case IDM_VIDEO:
		if ( Capture_Video == FALSE ) {
			Capture_Video = TRUE ;
			Set_Capture(0);
			CheckMenuItem( GetMenu(hWnd), IDM_VIDEO, MF_CHECKED );
		} else {
			Set_Capture(1);
			Capture_Video = FALSE;
			CheckMenuItem( GetMenu(hWnd), IDM_VIDEO, MF_UNCHECKED );
		}
		return(TRUE);
		
	case IDM_HWINFO :
		Save_Mode();
		DialogBox(hInst,"HWINFO",hWnd,(DLGPROC)ChipSettingProc);
		Restore_Mode();
		return(TRUE);
	case IDM_BTNEWDRIVER:
		if ( LoadExternBTDriver == TRUE ) {
			LoadExternBTDriver = FALSE;
			CheckMenuItem( GetMenu(hWnd), IDM_BTNEWDRIVER, MF_UNCHECKED );
		} else {
			if (( NT == TRUE ) && ( Check_BT_Plugin() == FALSE )) {
				// Treiber schon mal eintragen 
				GetCurrentDirectory(  sizeof (Text),Text);
				strcat(Text,"\\MDBT.SYS");
				LoadDeviceDriver( "MultiDec BT-Treiber", Text, (HANDLE*)&i,TRUE  );
			};
			LoadExternBTDriver = TRUE;
			CheckMenuItem( GetMenu(hWnd), IDM_BTNEWDRIVER, MF_CHECKED );
		};
		return(TRUE);
	case IDM_PLL:
		Save_Mode();
		DialogBox(hInst,"PLLINFO",hWnd,(DLGPROC)PLLSettingProc);
		Restore_Mode();
		return(TRUE);
		
	case IDM_IFORM :
		Save_Mode();
		DialogBox(hInst,"IFORMINFO",hWnd,(DLGPROC)IFormSettingProc);
		Restore_Mode();
		return(TRUE);
		
	case IDM_DONT_TOUCH: 
		if ( USETUNER == TRUE ) {
			USETUNER=FALSE;
			CheckMenuItem( GetMenu(hWnd), IDM_DONT_TOUCH, MF_CHECKED );
							 }  else {
			USETUNER=TRUE;
			CheckMenuItem( GetMenu(hWnd), IDM_DONT_TOUCH, MF_UNCHECKED );
							 }
							 return(TRUE);
							 
	case IDM_DONT_TOUCH_CARD: 
		if ( USECARD == TRUE ) {
			USECARD=FALSE;
			CheckMenuItem( GetMenu(hWnd), IDM_DONT_TOUCH_CARD, MF_CHECKED );
							 }  else {
			USECARD=TRUE;
			CheckMenuItem( GetMenu(hWnd), IDM_DONT_TOUCH_CARD, MF_UNCHECKED );
							 }
							 return(TRUE);
							 
							 
	case IDM_MANUELL_TUNER:
		
		Save_Mode();
		if ( Current_Mode == 0 ) DialogBox(hInst,"TUNERSETTINGS",hWnd,(DLGPROC)TunerSettingProc);
		else  if ( Current_Mode == 1 ) DialogBox(hInst,"TUNERSETTINGSDIGI",hWnd,(DLGPROC)TunerDigiSettingProc);
		Restore_Mode();
		return(TRUE);
		
	case IDM_CAPTURE_DVB_VT:
		if ( DVB_VT == TRUE ) {
			DVB_VT = FALSE; 
			CheckMenuItem( GetMenu(hWnd), IDM_CAPTURE_DVB_VT, MF_UNCHECKED );
			EnableMenuItem(GetMenu(hWnd),IDM_CALL_VIDEOTEXT,MF_GRAYED);
			EnableMenuItem(GetMenu(hWnd),IDM_VT_OUT,MF_GRAYED);
			if (( DVB_VT == FALSE )  && ( DVB_WINBIS == FALSE ) && (VT_Stream_ID != -1 )) Stop_VT_Stream();
			VT_ChannelChange();	
			return(TRUE);
		};
		CheckMenuItem( GetMenu(hWnd), IDM_CAPTURE_DVB_VT, MF_CHECKED );
		EnableMenuItem(GetMenu(hWnd),IDM_CALL_VIDEOTEXT,MF_ENABLED);
		EnableMenuItem(GetMenu(hWnd),IDM_VT_OUT,MF_ENABLED);
		DVB_VT = TRUE; 
		VT_ChannelChange();	
		VT_Stream_ID=Start_VT_Stream(Programm[CurrentProgramm].TeleText_pid);
		return(TRUE);
		
		
	case IDM_CAPTURE_DVB_WINBIS:
		
		if ( DVB_WINBIS == TRUE ) {
			DVB_WINBIS = FALSE; 
			Winbis_Exit();
			CheckMenuItem( GetMenu(hWnd), IDM_CAPTURE_DVB_WINBIS, MF_UNCHECKED );
			EnableMenuItem(GetMenu(hWnd),IDM_WINBIS_OUT,MF_GRAYED);
			if (( DVB_VT == FALSE ) && ( DVB_WINBIS == FALSE ) && (VT_Stream_ID != -1 )) Stop_VT_Stream();
			return(TRUE);
		};
		CheckMenuItem( GetMenu(hWnd), IDM_CAPTURE_DVB_WINBIS, MF_CHECKED );
		EnableMenuItem(GetMenu(hWnd),IDM_WINBIS_OUT,MF_ENABLED);
		Init_Winbis();
		DVB_WINBIS = TRUE; 
		VT_Stream_ID=Start_VT_Stream(Programm[CurrentProgramm].TeleText_pid);
		return(TRUE);
		
		
	case IDM_CAPTURE_TID :
		if ( Capture_TID == TRUE ) {
			Capture_TID = FALSE; 
			CheckMenuItem( GetMenu(hWnd), IDM_CAPTURE_TID, MF_UNCHECKED );
			Stop_TID_Filters();
			return(TRUE);
		};
		Capture_TID = TRUE; 
		CheckMenuItem( GetMenu(hWnd), IDM_CAPTURE_TID, MF_CHECKED );
		Start_TID_Filters();
		return(TRUE);
		
	case IDM_CAPTURE_EIT: 
		if ( Capture_Event_EIT == TRUE ) {
			Capture_Event_EIT = FALSE; 
			CheckMenuItem( GetMenu(hWnd), IDM_CAPTURE_EIT, MF_UNCHECKED );
			EnableMenuItem(GetMenu(hWnd),IDM_EVENT_INFO,MF_GRAYED);
			Stop_Eit_Filters();
			return(TRUE);
		};
		Capture_Event_EIT = TRUE; 
		CheckMenuItem( GetMenu(hWnd), IDM_CAPTURE_EIT, MF_CHECKED );
		EnableMenuItem(GetMenu(hWnd),IDM_EVENT_INFO,MF_ENABLED);
		Start_Eit_Filters();
		return(TRUE);
		
	case IDM_VBI_VT:
		if (VBI_Flags&VBI_VT) {
			VBI_Flags-=VBI_VT;
			CheckMenuItem( GetMenu(hWnd), IDM_VBI_VT, MF_UNCHECKED );
			EnableMenuItem(GetMenu(hWnd),IDM_VT_OUT,MF_GRAYED);
			EnableMenuItem(GetMenu(hWnd),IDM_CALL_VIDEOTEXT,MF_GRAYED);
			EnableMenuItem(GetMenu(hWnd),IDM_VT_RESET,MF_GRAYED);
			EnableMenuItem(GetMenu(hWnd),IDM_UNTERTITEL,MF_GRAYED);
			EnableMenuItem(GetMenu(hWnd),IDM_PDC_OUT,MF_GRAYED);
			EnableMenuItem(GetMenu(hWnd),IDM_VT_OUT,MF_GRAYED);
			return(TRUE);
						  };
		VBI_Flags+=VBI_VT;
		CheckMenuItem( GetMenu(hWnd), IDM_VBI_VT, MF_CHECKED );
		EnableMenuItem(GetMenu(hWnd),IDM_CALL_VIDEOTEXT,MF_ENABLED);
		EnableMenuItem(GetMenu(hWnd),IDM_VT_RESET,MF_ENABLED);
		EnableMenuItem(GetMenu(hWnd),IDM_UNTERTITEL,MF_ENABLED);
		EnableMenuItem(GetMenu(hWnd),IDM_PDC_OUT,MF_ENABLED);
		EnableMenuItem(GetMenu(hWnd),IDM_VT_OUT,MF_ENABLED);
		return(TRUE);
		
	case IDM_VBI_VD:
		if (VBI_Flags&VBI_VD) {
			if ( BeforeVD != 7 ) {
				Set_Capture(4);
				Sleep(30);
				VBI_Flags-=VBI_VD;
				VideoDat_Exit();
				CheckMenuItem( GetMenu(hWnd), IDM_VBI_VD, MF_UNCHECKED );
				VBI_lpf=16;
				MakeVBITable(VBI_lpf);
				MakeVBITable2(VBI_lpf);
				MakeVBITable3(VBI_lpf);
				MakeVBITable4(VBI_lpf);
				MakeVBITable5(VBI_lpf);
				CheckMenuItem( GetMenu(hWnd), TVTYPE+1120, MF_UNCHECKED );
				TVTYPE=BeforeVD;
				S_TVTYPE=0;
				CheckMenuItem( GetMenu(hWnd), TVTYPE+1120, MF_CHECKED );
				EnableMenuItem(GetMenu(hWnd),IDM_VD_OUT,MF_GRAYED);
				Init_Screen_Struct();
				Set_Capture(5);
				return(TRUE);
			};
			VBI_Flags-=VBI_VD;
			CheckMenuItem( GetMenu(hWnd), IDM_VBI_VD, MF_UNCHECKED );
			EnableMenuItem(GetMenu(hWnd),IDM_VD_OUT,MF_GRAYED);
			return(TRUE);
						  };
		
		
		BeforeVD=TVTYPE;
		if ( TVTYPE != 7 ) {
			Set_Capture(4);
			Sleep(30);
			VideoDat_Init();
			VBI_Flags+=VBI_VD;        // 
			CheckMenuItem( GetMenu(hWnd), IDM_VBI_VD, MF_CHECKED );
			VBI_lpf=19;
			MakeVBITable(VBI_lpf);
			MakeVBITable2(VBI_lpf);
			MakeVBITable3(VBI_lpf);
			MakeVBITable4(VBI_lpf);
			MakeVBITable5(VBI_lpf);
			CheckMenuItem( GetMenu(hWnd), TVTYPE+1120, MF_UNCHECKED );
			TVTYPE=7;
					       S_TVTYPE=0;
						   CheckMenuItem( GetMenu(hWnd), TVTYPE+1120, MF_CHECKED );
						   EnableMenuItem(GetMenu(hWnd),IDM_VD_OUT,MF_ENABLED);
                           Init_Screen_Struct();
						   Set_Capture(5);
                           return(TRUE);
		};
		VideoDat_Init();	
		VBI_Flags+=VBI_VD;         
		
		CheckMenuItem( GetMenu(hWnd), IDM_VBI_VD, MF_CHECKED );
		EnableMenuItem(GetMenu(hWnd),IDM_VD_OUT,MF_ENABLED);
		return(TRUE);
		
	case IDM_VBI_VPS:
		if (VBI_Flags&VBI_VPS) {
			VBI_Flags-=VBI_VPS;
			CheckMenuItem( GetMenu(hWnd), IDM_VBI_VPS, MF_UNCHECKED );
			EnableMenuItem(GetMenu(hWnd),IDM_VPS_OUT,MF_GRAYED);
			return(TRUE);
						  };
		VBI_Flags+=VBI_VPS;
		CheckMenuItem( GetMenu(hWnd), IDM_VBI_VPS, MF_CHECKED );
		EnableMenuItem(GetMenu(hWnd),IDM_VPS_OUT,MF_ENABLED);
		return(TRUE);
		
	case IDM_VBI_VC:
		if (VBI_Flags&VBI_VC) {
			VBI_Flags-=VBI_VC;
			CheckMenuItem( GetMenu(hWnd), IDM_VBI_VC, MF_UNCHECKED );
			EnableMenuItem(GetMenu(hWnd),IDM_CRYPTSTATUS,MF_GRAYED);
			return(TRUE);
						  };
		VBI_Flags+=VBI_VC;
		CheckMenuItem( GetMenu(hWnd), IDM_VBI_VC, MF_CHECKED );
		EnableMenuItem(GetMenu(hWnd),IDM_CRYPTSTATUS,MF_ENABLED);
		return(TRUE);
		
	case IDM_VBI_WINBIS:
		if (VBI_Flags&VBI_WINBIS) {
			VBI_Flags-=VBI_WINBIS;
			CheckMenuItem( GetMenu(hWnd), IDM_VBI_WINBIS, MF_UNCHECKED );
			EnableMenuItem(GetMenu(hWnd),IDM_WINBIS_OUT,MF_GRAYED);
			Winbis_Exit();
			return(TRUE);
						  };
						  Init_Winbis();
						  VBI_Flags+=VBI_WINBIS;
						  CheckMenuItem( GetMenu(hWnd), IDM_VBI_WINBIS, MF_CHECKED );
						  EnableMenuItem(GetMenu(hWnd),IDM_WINBIS_OUT,MF_ENABLED);
						  return(TRUE);
						  
	case IDM_UNTERTITEL:
		Save_Mode();
						  VThWnd=CreateDialog(hInst,"VIDEOTEXTUNTERTITEL",NULL,VideoTextUnterTitelProc);
						  if (( ALLWAYS_TOP==TRUE) || (WindowStyle != 0 )) SetWindowPos(VThWnd,HWND_TOPMOST,10,10,20,20,SWP_NOMOVE|SWP_NOCOPYBITS|SWP_NOSIZE);
						  return(TRUE);
						  
	case IDM_VIDEODAT_SETUP:
		Save_Mode();
						  DialogBox(hInst,"VDSETUP",hWnd,VDSettingProc);
						  Restore_Mode();
						  return(TRUE);
						  
	case IDM_VT_SETUP:
		Save_Mode();
						  DialogBox(hInst,"VTSETUP",hWnd,VTSettingProc);
						  Restore_Mode();
						  return(TRUE);
						  
	case IDM_WINBIS_SETUP:
		Save_Mode();
						  DialogBox(hInst,"WINBISSETUP",hWnd,WinBisSettingProc);
						  Restore_Mode();
                          return(TRUE);
						  
						  
	case IDM_AUTODECODE_SETTUNG:
		Save_Mode();
						  DialogBox(hInst,"AUTODECODESETTING",hWnd,DetectSettingProc);
						  Restore_Mode();
						  return(TRUE);
						  
	case IDM_TOGGLE_SETTUNGS:
		Save_Mode();
						  DialogBox(hInst,"TOGGLESETTING",hWnd,ToggleSettingProc);
						  Restore_Mode();
						  return(TRUE);
						  
	case IDM_CALL_VIDEOTEXT:
						  Save_Mode();
						  VThWnd=CreateDialog(hInst,"VIDEOTEXT",NULL,VideoTextProc);
						  if (( ALLWAYS_TOP==TRUE) || (WindowStyle != 0 ))  SetWindowPos(VThWnd,HWND_TOPMOST,10,10,20,20,SWP_NOMOVE|SWP_NOCOPYBITS|SWP_NOSIZE);
						  return(TRUE);
	case IDM_VT_RESET:
		VT_ChannelChange();
		return(TRUE);
		
	case IDM_SECAM_THRESHOLD:
		Save_Mode();
						  DialogBox(hInst,"SECAMTHRESHOLD",hWnd,SecamSettingProc);
						  Restore_Mode();
						  return(TRUE);
						  
	case IDM_DECODERSTATUS:
		Save_Mode();
						  DialogBox(hInst,"DECODERSTATUS",hWnd,DecoderStatusProc);
						  Restore_Mode();
						  return(TRUE);
						  
	case IDM_TIMER_REC:
		Save_Mode();
						  DialogBox(hInst,"TIMERRECORDER",hWnd,TimerRecSettingProc);
						  Restore_Mode();
						  return(TRUE);
						  
	case IDM_RECORD_SETTINGS:
		Save_Mode();
						  DialogBox(hInst,"RECORDERSETTINGS",hWnd,RecorderSettingProc);
						  Restore_Mode();
						  return(TRUE);
						  
	case IDM_DIREKT_RECORD_START:
		if ( Play == FALSE ) {
               if ( Record == TRUE ) Debi_Record_Stop();
			    else Debi_Record_Start(RecorderFormat,RecorderTyp);
		};

		return(TRUE);
	case IDM_RECORD_PLAY:
		if ( Record == FALSE )  {
			Save_Mode();
			DialogBox(hInst,"PLAYCONTROL",hWnd,PlayControlDlg);
			Restore_Mode();
		}
		return(TRUE);
		
	case IDM_PLAY_INFO_CONTROL:
		if (( Play == TRUE ) || ( Record == TRUE )) {
			Save_Mode();
			DialogBox(hInst,"PLAYINFOCONTROL",hWnd,PlayInfoControl);
			Restore_Mode();
		};
		return(TRUE);
		
	case IDM_PLAY_MARK_CONTROL:
		if ( Play == TRUE ) {
			Save_Mode();
			DialogBox(hInst,"PLAYMARKCONTROL",hWnd,PlayMarkControl);
			Restore_Mode();
		};
		return(TRUE);
		
	case IDM_PLAY_TIME_CONTROL:
		if ( Play == TRUE ) {
			Save_Mode();
			DialogBox(hInst,"PLAYTIMECONTROL",hWnd,PlayTimeControl);
			Restore_Mode();
		};
		return(TRUE);
		
	case IDM_RECORD_START:
		Save_Mode();
						  DialogBox(hInst,"RECORDCONTROL",hWnd,RecordControlDlg);
						  Restore_Mode();
						  return(TRUE);
						  
	case IDM_LNB:
		Save_Mode();
		if ( Is_Sat_Karte() == TRUE ) DialogBox(hInst,"LNBSETTING",hWnd,LNBSettingProc);
		else if (Is_Kabel_Karte() == TRUE ) DialogBox(hInst,"LNBSETTINGCABLE",hWnd,LNBSettingProcCable);
						  Restore_Mode();
						  return(TRUE);
						  
	case IDM_AUDIOSETTINGS:
		
		Save_Mode();
						  if ( Current_Mode == 0 ) DialogBox(hInst,"AUDIOEINSTELLUNGEN",hWnd,AudioSettingProc);
						  else if ( Current_Mode == 1 ) DialogBox(hInst,"AUDIOEINSTELLUNGENDIGI",hWnd,AudioDigiSettingProc);
						  Restore_Mode();
						  return(TRUE);
						  
	case IDM_REMOTECONTROL:
		Save_Mode();
						  DialogBox(hInst,"REMOTECONTROLSETTING",hWnd,RemoteControlSettingProc);
						  Restore_Mode();
						  return(TRUE);
						  
	case IDM_AUDIOSETTINGS1:
		Save_Mode();
						  DialogBox(hInst,"AUDIOEINSTELLUNGEN1",hWnd,AudioSettingProc1);
						  Restore_Mode();
						  return(TRUE);
						  
	case IDM_VIDEOSETTINGS:
		Save_Mode();
						  if ( Current_Mode == 0 ) DialogBox(hInst,"VIDEOEINSTELLUNGEN",hWnd,VideoSettingProc);
						  else  if ( Current_Mode == 1 ) DialogBox(hInst,"VIDEOEINSTELLUNGENDIGI",hWnd,VideoSettingDigiProc);
                          Restore_Mode();
						  return(TRUE);
						  
	case IDM_CRYPTSTATUS:
		Save_Mode();
						  DialogBox(hInst,"CRYPTSTATUS",hWnd,CryptModeProc);
                          Restore_Mode();
						  return(TRUE);
						  
	case IDM_PDC_OUT:
		Save_Mode();
						  DialogBox(hInst,"PDCSTATUS",hWnd,PDCProc);
                          Restore_Mode();
						  return(TRUE);
	case IDM_VPS_OUT:
		Save_Mode();
						  DialogBox(hInst,"VPSSTATUS",hWnd,VPSInfoProc);
                          Restore_Mode();
                          return(TRUE);
						  
	case IDM_MENU_SLOT_1: 
		CI_Menu_Slot=1;
		Save_Mode();
						  DialogBox(hInst,"CIMENU",hWnd,CIMenuProc);
                          Restore_Mode();
                          return(TRUE);
						  
	case IDM_MENU_SLOT_2: 
		CI_Menu_Slot=2;
		Save_Mode();
						  DialogBox(hInst,"CIMENU",hWnd,CIMenuProc);
                          Restore_Mode();
                          return(TRUE);
						  
	case IDM_PID_LOG:
		Save_Mode();
						  DialogBox(hInst,"STREAMFILTER",hWnd,StreamFilterSetup);
                          Restore_Mode();
                          return(TRUE);
						  
	case IDM_FILTERLISTE:
		Save_Mode();
						  DialogBox(hInst,"FILTERLISTE",hWnd,FilterListe);
                          Restore_Mode();
                          return(TRUE);
						  
						  
						  
						  
	case IDM_CI_RESET:
		CI_Reset();
		return(TRUE);
	case IDM_CA_INFO:
		Save_Mode();
						  DialogBox(hInst,"CALISTE",hWnd,CAListe);
                          Restore_Mode();
                          return(TRUE);
						  
	case IDM_CI_DIALOG:
		Save_Mode();
						  DialogBox(hInst,"CISTATUS",hWnd,CIStatusProc);
                          Restore_Mode();
                          return(TRUE);
						  
	case IDM_WINBIS_OUT:
		Save_Mode();
						  DialogBox(hInst,"WINBISSTATUS",hWnd,WINBISInfoProc);
                          Restore_Mode();
                          return(TRUE);
						  
						  
						  
	case IDM_VD_OUT:
		Save_Mode();
						  if ( VD_RAW==TRUE ) DialogBox(hInst,"VDSTATUSRAW",hWnd,VDInfoProcRaw);
						  else DialogBox(hInst,"VDSTATUS",hWnd,VDInfoProc);
						  
                          Restore_Mode();
                          return(TRUE);
						  
	case IDM_VT_OUT:
		Save_Mode();
						  DialogBox(hInst,"VTSTATUS",hWnd,VTInfoProc);
                          Restore_Mode();
                          return(TRUE);
						  
						  
	case IDM_AUTOMB:
		if (Auto_MovieBoost == FALSE ) {
			Auto_MovieBoost = TRUE;
			CheckMenuItem( GetMenu(hWnd), IDM_AUTOMB, MF_CHECKED );
		} else {
			Auto_MovieBoost = FALSE;
			CheckMenuItem( GetMenu(hWnd), IDM_AUTOMB, MF_UNCHECKED );
		}
		return(TRUE);
		
		
	case IDM_AUTODETECT:
		if ( Decode_AutoDetect == FALSE ) {
			Decode_AutoDetect = TRUE ;
			CheckMenuItem( GetMenu(hWnd), IDM_AUTODETECT, MF_CHECKED );
		} else {
			Decode_AutoDetect = FALSE;
			CheckMenuItem( GetMenu(hWnd), IDM_AUTODETECT, MF_UNCHECKED );
		}
		return(TRUE);
		
	case IDM_VBI:
		if ( Capture_VBI == FALSE ) {
			Start_VBI();
			Set_Capture(2);
			EnableMenuItem(GetMenu(hWnd),IDM_VBI_VT,MF_ENABLED);
			EnableMenuItem(GetMenu(hWnd),IDM_VBI_WINBIS,MF_ENABLED);
			if (VBI_Flags&VBI_WINBIS) {
				EnableMenuItem(GetMenu(hWnd),IDM_WINBIS_OUT,MF_ENABLED);
			}
			
			EnableMenuItem(GetMenu(hWnd),IDM_VBI_VC,MF_ENABLED);
			if (VBI_Flags&VBI_VC) {
				EnableMenuItem(GetMenu(hWnd),IDM_CRYPTSTATUS,MF_ENABLED);
			}
			
			EnableMenuItem(GetMenu(hWnd),IDM_VBI_VPS,MF_ENABLED);
			if (VBI_Flags&VBI_VPS) {
				EnableMenuItem(GetMenu(hWnd),IDM_VPS_OUT,MF_ENABLED);
			}
			
			EnableMenuItem(GetMenu(hWnd),IDM_VBI_VD,MF_ENABLED);
			
			if (VBI_Flags&VBI_VT) {
				EnableMenuItem(GetMenu(hWnd),IDM_CALL_VIDEOTEXT,MF_ENABLED);
				EnableMenuItem(GetMenu(hWnd),IDM_VT_RESET,MF_ENABLED);
				EnableMenuItem(GetMenu(hWnd),IDM_UNTERTITEL,MF_ENABLED);
				EnableMenuItem(GetMenu(hWnd),IDM_PDC_OUT,MF_ENABLED);
				EnableMenuItem(GetMenu(hWnd),IDM_VT_OUT,MF_ENABLED);
			}
			
			EnableMenuItem(GetMenu(hWnd),IDM_AUTODETECT,MF_ENABLED);
			Capture_VBI = TRUE ;
			CheckMenuItem( GetMenu(hWnd), IDM_VBI, MF_CHECKED );
		} else {
			Capture_VBI = FALSE;
			Stop_Thread();
			Stop_VBI();
			Set_Capture(4);
			EnableMenuItem(GetMenu(hWnd),IDM_VBI_VT,MF_GRAYED);
			EnableMenuItem(GetMenu(hWnd),IDM_VBI_WINBIS,MF_GRAYED);
			EnableMenuItem(GetMenu(hWnd),IDM_VBI_VC,MF_GRAYED);
			EnableMenuItem(GetMenu(hWnd),IDM_VBI_VPS,MF_GRAYED);
			EnableMenuItem(GetMenu(hWnd),IDM_VBI_VD,MF_GRAYED);
			EnableMenuItem(GetMenu(hWnd),IDM_CALL_VIDEOTEXT,MF_GRAYED);
			EnableMenuItem(GetMenu(hWnd),IDM_VT_RESET,MF_GRAYED);
			EnableMenuItem(GetMenu(hWnd),IDM_AUTODETECT,MF_GRAYED);
			EnableMenuItem(GetMenu(hWnd),IDM_CRYPTSTATUS,MF_GRAYED);
			CheckMenuItem( GetMenu(hWnd), IDM_VBI, MF_UNCHECKED );
			EnableMenuItem(GetMenu(hWnd),IDM_UNTERTITEL,MF_GRAYED);
			EnableMenuItem(GetMenu(hWnd),IDM_PDC_OUT,MF_GRAYED);
			EnableMenuItem(GetMenu(hWnd),IDM_VT_OUT,MF_GRAYED);
			EnableMenuItem(GetMenu(hWnd),IDM_WINBIS_OUT,MF_GRAYED);
			EnableMenuItem(GetMenu(hWnd),IDM_VPS_OUT,MF_GRAYED);
			Set_Capture(3);
		}
		return(TRUE);
		
	case IDM_CAPTURE_PAUSE:
		if ( Capture_Pause == FALSE ) {
			Capture_Pause = TRUE ;
			Set_Capture(4);
			CheckMenuItem( GetMenu(hWnd), IDM_CAPTURE_PAUSE, MF_CHECKED );
		} else {
			Set_Capture(5);
			Capture_Pause = FALSE;
			CheckMenuItem( GetMenu(hWnd), IDM_CAPTURE_PAUSE, MF_UNCHECKED );
		}
		return(TRUE);
		
	case IDM_RAW_SURFACE:
		if (( WTVBPP != 15  ) && ( WTVBPP != 16 ) ) {
			MessageBox(hWnd,"Kein 16-Bit (15-Bit) HiColor eingestellt","Multi-Decode",MB_ICONSTOP|MB_OK);
			return(TRUE);
		}
		OldMode=IDM_RAW_SURFACE;
		Stop_Thread();
		Set_Capture(4);
		if ( WTVBPP == 15 ) {
			CheckMenuItem( GetMenu(hWnd), ColourFormat+1130, MF_UNCHECKED );
			ColourFormat=3;
			S_ColourFormat= ColourFormat;
			
			CheckMenuItem( GetMenu(hWnd), ColourFormat+1130, MF_CHECKED );
			SetColourFormat(ColourFormat);
		}
		if ( WTVBPP == 16 ) {
			CheckMenuItem( GetMenu(hWnd), ColourFormat+1130, MF_UNCHECKED );
			ColourFormat=2;
			S_ColourFormat= ColourFormat;
			CheckMenuItem( GetMenu(hWnd), ColourFormat+1130, MF_CHECKED );
			SetColourFormat(ColourFormat);
		}
		Flip=0;
		ModeOverlay=FALSE;
		ModeRAWOverlay=FALSE;
		ModeRAWSurface=TRUE;
		OutDib=FALSE;
		CheckMenuItem( GetMenu(hWnd), IDM_OVERLAY, MF_UNCHECKED );
		CheckMenuItem( GetMenu(hWnd), IDM_INTERN_SURFACE, MF_UNCHECKED );
		CheckMenuItem( GetMenu(hWnd), IDM_DIB, MF_UNCHECKED );
		CheckMenuItem( GetMenu(hWnd), IDM_RAW_SURFACE, MF_CHECKED );
		CheckMenuItem( GetMenu(hWnd), IDM_RAW_OVERLAY, MF_UNCHECKED );
		Init_Screen_Struct();
		Set_Capture(5);
		return(TRUE);
		
	case IDM_RAW_OVERLAY:
		if ( !Can_Overlay ) return(TRUE);
		OldMode=IDM_RAW_OVERLAY;
		Stop_Thread();
		Set_Capture(4);
		CheckMenuItem( GetMenu(hWnd), ColourFormat+1130, MF_UNCHECKED );
		ColourFormat=4;
		CheckMenuItem( GetMenu(hWnd), ColourFormat+1130, MF_CHECKED );
		SetColourFormat(ColourFormat);
		Flip=0;
		ModeRAWOverlay=TRUE;
		ModeRAWSurface=FALSE;
		OutDib=FALSE;
		CheckMenuItem( GetMenu(hWnd), IDM_OVERLAY, MF_UNCHECKED );
		CheckMenuItem( GetMenu(hWnd), IDM_INTERN_SURFACE, MF_UNCHECKED );
		CheckMenuItem( GetMenu(hWnd), IDM_DIB, MF_UNCHECKED );
		CheckMenuItem( GetMenu(hWnd), IDM_RAW_SURFACE, MF_UNCHECKED );
		CheckMenuItem( GetMenu(hWnd), IDM_RAW_OVERLAY, MF_CHECKED );
		Init_Screen_Struct();
		Set_Capture(5);
		return(TRUE);
		
		
	case IDM_OVERLAY:
		if ( !Can_Overlay )  return(TRUE);
		Set_Capture(4);
		CheckMenuItem( GetMenu(hWnd), ColourFormat+1130, MF_UNCHECKED );
		ColourFormat=4;
		CheckMenuItem( GetMenu(hWnd), ColourFormat+1130, MF_CHECKED );
		OldMode=IDM_OVERLAY;
		ModeOverlay=TRUE;
		ModeRAWSurface=FALSE;
		ModeRAWOverlay=FALSE;
		Stop_Thread();
		Flip=0;
		OutDib=FALSE;
		CheckMenuItem( GetMenu(hWnd), IDM_OVERLAY, MF_CHECKED );
		CheckMenuItem( GetMenu(hWnd), IDM_INTERN_SURFACE, MF_UNCHECKED );
		CheckMenuItem( GetMenu(hWnd), IDM_DIB, MF_UNCHECKED );
		CheckMenuItem( GetMenu(hWnd), IDM_RAW_SURFACE, MF_UNCHECKED );
		CheckMenuItem( GetMenu(hWnd), IDM_RAW_OVERLAY, MF_UNCHECKED );
		Init_Screen_Struct();
		Set_Capture(5);
		return(TRUE);
		
		
	case IDM_INTERN_SURFACE:
		if (( WTVBPP != 15  ) && ( WTVBPP != 16 ) ) {
			MessageBox(hWnd,"Kein 16-Bit (15-Bit) HiColor eingestellt","Multi-Decode",MB_ICONSTOP|MB_OK);
			return(TRUE);
		}
		OldMode=IDM_INTERN_SURFACE;
		Stop_Thread();
		Set_Capture(4);
		if (( ColourFormat == 2 ) || ( ColourFormat == 3 )) {
			if ( WTVBPP == 15 ) {
				CheckMenuItem( GetMenu(hWnd), ColourFormat+1130, MF_UNCHECKED );
				ColourFormat=3;
				S_ColourFormat= ColourFormat;
				CheckMenuItem( GetMenu(hWnd), ColourFormat+1130, MF_CHECKED );
				SetColourFormat(ColourFormat);
			}
			if ( WTVBPP == 16 ) {
				CheckMenuItem( GetMenu(hWnd), ColourFormat+1130, MF_UNCHECKED );
				ColourFormat=2;
				S_ColourFormat= ColourFormat;
				CheckMenuItem( GetMenu(hWnd), ColourFormat+1130, MF_CHECKED );
				SetColourFormat(ColourFormat);
			}
		}
		Flip=0;
		ModeOverlay=FALSE;
		ModeRAWSurface=FALSE;
		ModeRAWOverlay=FALSE;
		OutDib=FALSE;
		CheckMenuItem( GetMenu(hWnd), IDM_OVERLAY, MF_UNCHECKED );
		CheckMenuItem( GetMenu(hWnd), IDM_INTERN_SURFACE, MF_CHECKED );
		CheckMenuItem( GetMenu(hWnd), IDM_DIB, MF_UNCHECKED );
		CheckMenuItem( GetMenu(hWnd), IDM_RAW_SURFACE, MF_UNCHECKED );
		CheckMenuItem( GetMenu(hWnd), IDM_RAW_OVERLAY, MF_UNCHECKED );
		Init_Screen_Struct();
		Set_Capture(5);
		return(TRUE);
		
	case IDM_DIB:
		OldMode=IDM_DIB;
		Stop_Thread();
		Set_Capture(4);
		if ( ColourFormat == 2 ) {
			CheckMenuItem( GetMenu(hWnd), ColourFormat+1130, MF_UNCHECKED );
			ColourFormat=3;
			S_ColourFormat= ColourFormat;
			CheckMenuItem( GetMenu(hWnd), ColourFormat+1130, MF_CHECKED );
			SetColourFormat(ColourFormat);
		}
		Flip=1;
		OutDib=TRUE;
		ModeOverlay=FALSE;
		ModeRAWSurface=FALSE;
		ModeRAWOverlay=FALSE;
		CheckMenuItem( GetMenu(hWnd), IDM_OVERLAY, MF_UNCHECKED );
		CheckMenuItem( GetMenu(hWnd), IDM_INTERN_SURFACE, MF_UNCHECKED );
		CheckMenuItem( GetMenu(hWnd), IDM_DIB, MF_CHECKED );
		CheckMenuItem( GetMenu(hWnd), IDM_RAW_SURFACE, MF_UNCHECKED );
		CheckMenuItem( GetMenu(hWnd), IDM_RAW_OVERLAY, MF_UNCHECKED );
		Init_Screen_Struct();
		Set_Capture(5);
		return(TRUE);
		
	case IDM_DX_OV_STRECH:
		Stop_Thread();
		Set_Capture(4);
						  if ( USE_DX_OV_STRECH == TRUE ) {
							  USE_DX_OV_STRECH=FALSE;
							  CheckMenuItem( GetMenu(hWnd), IDM_DX_OV_STRECH, MF_UNCHECKED );
						  } else {
							  USE_DX_OV_STRECH=TRUE;
							  CheckMenuItem( GetMenu(hWnd), IDM_DX_OV_STRECH, MF_CHECKED );
						  }
						  Init_Screen_Struct();
						  Set_Capture(5);
						  return(TRUE);
						  
	case IDM_COLORMIXER:
		Save_Mode();
						  DialogBox(hInst,"FARBMIXER",hWnd,ColorSettingProc);
						  Restore_Mode();
                          return(TRUE);
						  
						  
	case  IDM_AUDIO_MIXER:
		Save_Mode();
						  DialogBox(hInst,"MIXERSETUP",hWnd,MixerSetupProc);
						  Restore_Mode();
                          return(TRUE);
						  
	case IDM_COLORMIX:
		Stop_Thread();
		Set_Capture(4);
		if ( ColorMix == FALSE ) {
			CheckMenuItem( GetMenu(hWnd), IDM_COLORMIX, MF_CHECKED );
			ColorMix=TRUE;
			Init_Screen_Struct();
			Set_Capture(5);
			return(TRUE);
		} 
		CheckMenuItem( GetMenu(hWnd), IDM_COLORMIX, MF_UNCHECKED );
		ColorMix=FALSE;
		Init_Screen_Struct();
		Set_Capture(5);
		return(TRUE);
		
	case IDM_HALFMODE:
		Stop_Thread();
		Set_Capture(4);
		if ( HalfTVMode == FALSE ) {
			CheckMenuItem( GetMenu(hWnd), IDM_HALFMODE, MF_CHECKED );
			HalfTVMode=TRUE;
			Init_Screen_Struct();
			Set_Capture(5);
			return(TRUE);
		} 
		CheckMenuItem( GetMenu(hWnd), IDM_HALFMODE, MF_UNCHECKED );
		HalfTVMode=FALSE;
		Init_Screen_Struct();
		Set_Capture(5);
		return(TRUE);
		
	case IDM_DX_AUSTAST:
		Stop_Thread();
		Set_Capture(4);
						  if ( WAIT_DX_VBI == TRUE ) {
							  WAIT_DX_VBI=FALSE;
							  CheckMenuItem( GetMenu(hWnd), IDM_DX_AUSTAST, MF_UNCHECKED );
						  } else {
							  WAIT_DX_VBI=TRUE;
							  CheckMenuItem( GetMenu(hWnd), IDM_DX_AUSTAST, MF_CHECKED );
						  }
						  Init_Screen_Struct();
						  Set_Capture(5);
						  return(TRUE);
						  
	case IDM_DX_LOCK:
		Stop_Thread();
		Set_Capture(4);
						  if ( USE_DX_LOCK == TRUE ) {
							  USE_DX_LOCK=FALSE;
							  CheckMenuItem( GetMenu(hWnd), IDM_DX_LOCK, MF_UNCHECKED );
						  } else {
							  USE_DX_LOCK=TRUE;
							  CheckMenuItem( GetMenu(hWnd), IDM_DX_LOCK, MF_CHECKED );
						  }
						  Init_Screen_Struct();
						  Set_Capture(5);
						  return(TRUE);
						  
						  
	case IDM_STATUSBAR:
		if ( Display_Status_Bar == TRUE ) {
			Stop_Thread();
			
			Display_Status_Bar=FALSE;
			Stop_Thread();
			KillTimer(hWnd, 1);
			
			CheckMenuItem( GetMenu(hWnd), IDM_STATUSBAR, MF_UNCHECKED );
			Set_Capture(4);
			ShowWindow(Status_Bar,SW_HIDE);
			Init_Screen_Struct();
			Set_Capture(5);
			return(TRUE);
		};
		Display_Status_Bar=TRUE;
		Stop_Thread();
		SetTimer (hWnd, 1, 2500, NULL);
		CheckMenuItem( GetMenu(hWnd), IDM_STATUSBAR, MF_CHECKED );
		Set_Capture(4);
		ShowWindow(Status_Bar,SW_SHOW);
		Init_Screen_Struct();
		Set_Capture(5);
		return(TRUE);
		
	case IDM_ON_TOP:
		Stop_Thread();
		Set_Capture(4);
		WStyle=GetWindowLong(hWnd,GWL_EXSTYLE);
		if ( ALLWAYS_TOP ) {
							     WStyle=WStyle^8;
								 i=SetWindowLong(hWnd,GWL_EXSTYLE,WStyle);
								 ALLWAYS_TOP = FALSE ;
								 SetWindowPos(hWnd,HWND_NOTOPMOST,10,10,20,20,SWP_NOMOVE|SWP_NOCOPYBITS|SWP_NOSIZE);
                                 CheckMenuItem( GetMenu(hWnd), IDM_ON_TOP, MF_UNCHECKED );
		} else {
							     WStyle=WStyle|8;
								 i=SetWindowLong(hWnd,GWL_EXSTYLE,WStyle);
								 ALLWAYS_TOP = TRUE ;
                                 CheckMenuItem( GetMenu(hWnd), IDM_ON_TOP, MF_CHECKED );
								 SetWindowPos(hWnd,HWND_TOPMOST,10,10,20,20,SWP_NOMOVE|SWP_NOCOPYBITS|SWP_NOSIZE);
								 
		}
		Init_Screen_Struct();
		Set_Capture(5);
		return(TRUE);
		
	case IDM_FENSTER: 
		Stop_Thread();
		Set_Capture(4);
		if ( InitTV(hWnd,FALSE,CurrentX,CurrentY) == TRUE ) {
			if ( ALLWAYS_TOP==FALSE ) {
				WStyle=WStyle^8;
				i=SetWindowLong(hWnd,GWL_EXSTYLE,WStyle);
				SetWindowPos(hWnd,HWND_NOTOPMOST,10,10,20,20,SWP_NOMOVE|SWP_NOCOPYBITS|SWP_NOSIZE);
			} else {
				WStyle=WStyle|8;
				i=SetWindowLong(hWnd,GWL_EXSTYLE,WStyle);
				CheckMenuItem( GetMenu(hWnd), IDM_ON_TOP, MF_CHECKED );
			}
			
			WindowStyle=0;
			CheckMenuItem( GetMenu(hWnd), IDM_FENSTER, MF_CHECKED );
			CheckMenuItem( GetMenu(hWnd), IDM_640x480, MF_UNCHECKED );
			CheckMenuItem( GetMenu(hWnd), IDM_800x600, MF_UNCHECKED );
			CheckMenuItem( GetMenu(hWnd), IDM_OWN_RES, MF_UNCHECKED );
			
		} else  	MessageBox(hWnd,"Fehler beim Einstellen vom Bildschirmodus","Multi-Decode",MB_ICONSTOP|MB_OK);
		Init_Screen_Struct();
		Set_Capture(5);
		return(TRUE);
		
	case IDM_FENSTER768x576: 
		Stop_Thread();
		Set_Capture(4);
		i=768;
		j=576;
		if ( Show_Menu == FALSE ) {
			i+=2*GetSystemMetrics(SM_CXSIZEFRAME);
												j+=GetSystemMetrics(SM_CYMENU)+GetSystemMetrics(SM_CYMENUSIZE)+(2*GetSystemMetrics(SM_CYSIZEFRAME))+2;
												if ( Display_Status_Bar == TRUE ) j+=21;
												
		};
		if ( InitTV(hWnd,FALSE,CurrentX,CurrentY) == TRUE ) {
			if ( ALLWAYS_TOP==FALSE ) {
				SetWindowPos(hWnd,HWND_NOTOPMOST,10,10,i,j,SWP_NOMOVE|SWP_NOCOPYBITS);
			} else {
				SetWindowPos(hWnd,HWND_NOTOPMOST,10,10,i,j,SWP_NOMOVE|SWP_NOCOPYBITS);
			}
			
			WindowStyle=0;
			CheckMenuItem( GetMenu(hWnd), IDM_FENSTER, MF_CHECKED );
			CheckMenuItem( GetMenu(hWnd), IDM_640x480, MF_UNCHECKED );
			CheckMenuItem( GetMenu(hWnd), IDM_800x600, MF_UNCHECKED );
			CheckMenuItem( GetMenu(hWnd), IDM_OWN_RES, MF_UNCHECKED );
			
		} else  	MessageBox(hWnd,"Fehler beim Einstellen vom Bildschirmodus","Multi-Decode",MB_ICONSTOP|MB_OK);
		Init_Screen_Struct();
		Set_Capture(5);
		return(TRUE);
		
	case IDM_640x480:
		Stop_Thread();
		Set_Capture(4);
		SBTOK=BTOK;
		BTOK=FALSE;
		if (  InitTV(hWnd,TRUE,640,480) == TRUE ) {
			MoveWindow(hWnd,0,0,640,480,TRUE);
			WindowStyle=1;
			CheckMenuItem( GetMenu(hWnd), IDM_FENSTER, MF_UNCHECKED );
			CheckMenuItem( GetMenu(hWnd), IDM_640x480, MF_CHECKED );
			CheckMenuItem( GetMenu(hWnd), IDM_800x600, MF_UNCHECKED );
			CheckMenuItem( GetMenu(hWnd), IDM_OWN_RES, MF_UNCHECKED );
			
		} else {
							   	 InitTV(hWnd,FALSE,640,480);
                                 MoveWindow(hWnd,0,0,640,480,TRUE);
								 MessageBox(hWnd,"Fehler beim Einstellen vom Bildschirmodus","Multi-Decode",MB_ICONSTOP|MB_OK);
		}
		BTOK=SBTOK;
		Init_Screen_Struct();
		Set_Capture(5);
		return(TRUE);
		
	case IDM_800x600:
		
		Stop_Thread();
		Set_Capture(4);
		SBTOK=BTOK;
		BTOK=FALSE;
		if (  InitTV(hWnd,TRUE,800,600) == TRUE ) {
			MoveWindow(hWnd,0,0,800,600,TRUE); 
			WindowStyle=2;
			CheckMenuItem( GetMenu(hWnd), IDM_FENSTER, MF_UNCHECKED );
			CheckMenuItem( GetMenu(hWnd), IDM_640x480, MF_UNCHECKED );
			CheckMenuItem( GetMenu(hWnd), IDM_800x600, MF_CHECKED );
			CheckMenuItem( GetMenu(hWnd), IDM_OWN_RES, MF_UNCHECKED );
		} else {
						      InitTV(hWnd,FALSE,640,480);
							  MoveWindow(hWnd,0,0,640,480,TRUE);
		};
		BTOK=SBTOK;
		Init_Screen_Struct();
		Set_Capture(5);
		return(TRUE);
		
	case IDM_OWN_RES:
		if (( Res_X==0 ) || ( Res_Y==0 )) return(TRUE);
		Stop_Thread();
		Set_Capture(4);
		SBTOK=BTOK;
		BTOK=FALSE;
		if (  InitTV(hWnd,TRUE,Res_X,Res_Y) == TRUE ) {
			MoveWindow(hWnd,0,0,Res_X,Res_Y,TRUE);
			WindowStyle=3;
			CheckMenuItem( GetMenu(hWnd), IDM_FENSTER, MF_UNCHECKED );
			CheckMenuItem( GetMenu(hWnd), IDM_640x480, MF_UNCHECKED );
			CheckMenuItem( GetMenu(hWnd), IDM_800x600, MF_UNCHECKED );
			CheckMenuItem( GetMenu(hWnd), IDM_OWN_RES, MF_CHECKED );
			
		} else {
						      InitTV(hWnd,FALSE,640,480);
							  MoveWindow(hWnd,0,0,640,480,TRUE);
		};
		BTOK=SBTOK;
		Init_Screen_Struct();
		Set_Capture(5);
		return(TRUE);
		
	case  IDM_LOW :
		Save_Mode();
		DialogBox(hInst,"LOWVALUE",hWnd,(DLGPROC)LowSettingProc);
		Restore_Mode();
		return(TRUE);
		
	case  IDM_SETUP_OWN_RES :
		Save_Mode();
		DialogBox(hInst,"RESTYPE",hWnd,(DLGPROC)ResSettingProc);
		Restore_Mode();
		return(TRUE);
		
	case IDM_TV_16_9:
						  if ( USE_TV_16_9 == TRUE )
						  {
							  CheckMenuItem( hMenu, IDM_TV_16_9, MF_UNCHECKED );
							  USE_TV_16_9=FALSE;
							  SetMonitor_16_9(FALSE);
							  return(TRUE);
						  }
						  CheckMenuItem( hMenu, IDM_TV_16_9, MF_CHECKED );
						  USE_TV_16_9=TRUE;
						  SetMonitor_16_9(TRUE);
						  return(TRUE);
						  
						  
	case IDM_ANALOGSCAN:
		Save_Mode();
		DialogBox(hInst,"ANALOGSCAN",hWnd,(DLGPROC)AnalogScanProc);
		Restore_Mode();
		return(TRUE);
		
	case IDM_DIGIIMPORT:
		Save_Mode();
		DialogBox(hInst,"DIGIIMPORT",hWnd,(DLGPROC)DigiImport);
		Restore_Mode();
		return(TRUE);
		
	case IDM_EVENT_INFO:
		Save_Mode();
		DialogBox(hInst,"EVENTINFO",hWnd,(DLGPROC)EventListe);
		Restore_Mode();
		return(TRUE);
		
		
	case IDM_DVB_TIME:
		Save_Mode();
		DialogBox(hInst,"DVBTIME",hWnd,(DLGPROC)DVBTimeSettingProc);
		Restore_Mode();
		return(TRUE);
		
	case IDM_KANAL_LISTE:
		Save_Mode();
		DialogBox(hInst,"KANALLISTE",hWnd,(DLGPROC)ProgrammListe);
		Restore_Mode();
		return(TRUE);
		
	case IDM_NIT_LISTE:
		Save_Mode();
		DialogBox(hInst,"NITLISTE",hWnd,(DLGPROC)NitListe);
		Restore_Mode();
		return(TRUE);
		
	case IDM_PROGRAMPARAM:
		Save_Mode();
		EditProgramm=CurrentProgramm;
		DialogBox(hInst,"PROGRAMM",hWnd,(DLGPROC)ProgrammEdit);
		Restore_Mode();
		return(TRUE);
		
		
	case IDM_TREADPRIOR_0:
	case IDM_TREADPRIOR_1:
	case IDM_TREADPRIOR_2:
	case IDM_TREADPRIOR_3:
	case IDM_TREADPRIOR_4:
		CheckMenuItem( GetMenu(hWnd), ThreadClassId+1150, MF_UNCHECKED );
		ThreadClassId=wParam-1150;
		CheckMenuItem( GetMenu(hWnd), ThreadClassId+1150, MF_CHECKED );
		Set_Capture(4);
		Sleep(100);
		Set_Capture(5);
		return(TRUE);
		
		
	case IDM_PRIORCLASS_0:
	case IDM_PRIORCLASS_1:
	case IDM_PRIORCLASS_2:
		CheckMenuItem( GetMenu(hWnd), PriorClassId+1160, MF_UNCHECKED );
		PriorClassId=wParam-1160;
		CheckMenuItem( GetMenu(hWnd), PriorClassId+1160, MF_CHECKED );
		strcpy(Text, "Kann Priorität nicht ändern");
		if ( PriorClassId == 0 ) { 
			if ( SetPriorityClass(GetCurrentProcess(),NORMAL_PRIORITY_CLASS) == TRUE	) strcpy(Text, "Normale Priorität");
		} else if ( PriorClassId == 1 ) { 
			if ( SetPriorityClass(GetCurrentProcess(),HIGH_PRIORITY_CLASS) == TRUE	) strcpy(Text, "Hohe Priorität");
		} else {
			if ( SetPriorityClass(GetCurrentProcess(),REALTIME_PRIORITY_CLASS) == TRUE ) strcpy(Text, "RealTime Priorität");
		}
		SetWindowText(hwndTextField,Text);
		return(TRUE);
		
	case IDM_TUNER_4: 
		if ( Current_Mode == 0 ) {
			CheckMenuItem( GetMenu(hWnd), TunerType+1100, MF_UNCHECKED );
			CheckMenuItem( GetMenu(hWnd), IDM_EXTERN1, MF_UNCHECKED );
			CheckMenuItem( GetMenu(hWnd), IDM_EXTERN2, MF_UNCHECKED );
			CheckMenuItem( GetMenu(hWnd), IDM_EXTERN3, MF_UNCHECKED );
			TunerType=wParam-1100;
			CheckMenuItem( GetMenu(hWnd), TunerType+1100, MF_CHECKED );
			Set_Capture(4);
			VideoSource=0;
			SetVideoSource(VideoSource);
			Set_Capture(5);
			return(TRUE);
		};
		
		if ( Current_Mode == 1 ) {
			CheckMenuItem( GetMenu(hWnd), DVBTunerType+1100, MF_UNCHECKED );
			DVBTunerType=wParam-1100;
			CheckMenuItem( GetMenu(hWnd), DVBTunerType+1100, MF_CHECKED );
			sprintf(Text,"%04d. %s ",Make_ProgNr(CurrentProgramm)+1,Programm[CurrentProgramm].Name);
			if ( Display_Status_Bar == TRUE ) SetWindowText (hwndKeyField,Text);
		}
		return(TRUE);
		
		
		return(TRUE);
		
		
	case IDM_TUNER:
		if ( Current_Mode == 0 ) {
			CheckMenuItem( GetMenu(hWnd), TunerType+1100, MF_UNCHECKED );
			CheckMenuItem( GetMenu(hWnd), IDM_EXTERN1, MF_UNCHECKED );
			CheckMenuItem( GetMenu(hWnd), IDM_EXTERN2, MF_UNCHECKED );
			CheckMenuItem( GetMenu(hWnd), IDM_EXTERN3, MF_UNCHECKED );
			VideoSource=0;
			Set_Capture(4);
			Init_Tuner(TunerType);
			SetVideoSource(VideoSource);
			i=0;
			sprintf(Text,"%04d. %s ",Make_ProgNr(CurrentProgramm)+1,Programm[CurrentProgramm].Name);
			if ( Display_Status_Bar == TRUE ) SetWindowText (hwndKeyField,Text);
			Set_Capture(5);
			CheckMenuItem( GetMenu(hWnd), TunerType+1100, MF_CHECKED );
		};
		return(TRUE);
		
	case IDM_TUNER_0: 
	case IDM_TUNER_1: 
	case IDM_TUNER_2: 
	case IDM_TUNER_3: 
	case IDM_TUNER_5: 
	case IDM_TUNER_6:
	case IDM_TUNER_7:
	case IDM_TUNER_8:
		
		if ( Current_Mode == 0 ) {
			CheckMenuItem( GetMenu(hWnd), TunerType+1100, MF_UNCHECKED );
			CheckMenuItem( GetMenu(hWnd), IDM_EXTERN1, MF_UNCHECKED );
			CheckMenuItem( GetMenu(hWnd), IDM_EXTERN2, MF_UNCHECKED );
			CheckMenuItem( GetMenu(hWnd), IDM_EXTERN3, MF_UNCHECKED );
			
			VideoSource=0;
			TunerType=wParam-1100;
			Set_Capture(4);
			Init_Tuner(TunerType);
			SetVideoSource(VideoSource);
			i=0;
			sprintf(Text,"%04d. %s ",Make_ProgNr(CurrentProgramm)+1,Programm[CurrentProgramm].Name);
			if ( Display_Status_Bar == TRUE ) SetWindowText (hwndKeyField,Text);
			
			Set_Capture(5);
			
			CheckMenuItem( GetMenu(hWnd), TunerType+1100, MF_CHECKED );
		} else if ( Current_Mode == 1 ) {
			CheckMenuItem( GetMenu(hWnd), DVBTunerType+1100, MF_UNCHECKED );
			DVBTunerType=wParam-1100;
			CheckMenuItem( GetMenu(hWnd), DVBTunerType+1100, MF_CHECKED );
			sprintf(Text,"%04d. %s ",Make_ProgNr(CurrentProgramm)+1,Programm[CurrentProgramm].Name);
			if ( Display_Status_Bar == TRUE ) SetWindowText (hwndKeyField,Text);
		}
		return(TRUE);
		
	case IDM_TYPEFORMAT_0:
	case IDM_TYPEFORMAT_1:
	case IDM_TYPEFORMAT_2:
	case IDM_TYPEFORMAT_3:
	case IDM_TYPEFORMAT_4:
	case IDM_TYPEFORMAT_5:
	case IDM_TYPEFORMAT_6:
	case IDM_TYPEFORMAT_7:
		CheckMenuItem( GetMenu(hWnd), TVTYPE+1120, MF_UNCHECKED );
		
		if (( TVTYPE == 7 ) && (wParam-1120 != 7 )) {
			Set_Capture(4);
			VBI_Flags-=VBI_VD;        // STOP VD
			Sleep(30);
			CheckMenuItem( GetMenu(hWnd), IDM_VBI_VD, MF_UNCHECKED );
			VBI_lpf=16;
			MakeVBITable(VBI_lpf);
			MakeVBITable2(VBI_lpf);
			MakeVBITable3(VBI_lpf);
			MakeVBITable4(VBI_lpf);
			MakeVBITable5(VBI_lpf);
			TVTYPE=wParam-1120;
					       S_TVTYPE=TVTYPE;
						   CheckMenuItem( GetMenu(hWnd), TVTYPE+1120, MF_CHECKED );
                           Init_Screen_Struct();
						   Set_Capture(5);
						   return(TRUE);
		};
		
		if (( TVTYPE != 7 ) && (wParam-1120 == 7 )) {
			Set_Capture(4);
			Sleep(30);
			VBI_lpf=19;
			MakeVBITable(VBI_lpf);
			MakeVBITable2(VBI_lpf);
			MakeVBITable3(VBI_lpf);
			MakeVBITable4(VBI_lpf);
			MakeVBITable5(VBI_lpf);
			TVTYPE=wParam-1120;
					       S_TVTYPE=TVTYPE;
						   CheckMenuItem( GetMenu(hWnd), TVTYPE+1120, MF_CHECKED );
                           Init_Screen_Struct();
						   Set_Capture(5);
                           return(TRUE);
		};
		
		TVTYPE=wParam-1120;
		S_TVTYPE=TVTYPE;
		CheckMenuItem( GetMenu(hWnd), TVTYPE+1120, MF_CHECKED );
		Set_Capture(4);
		Init_Screen_Struct();
		Set_Capture(5);
		return(TRUE);
		
	case IDM_COLORFORMAT_0:
	case IDM_COLORFORMAT_1:
	case IDM_COLORFORMAT_2:
	case IDM_COLORFORMAT_3:
	case IDM_COLORFORMAT_4:
	case IDM_COLORFORMAT_5:
	case IDM_COLORFORMAT_6:
	case IDM_COLORFORMAT_7:
	case IDM_COLORFORMAT_8:
	case IDM_COLORFORMAT_9:
	case IDM_COLORFORMAT_10:
		CheckMenuItem( GetMenu(hWnd), ColourFormat+1130, MF_UNCHECKED );
		ColourFormat=wParam-1130;
		Set_Capture(4);
		SetColourFormat(ColourFormat);
		S_ColourFormat= ColourFormat;
		Set_Capture(5);
		CheckMenuItem( GetMenu(hWnd), ColourFormat+1130, MF_CHECKED );
		return(TRUE);
		
		
	default : 
		
		if (( wParam >= 40000 ) && ( wParam < 41000 )) {
			Save_Mode();
			(Send_External_DLL_Menu_Cmd)( wParam );
			Restore_Mode();
		};
		return(TRUE);
          };
		  return(TRUE);
}

void RGBOutThread( void  ) 
{
	
	int   DP;
	int   MBoost;
	DOUBLE YStep,YSum;   
	int i,j;
	int nZeileZiel;
	int CurrentFrame;
	DDSURFACEDESC	ddsd;
	HRESULT		    ddrval;
	
	
	ProzessorMask=1<<(AusgabeProzessor);
	i=SetThreadAffinityMask (GetCurrentThread(),ProzessorMask);         
	
	
	if ( ThreadClassId == 0 )  SetThreadPriority(GetCurrentThread(),THREAD_PRIORITY_BELOW_NORMAL);
	else if ( ThreadClassId == 1 ) SetThreadPriority(GetCurrentThread(),THREAD_PRIORITY_NORMAL);
	else if ( ThreadClassId == 2 ) SetThreadPriority(GetCurrentThread(),THREAD_PRIORITY_ABOVE_NORMAL);
	else if ( ThreadClassId == 3 ) SetThreadPriority(GetCurrentThread(),THREAD_PRIORITY_HIGHEST);
	else if ( ThreadClassId == 4 ) SetThreadPriority(GetCurrentThread(),THREAD_PRIORITY_TIME_CRITICAL);
	
	ddsd.dwSize=sizeof( ddsd );
	
	dwLastCount=GetTickCount();
	nFrame=0;
	for ( j= 0 ; j < 5 ; j++ ) {
		for ( i = 0; i < CurrentY ; i+=2 ) {
			pLinesAddressOdd[(j*CLEARLINES)+(i/2)] = (short *)pDisplay[j]+(i*CurrentX)+CurrentX;
			pLinesAddressEven[(j*CLEARLINES)+(i/2)] = (short *)pDisplay[j]+(i*CurrentX);
		}
	}
	
	
	MBoost=(int)(((double)MovieBoost/288)*CurrentY/2);
	
	YStep=(double)MaxY/(CurrentY-(MovieScale*4));
	YSum=0.0;
	ShowY=0;
	for ( nZeileZiel=0; nZeileZiel < CurrentY/2; nZeileZiel++ ) {
		LinesOutEven[nZeileZiel]=0;
		LinesOutOdd[nZeileZiel]=0;
	}
	
	for ( nZeileZiel=MovieScale; nZeileZiel < CurrentY/2-MovieScale; nZeileZiel++ ) {
		YSum=YSum+YStep;
		if ( YSum < 1 ) {
			LinesOutEven[nZeileZiel]=0;
		} else if ( YSum < 2 ) {
			if (( nZeileZiel > MBoost ) && ( nZeileZiel < CurrentY/2-MBoost )) LinesOutEven[nZeileZiel]=1;
			else LinesOutEven[nZeileZiel]=-1;
			YSum-=1;
			ShowY+=1;
		} else if ( YSum < 3 ) {
			if (( nZeileZiel > MBoost ) && ( nZeileZiel < CurrentY/2-MBoost )) LinesOutEven[nZeileZiel]=2;
			else LinesOutEven[nZeileZiel]=-2;
			
			YSum-=2;
			ShowY+=2;
		} else if ( YSum < 4 ) {
			if (( nZeileZiel > MBoost ) && ( nZeileZiel < CurrentY/2-MBoost )) LinesOutEven[nZeileZiel]=3;
			else LinesOutEven[nZeileZiel]=-3;
			
			YSum-=3;
			ShowY+=3;
		} else {
			if (( nZeileZiel > MBoost ) && ( nZeileZiel < CurrentY/2-MBoost )) LinesOutEven[nZeileZiel]=4;
			else LinesOutEven[nZeileZiel]=-4;
			YSum-=4;
			ShowY+=4;
		};
		
		YSum=YSum+YStep;
		if ( YSum < 1 ) {
			LinesOutOdd[nZeileZiel]=0;
		} else if ( YSum < 2 ) {
			if (( nZeileZiel > MBoost ) && ( nZeileZiel < CurrentY/2-MBoost )) LinesOutOdd[nZeileZiel]=1;
			else LinesOutOdd[nZeileZiel]=-1;
			YSum-=1;
			ShowY+=1;
		} else if ( YSum < 3 ) {
			if (( nZeileZiel > MBoost ) && ( nZeileZiel < CurrentY/2-MBoost )) LinesOutOdd[nZeileZiel]=2;
			else LinesOutOdd[nZeileZiel]=-2;
			
			YSum-=2;
			ShowY+=2;
		} else if ( YSum < 4 ) {
			if (( nZeileZiel > MBoost ) && ( nZeileZiel < CurrentY/2-MBoost )) LinesOutOdd[nZeileZiel]=3;
			else LinesOutOdd[nZeileZiel]=-3;
			
			YSum-=3;
			ShowY+=3;
		} else {
			if (( nZeileZiel > MBoost ) && ( nZeileZiel < CurrentY/2-MBoost )) LinesOutOdd[nZeileZiel]=4;
			else LinesOutOdd[nZeileZiel]=-4;
			YSum-=4;
			ShowY+=4;
		};
	};
	
	
	if ( OutDib == TRUE ) {
		DP=DibPitch/2;
		memset(&pDecode[0],0x00,m_dwDisplaySize+1024);
		j=0;
		for ( nZeileZiel=0; nZeileZiel < CurrentY/2; nZeileZiel++ ) {
			pLinesDstAddressEven[nZeileZiel]=(unsigned short*)&pDecode[j*DibPitch/2];
			if (LinesOutEven[nZeileZiel] > 0 )  j+=LinesOutEven[nZeileZiel];
			else j-=LinesOutEven[nZeileZiel];
			pLinesDstAddressOdd[nZeileZiel]=(unsigned short*)&pDecode[j*DibPitch/2];
			if (LinesOutOdd[nZeileZiel] > 0 )  j+=LinesOutOdd[nZeileZiel];
			else j-=LinesOutOdd[nZeileZiel];
			
		};
		
		
		
		for (;;) {
			nFrame++;
			Get_Thread_Status();
			CurrentFrame=NextFrame-1;
			if ( NextFrame== -1 ) {
				SetEvent(Decode_Event_Stop);
				return;
			};
			
			if ( CurrentFrame < 0 ) CurrentFrame=4;
			
			for ( nZeileZiel=MovieScale; nZeileZiel < CurrentY/2-MovieScale; nZeileZiel++ ) {
				
				if ( LinesOutEven[nZeileZiel] > 0 ) {
					if ( LinesOutEven[nZeileZiel] == 1 ) {
						memcpy(pLinesDstAddressEven[nZeileZiel],pLinesAddressEven[CurrentFrame*CLEARLINES+nZeileZiel], CurrentX*2);
					} else if ( LinesOutEven[nZeileZiel] == 2 ) {
						memcpy(pLinesDstAddressEven[nZeileZiel],pLinesAddressEven[CurrentFrame*CLEARLINES+nZeileZiel], CurrentX*2);
						memcpy((pLinesDstAddressEven[nZeileZiel]+DP),pLinesAddressEven[CurrentFrame*CLEARLINES+nZeileZiel], CurrentX*2);
					} else if ( LinesOutEven[nZeileZiel] == 3 ) {
						memcpy(pLinesDstAddressEven[nZeileZiel],pLinesAddressEven[CurrentFrame*CLEARLINES+nZeileZiel], CurrentX*2);
						memcpy((pLinesDstAddressEven[nZeileZiel]+DP),pLinesAddressEven[CurrentFrame*CLEARLINES+nZeileZiel], CurrentX*2);
						memcpy((pLinesDstAddressEven[nZeileZiel]+DP*2),pLinesAddressEven[CurrentFrame*CLEARLINES+nZeileZiel], CurrentX*2);
					} else {
						memcpy(pLinesDstAddressEven[nZeileZiel],pLinesAddressEven[CurrentFrame*CLEARLINES+nZeileZiel], CurrentX*2);
						memcpy((pLinesDstAddressEven[nZeileZiel]+DP),pLinesAddressEven[CurrentFrame*CLEARLINES+nZeileZiel], CurrentX*2);
						memcpy((pLinesDstAddressEven[nZeileZiel]+DP*2),pLinesAddressEven[CurrentFrame*CLEARLINES+nZeileZiel], CurrentX*2);
						memcpy((pLinesDstAddressEven[nZeileZiel]+DP*3),pLinesAddressEven[CurrentFrame*CLEARLINES+nZeileZiel], CurrentX*2);
					}
				}
				
				if ( LinesOutOdd[nZeileZiel] > 0 ) {
					if ( LinesOutOdd[nZeileZiel] == 1 ) {
						memcpy(pLinesDstAddressOdd[nZeileZiel],pLinesAddressOdd[CurrentFrame*CLEARLINES+nZeileZiel], CurrentX*2);
					} else if ( LinesOutOdd[nZeileZiel] == 2 ) {
						memcpy(pLinesDstAddressOdd[nZeileZiel],pLinesAddressOdd[CurrentFrame*CLEARLINES+nZeileZiel], CurrentX*2);
						memcpy((pLinesDstAddressOdd[nZeileZiel]+DP),pLinesAddressOdd[CurrentFrame*CLEARLINES+nZeileZiel], CurrentX*2);
					} else if ( LinesOutOdd[nZeileZiel] == 3 ) {
						memcpy(pLinesDstAddressOdd[nZeileZiel],pLinesAddressOdd[CurrentFrame*CLEARLINES+nZeileZiel], CurrentX*2);
						memcpy((pLinesDstAddressOdd[nZeileZiel]+DP),pLinesAddressOdd[CurrentFrame*CLEARLINES+nZeileZiel], CurrentX*2);
						memcpy((pLinesDstAddressOdd[nZeileZiel]+DP*2),pLinesAddressOdd[CurrentFrame*CLEARLINES+nZeileZiel], CurrentX*2);
					} else {
						memcpy(pLinesDstAddressOdd[nZeileZiel],pLinesAddressOdd[CurrentFrame*CLEARLINES+nZeileZiel], CurrentX*2);
						memcpy((pLinesDstAddressOdd[nZeileZiel]+DP),pLinesAddressOdd[CurrentFrame*CLEARLINES+nZeileZiel], CurrentX*2);
						memcpy((pLinesDstAddressOdd[nZeileZiel]+DP*2),pLinesAddressOdd[CurrentFrame*CLEARLINES+nZeileZiel], CurrentX*2);
						memcpy((pLinesDstAddressOdd[nZeileZiel]+DP*3),pLinesAddressOdd[CurrentFrame*CLEARLINES+nZeileZiel], CurrentX*2);
					}
				}
			}	
			
			SetDIBitsToDevice( DIB_Hdc,
				0,
				0,
				pDIB->bmiHeader.biWidth,
				pDIB->bmiHeader.biHeight,
				0,
				0,
				0,
				pDIB->bmiHeader.biHeight,
				pDecode,
				pDIB,
				DIB_RGB_COLORS
				);
			
			if ( Display_Status_Bar == TRUE ) {
				if(dwLastCount+960 <= GetTickCount())
				{
					sprintf(Text,"%d FPS",nFrame);
					SetWindowText (hwndFPSField,Text);
					nFrame=0;
					dwLastCount = GetTickCount();
				}
			}
		}
	}
	
	DP=SurfacePitch/2;
	j=0;
	for ( nZeileZiel=0; nZeileZiel < CurrentY/2; nZeileZiel++ ) {
		pLinesDstAddressEven[nZeileZiel]=(unsigned short*)lpSurface+(Offset_for_WindowOut/2)+(j*(SurfacePitch/2));
		if (LinesOutEven[nZeileZiel] > 0 )  j+=LinesOutEven[nZeileZiel];
		else j-=LinesOutEven[nZeileZiel];
		pLinesDstAddressOdd[nZeileZiel]=(unsigned short*)lpSurface+(Offset_for_WindowOut/2)+(j*(SurfacePitch/2));
		if (LinesOutOdd[nZeileZiel] > 0 )  j+=LinesOutOdd[nZeileZiel];
		else j-=LinesOutOdd[nZeileZiel];
	};
	
	if ( WTVBPP==16) {
		
		
		for (;;) {
			nFrame++;
			Get_Thread_Status();
			CurrentFrame=NextFrame-1;
			if ( NextFrame== -1 ) {
				SetEvent(Decode_Event_Stop);
				return;
			};
			
			if ( CurrentFrame < 0 ) CurrentFrame=4;
			if ( WAIT_DX_VBI ) ddrval=IDirectDraw_WaitForVerticalBlank(lpDD,DDWAITVB_BLOCKBEGIN, NULL);
			if ( USE_DX_LOCK ) ddrval=IDirectDrawSurface_Lock(lpDDSurface, NULL, &ddsd, DDLOCK_WAIT, NULL);
			for ( nZeileZiel=MovieScale; nZeileZiel < CurrentY/2-MovieScale; nZeileZiel++ ) {
				
				if ( LinesOutEven[nZeileZiel] > 0 ) {
					if ( LinesOutEven[nZeileZiel] == 1 ) {
						memcpy(pLinesDstAddressEven[nZeileZiel],pLinesAddressEven[CurrentFrame*CLEARLINES+nZeileZiel], CurrentX*2);
					} else if ( LinesOutEven[nZeileZiel] == 2 ) {
						memcpy(pLinesDstAddressEven[nZeileZiel],pLinesAddressEven[CurrentFrame*CLEARLINES+nZeileZiel], CurrentX*2);
						memcpy((pLinesDstAddressEven[nZeileZiel]+DP),pLinesAddressEven[CurrentFrame*CLEARLINES+nZeileZiel], CurrentX*2);
					} else if ( LinesOutEven[nZeileZiel] == 3 ) {
						memcpy(pLinesDstAddressEven[nZeileZiel],pLinesAddressEven[CurrentFrame*CLEARLINES+nZeileZiel], CurrentX*2);
						memcpy((pLinesDstAddressEven[nZeileZiel]+DP),pLinesAddressEven[CurrentFrame*CLEARLINES+nZeileZiel], CurrentX*2);
						memcpy((pLinesDstAddressEven[nZeileZiel]+DP*2),pLinesAddressEven[CurrentFrame*CLEARLINES+nZeileZiel], CurrentX*2);
					} else {
						memcpy(pLinesDstAddressEven[nZeileZiel],pLinesAddressEven[CurrentFrame*CLEARLINES+nZeileZiel], CurrentX*2);
						memcpy((pLinesDstAddressEven[nZeileZiel]+DP),pLinesAddressEven[CurrentFrame*CLEARLINES+nZeileZiel], CurrentX*2);
						memcpy((pLinesDstAddressEven[nZeileZiel]+DP*2),pLinesAddressEven[CurrentFrame*CLEARLINES+nZeileZiel], CurrentX*2);
						memcpy((pLinesDstAddressEven[nZeileZiel]+DP*3),pLinesAddressEven[CurrentFrame*CLEARLINES+nZeileZiel], CurrentX*2);
					}
				}
				
				if ( LinesOutOdd[nZeileZiel] > 0 ) {
					if ( LinesOutOdd[nZeileZiel] == 1 ) {
						memcpy(pLinesDstAddressOdd[nZeileZiel],pLinesAddressOdd[CurrentFrame*CLEARLINES+nZeileZiel], CurrentX*2);
					} else if ( LinesOutOdd[nZeileZiel] == 2 ) {
						memcpy(pLinesDstAddressOdd[nZeileZiel],pLinesAddressOdd[CurrentFrame*CLEARLINES+nZeileZiel], CurrentX*2);
						memcpy((pLinesDstAddressOdd[nZeileZiel]+DP),pLinesAddressOdd[CurrentFrame*CLEARLINES+nZeileZiel], CurrentX*2);
					} else if ( LinesOutOdd[nZeileZiel] == 3 ) {
						memcpy(pLinesDstAddressOdd[nZeileZiel],pLinesAddressOdd[CurrentFrame*CLEARLINES+nZeileZiel], CurrentX*2);
						memcpy((pLinesDstAddressOdd[nZeileZiel]+DP),pLinesAddressOdd[CurrentFrame*CLEARLINES+nZeileZiel], CurrentX*2);
						memcpy((pLinesDstAddressOdd[nZeileZiel]+DP*2),pLinesAddressOdd[CurrentFrame*CLEARLINES+nZeileZiel], CurrentX*2);
					} else {
						memcpy(pLinesDstAddressOdd[nZeileZiel],pLinesAddressOdd[CurrentFrame*CLEARLINES+nZeileZiel], CurrentX*2);
						memcpy((pLinesDstAddressOdd[nZeileZiel]+DP),pLinesAddressOdd[CurrentFrame*CLEARLINES+nZeileZiel], CurrentX*2);
						memcpy((pLinesDstAddressOdd[nZeileZiel]+DP*2),pLinesAddressOdd[CurrentFrame*CLEARLINES+nZeileZiel], CurrentX*2);
						memcpy((pLinesDstAddressOdd[nZeileZiel]+DP*3),pLinesAddressOdd[CurrentFrame*CLEARLINES+nZeileZiel], CurrentX*2);
					}
				}
			}
			
			if ( USE_DX_LOCK ) ddrval=IDirectDrawSurface_Unlock(lpDDSurface,  ddsd.lpSurface);
			
			if ( Display_Status_Bar == TRUE ) {
				if(dwLastCount+960 <= GetTickCount()) {
					
					sprintf(Text,"%d FPS",nFrame);
					SetWindowText (hwndFPSField,Text);
					nFrame=0;
					dwLastCount = GetTickCount();
				}
			}
		}
	}
	
	for (;;) {
		nFrame++;
		Get_Thread_Status();
		CurrentFrame=NextFrame-1;
		if ( NextFrame== -1 ) {
			SetEvent(Decode_Event_Stop);
			return;
		};
		if ( CurrentFrame < 0 ) CurrentFrame=4;
		if ( WAIT_DX_VBI ) ddrval=IDirectDraw_WaitForVerticalBlank(lpDD,DDWAITVB_BLOCKBEGIN, NULL);
		if ( USE_DX_LOCK ) ddrval=IDirectDrawSurface_Lock(lpDDSurface, NULL, &ddsd, DDLOCK_WAIT, NULL);
		for ( nZeileZiel=MovieScale; nZeileZiel < CurrentY/2-MovieScale; nZeileZiel++ ) {
			
			
			if ( LinesOutEven[nZeileZiel] > 0 ) {
				if ( LinesOutEven[nZeileZiel] == 1 ) {
					memcpy(pLinesDstAddressEven[nZeileZiel],pLinesAddressEven[CurrentFrame*CLEARLINES+nZeileZiel], CurrentX*2);
				} else if ( LinesOutEven[nZeileZiel] == 2 ) {
					memcpy(pLinesDstAddressEven[nZeileZiel],pLinesAddressEven[CurrentFrame*CLEARLINES+nZeileZiel], CurrentX*2);
					memcpy((pLinesDstAddressEven[nZeileZiel]+DP),pLinesAddressEven[CurrentFrame*CLEARLINES+nZeileZiel], CurrentX*2);
				} else if ( LinesOutEven[nZeileZiel] == 3 ) {
					memcpy(pLinesDstAddressEven[nZeileZiel],pLinesAddressEven[CurrentFrame*CLEARLINES+nZeileZiel], CurrentX*2);
					memcpy((pLinesDstAddressEven[nZeileZiel]+DP),pLinesAddressEven[CurrentFrame*CLEARLINES+nZeileZiel], CurrentX*2);
					memcpy((pLinesDstAddressEven[nZeileZiel]+DP*2),pLinesAddressEven[CurrentFrame*CLEARLINES+nZeileZiel], CurrentX*2);
				} else {
					memcpy(pLinesDstAddressEven[nZeileZiel],pLinesAddressEven[CurrentFrame*CLEARLINES+nZeileZiel], CurrentX*2);
					memcpy((pLinesDstAddressEven[nZeileZiel]+DP),pLinesAddressEven[CurrentFrame*CLEARLINES+nZeileZiel], CurrentX*2);
					memcpy((pLinesDstAddressEven[nZeileZiel]+DP*2),pLinesAddressEven[CurrentFrame*CLEARLINES+nZeileZiel], CurrentX*2);
					memcpy((pLinesDstAddressEven[nZeileZiel]+DP*3),pLinesAddressEven[CurrentFrame*CLEARLINES+nZeileZiel], CurrentX*2);
				}
			}
			
			if ( LinesOutOdd[nZeileZiel] > 0 ) {
				if ( LinesOutOdd[nZeileZiel] == 1 ) {
					memcpy(pLinesDstAddressOdd[nZeileZiel],pLinesAddressOdd[CurrentFrame*CLEARLINES+nZeileZiel], CurrentX*2);
				} else if ( LinesOutOdd[nZeileZiel] == 2 ) {
					memcpy(pLinesDstAddressOdd[nZeileZiel],pLinesAddressOdd[CurrentFrame*CLEARLINES+nZeileZiel], CurrentX*2);
					memcpy((pLinesDstAddressOdd[nZeileZiel]+DP),pLinesAddressOdd[CurrentFrame*CLEARLINES+nZeileZiel], CurrentX*2);
				} else if ( LinesOutOdd[nZeileZiel] == 3 ) {
					memcpy(pLinesDstAddressOdd[nZeileZiel],pLinesAddressOdd[CurrentFrame*CLEARLINES+nZeileZiel], CurrentX*2);
					memcpy((pLinesDstAddressOdd[nZeileZiel]+DP),pLinesAddressOdd[CurrentFrame*CLEARLINES+nZeileZiel], CurrentX*2);
					memcpy((pLinesDstAddressOdd[nZeileZiel]+DP*2),pLinesAddressOdd[CurrentFrame*CLEARLINES+nZeileZiel], CurrentX*2);
				} else {
					memcpy(pLinesDstAddressOdd[nZeileZiel],pLinesAddressOdd[CurrentFrame*CLEARLINES+nZeileZiel], CurrentX*2);
					memcpy((pLinesDstAddressOdd[nZeileZiel]+DP),pLinesAddressOdd[CurrentFrame*CLEARLINES+nZeileZiel], CurrentX*2);
					memcpy((pLinesDstAddressOdd[nZeileZiel]+DP*2),pLinesAddressOdd[CurrentFrame*CLEARLINES+nZeileZiel], CurrentX*2);
					memcpy((pLinesDstAddressOdd[nZeileZiel]+DP*3),pLinesAddressOdd[CurrentFrame*CLEARLINES+nZeileZiel], CurrentX*2);
				}
			}
			
		}	
		if ( USE_DX_LOCK ) ddrval=IDirectDrawSurface_Unlock(lpDDSurface,  ddsd.lpSurface);
		
		if ( Display_Status_Bar == TRUE  ) {
			if(dwLastCount+960 <= GetTickCount()) {
				sprintf(Text,"%d FPS",nFrame);
				SetWindowText (hwndFPSField,Text);
				nFrame=0;
				dwLastCount = GetTickCount();
			}
		}
	}
	
}


void RGBOutThreadHalf( void  ) 
{
	
	
	int   DP;
	int   MBoost;
	DOUBLE YStep,YSum;   
	int i,j;
	int nZeileZiel;
	int CurrentFrame;
	DDSURFACEDESC	ddsd;
	HRESULT		    ddrval;
	
	ProzessorMask=1<<(AusgabeProzessor);
	i=SetThreadAffinityMask (GetCurrentThread(),ProzessorMask);         
	
	
	if ( ThreadClassId == 0 )  SetThreadPriority(GetCurrentThread(),THREAD_PRIORITY_BELOW_NORMAL);
	else if ( ThreadClassId == 1 ) SetThreadPriority(GetCurrentThread(),THREAD_PRIORITY_NORMAL);
	else if ( ThreadClassId == 2 ) SetThreadPriority(GetCurrentThread(),THREAD_PRIORITY_ABOVE_NORMAL);
	else if ( ThreadClassId == 3 ) SetThreadPriority(GetCurrentThread(),THREAD_PRIORITY_HIGHEST);
	else if ( ThreadClassId == 4 ) SetThreadPriority(GetCurrentThread(),THREAD_PRIORITY_TIME_CRITICAL);
	
	ddsd.dwSize=sizeof( ddsd );
	
	dwLastCount=GetTickCount();
	nFrame=0;
	for ( j= 0 ; j < 5 ; j++ ) {
		for ( i = 0; i < 288 ; i++) {
			pLinesAddressEven[(j*CLEARLINES)+i] = (short *)pDisplay[j]+(i*CurrentX);
		}
	}
	
	
	MBoost=(int)(((double)MovieBoost/288)*CurrentY/2);
	
	YStep=(double)MaxY/((CurrentY/2)-(MovieScale*2));
	YSum=0.0;
	ShowY=0;
	for ( nZeileZiel=0; nZeileZiel < 288; nZeileZiel++ ) LinesOutEven[nZeileZiel]=0;
	
	
	for ( nZeileZiel=MovieScale; nZeileZiel < CurrentY/2-MovieScale; nZeileZiel++ ) {
		YSum=YSum+YStep;
		if ( YSum < 1 ) {
			LinesOutEven[nZeileZiel]=0;
		} else if ( YSum < 2 ) {
			if (( nZeileZiel > MBoost ) && ( nZeileZiel < CurrentY/2-MBoost )) LinesOutEven[nZeileZiel]=1;
			else LinesOutEven[nZeileZiel]=-1;
			YSum-=1;
			ShowY+=1;
		} else if ( YSum < 3 ) {
			if (( nZeileZiel > MBoost ) && ( nZeileZiel < CurrentY/2-MBoost )) LinesOutEven[nZeileZiel]=2;
			else LinesOutEven[nZeileZiel]=-2;
			
			YSum-=2;
			ShowY+=2;
		} else if ( YSum < 4 ) {
			if (( nZeileZiel > MBoost ) && ( nZeileZiel < CurrentY/2-MBoost )) LinesOutEven[nZeileZiel]=3;
			else LinesOutEven[nZeileZiel]=-3;
			
			YSum-=3;
			ShowY+=3;
		} else {
			if (( nZeileZiel > MBoost ) && ( nZeileZiel < CurrentY/2-MBoost )) LinesOutEven[nZeileZiel]=4;
			else LinesOutEven[nZeileZiel]=-4;
			YSum-=4;
			ShowY+=4;
		};
	};
	
	
	if ( OutDib == TRUE ) {
		DP=DibPitch/2;
		memset(&pDecode[0],0x00,m_dwDisplaySize+1024);
		j=0;
		for ( nZeileZiel=0; nZeileZiel < CurrentY/2; nZeileZiel++ ) {
			pLinesDstAddressEven[nZeileZiel]=(unsigned short*)&pDecode[j*DibPitch/2];
			if (LinesOutEven[nZeileZiel] > 0 )  j+=LinesOutEven[nZeileZiel];
			else j-=LinesOutEven[nZeileZiel];
		};
		
		
		
		for (;;) {
			nFrame++;
			Get_Thread_Status();
			CurrentFrame=NextFrame-1;
			if ( NextFrame== -1 ) {
				SetEvent(Decode_Event_Stop);
				return;
			};
			if ( CurrentFrame < 0 ) CurrentFrame=4;
			
			for ( nZeileZiel=MovieScale; nZeileZiel < CurrentY/2-MovieScale; nZeileZiel++ ) {
				
				if ( LinesOutEven[nZeileZiel] > 0 ) {
					if ( LinesOutEven[nZeileZiel] == 1 ) {
						memcpy(pLinesDstAddressEven[nZeileZiel],pLinesAddressEven[CurrentFrame*CLEARLINES+nZeileZiel], CurrentX*2);
					} else if ( LinesOutEven[nZeileZiel] == 2 ) {
						memcpy(pLinesDstAddressEven[nZeileZiel],pLinesAddressEven[CurrentFrame*CLEARLINES+nZeileZiel], CurrentX*2);
						memcpy((pLinesDstAddressEven[nZeileZiel]+DP),pLinesAddressEven[CurrentFrame*CLEARLINES+nZeileZiel], CurrentX*2);
					} else if ( LinesOutEven[nZeileZiel] == 3 ) {
						memcpy(pLinesDstAddressEven[nZeileZiel],pLinesAddressEven[CurrentFrame*CLEARLINES+nZeileZiel], CurrentX*2);
						memcpy((pLinesDstAddressEven[nZeileZiel]+DP),pLinesAddressEven[CurrentFrame*CLEARLINES+nZeileZiel], CurrentX*2);
						memcpy((pLinesDstAddressEven[nZeileZiel]+DP*2),pLinesAddressEven[CurrentFrame*CLEARLINES+nZeileZiel], CurrentX*2);
					} else {
						memcpy(pLinesDstAddressEven[nZeileZiel],pLinesAddressEven[CurrentFrame*CLEARLINES+nZeileZiel], CurrentX*2);
						memcpy((pLinesDstAddressEven[nZeileZiel]+DP),pLinesAddressEven[CurrentFrame*CLEARLINES+nZeileZiel], CurrentX*2);
						memcpy((pLinesDstAddressEven[nZeileZiel]+DP*2),pLinesAddressEven[CurrentFrame*CLEARLINES+nZeileZiel], CurrentX*2);
						memcpy((pLinesDstAddressEven[nZeileZiel]+DP*3),pLinesAddressEven[CurrentFrame*CLEARLINES+nZeileZiel], CurrentX*2);
					}
				}
				
			}	
			
			SetDIBitsToDevice( DIB_Hdc,
				0,
				0,
				pDIB->bmiHeader.biWidth,
				pDIB->bmiHeader.biHeight,
				0,
				0,
				0,
				pDIB->bmiHeader.biHeight,
				pDecode,
				pDIB,
				DIB_RGB_COLORS
				);
			
			if ( Display_Status_Bar == TRUE ) {
				if(dwLastCount+960 <= GetTickCount())
				{
					sprintf(Text,"%d FPS",nFrame);
					SetWindowText (hwndFPSField,Text);
					nFrame=0;
					dwLastCount = GetTickCount();
				}
			}
		}
	}
	
	DP=SurfacePitch/2;
	j=0;
	for ( nZeileZiel=0; nZeileZiel < CurrentY/2; nZeileZiel++ ) {
		pLinesDstAddressEven[nZeileZiel]=(unsigned short*)lpSurface+(Offset_for_WindowOut/2)+(j*(SurfacePitch/2));
		if (LinesOutEven[nZeileZiel] > 0 )  j+=LinesOutEven[nZeileZiel];
		else j-=LinesOutEven[nZeileZiel];
	};
	
	if ( WTVBPP==16) {
		
		
		for (;;) {
			nFrame++;
			Get_Thread_Status();
			CurrentFrame=NextFrame-1;
			if ( NextFrame== -1 ) {
				SetEvent(Decode_Event_Stop);
				return;
			};
			
			if ( CurrentFrame < 0 ) CurrentFrame=4;
			if ( WAIT_DX_VBI ) ddrval=IDirectDraw_WaitForVerticalBlank(lpDD,DDWAITVB_BLOCKBEGIN, NULL);
			if ( USE_DX_LOCK ) ddrval=IDirectDrawSurface_Lock(lpDDSurface, NULL, &ddsd, DDLOCK_WAIT, NULL);
			for ( nZeileZiel=MovieScale; nZeileZiel < CurrentY/2-MovieScale; nZeileZiel++ ) {
				
				if ( LinesOutEven[nZeileZiel] > 0 ) {
					if ( LinesOutEven[nZeileZiel] == 1 ) {
						memcpy(pLinesDstAddressEven[nZeileZiel],pLinesAddressEven[CurrentFrame*CLEARLINES+nZeileZiel], CurrentX*2);
					} else if ( LinesOutEven[nZeileZiel] == 2 ) {
						memcpy(pLinesDstAddressEven[nZeileZiel],pLinesAddressEven[CurrentFrame*CLEARLINES+nZeileZiel], CurrentX*2);
						memcpy((pLinesDstAddressEven[nZeileZiel]+DP),pLinesAddressEven[CurrentFrame*CLEARLINES+nZeileZiel], CurrentX*2);
					} else if ( LinesOutEven[nZeileZiel] == 3 ) {
						memcpy(pLinesDstAddressEven[nZeileZiel],pLinesAddressEven[CurrentFrame*CLEARLINES+nZeileZiel], CurrentX*2);
						memcpy((pLinesDstAddressEven[nZeileZiel]+DP),pLinesAddressEven[CurrentFrame*CLEARLINES+nZeileZiel], CurrentX*2);
						memcpy((pLinesDstAddressEven[nZeileZiel]+DP*2),pLinesAddressEven[CurrentFrame*CLEARLINES+nZeileZiel], CurrentX*2);
					} else {
						memcpy(pLinesDstAddressEven[nZeileZiel],pLinesAddressEven[CurrentFrame*CLEARLINES+nZeileZiel], CurrentX*2);
						memcpy((pLinesDstAddressEven[nZeileZiel]+DP),pLinesAddressEven[CurrentFrame*CLEARLINES+nZeileZiel], CurrentX*2);
						memcpy((pLinesDstAddressEven[nZeileZiel]+DP*2),pLinesAddressEven[CurrentFrame*CLEARLINES+nZeileZiel], CurrentX*2);
						memcpy((pLinesDstAddressEven[nZeileZiel]+DP*3),pLinesAddressEven[CurrentFrame*CLEARLINES+nZeileZiel], CurrentX*2);
					}
				}
				
			}
			
			if ( USE_DX_LOCK ) ddrval=IDirectDrawSurface_Unlock(lpDDSurface,  ddsd.lpSurface);
			
			if ( Display_Status_Bar == TRUE  ) {
				if(dwLastCount+960 <= GetTickCount()) {
					sprintf(Text,"%d FPS",nFrame);
					SetWindowText (hwndFPSField,Text);
					nFrame=0;
					dwLastCount = GetTickCount();
				}
			}
		}
	}
	
	for (;;) {
		nFrame++;
		Get_Thread_Status();
		CurrentFrame=NextFrame-1;
		if ( NextFrame== -1 ) {
			SetEvent(Decode_Event_Stop);
			return;
		};
		if ( CurrentFrame < 0 ) CurrentFrame=4;
		if ( WAIT_DX_VBI ) ddrval=IDirectDraw_WaitForVerticalBlank(lpDD,DDWAITVB_BLOCKBEGIN, NULL);
		if ( USE_DX_LOCK ) ddrval=IDirectDrawSurface_Lock(lpDDSurface, NULL, &ddsd, DDLOCK_WAIT, NULL);
		for ( nZeileZiel=MovieScale; nZeileZiel < CurrentY/2-MovieScale; nZeileZiel++ ) {
			
			
			if ( LinesOutEven[nZeileZiel] > 0 ) {
				if ( LinesOutEven[nZeileZiel] == 1 ) {
					memcpy(pLinesDstAddressEven[nZeileZiel],pLinesAddressEven[CurrentFrame*CLEARLINES+nZeileZiel], CurrentX*2);
				} else if ( LinesOutEven[nZeileZiel] == 2 ) {
					memcpy(pLinesDstAddressEven[nZeileZiel],pLinesAddressEven[CurrentFrame*CLEARLINES+nZeileZiel], CurrentX*2);
					memcpy((pLinesDstAddressEven[nZeileZiel]+DP),pLinesAddressEven[CurrentFrame*CLEARLINES+nZeileZiel], CurrentX*2);
				} else if ( LinesOutEven[nZeileZiel] == 3 ) {
					memcpy(pLinesDstAddressEven[nZeileZiel],pLinesAddressEven[CurrentFrame*CLEARLINES+nZeileZiel], CurrentX*2);
					memcpy((pLinesDstAddressEven[nZeileZiel]+DP),pLinesAddressEven[CurrentFrame*CLEARLINES+nZeileZiel], CurrentX*2);
					memcpy((pLinesDstAddressEven[nZeileZiel]+DP*2),pLinesAddressEven[CurrentFrame*CLEARLINES+nZeileZiel], CurrentX*2);
				} else {
					memcpy(pLinesDstAddressEven[nZeileZiel],pLinesAddressEven[CurrentFrame*CLEARLINES+nZeileZiel], CurrentX*2);
					memcpy((pLinesDstAddressEven[nZeileZiel]+DP),pLinesAddressEven[CurrentFrame*CLEARLINES+nZeileZiel], CurrentX*2);
					memcpy((pLinesDstAddressEven[nZeileZiel]+DP*2),pLinesAddressEven[CurrentFrame*CLEARLINES+nZeileZiel], CurrentX*2);
					memcpy((pLinesDstAddressEven[nZeileZiel]+DP*3),pLinesAddressEven[CurrentFrame*CLEARLINES+nZeileZiel], CurrentX*2);
				}
			}
			
		}	
		if ( USE_DX_LOCK ) ddrval=IDirectDrawSurface_Unlock(lpDDSurface,  ddsd.lpSurface);
		
		if ( Display_Status_Bar == TRUE ) {
			if(dwLastCount+960 <= GetTickCount()) {
				sprintf(Text,"%d FPS",nFrame);
				SetWindowText (hwndFPSField,Text);
				nFrame=0;
				dwLastCount = GetTickCount();
			}
		}
	}
	
}



void YUVOutThread() 
{
	char Text[128];
	LPDIRECTDRAWSURFACE *lpDDOut;
	int   MBoost;
	DOUBLE YStep,YSum;   
	int i,j;
	int nZeileZiel;
	int CurrentFrame;
	DDSURFACEDESC	ddsd;
	HRESULT		    ddrval;
	short *pQuelle;
	FARPROC Call_Ausgabe;
	FARPROC Call_Fill;
	
	ProzessorMask=1<<(AusgabeProzessor);
	i=SetThreadAffinityMask (GetCurrentThread(),ProzessorMask);         
	
	if ( ThreadClassId == 0 )  SetThreadPriority(GetCurrentThread(),THREAD_PRIORITY_BELOW_NORMAL);
	else if ( ThreadClassId == 1 ) SetThreadPriority(GetCurrentThread(),THREAD_PRIORITY_NORMAL);
	else if ( ThreadClassId == 2 ) SetThreadPriority(GetCurrentThread(),THREAD_PRIORITY_ABOVE_NORMAL);
	else if ( ThreadClassId == 3 ) SetThreadPriority(GetCurrentThread(),THREAD_PRIORITY_HIGHEST);
	else if ( ThreadClassId == 4 ) SetThreadPriority(GetCurrentThread(),THREAD_PRIORITY_TIME_CRITICAL);
	
	if ( ColorMix == TRUE ) {
		if ( OutDib == TRUE ) {
			Init_HCYUV2RGBTabelle(TRUE);
		} else {
			if ( WTVBPP==16) Init_HCYUV2RGBTabelle(FALSE);
			else Init_HCYUV2RGBTabelle(TRUE); 
		}
	};
	   
	ddsd.dwSize=sizeof( ddsd );
	
	dwLastCount=GetTickCount();
	nFrame=0;
	for ( j= 0 ; j < 5 ; j++ ) {
		for ( i = 0; i < CurrentY ; i+=2 ) {
			pLinesAddressOdd[(j*CLEARLINES)+(i/2)] = (short *)pDisplay[j]+(i*CurrentX)+CurrentX;
			pLinesAddressEven[(j*CLEARLINES)+(i/2)] = (short *)pDisplay[j]+(i*CurrentX);
		}
	}
	
	MBoost=(int)(((double)MovieBoost/CLEARLINES)*CurrentY/2);
	YStep=(double)MaxY/(CurrentY-(MovieScale*4));
	for ( nZeileZiel=0; nZeileZiel < CurrentY/2; nZeileZiel++ ) {
		LinesOutEven[nZeileZiel]=0;
		LinesOutOdd[nZeileZiel]=0;
	};
	
	YSum=0.0;
	ShowY=0;
	for ( nZeileZiel=MovieScale; nZeileZiel < CurrentY/2-MovieScale; nZeileZiel++ ) {
		YSum=YSum+YStep;
		if ( YSum < 1 ) {
			LinesOutEven[nZeileZiel]=0;
		} else if ( YSum < 2 ) {
			if (( nZeileZiel > MBoost ) && ( nZeileZiel < CurrentY/2-MBoost )) LinesOutEven[nZeileZiel]=1;
			else LinesOutEven[nZeileZiel]=-1;
			YSum-=1;
			ShowY+=1;
		} else if ( YSum < 3 ) {
			if (( nZeileZiel > MBoost ) && ( nZeileZiel < CurrentY/2-MBoost )) LinesOutEven[nZeileZiel]=2;
			else LinesOutEven[nZeileZiel]=-2;
			
			YSum-=2;
			ShowY+=2;
		} else if ( YSum < 4 ) {
			if (( nZeileZiel > MBoost ) && ( nZeileZiel < CurrentY/2-MBoost )) LinesOutEven[nZeileZiel]=3;
			else LinesOutEven[nZeileZiel]=-3;
			
			YSum-=3;
			ShowY+=3;
		} else {
			if (( nZeileZiel > MBoost ) && ( nZeileZiel < CurrentY/2-MBoost )) LinesOutEven[nZeileZiel]=4;
			else LinesOutEven[nZeileZiel]=-4;
			YSum-=4;
			ShowY+=4;
		};
		
		YSum=YSum+YStep;
		if ( YSum < 1 ) {
			LinesOutOdd[nZeileZiel]=0;
		} else if ( YSum < 2 ) {
			if (( nZeileZiel > MBoost ) && ( nZeileZiel < CurrentY/2-MBoost )) LinesOutOdd[nZeileZiel]=1;
			else LinesOutOdd[nZeileZiel]=-1;
			YSum-=1;
			ShowY+=1;
		} else if ( YSum < 3 ) {
			if (( nZeileZiel > MBoost ) && ( nZeileZiel < CurrentY/2-MBoost )) LinesOutOdd[nZeileZiel]=2;
			else LinesOutOdd[nZeileZiel]=-2;
			
			YSum-=2;
			ShowY+=2;
		} else if ( YSum < 4 ) {
			if (( nZeileZiel > MBoost ) && ( nZeileZiel < CurrentY/2-MBoost )) LinesOutOdd[nZeileZiel]=3;
			else LinesOutOdd[nZeileZiel]=-3;
			
			YSum-=3;
			ShowY+=3;
		} else {
			if (( nZeileZiel > MBoost ) && ( nZeileZiel < CurrentY/2-MBoost )) LinesOutOdd[nZeileZiel]=4;
			else LinesOutOdd[nZeileZiel]=-4;
			YSum-=4;
			ShowY+=4;
		};
		
		
	};
	ColFaktor = &HQTabelle[312*4*4];
	
	if ( OutDib == TRUE ) {
		memset(&pDecode[0],0x00,m_dwDisplaySize+1024);
		j=0;
		for ( nZeileZiel=0; nZeileZiel < CurrentY/2; nZeileZiel++ ) {
			pLinesDstAddressEven[nZeileZiel]=(unsigned short*)&pDecode[j*DibPitch/2];
			if (LinesOutEven[nZeileZiel] > 0 )  j+=LinesOutEven[nZeileZiel];
			else j-=LinesOutEven[nZeileZiel];
			pLinesDstAddressOdd[nZeileZiel]=(unsigned short*)&pDecode[j*DibPitch/2];
			if (LinesOutOdd[nZeileZiel] > 0 )  j+=LinesOutOdd[nZeileZiel];
			else j-=LinesOutOdd[nZeileZiel];
			
		};
		CurrentPitch=DibPitch;
		lpDDOut=&lpDDOverlay;
		if ( Current_Mode == 0 ) Call_Ausgabe=(FARPROC)(HQAusgebenRGB15);
		else if ( Current_Mode == 1 ) Call_Ausgabe=(FARPROC)(HQAusgebenRGB15DVB);
		Call_Fill=(FARPROC)(HQFillRGB);
		BytesOut=CurrentX*2;
		
	} else if (( ModeOverlay == TRUE ) && ( Can_Overlay ) && ( lpDDOverlay != NULL ))  {
		ColorMix=FALSE;
		
		Black_Overlay();
		if (( Can_Overlay_Strech ) && ( USE_DX_OV_STRECH )) {
			for ( nZeileZiel=MovieScale; nZeileZiel < CurrentY/2-MovieScale; nZeileZiel++ ) {
				if ( LinesOutEven[nZeileZiel] > 0 ) LinesOutEven[nZeileZiel]=1; 
				else if ( LinesOutEven[nZeileZiel] < 0 ) LinesOutEven[nZeileZiel]=-1; 
				if ( LinesOutOdd[nZeileZiel] > 0 ) LinesOutOdd[nZeileZiel]=1;
				else if ( LinesOutOdd[nZeileZiel] < 0 ) LinesOutOdd[nZeileZiel]=-1;
			}
		}
		
		
		j=0;
		for ( nZeileZiel=0; nZeileZiel < CurrentY/2; nZeileZiel++ ) {
			pLinesDstAddressEven[nZeileZiel]=(unsigned short*)lpOverlay+(j*(OverlayPitch/2));
			if (LinesOutEven[nZeileZiel] > 0 )  j+=LinesOutEven[nZeileZiel];
			else j-=LinesOutEven[nZeileZiel];
			pLinesDstAddressOdd[nZeileZiel]=(unsigned short*)lpOverlay+(j*(OverlayPitch/2));
			if (LinesOutOdd[nZeileZiel] > 0 )  j+=LinesOutOdd[nZeileZiel];
			else j-=LinesOutOdd[nZeileZiel];
		}
		
		CurrentPitch=OverlayPitch;
		lpDDOut=&lpDDOverlay;
		
		if ( Current_Mode == 0 ) Call_Ausgabe=(FARPROC)(HQAusgebenYUV2);
		else if ( Current_Mode == 1 ) Call_Ausgabe=(FARPROC)(HQAusgebenYUV2DVB);
		Call_Fill=(FARPROC)(HQFillYUV2);
		BytesOut=CurrentX*2;
	} else {
//		for ( i=0; i < MaxY; i++ ) fill( (unsigned short*)lpSurface+(Offset_for_WindowOut/2)+(i*(SurfacePitch/2)),CurrentX);
		j=0;
		for ( nZeileZiel=0; nZeileZiel < CurrentY/2; nZeileZiel++ ) {
			pLinesDstAddressEven[nZeileZiel]=(unsigned short*)lpSurface+(Offset_for_WindowOut/2)+(j*(SurfacePitch/2));
			if (LinesOutEven[nZeileZiel] > 0 )  j+=LinesOutEven[nZeileZiel];
			else j-=LinesOutEven[nZeileZiel];
			pLinesDstAddressOdd[nZeileZiel]=(unsigned short*)lpSurface+(Offset_for_WindowOut/2)+(j*(SurfacePitch/2));
			if (LinesOutOdd[nZeileZiel] > 0 )  j+=LinesOutOdd[nZeileZiel];
			else j-=LinesOutOdd[nZeileZiel];
		};
		CurrentPitch=SurfacePitch;
		lpDDOut=&lpDDSurface;
		if ( WTVBPP == 16 )  {
			if ( Current_Mode == 0 ) Call_Ausgabe=(FARPROC)(HQAusgebenRGB16);
			else if ( Current_Mode == 1 ) Call_Ausgabe=(FARPROC)(HQAusgebenRGB16DVB);
		} else {
			if ( Current_Mode == 0 ) Call_Ausgabe=(FARPROC)(HQAusgebenRGB15);
			else if ( Current_Mode == 1 ) Call_Ausgabe=(FARPROC)(HQAusgebenRGB15DVB);
		}
		Call_Fill=(FARPROC)(HQFillRGB);
		BytesOut=CurrentX*2;
	};
	
	for (;;) {
		nFrame++;
		Get_Thread_Status();
		CurrentFrame=NextFrame-1;
		if ( NextFrame== -1 ) {
			SetEvent(Decode_Event_Stop);
			return;
		};
		if ( CurrentFrame < 0 ) CurrentFrame=4;
		FindePalPlusZeile(pLinesAddressEven[CurrentFrame*CLEARLINES],&nPALplusData);
		if ( OutDib == FALSE ) {
			if ( WAIT_DX_VBI ) IDirectDraw_WaitForVerticalBlank(lpDD,DDWAITVB_BLOCKBEGIN, NULL);
			if ( USE_DX_LOCK ) ddrval=IDirectDrawSurface_Lock(*lpDDOut, NULL, &ddsd, DDLOCK_WAIT, NULL);
		};
		
		for ( nZeileZiel=MovieScale; nZeileZiel < CurrentY/2-MovieScale; nZeileZiel++ ) {
			
			if ( ColorMix==TRUE ) {
				if ( LinesOutEven[nZeileZiel] == 1 ) HCZeileAusgeben((DWORD*)pLinesAddressEven[CurrentFrame*CLEARLINES+nZeileZiel], (DWORD*)pLinesDstAddressEven[nZeileZiel],CurrentX);
				else if ( LinesOutEven[nZeileZiel] == 2 ) HCZeileAusgeben_2((DWORD*)pLinesAddressEven[CurrentFrame*CLEARLINES+nZeileZiel], (DWORD*)pLinesDstAddressEven[nZeileZiel],CurrentPitch, CurrentX);
				else if ( LinesOutEven[nZeileZiel] == 3 ) HCZeileAusgeben_3((DWORD*)pLinesAddressEven[CurrentFrame*CLEARLINES+nZeileZiel], (DWORD*)pLinesDstAddressEven[nZeileZiel],CurrentPitch, CurrentX);
				else if ( LinesOutEven[nZeileZiel] == 4 ) HCZeileAusgeben_4((DWORD*)pLinesAddressEven[CurrentFrame*CLEARLINES+nZeileZiel], (DWORD*)pLinesDstAddressEven[nZeileZiel],CurrentPitch, CurrentX);
				if ( LinesOutOdd[nZeileZiel] == 1 ) HCZeileAusgeben((DWORD*)pLinesAddressOdd[CurrentFrame*CLEARLINES+nZeileZiel], (DWORD*)pLinesDstAddressOdd[nZeileZiel],CurrentX);
				else if ( LinesOutOdd[nZeileZiel] == 2 ) HCZeileAusgeben_2((DWORD*)pLinesAddressOdd[CurrentFrame*CLEARLINES+nZeileZiel], (DWORD*)pLinesDstAddressOdd[nZeileZiel],CurrentPitch,CurrentX);
				else if ( LinesOutOdd[nZeileZiel] == 3 ) HCZeileAusgeben_3((DWORD*)pLinesAddressOdd[CurrentFrame*CLEARLINES+nZeileZiel], (DWORD*)pLinesDstAddressOdd[nZeileZiel],CurrentPitch,CurrentX);
				else if ( LinesOutOdd[nZeileZiel] == 4 ) HCZeileAusgeben_4((DWORD*)pLinesAddressOdd[CurrentFrame*CLEARLINES+nZeileZiel], (DWORD*)pLinesDstAddressOdd[nZeileZiel],CurrentPitch, CurrentX);
			} else {
				LinesAnzahl=LinesOutEven[nZeileZiel];
				if ( LinesAnzahl == 0 ) {
					pQuelle=pLinesAddressEven[(CurrentFrame*CLEARLINES)+nZeileZiel];
					_asm {
						mov edi,dword ptr [HQLinebufferEvenPtr];
						mov edx,pQuelle
							mov ebx,Call_Fill
							call ebx
					};
				} else if (( LinesAnzahl >= 1 ) && ( LinesAnzahl < 5 )) {
					pQuelle=pLinesAddressEven[(CurrentFrame*CLEARLINES)+nZeileZiel];
					TmpPtr=ZeilenPuffer;;
					ScreenPtr=pLinesDstAddressEven[nZeileZiel];
					if ( LinesAnzahl == 1 ) TmpPtr=ScreenPtr;
					_asm
					{
						mov edi,dword ptr [HQLinebufferEvenPtr];
						mov edx,pQuelle
							mov ebx,Call_Ausgabe
							call ebx
					};
				};
				
				LinesAnzahl=LinesOutOdd[nZeileZiel];
				if ( LinesAnzahl == 0 ) {
					pQuelle=pLinesAddressOdd[(CurrentFrame*CLEARLINES)+nZeileZiel];
					_asm {
						mov edi,dword ptr [HQLinebufferOddPtr];
						mov edx,pQuelle
							mov ebx,Call_Fill
							call ebx
					};
				} else if (( LinesAnzahl >= 1 ) && ( LinesAnzahl < 5 )) {
					pQuelle=pLinesAddressOdd[(CurrentFrame*CLEARLINES)+nZeileZiel];
					TmpPtr=ZeilenPuffer;;
					ScreenPtr=pLinesDstAddressOdd[nZeileZiel];
					if ( LinesAnzahl == 1 ) TmpPtr=ScreenPtr;
					_asm
					{
						mov edi,dword ptr [HQLinebufferOddPtr];
						mov edx,pQuelle
							mov ebx,Call_Ausgabe
							call ebx
					};	
				};		
				
			};
		}
		if ( OutDib == FALSE ) {
			if ( USE_DX_LOCK ) ddrval=IDirectDrawSurface_Unlock(*lpDDOut,  ddsd.lpSurface);
		} else SetDIBitsToDevice( DIB_Hdc,0,0,pDIB->bmiHeader.biWidth,ShowY,0,0,0,ShowY,pDecode,pDIB,DIB_RGB_COLORS);
		
		
		if ( Display_Status_Bar == TRUE  ) {
			if(dwLastCount+960 <= GetTickCount())
			{
				sprintf(Text,"%d FPS",nFrame);
				SetWindowText (hwndFPSField,Text);
				nFrame=0;
				dwLastCount = GetTickCount();
			}
		}
	}
};





void YUVOutThreadHalf() 
{
	char Text[128];
	LPDIRECTDRAWSURFACE *lpDDOut;
	int   MBoost;
	DOUBLE YStep,YSum;   
	int i,j;
	int nZeileZiel;
	int CurrentFrame;
	DDSURFACEDESC	ddsd;
	HRESULT		    ddrval;
	short *pQuelle;
	FARPROC Call_Ausgabe;
	FARPROC Call_Fill;
	
	ProzessorMask=1<<(AusgabeProzessor);
	i=SetThreadAffinityMask (GetCurrentThread(),ProzessorMask);         
	
	if ( ThreadClassId == 0 )  SetThreadPriority(GetCurrentThread(),THREAD_PRIORITY_BELOW_NORMAL);
	else if ( ThreadClassId == 1 ) SetThreadPriority(GetCurrentThread(),THREAD_PRIORITY_NORMAL);
	else if ( ThreadClassId == 2 ) SetThreadPriority(GetCurrentThread(),THREAD_PRIORITY_ABOVE_NORMAL);
	else if ( ThreadClassId == 3 ) SetThreadPriority(GetCurrentThread(),THREAD_PRIORITY_HIGHEST);
	else if ( ThreadClassId == 4 ) SetThreadPriority(GetCurrentThread(),THREAD_PRIORITY_TIME_CRITICAL);
	
	if ( ColorMix == TRUE ) {
		if ( OutDib == TRUE ) {
			Init_HCYUV2RGBTabelle(TRUE);
		} else {
			if ( WTVBPP==16) Init_HCYUV2RGBTabelle(FALSE);
			else Init_HCYUV2RGBTabelle(TRUE); 
		}
	};
	   
	ddsd.dwSize=sizeof( ddsd );
	
	dwLastCount=GetTickCount();
	nFrame=0;
	
	for ( j= 0 ; j < 5 ; j++ ) {
		for ( i = 0; i < 288; i++) {
			pLinesAddressEven[(j*CLEARLINES)+i] = (short *)pDisplay[j]+(i*CurrentX);
		}
	}
	
	MBoost=(int)(((double)MovieBoost/CLEARLINES)*CurrentY/2);
	YStep=(double)MaxY/(CurrentY/2-(MovieScale*2));
	for ( nZeileZiel=0; nZeileZiel < 288; nZeileZiel++ )  LinesOutEven[nZeileZiel]=0;
	
	YSum=0.0;
	ShowY=0;
	for ( nZeileZiel=MovieScale; nZeileZiel < CurrentY/2-MovieScale; nZeileZiel++ ) {
		YSum=YSum+YStep;
		if ( YSum < 1 ) {
			LinesOutEven[nZeileZiel]=0;
		} else if ( YSum < 2 ) {
			if (( nZeileZiel > MBoost ) && ( nZeileZiel < CurrentY/2-MBoost )) LinesOutEven[nZeileZiel]=1;
			else LinesOutEven[nZeileZiel]=-1;
			YSum-=1;
			ShowY+=1;
		} else if ( YSum < 3 ) {
			if (( nZeileZiel > MBoost ) && ( nZeileZiel < CurrentY/2-MBoost )) LinesOutEven[nZeileZiel]=2;
			else LinesOutEven[nZeileZiel]=-2;
			
			YSum-=2;
			ShowY+=2;
		} else if ( YSum < 4 ) {
			if (( nZeileZiel > MBoost ) && ( nZeileZiel < CurrentY/2-MBoost )) LinesOutEven[nZeileZiel]=3;
			else LinesOutEven[nZeileZiel]=-3;
			
			YSum-=3;
			ShowY+=3;
		} else {
			if (( nZeileZiel > MBoost ) && ( nZeileZiel < CurrentY/2-MBoost )) LinesOutEven[nZeileZiel]=4;
			else LinesOutEven[nZeileZiel]=-4;
			YSum-=4;
			ShowY+=4;
		};
		
		
	};
	
	
	ColFaktor = &HQTabelle[312*4*4];
	
	if ( OutDib == TRUE ) {
		memset(&pDecode[0],0x00,m_dwDisplaySize+1024);
		j=0;
		for ( nZeileZiel=0; nZeileZiel < CurrentY/2; nZeileZiel++ ) {
			pLinesDstAddressEven[nZeileZiel]=(unsigned short*)&pDecode[j*DibPitch/2];
			if (LinesOutEven[nZeileZiel] > 0 )  j+=LinesOutEven[nZeileZiel];
			else j-=LinesOutEven[nZeileZiel];
		};
		
		CurrentPitch=DibPitch;
		lpDDOut=&lpDDOverlay;
		if ( Current_Mode == 0 ) Call_Ausgabe=(FARPROC)(HQAusgebenRGB16);
		else if ( Current_Mode == 1 ) Call_Ausgabe=(FARPROC)(HQAusgebenRGB16DVB);
		Call_Fill=(FARPROC)(HQFillRGB);
		BytesOut=CurrentX*2;
		
	} else if (( ModeOverlay == TRUE ) && ( Can_Overlay ) && ( lpDDOverlay != NULL ))  {
		ColorMix=FALSE;
		Black_Overlay();
		if (( Can_Overlay_Strech ) && ( USE_DX_OV_STRECH )) {
			for ( nZeileZiel=MovieScale; nZeileZiel < CurrentY/2-MovieScale; nZeileZiel++ ) {
				if ( LinesOutEven[nZeileZiel] > 0 ) LinesOutEven[nZeileZiel]=1; 
				else if ( LinesOutEven[nZeileZiel] < 0 ) LinesOutEven[nZeileZiel]=-1; 
			}
		}
		j=0;
		for ( nZeileZiel=0; nZeileZiel < CurrentY/2; nZeileZiel++ ) {
			pLinesDstAddressEven[nZeileZiel]=(unsigned short*)lpOverlay+(j*(OverlayPitch/2));
			if (LinesOutEven[nZeileZiel] > 0 )  j+=LinesOutEven[nZeileZiel];
			else j-=LinesOutEven[nZeileZiel];
		}
		CurrentPitch=OverlayPitch;
		lpDDOut=&lpDDOverlay;
		if ( Current_Mode == 0 ) Call_Ausgabe=(FARPROC)(HQAusgebenYUV2);
		else if ( Current_Mode == 1 ) Call_Ausgabe=(FARPROC)(HQAusgebenYUV2DVB);
		Call_Fill=(FARPROC)(HQFillYUV2);
		BytesOut=CurrentX*2;
	} else {
		
//		for ( i=0; i < MaxY; i++ ) fill( (unsigned short*)lpSurface+(Offset_for_WindowOut/2)+(i*(SurfacePitch/2)),CurrentX);
		j=0;
		for ( nZeileZiel=0; nZeileZiel < CurrentY/2; nZeileZiel++ ) {
			pLinesDstAddressEven[nZeileZiel]=(unsigned short*)lpSurface+(Offset_for_WindowOut/2)+(j*(SurfacePitch/2));
			if (LinesOutEven[nZeileZiel] > 0 )  j+=LinesOutEven[nZeileZiel];
			else j-=LinesOutEven[nZeileZiel];
		};
		CurrentPitch=SurfacePitch;
		lpDDOut=&lpDDSurface;
		if ( WTVBPP == 16 ) {
			if ( Current_Mode == 0 ) Call_Ausgabe=(FARPROC)(HQAusgebenRGB16);
			else if ( Current_Mode == 1 ) Call_Ausgabe=(FARPROC)(HQAusgebenRGB16DVB);
		} else {  
			if ( Current_Mode == 0 ) Call_Ausgabe=(FARPROC)(HQAusgebenRGB15);
			else if ( Current_Mode == 1 ) Call_Ausgabe=(FARPROC)(HQAusgebenRGB15DVB);
		}
		Call_Fill=(FARPROC)(HQFillRGB);
		BytesOut=CurrentX*2;
	};
	
	for (;;) {
		nFrame++;
		Get_Thread_Status();
		CurrentFrame=NextFrame-1;
		if ( NextFrame== -1 ) {
			SetEvent(Decode_Event_Stop);
			return;
		};
		if ( CurrentFrame < 0 ) CurrentFrame=4;
		FindePalPlusZeile(pLinesAddressEven[CurrentFrame*CLEARLINES],&nPALplusData);
		if ( OutDib == FALSE ) {
			if ( WAIT_DX_VBI ) IDirectDraw_WaitForVerticalBlank(lpDD,DDWAITVB_BLOCKBEGIN, NULL);
			if ( USE_DX_LOCK ) ddrval=IDirectDrawSurface_Lock(*lpDDOut, NULL, &ddsd, DDLOCK_WAIT, NULL);
		};
		
		for ( nZeileZiel=MovieScale; nZeileZiel < CurrentY/2-MovieScale; nZeileZiel++ ) {
			
			if ( ColorMix==TRUE ) {
				if ( LinesOutEven[nZeileZiel] == 1 ) HCZeileAusgeben((DWORD*)pLinesAddressEven[CurrentFrame*CLEARLINES+nZeileZiel], (DWORD*)pLinesDstAddressEven[nZeileZiel],CurrentX);
				else if ( LinesOutEven[nZeileZiel] == 2 ) HCZeileAusgeben_2((DWORD*)pLinesAddressEven[CurrentFrame*CLEARLINES+nZeileZiel], (DWORD*)pLinesDstAddressEven[nZeileZiel],CurrentPitch, CurrentX);
				else if ( LinesOutEven[nZeileZiel] == 3 ) HCZeileAusgeben_3((DWORD*)pLinesAddressEven[CurrentFrame*CLEARLINES+nZeileZiel], (DWORD*)pLinesDstAddressEven[nZeileZiel],CurrentPitch, CurrentX);
				else if ( LinesOutEven[nZeileZiel] == 4 ) HCZeileAusgeben_4((DWORD*)pLinesAddressEven[CurrentFrame*CLEARLINES+nZeileZiel], (DWORD*)pLinesDstAddressEven[nZeileZiel],CurrentPitch, CurrentX);
			} else {
				LinesAnzahl=LinesOutEven[nZeileZiel];
				if ( LinesAnzahl == 0 ) {
					pQuelle=pLinesAddressEven[(CurrentFrame*CLEARLINES)+nZeileZiel];
					_asm {
						mov edi,dword ptr [HQLinebufferEvenPtr];
						mov edx,pQuelle
							mov ebx,Call_Fill
							call ebx
					};
				} else if (( LinesAnzahl >= 1 ) && ( LinesAnzahl < 5 )) {
					pQuelle=pLinesAddressEven[(CurrentFrame*CLEARLINES)+nZeileZiel];
					TmpPtr=ZeilenPuffer;;
					ScreenPtr=pLinesDstAddressEven[nZeileZiel];
					if ( LinesAnzahl == 1 ) TmpPtr=ScreenPtr;
					_asm
					{
						mov edi,dword ptr [HQLinebufferEvenPtr];
						mov edx,pQuelle
							mov ebx,Call_Ausgabe
							call ebx
					};
				};
			};		
		}
		if ( OutDib == FALSE ) {
			if ( USE_DX_LOCK ) ddrval=IDirectDrawSurface_Unlock(*lpDDOut,  ddsd.lpSurface);
		} else SetDIBitsToDevice( DIB_Hdc,0,0,pDIB->bmiHeader.biWidth,ShowY,0,0,0,ShowY,pDecode,pDIB,DIB_RGB_COLORS);
		
		if ( Display_Status_Bar == TRUE  ) {
			if(dwLastCount+960 <= GetTickCount())
			{
				sprintf(Text,"%d FPS",nFrame);
				SetWindowText (hwndFPSField,Text);
				nFrame=0;
				dwLastCount = GetTickCount();
			}
		}
	}
};


void NagraYUVHalfOutThread(  ) 
{
	
	LPDIRECTDRAWSURFACE *lpDDOut;
	int nZeileZiel   ;     
    int nZielInvers  ; 	
    int nZeileQuelle ; 	
    int nQuelleInvers; 	
    int Burst;
    DOUBLE YStep,YSum;   
	unsigned int i,j;
	DDSURFACEDESC	ddsd;
	HRESULT		    ddrval;
	short *pQuelle;
	FARPROC Call_Ausgabe;
	FARPROC Call_Fill;
	
	ProzessorMask=1<<(AusgabeProzessor);
	i=SetThreadAffinityMask (GetCurrentThread(),ProzessorMask);         
	
	if ( ThreadClassId == 0 )  SetThreadPriority(GetCurrentThread(),THREAD_PRIORITY_BELOW_NORMAL);
	else if ( ThreadClassId == 1 ) SetThreadPriority(GetCurrentThread(),THREAD_PRIORITY_NORMAL);
	else if ( ThreadClassId == 2 ) SetThreadPriority(GetCurrentThread(),THREAD_PRIORITY_ABOVE_NORMAL);
	else if ( ThreadClassId == 3 ) SetThreadPriority(GetCurrentThread(),THREAD_PRIORITY_HIGHEST);
	else if ( ThreadClassId == 4 ) SetThreadPriority(GetCurrentThread(),THREAD_PRIORITY_TIME_CRITICAL);
	
	
	ddsd.dwSize=sizeof( ddsd );
	
	nPixelSchritt = CurrentX/TESTPIXEL/2;
	memset(&HQLinebufferEven[0],0x00,13563);
	Init_More();
	
	dwLastCount=GetTickCount();
	nFrame=0;
	
	for ( i = 0; i < ZEILENZAHL; i++) {
		BlackSet[i]=FALSE;
	}
	
	
	for ( j= 0 ; j < 5 ; j++ ) {
		for ( i = 0; i < (unsigned)CurrentY ; i+=2 ) {
			pLinesAddressEven[(j*ZEILENZAHL)+(i/2)] = (short *)pDisplay[j]+((i/2)*CurrentX);
		}
	}
	
	
	YStep=(double)MaxY/(ZEILENZAHL-1-(MovieScale*2));
	YSum=0.0;
	ShowY=0;
	
	for ( nZeileZiel=0; nZeileZiel < ZEILENZAHL; nZeileZiel++ ) {
		LinesOutEven[nZeileZiel]=0;
	};
	
	for ( nZeileZiel=1+MovieScale; nZeileZiel < ZEILENZAHL-MovieScale; nZeileZiel++ ) {
		YSum=YSum+YStep;
		if ( YSum < 1 ) {
			LinesOutEven[nZeileZiel]=0;
		} else if ( YSum < 2 ) {
			if (( nZeileZiel > MovieBoost ) && ( nZeileZiel < ZEILENZAHL-MovieBoost )) LinesOutEven[nZeileZiel]=1;
			else LinesOutEven[nZeileZiel]=-1;
			YSum-=1;
			ShowY+=1;
		} else if ( YSum < 3 ) {
			if (( nZeileZiel > MovieBoost ) && ( nZeileZiel < ZEILENZAHL-MovieBoost )) LinesOutEven[nZeileZiel]=2;
			else LinesOutEven[nZeileZiel]=-2;
			
			YSum-=2;
			ShowY+=2;
		} else if ( YSum < 4 ) {
			if (( nZeileZiel > MovieBoost ) && ( nZeileZiel < ZEILENZAHL-MovieBoost )) LinesOutEven[nZeileZiel]=3;
			else LinesOutEven[nZeileZiel]=-3;
			
			YSum-=3;
			ShowY+=3;
		} else {
			if (( nZeileZiel > MovieBoost ) && ( nZeileZiel < ZEILENZAHL-MovieBoost )) LinesOutEven[nZeileZiel]=4;
			else LinesOutEven[nZeileZiel]=-4;
			YSum-=4;
			ShowY+=4;
		};
	};
	
	
	if ( OutDib == TRUE ) {
		memset(&pDecode[0],0x00,m_dwDisplaySize+1024);
		j=ShowY;
		for ( nZeileZiel=1+MovieScale; nZeileZiel < ZEILENZAHL-MovieScale; nZeileZiel++ ) {
			if (LinesOutEven[nZeileZiel] > 0 )  j-=LinesOutEven[nZeileZiel];
			else j+=LinesOutEven[nZeileZiel];
			pLinesDstAddressEven[nZeileZiel]=(unsigned short*)&pDecode[j*DibPitch/2];
		};
		
		CurrentPitch=DibPitch;
		lpDDOut=&lpDDOverlay;
		Call_Ausgabe=(FARPROC)(HQAusgebenRGB15);
		Call_Fill=(FARPROC)(HQFillRGB);
		BytesOut=CurrentX*2;
		
	} else if (( ModeOverlay == TRUE ) && ( Can_Overlay ) && ( lpDDOverlay != NULL ))  {
		ColorMix=FALSE;
		Black_Overlay();
		if (( Can_Overlay_Strech ) && ( USE_DX_OV_STRECH )) {
			for ( nZeileZiel=MovieScale; nZeileZiel < CurrentY/2-MovieScale; nZeileZiel++ ) {
				if ( LinesOutEven[nZeileZiel] > 0 ) LinesOutEven[nZeileZiel]=1; 
				else if ( LinesOutEven[nZeileZiel] < 0 ) LinesOutEven[nZeileZiel]=-1; 
			}
		}
		j=0;
		for ( nZeileZiel=0; nZeileZiel < CurrentY/2; nZeileZiel++ ) {
			pLinesDstAddressEven[nZeileZiel]=(unsigned short*)lpOverlay+(j*(OverlayPitch/2));
			if (LinesOutEven[nZeileZiel] > 0 )  j+=LinesOutEven[nZeileZiel];
			else j-=LinesOutEven[nZeileZiel];
		}
		CurrentPitch=OverlayPitch;
		lpDDOut=&lpDDOverlay;
		Call_Ausgabe=(FARPROC)(HQAusgebenYUV2);
		Call_Fill=(FARPROC)(HQFillYUV2);
		BytesOut=CurrentX*2;
	} else {
		
//		for ( i=0; i < (unsigned)MaxY; i++ ) fill( (unsigned short*)lpSurface+(Offset_for_WindowOut/2)+(i*(SurfacePitch/2)),CurrentX);
		j=0;
		for ( nZeileZiel=0; nZeileZiel < CurrentY/2; nZeileZiel++ ) {
			pLinesDstAddressEven[nZeileZiel]=(unsigned short*)lpSurface+(Offset_for_WindowOut/2)+(j*(SurfacePitch/2));
			if (LinesOutEven[nZeileZiel] > 0 )  j+=LinesOutEven[nZeileZiel];
			else j-=LinesOutEven[nZeileZiel];
		};
		CurrentPitch=SurfacePitch;
		lpDDOut=&lpDDSurface;
		if ( WTVBPP == 16 )    Call_Ausgabe=(FARPROC) (HQAusgebenRGB16);
		else  Call_Ausgabe=(FARPROC)(HQAusgebenRGB15);
		Call_Fill=(FARPROC)(HQFillRGB);
		BytesOut=CurrentX*2;
	};
	
	for (;;)  {
		nFrame++;
		Wait_DStatus();
		CurrentFrame=NextFrame-1;
		if ( NextFrame== -1 ) {
			SetEvent(Decode_Event_Stop);
			return;
		};
		if ( CurrentFrame < 0 ) CurrentFrame=4;
		Burst=( pBurstLine[CurrentFrame][3] < 128)?0:1;   //Pal Burst Position 
		
		FindYUVLineOrder(&pLinesAddressEven[CurrentFrame*ZEILENZAHL], &pSortLinesEven[0],&nPALplusZeile,&nPALplusData);
		if (bFilterBlueLines) {
			FilterBlueLines((DWORD**)(&pLinesAddressEven[CurrentFrame*ZEILENZAHL]), &pSortLinesEven[0],TRUE);
		}
		
		if ((!MoreLinesLock) && ( FirstWidthOdd > 0)) More_Even(nPALplusZeile);
		
		if ( OutDib == FALSE ) {
			if ( WAIT_DX_VBI ) IDirectDraw_WaitForVerticalBlank(lpDD,DDWAITVB_BLOCKBEGIN, NULL);
			if ( USE_DX_LOCK ) ddrval=IDirectDrawSurface_Lock(*lpDDOut, NULL, &ddsd, DDLOCK_WAIT, NULL);
		};
		
		for ( nZeileZiel=1+MovieScale; nZeileZiel < ZEILENZAHL-MovieScale; nZeileZiel++ ) {
			if ( TVTYPE == 2 ) ColFaktor = &NullTable[0];
			else {
				nZeileQuelle  = pSortLinesEven[nZeileZiel];
				nZielInvers   = nZeileZiel & 1;
				nQuelleInvers = nZeileQuelle & 1;
				if ( nZeileQuelle < 32 ) { 
					nQuelleInvers ^= 1;
					nZeileQuelle-=25;
				};
				if(Burst == 0 )
				{
					nQuelleInvers ^= 1;
					nZielInvers   ^= 1;
				}
				nZeileQuelle -= nZeileZiel;
				ColFaktor = &HQTabelle[((nZeileQuelle+312)*4+nQuelleInvers+nZielInvers*2)*4];
			};
			
			LinesAnzahl=LinesOutEven[nZeileZiel];
            if ( LinesAnzahl == 0 ) {
				pQuelle=pLinesAddressEven[(CurrentFrame*ZEILENZAHL)+pSortLinesEven[nZeileZiel]];
				_asm {
					mov edi,dword ptr [HQLinebufferEvenPtr];
					mov edx,pQuelle
						mov ebx,Call_Fill
						call ebx
				};
			} else if (( LinesAnzahl >= 1 ) && ( LinesAnzahl < 5 )) {
				pQuelle=pLinesAddressEven[(CurrentFrame*ZEILENZAHL)+pSortLinesEven[nZeileZiel]];
				TmpPtr=ZeilenPuffer;
				ScreenPtr=pLinesDstAddressEven[nZeileZiel];
				if ( LinesAnzahl == 1 ) TmpPtr=ScreenPtr;
				_asm
				{
					mov edi,dword ptr [HQLinebufferEvenPtr];
					mov edx,pQuelle
						mov ebx,Call_Ausgabe
						call ebx
				};
			};
		}
		
		if ( OutDib == FALSE ) {
			if ( USE_DX_LOCK ) ddrval=IDirectDrawSurface_Unlock(*lpDDOut,  ddsd.lpSurface);
		} else SetDIBitsToDevice( DIB_Hdc,0,0,pDIB->bmiHeader.biWidth,ShowY,0,0,0,ShowY,pDecode,pDIB,DIB_RGB_COLORS);
		
		if(dwLastCount+960 <= GetTickCount())
		{
			if (MoreLinesLock==FALSE) {
				if ( MatrixModeEven == TRUE ) MoreLinesLock=TRUE;
				if ( AutoLockCount < 10 ) AutoLockCount++;
				else MoreLinesLock=TRUE;
				if ( MoreLinesLock == TRUE ) {
					if (( Line1BlackEven == FALSE ) && 
						( Line285BlackEven == FALSE ) && 
						( Line286BlackEven == FALSE ) && 
						( MatrixModeEven == FALSE )) UntilEven=1;
				}
			} else {
				if (Check_Even() == FALSE ) Clear_Status();
			}
			
			if ( Display_Status_Bar == TRUE ) {
				sprintf(Text,"%d FPS",nFrame);
				SetWindowText (hwndFPSField,Text);
			}
			nFrame=0;
			dwLastCount = GetTickCount();
		}
	}
};




void NagraYUVOutThread(  ) 
{
    LPDIRECTDRAWSURFACE *lpDDOut;
	int nZeileZiel   ;     
    int nZielInvers  ; 	
    int nZeileQuelle ; 	
    int nQuelleInvers; 	
    int Burst;
    DOUBLE YStep,YSum;   
	unsigned int i,j;
	DDSURFACEDESC	ddsd;
	HRESULT		    ddrval;
	short *pQuelle;
	FARPROC Call_Ausgabe;
	FARPROC Call_Fill;
	
	ProzessorMask=1<<(AusgabeProzessor);
	i=SetThreadAffinityMask (GetCurrentThread(),ProzessorMask);         
	
	
	if ( ThreadClassId == 0 )  SetThreadPriority(GetCurrentThread(),THREAD_PRIORITY_BELOW_NORMAL);
	else if ( ThreadClassId == 1 ) SetThreadPriority(GetCurrentThread(),THREAD_PRIORITY_NORMAL);
	else if ( ThreadClassId == 2 ) SetThreadPriority(GetCurrentThread(),THREAD_PRIORITY_ABOVE_NORMAL);
	else if ( ThreadClassId == 3 ) SetThreadPriority(GetCurrentThread(),THREAD_PRIORITY_HIGHEST);
	else if ( ThreadClassId == 4 ) SetThreadPriority(GetCurrentThread(),THREAD_PRIORITY_TIME_CRITICAL);
	
	ddsd.dwSize=sizeof( ddsd );
	
	nPixelSchritt = CurrentX/TESTPIXEL/2;
	//fill(HQLinebufferEven,1480*4);
	//fill(HQLinebufferOdd,1480*4);
	memset(&HQLinebufferEven[0],0x00,13563);
	memset(&HQLinebufferOdd[0],0x00,13563);
	Init_More();
	
	for ( i = 0; i < ZEILENZAHL; BlackSet[i++]=FALSE ) ;
	
	
	dwLastCount=GetTickCount();
	nFrame=0;
	for ( j= 0 ; j < 5 ; j++ ) {
		for ( i = 0; i < (unsigned)CurrentY ; i+=2 ) {
			pLinesAddressOdd[(j*ZEILENZAHL)+(i/2)] = (short *)pDisplay[j]+(i*CurrentX)+CurrentX;
			pLinesAddressEven[(j*ZEILENZAHL)+(i/2)] = (short *)pDisplay[j]+(i*CurrentX);
		}
	}
	
	
	
	
	YStep=(double)MaxY/((ZEILENZAHL-1-MovieScale*2)*2);
	for ( nZeileZiel=0; nZeileZiel < ZEILENZAHL; nZeileZiel++ ) {
		LinesOutEven[nZeileZiel]=0;
		LinesOutOdd[nZeileZiel]=0;
	};
	
	YSum=0.0;
	ShowY=0;
	for ( nZeileZiel=MovieScale+1; nZeileZiel < ZEILENZAHL-MovieScale; nZeileZiel++ ) {
		YSum=YSum+YStep;
		if ( YSum < 1 ) {
			LinesOutEven[nZeileZiel]=0;
		} else if ( YSum < 2 ) {
			if (( nZeileZiel > MovieBoost ) && ( nZeileZiel < ZEILENZAHL-MovieBoost )) LinesOutEven[nZeileZiel]=1;
			else LinesOutEven[nZeileZiel]=-1;
			YSum-=1;
			ShowY+=1;
		}  else if ( YSum < 3 ) {
			if (( nZeileZiel > MovieBoost ) && ( nZeileZiel < ZEILENZAHL-MovieBoost )) LinesOutEven[nZeileZiel]=2;
			else LinesOutEven[nZeileZiel]=-2;
			
			YSum-=2;
			ShowY+=2;
		} else if ( YSum < 4 ) {
			if (( nZeileZiel > MovieBoost ) && ( nZeileZiel < ZEILENZAHL-MovieBoost )) LinesOutEven[nZeileZiel]=3;
			else LinesOutEven[nZeileZiel]=-3;
			
			YSum-=3;
			ShowY+=3;
		} else {
			if (( nZeileZiel > MovieBoost ) && ( nZeileZiel < ZEILENZAHL-MovieBoost )) LinesOutEven[nZeileZiel]=4;
			else LinesOutEven[nZeileZiel]=-4;
			YSum-=4;
			ShowY+=4;
		};
		
		
		YSum=YSum+YStep;
		if ( YSum < 1 ) {
			LinesOutOdd[nZeileZiel]=0;
		} else if ( YSum < 2 ) {
			if (( nZeileZiel > MovieBoost ) && ( nZeileZiel < ZEILENZAHL-MovieBoost )) LinesOutOdd[nZeileZiel]=1;
			else LinesOutOdd[nZeileZiel]=-1;
			YSum-=1;
			ShowY+=1;
		} else if ( YSum < 3 ) {
			if (( nZeileZiel > MovieBoost ) && ( nZeileZiel < ZEILENZAHL-MovieBoost )) LinesOutOdd[nZeileZiel]=2;
			else LinesOutOdd[nZeileZiel]=-2;
			
			YSum-=2;
			ShowY+=2;
		} else if ( YSum < 4 ) {
			if (( nZeileZiel > MovieBoost ) && ( nZeileZiel < ZEILENZAHL-MovieBoost )) LinesOutOdd[nZeileZiel]=3;
			else LinesOutOdd[nZeileZiel]=-3;
			
			YSum-=3;
			ShowY+=3;
		} else {
			if (( nZeileZiel > MovieBoost ) && ( nZeileZiel < ZEILENZAHL-MovieBoost )) LinesOutOdd[nZeileZiel]=4;
			else LinesOutOdd[nZeileZiel]=-4;
			YSum-=4;
			ShowY+=4;
		}; 
	};
	
	if ( OutDib == TRUE ) {
		memset(&pDecode[0],0x00,m_dwDisplaySize+1024);
		j=ShowY;
		for ( nZeileZiel=1; nZeileZiel < ZEILENZAHL; nZeileZiel++ ) {
			if (LinesOutEven[nZeileZiel] > 0 )  j-=LinesOutEven[nZeileZiel];
			else j+=LinesOutEven[nZeileZiel];
			pLinesDstAddressEven[nZeileZiel]=(unsigned short*)&pDecode[j*DibPitch/2];
			if (LinesOutOdd[nZeileZiel] > 0 )  j-=LinesOutOdd[nZeileZiel];
			else j+=LinesOutOdd[nZeileZiel];
			pLinesDstAddressOdd[nZeileZiel]=(unsigned short*)&pDecode[j*DibPitch/2];
		};
		
		CurrentPitch=DibPitch;
		lpDDOut=&lpDDOverlay;
		Call_Ausgabe=(FARPROC)(HQAusgebenRGB15);
		Call_Fill=(FARPROC)(HQFillRGB);
		BytesOut=CurrentX*2;
		
	} else if (( ModeOverlay == TRUE ) && ( Can_Overlay ) && ( lpDDOverlay != NULL ))  {
		ColorMix=FALSE;
		
		Black_Overlay();
		if (( Can_Overlay_Strech ) && ( USE_DX_OV_STRECH )) {
			for ( nZeileZiel=MovieScale; nZeileZiel < CurrentY/2-MovieScale; nZeileZiel++ ) {
				if ( LinesOutEven[nZeileZiel] > 0 ) LinesOutEven[nZeileZiel]=1; 
				else if ( LinesOutEven[nZeileZiel] < 0 ) LinesOutEven[nZeileZiel]=-1; 
				if ( LinesOutOdd[nZeileZiel] > 0 ) LinesOutOdd[nZeileZiel]=1;
				else if ( LinesOutOdd[nZeileZiel] < 0 ) LinesOutOdd[nZeileZiel]=-1;
			}
		}
		
		
		j=0;
		for ( nZeileZiel=0; nZeileZiel < CurrentY/2; nZeileZiel++ ) {
			pLinesDstAddressEven[nZeileZiel]=(unsigned short*)lpOverlay+(j*(OverlayPitch/2));
			if (LinesOutEven[nZeileZiel] > 0 )  j+=LinesOutEven[nZeileZiel];
			else j-=LinesOutEven[nZeileZiel];
			pLinesDstAddressOdd[nZeileZiel]=(unsigned short*)lpOverlay+(j*(OverlayPitch/2));
			if (LinesOutOdd[nZeileZiel] > 0 )  j+=LinesOutOdd[nZeileZiel];
			else j-=LinesOutOdd[nZeileZiel];
		}
		
		CurrentPitch=OverlayPitch;
		lpDDOut=&lpDDOverlay;
		Call_Ausgabe=(FARPROC)(HQAusgebenYUV2);
		Call_Fill=(FARPROC)(HQFillYUV2);
		BytesOut=CurrentX*2;
	} else {
//		for ( i=0; i < (unsigned)MaxY; i++ ) fill( (unsigned short*)lpSurface+(Offset_for_WindowOut/2)+(i*(SurfacePitch/2)),CurrentX);
		j=0;
		for ( nZeileZiel=0; nZeileZiel < CurrentY/2; nZeileZiel++ ) {
			pLinesDstAddressEven[nZeileZiel]=(unsigned short*)lpSurface+(Offset_for_WindowOut/2)+(j*(SurfacePitch/2));
			if (LinesOutEven[nZeileZiel] > 0 )  j+=LinesOutEven[nZeileZiel];
			else j-=LinesOutEven[nZeileZiel];
			pLinesDstAddressOdd[nZeileZiel]=(unsigned short*)lpSurface+(Offset_for_WindowOut/2)+(j*(SurfacePitch/2));
			if (LinesOutOdd[nZeileZiel] > 0 )  j+=LinesOutOdd[nZeileZiel];
			else j-=LinesOutOdd[nZeileZiel];
		};
		CurrentPitch=SurfacePitch;
		lpDDOut=&lpDDSurface;
		if ( WTVBPP == 16 )    Call_Ausgabe=(FARPROC) (HQAusgebenRGB16);
		else  Call_Ausgabe=(FARPROC)(HQAusgebenRGB15);
		Call_Fill=(FARPROC)(HQFillRGB);
		BytesOut=CurrentX*2;
	};
	
	
	
	for (;;)  {
		nFrame+=2;
		Wait_DStatus();
		CurrentFrame=NextFrame-1;
		if ( NextFrame== -1 ) {
			SetEvent(Decode_Event_Stop);
			return;
		};
		if ( CurrentFrame < 0 ) CurrentFrame=4;
		Burst=( pBurstLine[CurrentFrame][3] < 128)?0:1;   //Pal Burst Position 
		
		FindYUVLineOrder(&pLinesAddressEven[CurrentFrame*ZEILENZAHL], &pSortLinesEven[0],&nPALplusZeile,&nPALplusData);
		
		if ((!MoreLinesLock) && ( FirstWidthOdd > 0)) More_Even(nPALplusZeile);
		FindYUVSecondLineOrder(&pLinesAddressOdd[CurrentFrame*ZEILENZAHL], &pSortLinesOdd[0]);
		
		if (bFilterBlueLines) {
			FilterBlueLines((DWORD**)(&pLinesAddressEven[CurrentFrame*ZEILENZAHL]), &pSortLinesEven[0],TRUE);
			FilterBlueLines((DWORD**)(&pLinesAddressOdd[CurrentFrame*ZEILENZAHL]), &pSortLinesOdd[0],FALSE);
		}
		
		if ((!MoreLinesLock) && ( FirstWidthOdd > 0)) More_Odd();
		
		
		if ( OutDib == FALSE ) {
			if ( WAIT_DX_VBI ) IDirectDraw_WaitForVerticalBlank(lpDD,DDWAITVB_BLOCKBEGIN, NULL);
			if ( USE_DX_LOCK ) ddrval=IDirectDrawSurface_Lock(*lpDDOut, NULL, &ddsd, DDLOCK_WAIT, NULL);
		};
		
		for ( nZeileZiel=1+MovieScale; nZeileZiel < ZEILENZAHL-MovieScale; nZeileZiel++ ) {
			if ( TVTYPE == 2 ) ColFaktor = &NullTable[0];
			else {
				nZielInvers   = nZeileZiel & 1;
				nZeileQuelle  = pSortLinesEven[nZeileZiel];
				nQuelleInvers = nZeileQuelle & 1;
				if ( nZeileQuelle < 32 ) { 
					nQuelleInvers ^= 1;
					nZeileQuelle-=25;
				};
				nZeileQuelle -= nZeileZiel;
				if(Burst == 0 )
				{
					nQuelleInvers ^= 1;
					nZielInvers   ^= 1;
				}
				ColFaktor = &HQTabelle[((nZeileQuelle+312)*4+nQuelleInvers+nZielInvers*2)*4];
			};
			
			LinesAnzahl=LinesOutEven[nZeileZiel];
            if ( LinesAnzahl == 0 ) {
				pQuelle=pLinesAddressEven[(CurrentFrame*ZEILENZAHL)+pSortLinesEven[nZeileZiel]];
				_asm {
					mov edi,dword ptr [HQLinebufferEvenPtr];
					mov edx,pQuelle
						mov ebx,Call_Fill
						call ebx
				};
			} else if (( LinesAnzahl >= 1 ) && ( LinesAnzahl < 5 )) {
				
				
				
				
				pQuelle=pLinesAddressEven[(CurrentFrame*ZEILENZAHL)+pSortLinesEven[nZeileZiel]];
				TmpPtr=ZeilenPuffer;;
				ScreenPtr=pLinesDstAddressEven[nZeileZiel];
				if ( LinesAnzahl == 1 ) TmpPtr=ScreenPtr;
				
				
				
				_asm
				{
					mov edi,dword ptr [HQLinebufferEvenPtr];
					mov edx,pQuelle
						mov ebx,Call_Ausgabe
						call ebx
				};
				
			};
			
			
			if ( TVTYPE == 2 ) ColFaktor = &NullTable[0];
			else {
				nZeileQuelle  = pSortLinesOdd[nZeileZiel];
				nQuelleInvers = nZeileQuelle & 1;
				if ( nZeileQuelle < 32 ) nZeileQuelle-=26;
				nZeileQuelle -= nZeileZiel;
				if(Burst == 1 )  nQuelleInvers ^= 1;
				nZielInvers   ^= 1;
				ColFaktor = &HQTabelle[((nZeileQuelle+312)*4+nQuelleInvers+nZielInvers*2)*4];
			};
			
			LinesAnzahl=LinesOutOdd[nZeileZiel];
            if ( LinesAnzahl == 0 ) {
				pQuelle=pLinesAddressOdd[(CurrentFrame*ZEILENZAHL)+pSortLinesOdd[nZeileZiel]];
				_asm {
					mov edi,dword ptr [HQLinebufferOddPtr];
					mov edx,pQuelle
						mov ebx,Call_Fill
						call ebx
				};
			} else if (( LinesAnzahl >= 1 ) && ( LinesAnzahl < 5 )) {
				
				pQuelle=pLinesAddressOdd[(CurrentFrame*ZEILENZAHL)+pSortLinesOdd[nZeileZiel]];
				TmpPtr=ZeilenPuffer;;
				ScreenPtr=pLinesDstAddressOdd[nZeileZiel];
				if ( LinesAnzahl == 1 ) TmpPtr=ScreenPtr;
				_asm
				{
					mov edi,dword ptr [HQLinebufferOddPtr];
					mov edx,pQuelle
						mov ebx,Call_Ausgabe
						call ebx
				};
			};
		}
		
		if ( OutDib == FALSE ) {
			if ( USE_DX_LOCK ) ddrval=IDirectDrawSurface_Unlock(*lpDDOut,  ddsd.lpSurface);
		} else SetDIBitsToDevice( DIB_Hdc,0,0,pDIB->bmiHeader.biWidth,ShowY,0,0,0,ShowY,pDecode,pDIB,DIB_RGB_COLORS);
		
		if(dwLastCount+960 <= GetTickCount())
		{
			if (MoreLinesLock==FALSE) {
				if (( MatrixModeOdd == TRUE ) && ( MatrixModeEven == TRUE )) MoreLinesLock=TRUE;
				if ( AutoLockCount < 10 ) AutoLockCount++;
				else MoreLinesLock=TRUE;
				if ( MoreLinesLock == TRUE ) {
					if (( Line1BlackEven == FALSE ) && 
						( Line285BlackEven == FALSE ) && 
						( Line286BlackEven == FALSE ) && 
						( MatrixModeEven == FALSE )) UntilEven=1;
					if (( Line1BlackOdd == FALSE ) && 
						( Line285BlackOdd == FALSE ) && 
						( Line286BlackOdd == FALSE ) && 
						( MatrixModeOdd == FALSE )) UntilOdd=1;
				}
				
			} else {
				if (Check_Odd() == FALSE ) Clear_Status();
			}
			
			if ( Display_Status_Bar == TRUE ) {
				sprintf(Text,"%d FPS",nFrame);
				SetWindowText (hwndFPSField,Text);
			}
			nFrame=0;
			dwLastCount = GetTickCount();
		}
}
}


void DX_Lock_Thread() 
{
	LPDIRECTDRAWSURFACE *lpDDOut;
	DDSURFACEDESC	ddsd;
	HRESULT		    ddrval;
	
	ddsd.dwSize=sizeof( ddsd );
	
	if (( ModeOverlay == TRUE ) && ( Can_Overlay ) && ( lpDDOverlay != NULL ))  {
		lpDDOut=&lpDDOverlay;
	} else {
		lpDDOut=&lpDDSurface;
	};
	
	for (;;) {
		i=WaitForSingleObject(Decode_Event, 15 );
		if (( i != WAIT_TIMEOUT ) && ( i != WAIT_OBJECT_0 )) {
			SetEvent(Decode_Event_Stop);
			ExitThread(0);
		};
		if ( NextFrame== -1 ) {
			SetEvent(Decode_Event_Stop);
			return;
		};
		
		ResetEvent(Decode_Event);
		Sleep(1);
		ddrval=IDirectDrawSurface_Lock(*lpDDOut, NULL, &ddsd, DDLOCK_WAIT, NULL);
		Sleep(0);Sleep(0);Sleep(0);
		ddrval=IDirectDrawSurface_Unlock(*lpDDOut,  ddsd.lpSurface);
		Sleep(0);Sleep(0);Sleep(0);
	};
};




void Init_More()
{
	
	if ( MoreLinesLock == FALSE ) {
		AutoLockCount=0;
		UntilEven=0;
		UntilOdd=0;
		BlackEven=-1;
		BlackOdd=-1;
		MoreLinesLock=FALSE;  
		FirstWidthEven=TESTPIXEL;
		FirstWidthOdd=TESTPIXEL;
		FirstWidthEvenCount=0;
		FirstWidthOddCount=0;
		
		Line286BlackEven=FALSE;
		Line286BlackOdd=FALSE;
		Line1BlackEven=FALSE;
		Line1BlackOdd=FALSE;
		B1e=0;
		B286e=0;
		B1o=0;
		B286o=0;
		MatrixCountEven=0;
		MatrixModeEven=FALSE;
		MatrixLinesEven=0;
		MatrixCountOdd=0;
		MatrixModeOdd=FALSE;
		MatrixLinesOdd=0;
		
	}
	
};




BOOL Check_Even( void )
{
	int m=0;
	
	if (( Line0BlackEven == TRUE ) && (!BlackSet[pSortLinesEven[0]])) return(FALSE);
	if (( Line1BlackEven == TRUE ) && (!BlackSet[pSortLinesEven[1]])) return(FALSE);
	if (( Line285BlackEven == TRUE ) && (!BlackSet[pSortLinesEven[285]])) return(FALSE);
	if (( Line286BlackEven == TRUE ) && (!BlackSet[pSortLinesEven[286]])) return(FALSE);
	
	if ( MatrixModeEven == TRUE ) {
		for ( i=0 ; i < 68 ; i++ ) {
			if ( BlackSet[pSortLinesEven[i]]==TRUE ) m++;
		}
		if ( m < MatrixLinesEven ) return(FALSE);
	}
	return(TRUE);
};

BOOL Check_Odd( void )
{
	int m=0;
	
	if (( Line0BlackOdd == TRUE ) && (!BlackSet[pSortLinesOdd[0]])) return(FALSE);
	if (( Line1BlackOdd == TRUE ) && (!BlackSet[pSortLinesOdd[1]])) return(FALSE);
	if (( Line285BlackOdd == TRUE ) && (!BlackSet[pSortLinesOdd[285]])) return(FALSE);
	if (( Line286BlackOdd == TRUE ) && (!BlackSet[pSortLinesOdd[286]])) return(FALSE);
	
	if ( MatrixModeOdd == TRUE ) {
		for ( i=0 ; i < 68 ; i++ ) {
			if ( BlackSet[pSortLinesOdd[i]]==TRUE ) m++;
		}
		if ( m < MatrixLinesOdd ) return(FALSE);
	}
	return(TRUE);
};


void More_Even( int pp )
{
	int i;
	int m=0;
	int j;
	
    if ( BlackSet[pSortLinesEven[0]] ) {
		if ( B0e < 100 ) B0e++;
		else Line0BlackEven=TRUE;
		FirstWidthEvenCount=0;
	} else {
		B0e=0;
		if (( Line0BlackEven==TRUE ) &&
			( FirstWidthEven<TESTPIXEL)) FirstWidthEven++;
        FirstWidthEvenCount++;
        Line0BlackEven=FALSE;
	}
	
    if ( BlackSet[pSortLinesEven[1]] ) {
		if ( B1e < 100 ) B1e++;
		else Line1BlackEven=TRUE;
	} else {
		B1e=0;
        Line1BlackEven=FALSE;
	}
	
    if ( BlackSet[pSortLinesEven[286]] ) {
		if ( B286e < 100 ) B286e++;
		else Line286BlackEven=TRUE;
	} else {
		B286e=0;
        Line286BlackEven=FALSE;
	}
	
    if ( BlackSet[pSortLinesEven[285]] ) {
		if ( B285e < 100 ) B285e++;
		else Line285BlackEven=TRUE;
	} else {
		B285e=0;
        Line285BlackEven=FALSE;
	}
	
	if (( FirstWidthEvenCount > 100 ) &&
		( FirstWidthEven >3 )) {
		FirstWidthEvenCount=80;
		if (( MatrixModeEven == FALSE ) &&
			( Line0BlackEven == FALSE ) &&
			( Line1BlackEven == FALSE )) {
			AutoLockCount=0;
			FirstWidthEven--;
		};
	}
	
	if ( pp >= 0 ) BlackSet[pp]=TRUE;
	
	
	for ( i=0 ; i < 68 ; i++ ) {
		if ( BlackSet[pSortLinesEven[i]]==TRUE ) m++;
	}
	
	j=m;
	for ( i=68 ; i < 287 ; i++ ) {
		if ( BlackSet[pSortLinesEven[i]]==TRUE ) j++;
	}
	
	if ( j != BlackEven ) {
		AutoLockCount=0;
		BlackEven = j;
	}; 
	
	if (( m < 6 ) || ( m > 66 )) {
		MatrixModeEven=FALSE;
		MatrixCountEven=0;
						  MatrixLinesEven=0;
						  return;
	}
	
	if ( MatrixCountEven == 0 ) {
		MatrixLinesEven=m;
		MatrixCountEven++; 
	} else {
		if ( m==MatrixLinesEven )
		{ 
			if ( MatrixCountEven < 100 ) MatrixCountEven++; 
		} else MatrixCountEven-=10;
		
		if ( MatrixCountEven < 0 ) {
			MatrixCountEven=0;
			MatrixModeEven=FALSE;
			MatrixLinesEven=0;
			return;
		}
		
		if ( MatrixCountEven > 80) {
			if ( MatrixModeEven == FALSE ) {
				MatrixModeEven=TRUE;
				GetMatrix(MatrixLinesEven-1,0);
			}
		}
	}
}

void More_Odd()
{
	int m=0;
	int i;
	int j;
	
    if ( BlackSet[pSortLinesOdd[0]] ) {
		if ( B0o < 100 ) B0o++;
		else Line0BlackOdd=TRUE;
		FirstWidthOddCount=0;
		
	} else {
		B0o=0;
		if (( Line0BlackOdd==TRUE ) &&
			( FirstWidthOdd<TESTPIXEL)) FirstWidthOdd++;
		FirstWidthOddCount++;
        Line0BlackOdd=FALSE;
	}
	
	
    if ( BlackSet[pSortLinesOdd[1]] ) {
		if ( B1o < 100 ) B1o++;
		else Line1BlackOdd=TRUE;
	} else {
		B1o=0;
        Line1BlackOdd=FALSE;
	}
    if ( BlackSet[pSortLinesOdd[286]] ) {
		if ( B286o < 100 ) B286o++;
		else Line286BlackOdd=TRUE;
	} else {
		B286o=0;
        Line286BlackOdd=FALSE;
	}
    if ( BlackSet[pSortLinesOdd[285]] ) {
		if ( B285o < 100 ) B285o++;
		else Line285BlackOdd=TRUE;
	} else {
		B285o=0;
        Line285BlackOdd=FALSE;
	}
	
	
	if (( FirstWidthOddCount > 100 ) &&
		( FirstWidthOdd > 3 )) {
		FirstWidthOddCount=75;
		if (( MatrixModeOdd == FALSE ) &&
			( Line0BlackOdd == FALSE ) &&
			( Line1BlackOdd == FALSE )) {
			FirstWidthOdd--;
			AutoLockCount=0;
		};
	}
	
	
	
	for ( i=0 ; i < 68 ; i++ ) {
		if ( BlackSet[pSortLinesOdd[i]]==TRUE ) m++;
	}
	
	j=m;
	for ( i=68 ; i < 287 ; i++ ) {
		if ( BlackSet[pSortLinesOdd[i]]==TRUE ) j++;
	}
	
	if ( j != BlackOdd ) {
		AutoLockCount=0;
		BlackOdd = j;
	}; 
	
	
	if (( m < 6 ) || ( m > 66 )) {
		MatrixCountOdd=0;
						  MatrixLinesOdd=0;
						  return;
	}
	if ( MatrixCountOdd == 0 ) {
		MatrixLinesOdd=m;
		MatrixCountOdd++; 
	} else {
		if ( m==MatrixLinesOdd ) {
			if ( MatrixCountOdd < 100 ) MatrixCountOdd++; 
		} else MatrixCountOdd-=10;
		
		if ( MatrixCountOdd < 0 ) {
			MatrixCountOdd=0;
			MatrixModeOdd=FALSE;
			MatrixLinesOdd=0;
			return;
		}
		
		if ( MatrixCountOdd > 60 ) {
			if ( MatrixModeOdd == FALSE ) {
				MatrixModeOdd=TRUE;
				GetMatrix(MatrixLinesOdd-1,1);
			}
		}
	}
	
	
}



void VCYUVHalfOutThread(  ) 
{
	
#define VCCLEARLINES 287
	
    int nFrame=0;
	
	LPDIRECTDRAWSURFACE *lpDDOut;
	FARPROC Call_Ausgabe;
	
    int CurrentFrame;
	int nZeileZiel   ;     
    DOUBLE YStep,YSum;   
	unsigned int i,j;
    int OutX;
	int VCCut;
	int Burst;
	short *pQuelle;
	
	DDSURFACEDESC	ddsd;
    HRESULT		    ddrval;
	
	ProzessorMask=1<<(AusgabeProzessor);
	i=SetThreadAffinityMask (GetCurrentThread(),ProzessorMask);         
	
	if ( ThreadClassId == 0 )  SetThreadPriority(GetCurrentThread(),THREAD_PRIORITY_BELOW_NORMAL);
	else if ( ThreadClassId == 1 ) SetThreadPriority(GetCurrentThread(),THREAD_PRIORITY_NORMAL);
	else if ( ThreadClassId == 2 ) SetThreadPriority(GetCurrentThread(),THREAD_PRIORITY_ABOVE_NORMAL);
	else if ( ThreadClassId == 3 ) SetThreadPriority(GetCurrentThread(),THREAD_PRIORITY_HIGHEST);
	else if ( ThreadClassId == 4 ) SetThreadPriority(GetCurrentThread(),THREAD_PRIORITY_TIME_CRITICAL);
	
	OutX=710;
	if ( MaxX < OutX ) OutX=MaxX;
	Set_BT_Decrypt_Mode();
	
	ddsd.dwSize=sizeof( ddsd );
	
	
	YStep=(double)MaxY/(VCCLEARLINES-2-(MovieScale*2));
	YSum=0.0;
	ShowY=0;
	
	for ( nZeileZiel=0; nZeileZiel < VCCLEARLINES; nZeileZiel++ ) {
		LinesOutEven[nZeileZiel]=0;
	};
	
	for ( nZeileZiel=2+MovieScale; nZeileZiel < VCCLEARLINES-MovieScale; nZeileZiel++ ) {
		YSum=YSum+YStep;
		if ( YSum < 1 ) {
			LinesOutEven[nZeileZiel]=0;
		} else if ( YSum < 2 ) {
			if (( nZeileZiel > MovieBoost ) && ( nZeileZiel < VCCLEARLINES-MovieBoost )) LinesOutEven[nZeileZiel]=1;
			else LinesOutEven[nZeileZiel]=-1;
			YSum-=1;
			ShowY+=1;
		} else if ( YSum < 3 ) {
			if (( nZeileZiel > MovieBoost ) && ( nZeileZiel < VCCLEARLINES-MovieBoost )) LinesOutEven[nZeileZiel]=2;
			else LinesOutEven[nZeileZiel]=-2;
			
			YSum-=2;
			ShowY+=2;
		} else if ( YSum < 4 ) {
			if (( nZeileZiel > MovieBoost ) && ( nZeileZiel < VCCLEARLINES-MovieBoost )) LinesOutEven[nZeileZiel]=3;
			else LinesOutEven[nZeileZiel]=-3;
			
			YSum-=3;
			ShowY+=3;
		} else {
			if (( nZeileZiel > MovieBoost ) && ( nZeileZiel < VCCLEARLINES-MovieBoost )) LinesOutEven[nZeileZiel]=4;
			else LinesOutEven[nZeileZiel]=-4;
			YSum-=4;
			ShowY+=4;
		};
	};
	
	dwLastCount=GetTickCount();
	nFrame=0;
	for ( j= 0 ; j < 5 ; j++ ) {
		for ( i = 0; i < VCCLEARLINES ; i++ ) {
			pLinesAddressEven[(j*VCCLEARLINES)+i] = (short *)pDisplay[j]+(i*CurrentX)+VCLmargin;
		}
	}
	
	
	
	
	if ( OutDib == TRUE ) {
		memset(&pDecode[0],0x00,m_dwDisplaySize+1024);
		j=ShowY;
		for ( nZeileZiel=1; nZeileZiel < VCCLEARLINES; nZeileZiel++ ) {
			if (LinesOutEven[nZeileZiel] > 0 )  j-=LinesOutEven[nZeileZiel];
			else j+=LinesOutEven[nZeileZiel];
			pLinesDstAddressEven[nZeileZiel]=(unsigned short*)&pDecode[j*DibPitch/2];
		};
		CurrentPitch=DibPitch;
		Call_Ausgabe=(FARPROC) (VCHQAusgebenRGB15);
	} else if (( ModeOverlay == TRUE ) && ( Can_Overlay ) && ( lpDDOverlay != NULL ))  {
		Black_Overlay();
		j=0;
		if (( Can_Overlay_Strech ) && ( USE_DX_OV_STRECH )) {
            for ( nZeileZiel=1+MovieScale; nZeileZiel < VCCLEARLINES-MovieScale; nZeileZiel++ ) {
				if ( LinesOutEven[nZeileZiel] > 0 ) LinesOutEven[nZeileZiel] = 1;
				else if ( LinesOutEven[nZeileZiel] < 0 ) LinesOutEven[nZeileZiel] = -1;
			}
		}
		
		j=0;
		for ( nZeileZiel=1; nZeileZiel < VCCLEARLINES; nZeileZiel++ ) {
			pLinesDstAddressEven[nZeileZiel]=(unsigned short*)lpOverlay+(j*(OverlayPitch/2));
			if (LinesOutEven[nZeileZiel] > 0 )  j+=LinesOutEven[nZeileZiel];
			else j-=LinesOutEven[nZeileZiel];
		} 
		CurrentPitch=OverlayPitch;
		lpDDOut=&lpDDOverlay;
		Call_Ausgabe=(FARPROC)VCHQAusgebenYUV2;
	} else { 
//		for ( i=0; i < (unsigned)MaxY; i++ ) fill((unsigned short*)lpSurface+(Offset_for_WindowOut/2)+(i*(SurfacePitch/2)),OutX);
		j=0;
		for ( nZeileZiel=1; nZeileZiel < VCCLEARLINES; nZeileZiel++ ) {
			pLinesDstAddressEven[nZeileZiel]=(unsigned short*)lpSurface+(Offset_for_WindowOut/2)+(j*(SurfacePitch/2));
			if (LinesOutEven[nZeileZiel] > 0 )  j+=LinesOutEven[nZeileZiel];
			else j-=LinesOutEven[nZeileZiel];
		}
		CurrentPitch=SurfacePitch;
		lpDDOut=&lpDDSurface;
		if ( WTVBPP == 16 )    Call_Ausgabe=(FARPROC) (VCHQAusgebenRGB16);
		else  Call_Ausgabe=(FARPROC)(VCHQAusgebenRGB15);
	};
	
	BytesOut=OutX*2;
	
	VCFirstLine=2;
	while (( LinesOutEven[VCFirstLine] < 1 ) || ( LinesOutEven[VCFirstLine] > 5 )) VCFirstLine++;
	
	for ( i=0; i < 5; i++ ) {
		for ( nZeileZiel=VCFirstLine; nZeileZiel < VCCLEARLINES; nZeileZiel++ ) {
			if (( LinesOutEven[VCFirstLine] < 1 ) || ( LinesOutEven[VCFirstLine] > 5 )) {
				pLinesAddressEven[(i*VCCLEARLINES)+nZeileZiel]=NULL;
            };
		};
	}
	
	VCTotalLines=0;
	for ( nZeileZiel=VCFirstLine; nZeileZiel < VCCLEARLINES; nZeileZiel++ ) {
		if (pLinesAddressEven[nZeileZiel] != NULL ) VCTotalLines++;
	}
	
	VCTotalLines-=4;
	
	
	for (;;) {
		nFrame++;
		Wait_DStatus();
		if (!VideoPresent ) continue;
		CurrentFrame=NextFrame-1;
		if ( NextFrame== -1 ) {
			SetEvent(Decode_Event_Stop);
			return;
		};
		if ( CurrentFrame < 0 ) CurrentFrame=4;
		Burst=( pBurstLine[CurrentFrame][3] < 128)?0:1;   //Pal Burst Position 
		
		if (Find_Cut_Points_VC(&pLinesAddressEven[(CurrentFrame*VCCLEARLINES)],RealOffset,Burst)) {
			
			// Gute Dekodierung
		}
		
		if ( OutDib == FALSE ) {
			if ( WAIT_DX_VBI ) (void)IDirectDraw_WaitForVerticalBlank(lpDD,DDWAITVB_BLOCKBEGIN, NULL);
			if ( USE_DX_LOCK ) ddrval=IDirectDrawSurface_Lock(*lpDDOut, NULL, &ddsd, DDLOCK_WAIT, NULL);
		};
		for ( nZeileZiel=VCFirstLine; nZeileZiel < VCCLEARLINES; nZeileZiel++ ) {
			LinesAnzahl=LinesOutEven[nZeileZiel];
			if (( LinesAnzahl > 0 ) && ( LinesAnzahl < 5 ) ) {
				VCCut=RealOffset[nZeileZiel];
				if ( Chroma_Key == TRUE ) {
					if (((nZeileZiel+Burst)&1) == 1 ) ColFaktor=&VCHQTabelleBurst[VCCut*8];
					else ColFaktor=&VCHQTabelleNoBurst[VCCut*8];
				} else ColFaktor=&VCHQTabelleBurst[710*8];
				
				pQuelle=pLinesAddressEven[(CurrentFrame*VCCLEARLINES)+nZeileZiel];
				TmpPtr=ZeilenPuffer;
				ScreenPtr=pLinesDstAddressEven[nZeileZiel];
				
				if ( LinesAnzahl == 1 ) TmpPtr=ScreenPtr;
				_asm
				{
					mov esi,dword ptr [HQLinebufferEvenPtr];
					mov eax,VCCut
						shl eax,1
						add eax,2    
						mov edx,pQuelle
						mov ebx,Call_Ausgabe
						call ebx
				};	
			};  // Ende Anzahl Lines
		};
		
		if ( OutDib == TRUE ) {
			SetDIBitsToDevice( DIB_Hdc,
				0,
				0,
				pDIB->bmiHeader.biWidth,
				ShowY,
				0,
				0,
				0,
				ShowY,
				pDecode,
				pDIB,
				DIB_RGB_COLORS
				);
		} else {
			if ( USE_DX_LOCK ) ddrval=IDirectDrawSurface_Unlock(*lpDDOut,  ddsd.lpSurface);
		};
		
		if ( Display_Status_Bar == TRUE ) {
			if(dwLastCount+960 <= GetTickCount())
			{
				sprintf(Text,"%d FPS",nFrame);
				SetWindowText (hwndFPSField,Text);
				nFrame=0;
				dwLastCount = GetTickCount();
			}
		}
	}
}



void VCDecoderYUVHalfOutThread(  ) 
{
	LPDIRECTDRAWSURFACE *lpDDOut;
	FARPROC Call_Ausgabe;
    int nFrame=0;
	short *pQuelle;
    int CurrentFrame;
	int nZeileZiel   ;     
    DOUBLE YStep,YSum;   
	int Burst;
	unsigned int i,j;
    int VCCut;
	int VCIrq;
	int OutX;
    int Key=-1;
    int LastKey=-1;
	
	
	
	DDSURFACEDESC	ddsd;
    HRESULT		    ddrval;
	
	ProzessorMask=1<<(AusgabeProzessor);
	i=SetThreadAffinityMask (GetCurrentThread(),ProzessorMask);         
	
	if ( ThreadClassId == 0 )  SetThreadPriority(GetCurrentThread(),THREAD_PRIORITY_BELOW_NORMAL);
	else if ( ThreadClassId == 1 ) SetThreadPriority(GetCurrentThread(),THREAD_PRIORITY_NORMAL);
	else if ( ThreadClassId == 2 ) SetThreadPriority(GetCurrentThread(),THREAD_PRIORITY_ABOVE_NORMAL);
	else if ( ThreadClassId == 3 ) SetThreadPriority(GetCurrentThread(),THREAD_PRIORITY_HIGHEST);
	else if ( ThreadClassId == 4 ) SetThreadPriority(GetCurrentThread(),THREAD_PRIORITY_TIME_CRITICAL);
	
	VCIRQSync=FALSE;
	ddsd.dwSize=sizeof( ddsd );
	Set_BT_Decrypt_Mode();
	
	OutX=710;
	if ( MaxX < OutX ) OutX=MaxX;
	
	
	YStep=(double)MaxY/(ZEILENZAHL-(MovieScale*2));
	YSum=0.0;
	ShowY=0;
	
	for ( nZeileZiel=0; nZeileZiel < ZEILENZAHL; nZeileZiel++ ) {
		LinesOutEven[nZeileZiel]=0;
	};
	
	for ( nZeileZiel=1+MovieScale; nZeileZiel < ZEILENZAHL-MovieScale; nZeileZiel++ ) {
		YSum=YSum+YStep;
		if ( YSum < 1 ) {
			LinesOutEven[nZeileZiel]=0;
		} else if ( YSum < 2 ) {
			if (( nZeileZiel > MovieBoost ) && ( nZeileZiel < ZEILENZAHL-MovieBoost )) LinesOutEven[nZeileZiel]=1;
			else LinesOutEven[nZeileZiel]=-1;
			YSum-=1;
			ShowY+=1;
		} else if ( YSum < 3 ) {
			if (( nZeileZiel > MovieBoost ) && ( nZeileZiel < ZEILENZAHL-MovieBoost )) LinesOutEven[nZeileZiel]=2;
			else LinesOutEven[nZeileZiel]=-2;
			
			YSum-=2;
			ShowY+=2;
		} else if ( YSum < 4 ) {
			if (( nZeileZiel > MovieBoost ) && ( nZeileZiel < ZEILENZAHL-MovieBoost )) LinesOutEven[nZeileZiel]=3;
			else LinesOutEven[nZeileZiel]=-3;
			
			YSum-=3;
			ShowY+=3;
		} else {
			if (( nZeileZiel > MovieBoost ) && ( nZeileZiel < ZEILENZAHL-MovieBoost )) LinesOutEven[nZeileZiel]=4;
			else LinesOutEven[nZeileZiel]=-4;
			YSum-=4;
			ShowY+=4;
		};
	};
	
	
	
	dwLastCount=GetTickCount();
	nFrame=0;
	for ( j= 0 ; j < 5 ; j++ ) {
		for ( i = 0; i < ZEILENZAHL ; i++ ) {
			pLinesAddressEven[(j*ZEILENZAHL)+i] = (short *)pDisplay[j]+(i*CurrentX)+10;
		}
	}
	
	
	
	if ( OutDib == TRUE ) {
		memset(&pDecode[0],0x00,m_dwDisplaySize+1024);
		j=ShowY;
		for ( nZeileZiel=1; nZeileZiel < VCCLEARLINES; nZeileZiel++ ) {
			if (LinesOutEven[nZeileZiel] > 0 )  j-=LinesOutEven[nZeileZiel];
			else j+=LinesOutEven[nZeileZiel];
			pLinesDstAddressEven[nZeileZiel]=(unsigned short*)&pDecode[j*DibPitch/2];
		};
		CurrentPitch=DibPitch;
		Call_Ausgabe=(FARPROC)VCHQAusgebenRGB15;
	} else if (( ModeOverlay == TRUE ) && ( Can_Overlay ) && ( lpDDOverlay != NULL ))  {
		Black_Overlay();
		j=0;
		if (( Can_Overlay_Strech ) && ( USE_DX_OV_STRECH )) {
            for ( nZeileZiel=1+MovieScale; nZeileZiel < VCCLEARLINES-MovieScale; nZeileZiel++ ) {
				if ( LinesOutEven[nZeileZiel] > 0 ) LinesOutEven[nZeileZiel] = 1;
				else if ( LinesOutEven[nZeileZiel] < 0 ) LinesOutEven[nZeileZiel] = -1;
			}
		}
		
		j=0;
		for ( nZeileZiel=1; nZeileZiel < VCCLEARLINES; nZeileZiel++ ) {
			pLinesDstAddressEven[nZeileZiel]=(unsigned short*)lpOverlay+(j*(OverlayPitch/2));
			if (LinesOutEven[nZeileZiel] > 0 )  j+=LinesOutEven[nZeileZiel];
			else j-=LinesOutEven[nZeileZiel];
		} 
		CurrentPitch=OverlayPitch;
		lpDDOut=&lpDDOverlay;
		Call_Ausgabe=(FARPROC)VCHQAusgebenYUV2;
	} else { 
		memset(&pDecode[0],0x00,m_dwDisplaySize+1024);
		j=0;
		for ( nZeileZiel=1; nZeileZiel < VCCLEARLINES; nZeileZiel++ ) {
			pLinesDstAddressEven[nZeileZiel]=(unsigned short*)lpSurface+(Offset_for_WindowOut/2)+(j*(SurfacePitch/2));
			if (LinesOutEven[nZeileZiel] > 0 )  j+=LinesOutEven[nZeileZiel];
			else j-=LinesOutEven[nZeileZiel];
		}
		CurrentPitch=SurfacePitch;
		lpDDOut=&lpDDSurface;
		if ( WTVBPP == 16 )    Call_Ausgabe=(FARPROC) (VCHQAusgebenRGB16);
		else  Call_Ausgabe=(FARPROC)(VCHQAusgebenRGB15);
	};
	
	BytesOut=OutX*2;
	
	
	
	for (;;) {
		nFrame++;
		if (( MoreLinesLock == FALSE ) && (( nFrame==6 ) || ( nFrame==12 ) || ( nFrame==18 ))) VCIRQSync=FALSE;
		Wait_DStatus();
		if (!VideoPresent ) continue;
		CurrentFrame=NextFrame-1;
		if ( NextFrame== -1 ) {
			SetEvent(Decode_Event_Stop);
			return;
		};
		if ( CurrentFrame < 0 ) CurrentFrame=4;
		VCIrq=CurrentIrq;
		Burst=( pBurstLine[CurrentFrame][3] < 128)?0:1;   //Pal Burst Position 
		
		if ( VCIRQSync == FALSE ) {
			Key=Get_Decoder_Key(&pLinesAddressEven[(CurrentFrame*ZEILENZAHL)]);
			if ( Key == LastKey+1 ) {
				VCIRQSync=TRUE;
				Set_CurrentIrq(Key);
			} ;
			VCIrq=Key;
			LastKey=Key;
		};
		if ( OutDib == FALSE ) {
			if ( WAIT_DX_VBI ) (void)IDirectDraw_WaitForVerticalBlank(lpDD,DDWAITVB_BLOCKBEGIN, NULL);
			if ( USE_DX_LOCK ) ddrval=IDirectDrawSurface_Lock(*lpDDOut, NULL, &ddsd, DDLOCK_WAIT, NULL);
		};
		
		for ( nZeileZiel=2; nZeileZiel < VCCLEARLINES; nZeileZiel++ ) {
			
			LinesAnzahl=LinesOutEven[nZeileZiel];
			if (( LinesAnzahl > 0 ) && ( LinesAnzahl < 5 ) ) {
				VCCut=cp_odd[VCIrq][nZeileZiel-2];
				if ( Chroma_Key == TRUE ) {
					if (((nZeileZiel+Burst)&1) == 1 ) ColFaktor=&VCHQTabelleBurst[VCCut*8];
					else ColFaktor=&VCHQTabelleNoBurst[VCCut*8];
				};
				
				pQuelle=pLinesAddressEven[(CurrentFrame*VCCLEARLINES)+nZeileZiel];
				TmpPtr=ZeilenPuffer;;
				ScreenPtr=pLinesDstAddressEven[nZeileZiel];
				if ( LinesAnzahl == 1 ) TmpPtr=ScreenPtr;
				
				_asm
				{
					mov esi,dword ptr [HQLinebufferEvenPtr];
					mov eax,VCCut
						shl eax,1
						add eax,2    
						mov edx,pQuelle
						mov ebx,Call_Ausgabe
						call ebx
				};	
			};
			
		};
		if ( OutDib == TRUE ) {
			SetDIBitsToDevice( DIB_Hdc,
				0,
				0,
				pDIB->bmiHeader.biWidth,
				ShowY,
				0,
				0,
				0,
				ShowY,
				pDecode,
				pDIB,
				DIB_RGB_COLORS
				);
		} else {
			if ( USE_DX_LOCK ) ddrval=IDirectDrawSurface_Unlock(*lpDDOut,  ddsd.lpSurface);
		};
		
		if ( Display_Status_Bar == TRUE ) {
			if(dwLastCount+960 <= GetTickCount())
			{
				sprintf(Text,"%d FPS",nFrame);
				SetWindowText (hwndFPSField,Text);
				nFrame=0;
				dwLastCount = GetTickCount();
			}
		}
	}
}


BOOL APIENTRY ProgrammListe(
							HWND hDlg,
							UINT message,
							UINT wParam,
							LONG lParam)
{
	int i,j,k,p;
	char zeile[140];
    struct TProgramm save;
	
	static BOOL NextBlock=FALSE;
	LOGFONT Mfont = {15,0,0,0,0,0,0,0,0,0,0,0,0,"Times New Roman"};   
    LPDRAWITEMSTRUCT    lpdis;
    LPMEASUREITEMSTRUCT lpmis;
    RECT       ptrtoWndPos;
    static int currX,currY;
    int fwKeys = wParam;        // key flags 
    int xPos; // horizontal position of cursor 
	int yPos; // vertical position of cursor 
    static HCURSOR hSaveCursor=NULL; 
    static HCURSOR hsizex; 
    static int distance=-9999;
    static int sPos;
	static int EndPos;
	static int SetSel;
    BOOL SetOK;
    static BOOL InitWin=TRUE;
	
	switch (message) {
		
	case WM_INITDIALOG:
		currFont = CreateFontIndirect(&Mfont);
		hsizex = LoadCursor(hInst, "Csizex");
		
		ProgList=CreateWindow("LISTBOX",NULL,
			WS_BORDER |
			WS_CHILD |
			WS_VISIBLE |
			// LBS_NOINTEGRALHEIGHT |
			LBS_NOTIFY |
			// LBS_HASSTRINGS |
			LBS_OWNERDRAWVARIABLE |
			WS_VSCROLL | WS_HSCROLL ,
			//	  WS_HSCROLL |
			//	  LBS_SORT ,
			//       LBS_DISABLENOSCROLL ,
			0, 0, 0, 0,
			hDlg,
			NULL,
			hInst,
			NULL);
		EnableWindow(GetDlgItem(hDlg,IDC_BUTTON1),FALSE);
		EnableWindow(GetDlgItem(hDlg,IDC_BUTTON2),FALSE);
		SetCapture(hDlg);
		
		
		CurSel=Make_ProgNr(LastRealProg);
		if ( pgstarty != -1 ) {
			MoveWindow(hDlg,pgstartx,pgstarty,pgsizex,pgsizey,FALSE);
		}
		
		SetFocus(ProgList);
		PostMessage(ProgList,LB_SETCURSEL,CurSel,0);
		
		/* Fall-through ! */
		
	case RESET_LIST:
		SendMessage(ProgList,LB_RESETCONTENT,0,0);
		i=0;
		while ( i < MAXPROGS ) {
			if ( Programm[i].freq != 0 ) {
				if (IsValidSelect(i)) {
					k=SendMessage(ProgList,LB_ADDSTRING,i,(LPARAM)i);
				}
			}
			i++;
		}
		
		/* Fall-through ! */
		
	case WM_SIZE:
		
		GetWindowRect(hDlg, &ptrtoWndPos);
		currY=ptrtoWndPos.bottom-ptrtoWndPos.top;
		currX=ptrtoWndPos.right-ptrtoWndPos.left;
		currY=currY-90;
		currX=currX-60;
		if ( InitWin == FALSE ) {
			pgstartx=ptrtoWndPos.left;
			pgstarty=ptrtoWndPos.top;
			pgsizex=ptrtoWndPos.right-ptrtoWndPos.left;
			pgsizey=ptrtoWndPos.bottom-ptrtoWndPos.top;
		}
		InitWin=FALSE;
		i=0;
		j=6;
		p=0;
		while ( i < 15 ) {
			ButtonList[i].s=0;
			if (ButtonList[i].FeldId > -1 ) {
				
				if ( j < currX-12 ) {
					ShowWindow(GetDlgItem(hDlg,IDC_MOVE1+i),SW_SHOW);
					GetFeldName(ButtonList[i].FeldId,(char *)&zeile);
					SetWindowText(GetDlgItem(hDlg,IDC_MOVE1+i),zeile);
					k=ButtonList[i].x;
					if ( j + k > currX-12 ) k = ( currX -12) - j;
					MoveWindow(GetDlgItem(hDlg,IDC_MOVE1+i),j,2,k,19,TRUE);
					ButtonList[i].s=j;
					ButtonList[i].r=k;
					j=j+k;
				} else {
					ShowWindow(GetDlgItem(hDlg,IDC_MOVE1+i),SW_HIDE);
				}
			} else {
				p++;
				ShowWindow(GetDlgItem(hDlg,IDC_MOVE1+i),SW_HIDE);
			}
			i++;
		}
		
		if ( p > 0 ) {
			i=0;
			while (( i < 15 ) && (ButtonList[i].FeldId > -1 )) i++;
			if ( i < 15 ) {
				ShowWindow(GetDlgItem(hDlg,IDC_MOVE1+i),SW_SHOW);
				SetWindowText(GetDlgItem(hDlg,IDC_MOVE1+i),"...");
				MoveWindow(GetDlgItem(hDlg,IDC_MOVE1+i),j,2,(currX-j)+6,19,TRUE);
				ButtonList[i].x=(currX-j)+6;
				ButtonList[i].s=j;
				ButtonList[i].r=(currX-j)+6;
			}
		}
		
		MoveWindow(ProgList,
			6,
			25,
			currX,
			currY,
			TRUE);
		
		MoveWindow(GetDlgItem(hDlg,IDOK2),
			currX-20,
			40+currY,
			60,
			20,
			TRUE);
		
		MoveWindow(GetDlgItem(hDlg,IDM_NEU),
			6,
			40+currY,
			60,
			20,
			TRUE);
		
		MoveWindow(GetDlgItem(hDlg,IDM_KONFIG),
			currX/2-16,
			40+currY,
			80,
			20,
			TRUE);
		
		
		MoveWindow(GetDlgItem(hDlg,IDC_BUTTON1),
			currX+60-40,
			25,
			30,
			20,
			TRUE);
		
		MoveWindow(GetDlgItem(hDlg,IDC_BUTTON2),
			currX+60-40,
			25+currY-20,
			30,
			20,
			TRUE);
		
		
		break;
		
   case WM_MOVE:
	   
	   GetWindowRect(hDlg, &ptrtoWndPos);
	   if ( InitWin == FALSE ) {
		   pgstartx=ptrtoWndPos.left;
		   pgstarty=ptrtoWndPos.top;
		   pgsizex=ptrtoWndPos.right-ptrtoWndPos.left;
		   pgsizey=ptrtoWndPos.bottom-ptrtoWndPos.top;
	   }
	   break;
	   
   case WM_LBUTTONUP:
       xPos = LOWORD(lParam);  // horizontal position of cursor
       yPos = HIWORD(lParam);  // vertical position of cursor
       if (( yPos >= 2 ) &&  ( yPos <= 21 )) {
		   if (( distance == -9999 ) && ( hSaveCursor == NULL )) {
			   i=0;
			   while ( i < 15 ) {
				   if (xPos >= ButtonList[i].s + 2 && xPos <= ButtonList[i].s+ButtonList[i].r - 4) {
					   SelectButton=i;
					   if ( ButtonList[SelectButton].FeldId == -1 ) {
						   j=0;
						   while (( j < 15 ) && (ButtonList[j].FeldId != -1 )) j++;
						   SelectButton=j;
					   }
					   i=SendMessage(ProgList,LB_GETCURSEL,0,0);
					   DialogBox(hInst,"LISTFELDSETUP",hDlg,(DLGPROC)ListFeldSettingProc);
					   SetFocus(ProgList);
					   SendMessage(hDlg,RESET_LIST,0,0);
					   SendMessage(ProgList,LB_SETCURSEL,i,0);
					   return(TRUE);
				   }
				   i++;
			   }
		   }
	   }
	   break;
	   
	   
   case WM_MOUSEMOVE:
	   fwKeys = wParam;        // key flags
	   xPos = LOWORD(lParam);  // horizontal position of cursor
	   yPos = HIWORD(lParam);  // vertical position of cursor
	   SetOK=FALSE;
	   if (( yPos >= 2 ) &&  ( yPos <= 21 )) {
		   
		   if (( distance != -9999 ) && ( fwKeys == MK_LBUTTON )) {
			   p=ButtonList[EndPos].x;
			   k=ButtonList[EndPos+1].x;
			   p=p+(xPos-distance);
			   k=k-(xPos-distance);
			   distance=xPos;
			   if (( p > 4 ) && ( k > 4 )) {
				   ButtonList[EndPos].x=p;
				   ButtonList[EndPos+1].x=k;
				   ButtonList[EndPos].s=sPos;
				   ButtonList[EndPos+1].s=sPos+ButtonList[EndPos].x;
				   
				   MoveWindow(GetDlgItem(hDlg,IDC_MOVE1+EndPos),sPos,2,ButtonList[EndPos].x,19,TRUE);
				   
				   MoveWindow(GetDlgItem(hDlg,IDC_MOVE1+EndPos+1),sPos+ButtonList[EndPos].x,2,ButtonList[EndPos+1].x,19,TRUE);
				   InvalidateRect(ProgList,NULL,TRUE);
				   return(TRUE);
			   }
		   }
		   distance=-9999;
		   j=4;
		   sPos=6;
		   i=0;
		   while ( i < 14 ) {
			   if ( ButtonList[i+1].FeldId != -1 ) {
				   j=j+ButtonList[i].x;
				   if (( xPos >= j ) && ( xPos <= j+4 )) {
					   SetOK=TRUE;
					   if ( hSaveCursor == NULL ) {
						   hSaveCursor = SetCursor(hsizex);
						   SetCapture(hDlg);
					   }
					   if ( fwKeys == MK_LBUTTON ) {
						   if ( distance == -9999 ) {
							   distance=xPos;
							   EndPos=i;
							   return(TRUE);
						   }
					   }
				   }
			   }
			   sPos=sPos+ButtonList[i].x;
			   i++;
		   }
	   }
	   if (( SetOK == FALSE ) && (hSaveCursor != NULL )) {
		   SetCursor(hSaveCursor );
		   ReleaseCapture();
		   hSaveCursor = NULL ;
	   }
	   
	   break;
	   
	   
	   
   case WM_DRAWITEM:
	   /* Get pointer to the DRAWITEMSTRUCT */
	   lpdis = (LPDRAWITEMSTRUCT)lParam;
	   
	   if (lpdis->itemID == -1) {
	   /* We have a request to draw an item in the list box, yet there
	   * are no list box items. This is sent when the user TABS into
	   * an empty list box or an empty list box gets the focus. We
	   * have to indicate (somehow) that this owner-draw list box has
	   * the focus. We do it in response to this message. Note that
	   * lpdis->itemData field would be invalid in this instance so
	   * we can't allow it to fall into our standard routines.
		   */
		   HandleFocusStateKanalListe(ProgList,lpdis);
	   } else {
		   switch (lpdis->itemAction) {
		   case ODA_DRAWENTIRE:
			   DrawEntireItemKanalListe(ProgList,lpdis,0);
			   break;
			   
		   case ODA_SELECT:
			   HandleFocusStateKanalListe(ProgList,lpdis);
			   break;
			   
		   case ODA_FOCUS:
			   HandleFocusStateKanalListe(ProgList,lpdis);
			   break;
		   }
	   }
	   
	   /* Return TRUE meaning that we processed this message. */
	   return(TRUE);
	   break;
	   
   case WM_MEASUREITEM:
	   lpmis = (LPMEASUREITEMSTRUCT)lParam;
	   
	   /* All the items are the same height since the list box style is
	   * LBS_OWNERDRAWFIXED
	   */
	   lpmis->itemHeight = 14;
	   return(TRUE);
	   break;
	   
	   
   case WM_CHARTOITEM:
	   if ( (HWND)lParam == ProgList ) {
		   j=CurSel;
		   NextBlock=TRUE;
		   i=HIWORD(wParam);
		   if (( LOWORD(wParam) >= 0x30 ) && ( LOWORD(wParam) <= 0x39 )) {
			   EditProgramm=i;
			   KeyValue=(char)LOWORD(wParam);
			   i=DialogBox(hInst,"KANALNUMMER",hDlg,(DLGPROC)KanalNummerProc);
			   if ( i > 0 ) {
				   k=Get_Real_ProgNr(i);
				   i=k;
			   };
			   if ( i >= 0 ) {
				   memcpy(&save.Name[0],&Programm[CurSel].Name[0],sizeof(struct TProgramm));
				   for ( j=CurSel+1; j < MAXPROGS; j++) {
					   memcpy(&Programm[j-1].Name,&Programm[j].Name[0],sizeof(struct TProgramm));
				   };
				   for ( j=MAXPROGS-2; j >= i ; j--) {
					   memcpy(&Programm[j+1].Name,&Programm[j].Name[0],sizeof(struct TProgramm));
				   };
				   memcpy(&Programm[i].Name[0],&save.Name[0],sizeof(struct TProgramm));
				   Write_Programm_List();
				   SetFocus(ProgList);
				   SetSel=Make_ProgNr(i);
				   CurSel=i;
				   SendMessage(hDlg,RESET_LIST,0,0);
				   SendMessage(ProgList,LB_SETCURSEL,SetSel,0);
			   }
		   } else  if ( LOWORD(wParam) == 0x64 )  {
			   
			   for ( j=CurSel; j < MAXPROGS-1; j++) {
				   memcpy(&Programm[j].Name[0],&Programm[j+1].Name[0],sizeof(struct TProgramm));
			   };
			   memset(&Programm[MAXPROGS-1].Name[0],0x00,sizeof(struct TProgramm));
			   Write_Programm_List();
			   SetFocus(ProgList);
			   SetSel=Make_ProgNr(CurSel);
			   SendMessage(hDlg,RESET_LIST,0,0);
			   
			   SendMessage(ProgList,LB_SETCURSEL,SetSel,0);
		   }
	   };
	   return(TRUE);
	   
	   
   case WM_COMMAND:
	   
	   if ( (HWND)lParam==ProgList ){
		   if (HIWORD(wParam)==LBN_SELCHANGE){
			   if ( NextBlock ==TRUE ) {
				   NextBlock=FALSE;
				   PostMessage(ProgList,LB_SETCURSEL,SetSel,0);
				   return(TRUE);
			   };
			   i=SendMessage(ProgList,LB_GETCURSEL,0,0);
			   if ( i > 0 ) EnableWindow(GetDlgItem(hDlg,IDC_BUTTON1),TRUE);
			   else EnableWindow(GetDlgItem(hDlg,IDC_BUTTON1),FALSE);
			   if ( i+1 < SendMessage(ProgList,LB_GETCOUNT,0,0)) EnableWindow(GetDlgItem(hDlg,IDC_BUTTON2),TRUE);
			   else EnableWindow(GetDlgItem(hDlg,IDC_BUTTON2),FALSE);
			   
			   i= Get_Real_ProgNr(i);
			   
			   if (( i >= 0 ) && ( i < MAXPROGS )) {
				   CurSel=i;
				   if ( i != CurrentProgramm ) {
					   if (ValidModes(Programm[i].Typ) == TRUE  ) {
						   CurrentProgramm=i;
						   if (( Current_Mode == 0 ) && ( Programm[CurrentProgramm].Typ == 'D' )) Set_Mode_Digital();
						   else if (( Current_Mode == 1 ) && ( Programm[CurrentProgramm].Typ == 'A' )) Set_Mode_Analog();
						   
						   if ( Display_Status_Bar == TRUE ) {
							   sprintf(Text,"%04d. %s ",Make_ProgNr(CurrentProgramm)+1,Programm[CurrentProgramm].Name);
							   SetWindowText (hwndKeyField,Text);
						   }
						   
						   if (Programm[CurrentProgramm].Typ == 'A' ) Tuner_SetFrequency(TunerType,MulDiv(Programm[CurrentProgramm].freq*1000,16,1000000));
						   else Set_DVB_Channel(CurrentProgramm);
					   }
				   };
			   };
		   };
		   
		   if (HIWORD(wParam)==LBN_DBLCLK) {
			   i=SendMessage(ProgList,LB_GETCURSEL,0,0);
			   j=i;
			   i= Get_Real_ProgNr(i);
			   EditProgramm=i;
			   CurSel=i;
			   if (( i >= 0 ) && ( i < MAXPROGS )) {
				   if ( Display_Status_Bar == TRUE ) {
					   sprintf(Text,"%04d. %s ",Make_ProgNr(CurrentProgramm)+1,Programm[CurrentProgramm].Name);
					   SetWindowText (hwndKeyField,Text);
				   }
				   
				   DialogBox(hInst,"PROGRAMM",hWnd,(DLGPROC)ProgrammEdit);
				   SetFocus(ProgList);
				   SendMessage(ProgList,LB_SETCURSEL,j,0);
				   InvalidateRect(ProgList,NULL,TRUE);
				   
			   }
		   }
	   }
	   
	   if (LOWORD(wParam) == IDC_BUTTON2 ) {
		   i=SendMessage(ProgList,LB_GETCURSEL,0,0);
		   j= Get_Real_ProgNr(i+1);
		   i= Get_Real_ProgNr(i);
		   if ( j < MAXPROGS ) {
			   
			   if ( Programm[j].freq != 0 ) {
				   memcpy(&save,&Programm[i],sizeof(struct TProgramm));
				   memcpy(&Programm[i],&Programm[j],sizeof(struct TProgramm));
				   memcpy(&Programm[j],&save,sizeof(struct TProgramm));
				   SetFocus(ProgList);
				   i=SendMessage(ProgList,LB_GETCURSEL,0,0);
				   SendMessage(ProgList,LB_SETCURSEL,i+1,0);
				   InvalidateRect(ProgList,NULL,TRUE);
				   CurSel=Get_Real_ProgNr(i+1);
				   Write_Programm_List();
			   }
		   }
	   };
	   
	   if (LOWORD(wParam) == IDC_BUTTON1 ) {
		   i=SendMessage(ProgList,LB_GETCURSEL,0,0);
		   j= Get_Real_ProgNr(i-1);
		   i= Get_Real_ProgNr(i);
		   if ( i>= 1  ) {
			   memcpy(&save,&Programm[i],sizeof(struct TProgramm));
			   memcpy(&Programm[i],&Programm[j],sizeof(struct TProgramm));
			   memcpy(&Programm[j],&save,sizeof(struct TProgramm));
			   SetFocus(ProgList);
			   i=SendMessage(ProgList,LB_GETCURSEL,0,0);
			   SendMessage(ProgList,LB_SETCURSEL,i-1,0);
			   InvalidateRect(ProgList,NULL,TRUE);
			   Write_Programm_List();
			   CurSel=Get_Real_ProgNr(i-1);
			   
		   }
	   }
	   
	   if (LOWORD(wParam) == IDM_KONFIG ) {
		   j=SendMessage(ProgList,LB_GETCURSEL,0,0);
		   i=DialogBox(hInst,"PROGRAMMLISTEKONFIG",hDlg,(DLGPROC)ListKonfigProc);
		   if ( i == 1 ) SendMessage(hDlg,RESET_LIST,i-1,0);
		   else {
			   SetFocus(ProgList);
			   SendMessage(ProgList,LB_SETCURSEL,j,0);
		   };
	   }
	   
	   if (LOWORD(wParam) == IDM_NEU ) {
		   i=0;
		   
		   while ( i < MAXPROGS ) {
			   if ( Programm[i].freq == 0 ) {
				   j=i;
				   i=MAXPROGS;
			   };
			   i++;
		   }
		   Programm[j].freq=0;
		   Programm[j].Name[0]=0x00;
		   Programm[j].Typ='A';
		   // ggf Mehr
		   EditProgramm=j;
		   DialogBox(hInst,"PROGRAMM",hWnd,(DLGPROC)ProgrammEdit);
		   SetFocus(hDlg);
		   if ( Programm[j].freq != 0 ) {
			   CurSel=j;
			   CurrentProgramm=j;
		   };
		   SendMessage(hDlg,RESET_LIST,CurrentProgramm,0);
		   
	   };
	   
	   
	   if (LOWORD(wParam) == IDOK
		   || LOWORD(wParam) == IDCANCEL) {
		   EndDialog(hDlg,TRUE);
	   }
	   
	   break;
     }
     return (FALSE);
	 UNREFERENCED_PARAMETER(lParam);
 }
 
 
 BOOL APIENTRY ListKonfigProc(
	 HWND hDlg,
	 UINT message,
	 UINT wParam,
	 LONG lParam)
 {
	 switch (message) {
	 case WM_INITDIALOG:
		 CheckDlgButton(hDlg,IDC_CHECK1,VIEW_ANALOG);
		 CheckDlgButton(hDlg,IDC_CHECK2,VIEW_DIGITV);
		 CheckDlgButton(hDlg,IDC_CHECK3,VIEW_DIGIRADIO);
		 CheckDlgButton(hDlg,IDC_CHECK4,VIEW_DIGI_REST);
		 CheckDlgButton(hDlg,IDC_CHECK5,VIEW_FREE);
		 CheckDlgButton(hDlg,IDC_CHECK6,VIEW_TEMP_CHANNEL);
		 CheckDlgButton(hDlg,IDC_CHECK7,VIEW_LINK_CHANNEL);
		 break;
		 
	 case WM_COMMAND:
		 if (LOWORD(wParam)==IDCANCEL) EndDialog(hDlg,0);
		 
		 if ( LOWORD(wParam)==IDOK ) {
			 VIEW_ANALOG=IsDlgButtonChecked(hDlg,IDC_CHECK1);
			 VIEW_DIGITV=IsDlgButtonChecked(hDlg,IDC_CHECK2);
			 VIEW_DIGIRADIO=IsDlgButtonChecked(hDlg,IDC_CHECK3);
			 VIEW_DIGI_REST=IsDlgButtonChecked(hDlg,IDC_CHECK4);
			 VIEW_FREE=IsDlgButtonChecked(hDlg,IDC_CHECK5);
			 VIEW_TEMP_CHANNEL=IsDlgButtonChecked(hDlg,IDC_CHECK6);
			 VIEW_LINK_CHANNEL=IsDlgButtonChecked(hDlg,IDC_CHECK7);
			 EndDialog(hDlg,1);
		 }
		 break;
	 };
	 return (FALSE);
	 UNREFERENCED_PARAMETER(lParam);
 }
 
 
 BOOL IsValidSelect(int Nr )
 {
	 
	 if (( Nr < 0 ) || ( Nr > MAXPROGS+1 )) return(FALSE);
	 
	 if (Programm[Nr].Typ == 'A' ) return(VIEW_ANALOG);
	 if (Programm[Nr].Typ != 'D' ) return(FALSE);
	 if (( Programm[Nr].Link_SID != 0xffff ) && ( Programm[Nr].Link_TP != 0xffff )) return(VIEW_LINK_CHANNEL);
	 if ( Programm[Nr].Dynamisch == 0x01 ) return(VIEW_TEMP_CHANNEL);
	 if (( VIEW_FREE == TRUE ) && ( Programm[Nr].CA_ID != 0 )) return(FALSE);
	 if (( VIEW_DIGITV == TRUE ) && (Programm[Nr].Video_pid != 0x1fff)) return(TRUE);
	 if (( VIEW_DIGITV == TRUE ) && (Programm[Nr].ServiceTyp == 0 )) return(TRUE);
	 if (( VIEW_DIGITV == TRUE ) && (Programm[Nr].ServiceTyp == 1 )) return(TRUE);
	 if (( VIEW_DIGITV == TRUE ) && (Programm[Nr].ServiceTyp == 4 )) return(TRUE);
	 if (( VIEW_DIGITV == TRUE ) && (Programm[Nr].ServiceTyp == 5 )) return(TRUE);
	 if (( VIEW_DIGIRADIO == TRUE ) && (Programm[Nr].Video_pid == 0x1fff) && ( Programm[Nr].Audio_pid != 0x1fff)) return(TRUE);
	 if (( VIEW_DIGIRADIO == TRUE ) && (Programm[Nr].ServiceTyp == 0x02 )) return(TRUE);
	 if (( VIEW_DIGI_REST == TRUE ) && (Programm[Nr].Video_pid == 0x1fff) && ( Programm[Nr].Audio_pid == 0x1fff)) {
		 if ( Programm[Nr].ServiceTyp == 0 ) return(FALSE); 
		 if (Programm[Nr].ServiceTyp  == 1  ) return(FALSE);
		 if (Programm[Nr].ServiceTyp  == 2 ) return(FALSE);
		 if (Programm[Nr].ServiceTyp == 4 ) return(FALSE);
		 if (Programm[Nr].ServiceTyp == 5 ) return(FALSE);
		 return(TRUE);
	 };
	 
	 return(FALSE);
 };
 
 
 int Make_ProgNr( int Nr )
 {
	 int i=0;
	 int j=-1;
	 while (( j < Nr ) && ( j < MAXPROGS ))
	 {
		 if ( IsValidSelect(j) == TRUE )
			 i++;
		 j++;
	 };
	 return(i);
 };
 
 
 int Get_Real_ProgNr( int Nr )
 {
	 int i=-1;
	 int j=0;
	 while (( i != Nr ) && ( j < MAXPROGS )) {
		 if ( IsValidSelect(j) == TRUE ) {
			 i++;
		 };
		 j++;
	 };
	 return(j-1);
 };
 
 
 BOOL APIENTRY EventListe(
	 HWND hDlg,
	 UINT message,
	 UINT wParam,
	 LONG lParam)
 {
	 struct Sort_List_t {
		 unsigned short ProgrammNummer;
		 unsigned int DatumZeit;
		 unsigned short Eit_Index_Id;
		 unsigned short Eit_Index_Offset;
	 };
	 struct Sort_List_t	Save;
	 struct Sort_List_t	*Sort_List;
	 int size = sizeof( struct Sort_List_t );
	 int ListCount=0;
	 int i,j,k;
	 BOOL Austausch;
	 LOGFONT Mfont = {15,0,0,0,0,0,0,0,0,0,0,0,0,"Times New Roman"};   
	 LPDRAWITEMSTRUCT    lpdis;
	 LPMEASUREITEMSTRUCT lpmis;
	 RECT       ptrtoWndPos;
	 static int currX,currY;
	 static BOOL InitWin=TRUE;
	 switch (message) {
	 case WM_INITDIALOG:
		 currFont = CreateFontIndirect(&Mfont);
		 ProgList=CreateWindow("LISTBOX",NULL,
			 WS_BORDER | 
			 WS_CHILD | 
			 WS_VISIBLE |
			 // LBS_NOINTEGRALHEIGHT | 
			 LBS_NOTIFY |   
			 // LBS_HASSTRINGS | 
			 LBS_OWNERDRAWVARIABLE |
			 WS_VSCROLL | WS_HSCROLL ,
			 //	  WS_HSCROLL |
			 //	  LBS_SORT , 
			 //       LBS_DISABLENOSCROLL ,
			 0, 0, 0, 0,
			 hDlg,
			 NULL,
			 hInst,
			 NULL);
		 
		 CurSel=CurrentProgramm;
		 if ( evstarty != -1 ) {
			 MoveWindow(hDlg,evstartx,evstarty,evsizex,evsizey,FALSE);
		 };
		 
	 case RESET_LIST:
		 SendMessage(ProgList,LB_RESETCONTENT,0,0);
		 i=0;
		 Sort_List=calloc(65537,size);
		 
	        ListCount = 0;
			for ( i = 0 ; i < EIT_INDEX_SIZE ; i++ ) {
				if ( EIT[i].Anzahl > 0 ) {
					for ( j = 0 ; j < EIT[i].Anzahl; j++ ) {
						if (( prozess_kanal(i,j,EIT_Kanal) == TRUE ) && (  prozess_DatumZeit(i,j,EIT_Datum) == TRUE )) {
							Sort_List[ListCount].ProgrammNummer=Get_Sender(EIT[i].EIT_Infos[j].SID_Id,EIT[i].EIT_Infos[j].ts_id,NULL);
							Sort_List[ListCount].DatumZeit=(EIT[i].EIT_Infos[j].Date_Id << 16 ) + ( EIT[i].EIT_Infos[j].Start_Zeit_h<<8)+EIT[i].EIT_Infos[j].Start_Zeit_m; 
							Sort_List[ListCount].Eit_Index_Id=i;
							Sort_List[ListCount].Eit_Index_Offset=j;
							ListCount++;
						};
					};
				};
			};
			
            if ( EIT_Sort == 1 ) {
				
				j=ListCount;
				Austausch = TRUE ;
				
				while ( Austausch == TRUE ) {
					Austausch = FALSE ;
					j--;
					i=0;
					while ( i < j ) {
						if ( Sort_List[i].DatumZeit > Sort_List[i+1].DatumZeit ) {
							Austausch = TRUE;
							memcpy(&Save.ProgrammNummer, &Sort_List[i+1].ProgrammNummer , size );
							memcpy(&Sort_List[i+1].ProgrammNummer, &Sort_List[i].ProgrammNummer , size );
							memcpy(&Sort_List[i].ProgrammNummer,&Save.ProgrammNummer , size );
						} else if ( Sort_List[i].DatumZeit == Sort_List[i+1].DatumZeit ) {
							if ( Sort_List[i].ProgrammNummer > Sort_List[i+1].ProgrammNummer ) {
								Austausch = TRUE;
								memcpy(&Save.ProgrammNummer, &Sort_List[i+1].ProgrammNummer , size );
								memcpy(&Sort_List[i+1].ProgrammNummer, &Sort_List[i].ProgrammNummer , size );
								memcpy(&Sort_List[i].ProgrammNummer,&Save.ProgrammNummer , size );
							};
						};
						i++;
					};
				}
			} // Sort 1
			
			
            if ( EIT_Sort == 2 ) {
				j=ListCount;
				Austausch = TRUE ;
				while ( Austausch == TRUE ) {
					Austausch = FALSE ;
					j--;
					i=0;
					while ( i < j ) {
						
						if ( Sort_List[i].ProgrammNummer > Sort_List[i+1].ProgrammNummer ) {
							Austausch = TRUE;
							memcpy(&Save.ProgrammNummer, &Sort_List[i+1].ProgrammNummer , size );
							memcpy(&Sort_List[i+1].ProgrammNummer, &Sort_List[i].ProgrammNummer , size );
							memcpy(&Sort_List[i].ProgrammNummer,&Save.ProgrammNummer , size );
						} else if ( Sort_List[i].ProgrammNummer == Sort_List[i+1].ProgrammNummer ) {
							if ( Sort_List[i].DatumZeit > Sort_List[i+1].DatumZeit ) {
								Austausch = TRUE;
								memcpy(&Save.ProgrammNummer, &Sort_List[i+1].ProgrammNummer , size );
								memcpy(&Sort_List[i+1].ProgrammNummer, &Sort_List[i].ProgrammNummer , size );
								memcpy(&Sort_List[i].ProgrammNummer,&Save.ProgrammNummer , size );
							};
						};
						i++;
					};
				}
            } // Sort 2			
			i=0;
			while ( i < ListCount ) {
				k=SendMessage(ProgList,LB_ADDSTRING,i,(LPARAM)(Sort_List[i].Eit_Index_Id<<16)+Sort_List[i].Eit_Index_Offset);
				i++;
			};
            free(Sort_List);
            break;
			
	 case WM_SIZE:
		 
         GetWindowRect(hDlg,
			 &ptrtoWndPos);
		 currY=ptrtoWndPos.bottom-ptrtoWndPos.top;
		 currX=ptrtoWndPos.right-ptrtoWndPos.left;
		 currY=currY-60;
		 currX=currX-20;
		 if ( InitWin == FALSE ) {
			 evstartx=ptrtoWndPos.left;
			 evstarty=ptrtoWndPos.top;
			 evsizex=ptrtoWndPos.right-ptrtoWndPos.left;
			 evsizey=ptrtoWndPos.bottom-ptrtoWndPos.top;
		 };
         InitWin=FALSE;
         MoveWindow(ProgList,
			 5,
			 5,
			 currX,
			 currY,
			 TRUE);
		 
         MoveWindow(GetDlgItem(hDlg,IDCANCEL),
			 currX-55,
			 10+currY,
			 60,
			 20,
			 TRUE);
		 
         MoveWindow(GetDlgItem(hDlg,IDKONFIG),
			 (currX/2)+6-34,
			 10+currY,
			 80,
			 20,
			 TRUE);
		 
         MoveWindow(GetDlgItem(hDlg,IDOK),
			 6,
			 10+currY,
			 60,
			 20,
			 TRUE);
		 break;
		 
	 case WM_MOVE:
		 
         GetWindowRect(hDlg,&ptrtoWndPos);
		 if ( InitWin == FALSE ) {
			 evstartx=ptrtoWndPos.left;
			 evstarty=ptrtoWndPos.top;
			 evsizex=ptrtoWndPos.right-ptrtoWndPos.left;
			 evsizey=ptrtoWndPos.bottom-ptrtoWndPos.top;
		 };
		 break;
		 
		 
		 
	 case WM_DRAWITEM:
		 /* Get pointer to the DRAWITEMSTRUCT */
		 lpdis = (LPDRAWITEMSTRUCT)lParam;
		 
		 if (lpdis->itemID == -1)
		 {
		 /* We have a request to draw an item in the list box, yet there
		 * are no list box items. This is sent when the user TABS into
		 * an empty list box or an empty list box gets the focus. We
		 * have to indicate (somehow) that this owner-draw list box has
		 * the focus. We do it in response to this message. Note that
		 * lpdis->itemData field would be invalid in this instance so
		 * we can't allow it to fall into our standard routines.
			 */
			 HandleFocusStateEventListe(ProgList,lpdis);
		 }
		 else
		 {
			 switch (lpdis->itemAction)
			 {
			 case ODA_DRAWENTIRE:
				 DrawEntireItemEventListe(ProgList,lpdis,0);
				 break;
				 
			 case ODA_SELECT:
				 HandleFocusStateEventListe(ProgList,lpdis);
				 break;
				 
			 case ODA_FOCUS:
				 HandleFocusStateEventListe(ProgList,lpdis);
				 break;
			 }
		 }
		 
		 /* Return TRUE meaning that we processed this message. */
		 return(TRUE);
		 break;
		 
	 case WM_MEASUREITEM:
		 lpmis = (LPMEASUREITEMSTRUCT)lParam;
		 
		 /* All the items are the same height since the list box style is
		 * LBS_OWNERDRAWFIXED
		 */
		 lpmis->itemHeight = 14;
		 return(TRUE);
		 
	 case WM_COMMAND:
		 
         if ( (HWND)lParam==ProgList ){
			 
			 if (HIWORD(wParam)==LBN_DBLCLK) {
				 i=SendMessage(ProgList,LB_GETCURSEL,0,0);
				 SendMessage(ProgList,LB_GETTEXT,i,(LPARAM)&EIT_Message);
				 j=DialogBox(hInst,"EITDETAIL",hDlg,(DLGPROC)EITDetail_Proc);
				 SetFocus(hDlg);
			 }
		 }
		 
         if (LOWORD(wParam) == IDKONFIG ) {
			 DialogBox(hInst,"EVENTSETTING",hDlg,EITSettingProc);
			 PostMessage(hDlg, RESET_LIST,0,0);
			 break;
		 }
		 
		 if (LOWORD(wParam) == IDOK
			 || LOWORD(wParam) == IDCANCEL) {
			 EndDialog(hDlg,TRUE);
		 }
		 
		 break;
    }
    return (FALSE);
	UNREFERENCED_PARAMETER(lParam);
}


BOOL prozess_DatumZeit(int i_id, int o_id ,unsigned char setting)
{
	if ( setting == 0x00 ) return(TRUE);
    if (( setting == 1 ) && ( EIT[i_id].EIT_Infos[o_id].Running_Status > 0 )) return(TRUE);
    if (( setting == 2 ) && ( EIT[i_id].EIT_Infos[o_id].Date_Id == TDT.Date_Id )) return(TRUE);
    if (( setting == 3 ) && ( EIT[i_id].EIT_Infos[o_id].Date_Id >= TDT.Date_Id )) return(TRUE);
	return(FALSE);
};

BOOL prozess_kanal(int i_id, int o_id ,unsigned char setting)
{
	int i,k;
	
	k=Get_Sender(EIT[i_id].EIT_Infos[o_id].SID_Id,EIT[i_id].EIT_Infos[o_id].ts_id,NULL);
	if ( IsValidSelect(k) == FALSE ) return(FALSE);
	
	
	if ( setting == 0x00 ) return(TRUE);
	if ( Programm[CurrentProgramm].Typ == 'A' ) return(FALSE);
	if (( setting == 1 ) && ( EIT[i_id].EIT_Infos[o_id].SID_Id == Programm[CurrentProgramm].SID_pid) && ( EIT[i_id].EIT_Infos[o_id].ts_id == Programm[CurrentProgramm].tp_id )) return(TRUE);
	
	if ( setting == 2 ) {
		for ( i=0; i < MAXFAVORITEN; i++ ) {
			if  (( EIT[i_id].EIT_Infos[o_id].SID_Id == Favoriten[i].SID_Id) && ( EIT[i_id].EIT_Infos[o_id].ts_id == Favoriten[i].ts_id )) return(TRUE);
        };
	};
	return(FALSE);
};



VOID APIENTRY HandleFocusStateEventListe(HWND hDlg,LPDRAWITEMSTRUCT lpdis)
{
	DrawEntireItemEventListe(
		hDlg,
		lpdis,
		(lpdis->itemState & ODS_FOCUS || lpdis->itemState & ODS_SELECTED) ? 1 : 0
		);
}


VOID APIENTRY DrawEntireItemEventListe(HWND hDlg,LPDRAWITEMSTRUCT lpdis,INT Typ)
{
	HFONT OldFont;
	int Sender;
	char SenderName[30];
	unsigned char Zeile[257];
	int x;
	
	int i_id,o_id;
	int Virtual_Event_Id;
	int Virtual_Offset_Id;
	
	i_id=lpdis->itemData>>16;
	o_id=lpdis->itemData&0xffff;
	
	Sender=Get_Sender(EIT[i_id].EIT_Infos[o_id].SID_Id,EIT[i_id].EIT_Infos[o_id].ts_id,(char *)&SenderName);
	
	   if ( Typ == 0 ) {
		   
		   
		   if ( EIT[i_id].EIT_Infos[o_id].Running_Status > 0 ) {
			   if ( EIT[i_id].EIT_Infos[o_id].Running_Status == 2 ) SetTextColor(lpdis->hDC,RGB(255,0,0));
			   else SetTextColor(lpdis->hDC,RGB(0,0,255));
			   SetBkColor(lpdis->hDC,RGB(255,255,255));
		   } else {
			   SetTextColor(lpdis->hDC,RGB(0,0,0));
			   SetBkColor(lpdis->hDC,RGB(255,255,255));
		   };
	   } else {
		   SetTextColor(lpdis->hDC,RGB(255,255,255));
		   SetBkColor(lpdis->hDC,RGB(0,0,255));
	   }
	   
	   OldFont = SelectObject(lpdis->hDC, currFont);
	   
	   x=10;
	   if (( EIT_Datum == 0 ) || ( EIT_Datum == 3 )) {  
            Date_To_String((unsigned short)RealDay(EIT[i_id].EIT_Infos[o_id].Start_Zeit_h,
                                   EIT[i_id].EIT_Infos[o_id].Date_Id),Zeile);
		   Zeile[8]=0x00;
		   TextOut(lpdis->hDC,x,lpdis->rcItem.top+1,Zeile,strlen(Zeile));
		   x=x+65;
	   };
	   sprintf(Zeile,"%02d",RealTime(EIT[i_id].EIT_Infos[o_id].Start_Zeit_h));
	   TextOut(lpdis->hDC,x,lpdis->rcItem.top+1,Zeile,strlen(Zeile));
	   strcpy(Zeile,":");
	   x=x+11;
	   TextOut(lpdis->hDC,x,lpdis->rcItem.top+1,Zeile,strlen(Zeile));
	   sprintf(Zeile,"%02d                ",EIT[i_id].EIT_Infos[o_id].Start_Zeit_m);
	   x=x+3;
	   TextOut(lpdis->hDC,x,lpdis->rcItem.top+1,Zeile,strlen(Zeile));
	   x=x+16;
	   if ( EIT_Kanal != 1 ) {
		   if ( Sender >= 0 ) {
			   sprintf(Zeile,"%4d.                 ",Make_ProgNr(Sender)+1);
			   TextOut(lpdis->hDC,x,lpdis->rcItem.top+1,Zeile,strlen(Zeile));
			   sprintf(Zeile,"%s.                                                                              ",SenderName);
			   x=x+25;
			   TextOut(lpdis->hDC,x,lpdis->rcItem.top+1,Zeile,strlen(Zeile));
			   x=x+190;
		   } else {
			   sprintf(Zeile,"Sender ist nicht in Programmliste                                                   ");
			   TextOut(lpdis->hDC,x,lpdis->rcItem.top+1,Zeile,strlen(Zeile));
			   x=x+215;
		   }
	   };
	   
	   
	   
	   Virtual_Event_Id=i_id;
	   Virtual_Offset_Id=o_id;
	   
	   if (( EIT[i_id].EIT_Infos[o_id].virtual_eventid != 0xffff ) &&
		   ( EIT[i_id].EIT_Infos[o_id].virtual_sid != 0xffff )) {
		   i=Get_Virtual_Event(EIT[i_id].EIT_Infos[o_id].virtual_eventid,EIT[i_id].EIT_Infos[o_id].virtual_sid);
		   if ( i >= 0 ) {
			   Virtual_Event_Id=EIT[i_id].EIT_Infos[o_id].virtual_eventid;
			   Virtual_Offset_Id=i;
		   }
	   }
	   
	   if ( EIT[Virtual_Event_Id].EIT_Infos[Virtual_Offset_Id].TextSize > 0 ) {
		   strcpy(Zeile,"*    ");
		   TextOut(lpdis->hDC,x,lpdis->rcItem.top+1,Zeile,strlen(Zeile));
	   }
	   x=x+10;
	   i=0;
	   
	   
	   if ( EIT[Virtual_Event_Id].EIT_Infos[Virtual_Offset_Id].HeaderSize > 0 ) {
		   while (( EIT[Virtual_Event_Id].EIT_Infos[Virtual_Offset_Id].Header[i] != 0x00 ) && ( i < 255 )) {
			   if ( EIT[Virtual_Event_Id].EIT_Infos[Virtual_Offset_Id].Header[i] == 0x0a ) Zeile[i]= 0x2d;
			   else Zeile[i]=EIT[Virtual_Event_Id].EIT_Infos[Virtual_Offset_Id].Header[i];
			   i++;
		   };
		   while ( i < 256 ) Zeile[i++] = 0x20;
		   Zeile[i] = 0x00;
		   
		   TextOut(lpdis->hDC,x,lpdis->rcItem.top+1,Zeile,strlen(Zeile));
	   } else {
		   sprintf(Zeile,"< Kein Titel geladen >");
		   TextOut(lpdis->hDC,x,lpdis->rcItem.top+1,Zeile,strlen(Zeile));
		   
	   }
	   
	   SelectObject(lpdis->hDC,OldFont);
	   return;
}

BOOL APIENTRY EITSettingProc(HWND hDlg,UINT message,UINT wParam,LONG lParam)
{
	int i,j,k,l,m;
	char Zeile[255];
	char buffer[10];
	LOGFONT Mfont = {9,0,0,0,0,0,0,0,0,0,0,0,0,"Fixedsys"};   
	static HFONT LcurrFont;
	int Sender;
	char SenderName[30];
	switch (message) {
	case WM_INITDIALOG:
		LcurrFont = CreateFontIndirect(&Mfont);
		SendMessage(GetDlgItem(hDlg,IDC_LIST1),WM_SETFONT,(WPARAM)LcurrFont,TRUE);
		SendMessage(GetDlgItem(hDlg,IDC_LIST2),WM_SETFONT,(WPARAM)LcurrFont,TRUE);
		
		if ( EIT_Sort == 0 ) CheckDlgButton(hDlg,IDC_RADIO4,TRUE);
		else if ( EIT_Sort == 1 ) CheckDlgButton(hDlg,IDC_RADIO5,TRUE);
		else if ( EIT_Sort == 2 ) CheckDlgButton(hDlg,IDC_RADIO6,TRUE);
		
		if ( EIT_Datum == 0 ) CheckDlgButton(hDlg,IDC_RADIO7,TRUE);
		else if ( EIT_Datum == 1 ) CheckDlgButton(hDlg,IDC_RADIO8,TRUE);
		else if ( EIT_Datum == 2 ) CheckDlgButton(hDlg,IDC_RADIO9,TRUE);
		else if ( EIT_Datum == 3 ) CheckDlgButton(hDlg,IDC_RADIO10,TRUE);
		
		if ( EIT_Kanal == 0 ) CheckDlgButton(hDlg,IDC_RADIO11,TRUE);
		else if ( EIT_Kanal == 1 ) CheckDlgButton(hDlg,IDC_RADIO12,TRUE);
		else if ( EIT_Kanal == 2 ) CheckDlgButton(hDlg,IDC_RADIO13,TRUE);
		
		SendMessage(GetDlgItem(hDlg,IDC_LIST1),LB_RESETCONTENT,0,0);
		SendMessage(GetDlgItem(hDlg,IDC_LIST2),LB_RESETCONTENT,0,0);
		
		for ( i=0; i < MAXPROGS; i++ ) { 
			if (( Programm[i].Typ == 'D') && (Programm[i].freq != 0 )) {
				sprintf(Zeile,"%s                                 ",Programm[i].Name);
				sprintf(buffer,"<%d>",i+1);
				for ( j=0; j < (signed)strlen(buffer); j++ ) {
					Zeile[32+j]=buffer[j];
					Zeile[32+j+1]=0x00;
				};
				SendMessage(GetDlgItem(hDlg,IDC_LIST1),LB_ADDSTRING,0,(LPARAM)Zeile);
			};
		};	    
		for ( i=0; i < MAXFAVORITEN; i++ ) { 
			if (( Favoriten[i].ts_id != 0) || ( Favoriten[i].SID_Id != 0)) {
				
				Sender=Get_Sender(Favoriten[i].SID_Id ,Favoriten[i].ts_id,(char *)&SenderName);
				if ( Sender >= 0 ) {
					sprintf(Zeile,"%s                                 ",SenderName);
					sprintf(buffer,"<%d>",Sender+1);
					for ( j=0; j < (signed)strlen(buffer); j++ ) {
						Zeile[32+j]=buffer[j];
						Zeile[32+j+1]=0x00;
					};
					SendMessage(GetDlgItem(hDlg,IDC_LIST2),LB_ADDSTRING,0,(LPARAM)Zeile);
				} else {
					Favoriten[i].SID_Id=0;
					Favoriten[i].ts_id=0;
				}
			};
		};	    
		
		
		break;
		
		
	case WM_COMMAND:
		
		if (LOWORD(wParam) == IDOK ) {
			if ( IsDlgButtonChecked(hDlg,IDC_RADIO4) == TRUE ) EIT_Sort=0;
			else if ( IsDlgButtonChecked(hDlg,IDC_RADIO5)== TRUE )  EIT_Sort=1;	
			else if ( IsDlgButtonChecked(hDlg,IDC_RADIO6)== TRUE )  EIT_Sort=2;	
			
			if ( IsDlgButtonChecked(hDlg,IDC_RADIO7) == TRUE ) EIT_Datum=0;
			else if ( IsDlgButtonChecked(hDlg,IDC_RADIO8)== TRUE )  EIT_Datum=1;	
			else if ( IsDlgButtonChecked(hDlg,IDC_RADIO9)== TRUE )  EIT_Datum=2;	
			else if ( IsDlgButtonChecked(hDlg,IDC_RADIO10)== TRUE )  EIT_Datum=3;	
			
			if ( IsDlgButtonChecked(hDlg,IDC_RADIO11) == TRUE ) EIT_Kanal=0;
			else if ( IsDlgButtonChecked(hDlg,IDC_RADIO12)== TRUE )  EIT_Kanal=1;	
			else if ( IsDlgButtonChecked(hDlg,IDC_RADIO13)== TRUE )  EIT_Kanal=2;	
			
			for ( i=0; i < MAXFAVORITEN ; i++ ) {
				Favoriten[i].SID_Id=0;
				Favoriten[i].ts_id=0;
			}; 
			
			i=SendMessage(GetDlgItem(hDlg,IDC_LIST2),LB_GETCOUNT ,0,(LONG)0);
			j=0;
			m=0;
			while (  j < i ) {
				SendMessage(GetDlgItem(hDlg,IDC_LIST2),LB_GETTEXT,0,(LONG)&Zeile);
				k=33;
				while ( Zeile[k] != '>' ) {
					buffer[k-33]=Zeile[k];
					buffer[k-32]=0x00;
					k++;
				};
				l=atoi(buffer);
				l--;
				Favoriten[m].SID_Id=Programm[l].SID_pid;
				Favoriten[m].ts_id=Programm[l].tp_id;
				m++;
				SendMessage(GetDlgItem(hDlg,IDC_LIST2),LB_DELETESTRING ,0,(LONG)0);
				j++;
			};
			
			EndDialog(hDlg,TRUE);
			break;  
		}
		
       	if (LOWORD(wParam) == IDC_LIST1 ) {
			if ( HIWORD( wParam ) == LBN_SELCHANGE  ) {
				EnableWindow(GetDlgItem(hDlg,IDC_DEL),FALSE); 
				EnableWindow(GetDlgItem(hDlg,IDC_ADD),TRUE); 
			}
			break;	
		}
		
       	if (LOWORD(wParam) == IDC_LIST2 ) {
			if ( HIWORD( wParam ) == LBN_SELCHANGE  ) {
				EnableWindow(GetDlgItem(hDlg,IDC_DEL),TRUE); 
				EnableWindow(GetDlgItem(hDlg,IDC_ADD),FALSE); 
			}
		}
		
       	if (LOWORD(wParam) == IDC_ADD ) {
			i=SendMessage (GetDlgItem(hDlg,IDC_LIST1), LB_GETCURSEL, 0, 0);
			if ( i == LB_ERR ) break;
			SendMessage(GetDlgItem(hDlg,IDC_LIST1),LB_GETTEXT,i,(LONG)&Zeile);
			j=(int)SendMessage(GetDlgItem(hDlg,IDC_LIST2),LB_FINDSTRING,(WPARAM)-1,(LPARAM)Zeile);
			if ( j != LB_ERR ) break;
			SendMessage(GetDlgItem(hDlg,IDC_LIST2),LB_ADDSTRING,0,(LPARAM)Zeile);
		}
		
		
       	if (LOWORD(wParam) == IDC_DEL ) {
			i=SendMessage (GetDlgItem(hDlg,IDC_LIST2), LB_GETCURSEL, 0, 0);
			if ( i == LB_ERR ) break;
			SendMessage(GetDlgItem(hDlg,IDC_LIST2),LB_DELETESTRING ,i,(LONG)0);
		}
		
		
		if (LOWORD(wParam) == IDCANCEL ) {
            EndDialog(hDlg,TRUE);
			break;  
		}
		
 }
 
 return(FALSE);
}

BOOL APIENTRY EITDetail_Proc(HWND hDlg,UINT message,UINT wParam,LONG lParam)
{
	int Sender;
	char SenderName[30];
	char Zeile[30];
	unsigned char Buffer[1096];
	
	static int i_id;
	static int o_id;
	int Virtual_Event_Id;
	int Virtual_Offset_Id;
	
	int i,j,k;
	
	switch (message) {
	case WM_INITDIALOG:
	case WM_USER:
		i_id=EIT_Message>>16;
		o_id=EIT_Message&0xffff;
		Sender=Get_Sender(EIT[i_id].EIT_Infos[o_id].SID_Id,EIT[i_id].EIT_Infos[o_id].ts_id,(char *)&SenderName);  
		if ( Sender < 0 ) {
			SetDlgItemText(hDlg,IDC_EDIT2,"Sender ist nicht in Programmliste");
			EnableWindow(GetDlgItem(hDlg,ID_SWITCHTO),FALSE);
		} else {
			sprintf(Zeile,"%d",Make_ProgNr(Sender)+1);
			SetDlgItemText(hDlg,IDC_EDIT1,Zeile);
			SetDlgItemText(hDlg,IDC_EDIT2,SenderName);
			EnableWindow(GetDlgItem(hDlg,ID_SWITCHTO),TRUE);
		}
		
		
		sprintf(Zeile,"%d",EIT[i_id].EIT_Infos[o_id].Event_Id);
		SetDlgItemText(hDlg,IDC_EDIT3,Zeile);
		sprintf(Zeile,"%d",EIT[i_id].EIT_Infos[o_id].SID_Id);
		SetDlgItemText(hDlg,IDC_EDIT4,Zeile);
		sprintf(Zeile,"%d",EIT[i_id].EIT_Infos[o_id].ts_id);
		SetDlgItemText(hDlg,IDC_EDIT5,Zeile);
		sprintf(Zeile,"%d",EIT[i_id].EIT_Infos[o_id].Running_Status);
		SetDlgItemText(hDlg,IDC_EDIT9,Zeile);
		sprintf(Zeile,"%d",EIT[i_id].EIT_Infos[o_id].Current_Next);
		SetDlgItemText(hDlg,IDC_EDIT7,Zeile);
		Date_To_String((unsigned short)RealDay(EIT[i_id].EIT_Infos[o_id].Start_Zeit_h,
						       EIT[i_id].EIT_Infos[o_id].Date_Id),Zeile);
		SetDlgItemText(hDlg,IDC_EDIT6,Zeile);
		sprintf(Zeile,"%02d:%02d:%02d",RealTime(EIT[i_id].EIT_Infos[o_id].Start_Zeit_h),EIT[i_id].EIT_Infos[o_id].Start_Zeit_m,EIT[i_id].EIT_Infos[o_id].Start_Zeit_s);
		SetDlgItemText(hDlg,IDC_EDIT12,Zeile);
		sprintf(Zeile,"%02d:%02d:%02d",EIT[i_id].EIT_Infos[o_id].Dauer_h,EIT[i_id].EIT_Infos[o_id].Dauer_m,EIT[i_id].EIT_Infos[o_id].Dauer_s);
		SetDlgItemText(hDlg,IDC_EDIT13,Zeile);
		sprintf(Zeile,"%c%c%c",EIT[i_id].EIT_Infos[o_id].HeaderLang[0],EIT[i_id].EIT_Infos[o_id].HeaderLang[1],EIT[i_id].EIT_Infos[o_id].HeaderLang[2]);
		SetDlgItemText(hDlg,IDC_EDIT14,Zeile);
		sprintf(Zeile,"%c%c%c",EIT[i_id].EIT_Infos[o_id].TextLang[0],EIT[i_id].EIT_Infos[o_id].TextLang[1],EIT[i_id].EIT_Infos[o_id].TextLang[2]);
		SetDlgItemText(hDlg,IDC_EDIT15,Zeile);
		
		if (( EIT[i_id].EIT_Infos[o_id].virtual_eventid != 0xffff ) && ( EIT[i_id].EIT_Infos[o_id].virtual_sid != 0xffff )) j=1;
		else j=EIT[i_id].EIT_Infos[o_id].Extra_4ASize+EIT[i_id].EIT_Infos[o_id].Extra_50Size+EIT[i_id].EIT_Infos[o_id].Extra_54Size+EIT[i_id].EIT_Infos[o_id].Extra_55Size+EIT[i_id].EIT_Infos[o_id].Extra_5FSize+EIT[i_id].EIT_Infos[o_id].Extra_81Size+EIT[i_id].EIT_Infos[o_id].Extra_F0Size;
		
		
		
		if ( j > 0 ) EnableWindow(GetDlgItem(hDlg,IDC_BUTTON1),TRUE);
		else EnableWindow(GetDlgItem(hDlg,IDC_BUTTON1),FALSE);
		
		i=0;
		j=0;
		k=0;
		Virtual_Event_Id=i_id;
		Virtual_Offset_Id=o_id;
		
		if (( EIT[i_id].EIT_Infos[o_id].virtual_eventid != 0xffff ) &&
			( EIT[i_id].EIT_Infos[o_id].virtual_sid != 0xffff )) {
			i=Get_Virtual_Event(EIT[i_id].EIT_Infos[o_id].virtual_eventid,EIT[i_id].EIT_Infos[o_id].virtual_sid);
			if ( i >= 0 ) {
				Virtual_Event_Id=EIT[i_id].EIT_Infos[o_id].virtual_eventid;
				Virtual_Offset_Id=i;
			}
		}
		
		
		
		if ( EIT[Virtual_Event_Id].EIT_Infos[Virtual_Offset_Id].HeaderSize > 0 ) {
			while ( EIT[Virtual_Event_Id].EIT_Infos[Virtual_Offset_Id].Header[i] != 0x00 ) {
				Buffer[j] = EIT[Virtual_Event_Id].EIT_Infos[Virtual_Offset_Id].Header[i];
				if ( Buffer[j] < 0x20 ) { 
					if ( Buffer[j] == 0x0a ) {
						Buffer[j]=0x0d;
						j++;
						Buffer[j]=0x0a;
						k=0;
					} else {
						Buffer[j]=0x00;
						j--;
                        k--;
					};
				};
				if ( k > 70 ) {
					if ( Buffer[j]==0x20 ) {
						Buffer[j]=0x0d;
						j++;
						Buffer[j]=0x0a;
						k=0;
					}
				};
				k++;
				j++;
				i++;
			};
			Buffer[j]=0x00;
		} else {
			sprintf(Buffer,"<Kein Titel geladen>");
		}
		SetDlgItemText(hDlg,IDC_EDIT10,Buffer);
		
		if ( EIT[Virtual_Event_Id].EIT_Infos[Virtual_Offset_Id].TextSize > 0 ) {
			i=0;
			j=0;
			k=0;
			while ( EIT[Virtual_Event_Id].EIT_Infos[Virtual_Offset_Id].Text[i] != 0x00 ) {
				Buffer[j] = EIT[Virtual_Event_Id].EIT_Infos[Virtual_Offset_Id].Text[i];
				if ( Buffer[j] < 0x20 ) { 
					if ( Buffer[j] == 0x0a ) {
						Buffer[j]=0x0d;
						j++;
						Buffer[j]=0x0a;
						k=0;
					} else {
						Buffer[j]=0x00;
						j--;
                        k--;
					};
				};
				if ( k > 70 ) {
					if ( Buffer[j]==0x20 ) {
						Buffer[j]=0x0d;
						j++;
						Buffer[j]=0x0a;
						k=0;
					}
				};
				k++;
				j++;
				i++;
			};
			Buffer[j]=0x00;
			
			SetDlgItemText(hDlg,IDC_EDIT11,Buffer);
		} else SetDlgItemText(hDlg,IDC_EDIT11,"");
		break;
		
		
		
		case WM_COMMAND:
			
            if (LOWORD(wParam) == IDC_BUTTON1 ) {
                DialogBox(hInst,"EITDETAILEXTRA",hDlg,EITDetailExtra_Proc);
                SetFocus(hDlg);
				break;
			}
			
			if (LOWORD(wParam) == ID_SWITCHTO ) {
				i_id=EIT_Message>>16;
				o_id=EIT_Message&0xffff;
				Sender=Get_Sender(EIT[i_id].EIT_Infos[o_id].SID_Id,EIT[i_id].EIT_Infos[o_id].ts_id,(char *)&SenderName);  
				if ( Sender >= 0 ) {
					sprintf(ChannelString,"%d",Make_ProgNr(Sender)+1);
					SetTimer(hWnd,99,10,NULL);
				};
				break;
				
			};
			if (LOWORD(wParam) == ID_RECORD ) {
				
				i_id=EIT_Message>>16;
				o_id=EIT_Message&0xffff;
				
				for ( i=0; i < TIMERANZAHL; i++ ) {
					if ( TimerRec[i].StartZeit_h == 0xff) {
						TimerRec[i].Date_Id=EIT[i_id].EIT_Infos[o_id].Date_Id;
						TimerRec[i].StartZeit_h=(unsigned char)RealTime(EIT[i_id].EIT_Infos[o_id].Start_Zeit_h);
						TimerRec[i].StartZeit_m=EIT[i_id].EIT_Infos[o_id].Start_Zeit_m;
						j=RealTime(EIT[i_id].EIT_Infos[o_id].Start_Zeit_h);
						k=EIT[i_id].EIT_Infos[o_id].Start_Zeit_m;
						k=k+EIT[i_id].EIT_Infos[o_id].Dauer_m;
						if ( k >= 60 ) { 
							j++;
							k=k-60;
						};
						j=j+EIT[i_id].EIT_Infos[o_id].Dauer_h;
						if ( j >= 24 ) j=j-24;
						TimerRec[i].EndZeit_h=j;
						TimerRec[i].EndZeit_m=k;
						TimerRec[i].Started=FALSE;
						TimerRec[i].RecorderArt=RecorderFormat+(3*RecorderTyp);
						TimerRec[i].Programm=Get_Sender(EIT[i_id].EIT_Infos[o_id].SID_Id,EIT[i_id].EIT_Infos[o_id].ts_id,(char *)&SenderName);
						DialogBox(hInst,"TIMERRECORDER",hDlg,TimerRecSettingProc);
						return(TRUE);
					};
				};
				
			}
			
			
			
			if (LOWORD(wParam) == IDBEFORE ) {
				i=SendMessage(ProgList,LB_GETCURSEL,0,0);
				if ( i == LB_ERR ) break;
				i--;
				j=SendMessage(ProgList,LB_SETCURSEL,i,0);
				if ( j == LB_ERR ) break;
				SendMessage(ProgList,LB_GETTEXT,i,(LPARAM)&EIT_Message);
				PostMessage(hDlg,WM_USER,0,0);
				break;  
			}
			
			
			if (LOWORD(wParam) == IDNEXT ) {
				i=SendMessage(ProgList,LB_GETCURSEL,0,0);
				if ( i == LB_ERR ) break;
				i++;
				j=SendMessage(ProgList,LB_SETCURSEL,i,0);
				if ( j == LB_ERR ) break;
				SendMessage(ProgList,LB_GETTEXT,i,(LPARAM)&EIT_Message);
				PostMessage(hDlg,WM_USER,0,(LPARAM)0);
				break;  
			}
			
			
			if (LOWORD(wParam) == IDOK ) {
				EndDialog(hDlg,0);
				break;  
			}
			
			
			if (LOWORD(wParam) == IDCANCEL ) {
				EndDialog(hDlg,0);
				break;  
			}
			
 }
 
 return(FALSE);
}


BOOL APIENTRY EITDetailExtra_Proc(HWND hDlg,UINT message,UINT wParam,LONG lParam)
{
	int i_id;
	int o_id;
	int Virtual_Event_Id;
	int Virtual_Offset_Id;
	
	int i;
	char Zeile[128];
	
	BOOL ret;
	LOGFONT Mfont = {9,5,0,0,0,0,0,0,0,0,0,0,0,"Fixedsys"};   
	HFONT currFont;
	
	switch (message) {
	case WM_INITDIALOG:
		i_id=EIT_Message>>16;
		o_id=EIT_Message&0xffff;
		
		if (( EIT[i_id].EIT_Infos[o_id].virtual_eventid != 0xffff ) && ( EIT[i_id].EIT_Infos[o_id].virtual_sid != 0xffff )) sprintf(Zeile,"%d/%d ",EIT[i_id].EIT_Infos[o_id].virtual_eventid,EIT[i_id].EIT_Infos[o_id].virtual_sid);
		else sprintf(Zeile,"                        ");
		SetDlgItemText(hDlg,IDC_EDIT17,Zeile);
		
		currFont = CreateFontIndirect(&Mfont);
		SendMessage(GetDlgItem(hDlg,IDC_LIST1),WM_SETFONT,(WPARAM)currFont,TRUE);
		SendMessage (GetDlgItem(hDlg,IDC_LIST1), LB_RESETCONTENT, 0, 0);
		SendMessage(GetDlgItem(hDlg,IDC_LIST2),WM_SETFONT,(WPARAM)currFont,TRUE);
		SendMessage (GetDlgItem(hDlg,IDC_LIST2), LB_RESETCONTENT, 0, 0);
		
		
		ret=Is_Time_Link((unsigned short)i_id, (unsigned short)o_id,FALSE);
		
		for ( i = 0; i < 16; i++ ) {
			if ( MultiLinkZeit[i].fill != 0x00 ) {
				sprintf(Zeile,"%s  (%s)",MultiLinkZeit[i].ExtraText,MultiLinkZeit[i].Name);
				SendMessage (GetDlgItem(hDlg,IDC_LIST1), LB_INSERTSTRING, i, (LPARAM)Zeile);
			};
		};
		
		for ( i = 0; i < 16; i++ ) {
			if ( MultiLinkZeit[i].fill == 0x02 ) SendMessage (GetDlgItem(hDlg,IDC_LIST1), LB_SETCURSEL, i, 0);
		};
		
		Virtual_Event_Id=i_id;
		Virtual_Offset_Id=o_id;
		
		if (( EIT[i_id].EIT_Infos[o_id].virtual_eventid != 0xffff ) &&
			( EIT[i_id].EIT_Infos[o_id].virtual_sid != 0xffff )) {
			i=Get_Virtual_Event(EIT[i_id].EIT_Infos[o_id].virtual_eventid,EIT[i_id].EIT_Infos[o_id].virtual_sid);
			if ( i >= 0 ) {
				Virtual_Event_Id=EIT[i_id].EIT_Infos[o_id].virtual_eventid;
				Virtual_Offset_Id=i;
			}
		}
		
		
		ret=Is_View_Link((unsigned short)Virtual_Event_Id,(unsigned short)Virtual_Offset_Id,FALSE);
		
		for ( i = 0; i < 16; i++ ) {
			if ( MultiLinkView[i].fill != 0x00 ) {
				sprintf(Zeile,"%-12s (%s)",MultiLinkView[i].ExtraText,MultiLinkView[i].Name);
				SendMessage (GetDlgItem(hDlg,IDC_LIST2), LB_INSERTSTRING, i, (LPARAM)Zeile);
			};
		};
		
		for ( i = 0; i < 16; i++ ) {
			if ( MultiLinkView[i].fill == 0x02 ) SendMessage (GetDlgItem(hDlg,IDC_LIST2), LB_SETCURSEL, i, 0);
		};
		
		
		ret=Is_Pid_Param((unsigned short)Virtual_Event_Id,(unsigned short)Virtual_Offset_Id,FALSE);
		
		for ( i = 0; i < 16; i++ ) {
			if ( PidParam[i].fill!= 0x00 ) {
				sprintf(Zeile,"%02d %02d %02d %c%c%c %s",PidParam[i].Key_01,PidParam[i].Key_02,PidParam[i].Key_03,PidParam[i].Language[0],PidParam[i].Language[1],PidParam[i].Language[2],PidParam[i].Name );
				SendMessage (GetDlgItem(hDlg,IDC_LIST3), LB_INSERTSTRING, i, (LPARAM)Zeile);
				
			};
		};
		
		
		
		return(TRUE);
		
		
		
		
		break;
		
		
		
		
	case WM_COMMAND:
		
		if (LOWORD(wParam) == IDOK ) {
			EndDialog(hDlg,TRUE);
			break;  
		}
		
		
		if (LOWORD(wParam) == IDCANCEL ) {
            EndDialog(hDlg,TRUE);
			break;  
		}
		
	}
	
	return(FALSE);
}


#define FELDNUM 20
static char *FeldListe[] = {
	"Nummer",    "Typ",       "Name",        "Frequenz",   "Polarisation",
		"Video Pid", "Audio Pid", "Text Pid",    "SID Pid",    "PMT Pid",
		"QAM",       "AC3 Pid",   "Symbolrate",  "FEC",        "DiSEqC",
		"Provider",  "Land",      "Transponder", "Service-ID", "Crypt-System"};
	
	
	void GetFeldName(short id, char *zeile)
	{
		if ( id >= 0 && id < FELDNUM ) {
			strcpy(zeile, FeldListe[id]);
		} else {
			zeile[0] = 0;
		}
	}
	
	
	VOID APIENTRY HandleFocusStateKanalListe(HWND hDlg,LPDRAWITEMSTRUCT lpdis)
	{
		DrawEntireItemKanalListe(
			hDlg,
			lpdis,
			(lpdis->itemState & ODS_FOCUS || lpdis->itemState & ODS_SELECTED) ? 1 : 0
			);
	}
	
	
	VOID APIENTRY DrawEntireItemKanalListe(HWND hDlg,LPDRAWITEMSTRUCT lpdis,INT Typ)
	{
		
		int i, j;
		char Zeile[64];
		HFONT OldFont;
		
		OldFont = SelectObject(lpdis->hDC, currFont);
		
		if ( Typ == 0 ) {
			SetTextColor(lpdis->hDC,RGB(0,0,0));
			SetBkColor(lpdis->hDC,RGB(255,255,255));
		} else {
			SetTextColor(lpdis->hDC,RGB(255,255,255));
			SetBkColor(lpdis->hDC,RGB(0,0,255));
		}
		for ( i = 0; i < 15 ; i++ ) {
			if (ButtonList[i].FeldId < 0 || ButtonList[i].s == 0) continue;
			Zeile[0]=0x00;
			switch (ButtonList[i].FeldId) {
			case 0:
				sprintf(Zeile,"%4d",Make_ProgNr(lpdis->itemData)+1);
				break;
			case 1:
				if ( Programm[lpdis->itemData].Typ == 'A' ) {
					strcpy(Zeile,"Analog");
				} else {
					
					if (( Programm[lpdis->itemData].Link_SID != 0xffff ) && ( Programm[lpdis->itemData].Link_TP != 0xffff  )) {
						Make_Service_Name( 5 ,&Zeile[0]);
					} else if (( Programm[lpdis->itemData].ServiceTyp > 0 ) && (Programm[lpdis->itemData].ServiceTyp < 11 )) {
						Make_Service_Name( Programm[lpdis->itemData].ServiceTyp,&Zeile[0]);
					} else {
						if (Programm[lpdis->itemData].Video_pid != 0x1fff) {
							strcpy(Zeile,"TV");
						} else if (Programm[lpdis->itemData].Audio_pid != 0x1fff) {
							strcpy(Zeile,"Radio");
						} else {
							strcpy(Zeile,"Daten");
						}
					};
					
					if (Programm[lpdis->itemData].CA_ID != 0)  strcat(Zeile,"[C]");
				}
				break;
			case 2:
				strcpy(Zeile,Programm[lpdis->itemData].Name);
				break;
			case 3:
				sprintf(Zeile,"%10.3f", Programm[lpdis->itemData].freq/1000.f);
				break;
			}
			if ( Programm[lpdis->itemData].Typ == 'D' ) {
				switch (ButtonList[i].FeldId) {
				case 4:
					strcpy(Zeile, (Programm[lpdis->itemData].volt == 1) ? "Hor." : "Ver.");
					break;
				case 5:
					sprintf(Zeile,"0x%04x.",Programm[lpdis->itemData].Video_pid);
					break;
				case 6:
					sprintf(Zeile,"0x%04x.",Programm[lpdis->itemData].Audio_pid);
					break;
				case 7:
					sprintf(Zeile,"0x%04x.",Programm[lpdis->itemData].TeleText_pid);
					break;
				case 8:
					sprintf(Zeile,"0x%04x.",Programm[lpdis->itemData].SID_pid);
					break;
				case 9:
					sprintf(Zeile,"0x%04x.",Programm[lpdis->itemData].PMT_pid);
					break;
				case 10:
					if ( Programm[lpdis->itemData].qam == 0 ) strcpy(Zeile,"16");
					if ( Programm[lpdis->itemData].qam == 1 ) strcpy(Zeile,"32");
					if ( Programm[lpdis->itemData].qam == 2 ) strcpy(Zeile,"64");
					if ( Programm[lpdis->itemData].qam == 3 ) strcpy(Zeile,"128");
					if ( Programm[lpdis->itemData].qam == 4 ) strcpy(Zeile,"256");
					break;
				case 11:
					sprintf(Zeile,"0x%04X",Programm[lpdis->itemData].AC3_pid);
					break;
				case 12:
					sprintf(Zeile,"%d",Programm[lpdis->itemData].srate);
					break;
				case 13:
					if ( Programm[lpdis->itemData].fec == 0 ) strcpy(Zeile,"1/2");
					else if ( Programm[lpdis->itemData].fec == 1 ) strcpy(Zeile,"2/3");
					else if ( Programm[lpdis->itemData].fec == 2 ) strcpy(Zeile,"3/4");
					else if ( Programm[lpdis->itemData].fec == 3 ) strcpy(Zeile,"4/5");
					else if ( Programm[lpdis->itemData].fec == 4 ) strcpy(Zeile,"5/6");
					else if ( Programm[lpdis->itemData].fec == 5 ) strcpy(Zeile,"6/7");
					else if ( Programm[lpdis->itemData].fec == 6 ) strcpy(Zeile,"7/8");
					else if ( Programm[lpdis->itemData].fec == 7 ) strcpy(Zeile,"8/9");
					else if ( Programm[lpdis->itemData].fec == 8 ) strcpy(Zeile,"Auto");
					break;
				case 14:
					sprintf(Zeile,"%d.",Programm[lpdis->itemData].diseqc);
					break;
				case 15:
					strcpy(Zeile,Programm[lpdis->itemData].Anbieter);
					break;
				case 16:
					strcpy(Zeile,Programm[lpdis->itemData].Land);
					break;
				case 17:
					sprintf(Zeile,"%d",Programm[lpdis->itemData].tp_id);
					break;
				case 18:
					sprintf(Zeile,"%d",Programm[lpdis->itemData].ServiceTyp);
					break;
				case 19:
					Make_Crypt_Systems(lpdis->itemData, Zeile);
					break;
				}
			}
			
			for (j=strlen(Zeile); j<21; j++) Zeile[j] = ' ';
			Zeile[j] = 0;
			TextOut(lpdis->hDC, ButtonList[i].s-3, lpdis->rcItem.top+1, Zeile,strlen(Zeile));
}

SelectObject(lpdis->hDC,OldFont);
return;
}

BOOL APIENTRY KanalNummerProc(HWND hDlg,UINT message,UINT wParam,LONG lParam)
{
	int i;
	
	switch (message) {
		
	case WM_INITDIALOG:
		
		SetFocus(GetDlgItem(hDlg,IDC_EDIT1));
		SendMessage(GetDlgItem(hDlg,IDC_EDIT1),WM_CHAR,KeyValue,0);
		break;
		
		
	case WM_COMMAND:
		
		if (LOWORD(wParam) == IDOK ) {
			
			i=GetDlgItemInt(hDlg,IDC_EDIT1,NULL,FALSE);
			i--;
			if (i < 0 || i >= MAXPROGS) {
				MessageBox(hDlg,"Unzulässige KanalNummer","MultiDec",MB_ICONSTOP);
			} else {
				EndDialog(hDlg,i);
			}
			
		} else if (LOWORD(wParam) == IDCANCEL ) {
			
			EndDialog(hDlg,-1);
			
		}
		break;
	}
	
	return(FALSE);
}

BOOL APIENTRY ListFeldSettingProc(HWND hDlg,UINT message,UINT wParam,LONG lParam)
{
	int i,j;
	struct TBL save;
	
	switch (message) {
		
	case WM_INITDIALOG:
		
		SendMessage (GetDlgItem(hDlg,IDC_COMBO1), CB_RESETCONTENT, 0, 0);
		for (i=0; i<FELDNUM; i++) {
			SendMessage (GetDlgItem(hDlg,IDC_COMBO1), CB_INSERTSTRING, i, (LPARAM)(LPSTR)FeldListe[i]);
		}
        SendMessage (GetDlgItem(hDlg,IDC_COMBO1), CB_SETCURSEL, ButtonList[SelectButton].FeldId, 0);
		SetDlgItemInt(hDlg,IDC_EDIT1,SelectButton+1,FALSE);
		break;
		
	case WM_COMMAND:
		
		if (LOWORD(wParam) == IDOK ) {
			
			i=SendMessage(GetDlgItem(hDlg,IDC_COMBO1), CB_GETCURSEL, 0, 0);
			if (i >= 0 && i < FELDNUM) ButtonList[SelectButton].FeldId=i;
			i=GetDlgItemInt(hDlg,IDC_EDIT1,NULL,FALSE);
			i--;
			if (i >= 0 && i < 20 && i != SelectButton) {
				memcpy(&save.FeldId,&ButtonList[SelectButton].FeldId, sizeof(save));
				for (j=SelectButton; j < 14; j++) {
					memcpy(&ButtonList[j].FeldId,&ButtonList[j+1].FeldId, sizeof(save));
				}
				for (j=14; j >= i; j--) {
					memcpy(&ButtonList[j+1].FeldId,&ButtonList[j].FeldId,sizeof(save));
				}
				memcpy(&ButtonList[i].FeldId,&save.FeldId,sizeof(save));
			}
			EndDialog(hDlg,TRUE);
			
		} else if (LOWORD(wParam) == IDDEL ) {
			
            for (j=SelectButton; j < 14; j++) {
				memcpy(&ButtonList[j].FeldId,&ButtonList[j+1].FeldId, sizeof(save));
			}
            ButtonList[14].FeldId=-1;
            ButtonList[14].s=0;
            ButtonList[14].x=0;
            ButtonList[14].r=0;
			
			EndDialog(hDlg,TRUE);
			
		} else if (LOWORD(wParam) == IDCANCEL ) {
			EndDialog(hDlg,TRUE);
		}
		break;
	}
	
	return(FALSE);
}



BOOL APIENTRY NitListe(
					   HWND hDlg,
					   UINT message,
					   UINT wParam,
					   LONG lParam)
{
	int i,j;
	char zeile[255];
	LOGFONT Mfont = {9,0,0,0,0,0,0,0,0,0,0,0,0,"Fixedsys"};   
    HFONT currFont;
	
	
    switch (message) {
	case WM_INITDIALOG:
		currFont = CreateFontIndirect(&Mfont);
		SendMessage(GetDlgItem(hDlg,IDC_LIST1),WM_SETFONT,(WPARAM)currFont,TRUE);
		
		SendMessage(GetDlgItem(hDlg,IDC_LIST1),LB_RESETCONTENT,0,0);
		i=0;
		j=-1;
		while ( i < MAXNIT ) {
			if ( NIT[i].ts_id != 0 )  {
				sprintf(zeile,"%1d %5d %10.3f %5d ",NIT[i].DiseqNr,NIT[i].ts_id,NIT[i].freq/1000.f,NIT[i].srate);
				if ( NIT[i].polar == 1 ) strcat(zeile,"Horizontal");
				else strcat(zeile,"Vertikal");
				SendMessage(GetDlgItem(hDlg,IDC_LIST1),LB_INSERTSTRING,i,(LPARAM)zeile);
			};
			i++;
		};
		SendMessage(GetDlgItem(hDlg,IDC_LIST1),LB_SETCURSEL,0,0);
		SendMessage(hDlg,WM_USER,1,0);
		SetFocus(GetDlgItem(hDlg,IDC_LIST1));
		return(TRUE);
		
	case WM_USER :
		
		if ( wParam == 1 ) {
            i=SendMessage(GetDlgItem(hDlg,IDC_LIST1),LB_GETCURSEL,0,0);
			SetDlgItemInt(hDlg,IDC_EDIT1,NIT[i].orig_nw_id,FALSE);
			SetDlgItemText(hDlg,IDC_EDIT2,NIT[i].Name);
			sprintf(zeile,"%10.3f",NIT[i].freq/1000.f);
			SetDlgItemText(hDlg,IDC_EDIT3,zeile);
			SetDlgItemInt(hDlg,IDC_EDIT10,NIT[i].ts_id,FALSE);
			SetDlgItemInt(hDlg,IDC_EDIT9,NIT[i].DiseqNr,FALSE);
			sprintf(zeile,"%6.1f",NIT[i].orbit/10.f);
			SetDlgItemText(hDlg,IDC_EDIT4,zeile);
			if ( NIT[i].west_east == 1 ) SetDlgItemText(hDlg,IDC_EDIT5,"Ost");
			else SetDlgItemText(hDlg,IDC_EDIT5,"West");
			if ( NIT[i].polar == 1 ) SetDlgItemText(hDlg,IDC_EDIT6,"Horizontal");
			else SetDlgItemText(hDlg,IDC_EDIT6,"Vertikal");
			SetDlgItemInt(hDlg,IDC_EDIT7,NIT[i].srate,FALSE);
			if ( NIT[i].fec == 0 ) SetDlgItemText(hDlg,IDC_EDIT8,"1/2");
			else if ( NIT[i].fec == 1 ) SetDlgItemText(hDlg,IDC_EDIT8,"2/3");
			else if ( NIT[i].fec == 2 ) SetDlgItemText(hDlg,IDC_EDIT8,"3/4");
			else if ( NIT[i].fec == 3 ) SetDlgItemText(hDlg,IDC_EDIT8,"4/5");
			else if ( NIT[i].fec == 4 ) SetDlgItemText(hDlg,IDC_EDIT8,"5/6");
			else if ( NIT[i].fec == 5 ) SetDlgItemText(hDlg,IDC_EDIT8,"6/7");
			else if ( NIT[i].fec == 6 ) SetDlgItemText(hDlg,IDC_EDIT8,"7/8");
			else if ( NIT[i].fec == 7 ) SetDlgItemText(hDlg,IDC_EDIT8,"8/9");
			else if ( NIT[i].fec == 8 ) SetDlgItemText(hDlg,IDC_EDIT8,"Auto");
			
		};
		
		break;
		
	case WM_COMMAND:
		
		if (LOWORD(wParam) == IDC_LIST1 ) {
			if ( HIWORD( wParam ) == LBN_SELCHANGE  ) {
				SendMessage(hDlg,WM_USER,1,0);
			}
		}
		
		
		
		if (LOWORD(wParam) == IDOK
			|| LOWORD(wParam) == IDCANCEL) {
			EndDialog(hDlg,TRUE);
		}
		
		break;
    }
    return (FALSE);
	UNREFERENCED_PARAMETER(lParam);
}





BOOL APIENTRY NitListeNeu(
						  HWND hDlg,
						  UINT message,
						  UINT wParam,
						  LONG lParam)
{
	int i,j;
	char zeile[255];
	LOGFONT Mfont = {9,5,0,0,0,0,0,0,0,0,0,0,0,"Fixedsys"};   
    HFONT currFont;
	
	
    switch (message) {
	case WM_INITDIALOG:
		currFont = CreateFontIndirect(&Mfont);
		SendMessage(GetDlgItem(hDlg,IDC_LIST1),WM_SETFONT,(WPARAM)currFont,TRUE);
		
		SendMessage(GetDlgItem(hDlg,IDC_LIST1),LB_RESETCONTENT,0,0);
		i=0;
		j=-1;
		while ( i < NEUNIT ) {
			if ( NITNeu[i].ts_id != 0 )  {
				sprintf(zeile,"%1d %5d %10.3f %5d ",NITNeu[i].DiseqNr,NITNeu[i].ts_id,NITNeu[i].freq/1000.f,NITNeu[i].srate);
				if ( NITNeu[i].polar == 1 ) strcat(zeile,"Horizontal");
				else strcat(zeile,"Vertikal");
				SendMessage(GetDlgItem(hDlg,IDC_LIST1),LB_INSERTSTRING,i,(LPARAM)zeile);
			};
			i++;
		};
		SendMessage(GetDlgItem(hDlg,IDC_LIST1),LB_SETCURSEL,0,0);
		SendMessage(hDlg,WM_USER,1,0);
		SetFocus(GetDlgItem(hDlg,IDC_LIST1));
		return(TRUE);
		
	case WM_USER :
		
		if ( wParam == 1 ) {
            i=SendMessage(GetDlgItem(hDlg,IDC_LIST1),LB_GETCURSEL,0,0);
			
			SetDlgItemInt(hDlg,IDC_EDIT1,NITNeu[i].orig_nw_id,FALSE);
			SetDlgItemText(hDlg,IDC_EDIT2,NITNeu[i].Name);
			sprintf(zeile,"%10.3f",NITNeu[i].freq/1000.f);
			SetDlgItemText(hDlg,IDC_EDIT3,zeile);
			SetDlgItemInt(hDlg,IDC_EDIT10,NITNeu[i].ts_id,FALSE);
			SetDlgItemInt(hDlg,IDC_EDIT9,NITNeu[i].DiseqNr,FALSE);
			sprintf(zeile,"%6.1f",NITNeu[i].orbit/10.f);
			SetDlgItemText(hDlg,IDC_EDIT4,zeile);
			if ( NITNeu[i].west_east == 1 ) SetDlgItemText(hDlg,IDC_EDIT5,"Ost");
			else SetDlgItemText(hDlg,IDC_EDIT5,"West");
			if ( NITNeu[i].polar == 1 ) SetDlgItemText(hDlg,IDC_EDIT6,"Horizontal");
			else SetDlgItemText(hDlg,IDC_EDIT6,"Vertikal");
			SetDlgItemInt(hDlg,IDC_EDIT7,NITNeu[i].srate,FALSE);
			if ( NITNeu[i].fec == 0 ) SetDlgItemText(hDlg,IDC_EDIT8,"1/2");
			else if ( NITNeu[i].fec == 1 ) SetDlgItemText(hDlg,IDC_EDIT8,"2/3");
			else if ( NITNeu[i].fec == 2 ) SetDlgItemText(hDlg,IDC_EDIT8,"3/4");
			else if ( NITNeu[i].fec == 3 ) SetDlgItemText(hDlg,IDC_EDIT8,"4/5");
			else if ( NITNeu[i].fec == 4 ) SetDlgItemText(hDlg,IDC_EDIT8,"5/6");
			else if ( NITNeu[i].fec == 5 ) SetDlgItemText(hDlg,IDC_EDIT8,"6/7");
			else if ( NITNeu[i].fec == 6 ) SetDlgItemText(hDlg,IDC_EDIT8,"7/8");
			else if ( NITNeu[i].fec == 7 ) SetDlgItemText(hDlg,IDC_EDIT8,"8/9");
			else if ( NITNeu[i].fec == 8 ) SetDlgItemText(hDlg,IDC_EDIT8,"Auto");
			
		};
		
		break;
		
	case WM_COMMAND:
		
		
		if (LOWORD(wParam) == IDC_LIST1 ) {
			if ( HIWORD( wParam ) == LBN_SELCHANGE  ) {
				SendMessage(hDlg,WM_USER,1,0);
			}
			if (HIWORD(wParam)==LBN_DBLCLK) {
				
				i=SendMessage(GetDlgItem(hDlg,IDC_LIST1),LB_GETCURSEL,0,0);
				Transponder.volt=NITNeu[i].polar;
				
				CheckDlgButton(TransPonderDialog, IDC_RADIO20, FALSE);
				CheckDlgButton(TransPonderDialog, IDC_RADIO21, FALSE);
				
				if ( Transponder.volt == 1 ) CheckDlgButton(TransPonderDialog, IDC_RADIO20, TRUE);
				else CheckDlgButton(TransPonderDialog, IDC_RADIO21, TRUE);
				
				Transponder.fec=NITNeu[i].fec;
				CheckDlgButton(TransPonderDialog, IDC_RADIO12, FALSE);
				CheckDlgButton(TransPonderDialog, IDC_RADIO13, FALSE);
				CheckDlgButton(TransPonderDialog, IDC_RADIO14, FALSE);
				CheckDlgButton(TransPonderDialog, IDC_RADIO15, FALSE);
				CheckDlgButton(TransPonderDialog, IDC_RADIO16, FALSE);
				CheckDlgButton(TransPonderDialog, IDC_RADIO17, FALSE);
				CheckDlgButton(TransPonderDialog, IDC_RADIO18, FALSE);
				CheckDlgButton(TransPonderDialog, IDC_RADIO19, FALSE);
				
				
				if ( Transponder.fec == 0 ) CheckDlgButton(TransPonderDialog, IDC_RADIO11, TRUE);
				else if ( Transponder.fec == 1 ) CheckDlgButton(TransPonderDialog, IDC_RADIO12, TRUE);
				else if ( Transponder.fec == 2 ) CheckDlgButton(TransPonderDialog, IDC_RADIO13, TRUE);
				else if ( Transponder.fec == 3 ) CheckDlgButton(TransPonderDialog, IDC_RADIO14, TRUE);
				else if ( Transponder.fec == 4 ) CheckDlgButton(TransPonderDialog, IDC_RADIO15, TRUE);
				else if ( Transponder.fec == 5 ) CheckDlgButton(TransPonderDialog, IDC_RADIO16, TRUE);
				else if ( Transponder.fec == 6 ) CheckDlgButton(TransPonderDialog, IDC_RADIO17, TRUE);
				else if ( Transponder.fec == 7 ) CheckDlgButton(TransPonderDialog, IDC_RADIO18, TRUE);
				else if ( Transponder.fec == 8 ) CheckDlgButton(TransPonderDialog, IDC_RADIO19, TRUE);
				Transponder.srate=NITNeu[i].srate;
				SetDlgItemInt(TransPonderDialog,IDC_EDIT6,Transponder.srate,FALSE);
				Transponder.freq=NITNeu[i].freq;
				sprintf(zeile,"%10.3f",Transponder.freq/1000.f);
				SetDlgItemText(TransPonderDialog,IDC_EDIT2,zeile);
				PostMessage(TransPonderDialog,WM_COMMAND,IDTEST,0);
             			EndDialog(hDlg,TRUE);
						
			}
			
		}
		
		
		
		if (LOWORD(wParam) == IDOK
			|| LOWORD(wParam) == IDCANCEL) {
			EndDialog(hDlg,TRUE);
		}
		
		break;
    }
    return (FALSE);
	UNREFERENCED_PARAMETER(lParam);
}






BOOL HexCheck(unsigned char *Value)
{
	unsigned char NeuText[128];
	int i=0;
	unsigned char *Start=Value;
	NeuText[0]=0x00;
	NeuText[1]=0x00;
	NeuText[2]=0x00;
	
	while ( *Value != 0x00 ) {
		switch(*Value) {
		case 'a' : NeuText[i++]='A'; break;
		case 'b' : NeuText[i++]='B'; break;
		case 'c' : NeuText[i++]='C'; break;
		case 'd' : NeuText[i++]='D'; break;
		case 'e' : NeuText[i++]='E'; break;
		case 'f' : NeuText[i++]='F'; break;
		case 'A' : NeuText[i++]='A'; break;
		case 'B' : NeuText[i++]='B'; break;
		case 'C' : NeuText[i++]='C'; break;
		case 'D' : NeuText[i++]='D'; break;
		case 'E' : NeuText[i++]='E'; break;
		case 'F' : NeuText[i++]='F'; break;
		case '1' : NeuText[i++]='1'; break;
		case '2' : NeuText[i++]='2'; break;
		case '3' : NeuText[i++]='3'; break;
		case '4' : NeuText[i++]='4'; break;
		case '5' : NeuText[i++]='5'; break;
		case '6' : NeuText[i++]='6'; break;
		case '7' : NeuText[i++]='7'; break;
		case '8' : NeuText[i++]='8'; break;
		case '9' : NeuText[i++]='9'; break;
		case '0' : NeuText[i++]='0'; break;
		case ' ' : break;
		default  : return(FALSE);
		}
		NeuText[i]=0x00;
		Value++;
	};
	memcpy(Start,NeuText,i+1);
	return(TRUE);
};







BOOL APIENTRY CAListe(
					  HWND hDlg,
					  UINT message,
					  UINT wParam,
					  LONG lParam)
{
	int i,j,k;
	char zeile[512];
	char zeile1[255];
	LOGFONT Mfont = {9,0,0,0,0,0,0,0,0,0,0,0,0,"Fixedsys"};   
    HFONT currFont;
	
	
    switch (message) {
	case WM_INITDIALOG:
		currFont = CreateFontIndirect(&Mfont);
		SendMessage(GetDlgItem(hDlg,IDC_LIST1),WM_SETFONT,(WPARAM)currFont,TRUE);
		SendMessage(GetDlgItem(hDlg,IDC_LIST1),LB_RESETCONTENT,0,0);
		currFont = CreateFontIndirect(&Mfont);
		SendMessage(GetDlgItem(hDlg,IDC_EDIT6),WM_SETFONT,(WPARAM)currFont,TRUE);
		if ( Get_CAT() == TRUE ) {
			i=0;
			while ( i < TPCatAnzahl ) {
				Make_Crypt_Name((unsigned char)(TPCat[i].CA_ID>>8),zeile1);
				sprintf(zeile," %02x %3d %04x %04x  (%s)",TPCat[i].TAG,TPCat[i].DesLen,TPCat[i].CA_ID,TPCat[i].EMM,zeile1);
				SendMessage(GetDlgItem(hDlg,IDC_LIST1),LB_INSERTSTRING,i,(LPARAM)zeile);
				i++;
			};
			SendMessage(GetDlgItem(hDlg,IDC_LIST1),LB_SETCURSEL,0,0);
            SendMessage(hDlg,WM_USER,1,0);
            SetFocus(GetDlgItem(hDlg,IDC_LIST1));
		};
		return(TRUE);
		
	case WM_USER :
		
		if ( wParam == 1 ) {
            i=SendMessage(GetDlgItem(hDlg,IDC_LIST1),LB_GETCURSEL,0,0);
            if (( i >= 0 ) && ( i < TPCatAnzahl )) {  
				sprintf(zeile,"0x%02x",TPCat[i].TAG);
				SetDlgItemText(hDlg,IDC_EDIT1,zeile);
				sprintf(zeile,"%d",TPCat[i].DesLen);
				SetDlgItemText(hDlg,IDC_EDIT2,zeile);
				sprintf(zeile,"0x%04x",TPCat[i].CA_ID);
				SetDlgItemText(hDlg,IDC_EDIT3,zeile);
				Make_Crypt_Name((unsigned char)(TPCat[i].CA_ID>>8),zeile1);
				SetDlgItemText(hDlg,IDC_EDIT4,zeile1);
				sprintf(zeile,"0x%04x",TPCat[i].EMM);
				SetDlgItemText(hDlg,IDC_EDIT5,zeile);
				zeile[0]=0x00;
				k=0;
				for ( j = 0 ; j < TPCat[i].BufferLen ; j++ ) {
					sprintf(zeile1,"%02x ",TPCat[i].Buffer[j]);
					strcat(zeile,zeile1);
					if ( k > 15 ) {
						strcat(zeile,"\n");
						k=0;
					};
				};
				SetDlgItemText(hDlg,IDC_EDIT6,zeile);
            };
		};
		
		break;
		
	case WM_COMMAND:
		
		if (LOWORD(wParam) == IDC_LIST1 ) {
			if ( HIWORD( wParam ) == LBN_SELCHANGE  ) {
				SendMessage(hDlg,WM_USER,1,0);
			}
		}
		
		
		
		if (LOWORD(wParam) == IDOK
			|| LOWORD(wParam) == IDCANCEL) {
			EndDialog(hDlg,TRUE);
		}
		
		break;
    }
    return (FALSE);
	UNREFERENCED_PARAMETER(lParam);
}



BOOL APIENTRY ProgrammEdit(
						   HWND hDlg,
						   UINT message,
						   UINT wParam,
						   LONG lParam)
{
	char zeile[256];
	char Zeile1[128];
	char Zeile2[128];
    char PidName[128]; 
    unsigned long freq;
	float ffreq;
	
    PAINTSTRUCT wps;                   /* paint structure           */
    HDC hdc;
    HDC             hMemDC;
    HBITMAP         hOldBm;
	BITMAP bm;
	
    HBRUSH hBrush;
    HWND wndFill;
	
	int i,j; 
	RECT rc;
	int sig;
	int err;
	
	
	
	switch (message) {
	case WM_INITDIALOG:
		SetDlgItemInt(hDlg,IDC_PRGNR,EditProgramm+1,FALSE);
		SetDlgItemText(hDlg,IDC_EDIT1,Programm[EditProgramm].Name);
		SetTimer(hDlg,0,1000,NULL);
		
		if ( Programm[EditProgramm].Typ != 'D' ) {
			SetDlgItemText(hDlg,FREQTEXT,"Mhz");
				        CheckDlgButton(hDlg, IDC_RADIO1, TRUE);
                        EnableWindow(GetDlgItem(hDlg,IDC_RADIO7),FALSE);
                        EnableWindow(GetDlgItem(hDlg,IDC_RADIO8),FALSE);
                        EnableWindow(GetDlgItem(hDlg,IDC_RADIO9),FALSE);
                        EnableWindow(GetDlgItem(hDlg,IDC_RADIO10),FALSE);
                        EnableWindow(GetDlgItem(hDlg,IDC_RADIO11),FALSE);
                        EnableWindow(GetDlgItem(hDlg,IDC_RADIO12),FALSE);
                        EnableWindow(GetDlgItem(hDlg,IDC_RADIO13),FALSE);
                        EnableWindow(GetDlgItem(hDlg,IDC_RADIO14),FALSE);
                        EnableWindow(GetDlgItem(hDlg,IDC_RADIO15),FALSE);
                        EnableWindow(GetDlgItem(hDlg,IDC_RADIO16),FALSE);
                        EnableWindow(GetDlgItem(hDlg,IDC_RADIO17),FALSE);
                        EnableWindow(GetDlgItem(hDlg,IDC_RADIO18),FALSE);
                        EnableWindow(GetDlgItem(hDlg,IDC_RADIO19),FALSE);
                        EnableWindow(GetDlgItem(hDlg,IDC_RADIO20),FALSE);
                        EnableWindow(GetDlgItem(hDlg,IDC_RADIO21),FALSE);
                        EnableWindow(GetDlgItem(hDlg,IDC_EDIT6),FALSE);
                        EnableWindow(GetDlgItem(hDlg,IDC_EDIT7),FALSE);
                        EnableWindow(GetDlgItem(hDlg,IDC_EDIT8),FALSE);
                        EnableWindow(GetDlgItem(hDlg,IDC_EDIT9),FALSE);
                        EnableWindow(GetDlgItem(hDlg,IDC_EDIT10),FALSE);
                        EnableWindow(GetDlgItem(hDlg,IDC_EDIT11),FALSE);
                        EnableWindow(GetDlgItem(hDlg,IDC_EDIT12),FALSE);
                        EnableWindow(GetDlgItem(hDlg,IDC_RADIO24),FALSE);
                        EnableWindow(GetDlgItem(hDlg,IDC_RADIO25),FALSE);
                        EnableWindow(GetDlgItem(hDlg,IDC_CHECK6),FALSE);
                        EnableWindow(GetDlgItem(hDlg,IDC_CHECK7),FALSE);
                        EnableWindow(GetDlgItem(hDlg,IDGETPMT),FALSE);
                        EnableWindow(GetDlgItem(hDlg,IDGETSDT),FALSE);
                        EnableWindow(GetDlgItem(hDlg,IDC_EDIT13),FALSE);
						
                        EnableWindow(GetDlgItem(hDlg,IDC_EDIT14),FALSE);
                        EnableWindow(GetDlgItem(hDlg,IDC_EDIT17),FALSE);
                        EnableWindow(GetDlgItem(hDlg,IDC_EDIT18),FALSE);
                        EnableWindow(GetDlgItem(hDlg,IDC_EDIT19),FALSE);
                        EnableWindow(GetDlgItem(hDlg,IDC_LIST1),FALSE);
                        EnableWindow(GetDlgItem(hDlg,IDC_LIST2),FALSE);
                        EnableWindow(GetDlgItem(hDlg,IDGETPMT),FALSE);
                        EnableWindow(GetDlgItem(hDlg,IDGETSDT),FALSE);
						EnableWindow(GetDlgItem(hDlg,IDC_RADIO26),FALSE);
						EnableWindow(GetDlgItem(hDlg,IDC_RADIO27),FALSE);
						EnableWindow(GetDlgItem(hDlg,IDC_RADIO28),FALSE);
						EnableWindow(GetDlgItem(hDlg,IDC_RADIO29),FALSE);
						EnableWindow(GetDlgItem(hDlg,IDC_RADIO30),FALSE);
						
		} else {
			
			SetDlgItemText(hDlg,FREQTEXT,"Ghz");
			
			if ( Is_Sat_Karte() == TRUE ) {
				EnableWindow(GetDlgItem(hDlg,IDC_RADIO26),FALSE);
				EnableWindow(GetDlgItem(hDlg,IDC_RADIO27),FALSE);
				EnableWindow(GetDlgItem(hDlg,IDC_RADIO28),FALSE);
				EnableWindow(GetDlgItem(hDlg,IDC_RADIO29),FALSE);
				EnableWindow(GetDlgItem(hDlg,IDC_RADIO30),FALSE);
			};
			if ( Is_Kabel_Karte() == TRUE  ) {
				EnableWindow(GetDlgItem(hDlg,IDC_RADIO7),FALSE);
				EnableWindow(GetDlgItem(hDlg,IDC_RADIO8),FALSE);
				EnableWindow(GetDlgItem(hDlg,IDC_RADIO9),FALSE);
				EnableWindow(GetDlgItem(hDlg,IDC_RADIO10),FALSE);
				SetDlgItemText(hDlg,FREQTEXT,"Mhz");
				EnableWindow(GetDlgItem(hDlg, IDC_RADIO20), FALSE);
				EnableWindow(GetDlgItem(hDlg, IDC_RADIO21), FALSE);
				EnableWindow(GetDlgItem(hDlg, IDC_RADIO11), FALSE);
				EnableWindow(GetDlgItem(hDlg, IDC_RADIO12), FALSE);
				EnableWindow(GetDlgItem(hDlg, IDC_RADIO13), FALSE);
				EnableWindow(GetDlgItem(hDlg, IDC_RADIO14), FALSE);
				EnableWindow(GetDlgItem(hDlg, IDC_RADIO15), FALSE);
				EnableWindow(GetDlgItem(hDlg, IDC_RADIO16), FALSE);
				EnableWindow(GetDlgItem(hDlg, IDC_RADIO17), FALSE);
				EnableWindow(GetDlgItem(hDlg, IDC_RADIO18), FALSE);
				EnableWindow(GetDlgItem(hDlg, IDC_RADIO19), FALSE);
				
			};
			
			EnableWindow(GetDlgItem(hDlg,IDC_LIST1),TRUE);
			EnableWindow(GetDlgItem(hDlg,IDC_LIST2),TRUE);
			CheckDlgButton(hDlg, IDC_RADIO2, TRUE);
			EnableWindow(GetDlgItem(hDlg,IDGETPMT),TRUE);
			EnableWindow(GetDlgItem(hDlg,IDGETSDT),TRUE);
			
			if ( Programm[EditProgramm].diseqc == 0 ) CheckDlgButton(hDlg, IDC_RADIO7, TRUE);
			else if ( Programm[EditProgramm].diseqc == 1 ) CheckDlgButton(hDlg, IDC_RADIO8, TRUE);
			else if ( Programm[EditProgramm].diseqc == 2 ) CheckDlgButton(hDlg, IDC_RADIO9, TRUE);
			else if ( Programm[EditProgramm].diseqc == 3 ) CheckDlgButton(hDlg, IDC_RADIO10, TRUE);
			
			if ( UseDiseqC() == FALSE ) {
				EnableWindow(GetDlgItem(hDlg,IDC_RADIO7),FALSE);
				EnableWindow(GetDlgItem(hDlg,IDC_RADIO8),FALSE);
				EnableWindow(GetDlgItem(hDlg,IDC_RADIO9),FALSE);
				EnableWindow(GetDlgItem(hDlg,IDC_RADIO10),FALSE);
			} else {
				if ( LNB[1].Use == FALSE ) EnableWindow(GetDlgItem(hDlg,IDC_RADIO8),FALSE);
				if ( LNB[2].Use == FALSE ) EnableWindow(GetDlgItem(hDlg,IDC_RADIO9),FALSE);
				if ( LNB[3].Use == FALSE ) EnableWindow(GetDlgItem(hDlg,IDC_RADIO10),FALSE);
			};
			
			if ( Programm[EditProgramm].volt == 1 ) CheckDlgButton(hDlg, IDC_RADIO20, TRUE);
			else CheckDlgButton(hDlg, IDC_RADIO21, TRUE);
			
			if ( Programm[EditProgramm].afc == 1 ) CheckDlgButton(hDlg,IDC_RADIO24,TRUE);
			else CheckDlgButton(hDlg,IDC_RADIO25,TRUE);
			
			Make_Service_Name(Programm[EditProgramm].ServiceTyp,zeile);
			SetDlgItemText(hDlg,IDT_SERVICE,zeile);
			
			Make_Crypt_Systems(EditProgramm,zeile);
			SetDlgItemText(hDlg,IDT_CMODE,zeile);
			
			if ( Programm[EditProgramm].fec == 0 ) CheckDlgButton(hDlg, IDC_RADIO11, TRUE);
			else if ( Programm[EditProgramm].fec == 1 ) CheckDlgButton(hDlg, IDC_RADIO12, TRUE);
			else if ( Programm[EditProgramm].fec == 2 ) CheckDlgButton(hDlg, IDC_RADIO13, TRUE);
			else if ( Programm[EditProgramm].fec == 3 ) CheckDlgButton(hDlg, IDC_RADIO14, TRUE);
			else if ( Programm[EditProgramm].fec == 4 ) CheckDlgButton(hDlg, IDC_RADIO15, TRUE);
			else if ( Programm[EditProgramm].fec == 5 ) CheckDlgButton(hDlg, IDC_RADIO16, TRUE);
			else if ( Programm[EditProgramm].fec == 6 ) CheckDlgButton(hDlg, IDC_RADIO17, TRUE);
			else if ( Programm[EditProgramm].fec == 7 ) CheckDlgButton(hDlg, IDC_RADIO18, TRUE);
			else if ( Programm[EditProgramm].fec == 8 ) CheckDlgButton(hDlg, IDC_RADIO19, TRUE);
			
			if ( Programm[EditProgramm].qam == 0 ) CheckDlgButton(hDlg, IDC_RADIO26, TRUE);
			else if ( Programm[EditProgramm].qam == 1 ) CheckDlgButton(hDlg, IDC_RADIO27, TRUE);
			else if ( Programm[EditProgramm].qam == 2 ) CheckDlgButton(hDlg, IDC_RADIO28, TRUE);
			else if ( Programm[EditProgramm].qam == 3 ) CheckDlgButton(hDlg, IDC_RADIO29, TRUE);
			else if ( Programm[EditProgramm].qam == 4 ) CheckDlgButton(hDlg, IDC_RADIO30, TRUE);
			
			SetDlgItemInt(hDlg,IDC_EDIT6,Programm[EditProgramm].srate,FALSE);
			SetDlgItemInt(hDlg,IDC_EDIT7,Programm[EditProgramm].Video_pid,FALSE);
			SetDlgItemInt(hDlg,IDC_EDIT8,Programm[EditProgramm].Audio_pid,FALSE);
			SetDlgItemInt(hDlg,IDC_EDIT9,Programm[EditProgramm].PMT_pid,FALSE);
			SetDlgItemInt(hDlg,IDC_EDIT10,Programm[EditProgramm].TeleText_pid,FALSE);
			SetDlgItemInt(hDlg,IDC_EDIT11,Programm[EditProgramm].PCR_pid,FALSE);
			SetDlgItemInt(hDlg,IDC_EDIT12,Programm[EditProgramm].ECM_PID,FALSE);
			SetDlgItemInt(hDlg,IDC_EDIT13,Programm[EditProgramm].SID_pid,FALSE);
			SetDlgItemInt(hDlg,IDC_EDIT20,Programm[EditProgramm].tp_id,FALSE);
			SetDlgItemText(hDlg,IDC_EDIT17,Programm[EditProgramm].Anbieter);
			SetDlgItemText(hDlg,IDC_EDIT18,Programm[EditProgramm].Land);
			SetDlgItemInt(hDlg,IDC_EDIT14,Programm[EditProgramm].AC3_pid,FALSE);
			SendMessage (GetDlgItem(hDlg,IDC_LIST1), LB_RESETCONTENT, 0, 0);
			for ( i = 0 ; i < Programm[EditProgramm].Filteranzahl; i++ ) {
				Make_Pid_Name(Programm[EditProgramm].Filters[i].FilterId,&PidName[0]);
				sprintf( zeile,"%02x %04x %s (%s)", Programm[EditProgramm].Filters[i].FilterId,Programm[EditProgramm].Filters[i].PID,Programm[EditProgramm].Filters[i].FilterName,PidName);
				SendMessage (GetDlgItem(hDlg,IDC_LIST1), LB_INSERTSTRING, 0, (LPARAM)(LPSTR)zeile);
			};
			SendMessage (GetDlgItem(hDlg,IDC_LIST2), LB_RESETCONTENT, 0, 0);
			for ( i = 0 ; i < Programm[EditProgramm].CA_Anzahl; i++ ) {
				Make_Crypt_Name((unsigned char)(Programm[EditProgramm].CA_System[i].CA_Typ>>8),Zeile1);
				while (strlen(Zeile1) < 12 ) strcat(Zeile1," ");
				Zeile2[0]=0x00;
				if ((Programm[EditProgramm].CA_System[i].CA_Typ>>8) == 0x05 ) {
					Get_Viaccess_Provider_Name(Programm[EditProgramm].CA_System[i].Provider_Id,&Zeile2[0]);
				} else if ((Programm[EditProgramm].CA_System[i].CA_Typ>>8) == 0x01 ) {
					Get_Seca_Provider_Name(Programm[EditProgramm].CA_System[i].Provider_Id,&Zeile2[0]);
				} else if ((Programm[EditProgramm].CA_System[i].CA_Typ>>8) == 0x17 ) {
					Get_Ird_Beta_Provider_Name(Programm[EditProgramm].CA_System[i].EMM,&Zeile2[0]);
                } else if ((Programm[EditProgramm].CA_System[i].CA_Typ>>8) == 0x06 ) {
					Get_Ird_Beta_Provider_Name(Programm[EditProgramm].CA_System[i].EMM,&Zeile2[0]);
				};
				sprintf(zeile,"%04x %s EMM %04x ECM %04x  %s",Programm[EditProgramm].CA_System[i].CA_Typ,Zeile1,Programm[EditProgramm].CA_System[i].EMM,Programm[EditProgramm].CA_System[i].ECM,Zeile2);
				SendMessage (GetDlgItem(hDlg,IDC_LIST2), LB_INSERTSTRING, i, (LPARAM)(LPSTR)zeile);
			};
			if ( Programm[EditProgramm].CA_Anzahl > 0 ) EnableWindow(GetDlgItem(hDlg,IDGETCAT),TRUE);
			else EnableWindow(GetDlgItem(hDlg,IDGETCAT),FALSE);
			
};

sprintf(zeile,"%10.3f",Programm[EditProgramm].freq/1000.f);
SetDlgItemText(hDlg,IDC_EDIT2,zeile);
return(TRUE);


        case WM_TIMER:
			rc.top=111;
			rc.left=43;
			rc.bottom=162;
			rc.right=377;
			if ( Current_Mode == 1 ) InvalidateRect(hDlg,&rc,FALSE);
			break;
			
        case WM_PAINT:
			
			hdc = BeginPaint(hDlg,&wps);
			if ( Current_Mode == 1 ) {
				Get_Signal_Infos((unsigned char *)&dvb_front.sync,&sig,&err);
				hMemDC = CreateCompatibleDC(hdc);
				if (( dvb_front.sync&0x0f) > 0 ) hOldBm = SelectObject(hMemDC, BirneGruen);
				else hOldBm = SelectObject(hMemDC, BirneRot );
				GetObject(BirneGruen, sizeof(BITMAP), (LPSTR)&bm);
				
				BitBlt(hdc, 44, 114, bm.bmWidth, bm.bmHeight,hMemDC, 0, 0, SRCCOPY);// Signal
				
				SelectObject(hMemDC,hOldBm);
				if (( dvb_front.sync&0x0f) > 1 ) hOldBm = SelectObject(hMemDC, BirneGruen);
				else hOldBm = SelectObject(hMemDC, BirneRot );
				
				BitBlt(hdc, 44, 140, bm.bmWidth, bm.bmHeight,hMemDC, 0, 0, SRCCOPY); // Carrier
				SelectObject(hMemDC,hOldBm);
				
				
				if (( dvb_front.sync&0x0f) > 3 ) hOldBm = SelectObject(hMemDC, BirneGruen);
				else hOldBm = SelectObject(hMemDC, BirneRot );
				
				BitBlt(hdc, 100, 114, bm.bmWidth, bm.bmHeight,hMemDC, 0, 0, SRCCOPY); // Viterbi
				SelectObject(hMemDC,hOldBm);
				
				if (( dvb_front.sync&0x0f) > 7 ) hOldBm = SelectObject(hMemDC, BirneGruen);
				else hOldBm = SelectObject(hMemDC, BirneRot );
				
				BitBlt(hdc, 100, 140, bm.bmWidth, bm.bmHeight,hMemDC, 0, 0, SRCCOPY); // FSync
				SelectObject(hMemDC,hOldBm);
				DeleteDC(hMemDC);
				DeleteDC(hdc);
				for ( i = 0; i < 21 ; i++ ) {
					wndFill = GetDlgItem (hDlg, S1+i );
					GetClientRect(wndFill, &rc);
					hdc = GetDC(wndFill);
					rc.bottom-=1;
					rc.top+=1;
					rc.right-=1;
					rc.left+=1;
					
					if ( sig >= i * 12 ) {
						hBrush=CreateSolidBrush(RGB(255-(12*i),12*i,0));
					} else {
						hBrush=CreateSolidBrush(RGB(0,0,0));
					};
                    FillRect (hdc, &rc, hBrush);
					ReleaseDC(wndFill, hdc);
					DeleteObject(hBrush);
					
					wndFill = GetDlgItem (hDlg, E1+i );
					GetClientRect(wndFill, &rc);
					
					hdc = GetDC(wndFill);
					rc.bottom-=1;
					rc.top+=1;
					rc.right-=1;
					rc.left+=1;
					
					if ( err >= i * 12 ) {
						hBrush=CreateSolidBrush(RGB(255-(12*i),12*i,0));
					} else {
						hBrush=CreateSolidBrush(RGB(0,0,0));
					};
                    FillRect (hdc, &rc, hBrush);
					ReleaseDC (wndFill, hdc);
					DeleteObject(hBrush);
				}
				
			};
            EndPaint(hDlg,&wps);
			return(FALSE);
			
			
        case WM_COMMAND:
			
			if ( ((HWND)lParam==GetDlgItem(hDlg,IDC_LIST2)) &&
				(HIWORD(wParam)==LBN_DBLCLK)) {
				
				i=SendMessage(GetDlgItem(hDlg,IDC_LIST2),LB_GETCURSEL,0,0);
				if (( i >= 0 ) && ( i < Programm[EditProgramm].CA_Anzahl )) {
					Programm[EditProgramm].ECM_PID=Programm[EditProgramm].CA_System[i].ECM;
					SetDlgItemInt(hDlg,IDC_EDIT12,Programm[EditProgramm].ECM_PID,FALSE);
				};
				
				
				return(TRUE);
			}
			
			
			if ( ((HWND)lParam==GetDlgItem(hDlg,IDC_RADIO1)) ||
				((HWND)lParam==GetDlgItem(hDlg,IDC_RADIO2)) ) {
				if ( IsDlgButtonChecked(hDlg, IDC_RADIO1)) {
					SetDlgItemText(hDlg,FREQTEXT,"Mhz");
					EnableWindow(GetDlgItem(hDlg,IDC_RADIO7),FALSE);
					EnableWindow(GetDlgItem(hDlg,IDC_RADIO8),FALSE);
					EnableWindow(GetDlgItem(hDlg,IDC_RADIO9),FALSE);
					EnableWindow(GetDlgItem(hDlg,IDC_RADIO10),FALSE);
					EnableWindow(GetDlgItem(hDlg,IDC_RADIO11),FALSE);
					EnableWindow(GetDlgItem(hDlg,IDC_RADIO12),FALSE);
					EnableWindow(GetDlgItem(hDlg,IDC_RADIO13),FALSE);
					EnableWindow(GetDlgItem(hDlg,IDC_RADIO14),FALSE);
					EnableWindow(GetDlgItem(hDlg,IDC_RADIO15),FALSE);
					EnableWindow(GetDlgItem(hDlg,IDC_RADIO16),FALSE);
					EnableWindow(GetDlgItem(hDlg,IDC_RADIO17),FALSE);
					EnableWindow(GetDlgItem(hDlg,IDC_RADIO18),FALSE);
					EnableWindow(GetDlgItem(hDlg,IDC_RADIO19),FALSE);
					EnableWindow(GetDlgItem(hDlg,IDC_RADIO20),FALSE);
					EnableWindow(GetDlgItem(hDlg,IDC_RADIO21),FALSE);
					EnableWindow(GetDlgItem(hDlg,IDC_RADIO24),FALSE);
					EnableWindow(GetDlgItem(hDlg,IDC_RADIO25),FALSE);
					EnableWindow(GetDlgItem(hDlg,IDC_EDIT6),FALSE);
					EnableWindow(GetDlgItem(hDlg,IDC_EDIT7),FALSE);
					EnableWindow(GetDlgItem(hDlg,IDC_EDIT8),FALSE);
					EnableWindow(GetDlgItem(hDlg,IDC_EDIT9),FALSE);
					EnableWindow(GetDlgItem(hDlg,IDC_EDIT10),FALSE);
					EnableWindow(GetDlgItem(hDlg,IDC_EDIT11),FALSE);
					EnableWindow(GetDlgItem(hDlg,IDC_EDIT12),FALSE);
					EnableWindow(GetDlgItem(hDlg,IDC_CHECK6),FALSE);
					EnableWindow(GetDlgItem(hDlg,IDC_CHECK7),FALSE);
					EnableWindow(GetDlgItem(hDlg,IDGETPMT),FALSE);
					EnableWindow(GetDlgItem(hDlg,IDGETSDT),FALSE);
					
					EnableWindow(GetDlgItem(hDlg,IDC_EDIT13),FALSE);
					
					EnableWindow(GetDlgItem(hDlg,IDC_EDIT14),FALSE);
					EnableWindow(GetDlgItem(hDlg,IDC_EDIT17),FALSE);
					EnableWindow(GetDlgItem(hDlg,IDC_EDIT18),FALSE);
					EnableWindow(GetDlgItem(hDlg,IDC_EDIT19),FALSE);
					EnableWindow(GetDlgItem(hDlg,IDC_LIST1),FALSE);
					EnableWindow(GetDlgItem(hDlg,IDC_LIST2),FALSE);
					EnableWindow(GetDlgItem(hDlg,IDC_RADIO26),FALSE);
					EnableWindow(GetDlgItem(hDlg,IDC_RADIO27),FALSE);
					EnableWindow(GetDlgItem(hDlg,IDC_RADIO28),FALSE);
					EnableWindow(GetDlgItem(hDlg,IDC_RADIO29),FALSE);
					EnableWindow(GetDlgItem(hDlg,IDC_RADIO30),FALSE);
				} else {
					SetDlgItemText(hDlg,FREQTEXT,"Ghz");
					EnableWindow(GetDlgItem(hDlg,IDC_RADIO7),TRUE);
					EnableWindow(GetDlgItem(hDlg,IDC_RADIO8),TRUE);
					EnableWindow(GetDlgItem(hDlg,IDC_RADIO9),TRUE);
					EnableWindow(GetDlgItem(hDlg,IDC_RADIO10),TRUE);
					EnableWindow(GetDlgItem(hDlg,IDC_RADIO11),TRUE);
					EnableWindow(GetDlgItem(hDlg,IDC_RADIO12),TRUE);
					EnableWindow(GetDlgItem(hDlg,IDC_RADIO13),TRUE);
					EnableWindow(GetDlgItem(hDlg,IDC_RADIO14),TRUE);
					EnableWindow(GetDlgItem(hDlg,IDC_RADIO15),TRUE);
					EnableWindow(GetDlgItem(hDlg,IDC_RADIO16),TRUE);
					EnableWindow(GetDlgItem(hDlg,IDC_RADIO17),TRUE);
					EnableWindow(GetDlgItem(hDlg,IDC_RADIO18),TRUE);
					EnableWindow(GetDlgItem(hDlg,IDC_RADIO19),TRUE);
					EnableWindow(GetDlgItem(hDlg,IDC_RADIO20),TRUE);
					EnableWindow(GetDlgItem(hDlg,IDC_RADIO21),TRUE);
					EnableWindow(GetDlgItem(hDlg,IDC_LIST1),TRUE);
					EnableWindow(GetDlgItem(hDlg,IDC_LIST2),TRUE);
					EnableWindow(GetDlgItem(hDlg,IDC_RADIO24),TRUE);
					EnableWindow(GetDlgItem(hDlg,IDC_RADIO25),TRUE);
					
					EnableWindow(GetDlgItem(hDlg,IDC_EDIT6),TRUE);
					EnableWindow(GetDlgItem(hDlg,IDC_EDIT7),TRUE);
					EnableWindow(GetDlgItem(hDlg,IDC_EDIT8),TRUE);
					EnableWindow(GetDlgItem(hDlg,IDC_EDIT9),TRUE);
					EnableWindow(GetDlgItem(hDlg,IDC_EDIT10),TRUE);
					EnableWindow(GetDlgItem(hDlg,IDC_EDIT11),TRUE);
					EnableWindow(GetDlgItem(hDlg,IDC_EDIT12),TRUE);
					EnableWindow(GetDlgItem(hDlg,IDC_CHECK6),TRUE);
					EnableWindow(GetDlgItem(hDlg,IDC_CHECK7),TRUE);
					
					
					if ( Current_Mode == 1 ) {
                        EnableWindow(GetDlgItem(hDlg,IDGETPMT),TRUE);
                        EnableWindow(GetDlgItem(hDlg,IDGETSDT),TRUE);
					} else {
                        EnableWindow(GetDlgItem(hDlg,IDGETPMT),FALSE);
                        EnableWindow(GetDlgItem(hDlg,IDGETSDT),FALSE);
					};
					EnableWindow(GetDlgItem(hDlg,IDC_EDIT13),TRUE);
					
					EnableWindow(GetDlgItem(hDlg,IDC_EDIT14),TRUE);
					EnableWindow(GetDlgItem(hDlg,IDC_EDIT17),TRUE);
					EnableWindow(GetDlgItem(hDlg,IDC_EDIT18),TRUE);
					EnableWindow(GetDlgItem(hDlg,IDC_EDIT19),TRUE);
					EnableWindow(GetDlgItem(hDlg,IDC_COMBO5),TRUE);
					
					CheckDlgButton(hDlg, IDC_RADIO7, TRUE);
					
					if (( LNB[1].Use == FALSE ) && ( LNB[2].Use == FALSE ) && ( LNB[3].Use == FALSE )) {
						EnableWindow(GetDlgItem(hDlg,IDC_RADIO7),FALSE);
						EnableWindow(GetDlgItem(hDlg,IDC_RADIO8),FALSE);
						EnableWindow(GetDlgItem(hDlg,IDC_RADIO9),FALSE);
						EnableWindow(GetDlgItem(hDlg,IDC_RADIO10),FALSE);
					} else {
						if ( LNB[1].Use == FALSE ) EnableWindow(GetDlgItem(hDlg,IDC_RADIO8),FALSE);
						if ( LNB[2].Use == FALSE ) EnableWindow(GetDlgItem(hDlg,IDC_RADIO9),FALSE);
						if ( LNB[3].Use == FALSE ) EnableWindow(GetDlgItem(hDlg,IDC_RADIO10),FALSE);
					};
					
					if ( Is_Sat_Karte() == TRUE  ) {
						EnableWindow(GetDlgItem(hDlg,IDC_RADIO26),FALSE);
						EnableWindow(GetDlgItem(hDlg,IDC_RADIO27),FALSE);
						EnableWindow(GetDlgItem(hDlg,IDC_RADIO28),FALSE);
						EnableWindow(GetDlgItem(hDlg,IDC_RADIO29),FALSE);
						EnableWindow(GetDlgItem(hDlg,IDC_RADIO30),FALSE);
						
					};
					if ( Is_Kabel_Karte() == TRUE  ) {
						EnableWindow(GetDlgItem(hDlg,IDC_RADIO7),FALSE);
						EnableWindow(GetDlgItem(hDlg,IDC_RADIO8),FALSE);
						EnableWindow(GetDlgItem(hDlg,IDC_RADIO9),FALSE);
						EnableWindow(GetDlgItem(hDlg,IDC_RADIO10),FALSE);
						SetDlgItemText(hDlg,FREQTEXT,"Mhz");
						EnableWindow(GetDlgItem(hDlg, IDC_RADIO20), FALSE);
						EnableWindow(GetDlgItem(hDlg, IDC_RADIO21), FALSE);
						EnableWindow(GetDlgItem(hDlg, IDC_RADIO11), FALSE);
						EnableWindow(GetDlgItem(hDlg, IDC_RADIO12), FALSE);
						EnableWindow(GetDlgItem(hDlg, IDC_RADIO13), FALSE);
						EnableWindow(GetDlgItem(hDlg, IDC_RADIO14), FALSE);
						EnableWindow(GetDlgItem(hDlg, IDC_RADIO15), FALSE);
						EnableWindow(GetDlgItem(hDlg, IDC_RADIO16), FALSE);
						EnableWindow(GetDlgItem(hDlg, IDC_RADIO17), FALSE);
						EnableWindow(GetDlgItem(hDlg, IDC_RADIO18), FALSE);
						EnableWindow(GetDlgItem(hDlg, IDC_RADIO19), FALSE);
						EnableWindow(GetDlgItem(hDlg,IDC_RADIO26),TRUE);
						EnableWindow(GetDlgItem(hDlg,IDC_RADIO27),TRUE);
						EnableWindow(GetDlgItem(hDlg,IDC_RADIO28),TRUE);
						EnableWindow(GetDlgItem(hDlg,IDC_RADIO29),TRUE);
						EnableWindow(GetDlgItem(hDlg,IDC_RADIO30),TRUE);
						
					};
					
					
				}
			  }
			  
			  if (LOWORD(wParam) == IDOK ) {
				  GetDlgItemText(hDlg,IDC_EDIT1,Programm[EditProgramm].Name,sizeof(Programm[EditProgramm].Name));
				  
				  
				  GetDlgItemText(hDlg,IDC_EDIT19,zeile,sizeof(zeile));
				  
				  GetDlgItemText(hDlg,IDC_EDIT2,zeile,sizeof(zeile));
				  ffreq=(float)atof(zeile);
				  freq=(unsigned	long)(ffreq*1000);
				  Programm[EditProgramm].freq=freq;
				  if ( IsDlgButtonChecked(hDlg, IDC_RADIO1)) {
					  Programm[EditProgramm].Typ ='A';
				  } else {
					  Programm[EditProgramm].Typ ='D';
					  Programm[EditProgramm].afc=IsDlgButtonChecked(hDlg,IDC_RADIO24);
					  
					  if ( IsDlgButtonChecked(hDlg, IDC_RADIO7)) Programm[EditProgramm].diseqc = 0;
					  else if ( IsDlgButtonChecked(hDlg, IDC_RADIO8)) Programm[EditProgramm].diseqc = 1;
					  else if ( IsDlgButtonChecked(hDlg, IDC_RADIO9)) Programm[EditProgramm].diseqc = 2;
                      else if ( IsDlgButtonChecked(hDlg, IDC_RADIO10)) Programm[EditProgramm].diseqc = 3;
					  
					  if ( IsDlgButtonChecked(hDlg, IDC_RADIO11)) Programm[EditProgramm].fec = 0;
					  else if ( IsDlgButtonChecked(hDlg, IDC_RADIO12)) Programm[EditProgramm].fec = 1;
					  else if ( IsDlgButtonChecked(hDlg, IDC_RADIO13)) Programm[EditProgramm].fec = 2;
					  else if ( IsDlgButtonChecked(hDlg, IDC_RADIO14)) Programm[EditProgramm].fec = 3;
					  else if ( IsDlgButtonChecked(hDlg, IDC_RADIO15)) Programm[EditProgramm].fec = 4;
					  else if ( IsDlgButtonChecked(hDlg, IDC_RADIO16)) Programm[EditProgramm].fec = 5;
					  else if ( IsDlgButtonChecked(hDlg, IDC_RADIO17)) Programm[EditProgramm].fec = 6;
					  else if ( IsDlgButtonChecked(hDlg, IDC_RADIO18)) Programm[EditProgramm].fec = 7;
					  else if ( IsDlgButtonChecked(hDlg, IDC_RADIO19)) Programm[EditProgramm].fec = 8;
					  
					  if ( IsDlgButtonChecked(hDlg, IDC_RADIO26))  Programm[EditProgramm].qam = 0 ;
					  else if ( IsDlgButtonChecked(hDlg, IDC_RADIO27))  Programm[EditProgramm].qam = 1 ;
					  else if ( IsDlgButtonChecked(hDlg, IDC_RADIO28))  Programm[EditProgramm].qam = 2 ;
					  else if ( IsDlgButtonChecked(hDlg, IDC_RADIO29))  Programm[EditProgramm].qam = 3 ;
					  else if ( IsDlgButtonChecked(hDlg, IDC_RADIO30))  Programm[EditProgramm].qam = 4 ;
					  
					  if ( IsDlgButtonChecked(hDlg, IDC_RADIO3)) Programm[EditProgramm].TVType = 0;
					  else Programm[EditProgramm].TVType = 3 ;
					  
					  Programm[EditProgramm].srate=GetDlgItemInt(hDlg,IDC_EDIT6,NULL,FALSE);
					  Programm[EditProgramm].Video_pid=GetDlgItemInt(hDlg,IDC_EDIT7,NULL,FALSE);
					  Programm[EditProgramm].Audio_pid=GetDlgItemInt(hDlg,IDC_EDIT8,NULL,FALSE);
					  Programm[EditProgramm].PMT_pid=GetDlgItemInt(hDlg,IDC_EDIT9,NULL,FALSE);
					  Programm[EditProgramm].TeleText_pid=GetDlgItemInt(hDlg,IDC_EDIT10,NULL,FALSE);
					  Programm[EditProgramm].PCR_pid=GetDlgItemInt(hDlg,IDC_EDIT11,NULL,FALSE);
					  Programm[EditProgramm].ECM_PID=GetDlgItemInt(hDlg,IDC_EDIT12,NULL,FALSE);
					  Programm[EditProgramm].SID_pid=GetDlgItemInt(hDlg,IDC_EDIT13,NULL,FALSE);
					  GetDlgItemText(hDlg,IDC_EDIT17,Programm[EditProgramm].Anbieter,sizeof(Programm[EditProgramm].Anbieter));
					  GetDlgItemText(hDlg,IDC_EDIT18,Programm[EditProgramm].Land,sizeof(Programm[EditProgramm].Land));
					  Programm[EditProgramm].AC3_pid=GetDlgItemInt(hDlg,IDC_EDIT14,NULL,FALSE);
					  
				  }
				  
				  
				  KillTimer(hDlg,0);
				  Write_Programm_List();
				  Write_Nit_List();
				  Set_DVB_Channel(EditProgramm);
				  EndDialog(hDlg,TRUE);
			  }
			  
			  
			  if (LOWORD(wParam) == IDGETSDT ) {
				  if ( Current_Mode == 0 ) break; 
				  for ( i=0; i < NEUSIZE ; i++ ) {
					  memset(&ProgrammNeu[i].Name[0],0x00,sizeof( struct TProgramm));
					  ProgrammNeu[i].Video_pid=0x1fff;        
					  ProgrammNeu[i].Audio_pid=0x1fff;
					  ProgrammNeu[i].TeleText_pid=0x1fff;          /* Teletext PID */
					  ProgrammNeu[i].CA_Anzahl=0;
					  ProgrammNeu[i].CA_Land[0]=0;
					  ProgrammNeu[i].PMT_pid=0x1fff;
					  ProgrammNeu[i].PCR_pid=0x1fff;
					  ProgrammNeu[i].ECM_PID=0x1fff;
					  ProgrammNeu[i].AC3_pid=0x1fff;
					  ProgrammNeu[i].SID_pid=0x1fff;
					  ProgrammNeu[i].Merker=0x00;
					  ProgrammNeu[i].Link_SID=0xffff;
					  ProgrammNeu[i].Link_TP=0xffff;
					  ProgrammNeu[i].Dynamisch=0x00;
				  };
				  
				  if ( Get_SDT() == FALSE ) {
					  MessageBox(hDlg,"Keine SDT-Daten gefunden","Multi-Dec",MB_ICONSTOP|MB_OK);
					  break;
				  };
				  
				  i=Get_New_Prog_by_SID(Programm[EditProgramm].SID_pid);
				  if ( i < 0 ) {
					  sprintf( zeile,"Keine SDT-Daten für %s ( 0x%04x ) gefunden ! ",Programm[EditProgramm].Name,Programm[EditProgramm].SID_pid);
					  MessageBox(hDlg,zeile,"Multi-Dec",MB_ICONSTOP|MB_OK);
					  break;
				  }
				  
				  SetDlgItemInt(hDlg,IDC_EDIT13,ProgrammNeu[i].SID_pid,FALSE);
				  
				  Programm[EditProgramm].ServiceTyp=ProgrammNeu[i].ServiceTyp;
				  Make_Service_Name(Programm[EditProgramm].ServiceTyp,zeile);
				  SetDlgItemText(hDlg,IDT_SERVICE,zeile);
				  
				  Programm[EditProgramm].CA_ID=ProgrammNeu[i].CA_ID;
				  Make_Crypt_Systems(EditProgramm,zeile);
				  SetDlgItemText(hDlg,IDT_CMODE,zeile);
				  
				  if ( Programm[EditProgramm].CA_Anzahl > 0 ) EnableWindow(GetDlgItem(hDlg,IDGETCAT),TRUE);
				  else EnableWindow(GetDlgItem(hDlg,IDGETCAT),FALSE);
				  
				  SetDlgItemText(hDlg,IDC_EDIT17,ProgrammNeu[i].Anbieter);
				  SetDlgItemText(hDlg,IDC_EDIT18,ProgrammNeu[i].Land);
				  SetDlgItemText(hDlg,IDC_EDIT1,ProgrammNeu[i].Name);
			  };
			  
			  if (LOWORD(wParam) == IDGETPMT ) {
				  if ( Current_Mode == 0 ) break; 
				  memset(&ProgrammNeu[0].Name[0],0x00,sizeof( struct TProgramm));
				  ProgrammNeu[0].Video_pid=0x1fff;        
				  ProgrammNeu[0].Audio_pid=0x1fff;
				  ProgrammNeu[0].TeleText_pid=0x1fff;          /* Teletext PID */
				  ProgrammNeu[0].Merker=0x00;
				  ProgrammNeu[0].Link_SID=0xffff;
				  ProgrammNeu[0].Link_TP=0xffff;
				  ProgrammNeu[0].Dynamisch=0x00;
				  
				  ProgrammNeu[0].PMT_pid=Programm[EditProgramm].PMT_pid;
				  SendMessage (GetDlgItem(hDlg,IDC_LIST1), LB_RESETCONTENT, 0, 0);
				  SendMessage (GetDlgItem(hDlg,IDC_LIST2), LB_RESETCONTENT, 0, 0);
				  if ( Get_PMT(0,ProgrammNeu[0].PMT_pid) == FALSE ) {
					  MessageBox(hDlg,"Keine PMT-Daten gefunden","Multi-Dec",MB_ICONSTOP|MB_OK);
					  break;
				  };
				  
				  
				  SetDlgItemInt(hDlg,IDC_EDIT7,ProgrammNeu[0].Video_pid,FALSE);
				  SetDlgItemInt(hDlg,IDC_EDIT8,ProgrammNeu[0].Audio_pid,FALSE);
				  SetDlgItemInt(hDlg,IDC_EDIT10,ProgrammNeu[0].TeleText_pid,FALSE);
				  Programm[EditProgramm].Filteranzahl=ProgrammNeu[0].Filteranzahl;
				  
				  SendMessage (GetDlgItem(hDlg,IDC_LIST1), LB_RESETCONTENT, 0, 0);
				  for ( i = 0 ; i < ProgrammNeu[0].Filteranzahl; i++ ) {
					  Programm[EditProgramm].Filters[i].PID=ProgrammNeu[0].Filters[i].PID;
					  Programm[EditProgramm].Filters[i].FilterId=ProgrammNeu[0].Filters[i].FilterId;
					  memcpy(&Programm[EditProgramm].Filters[i].FilterName[0],&ProgrammNeu[0].Filters[i].FilterName[0],5);
					  sprintf( zeile,"%02x %04x   %s", ProgrammNeu[0].Filters[i].FilterId,ProgrammNeu[0].Filters[i].PID,ProgrammNeu[0].Filters[i].FilterName);
					  SendMessage (GetDlgItem(hDlg,IDC_LIST1), LB_INSERTSTRING, 0, (LPARAM)(LPSTR)zeile);
				  };
				  
				  Programm[EditProgramm].CA_Anzahl=ProgrammNeu[0].CA_Anzahl;
				  SendMessage (GetDlgItem(hDlg,IDC_LIST2), LB_RESETCONTENT, 0, 0);
				  for ( i = 0 ; i < ProgrammNeu[0].CA_Anzahl; i++ ) {
					  Programm[EditProgramm].CA_System[i].CA_Typ=ProgrammNeu[0].CA_System[i].CA_Typ;
					  Programm[EditProgramm].CA_System[i].ECM=ProgrammNeu[0].CA_System[i].ECM;
					  Programm[EditProgramm].CA_System[i].EMM=ProgrammNeu[0].CA_System[i].EMM;
					  Programm[EditProgramm].CA_System[i].Provider_Id=ProgrammNeu[0].CA_System[i].Provider_Id;
					  Make_Crypt_Name((unsigned char)(Programm[EditProgramm].CA_System[i].CA_Typ>>8),Zeile1);
					  while (strlen(Zeile1) < 12 ) strcat(Zeile1," ");
				Zeile2[0]=0x00;
				if ((Programm[EditProgramm].CA_System[i].CA_Typ>>8) == 0x05 ) {
					Get_Viaccess_Provider_Name(Programm[EditProgramm].CA_System[i].Provider_Id,&Zeile2[0]);
				} else if ((Programm[EditProgramm].CA_System[i].CA_Typ>>8) == 0x01 ) {
					Get_Seca_Provider_Name(Programm[EditProgramm].CA_System[i].Provider_Id,&Zeile2[0]);
				} else if ((Programm[EditProgramm].CA_System[i].CA_Typ>>8) == 0x17 ) {
					Get_Ird_Beta_Provider_Name(Programm[EditProgramm].CA_System[i].EMM,&Zeile2[0]);
                } else if ((Programm[EditProgramm].CA_System[i].CA_Typ>>8) == 0x06 ) {
					Get_Ird_Beta_Provider_Name(Programm[EditProgramm].CA_System[i].EMM,&Zeile2[0]);
				};
				sprintf(zeile,"%04x %s EMM %04x ECM %04x  %s",Programm[EditProgramm].CA_System[i].CA_Typ,Zeile1,Programm[EditProgramm].CA_System[i].EMM,Programm[EditProgramm].CA_System[i].ECM,Zeile2);
					  SendMessage (GetDlgItem(hDlg,IDC_LIST2), LB_INSERTSTRING, i, (LPARAM)(LPSTR)zeile);
				  };
				  memcpy(Programm[EditProgramm].CA_Land,ProgrammNeu[0].CA_Land,sizeof(ProgrammNeu[0].CA_Land));
				  
				  if ( Programm[EditProgramm].CA_Anzahl > 0 ) EnableWindow(GetDlgItem(hDlg,IDGETCAT),TRUE);
				  else EnableWindow(GetDlgItem(hDlg,IDGETCAT),FALSE);
				  
				  Make_Crypt_Systems(EditProgramm,zeile);
				  SetDlgItemText(hDlg,IDT_CMODE,zeile);
				  
			  };
			  
			  
			  if (LOWORD(wParam) == IDGETCAT ) {
				  if ( Current_Mode == 0 ) break; 
				  SendMessage (GetDlgItem(hDlg,IDC_LIST2), LB_RESETCONTENT, 0, 0);
				  if ( Get_CAT() == TRUE ) {
					  for ( i=0; i < TPCatAnzahl; i++ ) {
						  for ( j =0; j < Programm[EditProgramm].CA_Anzahl; j++ ) {
							  if ( TPCat[i].CA_ID == Programm[EditProgramm].CA_System[j].CA_Typ ) {
								  Programm[EditProgramm].CA_System[j].EMM=TPCat[i].EMM;
							  };
						  }
					  }
				  } else MessageBox(hDlg,"Keine CAT-Daten gefunden","Multi-Dec",MB_ICONSTOP|MB_OK);
				  
				  for ( i = 0 ; i < Programm[EditProgramm].CA_Anzahl; i++ ) {
					  Make_Crypt_Name((unsigned char)(Programm[EditProgramm].CA_System[i].CA_Typ>>8),Zeile1);
					  while (strlen(Zeile1) < 12 ) strcat(Zeile1," ");
						Zeile2[0]=0x00;
						if ((Programm[EditProgramm].CA_System[i].CA_Typ>>8) == 0x05 ) {
							Get_Viaccess_Provider_Name(Programm[EditProgramm].CA_System[i].Provider_Id,&Zeile2[0]);
						} else if ((Programm[EditProgramm].CA_System[i].CA_Typ>>8) == 0x01 ) {
							Get_Seca_Provider_Name(Programm[EditProgramm].CA_System[i].Provider_Id,&Zeile2[0]);
						} else if ((Programm[EditProgramm].CA_System[i].CA_Typ>>8) == 0x17 ) {
							Get_Ird_Beta_Provider_Name(Programm[EditProgramm].CA_System[i].EMM,&Zeile2[0]);
		                } else if ((Programm[EditProgramm].CA_System[i].CA_Typ>>8) == 0x06 ) {
							Get_Ird_Beta_Provider_Name(Programm[EditProgramm].CA_System[i].EMM,&Zeile2[0]);
					};
     				sprintf(zeile,"%04x %s EMM %04x ECM %04x  %s",Programm[EditProgramm].CA_System[i].CA_Typ,Zeile1,Programm[EditProgramm].CA_System[i].EMM,Programm[EditProgramm].CA_System[i].ECM,Zeile2);
					  SendMessage (GetDlgItem(hDlg,IDC_LIST2), LB_INSERTSTRING, i, (LPARAM)(LPSTR)zeile);
				  };
				  break;
			  }
			  
			  
			  
			  if (LOWORD(wParam) == IDTEST ) {
				  GetDlgItemText(hDlg,IDC_EDIT1,Programm[EditProgramm].Name,sizeof(Programm[EditProgramm].Name));
				  GetDlgItemText(hDlg,IDC_EDIT2,zeile,sizeof(zeile));
				  ffreq=(float)atof(zeile);
				  freq=(unsigned	long)(ffreq*1000);
				  Programm[MAXPROGS].freq=freq;
				  if ( IsDlgButtonChecked(hDlg, IDC_RADIO1)) {
					  Programm[MAXPROGS].Typ ='A';
				  } else {
					  Programm[MAXPROGS].Typ ='D';
					  if ( IsDlgButtonChecked(hDlg, IDC_RADIO7)) Programm[MAXPROGS].diseqc = 0;
					  else if ( IsDlgButtonChecked(hDlg, IDC_RADIO8)) Programm[MAXPROGS].diseqc = 1;
					  else if ( IsDlgButtonChecked(hDlg, IDC_RADIO9)) Programm[MAXPROGS].diseqc = 2;
                      else if ( IsDlgButtonChecked(hDlg, IDC_RADIO10)) Programm[MAXPROGS].diseqc = 3;
                      Programm[MAXPROGS].volt= IsDlgButtonChecked(hDlg, IDC_RADIO20);
					  
					  
					  if ( IsDlgButtonChecked(hDlg, IDC_RADIO11)) Programm[MAXPROGS].fec = 0;
					  else if ( IsDlgButtonChecked(hDlg, IDC_RADIO12)) Programm[MAXPROGS].fec = 1;
					  else if ( IsDlgButtonChecked(hDlg, IDC_RADIO13)) Programm[MAXPROGS].fec = 2;
					  else if ( IsDlgButtonChecked(hDlg, IDC_RADIO14)) Programm[MAXPROGS].fec = 3;
					  else if ( IsDlgButtonChecked(hDlg, IDC_RADIO15)) Programm[MAXPROGS].fec = 4;
					  else if ( IsDlgButtonChecked(hDlg, IDC_RADIO16)) Programm[MAXPROGS].fec = 5;
					  else if ( IsDlgButtonChecked(hDlg, IDC_RADIO17)) Programm[MAXPROGS].fec = 6;
					  else if ( IsDlgButtonChecked(hDlg, IDC_RADIO18)) Programm[MAXPROGS].fec = 7;
					  else if ( IsDlgButtonChecked(hDlg, IDC_RADIO19)) Programm[MAXPROGS].fec = 8;
					  
					  if ( IsDlgButtonChecked(hDlg, IDC_RADIO3)) Programm[MAXPROGS].TVType = 0;
					  else Programm[MAXPROGS].TVType = 3 ;
					  
					  if ( IsDlgButtonChecked(hDlg, IDC_RADIO26)) Programm[MAXPROGS].qam = 0;
					  else if ( IsDlgButtonChecked(hDlg, IDC_RADIO27)) Programm[MAXPROGS].qam = 1;
					  else if ( IsDlgButtonChecked(hDlg, IDC_RADIO28)) Programm[MAXPROGS].qam = 2;
					  else if ( IsDlgButtonChecked(hDlg, IDC_RADIO29)) Programm[MAXPROGS].qam = 3;
					  else if ( IsDlgButtonChecked(hDlg, IDC_RADIO30)) Programm[MAXPROGS].qam = 4;
					  
					  
					  Programm[MAXPROGS].srate=GetDlgItemInt(hDlg,IDC_EDIT6,NULL,FALSE);
					  Programm[MAXPROGS].Video_pid=GetDlgItemInt(hDlg,IDC_EDIT7,NULL,FALSE);
					  Programm[MAXPROGS].Audio_pid=GetDlgItemInt(hDlg,IDC_EDIT8,NULL,FALSE);
					  Programm[MAXPROGS].PMT_pid=GetDlgItemInt(hDlg,IDC_EDIT9,NULL,FALSE);
					  Programm[MAXPROGS].TeleText_pid=GetDlgItemInt(hDlg,IDC_EDIT10,NULL,FALSE);
					  Programm[MAXPROGS].PCR_pid=GetDlgItemInt(hDlg,IDC_EDIT11,NULL,FALSE);
					  Programm[MAXPROGS].ECM_PID=GetDlgItemInt(hDlg,IDC_EDIT12,NULL,FALSE);
					  Programm[MAXPROGS].SID_pid=GetDlgItemInt(hDlg,IDC_EDIT13,NULL,FALSE);
					  Programm[MAXPROGS].AC3_pid=GetDlgItemInt(hDlg,IDC_EDIT14,NULL,FALSE);
					  
				  }
				  
				  if (ValidModes(Programm[MAXPROGS].Typ) == TRUE  ) {
					  if (( Current_Mode == 0 ) && ( Programm[MAXPROGS].Typ == 'D' )) Set_Mode_Digital();
					  else if (( Current_Mode == 1 ) && ( Programm[MAXPROGS].Typ == 'A' )) Set_Mode_Analog();
                      
					  if (Programm[MAXPROGS].Typ == 'A' ) Tuner_SetFrequency(TunerType,MulDiv(Programm[MAXPROGS].freq*1000,16,1000000));
					  else {
						  CurrentProgramm=MAXPROGS;
						  Set_DVB_Channel(MAXPROGS);
						  InvalidateRect(hDlg,NULL,FALSE);
					  };
				  } else {
					  if ( Programm[MAXPROGS].Typ == 'D' ) MessageBox(hDlg,"Digitale Programme können Mangels Hardware nicht angezeigt werden","Multi-Dec",MB_ICONSTOP|MB_OK);
					  else if ( Programm[MAXPROGS].Typ == 'A' ) MessageBox(hDlg,"Analoge Programme können Mangels Hardware nicht angezeigt werden","Multi-Dec",MB_ICONSTOP|MB_OK);
				  }
				  
				  
			  }
			  
			  
			  if (LOWORD(wParam) == IDB_FPLUS ) {
				  GetDlgItemText(hDlg,IDC_EDIT2,zeile,sizeof(zeile));
				  ffreq=(float)atof(zeile);
				  ffreq=ffreq+(float)0.25;
				  freq=(unsigned	long)(ffreq*1000);
				  
				  if ( ValidModes(Programm[EditProgramm].Typ) == TRUE ) {
					  if ( Programm[EditProgramm].Typ == 'A' ) Tuner_SetFrequency(TunerType,MulDiv(freq*1000,16,1000000));
					  else SendMessage(hDlg,WM_COMMAND,IDTEST,0 );
				  }			 
				  
				  
				  sprintf(zeile,"%10.3f",(float)ffreq);
				  SetDlgItemText(hDlg,IDC_EDIT2,zeile);
			  };
			  
			  
			  if (LOWORD(wParam) == IDB_FMINUS ) {
				  GetDlgItemText(hDlg,IDC_EDIT2,zeile,sizeof(zeile));
				  ffreq=(float)atof(zeile);
				  ffreq=ffreq-(float)0.25;
				  freq=(unsigned	long)(ffreq*1000);
				  if ( ValidModes(Programm[EditProgramm].Typ) == TRUE ) {
					  if ( Programm[EditProgramm].Typ == 'A' ) Tuner_SetFrequency(TunerType,MulDiv(freq*1000,16,1000000));
					  else SendMessage(hDlg,WM_COMMAND,IDTEST,0 );
				  };
				  
				  sprintf(zeile,"%10.3f",(float)ffreq);
				  SetDlgItemText(hDlg,IDC_EDIT2,zeile);
			  };
			  
			  if ( LOWORD(wParam) == IDDEL) {
                  Programm[EditProgramm].freq=0;
                  Programm[EditProgramm].Name[0]=0x00;
				  i=EditProgramm;
                  while ( i < MAXPROGS-1 ) {
					  memcpy(&Programm[i],&Programm[i+1],sizeof(struct TProgramm));
					  i++;
				  };
                  Programm[i].freq = 0;
                  Programm[i].Name[0] = 0;
				  
				  Write_Programm_List();
				  Write_Nit_List();
				  
				  EndDialog(hDlg,TRUE);
				  
			  }
			  
			  
			  if ( LOWORD(wParam) == IDCANCEL) {
				  KillTimer(hDlg,0);
				  EndDialog(hDlg,TRUE);
			  }
			  
			  break;
    }
    return (FALSE);
	UNREFERENCED_PARAMETER(lParam);
}


BOOL APIENTRY TransponderSettingProc(
									 HWND hDlg,
									 UINT message,
									 UINT wParam,
									 LONG lParam)
{
	char zeile[140];
    unsigned long freq;
	float ffreq;
	
    PAINTSTRUCT wps;                   /* paint structure           */
    HDC hdc;
    HDC             hMemDC;
    HBITMAP         hOldBm;
	BITMAP bm;
	
    HBRUSH hBrush;
    HWND wndFill;
	
	
	
	char Zeile[128];
	
	int i,j,l,k; 
	RECT rc;
	int sig;
	int err;
	LOGFONT Mfont = {9,0,0,0,0,0,0,0,0,0,0,0,0,"Fixedsys"};   
    HFONT currFont;
	
	
	
	
    switch (message) {
	case WM_INITDIALOG:
		TransPonderDialog=hDlg;
		currFont = CreateFontIndirect(&Mfont);
		SendMessage(GetDlgItem(hDlg,IDC_LIST1),
			WM_SETFONT,
			(WPARAM)currFont,
			TRUE);
		SendMessage(GetDlgItem(hDlg,IDC_LIST1),LB_RESETCONTENT,0,0);
		Transponder.fec=Programm[CurrentProgramm].fec;
		Transponder.diseqc=Programm[CurrentProgramm].diseqc;
		Transponder.freq=Programm[CurrentProgramm].freq;
		Transponder.qam=Programm[CurrentProgramm].qam;
		Transponder.srate=Programm[CurrentProgramm].srate;
		Transponder.volt=Programm[CurrentProgramm].volt;
		Transponder.sync=dvb_front.sync;
		Transponder.ts_id=Programm[CurrentProgramm].tp_id;
		
		SetDlgItemInt(hDlg,IDC_EDIT20,Transponder.ts_id,FALSE);
		
		
		if ( Transponder.diseqc == 0 ) CheckDlgButton(hDlg, IDC_RADIO7, TRUE);
		else if ( Transponder.diseqc == 1 ) CheckDlgButton(hDlg, IDC_RADIO8, TRUE);
		else if ( Transponder.diseqc == 2 ) CheckDlgButton(hDlg, IDC_RADIO9, TRUE);
		else if ( Transponder.diseqc == 3 ) CheckDlgButton(hDlg, IDC_RADIO10, TRUE);
		
		if (UseDiseqC() == FALSE ) {
			EnableWindow(GetDlgItem(hDlg,IDC_RADIO7),FALSE);
			EnableWindow(GetDlgItem(hDlg,IDC_RADIO8),FALSE);
			EnableWindow(GetDlgItem(hDlg,IDC_RADIO9),FALSE);
			EnableWindow(GetDlgItem(hDlg,IDC_RADIO10),FALSE);
		} else {
			if ( LNB[1].Use == FALSE ) EnableWindow(GetDlgItem(hDlg,IDC_RADIO8),FALSE);
			if ( LNB[2].Use == FALSE ) EnableWindow(GetDlgItem(hDlg,IDC_RADIO9),FALSE);
			if ( LNB[3].Use == FALSE ) EnableWindow(GetDlgItem(hDlg,IDC_RADIO10),FALSE);
		};
		
		
		if ( Transponder.volt == 1 ) CheckDlgButton(hDlg, IDC_RADIO20, TRUE);
		else CheckDlgButton(hDlg, IDC_RADIO21, TRUE);
		
		
		if ( Transponder.fec == 0 ) CheckDlgButton(hDlg, IDC_RADIO11, TRUE);
		else if ( Transponder.fec == 1 ) CheckDlgButton(hDlg, IDC_RADIO12, TRUE);
		else if ( Transponder.fec == 2 ) CheckDlgButton(hDlg, IDC_RADIO13, TRUE);
		else if ( Transponder.fec == 3 ) CheckDlgButton(hDlg, IDC_RADIO14, TRUE);
		else if ( Transponder.fec == 4 ) CheckDlgButton(hDlg, IDC_RADIO15, TRUE);
		else if ( Transponder.fec == 5 ) CheckDlgButton(hDlg, IDC_RADIO16, TRUE);
		else if ( Transponder.fec == 6 ) CheckDlgButton(hDlg, IDC_RADIO17, TRUE);
		else if ( Transponder.fec == 7 ) CheckDlgButton(hDlg, IDC_RADIO18, TRUE);
		else if ( Transponder.fec == 8 ) CheckDlgButton(hDlg, IDC_RADIO19, TRUE);
		SetDlgItemInt(hDlg,IDC_EDIT6,Transponder.srate,FALSE);
		sprintf(zeile,"%10.3f",Transponder.freq/1000.f);
		SetDlgItemText(hDlg,IDC_EDIT2,zeile);
		if ((Transponder.sync&0x0f)==0x0f) EnableWindow(GetDlgItem(hDlg,ID_SCAN),TRUE);
		else EnableWindow(GetDlgItem(hDlg,ID_SCAN),FALSE);	
		SetTimer(hDlg,0,1000,NULL);
		
		return(TRUE);
		
	case WM_TIMER:
		rc.top=115;
		rc.left=15;
		rc.bottom=190;
		rc.right=378;
		if ( Current_Mode == 1 ) InvalidateRect(hDlg,&rc,FALSE);
		break;
		
		
	case WM_PAINT:
		
		Get_Signal_Infos((unsigned char *)&Transponder.sync,&sig,&err);
		hdc = BeginPaint(hDlg,&wps);
		hMemDC = CreateCompatibleDC(hdc);
		if (( Transponder.sync&0x0f) > 0 ) hOldBm = SelectObject(hMemDC, BirneGruen);
		else hOldBm = SelectObject(hMemDC, BirneRot );
		GetObject(BirneGruen, sizeof(BITMAP), (LPSTR)&bm);
		
		BitBlt(hdc, 53, 145, bm.bmWidth, bm.bmHeight,hMemDC, 0, 0, SRCCOPY);// Signal
		
		SelectObject(hMemDC,hOldBm);
		if (( Transponder.sync&0x0f) > 1 ) hOldBm = SelectObject(hMemDC, BirneGruen);
		else hOldBm = SelectObject(hMemDC, BirneRot );
		
		BitBlt(hdc, 53, 170, bm.bmWidth, bm.bmHeight,hMemDC, 0, 0, SRCCOPY); // Carrier
		SelectObject(hMemDC,hOldBm);
		
		
		if (( Transponder.sync&0x0f) > 3 ) hOldBm = SelectObject(hMemDC, BirneGruen);
		else hOldBm = SelectObject(hMemDC, BirneRot );
		
		BitBlt(hdc, 113, 145, bm.bmWidth, bm.bmHeight,hMemDC, 0, 0, SRCCOPY); // Viterbi
		SelectObject(hMemDC,hOldBm);
		
		if (( Transponder.sync&0x0f) > 7 ) hOldBm = SelectObject(hMemDC, BirneGruen);
		else hOldBm = SelectObject(hMemDC, BirneRot );
		
		BitBlt(hdc, 113, 170, bm.bmWidth, bm.bmHeight,hMemDC, 0, 0, SRCCOPY); // FSync
		SelectObject(hMemDC,hOldBm);
		DeleteDC(hMemDC);
		DeleteDC(hdc);
		for ( i = 0; i < 20 ; i++ ) {
            wndFill = GetDlgItem (hDlg, S1+i );
			GetClientRect(wndFill, &rc);
            hdc = GetDC(wndFill);
			rc.bottom-=1;
			rc.top+=1;
			rc.right-=1;
			rc.left+=1;
			
			if ( sig >= i * 12 ) {
				hBrush=CreateSolidBrush(RGB(255-(12*i),12*i,0));
			} else {
				hBrush=CreateSolidBrush(RGB(0,0,0));
			};
			FillRect (hdc, &rc, hBrush);
			ReleaseDC (wndFill, hdc);
			DeleteObject(hBrush);
			
            wndFill = GetDlgItem (hDlg, E1+i );
			GetClientRect(wndFill, &rc);
            hdc = GetDC(wndFill);
			rc.bottom-=1;
			rc.top+=1;
			rc.right-=1;
			rc.left+=1;
			
			if ( err >= i * 12 ) {
				hBrush=CreateSolidBrush(RGB(255-(12*i),12*i,0));
			} else {
				hBrush=CreateSolidBrush(RGB(0,0,0));
			};
			FillRect (hdc, &rc, hBrush);
			ReleaseDC (wndFill, hdc);
			DeleteObject(hBrush);
		}
		
		
		EndPaint(hDlg,&wps);
		return(FALSE);
		
        
	case WM_COMMAND:
		
		if ((HWND)lParam == GetDlgItem(hDlg,IDC_CHECK1)) {
			if ( IsDlgButtonChecked(hDlg, IDC_CHECK1) == FALSE) {
				EnableWindow(GetDlgItem(hDlg,IDC_RADIO5),FALSE);
				EnableWindow(GetDlgItem(hDlg,IDC_RADIO6),FALSE);
				EnableWindow(GetDlgItem(hDlg,IDC_RADIO24),FALSE);
				EnableWindow(GetDlgItem(hDlg,IDC_RADIO25),FALSE);
			} else {
				EnableWindow(GetDlgItem(hDlg,IDC_RADIO5),TRUE);
				EnableWindow(GetDlgItem(hDlg,IDC_RADIO6),TRUE);
				EnableWindow(GetDlgItem(hDlg,IDC_RADIO24),TRUE);
				EnableWindow(GetDlgItem(hDlg,IDC_RADIO25),TRUE);
			};		
		};
		
		
		
		if (LOWORD(wParam) == ID_UPDATEALL ) {
			j=0;
			for ( i=0; i < NEUSIZE ; i++ ) {
				if ( ProgrammNeu[i].SID_pid == 0x0000 ) {
					if ( Get_Nit((unsigned char)Transponder.diseqc) == TRUE ) {
						for ( j=0; j < NEUNIT; j++ ) {
							if ( NITNeu[j].ts_id != 0 ) {
								if ( Update_Nit_List(j) == TRUE ) sprintf(zeile,"Update Nit TS-ID %d",NITNeu[j].ts_id);
								else sprintf(zeile,"Nit TS-ID %d eingefügt",NITNeu[j].ts_id);
								SetDlgItemText(hDlg,IDC_TEXT,zeile);
							}
						}
					}
				}
				if (( ProgrammNeu[i].SID_pid != 0x0000 ) && ( ProgrammNeu[i].SID_pid != 0x1fff )) {
					sprintf(zeile,"Lade PMT für %s",ProgrammNeu[i].Name);
					SetDlgItemText(hDlg,IDC_TEXT,zeile);
					(void)Get_PMT(i,ProgrammNeu[i].PMT_pid);
					Insert_CAT(i);
					if ( Update_Programm_List(i) == TRUE ) sprintf(zeile,"Update Programm %s",ProgrammNeu[i].Name);
					else sprintf(zeile,"Programm %s eingefügt",ProgrammNeu[i].Name);
					SetDlgItemText(hDlg,IDC_TEXT,zeile);
				}
			}
			Write_Programm_List();
			Write_Nit_List();
		}
		
		if (LOWORD(wParam) == ID_UPDATEONE ) {
			i=SendMessage(GetDlgItem(hDlg,IDC_LIST1),LB_GETCURSEL,0,0);
			if (( i >= 0 ) && ( i < NEUSIZE )) {
				
				if ( ProgrammNeu[i].SID_pid == 0x0000 ) {
					if ( Get_Nit((unsigned char)Transponder.diseqc) == TRUE ) {
						for ( j=0; j < NEUNIT; j++ ) {
							if ( NITNeu[j].ts_id != 0 ) {
								if ( Update_Nit_List(j) == TRUE ) sprintf(zeile,"Update Nit TS-ID %d",NITNeu[j].ts_id);
								else sprintf(zeile,"Nit TS-ID %d eingefügt",NITNeu[j].ts_id);
								SetDlgItemText(hDlg,IDC_TEXT,zeile);
							}
						}
					}
				}
				
				if (( ProgrammNeu[i].SID_pid != 0x0000 ) && ( ProgrammNeu[i].SID_pid != 0x1fff )) {
					sprintf(zeile,"Lade PMT für %s",ProgrammNeu[i].Name);
					SetDlgItemText(hDlg,IDC_TEXT,zeile);
					(void)Get_PMT(i,ProgrammNeu[i].PMT_pid);
					Insert_CAT(i);
					if ( Update_Programm_List(i) == TRUE ) sprintf(zeile,"Update Programm %s",ProgrammNeu[i].Name);
					else sprintf(zeile,"Programm %s eingefügt",ProgrammNeu[i].Name);
					SetDlgItemText(hDlg,IDC_TEXT,zeile);
				};
			};
			Write_Programm_List();  
			Write_Nit_List();
		};
        
		
		
		
		if (LOWORD(wParam) == IDTEST ) {
			SendMessage(GetDlgItem(hDlg,IDC_LIST1),LB_RESETCONTENT,0,0);
			EnableWindow(GetDlgItem(hDlg,ID_UPDATEALL),FALSE);
			EnableWindow(GetDlgItem(hDlg,ID_UPDATEONE),FALSE);
			
			GetDlgItemText(hDlg,IDC_EDIT2,zeile,sizeof(zeile));
			ffreq=(float)atof(zeile);
			freq=(unsigned	long)(ffreq*1000);
			Transponder.freq=freq;
			
			
			if ( IsDlgButtonChecked(hDlg, IDC_RADIO7)) Transponder.diseqc = 0;
			else if ( IsDlgButtonChecked(hDlg, IDC_RADIO8)) Transponder.diseqc = 1;
			else if ( IsDlgButtonChecked(hDlg, IDC_RADIO9)) Transponder.diseqc = 2;
			else if ( IsDlgButtonChecked(hDlg, IDC_RADIO10)) Transponder.diseqc = 3;
			Transponder.volt= IsDlgButtonChecked(hDlg, IDC_RADIO20);
			
			
			if ( IsDlgButtonChecked(hDlg, IDC_RADIO11)) Transponder.fec = 0;
			else if ( IsDlgButtonChecked(hDlg, IDC_RADIO12)) Transponder.fec = 1;
			else if ( IsDlgButtonChecked(hDlg, IDC_RADIO13)) Transponder.fec = 2;
			else if ( IsDlgButtonChecked(hDlg, IDC_RADIO14)) Transponder.fec = 3;
			else if ( IsDlgButtonChecked(hDlg, IDC_RADIO15)) Transponder.fec = 4;
			else if ( IsDlgButtonChecked(hDlg, IDC_RADIO16)) Transponder.fec = 5;
			else if ( IsDlgButtonChecked(hDlg, IDC_RADIO17)) Transponder.fec = 6;
			else if ( IsDlgButtonChecked(hDlg, IDC_RADIO18)) Transponder.fec = 7;
			else if ( IsDlgButtonChecked(hDlg, IDC_RADIO19)) Transponder.fec = 8;
			Transponder.srate=GetDlgItemInt(hDlg,IDC_EDIT6,NULL,FALSE);
			if ( Transponder.srate != 0 ) {
				(void)Set_Transponder(&Transponder);
			} else {
				Set_Transponder(&Transponder);
				for ( i = 0 ; i < 16 ; i++ ) {
					if ( SRates[i] != 0 ) SetDlgItemInt(hDlg,IDC_EDIT6,SRates[i],FALSE);
					if ( Set_Transponder_SRate(&Transponder,SRates[i]) == TRUE ) break;
				}
			}
			if ((Transponder.sync&0x0f)==0x0f) EnableWindow(GetDlgItem(hDlg,ID_SCAN),TRUE);
			else EnableWindow(GetDlgItem(hDlg,ID_SCAN),FALSE);	
			InvalidateRect(hDlg,NULL,FALSE);
			break;
			
		}
		
		if ( LOWORD(wParam) == ID_SCAN) {
			SendMessage(GetDlgItem(hDlg,IDC_LIST1),LB_RESETCONTENT,0,0);
			EnableWindow(GetDlgItem(hDlg,ID_UPDATEALL),FALSE);
			EnableWindow(GetDlgItem(hDlg,ID_UPDATEONE),FALSE);
			Scan_TP(GetDlgItem(hDlg,IDC_TEXT));
			Transponder.ts_id=ProgrammNeu[0].tp_id;
			SetDlgItemInt(hDlg,IDC_EDIT20,Transponder.ts_id,FALSE);
			l=1;
			k=0;
			for ( i=0; i < NEUSIZE ; i++ ) {
				if ( ProgrammNeu[i].SID_pid != 0x1fff ) {
					k++;
					if ( ProgrammNeu[i].Name[0] == 0 ) {
						j=Find_Programm(ProgrammNeu[i].tp_id,ProgrammNeu[i].SID_pid);
						if ( j < 0 ) {
							sprintf(Zeile,"%d-%d",Transponder.freq/1000,l);
							l++;
							strcpy(ProgrammNeu[i].Name,Zeile);
						} else {
							strcpy(ProgrammNeu[i].Name,Programm[j].Name);
							strcpy(ProgrammNeu[i].Anbieter,Programm[j].Anbieter);
							strcpy(ProgrammNeu[i].Land,Programm[j].Land);
						}
					}
					if ( ProgrammNeu[i].PMT_pid != 0x1fff ) sprintf(Zeile,"%- 25s  %04x %04x",ProgrammNeu[i].Name,ProgrammNeu[i].SID_pid,ProgrammNeu[i].PMT_pid);
					else sprintf(Zeile,"%- 25s  %04x Prog. Offline",ProgrammNeu[i].Name,ProgrammNeu[i].SID_pid);
					SendMessage(GetDlgItem(hDlg,IDC_LIST1),LB_INSERTSTRING,i,(LPARAM)Zeile);
					j++;
				}
			}
			
			if ( k > 0 ) EnableWindow(GetDlgItem(hDlg,ID_UPDATEALL),TRUE);
			
			break;
		};
		
		if ( (HWND)lParam==GetDlgItem(hDlg,IDC_LIST1) ){
			if (HIWORD(wParam)==LBN_SELCHANGE){
				i=SendMessage(GetDlgItem(hDlg,IDC_LIST1),LB_GETCURSEL,0,0);
				if (( i >= 0 ) && ( i < NEUSIZE )) {
					EnableWindow(GetDlgItem(hDlg,ID_UPDATEONE),TRUE);
					if ( ProgrammNeu[i].SID_pid == 0x0000 ) {
						SetDlgItemText(hDlg,IDC_TEXT,"Lade NIT");
						if ( Get_Nit((unsigned char )Transponder.diseqc) == TRUE ) {
							SetDlgItemText(hDlg,IDC_TEXT,"");
							DialogBox(hInst,"NITLISTE",hWnd,NitListeNeu);
							for ( j=0; j < NEUNIT; j++ ) {
								if ( NITNeu[j].ts_id != 0 ) Update_Nit_List(j);
							}
							Write_Nit_List();
							
							// Nit anzeigen
						} else {
							MessageBox(hDlg,"NIT-Daten nicht geladen","Multi-Dec",MB_ICONSTOP|MB_OK);
						};
					} else if ( ProgrammNeu[i].PMT_pid != 0x1fff ) {
						if ( Get_PMT(i,ProgrammNeu[i].PMT_pid) == TRUE ) {
							Insert_CAT(i);
							memcpy(&Programm[MAXPROGS],&ProgrammNeu[i],sizeof(struct TProgramm));
							CurrentProgramm=MAXPROGS;
							Set_DVB_Channel(MAXPROGS);
						} else {
							MessageBox(hDlg,"PMT-Daten nicht geladen","Multi-Dec",MB_ICONSTOP|MB_OK);
						};
					}
				};
			};
			break;
		};
		
		
		
		if ( LOWORD(wParam) == IDOK) {
			TransPonderDialog=NULL;
			KillTimer(hDlg,0);
			EndDialog(hDlg,TRUE);
		}
		
		if ( LOWORD(wParam) == IDCANCEL) {
			TransPonderDialog=NULL;
			KillTimer(hDlg,0);
			EndDialog(hDlg,TRUE);
		}
		
		break;
    }
    return (FALSE);
	UNREFERENCED_PARAMETER(lParam);
}


BOOL APIENTRY TransponderSettingProcCable(
										  HWND hDlg,
										  UINT message,
										  UINT wParam,
										  LONG lParam)
{
	char zeile[140];
    unsigned long freq;
	float ffreq;
	
    PAINTSTRUCT wps;                   /* paint structure           */
    HDC hdc;
    HDC             hMemDC;
    HBITMAP         hOldBm;
	BITMAP bm;
	
    HBRUSH hBrush;
    HWND wndFill;
	
	
	
	char Zeile[128];
	
	int i,j,l,k; 
	RECT rc;
	int sig;
	int err;
	LOGFONT Mfont = {9,0,0,0,0,0,0,0,0,0,0,0,0,"Fixedsys"};   
    HFONT currFont;
	
	
	
	
    switch (message) {
	case WM_INITDIALOG:
		TransPonderDialog=hDlg;
		currFont = CreateFontIndirect(&Mfont);
		SendMessage(GetDlgItem(hDlg,IDC_LIST1),
			WM_SETFONT,
			(WPARAM)currFont,
			TRUE);
		SendMessage(GetDlgItem(hDlg,IDC_LIST1),LB_RESETCONTENT,0,0);
		Transponder.fec=Programm[CurrentProgramm].fec;
		Transponder.diseqc=Programm[CurrentProgramm].diseqc;
		Transponder.freq=Programm[CurrentProgramm].freq;
		Transponder.qam=Programm[CurrentProgramm].qam;
		Transponder.srate=Programm[CurrentProgramm].srate;
		Transponder.volt=Programm[CurrentProgramm].volt;
		Transponder.sync=dvb_front.sync;
		Transponder.ts_id=Programm[CurrentProgramm].tp_id;
		
		SetDlgItemInt(hDlg,IDC_EDIT20,Transponder.ts_id,FALSE);
		
		if ( Transponder.qam== 0 ) CheckDlgButton(hDlg, IDC_RADIO11, TRUE);
		else if ( Transponder.qam == 1 ) CheckDlgButton(hDlg, IDC_RADIO12, TRUE);
		else if ( Transponder.qam == 2 ) CheckDlgButton(hDlg, IDC_RADIO13, TRUE);
		else if ( Transponder.qam == 3 ) CheckDlgButton(hDlg, IDC_RADIO14, TRUE);
		else if ( Transponder.qam == 4 ) CheckDlgButton(hDlg, IDC_RADIO15, TRUE);
		
		SetDlgItemInt(hDlg,IDC_EDIT6,Transponder.srate,FALSE);
		sprintf(zeile,"%10.3f",Transponder.freq/1000.f);
		SetDlgItemText(hDlg,IDC_EDIT2,zeile);
		SetTimer(hDlg,0,1000,NULL);
		
		return(TRUE);
		
	case WM_TIMER:
		rc.top=115;
		rc.left=15;
		rc.bottom=190;
		rc.right=378;
		if ( Current_Mode == 1 ) InvalidateRect(hDlg,&rc,FALSE);
		if ((Transponder.sync&0x0f)==0x0f) EnableWindow(GetDlgItem(hDlg,ID_SCAN),TRUE);
		else EnableWindow(GetDlgItem(hDlg,ID_SCAN),FALSE);	
		break;
		
		
	case WM_PAINT:
		
		Get_Signal_Infos((unsigned char *)&Transponder.sync,&sig,&err);
		hdc = BeginPaint(hDlg,&wps);
		hMemDC = CreateCompatibleDC(hdc);
		if (( Transponder.sync&0x0f) > 0 ) hOldBm = SelectObject(hMemDC, BirneGruen);
		else hOldBm = SelectObject(hMemDC, BirneRot );
		GetObject(BirneGruen, sizeof(BITMAP), (LPSTR)&bm);
		
		BitBlt(hdc, 53, 105, bm.bmWidth, bm.bmHeight,hMemDC, 0, 0, SRCCOPY);// Signal
		
		SelectObject(hMemDC,hOldBm);
		if (( Transponder.sync&0x0f) > 1 ) hOldBm = SelectObject(hMemDC, BirneGruen);
		else hOldBm = SelectObject(hMemDC, BirneRot );
		
		BitBlt(hdc, 53, 130, bm.bmWidth, bm.bmHeight,hMemDC, 0, 0, SRCCOPY); // Carrier
		SelectObject(hMemDC,hOldBm);
		
		
		if (( Transponder.sync&0x0f) > 3 ) hOldBm = SelectObject(hMemDC, BirneGruen);
		else hOldBm = SelectObject(hMemDC, BirneRot );
		
		BitBlt(hdc, 113, 105, bm.bmWidth, bm.bmHeight,hMemDC, 0, 0, SRCCOPY); // Viterbi
		SelectObject(hMemDC,hOldBm);
		
		if (( Transponder.sync&0x0f) > 7 ) hOldBm = SelectObject(hMemDC, BirneGruen);
		else hOldBm = SelectObject(hMemDC, BirneRot );
		
		BitBlt(hdc, 113, 130, bm.bmWidth, bm.bmHeight,hMemDC, 0, 0, SRCCOPY); // FSync
		SelectObject(hMemDC,hOldBm);
		DeleteDC(hMemDC);
		DeleteDC(hdc);
		for ( i = 0; i < 20 ; i++ ) {
            wndFill = GetDlgItem (hDlg, E1+i );
			GetClientRect(wndFill, &rc);
            hdc = GetDC(wndFill);
			rc.bottom-=1;
			rc.top+=1;
			rc.right-=1;
			rc.left+=1;
			
			if ( err >= i * 12 ) {
				hBrush=CreateSolidBrush(RGB(255-(12*i),12*i,0));
			} else {
				hBrush=CreateSolidBrush(RGB(0,0,0));
			};
			FillRect (hdc, &rc, hBrush);
			ReleaseDC (wndFill, hdc);
			DeleteObject(hBrush);
		}
		
		
		EndPaint(hDlg,&wps);
		return(FALSE);
		
        
	case WM_COMMAND:
		
		
		if (LOWORD(wParam) == ID_UPDATEALL ) {
			j=0;
			for ( i=0; i < NEUSIZE ; i++ ) {
				if ( ProgrammNeu[i].SID_pid == 0x0000 ) {
					if ( Get_Nit((unsigned char)Transponder.diseqc) == TRUE ) {
						for ( j=0; j < NEUNIT; j++ ) {
							if ( NITNeu[j].ts_id != 0 ) {
								if ( Update_Nit_List(j) == TRUE ) sprintf(zeile,"Update Nit TS-ID %d",NITNeu[j].ts_id);
								else sprintf(zeile,"Nit TS-ID %d eingefügt",NITNeu[j].ts_id);
								SetDlgItemText(hDlg,IDC_TEXT,zeile);
							}
						}
						Write_Nit_List();
					}
				}
				if (( ProgrammNeu[i].SID_pid != 0x0000 ) && ( ProgrammNeu[i].SID_pid != 0x1fff )) {
					sprintf(zeile,"Lade PMT für %s",ProgrammNeu[i].Name);
					SetDlgItemText(hDlg,IDC_TEXT,zeile);
					(void)Get_PMT(i,ProgrammNeu[i].PMT_pid);
					Insert_CAT(i);
					if ( Update_Programm_List(i) == TRUE ) sprintf(zeile,"Update Programm %s",ProgrammNeu[i].Name);
					else sprintf(zeile,"Programm %s eingefügt",ProgrammNeu[i].Name);
					SetDlgItemText(hDlg,IDC_TEXT,zeile);
				}
			}
			Write_Programm_List();
			Write_Nit_List();
		}
		
		if (LOWORD(wParam) == ID_UPDATEONE ) {
			i=SendMessage(GetDlgItem(hDlg,IDC_LIST1),LB_GETCURSEL,0,0);
			if (( i >= 0 ) && ( i < NEUSIZE )) {
				
				if ( ProgrammNeu[i].SID_pid == 0x0000 ) {
					if ( Get_Nit((unsigned char)Transponder.diseqc) == TRUE ) {
						for ( j=0; j < NEUNIT; j++ ) {
							if ( NITNeu[j].ts_id != 0 ) {
								if ( Update_Nit_List(j) == TRUE ) sprintf(zeile,"Update Nit TS-ID %d",NITNeu[j].ts_id);
								else sprintf(zeile,"Nit TS-ID %d eingefügt",NITNeu[j].ts_id);
								SetDlgItemText(hDlg,IDC_TEXT,zeile);
							}
						}
						Write_Nit_List();
					}
				}
				
				if (( ProgrammNeu[i].SID_pid != 0x0000 ) && ( ProgrammNeu[i].SID_pid != 0x1fff )) {
					sprintf(zeile,"Lade PMT für %s",ProgrammNeu[i].Name);
					SetDlgItemText(hDlg,IDC_TEXT,zeile);
					(void)Get_PMT(i,ProgrammNeu[i].PMT_pid);
					Insert_CAT(i);
					if ( Update_Programm_List(i) == TRUE ) sprintf(zeile,"Update Programm %s",ProgrammNeu[i].Name);
					else sprintf(zeile,"Programm %s eingefügt",ProgrammNeu[i].Name);
					SetDlgItemText(hDlg,IDC_TEXT,zeile);
				};
			};
			Write_Programm_List();  
			Write_Nit_List();
		};
        
		
		if (LOWORD(wParam) == IDTEST ) {
			SendMessage(GetDlgItem(hDlg,IDC_LIST1),LB_RESETCONTENT,0,0);
			EnableWindow(GetDlgItem(hDlg,ID_UPDATEALL),FALSE);
			EnableWindow(GetDlgItem(hDlg,ID_UPDATEONE),FALSE);
			
			GetDlgItemText(hDlg,IDC_EDIT2,zeile,sizeof(zeile));
			ffreq=(float)atof(zeile);
			freq=(unsigned	long)(ffreq*1000);
			Transponder.freq=freq;
			
			
			if ( IsDlgButtonChecked(hDlg, IDC_RADIO11)) Transponder.qam = 0;
			else if ( IsDlgButtonChecked(hDlg, IDC_RADIO12)) Transponder.qam = 1;
			else if ( IsDlgButtonChecked(hDlg, IDC_RADIO13)) Transponder.qam = 2;
			else if ( IsDlgButtonChecked(hDlg, IDC_RADIO14)) Transponder.qam = 3;
			else if ( IsDlgButtonChecked(hDlg, IDC_RADIO15)) Transponder.qam = 4;
			
			Transponder.srate=GetDlgItemInt(hDlg,IDC_EDIT6,NULL,FALSE);
			
			if ( IsDlgButtonChecked(hDlg, IDC_RADIO18)) {
				
				for ( j = 0; j < 5 ; j++ ) {
                    Transponder.qam=j;
					if ( Transponder.qam== 0 ) CheckDlgButton(hDlg, IDC_RADIO11, TRUE);
					else if ( Transponder.qam == 1 ) CheckDlgButton(hDlg, IDC_RADIO12, TRUE);
					else if ( Transponder.qam == 2 ) CheckDlgButton(hDlg, IDC_RADIO13, TRUE);
					else if ( Transponder.qam == 3 ) CheckDlgButton(hDlg, IDC_RADIO14, TRUE);
					else if ( Transponder.qam == 4 ) CheckDlgButton(hDlg, IDC_RADIO15, TRUE);
					if ( Transponder.srate != 0 ) {
						(void)Set_Transponder(&Transponder);
					} else {
						Set_Transponder(&Transponder);
						for ( i = 0 ; i < 16 ; i++ ) {
							if ( SRates[i] != 0 ) SetDlgItemInt(hDlg,IDC_EDIT6,SRates[i],FALSE);
							if ( Set_Transponder_SRate(&Transponder,SRates[i]) == TRUE ) break;
						} // 16 mögliche Symbolraten
					}  // Transponder Symbolrate automatisch 
					
					
					if ((Transponder.sync&0x0f)==0x0f) {
						EnableWindow(GetDlgItem(hDlg,ID_SCAN),TRUE);
						InvalidateRect(hDlg,NULL,FALSE);
						return(TRUE);
					};
				};  // 5 Qam
				
			} else {  // Qam nicht automatisch 
				
				if ( Transponder.srate != 0 ) {
					(void)Set_Transponder(&Transponder);
				} else {
					Set_Transponder(&Transponder);
					for ( i = 0 ; i < 16 ; i++ ) {
						if ( SRates[i] != 0 ) SetDlgItemInt(hDlg,IDC_EDIT6,SRates[i],FALSE);
						if ( Set_Transponder_SRate(&Transponder,SRates[i]) == TRUE ) break;
						
					}
				}
				
				if ((Transponder.sync&0x0f)==0x0f) EnableWindow(GetDlgItem(hDlg,ID_SCAN),TRUE);
				else EnableWindow(GetDlgItem(hDlg,ID_SCAN),FALSE);	
			};						
			InvalidateRect(hDlg,NULL,FALSE);
			return(TRUE);
		};
		
		if ( LOWORD(wParam) == ID_SCAN) {
			SendMessage(GetDlgItem(hDlg,IDC_LIST1),LB_RESETCONTENT,0,0);
			EnableWindow(GetDlgItem(hDlg,ID_UPDATEALL),FALSE);
			EnableWindow(GetDlgItem(hDlg,ID_UPDATEONE),FALSE);
			Scan_TP(GetDlgItem(hDlg,IDC_TEXT));
			Transponder.ts_id=ProgrammNeu[0].tp_id;
			SetDlgItemInt(hDlg,IDC_EDIT20,Transponder.ts_id,FALSE);
			l=1;
			k=0;
			for ( i=0; i < NEUSIZE ; i++ ) {
				if ( ProgrammNeu[i].SID_pid != 0x1fff ) {
					k++;
					if ( ProgrammNeu[i].Name[0] == 0 ) {
						j=Find_Programm(ProgrammNeu[i].tp_id,ProgrammNeu[i].SID_pid);
						if ( j < 0 ) {
							sprintf(Zeile,"%d-%d",Transponder.freq/1000,l);
							l++;
							strcpy(ProgrammNeu[i].Name,Zeile);
						} else {
							strcpy(ProgrammNeu[i].Name,Programm[j].Name);
							strcpy(ProgrammNeu[i].Anbieter,Programm[j].Anbieter);
							strcpy(ProgrammNeu[i].Land,Programm[j].Land);
						}
					}
					if ( ProgrammNeu[i].PMT_pid != 0x1fff ) sprintf(Zeile,"%- 25s  %04x %04x",ProgrammNeu[i].Name,ProgrammNeu[i].SID_pid,ProgrammNeu[i].PMT_pid);
					else sprintf(Zeile,"%- 25s  %04x Prog. Offline",ProgrammNeu[i].Name,ProgrammNeu[i].SID_pid);
					SendMessage(GetDlgItem(hDlg,IDC_LIST1),LB_INSERTSTRING,i,(LPARAM)Zeile);
					j++;
				}
			}
			
			if ( k > 0 ) EnableWindow(GetDlgItem(hDlg,ID_UPDATEALL),TRUE);
			
			break;
		};
		
		if ( (HWND)lParam==GetDlgItem(hDlg,IDC_LIST1) ){
			if (HIWORD(wParam)==LBN_SELCHANGE){
				i=SendMessage(GetDlgItem(hDlg,IDC_LIST1),LB_GETCURSEL,0,0);
				if (( i >= 0 ) && ( i < NEUSIZE )) {
					EnableWindow(GetDlgItem(hDlg,ID_UPDATEONE),TRUE);
					if ( ProgrammNeu[i].SID_pid == 0x0000 ) {
						SetDlgItemText(hDlg,IDC_TEXT,"Lade NIT");
						if ( Get_Nit((unsigned char )Transponder.diseqc) == TRUE ) {
							SetDlgItemText(hDlg,IDC_TEXT,"");
							DialogBox(hInst,"NITLISTE",hWnd,NitListeNeu);
							for ( j=0; j < NEUNIT; j++ ) {
								if ( NITNeu[j].ts_id != 0 ) Update_Nit_List(j);
							}
							Write_Nit_List();
							// Nit anzeigen
						} else {
							MessageBox(hDlg,"NIT-Daten nicht geladen","Multi-Dec",MB_ICONSTOP|MB_OK);
						};
					} else if ( ProgrammNeu[i].PMT_pid != 0x1fff ) {
						if ( Get_PMT(i,ProgrammNeu[i].PMT_pid) == TRUE ) {
							Insert_CAT(i);
							memcpy(&Programm[MAXPROGS],&ProgrammNeu[i],sizeof(struct TProgramm));
							CurrentProgramm=MAXPROGS;
							Set_DVB_Channel(MAXPROGS);
						} else {
							MessageBox(hDlg,"PMT-Daten nicht geladen","Multi-Dec",MB_ICONSTOP|MB_OK);
						};
					}
				};
			};
			break;
		};
		
		
		
		if ( LOWORD(wParam) == IDOK) {
			TransPonderDialog=NULL;
			KillTimer(hDlg,0);
			EndDialog(hDlg,TRUE);
		}
		
		if ( LOWORD(wParam) == IDCANCEL) {
			TransPonderDialog=NULL;
			KillTimer(hDlg,0);
			EndDialog(hDlg,TRUE);
		}
		
		break;
    }
    return (FALSE);
	UNREFERENCED_PARAMETER(lParam);
}



void  Fine_Tune( int von , int bis , int step )
{
	
	unsigned int SaveFreq;
	unsigned int MaxSig;
	int freqstep;
	unsigned int BestFreq;
	char zeile[128];
	
	if ((Transponder.sync&0x0f)!=0x0f) return;
	
	Get_Signal_Infos((unsigned char *)&Transponder.sync,&Transponder.sig,&Transponder.err);
	MaxSig=Transponder.sig;
	SaveFreq=Transponder.freq;
	BestFreq=Transponder.freq;
	
	for (freqstep = von;  freqstep <= bis; freqstep += step) {
		
		Transponder.freq=SaveFreq + freqstep;
		Set_Transponder_Freq(&Transponder);
		sprintf(zeile,"%10.3f",Transponder.freq/1000.f);
		if ( ScanWndHandle != NULL ) SetWindowText(GetDlgItem(ScanWndHandle,IDC_EDIT2),zeile);
		DVB_Reset();
		Get_Signal_Infos((unsigned char *)&Transponder.sync,&Transponder.sig,&Transponder.err);
		if ( ScanWndHandle != NULL ) UpdateWindow(ScanWndHandle);
		if (Transponder.sig >= MaxSig && ((Transponder.sync&0x0f)==0x0f)) {
			MaxSig=Transponder.sig;
			BestFreq=Transponder.freq;
		}
		
		
	}
	
	Transponder.freq=BestFreq;
	Set_Transponder_Freq(&Transponder);
	DVB_Reset();
	Get_Signal_Infos((unsigned char *)&Transponder.sync,&Transponder.sig,&Transponder.err);
	if ( ScanWndHandle != NULL ) UpdateWindow(ScanWndHandle);
}

void Load_And_Update_TP( HWND DisplayWnd)
{
	char Zeile[128];
    int l,j,k;
	
	if ((Transponder.sync&0x0f)!=0x0f) return;
	Transponder.fec=8;
	
	if (DisplayWnd != NULL ) {
		SetWindowText(GetDlgItem(DisplayWnd,IDC_EDIT7),"");
		SetWindowText(GetDlgItem(DisplayWnd,IDC_EDIT5),"");
		SetWindowText(GetDlgItem(DisplayWnd,IDC_EDIT4),"");
		SetWindowText(GetDlgItem(DisplayWnd,IDC_EDIT13),"");
		SetWindowText(GetDlgItem(DisplayWnd,IDC_EDIT12),"");
		SetWindowText(GetDlgItem(DisplayWnd,IDC_EDIT14),"");
		SetWindowText(GetDlgItem(DisplayWnd,IDC_EDIT11),"");
		SetWindowText(GetDlgItem(DisplayWnd,IDC_EDIT15),"");
		SetWindowText(GetDlgItem(DisplayWnd,IDC_EDIT16),"");
		SetWindowText(GetDlgItem(DisplayWnd,IDC_EDIT18),"");
		Scan_TP(GetDlgItem(DisplayWnd,IDC_TEXT20));
		if ( Has_Pat ) {
			sprintf(Zeile,"%d",PAT.ts_id);
			SetWindowText(GetDlgItem(DisplayWnd,IDC_EDIT7),Zeile);
			sprintf(Zeile,"%d",PAT.version);
			SetWindowText(GetDlgItem(DisplayWnd,IDC_EDIT5),Zeile);
			sprintf(Zeile,"%d",PAT.ProgAnzahl);
			SetWindowText(GetDlgItem(DisplayWnd,IDC_EDIT4),Zeile);
		};
		if ( Has_Sdt ) {
			sprintf(Zeile,"%d",SDT.ts_id);
			SetWindowText(GetDlgItem(DisplayWnd,IDC_EDIT13),Zeile);
			sprintf(Zeile,"%d",SDT.version);
			SetWindowText(GetDlgItem(DisplayWnd,IDC_EDIT12),Zeile);
			sprintf(Zeile,"%d",SDT.orig_nw_id);
			SetWindowText(GetDlgItem(DisplayWnd,IDC_EDIT14),Zeile);
			sprintf(Zeile,"%d",SDT.ServiceAnzahl);
			SetWindowText(GetDlgItem(DisplayWnd,IDC_EDIT11),Zeile);
		};
		
	} else Scan_TP(NULL);
	
	
	l=1;
	
	for ( i=0; i < NEUSIZE ; i++ ) {
		if ( ContinueScan == FALSE ) return;
		ProgrammNeu[i].srate=Transponder.srate;
		ProgrammNeu[i].fec=Transponder.fec;
		ProgrammNeu[i].qam=Transponder.qam;
		if (( ProgrammNeu[i].SID_pid != 0x1fff ) && ( ProgrammNeu[i].SID_pid != 0x0000 )) {
			if ( ProgrammNeu[i].Name[0] == 0 ) {
				j=Find_Programm(ProgrammNeu[i].tp_id,ProgrammNeu[i].SID_pid);
				if ( j < 0 ) {
					sprintf(Zeile,"%d-%d",Transponder.freq/1000,l);
					l++;
					strcpy(ProgrammNeu[i].Name,Zeile);
				} else {
					strcpy(ProgrammNeu[i].Name,Programm[j].Name);
					strcpy(ProgrammNeu[i].Anbieter,Programm[j].Anbieter);
					strcpy(ProgrammNeu[i].Land,Programm[j].Land);
				}
			}
		}
		
		
		if ( ProgrammNeu[i].SID_pid == 0x0000 ) {
			if ( DisplayWnd != NULL ) SetWindowText(GetDlgItem(DisplayWnd,IDC_TEXT20),"Lade NIT");
			if ( Get_Nit((unsigned char)Transponder.diseqc) == TRUE ) {
				if ( DisplayWnd != NULL ) SetWindowText(GetDlgItem(DisplayWnd,IDC_TEXT20),"NIT geladen");
				for ( j=0; j < NEUNIT; j++ ) {
					if ( NITNeu[j].ts_id != 0 ) {
						k=Update_Nit_List(j);
					}
				}
			} else {
				if ( DisplayWnd != NULL ) SetWindowText(GetDlgItem(DisplayWnd,IDC_TEXT20),"NIT TimeOut");
			}
		}
		if (( ProgrammNeu[i].SID_pid != 0x0000 ) && ( ProgrammNeu[i].SID_pid != 0x1fff )) {
			if ( ProgrammNeu[i].PMT_pid != 0x1fff ) {
				if ( DisplayWnd != NULL ) SetWindowText(GetDlgItem(DisplayWnd,IDC_TEXT20),"Lade PMT");
				if ( DisplayWnd != NULL ) SetWindowText(GetDlgItem(DisplayWnd,IDC_EDIT15),ProgrammNeu[i].Name);
				sprintf(Zeile,"0x%04x",ProgrammNeu[i].SID_pid);
				if ( DisplayWnd != NULL ) SetWindowText(GetDlgItem(DisplayWnd,IDC_EDIT16),Zeile);
				sprintf(Zeile,"0x%04x",ProgrammNeu[i].PMT_pid);
				if ( DisplayWnd != NULL ) SetWindowText(GetDlgItem(DisplayWnd,IDC_EDIT18),Zeile);
				(void)Get_PMT(i,ProgrammNeu[i].PMT_pid);
				Insert_CAT(i);
			}
			k = Update_Programm_List(i);
		}
	} 
}


void Sat_Freq_Scan_Thread( void ) 
{
	int pol;
    int sratenum;
    unsigned int srate;
    char zeile[128];
	int ttk;
	int Band;
	
	while  ( Transponder.freq <= LNB[Transponder.diseqc].MaxFreq*1000) {
        if ( ContinueScan == FALSE ) return;
		pol=0;
		while ( pol < 2 ) {
            if ( ContinueScan == FALSE ) return;
			Transponder.volt = pol;   // Vertikal=0
			
			if ( UseDiseqC() == TRUE ) {
				
				if ( LNB[Transponder.diseqc].Switch22khz == TRUE ) {
					if ( Transponder.freq/1000 >= LNB[Transponder.diseqc].SwitchFreq )  ttk= 1;
					else ttk=0;
				};
				
				Band = ( Transponder.freq/1000 >= LNB[Transponder.diseqc].SwitchFreq ) ? 1 : 0;
               	SetDiSEqC(Transponder.diseqc,Band,pol);
                Set22K( ttk );
			};
			
			
			
			Set_Transponder_Pol(&Transponder);
			DVB_Reset();
            if ( Timing.DVB_Reset_Scan_Wait > 0 ) Sleep(Timing.DVB_Reset_Scan_Wait);
			if ( ScanWndHandle != NULL ) SetWindowText(GetDlgItem(ScanWndHandle,IDC_EDIT3), pol ? "Horizontal" : "Vertikal");
			Get_Signal_Infos((unsigned char *)&Transponder.sync,&Transponder.sig,&Transponder.err);
			if ( ScanWndHandle != NULL ) InvalidateRect(ScanWndHandle ,NULL,FALSE);
			if ( (Transponder.sync&0x0f) >= 0x03 ) {
				if ( auto_srate == TRUE ) {
                    for ( sratenum = 0 ; sratenum < 16 ; sratenum++ ) {
						srate=SRates[sratenum];
						
						if ( srate != 0 ) {
							Transponder.volt = pol;   // Vertikal=0
							Transponder.srate = srate;
							sprintf(zeile,"%d",srate);
							if ( ScanWndHandle != NULL ) {
								SetWindowText(GetDlgItem(ScanWndHandle,IDC_EDIT6),zeile);
								InvalidateRect(ScanWndHandle ,NULL,FALSE);
							};
                            if ( ContinueScan == FALSE ) return;
							if ( Set_Transponder_SRate(&Transponder,Transponder.srate) == TRUE ) {
								Fine_Tune(-1000,4000,500);
								Fine_Tune(-500,500,250);
								if ( ScanWndHandle != NULL ) InvalidateRect(ScanWndHandle ,NULL,FALSE);
                                Load_And_Update_TP( ScanWndHandle);
								Transponder.freq += 3000;
								sratenum=16;
								pol=3;
                            }; 
						};
					};
					if ( pol != 3 ) Set_Transponder_SRate(&Transponder,27500);
					if ( ScanWndHandle != NULL ) SetWindowText(GetDlgItem(ScanWndHandle,IDC_EDIT6),"0");
				} else {
					if ( ContinueScan == FALSE ) return;
					if ( Set_Transponder_SRate(&Transponder,Transponder.srate) == TRUE ) {
						if ( ScanWndHandle != NULL ) InvalidateRect(ScanWndHandle ,NULL,FALSE);
						Fine_Tune(-1000,4000,500);
						Fine_Tune(-500,500,250);
						if ( ScanWndHandle != NULL ) InvalidateRect(ScanWndHandle ,NULL,FALSE);
						Load_And_Update_TP( ScanWndHandle);
						if ( ContinueScan == FALSE ) return;
						Transponder.freq += 3000;
						pol=3;
						
					}; 
				}
				
			};
			pol++;
		};
		if ( ScanWndHandle != NULL ) SetWindowText(GetDlgItem(ScanWndHandle,IDC_TEXT20),"Scanne");
		pol=0;
		Transponder.volt = pol;   // Vertikal=0
		Transponder.freq += 3000;
		Transponder.fec = 8;
		Set_Transponder_Freq(&Transponder);
		sprintf(zeile,"%10.3f",Transponder.freq/1000.f);
		if ( ScanWndHandle != NULL ) SetWindowText(GetDlgItem(ScanWndHandle,IDC_EDIT2),zeile);
	};
	if ( ScanWndHandle != NULL ) EnableWindow(GetDlgItem(ScanWndHandle,IDOK),TRUE);
};




BOOL APIENTRY SATScanProc(
						  HWND hDlg,
						  UINT message,
						  UINT wParam,
						  LONG lParam)
{
	char zeile[80];
	
	
    PAINTSTRUCT wps;                   /* paint structure           */
    HDC hdc;
    HDC             hMemDC;
    HBITMAP         hOldBm;
	BITMAP bm;
	
	
    HBRUSH hBrush;
    HWND wndFill;
    unsigned int Scan_Srate;
    DWORD LinkThreadID;
	RECT rc;
    HANDLE Thread;
	
	
	switch (message) {
		
	case WM_INITDIALOG:
		
        Transponder.fec=8;
        Transponder.diseqc=Programm[CurrentProgramm].diseqc;
		if (( LNB[1].Use == FALSE ) && ( LNB[2].Use == FALSE ) && ( LNB[3].Use == FALSE )) Transponder.diseqc=0;
		
		CheckDlgButton(hDlg, IDC_RADIO7, ( Transponder.diseqc == 0 ));
		CheckDlgButton(hDlg, IDC_RADIO8, ( Transponder.diseqc == 1 ));
		CheckDlgButton(hDlg, IDC_RADIO9, ( Transponder.diseqc == 2 ));
		CheckDlgButton(hDlg, IDC_RADIO10, ( Transponder.diseqc == 3 ));
		
		Get_Signal_Infos((unsigned char *)&Transponder.sync,&Transponder.sig,&Transponder.err);
		
		if (( LNB[1].Use == FALSE ) && ( LNB[2].Use == FALSE ) && ( LNB[3].Use == FALSE )) {
            EnableWindow(GetDlgItem(hDlg,IDC_RADIO7),FALSE);
			EnableWindow(GetDlgItem(hDlg,IDC_RADIO8),FALSE);
			EnableWindow(GetDlgItem(hDlg,IDC_RADIO9),FALSE);
			EnableWindow(GetDlgItem(hDlg,IDC_RADIO10),FALSE);
		} else {
			if ( LNB[1].Use == FALSE ) EnableWindow(GetDlgItem(hDlg,IDC_RADIO8),FALSE);
			if ( LNB[2].Use == FALSE ) EnableWindow(GetDlgItem(hDlg,IDC_RADIO9),FALSE);
			if ( LNB[3].Use == FALSE ) EnableWindow(GetDlgItem(hDlg,IDC_RADIO10),FALSE);
		};
		ContinueScan=FALSE;
		ScanWndHandle=NULL;
		SetDlgItemText(hDlg,IDC_EDIT6,"0");
		return(TRUE);
		
		
    case WM_PAINT:
		
		hdc = BeginPaint(hDlg,&wps);
		hMemDC = CreateCompatibleDC(hdc);
        if ((Transponder.sync&0x0f) > 0) hOldBm = SelectObject(hMemDC, BirneGruen);
		else hOldBm = SelectObject(hMemDC, BirneRot );
        GetObject(BirneGruen, sizeof(BITMAP), (LPSTR)&bm);
		
		BitBlt(hdc, 48, 153, bm.bmWidth, bm.bmHeight,hMemDC, 0, 0, SRCCOPY);// Signal
		
		SelectObject(hMemDC,hOldBm);
        if ((Transponder.sync&0x0f) > 1) hOldBm = SelectObject(hMemDC, BirneGruen);
		else hOldBm = SelectObject(hMemDC, BirneRot );
		
		BitBlt(hdc, 48, 180, bm.bmWidth, bm.bmHeight,hMemDC, 0, 0, SRCCOPY); // Carrier
		SelectObject(hMemDC,hOldBm);
		
		
        if ((Transponder.sync&0x0f) > 3) hOldBm = SelectObject(hMemDC, BirneGruen);
		else hOldBm = SelectObject(hMemDC, BirneRot );
		
		BitBlt(hdc, 116, 153, bm.bmWidth, bm.bmHeight,hMemDC, 0, 0, SRCCOPY); // Viterbi
		SelectObject(hMemDC,hOldBm);
		
		if ((Transponder.sync&0x0f) > 7) hOldBm = SelectObject(hMemDC, BirneGruen);
		else hOldBm = SelectObject(hMemDC, BirneRot );
		
		BitBlt(hdc, 116, 180, bm.bmWidth, bm.bmHeight,hMemDC, 0, 0, SRCCOPY); // FSync
		SelectObject(hMemDC,hOldBm);
		DeleteDC(hMemDC);
		DeleteDC(hdc);
		for ( i = 0; i < 32 ; i++ ) {
			wndFill = GetDlgItem (hDlg, S1+i );
			GetClientRect(wndFill, &rc);
			hdc = GetDC(wndFill);
			rc.bottom-=1;
			rc.top+=1;
			rc.right-=1;
			rc.left+=1;
			
			if ( (signed int)Transponder.sig >= i * 8 ) {
                hBrush=CreateSolidBrush(RGB(255-(8*i),8*i,0));
			} else {
				hBrush=CreateSolidBrush(RGB(0,0,0));
			}
            FillRect (hdc, &rc, hBrush);
            ReleaseDC (wndFill, hdc);
			DeleteObject(hBrush);
			
			wndFill = GetDlgItem (hDlg, E1+i );
			GetClientRect(wndFill, &rc);
			hdc = GetDC(wndFill);
			rc.bottom-=1;
			rc.top+=1;
			rc.right-=1;
			rc.left+=1;
			
			if ( (signed int)Transponder.err >= i * 8 ) {
                hBrush=CreateSolidBrush(RGB(255-(8*i),8*i,0));
			} else {
				hBrush=CreateSolidBrush(RGB(0,0,0));
			}
            FillRect (hdc, &rc, hBrush);
            ReleaseDC (wndFill, hdc);
			DeleteObject(hBrush);
		}
		
        EndPaint(hDlg,&wps);
		return(FALSE);
		break;
		
		
	case WM_COMMAND:
		
		if (LOWORD(wParam) == IDSTART ) {
			KillTimer(hWnd,1);
			TPCatAnzahl=0;
			Set_Capture(4);
			ShutDownFilters();
			ContinueScan=TRUE;
			if ( IsDlgButtonChecked(hDlg, IDC_RADIO7))
				Transponder.diseqc = 0;
			else if ( IsDlgButtonChecked(hDlg, IDC_RADIO8))
				Transponder.diseqc = 1;
			else if ( IsDlgButtonChecked(hDlg, IDC_RADIO9))
				Transponder.diseqc = 2;
			else if ( IsDlgButtonChecked(hDlg, IDC_RADIO10))
				Transponder.diseqc = 3;
			Transponder.freq=LNB[Transponder.diseqc].MinFreq*1000;
			sprintf(zeile,"%10.3f",Transponder.freq/1000.f);
			SetDlgItemText(hDlg,IDC_EDIT2,zeile);
			
			Scan_Srate = GetDlgItemInt(hDlg,IDC_EDIT6,NULL,FALSE);
			if (Scan_Srate == 0) {
				auto_srate = TRUE;
				Scan_Srate=27500;
			} else {
				auto_srate = FALSE;
			}
			
			Transponder.fec = 8;
			Transponder.srate=Scan_Srate;
			Transponder.power=LNB[Transponder.diseqc].Power;
			Transponder.volt=0;
			Transponder.freq = LNB[Transponder.diseqc].MinFreq*1000;
			Set_Transponder(&Transponder);
            ScanWndHandle=hDlg;
			EnableWindow(GetDlgItem(hDlg,IDSTART),FALSE);
			EnableWindow(GetDlgItem(hDlg,IDOK),FALSE);
           	Thread=CreateThread ((LPSECURITY_ATTRIBUTES)NULL,       // No security.
				(DWORD)0,                          // Same stack size.
				(LPTHREAD_START_ROUTINE)Sat_Freq_Scan_Thread,// Thread procedure.
				NULL,                         // Parameter.
				(DWORD)0,                          // Start immediatly.
				(LPDWORD)&LinkThreadID);  
			
			
			CloseHandle(Thread);
			
			
			return(TRUE);
		}
		
		if ( LOWORD(wParam) == IDOK ) {
			ContinueScan=FALSE;
			ScanWndHandle=hDlg;
			ShutDownFilters();
			Write_Programm_List();
			Write_Nit_List();
			Set_DVB_Channel(CurrentProgramm);
			SetTimer(hWnd,1,2500,NULL);
			EndDialog(hDlg,TRUE);
		}
		
		
		if ( LOWORD(wParam) == IDCANCEL) {
			ContinueScan=FALSE;
			ScanWndHandle=NULL;
			ShutDownFilters();
			Load_Programm_List();
			Load_Nit_List();
			Set_DVB_Channel(CurrentProgramm);
			SetTimer(hWnd,1,2500,NULL);
			EndDialog(hDlg,TRUE);
		}
		
		break;
	}
	return (FALSE);
    UNREFERENCED_PARAMETER(lParam);
 }
 
 
 
 void Cable_Freq_Scan_Thread( void ) 
 {
	 char zeile[128];
	 
	 
	 while  ( Transponder.freq <= LNB[0].MaxFreq*1000) {
		 if ( ContinueScan == FALSE ) return;
		 for ( Transponder.qam = 0 ; Transponder.qam < 5 ; Transponder.qam++ ) {
			 if ( ContinueScan == FALSE ) return;
			 VES1820_SetInversion(VES1820_Inversion);
			 SetQAM((unsigned short)Transponder.qam,1);
			 DVB_Reset();
			 if ( Timing.DVB_Reset_Scan_Wait > 0 ) Sleep(Timing.DVB_Reset_Scan_Wait);
			 if ( Transponder.qam == 0 ) strcpy(zeile,"16");
			 else if ( Transponder.qam == 1 ) strcpy(zeile,"32");
			 else if ( Transponder.qam == 2 ) strcpy(zeile,"64");
			 else if ( Transponder.qam == 3 ) strcpy(zeile,"128");
			 else if ( Transponder.qam == 4 ) strcpy(zeile,"256");
			 
			 Get_Signal_Infos((unsigned char *)&Transponder.sync,&Transponder.sig,&Transponder.err);
			 if ( ScanWndHandle != NULL ) {
				 SetWindowText(GetDlgItem(ScanWndHandle,IDC_EDIT3),zeile);
				 InvalidateRect(ScanWndHandle ,NULL,FALSE);
			 };
			 
			 sprintf(zeile,"SAA7146 Cable-Scan : Get_Signal_Infos Sync-State %02x ",Transponder.sync);
			 Write_Log(zeile);
			 
			 if ( (Transponder.sync&0x0f) >= 0x03 ) {
				 if ( auto_srate == TRUE ) {
					 for ( i = 0 ; i < 16 ; i++ ) {
						 if ( ContinueScan == FALSE ) return;
						 srate=SRates[i];
						 if ( Set_Transponder_SRate(&Transponder,srate) == TRUE ) break;
					 }
				 } else {
					 Set_Transponder_SRate(&Transponder,Transponder.srate);
					 srate=Transponder.srate;
				 }
				 
				 Get_Signal_Infos((unsigned char *)&Transponder.sync,&Transponder.sig,&Transponder.err);
				 sprintf(zeile,"SAA7146 Cable-Scan : Final Get_Signal_Infos Sync-State %02x ",Transponder.sync);
				 Write_Log(zeile);
				 if ( ScanWndHandle != NULL ) InvalidateRect(ScanWndHandle ,NULL,FALSE);
				 
				 if ((Transponder.sync&0x0f)==0x0f) {
					 Load_And_Update_TP( ScanWndHandle);
					 Transponder.freq+=4000000;
					 Transponder.qam=5;
				 }
				 
			 }  // Sync auf 0x03
			 
		 } // Quam_Loop;
		 if ( ScanWndHandle != NULL ) SetWindowText(GetDlgItem(ScanWndHandle,IDC_TEXT20),"Scanne");
		 Transponder.freq += 4000000;
		 Set_Transponder_Freq(&Transponder);
		 sprintf(zeile,"%10.3f",Transponder.freq/1000.f);
		 if ( ScanWndHandle != NULL ) SetWindowText(GetDlgItem(ScanWndHandle,IDC_EDIT2),zeile);
	 }  // Ende Loop Freq;
	 if ( ScanWndHandle != NULL ) EnableWindow(GetDlgItem(ScanWndHandle,IDOK),TRUE);
 }
 
 
 BOOL APIENTRY CableScanProc(
	 HWND hDlg,
	 UINT message,
	 UINT wParam,
	 LONG lParam)
 {
	 char zeile[80];
	 
	 int i;
	 
	 PAINTSTRUCT wps;                   /* paint structure           */
	 HDC hdc;
	 HDC             hMemDC;
	 HBITMAP         hOldBm;
	 BITMAP bm;
	 DWORD LinkThreadID;
	 HANDLE Thread;
	 
	 HBRUSH hBrush;
	 HWND wndFill;
	 RECT rc;
	 
	 switch (message) {
	 case WM_INITDIALOG:
		 Get_Signal_Infos((unsigned char *)&Transponder.sync,&Transponder.sig,&Transponder.err);
		 SetDlgItemText(hDlg,IDC_EDIT6,"0");
		 return(TRUE);
		 
		 
	 case WM_PAINT:
		 
	        hdc = BeginPaint(hDlg,&wps);
			hMemDC = CreateCompatibleDC(hdc);
            if ((Transponder.sync&0x0f) > 0) hOldBm = SelectObject(hMemDC, BirneGruen);
			else hOldBm = SelectObject(hMemDC, BirneRot );
            GetObject(BirneGruen, sizeof(BITMAP), (LPSTR)&bm);
			
			BitBlt(hdc, 48, 113, bm.bmWidth, bm.bmHeight,hMemDC, 0, 0, SRCCOPY);// Signal
			
			SelectObject(hMemDC,hOldBm);
            if ((Transponder.sync&0x0f) > 1) hOldBm = SelectObject(hMemDC, BirneGruen);
			else hOldBm = SelectObject(hMemDC, BirneRot );
			
			BitBlt(hdc, 48, 140, bm.bmWidth, bm.bmHeight,hMemDC, 0, 0, SRCCOPY); // Carrier
			SelectObject(hMemDC,hOldBm);
			
			
            if ((Transponder.sync&0x0f) > 3) hOldBm = SelectObject(hMemDC, BirneGruen);
			else hOldBm = SelectObject(hMemDC, BirneRot );
			
			BitBlt(hdc, 116, 113, bm.bmWidth, bm.bmHeight,hMemDC, 0, 0, SRCCOPY); // Viterbi
			SelectObject(hMemDC,hOldBm);
			
			if ((Transponder.sync&0x0f) > 7) hOldBm = SelectObject(hMemDC, BirneGruen);
			else hOldBm = SelectObject(hMemDC, BirneRot );
			
			BitBlt(hdc, 116, 140, bm.bmWidth, bm.bmHeight,hMemDC, 0, 0, SRCCOPY); // FSync
			SelectObject(hMemDC,hOldBm);
			DeleteDC(hMemDC);
			DeleteDC(hdc);
			for ( i = 0; i < 21 ; i++ ) {
				wndFill = GetDlgItem (hDlg, E1+i );
				GetClientRect(wndFill, &rc);
				hdc = GetDC(wndFill);
				rc.bottom-=1;
				rc.top+=1;
				rc.right-=1;
				rc.left+=1;
				
				if ( (signed int)Transponder.err >= i * 12 ) {
                    hBrush=CreateSolidBrush(RGB(255-(12*i),12*i,0));
				} else {
					hBrush=CreateSolidBrush(RGB(0,0,0));
				};
				FillRect (hdc, &rc, hBrush);
				ReleaseDC (wndFill, hdc);
				DeleteObject(hBrush);
			}
			
            EndPaint(hDlg,&wps);
			return(FALSE);
			
			
	 case WM_COMMAND:
		 
		 
		 
		 if (LOWORD(wParam) == IDSTART ) {
			 KillTimer(hWnd,1);
			 TPCatAnzahl=0;
			 Set_Capture(4);
			 ShutDownFilters();
			 ContinueScan=TRUE;
			 Transponder.freq=LNB[0].MinFreq*1000;
			 sprintf(zeile,"%10.3f",Transponder.freq/1000.f);
             SetDlgItemText(hDlg,IDC_EDIT2,zeile);
			 Transponder.srate=GetDlgItemInt(hDlg,IDC_EDIT6,NULL,FALSE);
			 if (Transponder.srate == 0) {
				 auto_srate = TRUE;
			 } else {
				 auto_srate = FALSE;
			 }
			 
			 Transponder.power=LNB[0].Power;
			 Transponder.volt=0;
             Set_Transponder(&Transponder);
			 sprintf(zeile,"%10.3f",Transponder.freq/1000.f);
             SetDlgItemText(hDlg,IDC_EDIT2,zeile);
			 EnableWindow(GetDlgItem(hDlg,IDSTART),FALSE);
			 EnableWindow(GetDlgItem(hDlg,IDOK),FALSE);
             ScanWndHandle=hDlg;
             Thread=CreateThread ((LPSECURITY_ATTRIBUTES)NULL,       // No security.
				 (DWORD)0,                          // Same stack size.
				 (LPTHREAD_START_ROUTINE)Cable_Freq_Scan_Thread,// Thread procedure.
				 NULL,                         // Parameter.
				 (DWORD)0,                          // Start immediatly.
				 (LPDWORD)&LinkThreadID);  
			 
			 
			 CloseHandle(Thread);
			 return(TRUE);
		 }
		 
		 if ( LOWORD(wParam) == IDOK ) {
			 ContinueScan=FALSE;
			 ScanWndHandle=hDlg;
			 ShutDownFilters();
			 Write_Programm_List();
			 Write_Nit_List();
			 Set_DVB_Channel(CurrentProgramm);
			 SetTimer(hWnd,1,2500,NULL);
			 EndDialog(hDlg,TRUE);
		 }
		 
		 
		 if ( LOWORD(wParam) == IDCANCEL) {
			 ContinueScan=FALSE;
			 ScanWndHandle=NULL;
			 ShutDownFilters();
			 Load_Programm_List();
			 Load_Nit_List();
			 Set_DVB_Channel(CurrentProgramm);
			 SetTimer(hWnd,1,2500,NULL);
			 EndDialog(hDlg,TRUE);
		 }
		 
		 break;
    }
    return (FALSE);
	UNREFERENCED_PARAMETER(lParam);
}




BOOL APIENTRY AnalogScanProc(
							 HWND hDlg,
							 UINT message,
							 UINT wParam,
							 LONG lParam)
{
	char zeile[80];
	
	int i,j;
	
    PAINTSTRUCT wps;                   /* paint structure           */
    HDC hdc;
    HDC             hMemDC;
    HBITMAP         hOldBm;
	BITMAP bm;
	
	static BOOL STOP;
    static unsigned int Freq;
    static int ChannelNr;
    static unsigned int FirstFreq=0;
	
	MSG msg;
	
	
	
    switch (message) {
	case WM_INITDIALOG:
		SendMessage (GetDlgItem(hDlg,IDC_COMBO1), CB_RESETCONTENT, 0, 0);
		for ( i=0; i< 35; i++ ) {
			if (  Countries[i].Name[0]!=0x00 ) SendMessage (GetDlgItem(hDlg,IDC_COMBO1), CB_INSERTSTRING, i, (LPARAM)(LPSTR)Countries[i].Name);
		};
		SendMessage(GetDlgItem(hDlg,IDC_COMBO1),CB_SETCURSEL,CountryCode,0);
		Load_Country_Specific_Settings(CountryCode);
		CheckDlgButton(hDlg, IDC_CHECK1, TRUE);
		if ( USETUNER == FALSE ) EnableWindow(GetDlgItem(hDlg,IDSTART),TRUE);
		
		return(TRUE);
		
	case WM_PAINT:
		hdc = BeginPaint(hDlg,&wps);
		hMemDC = CreateCompatibleDC(hdc);
		if ( VideoPresent() == TRUE ) hOldBm = SelectObject(hMemDC, BirneGruen);
		else hOldBm = SelectObject(hMemDC, BirneRot );
		GetObject(BirneGruen, sizeof(BITMAP), (LPSTR)&bm);
		
		BitBlt(hdc, 170, 80, bm.bmWidth, bm.bmHeight,hMemDC, 0, 0, SRCCOPY);// Signal
		
		SelectObject(hMemDC,hOldBm);
		DeleteDC(hMemDC);
		DeleteDC(hdc);
		EndPaint(hDlg,&wps);
		return(FALSE);
		
		
		
	case WM_USER:
		if ( wParam == 0x101 ) {
			sprintf(zeile,"%10.2f",Freq/1000.f);
			SetDlgItemText(hDlg,IDC_EDIT2,zeile);
			SetDlgItemText(hDlg,IDC_TEXT20,"Scanne");
			InvalidateRect(hDlg,NULL,FALSE);
			UpdateWindow(hDlg);
			i=0;
			if (IsDlgButtonChecked(hDlg,IDC_CHECK1) == TRUE ) Sleep(400);
			SetDlgItemText(hDlg,IDC_TEXT20,"Sync");
			while (( i < 75) && ( VideoPresent()== FALSE )) {
				i++;
				if ( PeekMessage(&msg,NULL,0,0xffffffff,PM_REMOVE) == TRUE ) {
			                 SendMessage(msg.hwnd,msg.message,msg.wParam,msg.lParam);
				};
				if ( STOP == TRUE ) return(TRUE);
				Sleep(1);
			};
			
			InvalidateRect(hDlg,NULL,FALSE);
			UpdateWindow(hDlg);
			
			if  ( VideoPresent() ) {
				if ( FirstFreq == 0 ) FirstFreq=Freq;
				SetDlgItemText(hDlg,IDC_TEXT20,"VideoSignal gefunden");
				VPS_lastname[0]=0x00;
				Packet30.Identifier[0]=0x00;
				j=0;
				while (( j < MAXPROGS) &&  ( Programm[j].freq != Freq )) j++;
				
				if ( j >= MAXPROGS ) {
					j=0;
					while (( j < MAXPROGS) &&  ( Programm[j].freq != 0 )) j++;
					if ( j > MAXPROGS ) {
						MessageBox(hWnd,"Alle Speicherplätze belegt","Multi-Decoder",MB_ICONINFORMATION|MB_OK);
						return(TRUE);	
					};
				};
				Programm[j].freq=Freq;
				strcpy(Programm[j].Name,"<Kein PDC>");
				Programm[j].Typ='A';
				Freq=Freq+2500;
				if ( Capture_VBI == TRUE ) {
					i=0;
					while ( i < 100 ) {
						if ( PeekMessage(&msg,NULL,0,0xffffffff,PM_REMOVE) == TRUE ) {
							SendMessage(msg.hwnd,msg.message,msg.wParam,msg.lParam);
						};
						Sleep(2);
						if ( VPS_lastname[0] != 0x00 ) {
							strcpy(Programm[j].Name,VPS_lastname);
							i=100;
						} else if ( Packet30.Identifier[0] != 0x00 ) {
							strcpy(Programm[j].Name,Packet30.Identifier);
							i=100;
						};
						i++;
					} 
				}
				
				SetDlgItemText(hDlg,IDC_EDIT15,Programm[j].Name);
			}
			
			
			if ( STOP == TRUE ) return(TRUE);
			if ( IsDlgButtonChecked(hDlg,IDC_CHECK1)) {
				ChannelNr++;
				if ( ChannelNr < Channels.MaxChannel ) {
					Freq=Channels.freq[ChannelNr];
					if(!Tuner_SetFrequency(TunerType,MulDiv((Freq*1000),16,1000000)))
					{   
						sprintf(Text,"Frequenz %10.2f Mhz nicht eingestellt ",Freq/1000.f);
						SetWindowText(hwndTextField,Text);
						return(TRUE);
					}; 
					if ( STOP == FALSE ) PostMessage(hDlg,WM_USER,0x101,0);
				} else {
					EnableWindow(GetDlgItem(hDlg,IDOK),TRUE);
                    EnableWindow(GetDlgItem(hDlg,IDSTART),TRUE);
                    if ( FirstFreq != 0 )  (void)Tuner_SetFrequency(TunerType,MulDiv((FirstFreq*1000),16,1000000));
				}
			} else {
				
				
				if( Freq < 870000 ) {
					Freq=Freq+500;
					if(!Tuner_SetFrequency(TunerType,MulDiv((Freq*1000),16,1000000)))
					{   
						sprintf(Text,"Frequenz %10.2f Mhz nicht eingestellt ",Freq/1000.f);
						SetWindowText(hwndTextField,Text);
						return(TRUE);
					}; 
					if ( STOP == FALSE ) PostMessage(hDlg,WM_USER,0x101,0);
				} else {
					EnableWindow(GetDlgItem(hDlg,IDOK),TRUE);
					EnableWindow(GetDlgItem(hDlg,IDSTART),TRUE);
                    if ( FirstFreq != 0 )  (void)Tuner_SetFrequency(TunerType,MulDiv((FirstFreq*1000),16,1000000));
				}
			};
		};
		
		break;
        
		case WM_COMMAND:
			
			if  ((HWND)lParam == GetDlgItem(hDlg,IDC_COMBO1)) {
				if (HIWORD(wParam)==CBN_SELCHANGE){
					CountryCode=SendMessage(GetDlgItem(hDlg,IDC_COMBO1),CB_GETCURSEL,0,0);
					Load_Country_Specific_Settings(CountryCode);
				};
			};
			
			
			if (LOWORD(wParam) == IDSTART ) {
				
				if ( IsDlgButtonChecked(hDlg,IDC_CHECK1)) {
					Freq=Channels.freq[0];
                    ChannelNr = 0 ;
				} else {
                    Freq = 41000;
				};
				
				STOP=FALSE;
				sprintf(zeile,"%10.2f",Freq/1000.f);
				SetDlgItemText(hDlg,IDC_EDIT2,zeile);
				
				if(!Tuner_SetFrequency(TunerType,MulDiv((Freq*1000),16,1000000)))
				{   
					sprintf(Text,"Frequenz %10.2f Mhz nicht eingestellt ",(float)(Freq/1000));
					SetWindowText(hwndTextField,Text);
				}; 
				
				KillTimer(hWnd,1); 	
				PostMessage(hDlg,WM_USER,0x101,0);
				EnableWindow(GetDlgItem(hDlg,IDSTART),FALSE);
				EnableWindow(GetDlgItem(hDlg,IDOK),FALSE);
				
			}
			
			if ( LOWORD(wParam) == IDOK) {
				Write_Programm_List();
				EndDialog(hDlg,TRUE);
			}
			
			
			if ( LOWORD(wParam) == IDCANCEL) {
				STOP=TRUE;
				Load_Programm_List();
				SetTimer (hWnd, 1, 2500, NULL);
				EndDialog(hDlg,TRUE);
			}
			
			break;
    }
    return (FALSE);
	UNREFERENCED_PARAMETER(lParam);
}



void Sat_Nit_Scan_Thread( void ) 
{
    char zeile[128];
	
	int NitCount;
	
	for ( NitCount=0; NitCount < MAXNIT ; NitCount++ ) {
		
		if  ((NIT[NitCount].ts_id != 0 )  && ( NIT[NitCount].DiseqNr == Transponder.diseqc )) {
			sprintf(zeile,"%6.2f",NIT[NitCount].orbit/10.f);
			if ( ContinueScan==FALSE) return;
			if ( strcmp(zeile,Nit_Orbital) == 0 ) {
				Transponder.srate=NIT[NitCount].srate;
				Transponder.freq=NIT[NitCount].freq;
				Transponder.power=LNB[Transponder.diseqc].Power;
				Transponder.volt=NIT[NitCount].polar;
				Transponder.fec=NIT[NitCount].fec;
				Set_Transponder(&Transponder);
				DVB_Reset();
				sprintf(zeile,"%10.3f",Transponder.freq/1000.f);
				if ( Timing.DVB_Reset_Scan_Wait > 0 ) Sleep(Timing.DVB_Reset_Scan_Wait);
				
				Get_Signal_Infos((unsigned char *)&Transponder.sync,&Transponder.sig,&Transponder.err);
				if ( ScanWndHandle != NULL ) {
					SetWindowText(GetDlgItem(ScanWndHandle,IDC_EDIT2),zeile);
					SetWindowText(GetDlgItem(ScanWndHandle,IDC_EDIT3), Transponder.volt ? "Horizontal" : "Vertikal");
					InvalidateRect(ScanWndHandle,NULL,FALSE);
				}
				Load_And_Update_TP( ScanWndHandle);
				if ( ContinueScan==FALSE) return;
			};
		};
	};
	
	if ( ScanWndHandle != NULL ) {
		EnableWindow(GetDlgItem(ScanWndHandle,IDOK),TRUE);
		EnableWindow(GetDlgItem(ScanWndHandle,IDSTART),TRUE);
	};
	
};



BOOL APIENTRY SATScanNitProc(
							 HWND hDlg,
							 UINT message,
							 UINT wParam,
							 LONG lParam)
{
	char zeile[140];
	int i,j;
    PAINTSTRUCT wps;                   /* paint structure           */
    HDC hdc;
    HDC             hMemDC;
    HBITMAP         hOldBm;
	BITMAP bm;
	
    DWORD LinkThreadID;
    HANDLE Thread;
	
    HBRUSH hBrush;
    HWND wndFill;
	
	RECT rc;
	
    switch (message) {
	case WM_INITDIALOG:
		Transponder.fec=8;
		Transponder.diseqc=Programm[CurrentProgramm].diseqc;
		if (( LNB[1].Use == FALSE ) && ( LNB[2].Use == FALSE ) && ( LNB[3].Use == FALSE )) Transponder.diseqc=0;
		
		if ( Transponder.diseqc == 0 ) CheckDlgButton(hDlg, IDC_RADIO7, TRUE);
		else if ( Transponder.diseqc == 1 ) CheckDlgButton(hDlg, IDC_RADIO8, TRUE);
		else if ( Transponder.diseqc == 2 ) CheckDlgButton(hDlg, IDC_RADIO9, TRUE);
		else if ( Transponder.diseqc == 3 ) CheckDlgButton(hDlg, IDC_RADIO10, TRUE);
		Get_Signal_Infos((unsigned char *)&Transponder.sync,&Transponder.sig,&Transponder.err);
		
		if (( LNB[1].Use == FALSE ) && ( LNB[2].Use == FALSE ) && ( LNB[3].Use == FALSE )) {
			EnableWindow(GetDlgItem(hDlg,IDC_RADIO7),FALSE);
			EnableWindow(GetDlgItem(hDlg,IDC_RADIO8),FALSE);
			EnableWindow(GetDlgItem(hDlg,IDC_RADIO9),FALSE);
			EnableWindow(GetDlgItem(hDlg,IDC_RADIO10),FALSE);
		} else {
			if ( LNB[1].Use == FALSE ) EnableWindow(GetDlgItem(hDlg,IDC_RADIO8),FALSE);
			if ( LNB[2].Use == FALSE ) EnableWindow(GetDlgItem(hDlg,IDC_RADIO9),FALSE);
			if ( LNB[3].Use == FALSE ) EnableWindow(GetDlgItem(hDlg,IDC_RADIO10),FALSE);
		};
		
		PostMessage(hDlg,WM_USER,0x100,0);
		
		
		return(TRUE);
		
		
	case WM_PAINT:
		
		hdc = BeginPaint(hDlg,&wps);
		hMemDC = CreateCompatibleDC(hdc);
		if ((Transponder.sync&0x0f) > 0) hOldBm = SelectObject(hMemDC, BirneGruen);
		else hOldBm = SelectObject(hMemDC, BirneRot );
		GetObject(BirneGruen, sizeof(BITMAP), (LPSTR)&bm);
		
		BitBlt(hdc, 48, 153, bm.bmWidth, bm.bmHeight,hMemDC, 0, 0, SRCCOPY);// Signal
		
		SelectObject(hMemDC,hOldBm);
		if ((Transponder.sync&0x0f) > 1) hOldBm = SelectObject(hMemDC, BirneGruen);
		else hOldBm = SelectObject(hMemDC, BirneRot );
		
		BitBlt(hdc, 48, 180, bm.bmWidth, bm.bmHeight,hMemDC, 0, 0, SRCCOPY); // Carrier
		SelectObject(hMemDC,hOldBm);
		
		
		if ((Transponder.sync&0x0f) > 3) hOldBm = SelectObject(hMemDC, BirneGruen);
		else hOldBm = SelectObject(hMemDC, BirneRot );
		
		BitBlt(hdc, 116, 153, bm.bmWidth, bm.bmHeight,hMemDC, 0, 0, SRCCOPY); // Viterbi
		SelectObject(hMemDC,hOldBm);
		
		if ((Transponder.sync&0x0f) > 7) hOldBm = SelectObject(hMemDC, BirneGruen);
		else hOldBm = SelectObject(hMemDC, BirneRot );
		
		BitBlt(hdc, 116, 180, bm.bmWidth, bm.bmHeight,hMemDC, 0, 0, SRCCOPY); // FSync
		SelectObject(hMemDC,hOldBm);
		DeleteDC(hMemDC);
		DeleteDC(hdc);
		for ( i = 0; i < 32 ; i++ ) {
            wndFill = GetDlgItem (hDlg, S1+i );
			GetClientRect(wndFill, &rc);
            hdc = GetDC(wndFill);
			rc.bottom-=1;
			rc.top+=1;
			rc.right-=1;
			rc.left+=1;
			if ( (signed int)Transponder.sig >= i * 8 ) {
				hBrush=CreateSolidBrush(RGB(255-(8*i),8*i,0));
			} else {
				hBrush=CreateSolidBrush(RGB(0,0,0));
			};
			FillRect (hdc, &rc, hBrush);
			ReleaseDC (wndFill, hdc);
			DeleteObject(hBrush);
			
            wndFill = GetDlgItem (hDlg, E1+i );
			GetClientRect(wndFill, &rc);
            hdc = GetDC(wndFill);
			rc.bottom-=1;
			rc.top+=1;
			rc.right-=1;
			rc.left+=1;
			
			if ( (signed int)Transponder.err >= i * 8 ) {
				hBrush=CreateSolidBrush(RGB(255-(8*i),8*i,0));
			} else {
				hBrush=CreateSolidBrush(RGB(0,0,0));
			};
			FillRect (hdc, &rc, hBrush);
			ReleaseDC (wndFill, hdc);
			DeleteObject(hBrush);
		}
		
		EndPaint(hDlg,&wps);
		return(FALSE);
		
	case WM_USER:
		if ( wParam == 0x100 ) {
			SendMessage (GetDlgItem(hDlg,IDC_COMBO1), CB_RESETCONTENT, 0, 0);
			if ( IsDlgButtonChecked(hDlg, IDC_RADIO7)) Transponder.diseqc = 0;
			else if ( IsDlgButtonChecked(hDlg, IDC_RADIO8)) Transponder.diseqc = 1;
			else if ( IsDlgButtonChecked(hDlg, IDC_RADIO9)) Transponder.diseqc = 2;
			else if ( IsDlgButtonChecked(hDlg, IDC_RADIO10)) Transponder.diseqc = 3;
			
			for ( i=0; i < MAXNIT ; i++ ) {
				if  ((NIT[i].ts_id != 0 )  && ( NIT[i].DiseqNr == Transponder.diseqc )) {
					sprintf(zeile,"%6.2f",NIT[i].orbit/10.f);
					j=SendMessage (GetDlgItem(hDlg,IDC_COMBO1), CB_FINDSTRING, 0, (LPARAM)zeile);
					if ( j < 0 ) SendMessage (GetDlgItem(hDlg,IDC_COMBO1), CB_ADDSTRING, 0, (LPARAM)zeile);
				}
			}
			SendMessage (GetDlgItem(hDlg,IDC_COMBO1), CB_SETCURSEL, 0, 0);
            return(TRUE);
		}
		
        
	case WM_COMMAND:
		
		if ( ((HWND)lParam == GetDlgItem(hDlg,IDC_RADIO7)) ||
			((HWND)lParam == GetDlgItem(hDlg,IDC_RADIO8)) ||
			((HWND)lParam == GetDlgItem(hDlg,IDC_RADIO9)) ||
			((HWND)lParam == GetDlgItem(hDlg,IDC_RADIO10))) {
			PostMessage(hDlg,WM_USER,0x100,0);
		};
		
		
		if (LOWORD(wParam) == IDSTART ) {
			
			EnableWindow(GetDlgItem(hDlg,IDSTART),FALSE);
			EnableWindow(GetDlgItem(hDlg,IDOK),FALSE);
			i=SendMessage (GetDlgItem(hDlg,IDC_COMBO1), CB_GETCURSEL, 0, 0);
			SendMessage (GetDlgItem(hDlg,IDC_COMBO1), CB_GETLBTEXT, i, (LPARAM)&Nit_Orbital);
			
			KillTimer(hWnd,1);
			TPCatAnzahl=0;
			Set_Capture(4);
			ShutDownFilters();
			ContinueScan=TRUE;
			
			ScanWndHandle=hDlg;
			
           	Thread=CreateThread ((LPSECURITY_ATTRIBUTES)NULL,       // No security.
				(DWORD)0,                          // Same stack size.
				(LPTHREAD_START_ROUTINE)Sat_Nit_Scan_Thread,// Thread procedure.
				NULL,                         // Parameter.
				(DWORD)0,                          // Start immediatly.
				(LPDWORD)&LinkThreadID);  
			
			CloseHandle(Thread);
		};
		
		if ( LOWORD(wParam) == IDOK ) {
			ContinueScan=FALSE;
			ScanWndHandle=hDlg;
			ShutDownFilters();
			Write_Programm_List();
			Write_Nit_List();
			Set_DVB_Channel(CurrentProgramm);
			SetTimer(hWnd,1,2500,NULL);
			EndDialog(hDlg,TRUE);
		}
		
		
		if ( LOWORD(wParam) == IDCANCEL) {
			ContinueScan=FALSE;
			ScanWndHandle=NULL;
			ShutDownFilters();
			Load_Programm_List();
			Load_Nit_List();
			Set_DVB_Channel(CurrentProgramm);
			SetTimer(hWnd,1,2500,NULL);
			EndDialog(hDlg,TRUE);
		}
		
		break;
    }
    return (FALSE);
	UNREFERENCED_PARAMETER(lParam);
}

void Cable_Nit_Scan_Thread( void ) 
{
    char zeile[128];
	
	int i;
	for ( i=0; i < MAXNIT ; i++ ) {
		if ( ContinueScan==FALSE) return;
		if  (NIT[i].ts_id != 0 )  {
			Transponder.srate=NIT[i].srate;
			Transponder.freq=NIT[i].freq;
			Transponder.power=LNB[0].Power;
			Transponder.qam=NIT[i].qam;
			Transponder.volt=NIT[i].polar;
			Transponder.fec=NIT[i].fec;
			Set_Transponder(&Transponder);
			DVB_Reset();
			if ( Timing.DVB_Reset_Scan_Wait > 0 ) Sleep(Timing.DVB_Reset_Scan_Wait);
			Get_Signal_Infos((unsigned char *)&Transponder.sync,&Transponder.sig,&Transponder.err);
			if ( ScanWndHandle != NULL ) {
				sprintf(zeile,"%10.3f",Transponder.freq/1000.f);
				SetWindowText(GetDlgItem(ScanWndHandle,IDC_EDIT2),zeile);
				if ( Transponder.qam == 0 ) strcpy(zeile,"16");
				else if ( Transponder.qam == 1 ) strcpy(zeile,"32");
				else if ( Transponder.qam == 2 ) strcpy(zeile,"64");
				else if ( Transponder.qam == 3 ) strcpy(zeile,"128");
				else if ( Transponder.qam == 4 ) strcpy(zeile,"256");
				SetWindowText(GetDlgItem(ScanWndHandle,IDC_EDIT3), zeile);
				sprintf(zeile,"%d",Transponder.srate);
				SetWindowText(GetDlgItem(ScanWndHandle,IDC_EDIT6),zeile);
				InvalidateRect(ScanWndHandle,NULL,FALSE);
			}
			Load_And_Update_TP( ScanWndHandle);
			if ( ContinueScan==FALSE) return;
		}
		
				};
	if ( ScanWndHandle != NULL ) {
		EnableWindow(GetDlgItem(ScanWndHandle,IDOK),TRUE);
		EnableWindow(GetDlgItem(ScanWndHandle,IDSTART),TRUE);
	};
	
};




BOOL APIENTRY CableScanNitProc(
							   HWND hDlg,
							   UINT message,
							   UINT wParam,
							   LONG lParam)
{
	int i;
    DWORD LinkThreadID;
    HANDLE Thread;
    PAINTSTRUCT wps;                   /* paint structure           */
    HDC hdc;
    HDC             hMemDC;
    HBITMAP         hOldBm;
	BITMAP bm;
	
    HBRUSH hBrush;
    HWND wndFill;
	RECT rc;
    switch (message) {
	case WM_INITDIALOG:
		Transponder.diseqc=0;
		Get_Signal_Infos((unsigned char *)&Transponder.sync,&Transponder.sig,&Transponder.err);
		return(TRUE);
		
		
	case WM_PAINT:
		
		hdc = BeginPaint(hDlg,&wps);
		hMemDC = CreateCompatibleDC(hdc);
		if ((Transponder.sync&0x0f) > 0) hOldBm = SelectObject(hMemDC, BirneGruen);
		else hOldBm = SelectObject(hMemDC, BirneRot );
		GetObject(BirneGruen, sizeof(BITMAP), (LPSTR)&bm);
		
		BitBlt(hdc, 44, 83, bm.bmWidth, bm.bmHeight,hMemDC, 0, 0, SRCCOPY);// Signal
		
		SelectObject(hMemDC,hOldBm);
		if ((Transponder.sync&0x0f) > 1) hOldBm = SelectObject(hMemDC, BirneGruen);
		else hOldBm = SelectObject(hMemDC, BirneRot );
		
		BitBlt(hdc, 44, 110, bm.bmWidth, bm.bmHeight,hMemDC, 0, 0, SRCCOPY); // Carrier
		SelectObject(hMemDC,hOldBm);
		
		
		if ((Transponder.sync&0x0f) > 3) hOldBm = SelectObject(hMemDC, BirneGruen);
		else hOldBm = SelectObject(hMemDC, BirneRot );
		
		BitBlt(hdc, 112, 83, bm.bmWidth, bm.bmHeight,hMemDC, 0, 0, SRCCOPY); // Viterbi
		SelectObject(hMemDC,hOldBm);
		
		if ((Transponder.sync&0x0f) > 7) hOldBm = SelectObject(hMemDC, BirneGruen);
		else hOldBm = SelectObject(hMemDC, BirneRot );
		
		BitBlt(hdc, 112, 110, bm.bmWidth, bm.bmHeight,hMemDC, 0, 0, SRCCOPY); // FSync
		SelectObject(hMemDC,hOldBm);
		DeleteDC(hMemDC);
		DeleteDC(hdc);
		for ( i = 0; i < 21 ; i++ ) {
            wndFill = GetDlgItem (hDlg, E1+i );
			GetClientRect(wndFill, &rc);
            hdc = GetDC(wndFill);
			rc.bottom-=1;
			rc.top+=1;
			rc.right-=1;
			rc.left+=1;
			
			if ( (signed int)Transponder.err >= i * 12 ) {
				hBrush=CreateSolidBrush(RGB(255-(12*i),12*i,0));
			} else {
				hBrush=CreateSolidBrush(RGB(0,0,0));
			};
			FillRect (hdc, &rc, hBrush);
			ReleaseDC (wndFill, hdc);
			DeleteObject(hBrush);
		}
		
		EndPaint(hDlg,&wps);
		return(FALSE);
		
        
	case WM_COMMAND:
		
		if (LOWORD(wParam) == IDSTART ) {
			EnableWindow(GetDlgItem(hDlg,IDSTART),FALSE);
			EnableWindow(GetDlgItem(hDlg,IDOK),FALSE);
			KillTimer(hWnd,1);
			TPCatAnzahl=0;
			Set_Capture(4);
			ShutDownFilters();
			ContinueScan=TRUE;
			ScanWndHandle=hDlg;
			
           	Thread=CreateThread ((LPSECURITY_ATTRIBUTES)NULL,       // No security.
				(DWORD)0,                          // Same stack size.
				(LPTHREAD_START_ROUTINE)Cable_Nit_Scan_Thread,// Thread procedure.
				NULL,                         // Parameter.
				(DWORD)0,                          // Start immediatly.
				(LPDWORD)&LinkThreadID);  
			
			CloseHandle(Thread);
			
		};
		
		if ( LOWORD(wParam) == IDOK ) {
			ContinueScan=FALSE;
			ScanWndHandle=hDlg;
			ShutDownFilters();
			Write_Programm_List();
			Write_Nit_List();
			Set_DVB_Channel(CurrentProgramm);
			SetTimer(hWnd,1,2500,NULL);
			EndDialog(hDlg,TRUE);
		}
		
		
		if ( LOWORD(wParam) == IDCANCEL) {
			ContinueScan=FALSE;
			ScanWndHandle=NULL;
			ShutDownFilters();
			Load_Programm_List();
			Load_Nit_List();
			Set_DVB_Channel(CurrentProgramm);
			SetTimer(hWnd,1,2500,NULL);
			EndDialog(hDlg,TRUE);
		}
		
		break;
    }
    return (FALSE);
	UNREFERENCED_PARAMETER(lParam);
}



BOOL APIENTRY ByteFilterScanProc(
								 HWND hDlg,
								 UINT message,
								 UINT wParam,
								 LONG lParam)
{
	char zeile[140];
	char Zeile[140];
	char zeile1[140];
	static BOOL STOP=FALSE;
	
	static unsigned short NextPid=0x0000;
	static unsigned short NextTid=0x0000;
	
	static unsigned short Retry[MAXFILTER];
	
	LOGFONT Mfont = {9,5,0,0,0,0,0,0,0,0,0,0,0,"Fixedsys"};   
    HFONT currFont;
    int FilterID;
    static int MaxRetry=16;
	
	
    switch (message) {
	case WM_INITDIALOG:
		
		currFont = CreateFontIndirect(&Mfont);
		SendMessage(GetDlgItem(hDlg,IDC_LIST1),WM_SETFONT,(WPARAM)currFont,TRUE);
		SendMessage (GetDlgItem(hDlg,IDC_LIST1), LB_RESETCONTENT, 0, 0);
		SetDlgItemText(hDlg,IDC_EDIT3,"4");
		return(TRUE);
		
		
	case WM_TIMER:
		FilterID = LOWORD(wParam);
		KillTimer(hDlg,FilterID);
		if ( STOP ) return(TRUE);
		Retry[FilterID]++;
		
		if ( WaitForSingleObject(FilterEvent[FilterID], 0 ) == WAIT_OBJECT_0 ) {
			Retry[FilterID]=0xff;
			ResetEvent(FilterEvent[FilterID]);
			
			if ( Filter[FilterID].pid==0x0 ) sprintf(zeile,"PAT       (0x%02x)",Filter[FilterID].pid);
			else if ( Filter[FilterID].pid==0x1 )  sprintf(zeile,"BAT/CAT   (0x%02x)",Filter[FilterID].pid);
			else if ( Filter[FilterID].pid==0x10 ) sprintf(zeile,"NIT       (0x%02x)",Filter[FilterID].pid);
			else if ( Filter[FilterID].pid==0x11 ) sprintf(zeile,"SDT       (0x%02x)",Filter[FilterID].pid);
			else if ( Filter[FilterID].pid==0x12 ) sprintf(zeile,"EIT       (0x%02x)",Filter[FilterID].pid);
			else if ( Filter[FilterID].pid==0x13 ) sprintf(zeile,"RST       (0x%02x)",Filter[FilterID].pid);
			else if ( Filter[FilterID].pid==0x14 ) sprintf(zeile,"TDT/TOT/SF(0x%02x)",Filter[FilterID].pid);
			else								   sprintf(zeile,"Unbekannt (0x%02x)",Filter[FilterID].pid);
			
			
			if ( Filter[FilterID].Section == 0x00 )      sprintf(zeile1,"TID_PAT    (0x%02x)",Filter[FilterID].Section);
			else if ( Filter[FilterID].Section== 0x01 ) sprintf(zeile1,"TID_CAT/BAT(0x%02x)",Filter[FilterID].Section);
			else if ( Filter[FilterID].Section == 0x02 ) sprintf(zeile1,"TID_PMT    (0x%02x)",Filter[FilterID].Section);
			else if ( Filter[FilterID].Section == 0x40 ) sprintf(zeile1,"TID_CAT    (0x%02x)",Filter[FilterID].Section);
			else if ( Filter[FilterID].Section == 0x41 ) sprintf(zeile1,"TID_CAT    (0x%02x)",Filter[FilterID].Section);
			else if ( Filter[FilterID].Section == 0x42 ) sprintf(zeile1,"TID_CAT    (0x%02x)",Filter[FilterID].Section);
			else if ( Filter[FilterID].Section == 0x46 ) sprintf(zeile1,"TID_CAT    (0x%02x)",Filter[FilterID].Section);
			else if ( Filter[FilterID].Section == 0x70 ) sprintf(zeile1,"TID_CAT    (0x%02x)",Filter[FilterID].Section);
			else if ( Filter[FilterID].Section == 0x73 ) sprintf(zeile1,"TID_CAT    (0x%02x)",Filter[FilterID].Section);
			else if ( Filter[FilterID].Section == 0x12 ) sprintf(zeile1,"TID_EIT    (0x%02x)",Filter[FilterID].Section);
			else if ( Filter[FilterID].Section == 0x13 ) sprintf(zeile1,"TID_RST    (0x%02x)",Filter[FilterID].Section);
			else if ( Filter[FilterID].Section == 0x14 ) sprintf(zeile1,"TID_ST     (0x%02x)",Filter[FilterID].Section);
			else  sprintf(zeile1,"Unbekannt   (0x%02x)",Filter[FilterID].Section);
			sprintf(Zeile,"%s %s",zeile,zeile1);
			SendMessage(GetDlgItem(hDlg,IDC_LIST1),LB_INSERTSTRING,0,(LPARAM)Zeile);
			sprintf(zeile,"SAA7146 ByteFilterScan %s",Zeile);
			Write_Log(zeile);
		};
		
		if ( Retry[FilterID] > MaxRetry ) {
			DelBitFilter(FilterID);
			if ( NextPid < 0x1fff ) {
				SetDlgItemInt(hDlg,IDC_EDIT1,NextPid,FALSE);
				SetDlgItemInt(hDlg,IDC_EDIT2,NextTid,FALSE);
				Retry[FilterID]=0;
				FilterID=SetFilter(NextPid, (unsigned short)((NextTid<<8)|0x00ff), 4,"Bit-Filter Scan");             
				if ( FilterID > 0 ) {
					NextTid++;
					if ( NextTid > 0xff ) {
						NextPid++;
						NextTid=0;
					};
					SetTimer(hDlg,FilterID,250,NULL);
				};
			}; 
		} else {
			SetTimer(hDlg,FilterID,250,NULL);
		}
		break;
		
	case WM_COMMAND:
		
		
		if (LOWORD(wParam) == IDSTART ) {
			STOP = FALSE;
			MaxRetry=GetDlgItemInt(hDlg,IDC_EDIT3,NULL,FALSE);
            MaxRetry=MaxRetry*4;
			NextPid=0x00;
			NextTid=0;
            FilterID=1;
			while ( FilterID >= 0 ) {
                SetDlgItemInt(hDlg,IDC_EDIT1,NextPid,FALSE);
                SetDlgItemInt(hDlg,IDC_EDIT2,NextTid,FALSE);
				FilterID=SetFilter((unsigned short)NextPid, (unsigned short)((NextTid<<8)|0x00ff), 4,"Bit-Filter Scan");
                if ( FilterID >= 0 ) {
					Retry[FilterID]=0;
					NextTid++;
					SetTimer(hDlg,FilterID,250,NULL);
				};
			};
			
		};
		
		if ( LOWORD(wParam) == IDCANCEL) {
			STOP=TRUE;
			ShutDownFiltersByName("Bit-Filter Scan");
			EndDialog(hDlg,TRUE);
		}
		
		break;
    }
    return (FALSE);
	UNREFERENCED_PARAMETER(lParam);
}

BOOL APIENTRY StreamFilterScanProc(
								   HWND hDlg,
								   UINT message,
								   UINT wParam,
								   LONG lParam)
{
	char zeile[140];
	char Zeile[140];
	static BOOL STOP=FALSE;
	
	static unsigned short NextPid=0x0000;
	
	static unsigned short Retry[MAXFILTER];
	
	LOGFONT Mfont = {9,5,0,0,0,0,0,0,0,0,0,0,0,"Fixedsys"};   
    HFONT currFont;
    int FilterID;
	
    switch (message) {
	case WM_INITDIALOG:
		
		currFont = CreateFontIndirect(&Mfont);
		SendMessage(GetDlgItem(hDlg,IDC_LIST1),WM_SETFONT,(WPARAM)currFont,TRUE);
		SendMessage (GetDlgItem(hDlg,IDC_LIST1), LB_RESETCONTENT, 0, 0);
		return(TRUE);
		
		
	case WM_TIMER:
		FilterID = LOWORD(wParam);
		KillTimer(hDlg,FilterID);
		if ( STOP ) return(TRUE);
		Retry[FilterID]++;
		
		if ( WaitForSingleObject(FilterEvent[FilterID], 0 ) == WAIT_OBJECT_0 ) {
			Retry[FilterID]=0xff;
			ResetEvent(FilterEvent[FilterID]);
			sprintf(zeile,"Pid %d   ( 0x%04x ) ",Filter[FilterID].pid,Filter[FilterID].pid);
			SendMessage(GetDlgItem(hDlg,IDC_LIST1),LB_INSERTSTRING,0,(LPARAM)zeile);
			sprintf(Zeile,"SAA7146 PIDFilterScan %s",zeile);
			Write_Log(Zeile);
		};
		
		if ( Retry[FilterID] > 10 ) {
			DelBitFilter(FilterID);
			if ( NextPid < 0x1fff ) {
				SetDlgItemInt(hDlg,IDC_EDIT1,NextPid,FALSE);
				Retry[FilterID]=0;
				FilterID=SetStreamFilter(NextPid,"Pipe/Stream Scan");    
				if ( FilterID >= 0 ) {
					NextPid++;
					SetTimer(hDlg,FilterID,500,NULL);
				};
				
			};
			
		} else {
			SetTimer(hDlg,FilterID,500,NULL);
		}
		break;
		
	case WM_COMMAND:
		
		
		if (LOWORD(wParam) == IDSTART ) {
			STOP = FALSE;
			NextPid=0x0;
			FilterID=1;
			while ( FilterID >= 0 ) {
                SetDlgItemInt(hDlg,IDC_EDIT1,NextPid,FALSE);
				FilterID=SetStreamFilter((unsigned short)NextPid,"Pipe/Stream Scan");
                if ( FilterID >= 0 ) { 
					NextPid++;
					Retry[FilterID]=0;
					SetTimer(hDlg,FilterID,500,NULL);
				};
			};
			
		};
		
		if ( LOWORD(wParam) == IDCANCEL) {
			STOP=TRUE;
			ShutDownFiltersByName("Pipe/Stream Scan");
			EndDialog(hDlg,TRUE);
		}
		
		break;
    }
    return (FALSE);
	UNREFERENCED_PARAMETER(lParam);
}


BOOL APIENTRY AudioPidSettingProc(
								  HWND hDlg,
								  UINT message,
								  UINT wParam,
								  LONG lParam)
{
	
	struct CurrentAudio_t {
		unsigned short Pid;
		unsigned char Name[30];
	};
	
	static struct CurrentAudio_t Audio[13];
	char Zeile[50];
	int i,j,k;
	static int AudioAnzahl;
	
	LOGFONT Mfont = {9,5,0,0,0,0,0,0,0,0,0,0,0,"Fixedsys"};   
    HFONT currFont;
	
	
    switch (message) {
	case WM_INITDIALOG:
		currFont = CreateFontIndirect(&Mfont);
		SendMessage(GetDlgItem(hDlg,IDC_LIST1),WM_SETFONT,(WPARAM)currFont,TRUE);
		SendMessage (GetDlgItem(hDlg,IDC_LIST1), LB_RESETCONTENT, 0, 0);
		j=0;
		for ( i = 0 ; i < Programm[CurrentProgramm].Filteranzahl; i++ ) {
			if (( Programm[CurrentProgramm].Filters[i].FilterId == 0x03 ) || 
				( Programm[CurrentProgramm].Filters[i].FilterId == 0x04 )) {
				Audio[j].Pid=Programm[CurrentProgramm].Filters[i].PID;
				GetAudioName(CurrentProgramm,i, &Audio[j].Name[0]);
				j++;
			}
		};
		AudioAnzahl=j;
		
		for ( i = 0; i < AudioAnzahl; i++ ) {
			sprintf(Zeile,"0x%04x   %s",Audio[i].Pid,Audio[i].Name);
			SendMessage (GetDlgItem(hDlg,IDC_LIST1), LB_INSERTSTRING, i, (LPARAM)Zeile);
		};
		j=FindActiveAudioName(CurrentProgramm,(unsigned char *)&Zeile[0]);
		if ( j >= 0 ) {
			i=0;
			k=-1;
			while (( i < AudioAnzahl ) && ( k == -1 )) {
				if ( Programm[CurrentProgramm].Filters[j].PID == Audio[i].Pid ) k=i;
				i++;
			};
		}
		SetDlgItemText(hDlg,IDC_EDIT1,Programm[CurrentProgramm].Name);
		SetDlgItemText(hDlg,IDC_EDIT3,Zeile);
		sprintf(Zeile,"0x%04x",Programm[CurrentProgramm].Audio_pid);
		SetDlgItemText(hDlg,IDC_EDIT2,Zeile);
		
		if ( k >= 0 ) SendMessage (GetDlgItem(hDlg,IDC_LIST1), LB_SETCURSEL, k, 0);
		return(TRUE);
		
		
        
	case WM_COMMAND:
		
		
		if (LOWORD(wParam) == IDC_LIST1 ) {
			if (HIWORD(wParam)==LBN_SELCHANGE) {
				i=SendMessage(GetDlgItem(hDlg,IDC_LIST1),LB_GETCURSEL,0,0);
				if (( i >= 0 ) && (i < AudioAnzahl )) {
					Programm[CurrentProgramm].Temp_Audio=Audio[i].Pid;  
					SetAudioPid(Audio[i].Pid);
				}
			};
		};
		
		if ( LOWORD(wParam) == IDCANCEL) {
			Load_Programm_List();
			Programm[CurrentProgramm].Temp_Audio=Programm[CurrentProgramm].Audio_pid;
			SetAudioPid(Programm[CurrentProgramm].Audio_pid);
			EndDialog(hDlg,TRUE);
		}
		
		if ( LOWORD(wParam) == IDOK ) {
			
			EndDialog(hDlg,TRUE);
		}
		
		if ( LOWORD(wParam) == IDDEFAULT) {
			i=SendMessage(GetDlgItem(hDlg,IDC_LIST1),LB_GETCURSEL,0,0);
			if (( i >= 0 ) && (i < AudioAnzahl )) {
				Programm[CurrentProgramm].Audio_pid=Audio[i].Pid;
				Programm[CurrentProgramm].Temp_Audio=Audio[i].Pid;
				
				j=FindActiveAudioName(CurrentProgramm,(unsigned char *)&Zeile[0]);
				SetDlgItemText(hDlg,IDC_EDIT3,Zeile);
				sprintf(Zeile,"0x%04x",Programm[CurrentProgramm].Audio_pid);
				SetDlgItemText(hDlg,IDC_EDIT2,Zeile);
				Write_Programm_List();
				
			};
		}
		
		break;
    }
    return (FALSE);
	UNREFERENCED_PARAMETER(lParam);
								  }

								  
								  BOOL APIENTRY TextPidSettingProc(
									  HWND hDlg,
									  UINT message,
									  UINT wParam,
									  LONG lParam)
								  {
									  
									  struct CurrentText_t {
										  unsigned short Pid;
										  unsigned char Name[30];
									  };
									  
									  static struct CurrentText_t TextPid[64];
									  char Zeile[50];
									  int i,k;
									  static int TextAnzahl;
									  
									  LOGFONT Mfont = {9,5,0,0,0,0,0,0,0,0,0,0,0,"Fixedsys"};   
									  HFONT currFont;
									  
									  
									  switch (message) {
									  case WM_INITDIALOG:
										  currFont = CreateFontIndirect(&Mfont);
										  SendMessage(GetDlgItem(hDlg,IDC_LIST1),WM_SETFONT,(WPARAM)currFont,TRUE);
										  SendMessage (GetDlgItem(hDlg,IDC_LIST1), LB_RESETCONTENT, 0, 0);
										  TextAnzahl = 0 ;                           
										  k=-1;
										  for ( i = 0; i < MAXPROGS ; i++ ) {
											  
											  if (( Programm[CurrentProgramm].tp_id == Programm[i].tp_id ) &&
												  ( Programm[CurrentProgramm].diseqc == Programm[i].diseqc ) &&
												  ( Programm[CurrentProgramm].freq == Programm[i].freq ) &&
												  ( Programm[CurrentProgramm].volt== Programm[i].volt) &&
												  ( Programm[i].TeleText_pid != 0x1fff )) {
												  TextPid[TextAnzahl].Pid=Programm[i].TeleText_pid;
												  strcpy(TextPid[TextAnzahl].Name,Programm[i].Name);
												  if ( Set_TextPid == TextPid[TextAnzahl].Pid ) k=TextAnzahl;
												  TextAnzahl++;
											  };
										  };
										  
										  
										  for ( i = 0; i < TextAnzahl; i++ ) {
											  sprintf(Zeile,"0x%04x   %s",TextPid[i].Pid,TextPid[i].Name);
											  SendMessage (GetDlgItem(hDlg,IDC_LIST1), LB_INSERTSTRING, i, (LPARAM)Zeile);
										  };
										  
										  SetDlgItemText(hDlg,IDC_EDIT1,Programm[CurrentProgramm].Name);
										  sprintf(Zeile,"0x%04x",Programm[CurrentProgramm].TeleText_pid);
										  SetDlgItemText(hDlg,IDC_EDIT2,Zeile);
										  if ( k >= 0 ) SendMessage (GetDlgItem(hDlg,IDC_LIST1), LB_SETCURSEL, k, 0);
										  return(TRUE);
										  
										  
										  
									  case WM_COMMAND:
										  
										  if ( LOWORD(wParam) == IDCANCEL) {
											  Stop_VT_Stream();
											  Sleep(5);Sleep(0);Sleep(0);Sleep(0);
											  Sleep(5);Sleep(0);Sleep(0);Sleep(0);
											  Sleep(5);Sleep(0);Sleep(0);Sleep(0);
											  Sleep(5);Sleep(0);Sleep(0);Sleep(0);
											  Sleep(5);Sleep(0);Sleep(0);Sleep(0);
											  VT_ChannelChange();	
											  VT_Stream_ID=Start_VT_Stream(Programm[CurrentProgramm].TeleText_pid);
											  EndDialog(hDlg,TRUE);
										  }
										  
										  if ( LOWORD(wParam) == IDOK ) {
											  i=SendMessage(GetDlgItem(hDlg,IDC_LIST1),LB_GETCURSEL,0,0);
											  if (( i >= 0 ) && (i < TextAnzahl )) {
												  Stop_VT_Stream();
												  Sleep(5);Sleep(0);Sleep(0);Sleep(0);
												  Sleep(5);Sleep(0);Sleep(0);Sleep(0);
												  Sleep(5);Sleep(0);Sleep(0);Sleep(0);
												  Sleep(5);Sleep(0);Sleep(0);Sleep(0);
												  Sleep(5);Sleep(0);Sleep(0);Sleep(0);
												  VT_ChannelChange();	
												  VT_Stream_ID=Start_VT_Stream(TextPid[i].Pid);
											  }
											  
											  
											  EndDialog(hDlg,TRUE);
										  }
										  
										  break;
									  }
									  return (FALSE);
									  UNREFERENCED_PARAMETER(lParam);
								  }
								  
								  
								  
								  BOOL APIENTRY StartZeitSettingProc(
									  HWND hDlg,
									  UINT message,
									  UINT wParam,
									  LONG lParam)
								  {
									  
									  struct CurrentText_t {
										  unsigned short Pid;
										  unsigned char Name[30];
									  };
									  
									  char Zeile[50];
									  int i;
									  static int TextAnzahl;
									  
									  LOGFONT Mfont = {9,5,0,0,0,0,0,0,0,0,0,0,0,"Fixedsys"};   
									  HFONT currFont;
									  
									  
									  switch (message) {
									  case WM_INITDIALOG:
										  
										  if (Is_Time_Link(Programm[CurrentProgramm].SID_pid, Programm[CurrentProgramm].tp_id,TRUE) == FALSE ) {
											  if ( ( Programm[CurrentProgramm].Merker&1) == 1 ) Programm[CurrentProgramm].Merker-=1;
											  MessageBox(hDlg,"Kein Time-Link","Multi-Dec",MB_ICONSTOP|MB_OK);
											  EnableMenuItem(GetMenu(hWnd),IDM_TIMELISTE,MF_GRAYED);
											  return(FALSE);
										  }
										  Zeile[0]=0x00;
										  GetCurrentNext(Programm[CurrentProgramm].SID_pid,Programm[CurrentProgramm].tp_id,2,&Zeile[0],NULL );
										  if ( Zeile[0] != 0x00 ) {
											  SetDlgItemText(hDlg,IDC_EDIT1,Zeile);
										  };
										  
										  
										  currFont = CreateFontIndirect(&Mfont);
										  SendMessage(GetDlgItem(hDlg,IDC_LIST1),WM_SETFONT,(WPARAM)currFont,TRUE);
										  SendMessage (GetDlgItem(hDlg,IDC_LIST1), LB_RESETCONTENT, 0, 0);
										  
										  for ( i = 0; i < 16; i++ ) {
											  if ( MultiLinkZeit[i].fill != 0x00 ) {
												  sprintf(Zeile,"%s (%s)",MultiLinkZeit[i].ExtraText,MultiLinkZeit[i].Name);
												  SendMessage (GetDlgItem(hDlg,IDC_LIST1), LB_INSERTSTRING, i, (LPARAM)Zeile);
											  };
										  };
										  
										  for ( i = 0; i < 16; i++ ) {
											  if ( MultiLinkZeit[i].fill == 0x02 ) SendMessage (GetDlgItem(hDlg,IDC_LIST1), LB_SETCURSEL, i, 0);
										  };
										  return(TRUE);
										  
										  
										  
									  case WM_COMMAND:
										  
										  if ( (HWND)lParam==GetDlgItem(hDlg,IDC_LIST1)) {
											  if (HIWORD(wParam)==LBN_SELCHANGE){
												  
												  i=SendMessage(GetDlgItem(hDlg,IDC_LIST1),LB_GETCURSEL,0,0);
												  if ( Set_DVB_Channel_by_SID_TS_ID(MultiLinkZeit[i].ts_id,MultiLinkZeit[i].SID,Programm[LastRealProg].Name) == FALSE ) {
													  MessageBox(hDlg,"TS-Daten nicht aufgelösst","Multi-Dec",MB_ICONSTOP|MB_OK);
												  };
												  
											  };
										  }
										  
										  
										  if ( LOWORD(wParam) == IDCANCEL) {
											  EndDialog(hDlg,TRUE);
										  }
										  
										  if ( LOWORD(wParam) == IDOK ) {
											  EndDialog(hDlg,TRUE);
										  }
										  
										  break;
									  }
									  return (FALSE);
									  UNREFERENCED_PARAMETER(lParam);
								  }
								  
								  
								  BOOL APIENTRY MultiViewSettingProc(
									  HWND hDlg,
									  UINT message,
									  UINT wParam,
									  LONG lParam)
								  {
									  
									  struct CurrentText_t {
										  unsigned short Pid;
										  unsigned char Name[30];
									  };
									  
									  char Zeile[50];
									  int i;
									  static int TextAnzahl;
									  
									  LOGFONT Mfont = {9,5,0,0,0,0,0,0,0,0,0,0,0,"Fixedsys"};   
									  HFONT currFont;
									  
									  
									  switch (message) {
									  case WM_INITDIALOG:
										  
										  if (Is_View_Link(Programm[CurrentProgramm].SID_pid, Programm[CurrentProgramm].tp_id,TRUE) == FALSE ) {
											  if ( ( Programm[CurrentProgramm].Merker&2) == 2 ) Programm[CurrentProgramm].Merker-=2;
											  MessageBox(hDlg,"Keine Extra-Kanäle","Multi-Dec",MB_ICONSTOP|MB_OK);
											  EnableMenuItem(GetMenu(hWnd),IDM_TIMELISTE,MF_GRAYED);
											  return(FALSE);
										  }
										  Zeile[0]=0x00;
										  GetCurrentNext(Programm[CurrentProgramm].SID_pid,Programm[CurrentProgramm].tp_id,2,&Zeile[0],NULL );
										  if ( Zeile[0] != 0x00 ) {
											  SetDlgItemText(hDlg,IDC_EDIT1,Zeile);
										  };
										  
										  
										  currFont = CreateFontIndirect(&Mfont);
										  SendMessage(GetDlgItem(hDlg,IDC_LIST1),WM_SETFONT,(WPARAM)currFont,TRUE);
										  SendMessage (GetDlgItem(hDlg,IDC_LIST1), LB_RESETCONTENT, 0, 0);
										  
										  for ( i = 0; i < 16; i++ ) {
											  if ( MultiLinkView[i].fill != 0x00 ) {
												  sprintf(Zeile,"%-12s (%s)",MultiLinkView[i].ExtraText,MultiLinkView[i].Name);
												  SendMessage (GetDlgItem(hDlg,IDC_LIST1), LB_INSERTSTRING, i, (LPARAM)Zeile);
											  };
										  };
										  
										  for ( i = 0; i < 16; i++ ) {
											  if ( MultiLinkView[i].fill == 0x02 ) SendMessage (GetDlgItem(hDlg,IDC_LIST1), LB_SETCURSEL, i, 0);
										  };
										  return(TRUE);
										  
										  
										  
									  case WM_COMMAND:
										  
										  if ( (HWND)lParam==GetDlgItem(hDlg,IDC_LIST1)) {
											  if (HIWORD(wParam)==LBN_SELCHANGE){
												  
												  i=SendMessage(GetDlgItem(hDlg,IDC_LIST1),LB_GETCURSEL,0,0);
												  Relink_Current_Eit(Programm[LastRealProg].SID_pid,Programm[LastRealProg].tp_id,MultiLinkView[i].SID,MultiLinkView[i].ts_id);
												  if ( Set_DVB_Channel_by_SID_TS_ID(MultiLinkView[i].ts_id,MultiLinkView[i].SID,MultiLinkView[i].ExtraText) == FALSE ) {
													  MessageBox(hDlg,"TS-Daten nicht aufgelösst","Multi-Dec",MB_ICONSTOP|MB_OK);
												  };
												  
											  };
										  }
										  
										  
										  if ( LOWORD(wParam) == IDCANCEL) {
											  EndDialog(hDlg,TRUE);
										  }
										  
										  if ( LOWORD(wParam) == IDOK ) {
											  EndDialog(hDlg,TRUE);
										  }
										  
										  break;
									  }
									  return (FALSE);
									  UNREFERENCED_PARAMETER(lParam);
								  }
								  
								  
								  
								  BOOL APIENTRY DigiImport(
									  HWND hDlg,
									  UINT message,
									  UINT wParam,
									  LONG lParam)
								  {
									  OPENFILENAME OpenFileName;
									  CHAR         szDirName[256]   = "";
									  CHAR         szFile[256]      = "\0";
									  CHAR         szFileTitle[256];
									  CHAR szFilter[] = "DVB-Datei (*.dvb)\0*.dvb\0\0";
									  
									  struct DVBImport
									  {
										  unsigned char Buffer1[4];
										  unsigned short SID;
										  unsigned short PMT;
										  unsigned short Freq;
										  unsigned short Symb;
										  unsigned char   FEC;
										  unsigned char   BW;
										  unsigned char   POL;
										  unsigned char   Diseq; // Bit 0/1  SpecOff == 4 SpecOn == 8 SpecAuto == 0
										  unsigned short  Video_pid;
										  unsigned short  Audio_pid;
										  unsigned short  PCR_pid;
										  unsigned char  Filler1[2];
										  unsigned short  PMC;
										  unsigned char   TVType; //  == 00 PAL ; 11 == NTSC    
										  unsigned char   ServiceTyp;   
										  unsigned short  Text_pid;
										  unsigned short  TS_Id;
										  unsigned char   Name[20];
										  unsigned char   Buffer[12];
									  };
									  
									  struct DVBImport Import;
									  
									  int i,j,k,l;
									  int fd;
									  char CurrentDir[255];
									  char Datei[255];
									  char Text[255];
									  
									  GetCurrentDirectory(sizeof(CurrentDir),CurrentDir);
									  
									  switch (message) {
									  case WM_INITDIALOG:
										  if ( VES_TYPE == 0 ) CheckDlgButton(hDlg,IDC_CHECK2,TRUE);
										  else CheckDlgButton(hDlg,IDC_CHECK2,FALSE);
										  
										  
										  break;
										  
									  case WM_COMMAND:
										  
										  if (LOWORD(wParam) == IDC_BUTTON1 ) {
											  strcpy(szFilter,"DVB-Datei (*.dvb)\0*.dvb\0");
											  OpenFileName.lStructSize       = sizeof(OPENFILENAME);
											  OpenFileName.hwndOwner         = hDlg;
											  OpenFileName.hInstance         = (HANDLE) hInst;
											  OpenFileName.lpstrFilter       = szFilter;
											  OpenFileName.lpstrCustomFilter = (LPSTR) NULL;
											  OpenFileName.nMaxCustFilter    = 0L;
											  OpenFileName.nFilterIndex      = 0L;
											  OpenFileName.lpstrFile         = szFile;
											  OpenFileName.nMaxFile          = sizeof(szFile);
											  OpenFileName.lpstrFileTitle    = szFileTitle;
											  OpenFileName.nMaxFileTitle     = sizeof(szFileTitle);
											  OpenFileName.lpstrInitialDir   = (LPCTSTR)CurrentDir;
											  OpenFileName.lpstrTitle        = "DVB-Dateien importieren";
											  OpenFileName.nFileOffset       = 0;
											  OpenFileName.nFileExtension    = 0;
											  OpenFileName.lpstrDefExt       = (LPCTSTR)"*.dvb";
											  OpenFileName.lCustData         = 0;
											  OpenFileName.Flags = OFN_SHOWHELP | OFN_PATHMUSTEXIST |
												  OFN_HIDEREADONLY | OFN_NONETWORKBUTTON;
											  
											  
											  if (!GetOpenFileName(&OpenFileName)) return(FALSE);
											  SetDlgItemText(hDlg,IDC_EDIT1,OpenFileName.lpstrFile);
											  SetCurrentDirectory(CurrentDir);
										  };
										  
										  
										  
										  
										  if (LOWORD(wParam) == IDOK ) {
											  
											  GetDlgItemText(hDlg,IDC_EDIT1,Datei,sizeof(Datei));
											  
											  if ((fd=open(Datei,_O_RDONLY | _O_BINARY )) != -1 ) {
												  
												  if ( IsDlgButtonChecked(hDlg, IDC_CHECK1)) {
													  i=0;
													  k=0;
													  while ( i < MAXPROGS ) {
														  if ( Programm[i].Typ == 'D' ) {
															  k++;
															  sprintf(Text,"Lösche %d    ",k);
															  SetDlgItemText(hDlg,IDC_TEXT,Text);
															  j=i;
															  while ( j < MAXPROGS-1 ) {
																  memcpy(&Programm[j],&Programm[j+1],sizeof(struct TProgramm));
																  j++;
															  };
															  Programm[j].freq = 0;
															  Programm[j].Name[0] = 0;
															  
														  } else i++;
													  };
												  };
												  
												  k=0;
												  
												  while ( sizeof(struct DVBImport) == read(fd,&Import,sizeof(struct DVBImport)) ) {
													  
													  if ( Import.Freq != 0 ) {
														  i=0;
														  while ( i < MAXPROGS ) {
															  if (( Programm[i].freq == 0 ) && (Programm[i].Name[0]==0x00 )) break;
															  i++;
														  }
														  if ( i>= MAXPROGS ) {
															  MessageBox(hDlg,"Alle Speicherplätze voll","Multi-Dec",MB_ICONSTOP|MB_OK);
															  close(fd);
															  EndDialog(hDlg,TRUE);
														  };
														  
														  k++;
														  sprintf(Text,"Importiert %d    ",k);
														  SetDlgItemText(hDlg,IDC_TEXT,Text);
														  j=0;
														  l=0;
														  while ( j < (signed)strlen(Import.Name) ) {
															  if (( Import.Name[j] >= 0x20 ) && ( Import.Name[j] < 0x86 )) Programm[i].Name[l++]=Import.Name[j];
															  Programm[i].Name[l]=0x00;
															  j++;
														  };
														  Programm[i].Merker=0x00;
														  Programm[i].Link_SID=0xffff;
														  Programm[i].Link_TP=0xffff;
														  Programm[i].Dynamisch=0x00;
														  Programm[i].Name[19]=0x00;
														  Programm[i].CA_ID=0x00;
														  Programm[i].CA_Anzahl=0;
														  Programm[i].CA_Land[0]=0x00;
														  Programm[i].Filteranzahl=0;
														  Programm[i].Typ='D';
														  Programm[i].freq =   Swab16((unsigned char *)&Import.Freq) * 1000  ;
														  Programm[i].srate =  Swab16((unsigned char *)&Import.Symb)  ;
														  
														  
														  if ( Import.Filler1[0] == 0x01 ) { 
															  Programm[i].CA_ID=0x01;
														  };
														  
														  if ( Import.FEC == 1 ) Programm[i].fec=8;         // 1/1
														  else if ( Import.FEC == 2 ) Programm[i].fec=0;    // 1/2
														  else if ( Import.FEC == 3 ) Programm[i].fec=1;    // 2/3
														  else if ( Import.FEC == 4 ) Programm[i].fec=2;    // 3/4
														  else if ( Import.FEC == 5 ) Programm[i].fec=3;    // ????
														  else if ( Import.FEC == 6 ) Programm[i].fec=4;    // 5/6
														  else if ( Import.FEC == 7 ) Programm[i].fec=5;    // 7/8
														  else if ( Import.FEC == 0 ) Programm[i].fec=8;    // AUTO
														  Programm[i].volt = !Import.POL  ;
														  Programm[i].diseqc = Import.Diseq & 3 ;
														  Programm[i].Video_pid =  Swab16((unsigned char *)&Import.Video_pid);
														  Programm[i].Audio_pid =  Swab16((unsigned char *)&Import.Audio_pid);
														  Programm[i].PCR_pid =  Swab16((unsigned char *)&Import.PCR_pid) ;
														  Programm[i].ECM_PID =  Swab16((unsigned char *)&Import.PMC ) ;
														  Programm[i].PMT_pid = Swab16((unsigned char *)&Import.PMT);
														  Programm[i].TVType = Import.TVType & 3  ;
														  Programm[i].TeleText_pid =  Swab16((unsigned char *)&Import.Text_pid);
														  Programm[i].tp_id=  Swab16((unsigned char *)&Import.TS_Id);
														  Programm[i].ServiceTyp = Import.ServiceTyp;
														  Programm[i].SID_pid=Swab16((unsigned char *)&Import.SID);
														  
														  if ( IsDlgButtonChecked(hDlg, IDC_CHECK2) == TRUE ) {
															  Programm[i].freq = Programm[i].freq*100;
															  if ( Import.FEC == 1 ) Programm[i].qam=2;         // 1/1
															  else if ( Import.FEC == 2 ) Programm[i].qam=0;    // 1/2
															  else if ( Import.FEC == 3 ) Programm[i].qam=1;    // 2/3
															  else if ( Import.FEC == 4 ) Programm[i].qam=2;    // 3/4
															  else if ( Import.FEC == 5 ) Programm[i].qam=3;    // ????
															  else if ( Import.FEC == 6 ) Programm[i].qam=4;    // 5/6
															  else if ( Import.FEC == 7 ) Programm[i].qam=2;    // 7/8
															  else if ( Import.FEC == 0 ) Programm[i].qam=2;    // AUTO
														  };
													  }
												  };
											  } else {
												  MessageBox(hDlg," Datei nicht gefunden ","Multi-Dec",MB_ICONSTOP|MB_OK);
												  break;
											  };
											  
											  Write_Programm_List();
											  Write_Nit_List();
											  EndDialog(hDlg,TRUE);
											  
		 }
		 if (LOWORD(wParam) == IDCANCEL ) {
			 EndDialog(hDlg,TRUE);
		 }
		 
		 
		 break;
    }
    return (FALSE);
	UNREFERENCED_PARAMETER(lParam);
}


BOOL APIENTRY SRateSettingProc
(
 HWND hDlg,
 UINT message,
 UINT wParam,
 LONG lParam)
{
	int i;
	
	switch (message) {
	case WM_INITDIALOG:
		for ( i=0; i < 16 ; i++ ) SetDlgItemInt(hDlg,IDC_EDIT1+i,SRates[i],FALSE);
		break;
		
		
	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK ) {
			for ( i=0; i < 16 ; i++ ) SRates[i]=GetDlgItemInt(hDlg,IDC_EDIT1+i,NULL,FALSE);
			EndDialog(hDlg,TRUE);
		}
		
		
		if (LOWORD(wParam) == IDCANCEL ) {
			EndDialog(hDlg,TRUE);
		}
		
		break;
    }
	
	return(FALSE);
}


BOOL APIENTRY DVBTimeSettingProc
(
 HWND hDlg,
 UINT message,
 UINT wParam,
 LONG lParam)
{
	char Zeile[128];
	switch (message) {
	case WM_INITDIALOG:
		SetDlgItemInt(hDlg,IDC_EDIT3,TIMEZONE,TRUE);
		SetTimer(hDlg,0,1000,NULL);
		
	case WM_TIMER: 
		sprintf(Zeile,"%d",TDT.Date_Id);
		SetDlgItemText(hDlg,IDC_EDIT1,Zeile);
		Date_To_String((unsigned short)RealDay(TDT.Hour,TDT.Date_Id),Zeile);
		SetDlgItemText(hDlg,IDC_EDIT5,Zeile);
		sprintf(Zeile,"%02d:%02d:%02d",TDT.RefreshHour,TDT.RefreshMin,TDT.RefreshSec);
		SetDlgItemText(hDlg,IDC_EDIT2,Zeile);
		sprintf(Zeile,"%02d:%02d:%02d",RealTime(TDT.Hour),TDT.Min,TDT.Sec);
		SetDlgItemText(hDlg,IDC_EDIT4,Zeile);
		break;
		
		
	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK ) {
			TIMEZONE=GetDlgItemInt(hDlg,IDC_EDIT3,NULL,TRUE);
			EndDialog(hDlg,TRUE);
		}
		
		
		if (LOWORD(wParam) == IDCANCEL ) {
			EndDialog(hDlg,TRUE);
		}
		
		break;
    }
	
	return(FALSE);
}



BOOL APIENTRY VideoSettingProc
(
 HWND hDlg,
 UINT message,
 UINT wParam,
 LONG lParam)
{
	int x,y,i,j;
	
	switch (message) {
	case WM_INITDIALOG:
		
		TBrightness=InitialBrightness;
		TContrast=InitialContrast;
		THue=InitialHue;
		TSaturationU=InitialSaturationU;
		TSaturationV=InitialSaturationV;
		if ( TBrightness < 128 ) i=TBrightness;
		else i=-(256-TBrightness);
		SetDlgItemInt(hDlg,IDC_D1,i,TRUE);
		SetDlgItemInt(hDlg,IDC_D2,TContrast,FALSE);
		if ( THue < 128 ) i=THue;
		else i=-(256-THue);
		SetDlgItemInt(hDlg,IDC_D3,i,TRUE);
		LastSaturation = (TSaturationU+TSaturationV)/2; 
		SetDlgItemInt(hDlg,IDC_D4,LastSaturation,FALSE);
		SetDlgItemInt(hDlg,IDC_D5,TSaturationU,FALSE);
		SetDlgItemInt(hDlg,IDC_D6,TSaturationV,FALSE);
		
		if ( TBrightness < 128 ) i=TBrightness;
		else i=-(256-TBrightness);
		y=(int)(157-((double)i/256*160));
		MoveWindow(GetDlgItem(hDlg,IDC_S1),33,y-4,22,8,FALSE);
		
		y=(int)(235-((double)TContrast/256*160));
		MoveWindow(GetDlgItem(hDlg,IDC_S2),109,y-4,22,8,FALSE);
		
		if ( THue < 128 ) i=THue;
		else i=-(256-THue);
		y=(int)(157-((double)i/256*160));
		MoveWindow(GetDlgItem(hDlg,IDC_S3),183,y-4,22,8,FALSE);
		
		y=(int)(235-((double)LastSaturation/256*160));
		MoveWindow(GetDlgItem(hDlg,IDC_S4),258,y-4,22,8,FALSE);
		
		y=(int)(235-((double)TSaturationU/256*160));
		MoveWindow(GetDlgItem(hDlg,IDC_S5),334,y-4,22,8,FALSE);
		
		y=(int)(235-((double)TSaturationV/256*160));
		MoveWindow(GetDlgItem(hDlg,IDC_S6),410,y-4,22,8,FALSE);
		
		
		break;
		
		
		
	case WM_MOUSEMOVE:
		
		y=HIWORD( lParam );
		x=LOWORD( lParam );
		if (wParam == MK_LBUTTON) {
			if (( y >= 70) && ( y <= 240 )) {
				if (( x >= 33) && ( x <= 55 )) {
					i=(int)((double)(y-155)/160*255);
					if ( i < -127) i=-127;
					else if ( i > 127 ) i=127;
					i=-i;
					MoveWindow(GetDlgItem(hDlg,IDC_S1),33,y-2,22,8,TRUE);
					TBrightness=i;
					if ( TBrightness < 128 ) i=TBrightness;
			                 else i=-(256-TBrightness);
							 SetDlgItemInt(hDlg,IDC_D1,i,TRUE);
							 SetBrightness(TBrightness);
				}
				if (( x >= 109) && ( x <= 131 )) {
					i=(int)((double)(y-235)/160*255);
					i=-i;
					if ( i < 0) i=0;
					else if ( i > 255 ) i=255;
					MoveWindow(GetDlgItem(hDlg,IDC_S2),109,y-2,22,8,TRUE);
					TContrast=i;
					SetDlgItemInt(hDlg,IDC_D2,TContrast,FALSE);
					SetContrast(TContrast);
				}
				
				
				if (( x >= 183) && ( x <= 205 )) {
					i=(int)((double)(y-155)/160*255);
					if ( i < -127) i=-127;
					else if ( i > 127 ) i=127;
					i=-i;
					MoveWindow(GetDlgItem(hDlg,IDC_S3),183,y-2,22,8,TRUE);
					THue=i;
					if ( THue < 128 ) i=THue;
                 			else i=-(256-THue);
							SetDlgItemInt(hDlg,IDC_D3,i,TRUE);
							SetHue(THue);
				}
				if (( x >= 258) && ( x <= 280 )) {
					i=(int)((double)(y-235)/160*255);
					i=-i;
					if ( i < 0) i=0;
					else if ( i > 255 ) i=255;
					j=i-LastSaturation;
					
					if (( j+TSaturationV <= 255) && 
						( j+TSaturationU <= 255 ) && 
						( j+TSaturationV >= 0 ) &&
						( j+TSaturationU >= 0 )) {
						
						MoveWindow(GetDlgItem(hDlg,IDC_S4),258,y-2,22,8,TRUE);
						TSaturationU+=j;
						y=(int)(235-((double)TSaturationU/256*160));
						MoveWindow(GetDlgItem(hDlg,IDC_S5),334,y-2,22,8,TRUE);
						TSaturationV+=j;
						y=(int)(235-((double)TSaturationV/256*160));
						MoveWindow(GetDlgItem(hDlg,IDC_S6),410,y-2,22,8,TRUE);
						LastSaturation = (TSaturationU+TSaturationV)/2; 
						SetDlgItemInt(hDlg,IDC_D5,TSaturationU,FALSE);
						SetDlgItemInt(hDlg,IDC_D6,TSaturationV,FALSE);
						SetDlgItemInt(hDlg,IDC_D4,LastSaturation,FALSE);
						SetSaturationU(TSaturationU);
						SetSaturationV(TSaturationV);
					}
				}
				if (( x >= 334 ) && ( x <= 356 )) {
					i=(int)((double)(y-235)/160*255);
					i=-i;
					if ( i < 0) i=0;
					else if ( i > 255 ) i=255;
					MoveWindow(GetDlgItem(hDlg,IDC_S5),334,y-2,22,8,TRUE);
					TSaturationU=i;
                		  LastSaturation = (TSaturationU+TSaturationV)/2; 
						  SetDlgItemInt(hDlg,IDC_D4,LastSaturation,FALSE);
						  y=(int)(235-((double)LastSaturation/256*160));
						  MoveWindow(GetDlgItem(hDlg,IDC_S4),258,y-4,22,8,TRUE);
						  
						  SetDlgItemInt(hDlg,IDC_D5,TSaturationU,FALSE);
						  SetSaturationU(TSaturationU);
				}
				if (( x >= 410 ) && ( x <= 432 )) {
					i=(int)((double)(y-235)/160*255);
					i=-i;
					if ( i < 0) i=0;
					else if ( i > 255 ) i=255;
					MoveWindow(GetDlgItem(hDlg,IDC_S6),410,y-2,22,8,TRUE);
					TSaturationV=i;
                		  LastSaturation = (TSaturationU+TSaturationV)/2; 
						  SetDlgItemInt(hDlg,IDC_D4,LastSaturation,FALSE);
						  y=(int)(235-((double)LastSaturation/256*160));
						  MoveWindow(GetDlgItem(hDlg,IDC_S4),258,y-4,22,8,TRUE);
						  
						  SetDlgItemInt(hDlg,IDC_D6,TSaturationV,FALSE);
						  SetSaturationV(TSaturationV);
				}
				
				
			}
		}
		return(FALSE);
        case WM_COMMAND:
			if (LOWORD(wParam) == IDOK ) {
				InitialBrightness=TBrightness;
				InitialContrast=TContrast;
				InitialHue=THue;
				InitialSaturationU=TSaturationU;
				InitialSaturationV=TSaturationV;
				EndDialog(hDlg,TRUE);
			}
			
			
			if (LOWORD(wParam) == IDCANCEL ) {
				SetBrightness(InitialBrightness);
				SetContrast(InitialContrast);
				SetHue(InitialHue);
				SetSaturationU(InitialSaturationU);
				SetSaturationV(InitialSaturationV);
				
				EndDialog(hDlg,TRUE);
			}
			
			if ( LOWORD(wParam) == IDDEFAULT) {
				THue=0x00;
				TBrightness=0x00;			   
				TContrast=0xd8;	
				TSaturationU=0xfe;			   
				TSaturationV=0xb4;			   
				SetBrightness(TBrightness);
				SetContrast(TContrast);
				SetHue(THue);
				SetSaturationU(TSaturationU);
				SetSaturationV(TSaturationV);
				if ( TBrightness < 128 ) i=TBrightness;
             			else i=-(256-TBrightness);
						SetDlgItemInt(hDlg,IDC_D1,i,TRUE);
						SetDlgItemInt(hDlg,IDC_D2,TContrast,FALSE);
						if ( THue < 128 ) i=THue;
						else i=-(256-THue);
						SetDlgItemInt(hDlg,IDC_D3,i,TRUE);
						SetDlgItemInt(hDlg,IDC_D4,((TSaturationU+TSaturationV)/2),FALSE);
						SetDlgItemInt(hDlg,IDC_D5,TSaturationU,FALSE);
						SetDlgItemInt(hDlg,IDC_D6,TSaturationV,FALSE);
						y=(int)(157-((double)TBrightness/256*160));
						MoveWindow(GetDlgItem(hDlg,IDC_S1),33,y-4,22,8,TRUE);
						y=(int)(235-((double)TContrast/256*160));
						MoveWindow(GetDlgItem(hDlg,IDC_S2),109,y-4,22,8,TRUE);
						y=(int)(157-((double)THue/256*160));
						MoveWindow(GetDlgItem(hDlg,IDC_S3),183,y-4,22,8,TRUE);
						y=(int)(235-((double)((TSaturationU+TSaturationV)/2)/256*160));
						MoveWindow(GetDlgItem(hDlg,IDC_S4),258,y-4,22,8,TRUE);
						y=(int)(235-((double)TSaturationU/256*160));
						MoveWindow(GetDlgItem(hDlg,IDC_S5),334,y-4,22,8,TRUE);
						y=(int)(235-((double)TSaturationV/256*160));
						MoveWindow(GetDlgItem(hDlg,IDC_S6),410,y-4,22,8,TRUE);
						break;
						
			}
			
			break;
    }
	
	return(FALSE);
}


BOOL APIENTRY VideoSettingDigiProc
(
 HWND hDlg,
 UINT message,
 UINT wParam,
 LONG lParam)
{
	int x,y,i;
	
	switch (message) {
	case WM_INITDIALOG:
		
		TBrightness=InitialDVBBrightness;
		TContrast=InitialDVBContrast;
		TColor=InitialDVBColor;
		SetDlgItemInt(hDlg,IDC_D1,TBrightness,FALSE);
		SetDlgItemInt(hDlg,IDC_D2,TContrast,FALSE);
		SetDlgItemInt(hDlg,IDC_D3,TColor,FALSE);
		y=(int)(235-((double)TBrightness/256*160));
		MoveWindow(GetDlgItem(hDlg,IDC_S1),33,y-4,22,8,FALSE);
		
		y=(int)(235-((double)TContrast/128*160));
		MoveWindow(GetDlgItem(hDlg,IDC_S2),109,y-4,22,8,FALSE);
		
		y=(int)(235-((double)TColor/128*160));
		MoveWindow(GetDlgItem(hDlg,IDC_S3),183,y-4,22,8,FALSE);
		break;
		
		
		
	case WM_MOUSEMOVE:
		
		y=HIWORD( lParam );
		x=LOWORD( lParam );
		if (wParam == MK_LBUTTON) {
			if (( y >= 70) && ( y <= 240 )) {
				if (( x >= 33) && ( x <= 55 )) {
					i=(int)((double)(y-235)/160*255);
					i=-i;
					if ( i < 0) i=0;
					else if ( i > 255 ) i=255;
					MoveWindow(GetDlgItem(hDlg,IDC_S1),33,y-2,22,8,TRUE);
					TBrightness=i;
					SetDlgItemInt(hDlg,IDC_D1,i,TRUE);
					set_picture_prop(TBrightness,TContrast,TColor);
				}
				if (( x >= 109) && ( x <= 131 )) {
					i=(int)((double)(y-235)/160*127);
					i=-i;
					if ( i < 0) i=0;
					else if ( i > 127 ) i=127;
					MoveWindow(GetDlgItem(hDlg,IDC_S2),109,y-2,22,8,TRUE);
					TContrast=i;
					SetDlgItemInt(hDlg,IDC_D2,TContrast,FALSE);
					set_picture_prop(TBrightness,TContrast,TColor);
				}
				
				
				if (( x >= 183) && ( x <= 205 )) {
					i=(int)((double)(y-235)/160*127);
					i=-i;
					if ( i < 0) i=0;
					else if ( i > 127) i=127;
					MoveWindow(GetDlgItem(hDlg,IDC_S3),183,y-2,22,8,TRUE);
					TColor=i;
					SetDlgItemInt(hDlg,IDC_D3,TColor,TRUE);
					set_picture_prop(TBrightness,TContrast,TColor);
				}
				
				
			}
		}
		return(FALSE);
	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK ) {
			InitialDVBBrightness=TBrightness;
			InitialDVBContrast=TContrast;
			InitialDVBColor=TColor;
			EndDialog(hDlg,TRUE);
		}
		
		
		if (LOWORD(wParam) == IDCANCEL ) {
			set_picture_prop(InitialDVBBrightness,InitialDVBContrast,InitialDVBColor);
			EndDialog(hDlg,TRUE);
		}
		
		if ( LOWORD(wParam) == IDDEFAULT) {
			TBrightness=0x80;
			TContrast=0x40;			   
			TColor=0x40;	
			set_picture_prop(InitialDVBBrightness,InitialDVBContrast,InitialDVBColor);
			SetDlgItemInt(hDlg,IDC_D1,TBrightness,FALSE);
			SetDlgItemInt(hDlg,IDC_D2,TContrast,FALSE);
			SetDlgItemInt(hDlg,IDC_D3,TColor,FALSE);
			y=(int)(235-((double)TBrightness/256*160));
			MoveWindow(GetDlgItem(hDlg,IDC_S1),33,y-4,22,8,TRUE);
			
			y=(int)(235-((double)TContrast/128*160));
			MoveWindow(GetDlgItem(hDlg,IDC_S2),109,y-4,22,8,TRUE);
			
			y=(int)(235-((double)TColor/128*160));
			MoveWindow(GetDlgItem(hDlg,IDC_S3),183,y-4,22,8,TRUE);
			break;
		}
		
		break;
    }
	
	return(FALSE);
}




BOOL APIENTRY AudioSettingProc
(
 HWND hDlg,
 UINT message,
 UINT wParam,
 LONG lParam)
{
	int x,y,i;
	
	switch (message) {
	case WM_INITDIALOG:
		if ( Has_MSP == FALSE ) {
			MessageBox(hWnd,"Kein MSP-Audio-Device gefunden","Multi-Decode",MB_ICONSTOP|MB_OK);
			EndDialog(hDlg,0);
		}
		
		TVolume=InitialVolume;
		TSpecial=InitialSpecial;
		TLoudness=InitialLoudness;
		TBass=InitialBass;
		TTreble=InitialTreble;
		TBalance=InitialBalance;
		TSuperBass=InitialSuperBass;
		
		
		SetDlgItemInt(hDlg,IDC_D1,TVolume,FALSE);
		SetDlgItemInt(hDlg,IDC_D2,TSpecial,TRUE);
		SetDlgItemInt(hDlg,IDC_D3,TLoudness,TRUE);
		SetDlgItemInt(hDlg,IDC_D4,TBass,TRUE);
		SetDlgItemInt(hDlg,IDC_D5,TTreble,TRUE);
		SetDlgItemInt(hDlg,IDC_D6,TBalance,TRUE);
		
		CheckDlgButton(hDlg, IDC_CHECK1, TSuperBass);
		
		y=(int)(235-((double)(TVolume+1)/1000*160));
		MoveWindow(GetDlgItem(hDlg,IDC_S1),33,y-4,22,8,FALSE);
		
		y=(int)(157-((double)(TSpecial+1)/256*160));
		MoveWindow(GetDlgItem(hDlg,IDC_S2),109,y-4,22,8,FALSE);
		
		y=(int)(235-((TLoudness+1)/68*160));
		MoveWindow(GetDlgItem(hDlg,IDC_S3),183,y-4,22,8,FALSE);
		
		y=(int)(157-((double)(TBass+1)/256*160));
		MoveWindow(GetDlgItem(hDlg,IDC_S4),258,y-4,22,8,FALSE);
		
		y=(int)(157-((double)(TTreble+1)/256*160));
		MoveWindow(GetDlgItem(hDlg,IDC_S5),334,y-4,22,8,FALSE);
		
		x=(int)(279-((double)(TBalance+1)/256*160));
		MoveWindow(GetDlgItem(hDlg,IDC_S6),x-4,271,8,22,FALSE);
		
		
		break;
		
		
		
	case WM_MOUSEMOVE:
		
		y=HIWORD( lParam );
		x=LOWORD( lParam );
		if (wParam == MK_LBUTTON) {
			if (( y >= 70) && ( y <= 240 )) {
				if (( x >= 33) && ( x <= 55 )) {
					i=(int)((double)(y-235)/160*1000);
					i=-i;
					if ( i < 0 ) i=0;
					else if ( i > 1000 ) i=1000;
					MoveWindow(GetDlgItem(hDlg,IDC_S1),33,y-2,22,8,TRUE);
					InitialVolume=i;
					SetDlgItemInt(hDlg,IDC_D1,InitialVolume,FALSE);
					Audio_SetVolume(InitialVolume);
				}
				if (( x >= 109) && ( x <= 131 )) {
					i=(int)((double)(y-155)/160*255);
					if ( i < -127) i=-127;
					else if ( i > 128 ) i=128;
					i=-i;
					MoveWindow(GetDlgItem(hDlg,IDC_S2),109,y-2,22,8,TRUE);
					InitialSpecial=i;
					SetDlgItemInt(hDlg,IDC_D2,InitialSpecial,TRUE);
					Audio_SetSpatial(InitialSpecial); 
				}
				
				
				if (( x >= 183) && ( x <= 205 )) {
					i=(int)((double)(y-235)/160*68);
					i=-i;
					if ( i < 0 ) i=0;
					else if ( i > 68 ) i=68;
					MoveWindow(GetDlgItem(hDlg,IDC_S3),183,y-2,22,8,TRUE);
					InitialLoudness=i;
					SetDlgItemInt(hDlg,IDC_D3,InitialLoudness,FALSE);
					Audio_SetLoudness(InitialLoudness);
				}
				if (( x >= 258) && ( x <= 280 )) {
					i=(int)((double)(y-155)/160*255);
					i=-i;
					if ( i < -96) i=-96;
					else if ( i > 127 ) i=127;
					MoveWindow(GetDlgItem(hDlg,IDC_S4),258,y-2,22,8,TRUE);
					InitialBass=i;
					SetDlgItemInt(hDlg,IDC_D4,InitialBass,TRUE);
					Audio_SetBass(InitialBass);
				}
				if (( x >= 334 ) && ( x <= 356 )) {
					i=(int)((double)(y-155)/160*255);
					i=-i;
					if ( i < -96) i=-96;
					else if ( i > 127 ) i=127;
					MoveWindow(GetDlgItem(hDlg,IDC_S5),334,y-2,22,8,TRUE);
					InitialTreble=i;
					SetDlgItemInt(hDlg,IDC_D5,InitialTreble,TRUE);
					Audio_SetTreble(InitialTreble);
				}
				
				
			}
			if (( y >= 271 ) && ( y <= 293 ) && ( x >= 195 ) && ( x <= 360 )) {
				
				i=(int)((double)(x-277)/160*255);
				i=-i;
				if ( i < -127) i=-127;
				else if ( i > 127 ) i=127;
				InitialBalance=i;
				SetDlgItemInt(hDlg,IDC_D6,InitialBalance,TRUE);
             			x=(int)(279-((double)(InitialBalance+1)/256*160));
						MoveWindow(GetDlgItem(hDlg,IDC_S6),x-4,271,8,22,TRUE);
						Audio_SetBalance(InitialBalance); // -127 - +128
						
			}
			
		}
		return(FALSE);
	case WM_COMMAND:
		if ( (HWND)lParam==GetDlgItem(hDlg,IDC_CHECK1) ) Audio_SetSuperBass(IsDlgButtonChecked(hDlg, IDC_CHECK1));
		
		if (LOWORD(wParam) == IDOK ) {
			
			SetAudioSource(4);
			Audio_SetSuperBass(IsDlgButtonChecked(hDlg, IDC_CHECK1));
			Audio_SetVolume(InitialVolume);
			Audio_SetSpatial(InitialSpecial); 
			Audio_SetLoudness(InitialLoudness);
			Audio_SetBass(InitialBass);
			Audio_SetTreble(InitialTreble);
			Audio_SetBalance(InitialBalance); 
			
			SetAudioSource(AudioSource);
			EndDialog(hDlg,TRUE);
		}
		
		
		if (LOWORD(wParam) == IDCANCEL ) {
			SetAudioSource(4);
			Audio_SetSuperBass(TSuperBass);
			Audio_SetVolume(TVolume);
			Audio_SetSpatial(TSpecial); 
			Audio_SetLoudness(TLoudness);
			Audio_SetBass(TBass);
			Audio_SetTreble(TTreble);
			Audio_SetBalance(TBalance); 
			SetAudioSource(AudioSource);
			EndDialog(hDlg,TRUE);
		}
		
		
		
		if ( LOWORD(wParam) == IDDEFAULT) {
			
			TVolume=1000;
			TBalance=0x00;
			TSpecial=0x00;
			TLoudness=0x00;
			TBass=0x00;
			TTreble=0x00;
			CheckDlgButton(hDlg, IDC_CHECK1, FALSE);
			SetAudioSource(4);
			Audio_SetSuperBass(FALSE);
			Audio_SetVolume(TVolume);
			Audio_SetSpatial(TSpecial); 
			Audio_SetLoudness(TLoudness);
			Audio_SetBass(TBass);
			Audio_SetTreble(TTreble);
			Audio_SetBalance(TBalance); 
			
			SetDlgItemInt(hDlg,IDC_D1,TVolume,FALSE);
			SetDlgItemInt(hDlg,IDC_D2,TSpecial,TRUE);
			SetDlgItemInt(hDlg,IDC_D3,TLoudness,TRUE);
			SetDlgItemInt(hDlg,IDC_D4,TBass,TRUE);
			SetDlgItemInt(hDlg,IDC_D5,TTreble,TRUE);
			SetDlgItemInt(hDlg,IDC_D6,TBalance,TRUE);
            SetAudioSource(AudioSource);
			
			y=(int)(235-((double)(TVolume+1)/1000*160));
			MoveWindow(GetDlgItem(hDlg,IDC_S1),33,y-4,22,8,TRUE);
			
			y=(int)(157-((double)(TSpecial+1)/256*160));
			MoveWindow(GetDlgItem(hDlg,IDC_S2),109,y-4,22,8,TRUE);
			
			y=(int)(235-((TLoudness+1)/68*160));
			MoveWindow(GetDlgItem(hDlg,IDC_S3),183,y-4,22,8,TRUE);
			
			y=(int)(157-((double)(TBass+1)/256*160));
			MoveWindow(GetDlgItem(hDlg,IDC_S4),258,y-4,22,8,TRUE);
			
			y=(int)(157-((double)(TTreble+1)/256*160));
			MoveWindow(GetDlgItem(hDlg,IDC_S5),334,y-4,22,8,TRUE);
			
			x=(int)(279-((double)(TBalance+1)/256*160));
			MoveWindow(GetDlgItem(hDlg,IDC_S6),x-4,271,8,22,TRUE);
			
			break;
			
		}
		
		break;
    }
	
	return(FALSE);
}


BOOL APIENTRY AudioDigiSettingProc
(
 HWND hDlg,
 UINT message,
 UINT wParam,
 LONG lParam)
{
	int x,y,i,j;
	
	switch (message) {
	case WM_INITDIALOG:
		TVolumeLinks=InitialDVBVolumeLinks;
		TVolumeRechts=InitialDVBVolumeRechts;
		TVolume=(InitialDVBVolumeLinks+InitialDVBVolumeRechts)/2;
		
		SetDlgItemInt(hDlg,IDC_D1,TVolumeLinks,FALSE);
		SetDlgItemInt(hDlg,IDC_D2,TVolume,FALSE);
		SetDlgItemInt(hDlg,IDC_D3,TVolumeRechts,FALSE);
		
		y=(int)(235-((double)TVolumeLinks/255*160));
		MoveWindow(GetDlgItem(hDlg,IDC_S1),33,y-4,22,8,FALSE);
		
		y=(int)(235-((double)TVolume/255*160));
		MoveWindow(GetDlgItem(hDlg,IDC_S2),109,y-4,22,8,FALSE);
		
		y=(int)(235-((double)TVolumeRechts/255*160));
		MoveWindow(GetDlgItem(hDlg,IDC_S3),183,y-4,22,8,FALSE);
		break;
		
		
		
	case WM_MOUSEMOVE:
		
		y=HIWORD( lParam );
		x=LOWORD( lParam );
		if (wParam == MK_LBUTTON) {
			if (( y >= 70) && ( y <= 240 )) {
				if (( x >= 33) && ( x <= 55 )) {
					i=(int)((double)(y-235)/160*255);
					i=-i;
					if ( i < 0) i=0;
					else if ( i > 255 ) i=255;
					MoveWindow(GetDlgItem(hDlg,IDC_S1),33,y-2,22,8,TRUE);
					TVolumeLinks=i;
					SetDlgItemInt(hDlg,IDC_D1,i,TRUE);
                		  TVolume = (TVolumeLinks+TVolumeRechts)/2; 
						  SetDlgItemInt(hDlg,IDC_D2,TVolume,FALSE);
						  y=(int)(235-((double)TVolume/256*160));
						  MoveWindow(GetDlgItem(hDlg,IDC_S2),109,y-4,22,8,TRUE);
						  DVB_SetVolume((unsigned char)TVolumeLinks, (unsigned char)TVolumeRechts);
				}
				if (( x >= 109) && ( x <= 131 )) {
					i=(int)((double)(y-235)/160*255);
					i=-i;
					if ( i < 0) i=0;
					else if ( i > 255 ) i=255;
					j=i-TVolume;
					
					if (( j+TVolumeLinks <= 255) && 
						( j+TVolumeRechts <= 255 ) && 
						( j+TVolumeLinks >= 0 ) &&
						( j+TVolumeRechts >= 0 )) {
						
						MoveWindow(GetDlgItem(hDlg,IDC_S2),109,y-4,22,8,TRUE);
						
						TVolumeLinks+=j;
						y=(int)(235-((double)TVolumeLinks/255*160));
						MoveWindow(GetDlgItem(hDlg,IDC_S1),33,y-2,22,8,TRUE);
						
						TVolumeRechts+=j;
						y=(int)(235-((double)TVolumeRechts/255*160));
						MoveWindow(GetDlgItem(hDlg,IDC_S3),183,y-2,22,8,TRUE);
						
						TVolume = (TVolumeLinks+TVolumeRechts)/2; 
						SetDlgItemInt(hDlg,IDC_D1,TVolumeLinks,FALSE);
						SetDlgItemInt(hDlg,IDC_D2,TVolume,FALSE);
						SetDlgItemInt(hDlg,IDC_D3,TVolumeRechts,FALSE);
						DVB_SetVolume((unsigned char)TVolumeLinks, (unsigned char)TVolumeRechts);
					}
				}
				
				
				if (( x >= 183) && ( x <= 205 )) {
					i=(int)((double)(y-235)/160*255);
					i=-i;
					if ( i < 0) i=0;
					else if ( i > 255 ) i=255;
					MoveWindow(GetDlgItem(hDlg,IDC_S3),183,y-2,22,8,TRUE);
					TVolumeRechts=i;
					SetDlgItemInt(hDlg,IDC_D3,i,TRUE);
                		  TVolume = (TVolumeLinks+TVolumeRechts)/2; 
						  SetDlgItemInt(hDlg,IDC_D2,TVolume,FALSE);
						  y=(int)(235-((double)TVolume/255*160));
						  MoveWindow(GetDlgItem(hDlg,IDC_S2),109,y-4,22,8,TRUE);
						  DVB_SetVolume((unsigned char)TVolumeLinks, (unsigned char)TVolumeRechts);
				}
			}
			
		}
		return(FALSE);
	case WM_COMMAND:
		
		if (LOWORD(wParam) == IDOK ) {
			InitialDVBVolumeRechts=TVolumeRechts;
			InitialDVBVolumeLinks=TVolumeLinks;
			EndDialog(hDlg,TRUE);
		}
		
		if (LOWORD(wParam) == IDCANCEL ) {
			DVB_SetVolume((unsigned char)InitialDVBVolumeLinks, (unsigned char)InitialDVBVolumeRechts);
			EndDialog(hDlg,TRUE);
		}
		
		
		
		if ( LOWORD(wParam) == IDDEFAULT) {
			
			TVolumeRechts=0xff;
			TVolumeLinks=0xff;
			DVB_SetVolume((unsigned char)TVolumeLinks, (unsigned char)TVolumeRechts);
			
			SetDlgItemInt(hDlg,IDC_D1,TVolumeLinks,FALSE);
			SetDlgItemInt(hDlg,IDC_D2,TVolume,FALSE);
			SetDlgItemInt(hDlg,IDC_D3,TVolumeRechts,FALSE);
			
			y=(int)(235-((double)TVolumeLinks/128*160));
			MoveWindow(GetDlgItem(hDlg,IDC_S1),33,y-4,22,8,TRUE);
			
			y=(int)(235-((double)TVolume/128*160));
			MoveWindow(GetDlgItem(hDlg,IDC_S2),109,y-4,22,8,TRUE);
			
			y=(int)(235-((double)TVolumeRechts/128*160));
			MoveWindow(GetDlgItem(hDlg,IDC_S3),183,y-4,22,8,TRUE);
			
			break;
			
		}
		
		break;
    }
	
	return(FALSE);
}




BOOL APIENTRY OSDSettingProc
(
 HWND hDlg,
 UINT message,
 UINT wParam,
 LONG lParam)
{
	
	int x,y,i;
	static int cl;
	static struct OSD_Colors_t OSD_Colors_Save[4];   
	switch (message) {
	case WM_INITDIALOG:
		memcpy(OSD_Colors_Save,OSD_Colors,sizeof(struct OSD_Colors_t) * 4 );
		CheckDlgButton(hDlg, IDC_RADIO1, TRUE);
		Update_OSD_Color();
		
		
	case WM_USER:
		
		if ( IsDlgButtonChecked(hDlg,IDC_RADIO1) == TRUE ) cl=0;
		else if ( IsDlgButtonChecked(hDlg,IDC_RADIO2) == TRUE ) cl=1;
		else if ( IsDlgButtonChecked(hDlg,IDC_RADIO3) == TRUE ) cl=2;
		else if ( IsDlgButtonChecked(hDlg,IDC_RADIO4) == TRUE ) cl=3;
		SetDlgItemInt(hDlg,IDC_R,OSD_Colors[cl].R,FALSE);
		SetDlgItemInt(hDlg,IDC_G,OSD_Colors[cl].G,FALSE);
		SetDlgItemInt(hDlg,IDC_B,OSD_Colors[cl].B,FALSE);
		SetDlgItemInt(hDlg,IDC_BLEND,OSD_Colors[cl].Blend,FALSE);
		
		x=(int)(105+((double)(OSD_Colors[cl].R)/256*168));
		MoveWindow(GetDlgItem(hDlg,IDC_SR),x-4,76,8,22,TRUE);
		
		x=(int)(105+((double)(OSD_Colors[cl].G)/256*168));
		MoveWindow(GetDlgItem(hDlg,IDC_SG),x-4,110,8,22,TRUE);
		
		x=(int)(105+((double)(OSD_Colors[cl].B)/256*168));
		MoveWindow(GetDlgItem(hDlg,IDC_SB),x-4,141,8,22,TRUE);
		
		x=(int)(105+((double)(OSD_Colors[cl].Blend)/256*168));
		MoveWindow(GetDlgItem(hDlg,IDC_SBLEND),x-4,183,8,22,TRUE);
		
		break;
		
		
		
	case WM_MOUSEMOVE:
		
		y=HIWORD( lParam );
		x=LOWORD( lParam );
		if (wParam == MK_LBUTTON) {
			if (( x >= 105 ) && ( x <= 273 )) {
				
				if (( y >= 75 ) && ( y <= 100 )) {   // ROT
					i=(int)((double)(x-105)/168*255);
					if ( i < 0) i=0;
					else if ( i > 255 ) i=255;
					OSD_Colors[cl].R=i;
					SetDlgItemInt(hDlg,IDC_R,OSD_Colors[cl].R,FALSE);
					x=(int)(105+((double)(OSD_Colors[cl].R)/256*168));
					MoveWindow(GetDlgItem(hDlg,IDC_SR),x-4,76,8,22,TRUE);
					Update_OSD_Color();
				} // ROT
				
				if (( y >= 109 ) && ( y <= 133 )) {   // GRUEN
					i=(int)((double)(x-105)/168*255);
					if ( i < 0) i=0;
					else if ( i > 255 ) i=255;
					OSD_Colors[cl].G=i;
					SetDlgItemInt(hDlg,IDC_G,OSD_Colors[cl].G,FALSE);
					x=(int)(105+((double)(OSD_Colors[cl].G)/256*168));
					MoveWindow(GetDlgItem(hDlg,IDC_SG),x-4,110,8,22,TRUE);
					Update_OSD_Color();
				} // GRUEN
				
				if (( y >= 140 ) && ( y <= 164 )) {   // BLAU
					i=(int)((double)(x-105)/168*255);
					if ( i < 0) i=0;
					else if ( i > 255 ) i=255;
					OSD_Colors[cl].B=i;
					SetDlgItemInt(hDlg,IDC_G,OSD_Colors[cl].B,FALSE);
					x=(int)(105+((double)(OSD_Colors[cl].B)/256*168));
					MoveWindow(GetDlgItem(hDlg,IDC_SB),x-4,141,8,22,TRUE);
					Update_OSD_Color();
				} // BLAU
				
				if (( y >= 182 ) && ( y <= 206)) {   // BLEND
					i=(int)((double)(x-105)/168*255);
					if ( i < 0) i=0;
					else if ( i > 255 ) i=255;
					OSD_Colors[cl].Blend=i;
					SetDlgItemInt(hDlg,IDC_BLEND,OSD_Colors[cl].Blend,FALSE);
					x=(int)(105+((double)(OSD_Colors[cl].Blend)/256*168));
					MoveWindow(GetDlgItem(hDlg,IDC_SBLEND),x-4,183,8,22,TRUE);
					Update_OSD_Color();
				} // BLEND
				
				
			};
			
			
			
		}
		return(FALSE);
	case WM_COMMAND:
		if ( ((HWND)lParam==GetDlgItem(hDlg,IDC_RADIO1) ) ||
			((HWND)lParam==GetDlgItem(hDlg,IDC_RADIO2) ) ||
			((HWND)lParam==GetDlgItem(hDlg,IDC_RADIO3) ) ||
			((HWND)lParam==GetDlgItem(hDlg,IDC_RADIO4) )) {
			PostMessage(hDlg,WM_USER,0,0);
			break;
		};
		
		if (LOWORD(wParam) == IDOK ) {
			Stop_Osd_Key_Event();
			EndDialog(hDlg,TRUE);
		}
		
		
		if (LOWORD(wParam) == IDCANCEL ) {
			memcpy(OSD_Colors,OSD_Colors_Save,sizeof(struct OSD_Colors_t) * 4 );
			Stop_Osd_Key_Event();
			EndDialog(hDlg,TRUE);
		}
		
		
		
		if ( LOWORD(wParam) == IDDEFAULT) {
			OSD_Colors[0].R=0;OSD_Colors[0].G=0;OSD_Colors[0].B=100;OSD_Colors[0].Blend=255;
			OSD_Colors[1].R=50;OSD_Colors[1].G=50;OSD_Colors[1].B=200;OSD_Colors[1].Blend=255;
			OSD_Colors[2].R=255;OSD_Colors[2].G=255;OSD_Colors[2].B=0;OSD_Colors[2].Blend=255;
			OSD_Colors[3].R=255;OSD_Colors[3].G=255;OSD_Colors[3].B=255;OSD_Colors[3].Blend=255;
			Update_OSD_Color();
			PostMessage(hDlg,WM_USER,0,0);
			break;
			
		}
		
		break;
    }
	
	return(FALSE);
}



BOOL APIENTRY LNBSettingProc
(
 HWND hDlg,
 UINT message,
 UINT wParam,
 LONG lParam)
{
	
	char zeile[140];
    unsigned long freq;
	float ffreq;
	
    PAINTSTRUCT wps;                   /* paint structure           */
    HDC hdc;
    HDC             hMemDC;
    HBITMAP         hOldBm;
	BITMAP bm;
	
	int diseqnr;
	
    HBRUSH hBrush;
    HWND wndFill;
	
	char Zeile[128];
	
	int i; 
	RECT rc;
	int sig;
	int err;
	
	
	switch (message) {
		
	case WM_INITDIALOG:
		Write_INI("Multidec.ini");
		Transponder.fec=Programm[CurrentProgramm].fec;
		Transponder.diseqc=Programm[CurrentProgramm].diseqc;
		Transponder.freq=Programm[CurrentProgramm].freq;
		Transponder.qam=Programm[CurrentProgramm].qam;
		Transponder.srate=Programm[CurrentProgramm].srate;
		Transponder.volt=Programm[CurrentProgramm].volt;
		Transponder.sync=dvb_front.sync;
		
		if ( Transponder.volt == 1 ) CheckDlgButton(hDlg, IDC_RADIO20, TRUE);
		else CheckDlgButton(hDlg, IDC_RADIO21, TRUE);
		
		if ( Transponder.fec == 0 ) CheckDlgButton(hDlg, IDC_RADIO11, TRUE);
		else if ( Transponder.fec == 1 ) CheckDlgButton(hDlg, IDC_RADIO12, TRUE);
		else if ( Transponder.fec == 2 ) CheckDlgButton(hDlg, IDC_RADIO13, TRUE);
		else if ( Transponder.fec == 3 ) CheckDlgButton(hDlg, IDC_RADIO14, TRUE);
		else if ( Transponder.fec == 4 ) CheckDlgButton(hDlg, IDC_RADIO15, TRUE);
		else if ( Transponder.fec == 5 ) CheckDlgButton(hDlg, IDC_RADIO16, TRUE);
		else if ( Transponder.fec == 6 ) CheckDlgButton(hDlg, IDC_RADIO17, TRUE);
		else if ( Transponder.fec == 7 ) CheckDlgButton(hDlg, IDC_RADIO18, TRUE);
		else if ( Transponder.fec == 8 ) CheckDlgButton(hDlg, IDC_RADIO19, TRUE);
		
		SetDlgItemInt(hDlg,IDC_EDIT6,Transponder.srate,FALSE);
		sprintf(zeile,"%10.3f",Transponder.freq/1000.f);
		SetDlgItemText(hDlg,IDC_EDIT2,zeile);
		
		if ( Transponder.diseqc == 0 ) CheckDlgButton(hDlg, IDC_RADIO1, TRUE);
		else if ( Transponder.diseqc == 1 ) CheckDlgButton(hDlg, IDC_RADIO2, TRUE);
		else if ( Transponder.diseqc == 2 ) CheckDlgButton(hDlg, IDC_RADIO3, TRUE);
		else if ( Transponder.diseqc == 3 ) CheckDlgButton(hDlg, IDC_RADIO4, TRUE);
		
		CheckDlgButton(hDlg, IDC_USELNB2, LNB[1].Use);
		CheckDlgButton(hDlg, IDC_USELNB3, LNB[2].Use);
		CheckDlgButton(hDlg, IDC_USELNB4, LNB[3].Use);
		
		PostMessage(hDlg,WM_USER,1,1);
		SetTimer(hDlg,0,2000,NULL);
		
		
		return(TRUE);
		
	case WM_USER:
		
		if ( wParam == 1 ) {
			
			EnableWindow(GetDlgItem(hDlg,IDC_RADIO2),FALSE);
			EnableWindow(GetDlgItem(hDlg,IDC_RADIO3),FALSE);
			EnableWindow(GetDlgItem(hDlg,IDC_RADIO4),FALSE);
			
			LNB[0].Use=TRUE;
			LNB[1].Use=IsDlgButtonChecked(hDlg, IDC_USELNB2 );
			LNB[2].Use=IsDlgButtonChecked(hDlg, IDC_USELNB3 );
			LNB[3].Use=IsDlgButtonChecked(hDlg, IDC_USELNB4 );
			
			
			if ( UseDiseqC() == TRUE ) {
				LNB[0].MiniDiseqc=0x00;
				LNB[1].MiniDiseqc=0x00;
				LNB[2].MiniDiseqc=0x00;
				LNB[3].MiniDiseqc=0x00;
				
				if ( IsDlgButtonChecked(hDlg, IDC_MINIDISEQ1 )) LNB[0].MiniDiseqc=0x01;
				if (( LNB[1].Use == TRUE ) && ( IsDlgButtonChecked(hDlg, IDC_MINIDISEQ2 ))) LNB[1].MiniDiseqc=0x01;
				if (( LNB[2].Use == TRUE ) && ( IsDlgButtonChecked(hDlg, IDC_MINIDISEQ3 ))) LNB[2].MiniDiseqc=0x01;
				if (( LNB[3].Use == TRUE ) && ( IsDlgButtonChecked(hDlg, IDC_MINIDISEQ4 ))) LNB[3].MiniDiseqc=0x01;
				
			}
			
			
			SetDlgItemInt(hDlg,IDC_MINFREQ1,LNB[0].MinFreq,FALSE);
			SetDlgItemInt(hDlg,IDC_MAXFREQ1,LNB[0].MaxFreq,FALSE);
			SetDlgItemInt(hDlg,IDC_LOF_LOW1,LNB[0].LofLow,FALSE);
			SetDlgItemInt(hDlg,IDC_LOF_SWITCH1,LNB[0].SwitchFreq,FALSE);
			SetDlgItemInt(hDlg,IDC_LOF_HIGH1,LNB[0].LofHigh,FALSE);
			CheckDlgButton(hDlg, IDC_LNBPOWER1, LNB[0].Power);
			CheckDlgButton(hDlg, IDC_LNBSWITSH22KHZ1, LNB[0].Switch22khz);
			
			if ( UseDiseqC() == TRUE ) {
				
				EnableWindow(GetDlgItem(hDlg,IDC_TONEBURSTNONE1),TRUE);
				EnableWindow(GetDlgItem(hDlg,IDC_TONEBURSTAUS1),TRUE);
				EnableWindow(GetDlgItem(hDlg,IDC_TONEBURSTAN1),TRUE);
				EnableWindow(GetDlgItem(hDlg,IDC_DISEQCRESET1),TRUE);
				EnableWindow(GetDlgItem(hDlg,IDC_MINIDISEQ1),TRUE);
				CheckDlgButton(hDlg, IDC_DISEQCRESET1,LNB[0].PowerReset);
				
				if ( LNB[0].MiniDiseqc == 0x01 ) {
					CheckDlgButton(hDlg, IDC_MINIDISEQ1,TRUE);
					EnableWindow(GetDlgItem(hDlg,IDC_POSALNB1),FALSE);
					EnableWindow(GetDlgItem(hDlg,IDC_POSBLNB1),FALSE);
					EnableWindow(GetDlgItem(hDlg,IDC_OPTALNB1),FALSE);
					EnableWindow(GetDlgItem(hDlg,IDC_OPTBLNB1),FALSE);
				} else {
					CheckDlgButton(hDlg, IDC_MINIDISEQ1,FALSE);
					EnableWindow(GetDlgItem(hDlg,IDC_POSALNB1),TRUE);
					EnableWindow(GetDlgItem(hDlg,IDC_POSBLNB1),TRUE);
					EnableWindow(GetDlgItem(hDlg,IDC_OPTALNB1),TRUE);
					EnableWindow(GetDlgItem(hDlg,IDC_OPTBLNB1),TRUE);
				};
				
				if ( LNB[0].BurstVal == -1 ) CheckDlgButton(hDlg, IDC_TONEBURSTNONE1,TRUE);
				else if ( LNB[0].BurstVal == 0 ) CheckDlgButton(hDlg, IDC_TONEBURSTAUS1,TRUE);
				else if ( LNB[0].BurstVal == 1 ) CheckDlgButton(hDlg, IDC_TONEBURSTAN1,TRUE);
				
                if ( LNB[0].PosVal == 'A' ) CheckDlgButton(hDlg, IDC_POSALNB1,TRUE);
				else if ( LNB[0].PosVal == 'B') CheckDlgButton(hDlg, IDC_POSBLNB1,TRUE);
				
                if ( LNB[0].OptVal == 'A' ) CheckDlgButton(hDlg, IDC_OPTALNB1,TRUE);
				else if ( LNB[0].OptVal == 'B') CheckDlgButton(hDlg, IDC_OPTBLNB1,TRUE);
			} else {
				CheckDlgButton(hDlg, IDC_RADIO1,TRUE);
				if ( LNB[0].BurstVal == -1 ) CheckDlgButton(hDlg, IDC_TONEBURSTNONE1,TRUE);
				else if ( LNB[0].BurstVal == 0 ) CheckDlgButton(hDlg, IDC_TONEBURSTAUS1,TRUE);
				else if ( LNB[0].BurstVal == 1 ) CheckDlgButton(hDlg, IDC_TONEBURSTAN1,TRUE);
				
                if ( LNB[0].PosVal == 'A' ) CheckDlgButton(hDlg, IDC_POSALNB1,TRUE);
				else if ( LNB[0].PosVal == 'B') CheckDlgButton(hDlg, IDC_POSBLNB1,TRUE);
                if ( LNB[0].OptVal == 'A' ) CheckDlgButton(hDlg, IDC_OPTALNB1,TRUE);
				EnableWindow(GetDlgItem(hDlg,IDC_TONEBURSTNONE1),FALSE);
				EnableWindow(GetDlgItem(hDlg,IDC_TONEBURSTAUS1),FALSE);
				EnableWindow(GetDlgItem(hDlg,IDC_TONEBURSTAN1),FALSE);
				EnableWindow(GetDlgItem(hDlg,IDC_POSALNB1),FALSE);
				EnableWindow(GetDlgItem(hDlg,IDC_POSBLNB1),FALSE);
				EnableWindow(GetDlgItem(hDlg,IDC_OPTALNB1),FALSE);
				EnableWindow(GetDlgItem(hDlg,IDC_OPTBLNB1),FALSE);
				EnableWindow(GetDlgItem(hDlg,IDC_DISEQCRESET1),FALSE);
				EnableWindow(GetDlgItem(hDlg,IDC_MINIDISEQ1),FALSE);
				
			};
			
			if ( LNB[1].Use == TRUE ) {
				CheckDlgButton(hDlg, IDC_USELNB2, TRUE);
				EnableWindow(GetDlgItem(hDlg,IDC_RADIO2),TRUE);
				EnableWindow(GetDlgItem(hDlg,IDC_MINFREQ2),TRUE);
				EnableWindow(GetDlgItem(hDlg,IDC_MAXFREQ2),TRUE);
				EnableWindow(GetDlgItem(hDlg,IDC_LOF_LOW2),TRUE);
				EnableWindow(GetDlgItem(hDlg,IDC_LOF_SWITCH2),TRUE);
				EnableWindow(GetDlgItem(hDlg,IDC_LOF_HIGH2),TRUE);
				EnableWindow(GetDlgItem(hDlg,IDC_LNBPOWER2),TRUE);
				EnableWindow(GetDlgItem(hDlg,IDC_LNBSWITSH22KHZ2),TRUE);
                EnableWindow(GetDlgItem(hDlg,IDC_TONEBURSTNONE2),TRUE);
				EnableWindow(GetDlgItem(hDlg,IDC_TONEBURSTAUS2),TRUE);
				EnableWindow(GetDlgItem(hDlg,IDC_TONEBURSTAN2),TRUE);
				EnableWindow(GetDlgItem(hDlg,IDC_DISEQCRESET2),TRUE);
				EnableWindow(GetDlgItem(hDlg,IDC_MINIDISEQ2),TRUE);
				
				SetDlgItemInt(hDlg,IDC_MINFREQ2,LNB[1].MinFreq,FALSE);
				SetDlgItemInt(hDlg,IDC_MAXFREQ2,LNB[1].MaxFreq,FALSE);
				SetDlgItemInt(hDlg,IDC_LOF_LOW2,LNB[1].LofLow,FALSE);
				SetDlgItemInt(hDlg,IDC_LOF_SWITCH2,LNB[1].SwitchFreq,FALSE);
				SetDlgItemInt(hDlg,IDC_LOF_HIGH2,LNB[1].LofHigh,FALSE);
				
				CheckDlgButton(hDlg, IDC_DISEQCRESET2,LNB[1].PowerReset);
				CheckDlgButton(hDlg, IDC_LNBPOWER2, LNB[1].Power);
				CheckDlgButton(hDlg, IDC_LNBSWITSH22KHZ2, LNB[1].Switch22khz);
				
				if ( LNB[1].MiniDiseqc == 0x01 ) {
					CheckDlgButton(hDlg, IDC_MINIDISEQ2,TRUE);
					EnableWindow(GetDlgItem(hDlg,IDC_POSALNB2),FALSE);
					EnableWindow(GetDlgItem(hDlg,IDC_POSBLNB2),FALSE);
					EnableWindow(GetDlgItem(hDlg,IDC_OPTALNB2),FALSE);
					EnableWindow(GetDlgItem(hDlg,IDC_OPTBLNB2),FALSE);
				} else {
					CheckDlgButton(hDlg, IDC_MINIDISEQ2,FALSE);
					EnableWindow(GetDlgItem(hDlg,IDC_POSALNB2),TRUE);
					EnableWindow(GetDlgItem(hDlg,IDC_POSBLNB2),TRUE);
					EnableWindow(GetDlgItem(hDlg,IDC_OPTALNB2),TRUE);
					EnableWindow(GetDlgItem(hDlg,IDC_OPTBLNB2),TRUE);
				};
				
                
				if ( LNB[1].BurstVal == -1 ) CheckDlgButton(hDlg, IDC_TONEBURSTNONE2,TRUE);
				else if ( LNB[1].BurstVal == 0 ) CheckDlgButton(hDlg, IDC_TONEBURSTAUS2,TRUE);
				else if ( LNB[1].BurstVal == 1 ) CheckDlgButton(hDlg, IDC_TONEBURSTAN2,TRUE);
				
                if ( LNB[1].PosVal == 'A' ) CheckDlgButton(hDlg, IDC_POSALNB2,TRUE);
				else if ( LNB[1].PosVal == 'B') CheckDlgButton(hDlg, IDC_POSBLNB2,TRUE);
				
                if ( LNB[1].OptVal == 'A' ) CheckDlgButton(hDlg, IDC_OPTALNB2,TRUE);
				else if ( LNB[1].OptVal == 'B') CheckDlgButton(hDlg, IDC_OPTBLNB2,TRUE);
			} else {
				CheckDlgButton(hDlg, IDC_USELNB2, FALSE);
				EnableWindow(GetDlgItem(hDlg,IDC_MINFREQ2),FALSE);
				EnableWindow(GetDlgItem(hDlg,IDC_MAXFREQ2),FALSE);
				EnableWindow(GetDlgItem(hDlg,IDC_LOF_LOW2),FALSE);
				EnableWindow(GetDlgItem(hDlg,IDC_LOF_SWITCH2),FALSE);
				EnableWindow(GetDlgItem(hDlg,IDC_LOF_HIGH2),FALSE);
				EnableWindow(GetDlgItem(hDlg,IDC_LNBPOWER2),FALSE);
				EnableWindow(GetDlgItem(hDlg,IDC_LNBSWITSH22KHZ2),FALSE);
				EnableWindow(GetDlgItem(hDlg,IDC_TONEBURSTNONE2),FALSE);
				EnableWindow(GetDlgItem(hDlg,IDC_TONEBURSTAUS2),FALSE);
				EnableWindow(GetDlgItem(hDlg,IDC_TONEBURSTAN2),FALSE);
				EnableWindow(GetDlgItem(hDlg,IDC_POSALNB2),FALSE);
				EnableWindow(GetDlgItem(hDlg,IDC_POSBLNB2),FALSE);
				EnableWindow(GetDlgItem(hDlg,IDC_OPTALNB2),FALSE);
				EnableWindow(GetDlgItem(hDlg,IDC_OPTBLNB2),FALSE);
				EnableWindow(GetDlgItem(hDlg,IDC_MINIDISEQ2),FALSE);
				EnableWindow(GetDlgItem(hDlg,IDC_DISEQCRESET2),FALSE);
			}
			
			if ( LNB[2].Use == TRUE ) {
				CheckDlgButton(hDlg, IDC_USELNB3, TRUE);
				EnableWindow(GetDlgItem(hDlg,IDC_RADIO3),TRUE);
				EnableWindow(GetDlgItem(hDlg,IDC_MINFREQ3),TRUE);
				EnableWindow(GetDlgItem(hDlg,IDC_MAXFREQ3),TRUE);
				EnableWindow(GetDlgItem(hDlg,IDC_LOF_LOW3),TRUE);
				EnableWindow(GetDlgItem(hDlg,IDC_LOF_SWITCH3),TRUE);
				EnableWindow(GetDlgItem(hDlg,IDC_LOF_HIGH3),TRUE);
				EnableWindow(GetDlgItem(hDlg,IDC_LNBPOWER3),TRUE);
				EnableWindow(GetDlgItem(hDlg,IDC_LNBSWITSH22KHZ3),TRUE);
                EnableWindow(GetDlgItem(hDlg,IDC_TONEBURSTNONE3),TRUE);
				EnableWindow(GetDlgItem(hDlg,IDC_TONEBURSTAUS3),TRUE);
				EnableWindow(GetDlgItem(hDlg,IDC_TONEBURSTAN3),TRUE);
                EnableWindow(GetDlgItem(hDlg,IDC_MINIDISEQ3),TRUE);
				EnableWindow(GetDlgItem(hDlg,IDC_DISEQCRESET3),TRUE);
				
				SetDlgItemInt(hDlg,IDC_MINFREQ3,LNB[2].MinFreq,FALSE);
				SetDlgItemInt(hDlg,IDC_MAXFREQ3,LNB[2].MaxFreq,FALSE);
				SetDlgItemInt(hDlg,IDC_LOF_LOW3,LNB[2].LofLow,FALSE);
				SetDlgItemInt(hDlg,IDC_LOF_SWITCH3,LNB[2].SwitchFreq,FALSE);
				SetDlgItemInt(hDlg,IDC_LOF_HIGH3,LNB[2].LofHigh,FALSE);
				CheckDlgButton(hDlg, IDC_DISEQCRESET3, LNB[2].PowerReset);
				CheckDlgButton(hDlg, IDC_LNBPOWER3, LNB[2].Power);
				CheckDlgButton(hDlg, IDC_LNBSWITSH22KHZ3, LNB[2].Switch22khz);
				
				if ( LNB[2].MiniDiseqc == 0x01 ) {
					CheckDlgButton(hDlg, IDC_MINIDISEQ3,TRUE);
					EnableWindow(GetDlgItem(hDlg,IDC_POSALNB3),FALSE);
					EnableWindow(GetDlgItem(hDlg,IDC_POSBLNB3),FALSE);
					EnableWindow(GetDlgItem(hDlg,IDC_OPTALNB3),FALSE);
					EnableWindow(GetDlgItem(hDlg,IDC_OPTBLNB3),FALSE);
				} else {
					CheckDlgButton(hDlg, IDC_MINIDISEQ3,FALSE);
					EnableWindow(GetDlgItem(hDlg,IDC_POSALNB3),TRUE);
					EnableWindow(GetDlgItem(hDlg,IDC_POSBLNB3),TRUE);
					EnableWindow(GetDlgItem(hDlg,IDC_OPTALNB3),TRUE);
					EnableWindow(GetDlgItem(hDlg,IDC_OPTBLNB3),TRUE);
				};
                
				
				if ( LNB[2].BurstVal == -1 ) CheckDlgButton(hDlg, IDC_TONEBURSTNONE3,TRUE);
				else if ( LNB[2].BurstVal == 0 ) CheckDlgButton(hDlg, IDC_TONEBURSTAUS3,TRUE);
				else if ( LNB[2].BurstVal == 1 ) CheckDlgButton(hDlg, IDC_TONEBURSTAN3,TRUE);
				
                if ( LNB[2].PosVal == 'A' ) CheckDlgButton(hDlg, IDC_POSALNB3,TRUE);
				else if ( LNB[2].PosVal == 'B') CheckDlgButton(hDlg, IDC_POSBLNB3,TRUE);
				
                if ( LNB[2].OptVal == 'A' ) CheckDlgButton(hDlg, IDC_OPTALNB3,TRUE);
				else if ( LNB[2].OptVal == 'B') CheckDlgButton(hDlg, IDC_OPTBLNB3,TRUE);
			} else {
				CheckDlgButton(hDlg, IDC_USELNB3, FALSE);
				EnableWindow(GetDlgItem(hDlg,IDC_DISEQCRESET3),FALSE);
				EnableWindow(GetDlgItem(hDlg,IDC_MINFREQ3),FALSE);
				EnableWindow(GetDlgItem(hDlg,IDC_MAXFREQ3),FALSE);
				EnableWindow(GetDlgItem(hDlg,IDC_LOF_LOW3),FALSE);
				EnableWindow(GetDlgItem(hDlg,IDC_LOF_SWITCH3),FALSE);
				EnableWindow(GetDlgItem(hDlg,IDC_LOF_HIGH3),FALSE);
				EnableWindow(GetDlgItem(hDlg,IDC_LNBPOWER3),FALSE);
				EnableWindow(GetDlgItem(hDlg,IDC_LNBSWITSH22KHZ3),FALSE);
				EnableWindow(GetDlgItem(hDlg,IDC_TONEBURSTNONE3),FALSE);
				EnableWindow(GetDlgItem(hDlg,IDC_TONEBURSTAUS3),FALSE);
				EnableWindow(GetDlgItem(hDlg,IDC_TONEBURSTAN3),FALSE);
				EnableWindow(GetDlgItem(hDlg,IDC_POSALNB3),FALSE);
				EnableWindow(GetDlgItem(hDlg,IDC_POSBLNB3),FALSE);
				EnableWindow(GetDlgItem(hDlg,IDC_OPTALNB3),FALSE);
				EnableWindow(GetDlgItem(hDlg,IDC_OPTBLNB3),FALSE);
				EnableWindow(GetDlgItem(hDlg,IDC_MINIDISEQ3),FALSE);
			}
			
			if ( LNB[3].Use == TRUE ) {
				CheckDlgButton(hDlg, IDC_USELNB4, TRUE);
				EnableWindow(GetDlgItem(hDlg,IDC_RADIO4),TRUE);
				EnableWindow(GetDlgItem(hDlg,IDC_MINFREQ4),TRUE);
				EnableWindow(GetDlgItem(hDlg,IDC_MAXFREQ4),TRUE);
				EnableWindow(GetDlgItem(hDlg,IDC_LOF_LOW4),TRUE);
				EnableWindow(GetDlgItem(hDlg,IDC_LOF_SWITCH4),TRUE);
				EnableWindow(GetDlgItem(hDlg,IDC_LOF_HIGH4),TRUE);
				EnableWindow(GetDlgItem(hDlg,IDC_LNBPOWER4),TRUE);
				EnableWindow(GetDlgItem(hDlg,IDC_LNBSWITSH22KHZ4),TRUE);
                EnableWindow(GetDlgItem(hDlg,IDC_TONEBURSTNONE4),TRUE);
				EnableWindow(GetDlgItem(hDlg,IDC_TONEBURSTAUS4),TRUE);
				EnableWindow(GetDlgItem(hDlg,IDC_TONEBURSTAN4),TRUE);
                EnableWindow(GetDlgItem(hDlg,IDC_MINIDISEQ4),TRUE);
				EnableWindow(GetDlgItem(hDlg,IDC_DISEQCRESET4),TRUE);
				SetDlgItemInt(hDlg,IDC_MINFREQ4,LNB[3].MinFreq,FALSE);
				SetDlgItemInt(hDlg,IDC_MAXFREQ4,LNB[3].MaxFreq,FALSE);
				SetDlgItemInt(hDlg,IDC_LOF_LOW4,LNB[3].LofLow,FALSE);
				SetDlgItemInt(hDlg,IDC_LOF_SWITCH4,LNB[3].SwitchFreq,FALSE);
				SetDlgItemInt(hDlg,IDC_LOF_HIGH4,LNB[3].LofHigh,FALSE);
				CheckDlgButton(hDlg, IDC_DISEQCRESET4, LNB[3].PowerReset);
				CheckDlgButton(hDlg, IDC_LNBPOWER4, LNB[3].Power);
				CheckDlgButton(hDlg, IDC_LNBSWITSH22KHZ4, LNB[3].Switch22khz);
				
                
				if ( LNB[3].MiniDiseqc == 0x01 ) {
					CheckDlgButton(hDlg, IDC_MINIDISEQ4,TRUE);
					EnableWindow(GetDlgItem(hDlg,IDC_POSALNB4),FALSE);
					EnableWindow(GetDlgItem(hDlg,IDC_POSBLNB4),FALSE);
					EnableWindow(GetDlgItem(hDlg,IDC_OPTALNB4),FALSE);
					EnableWindow(GetDlgItem(hDlg,IDC_OPTBLNB4),FALSE);
				} else {
					CheckDlgButton(hDlg, IDC_MINIDISEQ4,FALSE);
					EnableWindow(GetDlgItem(hDlg,IDC_POSALNB4),TRUE);
					EnableWindow(GetDlgItem(hDlg,IDC_POSBLNB4),TRUE);
					EnableWindow(GetDlgItem(hDlg,IDC_OPTALNB4),TRUE);
					EnableWindow(GetDlgItem(hDlg,IDC_OPTBLNB4),TRUE);
				};
				
				
				if ( LNB[3].BurstVal == -1 ) CheckDlgButton(hDlg, IDC_TONEBURSTNONE4,TRUE);
				else if ( LNB[3].BurstVal == 0 ) CheckDlgButton(hDlg, IDC_TONEBURSTAUS4,TRUE);
				else if ( LNB[3].BurstVal == 1 ) CheckDlgButton(hDlg, IDC_TONEBURSTAN4,TRUE);
				
                if ( LNB[3].PosVal == 'A' ) CheckDlgButton(hDlg, IDC_POSALNB4,TRUE);
				else if ( LNB[3].PosVal == 'B') CheckDlgButton(hDlg, IDC_POSBLNB4,TRUE);
				
                if ( LNB[3].OptVal == 'A' ) CheckDlgButton(hDlg, IDC_OPTALNB4,TRUE);
				else if ( LNB[3].OptVal == 'B') CheckDlgButton(hDlg, IDC_OPTBLNB4,TRUE);
			} else {
				CheckDlgButton(hDlg, IDC_USELNB4, FALSE);
				EnableWindow(GetDlgItem(hDlg,IDC_DISEQCRESET4),FALSE);
				EnableWindow(GetDlgItem(hDlg,IDC_MINFREQ4),FALSE);
				EnableWindow(GetDlgItem(hDlg,IDC_MAXFREQ4),FALSE);
				EnableWindow(GetDlgItem(hDlg,IDC_LOF_LOW4),FALSE);
				EnableWindow(GetDlgItem(hDlg,IDC_LOF_SWITCH4),FALSE);
				EnableWindow(GetDlgItem(hDlg,IDC_LOF_HIGH4),FALSE);
				EnableWindow(GetDlgItem(hDlg,IDC_LNBPOWER4),FALSE);
				EnableWindow(GetDlgItem(hDlg,IDC_LNBSWITSH22KHZ4),FALSE);
				EnableWindow(GetDlgItem(hDlg,IDC_TONEBURSTNONE4),FALSE);
				EnableWindow(GetDlgItem(hDlg,IDC_TONEBURSTAUS4),FALSE);
				EnableWindow(GetDlgItem(hDlg,IDC_TONEBURSTAN4),FALSE);
				EnableWindow(GetDlgItem(hDlg,IDC_POSALNB4),FALSE);
				EnableWindow(GetDlgItem(hDlg,IDC_POSBLNB4),FALSE);
				EnableWindow(GetDlgItem(hDlg,IDC_OPTALNB4),FALSE);
				EnableWindow(GetDlgItem(hDlg,IDC_OPTBLNB4),FALSE);
				EnableWindow(GetDlgItem(hDlg,IDC_MINIDISEQ4),FALSE);
			}
			
			};
			
            break;			
			
			
        case WM_TIMER:
			rc.top=150;
			rc.left=410;
			rc.bottom=182;
			rc.right=640;
			if ( Current_Mode == 1 ) InvalidateRect(hDlg,&rc,FALSE);
			break;
			
			
        case WM_PAINT:
			
			Get_Signal_Infos((unsigned char *)&Transponder.sync,&sig,&err);
			hdc = BeginPaint(hDlg,&wps);
			hMemDC = CreateCompatibleDC(hdc);
            if (( Transponder.sync&0x0f) > 0 ) hOldBm = SelectObject(hMemDC, BirneGruen);
			else hOldBm = SelectObject(hMemDC, BirneRot );
            GetObject(BirneGruen, sizeof(BITMAP), (LPSTR)&bm);
			
			BitBlt(hdc, 60, 445, bm.bmWidth, bm.bmHeight,hMemDC, 0, 0, SRCCOPY);// Signal
			
			SelectObject(hMemDC,hOldBm);
            if (( Transponder.sync&0x0f) > 1 ) hOldBm = SelectObject(hMemDC, BirneGruen);
			else hOldBm = SelectObject(hMemDC, BirneRot );
			
			BitBlt(hdc, 125, 445, bm.bmWidth, bm.bmHeight,hMemDC, 0, 0, SRCCOPY); // Carrier
			SelectObject(hMemDC,hOldBm);
			
			
            if (( Transponder.sync&0x0f) > 3 ) hOldBm = SelectObject(hMemDC, BirneGruen);
			else hOldBm = SelectObject(hMemDC, BirneRot );
			
			BitBlt(hdc, 195, 445, bm.bmWidth, bm.bmHeight,hMemDC, 0, 0, SRCCOPY); // Viterbi
			SelectObject(hMemDC,hOldBm);
			
			if (( Transponder.sync&0x0f) > 7 ) hOldBm = SelectObject(hMemDC, BirneGruen);
			else hOldBm = SelectObject(hMemDC, BirneRot );
			
			BitBlt(hdc, 265, 445, bm.bmWidth, bm.bmHeight,hMemDC, 0, 0, SRCCOPY); // FSync
			SelectObject(hMemDC,hOldBm);
			DeleteDC(hMemDC);
			DeleteDC(hdc);
			for ( i = 0; i < 20 ; i++ ) {
				wndFill = GetDlgItem (hDlg, S1+i );
				GetClientRect(wndFill, &rc);
				hdc = GetDC(wndFill);
				rc.bottom-=1;
				rc.top+=1;
				rc.right-=1;
				rc.left+=1;
				
				if ( sig >= i * 12 ) {
                    hBrush=CreateSolidBrush(RGB(255-(12*i),12*i,0));
				} else {
					hBrush=CreateSolidBrush(RGB(0,0,0));
				};
				FillRect (hdc, &rc, hBrush);
				ReleaseDC (wndFill, hdc);
				DeleteObject(hBrush);
				
				wndFill = GetDlgItem (hDlg, E1+i );
				GetClientRect(wndFill, &rc);
				hdc = GetDC(wndFill);
				rc.bottom-=1;
				rc.top+=1;
				rc.right-=1;
				rc.left+=1;
				
				if ( err >= i * 12 ) {
                    hBrush=CreateSolidBrush(RGB(255-(12*i),12*i,0));
				} else {
					hBrush=CreateSolidBrush(RGB(0,0,0));
				};
				FillRect (hdc, &rc, hBrush);
				ReleaseDC (wndFill, hdc);
				DeleteObject(hBrush);
			}
			
			
            EndPaint(hDlg,&wps);
			return(FALSE);
			
			
			
			
        case WM_COMMAND:
			
            if ( (LOWORD(wParam) == IDC_USELNB2 ) ||
				(LOWORD(wParam) == IDC_USELNB3 ) || 
				(LOWORD(wParam) == IDC_USELNB4 )) {
				PostMessage(hDlg,WM_USER,1,1);
				return(FALSE);
			};
			
            if ( (LOWORD(wParam) == IDC_MINIDISEQ1 ) ||
				(LOWORD(wParam) == IDC_MINIDISEQ2 ) || 
				(LOWORD(wParam) == IDC_MINIDISEQ3 ) || 
				(LOWORD(wParam) == IDC_MINIDISEQ4 )) {
				PostMessage(hDlg,WM_USER,1,1);
				return(FALSE);
			};
			
			
            if ((LOWORD(wParam) == IDTEST ) ||
				(LOWORD(wParam) == IDTEST2 )) {
				
				if ( IsDlgButtonChecked(hDlg, IDC_RADIO1)) diseqnr = 0;
				else if ( IsDlgButtonChecked(hDlg, IDC_RADIO2)) diseqnr = 1;
				else if ( IsDlgButtonChecked(hDlg, IDC_RADIO3)) diseqnr = 2;
				else if ( IsDlgButtonChecked(hDlg, IDC_RADIO4)) diseqnr = 3;
				
				if ( LNB[diseqnr].Use == FALSE ) return(TRUE);
				
				if ( diseqnr == 0 ) {
					LNB[0].MinFreq=GetDlgItemInt(hDlg,IDC_MINFREQ1,NULL,FALSE);
					LNB[0].MaxFreq=GetDlgItemInt(hDlg,IDC_MAXFREQ1,NULL,FALSE);
					LNB[0].LofLow=GetDlgItemInt(hDlg,IDC_LOF_LOW1,NULL,FALSE);
					LNB[0].SwitchFreq=GetDlgItemInt(hDlg,IDC_LOF_SWITCH1,NULL,FALSE);
					LNB[0].LofHigh=GetDlgItemInt(hDlg,IDC_LOF_HIGH1,NULL,FALSE);
					LNB[0].Power=IsDlgButtonChecked(hDlg, IDC_LNBPOWER1 );
					LNB[0].PowerReset=IsDlgButtonChecked(hDlg, IDC_DISEQCRESET1 );
					LNB[0].Switch22khz=IsDlgButtonChecked(hDlg, IDC_LNBSWITSH22KHZ1 );
					LNB[0].MiniDiseqc=0x00;
					if ( IsDlgButtonChecked(hDlg, IDC_MINIDISEQ1 )) LNB[0].MiniDiseqc=0x01;
					if ( IsDlgButtonChecked(hDlg, IDC_TONEBURSTNONE1 ) == TRUE ) LNB[0].BurstVal = -1; 
					else if ( IsDlgButtonChecked(hDlg, IDC_TONEBURSTAUS1 ) == TRUE ) LNB[0].BurstVal = 0;  
					else if ( IsDlgButtonChecked(hDlg, IDC_TONEBURSTAN1 ) == TRUE ) LNB[0].BurstVal = 1;
					if ( IsDlgButtonChecked(hDlg, IDC_POSALNB1) == TRUE ) LNB[0].PosVal='A';
					else if ( IsDlgButtonChecked(hDlg, IDC_POSBLNB1) == TRUE ) LNB[0].PosVal='B';
					
					if ( IsDlgButtonChecked(hDlg, IDC_OPTALNB1) == TRUE ) LNB[0].OptVal='A';
					else if ( IsDlgButtonChecked(hDlg, IDC_OPTBLNB1) == TRUE ) LNB[0].OptVal='B';
				} else if ( diseqnr == 1 ) {
					LNB[1].MinFreq=GetDlgItemInt(hDlg,IDC_MINFREQ2,NULL,FALSE);
					LNB[1].MaxFreq=GetDlgItemInt(hDlg,IDC_MAXFREQ2,NULL,FALSE);
					LNB[1].LofLow=GetDlgItemInt(hDlg,IDC_LOF_LOW2,NULL,FALSE);
					LNB[1].SwitchFreq=GetDlgItemInt(hDlg,IDC_LOF_SWITCH2,NULL,FALSE);
					LNB[1].LofHigh=GetDlgItemInt(hDlg,IDC_LOF_HIGH2,NULL,FALSE);
					LNB[1].Power=IsDlgButtonChecked(hDlg, IDC_LNBPOWER2 );
					LNB[1].PowerReset=IsDlgButtonChecked(hDlg, IDC_DISEQCRESET2 );
					LNB[1].Switch22khz=IsDlgButtonChecked(hDlg, IDC_LNBSWITSH22KHZ2 );
					LNB[1].MiniDiseqc=0x00;
					if ( IsDlgButtonChecked(hDlg, IDC_MINIDISEQ2 )) LNB[1].MiniDiseqc=0x01;
					
					if ( IsDlgButtonChecked(hDlg, IDC_TONEBURSTNONE2 ) == TRUE ) LNB[1].BurstVal = -1; 
					else if ( IsDlgButtonChecked(hDlg, IDC_TONEBURSTAUS2 ) == TRUE ) LNB[1].BurstVal = 0;  
					else if ( IsDlgButtonChecked(hDlg, IDC_TONEBURSTAN2 ) == TRUE ) LNB[1].BurstVal = 1;
					if ( IsDlgButtonChecked(hDlg, IDC_POSALNB2) == TRUE ) LNB[1].PosVal='A';
					else if ( IsDlgButtonChecked(hDlg, IDC_POSBLNB2) == TRUE ) LNB[1].PosVal='B';
					
					if ( IsDlgButtonChecked(hDlg, IDC_OPTALNB2) == TRUE ) LNB[1].OptVal='A';
					else if ( IsDlgButtonChecked(hDlg, IDC_OPTBLNB2) == TRUE ) LNB[1].OptVal='B';
				} else if ( diseqnr == 2 ) {
					LNB[2].MinFreq=GetDlgItemInt(hDlg,IDC_MINFREQ3,NULL,FALSE);
					LNB[2].MaxFreq=GetDlgItemInt(hDlg,IDC_MAXFREQ3,NULL,FALSE);
					LNB[2].LofLow=GetDlgItemInt(hDlg,IDC_LOF_LOW3,NULL,FALSE);
					LNB[2].SwitchFreq=GetDlgItemInt(hDlg,IDC_LOF_SWITCH3,NULL,FALSE);
					LNB[2].LofHigh=GetDlgItemInt(hDlg,IDC_LOF_HIGH3,NULL,FALSE);
					LNB[2].PowerReset=IsDlgButtonChecked(hDlg, IDC_DISEQCRESET3 );
					LNB[2].Power=IsDlgButtonChecked(hDlg, IDC_LNBPOWER3 );
					LNB[2].Switch22khz=IsDlgButtonChecked(hDlg, IDC_LNBSWITSH22KHZ3 );
					LNB[2].MiniDiseqc=0x00;
					if ( IsDlgButtonChecked(hDlg, IDC_MINIDISEQ3 )) LNB[2].MiniDiseqc=0x01;
					
					if ( IsDlgButtonChecked(hDlg, IDC_TONEBURSTNONE3 ) == TRUE ) LNB[2].BurstVal = -1; 
					else if ( IsDlgButtonChecked(hDlg, IDC_TONEBURSTAUS3 ) == TRUE ) LNB[2].BurstVal = 0;  
					else if ( IsDlgButtonChecked(hDlg, IDC_TONEBURSTAN3 ) == TRUE ) LNB[2].BurstVal = 1;
					if ( IsDlgButtonChecked(hDlg, IDC_POSALNB3) == TRUE ) LNB[2].PosVal='A';
					else if ( IsDlgButtonChecked(hDlg, IDC_POSBLNB3) == TRUE ) LNB[2].PosVal='B';
					if ( IsDlgButtonChecked(hDlg, IDC_OPTALNB3) == TRUE ) LNB[2].OptVal='A';
					else if ( IsDlgButtonChecked(hDlg, IDC_OPTBLNB3) == TRUE ) LNB[2].OptVal='B';
					
				} else if ( diseqnr == 3 ) {
					LNB[3].MinFreq=GetDlgItemInt(hDlg,IDC_MINFREQ4,NULL,FALSE);
					LNB[3].MaxFreq=GetDlgItemInt(hDlg,IDC_MAXFREQ4,NULL,FALSE);
					LNB[3].LofLow=GetDlgItemInt(hDlg,IDC_LOF_LOW4,NULL,FALSE);
					LNB[3].SwitchFreq=GetDlgItemInt(hDlg,IDC_LOF_SWITCH4,NULL,FALSE);
					LNB[3].LofHigh=GetDlgItemInt(hDlg,IDC_LOF_HIGH4,NULL,FALSE);
					LNB[3].PowerReset=IsDlgButtonChecked(hDlg, IDC_DISEQCRESET4 );
					LNB[3].Power=IsDlgButtonChecked(hDlg, IDC_LNBPOWER4 );
					LNB[3].Switch22khz=IsDlgButtonChecked(hDlg, IDC_LNBSWITSH22KHZ4 );
					LNB[3].MiniDiseqc=0x00;
					if ( IsDlgButtonChecked(hDlg, IDC_MINIDISEQ4 )) LNB[3].MiniDiseqc=0x01;
					if ( IsDlgButtonChecked(hDlg, IDC_TONEBURSTNONE4 ) == TRUE ) LNB[3].BurstVal = -1; 
					else if ( IsDlgButtonChecked(hDlg, IDC_TONEBURSTAUS4 ) == TRUE ) LNB[3].BurstVal = 0;  
					else if ( IsDlgButtonChecked(hDlg, IDC_TONEBURSTAN4 ) == TRUE ) LNB[3].BurstVal = 1;
					if ( IsDlgButtonChecked(hDlg, IDC_POSALNB4) == TRUE ) LNB[3].PosVal='A';
					else if ( IsDlgButtonChecked(hDlg, IDC_POSBLNB4) == TRUE ) LNB[3].PosVal='B';
					if ( IsDlgButtonChecked(hDlg, IDC_OPTALNB4) == TRUE ) LNB[3].OptVal='A';
					else if ( IsDlgButtonChecked(hDlg, IDC_OPTBLNB4) == TRUE ) LNB[3].OptVal='B';
				}
				
				GetDlgItemText(hDlg,IDC_EDIT2,zeile,sizeof(zeile));
				
				ffreq=(float)atof(zeile);
				freq=(unsigned	long)(ffreq*1000);
				Transponder.freq=freq;
				Transponder.diseqc = diseqnr;
				if ( IsDlgButtonChecked(hDlg, IDC_RADIO20 ) == TRUE ) Transponder.volt = 1;
				else Transponder.volt=0;
				Transponder.power=LNB[diseqnr].Power;
				
				if ( IsDlgButtonChecked(hDlg, IDC_RADIO11)) Transponder.fec = 0;
				else if ( IsDlgButtonChecked(hDlg, IDC_RADIO12)) Transponder.fec = 1;
				else if ( IsDlgButtonChecked(hDlg, IDC_RADIO13)) Transponder.fec = 2;
				else if ( IsDlgButtonChecked(hDlg, IDC_RADIO14)) Transponder.fec = 3;
				else if ( IsDlgButtonChecked(hDlg, IDC_RADIO15)) Transponder.fec = 4;
				else if ( IsDlgButtonChecked(hDlg, IDC_RADIO16)) Transponder.fec = 5;
				else if ( IsDlgButtonChecked(hDlg, IDC_RADIO17)) Transponder.fec = 6;
				else if ( IsDlgButtonChecked(hDlg, IDC_RADIO18)) Transponder.fec = 7;
				else if ( IsDlgButtonChecked(hDlg, IDC_RADIO19)) Transponder.fec = 8;
				Transponder.srate=GetDlgItemInt(hDlg,IDC_EDIT6,NULL,FALSE);
				
				if (LOWORD(wParam) == IDTEST ) {
					(void)Set_Transponder_Trace(hDlg,&Transponder);
					SetDlgItemText(hDlg,TEXT20,"Synchronisiere mit Transponder......");
					Sleep(250);
					rc.top=400;
					rc.left=40;
					rc.bottom=500;
					rc.right=300;
					InvalidateRect(hDlg,&rc,FALSE);
					Get_Signal_Infos((unsigned char *)&Transponder.sync,&sig,&err);
					
					if ((Transponder.sync&0x0f)==0x0f) {
						for ( i=0; i < NEUSIZE ; i++ ) {
							memset(&ProgrammNeu[i].Name[0],0x00,sizeof( struct TProgramm));
							ProgrammNeu[i].Video_pid=0x1fff;        
							ProgrammNeu[i].Audio_pid=0x1fff;
							ProgrammNeu[i].TeleText_pid=0x1fff;
							ProgrammNeu[i].CA_Anzahl=0;
							ProgrammNeu[i].CA_Land[0]=0x00;
							ProgrammNeu[i].PMT_pid=0x1fff;
							ProgrammNeu[i].PCR_pid=0x1fff;
							ProgrammNeu[i].ECM_PID=0x1fff;
							ProgrammNeu[i].SID_pid=0x1fff;
							ProgrammNeu[i].AC3_pid=0x1fff;
							ProgrammNeu[i].Merker=0x00;
							ProgrammNeu[i].Link_SID=0xffff;
							ProgrammNeu[i].Link_TP=0xffff;
							ProgrammNeu[i].Dynamisch=0x00;
						};
						
						if ( Get_Pat() == TRUE ) {
							Transponder.ts_id=PAT.ts_id;
							sprintf(Zeile,"Synchronisiert mit Transponder ID %d",Transponder.ts_id);  
							SetDlgItemText(hDlg,TEXT20,Zeile);
						} else {
							sprintf(Zeile,"Synchronisiert aber keine TS-ID gefunden");  
							SetDlgItemText(hDlg,TEXT20,Zeile);
						};
					} else {
						sprintf(Zeile,"Kein Signal");  
						SetDlgItemText(hDlg,TEXT20,Zeile);
					};
				};
				
				if (LOWORD(wParam) == IDTEST2 ) {
					
					(void)Set_Transponder_Diseq_Scan(hDlg,&Transponder);
				};
				
				break;
			}
			
			
			if (LOWORD(wParam) == IDOK) {
				LNB[0].Use=TRUE; 
				LNB[1].Use=FALSE; 
				LNB[2].Use=FALSE; 
				LNB[3].Use=FALSE; 
				LNB[0].MinFreq=GetDlgItemInt(hDlg,IDC_MINFREQ1,NULL,FALSE);
				LNB[0].MaxFreq=GetDlgItemInt(hDlg,IDC_MAXFREQ1,NULL,FALSE);
				LNB[0].LofLow=GetDlgItemInt(hDlg,IDC_LOF_LOW1,NULL,FALSE);
				LNB[0].SwitchFreq=GetDlgItemInt(hDlg,IDC_LOF_SWITCH1,NULL,FALSE);
				LNB[0].LofHigh=GetDlgItemInt(hDlg,IDC_LOF_HIGH1,NULL,FALSE);
				LNB[0].PowerReset=IsDlgButtonChecked(hDlg, IDC_DISEQCRESET1 );
				LNB[0].Power=IsDlgButtonChecked(hDlg, IDC_LNBPOWER1 );
				LNB[0].Switch22khz=IsDlgButtonChecked(hDlg, IDC_LNBSWITSH22KHZ1 );
				if ( IsDlgButtonChecked(hDlg, IDC_TONEBURSTNONE1 ) == TRUE ) LNB[0].BurstVal = -1; 
                else if ( IsDlgButtonChecked(hDlg, IDC_TONEBURSTAUS1 ) == TRUE ) LNB[0].BurstVal = 0;  
				else if ( IsDlgButtonChecked(hDlg, IDC_TONEBURSTAN1 ) == TRUE ) LNB[0].BurstVal = 1;
                if ( IsDlgButtonChecked(hDlg, IDC_POSALNB1) == TRUE ) LNB[0].PosVal='A';
				else if ( IsDlgButtonChecked(hDlg, IDC_POSBLNB1) == TRUE ) LNB[0].PosVal='B';
				if ( IsDlgButtonChecked(hDlg, IDC_OPTALNB1) == TRUE ) LNB[0].OptVal='A';
				else if ( IsDlgButtonChecked(hDlg, IDC_OPTBLNB1) == TRUE ) LNB[0].OptVal='B';
				
				if (IsDlgButtonChecked(hDlg, IDC_USELNB2 ) == TRUE ) {
					LNB[1].Use=TRUE;
					LNB[1].MinFreq=GetDlgItemInt(hDlg,IDC_MINFREQ2,NULL,FALSE);
					LNB[1].MaxFreq=GetDlgItemInt(hDlg,IDC_MAXFREQ2,NULL,FALSE);
					LNB[1].LofLow=GetDlgItemInt(hDlg,IDC_LOF_LOW2,NULL,FALSE);
					LNB[1].SwitchFreq=GetDlgItemInt(hDlg,IDC_LOF_SWITCH2,NULL,FALSE);
					LNB[1].PowerReset=IsDlgButtonChecked(hDlg, IDC_DISEQCRESET2 );
					LNB[1].LofHigh=GetDlgItemInt(hDlg,IDC_LOF_HIGH2,NULL,FALSE);
					LNB[1].Power=IsDlgButtonChecked(hDlg, IDC_LNBPOWER2 );
					LNB[1].Switch22khz=IsDlgButtonChecked(hDlg, IDC_LNBSWITSH22KHZ2 );
					if ( IsDlgButtonChecked(hDlg, IDC_TONEBURSTNONE2 ) == TRUE ) LNB[1].BurstVal = -1; 
					else if ( IsDlgButtonChecked(hDlg, IDC_TONEBURSTAUS2 ) == TRUE ) LNB[1].BurstVal = 0;  
					else if ( IsDlgButtonChecked(hDlg, IDC_TONEBURSTAN2 ) == TRUE ) LNB[1].BurstVal = 1;
					if ( IsDlgButtonChecked(hDlg, IDC_POSALNB2) == TRUE ) LNB[1].PosVal='A';
					else if ( IsDlgButtonChecked(hDlg, IDC_POSBLNB2) == TRUE ) LNB[1].PosVal='B';
					
					if ( IsDlgButtonChecked(hDlg, IDC_OPTALNB2) == TRUE ) LNB[1].OptVal='A';
					else if ( IsDlgButtonChecked(hDlg, IDC_OPTBLNB2) == TRUE ) LNB[1].OptVal='B';
				};
				if (IsDlgButtonChecked(hDlg, IDC_USELNB3 ) == TRUE ) {
					LNB[2].Use=TRUE;
					LNB[2].MinFreq=GetDlgItemInt(hDlg,IDC_MINFREQ3,NULL,FALSE);
					LNB[2].MaxFreq=GetDlgItemInt(hDlg,IDC_MAXFREQ3,NULL,FALSE);
					LNB[2].LofLow=GetDlgItemInt(hDlg,IDC_LOF_LOW3,NULL,FALSE);
					LNB[2].PowerReset=IsDlgButtonChecked(hDlg, IDC_DISEQCRESET3 );
					LNB[2].SwitchFreq=GetDlgItemInt(hDlg,IDC_LOF_SWITCH3,NULL,FALSE);
					LNB[2].LofHigh=GetDlgItemInt(hDlg,IDC_LOF_HIGH3,NULL,FALSE);
					LNB[2].Power=IsDlgButtonChecked(hDlg, IDC_LNBPOWER3 );
					LNB[2].Switch22khz=IsDlgButtonChecked(hDlg, IDC_LNBSWITSH22KHZ3 );
					if ( IsDlgButtonChecked(hDlg, IDC_TONEBURSTNONE3 ) == TRUE ) LNB[2].BurstVal = -1; 
					else if ( IsDlgButtonChecked(hDlg, IDC_TONEBURSTAUS3 ) == TRUE ) LNB[2].BurstVal = 0;  
					else if ( IsDlgButtonChecked(hDlg, IDC_TONEBURSTAN3 ) == TRUE ) LNB[2].BurstVal = 1;
					if ( IsDlgButtonChecked(hDlg, IDC_POSALNB3) == TRUE ) LNB[2].PosVal='A';
					else if ( IsDlgButtonChecked(hDlg, IDC_POSBLNB3) == TRUE ) LNB[2].PosVal='B';
					if ( IsDlgButtonChecked(hDlg, IDC_OPTALNB3) == TRUE ) LNB[2].OptVal='A';
					else if ( IsDlgButtonChecked(hDlg, IDC_OPTBLNB3) == TRUE ) LNB[2].OptVal='B';
				};
				if (IsDlgButtonChecked(hDlg, IDC_USELNB4 ) == TRUE ) {
					LNB[3].Use=TRUE;
					
					LNB[3].MinFreq=GetDlgItemInt(hDlg,IDC_MINFREQ4,NULL,FALSE);
					LNB[3].MaxFreq=GetDlgItemInt(hDlg,IDC_MAXFREQ4,NULL,FALSE);
					LNB[3].LofLow=GetDlgItemInt(hDlg,IDC_LOF_LOW4,NULL,FALSE);
					LNB[3].SwitchFreq=GetDlgItemInt(hDlg,IDC_LOF_SWITCH4,NULL,FALSE);
					LNB[3].LofHigh=GetDlgItemInt(hDlg,IDC_LOF_HIGH4,NULL,FALSE);
					LNB[3].PowerReset=IsDlgButtonChecked(hDlg, IDC_DISEQCRESET4 );
					LNB[3].Power=IsDlgButtonChecked(hDlg, IDC_LNBPOWER4 );
					LNB[3].Switch22khz=IsDlgButtonChecked(hDlg, IDC_LNBSWITSH22KHZ4 );
					if ( IsDlgButtonChecked(hDlg, IDC_TONEBURSTNONE4 ) == TRUE ) LNB[3].BurstVal = -1; 
					else if ( IsDlgButtonChecked(hDlg, IDC_TONEBURSTAUS4 ) == TRUE ) LNB[3].BurstVal = 0;  
					else if ( IsDlgButtonChecked(hDlg, IDC_TONEBURSTAN4 ) == TRUE ) LNB[3].BurstVal = 1;
					if ( IsDlgButtonChecked(hDlg, IDC_POSALNB4) == TRUE ) LNB[3].PosVal='A';
					else if ( IsDlgButtonChecked(hDlg, IDC_POSBLNB4) == TRUE ) LNB[3].PosVal='B';
					if ( IsDlgButtonChecked(hDlg, IDC_OPTALNB4) == TRUE ) LNB[3].OptVal='A';
					else if ( IsDlgButtonChecked(hDlg, IDC_OPTBLNB4) == TRUE ) LNB[3].OptVal='B';
				}
				
                Write_INI("MultiDec.ini");
				EndDialog(hDlg,TRUE);
			}
            
			
			if (LOWORD(wParam) == IDCANCEL ) {
                Load_INI("MultiDec.ini");
				EndDialog(hDlg,TRUE);
			}
			
			break;
    }
	
	return(FALSE);
}

BOOL APIENTRY LNBSettingProcCable
(
 HWND hDlg,
 UINT message,
 UINT wParam,
 LONG lParam)
{
	
	char zeile[140];
    unsigned long freq;
	float ffreq;
	
    PAINTSTRUCT wps;                   /* paint structure           */
    HDC hdc;
    HDC             hMemDC;
    HBITMAP         hOldBm;
	BITMAP bm;
	
	
    HBRUSH hBrush;
    HWND wndFill;
	
	char Zeile[128];
	
	int i; 
	RECT rc;
	int sig;
	int err;
	
	
	switch (message) {
		
	case WM_INITDIALOG:
		Write_INI("Multidec.ini");
		Transponder.fec=Programm[CurrentProgramm].fec;
		Transponder.diseqc=Programm[CurrentProgramm].diseqc;
		Transponder.freq=Programm[CurrentProgramm].freq;
		Transponder.qam=Programm[CurrentProgramm].qam;
		Transponder.srate=Programm[CurrentProgramm].srate;
		Transponder.volt=Programm[CurrentProgramm].volt;
		Transponder.sync=dvb_front.sync;
		
		if ( Transponder.qam == 0 ) CheckDlgButton(hDlg, IDC_RADIO11, TRUE);
		else if ( Transponder.qam == 1 ) CheckDlgButton(hDlg, IDC_RADIO12, TRUE);
		else if ( Transponder.qam == 2 ) CheckDlgButton(hDlg, IDC_RADIO13, TRUE);
		else if ( Transponder.qam == 3 ) CheckDlgButton(hDlg, IDC_RADIO14, TRUE);
		else if ( Transponder.qam == 4 ) CheckDlgButton(hDlg, IDC_RADIO15, TRUE);

		if ( VES1820_Inversion == 0 ) CheckDlgButton(hDlg,IDC_RADIO1, TRUE);
		  else if ( VES1820_Inversion == 1 ) CheckDlgButton(hDlg,IDC_RADIO2, TRUE);
    		  else if ( VES1820_Inversion == 2 ) CheckDlgButton(hDlg,IDC_RADIO3, TRUE);

		SetDlgItemInt(hDlg,IDC_EDIT6,Transponder.srate,FALSE);
		sprintf(zeile,"%10.3f",Transponder.freq/1000.f);
		SetDlgItemText(hDlg,IDC_EDIT2,zeile);
		PostMessage(hDlg,WM_USER,1,1);
		SetTimer(hDlg,0,2000,NULL);
		
		
		return(TRUE);
		
	case WM_USER:
		
		if ( wParam == 1 ) {
			LNB[0].Use=TRUE;
			SetDlgItemInt(hDlg,IDC_MINFREQ1,LNB[0].MinFreq,FALSE);
			SetDlgItemInt(hDlg,IDC_MAXFREQ1,LNB[0].MaxFreq,FALSE);
			CheckDlgButton(hDlg, IDC_LNBPOWER1, LNB[0].Power);
		};
		
		break;			
		
		
	case WM_TIMER:
		Get_Signal_Infos((unsigned char *)&Transponder.sync,&sig,&err);
		rc.top=150;
		rc.left=410;
		rc.bottom=182;
		rc.right=640;
		if ( Current_Mode == 1 ) InvalidateRect(hDlg,&rc,FALSE);
		break;
		
		
	case WM_PAINT:
		
		Get_Signal_Infos((unsigned char *)&Transponder.sync,&sig,&err);
		hdc = BeginPaint(hDlg,&wps);
		hMemDC = CreateCompatibleDC(hdc);
		if (( Transponder.sync&0x0f) > 0 ) hOldBm = SelectObject(hMemDC, BirneGruen);
		else hOldBm = SelectObject(hMemDC, BirneRot );
		GetObject(BirneGruen, sizeof(BITMAP), (LPSTR)&bm);
		
		BitBlt(hdc, 60, 286, bm.bmWidth, bm.bmHeight,hMemDC, 0, 0, SRCCOPY);// Signal
		
		SelectObject(hMemDC,hOldBm);
		if (( Transponder.sync&0x0f) > 1 ) hOldBm = SelectObject(hMemDC, BirneGruen);
		else hOldBm = SelectObject(hMemDC, BirneRot );
		
		BitBlt(hdc, 125, 286, bm.bmWidth, bm.bmHeight,hMemDC, 0, 0, SRCCOPY); // Carrier
		SelectObject(hMemDC,hOldBm);
		
		
		if (( Transponder.sync&0x0f) > 3 ) hOldBm = SelectObject(hMemDC, BirneGruen);
		else hOldBm = SelectObject(hMemDC, BirneRot );
		
		BitBlt(hdc, 195, 286, bm.bmWidth, bm.bmHeight,hMemDC, 0, 0, SRCCOPY); // Viterbi
		SelectObject(hMemDC,hOldBm);
		
		if (( Transponder.sync&0x0f) > 7 ) hOldBm = SelectObject(hMemDC, BirneGruen);
		else hOldBm = SelectObject(hMemDC, BirneRot );
		
		BitBlt(hdc, 265, 286, bm.bmWidth, bm.bmHeight,hMemDC, 0, 0, SRCCOPY); // FSync
		SelectObject(hMemDC,hOldBm);
		DeleteDC(hMemDC);
		DeleteDC(hdc);
		
		for ( i = 0; i < 20 ; i++ ) {
            wndFill = GetDlgItem (hDlg, E1+i );
			GetClientRect(wndFill, &rc);
            hdc = GetDC(wndFill);
			rc.bottom-=1;
			rc.top+=1;
			rc.right-=1;
			rc.left+=1;
			
			if ( err >= i * 12 ) {
				hBrush=CreateSolidBrush(RGB(255-(12*i),12*i,0));
			} else {
				hBrush=CreateSolidBrush(RGB(0,0,0));
			};
			FillRect (hdc, &rc, hBrush);
			ReleaseDC (wndFill, hdc);
			DeleteObject(hBrush);
		}
		
		
		
		EndPaint(hDlg,&wps);
		return(FALSE);
		
		
		
		
	case WM_COMMAND:
		
		if (LOWORD(wParam) == IDTEST ) {

		if ( VES1820_Inversion == 0 ) if ( IsDlgButtonChecked(hDlg,IDC_RADIO1) == TRUE ) VES1820_Inversion = 0;
		  else if ( IsDlgButtonChecked(hDlg,IDC_RADIO2) == TRUE ) VES1820_Inversion = 1;
			  else if ( IsDlgButtonChecked(hDlg,IDC_RADIO3) == TRUE ) VES1820_Inversion = 2;

			LNB[0].MinFreq=GetDlgItemInt(hDlg,IDC_MINFREQ1,NULL,FALSE);
			LNB[0].MaxFreq=GetDlgItemInt(hDlg,IDC_MAXFREQ1,NULL,FALSE);
			LNB[0].Power=IsDlgButtonChecked(hDlg, IDC_LNBPOWER1 );
			
			GetDlgItemText(hDlg,IDC_EDIT2,zeile,sizeof(zeile));
			
			ffreq=(float)atof(zeile);
			freq=(unsigned	long)(ffreq*1000);
			Transponder.freq=freq;
			Transponder.diseqc = 0;
			
			Transponder.power=LNB[0].Power;
			
			if ( IsDlgButtonChecked(hDlg, IDC_RADIO11)) Transponder.qam = 0;
			else if ( IsDlgButtonChecked(hDlg, IDC_RADIO12)) Transponder.qam = 1;
			else if ( IsDlgButtonChecked(hDlg, IDC_RADIO13)) Transponder.qam = 2;
			else if ( IsDlgButtonChecked(hDlg, IDC_RADIO14)) Transponder.qam = 3;
			else if ( IsDlgButtonChecked(hDlg, IDC_RADIO15)) Transponder.qam = 4;
			Transponder.srate=GetDlgItemInt(hDlg,IDC_EDIT6,NULL,FALSE);
			
				(void)Set_Transponder(&Transponder);
				SetDlgItemText(hDlg,TEXT20,"Synchronisiere mit Transponder......");
				Sleep(250);
				rc.top=400;
				rc.left=40;
				rc.bottom=500;
				rc.right=300;
				InvalidateRect(hDlg,&rc,FALSE);
				Get_Signal_Infos((unsigned char *)&Transponder.sync,&sig,&err);
				
				if ((Transponder.sync&0x0f)==0x0f) {
					for ( i=0; i < NEUSIZE ; i++ ) {
						memset(&ProgrammNeu[i].Name[0],0x00,sizeof( struct TProgramm));
						ProgrammNeu[i].Video_pid=0x1fff;        
						ProgrammNeu[i].Audio_pid=0x1fff;
						ProgrammNeu[i].TeleText_pid=0x1fff; 
						ProgrammNeu[i].CA_Anzahl=0;
						ProgrammNeu[i].CA_Land[0]=0x00;
						ProgrammNeu[i].PMT_pid=0x1fff;
						ProgrammNeu[i].PCR_pid=0x1fff;
						ProgrammNeu[i].ECM_PID=0x1fff;
						ProgrammNeu[i].SID_pid=0x1fff;
						ProgrammNeu[i].AC3_pid=0x1fff;
						ProgrammNeu[i].Merker=0x00;
						ProgrammNeu[i].Link_SID=0xffff;
						ProgrammNeu[i].Link_TP=0xffff;
						ProgrammNeu[i].Dynamisch=0x00;
					};
					
					if ( Get_Pat() == TRUE ) {
						Transponder.ts_id=PAT.ts_id;
						sprintf(Zeile,"Synchronisiert mit Transponder ID %d",Transponder.ts_id);  
						SetDlgItemText(hDlg,TEXT20,Zeile);
					} else {
						sprintf(Zeile,"Synchronisiert aber keine TS-ID gefunden");  
						SetDlgItemText(hDlg,TEXT20,Zeile);
					};
				} else {
					sprintf(Zeile,"Kein Signal");  
					SetDlgItemText(hDlg,TEXT20,Zeile);
				};
			
			break;
		}
		
		
		if (LOWORD(wParam) == IDOK) {
		if ( VES1820_Inversion == 0 ) if ( IsDlgButtonChecked(hDlg,IDC_RADIO1) == TRUE ) VES1820_Inversion = 0;
		  else if ( IsDlgButtonChecked(hDlg,IDC_RADIO2) == TRUE ) VES1820_Inversion = 1;
			  else if ( IsDlgButtonChecked(hDlg,IDC_RADIO3) == TRUE ) VES1820_Inversion = 2;
			LNB[0].Use=TRUE; 
			LNB[1].Use=FALSE; 
			LNB[2].Use=FALSE; 
			LNB[3].Use=FALSE; 
			LNB[0].MinFreq=GetDlgItemInt(hDlg,IDC_MINFREQ1,NULL,FALSE);
			LNB[0].MaxFreq=GetDlgItemInt(hDlg,IDC_MAXFREQ1,NULL,FALSE);
			LNB[0].Power=IsDlgButtonChecked(hDlg, IDC_LNBPOWER1 );
			Write_INI("MultiDec.ini");
			EndDialog(hDlg,TRUE);
		}
		
		
		if (LOWORD(wParam) == IDCANCEL ) {
			Load_INI("MultiDec.ini");
			EndDialog(hDlg,TRUE);
		}
		
		break;
    }
	
	return(FALSE);
}



BOOL APIENTRY SecamSettingProc
(
 HWND hDlg,
 UINT message,
 UINT wParam,
 LONG lParam)
{
	int x,y,i;
	
	switch (message) {
	case WM_INITDIALOG:
		
		TUFilter=iUFilterThresholdPCent;
		TVFilter=iVFilterThresholdPCent;
		
		SetDlgItemInt(hDlg,IDC_D1,iUFilterThresholdPCent,FALSE);
		SetDlgItemInt(hDlg,IDC_D2,iVFilterThresholdPCent,FALSE);
		
		y=(int)(235-((double)(TUFilter+1)/100*160));
		MoveWindow(GetDlgItem(hDlg,IDC_S1),63,y-4,22,8,FALSE);
		
		y=(int)(235-((double)(TVFilter+1)/100*160));
		MoveWindow(GetDlgItem(hDlg,IDC_S2),138,y-4,22,8,FALSE);
		break;
		
		
		
	case WM_MOUSEMOVE:
		
		y=HIWORD( lParam );
		x=LOWORD( lParam );
		if (wParam == MK_LBUTTON) {
			if (( y >= 70) && ( y <= 240 )) {
				if (( x >= 63) && ( x <= 85 )) {
					i=(int)((double)(y-235)/160*100);
					i=-i;
					if ( i < 0 ) i=1;
					else if ( i > 100 ) i=100;
					MoveWindow(GetDlgItem(hDlg,IDC_S1),63,y-2,22,8,TRUE);
					iUFilterThresholdPCent=i;
					SetDlgItemInt(hDlg,IDC_D1,iUFilterThresholdPCent,FALSE);
				}
				if (( x >= 139) && ( x <= 160 )) {
					i=(int)((double)(y-235)/160*100);
					i=-i;
					if ( i < 0 ) i=1;
					else if ( i > 100 ) i=100;
					MoveWindow(GetDlgItem(hDlg,IDC_S2),138,y-2,22,8,TRUE);
					iVFilterThresholdPCent=i;
					SetDlgItemInt(hDlg,IDC_D2,iVFilterThresholdPCent,FALSE);
				}
			}              
			
			
		}
		return(FALSE);
	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK ) {
			EndDialog(hDlg,TRUE);
		}
		
		
		if (LOWORD(wParam) == IDCANCEL ) {
			iUFilterThresholdPCent=TUFilter;
			iVFilterThresholdPCent=TVFilter;
			EndDialog(hDlg,TRUE);
		}
		
		
		
		if ( LOWORD(wParam) == IDDEFAULT) {
			
			iUFilterThresholdPCent=70;
			iVFilterThresholdPCent=70;
			
			SetDlgItemInt(hDlg,IDC_D1,iUFilterThresholdPCent,FALSE);
			SetDlgItemInt(hDlg,IDC_D2,iVFilterThresholdPCent,FALSE);
			y=(int)(235-((double)(iUFilterThresholdPCent+1)/100*160));
			MoveWindow(GetDlgItem(hDlg,IDC_S1),63,y-4,22,8,TRUE);
			y=(int)(235-((double)(iVFilterThresholdPCent+1)/100*160));
			MoveWindow(GetDlgItem(hDlg,IDC_S2),138,y-4,22,8,TRUE);
			break;
			
		}
		
		break;
    }
	
	return(FALSE);
}



BOOL APIENTRY AudioSettingProc1
(
 HWND hDlg,
 UINT message,
 UINT wParam,
 LONG lParam)
{
	int x,y,i;
	
	switch (message) {
	case WM_INITDIALOG:
		if ( Has_MSP == FALSE ) {
			MessageBox(hWnd,"Kein MSP-Audio-Device gefunden","Multi-Decode",MB_ICONSTOP|MB_OK);
			EndDialog(hDlg,0);
		}
		
		TEqualizer1=InitialEqualizer1;
		TEqualizer2=InitialEqualizer2;
		TEqualizer3=InitialEqualizer3;
		TEqualizer4=InitialEqualizer4;
		TEqualizer5=InitialEqualizer5;
		
		SetDlgItemInt(hDlg,IDC_D1,TEqualizer1,TRUE);
		SetDlgItemInt(hDlg,IDC_D2,TEqualizer2,TRUE);
		SetDlgItemInt(hDlg,IDC_D3,TEqualizer3,TRUE);
		SetDlgItemInt(hDlg,IDC_D4,TEqualizer4,TRUE);
		SetDlgItemInt(hDlg,IDC_D5,TEqualizer5,TRUE);
		
		y=(int)(157-((double)(TEqualizer1)/192*210));
		MoveWindow(GetDlgItem(hDlg,IDC_S1),33,y-4,22,8,FALSE);
		
		y=(int)(157-((double)(TEqualizer2)/192*210));
		MoveWindow(GetDlgItem(hDlg,IDC_S2),109,y-4,22,8,FALSE);
		
		y=(int)(157-((double)(TEqualizer3)/192*210));
		MoveWindow(GetDlgItem(hDlg,IDC_S3),183,y-4,22,8,FALSE);
		
		y=(int)(157-((double)(TEqualizer4)/192*210));
		MoveWindow(GetDlgItem(hDlg,IDC_S4),258,y-4,22,8,FALSE);
		
		y=(int)(157-((double)(TEqualizer5)/192*210));
		MoveWindow(GetDlgItem(hDlg,IDC_S5),334,y-4,22,8,FALSE);
		break;
		
		
		
	case WM_MOUSEMOVE:
		
		y=HIWORD( lParam );
		x=LOWORD( lParam );
		if (wParam == MK_LBUTTON) {
			if (( y >= 70) && ( y <= 240 )) {
				if (( x >= 33) && ( x <= 55 )) {
					i=(int)((double)(y-155)/210*192);
					if ( i < -69) i=-69;
					else if ( i > 69 ) i=69;
					i=-i;
					MoveWindow(GetDlgItem(hDlg,IDC_S1),33,y-2,22,8,TRUE);
					InitialEqualizer1=i;
					SetDlgItemInt(hDlg,IDC_D1,InitialEqualizer1,TRUE);
					Audio_SetEqualizer1(InitialEqualizer1);
				}
				if (( x >= 109) && ( x <= 131 )) {
					i=(int)((double)(y-155)/210*192);
					if ( i < -69) i=-69;
					else if ( i > 69 ) i=69;
					i=-i;
					MoveWindow(GetDlgItem(hDlg,IDC_S2),109,y-2,22,8,TRUE);
					InitialEqualizer2=i;
					SetDlgItemInt(hDlg,IDC_D2,InitialEqualizer2,TRUE);
					Audio_SetEqualizer2(InitialEqualizer2);
				}
				if (( x >= 183) && ( x <= 205 )) {
					i=(int)((double)(y-155)/210*192);
					if ( i < -69) i=-69;
					else if ( i > 69 ) i=69;
					i=-i;
					MoveWindow(GetDlgItem(hDlg,IDC_S3),183,y-2,22,8,TRUE);
					InitialEqualizer3=i;
					SetDlgItemInt(hDlg,IDC_D3,InitialEqualizer3,TRUE);
					Audio_SetEqualizer3(InitialEqualizer3);
				}
				if (( x >= 258) && ( x <= 280 )) {
					i=(int)((double)(y-155)/210*192);
					if ( i < -69) i=-69;
					else if ( i > 69 ) i=69;
					i=-i;
					MoveWindow(GetDlgItem(hDlg,IDC_S4),258,y-2,22,8,TRUE);
					InitialEqualizer4=i;
					SetDlgItemInt(hDlg,IDC_D4,InitialEqualizer4,TRUE);
					Audio_SetEqualizer4(InitialEqualizer4);
				}
				if (( x >= 334 ) && ( x <= 356 )) {
					i=(int)((double)(y-155)/210*192);
					if ( i < -69) i=-69;
					else if ( i > 69 ) i=69;
					i=-i;
					MoveWindow(GetDlgItem(hDlg,IDC_S5),334,y-2,22,8,TRUE);
					InitialEqualizer5=i;
					SetDlgItemInt(hDlg,IDC_D5,InitialEqualizer5,TRUE);
					Audio_SetEqualizer5(InitialEqualizer5);
				}
			}
			
		}
		return(FALSE);
	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK ) {
			SetAudioSource(4);
			Audio_SetEqualizer1(InitialEqualizer1);
			Audio_SetEqualizer2(InitialEqualizer2);
			Audio_SetEqualizer3(InitialEqualizer3);
			Audio_SetEqualizer4(InitialEqualizer4);
			Audio_SetEqualizer5(InitialEqualizer5);
			SetAudioSource(AudioSource);
			EndDialog(hDlg,TRUE);
		}
		
		
		if (LOWORD(wParam) == IDCANCEL ) {
			SetAudioSource(4);
			Audio_SetEqualizer1(TEqualizer1);
			Audio_SetEqualizer2(TEqualizer2);
			Audio_SetEqualizer3(TEqualizer3);
			Audio_SetEqualizer4(TEqualizer4);
			Audio_SetEqualizer5(TEqualizer5);
			SetAudioSource(AudioSource);
			EndDialog(hDlg,TRUE);
		}
		
		
		
		if ( LOWORD(wParam) == IDDEFAULT) {
			
            TEqualizer1=0x00;
			TEqualizer2=0x00;
			TEqualizer3=0x00;
            TEqualizer4=0x00;
            TEqualizer5=0x00;
			SetAudioSource(4);
			Audio_SetEqualizer1(0);
			Audio_SetEqualizer2(0);
			Audio_SetEqualizer3(0);
			Audio_SetEqualizer4(0);
			Audio_SetEqualizer5(0);
            SetAudioSource(AudioSource);
			SetDlgItemInt(hDlg,IDC_D1,TEqualizer1,TRUE);
			SetDlgItemInt(hDlg,IDC_D2,TEqualizer2,TRUE);
			SetDlgItemInt(hDlg,IDC_D3,TEqualizer3,TRUE);
			SetDlgItemInt(hDlg,IDC_D4,TEqualizer4,TRUE);
			SetDlgItemInt(hDlg,IDC_D5,TEqualizer5,TRUE);
			
			y=(int)(157-((double)(TEqualizer1)/192*210));
			MoveWindow(GetDlgItem(hDlg,IDC_S1),33,y-4,22,8,TRUE);
			
			y=(int)(157-((double)(TEqualizer2)/192*210));
			MoveWindow(GetDlgItem(hDlg,IDC_S2),109,y-4,22,8,TRUE);
			
			y=(int)(157-((double)(TEqualizer3)/192*210));
			MoveWindow(GetDlgItem(hDlg,IDC_S3),183,y-4,22,8,TRUE);
			
			y=(int)(157-((double)(TEqualizer4)/192*210));
			MoveWindow(GetDlgItem(hDlg,IDC_S4),258,y-4,22,8,TRUE);
			
			y=(int)(157-((double)(TEqualizer5)/192*210));
			MoveWindow(GetDlgItem(hDlg,IDC_S5),334,y-4,22,8,TRUE);
			break;
			
		}
		
		break;
    }
	
	return(FALSE);
}



BOOL APIENTRY ResSettingProc
(
 HWND hDlg,
 UINT message,
 UINT wParam,
 LONG lParam)
{
	int i,j;
	int Boot_Up_Loop=0;
	
	switch (message) {
	case WM_INITDIALOG:
		Stop_Thread();
		Set_Capture(4);
		OutDib=TRUE;
		InitTV(hWnd,FALSE,640,480);
		MoveWindow(hWnd,0,0,640,480,TRUE);
		Init_Screen_Struct();
		emstartx=Res_X;
		emstarty=Res_Y;
		Set_Capture(5);
		SetDlgItemInt(hDlg,IDC_RESX,Res_X,FALSE);
		SetDlgItemInt(hDlg,IDC_RESY,Res_Y,FALSE);
		break;
	case WM_TIMER:
		if (LOWORD(wParam) == 2 ) {
			KillTimer(hDlg,2);
			Stop_Thread();
			Set_Capture(4);
			InitTV(hWnd,FALSE,640,480);
			MoveWindow(hWnd,0,0,640,480,TRUE);
			ShowWindow(hDlg,SW_SHOW);
			EnableWindow(GetDlgItem(hDlg,IDOK),FALSE);
			Init_Screen_Struct();
			Set_Capture(5);
			if  ( MessageBox(hDlg,"War es in Ordnung ?? ","Multi-Dekoder",MB_ICONQUESTION | MB_YESNO ) == IDYES ) EnableWindow(GetDlgItem(hDlg,IDOK),TRUE);
		}
		break;
		
	case WM_COMMAND:
		
		if (LOWORD(wParam) == IDC_BUTTON1 ) {
			i=GetDlgItemInt(hDlg,IDC_RESX,NULL,FALSE);
			j=GetDlgItemInt(hDlg,IDC_RESY,NULL,FALSE);
			Stop_Thread();
			Set_Capture(4);
			if (  InitTV(hWnd,TRUE,i,j) == FALSE ) {
				i=0;
				InitTV(hWnd,FALSE,640,480);
				MoveWindow(hWnd,0,0,640,480,TRUE);
			}
			Init_Screen_Struct();
			Set_Capture(5);
			if ( i != 0 ) {
				ShowWindow(hDlg,SW_HIDE);
				SetTimer(hDlg,2,10000,NULL);
			}
		}
		
		
		if (LOWORD(wParam) == IDOK ) {
			
			Res_X=GetDlgItemInt(hDlg,IDC_RESX,NULL,FALSE);
			Res_Y=GetDlgItemInt(hDlg,IDC_RESY,NULL,FALSE);
			Stop_Thread();
			Set_Capture(4);
			if (  InitTV(hWnd,TRUE,Res_X,Res_Y) == FALSE ) {
				InitTV(hWnd,FALSE,640,480);
				MoveWindow(hWnd,0,0,640,480,TRUE);
				Init_Screen_Struct();
				Set_Capture(5);
			} else {
				WindowStyle=3;
				CheckMenuItem( GetMenu(hWnd), IDM_FENSTER, MF_UNCHECKED );
				CheckMenuItem( GetMenu(hWnd), IDM_640x480, MF_UNCHECKED );
				CheckMenuItem( GetMenu(hWnd), IDM_800x600, MF_UNCHECKED );
				CheckMenuItem( GetMenu(hWnd), IDM_OWN_RES, MF_CHECKED );
				
				Init_Screen_Struct();
				Set_Capture(5);
				EndDialog(hDlg,TRUE);
			}
		}
		
		if (LOWORD(wParam) == IDCANCEL ) {
			Res_X=emstartx;
			Res_Y=emstarty;
			EndDialog(hDlg,TRUE);
		}
		
		break;
    }
	
	return(FALSE);
}


void Write_Programm_List()
{
	int i,j;
	int fd;
	
	struct TProgramm VersionID;
	
    strcpy(VersionID.Name,"MultiDec 8.2");
	
	if ((fd=open("Programm.set" ,_O_WRONLY | _O_TRUNC | _O_CREAT | _O_BINARY , _S_IWRITE | _S_IREAD)) == -1 ) {
		MessageBox(hWnd,"Kann Programmdatei <programm.set> nicht schreiben","Multi-Decode",MB_ICONSTOP|MB_OK);
		return;
	};
	
	j=write(fd,&VersionID,sizeof(struct TProgramm));
	
	i=0;
	while ( i < MAXPROGS ) {
		if ( Programm[i].Name[0] != 0x00 ) {
			j=write(fd,&Programm[i],sizeof(struct TProgramm));
		};
		i++;
	};
	close(fd);
};

void Write_Nit_List()
{
	int i,j,insert_to,Low;
	int fd;
	
	unsigned int freq;
    unsigned char diseq;
	
	BOOL Found_More=TRUE;
	BOOL Found_Diseq=TRUE;
	
    struct tNIT Sort_NIT[MAXNIT];
	
	
	for ( i = 0; i < MAXNIT; i++ ) {
		memset(&Sort_NIT[i].DiseqNr,0x00,sizeof( struct tNIT));
	};
	diseq=0x00;
	insert_to=0;
	
	while ( Found_More == TRUE ) {
        Found_More=FALSE;
		Found_Diseq=FALSE;
		Low=-1;
		freq=0xffffffff;
		for ( i = 0; i < MAXNIT; i++ ) {
            if ( NIT[i].freq != 0 ) {
				Found_More=TRUE;
				if ( NIT[i].DiseqNr == diseq ) {
					Found_Diseq=TRUE;
					if ( (NIT[i].freq/1000) < freq ) {
						freq=(NIT[i].freq/1000);
						Low=i;
					} else if ( (NIT[i].freq/1000) == freq ) {
						memset(&NIT[i].DiseqNr,0x00,sizeof( struct tNIT));
					};
					
					
				}
			}
		};
		if ( Low != -1 ) {
			memcpy(&Sort_NIT[insert_to].DiseqNr,&NIT[Low].DiseqNr,sizeof( struct tNIT ));
			memset(&NIT[Low].DiseqNr,0x00,sizeof( struct tNIT));
			insert_to++;
		};
		if ( Found_Diseq == FALSE ) diseq++;
		  };
	
	
	if ((fd=open("Nit.set" ,_O_WRONLY | _O_TRUNC | _O_CREAT | _O_BINARY , _S_IWRITE | _S_IREAD)) == -1 ) {
		MessageBox(hWnd,"Kann Programmdatei <Nit.set> nicht schreiben","Multi-Decode",MB_ICONSTOP|MB_OK);
		return;
	};
	
	i=0;
	while ( i < MAXNIT ) {
		if ( Sort_NIT[i].freq != 0x00 ) {
			j=write(fd,&Sort_NIT[i],sizeof(struct tNIT));
			memcpy(&NIT[i].DiseqNr,&Sort_NIT[i].DiseqNr,sizeof( struct tNIT ));
		};
		i++;
	};
	close(fd);
};




void Load_Programm_List()
{
	int i;
    int j;
	int fd;

struct TCA_SystemAlt {
		 unsigned short CA_Typ;
		 unsigned short ECM;
		 unsigned short EMM;
};
	
	typedef struct TProgrammAlt
{
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
	unsigned short    tp_id;        
	unsigned short    Video_pid;        
	unsigned short    Audio_pid;
    unsigned short    TeleText_pid;          /* Teletext PID */
	unsigned short    PMT_pid;
    unsigned short    PCR_pid;
	unsigned short    ECM_PID;
	unsigned short    SID_pid;
	unsigned short    AC3_pid;
	unsigned char     TVType; //  == 00 PAL ; 11 == NTSC    
	unsigned char     ServiceTyp;
    unsigned char     CA_ID;
	unsigned short    Temp_Audio;
	unsigned short    Filteranzahl;
    struct PIDFilters Filters[12];
	unsigned short    CA_Anzahl;
    struct TCA_SystemAlt CA_System[6];
    char    CA_Land[5];
    unsigned char Merker;
    unsigned short Link_TP;
    unsigned short Link_SID;
    unsigned char Dynamisch;

    char Extern_Buffer[16];
};
	
	struct TProgrammAlt ProgrammAlt;
	
	
	
	i=0;
	for ( i=0; i < MAXPROGS; i++ ) memset(&Programm[i].Name[0],0x00,sizeof(struct TProgramm));
	i=0;
	if ((fd=open("Programm.set" ,_O_RDONLY | _O_BINARY )) == -1 ) {
		strcpy(Programm[0].Name,"MultiDec 8.2");
		CurrentProgramm=0;
		return;
	};
	
	j= read(fd,&Programm[0],sizeof(Programm[0]));
	if ( j < sizeof(Programm[0] )) {
		memset(&Programm[0].Name[0],0x00,sizeof(struct TProgramm));
		close(fd);
		return;
	};
	
	if (( strcmp(Programm[0].Name,"MultiDec 8.2") == 0 ) && ( strlen(Programm[0].Name) > 0 )) {
		// ist schon neue struktur 	
		i=0;
		while ( sizeof(Programm[i]) == read(fd,&Programm[i],sizeof(Programm[i]))) {
			Programm[i].Merker=0x00;
			if (( Programm[i].Dynamisch == 0 )  &&
				( Programm[i].Link_TP!= 0xffff )  &&
				( Programm[i].Link_SID != 0xffff ))  {
				Programm[i].Dynamisch = 0x01;
				Programm[i].ServiceTyp = 0x05;
			};
			if (( Programm[i].Typ == 'D' ) || ( Programm[i].Typ == 'A' )) i++; 
			
		};
		close(fd);
		return;
	}
	// Konvertieren
	
	_lseek(fd,0,SEEK_SET);
	
	i=0;
	while ( sizeof(ProgrammAlt) == read(fd,&ProgrammAlt,sizeof(ProgrammAlt))) {
		memcpy(&Programm[i],&ProgrammAlt,sizeof(struct TProgrammAlt));


       for ( j=0; j < 12 ; j++ ) {
			Programm[i].Filters[j].FilterId=ProgrammAlt.Filters[j].FilterId;
			Programm[i].Filters[j].PID=ProgrammAlt.Filters[j].PID;
			memcpy(&Programm[i].Filters[j].FilterName[0],&ProgrammAlt.Filters[j].FilterName[0],5);
	   };
            Programm[i].CA_Anzahl=ProgrammAlt.CA_Anzahl;

        for ( j=0; j < 6 ; j++ ) {
			Programm[i].CA_System[j].CA_Typ=ProgrammAlt.CA_System[j].CA_Typ;
			Programm[i].CA_System[j].ECM=ProgrammAlt.CA_System[j].ECM;
			Programm[i].CA_System[j].EMM=ProgrammAlt.CA_System[j].EMM;
            Programm[i].CA_System[j].Provider_Id=-1;
		};
            memcpy(&Programm[i].CA_Land[0],&ProgrammAlt.CA_Land[0],5);
			Programm[i].Merker=ProgrammAlt.Merker;
			Programm[i].Link_TP=ProgrammAlt.Link_TP;
			Programm[i].Link_SID=ProgrammAlt.Link_SID;
			Programm[i].Dynamisch=ProgrammAlt.Dynamisch;
            memcpy(&Programm[i].Extern_Buffer[0],&ProgrammAlt.Extern_Buffer[0],16);
		i++;
	};
	close(fd);
	Write_Programm_List();
}


void Load_Nit_List()
{
	int i;
	int fd;
	
	for ( i = 0; i < MAXNIT ; i++ ) {
		NIT[i].DiseqNr=0;
		NIT[i].freq=0;
		NIT[i].ts_id=0;
	};
	
	i=0;
	if ((fd=open("Nit.set" ,_O_RDONLY | _O_BINARY )) == -1 ) return;
	while ( sizeof(NIT[i]) == read(fd,&NIT[i],sizeof(NIT[i]))) {
		i++;
	}
	close(fd);
	
}


BOOL APIENTRY SplashProc(
						 HWND hDlg,
						 UINT message,
						 UINT wParam,
						 LONG lParam)
{
	
	static HBITMAP BitmapInfo;
	
    static int Count=0;
	HDC wDC;
    PAINTSTRUCT wps;                  
	BITMAP bm;
    HBITMAP         hOldBm;
    RECT            rect;
    HDC             hMemDC;
	
	
	switch (message) {
	case WM_INITDIALOG: 
		//			ShowWindow(hDlg,SW_SHOW);
		BitmapInfo= LoadBitmap(hInst,"STARTUP");
		
		InvalidateRect(hDlg,NULL,TRUE); 
		SetTimer(hDlg,1,2000,NULL);
		return(TRUE);
		
	case WM_TIMER:
		if ( wParam == 1 ) {
			Count++;
			DeleteObject(BitmapInfo);
			if ( Count == 1 ) BitmapInfo= LoadBitmap(hInst,"STARTUP1");
			if ( Count == 2 ) BitmapInfo= LoadBitmap(hInst,"STARTUP2");
			if ( Count == 3 ) BitmapInfo= LoadBitmap(hInst,"STARTUP3");
			if ( Count == 4 ) BitmapInfo= LoadBitmap(hInst,"STARTUP4");
			if ( Count == 5 ) BitmapInfo= LoadBitmap(hInst,"STARTUP5");
			if ( Count == 6 ) {
 			            KillTimer(hDlg,1); 
						EndDialog(hDlg,0);
			};
			InvalidateRect(hDlg,NULL,FALSE); 
			return(TRUE);
		}
		
		
	case WM_PAINT:
		wDC=BeginPaint(hDlg,&wps);
		GetClientRect(hDlg, &rect);
		hMemDC = CreateCompatibleDC(wDC);
		hOldBm = SelectObject(hMemDC, BitmapInfo);
		GetObject(BitmapInfo, sizeof(BITMAP), (LPSTR)&bm);
		if ( Count == 0 ) {
			BitBlt(wDC, 0, 0, bm.bmWidth, bm.bmHeight,hMemDC, 0, 0, SRCCOPY);
		} else { 
			BitBlt(wDC, 10, 32, bm.bmWidth, bm.bmHeight,hMemDC, 0, 0, SRCCOPY);
		};
		
		
		SelectObject(hMemDC,hOldBm);
		DeleteDC(hMemDC);
		DeleteDC(wDC);
		EndPaint(hDlg,&wps);
		return(TRUE);
		
		
		
		return(FALSE);
		
    }
    return (FALSE);
	UNREFERENCED_PARAMETER(lParam);
}

// hier externe datei 1 by ELMS
#include "IniHand.c"

/*
void fill(SHORT *pDest, DWORD bytes)
{
	_asm {
		xor eax, eax; // fill value is zero
		mov ecx, bytes
			shr ecx, 1 // filling dwords
			mov edi, pDest		
			rep stosd
	}
}
*/

void Init_Screen_Struct()
{
	RECT  sRect;
	int OutX;
	
	GetClientRect(hWnd, &rectscreen);
	ClientToScreen(hWnd, (POINT*)&rectscreen.left);
	ClientToScreen(hWnd, (POINT*)&rectscreen.right);
			 
			 if (rectscreen.left < 0) rectscreen.left=0;
			 if (rectscreen.top < 0) rectscreen.top=0;
			 if (rectscreen.right > GetSystemMetrics(SM_CXSCREEN)) rectscreen.right=GetSystemMetrics(SM_CXSCREEN);
			 if (rectscreen.bottom > GetSystemMetrics(SM_CYSCREEN)) rectscreen.bottom=GetSystemMetrics(SM_CYSCREEN);
			 
			 if ( pDecode != NULL ) {
				 free(pDecode);
				 pDecode=NULL;
			 };                     
			 
             Destroy_Overlay();
			 
             if ((ModeRAWSurface == TRUE ) || (( OutDib == FALSE ) && ( ModeOverlay == FALSE ))) InvalidateRect(NULL,NULL,TRUE);
			 else if ( ModeOverlay == FALSE ) InvalidateRect(hWnd,NULL,TRUE); 
			 CurrentX=(rectscreen.right-rectscreen.left);
			 CurrentY=(rectscreen.bottom-rectscreen.top);
			 CurrentX=(CurrentX/2)*2;
			 
			 
			 
			 if ( Display_Status_Bar == TRUE ) CurrentY-=21;
			 
			 if ( CurrentY <= 10 ) CurrentY=10;
			 if ( CurrentX <= 10 ) CurrentX=10;
			 
			 MaxY=ShowY=CurrentY;
			 if ( MaxY > 1024 ) MaxY=1024;
			 MaxX=CurrentX;
			 
			 
             if (( DecodeNagra == TRUE ) || ( DecodeVCrypt == TRUE ) || ( DecodeDecoderVCrypt == TRUE )) {
				 CurrentY=TVSettings[TVTYPE].wCropHeight;
                 
             };
			 
			 BurstOffset=0;
			 
			 
			 if ( Current_Mode == 0 ) {
				 if ( CurrentX > TVSettings[TVTYPE].wCropWidth ) CurrentX=TVSettings[TVTYPE].wCropWidth;
				 if ( CurrentY > TVSettings[TVTYPE].wCropHeight) CurrentY=TVSettings[TVTYPE].wCropHeight;
			 };
			 if ( Current_Mode == 1 ) {
				 if ( ModeRAWSurface == FALSE ) {
					 if ( CurrentX > 800 ) CurrentX=800;
			              }
				 if ( CurrentY > 576 ) CurrentY=576;
			 };
			 
			 if (( ModeOverlay == TRUE ) && ( Can_Overlay_Strech == TRUE ) && (USE_DX_OV_STRECH== TRUE )) {
				 if ( CurrentX > 768 ) CurrentX=768;
			 };
			 
             
			 if ( ShowY > CurrentY ) ShowY=CurrentY;		
			 
             if (( DecodeVCrypt == TRUE ) || ( DecodeDecoderVCrypt == TRUE )) CurrentX=720;
			 
			 
			 if (( TVTYPE == 5 )  && (( DecodeNagra == TRUE ) || (DecodeDecoderVCrypt == TRUE ) || ( DecodeVCrypt == TRUE )))  {
				 BurstOffset=(int)((double)CurrentX/(double)TVSettings[TVTYPE].wCropWidth*(double)76);
				 BurstOffset=(BurstOffset/2)*2;
				 if ( (CurrentX + BurstOffset ) > TVSettings[TVTYPE].wCropWidth ) {
					 CurrentX=844;
					 BurstOffset=76;
				 };
				 
			 }
			 
			 
			 if (pDIB==NULL) return;
			 
			 pDIB->bmiHeader.biSize          = sizeof(pDIB->bmiHeader);
			 pDIB->bmiHeader.biWidth         = CurrentX;
			 pDIB->bmiHeader.biHeight        = MaxY;
			 pDIB->bmiHeader.biPlanes        = 1;
			 pDIB->bmiHeader.biBitCount      = 16;    // 16 
			 pDIB->bmiHeader.biCompression   = BI_RGB;
			 pDIB->bmiHeader.biSizeImage     = pDIB->bmiHeader.biWidth    *
				 pDIB->bmiHeader.biHeight   *
				 pDIB->bmiHeader.biBitCount / 8
				 +128;
			 pDIB->bmiHeader.biXPelsPerMeter = 0;
			 pDIB->bmiHeader.biYPelsPerMeter = 0;
			 pDIB->bmiHeader.biClrUsed       = 0;
			 pDIB->bmiHeader.biClrImportant  = 0;
			 
			 
			 DibPitch=(pDIB->bmiHeader.biBitCount/8*CurrentX);
			 
			 
			 m_dwDisplaySize=(pDIB->bmiHeader.biSizeImage);
			 
			 pDecode=malloc(m_dwDisplaySize+1024);
			 if ( pDecode == NULL ) {
				 MessageBox(hWnd,"Kein Decoding-Memory","Multi-Decode",MB_ICONSTOP|MB_OK);
				 SendMessage(hWnd,WM_DESTROY,0,0);
				 return;
             }
			 
			 
			 
			 Offset_for_WindowOut=(SurfacePitch * rectscreen.top) + (rectscreen.left*2);
			 if ( Offset_for_WindowOut < 0 ) {
				 Offset_for_WindowOut=0;
			 }
			 
			 if ((( ModeOverlay == TRUE ) || (ModeRAWOverlay == TRUE )) && ( Can_Overlay )) {
				 
				 if ( MaxX < CurrentX ) ModeOverlay=CreateOverlay(MaxX,MaxY); 
				 else ModeOverlay=CreateOverlay(CurrentX,MaxY); 
				 if (ModeOverlay == FALSE ) {
					 ModeRAWOverlay=FALSE;
				 };
				 
				 if (ModeOverlay == TRUE ) {
					 if (( Can_Overlay_Strech == TRUE ) && (USE_DX_OV_STRECH== TRUE )) {
						 GetClientRect(hWnd, &sRect);
						 ClientToScreen(hWnd, (POINT*)&sRect.left);
						 ClientToScreen(hWnd, (POINT*)&sRect.right);
						 if ( Display_Status_Bar == TRUE ) sRect.bottom-=21;
						 
						 if ( sRect.left < 0 ) sRect.left=0;
						 if ( sRect.top < 0 ) sRect.top=0;
						 if (sRect.right > GetSystemMetrics(SM_CXSCREEN)) sRect.right=GetSystemMetrics(SM_CXSCREEN);
						 if (sRect.bottom > GetSystemMetrics(SM_CYSCREEN)) sRect.bottom=GetSystemMetrics(SM_CYSCREEN);
						 
						 
						 if (( DecodeVCrypt == TRUE ) || ( DecodeDecoderVCrypt == TRUE ))  {
							 OutX=MaxX;
							 if ( OutX > 712 ) OutX=712;
							 if ( MaxY < (ZEILENZAHL-(MovieScale*2)) ) OverlayUpdate(OutX,MaxY,&sRect, DDOVER_SHOW,TRUE); 
							 else OverlayUpdate(OutX,(ZEILENZAHL-(MovieScale*2)),&sRect, DDOVER_SHOW,TRUE); 
						 } else if ( DecodeNagra == TRUE ) {
							 if ( NagraFullField == TRUE ) {
								 if ( MaxY < (ZEILENZAHL-(MovieScale*2))*2 ) OverlayUpdate(CurrentX,MaxY,&sRect, DDOVER_SHOW,TRUE); 
								 else OverlayUpdate(CurrentX,(ZEILENZAHL-(MovieScale*2))*2,&sRect, DDOVER_SHOW,TRUE);
							 } else {
								 if ( MaxY < (ZEILENZAHL-(MovieScale*2)) ) OverlayUpdate(CurrentX,MaxY,&sRect, DDOVER_SHOW,TRUE); 
								 else OverlayUpdate(CurrentX,(ZEILENZAHL-(MovieScale*2)),&sRect, DDOVER_SHOW,TRUE); 
							 }
						 } else {
							 if ( HalfTVMode == TRUE ) {
								 OverlayUpdate(CurrentX,(CurrentY/2)-(MovieScale*2),&sRect, DDOVER_SHOW,TRUE);
							 } else {
								 OverlayUpdate(CurrentX,CurrentY-(MovieScale*4),&sRect, DDOVER_SHOW,TRUE);
							 }
						 }
						 
					 } else {
						 GetClientRect(hWnd, &sRect);
						 ClientToScreen(hWnd, (POINT*)&sRect.left);
						 ClientToScreen(hWnd, (POINT*)&sRect.right);
						 if ( sRect.right - sRect.left > CurrentX ) sRect.right=sRect.left+CurrentX;
						 if ( Display_Status_Bar == TRUE ) sRect.bottom-=21;
						 OverlayUpdate(CurrentX,MaxY,&sRect, DDOVER_SHOW,TRUE);
						 
					 }
					 Black_Overlay();
				 };
			 }
			 
			 if (( Has_BT == TRUE ) && ( Current_Mode==0 )) SetGeoSize(CurrentX+BurstOffset,CurrentY);
			 if (( Has_DVB == TRUE ) && ( Current_Mode==1 )) set_up_grabbing();
			 return;
}

void Restore_Mode()
{
	if ( MenuSave == TRUE ) {
		MenuSave=FALSE;   	
		Stop_Thread();
		Set_Capture(4);
		Flip=MenuFlip;
		OutDib=MenuOutDib;
		ModeOverlay=MenuModeOverlay;
		ModeRAWSurface=MenuModeRAWSurface;
		ModeRAWOverlay=MenuModeRAWOverlay;
		Init_Screen_Struct();
		Set_Capture(5);
	};
	
}

void Save_Mode()
{ 
	if ( MenuSave == FALSE ) {
		if ( OutDib == TRUE ) return; 
		MenuSave=TRUE;   			  
		Stop_Thread();
		Set_Capture(4);
		MenuFlip=Flip;
		MenuOutDib=OutDib;
		MenuModeOverlay=ModeOverlay;
		MenuModeRAWSurface=ModeRAWSurface;
		MenuModeRAWOverlay=ModeRAWOverlay;
		Flip=1;
		OutDib=TRUE;
		ModeOverlay=FALSE;
		ModeRAWSurface=FALSE;
		ModeRAWOverlay=FALSE;
		Init_Screen_Struct();
		Set_Capture(5);
		
	}
}


void Clear_Status()
{
	AutoLockCount=0;
	UntilEven=0;
	UntilOdd=0;
	BlackEven=-1;
	BlackOdd=-1;
	FirstWidthEvenCount=0;
	FirstWidthOddCount=0;
	FirstWidthEven=TESTPIXEL;
	FirstWidthOdd=TESTPIXEL;
	Line286BlackEven=FALSE;
	Line286BlackOdd=FALSE;
	Line285BlackEven=FALSE;
	Line285BlackOdd=FALSE;
	Line0BlackEven=FALSE;
	Line0BlackOdd=FALSE;
	Line1BlackEven=FALSE;
	Line1BlackOdd=FALSE;
	
	MatrixModeEven=FALSE;
	MatrixModeOdd=FALSE;
	MatrixCountEven=0;
	MatrixCountOdd=0;
	MoreLinesLock=FALSE;
}



int G_Wert(int Wert1,int Wert2) 
{ if (Wert1>=Wert2) return(Wert1); 
else return(Wert2);
}


void Init_HCYUV2RGBTabelle(BOOL  RGB15)
{
	int nY;
	int nV;
	int nU;
	int nR;
	int nG;
	int nB;
	
	double Faktor;
	
	Faktor=(double)(255-HCSchwelle)/255;
	HCInit15=RGB15;
	//
	// Das Format der Tabelle YUV2RGB:
	//
	//   0x3F00    0x0FC0  0x003F
	//  17--V--12 11--U--6 5--Y--0
	//  MSB                    LSB
	//
	for(nY=0; nY<256; nY+=4)  
	{
		for(nU=0; nU<256; nU+=4)
		{
			for(nV=0; nV<256; nV+=4)
			{
				
				
				
				if ( HCMode1 == TRUE ) {
					if ( nY < HCSchwelle ) {
						nR=0;
						nG=0;
						nB=0;
					} else {
						nB = (116*(nY-16)                + 202*(nU-128)) / 100;
						nG = (116*(nY-16) -  81*(nV-128) -  39*(nU-128)) / 100;
						nR = (116*(nY-16) + 160*(nV-128)               ) / 100;
					}            
				} else if ( HCMode2 != TRUE ) {
					nB = (116*(nY-16)                + 202*(nU-128)) / 100;
					nG = (116*(nY-16) -  81*(nV-128) -  39*(nU-128)) / 100;
					nR = (116*(nY-16) + 160*(nV-128)               ) / 100;
				}
				
				
				if ( HCMode1 == TRUE ) {
					nR=nR+HCRotFaktor;
					nG=nG+HCGruenFaktor;
					nB=nB+HCBlauFaktor;
				}
				
				if ( HCMode2 == TRUE ) {
					if ( nY < HCCutFaktor ) {
						nR=HCCutLoRotFaktor;
						nG=HCCutLoGruenFaktor;
						nB=HCCutLoBlauFaktor;
					} else {
						nR=HCCutHiRotFaktor;
						nG=HCCutHiGruenFaktor;
						nB=HCCutHiBlauFaktor;
						
					}
					
				}
				
				
				if ( InvertRot) nR=255-nR ;
				if ( InvertGruen) nG=255-nG ;
				if ( InvertBlau) nB=255-nB ;
				
				if ( OffRot) nR=0;
				if ( OffGruen) nG=0;
				if ( OffBlau) nB=0;
				
				
				if(nR>0xff) nR=0xff;
				else if (nR<0) nR=0;
				if(nG>0xff) nG=0xff;
				else if (nG<0) nG=0;
				if(nB>0xff) nB=0xff;
				else if (nB<0) nB=0;
				
				
				if(RGB15)
					HCYUV2RGBTabelle[((nV>>2)<<12) | ((nU>>2)<<6) | nY>>2] = (USHORT)((nB>>3) | ((nG>>3)<<5) | ((nR>>3)<<10));
				else
					HCYUV2RGBTabelle[((nV>>2)<<12) | ((nU>>2)<<6) | nY>>2] = (USHORT)((nB>>3) | ((nG>>2)<<5) | ((nR>>3)<<11));
			}
		}
	}
}



void HCZeileAusgeben(DWORD *pQuelle,DWORD *pZiel, int Count)
{
	int    nPixel;
	DWORD  nUV;
	DWORD  nRGB1;
	DWORD  nRGB2;
	
	
	
	// v = v, u = u
	for(nPixel=0; nPixel<Count; nPixel+=2)
	{
		nUV = *pQuelle>>4 & 0xfc0 | *pQuelle>>14 & 0x3f000;
		
		
		nRGB1 = HCYUV2RGBTabelle[*pQuelle>>2 & 0x3f | nUV];
		nRGB2 = HCYUV2RGBTabelle[*pQuelle>>18 & 0x3f | nUV];
		
		*pZiel++ = nRGB1 | nRGB2<<16;
		pQuelle++;
	}
	
}

void HCZeileAusgeben_2(DWORD *pQuelle,DWORD  *pZiel,int Pitch, int Count)
{
	int    nPixel;
	DWORD  nUV;
	DWORD  nRGB1;
	DWORD  nRGB2;
	DWORD nRGB;
	int P=Pitch/4;
	
	// v = v, u = u
	for(nPixel=0; nPixel<Count; nPixel+=2)
	{
		nUV = *pQuelle>>4 & 0xfc0 | *pQuelle>>14 & 0x3f000;
		
		nRGB1 = HCYUV2RGBTabelle[*pQuelle>>2 & 0x3f | nUV];
		nRGB2 = HCYUV2RGBTabelle[*pQuelle>>18 & 0x3f | nUV];
		nRGB  = nRGB1 | nRGB2<<16;
		*(pZiel+P)=nRGB;
		*pZiel++=nRGB;
		pQuelle++;
	}
}

void HCZeileAusgeben_3(DWORD *pQuelle,DWORD *pZiel,int Pitch, int Count)
{
	int    nPixel;
	DWORD  nUV;
	DWORD  nRGB1;
	DWORD  nRGB2;
	DWORD nRGB; 
	int P=Pitch/4;
	int P1=Pitch/2;
	
	// v = v, u = u
	for(nPixel=0; nPixel<Count; nPixel+=2)
	{
		nUV = *pQuelle>>4 & 0xfc0 | *pQuelle>>14 & 0x3f000;
		
		nRGB1 = HCYUV2RGBTabelle[*pQuelle>>2 & 0x3f | nUV];
		nRGB2 = HCYUV2RGBTabelle[*pQuelle>>18 & 0x3f | nUV];
		nRGB  = nRGB1 | nRGB2<<16;
		*(pZiel+P)=nRGB;
		*(pZiel+P1)=nRGB;
		*pZiel++=nRGB;
		pQuelle++;
	}
	
}

void HCZeileAusgeben_4(DWORD *pQuelle,DWORD *pZiel,int Pitch, int Count)
{
	int    nPixel;
	DWORD  nUV;
	DWORD  nRGB1;
	DWORD  nRGB2;
	DWORD nRGB;
	int P=Pitch/4;
	int P1=Pitch/2;
	int P2=Pitch;
	
	
	
	// v = v, u = u
	for(nPixel=0; nPixel<Count; nPixel+=2)
	{
		nUV = *pQuelle>>4 & 0xfc0 | *pQuelle>>14 & 0x3f000;
		
		nRGB1 = HCYUV2RGBTabelle[*pQuelle>>2 & 0x3f | nUV];
		nRGB2 = HCYUV2RGBTabelle[*pQuelle>>18 & 0x3f | nUV];
		nRGB  = nRGB1 | nRGB2<<16;
		*(pZiel+P)=nRGB;
		*(pZiel+P1)=nRGB;
		*(pZiel+P2)=nRGB;
		*pZiel++=nRGB;
		pQuelle++;
	}
	
}


BOOL APIENTRY ColorSettingProc
(
 HWND hDlg,
 UINT message,
 UINT wParam,
 LONG lParam)
{
	int x,y,i;
	
	switch (message) {
	case WM_INITDIALOG:
		
		CheckDlgButton(hDlg, IDC_CHECK3, InvertRot);
		CheckDlgButton(hDlg, IDC_CHECK4, InvertGruen);
		CheckDlgButton(hDlg, IDC_CHECK5, InvertBlau);
		CheckDlgButton(hDlg, IDC_CHECK8, OffRot);
		CheckDlgButton(hDlg, IDC_CHECK9, OffGruen);
		CheckDlgButton(hDlg, IDC_CHECK10,OffBlau);
		CheckDlgButton(hDlg, IDC_CHECK7, HCMode1);
		CheckDlgButton(hDlg, IDC_CHECK6, HCMode2);
		
		
		y=(int)(269-((double)(HCSchwelle)/256*160));
		MoveWindow(GetDlgItem(hDlg,IDC_S1),40,y-4,22,8,FALSE);
		SetDlgItemInt(hDlg,IDC_D1,HCSchwelle,FALSE);
		
		y=(int)(189-((double)(HCRotFaktor)/512*160));
		MoveWindow(GetDlgItem(hDlg,IDC_S2),116,y-4,22,8,FALSE);
		SetDlgItemInt(hDlg,IDC_D2,HCRotFaktor,TRUE);
		
		y=(int)(189-((double)(HCGruenFaktor)/512*160));
		MoveWindow(GetDlgItem(hDlg,IDC_S3),191,y-4,22,8,FALSE);
		SetDlgItemInt(hDlg,IDC_D3,HCGruenFaktor,TRUE);
		
		y=(int)(189-((double)(HCBlauFaktor)/512*160));
		MoveWindow(GetDlgItem(hDlg,IDC_S4),264,y-4,22,8,FALSE);
		SetDlgItemInt(hDlg,IDC_D4,HCBlauFaktor,TRUE);
		
		y=(int)(269-((double)(HCCutFaktor)/256*160));
		MoveWindow(GetDlgItem(hDlg,IDC_S5),377,y-4,22,8,FALSE);
		SetDlgItemInt(hDlg,IDC_D5,HCCutFaktor,FALSE);
		
		x=(int)(503+((double)(HCCutHiRotFaktor)/256*160));
		MoveWindow(GetDlgItem(hDlg,IDC_S6),x-4,49,8,22,FALSE);
		SetDlgItemInt(hDlg,IDC_D6,HCCutHiRotFaktor,FALSE);
		
		x=(int)(503+((double)(HCCutHiGruenFaktor)/256*160));
		MoveWindow(GetDlgItem(hDlg,IDC_S7),x-4,94,8,22,FALSE);
		SetDlgItemInt(hDlg,IDC_D7,HCCutHiGruenFaktor,FALSE);
		
		x=(int)(503+((double)(HCCutHiBlauFaktor)/256*160));
		MoveWindow(GetDlgItem(hDlg,IDC_S8),x-4,141,8,22,FALSE);
		SetDlgItemInt(hDlg,IDC_D8,HCCutHiBlauFaktor,FALSE);
		
		x=(int)(503+((double)(HCCutLoRotFaktor)/256*160));
		MoveWindow(GetDlgItem(hDlg,IDC_S9),x-4,187,8,22,FALSE);
		SetDlgItemInt(hDlg,IDC_D9,HCCutLoRotFaktor,FALSE);
		
		x=(int)(503+((double)(HCCutLoGruenFaktor)/256*160));
		MoveWindow(GetDlgItem(hDlg,IDC_S10),x-4,231,8,22,FALSE);
		SetDlgItemInt(hDlg,IDC_D10,HCCutLoGruenFaktor,FALSE);
		
		x=(int)(503+((double)(HCCutLoBlauFaktor)/256*160));
		MoveWindow(GetDlgItem(hDlg,IDC_S11),x-4,278,8,22,FALSE);
		SetDlgItemInt(hDlg,IDC_D11,HCCutLoBlauFaktor,FALSE);
		
		break;
		
		
		
	case WM_MOUSEMOVE:
		
		y=HIWORD( lParam );
		x=LOWORD( lParam );
		if (wParam == MK_LBUTTON) {
			if (( y >= 109) && ( y <= 269 )) {
				if (( x >= 40) && ( x <= 62 )) {
					
					i=(int)((double)(y-269)/160*256);
					i=-i;
					if ( i < 0 ) i=0;
					else if ( i > 255 ) i=255;
					MoveWindow(GetDlgItem(hDlg,IDC_S1),40,y-2,22,8,TRUE);
					HCSchwelle=i;
					SetDlgItemInt(hDlg,IDC_D1,HCSchwelle,FALSE);
				}
				
				if (( x >= 116) && ( x <= 138 )) {
					i=(int)((double)(y-189)/160*512);
					if ( i < -255) i=-255;
					else if ( i > 255 ) i=255;
					i=-i;
					MoveWindow(GetDlgItem(hDlg,IDC_S2),116,y-2,22,8,TRUE);
					SetDlgItemInt(hDlg,IDC_D2,i,TRUE);
					HCRotFaktor=i;
				}
				
				
				if (( x >= 191) && ( x <= 213 )) {
					i=(int)((double)(y-189)/160*512);
					if ( i < -255) i=-255;
					else if ( i > 255 ) i=255;
					i=-i;
					MoveWindow(GetDlgItem(hDlg,IDC_S3),191,y-2,22,8,TRUE);
					SetDlgItemInt(hDlg,IDC_D3,i,TRUE);
					HCGruenFaktor=i;
				}
				
				if (( x >= 264) && ( x <= 286 )) {
					i=(int)((double)(y-189)/160*512);
					if ( i < -255) i=-255;
					else if ( i > 255 ) i=255;
					i=-i;
					MoveWindow(GetDlgItem(hDlg,IDC_S4),264,y-2,22,8,TRUE);
					SetDlgItemInt(hDlg,IDC_D4,i,TRUE);
					HCBlauFaktor=i;
				}
				
				if (( x >= 377) && ( x <= 399 )) {
					i=(int)((double)(y-269)/160*256);
					i=-i;
					if ( i < 0 ) i=0;
					else if ( i > 255 ) i=255;
					MoveWindow(GetDlgItem(hDlg,IDC_S5),377,y-2,22,8,TRUE);
					SetDlgItemInt(hDlg,IDC_D5,i,FALSE);
					HCCutFaktor=i;
				}
				Init_HCYUV2RGBTabelle(HCInit15);
			}
			
			if (( x >= 503 ) && ( x <= 663 )) {
				
				if (( y >=49 ) && ( y <=71)) {
					i=(int)((double)(x-503)/160*255);
					if ( i < 0) i=0;
					else if ( i > 255 ) i=255;
					SetDlgItemInt(hDlg,IDC_D6,i,TRUE);
					MoveWindow(GetDlgItem(hDlg,IDC_S6),x-4,49,8,22,TRUE);
					HCCutHiRotFaktor=i;
				}
				if (( y >=94 ) && ( y <=116)) {
					i=(int)((double)(x-503)/160*255);
					if ( i < 0) i=0;
					else if ( i > 255 ) i=255;
					SetDlgItemInt(hDlg,IDC_D7,i,TRUE);
					MoveWindow(GetDlgItem(hDlg,IDC_S7),x-4,94,8,22,TRUE);
					HCCutHiGruenFaktor=i;
				}
				if (( y >=141 ) && ( y <=163)) {
					i=(int)((double)(x-503)/160*255);
					if ( i < 0) i=0;
					else if ( i > 255 ) i=255;
					SetDlgItemInt(hDlg,IDC_D8,i,TRUE);
					MoveWindow(GetDlgItem(hDlg,IDC_S8),x-4,141,8,22,TRUE);
					HCCutHiBlauFaktor=i;
				}
				
				if (( y >=187 ) && ( y <=209)) {
					i=(int)((double)(x-503)/160*255);
					if ( i < 0) i=0;
					else if ( i > 255 ) i=255;
					SetDlgItemInt(hDlg,IDC_D9,i,TRUE);
					MoveWindow(GetDlgItem(hDlg,IDC_S9),x-4,187,8,22,TRUE);
					HCCutLoRotFaktor=i;
				}
				if (( y >=231 ) && ( y <=253)) {
					i=(int)((double)(x-503)/160*255);
					if ( i < 0) i=0;
					else if ( i > 255 ) i=255;
					SetDlgItemInt(hDlg,IDC_D10,i,TRUE);
					MoveWindow(GetDlgItem(hDlg,IDC_S10),x-4,231,8,22,TRUE);
					HCCutLoGruenFaktor=i;
				}
				if (( y >=278 ) && ( y <=300)) {
					i=(int)((double)(x-503)/160*255);
					if ( i < 0) i=0;
					else if ( i > 255 ) i=255;
					SetDlgItemInt(hDlg,IDC_D11,i,TRUE);
					MoveWindow(GetDlgItem(hDlg,IDC_S11),x-4,278,8,22,TRUE);
					HCCutLoBlauFaktor=i;
				}
				Init_HCYUV2RGBTabelle(HCInit15);
			}
			
			}
			return(FALSE);
        case WM_COMMAND:
			if (((HWND)lParam==GetDlgItem(hDlg,IDC_CHECK3)) ||
				((HWND)lParam==GetDlgItem(hDlg,IDC_CHECK4)) ||
				((HWND)lParam==GetDlgItem(hDlg,IDC_CHECK5)) ||
				((HWND)lParam==GetDlgItem(hDlg,IDC_CHECK6)) ||
				((HWND)lParam==GetDlgItem(hDlg,IDC_CHECK7)) ||
				((HWND)lParam==GetDlgItem(hDlg,IDC_CHECK8)) ||
				((HWND)lParam==GetDlgItem(hDlg,IDC_CHECK9)) ||
				((HWND)lParam==GetDlgItem(hDlg,IDC_CHECK10))) {
				InvertRot=IsDlgButtonChecked(hDlg, IDC_CHECK3);
				InvertGruen=IsDlgButtonChecked(hDlg, IDC_CHECK4);
				InvertBlau=IsDlgButtonChecked(hDlg, IDC_CHECK5);
				OffRot=IsDlgButtonChecked(hDlg, IDC_CHECK8);
				OffGruen=IsDlgButtonChecked(hDlg, IDC_CHECK9);
				OffBlau=IsDlgButtonChecked(hDlg, IDC_CHECK10);
				HCMode1=IsDlgButtonChecked(hDlg, IDC_CHECK7);
				HCMode2=IsDlgButtonChecked(hDlg, IDC_CHECK6);
				Init_HCYUV2RGBTabelle(HCInit15);
			}				 
         			
			if (LOWORD(wParam) == IDOK ) {
				
				EndDialog(hDlg,TRUE);
			}
			
			
			if ( LOWORD(wParam) == IDDEFAULT) {
				InvertRot=FALSE;
				InvertGruen=FALSE;
				InvertBlau=FALSE;
				OffRot=FALSE;
				OffGruen=FALSE;
				OffBlau=FALSE;
				HCMode1=FALSE;
				HCMode2=FALSE;
				HCSchwelle=0x00;
				HCRotFaktor=0x00;
				HCGruenFaktor=0x00;
				HCBlauFaktor=0x00;
				HCCutFaktor=0x00;
				HCCutHiRotFaktor=0x00;
				HCCutHiGruenFaktor=0x00;
				HCCutHiBlauFaktor=0x00;
				HCCutLoRotFaktor=0x00;
				HCCutLoGruenFaktor=0x00;
				HCCutLoBlauFaktor=0x00;
				Init_HCYUV2RGBTabelle(HCInit15);
				
				CheckDlgButton(hDlg, IDC_CHECK3, InvertRot);
				CheckDlgButton(hDlg, IDC_CHECK4, InvertGruen);
				CheckDlgButton(hDlg, IDC_CHECK5, InvertBlau);
				CheckDlgButton(hDlg, IDC_CHECK8, OffRot);
				CheckDlgButton(hDlg, IDC_CHECK9, OffGruen);
				CheckDlgButton(hDlg, IDC_CHECK10,OffBlau);
				CheckDlgButton(hDlg, IDC_CHECK6, HCMode1);
				CheckDlgButton(hDlg, IDC_CHECK7, HCMode2);
				
				
				y=(int)(269-((double)(HCSchwelle)/256*160));
				MoveWindow(GetDlgItem(hDlg,IDC_S1),40,y-4,22,8,TRUE);
				SetDlgItemInt(hDlg,IDC_D1,HCSchwelle,FALSE);
				
				y=(int)(189-((double)(HCRotFaktor)/512*160));
				MoveWindow(GetDlgItem(hDlg,IDC_S2),116,y-4,22,8,TRUE);
				SetDlgItemInt(hDlg,IDC_D2,HCRotFaktor,TRUE);
				
				y=(int)(189-((double)(HCGruenFaktor)/512*160));
				MoveWindow(GetDlgItem(hDlg,IDC_S3),191,y-4,22,8,TRUE);
				SetDlgItemInt(hDlg,IDC_D3,HCGruenFaktor,TRUE);
				
				y=(int)(189-((double)(HCBlauFaktor)/512*160));
				MoveWindow(GetDlgItem(hDlg,IDC_S4),264,y-4,22,8,TRUE);
				SetDlgItemInt(hDlg,IDC_D4,HCBlauFaktor,TRUE);
				
				y=(int)(269-((double)(HCCutFaktor)/256*160));
				MoveWindow(GetDlgItem(hDlg,IDC_S5),377,y-4,22,8,TRUE);
				SetDlgItemInt(hDlg,IDC_D5,HCCutFaktor,FALSE);
				
				x=(int)(503+((double)(HCCutHiRotFaktor)/256*160));
				MoveWindow(GetDlgItem(hDlg,IDC_S6),x-4,49,8,22,TRUE);
				SetDlgItemInt(hDlg,IDC_D6,HCCutHiRotFaktor,FALSE);
				
				x=(int)(503+((double)(HCCutHiGruenFaktor)/256*160));
				MoveWindow(GetDlgItem(hDlg,IDC_S7),x-4,94,8,22,TRUE);
				SetDlgItemInt(hDlg,IDC_D7,HCCutHiGruenFaktor,FALSE);
				
				x=(int)(503+((double)(HCCutHiBlauFaktor)/256*160));
				MoveWindow(GetDlgItem(hDlg,IDC_S8),x-4,141,8,22,TRUE);
				SetDlgItemInt(hDlg,IDC_D8,HCCutHiBlauFaktor,FALSE);
				
				x=(int)(503+((double)(HCCutLoRotFaktor)/256*160));
				MoveWindow(GetDlgItem(hDlg,IDC_S9),x-4,187,8,22,TRUE);
				SetDlgItemInt(hDlg,IDC_D9,HCCutLoRotFaktor,FALSE);
				
				x=(int)(503+((double)(HCCutLoGruenFaktor)/256*160));
				MoveWindow(GetDlgItem(hDlg,IDC_S10),x-4,231,8,22,TRUE);
				SetDlgItemInt(hDlg,IDC_D10,HCCutLoGruenFaktor,FALSE);
				
				x=(int)(503+((double)(HCCutLoBlauFaktor)/256*160));
				MoveWindow(GetDlgItem(hDlg,IDC_S11),x-4,278,8,22,TRUE);
				SetDlgItemInt(hDlg,IDC_D11,HCCutLoBlauFaktor,FALSE);
				
				
				
			}
			
			break;
    }
	
	return(FALSE);
}



BOOL APIENTRY FilterListe(
						  HWND hDlg,
						  UINT message,
						  UINT wParam,
						  LONG lParam)
{
	int i;
	char zeile[256];
    char SelectZeile[256];
	LOGFONT Mfont = {9,0,0,0,0,0,0,0,0,0,0,0,0,"Fixedsys"};   
    HFONT currFont;
	
	
    switch (message) {
	case WM_INITDIALOG:
		currFont = CreateFontIndirect(&Mfont);
		SendMessage(GetDlgItem(hDlg,IDC_LIST1),WM_SETFONT,(WPARAM)currFont,TRUE);
		
		SendMessage(GetDlgItem(hDlg,IDC_LIST1),LB_RESETCONTENT,0,0);
		i=0;
		while ( i < MAXFILTER ) {
			if ( Filter[i].pid != 0xffff ) {
				sprintf(zeile,"%02d %s",i,Filter[i].Name);
				SendMessage(GetDlgItem(hDlg,IDC_LIST1),LB_ADDSTRING,0,(LPARAM)zeile);
			};
			i++;
		};
		return(TRUE);
		
		
	case WM_COMMAND:
		
		if (LOWORD(wParam) == IDDELFILTER ) {
			i=SendMessage (GetDlgItem(hDlg,IDC_LIST1), LB_GETCURSEL, 0, 0);
			if ( i == LB_ERR ) break;
			i=SendMessage(GetDlgItem(hDlg,IDC_LIST1),LB_GETTEXT,i,(LPARAM)SelectZeile);
			i=0;
			while ( i < MAXFILTER ) {
                if ( Filter[i].pid != 0xffff ) {
                    sprintf(zeile,"%02d %s",i,Filter[i].Name);
                    if ( strcmp(zeile,SelectZeile) == 0 ) {
                        ShutDownFiltersByName(Filter[i].Name);
						i=MAXFILTER;
					};
				};
				i++;
			};
			
			
            SendMessage(GetDlgItem(hDlg,IDC_LIST1),LB_RESETCONTENT,0,0);
			i=0;
			while ( i < MAXFILTER ) {
                if ( Filter[i].pid != 0xffff ) {
                    sprintf(zeile,"%02d %s",i,Filter[i].Name);
					SendMessage(GetDlgItem(hDlg,IDC_LIST1),LB_ADDSTRING,0,(LPARAM)zeile);
				};
				i++;
			};
		} 
		
		if (LOWORD(wParam) == IDOK
			|| LOWORD(wParam) == IDCANCEL) {
			EndDialog(hDlg,TRUE);
		}
		
		break;
    }
    return (FALSE);
	UNREFERENCED_PARAMETER(lParam);
}





BOOL APIENTRY CryptModeProc
(
 HWND hDlg,
 UINT message,
 UINT wParam,
 LONG lParam)
{
	char Text[120];
	
	switch (message) {
	case WM_INITDIALOG:
		SetTimer(hDlg,0,20,NULL);
	case WM_TIMER:
		if ( CryptInfo.LastCryptMode == 0 ) {
			SetDlgItemText(hDlg,TEXT0,"Free-TV");
			SetDlgItemText(hDlg,TEXT1,"");
			SetDlgItemText(hDlg,TEXT2,"");
			SetDlgItemText(hDlg,TEXT3,"");
			SetDlgItemText(hDlg,TEXT4,"");
		} else if ( CryptInfo.LastCryptMode == 1 ) {
			SetDlgItemText(hDlg,TEXT0,"Syster/NagraVision");
			SetDlgItemText(hDlg,TEXT1,"");
			SetDlgItemText(hDlg,TEXT2,"");
			SetDlgItemText(hDlg,TEXT3,"");
			SetDlgItemText(hDlg,TEXT4,"");
		} 
		//			else {
		if ( CryptInfo.LastCryptMode==2 ) SetDlgItemText(hDlg,TEXT0,"VideoCrypt I Dekodermodus");
		if ( CryptInfo.LastCryptMode==3 ) SetDlgItemText(hDlg,TEXT0,"VideoCrypt I");
		if ( CryptInfo.OSD0Set == 1 ) {
			CryptInfo.OSD0Set = 0;
			SetDlgItemText(hDlg,W1,"X");
			sprintf(Text,"OSD0   %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x",CryptInfo.OSD0[0],CryptInfo.OSD0[1],CryptInfo.OSD0[2],CryptInfo.OSD0[3],CryptInfo.OSD0[4],CryptInfo.OSD0[5],CryptInfo.OSD0[6],CryptInfo.OSD0[7],CryptInfo.OSD0[8],CryptInfo.OSD0[9]);
			SetDlgItemText(hDlg,TEXT1,Text);
		} else SetDlgItemText(hDlg,W1,"");
		if ( CryptInfo.OSD1Set == 1 ) {
			SetDlgItemText(hDlg,W2,"X");
			CryptInfo.OSD1Set = 0;
			sprintf(Text,"OSD1   %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x",CryptInfo.OSD1[0],CryptInfo.OSD1[1],CryptInfo.OSD1[2],CryptInfo.OSD1[3],CryptInfo.OSD1[4],CryptInfo.OSD1[5],CryptInfo.OSD1[6],CryptInfo.OSD1[7],CryptInfo.OSD1[8],CryptInfo.OSD1[9]);
			SetDlgItemText(hDlg,TEXT2,Text);
		} else SetDlgItemText(hDlg,W2,"");
		if ( CryptInfo.MSG0Set == 1 ) {
			SetDlgItemText(hDlg,W3,"X");
			CryptInfo.MSG0Set = 0;
			sprintf(Text,"MSG0   %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x",CryptInfo.MSG0[0],CryptInfo.MSG0[1],CryptInfo.MSG0[2],CryptInfo.MSG0[3],CryptInfo.MSG0[4],CryptInfo.MSG0[5],CryptInfo.MSG0[6],CryptInfo.MSG0[7],CryptInfo.MSG0[8],CryptInfo.MSG0[9]);
			SetDlgItemText(hDlg,TEXT3,Text);
		} else SetDlgItemText(hDlg,W3,"");
		if ( CryptInfo.MSG1Set == 1 ) {
			CryptInfo.MSG1Set = 0;
			SetDlgItemText(hDlg,W4,"X");
			sprintf(Text,"MSG1   %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x",CryptInfo.MSG1[0],CryptInfo.MSG1[1],CryptInfo.MSG1[2],CryptInfo.MSG1[3],CryptInfo.MSG1[4],CryptInfo.MSG1[5],CryptInfo.MSG1[6],CryptInfo.MSG1[7],CryptInfo.MSG1[8],CryptInfo.MSG1[9]);
			SetDlgItemText(hDlg,TEXT4,Text);
		} else SetDlgItemText(hDlg,W4,"");
		//			}
		break;
	case WM_COMMAND:
		if ((LOWORD(wParam) == IDOK ) || (LOWORD(wParam) == IDCANCEL )) {
			KillTimer(hDlg,0);
			EndDialog(hDlg,TRUE);
		}
		break;
    }
	
	return(FALSE);
}


BOOL APIENTRY AboutProc
(
 HWND hDlg,
 UINT message,
 UINT wParam,
 LONG lParam)
{
	
	switch (message) {
	case WM_INITDIALOG:
		break;
	case WM_COMMAND:
		if (LOWORD(wParam) == IDC_BUTTON1 ) {
			DialogBox(hInst,"ABOUT1",hWnd,AboutProc1);
		}
		
		if ((LOWORD(wParam) == IDOK ) || (LOWORD(wParam) == IDCANCEL )) {
			EndDialog(hDlg,TRUE);
		}
		break;
    }
	
	return(FALSE);
}

BOOL APIENTRY AboutProc1
(
 HWND hDlg,
 UINT message,
 UINT wParam,
 LONG lParam)
{
	
	switch (message) {
	case WM_INITDIALOG:
		break;
	case WM_COMMAND:
		
		if ((LOWORD(wParam) == IDOK ) || (LOWORD(wParam) == IDCANCEL )) {
			EndDialog(hDlg,TRUE);
		}
		break;
    }
	
	return(FALSE);
}



BOOL APIENTRY NagraKeySetProc
(
 HWND hDlg,
 UINT message,
 UINT wParam,
 LONG lParam)
{
	
	switch (message) {
	case WM_INITDIALOG:
		SetDlgItemText(hDlg,EDIT1,OPKEY[0].Name);
		SetDlgItemText(hDlg,EDIT2,OPKEY[0].Extension);
		SetDlgItemText(hDlg,EDIT3,OPKEY[1].Name);
		SetDlgItemText(hDlg,EDIT4,OPKEY[1].Extension);
		SetDlgItemText(hDlg,EDIT5,OPKEY[2].Name);
		SetDlgItemText(hDlg,EDIT6,OPKEY[2].Extension);
		SetDlgItemText(hDlg,EDIT7,OPKEY[3].Name);
		SetDlgItemText(hDlg,EDIT8,OPKEY[3].Extension);
		if ( ActiveKey == 0 ) CheckDlgButton(hDlg,IDC_RADIO1,TRUE);
		else if ( ActiveKey == 1 ) CheckDlgButton(hDlg,IDC_RADIO2,TRUE);
		else if ( ActiveKey == 2 ) CheckDlgButton(hDlg,IDC_RADIO3,TRUE);
		else if ( ActiveKey == 3 ) CheckDlgButton(hDlg,IDC_RADIO4,TRUE);
		break;
	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK ) {
            GetDlgItemText(hDlg,EDIT1,OPKEY[0].Name,20);
            GetDlgItemText(hDlg,EDIT2,OPKEY[0].Extension,4);
            GetDlgItemText(hDlg,EDIT3,OPKEY[1].Name,20);
            GetDlgItemText(hDlg,EDIT4,OPKEY[1].Extension,4);
            GetDlgItemText(hDlg,EDIT5,OPKEY[2].Name,20);
            GetDlgItemText(hDlg,EDIT6,OPKEY[2].Extension,4);
            GetDlgItemText(hDlg,EDIT7,OPKEY[3].Name,20);
            GetDlgItemText(hDlg,EDIT8,OPKEY[3].Extension,4);
			OPKEY[0].Extension[3]=0x00;
			OPKEY[1].Extension[3]=0x00;
			OPKEY[2].Extension[3]=0x00;
			OPKEY[3].Extension[3]=0x00;
            if ( IsDlgButtonChecked(hDlg, IDC_RADIO1)) ActiveKey=0;
			else if ( IsDlgButtonChecked(hDlg, IDC_RADIO2)) ActiveKey=1;
			else if ( IsDlgButtonChecked(hDlg, IDC_RADIO3)) ActiveKey=2;
			else if ( IsDlgButtonChecked(hDlg, IDC_RADIO4)) ActiveKey=3;
			if ( !Init_Nagra_Decoding(MasterTestzeilen)) {
				SendMessage(hWnd,WM_COMMAND,IDM_DECODING_OFF,0);
				break;
			};
			EndDialog(hDlg,TRUE);
		};
		
		if (LOWORD(wParam) == IDCANCEL ) {
			EndDialog(hDlg,TRUE);
		}
		break;
    }
	
	return(FALSE);
}



BOOL APIENTRY DetectSettingProc
(
 HWND hDlg,
 UINT message,
 UINT wParam,
 LONG lParam)
{
	
	switch (message) {
	case WM_INITDIALOG:
		CheckDlgButton(hDlg, IDC_CHECK1, AutoDetect_Nagra);
		CheckDlgButton(hDlg, IDC_CHECK2, AutoDetect_VC);
		break;
		
		
	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK ) {
			AutoDetect_Nagra=IsDlgButtonChecked(hDlg, IDC_CHECK1);
			AutoDetect_VC=IsDlgButtonChecked(hDlg, IDC_CHECK2);
            EndDialog(hDlg,TRUE);
			break;  
		}
		
		if (LOWORD(wParam) == IDCANCEL ) {
			EndDialog(hDlg,TRUE);
			break;  
		}
	}
	
	return(FALSE);
}



BOOL APIENTRY ToggleSettingProc
(
 HWND hDlg,
 UINT message,
 UINT wParam,
 LONG lParam)
{
	
	switch (message) {
	case WM_INITDIALOG:
		if ( Toggle_WithOut_Frame == TRUE ) CheckDlgButton(hDlg, IDC_RADIO1, TRUE);
		else CheckDlgButton(hDlg, IDC_RADIO2, TRUE);
		break;
		
		
	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK ) {
			Toggle_WithOut_Frame=IsDlgButtonChecked(hDlg, IDC_RADIO1);
            EndDialog(hDlg,TRUE);
			break;  
		}
		
		if (LOWORD(wParam) == IDCANCEL ) {
			EndDialog(hDlg,TRUE);
			break;  
		}
	}
	
	return(FALSE);
}





BOOL APIENTRY VPSInfoProc
(
 HWND hDlg,
 UINT message,
 UINT wParam,
 LONG lParam)
{
	
	switch (message) {
	case WM_INITDIALOG:
		ShowVPSInfo=hDlg;
		SetTimer(hDlg,100,1000,NULL);
		break;
	case WM_TIMER:
		SetDlgItemInt(hDlg,IDT_VBI_FPS,VBI_FPS,FALSE);
		break;
		
	case WM_COMMAND:
		if ((LOWORD(wParam) == IDOK ) || (LOWORD(wParam) == IDCANCEL )) {
			ShowVPSInfo=NULL;
			KillTimer(hDlg,100); 
			EndDialog(hDlg,TRUE);
		}
		break;
    }
	
	return(FALSE);
}


BOOL APIENTRY WinBisSettingProc
(
 HWND hDlg,
 UINT message,
 UINT wParam,
 LONG lParam)
{
	
	switch (message) {
	case WM_INITDIALOG:
		SetDlgItemText(hDlg,IDC_EDIT1,WINIBIS_DIR);
		break;
		
		
	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK ) {
			GetDlgItemText(hDlg,IDC_EDIT1,WINIBIS_DIR,sizeof(WINIBIS_DIR));
			i=(int)CreateDirectory(WINIBIS_DIR,NULL);
			EndDialog(hDlg,TRUE);
		}
		
		if (LOWORD(wParam) == IDCANCEL ) {
			EndDialog(hDlg,TRUE);
		}
		break;
    }
	
	return(FALSE);
}



BOOL APIENTRY VDSettingProc
(
 HWND hDlg,
 UINT message,
 UINT wParam,
 LONG lParam)
{
	char FileName[255];
	unsigned char Buffer[10];
	FILE *sFile;
	
	switch (message) {
	case WM_INITDIALOG:
		SetDlgItemText(hDlg,IDC_EDIT1,VD_DIR);
		SetDlgItemText(hDlg,IDC_EDIT2,VDat.RawName);
		
		CheckDlgButton(hDlg, IDC_CHECK1, VD_RAW);
		
		if (VBI_Flags&VBI_VD) EnableWindow(GetDlgItem(hDlg,IDC_BUTTON1),FALSE); // Videodat läuft also umwandeln nicht möglich !!
		sprintf(FileName,"%s//%s",VD_DIR,VDat.RawName);
		sFile = fopen(FileName, "rb");
		if ( sFile == NULL ) EnableWindow(GetDlgItem(hDlg,IDC_BUTTON1),FALSE); // Keine RAW-Datei da !!
		else fclose(sFile);
		if ( VD_RAW == FALSE ) {
			EnableWindow(GetDlgItem(hDlg,IDC_EDIT2),FALSE);
			EnableWindow(GetDlgItem(hDlg,IDC_BUTTON1),FALSE);
		}
		break;
		
		
	case WM_COMMAND:
		
		if ( (HWND)lParam==GetDlgItem(hDlg,IDC_CHECK1) ){
			VD_RAW=IsDlgButtonChecked(hDlg, IDC_CHECK1);
 		        	if ( VD_RAW == FALSE ) {
						EnableWindow(GetDlgItem(hDlg,IDC_EDIT2),FALSE);
						EnableWindow(GetDlgItem(hDlg,IDC_BUTTON1),FALSE);
					} else if ( VD_RAW == TRUE ) {
						EnableWindow(GetDlgItem(hDlg,IDC_EDIT2),TRUE);
						EnableWindow(GetDlgItem(hDlg,IDC_BUTTON1),TRUE);
					}
		};
		
		
		if (LOWORD(wParam) == IDC_BUTTON1 ) {
			sprintf(FileName,"%s//%s",VD_DIR,VDat.RawName);
			sFile = fopen(FileName, "rb");
			if ( sFile == NULL ) {
				EnableWindow(GetDlgItem(hDlg,IDC_BUTTON1),FALSE); // Keine RAW-Datei da !!
				return(TRUE);
			};
			VideoDat_Init();
			while ( fread(Buffer, sizeof(unsigned char), 10, sFile) == (size_t)10 )  {
				Work_VideoDat(Buffer);
			};                  
			fclose(sFile);
			return(TRUE);
		}
		
		if (LOWORD(wParam) == IDOK ) {
			VD_RAW=IsDlgButtonChecked(hDlg, IDC_CHECK1);
			GetDlgItemText(hDlg,IDC_EDIT1,VD_DIR,sizeof(VD_DIR));
			i=(int)CreateDirectory(VD_DIR,NULL);
			GetDlgItemText(hDlg,IDC_EDIT2,VDat.RawName,sizeof(VDat.RawName));
			EndDialog(hDlg,TRUE);
		}
		
		if (LOWORD(wParam) == IDCANCEL ) {
			EndDialog(hDlg,TRUE);
		}
		break;
    }
	
	return(FALSE);
}




BOOL APIENTRY WINBISInfoProc
(
 HWND hDlg,
 UINT message,
 UINT wParam,
 LONG lParam)
{
	int i;
	char Zeile[20]; 
	switch (message) {
	case WM_INITDIALOG:
		SetDlgItemText(hDlg,TEXT1,WINIBIS_DIR);
		SetDlgItemText(hDlg,TEXT2,"Winbis.html");
		SetTimer(hDlg,0,500,NULL);
		
	case WM_TIMER:
		SetDlgItemInt(hDlg,IDT_VBI_FPS,VBI_FPS,FALSE);
		SetDlgItemInt(hDlg,TEXT3,fifo_in,FALSE);
		SetDlgItemInt(hDlg,TEXT4,fifo_out,FALSE);
		Zeile[0]=0x00;
		i=fifo_in-1;
		if ( i >= 0 ) sprintf(Zeile,"%02x",fifo[i][0]);
		SetDlgItemText(hDlg,TEXT5,Zeile);
		if ( i >= 0 ) sprintf(Zeile,"%02x",fifo[i][1]);
		SetDlgItemText(hDlg,TEXT6,Zeile);
        break;
		
	case WM_COMMAND:
		if ((LOWORD(wParam) == IDOK ) || (LOWORD(wParam) == IDCANCEL )) { 
			KillTimer(hDlg,0);
			EndDialog(hDlg,TRUE);
		}
		break;
    }
	
	return(FALSE);
}



BOOL APIENTRY VDInfoProc
(
 HWND hDlg,
 UINT message,
 UINT wParam,
 LONG lParam)
{
	
	switch (message) {
	case WM_INITDIALOG:
		SetTimer(hDlg,0,500,NULL);
	case WM_TIMER:
		SetDlgItemText(hDlg,TEXT3,SOTInfoRec.GeneralName);
		SetDlgItemText(hDlg,TEXT11,SOTInfoRec.Kommentar);
		SetDlgItemInt(hDlg,TEXT13, SOTInfoRec.date,FALSE);
		SetDlgItemInt(hDlg,TEXT9,SOTInfoRec.fanz,FALSE);
		SetDlgItemText(hDlg,TEXT1,VDat.FileName);
		SetDlgItemInt(hDlg,TEXT2, VDat.FileSize,FALSE);
		SetDlgItemInt(hDlg,TEXT8,VDat.BlockSoFar,FALSE);
		SetDlgItemInt(hDlg,TEXT6,VDat.CRCError,FALSE);
		SetDlgItemInt(hDlg,TEXT4,VDat.LBN,FALSE);
		SetDlgItemInt(hDlg,TEXT5,VDat.Lenght,FALSE);
		SetDlgItemInt(hDlg,TEXT7,VDat.KEY,FALSE);
		SetDlgItemText(hDlg,TEXT10,VDat.Error);
		SetDlgItemInt(hDlg,IDT_VBI_FPS,VBI_FPS,FALSE);
		
        break;
		
	case WM_COMMAND:
		if ((LOWORD(wParam) == IDOK ) || (LOWORD(wParam) == IDCANCEL )) { 
			KillTimer(hDlg,0);
			EndDialog(hDlg,TRUE);
		}
		break;
    }
	
	return(FALSE);
}



BOOL APIENTRY VDInfoProcRaw
(
 HWND hDlg,
 UINT message,
 UINT wParam,
 LONG lParam)
{
	
	switch (message) {
	case WM_INITDIALOG:
		SetTimer(hDlg,0,500,NULL);
	case WM_TIMER:
		SetDlgItemText(hDlg,TEXT1,VD_DIR);
		SetDlgItemText(hDlg,TEXT2,VDat.RawName);
		SetDlgItemInt(hDlg,TEXT3,VDat.BlocksOK,FALSE);
		SetDlgItemInt(hDlg,TEXT5,VDat.BlocksError,FALSE);
		SetDlgItemText(hDlg,TEXT10,VDat.Error);
		SetDlgItemInt(hDlg,IDT_VBI_FPS,VBI_FPS,FALSE);
        break;
		
	case WM_COMMAND:
		if ((LOWORD(wParam) == IDOK ) || (LOWORD(wParam) == IDCANCEL )) { 
			KillTimer(hDlg,0);
			EndDialog(hDlg,TRUE);
		}
		break;
    }
	
	return(FALSE);
}




BOOL APIENTRY PDCProc
(
 HWND hDlg,
 UINT message,
 UINT wParam,
 LONG lParam)
{
	switch (message) {
	case WM_INITDIALOG:
		ShowPDCInfo=hDlg;
		SetTimer(hDlg,100,1000,NULL);
		break;
		
	case WM_TIMER:
		SetDlgItemInt(hDlg,IDT_VBI_FPS,VBI_FPS,FALSE);
		break;
		
	case WM_COMMAND:
		if ((LOWORD(wParam) == IDOK ) || (LOWORD(wParam) == IDCANCEL )) {
			KillTimer(hDlg,100); 
			ShowPDCInfo=NULL;
			EndDialog(hDlg,TRUE);
		}
		break;
    }
	
	return(FALSE);
}

BOOL APIENTRY VideoTextUnterTitelProc
(
 HWND hDlg,
 UINT message,
 UINT wParam,
 LONG lParam)
{
	LPDRAWITEMSTRUCT    lpdis;
	LPMEASUREITEMSTRUCT lpmis;
	RECT       ptrtoWndPos;
	int currX,currY;
	int row,n;
	int i;
	unsigned short CurrentFg, CurrentBkg, RealBkg;
	BOOL bGraph,bHoldGraph,bSepGraph,bBox,bFlash,bDouble,bConceal,bHasDouble;
	BYTE nLastGraph;
	BYTE c,ch;
	unsigned short Black,ForceBlack,ForceTransparent;
	unsigned char buffer[40];
	
	LOGBRUSH BackGround;
	HBRUSH hBrush;
	
	char VTUBuffer[400];
	unsigned short VTUBufferFg[400];
	char ZielChar;
	int BufferFill;
	int InsertPos;
	BYTE PageLang;
	
	
	switch (message) {
	case WM_INITDIALOG:
		SetDlgItemInt(hDlg,IDC_EDIT1,UTPage+100,TRUE);
		currFont = CreateFontIndirect(&lf);
		UTList=CreateWindow("LISTBOX",NULL,
			WS_BORDER | 
			WS_CHILD | 
			WS_VISIBLE |
			// LBS_NOINTEGRALHEIGHT | 
			LBS_NOTIFY |
			// LBS_HASSTRINGS | 
			// LBS_WANTKEYBOARDINPUT |
			LBS_OWNERDRAWVARIABLE |
			WS_VSCROLL ,
			//	  WS_HSCROLL |
			//	  LBS_SORT , 
			//       LBS_DISABLENOSCROLL ,
								    0, 0, 0, 0,
                                    hDlg,
                                    NULL,
                                    hInst,
                                    NULL);
		
		
		SendMessage(UTList,
			WM_SETFONT,
			(WPARAM)currFont,
			TRUE);
		
		
		UTDialog=hDlg;
		
		
		
	case WM_SIZE:
		GetWindowRect(hDlg,
			&ptrtoWndPos);
					   currY=ptrtoWndPos.bottom-ptrtoWndPos.top;
					   currX=ptrtoWndPos.right-ptrtoWndPos.left;
					   currY=currY-90;
					   currX=currX-20;
					   
					   MoveWindow(UTList,
						   6,
						   35,
						   currX,
						   currY,
						   TRUE);
					   
					   MoveWindow(GetDlgItem(hDlg,IDOK),
						   currX/2-10,
						   40+currY,
						   40,
						   20,
						   TRUE);
					   
					   break;
					   
	case WM_CTLCOLORLISTBOX:
		BackGround.lbStyle = BS_SOLID;
		BackGround.lbColor = RGB(0,0,0);
		BackGround.lbHatch = 0;          
		hBrush=CreateBrushIndirect(&BackGround);          
		return((int)hBrush);
		
		
	case WM_DRAWITEM:
		/* Get pointer to the DRAWITEMSTRUCT */
		lpdis = (LPDRAWITEMSTRUCT)lParam;
		
		if (lpdis->itemID == -1)
		{
		/* We have a request to draw an item in the list box, yet there
		* are no list box items. This is sent when the user TABS into
		* an empty list box or an empty list box gets the focus. We
		* have to indicate (somehow) that this owner-draw list box has
		* the focus. We do it in response to this message. Note that
		* lpdis->itemData field would be invalid in this instance so
		* we can't allow it to fall into our standard routines.
			*/
			HandleFocusState(UTList,lpdis);
		}
		else
		{
			switch (lpdis->itemAction)
			{
			case ODA_DRAWENTIRE:
				DrawEntireItem(UTList,lpdis,0);
				break;
				
			case ODA_SELECT:
				HandleFocusState(UTList,lpdis);
				break;
				
			case ODA_FOCUS:
				HandleFocusState(UTList,lpdis);
				break;
			}
		}
		
		/* Return TRUE meaning that we processed this message. */
		return(TRUE);
		break;
		
	case WM_MEASUREITEM:
		lpmis = (LPMEASUREITEMSTRUCT)lParam;
		
		/* All the items are the same height since the list box style is
		* LBS_OWNERDRAWFIXED
		*/
		lpmis->itemHeight = 14;
		return(TRUE);
		
		
	case WM_USER:
		if ( wParam == 64 ) {
			if ( VTFrame[UTPage].SubCount==0 ) return(TRUE);
			PageLang= VTFrame[UTPage].SubPage[0].Lang;
			BufferFill=0;
			InsertPos=0;
			for (row=20; row<25; row++) {
				
				bGraph=bHoldGraph=bSepGraph=bBox=bFlash=bDouble=bConceal=bHasDouble=FALSE;
				nLastGraph=' ';
				Black=0; //
				ForceTransparent=8;
				ForceBlack=0;
				
				for (n=0; n<40; n++) {
					buffer[n]=VTFrame[UTPage].SubPage[0].Frame[row][n];
				}
				
				CurrentFg=7;
				for (n=0; n<40; n++) {
					c=buffer[n];
					ch=c;
					if(c<' ') {
						if(c<=0x07) {
							CurrentFg=c;
							bGraph=FALSE;
						} else if (c==0x08) {
							bFlash=TRUE;
						} else if (c==0x09) {
							bFlash=FALSE;
						} else if (c==0x0c) {
							bDouble=FALSE;
						} else if (c==0x0d) {
							bDouble=TRUE;
						} else if (c>=0x10 && c<=0x17) {
							bGraph=TRUE;
							CurrentFg=c-0x10;
						} else if (c==0x18) {
							bConceal=TRUE;
						} else if (c==0x19) {
							bSepGraph=FALSE;
						} else if (c==0x1a) {
							bSepGraph=TRUE;
						} else if (c==0x1c) {
							CurrentBkg=bBox?ForceBlack:Black;
							RealBkg=ForceBlack;
						} else if (c==0x1d) {
							CurrentBkg=(bBox || Black!=ForceTransparent) ?
CurrentFg : ForceTransparent;
							RealBkg=CurrentFg;
						} else if (c==0x1e) {
							bHoldGraph=TRUE;
						} else if (c==0x1f) {
							bHoldGraph=FALSE;
						}
						ch=bHoldGraph?nLastGraph:' ';
					}
					nLastGraph=' ';
					if(bGraph && (ch&0x20))
					{
						nLastGraph=ch;
						ch=(ch&0x1f)|((ch&0x40)>>1);
						ch+=96;
						if(bSepGraph) ch+=64;
					}
					else ch-=32;
					
					ZielChar=' ';
					if ( ch < 96 ) {
						ZielChar=VTtoAscii[PageLang][ch];
						if (( BufferFill == 0 ) && ( ZielChar != ' ' )) {
							VTUBuffer[BufferFill]=ZielChar;
							VTUBufferFg[BufferFill]=CurrentFg;
							BufferFill++;
						} else if ( BufferFill > 0 ) {
							if (( ZielChar == ' ' ) && (VTUBuffer[BufferFill-1] != ' ' )) {
								VTUBuffer[BufferFill]=ZielChar;
								VTUBufferFg[BufferFill]=CurrentFg;
								BufferFill++;
							} else if ( ZielChar != ' ' ) {
								VTUBuffer[BufferFill]=ZielChar;
								VTUBufferFg[BufferFill]=CurrentFg;
								BufferFill++;
							}
						}
					}
				}
				
				if ( BufferFill > 0 ) {
					if ( VTUBuffer[BufferFill-1] != ' ' ) {
						VTUBuffer[BufferFill]=' ';
						VTUBufferFg[BufferFill]=CurrentFg;
						BufferFill++;
					}
				}
			}
			
			
			i=0;
			n=0;
			
			while ( n < BufferFill) {
				
				if ( i == 0 ) UT[UTLoop].Fg=VTUBufferFg[n];
				
				UT[UTLoop].Zeile[i] = VTUBuffer[n];
				i++;
				n++;
				UT[UTLoop].Zeile[i] = 0x00;
				
				if ( ((n == BufferFill ) ||
					( VTUBufferFg[n] != UT[UTLoop].Fg )) &&
					( strlen(UT[UTLoop].Zeile) > 0 )) {
					SendMessage(UTList,LB_INSERTSTRING,InsertPos++,(LPARAM)UTLoop);
					SendMessage(UTList,LB_DELETESTRING,128,(LPARAM)0);
					UTLoop++;
					i=0;
					if ( UTLoop > 127 ) UTLoop=0;
				}
			}
			}
			// Zeile Voll //
			break;
			
			
		case WM_COMMAND:
			
			if ( (HWND)lParam==GetDlgItem(hDlg,IDC_EDIT1) ){
				i=GetDlgItemInt(hDlg,IDC_EDIT1,NULL,FALSE);		  
                if (( i >= 100 ) && ( i <= 899 )) UTPage=i-100;
				break;
			};
			
			
			if ((LOWORD(wParam) == IDOK ) || (LOWORD(wParam) == IDCANCEL )) {
				Restore_Mode();
				UTDialog=NULL;
				EndDialog(hDlg,TRUE);
			}
			break;
    }
	
	return(FALSE);
}

VOID APIENTRY HandleFocusState(HWND hDlg,LPDRAWITEMSTRUCT lpdis)
{
	int Typ;
	if (( lpdis->itemState & ODS_FOCUS) || (lpdis->itemState & ODS_SELECTED))
	{
		Typ=1;
	}
	else
	{
		Typ=0;
	}
	
	DrawEntireItem(hDlg,lpdis,Typ);
}

VOID APIENTRY DrawEntireItem(HWND hDlg,LPDRAWITEMSTRUCT lpdis,INT Typ)
{
	HFONT OldFont;
	int ColorNr;
	
	
	OldFont = SelectObject(lpdis->hDC, currFont);
	ColorNr = UT[lpdis->itemData].Fg;
	if ( ColorNr == 0 ) ColorNr=7;
	SetTextColor(lpdis->hDC,VTColourTable[ColorNr]);
	SetBkColor(lpdis->hDC,RGB(0,0,0));
  		TextOut(lpdis->hDC,10,lpdis->rcItem.top+1,UT[lpdis->itemData].Zeile,strlen(UT[lpdis->itemData].Zeile));
		SelectObject(lpdis->hDC,OldFont);
		return;
}

void  SetMenuAnalog()
{
	if ( Has_DVB == FALSE ) {
		EnableMenuItem(GetMenu(hWnd),IDM_SWAPMODE,MF_GRAYED);
	};
	
	CheckMenuItem( GetMenu(hWnd), ThreadClassId+1150, MF_CHECKED );
	CheckMenuItem( GetMenu(hWnd), PriorClassId+1160, MF_CHECKED );
	if ( Sync_Irq == TRUE ) CheckMenuItem( GetMenu(hWnd), IDM_SYNC_IRQ, MF_CHECKED );
	if ( WindowStyle == 0 ) {
		CheckMenuItem( GetMenu(hWnd), IDM_FENSTER, MF_CHECKED );
		CheckMenuItem( GetMenu(hWnd), IDM_640x480, MF_UNCHECKED );
		CheckMenuItem( GetMenu(hWnd), IDM_800x600, MF_UNCHECKED );
		CheckMenuItem( GetMenu(hWnd), IDM_OWN_RES, MF_UNCHECKED );
	} else if  ( WindowStyle == 1 ) {
		CheckMenuItem( GetMenu(hWnd), IDM_FENSTER, MF_UNCHECKED );
		CheckMenuItem( GetMenu(hWnd), IDM_640x480, MF_CHECKED );
		CheckMenuItem( GetMenu(hWnd), IDM_800x600, MF_UNCHECKED );
		CheckMenuItem( GetMenu(hWnd), IDM_OWN_RES, MF_UNCHECKED );
	} else if	( WindowStyle == 2 ) {
		CheckMenuItem( GetMenu(hWnd), IDM_FENSTER, MF_UNCHECKED );
		CheckMenuItem( GetMenu(hWnd), IDM_640x480, MF_UNCHECKED );
		CheckMenuItem( GetMenu(hWnd), IDM_800x600, MF_CHECKED );
		CheckMenuItem( GetMenu(hWnd), IDM_OWN_RES, MF_UNCHECKED );
	} else if	( WindowStyle == 3 ) {
		CheckMenuItem( GetMenu(hWnd), IDM_FENSTER, MF_UNCHECKED );
		CheckMenuItem( GetMenu(hWnd), IDM_640x480, MF_UNCHECKED );
		CheckMenuItem( GetMenu(hWnd), IDM_800x600, MF_UNCHECKED );
		CheckMenuItem( GetMenu(hWnd), IDM_OWN_RES, MF_CHECKED );
	};
	
	if ( WTVBPP == 0 ) {
		EnableMenuItem(GetMenu(hWnd),IDM_INTERN_SURFACE,MF_GRAYED);
		EnableMenuItem(GetMenu(hWnd),IDM_RAW_SURFACE,MF_GRAYED);
	}		
	if ( Can_Overlay) {
		EnableMenuItem(GetMenu(hWnd),IDM_OVERLAY,MF_ENABLED);
		EnableMenuItem(GetMenu(hWnd),IDM_RAW_OVERLAY,MF_ENABLED);
		if ( Can_Overlay_Strech == FALSE ) EnableMenuItem(GetMenu(hWnd),IDM_DX_OV_STRECH,MF_GRAYED);
		else EnableMenuItem(GetMenu(hWnd),IDM_DX_OV_STRECH,MF_ENABLED);
	} else {
		EnableMenuItem(GetMenu(hWnd),IDM_OVERLAY,MF_GRAYED);
		EnableMenuItem(GetMenu(hWnd),IDM_DX_OV_STRECH,MF_GRAYED);
		EnableMenuItem(GetMenu(hWnd),IDM_RAW_OVERLAY,MF_GRAYED);
	}
	if ( bFilterBlueLines == TRUE )CheckMenuItem( GetMenu(hWnd), IDM_NOBLUELINES, MF_CHECKED );
	if ( USE_DX_OV_STRECH == TRUE ) CheckMenuItem( GetMenu(hWnd), IDM_DX_OV_STRECH, MF_CHECKED );
	if ( WAIT_DX_VBI == TRUE )  CheckMenuItem( GetMenu(hWnd), IDM_DX_AUSTAST, MF_CHECKED );
	if ( USE_DX_LOCK == TRUE ) CheckMenuItem( GetMenu(hWnd), IDM_DX_LOCK, MF_CHECKED );
	if ( ColorMix == TRUE ) CheckMenuItem( GetMenu(hWnd), IDM_COLORMIX, MF_CHECKED ); 
	if ( HalfTVMode == TRUE ) CheckMenuItem( GetMenu(hWnd), IDM_HALFMODE, MF_CHECKED ); 
	if ( Decode_AutoDetect == TRUE ) CheckMenuItem( GetMenu(hWnd), IDM_AUTODETECT, MF_CHECKED ); 
    if (VBI_Flags&VBI_VT) {
					   CheckMenuItem( GetMenu(hWnd), IDM_VBI_VT, MF_CHECKED );
					   if ( Capture_VBI == TRUE ) {
						   EnableMenuItem(GetMenu(hWnd),IDM_CALL_VIDEOTEXT,MF_ENABLED);
						   EnableMenuItem(GetMenu(hWnd),IDM_VT_RESET,MF_ENABLED);
						   EnableMenuItem(GetMenu(hWnd),IDM_UNTERTITEL,MF_ENABLED);
						   EnableMenuItem(GetMenu(hWnd),IDM_PDC_OUT,MF_ENABLED);
						   EnableMenuItem(GetMenu(hWnd),IDM_VT_OUT,MF_ENABLED);
					   }
	}
	if (VBI_Flags&VBI_WINBIS) {
		CheckMenuItem( GetMenu(hWnd), IDM_VBI_WINBIS, MF_CHECKED );
					   if ( Capture_VBI == TRUE ) {
						   EnableMenuItem(GetMenu(hWnd),IDM_WINBIS_OUT,MF_ENABLED);
					   }
	}
	
	if (VBI_Flags&VBI_VC) {
					   CheckMenuItem( GetMenu(hWnd), IDM_VBI_VC, MF_CHECKED );
					   if ( Capture_VBI == TRUE ) {
						   EnableMenuItem(GetMenu(hWnd),IDM_CRYPTSTATUS,MF_ENABLED);
					   }
	}
	if (VBI_Flags&VBI_VPS) {
		CheckMenuItem( GetMenu(hWnd), IDM_VBI_VPS, MF_CHECKED );
					   if ( Capture_VBI == TRUE ) {
						   EnableMenuItem(GetMenu(hWnd),IDM_VPS_OUT,MF_ENABLED);
					   }
	}				   
	
	if ( Auto_MovieBoost == TRUE ) CheckMenuItem( GetMenu(hWnd), IDM_AUTOMB, MF_CHECKED ); 
	
	if ( USECARD == FALSE ) CheckMenuItem( GetMenu(hWnd), IDM_DONT_TOUCH_CARD, MF_CHECKED ); 
	
	if ( USETUNER == FALSE ) {
		CheckMenuItem( GetMenu(hWnd), IDM_DONT_TOUCH, MF_CHECKED );
		EnableMenuItem(GetMenu(hWnd),IDM_TUNER_0,MF_GRAYED);
		EnableMenuItem(GetMenu(hWnd),IDM_TUNER_1,MF_GRAYED);
		EnableMenuItem(GetMenu(hWnd),IDM_TUNER_2,MF_GRAYED);
		EnableMenuItem(GetMenu(hWnd),IDM_TUNER_3,MF_GRAYED);
		EnableMenuItem(GetMenu(hWnd),IDM_TUNER_5,MF_GRAYED);
		EnableMenuItem(GetMenu(hWnd),IDM_TUNER_6,MF_GRAYED);
		EnableMenuItem(GetMenu(hWnd),IDM_TUNER_7,MF_GRAYED);
		EnableMenuItem(GetMenu(hWnd),IDM_TUNER_8,MF_GRAYED);
		EnableMenuItem(GetMenu(hWnd),IDM_CHANNELPLUS,MF_GRAYED);
		EnableMenuItem(GetMenu(hWnd),IDM_CHANNELMINUS,MF_GRAYED);
		EnableMenuItem(GetMenu(hWnd),IDM_ANALOGSCAN,MF_GRAYED);
		EnableMenuItem(GetMenu(hWnd),IDM_MANUELL_TUNER,MF_GRAYED);
	};
	
	CheckMenuItem( GetMenu(hWnd), CountryCode+5000, MF_CHECKED );
	CheckMenuItem( GetMenu(hWnd), CardType+1080, MF_CHECKED );
    CheckMenuItem( GetMenu(hWnd), AudioSource+1110, MF_CHECKED );
	CheckMenuItem( GetMenu(hWnd), TVTYPE+1120, MF_CHECKED );
	CheckMenuItem( GetMenu(hWnd), ColourFormat+1130, MF_CHECKED );
    if ( VideoSource > 0 ) CheckMenuItem( GetMenu(hWnd), VideoSource+1089, MF_CHECKED );
	else {
		CheckMenuItem( GetMenu(hWnd), TunerType+1100, MF_CHECKED );
	}
	CheckMenuItem( GetMenu(hWnd), IDM_RAW_SURFACE, MF_UNCHECKED );
	CheckMenuItem( GetMenu(hWnd), IDM_RAW_OVERLAY, MF_UNCHECKED );
	CheckMenuItem( GetMenu(hWnd), IDM_INTERN_SURFACE, MF_UNCHECKED );
	CheckMenuItem( GetMenu(hWnd), IDM_OVERLAY, MF_UNCHECKED );
	CheckMenuItem( GetMenu(hWnd), IDM_DIB, MF_UNCHECKED );
	
    if ( ModeRAWSurface==TRUE ) CheckMenuItem( GetMenu(hWnd), IDM_RAW_SURFACE, MF_CHECKED );
	else if ( ModeRAWOverlay==TRUE ) CheckMenuItem( GetMenu(hWnd), IDM_RAW_OVERLAY, MF_CHECKED );
	else if ( OutDib==TRUE ) CheckMenuItem( GetMenu(hWnd), IDM_DIB, MF_CHECKED );
			 else if ( ModeOverlay == TRUE ) CheckMenuItem( GetMenu(hWnd), IDM_OVERLAY, MF_CHECKED );
			 else  CheckMenuItem( GetMenu(hWnd), IDM_INTERN_SURFACE, MF_CHECKED );
			 
			 CheckMenuItem( GetMenu(hWnd), 1600+MSPMode, MF_CHECKED );
			 CheckMenuItem( GetMenu(hWnd), 1630+MSPStereo, MF_CHECKED );
			 CheckMenuItem( GetMenu(hWnd), 1610+MSPMajorMode, MF_CHECKED );
			 CheckMenuItem( GetMenu(hWnd), 1620+MSPMinorMode, MF_CHECKED );
			 
			 if ( LoadExternBTDriver == TRUE ) CheckMenuItem( GetMenu(hWnd), IDM_BTNEWDRIVER, MF_CHECKED );
			 
			 if ( Capture_Video==TRUE ) CheckMenuItem( GetMenu(hWnd), IDM_VIDEO, MF_CHECKED );
			 if ( Capture_VBI ==TRUE ) {
				 CheckMenuItem( GetMenu(hWnd), IDM_VBI, MF_CHECKED );
				 EnableMenuItem(GetMenu(hWnd),IDM_VBI_VT,MF_ENABLED);
				 EnableMenuItem(GetMenu(hWnd),IDM_VBI_WINBIS,MF_ENABLED);
				 EnableMenuItem(GetMenu(hWnd),IDM_VBI_VD,MF_ENABLED);
				 EnableMenuItem(GetMenu(hWnd),IDM_VBI_VC,MF_ENABLED);
				 EnableMenuItem(GetMenu(hWnd),IDM_VBI_VPS,MF_ENABLED);
				 EnableMenuItem(GetMenu(hWnd),IDM_AUTODETECT,MF_ENABLED);
			 };
};

void SetMenu_CI()
{
	char Zeile[128];
	
	
	
	if ( ci_interface == 0 ) {
		EnableMenuItem(hMenu,IDM_CI_RESET,MF_GRAYED);
		EnableMenuItem(hMenu,IDM_CI_DIALOG,MF_GRAYED);
		EnableMenuItem(hMenu,IDM_MENU_SLOT_1,MF_GRAYED);
		EnableMenuItem(hMenu,IDM_MENU_SLOT_2,MF_GRAYED);
	} else {
		EnableMenuItem(hMenu,IDM_CI_RESET,MF_ENABLED);
		EnableMenuItem(hMenu,IDM_CI_DIALOG,MF_ENABLED);
		if ( ci_module[0].Present > 0 )  {
			if ( ci_module[0].ModuleName[0] != 0x00 ) {
				sprintf(Zeile,"Menu Slot 1 ( %s )",ci_module[0].ModuleName);
				ModifyMenu(hMenu,IDM_MENU_SLOT_1, MF_BYCOMMAND | MF_STRING , IDM_MENU_SLOT_1, Zeile); 
			} else {
				ModifyMenu(hMenu,IDM_MENU_SLOT_1, MF_BYCOMMAND | MF_STRING , IDM_MENU_SLOT_1, "Menu Slot 1 ( Cam noch nicht bekannt )"); 
			}
			EnableMenuItem(hMenu,IDM_MENU_SLOT_1,MF_ENABLED);
		} else {
			EnableMenuItem(hMenu,IDM_MENU_SLOT_1,MF_GRAYED);
		};
		if ( ci_module[1].Present > 0 ) {
			if ( ci_module[1].ModuleName[0] != 0x00 ) {
				sprintf(Zeile,"Menu Slot 2 ( %s )",ci_module[1].ModuleName);
				ModifyMenu(hMenu,IDM_MENU_SLOT_2, MF_BYCOMMAND | MF_STRING , IDM_MENU_SLOT_2, Zeile); 
			} else {
				ModifyMenu(hMenu,IDM_MENU_SLOT_2, MF_BYCOMMAND | MF_STRING , IDM_MENU_SLOT_2, "Menu Slot 2 ( Cam noch nicht bekannt )"); 
			}
			EnableMenuItem(hMenu,IDM_MENU_SLOT_2,MF_ENABLED);
		} else {
			EnableMenuItem(hMenu,IDM_MENU_SLOT_2,MF_GRAYED);
		};
		
	}
}


void  SetMenuDigital()
{
	char Name[128];
	int i;
	
	for ( i = 0; i < 5 ; i++ ) {
		
		if ( Ext_Dll[i].Extern_Menu  != NULL ) {
			
			if ( Ext_Dll[i].Extern_Send_Dll_ID_Name != NULL ) (Ext_Dll[i].Extern_Send_Dll_ID_Name)(&Name[0]);
			else sprintf(Name,"DLL %d Menü",i);
			InsertMenu( hMenu , 0xFFFFFFFF,MF_BYPOSITION | MF_POPUP | MF_ENABLED, (unsigned int)Ext_Dll[i].Extern_Menu,(LPCTSTR)Name);
			
		};
	};
	
	if ( Has_BT == FALSE ) EnableMenuItem(hMenu,IDM_SWAPMODE,MF_GRAYED);
	
	CheckMenuItem( hMenu, DVBDecoderType+1080, MF_CHECKED );
	CheckMenuItem( hMenu, DVBTunerType+1100, MF_CHECKED );
	
	CheckMenuItem( hMenu, ThreadClassId+1150, MF_CHECKED );
	CheckMenuItem( hMenu, PriorClassId+1160, MF_CHECKED );
	if ( Sync_Irq == TRUE ) CheckMenuItem( GetMenu(hWnd), IDM_SYNC_IRQ, MF_CHECKED );
	if ( WindowStyle == 0 ) {
		CheckMenuItem( hMenu, IDM_FENSTER, MF_CHECKED );
		CheckMenuItem( hMenu, IDM_640x480, MF_UNCHECKED );
		CheckMenuItem( hMenu, IDM_800x600, MF_UNCHECKED );
		CheckMenuItem( hMenu, IDM_OWN_RES, MF_UNCHECKED );
	} else if  ( WindowStyle == 1 ) {
		CheckMenuItem( hMenu, IDM_FENSTER, MF_UNCHECKED );
		CheckMenuItem( hMenu, IDM_640x480, MF_CHECKED );
		CheckMenuItem( hMenu, IDM_800x600, MF_UNCHECKED );
		CheckMenuItem( hMenu, IDM_OWN_RES, MF_UNCHECKED );
	} else if	( WindowStyle == 2 ) {
		CheckMenuItem( hMenu, IDM_FENSTER, MF_UNCHECKED );
		CheckMenuItem( hMenu, IDM_640x480, MF_UNCHECKED );
		CheckMenuItem( hMenu, IDM_800x600, MF_CHECKED );
		CheckMenuItem( hMenu, IDM_OWN_RES, MF_UNCHECKED );
	} else if	( WindowStyle == 3 ) {
		CheckMenuItem( hMenu, IDM_FENSTER, MF_UNCHECKED );
		CheckMenuItem( hMenu, IDM_640x480, MF_UNCHECKED );
		CheckMenuItem( hMenu, IDM_800x600, MF_UNCHECKED );
		CheckMenuItem( hMenu, IDM_OWN_RES, MF_CHECKED );
	};
	
	if ( WTVBPP == 0 ) {
		EnableMenuItem(hMenu,IDM_INTERN_SURFACE,MF_GRAYED);
		EnableMenuItem(hMenu,IDM_RAW_SURFACE,MF_GRAYED);
	}		
	if ( Can_Overlay) {
		EnableMenuItem(hMenu,IDM_OVERLAY,MF_ENABLED);
		EnableMenuItem(hMenu,IDM_RAW_OVERLAY,MF_ENABLED);
		if ( Can_Overlay_Strech == FALSE ) EnableMenuItem(hMenu,IDM_DX_OV_STRECH,MF_GRAYED);
		else EnableMenuItem(hMenu,IDM_DX_OV_STRECH,MF_ENABLED);
	} else {
		EnableMenuItem(hMenu,IDM_OVERLAY,MF_GRAYED);
		EnableMenuItem(hMenu,IDM_DX_OV_STRECH,MF_GRAYED);
		EnableMenuItem(hMenu,IDM_RAW_OVERLAY,MF_GRAYED);
	}
	
	if ( USE_TV_16_9 == TRUE ) CheckMenuItem( hMenu, IDM_TV_16_9, MF_CHECKED );
	else  CheckMenuItem( hMenu, IDM_TV_16_9, MF_UNCHECKED );
	
	if ( USE_DX_OV_STRECH == TRUE ) CheckMenuItem( hMenu, IDM_DX_OV_STRECH, MF_CHECKED );
	if ( WAIT_DX_VBI == TRUE )  CheckMenuItem( hMenu, IDM_DX_AUSTAST, MF_CHECKED );
	if ( USE_DX_LOCK == TRUE ) CheckMenuItem( hMenu, IDM_DX_LOCK, MF_CHECKED );
	if ( ColorMix == TRUE ) CheckMenuItem( hMenu, IDM_COLORMIX, MF_CHECKED ); 
	if ( HalfTVMode == TRUE ) CheckMenuItem( hMenu, IDM_HALFMODE, MF_CHECKED ); 
	
	if ( Auto_MovieBoost == TRUE ) CheckMenuItem( hMenu, IDM_AUTOMB, MF_CHECKED ); 
    CheckMenuItem( hMenu, AudioSource+1110, MF_CHECKED );
	CheckMenuItem( hMenu, TVTYPE+1120, MF_CHECKED );
	CheckMenuItem( hMenu, ColourFormat+1130, MF_CHECKED );
	CheckMenuItem( hMenu, IDM_RAW_SURFACE, MF_UNCHECKED );
	CheckMenuItem( hMenu, IDM_RAW_OVERLAY, MF_UNCHECKED );
	CheckMenuItem( hMenu, IDM_INTERN_SURFACE, MF_UNCHECKED );
	CheckMenuItem( hMenu, IDM_OVERLAY, MF_UNCHECKED );
	CheckMenuItem( hMenu, IDM_DIB, MF_UNCHECKED );
	
    if ( Sync_Irq == TRUE ) CheckMenuItem( GetMenu(hWnd), IDM_SYNC_IRQ, MF_CHECKED );
	
	SetMenu_CI();
	
	if ( DVB_AUDIO == 0 ) {
		CheckMenuItem( GetMenu(hWnd), IDM_DVB_AUDIO_CRYSTAL, MF_UNCHECKED );
		CheckMenuItem( GetMenu(hWnd), IDM_DVB_AUDIO_TI, MF_CHECKED );
	} else {
		CheckMenuItem( GetMenu(hWnd), IDM_DVB_AUDIO_CRYSTAL, MF_CHECKED );
		CheckMenuItem( GetMenu(hWnd), IDM_DVB_AUDIO_TI, MF_UNCHECKED );
	}
    if ( DVB_VT == FALSE ) CheckMenuItem(hMenu, IDM_CAPTURE_DVB_VT, MF_UNCHECKED );
	else  CheckMenuItem(hMenu, IDM_CAPTURE_DVB_VT, MF_CHECKED );
    if ( DVB_WINBIS == FALSE ) CheckMenuItem(hMenu, IDM_CAPTURE_DVB_WINBIS, MF_UNCHECKED );
	else  CheckMenuItem(hMenu, IDM_CAPTURE_DVB_WINBIS, MF_CHECKED );
	
    if ( ModeRAWSurface==TRUE ) CheckMenuItem( hMenu, IDM_RAW_SURFACE, MF_CHECKED );
	else if ( ModeRAWOverlay==TRUE ) CheckMenuItem( hMenu, IDM_RAW_OVERLAY, MF_CHECKED );
	else if ( OutDib==TRUE ) CheckMenuItem( hMenu, IDM_DIB, MF_CHECKED );
			 else if ( ModeOverlay == TRUE ) CheckMenuItem( hMenu, IDM_OVERLAY, MF_CHECKED );
			 else  CheckMenuItem( hMenu, IDM_INTERN_SURFACE, MF_CHECKED );
			 
			 if ( Capture_Digi_Text == TRUE ) {
             		  EnableMenuItem(hMenu,IDM_CALL_VIDEOTEXT,MF_ENABLED);
					  EnableMenuItem(hMenu,IDM_UNTERTITEL,MF_ENABLED);
					  EnableMenuItem(hMenu,IDM_VT_OUT,MF_ENABLED);
					  EnableMenuItem(hMenu,IDM_UNTERTITEL,MF_ENABLED);
                      EnableMenuItem(hMenu,IDM_CAPTURE_DVB_VT,MF_GRAYED);
                      EnableMenuItem(hMenu,IDM_CAPTURE_DVB_WINBIS,MF_GRAYED);
                      EnableMenuItem(hMenu,IDM_WINBIS_OUT,MF_GRAYED);
					  EnableMenuItem(hMenu,IDM_CALL_VIDEOTEXT,MF_ENABLED);
					  EnableMenuItem(hMenu,IDM_VT_OUT,MF_ENABLED);
					  
			 } else {
				 EnableMenuItem(hMenu,IDM_CAPTURE_DVB_VT,MF_ENABLED);
				 EnableMenuItem(hMenu,IDM_CAPTURE_DVB_WINBIS,MF_ENABLED);
				 
				 if ( DVB_VT == FALSE ) {
					 EnableMenuItem(hMenu,IDM_CALL_VIDEOTEXT,MF_GRAYED);
					 EnableMenuItem(hMenu,IDM_VT_OUT,MF_GRAYED);
					 EnableMenuItem(hMenu,IDM_UNTERTITEL,MF_GRAYED);
				 } else {
					 EnableMenuItem(hMenu,IDM_CALL_VIDEOTEXT,MF_ENABLED);
					 EnableMenuItem(hMenu,IDM_VT_OUT,MF_ENABLED);
					 EnableMenuItem(hMenu,IDM_UNTERTITEL,MF_ENABLED);
				 }
				 if ( DVB_WINBIS == FALSE ) {
					 EnableMenuItem(hMenu,IDM_WINBIS_OUT,MF_GRAYED);
				 } else {
					 EnableMenuItem(hMenu,IDM_WINBIS_OUT,MF_ENABLED);
				 };
			 };
			 
			 if ( Capture_Event_EIT == TRUE ) {
             		  EnableMenuItem(hMenu,IDM_EVENT_INFO,MF_ENABLED);
					  CheckMenuItem( hMenu, IDM_CAPTURE_EIT, MF_CHECKED );
			 } else {
             		  EnableMenuItem(hMenu,IDM_EVENT_INFO,MF_GRAYED);
					  CheckMenuItem( hMenu, IDM_CAPTURE_EIT, MF_UNCHECKED );
			 };
			 
			 if ( Capture_TID == TRUE ) {
				 CheckMenuItem( hMenu, IDM_CAPTURE_TID, MF_CHECKED );
			 } else {
				 CheckMenuItem( hMenu, IDM_CAPTURE_TID, MF_UNCHECKED );
			 };
			 
			 UpdateWindow(hWnd);	
};


BOOL ValidModes(char Mode)
{
	if (( Mode == 'A' ) && ( Has_BT == TRUE )) return(TRUE);
	if (( Mode == 'D' ) && ( Has_DVB == TRUE )) return(TRUE);
	return(FALSE);
}

void Set_Mode_Digital( void )
{
	Write_Log("SAA7146 Set_Mode_Digital");
	DOSIZE=FALSE;
	Set_Capture(4);
	Stop_Thread();
	Run_BT_Irq(FALSE);
	SetAudioSource(4);
	Audio_SetVolume(0);
	ResetIrq=0;
	DecodeVCrypt=FALSE; 
	DecodeNagra=FALSE;
	MoreLinesLock=FALSE;
	DecodeDecoderVCrypt=FALSE;
	Free_Nagra_Decoding();
	DestroyMenu(hMenu);
	hMenu=LoadMenu(hInst,"DIGITALMENU");
	if ( Show_Menu == TRUE ) SetMenu(hWnd,hMenu);
	SetMenuDigital();
	DestroyAcceleratorTable(hAccel);
	hAccel = LoadAccelerators (hInst, "DIGITALACCEL");
	
	// Digital  // 1 Screens-Buffer
	pDisplay[0]=DVB_Display_dma.pUserAddr;
	Current_Mode = 1; 
	ResetIrq=0;
	Init_Screen_Struct();
	DOSIZE=TRUE;
	Run_DVB_Irq(TRUE,TRUE);
	DVB_SetVolume((unsigned char)InitialDVBVolumeLinks, (unsigned char)InitialDVBVolumeRechts);
	Set_Capture(5);
	
};

void Set_Mode_Analog( void )
{
	int i;
	
	Write_Log("BT8X8 Set_Mode_Analog");
	DOSIZE=FALSE;
	Set_Capture(4);
	Stop_Thread();
	Stop_Osd_Key_Event();
	KillTimer(hWnd,22);
	Run_DVB_Irq(FALSE,TRUE);
	DVB_SetVolume((unsigned char)0, (unsigned char)0);
	DestroyMenu(hMenu);
	hMenu=LoadMenu(hInst,"ANALOGMENU");
	if ( Show_Menu == TRUE ) SetMenu(hWnd,hMenu);
	SetMenuAnalog();
	DestroyAcceleratorTable(hAccel);
	hAccel = LoadAccelerators(hInst, "ANALOGACCEL");
	Current_Mode=0;
	ResetIrq=0;
	for ( i=0; i < 5 ; i++ ) {
		pDisplay[i]=Display_dma[i]->pUserAddr;
	}
	Init_Screen_Struct();
	Run_BT_Irq(TRUE);
	DOSIZE=TRUE;
	Set_Capture(5);
	ResetIrq=0;
	SetAudioSource(AudioSource);
	Audio_SetVolume(InitialVolume);
	
};


void Init_Kernel_Memory( void )
{ 
	if ( Has_BT == TRUE ) {
		if ( !Init_BT_Kernel_Memory() ) {
			BT_Close();
			HardWareFound--;
			Has_BT = FALSE;
		};
	};
	
	if ( Has_DVB == TRUE ) {
		if ( !Init_DVB_Memory() ) {
            DVB_Close();
			HardWareFound--;
			Has_DVB = FALSE;
		};
	};
};


void VT_ChannelChange()
{
	int i;
	int Save_Flags;
	BOOL Save_Capture;
	BOOL Save_DVB_VT;
	
	Save_Flags=VBI_Flags;
	Save_Capture=Capture_Digi_Text;
	Save_DVB_VT=DVB_VT;
	
	if (VBI_Flags&VBI_VT)  VBI_Flags-=VBI_VT;
	Capture_Digi_Text = FALSE;
	DVB_VT=FALSE;
	for (i=0; i < 12; i++ ) UTPages[i]=0;
	UTCount=0;
	Sleep(2);Sleep(0);Sleep(2);Sleep(0);Sleep(2);
	Sleep(2);Sleep(0);Sleep(2);Sleep(0);Sleep(2);
	Sleep(2);Sleep(0);Sleep(2);Sleep(0);Sleep(2);
	
	for ( i=0; i < 800 ; i++ ) {
		VTFrame[i].SubCount=0;
		if ( VTFrame[i].SubPage != NULL  ) free(VTFrame[i].SubPage);
		VTFrame[i].SubPage=NULL;
	};
	
	Capture_Digi_Text = Save_Capture;
	VBI_Flags=Save_Flags;
	DVB_VT=Save_DVB_VT;
	VT_Cache=0;
}

extern unsigned char revham[16];

void Work_Text(unsigned char *TmpBuffer)
{
	
	int PageNum;
    int SubPagesNummer;
    int sub;
    int SubPagesAnzahl;
	int fd;
    unsigned char mag;
    unsigned char PageLang;
	
	char RawName[255];
	unsigned int Page_CRC;
	unsigned char *Ham8_4;
    BOOL Has_Sub;
    BOOL Is_UT;
	if ( TmpBuffer == NULL ) return;
	
	if ( VT_EXPORT_TYP == 0 ) {
		sprintf(RawName,"%s\\ttx.raw",VT_BASE_DIR);
		fd=_open(RawName,_O_RDWR  | _O_CREAT | _O_BINARY,_S_IREAD | _S_IWRITE);
		if (fd >= 0) {
			_lseek( fd , 0, SEEK_END);
			write(fd,TmpBuffer,1008);
			close(fd);
		}
		return;
	}
	
	if (!Capture_Digi_Text && !DVB_VT && !(VBI_Flags & VBI_VT)) return;
	
	mag = TmpBuffer[1000] >> 4;
	TmpBuffer[1000] &= 0x0f;
	if (TmpBuffer[1000] > 0x09 || TmpBuffer[1001] > 0x09) return;
	if (mag == 0 && TmpBuffer[8] >= '1' && TmpBuffer[8] <= '7') mag = TmpBuffer[8] & 0x07;
	
	PageNum = mag*100 +
		TmpBuffer[1001]*10 +
		TmpBuffer[1000] - 100;
	sub = (TmpBuffer[1002]&0x0f) +
		((TmpBuffer[1003]&0x07)<<4) +
		((TmpBuffer[1004]&0x0f)<<8) +
		((TmpBuffer[1005]&0x03)<<12);
	
	if (( VT_Lang <= 0 ) || ( VT_Lang > 6 ))  {
		PageLang = revham[TmpBuffer[1007]] & 0x7;
    } else PageLang=VT_Lang-1;
	
	Ham8_4=TmpBuffer;
	for (i=0; i<40 ; i++) {
		VT_Header_Line[i]=*(Ham8_4++)&0x7f;  // Remove parity bit
	}
	
	if (PageNum >= 0 && PageNum < 800) {
		
		if ( LastWork == PageNum && TmpBuffer[1006]&0x2 != 0x2) return;
		LastWork = PageNum;
		// Gültige Seite
		
		// 	     	TmpBuffer[1003]&0x8; // C4 : erase page
		//		 	TmpBuffer[1005]&0x4; // C5 : news
		//		 	TmpBuffer[1005]&0x8; // C6 : subtitle
		//		 	TmpBuffer[1006]&0x1; // C7 : Suppress Header
		//		 	TmpBuffer[1006]&0x2; // C8 : Update
		//			TmpBuffer[1006]&0x4; // C9 : Interrupted
		//			TmpBuffer[1006]&0x8; // C10 : Inhibit Display
		//			TmpBuffer[1007]&0x1; // C11 : Mag Serial
		//			TmpBuffer[1007]&0xE; // C12/C13/C14 : Language
		
		Is_UT = (TmpBuffer[1005]&0x08 == 0x08);
		SubPagesNummer = sub % 16;
		SubPagesAnzahl = 1;
		Has_Sub=(SubPagesNummer > 0);
		if (Has_Sub) SubPagesNummer--;
		
		Page_CRC=VT_CRC(TmpBuffer);
		
		if (VTFrame[PageNum].SubCount > SubPagesNummer &&
			VTFrame[PageNum].SubPage[SubPagesNummer].CRC == Page_CRC) {
			
			VTFrame[PageNum].SubPage[SubPagesNummer].crc_count++;
			if (VTFrame[PageNum].SubPage[SubPagesNummer].crc_count > 9) {
				VTFrame[PageNum].SubPage[SubPagesNummer].CRC = 0;
				VTFrame[PageNum].SubPage[SubPagesNummer].crc_count = 0;
			}
			return;
		}
		
		if (Has_Sub && SubPagesNummer >= VTFrame[PageNum].SubCount) {
			SubPagesAnzahl = Get_SubPage_Anzahl(SubPagesNummer+1, TmpBuffer);
		}
		
		TXTStatus=Save_TXT_Page(PageNum,  SubPagesNummer, SubPagesAnzahl,Page_CRC, PageLang, TmpBuffer);
		if (UTDialog && PageNum == UTPage && SubPagesNummer == 0) {
			PostMessage(UTDialog,WM_USER,64,0);
		}
		
		if (ShowVTInfo != NULL)
			SetDlgItemInt(ShowVTInfo, TEXT2, PageNum+100, FALSE);
		
		if (VT_ALWAYS_EXPORT && TXTStatus == 0)
			Export_VT_Page(NULL, PageNum+100, SubPagesNummer);
		
		if (Is_UT) {
			i=0;
			while (i < UTCount && i < 12 && UTPages[i] != PageNum+100) i++;
			if (i < 12 && i >= UTCount) {
				UTPages[UTCount]=PageNum+100;
				UTCount++;
			}
		}
	}
 }
 
 int Save_TXT_Page(int PageNum, int SubPageNum, int SubPagesAnzahl,unsigned int Page_CRC, unsigned char PageLang, unsigned char *Buffer)
 {
	 struct TVTPage *TmpVTFrame;
	 int i;
	 
	 if (VTFrame[PageNum].SubCount == 0) {
		 VTFrame[PageNum].SubPage=calloc(SubPagesAnzahl,sizeof(struct TVTPage));
		 if ( VTFrame[PageNum].SubPage == NULL ) {
			 strcpy(TXTError,"Kein Speicher mehr frei (calloc)");
			 return(-1);
		 };
		 VTFrame[PageNum].SubCount=SubPagesAnzahl;
		 for ( i=0; i < SubPagesAnzahl ; i++ ) {
			 VTFrame[PageNum].SubPage[i].bUpdated=0x00;
			 VTFrame[PageNum].SubPage[i].Fill=0x00;
			 VTFrame[PageNum].SubPage[i].CRC=0;
			 VTFrame[PageNum].SubPage[i].crc_count=0x00;
			 VTFrame[PageNum].SubPage[i].Lang=0x00;
		 };
	 } else if ( SubPageNum >= VTFrame[PageNum].SubCount ) {
		 TmpVTFrame=VTFrame[PageNum].SubPage;
         VTFrame[PageNum].SubPage=realloc(TmpVTFrame,sizeof(struct TVTPage)*SubPagesAnzahl);
		 if ( VTFrame[PageNum].SubPage == NULL ) {
			 strcpy(TXTError,"Kein Speicher mehr frei (realloc)");
			 VTFrame[PageNum].SubPage=TmpVTFrame;
			 return(-1);
		 };
		 
		 for ( i=VTFrame[PageNum].SubCount; i < SubPagesAnzahl ; i++ ) {
			 VTFrame[PageNum].SubPage[i].bUpdated=0x00;
			 VTFrame[PageNum].SubPage[i].Fill=0x00;
			 VTFrame[PageNum].SubPage[i].CRC=0;
			 VTFrame[PageNum].SubPage[i].crc_count=0x00;
			 VTFrame[PageNum].SubPage[i].Lang=0x00;
		 };
		 
		 VTFrame[PageNum].SubCount=SubPagesAnzahl;
	 };
	 
  	  if ( VTFrame[PageNum].SubPage[SubPageNum].Fill == 0x00 ) VT_Cache++;
	  VTFrame[PageNum].SubPage[SubPageNum].CRC=Page_CRC;
	  memcpy(VTFrame[PageNum].SubPage[SubPageNum].Frame,Buffer,1000);
	  VTFrame[PageNum].SubPage[SubPageNum].bUpdated=0x01;
	  VTFrame[PageNum].SubPage[SubPageNum].Fill=0x01;
	  VTFrame[PageNum].SubPage[SubPageNum].Lang=PageLang;
	  return(0);
 }
 
 
 
 int Get_Page_Num(unsigned char *Buffer)
 {
	 int PageNum;
	 char PageNumChar[4];
	 int i;
	 
	 if  ((Is_Val(Buffer[7]) == FALSE ) && 
		 (Is_Val(Buffer[8]) == TRUE)   &&
		 (Is_Val(Buffer[9]) == TRUE)   &&
		 (Is_Val(Buffer[10]) == TRUE)  &&
		 (Is_Val(Buffer[11]) == FALSE)) {
		 
		 PageNumChar[0]=Buffer[8];
		 PageNumChar[1]=Buffer[9];
		 PageNumChar[2]=Buffer[10];
		 PageNumChar[3]=0x00;    
		 PageNum=atoi(PageNumChar);
		 PageNum-=100;
		 return(PageNum);
	 };
	 
	 for ( i=8 ; i < 32; i++  ) {
		 if  ((Is_Val(Buffer[i]) == FALSE ) && 
			 (Is_Val(Buffer[i+1]) == TRUE)   &&
			 (Is_Val(Buffer[i+2]) == TRUE)   &&
			 (Is_Val(Buffer[i+3]) == TRUE)  &&
			 (Is_Val(Buffer[i+4]) == FALSE)) {
			 PageNumChar[0]=Buffer[i+1];
			 PageNumChar[1]=Buffer[i+2];
			 PageNumChar[2]=Buffer[i+3];
			 PageNumChar[3]=0x00;    
			 PageNum=atoi(PageNumChar);
			 PageNum-=100;
			 return(PageNum);
		 };
	 };
	 return(-1);
 };
 
 int Get_SubPage_Anzahl(int Current,unsigned char *Buffer)
 {
	 int SubPageAnzahl;
	 char SubPageNumChar[8];
	 char SubPageCountChar[8];
	 unsigned char *Pos;
	 int i;
	 sprintf(SubPageNumChar,"%d/",Current);
	 i=0;
	 Pos = Buffer;
	 if ( Current < 10 ) {
		 for ( i=0; i < 997 ; i++ ) {
			 if (( *Pos == SubPageNumChar[0] ) &&
				 ( *(Pos+1) == SubPageNumChar[1] )) {
                 Pos+=2; 
				 if ( Is_Val(*Pos) == TRUE ) {
					 SubPageCountChar[0]=*(Pos++);
					 SubPageCountChar[1]=0x00;
					 if ( Is_Val(*Pos) == TRUE ) {
						 SubPageCountChar[1]=*(Pos++);
						 SubPageCountChar[2]=0x00;
						 if ( Is_Val(*Pos) == TRUE ) {
							 SubPageCountChar[2]=*(Pos++);
							 SubPageCountChar[3]=0x00;
						 }; // 3 Stellig
					 }; // 2 Stellig
					 SubPageAnzahl=atoi(SubPageCountChar);
					 if (( SubPageAnzahl >= Current ) && ( SubPageAnzahl <= 64 )) return(SubPageAnzahl);
				 }; // 1 Stellig
			 };  // X/ Gefunden 
			 Pos++;
		 }; // Schleife über Schirm
		 return( Current ); // Nix gefunden also das
	 } // < 10
	 
	 
	 for ( i=0; i < 997; i++ ) {
		 if (( *Pos == SubPageNumChar[0] ) &&
			 ( *(Pos+1) == SubPageNumChar[1] ) &&
			 ( *(Pos+2) == SubPageNumChar[2] )) {
			 Pos+=3; 
			 if ( Is_Val(*Pos) == TRUE ) {
				 SubPageCountChar[0]=*(Pos++);
				 SubPageCountChar[1]=0x00;
				 if ( Is_Val(*Pos) == TRUE ) {
					 SubPageCountChar[1]=*(Pos++);
					 SubPageCountChar[2]=0x00;
					 if ( Is_Val(*Pos) == TRUE ) {
						 SubPageCountChar[2]=*(Pos++);
						 SubPageCountChar[3]=0x00;
					 }; // 3 Stellig
				 }; // 2 Stellig
				 SubPageAnzahl=atoi(SubPageCountChar);
				 if (( SubPageAnzahl >= Current ) && ( SubPageAnzahl <= 64 )) return(SubPageAnzahl);
			 }; // 1 Stellig
		 };  // XX/ Gefunden 
		 Pos++;
	 }; // Schleife über Schirm
	 return( Current ); // Nix gefunden also das
 }
 
 
 
 unsigned int VT_CRC(unsigned char *VTptr)
 {
	 int i;
	 unsigned char *ptr;
	 unsigned int sum;
	 ptr=VTptr;
	 for ( i=0; i < 40; i++ ) {
		 *ptr=*ptr&0x7f;
		 ptr++;
	 };
	 
	 sum=0;
	 for ( i=0; i < 960; i++ ) {
		 *ptr=*ptr&0x7f;
		 sum=sum+*(ptr++);
	 };
	 return(sum);
 }
 
 BOOL APIENTRY MDBootOptionProc
	 (
	 HWND hDlg,
	 UINT message,
	 UINT wParam,
	 LONG lParam)
 {
	 
	 char Path[255];
	 
	 switch (message) {
		 
	 case WM_INITDIALOG:
		 
		 if ( NT == FALSE ) {
			 Keep_Driver_Loaded=FALSE;
			 EnableWindow(GetDlgItem(hDlg,IDC_CHECK1),FALSE);
			 EnableWindow(GetDlgItem(hDlg,IDOK),FALSE);
		 };
		 
		 if ( Fail_BOOT==TRUE ) {
			 Keep_Driver_Loaded=TRUE;
			 SetDlgItemText(hDlg,IDC_TEXT1,"Nach der Installation der Treiber wird das System gebootet.");
		 }
		 
		 CheckDlgButton(hDlg,IDC_CHECK1, Keep_Driver_Loaded );
		 
		 
		 return(TRUE);
		 
		 
	 case WM_COMMAND:
		 
		 if (LOWORD(wParam) == IDOK ) {
			 Keep_Driver_Loaded =IsDlgButtonChecked(hDlg, IDC_CHECK1);
			 Write_INI("MultiDec.ini");
			 if ( Fail_BOOT==TRUE ) {
				 GetCurrentDirectory(  sizeof (Path),Path );
				 strcat(Path,"\\MDHALDR.SYS");
				 if ( ! LoadDeviceDriver( "MultiDec HAL-Treiber", Path, &Device_Handle,TRUE  ) )  {
					 MessageBox(hDlg,"NT-Device-Treiber MDHALDR.sys kann nicht installiert werden","Multi-Decode",MB_ICONSTOP|MB_OK);
					 ExitThread(9);
					 if ( LogFile != NULL ) fclose(LogFile);
				 };
				 
				 sprintf(ShutDownText,"\nAchtung !\nReboot nach Treiberinstallation");
				 CurrentShutDownFlag=EWX_REBOOT;
				 DialogBox(hInst,"SHUTDOWN",NULL,Stop_SystemProc);
			 };		 
			 EndDialog(hDlg,TRUE);
		 }
		 
		 if (LOWORD(wParam) == IDCANCEL ) {
			 EndDialog(hDlg,TRUE);
		 }
		 
		 break;
	 }
	 
	 return(FALSE);
 }
 
 BOOL APIENTRY MDShutdownOptionProc
	 (
	 HWND hDlg,
	 UINT message,
	 UINT wParam,
	 LONG lParam)
 {
	 
	 
	 switch (message) {
		 
	 case WM_INITDIALOG:
		 if ( ShutDownValue&EWX_LOGOFF ) CheckDlgButton(hDlg,IDC_RADIO1, TRUE );
		 else if ( ShutDownValue&EWX_POWEROFF ) CheckDlgButton(hDlg,IDC_RADIO2, TRUE );
		 else if ( ShutDownValue&EWX_REBOOT ) CheckDlgButton(hDlg,IDC_RADIO3, TRUE );
		 else if ( ShutDownValue&EWX_SHUTDOWN ) CheckDlgButton(hDlg,IDC_RADIO4, TRUE );
		 if ( ShutDownValue&EWX_FORCE ) CheckDlgButton(hDlg,IDC_CHECK1, TRUE );
		 return(TRUE);
		 
		 
	 case WM_COMMAND:
		 
		 if (LOWORD(wParam) == IDOK ) {
			 
			 if (IsDlgButtonChecked(hDlg,IDC_RADIO1)==TRUE ) ShutDownValue=EWX_LOGOFF;
			 else if (IsDlgButtonChecked(hDlg,IDC_RADIO2)==TRUE ) ShutDownValue=EWX_POWEROFF;
			 else if (IsDlgButtonChecked(hDlg,IDC_RADIO3)==TRUE ) ShutDownValue=EWX_REBOOT;
			 else if (IsDlgButtonChecked(hDlg,IDC_RADIO4)==TRUE ) ShutDownValue=EWX_SHUTDOWN;
			 
			 if (IsDlgButtonChecked(hDlg,IDC_CHECK1)==TRUE ) ShutDownValue+=EWX_FORCE;
			 EndDialog(hDlg,TRUE);
			 
		 }
		 
		 if (LOWORD(wParam) == IDCANCEL ) {
			 EndDialog(hDlg,TRUE);
		 }
		 
		 break;
	 }
	 
	 return(FALSE);
 }
 
 BOOL APIENTRY MDShutdownSetProc
	 (
	 HWND hDlg,
	 UINT message,
	 UINT wParam,
	 LONG lParam)
 {
	 char Zeile[32];
	 
	 switch (message) {
		 
	 case WM_INITDIALOG:
		 if ( ShutdownZeit.H != 0xff ) {
			 sprintf(Zeile,"%02d",ShutdownZeit.H);
			 SetDlgItemText(hDlg,IDC_EDIT1,Zeile);
			 sprintf(Zeile,"%02d",ShutdownZeit.M);
			 SetDlgItemText(hDlg,IDC_EDIT2,Zeile);
		 } else {
			 SetDlgItemText(hDlg,IDC_EDIT1,"");
			 SetDlgItemText(hDlg,IDC_EDIT2,"");
		 }
		 
		 sprintf(Zeile,"%02d",TDT.Hour);
		 SetDlgItemText(hDlg,IDC_EDIT3,Zeile);
		 sprintf(Zeile,"%02d",TDT.Min);
		 SetDlgItemText(hDlg,IDC_EDIT4,Zeile);
		 
		 SetFocus(GetDlgItem(hDlg,IDC_EDIT1));
		 return(TRUE);
		 
		 
	 case WM_COMMAND:
		 
		 
		 
		 
		 if (LOWORD(wParam) == IDOK ) {
			 
			 GetDlgItemText(hDlg,IDC_EDIT1,Zeile,32);
			 if (( Zeile[0] == 0x00 ) || ( Zeile[0] == 0x20 )) ShutdownZeit.H = 0xff;
			 else {
				 i=atoi(Zeile);
				 if (( i < 0 ) || ( i > 23 )) {
					 MessageBox(hDlg,"Stunden zwischen 0 und 23 Uhr !!!","Multi-Decode",MB_ICONSTOP|MB_OK);
					 SetFocus(GetDlgItem(hDlg,IDC_EDIT1));
					 break;
				 }
				 ShutdownZeit.H=i;
			 };
			 
			 GetDlgItemText(hDlg,IDC_EDIT2,Zeile,32);
			 if (( Zeile[0] == 0x00 ) || ( Zeile[0] == 0x20 )) ShutdownZeit.H = 0xff;
			 else {
				 i=atoi(Zeile);
				 if (( i < 0 ) || ( i > 59 )) {
					 MessageBox(hDlg,"Minuten zwischen 0 und 59 Uhr !!!","Multi-Decode",MB_ICONSTOP|MB_OK);
					 SetFocus(GetDlgItem(hDlg,IDC_EDIT2));
					 break;
					 
				 }
				 ShutdownZeit.M=i;
			 };
			 EndDialog(hDlg,TRUE);
			 
		 }
		 
		 if (LOWORD(wParam) == IDCANCEL ) {
			 EndDialog(hDlg,TRUE);
		 }
		 
		 break;
	 }
	 
	 return(FALSE);
 }
 
 
 BOOL APIENTRY Stop_SystemProc
	 (
	 HWND hDlg,
	 UINT message,
	 UINT wParam,
	 LONG lParam)
 {
	 HANDLE hToken; 
	 TOKEN_PRIVILEGES tkp; 
	 
	 static int Sekunden = 20 ;
	 
	 switch (message) {
	 case WM_INITDIALOG:
		 
		 if ( NT == TRUE ) {
			 if (!OpenProcessToken(GetCurrentProcess(), 
				 TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken)) {
				 SetDlgItemText(hDlg,IDC_SHUTDOWNTEXT,"Automatischer Shutdown fehlerhaft!\n\nProzessToken nicht bekommen");
				 break;
			 }
			 LookupPrivilegeValue(NULL, SE_SHUTDOWN_NAME, 
				 &tkp.Privileges[0].Luid); 
			 
			 tkp.PrivilegeCount = 1;  // one privilege to set    
			 tkp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED; 
			 
			 AdjustTokenPrivileges(hToken, FALSE, &tkp, 0, 
				 (PTOKEN_PRIVILEGES) NULL, 0); 
			 
			 if (GetLastError() != ERROR_SUCCESS) {
				 SetDlgItemText(hDlg,IDC_SHUTDOWNTEXT,"Automatischer Shutdown fehlerhaft!\n\nKeine Berechtigung");
				 break;
			 }
			 
		 };
		 
		 SetDlgItemText(hDlg,IDC_SHUTDOWNTEXT,ShutDownText);
         SetDlgItemInt(hDlg,IDC_TIMER,Sekunden,FALSE);
		 SetTimer(hDlg,1,1000,NULL);
         break;
		 
	 case WM_TIMER:
		 if ( wParam == 1 ) {
			 Sekunden--;
			 if ( Sekunden >= 0 ) SetDlgItemInt(hDlg,IDC_TIMER,Sekunden,FALSE);
			 else SetDlgItemInt(hDlg,IDC_TIMER,0,FALSE);
			 if ( Sekunden < 0 ) {
				 SetDlgItemText(hDlg,IDC_SHUTDOWNTEXT,"Automatischer Shutdown läuft!");
				 System_Shutdown=TRUE;
				 SendMessage(hWnd,WM_DESTROY,0,0);
				 
			 }
			 if ( Sekunden < 0 ) {
				 Sleep(2000);
				 EndDialog(hDlg,TRUE);
			 };
		 };
		 break;
		 
	 case WM_COMMAND:
		 
		 if (LOWORD(wParam) == IDCANCEL ) {
			 KillTimer(hDlg,1);
			 SetDlgItemText(hDlg,IDC_SHUTDOWNTEXT,"Automatischer Shutdown abgebrochen!");
			 Sleep(2000);
			 EndDialog(hDlg,TRUE);
			 break;	
		 }
		 if (LOWORD(wParam) == IDOK ) {
			 SetDlgItemText(hDlg,IDC_SHUTDOWNTEXT,"Automatischer Shutdown jetzt!");
			 Sekunden=0;
			 break;
		 };
	 };         
	 return(FALSE);
 }
 
 
 void Write_Log(LPSTR Text)
 {
	 if ( LogFile==NULL ) return;
	 fprintf(LogFile,"%s\n",Text);
 };
 
 
 BOOL Tab_Scan(char *FileName,char *Key, char *zeile)
 {
	 
	 FILE* fdptr;
	 char line[512];
	 
	 zeile[0]=0x00;
	 fdptr=fopen(FileName,"r");
	 if ( fdptr == NULL ) return(FALSE);
	 
     while (fgets(line,512,fdptr)) {
		 if ( line[0] != ';' ) {
             if ( strstr(line,Key) == &line[0] )
			 {
				 i=strlen(line)-1;
				 if ( line[i] == 10 ) line[i]=0x00;
				 
				 strcpy(zeile,line);
				 fclose(fdptr);
				 return(TRUE);
			 };
			 
		 };
		 
	 }
	 
	 fclose(fdptr);
	 return(FALSE);
 }
 
 
 BOOL Is_Sat_Karte(void) 
 {
	 if (( VES_TYPE == 1 ) || ( VES_TYPE == 2 ) || ( VES_TYPE == 3 )) return(TRUE);
	 return(FALSE);
 };
 
 BOOL Is_Kabel_Karte(void) 
 {
	 if ( VES_TYPE == 0 ) return(TRUE);
	 return(FALSE);
 };
 
BOOL ExternCommandDispatch(HWND hWnd,UINT message,UINT wParam,LONG lParam)
{
	 char Zeile[128];
	 
	 unsigned int Irq_Proc_Id;
	 
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
		 char Zeile[128];
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
	 
	 
	 struct TProgrammNummer TPNr;
	 struct TTransponder TPtp;
	 struct TProgramm TPtg;
	 struct TTPCatio TPc;
	 struct TOSD_DRAW osd_draw;
	 struct TOSD_SETFONT osd_font;
	 struct TOSD_SETEXT osd_text;
	 struct TSTART_FILTER start_filter;
	 struct TDVB_COMMAND dvb_com;
	 
	 
	 
	 if ( Current_Mode == 0 ) return(FALSE);
	 if ( wParam == MDAPI_GET_PROGRAMM_NUMMER ) {
		 sprintf(Zeile,"MDAPI_CALL : MDAPI_GET_PROGRAMM_NUMMER ");
		 Write_Log(Zeile);
		 TPNr.VirtNummer=Make_ProgNr(CurrentProgramm)+1;
		 TPNr.RealNummer=CurrentProgramm;
		 if ( lParam != 0 )
			 memcpy((struct TProgrammNummer *)lParam,&TPNr.RealNummer,8);
		 return(TRUE);
	 } else if ( wParam == MDAPI_GET_VERSION ) {
		 sprintf(Zeile,"MDAPI_CALL : MDAPI_GET_VERSION");
		 Write_Log(Zeile);
		 if ( lParam != 0 )
			 memcpy((unsigned char *)lParam,&MD_API_Version[0],32);
		 return(TRUE);
	 } else if ( wParam == MDAPI_SET_PROGRAMM_NUMMER ) {
		 sprintf(Zeile,"MDAPI_CALL : MDAPI_SET_PROGRAMM_NUMMER %d",lParam);
		 Write_Log(Zeile);
		 sprintf(ChannelString,"%d",lParam);
		 SetTimer(hWnd,99,1000,NULL);
		 return(TRUE);
	 } else if ( wParam == MDAPI_GET_TRANSPONDER ) {
		 sprintf(Zeile,"MDAPI_CALL : MDAPI_GET_TRANSPONDER");
		 Write_Log(Zeile);
		 memset(&TPtp,0x00,sizeof(struct TTransponder));
		 TPtp.fec=Programm[CurrentProgramm].fec;
		 TPtp.diseqc=Programm[CurrentProgramm].diseqc;
		 TPtp.freq=Programm[CurrentProgramm].freq;
		 TPtp.qam=Programm[CurrentProgramm].qam;
		 TPtp.srate=Programm[CurrentProgramm].srate;
		 TPtp.volt=Programm[CurrentProgramm].volt;
		 TPtp.sync=dvb_front.sync;
		 TPtp.ts_id=Programm[CurrentProgramm].tp_id;
		 if ( lParam != 0 ) memcpy((struct TTransponder *)lParam,&TPtp,sizeof(struct TTransponder));
		 return(TRUE);
	 } else if ( wParam == MDAPI_SET_TRANSPONDER ) {
		 sprintf(Zeile,"MDAPI_CALL : MDAPI_SET_TRANSPONDER");
		 Write_Log(Zeile);
		 if ( lParam != 0 ) memcpy(&TPtp,(struct TTransponder *)lParam,sizeof(struct TTransponder));
		 Set_Transponder(&TPtp);
		 return(TRUE);
	 } else if ( wParam == MDAPI_SET_PROGRAMM ) {
		 sprintf(Zeile,"MDAPI_CALL : MDAPI_GET_PROGRAMM");
		 Write_Log(Zeile);
		 if ( lParam != 0 ) memcpy(&Programm[CurrentProgramm],(struct TProgramm *)lParam,sizeof(struct TProgramm));
		 return(TRUE);
	 } else if ( wParam == MDAPI_GET_PROGRAMM ) {
		 sprintf(Zeile,"MDAPI_CALL : MDAPI_SET_PROGRAMM");
		 Write_Log(Zeile);
		 if ( lParam != 0 ) memcpy((struct TProgramm *)lParam,&Programm[CurrentProgramm],sizeof(struct TProgramm));
		 return(TRUE);
	 } else if ( wParam == MDAPI_RESCAN_PROGRAMM ) {
		 if ( lParam == 0 ) return(FALSE);
		 memcpy(&TPtg,(struct TProgramm *)lParam,sizeof(struct TProgramm));
		 sprintf(Zeile,"MDAPI_CALL : MDAPI_RESCAN_PROGRAMM");
		 Write_Log(Zeile);
		 Load_All_Parameters(&TPtg);
		 memcpy((struct TProgramm *)lParam,&TPtg,sizeof(struct TProgramm));
		 return(TRUE);
	 } else if ( wParam == MDAPI_SAVE_PROGRAMM ) {
		 sprintf(Zeile,"MDAPI_CALL : MDAPI_SAVE_PROGRAMM");
		 Write_Log(Zeile);
		 Write_Programm_List();
		 return(TRUE);
	 } else if ( wParam == MDAPI_SCAN_CURRENT_TP ) {
		 if ( lParam == 0 ) return(FALSE);
		 sprintf(Zeile,"MDAPI_CALL : MDAPI_SCAN_CURRENT_TP");
		 Write_Log(Zeile);
		 Scan_TP( NULL );
		 memcpy((struct TProgramm *)lParam,&ProgrammNeu[0],sizeof(struct TProgramm)*NEUSIZE);
		 return(TRUE);
	 } else if ( wParam == MDAPI_SCAN_CURRENT_CAT ) {
		 if ( lParam == 0 ) return(FALSE);
		 sprintf(Zeile,"MDAPI_CALL : MDAPI_SCAN_CURRENT_CAT");
		 Write_Log(Zeile);
		 Get_CAT();
		 memcpy(	TPc.TPCat,TPCat,sizeof( struct TTPCat)*32);	
		 TPc.TPCatAnzahl = TPCatAnzahl;
		 memcpy((struct TTPCatio *)lParam,&TPc,sizeof(struct TTPCatio));
		 return(TRUE);
	 } else if ( wParam == MDAPI_START_OSD ) {
		 if ( lParam == 0 ) return(FALSE);
		 sprintf(Zeile,"MDAPI_CALL : MDAPI_START_OSD");
		 Write_Log(Zeile);
		 memcpy(&DLL_OSD_Call,(struct TOSD_START *)lParam,sizeof(struct TOSD_START));
		 
		 if ( DLL_OSD_Created == NULL ) DLL_OSD_Created=CreateEvent(NULL,FALSE,FALSE,NULL);
		 Create_Osd_Thread((LPTHREAD_START_ROUTINE)Run_OSD_DLL_Call);
		 WaitForSingleObject(DLL_OSD_Created, 4000 );
		 CloseHandle(DLL_OSD_Created);
		 DLL_OSD_Created=NULL;
		 
		 return(TRUE);
	 } else if ( wParam == MDAPI_OSD_DRAWBLOCK ) {
		 if ( lParam == 0 ) return(FALSE);
		 sprintf(Zeile,"MDAPI_CALL : MDAPI_OSD_DRAWBLOCK");
		 Write_Log(Zeile);
		 memcpy(&osd_draw,(struct TOSD_DRAW *)lParam,sizeof(struct TOSD_DRAW));
		 DrawBlock(1, osd_draw.x, osd_draw.y,osd_draw.Sizex, osd_draw.Sizey,osd_draw.Color);
		 return(TRUE);
	 }
	 else
		 if ( wParam == MDAPI_OSD_SETFONT )
		 {
			 if ( lParam == 0 ) return(FALSE);
			 sprintf(Zeile,"MDAPI_CALL : MDAPI_OSD_SETFONT");
			 Write_Log(Zeile);
			 memcpy(&osd_font,(struct TOSD_SETFONT *)lParam,sizeof(struct TOSD_SETFONT));
			 SetFont(1, osd_font.Typ, osd_font.Fg_Color, osd_font.Bg_Color);
			 return(TRUE);
		 }
		 else
			 if ( wParam == MDAPI_OSD_TEXT )
			 {
				 if ( lParam == 0 ) return(FALSE);
				 sprintf(Zeile,"MDAPI_CALL : MDAPI_OSD_TEXT");
				 Write_Log(Zeile);
				 memcpy(&osd_text,(struct TOSD_SETEXT *)lParam,sizeof(struct TOSD_SETEXT));
				 WriteText( 1, osd_text.x,osd_text.y,osd_text.Zeile);
				 return(TRUE);
			 }
			 else
				 if ( wParam == MDAPI_SEND_OSD_KEY )
				 {
					 sprintf(Zeile,"MDAPI_CALL : MDAPI_SEND_OSD_KEY");
					 Write_Log(Zeile);
					 OSD_Key_Value=(unsigned char)lParam;
					 if ( OSD_KEY_EVENT != NULL ) SetEvent(OSD_KEY_EVENT);
					 return(TRUE);
				 }
				 else
					 if ( wParam == MDAPI_STOP_OSD )
					 {
						 sprintf(Zeile,"MDAPI_CALL : MDAPI_STOP_OSD");
						 Write_Log(Zeile);
						 OSD_Key_Value=0;
						 if ( OSD_KEY_EVENT != NULL ) SetEvent(OSD_KEY_EVENT);
						 return(TRUE);
					 }
					 else
						 if ( wParam == MDAPI_START_FILTER )
						 {
		 if ( lParam == 0 ) return(FALSE);
		 memcpy(&start_filter,(struct TSTART_FILTER *)lParam,sizeof(struct TSTART_FILTER));
		 sprintf(Zeile,"MDAPI_CALL : MDAPI_START_FILTER");
		 Write_Log(Zeile);
		 Irq_Proc_Id=256+(start_filter.DLL_ID<<16)+start_filter.Filter_ID;
		 Ext_Dll[start_filter.DLL_ID].Extern_Stream_Function[start_filter.Filter_ID]=start_filter.Irq_Call_Adresse;
		 start_filter.Running_ID=StartLogFilter(start_filter.Pid,Irq_Proc_Id,start_filter.Name);
		 memcpy((struct TSTART_FILTER *)lParam,&start_filter,sizeof(struct TSTART_FILTER));
		 return(TRUE);
	 } else if ( wParam == MDAPI_STOP_FILTER ) {
		 sprintf(Zeile,"MDAPI_CALL : MDAPI_STOP_FILTER");
		 Write_Log(Zeile);
		 Mapi_DelBitFilter((int)lParam);
		 return(TRUE);
	 } else if ( wParam == MDAPI_DVB_COMMAND ) {
		 if ( lParam == 0 ) return(FALSE);
		 memcpy(&dvb_com,(struct TDVB_COMMAND *)lParam,sizeof(struct TDVB_COMMAND));
		 sprintf(Zeile,"MDAPI_CALL : MDAPI_DVB_COMMAND");
		 Write_Log(Zeile);
		 SAA7146_OutCommand(&dvb_com.Cmd_Buffer[0],dvb_com.Cmd_laenge);
		 
		 return(TRUE);
	 };
	 
	 return(FALSE);
};


void Unload_External_Dll(int Nummer)
{
	unsigned char Zeile[512];
	
	if ( Ext_Dll[Nummer].Externe_DLL != NULL ) {
		if ( Ext_Dll[Nummer].Extern_Exit != NULL ) {
			(Ext_Dll[Nummer].Extern_Exit)();
			FreeLibrary(Ext_Dll[Nummer].Externe_DLL);
			Ext_Dll[Nummer].Externe_DLL=NULL;
			sprintf(Zeile,"MDAPI DLL %s entladen",Ext_Dll[Nummer].Name);
			Write_Log(Zeile);
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
		};
	}; 
};


void Send_External_DLL_Menu_Cmd( unsigned int MessageId)
{
	int i;
	
	for ( i=0; i < 5; i++ ) {
		if ( Ext_Dll[i].Extern_Menu_Cmd != NULL ) (Ext_Dll[i].Extern_Menu_Cmd)(MessageId);
	};
};     



void Load_External_Dll(void)
{
	unsigned char Zeile[512];
    unsigned char CurrentDir[512];
	struct _finddata_t c_file;
    long hDllBase;
	
	
	GetCurrentDirectory(255,CurrentDir);
	
	sprintf(Zeile,"%s\\*.dll",CurrentDir);
    hDllBase = _findfirst( Zeile, &c_file );
    if ( hDllBase == -1L ) return;
	
	
	while ( External_Dll_Count < 5 ) {
		sprintf(Ext_Dll[External_Dll_Count].Name,"%s\\%s",CurrentDir,c_file.name);
		
		sprintf(Zeile,"MDAPI Scanne DLL %s",Ext_Dll[External_Dll_Count].Name);
		Write_Log(Zeile);
		
		Ext_Dll[External_Dll_Count].Externe_DLL=LoadLibrary(Ext_Dll[External_Dll_Count].Name);
		if ( Ext_Dll[External_Dll_Count].Externe_DLL != NULL ) {
			Ext_Dll[External_Dll_Count].Extern_Send_Dll_ID_Name=(Extern_Send_Dll_ID_Name_DLL)GetProcAddress(Ext_Dll[External_Dll_Count].Externe_DLL,"On_Send_Dll_ID_Name");
			if ( Ext_Dll[External_Dll_Count].Extern_Send_Dll_ID_Name != 0 ) {
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
				sprintf(Zeile,"MDAPI DLL %s geladen",Ext_Dll[External_Dll_Count].Name);
				Write_Log(Zeile);
				sprintf(Zeile,"  Function On_Send_Dll_ID_Name       = 0x%08x",Ext_Dll[External_Dll_Count].Extern_Send_Dll_ID_Name);
				Write_Log(Zeile);
				sprintf(Zeile,"  Menu                               = 0x%08x",Ext_Dll[External_Dll_Count].Extern_Menu);
				Write_Log(Zeile);
				sprintf(Zeile,"  Function On_Start                  = 0x%08x",Ext_Dll[External_Dll_Count].Extern_Init);
				Write_Log(Zeile);
				sprintf(Zeile,"  Function On_Exit                   = 0x%08x",Ext_Dll[External_Dll_Count].Extern_Exit);
				Write_Log(Zeile);
				sprintf(Zeile,"  Function On_Hot_Key                = 0x%08x",Ext_Dll[External_Dll_Count].Extern_Hot_Key);
				Write_Log(Zeile);
				sprintf(Zeile,"  Function On_Osd_Key                = 0x%08x",Ext_Dll[External_Dll_Count].Extern_OSD_Key);
				Write_Log(Zeile);
				sprintf(Zeile,"  Function On_Channel_Change         = 0x%08x",Ext_Dll[External_Dll_Count].Extern_Channel_Change);
				Write_Log(Zeile);
				sprintf(Zeile,"  Function On_Menu_Cmd               = 0x%08x",Ext_Dll[External_Dll_Count].Extern_Menu_Cmd);
				Write_Log(Zeile);
				sprintf(Zeile,"  Function On_Filter_Close           = 0x%08x",Ext_Dll[External_Dll_Count].Extern_Filter_Close);
				Write_Log(Zeile);
				sprintf(Zeile,"  Function On_Rec_Play               = 0x%08x",Ext_Dll[External_Dll_Count].Extern_RecPlay);
				Write_Log(Zeile);
				External_Dll_Count++;     
			} else {
				// Dll Ohne Funktion Extern_Send_Dll_ID_Name
				sprintf(Zeile,"DLL %s hat keine Funktion On_Send_Dll_ID_Name",Ext_Dll[External_Dll_Count].Name);
				Write_Log(Zeile);
				FreeLibrary(Ext_Dll[External_Dll_Count].Externe_DLL);
				Ext_Dll[External_Dll_Count].Externe_DLL=NULL;
			}
		};
		
		if ( _findnext( hDllBase, &c_file ) != 0 ) {
			_findclose( hDllBase );
			return;
		};
		
		
		
	};
	
	return;
};



void Load_All_Parameters(struct TProgramm *ReloadProg)
{
	int i,j;
	
	for ( i=0; i < NEUSIZE ; i++ ) {
		memset(&ProgrammNeu[i].Name[0],0x00,sizeof( struct TProgramm));
		ProgrammNeu[i].Video_pid=0x1fff;        
		ProgrammNeu[i].Audio_pid=0x1fff;
		ProgrammNeu[i].TeleText_pid=0x1fff;          /* Teletext PID */
		ProgrammNeu[i].CA_Anzahl=0;
		ProgrammNeu[i].CA_Land[0]=0;
		ProgrammNeu[i].PMT_pid=0x1fff;
		ProgrammNeu[i].PCR_pid=0x1fff;
		ProgrammNeu[i].ECM_PID=0x1fff;
		ProgrammNeu[i].AC3_pid=0x1fff;
		ProgrammNeu[i].SID_pid=0x1fff;
		ProgrammNeu[i].Merker=0x00;
		ProgrammNeu[i].Link_SID=0xffff;
		ProgrammNeu[i].Link_TP=0xffff;
		ProgrammNeu[i].Dynamisch=0x00;
	};
	
	if ( Get_SDT() == FALSE ) return;
	
	i=Get_New_Prog_by_SID(ReloadProg->SID_pid);
	if ( i < 0 ) return;
	
	ReloadProg->ServiceTyp=ProgrammNeu[i].ServiceTyp;
	ReloadProg->CA_ID=ProgrammNeu[i].CA_ID;
	
	memset(&ProgrammNeu[0].Name[0],0x00,sizeof( struct TProgramm));
	ProgrammNeu[0].Video_pid=0x1fff;        
	ProgrammNeu[0].Audio_pid=0x1fff;
    ProgrammNeu[0].TeleText_pid=0x1fff;          /* Teletext PID */
	ProgrammNeu[0].Merker=0x00;
	ProgrammNeu[0].Link_SID=0xffff;
	ProgrammNeu[0].Link_TP=0xffff;
	ProgrammNeu[0].Dynamisch=0x00;
	
    ProgrammNeu[0].PMT_pid=ReloadProg->PMT_pid;
	if ( Get_PMT(0,ProgrammNeu[0].PMT_pid) == FALSE ) return;
    ReloadProg->Filteranzahl=ProgrammNeu[0].Filteranzahl;
	
	for ( i = 0 ; i < ProgrammNeu[0].Filteranzahl; i++ ) {
		ReloadProg->Filters[i].PID=ProgrammNeu[0].Filters[i].PID;
		ReloadProg->Filters[i].FilterId=ProgrammNeu[0].Filters[i].FilterId;
		memcpy(&ReloadProg->Filters[i].FilterName[0],&ProgrammNeu[0].Filters[i].FilterName[0],5);
	}
	
	ReloadProg->CA_Anzahl=ProgrammNeu[0].CA_Anzahl;
	for ( i = 0 ; i < ProgrammNeu[0].CA_Anzahl; i++ ) {
		ReloadProg->CA_System[i].CA_Typ=ProgrammNeu[0].CA_System[i].CA_Typ;
		ReloadProg->CA_System[i].ECM=ProgrammNeu[0].CA_System[i].ECM;
		ReloadProg->CA_System[i].EMM=ProgrammNeu[0].CA_System[i].EMM;
	}
	memcpy(ReloadProg->CA_Land,ProgrammNeu[0].CA_Land,sizeof(ProgrammNeu[0].CA_Land));
	
    if ( ReloadProg->CA_Anzahl > 0 ) {
		if ( Get_CAT() == TRUE ) {
			for ( i=0; i < TPCatAnzahl; i++ ) {
				for ( j =0; j < ReloadProg->CA_Anzahl; j++ ) {
					if ( TPCat[i].CA_ID == ReloadProg->CA_System[j].CA_Typ ) {
						ReloadProg->CA_System[j].EMM=TPCat[i].EMM;
					}
				}
			}
		}
	}
}


