#ifndef LITE
#include <windows.h>
#include <commctrl.h>
#include <time.h>
#include "TSReader.h"
#include "util.h"
#include "formatter.h"
#include "resource.h"

// Forward declarations - should be in TSReader.h
int GetTotalPMTChannels();
BOOL IsAC3AudioStream(int nPMTProgramIndex, int nESIndex);
BOOL IsPCMAudioStream(int nPMTProgramIndex, int nESIndex);
void GetLanguageFromDescriptor(char * szLanguage, int nPMTIndex, int nESIndex);
void XMLExport(HWND hDlg, HANDLE hXMLFile);
void XMLTVExport(HWND hDlg, HANDLE hXMLFile);
void HTMLExport(HANDLE hHTMFile, int nExportSITables, char * szOutputFilename);
void RestartTSReader_Stop(HWND hDlg);
void RestartTSReader_Start(HWND hDlg);
BOOL LoadSource(HWND hWnd);
void CloseExistingChart(HWND hDlg, DWORD dwMenuID);
void GetSourceInfoLine(int nLine, char * szOutput);
int __cdecl SortPIDsByPID(const void *elem1, const void *elem2);
BOOL GetPIDTooltipInfo(int nPID, char * szString);
void GetNextECMPID();
HTREEITEM AddItemToSITree(HWND hwndTV, LPTSTR lpszItem, int nLevel, LPARAM lParam, int nIconIndex, HTREEITEM hParent, HTREEITEM hInsertAfter);

// Global variables
extern PVARIABLES v;
extern int  (* GetSyncLossCount) (BOOL fReset);
extern int  (* GetRetuneCount) (BOOL fReset);

// Should move into v->
SOCKET ControlBaseSocket = INVALID_SOCKET;
SOCKET ControlSocket = INVALID_SOCKET;
BOOL fServerThreadActive;
char szNewSourceName[MAX_PATH] = {0};
#ifdef PRO
BOOL fXMLStreamRunning = FALSE;
BOOL fKillXMLStreamThread = FALSE;
BOOL fDataRequest = FALSE;
int nDataRequestlParam = 0;
char szCurrentlyDoingTerminator[128] = {""};
#endif PRO

static char szError506[] = {"506 Already playing or recording\r\n"};
static char szError507[] = {"507 No program selected\r\n"};
static char szOK304[] =    {"304 Starting record\r\n"};

char szBaseHelp[] = {
		"201 TSReader Control Socket Help\r\n"
		"201 \r\n"
		"201 AUDIO\t\tReturns/sets the audio stream to use\r\n"
		"201 EXPORT\t\tExports SI tables\r\n"
		"201 GRAPH\t\tDisplays a graph\r\n"
		"201 HELP\t\tThis option\r\n"
		"201 INFO\t\tGet info on source or mux\r\n"
		"201 MANUALCHANNEL\tAdds a manual channel\r\n"
		"201 PIDS\t\tLists PIDs in the mux\r\n"
		"201 PLAY\t\tInitiates stream playback\r\n"
		"201 PROGRAM\t\tSelects the current program\r\n"
		"201 QUIT\t\tTerminates the control server connection\r\n"
		"201 RECORD\t\tRecords streams\r\n"
		"201 SETVLC\t\tSets up VLC command strings\r\n"
		"201 SETTING\t\tChanges TSReader's settings\r\n"
		"201 SOURCE\t\tSpecifies the source\r\n"
		"201 STALL\t\tWaits for SI parsing to complete\r\n"
		"201 STOP\t\tStops recording or playback\r\n"
		"201 TERMINATE\t\tTerminates TSReader\r\n"
		"201 THUMBNAIL\t\tWrites a specified thumbnail\r\n"
		"201 TUNE\t\tRetunes TSReader\r\n"
		"201 WINDOW\t\tAlters the TSReader window\r\n"
		"201\r\n"
		"201 Type HELP command for command parameters\r\n"
		"201\r\n"
		"201 Full documentation is at:\r\n"
		"201 https://tsreader.co.uk"
};

static char szAudioHelp[] = {
		"201 AUDIO help\r\n"
};

static char szExportHelp[] = {
		"201 EXPORT help\r\n"
};

static char szGraphHelp[] = {
		"201 GRAPH help\r\n"
};

static char szHelpHelp[] = {
		"201 HELP help\r\n"
};

static char szInfoHelp[] = {
		"201 INFO help\r\n"
};

static char szInfoManualChannel[] = {
		"201 MANUALCHANNEL help\r\n"
};

static char szPlayHelp[] = {
		"201 PLAY help\r\n"
};

static char szQuitHelp[] = {
		"201 QUIT help\r\n"
};

static char szRecordHelp[] = {
		"201 RECORD help\r\n"
};

static char szPIDsHelp[] = {
		"201 PIDS help\r\n"
};

static char szProgramHelp[] = {
		"201 PROGRAM help\r\n"
};

static char szSetVLCHelp[] = {
		"201 SETVLC help\r\n"
};

static char szSetttingHelp[] = {
		"201 SETTING help\r\n"
};

static char szSourceHelp[] = {
		"201 SOURCE help\r\n"
};

static char szStallHelp[] = {
		"201 STALL help\r\n"
};

static char szStopHelp[] = {
		"201 STOP help\r\n"
};

static char szTerminateHelp[] = {
		"201 TERMINATE help\r\n"
};

static char szTuneHelp[] = {
		"201 TUNE help\r\n"
};

static char szThumbnailHelp[] = {
		"201 THUMBNAIL help\r\n"
};

static char szWindowHelp[] = {
		"201 WINDOW help\r\n"
};

int SendControlResponse(char * szMessage, int nLength)
{
	int nRetVal;

	nRetVal = send(ControlSocket, szMessage, nLength, 0);
#ifdef DEBUG_MESSAGES
	{
		char szTemp[10 * 1024];

		wsprintf(szTemp, "TSReader: Control server response: %s", szMessage);
		if (szMessage[lstrlen(szMessage) - 1] != '\n')
			lstrcat(szTemp, "\n");
		OutputDebugString(szTemp);
	}
#endif DEBUG_MESSAGES

	return nRetVal;
}

void CS__SendHelp(char * szSpacePtr)
{
	int nOffset;
	int nRemainder;
	char * szHelp = NULL;
	static char szError[] = {"527 HELP command parameters are incorrect\r\n"};

	if (szSpacePtr == NULL)
		szHelp = szBaseHelp;
	else
	{
		strupr(szSpacePtr);
		if (strcmp(szSpacePtr, "AUDIO") == 0)
			szHelp = szAudioHelp;
		else if (strcmp(szSpacePtr, "EXPORT") == 0)
			szHelp = szExportHelp;
		else if (strcmp(szSpacePtr, "GRAPH") == 0)
			szHelp = szGraphHelp;
		else if (strcmp(szSpacePtr, "HELP") == 0)
			szHelp = szHelpHelp;
		else if (strcmp(szSpacePtr, "INFO") == 0)
			szHelp = szInfoHelp;
		else if (strcmp(szSpacePtr, "MANUALCHANNEL") == 0)
			szHelp = szInfoManualChannel;
		else if (strcmp(szSpacePtr, "PLAY") == 0)
			szHelp = szPlayHelp;
		else if (strcmp(szSpacePtr, "QUIT") == 0)
			szHelp = szQuitHelp;
		else if (strcmp(szSpacePtr, "RECORD") == 0)
			szHelp = szRecordHelp;
		else if (strcmp(szSpacePtr, "PIDS") == 0)
			szHelp = szPIDsHelp;
		else if (strcmp(szSpacePtr, "PROGRAM") == 0)
			szHelp = szProgramHelp;
		else if (strcmp(szSpacePtr, "SETVLC") == 0)
			szHelp = szSetVLCHelp;
		else if (strcmp(szSpacePtr, "SETTING") == 0)
			szHelp = szSetttingHelp;	
		else if (strcmp(szSpacePtr, "SOURCE") == 0)
			szHelp = szSourceHelp;
		else if (strcmp(szSpacePtr, "STALL") == 0)
			szHelp = szStallHelp;
		else if (strcmp(szSpacePtr, "STOP") == 0)
			szHelp = szStopHelp;
		else if (strcmp(szSpacePtr, "TERMINATE") == 0)
			szHelp = szTerminateHelp;
		else if (strcmp(szSpacePtr, "TUNE") == 0)
			szHelp = szTuneHelp;
		else if (strcmp(szSpacePtr, "THUMBNAIL") == 0)
			szHelp = szThumbnailHelp;
		else if (strcmp(szSpacePtr, "WINDOW") == 0)
			szHelp = szWindowHelp;
	}

	if (szHelp == NULL)
		szHelp = szError;

	nRemainder = lstrlen(szHelp);
	for (nOffset = 0; nOffset < lstrlen(szHelp); nOffset += 100)
	{
		int nThisTime = nRemainder;
		if (nThisTime > 100)
			nThisTime = 100;
		SendControlResponse(&szHelp[nOffset], nThisTime);
		nRemainder -= nThisTime;
	}
}

void CS__Program(char * szSpacePtr)
{
	if (szSpacePtr == NULL)
	{
		// Report programs
		int nPMTIndex;
		static char szProgramComplete[] = {"314 PROGRAM list complete\r\n"};

		for (nPMTIndex = 0; nPMTIndex < MAX_PAT_ENTRIES; nPMTIndex++)
		{
			char szResponse[128];

			if (v->pat.pmt[nPMTIndex].nPMTPID == 0)
				break;
			if (v->pat.pmt[nPMTIndex].nProgramNumber == 0)
				continue;
			if (v->pat.pmt[nPMTIndex].fSetupSDTName && v->pChannelData[v->pat.pmt[nPMTIndex].nProgramNumber] != NULL)
				wsprintf(szResponse, "202 %5d %s", v->pat.pmt[nPMTIndex].nProgramNumber, v->pChannelData[v->pat.pmt[nPMTIndex].nProgramNumber]->szShortName);
			else
				wsprintf(szResponse, "202 %5d", v->pat.pmt[nPMTIndex].nProgramNumber);
			if (nPMTIndex == v->nSelectedProgram)
				lstrcat(szResponse, " *");
			lstrcat(szResponse, "\r\n");
			SendControlResponse(szResponse, lstrlen(szResponse));
		}

		SendControlResponse(szProgramComplete, lstrlen(szProgramComplete));
	}
	else
	{
		// Set program
		int nSelectProgramNumber;
		int nPMTIndex;

		sscanf(szSpacePtr, "%d", &nSelectProgramNumber);
		if (nSelectProgramNumber <= 0 || nSelectProgramNumber > 65535)
		{
			static char szError[] = {"501 Invalid program number\r\n"};
			SendControlResponse(szError, lstrlen(szError));
			return;
		}
		for (nPMTIndex = 0; nPMTIndex < MAX_PAT_ENTRIES; nPMTIndex++)
		{
			if (v->pat.pmt[nPMTIndex].nPMTPID == 0)
				break;
			if (v->pat.pmt[nPMTIndex].nProgramNumber == nSelectProgramNumber)
			{
				HWND hWndTV = GetDlgItem(v->hDlgSIParser, IDC_SI_TREE);

				TreeView_SelectItem(hWndTV, v->pat.pmt[nPMTIndex].hPMTTreeItem);
				TreeView_SelectSetFirstVisible(hWndTV, v->pat.pmt[nPMTIndex].hPMTTreeItem);				
				{
					static char szOK[] = {"300 Program selected\r\n"};
					SendControlResponse(szOK, lstrlen(szOK));
				}
				return;
			}
		}
		{
			char szError[] = {"502 Program not found\r\n"};
			SendControlResponse(szError, lstrlen(szError));
		}
	}
}

void CS__TerminateTSReader(char * szParameters)
{
	if (szParameters == NULL)
	{
		static char szError[] = {"503 Terminate command not complete\r\n"};
		SendControlResponse(szError, lstrlen(szError));
		return;
	}
	if (lstrcmp(szParameters, "xyzzy") != 0)
	{
		static char szError[] = {"504 Terminate sequence incorrect\r\n"};
		SendControlResponse(szError, lstrlen(szError));
		return;
	}
	{
		static char szOK[] = {"301 Terminating\r\n"};
		SendControlResponse(szOK, lstrlen(szOK));
	}
	v->fDirtyManualChannels = FALSE;
	PostMessage(v->hWndMainWindow, WM_CLOSE, 0, 0);
}

void CS__PIDs(char * szParameters)
{
	int i;
	double dPercent, dRate = 0.0;
	PIDCOUNTER pc[8192];
	static char szComplete[] = {"317 PID list complete\r\n"};

	EnterCriticalSection(&v->ss.csPIDCounter);
	memcpy(&pc, &v->pc, sizeof(PIDCOUNTER) * 8192);
	LeaveCriticalSection(&v->ss.csPIDCounter);
	qsort(&pc, 8192, sizeof(PIDCOUNTER), SortPIDsByPID);

	for (i = 0; i < 8192; i++)
	{
		if (pc[i].lnPackets)
		{
			char * szPIDFirstSpace;
			char szResponse[512];
			char szPIDUsage[256] = {0};

			GetPIDTooltipInfo(pc[i].nPID, szPIDUsage);
			szPIDFirstSpace = strstr(szPIDUsage, " ") + 1;

			dPercent = ((double)pc[i].lnPackets / (double)v->lnCopyTotalTSPackets) * 100.0;
			if (pc[i].dPIDRate)
				dRate = pc[i].dPIDRate * 8.0 / 1000.0 / 1000.0;
			else
			{
				if (v->dDisplayMuxRate)
				{
					dRate = (v->dDisplayMuxRate / (double)v->nMuxRateCounter) * 8.0;
					dRate = ((dRate / 100.0) * dPercent) / 1000.0 / 1000.0;
				}
			}

			sprintf(szResponse, "209 0x%04x %d %d %.2f %.3f \"%s\"\r\n",
				    pc[i].nPID,
					pc[i].nPIDHasContinuityErrors,
					pc[i].nPIDTEICount,
					dPercent,
					dRate,
					szPIDFirstSpace);
			SendControlResponse(szResponse, lstrlen(szResponse));
		}
	}
	SendControlResponse(szComplete, lstrlen(szComplete));
}

void CS__Play(char * szParameters)
{
	BOOL fResult = FALSE;

	if (szParameters == NULL)
	{
		static char szError[] = {"505 Play command not complete\r\n"};
		SendControlResponse(szError, lstrlen(szError));
		return;
	}
	if (v->fRecording == TRUE)
	{
		SendControlResponse(szError506, lstrlen(szError506));
		return;
	}
	if (v->nSelectedProgram == -1)
	{
		SendControlResponse(szError507, lstrlen(szError507));
		return;
	}
	
	strupr(szParameters);
	if (lstrcmp(szParameters, "ROKU") == 0)
	{
		v->nAutoRecord = AUTO_RECORD_VLC;
		v->fAutoRecordFromControlServer = TRUE;
		PostMessage(v->hWndMainWindow, WM_COMMAND, ID_PLAYBACK_ROKUHD1000, 0);
		fResult = TRUE;
	}
	else if (lstrcmp(szParameters, "STRADIS") == 0)
	{
		v->nAutoRecord = AUTO_RECORD_VLC;
		v->fAutoRecordFromControlServer = TRUE;
		PostMessage(v->hWndMainWindow, WM_COMMAND, IDC_SI_PARSER_TO_STRADIS, 0);
		fResult = TRUE;
	}
	else if (lstrcmp(szParameters, "XNS") == 0)
	{
		v->nAutoRecord = AUTO_RECORD_VLC;
		v->fAutoRecordFromControlServer = TRUE;
		PostMessage(v->hWndMainWindow, WM_COMMAND, ID_PLAYBACK_XNSSERVER, 0);
		fResult = TRUE;
	}
	else if (szParameters[0] == 'V' && szParameters[1] == 'L' && szParameters[2] == 'C')
	{
		int nVLCConfiguration;

		sscanf(&szParameters[3], "%d", &nVLCConfiguration);
		if (nVLCConfiguration >= 1 && nVLCConfiguration <= 16)
		{
			v->nAutoRecord = AUTO_RECORD_VLC;
			v->fAutoRecordFromControlServer = TRUE;
			PostMessage(v->hWndMainWindow, WM_COMMAND, ID_PLAYBACK_VLC_1 + (nVLCConfiguration - 1), 0);
			fResult = TRUE;
		}
	}
	if (fResult == FALSE)
	{
		static char szError[] = {"508 Unknown playback device specified\r\n"};
		SendControlResponse(szError, lstrlen(szError));
		return;
	}
	{
		static char szOK[] = {"302 Playback starting\r\n"};
		SendControlResponse(szOK, lstrlen(szOK));
	}
}

void CS__Stop(char * szParameters)
{
	if (v->fRecording == FALSE)
	{
		static char szError[] = {"509 Not currently recording or playing\r\n"};
		SendControlResponse(szError, lstrlen(szError));
		return;
	}
	
	switch(v->nStreamTo)		
	{
	case 0:
		if (v->fRecordAllTS == TRUE)
			PostMessage(v->hWndMainWindow, WM_COMMAND, IDC_SI_PARSER_RECORD_ALL, 0);
		else
			PostMessage(v->hWndMainWindow, WM_COMMAND, IDC_SI_PARSER_RECORD, 0);
		break;
	case STREAM_TO_STRADIS:
		PostMessage(v->hWndMainWindow, WM_COMMAND, IDC_SI_PARSER_TO_STRADIS, 0);
		break;
	case STREAM_TO_XNS:
		PostMessage(v->hWndMainWindow, WM_COMMAND, ID_PLAYBACK_XNSSERVER, 0);
		break;
	case STREAM_TO_DVHS:
		PostMessage(v->hWndMainWindow, WM_COMMAND, ID_RECORD_RECORDPROGRAMTODVHS, 0);
		break;
	case STREAM_TO_VLC:
		PostMessage(v->hWndMainWindow, WM_COMMAND, ID_PLAYBACK_VLC_1, 0);
		break;
	case STREAM_TO_ROKU:
		PostMessage(v->hWndMainWindow, WM_COMMAND, ID_PLAYBACK_ROKUHD1000, 0);
		break;
	}
	v->nAutoRecord = 0;
	{
		static char szOK[] = {"303 Stopping\r\n"};
		SendControlResponse(szOK, lstrlen(szOK));
	}
}


void CS__Reset(char * szParameters)
{
	if (szParameters == NULL)
	{
		static char szError[] = {"5xx No reset value specified\r\n"};
		SendControlResponse(szError, lstrlen(szError));
	}
	else
	{
		int nOptions = atoi(szParameters);
		
		if (nOptions & 0x01)
			PostMessage(v->hWndMainWindow, WM_COMMAND, ID_HELP_RESETCONTINUITYCOUNTERS, 0);
		if (nOptions & 0x02)
			PostMessage(v->hWndMainWindow, WM_COMMAND, ID_HELP_RESETTEICOUNTERS, 0);
		if (nOptions & 0x04)
			PostMessage(v->hWndMainWindow, WM_COMMAND, ID_HELP_RESETCRCCOUNTERS, 0);
		if (nOptions & 0x08)
			PostMessage(v->hWndMainWindow, WM_COMMAND, ID_HELP_RESETSECTIONCOUNTERS, 0);
		if (nOptions & 0x10)
			PostMessage(v->hWndMainWindow, WM_COMMAND, ID_HELP_RESETPIDCHART, 0);
		{
			char szOK[] = {"325 Specified counters reset\r\n"};
			SendControlResponse(szOK, lstrlen(szOK));
		}
	}
}

void CS__Record(char * szParameters)
{
	if (szParameters == NULL)
	{
		if (v->fRecording == FALSE)
		{
			static char szError[] = {"517 Not currently recording\r\n"};
			SendControlResponse(szError, lstrlen(szError));
		}
		else
		{
			if (v->fRecordAllTS == TRUE)
			{
				// record all
				char szOK[128];
				sprintf(szOK, "207 Recording ALL to \"%s\" %.3f GB recorded\r\n", v->szRecordFile, v->dTotalRecorded / 1024.0 / 1024.0 / 1024.0);
				SendControlResponse(szOK, lstrlen(szOK));
			}
			else
			{
				if (v->fStradisActive == TRUE)
				{
					char szOK[128];

					switch(v->nStreamTo)
					{
					case STREAM_TO_STRADIS:
						lstrcpy(szOK, "207 Streaming to STRADIS\r\n");
						break;
					case STREAM_TO_DIRECTSHOW:
						lstrcpy(szOK, "207 Streaming to DIRECTSHOW\r\n");
						break;
					case STREAM_TO_XNS:
						lstrcpy(szOK, "207 Streaming to XNS\r\n");
						break;
					case STREAM_TO_DVHS:
						sprintf(szOK, "207 Recording to D-VHS. %.3f GB recorded\r\n", v->dTotalRecorded / 1024.0 / 1024.0 / 1024.0);
						break;
					case STREAM_TO_VLC:
						lstrcpy(szOK, "207 Streaming to VLC\r\n");
						break;
					case STREAM_TO_ROKU:
						lstrcpy(szOK, "207 Streaming to ROKU\r\n");
						break;
					default:
						lstrcpy(szOK, "207 Streaming to unknown destination\r\n");
						break;
					}
					SendControlResponse(szOK, lstrlen(szOK));
				}
				else
				{
					char szOK[128];
					sprintf(szOK, "207 Recording PROGRAM to \"%s\" %.3f GB recorded\r\n", v->szRecordFile, v->dTotalRecorded / 1024.0 / 1024.0 / 1024.0);
					SendControlResponse(szOK, lstrlen(szOK));
				}
			}
		}
	}
	else
	{
		char * szSpace = strstr(szParameters, " ");

		if (v->fRecording == TRUE)
		{
			SendControlResponse(szError506, lstrlen(szError506));
			return;
		}

		if (szSpace != NULL)
			*szSpace = '\0';
		strupr(szParameters);
		if (lstrcmp(szParameters, "D-VHS") == 0)
		{
			if (v->nSelectedProgram == -1)
			{
				SendControlResponse(szError507, lstrlen(szError507));
				return;
			}
			lstrcpy(v->szAutoRecordFile, szSpace + 1);
			v->nAutoRecord = AUTO_RECORD_VLC;
			v->fAutoRecordFromControlServer = TRUE;
			PostMessage(v->hWndMainWindow, WM_COMMAND, ID_RECORD_RECORDPROGRAMTODVHS, 0);
			SendControlResponse(szOK304, lstrlen(szOK304));
		}
		if (szSpace == NULL)
		{
			char szError[] = {"510 Record filename missing\r\n"};
			SendControlResponse(szError, lstrlen(szError));
			return;
		}
		if (lstrcmp(szParameters, "ALL") == 0)
		{
			lstrcpy(v->szAutoRecordFile, szSpace + 1);
			v->nAutoRecord = AUTO_RECORD_ALL;
			v->fAutoRecordFromControlServer = TRUE;
			PostMessage(v->hWndMainWindow, WM_COMMAND, IDC_SI_PARSER_RECORD_ALL, 0);
			SendControlResponse(szOK304, lstrlen(szOK304));
		}
		else if (lstrcmp(szParameters, "PROGRAM") == 0)
		{
			if (v->nSelectedProgram == -1)
			{
				SendControlResponse(szError507, lstrlen(szError507));
				return;
			}
			lstrcpy(v->szRecordFile, szSpace + 1);
			v->nAutoRecord = AUTO_RECORD_VLC;
			v->fAutoRecordFromControlServer = TRUE;
			PostMessage(v->hWndMainWindow, WM_COMMAND, IDC_SI_PARSER_RECORD, 0);
			SendControlResponse(szOK304, lstrlen(szOK304));
		}
		else
		{
			static char szError[] = {"511 Invalid record mode\r\n"};
			SendControlResponse(szError, lstrlen(szError));
			return;
		}
	}
}

void CS__Export(char * szParameters)
{
	// todo - test
	int nHTMLExportOptions = 0xffff;

	char * szSpace;
	static char szOK[] = {"309 Data exported\r\n"};
	static char szOpenError[] = {"520 Unable to open export file\r\n"};

	if (szParameters == NULL)
	{
		static char szError[] = {"512 No EXPORT parameters specified\r\n"};
		SendControlResponse(szError, lstrlen(szError));
		return;
	}

	szSpace = strstr(szParameters, " ");
	if (szSpace == NULL)
	{
		static char szError[] = {"513 No EXPORT filename specified\r\n"};
		SendControlResponse(szError, lstrlen(szError));
		return;
	}
	*szSpace = '\0';
	strupr(szParameters);

	if (strstr(szParameters, "HTML-") != NULL)
	{
		sscanf(&szParameters[5], "%x", &nHTMLExportOptions);
		szParameters[4] = '\0';
	}
	if (lstrcmp(szParameters, "HTML") == 0)
	{
		HANDLE hHTMLFile;

		hHTMLFile = CreateFile(szSpace + 1,
							  GENERIC_WRITE,
							  0,
							  (LPSECURITY_ATTRIBUTES) NULL,
							  CREATE_ALWAYS,
							  FILE_ATTRIBUTE_NORMAL,
							  (HANDLE) NULL);
		if (hHTMLFile != INVALID_HANDLE_VALUE)					
		{
			HTMLExport(hHTMLFile, nHTMLExportOptions, szSpace + 1);
			CloseHandle(hHTMLFile);
			SendControlResponse(szOK, lstrlen(szOK));
			return;
		}
		SendControlResponse(szOpenError, lstrlen(szOpenError));
		return;
	}
	else if (lstrcmp(szParameters, "XML") == 0)
	{
		HANDLE hXMLFile;

		hXMLFile = CreateFile(szSpace + 1,
							  GENERIC_WRITE,
							  0,
							  (LPSECURITY_ATTRIBUTES) NULL,
							  CREATE_ALWAYS,
							  FILE_ATTRIBUTE_NORMAL,
							  (HANDLE) NULL);
		if (hXMLFile != INVALID_HANDLE_VALUE)					
		{
			XMLExport(v->hWndMainWindow, hXMLFile);
			CloseHandle(hXMLFile);
			SendControlResponse(szOK, lstrlen(szOK));
			return;
		}
		SendControlResponse(szOpenError, lstrlen(szOpenError));
		return;
	}
	else if (lstrcmp(szParameters, "XMLTV") == 0)
	{
		HANDLE hXMLFile;

		hXMLFile = CreateFile(szSpace + 1,
							  GENERIC_WRITE,
							  0,
							  (LPSECURITY_ATTRIBUTES) NULL,
							  CREATE_ALWAYS,
							  FILE_ATTRIBUTE_NORMAL,
							  (HANDLE) NULL);
		if (hXMLFile != INVALID_HANDLE_VALUE)					
		{
			XMLTVExport(v->hWndMainWindow, hXMLFile);
			CloseHandle(hXMLFile);
			SendControlResponse(szOK, lstrlen(szOK));
			return;
		}
		SendControlResponse(szOpenError, lstrlen(szOpenError));
		return;
	}
	else
	{
		static char szError[] = {"514 Export mode not supported\r\n"};
		SendControlResponse(szError, lstrlen(szError));
		return;
	}
}

void SendControlserverStatistics(int nTable, char * szTableName)
{
	char szOutput[256];

	wsprintf(szOutput, "208 %s Sections: %d CRC Errors: %d\r\n",
		     szTableName,
			 v->nSIParserPackets[nTable], v->nSIParserCRCs[nTable]);
	SendControlResponse(szOutput, lstrlen(szOutput));	
}

void CS__ManualChannel(char * szParameters)
{
	int nConvertCount;
	int nProgramNumber, nPCRPID;
	int nPMTIndex, nESIndex;
	char * szSpace;
	char szTemp[128];
	static char szErrorChannelorPCR[] = {"539 Channel number or PCR PID incorrect\r\n"};
	static char szOK[] = {"322 Manual channel added\r\n"};

	if (szParameters == NULL)
	{
		static char szError[] = {"540 No parameters specified\r\n"};
		SendControlResponse(szError, lstrlen(szError));
		return;
	}

	// Get and validate program number and PCR PID
	nConvertCount = sscanf(szParameters, "%d %x", &nProgramNumber, &nPCRPID);
	if (nConvertCount != 2)
	{
		SendControlResponse(szErrorChannelorPCR, lstrlen(szErrorChannelorPCR));
		return;
	}
	if (nProgramNumber <= 0 || nProgramNumber > 65534)
	{
		static char szError[] = {"541 Invalid program number\r\n"};
		SendControlResponse(szError, lstrlen(szError));
		return;
	}
	for (nPMTIndex = 0; nPMTIndex < MAX_PAT_ENTRIES; nPMTIndex++)
	{
		if (v->pat.pmt[nPMTIndex].nPMTPID == 0)
			break;
		if (v->pat.pmt[nPMTIndex].nProgramNumber == nProgramNumber)
		{
			static char szError[] = {"542 Channel already exists\r\n"};
			SendControlResponse(szError, lstrlen(szError));
			return;
		}
	}
	if (nPCRPID <= 0 || nPCRPID >= 0x1fff)
	{
		static char szError[] = {"543 Invalid PCR PID\r\n"};
		SendControlResponse(szError, lstrlen(szError));
		return;
	}
	szSpace = strstr(szParameters, " ");
	if (szSpace == NULL)
	{
		SendControlResponse(szErrorChannelorPCR, lstrlen(szErrorChannelorPCR));
		return;
	}
	szSpace = strstr(szSpace + 1, " ");
	if (szSpace == NULL)
	{
		SendControlResponse(szErrorChannelorPCR, lstrlen(szErrorChannelorPCR));
		return;
	}
	szSpace++;

	// Now each of the ES - szSpace points to the first one
	memset(&v->editpmt, 0, sizeof(PMT));
	v->editpmt.nPCRPID = nPCRPID;
	v->editpmt.nPMTPID = -2;
	v->editpmt.nProgramNumber = nProgramNumber;

	nESIndex = 0;
	do
	{
		int nESType = 0;
		int nESPID = 0;
		
		nConvertCount = sscanf(szSpace, "%x %x", &nESType, &nESPID);
		if ( (nConvertCount != 2)
			|| (nESType <= 0 || nESType > 0xff)
			|| (nESPID <= 0 || nESPID >= 0x1fff) )
		{
			static char szError[] = {"544 Invalid ES parameters\r\n"};
			SendControlResponse(szError, lstrlen(szError));
			return;
		}
		v->editpmt.es[nESIndex].nESPID = nESPID;
		v->editpmt.es[nESIndex].nStreamType = nESType;
		nESIndex++;
		
		// Skip past what we just converted
		szSpace = strstr(szSpace + 1, " ");
		szSpace = strstr(szSpace + 1, " ");
	} while (szSpace != NULL);

	if (nESIndex == 0)
	{
		static char szError[] = {"545 No elementary streams specified\r\n"};
		SendControlResponse(szError, lstrlen(szError));
		return;
	}

	// Now we can add the channel
	for (nPMTIndex = 0; nPMTIndex < MAX_PAT_ENTRIES; nPMTIndex++)
	{
		if (v->pat.pmt[nPMTIndex].nPMTPID == 0 && v->pat.pmt[nPMTIndex].nProgramNumber == 0)
			break;
	}
	if (nPMTIndex == MAX_PAT_ENTRIES)
	{
		static char szError[] = {"546 Too many channels defined\r\n"};
		SendControlResponse(szError, lstrlen(szError));
		return;
	}
	memcpy(&v->pat.pmt[nPMTIndex], &v->editpmt, sizeof(PMT));

	wsprintf(szTemp, "Manual - Program %d", v->pat.pmt[nPMTIndex].nProgramNumber);
	v->pat.pmt[nPMTIndex].hPMTTreeItem = AddItemToSITree(GetDlgItem(v->hDlgSIParser, IDC_SI_TREE), szTemp, 2, SI_PARSER_PMT + nPMTIndex, 1, v->pat.hPATTreeItem, NULL);
	PostMessage(v->hDlgSIParser, WM_USER + 2, SI_PARSER_PMT, nPMTIndex);
	v->fDirtyManualChannels = TRUE;

	SendControlResponse(szOK, lstrlen(szOK));
	return;

}

void CS__GetInfo(char * szParameters)
{
	static char szInfoComplete[] = {"315 INFO complete\r\n"};

	if (szParameters == NULL)
	{
		static char szError[] = {"534 No parameters specified\r\n"};
		SendControlResponse(szError, lstrlen(szError));
		return;
	}
	strupr(szParameters);
	if (lstrcmp(szParameters, "SOURCE") == 0)
	{
		int nLine;

		for (nLine = 0; nLine < 10; nLine++)
		{
			char szTemp[128];
			GetSourceInfoLine(nLine, szTemp);
			if (lstrlen(szTemp))
			{
				char szOutput[128];
				wsprintf(szOutput, "208 %s\r\n", szTemp);
				SendControlResponse(szOutput, lstrlen(szOutput));
			}
		}
		SendControlResponse(szInfoComplete, lstrlen(szInfoComplete));
	}
	else if (lstrcmp(szParameters, "MPEG") == 0)
	{
		char szOutput[256];
		char szNITLabel[8] = {"NIT"};
		char szSDTLabel[8] = {"SDT"};
		char szMuxRate[64] = {"n/a"};

		if (v->nNetworkPID == 0x1ffb)
		{
			lstrcpy(szNITLabel, "ETT");
			lstrcpy(szSDTLabel, "PSIP");
		}
				
		SendControlserverStatistics(SI_PARSER_STATS_PAT, "PAT");
		SendControlserverStatistics(SI_PARSER_STATS_CAT, "CAT");
		SendControlserverStatistics(SI_PARSER_STATS_PMT, "PMT");
		SendControlserverStatistics(SI_PARSER_STATS_NIT, szNITLabel);
		if (v->nNetworkPID != 0x0ffe)
		{
			SendControlserverStatistics(SI_PARSER_STATS_SDT, szSDTLabel);
			SendControlserverStatistics(SI_PARSER_STATS_EIT, "EIT");
		}

		wsprintf(szOutput, "208 Continuity errors: %d\r\n", v->nContinuityErrors);
		SendControlResponse(szOutput, lstrlen(szOutput));
		wsprintf(szOutput, "208 TEI errors: %d\r\n", v->nTEIErrors); 
		SendControlResponse(szOutput, lstrlen(szOutput));
		if (GetSyncLossCount != NULL)
		{
			wsprintf(szOutput, "208 Sync Losses: %d\r\n", GetSyncLossCount(FALSE));
			SendControlResponse(szOutput, lstrlen(szOutput));
		}
		if (GetRetuneCount != NULL)
		{
			wsprintf(szOutput, "208 Retunes: %d\r\n", GetRetuneCount(FALSE)); 
			SendControlResponse(szOutput, lstrlen(szOutput));
		}

		if (v->dDisplayMuxRate > 0)
		{
			EnterCriticalSection(&v->ss.csPIDCounter);
			sprintf(szMuxRate, "%.0f bps", (v->dDisplayMuxRate / (double)v->nMuxRateCounter) * 8.0);
			LeaveCriticalSection(&v->ss.csPIDCounter);
		}
		wsprintf(szOutput, "208 Calculated multiplex rate: %s\r\n", szMuxRate);
		SendControlResponse(szOutput, lstrlen(szOutput));
		
		EnterCriticalSection(&v->ss.csPIDCounter);
		sprintf(szOutput, "208 Data Last second: %.3f Mbit\r\n", (double)v->lnPIDSecondCounter[59] * 8.0 / 1000.0 / 1000.0);
		LeaveCriticalSection(&v->ss.csPIDCounter);
		SendControlResponse(szOutput, lstrlen(szOutput));

		SendControlResponse(szInfoComplete, lstrlen(szInfoComplete));
	}
	else
	{
		static char szError[] = {"535 INFO mode not supported\r\n"};
		SendControlResponse(szError, lstrlen(szError));
		return;
	}
}

#ifdef PRO
static char szXMLLeadin[] = {"<?xml version = \"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\r\n"};
void CS__XML_SendGeneralInfo()
{
	char szSource[128], * szSourcePtr;
	char szTuner[128], * szTunerPtr;
	char szSignal[128], * szSignalPtr;
	char szProfile[128], * szProfilePtr;
	char szNetType[128], * szNetTypePtr;
	char szRunTime[128], * szRunTimePtr;
	char szResponse[512];
	static char szNA[] = {"n/a"};

	GetSourceInfoLine(0, szSource);		szSourcePtr = strstr(szSource, ":");
	GetSourceInfoLine(1, szTuner);		szTunerPtr = strstr(szTuner, ":");
	GetSourceInfoLine(2, szSignal);		szSignalPtr = strstr(szSignal, ":");
	GetSourceInfoLine(6, szProfile);	szProfilePtr = strstr(szProfile, ":");
	GetSourceInfoLine(7, szNetType);	szNetTypePtr = strstr(szNetType, ":");
	GetSourceInfoLine(8, szRunTime);	szRunTimePtr = strstr(szRunTime, ":");
	
	if (szSourcePtr == NULL)	szSourcePtr = szNA;		else szSourcePtr += 2;
	if (szTunerPtr == NULL)		szTunerPtr = szNA;		else szTunerPtr += 2;
	if (szSignalPtr == NULL)	szSignalPtr = szNA;		else szSignalPtr += 2;
	if (szProfilePtr == NULL)	szProfilePtr = szNA;	else szProfilePtr += 2;
	if (szNetTypePtr == NULL)	szNetTypePtr = szNA;	else szNetTypePtr += 2;
	if (szRunTimePtr == NULL)	szRunTimePtr = szNA;	else szRunTimePtr += 2;

	SendControlResponse(szXMLLeadin, lstrlen(szXMLLeadin));
	
	wsprintf(szResponse, "<GEN_INFO source=\"%s\" tuner=\"%s\" signal=\"%s\" profile=\"%s\" net_type=\"%s\" run_time=\"%s\" />\r\n\0",
		     szSourcePtr,
			 szTunerPtr,
			 szSignalPtr,
			 szProfilePtr,
			 szNetTypePtr,
			 szRunTimePtr);
	SendControlResponse(szResponse, lstrlen(szResponse) + 1);
}

void CS__XML_SendPIDs()
{
	int i;
	double dPercent, dRate = 0.0;
	PIDCOUNTER pc[8192];
	char * szString;

	EnterCriticalSection(&v->ss.csPIDCounter);
	memcpy(&pc, &v->pc, sizeof(PIDCOUNTER) * 8192);
	LeaveCriticalSection(&v->ss.csPIDCounter);
	qsort(&pc, 8192, sizeof(PIDCOUNTER), SortPIDsByPID);

	SendControlResponse(szXMLLeadin, lstrlen(szXMLLeadin));
	szString = "<MUXUTIL>\r\n";
	SendControlResponse(szString, lstrlen(szString));
	szString = " <PIDS>\r\n";
	SendControlResponse(szString, lstrlen(szString));

	for (i = 0; i < 8192; i++)
	{
		if (pc[i].lnPackets)
		{
			char szResponse[512];

			dPercent = ((double)pc[i].lnPackets / (double)v->lnCopyTotalTSPackets) * 100.0;
			if (pc[i].dPIDRate)
				dRate = pc[i].dPIDRate * 8.0 / 1000.0 / 1000.0;
			else
			{
				if (v->dDisplayMuxRate)
				{
					dRate = (v->dDisplayMuxRate / (double)v->nMuxRateCounter) * 8.0;
					dRate = ((dRate / 100.0) * dPercent) / 1000.0 / 1000.0;
				}
			}

			sprintf(szResponse, "  <PID pid='0x%04x' cc_errors='%d' tei_errors='%d' pct='%.2f' bitrate='%.3f' scrambled='%d'/>\r\n",
				    pc[i].nPID,
					pc[i].nPIDHasContinuityErrors,
					pc[i].nPIDTEICount,
					dPercent,
					dRate,
					pc[i].fScrambled);
			SendControlResponse(szResponse, lstrlen(szResponse));
		}
	}
	szString = " </PIDS>\r\n";
	SendControlResponse(szString, lstrlen(szString));
	szString = "</MUXUTIL>\r\n\0";
	SendControlResponse(szString, lstrlen(szString) + 1);
}

char * CS__XMLFormatStats(int nValue)
{
	static char szReturnValue[128];
	
	if (nValue > 1000 * 1000)
		sprintf(szReturnValue, "%.1fm", (double)nValue / 1000.0 / 1000.0);
	else if (nValue > 1000)
		sprintf(szReturnValue, "%.1fk", (double)nValue / 1000.0);
	else
		wsprintf(szReturnValue, "%d", nValue);

	return szReturnValue;
}

char * CS__XMLFormatStats64(__int64 nValue)
{
	static char szReturnValue[128];
	
	if (nValue > 1000 * 1000 * 1000)
		sprintf(szReturnValue, "%.1fb", (double)nValue / 1000.0 / 1000.0 / 1000.0);
	else if (nValue > 1000 * 1000)
		sprintf(szReturnValue, "%.1fm", (double)nValue / 1000.0 / 1000.0);
	else if (nValue > 1000)
		sprintf(szReturnValue, "%.1fk", (double)nValue / 1000.0);
	else
		wsprintf(szReturnValue, "%d", nValue);

	return szReturnValue;
}

void CS__XML_SendStats()
{
	char * szString;
	char szSections[1024] =  {" <SECTIONS "};
	char szCRCErrors[1024] = {" <CRCERRORS "};
	char szMiscErrors[1024] = {" <MISCERRORS "};
	char szNITLabel[8] = {"nit"};
	char szSDTLabel[8] = {"sdt"};
	char szTemp[128];

	SendControlResponse(szXMLLeadin, lstrlen(szXMLLeadin));
	szString = "<STATUS>\r\n";
	SendControlResponse(szString, lstrlen(szString));

	if (v->nNetworkPID == 0x1ffb)
	{
		lstrcpy(szNITLabel, "ett");
		lstrcpy(szSDTLabel, "psip");
	}

	wsprintf(szTemp, "pat='%s' ", CS__XMLFormatStats64(v->nSIParserPackets[SI_PARSER_STATS_PAT]));				lstrcat(szSections, szTemp);
	wsprintf(szTemp, "pat='%s' ", CS__XMLFormatStats64(v->nSIParserCRCs[SI_PARSER_STATS_PAT]));				lstrcat(szCRCErrors, szTemp);

	wsprintf(szTemp, "cat='%s' ", CS__XMLFormatStats64(v->nSIParserPackets[SI_PARSER_STATS_CAT]));				lstrcat(szSections, szTemp);
	wsprintf(szTemp, "cat='%s' ", CS__XMLFormatStats64(v->nSIParserCRCs[SI_PARSER_STATS_CAT]));				lstrcat(szCRCErrors, szTemp);

	wsprintf(szTemp, "pmt='%s' ", CS__XMLFormatStats64(v->nSIParserPackets[SI_PARSER_STATS_PMT]));				lstrcat(szSections, szTemp);
	wsprintf(szTemp, "pmt='%s' ", CS__XMLFormatStats64(v->nSIParserCRCs[SI_PARSER_STATS_PMT]));				lstrcat(szCRCErrors, szTemp);

	wsprintf(szTemp, "%s='%s' ", szNITLabel, CS__XMLFormatStats64(v->nSIParserPackets[SI_PARSER_STATS_NIT]));				lstrcat(szSections, szTemp);
	wsprintf(szTemp, "%s='%s' ", szNITLabel, CS__XMLFormatStats64(v->nSIParserCRCs[SI_PARSER_STATS_NIT]));				lstrcat(szCRCErrors, szTemp);

	if (v->nNetworkPID != 0x0ffe)
	{
		wsprintf(szTemp, "%s='%s' ", szSDTLabel, CS__XMLFormatStats64(v->nSIParserPackets[SI_PARSER_STATS_SDT]));				lstrcat(szSections, szTemp);
		wsprintf(szTemp, "%s='%s' ", szSDTLabel, CS__XMLFormatStats64(v->nSIParserCRCs[SI_PARSER_STATS_SDT]));				lstrcat(szCRCErrors, szTemp);

		wsprintf(szTemp, "eit='%s' ", CS__XMLFormatStats64(v->nSIParserPackets[SI_PARSER_STATS_EIT]));				lstrcat(szSections, szTemp);
		wsprintf(szTemp, "eit='%s' ", CS__XMLFormatStats64(v->nSIParserCRCs[SI_PARSER_STATS_EIT]));				lstrcat(szCRCErrors, szTemp);
	}
	lstrcat(szSections, "/>\r\n");
	SendControlResponse(szSections, lstrlen(szSections));
	lstrcat(szCRCErrors, "/>\r\n");
	SendControlResponse(szCRCErrors, lstrlen(szCRCErrors));

	wsprintf(szTemp, " <MISCERRORS continuity='%d' tei='%d' synclosses='%d' retunes='%d'/>\r\n",
		     v->nContinuityErrors,
			 v->nTEIErrors,
			 v->nSyncLossCount,
			 v->nRetuneCount);
	SendControlResponse(szTemp, lstrlen(szTemp));

	/*	todo
   <DATARATE mux_bitrate='34224807 b/s' last_sec='38.235 Mbit' in_buf='0' 
out_buf='0'/>
</STATUS>
*/
	szString = "</STATUS>\r\n\0";
	SendControlResponse(szString, lstrlen(szString) + 1);

}

#define SERVICE_VIDEO 1
#define SERVICE_AUDIO 2
#define SERVICE_TELETEXT 3
#define SERVICE_SUBTITLE 4
#define SERVICE_DATA 5
#define SERVICE_UNKNOWN 0

void CS__XMLFinishCurrentTable(int * nCurrentlyDoing)
{
	if (*nCurrentlyDoing != SI_PARSER_NOP)
	{
		SendControlResponse(szCurrentlyDoingTerminator, lstrlen(szCurrentlyDoingTerminator));
		szCurrentlyDoingTerminator[0] = '\0';
	}
	*nCurrentlyDoing = SI_PARSER_NOP;
}

void SendRequestedData(int nDataRequestlParam)
{
	int nItemIndex;
	char * szResponse = NULL;

	switch(nDataRequestlParam & 0xf0000000)
	{
	case SI_PARSER_CVCT:
		nItemIndex = nDataRequestlParam - SI_PARSER_CVCT;
		szResponse = FormatCVCT(nItemIndex);
		break;
	case SI_PARSER_BAT:
		nItemIndex = nDataRequestlParam - SI_PARSER_BAT;
		szResponse = FormatBAT(nItemIndex);
		break;
	case SI_PARSER_MGT:
		szResponse = FormatMGT();
		break;
	case SI_PARSER_CDT:	// Also the RRT
		switch(v->nNetworkPID)
		{
		default:
			nItemIndex = nDataRequestlParam - SI_PARSER_CDT;
			szResponse = FormatCDTEntry(nItemIndex);
			break;
		case 0x1ffb:
			nItemIndex = nDataRequestlParam - SI_PARSER_RRT;
			szResponse = FormatRRTEntry(nItemIndex);
			break;
		}
		break;
	case SI_PARSER_TDT:
		switch(v->nNetworkPID)
		{
		default:
			nItemIndex = nDataRequestlParam - SI_PARSER_TDT;
			szResponse = FormatTDTEntry(nItemIndex);
			break;
		case 0x0010:
			if ((nDataRequestlParam & 0x0fffffff) == 0)
			{
				szResponse = "";
			}
			else
			{
				szResponse = FormatDVBTOT();
			}
			break;
			/*
		case 0x1ffb:
			{
				int DS_status = (v->dvbtdt.nDaylightSavings >> 15) & 1;
				int DS_day_of_month = (v->dvbtdt.nDaylightSavings >> 8) & 0x1f;
				int DS_hour = v->dvbtdt.nDaylightSavings & 0xff;
				char szTemp[1024];

				wsprintf(szTemp, "STT: GPS/UTC offset: %d\r\nSTT: DS Status: %d DS Day of Month: %d DS Hour: %d\r\n",
					     v->dvbtdt.nGPSOffset,
						 DS_status,
						 DS_day_of_month,
						 DS_hour);
#ifdef PRO
				{
					__int64 nSystemTime, nStream;
					__int64 nDifference;
					SYSTEMTIME stSystemTime, stStreamTime;
					char szTemp2[128];

					GetSystemTime(&stSystemTime);
					SystemTimeToFileTime(&stSystemTime, (FILETIME *)&nSystemTime);
					
					memset(&stStreamTime, 0, sizeof(stStreamTime));
					stStreamTime.wYear = v->dvbtdt.nYear;
					stStreamTime.wMonth = v->dvbtdt.nMonth;
					stStreamTime.wDay = v->dvbtdt.nDay;
					stStreamTime.wHour = v->dvbtdt.nHour;
					stStreamTime.wMinute = v->dvbtdt.nMinute;
					stStreamTime.wSecond = v->dvbtdt.nSecond;
					SystemTimeToFileTime(&stStreamTime, (FILETIME *)&nStream);

					nDifference = nStream - nSystemTime;
					nDifference /= 10000000;
					wsprintf(szTemp2, "Difference between stream and PC time: %d seconds\r\n", (int)nDifference);
					lstrcat(szTemp, szTemp2);
				}
#endif PRO
				SetDlgItemText(hDlg, IDC_SI_TEXT, szTemp);
			}
			memset(&v->nHighlightPIDs, -1, sizeof(v->nHighlightPIDs));
			v->nHighlightPIDs[0] = 0x1ffb;
			PIDManagement(TRUE, v->nHighlightPIDs[0], TRUE);
			ForcePIDChartRepaint(hDlg);
			break;*/
		}
		break;
	case SI_PARSER_SIT:
		nItemIndex = nDataRequestlParam - SI_PARSER_SIT;
		szResponse = FormatSITEntry(nItemIndex);
		break;
	case SI_PARSER_MMT:
		nItemIndex = nDataRequestlParam - SI_PARSER_MMT;
		szResponse = FormatMMTEntry(nItemIndex);
		break;
	case SI_PARSER_NIT:
		nItemIndex = nDataRequestlParam - SI_PARSER_NIT;
		szResponse = FormatNITEntry(nItemIndex, FALSE);
		break;
	case SI_PARSER_PAT:
		szResponse = FormatPAT(FALSE, v->nExportSITables);
		break;
	case SI_PARSER_PMT:
		nItemIndex = nDataRequestlParam - SI_PARSER_PMT;
		szResponse = FormatPMTEntry(nItemIndex, FALSE);
		break;
	case SI_PARSER_ES:
		nItemIndex = nDataRequestlParam - SI_PARSER_ES;	// this isn't the index, it's the ES PID
		szResponse = FormatESEntry(nItemIndex);
		break;
	case SI_PARSER_SDT:
	case SI_PARSER_VCT:
		nItemIndex = nDataRequestlParam - SI_PARSER_SDT;
		szResponse = FormatSDTEntry(nItemIndex, FALSE);
		break;
	case SI_PARSER_EIT:
		nItemIndex = nDataRequestlParam - SI_PARSER_EIT;
		szResponse = FormatEITEntry(nItemIndex, EIT_FORMAT_PLAIN, FALSE);
		break;
	case SI_PARSER_CAT:
		szResponse = FormatCAT(FALSE);
		break;
	}

	if (szResponse != NULL)
	{
		char * szString = "<SITEXT data='";
		SendControlResponse(szString, lstrlen(szString));
		SendControlResponse(szResponse, lstrlen(szResponse));
		szString = "'/>\r\n\0";
		SendControlResponse(szString, lstrlen(szString) + 1);
	}
}

DWORD WINAPI ControlServerXMLStreamer(LPVOID lpv)
{
	DWORD dwNextStats = GetTickCount();
	char * szString;

	while (!fKillXMLStreamThread && v->fRunning)
	{
		int nXMLLogIndex;
		int nCurrentlyDoing = SI_PARSER_NOP;
		BOOL fSentAtLeastOne = FALSE;
		
		EnterCriticalSection(&v->csXMLLog);

		if (fDataRequest)
		{
			SendRequestedData(nDataRequestlParam);
			nDataRequestlParam = 0;
			fDataRequest = FALSE;
		}

		// Send the SI
		for (nXMLLogIndex = 0; nXMLLogIndex < v->nXMLLogCount; nXMLLogIndex++)
		{
			if (v->XMLLog[nXMLLogIndex].fSent == FALSE && v->XMLLog[nXMLLogIndex].nXMLLogType == XML_LOG_TYPE_WMUSER2)
			{
				if (!fSentAtLeastOne)
				{
					SendControlResponse(szXMLLeadin, lstrlen(szXMLLeadin));
					szString = "<SI label='System Information'>\r\n";
					SendControlResponse(szString, lstrlen(szString));
					fSentAtLeastOne = TRUE;
				}

				switch(v->XMLLog[nXMLLogIndex].wParam)
				{
				case SI_PARSER_PAT:
					{
						int i;
						char szTemp[1024];

						if (nCurrentlyDoing)
							CS__XMLFinishCurrentTable(&nCurrentlyDoing);
						wsprintf(szTemp, " <PAT level='1' id=\"%08x\" parent=\"%08x\" select=\"%08x\" label=\"PAT\" pid=\"0x0000\">\r\n",
						         v->pat.hPATTreeItem,
								 0,	// no parent
								 SI_PARSER_PAT);
						SendControlResponse(szTemp, lstrlen(szTemp));

						for (i = 0; i < MAX_PAT_ENTRIES; i++)
						{
							char szLabel[128];

							if (v->pat.pmt[i].nPMTPID == 0)
								break;
							if (v->pat.pmt[i].nPMTPID == -2)
								continue;

							if (v->pat.pmt[i].nProgramNumber)
								wsprintf(szLabel, "PMT Program %d", v->pat.pmt[i].nProgramNumber);
							else
								wsprintf(szLabel, "PMT Network");
							wsprintf(szTemp, "  <PMT level='2' id=\"%08x\" parent=\"%08x\" select=\"%08x\" label=\"%s\" pid=\"0x%04x\" program='%d'/>\r\n",
								     v->pat.pmt[i].hPMTTreeItem,
									 v->pat.hPATTreeItem,
									 SI_PARSER_PMT + i,
									 szLabel,
									 v->pat.pmt[i].nPMTPID,
									 v->pat.pmt[i].nProgramNumber);
							SendControlResponse(szTemp, lstrlen(szTemp));
						}
						szString = " </PAT>\r\n";
						SendControlResponse(szString, lstrlen(szString));
					}
					break;
				case SI_PARSER_PMT:
					{
						int nPMTIndex = (int)v->XMLLog[nXMLLogIndex].lParam;
						int nESIndex;
						BOOL fVideoService = FALSE;
						char szTemp[1024];

						if (nCurrentlyDoing)
							CS__XMLFinishCurrentTable(&nCurrentlyDoing);
						wsprintf(szTemp, " <PMT program='%d' pcr_pid=\"0x%04x\" label=\"PMT Program %d\" pid=\"0x%04x\">\r\n",
							     v->pat.pmt[nPMTIndex].nProgramNumber,
								 v->pat.pmt[nPMTIndex].nPCRPID,
								 v->pat.pmt[nPMTIndex].nProgramNumber,
								 v->pat.pmt[nPMTIndex].nPMTPID);
						SendControlResponse(szTemp, lstrlen(szTemp));
						for (nESIndex = 0; nESIndex < MAX_ESLIST_ENTRIES; nESIndex++)
						{
							int nService = SERVICE_UNKNOWN;
							char * szService;
							char szLabel[64];

							if (v->pat.pmt[nPMTIndex].es[nESIndex].nESPID == 0)
								break;

							switch(v->pat.pmt[nPMTIndex].es[nESIndex].nStreamType)
							{
							case 0x01:	// MPEG-1
							case 0x02:	// MPEG-2
							case 0x10:	// MPEG-4
							case 0x1b:	// H264
							case 0xea:	// VC1
								nService = SERVICE_VIDEO;
								break;
							case 0x03:	// MPEG-1
							case 0x04:	// MPEG-2
							case 0x11:	// MPEG-4
							case 0x81:	// AC3
							case 0x83:	// LPCM
							case 0x85:	// DTS
							case 0xe6:	// WM9
								nService = SERVICE_AUDIO;
								break;
							case 0x06:			// perhaps AC3/PCM. Let's see if there's an AC3 descriptor
								if (   IsAC3AudioStream(nPMTIndex, nESIndex) == TRUE
									|| IsPCMAudioStream(nPMTIndex, nESIndex) == TRUE
									|| IsDTSAudioStream(nPMTIndex, nESIndex) == TRUE)
									nService = SERVICE_AUDIO;
								else if (IsTeleTextOrVBIStream(nPMTIndex, nESIndex) == TRUE)
									nService = SERVICE_TELETEXT;
								else if (IsSubtitleStream(nPMTIndex, nESIndex) == TRUE)
									nService = SERVICE_SUBTITLE;
								else
									nService = SERVICE_DATA;
								break;
							default:
								if (v->nNetworkPID != 0x0010)
								{
									if (v->pat.pmt[nPMTIndex].es[nESIndex].nStreamType == 0x80) // DCII video
									{
										nService = SERVICE_VIDEO;
										break;
									}
								}
								if (IsDataBroadcastStream(nPMTIndex, nESIndex) == TRUE)
								{
									nService = SERVICE_DATA;
									break;
								}
								nService = SERVICE_UNKNOWN;
								break;
							}
							switch(nService)
							{
							case SERVICE_VIDEO:
								szService = "Video";
								break;
							case SERVICE_AUDIO:
								szService = "Audio";
								break;
							case SERVICE_TELETEXT:
								szService = "Teltext";
								break;
							case SERVICE_SUBTITLE:
								szService = "Subtitle";
								break;
							case SERVICE_DATA:
								szService = "Data";
								break;
							default:
								szService = "Unknown";
								break;
							}

							wsprintf(szLabel, "ES PID 0x%04x", v->pat.pmt[nPMTIndex].es[nESIndex].nESPID);
							wsprintf(szTemp, "  <ES level='3' id=\"%08x\" parent=\"%08x\" select=\"%08x\" label=\"%s\" pid=\"0x%04x\" es_type=\"0x%02x\" type=\"%s\">\r\n",
								     v->pat.pmt[nPMTIndex].es[nESIndex].hESTreeItem,
									 v->pat.pmt[nPMTIndex].hPMTTreeItem,
									 SI_PARSER_ES + v->pat.pmt[nPMTIndex].es[nESIndex].nESPID,
									 szLabel,
									 v->pat.pmt[nPMTIndex].es[nESIndex].nESPID,
									 v->pat.pmt[nPMTIndex].es[nESIndex].nStreamType,
									 szService);
							SendControlResponse(szTemp, lstrlen(szTemp));

							if (v->pat.pmt[nPMTIndex].es[nESIndex].pDescriptors != NULL)
							{
								int nDescriptorsLength = v->pat.pmt[nPMTIndex].es[nESIndex].nDescriptorsLength;
								int nCurrentIndex = 0;

								do
								{
									char szDescriptor[128];

									DecodeDescriptorNames(szDescriptor, v->pat.pmt[nPMTIndex].es[nESIndex].pDescriptors[nCurrentIndex]);

									wsprintf(szTemp, "    <DS level='4' id=\"00000000\" parent=\"%08x\" label=\"%s\"/>\r\n",
										     v->pat.pmt[nPMTIndex].es[nESIndex].hESTreeItem,
											 szDescriptor);
									SendControlResponse(szTemp, lstrlen(szTemp));

									nCurrentIndex += (BYTE)v->pat.pmt[nPMTIndex].es[nESIndex].pDescriptors[nCurrentIndex + 1]; // descriptor length
									nCurrentIndex += 2;	// descriptor tag and length
								} while (nCurrentIndex < nDescriptorsLength);
							}
							szString = "  </ES>\r\n";
							SendControlResponse(szString, lstrlen(szString));

						}

						szString = " </PMT>\r\n";
						SendControlResponse(szString, lstrlen(szString));
					}
					break;
				case SI_PARSER_SDT:
				case SI_PARSER_VCT:
					{
						int nChannelNumber = (int)v->XMLLog[nXMLLogIndex].lParam;
						int nTablePID;
						char szTemp[1024];
						char szLabel[32];
						char szType[8];
						char szPIDAttribute[32] = {""};
						
						if (v->nNetworkPID == 0x0ffe)
							break;
						
						if (nCurrentlyDoing)
						{
							if (nCurrentlyDoing != SI_PARSER_SDT)
								CS__XMLFinishCurrentTable(&nCurrentlyDoing);
						}
						if (v->XMLLog[nXMLLogIndex].wParam == SI_PARSER_SDT)
						{
							wsprintf(szLabel, "SDT PID 0x0011");
							wsprintf(szType, "SDT");
							nTablePID = 0x0011;
							lstrcpy(szPIDAttribute, " pid=\"0x0011\"");
						}
						else
						{
							wsprintf(szLabel, "TCVT PID 0x1ffb");
							wsprintf(szType, "TCVT");
							nTablePID = 0x1ffb;
						}
						if (nCurrentlyDoing == SI_PARSER_NOP)
						{
							wsprintf(szTemp, " <%s level='1' id=\"%08x\" parent=\"00000000\" label=\"%s\" pid=\"0x%04x\">\r\n",
									 szType,
									 v->hSDTRootTreeItem,
									 szLabel,
									 nTablePID);
							SendControlResponse(szTemp, lstrlen(szTemp));
							wsprintf(szCurrentlyDoingTerminator, " </%s>\r\n", szType);
							nCurrentlyDoing = SI_PARSER_SDT;
						}
						wsprintf(szTemp, "  <%s_ENTRY level='2' id=\"%08x\" parent=\"%08x\" select=\"%08x\" label=\"%d %s\" program='%d'%s/>\r\n",
							     szType,
								 v->pChannelData[nChannelNumber]->hSDTTreeItem,
								 v->hSDTRootTreeItem,
								 SI_PARSER_SDT + nChannelNumber,
							     v->pChannelData[nChannelNumber]->nChannelNumber,
								 v->pChannelData[nChannelNumber]->szShortName,
								 nChannelNumber,
								 szPIDAttribute);
						SendControlResponse(szTemp, lstrlen(szTemp));						
					}
					break;
				case SI_PARSER_EIT:
					{
						int nChannelNumber = (int)v->XMLLog[nXMLLogIndex].lParam;
						char szTemp[1024];

						if (nCurrentlyDoing)
						{
							if (nCurrentlyDoing != SI_PARSER_EIT)
								CS__XMLFinishCurrentTable(&nCurrentlyDoing);
						}
						if (nCurrentlyDoing == SI_PARSER_NOP)
						{
							char szLabel[32];

							if (v->nNetworkPID == 0x0010)
								wsprintf(szLabel, "EIT PID 0x%04x", v->nEITPID);
							else
								lstrcpy(szLabel, "EIT/ETT");
							wsprintf(szTemp, " <EIT level='1' id=\"%08x\" parent=\"00000000\" label=\"%s\">\r\n",
									 v->hEITRootTreeItem,
									 szLabel);
							SendControlResponse(szTemp, lstrlen(szTemp));
							wsprintf(szCurrentlyDoingTerminator, " </EIT>\r\n");
							nCurrentlyDoing = SI_PARSER_EIT;
						}
						wsprintf(szTemp, "  <EIT_ENTRY level='2' id=\"%08x\" parent=\"%08x\" select=\"%08x\" label=\"EIT Program %d\" program='%d'/>\r\n",
								 v->hEITTreeItem[nChannelNumber],
								 v->hEITRootTreeItem,
								 SI_PARSER_EIT + nChannelNumber,
								 nChannelNumber,
								 nChannelNumber);
						SendControlResponse(szTemp, lstrlen(szTemp));						
					}
					break;					
				case SI_PARSER_TDT:
					{
						int nTDTIndex = (int)v->XMLLog[nXMLLogIndex].lParam;
						char szTemp[1024];

						if (nCurrentlyDoing != SI_PARSER_NOP)
							CS__XMLFinishCurrentTable(&nCurrentlyDoing);

						switch(v->nNetworkPID)
						{
						case 0x0010:
							if (nTDTIndex == 0)
							{
								// DVB TDT
								wsprintf(szTemp, " <TDT level='1' id=\"%08x\" parent=\"00000000\" label=\"TDT PID 0x0014\" pid=\"0x0014\">\r\n",
									     v->dvbtdt.hRootTreeItem);
								SendControlResponse(szTemp, lstrlen(szTemp));						

								wsprintf(szTemp, "  <TDT_DATE_TIME level='2' id=\"%08x\" parent=\"%08x\" label=\"%04d/%02d/%02d %02d:%02d:%02d\" />\r\n",
									     v->dvbtdt.hTreeItem,
										 v->dvbtdt.hRootTreeItem,
										 v->dvbtdt.nYear, v->dvbtdt.nMonth, v->dvbtdt.nDay,
										 v->dvbtdt.nHour, v->dvbtdt.nMinute, v->dvbtdt.nSecond);
								SendControlResponse(szTemp, lstrlen(szTemp));
								szString = " </TDT>\r\n";
								SendControlResponse(szString, lstrlen(szString));
							}
							else
							{
								wsprintf(szTemp, " <TOT level='1' id=\"%08x\" parent=\"00000000\" label=\"TDT PID 0x0014\" pid=\"0x0014\">\r\n",
									     v->dvbtot.hRootTreeItem);
								SendControlResponse(szTemp, lstrlen(szTemp));						

								wsprintf(szTemp, "  <TOT_DATE_TIME level='2' id=\"%08x\" parent=\"%08x\" select=\"%08x\" label=\"%04d/%02d/%02d %02d:%02d:%02d\" />\r\n",
									     v->dvbtot.hTreeItem,
										 v->dvbtot.hRootTreeItem,
										 SI_PARSER_TDT + 1,
										 v->dvbtot.nYear, v->dvbtot.nMonth, v->dvbtot.nDay,
										 v->dvbtot.nHour, v->dvbtot.nMinute, v->dvbtot.nSecond);
								SendControlResponse(szTemp, lstrlen(szTemp));
								szString = " </TOT>\r\n";
								SendControlResponse(szString, lstrlen(szString));
							}
							break;
						case 0x1ffb:
							{
								wsprintf(szTemp, " <STT level='1' id=\"%08x\" parent=\"00000000\" label=\"STT PID 0x1ffb\" pid=\"0x1ffb\">\r\n",
									     v->dvbtdt.hRootTreeItem);
								SendControlResponse(szTemp, lstrlen(szTemp));						

								wsprintf(szTemp, "  <STT_DATE_TIME level='2' id=\"%08x\" parent=\"%08x\" select=\"%08x\" label=\"%04d/%02d/%02d %02d:%02d:%02d\" />\r\n",
									     v->dvbtdt.hTreeItem,
										 v->dvbtdt.hRootTreeItem,
										 SI_PARSER_TDT,
										 v->dvbtdt.nYear, v->dvbtdt.nMonth, v->dvbtdt.nDay,
										 v->dvbtdt.nHour, v->dvbtdt.nMinute, v->dvbtdt.nSecond);
								SendControlResponse(szTemp, lstrlen(szTemp));
								szString = " </STT>\r\n";
								SendControlResponse(szString, lstrlen(szString));
							}
							break;
						}
					}
					break;
				default:
					if (nCurrentlyDoing != SI_PARSER_SDT)
						CS__XMLFinishCurrentTable(&nCurrentlyDoing);
					break;
				}
				v->XMLLog[nXMLLogIndex].fSent = TRUE;
			}
		}
		LeaveCriticalSection(&v->csXMLLog);
		if (nCurrentlyDoing != SI_PARSER_NOP)
			CS__XMLFinishCurrentTable(&nCurrentlyDoing);

		if (fSentAtLeastOne)
		{
			szString = "</SI>\r\n\0";
			SendControlResponse(szString, lstrlen(szString) + 1);
		}

		// Thumbnails now
		fSentAtLeastOne = FALSE;
		EnterCriticalSection(&v->csXMLLog);
		for (nXMLLogIndex = 0; nXMLLogIndex < v->nXMLLogCount; nXMLLogIndex++)
		{
			if (v->XMLLog[nXMLLogIndex].fSent == FALSE && v->XMLLog[nXMLLogIndex].nXMLLogType == XML_LOG_TYPE_THUMBNAIL)
			{
				char szTemp[1024];

				if (!fSentAtLeastOne)
				{
					SendControlResponse(szXMLLeadin, lstrlen(szXMLLeadin));
					szString = "<THUMBS>\r\n";
					SendControlResponse(szString, lstrlen(szString));
					fSentAtLeastOne = TRUE;
				}
				
				wsprintf(szTemp, " <THUMB thumbnail=\"thumbnail\" program=\"%d\" filename=\"%s\" es_index=\"%d\" />\r\n",
					     v->XMLLog[nXMLLogIndex].nProgram,
					     v->XMLLog[nXMLLogIndex].szFilename,
					     v->XMLLog[nXMLLogIndex].nESIndex);
				SendControlResponse(szTemp, lstrlen(szTemp));
				v->XMLLog[nXMLLogIndex].fSent = TRUE;
			}
		}
		LeaveCriticalSection(&v->csXMLLog);
		if (fSentAtLeastOne)
		{
			szString = "</THUMBS>\r\n\0";
			SendControlResponse(szString, lstrlen(szString) + 1);
		}

		// Wait for the next time
		Sleep(100);
		if (GetTickCount() > dwNextStats)
		{
			dwNextStats = GetTickCount() + 1000;
			CS__XML_SendPIDs();
			CS__XML_SendStats();
			CS__XML_SendGeneralInfo();
		}
	}

	fXMLStreamRunning = FALSE;
	return FALSE;
}

void CS__XML(char * szXML)
{
	if (!v->fStreamingXMLMode)
	{
		static char szError[] = {"5xx Streaming XML mode not enabled\r\n"};
		SendControlResponse(szError, lstrlen(szError));
		return;
	}
	if (strstr(szXML, "stream='t'") != NULL)
	{
		if (!fXMLStreamRunning)
		{
			HANDLE hThread;
			DWORD dwThreadID;

			hThread = CreateThread(NULL, 0, ControlServerXMLStreamer, (LPVOID)0, 0, &dwThreadID);
			CloseHandle(hThread);
			fXMLStreamRunning = TRUE;
		}
	}
	else if (strstr(szXML, "stream='f'") != NULL)
	{
		if (fXMLStreamRunning)
		{
			fKillXMLStreamThread = TRUE;
			while (fXMLStreamRunning == TRUE)
				Sleep(50);
			fKillXMLStreamThread = FALSE;
		}
	}
	else if (strstr(szXML, "SIREFRESH") != NULL)
	{
		int i;

		EnterCriticalSection(&v->csXMLLog);
		for (i = 0; i < v->nXMLLogCount; i++)
			v->XMLLog[i].fSent = FALSE;
		LeaveCriticalSection(&v->csXMLLog);
	}
	else if (strstr(szXML, "select=") != NULL)
	{
		int nParameter;
		char * szParameterPtr = strstr(szXML, "select=") + 8;
		
		sscanf(szParameterPtr, "%x", &nParameter);	

		EnterCriticalSection(&v->csXMLLog);
		nDataRequestlParam = nParameter;
		fDataRequest = TRUE;
		LeaveCriticalSection(&v->csXMLLog);
	}
}
#endif PRO

void CS__Window(char * szParameters)
{
	static char szOK[] = {"316 WINDOW command sucessful\r\n"};

	if (szParameters == NULL)
	{
		static char szError[] = {"536 No parameters specified\r\n"};
		SendControlResponse(szError, lstrlen(szError));
		return;
	}
	strupr(szParameters);
	if (lstrcmp(szParameters, "MAXIMIZE") == 0)
	{
		ShowWindow(v->hWndMainWindow, SW_MAXIMIZE);
		SendControlResponse(szOK, lstrlen(szOK));
	}
	else if (lstrcmp(szParameters, "MINIMIZE") == 0)
	{
		ShowWindow(v->hWndMainWindow, SW_MINIMIZE);
		SendControlResponse(szOK, lstrlen(szOK));
	}
	else if (lstrcmp(szParameters, "RESTORE") == 0)
	{
		ShowWindow(v->hWndMainWindow, SW_RESTORE);
		SendControlResponse(szOK, lstrlen(szOK));
	}
	else if (lstrcmp(szParameters, "FOREGROUND") == 0)
	{
		SetForegroundWindow(v->hWndMainWindow);
		SendControlResponse(szOK, lstrlen(szOK));
	}
	else
	{
		static char szError[] = {"537 Incorrect parameter\r\n"};
		SendControlResponse(szError, lstrlen(szError));
	}
}

void CS__WriteThumbnail(char * szParameters)
{
	int nProgramNumber;
	int nPMTIndex, nESIndex;
	BOOL fFoundProgram = FALSE;
	BOOL fFoundVideo = FALSE;
	char * szFilenamePtr;
	HISDEST	hDestinationObject;
	char szCheckParameters[256];
	static char szThumbnailOK[] = {"318 Thumbnail command OK\r\n"};

	if (szParameters == NULL)
	{
		static char szError[] = {"528 No parameters specified\r\n"};
		SendControlResponse(szError, lstrlen(szError));
		return;
	}
	lstrcpy(szCheckParameters, szParameters);
	strlwr(szCheckParameters);
	if (lstrcmp(szCheckParameters, "off") == 0)
	{
		SendMessage(v->hWndMainWindow, WM_COMMAND, ID_SETTINGS_THUMBNAILTHREADPRIORITY_DISABLED, 0);
		SendControlResponse(szThumbnailOK, lstrlen(szThumbnailOK));
		return;
	}
	else if (lstrcmp(szCheckParameters, "low") == 0)
	{
		SendMessage(v->hWndMainWindow, WM_COMMAND, ID_SETTINGS_THUMBNAILTHREADPRIORITY_LOW, 0);
		SendControlResponse(szThumbnailOK, lstrlen(szThumbnailOK));
		return;
	}
	else if (lstrcmp(szCheckParameters, "normal") == 0)
	{
		SendMessage(v->hWndMainWindow, WM_COMMAND, ID_SETTINGS_THUMBNAILTHREADPRIORITY_NORMAL, 0);
		SendControlResponse(szThumbnailOK, lstrlen(szThumbnailOK));
		return;
	}
	else if (lstrcmp(szCheckParameters, "high") == 0)
	{
		SendMessage(v->hWndMainWindow, WM_COMMAND, ID_SETTINGS_THUMBNAILTHREADPRIORITY_HIGH, 0);
		SendControlResponse(szThumbnailOK, lstrlen(szThumbnailOK));
		return;
	}
	
	szFilenamePtr = strstr(szParameters, " ");
	if (szFilenamePtr == NULL)
	{
		static char szError[] = {"529 No filename or command specified\r\n"};
		SendControlResponse(szError, lstrlen(szError));
		return;
	}
	*szFilenamePtr = '\0';
	szFilenamePtr++;

	nProgramNumber = atoi(szParameters);
	if (nProgramNumber < 1 || nProgramNumber > 65535)
	{
		lstrcpy(szCheckParameters, szParameters);
		strlwr(szCheckParameters);
		if (lstrcmp(szCheckParameters, "refresh") == 0)
		{
			int nRefreshRate = atoi(szFilenamePtr);
			v->nESParsingCounterReload = nRefreshRate;
			v->nESParsingCounter = 0;
			SendControlResponse(szThumbnailOK, lstrlen(szThumbnailOK));
			return;
		}
		else
		{
			static char szError[] = {"530 Invalid program number range\r\n"};
			SendControlResponse(szError, lstrlen(szError));
			return;
		}
	}

	for (nPMTIndex = 0; nPMTIndex < MAX_PAT_ENTRIES; nPMTIndex++)
	{
		if (v->pat.pmt[nPMTIndex].nPMTPID == 0)
			break;
		if (v->pat.pmt[nPMTIndex].nProgramNumber == 0)
			continue;
		if (v->pat.pmt[nPMTIndex].nProgramNumber == nProgramNumber)
		{
			fFoundProgram = TRUE;
			break;
		}
	}
	if (!fFoundProgram)
	{
		static char szError[] = {"531 Invalid program number\r\n"};
		SendControlResponse(szError, lstrlen(szError));
		return;
	}

	EnterCriticalSection(&v->csThumbnails);
	for (nESIndex = 0; nESIndex < MAX_ESLIST_ENTRIES; nESIndex++)
	{
		if (v->pat.pmt[nPMTIndex].es[nESIndex].nESPID == 0)
			break;
		if (v->pat.pmt[nPMTIndex].es[nESIndex].nStreamType == 0x01			// MPEG-1
			|| v->pat.pmt[nPMTIndex].es[nESIndex].nStreamType == 0x02		// MPEG-2
			|| v->pat.pmt[nPMTIndex].es[nESIndex].nStreamType == 0x10		// MPEG-4			
			|| v->pat.pmt[nPMTIndex].es[nESIndex].nStreamType == 0x1b		// h.264
#ifdef PRO
			|| v->pat.pmt[nPMTIndex].es[nESIndex].nStreamType == 0xea		// VC1
#endif PRO
			|| v->pat.pmt[nPMTIndex].es[nESIndex].nStreamType == 0x80)		// DC-II
		{
			if (v->pat.pmt[nPMTIndex].es[nESIndex].pRGBVideoFrame != NULL)
			{
				fFoundVideo = TRUE;
				break;
			}
		}
	}
	if (!fFoundVideo)
	{
		static char szError[] = {"532 No video thumbnail exists for the program\r\n"};
		LeaveCriticalSection(&v->csThumbnails);
		SendControlResponse(szError, lstrlen(szError));
		return;
	}
	if (v->pat.pmt[nPMTIndex].es[nESIndex].fDecoderCrashed)
	{
		static char szError[] = {"555 Thumbnail decoder crashed\r\n"};
		LeaveCriticalSection(&v->csThumbnails);
		SendControlResponse(szError, lstrlen(szError));
		return;
	}

	hDestinationObject = _ISOpenFileDest(szFilenamePtr);
	if (hDestinationObject != NULL)
	{
		static char szOK[] = {"313 THUMBNAIL command sucessful\r\n"};

		_ISWriteRGBToJPG(hDestinationObject,
						 v->pat.pmt[nPMTIndex].es[nESIndex].pRGBVideoFrame,
						 v->pat.pmt[nPMTIndex].es[nESIndex].nVideoWidth,
						 v->pat.pmt[nPMTIndex].es[nESIndex].nVideoHeight,
						 100,
						 0);
		LeaveCriticalSection(&v->csThumbnails);
		_ISCloseDest(hDestinationObject);
		SendControlResponse(szOK, lstrlen(szOK));
	}
	else
	{
		static char szError[] = {"533 Unable to write thumbnail file\r\n"};
		LeaveCriticalSection(&v->csThumbnails);
		SendControlResponse(szError, lstrlen(szError));
	}
}

void CS__Graph(char * szParameters)
{
	BOOL fChartOK = FALSE;
	int nChartOption;
	char * szSpace;
	static int nChartIndex[] = {0,								// 0-hide any current graph
							    ID_VIEW_ACTIVEPIDSBYRATE,		// 1-Active PIDs by rate 
								ID_VIEW_ACTIVEPIDSBYPID,		// 2-Active PIDs by PID
								ID_VIEW_PIDUSAGE2DPIECHART,		// 3-PID usage 2D pie
								ID_VIEW_PIDPIECHART,			// 4-PID usage 3D pie
								ID_VIEW_MUXUSAGEAREACHART,		// 5-Mux usage stacked area
								ID_VIEW_MUXUSAGELINECHART,		// 6-Mux usage line
								ID_VIEW_VIDEOBITRATEAREACHART,	// 7-Video rate area
								ID_VIEW_VIDEOBITRATECHART,		// 8-Video rate line
								ID_VIEW_CHART_PROGRAMUSAGESTACKEDBARCHART, // 9-Program usage
								ID_VIEW_CHART_VIDEOCOMPOSITIONCHART,	// 10-Video composition
								ID_VIEW_CHART_SIGNALCHART};		// 11 - Signal 

	if (szParameters == NULL)
	{
		static char szError[] = {"523 No parameters specified\r\n"};
		SendControlResponse(szError, lstrlen(szError));
		return;
	}

	szSpace = strstr(szParameters, " ");
	if (szSpace != NULL)
		*szSpace = '\0';
	strupr(szParameters);
	if (lstrcmp(szParameters, "REALTIME") == 0)
	{
		static char szOK[] = {"319 Graph mode set to realtime\r\n"};
		v->fRealtimeCharting = TRUE;
		CheckMenuItem(GetMenu(v->hWndMainWindow), ID_SETTINGS_REALTIMECHARTING, MF_CHECKED | MF_BYCOMMAND);
		SendControlResponse(szOK, lstrlen(szOK));
		return;
	}
	else if (lstrcmp(szParameters, "AVERAGE") == 0)
	{
		static char szOK[] = {"320 Graph mode set to average\r\n"};
		v->fRealtimeCharting = FALSE;
		CheckMenuItem(GetMenu(v->hWndMainWindow), ID_SETTINGS_REALTIMECHARTING, MF_UNCHECKED | MF_BYCOMMAND);
		SendControlResponse(szOK, lstrlen(szOK));
		return;
	}
	else if (lstrcmp(szParameters, "REFRESH") == 0)
	{
		static char szOK[] = {"321 Graph refresh rate updated\r\n"};

		if (szSpace == NULL)
		{
			static char szError[] = {"538 No refresh rate specified\r\n"};
			SendControlResponse(szError, lstrlen(szError));
			return;
		}
		v->nGraphRefreshRate = atoi(szSpace + 1);
		SendControlResponse(szOK, lstrlen(szOK));
		return;
	}

	// Not a command - see if it's a valid number to display chart
	nChartOption = atoi(szParameters);
	switch(nChartOption)
	{
	case 0:			// hide current graph
		{
			int i;		

			for (i = 0; i < MAX_CHARTS; i++)
			{
				if (v->hWndChart[i] != NULL)
				{
					SendMessage(v->hWndChart[i], WM_CLOSE, 0, 0);
					v->hWndChart[i] = NULL;
				}
			}
			fChartOK = TRUE;
		}
		break;
	default:
		if (nChartOption >= 1 && nChartOption <= 11)
		{
			PostMessage(v->hWndMainWindow, WM_COMMAND, nChartIndex[nChartOption], 0);
			fChartOK = TRUE;
		}
		break;
	}
	if (fChartOK)
	{
		static char szOK[] = {"310 GRAPH command sucessful\r\n"};
		SendControlResponse(szOK, lstrlen(szOK));
	}
	else
	{
		static char szError[] = {"524 Invalid mode specified for GRAPH command\r\n"};
		SendControlResponse(szError, lstrlen(szError));
	}
}

void CS__Tune(char * szParameters)
{
	BOOL (* ParseCommandLine) (PSOURCESTRUCT ss, char * szCommandLine, BOOL fQuiet);
	static char szSourceParameters[256];

	if (szParameters == NULL)
	{
		static char szError[] = {"521 No parameters specified\r\n"};
		SendControlResponse(szError, lstrlen(szError));
		return;
	}

	// Stop current processing
	RestartTSReader_Stop(v->hWndMainWindow);

	// Switch source if we need to
	if (lstrlen(szNewSourceName))
	{
		FreeLibrary(v->hSource);
		lstrcpy(v->szSourceName, szNewSourceName);
		LoadSource(v->hWndMainWindow);
	}

	// Send command-line over
	ParseCommandLine = (td_ParseCommandLine)GetProcAddress(v->hSource, "TSReader_ParseCommandLine");
	lstrcpy(szSourceParameters, szParameters);
	if (ParseCommandLine(&v->ss, szSourceParameters, TRUE) == FALSE)
	{
		static char szError[] = {"522 Source reported error in parameters\r\n"};
		SendControlResponse(szError, lstrlen(szError));
		return;
	}
	
	// Now restart
	v->ss.fQuietMode = TRUE;
	v->fTuneFromControlServer = TRUE;
	RestartTSReader_Start(v->hWndMainWindow);
	{
		static char szOK[] = {"308 Source restarted\r\n"};
		SendControlResponse(szOK, lstrlen(szOK));
	}
}

void SendVLCConfigurationInfo(int nIndex)
{
	char szResponse[MAX_PATH];

	wsprintf(szResponse, "204 %02d %s\r\n205 %s\r\n", nIndex + 1, v->szVLCConfigDescription[nIndex], v->szVLCConfigCommand[nIndex]);
	SendControlResponse(szResponse, lstrlen(szResponse));
}

void SetMenuOption(BOOL * fValue, int nMenuID, char * szNewValue)
{
	HMENU hMenu = GetMenu(v->hWndMainWindow);
	
	if (szNewValue[0] == 'T')
	{
		*fValue = TRUE;
		CheckMenuItem(hMenu, nMenuID, MF_CHECKED | MF_BYCOMMAND);
	}
	else
	{
		*fValue = FALSE;
		CheckMenuItem(hMenu, nMenuID, MF_UNCHECKED | MF_BYCOMMAND);
	}
}

void CS__Setting(char * szParameters)
{
	if (szParameters == NULL)
	{
		char szResponse[256];

		wsprintf(szResponse, "210 SETTING IGNORE_EITSDT %s\r\n", TrueFalseString(v->fIgnoreEIT));
		SendControlResponse(szResponse, lstrlen(szResponse));
		wsprintf(szResponse, "210 SETTING SDT_CURRENT_MUX %s\r\n", TrueFalseString(v->fSDTOnlyForCurrentMux));
		SendControlResponse(szResponse, lstrlen(szResponse));
		wsprintf(szResponse, "210 SETTING PAST_EIT %s\r\n", TrueFalseString(v->fKeepPastEITData));
		SendControlResponse(szResponse, lstrlen(szResponse));
		wsprintf(szResponse, "210 SETTING IGNORE_CRC %s\r\n", TrueFalseString(v->fIgnoreTableCRCErrors));
		SendControlResponse(szResponse, lstrlen(szResponse));	
		wsprintf(szResponse, "210 SETTING IGNORE_DCIIPMT80 %s\r\n", TrueFalseString(v->fIgnorePMT800x0ff6));
		SendControlResponse(szResponse, lstrlen(szResponse));
		wsprintf(szResponse, "210 SETTING IGNORE_PMT_ABOVE_65500 %s\r\n", TrueFalseString(v->fIgnorePMT65500));
		SendControlResponse(szResponse, lstrlen(szResponse));
		wsprintf(szResponse, "210 SETTING PLAIN_CA_DESCRIPTORS %s\r\n", TrueFalseString(v->fPlainCADescriptors));
		SendControlResponse(szResponse, lstrlen(szResponse));
		wsprintf(szResponse, "210 SETTING KEEP_SPECIAL_XML %s\r\n", TrueFalseString(v->fKeepSpecialXMLCharacters));
		SendControlResponse(szResponse, lstrlen(szResponse));
	}
	else
	{
		int nParsed;
		char szSettingName[64];
		char szNewValue[64];
		char szOK[] = {"323 SETTING updated\r\n"};

		nParsed = sscanf(szParameters, "%s %s", szSettingName, szNewValue);
		if (nParsed != 2)
		{
			char szError[] = {"547 SETTING subparameter or value missing\r\n"};
			SendControlResponse(szError, lstrlen(szError));
			return;
		}
		strupr(szNewValue);
		if (szNewValue[0] != 'T' && szNewValue[0] != 'F')
		{
			char szError[] = {"548 SETTING true/false setting incorrect\r\n"};
			SendControlResponse(szError, lstrlen(szError));
			return;
		}
		strupr(szSettingName);
		
		if (lstrcmp(szSettingName, "IGNORE_EITSDT") == 0)
			SetMenuOption(&v->fIgnoreEIT, ID_SETTINGS_IGNOREEIT, szNewValue);
		else if (lstrcmp(szSettingName, "SDT_CURRENT_MUX") == 0)
			SetMenuOption(&v->fSDTOnlyForCurrentMux, ID_SETTINGS_SDTONLYFORCURRENTMUX, szNewValue);
		else if (lstrcmp(szSettingName, "PAST_EIT") == 0)
			SetMenuOption(&v->fKeepPastEITData, ID_SETTINGS_KEEPPASTEITDATA, szNewValue);
		else if (lstrcmp(szSettingName, "IGNORE_CRC") == 0)
			SetMenuOption(&v->fIgnoreTableCRCErrors, ID_SETTINGS_IGNORETABLECRCERRORS, szNewValue);
		else if (lstrcmp(szSettingName, "IGNORE_DCIIPMT80") == 0)
			SetMenuOption(&v->fIgnorePMT800x0ff6, ID_SETTINGS_IGNOREDCIIPMTCH80, szNewValue);		
		else if (lstrcmp(szSettingName, "IGNORE_PMT_ABOVE_65500") == 0)
			SetMenuOption(&v->fIgnorePMT65500, ID_SETTINGS_IGNOREPMTSABOVECH65500, szNewValue);
		else if (lstrcmp(szSettingName, "PLAIN_CA_DESCRIPTORS") == 0)
			SetMenuOption(&v->fPlainCADescriptors, ID_SETTINGS_PLAINCADESCRIPTORDECODING, szNewValue);
		else if (lstrcmp(szSettingName, "KEEP_SPECIAL_XML") == 0)
			SetMenuOption(&v->fKeepSpecialXMLCharacters, ID_SETTINGS_KEEPSPECIALXMLCHARACTERS, szNewValue);
		else
		{
			char szError[] = {"549 SETTING subparameter incorrect\r\n"};
			SendControlResponse(szError, lstrlen(szError));
			return;
		}
		SendControlResponse(szOK, lstrlen(szOK));
	}
}

void CS__SetVLC(char * szParameters)
{
	if (szParameters == NULL)
	{
		int i;
		for (i = 0; i < MAX_VLC_CONFIGURATIONS; i++)
		{
			SendVLCConfigurationInfo(i);
		}
	}
	else
	{
		int nVLCConfiguration = 0;
		int nConversionCount = sscanf(szParameters, "%d", &nVLCConfiguration);
		char * szSpace = strstr(szParameters, " ");
		
		if (nConversionCount != 1 || nVLCConfiguration < 1 || nVLCConfiguration > MAX_VLC_CONFIGURATIONS + 1)
		{
			static char szError[] = {"515 Invalid VLC configuration\r\n"};
			SendControlResponse(szError, lstrlen(szError));
			return;
		}
		if (szSpace == NULL)
		{
			SendVLCConfigurationInfo(nVLCConfiguration - 1);
		}
		else
		{
			static char szResponse[] = {"305 VLC command updated\r\n"};
			lstrcpy(v->szVLCConfigCommand[nVLCConfiguration - 1], szSpace + 1);
			SendControlResponse(szResponse, lstrlen(szResponse));
		}
	}
}

void CS__Source(char * szParameters)
{
	int i;
	char szCurrentDir[MAX_PATH];

	szNewSourceName[0] = '\0';

	GetModuleFileName(v->hInstance, szCurrentDir, sizeof(szCurrentDir));
	for (i = lstrlen(szCurrentDir); i > 0; i--)
	{
		if (szCurrentDir[i] == '\\')
		{
			szCurrentDir[i] = 0;
			break;
		}
	}

	if (szParameters != NULL)
	{
		HMODULE hSource;
		BOOL (* GetDescription) (char * szDescription, char * szCommandLineParameters, BOOL * fCanBeStopped, int * nMaxPIDs, DWORD * dwCapabilities);
		char szSourceName[MAX_PATH];

		lstrcpy(szSourceName, szCurrentDir);
		lstrcat(szSourceName, "\\Sources\\");
		lstrcat(szSourceName, szParameters);

		hSource = LoadLibrary(szSourceName);
		if (hSource == NULL)
		{
			char szError[] = {"518 Unable to load source file\r\n"};
			SendControlResponse(szError, lstrlen(szError));
			return;
		}
		GetDescription = (td_GetDescription)GetProcAddress(hSource, "TSReader_GetDescription");
		if (GetDescription != NULL)
		{
			char szOK[] = {"300 Source updated - use the TUNE command to restart TSReader\r\n"};
			lstrcpy(szNewSourceName, szSourceName);
			SendControlResponse(szOK, lstrlen(szOK));
		}
		else
		{
			char szError[] = {"519 Source specified isn't a TSReader source\r\n"};
			SendControlResponse(szError, lstrlen(szError));
		}
		FreeLibrary(hSource);
	}
	else
	{
		// Tell them the sources we have
		HANDLE hFind;
		char szCurrentFile[MAX_PATH];
		WIN32_FIND_DATA fd;

		sprintf(szCurrentFile, "%s\\Sources\\*.dll", szCurrentDir);
		hFind = FindFirstFile(szCurrentFile, &fd);
		if (hFind != INVALID_HANDLE_VALUE)
		{
			do
			{
				HMODULE hSource;
				char * szStartOfModuleName = strstr(fd.cFileName, "_");
				
				if (szStartOfModuleName != NULL)
				{			
					wsprintf(szCurrentFile, "%s\\Sources\\%s", szCurrentDir, fd.cFileName);
					if (strstr(szCurrentFile, "TSReader_TTBudget.dll") != NULL)
						DeleteFile(szCurrentFile);
					if (strstr(szCurrentFile, "TSReader_Twinhan2.dll") != NULL)
						DeleteFile(szCurrentFile);
					if (strstr(szCurrentFile, "TSReader_Twinhan.dll") != NULL)
						DeleteFile(szCurrentFile);
					
					hSource = LoadLibrary(szCurrentFile);
					if (hSource != NULL)
					{
						BOOL (* GetDescription) (char * szDescription, char * szCommandLineParameters, BOOL * fCanBeStopped, int * nMaxPIDs, DWORD * dwCapabilities);
						char szResponse[128];

						GetDescription = (td_GetDescription)GetProcAddress(hSource, "TSReader_GetDescription");
						if (GetDescription != NULL)
						{
							char szDescription[128] = {0};
							char szCommandLineParameters[128] = {0};
							BOOL fCanBeStopped;
							int nMaxPIDs;
							DWORD dwCapabilities;

//BOOL (* GetDescription) (char * szDescription, char * szCommandLineParameters, BOOL * fCanBeStopped, int * nMaxPIDs, DWORD * dwCapabilities);

							GetDescription(szDescription, szCommandLineParameters,  &fCanBeStopped, &nMaxPIDs, &dwCapabilities);
							wsprintf(szResponse, "203 %s", fd.cFileName);
							if (lstrcmp(v->szSourceName, szCurrentFile) == 0)
								lstrcat(szResponse, " *");
							lstrcat(szResponse, "\r\n");
							SendControlResponse(szResponse, lstrlen(szResponse));
						}
						FreeLibrary(hSource);
					}
				}
			} while (FindNextFile(hFind, &fd) == TRUE);
			FindClose(hFind);
		}
	}
}

void CS__Audio(char * szParameters)
{
	if (szParameters == NULL)
	{
		if (v->pat.hPATTreeItem != NULL || GetTotalPMTChannels())
		{
			int i;
			int nAudioIndex = 0;

			for (i = 0; i < MAX_ESLIST_ENTRIES; i++)
			{
				char szTemp[256];

				if (v->pat.pmt[v->nSelectedProgram].es[i].nESPID == 0)
					break;
				switch(v->pat.pmt[v->nSelectedProgram].es[i].nStreamType)
				{
				case 0x03:	// MPEG-1 audio
				case 0x04:	// MPEG-2 audio
				case 0x81:	// AC3 audio
				case 0x83:	// LPCM audio
				case 0x06:	// maybe AC3
					{
						char szStreamTypeEnglish[100];
						char szLanguage[16] = {0};
						if (v->pat.pmt[v->nSelectedProgram].es[i].nStreamType == 0x06)
						{
							if (IsAC3AudioStream(v->nSelectedProgram, i) == FALSE && IsPCMAudioStream(v->nSelectedProgram, i) == FALSE)
								break;
						}
						if (v->fAllowStradisAC3 == FALSE && IsAC3AudioStream(v->nSelectedProgram, i) == TRUE)
						{
							if (   (v->fRecordDialogStreamOnly == TRUE)
								&& (v->nStreamTo != STREAM_TO_DVHS)
								&& (v->nStreamTo != STREAM_TO_VLC)
								&& (v->pat.pmt[v->nSelectedProgram].es[i].nStreamType == 0x81) )
								break; // AC3 audio doesn't work on Stradis cards
						}
						DecodeStreamType(v->pat.pmt[v->nSelectedProgram].es[i].nStreamType, szStreamTypeEnglish, v->nSelectedProgram, i);

						wsprintf(szTemp, "206 %02d %s", nAudioIndex + 1, szStreamTypeEnglish);
						GetLanguageFromDescriptor(szLanguage, v->nSelectedProgram, i);
						if (lstrlen(szLanguage))
						{
							char szTemp2[16];
							wsprintf(szTemp2, " (%s)", szLanguage);
							lstrcat(szTemp, szTemp2);
						}
						lstrcat(szTemp, "\r\n");
						SendControlResponse(szTemp, lstrlen(szTemp));
						nAudioIndex++;
					}
				}
			}
		}
	}
	else
	{
		int nAutoRecordAudioTrack;
		static char szTemp[] = {"306 Audio stream set\r\n"};

		sscanf(szParameters, "%d", &nAutoRecordAudioTrack);
		if (nAutoRecordAudioTrack > 0 && nAutoRecordAudioTrack < 100)
		{
			static char szTemp[] = {"306 Audio stream set\r\n"};
			v->nAutoRecordAudioTrack = nAutoRecordAudioTrack;
			SendControlResponse(szTemp, lstrlen(szTemp));
		}
		else
		{
			static char szTemp[] = {"516 Audio stream value not valid\r\n"};
			SendControlResponse(szTemp, lstrlen(szTemp));
		}
	}
}

void CS__DiSEqC(char * szParameters)
{
	int nDiSEqCSequenceLength = 0;
	BYTE bDiSEqCSequence[16];

	BOOL (*SendDiSEqC) (BYTE * bCommand, int nLength);

	if ((v->dwSourceCapabilities & CAPABILITIES_DISEQC_POSITIONER) == 0)
	{
		static char szTemp[] = {"550 Source doesn't support DiSEqC positioner commands\r\n"};
		SendControlResponse(szTemp, lstrlen(szTemp));
		return;
	}

	SendDiSEqC = (td_SendDiSEqC)GetProcAddress(v->hSource, "TSReader_SendDiSEqC");
	if (SendDiSEqC == NULL)
	{
		static char szTemp[] = {"551 Source doesn't contain a DiSEqC entry-point\r\n"};
		SendControlResponse(szTemp, lstrlen(szTemp));
		return;
	}
	if (szParameters == NULL)
	{
		static char szTemp[] = {"552 No parameters\r\n"};
		SendControlResponse(szTemp, lstrlen(szTemp));
		return;
	}
	strupr(szParameters);
	do
	{
		if (*szParameters == '\0')
			break;
		if (*szParameters == ' ')
		{
			szParameters++;
			continue;
		}
		if (*szParameters >= '0' && *szParameters <= '9' ||
			*szParameters >= 'A' && *szParameters <= 'F')
		{
			int nValue;
			char szTemp[4];

			szTemp[0] = *szParameters;
			szTemp[1] = *(szParameters+1);
			szTemp[2] = '\0';
			sscanf(szTemp, "%x", &nValue);
			szParameters += 2;
			bDiSEqCSequence[nDiSEqCSequenceLength] = nValue;
			nDiSEqCSequenceLength++;
		}
		else
		{
			static char szTemp[] = {"553 Invalid DiSEqC sequence\r\n"};
			SendControlResponse(szTemp, lstrlen(szTemp));
			return;
		}
	} while (TRUE);

	if (nDiSEqCSequenceLength == 0)
	{
		static char szTemp[] = {"554 No DiSEqC message found\r\n"};
		SendControlResponse(szTemp, lstrlen(szTemp));
		return;
	}
	SendDiSEqC(bDiSEqCSequence, nDiSEqCSequenceLength);
	{
		static char szOK[] = {"324 DiSEqC sequence sent\r\n"};
		SendControlResponse(szOK, lstrlen(szOK));
	}
}

void CS__Stall(char * szParameters)
{
	BOOL fThumbnailMode = FALSE;
	int nStallTimeout, nStallTimeoutBackup;
	char * szSpace;

	if (szParameters == NULL)
	{
		static char szError[] = {"525 No parameters specified\r\n"};
		SendControlResponse(szError, lstrlen(szError));
		return;
	}
	sscanf(szParameters, "%d", &nStallTimeout);
	if (nStallTimeout <= 0)
	{
		static char szError[] = {"526 Invalid stall timeout\r\n"};
		SendControlResponse(szError, lstrlen(szError));
		return;
	}
	nStallTimeoutBackup = nStallTimeout;

	// Check for optional THUMBNAILS parameter
	szSpace = strstr(szParameters, " ");
	if (szSpace != NULL)
	{
		char szTemp[256];

		lstrcpy(szTemp, szSpace);
		strupr(szTemp);
		if (lstrcmp(szTemp, " THUMBNAILS") == 0)
			fThumbnailMode = TRUE;
	}

	// Wait for SI parsing to complete
	do
	{
		if (v->nTreeUpdateCounter2 == -1)
			break;
		Sleep(100);
		nStallTimeout--;
	} while (nStallTimeout);

	// Wait for Thumbnails
	if (fThumbnailMode == TRUE && nStallTimeout)
	{
		nStallTimeout = nStallTimeoutBackup;
		do
		{
			if (v->fCompletedESParsing == TRUE)
				break;
			Sleep(100);
			nStallTimeout--;
		} while (nStallTimeout);
	}

	if (nStallTimeout)
	{
		static char szOK1[] = {"311 Table decoding complete\r\n"};
		static char szOK2[] = {"311 Table and thumbnail decoding complete\r\n"};
		if (fThumbnailMode)
			SendControlResponse(szOK2, lstrlen(szOK2));
		else
			SendControlResponse(szOK1, lstrlen(szOK1));
	}
	else
	{
		static char szOK[] = {"312 STALL command timed-out\r\n"};
		SendControlResponse(szOK, lstrlen(szOK));
	}
}

void RemoveCommandBackspaces(char * szCommandBuffer)
{
	int nInputIndex, nOutputIndex;
	char szNewCommandBuffer[512];

	memset(szNewCommandBuffer, 0, sizeof(szNewCommandBuffer));
	nOutputIndex = 0;
	for (nInputIndex = 0; nInputIndex < lstrlen(szCommandBuffer); nInputIndex++)
	{
		if (szCommandBuffer[nInputIndex] != 0x08)
			szNewCommandBuffer[nOutputIndex++] = szCommandBuffer[nInputIndex];
		else
		{
			nOutputIndex--;
			if (nOutputIndex < 0)
				nOutputIndex = 0;
		}
	}
	lstrcpy(szCommandBuffer, szNewCommandBuffer);
}

DWORD WINAPI ControlServerThread(LPVOID lpv)
{
	char szTemp[128];
	SOCKADDR_IN acc_sin;
	int acc_sin_len;

	OutputDebugString("TSReader: +ControlServerThread\n");

	// Start listening
	if (listen(ControlBaseSocket, 100) < 0)
	{
		OutputDebugString("TSReader: listen(ControlBaseSocket) failed\n");
		closesocket(ControlBaseSocket);
		return 0;
	}
	fServerThreadActive = TRUE;

	do
	{
		char szCommandBuffer[1024];

		// Wait for a connection on port 1399
		acc_sin_len = sizeof(acc_sin);
		ControlSocket = accept(ControlBaseSocket, (struct sockaddr FAR *)&acc_sin, (int FAR *)&acc_sin_len);
		if (ControlSocket == INVALID_SOCKET)
		{
			OutputDebugString("TSReader: accept failed in ControlServer\n");
			break;
		}
		wsprintf(szTemp, "TSReader: Accepted control connection from %d.%d.%d.%d\n", acc_sin.sin_addr.S_un.S_un_b.s_b1,
			                                                            acc_sin.sin_addr.S_un.S_un_b.s_b2,
																		acc_sin.sin_addr.S_un.S_un_b.s_b3,
																		acc_sin.sin_addr.S_un.S_un_b.s_b4);
		OutputDebugString(szTemp);
		{
			int flag;
			flag = 1;
			setsockopt (ControlSocket, IPPROTO_TCP, TCP_NODELAY, (char *) &flag, sizeof(int));
			//flag = 2048;
			//setsockopt (gHTTPSocket, IPPROTO_TCP, TCP_MAXSEG, (char *) &flag, sizeof(int));
		}

#ifdef PRO
		{
			int i;

			EnterCriticalSection(&v->csXMLLog);
			for (i = 0; i < v->nXMLLogCount; i++)
				v->XMLLog[i].fSent = FALSE;
			LeaveCriticalSection(&v->csXMLLog);
		}
		if (!v->fStreamingXMLMode)
#endif PRO
		{
			wsprintf(szTemp, "200 TSReader version %s Control Server\r\n", GetTSRVersion(NULL));
			SendControlResponse(szTemp, lstrlen(szTemp));
		}

		// Loop for commands
		do
		{
#ifdef PRO
			BOOL fXML = FALSE;
#endif PRO
			int nLength;
			int nCommandLength = 0;
			char * szSpacePtr;
			char szCommand[128];

			memset(szCommandBuffer, 0, sizeof(szCommandBuffer));
			do
			{
				nLength = recv(ControlSocket, &szCommandBuffer[nCommandLength], 1, 0);
				if (nLength <= 0)
					break;
#ifdef PRO
				if (nCommandLength == 0 && szCommandBuffer[0] == '<')
					fXML = TRUE;
				if (fXML == FALSE)
				{
#endif PRO
					if (szCommandBuffer[nCommandLength] == '\r')
					{
						szCommandBuffer[nCommandLength] = '\0';
						break;
					}
					if (szCommandBuffer[nCommandLength] != '\n')
						nCommandLength++;
#ifdef PRO
				}
				else
				{	
					if (szCommandBuffer[nCommandLength] == '\0')
					{
						szCommandBuffer[nCommandLength] = '\0';
						break;
					}
					nCommandLength++;
				}
#endif PRO
			} while (nCommandLength < sizeof(szCommandBuffer));
			if (nLength <= 0)
				break;
			{
				char szTemp[1024];
				wsprintf(szTemp, "TSReader: Control server command:  %s\n", szCommandBuffer);
				OutputDebugString(szTemp);
			}
			RemoveCommandBackspaces(szCommandBuffer);
#ifdef PRO
			if (fXML == TRUE)
			{
				CS__XML(szCommandBuffer);
				continue;
			}
#endif PRO

			// Parse command
			szSpacePtr = strstr(szCommandBuffer, " ");
			if (szSpacePtr != NULL)
			{
				*szSpacePtr = '\0';
				szSpacePtr++;
			}
			lstrcpy(szCommand, szCommandBuffer);
			if (!lstrlen(szCommand))
				continue;
			strupr(szCommand);
			if      (lstrcmp(szCommand, "QUIT") == 0)				break;
			else if (lstrcmp(szCommand, "?") == 0)					CS__SendHelp(szSpacePtr);
			else if (lstrcmp(szCommand, "AUDIO") == 0)				CS__Audio(szSpacePtr);
			else if (lstrcmp(szCommand, "DISEQC") == 0)				CS__DiSEqC(szSpacePtr);
			else if (lstrcmp(szCommand, "EXPORT") == 0)				CS__Export(szSpacePtr);
			else if (lstrcmp(szCommand, "HELP") == 0)				CS__SendHelp(szSpacePtr);
			else if (lstrcmp(szCommand, "INFO") == 0)				CS__GetInfo(szSpacePtr);
			else if (lstrcmp(szCommand, "MANUALCHANNEL") == 0)		CS__ManualChannel(szSpacePtr);
			else if (lstrcmp(szCommand, "PIDS") == 0)				CS__PIDs(szSpacePtr);
			else if (lstrcmp(szCommand, "PLAY") == 0)				CS__Play(szSpacePtr);
			else if (lstrcmp(szCommand, "PROGRAM") == 0)			CS__Program(szSpacePtr);
			else if (lstrcmp(szCommand, "RECORD") == 0)				CS__Record(szSpacePtr);
			else if (lstrcmp(szCommand, "RESET") == 0)				CS__Reset(szSpacePtr);
			else if (lstrcmp(szCommand, "SETTING") == 0)			CS__Setting(szSpacePtr);
			else if (lstrcmp(szCommand, "SETVLC") == 0)				CS__SetVLC(szSpacePtr);
			else if (lstrcmp(szCommand, "SOURCE") == 0)				CS__Source(szSpacePtr);
			else if (lstrcmp(szCommand, "STALL") == 0)				CS__Stall(szSpacePtr);
			else if (lstrcmp(szCommand, "STOP") == 0)				CS__Stop(szSpacePtr);
			else if (lstrcmp(szCommand, "TERMINATE") == 0)			CS__TerminateTSReader(szSpacePtr);
			else if (lstrcmp(szCommand, "TUNE") == 0)				CS__Tune(szSpacePtr);
			else if (lstrcmp(szCommand, "GRAPH") == 0)				CS__Graph(szSpacePtr);
			else if (lstrcmp(szCommand, "THUMBNAIL") == 0)			CS__WriteThumbnail(szSpacePtr);
			else if (lstrcmp(szCommand, "WINDOW") == 0)				CS__Window(szSpacePtr);
			else
			{
				static char szUnrecognizedCommand[] = {"500 Command unrecognized\r\n"};
				SendControlResponse(szUnrecognizedCommand, lstrlen(szUnrecognizedCommand));
			}
		} while (TRUE);
		closesocket(ControlSocket);
		ControlSocket = INVALID_SOCKET;
	} while (TRUE);

	OutputDebugString("TSReader: -ControlServerThread\n");
	return 0;
}

BOOL StartControlServer()
{
	HANDLE hThread;
	DWORD dwThreadID;
	SOCKADDR_IN local_sin;

	ControlBaseSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (ControlBaseSocket == INVALID_SOCKET)
		return FALSE;

	// Setup for TCP/IP on port 1400
	local_sin.sin_family = AF_INET;
	local_sin.sin_addr.s_addr = INADDR_ANY;
	local_sin.sin_port = htons((short)v->nControlServerPort);

	// Bind the socket
	if (bind(ControlBaseSocket, (struct sockaddr FAR *) &local_sin, sizeof(local_sin)) == SOCKET_ERROR)
	{
		closesocket(ControlBaseSocket);
		return FALSE;
	}

	// Thread to accept and manage the HTTP socket connection
	hThread = CreateThread(NULL, 0, ControlServerThread, (LPVOID)0, 0, &dwThreadID);
	CloseHandle(hThread);

	return TRUE;

}

BOOL TerminateControlServer()
{

	if (ControlBaseSocket != INVALID_SOCKET)
	{
		closesocket(ControlBaseSocket);
		ControlBaseSocket = INVALID_SOCKET;
	}
	if (ControlSocket != INVALID_SOCKET)
	{
		closesocket(ControlSocket);
		ControlSocket = INVALID_SOCKET;
	}
	
	return TRUE;
}

#endif LITE
