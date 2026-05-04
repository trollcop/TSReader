#include <windows.h>
#include <commctrl.h>

#include "../TSReader.h"
#include "../resource.h"

#define STRADIS_READ_SIZE 384 * 188
#define MAX_CONNECTIONS 4

SOCKET gHTTPSocket[MAX_CONNECTIONS];
SOCKET gHTTPBaseSocket;
PVARIABLES v;
BOOL fReadyForData[MAX_CONNECTIONS];
BOOL fSourceBufferThreadActive;
BOOL fServerThreadActive;
BOOL fAbort;

DWORD WINAPI StradisDataThread(LPVOID lpv)
{
	DWORD dwRead;
	BYTE * buffer = (BYTE *)LocalAlloc(LPTR, STRADIS_READ_SIZE);
	HANDLE hRecordFile = INVALID_HANDLE_VALUE;

	fSourceBufferThreadActive = TRUE;

	OutputDebugString("TSReader_VLC: +HTTPDataThread\n");
	do
	{
		int nSocketIndex;
		int nSocketConnections = 0;

		for (nSocketIndex = 0; nSocketIndex < MAX_CONNECTIONS; nSocketIndex++)
		{
			if (gHTTPSocket[nSocketIndex] != NULL)
				nSocketConnections++;
		}
		if (ReadFile(v->hStradisReadPipe, buffer, STRADIS_READ_SIZE, &dwRead, NULL) == FALSE)
			break;
		for (nSocketIndex = 0; nSocketIndex < MAX_CONNECTIONS; nSocketIndex++)
		{
			if (gHTTPSocket[nSocketIndex] != NULL)
			{
				if (fReadyForData[nSocketIndex] == TRUE)
				{
					if (nSocketConnections > 1)
					{
						fd_set ReadFDset;
						fd_set WriteFDset;
						fd_set ExceptFDset;
						TIMEVAL timeout;

						timeout.tv_sec = timeout.tv_usec = 0;

						FD_ZERO(&ReadFDset);
						FD_ZERO(&WriteFDset);
						FD_ZERO(&ExceptFDset);
						FD_SET(gHTTPSocket[nSocketIndex], &WriteFDset);
						if (select(0, &ReadFDset, &WriteFDset, &ExceptFDset, &timeout) == 0)
							continue;	// socket is busy
					}

					if (send(gHTTPSocket[nSocketIndex], (char *)buffer, dwRead, 0) != (int)dwRead)
					{
						char szTemp[128];
						wsprintf(szTemp, "TSReader_VLC: Terminated socket #%d\n", nSocketIndex);
						OutputDebugString(szTemp);
						closesocket(gHTTPSocket[nSocketIndex]);
						gHTTPSocket[nSocketIndex] = NULL;
						fReadyForData[nSocketIndex] = FALSE;					
					}
				}
			}
		}
		EnterCriticalSection(&v->csPipeBytes);
		v->nPipeBytes -= dwRead;
		LeaveCriticalSection(&v->csPipeBytes);
	} while (TRUE);

	LocalFree(buffer);
	fSourceBufferThreadActive = FALSE;
	OutputDebugString("TSReader_VLC: -HTTPDataThread\n");
	return 0;
}

DWORD WINAPI HTTP__ServerConnectionThread(LPVOID lpv)
{
	int acc_sin_len;
	int nSocket;
	SOCKADDR_IN acc_sin;
	SOCKADDR_IN local_sin;
	char szTemp[512];

	OutputDebugString("TSReader_VLC: +HTTP__ServerConnectionThread\n");

	// Make sure we can bind - if not we have another HTTP server so we will then
	// disable the HTTP interface
	gHTTPBaseSocket = socket(AF_INET, SOCK_STREAM, 0);

	// Setup for TCP/IP on port 1400
	local_sin.sin_family = AF_INET;
	local_sin.sin_addr.s_addr = INADDR_ANY;
	local_sin.sin_port = htons((short)v->nVLCPort);

	// Bind the socket
	if (bind(gHTTPBaseSocket, (struct sockaddr FAR *) &local_sin, sizeof(local_sin)) == SOCKET_ERROR)
	{
		wsprintf(szTemp, "TSReader_VLC: Unable to bind HTTP socket GLE = %d\n", WSAGetLastError());
		OutputDebugString(szTemp);
		closesocket(gHTTPBaseSocket);
		return FALSE;
	}

	// Start listening
	if (listen(gHTTPBaseSocket, SOMAXCONN) < 0)
	{
		wsprintf(szTemp, "TSReader_VLC: listen(gHTTPBaseSocket) failed with GLE = %d\n", WSAGetLastError());
		OutputDebugString(szTemp);
		closesocket(gHTTPBaseSocket);
		return 0;
	}

	// Now wait for a connection
	fServerThreadActive = TRUE;
	do
	{
		int nSocketIndex;

		for (nSocketIndex = 0; nSocketIndex < MAX_CONNECTIONS; nSocketIndex++)
		{
			if (gHTTPSocket[nSocketIndex] == NULL)
				break;
		}
		if (nSocketIndex == MAX_CONNECTIONS)
		{
			OutputDebugString("TSReader_VLC: Out of connections!\n");
			break;
		}
		fReadyForData[nSocketIndex] = FALSE;
		acc_sin_len = sizeof(acc_sin);
		gHTTPSocket[nSocketIndex] = accept(gHTTPBaseSocket, (struct sockaddr FAR *)&acc_sin, (int FAR *)&acc_sin_len);
		if (gHTTPSocket[nSocketIndex] == INVALID_SOCKET)
		{
			wsprintf(szTemp, "TSReader_VLC: accept failed in VLC server with GLE = %d\n", WSAGetLastError());
			OutputDebugString(szTemp);
			break;
		}
		wsprintf(szTemp, "TSReader_VLC: Accepted HTTP connection from %d.%d.%d.%d on socket #%d\n", acc_sin.sin_addr.S_un.S_un_b.s_b1,
			                                                            acc_sin.sin_addr.S_un.S_un_b.s_b2,
																		acc_sin.sin_addr.S_un.S_un_b.s_b3,
																		acc_sin.sin_addr.S_un.S_un_b.s_b4, nSocketIndex);
		OutputDebugString(szTemp);
		{
			int flag;
			int nRet;
			int nTransmitUDPBufferSize = 256 * 1024;

			flag = 1;
			setsockopt (gHTTPSocket[nSocketIndex], IPPROTO_TCP, TCP_NODELAY, (char *) &flag, sizeof(int));
			nRet = setsockopt(gHTTPSocket[nSocketIndex], SOL_SOCKET, SO_SNDBUF, (char *)&nTransmitUDPBufferSize, sizeof(nTransmitUDPBufferSize));
			if (nRet == SOCKET_ERROR)
			{
				OutputDebugString("TSReader_VLC: setsockopt(SO_SNDBUF) failed\n");
			}
		}
		{
			int nLength;
			char szCommandBuffer[512];

			// Get the GET request from the client
			nLength = recv(gHTTPSocket[nSocketIndex], szCommandBuffer, sizeof(szCommandBuffer), 0);
			if (nLength <= 0)
			{
				OutputDebugString("TSReader_VLC: recv() for HTTP protocol failed\n");
				break;
			}
			
			// Send the HTTP response
			if (v->fSendBogusHTTPSize == TRUE)
				wsprintf(szCommandBuffer, "HTTP/1.0 200 OK\r\nContent-Length: %d\r\nCache-Control: no-cache\r\n\r\n",
					1024 * 1024 * 1024);
			else
				wsprintf(szCommandBuffer, "HTTP/1.0 200 OK\r\nContent-type: %s\r\nCache-Control: no-cache\r\n\r\n", v->szHTTPContentType);

			if (send(gHTTPSocket[nSocketIndex], szCommandBuffer, lstrlen(szCommandBuffer), 0) != (int)lstrlen(szCommandBuffer))
			{
				wsprintf(szTemp, "TSReader_VLC: F2 fails with GLE = %d\n", WSAGetLastError());
				OutputDebugString(szTemp);
				closesocket(gHTTPSocket[nSocketIndex]);
				gHTTPSocket[nSocketIndex] = NULL;
			}
			else
				fReadyForData[nSocketIndex] = TRUE;
		}
	} while (TRUE);

	for (nSocket = 0; nSocket < MAX_CONNECTIONS; nSocket++)
	{
		if (gHTTPSocket[nSocket] != NULL)
			closesocket(gHTTPSocket[nSocket]);
	}

	fServerThreadActive = FALSE;
	OutputDebugString("TSReader_VLC: -HTTP__ServerConnectionThread\n");	
	return 0;
}


BOOL SetupStradis(PVARIABLES pv)
{
	HANDLE hThread;
	DWORD dwThreadID;
	int i;

	v = pv;		// save for us

	for (i = 0; i < MAX_CONNECTIONS; i++)
	{
		fReadyForData[i] = FALSE;
		gHTTPSocket[i] = NULL;
	}

	fSourceBufferThreadActive = FALSE;
	fServerThreadActive = FALSE;
	fAbort = FALSE;

	// Thread to accept and manage the HTTP socket connection
	hThread = CreateThread(NULL, 0, HTTP__ServerConnectionThread, (LPVOID)0, 0, &dwThreadID);
	CloseHandle(hThread);

	// Get a thread going to read the data written by the main
	// parser thread
	hThread = CreateThread(NULL, 0, StradisDataThread, (LPVOID)0, 0, &dwThreadID);
	CloseHandle(hThread);
				
	return TRUE;
}

void ShutdownStradis()
{
	int nCount = 0;

	// Wait till the pipe read thread aborts
	// this is because TSReader closed it's pipe
	do
	{
		if (fSourceBufferThreadActive == FALSE)
			break;
		Sleep(50);
	} while (nCount++ < 300);
	OutputDebugString("TSReader_VLC: fSourceBufferThreadActive == FALSE\n");

	// Terminate the socket thread
	Sleep(500);
	nCount = 0;
	fAbort = TRUE;
	closesocket(gHTTPBaseSocket);
	do
	{
		if (fServerThreadActive == FALSE)
			break;
		Sleep(50);
	} while (nCount++ < 300);
	OutputDebugString("TSReader_VLC: fServerThreadActive == FALSE\n");

	
}


