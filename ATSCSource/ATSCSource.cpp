/*
Copyright (c) David R. Cattley (dcattley@msn.com). All rights reserved.

Module Name:

    ATSCSource.cpp

Abstract:

	ATSC BDA Source for TSReader (www.coolstf.com)

Author:

    David R. Cattley (dcattley@msn.com)

Revision History:

	01-Feb-2005 - Created
*/

#include "stdafx.h"
#include "resource.h"
#include "ATSCSource.h"

#define	TSREADER_ENTRY	__cdecl

HINSTANCE	g_hInstance;
CATSCSource g_theSource;


BOOL 
APIENTRY 
DllMain( 
	HANDLE hModule, 
    DWORD  ul_reason_for_call, 
    LPVOID lpReserved
	)
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		g_hInstance = (HINSTANCE)hModule;
		break;

	case DLL_PROCESS_DETACH:
		break;
	}

    return TRUE;
}



//
// Dispatch entry points to the g_theSource functions.
//

//
// TSReader_GetDescription()
//
//	First function called by TSReader to collect information 
//	about the source.
//
//	
//
BOOL
TSREADER_ENTRY
TSReader_GetDescription(
	OUT LPSTR lpszDescription, 
	OUT LPSTR lpszCommandLineParameters, 
	OUT LPBOOL fCanBeStopped, 
	OUT LPINT nMaxPIDs, 
	OUT LPDWORD dwCapabilities
	)
{
	if (lpszDescription)
	{
#ifndef QAM
 #ifndef NOSTATUS
		LoadStringA(g_hInstance, IDS_SOURCE_DESCRIPTION, lpszDescription, 256);
 #else NOSTATUS
		LoadStringA(g_hInstance, IDS_SOURCE_DESCRIPTION_NOSTATUS, lpszDescription, 256);
 #endif NOSTATUS
#else QAM
 #ifndef CREATOR
		LoadStringA(g_hInstance, IDS_SOURCE_DESCRIPTION_QAM_GT, lpszDescription, 256);
 #else CREATOR
		LoadStringA(g_hInstance, IDS_SOURCE_DESCRIPTION_QAM_CR, lpszDescription, 256);
 #endif CREATOR
#endif QAM
	}

	if (lpszCommandLineParameters)
	{
		LoadStringA(g_hInstance, IDS_SOURCE_COMMAND_LINE_PARAMETERS, lpszCommandLineParameters, 256);
	}

	if (fCanBeStopped)
	{
		*fCanBeStopped = FALSE;
	}

	if (nMaxPIDs)
	{
		*nMaxPIDs = 8192;
	}

	if (dwCapabilities)
	{
		*dwCapabilities = 0;
	}

	return TRUE;
}


//
// TSReader_ParseCommandLine()
//
//	Second function called by TSReader to give the source a chance to parse
//	the command line.
//
BOOL 
TSREADER_ENTRY
TSReader_ParseCommandLine(
	IN PSOURCESTRUCT ss, 
	IN LPCSTR lpszCommandLine, 
	IN BOOL fQuiet
	)
{
	return g_theSource.ParseCommandLine(ss, lpszCommandLine, fQuiet);
}


//
// TSReader_Init()
//
//
BOOL
TSREADER_ENTRY
TSReader_Init(PSOURCESTRUCT pss)
{
	return g_theSource.Init(pss);
}


BOOL 
TSREADER_ENTRY
TSReader_TuneDialog(HWND hWndParent)
{
	return g_theSource.TuneDialog(hWndParent);
}

//
// TSReader_DeInit()
//
//	Last function called by TSReader to give the source a chance to cleanup.
//
BOOL 
TSREADER_ENTRY
TSReader_DeInit(void)
{
	return g_theSource.DeInit();
}


BOOL 
TSREADER_ENTRY
TSReader_Tune(void)
{
	return g_theSource.Tune();
}

BOOL 
TSREADER_ENTRY
TSReader_Start(void)
{
	return g_theSource.Start();
}

BOOL 
TSREADER_ENTRY
TSReader_Stop(void)
{
	return g_theSource.Stop();
}

BOOL 
TSREADER_ENTRY
TSReader_PIDManagement(BOOL fAdd, int nPID, BOOL fTemporary)
{
	return g_theSource.PIDManagement(fAdd, nPID, fTemporary);
}

BOOL 
TSREADER_ENTRY
TSReader_IsPIDActive(int nPID)
{
	return g_theSource.IsPIDActive(nPID);
}

BOOL 
TSREADER_ENTRY
TSReader_SetChannel(int nChannel)
{
	return g_theSource.SetChannel(nChannel);
}

BOOL 
TSREADER_ENTRY
TSReader_GetSignalString(char * szString)
{
	return g_theSource.GetSignalString(szString);
}

BOOL 
TSREADER_ENTRY
TSReader_GetTunerString(char * szString)
{
	return g_theSource.GetTunerString(szString);
}

BOOL 
TSREADER_ENTRY
TSReader_SendDiSEqC(BYTE * bCommand, int nLength)
{
	return g_theSource.SendDiSEqC(bCommand, nLength);
}

int 
TSREADER_ENTRY
TSReader_GetSyncLossCount(BOOL fReset)
{
	return SourceHelper_GetSyncLossCount(fReset);
}


//
// CATSCSource Implemenation
//