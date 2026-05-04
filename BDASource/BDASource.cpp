// BDASource.cpp
//
// BDA DVB-T plugin for TSReader
// by Jeremy Quirke 2004-2005
//
// Based on:
// Sample source for TSReader
// Copyright (C) 2004 COOLSTF.com Inc.
//
// Sync Helper functions added in by 
// C Helliwell (LN Systems Limited) Sep 2006

#include <windows.h>
#include "../sources.h"
#include "BDASource.h"

PSOURCESTRUCT ss;
HANDLE hInstance;
BOOL fNeedTuneDialog = TRUE;
int nFrequency;
BOOL fSpectrumInversion;
int nBandwidth;

char szLastSignalReport[128] = {0};        
char szLastTune[128] = {0};

// Define this for hardware which does not sync the data stream to the buffer
#define _TSREADER_SOURCEHELPER_SYNC	1

#ifdef DVBT
char gszSourceName[] = {"DVB-T BDA compatible source"};
#elif
char gszSourceName[] = {"BDA compatible source"};
#endif

BDAGraph *bdagraph = NULL; // the BDA Graph object


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
	int bandwidths[] = {6,7,8};

	if (!bdagraph)
		return FALSE;

	if (ss->nBandwidth < 0 || ss->nBandwidth > 2)
		return FALSE;

	if (bdagraph->TuneDVB(ss->nFrequency, bandwidths[ss->nBandwidth], -1, -1, -1))
	{
		if (ss->fQuietMode == FALSE)
			MessageBox(ss->hWndTSReader, "Failed to lock signal", gszSourceName, MB_OK | MB_ICONWARNING);
		return FALSE;
	}

	return TRUE;
}

// JQ: called by the DirectShow Samplegrabber filter when data is to be received

int nTSBufferIndex = 0;

int receiveData(double time, BYTE *buffer, long len, LPVOID context)
{
#ifdef	_TSREADER_SOURCEHELPER_SYNC
	BOOL fSuccess = SourceHelper_SyncData(buffer, len);
#else
	PSOURCESTRUCT source = (PSOURCESTRUCT) context;

	int remaining = len, chunk;

	while(remaining && !source->fTerminateReadThread)
	{
		chunk = (remaining > TS_BUFFER_SIZE) ? TS_BUFFER_SIZE : remaining;

		EnterCriticalSection(&source->csPIDCounter);
		source->nLastSecondByteCounter += chunk;
		LeaveCriticalSection(&source->csPIDCounter);

		CopyMemory(source->tsb[nTSBufferIndex].pData, buffer, chunk);
		source->tsb[nTSBufferIndex++].nSize = chunk;
		nTSBufferIndex %= MAX_TS_BUFFERS;

		EnterCriticalSection(&source->csTSBuffersInUse);
		source->nTSBuffersInUse++;
		LeaveCriticalSection(&source->csTSBuffersInUse);
		
		remaining-=chunk;
	}
#endif

	return 0;
}

// This function is called by TSReader when it wants to start the source so it can receive
// data. This gives you a chance to setup a thread to move the data

// JQ: We don't create a thread as that is handled by DirectShow

BOOL TSReader_Start()
{
	if (!bdagraph)
		return FALSE;

	if (bdagraph->Play())
		return FALSE;

	ss->hReadDataThread = NULL; // don't need this
	ss->fTerminateReadThread = FALSE;

	return TRUE;
}

// Opposite of the start function - close down the thread before returning

BOOL TSReader_Stop()
{
	ss->fTerminateReadThread = TRUE;

	if (!bdagraph)
		return FALSE;

	if (bdagraph->Stop())
		return FALSE;

	ss->fReadThreadTerminated = TRUE;
	ss->fTerminateReadThread = FALSE;

	EnterCriticalSection(&ss->csTSBuffersInUse);
	ss->nTSBuffersInUse = -1000;
	LeaveCriticalSection(&ss->csTSBuffersInUse);

	return TRUE;
}

// This function is called when TSReader first loads up the source. Use it to find and initialize
// your hardware. Return FALSE if your hardware didn't initialize.

BOOL TSReader_Init(PSOURCESTRUCT pss)
{
	BOOL res = TRUE;
	ss=pss;
	nTSBufferIndex = 0;

	if (FAILED(CoInitialize(NULL)))
		goto cleanup_NO;

	bdagraph = new /*(std::nothrow)*/ BDAGraph;

	if (!bdagraph)	
		goto cleanup_CO;

	if (bdagraph->Create(NT_DVBT, &receiveData, (LPVOID) ss))
	{
		if (!ss->fQuietMode)
			MessageBox(NULL, "Cannot create a DVBT filter graph", "Error", MB_OK|MB_ICONERROR);
		goto cleanup_BG;
	}

#ifdef _TSREADER_SOURCEHELPER_SYNC
	res = SourceHelper_StartSyncThread(pss, FALSE);
#endif
	return res;

cleanup_BG:

	delete bdagraph;
	bdagraph = NULL;

cleanup_CO:

	CoUninitialize();

cleanup_NO:

	return FALSE;
}

// Opposite of the Init() function
BOOL TSReader_DeInit()
{
	BOOL res = TRUE;

	if (bdagraph)
	{
		delete bdagraph;
		bdagraph = NULL;
	}

	CoUninitialize();

	ss = NULL;

#ifdef _TSREADER_SOURCEHELPER_SYNC
	res =  SourceHelper_StopSyncThread();
#endif
	return res;
}

// TSReader calls this function to get the tuner parameters. You can build your own dialog
// or use one of the TSReader standard ones exported in TSReader_SourceHelper.dll
BOOL TSReader_TuneDialog(HWND hWndParent)
{
	if (fNeedTuneDialog)
	{
		if (SourceHelper_DVBTTuneDialog(hWndParent) == FALSE)
			return FALSE;
	}
	else
	{
		ss->nFrequency = nFrequency;
		ss->fSpectrumInversion = fSpectrumInversion;
		ss->nBandwidth = nBandwidth;
	}
	return TRUE;
}

// This function is used for sources that have demuxes. TSReader will call this function as 
// it needs access to a PID. For devices that return the entire transport stream, this function
// can be ignored.
BOOL TSReader_PIDManagement(BOOL fAdd, int nPID, BOOL fTemporary)
{
	// No DeMux
	return TRUE;
}

// If using an interface with a demux, this function tells TSReader if the PID is currently
// active (turned on in the demux) or not.
BOOL TSReader_IsPIDActive(int nPID)
{
	// No DeMux
	return TRUE;
}

// This function returns information and cabilities about your source
BOOL TSReader_GetDescription(char * szDescription, char * szCommandLineParameters, BOOL * fCanBeStopped, int * nMaxPIDs, DWORD * dwCapabilities)
{
	if (szDescription != NULL)
		lstrcpy(szDescription, gszSourceName);
	if (fCanBeStopped != NULL)
		*fCanBeStopped = TRUE;
	if (nMaxPIDs != NULL)
		*nMaxPIDs = 8192;
	if (szCommandLineParameters != NULL)
		lstrcpy(szCommandLineParameters, "freq inversion bandwidth");
	if (dwCapabilities != NULL)
		*dwCapabilities = 0;
	return TRUE;
}

// This is where you get a chance to parse the command-line and save the parameters which
// can be used later in the TuneDialog function rather than presenting a tune dialog.
// This function is only called with the full-version of TSReader.
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

// This function will be used in the future for CI-CAMs
BOOL TSReader_SetChannel(int nChannel)
{
	// not supported
	return TRUE;
}

// This function is called at a 1Hz rate and returns a signal report string which
// TSReader then displays in it's main window
BOOL TSReader_GetSignalString(char * szString)
{
	LONG TuneStatus=1;
	BOOLEAN extra, locked;
	LONG strength, quality;
	
	if (bdagraph)
	{
		if (!bdagraph->GetTuneStatus(&TuneStatus, &extra, &locked, &quality, &strength))
		{
			if (TuneStatus == -1)
				wsprintf(szString, "Unknown");
			else 
			{
				if (TuneStatus == 0)
					wsprintf(szString, "Not tuned");
				else
				{
					if (extra)
						wsprintf(szString, "%d Quality: %d%%",
							(int)strength, (int) quality);
					else
						wsprintf(szString, "%d", (int) TuneStatus);
				}
			}
		}
		else
			wsprintf(szString, "Unknown");
	}
	else
		wsprintf(szString, "No tuner loaded");

	return TRUE;
}

// This function is called at a 1Hz rate and returns a tune report string which
// TSReader then displays in it's main window
BOOL TSReader_GetTunerString(char * szString)
{
	LONG TuneStatus=1;
	BOOLEAN extra, locked;
	LONG strength, quality;

	char devname[256] = "Unknown";

	LPWSTR pdevname;

	if (!bdagraph->GetDeviceName(&pdevname))
	{
		wcstombs(devname, pdevname, 256);
	}

	if (bdagraph)
		if (!bdagraph->GetTuneStatus(&TuneStatus, &extra, &locked, &quality, &strength))
			if (extra)
				if (locked)
					wsprintf(szString, "%s: Locked %d.%d MHz", devname, ss->nFrequency/1000, ss->nFrequency%1000);
				else
					wsprintf(szString, "%s: No signal", devname);
					
			else
				if (TuneStatus > 0)
					wsprintf(szString, "%s: Locked %d.%d MHz", devname, ss->nFrequency/1000, ss->nFrequency%1000);
				else if (TuneStatus == 0)
					wsprintf(szString, "%s: Not tuned", devname);
				else
					wsprintf(szString, "Unknown");
		else
			wsprintf(szString, "Unknown");
	else
		wsprintf(szString, "No tuner loaded");

	return TRUE;
}

// This function is called by TSReader when it needs to send a DiSEqC command.
// Only sources that have CAPABILITIES_DISEQC_POSITIONER will get called.
BOOL TSReader_SendDiSEqC(BYTE * bCommand, int nLength)
{
	// not supported
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