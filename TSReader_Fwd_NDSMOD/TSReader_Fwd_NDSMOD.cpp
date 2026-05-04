#include <windows.h>
#include <commctrl.h>
#include "initguid.h"
#include "winioctl.h"
#include "setupapi.h"	// VC++ 5 one is out of date
#include <stdio.h>

#include <CyAPI.h>

#include "..\forwarder.h"

CCyUSBDevice * USBDevice = NULL;
CCyBulkEndPoint * MPEGOutEpt = NULL;   
OVERLAPPED outOvLap;
OVERLAPPED inOvLap; 
BOOL fDataThreadRunning;
BOOL fAbortDataThread;
int gnPacketLength;
int nBufferReadPtr, nBufferWritePtr, nBytesInBuffer;
BYTE * pPacketBuffer;
CRITICAL_SECTION csBuffer;

static char gszModuleName[] = {"NDS QPSK Modulator Interface"};

// There's a cockup on the PCB. D0 and D1 are wired around the wrong way. This translation table
// swaps D0 and D1 so we don't have any wires on the board.
static BYTE bTranslate[256] = {
	0x00, 0x02, 0x01, 0x03, 0x04, 0x06, 0x05, 0x07, 0x08, 0x0a, 0x09, 0x0b, 0x0c, 0x0e, 0x0d, 0x0f, 
	0x10, 0x12, 0x11, 0x13, 0x14, 0x16, 0x15, 0x17, 0x18, 0x1a, 0x19, 0x1b, 0x1c, 0x1e, 0x1d, 0x1f, 
	0x20, 0x22, 0x21, 0x23, 0x24, 0x26, 0x25, 0x27, 0x28, 0x2a, 0x29, 0x2b, 0x2c, 0x2e, 0x2d, 0x2f, 
	0x30, 0x32, 0x31, 0x33, 0x34, 0x36, 0x35, 0x37, 0x38, 0x3a, 0x39, 0x3b, 0x3c, 0x3e, 0x3d, 0x3f, 
	0x40, 0x42, 0x41, 0x43, 0x44, 0x46, 0x45, 0x47, 0x48, 0x4a, 0x49, 0x4b, 0x4c, 0x4e, 0x4d, 0x4f, 
	0x50, 0x52, 0x51, 0x53, 0x54, 0x56, 0x55, 0x57, 0x58, 0x5a, 0x59, 0x5b, 0x5c, 0x5e, 0x5d, 0x5f, 
	0x60, 0x62, 0x61, 0x63, 0x64, 0x66, 0x65, 0x67, 0x68, 0x6a, 0x69, 0x6b, 0x6c, 0x6e, 0x6d, 0x6f, 
	0x70, 0x72, 0x71, 0x73, 0x74, 0x76, 0x75, 0x77, 0x78, 0x7a, 0x79, 0x7b, 0x7c, 0x7e, 0x7d, 0x7f, 
	0x80, 0x82, 0x81, 0x83, 0x84, 0x86, 0x85, 0x87, 0x88, 0x8a, 0x89, 0x8b, 0x8c, 0x8e, 0x8d, 0x8f, 
	0x90, 0x92, 0x91, 0x93, 0x94, 0x96, 0x95, 0x97, 0x98, 0x9a, 0x99, 0x9b, 0x9c, 0x9e, 0x9d, 0x9f, 
	0xa0, 0xa2, 0xa1, 0xa3, 0xa4, 0xa6, 0xa5, 0xa7, 0xa8, 0xaa, 0xa9, 0xab, 0xac, 0xae, 0xad, 0xaf, 
	0xb0, 0xb2, 0xb1, 0xb3, 0xb4, 0xb6, 0xb5, 0xb7, 0xb8, 0xba, 0xb9, 0xbb, 0xbc, 0xbe, 0xbd, 0xbf, 
	0xc0, 0xc2, 0xc1, 0xc3, 0xc4, 0xc6, 0xc5, 0xc7, 0xc8, 0xca, 0xc9, 0xcb, 0xcc, 0xce, 0xcd, 0xcf, 
	0xd0, 0xd2, 0xd1, 0xd3, 0xd4, 0xd6, 0xd5, 0xd7, 0xd8, 0xda, 0xd9, 0xdb, 0xdc, 0xde, 0xdd, 0xdf, 
	0xe0, 0xe2, 0xe1, 0xe3, 0xe4, 0xe6, 0xe5, 0xe7, 0xe8, 0xea, 0xe9, 0xeb, 0xec, 0xee, 0xed, 0xef, 
	0xf0, 0xf2, 0xf1, 0xf3, 0xf4, 0xf6, 0xf5, 0xf7, 0xf8, 0xfa, 0xf9, 0xfb, 0xfc, 0xfe, 0xfd, 0xff
};

#define outDMAControl 7
#define outGetConfiguration 8
#define MPEG2_PACKETS 128
#define WRITE_TO_FX2_SIZE 204 * MPEG2_PACKETS
#define PACKET_BUFFER_LENGTH 3 * 1024 * 1024

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

void EnableDMA(BOOL fEnable)
{
	BYTE bOut[2];

	bOut[0] = outDMAControl;
	bOut[1] = fEnable;

	if (!SendUSB(bOut, sizeof(bOut)))
		OutputDebugString("EnableDMA failed\n");
}

BOOL OpenUSBDriver(HWND hWnd)
{
	int d = 0;
	GUID DTVWorksGUID;
	char szFoundDevices[512] = {0};

	DTVWorksGUID.Data1 = 0xFA58C45D;
	DTVWorksGUID.Data2 = 0x5B19; DTVWorksGUID.Data3 = 0x428b;
	DTVWorksGUID.Data4[0] = 0xA2; DTVWorksGUID.Data4[1] = 0xD1; DTVWorksGUID.Data4[2] = 0x27; DTVWorksGUID.Data4[3] = 0x07;
	DTVWorksGUID.Data4[4] = 0x85; DTVWorksGUID.Data4[5] = 0x6D; DTVWorksGUID.Data4[6] = 0x7E; DTVWorksGUID.Data4[7] = 0x19;
	
	USBDevice = new CCyUSBDevice(NULL, DTVWorksGUID);
    int nDevices = USBDevice->DeviceCount(); 
	if (nDevices == 0)
	{
		MessageBox(hWnd, "Unable to locate the MPEG-2 Output interface. Is it connected?", gszModuleName, MB_ICONSTOP);
		return FALSE;
	}

    do
	{
		BYTE bOut[1];
		BYTE bIn[2] = {0, 0};

		USBDevice->Open(d);   // Open automatically  calls Close() if necessary 

		int vID = USBDevice->VendorID; 
        int pID  = USBDevice->ProductID;

		if (USBDevice->BulkOutEndPt != NULL)
		{
			bOut[0] = outGetConfiguration;
			if (!SendUSB(bOut, sizeof(bOut)))
				OutputDebugString("Write for GetConfiguration failed\n");
			ReceiveUSB(bIn, sizeof(bIn));
			if (bIn[0] == 0x05)		// an output device
				break;
		}

		d++;
    } while (d < nDevices);

	if (d == nDevices)
	{
		MessageBox(hWnd, "Unable to locate the MPEG-2 Output interface. Is it connected?", gszModuleName, MB_ICONSTOP);
		return FALSE;
	}

	if (USBDevice->EndPoints == NULL)
	{
		MessageBox(hWnd, "NULL endpoint from the USB device", gszModuleName, MB_ICONSTOP);
		return FALSE;
	}

	return TRUE;
}

BOOL TSReader_Fwd_Init(HWND hWnd, int nPacketLength, int nBitRate)
{
	int nEndPointCount;
	int nIndex;
	BYTE bOut[1];
	BYTE bIn[2];

	gnPacketLength = nPacketLength;

	outOvLap.hEvent = CreateEvent(NULL, false, false, "CYUSB_OUT"); 
	inOvLap.hEvent = CreateEvent(NULL, false, false, "CYUSB_IN"); 
	if (OpenUSBDriver(hWnd) == FALSE)
		return FALSE;

	// Find the USB pipe we use for data transfer (endpoint OUT 2)
	nEndPointCount = USBDevice->EndPointCount();  
	for (nIndex = 1; nIndex < nEndPointCount; nIndex++)
	{
		if (USBDevice->EndPoints[nIndex]->Address == 0x02)
		{
			MPEGOutEpt = (CCyBulkEndPoint *)USBDevice->EndPoints[nIndex];
			break;
		}
	}
	if (nIndex == nEndPointCount)
	{
		OutputDebugString("Couldn't find MPEG in endpoint\n");
		return FALSE;
	}

	// Make sure we have a USB 2.0 connection
	bOut[0] = outGetConfiguration;
	if (!SendUSB(bOut, sizeof(bOut)))
		OutputDebugString("Write for GetConfiguarion failed\n");
	ReceiveUSB(bIn, sizeof(bIn));
	if ((bIn[1] & 1) == 0)
	{
		MessageBox(hWnd, "Interface is not connected to a USB 2.0 port", gszModuleName, MB_ICONSTOP);
		return FALSE;
	}
	fDataThreadRunning = FALSE;
	fAbortDataThread = FALSE;
	nBufferReadPtr = nBufferWritePtr = nBytesInBuffer = 0;
	InitializeCriticalSection(&csBuffer);
	pPacketBuffer = (BYTE *)LocalAlloc(LPTR, PACKET_BUFFER_LENGTH);
	return TRUE;
}

BOOL TSReader_Fwd_DeInit()
{
	fAbortDataThread = TRUE;
	while (fDataThreadRunning)
		Sleep(50);

	DeleteCriticalSection(&csBuffer);
	LocalFree(pPacketBuffer);
	delete USBDevice;
	return TRUE;
}

int ReadPackets(BYTE * pOutputData, int nMaxLen)
{
	BOOL fAbort = FALSE;
	BOOL fOKToContinue = FALSE;
	int nBytesWritten = 0;

	do
	{
		fOKToContinue = FALSE;

		EnterCriticalSection(&csBuffer);
		if (nBytesInBuffer >= gnPacketLength)
			fOKToContinue = TRUE;
		else if (fAbortDataThread)
		{
			fOKToContinue = TRUE;
			fAbort = TRUE;
		}
		LeaveCriticalSection(&csBuffer);
		if (fAbort)
			return 0;			
		if (!fOKToContinue)
		{
			Sleep(5);		
		}
		else
		{
			int i;

			*(pOutputData++) = bTranslate[pPacketBuffer[nBufferReadPtr++]]; nBytesWritten++; nMaxLen--;
			*(pOutputData++) = 0xff; nBytesWritten++; nMaxLen--;		// start of packet bit on
			for (i = 1; i < gnPacketLength; i++)
			{			
				*(pOutputData++) = bTranslate[pPacketBuffer[nBufferReadPtr++]]; nBytesWritten++; nMaxLen--;
				*(pOutputData++) = 0x00; nBytesWritten++; nMaxLen--;	// packet start off
			}
			if (nBufferReadPtr + gnPacketLength >= PACKET_BUFFER_LENGTH)
				nBufferReadPtr = 0;
			EnterCriticalSection(&csBuffer);
			nBytesInBuffer -= gnPacketLength;
			LeaveCriticalSection(&csBuffer);
			if (nMaxLen < gnPacketLength * 2)
				break;
			fOKToContinue = FALSE;
		}
	} while (fOKToContinue == FALSE);

	return nBytesWritten;
}

DWORD WINAPI SendFX2Thread(LPVOID lpv)
{
	int nCurrentBuffer;
	int j = 0;
	BOOL fOKToQuit = FALSE;

	OutputDebugString("+SendFX2Thread\n");
	MPEGOutEpt->SetXferSize(WRITE_TO_FX2_SIZE);

	// Setup the asynchronous transfer buffers
	int nQueueSize = 32;
	PUCHAR *buffers = new PUCHAR[nQueueSize];
	PUCHAR *contexts = new PUCHAR[nQueueSize];
	OVERLAPPED * outMPEGOvLap = new OVERLAPPED[nQueueSize];
	int *nInputLengths = new int[nQueueSize];

	for (nCurrentBuffer = 0; nCurrentBuffer < nQueueSize; nCurrentBuffer++)
	{
		buffers[nCurrentBuffer] = new UCHAR[WRITE_TO_FX2_SIZE];
		outMPEGOvLap[nCurrentBuffer].Internal = outMPEGOvLap[nCurrentBuffer].InternalHigh = 0;
		outMPEGOvLap[nCurrentBuffer].Offset = outMPEGOvLap[nCurrentBuffer].OffsetHigh = 0;
		outMPEGOvLap[nCurrentBuffer].hEvent = CreateEvent(NULL, false, false, NULL);		
		nInputLengths[nCurrentBuffer] = ReadPackets(buffers[nCurrentBuffer], WRITE_TO_FX2_SIZE);
	}

	// Queue-up the first batch of transfer requests
	EnableDMA(TRUE);
	for (nCurrentBuffer = 0; nCurrentBuffer < nQueueSize; nCurrentBuffer++)
		contexts[nCurrentBuffer] = MPEGOutEpt->BeginDataXfer(buffers[nCurrentBuffer], nInputLengths[nCurrentBuffer], &outMPEGOvLap[nCurrentBuffer]);

	nCurrentBuffer = 0;
	while (!fAbortDataThread)
	{
		LONG nTransmitLength = 0;
		
		if (!MPEGOutEpt->WaitForXfer(&outMPEGOvLap[nCurrentBuffer], 2500))
		{
			char szTemp[128];
			wsprintf(szTemp, "WaitForXfer() timed out buffer = %d ***********\n", nCurrentBuffer);
			OutputDebugString(szTemp);
			break;
		}

		if (MPEGOutEpt->FinishDataXfer(buffers[nCurrentBuffer], nTransmitLength, &outMPEGOvLap[nCurrentBuffer],
			contexts[nCurrentBuffer]))
		{
			// transfer completed OK
		}
		else
			OutputDebugString("%");

		nInputLengths[nCurrentBuffer] = ReadPackets(buffers[nCurrentBuffer], WRITE_TO_FX2_SIZE);
	    contexts[nCurrentBuffer] = MPEGOutEpt->BeginDataXfer(buffers[nCurrentBuffer], nInputLengths[nCurrentBuffer], &outMPEGOvLap[nCurrentBuffer]);
		nCurrentBuffer++;
		if (nCurrentBuffer == nQueueSize)
			nCurrentBuffer = 0;
	}

	MPEGOutEpt->Abort();
	for (nCurrentBuffer = 0; nCurrentBuffer < nQueueSize; nCurrentBuffer++)
	{
		// Wait for all the queued requests to be cancelled 
		LONG nReceiveLength = 0;
		MPEGOutEpt->FinishDataXfer(buffers[nCurrentBuffer], nReceiveLength, &outMPEGOvLap[nCurrentBuffer], contexts[nCurrentBuffer]);
		CloseHandle(outMPEGOvLap[nCurrentBuffer].hEvent);
		delete [] buffers[nCurrentBuffer];
	}

	delete [] buffers;
	delete [] contexts;
	delete [] outMPEGOvLap;
	delete [] nInputLengths;

	EnableDMA(FALSE);

	fDataThreadRunning = FALSE;
	OutputDebugString("-SendFX2Thread\n");
	return 0;
}

BOOL TSReader_Fwd_Data(BYTE * pBuffer, int nLength)
{
	while (nLength)
	{
		if (nBufferWritePtr + gnPacketLength > PACKET_BUFFER_LENGTH)
			nBufferWritePtr = 0;
		memcpy(&pPacketBuffer[nBufferWritePtr], pBuffer, gnPacketLength);
		pBuffer += gnPacketLength;
		nLength -= gnPacketLength;
		nBufferWritePtr += gnPacketLength;
		EnterCriticalSection(&csBuffer);
		nBytesInBuffer += gnPacketLength;
		LeaveCriticalSection(&csBuffer);
	}
	if (!fDataThreadRunning)
	{
		HANDLE hThread;
		DWORD dwThreadID;

		hThread = CreateThread(NULL, 0, SendFX2Thread, (LPVOID)0, CREATE_SUSPENDED, &dwThreadID);
		//SetThreadPriority(hThread, THREAD_PRIORITY_TIME_CRITICAL);
		ResumeThread(hThread);
		CloseHandle(hThread);
		fDataThreadRunning = TRUE;
	}
	return TRUE;
}

BOOL TSReader_Fwd_GetDescription(char * szDeviceNameBuffer)
{
	lstrcpy(szDeviceNameBuffer, gszModuleName);
	return TRUE;
}

BOOL APIENTRY DllMain(HANDLE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
    switch( ul_reason_for_call )
	{
    case DLL_PROCESS_ATTACH:
		break;
    case DLL_PROCESS_DETACH:
		break;
    }
    return TRUE;
}

