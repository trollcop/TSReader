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
BOOL fStarted = FALSE;
HANDLE hInstance;
CRITICAL_SECTION csSignal;

int nFrequency;
#ifdef DVBS
int nPolarity;
int nSymbolRate;
int nLNBFrequency;
int n22KHz;
int nDiSEqCInput = FALSE;
BOOL fDiSEqC12;
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
IB2C2MPEG2DataCtrl6 	*pB2C2FilterDataCtrl = NULL;
IB2C2MPEG2TunerCtrl4 	*pB2C2FilterTunerCtrl = NULL;
IFileSinkFilter			*pFileSink = NULL;
IMediaControl			*pMediaControl = NULL;

#define DUMPFILEPATHNAME_SZ		1024			// SParams.szDumpFilePathname string size
OLECHAR					szOleFilePathname[DUMPFILEPATHNAME_SZ];
BSTR					bstrFileName;

#ifdef ATSC
 #ifndef QAM
  char gszSourceName[] = {"Technisat/B2C2 8VSB"};
 #else QAM
  char gszSourceName[] = {"Technisat/B2C2 QAM-B"};
 #endif QAM
#endif ATSC
#ifdef DVBS
char gszSourceName[] = {"Technisat/B2C2 DVB-S"};
#endif DVBS
#ifdef DVBT
char gszSourceName[] = {"Technisat/B2C2 DVB-T"};
#endif DVBT
#ifdef DVBC
char gszSourceName[] = {"Technisat/B2C2 DVB-C"};
#endif DVBC

// {00E4FCA9-362F-454a-A52D-8AA17460FDE1}
DEFINE_GUID(CLSID_Dump, 
0xe4fca9, 0x362f, 0x454a, 0xa5, 0x2d, 0x8a, 0xa1, 0x74, 0x60, 0xfd, 0xe1);

char szLastTune[128] = {"n/a"};
char szLastSignalReport[128] = {"n/a"};

extern "C" {
	BOOL __cdecl SourceHelper_ValidateSourceContainer(PSOURCESTRUCT pss);
}

BOOL TSReader_Start()
{
	HRESULT hr;
	int nIndex = 0;
	long nSetupPIDCount = 1;
	long nPID = 0x2000;

	// Use the standard B2C2 interface
	hr = pb2c2Adapter->CreateTsFilter(0, CLSID_Dump, NULL);	
	if (FAILED (hr))
	{
		MessageBox(NULL, "Failed to connect with the transport filter. Is TSReader_B2C2_Dump.ax registered?", gszSourceName, MB_ICONSTOP);
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
		MessageBox(NULL, szTemp, gszSourceName, MB_ICONSTOP);
		return FALSE;
	}

	// 2) Set the pathname

	// First convert to an OLE-compliant string, then use that to
	// allocate and initialize a BSTR which COM requires.
	{
		char szTemp[128];
		wsprintf(szTemp, "%x", ss);
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
		MessageBox(NULL, szTemp, gszSourceName, MB_ICONSTOP);
		return FALSE;
	}
	else
	{
		char szTemp[128];

		wsprintf(szTemp, "B2C2: TSReader_B2C2.cpp %s\n", (char *)bstrFileName);
		OutputDebugString(szTemp);
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
		MessageBox(NULL, szTemp, gszSourceName, MB_ICONSTOP);
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
		MessageBox(NULL, szTemp, gszSourceName, MB_ICONSTOP);
		return FALSE;
	}

	// Start data flow thru Filter Graph
	pMediaControl->Run ();


	hr = pB2C2FilterDataCtrl->AddPIDsToPin(&nSetupPIDCount, &nPID, 0);
	if (FAILED (hr))
	{
		char szTemp[128];
		wsprintf(szTemp, "B2C2: MPEG2 Filter Data Ctrl. Interface AddPIDs method failed (#%08X)!\n", hr);
		OutputDebugString(szTemp);
	}

	fStarted = TRUE;
	return TRUE;
}

BOOL TSReader_Stop()
{
	if (fStarted)
	{
		int nIndex = 0;
		HRESULT hr;
		long nPID = 0x2000;

		hr = pB2C2FilterDataCtrl->DeletePIDsFromPin(1, &nPID, 0);
		if (FAILED (hr))
		{
			char szTemp[128];
			wsprintf(szTemp, "B2C2: MPEG2 Filter Data Ctrl. Interface DeletePIDs method failed (#%08X)!\n", hr);
			OutputDebugString(szTemp);
		}
		pMediaControl->Stop();
	}

	ss->fTerminateReadThread = TRUE;
	while (ss->fReadThreadTerminated == FALSE)
		Sleep(50);

	EnterCriticalSection(&ss->csTSBuffersInUse);
	ss->nTSBuffersInUse = -1000;
	LeaveCriticalSection(&ss->csTSBuffersInUse);

	fStarted = FALSE;
	return TRUE;
}

BOOL TSReader_Init(PSOURCESTRUCT pss)
{
	HRESULT hr;

	if (SourceHelper_ValidateSourceContainer(pss) == FALSE)
		return FALSE;
	fStarted = FALSE;
	InitializeCriticalSection(&csSignal);

	ss = pss;

	pb2c2Adapter = new B2C2MPEG2Adapter("");

	// Query interfaces; if Initialize succeeded, interfaces are available
	hr = pb2c2Adapter->Initialize ();
	if (FAILED (hr)) 
	{
		char szTemp[128];
		wsprintf(szTemp, "B2C2: %s failed (#%08X)!\n", pb2c2Adapter->GetLastErrorText (), pb2c2Adapter->GetLastError ());
		OutputDebugString(szTemp);
		if (strstr(szTemp, "IB2C2MPEG2DataCtrl6") != NULL)
			MessageBox(NULL, "Before using this source you must upgrade your card's drivers. Please visit:\n\nhttp://www.bbti.us\n\tor\nhttp://www.t-data.lu\n\nDownload version 4.4 or later of the driver, install and try again.", gszSourceName, MB_ICONSTOP);
		return FALSE;
	}
	pB2C2FilterDataCtrl = pb2c2Adapter->GetDataControl ();
	pB2C2FilterTunerCtrl = pb2c2Adapter->GetTunerControl ();

	// Get the device list so we can find the right interface
	{
		int nSourceIndex = ss->nSourceIndex;
		long lSize;
		DWORD dwDeviceCount = MAX_DEVICES_SUPPORTED;
		tDEVICE_INFORMATION ListOfDevices[MAX_DEVICES_SUPPORTED];

		lSize = sizeof(ListOfDevices);
		pB2C2FilterDataCtrl->GetDeviceList(ListOfDevices, &lSize, &dwDeviceCount);
		if (dwDeviceCount)
		{
			int nDeviceIndex;

			for (nDeviceIndex = 0; nDeviceIndex < (int)dwDeviceCount; nDeviceIndex++)
			{
#ifdef ATSC
				if (ListOfDevices[nDeviceIndex].eTunerModulation == TUNER_TERRESTRIAL_ATSC)
#endif ATSC
#ifdef DVBS
				if (ListOfDevices[nDeviceIndex].eTunerModulation == TUNER_SATELLITE)
#endif DVBS
#ifdef DVBC
				if (ListOfDevices[nDeviceIndex].eTunerModulation == TUNER_CABLE)
#endif DVBC
#ifdef DVBT
				if (ListOfDevices[nDeviceIndex].eTunerModulation == TUNER_TERRESTRIAL_DVB)
#endif DVBT
				{
					if (nSourceIndex == 0)
						break;
					if (nSourceIndex)
						nSourceIndex--;
				}
			}
			if (nDeviceIndex == (int)dwDeviceCount)
			{
				MessageBox(NULL, "Unable to locate a Technisat/B2C2 card of the type specified", gszSourceName, MB_ICONSTOP);
				return FALSE;
			}
			hr = pB2C2FilterDataCtrl->SelectDevice(ListOfDevices[nDeviceIndex].dwDeviceID);
			
		}
		else
		{
			MessageBox(NULL, "Unable to locate any Technisat/B2C2 interfaces", gszSourceName, MB_ICONSTOP);
			return FALSE;
		}
	}

	{
		long lLength = 0;
		tTunerCapabilities tc;

#ifdef DVBS
		fDiSEqC12 = FALSE;
#endif DVBS
		hr = pB2C2FilterTunerCtrl->GetTunerCapabilities(&tc, &lLength);
		if (SUCCEEDED(hr))
		{
#ifdef DVBS
			if (tc.dwAcquisitionCapabilities & ACQUISITION_DISEQC_12)
				fDiSEqC12 = TRUE;
#endif DVBS
		}
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
	int nTuneLoopCounter = 0;

	SetupLastTune();

#ifdef ATSC
	pB2C2FilterTunerCtrl->SetFrequencyKHz(ss->nFrequency * 1000);	
 #ifdef QAM
	nTuneLoopCounter = 1;
 #endif QAM
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

	do
	{
		hr = pB2C2FilterTunerCtrl->SetTunerStatus();
		if (FAILED(hr) && nTuneLoopCounter == 0)
		{
			if (ss->fQuietMode == FALSE)
				MessageBox(NULL, "Failed to lock signal (a)", gszSourceName, MB_OK | MB_ICONWARNING);
			return FALSE;
		}
		else
			break;
		nTuneLoopCounter--;
	} while (nTuneLoopCounter >= 0);
#ifdef QAM
	nTuneLoopCounter = 0;
	do
	{
		hr = pB2C2FilterTunerCtrl->CheckLock();
		if (FAILED(hr)  && nTuneLoopCounter == 0)
		{
			if (ss->fQuietMode == FALSE)
				MessageBox(NULL, "Failed to lock signal (b)", gszSourceName, MB_OK | MB_ICONWARNING);
			return FALSE;
		}
		else
			break;
		nTuneLoopCounter--;
	} while (nTuneLoopCounter >= 0);
#endif QAM

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
		*dwCapabilities = CAPABILITIES_UNIPROCESSOR
		                | CAPABILITIES_MULTICARD;
#endif ATSC
#ifdef DVBS
	if (szCommandLineParameters != NULL)
		lstrcpy(szCommandLineParameters, "freq pol sr lnbf 22khz {input}");
	if (dwCapabilities != NULL)
	{
		*dwCapabilities = CAPABILITIES_DISEQC
//			            | CAPABILITIES_DISEQC_POSITIONER
		                | CAPABILITIES_TONEBURST
						| CAPABILITIES_POWER
						| CAPABILITIES_MULTICARD
						| CAPABILITIES_UNIPROCESSOR;
		if (fDiSEqC12)
			*dwCapabilities |= CAPABILITIES_DISEQC_POSITIONER;
	}
#endif DVBS
#ifdef DVBT
		lstrcpy(szCommandLineParameters, "freq inversion bandwidth");
	if (dwCapabilities != NULL)
		*dwCapabilities = CAPABILITIES_UNIPROCESSOR
		                | CAPABILITIES_MULTICARD;
#endif DVBT
#ifdef DVBC
		lstrcpy(szCommandLineParameters, "freq sr QAM inversion bandwidth");
	if (dwCapabilities != NULL)
		*dwCapabilities = CAPABILITIES_UNIPROCESSOR
		                | CAPABILITIES_MULTICARD;
#endif DVBC
	if (fCanBeStopped != NULL)
		*fCanBeStopped = FALSE;
	if (nMaxPIDs != NULL)
		*nMaxPIDs = 8192;

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
#ifdef DVBS
//	pB2C2FilterTunerCtrl->SendDiSEqCCommand(nLength, bCommand);
#endif DVBS
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
