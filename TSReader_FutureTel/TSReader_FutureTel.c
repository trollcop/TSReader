#include <windows.h>
#include <commctrl.h>
#include <setupapi.h>
#include <initguid.h>
#include <stdio.h>
#include <math.h>

#include <primeware.h>
#include <mav.h>

#include "../sources.h"

PSOURCESTRUCT ss;
CRITICAL_SECTION csSignal;
BOOL fInited;

char * szCmdLinePtr;
char gszSourceName[] = {"FutureTel PrimeWare Encoder"};
char szLastTune[128] = {"n/a"};
char szLastSignalReport[128] = {"n/a"};

BOOL __cdecl SourceHelper_ValidateSourceContainer(PSOURCESTRUCT pss);

// Hardware specific stuff

int NS320CallbackFunction(int nChannel, STRM_STREAM_ID streamId, BYTE *pBuffer, int nByteCount)
{
	SourceHelper_SyncData(pBuffer, nByteCount);
	return SUCCESS;
}

DWORD WINAPI ReadTTThread(LPVOID lpv)
{
	ss->fReadThreadTerminated = FALSE;
	ss->fTerminateReadThread = FALSE;

	SourceHelper_StartSyncThread(ss, FALSE);
	StreamerCommand(0, SET, STRM_CALLBACK, NS320CallbackFunction, STRM_ID_MUX);
	EncoderCommand(0, CONTROL, START);
	while (!ss->fTerminateReadThread)
	{
		Sleep(10);
	}
	EncoderCommand(0, CONTROL, STOP);
	
	// Acutally wait for the encoder to stop
	Sleep(1000);
	do
	{
		ENCODER_STATUS encstat;

		EncoderCommand(0, GET, ENC_STATUSBLOCK, &encstat);
		if (encstat.mveState != RUNNING)
			break;
		Sleep(100);
	} while (TRUE);

	SourceHelper_StopSyncThread();
	CloseHandle(ss->hReadDataThread);
	ss->fReadThreadTerminated = TRUE;
	ss->fTerminateReadThread = FALSE;
	OutputDebugString("ReadTTThread-\n");
	return 0;
}

BOOL TSReader_Start()
{
	DWORD dwThreadID;

	ss->hReadDataThread = CreateThread(NULL, 0, ReadTTThread, (LPVOID)0, CREATE_SUSPENDED, &dwThreadID);
	SourceHelper_SetWorkerThreadPriorities(FALSE);
	ResumeThread(ss->hReadDataThread);

	return TRUE;
}

BOOL TSReader_Stop()
{
	ss->fTerminateReadThread = TRUE;
	while (ss->fReadThreadTerminated == FALSE)
		Sleep(50);

	EnterCriticalSection(&ss->csTSBuffersInUse);
	ss->nTSBuffersInUse = -1000;
	LeaveCriticalSection(&ss->csTSBuffersInUse);

	return TRUE;
}

BOOL TSReader_Init(PSOURCESTRUCT pss)
{
	if (SourceHelper_ValidateSourceContainer(pss) == FALSE)
		return FALSE;
	InitializeCriticalSection(&csSignal);

	ss = pss;

	if (!fInited)
	{
		MpegInit(ss->hTSReaderInst, 3, NULL);
		fInited = TRUE;
	}

	return TRUE;
}

BOOL TSReader_DeInit()
{
	DeleteCriticalSection(&csSignal);

	return TRUE;
}

BOOL TSReader_Tune()
{
	return TRUE;
}

BOOL TSReader_TuneDialog(HWND hWnd)
{
	if (EncoderConfig(NULL, 0) == IDCANCEL)
		return FALSE;
	StreamerCommand(0, SAVE);
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
		lstrcpy(szCommandLineParameters, "n/a");
	if (dwCapabilities != NULL)
		*dwCapabilities = 0;
	if (fCanBeStopped != NULL)
		*fCanBeStopped = FALSE;
	if (nMaxPIDs != NULL)
		*nMaxPIDs = 8192;

	return TRUE;
}

BOOL TSReader_ParseCommandLine(PSOURCESTRUCT ss, char * szCommandLine, BOOL fQuiet)
{
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

BOOL TSReader_SendDiSEqC(BYTE * bCommand, int nLength)
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

BOOL APIENTRY DllMain(HANDLE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
    switch( ul_reason_for_call )
	{
    case DLL_PROCESS_ATTACH:
		fInited = FALSE;
		break;
    case DLL_PROCESS_DETACH:
		if (fInited)
		{
			MpegTerminate();
			fInited = FALSE;
		}
		break;
    }
    return TRUE;
}
