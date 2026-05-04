#include <windows.h>
#include <commctrl.h>
#include "initguid.h"
#include "winioctl.h"
#include "setupapi.h"	// VC++ 5 one is out of date
#include <stdio.h>

#include "..\sources.h"

#include "ATDV_API.H"
#include <algorithm>

#include "resource.h"

CATBoard *pBoard;
CAtBoardManager *pMan;

PSOURCESTRUCT ss;

HINSTANCE hInstance;
BOOL fNeedTuneDialog = TRUE;
int nDeviceCount;
CRITICAL_SECTION csSignal;
char szLastSignal[128] = {"n/a"};
char szLastTune[128] = {"n/a"};

#ifdef ASISPI
int nInputSelect = 0;
BOOL fDontAskMode = FALSE;
BOOL fNoInputSelect = FALSE;
char gszSourceName[] = {"Alitronika AT200/AT400/AT4"};
char gszAlitronikaKeyName[] = {"Software\\COOL.STF\\TSReader\\Alitronika"};
#else ASISPI
int nFrequency;
#endif ASISPI

#ifdef DVBS
int nPolarity;
int nSymbolRate;
int nLNBFrequency;
int n22KHz;
int nDiSEqCInput;
char gszSourceName[] = {"Alitronika AT600 DVB-S"};
#endif DVBS

#ifdef DVBT
int nBandwidth;
BOOL fSpectrumInversion;
char gszSourceName[] = {"Alitronika AT800 DVB-T"};
#endif DVBT

#ifdef DVBC
int nQAM;
int nSymbolRate;
int nBandwidth;
BOOL fSpectrumInversion;
char gszSourceName[] = {"Alitronika AT700 DVB-C"};
#endif DVBC

#ifdef EIGHTVSB
char gszSourceName[] = {"Alitronika AT720 8VSB Mode"};
#endif EIGHTVSB

#ifdef QAM
char gszSourceName[] = {"Alitronika AT720 QAM Mode"};
#endif QAM

#define RECORD_BUFFERSIZE (64 * 1024)
#define STATUS_MS			50

#ifdef ASISPI
void LoadAlitronikaSettings()
{
	DWORD dwDisposition;
	DWORD dwDataSize;
	DWORD dwType;
	LONG lKey;
	HKEY hkMainReg;

	lKey = RegCreateKeyEx(HKEY_CURRENT_USER,
		                  gszAlitronikaKeyName,
						  0,
						  gszSourceName,
					      REG_OPTION_NON_VOLATILE,
						  KEY_ALL_ACCESS,
						  NULL,
						  &hkMainReg,
						  &dwDisposition);
	if (lKey == ERROR_SUCCESS)
	{
		if (dwDisposition != REG_CREATED_NEW_KEY)
		{
			dwDataSize = sizeof(nInputSelect);
			RegQueryValueEx(hkMainReg, "InputSelect", NULL, &dwType, (BYTE *)&nInputSelect, &dwDataSize);
			dwDataSize = sizeof(fDontAskMode);
			RegQueryValueEx(hkMainReg, "DontAskMode", NULL, &dwType, (BYTE *)&fDontAskMode, &dwDataSize);
		}
		RegCloseKey(hkMainReg);
	}
}

void SaveAlitronikaSettings()
{
	LONG lKey;
	HKEY hkMainReg;
	DWORD dwDisposition;

	lKey = RegCreateKeyEx(HKEY_CURRENT_USER,
		                  gszAlitronikaKeyName,
						  0,
						  gszSourceName,
					      REG_OPTION_NON_VOLATILE,
						  KEY_ALL_ACCESS,
						  NULL,
						  &hkMainReg,
						  &dwDisposition);
	if (lKey == ERROR_SUCCESS)
	{
		RegSetValueEx(hkMainReg, "InputSelect", 0, REG_DWORD, (BYTE *)&nInputSelect, sizeof(DWORD));
		RegSetValueEx(hkMainReg, "DontAskMode", 0, REG_DWORD, (BYTE *)&fDontAskMode, sizeof(DWORD));
		RegCloseKey(hkMainReg);
	}
}

BOOL CALLBACK InputModeDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg)
	{
	case WM_INITDIALOG:
		switch(nInputSelect)
		{
		case 0:
			CheckDlgButton(hDlg, IDC_INPUT_DVB_ASI, BST_CHECKED);
			break;
		case 1:
			CheckDlgButton(hDlg, IDC_INPUT_DVB_SPI, BST_CHECKED);
			break;
		}
		SetFocus(GetDlgItem(hDlg, IDOK));
		break;
	case WM_CLOSE:
		EndDialog(hDlg, FALSE);
		break;
	case WM_COMMAND:
		switch(LOWORD(wParam))
		{
		case IDOK:
			if (IsDlgButtonChecked(hDlg, IDC_INPUT_DVB_ASI))
				nInputSelect = 0;
			else if (IsDlgButtonChecked(hDlg, IDC_INPUT_DVB_SPI))
				nInputSelect = 1;
			fDontAskMode = IsDlgButtonChecked(hDlg, IDC_DONT_ASK_AGAIN);
			EndDialog(hDlg, TRUE);
			break;
		case IDCANCEL:
			EndDialog(hDlg, FALSE);
			break;
		}
		break;
	}

	return FALSE;
}
#endif ASISPI

DWORD WINAPI AlitronikaReadThread(LPVOID lpv)
{
	int nLockStatusCounter = 200;
	DWORD dwBytes;
	BYTE * pBuffer = (BYTE *)LocalAlloc(LPTR, RECORD_BUFFERSIZE);

	OutputDebugString("AlitronikaReadThread+\n");
	SourceHelper_StartSyncThread(ss, FALSE);
	pBoard->StartRecording();
	while (!ss->fTerminateReadThread)
	{
		dwBytes = 0;
		if (pBoard->GetRecordPacketDirect(pBuffer, RECORD_BUFFERSIZE, &dwBytes) == FALSE)
			break;
		if (dwBytes)
		{
			SourceHelper_SyncData(pBuffer, dwBytes);
		}
		else
		{
			Sleep(1);
#ifndef ASISPI
			if (nLockStatusCounter++ > 200)
			{
				int nBitrateError;
				int status;
				double dBER;
				char szLockStatus[16];

			    nLockStatusCounter = 0;
				pBoard->DvbSource(SRC_READ_BER, &nBitrateError);
				//dBER = (nBitrateError * 100.0) / 0xffff;
				dBER = 1.0 / nBitrateError;
				pBoard->DvbSource(SRC_READ_STATUS, &status);
				if (status & SRC_HAS_LOCK)
					lstrcpy(szLockStatus, "Locked");
				else
					lstrcpy(szLockStatus, "Unlocked");
				
				EnterCriticalSection(&csSignal);
				sprintf(szLastSignal, "%s: BER %0.1E", szLockStatus, dBER);
				LeaveCriticalSection(&csSignal);
			}
#endif ASISPI
		}
	}
	pBoard->StopRecording();
	SourceHelper_StopSyncThread();

	LocalFree(pBuffer);
	ss->fReadThreadTerminated = TRUE;
	OutputDebugString("AlitronikaReadThread-\n");
	return 0;
}

BOOL TSReader_Start()
{
	DWORD dwThreadID;

	ss->fTerminateReadThread = FALSE;
	ss->hReadDataThread = CreateThread(NULL, 0, AlitronikaReadThread, (LPVOID)0, CREATE_SUSPENDED, &dwThreadID);
	SourceHelper_SetWorkerThreadPriorities(FALSE);
	ResumeThread(ss->hReadDataThread);
	return TRUE;
}

BOOL TSReader_Stop()
{
	// Terminate our read thread
	ss->fTerminateReadThread = TRUE;
	while (ss->fReadThreadTerminated == FALSE)
		Sleep(50);

	// Tell the main TSReader input thread that's now terminated
	EnterCriticalSection(&ss->csTSBuffersInUse);
	ss->nTSBuffersInUse = -1000;
	LeaveCriticalSection(&ss->csTSBuffersInUse);

	return TRUE;
}

void PrintDev(char *pDevName)
{
	char szTemp[256];

	wsprintf(szTemp, "TSReader_ATxxx: device name %s\n", pDevName);
	OutputDebugString(szTemp);

	nDeviceCount++;
}

#ifdef ASISPI
static BYTE bValidDevices[] = {
	AT4,
	AT20,			// Mini ASI
//	AT22,			// Mini LVDS
	AT40,			// Mini ASI I/O
	AT200,			// USB/PCI ASI
	AT400,			// USB/PCI ASI I/O
	ATDEMO
};
#endif ASISPI
#ifdef DVBS
static BYTE bValidDevices[] = {
	AT60,			// Mini DVB-S
	AT600,			// USB/PCI DVB-S
	ATDEMO
};
#endif DVBS
#ifdef DVBC
static BYTE bValidDevices[] = {
	AT70,			// Mini DVB-C
	AT700,			// USB/PCI DVB-C
	ATDEMO
};
#endif DVBC
#ifdef DVBT
static BYTE bValidDevices[] = {
	AT80,			// Mini DVB-T
	AT800,			// USB/PCI DVB-T
	ATDEMO
};
#endif DVBT
#ifdef EIGHTVSB
static BYTE bValidDevices[] = {
	AT72,			// Mini 8VSB/QAM
	AT720,			// USB/PCI 8VSB/QAM
	ATDEMO
};
#endif EIGHTVSB
#ifdef QAM
static BYTE bValidDevices[] = {
	AT72,			// Mini 8VSB/QAM
	AT720,			// USB/PCI 8VSB/QAM
	ATDEMO
};
#endif QAM

BOOL TSReader_Init(PSOURCESTRUCT pss)
{
	int nDevice;
    CAtDeviceList devlist;

	ss = pss;
	InitializeCriticalSection(&csSignal);
#ifdef ASISPI
	fNoInputSelect = FALSE;
#endif ASISPI
	
    pMan = CAtBoardManager::Instance();
    pMan->GetDeviceList(devlist);
	nDeviceCount  = 0;
   	std::for_each(devlist.Names.begin(), devlist.Names.end(), PrintDev);
	if (nDeviceCount == 0)
	{
		MessageBox(NULL, "Unable to locate any Alitronika devices", gszSourceName, MB_ICONSTOP);
		return FALSE;
	}

	for (nDevice = 0; nDevice < nDeviceCount; nDevice++)
	{
		int nDeviceType;
	    ATDEVICEINFO iDevice;

		pBoard = pMan->GetBoard(devlist.Names[nDevice]);
		if (pBoard == NULL)
			continue;
	    pBoard->GetDeviceInfo(&iDevice);

		for (nDeviceType = 0; bValidDevices[nDeviceType] != ATDEMO; nDeviceType++)
		{
			if (iDevice.DeviceType == bValidDevices[nDeviceType])
			{
#ifdef ASISPI
				if (iDevice.DeviceType == AT4)
					fNoInputSelect = TRUE;
#endif ASISPI
				break;	// found it!
			}
		}
		if (bValidDevices[nDeviceType] != ATDEMO)
			break;
		pMan->ReleaseBoard(pBoard);
		pBoard = NULL;
	}
	if (nDevice == nDeviceCount)
	{
		MessageBox(NULL, "Unable to locate a suitable Alitronika interface", gszSourceName, MB_ICONSTOP);
		return FALSE;
	}

	if (!pBoard->IsUsbDeviceHighSpeed())
	{
		MessageBox(NULL, "Device must be connected to a USB 2.0 port.", gszSourceName, MB_ICONSTOP);
		return FALSE;
	}

	// Upload the correct FPGA file
	pBoard->ProgramFPGA(REC_DVB);

	return TRUE;
}

BOOL TSReader_DeInit()
{
	if (pBoard != NULL)
		pMan->ReleaseBoard(pBoard);

	DeleteCriticalSection(&csSignal);

	return TRUE;
}

void SetupLastTune()
{
#ifdef DVBS
	char szPolarity[4] = {"H/L"};
	char szModulation[16] = {0};

	if (ss->nPolarity == 0)
		lstrcpy(szPolarity, "V/R");
	lstrcpy(szModulation, "DVB QPSK");
	wsprintf(szLastTune, "%d MHz %s %d %s", ss->nFrequency, szPolarity, ss->nSymbolRate, szModulation);
#endif DVBS
#ifdef DVBT
	sprintf(szLastTune, "%.3f MHz", ss->nFrequency / 1000.0);
#endif DVBT
#ifdef DVBC
	sprintf(szLastTune, "%.1f MHz", ss->nFrequency / 1000.0);
#endif DVBC
#ifdef EIGHTVSB
	sprintf(szLastTune, "%.1f MHz", ss->nFrequency / 1000.0);
#endif EIGHTVSB
#ifdef QAM
	sprintf(szLastTune, "%.1f MHz", ss->nFrequency / 1000.0);
#endif QAM
}

BOOL TSReader_Tune()
{
	CAtRegisters RegAcc(pBoard);

#ifdef ASISPI

	// Select DVB input	
	RegAcc.m_Registers.m_RecordConfig &= ~(AT_RCONF_DVB | AT_RCONF_SMP); 
	RegAcc.m_Registers.m_RecordConfig |= AT_RCONF_DVB | AT_RCONF_RENA; 	
	RegAcc.m_Registers.m_DvbSmpte = 0; //SET	
	switch(nInputSelect)
	{
	case 0:
		RegAcc.m_Registers.m_RecordConfig |= AT_RCONF_ISEL1 | AT_RCONF_ISEL0;
		break;
	case 1:
		RegAcc.m_Registers.m_RecordConfig |= AT_RCONF_ISEL1;
		break;
	}
    pBoard->UpdateRegisters();
	return TRUE;
#else ASISPI
	SetupLastTune();
    pBoard->GetRegisters();
	RegAcc.m_Registers.m_RecordConfig &= ~(AT_RCONF_DVB | AT_RCONF_SMP | AT_RCONF_RENA); 
	RegAcc.m_Registers.m_RecordConfig |= AT_RCONF_DVB | AT_RCONF_RENA | AT_RCONF_ISEL1 | AT_RCONF_ISEL0; 	
	RegAcc.m_Registers.m_DvbSmpte = 0; //SET	
	pBoard->UpdateRegisters();
#endif ASISPI

#ifdef DVBS
	int nFrequency;
	int nLnbExtVolt = 0;
    dvb_frontend_parameters parms;
    src_sec_tone_mode_t eBand;

	src_sec_voltage_t LnbVoltage = ss->nPolarity ? SEC_VOLTAGE_13 : SEC_VOLTAGE_18;
    pBoard->DvbSource(SRC_SET_VOLTAGE, &LnbVoltage);
    pBoard->DvbSource(SRC_ENABLE_HIGH_LNB_VOLTAGE, (void*)nLnbExtVolt);
	
	switch(ss->nDiSEqCInput)
	{
	case 1:
	case 2:
	case 3:
	case 4:
		{
			int nInput = ss->nDiSEqCInput;
			dvb_diseqc_master_cmd diseqc;
			BYTE bPositionByte[] = {0xc0, 0xc4, 0xc8, 0xcc};

			nInput--;
			if ((nInput >= 0) && (nInput <= 3) )
			{
				diseqc.msg[0] = 0xe0;	// master to slave no response
				diseqc.msg[1] = 0x10;	// address
				diseqc.msg[2] = 0x38;	// switch port
				diseqc.msg[3] = bPositionByte[nInput];
				diseqc.msg_len = 4;
			    pBoard->DvbSource(SRC_DISEQC_SEND_MASTER_CMD, (void*)&diseqc);
			}
		}
		Sleep(100);
		break;
	case 5:	// Tone A
	case 6:	// Tone B
		{
			src_sec_mini_cmd_t tone;

			if (ss->nDiSEqCInput == 5)
				tone = SEC_MINI_A;
			else
				tone = SEC_MINI_B;
		    pBoard->DvbSource(SRC_DISEQC_SEND_BURST, (void*)tone);
			Sleep(100);
		}
		break;
	}
	
	if (ss->n22KHz)
		eBand = SEC_TONE_ON;
	else
        eBand = SEC_TONE_OFF;
    pBoard->DvbSource(SRC_SET_TONE, (void*)SEC_TONE_OFF);
    pBoard->DvbSource(SRC_SET_TONE, (void*)eBand);
    
	if (ss->nFrequency > ss->nLNBFrequency)
		nFrequency = ss->nFrequency - ss->nLNBFrequency;
	else
		nFrequency = ss->nLNBFrequency - ss->nFrequency;
    parms.frequency = nFrequency * 1000;
    parms.inversion = INVERSION_AUTO;
    parms.u.qpsk.fec_inner = FEC_AUTO;
    parms.u.qpsk.symbol_rate = ss->nSymbolRate * 1000;
    pBoard->DvbSource(SRC_SET_FRONTEND, &parms);

    src_status_t status;
    pBoard->DvbSource(SRC_READ_STATUS, &status);
	if (status & SRC_HAS_LOCK)
		return TRUE;

	if (ss->fQuietMode == FALSE)
		MessageBox(ss->hWndTSReader, "Failed to lock signal", gszSourceName, MB_OK | MB_ICONWARNING);
	return FALSE;
#endif DVBS

#ifdef DVBT
	dvb_frontend_parameters parms;

    parms.frequency = ss->nFrequency * 1000;
    parms.inversion = INVERSION_AUTO;
	switch(ss->nBandwidth)
	{
	case 1:
		parms.u.ofdm.bandwidth = BANDWIDTH_7_6_MHZ;
		break;
	case 2:
		parms.u.ofdm.bandwidth = BANDWIDTH_8_7_MHZ;
		break;
	}
	pBoard->DvbSource(SRC_SET_FRONTEND, &parms);

	src_status_t status;
    pBoard->DvbSource(SRC_READ_STATUS, &status);
	if (status & (SRC_HAS_VITERBI | SRC_HAS_SIGNAL | SRC_HAS_CARRIER | SRC_HAS_SYNC) == (SRC_HAS_VITERBI | SRC_HAS_SIGNAL | SRC_HAS_CARRIER | SRC_HAS_SYNC))
		return TRUE;

	if (ss->fQuietMode == FALSE)
		MessageBox(ss->hWndTSReader, "Failed to lock signal", gszSourceName, MB_OK | MB_ICONWARNING);
	return FALSE;
#endif DVBT

#ifdef DVBC
	dvb_frontend_parameters parms;

    parms.frequency = ss->nFrequency * 1000;;
    parms.inversion = INVERSION_AUTO;
	switch(nQAM)
	{
	case 0:
		parms.u.qam.modulation = QAM_16;
		break;
	case 1:
		parms.u.qam.modulation = QAM_32;
		break;
	case 2:
		parms.u.qam.modulation = QAM_64;
		break;
	case 3:
		parms.u.qam.modulation = QAM_128;
		break;
	case 4:
		parms.u.qam.modulation = QAM_256;
		break;
	default:
		parms.u.qam.modulation = QAM_AUTO;
		break;
	}
    parms.u.qam.symbol_rate= ss->nSymbolRate * 1000;  
    pBoard->DvbSource(SRC_SET_FRONTEND, &parms);

	src_status_t status;
	pBoard->DvbSource(SRC_READ_STATUS, &status);
	if (status & (SRC_HAS_CARRIER | SRC_HAS_SYNC) == (SRC_HAS_CARRIER | SRC_HAS_SYNC))
		return TRUE;
	
	if (ss->fQuietMode == FALSE)
		MessageBox(ss->hWndTSReader, "Failed to lock signal", gszSourceName, MB_OK | MB_ICONWARNING);
	return FALSE;
#endif DVBC

#ifdef EIGHTVSB
	dvb_frontend_parameters parms;

    parms.frequency = ss->nFrequency * 1000;
	pBoard->DvbSource(SRC_SET_FRONTEND, &parms);

	src_status_t status;
    pBoard->DvbSource(SRC_READ_STATUS, &status);
	if (status & (SRC_HAS_VITERBI | SRC_HAS_SIGNAL | SRC_HAS_CARRIER | SRC_HAS_SYNC) == (SRC_HAS_VITERBI | SRC_HAS_SIGNAL | SRC_HAS_CARRIER | SRC_HAS_SYNC))
		return TRUE;

	if (ss->fQuietMode == FALSE)
		MessageBox(ss->hWndTSReader, "Failed to lock signal", gszSourceName, MB_OK | MB_ICONWARNING);
	return FALSE;
#endif EIGHTVSB

#ifdef QAM
	dvb_frontend_parameters parms;

    parms.frequency = ss->nFrequency * 1000;
    parms.u.qam.modulation = QAM_256;     // Or QAM_64
	pBoard->DvbSource(SRC_SET_FRONTEND, &parms);

	src_status_t status;
    pBoard->DvbSource(SRC_READ_STATUS, &status);
	if (status & (SRC_HAS_FAT_LOCK | SRC_HAS_QAMSYNC_LOCK | SRC_HAS_MPEG_LOCK) == (SRC_HAS_FAT_LOCK | SRC_HAS_QAMSYNC_LOCK | SRC_HAS_MPEG_LOCK))
		return TRUE;

	if (ss->fQuietMode == FALSE)
		MessageBox(ss->hWndTSReader, "Failed to lock signal", gszSourceName, MB_OK | MB_ICONWARNING);
	return FALSE;
#endif QAM

}

BOOL TSReader_TuneDialog(HWND hWnd)
{
	if (fNeedTuneDialog)
	{
#ifdef ASISPI
		if (fNoInputSelect == TRUE)
		{
			nInputSelect = 0;	// always ASI
			return TRUE;
		}
		LoadAlitronikaSettings();
		if (fDontAskMode == FALSE)
		{
			if (DialogBox((HINSTANCE)hInstance, MAKEINTRESOURCE(IDD_INPUT_SELECT), ss->hWndTSReader, InputModeDlgProc) == FALSE)
				return FALSE;
		}
		SaveAlitronikaSettings();
#endif ASISPI
#ifdef DVBS
		if (SourceHelper_DVBSTuneDialog(hWnd) == FALSE)
			return FALSE;
#endif DVBS
#ifdef DVBT
		if (SourceHelper_DVBTTuneDialog(hWnd) == FALSE)
			return FALSE;
#endif DVBT
#ifdef DVBC
		if (SourceHelper_DVBCTuneDialog(hWnd) == FALSE)
			return FALSE;
#endif DVBC
#ifdef EIGHTVSB
		if (SourceHelper_ATSCTuneDialog(hWnd) == FALSE)
			return FALSE;
#endif EIGHTVSB
#ifdef QAM
		if (SourceHelper_QAMTuneDialog(hWnd) == FALSE)
			return FALSE;
#endif QAM
	}
	else
	{
#ifdef DVBS
		ss->nPolarity = nPolarity;
		ss->nSymbolRate = nSymbolRate;
		ss->nLNBFrequency = nLNBFrequency;
		ss->n22KHz = n22KHz;
		ss->nDiSEqCInput = nDiSEqCInput;
#endif DVBS
#ifdef DVBT
		ss->nBandwidth = nBandwidth;
		ss->fSpectrumInversion = fSpectrumInversion;
#endif DVBT
#ifdef DVBC
		ss->nQAM = nQAM;
		ss->nSymbolRate = nSymbolRate;
#endif DVBC
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

#ifdef ASISPI
	if (szCommandLineParameters != NULL)
		lstrcpy(szCommandLineParameters, "input");
	if (dwCapabilities != NULL)
		*dwCapabilities = 0;
#endif ASISPI
#ifdef DVBS
	if (szCommandLineParameters != NULL)
		lstrcpy(szCommandLineParameters, "freq pol sr lnbf 22khz {input}");
	if (dwCapabilities != NULL)
		*dwCapabilities = CAPABILITIES_POWER
		                | CAPABILITIES_DISEQC
						| CAPABILITIES_TONEBURST
						| CAPABILITIES_DISEQC_POSITIONER;
#endif DVBS
#ifdef DVBT
	if (szCommandLineParameters != NULL)
		lstrcpy(szCommandLineParameters, "freq bandwidth");
	if (dwCapabilities != NULL)
		*dwCapabilities = 0;
#endif DVBT
#ifdef DVBC
	if (szCommandLineParameters != NULL)
		lstrcpy(szCommandLineParameters, "freq sr QAM inversion bandwidth");
	if (dwCapabilities != NULL)
		*dwCapabilities = 0;
#endif DVBC

	return TRUE;
}

BOOL TSReader_ParseCommandLine(PSOURCESTRUCT ss, char * szCommandLine, BOOL fQuiet)
{
	fNeedTuneDialog = TRUE;
	if (lstrlen(szCommandLine))
	{
		int nConversionCount;

#ifdef ASISPI
		nInputSelect = 0;
		nConversionCount = sscanf(szCommandLine,
								  "%d", 
								  &nInputSelect);
		if (nConversionCount < 1 || (nInputSelect < 0 || nInputSelect > 1) )
		{
			if (!fQuiet)
				MessageBox(NULL,
					   "Usage for this source: input\n"
					   "\n"
					   "input = 0 for DVB-ASI or 1 for DVB-SPI",
					   gszSourceName,
					   MB_OK | MB_ICONSTOP);
			return FALSE;
		}
		fNeedTuneDialog = FALSE;
#endif ASISPI
#ifdef DVBS
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
#endif DVBS
#ifdef DVBT
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
#endif DVBT
#ifdef DVBC
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
#endif DVBC
	}
	return TRUE;
}

BOOL TSReader_IsPIDActive(int nPID)
{
	return TRUE;
}

BOOL TSReader_GetSignalString(char * szString)
{
	EnterCriticalSection(&csSignal);
	lstrcpy(szString, szLastSignal);
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
#ifdef DVBS
	if (bCommand != NULL && nLength)
	{
		dvb_diseqc_master_cmd diseqc;

		memcpy(diseqc.msg, bCommand, nLength);
		diseqc.msg_len = nLength;
		pBoard->DvbSource(SRC_DISEQC_SEND_MASTER_CMD, (void*)&diseqc);
	}		
#endif DVBS
	return TRUE;
}

BOOL TSReader_SetChannel(int nChannel)
{
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
