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
char gszSourceName[] = {"Alitronika ASI/SPI"};
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
			Sleep(5);
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
				{
					lstrcpy(szLockStatus, "Unlocked");
					dBER = 0.0;
				}
				
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

// The szValidDevices array contains a list of Alitronika devices applicable
// to the particular source module. So for example when we're a DVB-C source, 
// we only look at Alitronika DVB-C devices. ATDEMO means the end of the list
// since that's also returned if there are no supported cards/interfaces.

#ifdef ASISPI
static char * szValidDevices[] = {
	"AT4USB",
	"AT20USB",			// Mini ASI
	"AT40USB",			// Mini ASI I/O
	"AT40R1USB",
	"AT400USB",
	"AT20PCI",			// USB/PCI ASI
	"AT40PCI",			// USB/PCI ASI I/O
	"AT400PCI",
	"AT40XUSB",
	"AT20XPCI",
	"AT40XPCI",
	"AT2700PCI",
	"AT2700USB",
	"AT2800PCI",
	"AT2800USB",
	"ATDEMO"
};
#endif ASISPI
#ifdef DVBS
static char * szValidDevices[] = {
	"AT60USB",			// Mini DVB-S
	"AT600USB",
	"AT600PCI",			// USB/PCI DVB-S
	"ATDEMO"
};
#endif DVBS
#ifdef DVBC
static char * szValidDevices[] = {
	"AT70USB",			// Mini DVB-C
	"AT700USB",
	"AT700PCI",			// USB/PCI DVB-C
	"ATDEMO"
};
#endif DVBC
#ifdef DVBT
static char * szValidDevices[] = {
	"AT80USB",			// Mini DVB-T
	"AT800USB",
	"AT800PCI",			// USB/PCI DVB-T
	"AT82",
	"AT820",
	"ATDEMO"
};
#endif DVBT
#ifdef EIGHTVSB
static char * szValidDevices[] = {
	"AT72USB",			// Mini 8VSB/QAM
	"AT720PCI",		// USB/PCI 8VSB/QAM
	"ATDEMO"
};
#endif EIGHTVSB
#ifdef QAM
static char * szValidDevices[] = {
	"AT72USB",			// Mini 8VSB/QAM
	"AT720PCI",			// USB/PCI 8VSB/QAM
	"ATDEMO"
};
#endif QAM

BOOL TSReader_Init(PSOURCESTRUCT pss)
{
	int nSourceIndex = pss->nSourceIndex;
    char * pFriendlyName = NULL;
    char * pDevicesBoardName = NULL;
    CAtDeviceList iDevices;
    
	pMan = CAtBoardManager::Instance();

	ss = pss;
	InitializeCriticalSection(&csSignal);
#ifdef ASISPI
	fNoInputSelect = FALSE;
#endif ASISPI
	
	nDeviceCount  = 0;
	pBoard = NULL;

    //Update device list.
    pMan->GetDeviceList(iDevices);

    //Get the first board name from the device list.
    pDevicesBoardName =  iDevices.GetFirstBoardName();
	if (pDevicesBoardName == NULL)
		OutputDebugString("TSReader Alitronika: GetFirstBoardName() returned NULL\n");

    // Loop through all devices in the list.
    // Stop the loop when the board name = NULL indicating no board found 
    while(pDevicesBoardName)
    {
        //Get the friendly name of the board
        pFriendlyName = iDevices.GetFriendlyName(pDevicesBoardName);
		{
			char szTemp[128];
			wsprintf(szTemp, "TSReader Alitronika: Device found %s\n", pFriendlyName);
			OutputDebugString(szTemp);
		}

        //Check if the boardname is 
        if(iDevices.IsBoardNameValid(pDevicesBoardName))
        {
			int nDeviceType;

			for (nDeviceType = 0; lstrcmp(szValidDevices[nDeviceType], "ATDEMO") != 0; nDeviceType++)
			{
				if (lstrcmp(szValidDevices[nDeviceType], pFriendlyName) == 0)
				{
					// We've found an interface with the right name.
					// Let's use it provided the board index says so.

					if (!nSourceIndex)
					{
#ifdef ASISPI
						if (lstrcmp(pFriendlyName, "AT4USB") == 0)
							fNoInputSelect = TRUE;
#endif ASISPI
						iDevices.SetBoardUsed(pDevicesBoardName, TRUE);
						pBoard = pMan->GetBoard(pDevicesBoardName);
						break;
					}
					nSourceIndex--;
				}
			}
        }
        else
        {
            // Board has errors, not detected correctly
            //
            // Do your own handling here
        }

		if (pBoard != NULL)
			break;

        //Get the next board name from the device list
        pDevicesBoardName = iDevices.GetNextBoardName();
    }

	if (pBoard == NULL)
	{
		MessageBox(NULL, "Unable to locate a suitable Alitronika interface", gszSourceName, MB_ICONSTOP);
		return FALSE;
	}
	
	
	
	/*   	std::for_each(devlist.Names.begin(), devlist.Names.end(), PrintDev);
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
*/
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
	wsprintf(szLastTune, "Channel %d (%d MHz)", SourceHelper_GetATSCChannelFromFrequency(ss->nFrequency), ss->nFrequency);
#endif EIGHTVSB
#ifdef QAM
	wsprintf(szLastTune, "Channel %d (%d MHz)", SourceHelper_GetQAMChannelFromFrequency(ss->nFrequency), ss->nFrequency);
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

	memset(&parms, 0, sizeof(parms));
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

	memset(&parms, 0, sizeof(parms));
    parms.frequency = ss->nFrequency * 1000;
    parms.inversion = INVERSION_AUTO;
	switch(ss->nBandwidth)
	{
	case 0:
		parms.u.ofdm.bandwidth = BANDWIDTH_6_MHZ;
		break;
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
	
	
////////////////
	{
		char szTemp[256];

		if (status & SRC_HAS_VITERBI)
			OutputDebugString("Alitronika DVB-T SRC_HAS_VITERBI\n");
		if (status & SRC_HAS_SIGNAL)
			OutputDebugString("Alitronika DVB-T SRC_HAS_SIGNAL\n");
		if (status & SRC_HAS_CARRIER)
			OutputDebugString("Alitronika DVB-T SRC_HAS_CARRIER\n");
		if (status & SRC_HAS_SYNC)
			OutputDebugString("Alitronika DVB-T SRC_HAS_SYNC\n");

		int nBitrateError;
		pBoard->DvbSource(SRC_READ_BER, &nBitrateError);
		wsprintf(szTemp, "Alitronika DVB-T BER: %.3lf\n", (nBitrateError*100.0)/0xffff);
		OutputDebugString(szTemp);

		int nSigStrength;
		pBoard->DvbSource(SRC_READ_SIGNAL_STRENGTH, &nSigStrength);
		wsprintf(szTemp, "Alitronika DVB-T Signal: %d\n", nSigStrength);
		OutputDebugString(szTemp);

		struct dvb_frontend_parameters Info;
		pBoard->DvbSource(SRC_GET_FRONTEND, &Info);
		wsprintf(szTemp, "Alitronika DVB-T %6.3f MHz\n", Info.frequency/1000000.0);
		OutputDebugString(szTemp);

		switch(Info.u.ofdm.constellation)
		{
		case QPSK:
			OutputDebugString("Alitronika DVB-T Modulation: QPSK\n");
			break;
		case  QAM_16:
			OutputDebugString("Alitronika DVB-T Modulation: QAM 16\n");
			break;
		case  QAM_32:
			OutputDebugString("Alitronika DVB-T Modulation: QAM 32\n");
			break;
		case  QAM_64:
			OutputDebugString("Alitronika DVB-T Modulation: QAM 64\n");
			break;
		case  QAM_128:
			OutputDebugString("Alitronika DVB-T Modulation: QAM 128\n");
			break;
		case  QAM_256:
			OutputDebugString("Alitronika DVB-T Modulation: QAM 256\n");
			break;
		}
		switch(Info.u.ofdm.guard_interval)
		{
		case  GUARD_INTERVAL_1_32:
			OutputDebugString("Alitronika DVB-T GI: 1/32\n");
			break;
		case  GUARD_INTERVAL_1_16:
			OutputDebugString("Alitronika DVB-T GI: 1/16\n");
			break;
		case  GUARD_INTERVAL_1_8:
			OutputDebugString("Alitronika DVB-T GI: 1/8\n");
			break;
		case  GUARD_INTERVAL_1_4:
			OutputDebugString("Alitronika DVB-T GI: 1/4\n");
			break;
		}
		switch(Info.u.ofdm.hierarchy_information)
		{
		case  HIERARCHY_NONE:
			OutputDebugString("Alitronika DVB-T Hierarchy: None\n");
			break;
		case  HIERARCHY_1:
			OutputDebugString("Alitronika DVB-T Hierarchy: 1\n");
			break;
		case  HIERARCHY_2:
			OutputDebugString("Alitronika DVB-T Hierarchy: 2\n");
			break;
		case  HIERARCHY_4:
			OutputDebugString("Alitronika DVB-T Hierarchy: 4\n");
			break;
		}

		src_code_rate_t *pCr[2] = {&Info.u.ofdm.code_rate_HP, &Info.u.ofdm.code_rate_LP};
		for (int i=0; i<2; ++i)
		{
			switch(*pCr[i])
			{
			case FEC_AUTO:
				wsprintf(szTemp, "Alitronika DVB-T FEC rate %d: Auto\n", i);
				break;
			case FEC_1_2:
				wsprintf(szTemp, "Alitronika DVB-T FEC rate %d: 1/2\n");
				break;
			case FEC_2_3:
				wsprintf(szTemp, "Alitronika DVB-T FEC rate %d: 2/3\n");
				break;
			case FEC_3_4:
				wsprintf(szTemp, "Alitronika DVB-T FEC rate %d: 3/4\n");
				break;
			case FEC_4_5:
				wsprintf(szTemp, "Alitronika DVB-T FEC rate %d: 4/5\n");
				break;
			case FEC_5_6:
				wsprintf(szTemp, "Alitronika DVB-T FEC rate %d: 5/6\n");
				break;
			case FEC_6_7:
				wsprintf(szTemp, "Alitronika DVB-T FEC rate %d: 6/7\n");
				break;
			case FEC_7_8:
				wsprintf(szTemp, "Alitronika DVB-T FEC rate %d: 7/8\n");
				break;
			default:
				wsprintf(szTemp, "Alitronika DVB-T FEC rate %d: Default\n");
				break;
			}
			OutputDebugString(szTemp);
		}
		switch(Info.u.ofdm.transmission_mode)
		{
		case TRANSMISSION_MODE_2K:
			OutputDebugString("Alitronika DVB-T Mode: 2k\n");
			break;
		case TRANSMISSION_MODE_8K:
			OutputDebugString("Alitronika DVB-T Mode: 8k\n");
			break;
		default://TRANSMISSION_MODE_AUTO
			OutputDebugString("Alitronika DVB-T Mode: 8k\n");
			break;
		}
	}
////////////////
	
	
	if (status & (SRC_HAS_VITERBI | SRC_HAS_SIGNAL | SRC_HAS_CARRIER | SRC_HAS_SYNC) == (SRC_HAS_VITERBI | SRC_HAS_SIGNAL | SRC_HAS_CARRIER | SRC_HAS_SYNC))
		return TRUE;

	if (ss->fQuietMode == FALSE)
		MessageBox(ss->hWndTSReader, "Failed to lock signal", gszSourceName, MB_OK | MB_ICONWARNING);
	return FALSE;
#endif DVBT

#ifdef DVBC
	dvb_frontend_parameters parms;

	memset(&parms, 0, sizeof(parms));
    parms.frequency = ss->nFrequency * 1000;
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
    parms.u.qam.symbol_rate = ss->nSymbolRate * 1000;  
    pBoard->DvbSource(SRC_SET_FRONTEND, &parms);
	Sleep(500);

	src_status_t status;
	pBoard->DvbSource(SRC_READ_STATUS, &status);

	if (status & (SRC_HAS_CARRIER | SRC_HAS_SYNC) == (SRC_HAS_CARRIER | SRC_HAS_SYNC))
		return TRUE;
	
	if (ss->fQuietMode == FALSE)
		MessageBox(ss->hWndTSReader, "Failed to lock signal", gszSourceName, MB_OK | MB_ICONWARNING);
	return FALSE;
#endif DVBC

#ifdef EIGHTVSB
	int nLockMask = SRC_HAS_VITERBI | SRC_HAS_SIGNAL | SRC_HAS_CARRIER | SRC_HAS_SYNC;
	int nLockStatus;
	dvb_frontend_parameters parms;

	memset(&parms, 0, sizeof(parms));
    parms.frequency = ss->nFrequency * 1000 * 1000;
    parms.u.qam.modulation = VSB_8;
	pBoard->DvbSource(SRC_SET_FRONTEND, &parms);

	src_status_t status;
    pBoard->DvbSource(SRC_READ_STATUS, &status);
	{
		char szTemp[128];
		wsprintf(szTemp, "Alitronika 8VSB: status = %08x\n", status);
		OutputDebugString(szTemp);
	}
	nLockStatus = (int)status;
	nLockStatus &= nLockMask;
//	if (nLockStatus == nLockMask)
		return TRUE;

	if (ss->fQuietMode == FALSE)
		MessageBox(ss->hWndTSReader, "Failed to lock signal", gszSourceName, MB_OK | MB_ICONWARNING);
	return FALSE;
#endif EIGHTVSB

#ifdef QAM
	dvb_frontend_parameters parms;

	memset(&parms, 0, sizeof(parms));
    parms.frequency = ss->nFrequency * 1000 * 1000;
    parms.u.qam.modulation = QAM_256;     // Or QAM_64
	pBoard->DvbSource(SRC_SET_FRONTEND, &parms);

	src_status_t status;
    pBoard->DvbSource(SRC_READ_STATUS, &status);
	{
		char szTemp[128];
		wsprintf(szTemp, "Alitronika QAM: status = %08x\n", status);
		OutputDebugString(szTemp);
	}
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
		*dwCapabilities = CAPABILITIES_MULTICARD;
#endif ASISPI
#ifdef DVBS
	if (szCommandLineParameters != NULL)
		lstrcpy(szCommandLineParameters, "freq pol sr lnbf 22khz {input}");
	if (dwCapabilities != NULL)
		*dwCapabilities = CAPABILITIES_POWER
		                | CAPABILITIES_DISEQC
						| CAPABILITIES_TONEBURST
						| CAPABILITIES_DISEQC_POSITIONER
						| CAPABILITIES_MULTICARD;
#endif DVBS
#ifdef DVBT
	if (szCommandLineParameters != NULL)
		lstrcpy(szCommandLineParameters, "freq bandwidth");
	if (dwCapabilities != NULL)
		*dwCapabilities = CAPABILITIES_MULTICARD;
#endif DVBT
#ifdef DVBC
	if (szCommandLineParameters != NULL)
		lstrcpy(szCommandLineParameters, "freq sr QAM inversion bandwidth");
	if (dwCapabilities != NULL)
		*dwCapabilities = CAPABILITIES_MULTICARD;
#endif DVBC
#ifdef EIGHTVSB
	if (szCommandLineParameters != NULL)
		lstrcpy(szCommandLineParameters, "freq");
	if (dwCapabilities != NULL)
		*dwCapabilities = CAPABILITIES_MULTICARD;
#endif EIGHTVSB
#ifdef QAM
	if (szCommandLineParameters != NULL)
		lstrcpy(szCommandLineParameters, "freq");
	if (dwCapabilities != NULL)
		*dwCapabilities = CAPABILITIES_MULTICARD;
#endif QAM

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
#ifdef EIGHTVSB
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
			nFrequency = SourceHelper_GetFrequencyFromATSCChannel(nFrequency);
#endif EIGHTVSB
#ifdef QAM
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
			nFrequency = SourceHelper_GetFrequencyFromQAMChannel(nFrequency);
#endif QAM
		fNeedTuneDialog = FALSE;
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
