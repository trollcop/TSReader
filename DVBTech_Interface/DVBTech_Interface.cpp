#include <windows.h>
#include <commctrl.h>
#include <setupapi.h>
#include <initguid.h>
#include <stdio.h>
#include <math.h>

#include "CyAPI.h"

//#include "resource.h"

//EP0 commands
#define outDNHardware 1
#define outWriteBCM4500RAM 6
#define outDMAControl 7
#define outGetConfiguration 8
#define outWriteIntersil 9
#define outReadIntersil 10
#define out22KHzOnOff 11
#define outDiSEqC 14
#define outToneBurst 13
#define outTune 16
#define outLockStatus 17
#define outDishSwitch 21

//ST register definitions
#define PCL		0x80
#define ISEL1	0x00
#define TEN		0x20
#define LLC		0x10
#define VSEL1	0x08
#define EN1		0x04
#define OTF		0x02
#define OLF		0x01

// Other hardware stuff
#define TARGET_PRODUCT_ID 0x0100

// Modulation Modes
#define ADV_MOD_DVB_QPSK 0						// DVB-S QPSK
#define ADV_MOD_TURBO_QPSK 1					// Turbo QPSK
#define ADV_MOD_TURBO_8PSK 2					// Turbo 8PSK (also used for Trellis 8PSK DVB-SNG)
#define ADV_MOD_TURBO_16QAM 3					// Turbo 16QAM (also used for Trellis 16QAM DVB-SNG)
#define ADV_MOD_DCII_C_QPSK 4					// Digicipher II Combo
#define ADV_MOD_DCII_I_QPSK 5					// Digicipher II I-stream
#define ADV_MOD_DCII_Q_QPSK 6					// Digicipher II Q-stream
#define ADV_MOD_DCII_C_OQPSK 7					// Digicipher II offset QPSK

// Storage
BOOL fPowerOn;
BOOL fReadThreadTerminated;
BOOL fTerminateReadThread;
BOOL fDoneDeInit;
BOOL fInitOK;
HANDLE hInstance;
HANDLE hReadDataThread;
CCyUSBDevice * USBDevice = NULL;
CCyBulkEndPoint * MPEGInEpt = NULL;   
OVERLAPPED outOvLap;
OVERLAPPED inOvLap; 

// Ring/Sync Buffer
#define MAX_TS_BUFFERS 30000
typedef struct _tagTSBuffer
{
	BYTE bByteCount;
	BYTE bBuffer[188];
} TSBUFFER, *PTSBUFFER;
PTSBUFFER pTS;
int nTSBufferRead, nTSBufferWrite;
CRITICAL_SECTION csTSBuffer;
BOOL fSynced;

void SyncData(BYTE * pBuffer, int nLength)
{
ReSync_Loop:
	if (fSynced == FALSE)
	{
		int nOffset = 0;

		while (nOffset < nLength - 188 * 3)
		{
			if (*(pBuffer + nOffset) == 0x47)
			{
				if (*(pBuffer + nOffset + 188) == 0x47)
				{
					if (*(pBuffer + nOffset + 188 + 188) == 0x47)
					{
						fSynced = TRUE;
						nLength -= nOffset;
						pBuffer += nOffset;
						break;
					}
				}
			}
			nOffset++;
		}
		// Still not synced
	}
	
	if (fSynced == TRUE)
	{
		// pBuffer -> aligned data, nLength has length
		while (nLength >= 188)
		{
			int nWriteLength;

			if (*pBuffer != 0x47 && pTS[nTSBufferWrite].bByteCount == 0)
			{
				fSynced = FALSE;
				goto ReSync_Loop;
			}			
			nWriteLength = 188 - pTS[nTSBufferWrite].bByteCount;
			memcpy(pTS[nTSBufferWrite].bBuffer + pTS[nTSBufferWrite].bByteCount, pBuffer, nWriteLength);
			pTS[nTSBufferWrite].bByteCount += nWriteLength;
			nLength -= nWriteLength;
			pBuffer += nWriteLength;
			if (pTS[nTSBufferWrite].bByteCount == 188)
			{
				EnterCriticalSection(&csTSBuffer);
				nTSBufferWrite++;
				if (nTSBufferWrite == MAX_TS_BUFFERS)
					nTSBufferWrite = 0;
				LeaveCriticalSection(&csTSBuffer);
			}
		}
		if (nLength)
		{
			// Some residial - pick up the rest from the next inbound buffer
			if (*pBuffer != 0x47)
			{
				fSynced = FALSE;
				pTS[nTSBufferWrite].bByteCount = 0;
			}
			else
			{
				memcpy(pTS[nTSBufferWrite].bBuffer, pBuffer, nLength);
				pTS[nTSBufferWrite].bByteCount = nLength;
			}
		}
	}
}

int ReadTransportData(BYTE * pBuffer, int nMaxLength, int nTimeoutMS)
{
	int nTotalWritten = 0;
	DWORD dwTimeout = GetTickCount() + nTimeoutMS;

	if (!fInitOK)
		return 0;
	
	while (nMaxLength)
	{
		int nTSBufferWriteCopy;

		// See if we timed out
		if (GetTickCount() > dwTimeout)
			break;

		// See if there's a buffer for us
		EnterCriticalSection(&csTSBuffer);
		nTSBufferWriteCopy = nTSBufferWrite;
		LeaveCriticalSection(&csTSBuffer);
		if (nTSBufferWriteCopy != nTSBufferRead)
		{
			if (pTS[nTSBufferRead].bByteCount == 188)
			{
				memcpy(pBuffer, pTS[nTSBufferRead].bBuffer, 188);
				pTS[nTSBufferRead].bByteCount = 0;
				pBuffer += 188;
				nMaxLength  -= 188;
				nTotalWritten += 188;
				nTSBufferRead++;
				if (nTSBufferRead == MAX_TS_BUFFERS)
					nTSBufferRead = 0;
			}
		}
		else
		{
			// No data available - snooze
			Sleep(10);
		}
	}

	return nTotalWritten;
}

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

	UCHAR  * inContext = USBDevice->BulkInEndPt->BeginDataXfer(bIn, nSize, &inOvLap); 
	USBDevice->BulkInEndPt->WaitForXfer(&inOvLap, 1000); 
	USBDevice->BulkInEndPt->FinishDataXfer(bIn, nBytesReceived, &inOvLap,inContext);

	return nBytesReceived;
}

BYTE ReadIntersil()
{
	BYTE bOut[2];
	BYTE bIn[1];

	bOut[0] = outReadIntersil;
	bOut[1] = 0x10;

	SendUSB(bOut, sizeof(bOut));
	ReceiveUSB(bIn, sizeof(bIn));
	return bIn[0];
}

void WriteIntersil(BYTE bValue)
{
	BYTE bOut[3];

	bOut[0] = outWriteIntersil;
	bOut[1] = 0x10;
	bOut[2] = bValue;

	SendUSB(bOut, sizeof(bOut));
}

void Setup22KHz(BOOL f22KHz)
{
	BYTE bOut[2];

	bOut[0] = out22KHzOnOff;
	bOut[1] = f22KHz;

	SendUSB(bOut, sizeof(bOut));
}

void SetTuner22KHzAndPower(BOOL f22KHz, BOOL fPower, int nPolarity)
{
	if (fPower == FALSE)
	{
		Setup22KHz(FALSE);
		WriteIntersil(ISEL1);
	}
	else
	{
		switch(nPolarity)
		{
		case 0:	// vertical
			WriteIntersil(ISEL1 | EN1);
			Setup22KHz(f22KHz);
			break;
		case 1:	// horizontal
			WriteIntersil(ISEL1 | VSEL1 | EN1);
			Setup22KHz(f22KHz);
			break;
		default: // power off
			WriteIntersil(ISEL1);
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
		BYTE bOut[6];

		bOut[0] = outDiSEqC;
		bOut[1] = 4;
		bOut[2] = 0xe0;		// master to slave no response
		bOut[3] = 0x10;		// address
		bOut[4] = 0x38;		// switch port
		bOut[5] = bPositionByte[nInput];
		SendUSB(bOut, sizeof(bOut));

		// Stall while the firmware sends the DiSEqC command. 12.5 ms per byte
		// plus 15 ms after all bytes
		Sleep((bOut[1] * 13) + 15 + 10);
	}
}

BOOL LastTuneDCIIOQPSK()
{
	BYTE bOut[1];
	BYTE bIn[2];

	bOut[0] = outGetConfiguration;
	SendUSB(bOut, sizeof(bOut));
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
	SendUSB(bOut, sizeof(bOut));
	ReceiveUSB(bIn, sizeof(bIn));
	wRetVal = bIn[0] << 8 | bIn[1];
	return wRetVal;
}

void SetupDNIF()
{
	WriteAndReadDNIF(0x8000);

	WriteAndReadDNIF(0x0200);
	WriteAndReadDNIF(0x0200);
	WriteAndReadDNIF(0x0100);
	WriteAndReadDNIF(0x0100);
	WriteAndReadDNIF(0x0100);
	WriteAndReadDNIF(0x0100);
	WriteAndReadDNIF(0x0100);
	WriteAndReadDNIF(0x0100);
	Sleep(100);

	WriteAndReadDNIF(0x0201);	// 3V on after this
	WriteAndReadDNIF(0x0203); // CLK driven after this
	Sleep(100);

	WriteAndReadDNIF(0x0104);	// 
	Sleep(100);

	WriteAndReadDNIF(0x0100);	// I2C relay off
	WriteAndReadDNIF(0x0102);	// I2C relay on
	WriteAndReadDNIF(0x0103);	// I2C relay on and bus on
}

BOOL WriteBCM4500Memory()
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

			do
			{
				nThisTime = nRemaining;
				if (nThisTime > 60)
					nThisTime = 60;

				bOut[0] = outWriteBCM4500RAM;
				bOut[1] = nThisTime;	// count
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

BOOL LoadBCM4500Firmware()
{
	SetupDNIF();
	SetupDNIF();
	return WriteBCM4500Memory();
}

void SetupDiSEqC(int nDiSEqCInput)
{
	switch(nDiSEqCInput)
	{
	case 0:
		{
			int i;
			for (i = 0; i < 1; i++)
			{
				BYTE bOut[2];
				BYTE bIn[2];

				bOut[0] = outDishSwitch;
				bOut[1] = 0x65;
				SendUSB(bOut, sizeof(bOut));
				
				do
				{
					Sleep(5);
					bOut[0] = outGetConfiguration;
					SendUSB(bOut, sizeof(bOut));
					ReceiveUSB(bIn, sizeof(bIn));
				} while (bIn[1] & 2);
			}
		}
		break;
	case 1:
	case 2:
	case 3:
	case 4:
		Sleep(750);		// allow time for switch to reset
		SelectDiSEqCInput(nDiSEqCInput);
		Sleep(100);
		break;
	case 5:	// Tone A
	case 6:	// Tone B
		{
			BYTE bOut[2];

			Sleep(750);
			bOut[0] = outToneBurst;
			bOut[1] = nDiSEqCInput == 6;
			SendUSB(bOut, sizeof(bOut));
			Sleep(50);
		}
		break;
	default:
		{
			if (nDiSEqCInput >= 7 && nDiSEqCInput <= 20)
			{
				// Dish Network Switch
				BYTE bOut[2];
				static BYTE bDishBytes[] = {
					0x34, // SW21 Dish 1
					0x65, // SW21 Dish 2
					0x46, // SW42 Dish 1
					0x17, // SW42 Dish 2
					0x68, // SW44 Dish 2
					0x39, // SW64 Dish 1A
					0x1A, // SW64 Dish 1B
					0x4B, // SW64 Dish 2A
					0x5C, // SW64 Dish 2B
					0x0D, // SW64 Dish 3A
					0x2E, // SW64 Dish 3B
					0x72, // Twin LNB 1
					0x23, // Twin LNB 2
					0x51}; // Quad LNB 2

				Sleep(750);
				bOut[0] = outDishSwitch;
				bOut[1] = bDishBytes[nDiSEqCInput - 7];
				SendUSB(bOut, sizeof(bOut));
				Sleep(125);
			}
		}
		break;
	}
}

BOOL Tune(int nFrequency, int nSymbolRate, int nPolarity, int nModulationMode, int nCodeRate, 
		  int nLNBFrequency, BOOL f22KHz, int nDiSEqCInput, int nLockTimeout)
{
	BOOL StatusSuccess = FALSE;
	int nLBand;
	int nBytesReceived;
	DWORD dwLockTime;
	DWORD dwFrequency, dwSymbolRate;
	BYTE bOut[16];
	BYTE bIn[1];

	if (!fInitOK)
		return FALSE;

	if (LastTuneDCIIOQPSK() == TRUE && nModulationMode != ADV_MOD_DCII_C_OQPSK)
		LoadBCM4500Firmware();

	if (fPowerOn == FALSE)
	{
		// Turn on power but no 22KHz yet
		SetTuner22KHzAndPower(FALSE, TRUE, nPolarity);
		fPowerOn = TRUE;
		Sleep(50);
	}

	SetupDiSEqC(nDiSEqCInput);

	// Now turn on 22KHz as appropriate
	SetTuner22KHzAndPower(f22KHz, TRUE, nPolarity);

	// Calculate l-band, freq in hz and SR in symbols
	if (nFrequency > nLNBFrequency)
		nLBand = nFrequency - nLNBFrequency;
	else
		nLBand = nLNBFrequency - nFrequency;
	dwFrequency = nLBand * 1000000;
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
	SendUSB(bOut, 11);

	// Get status back 
	nBytesReceived = ReceiveUSB(bIn, sizeof(bIn));
	if (nBytesReceived != sizeof(bIn))
	{
		return FALSE;
	}
	if (bIn[0] != 0)
	{
		return FALSE;
	}

	dwLockTime = GetTickCount();	
	while (!StatusSuccess)
	{
		BYTE bOut[1];
		BYTE bIn[3];

		DWORD dwCountNow = GetTickCount() - dwLockTime;
		if (dwCountNow > (DWORD)nLockTimeout)
		{
			break;
		}

		bOut[0] = outLockStatus;
		SendUSB(bOut, sizeof(bOut));
		ReceiveUSB(bIn, sizeof(bIn));
		StatusSuccess = bIn[0];
		if (!StatusSuccess)
			Sleep(1);
	}

	return StatusSuccess;
}

void EnableDMA(BOOL fEnable)
{
	BYTE bOut[2];

	bOut[0] = outDMAControl;
	bOut[1] = fEnable;

	SendUSB(bOut, sizeof(bOut));
}

#define READ_FROM_FX2_SIZE 32 * 1024
DWORD WINAPI ReadFX2Thread(LPVOID lpv)
{
	int nReadPtr = 0;
	int nCurrentBuffer;
	BOOL fFirstPacket = TRUE;
	BOOL fRestart = TRUE;

	fReadThreadTerminated = FALSE;
	fTerminateReadThread = FALSE;

RestartFX2DataThread:
	fRestart = FALSE;
	LONG nTransferLength = READ_FROM_FX2_SIZE;

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

	nCurrentBuffer = 0;
	while (!fTerminateReadThread)
	{
		LONG nReceiveLength = 0;
		
		if (!MPEGInEpt->WaitForXfer(&inMPEGOvLap[nCurrentBuffer], 2500))
		{
			fRestart = TRUE;
			break;
		}

		if (MPEGInEpt->FinishDataXfer(buffers[nCurrentBuffer], nReceiveLength, &inMPEGOvLap[nCurrentBuffer],
			contexts[nCurrentBuffer]))
		{
			if (fFirstPacket == TRUE)
				fFirstPacket = FALSE;
			else
			{
				SyncData(buffers[nCurrentBuffer], nReceiveLength);
			}
		}
		else
			Sleep(1);

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
	
	if (fRestart == TRUE && !fTerminateReadThread)
		goto RestartFX2DataThread;

	fReadThreadTerminated = TRUE;
	fTerminateReadThread = FALSE;
	CloseHandle(hReadDataThread);
	return 0;
}

BOOL Start()
{
	DWORD dwThreadID;

	hReadDataThread = CreateThread(NULL, 0, ReadFX2Thread, (LPVOID)0, CREATE_SUSPENDED, &dwThreadID);
	SetThreadPriority(hReadDataThread, THREAD_PRIORITY_TIME_CRITICAL);
	ResumeThread(hReadDataThread);

	return TRUE;
}

BOOL Stop()
{
	fTerminateReadThread = TRUE;
	while (fReadThreadTerminated == FALSE)
		Sleep(50);

	return TRUE;
}

BOOL OpenUSBDriver()
{
	int d = 0;
	GUID OASISguid;

	OASISguid.Data1 = 0xFA58C45D;
	OASISguid.Data2 = 0x5B19; OASISguid.Data3 = 0x428b;
	OASISguid.Data4[0] = 0xA2; OASISguid.Data4[1] = 0xD1; OASISguid.Data4[2] = 0x27; OASISguid.Data4[3] = 0x07;
	OASISguid.Data4[4] = 0x85; OASISguid.Data4[5] = 0x6D; OASISguid.Data4[6] = 0x7E; OASISguid.Data4[7] = 0x19;
	
	USBDevice = new CCyUSBDevice(NULL, OASISguid);
    int nDevices = USBDevice->DeviceCount(); 
	if (nDevices == 0)
		return FALSE;

    do
	{
		BYTE bOut[1];
		BYTE bIn[2] = {0, 0};

		USBDevice->Open(d);   // Open automatically  calls Close() if necessary 

		int vID = USBDevice->VendorID; 
        int pID  = USBDevice->ProductID;

		if (vID != 0x14ac)	// must be a DTVWorks/COOLSTF product
			goto LoopForNextUSBDevice;

		if (USBDevice->BulkOutEndPt == NULL)
		{
			return FALSE;
		}

		bOut[0] = outGetConfiguration;
		SendUSB(bOut, sizeof(bOut));
		ReceiveUSB(bIn, sizeof(bIn));
		if (pID == TARGET_PRODUCT_ID)
			break;
		
LoopForNextUSBDevice:
		USBDevice->Close();
		d++;
    } while (d < nDevices);

	if (d == nDevices)
		return FALSE;

	if (USBDevice->EndPoints == NULL)
	{
		return FALSE;
	}

	return TRUE;
}

BOOL InitHardware()
{
	int nTimeout = 5;

	if (OpenUSBDriver() == FALSE)
		return FALSE;

	// Try sending a get status command to see if the hardware is alive
	do
	{
		BYTE bOut[1];
		BYTE bIn[2];

		bOut[0] = outGetConfiguration;
		SendUSB(bOut, sizeof(bOut));
		ReceiveUSB(bIn, sizeof(bIn));
		if ((bIn[1] & 4) == 4)
			break;	// board is ready to go
		if (LoadBCM4500Firmware() == FALSE)
			return FALSE;
		Sleep(100);
	} while (nTimeout--);

	return (nTimeout > 0) & 1;
}

void CloseEP1OvelapppedEvents()
{
	CloseHandle(outOvLap.hEvent); outOvLap.hEvent = NULL;
	CloseHandle(inOvLap.hEvent);  inOvLap.hEvent = NULL;
}

BOOL Init()
{
	int nIndex;
	int nEndPointCount;
	BYTE bOut[1];
	BYTE bIn[2];

	fDoneDeInit = FALSE;

	outOvLap.hEvent = CreateEvent(NULL, false, false, "CYUSB_OUT"); 
	inOvLap.hEvent = CreateEvent(NULL, false, false, "CYUSB_IN"); 

	if (InitHardware() == FALSE)
		return FALSE;

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
		CloseEP1OvelapppedEvents();
		return FALSE;
	}

	// Make sure we have a USB 2.0 connection
	bOut[0] = outGetConfiguration;
	SendUSB(bOut, sizeof(bOut));
	ReceiveUSB(bIn, sizeof(bIn));
	if ((bIn[1] & 1) == 0)
		return FALSE;

	// Setup sync/ring buffer
	pTS = (PTSBUFFER)LocalAlloc(LPTR, sizeof(TSBUFFER) * MAX_TS_BUFFERS);
	nTSBufferRead = nTSBufferWrite = 0;
	fSynced = FALSE;
	InitializeCriticalSection(&csTSBuffer);

	return TRUE;
}

BOOL DeInit()
{
	if (fDoneDeInit)
		return TRUE;

	CloseEP1OvelapppedEvents();
	if (USBDevice->EndPoints != NULL)
	{
		delete USBDevice; 
		USBDevice = NULL;
	}

	LocalFree(pTS);
	DeleteCriticalSection(&csTSBuffer);
	
	fDoneDeInit = TRUE;
	return TRUE;
}

BOOL GetSignal(BOOL * fLocked, double * dSNR)
{
	BYTE bOut[1];
	BYTE bIn[3];

	if (!fInitOK)
		return FALSE;

	bOut[0] = outLockStatus;
	SendUSB(bOut, sizeof(bOut));
	ReceiveUSB(bIn, sizeof(bIn));

	*dSNR = (double)(bIn[1] << 8 | bIn[2]) / 256.0;
	if (bIn[0])
		*fLocked = TRUE;
	else
		*fLocked = FALSE;

	return TRUE;
}

BOOL APIENTRY DllMain(HANDLE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
    switch( ul_reason_for_call )
	{
    case DLL_PROCESS_ATTACH:
		hInstance = (HINSTANCE)hModule;
		fPowerOn = FALSE;
		fReadThreadTerminated = FALSE;
		fTerminateReadThread = FALSE;
		fInitOK = Init();
		break;
    case DLL_PROCESS_DETACH:
		DeInit();
		break;
    }
    return TRUE;
}

