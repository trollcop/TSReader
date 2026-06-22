#include <windows.h>
#include <commctrl.h>
#include <shlobj.h>
#include <stdio.h>

#include "TSReader.h"
#include "util.h"
#include "resource.h"

extern PVARIABLES v;
extern HANDLE hInstance;
extern char gszAppName[];

#define GPS_SERIAL_BUFFER_SIZE 4096

void GetSourceInfoLine(int nLine, char * szOutput);

DWORD WINAPI GPSSerialReadThread(LPVOID pvarg)
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
	EnterCriticalSection(&v->csGPSSerial);
	v->nGPSSerialRead = 0;
	v->nGPSSerialWrite = 0;
	LeaveCriticalSection(&v->csGPSSerial);

    GetCommTimeouts(v->hGPSPort, &CommTimeouts);
    CommTimeouts.ReadIntervalTimeout = MAXDWORD;
    CommTimeouts.ReadTotalTimeoutMultiplier = 0;
    CommTimeouts.ReadTotalTimeoutConstant = 0;	// Read timeout at 50msec
	SetCommTimeouts(v->hGPSPort, &CommTimeouts);

	PurgeComm(v->hGPSPort, PURGE_TXCLEAR | PURGE_RXCLEAR);
	SetupComm(v->hGPSPort, 32 * 1024, 32 * 1024);
	SetCommMask (v->hGPSPort, EV_RXCHAR | EV_CTS | EV_DSR | EV_RLSD | EV_RING);

	v->fGPSSerialThreadReady = TRUE;
	v->fGPSSerialThreadRunning = TRUE;

	while (!v->fStopGPSSerialThread)
	{
		if (!WaitCommEvent(v->hGPSPort, &fdwCommMask, &OverLapped))
		{
			if (GetLastError() == ERROR_IO_PENDING)
			{
				WaitForSingleObject(OverLapped.hEvent, INFINITE);
			}
        }

		// Reset the comm Mask
		SetCommMask(v->hGPSPort, EV_RXCHAR | EV_CTS | EV_DSR | EV_RING);

		if (fdwCommMask & EV_RXCHAR)
		{
			// Loop waiting for data.
			do
			{
				if (!ReadFile (v->hGPSPort, &InByte, 1, &BytesTransferred,
							   &OverLapped))
				{
					if (ERROR_IO_PENDING == GetLastError())
					{
						WaitForSingleObject(OverLapped.hEvent, INFINITE);
					}
                }
				if (1 == BytesTransferred)
				{
					EnterCriticalSection(&v->csGPSSerial);
					v->szGPSSerialBuffer[v->nGPSSerialWrite++] = InByte;
					if (v->nGPSSerialWrite == GPS_SERIAL_BUFFER_SIZE)
						v->nGPSSerialWrite = 0;
					if (v->nGPSSerialWrite == v->nGPSSerialRead)
					{
						LeaveCriticalSection(&v->csGPSSerial);
#ifdef _DEBUG
						{
							char szTemp[100];
							wsprintf(szTemp, "Serial receiver overrun v->nGPSSerialWrite[%d] = %d v->nGPSSerialRead[%d] = %d\r\n", v->nGPSSerialWrite, v->nGPSSerialRead);
							OutputDebugString(szTemp);
						}
#endif _DEBUG
						break;
					}
					LeaveCriticalSection(&v->csGPSSerial);

				}
			} while (1 == BytesTransferred);
		}

	}

	CloseHandle (OverLapped.hEvent);
#ifdef _DEBUG
	{
		char szTemp[100];
		wsprintf(szTemp, "GPSSerialReadThread terminated\r\n");
		OutputDebugString(szTemp);
	}
#endif _DEBUG
	v->fGPSSerialThreadRunning = FALSE;
	return 0;
}

BYTE ReceiveGPSSerial(int * nTimeout)
{
	BYTE c;

	// Pickup a character
	do
	{
		EnterCriticalSection(&v->csGPSSerial);	
		if (v->nGPSSerialWrite != v->nGPSSerialRead)
		{
			c = v->szGPSSerialBuffer[v->nGPSSerialRead++];
			if (v->nGPSSerialRead == GPS_SERIAL_BUFFER_SIZE)
				v->nGPSSerialRead = 0;
			LeaveCriticalSection(&v->csGPSSerial);
			return c;
		}
		else
		{
			LeaveCriticalSection(&v->csGPSSerial);
			Sleep(1);
			(*nTimeout)--;
			if (*nTimeout == 0)
				return 0;
		}
	} while (TRUE);

	return 0;
}

void SetupGPSPortParameters(int nBaudRate)
{
	DCB		PortDCB;
    PortDCB.DCBlength = sizeof(DCB);

	// Set the port info.
    GetCommState(v->hGPSPort, &PortDCB);
	PortDCB.BaudRate		= nBaudRate;
	PortDCB.fBinary			= TRUE;
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
	PortDCB.ByteSize        = 7;
	PortDCB.Parity          = EVENPARITY;
	PortDCB.fParity			= TRUE;
	PortDCB.StopBits        = 2;
	SetCommState(v->hGPSPort, &PortDCB);
}

BOOL OpenGPSSerialPort(int nBaudRate)
{
	HANDLE	hThread;
	DWORD	ThreadID;
	char	szPortName[32];

	v->fGPSSerialThreadReady = FALSE;

	wsprintf(szPortName, "\\\\.\\%s", v->szGPSSerialPort);
	v->hGPSPort = CreateFile(szPortName, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_FLAG_OVERLAPPED, NULL);
    if (v->hGPSPort == INVALID_HANDLE_VALUE)
		return FALSE;

	InitializeCriticalSection(&v->csGPSSerial);
	v->szGPSSerialBuffer = LocalAlloc(LPTR, GPS_SERIAL_BUFFER_SIZE);

	SetupGPSPortParameters(nBaudRate);
	SetupComm(v->hGPSPort, 10 * 1024, 10 * 1024);

	// Start the read thread
	v->fStopGPSSerialThread = FALSE;
	hThread = CreateThread(NULL, 0, GPSSerialReadThread, (LPVOID)0, 0, &ThreadID);
	SetThreadPriority(hThread, THREAD_PRIORITY_IDLE);
	CloseHandle(hThread);
	
	return TRUE;
}

BOOL CloseGPSSerialPort(void)
{
	v->fStopGPSSerialThread = TRUE;
	CloseHandle(v->hGPSPort);
	while (v->fGPSSerialThreadRunning)
		Sleep(50);
	DeleteCriticalSection(&v->csGPSSerial);
	LocalFree(v->szGPSSerialBuffer);
	v->hGPSPort = INVALID_HANDLE_VALUE;
	return TRUE;
}

BOOL OpenCurrentlySelectedGPSSerialPort(HWND hDlg)
{
	int nItem = (int)SendDlgItemMessage(hDlg, IDC_GPS_PORT, CB_GETCURSEL, 0, 0);
	if (nItem != CB_ERR)
	{
		SetDlgItemText(hDlg, IDC_GPS_DATA, "");
		if (v->hGPSPort != INVALID_HANDLE_VALUE)
			CloseGPSSerialPort();
		SendDlgItemMessage(hDlg, IDC_GPS_PORT, CB_GETLBTEXT, nItem, (LPARAM)v->szGPSSerialPort);
		if (OpenGPSSerialPort(v->nGPSSerialBaudRate) == TRUE)
			return TRUE;
		SetDlgItemText(hDlg, IDC_GPS_DATA, "Unable to open selected serial port - try another port");
	}
	return FALSE;
}

// Parses a line of GPS NMEA - if real GPS data, returns TRUE
BOOL ParseGPSLine(char * szBuffer)
{
	if (szBuffer[0] == '$' && szBuffer[1] == 'G' && szBuffer[2] == 'P')
	{
		char * szChecksumPtr = strstr(szBuffer, "*");
		if (szChecksumPtr != NULL)
		{
			char * szChecksumCheckPtr = &szBuffer[1];
			int nReceivedChecksum = -1;
			BYTE bCalculatedChecksum = 0;

			*szChecksumPtr = '\0';
			sscanf(&szChecksumPtr[1], "%x", &nReceivedChecksum);
			while (szChecksumCheckPtr != szChecksumPtr)
			{
				bCalculatedChecksum ^= *szChecksumCheckPtr;
				szChecksumCheckPtr++;
			}
			if (bCalculatedChecksum == nReceivedChecksum)
			{
				if (memcmp(&szBuffer[3], "GGA", 3) == 0)
				{
					// GPGGA message - that's the one we want
					int nElementIndex = 0;
					int nItemIndex = 0;
					char * szBufferPtr;
					char * stopstring;
					char szElements[20][32];

					memset(szElements, 0, sizeof(szElements));
					szBufferPtr = szBuffer + 7;	// points to the first element of the line
					while (*szBufferPtr != '\0')
					{
						if (*szBufferPtr == ',')
						{
							nElementIndex++;
							nItemIndex = 0;
						}
						else
						{
							szElements[nElementIndex][nItemIndex++] = *szBufferPtr;
						}
						szBufferPtr++;
					}

					v->dGPSLatitude = strtod(szElements[1], &stopstring) / 100.0;
					v->fGPSNorth = (szElements[2][0] == 'N');
					v->dGPSLongitude = strtod(szElements[3], &stopstring) / 100.0;
					v->fGPSEast = (szElements[4][0] == 'E');;
					v->nGPSFixQuality = atoi(szElements[5]);
					v->nGPSSatellitesTracked = atoi(szElements[6]);

					if (v->nGPSReceiveState >= GPS_STATE_DATA_NMEA)
					{
						if (v->nGPSFixQuality == 0)
							v->nGPSReceiveState = GPS_STATE_DATA_NO_FIX;
						else
							v->nGPSReceiveState = GPS_STATE_DATA_FIX;
					}
				}
				else if (memcmp(&szBuffer[3], "RMC", 3) == 0)
				{
					int nElementIndex = 0;
					int nItemIndex = 0;
					char * szBufferPtr;
					char szElements[20][32];

					memset(szElements, 0, sizeof(szElements));
					szBufferPtr = szBuffer + 7;	// points to the first element of the line
					while (*szBufferPtr != '\0')
					{
						if (*szBufferPtr == ',')
						{
							nElementIndex++;
							nItemIndex = 0;
						}
						else
						{
							szElements[nElementIndex][nItemIndex++] = *szBufferPtr;
						}
						szBufferPtr++;
					}
					if (lstrlen(szElements[0]) && lstrlen(szElements[8]))
					{
						char szTemp[4];
						
						szTemp[3] = '\0';
						szTemp[0] = szElements[0][0]; szTemp[1] = szElements[0][1];
						v->stGPS.wHour = (WORD)atoi(szTemp);
						szTemp[0] = szElements[0][2]; szTemp[1] = szElements[0][3];
						v->stGPS.wMinute = (WORD)atoi(szTemp);
						szTemp[0] = szElements[0][4]; szTemp[1] = szElements[0][5];
						v->stGPS.wSecond = (WORD)atoi(szTemp);
						szTemp[0] = szElements[8][0]; szTemp[1] = szElements[8][1];
						v->stGPS.wDay = (WORD)atoi(szTemp);
						szTemp[0] = szElements[8][2]; szTemp[1] = szElements[8][3];
						v->stGPS.wMonth = (WORD)atoi(szTemp);
						szTemp[0] = szElements[8][4]; szTemp[1] = szElements[8][5];
						v->stGPS.wYear = (WORD)atoi(szTemp) + 2000;
					}
				}

			}
			return TRUE;
		}
	}
	return FALSE;
}

int LoadGPSPortList(HWND hDlg, BOOL fActuallyLoad)
{
	int nRetVal = 0;
	HKEY hCU;

	if (fActuallyLoad)
		while(SendDlgItemMessage(hDlg, IDC_GPS_PORT, CB_DELETESTRING, 0, 0) != CB_ERR);

	// look at the values under HKEY_LOCAL_MACHINE\Hardware\Devicemap\Serialcomm
	if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, 
					 TEXT("Hardware\\Devicemap\\Serialcomm"),
					 0,
					 KEY_QUERY_VALUE,
					 &hCU) == ERROR_SUCCESS)
	{
		int nSelectedGPSItem = -1;
		int i;

		for (i = 0; ; i++)
		{
			LONG lResult;
			DWORD dwType;
			char szValueName[MAX_PATH];
			char szValue[MAX_PATH];
			DWORD dwValueName = sizeof(szValueName);
			DWORD dwValue = sizeof(szValue);

			lResult = RegEnumValue(hCU,
								   i,
								   szValueName,
								   &dwValueName,
								   NULL,
								   &dwType,
								   (LPBYTE)szValue,
								   &dwValue);
			if (lResult != ERROR_SUCCESS)
				break;
			else
			{
				if (fActuallyLoad)
				{
					int nItem;
					nItem = (int)SendDlgItemMessage(hDlg, IDC_GPS_PORT, CB_ADDSTRING, 0, (LPARAM)szValue);
					if (lstrcmp(szValue, v->szGPSSerialPort) == 0)
					{
						SendDlgItemMessage(hDlg, IDC_GPS_PORT, CB_SETCURSEL, nItem, 0);
						nSelectedGPSItem = nItem;
					}
				}
				else
				{
					//todo
				}
				nRetVal++;
			}
		}
		RegCloseKey(hCU);
		//if (nSelectedGPSItem == -1)
		//	SendDlgItemMessage(hDlg, IDC_GPS_PORT, CB_SETCURSEL, 0, 0);
	}
	return nRetVal;
}

void LogGPS(HWND hDlg, BOOL fManual)
{
	HANDLE hOutputFile;
	DWORD dwWritten;
	SYSTEMTIME st;
	char * szNorthSouth;
	char * szEastWest;
	char szOutputFile[MAX_PATH];
	char szTime[128] = {""};
	char szOutputLine[MAX_PATH];

	if (v->nGPSReceiveState != GPS_STATE_DATA_FIX)
	{
		if (fManual)
			MessageBox(hDlg, "Unable to log data since the GPS currently doesn't have a fix", gszAppName, MB_ICONSTOP);
		return;
	}

	GetDlgItemText(hDlg, IDC_GPS_OUTPUT_FILE, szOutputFile, sizeof(szOutputFile));
	hOutputFile = CreateFile(szOutputFile, GENERIC_READ | GENERIC_WRITE, 0, (LPSECURITY_ATTRIBUTES)NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, (HANDLE)NULL);
	if (hOutputFile == INVALID_HANDLE_VALUE)
	{
		char szTemp[MAX_PATH];
		wsprintf(szTemp, "Unable to open log file '%s'", szOutputFile);
		MessageBox(hDlg, szTemp, gszAppName, MB_ICONSTOP);
		return;
	}
	SetFilePointer(hOutputFile, 0, NULL, FILE_END);
	
	// Get Time
	switch(v->nGPSLogTime)
	{
	default:
		memcpy(&st, &v->stGPS, sizeof(SYSTEMTIME));
		break;
	case 1:
		GetLocalTime(&st);
		break;
	case 2:
		GetSystemTime(&st);
		break;
	}

	// Log it
	if (v->fGPSNorth)
		szNorthSouth = "N";
	else
		szNorthSouth = "S";
	if (v->fGPSEast)
		szEastWest = "E";
	else
		szEastWest = "W";

	wsprintf(szTime, "%04d/%02d/%02d,%02d:%02d:%02d", st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);
	sprintf(szOutputLine, "%s,%.3f,%s,%.3f,%s,%s", szTime, v->dGPSLatitude, szNorthSouth, v->dGPSLongitude, szEastWest, v->szGPSLastSignal);
	if (v->fGPSErrorLogEnabled)
	{
		char szTemp[128];
		wsprintf(szTemp, ",%d,%d", v->nGPSErrorLogContinuityErrorsDelta, v->nGPSErrorLogTEIErrorsDelta);
		lstrcat(szOutputLine, szTemp);
	}
	lstrcat(szOutputLine, "\r\n");
	WriteFile(hOutputFile, szOutputLine, lstrlen(szOutputLine), &dwWritten, NULL);

	CloseHandle(hOutputFile);
	GetLocalTime(&v->stLastGPSSample);
}

INT_PTR CALLBACK GPSSignalExportDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg)
	{
	case WM_INITDIALOG:
		{
			HDC hDC;
			char szSignal[64];

			SetDlgItemText(hDlg, IDC_GPS_DATA, "");
			EnableWindow(GetDlgItem(hDlg, IDOK), v->fGPSLogManually);
			EnableWindow(GetDlgItem(hDlg, IDC_GPS_LOG_SECONDS), !v->fGPSLogManually);
			if (v->fGPSLogManually)
				CheckDlgButton(hDlg, IDC_GPS_LOG_MANUAL, BST_CHECKED);
			else
				CheckDlgButton(hDlg, IDC_GPS_LOG_TIMED, BST_CHECKED);
			SetDlgItemInt(hDlg, IDC_GPS_LOG_SECONDS, v->nGPSLogSeconds, FALSE);
			SetDlgItemText(hDlg, IDC_GPS_OUTPUT_FILE, v->szGPSLogFile);
			switch(v->nGPSLogTime)
			{
			default:
				CheckDlgButton(hDlg, IDC_GPS_GPS_TIME, BST_CHECKED);
				break;
			case 1:
				CheckDlgButton(hDlg, IDC_GPS_PC_LOCAL_TIME, BST_CHECKED);
				break;
			case 2:
				CheckDlgButton(hDlg, IDC_GPS_PC_UTC_TIME, BST_CHECKED);
				break;
			}
			CheckDlgButton(hDlg, IDC_GPS_ERROR_LOG_ENABLED, v->fGPSErrorLogEnabled);
			SetDlgItemInt(hDlg, IDC_GPS_ERROR_LOG_MS, v->nGPSErrorLogMilliseconds, FALSE);

			v->hGPSPort = INVALID_HANDLE_VALUE;
			memset(v->szGPSLineBuffer, 0, sizeof(v->szGPSLineBuffer));
			v->nGPSLineBufferWritePos = 0;
			v->nGPSFixQuality = v->nGPSSatellitesTracked = 0;
			v->dGPSLatitude = 0.0;
			v->dGPSLongitude = 0.0;
			v->nGPSReceiveState = GPS_STATE_NO_DATA;
			v->nGPSReceiveStateCounter = 0;
			v->nGPSBadPacketCount = 0;
			v->nGPSSerialPortCount = 0;
			memset(&v->stGPS, 0, sizeof(v->stGPS));
			memset(&v->stLastGPSSample, 0, sizeof(v->stLastGPSSample));
			v->szGPSLastSignal[0] = '\0';
			v->nGPSErrorLogContinuityErrorsDelta = v->nGPSErrorLogTEIErrorsDelta = 0;
			v->nGPSAutoLogHalfSecondCounter = 0;
			if (v->nGPSLogSeconds)
				v->nGPSAutoLogHalfSecondCounter = v->nGPSLogSeconds * 2;

			SendDlgItemMessage(hDlg, IDC_GPS_DATA, WM_SETFONT, (WPARAM)v->hCourierNew, MAKELONG(TRUE, 0));
			
			{
				int nBaudRates[] = {4800, 9600, 19200, 38400, 57600, 0};
				int i;

				for (i = 0; nBaudRates[i] != 0; i++)
				{
					int nItem;
					char szTemp[16];

					wsprintf(szTemp, "%d", nBaudRates[i]);
					nItem = (int)SendDlgItemMessage(hDlg, IDC_GPS_RATE, CB_ADDSTRING, 0, (LPARAM)szTemp);
					if (nBaudRates[i] == v->nGPSSerialBaudRate)
						SendDlgItemMessage(hDlg, IDC_GPS_RATE, CB_SETCURSEL, nItem, 0);
				}
			}

			v->nGPSSerialPortCount = LoadGPSPortList(hDlg, TRUE);
			if (OpenCurrentlySelectedGPSSerialPort(hDlg) == TRUE)
			{
				SetTimer(hDlg, 1, 25, NULL);
				SetTimer(hDlg, 2, 500, NULL);
			}
			GetSourceInfoLine(2, szSignal);
			v->nGPSSignalMode = DetermineSignalType(szSignal);
			
			hDC = GetDC(hDlg);
			v->hGPSDisplayFont = CreateFont(-MulDiv(14, GetDeviceCaps(hDC, LOGPIXELSY), 72),
									   0,
									   0,
									   0,
									   500,
									   FALSE,
									   FALSE,
									   FALSE,
									   ANSI_CHARSET,
									   OUT_DEFAULT_PRECIS,
									   CLIP_DEFAULT_PRECIS,
									   ANTIALIASED_QUALITY,
									   FF_DONTCARE | VARIABLE_PITCH,
									   "Arial");						
			ReleaseDC(hDlg, hDC);

			SetFocus(GetDlgItem(hDlg, IDOK));
		}
		break;
	case WM_CLOSE:
		EndDialog(hDlg, FALSE);
		break;
	case WM_COMMAND:
		switch(HIWORD(wParam))
		{
		case BN_CLICKED:
			switch(LOWORD(wParam))
			{
			case IDOK:
				if (v->fGPSErrorLogEnabled && v->nGPSErrorLogMilliseconds > 0)
				{
					v->nGPSErrorLogContinuityErrors = v->nContinuityErrors;
					v->nGPSErrorLogTEIErrors = v->nTEIErrors;
					SetTimer(hDlg, 3, v->nGPSErrorLogMilliseconds, NULL);
				}
				else
					LogGPS(hDlg, TRUE);
				break;
			case IDCANCEL:
				EndDialog(hDlg, FALSE);
				break;
			case IDC_GPS_LOG_MANUAL:
			case IDC_GPS_LOG_TIMED:
				v->fGPSLogManually = IsDlgButtonChecked(hDlg, IDC_GPS_LOG_MANUAL);
				EnableWindow(GetDlgItem(hDlg, IDOK), v->fGPSLogManually);
				EnableWindow(GetDlgItem(hDlg, IDC_GPS_LOG_SECONDS), !v->fGPSLogManually);
				if (LOWORD(wParam) == IDC_GPS_LOG_TIMED)
					SetFocus(GetDlgItem(hDlg, IDC_GPS_LOG_SECONDS));
				break;
			case IDC_GPS_OUTPUT_FILE_BROWSE:
				{
					OPENFILENAME ofn;
					char szInitialDir[MAX_PATH] = {0};

					memset( &(ofn), 0, sizeof(ofn));
					ofn.lStructSize	= sizeof(ofn);
					ofn.hwndOwner = hDlg;
					ofn.lpstrFile = v->szGPSLogFile;
					ofn.nMaxFile = MAX_PATH;
					ofn.lpstrFilter = TEXT("Text Files (*.txt)\0*.txt\0All Files (*.*)\0*.*\0\0");	
					ofn.lpstrTitle = TEXT("Select GPS Log File");
					ofn.lpstrDefExt = TEXT("txt");
					ofn.Flags =  OFN_HIDEREADONLY;
					ofn.lpstrInitialDir = szInitialDir;				
					if (myGetSaveFileName(&ofn) == TRUE)
					{
						lstrcpy(v->szGPSLogFile, ofn.lpstrFile);
						SetDlgItemText(hDlg, IDC_GPS_OUTPUT_FILE, v->szGPSLogFile);
					}
				}
				break;
			case IDC_GPS_GPS_TIME:
				v->nGPSLogTime = 0;
				break;
			case IDC_GPS_PC_LOCAL_TIME:
				v->nGPSLogTime = 1;
				break;
			case IDC_GPS_PC_UTC_TIME:
				v->nGPSLogTime = 2;
				break;
			case IDC_GPS_ERROR_LOG_ENABLED:
				v->fGPSErrorLogEnabled = IsDlgButtonChecked(hDlg, IDC_GPS_ERROR_LOG_ENABLED);
				break;
			}
			break;
		case EN_CHANGE:
			switch(LOWORD(wParam))
			{
			case IDC_GPS_LOG_SECONDS:
				v->nGPSLogSeconds = GetDlgItemInt(hDlg, IDC_GPS_LOG_SECONDS, NULL, FALSE);
				if (v->nGPSLogSeconds)
					v->nGPSAutoLogHalfSecondCounter = v->nGPSLogSeconds * 2;
				break;
			case IDC_GPS_OUTPUT_FILE:
				GetDlgItemText(hDlg, IDC_GPS_OUTPUT_FILE, v->szGPSLogFile, sizeof(v->szGPSLogFile));
				break;
			case IDC_GPS_ERROR_LOG_MS:
				v->nGPSErrorLogMilliseconds = GetDlgItemInt(hDlg, IDC_GPS_ERROR_LOG_MS, NULL, FALSE);
				break;
			}
			break;
		case CBN_SELCHANGE:
			switch(LOWORD(wParam))
			{
			case IDC_GPS_PORT:
				if (v->hGPSPort != INVALID_HANDLE_VALUE)
				{
					KillTimer(hDlg, 1);
					KillTimer(hDlg, 2);
				}
				memset(v->szGPSLineBuffer, 0, sizeof(v->szGPSLineBuffer));
				v->nGPSLineBufferWritePos = 0;
				v->nGPSReceiveState = GPS_STATE_NO_DATA;
				v->nGPSReceiveStateCounter = 0;
				if (OpenCurrentlySelectedGPSSerialPort(hDlg) == TRUE)
				{
					SetTimer(hDlg, 1, 25, NULL);
					SetTimer(hDlg, 2, 500, NULL);
				}
				break;
			case IDC_GPS_RATE:
				{
					int nItem = (int)SendDlgItemMessage(hDlg, IDC_GPS_RATE, CB_GETCURSEL, 0, 0);
					if (nItem != CB_ERR)
					{
						char szTemp[16];
						SendDlgItemMessage(hDlg, IDC_GPS_RATE, CB_GETLBTEXT, nItem, (LPARAM)szTemp);
						v->nGPSSerialBaudRate = atoi(szTemp);
						SetupGPSPortParameters(v->nGPSSerialBaudRate);
					}
				}
				break;
			}
			break;
		}
		break;
	case WM_DESTROY:
		if (v->hGPSPort != INVALID_HANDLE_VALUE)
		{
			CloseGPSSerialPort();
			KillTimer(hDlg, 1);
			KillTimer(hDlg, 2);
		}
		DeleteObject(v->hGPSDisplayFont);
		break;
	case WM_TIMER:
		switch(wParam)
		{
		case 1:		// serial port
			{
				do
				{
					int nTimeout = 1;
					BYTE c;

					c = ReceiveGPSSerial(&nTimeout);
					if (nTimeout == 0)
						break;
					v->nGPSReceiverTimeout = 0;
					if (v->nGPSReceiveState == GPS_STATE_NO_DATA)
					{
						v->nGPSReceiveState = GPS_STATE_DATA_NO_NMEA;
						v->nGPSReceiveStateCounter = 0;
					}

					if (c != 10) // ignore LF
					{
						if (c == 13)
						{
							v->szGPSLineBuffer[v->nGPSLineBufferWritePos] = '\0';
							if (strlen(v->szGPSLineBuffer))
							{
								if (ParseGPSLine(v->szGPSLineBuffer) == TRUE)
								{
									v->nGPSBadPacketCount = 0;
									if (v->nGPSReceiveState == GPS_STATE_DATA_NO_NMEA)
									{
										v->nGPSReceiveState = GPS_STATE_DATA_NMEA;
										v->nGPSReceiveStateCounter = 0;
									}
								}
								else
								{
									v->nGPSBadPacketCount++;
									if (v->nGPSBadPacketCount >= 5)
										v->nGPSReceiveState = GPS_STATE_DATA_NO_NMEA;
								}
							}
							memset(v->szGPSLineBuffer, 0, sizeof(v->szGPSLineBuffer));
							v->nGPSLineBufferWritePos = 0;
						}
						else if (c >= 32 && c <= 127)
						{
							if (v->nGPSLineBufferWritePos < sizeof(v->szGPSLineBuffer))
								v->szGPSLineBuffer[v->nGPSLineBufferWritePos++] = c;
							else
							{
								memset(v->szGPSLineBuffer, 0, sizeof(v->szGPSLineBuffer));
								v->nGPSLineBufferWritePos = 0;
								v->nGPSReceiveState = GPS_STATE_DATA_NO_NMEA;
							}
						}
					}
				} while (TRUE);
				SetDlgItemText(hDlg, IDC_GPS_DATA, v->szGPSLineBuffer);
			}
			break;
		case 2:		// sample clock
			{
				float fSignal[2];

				if (v->nGPSReceiveState != GPS_STATE_NO_DATA)
				{
					v->nGPSReceiverTimeout++;
					if (v->nGPSReceiverTimeout >= 10)
						v->nGPSReceiveState = GPS_STATE_NO_DATA;
				}
				if (v->nGPSSerialPortCount != LoadGPSPortList(hDlg, FALSE))
				{
					v->nGPSSerialPortCount = LoadGPSPortList(hDlg, TRUE);
				}

				// Get Signal
				lstrcpy(v->szGPSLastSignal, "Unable to determine tuner signal");
				ExtractSignalData(v->nGPSSignalMode, &fSignal[0], &fSignal[1]);
				switch(v->nGPSSignalMode)
				{
				case SIGNAL_CHART_MODE_SNR:
					sprintf(v->szGPSLastSignal, "SNR,%.3f", fSignal[0]);
					break;
				case SIGNAL_CHART_MODE_BER:
					sprintf(v->szGPSLastSignal, "BER,%.3f", fSignal[0]);
					break;
				case SIGNAL_CHART_MODE_QUALITY:
					sprintf(v->szGPSLastSignal, "S/Q,%.3f,%.3f", fSignal[0], fSignal[1]);
					break;
				case SIGNAL_CHART_MODE_QUALITY_DBM:
					sprintf(v->szGPSLastSignal, "S/D,%.3f,%.3f", fSignal[0], fSignal[1]);
					break;
				}

				// Automatically log if we're setup to
				if (!v->fGPSLogManually)
				{
					v->nGPSAutoLogHalfSecondCounter--;
					if (v->nGPSAutoLogHalfSecondCounter == 0)
					{
						LogGPS(hDlg, FALSE);
						v->nGPSAutoLogHalfSecondCounter = v->nGPSLogSeconds * 2;
					}
				}
			}
			InvalidateRect(hDlg, NULL, FALSE);
			break;
		case 3:		// Error log (CC/TEI)
			KillTimer(hDlg, 3);
			v->nGPSErrorLogContinuityErrorsDelta = v->nContinuityErrors - v->nGPSErrorLogContinuityErrors;
			v->nGPSErrorLogTEIErrorsDelta = v->nTEIErrors - v->nGPSErrorLogTEIErrors;
			LogGPS(hDlg, FALSE);
			if (!v->fGPSLogManually)
			{
				v->nGPSErrorLogContinuityErrors = v->nContinuityErrors;
				v->nGPSErrorLogTEIErrors = v->nTEIErrors;
				SetTimer(hDlg, 3, v->nGPSErrorLogMilliseconds, NULL);
			}
			break;			
		}
	case WM_PAINT:
		{
			// 
			RECT rc, rcFrame, rcParent;
			HDC hDC, hRealDC;
			HBITMAP memBM;
			PAINTSTRUCT ps;
			HANDLE hBlackBrush = CreateSolidBrush(0);

			char szTemp[256]  = {""};
			char szLastSample[128];

			hRealDC = BeginPaint(hDlg, &ps);
			hDC = CreateCompatibleDC(hRealDC);
			GetClientRect(GetDlgItem(hDlg, IDC_GPS_LOG_INFO), &rc);
			rc.right -= 7;		// otherwise it's too big for the frame
			rc.bottom -= 21;	// same thing
			memBM = CreateCompatibleBitmap (hRealDC, rc.right, rc.bottom);
			SelectObject(hDC, memBM);
			SetBkMode(hDC, TRANSPARENT);

			// Show status
			SetTextColor(hDC, RGB(0, 255, 0));
			SelectObject(hDC, v->hGPSDisplayFont);			

			switch(v->nGPSReceiveState)
			{
			case GPS_STATE_NO_DATA:
				lstrcpy(szTemp, "No GPS data detected");
				break;
			case GPS_STATE_DATA_NO_NMEA:
				lstrcpy(szTemp, "Data detected - not GPS format");
				break;
			case GPS_STATE_DATA_NMEA:
				lstrcpy(szTemp, "GPS data detected");
				break;
			case GPS_STATE_DATA_NO_FIX:
				lstrcpy(szTemp, "No GPS fix");
				break;
			case GPS_STATE_DATA_FIX:
				{
					char * szNorthSouth;
					char * szEastWest;

					if (v->fGPSNorth)
						szNorthSouth = "N";
					else
						szNorthSouth = "S";
					if (v->fGPSEast)
						szEastWest = "E";
					else
						szEastWest = "W";
					sprintf(szTemp, "%.3f %s %.3f %s", v->dGPSLatitude, szNorthSouth, v->dGPSLongitude, szEastWest);
				}
			}
			TextOut(hDC, 5, 20, szTemp, lstrlen(szTemp));

			// Show GPS signal meter
			{
				int nStartX = 100, nStartY = 10;
				int i;

				nStartX = rc.right - (5 * 9);
				nStartY = rc.top + ((rc.bottom - rc.top) / 2) - ((5 * 8) / 2);

				for (i = 0; i < 5; i++)
				{
					RECT rcRectangle;

					rcRectangle.left = nStartX + (8 * i);
					rcRectangle.top = nStartY + ((4 - i) * 8);
					rcRectangle.right = rcRectangle.left + 6;
					rcRectangle.bottom = nStartY + (5 * 8);
					if (v->nGPSReceiveState < GPS_STATE_DATA_NO_FIX)
						FrameRect(hDC, &rcRectangle, v->hBrRed);
					else
					{
						if (v->nGPSReceiveState == GPS_STATE_DATA_NO_FIX)
							FrameRect(hDC, &rcRectangle, v->hBrGreen);
						else
						{
							if (v->nGPSSatellitesTracked < 4 && i == 0)
								FillRect(hDC, &rcRectangle, v->hBrGreen);
							else if (v->nGPSSatellitesTracked >= 4 && v->nGPSSatellitesTracked < 6 && i < 2)
								FillRect(hDC, &rcRectangle, v->hBrGreen);
							else if (v->nGPSSatellitesTracked >= 6 && v->nGPSSatellitesTracked < 8 && i < 3)
								FillRect(hDC, &rcRectangle, v->hBrGreen);
							else if (v->nGPSSatellitesTracked >= 8 && v->nGPSSatellitesTracked < 10 && i < 4)
								FillRect(hDC, &rcRectangle, v->hBrGreen);
							else if (v->nGPSSatellitesTracked >= 10)
								FillRect(hDC, &rcRectangle, v->hBrGreen);
							else
								FrameRect(hDC, &rcRectangle, v->hBrGreen);
						}
					}
				if (v->nGPSReceiveState < GPS_STATE_DATA_NO_FIX)
					SelectObject(hDC, v->hRedPen);
				else
					SelectObject(hDC, v->hGreenPen);

				}
			}

			// Antenna signal info
			TextOut(hDC, 5, 40, v->szGPSLastSignal, lstrlen(v->szGPSLastSignal));

			// Last sample
			szLastSample[0] = '\0';
			if (v->stLastGPSSample.wYear)
				GetTimeFormat(LOCALE_USER_DEFAULT, 0, &v->stLastGPSSample, NULL, szLastSample, sizeof(szLastSample));		
			wsprintf(szTemp, "Last sample taken: %s", szLastSample);
			TextOut(hDC, 5, 60, szTemp, lstrlen(szTemp));
			
			// Blt it into the real DC
			GetWindowRect(GetDlgItem(hDlg, IDC_GPS_LOG_INFO), &rcFrame);
			GetWindowRect(hDlg, &rcParent);
			rcFrame.top -= rcParent.top; rcFrame.top -= 5;
			rcFrame.bottom -= rcParent.bottom;
			rcFrame.left -= rcParent.left;
			rcFrame.right -= rcParent.right;
			BitBlt(hRealDC, rcFrame.left, rcFrame.top, rc.right, rc.bottom, hDC, 0, 0, SRCCOPY);
			DeleteObject(memBM);
			DeleteDC(hDC);
			EndPaint(hDlg, &ps);
			DeleteObject(hBlackBrush);
		}
		break;
	}
	return FALSE;
}
