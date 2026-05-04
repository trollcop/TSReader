#include <windows.h>
#include <commctrl.h>
#include <stdio.h>

#include "..\sources.h"

#include "seconst.h"
#include "seerror.h"
#include "seprot.h"

PSOURCESTRUCT ss;
char * szCmdLinePtr;
BOOL fNeedTuneDialog = TRUE;
HANDLE hInstance;
CRITICAL_SECTION csSignal;
BOOL * pPIDActiveList = NULL;
BOOL fPIDManagementFailed = FALSE;
int nTSBufferIndex;
int nDispatchedBuffers;
HMODULE hBLInterface = NULL;

int nFrequency;
int nPolarity;
int nSymbolRate;
int nLNBFrequency;
int n22KHz;
int nDiSEqCInput;

// Stuff in ADPSI.DLL
SE_STATUS (*pADPTSE_FreeBuffer) (SE_INT iBoardNumber, \
                                    SE_HANDLE seHandle, \
                                    PSE_VOID pStreamBuffer);
SE_STATUS (*pADPTSE_StartCapture) (SE_INT iBoardNumber, \
                                      SE_HANDLE seHandle);
SE_STATUS (*pADPTSE_StopCapture) (SE_INT iBoardNumber, \
                                     SE_HANDLE seHandle);
SE_STATUS (*pADPTSE_OpenStream) (SE_INT iBoardNumber, \
                                    PADPTSE_STREAM_SETUP pStreamSetup,
                                    PSE_HANDLE pseHandle);
SE_STATUS (*pADPTSE_CloseStream) (SE_INT iBoardNumber, \
                                     SE_HANDLE seHandle);
SE_STATUS (*pADPTSE_RegisterApp) (SE_INT iBoardNumber, \
                                     PADPTSE_APP_CALLBACK pAppCallbackFunction, \
                                     SE_BOOL bRequestMaster);
SE_STATUS (*pADPTSE_UnRegisterApp) (SE_INT iBoardNumber);
SE_STATUS (*pADPTSE_QueryFELockStatus) (SE_INT iBoardNumber, \
                                           PFE_LOCK_STATUS pFELockStatus);
SE_STATUS (*pADPTSE_QueryFEErrorStatus) (SE_INT iBoardNumber, \
                                            PFE_ERROR_STATUS pFEErrorStatus);
SE_STATUS (*pADPTSE_SetLNBFrequency) (SE_INT iBoardNumber, \
                                         SE_ULONG ulLNBFrequencyLow, \
                                         SE_ULONG ulLNBFrequencyHigh);
SE_STATUS (*pADPTSE_SetTunerWithLNB) (SE_INT iBoardNumber, \
                                         PSAT_TUNER_SETTINGS pSatTunerSettings);

BOOL __cdecl SourceHelper_ValidateSourceContainer(PSOURCESTRUCT pss);

typedef struct __tagPIDfilter
{
	long nPID;
	BOOL fTemporary;
	int nAge;
	SE_HANDLE seHandle;
} PIDFILTER, *PPIDFILTER;

#define MAX_DEMUX_PIDS 8
PIDFILTER PIDFilter[MAX_DEMUX_PIDS];

long nActivePIDCount = 0;
long nActivePIDs[40];
long nBasePIDs[] =  {0x0000, 0x0001, 0x0010, 0x0011, 0x0012, 0x0013, 0x0014};

char gszSourceName[] = {"Broadlogic"};
char szLastTune[128] = {"n/a"};
char szLastSignalReport[128] = {"n/a"};

void __cdecl StreamCallBack(SE_UINT   iBoardNumber, SE_HANDLE seHandle, PSE_VOID  pvUserData, PSE_VOID  pvData, SE_UINT   uiDataBlockLength, SE_INT    iPacketStatus)
{
	int nMaxSize = TS_BUFFER_SIZE;

	if (nDispatchedBuffers < MAX_TS_BUFFERS / 3)
		nMaxSize = TS_BUFFER_SIZE / 4;

	if (ss->tsb[nTSBufferIndex].nSize + (int)uiDataBlockLength >= nMaxSize)
	{
		nTSBufferIndex++;
		if (nTSBufferIndex == MAX_TS_BUFFERS)
			nTSBufferIndex = 0;
		EnterCriticalSection(&ss->csTSBuffersInUse);
		ss->nTSBuffersInUse++;
		LeaveCriticalSection(&ss->csTSBuffersInUse);
		ss->tsb[nTSBufferIndex].nSize = 0;
		nDispatchedBuffers++;
	}

	memcpy(&ss->tsb[nTSBufferIndex].pData[ss->tsb[nTSBufferIndex].nSize], pvData, uiDataBlockLength);
	ss->tsb[nTSBufferIndex].nSize += uiDataBlockLength;

	EnterCriticalSection(&ss->csPIDCounter);
	ss->nLastSecondByteCounter += uiDataBlockLength;
	LeaveCriticalSection(&ss->csPIDCounter);

	pADPTSE_FreeBuffer(iBoardNumber, seHandle, pvData);
}

void AddPIDToPin(int nPIDFilterItem)
{
	ADPTSE_STREAM_SETUP streamsetup={TRANSPORT_STREAM, 0x1FFF, 1, 188, NULL, StreamCallBack};  

	streamsetup.uiPID = PIDFilter[nPIDFilterItem].nPID;
	streamsetup.uiBuffers = 512;
	streamsetup.pvUserData = (int *)streamsetup.uiPID;
	pADPTSE_OpenStream(0, &streamsetup, &PIDFilter[nPIDFilterItem].seHandle);
	pADPTSE_StartCapture(0, PIDFilter[nPIDFilterItem].seHandle);
}

void DeletePIDFromPin(int nPIDFilterItem)
{
	pADPTSE_StopCapture(0, PIDFilter[nPIDFilterItem].seHandle);
	pADPTSE_CloseStream(0, PIDFilter[nPIDFilterItem].seHandle);
}

BOOL TSReader_Start()
{
	int nPID = 0;
	int nIndex = 0;

	nTSBufferIndex = 0;
	nDispatchedBuffers = 0;

	// Add in startup PIDs -- others will be found in the thread
	nActivePIDCount = sizeof(nBasePIDs) / sizeof(long);
	for (nIndex = 0; nIndex < nActivePIDCount; nIndex++)
	{
		PIDFilter[nIndex].nPID = nBasePIDs[nIndex];
		if (nBasePIDs[nIndex] != 0x0000)
			PIDFilter[nIndex].fTemporary = TRUE;
		else
			PIDFilter[nIndex].fTemporary = FALSE;
		PIDFilter[nIndex].nAge = 0;
		AddPIDToPin(nIndex);
	}
	
	return TRUE;
}

BOOL TSReader_Stop()
{
	int nIndex = 0;

	for (nIndex = 0; nIndex < nActivePIDCount; nIndex++)
	{
		DeletePIDFromPin(nIndex);
		PIDFilter[nIndex].nPID = -1;
		PIDFilter[nIndex].fTemporary = TRUE;
		PIDFilter[nIndex].nAge = 0;
	}
	nActivePIDCount = 0;

	EnterCriticalSection(&ss->csTSBuffersInUse);
	ss->nTSBuffersInUse = -1000;
	LeaveCriticalSection(&ss->csTSBuffersInUse);

	return TRUE;
}

void __cdecl MasterStatusCallBack(SE_UINT iBoardNumber)
{
	OutputDebugString("Broadlogic: MasterStatusCallBack\n");
}

void __cdecl CAMessageCallBack(SE_UINT  iBoardNumber, PSE_VOID pvMessage, SE_INT   iMessageLength)
{
	OutputDebugString("Broadlogic: CAMessageCallBack\n");
}

void __cdecl AddMultiCastAddressCallBack(SE_UINT  iBoardNumber, SE_ULONG ulEthernetAddress)
{
	OutputDebugString("Broadlogic: AddMultiCastAddressCallBack\n");
}

void __cdecl RemoveMultiCastAddressCallBack(SE_UINT  iBoardNumber, SE_ULONG ulEthernetAddress)
{
	OutputDebugString("Broadlogic: RemoveMultiCastAddressCallBack\n");
}

SE_INT  __cdecl DownloadSoftwareCallBack(SE_INT  iBoardNumber, SE_INT  iPercentComplete, SE_BOOL bCanCancel)
{
	OutputDebugString("Broadlogic: DownloadSoftwareCallBack\n");
	return 0;
}

void    __cdecl TableSectionCallBack(SE_UINT iBoardNumber, SE_HANDLE seHandle, PSE_VOID pvTableSection, SE_UINT uiDataBlockLength)
{
	OutputDebugString("Broadlogic: TableSectionCallBack\n");
}

void    __cdecl StreamDataCallBack(SE_UINT iBoardNumber,  SE_HANDLE seHandle,  PSE_VOID pvStreamData,  SE_UINT uiDataBlockLength)
{
	pADPTSE_FreeBuffer(iBoardNumber,seHandle,pvStreamData);
}

void __cdecl TableCallback(SE_UINT iBoardNumber, // Board Number
						   SE_HANDLE seHandle, // Capture Handle
						   PSE_VOID pvUserData, // User Context data
						   PSE_VOID pvData, // Data
						   SE_UINT uiDataBlockLength, // Length of data
						   SE_INT iPacketStatus // Status of packet.
						  )
{
	pADPTSE_FreeBuffer(iBoardNumber,seHandle,pvData);
}

BOOL TSReader_Init(PSOURCESTRUCT pss)
{
	SE_STATUS status;
	ADPTSE_APP_CALLBACK AppCallbackFunction={MasterStatusCallBack,
											 TableSectionCallBack,
											 StreamDataCallBack,
											 CAMessageCallBack,
											 AddMultiCastAddressCallBack,
											 RemoveMultiCastAddressCallBack};

	if (SourceHelper_ValidateSourceContainer(pss) == FALSE)
		return FALSE;
	InitializeCriticalSection(&csSignal);

	ss = pss;

	hBLInterface = LoadLibrary("adpsi30.dll");
	if (hBLInterface == NULL)
	{
		MessageBox(NULL, "Unable to load the Broadlogic interface adpsi30.dll", gszSourceName, MB_ICONSTOP);
		return FALSE;
	}
	pADPTSE_FreeBuffer = (void *)GetProcAddress(hBLInterface, "ADPTSE_FreeBuffer");
	pADPTSE_StartCapture = (void *)GetProcAddress(hBLInterface, "ADPTSE_StartCapture");
	pADPTSE_StopCapture = (void *)GetProcAddress(hBLInterface, "ADPTSE_StopCapture");
	pADPTSE_OpenStream = (void *)GetProcAddress(hBLInterface, "ADPTSE_OpenStream");
	pADPTSE_CloseStream = (void *)GetProcAddress(hBLInterface, "ADPTSE_CloseStream");
	pADPTSE_RegisterApp = (void *)GetProcAddress(hBLInterface, "ADPTSE_RegisterApp");
	pADPTSE_UnRegisterApp = (void *)GetProcAddress(hBLInterface, "ADPTSE_UnRegisterApp");
	pADPTSE_QueryFELockStatus = (void *)GetProcAddress(hBLInterface, "ADPTSE_QueryFELockStatus");
	pADPTSE_QueryFEErrorStatus = (void *)GetProcAddress(hBLInterface, "ADPTSE_QueryFEErrorStatus");
	pADPTSE_SetLNBFrequency= (void *)GetProcAddress(hBLInterface, "ADPTSE_SetLNBFrequency");
	pADPTSE_SetTunerWithLNB = (void *)GetProcAddress(hBLInterface, "ADPTSE_SetTunerWithLNB");

	status = pADPTSE_RegisterApp(0, &AppCallbackFunction, TRUE);
	if (status != SE_OK)
	{
		MessageBox(NULL, "The Broadlogic interface failed to register. Perhaps you have another program using the card?", gszSourceName, MB_OK | MB_ICONSTOP);
		return FALSE;
	}

	return TRUE;
}

BOOL TSReader_DeInit()
{
	SE_STATUS status;
	status = pADPTSE_UnRegisterApp(0);

	FreeLibrary(hBLInterface);
	hBLInterface = NULL;

	DeleteCriticalSection(&csSignal);
	return TRUE;
}

void SetupLastTune()
{
	char szPolarity[4] = {"H/L"};
	char szModulation[16] = {0};

	lstrcpy(szLastSignalReport, "n/a");

	if (ss->nPolarity == 0)
		lstrcpy(szPolarity, "V/R");
	switch(ss->nADVModulationMode)
	{
	case ADV_MOD_DVB_QPSK:
		lstrcpy(szModulation, "DVB QPSK");
		break;
	}
	wsprintf(szLastTune, "%d MHz %s %d %s", ss->nFrequency, szPolarity, ss->nSymbolRate, szModulation);
}

BOOL TSReader_Tune()
{
	SAT_TUNER_SETTINGS tunersetting={12110000, POLARIZATION_HORIZONTAL, 15000 };
	SE_STATUS status;

	SetupLastTune();

	status = pADPTSE_SetLNBFrequency(0, ss->nLNBFrequency, 0);	
	tunersetting.ulTunerFrequency_Khz = ss->nFrequency * 1000;
	tunersetting.ulSymbolRate_KSym = ss->nSymbolRate;
	if (ss->nPolarity)
		tunersetting.polarization = POLARIZATION_HORIZONTAL;
	else
		tunersetting.polarization = POLARIZATION_VERTICAL;
	status = pADPTSE_SetTunerWithLNB(0, &tunersetting);
	if (status != SE_OK)
	{
		MessageBox(ss->hWndTSReader, "Failed to lock signal", gszSourceName, MB_OK | MB_ICONSTOP);
		return FALSE;
	}

	return TRUE;
}

BOOL TSReader_TuneDialog(HWND hWnd)
{
	if (ss->fDontTune == TRUE)
	{
		fNeedTuneDialog = FALSE;
		OutputDebugString("Broadlogic: TuneDialog() set fNeedTuneDialog = FALSE\n");
	}

	{
		char szTemp[100];
		wsprintf(szTemp, "Broadlogic: fNeedTuneDialog = %d\n", fNeedTuneDialog);
		OutputDebugString(szTemp);
	}
	if (fNeedTuneDialog)
	{
		if (SourceHelper_DVBSTuneDialog(hWnd) == FALSE)
			return FALSE;
	}
	else
	{
		ss->nFrequency = nFrequency;
		ss->nPolarity = nPolarity;
		ss->nSymbolRate = nSymbolRate;
		ss->nLNBFrequency = nLNBFrequency;
		ss->n22KHz = n22KHz;
		ss->nDiSEqCInput = nDiSEqCInput;
		if (ss->fQuietMode == FALSE)
		{
			fNeedTuneDialog = TRUE;
			OutputDebugString("Broadlogic: TuneDialog() set fNeedTuneDialog = TRUE\n");
		}
	}

	return TRUE;
}

BOOL TSReader_PIDManagement(BOOL fAdd, int nPID, BOOL fTemporary)
{
	int nIndex;
	int nMaxAge;
	int nOldestItem;

	if (fPIDManagementFailed)
		return FALSE;

	// Bump all active PIDs
	for (nIndex = 0; nIndex < nActivePIDCount; nIndex++)
	{
		if (PIDFilter[nIndex].nPID != -1)
			PIDFilter[nIndex].nAge++;
	}

	if (fAdd)
	{
		// See if this PID is already active
		for (nIndex = 0; nIndex < nActivePIDCount; nIndex++)
		{
			if (PIDFilter[nIndex].nPID == nPID)
			{
				PIDFilter[nIndex].fTemporary = fTemporary;
				PIDFilter[nIndex].nAge = 0;
				return TRUE;
			}
		}

		// PID isn't active - see if there's room
		if (nActivePIDCount < MAX_DEMUX_PIDS)
		{
			//ss->nPIDContinuity[nPID] = -1;
			PIDFilter[nActivePIDCount].nPID = nPID;
			PIDFilter[nActivePIDCount].nAge = 0;
			PIDFilter[nActivePIDCount].fTemporary = fTemporary;
			AddPIDToPin(nActivePIDCount);
			nActivePIDCount++;
			return TRUE;
		}

		// PID isn't active and there's no room left - find the oldest temporary
		// PID and remove it
		nMaxAge = 0;
		nOldestItem = -1;
		for (nIndex = 0; nIndex < nActivePIDCount; nIndex++)
		{
			if (PIDFilter[nIndex].fTemporary)
			{
				if (PIDFilter[nIndex].nAge > nMaxAge)
				{
					nMaxAge = PIDFilter[nIndex].nAge;
					nOldestItem = nIndex;
				}
			}
		}
		if (nOldestItem == -1)
		{
			MessageBox(ss->hWndTSReader, "Unable to free up any temporary PIDs - demux is full - oops", gszSourceName, MB_ICONSTOP);
			fPIDManagementFailed = TRUE;
			return FALSE;
		}
		DeletePIDFromPin(nOldestItem);
		//ss->nPIDContinuity[nPID] = -1;
		PIDFilter[nOldestItem].nPID = nPID;
		PIDFilter[nOldestItem].nAge = 0;
		PIDFilter[nOldestItem].fTemporary = fTemporary;
		AddPIDToPin(nOldestItem);
	}
	else
	{
		for (nIndex = 0; nIndex < nActivePIDCount; nIndex++)
		{
			if (PIDFilter[nIndex].nPID == nPID)
			{
				PIDFilter[nIndex].fTemporary = TRUE;
				return TRUE;
			}
		}
	}
	return TRUE;
}

BOOL TSReader_GetDescription(char * szDescription, char * szCommandLineParameters, BOOL * fCanBeStopped, int * nMaxPIDs, DWORD * dwCapabilities)
{
	if (szDescription != NULL)
		lstrcpy(szDescription, gszSourceName);
	if (szCommandLineParameters != NULL)
		lstrcpy(szCommandLineParameters, "freq pol sr lnbf 22khz");
	if (dwCapabilities != NULL)
		*dwCapabilities = CAPABILITIES_POWER;
	if (fCanBeStopped != NULL)
		*fCanBeStopped = FALSE;
	if (nMaxPIDs != NULL)
		*nMaxPIDs = MAX_DEMUX_PIDS;

	return TRUE;
}

BOOL TSReader_ParseCommandLine(PSOURCESTRUCT ss, char * szCommandLine, BOOL fQuiet)
{
	fNeedTuneDialog = TRUE;
	if (lstrlen(szCommandLine))
	{
		int nConversionCount;

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
	int nIndex;

	for (nIndex = 0; nIndex < nActivePIDCount; nIndex++)
	{
		if (PIDFilter[nIndex].nPID == nPID)
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
	FE_LOCK_STATUS FELockStatus;
	FE_ERROR_STATUS FEErrorStatus;

	pADPTSE_QueryFELockStatus(0, &FELockStatus);
	if ((FELockStatus.byLockStatusBits & 0x17) == 0x17)
	{
		pADPTSE_QueryFEErrorStatus(0, &FEErrorStatus);
		sprintf(szString, "Locked BER %.3G EbNo %3.f", FEErrorStatus.dBitErrRate, FEErrorStatus.dEbNo);
	}
	else
		lstrcpy(szString, "Unlocked");

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

BOOL APIENTRY DllMain(HANDLE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
    switch( ul_reason_for_call )
	{
    case DLL_PROCESS_ATTACH:
		hInstance = hModule;
		break;
    case DLL_PROCESS_DETACH:
		break;
    }
    return TRUE;
}
