#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdio.h>
//#include <windows.h>
//#include <commctrl.h>
//#include <setupapi.h>
//#include <initguid.h>

#include "..\sources.h"

#ifndef UNICAST
//#ifndef IP_ADD_SOURCE_MEMBERSHIP
#define IP_ADD_SOURCE_MEMBERSHIP 15
#define IP_DROP_SOURCE_MEMBERSHIP 16
//#endif
struct ip_mreq_source {
    struct in_addr imr_multiaddr;   // IP multicast address of group
    struct in_addr imr_sourceaddr;  // IP address of source
    struct in_addr imr_interface;   // local IP address of interface
};
#endif UNICAST

PSOURCESTRUCT ss;
BOOL fNeedTuneDialog = TRUE;
SOCKET sock;  
struct sockaddr_in server;
#ifndef UNICAST
char szUDPMulticastAddress[MAX_PATH] = {0};
char szUDPMulticastInterface[MAX_PATH] = {0};
#endif UNICAST
int nUDPMulticastPort = 0;

//#define DEBUG_FILE
#ifdef DEBUG_FILE
HANDLE hDebugFile;
#endif DEBUG_FILE

#ifndef RTP
#define DEBUG_NAME "UDP:"
 #ifndef UNICAST
  char gszSourceName[] = {"Multicast UDP"};
 #else UNICAST
  char gszSourceName[] = {"Unicast UDP"};
 #endif UNICAST
#else RTP
#ifndef HRTP
 #define DEBUG_NAME "RTP:"
  #ifndef UNICAST
   char gszSourceName[] = {"Multicast RTP"};
  #else UNICAST
   char gszSourceName[] = {"Unicast RTP"};
  #endif UNICAST
 #else HRTP
 #define DEBUG_NAME "HRTP:"
  #ifndef UNICAST
   char gszSourceName[] = {"Multicast HRTP"};
  #else UNICAST
   char gszSourceName[] = {"Unicast HRTP"};
  #endif UNICAST
 #endif HRTP
#endif RTP
char gszLastTune[128] = {"n/a"};
char gszKeyName[] = {"Software\\COOL.STF\\TSReader\\UDPSource"};

BOOL __cdecl SourceHelper_ValidateSourceContainer(PSOURCESTRUCT pss);

#define RTP_HEADER_LEN 12

int nTSBufferIndex;
BOOL fStraightThrough = FALSE;

void WriteMPEGTSData(BYTE * pBuffer, int nSize)
{
	if (!fStraightThrough)
		SourceHelper_SyncData(pBuffer, nSize);
	else
	{
		ss->tsb[nTSBufferIndex].nSize = nSize;
		memcpy(ss->tsb[nTSBufferIndex].pData, pBuffer, nSize);
		if (ss->fTimestampPackets)
		{
			int nPacket;
			int nPackets = nSize / 188;
			LARGE_INTEGER count;

			QueryPerformanceCounter(&count);
			for (nPacket = 0; nPacket < nPackets; nPacket++)
			{
				if (ss->tsb[nTSBufferIndex].pTimestamps != NULL)
					ss->tsb[nTSBufferIndex].pTimestamps[nPacket] = count.LowPart;
			}
		}

		EnterCriticalSection(&ss->csPIDCounter);
		ss->nLastSecondByteCounter += ss->tsb[nTSBufferIndex].nSize;
		LeaveCriticalSection(&ss->csPIDCounter);
		nTSBufferIndex++;
		if (nTSBufferIndex == MAX_TS_BUFFERS)
			nTSBufferIndex = 0;
		EnterCriticalSection(&ss->csTSBuffersInUse);
		ss->nTSBuffersInUse++;
		LeaveCriticalSection(&ss->csTSBuffersInUse);
		ss->tsb[nTSBufferIndex].nSize = 0;
	}
}

DWORD WINAPI ReadUDPThread(LPVOID lpv)
{
	int nTSBufferIndex = 0;
	int nRemaining;
	int nOffset;
	BYTE bBuffer[64 * 1024];

	OutputDebugString(DEBUG_NAME" ReadUDPThread+\n");

	if (!fStraightThrough)
		SourceHelper_StartSyncThread(ss, FALSE);

	nTSBufferIndex = 0;
	ss->fReadThreadTerminated = FALSE;
	ss->fTerminateReadThread = FALSE;

	nRemaining = sizeof(bBuffer);
	nOffset = 0;
	while (!ss->fTerminateReadThread)
	{
		int structlength = sizeof(server);
		int recvd = recvfrom(sock, &bBuffer[nOffset], nRemaining, 0, (struct sockaddr *) &server, &structlength);
		if (recvd < 0)
			break;
#ifdef DEBUG_FILE
		{
			DWORD dwWritten;
			WriteFile(hDebugFile, &bBuffer[nOffset], recvd, &dwWritten, NULL);
		}
#endif DEBUG_FILE
#ifdef RTP
		{
			int nRTPVersion;
			int nCSRCCount;
			int nPayloadType;
			int nSkip;
#ifdef HRTP
			int nExtended;
			int nExtlen;
#endif HRTP

			// Parse the header and check for MPEG-2 TS
			nRTPVersion  = (bBuffer[0] & 0xC0) >> 6;
			nCSRCCount   = (bBuffer[0] & 0x0F);
			nPayloadType = (bBuffer[1] & 0x7F);
			if (nRTPVersion != 2)
				continue;
			if (nPayloadType != 0x21)
				continue;

			// A CSRC extension field is 32 bits in size (4 bytes) 
			nSkip = RTP_HEADER_LEN + (4 * nCSRCCount);

#ifdef HRTP
			nExtended    = (bBuffer[0] & 0x10) >> 4;
			nExtlen		 = (bBuffer[15]);
			if (nExtended)
			{
				nExtlen *= 4;
				nSkip += nExtlen + 4;
			}
#endif HRTP			
			if (recvd < nSkip)
				continue; // Packet is not big enough

			// Write the packet without the RTP header. 
			WriteMPEGTSData(&bBuffer[nSkip], recvd - nSkip);
		}
#else RTP

		WriteMPEGTSData(bBuffer, recvd);
/*
		nRemaining -= recvd;
		nOffset += recvd;
		if (nOffset + recvd >= sizeof(bBuffer))
		{
			SourceHelper_SyncData(bBuffer, nOffset);
			nRemaining = sizeof(bBuffer);
			nOffset = 0;
		}
		*/
#endif RTP
	}

	if (!fStraightThrough)
		SourceHelper_StopSyncThread();
	CloseHandle(ss->hReadDataThread);
	ss->fReadThreadTerminated = TRUE;
	ss->fTerminateReadThread = FALSE;
	OutputDebugString(DEBUG_NAME" ReadUDPThread-\n");
	return 0;
}

BOOL TSReader_Start()
{
	DWORD dwThreadID;
	int structlength; // Length of sockaddr structure 
	int nRet = SOCKET_ERROR;
	BOOL fFlag = TRUE;
	int nReceiveUDPBufferSize = 256 * 1024;

	sock = socket(AF_INET, SOCK_DGRAM, 0);
	if ((int)sock < 0)
	{
		OutputDebugString(DEBUG_NAME" socket() failed\n");
		return FALSE;
	}

	nRet = setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (char *)&fFlag, sizeof(fFlag));
	if (nRet == SOCKET_ERROR)
	{
		OutputDebugString("setsockopt(SO_REUSEADDR) failed\n");
		return FALSE;
	}

	memset(&server, 0, sizeof(server));
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = htonl(INADDR_ANY);
	server.sin_port = htons(ss->nUDPMulticastPort);
	structlength = sizeof(server);
	nRet = bind(sock, (struct sockaddr *)&server, structlength);
	if (nRet < 0)
	{
		OutputDebugString(DEBUG_NAME" bind() failed\n");
		//return FALSE;
	}

	nRet = setsockopt(sock, SOL_SOCKET, SO_RCVBUF, (char *)&nReceiveUDPBufferSize, sizeof(nReceiveUDPBufferSize));
	if (nRet == SOCKET_ERROR)
	{
		OutputDebugString("setsockopt(SO_RCVBUF) failed\n");
		return FALSE;
	}

#ifndef UNICAST
	if (strstr(ss->szUDPMulticastAddress, "@") == NULL)
	{
		struct ip_mreq stIpMreq;
		stIpMreq.imr_multiaddr.s_addr = inet_addr(ss->szUDPMulticastAddress); // group addr 
		if (lstrlen(ss->szUDPMulticastInterface))
			stIpMreq.imr_interface.s_addr = inet_addr(ss->szUDPMulticastInterface);
		else
			stIpMreq.imr_interface.s_addr = INADDR_ANY; // use default 
		nRet = setsockopt(sock, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char FAR *)&stIpMreq, sizeof (struct ip_mreq));
	}
	else
	{
		char * szDestination;
		char szSource[MAX_PATH];

		lstrcpy(szSource, ss->szUDPMulticastAddress);
		szDestination = strstr(szSource, "@");
		if (szDestination != NULL)
		{
			struct ip_mreq_source stIpMreqSource;

			*szDestination = '\0';
			szDestination++;

			stIpMreqSource.imr_multiaddr.s_addr = inet_addr(szDestination); // group addr 
			if (lstrlen(ss->szUDPMulticastInterface))
				stIpMreqSource.imr_interface.s_addr = inet_addr(ss->szUDPMulticastInterface);
			else
				stIpMreqSource.imr_interface.s_addr = INADDR_ANY; // use default 
			stIpMreqSource.imr_sourceaddr.s_addr = inet_addr(szSource);
			nRet = setsockopt(sock, IPPROTO_IP, IP_ADD_SOURCE_MEMBERSHIP, (char FAR *)&stIpMreqSource, sizeof(struct ip_mreq_source));
		}
	}
	if (nRet == SOCKET_ERROR)
	{
		char szTemp[128];
		wsprintf(szTemp, DEBUG_NAME" setsockopt() failed with %d\n", WSAGetLastError());
		OutputDebugString(szTemp);
		return FALSE;
	}
#endif UNICAST
#ifdef DEBUG_FILE
	hDebugFile = CreateFile("c:\\tsreader-udp.ts", GENERIC_WRITE, 0, (LPSECURITY_ATTRIBUTES)NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, (HANDLE)NULL);
#endif DEBUG_FILE

	ss->hReadDataThread = CreateThread(NULL, 0, ReadUDPThread, (LPVOID)0, CREATE_SUSPENDED, &dwThreadID);
	SourceHelper_SetWorkerThreadPriorities(FALSE);
	ResumeThread(ss->hReadDataThread);

	return TRUE;
}

BOOL TSReader_Stop()
{
#ifndef UNICAST
	int nRet;
#endif UNICAST

	OutputDebugString(DEBUG_NAME" Wait for read thread terminate\n");
	ss->fTerminateReadThread = TRUE;
	
#ifndef UNICAST
	if (strstr(ss->szUDPMulticastAddress, "@") == NULL)
	{
		struct ip_mreq stIpMreq;
		stIpMreq.imr_multiaddr.s_addr = inet_addr(ss->szUDPMulticastAddress); // group addr 
		if (lstrlen(ss->szUDPMulticastInterface))
			stIpMreq.imr_interface.s_addr = inet_addr(ss->szUDPMulticastInterface);
		else
			stIpMreq.imr_interface.s_addr = INADDR_ANY; // use default 
		nRet = setsockopt(sock, IPPROTO_IP, IP_DROP_MEMBERSHIP, (char FAR *)&stIpMreq, sizeof (struct ip_mreq));
	}
	else
	{
		char * szDestination;
		char szSource[MAX_PATH];

		lstrcpy(szSource, ss->szUDPMulticastAddress);
		szDestination = strstr(szSource, "@");
		if (szDestination != NULL)
		{
			struct ip_mreq_source stIpMreqSource;

			*szDestination = '\0';
			szDestination++;

			stIpMreqSource.imr_multiaddr.s_addr = inet_addr(szDestination); // group addr 
			if (lstrlen(ss->szUDPMulticastInterface))
				stIpMreqSource.imr_interface.s_addr = inet_addr(ss->szUDPMulticastInterface);
			else
				stIpMreqSource.imr_interface.s_addr = INADDR_ANY; // use default 
			stIpMreqSource.imr_sourceaddr.s_addr = inet_addr(szSource);
			nRet = setsockopt(sock, IPPROTO_IP, IP_DROP_SOURCE_MEMBERSHIP, (char FAR *)&stIpMreqSource, sizeof(struct ip_mreq_source));
		}
	}
	if (nRet == SOCKET_ERROR)
	{
		OutputDebugString(DEBUG_NAME" setsockopt() IP_DROP_MEMBERSHIP failed\n");
	}
	else
#endif UNICAST
	{
		closesocket(sock);

		while (ss->fReadThreadTerminated == FALSE)
			Sleep(50);
	}

#ifdef DEBUG_FILE
	CloseHandle(hDebugFile);
#endif DEBUG_FILE

	EnterCriticalSection(&ss->csTSBuffersInUse);
	ss->nTSBuffersInUse = -1000;
	LeaveCriticalSection(&ss->csTSBuffersInUse);

	OutputDebugString(DEBUG_NAME" TSReader_Stop() complete\n");
	return TRUE;
}

BOOL TSReader_Init(PSOURCESTRUCT pss)
{
	DWORD dwDisposition;
	DWORD dwDataSize;
	DWORD dwType;
	LONG lKey;
	HKEY hkMainReg;
//	WSADATA WSAData;
	
	if (SourceHelper_ValidateSourceContainer(pss) == FALSE)
		return FALSE;

	ss = pss;	
	//WSAStartup(0x202, &WSAData);

	lKey = RegCreateKeyEx(HKEY_CURRENT_USER,
		                  gszKeyName,
						  0,
						  gszSourceName,
					      REG_OPTION_NON_VOLATILE,
						  KEY_ALL_ACCESS,
						  NULL,
						  &hkMainReg,
						  &dwDisposition);
	if (lKey == ERROR_SUCCESS)
	{
		if (dwDisposition != REG_CREATED_NEW_KEY)
		{
			dwDataSize = sizeof(fStraightThrough);
			RegQueryValueEx(hkMainReg, "StraightThrough", NULL, &dwType, (BYTE *)&fStraightThrough, &dwDataSize);
		}
		RegCloseKey(hkMainReg);
	}

	return TRUE;
}


BOOL TSReader_DeInit()
{
//	WSACleanup();

	return TRUE;
}

BOOL TSReader_Tune()
{
#ifndef UNICAST
	wsprintf(gszLastTune, "%s port %d", ss->szUDPMulticastAddress, ss->nUDPMulticastPort);
#else UNICAST
	wsprintf(gszLastTune, "Port %d", ss->nUDPMulticastPort);
#endif UNICAST
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
		*dwCapabilities = CAPABILITIES_TIMESTAMP;
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
#ifndef UNICAST
		if (SourceHelper_UDPMulticastTuneDialog(hWnd) == FALSE)
#else UNICAST
		if (SourceHelper_UDPUnicastTuneDialog(hWnd) == FALSE)
#endif UNICAST
			return FALSE;
	}
	else
	{
		ss->nUDPMulticastPort = nUDPMulticastPort;
#ifndef UNICAST
		lstrcpy(ss->szUDPMulticastAddress, szUDPMulticastAddress);
		if (lstrlen(szUDPMulticastInterface))
			lstrcpy(ss->szUDPMulticastInterface, szUDPMulticastInterface);
#endif UNICAST
	}

	return TRUE;
}

BOOL TSReader_ParseCommandLine(PSOURCESTRUCT ss, char * szCommandLine, BOOL fQuiet)
{
	if (lstrlen(szCommandLine))
	{
		int nConversionCount = 0;
#ifndef UNICAST
		int nTemp;
		DWORD dwDotted;
		char * szSpace;
		char * szAtSign;
#endif UNICAST

#ifndef UNICAST
		szAtSign = strstr(szCommandLine, "@");
		if (szAtSign != NULL)
			*szAtSign = '\0';
		nConversionCount = sscanf(szCommandLine,
								  "%d.%d.%d.%d %d", 
								  &nTemp,
								  &nTemp,
								  &nTemp,
								  &nTemp,
								  &nUDPMulticastPort);
		if (nConversionCount < 5)
		{
			if (!fQuiet)
				MessageBox(NULL,
					   "Usage for this source: ipaddr port {interface-addr}\n"
					   "\n"
					   "ipaddr = dotted decimal IP address\n"
					   "port = UDP port to listen on\n"
					   "interface-addr = interface address to listen on",
					   gszSourceName,
					   MB_OK | MB_ICONSTOP);
			return FALSE;
		}
		dwDotted = inet_addr(szCommandLine);
		wsprintf(szUDPMulticastAddress, "%d.%d.%d.%d", (dwDotted) & 0xff, (dwDotted >> 8) & 0xff, (dwDotted >> 16) & 0xff, (dwDotted >> 24) & 0xff);
		if (szAtSign != NULL)
		{
			lstrcat(szUDPMulticastAddress, "@");
			lstrcat(szUDPMulticastAddress, &szAtSign[1]);
		}

		szSpace = strstr(szCommandLine, " ");
		if (szSpace != NULL)
		{
			szSpace = strstr(szSpace + 1, " ");
			if (szSpace != NULL)
			{
				dwDotted = inet_addr(szSpace + 1);
				wsprintf(szUDPMulticastInterface, "%d.%d.%d.%d", (dwDotted) & 0xff, (dwDotted >> 8) & 0xff, (dwDotted >> 16) & 0xff, (dwDotted >> 24) & 0xff);
			}
		}
#else UNICAST
		nConversionCount = sscanf(szCommandLine,
								  "%d", 
								  &nUDPMulticastPort);
		if (nConversionCount < 1)
		{
			if (!fQuiet)
				MessageBox(NULL,
					   "Usage for this source: port\n"
					   "\n"
					   "port = UDP port to listen on\n",
					   gszSourceName,
					   MB_OK | MB_ICONSTOP);
			return FALSE;
		}
#endif UNICAST
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
