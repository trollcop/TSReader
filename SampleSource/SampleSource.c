// SampleSource.c
// Copyright (C) 2004-2005 COOLSTF.com Inc.
//
// Sample source for TSReader
//

#include <windows.h>
#include "inc\sources.h"

PSOURCESTRUCT ss;
HANDLE hInstance;
char szLastSignalReport[128] = {0};        
char szLastTune[128] = {0};
char gszSourceName[] = {"TSReader Sample Source"};

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
	BOOL fLocked = TRUE;	// we're always locked!

	if (ss->fSerialReceiverControlEnabled)
	{
		if (SourceHelper_TuneSerialControl(szLastTune) == FALSE)
			return FALSE;
	}

	if (ss->fQuietMode == FALSE)
	{
		if (!fLocked)
		{
			MessageBox(ss->hWndTSReader, "Failed to lock", gszSourceName, MB_ICONSTOP);
			return FALSE;
		}
	}
	
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

	buffer = LocalAlloc(LPTR, TS_BUFFER_SIZE);
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

// This function is called by TSReader when it wants to start the source so it can receive
// data. This gives you a chance to setup a thread to move the data. 
BOOL TSReader_Start()
{
	DWORD dwThreadID;
	
	ss->hReadDataThread = CreateThread(NULL, 0, ReadDataThreadAligned, (LPVOID)0, CREATE_SUSPENDED, &dwThreadID);
	//ss->hReadDataThread = CreateThread(NULL, 0, ReadDataThreadNonAligned, (LPVOID)0, CREATE_SUSPENDED, &dwThreadID);
	SourceHelper_SetWorkerThreadPriorities(FALSE);
	ResumeThread(ss->hReadDataThread);

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
	ss = pss;
	return TRUE;
}

// Opposite of the Init() function
BOOL TSReader_DeInit()
{
	return TRUE;
}

// TSReader calls this function to get the tuner parameters. You can build your own dialog
// or use one of the TSReader standard ones exported in TSReader_SourceHelper.dll
BOOL TSReader_TuneDialog(HWND hWndParent)
{
#ifdef ATSC
		if (SourceHelper_ATSCTuneDialog(hWndParent) == FALSE)					return FALSE;
#endif ATSC
#ifdef DVBS
		if (SourceHelper_DVBSTuneDialog(hWndParent) == FALSE)					return FALSE;
#endif DVBS
#ifdef DVBT
		if (SourceHelper_DVBTTuneDialog(hWndParent) == FALSE)					return FALSE;
#endif DVBT
#ifdef DVBC
		if (SourceHelper_DVBCTuneDialog(hWndParent) == FALSE)					return FALSE;
#endif DVBC
#ifdef DSS
		if (SourceHelper_DSSTuneDialog(hWndParent) == FALSE)					return FALSE;
#endif DSS
#ifdef ADVANCED_SATELLITE
		if (SourceHelper_ADVTuneDialog(hWndParent) == FALSE)					return FALSE;
#endif ADVANCED_SATELLITE
#ifdef QAM
		if (SourceHelper_QAMTuneDialog(hWndParent) == FALSE)					return FALSE;
#endif QAM
#ifdef UDPMULTICAST
		if (SourceHelper_UDPMulticastTuneDialog(hWndParent) == FALSE)			return FALSE;
#endif UDPMULTICAST

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

// This function returns information and cabilities about your source
BOOL TSReader_GetDescription(char * szDescription, char * szCommandLineParameters, BOOL * fCanBeStopped, int * nMaxPIDs, DWORD * dwCapabilities)
{
	if (szDescription != NULL)
		lstrcpy(szDescription, gszSourceName);
	if (fCanBeStopped != NULL)
		*fCanBeStopped = FALSE;
	if (nMaxPIDs != NULL)
		*nMaxPIDs = 8192;
	if (szCommandLineParameters != NULL)
		lstrcpy(szCommandLineParameters, "n/a");	
	if (dwCapabilities != NULL)
		*dwCapabilities = CAPABILITIES_POWER
		                | CAPABILITIES_DISEQC
						| CAPABILITIES_TONEBURST
						| CAPABILITIES_DISEQC_POSITIONER;
	return TRUE;
}

// This is where you get a chance to parse the command-line and save the parameters which
// can be used later in the TuneDialog function rather than presenting a tune dialog.
// This function is only called with the full-version of TSReader.
BOOL TSReader_ParseCommandLine(PSOURCESTRUCT ss, char * szCommandLine, BOOL fQuiet)
{
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
	lstrcpy(szString, szLastSignalReport);
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
	return TRUE;
}

// Standard DLL load point. Save the hModule for later if you want to get to
// your resources
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
