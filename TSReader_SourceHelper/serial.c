#define _WIN32_WINNT 0x0400
#include <windows.h>
#include <commctrl.h>

#include "..\TSReader.h"
#include "resource.h"

extern PVARIABLES v;
extern HANDLE hInstance;

#define SERIAL_BUFFER_SIZE 4096

HANDLE ghPort;
CRITICAL_SECTION csSerial;
BOOL gfStopSerialThread;
BOOL gfSerialThreadReady;
int nSerialWrite;
int nSerialRead;
BYTE * szSerialBuffer;

DWORD WINAPI SerialReadThread(LPVOID pvarg)
{
	BYTE	InByte;
	DWORD	BytesTransferred;
	DWORD	fdwCommMask;
    COMMTIMEOUTS CommTimeouts;
	BOOL fEscapedMode = FALSE;
	OVERLAPPED	OverLapped;

	memset ((char *)&OverLapped, 0, sizeof(OVERLAPPED));
	OverLapped.hEvent = CreateEvent (NULL, TRUE, TRUE, 0);

	// Setup ring buffer
	EnterCriticalSection(&csSerial);
	nSerialRead = 0;
	nSerialWrite = 0;
	LeaveCriticalSection(&csSerial);

    GetCommTimeouts(ghPort, &CommTimeouts);
    CommTimeouts.ReadIntervalTimeout = MAXDWORD;
    CommTimeouts.ReadTotalTimeoutMultiplier = 0;
    CommTimeouts.ReadTotalTimeoutConstant = 0;	// Read timeout at 50msec
	SetCommTimeouts(ghPort, &CommTimeouts);

	PurgeComm(ghPort, PURGE_TXCLEAR | PURGE_RXCLEAR);
	SetupComm(ghPort, 32 * 1024, 32 * 1024);
	SetCommMask (ghPort, EV_RXCHAR | EV_CTS | EV_DSR | EV_RLSD | EV_RING);

	gfSerialThreadReady = TRUE;
	v->fSerialReceiverControlThreadRunning = TRUE;

	while (!gfStopSerialThread)
	{
		if (!WaitCommEvent(ghPort, &fdwCommMask, &OverLapped))
		{
			if (GetLastError() == ERROR_IO_PENDING)
			{
				WaitForSingleObject(OverLapped.hEvent, INFINITE);
			}
        }

		// Reset the comm Mask
		SetCommMask(ghPort, EV_RXCHAR | EV_CTS | EV_DSR | EV_RING);

		if (fdwCommMask & EV_RXCHAR)
		{
			// Loop waiting for data.
			do
			{
				if (!ReadFile (ghPort, &InByte, 1, &BytesTransferred,
							   &OverLapped))
				{
					if (ERROR_IO_PENDING == GetLastError())
					{
						WaitForSingleObject(OverLapped.hEvent, INFINITE);
					}
                }
				if (1 == BytesTransferred)
				{
					EnterCriticalSection(&csSerial);
					szSerialBuffer[nSerialWrite++] = InByte;
					if (nSerialWrite == SERIAL_BUFFER_SIZE)
						nSerialWrite = 0;
					if (nSerialWrite == nSerialRead)
					{
						LeaveCriticalSection(&csSerial);
#ifdef _DEBUG
						{
							char szTemp[100];
							wsprintf(szTemp, "Serial receiver overrun nSerialWrite[%d] = %d nSerialRead[%d] = %d\r\n", nSerialWrite, nSerialRead);
							OutputDebugString(szTemp);
						}
#endif _DEBUG
						break;
					}
					LeaveCriticalSection(&csSerial);

				}
			} while (1 == BytesTransferred);
		}

	}

	CloseHandle (OverLapped.hEvent);
#ifdef _DEBUG
	{
		char szTemp[100];
		wsprintf(szTemp, "SerialReadThread terminated\r\n");
		OutputDebugString(szTemp);
	}
#endif _DEBUG
	v->fSerialReceiverControlThreadRunning = FALSE;
	return 0;
}

BYTE SourceHelper_ReceiveReceiverSerial(int * nTimeout)
{
	BYTE c;

	// Pickup a character
	do
	{
		EnterCriticalSection(&csSerial);	
		if (nSerialWrite != nSerialRead)
		{
			c = szSerialBuffer[nSerialRead++];
			if (nSerialRead == SERIAL_BUFFER_SIZE)
				nSerialRead = 0;
			LeaveCriticalSection(&csSerial);
			return c;
		}
		else
		{
			LeaveCriticalSection(&csSerial);
			Sleep(1);
			(*nTimeout)--;
			if (*nTimeout == 0)
				return 0;
		}
	} while (TRUE);

	return 0;
}

int SourceHelper_SendReceiverSerial(unsigned char * lpByte, DWORD dwBytesToWrite)
{
	DWORD dwBytesWritten;
	DWORD dwTotalWritten = 0;
	int i;

	OVERLAPPED	OverLapped;

	memset ((char *)&OverLapped, 0, sizeof(OVERLAPPED));
	OverLapped.hEvent = CreateEvent(NULL, TRUE, TRUE, 0);

	for (i = 0; i < (int)dwBytesToWrite; i++)
	{
		if (!WriteFile(ghPort, lpByte++, 1, &dwBytesWritten, &OverLapped))
		{
			if (ERROR_IO_PENDING == GetLastError())
			{
				WaitForSingleObject (OverLapped.hEvent, INFINITE);
			}
		}
		dwTotalWritten++;
	}
	CloseHandle (OverLapped.hEvent);
	return dwTotalWritten;
}

BOOL OpenSerialPort(int nBaudRate, int nByteSize, int nParity, int nStopBits, BOOL fDTR)
{
	HANDLE	hThread;
	DWORD	ThreadID;
	DCB		PortDCB;
	char	szPortName[10];
    PortDCB.DCBlength = sizeof(DCB);

	gfSerialThreadReady = FALSE;
	InitializeCriticalSection(&csSerial);
	szSerialBuffer = LocalAlloc(LPTR, SERIAL_BUFFER_SIZE);

	lstrcpy(szPortName, v->szSerialReceiverPort);
	lstrcat(szPortName, ":");
	ghPort = CreateFile(szPortName,
					   GENERIC_READ | GENERIC_WRITE,
					   0, NULL, OPEN_EXISTING,
					   FILE_FLAG_OVERLAPPED,
					   NULL);
    if (ghPort == INVALID_HANDLE_VALUE)
	{
		return FALSE;
	}

	// Set the port info.
    GetCommState(ghPort, &PortDCB);
	PortDCB.BaudRate = nBaudRate;
	PortDCB.fBinary = TRUE;
	PortDCB.fOutxCtsFlow    = FALSE;//TRUE;  // ignore possible hangups
	PortDCB.fOutxDsrFlow    = FALSE;    // don't wait on the DSR line
	PortDCB.fDtrControl     = FALSE; //DTR_CONTROL_ENABLE;
	PortDCB.fDsrSensitivity = FALSE;
	PortDCB.fTXContinueOnXoff = FALSE;
	PortDCB.fOutX           = FALSE; // no XON/XOFF control
	PortDCB.fInX            = FALSE;
	PortDCB.fErrorChar      = FALSE;
	PortDCB.fNull           = FALSE;
	PortDCB.fRtsControl     = FALSE; //RTS_CONTROL_HANDSHAKE;
	PortDCB.fAbortOnError   = FALSE;

	PortDCB.ByteSize        = nByteSize;
	PortDCB.Parity          = nParity;
	if (PortDCB.Parity == NOPARITY)
		PortDCB.fParity = FALSE;
	else
		PortDCB.fParity = TRUE;
	PortDCB.StopBits        = nStopBits;

	SetCommState(ghPort, &PortDCB);
	if (fDTR)
		EscapeCommFunction(ghPort, SETDTR);
	else
		EscapeCommFunction(ghPort, CLRDTR);
	SetupComm(ghPort, 10 * 1024, 10 * 1024);

	// Start the read thread
	v->fSerialReceiverControlThreadRunning = FALSE;
	gfStopSerialThread = FALSE;
	hThread = CreateThread(NULL, 0, SerialReadThread, (LPVOID)0, 0, &ThreadID);
	SetThreadPriority(hThread, THREAD_PRIORITY_IDLE);
	CloseHandle(hThread);
	
	return TRUE;
}

BOOL CloseSerialPort()
{
	gfStopSerialThread = TRUE;
	CloseHandle(ghPort);
	Sleep(100);
	DeleteCriticalSection(&csSerial);
	LocalFree(szSerialBuffer);
	return TRUE;
}

