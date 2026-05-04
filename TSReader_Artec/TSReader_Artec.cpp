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
int nFrequency;
CRITICAL_SECTION csSignal;
HANDLE hInstance;

CCyUSBDevice * USBDevice = NULL;
CCyBulkEndPoint * MPEGInEpt = NULL;   
CCyBulkEndPoint * ControlInEpt = NULL;
CCyBulkEndPoint * ControlOutEpt = NULL;
OVERLAPPED outOvLap;
OVERLAPPED inOvLap; 

char szLastSignalReport[128] = {"n/a"};        
char szLastTune[128] = {"n/a"};

#ifndef QAM
 char gszSourceName[] = {"Artec T14A Mini 8VSB"};
#else QAM
 char gszSourceName[] = {"Artec T14A Mini QAM"};
#endif QAM

extern "C" {
	BOOL __cdecl SourceHelper_ValidateSourceContainer(PSOURCESTRUCT pss);
}

BOOL SendUSB(BYTE * bOut, int nSize)
{
	LONG nBytesSent;
	int nRetVal;

	UCHAR  * outContext = ControlOutEpt->BeginDataXfer(bOut, nSize, &outOvLap); 
	ControlOutEpt->WaitForXfer(&outOvLap, 1000);
	nRetVal = ControlOutEpt->FinishDataXfer(bOut, nBytesSent, &outOvLap, outContext);

	return nRetVal;
}

int ReceiveUSB(BYTE * bIn, int nSize)
{
	int nRetVal;
	LONG nBytesReceived;

	UCHAR  *inContext = ControlInEpt->BeginDataXfer(bIn, nSize, &inOvLap); 
	ControlInEpt->WaitForXfer(&inOvLap, 500); 
	nRetVal = ControlInEpt->FinishDataXfer(bIn, nBytesReceived, &inOvLap,inContext);

	return nBytesReceived;
}

void SetupLastTune()
{
#ifndef QAM
	wsprintf(szLastTune, "Channel %d (%d MHz)", SourceHelper_GetATSCChannelFromFrequency(ss->nFrequency), ss->nFrequency);
#else QAM
	wsprintf(szLastTune, "Channel %d (%d MHz)", SourceHelper_GetQAMChannelFromFrequency(ss->nFrequency), ss->nFrequency);
#endif QAM
}

void EnableDMA(BOOL fEnable)
{
	BYTE bOut[2];

	bOut[0] = 0x07;
	if (fEnable)
		bOut[1] = 0x01;
	else
		bOut[1] = 0x02;

	if (!SendUSB(bOut, sizeof(bOut)))
		OutputDebugString("Atec: EnableDMA failed\n");
}

void WriteI2CSimple(int nAddress, int nRegister, int nData)
{
	BYTE bOut[64];

	bOut[0] = 0x03;
	bOut[1] = nAddress;
	bOut[2] = nRegister;
	bOut[3] = nData;
	SendUSB(bOut, 4);
}

int ReadI2CSimple(int nAddress, int nRegister)
{
	BYTE bOut[64];
	BYTE bIn[4];

	bOut[0] = 0x02;
	bOut[1] = nAddress;
	bOut[2] = nRegister;
	bOut[3] = 0;	// ?
	bOut[4] = 1;	// count

	SendUSB(bOut, 5);
	ReceiveUSB(bIn, 1);

	return bIn[0];
}

void SendTuneCommands()
{
	int nLNABand;
	double fLO1, fLO2;
	double DIV1, NUM1, DIV2, NUM2;
	double mode1, mode2;
	BYTE bOut[8];

	if (ss->nFrequency >= 48 && ss->nFrequency <= 95)			nLNABand = 11;
	else if (ss->nFrequency >= 95 && ss->nFrequency <= 180)		nLNABand = 10;
	else if (ss->nFrequency >= 180 && ss->nFrequency <= 260)	nLNABand = 9;
	else if (ss->nFrequency >= 260 && ss->nFrequency <= 335)	nLNABand = 8;
	else if (ss->nFrequency >= 335 && ss->nFrequency <= 425)	nLNABand = 7;
	else if (ss->nFrequency >= 425 && ss->nFrequency <= 490)	nLNABand = 6;
	else if (ss->nFrequency >= 490 && ss->nFrequency <= 570)	nLNABand = 5;
	else if (ss->nFrequency >= 570 && ss->nFrequency <= 645)	nLNABand = 4;
	else if (ss->nFrequency >= 645 && ss->nFrequency <= 730)	nLNABand = 3;
	else if (ss->nFrequency >= 730 && ss->nFrequency <= 810)	nLNABand = 2;
	else if (ss->nFrequency >= 810 && ss->nFrequency <= 860)	nLNABand = 1;

	fLO1 = ss->nFrequency + 1220.0;
	fLO2 = 1475.0 - ss->nFrequency - 36.15;		
	DIV1 = fLO1 / 16.0;
	mode1 = fLO1 / 16.0; mode1 = (mode1 - (int)mode1) * 16.0;
	NUM1 = (64.0 * mode1) / 16.0;
	DIV2 = fLO2 / 16.0;
	mode2 = fLO2 / 16.0;
	mode2 = (mode2 - (int)mode2);
	NUM2 = 8192.0 * (mode2 / 16.0) + 0.5;
	
	WriteI2CSimple(0xc0, 0x0c, 0xff);	// required by the MT2060

	bOut[0] = 0x03;		// i2c write
	bOut[1] = 0xc0;		// addr
	bOut[2] = 0x01;		// register
	bOut[3] = nLNABand << 4 | ((int)NUM1 >> 2) & 0x0f;
	bOut[4] = (int)DIV1 & 0xff;
	bOut[5] = ((int)NUM1 & 0x03) << 4 | (int)NUM2 & 0x0f;
	bOut[6] = ((int)NUM2 >> 4) & 0xff;
	bOut[7] = ((int)DIV2 << 1) | ((int)NUM2 >> 12) & 0x01;
	
	bOut[5] = 0x00;
	bOut[6] = 0x00;
	bOut[7] = 0x93;
	SendUSB(bOut, 8);
}

BOOL TSReader_Tune()
{
	BOOL fLocked = TRUE;
	DWORD dwStartLockTime = 0;
	int nINLVTHD = 0;

	fLocked = FALSE;
	SetupLastTune();

#ifndef QAM
	WriteI2CSimple(0x1c, 0x00, 0x03);	// VSB mode
	WriteI2CSimple(0x1c, 0x0d, 0x40);
	WriteI2CSimple(0x1c, 0x0e, 0x87);
	WriteI2CSimple(0x1c, 0x0f, 0x8e);
	WriteI2CSimple(0x1c, 0x10, 0x01);
	WriteI2CSimple(0x1c, 0x42, 0xf7);
	WriteI2CSimple(0x1c, 0x43, 0xc0);
	WriteI2CSimple(0x1c, 0x45, 0x88);
	WriteI2CSimple(0x1c, 0x46, 0x44);
	WriteI2CSimple(0x1c, 0x47, 0x88);
	WriteI2CSimple(0x1c, 0x48, 0x44);
	WriteI2CSimple(0x1c, 0x4d, 0x1e);
	WriteI2CSimple(0x1c, 0x51, 0x61);
	WriteI2CSimple(0x1c, 0x87, 0xff);
#else QAM
	WriteI2CSimple(0x1c, 0x00, 0x01);	// 256QAM mode		
	WriteI2CSimple(0x1c, 0x0d, 0x0f);
	WriteI2CSimple(0x1c, 0x0e, 0x5c);
	WriteI2CSimple(0x1c, 0x0f, 0x28);
	WriteI2CSimple(0x1c, 0x10, 0xf6);
	WriteI2CSimple(0x1c, 0x42, 0x47);
	WriteI2CSimple(0x1c, 0x43, 0x6b);
	WriteI2CSimple(0x1c, 0x45, 0x88);
	WriteI2CSimple(0x1c, 0x46, 0x89);
	WriteI2CSimple(0x1c, 0x47, 0x66);
	WriteI2CSimple(0x1c, 0x48, 0x66);
	WriteI2CSimple(0x1c, 0x49, 0x08);
	WriteI2CSimple(0x1c, 0x4a, 0x9b);
	WriteI2CSimple(0x1c, 0x4b, 0x94);
	WriteI2CSimple(0x1c, 0x4c, 0x14);
	WriteI2CSimple(0x1c, 0x4d, 0x1a);
	WriteI2CSimple(0x1c, 0x51, 0x63);


#endif QAM
	WriteI2CSimple(0x1c, 0x02, 0x00);	// demod soft reset on
	WriteI2CSimple(0x1c, 0x02, 0x01);	// demod soft reset off

	SendTuneCommands();
	ReadI2CSimple(0xc1, 0x06);
	Sleep(500);

	// Wait for a potential lock
	dwStartLockTime = GetTickCount();
	while (TRUE)
	{
		if ((GetTickCount() - dwStartLockTime) > 3000)
		{
			OutputDebugString("Atec: No lock after 3 seconds\n");
			break;
		}

		// See if we're locked
		if (ReadI2CSimple(0x1c, 0x05))	// non-zero if there's something there
		{
			fLocked = TRUE;
			break;	// we've locked!
		}
		Sleep(1000);
		SendTuneCommands();
	}

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
	int nRawSNR = ((ReadI2CSimple(0x1c, 0x6e) & 0x78) >> 3) << 16
			    | ReadI2CSimple(0x1c, 0x6f) << 8
				| ReadI2CSimple(0x1c, 0x70);
	char szTemp[128];

	if (nRawSNR > 0xffff)
	{
		lstrcpy(szTemp, "Unlocked");
	}
	else
	{
		double fSNR = 10 * log10(double((25 * 32 * 32) / nRawSNR));
		if (fSNR < 0.0)
			fSNR = 0.0;
		sprintf(szTemp, "Locked SNR %.1f dB", fSNR);
	}

	EnterCriticalSection(&csSignal);
	lstrcpy(szLastSignalReport, szTemp);
	LeaveCriticalSection(&csSignal);
}

#define READ_BUFFER_SIZE 48 * 1024
DWORD WINAPI ReadMPEGINThread(LPVOID lpv)
{
	int nReadPtr = 0;
	int nCurrentBuffer;
	BOOL fFirstPacket = FALSE;
	BOOL fRestart = TRUE;
	DWORD dwLastTickCount = 0;
	//HANDLE hDebug;

	OutputDebugString("Atec: +ReadMPEGINThread\n");
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
	int nQueueSize = 16;
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
	//OutputDebugString("Atec: enter main read loop\n");
	while (!ss->fTerminateReadThread)
	{
		LONG nReceiveLength = 0;
		
		if (!MPEGInEpt->WaitForXfer(&inMPEGOvLap[nCurrentBuffer], 2500))
		{
			char szTemp[128];
			wsprintf(szTemp, "Atec: WaitForXfer() timed out buffer = %d ***********\n", nCurrentBuffer);
			OutputDebugString(szTemp);
			fRestart = TRUE;
			break;
		}

		if (MPEGInEpt->FinishDataXfer(buffers[nCurrentBuffer], nReceiveLength, &inMPEGOvLap[nCurrentBuffer],
			contexts[nCurrentBuffer]))
		{
			SourceHelper_SyncData(buffers[nCurrentBuffer], nReceiveLength);
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
	OutputDebugString("Atec: left main read loop\n");
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
	OutputDebugString("Atec: -ReadMPEGINThread\n");
	return 0;
	goto RestartSasemDataThread;	// to stop the compiler bitching
}

BOOL TSReader_Start()
{
	DWORD dwThreadID;

	OutputDebugString("Atec: enter Start()\n");

	ss->hReadDataThread = CreateThread(NULL, 0, ReadMPEGINThread, (LPVOID)0, CREATE_SUSPENDED, &dwThreadID);
	SourceHelper_SetWorkerThreadPriorities(FALSE);
	ResumeThread(ss->hReadDataThread);

	OutputDebugString("Atec: leave Start()\n");
	return TRUE;
}

BOOL TSReader_Stop()
{
	OutputDebugString("Atec: enter Stop()\n");

	ss->fTerminateReadThread = TRUE;
	while (ss->fReadThreadTerminated == FALSE)
		Sleep(50);

	OutputDebugString("Atec: leave Stop()\n");
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
		MessageBox(NULL, "Unable to locate the Artec interface. Make sure you've switched to the DTVWorks driver.", gszSourceName, MB_ICONSTOP);
		return FALSE;
	}

	// Find the Sasem device by it's vendor number
    do
	{
		USBDevice->Open(d);   // Open automatically  calls Close() if necessary 

		int vID = USBDevice->VendorID; 
        int pID  = USBDevice->ProductID;

		if (vID == 0x05d8 && pID == 0x810e)
			break;
		USBDevice->Close();
		d++;
    } while (d < nDevices);

	if (d == nDevices)
	{
		MessageBox(NULL, "Unable to locate the Artec interface", gszSourceName, MB_ICONSTOP);
		return FALSE;
	}

	if (USBDevice->EndPoints == NULL)
	{
		OutputDebugString("Atec: NULL endpoint from the USB device\n");
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
	
	OutputDebugString("Atec: Init\n");
	lstrcpy(szLastSignalReport, "n/a");
	InitializeCriticalSection(&csSignal);

	ss = pss;

	outOvLap.hEvent = CreateEvent(NULL, false, false, "CYUSB_OUT"); 
	inOvLap.hEvent = CreateEvent(NULL, false, false, "CYUSB_IN"); 

	MPEGInEpt = (CCyBulkEndPoint *)NULL;

	if (OpenUSBDriver() == FALSE)
		return FALSE;

	// Find the USB pipes
	nEndPointCount = USBDevice->EndPointCount();  
	for (nIndex = 0; nIndex < nEndPointCount; nIndex++)
	{
		if (USBDevice->EndPoints[nIndex]->Address == 0x01)
			ControlOutEpt = (CCyBulkEndPoint *)USBDevice->EndPoints[nIndex];
		else if (USBDevice->EndPoints[nIndex]->Address == 0x81)
			ControlInEpt = (CCyBulkEndPoint *)USBDevice->EndPoints[nIndex];
		else if (USBDevice->EndPoints[nIndex]->Address == 0x86)
			MPEGInEpt = (CCyBulkEndPoint *)USBDevice->EndPoints[nIndex];
	}
	if (   MPEGInEpt == (CCyBulkEndPoint *)NULL
		|| ControlOutEpt == (CCyBulkEndPoint *)NULL
		|| ControlInEpt == (CCyBulkEndPoint *)NULL )
	{
		OutputDebugString("Atec: Couldn't find endpoinst\n");
		return FALSE;
	}

	{
		BYTE bOut[] = {0x07, 0x00, 0x01}; // wake up
		SendUSB(bOut, sizeof(bOut));
	}
	{
		BYTE bOut[] = {0x03, 0xc0, 0x01,	 // i2c write address c0 reg 1
			0x3f, 0x74, 0x00, 0x08, 0x93, 0x88, 0x80, 0x60, 0x20, 0x1e, 0x30, 0xff, 0x80, 0xff, 0x00, 0x2c, 0x42};
		SendUSB(bOut, sizeof(bOut));
	}
	WriteI2CSimple(0x1c, 0x04, 0x00);		// turn off demod interrupts
	Sleep(50);

	return TRUE;
}

BOOL TSReader_DeInit()
{
	OutputDebugString("Atec: +DeInit\n");

	{
		BYTE bOut[4] = {0x07, 0x00, 0x00}; // sleep
		SendUSB(bOut, 3);
	}

	CloseOvelapppedEvents();

	{
		char szTemp[128];
		wsprintf(szTemp, "Atec: USBDevice->EndPoints = 0x%08x\n", USBDevice->EndPoints);
		OutputDebugString(szTemp);
	}
	if (USBDevice->EndPoints != NULL)
		delete USBDevice;

	DeleteCriticalSection(&csSignal);
	OutputDebugString("Atec: -DeInit\n");
	return TRUE;
}

BOOL TSReader_TuneDialog(HWND hWnd)
{
	OutputDebugString("Atec: TSReader_TuneDialog\n");

	if (ss->fDontTune == TRUE)
		fNeedTuneDialog = FALSE;

	if (fNeedTuneDialog)
	{
		OutputDebugString("Atec: TSReader_TuneDialog tuning dialog is required\n");
		ss->fQuietMode = FALSE;
#ifndef QAM
		if (SourceHelper_ATSCTuneDialog(hWnd) == FALSE)
#else QAM
		if (SourceHelper_QAMTuneDialog(hWnd) == FALSE)
#endif QAM
			return FALSE;
	}
	else
	{
		OutputDebugString("Atec: TSReader_TuneDialog tune NOT required\n");
		ss->nFrequency = nFrequency;
		fNeedTuneDialog = TRUE;
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
	if (fCanBeStopped != NULL)
		*fCanBeStopped = FALSE;
	if (nMaxPIDs != NULL)
		*nMaxPIDs = 8192;
	if (szCommandLineParameters != NULL)
		lstrcpy(szCommandLineParameters, "freq");	
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
								  "%d", 
								  &nFrequency);
		if (nConversionCount < 1)
		{
			if (!fQuiet)
				MessageBox(NULL,
					   "Usage for this source: freq inversion bandwidth\n"
					   "\n"
					   "freq = frequency to tune in MHz\n",
					   gszSourceName,
					   MB_OK | MB_ICONSTOP);
			return FALSE;
		}
		if (*szCommandLine == '0')
#ifndef QAM
			nFrequency = SourceHelper_GetFrequencyFromATSCChannel(nFrequency);
#else QAM
			nFrequency = SourceHelper_GetFrequencyFromQAMChannel(nFrequency);
#endif QAM
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


