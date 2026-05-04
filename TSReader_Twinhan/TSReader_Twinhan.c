#include <windows.h>
#include <commctrl.h>

#include "..\TSReader.h"

PVARIABLES v;
BOOL fNeedTuneDialog = TRUE;
int nFrequency;
int nPolarity;
int nSymbolRate;
int nLNBFrequency;
int n22KHz;
int nDiSEqCInput;

char gszSourceName[] = {"Twinhan 1020 Satellite Card (old API)"};

// Hardware specific stuff
int  (__stdcall * InitTsBuf) (DWORD PacketNumPerBuf, DWORD TsBufNum);
DWORD (WINAPI * StartCaptureTS) ();
DWORD (__stdcall * CaptureTS) (BYTE ** pTSData, LPSTR Savefile);
void (__stdcall * StopCaptureTS) ();
HANDLE (__stdcall * GetCapTsEvent) ();

void  (__stdcall * InitDevice) ();
int (__stdcall * SetDiseCom1) (BOOL K22Enable, BOOL ToneBurstEnable, BOOL DataBurstEnable, BOOL LNBPowerEnable);
int (__stdcall * SetDiseCom2) (BYTE x1, BYTE x2, BYTE x3, BYTE x4);
void (__stdcall * SetLNBValue) (ULONG lnb);
int	(__stdcall * LockCH) (DWORD Frequency, DWORD SymbolRate, DWORD H_V);
int	(__stdcall * StartCapture) ();
void (__stdcall * ReleaseDevice) ();
int (CALLBACK * GetSignalStrAndQu) (int *Str,int *Qu);
void (__stdcall * SetDataType) (BYTE bType , BYTE bModulation);

void SelectDiSEqCInput(int nInput)
{

	BYTE bPositionByte[] = {0xf0, 0xf4, 0xf8, 0xfc};

	nInput--;
	if ((nInput >= 0) && (nInput <= 3) )
	{
		SetDiseCom2(0xe0, 0x10, 0x38, bPositionByte[nInput]);
	}
}

DWORD WINAPI ReadTwinhanThread(LPVOID lpv)
{
	int nTSBufferIndex = 0;
	HANDLE hCapTS;

	v->fReadThreadTerminated = FALSE;
	v->fTerminateReadThread = FALSE;

	//start device here
	StartCapture();
	InitTsBuf(TS_PACKETS_AT_A_TIME, MAX_TS_BUFFERS);// first is number of TS packets per buffer, next buffer count
	StartCaptureTS();// start capturing;
	hCapTS = GetCapTsEvent();

	while (!v->fTerminateReadThread)
	{
		DWORD dwReturnedBytes;
		BYTE * pData;

		if(WAIT_OBJECT_0 == WaitForSingleObject(hCapTS,INFINITE))
		{
			dwReturnedBytes = CaptureTS(&pData , NULL);
			if(dwReturnedBytes)
			{
				EnterCriticalSection(&v->csPIDCounter);
				v->nLastSecondByteCounter += dwReturnedBytes;
				LeaveCriticalSection(&v->csPIDCounter);
				v->tsb[nTSBufferIndex].nSize = dwReturnedBytes;
				memcpy(v->tsb[nTSBufferIndex].pData, pData, dwReturnedBytes);
				v->nLWPtr = nTSBufferIndex;
				nTSBufferIndex++;
				if (nTSBufferIndex == MAX_TS_BUFFERS)
					nTSBufferIndex = 0;
				EnterCriticalSection(&v->csTSBuffersInUse);
				v->nTSBuffersInUse++;
				LeaveCriticalSection(&v->csTSBuffersInUse);
			}
			else
			{
#ifdef DEBUG_MESSAGES
				OutputDebugString("No data!\n");
#endif DEBUG_MESSAGES
				Sleep(5);
			}
		}
		else
		{
#ifdef DEBUG_MESSAGES
			OutputDebugString("WaitForSingleObject failed\n");
#endif DEBUG_MESSAGES
		}
	}

	v->fReadThreadTerminated = TRUE;
	v->fTerminateReadThread = FALSE;
	EnterCriticalSection(&v->csTSBuffersInUse);
	v->nTSBuffersInUse = -1000;
	LeaveCriticalSection(&v->csTSBuffersInUse);
	CloseHandle(v->hReadDataThread);
	OutputDebugString("ReadTwinhanThread-\n");
	return 0;
}


BOOL TSReader_Start()
{
	DWORD dwThreadID;
	v->hReadDataThread = CreateThread(NULL, 0, ReadTwinhanThread, (LPVOID)0, CREATE_SUSPENDED, &dwThreadID);
	SourceHelper_SetWorkerThreadPriorities(FALSE);
	ResumeThread(v->hReadDataThread);

	return TRUE;
}

BOOL TSReader_Stop()
{
	v->fTerminateReadThread = TRUE;
	while (v->fReadThreadTerminated == FALSE)
		Sleep(50);
	
	StopCaptureTS();//stop capturing and release buffers
	if (SetDiseCom1 != NULL && v->nPolarity != -1)
		SetDiseCom1(0,0,0,0);

	return TRUE;
}

BOOL TSReader_Init(PVARIABLES pv)
{
	v = pv;

	v->hThdemul = LoadLibrary("thdemul.dll");
	if (v->hThdemul == NULL)
		return FALSE;
	v->hThdevice = LoadLibrary("thdevice.dll");
	if (v->hThdevice == NULL)
		return FALSE;

	//SetDataType = (void *)GetProcAddress(v->hThdemul, "SetDataType");
	InitTsBuf = (void *)GetProcAddress(v->hThdemul, "InitTsBuf");
	StartCaptureTS = (void *)GetProcAddress(v->hThdemul, "StartCaptureTS");
	StopCaptureTS = (void *)GetProcAddress(v->hThdemul, "StopCaptureTS");
	CaptureTS = (void *)GetProcAddress(v->hThdemul, "CaptureTS");
	GetCapTsEvent = (void *)GetProcAddress(v->hThdemul,"GetCapTsEvent");

	InitDevice = (void *)GetProcAddress(v->hThdevice, "InitDevice");
	SetDiseCom1 = (void *)GetProcAddress(v->hThdevice, "SetDiseCom1");
	SetDiseCom2 = (void *)GetProcAddress(v->hThdevice, "SetDiseCom2");
	SetLNBValue = (void *)GetProcAddress(v->hThdevice, "SetLNBValue");
	LockCH = (void *)GetProcAddress(v->hThdevice, "LockCH");
	StartCapture = (void *)GetProcAddress(v->hThdevice, "StartCapture");
	ReleaseDevice = (void *)GetProcAddress(v->hThdevice, "ReleaseDevice");
	GetSignalStrAndQu = (void *)GetProcAddress(v->hThdevice, "GetSignalStrAndQu");
	SetDataType = (void *)GetProcAddress(v->hThdevice, "SetDataType");

	//set data format:
	{
		int nMode = 0;
		SetDataType((BYTE)nMode, 0);		// first 0=DVB 1=DSS second 0=QPSK 1=BPSK
		if (nMode)
			SendMessage(v->hDlgSIParser, WM_SETTEXT, 0, (LPARAM)"WARNING - DSS MODE!!");
	}

	//initialize device here:
	InitDevice();

	return TRUE;
}

BOOL TSReader_DeInit()
{
	if (v->hThdevice)
		ReleaseDevice();

	FreeLibrary(v->hThdemul);
	FreeLibrary(v->hThdevice);

	return TRUE;
}

BOOL TSReader_Tune()
{
	int nRetVal;

	//turn 22K/tone burst/data burst/power on
	if (v->fDontTune == FALSE)
	{
		int nTemporaryPolarity = v->nPolarity;

		if (SetDiseCom1 != NULL && v->nPolarity != -1)
			SetDiseCom1(v->n22KHz, 0, 0, 1);

		if (v->nDiSEqCInput)
			SelectDiSEqCInput(v->nDiSEqCInput);
		
		//set LNB value:
		SetLNBValue(v->nLNBFrequency);

		if (nTemporaryPolarity == -1)
			nTemporaryPolarity = 0;
		nRetVal = LockCH(v->nFrequency, v->nSymbolRate, nTemporaryPolarity);
	}
	else
		nRetVal = TRUE;

	if (!nRetVal)
	{
		if (SetDiseCom1 != NULL && v->nPolarity != -1)
			SetDiseCom1(0, 0, 0, 0);		// turn off power
		if (v->fAutoXMLExport == FALSE)
			MessageBox(v->hDlgSIParser, "Failed to lock signal", gszSourceName, MB_OK | MB_ICONWARNING);
		return FALSE;
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
	if (szCommandLineParameters != NULL)
		lstrcpy(szCommandLineParameters, "freq pol sr lnbf 22khz {input}");
	if (fCanBeStopped != NULL)
		*fCanBeStopped = FALSE;
	if (nMaxPIDs != NULL)
		*nMaxPIDs = 8192;
	if (dwCapabilities != NULL)
		*dwCapabilities = CAPABILITIES_DISEQC;
	return TRUE;
}

BOOL TSReader_TuneDialog(HWND hWnd)
{
	if (v->fDontTune == TRUE)
		fNeedTuneDialog = FALSE;

	if (fNeedTuneDialog)
	{
		if (SourceHelper_DVBSTuneDialog(hWnd) == FALSE)
			return FALSE;
	}
	else
	{
		v->nFrequency = nFrequency;
		v->nPolarity = nPolarity;
		v->nSymbolRate = nSymbolRate;
		v->nLNBFrequency = nLNBFrequency;
		v->n22KHz = n22KHz;
		v->nDiSEqCInput = nDiSEqCInput;
		fNeedTuneDialog = TRUE;
	}

	return TRUE;
}

BOOL TSReader_ParseCommandLine(PVARIABLES v, char * szCommandLine, BOOL fQuiet)
{
	if (lstrlen(szCommandLine))
	{
		int nConversionCount;

		SourceHelper_ConvertPolarity(szCommandLine);
		v->nDiSEqCInput = 1;
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
