#include <windows.h>
#include <commctrl.h>
#include <setupapi.h>
#include <initguid.h>
#include <stdio.h>
#include <math.h>

#include "CyAPI.h"

#include "..\sources.h"
#include "firmware.h"
#include "resource.h"

PSOURCESTRUCT ss;
BOOL fNeedTuneDialog = TRUE;
BOOL fSharpInitialized = FALSE;
int nTunerStatusTimer;
CRITICAL_SECTION csSignal;
HANDLE hInstance;

CCyUSBDevice * USBDevice = NULL;
CCyBulkEndPoint * MPEGInEpt = NULL;   
OVERLAPPED outOvLap;
OVERLAPPED inOvLap; 

char szLastSignalReport[128] = {"n/a"};        
char szLastTune[128] = {"n/a"};
BYTE bPowerCommand[] = {0x30, 0x00};

#ifndef NEXTORM
BOOL f9vUnit = FALSE;
BOOL fDontAskMode = FALSE;
char gszPSUKeyName[] = {"Software\\DTVWorks\\DVBWorldInterface"};
#ifndef MATCHBOXPRO
 #ifndef DSS
  char gszSourceName[] = {"DVBWorld DVB-S2101 DVB-S"};
 #else DSS
  char gszSourceName[] = {"DVBWorld DVB-S2101 DSS"};
 #endif DSS
#else MATCHBOXPRO
  char gszSourceName[] = {"Match Box Pro DVB-S"};
#endif MATCHBOXPRO
#else NEXTORM
#ifndef DSS
char gszSourceName[] = {"Nextorm DVB-S2101 DVB-S"};
#else DSS
char gszSourceName[] = {"Nextorm DVB-S2101 DSS"};
#endif DSS
#endif NEXTORM

int nFrequency;
int nPolarity;
int nSymbolRate;
int nLNBFrequency;
int n22KHz;
int nDiSEqCInput;
BOOL fPowerOn;
int nRetuneCount;

extern "C" {
	BOOL __cdecl SourceHelper_ValidateSourceContainer(PSOURCESTRUCT pss);
}

BOOL OpenUSBDriver(BOOL fQuiet)
{
	int d = 0;
	GUID OASISguid;
	char szFoundDevices[512] = {0};

	OASISguid.Data1 = 0xFA58C45D;
	OASISguid.Data2 = 0x5B19; OASISguid.Data3 = 0x428b;
	OASISguid.Data4[0] = 0xA2; OASISguid.Data4[1] = 0xD1; OASISguid.Data4[2] = 0x27; OASISguid.Data4[3] = 0x07;
	OASISguid.Data4[4] = 0x85; OASISguid.Data4[5] = 0x6D; OASISguid.Data4[6] = 0x7E; OASISguid.Data4[7] = 0x19;
	
	USBDevice = new CCyUSBDevice(NULL, OASISguid);
    int nDevices = USBDevice->DeviceCount(); 
	if (nDevices == 0)
	{
		if (!fQuiet)
			MessageBox(NULL, "Unable to locate any DVBWorld interfaces", gszSourceName, MB_ICONSTOP);
		return FALSE;
	}

    do
	{
		USBDevice->Open(d);   // Open automatically  calls Close() if necessary 

		int vID = USBDevice->VendorID; 
        int pID  = USBDevice->ProductID;

#ifndef MATCHBOXPRO
		if (vID == 0x04b4 && pID == 0x2101 || vID == 0x04b4 && pID == 0x2102)
#else MATCHBOXPRO
		if (vID == 0x734C && pID == 0x2601)
#endif MATCHBOXPRO
			break;
		USBDevice->Close();
		d++;
    } while (d < nDevices);

	if (d == nDevices)
	{
		if (!fQuiet)
			MessageBox(NULL, "Unable to locate the DVBWorld interface", gszSourceName, MB_ICONSTOP);
		return FALSE;
	}

	if (USBDevice->EndPoints == NULL)
	{
		OutputDebugString("DVBWorld: NULL endpoint from the USB device\n");
		return FALSE;
	}

	return TRUE;
}

void CursorNormal()
{
	ReleaseCapture();
	SetCursor(LoadCursor(NULL, IDC_ARROW));
}

void CursorWait(HWND hWnd)
{
	SetCapture(hWnd);
	SetCursor(LoadCursor(NULL, IDC_WAIT));
}

void WriteB2Command(BYTE * pBuffer, int nLength)
{
	LONG len = nLength;

	USBDevice->ControlEndPt->Target = TGT_DEVICE;
	USBDevice->ControlEndPt->ReqType   = REQ_VENDOR; 
	USBDevice->ControlEndPt->Direction = DIR_TO_DEVICE;  
	USBDevice->ControlEndPt->ReqCode   = 0xb2;   
	USBDevice->ControlEndPt->Value     = 0;  
	USBDevice->ControlEndPt->Index     = 0; 
	USBDevice->ControlEndPt->XferData(pBuffer, len);
}

#ifndef MATCHBOXPRO
BYTE ReadB6(BYTE bIndex)
{
	BYTE bBuf[2];
	LONG len = sizeof(bBuf);

	USBDevice->ControlEndPt->Target = TGT_DEVICE;
	USBDevice->ControlEndPt->ReqType   = REQ_VENDOR; 
	USBDevice->ControlEndPt->Direction = DIR_FROM_DEVICE;  
	USBDevice->ControlEndPt->ReqCode   = 0xb6;   
	USBDevice->ControlEndPt->Value     = 0xa0;  
	USBDevice->ControlEndPt->Index     = bIndex; 
	USBDevice->ControlEndPt->XferData(bBuf, len);
	return (len == sizeof(bBuf));
	return (bBuf[0]);
}

BYTE * ReadB7(BYTE bValue)
{
	static BYTE bBuf[4];
	LONG len = sizeof(bBuf);

	USBDevice->ControlEndPt->Target = TGT_DEVICE;
	USBDevice->ControlEndPt->ReqType   = REQ_VENDOR; 
	USBDevice->ControlEndPt->Direction = DIR_FROM_DEVICE;  
	USBDevice->ControlEndPt->ReqCode   = 0xb7;   
	USBDevice->ControlEndPt->Value     = bValue;  
	USBDevice->ControlEndPt->Index     = 0; 
	USBDevice->ControlEndPt->XferData(bBuf, len);
	return bBuf;
}

BYTE ReadB9()
{
	BYTE bBuf[2];
	LONG len = sizeof(bBuf);

	USBDevice->ControlEndPt->Target = TGT_DEVICE;
	USBDevice->ControlEndPt->ReqType   = REQ_VENDOR; 
	USBDevice->ControlEndPt->Direction = DIR_FROM_DEVICE;  
	USBDevice->ControlEndPt->ReqCode   = 0xb9;   
	USBDevice->ControlEndPt->Value     = 0;  
	USBDevice->ControlEndPt->Index     = 0; 
	USBDevice->ControlEndPt->XferData(bBuf, len);
	return (bBuf[0]);
}

BYTE ReadBA()
{
	BYTE bBuf[7];
	LONG len = sizeof(bBuf);

	USBDevice->ControlEndPt->Target = TGT_DEVICE;
	USBDevice->ControlEndPt->ReqType   = REQ_VENDOR; 
	USBDevice->ControlEndPt->Direction = DIR_FROM_DEVICE;  
	USBDevice->ControlEndPt->ReqCode   = 0xbb;   
	USBDevice->ControlEndPt->Value     = 0x00;  
	USBDevice->ControlEndPt->Index     = 0; 
	USBDevice->ControlEndPt->XferData(bBuf, len);
	return (bBuf[0]);
}

BYTE ReadBB()
{
	BYTE bBuf[2];
	LONG len = sizeof(bBuf);

	USBDevice->ControlEndPt->Target = TGT_DEVICE;
	USBDevice->ControlEndPt->ReqType   = REQ_VENDOR; 
	USBDevice->ControlEndPt->Direction = DIR_FROM_DEVICE;  
	USBDevice->ControlEndPt->ReqCode   = 0xbb;   
	USBDevice->ControlEndPt->Value     = 0xc3;  
	USBDevice->ControlEndPt->Index     = 0; 
	USBDevice->ControlEndPt->XferData(bBuf, len);
	return (bBuf[0]);
}

BYTE ReadBC()
{
	BYTE bBuf[2];
	LONG len = sizeof(bBuf);

	USBDevice->ControlEndPt->Target = TGT_DEVICE;
	USBDevice->ControlEndPt->ReqType   = REQ_VENDOR; 
	USBDevice->ControlEndPt->Direction = DIR_FROM_DEVICE;  
	USBDevice->ControlEndPt->ReqCode   = 0xbc;   
	USBDevice->ControlEndPt->Value     = 0x30;  
	USBDevice->ControlEndPt->Index     = 0; 
	USBDevice->ControlEndPt->XferData(bBuf, len);
	return (bBuf[0]);
}

void SendNextormInitCommands()
{
	ReadBC();
	ReadBA();
	Sleep(50);
	ReadBA();
}

void WriteSharpTuner(BYTE * bTunerBytes)
{
	BYTE bBuf[7];
	LONG len = sizeof(bBuf);
	bBuf[0] = 0x2c;
	bBuf[1] = 0x05;		// register 5
	bBuf[2] = 0xc0;		// turn on bypass mode
	bBuf[3] = bTunerBytes[0];
	bBuf[4] = bTunerBytes[1];
	bBuf[5] = bTunerBytes[2];
	bBuf[6] = bTunerBytes[3];

	USBDevice->ControlEndPt->Target = TGT_DEVICE;
	USBDevice->ControlEndPt->ReqType   = REQ_VENDOR; 
	USBDevice->ControlEndPt->Direction = DIR_TO_DEVICE;  
	USBDevice->ControlEndPt->ReqCode   = 0xb2;   
	USBDevice->ControlEndPt->Value     = 0;  
	USBDevice->ControlEndPt->Index     = 0; 
	USBDevice->ControlEndPt->XferData(bBuf, len);
#ifdef TRACE_I2C
	{
		char szTemp[128];
		wsprintf(szTemp, "DVBWorld: Tuner bytes %02x %02x %02x %02x\n", bTunerBytes[0], bTunerBytes[1], bTunerBytes[2], bTunerBytes[3]);
		OutputDebugString(szTemp);
	}
#endif TRACE_I2C
}

void WriteSharpRegister(BYTE bRegister, BYTE bValue)
{
	BYTE bBuf[3];
	LONG len = sizeof(bBuf);
	bBuf[0] = 0x2a;
	bBuf[1] = bRegister;
	bBuf[2] = bValue;

	USBDevice->ControlEndPt->Target = TGT_DEVICE;
	USBDevice->ControlEndPt->ReqType   = REQ_VENDOR; 
	USBDevice->ControlEndPt->Direction = DIR_TO_DEVICE;  
	USBDevice->ControlEndPt->ReqCode   = 0xb2;   
	USBDevice->ControlEndPt->Value     = 0;  
	USBDevice->ControlEndPt->Index     = 0; 
	USBDevice->ControlEndPt->XferData(bBuf, len);
#ifdef TRACE_I2C
	{
		char szTemp[128];
		wsprintf(szTemp, "DVBWorld: Write reg %02x value %02x\n", bRegister, bValue);
		OutputDebugString(szTemp);
	}
#endif TRACE_I2C
}

BYTE ReadSharpRegister(BYTE bRegister)
{
	BYTE bBuf[2];
	LONG len = sizeof(bBuf);

	USBDevice->ControlEndPt->Target = TGT_DEVICE;
	USBDevice->ControlEndPt->ReqType   = REQ_VENDOR; 
	USBDevice->ControlEndPt->Direction = DIR_FROM_DEVICE;  
	USBDevice->ControlEndPt->ReqCode   = 0xb5;   
	USBDevice->ControlEndPt->Value     = bRegister;  
	USBDevice->ControlEndPt->Index     = 0; 
	USBDevice->ControlEndPt->XferData(bBuf, len);
#ifdef TRACE_I2C
	{
		char szTemp[128];
		wsprintf(szTemp, "DVBWorld: Read reg  %02x value %02x\n", bRegister, bBuf[0]);
		OutputDebugString(szTemp);
	}
#endif TRACE_I2C
	return (bBuf[0]);
}

void InitTunerVCO()
{
	// Init the VCO on the tuner
	int nLBand = 950 * 2;
	int i;
nLBand = 0x4208;
	for (i = 0; i < 1; i++)
	{
		BYTE bTunerBytes[4];

		ReadB7(0x5d);
		bTunerBytes[0] = (BYTE)((nLBand >> 8) & 0xff);
		bTunerBytes[1] = (BYTE)(nLBand & 0xff);
		bTunerBytes[2] = 0xe1;
		bTunerBytes[3] = 0x00;
		WriteSharpTuner(bTunerBytes);
		WriteSharpRegister(0x22, 0x00);	// clear offset L
		WriteSharpRegister(0x23, 0x00);	// clear offset U
		Sleep(100);

		ReadB7(0xb7);
		bTunerBytes[2] = 0xe1 | 0x04;
		bTunerBytes[3] = 0x00;
		WriteSharpTuner(bTunerBytes);
		WriteSharpRegister(0x22, 0x00);	// clear offset L
		WriteSharpRegister(0x23, 0x00);	// clear offset U
		Sleep(100);
		
		ReadB7(0x75);
		bTunerBytes[2] = 0xf5;
		bTunerBytes[3] = 0x0c;
		WriteSharpTuner(bTunerBytes);
		WriteSharpRegister(0x22, 0x00);	// clear offset L
		WriteSharpRegister(0x23, 0x00);	// clear offset U
		Sleep(100);
		nLBand = 1450 * 2;
	}
	ReadBB();
}

void InitSharp()
{
	int i = 0;
#define ORIGINAL_INIT_CODE
#ifdef ORIGINAL_INIT_CODE
	static BYTE bInitValues[] = {
		0xa1,	// 0x00 - ID	
		0x15,	// 0x01 - RCR
		0x38,	// 0x02 - MCR	--00
		0x00,	// 0x03 - ACR
		0x7d,	// 0x04 - F22FR
		0x35,	// 0x05 - I2CRPT
		0x02,	// 0x06 - DACR1 (MSB)
		0x00,	// 0x07 - DACR2 (LSB)
		0x44,	// 0x08 - DISEqC
		0x00,	// 0x09 - DISEqC FIFO
		0x82,	// 0x0a - DiSEqC status
		0x00,	// 0x0b - reserved
		0x51,	// 0x0c - IOCFG
		0x81,	// 0x0d - AGC1C
		0x23,	// 0x0e - RTC
		0x1f,	// 0x0f - AGC1R
		0x34,	// 0x10 - AGC2O
		0x84,	// 0x11 - TLSR
		0xf7,	// 0x12 - CFD  --77
		0x9b,	// 0x13 - ACLC
		0x9e,	// 0x14 - BCLC
		0xe3,	// 0x15 - CLDT
		0x80,	// 0x16 - AGC1I
		0x18,	// 0x17 - TLIR
		0xff,	// 0x18 - AGC2I1 (MSB)
		0xff,	// 0x19 - AGC2I2 (LSB)
		0x82,	// 0x1a - RTF
		0x00,	// 0x1b - VSTATUS
		0x7f,	// 0x1c - CLDI
		0x00,	// 0x1d - ERRCNT_HIGH
		0x00,	// 0x1e - ERRCNT_LOW
		0x00,	// 0x1f - SFRH
		0x00,	// 0x20 - SFRM
		0x00,	// 0x21 - SFRL
		0x80,	// 0x22 - CFRM
		0x0b,	// 0x23 - CFRL
		0x2b,	// 0x24 - NIRH
		0x75,	// 0x25 - NIRL
		0x1a,	// 0x26 - VERROR
		0x00,	// 0x27 - not used
		0x00,	// 0x28 - FECM
		0x1e,	// 0x29 - VTH0
		0x14,	// 0x2a - VTH1
		0x0f,	// 0x2b - VTH2
		0x09,	// 0x2c - VTH3
		0x05,	// 0x2d - VTH4
		0x00,	// 0x2e - not used
		0x00,	// 0x2f - not used
		0x00,	// 0x30 - not used
		0x1f,	// 0x31 - PR
		0x19,	// 0x32 - SEARCH
		0xfd,	// 0x33 - RS
		0x03,	// 0x34 - ERRCNT	--13
		0x00,	// 0x35 - not used
		0x00,	// 0x36 - not used
		0x00,	// 0x37 - not used
		0x00,	// 0x38 - not used
		0x00,	// 0x39 - not used
		0x00,	// 0x3a - not used
		0x00,	// 0x3b - not used
		0x00,	// 0x3c - not used
		0x00,	// 0x3d - not used
		0x00,	// 0x3e - not used
		0x00,	// 0x3f - not used
		0x00,	// 0x40 - unknown
		0x00,	// 0x41 - unknown
		0x00,	// 0x42 - unknown
		0x00,	// 0x43 - unknown
		0x00,	// 0x44 - unknown
		0x00,	// 0x45 - unknown
		0x00,	// 0x46 - unknown
		0x00,	// 0x47 - unknown
		0x00,	// 0x48 - unknown
		0x00,	// 0x49 - unknown
		0x00,	// 0x4a - unknown
		0x00,	// 0x4b - unknown
		0x00,	// 0x4c - unknown
		0x00,	// 0x4d - unknown
		0x00,	// 0x4e - unknown
		0x00	// 0x4f - unknown
	};

	ReadB6(8);
	ReadB6(9);
	ReadB6(10);
	ReadB6(11);
	ReadB6(12);
	ReadB6(13);
	Sleep(100);

	ReadB6(3);
	ReadB6(4);
	ReadB6(5);
	ReadB6(6);
	ReadB9();

	//WriteSharpRegister(0x02, 0x00);	// MCR
	for (i = 0; i < sizeof(bInitValues); i++)
	{
		if (i == 0x09)
			continue;
		if (i == 0x27 || i == 0x2e || i == 0x2f || i == 0x30)
			continue;
		if (i >= 0x35 && i <= 0x3f)
			continue;
		WriteSharpRegister(i, bInitValues[i]);
	}
#else ORIGINAL_INIT_CODE
	WriteSharpRegister(0x02, 0x00);	// MCR
	WriteSharpRegister(0x01, 0x15);	// RCR
	WriteSharpRegister(0x02, 0x30);	// MCR
	WriteSharpRegister(0x03, 0x2a);	// ACR
	WriteSharpRegister(0x04, 0x7d);	// F22FR
	WriteSharpRegister(0x05, 0x05);	// I2CRPT
	WriteSharpRegister(0x06, 0xa2);	// DACR1 (MSB)
	WriteSharpRegister(0x07, 0x00);	// DACR2 (LSB)
	WriteSharpRegister(0x08, 0x60);	// DiSEqC

	WriteSharpRegister(0x29, 0x1e);	// VTH0
	WriteSharpRegister(0x2a, 0x14);	// VTH1
	WriteSharpRegister(0x2b, 0x0f);	// VTH2
	WriteSharpRegister(0x2c, 0x09);	// VTH3
	WriteSharpRegister(0x2d, 0x05);	// VTH4
	
	WriteSharpRegister(0x31, 0x1f);	// PR (code rate)
	WriteSharpRegister(0x32, 0x19);	// VSEARCH
	WriteSharpRegister(0x33, 0xf8);	// RS
	WriteSharpRegister(0x34, 0x03);	// ERRCNT - Post Viterbi error count
#endif ORIGINAL_INIT_CODE

	InitTunerVCO();
}

#ifndef NEXTORM
void LoadPowerSupplySettings()
{
	DWORD dwDisposition;
	DWORD dwDataSize;
	DWORD dwType;
	LONG lKey;
	HKEY hkMainReg;

	lKey = RegCreateKeyEx(HKEY_CURRENT_USER,
		                  gszPSUKeyName,
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
			dwDataSize = sizeof(f9vUnit);
			RegQueryValueEx(hkMainReg, "NineVoltMode", NULL, &dwType, (BYTE *)&f9vUnit, &dwDataSize);
			dwDataSize = sizeof(fDontAskMode);
			RegQueryValueEx(hkMainReg, "DontAskMode", NULL, &dwType, (BYTE *)&fDontAskMode, &dwDataSize);
		}
		RegCloseKey(hkMainReg);
	}
}

void SavePowerSupplySettings()
{
	LONG lKey;
	HKEY hkMainReg;
	DWORD dwDisposition;

	lKey = RegCreateKeyEx(HKEY_CURRENT_USER,
		                  gszPSUKeyName,
						  0,
						  gszSourceName,
					      REG_OPTION_NON_VOLATILE,
						  KEY_ALL_ACCESS,
						  NULL,
						  &hkMainReg,
						  &dwDisposition);
	if (lKey == ERROR_SUCCESS)
	{
		RegSetValueEx(hkMainReg, "NineVoltMode", 0, REG_DWORD, (BYTE *)&f9vUnit, sizeof(DWORD));
		RegSetValueEx(hkMainReg, "DontAskMode", 0, REG_DWORD, (BYTE *)&fDontAskMode, sizeof(DWORD));
		RegCloseKey(hkMainReg);
	}
}

BOOL CALLBACK PSUModeDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg)
	{
	case WM_INITDIALOG:
		if (f9vUnit == FALSE)
			CheckDlgButton(hDlg, IDC_5V_PSU, BST_CHECKED);
		else
			CheckDlgButton(hDlg, IDC_9V_PSU, BST_CHECKED);
		SetFocus(GetDlgItem(hDlg, IDOK));
		break;
	case WM_CLOSE:
		EndDialog(hDlg, FALSE);
		break;
	case WM_COMMAND:
		switch(LOWORD(wParam))
		{
		case IDOK:
			f9vUnit = IsDlgButtonChecked(hDlg, IDC_9V_PSU);
			fDontAskMode = IsDlgButtonChecked(hDlg, IDC_DONT_ASK);
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
#endif NEXTORM
#endif MATCHBOXPRO

void UploadMemory(WORD wAddress, BYTE * pMemory, int nLength)
{
	LONG len = nLength;

	USBDevice->ControlEndPt->Target = TGT_DEVICE;
	USBDevice->ControlEndPt->ReqType   = REQ_VENDOR; 
	USBDevice->ControlEndPt->Direction = DIR_TO_DEVICE;  
	USBDevice->ControlEndPt->ReqCode   = 0xa0;   
	USBDevice->ControlEndPt->Value     = wAddress;  
	USBDevice->ControlEndPt->Index     = 0; 
	USBDevice->ControlEndPt->XferData(pMemory, len);
}

BOOL UploadFirmware()
{
	int nFirmwareOffset = 0;
	BYTE bOne = 1;
	BYTE bZero = 0;
	int nTimeout = 40;	// 10 seconds in 250 ms steps
	int nFirmwareSize;

#ifndef NEXTORM
#ifndef MATCHBOXPRO
	LoadPowerSupplySettings();
	if (!fDontAskMode)
	{
		if (DialogBox((HINSTANCE)hInstance, MAKEINTRESOURCE(IDD_DVBWORLD_TYPE), ss->hWndTSReader, PSUModeDlgProc) == FALSE)
			return FALSE;
		SavePowerSupplySettings();
	}
#endif MATCHBOXPRO
#endif NEXTORM

	OutputDebugString("DVBWorld: Loading firmware\n");
	UploadMemory(0x7f92, &bOne, 1);
	UploadMemory(0xe600, &bOne, 1);
	Sleep(100);
#ifndef NEXTORM
#ifndef MATCHBOXPRO
	if (f9vUnit)
		nFirmwareSize = sizeof(firmware_9v);
	else
		nFirmwareSize = sizeof(firmware_5v);
#else MATCHBOXPRO
	nFirmwareSize = sizeof(firmware);
#endif MATCHBOXPRO
#else NEXTORM
	nFirmwareSize = sizeof(firmware);
#endif NEXTORM
	while (nFirmwareOffset < nFirmwareSize)
	{
#ifndef NEXTORM
#ifndef MATCHBOXPRO
		if (f9vUnit)
			UploadMemory(nFirmwareOffset, &firmware_9v[nFirmwareOffset], 0x40);
		else
			UploadMemory(nFirmwareOffset, &firmware_5v[nFirmwareOffset], 0x40);
#else MATCHBOXPRO
		UploadMemory(nFirmwareOffset, &firmware[nFirmwareOffset], 0x40);
#endif MATCHBOXPRO
#else NEXTORM
		UploadMemory(nFirmwareOffset, &firmware[nFirmwareOffset], 0x40);
#endif NEXTORM
		nFirmwareOffset += 0x40;
	}
	UploadMemory(0x7f92, &bZero, 1);
	UploadMemory(0xe600, &bZero, 1);
	USBDevice->ReConnect();

	//USBDevice->Reset();
	delete USBDevice;
	OutputDebugString("DVBWorld: Firmware sent\n");
	CursorWait(ss->hWndTSReader);
	while (OpenUSBDriver(TRUE) == FALSE && nTimeout-- > 0)
		Sleep(250);

#ifndef MATCHBOXPRO
	if (nTimeout)
	{
		ReadB6(0x08);
		ReadB6(0x09);
		ReadB6(0x0a);	
		ReadB6(0x0b);	
		ReadB6(0x0c);	
		ReadB6(0x0d);	
		ReadB6(0x05);
#ifndef NEXTORM
		if (f9vUnit)
			SendNextormInitCommands();
#else NEXTORM
		SendNextormInitCommands();
#endif NEXTORM
	}
#endif MATCHBOXPRO

	CursorNormal();

	return TRUE;
}

void SetupLastTune(int nFrequency, int nSymbolRate)
{
	szLastSignalReport[0] = '\0';
	char szPolarity[4] = {"H/L"};
	char szModulation[16] = {0};

	if (ss->nPolarity == 0)
		lstrcpy(szPolarity, "V/R");
	switch(ss->nADVModulationMode)
	{
	case ADV_MOD_DVB_QPSK:
#ifndef DSS
		lstrcpy(szModulation, "DVB QPSK");
#else DSS
		lstrcpy(szModulation, "DSS QPSK");
#endif DSS
		break;
	}
	wsprintf(szLastTune, "%d MHz %s %d %s", nFrequency, szPolarity, nSymbolRate, szModulation);
}

void SendDiSEqc(int nInput)
{
	BYTE bPositionByte[] = {0xc0, 0xc4, 0xc8, 0xcc};
	BYTE bInputString[] = {0xe0, 0x10, 0x38, 0x00};

#ifndef MATCHBOXPRO
	nInput--;
	if ((nInput >= 0) && (nInput <= 3) )
	{
		int i;
		int nTimeout;

		bInputString[3] = bPositionByte[nInput];
		WriteSharpRegister(0x08, 0x06);		// DiSEqC - DiSEqC mode
		Sleep(15);
		for (i = 0; i < 4; i++)
		{
			while ((ReadSharpRegister(0x0a) & 1) == 1)
				Sleep(1);	// stall if FIFO full
			WriteSharpRegister(0x09, bInputString[i]);
		}
		nTimeout = 100;
		while ((ReadSharpRegister(0x0a) & 2) == 2 && nTimeout-- > 0)
			Sleep(1);		// stall until FIFO empty
		Sleep(15);
		WriteSharpRegister(0x08, 0x04);		// DiSEqC - tone off
		//WriteSharpRegister(0x09, 0x00);		// DiSEqC FIFO
		Sleep(15);
	}
#endif MATCHBOXPRO
}

void SetSymbolRate(int nSymbolRate)
{
#ifndef MATCHBOXPRO
	{
		char szTemp[128];
		wsprintf(szTemp, "DVBWorld: SetSymbolRate(nSymbolRate = %d)\n", nSymbolRate);
		OutputDebugString(szTemp);
	}

	nSymbolRate = nSymbolRate << 16;
	nSymbolRate = nSymbolRate / 5500;
	nSymbolRate = nSymbolRate<< 4;
	BYTE bSRH = (nSymbolRate >> 16) & 0x0ff;
	BYTE bSRM = (nSymbolRate >> 8) & 0x0ff;
	BYTE bSRL = (nSymbolRate) & 0xff;
	WriteSharpRegister(0x1f, bSRH);		// SRH
	WriteSharpRegister(0x20, bSRM);		// SRM	
	WriteSharpRegister(0x21, bSRL);		// SRL
	WriteSharpRegister(0x22, 0x00);	// clear offset L
	WriteSharpRegister(0x23, 0x00);	// clear offset U
#endif MATCHBOXPRO
}

void SetFrequency(int nFrequency, int nSymbolRate)
{
#ifndef MATCHBOXPRO
	int nMuxWidth;
	BYTE bTunerBytes[4] = {0x00, 0x00, 0xe1, 0x00};

	{
		char szTemp[128];
		wsprintf(szTemp, "DVBWorld: SetFrequency(nFrequency = %d nSymbolRate = %d)\n", nFrequency, nSymbolRate);
		OutputDebugString(szTemp);
	}

	if (nFrequency >= 950 && nFrequency < 970)
		bTunerBytes[3] |= 0xa2;
	else if (nFrequency >= 970 && nFrequency < 1065)
		bTunerBytes[3] |= 0xc2;
	else if (nFrequency >= 1065 && nFrequency < 1170)
		bTunerBytes[3] |= 0xe2;
	else if (nFrequency >= 1170 && nFrequency < 1300)
		bTunerBytes[3] |= 0x20;
	else if (nFrequency >= 1300 && nFrequency < 1445)
		bTunerBytes[3] |= 0x40;
	else if (nFrequency >= 1445 && nFrequency < 1607)
		bTunerBytes[3] |= 0x60;
	else if (nFrequency >= 1607 && nFrequency < 1778)
		bTunerBytes[3] |= 0x80;
	else if (nFrequency >= 1778 && nFrequency < 1942)
		bTunerBytes[3] |= 0xa0;
	else if (nFrequency >= 1942 && nFrequency < 2131)
		bTunerBytes[3] |= 0xc0;
	else if (nFrequency >= 2131 && nFrequency <= 2150)
		bTunerBytes[3] |= 0xe0;

	if (nSymbolRate < 12000)
	{
		// 10MHz
		bTunerBytes[2] |= 0x18;		// PD5=1 PD4=1
									// PD3=0 PD2=0
		OutputDebugString("DVBWorld: LPF at 10 MHz\n");
	}
	else
	{
		nMuxWidth = (nSymbolRate / 1000) + 2;
		/*if (nMuxWidth <= 12)
		{
			// 12MHz
										// PD5=0 PD4=0 
			bTunerBytes[3] |= 0x08;		// PD3=1 PD2=0
			OutputDebugString("DVBWorld: LPF at 12 MHz\n");
		}
		else if (nMuxWidth <= 14)
		{
			// 14MHz
			bTunerBytes[2] |= 0x10;		// PD5=1 PD4=0 
			bTunerBytes[3] |= 0x08;		// PD3=1 PD2=0
			OutputDebugString("DVBWorld: LPF at 14 MHz\n");
		}
		else if (nMuxWidth <= 16)
		{
			// 16MHz
			bTunerBytes[2] |= 0x08;		// PD5=0 PD4=1 
			bTunerBytes[3] |= 0x08;		// PD3=1 PD2=0
			OutputDebugString("DVBWorld: LPF at 16 MHz\n");
		}
		else if (nMuxWidth <= 18)
		{
			// 18MHz
			bTunerBytes[2] |= 0x18;		// PD5=1 PD4=1 
			bTunerBytes[3] |= 0x08;		// PD3=1 PD2=0
			OutputDebugString("DVBWorld: LPF at 18 MHz\n");
		}
		else if (nMuxWidth <= 20)
		{
			// 20MHz
										// PD5=0 PD4=0 
			bTunerBytes[3] |= 0x04;		// PD3=0 PD2=1
			OutputDebugString("DVBWorld: LPF at 20 MHz\n");
		}
		else if (nMuxWidth <= 22)
		{
			// 22MHz
			bTunerBytes[2] |= 0x10;		// PD5=1 PD4=0 
			bTunerBytes[3] |= 0x04;		// PD3=0 PD2=1
			OutputDebugString("DVBWorld: LPF at 22 MHz\n");
		}
		else if (nMuxWidth <= 24)
		{
			// 24MHz
			bTunerBytes[2] |= 0x08;		// PD5=0 PD4=1 
			bTunerBytes[3] |= 0x04;		// PD3=0 PD2=1
			OutputDebugString("DVBWorld: LPF at 24 MHz\n");
		}
		else if (nMuxWidth <= 26)
		{
			// 26MHz
			bTunerBytes[2] |= 0x18;		// PD5=1 PD4=1 
			bTunerBytes[3] |= 0x04;		// PD3=0 PD2=1
			OutputDebugString("DVBWorld: LPF at 26 MHz\n");
		}
		else if (nMuxWidth <= 28)
		{
			// 28MHz
										// PD5=0 PD4=0 
			bTunerBytes[3] |= 0x0c;		// PD3=1 PD2=1
			OutputDebugString("DVBWorld: LPF at 28 MHz\n");
		}
		else*/
		{
			/// 30MHz
			bTunerBytes[2] |= 0x10;		// PD5=1 PD4=0 
			bTunerBytes[3] |= 0x0c;		// PD3=1 PD2=1
			OutputDebugString("DVBWorld: LPF at 30 MHz\n");
		}	
	}

	nFrequency = nFrequency * 2;
	bTunerBytes[0] = (BYTE)((nFrequency >> 8) & 0xff);
	bTunerBytes[1] = (BYTE)(nFrequency & 0xff);
	WriteSharpTuner(bTunerBytes);
	WriteSharpRegister(0x22, 0x00);	// clear offset L
	WriteSharpRegister(0x23, 0x00);	// clear offset U
#endif MATCHBOXPRO
}

BOOL TSReader_Tune()
{
#ifndef MATCHBOXPRO
	BOOL fInverted = FALSE;
	int nInversionCounter = 0;
	int nLBand;
	int nFrequencyOffset = 0;
	int nSymbolRateOffset = 0;
	int nSymbolRateCounter = 0;
	DWORD dwLockTimeout = 15500;
	DWORD dwLockTime;

	SetupLastTune(ss->nFrequency, ss->nSymbolRate);
	InitTunerVCO();

	if (ss->nFrequency > ss->nLNBFrequency)
		nLBand = ss->nFrequency - ss->nLNBFrequency;
	else
		nLBand = ss->nLNBFrequency - ss->nFrequency;

#ifndef DSS
	WriteSharpRegister(0x28, 0x02);	// FECM (DVB QPSK serial)
	WriteSharpRegister(0x14, 0x55);	// BCLC (QPSK2)
	WriteSharpRegister(0x33, 0xfd);	// RS
#else DSS
	WriteSharpRegister(0x28, 0x42);	// FECM (DSS QPSK serial)
	WriteSharpRegister(0x14, 0x95);	// BCLC (QPSK2)
	WriteSharpRegister(0x33, 0xf1);	// RS
#endif DSS

	WriteSharpRegister(0x02, 0x38);	// MCR
	WriteSharpRegister(0x34, 0x03);	// ERRCNT
	WriteSharpRegister(0x0f, 0x1f);	// AGC1R
	WriteSharpRegister(0x12, 0xf7);	// CFD
	WriteSharpRegister(0x0c, 0xf1);	// IOCFG (not inverted)

	MPEGInEpt->Reset();

	bPowerCommand[1] = ss->nPolarity;
	WriteB2Command(bPowerCommand, sizeof(bPowerCommand));

	SendDiSEqc(ss->nDiSEqCInput);
	if (ss->n22KHz)
		WriteSharpRegister(0x08, 0x03);		// DiSEqC - tone on
	else
		WriteSharpRegister(0x08, 0x04);		// DiSEqC - tone off

	SetSymbolRate(ss->nSymbolRate + nSymbolRateOffset);
	SetFrequency(nLBand + nFrequencyOffset, ss->nSymbolRate);
	ReadB7(0x00);
	dwLockTime = GetTickCount();	
	while (TRUE)
	{
		BYTE bVStatus;
		bVStatus = ReadSharpRegister(0x1b);
		if ((bVStatus & 0x98) == 0x98)
		{
			if (nFrequencyOffset || nSymbolRateOffset)
				SetupLastTune(ss->nFrequency + nFrequencyOffset, ss->nSymbolRate + nSymbolRateOffset);

			return TRUE;		// we're locked
		}

		DWORD dwCountNow = GetTickCount() - dwLockTime;
		if (dwCountNow > dwLockTimeout)
		{
			OutputDebugString("DVBWorld: Tune Timeout\n");
			break;
		}
		if (nInversionCounter++ == 30)
		{
			Sleep(1);
			nInversionCounter = 0;
			fInverted = ~fInverted;
			if (fInverted == FALSE)
				WriteSharpRegister(0x0c, 0xf1);	// IOCFG (not inverted)
			else
				WriteSharpRegister(0x0c, 0xf0);	// IOCFG (inverted)
#define ZIGZAG
#ifdef ZIGZAG
			nSymbolRateCounter++;
			if (nSymbolRateCounter > 5)
			{
				nSymbolRateCounter = 0;
				if (nSymbolRateOffset == 0)
				{
					nSymbolRateOffset = 1;
				}
				else
				{
					if (nSymbolRateOffset > 0)
						nSymbolRateOffset = -nSymbolRateOffset;
					else
					{
						nSymbolRateOffset = -nSymbolRateOffset;
						nSymbolRateOffset++;
						if (nSymbolRateOffset >  3)
						{
							nSymbolRateOffset = 0;
							if (nFrequencyOffset == 0)
								nFrequencyOffset = 1;
							else if (nFrequencyOffset == 1)
								nFrequencyOffset = -1;
							else if (nFrequencyOffset == -1)
								nFrequencyOffset = 2;
							else if (nFrequencyOffset == 2)
								nFrequencyOffset = -2;
							else if (nFrequencyOffset == -2)
								nFrequencyOffset = 0;
							SetFrequency(nLBand + nFrequencyOffset, ss->nSymbolRate + nSymbolRateOffset);
						}
					}
				}
				SetSymbolRate(ss->nSymbolRate + nSymbolRateOffset);
			}
#endif ZIGZAG
		}	
	}
#else MATCHBOXPRO
	int nLBand;
	BYTE b2_2a[] = {0x2a, 0x00, 0x02, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00};
	BYTE b2_28[] = {0x28, 0x00, 0x04, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00};
	BYTE b2_24[] = {0x24, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00};
	BYTE b2_20[] = {0x20, 0x00, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
	BYTE b2_22[] = {0x22, 0x00, 0x01, 0xdd, 0x00, 0x00, 0x00, 0x00, 0x00};

	SetupLastTune(ss->nFrequency, ss->nSymbolRate);

	if (ss->nFrequency > ss->nLNBFrequency)
		nLBand = ss->nFrequency - ss->nLNBFrequency;
	else
		nLBand = ss->nLNBFrequency - ss->nFrequency;
	WriteB2Command(b2_2a, 9);
	WriteB2Command(b2_28, 9);
	WriteB2Command(b2_24, 9);
	WriteB2Command(b2_28, 9);

	b2_20[3] = (nLBand >> 8) & 0xff;
	b2_20[4] = nLBand & 0xff;
	b2_20[6] = (ss->nSymbolRate >> 8) & 0xff;
	b2_20[7] = ss->nSymbolRate & 0xff;
	if (ss->nPolarity == 0)
		b2_20[8] = 1;
	else
		b2_20[8] = 0;
	WriteB2Command(b2_20, 9);
	WriteB2Command(b2_22, 9);
	b2_2a[3] = 0x00;
	WriteB2Command(b2_2a, 9);

	return TRUE;

#endif MATCHBOXPRO

	if (ss->fQuietMode == FALSE)
		MessageBox(ss->hWndTSReader, "Failed to lock signal", gszSourceName, MB_ICONWARNING);
	return FALSE;
}

void CheckLockStatus()
{
	if (nTunerStatusTimer++ > 50)
	{
#ifndef MATCHBOXPRO
		int nErrCnt;
		BOOL fLocked = TRUE;
		double dBER;
		double dDivisor = 2097152.0;
		char szLockStatus[16];

		nErrCnt = ReadSharpRegister(0x1d) << 8 | ReadSharpRegister(0x1e);
		if (nErrCnt > 8192)
			nErrCnt = 8192;
		dBER = (double)nErrCnt / dDivisor;

		if ((ReadSharpRegister(0x1b) & 0x98) == 0x98)
			lstrcpy(szLockStatus, "Locked");
		else
		{
			fLocked = FALSE;
			lstrcpy(szLockStatus, "Unlocked");
		}

		EnterCriticalSection(&csSignal);			
		sprintf(szLastSignalReport, "%s: BER %0.1E", szLockStatus, dBER);
		LeaveCriticalSection(&csSignal);

		if (!fLocked)
		{
			BOOL fQuietMode = ss->fQuietMode;
			ss->fQuietMode = TRUE;
			nRetuneCount++;
			TSReader_Tune();
			ss->fQuietMode = fQuietMode;
		}
#endif MATCHBOXPRO
		nTunerStatusTimer = 0;
	}
}

//#define FILE_OUTPUT
#define READ_FROM_FX2_SIZE 128 * 1024
DWORD WINAPI ReadFX2Thread(LPVOID lpv)
{
	int nReadPtr = 0;
	int nCurrentBuffer;
	int nEarlyPacketCounter = 0;
	BOOL fRestart = TRUE;
#ifdef FILE_OUTPUT
	HANDLE hDebug = CreateFile("c:\\tsreader.ts", GENERIC_WRITE, 0, (LPSECURITY_ATTRIBUTES)NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, (HANDLE)NULL);
#endif FILE_OUTPUT
	OutputDebugString("DVBWorld: +ReadFX2Thread\n");

	nTunerStatusTimer = 65535;
	ss->fReadThreadTerminated = FALSE;
	ss->fTerminateReadThread = FALSE;

#ifdef DSS
	SourceHelper_StartSyncThread(ss, TRUE);
#else DSS
	SourceHelper_StartSyncThread(ss, FALSE);
#endif DSS

RestartFX2DataThread:
	fRestart = FALSE;
	LONG nTransferLength = READ_FROM_FX2_SIZE;

	if (ss->nSymbolRate < 10000)
	{
		nTransferLength /= 4;
		if (ss->nSymbolRate < 5000)
			nTransferLength /= 2;
	}
	MPEGInEpt->SetXferSize(nTransferLength);

	// Setup the asynchronous transfer buffers
	int nQueueSize = 32;
	PUCHAR *buffers = new PUCHAR[nQueueSize];
	PUCHAR *contexts = new PUCHAR[nQueueSize];
	OVERLAPPED * inMPEGOvLap = new OVERLAPPED[nQueueSize];

	for (nCurrentBuffer = 0; nCurrentBuffer < nQueueSize; nCurrentBuffer++)
	{ 
       buffers[nCurrentBuffer] = new UCHAR[nTransferLength];
	   inMPEGOvLap[nCurrentBuffer].Internal = inMPEGOvLap[nCurrentBuffer].InternalHigh = 0;
	   inMPEGOvLap[nCurrentBuffer].Offset = inMPEGOvLap[nCurrentBuffer].OffsetHigh = 0;
	   inMPEGOvLap[nCurrentBuffer].hEvent = CreateEvent(NULL, false, false, NULL);
	}

	// Queue-up the first batch of transfer requests
	for (nCurrentBuffer = 0; nCurrentBuffer < nQueueSize; nCurrentBuffer++)
	   contexts[nCurrentBuffer] = MPEGInEpt->BeginDataXfer(buffers[nCurrentBuffer], nTransferLength, &inMPEGOvLap[nCurrentBuffer]);

	nCurrentBuffer = 0;
	//OutputDebugString("DVBWorld: enter main read loop\n");
	while (!ss->fTerminateReadThread)
	{
		LONG nReceiveLength = 0;
		
		if (!MPEGInEpt->WaitForXfer(&inMPEGOvLap[nCurrentBuffer], 1000))
		{
			char szTemp[128];
			wsprintf(szTemp, "DVBWorld: WaitForXfer() timed out buffer = %d ***********\n", nCurrentBuffer);
			OutputDebugString(szTemp);
			fRestart = TRUE;
			break;
		}

		if (MPEGInEpt->FinishDataXfer(buffers[nCurrentBuffer], nReceiveLength, &inMPEGOvLap[nCurrentBuffer],
			contexts[nCurrentBuffer]))
		{
			if (nEarlyPacketCounter < 5)
			{
				nEarlyPacketCounter++;
			}
			else if (nEarlyPacketCounter == 5)
			{
#ifndef MATCHBOXPRO
				// By default the Sharp tuner is in serial mode.
				// If we only get 0x00 and 0x80, then we need to switch to parallel

				int i;

				for (i = 0; i < nReceiveLength; i++)
				{
					if (buffers[nCurrentBuffer][i] != 0x00 && buffers[nCurrentBuffer][i] != 0x80)
						break;	// must be serial
				}
				if (i == nReceiveLength)
				{
#ifndef DSS
					WriteSharpRegister(0x28, 0x00);	// FECM (DVB QPSK parallel)
#else DSS
					WriteSharpRegister(0x28, 0x40);	// FECM (DSS QPSK parallel)
#endif DSS
					OutputDebugString("DVBWorld: Switched to parallel\n");
				}
#endif MATCHBOXPRO
				nEarlyPacketCounter++;
			}
			else
			{
				SourceHelper_SyncData(buffers[nCurrentBuffer], nReceiveLength);
#ifdef FILE_OUTPUT
				{
					DWORD dwWritten;
					WriteFile(hDebug, buffers[nCurrentBuffer], nReceiveLength, &dwWritten, NULL);
				}
#endif FILE_OUTPUT
			}
		}
		else
			Sleep(1);

	    contexts[nCurrentBuffer] = MPEGInEpt->BeginDataXfer(buffers[nCurrentBuffer], nTransferLength, &inMPEGOvLap[nCurrentBuffer]);
		nCurrentBuffer++;
		if (nCurrentBuffer == nQueueSize)
			nCurrentBuffer = 0;
		CheckLockStatus();
	}
	OutputDebugString("DVBWorld: left main read loop\n");
	nTunerStatusTimer = 65535;
	CheckLockStatus();

	MPEGInEpt->Abort();
	for (nCurrentBuffer = 0; nCurrentBuffer < nQueueSize; nCurrentBuffer++)
	{
		// Wait for all the queued requests to be cancelled 
		LONG nReceiveLength = 0;

		MPEGInEpt->FinishDataXfer(buffers[nCurrentBuffer], nReceiveLength, &inMPEGOvLap[nCurrentBuffer], contexts[nCurrentBuffer]);
		CloseHandle(inMPEGOvLap[nCurrentBuffer].hEvent);
		delete [] buffers[nCurrentBuffer];
	}

	delete [] buffers;
	delete [] contexts;
	delete [] inMPEGOvLap;

	if (fRestart == TRUE && !ss->fTerminateReadThread)
		goto RestartFX2DataThread;

	SourceHelper_StopSyncThread();
	ss->fReadThreadTerminated = TRUE;
	ss->fTerminateReadThread = FALSE;
	EnterCriticalSection(&ss->csTSBuffersInUse);
	ss->nTSBuffersInUse = -1000;
	LeaveCriticalSection(&ss->csTSBuffersInUse);
	CloseHandle(ss->hReadDataThread);
#ifdef FILE_OUTPUT
	CloseHandle(hDebug);
#endif FILE_OUTPUT
	OutputDebugString("DVBWorld: -ReadFX2Thread\n");
	return 0;
}

BOOL TSReader_Start()
{
	DWORD dwThreadID;

	OutputDebugString("DVBWorld: enter Start()\n");

	nRetuneCount = 0;
	ss->hReadDataThread = CreateThread(NULL, 0, ReadFX2Thread, (LPVOID)0, CREATE_SUSPENDED, &dwThreadID);
	SourceHelper_SetWorkerThreadPriorities(FALSE);
	ResumeThread(ss->hReadDataThread);

	OutputDebugString("DVBWorld: leave Start()\n");
	return TRUE;
}

BOOL TSReader_Stop()
{
	OutputDebugString("DVBWorld: enter Stop()\n");

	ss->fTerminateReadThread = TRUE;
	while (ss->fReadThreadTerminated == FALSE)
		Sleep(50);
	
	OutputDebugString("DVBWorld: leave Stop()\n");
	return TRUE;
}

BOOL TSReader_Init(PSOURCESTRUCT pss)
{
	int nIndex;
	int nEndPointCount;
	BOOL fTriedFirmwareUpload = FALSE;

	if (SourceHelper_ValidateSourceContainer(pss) == FALSE)
		return FALSE;
	OutputDebugString("DVBWorld: Init\n");
	InitializeCriticalSection(&csSignal);

	ss = pss;

	if (OpenUSBDriver(FALSE) == FALSE)
		return FALSE;

	// Find the USB pipe we use for data transfer (endpoint IN 2)
TSReader_Init_RetryAfterFirmwareUpload:
	nEndPointCount = USBDevice->EndPointCount();  
	for (nIndex = 1; nIndex < nEndPointCount; nIndex++)
	{
		if (USBDevice->EndPoints[nIndex]->Address == 0x82)
		{
			MPEGInEpt = (CCyBulkEndPoint *)USBDevice->EndPoints[nIndex];
			break;
		}
	}
	if (nIndex == nEndPointCount)
	{
		if (fTriedFirmwareUpload == FALSE)
		{
			if (UploadFirmware())
			{
				fTriedFirmwareUpload = TRUE;
				goto TSReader_Init_RetryAfterFirmwareUpload;
			}
			return FALSE;
		}
		else
		{
			OutputDebugString("DVBWorld: Couldn't find MPEG in endpoint\n");
			return FALSE;
		}
	}

#ifndef MATCHBOXPRO
	if (!fSharpInitialized)
	{
		InitSharp();
		fSharpInitialized = TRUE;
	}
#endif MATCHBOXPRO

	return TRUE;
}

BOOL TSReader_DeInit()
{
	OutputDebugString("DVBWorld: +DeInit\n");

	{
		char szTemp[128];
		wsprintf(szTemp, "DVBWorld: USBDevice->EndPoints = 0x%08x\n", USBDevice->EndPoints);
		OutputDebugString(szTemp);
	}
	if (USBDevice->EndPoints != NULL)
	{
		delete USBDevice;
	}

	DeleteCriticalSection(&csSignal);

	OutputDebugString("DVBWorld: -DeInit\n");
	return TRUE;
}

BOOL TSReader_TuneDialog(HWND hWnd)
{
	OutputDebugString("DVBWorld: TSReader_TuneDialog\n");

	if (ss->fDontTune == TRUE)
		fNeedTuneDialog = FALSE;

	if (fNeedTuneDialog)
	{
		ss->fQuietMode = FALSE;
		OutputDebugString("DVBWorld: TSReader_TuneDialog tuning dialog is required\n");
#ifndef DSS
		if (SourceHelper_DVBSTuneDialog(hWnd) == FALSE)
#else DSS
		if (SourceHelper_DSSTuneDialog(hWnd) == FALSE)
#endif DSS
			return FALSE;
	}
	else
	{
		OutputDebugString("DVBWorld: TSReader_TuneDialog tune NOT required\n");
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
						| CAPABILITIES_DISEQC_POSITIONER
		                | CAPABILITIES_DISEQC;
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
#ifndef MATCHBOXPRO
	int i;
	int nTimeout;

	// Make sure power is on
	bPowerCommand[1] = ss->nPolarity;
	WriteB2Command(bPowerCommand, sizeof(bPowerCommand));

	WriteSharpRegister(0x08, 0x06);		// DiSEqC - DiSEqC mode
	for (i = 0; i < nLength; i++)
	{
		while ((ReadSharpRegister(0x0a) & 1) == 1)
			Sleep(1);	// stall if FIFO full
		WriteSharpRegister(0x09, bCommand[i]);
	}
	nTimeout = 100;
	while ((ReadSharpRegister(0x0a) & 2) == 2 && nTimeout-- > 0)
		Sleep(1);		// stall until FIFO empty
	Sleep(15);
	if (ss->n22KHz)
		WriteSharpRegister(0x08, 0x03);		// DiSEqC - tone on
	else
		WriteSharpRegister(0x08, 0x04);		// DiSEqC - tone off
	Sleep(15);
#endif MATCHBOXPRO
	return TRUE;
}

int TSReader_GetSyncLossCount(BOOL fReset)
{
	return SourceHelper_GetSyncLossCount(fReset);
}

int TSReader_GetRetuneCount(BOOL fReset)
{
	if (fReset)
		nRetuneCount = 0;
	return nRetuneCount;
}

BOOL APIENTRY DllMain(HANDLE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
    switch( ul_reason_for_call )
	{
    case DLL_PROCESS_ATTACH:
		hInstance = (HINSTANCE)hModule;
		fSharpInitialized = FALSE;
		break;
    case DLL_PROCESS_DETACH:
		break;
    }
    return TRUE;
}
