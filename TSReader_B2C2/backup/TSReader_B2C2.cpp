#include <windows.h>
#include <commctrl.h>
#include <stdio.h>

#include <Dshow.h>
#include <initguid.h>

#include "B2C2_Defs.h"
#include "b2c2mpeg2adapter.h"
#include "IB2C2MPEG2DataCtrl.h"
#include "IB2C2MPEG2TunerCtrl.h"

#include "..\sources.h"

PSOURCESTRUCT ss;
char * szCmdLinePtr;
BOOL fNeedTuneDialog = TRUE;
HANDLE hInstance;
CRITICAL_SECTION csSignal;
BOOL * pPIDActiveList = NULL;
BOOL fIsDemuxFree = FALSE;
BOOL fPIDManagementFailed = FALSE;
BOOL fAlternativeInterface = FALSE;
BOOL fDontPromptForInterface = FALSE;
HMODULE hFlexDump = NULL;

int nFrequency;
#ifdef DVBS
int nPolarity;
int nSymbolRate;
int nLNBFrequency;
int n22KHz;
int nDiSEqCInput;
#endif DVBS
#ifdef DVBT
BOOL fSpectrumInversion;
int nBandwidth;
#endif DVBT
#ifdef DVBC
int nSymbolRate;
int nQAM;
BOOL fSpectrumInversion;
int nBandwidth;
#endif DVBC

B2C2MPEG2Adapter		*pb2c2Adapter;
IB2C2MPEG2DataCtrl3 	*pB2C2FilterDataCtrl = NULL;
IB2C2MPEG2TunerCtrl2 	*pB2C2FilterTunerCtrl = NULL;
IFileSinkFilter			*pFileSink = NULL;
IMediaControl			*pMediaControl = NULL;

#define DUMPFILEPATHNAME_SZ		1024			// SParams.szDumpFilePathname string size
OLECHAR					szOleFilePathname[DUMPFILEPATHNAME_SZ];
BSTR					bstrFileName;

typedef struct __tagPIDfilter
{
	long nPID;
	BOOL fTemporary;
	int nAge;
} PIDFILTER, *PPIDFILTER;

#define MAX_DEMUX_PIDS 39
PIDFILTER PIDFilter[MAX_DEMUX_PIDS];

long nActivePIDCount = 0;
long nActivePIDs[40];

// Christian Hackbart's code
typedef enum 
{
ttCable,
ttSatellite,
ttTerrestrial,
ttATSC
} eTunerType;

typedef struct _tagTuner 
{   
  DWORD Frequency;
  DWORD SymbolRate;
  DWORD FEC;
  DWORD Polarity;
  DWORD LNBFreq;
  DWORD LNBSelection;
  DWORD Diseqc;
  DWORD Modulation;
  DWORD BandWidth; // only Terrestrial
  DWORD QAMMode; // only Cable
} TUNER, *PTUNER;

 
HRESULT __stdcall TCallBackFunc(BYTE * buf, int len_in);

HRESULT (* GetTuner) (PTUNER Tuner);
HRESULT (* SetTuner) (PTUNER Tuner);
eTunerType (* GetTunerType) ();
HRESULT (* StopStream) ();
HRESULT (* StartStream) (DWORD CallbackFunction);

typedef HRESULT (__cdecl * td_GetTuner) (PTUNER Tuner);
typedef HRESULT (__cdecl * td_SetTuner) (PTUNER Tuner);
typedef eTunerType (__cdecl * td_GetTunerType) ();
typedef HRESULT (__cdecl * td_StopStream) ();
typedef HRESULT (__cdecl * td_StartStream) (DWORD CallbackFunction);

#ifdef ATSC
// Base PIDs for the ATSC system
long nBasePIDs[] =  {0x0000, 0x0001, 0x1ffb};
#endif ATSC

#ifdef DVBS
// Base PIDs for the DVB system
long nBasePIDs[] =  {0x0000, 0x0001, 0x0010, 0x0011, 0x0012, 0x0013, 0x0014};
#endif DVBS

#ifdef DVBT
// Base PIDs for the DVB system
long nBasePIDs[] =  {0x0000, 0x0001, 0x0010, 0x0011, 0x0012, 0x0013, 0x0014};
#endif DVBT

#ifdef DVBC
// Base PIDs for the DVB system
long nBasePIDs[] =  {0x0000, 0x0001, 0x0010, 0x0011, 0x0012, 0x0013, 0x0014};
#endif DVBC

#ifdef ATSC
 #ifndef QAM
  char gszSourceName[] = {"B2C2 Terrestrial ATSC"};
 #else QAM
  char gszSourceName[] = {"B2C2 Annex B QAM"};
 #endif QAM
#endif ATSC
#ifdef DVBS
char gszSourceName[] = {"B2C2 Satellite DVB-S"};
#endif DVBS
#ifdef DVBT
char gszSourceName[] = {"B2C2 Terrestrial DVB-T"};
#endif DVBT
#ifdef DVBC
char gszSourceName[] = {"B2C2 Cable DVB-C"};
#endif DVBC

// {00E4FCA9-362F-454a-A52D-8AA17460FDE1}
DEFINE_GUID(CLSID_Dump, 
0xe4fca9, 0x362f, 0x454a, 0xa5, 0x2d, 0x8a, 0xa1, 0x74, 0x60, 0xfd, 0xe1);

char szLastTune[128] = {"n/a"};
char szLastSignalReport[128] = {"n/a"};

extern "C" {
	BOOL __cdecl SourceHelper_ValidateSourceContainer(PSOURCESTRUCT pss);
}

HRESULT __stdcall TCallBackFunc(BYTE * buf, int len_in)
{
	char szTemp[128];
	wsprintf(szTemp, "FlexDump: buf = %08x len_in = %d\n", buf, len_in);
	OutputDebugString(szTemp);

	return 0;
}

void AddPIDToPin(int nPIDFilterItem)
{
	HRESULT hr;
	long nSetupPIDCount = 1;

	/*char szTemp[128];
	wsprintf(szTemp, "B2C2: AddPIDToPin 0x%04x\n", PIDFilter[nPIDFilterItem].nPID);
	OutputDebugString(szTemp);*/

	hr = pB2C2FilterDataCtrl->AddPIDsToPin(&nSetupPIDCount, &PIDFilter[nPIDFilterItem].nPID, 0);
	if (FAILED (hr))
	{
		char szTemp[128];
		wsprintf(szTemp, "B2C2: MPEG2 Filter Data Ctrl. Interface AddPIDs method failed (#%08X)!\n", hr);
		OutputDebugString(szTemp);
	}
}

void DeletePIDFromPin(int nPIDFilterItem)
{
	HRESULT hr;

	/*char szTemp[128];
	wsprintf(szTemp, "B2C2: DeletePIDFromPin 0x%04x\n", PIDFilter[nPIDFilterItem].nPID);
	OutputDebugString(szTemp);*/

	hr = pB2C2FilterDataCtrl->DeletePIDsFromPin(1, &PIDFilter[nPIDFilterItem].nPID, 0);
	if (FAILED (hr))
	{
		char szTemp[128];
		wsprintf(szTemp, "B2C2: MPEG2 Filter Data Ctrl. Interface DeletePIDs method failed (#%08X)!\n", hr);
		OutputDebugString(szTemp);
	}
}

BOOL TSReader_Start()
{
	HRESULT hr;
	int nPID = 0;
	int nIndex = 0;

	if (fAlternativeInterface)
	{
		StartStream((DWORD)TCallBackFunc);
		return TRUE;
	}

	// Use the standard B2C2 interface
	hr = pb2c2Adapter->CreateTsFilter(0, CLSID_Dump, NULL);	
	if (FAILED (hr))
	{
		MessageBox(ss->hWndTSReader, "Failed to connect with the transport filter. Is TSReader_B2C2_Dump.ax registered?", gszSourceName, MB_ICONSTOP);
		return FALSE;
	}
	// 1) Get corresponding interface.
	hr = pb2c2Adapter->GetTsInterfaceFilter(0, 
										  IID_IFileSinkFilter, 
										  (IUnknown**) &pFileSink);
	
	if (FAILED (hr))
	{
		char szTemp[128];
		wsprintf(szTemp, "%s on pin 0 failed (#%08X)!\n", 
						 pb2c2Adapter->GetLastErrorText (), 
						 pb2c2Adapter->GetLastError () );
		MessageBox(ss->hWndTSReader, szTemp, gszSourceName, MB_ICONSTOP);
		return FALSE;
	}

	// 2) Set the pathname

	// First convert to an OLE-compliant string, then use that to
	// allocate and initialize a BSTR which COM requires.
	{
		char szTemp[128];
		wsprintf(szTemp, "%x %d", ss, fIsDemuxFree);
		lstrcpy ((char *)szOleFilePathname, szTemp);
	}

	// NB: From this point on, bstrFileName must be freed before exiting
	// program, using FREE_BSTRS() macro.
	bstrFileName = SysAllocString(szOleFilePathname);
	hr = pFileSink->SetFileName(bstrFileName, NULL);
	if (FAILED (hr))
	{
		char szTemp[128];
		wsprintf(szTemp, "IFileSinkFilter Interface SetFileName method failed for Dump Filter 0 (#%08X)!\n", 
						  hr);
		MessageBox(ss->hWndTSReader, szTemp, gszSourceName, MB_ICONSTOP);
		return FALSE;
	}
	else
	{
		char szTemp[128];

		wsprintf(szTemp, "B2C2: TSReader_B2C2.cpp %s\n", (char *)bstrFileName);
		OutputDebugString(szTemp);
		sscanf((char *)bstrFileName, "%x", &pPIDActiveList);
	}

	// ***************************************************************************************
	// Locate B2C2 Filter Data Output pins and connect to Dump Filter input pin:
	// ***************************************************************************************

	// Directly connect B2C2 Filter data output pin 0 to Dump Filter input pin.
	hr = pb2c2Adapter->ConnectTsFilterInToTsOutPin (0);
	if (FAILED (hr))
	{
		char szTemp[128];
		wsprintf(szTemp, "%s on pin 0 failed (#%08X)!\n", 
						 pb2c2Adapter->GetLastErrorText (),  
						 pb2c2Adapter->GetLastError () );
		MessageBox(ss->hWndTSReader, szTemp, gszSourceName, MB_ICONSTOP);
		return FALSE;
	}

	// **********************************************************************
	// *** Run Filter Graph
	// **********************************************************************

	// Get Media Control interface for control of Filter Graph.
	hr = pb2c2Adapter->GetMediaControl (&pMediaControl);
	if (FAILED (hr))
	{
		char szTemp[128];
		wsprintf(szTemp, "%s failed (#%08X)!\n", pb2c2Adapter->GetLastErrorText (), pb2c2Adapter->GetLastError ());
		MessageBox(ss->hWndTSReader, szTemp, gszSourceName, MB_ICONSTOP);
		return FALSE;
	}

	// Start data flow thru Filter Graph
	pMediaControl->Run ();

	if (!fIsDemuxFree)
	{
		// Remove all PIDs first
		for (nPID = 0; nPID < 8191; nPID++)
		{
			long nCount = 1;
			long nDeletePID = nPID;
			hr = pB2C2FilterDataCtrl->DeletePIDsFromPin(nCount, &nDeletePID, 0);
		}

		// Add in startup PIDs -- others will be found in the thread
		nActivePIDCount = sizeof(nBasePIDs) / sizeof(long);
		for (nIndex = 0; nIndex < nActivePIDCount; nIndex++)
		{
			PIDFilter[nIndex].nPID = nBasePIDs[nIndex];
			PIDFilter[nIndex].fTemporary = FALSE;
			PIDFilter[nIndex].nAge = 0;
			AddPIDToPin(nIndex);
		}
	}
	else
	{
		long nSetupPIDCount = 1;
		long nPID = 0x2000;

		hr = pB2C2FilterDataCtrl->AddPIDsToPin(&nSetupPIDCount, &nPID, 0);
		if (FAILED (hr))
		{
			char szTemp[128];
			wsprintf(szTemp, "B2C2: MPEG2 Filter Data Ctrl. Interface AddPIDs method failed (#%08X)!\n", hr);
			OutputDebugString(szTemp);
		}
	}
	
	return TRUE;
}

BOOL TSReader_Stop()
{
	int nIndex = 0;

	pMediaControl->Stop();

	if (!fAlternativeInterface)
	{
		if (!fIsDemuxFree)
		{
			for (nIndex = 0; nIndex < nActivePIDCount; nIndex++)
			{
				DeletePIDFromPin(nIndex);
				PIDFilter[nIndex].nPID = -1;
				PIDFilter[nIndex].fTemporary = TRUE;
				PIDFilter[nIndex].nAge = 0;
			}
			nActivePIDCount = 0;
		}
		else
		{
			HRESULT hr;
			long nPID = 0x2000;

			hr = pB2C2FilterDataCtrl->DeletePIDsFromPin(1, &nPID, 0);
			if (FAILED (hr))
			{
				char szTemp[128];
				wsprintf(szTemp, "B2C2: MPEG2 Filter Data Ctrl. Interface DeletePIDs method failed (#%08X)!\n", hr);
				OutputDebugString(szTemp);
			}
		}
	}
	else
	{
		StopStream();
	}

	ss->fTerminateReadThread = TRUE;
	while (ss->fReadThreadTerminated == FALSE)
		Sleep(50);

	EnterCriticalSection(&ss->csTSBuffersInUse);
	ss->nTSBuffersInUse = -1000;
	LeaveCriticalSection(&ss->csTSBuffersInUse);

	return TRUE;
}

BOOL IsDemuxFree()
{
	DWORD dwVerInfoSize;
	DWORD dwVerHnd = 0;
	BOOL fRetVal = FALSE;
	char szTestFilename[MAX_PATH];

	GetWindowsDirectory(szTestFilename, sizeof(szTestFilename));
	lstrcat(szTestFilename, "\\system32\\drivers\\SkyNET.sys");

	dwVerInfoSize = GetFileVersionInfoSize(szTestFilename, &dwVerHnd);
	if (dwVerInfoSize) 
	{
		LPSTR   lpstrVffInfo;
		HANDLE  hMem;
		UINT uVersionLen;
		LPVOID lpBuffer;			
		BOOL bRetCode;
		static char szGetName[64] = {"\\StringFileInfo\\040904B0\\FileVersion"};

		hMem = GlobalAlloc(GMEM_MOVEABLE, dwVerInfoSize);
		lpstrVffInfo  = (char *)GlobalLock(hMem);
		GetFileVersionInfo(szTestFilename, dwVerHnd, dwVerInfoSize, lpstrVffInfo);
	
		// Get the "FileVersion" information:
		bRetCode = VerQueryValue((LPVOID)lpstrVffInfo,
					             (LPSTR)szGetName,
								 &lpBuffer,
								 (UINT *)&uVersionLen);
		if (bRetCode != 0)
		{
			char * szDriverVersion = (char *)lpBuffer;
			int nMajor, nMinor, nEdit, nPatch;
			int nConversionCount = sscanf(szDriverVersion, "%d,%d,%d,%d", &nMajor, &nMinor, &nEdit, &nPatch);
			if (nConversionCount == 4)
			{
				if (nMajor >= 4)
				{
					if (nMinor >= 3)
						fRetVal = TRUE;
					else
					{
						if (nMinor >= 2)
						{
							if (nEdit >= 12)
							{
								if (nPatch >= 3)
									fRetVal = TRUE;
							}
						}
					}
				}
			}
		}
		GlobalUnlock(hMem);
		GlobalFree(hMem);
	}

	return fRetVal;
}

BOOL TSReader_Init(PSOURCESTRUCT pss)
{
	HRESULT hr;

	if (SourceHelper_ValidateSourceContainer(pss) == FALSE)
		return FALSE;

	ss = pss;

	if (!fAlternativeInterface)
	{
		fIsDemuxFree = IsDemuxFree();
		{
			char szTemp[128];
			wsprintf(szTemp, "B2C2: IsDemuxFree() = %d\n", fIsDemuxFree);
			OutputDebugString(szTemp);
		}
	}
	
	pb2c2Adapter = new B2C2MPEG2Adapter("");

	// Query interfaces; if Initialize succeeded, interfaces are available
	hr = pb2c2Adapter->Initialize ();
	if (FAILED (hr)) 
	{
		char szTemp[128];
		wsprintf(szTemp, "B2C2: %s failed (#%08X)!\n", pb2c2Adapter->GetLastErrorText (), pb2c2Adapter->GetLastError ());
		OutputDebugString(szTemp);
		return FALSE;
	}
	pB2C2FilterTunerCtrl = pb2c2Adapter->GetTunerControl ();
	pB2C2FilterDataCtrl = pb2c2Adapter->GetDataControl ();

	InitializeCriticalSection(&csSignal);
	if (!fAlternativeInterface)
	{
		if (!fIsDemuxFree)
		{
			int nIndex;

			for (nIndex = 0; nIndex < MAX_DEMUX_PIDS; nIndex++)
			{
				PIDFilter[nIndex].nPID = -1;
				PIDFilter[nIndex].nAge = 0;
				PIDFilter[nIndex].fTemporary = TRUE;
			}
			fPIDManagementFailed = FALSE;
		}
	}

	if (fAlternativeInterface)
	{
		int i;
		char szDLLName[MAX_PATH];

		GetModuleFileName(ss->hTSReaderInst, szDLLName, sizeof(szDLLName));
		for (i = lstrlen(szDLLName); i > 0; i--)
		{
			if (szDLLName[i] == '\\')
			{
				szDLLName[i + 1] = 0;
				break;
			}
		}
		lstrcat(szDLLName, "Sources\\FlexDump.dll");
		hFlexDump = LoadLibrary(szDLLName);
		if (hFlexDump != NULL)
		{
			GetTuner = (td_GetTuner)GetProcAddress(hFlexDump, "GetTuner");
			SetTuner = (td_SetTuner)GetProcAddress(hFlexDump, "SetTuner");
			GetTunerType = (td_GetTunerType)GetProcAddress(hFlexDump, "GetTunerType");
			StopStream = (td_StopStream)GetProcAddress(hFlexDump, "StopStream");
			StartStream = (td_StartStream)GetProcAddress(hFlexDump, "StartStream");
		}
		else
			return FALSE;
	}
	return TRUE;
}

BOOL TSReader_DeInit()
{
	pb2c2Adapter->Release();
	delete pb2c2Adapter;
	DeleteCriticalSection(&csSignal);
	return TRUE;
}

void SetupLastTune()
{
#ifdef DVBS
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
#endif DVBS
#ifdef ATSC
 #ifndef QAM
	wsprintf(szLastTune, "Channel %d (%d MHz)", SourceHelper_GetATSCChannelFromFrequency(ss->nFrequency), ss->nFrequency);
 #else QAM
	wsprintf(szLastTune, "Channel %d (%d MHz)", SourceHelper_GetQAMChannelFromFrequency(ss->nFrequency), ss->nFrequency);
 #endif QAM
#endif ATSC
#ifdef DVBT
	sprintf(szLastTune, "%.3f MHz", (double)ss->nFrequency / 1000.0);
#endif DVBT
#ifdef DVBC
	char szQAMMode[16] = {0};

	switch(ss->nQAM)
	{
	case 0:
		lstrcpy(szQAMMode, "QAM16");
		break;
	case 1:
		lstrcpy(szQAMMode, "QAM32");
		break;
	case 2:	
		lstrcpy(szQAMMode, "QAM64");
		break;
	case 3:
		lstrcpy(szQAMMode, "QAM128");
		break;
	case 4:
		lstrcpy(szQAMMode, "QAM256");
		break;
	}
	sprintf(szLastTune, "%.1f MHz %s", (double)ss->nFrequency / 1000.0, szQAMMode);
#endif DVBC
}

BOOL TSReader_Tune()
{
	HRESULT hr;

	SetupLastTune();

#ifdef ATSC
	pB2C2FilterTunerCtrl->SetFrequencyKHz(ss->nFrequency * 1000);	
#endif ATSC

#ifdef DVBS
	pB2C2FilterTunerCtrl->SetFrequencyKHz(ss->nFrequency * 1000);
	pB2C2FilterTunerCtrl->SetSymbolRate(ss->nSymbolRate);
	pB2C2FilterTunerCtrl->SetLnbFrequency(ss->nLNBFrequency);

	pB2C2FilterTunerCtrl->SetFec(FEC_AUTO);
	if (ss->nPolarity == 0)
		pB2C2FilterTunerCtrl->SetPolarity(POLARITY_VERTICAL);
	else
		pB2C2FilterTunerCtrl->SetPolarity(POLARITY_HORIZONTAL);

	if (ss->n22KHz == 0)
		pB2C2FilterTunerCtrl->SetLnbKHz(LNB_SELECTION_0);
	else
		pB2C2FilterTunerCtrl->SetLnbKHz(LNB_SELECTION_22);
 
	switch(ss->nDiSEqCInput)
	{
	case 0:
		break;
	case 1:
		pB2C2FilterTunerCtrl->SetDiseqc(DISEQC_LEVEL_1_A_A);
		break;
	case 2:
		pB2C2FilterTunerCtrl->SetDiseqc(DISEQC_LEVEL_1_B_A);
		break;
	case 3:
		pB2C2FilterTunerCtrl->SetDiseqc(DISEQC_LEVEL_1_A_B);
		break;
	case 4:
		pB2C2FilterTunerCtrl->SetDiseqc(DISEQC_LEVEL_1_B_B);
		break;
	case 5:
		pB2C2FilterTunerCtrl->SetDiseqc(DISEQC_SIMPLE_A);
		break;
	case 6:
		pB2C2FilterTunerCtrl->SetDiseqc(DISEQC_SIMPLE_B);
		break;
	}
#endif DVBS

#ifdef DVBT
	pB2C2FilterTunerCtrl->SetFrequencyKHz(ss->nFrequency);	
	pB2C2FilterTunerCtrl->SetGuardInterval(GUARD_INTERVAL_AUTO);
	switch(ss->nBandwidth)
	{
	case 0:
		pB2C2FilterTunerCtrl->SetPolarity(6);
		break;
	case 1:
		pB2C2FilterTunerCtrl->SetPolarity(7);
		break;
	case 2:
		pB2C2FilterTunerCtrl->SetPolarity(8);
		break;
	}
	{
		char szTemp[128];
		wsprintf(szTemp, "B2C2: DVBT tune %d KHz bandwidth-index %d\n", ss->nFrequency, ss->nBandwidth);
		OutputDebugString(szTemp);
	}
#endif DVBT

#ifdef DVBC
	pB2C2FilterTunerCtrl->SetFrequencyKHz(ss->nFrequency);	
	pB2C2FilterTunerCtrl->SetSymbolRate(ss->nSymbolRate);
	switch(ss->nQAM)
	{
	case 0:
		pB2C2FilterTunerCtrl->SetModulation(QAM_16);
		break;
	case 1:
		pB2C2FilterTunerCtrl->SetModulation(QAM_32);
		break;
	case 2:
		pB2C2FilterTunerCtrl->SetModulation(QAM_64);
		break;
	case 3:
		pB2C2FilterTunerCtrl->SetModulation(QAM_128);
		break;
	case 4:
		pB2C2FilterTunerCtrl->SetModulation(QAM_256);
		break;
	}
	{
		char szTemp[128];
		wsprintf(szTemp, "B2C2: DVBC tune %d KHz symbol rate %d QAM index %d\n", ss->nFrequency, ss->nSymbolRate, ss->nQAM);
		OutputDebugString(szTemp);
	}
#endif DVBC

	hr = pB2C2FilterTunerCtrl->SetTunerStatus();
	if (FAILED(hr))
	{
		if (ss->fQuietMode == FALSE)
			MessageBox(ss->hWndTSReader, "Failed to lock signal (a)", gszSourceName, MB_OK | MB_ICONWARNING);
		return FALSE;
	}

	hr = pB2C2FilterTunerCtrl->CheckLock();
	if (FAILED(hr))
	{
		if (ss->fQuietMode == FALSE)
			MessageBox(ss->hWndTSReader, "Failed to lock signal (b)", gszSourceName, MB_OK | MB_ICONWARNING);
		return FALSE;
	}
	
	return TRUE;
}

BOOL TSReader_TuneDialog(HWND hWnd)
{
	if (ss->fDontTune == TRUE)
	{
		fNeedTuneDialog = FALSE;
		OutputDebugString("B2C2: TuneDialog() set fNeedTuneDialog = FALSE\n");
	}

	{
		char szTemp[100];
		wsprintf(szTemp, "B2C2: fNeedTuneDialog = %d\n", fNeedTuneDialog);
		OutputDebugString(szTemp);
	}
	if (fNeedTuneDialog)
	{
#ifdef ATSC
 #ifndef QAM
		if (SourceHelper_ATSCTuneDialog(hWnd) == FALSE)
 #else QAM
		if (SourceHelper_QAMTuneDialog(hWnd) == FALSE)
 #endif QAM
#endif ATSC
#ifdef DVBS
		if (SourceHelper_DVBSTuneDialog(hWnd) == FALSE)
#endif DVBS
#ifdef DVBT
		if (SourceHelper_DVBTTuneDialog(hWnd) == FALSE)
#endif DVBT
#ifdef DVBC
		if (SourceHelper_DVBCTuneDialog(hWnd) == FALSE)
#endif DVBC
			return FALSE;
	}
	else
	{
		ss->nFrequency = nFrequency;
#ifdef DVBS
		ss->nPolarity = nPolarity;
		ss->nSymbolRate = nSymbolRate;
		ss->nLNBFrequency = nLNBFrequency;
		ss->n22KHz = n22KHz;
		ss->nDiSEqCInput = nDiSEqCInput;
#endif DVBS
#ifdef DVBT
		ss->nBandwidth = nBandwidth;
#endif DVBT
#ifdef DVBC
		ss->nSymbolRate = nSymbolRate;
		ss->nQAM = nQAM;
		ss->fSpectrumInversion = fSpectrumInversion;
		ss->nBandwidth = nBandwidth;
#endif DVBC
		if (ss->fQuietMode == FALSE)
		{
			fNeedTuneDialog = TRUE;
			OutputDebugString("B2C2: TuneDialog() set fNeedTuneDialog = TRUE\n");
		}
	}

	return TRUE;
}

BOOL TSReader_PIDManagement(BOOL fAdd, int nPID, BOOL fTemporary)
{
	if (!fIsDemuxFree)
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
					//DeletePIDFromPin(nIndex);
					//PIDFilter[nIndex].nPID = -1;
					PIDFilter[nIndex].fTemporary = TRUE;
					//PIDFilter[nIndex].nAge = 0;
					return TRUE;
				}
			}
		}
	}

	return TRUE;
}

BOOL TSReader_GetDescription(char * szDescription, char * szCommandLineParameters, BOOL * fCanBeStopped, int * nMaxPIDs, DWORD * dwCapabilities)
{
	if (szDescription != NULL)
		lstrcpy(szDescription, gszSourceName);
#ifdef ATSC
	if (szCommandLineParameters != NULL)
		lstrcpy(szCommandLineParameters, "freq");
	if (dwCapabilities != NULL)
		*dwCapabilities = CAPABILITIES_UNIPROCESSOR;
#endif ATSC
#ifdef DVBS
	if (szCommandLineParameters != NULL)
		lstrcpy(szCommandLineParameters, "freq pol sr lnbf 22khz {input}");
	if (dwCapabilities != NULL)
		*dwCapabilities = CAPABILITIES_DISEQC
		                | CAPABILITIES_TONEBURST
						| CAPABILITIES_POWER
						| CAPABILITIES_UNIPROCESSOR;
#endif DVBS
#ifdef DVBT
		lstrcpy(szCommandLineParameters, "freq inversion bandwidth");
	if (dwCapabilities != NULL)
		*dwCapabilities = CAPABILITIES_UNIPROCESSOR;
#endif DVBT
#ifdef DVBC
		lstrcpy(szCommandLineParameters, "freq sr QAM inversion bandwidth");
	if (dwCapabilities != NULL)
		*dwCapabilities = CAPABILITIES_UNIPROCESSOR;
#endif DVBC
	if (fCanBeStopped != NULL)
		*fCanBeStopped = FALSE;
	if (nMaxPIDs != NULL)
	{
		if (!IsDemuxFree())
			*nMaxPIDs = MAX_DEMUX_PIDS;
		else
			*nMaxPIDs = 8192;
	}

	return TRUE;
}

#ifdef ATSC
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
					   "Usage for this source: freq\n"
					   "\n"
					   "freq = frequency to tune in MHz or prefix with 0 for channel number, e.g. 022 for channel 22",
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
#endif ATSC

#ifdef DVBS
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
#endif DVBS

#ifdef DVBT
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
#endif DVBT

#ifdef DVBC
BOOL TSReader_ParseCommandLine(PSOURCESTRUCT ss, char * szCommandLine, BOOL fQuiet)
{
	fNeedTuneDialog = TRUE;
	OutputDebugString("B2C2: TSReader_ParseCommandLine() set fNeedTuneDialog = TRUE\n");

	if (lstrlen(szCommandLine))
	{
		int nConversionCount;

		SourceHelper_ConvertPolarity(szCommandLine);
		ss->nDiSEqCInput = 0;
		nConversionCount = sscanf(szCommandLine,
								  "%d %d %d %d %d", 
								  &nFrequency,
								  &nSymbolRate,
								  &nQAM,
								  &fSpectrumInversion,
								  &nBandwidth);
		if (nConversionCount < 5)
		{
			if (!fQuiet)
				MessageBox(NULL,
					   "Usage for this source: freq sr QAM inversion bandwidth\n"
					   "\n"
					   "freq = frequency to tune in KHz\n"
					   "sr = symbol rate\n"
					   "QAM = QAM Mode (0=QAM-16 1=QAM-32 2=QAM-64 3=QAM-128 4=QAM-256)\n"
					   "inversion = inverted spectrum (0 or 1)\n"
					   "bandwidth = bandwidth of signal (0 = 6, 1 = 7, 2 = 8 MHz)",
					   gszSourceName,
					   MB_OK | MB_ICONSTOP);
			return FALSE;
		}
		else
		{
			fNeedTuneDialog = FALSE;
			OutputDebugString("B2C2: NO TUNE DIALOG\n");
		}
	}
	else
	{
		fNeedTuneDialog = TRUE;
		OutputDebugString("B2C2: TUNE DIALOG\n");
	}

	return TRUE;
}
#endif DVBC

BOOL TSReader_IsPIDActive(int nPID)
{
	if (!fIsDemuxFree)
	{
		int nIndex;

		for (nIndex = 0; nIndex < nActivePIDCount; nIndex++)
		{
			if (PIDFilter[nIndex].nPID == nPID)
				return TRUE;
		}

		return FALSE;
	}

	return TRUE;
}

BOOL TSReader_SetChannel(int nChannel)
{
	return TRUE;
}

BOOL TSReader_GetSignalString(char * szString)
{
	HRESULT hr;
	float fSNR;
	char szLockStatus[16];

	hr = pB2C2FilterTunerCtrl->CheckLock();
	if (FAILED(hr))
		lstrcpy(szLockStatus, "Unlocked");
	else
		lstrcpy(szLockStatus, "Locked");

	hr = pB2C2FilterTunerCtrl->GetSNR(&fSNR);
	
	sprintf(szString, "%s SNR: %.1f dB", szLockStatus, fSNR);
	return TRUE;
}

BOOL TSReader_GetTunerString(char * szString)
{
	lstrcpy(szString, szLastTune);
	return TRUE;
}

BOOL TSReader_SendDiSEqC(BYTE * bCommand, int nLength)
{
	// todo
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
