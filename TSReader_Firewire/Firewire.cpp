// SampleSource.c
// Copyright (C) 2004 COOLSTF.com Inc.
//
// Sample source for TSReader
//

#define _WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "..\sources.h"
#include "dctrecord.h"

PSOURCESTRUCT ss;
char szLastSignalReport[128] = {"n/a"};        
char szLastTune[256] = {"n/a"};
char gszSourceName[] = {"Firewire Interface"};
CAPDEVICES gDevices[MAX_DEVICES];
int gDeviceCount;

// from dctrecord.cpp

extern "C" {
    BOOL __cdecl SourceHelper_ValidateSourceContainer(PSOURCESTRUCT pss);
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
    return TRUE;
}

DWORD WINAPI ReadFirewireThread(LPVOID lpv)
{
    CoInitialize(NULL);
    int nRetVal = dctrecord();
    CoUninitialize();

    ss->fReadThreadTerminated = TRUE;
    ss->fTerminateReadThread = FALSE;
    EnterCriticalSection(&ss->csTSBuffersInUse);
    ss->nTSBuffersInUse = -1000;
    LeaveCriticalSection(&ss->csTSBuffersInUse);

    return 0;
}

// This function is called by TSReader when it wants to start the source so it can receive
// data. This gives you a chance to setup a thread to move the data
BOOL TSReader_Start()
{
    DWORD dwThreadID;

	SourceHelper_StartSyncThread(ss, FALSE);

    ss->hReadDataThread = CreateThread(NULL, 0, ReadFirewireThread, (LPVOID)0, CREATE_SUSPENDED, &dwThreadID);
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
	SourceHelper_StopSyncThread();
    return TRUE;
}

// This function is called when TSReader first loads up the source. Use it to find and initialize
// your hardware. Return FALSE if your hardware didn't initialize.
BOOL TSReader_Init(PSOURCESTRUCT pss)
{
    if (SourceHelper_ValidateSourceContainer(pss) == FALSE)
	return FALSE;

    CoInitialize(NULL);

    if (EnumCaptureSources() != S_OK)
	return FALSE;

    if (gDeviceCount == 0)
	return FALSE;

    if (pss->nSourceIndex > gDeviceCount - 1 || pss->nSourceIndex >= MAX_DEVICES)
	return FALSE;

    ss = pss;
    return TRUE;
}

// Opposite of the Init() function
BOOL TSReader_DeInit()
{
    CoUninitialize();
    return TRUE;
}

// TSReader calls this function to get the tuner parameters. You can build your own dialog
// or use one of the TSReader standard ones exported in TSReader_SourceHelper.dll
BOOL TSReader_TuneDialog(HWND hWndParent)
{
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
		*dwCapabilities = CAPABILITIES_MULTICARD | CAPABILITIES_TUNE_BY_CHANNEL;
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
