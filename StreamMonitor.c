#include <windows.h>
#include <commctrl.h>
#include <shlobj.h>

#include "TSReader.h"
#include "bcdmux.h"
#include "util.h"
#include "CCDecoder.h"

#include "resource.h"

extern PVARIABLES v;
extern char gszAppName[];
extern char gszKeyName[];

// in TSReader.c
void GetSourceInfoLine(int nLine, char * szOutput);
BOOL GetPIDTooltipInfo(int nPID, char * szString);

double GetStreamMonitorTime()
{
	double dRetVal = 0.0;

	if (v->lnMuxRatePCR && !v->fStreamMonitorClockSystem)
	{
		dRetVal = (1.0/27000000.0) * (double)v->lnMuxRatePCR;
	}
	else
	{
		LARGE_INTEGER lnCount;

		QueryPerformanceCounter(&lnCount);
		dRetVal = (double)lnCount.QuadPart / (double)v->lnTicksPerSecond;
	}

	return dRetVal;
}

void TranslateAlarmCode(char * szText, int nAlarmCode, int nAlarmSubcode)
{
	switch(nAlarmCode)
	{
	case MONITOR_ETR290_1_1:
		lstrcpy(szText, "1.1: TS_sync_loss: Loss of synchronization with consideration of hysteresis parameters");
		break;
	case MONITOR_ETR290_1_2:
		lstrcpy(szText, "1.2: Sync_byte_error: Sync byte not equal 0x47");
		break;
	case MONITOR_ETR290_1_3:			// Continuity_count_error
		lstrcpy(szText, "1.3: PAT_error: ");
		switch(nAlarmSubcode)
		{
		case 0:
			lstrcat(szText, "PID 0x0000 does not occur at least every 0.5 seconds");
			break;
		case 1:
			lstrcat(szText, "PID 0x0000 does not contain a table_id 0x00 ( i.e. a PAT)");
			break;
		case 2:
			lstrcat(szText, "Scrambling_control_field is not 00 for PID 0x0000");
			break;
		}
		break;
	case MONITOR_ETR290_1_4:			// Continuity_count_error
		lstrcpy(szText, "1.4: Continuity_count_error: Incorrect packet order");
		break;
	case MONITOR_ETR290_1_5:
		lstrcpy(szText, "1.5: PMT_error: ");
		switch(nAlarmSubcode)
		{
		case 0:
			lstrcat(szText, "Sections with table_id 0x02, do not occur at least every 0.5 seconds");
			break;
		case 1:
			lstrcat(szText, "Scrambling_control_field is not 00 for all PIDs containing sections with table_id 0x02");
			break;
		}
		break;
	case MONITOR_ETR290_2_1:			// Transport_error
		lstrcpy(szText, "2.1: Transport_error: Transport_error_indicator is the TS-Header is set to \"1\"");
		break;
	case MONITOR_ETR290_2_2:			// CRC_error
		lstrcpy(szText, "2.2: CRC_error: CRC error occured in CAT, PAT, PMT, NIT, EIT, BAT, SDT or TOT table");
		break;
	case MONITOR_ETR290_2_3:			// PCR_error
		lstrcpy(szText, "2.3: PCR_error: Time interval between two consecutive PCR values more than 40 ms");
		break;
	case MONITOR_ETR290_2_6:			// CAT_error
		lstrcpy(szText, "2.6: CAT_error: Scrambled packets with no CAT or CAT with table_id other than 0x01");
		break;
	case MONITOR_ETR290_3_1:			// NIT_error
		lstrcpy(szText, "3.1: NIT_error: ");
		switch(nAlarmSubcode)
		{
		case 0:
			lstrcat(szText, "Section with table_id other than 0x40 or 0x41 or 0x72 found on PID 0x0010");
			break;
		case 1:
			lstrcat(szText, "No section with table_id 0x40 or 0x41 in PID 0x0010 for more than 10 seconds");
			break;
		}
		break;
	case MONITOR_ETR290_3_4:			// Unreferenced_PID
		lstrcpy(szText, "3.4: Unreferenced_PID: One of more PIDs not referenced by PMT/PSI tables");
		break;
	case MONITOR_ETR290_3_5:
		lstrcpy(szText, "3.5: SDT_error: ");
		switch(nAlarmSubcode)
		{
		case 0:
			lstrcat(szText, "Sections with table_id = 0x42 (SDT, actual TS) not present on PID 0x0011 for more than 2 seconds");
			break;
		case 1:
			lstrcat(szText, "Sections with table_ids other than 0x42, 0x46, 0x4A or 0x72 found on PID 0x0011");
			break;
		}
		break;
	case MONITOR_ETR290_3_6:
		lstrcpy(szText, "3.6: EIT_error: ");
		switch(nAlarmSubcode)
		{
		case 0:
			lstrcat(szText, "Sections with table_id = 0x4E (EIT-P/F, actual TS) not present on PID 0x0012 for more than 2 seconds");
			break;
		case 1:
			lstrcat(szText, "Sections with table_ids other than in the range 0x4E - 0x6F or 0x72 found on PID 0x0012");
			break;
		}
		break;
	case MONITOR_ETR290_3_7:			// RST_error
		lstrcpy(szText, "3.7: RST_error: Sections with table other than 0x71/0x72 on PID 0x0013");
		break;
	case MONITOR_ETR290_3_8:			// TDT_error
		lstrcpy(szText, "3.8: TDT_error: ");
		switch(nAlarmSubcode)
		{
		case 0:
			lstrcat(szText, "Sections with table_id = 0x70 (TDT) not present on PID 0x0014 for more than 30 seconds");
			break;
		case 1:
			lstrcat(szText, "Sections with table_id other than 0x70, 0x72 (ST) or 0x73 (TOT) found on PID 0x0014");
			break;
		}
		break;
	default:
		wsprintf(szText, "Not implemented yet (nAlarmCode = %d)", nAlarmCode);
		break;
	}
}

void OpenStreamMonitorLogFile()
{
	BOOL fNewFile = FALSE;
	SYSTEMTIME stNow;

	GetLocalTime(&stNow);
	if (v->hStreamMonitorLog == NULL)
		fNewFile = TRUE;
	else
	{
		if (   stNow.wYear != v->stStreamMonitorFile.wYear
			|| stNow.wMonth != v->stStreamMonitorFile.wMonth
			|| stNow.wDay != v->stStreamMonitorFile.wDay)
		{
			fNewFile = TRUE;
			CloseHandle(v->hStreamMonitorLog);
			v->hStreamMonitorLog = NULL;
		}
	}
	if (fNewFile)
	{
		char szStreamMonitorLog[MAX_PATH];

		wsprintf(szStreamMonitorLog, "StreamMonitor-%04d%02d%02d.txt",
				 stNow.wYear, stNow.wMonth, stNow.wDay);
		v->hStreamMonitorLog = CreateFile(szStreamMonitorLog, GENERIC_WRITE, 0, (LPSECURITY_ATTRIBUTES) NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, (HANDLE) NULL);
		SetFilePointer(v->hStreamMonitorLog, 0, NULL, FILE_END);	
		memcpy(&v->stStreamMonitorFile, &stNow, sizeof(SYSTEMTIME));
	}
}

DWORD WINAPI EmailThread(LPVOID pPtr);

void MonitorAlarmOn(int nAlarmIndex, HWND hDlg, int nAlarmSubcode)
{
	DWORD dwWritten;
	LV_ITEM lvi; 
	SYSTEMTIME stLocal;
	char szLogDescription[1024];
	char szAlarmMessage[2048];

	memset(&lvi, 0, sizeof(lvi));

	v->sm[nAlarmIndex].nStatus = 2;	// alarm
	PostMessage(hDlg, WM_USER + 1, nAlarmIndex, v->sm[nAlarmIndex].nStatus);
	v->sm[nAlarmIndex].lnLastAlarmTime = 0;

	v->sml[v->nStreamMonitorCurrent].nAlarmCode = nAlarmIndex;
	v->sml[v->nStreamMonitorCurrent].nAlarmSubcode = nAlarmSubcode;
	GetSystemTime(&v->sml[v->nStreamMonitorCurrent].st);
	
	TranslateAlarmCode(szLogDescription, nAlarmIndex, nAlarmSubcode);
	SystemTimeToTzSpecificLocalTime(NULL, &v->sml[v->nStreamMonitorCurrent].st, &stLocal);
	wsprintf(szAlarmMessage, "%04d:%02d:%02d %02d:%02d:%02d\t%s\r\n", 
		     stLocal.wYear, stLocal.wMonth, stLocal.wDay,
			 stLocal.wHour, stLocal.wMinute, stLocal.wSecond,
			 szLogDescription);
	OpenStreamMonitorLogFile();
	WriteFile(v->hStreamMonitorLog, szAlarmMessage, lstrlen(szAlarmMessage), &dwWritten, NULL);
	FlushFileBuffers(v->hStreamMonitorLog);
	if (v->fStreamMonitorEmailEnabled)
	{
		DWORD dwThreadID;
		HANDLE hThread;
		PEVENTEMAILITEM pEmail = LocalAlloc(LPTR, sizeof(EVENTEMAILITEM));
		char * szEmailBody = LocalAlloc(LPTR, lstrlen(szAlarmMessage) + 1024);
		char * szColon;
		char szShortLogDescription[1024];

		lstrcpy(szShortLogDescription, szLogDescription);
		szColon = strstr(szShortLogDescription, ":");
		if (szColon != NULL)
		{
			szColon = strstr(szColon + 1, ":");
			if (szColon != NULL)
				*szColon = '\0';
		}
		wsprintf(szEmailBody,
				 "Subject: %s\r\n"
				 "\r\n"
				 "%s\r\n",
				 szShortLogDescription, szAlarmMessage);

		lstrcpy(pEmail->szSMTPServer, v->szSMTPServer);
		lstrcpy(pEmail->szEmailAddress, v->szEmailDestination);
		lstrcpy(pEmail->szEmailFrom, v->szEmailFrom);
		pEmail->szMessageBody = szEmailBody;

		hThread = CreateThread(NULL, 0, EmailThread, (LPVOID)pEmail, CREATE_SUSPENDED, &dwThreadID);
		SetThreadPriority(hThread, THREAD_PRIORITY_BELOW_NORMAL);
		ResumeThread(hThread);	
		CloseHandle(hThread);
	}

	lvi.pszText = LPSTR_TEXTCALLBACK;   // app. maintains text 
	lvi.mask = LVIF_TEXT | LVIF_PARAM | LVIF_STATE; 
	lvi.iItem = 0; 
	lvi.lParam = (LPARAM)v->nStreamMonitorCurrent;
	ListView_InsertItem(GetDlgItem(hDlg, IDC_MONITOR_LIST), &lvi);

	v->nStreamMonitorCurrent++;
	if (v->nStreamMonitorCurrent == v->nStreamMonitorMax)
	{
		STREAMMONITORLOG * sml_new;
		v->nStreamMonitorMax += 100;
		sml_new = LocalAlloc(LPTR, sizeof(STREAMMONITORLOG) * v->nStreamMonitorMax);
		memcpy(sml_new, v->sml, sizeof(STREAMMONITORLOG) * v->nStreamMonitorCurrent);
		LocalFree(v->sml);
		v->sml = sml_new;
	}
}

int GetTotalCRCErrors()
{
	int nRetVal = 0;
	int i;

	for (i = 0; i < 16; i++)
		nRetVal += (int)v->nSIParserCRCs[i];

	return nRetVal;
}
/////
int nMonitorScrambledPATs;
int nMonitorScrambledPMTs;
int nMonitorPCRDelayError;
/////

DWORD WINAPI StreamMonitorThread(LPVOID lpv)
{
	HWND hDlg = (HWND)lpv;
	int nIndex;

	////
	int nMonitorCATCounter;
	int nMonitorRSTCounter;
	int nMonitorScrambledPATCounter;
	int nMonitorScrambledPMTCounter;
	int nMonitorPCRDelayCounter;
	int nMonitorNITTimingErrors;
	int nMonitorNITTableErrors;
	int nMonitorSDTTimingErrors;
	int nMonitorSDTTableErrors;
	int nMonitorEITTimingErrors;
	int nMonitorEITTableErrors;
	int nMonitorTDTTimingErrors;
	int nMonitorTDTTableErrors;
	////

	for (nIndex = 0; nIndex < MONITOR_COUNT; nIndex++)
	{
		if (v->sm[nIndex].fDisabled)
			v->sm[nIndex].nStatus = 0;
		else
			v->sm[nIndex].nStatus = 1;
		PostMessage(hDlg, WM_USER + 1, nIndex, v->sm[nIndex].nStatus);
	}
	v->nMonitorSyncLossCount = -1;
	v->nMonitorContinuityErrors = -1;
	v->nMonitorTEICounter = -1;
	v->nMonitorCRCCounter = -1;
	v->nMonitorPATTimingErrors = -1;
	v->nMonitorPATTableErrors = -1;
	v->nMonitorPMTTimingErrors = -1;
	nMonitorCATCounter = -1;
	nMonitorRSTCounter = -1;
	nMonitorScrambledPATCounter = -1;
	nMonitorScrambledPMTCounter = -1;
	nMonitorPCRDelayCounter = -1;
	nMonitorNITTimingErrors = -1;
	nMonitorNITTableErrors = -1;
	nMonitorSDTTimingErrors = -1;
	nMonitorSDTTableErrors = -1;
	nMonitorEITTimingErrors = -1;
	nMonitorEITTableErrors = -1;
	nMonitorTDTTimingErrors = -1;
	nMonitorTDTTableErrors = -1;
	
	while (!v->fMonitorTerminateThread)
	{
		for (nIndex = 0; nIndex < MONITOR_COUNT; nIndex++)
		{
			if (v->sm[nIndex].fDisabled)
				continue;

			switch(nIndex)
			{
			case MONITOR_ETR290_1_1:			// TS_sync_loss
				{
					int nNewStatus = -1;
					char szSignal[64];

					GetSourceInfoLine(2, szSignal);
					strlwr(szSignal);
					if (strstr(szSignal, "unlocked") != NULL)
					{
						nNewStatus = 2;
					}
					else if (strstr(szSignal, "locked") != NULL)
					{
						nNewStatus = 1;
					}
					if (nNewStatus == -1)
						nNewStatus = 0;
					if (nNewStatus != v->sm[nIndex].nStatus)
					{
						if (nNewStatus != 0)
							MonitorAlarmOn(nIndex, hDlg, 0);
						else
						{
							PostMessage(hDlg, WM_USER + 1, nIndex, nNewStatus);
						}
					}
				}
				break;
			case MONITOR_ETR290_1_2:			// Sync_byte_error
				{
					if (v->nMonitorSyncLossCount == -1)
						v->nMonitorSyncLossCount = v->nSyncLossCount;
					else
					{
						if (v->nMonitorSyncLossCount != v->nSyncLossCount)
						{
							v->nMonitorSyncLossCount = v->nSyncLossCount;
							MonitorAlarmOn(nIndex, hDlg, 0);
						}
					}
				}
				break;
			case MONITOR_ETR290_1_3:			// PAT_error
				{
					// Check timing (every 500 ms)
					if (v->nMonitorPATTimingErrors == -1)
						v->nMonitorPATTimingErrors = v->nSIParserTimingErrors[SI_PARSER_STATS_PAT];
					else
					{
						if (v->nMonitorPATTimingErrors != v->nSIParserTimingErrors[SI_PARSER_STATS_PAT])
						{
							v->nMonitorPATTimingErrors = v->nSIParserTimingErrors[SI_PARSER_STATS_PAT];
							MonitorAlarmOn(nIndex, hDlg, 0);
						}
					}

					// Check table errors
					if (v->nMonitorPATTableErrors == -1)
						v->nMonitorPATTableErrors = v->nSIParserTableErrors[SI_PARSER_STATS_PAT];
					else
					{
						if (v->nMonitorPATTableErrors != v->nSIParserTableErrors[SI_PARSER_STATS_PAT])
						{
							v->nMonitorPATTableErrors = v->nSIParserTableErrors[SI_PARSER_STATS_PAT];
							MonitorAlarmOn(nIndex, hDlg, 1);
						}
					}

					// Check scrambling for PAT
					if (nMonitorScrambledPATCounter == -1)
						nMonitorScrambledPATCounter = nMonitorScrambledPATs;
					else
					{
						if (nMonitorScrambledPATCounter != nMonitorScrambledPATs)
						{
							nMonitorScrambledPATCounter = nMonitorScrambledPATs;
							MonitorAlarmOn(nIndex, hDlg, 2);
						}
					}
				}
				break;
			case MONITOR_ETR290_1_4:			// Continuity_count_error
				{
					if (v->nMonitorContinuityErrors == -1)
						v->nMonitorContinuityErrors = v->nContinuityErrors;
					else
					{
						if (v->nMonitorContinuityErrors != v->nContinuityErrors)
						{
							v->nMonitorContinuityErrors = v->nContinuityErrors;
							MonitorAlarmOn(nIndex, hDlg, 0);
						}
					}				
				}
				break;
			case MONITOR_ETR290_1_5:
				{
					// Check timing (every 500 ms)
					if (v->nMonitorPMTTimingErrors == -1)
						v->nMonitorPMTTimingErrors = v->nSIParserTimingErrors[SI_PARSER_STATS_PMT];
					else
					{
						if (v->nMonitorPMTTimingErrors != v->nSIParserTimingErrors[SI_PARSER_STATS_PMT])
						{
							v->nMonitorPMTTimingErrors = v->nSIParserTimingErrors[SI_PARSER_STATS_PMT];
							MonitorAlarmOn(nIndex, hDlg, 0);
						}
					}

					// Check scrambling for PAT
					if (nMonitorScrambledPMTCounter == -1)
						nMonitorScrambledPMTCounter = nMonitorScrambledPMTs;
					else
					{
						if (nMonitorScrambledPMTCounter != nMonitorScrambledPMTs)
						{
							nMonitorScrambledPMTCounter = nMonitorScrambledPMTs;
							MonitorAlarmOn(nIndex, hDlg, 1);
						}
					}
				}
				break;
			case MONITOR_ETR290_2_1:			// Transport_error
				{
					if (v->nMonitorTEICounter == -1)
						v->nMonitorTEICounter = v->nTEIErrors;
					else
					{
						if (v->nMonitorTEICounter != v->nTEIErrors)
						{
							v->nMonitorTEICounter = v->nTEIErrors;
							MonitorAlarmOn(nIndex, hDlg, 0);
						}
					}				
				}
				break;
			case MONITOR_ETR290_2_2:			// CRC_error
				{
					if (v->nMonitorCRCCounter == -1)
						v->nMonitorCRCCounter = GetTotalCRCErrors();
					else
					{
						if (v->nMonitorCRCCounter != GetTotalCRCErrors())
						{
							v->nMonitorCRCCounter = GetTotalCRCErrors();
							MonitorAlarmOn(nIndex, hDlg, 0);
						}
					}
				}
				break;
			case MONITOR_ETR290_2_3:			// PCR_error
				{
					if (nMonitorPCRDelayCounter == -1)
						nMonitorPCRDelayCounter = nMonitorPCRDelayError;
					else
					{
						if (nMonitorPCRDelayCounter != nMonitorPCRDelayError)
						{
							nMonitorPCRDelayCounter = nMonitorPCRDelayError;
							MonitorAlarmOn(nIndex, hDlg, 0);
						}
					}
				}
				break;
			case MONITOR_ETR290_2_6:			// CAT_error
				{
					if (nMonitorCATCounter == -1)
						nMonitorCATCounter = v->nSIParserTableErrors[SI_PARSER_STATS_CAT];
					else
					{
						if (v->nSIParserTableErrors[SI_PARSER_STATS_CAT] != nMonitorCATCounter)
						{
							nMonitorCATCounter = v->nSIParserTableErrors[SI_PARSER_STATS_CAT];
							if (v->sm[nIndex].nStatus != 2)
								MonitorAlarmOn(nIndex, hDlg, 0);
						}
					}				
				}
				break;
			case MONITOR_ETR290_3_1:			// NIT_error
				{
					// Check table errors
					if (nMonitorNITTableErrors == -1)
						nMonitorNITTableErrors = v->nSIParserTableErrors[SI_PARSER_STATS_NIT];
					else
					{
						if (nMonitorNITTableErrors != v->nSIParserTableErrors[SI_PARSER_STATS_NIT])
						{
							nMonitorNITTableErrors = v->nSIParserTableErrors[SI_PARSER_STATS_NIT];
							MonitorAlarmOn(nIndex, hDlg, 0);
						}
					}

					// Check timing (every 500 ms)
					if (nMonitorNITTimingErrors == -1)
						nMonitorNITTimingErrors = v->nSIParserTimingErrors[SI_PARSER_STATS_NIT];
					else
					{
						if (nMonitorNITTimingErrors != v->nSIParserTimingErrors[SI_PARSER_STATS_NIT])
						{
							nMonitorNITTimingErrors = v->nSIParserTimingErrors[SI_PARSER_STATS_NIT];
							MonitorAlarmOn(nIndex, hDlg, 1);
						}
					}
				}
				break;
			case MONITOR_ETR290_3_4:			// Unreferenced_PID
				{
					int i;

					for (i = 0; i < 8192; i++)
					{
						char szTemp[1024];

						if (v->fSortChartByPID == FALSE)
						{
							if (v->pc[i].lnPackets == 0)
								break;
						}
						else
						{
							if (v->pc[i].lnPackets == 0)
								continue;
						}

						if (GetPIDTooltipInfo(v->pc[i].nPID, szTemp) == FALSE)
						{
							if (v->sm[nIndex].nStatus != 2)
								MonitorAlarmOn(nIndex, hDlg, 0);
						}
					}				
				}
				break;
			case MONITOR_ETR290_3_5:		// SDT_error
				{
					// Check table errors
					if (nMonitorSDTTableErrors == -1)
						nMonitorSDTTableErrors = v->nSIParserTableErrors[SI_PARSER_STATS_SDT];
					else
					{
						if (nMonitorSDTTableErrors != v->nSIParserTableErrors[SI_PARSER_STATS_SDT])
						{
							nMonitorSDTTableErrors = v->nSIParserTableErrors[SI_PARSER_STATS_SDT];
							MonitorAlarmOn(nIndex, hDlg, 0);
						}
					}

					// Check timing (every 500 ms)
					if (nMonitorSDTTimingErrors == -1)
						nMonitorSDTTimingErrors = v->nSIParserTimingErrors[SI_PARSER_STATS_SDT];
					else
					{
						if (nMonitorSDTTimingErrors != v->nSIParserTimingErrors[SI_PARSER_STATS_SDT])
						{
							nMonitorSDTTimingErrors = v->nSIParserTimingErrors[SI_PARSER_STATS_SDT];
							MonitorAlarmOn(nIndex, hDlg, 1);
						}
					}
				}
				break;
			case MONITOR_ETR290_3_6:		// EIT_error
				{
					// Check table errors
					if (nMonitorEITTableErrors == -1)
						nMonitorEITTableErrors = v->nSIParserTableErrors[SI_PARSER_STATS_EIT];
					else
					{
						if (nMonitorEITTableErrors != v->nSIParserTableErrors[SI_PARSER_STATS_EIT])
						{
							nMonitorEITTableErrors = v->nSIParserTableErrors[SI_PARSER_STATS_EIT];
							MonitorAlarmOn(nIndex, hDlg, 0);
						}
					}

					// Check timing (every 500 ms)
					if (nMonitorEITTimingErrors == -1)
						nMonitorEITTimingErrors = v->nSIParserTimingErrors[SI_PARSER_STATS_EIT];
					else
					{
						if (nMonitorEITTimingErrors != v->nSIParserTimingErrors[SI_PARSER_STATS_EIT])
						{
							nMonitorEITTimingErrors = v->nSIParserTimingErrors[SI_PARSER_STATS_EIT];
							MonitorAlarmOn(nIndex, hDlg, 1);
						}
					}
				}
				break;
			case MONITOR_ETR290_3_7:			// RST_error
				{
					if (nMonitorRSTCounter == -1)
						nMonitorRSTCounter = v->nSIParserTableErrors[SI_PARSER_STATS_RST];
					else
					{
						if (v->nSIParserTableErrors[SI_PARSER_STATS_RST] != nMonitorRSTCounter)
						{
							nMonitorRSTCounter = v->nSIParserTableErrors[SI_PARSER_STATS_RST];
							MonitorAlarmOn(nIndex, hDlg, 0);
						}
					}				
				}
				break;
			case MONITOR_ETR290_3_8:			// TDT_error
				{
					// Check table errors
					if (nMonitorTDTTableErrors == -1)
						nMonitorTDTTableErrors = v->nSIParserTableErrors[SI_PARSER_STATS_TDT];
					else
					{
						if (nMonitorTDTTableErrors != v->nSIParserTableErrors[SI_PARSER_STATS_TDT])
						{
							nMonitorTDTTableErrors = v->nSIParserTableErrors[SI_PARSER_STATS_TDT];
							MonitorAlarmOn(nIndex, hDlg, 0);
						}
					}

					// Check timing (every 500 ms)
					if (nMonitorTDTTimingErrors == -1)
						nMonitorTDTTimingErrors = v->nSIParserTimingErrors[SI_PARSER_STATS_TDT];
					else
					{
						if (nMonitorTDTTimingErrors != v->nSIParserTimingErrors[SI_PARSER_STATS_TDT])
						{
							nMonitorTDTTimingErrors = v->nSIParserTimingErrors[SI_PARSER_STATS_TDT];
							MonitorAlarmOn(nIndex, hDlg, 1);
						}
					}
				}
				break;
			default:							// disabled because not implemented!
				if (v->sm[nIndex].nStatus != -1)
				{
					v->sm[nIndex].nStatus = -1;
					PostMessage(hDlg, WM_USER + 1, nIndex, 4);
				}
				break;
			}

			// See if any alarms are out of date
			if (v->sm[nIndex].nStatus == 2)
			{
				v->sm[nIndex].lnLastAlarmTime++;
				if (v->sm[nIndex].lnLastAlarmTime > v->nStreamMonitorAlarmTimeout)
				{
					v->sm[nIndex].nStatus = 3;	// yellow
					PostMessage(hDlg, WM_USER + 1, nIndex, v->sm[nIndex].nStatus);
				}
			}
		}

		Sleep(1000);
	}

	for (nIndex = 0; nIndex < MONITOR_COUNT; nIndex++)
	{
		v->sm[nIndex].nStatus = 0;
		PostMessage(hDlg, WM_USER + 1, nIndex, v->sm[nIndex].nStatus);
	}
	
	if (v->hStreamMonitorLog != NULL)
	{
		CloseHandle(v->hStreamMonitorLog);
		v->hStreamMonitorLog = NULL;
	}
	v->fMonitorRunning = FALSE;
	return 0;
}

void GetStreamMonitorDispInfo(LV_DISPINFO *pnmv) 
{ 
    // Provide the item or subitem's text, if requested. 
    if (pnmv->item.mask & LVIF_TEXT)
	{ 
        int nItemIndex = (int)(pnmv->item.lParam);
		switch(pnmv->item.iSubItem)
		{
		case 0:
			{
				SYSTEMTIME stLocal;
				char szDate[128];
				char szTime[128];

				SystemTimeToTzSpecificLocalTime(NULL, &v->sml[nItemIndex].st, &stLocal);
				GetDateFormat(LOCALE_SYSTEM_DEFAULT, DATE_SHORTDATE, &stLocal, NULL, szDate, sizeof(szDate));
				GetTimeFormat(LOCALE_SYSTEM_DEFAULT, 0, &stLocal, NULL, szTime, sizeof(szTime));

				wsprintf(pnmv->item.pszText, "%s %s", szDate, szTime);
			}
			break;
		case 1:
			TranslateAlarmCode(pnmv->item.pszText, v->sml[nItemIndex].nAlarmCode, v->sml[nItemIndex].nAlarmSubcode);
			break;
		}
	}
}


INT_PTR CALLBACK StreamMonitorSettingsDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg)
	{
	case WM_INITDIALOG:
		SetDlgItemInt(hDlg, IDC_STREAM_MONITOR_ALARM_TIMEOUT, v->nStreamMonitorAlarmTimeout, FALSE);
		if (v->fStreamMonitorClockSystem)
			CheckDlgButton(hDlg, IDC_STREAM_MONITOR_ALARM_CLOCK_SYSTEM, BST_CHECKED);
		else
			CheckDlgButton(hDlg, IDC_STREAM_MONITOR_ALARM_CLOCK_PCR, BST_CHECKED);
		CheckDlgButton(hDlg, IDC_STREAM_MONITOR_ALARM_EMAIL_ENABLED, v->fStreamMonitorEmailEnabled);
		SetFocus(GetDlgItem(hDlg, IDOK));
		break;
	case WM_CLOSE:
		EndDialog(hDlg, FALSE);
		break;
	case WM_COMMAND:
		switch(LOWORD(wParam))
		{
		case IDCANCEL:
			EndDialog(hDlg, FALSE);
			break;
		case IDOK:
			v->nStreamMonitorAlarmTimeout = GetDlgItemInt(hDlg, IDC_STREAM_MONITOR_ALARM_TIMEOUT, NULL, FALSE);
			v->fStreamMonitorClockSystem = IsDlgButtonChecked(hDlg, IDC_STREAM_MONITOR_ALARM_CLOCK_SYSTEM);
			v->fStreamMonitorEmailEnabled = IsDlgButtonChecked(hDlg, IDC_STREAM_MONITOR_ALARM_EMAIL_ENABLED);
			EndDialog(hDlg, TRUE);
			break;
		}
		break;
	}
	return FALSE;
}

BOOL fScrambleTest = FALSE;
BOOL fStreamMonitorFirstActivate;

INT_PTR CALLBACK StreamMonitorDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg)
	{
	case WM_INITDIALOG:
		{
			int i;
			int nColumnPosition = 0;
			HWND hWndLV = GetDlgItem(hDlg, IDC_MONITOR_LIST);
			LV_COLUMN lvc; 
			char szTemp[128];

			v->fMonitorRunning = FALSE;
			fStreamMonitorFirstActivate = FALSE;
			for (i = 0; i < MONITOR_COUNT; i++)
			{
				v->sm[i].lnLastAlarmTime = 0;
				v->sm[i].nStatus = 0;
			}
			if (v->nMonitorWindowX && v->nMonitorWindowY)
			{
				SetWindowPos(hDlg, v->hWndMainWindow,
					         v->nMonitorWindowX, v->nMonitorWindowY,
							 0, 0,
							 SWP_NOSIZE);
			}

			// Setup list view columns. 
			lvc.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM; 
			lvc.pszText = szTemp; 

			lvc.fmt = LVCFMT_LEFT; 
			lvc.cx = 130; 
			lstrcpy(szTemp, TEXT("Date/Time"));
			ListView_InsertColumn(hWndLV, nColumnPosition++, &lvc); 

			lvc.fmt = LVCFMT_LEFT; 
			lvc.cx = 365; 
			lstrcpy(szTemp, TEXT("Event"));
			ListView_InsertColumn(hWndLV, nColumnPosition++, &lvc); 

			ListView_SetExtendedListViewStyle(hWndLV, LVS_EX_FULLROWSELECT);

			v->nStreamMonitorMax = 100;
			v->nStreamMonitorCurrent = 0;
			v->sml = LocalAlloc(LPTR, sizeof(STREAMMONITORLOG) * v->nStreamMonitorMax);

			v->hStreamMonitorLog = NULL;			
			OpenStreamMonitorLogFile();

			v->hMonitorStatusIcon[0] = LoadImage(v->hInstance, MAKEINTRESOURCE(IDI_MONITOR_GREY), IMAGE_ICON, 32, 32, 0);
			v->hMonitorStatusIcon[1] = LoadImage(v->hInstance, MAKEINTRESOURCE(IDI_MONITOR_GREEN), IMAGE_ICON, 32, 32, 0);
			v->hMonitorStatusIcon[2] = LoadImage(v->hInstance, MAKEINTRESOURCE(IDI_MONITOR_RED), IMAGE_ICON, 32, 32, 0);
			v->hMonitorStatusIcon[3] = LoadImage(v->hInstance, MAKEINTRESOURCE(IDI_MONITOR_YELLOW), IMAGE_ICON, 32, 32, 0);
			v->hMonitorStatusIcon[4] = LoadImage(v->hInstance, MAKEINTRESOURCE(IDI_MONITOR_DISABLED), IMAGE_ICON, 32, 32, 0);

#ifndef _DEBUG
			EnableWindow(GetDlgItem(hDlg, IDC_DEBUG), FALSE);
			ShowWindow(GetDlgItem(hDlg, IDC_DEBUG), SW_HIDE);
#endif _DEBUG
			SetFocus(GetDlgItem(hDlg, IDOK));
		}
		break;
	case WM_ACTIVATE:
		if (fStreamMonitorFirstActivate == FALSE)
		{
			fStreamMonitorFirstActivate = TRUE;
			PostMessage(hDlg, WM_COMMAND, IDOK, 0);
		}
		break;
	case WM_CLOSE:
		if (v->fMonitorRunning)
			SendMessage(hDlg, WM_COMMAND, IDOK, 0);
		DestroyWindow(hDlg);
		break;
	case WM_DESTROY:
		if (!v->fMonitorMinimizedFlag)
		{
			RECT rcMonitorWindow;

			GetWindowRect(hDlg, &rcMonitorWindow);
			v->nMonitorWindowX = rcMonitorWindow.left;
			v->nMonitorWindowY = rcMonitorWindow.top;
		}
		DestroyIcon(v->hMonitorStatusIcon[0]);
		DestroyIcon(v->hMonitorStatusIcon[1]);
		DestroyIcon(v->hMonitorStatusIcon[2]);
		DestroyIcon(v->hMonitorStatusIcon[3]);
		DestroyIcon(v->hMonitorStatusIcon[4]);
		v->hDlgStreamMonitor = NULL;
		break;
	case WM_COMMAND:
		switch(LOWORD(wParam))
		{			
		case ID_IDRSTREAMMONITORPOPUP_MASKTHISITEM:
			v->sm[v->nStreamMonitorClickIndex].fDisabled = ~v->sm[v->nStreamMonitorClickIndex].fDisabled & 1;
			if (v->sm[v->nStreamMonitorClickIndex].fDisabled)
				v->sm[v->nStreamMonitorClickIndex].nStatus = 0;
			else
				v->sm[v->nStreamMonitorClickIndex].nStatus = 1;
			PostMessage(hDlg, WM_USER + 1, v->nStreamMonitorClickIndex, v->sm[v->nStreamMonitorClickIndex].nStatus);
			break;
		case ID_IDRSTREAMMONITORPOPUP_RESETALARM:
			if (!v->sm[v->nStreamMonitorClickIndex].fDisabled)
				v->sm[v->nStreamMonitorClickIndex].nStatus = 1;
			PostMessage(hDlg, WM_USER + 1, v->nStreamMonitorClickIndex, v->sm[v->nStreamMonitorClickIndex].nStatus);
			break;
		case IDC_MONITOR_1_1:
		case IDC_MONITOR_1_2:
		case IDC_MONITOR_1_3:
		case IDC_MONITOR_1_4:
		case IDC_MONITOR_1_5:
		case IDC_MONITOR_1_6:
		case IDC_MONITOR_2_1:
		case IDC_MONITOR_2_2:
		case IDC_MONITOR_2_3:
		case IDC_MONITOR_2_4:
		case IDC_MONITOR_2_5:
		case IDC_MONITOR_2_6:
		case IDC_MONITOR_3_1:
		case IDC_MONITOR_3_2:
		case IDC_MONITOR_3_3:
		case IDC_MONITOR_3_4:
		case IDC_MONITOR_3_5:
		case IDC_MONITOR_3_6:
		case IDC_MONITOR_3_7:
		case IDC_MONITOR_3_8:
		case IDC_MONITOR_3_9:
		case IDC_MONITOR_3_10:
			{
				int nIndex = LOWORD(wParam) - IDC_MONITOR_1_1;
				HMENU hMenu;            // menu template         
				HMENU hMenuTrackPopup;  // floating pop-up menu
				RECT rc;
				RECT rcClient;
				POINT ptMouse;

				v->nStreamMonitorClickIndex = nIndex;
				if (v->sm[v->nStreamMonitorClickIndex].nStatus == -1)
					break;	// not implemented so no popup

				hMenu = LoadMenu(v->hInstance, MAKEINTRESOURCE(IDR_STREAM_MONITOR_POPUP)); 
				if (hMenu == NULL) 
					break; 

				GetWindowRect(GetDlgItem(hDlg, LOWORD(wParam)), &rc);
				GetClientRect(GetDlgItem(hDlg, LOWORD(wParam)), &rcClient);

				ptMouse.x = rc.left + (rcClient.right / 2);
				ptMouse.y = rc.top + (rcClient.bottom / 2);
				hMenuTrackPopup = GetSubMenu(hMenu, 0);
				if (v->sm[nIndex].fDisabled)
					CheckMenuItem(hMenuTrackPopup, ID_IDRSTREAMMONITORPOPUP_MASKTHISITEM, MF_CHECKED | MF_BYCOMMAND);
				if (v->sm[nIndex].nStatus != 2 && v->sm[nIndex].nStatus != 3)
					EnableMenuItem(hMenuTrackPopup, ID_IDRSTREAMMONITORPOPUP_RESETALARM, MF_GRAYED | MF_BYCOMMAND);
				TrackPopupMenu(hMenuTrackPopup, TPM_LEFTALIGN | TPM_LEFTBUTTON, ptMouse.x, ptMouse.y, 0, hDlg, NULL); 
				DestroyMenu(hMenu);
			}
			break;
		case IDCANCEL:
			PostMessage(hDlg, WM_CLOSE, 0, 0);
			break;
		case IDOK:
			if (v->fMonitorRunning == FALSE)
			{
				HANDLE hThread;
				DWORD dwThreadID;

				v->fMonitorTerminateThread = FALSE;
				nMonitorScrambledPATs = 0;
				nMonitorScrambledPMTs = 0;
				nMonitorPCRDelayError = 0;

				hThread = CreateThread(NULL, 0, StreamMonitorThread, (LPVOID)hDlg, 0, &dwThreadID);
				CloseHandle(hThread);
				v->fMonitorRunning = TRUE;
			}
			else
			{
				v->fMonitorTerminateThread = TRUE;
				while (v->fMonitorRunning)
					Sleep(50);
			}
			break;
		case IDC_DEBUG:
			fScrambleTest = TRUE;
			break;
		case IDC_STREAM_MONITOR_SETTINGS:
			DialogBox(v->hInstance, MAKEINTRESOURCE(IDD_STREAM_MONITOR_SETTINGS), hDlg, StreamMonitorSettingsDlgProc);
			break;
			/*
		case IDC_STREAM_MONITOR_SAVE:
			{
				OPENFILENAME ofn;
				char szOutputFile[MAX_PATH] = {0};

				memset( &(ofn), 0, sizeof(ofn));
				ofn.lStructSize	= sizeof(ofn);
				ofn.hwndOwner = hDlg;
				ofn.lpstrFile = szOutputFile;
				ofn.nMaxFile = MAX_PATH;
				ofn.lpstrFilter = TEXT("Text Files (*.txt)\0*.txt\0All Files (*.*)\0*.*\0\0");	
				ofn.lpstrTitle = TEXT("Save Stream Monitor Log");
				ofn.lpstrDefExt = TEXT("txt");
				ofn.lpstrInitialDir = v->szSaveStreamLogFolder;
				ofn.Flags =  OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT | OFN_EXPLORER;
				ofn.hInstance = v->hInstance;						
				if (myGetSaveFileName(&ofn) == TRUE)
				{
					HANDLE hOutputFile = CreateFile(szOutputFile, GENERIC_WRITE, 0, (LPSECURITY_ATTRIBUTES) NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, (HANDLE) NULL);
					
					if (hOutputFile != INVALID_HANDLE_VALUE)
					{
						CloseHandle(hOutputFile);


					}
					else
						MessageBox(hDlg, "Unable to save Stream Monitor log file", gszAppName, MB_ICONSTOP);		
				}
			}
			break;
			*/
		}
		break;
	case WM_SIZE:
		v->fMonitorMinimizedFlag = wParam == SIZE_MINIMIZED;
		break;
	case WM_NOTIFY:
		switch (((LPNMHDR) lParam)->code)
		{ 
		case LVN_GETDISPINFO:
			{
				NMLVDISPINFO * pnmv = (NMLVDISPINFO *)lParam;
				GetStreamMonitorDispInfo((LV_DISPINFO *) lParam);
			}
			break;
		}
		break;
	case WM_USER + 1:
		{
			int nResource = IDC_MONITOR_1_1 + (int)wParam;
			SendDlgItemMessage(hDlg, nResource, STM_SETIMAGE, IMAGE_ICON, (LPARAM)v->hMonitorStatusIcon[lParam]);
			if (lParam == 4)
			{
				// Switching off because unavailable
				EnableWindow(GetDlgItem(hDlg, IDC_MONITOR_1_1_CAPTION + (int)wParam), FALSE);
			}
		}
		break;
	}
	
	return FALSE;
}

void MonitorProgramData(BYTE * pBuffer, int nLength)
{
	int nBufferOffset = 0;
	int i;
	double dPCR[8192];

	for (i = 0; i < 8192; i++)
		dPCR[i] = -1.0;

	while (nLength > 0)
	{
		int nPID;
		
		nPID = (pBuffer[nBufferOffset + 1] << 8 | pBuffer[nBufferOffset + 2]) & 0x1fff;
		if (fScrambleTest && nPID == 0x0047)
		{
			fScrambleTest = FALSE;
			pBuffer[nBufferOffset + 3] |= 0xc0;
		}

		// Check for scrambled PAT/PMTs
		if (nPID == 0x0000)
		{
			if ((pBuffer[nBufferOffset + 3] & 0xc0) != 0)
				nMonitorScrambledPATs++;
		}
		else
		{
			int nPMTIndex;

			for (nPMTIndex = 0; nPMTIndex < MAX_PAT_ENTRIES; nPMTIndex++)
			{
				if (v->pat.pmt[nPMTIndex].nPMTPID == 0)
					break;
				if (v->pat.pmt[nPMTIndex].nPMTPID == nPID)
				{
					if ((pBuffer[nBufferOffset + 3] & 0xc0) != 0)
						nMonitorScrambledPMTs++;
				}
			}
		}

		// Check PCR delay
		if (nPID < 0x1fff)
		{
			if   ( ((pBuffer[nBufferOffset + 3] & 0x20) == 0x20)	// Adaptation field?
				&& (pBuffer[nBufferOffset + 4] > 0)					// Adaptation len
				&& ((pBuffer[nBufferOffset + 5] & 0x10) == 0x10)	// PCR flag
				&& ((pBuffer[nBufferOffset + 1] & 0x80) == 0x00) )	// no TEI error
			{
				// We can now start monitoring for the bitrate
				double dCurrentPCR = (double)DecodeMPEG2PCR(&pBuffer[nBufferOffset + 4]);
				
				if (dPCR[nPID] == -1.0)
					dPCR[nPID] = dCurrentPCR;
				else
				{
					double dDelay = (1.0/27000000.0 * (dCurrentPCR - dPCR[nPID]));

					if (dDelay > 0.040)
						nMonitorPCRDelayError++;
					dPCR[nPID] = dCurrentPCR;
				}
			}
		}

		nLength -= 188;
		nBufferOffset += 188;
	}
}
