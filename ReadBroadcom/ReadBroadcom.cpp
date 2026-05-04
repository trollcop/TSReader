// You'll need the Windows 2000 or later Platform SDK from Microsoft to build this sample
#include <windows.h>
#include <stdio.h>
#include <setupapi.h>
#include <initguid.h>

#include "resource.h"

// This is part of the Cypress "USB Developer's uStudio". That can be downloaded from this URL:
// http://www.cypress.com/support/reference_designs.cfm?objectID=059DD019-11E8-405D-844010255E17671C&tid=54A040FA-2262-424A-B14741267CBD1308
// We assume you've installed the CyAPI header/library files in C:\SDKs\Cypress
#include "CyAPI.h"

// These are the command definitions for the USB interface
#include "../USBInterfaceSample/hardware.h"

CCyUSBDevice * USBDevice = NULL;
CCyBulkEndPoint * MPEGInEpt = NULL;   
OVERLAPPED inOvLap, outOvLap;
HINSTANCE hInstance;
char gszAppName[] = {"Read Broadcom Firmware Utility"};

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

BOOL ReadBroadcomMemory(HWND hDlg)
{
	HANDLE hFile;
	DWORD dwWritten;
	int i;
	BOOL fRetVal = TRUE;
	char szFirmwareFile[MAX_PATH];

	CursorWait(hDlg);
	GetModuleFileName((HMODULE)hInstance, szFirmwareFile, sizeof(szFirmwareFile));
	for (i = lstrlen(szFirmwareFile); i > 0; i--)
	{
		if (szFirmwareFile[i] == '\\')
		{
			szFirmwareFile[i + 1] = 0;
			break;
		}
	}
	lstrcat(szFirmwareFile, "oasis_8psk_firmware.bin");

	hFile = CreateFile(szFirmwareFile, GENERIC_WRITE, 0, (LPSECURITY_ATTRIBUTES) NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, (HANDLE) NULL);
	if (hFile != INVALID_HANDLE_VALUE)
	{
		int nRemaining;
		int nAddr;
		BYTE bOut[4];
		BYTE bIn[64];
		char szTemp[64];

		nAddr = 0x0000;
		nRemaining = 0x4000;
		do
		{
			int nThisTime = nRemaining;
			if (nThisTime > 64)
				nThisTime = 64;

			bOut[0] = outReadBroadcomRAM;
			bOut[1] = nThisTime;
			bOut[2] = (nAddr >> 8) & 0xff;
			bOut[3] = nAddr & 0xff;
			if (!SendUSB(bOut, sizeof(bOut)))
			{
				SetDlgItemText(hDlg, IDC_STATUS, "Problem telling the USB interface to read data");
				fRetVal = FALSE;
				break;
			}

			ReceiveUSB(bIn, nThisTime);
			WriteFile(hFile, bIn, nThisTime, &dwWritten, NULL);
			wsprintf(szTemp, "Read %d bytes from address 0x%04x", nThisTime, nAddr);
			SetDlgItemText(hDlg, IDC_STATUS, szTemp);
			nAddr += nThisTime;
			nRemaining -= nThisTime;
		} while (nRemaining);
		CloseHandle(hFile);
	}
	else
	{
		SetDlgItemText(hDlg, IDC_STATUS, "Problem opening the oasis_8psk_firmware.bin file");
		fRetVal = FALSE;
	}

	CursorNormal();
	return fRetVal;
}

BOOL OpenUSBDriver()
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

	return TRUE;
}

BOOL CALLBACK MainDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg)
	{
	case WM_INITDIALOG:
		outOvLap.hEvent = CreateEvent(NULL, false, false, "CYUSB_OUT"); 
		inOvLap.hEvent = CreateEvent(NULL, false, false, "CYUSB_IN"); 
		if (OpenUSBDriver() == FALSE)
		{
			MessageBox(hDlg, "Unable to open USB driver", gszAppName, MB_ICONSTOP);
			EndDialog(hDlg, FALSE);
			break;
		}
		break;
	case WM_CLOSE:
		EndDialog(hDlg, FALSE);
		break;
	case WM_DESTROY:
		CloseHandle(outOvLap.hEvent);
		CloseHandle(inOvLap.hEvent);
		break;
	case WM_COMMAND:
		switch(LOWORD(wParam))
		{
		case IDCANCEL:
			EndDialog(hDlg, FALSE);
			break;
		case IDOK:
			if (ReadBroadcomMemory(hDlg) == TRUE)
				SetDlgItemText(hDlg, IDC_STATUS, "Sucessfully read into oasis_8psk_firmware.bin in the same folder as this program");
			MessageBeep(0);
			break;
		}
		break;
	}

	return FALSE;
}

int WINAPI WinMain(HINSTANCE hInputInstance, HINSTANCE hPrevInstance, LPSTR lpszCmdLine, int nCmdShow)
{
	hInstance = hInputInstance;
	DialogBox(hInstance, MAKEINTRESOURCE(IDD_MAIN_DLG), NULL, MainDlgProc);
	return 0;
}
