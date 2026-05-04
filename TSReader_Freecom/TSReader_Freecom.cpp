#include <windows.h>
#include <commctrl.h>
#include <setupapi.h>
#include <initguid.h>
#include <stdio.h>
#include <math.h>

#include "CyAPI.h"
#include "..\sources.h"

PSOURCESTRUCT ss;
BOOL fNeedTuneDialog = TRUE;
BOOL f256QAM;
int nFrequency;
int nBandwidth;
BOOL fSpectrumInversion;
CRITICAL_SECTION csSignal;
HANDLE hInstance;

CCyUSBDevice * USBDevice = NULL;
CCyBulkEndPoint * MPEGInEpt = NULL;   
OVERLAPPED outOvLap;
OVERLAPPED inOvLap; 
#define MAX_PIDS 16
int nPIDList[MAX_PIDS];

char szLastSignalReport[128] = {"n/a"};        
char szLastTune[128] = {"n/a"};

#ifndef RF_CENTRAL
 char gszSourceName[] = {"Freecom DVB-T stick"};
#else RF_CENTRAL
 char gszSourceName[] = {"RF Central RFX-MDR-PC"};
#endif RF_CENTRAL

extern "C" {
	BOOL __cdecl SourceHelper_ValidateSourceContainer(PSOURCESTRUCT pss);
}

BOOL SendUSB(BYTE * bOut, int nSize)
{
	LONG nBytesSent;
	int nRetVal;

	UCHAR  * outContext = USBDevice->BulkOutEndPt->BeginDataXfer(bOut, nSize, &outOvLap); 
	USBDevice->BulkOutEndPt->WaitForXfer(&outOvLap, 1000);
	nRetVal = USBDevice->BulkOutEndPt->FinishDataXfer(bOut, nBytesSent, &outOvLap, outContext);

	return nRetVal;
}

int ReceiveUSB(BYTE * bIn, int nSize)
{
	int nRetVal;
	LONG nBytesReceived;

	UCHAR  *inContext = USBDevice->BulkInEndPt->BeginDataXfer(bIn, nSize, &inOvLap); 
	USBDevice->BulkInEndPt->WaitForXfer(&inOvLap, 500); 
	nRetVal = USBDevice->BulkInEndPt->FinishDataXfer(bIn, nBytesReceived, &inOvLap,inContext);

	return nBytesReceived;
}

void SetupLastTune()
{
	sprintf(szLastTune, "%.3f MHz", (double)ss->nFrequency / 1000.0);
}

void EnableDMA(BOOL fEnable)
{
	BYTE bOut[2];

	bOut[0] = 0x08;
	bOut[1] = fEnable;

	if (!SendUSB(bOut, sizeof(bOut)))
		OutputDebugString("Freecom: EnableDMA failed\n");
}

void SetupDemux()
{
	BYTE bOut[4];
	int nPIDOffset;
	int nMuxPIDOffset = 0;

	for (nPIDOffset = 0; nPIDOffset < MAX_PIDS; nPIDOffset++)
	{
		if (nPIDList[nPIDOffset] == -1)
			continue;
		bOut[0] = 0x04;		// PID filter
		bOut[1] = nMuxPIDOffset;
		bOut[2] = nPIDList[nPIDOffset] & 0xff;
		bOut[3] = nPIDList[nPIDOffset] >> 8;
		SendUSB(bOut, 4);
		nMuxPIDOffset++;
	}
}

BOOL TSReader_Tune()
{
	int nActualFrequency;
	BOOL fLocked = TRUE;
	DWORD dwStartLockTime = 0;
	BYTE bOut[12];
	BYTE bIn[3];

	fLocked = FALSE;
	SetupLastTune();

	//for (x = 0; x < 2; x++)
	{
		// No idea - reset maybe?
		bOut[0] = 0x01;
		SendUSB(bOut, 1);

		// No idea
		bOut[0] = 0x00;
		SendUSB(bOut, 1);
		ReceiveUSB(bIn, 1);
	}

	EnableDMA(FALSE);

	// No idea
	bOut[0] = 0x05;
	SendUSB(bOut, 1);
	
	// Setup bandwidth
	bOut[0] = 0x03;
	switch(ss->nBandwidth)
	{
	case 0:
		bOut[1] = 6;
		break;
	case 1:
		bOut[1] = 7;
		break;
	case 2:
		bOut[1] = 8;
		break;
	}
	SendUSB(bOut, 2);

	// Setup frequency
#ifndef RF_CENTRAL
	nActualFrequency = ss->nFrequency / 250;
#else RF_CENTRAL
	nActualFrequency = (ss->nFrequency - 1833000) / 250;
#endif RF_CENTRAL
	bOut[0] = 0x02;
	bOut[1] = nActualFrequency & 0xff;
	bOut[2] = nActualFrequency >> 8;
	SendUSB(bOut, 3);

	// No idea
	bOut[0] = 0x88;
	SendUSB(bOut, 1);
	ReceiveUSB(bIn, 2);

	EnableDMA(FALSE);
	bOut[0] = 0x05;		// reset demux
	SendUSB(bOut, 1);
	SetupDemux();


#define WAIT_LOCK
#ifdef WAIT_LOCK
	// Wait for a potential lock
	dwStartLockTime = GetTickCount();
	while (TRUE)
	{
		BYTE bTunerData[2];

		if ((GetTickCount() - dwStartLockTime) > 3000)
		{
			OutputDebugString("Freecom: No lock after 3 seconds\n");
			break;
		}

		// See if we're locked
		bOut[0] = 0x89;
		SendUSB(bOut, 1);
		ReceiveUSB(bTunerData, 1);
		if (bTunerData[0] != 0xff)
		{
			fLocked = TRUE;
			break;	// we've locked!
		}
		Sleep(250);
	}
#else WAIT_LOCK
	fLocked = TRUE;
#endif WAIT_LOCK

	if (!fLocked)
	{
		if (ss->fQuietMode == FALSE)
			MessageBox(ss->hWndTSReader, "Failed to lock signal", gszSourceName, MB_ICONWARNING);
		return FALSE;
	}
	return TRUE;
}

void CheckLockStatus()
{
	BYTE bResponses[16];
	BYTE bOut[2];
	char szTemp[256];

	bOut[0] = 0x89;
	SendUSB(bOut, 1);
	ReceiveUSB(&bResponses[0], 1);
	bOut[0] = 0x8a;
	SendUSB(bOut, 1);
	ReceiveUSB(&bResponses[1], 1);
	bOut[0] = 0x8c;
	SendUSB(bOut, 1);
	ReceiveUSB(&bResponses[2], 3);
	bOut[0] = 0x8e;
	SendUSB(bOut, 1);
	ReceiveUSB(&bResponses[5], 2);
	bOut[0] = 0x8d;
	SendUSB(bOut, 1);
	ReceiveUSB(&bResponses[7], 3);
	bOut[0] = 0x81;		// Reflects frequency
	SendUSB(bOut, 1);
	ReceiveUSB(&bResponses[10], 3);

	wsprintf(szTemp, "89=%02x 8a=%02x 8c=%02x%02x%02x 8e=%02x%02x 8d=%02x%02x%02x 81=%02x%02x%02x\n",
		     bResponses[0], bResponses[1], bResponses[2], bResponses[3], bResponses[4],
			 bResponses[5], bResponses[6], bResponses[7], bResponses[8], bResponses[9],
			 bResponses[10], bResponses[11], bResponses[12], bResponses[13]);
	//OutputDebugString(szTemp);

	if (bResponses[0] != 0xff)
		wsprintf(szTemp, "Locked (%03d/%03d)", bResponses[0], bResponses[1]);
	else
		wsprintf(szTemp, "Unlocked (255/%03d)", bResponses[1]);
	EnterCriticalSection(&csSignal);
	lstrcpy(szLastSignalReport, szTemp);
	LeaveCriticalSection(&csSignal);
}

#define READ_BUFFER_SIZE 3968
DWORD WINAPI ReadMPEGINThread(LPVOID lpv)
{
	int nReadPtr = 0;
	int nCurrentBuffer;
	BOOL fRestart = TRUE;
	DWORD dwLastTickCount = 0;
	//HANDLE hDebug;

	OutputDebugString("Freecom: +ReadMPEGINThread\n");
	//hDebug = CreateFile("c:\\tsreader.ts", GENERIC_WRITE, 0, (LPSECURITY_ATTRIBUTES)NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, (HANDLE)NULL);

	ss->fReadThreadTerminated = FALSE;
	ss->fTerminateReadThread = FALSE;

	// Setup the pipe used to sync up with the TS packets and 
	// feed TSReader	
	SourceHelper_StartSyncThread(ss, FALSE);

RestartSasemDataThread:
	fRestart = FALSE;
	LONG nTransferLength = READ_BUFFER_SIZE;
	MPEGInEpt->SetXferSize(nTransferLength);

	// Setup the asynchronous transfer buffers
	int nQueueSize = 64;
	PUCHAR *buffers = new PUCHAR[nQueueSize];
	PUCHAR *contexts = new PUCHAR[nQueueSize];
	OVERLAPPED * inMPEGOvLap = new OVERLAPPED[nQueueSize];

	for (nCurrentBuffer = 0; nCurrentBuffer < nQueueSize; nCurrentBuffer++)
	{ 
       buffers[nCurrentBuffer] = new UCHAR[nTransferLength];
	   inMPEGOvLap[nCurrentBuffer].Internal = inMPEGOvLap[nCurrentBuffer].InternalHigh = 0;
	   inMPEGOvLap[nCurrentBuffer].Offset = inMPEGOvLap[nCurrentBuffer].OffsetHigh = 0;
	   inMPEGOvLap[nCurrentBuffer].hEvent = CreateEvent(NULL, false, false, NULL);
	}

	// Queue-up the first batch of transfer requests
	for (nCurrentBuffer = 0; nCurrentBuffer < nQueueSize; nCurrentBuffer++)
	   contexts[nCurrentBuffer] = MPEGInEpt->BeginDataXfer(buffers[nCurrentBuffer], nTransferLength, &inMPEGOvLap[nCurrentBuffer]);
	EnableDMA(TRUE);

	nCurrentBuffer = 0;
	//OutputDebugString("Freecom: enter main read loop\n");
	while (!ss->fTerminateReadThread)
	{
		LONG nReceiveLength = 0;
		
		if (!MPEGInEpt->WaitForXfer(&inMPEGOvLap[nCurrentBuffer], 2500))
		{
			char szTemp[128];
			wsprintf(szTemp, "Freecom: WaitForXfer() timed out buffer = %d ***********\n", nCurrentBuffer);
			OutputDebugString(szTemp);
			fRestart = TRUE;
			break;
		}

		if (MPEGInEpt->FinishDataXfer(buffers[nCurrentBuffer], nReceiveLength, &inMPEGOvLap[nCurrentBuffer],
			contexts[nCurrentBuffer]))
		{
			int nOffset = 0;
			int i;

			for (i = 0; i < nReceiveLength / 189; i++)
			{
				SourceHelper_SyncData(&buffers[nCurrentBuffer][nOffset], 188);
				nOffset += 189;
			}
			SourceHelper_SyncData(&buffers[nCurrentBuffer][nOffset], 188);
		}
		else
			Sleep(1);
	    contexts[nCurrentBuffer] = MPEGInEpt->BeginDataXfer(buffers[nCurrentBuffer], nTransferLength, &inMPEGOvLap[nCurrentBuffer]);
		nCurrentBuffer++;
		if (nCurrentBuffer == nQueueSize)
			nCurrentBuffer = 0;

		if (GetTickCount() > dwLastTickCount)
		{
			dwLastTickCount = GetTickCount() + 1000;
			CheckLockStatus();
		}
	}
	OutputDebugString("Freecom: left main read loop\n");
	CheckLockStatus();

	MPEGInEpt->Abort();
	for (nCurrentBuffer = 0; nCurrentBuffer < nQueueSize; nCurrentBuffer++)
	{
		// Wait for all the queued requests to be cancelled 
		LONG nReceiveLength = 0;

		MPEGInEpt->FinishDataXfer(buffers[nCurrentBuffer], nReceiveLength, &inMPEGOvLap[nCurrentBuffer], contexts[nCurrentBuffer]);
		CloseHandle(inMPEGOvLap[nCurrentBuffer].hEvent);
		delete [] buffers[nCurrentBuffer];
	}

	delete [] buffers;
	delete [] contexts;
	delete [] inMPEGOvLap;

	EnableDMA(FALSE);
	if (fRestart == TRUE && !ss->fTerminateReadThread)
	{
		//SendTunerFrequency(ss->nFrequency);
		goto RestartSasemDataThread;
	}

	SourceHelper_StopSyncThread();

	ss->fReadThreadTerminated = TRUE;
	ss->fTerminateReadThread = FALSE;
	EnterCriticalSection(&ss->csTSBuffersInUse);
	ss->nTSBuffersInUse = -1000;
	LeaveCriticalSection(&ss->csTSBuffersInUse);
	CloseHandle(ss->hReadDataThread);
	//CloseHandle(hDebug);
	OutputDebugString("Freecom: -ReadMPEGINThread\n");
	return 0;
	goto RestartSasemDataThread;	// to stop the compiler bitching
}

BOOL TSReader_Start()
{
	DWORD dwThreadID;

	OutputDebugString("Freecom: enter Start()\n");

	ss->hReadDataThread = CreateThread(NULL, 0, ReadMPEGINThread, (LPVOID)0, CREATE_SUSPENDED, &dwThreadID);
	SourceHelper_SetWorkerThreadPriorities(FALSE);
	ResumeThread(ss->hReadDataThread);

	OutputDebugString("Freecom: leave Start()\n");
	return TRUE;
}

BOOL TSReader_Stop()
{
	OutputDebugString("Freecom: enter Stop()\n");

	ss->fTerminateReadThread = TRUE;
	while (ss->fReadThreadTerminated == FALSE)
		Sleep(50);

	OutputDebugString("Freecom: leave Stop()\n");
	return TRUE;
}

BOOL OpenUSBDriver()
{
	int d = 0;
	GUID OASISguid;

	OASISguid.Data1 = 0xFA58C45D;
	OASISguid.Data2 = 0x5B19; OASISguid.Data3 = 0x428b;
	OASISguid.Data4[0] = 0xA2; OASISguid.Data4[1] = 0xD1; OASISguid.Data4[2] = 0x27; OASISguid.Data4[3] = 0x07;
	OASISguid.Data4[4] = 0x85; OASISguid.Data4[5] = 0x6D; OASISguid.Data4[6] = 0x7E; OASISguid.Data4[7] = 0x19;
	
	USBDevice = new CCyUSBDevice(NULL, OASISguid);
    int nDevices = USBDevice->DeviceCount(); 
	if (nDevices == 0)
	{
		MessageBox(NULL, "Unable to locate the Freecom interface. Make sure you've switched to the DTVWorks driver.", gszSourceName, MB_ICONSTOP);
		return FALSE;
	}

	// Find the Sasem device by it's vendor number
    do
	{
		USBDevice->Open(d);   // Open automatically  calls Close() if necessary 

		int vID = USBDevice->VendorID; 
        int pID  = USBDevice->ProductID;

		if (vID == 0x14aa && pID == 0x022b)
			break;
		USBDevice->Close();
		d++;
    } while (d < nDevices);

	if (d == nDevices)
	{
		MessageBox(NULL, "Unable to locate the Freecom interface", gszSourceName, MB_ICONSTOP);
		return FALSE;
	}

	if (USBDevice->EndPoints == NULL)
	{
		OutputDebugString("Freecom: NULL endpoint from the USB device\n");
		return FALSE;
	}

	if (USBDevice->BulkOutEndPt == NULL)
	{
		OutputDebugString("Freecom: Couldn't find control out endpoint\n");
		return FALSE;
	}

	return TRUE;
}

void CloseOvelapppedEvents()
{
	CloseHandle(outOvLap.hEvent);
	CloseHandle(inOvLap.hEvent);
}

BOOL TSReader_Init(PSOURCESTRUCT pss)
{
	int nIndex;
	int nEndPointCount;

	if (SourceHelper_ValidateSourceContainer(pss) == FALSE)
		return FALSE;
	
	OutputDebugString("Freecom: Init\n");
	lstrcpy(szLastSignalReport, "n/a");
	InitializeCriticalSection(&csSignal);

	ss = pss;

	outOvLap.hEvent = CreateEvent(NULL, false, false, "CYUSB_OUT"); 
	inOvLap.hEvent = CreateEvent(NULL, false, false, "CYUSB_IN"); 

	MPEGInEpt = (CCyBulkEndPoint *)NULL;

	if (OpenUSBDriver() == FALSE)
		return FALSE;

	// Find the USB pipe we use for data transfer (IN 2 for Freecom)
	nEndPointCount = USBDevice->EndPointCount();  
	for (nIndex = 1; nIndex < nEndPointCount; nIndex++)
	{
		if (USBDevice->EndPoints[nIndex]->Address == 0x82)
		{
			MPEGInEpt = (CCyBulkEndPoint *)USBDevice->EndPoints[nIndex];
			break;
		}
	}
	if (MPEGInEpt == (CCyBulkEndPoint *)NULL)
	{
		OutputDebugString("Freecom: Couldn't find endpoinst\n");
		return FALSE;
	}

	// Setup default PIDs
	for (nIndex = 0; nIndex < MAX_PIDS; nIndex++)
		nPIDList[nIndex] = -1;
	nPIDList[0] = 0x0000;	// PAT
	nPIDList[1] = 0x1fff;	// nulls
	nPIDList[2] = 0x0001;	// CAT
	nPIDList[3] = 0x0010;	// DVB NIT
	nPIDList[4] = 0x0011;	// DVB SDT
	nPIDList[5] = 0x0012;	// DVB EIT
	nPIDList[6] = 0x0013;	// DVB TDT
	nPIDList[7] = 0x1ffb;	// ATSC VCT
	return TRUE;
}

BOOL TSReader_DeInit()
{
	OutputDebugString("Freecom: +DeInit\n");

	CloseOvelapppedEvents();

	{
		char szTemp[128];
		wsprintf(szTemp, "Freecom: USBDevice->EndPoints = 0x%08x\n", USBDevice->EndPoints);
		OutputDebugString(szTemp);
	}
	if (USBDevice->EndPoints != NULL)
		delete USBDevice;

	DeleteCriticalSection(&csSignal);
	OutputDebugString("Freecom: -DeInit\n");
	return TRUE;
}

BOOL TSReader_TuneDialog(HWND hWnd)
{
	OutputDebugString("Freecom: TSReader_TuneDialog\n");

	if (ss->fDontTune == TRUE)
		fNeedTuneDialog = FALSE;

	if (fNeedTuneDialog)
	{
		OutputDebugString("Freecom: TSReader_TuneDialog tuning dialog is required\n");
		ss->fQuietMode = FALSE;
		if (SourceHelper_DVBTTuneDialog(hWnd) == FALSE)
			return FALSE;
	}
	else
	{
		OutputDebugString("Freecom: TSReader_TuneDialog tune NOT required\n");
		ss->nFrequency = nFrequency;
		ss->nBandwidth = nBandwidth;
		fNeedTuneDialog = TRUE;
	}

	return TRUE;
}

BOOL TSReader_PIDManagement(BOOL fAdd, int nPID, BOOL fTemporary)
{
	int i;
	
	if (fAdd)
	{
		for (i = 0; i < MAX_PIDS; i++)
		{
			if (nPIDList[i] == nPID)
				return TRUE;	// already doing this PID

			if (nPIDList[i] == -1)
			{
				nPIDList[i] = nPID;
				break;
			}
		}
	}
	else
	{
		for (i = 0; i < MAX_PIDS; i++)
		{
			if (nPIDList[i] == nPID)
			{
				nPIDList[i] = -1;
				break;
			}
		}
	}

	SetupDemux();

	return TRUE;
}

BOOL TSReader_GetDescription(char * szDescription, char * szCommandLineParameters, BOOL * fCanBeStopped, int * nMaxPIDs, DWORD * dwCapabilities)
{
	if (szDescription != NULL)
		lstrcpy(szDescription, gszSourceName);
	if (fCanBeStopped != NULL)
		*fCanBeStopped = FALSE;
	if (nMaxPIDs != NULL)
		*nMaxPIDs = MAX_PIDS;
	if (szCommandLineParameters != NULL)
		lstrcpy(szCommandLineParameters, "freq spectrum-invert bandwidth");	
	if (dwCapabilities != NULL)
		*dwCapabilities = 0;

	return TRUE;
}

BOOL TSReader_ParseCommandLine(PSOURCESTRUCT ss, char * szCommandLine, BOOL fQuiet)
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
			if (!fQuiet)
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
	int i;

	for (i = 0; i < MAX_PIDS; i++)
	{
		if (nPIDList[i] == nPID)
			return TRUE;
	}
	return FALSE;
}

BOOL TSReader_SetChannel(int nChannel)
{
	return TRUE;
}

BOOL TSReader_GetSignalString(char * szString)
{
	EnterCriticalSection(&csSignal);
	lstrcpy(szString, szLastSignalReport);
	LeaveCriticalSection(&csSignal);
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

int TSReader_GetSyncLossCount(BOOL fReset)
{
	return SourceHelper_GetSyncLossCount(fReset);
}

BOOL APIENTRY DllMain(HANDLE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
    switch( ul_reason_for_call )
	{
    case DLL_PROCESS_ATTACH:
		hInstance = (HINSTANCE)hModule;
		break;
    case DLL_PROCESS_DETACH:
		break;
    }
    return TRUE;
}


