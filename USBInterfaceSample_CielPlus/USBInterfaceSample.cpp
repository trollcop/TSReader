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

	bOut[0] = 8;
	if (!SendUSB(bOut, sizeof(bOut)))
		OutputDebugString("?Write for GetConfiguration failed\n");
	else
		ReceiveUSB(bIn, sizeof(bIn));
	return ((bIn[1] & 1) == 1);
}

void EnableDMA(BOOL fEnable)
{
	BYTE bOut[2];

	bOut[0] = 7;
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
			CheckDlgButton(hDlg, IDC_MODE_ASYNC, TRUE);
		}
		break;
	case WM_DESTROY:
		CloseHandle(outOvLap.hEvent);
		CloseHandle(inOvLap.hEvent);
		break;
	case WM_COMMAND:
		switch(LOWORD(wParam))
		{
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
