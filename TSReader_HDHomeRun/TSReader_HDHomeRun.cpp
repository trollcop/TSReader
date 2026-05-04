#include <windows.h>
//#include <winsock.h>
#include <commctrl.h>
//#include <setupapi.h>
//#include <initguid.h>
#include <stdio.h>
#include <commctrl.h>

#include "resource.h"
#include "..\sources.h"
#include "libhdhomerun\hdhomerun.h"

PSOURCESTRUCT ss;
BOOL fNeedTuneDialog = TRUE;

HINSTANCE hInstance;
int nFrequency;
BOOL fDontPromptForMap = FALSE;
int nHDHRDeviceIndex = -1;
int nTunerID;
int nHDHRCount;
CRITICAL_SECTION csSignal;
struct hdhomerun_discover_device_t hdhr_result_list[64];
struct hdhomerun_device_t *myhd = NULL;

#define DEBUG_NAME "HDHR:"
#ifndef QAM
 char gszSourceName[] = {"HDHomeRun 8VSB"};
#else QAM
 char gszSourceName[] = {"HDHomeRun QAM"};
#endif QAM
char szLastSignalReport[128] = {"n/a"};
char szLastTune[128] = {"n/a"};
char gszHDHRSourceKeyName[] = {"Software\\COOL.STF\\TSReader\\HDHomeRunSource"};

void LoadSettings()
{
	DWORD dwDisposition;
	DWORD dwDataSize;
	DWORD dwType;
	LONG lKey;
	HKEY hkMainReg;

	lKey = RegCreateKeyEx(HKEY_CURRENT_USER,
		                  gszHDHRSourceKeyName,
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
			dwDataSize = sizeof(fDontPromptForMap);
			RegQueryValueEx(hkMainReg, "DontPromptForMap", NULL, &dwType, (BYTE *)&fDontPromptForMap, &dwDataSize);
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
		                  gszHDHRSourceKeyName,
						  0,
						  gszSourceName,
					      REG_OPTION_NON_VOLATILE,
						  KEY_ALL_ACCESS,
						  NULL,
						  &hkMainReg,
						  &dwDisposition);
	if (lKey == ERROR_SUCCESS)
	{
		RegSetValueEx(hkMainReg, "DontPromptForMap", 0, REG_DWORD, (BYTE *)&fDontPromptForMap, sizeof(DWORD));		
		RegCloseKey(hkMainReg);
	}
}

DWORD WINAPI ReadHDHRThread(LPVOID lpv)
{
	int nFirstPackets = 0;
	int nStatusCounter = 1;

	SourceHelper_StartSyncThread(ss, FALSE);
	ss->fReadThreadTerminated = FALSE;
	ss->fTerminateReadThread = FALSE;

	hdhomerun_device_stream_start(myhd, 0);
	while (!ss->fTerminateReadThread)
	{
		size_t actual_size;
		uint8_t *data = hdhomerun_device_stream_recv(myhd, 1024 * 1024, &actual_size);
		if (actual_size)
		{
			if (nFirstPackets < 5)
				nFirstPackets++;
			else
				SourceHelper_SyncData((BYTE *)data, actual_size);
		}
		else
		{
			if (--nStatusCounter == 0)
			{
				struct hdhomerun_tuner_status_t status;
				char szLock[16];

				hdhomerun_device_get_tuner_status(myhd, &status);
				EnterCriticalSection(&csSignal);
				if (status.signal_to_noise_quality > 25)
					lstrcpy(szLock, "Locked");
				else
					lstrcpy(szLock, "Unlocked");
				wsprintf(szLastSignalReport, "%s Signal %d Quality %d", szLock, status.signal_strength, status.signal_to_noise_quality);
				LeaveCriticalSection(&csSignal);

				nStatusCounter = 100;
			}
			Sleep(5);
		}
	}
	hdhomerun_device_stream_stop(myhd);

	SourceHelper_StopSyncThread();
	CloseHandle(ss->hReadDataThread);
	ss->fReadThreadTerminated = TRUE;
	ss->fTerminateReadThread = FALSE;
	return 0;
}

BOOL TSReader_Start()
{
	DWORD dwThreadID;

	ss->hReadDataThread = CreateThread(NULL, 0, ReadHDHRThread, (LPVOID)0, CREATE_SUSPENDED, &dwThreadID);
	SourceHelper_SetWorkerThreadPriorities(FALSE);
	ResumeThread(ss->hReadDataThread);

	return TRUE;
}

BOOL TSReader_Stop()
{
	ss->fTerminateReadThread = TRUE;
	
	while (ss->fReadThreadTerminated == FALSE)
		Sleep(50);

	EnterCriticalSection(&ss->csTSBuffersInUse);
	ss->nTSBuffersInUse = -1000;
	LeaveCriticalSection(&ss->csTSBuffersInUse);

	return TRUE;
}


int __cdecl SortHDHRList(const void *elem1, const void *elem2)
{
	struct hdhomerun_discover_device_t * pHDHR1 = (struct hdhomerun_discover_device_t *)elem1;
	struct hdhomerun_discover_device_t * pHDHR2 = (struct hdhomerun_discover_device_t *)elem2;

	if (pHDHR1->device_id < pHDHR2->device_id)
		return -1;
	if (pHDHR1->device_id > pHDHR2->device_id)
		return 1;

	return 0;
}

BOOL TSReader_Init(PSOURCESTRUCT pss)
{
	ss = pss;	
	LoadSettings();
	nHDHRCount = hdhomerun_discover_find_devices(HDHOMERUN_DEVICE_TYPE_TUNER, hdhr_result_list, 64);
	qsort(hdhr_result_list, nHDHRCount, sizeof(hdhomerun_discover_device_t), SortHDHRList);			
	InitializeCriticalSection(&csSignal);
	return TRUE;
}


BOOL TSReader_DeInit()
{
	if (myhd != NULL)
	{
		hdhomerun_device_destroy(myhd);
		myhd = NULL;
	}
	SaveSettings();
	DeleteCriticalSection(&csSignal);
	return TRUE;
}

BOOL TSReader_Tune()
{
	DWORD dwTimeout;
	char szValue[128];

	// Open the connection if it's not already
	if (myhd == NULL)
	{
		if (nHDHRDeviceIndex == -1)	// not setup by selection dialog
		{
			nHDHRDeviceIndex = ss->nSourceIndex >> 1;
			nTunerID = ss->nSourceIndex & 1;
		}
		myhd = hdhomerun_device_create(hdhr_result_list[nHDHRDeviceIndex].device_id, hdhr_result_list[nHDHRDeviceIndex].ip_addr, nTunerID);
		if (myhd == NULL)
			return FALSE;
	}

#ifndef QAM
	wsprintf(szValue, "8vsb:%d", ss->nFrequency * 1000 * 1000);
	wsprintf(szLastTune, "Channel %d (%d MHz)", SourceHelper_GetATSCChannelFromFrequency(ss->nFrequency), ss->nFrequency);
#else QAM
	if (SourceHelper_GetQAMHRCStatus() == FALSE)
		wsprintf(szValue, "qam:%d", ss->nFrequency * 1000 * 1000);
	else
	{
		// On HRC, the frequency is actually -1.25 MHz. The source helper
		// takes off the 1 MHz but we need to take the other 250 KHz off for the HDHR
		wsprintf(szValue, "qam:%d", (ss->nFrequency * 1000 * 1000) - (250 * 1000));
	}
	wsprintf(szLastTune, "Channel %d (%d MHz)", SourceHelper_GetQAMChannelFromFrequency(ss->nFrequency), ss->nFrequency);
#endif QAM
	hdhomerun_device_set_tuner_channel(myhd, szValue);
	Sleep(100);

	// See if we've got a lock
	dwTimeout = GetTickCount() + 1500;
	while (GetTickCount() < dwTimeout)
	{
		struct hdhomerun_tuner_status_t status;

		hdhomerun_device_get_tuner_status(myhd, &status);
		if (status.signal_to_noise_quality > 25)
			return TRUE;	// we're locked

		Sleep(100);		// give it more time
	}
	if (ss->fQuietMode == FALSE)
		MessageBox(ss->hWndTSReader, "Failed to lock signal", gszSourceName, MB_ICONWARNING);
	return FALSE;
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
		lstrcpy(szCommandLineParameters, "freq");
	if (dwCapabilities != NULL)
		*dwCapabilities = CAPABILITIES_MULTICARD;
	if (fCanBeStopped != NULL)
		*fCanBeStopped = FALSE;
	if (nMaxPIDs != NULL)
		*nMaxPIDs = 8192;
	return TRUE;
}

void GetHDHRDisplayInfo(LV_DISPINFO *pnmv) 
{ 
    // Provide the item or subitem's text, if requested. 
    if (pnmv->item.mask & LVIF_TEXT)
	{ 
        int nItemIndex = (int)(pnmv->item.iItem);

		if (nHDHRCount < 0)
		{
			if (pnmv->item.iSubItem == 0)
				lstrcpy(pnmv->item.pszText, "Network error");
			return;
		}
		if (nHDHRCount == 0)
		{
			if (pnmv->item.iSubItem == 0)
				lstrcpy(pnmv->item.pszText, "No HDHRs");
			return;
		}
		switch(pnmv->item.iSubItem)
		{
		case 0:		// The TSReader source index
			wsprintf(pnmv->item.pszText, "%d", pnmv->item.iItem);
			break;
		case 1:		// The device ID
			wsprintf(pnmv->item.pszText, "%08x", hdhr_result_list[pnmv->item.iItem >> 1].device_id);
			break;
		case 2:		// The tuner number
			wsprintf(pnmv->item.pszText, "%d", pnmv->item.iItem & 1);
			break;
		}
	}
}

BOOL CALLBACK HDHRMapDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg)
	{
	case WM_INITDIALOG:
		{
			int nColumnPosition = 0;
			HWND hWndMapList = GetDlgItem(hDlg, IDC_HDHR_MAP_LIST);
			LV_COLUMN lvc; 
			LV_ITEM lvi; 

			memset(&lvc, 0, sizeof(lvc));
			memset(&lvi, 0, sizeof(lvi));
			lvi.pszText = LPSTR_TEXTCALLBACK;
			lvi.mask = LVIF_TEXT | LVIF_STATE; 

			// Setup the list view
			lvc.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM; 
			lvc.fmt = LVCFMT_LEFT; 
			lvc.cx = 100; 
			lvc.pszText = "Source Index"; 
			ListView_InsertColumn(hWndMapList, nColumnPosition++, &lvc); 
			lvc.fmt = LVCFMT_LEFT; 
			lvc.cx = 100; 
			lvc.pszText = "HDHR Device";
			ListView_InsertColumn(hWndMapList, nColumnPosition++, &lvc); 
			lvc.fmt = LVCFMT_LEFT; 
			lvc.cx = 100; 
			lvc.pszText = "Tuner";
			ListView_InsertColumn(hWndMapList, nColumnPosition++, &lvc); 
			ListView_SetExtendedListViewStyle(hWndMapList, LVS_EX_FULLROWSELECT);

			// Populate the HDHR list - we got the list during the Init function
			if (nHDHRCount < 0)
			{
				// Network error - insert one list view item
				ListView_InsertItem(hWndMapList, &lvi);
			}
			else if (nHDHRCount == 0)
			{
				// No devices - again one item
				ListView_InsertItem(hWndMapList, &lvi);
			}
			else
			{
				int nIndex;
				int nItemIndex = 0;

				for(nIndex = 0; nIndex < nHDHRCount; nIndex++)
				{
					// Insert two items - one for each tuner
					ListView_InsertItem(hWndMapList, &lvi);
					lvi.iItem++; 
					ListView_InsertItem(hWndMapList, &lvi);
					lvi.iItem++; 
				}
			}
			if (nHDHRCount <= 0)
				EnableWindow(GetDlgItem(hDlg, IDOK), FALSE);
			SetFocus(GetDlgItem(hDlg, IDOK));
		}
		break;
	case WM_CLOSE:
		EndDialog(hDlg, FALSE);
		break;
	case WM_COMMAND:
		switch(LOWORD(wParam))
		{
		case IDOK:
			{
				// Find the selected item
				int nCount = ListView_GetItemCount(GetDlgItem(hDlg, IDC_HDHR_MAP_LIST));
				int nSelectedItem = -1;
				int nIndex;

				for (nIndex = 0; nIndex < nCount; nIndex++)
				{
					LV_ITEM lvItem;

					lvItem.mask = LVIF_STATE | LVIF_PARAM;
					lvItem.iItem = nIndex;
					lvItem.iSubItem = 0;
					lvItem.stateMask = LVIS_SELECTED;
					ListView_GetItem(GetDlgItem(hDlg, IDC_HDHR_MAP_LIST), &lvItem);
					if ((lvItem.state & LVIS_SELECTED) > 0)
					{
						nSelectedItem = nIndex;
						break;
					}
				}
				if (nSelectedItem == -1)
				{
					MessageBox(hDlg, "Please select the HDHomeRun unit/tuner to use", gszSourceName, MB_ICONSTOP);
					SetFocus(GetDlgItem(hDlg, IDC_HDHR_MAP_LIST));
					break;
				}
				nHDHRDeviceIndex = nSelectedItem >> 1;
				nTunerID = nSelectedItem & 1;
				EndDialog(hDlg, TRUE);
			}
			break;
		case IDCANCEL:
			EndDialog(hDlg, FALSE);
			break;
		case IDC_HDHR_DONT_SHOW:
			fDontPromptForMap = IsDlgButtonChecked(hDlg, IDC_HDHR_DONT_SHOW);
			break;
		}
		break;
	case WM_NOTIFY: 
		switch (((LPNMHDR) lParam)->code)
		{ 
		case LVN_GETDISPINFO:
			{
				NMLVDISPINFO * pnmv = (NMLVDISPINFO *)lParam;
				GetHDHRDisplayInfo((LV_DISPINFO *) lParam);
			}
			break;
		case NM_DBLCLK:
			if (nHDHRCount)
				PostMessage(hDlg, WM_COMMAND, IDOK, 0);
			break;
		}
		break;
	}
	return FALSE;
}

BOOL TSReader_TuneDialog(HWND hWnd)
{
	if (ss->fDontTune == TRUE)
		fNeedTuneDialog = FALSE;

	if (fNeedTuneDialog)
	{
		if (!fDontPromptForMap && nHDHRDeviceIndex == -1)
		{
			if (DialogBox((HINSTANCE)hInstance, MAKEINTRESOURCE(IDD_HDHR_MAP), ss->hWndTSReader, HDHRMapDlgProc) == FALSE)
				return FALSE;
		}
#ifndef QAM
		if (SourceHelper_ATSCTuneDialog(hWnd) == FALSE)
#else QAM
		if (SourceHelper_QAMTuneDialog(hWnd) == FALSE)
#endif QAM
			return FALSE;
	}
	else
	{
		ss->nFrequency = nFrequency;
		fNeedTuneDialog = TRUE;
	}

	return TRUE;
}

BOOL TSReader_ParseCommandLine(PSOURCESTRUCT ss, char * szCommandLine, BOOL fQuiet)
{
	fNeedTuneDialog = TRUE;
	if (lstrlen(szCommandLine))
	{
		int nConversionCount;

		nConversionCount = sscanf(szCommandLine,
								  "%d", 
								  &nFrequency);
		if (nConversionCount < 1)
		{
			if (!fQuiet)
				MessageBox(NULL,
					   "Usage for this source: freq\n"
					   "\n"
					   "freq = frequency to tune in MHz or prefix with 0 for channel number, e.g. 022 for channel 22",
					   gszSourceName,
					   MB_OK | MB_ICONSTOP);
			return FALSE;
		}
		if (*szCommandLine == '0')
#ifndef QAM
			nFrequency = SourceHelper_GetFrequencyFromATSCChannel(nFrequency);
#else QAM
			nFrequency = SourceHelper_GetFrequencyFromQAMChannel(nFrequency);
#endif QAM
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
	EnterCriticalSection(&csSignal);
	lstrcpy(szString, szLastSignalReport);
	LeaveCriticalSection(&csSignal);
	return TRUE;
}

BOOL TSReader_GetTunerString(char * szString)
{
	lstrcpy(szString, szLastTune);
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
