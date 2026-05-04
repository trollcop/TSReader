#include <windows.h>
#include <commctrl.h>
#include <time.h>
#include <winsock.h>
#include "../TSReader.h"
#include "../resource.h"

#define STRADIS_READ_SIZE 1024 * 8
#define XNS_RING_BUFFER_SIZE 1024 * 1024 * 5
#define XNS_SEND_BUFFER_SIZE 1024 * 32
#define XNS_CATALOG_BUFFER_SIZE 1024 * 32

PVARIABLES v;

SOCKET gXNSBaseSocket;
SOCKET gXNSSocket;
HANDLE hFile = INVALID_HANDLE_VALUE;
BYTE * szRingBuffer;
int nRingWrite, nRingRead;
int nRingBytes;
int nRingReset;
int nPriorRead;
BOOL fConnected = FALSE;
CRITICAL_SECTION csRing;
BYTE szSendBuffer[XNS_SEND_BUFFER_SIZE];
BYTE * szCatalogData;
BOOL fSourceBufferThreadActive = FALSE;
//HANDLE hDebug;

void myOutputDebugString(char * szString)
{
	//OutputDebugString(szString);
}

BOOL XNS__Catalog(char * szArgument)
{
	int nItemIndex;
	char szFirstLine[34];

	szCatalogData = LocalAlloc(LPTR, XNS_CATALOG_BUFFER_SIZE);
	lstrcpy(szCatalogData, "<CATALOGUE>");
	for (nItemIndex = 0; nItemIndex < MAX_PAT_ENTRIES; nItemIndex++)
	{
		char szFileName[100];
		if (v->pat.pmt[nItemIndex].nPMTPID == 0)
			break;
		if (v->pat.pmt[nItemIndex].nProgramNumber)
		{
			// Make sure the stream has a video and audio ES
			BOOL fGotAudio = FALSE;
			BOOL fGotVideo = FALSE;
			int i;

			for (i = 0; i < MAX_ESLIST_ENTRIES; i++)
			{
				if (v->pat.pmt[nItemIndex].es[i].nESPID == 0)
					break;
				switch(v->pat.pmt[nItemIndex].es[i].nStreamType)
				{
				case 0x01:		// MPEG-1
				case 0x02:		// MPEG-2
					fGotVideo = TRUE;
					break;
				case 0x03:		// MPEG-1
				case 0x04:		// MPEG-2
				case 0x81:		// AC3
					fGotAudio = TRUE;
					break;
				}
			}

			if ( (!fGotAudio) || (!fGotVideo) )
				continue;

			wsprintf(szFileName, "%05d", v->pat.pmt[nItemIndex].nProgramNumber);
			if (v->pChannelData[v->pat.pmt[nItemIndex].nProgramNumber] != NULL)
			{
				lstrcat(szFileName, " - ");
				lstrcat(szFileName, v->pChannelData[v->pat.pmt[nItemIndex].nProgramNumber]->szShortName);
			}
			lstrcat(szFileName, ".mpg");
			lstrcat(szCatalogData, "<ITEM><ATTRIB>128</ATTRIB><PATH>");
			lstrcat(szCatalogData, szFileName);
			lstrcat(szCatalogData, "</PATH></ITEM>");
		}
	}
	lstrcat(szCatalogData, "</CATALOGUE>");

	wsprintf(szFirstLine, "%d OK", lstrlen(szCatalogData));
	while (lstrlen(szFirstLine) < 32)
		lstrcat(szFirstLine, " ");

	if (send(gXNSSocket, szFirstLine, lstrlen(szFirstLine), 0) != lstrlen(szFirstLine))
		return FALSE;
	if (send(gXNSSocket, szCatalogData, lstrlen(szCatalogData), 0) != lstrlen(szCatalogData))
		return FALSE;
	
	LocalFree(szCatalogData);
	return TRUE;
}

BOOL XNS__OpenFile(char * szArgument)
{
	int i;
	int nOffset;
	int nProgramNumber;
	DWORD dwFileSize;
	char szOKResponse[34];

	if (strstr(szArgument, ".mpg") == NULL)
	{
		char szBadResponse[] = {"-1 ERROR PERMISSION DENIED      "};
		send(gXNSSocket, szBadResponse, lstrlen(szBadResponse), 0);
		return FALSE;
	}
	nOffset = 0;
	while ((szArgument[nOffset] == '/') || (szArgument[nOffset] == ',') )
		nOffset++;
	if (sscanf(szArgument + nOffset, "%d", &nProgramNumber) != 1)
		return FALSE;
	for (i = 0; i < MAX_PAT_ENTRIES; i++)
	{
		if (v->pat.pmt[i].nPMTPID == 0)
			break;
		if (v->pat.pmt[i].nProgramNumber == nProgramNumber)
		{
			if (v->nSelectedProgram != i)
			{
				// Change channel first
				HWND hWndTV = GetDlgItem(v->hDlgSIParser, IDC_SI_TREE);

				TreeView_SelectItem(hWndTV, v->pat.pmt[i].hPMTTreeItem);
				TreeView_SelectSetFirstVisible(hWndTV, v->pat.pmt[i].hPMTTreeItem);
				//PostMessage(v->hDlgSIParser, WM_COMMAND, IDC_SI_PARSER_RECORD, 0);
			}
			break;
		}
	}

	szRingBuffer = LocalAlloc(LPTR, XNS_RING_BUFFER_SIZE);
	nRingWrite =  nRingRead = nRingBytes = 0;
	nRingReset = 0;
	nPriorRead = 0;
	InitializeCriticalSection(&csRing);
	//hDebug = CreateFile("e:\\XNS.mpg", GENERIC_WRITE, 0, (LPSECURITY_ATTRIBUTES)NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, (HANDLE)NULL);
	fConnected = TRUE;

	do
	{
		int nCheckRingBytes;
		EnterCriticalSection(&csRing);
		nCheckRingBytes = nRingBytes;
		LeaveCriticalSection(&csRing);
		if (nCheckRingBytes >= 768 * 1024)
			break;
		Sleep(100);
		{
			char szTemp[100];
			wsprintf(szTemp, "Buffering data for XNS client - %d bytes", nCheckRingBytes);
			SetDlgItemText(v->hDlgSIParser, IDC_STATUS, szTemp);
		}
	} while (TRUE);

	dwFileSize = ((1 * 1024 * 1024 * 1024) << 1);
	dwFileSize -= 1024;
	wsprintf(szOKResponse, "%d OK", dwFileSize);
	while (lstrlen(szOKResponse) < 32)
		lstrcat(szOKResponse, " ");
	if (send(gXNSSocket, szOKResponse, lstrlen(szOKResponse), 0) != lstrlen(szOKResponse))
		return FALSE;

	SetDlgItemText(v->hDlgSIParser, IDC_STATUS, "XNS client now reading data...");	
	return TRUE;
}

BOOL XNS__CloseFile(char * szArgument, BOOL fFromServer)
{
	char szOKResponse[] = {"0 OK                            "};
	int x = lstrlen(szOKResponse);

	//CloseHandle(hDebug);
	fConnected = FALSE;
	if (szRingBuffer != NULL)
		LocalFree(szRingBuffer);
	DeleteCriticalSection(&csRing);

	if (fFromServer)
	{
		if (send(gXNSSocket, szOKResponse, lstrlen(szOKResponse), 0) != lstrlen(szOKResponse))
			return FALSE;
	}

	return TRUE;
}

BOOL XNS__ReadFile(char * szArgument)
{
	int nLength, nOffset;
	int i;
	int nRetVal;
	char szFirstLine[34];

	nRetVal = sscanf(szArgument, ",%d,%d", &nOffset, &nLength);
	if (nRetVal != 2)
	{
		char szTemp[100];
		wsprintf(szTemp, "TSReader_XNS: ***** XNS Arugment Error nRetVal = %d*******\n", nRetVal);
		OutputDebugString(szTemp);
		return FALSE;
	}

	if (nLength > XNS_SEND_BUFFER_SIZE)
	{
		OutputDebugString("TSReader_XNS: ***** XNS_SEND_BUFFER_SIZE to small ********\n");
		return FALSE;
	}

	// Get the buffer we'll use for the send() and wait for the data
	do
	{
		EnterCriticalSection(&csRing);
		if (nRingBytes >= nLength)
			break;
		LeaveCriticalSection(&csRing);
		Sleep(1);
	} while (TRUE);

	if (nOffset != nPriorRead + nLength)
	{
		char szTemp[100];
		
		nRingRead = nOffset;		
		wsprintf(szTemp, "TSReader_XNS: ---Skipping to %d (%s)\n", nOffset, szArgument);
		OutputDebugString(szTemp);
	}
	nPriorRead = nOffset;

	// Transfer data from the ring to the send() buffer
	LeaveCriticalSection(&csRing);
	for (i = 0; i < nLength; i++)
	{
		szSendBuffer[i] = szRingBuffer[nRingRead++];
		//nRingBytes--;
		if (nRingRead == XNS_RING_BUFFER_SIZE)
			nRingRead = 0;
	}
	EnterCriticalSection(&csRing);
	nRingBytes -= nLength;
	LeaveCriticalSection(&csRing);

	wsprintf(szFirstLine, "%d OK", nLength);
	while (lstrlen(szFirstLine) < 32)
		lstrcat(szFirstLine, " ");

	if (send(gXNSSocket, szFirstLine, lstrlen(szFirstLine), 0) != lstrlen(szFirstLine))
	{
		OutputDebugString("TSReader_XNS: F1 ");
		return FALSE;
	}
	if (send(gXNSSocket, szSendBuffer, nLength, 0) != (int)nLength)
	{
		OutputDebugString("TSReader_XNS: F2 ");
		return FALSE;
	}

	return TRUE;
}

BOOL XNS__TellFile(char * szArgument)
{
	return TRUE;
}

DWORD WINAPI XNS__ServerThread(LPVOID lpv)
{
	char szTemp[100];
	SOCKADDR_IN acc_sin;
	int acc_sin_len;

	OutputDebugString("TSReader_XNS: +XNS__ServerThread\n");

	// Start listening
	if (listen(gXNSBaseSocket, 100) < 0)
	{
		OutputDebugString("TSReader_XNS: listen(gXNSBaseSocket) failed\n");
		closesocket(gXNSBaseSocket);
		return 0;
	}

	// Wait for a connection on port 1400
	do
	{
		char szCommandBuffer[100];
		BOOL fDoneHandshake = FALSE;
		int nLength;

		acc_sin_len = sizeof(acc_sin);
		gXNSSocket = accept(gXNSBaseSocket, (struct sockaddr FAR *)&acc_sin, (int FAR *)&acc_sin_len);
		if (gXNSSocket == INVALID_SOCKET)
			break;
		wsprintf(szTemp, "TSReader_XNS: Accepted XNS connection from %d.%d.%d.%d\n", acc_sin.sin_addr.S_un.S_un_b.s_b1,
			                                                            acc_sin.sin_addr.S_un.S_un_b.s_b2,
																		acc_sin.sin_addr.S_un.S_un_b.s_b3,
																		acc_sin.sin_addr.S_un.S_un_b.s_b4);
		OutputDebugString(szTemp);
		{
			int flag;
			flag = 1;
			setsockopt (gXNSSocket, IPPROTO_TCP, TCP_NODELAY, (char *) &flag, sizeof(int));
			//flag = 2048;
			//setsockopt (gXNSSocket, IPPROTO_TCP, TCP_MAXSEG, (char *) &flag, sizeof(int));
		}

		do
		{
			memset(szCommandBuffer, 0, sizeof(szCommandBuffer));
			nLength = recv(gXNSSocket, szCommandBuffer, sizeof(szCommandBuffer), 0);
			if (nLength <= 0)
				break;
			if (   (szCommandBuffer[lstrlen(szCommandBuffer) - 1] == '\n')
				|| (szCommandBuffer[lstrlen(szCommandBuffer) - 1] == '\r') )
				szCommandBuffer[lstrlen(szCommandBuffer) - 1] = '\0';
			/*{
				char szTemp[100];
				wsprintf(szTemp, "TSReader_XNS: XNS RX: nPipeBytes = %d len = %d %s\n", nRingBytes, nLength, szCommandBuffer);
				myOutputDebugString(szTemp);
			}*/
			if (!fDoneHandshake)
			{
				char szGreeting[] = {"HELLO XBOX!"};
				send(gXNSSocket, szGreeting, lstrlen(szGreeting), 0);
				fDoneHandshake = TRUE;
			}
			else
			{
				int nRetVal = TRUE;
				char szCommand[6];

				memcpy(szCommand, szCommandBuffer, 4);
				szCommand[4] = 0;

				if (lstrcmp(szCommand, "*CAT") == 0)
					nRetVal = XNS__Catalog(szCommandBuffer + 4);
				else if (lstrcmp(szCommand, "OPEN") == 0)
					nRetVal = XNS__OpenFile(szCommandBuffer + 4);
				else if (lstrcmp(szCommand, "CLSE") == 0)
					nRetVal = XNS__CloseFile(szCommandBuffer + 4, TRUE);
				else if (lstrcmp(szCommand, "READ") == 0)
					nRetVal = XNS__ReadFile(szCommandBuffer + 4);
				else if (lstrcmp(szCommand, "TELL") == 0)
					nRetVal = XNS__TellFile(szCommandBuffer + 4);
				if (nRetVal == FALSE)
				{
					OutputDebugString("TSReader_XNS: XNS failed on command: ");
					OutputDebugString(szCommandBuffer);
					OutputDebugString("\n");
					break;
				}
			}
		} while (TRUE);

		closesocket(gXNSSocket);
		OutputDebugString("TSReader_XNS: XNS socket closed\n");
	} while (TRUE);

	OutputDebugString("TSReader_XNS: -XNS__ServerThread\n");	
	return 0;
}

DWORD WINAPI StradisDataThread(LPVOID lpv)
{
	DWORD dwRead;
	BYTE * buffer = (BYTE *)LocalAlloc(LPTR, STRADIS_READ_SIZE);
	HANDLE hRecordFile = INVALID_HANDLE_VALUE;

	fSourceBufferThreadActive = TRUE;

	OutputDebugString("TSReader_XNS: +XNSDataThread\n");
	do
	{
		if (ReadFile(v->hStradisReadPipe, buffer, STRADIS_READ_SIZE, &dwRead, NULL) == FALSE)
			break;
		if (fConnected)
		{
			int i;
		
			for (i = 0; i < (int)dwRead; i++)
			{
				szRingBuffer[nRingWrite++] = buffer[i];
				//nRingBytes++;
				if (nRingWrite == XNS_RING_BUFFER_SIZE)
					nRingWrite = 0;
			}
			EnterCriticalSection(&csRing);
			nRingBytes += dwRead;
			while (nRingBytes > (XNS_RING_BUFFER_SIZE / 8) * 7)
			{
				LeaveCriticalSection(&csRing);
				Sleep(10);
				EnterCriticalSection(&csRing);
			}
			LeaveCriticalSection(&csRing);
		}
		EnterCriticalSection(&v->csPipeBytes);
		v->nPipeBytes -= dwRead;
		LeaveCriticalSection(&v->csPipeBytes);
	} while (TRUE);

	LocalFree(buffer);
	fSourceBufferThreadActive = FALSE;
	OutputDebugString("TSReader_XNS: -XNSDataThread\n");
	return 0;
}

BOOL SetupStradis(PVARIABLES pv)
{
	HANDLE hThread;
	DWORD dwThreadID;

	OutputDebugString("TSReader_XNS: SetupStradis()\n");
	v = pv;		// save for us
	fConnected = FALSE;
	
	// Get a thread going to read the data written by the main
	// parser thread
	hThread = CreateThread(NULL, 0, StradisDataThread, (LPVOID)0, 0, &dwThreadID);
	CloseHandle(hThread);
			
	return TRUE;
}

void ShutdownStradis()
{
	OutputDebugString("TSReader_XNS: ShutdownStradis()\n");
}

BOOL SetupSocket(PVARIABLES pv)
{
	HANDLE hThread;
	DWORD dwThreadID;
	SOCKADDR_IN local_sin;

	v = pv;		// save for us

	// Make sure we can bind - if not we have another XNS server so we will then
	// disable the XNS interface
	gXNSBaseSocket = socket(AF_INET, SOCK_STREAM, 0);

	// Setup for TCP/IP on port 1400
	local_sin.sin_family = AF_INET;
	local_sin.sin_addr.s_addr = INADDR_ANY;
	local_sin.sin_port = htons((short)v->nXNSServerPort);

	// Bind the socket
	if (bind(gXNSBaseSocket, (struct sockaddr FAR *) &local_sin, sizeof(local_sin)) == SOCKET_ERROR)
	{
		closesocket(gXNSBaseSocket);
		return FALSE;
	}

	// Thread to accept and manage the XNS socket connection
	hThread = CreateThread(NULL, 0, XNS__ServerThread, (LPVOID)0, 0, &dwThreadID);
	CloseHandle(hThread);

	return TRUE;
}

void ShutdownSocket(PVARIABLES pv)
{	
	if (pv->fXNSInterface)
	{
		closesocket(gXNSBaseSocket);
		closesocket(gXNSSocket);
		Sleep(500);
		XNS__CloseFile(NULL, FALSE);
	}
	pv->fXNSTerminated = TRUE;
}
