#ifdef _DEBUG
//#define DIALOG_TEST
#endif _DEBUG

#include <windows.h>
#include <commctrl.h>
#include <stdint.h>
#include <strsafe.h>
#include <shlwapi.h>
#include "initguid.h"
#include "winioctl.h"
#include <setupapi.h>
#include <stdio.h>

#include "..\sources.h"
#include "resource.h"

typedef struct tPCR
{
	unsigned int nRecord;
	unsigned int base;
	unsigned int ext;
} tPCR;

typedef struct tRTC
{
	__int64 pcr;
	int nRecord;
} tRTC;

static PSOURCESTRUCT ss;
static BOOL fReedSolomonIncluded;
static BOOL fTimestampsIncluded;
static BOOL fDSSMode;
static BOOL fSearchThreadRunning;
static BOOL fAbortSearchThread;
static BOOL fNoEOFPrompt;
static int nPacketLength;
static DWORD dwFileStartOffset;

static BOOL fProgramStream;
static BOOL fRateControlled;
static BOOL fRateControlAuto;
static int nRateManual;
#ifndef LOOP
static BOOL fCloseOnEOF;
static int nStartOffsetPercentage;
#endif LOOP

static HANDLE hInstance;
/* handle to current filename */
static HANDLE hInputStream;
/* currently loaded file size */
static ULARGE_INTEGER g_nInputFileSize;
static ULARGE_INTEGER g_nCurrentReadSize;
#ifdef LOOP
/* how many times have we looped */
uint32_t g_nFileLoops = 0;
#endif

/* currently loaded filename (full path) */
static wchar_t szAlternateFilename[MAX_PATH] = { 0 };
static wchar_t szFileModeFilename[MAX_PATH] = { 0, };
static wchar_t szCmdLineW[MAX_PATH] = { 0, };
static wchar_t *szCmdLinePtr = NULL;
static wchar_t szShortFileName[MAX_PATH] = { 0, };
static char szSearchFailureReason[MAX_PATH + 128];

static CRITICAL_SECTION g_csFileAccess;

#ifndef LOOP
#ifndef CONTINUOUS
wchar_t gszSourceNameW[] = { L"Transport Stream File" };
wchar_t gszSourceName[] = { "Transport Stream File" };
#else CONTINUOUS
wchar_t gszSourceNameW[] = { L"Transport Stream File-continuous" };
char_t gszSourceName[] = { "Transport Stream File-continuous" };
#endif CONTINUOUS
#else LOOP
wchar_t gszSourceNameW[] = { L"Transport Stream File-loop" };
char gszSourceName[] = { "Transport Stream File-loop" };
#endif LOOP

static char gszKeyName[] = "Software\\COOL.STF\\TSReader\\FileSource";

BOOL __cdecl SourceHelper_ValidateSourceContainer(PSOURCESTRUCT pss);

static void FormatLongFileName(const wchar_t *szFileName)
{
	const wchar_t *ptr;
	wchar_t szTemp[MAX_PATH] = { 0, };

	/* look for path separator in reverse - basically strrchr() */
	ptr = &szFileName[lstrlenW(szFileName)];
	if (ptr != szFileName) {
		while (*ptr != L'\\') {
			if (--ptr == szFileName)
				break;
		}
		lstrcpyW(szTemp, ptr + 1);
	}

#define MAX_LEN (27)
	if (lstrlenW(szTemp)) {
		if (lstrlenW(szTemp) >= MAX_LEN)
			wcscpy(&szTemp[MAX_LEN - 3], L"...");
		EnterCriticalSection(&g_csFileAccess);
		lstrcpyW(szShortFileName, szTemp);
		LeaveCriticalSection(&g_csFileAccess);
	}
}

BOOL CALLBACK ReachedEndDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg)
	{
	case WM_INITDIALOG:
		MessageBeep(0);
		break;
	case WM_CLOSE:
		EndDialog(hDlg, FALSE);
		break;
	case WM_COMMAND:
		switch(LOWORD(wParam))
		{
		case IDOK:
			fNoEOFPrompt = IsDlgButtonChecked(hDlg, IDC_DONT_TELL_ME_AGAIN);
			EndDialog(hDlg, FALSE);
			break;
		}
		break;
	}
	return FALSE;
}

#ifndef LOOP
BOOL CheckForNextTSFile(void)
{
	char szTemp[MAX_PATH];
	char * szSecondPeriod;

	// If it's a .TS file, let's try the next one
	szSecondPeriod = strstr(szFileModeFilename, ".ts");
	if (szSecondPeriod != NULL)
	{
		// Double check. Should have a period, four numbers and then another period
		char * szFirstPeriod = szSecondPeriod - 5;
		if (*szFirstPeriod == '.' || *szFirstPeriod == '_')
		{
			if (     (*(szFirstPeriod + 1) >= '0') && (*(szFirstPeriod + 1) <= '9')
				 &&  (*(szFirstPeriod + 2) >= '0') && (*(szFirstPeriod + 2) <= '9')
				 &&  (*(szFirstPeriod + 3) >= '0') && (*(szFirstPeriod + 3) <= '9')
				 &&  (*(szFirstPeriod + 4) >= '0') && (*(szFirstPeriod + 4) <= '9') )
			{
				char szTemp2[20];
				int nNextSequence;

				sscanf(szFirstPeriod + 1, "%d", &nNextSequence);
				if ( (nNextSequence >= 0) && (nNextSequence < 9999) )
				{
					char szSeperator[] = {"."};

					if (*szFirstPeriod == '_')
						szSeperator[0] = '_';
					nNextSequence++;
					CloseHandle(hInputStream);
					*szFirstPeriod = '\0';
					lstrcpy(szTemp, szFileModeFilename);
					lstrcat(szTemp, szSeperator);
					wsprintf(szTemp2, "%04d.ts", nNextSequence);
					lstrcat(szTemp, szTemp2);
					lstrcpy(szFileModeFilename, szTemp);
					hInputStream = CreateFile(szFileModeFilename, GENERIC_READ, FILE_SHARE_READ, (LPSECURITY_ATTRIBUTES)NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, (HANDLE)NULL);
					if (hInputStream != INVALID_HANDLE_VALUE)
					{
						// Ok to continue on with this file
						dbg_printf("Now reading from %s\n", szFileModeFilename);

						/* get file size for progress report */
						g_nInputFileSize.LowPart = GetFileSize(hInputStream, &g_nInputFileSize.HighPart);
						g_nCurrentReadSize.QuadPart = 0;

						return TRUE;
					}
				}
			}
		}
	}

	// If it's a .tp file, try the next
	szSecondPeriod = strstr(szFileModeFilename, ".tp");
	if (szSecondPeriod != NULL)
	{
		char * szSequenceSeperator = strstr(szFileModeFilename, "_");

		if (szSequenceSeperator == NULL)
		{
			// no sequence - start at 1
			*szSecondPeriod = '\0';
			lstrcat(szFileModeFilename, "_01.tp");
		}
		else
		{
			int nCurrentSequence;

			*szSecondPeriod = '\0';
			if (sscanf(szSequenceSeperator + 1, "%d", &nCurrentSequence) == 1)
			{
				char szFilename[32];
				nCurrentSequence++;
				wsprintf(szFilename, "_%02d.tp", nCurrentSequence);
				lstrcpy(szSequenceSeperator, szFilename);
			}
			else
				return FALSE;
		}
		CloseHandle(hInputStream);
		hInputStream = CreateFile(szFileModeFilename, GENERIC_READ, FILE_SHARE_READ, (LPSECURITY_ATTRIBUTES)NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, (HANDLE)NULL);
		if (hInputStream != INVALID_HANDLE_VALUE)
		{
			// Ok to continue on with this file
			dbg_printf("Now reading from %s\n", szFileModeFilename);

			/* get file size for progress report */
			g_nInputFileSize.LowPart = GetFileSize(hInputStream, &g_nInputFileSize.HighPart);
			g_nCurrentReadSize.QuadPart = 0;

			return TRUE;
		}
	}

	return FALSE;
}
#endif LOOP

void ThreadTerminateCleanup(void)
{
	Sleep(1000);
	do
	{
		int nBuffers;
		EnterCriticalSection(&ss->csTSBuffersInUse);
		nBuffers = ss->nTSBuffersInUse;
		LeaveCriticalSection(&ss->csTSBuffersInUse);
		if (nBuffers == 0)
			break;
		Sleep(25);
	} while (!ss->fTerminateReadThread);

	// Now we can terminate
	CloseHandle(hInputStream);

	EnterCriticalSection(&ss->csTSBuffersInUse);
	ss->nTSBuffersInUse = -1000;
	LeaveCriticalSection(&ss->csTSBuffersInUse);

	if (!ss->fTerminateReadThread)
	{
#ifndef LOOP
		if (!fNoEOFPrompt && !fCloseOnEOF)
#else LOOP
		if (!fNoEOFPrompt)
#endif LOOP
			DialogBox((HINSTANCE)hInstance, MAKEINTRESOURCE(IDD_REACHED_END), NULL, ReachedEndDlgProc);
	}

	CloseHandle(ss->hReadDataThread);
	ss->fTerminateReadThread = FALSE;
	ss->fReadThreadTerminated = TRUE;
#ifndef LOOP
	if (fCloseOnEOF)
		PostMessage(GetParent(ss->hWndTSReader), WM_CLOSE, 0 ,0);
#endif LOOP
}

DWORD WINAPI ReadRateControlledThread(LPVOID lpv)
{
	int nTSBufferIndex = 0;
	int nPacketsPerSecond;
	int nTicksPerPacket;
	int nPacketsPerBuffer;
	int nRateMbps;
	int nNonRSPacketLength = 188;
	int nRSPacketLength = 204;
	__int64 lnTicksPerSecond;
	__int64 lnCurrentTime, lnTargetTime;
	double dInterPacketDelay;
	double dnsPerTick;
	double dInterPacketns;
	double dTickError;
	double dTickRemainder;
	double dTicksPerPacket;
	BYTE *pConversionBuffer = NULL;

	dbg_printf("ReadRateControlledThread+\n");

	ss->fReadThreadTerminated = FALSE;
	ss->fTerminateReadThread = FALSE;

	if (fDSSMode)
	{
		nNonRSPacketLength = 131;
		nRSPacketLength = 147;
	}
	
	nRateMbps = nRateManual / 1000000;
	if (nRateMbps < 1)
		nRateMbps = 1;
	nPacketsPerBuffer = 4 * nRateMbps;
	if (nPacketsPerBuffer > TS_PACKETS_AT_A_TIME)
		nPacketsPerBuffer = TS_PACKETS_AT_A_TIME;
	nPacketsPerSecond = nRateManual / (nPacketsPerBuffer * nNonRSPacketLength * 8);
	dInterPacketDelay = 1.0 / (double)nPacketsPerSecond;
	QueryPerformanceFrequency((LARGE_INTEGER *)&lnTicksPerSecond);
	dnsPerTick = 1000000000.0 / (double)lnTicksPerSecond;
	dInterPacketns = dInterPacketDelay * 1000000000.0;
	dTicksPerPacket = dInterPacketns / dnsPerTick;
	nTicksPerPacket = (int)dTicksPerPacket;
	dTickRemainder = dTicksPerPacket - (double)nTicksPerPacket;
	dTickError = 0.0;

	if (nPacketLength != nNonRSPacketLength)
		pConversionBuffer = LocalAlloc(LPTR, nPacketLength * nPacketsPerBuffer);

	QueryPerformanceCounter((LARGE_INTEGER *)&lnTargetTime);
	lnTargetTime += nTicksPerPacket;
	while (!ss->fTerminateReadThread)
	{
		BOOL fNeedToSleep = FALSE;
		DWORD dwRead;
		
		if (nPacketLength == nNonRSPacketLength)
		{
			// Very simple for nNonRSPacketLength bytes
			ReadFile(hInputStream, ss->tsb[nTSBufferIndex].pData, nPacketsPerBuffer * nNonRSPacketLength, &dwRead, NULL);
			g_nCurrentReadSize.QuadPart += dwRead;
			ss->tsb[nTSBufferIndex].nSize = dwRead;
			EnterCriticalSection(&ss->csPIDCounter);
			ss->nLastSecondByteCounter += dwRead;
			LeaveCriticalSection(&ss->csPIDCounter);
		}
		else
		{
			int nPacket;
			int nPackets;
			BYTE * pSourcePtr = pConversionBuffer;
			BYTE * pDestPtr = ss->tsb[nTSBufferIndex].pData;

			// Timestamp or R/S codes
			ReadFile(hInputStream, pConversionBuffer, nPacketsPerBuffer * nPacketLength, &dwRead, NULL);
			g_nCurrentReadSize.QuadPart += dwRead;
			if (dwRead)
			{
				nPackets = (int)dwRead / nPacketLength;
				for (nPacket = 0; nPacket < nPackets; nPacket++)
				{
					memcpy(pDestPtr, pSourcePtr, nNonRSPacketLength);
					pDestPtr += nNonRSPacketLength;
					pSourcePtr += nNonRSPacketLength;
					if (nPacketLength == 192)
					{
						if (ss->tsb[nTSBufferIndex].pTimestamps != NULL)
							memcpy(&ss->tsb[nTSBufferIndex].pTimestamps[nPacket], pSourcePtr, 4);
						pSourcePtr += 4;
					}
					else
						pSourcePtr += 16;
				}		
				ss->tsb[nTSBufferIndex].nSize = nPackets * nNonRSPacketLength;
				EnterCriticalSection(&ss->csPIDCounter);
				ss->nLastSecondByteCounter += nPackets * nNonRSPacketLength;
				LeaveCriticalSection(&ss->csPIDCounter);
			}
		}

		if (dwRead == 0)
		{
#ifdef LOOP
			if (ss->fTerminateReadThread)
				break;
			SetFilePointer(hInputStream, dwFileStartOffset, NULL, FILE_BEGIN);
			g_nFileLoops++;
			g_nCurrentReadSize.QuadPart = 0;
			continue;
#else LOOP
			if (CheckForNextTSFile() == TRUE)
				continue;
#endif LOOP
			break;
		}
		
		nTSBufferIndex++;
		if (nTSBufferIndex == MAX_TS_BUFFERS)
			nTSBufferIndex = 0;
		EnterCriticalSection(&ss->csTSBuffersInUse);
		ss->nTSBuffersInUse++;
		LeaveCriticalSection(&ss->csTSBuffersInUse);

		do
		{
			QueryPerformanceCounter((LARGE_INTEGER *)&lnCurrentTime);
			if (((lnTargetTime - lnCurrentTime) * (int)dnsPerTick) > 2000000)
				Sleep(1);
		} while (lnCurrentTime < lnTargetTime);
		lnTargetTime += nTicksPerPacket;
		dTickError += dTickRemainder;
		if (dTickError > 50.0)
		{
			lnTargetTime += 50;
			dTickError -= 50.0;
		}
	}

	if (nPacketLength != nNonRSPacketLength)
		LocalFree(pConversionBuffer);
	ThreadTerminateCleanup();
	dbg_printf("ReadRateControlledThread-\n");
	return 0;
}

DWORD WINAPI ReadUncontrolledThread(LPVOID lpv)
{
	int nTSBufferIndex = 0;
	BOOL fNeedToSleep;
	BYTE * tsbuffer;
	int nReadSize;
#ifdef CONTINUOUS
	int nNoDataCount = 0;
#endif CONTINUOUS

#ifdef DEBUG_MESSAGES
	dbg_printf("+ReadUncontrolledThread\n");
#endif DEBUG_MESSAGES

#ifdef CONTINUOUS
	SourceHelper_StartSyncThread(ss, FALSE);
#endif CONTINUOUS
	ss->fReadThreadTerminated = FALSE;
	ss->fTerminateReadThread = FALSE;

	nReadSize = TS_PACKETS_AT_A_TIME * nPacketLength;
	tsbuffer = LocalAlloc(LPTR, nReadSize);
	while (!ss->fTerminateReadThread)
	{
		DWORD dwRead;

		fNeedToSleep = FALSE;
		ReadFile(hInputStream, tsbuffer, nReadSize, &dwRead, NULL);
		g_nCurrentReadSize.QuadPart += dwRead;
		if (dwRead == 0)
		{
#ifdef LOOP
			if (ss->fTerminateReadThread)
				break;
			SetFilePointer(hInputStream, dwFileStartOffset, NULL, FILE_BEGIN);
			continue;
#else LOOP
			if (CheckForNextTSFile() == TRUE)
				continue;
#endif LOOP
#ifdef CONTINUOUS
			// Continuous mode - sleep for up to 5 seconds before saying EOF
			Sleep(100);
			if (++nNoDataCount > 50)
				break;
			continue;
#endif CONTINUOUS
			break;
		}
#ifdef CONTINUOUS
		else
			nNoDataCount = 0;
#endif CONTINUOUS

#ifndef CONTINUOUS
		EnterCriticalSection(&ss->csPIDCounter);
		ss->nLastSecondByteCounter += dwRead;
		LeaveCriticalSection(&ss->csPIDCounter);

		if (fReedSolomonIncluded == FALSE)
		{
			if (fTimestampsIncluded == FALSE)
			{
				// Regular 188 byte TS packets
				ss->tsb[nTSBufferIndex].nSize = dwRead;
				memcpy(ss->tsb[nTSBufferIndex].pData, tsbuffer, dwRead);
			}
			else
			{
				// 192 byte packets - 188 payload, 4 timestamp
				int nPacket;
				int nPackets = (int)dwRead / 192;
				BYTE * pSourcePtr = tsbuffer;
				BYTE * pDestPtr = ss->tsb[nTSBufferIndex].pData;

				ss->tsb[nTSBufferIndex].nSize = nPackets * 188;
				for (nPacket = 0; nPacket < nPackets; nPacket++)
				{
					memcpy(pDestPtr, pSourcePtr, 188);
					pDestPtr += 188;
					pSourcePtr += 188;
					if (ss->tsb[nTSBufferIndex].pTimestamps != NULL)
						memcpy(&ss->tsb[nTSBufferIndex].pTimestamps[nPacket], pSourcePtr, 4);
					pSourcePtr += 4;
				}
			}
		}
		else
		{
			// 204 byte TS + Reed-Solomon
			int nNonRSLength = 188;
			int nRSLength = 204;
			int nPackets;
			int nPacket;
			BYTE * pSourcePtr = tsbuffer;
			BYTE * pDestPtr = ss->tsb[nTSBufferIndex].pData;

			if (fDSSMode)
			{
				nNonRSLength = 131;
				nRSLength = 147;
			}
			nPackets = (int)dwRead / nRSLength;
			ss->tsb[nTSBufferIndex].nSize = nPackets * nNonRSLength;
			for (nPacket = 0; nPacket < nPackets; nPacket++)
			{
				memcpy(pDestPtr, pSourcePtr, nNonRSLength);
				pDestPtr += nNonRSLength;
				pSourcePtr += nRSLength;
			}
		}
		nTSBufferIndex++;
		if (nTSBufferIndex == MAX_TS_BUFFERS)
			nTSBufferIndex = 0;
		EnterCriticalSection(&ss->csTSBuffersInUse);
		ss->nTSBuffersInUse++;
		if (ss->nTSBuffersInUse > (MAX_TS_BUFFERS / 8) * 7)
			fNeedToSleep = TRUE;
		LeaveCriticalSection(&ss->csTSBuffersInUse);
		if (fNeedToSleep == TRUE)
		{
			BOOL fOKToExit = FALSE;
			do
			{
				if (ss->fTerminateReadThread)
					break;
				Sleep(100);
				EnterCriticalSection(&ss->csTSBuffersInUse);
				if (ss->nTSBuffersInUse < (MAX_TS_BUFFERS / 8) * 7)
					fOKToExit = TRUE;
				LeaveCriticalSection(&ss->csTSBuffersInUse);
			} while (fOKToExit == FALSE);
		}
#else CONTINUOUS
		SourceHelper_SyncData(tsbuffer, dwRead);
#endif CONTINUOUS
	}

#ifdef CONTINUOUS
	SourceHelper_StopSyncThread();
#endif CONTINUOUS
	ThreadTerminateCleanup();
	LocalFree(tsbuffer);
	dbg_printf("ReadUncontrolledThread-\n");
	return 0;
}

BOOL TSReader_Start(void)
{
	DWORD dwThreadID;

	if (fRateControlled)
		ss->hReadDataThread = CreateThread(NULL, 0, ReadRateControlledThread, (LPVOID)0, CREATE_SUSPENDED, &dwThreadID);
	else
		ss->hReadDataThread = CreateThread(NULL, 0, ReadUncontrolledThread, (LPVOID)0, CREATE_SUSPENDED, &dwThreadID);
	SourceHelper_SetWorkerThreadPriorities(FALSE);
	ResumeThread(ss->hReadDataThread);

	return TRUE;
}

BOOL TSReader_Stop(void)
{
	ss->fTerminateReadThread = TRUE;
	while (ss->fReadThreadTerminated == FALSE)
		Sleep(50);

	return TRUE;
}

BOOL TSReader_Init(PSOURCESTRUCT pss)
{
	LONG lKey;
	HKEY hkMainReg;
	DWORD dwDisposition;

	if (SourceHelper_ValidateSourceContainer(pss) == FALSE)
		return FALSE;

	ss = pss;

	fReedSolomonIncluded = FALSE;
	fTimestampsIncluded = FALSE;
	fDSSMode = FALSE;
	fNoEOFPrompt = FALSE;
	fRateControlled = FALSE;
	fRateControlAuto = TRUE;
	nRateManual = 0;
	g_nCurrentReadSize.QuadPart = 0;
	g_nInputFileSize.QuadPart = 0;

#ifndef LOOP
	fCloseOnEOF = FALSE;
	nStartOffsetPercentage = 0;
#endif LOOP

	InitializeCriticalSection(&g_csFileAccess);
	DragAcceptFiles(ss->hWndTSReader, TRUE);

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
			DWORD dwDataSize = sizeof(DWORD);
			DWORD dwType;

			RegQueryValueEx(hkMainReg, "NoEOFPrompt", NULL, &dwType, (BYTE *)&fNoEOFPrompt, &dwDataSize);
			RegQueryValueEx(hkMainReg, "RateControlled", NULL, &dwType, (BYTE *)&fRateControlled, &dwDataSize);
			RegQueryValueEx(hkMainReg, "RateControlAuto", NULL, &dwType, (BYTE *)&fRateControlAuto, &dwDataSize);
			RegQueryValueEx(hkMainReg, "RateManual", NULL, &dwType, (BYTE *)&nRateManual, &dwDataSize);
#ifndef LOOP			
			RegQueryValueEx(hkMainReg, "CloseOnEOF", NULL, &dwType, (BYTE *)&fCloseOnEOF, &dwDataSize);
#endif LOOP
		}
		RegCloseKey(hkMainReg);
	}

	return TRUE;
}

BOOL TSReader_DeInit(void)
{
	LONG lKey;
	HKEY hkMainReg;
	DWORD dwDisposition;

	DragAcceptFiles(ss->hWndTSReader, FALSE); 

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
		RegSetValueEx(hkMainReg, "NoEOFPrompt", 0, REG_DWORD, (BYTE *)&fNoEOFPrompt, sizeof(fNoEOFPrompt));
		RegSetValueEx(hkMainReg, "RateControlled", 0, REG_DWORD, (BYTE *)&fRateControlled, sizeof(fRateControlled));
		RegSetValueEx(hkMainReg, "RateControlAuto", 0, REG_DWORD, (BYTE *)&fRateControlAuto, sizeof(fRateControlAuto));
		RegSetValueEx(hkMainReg, "RateManual", 0, REG_DWORD, (BYTE *)&nRateManual, sizeof(nRateManual));
#ifndef LOOP
		RegSetValueEx(hkMainReg, "CloseOnEOF", 0, REG_DWORD, (BYTE *)&fCloseOnEOF, sizeof(fCloseOnEOF));		
#endif LOOP
		RegCloseKey(hkMainReg);
	}
	DeleteCriticalSection(&g_csFileAccess);
	return TRUE;
}

void EnableDisableRateControl(HWND hDlg)
{
	EnableWindow(GetDlgItem(hDlg, IDC_RATE_CALCULATED), fRateControlled);
	EnableWindow(GetDlgItem(hDlg, IDC_RATE_MANUAL), fRateControlled);
	if (!fRateControlled)
		EnableWindow(GetDlgItem(hDlg, IDC_MANUAL_RATE), FALSE);
	else
		EnableWindow(GetDlgItem(hDlg, IDC_MANUAL_RATE), !fRateControlAuto);
}

UINT FAR PASCAL GetMPEGFileNameHookProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)  // BOOL CALLBACK ??
{ 
	switch(uMsg)
	{ 
	// WM_INITDIALOG is received after commdlg 
	//is processed. 
	case WM_INITDIALOG:
		CheckDlgButton(hDlg, IDC_RATE_CONTROL_ENABLED, fRateControlled);
		if (fRateControlAuto)
			CheckDlgButton(hDlg, IDC_RATE_CALCULATED, BST_CHECKED);
		else
			CheckDlgButton(hDlg, IDC_RATE_MANUAL, BST_CHECKED);
		SetDlgItemInt(hDlg, IDC_MANUAL_RATE, nRateManual, FALSE);
		EnableDisableRateControl(hDlg);
#ifndef LOOP
		CheckDlgButton(hDlg, IDC_END_TSR_ON_EOF, fCloseOnEOF);
		SetDlgItemText(hDlg, IDC_FILE_OFFSET_DISPLAY, "0%");
		SendDlgItemMessage(hDlg, IDC_FILE_OFFSET, TBM_SETRANGE, FALSE, MAKELONG(0, 99));
#endif LOOP
		break;
	case WM_COMMAND:
		switch (HIWORD(wParam))
		{
		case BN_CLICKED:
			switch(LOWORD(wParam))
			{
			case IDC_RATE_CONTROL_ENABLED:
				fRateControlled = IsDlgButtonChecked(hDlg, IDC_RATE_CONTROL_ENABLED);
				EnableDisableRateControl(hDlg);
				break;
			case IDC_RATE_CALCULATED:
			case IDC_RATE_MANUAL:
				fRateControlAuto = IsDlgButtonChecked(hDlg, IDC_RATE_CALCULATED);
				EnableDisableRateControl(hDlg);
				break;
#ifndef LOOP
			case IDC_END_TSR_ON_EOF:
				fCloseOnEOF = IsDlgButtonChecked(hDlg, IDC_END_TSR_ON_EOF);
				break;
#endif LOOP
			}
			break;
		}
		break;
#ifndef LOOP
		case WM_HSCROLL:
			nStartOffsetPercentage = SendDlgItemMessage(hDlg, IDC_FILE_OFFSET, TBM_GETPOS, 0, 0);
			{
				char szTemp[128];
				wsprintf(szTemp, "%d%%", nStartOffsetPercentage);
				SetDlgItemText(hDlg, IDC_FILE_OFFSET_DISPLAY, szTemp);
			}
			break;
#endif LOOP
	case WM_DESTROY:
		nRateManual = GetDlgItemInt(hDlg, IDC_MANUAL_RATE, NULL, FALSE);
		break;

	}
	return FALSE; 
} 

BOOL GetInputMPEGFilename(HWND hDlg, wchar_t *szFilename)
{
	OPENFILENAMEW ofn = { 0, };
	wchar_t szTemp[MAX_PATH];
	BOOL rv;

	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = hDlg;
	if (ofn.hwndOwner == NULL)
		ofn.hwndOwner = GetDesktopWindow();
	ofn.lpstrFile = szFilename;
	ofn.nMaxFile = MAX_PATH;
	switch(ss->fLastFileTS)
	{
	case TRUE:
		ofn.lpstrFilter = L"Transport Stream Files(*.ts)\0*.ts\0MPEG Files(*.mpg)\0*.mpg\0MyHD .TP Files (*.tp)\0*.tp\0All Files(*.*)\0*.*\0\0";	
		ofn.lpstrDefExt = L"ts";
		break;
	case FALSE:
		ofn.lpstrFilter = L"MPEG Files(*.mpg)\0*.mpg\0Transport Stream Files(*.ts)\0*.ts\0MyHD .TP Files (*.tp)\0*.tp\0All Files(*.*)\0*.*\0\0";
		ofn.lpstrDefExt = L"mpg";
		break;
	case 2:
		ofn.lpstrFilter = L"MyHD .TP Files (*.tp)\0*.tp\0MPEG Files(*.mpg)\0*.mpg\0Transport Stream Files(*.ts)\0*.ts\0All Files(*.*)\0*.*\0\0";	
		ofn.lpstrDefExt = L"tp";
		break;
	case 3:
		ofn.lpstrFilter = L"All Files(*.*)\0*.*\0MPEG Files(*.mpg)\0*.mpg\0Transport Stream Files(*.ts)\0*.ts\0MyHD .TP Files (*.tp)\0*.tp\0\0";	
		ofn.lpstrDefExt = L"*";
		break;
	}
#ifndef LOOP
	ofn.lpstrTitle = L"Select Transport Stream File";
#else LOOP
	ofn.lpstrTitle = L"Select Transport Stream File to loop";
#endif LOOP
	ofn.Flags =  OFN_HIDEREADONLY | OFN_ENABLEHOOK | OFN_ENABLETEMPLATE | OFN_EXPLORER;
	mbstowcs(szTemp, ss->szTransportStreamInitialDir, strlen(ss->szTransportStreamInitialDir));
	ofn.lpstrInitialDir = szTemp;
	ofn.lpfnHook = GetMPEGFileNameHookProc;
	ofn.lpTemplateName = MAKEINTRESOURCEW(IDD_RATE_CONTROL_HOOK);
	ofn.hInstance = (HINSTANCE)hInstance;
	rv = SourceHelper_myGetOpenFileNameW(&ofn);
	wcstombs(ss->szTransportStreamInitialDir, szTemp, lstrlenW(szTemp));

	return rv;
}

int CalculateMPEGBitrate(HWND hDlg)
{
	int nRecord = 0;
	int nPCRCount = 0;
	uint16_t nPID = 0;
	tPCR pcr[2];
	BYTE pBuffer[204];

	SetDlgItemText(hDlg, IDC_STATUS, "Calculating MPEG bitrate");

	do
	{
		DWORD dwRead;

		memset(pBuffer, 0, sizeof(pBuffer));
		ReadFile(hInputStream, pBuffer, nPacketLength, &dwRead, NULL);
		if (dwRead != (DWORD)nPacketLength)
			return -1;
		nRecord++;

		// Check for an adaptation field and a PCR value.
		if ((pBuffer[3] & 0x20) && (pBuffer[4] > 0) && (pBuffer[5] & 0x10))
		{
			if (nPCRCount == 0)
				nPID = (uint16_t)(((pBuffer[1] & 0x1F) << 8) + pBuffer[2]);
			else
			{
				if (nPID != (uint16_t)(((pBuffer[1] & 0x1F) << 8) + pBuffer[2]))
					continue;
			}
			pcr[nPCRCount].base = (pBuffer[6] << 25)
				+ (pBuffer[7] << 17)
				+ (pBuffer[8] << 9)
				+ (pBuffer[9] << 1)
				+ ((pBuffer[10] & 0x80) >> 7);
			pcr[nPCRCount].ext = ((pBuffer[10] & 0x01) << 8)
				+ (pBuffer[11]);
			pcr[nPCRCount].nRecord = nRecord;
			nPCRCount++;
		}

		// If we've seen two PCRs, we can calculate the rate
		if (nPCRCount == 2)
		{	
			unsigned int nBitRate[3];
			//int i;
			float BitRate, TimeDifference, BitDifference;
			//unsigned char cBitRate[3][12];

			TimeDifference = (float)((unsigned int)((pcr[1].base * 300) + pcr[1].ext) - ((pcr[0].base * 300) + pcr[0].ext));
			BitDifference = (float)(pcr[1].nRecord - pcr[0].nRecord) * nPacketLength * 8;
			BitRate = 1 / ((TimeDifference / BitDifference) / 27000000);
			nBitRate[0] = (unsigned int)BitRate;
			dbg_printf("File: Rate controlled calculated rate %d\n", nBitRate[0]);

			return nBitRate[0];
		}
	} while (!fAbortSearchThread);

	return -1;		// should never get here
}

int CalculateDSSBitrate(HWND hDlg)
{
	int nRecord = 0;
	int nPCRCount = 0;
	uint16_t nSCID = 0;
	tRTC rtc[2];
	BYTE pBuffer[147];

	SetDlgItemText(hDlg, IDC_STATUS, "Calculating DSS bitrate");

	do
	{
		DWORD dwRead;

		ReadFile(hInputStream, pBuffer, nPacketLength, &dwRead, NULL);
		if (dwRead != (DWORD)nPacketLength)
			return -1;
		nRecord++;

		// Check for an adaptation field and a PCR value.
		if ((pBuffer[1] & 0xa0) == 0xa0 && pBuffer[4] == 0xc3 && pBuffer[5] == 0x7d)
		{
			if (nPCRCount == 0)
				nSCID = (pBuffer[1] << 8 | pBuffer[2]) & 0xfff;
			else
			{
				if (nSCID != ((pBuffer[1] << 8 | pBuffer[2]) & 0xfff))
					continue;
			}
			rtc[nPCRCount].pcr = ((__int64)pBuffer[7 + 0] << 24 | (__int64)pBuffer[7 + 1] << 16 | (__int64)pBuffer[7 + 2] << 8 | (__int64)pBuffer[7 + 3]);
			rtc[nPCRCount].nRecord = nRecord;
			nPCRCount++;
		}

		// If we've seen two PCRs, we can calculate the rate
		if (nPCRCount == 2)
		{	
			unsigned int nBitRate[3];
			//int i;
			float BitRate, TimeDifference, BitDifference;
			//unsigned char cBitRate[3][12];

			TimeDifference = (float)rtc[1].pcr - rtc[0].pcr;
			if (TimeDifference < 0)
			{
				// PCR (well RTC in DSS parlence) has wrapped so let's do this again
				nPCRCount = 0;
				continue;
			}

			BitDifference = (float)(rtc[1].nRecord - rtc[0].nRecord) * nPacketLength * 8;
			BitRate = 1 / ((TimeDifference / BitDifference) / 27000000);
			nBitRate[0] = (unsigned int)BitRate;
			dbg_printf("File: Rate controlled calculated rate %d\n", nBitRate[0]);

			return nBitRate[0];
		}
	} while (!fAbortSearchThread);

	return -1;		// should never get here

}

DWORD WINAPI SearchMPEGFileForSyncThread(LPVOID lpv)
{
	HWND hDlg = (HWND)lpv;
	BYTE * pPtr;
	HANDLE hMap;
	BYTE * pFile;
	FILETIME ft;		// yeah I know...
	DWORD64 dwFileSize, dwMapSize;
#ifndef LOOP
	DWORD64 dwOffset;
#endif LOOP
	SYSTEM_INFO SystemInfo;

	GetSystemInfo(&SystemInfo);

	fSearchThreadRunning = TRUE;
	fProgramStream = FALSE;

	// Memory map the file so we can search for the sync pattern
	ft.dwLowDateTime = GetFileSize(hInputStream, &ft.dwHighDateTime);
	memcpy(&dwFileSize, &ft, sizeof(DWORD64));
	if (dwFileSize == 0)
	{
		StringCchCopy(szSearchFailureReason, sizeof(szSearchFailureReason), "Zero length files are not accepted");
		fSearchThreadRunning = FALSE;
		PostMessage(hDlg, WM_USER + 1, 0, 0);
		return 0;
	}
	dwMapSize = dwFileSize;
	if (dwMapSize > (DWORD64)0x10000000)
		dwMapSize = (DWORD64)0x10000000;	// 0.25GB

	memset(&ft, 0, sizeof(ft));
#ifndef LOOP
	dwOffset = (dwFileSize / 100) * nStartOffsetPercentage;
	dwOffset &= ~(SystemInfo.dwAllocationGranularity - 1);
	memcpy(&ft, &dwOffset, sizeof(ft));
	if (dwOffset + dwMapSize > dwFileSize)
		dwMapSize = dwFileSize - dwOffset;
#endif LOOP
	hMap = CreateFileMapping(hInputStream, NULL, PAGE_READONLY | SEC_COMMIT, 0,  (DWORD)dwMapSize, NULL);
	if (hMap == NULL)
	{
		StringCchPrintf(szSearchFailureReason, sizeof(szSearchFailureReason), "Unable to create mapping file - %d (size = 0x%x)", GetLastError(), (DWORD)dwMapSize);
		fSearchThreadRunning = FALSE;
		PostMessage(hDlg, WM_USER + 1, 0, 0);
		return 0;
	}
	pFile = (BYTE *)MapViewOfFile(hMap, FILE_MAP_READ, ft.dwHighDateTime, ft.dwLowDateTime, (DWORD)dwMapSize);
	if (pFile == NULL)
	{
		StringCchPrintf(szSearchFailureReason, sizeof(szSearchFailureReason), "Unable to create memory view - %d (size = 0x%x)", GetLastError(), (DWORD)dwMapSize);
		CloseHandle(hMap);
		fSearchThreadRunning = FALSE;
		PostMessage(hDlg, WM_USER + 1, 0, 0);
		return 0;
	}

	// Find the sync
	pPtr = pFile;
	dwFileStartOffset = 0;

	// Check for MPEG-2 program stream
	if (   (*(pPtr + 0) == 0x00)
		&& (*(pPtr + 1) == 0x00)
		&& (*(pPtr + 2) == 0x01)
		&& (*(pPtr + 3) == 0xba)) // pack start?
	{
		fProgramStream = TRUE;
	}
	else
	{
		do
		{
			if (*pPtr == 0x47)
			{
				int i;

				for (i = 1; i < 10; i++)
				{
					// maybe got a TS sync word
					if (*(pPtr + (i * 188)) != 0x47)
						break;
				}
				if (i == 10)
				{
					// Yes! Got a file without RS
					fReedSolomonIncluded = FALSE;
					fDSSMode = FALSE;
					fTimestampsIncluded = FALSE;
					dbg_printf("File: Synced with 188 byte MPEG-2\n");
					break;
				}

				for (i = 1; i < 10; i++)
				{
					// maybe got a TS sync word
					if (*(pPtr + (i * 204)) != 0x47)
						break;
				}
				if (i == 10)
				{
					// Yes! Got a file with RS
					fReedSolomonIncluded = TRUE;
					fDSSMode = FALSE;
					fTimestampsIncluded = FALSE;
					dbg_printf("File: Synced with 204 byte MPEG-2\n");
					break;
				}

				for (i = 1; i < 10; i++)
				{
					// maybe got a TS sync word
					if (*(pPtr + (i * 192)) != 0x47)
						break;
				}
				if (i == 10)
				{
					// Yes! Got a file with timestamps
					fReedSolomonIncluded = FALSE;
					fDSSMode = FALSE;
					fTimestampsIncluded = TRUE;
					dbg_printf("File: Synced with 192 byte MPEG-2\n");
					break;
				}
			}
			else if (*pPtr == 0x1d)
			{
				int i;
				for (i = 1; i < 10; i++)
				{
					// maybe got a TS sync word
					if (*(pPtr + (i * 131)) != 0x1d)
						break;
				}
				if (i == 10)
				{
					// Yes! Got a file without RS
					fReedSolomonIncluded = FALSE;
					fDSSMode = TRUE;
					fTimestampsIncluded = FALSE;
					dbg_printf("File: Synced with 131 byte DSS\n");
					break;
				}

				for (i = 1; i < 10; i++)
				{
					// maybe got a TS sync word
					if (*(pPtr + (i * 147)) != 0x1d)
						break;
				}
				if (i == 10)
				{
					// Yes! Got a file with RS
					fReedSolomonIncluded = TRUE;
					fDSSMode = TRUE;
					fTimestampsIncluded = FALSE;
					dbg_printf("File: Synced with 147 byte DSS\n");
					break;
				}
			}
			pPtr++;
			dwFileStartOffset++;
			if ((dwFileStartOffset & 0xffff) == 0 && !fAbortSearchThread)
				PostMessage(GetDlgItem(hDlg, IDC_SYNC_PROGRESS), PBM_SETPOS, dwFileStartOffset, 0);
		} while (dwFileStartOffset < (dwMapSize - (204 * 2)) && !fAbortSearchThread);
	}

	dbg_printf("File: out of sync search loop\n");
	UnmapViewOfFile(pFile);
	CloseHandle(hMap);

	if (fDSSMode == FALSE)
	{
		if (fReedSolomonIncluded == FALSE)
		{
			if (fTimestampsIncluded == FALSE)
				nPacketLength = 188;
			else
				nPacketLength = 192;
		}
		else
			nPacketLength = 204;
	}
	else
	{
		if (fReedSolomonIncluded == FALSE)
			nPacketLength = 131;
		else
			nPacketLength = 147;
	}

	if (fProgramStream)
	{
		StringCchCopy(szSearchFailureReason, sizeof(szSearchFailureReason), "This file appears to be an MPEG-2 Program Stream. TSReader only works with MPEG-2 Transport Streams");
		fSearchThreadRunning = FALSE;
		PostMessage(hDlg, WM_USER + 1, 0, 0);
		return 0;
	}

	if (fAbortSearchThread)
	{
		StringCchCopy(szSearchFailureReason, sizeof(szSearchFailureReason), "Sync search aborted");
		fSearchThreadRunning = FALSE;
		PostMessage(hDlg, WM_USER + 1, 0, 0);
		return 0;
	}

	if (dwFileStartOffset == (dwMapSize - (204 * 2)))
	{
		// Couldn't determine file type
		StringCchCopy(szSearchFailureReason, sizeof(szSearchFailureReason), "Unable to determine type of file. Ensure this is an MPEG-2 or DSS transport stream file");
		fSearchThreadRunning = FALSE;
		PostMessage(hDlg, WM_USER + 1, 0, 0);
		return 0;
	}

	SetFilePointer(hInputStream, dwFileStartOffset, NULL, FILE_BEGIN);
	if (fRateControlled && fRateControlAuto)
	{
		int nTemp;

		if (!fDSSMode)
			nTemp = CalculateMPEGBitrate(hDlg);
		else
			nTemp = CalculateDSSBitrate(hDlg);
		if (nTemp == -1)
		{
			if (fAbortSearchThread)
				StringCchCopy(szSearchFailureReason, sizeof(szSearchFailureReason), "Bitrate calculation aborted");
			else
				StringCchCopy(szSearchFailureReason, sizeof(szSearchFailureReason), "Unable to calculate bitrate - please use the manual bitrate function");
			fSearchThreadRunning = FALSE;
			PostMessage(hDlg, WM_USER + 1, 0, 0);
			return 0;
		}
		nRateManual = nTemp;
		SetFilePointer(hInputStream, dwFileStartOffset, NULL, FILE_BEGIN);
	}
	
	fSearchThreadRunning = FALSE;
	PostMessage(hDlg, WM_COMMAND, IDOK, 0);

	return 0;
}

void AbortSearchThread(void)
{
	fAbortSearchThread = TRUE;

	while (fSearchThreadRunning)
		Sleep(1);	
}

BOOL CALLBACK SearchSyncDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg)
	{
	case WM_INITDIALOG:
		{
			HANDLE hThread;
			DWORD dwThreadID;
			DWORD64 dwFileSize;
			FILETIME ft;

			ft.dwLowDateTime = GetFileSize(hInputStream, &ft.dwHighDateTime);
			memcpy(&dwFileSize, &ft, sizeof(DWORD64));
			if (dwFileSize > (DWORD64)0x10000000)
				dwFileSize = (DWORD64)0x10000000;	// 0.25GB

			SendDlgItemMessage(hDlg, IDC_SYNC_PROGRESS, PBM_SETRANGE32, 0, (DWORD)dwFileSize);
			fAbortSearchThread = FALSE;
			fSearchThreadRunning = FALSE;
			hThread = CreateThread(NULL, 0, SearchMPEGFileForSyncThread, (LPVOID)hDlg, 0, &dwThreadID);
			CloseHandle(hThread);
		}
		break;
	case WM_COMMAND:
		switch(LOWORD(wParam))
		{
		case IDCANCEL:
			dbg_printf("File: Aborting sync search\n");
			AbortSearchThread();
			EndDialog(hDlg, FALSE);
			break;
		case IDOK:
			EndDialog(hDlg, TRUE);
			break;
		}
		break;
	case WM_CLOSE:
		AbortSearchThread();
		EndDialog(hDlg, FALSE);
		break;
	case WM_DESTROY:
		while (fSearchThreadRunning == TRUE)
			Sleep(10);
		break;
	case WM_USER + 1:
		MessageBox(hDlg, szSearchFailureReason, gszSourceName, MB_ICONSTOP);
		EndDialog(hDlg, FALSE);
		break;
	}

	return FALSE;
}

BOOL OpenTransportFile(wchar_t *szMPEGTSFilename)
{
	BOOL fRetVal;
	wchar_t szTemp[MAX_PATH];

	hInputStream = CreateFileW(szMPEGTSFilename, GENERIC_READ, FILE_SHARE_READ, (LPSECURITY_ATTRIBUTES)NULL, OPEN_EXISTING, FILE_FLAG_SEQUENTIAL_SCAN, (HANDLE)NULL);
	if (hInputStream == INVALID_HANDLE_VALUE)
	{
		wchar_t szMessage[MAX_PATH + 100];
		char szMsgBuf[MAX_PATH];
		
		FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
					  NULL,
					  GetLastError(),
					  MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
					  szMsgBuf,
					  sizeof(szMsgBuf),
					  NULL);
		StringCchPrintfW(szMessage, _countof(szMessage), L"Unable to open file:\n\n%s\n\n%S", szMPEGTSFilename, szMsgBuf);
		if (!ss->fQuietMode)
			MessageBoxW(ss->hWndTSReader, szMessage, gszSourceNameW, MB_OK | MB_ICONSTOP);
		return FALSE;
	}

	/* get file size for progress report */
	g_nInputFileSize.LowPart = GetFileSize(hInputStream, &g_nInputFileSize.HighPart);
	g_nCurrentReadSize.QuadPart = 0;

	FormatLongFileName(szMPEGTSFilename);

	// Flag if this is a .ts file
	lstrcpyW(szTemp, szMPEGTSFilename);
	_wcslwr(szTemp);
	if (wcsstr(szTemp, L".ts") != NULL)
		ss->fLastFileTS = TRUE;
	else if (wcsstr(szTemp, L".mpg") != NULL)
		ss->fLastFileTS = FALSE;
	else if (wcsstr(szTemp, L".tp") != NULL)
		ss->fLastFileTS = 2;
	else
		ss->fLastFileTS = 3;

	// Save filename for later in case we have a .TS file
	lstrcpyW(szFileModeFilename, szMPEGTSFilename);

	fRetVal = DialogBox((HINSTANCE)hInstance, MAKEINTRESOURCE(IDD_SEARCH_SYNC), NULL, SearchSyncDlgProc);
	if (fRetVal == FALSE)
		CloseHandle(hInputStream);
	return fRetVal;
}

BOOL TSReader_Tune(void)
{
	return TRUE;
}

BOOL TSReader_TuneDialog(HWND hWnd)
{
#ifndef DIALOG_TEST
	szAlternateFilename[0] = L'\0';
	if (lstrlen(ss->szDropFilename) == 0)
	{
		if (lstrlenW(szCmdLinePtr) == 0)
		{
			if (GetInputMPEGFilename(hWnd, szAlternateFilename) == FALSE)
				return FALSE;
			szCmdLinePtr = szAlternateFilename;
		}
	}
	else
	{
		mbstowcs(szAlternateFilename, ss->szDropFilename, lstrlenA(ss->szDropFilename));
		ss->szDropFilename[0] = 0;
		szCmdLinePtr = szAlternateFilename;
	}

	if (OpenTransportFile(szCmdLinePtr) == FALSE)
		return FALSE;
#else DIALOG_TEST
	if (SourceHelper_ADVTuneDialog(hWnd) == FALSE)
		return FALSE;
#endif DIALOG_TEST
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
		lstrcpy(szCommandLineParameters, "{filename}");
	if (fCanBeStopped != NULL)
		*fCanBeStopped = TRUE;
	if (nMaxPIDs != NULL)
		*nMaxPIDs = 8192;
	if (dwCapabilities != NULL)
#ifndef DIALOG_TEST
		*dwCapabilities = 0;
#else DIALOG_TEST
		*dwCapabilities = CAPABILITIES_POWER
		                | CAPABILITIES_DISEQC
						| CAPABILITIES_TONEBURST
						| CAPABILITIES_DISEQC_POSITIONER
						| CAPABILITIES_DISH_SWITCH
						| CAPABILITIES_ADV_SATELLITE;
#endif DIALOG_TEST

	return TRUE;
}

BOOL TSReader_ParseCommandLine(PSOURCESTRUCT pss, char *szCommandLine, BOOL fQuiet)
{
	if (lstrlenA(szCommandLine)) {
		mbstowcs(szCmdLineW, szCommandLine, lstrlenA(szCommandLine));
		szCmdLinePtr = szCmdLineW;

		if (szCmdLinePtr[0] == '"')
			szCmdLinePtr++;
		if (szCmdLinePtr[lstrlenW(szCmdLinePtr) - 1] == L'"')
			szCmdLinePtr[lstrlenW(szCmdLinePtr) - 1] = L'\0';
	}

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
	if (g_nInputFileSize.QuadPart == 0 || g_nCurrentReadSize.QuadPart == 0) {
		lstrcpy(szString, "n/a");
		return TRUE;
	}

	StringCchPrintf(szString, 64, "File Read: %.1f%%", ((double)g_nCurrentReadSize.QuadPart / (double)g_nInputFileSize.QuadPart) * 100.0);

	return TRUE;
}

static int mywcstombs(char *dest, int len, const wchar_t *src)
{
	int bytes_needed = WideCharToMultiByte(CP_UTF8, 0, src, -1, NULL, 0, NULL, NULL);

	if (!bytes_needed)
		return 0;

	/* trim */
	if (bytes_needed > len)
		bytes_needed = len - 1;

	return WideCharToMultiByte(CP_UTF8, 0, src, -1, dest, bytes_needed, NULL, NULL);
}

BOOL TSReader_GetTunerString(char *szString)
{
	EnterCriticalSection(&g_csFileAccess);
	mywcstombs(szString, MAX_PATH, szShortFileName);
	LeaveCriticalSection(&g_csFileAccess);

	return TRUE;
}

BOOL TSReader_GetMiscString(char *szString)
{
#ifdef LOOP
	wsprintf(szString, "File Loops: %d", g_nFileLoops);
#endif

	return TRUE;
}

BOOL TSReader_SendDiSEqC(BYTE *bCommand, int nLength)
{
	return TRUE;
}

BOOL APIENTRY DllMain(HANDLE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
    switch( ul_reason_for_call )
	{
    case DLL_PROCESS_ATTACH:
		hInstance = (HINSTANCE)hModule;
		break;
    case DLL_PROCESS_DETACH:
		break;
    }
    return TRUE;
}
