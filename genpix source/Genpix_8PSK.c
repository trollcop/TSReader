// SampleSource.c
// Copyright (C) 2004-2005 COOLSTF.com Inc.
//
// Sample source for TSReader
//

#include <windows.h>
#include "inc\sources.h"
#include "inc\GP_debug.h"
#include "inc\CyAPI.h"
#include "inc\8psk_VendorCommandCodes.h"


// FW in the form of C record
#include "inc\6000.h"

#define	GP8PSK_VID	0x09C0
#define GP8PSK_PID	0x0201

CCyUSBDevice *GpDevice;
CRITICAL_SECTION csSignal;
BOOL fInitDone;
BOOL fShutDown;

static BYTE bDishBytes[] = {
	0x34, // SW21 Dish 1
	0x65, // SW21 Dish 2
	0x46, // SW42 Dish 1
	0x17, // SW42 Dish 2
	0x68, // SW44 Dish 2
	0x39, // SW64 Dish 1A
	0x1A, // SW64 Dish 1B
	0x4B, // SW64 Dish 2A
	0x5C, // SW64 Dish 2B
	0x0D, // SW64 Dish 3A
	0x2E, // SW64 Dish 3B
	0x72, // Twin LNB 1
	0x23, // Twin LNB 2
	0x51}; // Quad LNB 2

typedef struct
{
	DWORD	SymbolRate;
	DWORD	Frequency;
	BYTE	ModulationMode;
	BYTE	CodeRate;
} TuneParameters;


BOOL GP_InitUSBdriver();
void GP_DeInitUSBdriver();
BOOL __fastcall GP_SendDISEqCpacket(BYTE *Buf, int length);
static BOOL GP_Tune();
BOOL GP_BCM4500Locked();
BOOL __fastcall GP_EnableMPEGstream(BOOL Start);
static int __fastcall GP_SignalStrength();

#define READ_FROM_FX2_SIZE 128 * 1024

PSOURCESTRUCT ss;
BOOL fNeedTuneDialog;
int nFrequency;
int nPolarity;
int nSymbolRate;
int nLNBFrequency;
int n22KHz;
int nADVModulationMode;
int nCodeRate;
int nDiSEqCInput;
HANDLE hInstance;
char szLastSignalReport[128] = {0};        
char szLastTune[128] = {0};
char gszSourceName[] = {"Genpix 8PSK-USB"};


// Function checks for Signal Strength, updates LastSignalReport string
// returns TRUE if tuner is locked
BOOL CheckLockStatus()
{
	BOOL fLocked = GP_BCM4500Locked();
	if (!fLocked)
	{
		wsprintf(szLastSignalReport, "NOT locked");
		return FALSE;
	}
	int i = GP_SignalStrength()/38.4;
	wsprintf(szLastSignalReport, "Locked Signal %i%%", i);
	return TRUE;
}

void SetupLastTune()
{
//	szLastSignalReport[0] = '\0';
	char szPolarity[4] = {"H/L"};
	char szModulation[16] = {0};
	if (ss->nPolarity == 0)
		lstrcpy(szPolarity, "V/R");
	switch(ss->nADVModulationMode)
	{
		case ADV_MOD_DVB_QPSK:
			lstrcpy(szModulation, "DVB QPSK");
			break;
		case ADV_MOD_TURBO_QPSK:
			lstrcpy(szModulation, "Turbo QPSK");
			break;
		case ADV_MOD_TURBO_8PSK:
			lstrcpy(szModulation, "8PSK");
			break;
		case ADV_MOD_TURBO_16QAM:
			lstrcpy(szModulation, "16QAM");
			break;
		case ADV_MOD_DCII_C_QPSK:
			lstrcpy(szModulation, "DC2C QPSK");
			break;
		case ADV_MOD_DCII_I_QPSK:
			lstrcpy(szModulation, "DC2I QPSK");
			break;
		case ADV_MOD_DCII_Q_QPSK:
			lstrcpy(szModulation, "DC2Q QPSK");
			break;
		case ADV_MOD_DCII_C_OQPSK:
			lstrcpy(szModulation, "DC2 OQPSK");
			break;
	}
	wsprintf(szLastTune, "%d MHz %s %d %s", ss->nFrequency, szPolarity, ss->nSymbolRate, szModulation);
}

// TSReader_Tune()
// 
// This is where you setup the tuner to receive the transport stream
// If you're a source with CAPABILITIES_SERIAL_CONTROL you should
// call into the SourceHelper_TuneSerialControl() function to have it
// tune the serially controlled receiver.
//
// Return TRUE if you locked, FALSE if no lock and -1 if you're a terrestrial tuner that detects analog signals
BOOL TSReader_Tune()
{
//	BOOL fLocked = TRUE;	// we're always locked!
	BOOL fLocked;
	
	for (int j = 0; j < 2; j++)
	{
		GP_Tune();
		for (int i = 0; i < 20; i++)
		{
			fLocked = GP_BCM4500Locked();
			if (fLocked)
				break;
			Sleep(50);
		}
		if (fLocked)
			break;
	}
	if (ss->fQuietMode == FALSE)
	{
		if (!fLocked)
		{
			MessageBox(ss->hWndTSReader, "Failed to lock", gszSourceName, MB_ICONSTOP);
			return FALSE;
		}
	}
	CheckLockStatus();
	SetupLastTune();
	return TRUE;
}

// Here's a pretend function that returns data to the current TS buffer.
// In this case we return all null packets
int MagicReadFunction(BYTE * pBuffer, int nMaxLen)
{
	int nOffset;

	memset(pBuffer, 0xff, nMaxLen);		// null packets are all 0xff
	for (nOffset = 0; nOffset < nMaxLen; nOffset += 188)
	{
		pBuffer[nOffset + 0] = 0x47;		// MPEG-2 sync
		pBuffer[nOffset + 1] = 0x1f;		// PID MSB + flags (don't care)
		pBuffer[nOffset + 2] = 0xff;		// PID LSB
		pBuffer[nOffset + 3] = 0x00;		// who cares about other flags
	}

	Sleep(25);		// don't send data at full bore
	return nMaxLen;
}

// This is the thread where you put code to take data from your interface
// and pass it onto TSReader. Data sent to TSReader MUST be packet aligned (0x47 at offset zero)
// with 188 bytes per packet, i.e. if your interface sends 204 byte packets, you must
// strip them down in here before sending to TSReader. You must also send over complete
// packets. There's no semaphore other than a non-zero ss->nTSBufferInUse - when TSReader's main
// thread sees this non-zero, it copies the current buffer and decrements this counter, otherwise
// it sleeps for 5 ms and tries again.

// Make sure you also look at the routine after this one too (ReadDataThreadNonAligned) as
// it shows another method of moving data into TSReader.

DWORD WINAPI ReadDataThreadAligned(LPVOID lpv)
{
	int nTSBufferIndex = 0;

	while (!ss->fTerminateReadThread)
	{
		int nRetLen = MagicReadFunction(ss->tsb[nTSBufferIndex].pData, TS_BUFFER_SIZE);
		if (nRetLen > 0)
		{
			EnterCriticalSection(&ss->csPIDCounter);
			ss->nLastSecondByteCounter += nRetLen;
			LeaveCriticalSection(&ss->csPIDCounter);

			ss->tsb[nTSBufferIndex].nSize = nRetLen;
			nTSBufferIndex++;
			if (nTSBufferIndex == MAX_TS_BUFFERS)
				nTSBufferIndex = 0;
			EnterCriticalSection(&ss->csTSBuffersInUse);
			ss->nTSBuffersInUse++;
			LeaveCriticalSection(&ss->csTSBuffersInUse);
		}
	}

	// Time to quit - flag buffers in use as -1000 which tells
	// TSReader's main thread to terminate. If you don't do this
	// TSReader will hang when exiting.
	ss->fReadThreadTerminated = TRUE;
	ss->fTerminateReadThread = FALSE;
	EnterCriticalSection(&ss->csTSBuffersInUse);
	ss->nTSBuffersInUse = -1000;
	LeaveCriticalSection(&ss->csTSBuffersInUse);

	return 0;
}

// This is another sample input data thread that can handle unaligned
// transport streams. The TSReader_SourceHelper.dll has three functions
// which allow you to send over non-aligned MPEG-2 transport stream data
// with or without Reed-Solomon codes and/or timestampts and then pass
// the aligned data into TSReader. This isn't as efficient as the prior
// sample thread but there's very little additional CPU load to handle
// stream alignment in software. 
//
// NOTE: This method is supported in version 2.6.42 and later.

DWORD WINAPI ReadDataThreadNonAligned(LPVOID lpv)
{
	int nTSBufferIndex = 0;
	BOOL fDSSModeFlag = FALSE;		// Set TRUE for DIRECTV streams
	BYTE * buffer;

	buffer = (BYTE *) LocalAlloc(LPTR, TS_BUFFER_SIZE);
	SourceHelper_StartSyncThread(ss, fDSSModeFlag);
	while (!ss->fTerminateReadThread)
	{
		int nRetLen = MagicReadFunction(buffer, TS_BUFFER_SIZE);
		SourceHelper_SyncData(buffer, nRetLen);
	}
	SourceHelper_StopSyncThread();
	LocalFree(buffer);

	// Time to quit - flag buffers in use as -1000 which tells
	// TSReader's main thread to terminate. If you don't do this
	// TSReader will hang when exiting.
	ss->fReadThreadTerminated = TRUE;
	ss->fTerminateReadThread = FALSE;
	EnterCriticalSection(&ss->csTSBuffersInUse);
	ss->nTSBuffersInUse = -1000;
	LeaveCriticalSection(&ss->csTSBuffersInUse);
	return 0;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////

DWORD WINAPI ReadFX2Thread(LPVOID lpv)
{
	BOOL fDSSModeFlag = FALSE;
	int nReadPtr = 0;
	int nCurrentBuffer;
	LONG nTransferLength;
	BOOL fFirstPacket = TRUE;
	BOOL fRestart = TRUE;
	int nTunerStatusTimer = 65535;

	TRACE("genpix 8PSK: +ReadFX2Thread\n");
	ss->fReadThreadTerminated = FALSE;
	ss->fTerminateReadThread = FALSE;
	
	CCyUSBEndPoint * MPEGInEpt = GpDevice->EndPointOf(0x82);   
	if (!MPEGInEpt)
	{
		ERR("ReadFX2Thread: Couldn't find MPEG in endpoint\n");
		return FALSE;
	}

	SourceHelper_StartSyncThread(ss, fDSSModeFlag);
 
RestartFX2DataThread:
	fRestart = FALSE;
	nTransferLength = READ_FROM_FX2_SIZE;
 
	if (ss->nSymbolRate < 10000)
	{
		nTransferLength /= 4;
		if (ss->nSymbolRate < 5000)
		nTransferLength /= 2;
	}
 
	MPEGInEpt->SetXferSize(nTransferLength);

	// Setup the asynchronous transfer buffers
	
	int nQueueSize = 32;
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
	
	MPEGInEpt->Reset();
//	MPEGInEpt->Abort();
	// Queue-up the first batch of transfer requests
//	GP_EnableMPEGstream(TRUE);
	for (nCurrentBuffer = 0; nCurrentBuffer < nQueueSize; nCurrentBuffer++)
		contexts[nCurrentBuffer] = MPEGInEpt->BeginDataXfer(buffers[nCurrentBuffer], nTransferLength, &inMPEGOvLap[nCurrentBuffer]);	
	GP_EnableMPEGstream(TRUE);

	nCurrentBuffer = 0;
	while (!ss->fTerminateReadThread)
	{
		LONG nReceiveLength = 0;
	 
		if (!MPEGInEpt->WaitForXfer(&inMPEGOvLap[nCurrentBuffer], 2500))
		{
			ERR("genpix 8PSK: WaitForXfer() timed out buffer = %d ***********\n", nCurrentBuffer);
			fRestart = TRUE;
			break;
		}
	 
		if (MPEGInEpt->FinishDataXfer(buffers[nCurrentBuffer], nReceiveLength, &inMPEGOvLap[nCurrentBuffer], contexts[nCurrentBuffer]))
		{
			if (fFirstPacket == TRUE)
				fFirstPacket = FALSE;
			else
			SourceHelper_SyncData(buffers[nCurrentBuffer], nReceiveLength);
		}
		else
			Sleep(5);

		contexts[nCurrentBuffer] = MPEGInEpt->BeginDataXfer(buffers[nCurrentBuffer], nTransferLength, &inMPEGOvLap[nCurrentBuffer]);
		nCurrentBuffer++;
		if (nCurrentBuffer == nQueueSize)
			nCurrentBuffer = 0;
		CheckLockStatus();
	}
	TRACE("genpix 8PSK: left main read loop\n");
	nTunerStatusTimer = 65535; 
//	CheckLockStatus();
 
	MPEGInEpt->Abort();
	for (nCurrentBuffer = 0; nCurrentBuffer < nQueueSize; nCurrentBuffer++)
	{
		// Wait for all the queued requests to be cancelled 
		LONG nReceiveLength = 0;
	 
		MPEGInEpt->FinishDataXfer(buffers[nCurrentBuffer], nReceiveLength, &inMPEGOvLap[nCurrentBuffer], contexts[nCurrentBuffer]);
		CloseHandle(inMPEGOvLap[nCurrentBuffer].hEvent);
		delete [] buffers[nCurrentBuffer];
	}

	delete [] inMPEGOvLap;
	delete [] contexts;
	delete [] buffers;
	
	GP_EnableMPEGstream(FALSE);

	if (fRestart == TRUE && !ss->fTerminateReadThread)
		goto RestartFX2DataThread;
 
	SourceHelper_StopSyncThread();
	ss->fReadThreadTerminated = TRUE;
	ss->fTerminateReadThread = FALSE;
	EnterCriticalSection(&ss->csTSBuffersInUse);
	ss->nTSBuffersInUse = -1000;
	LeaveCriticalSection(&ss->csTSBuffersInUse);
	CloseHandle(ss->hReadDataThread);
	TRACE("genpix 8PSK: -ReadFX2Thread\n");
 
	return 0;
}

// This function is called by TSReader when it wants to start the source so it can receive
// data. This gives you a chance to setup a thread to move the data. 
BOOL TSReader_Start()
{
	DWORD dwThreadID;
	
	TRACE("genpix 8PSK: Entering TSReader_Start()\n");
	ss->hReadDataThread = CreateThread(NULL, 0, ReadFX2Thread, (LPVOID)0, CREATE_SUSPENDED, &dwThreadID);
//	ss->hReadDataThread = CreateThread(NULL, 0, ReadDataThreadAligned, (LPVOID)0, CREATE_SUSPENDED, &dwThreadID);
	//ss->hReadDataThread = CreateThread(NULL, 0, ReadDataThreadNonAligned, (LPVOID)0, CREATE_SUSPENDED, &dwThreadID);
	SourceHelper_SetWorkerThreadPriorities(FALSE);
	SetThreadPriority(ss->hReadDataThread, THREAD_PRIORITY_TIME_CRITICAL);
	TRACE("genpix 8PSK: ReadFX2Thread at THREAD_PRIORITY_TIME_CRITICAL\n");
	ResumeThread(ss->hReadDataThread);
	TRACE("genpix 8PSK: Leaving TSReader_Start()\n");

	return TRUE;
}

// Opposite of the start function - close down the thread before returning
BOOL TSReader_Stop()
{
	ss->fTerminateReadThread = TRUE;
	while (ss->fReadThreadTerminated == FALSE)
		Sleep(50);
	return TRUE;
}


// This function is called when TSReader first loads up the source. Use it to find and initialize
// your hardware. Return FALSE if your hardware didn't initialize.
BOOL TSReader_Init(PSOURCESTRUCT pss)
{
	BOOL fInitialized;
	ss = pss;
	TRACE("genpix 8PSK: +Init()\n");
	if (!fInitDone)
	{
		fInitialized = GP_InitUSBdriver();
		fInitDone = TRUE;
	}
		fInitialized = TRUE;
	// do initialization here
	InitializeCriticalSection(&csSignal);
	TRACE("genpix 8PSK: -Init()\n");
	return fInitialized;
}

// Opposite of the Init() function
BOOL TSReader_DeInit()
{
//	TRACE("genpix 8PSK: +DeInit()\n");
	DeleteCriticalSection(&csSignal);
	fShutDown = TRUE;
//	TRACE("genpix 8PSK: -DeInit()\n");
	return TRUE;
}
#define ADVANCED_SATELLITE
// TSReader calls this function to get the tuner parameters. You can build your own dialog
// or use one of the TSReader standard ones exported in TSReader_SourceHelper.dll
BOOL TSReader_TuneDialog(HWND hWndParent)
{
	if (ss->fDontTune == TRUE)
		fNeedTuneDialog = FALSE;
	if (fNeedTuneDialog)
	{
		ss->fQuietMode = FALSE;
		TRACE("DTVWorks: TSReader_TuneDialog tuning dialog is required\n");
		if (SourceHelper_ADVTuneDialog(hWndParent) == FALSE)
			return FALSE;
	}
	else
	{
		TRACE("DTVWorks: TSReader_TuneDialog tune NOT required\n");
		ss->nFrequency = nFrequency;
		ss->nPolarity = nPolarity;
		ss->nSymbolRate = nSymbolRate;
		ss->nLNBFrequency = nLNBFrequency;
		ss->n22KHz = n22KHz;
		ss->nDiSEqCInput = nDiSEqCInput;
		ss->nCodeRate = nCodeRate;
		ss->nADVModulationMode = nADVModulationMode;
		fNeedTuneDialog = TRUE;
	}
	return TRUE;
}

// This function is used for sources that have demuxes. TSReader will call this function as 
// it needs access to a PID. For devices that return the entire transport stream, this function
// can be ignored.
BOOL TSReader_PIDManagement(BOOL fAdd, int nPID, BOOL fTemporary)
{
	return TRUE;
}

// If using an interface with a demux, this function tells TSReader if the PID is currently
// active (turned on in the demux) or not.
BOOL TSReader_IsPIDActive(int nPID)
{
	return TRUE;
}

int TSReader_GetSyncLossCount(BOOL fReset)
{
	return SourceHelper_GetSyncLossCount(fReset);
}

// This function returns information and cabilities about your source
BOOL TSReader_GetDescription(char * szDescription, char * szCommandLineParameters, BOOL * fCanBeStopped, int * nMaxPIDs, DWORD * dwCapabilities)
{
	if (szDescription != NULL)
		lstrcpy(szDescription, gszSourceName);
	if (szCommandLineParameters != NULL)
		lstrcpy(szCommandLineParameters, "freq pol sr lnbf 22khz mode fec {input}"); 
	if (fCanBeStopped != NULL)
		*fCanBeStopped = TRUE;
	if (nMaxPIDs != NULL)
		*nMaxPIDs = 8192;
	if (dwCapabilities != NULL)
		*dwCapabilities = CAPABILITIES_POWER
						| CAPABILITIES_DISEQC
						| CAPABILITIES_TONEBURST
						| CAPABILITIES_DISEQC_POSITIONER
						| CAPABILITIES_DISH_SWITCH
						| CAPABILITIES_ADV_SATELLITE;
	return TRUE;
}

// This is where you get a chance to parse the command-line and save the parameters which
// can be used later in the TuneDialog function rather than presenting a tune dialog.
// This function is only called with the full-version of TSReader.
BOOL TSReader_ParseCommandLine(PSOURCESTRUCT ss, char * szCommandLine, BOOL fQuiet)
{
	if (lstrlen(szCommandLine))
	{
		int nConversionCount = 0;
		
		SourceHelper_ConvertPolarity(szCommandLine);
		ss->nDiSEqCInput = 0;
		nConversionCount = sscanf(szCommandLine, "%d %d %d %d %d %d %d %d", 
								&nFrequency, 
								&nPolarity, 
								&nSymbolRate, 
								&nLNBFrequency, 
								&n22KHz, 
								&nADVModulationMode, 
								&nCodeRate, 
								&nDiSEqCInput);
		if (nConversionCount < 7)
		{
			if (!fQuiet)
				MessageBox(NULL,
				"Usage for this source: freq pol sr lnbf 22khz mode fec {input}\n"
				"\n"
				"freq = frequency to tune\n"
				"pol = 0 vertical/RHCP 1 horizontal/LHCP\n"
				"sr = symbol rate\n"
				"lnbf = LNB frequency\n"
				"22k = 22KHz tone enable\n"
				"mode = modulation mode (see readme)\n"
				"FEC = code rate selection (see readme)\n"
				"input = select DiSEqC input number (1-4) - optional",
				gszSourceName, MB_OK | MB_ICONSTOP);
			return FALSE;
		}
		fNeedTuneDialog = FALSE;
	}
	else
		fNeedTuneDialog = TRUE;
	return TRUE;
}

// This function will be used in the future for CI-CAMs
BOOL TSReader_SetChannel(int nChannel)
{
	return TRUE;
}

// This function is called at a 1Hz rate and returns a signal report string which
// TSReader then displays in it's main window
BOOL TSReader_GetSignalString(char * szString)
{
	EnterCriticalSection(&csSignal);
	lstrcpy(szString, szLastSignalReport);
	LeaveCriticalSection(&csSignal);
	return TRUE;
}

// This function is called at a 1Hz rate and returns a tune report string which
// TSReader then displays in it's main window
BOOL TSReader_GetTunerString(char * szString)
{
	lstrcpy(szString, szLastTune);
	return TRUE;
}

// This function is called by TSReader when it needs to send a DiSEqC command.
// Only sources that have CAPABILITIES_DISEQC_POSITIONER will get called.
BOOL TSReader_SendDiSEqC(BYTE * bCommand, int nLength)
{
	return GP_SendDISEqCpacket(bCommand, nLength);
//	return TRUE;
}

// Standard DLL load point. Save the hModule for later if you want to get to
// your resources
BOOL APIENTRY DllMain(HANDLE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
    switch( ul_reason_for_call )
	{
    case DLL_PROCESS_ATTACH:
		hInstance = (HINSTANCE)hModule;
		fInitDone = FALSE;
		break;
    case DLL_PROCESS_DETACH:
		if (fShutDown)
			GP_DeInitUSBdriver();
		break;
    }
    return TRUE;
}


///////////////////////////////////////////////////////////////////////////////////////////
//
//    Genpix Functions
//
//////////////////////////////////////////////////////////////////////////////////////////



/////////////////////////////////////////////////////////////////////////////////////////////////
// This function returns the current configuration of 8PSK-2-USB adapter.
// use the bit mask defined in 8psk_VendorCommandCodes.h to decode the status.
//
//	bm8pskStarted	0x01 - power to tuner and receiver On/Off
//	bm8pskFW_Loaded	0x02 - BCM4500 firmware loaded
//	bmIntersilOn	0x04 - power to LNB is On/Off
//	bmDVBmode		0x08 - DVB (1) or DSS (0) mode (packets are synched or not)
//	bm22kHz 		0x10 - current status 22kHz tone output
//	bmSEL18V		0x20 - voltage on LNB: 18V(1) or 13V(0)
//	bmTuned			0x40 - status of the tuner (updated every time the Signal Strength is checked)
//	bmArmed			0x80 - MPEG passthrough is allowed (or not)
//

BYTE GP_Get8pskConfiguration()
{
	BYTE Buf;
	LONG buflen = 1;
//	ept->Target		= TGT_DEVICE;
//	ept->ReqType	= REQ_VENDOR;
	GpDevice->ControlEndPt->Direction	= DIR_FROM_DEVICE;
	GpDevice->ControlEndPt->ReqCode		= GET_8PSK_CONFIG;
	GpDevice->ControlEndPt->XferData(&Buf, buflen);
	TRACE_F(" GP_Configuration: %X\n", Buf);
	return Buf;
}


/////////////////////////////////////////////////////////////////////////////////////////////////
// This function powers Up/Down the 8PSK module (tuner and decoder).
// Returns status of the power (TRUE = Power On, FALSE = Power Off).
//
//  NB! :	This function does NOT affect output of LNB power.
//			LNB power is controlled via Intersil calls.
//
BOOL __fastcall GP_Boot8psk(BOOL Start)
{
	BYTE Buf;
	LONG buflen = 1;
//	GpDevice->ControlEndPt->Target		= TGT_DEVICE;
//	GpDevice->ControlEndPt->ReqType		= REQ_VENDOR;
	GpDevice->ControlEndPt->Direction	= DIR_FROM_DEVICE;
	GpDevice->ControlEndPt->ReqCode		= BOOT_8PSK;
	GpDevice->ControlEndPt->Value		= Start;
	GpDevice->ControlEndPt->XferData(&Buf, buflen);
	ULONG Error = GpDevice->ControlEndPt->UsbdStatus;
	if (Error)
	{
		char ErrMsg[64];
		GpDevice->UsbdStatusString(Error, ErrMsg);
		ERR("GP_Boot8psk: error 0x%lx (\"%s\")\n", Error, ErrMsg);
		return FALSE;
	}
	else
	{
		if (Buf)
			TRACE(" 8PSK module: +Power\n");
		else
			TRACE(" 8PSK module: -Power\n");
	}
	return (BOOL)Buf;
}

/////////////////////////////////////////////////////////////////////////////////////////////////
//	This function reads from I2C device (at provided address).
//	optimized for BCM4500 communication.
//
BOOL __fastcall GP_I2Cread(DWORD I2C_Address, DWORD Register, BYTE *buffer, int length)
{
	LONG buflen = (LONG) length;
//	GpDevice->ControlEndPt->Target		= TGT_DEVICE;
//	GpDevice->ControlEndPt->ReqType		= REQ_VENDOR;
	GpDevice->ControlEndPt->Direction	= DIR_FROM_DEVICE;
	GpDevice->ControlEndPt->ReqCode		= I2C_READ;
	GpDevice->ControlEndPt->Value		= (WORD) I2C_Address;
	GpDevice->ControlEndPt->Index		= (WORD) Register;
	GpDevice->ControlEndPt->XferData(buffer, buflen);
	ULONG Error = GpDevice->ControlEndPt->UsbdStatus;
	if (Error)
	{
		char ErrMsg[64];
		GpDevice->UsbdStatusString(Error, ErrMsg);
		ERR("GP_I2Cread: error 0x%lx (\"%s\")\n", Error, ErrMsg);
		return FALSE;
	}
	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////////////////////////
//	This function writes to I2C device (at provided address).
//	optimized for BCM4500 communication.
//
BOOL __fastcall GP_I2Cwrite(DWORD I2C_Address, DWORD Register, BYTE *buffer, int length)
{
	LONG buflen = (LONG) length;
//	GpDevice->ControlEndPt->Target		= TGT_DEVICE;
//	GpDevice->ControlEndPt->ReqType		= REQ_VENDOR;
	GpDevice->ControlEndPt->Direction	= DIR_TO_DEVICE;
	GpDevice->ControlEndPt->ReqCode		= I2C_WRITE;
	GpDevice->ControlEndPt->Value		= (WORD) I2C_Address;
	GpDevice->ControlEndPt->Index		= (WORD) Register;
	GpDevice->ControlEndPt->XferData(buffer, buflen);
	ULONG Error = GpDevice->ControlEndPt->UsbdStatus;
	if (Error)
	{
		char ErrMsg[64];
		GpDevice->UsbdStatusString(Error, ErrMsg);
		ERR("GP_I2Cwrite: error 0x%lx (\"%s\")\n", Error, ErrMsg);
		return FALSE;
	}
	return TRUE;
}

BOOL __fastcall GP_EnableMPEGstream(BOOL Start)
{
	LONG buflen = 0;
//	GpDevice->ControlEndPt->Target		= TGT_DEVICE;
//	GpDevice->ControlEndPt->ReqType		= REQ_VENDOR;
	GpDevice->ControlEndPt->Direction	= DIR_TO_DEVICE;
	GpDevice->ControlEndPt->ReqCode		= ARM_TRANSFER;
	GpDevice->ControlEndPt->Value		= Start;
	GpDevice->ControlEndPt->XferData(NULL, buflen);
	ULONG Error = GpDevice->ControlEndPt->UsbdStatus;
	if (Error)
	{
		char ErrMsg[64];
		GpDevice->UsbdStatusString(Error, ErrMsg);
		ERR("GP_EnableMPEGstream: error 0x%lx (\"%s\")\n", Error, ErrMsg);
		return FALSE;
	}
	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////////////////////////
//	This function sends tune parameters to 8PSK module:
//	SymbolRate, Frequency, ModulationMode, and CodeRate - total 10 bytes
BOOL __fastcall GP_Tune8PSKmodule(TuneParameters *TP)
{
	LONG buflen = 10;
//	GpDevice->ControlEndPt->Target		= TGT_DEVICE;
//	GpDevice->ControlEndPt->ReqType		= REQ_VENDOR;
	GpDevice->ControlEndPt->Direction	= DIR_TO_DEVICE;
	GpDevice->ControlEndPt->ReqCode		= TUNE_8PSK;
	GpDevice->ControlEndPt->XferData((BYTE *) TP, buflen);
	ULONG Error = GpDevice->ControlEndPt->UsbdStatus;
	if (Error)
	{
		char ErrMsg[64];
		GpDevice->UsbdStatusString(Error, ErrMsg);
		ERR("GP_Tune8PSKmodule: error 0x%lx (\"%s\")\n", Error, ErrMsg);
		return FALSE;
	}
	return TRUE;
}

////////////////////////////////////////////////////////////////////////////////////////////////
//	this function returns Strength of the sigal (SNR)
//	units = ...*256dB
//
//	to spead up the response function returns SNR measured during previous request
//	and arms the FX2 to measure it again in spare time.
//	So, signal strength is always one cyle behind
static int __fastcall GP_SignalStrength()
{
	WORD SNR;
	LONG buflen = 2;
//	GpDevice->ControlEndPt->Target		= TGT_DEVICE;
//	GpDevice->ControlEndPt->ReqType		= REQ_VENDOR;
	GpDevice->ControlEndPt->Direction	= DIR_FROM_DEVICE;
	GpDevice->ControlEndPt->ReqCode		= GET_SIGNAL_STRENGTH;
	GpDevice->ControlEndPt->XferData((BYTE *) &SNR, buflen);
	ULONG Error = GpDevice->ControlEndPt->UsbdStatus;
	if (Error)
	{
		char ErrMsg[64];
		GpDevice->UsbdStatusString(Error, ErrMsg);
		ERR("GP_SignalStrength: error 0x%lx (\"%s\")\n", Error, ErrMsg);
		return 0;
	}
	return (int) SNR;
}

/////////////////////////////////////////////////////////////////////////////////////////////////
// This function powers Up/Down the LNB (controller is on adapter board, not on 8PSK module).
//
// Returns status of the command (NOT the status of the LNB power):
// TRUE = i2c command was succesful
// FALSE = error during Intersil programming
//
//  NB! :	This function does NOT affect the power of tuner and receiver.
//			8PSK power is cintrolled via GP_Boot8psk() calls.
//
BOOL __fastcall GP_Intersil(BOOL Start)
{
	BYTE Buf;
	LONG buflen = 1;
//	GpDevice->ControlEndPt->Target		= TGT_DEVICE;
//	GpDevice->ControlEndPt->ReqType		= REQ_VENDOR;
	GpDevice->ControlEndPt->Direction	= DIR_FROM_DEVICE;
	GpDevice->ControlEndPt->ReqCode		= START_INTERSIL;
	GpDevice->ControlEndPt->Value		= Start;
	GpDevice->ControlEndPt->XferData(&Buf, buflen);
	ULONG Error = GpDevice->ControlEndPt->UsbdStatus;
	if (Error)
	{
		char ErrMsg[64];
		GpDevice->UsbdStatusString(Error, ErrMsg);
		ERR("GP_Intersil: error 0x%lx (\"%s\")\n", Error, ErrMsg);
		return FALSE;
	}
	else
	{
		if (Start)
			TRACE(" LNB: +Power\n");
		else
			TRACE(" LNB: -Power\n");
	}
	return (BOOL) Buf;
}

/////////////////////////////////////////////////////////////////////////////////////////////////
// This function changes the level of LNB power: 1 = 18V, 0 = 13V.
// This function has no affect on actual voltage if Intersil controller is not powered.
//
// Returns status of the command (NOT the level of LNB power):
// TRUE = USB command was accepted
// FALSE = error during USB communication
//
BOOL __fastcall GP_SetLNBVoltage(BOOL SetHighVoltage)
{
	LONG buflen = 0;
//	GpDevice->ControlEndPt->Target		= TGT_DEVICE;
//	GpDevice->ControlEndPt->ReqType		= REQ_VENDOR;
	GpDevice->ControlEndPt->Direction	= DIR_TO_DEVICE;
	GpDevice->ControlEndPt->ReqCode		= SET_LNB_VOLTAGE;
	GpDevice->ControlEndPt->Value		= SetHighVoltage;
	GpDevice->ControlEndPt->XferData(NULL, buflen);
	ULONG Error = GpDevice->ControlEndPt->UsbdStatus;
	if (Error)
	{
		char ErrMsg[64];
		GpDevice->UsbdStatusString(Error, ErrMsg);
		ERR("GP_SetLNBVoltage: error 0x%lx (\"%s\")\n", Error, ErrMsg);
		return FALSE;
	}
	else
	{
		if (SetHighVoltage)
			TRACE(" LNB: 18V\n");
		else
			TRACE(" LNB: 13V\n");
	}
	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////////////////////////
// This function switches On nad Off the 22kHz output.
// This function has no affect if Intersil controller is not powered.
//
// Returns status of the command (NOT the status of 22KHz tone):
// TRUE = USB command was accepted
// FALSE = error during USB communication
//
BOOL __fastcall GP_Set22kHzTone(BOOL Start)
{
	LONG buflen = 0;
//	GpDevice->ControlEndPt->Target		= TGT_DEVICE;
//	GpDevice->ControlEndPt->ReqType		= REQ_VENDOR;
	GpDevice->ControlEndPt->Direction	= DIR_TO_DEVICE;
	GpDevice->ControlEndPt->ReqCode		= SET_22KHZ_TONE;
	GpDevice->ControlEndPt->Value		= Start;
	GpDevice->ControlEndPt->XferData(NULL, buflen);
	ULONG Error = GpDevice->ControlEndPt->UsbdStatus;
	if (Error)
	{
		char ErrMsg[64];
		GpDevice->UsbdStatusString(Error, ErrMsg);
		ERR("GP_Set22KHzTone: error 0x%lx (\"%s\")\n", Error, ErrMsg);
		return FALSE;
	}
	else
	{
		if (Start)
			TRACE(" 22kHz: ON\n");
		else 
			TRACE(" 22kHz: off\n");
	}
	return TRUE;
}

////////////////////////////////////////////////////////////////////////////////////////////////
// This function sends DISEqC packet (several bytes)
// It waites for 15ms before sending bytes. 
// Thus, you don't need to use any delays between packets.
// 
// If length = 0 function sends ToneBurst (type of burst is specified by Buf[0]
// 
// NB!	it sets LNB power to 13V and
//		switches off 22kHz tone (if it was ON)
//		You have to restore them if you need.
//
// Returns status of USB command (NOT the status of DISEqC command):
// TRUE = USB command was accepted
// FALSE = error during USB communication
//
BOOL __fastcall GP_SendDISEqCpacket(BYTE *Buf, int length)
{
	LONG buflen = (LONG) length;
//	GpDevice->ControlEndPt->Target		= TGT_DEVICE;
//	GpDevice->ControlEndPt->ReqType		= REQ_VENDOR;
	GpDevice->ControlEndPt->Direction	= DIR_TO_DEVICE;
	GpDevice->ControlEndPt->ReqCode		= SEND_DISEQC_COMMAND;
	GpDevice->ControlEndPt->Value		= Buf[0];
	GpDevice->ControlEndPt->XferData(Buf, (LONG) buflen);
	ULONG Error = GpDevice->ControlEndPt->UsbdStatus;
	if (Error)
	{
		char ErrMsg[64];
		GpDevice->UsbdStatusString(Error, ErrMsg);
		ERR("GP_SendDISEqC: error 0x%lx (\"%s\")\n", Error, ErrMsg);
		return FALSE;
	}
	return TRUE;
}

static BYTE Diseqc4x1[4] = {0xE0, 0x10, 0x38, 0x00};

BOOL __fastcall GP_SelectDISEqCinput(int n)
{
	if (n < 1) 
		n = 1;
	else
		if (n > 4)
			n = 4;
	Diseqc4x1[3] = (BYTE) 0xF0 + 4*(n-1);
	return (GP_SendDISEqCpacket(Diseqc4x1, 4));
}

////////////////////////////////////////////////////////////////////////////////////////////////
// This function switches USB adapter between DVB (1) and DSS (0) modes.
// In DSS mode, entier MPEG stream is passed to the host.
// In DVB mode, adapter starts transmission synched to the first SYNC sygnal,
// writes (CLK) are enabled by VALID signal (downcobversion to 188-packets).
//
// Returns status of the command (NOT the status of DVB mode):
// TRUE = USB command was accepted
// FALSE = error during USB communication
//
BOOL __fastcall GP_SetDVBmode(BOOL DVB)
{
	LONG buflen = 0;
//	GpDevice->ControlEndPt->Target		= TGT_DEVICE;
//	GpDevice->ControlEndPt->ReqType		= REQ_VENDOR;
	GpDevice->ControlEndPt->Direction	= DIR_TO_DEVICE;
	GpDevice->ControlEndPt->ReqCode		= SET_DVB_MODE;
	GpDevice->ControlEndPt->Value		= DVB;
	GpDevice->ControlEndPt->XferData(NULL, buflen);
	ULONG Error = GpDevice->ControlEndPt->UsbdStatus;
	if (Error)
	{
		char ErrMsg[64];
		GpDevice->UsbdStatusString(Error, ErrMsg);
		ERR("GP_SetDVBmode: error 0x%lx (\"%s\")\n", Error, ErrMsg);
		return FALSE;
	}
/*	
	else
	{
		if (DVB)
			TRACE("+DVB mode is selected\n");
		else
			TRACE("-DSS mode is selected\n");
	}
*/	
	return TRUE;
}


/////////////////////////////////////////////////////////////////////////////////////////////////
//	This function sends serial command over 13/18V LNB line to control legacy Dishnetwork switches
BOOL __fastcall GP_SelectDNinput(BYTE Data)
{
	LONG buflen = 0;
//	GpDevice->ControlEndPt->Target		= TGT_DEVICE;
//	GpDevice->ControlEndPt->ReqType		= REQ_VENDOR;
	GpDevice->ControlEndPt->Direction	= DIR_TO_DEVICE;
	GpDevice->ControlEndPt->ReqCode		= SET_DN_SWITCH;
	GpDevice->ControlEndPt->Value		= Data;
	GpDevice->ControlEndPt->XferData(NULL, buflen);
	ULONG Error = GpDevice->ControlEndPt->UsbdStatus;
	if (Error)
	{
		char ErrMsg[64];
		GpDevice->UsbdStatusString(Error, ErrMsg);
		ERR("GP_SelectDNinput: error 0x%lx (\"%s\")\n", Error, ErrMsg);
		return FALSE;
	}
	return TRUE;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
// function loads BCM4500 firmware through EP1OUT interface
// returns: 
//	TRUE = load successful
//	FALSE = load failed
//
static BOOL GP_LoadBCM4500fw()
{
	LONG buflen = 0;
	BYTE * ptr = BCM4500_firmware;
//	GpDevice->ControlEndPt->Target		= TGT_DEVICE;
//	GpDevice->ControlEndPt->ReqType		= REQ_VENDOR;
	GpDevice->ControlEndPt->Direction	= DIR_TO_DEVICE;
	GpDevice->ControlEndPt->ReqCode		= LOAD_BCM4500;
	// next commands ARMs the USB-firmware to receive data through the EP1OUT
	GpDevice->ControlEndPt->XferData(NULL, buflen);
	ULONG Error = GpDevice->ControlEndPt->UsbdStatus;
	if (Error)
	{
        char ErrMsg[64];
		GpDevice->UsbdStatusString(Error, ErrMsg);
		ERR("GP_LoadBCM4500firmware: initialization error 0x%lx (\"%s\")\n", Error, ErrMsg);
        return FALSE;
    }
	while (ptr[0] != 0xFF)
	{
		buflen = ptr[0] + 4;
		GpDevice->EndPoints[1]->XferData(ptr, buflen);
	//	TRACE_F("pipe: %i, transfered: %u bytes\n", 1, buflen);
		ULONG Error = GpDevice->EndPoints[1]->UsbdStatus;
		if (Error)
		{
			char ErrMsg[64];
			GpDevice->UsbdStatusString(Error, ErrMsg);
			ERR("GP_LoadBCM4500firmware: pipe error 0x%lx (\"%s\")\n", Error, ErrMsg);
			return FALSE;
		}
		ptr += buflen;
	}
	return (BOOL) (GP_Get8pskConfiguration() & bm8pskFW_Loaded);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
//	these are some functon which are very close to IRD6000 polling functions.
//	though, their meaning is pure speculation.
//	In reality, they could serve some other purpose
//

static BOOL GP_BCM4500Locked()
{
	BYTE B;
	GP_I2Cread(0x10, 0xA4, &B, 1);
	return (BOOL) (B & 0x20);
}

//////////////////////////////////////////////////////////////////////////////////////////////////
//	Tuning to TP
static BOOL GP_Tune()
{
	BYTE B;

	TuneParameters TP;

	if (ss->nDiSEqCInput < 21)
		switch (ss->nDiSEqCInput)
		{
			case 0:
				break;
			case 1:
			case 2:
			case 3:
			case 4:
				GP_SelectDISEqCinput(ss->nDiSEqCInput);
				break;
			case 5:
				B = 0;
				GP_SendDISEqCpacket(&B, 0);
				break;
			case 6:
				B = 1;
				GP_SendDISEqCpacket(&B, 0);
				break;
			default:
				GP_SelectDNinput(bDishBytes[ss->nDiSEqCInput - 7]);
				break;
		}
	TP.SymbolRate		= (DWORD)  ss->nSymbolRate *1000;
	if (ss->nFrequency > ss->nLNBFrequency)
		TP.Frequency		= (DWORD) (ss->nFrequency - ss->nLNBFrequency) * 1000000;
	else
		TP.Frequency		= (DWORD) (ss->nLNBFrequency - ss->nFrequency) * 1000000;	
	TP.ModulationMode	= (BYTE)   ss->nADVModulationMode;
	TP.CodeRate			= (BYTE)   ss->nCodeRate;

	BOOL IntersilOn = (BOOL) (GP_Get8pskConfiguration() & bmIntersilOn);
	if (ss->nPolarity == -1)
	{
		if (IntersilOn)
			GP_Intersil(FALSE);
	}
	else
	{
		if (!IntersilOn)
			GP_Intersil(TRUE);
		GP_SetLNBVoltage(ss->nPolarity);
	}
	GP_Set22kHzTone(n22KHz);
	GP_SetDVBmode(TRUE);
	GP_Tune8PSKmodule(&TP);
	return TRUE;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
// This function is called when TSReader first loads up the source (through Init call). Use it to find 
// GENEPIX hardware. Return FALSE if hardware was not found, or firmware can not be loaded.
static BOOL GP_InitUSBdriver()
{
	TRACE("GP: +Init()\n");
	
	GpDevice = new CCyUSBDevice(NULL);
	int devices = GpDevice->DeviceCount();
	if (devices == 0) 
		goto Exit;
	int d = 0;
	int pID = 0;
	int vID = 0;
	while ((d < devices) && (vID != GP8PSK_VID) && (pID != GP8PSK_PID)) 
	{
		GpDevice->Open(d);
		if (!GpDevice->IsOpen())
		{  // Try a reset
			GpDevice->Reset();
			Sleep(1000);
			GpDevice->Open(d);
		}
		vID = GpDevice->VendorID;
		pID = GpDevice->ProductID;
		d++;
    }

	if (GpDevice->IsOpen() & (vID == GP8PSK_VID) & (pID == GP8PSK_PID))
	{
		// all requests via EP0 are Vendor commands
		GpDevice->ControlEndPt->ReqType	= REQ_VENDOR;
		GpDevice->ControlEndPt->Target	= TGT_DEVICE;
	}
	else
		goto Exit;

	BYTE Config = GP_Get8pskConfiguration();
	if (!(Config & bm8pskStarted))
		GP_Boot8psk(TRUE);
	if (!(Config & bm8pskFW_Loaded))
		if (!GP_LoadBCM4500fw())
		{
			//MessageBox(ss->hWndTSReader, "Failed to load firmware", gszSourceName, MB_ICONSTOP);
			ERR("GP: failed to load firmware\n");
			goto Exit;
		}
	if (!(Config & bmIntersilOn))
		GP_Intersil(TRUE);
	GP_SetDVBmode(TRUE);

	fShutDown = FALSE;

	TRACE("GP: -Init()\n");
	return TRUE;

Exit:
    delete GpDevice;
    return FALSE;
}

// Opposite of the Init() function
void GP_DeInitUSBdriver()
{
	GP_Intersil(FALSE);
	GP_Boot8psk(FALSE);
	delete GpDevice;
}
