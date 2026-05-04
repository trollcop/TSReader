#include <windows.h>
#include <winsock.h>
#include <commctrl.h>
#include <setupapi.h>
#include <initguid.h>
#include <stdio.h>
#include <wininet.h>
#include "resource.h"

#include "..\sources.h"

PSOURCESTRUCT ss;
BOOL fNeedTuneDialog = TRUE;

char szTCPAddress[128];
int nTCPPort;
HINTERNET hInet;
HINTERNET hHTTPConnection;
HINSTANCE hInstance;

#define DEBUG_NAME "HTTP:"
char gszSourceName[] = {"HTTP"};
char gszLastTune[128] = {"n/a"};
char szURL[MAX_PATH] = {""};
char gszHTTPSourceKeyName[] = {"Software\\COOL.STF\\TSReader\\HTTPSource"};

BOOL __cdecl SourceHelper_ValidateSourceContainer(PSOURCESTRUCT pss);

int nTSBufferIndex;

void LoadSettings()
{
	DWORD dwDisposition;
	DWORD dwDataSize;
	DWORD dwType;
	LONG lKey;
	HKEY hkMainReg;

	lKey = RegCreateKeyEx(HKEY_CURRENT_USER,
		                  gszHTTPSourceKeyName,
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
			dwDataSize = sizeof(szURL);
			RegQueryValueEx(hkMainReg, "URL", NULL, &dwType, (BYTE *)szURL, &dwDataSize);
		}
		RegCloseKey(hkMainReg);
	}
}

void SaveSettings()
{
	LONG lKey;
	HKEY hkMainReg;
	DWORD dwDisposition;

	lKey = RegCreateKeyEx(HKEY_CURRENT_USER,
		                  gszHTTPSourceKeyName,
						  0,
						  gszSourceName,
					      REG_OPTION_NON_VOLATILE,
						  KEY_ALL_ACCESS,
						  NULL,
						  &hkMainReg,
						  &dwDisposition);
	if (lKey == ERROR_SUCCESS)
	{
		RegSetValueEx(hkMainReg, "URL", 0, REG_SZ, (BYTE *)szURL, lstrlen(szURL) + 1);
		RegCloseKey(hkMainReg);
	}
}

DWORD WINAPI ReadHTTPThread(LPVOID lpv)
{
	int nTSBufferIndex = 0;
	int nRemaining;
	int nOffset;
	BYTE bBuffer[64 * 1024];

	OutputDebugString(DEBUG_NAME" ReadHTTPThread+\n");

	SourceHelper_StartSyncThread(ss, FALSE);

	nTSBufferIndex = 0;
	ss->fReadThreadTerminated = FALSE;
	ss->fTerminateReadThread = FALSE;

	nRemaining = sizeof(bBuffer);
	nOffset = 0;
	while (!ss->fTerminateReadThread)
	{
		DWORD dwRead;
		BOOL fRetVal = InternetReadFile(hHTTPConnection, bBuffer, sizeof(bBuffer), &dwRead);
		if (dwRead == 0)
			break;		// EOF
		SourceHelper_SyncData(bBuffer, dwRead);
	}

	SourceHelper_StopSyncThread();
	CloseHandle(ss->hReadDataThread);
	ss->fReadThreadTerminated = TRUE;
	ss->fTerminateReadThread = FALSE;
	OutputDebugString(DEBUG_NAME" ReadHTTPThread-\n");
	return 0;
}

BOOL TSReader_Start()
{
	DWORD dwThreadID;

	ss->hReadDataThread = CreateThread(NULL, 0, ReadHTTPThread, (LPVOID)0, CREATE_SUSPENDED, &dwThreadID);
	SourceHelper_SetWorkerThreadPriorities(FALSE);
	ResumeThread(ss->hReadDataThread);

	return TRUE;
}

BOOL TSReader_Stop()
{
	OutputDebugString(DEBUG_NAME" Wait for read thread terminate\n");
	ss->fTerminateReadThread = TRUE;
	
	if (hHTTPConnection != NULL)
	{
		InternetCloseHandle(hHTTPConnection);
		hHTTPConnection = NULL;
	}

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
	if (SourceHelper_ValidateSourceContainer(pss) == FALSE)
		return FALSE;

	ss = pss;	

	hInet = InternetOpen("TSReader_HTTP", INTERNET_OPEN_TYPE_DIRECT, NULL, NULL, 0);
	if (hInet == NULL)
		return FALSE;

	LoadSettings();

	return TRUE;
}


BOOL TSReader_DeInit()
{
	if (hInet != NULL)
	{
		InternetCloseHandle(hInet);
		hInet = NULL;
	}
	
	SaveSettings();

	return TRUE;
}

BOOL TSReader_Tune()
{
	hHTTPConnection = InternetOpenUrl(hInet, szURL, NULL, 0, INTERNET_FLAG_RAW_DATA | INTERNET_FLAG_RELOAD, 0);
	if (hHTTPConnection == NULL)
	{
		MessageBox(NULL, "Unable to connect to URL", gszSourceName, MB_ICONSTOP);
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
		lstrcpy(szCommandLineParameters, "URL");
	if (dwCapabilities != NULL)
		*dwCapabilities = 0;
	if (fCanBeStopped != NULL)
		*fCanBeStopped = FALSE;
	if (nMaxPIDs != NULL)
		*nMaxPIDs = 8192;
	return TRUE;
}

BOOL CALLBACK HTTPTuneDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg)
	{
	case WM_INITDIALOG:
		SetDlgItemText(hDlg, IDC_URL, szURL);
		SendDlgItemMessage(hDlg, IDC_URL, EM_SETSEL, 0, -1);
		SetFocus(GetDlgItem(hDlg, IDC_URL));
		break;
	case WM_CLOSE:
		EndDialog(hDlg, FALSE);
		break;
	case WM_COMMAND:
		switch(LOWORD(wParam))
		{
		case IDOK:
			GetDlgItemText(hDlg, IDC_URL, szURL, sizeof(szURL));
			EndDialog(hDlg, TRUE);
			break;
		case IDCANCEL:
			EndDialog(hDlg, FALSE);
			break;
		}
		break;
	}

	return FALSE;
}

BOOL TSReader_TuneDialog(HWND hWnd)
{
	OutputDebugString(DEBUG_NAME" TSReader_TuneDialog\n");

	if (ss->fDontTune == TRUE)
		fNeedTuneDialog = FALSE;

	if (fNeedTuneDialog)
	{
		if (DialogBox((HINSTANCE)hInstance, MAKEINTRESOURCE(IDD_HTTP_TUNE_DIALOG), ss->hWndTSReader, HTTPTuneDlgProc) == FALSE)
			return FALSE;
	}

	return TRUE;
}

BOOL TSReader_ParseCommandLine(PSOURCESTRUCT ss, char * szCommandLine, BOOL fQuiet)
{
	if (lstrlen(szCommandLine))
	{
		lstrcpy(szURL, szCommandLine);
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
