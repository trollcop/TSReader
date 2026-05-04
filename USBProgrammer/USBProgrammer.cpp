// You'll need the Windows 2000 or later Platform SDK from Microsoft to build this sample
#include <windows.h>
#include <stdio.h>
#include <setupapi.h>
#include <initguid.h>
#include <commctrl.h>

#include "resource.h"

// This is part of the Cypress "USB Developer's uStudio". That can be downloaded from this URL:
// http://www.cypress.com/support/reference_designs.cfm?objectID=059DD019-11E8-405D-844010255E17671C&tid=54A040FA-2262-424A-B14741267CBD1308
// We assume you've installed the CyAPI header/library files in C:\SDKs\Cypress
#include "CyAPI.h"

// Global variables
CCyUSBDevice * USBDevice = NULL;
CCyBulkEndPoint * MPEGInEpt = NULL;   
OVERLAPPED inOvLap, outOvLap;
HINSTANCE hInstance;
BOOL fFullMode = FALSE;
char gszAppName[] = {"USB Interface Programmer"};

// Code
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

BOOL SendUSB(BYTE * bOut, int nSize)
{
	LONG nBytesSent;

	UCHAR  * outContext = USBDevice->BulkOutEndPt->BeginDataXfer(bOut, nSize, &outOvLap); 
	USBDevice->BulkOutEndPt->WaitForXfer(&outOvLap, 100);
	return (USBDevice->BulkOutEndPt->FinishDataXfer(bOut, nBytesSent, &outOvLap,outContext));
}

int ReceiveUSB(BYTE * bIn, int nSize)
{
	LONG nBytesReceived;

	UCHAR  *inContext = USBDevice->BulkInEndPt->BeginDataXfer(bIn, nSize, &inOvLap); 
	USBDevice->BulkInEndPt->WaitForXfer(&inOvLap, 100); 
	USBDevice->BulkInEndPt->FinishDataXfer(bIn, nBytesReceived, &inOvLap,inContext);

	return nBytesReceived;
}

void ReadEEPROM(int nAddress, BYTE * pData, int nLength)
{
	BYTE bOut[5];
	
	bOut[0] = 18;		// I2C read command
	bOut[1] = 0xA2;				// address
	bOut[2] = nLength;			// byte count
	bOut[3] = nAddress >> 8;
	bOut[4] = nAddress & 0xff;
	SendUSB(bOut, sizeof(bOut));
	ReceiveUSB(pData, nLength);
}

void EEPROMWriteEnable(BOOL fEnable)
{
	BYTE bOut[2];

	bOut[0] = 27;
	bOut[1] = fEnable;
	SendUSB(bOut, sizeof(bOut));
}

BOOL EEPROMWriteEnableRequired()
{
	if (USBDevice->VendorID == 0x14AC && (USBDevice->ProductID & 0x0fff) == 0x0007) // cielplus 5000
		return TRUE;
	if (USBDevice->VendorID == 0x14AC && (USBDevice->ProductID & 0x0fff) == 0x0006) // cielplus sky
		return TRUE;
	if (USBDevice->VendorID == 0x14AC && (USBDevice->ProductID & 0x0fff) == 0x0005) // MPEG-2 output
		return TRUE;
	if (USBDevice->VendorID == 0x14AC && (USBDevice->ProductID & 0x0fff) == 0x0003) // DVB-SPI
		return TRUE;
	if (USBDevice->VendorID == 0x14AC && (USBDevice->ProductID & 0x0fff) == 0x0100) // DVBTech 8PSK
		return TRUE;
	if (USBDevice->VendorID == 0x14AC && (USBDevice->ProductID & 0x0fff) == 0x0010) // Horizon TSR-S1
		return TRUE;

	return FALSE;
}

void WriteEEPROM(int nAddress, BYTE * pData, int nLength)
{
	BYTE bOut[64];

	if (EEPROMWriteEnableRequired())
		EEPROMWriteEnable(TRUE);

	bOut[0] = 3;		// I2C write command
	bOut[1] = 0xA2;				// address
	bOut[2] = nLength + 2;		// byte count
	bOut[3] = nAddress >> 8;
	bOut[4] = nAddress & 0xff;
	memcpy(&bOut[5], pData, nLength);
	SendUSB(bOut, 3 + 2 + nLength);

	if (EEPROMWriteEnableRequired())
		EEPROMWriteEnable(FALSE);
}

int OpenUSBDriver(HWND hDlg)
{
	GUID InterfaceGUID;
	InterfaceGUID.Data1 = 0xFA58C45D;
	InterfaceGUID.Data2 = 0x5B19; InterfaceGUID.Data3 = 0x428b;
	InterfaceGUID.Data4[0] = 0xA2; InterfaceGUID.Data4[1] = 0xD1; InterfaceGUID.Data4[2] = 0x27; InterfaceGUID.Data4[3] = 0x07;
	InterfaceGUID.Data4[4] = 0x85; InterfaceGUID.Data4[5] = 0x6D; InterfaceGUID.Data4[6] = 0x7E; InterfaceGUID.Data4[7] = 0x19;
	
	USBDevice = new CCyUSBDevice(NULL, InterfaceGUID);
    int nDevices = USBDevice->DeviceCount(); 
	if (nDevices == 0)
		return FALSE;
	if (USBDevice->EndPoints == NULL)
	{
		OutputDebugString("?NULL endpoint from the USB device\n");
		return FALSE;
	}
	if (nDevices > 1)
	{
		MessageBox(NULL, "More than one supported USB Interface is currently connected. To upgrade the firmware\nyou must only have one supported USB Interface connected.\n\nPlease disconnect all supported USB Interfaces except the one you've selected to upgrade.", gszAppName, MB_ICONSTOP);
		return -1;
	}

	return TRUE;
}

int ReadLine(HANDLE hFile, char * szBuffer, int nMaxLength)
{
	DWORD dwBytesRead;
	char szTemp[2];
	int nOutputPosition = 0;

	do
	{
		ReadFile(hFile, szTemp, 1, &dwBytesRead, NULL);
		if (dwBytesRead != 1)
			return nOutputPosition;
		if (szTemp[0] == 0x0d)
		{
			szBuffer[nOutputPosition] = '\0';
			return nOutputPosition;
		}
		if (szTemp[0] != 0x0a)
			szBuffer[nOutputPosition++] = szTemp[0];
		if (nOutputPosition == nMaxLength - 2)
		{
			szBuffer[nOutputPosition] = '\0';
			return nOutputPosition;
		}
	} while (TRUE);
}

void WriteByteBuffer(HWND hDlg, BYTE * bBuffer, int nAddress, int nLength, int * nCurrentEEPROMAddress)
{
	int nMyCurrentEEPROM = *nCurrentEEPROMAddress;
	int i;
	BYTE bWriteData;

	SendMessage(GetDlgItem(hDlg, IDC_PROGRESS), PBM_SETPOS, nMyCurrentEEPROM, 0);

	bWriteData = nLength >> 8;
	WriteEEPROM(nMyCurrentEEPROM++, &bWriteData, 1);
	bWriteData = nLength & 0xff;
	WriteEEPROM(nMyCurrentEEPROM++, &bWriteData, 1);

	bWriteData = nAddress >> 8;
	WriteEEPROM(nMyCurrentEEPROM++, &bWriteData, 1);
	bWriteData = nAddress & 0xff;
	WriteEEPROM(nMyCurrentEEPROM++, &bWriteData, 1);

	for (i = 0; i < nLength; i++)
		WriteEEPROM(nMyCurrentEEPROM++, &bBuffer[i], 1);
	
	*nCurrentEEPROMAddress = nMyCurrentEEPROM;
}

unsigned int CalculateChecksum(char * szLineBuffer)
{
	int nLength, i;
	unsigned int nData;
	BYTE bChecksum = 0;
	char szData[4];

	nLength = lstrlen(szLineBuffer) - 3;
	for (i = 0; i < nLength / 2; i++)
	{
		memcpy(szData, &szLineBuffer[1 + (i * 2)], 2); szData[2] = '\0';
		sscanf(szData, "%x", &nData);
		bChecksum += (BYTE)nData;
	}

	return (0x100 - bChecksum) & 0xff;
}

BOOL SetNewDeviceID(HWND hDlg, BOOL fQuiet)
{
	int nOffset = 0;
	int nDeviceID = -1;
	int nNewDeviceID;
	int nBaseAddress;
	BYTE bStartCode[16];

	nNewDeviceID = GetDlgItemInt(hDlg, IDC_DEVICE_ID, NULL, FALSE);
	if (nNewDeviceID < 0 || nNewDeviceID > 7)
	{
		MessageBox(hDlg, "Please enter a device ID between 0 and 7", gszAppName, MB_ICONSTOP);
		return FALSE;
	}

	nBaseAddress = 0;
	while (nBaseAddress < 8192)
	{
		ReadEEPROM(nBaseAddress, bStartCode, 16);
		while (nOffset < 15)
		{
			if (bStartCode[nOffset] == 0xac && bStartCode[nOffset + 1] == 0x14)
			{
				bStartCode[nOffset + 3] = (bStartCode[nOffset + 3] & 0x0f)
										| (nNewDeviceID << 4);
				WriteEEPROM(nBaseAddress, bStartCode, 16);
				if (!fQuiet)
					MessageBox(hDlg, "Device ID set. Now disconnect the USB cable and power if applicable. Once powered back on\nand reconnected you may see the New Device wizard", gszAppName, MB_ICONINFORMATION);
				return TRUE;
			}
			nOffset++;
		}
		nBaseAddress += 16;
		nOffset = 0;
	}
	if (!fQuiet)
		MessageBox(hDlg, "Current interface is not programmed - please program first", gszAppName, MB_ICONSTOP);
	return FALSE;
}

void ReprogramEEPROM(HWND hDlg, HANDLE hFile)
{
	BOOL fAbort = FALSE;
	int nCurrentEEPROMAddress = 0;
	int nCurrentFX2Address = -1;
	int nFX2StartAddress = -1;
	int nByteCounter = 0;
	int i;
	BYTE bZero[] = {0xc2, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01};
	BYTE bFinal[] = {0x80, 0x01, 0xe6, 0x00, 0x00};
	BYTE bBuffer[512];

	CursorWait(hDlg);

	// First write a zero to location zero - if we crash we can reprogram with Cypress tools
	for (i = 0; i < sizeof(bZero); i++)
		WriteEEPROM(nCurrentEEPROMAddress++, &bZero[i], 1);
	do
	{
		int nRead;
		int nRecordLength, nAddress, nRecordType;
		unsigned int nData;
		unsigned int nCalculatedChecksum;
		char szLineBuffer[256];
		char szRecordLength[4];
		char szAddress[6];
		char szRecordType[4];
		char szData[4];
		BYTE bData[64];

		nRead = ReadLine(hFile, szLineBuffer, sizeof(szLineBuffer));
		if (nRead == 0)
			break;
		if (szLineBuffer[0] != ':')
		{
			MessageBox(hDlg, "Badly formatted hex file.\n\nDo NOT power cycle your USB interface until reprogrammed correctly!", gszAppName, MB_ICONSTOP);
			fAbort = TRUE;
			break;
		}
		nCalculatedChecksum = CalculateChecksum(szLineBuffer);

		memcpy(szRecordLength, &szLineBuffer[1], 2); szRecordLength[2] = '\0';
		sscanf(szRecordLength, "%x", &nRecordLength);
		memcpy(szAddress, &szLineBuffer[3], 4); szAddress[4] = '\0';
		sscanf(szAddress, "%x", &nAddress);
		memcpy(szRecordType, &szLineBuffer[7], 2); szRecordType[2] = '\0';
		sscanf(szRecordType, "%x", &nRecordType);
		if (nRecordType == 0x02)
		{
			MessageBox(hDlg, "Hex file contains extended address fields which aren't supported\n\nDo NOT power cycle your USB interface until reprogrammed correctly!", gszAppName, MB_ICONSTOP);
			fAbort = TRUE;
			break;
		}
		else if (nRecordType == 0x01)
			break;

		for (i = 0; i < nRecordLength; i++)
		{
			memcpy(szData, &szLineBuffer[9 + (i * 2)], 2); szData[2] = '\0';
			sscanf(szData, "%x", &nData);
			bData[i] = (BYTE)nData;
		}
		memcpy(szData, &szLineBuffer[9 + (nRecordLength * 2)], 2); szData[2] = '\0';
		sscanf(szData, "%x", &nData);
		if (nData != nCalculatedChecksum)
		{
			char szTemp[256];
			wsprintf(szTemp, "Checksum failure - calculated = 0x%02x found = 0x%02x on line:\n%s\n\nDo NOT power cycle your USB interface until reprogrammed correctly!",
				     nCalculatedChecksum, nData, szLineBuffer);
			MessageBox(hDlg, szTemp, gszAppName, MB_ICONSTOP);
			OutputDebugString(szLineBuffer);
			OutputDebugString("\n");
			fAbort = TRUE;
			break;
		}

		// Buffer up to 1K provided the address is contiguous
		if (nCurrentFX2Address == -1)
		{
			nCurrentFX2Address = nAddress;
			nFX2StartAddress = nAddress;
		}

		if (   (nAddress != nCurrentFX2Address)
			|| (nByteCounter + nRecordLength > 512) )
		{
			WriteByteBuffer(hDlg, bBuffer, nFX2StartAddress, nByteCounter, &nCurrentEEPROMAddress);
			nCurrentFX2Address = nAddress;
			nFX2StartAddress = nAddress;
			nByteCounter = 0;
		}
		memcpy(&bBuffer[nByteCounter], bData, nRecordLength);
		nByteCounter += nRecordLength;	
		nCurrentFX2Address += nRecordLength;

	} while (TRUE);

	// All done - write the end packet to bring the chip out of reset
	// and then C2 to zero so the hardware knows what to do
	if (!fAbort)
	{
		SendMessage(GetDlgItem(hDlg, IDC_PROGRESS), PBM_SETPOS, 8192, 0);
		if (nByteCounter)
			WriteByteBuffer(hDlg, bBuffer, nFX2StartAddress, nByteCounter, &nCurrentEEPROMAddress);
		{
			char szTemp[128];
			wsprintf(szTemp, "Final EEPROM address = 0x%04x (%d)\n", nCurrentEEPROMAddress, nCurrentEEPROMAddress);
			OutputDebugString(szTemp);
		}
		if (nCurrentEEPROMAddress > 8)		// prevent zero length firmware
		{
			for (i = 0; i < sizeof(bFinal); i++)
				WriteEEPROM(nCurrentEEPROMAddress++, &bFinal[i], 1);
			SetNewDeviceID(hDlg, TRUE);
			MessageBox(hDlg, "Firmware upgrade completed!\n\nClose this window and then cycle the power on the device", gszAppName, MB_ICONINFORMATION);
			PostMessage(hDlg, WM_CLOSE, 0, 0);
		}
		else
		{
			MessageBox(hDlg, "Invalid HEX file!\n\nDo NOT power cycle your USB interface until reprogrammed correctly!", gszAppName, MB_ICONSTOP);
		}
	}

	CursorNormal();
	
}

void ReadCurrentEEPROM(HWND hDlg)
{
	int i;
	DWORD dwWritten;
	HANDLE hFile = CreateFile("eeprom.bin", GENERIC_WRITE, 0, (LPSECURITY_ATTRIBUTES) NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, (HANDLE) NULL);
	BYTE bBuffer[8192];

	CursorWait(hDlg);
	for (i = 0; i < 8192; i += 32)
	{
		SendMessage(GetDlgItem(hDlg, IDC_PROGRESS), PBM_SETPOS, i, 0);
		ReadEEPROM(i, &bBuffer[i], 32);
	}
	WriteFile(hFile, bBuffer, 8192, &dwWritten, NULL);
	CloseHandle(hFile);
	CursorNormal();
	MessageBeep(0);
}

BOOL CALLBACK MainDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg)
	{
	case WM_INITDIALOG:
		{
			int nOffset = 0;
			int nDeviceID = -1;
			BOOL fFailed = FALSE;
			BYTE bStartCode[16];

			outOvLap.hEvent = CreateEvent(NULL, false, false, "CYUSB_OUT"); 
			inOvLap.hEvent = CreateEvent(NULL, false, false, "CYUSB_IN"); 

			switch(OpenUSBDriver(hDlg))
			{
			case FALSE:
				MessageBox(hDlg, "Unable to open USB driver", gszAppName, MB_ICONSTOP);
				EndDialog(hDlg, FALSE);
				fFailed = TRUE;
				break;
			case -1:
				EndDialog(hDlg, FALSE);
				fFailed = TRUE;
				break;
			}
			if (fFailed)
				break;

			SendDlgItemMessage(hDlg, IDC_PROGRESS, PBM_SETRANGE, 0, MAKELPARAM (0, 8192));
			if (fFullMode)
			{
				ShowWindow(GetDlgItem(hDlg, IDC_READ_CURRENT), SW_SHOW);
				ShowWindow(GetDlgItem(hDlg, IDC_CLEAR_EEPROM), SW_SHOW);
			}

			SetDlgItemText(hDlg, IDC_DEVICE_DETECTED, USBDevice->DeviceName);
			ReadEEPROM(0x10, bStartCode, 16);
			while (nOffset < 15)
			{
				if (bStartCode[nOffset] == 0xac && bStartCode[nOffset + 1] == 0x14)
				{
					nDeviceID = bStartCode[nOffset + 3] >> 4 & 0x0f;
					break;
				}
				nOffset++;
			}
			if (nDeviceID != -1)
				SetDlgItemInt(hDlg, IDC_DEVICE_ID, nDeviceID, FALSE);
			else
				SetDlgItemText(hDlg, IDC_DEVICE_ID, "n/a");
		}
		break;
	case WM_DESTROY:
		CloseHandle(outOvLap.hEvent);
		CloseHandle(inOvLap.hEvent);
		break;
	case WM_CLOSE:
		EndDialog(hDlg, FALSE);
		break;
	case WM_COMMAND:
		switch(LOWORD(wParam))
		{
		case IDC_SET_DEVICE_ID:
			if (SetNewDeviceID(hDlg, FALSE) == TRUE)
				EndDialog(hDlg, TRUE);
			break;
		case IDOK:
			{
				HANDLE hFile;
				char szHexFile[MAX_PATH];
				
				GetDlgItemText(hDlg, IDC_HEX_FILE, szHexFile, sizeof(szHexFile));
				hFile = CreateFile(szHexFile, GENERIC_READ, 0, (LPSECURITY_ATTRIBUTES) NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, (HANDLE) NULL);
				if (hFile == INVALID_HANDLE_VALUE)
				{
					MessageBox(hDlg, "Unable to open the hex file you've specified", gszAppName, MB_ICONSTOP);
					break;
				}
				ReprogramEEPROM(hDlg, hFile);
				CloseHandle(hFile);
			}
			break;
		case IDCANCEL:
			EndDialog(hDlg, FALSE);
			break;
		case IDC_BROWSE:
			{
				OPENFILENAME ofn;
				char szInputFile[MAX_PATH] = {0};

				memset( &(ofn), 0, sizeof(ofn));
				ofn.lStructSize	= sizeof(ofn);
				ofn.hwndOwner = hDlg;
				ofn.lpstrFile = szInputFile;
				ofn.nMaxFile = MAX_PATH;
				ofn.lpstrFilter = TEXT("Hex Files(*.hex)\0*.hex\0All Files (*.*)\0*.*\0\0");	
				ofn.lpstrTitle = TEXT("Select firmware upgrade file");
				ofn.lpstrDefExt = TEXT("hex");
				ofn.Flags =  OFN_HIDEREADONLY;
				ofn.lpstrInitialDir = NULL;
				
				if (GetOpenFileName(&ofn) == TRUE)
					SetDlgItemText(hDlg, IDC_HEX_FILE, szInputFile);
			}
			break;
		case IDC_READ_CURRENT:
			ReadCurrentEEPROM(hDlg);
			break;
		case IDC_CLEAR_EEPROM:
			{
				int i;
				BYTE bBuffer[32];

				memset(bBuffer, 0xff, 32);
				for (i = 0; i < 8192; i += 32)
				{
					SendMessage(GetDlgItem(hDlg, IDC_PROGRESS), PBM_SETPOS, i, 0);
					WriteEEPROM(i, bBuffer, 32);
				}
				MessageBeep(0);
			}
			break;
		}
		break;
	}
	return FALSE;
}

int WINAPI WinMain(HINSTANCE hInputInstance, HINSTANCE hPrevInstance, LPSTR lpszCmdLine, int nCmdShow)
{
	hInstance = hInputInstance;
	InitCommonControls();
	if (lstrcmp(lpszCmdLine, "-xyzzy") == 0)
		fFullMode = TRUE;
	DialogBox(hInstance, MAKEINTRESOURCE(IDD_MAIN_DLG), NULL, MainDlgProc);
	return 0;
}
