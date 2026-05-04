#include <windows.h>
#include <commctrl.h>
#include <setupapi.h>
#include <initguid.h>
#include <stdio.h>
#include <math.h>

#include "..\sources.h"
#include "dvblib.h"

PSOURCESTRUCT ss;
BOOL fNeedTuneDialog = TRUE;
int nTunerStatusTimer;
CRITICAL_SECTION csSignal;
HANDLE hInstance;
CDVBLib * m_pDVBDrv;

char szLastSignalReport[128] = {"n/a"};        
char szLastTune[128] = {"n/a"};

char gszSourceName[] = {"Tongshi DVB-S"};

int nFrequency;
int nPolarity;
int nSymbolRate;
int nLNBFrequency;
int n22KHz;
int nDiSEqCInput;
int nADVModulationMode;
int nCodeRate;

extern "C" {
	BOOL __cdecl SourceHelper_ValidateSourceContainer(PSOURCESTRUCT pss);
}

void SelectDiSEqCInput(int nInput)
{
	BYTE bPositionByte[] = {0xc0, 0xc4, 0xc8, 0xcc};

	{
		char szDebug[128];
		wsprintf(szDebug, "Tongshi: SelectDiSEqCInput(%d)\n", nInput);
		OutputDebugString(szDebug);
	}
	nInput--;
	if ((nInput >= 0) && (nInput <= 3) )
	{
		BYTE bCommand[] = {0xe0, 0x10, 0x38, 0x00};
		bCommand[3] = bPositionByte[nInput];
		m_pDVBDrv->TSDVB0_SendRawDiSEqCMsg(bCommand, 4);
		Sleep((4 * 13) + 15 + 10);
	}
}

void SetupDiSEqC()
{
	switch(ss->nDiSEqCInput)
	{
	case 1:
	case 2:
	case 3:
	case 4:
		SelectDiSEqCInput(ss->nDiSEqCInput);
		Sleep(100);
		break;
	}
}

void SetupLastTune()
{
	szLastSignalReport[0] = '\0';

	char szPolarity[4] = {"H/L"};
	char szModulation[16] = {0};

	if (ss->nPolarity == 0)
		lstrcpy(szPolarity, "V/R");
	wsprintf(szLastTune, "%d MHz %s %d", ss->nFrequency, szPolarity, ss->nSymbolRate);
}

BOOL TSReader_Tune()
{
	BOOL StatusSuccess = FALSE;
	int nLBand;
	DWORD dwFrequency;

	SetupLastTune();
	SetupDiSEqC();

	// Calculate l-band, freq in hz and SR in symbols
	if (ss->nFrequency > ss->nLNBFrequency)
		nLBand = ss->nFrequency - ss->nLNBFrequency;
	else
		nLBand = ss->nLNBFrequency - ss->nFrequency;
	dwFrequency = nLBand * 1000;

	StatusSuccess = m_pDVBDrv->TSDVB0_LockTransponder(dwFrequency, ss->nSymbolRate, ss->nPolarity, ss->n22KHz);
	if (!StatusSuccess)
	{
		if (ss->fQuietMode == FALSE)
		{
			MessageBox(ss->hWndTSReader, "Failed to lock signal", gszSourceName, MB_ICONWARNING);
			return FALSE;
		}
	}

	return StatusSuccess;
}

#define READ_SIZE 64 * 1024

DWORD WINAPI ReadThread(LPVOID lpv)
{
	BOOL nBufferCounter = 0;
	int nSignalCounter = 0;
	BYTE bBuffer[READ_SIZE];

	SourceHelper_StartSyncThread(ss, FALSE);
	while (!ss->fTerminateReadThread)
	{
		int nReadLength = m_pDVBDrv->TSDVB0_ReadBuffer(bBuffer, READ_SIZE);
		if (nReadLength)
		{
			if (nBufferCounter < 8)
				nBufferCounter++;
			else
				SourceHelper_SyncData(bBuffer, nReadLength);
		}
		else
		{
			Sleep(1);
			if (nSignalCounter++ > 1000)
			{
				BOOL fLocked;
				DWORD dwQuality, dwStrength;
				char szLockStatus[32] = {0};

				nSignalCounter = 0;
				m_pDVBDrv->TSDVB0_GetSignal(&fLocked, &dwQuality, &dwStrength);
				if (fLocked)
					lstrcpy(szLockStatus, "Locked");
				else
					lstrcpy(szLockStatus, "Unlocked");
				
				EnterCriticalSection(&csSignal);
				wsprintf(szLastSignalReport, "%s Quality %d%% Signal %d%%", szLockStatus, dwQuality, dwStrength);
				LeaveCriticalSection(&csSignal);

			}
		}
	}	
	SourceHelper_StopSyncThread();
	
	ss->fReadThreadTerminated = TRUE;
	ss->fTerminateReadThread = FALSE;
	EnterCriticalSection(&ss->csTSBuffersInUse);
	ss->nTSBuffersInUse = -1000;
	LeaveCriticalSection(&ss->csTSBuffersInUse);
	CloseHandle(ss->hReadDataThread);

	return 0;
}

BOOL TSReader_Start()
{
	DWORD dwThreadID;

	OutputDebugString("Tongshi: enter Start()\n");

	m_pDVBDrv->TSDVB0_PassFullStream(TRUE);

	ss->hReadDataThread = CreateThread(NULL, 0, ReadThread, (LPVOID)0, CREATE_SUSPENDED, &dwThreadID);
	SourceHelper_SetWorkerThreadPriorities(FALSE);
	ResumeThread(ss->hReadDataThread);

	OutputDebugString("Tongshi: leave Start()\n");
	return TRUE;
}

BOOL TSReader_Stop()
{
	OutputDebugString("Tongshi: enter Stop()\n");

	ss->fTerminateReadThread = TRUE;
	while (ss->fReadThreadTerminated == FALSE)
		Sleep(50);
	
	OutputDebugString("Tongshi: leave Stop()\n");
	return TRUE;
}

BOOL TSReader_Init(PSOURCESTRUCT pss)
{
	int nCount;

	if (SourceHelper_ValidateSourceContainer(pss) == FALSE)
		return FALSE;
	OutputDebugString("Tongshi: Init\n");
	InitializeCriticalSection(&csSignal);

	ss = pss;
	m_pDVBDrv = new CDVBLib();
	nCount = m_pDVBDrv->TSDVB0_GetDevCount();
	if (!m_pDVBDrv->TSDVB0_OpenDevice(0))
		return FALSE;

	return TRUE;
}

BOOL TSReader_DeInit()
{
	OutputDebugString("Tongshi: +DeInit\n");

	delete m_pDVBDrv;

	DeleteCriticalSection(&csSignal);
	OutputDebugString("Tongshi: -DeInit\n");
	return TRUE;
}


BOOL TSReader_TuneDialog(HWND hWnd)
{
	OutputDebugString("Tongshi: TSReader_TuneDialog\n");

	if (ss->fDontTune == TRUE)
		fNeedTuneDialog = FALSE;

	if (fNeedTuneDialog)
	{
		ss->fQuietMode = FALSE;
		OutputDebugString("Tongshi: TSReader_TuneDialog tuning dialog is required\n");
		if (SourceHelper_DVBSTuneDialog(hWnd) == FALSE)
			return FALSE;
	}
	else
	{
		OutputDebugString("Tongshi: TSReader_TuneDialog tune NOT required\n");
		ss->nFrequency = nFrequency;
		ss->nPolarity = nPolarity;
		ss->nSymbolRate = nSymbolRate;
		ss->nLNBFrequency = nLNBFrequency;
		ss->n22KHz = n22KHz;
		ss->nDiSEqCInput = nDiSEqCInput;
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
		lstrcpy(szCommandLineParameters, "freq pol sr lnbf 22khz {input}");	
	if (dwCapabilities != NULL)
		*dwCapabilities = CAPABILITIES_POWER
		                | CAPABILITIES_DISEQC
						| CAPABILITIES_DISEQC_POSITIONER;
						//| CAPABILITIES_MULTICARD;
	return TRUE;
}

BOOL TSReader_ParseCommandLine(PSOURCESTRUCT ss, char * szCommandLine, BOOL fQuiet)
{
	if (lstrlen(szCommandLine))
	{
		int nConversionCount = 0;
		SourceHelper_ConvertPolarity(szCommandLine);
		ss->nDiSEqCInput = 0;
		nConversionCount = sscanf(szCommandLine,
								  "%d %d %d %d %d %d", 
								  &nFrequency,
								  &nPolarity,
								  &nSymbolRate,
								  &nLNBFrequency,
								  &n22KHz,
								  &nDiSEqCInput);
		if (nConversionCount < 5)
		{
			if (!fQuiet)
				MessageBox(NULL,
					   "Usage for this source: freq pol sr lnbf 22khz {input}\n"
					   "\n"
					   "freq = frequency to tune\n"
					   "pol = 0 vertical/RHCP 1 horizontal/LHCP\n"
					   "sr = symbol rate\n"
					   "lnbf = LNB frequency\n"
					   "22k = 22KHz tone enable\n"
					   "input = select DiSEqC input number (1-4) - optional",
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
	m_pDVBDrv->TSDVB0_SendRawDiSEqCMsg(bCommand, nLength);
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
