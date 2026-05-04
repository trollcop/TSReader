// MDSampleIP.c - sample IP/DVB plugin for TSReader.

#include <windows.h>
#include <time.h>
#include "resource.h"

// Global variables
HINSTANCE hTSReaderInstance;
HWND hTSReaderWnd;
BOOL fRunning;
int nMyDLLID;
char szThisFilterName[] = {"TSReader IP Sample"};

// Structures
typedef struct _tagMPEIPPACKET
{
	// MPEG-2 section
	int table_id, section_length;
	int section_syntax_indicator, private_indicator;

	// MPE header
	int current_next_indicator, section_number, last_section_number;
	int LLC_SNAP_flag, payload_scrambling_control, address_scrambling_control;
	BYTE MAC_Address[6];

	// IP Header
	int IPHeader_Version, IPHeader_IHL, IPHeader_TOS;
	int IPHeader_TotalLength, IPHeader_Identification, IPHeader_Flags;
	int IPHeader_FragmentOffset, IPHeader_TTL, IPHeader_Protocol;
	int IPHeader_HeaderChecksum, IPHeader_SourceAddress, IPHeader_DestinationAddress;
} MPEIPPACKET, *PMPEIPPACKET;

// Defines
#define TSREADER_MDAPI_SWITCH_IP_MODE		0x01030001

// On_Start
// This function gets called when TSReader loads the plugin
// Saves important stuff like TSReader's hWnd

void On_Start(HINSTANCE hInstance, HWND hWnd, BOOL bLogSet, int nDLLID, char * szHotKey)
{
	hTSReaderInstance = hInstance;
	hTSReaderWnd = hWnd;
	nMyDLLID = nDLLID;
	fRunning = FALSE;
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
}

// On_Filter_Receive
// Called by TSReader when a PID filter is active and data is present
// on the PID. TSReader returns up to 9 transport stream packets with the
// transport stream header missing just like MultiDec, so each packet
// is 184 bytes long
// This makes parsing sections a total pain because you've got no idea
// when a section starts since that flag is in the transport stream header.
// So TSReader has a special feature that if you send the PID to filter
// with the lower word's MSB set, TSReader sends the entire 188 byte packet.
// We use that in this demo to parse the PAT.

void On_Filter_Receive(int nMyFilter, int nLength, unsigned char * pBuffer)
{
}

// Called by TSReader as IP traffic is received when TSReader is IP/DVB mode.
// pData points to the start of the MPEG section containing the MPE packet
// mpe points to a struct containing the MPEG, MPE and IP headers pre-decoded
// (TSReader decodes this for every packet anyway so it makes sense to pass it
// along to plugins).
void On_IP_Data_Receive(int nPID, BYTE * pData, PMPEIPPACKET mpe)
{
	char szDebug[128];

	if (!fRunning)
		return;

	wsprintf(szDebug, "%04x %02x:%02x:%02x:%02x:%02x:%02x %03d.%03d.%03d.%03d->%03d.%03d.%03d.%03d\n",
			 nPID,     
			 mpe->MAC_Address[0], mpe->MAC_Address[1], mpe->MAC_Address[2], 
		     mpe->MAC_Address[3], mpe->MAC_Address[4], mpe->MAC_Address[5], 
			 (mpe->IPHeader_SourceAddress >> 24) & 0xff,
			 (mpe->IPHeader_SourceAddress >> 16) & 0xff,
			 (mpe->IPHeader_SourceAddress >> 8) & 0xff,
			 mpe->IPHeader_SourceAddress & 0xff,
			 (mpe->IPHeader_DestinationAddress >> 24) & 0xff,
			 (mpe->IPHeader_DestinationAddress >> 16) & 0xff,
			 (mpe->IPHeader_DestinationAddress >> 8) & 0xff,
			 mpe->IPHeader_DestinationAddress & 0xff);
	OutputDebugString(szDebug);
}

// On_Menu_Select
// TSReader calls this routine when a menu item from the EXTERN menu
// in this DLL is selected by the user. The menu message must have a range
// of 40000 through 41000 but really should start at 40500 to prevent
// interference with TSReader's menus
void On_Menu_Select(unsigned int nMenuID)
{
	switch(nMenuID)
	{
	case ID_DECODE_IP_TRAFFIC:
		if (fRunning == FALSE)
		{
			// Up to 16 PIDs - terminated with 1fff
			int nPIDList[] = {0x0400, 0x0401, 0x1000, 0x1fff};
			fRunning = TRUE;

			SendMessage(hTSReaderWnd, WM_USER, TSREADER_MDAPI_SWITCH_IP_MODE, (LPARAM)nPIDList);			
		}
		else
		{
			// Single entry with 0x1fff turns off IP/DVB mode
			int nPIDList = 0x1fff;
			SendMessage(hTSReaderWnd, WM_USER, TSREADER_MDAPI_SWITCH_IP_MODE, (LPARAM)&nPIDList);
			fRunning = FALSE;
		}
		break;
	}
}
