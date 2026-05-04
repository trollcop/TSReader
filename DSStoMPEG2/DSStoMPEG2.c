// MDSample.c - sample plugin for the MultiDec API

#include <windows.h>
//#include <time.h>

#define MDAPI_START_FILTER           0x01020020		
#define MDAPI_STOP_FILTER            0x01020021		

typedef struct  
{
	char FilterName[5];
	unsigned char FilterId;
	unsigned short PID;
} TPIDFilters;

typedef struct  
{
	unsigned short CA_Typ;
	unsigned short ECM;
	unsigned short EMM;
} TCA_System;

typedef struct 
{
	char				Name[30];
	char				Anbieter[30];
	char				Land[30];
    unsigned long		freq;
    unsigned char		Typ;
	unsigned char		volt;              
	unsigned char		afc;
	unsigned char		diseqc;            
	unsigned int		srate;         
	unsigned char		qam;               
	unsigned char		fec;   
	unsigned char		norm;
	unsigned short		tp_id;        
	unsigned short		Video_pid;        
	unsigned short		Audio_pid;
    unsigned short		TeleText_pid;          
	unsigned short		PMT_pid;
    unsigned short		PCR_pid;
	unsigned short		ECM_PID;
	unsigned short		SID_pid;
	unsigned short		AC3_pid;
	unsigned char		TVType; 
	unsigned char		ServiceTyp;
    unsigned char		CA_ID;
	unsigned short		Temp_Audio;
	unsigned short		Filteranzahl;
    TPIDFilters			Filters[12];
	unsigned short		CA_Anzahl;
    TCA_System			CA_System[6];
    char				CA_Land[5];
    unsigned char		Merker;
    unsigned short		Link_TP;
    unsigned short		Link_SID;
    unsigned char		Dynamisch;

    char Extern_Buffer[16];
} TPROGRAM;

typedef struct
{
	unsigned short	DLL_ID;
	unsigned short	Filter_ID;
	unsigned short	Pid;
	unsigned char	Name[32];
	DWORD			Irq_Call_Adresse;
	int				Running_ID;
} TSTART_FILTER;

// Global variables
HINSTANCE hTSReaderInstance;
HWND hTSReaderWnd;
int nMyDLLID;
char szThisFilterName[] = {"DSS to MPEG-2 converter"};
TSTART_FILTER Filter;
BYTE bContinuity[8192];

// On_Filter_Receive
// Notice how the receive function returns an int - this is ONLY used when
// the entire mux is hooked - return the actual size. Maximum is just under 128K.

BYTE bMPEG2Buffer[128 * 1024];

int On_Filter_Receive(int nMyFilter, int nLength, unsigned char * pBuffer)
{
	int nPackets = nLength / 131;
	int i;
	BYTE * pOutput = bMPEG2Buffer;
	BYTE * pInput = pBuffer;

	// Don't tread on non-DSS streams
	if (pBuffer[0] != 0x1d)
		return nLength;

	for (i = 0; i < nPackets; i++)
	{
		int nSCID = (pInput[1] << 8 | pInput[2]) & 0xfff;
		BYTE bDSSScrambling = (pInput[1] & 0x30);
		BYTE bScrambling;

		if (nSCID == 0)		// DSS null packet
			nSCID = 0x1fff;	// MPEG-2 null packet
		switch(bDSSScrambling)
		{
		case 0x20:			// no DSS scrambling
		case 0x30:			// not scrambled (with odd key!)
			bScrambling = 0x00;
			break;
		case 0x00:			// scrambled with even key
			bScrambling = 0x40;
			break;
		case 0x10:			// scrambled with odd key
			bScrambling = 0x80;
			break;
		}

		// This bit is reasonable
		pOutput[0] = 0x47;
		pOutput[1] = nSCID >> 8;
		pOutput[2] = nSCID & 0xff;
		pOutput[3] = bScrambling | 0x10 | bContinuity[nSCID];
		bContinuity[nSCID] = (bContinuity[nSCID] + 1) & 15;
		
		// This isn't!!
		memcpy(&pOutput[4], &pInput[4], 127);
		memset(&pOutput[4 + 127], 0xff, 184 - 127);
		
		// Onto the next packet
		pOutput += 188;
		pInput += 131;
	}

	// Replace the received buffer with our own
	memcpy(pBuffer, bMPEG2Buffer, nPackets * 188);
	return nPackets * 188;
}

// On_Start
// This function gets called when TSReader loads the plugin
// Saves important stuff like TSReader's hWnd

void On_Start(HINSTANCE hInstance, HWND hWnd, BOOL bLogSet, int nDLLID, char * szHotKey)
{
	hTSReaderInstance = hInstance;
	hTSReaderWnd = hWnd;
	nMyDLLID = nDLLID;

	memset(bContinuity, 0, sizeof(bContinuity));

	// Hook the mux
	ZeroMemory(&Filter.Name, sizeof(Filter.Name));
	strncpy(Filter.Name, "DSS to MPEG-2", sizeof(Filter.Name));
	Filter.DLL_ID = nMyDLLID;
	Filter.Filter_ID = 0;	// filled in by TSReader
	Filter.Pid = 0x9fff;	// entire stream please (TSReader Standard and Pro only)
	Filter.Irq_Call_Adresse = (DWORD)On_Filter_Receive;			
	PostMessage(hTSReaderWnd, WM_USER, MDAPI_START_FILTER, (LPARAM)&Filter);			
}

// On_Channel_Change
// This function gets called when the user changes channel in TSReader
// by clicking on a PMT entry

void On_Channel_Change(TPROGRAM CurrentProgram)
{
}

// On_Send_Dll_ID_Name
// Called by TSReader when loading the plugin to get
// it's name

void On_Send_Dll_ID_Name(char * szName) 
{
	lstrcpy(szName, szThisFilterName);
}

// On_Exit
// Called by TSReader when the plugin is being unloaded
void On_Exit(HINSTANCE hInstance, HWND hWnd, BOOL bLogSet)
{
	SendMessage(hTSReaderWnd, WM_USER, MDAPI_STOP_FILTER, Filter.Running_ID);
}

// On_Menu_Select
void On_Menu_Select(unsigned int nMenuID)
{
	// No menu for us
}
