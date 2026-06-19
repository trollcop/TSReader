#include <windows.h>
#include <commctrl.h>
#include "TSReader.h"
#include "util.h"
#include "resource.h"

extern PVARIABLES v;

//telnet IAC commands
#define IAC 255				// 0xff
#define IAC_SB 250			// 0xfa
#define IAC_SE 240			// 0xf0	
#define IAC_WILL 251		// 0xfb
#define IAC_WONT 252		// 0xfc
#define IAC_DO 253			// 0xfd
#define IAC_DONT 254		// 0xfe

//telnet connection states
#define STATE_WAIT_LOGON 0
#define STATE_WAIT_PASSWORD 1
#define STATE_WAIT_SET_RESULTS 2
#define STATE_MONITOR_PLAYBACK 3

SOCKET sockTelnet;
HOSTENT	phe;
LPHOSTENT pHostent;
SOCKADDR_IN sinTelnet;
HWND hWndTrace;
BOOL fAbortTrace;

BYTE bIACOption = 0;
BYTE bIACCommand = 0;
BYTE bIACBlock[128];
int nIACBlockPos;
BOOL fIACBlockFlag = FALSE;
BOOL fDoingIAC = FALSE;
BOOL fFirstIACByte = FALSE;

// Should be in TSReader.h
void CopyListControlToClipboard(HWND hListControl, BOOL fAddCR);

void SendIACEmulatorName()
{
	int i;
	int nOutIndex;
	char szTempEmulation[] = {"ANSI"};
	char szTemp[100];
	
	// IAC SB terminal-type IS <term> IAC SE
	nOutIndex = 0;
	szTemp[nOutIndex++] = (TBYTE)IAC;
	szTemp[nOutIndex++] = (TBYTE)IAC_SB;
	szTemp[nOutIndex++] = (TBYTE)24;
	szTemp[nOutIndex++] = (TBYTE)0;
	for (i = 0; i < (int)lstrlen(szTempEmulation); i++)
		szTemp[nOutIndex++] = szTempEmulation[i];
	szTemp[nOutIndex++] = (TBYTE)IAC;
	szTemp[nOutIndex++] = (TBYTE)IAC_SE;
	send(sockTelnet, szTemp, 6 + lstrlen(szTempEmulation), 0);
}

void AnswerIAC()
{
	char szTemp[100];
	int nOutIndex;

	if (fIACBlockFlag == TRUE)
	{
		if ((TBYTE)bIACBlock[0] == 33)
		{
			return;
		}
		else
		{
			if (bIACBlock[0] == 24)
				// Kludge for Linux systems which send over the terminal
				// name as a ^A and then expect the terminal type to be
				// sent. It's not meant to work that way...
				SendIACEmulatorName();
			else
				// Tell them we WON'T		
				wsprintf(szTemp,TEXT("%c%c%c"),
								IAC,
								IAC_WONT,
								(TBYTE)bIACBlock[0]);
			send(sockTelnet, szTemp, 3, 0);
			return;
		}
	}
	else
	{
		lstrcpy(szTemp, TEXT(""));
		switch (bIACOption)
			{
			case 1:  // echo
				
				switch (bIACCommand)
				{
				case IAC_DO:
					//fLocalEcho = FALSE;
					break;
				case IAC_DONT:
					//fLocalEcho = TRUE;
					break;
				case IAC_WILL:
					wsprintf(szTemp,TEXT("%c%c%c"),
									IAC,
									IAC_DO,
									1);
					send(sockTelnet, szTemp, 3, 0);
					break;
				}
				
				/*
				wsprintf(szTemp,TEXT("%c%c%c"),
								IAC,
								IAC_WONT,
								1);
				send(sockTelnet, szTemp, 3, 0);
				*/
				break;

			case 3: // supress go-ahead
				break;

			case 24: // terminal type
				if (bIACCommand == IAC_DO)
				{
					// IAC WILL terminal-type
					wsprintf(szTemp,TEXT("%c%c%c"),
									IAC,
									IAC_WILL,
									24);
					send(sockTelnet, szTemp, 3, 0);
				}
				else
					SendIACEmulatorName();
				break;

			case 31: // Window size
				wsprintf(szTemp,TEXT("%c%c%c"),
								IAC,
								IAC_WONT,
								31);
				send(sockTelnet, szTemp, 3, 0);
				break;

			case 32: // Terminal speed
				wsprintf(szTemp,TEXT("%c%c%c"),
								IAC,
								IAC_WONT,
								32);
				send(sockTelnet, szTemp, 3, 0);
				break;

				if (bIACCommand == IAC_DO)
				{
					// IAC WILL terminal speed
					wsprintf(szTemp,TEXT("%c%c%c"),
									IAC,
									IAC_WILL,
									32);
					send(sockTelnet, szTemp, 3, 0);
				}
				else
				{
					char szSpeed[] = {TEXT("57600,57600")};
					int i;

					nOutIndex = 0;
					szTemp[nOutIndex++] = (TBYTE)IAC;
					szTemp[nOutIndex++] = (TBYTE)IAC_SB;
					szTemp[nOutIndex++] = (TBYTE)32;
					szTemp[nOutIndex++] = (TBYTE)0;
					for (i = 0; i < (int)lstrlen(szSpeed); i++)
						szTemp[nOutIndex++] = szSpeed[i];
					szTemp[nOutIndex++] = (TBYTE)IAC;
					szTemp[nOutIndex++] = (TBYTE)IAC_SE;
					send(sockTelnet, szTemp, 17, 0);
				}
				break;

			case 33: // Remote Flow control
				if (bIACCommand == IAC_DO)
				{
					wsprintf(szTemp,TEXT("%c%c%c"),
									IAC,
									IAC_WILL,
									33);
					send(sockTelnet, szTemp, 3, 0);
				}
				break;

			case 35: // X display location
				wsprintf(szTemp, TEXT("%c%c%c"),
								 IAC,
								 IAC_WONT,
								 35);
				send(sockTelnet, szTemp, 3, 0);
				break;

			case 36: // Environment option
				wsprintf(szTemp,TEXT("%c%c%c"),
								IAC,
								IAC_WONT,
								36);
				send(sockTelnet, szTemp, 3, 0);
				break;

			case 37: // Authentication
				if (bIACCommand == IAC_DO)
				{
					// IAC WILL authenticate
					wsprintf(szTemp,TEXT("%c%c%c"),
									IAC,
									IAC_WONT,
									37);
					send(sockTelnet, szTemp, 3, 0);
				}
				else
				{
					// IAC SB 0000 IAC SE
					nOutIndex = 0;
					szTemp[nOutIndex++] = (TBYTE)IAC;
					szTemp[nOutIndex++] = (TBYTE)IAC_SB;
					szTemp[nOutIndex++] = (TBYTE)37;
					szTemp[nOutIndex++] = (TBYTE)0;
					szTemp[nOutIndex++] = (TBYTE)0;
					szTemp[nOutIndex++] = (TBYTE)0;
					szTemp[nOutIndex++] = (TBYTE)0;
					szTemp[nOutIndex++] = (TBYTE)IAC;
					szTemp[nOutIndex++] = (TBYTE)IAC_SE;
					send(sockTelnet, szTemp, 9, 0);
				}
				break;

			case 39:
				wsprintf(szTemp,TEXT("%c%c%c"),
								IAC,
								IAC_WONT,
								39);
				send(sockTelnet, szTemp, 3, 0);
				break;

			case 0x26:
				wsprintf(szTemp,TEXT("%c%c%c"),
								IAC,
								IAC_DONT,
								0x26);
				send(sockTelnet, szTemp, 3, 0);
				break;

			default: // Catch all
				wsprintf(szTemp,TEXT("%c%c%c"),
								IAC,
								IAC_WONT,
								bIACOption);
				send(sockTelnet, szTemp, 3, 0);
				break;
			}
		}
}

BOOL ReadTelnetByte(BYTE * pByte)
{
	nIACBlockPos = 0;
	do
	{
		int nStatus = recv(sockTelnet, pByte, 1, 0);
		if (nStatus != 1)
			return FALSE;

		if (fDoingIAC == FALSE)
		{
			if (*pByte == IAC)
			{
				bIACOption = 0;
				bIACCommand = 0;
				fIACBlockFlag = FALSE;
				fDoingIAC = TRUE;
				fFirstIACByte = TRUE;
			}
			else
				return TRUE;
		}
		else
		{
			// We're processing IAC now
			if (fIACBlockFlag == FALSE)
			{
				if (*pByte == IAC_SB)
				{
					fIACBlockFlag = TRUE;
					nIACBlockPos = 0;
				}
				else
				{
					if (*pByte < 250)
					{
						// All done
						bIACOption = *pByte;
						fDoingIAC = FALSE;
					}
					else 
						bIACCommand = *pByte;
					if (fDoingIAC == FALSE)
					{
						if (bIACCommand == 0)
							bIACCommand = bIACOption;
						AnswerIAC();
					}			
				}
			}
			else
			{
				bIACBlock[nIACBlockPos] = *pByte;
				nIACBlockPos++;
				if (*pByte == IAC_SE)
				{
					fDoingIAC = FALSE;
					AnswerIAC();
				}
			}
		}
	} while (TRUE);

	return TRUE;
}

BOOL SendTelnetString(char * szString)
{
	int i;

	for (i = 0; i < lstrlen(szString); i++)
	{
		int nStatus;

		if (szString[i] == 13)
		{
			char szTemp[4];

			szTemp[0] = 13;
			szTemp[1] = 10;
			nStatus = send(sockTelnet, szTemp, 2, 0);
			if (nStatus != 2)
				return FALSE;
		}
		else
		{
			nStatus = send(sockTelnet, &szString[i], 1, 0);
			if (nStatus != 1)
				return FALSE;
		}
	}

	return TRUE;
}

INT_PTR CALLBACK RokuTraceDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg)
	{
	case WM_INITDIALOG:
		CheckDlgButton(hDlg, IDC_ROKU_AUTOSCROLL, v->fRokuTraceAutoscroll);
		SetTimer(hDlg, 1, 100, NULL);
		break;
	case WM_DESTROY:
		v->fRokuTraceAutoscroll = IsDlgButtonChecked(hDlg, IDC_ROKU_AUTOSCROLL);
		KillTimer(hDlg, 1);
		break;
	case WM_COMMAND:
		switch(LOWORD(wParam))
		{
		case IDC_ROKU_TRACE_COPY:
			CopyListControlToClipboard(GetDlgItem(hDlg, IDC_TRACE_LIST), FALSE);
			break;
		}
		break;
	case WM_CLOSE:
		PostMessage(v->hWndMainWindow, WM_COMMAND, ID_PLAYBACK_ROKUHD1000, 0);
		break;

	}

	return FALSE;
}

DWORD WINAPI RokuTraceWindowThread(LPVOID lpv)
{
	MSG msg;

	hWndTrace = CreateDialog(v->hInstance, MAKEINTRESOURCE(IDD_ROKU_TRACE), NULL, RokuTraceDlgProc);
	ShowWindow(hWndTrace, SW_SHOW);

	while (GetMessage(&msg, hWndTrace, 0, 0) && !fAbortTrace)
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	DestroyWindow(hWndTrace);
	hWndTrace = NULL;
	fAbortTrace = FALSE;
	return 0;
}

void AddTrace(char * szString)
{
	int nItemCount;
	char * szLineTerm = strstr(szString, "\r");
	
	if (szLineTerm != NULL)
		*szLineTerm = '\0';
	
	SendDlgItemMessage(hWndTrace, IDC_TRACE_LIST, LB_ADDSTRING, 0, (LPARAM)szString);
	nItemCount = (int)SendDlgItemMessage(hWndTrace, IDC_TRACE_LIST, LB_GETCOUNT, 0, 0);
	if (nItemCount == 1024)
	{
		SendDlgItemMessage(hWndTrace, IDC_TRACE_LIST, LB_DELETESTRING, 0, 0);
		nItemCount--;
	}
	if (IsDlgButtonChecked(hWndTrace, IDC_ROKU_AUTOSCROLL))
		SendDlgItemMessage(hWndTrace, IDC_TRACE_LIST, LB_SETCURSEL, nItemCount - 1, 0);
}

DWORD WINAPI RokuTelnetControlThread(LPVOID lpv)
{
	int status;
	int nCurrentState;
	int nCurrentLinePtr;
	int nLoginErrorCounter = 0;
	BOOL fAbort;
	BOOL fSentCommand = FALSE;
	BOOL fOKtoDisplay = FALSE;
	char szCurrentLine[2 * 1024];
	char szMonitorLine[256];

	hWndTrace = NULL;
	Sleep(250);	// give time for the HTTP server to start

	sockTelnet = socket(AF_INET, SOCK_STREAM, 0);
	if (sockTelnet == INVALID_SOCKET)
	{
		return FALSE;
	}

	//Retrieve the IP address
	if (!FillAddr(&sinTelnet, v->szRokuIP, 23))
	{
		closesocket(sockTelnet);
		return FALSE;
	}

	// Connect
	status = connect(sockTelnet, (PSOCKADDR)&sinTelnet, sizeof(sinTelnet));
	if (status != 0)
	{
		closesocket(sockTelnet);
		return FALSE;
	}

	// Get the status window going
	if (!v->fRokuTraceDisabled)
	{
		DWORD dwThreadID;
		HANDLE hThread;

		fAbortTrace = FALSE;
		hThread = CreateThread(NULL, 0, RokuTraceWindowThread, (LPVOID)0, 0, &dwThreadID);
		CloseHandle(hThread);
	}

	// Log onto the Roku and do our thing
	nCurrentState = STATE_WAIT_LOGON;
	memset(szCurrentLine, 0, sizeof(szCurrentLine));
	nCurrentLinePtr = 0;
	fAbort = FALSE;
	do
	{
		BOOL fStatus;
		BYTE b;

		fStatus = ReadTelnetByte(&b);
		if (fStatus == FALSE)
			break;
		szCurrentLine[nCurrentLinePtr++] = b;

		switch(nCurrentState)
		{
		case STATE_WAIT_LOGON:
			if (strstr(szCurrentLine, "login: ") != NULL)
			{
				SendTelnetString(v->szRokuUsername);
				SendTelnetString("\r");
				nCurrentState = STATE_WAIT_PASSWORD;
				memset(szCurrentLine, 0, sizeof(szCurrentLine));
				nCurrentLinePtr = 0;
			}
			break;
		case STATE_WAIT_PASSWORD:
			if (strstr(szCurrentLine, "# ") != NULL)
			{
				nCurrentState = STATE_WAIT_SET_RESULTS;
				SendTelnetString("set\r");
				memset(szCurrentLine, 0, sizeof(szCurrentLine));
				nCurrentLinePtr = 0;
			}
			else if (strstr(szCurrentLine, "Password: ") != NULL)
			{
				SendTelnetString(v->szRokuPassword);
				SendTelnetString("\r");
				nCurrentState = STATE_WAIT_PASSWORD;
				memset(szCurrentLine, 0, sizeof(szCurrentLine));
				nCurrentLinePtr = 0;
			}
			else if (strstr(szCurrentLine, "Login incorrect") != NULL)
			{
				nLoginErrorCounter++;
				if (nLoginErrorCounter > 3)
				{
					fAbort = TRUE;
					break;
				}
				nCurrentState = STATE_WAIT_LOGON;
				memset(szCurrentLine, 0, sizeof(szCurrentLine));
				nCurrentLinePtr = 0;
			}
			break;
		case STATE_WAIT_SET_RESULTS:
			if (b == '\n')
			{
				char * szPotentialRemoteHost = strstr(szCurrentLine, "REMOTEHOST=");
				if (szPotentialRemoteHost != NULL && !fSentCommand)
				{
					int i;
					char szRemoteHost[MAX_PATH];
					char szAppFolder[MAX_PATH] = {0};
					char szAppName[MAX_PATH] = {0};
					char szCommand[MAX_PATH];

					fSentCommand = TRUE;
					// Got the remote host (i.e. us) so we can now
					// send the MpegPSPlay command
					szPotentialRemoteHost = strstr(szCurrentLine, "=");
					lstrcpy(szRemoteHost, szPotentialRemoteHost + 1);
					szRemoteHost[lstrlen(szRemoteHost) - 2] = '\0';	// remote CRLF

					lstrcpy(szAppFolder, v->szRokuMpegPSPlayLocation);
					for (i = lstrlen(szAppFolder); i > 0; i--)
					{
						if (szAppFolder[i] == '/')
						{
							szAppFolder[i] = '\0';
							lstrcpy(szAppName, &szAppFolder[i + 1]);
							break;
						}
					}

					wsprintf(szCommand, "cd %s\r", szAppFolder);
					SendTelnetString(szCommand);
					wsprintf(szCommand, "%s http://%s:%d/\r", szAppName, szRemoteHost, v->nVLCPort);
					SendTelnetString(szCommand);
					nCurrentState = STATE_MONITOR_PLAYBACK;
					memset(szCurrentLine, 0, sizeof(szCurrentLine));
					nCurrentLinePtr = 0;
					lstrcpy(szMonitorLine, "# ");
					lstrcat(szMonitorLine, szCommand);
				}
				memset(szCurrentLine, 0, sizeof(szCurrentLine));
				nCurrentLinePtr = 0;
			}
			break;
		case STATE_MONITOR_PLAYBACK:
			if (b == '\n')
			{
				if (strstr(szCurrentLine, "Exiting.") != NULL)
					PostMessage(v->hDlgSIParser, WM_COMMAND, ID_PLAYBACK_ROKUHD1000, 0);
				if (strstr(szCurrentLine, szMonitorLine) != NULL)
					fOKtoDisplay = TRUE;
				if (fOKtoDisplay)
					AddTrace(szCurrentLine);
				memset(szCurrentLine, 0, sizeof(szCurrentLine));
				nCurrentLinePtr = 0;
			}
			break;
		}

	} while (!fAbort && !v->fTerminateRokuThread);
	if (fOKtoDisplay)
		AddTrace(szCurrentLine);
	fAbortTrace = TRUE;

	// All done - disconnect
	v->fTerminateRokuThread = FALSE;
	closesocket(sockTelnet);
	
	OutputDebugString("TSReader: Roku Telnet client thread closed\n");
	return 0;
}

