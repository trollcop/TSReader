#include <windows.h>
#include <winsock.h>
#include <commctrl.h>
#include <setupapi.h>
#include <initguid.h>
#include <stdio.h>

#include "..\sources.h"

PSOURCESTRUCT ss;
BOOL fNeedTuneDialog = TRUE;
SOCKET sock;  
struct sockaddr_in server;

char szTCPAddress[128];
int nTCPPort;

#define DEBUG_NAME "TCP:"
char gszSourceName[] = {"TCP/IP"};
char gszLastTune[128] = {"n/a"};

BOOL __cdecl SourceHelper_ValidateSourceContainer(PSOURCESTRUCT pss);

int nTSBufferIndex;

void WriteMPEGTSData(BYTE * pBuffer, int nSize)
{
	SourceHelper_SyncData(pBuffer, nSize);
}

DWORD WINAPI ReadTCPThread(LPVOID lpv)
{
	int nTSBufferIndex = 0;
	int nRemaining;
	int nOffset;
	BYTE bBuffer[64 * 1024];

	OutputDebugString(DEBUG_NAME" ReadTCPThread+\n");

	SourceHelper_StartSyncThread(ss, FALSE);

	nTSBufferIndex = 0;
	ss->fReadThreadTerminated = FALSE;
	ss->fTerminateReadThread = FALSE;

	nRemaining = sizeof(bBuffer);
	nOffset = 0;
	while (!ss->fTerminateReadThread)
	{
		int nrecvRetVal = recv(sock, (char *)&bBuffer, sizeof(bBuffer), 0);
		if ( (nrecvRetVal == 0) || (nrecvRetVal == SOCKET_ERROR) )
			break;

		WriteMPEGTSData(bBuffer, nrecvRetVal);
	}

	SourceHelper_StopSyncThread();
	CloseHandle(ss->hReadDataThread);
	ss->fReadThreadTerminated = TRUE;
	ss->fTerminateReadThread = FALSE;
	OutputDebugString(DEBUG_NAME" ReadTCPThread-\n");
	return 0;
}

BOOL TSReader_Start()
{
	DWORD dwThreadID;
	int nReceiveUDPBufferSize = 256 * 1024;

/*
	nRet = setsockopt(sock, SOL_SOCKET, SO_RCVBUF, (char *)&nReceiveUDPBufferSize, sizeof(nReceiveUDPBufferSize));
	if (nRet == SOCKET_ERROR)
	{
		OutputDebugString("setsockopt(SO_RCVBUF) failed\n");
		return FALSE;
	}
*/
	ss->hReadDataThread = CreateThread(NULL, 0, ReadTCPThread, (LPVOID)0, CREATE_SUSPENDED, &dwThreadID);
	SourceHelper_SetWorkerThreadPriorities(FALSE);
	ResumeThread(ss->hReadDataThread);

	return TRUE;
}

BOOL TSReader_Stop()
{
	OutputDebugString(DEBUG_NAME" Wait for read thread terminate\n");
	ss->fTerminateReadThread = TRUE;
	
	closesocket(sock);

	while (ss->fReadThreadTerminated == FALSE)
		Sleep(50);

	EnterCriticalSection(&ss->csTSBuffersInUse);
	ss->nTSBuffersInUse = -1000;
	LeaveCriticalSection(&ss->csTSBuffersInUse);

	OutputDebugString(DEBUG_NAME" TSReader_Stop() complete\n");
	return TRUE;
}

BOOL TSReader_Init(PSOURCESTRUCT pss)
{
	WSADATA WSAData;
	
	if (SourceHelper_ValidateSourceContainer(pss) == FALSE)
		return FALSE;

	ss = pss;	
	WSAStartup(0x202, &WSAData);

	return TRUE;
}


BOOL TSReader_DeInit()
{
	return TRUE;
}

BOOL TSReader_Tune()
{
	int nStat;
	SOCKADDR_IN sin;
	int nConvertCount;
	int nTemp;
	unsigned long dwDotted;
	HOSTENT	phe;
	LPHOSTENT pHostent;

	wsprintf(gszLastTune, "%s port %d", ss->szUDPMulticastAddress, ss->nUDPMulticastPort);

	sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock == INVALID_SOCKET)
		return FALSE;
	
	pHostent = &phe;
	sin.sin_family = AF_INET;

	// See if this is a dotted notation
	nConvertCount = sscanf(ss->szUDPMulticastAddress, TEXT("%d.%d.%d.%d"),
						   &nTemp,
						   &nTemp,
						   &nTemp,
						   &nTemp);
	if (nConvertCount == 4)
	{
		// Is an IP address, use without name lookup
		dwDotted = inet_addr(ss->szUDPMulticastAddress);
		memcpy((char FAR *)&(sin.sin_addr), (char FAR *)&dwDotted, 4);
	}
	else
	{
		// Not an IP address, do name lookup
		pHostent = gethostbyname(ss->szUDPMulticastAddress);
		if (pHostent == NULL)
		{
			//"Couldn't resolve Host Name"
			return FALSE;
		}
		memcpy((char FAR *)&(sin.sin_addr), pHostent->h_addr, pHostent->h_length);
	}
	
	//Setup port to connect to
	sin.sin_port = htons(ss->nUDPMulticastPort);

	// Now connect
	nStat = connect(sock, (PSOCKADDR)&sin, sizeof(sin));
	if (nStat == SOCKET_ERROR)
	{
		MessageBox(ss->hWndTSReader, "Failed to connect to socket", gszSourceName, MB_ICONWARNING);
		return FALSE;
	}
	return TRUE;
}

BOOL TSReader_PIDManagement(BOOL fAdd, int nPID, BOOL fTemporary)
{
	return TRUE;
}

BOOL TSReader_GetDescription(char * szDescription, char * szCommandLineParameters, BOOL * fCanBeStopped, int * nMaxPIDs, DWORD * dwCapabilities)
{
	if (szDescription != NULL)
		lstrcpy(szDescription, gszSourceName);
	if (szCommandLineParameters != NULL)
		lstrcpy(szCommandLineParameters, "ipaddr port");
	if (dwCapabilities != NULL)
		*dwCapabilities = 0;
	if (fCanBeStopped != NULL)
		*fCanBeStopped = FALSE;
	if (nMaxPIDs != NULL)
		*nMaxPIDs = 8192;
	return TRUE;
}

BOOL TSReader_TuneDialog(HWND hWnd)
{
	OutputDebugString(DEBUG_NAME" TSReader_TuneDialog\n");

	if (ss->fDontTune == TRUE)
		fNeedTuneDialog = FALSE;

	if (fNeedTuneDialog)
	{
		if (SourceHelper_TCPTuneDialog(hWnd) == FALSE)
			return FALSE;
	}
	else
	{
		ss->nUDPMulticastPort = nTCPPort;
		lstrcpy(ss->szUDPMulticastAddress, szTCPAddress);
	}

	return TRUE;
}

BOOL TSReader_ParseCommandLine(PSOURCESTRUCT ss, char * szCommandLine, BOOL fQuiet)
{
	if (lstrlen(szCommandLine))
	{
		int nConversionCount = 0;
		int nTemp;
		DWORD dwDotted;

		nConversionCount = sscanf(szCommandLine,
								  "%d.%d.%d.%d %d", 
								  &nTemp,
								  &nTemp,
								  &nTemp,
								  &nTemp,
								  &nTCPPort);
		if (nConversionCount < 5)
		{
			if (!fQuiet)
				MessageBox(NULL,
					   "Usage for this source: ipaddr port\n"
					   "\n"
					   "ipaddr = dotted decimal IP address\n"
					   "port = TCP port to connect to\n",
					   gszSourceName,
					   MB_OK | MB_ICONSTOP);
			return FALSE;
		}
		dwDotted = inet_addr(szCommandLine);
		wsprintf(szTCPAddress, "%d.%d.%d.%d", (dwDotted) & 0xff, (dwDotted >> 8) & 0xff, (dwDotted >> 16) & 0xff, (dwDotted >> 24) & 0xff);
		fNeedTuneDialog = FALSE;
	}
	else
		fNeedTuneDialog = TRUE;

	return TRUE;
}

BOOL TSReader_IsPIDActive(int nPID)
{
	return TRUE;
}

BOOL TSReader_SetChannel(int nChannel)
{
	return TRUE;
}

BOOL TSReader_GetSignalString(char * szString)
{
	lstrcpy(szString, "n/a");
	return TRUE;
}

BOOL TSReader_GetTunerString(char * szString)
{
	lstrcpy(szString, gszLastTune);
	return TRUE;
}

BOOL TSReader_SendDiSEqC(BYTE * bCommand, int nLength)
{
	return TRUE;
}

int TSReader_GetSyncLossCount(BOOL fReset)
{
	return SourceHelper_GetSyncLossCount(fReset);
}
