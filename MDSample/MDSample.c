// MDSample.c - sample plugin for the MultiDec API

#include <windows.h>
#include <time.h>
#include "MDSample.h"
#include "crc.h"
#include "resource.h"

// Global variables
HINSTANCE hTSReaderInstance;
HWND hTSReaderWnd;
BOOL fRunning;
int nMyDLLID;
int nFillPtr;
int nPointerKludge = 0;
BYTE bSIBuffer[1024];
char szDecodedPAT[1000] = {0};
char szThisFilterName[] = {"TSReader Sample"};
TSTART_FILTER Filter;

// On_Start
// This function gets called when TSReader loads the plugin
// Saves important stuff like TSReader's hWnd

void On_Start(HINSTANCE hInstance, HWND hWnd, BOOL bLogSet, int nDLLID, char * szHotKey)
{
	hTSReaderInstance = hInstance;
	hTSReaderWnd = hWnd;
	nMyDLLID = nDLLID;
	fRunning = FALSE;
	CRC_Init();
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
}


BOOL ParsePATPacket(BYTE * pSectionPointer, int nPacketLength)
{
	int nTableID, nSectionLength;
	int nSectionNumber, nLastSectionNumber;
	int nVersionNumber;
	int nTransportStreamID;

	nTableID = pSectionPointer[0];
	if (nTableID != 0)
		return FALSE;

	nSectionLength = ((pSectionPointer[1] << 8) + pSectionPointer[2]) & 0xfff;
	if ( (nSectionLength <= 0) || (nSectionLength > 1024) )
		return FALSE;

	if (CRC_Check(pSectionPointer, nSectionLength + 3) != TRUE)
	{
		// Let's see if this is a screwy ONN like feed with an extra
		// byte after the pointer
		if ( (pSectionPointer[0] == 0)
			&& (pSectionPointer[1] == 0) 
			&& ((pSectionPointer[2] & 0x80) == 0x80) 
			&& (nPointerKludge == 0) )
		{
			nPointerKludge = 1;
			return FALSE;
		}
		return FALSE;
	}

	nTransportStreamID = (pSectionPointer[3] << 8) + pSectionPointer[4];
	nVersionNumber = (pSectionPointer[5] >> 1) & 0x1f;
	nSectionNumber = pSectionPointer[6];
	nLastSectionNumber = pSectionPointer[7];

	wsprintf(szDecodedPAT, "Transport stream id: %d\nPAT Version number: %d\n\n", nTransportStreamID, nVersionNumber);
	nSectionLength -= 5;
	pSectionPointer += 8;
	do
	{
		int nProgramNumber, nPMTPID;
		char szTemp[100];

		nProgramNumber = (pSectionPointer[0] << 8) + pSectionPointer[1];
		nPMTPID = ((pSectionPointer[2] << 8) + pSectionPointer[3]) & 0x1fff;

		wsprintf(szTemp, "Program Number: %d PMTPID: 0x%04x\n", nProgramNumber, nPMTPID);
		lstrcat(szDecodedPAT, szTemp);

		pSectionPointer += 4;
		nSectionLength -= 4;
	} while (nSectionLength > 4);

	return (nSectionNumber == nLastSectionNumber);
}

// On_Filter_Receive
// Called by TSReader when a PID filter is active and data is present
// on the PID. TSReader returns up to 9 transport stream packets with the
// transport stream header missing just like MultiDec, so each packet
// is 184 bytes long
// This makes parsing sections a total pain because you've got no idea
// when a section starts since that flag is in the transport stream header.
// So TSReader has a special feature that if you send the PID to filter
// with the lower word's MSb set, TSReader sends the entire 188 byte packet.
// We use that in this demo to parse the PAT.

void On_Filter_Receive(int nMyFilter, int nLength, unsigned char * pBuffer)
{
	int nBufferOffset;

	// Remember TSReader is sending us 188 byte packets - this isn't normal
	// for an MD-API plugin
	for (nBufferOffset = 0; nBufferOffset < nLength; nBufferOffset += 188)
	{
		if ((pBuffer[nBufferOffset + 1] & 0x40) == 0x40)		// PES/PSI start?
		{
			int nPointer = pBuffer[nBufferOffset + 4] + nPointerKludge;
			if (nFillPtr > 0)
			{
				// Complete the rest of this section
				if (nPointer > 0)
				{
					memcpy(&bSIBuffer[nFillPtr], &pBuffer[nBufferOffset + 5], nPointer);
					nFillPtr += nPointer;
				}

				// Parse the section
				if (ParsePATPacket(bSIBuffer, nFillPtr) == TRUE)
				{
					// Send a message through to TSReader to simulate our menu ID to turn off the filter
					SendMessage(hTSReaderWnd, WM_COMMAND, ID_DECODEPAT, 0);
					MessageBox(hTSReaderWnd, szDecodedPAT, szThisFilterName, MB_ICONINFORMATION);
					return;
				}
				nFillPtr = 0;

			}
			// Add on any data from the next section
			memcpy(&bSIBuffer[nFillPtr], &pBuffer[5 + nBufferOffset + nPointer], 188 - 5 - nPointer);
			nFillPtr += 188 - 5 - nPointer;
		}
		else
		{
			// Not the start of a section, but if we've seen the start previously, add this data
			if (nFillPtr != 0)
			{
				memcpy(&bSIBuffer[nFillPtr], &pBuffer[4 + nBufferOffset], 188 - 4);
				nFillPtr += 188 - 4;
			}
		}
	}
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
	case ID_DECODEPAT:
		if (fRunning == FALSE)
		{
			fRunning = TRUE;

			ZeroMemory(&Filter.Name, sizeof(Filter.Name));
			strncpy(Filter.Name, "TSReader Sample PID 0", sizeof(Filter.Name));

			Filter.DLL_ID = nMyDLLID;
			Filter.Filter_ID = 0;	// filled in by TSReader
			Filter.Pid = 0;		// PID containing the PAT
			Filter.Pid |= 0x8000;	// 188 byte mode flag (TSReader only) -- see On_Filter_Receive for details
			nFillPtr = 0;
			Filter.Irq_Call_Adresse = (DWORD)On_Filter_Receive;			
			SendMessage(hTSReaderWnd, WM_USER, MDAPI_START_FILTER, (LPARAM)&Filter);			
		}
		else
		{
			SendMessage(hTSReaderWnd, WM_USER, MDAPI_STOP_FILTER, Filter.Running_ID);
			fRunning = FALSE;
		}
		break;
	}
}

