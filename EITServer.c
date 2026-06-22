#include <windows.h>
#include <commctrl.h>
#include <limits.h>
#include "TSReader.h"

extern PVARIABLES v;

BOOL SendRemoteEPGData(int nConnectionNumber, char * szBuffer)
{
	return (send(v->EITConnection[nConnectionNumber].socket, szBuffer, lstrlen(szBuffer), 0) == lstrlen(szBuffer));
}

PEITEVENT LocateCurrentProgramFromEPG(int nServiceID)
{
	PEITEVENT pCurrent;
	PEITEVENT pTargetEvent = NULL;
	SYSTEMTIME stSystemTime;

	pCurrent = v->pEvents[nServiceID];
	if (pCurrent != NULL)
	{
		FILETIME ftProgramStart, ftNow;
		DWORD64 lnProgramStart, lnNow;
		DWORD64 lnMultiplier = 10000000;

		GetSystemTime(&stSystemTime);
		stSystemTime.wMilliseconds = 0;
		SystemTimeToFileTime(&stSystemTime, &ftNow);
		memcpy(&lnNow, &ftNow, sizeof(DWORD64));

		do
		{
			DWORD64 lnRunTime = ( (pCurrent->stRunTime.wHour * 60 * 60)
									   + (pCurrent->stRunTime.wMinute * 60)
									   + (pCurrent->stRunTime.wSecond) ) * lnMultiplier;

			SystemTimeToFileTime(&pCurrent->stStartTime, &ftProgramStart);
			memcpy(&lnProgramStart, &ftProgramStart, sizeof(DWORD64));

			if (lnNow >= lnProgramStart && lnNow <= lnProgramStart + lnRunTime)
				pTargetEvent = pCurrent;
			pCurrent = (PEITEVENT)pCurrent->dwNextEvent;
		} while (pCurrent != NULL);
		return pTargetEvent;
	}
	return NULL;
}

DWORD WINAPI EITServerThread(LPVOID pvarg)
{
	int acc_sin_len;
	int nConnectionNumber;
	int i;
	int nKeepAliveCounter;
	DWORD ThreadID;
	HANDLE hThread;
	SYSTEMTIME stPrior;
	char szChannel[8];
	char szTemp[256];

	stPrior.wYear = 0;

	// find a free connection slot
	EnterCriticalSection(&v->csEITConnection);
	for (nConnectionNumber = 0; nConnectionNumber < MAX_EIT_CONNECTIONS; nConnectionNumber++)
	{
		if (v->EITConnection[nConnectionNumber].socket == INVALID_SOCKET)
			break;
	}
	if (nConnectionNumber == MAX_EIT_CONNECTIONS)
		return (DWORD)-1;
	v->EITConnection[nConnectionNumber].fAbort = FALSE;
	v->EITConnection[nConnectionNumber].nCurrentEventID = -1;

	// wait until we get a connection
	acc_sin_len = sizeof(v->EITConnection[nConnectionNumber].acc_sin);
	LeaveCriticalSection(&v->csEITConnection);
	v->EITConnection[nConnectionNumber].socket = accept(v->EITBaseSocket, (struct sockaddr FAR *)&v->EITConnection[nConnectionNumber].acc_sin, (int FAR *)&acc_sin_len);
	if (v->EITConnection[nConnectionNumber].socket == INVALID_SOCKET)
		return 0;

	// spawn another thread to handle the next connection
	hThread = CreateThread(NULL, 0, EITServerThread, 0, 0, &ThreadID);
	CloseHandle(hThread);

	// read five bytes of channel number from this socket
	for (i = 0; i < 5; i++)
		recv(v->EITConnection[nConnectionNumber].socket, &szChannel[i], 1, 0);
	szChannel[i] = '\0';
	sscanf(szChannel, "%d", &v->EITConnection[nConnectionNumber].nChannel);
	if ( (v->EITConnection[nConnectionNumber].nChannel < 0) || (v->EITConnection[nConnectionNumber].nChannel >= MAX_EIT_CHANNEL_DATA) )
	{
		closesocket(v->EITConnection[nConnectionNumber].socket);
		v->EITConnection[nConnectionNumber].socket = INVALID_SOCKET;
		return 0;
	}

	if (v->pEvents[v->EITConnection[nConnectionNumber].nChannel] == NULL)
	{
		closesocket(v->EITConnection[nConnectionNumber].socket);
		v->EITConnection[nConnectionNumber].socket = INVALID_SOCKET;
		return 0;
	}

	nKeepAliveCounter = 0;
	do
	{
		PEITEVENT pCurrent = LocateCurrentProgramFromEPG(v->EITConnection[nConnectionNumber].nChannel);
		if (pCurrent == NULL)
			break;
		if ( (stPrior.wYear == 0)
			|| (memcmp(&pCurrent->stStartTime, &stPrior, sizeof(SYSTEMTIME)) != 0) )
		{
			memcpy(&stPrior, &pCurrent->stStartTime, sizeof(SYSTEMTIME));
			SendRemoteEPGData(nConnectionNumber, v->pChannelData[v->EITConnection[nConnectionNumber].nChannel]->szShortName);
			SendRemoteEPGData(nConnectionNumber, "\r\n");
			
			if (SendRemoteEPGData(nConnectionNumber, pCurrent->szEventName) == FALSE)
				break; 
			if (SendRemoteEPGData(nConnectionNumber, "\r\n") == FALSE)
				break; 
			if (SendRemoteEPGData(nConnectionNumber, pCurrent->szShortEventDescription) == FALSE)
				break; 
			if (SendRemoteEPGData(nConnectionNumber, "\r\n") == FALSE)
				break; 
			wsprintf(szTemp, "%04d/%02d/%02d %02d:%02d:%02d\r\n",
				pCurrent->stStartTime.wYear,
				pCurrent->stStartTime.wMonth,
				pCurrent->stStartTime.wDay,
				pCurrent->stStartTime.wHour,
				pCurrent->stStartTime.wMinute,
				pCurrent->stStartTime.wSecond);
			if (SendRemoteEPGData(nConnectionNumber, szTemp) == FALSE)
				break;
			wsprintf(szTemp, "%02d:%02d:%02d\r\n",
				pCurrent->stRunTime.wHour,
				pCurrent->stRunTime.wMinute,
				pCurrent->stRunTime.wSecond);
			if (SendRemoteEPGData(nConnectionNumber, szTemp) == FALSE)
				break;
			Sleep(1000);		// snooze so we don't send this event again
		}

		// Snooze 1/2 second and then send a keep alive
		Sleep(250);
		if (nKeepAliveCounter++ >= 10)
		{
			if (send(v->EITConnection[nConnectionNumber].socket, "\0", 1, 0) != 1)
				break;
			nKeepAliveCounter = 0;
		}
	} while (TRUE);

	closesocket(v->EITConnection[nConnectionNumber].socket);
	memset(&v->EITConnection[nConnectionNumber], 0, sizeof(EITCONNECTION));
	v->EITConnection[nConnectionNumber].socket = INVALID_SOCKET;

	return 0;
}

BOOL StartEITServer(void)
{
	HANDLE hThread;
	DWORD dwThreadID;
	int nStat;
	int i;
	SOCKADDR_IN local_sin;
	WSADATA WSAData;
	char szTemp[128];

	for (i = 0; i < MAX_EIT_CONNECTIONS; i++)
	{
		memset(&v->EITConnection[i], 0, sizeof(EITCONNECTION));
		v->EITConnection[i].socket = INVALID_SOCKET;
	}

	if ((nStat = WSAStartup(MAKEWORD(1,1), &WSAData)) != 0)
	{
		OutputDebugString("EITServer: WSAStartup failed\n");
		return FALSE;
	}

	// Create a socket for incoming connections
	v->EITBaseSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (v->EITBaseSocket == INVALID_SOCKET)
	{
		OutputDebugString("EITServer: socket() failed\n");
		return FALSE;
	}

	// Setup for the telnet port
	local_sin.sin_family = AF_INET;
	local_sin.sin_addr.s_addr = INADDR_ANY;
	//dwDotted = inet_addr(g_szBindAddr);
	//memcpy((char FAR *)&(local_sin.sin_addr), (char FAR *)&dwDotted, 4);
	local_sin.sin_port = htons((unsigned short)v->nEITServerPort);

	// Bind the socket
	if (bind(v->EITBaseSocket, (struct sockaddr FAR *) &local_sin, sizeof(local_sin)) == SOCKET_ERROR)
	{
		wsprintf(szTemp, "EITServer: bind() failed with %d\n", WSAGetLastError());
		OutputDebugString(szTemp);
		closesocket((unsigned short)v->EITBaseSocket);
		return FALSE;
	}

	// Start listening
	if (listen(v->EITBaseSocket, 100) < 0)
	{
		wsprintf(szTemp, "EITServer: listen() failed with %d\n", WSAGetLastError());
		OutputDebugString(szTemp);
		closesocket(v->EITBaseSocket);
		return FALSE;
	}
	
	InitializeCriticalSection(&v->csEITConnection);
	v->fEITServerInitialized = TRUE;
	hThread = CreateThread(NULL, 0, EITServerThread, 0, 0, &dwThreadID);
	CloseHandle(hThread);
	return TRUE;
}

int GetEITConnectionCount(void)
{
	int nConnectionCount = 0;
	int nConnectionNumber;

	if (v->fEITServerInitialized)
	{
		EnterCriticalSection(&v->csEITConnection);
		for (nConnectionNumber = 0; nConnectionNumber < MAX_EIT_CONNECTIONS; nConnectionNumber++)
		{
			if (v->EITConnection[nConnectionNumber].socket != INVALID_SOCKET)
				nConnectionCount++;
		}
		LeaveCriticalSection(&v->csEITConnection);
	}

	return nConnectionCount;
}

BOOL TerminateEITServer(void)
{
	if (v->fEITServerInitialized)
	{
		closesocket(v->EITBaseSocket);
		Sleep(1000);
		DeleteCriticalSection(&v->csEITConnection);
		v->fEITServerInitialized = FALSE;
	}
	return TRUE;
}
