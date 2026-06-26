#include <windows.h>
#include <commctrl.h>
#include "TSReader.h"
#include "Pegrpapi.h"
#include "bcdmux.h"
#include "util.h"

#ifdef PRO_BROKEN
 #include "fvd_mp4.h"
 FVD_VIDEO_DEC * H264Dec = 0;
#endif PRO_BROKEN

// Stuff in TSReader.c
extern PVARIABLES v;
extern BOOL (* GetTunerString) (char * szString);
extern char gszAppName[];

int __cdecl SortPIDsByPackets(const void *elem1, const void *elem2);
int __cdecl SortPIDsByPID(const void *elem1, const void *elem2);

// Stuff in CCDecoder (why there?)
void bs_init(bs_t* b, uint8_t* buf, int size);
uint32_t bs_read_ue(bs_t* b);

LARGE_INTEGER rate, priorcount;
double dMaxRate;
// Stuff in here
BOOL CheckCommonGraphMessages(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg)
	{
	case WM_CREATE:
		{
			LPCREATESTRUCT lpcs = (LPCREATESTRUCT)lParam;
			int nChartIndex = *((int*)lpcs->lpCreateParams);

			SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)nChartIndex);
			if (v->fSaveChartDataEnabled)
			{
				SYSTEMTIME st;
				char szTemp[128];
				char szSaveDataFile[MAX_PATH];

				lstrcpy(szSaveDataFile, v->szSaveChartDataFolder);
				if (szSaveDataFile[lstrlen(szSaveDataFile) - 1] != '\\')
					lstrcat(szSaveDataFile, "\\");
				
				GetSystemTime(&st);
				wsprintf(szTemp, "ChartData-%04d%02d%02d-%02d%02d%02d.txt",
					     st.wYear, st.wMonth, st.wDay,
						 st.wHour, st.wMinute, st.wSecond);
				lstrcat(szSaveDataFile, szTemp);
				v->hSaveDataFile[nChartIndex] = CreateFile(szSaveDataFile, GENERIC_WRITE, FILE_SHARE_READ, (LPSECURITY_ATTRIBUTES) NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL,	(HANDLE) NULL);
			}
		}
		break;
	case WM_SIZE:
		{
			int nChartIndex = (int)GetWindowLongPtr(hWnd, GWLP_USERDATA);
			v->fChartMinimizedFlag = wParam == SIZE_MINIMIZED;
			v->fChartMaximizedFlag = wParam == SIZE_MAXIMIZED;		
			if (v->m_hPE[nChartIndex])
			{
				RECT rc;
				GetClientRect(hWnd, &rc);
				MoveWindow(v->m_hPE[nChartIndex], 0, 0, rc.right, rc.bottom, FALSE);
			}
		}
		break;
	case WM_CLOSE:
		DestroyWindow(hWnd);
		break;
	case WM_DESTROY:
		{
			int nChartIndex = (int)GetWindowLongPtr(hWnd, GWLP_USERDATA);

			if (!v->fChartMinimizedFlag && !v->fChartMaximizedFlag)
			{
				RECT rcChartWindow;

				GetWindowRect(hWnd, &rcChartWindow);
				v->nChartWindowX = rcChartWindow.left;
				v->nChartWindowY = rcChartWindow.top;
				v->nChartWindowW = rcChartWindow.right - rcChartWindow.left;
				v->nChartWindowH = rcChartWindow.bottom - rcChartWindow.top;
			}
			if (v->m_hPE[nChartIndex])
			{
				PEdestroy(v->m_hPE[nChartIndex]);
				v->m_hPE[nChartIndex] = NULL;
			}
			KillTimer(hWnd, 100 + nChartIndex);
			SetForegroundWindow(v->hWndMainWindow);
			if (v->fSaveChartDataEnabled)
				CloseHandle(v->hSaveDataFile[nChartIndex]);
			PostMessage(v->hWndMainWindow, WM_USER + 9, 0, nChartIndex);
		}
		break;
	case WM_KEYDOWN:
		switch(wParam)
		{
		case VK_ESCAPE:
			PostMessage(hWnd, WM_CLOSE, 0, 0);
			break;
		}
		break;
	case WM_SETFOCUS:
		InvalidateThumbnails();
		break;
	default:
		return FALSE;
	}

	return TRUE;
}

void SaveChartData(int nChartIndex, char * szSaveData)
{
	DWORD dwWritten;
	SYSTEMTIME st;
	char szTemp[2048];

	GetSystemTime(&st);
	wsprintf(szTemp, "%04d%02d%02d,%02d%02d%02d,%s\r\n",
			 st.wYear, st.wMonth, st.wDay,
			 st.wHour, st.wMinute, st.wSecond,
			 szSaveData);
	WriteFile(v->hSaveDataFile[nChartIndex], szTemp, lstrlen(szTemp), &dwWritten, NULL);
}

void Charting__UpdateQuickStyle(int nChartIndex)
{
	PEnset(v->m_hPE[nChartIndex], PEP_bBITMAPGRADIENTMODE, v->fChartGradientBitmap);
	PEnset(v->m_hPE[nChartIndex], PEP_nQUICKSTYLE, v->nChartStyle);
}

void SetupPIEGraphData(int nChartIndex)
{
	int nActivePIDCounter = 0;
	int i;
	int nChartPoints = 0;
	PIDCOUNTER pc[8192];
	char szSaveData[1024];

	EnterCriticalSection(&v->ss.csPIDCounter);
	memcpy(&pc, &v->pc, sizeof(PIDCOUNTER) * 8192);
	LeaveCriticalSection(&v->ss.csPIDCounter);//zz

	for (i = 0; i < 8192; i++)
	{
		if (pc[nActivePIDCounter].lnPackets)
			nChartPoints++;
		nActivePIDCounter++;
	}
	qsort(&pc, nActivePIDCounter, sizeof(PIDCOUNTER), SortPIDsByPackets);
	if (nChartPoints > 32)
		nChartPoints = 32;
	if (!v->fSetPoints[nChartIndex])
	{
		v->fSetPoints[nChartIndex] = TRUE;
		PEnset(v->m_hPE[nChartIndex], PEP_nPOINTS, nChartPoints);
	}
	for (i = 0; i < nChartPoints; i++)
	{
		char szPointLabel[128];
		float f = (float)pc[i].lnPackets;
		PEvsetcellEx(v->m_hPE[nChartIndex], PEP_faXDATA, 0, i, &f);
		GetPIDTooltipInfo(pc[i].nPID, szPointLabel, sizeof(szPointLabel));
		PEvsetcell(v->m_hPE[nChartIndex], PEP_szaPOINTLABELS, i, szPointLabel);
		if (v->fSaveChartDataEnabled)
		{
			sprintf(szSaveData, "PID=%04x,Packets=%d", pc[i].nPID, (int)f);
			SaveChartData(nChartIndex, szSaveData);
		}
	}
}

void SetupChartSubtitle(int nChartIndex)
{
	char szTunerString[256] = {0};

	if (GetTunerString != NULL)
		GetTunerString(szTunerString);
	if (lstrcmp(szTunerString, "n/a") != 0)
		PEszset(v->m_hPE[nChartIndex], PEP_szSUBTITLE, szTunerString);
	else
		PEszset(v->m_hPE[nChartIndex], PEP_szSUBTITLE, "");
}

LRESULT FAR PASCAL PIDPieChartWindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg)
	{
	case WM_CREATE:
		{
			RECT rect;
			DWORD c[16];
			LPCREATESTRUCT lpcs = (LPCREATESTRUCT)lParam;
			int nChartIndex = *((int*)lpcs->lpCreateParams);

			SendMessage(hWnd, WM_SETICON, ICON_BIG, (LPARAM)v->hDialogIcon);

			v->fSetPoints[nChartIndex] = FALSE;
			GetClientRect(hWnd, &rect);
			v->m_hPE[nChartIndex] = PEcreate(PECONTROL_PIE, WS_VISIBLE, &rect, hWnd, 1001);
			PEnset(v->m_hPE[nChartIndex], PEP_nSUBSETS, 1);			
			SetupPIEGraphData(nChartIndex);

			// Set Slice Colors
			c[0] = RGB(198, 0, 0);
			c[1] = RGB(0, 198, 0);
			c[2] = RGB(198, 198, 0);
			c[3] = RGB(0, 0, 198);
			c[4] = RGB(198, 0, 198);
			c[5] = RGB(0, 198, 198);
			c[6] = RGB(192, 192, 192);
			c[7] = RGB(148, 0, 0);
			c[8] = RGB(0, 148, 0);
			c[9] = RGB(148, 148, 0);
			c[10] = RGB(0, 148, 148);
			c[11] = RGB(122, 0, 0);
			c[12] = RGB(0, 122, 0);
			c[13] = RGB(122, 122, 0);
			c[14] = RGB(0, 0, 122);
			c[15] = RGB(122, 0, 122);
			PEvset(v->m_hPE[nChartIndex], PEP_dwaSUBSETCOLORS, c, 16);

			// Set Main Title
			PEszset(v->m_hPE[nChartIndex], PEP_szMAINTITLE, "PID Usage");
			SetupChartSubtitle(nChartIndex);

			// Set various other properties
			PEnset(v->m_hPE[nChartIndex], PEP_nDATAPRECISION, 1);				   
			PEnset(v->m_hPE[nChartIndex], PEP_nGROUPINGPERCENT, 1);
			if (v->nChartParameters == TRUE)
			{
				SendMessage(hWnd, WM_SETTEXT, 0, (LPARAM)"PID Usage 3D Pie chart");
				PEnset(v->m_hPE[nChartIndex], PEP_nDATASHADOWS, PEDS_3D);					 
			}
			else
				SendMessage(hWnd, WM_SETTEXT, 0, (LPARAM)"PID Usage 2D Pie chart");
			PEnset(v->m_hPE[nChartIndex], PEP_bDISABLE3DSHADOW, FALSE);
			PEnset(v->m_hPE[nChartIndex], PEP_nAUTOEXPLODE, PEAE_ALLSUBSETS);
			PEnset(v->m_hPE[nChartIndex], PEP_bFOCALRECT, FALSE);
			PEnset(v->m_hPE[nChartIndex], PEP_bPREPAREIMAGES, TRUE);
			PEnset(v->m_hPE[nChartIndex], PEP_bCACHEBMP, TRUE);
			PEnset(v->m_hPE[nChartIndex], PEP_nFONTSIZE, PEFS_LARGE);
			PEnset(v->m_hPE[nChartIndex], PEP_bFIXEDFONTS, TRUE);
			Charting__UpdateQuickStyle(nChartIndex);
			PEnset(v->m_hPE[nChartIndex], PEP_dwTEXTCOLOR, 0);
			PEnset(v->m_hPE[nChartIndex], PEP_bMAINTITLEBOLD, TRUE);
			PEnset(v->m_hPE[nChartIndex], PEP_bSUBTITLEBOLD, TRUE);
			PEnset(v->m_hPE[nChartIndex], PEP_bLABELBOLD, TRUE);
			PEnset(v->m_hPE[nChartIndex], PEP_nFONTSIZE, PEFS_LARGE);
			SetTimer(hWnd, 100 + nChartIndex, v->nGraphRefreshRate, NULL);
			CheckCommonGraphMessages(hWnd, uMsg, wParam, lParam);
		}	
		break;
	case WM_TIMER:
		{
			int nChartIndex = (int)GetWindowLongPtr(hWnd, GWLP_USERDATA);
			SetupPIEGraphData(nChartIndex);
			InvalidateRect(hWnd, NULL, FALSE);
		}
		break;
	default:
		if (CheckCommonGraphMessages(hWnd, uMsg, wParam, lParam) == TRUE)
			break;
		return DefWindowProc(hWnd, uMsg, wParam, lParam);
	}

	return FALSE;
}

void AddVideoRateData(int nChartIndex)
{
	int nPMTIndex, nESIndex;
	int nPointIndex = 0;
	float * fNewValues = LocalAlloc(LPTR, sizeof(float) * GetVideoStreamCount());
	SYSTEMTIME stLocal;
	char szTemp[128];
	char szSaveData[1024];

	for (nPMTIndex = 0; nPMTIndex < MAX_PAT_ENTRIES; nPMTIndex++)
	{
		if (v->pat.pmt[nPMTIndex].nPMTPID == 0)
			break;
		for (nESIndex = 0; nESIndex < MAX_ESLIST_ENTRIES; nESIndex++)
		{
			BOOL fAddThisOne = FALSE;

			if (v->pat.pmt[nPMTIndex].es[nESIndex].nESPID == 0)
				break;
			switch(v->pat.pmt[nPMTIndex].es[nESIndex].nStreamType)
			{
			case 0x01:	// mpeg-1 video
			case 0x02:	// mpeg-2 video
			case 0x10:	// MPEG-4 video
			case 0x1b:	// H.264 video
				fAddThisOne = TRUE;
				break;
			case 0x80:	// dc-ii video
				if (v->nNetworkPID != 0x0010)
					fAddThisOne = TRUE;
				break;
			}
			if (fAddThisOne)
			{
				int nESPID = v->pat.pmt[nPMTIndex].es[nESIndex].nESPID;
				double dPIDRate = 0.0;

				if (v->lnPIDRateSamples[nESPID])
					dPIDRate = (v->dPIDRate[nESPID] / (double)v->lnPIDRateSamples[nESPID]) * 8.0;
				else
				{
					double dPercent = ((double)v->lnPIDCounter[nESPID] / (double)v->lnTotalTSPackets) * 100.0;
					dPIDRate = (v->dDisplayMuxRate / (double)v->nMuxRateCounter) * 8.0;
					dPIDRate  = ((dPIDRate / 100.0) * dPercent);
				}
				fNewValues[nPointIndex++] = (float)dPIDRate;
				if (v->fSaveChartDataEnabled)
				{
					sprintf(szSaveData, "Program=%d,Rate=%.3f",
							 v->pat.pmt[nPMTIndex].nProgramNumber, dPIDRate);
					SaveChartData(nChartIndex, szSaveData);
				}
				break;
			}
		}
	}

	PEvset(v->m_hPE[nChartIndex], PEP_faAPPENDYDATA, fNewValues, 1);

	GetLocalTime(&stLocal);
	wsprintf(szTemp, "%02d:%02d", stLocal.wHour, stLocal.wMinute);
	PEvset(v->m_hPE[nChartIndex], PEP_szaAPPENDPOINTLABELDATA, szTemp, 1);

	PEreinitialize(v->m_hPE[nChartIndex]);
	PEresetimage(v->m_hPE[nChartIndex], 0, 0);
	LocalFree(fNewValues);
}

LRESULT FAR PASCAL VideoBitrateChartWindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg)
	{
	case WM_CREATE:
		{
			LPCREATESTRUCT lpcs = (LPCREATESTRUCT)lParam;
			int nChartIndex = *((int*)lpcs->lpCreateParams);
			int nVideoStreamCount = GetVideoStreamCount();
			DWORD col = RGB(0, 198, 0);
			int nPMTIndex, nESIndex;
			double dMaxBitrate = 0.0;
			double arg;
			RECT rect;
			char szPointLabels[10 * 1024] = {0};

			SendMessage(hWnd, WM_SETICON, ICON_BIG, (LPARAM)v->hDialogIcon);
			v->fSetPoints[nChartIndex] = FALSE;

			GetClientRect(hWnd, &rect);

			v->m_hPE[nChartIndex] = PEcreate(PECONTROL_GRAPH, WS_VISIBLE, &rect, hWnd, 1001);
			PEnset(v->m_hPE[nChartIndex], PEP_nSUBSETS, nVideoStreamCount);
			PEnset(v->m_hPE[nChartIndex], PEP_nPOINTS, v->nGraphHistoricalPoints);

			if (v->nChartParameters == TRUE)
			{
				PEnset(v->m_hPE[nChartIndex], PEP_nPLOTTINGMETHOD, PEGPM_AREA);
				PEnset(v->m_hPE[nChartIndex], PEP_bNOSTACKEDDATA, FALSE);
				PEnset(v->m_hPE[nChartIndex], PEP_nDATASHADOWS, PEDS_NONE);					 
				PEszset(v->m_hPE[nChartIndex], PEP_szMAINTITLE, "Video Bitrate Area Chart");
			}
			else
				PEszset(v->m_hPE[nChartIndex], PEP_szMAINTITLE, "Video Bitrate Line Chart");

			SetupChartSubtitle(nChartIndex);
			PEnset(v->m_hPE[nChartIndex], PEP_bNORANDOMPOINTSTOEXPORT, TRUE);
			PEnset(v->m_hPE[nChartIndex], PEP_bFOCALRECT, FALSE);
			PEnset(v->m_hPE[nChartIndex], PEP_bALLOWBAR, FALSE);
			PEnset(v->m_hPE[nChartIndex], PEP_bALLOWPOPUP, FALSE);
			PEnset(v->m_hPE[nChartIndex], PEP_bPREPAREIMAGES, TRUE);
			PEnset(v->m_hPE[nChartIndex], PEP_bCACHEBMP, TRUE);
			PEnset(v->m_hPE[nChartIndex], PEP_bFIXEDFONTS, TRUE);
				
			for (nPMTIndex = 0; nPMTIndex < MAX_PAT_ENTRIES; nPMTIndex++)
			{
				if (v->pat.pmt[nPMTIndex].nPMTPID == 0)
					break;
				for (nESIndex = 0; nESIndex < MAX_ESLIST_ENTRIES; nESIndex++)
				{
					BOOL fAddThisOne = FALSE;

					if (v->pat.pmt[nPMTIndex].es[nESIndex].nESPID == 0)
						break;
					switch(v->pat.pmt[nPMTIndex].es[nESIndex].nStreamType)
					{
					case 0x01:	// mpeg-1 video
					case 0x02:	// mpeg-2 video
					case 0x10:	// MPEG-4 video
					case 0x1b:	// H.264 video
						fAddThisOne = TRUE;
						break;
					case 0x80:	// dc-ii video
						if (v->nNetworkPID != 0x0010)
							fAddThisOne = TRUE;
						break;
					}
					if (fAddThisOne)
					{
						int nESPID = v->pat.pmt[nPMTIndex].es[nESIndex].nESPID;
						double dPIDRate;
						char szTemp[256];
						char szChannelName[256] = {0};

						if (v->pChannelData[v->pat.pmt[nPMTIndex].nProgramNumber] != NULL)
						{
							lstrcpy(szChannelName, " ");
							lstrcat(szChannelName, v->pChannelData[v->pat.pmt[nPMTIndex].nProgramNumber]->szShortName);
						}
						wsprintf(szTemp, "Program %d%s\t", v->pat.pmt[nPMTIndex].nProgramNumber, szChannelName);
						lstrcat(szPointLabels, szTemp);
						
						if (v->lnPIDRateSamples[nESPID])
							dPIDRate = (v->dPIDRate[nESPID] / (double)v->lnPIDRateSamples[nESPID]) * 8.0;
						else
						{
							double dPercent = ((double)v->lnPIDCounter[nESPID] / (double)v->lnTotalTSPackets) * 100.0;
							dPIDRate = (v->dDisplayMuxRate / (double)v->nMuxRateCounter) * 8.0;
							dPIDRate  = ((dPIDRate / 100.0) * dPercent);
						}
						if (dPIDRate > dMaxBitrate)
							dMaxBitrate = dPIDRate;
						break;
					}
				}
			}			
			PEvset(v->m_hPE[nChartIndex], PEP_szaSUBSETLABELS , szPointLabels, nVideoStreamCount);
			
			// Set Manual Y scale //
			//PEnset(v->m_hPE[nChartIndex], PEP_nMANUALSCALECONTROLY, PEMSC_MINMAX);

			arg = 1.0F;
			PEvset(v->m_hPE[nChartIndex], PEP_fMANUALMINY, &arg, 1);
			dMaxBitrate += dMaxBitrate / 10.0;
			PEvset(v->m_hPE[nChartIndex], PEP_fMANUALMAXY, &dMaxBitrate, 1);

			PEvsetcell(v->m_hPE[nChartIndex], PEP_dwaSUBSETCOLORS, 0, &col);
			PEnset(v->m_hPE[nChartIndex], PEP_nGRADIENTBARS, 8);
			PEnset(v->m_hPE[nChartIndex], PEP_bMAINTITLEBOLD, TRUE);
			PEnset(v->m_hPE[nChartIndex], PEP_bSUBTITLEBOLD, TRUE);
			PEnset(v->m_hPE[nChartIndex], PEP_bLABELBOLD, TRUE);
			PEnset(v->m_hPE[nChartIndex], PEP_bLINESHADOWS, TRUE);
			PEnset(v->m_hPE[nChartIndex], PEP_nFONTSIZE, PEFS_LARGE);
			Charting__UpdateQuickStyle(nChartIndex);

			PEszset(v->m_hPE[nChartIndex], PEP_szaPOINTLABELS, "");
			PEszset(v->m_hPE[nChartIndex], PEP_szXAXISLABEL, "Time");
			PEszset(v->m_hPE[nChartIndex], PEP_szYAXISLABEL, "Bitrate");

			PEreinitialize(v->m_hPE[nChartIndex]);
			PEresetimage(v->m_hPE[nChartIndex], 0, 0);
			AddVideoRateData(nChartIndex);
			AddVideoRateData(nChartIndex);
			AddVideoRateData(nChartIndex);
			AddVideoRateData(nChartIndex);

			SendMessage(hWnd, WM_SETTEXT, 0, (LPARAM)"Video Bitrate Chart");
			SetTimer(hWnd, 100 + nChartIndex, v->nGraphRefreshRate, NULL);
			CheckCommonGraphMessages(hWnd, uMsg, wParam, lParam);
		}
		break;
	case WM_TIMER:
		{
			int nChartIndex = (int)GetWindowLongPtr(hWnd, GWLP_USERDATA);
			AddVideoRateData(nChartIndex);
			InvalidateRect(hWnd, NULL, FALSE);
		}
		break;
	default:
		if (CheckCommonGraphMessages(hWnd, uMsg, wParam, lParam) == TRUE)
			break;
		return DefWindowProc(hWnd, uMsg, wParam, lParam);
	}

	return FALSE;
}

void AddMuxRateData(int nChartIndex)
{
	int nPMTIndex, nESIndex;
	int nPointIndex = 0;
	int i;
	__int64 lnTotalTSPackets;
	float fNewValues[8192];
	float fPercent;
	SYSTEMTIME stLocal;
	PIDCOUNTER pc[8192];
	char szTemp[128];
	char szSaveData[1024];

	// Copy over the PID stats
	memset(&pc, 0, sizeof(PIDCOUNTER) * 8192);
	EnterCriticalSection(&v->ss.csPIDCounter);
	for (i = 0; i < 8192; i++)
	{
		if (v->pc[i].lnPackets)
			memcpy(&pc[v->pc[i].nPID], &v->pc[i], sizeof(PIDCOUNTER));
	}
	lnTotalTSPackets = v->lnCopyTotalTSPackets;
	LeaveCriticalSection(&v->ss.csPIDCounter);
		
	// Sort the streams into program and then count the ES usage
	for (nPMTIndex = 0; nPMTIndex < MAX_PAT_ENTRIES; nPMTIndex++)
	{
		fPercent = 0.00000001f;

		if (v->pat.pmt[nPMTIndex].nPMTPID == 0)
			break;
		if (v->pat.pmt[nPMTIndex].nProgramNumber == 0)
			continue;
		for (nESIndex = 0; nESIndex < MAX_ESLIST_ENTRIES; nESIndex++)
		{
			int nESPID;
			
			if (v->pat.pmt[nPMTIndex].es[nESIndex].nESPID == 0)
				break;
			nESPID = v->pat.pmt[nPMTIndex].es[nESIndex].nESPID;
			if (pc[nESPID].lnPackets != 0)
			{
				fPercent += ((float)pc[nESPID].lnPackets / (float)lnTotalTSPackets) * 100.0f;
				pc[nESPID].lnPackets = 0;
			}
		}
		fNewValues[nPointIndex++] = fPercent;
		if (v->fSaveChartDataEnabled)
		{
			sprintf(szSaveData, "Program=%d,Percentage=%.3f",
					 v->pat.pmt[nPMTIndex].nProgramNumber, fPercent);
			SaveChartData(nChartIndex, szSaveData);
		}
	}

	// Now look for other usage - anything left over other than nulls
	fPercent = 0.00000001f;
	for (i = 0; i < 8191; i++)
	{
		if (pc[i].lnPackets != 0)
		{
			fPercent += ((float)pc[i].lnPackets / (float)lnTotalTSPackets) * 100.0f;
			pc[i].lnPackets = 0;
		}
	}
	fNewValues[nPointIndex++] = fPercent;
	if (v->fSaveChartDataEnabled)
	{
		sprintf(szSaveData, "Other,Percentage=%.3f",
				 (double)fPercent);
		SaveChartData(nChartIndex, szSaveData);
	}

	// Now NULL packets
	fPercent = 0.00000001f;
	if (pc[0x1fff].lnPackets)
		fPercent = ((float)pc[0x1fff].lnPackets / (float)lnTotalTSPackets) * 100.0f;
	fNewValues[nPointIndex++] = fPercent;
	if (v->fSaveChartDataEnabled)
	{
		sprintf(szSaveData, "Null,Percentage=%.3f",
				 fPercent);
		SaveChartData(nChartIndex, szSaveData);
	}
	PEvset(v->m_hPE[nChartIndex], PEP_faAPPENDYDATA, fNewValues, 1);

	GetLocalTime(&stLocal);
	wsprintf(szTemp, "%02d:%02d", stLocal.wHour, stLocal.wMinute);
	PEvset(v->m_hPE[nChartIndex], PEP_szaAPPENDPOINTLABELDATA, szTemp, 1);

	PEreinitialize(v->m_hPE[nChartIndex]);
	PEresetimage(v->m_hPE[nChartIndex], 0, 0);
}

LRESULT FAR PASCAL MuxrateChartWindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg)
	{
	case WM_CREATE:
		{
			LPCREATESTRUCT lpcs = (LPCREATESTRUCT)lParam;
			int nChartIndex = *((int*)lpcs->lpCreateParams);
			int nPMTIndex;
			int nProgramCount = GetProgramCount();
			RECT rect;
			char szPointLabels[10 * 1024] = {0};

			SendMessage(hWnd, WM_SETICON, ICON_BIG, (LPARAM)v->hDialogIcon);
			v->fSetPoints[nChartIndex] = FALSE;

			GetClientRect(hWnd, &rect);
			v->m_hPE[nChartIndex] = PEcreate(PECONTROL_GRAPH, WS_VISIBLE, &rect, hWnd, 1001);

			if (v->nChartParameters == TRUE)
			{
				PEnset(v->m_hPE[nChartIndex], PEP_bNOSTACKEDDATA, FALSE);
				PEnset(v->m_hPE[nChartIndex], PEP_nPLOTTINGMETHOD, PEGPM_AREASTACKED);
				PEnset(v->m_hPE[nChartIndex], PEP_nDATASHADOWS, PEDS_NONE);	
				PEnset(v->m_hPE[nChartIndex], PEP_bGRIDINFRONT, TRUE);
				PEszset(v->m_hPE[nChartIndex], PEP_szMAINTITLE, "Mux Usage Area Chart");
				SendMessage(hWnd, WM_SETTEXT, 0, (LPARAM)"Mux Usage Area Chart");
			}
			else
			{
				PEszset(v->m_hPE[nChartIndex], PEP_szMAINTITLE, "Mux Usage Line Chart");
				SendMessage(hWnd, WM_SETTEXT, 0, (LPARAM)"Mux Usage Line Chart");
			}
			PEszset(v->m_hPE[nChartIndex], PEP_szXAXISLABEL, "Time");

			SetupChartSubtitle(nChartIndex);
			PEnset(v->m_hPE[nChartIndex], PEP_nSUBSETS, nProgramCount + 2);
			PEnset(v->m_hPE[nChartIndex], PEP_nPOINTS, v->nGraphHistoricalPoints);
			PEnset(v->m_hPE[nChartIndex], PEP_bNORANDOMPOINTSTOEXPORT, TRUE);
			PEnset(v->m_hPE[nChartIndex], PEP_bFOCALRECT, FALSE);
			PEnset(v->m_hPE[nChartIndex], PEP_bALLOWBAR, FALSE);
			PEnset(v->m_hPE[nChartIndex], PEP_bALLOWPOPUP, FALSE);
			PEnset(v->m_hPE[nChartIndex], PEP_bPREPAREIMAGES, TRUE);
			PEnset(v->m_hPE[nChartIndex], PEP_bCACHEBMP, TRUE);
			PEnset(v->m_hPE[nChartIndex], PEP_bFIXEDFONTS, TRUE);
			PEnset(v->m_hPE[nChartIndex], PEP_nSHOWXAXIS, PESA_ALL);
			PEnset(v->m_hPE[nChartIndex], PEP_bXAXISVERTNUMBERING, TRUE);
			PEnset(v->m_hPE[nChartIndex], PEP_nFORCEVERTICALPOINTS, PEFVP_HORZ);
			PEszset(v->m_hPE[nChartIndex], PEP_szaPOINTLABELS, "");
				
			for (nPMTIndex = 0; nPMTIndex < MAX_PAT_ENTRIES; nPMTIndex++)
			{
				if (v->pat.pmt[nPMTIndex].nPMTPID == 0)
					break;
				if (v->pat.pmt[nPMTIndex].nProgramNumber == 0)
					continue;
				{
					char szTemp[256];
					char szChannelName[256] = {0};

					if (v->pChannelData[v->pat.pmt[nPMTIndex].nProgramNumber] != NULL)
					{
						lstrcpy(szChannelName, " ");
						lstrcat(szChannelName, v->pChannelData[v->pat.pmt[nPMTIndex].nProgramNumber]->szShortName);
					}
					wsprintf(szTemp, "Program %d%s\t", v->pat.pmt[nPMTIndex].nProgramNumber, szChannelName);
					lstrcat(szPointLabels, szTemp);
				}
			}	
			lstrcat(szPointLabels, "Other\tNULL Packets\t");
			PEvset(v->m_hPE[nChartIndex], PEP_szaSUBSETLABELS, szPointLabels, nProgramCount + 2);
			
			PEnset(v->m_hPE[nChartIndex], PEP_nGRADIENTBARS, 8);
			PEnset(v->m_hPE[nChartIndex], PEP_bMAINTITLEBOLD, TRUE);
			PEnset(v->m_hPE[nChartIndex], PEP_bSUBTITLEBOLD, TRUE);
			PEnset(v->m_hPE[nChartIndex], PEP_bLABELBOLD, TRUE);
			PEnset(v->m_hPE[nChartIndex], PEP_bLINESHADOWS, TRUE);
			PEnset(v->m_hPE[nChartIndex], PEP_nFONTSIZE, PEFS_LARGE);
			Charting__UpdateQuickStyle(nChartIndex);
			PEszset(v->m_hPE[nChartIndex], PEP_szYAXISLABEL, "Percentage of mux");

			PEreinitialize(v->m_hPE[nChartIndex]);
			PEresetimage(v->m_hPE[nChartIndex], 0, 0);

			AddMuxRateData((int)lParam);
			AddMuxRateData((int)lParam);
			AddMuxRateData((int)lParam);
			AddMuxRateData((int)lParam);

			SetTimer(hWnd, 100 + nChartIndex, v->nGraphRefreshRate, NULL);
			CheckCommonGraphMessages(hWnd, uMsg, wParam, lParam);
		}
		break;
	case WM_TIMER:
		{
			int nChartIndex = (int)GetWindowLongPtr(hWnd, GWLP_USERDATA);
			AddMuxRateData(nChartIndex);
			InvalidateRect(hWnd, NULL, FALSE);
		}
		break;
	default:
		if (CheckCommonGraphMessages(hWnd, uMsg, wParam, lParam) == TRUE)
			break;
		return DefWindowProc(hWnd, uMsg, wParam, lParam);
	}

	return FALSE;
}

void SetupActivePIDData(int nChartIndex)
{
	int nActivePIDCounter = 0;
	int i;
	int nChartPoints = 0;
	PIDCOUNTER pc[8192];
	DWORD * pdwPointColors;
	char szSaveData[1024];

	EnterCriticalSection(&v->ss.csPIDCounter);
	memcpy(&pc, &v->pc, sizeof(PIDCOUNTER) * 8192);
	LeaveCriticalSection(&v->ss.csPIDCounter);

	for (i = 0; i < 8192; i++)
	{
		if (pc[nActivePIDCounter].lnPackets)
			nChartPoints++;
		nActivePIDCounter++;
	}
	if (v->fActivePIDsByPID[nChartIndex] == FALSE)
		qsort(&pc, nActivePIDCounter, sizeof(PIDCOUNTER), SortPIDsByPackets);
	else
		qsort(&pc, nActivePIDCounter, sizeof(PIDCOUNTER), SortPIDsByPID);

	pdwPointColors = LocalAlloc(LPTR, sizeof(DWORD) * nChartPoints);
	PEnset(v->m_hPE[nChartIndex], PEP_nPOINTS, nChartPoints);

	if (v->fActivePIDsByPID[nChartIndex] == FALSE)
	{
		for (i = 0; i < nChartPoints; i++)
		{
			float f = ((float)pc[i].lnPackets / (float)v->lnCopyTotalTSPackets) * 100.0f;
			PEvsetcellEx(v->m_hPE[nChartIndex], PEP_faYDATA, 0, i, &f);
			PEvsetcell(v->m_hPE[nChartIndex], PEP_szaPOINTLABELS, i, FormatTooltipPID(pc[i].nPID));
			if (pc[i].fScrambled)
				pdwPointColors[i] = v->dwScrambledPIDColor;
			else
				pdwPointColors[i] = v->dwUnscrambledPIDColor;
			if (v->fSaveChartDataEnabled)
			{
				sprintf(szSaveData, "PID=%04x,Scrambled=%d,Percentage=%.3f",
					     pc[i].nPID, pc[i].fScrambled, f);
				SaveChartData(nChartIndex, szSaveData);
			}
		}
	}
	else
	{
		int nXAxisIndex = 0;

		for (i = 0; i < 8192; i++)
		{
			if (pc[i].lnPackets)
			{
				float f = ((float)pc[i].lnPackets / (float)v->lnCopyTotalTSPackets) * 100.0f;
				PEvsetcellEx(v->m_hPE[nChartIndex], PEP_faYDATA, 0, nXAxisIndex, &f);
				PEvsetcell(v->m_hPE[nChartIndex], PEP_szaPOINTLABELS, nXAxisIndex, FormatTooltipPID(pc[i].nPID));
				if (pc[i].fScrambled)
					pdwPointColors[nXAxisIndex] = v->dwScrambledPIDColor;
				else
					pdwPointColors[nXAxisIndex] = v->dwUnscrambledPIDColor;
				nXAxisIndex++;
				if (v->fSaveChartDataEnabled)
				{
					sprintf(szSaveData, "PID=%04x,Scrambled=%d,Percentage=%.3f",
							 pc[i].nPID, pc[i].fScrambled, f);
					SaveChartData(nChartIndex, szSaveData);
				}
			}
		}
	}
	PEvset(v->m_hPE[nChartIndex], PEP_dwaPOINTCOLORS, pdwPointColors, nChartPoints);
	LocalFree(pdwPointColors);
}

LRESULT FAR PASCAL ActivePIDsChartWindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg)
	{
	case WM_CREATE:
		{
			LPCREATESTRUCT lpcs = (LPCREATESTRUCT)lParam;
			int nChartIndex = *((int*)lpcs->lpCreateParams);
			int nAltFrequencies = 1;
			DWORD col = RGB(0, 198, 0);
			RECT rect;
			char szPointLabels[10 * 1024] = {0};

			SendMessage(hWnd, WM_SETICON, ICON_BIG, (LPARAM)v->hDialogIcon);
			v->fSetPoints[nChartIndex] = FALSE;
			v->fActivePIDsByPID[nChartIndex] = v->nChartParameters;

			GetClientRect(hWnd, &rect);
			v->m_hPE[nChartIndex] = PEcreate(PECONTROL_GRAPH, WS_VISIBLE, &rect, hWnd, 1001);
			SetupActivePIDData(nChartIndex);

			//PEnset(v->m_hPE[nChartIndex], PEP_bNOSTACKEDDATA, FALSE);
			//PEnset(v->m_hPE[nChartIndex], PEP_nPLOTTINGMETHOD, PEGPM_AREASTACKED);
			PEnset(v->m_hPE[nChartIndex], PEP_nDATASHADOWS, PEDS_NONE);	
			//PEnset(v->m_hPE[nChartIndex], PEP_bGRIDINFRONT, TRUE);
			PEszset(v->m_hPE[nChartIndex], PEP_szMAINTITLE, "Active PIDs Chart");
			SendMessage(hWnd, WM_SETTEXT, 0, (LPARAM)"Active PIDs Chart");

			SetupChartSubtitle(nChartIndex);
			PEnset(v->m_hPE[nChartIndex], PEP_nPLOTTINGMETHOD, PEGPM_BAR);
			PEnset(v->m_hPE[nChartIndex], PEP_bNORANDOMPOINTSTOEXPORT, TRUE);
			PEnset(v->m_hPE[nChartIndex], PEP_bFOCALRECT, FALSE);
			PEnset(v->m_hPE[nChartIndex], PEP_bALLOWBAR, FALSE);
			PEnset(v->m_hPE[nChartIndex], PEP_bALLOWPOPUP, FALSE);
			PEnset(v->m_hPE[nChartIndex], PEP_bPREPAREIMAGES, TRUE);
			PEnset(v->m_hPE[nChartIndex], PEP_bCACHEBMP, TRUE);
			PEnset(v->m_hPE[nChartIndex], PEP_bFIXEDFONTS, TRUE);
			PEnset(v->m_hPE[nChartIndex], PEP_nSHOWXAXIS, PESA_ALL);
			PEnset(v->m_hPE[nChartIndex], PEP_bXAXISVERTNUMBERING, TRUE);
			PEnset(v->m_hPE[nChartIndex], PEP_nFORCEVERTICALPOINTS, PEFVP_VERT);
			PEvset(v->m_hPE[nChartIndex], PEP_naALTFREQUENCIES, &nAltFrequencies, 1);

					
			PEnset(v->m_hPE[nChartIndex], PEP_nGRADIENTBARS, 8);
			//PEnset(v->m_hPE[nChartIndex], PEP_nTEXTSHADOWS, PETS_BOLD_TEXT);
			PEnset(v->m_hPE[nChartIndex], PEP_bMAINTITLEBOLD, TRUE);
			PEnset(v->m_hPE[nChartIndex], PEP_bSUBTITLEBOLD, TRUE);
			PEnset(v->m_hPE[nChartIndex], PEP_bLABELBOLD, TRUE);
			PEnset(v->m_hPE[nChartIndex], PEP_bLINESHADOWS, TRUE);
			PEnset(v->m_hPE[nChartIndex], PEP_nFONTSIZE, PEFS_LARGE);
			Charting__UpdateQuickStyle(nChartIndex);

			PEszset(v->m_hPE[nChartIndex], PEP_szXAXISLABEL, "PID");
			PEszset(v->m_hPE[nChartIndex], PEP_szYAXISLABEL, "Percentage of mux");

			PEreinitialize(v->m_hPE[nChartIndex]);
			PEresetimage(v->m_hPE[nChartIndex], 0, 0);

			SetTimer(hWnd, 100 + nChartIndex, v->nGraphRefreshRate, NULL);
			CheckCommonGraphMessages(hWnd, uMsg, wParam, lParam);
		}
		break;
	case WM_TIMER:
		{
			int nChartIndex = (int)GetWindowLongPtr(hWnd, GWLP_USERDATA);
			SetupActivePIDData(nChartIndex);
			InvalidateRect(hWnd, NULL, FALSE);
		}
		break;
	default:
		if (CheckCommonGraphMessages(hWnd, uMsg, wParam, lParam) == TRUE)
			break;
		return DefWindowProc(hWnd, uMsg, wParam, lParam);
	}

	return FALSE;
}

void ProgramUsageCheckMaxRate(int nChartIndex, float * fBandwidth)
{
	float fAllBandwidth = fBandwidth[0] + fBandwidth[1] + fBandwidth[2] + fBandwidth[3] + fBandwidth[4];

	if (fAllBandwidth > v->fVideoMaxRate[nChartIndex])
	{
		double d;
		
		v->fVideoMaxRate[nChartIndex] = fAllBandwidth;
		d = (double)v->fVideoMaxRate[nChartIndex];
		d += d / 20.0;
		PEvset(v->m_hPE[nChartIndex], PEP_fMANUALMAXY, &d, 1);
		PEvset(v->m_hPE[nChartIndex], PEP_fMANUALSTACKEDMAXY, &d, 1);

		PEreinitialize(v->m_hPE[nChartIndex]);
		PEresetimage(v->m_hPE[nChartIndex], 0, 0);
	}
}

void SetupProgramUsageData(int nChartIndex)
{
	int nPMTIndex, nESIndex;
	uint16_t nESPID;
	int s = 0;
	float fPercent;
	float fPIDRate;
	float fBandwidth[6];
	char szSaveData[1024];
	BYTE bPIDList[8192];

	memset(bPIDList, 0, sizeof(bPIDList));
	for (nPMTIndex = 0; nPMTIndex < MAX_PAT_ENTRIES; nPMTIndex++)
	{
		if (v->pat.pmt[nPMTIndex].nPMTPID == 0)
			break;
		if (v->pat.pmt[nPMTIndex].nProgramNumber == 0)
			continue;

		fBandwidth[0] = fBandwidth[1] = fBandwidth[2] = fBandwidth[3] = fBandwidth[4] = 0.0;
		for (nESIndex = 0; nESIndex < MAX_ESLIST_ENTRIES; nESIndex++)
		{
			int nBitrateIndex = 4;

			if (v->pat.pmt[nPMTIndex].es[nESIndex].nESPID == 0)
				break;
			nESPID = v->pat.pmt[nPMTIndex].es[nESIndex].nESPID;
			switch(v->pat.pmt[nPMTIndex].es[nESIndex].nStreamType)
			{
			case 0x01:	// mpeg-1 video
			case 0x02:	// mpeg-2 video
			case 0x10:	// MPEG-4 video
			case 0x1b:	// H.264 video
				nBitrateIndex = 0;
				break;
			case 0x80:	// dc-ii video
				if (v->nNetworkPID != 0x0010)
					nBitrateIndex = 0;
				break;
			case 0x03:	// MPEG-1 audio
			case 0x04:	// MPEG-2 audio
			case 0x06:	// maybe AC3
			case 0x0f:	// MPEG-2 AAC
			case 0x11:	// MPEG-4 audio
			case 0x81:	// AC3 audio
			case 0x83:	// LPCM audio
			case 0x85:	// DTS audio
			case 0xe6:	// WMA9 audio
				if (v->pat.pmt[nPMTIndex].es[nESIndex].nStreamType == 0x06)
				{
					if (   IsAC3AudioStream(nPMTIndex, nESIndex) == FALSE
						&& IsPCMAudioStream(nPMTIndex, nESIndex) == FALSE
						&& IsDTSAudioStream(nPMTIndex, nESIndex) == FALSE)
						break;
				}
				nBitrateIndex = 1;
				break;
			}
			if (nBitrateIndex < 4 && bPIDList[nESPID])
				nBitrateIndex += 2;

			if (v->lnPIDRateSamples[nESPID])
				fPIDRate = ((float)v->dPIDRate[nESPID] / (float)v->lnPIDRateSamples[nESPID]) * 8.0f;
			else
			{
				fPercent = ((float)v->lnPIDCounter[nESPID] / (float)v->lnTotalTSPackets) * 100.0f;
				fPIDRate = ((float)v->dDisplayMuxRate / (float)v->nMuxRateCounter) * 8.0f;
				fPIDRate  = ((fPIDRate / 100.0f) * fPercent);
			}
			fBandwidth[nBitrateIndex] += fPIDRate;
			bPIDList[nESPID] = 1;
		}
		ProgramUsageCheckMaxRate(nChartIndex, fBandwidth);
		PEvsetcellEx(v->m_hPE[nChartIndex], PEP_faYDATA, 0, s, &fBandwidth[0]);
		PEvsetcellEx(v->m_hPE[nChartIndex], PEP_faYDATA, 1, s, &fBandwidth[1]);
		PEvsetcellEx(v->m_hPE[nChartIndex], PEP_faYDATA, 2, s, &fBandwidth[2]);
		PEvsetcellEx(v->m_hPE[nChartIndex], PEP_faYDATA, 3, s, &fBandwidth[3]);
		PEvsetcellEx(v->m_hPE[nChartIndex], PEP_faYDATA, 4, s, &fBandwidth[4]);
		s++;
		if (v->fSaveChartDataEnabled)
		{
			sprintf(szSaveData, "Program=%d,Video=%.3f,Audio=%.3f,Virt-Video=%.3f,Virt-Audio=%.3f,Other=%.3f",
				     v->pat.pmt[nPMTIndex].nProgramNumber,
					 fBandwidth[0], fBandwidth[1], fBandwidth[2], fBandwidth[3], fBandwidth[4]);
			SaveChartData(nChartIndex, szSaveData);
		}
	}

	// SI/Other PIDs
	{
		float fGhostBandwidth[5];
		fBandwidth[0] = fBandwidth[1] = fBandwidth[2] = fBandwidth[3] = fBandwidth[4] = 0.0;
		fGhostBandwidth[0] = fGhostBandwidth[1] = fGhostBandwidth[2] = fGhostBandwidth[3] = fGhostBandwidth[4] = 0.0;
		for (nESPID = 0; nESPID < 8191; nESPID++)
		{
			char szTemp[256];

			if (bPIDList[nESPID] == 1)
				continue;
			if (v->lnPIDRateSamples[nESPID])
				fPIDRate = ((float)v->dPIDRate[nESPID] / (float)v->lnPIDRateSamples[nESPID]) * 8.0f;
			else
			{
				fPercent = ((float)v->lnPIDCounter[nESPID] / (float)v->lnTotalTSPackets) * 100.0f;
				fPIDRate = ((float)v->dDisplayMuxRate / (float)v->nMuxRateCounter) * 8.0f;
				fPIDRate  = ((fPIDRate / 100.0f) * fPercent);
			}
			if (GetPIDTooltipInfo(nESPID, szTemp, sizeof(szTemp)) == TRUE)
				fBandwidth[4] += fPIDRate;
			else
				fGhostBandwidth[4] += fPIDRate;
		}
		ProgramUsageCheckMaxRate(nChartIndex, fBandwidth);
		PEvsetcellEx(v->m_hPE[nChartIndex], PEP_faYDATA, 0, s, &fBandwidth[0]);
		PEvsetcellEx(v->m_hPE[nChartIndex], PEP_faYDATA, 1, s, &fBandwidth[1]);
		PEvsetcellEx(v->m_hPE[nChartIndex], PEP_faYDATA, 2, s, &fBandwidth[2]);
		PEvsetcellEx(v->m_hPE[nChartIndex], PEP_faYDATA, 3, s, &fBandwidth[3]);
		PEvsetcellEx(v->m_hPE[nChartIndex], PEP_faYDATA, 4, s, &fBandwidth[4]);
		s++;
		if (v->fSaveChartDataEnabled)
		{
			sprintf(szSaveData, "SI,Other=%.3f", fBandwidth[4]);
			SaveChartData(nChartIndex, szSaveData);
		}
		ProgramUsageCheckMaxRate(nChartIndex, fGhostBandwidth);
		PEvsetcellEx(v->m_hPE[nChartIndex], PEP_faYDATA, 0, s, &fGhostBandwidth[0]);
		PEvsetcellEx(v->m_hPE[nChartIndex], PEP_faYDATA, 1, s, &fGhostBandwidth[1]);
		PEvsetcellEx(v->m_hPE[nChartIndex], PEP_faYDATA, 2, s, &fGhostBandwidth[2]);
		PEvsetcellEx(v->m_hPE[nChartIndex], PEP_faYDATA, 3, s, &fGhostBandwidth[3]);
		PEvsetcellEx(v->m_hPE[nChartIndex], PEP_faYDATA, 4, s, &fGhostBandwidth[4]);
		s++;
		if (v->fSaveChartDataEnabled)
		{
			sprintf(szSaveData, "Ghost,Other=%.3f", fBandwidth[4]);
			SaveChartData(nChartIndex, szSaveData);
		}
	}

	// NULL PID
	fBandwidth[0] = fBandwidth[1] = fBandwidth[2] = fBandwidth[3] = fBandwidth[4] = 0.0;
	fPercent = ((float)v->lnPIDCounter[0x1fff] / (float)v->lnTotalTSPackets) * 100.0f;
	fPIDRate = ((float)v->dDisplayMuxRate / (float)v->nMuxRateCounter) * 8.0f;
	fBandwidth[4]  = ((fPIDRate / 100.0f) * fPercent);
	ProgramUsageCheckMaxRate(nChartIndex, fBandwidth);
	PEvsetcellEx(v->m_hPE[nChartIndex], PEP_faYDATA, 0, s, &fBandwidth[0]);
	PEvsetcellEx(v->m_hPE[nChartIndex], PEP_faYDATA, 1, s, &fBandwidth[1]);
	PEvsetcellEx(v->m_hPE[nChartIndex], PEP_faYDATA, 2, s, &fBandwidth[2]);
	PEvsetcellEx(v->m_hPE[nChartIndex], PEP_faYDATA, 3, s, &fBandwidth[3]);
	PEvsetcellEx(v->m_hPE[nChartIndex], PEP_faYDATA, 4, s, &fBandwidth[4]);
	if (v->fSaveChartDataEnabled)
	{
		sprintf(szSaveData, "Null,Other=%.3f", fBandwidth[4]);
		SaveChartData(nChartIndex, szSaveData);
	}
}

LRESULT FAR PASCAL ProgramUsageChartWindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg)
	{
	case WM_CREATE:
		{
			int nProgramCount;
			int nCellCount = 0;
			int nPMTIndex;
			DWORD dwColorArray[5] = { RGB(0, 198, 0),
				                      RGB(0, 0, 198 ),
									  RGB(0 ,128,0 ),
									  RGB( 0,0,128 ),
									  RGB(198, 198, 0)};
			RECT rect;
			LPCREATESTRUCT lpcs = (LPCREATESTRUCT)lParam;
			int nChartIndex = *((int*)lpcs->lpCreateParams);

			v->fVideoMaxRate[nChartIndex] = 0.0f;
			nProgramCount = GetProgramCount();
			SendMessage(hWnd, WM_SETICON, ICON_BIG, (LPARAM)v->hDialogIcon);
			SendMessage(hWnd, WM_SETTEXT, 0, (LPARAM)"Program Usage Chart");

			GetClientRect(hWnd, &rect);			
			v->m_hPE[nChartIndex] = PEcreate(PECONTROL_GRAPH, WS_VISIBLE, &rect, hWnd, 1001);
			
			PEnset(v->m_hPE[nChartIndex], PEP_bPREPAREIMAGES, TRUE);
			PEnset(v->m_hPE[nChartIndex], PEP_nSUBSETS, 5);
			PEnset(v->m_hPE[nChartIndex], PEP_nPOINTS, nProgramCount + 3);
			PEnset(v->m_hPE[nChartIndex], PEP_nALTFREQTHRESHOLD, nProgramCount + 3 + 1); 
			// subset labels //
			PEvsetcell( v->m_hPE[nChartIndex], PEP_szaSUBSETLABELS, 0, "Video" );
			PEvsetcell( v->m_hPE[nChartIndex], PEP_szaSUBSETLABELS, 1, "Audio" );
			PEvsetcell( v->m_hPE[nChartIndex], PEP_szaSUBSETLABELS, 2, "Virtual Video" );
			PEvsetcell( v->m_hPE[nChartIndex], PEP_szaSUBSETLABELS, 3, "Virtual Audio" );			
			PEvsetcell( v->m_hPE[nChartIndex], PEP_szaSUBSETLABELS, 4, "Other" );
			PEnset(v->m_hPE[nChartIndex], PEP_bXAXISVERTNUMBERING, TRUE);
			PEnset(v->m_hPE[nChartIndex], PEP_nFORCEVERTICALPOINTS, PEFVP_VERT);

			PEnset(v->m_hPE[nChartIndex], PEP_nMANUALSCALECONTROLY, PEMSC_MINMAX);
			{
				double d = 0.1f;
				PEvset(v->m_hPE[nChartIndex], PEP_fMANUALMINY, &d, 1);
				PEvset(v->m_hPE[nChartIndex], PEP_fMANUALSTACKEDMINY, &d, 1);
				d = 1000.0f;
				PEvset(v->m_hPE[nChartIndex], PEP_fMANUALMAXY, &d, 1);
				PEvset(v->m_hPE[nChartIndex], PEP_fMANUALSTACKEDMAXY, &d, 1);
			}

			SetupProgramUsageData(nChartIndex);

			PEnset(v->m_hPE[nChartIndex], PEP_nDATASHADOWS, PEDS_SHADOWS);
			PEszset(v->m_hPE[nChartIndex], PEP_szMAINTITLE, "Program Usage");
			SetupChartSubtitle(nChartIndex);
			PEszset(v->m_hPE[nChartIndex], PEP_szYAXISLABEL, "Bitrate");
			PEszset(v->m_hPE[nChartIndex], PEP_szXAXISLABEL, "");
			PEnset(v->m_hPE[nChartIndex], PEP_bFOCALRECT, FALSE);
			PEnset(v->m_hPE[nChartIndex], PEP_nPLOTTINGMETHOD, PEGPM_BAR);
			PEnset(v->m_hPE[nChartIndex], PEP_nGRIDLINECONTROL, PEGLC_NONE);
			PEnset(v->m_hPE[nChartIndex], PEP_bALLOWRIBBON, TRUE);
			PEnset(v->m_hPE[nChartIndex], PEP_nALLOWZOOMING, PEAZ_HORZANDVERT);
			PEnset(v->m_hPE[nChartIndex], PEP_nZOOMSTYLE, PEZS_RO2_NOT);
			Charting__UpdateQuickStyle(nChartIndex);

			// point labels
			for (nPMTIndex = 0; nPMTIndex < MAX_PAT_ENTRIES; nPMTIndex++)
			{
				if (v->pat.pmt[nPMTIndex].nPMTPID == 0)
					break;
				if (v->pat.pmt[nPMTIndex].nProgramNumber == 0)
					continue;
				{
					char szTemp[256];
					char szChannelName[256] = {0};

					if (v->pChannelData[v->pat.pmt[nPMTIndex].nProgramNumber] != NULL)
					{
						lstrcpy(szChannelName, "-");
						lstrcat(szChannelName, v->pChannelData[v->pat.pmt[nPMTIndex].nProgramNumber]->szShortName);
					}
					wsprintf(szTemp, "%d%s", v->pat.pmt[nPMTIndex].nProgramNumber, szChannelName);
					PEvsetcell(v->m_hPE[nChartIndex], PEP_szaPOINTLABELS, nCellCount++, szTemp);
				}
			}	
			PEvsetcell(v->m_hPE[nChartIndex], PEP_szaPOINTLABELS, nCellCount++, "SI/Other");
			PEvsetcell(v->m_hPE[nChartIndex], PEP_szaPOINTLABELS, nCellCount++, "Ghost Data");
			PEvsetcell(v->m_hPE[nChartIndex], PEP_szaPOINTLABELS, nCellCount++, "NULL Packets");
			PEnset(v->m_hPE[nChartIndex], PEP_nFONTSIZE, PEFS_SMALL);
			{
				int n = 0;
				PEvsetcell(v->m_hPE[nChartIndex], PEP_naSUBSETSTOLEGEND, -1, &n);
			}

			// subset colors //
			PEvsetEx(v->m_hPE[nChartIndex], PEP_dwaSUBSETCOLORS, 0, 5, dwColorArray, 0);

			// Enable Stacked type charts //
			PEnset(v->m_hPE[nChartIndex], PEP_bNOSTACKEDDATA, FALSE);

			// Set plotting method //
			PEnset(v->m_hPE[nChartIndex], PEP_nPLOTTINGMETHOD, PEGPM_BARSTACKED);

			// Add a table //
			//PEnset(v->m_hPE[nChartIndex], PEP_nGRAPHPLUSTABLE, PEGPT_BOTH);
			PEnset(v->m_hPE[nChartIndex], PEP_nGRAPHPLUSTABLE, PEGPT_GRAPH);
			//PEnset(v->m_hPE[nChartIndex], PEP_nDATAPRECISION, 2);
			PEnset(v->m_hPE[nChartIndex], PEP_bALLOWHORZBARSTACKED, TRUE);
			PEnset(v->m_hPE[nChartIndex], PEP_bALLOWTABLE, FALSE);

			PEreinitialize(v->m_hPE[nChartIndex]);
			PEresetimage(v->m_hPE[nChartIndex], 0, 0);

			SetTimer(hWnd, 100 + nChartIndex, v->nGraphRefreshRate, NULL);
			CheckCommonGraphMessages(hWnd, uMsg, wParam, lParam);
		}
		break;
	case WM_TIMER:
		{
			int nChartIndex = (int)GetWindowLongPtr(hWnd, GWLP_USERDATA);
			SetupProgramUsageData(nChartIndex);
			InvalidateRect(hWnd, NULL, FALSE);
		}
		break;
	default:
		if (CheckCommonGraphMessages(hWnd, uMsg, wParam, lParam) == TRUE)
			break;
		return DefWindowProc(hWnd, uMsg, wParam, lParam);
	}

	return FALSE;
}

void AddSignalChartData(int nChartIndex)
{
	float fNewValues[2];
	SYSTEMTIME stLocal;
	char szTemp[128];
	char szSaveData[128] = {""};

	ExtractSignalData(v->nSignalChartMode[nChartIndex], &fNewValues[0], &fNewValues[1]);
	switch(v->nSignalChartMode[nChartIndex])
	{
	case SIGNAL_CHART_MODE_SNR:
		if (v->fSaveChartDataEnabled)
			sprintf(szSaveData, "SNR=%.3f", fNewValues[0]);
		break;
	case SIGNAL_CHART_MODE_BER:
		if (v->fSaveChartDataEnabled)
			sprintf(szSaveData, "BER=%.3f", fNewValues[0]);
		break;
	case SIGNAL_CHART_MODE_QUALITY:
		if (v->fSaveChartDataEnabled)
			sprintf(szSaveData, "Quality=%d,Signal=%d", (int)fNewValues[0], (int)fNewValues[1]);
		break;
	case SIGNAL_CHART_MODE_QUALITY_DBM:
		if (v->fSaveChartDataEnabled)
			sprintf(szSaveData, "Quality=%d,dBm=%.3f", (int)fNewValues[0], fNewValues[1]);
		break;
	}

	PEvset(v->m_hPE[nChartIndex], PEP_faAPPENDYDATA, fNewValues, 1);

	GetLocalTime(&stLocal);
	wsprintf(szTemp, "%02d:%02d", stLocal.wHour, stLocal.wMinute);
	PEvset(v->m_hPE[nChartIndex], PEP_szaAPPENDPOINTLABELDATA, szTemp, 1);
	
	PEreinitialize(v->m_hPE[nChartIndex]);
	PEresetimage(v->m_hPE[nChartIndex], 0, 0);

	if (v->fSaveChartDataEnabled)
		SaveChartData(nChartIndex, szSaveData);
}

LRESULT FAR PASCAL SignalChartWindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg)
	{
	case WM_CREATE:
		{
			LPCREATESTRUCT lpcs = (LPCREATESTRUCT)lParam;
			int nChartIndex = *((int*)lpcs->lpCreateParams);
			DWORD col = RGB(0, 198, 0);
			RECT rect;
			double arg;
			char szSignal[64];
			char szPointLabels[256] = {0};

			SendMessage(hWnd, WM_SETICON, ICON_BIG, (LPARAM)v->hDialogIcon);

			GetSourceInfoLine(2, szSignal);
			v->nSignalChartMode[nChartIndex] = DetermineSignalType(szSignal);

			switch(v->nSignalChartMode[nChartIndex])
			{
			case SIGNAL_CHART_MODE_SNR:
				lstrcpy(szPointLabels, "dB SNR\t");
				break;
			case SIGNAL_CHART_MODE_BER:
				lstrcpy(szPointLabels, "BER\t");
				break;
			case SIGNAL_CHART_MODE_QUALITY:
				lstrcpy(szPointLabels, "Signal\tQuality\t");
				break;
			case SIGNAL_CHART_MODE_QUALITY_DBM:
				lstrcpy(szPointLabels, "Signal\tdBm\t");
				break;
			}

			GetClientRect(hWnd, &rect);
			v->m_hPE[nChartIndex] = PEcreate(PECONTROL_GRAPH, WS_VISIBLE, &rect, hWnd, 1001);
			if (v->nSignalChartMode[nChartIndex] == SIGNAL_CHART_MODE_QUALITY ||
				v->nSignalChartMode[nChartIndex] == SIGNAL_CHART_MODE_QUALITY_DBM)
			{
				PEnset(v->m_hPE[nChartIndex], PEP_nSUBSETS, 2);
				PEvset(v->m_hPE[nChartIndex], PEP_szaSUBSETLABELS , szPointLabels, 2);
			}
			else
			{
				PEnset(v->m_hPE[nChartIndex], PEP_nSUBSETS, 1);
				PEvset(v->m_hPE[nChartIndex], PEP_szaSUBSETLABELS , szPointLabels, 1);
			}
			PEszset(v->m_hPE[nChartIndex], PEP_szMAINTITLE, "Signal Chart");
			PEszset(v->m_hPE[nChartIndex], PEP_szXAXISLABEL, "Time");

			SetupChartSubtitle(nChartIndex);
			PEnset(v->m_hPE[nChartIndex], PEP_nPOINTS, v->nGraphHistoricalPoints);
			PEnset(v->m_hPE[nChartIndex], PEP_bNORANDOMPOINTSTOEXPORT, TRUE);
			PEnset(v->m_hPE[nChartIndex], PEP_bFOCALRECT, FALSE);
			PEnset(v->m_hPE[nChartIndex], PEP_bALLOWBAR, FALSE);
			PEnset(v->m_hPE[nChartIndex], PEP_bALLOWPOPUP, FALSE);
			PEnset(v->m_hPE[nChartIndex], PEP_bPREPAREIMAGES, TRUE);
			PEnset(v->m_hPE[nChartIndex], PEP_bCACHEBMP, TRUE);
			PEnset(v->m_hPE[nChartIndex], PEP_bFIXEDFONTS, TRUE);
			PEnset(v->m_hPE[nChartIndex], PEP_nSHOWXAXIS, PESA_ALL);
			PEnset(v->m_hPE[nChartIndex], PEP_bXAXISVERTNUMBERING, TRUE);
			PEnset(v->m_hPE[nChartIndex], PEP_nFORCEVERTICALPOINTS, PEFVP_HORZ);
						
			arg = 0.0f;
			PEvset(v->m_hPE[nChartIndex], PEP_fMANUALMINY, &arg, 0);
			PEnset(v->m_hPE[nChartIndex], PEP_nMANUALSCALECONTROLY, PEMSC_MIN);

			PEszset(v->m_hPE[nChartIndex], PEP_szaPOINTLABELS, "");

			PEnset(v->m_hPE[nChartIndex], PEP_nGRADIENTBARS, 8);
			PEnset(v->m_hPE[nChartIndex], PEP_bMAINTITLEBOLD, TRUE);
			PEnset(v->m_hPE[nChartIndex], PEP_bSUBTITLEBOLD, TRUE);
			PEnset(v->m_hPE[nChartIndex], PEP_bLABELBOLD, TRUE);
			PEnset(v->m_hPE[nChartIndex], PEP_bLINESHADOWS, TRUE);
			PEnset(v->m_hPE[nChartIndex], PEP_nFONTSIZE, PEFS_LARGE);
			Charting__UpdateQuickStyle(nChartIndex);

			switch(v->nSignalChartMode[nChartIndex])
			{
			case SIGNAL_CHART_MODE_SNR:
				PEszset(v->m_hPE[nChartIndex], PEP_szYAXISLABEL, "dB SNR");
				break;
			case SIGNAL_CHART_MODE_BER:
				PEszset(v->m_hPE[nChartIndex], PEP_szYAXISLABEL, "Bit Error Rate");
				PEnset(v->m_hPE[nChartIndex], PEP_nYAXISSCALECONTROL, PEAC_LOG);
				PEnset(v->m_hPE[nChartIndex], PEP_bAUTOSCALEDATA, FALSE);
				PEnset(v->m_hPE[nChartIndex], PEP_bLOGSCALEEXPLABELS, TRUE);
				break;
			case SIGNAL_CHART_MODE_QUALITY:
				PEszset(v->m_hPE[nChartIndex], PEP_szYAXISLABEL, "Signal/Quality");
				break;
			case SIGNAL_CHART_MODE_QUALITY_DBM:
				PEszset(v->m_hPE[nChartIndex], PEP_szYAXISLABEL, "Quality/dBm");
				break;
			}

			PEreinitialize(v->m_hPE[nChartIndex]);
			PEresetimage(v->m_hPE[nChartIndex], 0, 0);

			AddSignalChartData(nChartIndex);
			AddSignalChartData(nChartIndex);
			AddSignalChartData(nChartIndex);
			AddSignalChartData(nChartIndex);

			SendMessage(hWnd, WM_SETTEXT, 0, (LPARAM)"Signal Chart");
			SetTimer(hWnd, 100 + nChartIndex, v->nGraphRefreshRate, NULL);
			CheckCommonGraphMessages(hWnd, uMsg, wParam, lParam);
		}
		break;
	case WM_TIMER:
		{
			int nChartIndex = (int)GetWindowLongPtr(hWnd, GWLP_USERDATA);
			AddSignalChartData(nChartIndex);
			InvalidateRect(hWnd, NULL, FALSE);
		}
		break;
	default:
		if (CheckCommonGraphMessages(hWnd, uMsg, wParam, lParam) == TRUE)
			break;
		return DefWindowProc(hWnd, uMsg, wParam, lParam);
	}
	return FALSE;
}

LRESULT FAR PASCAL VideoCompositionChartWindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg)
	{
	case WM_CREATE:
		{
			LPCREATESTRUCT lpcs = (LPCREATESTRUCT)lParam;
			int nChartIndex = *((int*)lpcs->lpCreateParams);
			RECT rect;

			// Various variable setups
			memset(v->nPictureDataCount[nChartIndex], 0, sizeof(int) * MAX_CHART_GOP_LENGTH);
			memset(v->nPictureType[nChartIndex], 0, sizeof(int) * MAX_CHART_GOP_LENGTH);
			v->nPictureIndex[nChartIndex] = -1;
			v->nVideoCompositionPoints[nChartIndex] = 0;
			if ((v->nChartParameters & 0xffff0000) == 0x00010000)
			{
				// H264
				if (v->nGraphHistoricalPoints < MAX_CHART_GOP_LENGTH)
					v->nVideoCompositionPoints[nChartIndex] = v->nGraphHistoricalPoints;
				else
					v->nVideoCompositionPoints[nChartIndex] = MAX_CHART_GOP_LENGTH;
			}
			v->fVideoMaxRate[nChartIndex] = 0.0f;

			// Actually create the chart
			GetClientRect(hWnd, &rect);
			v->m_hPE[nChartIndex] = PEcreate(PECONTROL_GRAPH, WS_VISIBLE, &rect, hWnd, 1001);
			if (v->m_hPE[nChartIndex]) 
			{
				int nProgramNumber;
				char szTemp[256];
				char szChannelName[256] = {0};
				char szInitialPointLabel[128] = {"Waiting for start of GOP"};

				PEnset(v->m_hPE[nChartIndex], PEP_nPOINTS, 0);
				PEnset(v->m_hPE[nChartIndex], PEP_bPREPAREIMAGES, TRUE);
				PEnset(v->m_hPE[nChartIndex], PEP_nSUBSETS, 1);
				PEnset(v->m_hPE[nChartIndex], PEP_nPLOTTINGMETHOD, PEGPM_BAR);
				PEnset(v->m_hPE[nChartIndex], PEP_nGRIDLINECONTROL, PEGLC_NONE);
				PEnset(v->m_hPE[nChartIndex], PEP_bALLOWRIBBON, TRUE);
				PEnset(v->m_hPE[nChartIndex], PEP_nALLOWZOOMING, PEAZ_HORZANDVERT);
				PEnset(v->m_hPE[nChartIndex], PEP_nZOOMSTYLE, PEZS_RO2_NOT);
				PEvsetcell(v->m_hPE[nChartIndex], PEP_szaSUBSETLABELS, 0, "Video Data" );
				PEszset(v->m_hPE[nChartIndex], PEP_szMAINTITLE, "Video Composition");
				PEszset(v->m_hPE[nChartIndex], PEP_szYAXISLABEL, "Bits per Picture");
				PEszset(v->m_hPE[nChartIndex], PEP_szXAXISLABEL, "Picture Type");
				if ((v->nChartParameters & 0xffff0000) == 0x00010000)
					lstrcpy(szInitialPointLabel, "Waiting for first picture");
				PEvsetcell( v->m_hPE[nChartIndex], PEP_szaPOINTLABELS, 0, szInitialPointLabel);
				{
					float fData = 0.0;
					PEvsetcellEx(v->m_hPE[nChartIndex], PEP_faYDATA, 0, 0, &fData);
				}

				nProgramNumber = v->pat.pmt[v->nSelectedProgram].nProgramNumber;
				if (v->pChannelData[nProgramNumber] != NULL)
				{
					lstrcpy(szChannelName, " ");
					lstrcat(szChannelName, v->pChannelData[nProgramNumber]->szShortName);
				}
				wsprintf(szTemp, "Program %d%s", nProgramNumber, szChannelName);
				PEszset(v->m_hPE[nChartIndex], PEP_szSUBTITLE, szTemp); 
				
				Charting__UpdateQuickStyle(nChartIndex);

				PEnset(v->m_hPE[nChartIndex], PEP_nMANUALSCALECONTROLY, PEMSC_MINMAX);
				{
					double d = 0.0f;
					PEvset(v->m_hPE[nChartIndex], PEP_fMANUALMINY, &d, 1);
					d = 1000.0f;
					PEvset(v->m_hPE[nChartIndex], PEP_fMANUALMAXY, &d, 1);
				}
				PEreinitialize(v->m_hPE[nChartIndex]);
				PEresetimage(v->m_hPE[nChartIndex], 0, 0);
			}

			SendMessage(hWnd, WM_SETICON, ICON_BIG, (LPARAM)v->hDialogIcon);
			SendMessage(hWnd, WM_SETTEXT, 0, (LPARAM)"Video Composition Chart");

			v->nMaxGOPLength[nChartIndex] = 0;
			v->nMinGOPLength[nChartIndex] = 9999999;
			v->nTotalGOPLength[nChartIndex] = 0;
			v->nGOPLengthSamples[nChartIndex] = 0;

#ifdef PRO_BROKEN
			if ((v->nChartParameters & 0xffff0000) == 0x00010000)
			{
				// H.264 - initialise the decoder
				EnterCriticalSection(&v->csH264VideoChart);
				H264Dec = Fvd_New_Decoder(FVD_MPEG4_H264);
				Fvd_Dec_Set_Param(H264Dec, FVD_CMD_NO_FILTER, 1,  0);
				//Fvd_Dec_Set_Param(H264Dec, FVD_CMD_CPU, FVD_CPU_DETECT, 0);
				LeaveCriticalSection(&v->csH264VideoChart);
			}
#endif PRO_BROKEN
			CheckCommonGraphMessages(hWnd, uMsg, wParam, lParam);
		}
		break;
	case WM_DESTROY:
		{
			int nChartIndex = (int)GetWindowLongPtr(hWnd, GWLP_USERDATA);
			CheckCommonGraphMessages(hWnd, uMsg, wParam, lParam);
#ifdef PRO_BROKEN
			if ((v->nVideoCompositionPID[nChartIndex] & 0xffff0000) == 0x00010000)
			{
				EnterCriticalSection(&v->csH264VideoChart);
				Fvd_Delete_Decoder(H264Dec);
				H264Dec = 0;
				LeaveCriticalSection(&v->csH264VideoChart);
			}
#endif PRO_BROKEN
			v->nVideoCompositionPID[nChartIndex] = -1;
		}
		break;
	default:
		if (CheckCommonGraphMessages(hWnd, uMsg, wParam, lParam) == TRUE)
			break;
		return DefWindowProc(hWnd, uMsg, wParam, lParam);
	}
	return FALSE;
}

void UpdateVideoCompositionChart(int nGOPLength, int nChartIndex, BOOL fH264)
{
	int nPicture;
	char szNewXAxisLabel[256];

	if (v->nVideoCompositionPoints[nChartIndex] < nGOPLength || fH264)
	{
		DWORD dwPointColors[MAX_CHART_GOP_LENGTH];

		v->nVideoCompositionPoints[nChartIndex] = nGOPLength;
		PEnset(v->m_hPE[nChartIndex], PEP_nPOINTS, v->nVideoCompositionPoints[nChartIndex] + 1);
		PEnset(v->m_hPE[nChartIndex], PEP_nALTFREQTHRESHOLD, v->nVideoCompositionPoints[nChartIndex] + 1); 
		for (nPicture = 0; nPicture <= v->nVideoCompositionPoints[nChartIndex]; nPicture++)
		{
			if (!fH264)
			{
				switch(v->nPictureType[nChartIndex][nPicture])
				{
				case 1:		// I
					dwPointColors[nPicture] = RGB(0x00, 0xff, 0x00);
					break;
				case 2:		// P
					dwPointColors[nPicture] = RGB(0xff, 0xff, 0x00);
					break;
				case 3:		// B
					dwPointColors[nPicture] = RGB(0x00, 0x00, 0xff);
					break;
				case 4:		// D
					dwPointColors[nPicture] = RGB(0xff, 0x00, 0x00);
					break;
				}
			}
			else
			{
				switch(v->nPictureType[nChartIndex][nPicture])
				{
				case 1:
				case 2:
					dwPointColors[nPicture] = RGB(0x00, 0xff, 0x00);
					break;
				case 3:
					dwPointColors[nPicture] = RGB(0xff, 0xff, 0x00);
					break;
				case 4:
					dwPointColors[nPicture] = RGB(0x00, 0x00, 0xff);
					break;
				default:
					{
						int a=1;
						a=1;
					}
					break;
				}
			}
		}
		PEvset(v->m_hPE[nChartIndex], PEP_dwaPOINTCOLORS, dwPointColors, v->nVideoCompositionPoints[nChartIndex] + 1);
		//PEreinitialize(v->m_hPE[nChartIndex]);
		//PEresetimage(v->m_hPE[nChartIndex], 0, 0);
	}

	for (nPicture = 0; nPicture <= v->nVideoCompositionPoints[nChartIndex]; nPicture++)
	{
		float fData = (float)v->nPictureDataCount[nChartIndex][nPicture] * 8.0f;
		char szPictureType[4] = {" "};

		PEvsetcellEx(v->m_hPE[nChartIndex], PEP_faYDATA, 0, nPicture, &fData);
		if (fData > v->fVideoMaxRate[nChartIndex])
		{
			double d;
			
			v->fVideoMaxRate[nChartIndex] = fData;
			d = (double)v->fVideoMaxRate[nChartIndex];
			d += d / 10;
			PEvset(v->m_hPE[nChartIndex], PEP_fMANUALMAXY, &d, 1);
		}

		if (!fH264)
		{
			switch(v->nPictureType[nChartIndex][nPicture])
			{
			case 1:		// I
				szPictureType[0] = 'I';
				break;
			case 2:		// P
				szPictureType[0] = 'P';
				break;
			case 3:		// B
				szPictureType[0] = 'B';
				break;
			case 4:		// D
				szPictureType[0] = 'D';
				break;
			}
		}
		else
		{
			switch(v->nPictureType[nChartIndex][nPicture])
			{
			case 1:
			case 2:
				lstrcpy(szPictureType, "I");
				break;
			case 3:
				lstrcpy(szPictureType, "P");
				break;
			case 4:
				lstrcpy(szPictureType, "B");
				break;
			}
		}
		PEvsetcell(v->m_hPE[nChartIndex], PEP_szaPOINTLABELS, nPicture, szPictureType);
	}

	if (!fH264)
	{
		// nGOPLength is really an index, so we bump it by 1 to get the actual
		// number of pictures in the GOP
		nGOPLength++;
		if (nGOPLength > v->nMaxGOPLength[nChartIndex])
			v->nMaxGOPLength[nChartIndex] = nGOPLength;
		if (nGOPLength < v->nMinGOPLength[nChartIndex])
			v->nMinGOPLength[nChartIndex] = nGOPLength;
		v->nTotalGOPLength[nChartIndex] += nGOPLength;
		v->nGOPLengthSamples[nChartIndex]++;
		wsprintf(szNewXAxisLabel, "Picture Type (Max GOP Length: %d Min: %d Average: %d)",
				 v->nMaxGOPLength[nChartIndex], 
				 v->nMinGOPLength[nChartIndex],
				 v->nTotalGOPLength[nChartIndex] / v->nGOPLengthSamples[nChartIndex]);		     
		PEszset(v->m_hPE[nChartIndex], PEP_szXAXISLABEL, szNewXAxisLabel);
		memset(v->nPictureDataCount[nChartIndex], 0, sizeof(int) * MAX_CHART_GOP_LENGTH);
		memset(v->nPictureType[nChartIndex], 0, sizeof(int) * MAX_CHART_GOP_LENGTH);
	}

	PEreinitialize(v->m_hPE[nChartIndex]);
	PEresetimage(v->m_hPE[nChartIndex], 0, 0);
	InvalidateRect(v->hWndChart[nChartIndex], NULL, FALSE);
}

#ifdef PRO_BROKEN
void InputH264VideoCompositionESData(BYTE * pPESPacket, int nPESLength, int nChartIndex)
{
	int nMaxPoints = v->nGraphHistoricalPoints;
	if (v->nGraphHistoricalPoints >= MAX_CHART_GOP_LENGTH)
		nMaxPoints = MAX_CHART_GOP_LENGTH;

	EnterCriticalSection(&v->csH264VideoChart);
	if (H264Dec == 0)
	{
		LeaveCriticalSection(&v->csH264VideoChart);
		return;
	}

	while (nPESLength)
	{
		FVD_ERROR_CODE Error_Status;
		int nRead;
		
		nRead = Fvd_Dec_Decode(H264Dec, pPESPacket, nPESLength);
		Error_Status = (FVD_ERROR_CODE)Fvd_Dec_Get_Param(H264Dec, FVD_CMD_GET_ERROR_STATUS);
		if (Error_Status != FVD_ERROR_NONE)
		{
			char szTemp[512];
			wsprintf(szTemp, "TSReader: H264 Chart error %d\n", Error_Status);
			OutputDebugString(szTemp);
			break;
		}
		if (nRead < 0)
		{
			OutputDebugString("TSReader: H264 Chart error decoding\n");
			break;
		}

		nPESLength -= nRead;
		pPESPacket += nRead;
		if (v->nPictureIndex[nChartIndex] != -1)
			v->nPictureDataCount[nChartIndex][v->nPictureIndex[nChartIndex]] += nRead;

		if (Fvd_Dec_Has_Pending_Frames(H264Dec))
		{
			FVD_PIC Pic;
			Fvd_Dec_Consume_Frame(H264Dec, &Pic);

			if (v->nPictureIndex[nChartIndex] == -1)
			{
				// First decoded frame
				v->nPictureIndex[nChartIndex] = 0;
			}
			else
			{
				v->nPictureType[nChartIndex][v->nPictureIndex[nChartIndex]] = Pic.Coding;
				UpdateVideoCompositionChart(v->nPictureIndex[nChartIndex], nChartIndex, TRUE);

				v->nPictureIndex[nChartIndex]++;
				if (v->nPictureIndex[nChartIndex] == nMaxPoints - 1)
				{
					int i;

					for (i = 0; i < nMaxPoints - 2; i++)
					{
						v->nPictureDataCount[nChartIndex][i] = v->nPictureDataCount[nChartIndex][i + 1];
						v->nPictureType[nChartIndex][i] = v->nPictureType[nChartIndex][i + 1];
					}
					v->nPictureIndex[nChartIndex] = nMaxPoints - 2;
					v->nPictureDataCount[nChartIndex][v->nPictureIndex[nChartIndex]] = 0;
					v->nPictureType[nChartIndex][v->nPictureIndex[nChartIndex]] = 0;
				}
			}
		}
	}
	LeaveCriticalSection(&v->csH264VideoChart);


/*	int nCurrentPos = 0;
	BOOL fFoundPictureStart = FALSE;

	while (nCurrentPos < nPESLength - 4)
	{
		if (   pPESPacket[nCurrentPos + 0] == 0x00
			&& pPESPacket[nCurrentPos + 1] == 0x00
			&& pPESPacket[nCurrentPos + 2] == 0x01)
		{
			switch (pPESPacket[nCurrentPos + 3] & 0x1f)
			{
			case 1:		// Coded slice of a non-IDR picture
			case 5:		// Coded slice of an IDR picture
			case 19:	// Coded slice of an auxiliary coded picture without partitioning
				{
					uint32_t first_mb_in_slice;
					uint32_t slice_type;
					//uint32_t pic_parameter_set_id;
					//uint32_t frame_num;
					bs_t b;
					
					memset(&b, 0, sizeof(b));
					bs_init(&b, &pPESPacket[nCurrentPos + 4], nPESLength - (nCurrentPos + 4));
					
					first_mb_in_slice = bs_read_ue(&b);
					slice_type = bs_read_ue(&b);
					if (slice_type > 4)
						slice_type -= 5;

					//pic_parameter_set_id = bs_read_ue(&b);
					//frame_num = bs_read_u(&b, sps->log2_max_frame_num_minus4 + 4 ); // was u(v)

					switch(slice_type)
					{
					case SH_SLICE_TYPE_I:
						if (v->nPictureIndex[nChartIndex] != -1)
							UpdateVideoCompositionChart(v->nPictureIndex[nChartIndex], nChartIndex, TRUE);
						v->nPictureIndex[nChartIndex] = 0;
						OutputDebugString("I");
						break;
					case SH_SLICE_TYPE_P:
						if (v->nPictureIndex[nChartIndex] != -1)
							v->nPictureIndex[nChartIndex]++;
						OutputDebugString("P");
						break;
					case SH_SLICE_TYPE_B:
						if (v->nPictureIndex[nChartIndex] != -1)
							v->nPictureIndex[nChartIndex]++;
						OutputDebugString("B");
						break;
					case SH_SLICE_TYPE_SP:
						if (v->nPictureIndex[nChartIndex] != -1)
							v->nPictureIndex[nChartIndex]++;
						OutputDebugString("Sp");
						break;
					case SH_SLICE_TYPE_SI:
						if (v->nPictureIndex[nChartIndex] != -1)
							v->nPictureIndex[nChartIndex]++;
						OutputDebugString("Si");
						break;
					}
					if (v->nPictureIndex[nChartIndex] != -1)
					{
						v->nPictureType[nChartIndex][v->nPictureIndex[nChartIndex]] = slice_type;
						v->nPictureDataCount[nChartIndex][v->nPictureIndex[nChartIndex]] += nPESLength - nCurrentPos;
					}
					fFoundPictureStart = TRUE;
				}
				break;
			}
		}
		nCurrentPos++;
	}

	if (!fFoundPictureStart)
	{
		if (v->nPictureIndex[nChartIndex] != -1)
			v->nPictureDataCount[nChartIndex][v->nPictureIndex[nChartIndex]] += nPESLength;
	}
	*/
}
#endif PRO_BROKEN

void InputMPEG2VideoCompositionESData(BYTE * pPESPacket, int nPESLength, int nChartIndex)
{
	int nCurrentPos = 0;
	BOOL fFoundPictureStart = FALSE;

	while (nCurrentPos < nPESLength - 4)
	{
		if (   pPESPacket[nCurrentPos + 0] == 0x00
			&& pPESPacket[nCurrentPos + 1] == 0x00
			&& pPESPacket[nCurrentPos + 2] == 0x01)
		{
			switch (pPESPacket[nCurrentPos + 3])
			{
			case 0x00:	// picture start code
				if (v->nPictureIndex[nChartIndex] != -1)
					v->nPictureDataCount[nChartIndex][v->nPictureIndex[nChartIndex]] += nCurrentPos;
				set_buf(BM_PARSER_THREAD, &pPESPacket[nCurrentPos + 4], 0, FALSE);
				{
					int temporal_reference = get_bits(BM_PARSER_THREAD, 10);
					int picture_coding_type = get_bits(BM_PARSER_THREAD, 3);

					switch(picture_coding_type)
					{
					case 0:		// forbidden
					default:	// reserved
						break;
					case 1:		// I
						if (v->nPictureIndex[nChartIndex] != -1)
							UpdateVideoCompositionChart(v->nPictureIndex[nChartIndex], nChartIndex, FALSE);
						v->nPictureIndex[nChartIndex] = 0;
						break;
					case 2:		// P
						if (v->nPictureIndex[nChartIndex] != -1)
							v->nPictureIndex[nChartIndex]++;
						break;
					case 3:		// B
						if (v->nPictureIndex[nChartIndex] != -1)
							v->nPictureIndex[nChartIndex]++;
						break;
					case 4:		// D
						if (v->nPictureIndex[nChartIndex] != -1)
							v->nPictureIndex[nChartIndex]++;
						break;
					}
					if (v->nPictureIndex[nChartIndex] != -1)
					{
						v->nPictureType[nChartIndex][v->nPictureIndex[nChartIndex]] = picture_coding_type;
						v->nPictureDataCount[nChartIndex][v->nPictureIndex[nChartIndex]] += nPESLength - nCurrentPos;
					}
					fFoundPictureStart = TRUE;
				}
				break;
			}
		}
		nCurrentPos++;
	}

	if (!fFoundPictureStart)
	{
		if (v->nPictureIndex[nChartIndex] != -1)
			v->nPictureDataCount[nChartIndex][v->nPictureIndex[nChartIndex]] += nPESLength;
	}
}

typedef struct _tagPIDUsageStacked
{
	int nPIDList[8192];
	int nMaxRate;
	int nPointCount;
} PIDUSAGESTACKED, *PPIDUSAGESTACKED;

PPIDUSAGESTACKED pidusagestacked;

LRESULT FAR PASCAL PIDUsageVBRStackedAreaWindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg)
	{
	case WM_CREATE:
		{
			LPCREATESTRUCT lpcs = (LPCREATESTRUCT)lParam;
			int nChartIndex = *((int*)lpcs->lpCreateParams);
			double arg;
			RECT rect;

			SendMessage(hWnd, WM_SETICON, ICON_BIG, (LPARAM)v->hDialogIcon);
			v->fSetPoints[nChartIndex] = FALSE;
			rate.QuadPart = priorcount.QuadPart = 0;
			dMaxRate = 0.0;

			GetClientRect(hWnd, &rect);
			v->m_hPE[nChartIndex] = PEcreate(PECONTROL_GRAPH, WS_VISIBLE, &rect, hWnd, 1001);
			PEnset(v->m_hPE[nChartIndex], PEP_nPOINTS, v->nGraphHistoricalPoints);

			PEnset(v->m_hPE[nChartIndex], PEP_bNOSTACKEDDATA, FALSE);
			PEnset(v->m_hPE[nChartIndex], PEP_nPLOTTINGMETHOD, PEGPM_AREASTACKED);
			PEnset(v->m_hPE[nChartIndex], PEP_nDATASHADOWS, PEDS_NONE);	
			PEnset(v->m_hPE[nChartIndex], PEP_bGRIDINFRONT, TRUE);
			PEszset(v->m_hPE[nChartIndex], PEP_szMAINTITLE, "PID Usage Area Chart");

			SetupChartSubtitle(nChartIndex);
			PEnset(v->m_hPE[nChartIndex], PEP_bNORANDOMPOINTSTOEXPORT, TRUE);
			PEnset(v->m_hPE[nChartIndex], PEP_bFOCALRECT, FALSE);
			PEnset(v->m_hPE[nChartIndex], PEP_bALLOWBAR, FALSE);
			PEnset(v->m_hPE[nChartIndex], PEP_bALLOWPOPUP, FALSE);
			PEnset(v->m_hPE[nChartIndex], PEP_bPREPAREIMAGES, TRUE);
			PEnset(v->m_hPE[nChartIndex], PEP_bCACHEBMP, TRUE);
			PEnset(v->m_hPE[nChartIndex], PEP_bFIXEDFONTS, TRUE);

			arg = 0.1f;
			PEvset(v->m_hPE[nChartIndex], PEP_fMANUALMINY, &arg, 1);
			PEnset(v->m_hPE[nChartIndex], PEP_nGRADIENTBARS, 8);
			PEnset(v->m_hPE[nChartIndex], PEP_bMAINTITLEBOLD, TRUE);
			PEnset(v->m_hPE[nChartIndex], PEP_bSUBTITLEBOLD, TRUE);
			PEnset(v->m_hPE[nChartIndex], PEP_bLABELBOLD, TRUE);
			PEnset(v->m_hPE[nChartIndex], PEP_bLINESHADOWS, TRUE);
			PEnset(v->m_hPE[nChartIndex], PEP_nFONTSIZE, PEFS_LARGE);
			Charting__UpdateQuickStyle(nChartIndex);

			PEszset(v->m_hPE[nChartIndex], PEP_szaPOINTLABELS, "");
			PEszset(v->m_hPE[nChartIndex], PEP_szXAXISLABEL, "Time");
			PEszset(v->m_hPE[nChartIndex], PEP_szYAXISLABEL, "Bitrate");

			PEnset(v->m_hPE[nChartIndex], PEP_bSUBSETBYPOINT, TRUE);
			
			PEreinitialize(v->m_hPE[nChartIndex]);
			PEresetimage(v->m_hPE[nChartIndex], 0, 0);

			SendMessage(hWnd, WM_SETTEXT, 0, (LPARAM)"PID Usage Area Chart");
			CheckCommonGraphMessages(hWnd, uMsg, wParam, lParam);

			pidusagestacked = (PPIDUSAGESTACKED)LocalAlloc(LPTR, sizeof(PIDUSAGESTACKED));
			{
				int i;
				for (i = 0; i < 8192; i++)
					pidusagestacked->nPIDList[i] = -1;
			}
			v->nPIDUsageStackedAreaChartIndex = nChartIndex;
		}
		break;
	case WM_DESTROY:
		v->nPIDUsageStackedAreaChartIndex = -1;
		LocalFree(pidusagestacked);
		// note - no break
	default:
		if (CheckCommonGraphMessages(hWnd, uMsg, wParam, lParam) == TRUE)
			break;
		return DefWindowProc(hWnd, uMsg, wParam, lParam);
	}

	return FALSE;
}

void AddDataToPIDUsageStackedChart(int nChartIndex)
{
	int nPIDIndex;
	LARGE_INTEGER count;
	BOOL fAddedNewPID = FALSE;
	BOOL fResetMax = FALSE;
	SYSTEMTIME stLocal;
	char szTemp[1024];
	//static float fAverage = 0;
	//static int nAveragePoints = 0;

	if (rate.QuadPart == 0)
	{
		QueryPerformanceFrequency(&rate);
		QueryPerformanceCounter(&priorcount);
		return;
	}
	QueryPerformanceCounter(&count);

	// See if we have a PID we've never seen before - needs new subset
	for (nPIDIndex = 0; nPIDIndex < 8192; nPIDIndex++)
	{	
		if (v->pc[nPIDIndex].lnPackets)
		{
			int i;

			for (i = 0; i < 8192; i++)
			{
				if (pidusagestacked->nPIDList[i] == -1)
				{
					pidusagestacked->nPIDList[i] = v->pc[nPIDIndex].nPID;
					fAddedNewPID = TRUE;
					break;
				}
				if (pidusagestacked->nPIDList[i] == v->pc[nPIDIndex].nPID)
					break;
			}
		}
	}

	if (fAddedNewPID)
	{
		int i;
		int nPointCount = 0;
		char szPointLabels[10 * 1024] = {""};

		for (i = 0; i < 8192; i++)
		{
			if (pidusagestacked->nPIDList[i] == -1)
				break;
			wsprintf(szTemp, "PID %04x\t", pidusagestacked->nPIDList[i]);
			lstrcat(szPointLabels, szTemp);
			nPointCount++;
		}
		szPointLabels[lstrlen(szPointLabels) - 1] = '\0';
		PEnset(v->m_hPE[nChartIndex], PEP_nSUBSETS, nPointCount);
		PEvset(v->m_hPE[nChartIndex], PEP_szaSUBSETLABELS, szPointLabels, nPointCount);
		PEreinitialize(v->m_hPE[nChartIndex]);
		pidusagestacked->nPointCount = nPointCount;
	}

	if (pidusagestacked->nPointCount)
	{
		int i;
		float fTotal = 0;
		float * fNewValues = (float *)LocalAlloc(LPTR, sizeof(float) * pidusagestacked->nPointCount);

		GetLocalTime(&stLocal);
		wsprintf(szTemp, "%02d:%02d", stLocal.wHour, stLocal.wMinute);
		PEvset(v->m_hPE[nChartIndex], PEP_szaAPPENDPOINTLABELDATA, szTemp, 1);

		for (i = 0; i < pidusagestacked->nPointCount; i++)
		{
			for (nPIDIndex = 0; nPIDIndex < 8192; nPIDIndex++)
			{
				if (pidusagestacked->nPIDList[i] == v->pc[nPIDIndex].nPID)
				{
					__int64 lnBytes = (v->pc[nPIDIndex].lnPackets * 188 * 8);
					float fDifference = (1.0f/(float)rate.QuadPart * ((float)count.QuadPart - (float)priorcount.QuadPart));
					fNewValues[i] = lnBytes * (1.0f / (1.0f/(float)rate.QuadPart * ((float)count.QuadPart - (float)priorcount.QuadPart)));				
					if (fNewValues[i] == 0.0f)
						fNewValues[i] = 0.000000001f;
#ifdef _DEBUG
					sprintf(szTemp, "PID %04x Rate = %f Difference = %f\n", 
						pidusagestacked->nPIDList[i],
						fNewValues[i],
						fDifference);
					OutputDebugString(szTemp);
#endif _DEBUG
					fTotal += fNewValues[i];						    
					break;
				}
			}
		}
		if (fTotal > (float)dMaxRate)
		{
			double dTemp;
			dMaxRate = (double)fTotal;
			dTemp = dMaxRate + (dMaxRate / 10.0);
			PEvset(v->m_hPE[nChartIndex], PEP_fMANUALMAXY, &dTemp, 1);
			fResetMax = TRUE;
		}
		//fAverage += fTotal;
		//nAveragePoints++;
		//sprintf(szTemp, "-------------Total: %f Average: %f\n", fTotal, fAverage / (float)nAveragePoints);
		//OutputDebugString(szTemp);

		PEvset(v->m_hPE[nChartIndex], PEP_faAPPENDYDATA, fNewValues, 1);
		LocalFree(fNewValues);
	}

	if (fAddedNewPID || fResetMax)
	{
		PEreinitialize(v->m_hPE[nChartIndex]);
		PEresetimage(v->m_hPE[nChartIndex], 0, 0);
		InvalidateRect(v->hWndChart[nChartIndex], NULL, FALSE);
	}

	priorcount.QuadPart = count.QuadPart;
}
