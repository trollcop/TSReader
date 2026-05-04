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
#include "hardware.h"

// Global variables
CCyUSBDevice * USBDevice = NULL;
CCyBulkEndPoint * MPEGInEpt = NULL;   
OVERLAPPED inOvLap, outOvLap;
HANDLE hOutputFile = INVALID_HANDLE_VALUE;
BOOL fAbortReadMPEGThread = FALSE;
BOOL fReadMPEGTheadRunning = FALSE;
HINSTANCE hInstance;
double dBytesReceived = 0.0;
char gszAppName[] = {"USB Interface Sample App"};

#define READ_SIZE 128 * 1024

// Code
BOOL SendUSB(BYTE * bOut, int nSize)
{
	LONG nBytesSent;

	UCHAR  * outContext = USBDevice->BulkOutEndPt->BeginDataXfer(bOut, nSize, &outOvLap); 
	USBDevice->BulkOutEndPt->WaitForXfer(&outOvLap, 1000);
	return (USBDevice->BulkOutEndPt->FinishDataXfer(bOut, nBytesSent, &outOvLap,outContext));
}

int ReceiveUSB(BYTE * bIn, int nSize)
{
	LONG nBytesReceived;

	UCHAR  *inContext = USBDevice->BulkInEndPt->BeginDataXfer(bIn, nSize, &inOvLap); 
	USBDevice->BulkInEndPt->WaitForXfer(&inOvLap, 1000); 
	USBDevice->BulkInEndPt->FinishDataXfer(bIn, nBytesReceived, &inOvLap,inContext);

	return nBytesReceived;
}

BYTE ReadSTLNBP21()
{
	BYTE bOut[2];
	BYTE bIn[1] = {0};

	bOut[0] = outReadSTLNBP21;
	bOut[1] = 0x10;
	if (!SendUSB(bOut, sizeof(bOut)))
		OutputDebugString("?Write for ReadSTLNBP21 failed\n");
	else
		ReceiveUSB(bIn, sizeof(bIn));
	return bIn[0];
}

void WriteSTLNBP21(BYTE bValue)
{
	BYTE bOut[3];

	bOut[0] = outWriteSTLNBP21;
	bOut[1] = 0x10;
	bOut[2] = bValue;
	if (!SendUSB(bOut, sizeof(bOut)))
		OutputDebugString("?WriteSTLNBP21 failed\n");
}

void Setup22KHz(BOOL f22KHz)
{
	BYTE bOut[2];

	bOut[0] = out22KHzOnOff;
	bOut[1] = f22KHz;
	if (!SendUSB(bOut, sizeof(bOut)))
		OutputDebugString("?Write 22KHz tone control failed\n");
}

void SetTuner22KHzAndPower(BOOL f22KHz, BOOL fPower, int nPolarity)
{
	if (fPower == FALSE)
	{
		Setup22KHz(FALSE);
		WriteSTLNBP21(ISEL1);
	}
	else
	{
		switch(nPolarity)
		{
		case 0:	// vertical
			WriteSTLNBP21(ISEL1 | EN1);
			Setup22KHz(f22KHz);
			break;
		case 1:	// horizontal
			WriteSTLNBP21(ISEL1 | VSEL1 | EN1);
			Setup22KHz(f22KHz);
			break;
		default: // power off
			WriteSTLNBP21(ISEL1);
			break;
		}
	}
}

void SelectDiSEqCInput(int nInput)
{
	BYTE bPositionByte[] = {0xc0, 0xc4, 0xc8, 0xcc};

	nInput--;
	if ((nInput >= 0) && (nInput <= 3) )
	{
		int i;

		for (i = 0; i < 3; i++)
		{
			BYTE bOut[6];

			bOut[0] = outDiSEqC;
			bOut[1] = 4;
			bOut[2] = 0xe0;
			bOut[3] = 0x10;
			bOut[4] = 0x38;
			bOut[5] = bPositionByte[nInput];
			if (!SendUSB(bOut, sizeof(bOut)))
				OutputDebugString("?DiSEqC failed\n");

			// Stall while the firmware sends the DiSEqC command. 12.5 ms per byte
			// plus 15 ms after all bytes
			Sleep((bOut[1] * 13) + 15 + 10);
		}
	}
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

BOOL CheckUSB20()
{
	BYTE bOut[1];
	BYTE bIn[2] = {0, 0};

	bOut[0] = outGetConfiguration;
	if (!SendUSB(bOut, sizeof(bOut)))
		OutputDebugString("?Write for GetConfiguration failed\n");
	else
		ReceiveUSB(bIn, sizeof(bIn));
	return ((bIn[1] & 1) == 1);
}

BOOL BroadcomDemodRunning()
{
	BYTE bOut[1];
	BYTE bIn[2] = {0, 0};

	bOut[0] = outGetConfiguration;
	if (!SendUSB(bOut, sizeof(bOut)))
		OutputDebugString("?Write for GetConfiguration failed\n");
	else
		ReceiveUSB(bIn, sizeof(bIn));
	return ((bIn[1] & 4) == 4);
}

BOOL LastTuneDCIIOQPSK()
{
	BYTE bOut[1];
	BYTE bIn[2] = {0, 0};

	bOut[0] = outGetConfiguration;
	if (!SendUSB(bOut, sizeof(bOut)))
		OutputDebugString("FX2: Write for GetConfiguration failed\n");
	ReceiveUSB(bIn, sizeof(bIn));
	if (bIn[1] & 0x08)
		return TRUE;
	return FALSE;
}


WORD WriteAndReadDNIF(WORD wOut)
{
	WORD wRetVal;
	BYTE bOut[3];
	BYTE bIn[2] = {0, 0};

	bOut[0] = outDNHardware;
	bOut[1] = wOut >> 8;
	bOut[2] = wOut & 0xff;
	if (!SendUSB(bOut, sizeof(bOut)))
		OutputDebugString("?DNHardware out failed\n");
	else
		ReceiveUSB(bIn, sizeof(bIn));
	wRetVal = bIn[0] << 8 | bIn[1];
	return wRetVal;
}

void DoDNIF(HWND hDlgProgress, WORD wOut)
{
	WriteAndReadDNIF(wOut);
	SendMessage(hDlgProgress, PBM_STEPIT, 0, 0);
}

void SetupDNIF(HWND hDlg)
{
	HWND hDlgProgress = GetDlgItem(hDlg, IDC_BROADCOM_PROGRESS);

	DoDNIF(hDlgProgress, 0x8000);

	DoDNIF(hDlgProgress, 0x0200);
	DoDNIF(hDlgProgress, 0x0200);
	DoDNIF(hDlgProgress, 0x0100);
	DoDNIF(hDlgProgress, 0x0100);
	DoDNIF(hDlgProgress, 0x0100);
	DoDNIF(hDlgProgress, 0x0100);
	DoDNIF(hDlgProgress, 0x0100);
	DoDNIF(hDlgProgress, 0x0100);
	Sleep(100);

	DoDNIF(hDlgProgress, 0x0201);	// 3V on after this
	DoDNIF(hDlgProgress, 0x0203); // CLK driven after this
	Sleep(100);

	DoDNIF(hDlgProgress, 0x0104);	// 
	Sleep(100);

	DoDNIF(hDlgProgress, 0x0100);	// I2C relay off
	DoDNIF(hDlgProgress, 0x0102);	// I2C relay on
	DoDNIF(hDlgProgress, 0x0103);	// I2C relay on and bus on
}

BOOL WriteBroadcomMemory(HWND hDlg)
{
	HANDLE hFile;
	DWORD dwRead;
	int i;
	BOOL fRetVal = TRUE;
	char szFirmwareFile[MAX_PATH];

	GetModuleFileName((HMODULE)hInstance, szFirmwareFile, sizeof(szFirmwareFile));
	for (i = lstrlen(szFirmwareFile); i > 0; i--)
	{
		if (szFirmwareFile[i] == '\\')
		{
			szFirmwareFile[i + 1] = 0;
			break;
		}
	}
	lstrcat(szFirmwareFile, "dvbtech_8psk_firmware.bin");

	hFile = CreateFile(szFirmwareFile, GENERIC_READ, 0, (LPSECURITY_ATTRIBUTES) NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, (HANDLE) NULL);
	if (hFile != INVALID_HANDLE_VALUE)
	{
		BYTE bOut[64];
		BYTE * pBuffer = (BYTE *)LocalAlloc(LPTR, 0x4000);

		ReadFile(hFile, pBuffer, 0x4000, &dwRead, NULL);
		if (dwRead == 0x4000)
		{
			int nAddr = 0;
			int nRemaining = 0x4000;
			int nThisTime;
			int nProgress = 0;

			SendDlgItemMessage(hDlg, IDC_BROADCOM_PROGRESS, PBM_SETRANGE32, 0, 0x4000);

			do
			{
				nThisTime = nRemaining;
				if (nThisTime > 60)
					nThisTime = 60;
				nProgress += nThisTime;
				SendDlgItemMessage(hDlg, IDC_BROADCOM_PROGRESS, PBM_SETPOS, nProgress, 0);

				bOut[0] = outWriteBroadcomRAM;
				bOut[1] = nThisTime;
				bOut[2] = (nAddr >> 8) & 0xff;
				bOut[3] = nAddr & 0xff;
				memcpy(&bOut[4], &pBuffer[nAddr], nThisTime);
				SendUSB(bOut, nThisTime + 4);
				nAddr += nThisTime;
				nRemaining -= nThisTime;
			} while (nRemaining);
		}
		else
		{
			OutputDebugString("Failed to read 0x4000 bytes\n");
			fRetVal = FALSE;
		}

		LocalFree(pBuffer);
		CloseHandle(hFile);
		Sleep(100);
	}
	else
		fRetVal = FALSE;

	return fRetVal;
}

BOOL CALLBACK LoadBroadcomDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	static BOOL fFirstTime;

	switch(uMsg)
	{
	case WM_INITDIALOG:
		fFirstTime = TRUE;
		break;
	case WM_ACTIVATE:
		if (fFirstTime == TRUE)
		{
			fFirstTime = FALSE;
			PostMessage(hDlg, WM_USER + 1, 0, 0);
		}
		break;
	case WM_USER + 1:
		SetDlgItemText(hDlg, IDC_STATUS, "Initialising module");
		SendDlgItemMessage(hDlg, IDC_BROADCOM_PROGRESS, PBM_SETRANGE32, 0, 30);
		SendDlgItemMessage(hDlg, IDC_BROADCOM_PROGRESS, PBM_SETSTEP, 1, 0);
		SetupDNIF(hDlg);
		SetupDNIF(hDlg);
		SetDlgItemText(hDlg, IDC_STATUS, "Loading firmware");
		if (WriteBroadcomMemory(hDlg) == FALSE)
		{
			EndDialog(hDlg, FALSE);
			break;
		}	
		EndDialog(hDlg, TRUE);
		break;
	}
	return FALSE;
}

BOOL Tune(int nLBandFrequency, int nSymbolRate, int nModulationMode, int nCodeRate, int nInput, int nTone, int nPolarity)
{
	BOOL StatusSuccess = FALSE;
	int nBytesReceived;
	DWORD dwLockTime;
	DWORD dwLockTimeout = 3000;
	DWORD dwFrequency, dwSymbolRate;
	BYTE bOut[11];
	BYTE bIn[1];

	// Reload the demod if we were previously setup for a DCII OQPSK signal
	if (LastTuneDCIIOQPSK() == TRUE && nModulationMode != 7)
		DialogBox((HINSTANCE)hInstance, MAKEINTRESOURCE(IDD_LOAD_BROADCOM), NULL, LoadBroadcomDlgProc);

	// Turn on power but no 22KHz yet
	SetTuner22KHzAndPower(FALSE, TRUE, nPolarity);

	switch(nInput)
	{
	case 0:
		break;
	case 1:
	case 2:
	case 3:
	case 4:
		Sleep(750);		// allow time for switch to reset
		SelectDiSEqCInput(nInput);
		Sleep(100);
		break;
	case 5:	// Tone A
	case 6:	// Tone B
		{
			BYTE bOut[2];

			Sleep(750);
			bOut[0] = outToneBurst;
			bOut[1] = nInput == 6;
			SendUSB(bOut, sizeof(bOut));
			Sleep(50);
		}
		break;
	}

	// Now turn on 22KHz as appropriate
	SetTuner22KHzAndPower(nTone, TRUE, nPolarity);

	// Calculate freq in hz and SR in symbols
	dwFrequency = nLBandFrequency * 1000000;
	dwSymbolRate = nSymbolRate * 1000;

	// Send the tune command
	bOut[0] = outTune;
	bOut[1] = (BYTE)(dwFrequency >> 24);
	bOut[2] = (BYTE)(dwFrequency >> 16);
	bOut[3] = (BYTE)(dwFrequency >> 8);
	bOut[4] = (BYTE)(dwFrequency & 0xff);
	bOut[5] = (BYTE)(dwSymbolRate >> 24);
	bOut[6] = (BYTE)(dwSymbolRate >> 16);
	bOut[7] = (BYTE)(dwSymbolRate >> 8);
	bOut[8] = (BYTE)(dwSymbolRate & 0xff);
	bOut[9] = nModulationMode;
	bOut[10] = nCodeRate;
	SendUSB(bOut, sizeof(bOut));

	// Get status back 
	nBytesReceived = ReceiveUSB(bIn, sizeof(bIn));
	if (nBytesReceived != sizeof(bIn))
		return FALSE;
	if (bIn[0] != 0)
		return FALSE;

	dwLockTime = GetTickCount();	
	while (!StatusSuccess)
	{
		BYTE bOut[1];
		BYTE bIn[3];

		DWORD dwCountNow = GetTickCount() - dwLockTime;
		if (dwCountNow > dwLockTimeout)
		{
			OutputDebugString("?Tune Timeout\n");
			break;
		}

		bOut[0] = outLockStatus;
		SendUSB(bOut, sizeof(bOut));
		ReceiveUSB(bIn, sizeof(bIn));
		StatusSuccess = bIn[0];
		if (!StatusSuccess)
			Sleep(1);
	}
	if (!StatusSuccess)
		SetTuner22KHzAndPower(FALSE, FALSE, 0);

	return StatusSuccess;
}

void EnableDMA(BOOL fEnable)
{
	BYTE bOut[2];

	bOut[0] = outDMAControl;
	bOut[1] = fEnable;
	if (!SendUSB(bOut, sizeof(bOut)))
		OutputDebugString("?EnableDMA failed\n");
}

DWORD WINAPI ReadMPEGData(LPVOID lpv)
{
	// We have 32 buffers of 128K each for incoming data. That's 4MB or about 1 second's worth of a 30Mbit stream
	int nCurrentBuffer;
	BOOL fFirstPacket = TRUE;
	LONG nTransferLength = READ_SIZE;
	
	fReadMPEGTheadRunning = TRUE;
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
	EnableDMA(TRUE);
	for (nCurrentBuffer = 0; nCurrentBuffer < nQueueSize; nCurrentBuffer++)
	   contexts[nCurrentBuffer] = MPEGInEpt->BeginDataXfer(buffers[nCurrentBuffer], nTransferLength, &inMPEGOvLap[nCurrentBuffer]);

	// Now loop as the MPEG data comes in
	nCurrentBuffer = 0;
	while (!fAbortReadMPEGThread)
	{
		LONG nReceiveLength = 0;
		
		// Wait for the current buffer to fill from the USB driver
		if (!MPEGInEpt->WaitForXfer(&inMPEGOvLap[nCurrentBuffer], 5000))
		{
			char szTemp[128];
			wsprintf(szTemp, "WaitForXfer() timed out buffer = %d ***********\n", nCurrentBuffer);
			OutputDebugString(szTemp);
			break;
		}

		// Now get the data from the just completed buffer
		if (MPEGInEpt->FinishDataXfer(buffers[nCurrentBuffer], nReceiveLength, &inMPEGOvLap[nCurrentBuffer],
			contexts[nCurrentBuffer]))
		{
			// We now have data back from the USB interface. This is where you'd do your processing but since
			// this is a simple app we just write it to a file. Note the first packet is thrown away - might be
			// stale data and cause a burst of continuity errors right at the start.
			// Also note that the data comes in as it's sent out of the interface - there's no alignment to
			// MPEG-2 packets and the packets might be followed by 16-byte Reed-Solomon code depending on the tuner.
			// So the first thing to do in your code is to sync up to the MPEG-2 stream.
			DWORD dwWritten;
			if (fFirstPacket == TRUE)
				fFirstPacket = FALSE;
			else
			{
				WriteFile(hOutputFile, buffers[nCurrentBuffer], nReceiveLength, &dwWritten, NULL);
				dBytesReceived += (double)dwWritten;
			}
		}
		else
			Sleep(1);

		// Now this buffer can be re-queued
	    contexts[nCurrentBuffer] = MPEGInEpt->BeginDataXfer(buffers[nCurrentBuffer], nTransferLength, &inMPEGOvLap[nCurrentBuffer]);
		nCurrentBuffer++;
		if (nCurrentBuffer == nQueueSize)
			nCurrentBuffer = 0;
	}

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

	EnableDMA(FALSE);
	fAbortReadMPEGThread = FALSE;
	fReadMPEGTheadRunning = FALSE;
	return 0;
}

BOOL CALLBACK MainDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg)
	{
	case WM_INITDIALOG:
		{
			int nEndPointCount, nIndex;
			outOvLap.hEvent = CreateEvent(NULL, false, false, "CYUSB_OUT"); 
			inOvLap.hEvent = CreateEvent(NULL, false, false, "CYUSB_IN"); 

			if (OpenUSBDriver() == FALSE)
			{
				MessageBox(hDlg, "Unable to open USB driver", gszAppName, MB_ICONSTOP);
				EndDialog(hDlg, FALSE);
				break;
			}

			// Find the USB pipe we use for data transfer (endpoint IN 2)
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
				MessageBox(hDlg, "Unable to locate MPEG data input endpoint", gszAppName, MB_ICONSTOP);
				EndDialog(hDlg, FALSE);
				break;
			}

			if (CheckUSB20() == FALSE)
			{
				MessageBox(hDlg, "Interface is connected to a non-USB 2.0 port", gszAppName, MB_ICONSTOP);
				EndDialog(hDlg, FALSE);
				break;
			}

			// Setup come default values. This is for 12.297 GHz LHCP on Dish Network at 119w. This is the mux with
			// NASA TV unscrambled
			SetDlgItemInt(hDlg, IDC_FREQUENCY, 12297 - 11250, FALSE);
			SetDlgItemInt(hDlg, IDC_SYMBOL_RATE, 20000, FALSE);
			SetDlgItemInt(hDlg, IDC_MODULATION_MODE, 0, FALSE);		// DVB-S QPSK
			SetDlgItemInt(hDlg, IDC_CODE_RATE, 5, FALSE);			// Auto code rate
			SetDlgItemInt(hDlg, IDC_SWITCH_INPUT, 0, FALSE);
			SetDlgItemInt(hDlg, IDC_22KHZ_TONE, 0, FALSE);
			SetDlgItemInt(hDlg, IDC_POLARITY, 1, FALSE);			// LHCP
			SetDlgItemText(hDlg, IDC_FILENAME, "test.ts");
		}
		break;
	case WM_DESTROY:
		CloseHandle(outOvLap.hEvent);
		CloseHandle(inOvLap.hEvent);
		break;
	case WM_COMMAND:
		switch(LOWORD(wParam))
		{
		case IDC_TUNE:
			{
				int nLBandFrequency, nSymbolRate, nModulationMode, nCodeRate;
				int nInput, nTone, nPolarity;

				// Make sure the demod is running firmware - if not, we need to download
				if (BroadcomDemodRunning() == FALSE)
				{
					if (DialogBox((HINSTANCE)hInstance, MAKEINTRESOURCE(IDD_LOAD_BROADCOM), NULL, LoadBroadcomDlgProc) == FALSE)
					{
						MessageBox(hDlg, "Unable to open the firmware file or the firmware file is too small", gszAppName, MB_ICONSTOP);
						break;
					}
				}				

				// Get Parameters and try tuning
				nLBandFrequency = GetDlgItemInt(hDlg, IDC_FREQUENCY, NULL, FALSE);
				nSymbolRate = GetDlgItemInt(hDlg, IDC_SYMBOL_RATE, NULL, FALSE);
				nModulationMode = GetDlgItemInt(hDlg, IDC_MODULATION_MODE, NULL, FALSE);
				nCodeRate = GetDlgItemInt(hDlg, IDC_CODE_RATE, NULL, FALSE);
				nInput = GetDlgItemInt(hDlg, IDC_SWITCH_INPUT, NULL, FALSE);
				nTone = GetDlgItemInt(hDlg, IDC_22KHZ_TONE, NULL, FALSE);
				nPolarity = GetDlgItemInt(hDlg, IDC_POLARITY, NULL, FALSE);

				SetDlgItemText(hDlg, IDC_LOCK_STATUS, "Tuning");
				if (Tune(nLBandFrequency, nSymbolRate, nModulationMode, nCodeRate, nInput, nTone, nPolarity) == TRUE)
					SetDlgItemText(hDlg, IDC_LOCK_STATUS, "Locked");
				else
					SetDlgItemText(hDlg, IDC_LOCK_STATUS, "No Lock");
				MessageBeep(0);
			}
			break;
		case IDC_TRANSFER:
			if (hOutputFile == INVALID_HANDLE_VALUE)
			{
				char szFilename[MAX_PATH];
				
				GetDlgItemText(hDlg, IDC_FILENAME, szFilename, sizeof(szFilename));
				hOutputFile = CreateFile(szFilename, GENERIC_WRITE, 0, (LPSECURITY_ATTRIBUTES) NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, (HANDLE) NULL);
				if (hOutputFile == INVALID_HANDLE_VALUE)
					MessageBox(hDlg, "Unable to open output file", gszAppName, MB_ICONSTOP);
				else
				{
					HANDLE hThread;
					DWORD dwThreadID;

					dBytesReceived = 0.0;
					hThread = CreateThread(NULL, 0, ReadMPEGData, (LPVOID)0, CREATE_SUSPENDED, &dwThreadID);
					SetThreadPriority(hThread, THREAD_PRIORITY_ABOVE_NORMAL);
					ResumeThread(hThread);
					CloseHandle(hThread);
					SetDlgItemText(hDlg, IDC_TRANSFER, "Stop");
					EnableWindow(GetDlgItem(hDlg, IDC_TUNE), FALSE);
					SetTimer(hDlg, 1, 100, NULL);
				}
			}
			else
			{
				fAbortReadMPEGThread = TRUE;
				while (fReadMPEGTheadRunning == TRUE)
					Sleep(20);
				CloseHandle(hOutputFile);
				hOutputFile = INVALID_HANDLE_VALUE;
				SetDlgItemText(hDlg, IDC_TRANSFER, "Start");
				EnableWindow(GetDlgItem(hDlg, IDC_TUNE), TRUE);
				KillTimer(hDlg, 1);
			}
			break;
		case IDCANCEL:
			PostMessage(hDlg, WM_CLOSE, 0, 0);
			break;
		}
		break;
	case WM_CLOSE:
		if (fReadMPEGTheadRunning == TRUE)
			SendMessage(hDlg, WM_COMMAND, IDC_TRANSFER, 0);
		EndDialog(hDlg, TRUE);
		break;
	case WM_TIMER:
		{
			char szTemp[32];

			if (dBytesReceived / 1024.0 / 1024.0 > 1000)
				sprintf(szTemp, "%.3f GB", dBytesReceived / 1024.0 / 1024.0 / 1024.0);
			else			
				sprintf(szTemp, "%.3f MB", dBytesReceived / 1024.0 / 1024.0);
			SetDlgItemText(hDlg, IDC_DATA_RECEIVED, szTemp);
		}
		break;
	}

	return FALSE;
}

int WINAPI WinMain(HINSTANCE hInputInstance, HINSTANCE hPrevInstance, LPSTR lpszCmdLine, int nCmdShow)
{
	hInstance = hInputInstance;
	InitCommonControls();
	DialogBox(hInstance, MAKEINTRESOURCE(IDD_MAIN_DLG), NULL, MainDlgProc);
	return 0;
}
