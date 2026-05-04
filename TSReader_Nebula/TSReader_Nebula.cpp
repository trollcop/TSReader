#include "stdafx.h"
//#include <windows.h>
#include <commctrl.h>
#include <setupapi.h>
#include <initguid.h>
#include <stdio.h>
#include <Commdlg.h>

#include "..\sources.h"

#include "defs.h"
//#include <ks.h>
#include "device.h"
#include "PCIDriver.h"
#include "tuner.h"
#include "USBDriver.h"
#include "sdk.h"
#include "record.h"

#define SYNCWORD 0x47
#define PACKET_LEN_NO_RS 188
#define PACKET_LEN_RS 204

PSOURCESTRUCT ss;
char * szCmdLinePtr;
BOOL fNeedTuneDialog = TRUE;
BOOL fPipeThreadTerminated = FALSE;
int nFrequency;
int fSpectrumInversion;
int nBandwidth;
int nPipeBytes;
CRITICAL_SECTION csPipeBytes;
HANDLE hPipeRead;

//HANDLE hAudio;                     // Handle to the Audio minidriver
//HANDLE hVideo;                     // Handle to the Video minidriver

char Save_File[] = {"none.ts"};
bool Recording = true;
static __int64      Reset_Time              = 0;                    // Base point from which times are calculated
static LARGE_INTEGER    Frequency;                                  // Performance counter frequency

int Variant;                    // Which variant of the hardware is installed
int                 Base_Variant;                                   // Which variant of the hardware is installed
short COFDM_Type              = MT_352;               // The installed COFDM chip

char gszSourceName[] = {"Nebula DigiTV DVB-T"};
char szLastTune[128] = {"n/a"};
char szLastSignalReport[128] = {"n/a"};

extern "C" {
	BOOL __cdecl SourceHelper_ValidateSourceContainer(PSOURCESTRUCT pss);
}

void SetupLastTune()
{
	sprintf(szLastTune, "%.3f MHz", ss->nFrequency / 1000.0);
}

BOOL TSReader_Tune()
{
	int nSNR;
	int i;
	float fFrequency;
	int nBandwidth = 8;

	OutputDebugString("Nebula: Tune\n");
	SetupLastTune();

	switch(ss->nBandwidth)
	{
	case 0:		// 6MHz
		nBandwidth = 6;
		OutputDebugString("Nebula: Bandwidth = 6 MHz (not supported)\n");
		MessageBox(ss->hWndTSReader, "This hardware interface doesn't support this bandwidth", gszSourceName, MB_ICONSTOP);
		return FALSE;
	case 1:		// 7MHz
		OutputDebugString("Nebula: set_tuner_bandwidth(7)\n");
		nBandwidth = 7;
		break;
	case 2:
		OutputDebugString("Nebula: set_tuner_bandwidth(8)\n");
		nBandwidth = 8;
		break;
	}
	fFrequency = (float)ss->nFrequency / 1000.0f;
	{
		char szDebug[128];
		sprintf(szDebug, "Nebula: set_tuner_frequency(fFrequency = %.3f)\n", fFrequency);
		OutputDebugString(szDebug);
	}

	for (i = 0; i < 3; i++)
	{
		set_tuner_frequency(fFrequency, nBandwidth);

		OutputDebugString("Nebula: post set_tuner_frequency snooze before SNR\n");
		Sleep(500);

		nSNR  = get_snr();
		{
			char szDebug[128];
			wsprintf(szDebug, "Nebula: Post tune SNR = %d\n", nSNR);
			OutputDebugString(szDebug);
		}
		if (nSNR)
			break;	// we're locked
	}
	if (nSNR)
	{
		OutputDebugString("Nebula: tune function returning TRUE\n");
		return TRUE;
	}

	if (ss->fQuietMode == FALSE)
	{
		MessageBox(ss->hWndTSReader, "Failed to lock signal", gszSourceName, MB_ICONWARNING);
	}

	OutputDebugString("Nebula: tune function returning FALSE\n");
	return FALSE;
}

BOOL GetTunerStatus(BOOL * fLock, int * nSNR)
{
	*nSNR = get_snr();
	if (*nSNR == 0)
		*fLock = TRUE;
	else
		*fLock = FALSE;
	return TRUE;
}

/*int ReadFromPipe(BYTE * pBuffer, int nLength)
{
	int nRequestedLength = nLength;
	DWORD dwRead;

	while (nLength)
	{
		ReadFile(hPipeRead, pBuffer, nLength, &dwRead, NULL);
		if (dwRead == 0)
			return 0;
		EnterCriticalSection(&csPipeBytes);
		nPipeBytes -= dwRead;
		LeaveCriticalSection(&csPipeBytes);
		pBuffer += dwRead;
		nLength -= dwRead;
	}

	return nRequestedLength;
}

DWORD WINAPI ReadPipeThread(LPVOID lpv)
{
	int nTSBufferIndex = 0;
	int nSync = 0;
	int nRead;
	BOOL fAbort = FALSE;
	BOOL fFirstBuffer = TRUE;
	BYTE syncbuffer[PACKET_LEN_RS * 1024];

	OutputDebugString("Nebula: +ReadPipeThread\n");

	fPipeThreadTerminated = FALSE;
	do
	{
		if (nSync == 0)
		{
			// Not in sync
			do
			{
				DWORD dwRead;
				int nComparePipeBytes;

				ReadFile(hPipeRead, syncbuffer, 1, &dwRead, NULL);
				if (dwRead == 0)
				{
					fAbort = TRUE;
					break;
				}
				EnterCriticalSection(&csPipeBytes);
				nPipeBytes -= dwRead;
				nComparePipeBytes = nPipeBytes;
				LeaveCriticalSection(&csPipeBytes);

				if (syncbuffer[0] == SYNCWORD)
				{
					// Maybe got a sync - read 188 bytes to see if we get another one
					nRead = ReadFromPipe(&syncbuffer[1], PACKET_LEN_NO_RS);
					if (nRead == 0)
					{
						fAbort = TRUE;
						break;
					}
					if (nRead != PACKET_LEN_NO_RS)
						OutputDebugString("Nebula: read error 1\n");
					if (syncbuffer[PACKET_LEN_NO_RS] == SYNCWORD) // synced
					{
						OutputDebugString("Nebula: Sync with 188 bytes\n");
						nSync = PACKET_LEN_NO_RS;
						fFirstBuffer = TRUE;
						nRead = ReadFromPipe(syncbuffer, PACKET_LEN_NO_RS - 1);
						if (nRead == 0)
						{
							fAbort = TRUE;
							break;
						}
						if (nRead != PACKET_LEN_NO_RS - 1)
							OutputDebugString("Nebula: read error 2\n");
						break;
					}

					// No sync after 188 bytes, so read 15 more to see if we then get sync
					nRead = ReadFromPipe(&syncbuffer[PACKET_LEN_NO_RS + 1], 16);
					if (nRead == 0)
					{
						fAbort = TRUE;
						break;
					}
					if (nRead != 16)
						OutputDebugString("Nebula: read error 3\n");

					if (syncbuffer[PACKET_LEN_RS] == SYNCWORD) // synced
					{
						OutputDebugString("Nebula: Sync with 204 bytes\n");
						nSync = PACKET_LEN_RS;
						fFirstBuffer = TRUE;
						nRead = ReadFromPipe(syncbuffer, PACKET_LEN_RS - 1);
						if (nRead == 0)
						{
							fAbort = TRUE;
							break;
						}
						if (nRead != PACKET_LEN_RS - 1)
							OutputDebugString("Nebula: read error 4\n");
						break;
					}
				}	
				if (nComparePipeBytes > 1024 * PACKET_LEN_NO_RS)
				{
					// Still haven't synced and there's 1024 packets
					// at least in the pipe. Empty these out before
					// we continue looking
					nComparePipeBytes = (1024 * PACKET_LEN_NO_RS) - (PACKET_LEN_NO_RS / 2);
					ReadFromPipe(syncbuffer, nComparePipeBytes);
					Sleep(10);
				}
			} while (TRUE);
		}
		else
		{
			while ((TS_BUFFER_SIZE - ss->tsb[nTSBufferIndex].nSize) >= PACKET_LEN_NO_RS)
			{
				nRead = ReadFromPipe(ss->tsb[nTSBufferIndex].pData + ss->tsb[nTSBufferIndex].nSize, PACKET_LEN_NO_RS);
				if (nRead == 0)
				{
					fAbort = TRUE;
					break;
				}
				if (nRead != PACKET_LEN_NO_RS)
					OutputDebugString("Nebula: read error 5\n");
				if (*(ss->tsb[nTSBufferIndex].pData + ss->tsb[nTSBufferIndex].nSize) != SYNCWORD)
				{
					nSync = 0;
					OutputDebugString("Nebula: Lost Sync!\n");
					break;
				}
				ss->tsb[nTSBufferIndex].nSize += nRead;
				if (nSync == PACKET_LEN_RS)
				{
					nRead = ReadFromPipe(syncbuffer, 16);
					if (nRead == 0)
					{
						fAbort = TRUE;
						break;
					}
					if (nRead != 16)
						OutputDebugString("Nebula: read error 6\n");
				}
			}		 
			
			if (fAbort == FALSE)
			{
				if (fFirstBuffer == TRUE)
				{
					ss->tsb[nTSBufferIndex].nSize = 0;
					fFirstBuffer = FALSE;
				}
				else
				{
					EnterCriticalSection(&ss->csPIDCounter);
					ss->nLastSecondByteCounter += ss->tsb[nTSBufferIndex].nSize;
					LeaveCriticalSection(&ss->csPIDCounter);
					nTSBufferIndex++;
					if (nTSBufferIndex == MAX_TS_BUFFERS)
						nTSBufferIndex = 0;
					EnterCriticalSection(&ss->csTSBuffersInUse);
					ss->nTSBuffersInUse++;
					LeaveCriticalSection(&ss->csTSBuffersInUse);
					ss->tsb[nTSBufferIndex].nSize = 0;
				}
			}
		}
	} while (fAbort == FALSE);

	CloseHandle(hPipeRead);
	OutputDebugString("Nebula: -ReadPipeThread\n");
	fPipeThreadTerminated = TRUE;
	return 0;
}*/

DWORD WINAPI ReadNebulaThread(LPVOID lpv)
{
	int nReadPtr = 0;
	int nTunerStatusTimer = 250;
	//DWORD dwThreadID;
	//HANDLE hPipeReadThread;
	//HANDLE hPipeWrite;

	BYTE * pBuffer;
	BYTE * pData;
	fusion_register Register;

	//HANDLE hDebug = CreateFile("c:\\tsreader.ts", GENERIC_WRITE, 0, (LPSECURITY_ATTRIBUTES) NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, (HANDLE) NULL);

	OutputDebugString("Nebula: +ReadNebulaThread\n");

	ss->fReadThreadTerminated = FALSE;
	ss->fTerminateReadThread = FALSE;

	SourceHelper_StartSyncThread(ss, FALSE);

	switch (Variant)
	{
	case PCI_VARIANT:
		pBuffer = (byte*) LocalAlloc (LPTR, sizeof (pci_data));
		pData   = ((pci_data*) pBuffer) -> Data; 
		break;
	case USB_VARIANT:
		pBuffer = (byte*) LocalAlloc (LPTR, sizeof (usb_data)); 
		pData   = ((usb_data*) pBuffer) -> Data; 
		break;
	}
	write_property (AUDIO, AUDIO_SYNC_FIFO, &Register);

	OutputDebugString("Nebula: enter main read loop\n");
	while (!ss->fTerminateReadThread)
	{
		int nBytesRead;
		
		switch (Variant)
		{
		case PCI_VARIANT:
			nBytesRead = get_data_pci (pBuffer);
			break;
		case USB_VARIANT:
			nBytesRead = get_data_usb (pBuffer);
			break;
		}
		if (nBytesRead)
		{
			SourceHelper_SyncData(pData, nBytesRead);
		}
		else
		{
			Sleep(10);
		}
		if (nTunerStatusTimer++ > 250)
		{
			BOOL fLock, nSNR;
			float fBER = get_ber();

			nTunerStatusTimer = 0;
			GetTunerStatus(&fLock, &nSNR);
			sprintf(szLastSignalReport, "BER %0.1E", fBER);
		}
	}
	OutputDebugString("Nebula: left main read loop\n");
	if (pBuffer)
		LocalFree(pBuffer);

	SourceHelper_StopSyncThread();

	ss->fReadThreadTerminated = TRUE;
	ss->fTerminateReadThread = FALSE;
	EnterCriticalSection(&ss->csTSBuffersInUse);
	ss->nTSBuffersInUse = -1000;
	LeaveCriticalSection(&ss->csTSBuffersInUse);
	CloseHandle(ss->hReadDataThread);
	OutputDebugString("Nebula: -ReadNebulaThread\n");
	return 0;
}

BOOL TSReader_Start()
{
	DWORD dwThreadID;

	OutputDebugString("Nebula: enter Start()\n");
	nPipeBytes = 0;
	InitializeCriticalSection(&csPipeBytes);

	ss->hReadDataThread = CreateThread(NULL, 0, ReadNebulaThread, (LPVOID)0, CREATE_SUSPENDED, &dwThreadID);
	SourceHelper_SetWorkerThreadPriorities(FALSE);
	ResumeThread(ss->hReadDataThread);

	OutputDebugString("Nebula: leave Start()\n");
	return TRUE;
}

BOOL TSReader_Stop()
{
	OutputDebugString("Nebula: enter Stop()\n");

	ss->fTerminateReadThread = TRUE;
	while (ss->fReadThreadTerminated == FALSE)
		Sleep(50);

	DeleteCriticalSection(&csPipeBytes);

	OutputDebugString("Nebula: 	leave Stop()\n");
	return TRUE;
}

BOOL TSReader_Init(PSOURCESTRUCT pss)
{
short               Num_Devices             = 0;                    // The number of connected DigiTV devices
short               Device_No               = 0;                    // The DigiTV device number of this instance

int              Result_PCI;
int              Result_USB;

short             Dev_Pos     [MAX_DEVICES];

	if (SourceHelper_ValidateSourceContainer(pss) == FALSE)
		return FALSE;

	OutputDebugString("Nebula: Init\n");
	ss = pss;
 
	Num_Devices = num_usb_devs (Dev_Pos);

	Result_USB = open_usb_driver (Dev_Pos [Device_No]);
	Result_PCI = open_pci_driver ();

	if (Result_USB)
		Base_Variant = USB_VARIANT;
	else if (Result_PCI)
		Base_Variant = PCI_VARIANT;
	else
		Base_Variant = NO_HARDWARE;

	if (Result_USB)
		Variant = USB_VARIANT;
	else if (Result_PCI)
		Variant = PCI_VARIANT;
	else
		Variant = NO_HARDWARE;

	switch (Variant)
	{
	case PCI_VARIANT:
		init_pci_device();
		break;
	case USB_VARIANT:
		init_usb_device();
		break;
	}

	if (read_i2c (NXT_CHIP_ID) == 0x0b)
		COFDM_Type = NXT_6000;
	else
		COFDM_Type = MT_352;

	{
		char szTemp[256];
		char szDeviceType[16];
		char szDemodType[16];

		if (COFDM_Type == NXT_6000)
			lstrcpy(szDemodType, "NXT 6000");
		else
			lstrcpy(szDemodType, "MT 352");

		switch (Variant)
		{
		case PCI_VARIANT:
			lstrcpy(szDeviceType, "PCI");
			break;
		case USB_VARIANT:
			lstrcpy(szDeviceType, "USB");
			break;
		case NO_HARDWARE:
			lstrcpy(szDeviceType, "None");
			break;
		}

		wsprintf(szTemp, "Nebula: Device type %s Demod %s\n", szDeviceType, szDemodType);
		OutputDebugString(szTemp);
	}

	init_tuner (TRUE);
	return Variant != NO_HARDWARE;
}

BOOL TSReader_DeInit()
{
	OutputDebugString("Nebula: DeInit\n");

	close_pci_driver();
	return TRUE;
}

BOOL TSReader_TuneDialog(HWND hWnd)
{
	OutputDebugString("Nebula: TuneDialog\n");

	if (ss->fDontTune == TRUE)
	{
		fNeedTuneDialog = FALSE;
		OutputDebugString("TuneDialog() set fNeedTuneDialog = FALSE\n");
	}

	{
		char szTemp[100];
		wsprintf(szTemp, "fNeedTuneDialog = %d\n", fNeedTuneDialog);
		OutputDebugString(szTemp);
	}
	if (fNeedTuneDialog)
	{
		if (SourceHelper_DVBTTuneDialog(hWnd) == FALSE)
			return FALSE;
	}
	else
	{
		ss->nFrequency = nFrequency;
		ss->fSpectrumInversion = fSpectrumInversion;
		ss->nBandwidth = nBandwidth;

		if (ss->fQuietMode == FALSE)
		{
			fNeedTuneDialog = TRUE;
			OutputDebugString("TuneDialog() set fNeedTuneDialog = TRUE\n");
		}
	}

	return TRUE;
}

BOOL TSReader_PIDManagement(BOOL fAdd, int nPID, BOOL fTemporary)
{
	return TRUE;
}

BOOL TSReader_GetDescription(char * szDescription, char * szCommandLineParameters, BOOL * fCanBeStopped, int * nMaxPIDs, DWORD * dwCapabilities)
{
	if (szDescription != NULL)
		lstrcpy(szDescription, gszSourceName);
	if (szCommandLineParameters != NULL)
		lstrcpy(szCommandLineParameters, "freq inversion bandwidth");
	if (fCanBeStopped != NULL)
		*fCanBeStopped = FALSE;
	if (nMaxPIDs != NULL)
		*nMaxPIDs = 8192;
	if (dwCapabilities != NULL)
		*dwCapabilities = 0;

	return TRUE;
}

BOOL TSReader_ParseCommandLine(PSOURCESTRUCT ss, char * szCommandLine)
{
	fNeedTuneDialog = TRUE;
	if (lstrlen(szCommandLine))
	{
		int nConversionCount;

		nConversionCount = sscanf(szCommandLine,
								  "%d %d %d", 
								  &nFrequency,
								  &fSpectrumInversion,
								  &nBandwidth);
		if (nConversionCount < 3)
		{
			MessageBox(NULL,
					   "Usage for this source: freq inversion bandwidth\n"
					   "\n"
					   "freq = frequency to tune in KHz\n"
					   "inversion = inverted spectrum (0 or 1)\n"
					   "bandwidth = bandwidth of signal (0 = 6, 1 = 7, 2 = 8 MHz)",
					   gszSourceName,
					   MB_OK | MB_ICONSTOP);
			return FALSE;
		}
		fNeedTuneDialog = FALSE;
	}
	else
		fNeedTuneDialog = TRUE;

	return TRUE;
}

BOOL TSReader_IsPIDActive(int nPID)
{
	return TRUE;
}

BOOL TSReader_SetChannel(int nChannel)
{
	return TRUE;
}

BOOL TSReader_GetSignalString(char * szString)
{
	lstrcpy(szString, szLastSignalReport);
	return TRUE;
}

BOOL TSReader_GetTunerString(char * szString)
{
	lstrcpy(szString, szLastTune);
	return TRUE;
}

BOOL TSReader_SendDiSEqC(BYTE * bCommand, int nLength)
{
	return TRUE;
}

DWORD get_system_time (bool     Reset)
{
  __int64           Ticks;

  LARGE_INTEGER     Count;

  QueryPerformanceCounter   (&Count);

  Ticks = Count. QuadPart;

  if (Reset) Reset_Time = Ticks;

  return ((DWORD) (((Ticks - Reset_Time) * 1000) / Frequency. QuadPart));
} // get_system_time
