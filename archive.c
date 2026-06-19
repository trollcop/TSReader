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

int BuildTSSection(BYTE * pInput, int nInputLength, BYTE * pOutput, int nMaxOutput, int nPID);
BOOL LocateCurrentProgram(int nPMTIndex, char * szCurrentProgram, char * szDescription,
						  SYSTEMTIME * stStart, SYSTEMTIME * stDuration, BOOL fForDisplay);
void CursorNormal();
void CursorWait(HWND hWnd);
DWORD WINAPI LaunchVLCThread(LPVOID lpv);
DWORD WINAPI EmailThread(LPVOID pPtr);

#define MAX_ARCHIVE_PID_STREAMS 32
#define EPG_TYPE_LOCAL_EPG 0
#define EPG_TYPE_REMOTE_EPG 1
#define EPG_TYPE_30_MIN 2
#define MAX_HISTORIC_SAMPLES 1600
#define MAX_ASYNC_IO 64
#define MAX_THUMBNAILS 60 * 24
#define MAX_EVENTS 100
#define MAX_CAPTION_CHANNELS 8

#define STATUS_UNKNOWN 0
#define STATUS_NORMAL 1
#define STATUS_SCRAMBLED 2
#define STATUS_NO_DATA 4
#define STATUS_NO_ASYNC_IO 8

#define ERROR_BEEP_MPEG 1
#define ERROR_BEEP_DISK_SPACE 2

typedef struct _tagArchivedThumbnails
{
	SYSTEMTIME stDate;
	LARGE_INTEGER lnByteOffset;
	char szFilename[96];
} ARCHIVEDTHUMBNAILS, *PARCHIVEDTHUMBNAILS;

typedef struct _tagArchivedPrograms
{
	char szStartDate[16], szStartTime[16];
	char szDuration[16];
	char szChannel[64];
	char szProgram[256];
	char szDescription[4096];
	char szRating[64];
	char szMPEGFile[MAX_PATH];
	ARCHIVEDTHUMBNAILS at[MAX_THUMBNAILS];

} ARCHIVEDPROGRAMS, *PARCHIVEDPROGRAMS;

typedef struct _tagArchiveGOPBuffer
{
	int nUserDataLength;
	BYTE bUserData[128];
	BYTE fActive;
} ARCHIVEGOPBUFFER;

typedef struct _tagThumbnailList
{
	BOOL fActive;
	LARGE_INTEGER lnByteOffset;
	SYSTEMTIME st;
	char szFilename[96];
} THUMBNAILLIST, *PTHUMBNAILLIST;

typedef struct _tagArchiveProgram
{
	LARGE_INTEGER lnCurrentFileWritePtr;

	BOOL fEnabled;
	BOOL fUserDataArmed;
	BOOL fPreviouslyLocalEPG;
	BOOL fPreviouslyRemoteEPG;
	BOOL fOldArchiveEPGServer;
	BOOL fOldArchiveEPGGotChannelName;
	BOOL fXDSActive;
	BOOL fRemoteEPGThreadRunning;
	BOOL fNewRemoteProgramDetected;
	
	int nWriteBufferOffset[MAX_ASYNC_IO];
	int nEPGType;
	int nStatus;
	int nGraphBytes, nGraphBytesMin, nGraphBytesMax;
	int nLastSampleGraphBytes;
	int nHistoricLastSampleGraphBytes[MAX_HISTORIC_SAMPLES];
	int nCurrentAsyncIO;
	int nVideoPID, nVideoPESLength, nVideoESFillPtr;
	int nSubtitlePID, nSubtitlePESLength, nSubtitleESFillPtr;
	int nTemporalRererence, nPictureCodingType;
	int nGOPOutputPos;
	int nNoDataCounter;
	int nCaptionOutputPosition[MAX_CAPTION_CHANNELS];
	int nCaptionProgramBytes[MAX_CAPTION_CHANNELS];
	int nCaptionCurrentOutputFileIndex[2];
	int nUserDataBytes;
	int nPMTPackets;
	int nPMTPacketOutputCounter;
	int nPMTOutputContinuityCounter;
	int nSyncLossCount;
	int nContinuityErrorCount, nTEIErrorCount;
	int nRemoteEPGReconnectCounter;

	BYTE * pBlockBuffer[MAX_ASYNC_IO];

	HANDLE hOutputFile;
	HANDLE hUserDataOutputFile;
	HANDLE hCaptionOutputFile[MAX_CAPTION_CHANNELS];
	HANDLE hPriorOutputFile;
	HANDLE hAsyncOutputFile[MAX_ASYNC_IO];

	double dCurrentFileWritten;
	double dAllBytesWritten;
	double dGraphBytesTotal;
	double dGraphBytesTotalCount;

	SYSTEMTIME stCurrentFileRun;
	SYSTEMTIME stCurrentStart, stCurrentDuration;
	SYSTEMTIME stNextStart, stNextDuration;
	SYSTEMTIME stActualStart;

	char szChannelName[64];
	char szOutputLocation[MAX_PATH];
	char szOutputFileName[MAX_PATH];
	char szRemoteEPGServer[256];
	char szCurrentProgram[1024];
	char szCurrentDescription[4096];
	char szCurrentRating[64];
	char szNextProgram[1024];
	char szNextDescription[4096];
	char szNextRating[64];
	char szUserDataOutputFileName[MAX_PATH];
	char szCaptionOutputFilename[MAX_CAPTION_CHANNELS][MAX_PATH];
	
	BYTE pat[188];
	BYTE pmt[20 * 188];

	OVERLAPPED overlappedWrites[MAX_ASYNC_IO];
	CRITICAL_SECTION csThumbnailList;
	PTHUMBNAILLIST tl[MAX_THUMBNAILS];
	ARCHIVEGOPBUFFER gopbuffer[MAX_GOP_SIZE];
	SOCKET EPGSocket;

	BYTE bVideoESBuffer[ES_BUFFER_SIZE];
	BYTE bSubtitleESBuffer[ES_BUFFER_SIZE];

} ARCHIVEPROGRAM, *PARCHIVEPROGRAM;

typedef struct _tagArchive
{
	int nListSelectedProgram;
	int nListSelectedPMT;
	int nBlockSize;
	int nPipeBytes;
	int nMaxIOsPending;
	int nCurrentThumbnailDisplay;
	int nThumbnailCounter;
	int nThumbnailWidth, nThumbnailHeight;
	int nErrorBeep, nErrorPMTIndex;
	int nAutoRemoveSizeGB;
	int nWarningGB, nErrorGB;
	int nAutoRemoveOldFiles;

	DWORD dwEventIDCounter;

	BOOL fReadyForData;
	BOOL fForceNew;
	BOOL fNeedNewThumbnail;
	BOOL fBeepThreadRunning;
	BOOL fTerminateBeepThread;
	BOOL fSilent;
	BOOL fAutoRestart;
	BOOL fTerminateGraphThread;
	BOOL fLocalTime;

	HWND hWndStatus;
	HWND hWndLVEvent;
	HFONT hSmallTextFont;
	HFONT hSmallTextFontBold;
	HBRUSH hGreenBrush;
	HBRUSH hRedBrush;
	HPEN hGreenPen;
	HPEN hYellowPen;
	HIMAGELIST ghimlEventSmall;
	HICON hStatusIcon[3];

	BYTE * pThumbnailRGB;

	CRITICAL_SECTION csEventLog;
	CRITICAL_SECTION csPipeBytes;
	CRITICAL_SECTION csEventList;

	SYSTEMTIME stTimeCheck;
	SYSTEMTIME stTotalRun;

	PARCHIVEPROGRAM ap[MAX_PAT_ENTRIES];

	EVENTLIST eventlist[MAX_EVENTS];

	char szAutoRemoveLocation[MAX_PATH];
	char szAlarmSoundFilename[MAX_PATH];

	BYTE nArchivePIDs[8192][MAX_ARCHIVE_PID_STREAMS];
	BYTE bActiveDrive[26];
	BYTE bFileRemovalActive[26];
	
	double dDriveFreeSpace[26];

} ARCHIVE, *PARCHIVE;

PARCHIVE archive;
PARCHIVEDPROGRAMS arcprogs;

int GetProgramCCorTEICount(int nPMTIndex, BOOL fReturnCCCount)
{
	int nCount = 0;
	int nESIndex;

	for (nESIndex = 0; nESIndex < MAX_ESLIST_ENTRIES; nESIndex++)
	{
		if (v->pat.pmt[nPMTIndex].es[nESIndex].nESPID == 0)
			break;
		EnterCriticalSection(&v->ss.csPIDCounter);
		if (fReturnCCCount)
			nCount += v->nPIDHasContinuityErrors[v->pat.pmt[nPMTIndex].es[nESIndex].nESPID];
		else
			nCount += v->nPIDTEICount[v->pat.pmt[nPMTIndex].es[nESIndex].nESPID];
		LeaveCriticalSection(&v->ss.csPIDCounter);
	}
	
	return nCount;
}

void ArchiveEventListWrite(int nIcon, char * szShortStatus, char * szLongStatus)
{
	int i;

	EnterCriticalSection(&archive->csEventList);
	// move all the items down the list
	for (i = MAX_EVENTS - 1; i > 0; i--)
		memcpy(&archive->eventlist[i], &archive->eventlist[i - 1], sizeof(EVENTLIST));
	archive->eventlist[0].nEventType = nIcon;
	GetSystemTime(&archive->eventlist[0].stEvent);
	lstrcpy(archive->eventlist[0].szShort, szShortStatus);
	lstrcpy(archive->eventlist[0].szLong, szLongStatus);
	LeaveCriticalSection(&archive->csEventList);
	InvalidateRect(archive->hWndStatus, NULL, FALSE);

	if (nIcon > EVENT_ICON_INFORMATION)
	{
		DWORD dwThreadID;
		HANDLE hThread;
		PEVENTEMAILITEM pEmail = LocalAlloc(LPTR, sizeof(EVENTEMAILITEM));

		memcpy(&pEmail->el, &archive->eventlist[0], sizeof(EVENTLIST));
		lstrcpy(pEmail->szSMTPServer, v->szSMTPServer);
		lstrcpy(pEmail->szEmailAddress, v->szEmailDestination);
		lstrcpy(pEmail->szEmailFrom, v->szEmailFrom);

		hThread = CreateThread(NULL, 0, EmailThread, (LPVOID)pEmail, CREATE_SUSPENDED, &dwThreadID);
		SetThreadPriority(hThread, THREAD_PRIORITY_BELOW_NORMAL);
		ResumeThread(hThread);	
		CloseHandle(hThread);
	}
}

void LoadArchiveSettings()
{
	DWORD dwDisposition;
	DWORD dwDataSize;
	DWORD dwType;
	LONG lKey;
	HKEY hkMainReg;
	char szTemp[128] = {0};
	char szKeyName[MAX_PATH];

	lstrcpy(szKeyName, gszKeyName);
	if (lstrlen(v->szProfileName))
	{
		lstrcat(szKeyName, "-");
		lstrcat(szKeyName, v->szProfileName);
	}
	lstrcat(szKeyName, "\\Archive");

	lKey = RegCreateKeyEx(v->hRegistryRoot,
		                  szKeyName,
						  0,
						  gszAppName,
					      REG_OPTION_NON_VOLATILE,
						  KEY_ALL_ACCESS,
						  NULL,
						  &hkMainReg,
						  &dwDisposition);
	if (lKey == ERROR_SUCCESS)
	{
		int nPMTIndex;

		if (dwDisposition != REG_CREATED_NEW_KEY)
		{
			// Load channel settings
			for (nPMTIndex = 0; nPMTIndex < MAX_PAT_ENTRIES; nPMTIndex++)
			{
				if (v->pat.pmt[nPMTIndex].nPMTPID == 0)
					break;
				if (v->pat.pmt[nPMTIndex].nProgramNumber != 0)
				{
					char szValueName[128];

					wsprintf(szValueName, "%05d_OutputLocation", v->pat.pmt[nPMTIndex].nProgramNumber);
					dwDataSize = sizeof(archive->ap[nPMTIndex]->szOutputLocation);
					RegQueryValueEx(hkMainReg, szValueName, NULL, &dwType, (BYTE *)archive->ap[nPMTIndex]->szOutputLocation, &dwDataSize);

					wsprintf(szValueName, "%05d_ChannelName", v->pat.pmt[nPMTIndex].nProgramNumber);
					dwDataSize = sizeof(archive->ap[nPMTIndex]->szChannelName);
					RegQueryValueEx(hkMainReg, szValueName, NULL, &dwType, (BYTE *)archive->ap[nPMTIndex]->szChannelName, &dwDataSize);

					wsprintf(szValueName, "%05d_Enabled", v->pat.pmt[nPMTIndex].nProgramNumber);
					dwDataSize = sizeof(archive->ap[nPMTIndex]->fEnabled);
					RegQueryValueEx(hkMainReg, szValueName, NULL, &dwType, (BYTE *)&archive->ap[nPMTIndex]->fEnabled, &dwDataSize);

					wsprintf(szValueName, "%05d_EPGType", v->pat.pmt[nPMTIndex].nProgramNumber);
					dwDataSize = sizeof(archive->ap[nPMTIndex]->nEPGType);
					RegQueryValueEx(hkMainReg, szValueName, NULL, &dwType, (BYTE *)&archive->ap[nPMTIndex]->nEPGType, &dwDataSize);

					wsprintf(szValueName, "%05d_EPGServer", v->pat.pmt[nPMTIndex].nProgramNumber);
					dwDataSize = sizeof(archive->ap[nPMTIndex]->szRemoteEPGServer);
					RegQueryValueEx(hkMainReg, szValueName, NULL, &dwType, (BYTE *)&archive->ap[nPMTIndex]->szRemoteEPGServer, &dwDataSize);

				}
			}

			// Load other settings
			dwDataSize = sizeof(archive->nBlockSize);
			RegQueryValueEx(hkMainReg, "BlockSize", NULL, &dwType, (BYTE *)&archive->nBlockSize, &dwDataSize);	
			dwDataSize = sizeof(archive->nAutoRemoveOldFiles);
			RegQueryValueEx(hkMainReg, "AutoRemoveOldFiles", NULL, &dwType, (BYTE *)&archive->nAutoRemoveOldFiles, &dwDataSize);	
			dwDataSize = sizeof(archive->nAutoRemoveSizeGB);
			RegQueryValueEx(hkMainReg, "AutoRemoveSizeGB", NULL, &dwType, (BYTE *)&archive->nAutoRemoveSizeGB, &dwDataSize);				
			dwDataSize = sizeof(archive->fLocalTime);
			RegQueryValueEx(hkMainReg, "LocalTime", NULL, &dwType, (BYTE *)&archive->fLocalTime, &dwDataSize);						
			dwDataSize = sizeof(archive->fSilent);
			RegQueryValueEx(hkMainReg, "Silent", NULL, &dwType, (BYTE *)&archive->fSilent, &dwDataSize);			
			dwDataSize = sizeof(archive->nWarningGB);
			RegQueryValueEx(hkMainReg, "WarningGB", NULL, &dwType, (BYTE *)&archive->nWarningGB, &dwDataSize);			
			dwDataSize = sizeof(archive->nErrorGB);
			RegQueryValueEx(hkMainReg, "ErrorGB", NULL, &dwType, (BYTE *)&archive->nErrorGB, &dwDataSize);			
			dwDataSize = sizeof(archive->szAutoRemoveLocation);
			RegQueryValueEx(hkMainReg, "AutoRemoveLocation", NULL, &dwType, (BYTE *)&archive->szAutoRemoveLocation, &dwDataSize);			
			dwDataSize = sizeof(archive->fAutoRestart);
			RegQueryValueEx(hkMainReg, "AutoRestart", NULL, &dwType, (BYTE *)&archive->fAutoRestart, &dwDataSize);			
			dwDataSize = sizeof(archive->szAlarmSoundFilename);
			RegQueryValueEx(hkMainReg, "AlarmSoundFilename", NULL, &dwType, (BYTE *)&archive->szAlarmSoundFilename, &dwDataSize);			
			if (lstrlen(archive->szAlarmSoundFilename) == 0)
			{
				SourceHelper_GetTSReaderEXEDirectory(v->hInstance, archive->szAlarmSoundFilename, sizeof(archive->szAlarmSoundFilename));
				lstrcat(archive->szAlarmSoundFilename, "\\archive-siren.wav");
			}
		}
		else
		{
			// First time - enable everything
			for (nPMTIndex = 0; nPMTIndex < MAX_PAT_ENTRIES; nPMTIndex++)
			{
				if (v->pat.pmt[nPMTIndex].nPMTPID == 0)
					break;
				if (v->pat.pmt[nPMTIndex].nProgramNumber != 0)
				{
					archive->ap[nPMTIndex]->fEnabled = TRUE;
					lstrcpy(archive->ap[nPMTIndex]->szOutputLocation, "c:\\");
				}
			}

			// Other first time settings
			archive->nBlockSize = 64;
		}
		RegCloseKey(hkMainReg);
	}
}

void SaveArchiveSettings()
{
	LONG lKey;
	HKEY hkMainReg;
	DWORD dwDisposition;
	char szKeyName[MAX_PATH];

	lstrcpy(szKeyName, gszKeyName);
	if (lstrlen(v->szProfileName))
	{
		lstrcat(szKeyName, "-");
		lstrcat(szKeyName, v->szProfileName);
	}
	lstrcat(szKeyName, "\\Archive");

	lKey = RegCreateKeyEx(v->hRegistryRoot,
		                  szKeyName,
						  0,
						  gszAppName,
					      REG_OPTION_NON_VOLATILE,
						  KEY_ALL_ACCESS,
						  NULL,
						  &hkMainReg,
						  &dwDisposition);
	if (lKey == ERROR_SUCCESS)
	{
		int nPMTIndex;

		// Save channel settings
		for (nPMTIndex = 0; nPMTIndex < MAX_PAT_ENTRIES; nPMTIndex++)
		{
			if (v->pat.pmt[nPMTIndex].nPMTPID == 0)
				break;
			if (v->pat.pmt[nPMTIndex].nProgramNumber != 0)
			{
				char szValueName[128];

				wsprintf(szValueName, "%05d_OutputLocation", v->pat.pmt[nPMTIndex].nProgramNumber);
				RegSetValueEx(hkMainReg, szValueName, 0, REG_SZ, (BYTE *)archive->ap[nPMTIndex]->szOutputLocation, lstrlen(archive->ap[nPMTIndex]->szOutputLocation) + 1);

				wsprintf(szValueName, "%05d_ChannelName", v->pat.pmt[nPMTIndex].nProgramNumber);
				RegSetValueEx(hkMainReg, szValueName, 0, REG_SZ, (BYTE *)archive->ap[nPMTIndex]->szChannelName, lstrlen(archive->ap[nPMTIndex]->szChannelName) + 1);

				wsprintf(szValueName, "%05d_Enabled", v->pat.pmt[nPMTIndex].nProgramNumber);
				RegSetValueEx(hkMainReg, szValueName, 0, REG_DWORD, (BYTE *)&archive->ap[nPMTIndex]->fEnabled, sizeof(DWORD));

				wsprintf(szValueName, "%05d_EPGType", v->pat.pmt[nPMTIndex].nProgramNumber);
				RegSetValueEx(hkMainReg, szValueName, 0, REG_DWORD, (BYTE *)&archive->ap[nPMTIndex]->nEPGType, sizeof(DWORD));

				wsprintf(szValueName, "%05d_EPGServer", v->pat.pmt[nPMTIndex].nProgramNumber);
				RegSetValueEx(hkMainReg, szValueName, 0, REG_SZ, (BYTE *)archive->ap[nPMTIndex]->szRemoteEPGServer, lstrlen(archive->ap[nPMTIndex]->szRemoteEPGServer) + 1);
			}
		}

		// Save global settings
		RegSetValueEx(hkMainReg, "BlockSize", 0, REG_DWORD, (BYTE *)&archive->nBlockSize, sizeof(DWORD));
		RegSetValueEx(hkMainReg, "AutoRemoveOldFiles", 0, REG_DWORD, (BYTE *)&archive->nAutoRemoveOldFiles, sizeof(DWORD));
		RegSetValueEx(hkMainReg, "AutoRemoveSizeGB", 0, REG_DWORD, (BYTE *)&archive->nAutoRemoveSizeGB, sizeof(DWORD));		
		RegSetValueEx(hkMainReg, "LocalTime", 0, REG_DWORD, (BYTE *)&archive->fLocalTime, sizeof(DWORD));
		RegSetValueEx(hkMainReg, "Silent", 0, REG_DWORD, (BYTE *)&archive->fSilent, sizeof(DWORD));
		RegSetValueEx(hkMainReg, "WarningGB", 0, REG_DWORD, (BYTE *)&archive->nWarningGB, sizeof(DWORD));
		RegSetValueEx(hkMainReg, "ErrorGB", 0, REG_DWORD, (BYTE *)&archive->nErrorGB, sizeof(DWORD));
		RegSetValueEx(hkMainReg, "AutoRemoveLocation", 0, REG_SZ, (BYTE *)archive->szAutoRemoveLocation, lstrlen(archive->szAutoRemoveLocation) + 1);
		RegSetValueEx(hkMainReg, "AutoRestart", 0, REG_DWORD, (BYTE *)&archive->fAutoRestart, sizeof(DWORD));
		RegSetValueEx(hkMainReg, "AlarmSoundFilename", 0, REG_SZ, (BYTE *)archive->szAlarmSoundFilename, lstrlen(archive->szAlarmSoundFilename) + 1);

		RegCloseKey(hkMainReg);
	}
}

void GetArchiveChannelListDispInfo(LV_DISPINFO *pnmv) 
{ 
    // Provide the item or subitem's text, if requested. 
    if (pnmv->item.mask & LVIF_TEXT)
	{ 
        int nPMTIndex = (int)(pnmv->item.lParam);
		switch(pnmv->item.iSubItem)
		{
		case 0:
			wsprintf(pnmv->item.pszText, "%d", v->pat.pmt[nPMTIndex].nProgramNumber);
			break;
		case 1:
			if (archive->ap[nPMTIndex]->fEnabled)
				lstrcpy(pnmv->item.pszText, "Enabled");
			else
				pnmv->item.pszText[0] = '\0';
			break;
		case 2:
			switch(archive->ap[nPMTIndex]->nEPGType)
			{
			case EPG_TYPE_LOCAL_EPG:
				lstrcpy(pnmv->item.pszText, "Local");
				break;
			case EPG_TYPE_REMOTE_EPG:
				lstrcpy(pnmv->item.pszText, "Remote");
				break;
			case EPG_TYPE_30_MIN:
				lstrcpy(pnmv->item.pszText, "Timed");
				break;
			}
			break;
		case 3:
			lstrcpy(pnmv->item.pszText, archive->ap[nPMTIndex]->szChannelName);
			break;
		case 4:
			lstrcpy(pnmv->item.pszText, archive->ap[nPMTIndex]->szOutputLocation);
			break;
		}
	}
}

void AddPMTEntryToArchivePIDList(int nPID, int nPMTIndex)
{
	int i;

	for (i = 0; i < MAX_ARCHIVE_PID_STREAMS; i++)
	{
		if (archive->nArchivePIDs[nPID][i] == 0)
		{
			archive->nArchivePIDs[nPID][i] = nPMTIndex + 1;
			return;
		}
	}
	MessageBox(v->hWndMainWindow, "OUT OF ROOM in AddPMTEntryToArchivePIDList - tell support@tsreader.co.uk", gszAppName, MB_ICONSTOP);
}

void BuildArchivePIDList()
{
	// This initializes the nArchivePIDs[] list
	// Each nArchivePIDs entry that's being recorded
	// contains a pointer +1 to the PMT index (archive->ap too)
	// so when we receive data from the main thread we can tell
	// really quick which program this recording goes to
	int nPMTIndex;

	for (nPMTIndex = 0; nPMTIndex < MAX_PAT_ENTRIES; nPMTIndex++)
	{
		if (v->pat.pmt[nPMTIndex].nPMTPID == 0)
			break;
		if (v->pat.pmt[nPMTIndex].nProgramNumber != 0)
		{
			if (archive->ap[nPMTIndex]->fEnabled == TRUE)
			{
				int nESIndex;
				BOOL fAlreadyGotPCR = FALSE;

				archive->ap[nPMTIndex]->nVideoPID = archive->ap[nPMTIndex]->nSubtitlePID = 0x1fff;
				for (nESIndex = 0; nESIndex < MAX_ESLIST_ENTRIES; nESIndex++)
				{
					if (v->pat.pmt[nPMTIndex].es[nESIndex].nESPID == 0)
						break;
					if (v->pat.pmt[nPMTIndex].es[nESIndex].nESPID == v->pat.pmt[nPMTIndex].nPCRPID)
						fAlreadyGotPCR = TRUE;
					AddPMTEntryToArchivePIDList(v->pat.pmt[nPMTIndex].es[nESIndex].nESPID, nPMTIndex);
					switch(v->pat.pmt[nPMTIndex].es[nESIndex].nStreamType)
					{
					case 0x01:	// MPEG-1
					case 0x02:	// MPEG-2
					case 0x10:	// MPEG-4 video
					case 0x1b:	// H.264 video
					case 0x80:	// DCII
						archive->ap[nPMTIndex]->nVideoPID = v->pat.pmt[nPMTIndex].es[nESIndex].nESPID;
						break;
					//case 0x06:	// 
					//	if (IsSubtitleStream(nPMTIndex, nESIndex) == TRUE)
					//		archive->ap[nPMTIndex]->nSubtitlePID = v->pat.pmt[nPMTIndex].es[nESIndex].nESPID;
					//	break;
					}
				}

				// Plus the PCR if it's on a seperate PID
				if (!fAlreadyGotPCR)
					AddPMTEntryToArchivePIDList(v->pat.pmt[nPMTIndex].nPCRPID, nPMTIndex);
				
				// Plus the PMT 
				//AddPMTEntryToArchivePIDList(v->pat.pmt[nPMTIndex].nPMTPID, nPMTIndex);
			}
		}
	}
}

void BuildOutputPAT(BYTE * pat, int nPMTIndex)
{
	DWORD dwCRC;

	set_buf(BM_ARCHIVE_THREAD, pat, 188, TRUE);
	set_bits(BM_ARCHIVE_THREAD, 0x47, 8);		// sync
	set_bits(BM_ARCHIVE_THREAD, 0x40, 8);		// PES start/PID
	set_bits(BM_ARCHIVE_THREAD, 0x00, 8);		// PID
	set_bits(BM_ARCHIVE_THREAD, 0x10, 8);		// TSF flags & continuity
	set_bits(BM_ARCHIVE_THREAD, 0x00, 8);		// pointer
	set_bits(BM_ARCHIVE_THREAD, 0x00, 8);		// table ID
	set_bits(BM_ARCHIVE_THREAD, 0x1, 1);		// section syntax indicator
	set_bits(BM_ARCHIVE_THREAD, 0x0, 1);		// '0'
	set_bits(BM_ARCHIVE_THREAD, 0x3, 2);		// reserved
	set_bits(BM_ARCHIVE_THREAD, 0x00d, 12);	// section length
	set_bits(BM_ARCHIVE_THREAD, v->pat.nTransportStreamID, 16);	// transport stream ID
	set_bits(BM_ARCHIVE_THREAD, 0x3, 2);		// reserved
	set_bits(BM_ARCHIVE_THREAD, v->pat.nVersionNumber, 5);		// version number
	set_bits(BM_ARCHIVE_THREAD, 0x1, 1);		// current/next indicator
	set_bits(BM_ARCHIVE_THREAD, 0x00, 8);		// section number
	set_bits(BM_ARCHIVE_THREAD, 0x00, 8);		// last section number
	set_bits(BM_ARCHIVE_THREAD, v->pat.pmt[nPMTIndex].nProgramNumber, 16);	// program number
	set_bits(BM_ARCHIVE_THREAD, 0x7, 3);		// reserved
	set_bits(BM_ARCHIVE_THREAD, v->pat.pmt[nPMTIndex].nPMTPID, 13);	// PMT PID
	dwCRC = SourceHelper_CRC_Calc(&pat[5], 21 - 5 - 4);	
	set_bits(BM_ARCHIVE_THREAD, dwCRC, 32);
	while (get_byte_pos(BM_ARCHIVE_THREAD) < 188)
		set_bits(BM_ARCHIVE_THREAD, 0xff, 8);
}

void BuildOutputPMT(BYTE * pmt, int nPMTMaxLength, int nPMTIndex)
{
	int nSectionLength = 13;		// minimum PMT size
	int nESIndex, i;
	DWORD dwCRC;
	BYTE pmt_section[3096];

	// Calculate the section length
	nSectionLength += v->pat.pmt[nPMTIndex].nProgramInfoLength;
	for (nESIndex = 0; nESIndex < MAX_ESLIST_ENTRIES; nESIndex++)
	{
		if (v->pat.pmt[nPMTIndex].es[nESIndex].nESPID == 0)
			break;
		nSectionLength += 5 + v->pat.pmt[nPMTIndex].es[nESIndex].nDescriptorsLength;
	}

	// Build the main PMT section
	set_buf(BM_ARCHIVE_THREAD, pmt_section, sizeof(pmt_section), TRUE);
	set_bits(BM_ARCHIVE_THREAD, 0x02, 8);		// table ID
	set_bits(BM_ARCHIVE_THREAD, 0x1, 1);		// section syntax indicator
	set_bits(BM_ARCHIVE_THREAD, 0x0, 1);		// '0'
	set_bits(BM_ARCHIVE_THREAD, 0x3, 2);		// reserved
	set_bits(BM_ARCHIVE_THREAD, nSectionLength, 12);	// section length
	set_bits(BM_ARCHIVE_THREAD, v->pat.pmt[nPMTIndex].nProgramNumber, 16);	// program number
	set_bits(BM_ARCHIVE_THREAD, 0x3, 2);		// reserved
	set_bits(BM_ARCHIVE_THREAD, 0x01, 5);		// version number
	set_bits(BM_ARCHIVE_THREAD, 0x1, 1);		// current/next indicator
	set_bits(BM_ARCHIVE_THREAD, 0x00, 8);		// section number
	set_bits(BM_ARCHIVE_THREAD, 0x00, 8);		// last section number
	set_bits(BM_ARCHIVE_THREAD, 0x7, 3);		// reserved
	set_bits(BM_ARCHIVE_THREAD, v->pat.pmt[nPMTIndex].nPCRPID, 13); // PCR PID
	set_bits(BM_ARCHIVE_THREAD, 0xf, 4);		// reserved
	
	// Program info descriptors
	set_bits(BM_ARCHIVE_THREAD, v->pat.pmt[nPMTIndex].nProgramInfoLength, 12);	// program info length
	for (i = 0; i < v->pat.pmt[nPMTIndex].nProgramInfoLength; i++)
		set_bits(BM_ARCHIVE_THREAD, v->pat.pmt[nPMTIndex].pProgramInfo[i], 8);

	// ES loop
	for (nESIndex = 0; nESIndex < MAX_ESLIST_ENTRIES; nESIndex++)
	{
		if (v->pat.pmt[nPMTIndex].es[nESIndex].nESPID == 0)
			break;

		set_bits(BM_ARCHIVE_THREAD, v->pat.pmt[nPMTIndex].es[nESIndex].nStreamType, 8);		// stream type
		set_bits(BM_ARCHIVE_THREAD, 0x7, 3);		// reserved
		set_bits(BM_ARCHIVE_THREAD, v->pat.pmt[nPMTIndex].es[nESIndex].nESPID, 13);
		set_bits(BM_ARCHIVE_THREAD, 0xf, 4);		// reserved
		set_bits(BM_ARCHIVE_THREAD, v->pat.pmt[nPMTIndex].es[nESIndex].nDescriptorsLength, 12);	// ES info Length
		for (i = 0; i < v->pat.pmt[nPMTIndex].es[nESIndex].nDescriptorsLength; i++)
			set_bits(BM_ARCHIVE_THREAD, v->pat.pmt[nPMTIndex].es[nESIndex].pDescriptors[i], 8);
	}

	dwCRC = SourceHelper_CRC_Calc(pmt_section, nSectionLength - 1);
	set_bits(BM_ARCHIVE_THREAD, dwCRC, 32);
	archive->ap[nPMTIndex]->nPMTPackets = BuildTSSection(pmt_section, nSectionLength + 4, pmt, nPMTMaxLength, v->pat.pmt[nPMTIndex].nPMTPID);
	archive->ap[nPMTIndex]->nPMTPacketOutputCounter = 0;
	archive->ap[nPMTIndex]->nPMTOutputContinuityCounter = 0;
}

void GenerateArchivePATs()
{
	int nPMTIndex;

	for (nPMTIndex = 0; nPMTIndex < MAX_PAT_ENTRIES; nPMTIndex++)
	{
		if (v->pat.pmt[nPMTIndex].nPMTPID == 0)
			break;
		if (v->pat.pmt[nPMTIndex].nProgramNumber != 0)
		{
			if (archive->ap[nPMTIndex]->fEnabled == TRUE)
			{
				BuildOutputPAT(archive->ap[nPMTIndex]->pat, nPMTIndex);
				BuildOutputPMT(archive->ap[nPMTIndex]->pmt, sizeof(archive->ap[nPMTIndex]->pmt), nPMTIndex);
			}
		}
	}
}

DWORD WriteFileWithCRLF(HANDLE hFile, char * szBuffer)
{
	DWORD dwWritten1, dwWritten2;
	WriteFile(hFile, szBuffer, lstrlen(szBuffer), &dwWritten1, NULL);
	WriteFile(hFile, "\r\n", 2, &dwWritten2, NULL);

	return dwWritten1 + dwWritten2;
}

void CloseUserDataAndCaptionFiles(int nPMTIndex)
{
	int nCaptionFile;

	if (archive->ap[nPMTIndex]->hUserDataOutputFile != NULL)
	{
		CloseHandle(archive->ap[nPMTIndex]->hUserDataOutputFile);
		archive->ap[nPMTIndex]->hUserDataOutputFile = NULL;
		if (archive->ap[nPMTIndex]->nUserDataBytes == 0)
		{
			DeleteFile(archive->ap[nPMTIndex]->szUserDataOutputFileName);
		}
	}
	for (nCaptionFile = 0; nCaptionFile < MAX_CAPTION_CHANNELS; nCaptionFile++)
	{
		if (archive->ap[nPMTIndex]->hCaptionOutputFile[nCaptionFile] != NULL)
		{
			CloseHandle(archive->ap[nPMTIndex]->hCaptionOutputFile[nCaptionFile]);
			archive->ap[nPMTIndex]->hCaptionOutputFile[nCaptionFile] = NULL;
			if (archive->ap[nPMTIndex]->nCaptionProgramBytes[nCaptionFile] == 0)
			{
				DeleteFile(archive->ap[nPMTIndex]->szCaptionOutputFilename[nCaptionFile]);
			}
		}
	}
}

void CleanupArchiveFilename(char * szFilename)
{
	int i;

	for (i = 0; i < lstrlen(szFilename); i++)
	{
		switch(szFilename[i])
		{
		case '/':
		case '\\':
		case ':':
		case '*':
		case '\"':
		case '<':
		case '>':
		case '|':
		case '?':
			szFilename[i] = ' ';
			break;
		}
	}
}

void OpenCurrentArchiveFile(int nPMTIndex)
{
	int nCaptionFile;
	HANDLE hTextInfoFile;
	char * szExtension;
	char szChannelName[64];
	char szTemp[MAX_PATH];
	char szShortStatus[256 + MAX_PATH] = {0};
	char szLongStatus[1024 + MAX_PATH] = {0};
	char szTextInfoFile[MAX_PATH];

	lstrcpy(archive->ap[nPMTIndex]->szOutputFileName, archive->ap[nPMTIndex]->szOutputLocation);
	if (archive->ap[nPMTIndex]->szOutputFileName[lstrlen(archive->ap[nPMTIndex]->szOutputFileName) - 1] != '\\')
		lstrcat(archive->ap[nPMTIndex]->szOutputFileName, "\\");
	GetSystemTime(&archive->ap[nPMTIndex]->stActualStart);
	lstrcpy(szChannelName, archive->ap[nPMTIndex]->szChannelName);
	if (lstrlen(szChannelName) == 0)
		wsprintf(szChannelName, "%05d", v->pat.pmt[nPMTIndex].nProgramNumber);
	wsprintf(szTemp, "%s_%04d%02d%02d_%02d%02d%02d", 
			 szChannelName,
			 archive->ap[nPMTIndex]->stActualStart.wYear, archive->ap[nPMTIndex]->stActualStart.wMonth, archive->ap[nPMTIndex]->stActualStart.wDay,
			 archive->ap[nPMTIndex]->stActualStart.wHour, archive->ap[nPMTIndex]->stActualStart.wMinute, archive->ap[nPMTIndex]->stActualStart.wSecond);
	if (lstrlen(archive->ap[nPMTIndex]->szCurrentProgram))
	{
		lstrcat(szTemp, "_");
		lstrcat(szTemp, archive->ap[nPMTIndex]->szCurrentProgram);
	}
	lstrcat(szTemp, ".mpg");
	CleanupArchiveFilename(szTemp);
	lstrcat(archive->ap[nPMTIndex]->szOutputFileName, szTemp);
	
	archive->ap[nPMTIndex]->dCurrentFileWritten = 0.0;
	memset(&archive->ap[nPMTIndex]->stCurrentFileRun, 0, sizeof(archive->ap[nPMTIndex]->stCurrentFileRun));
	archive->ap[nPMTIndex]->lnCurrentFileWritePtr.QuadPart = 0;
	archive->ap[nPMTIndex]->hOutputFile = CreateFile(archive->ap[nPMTIndex]->szOutputFileName, GENERIC_WRITE, FILE_SHARE_READ, (LPSECURITY_ATTRIBUTES)NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED, (HANDLE)NULL);

	lstrcpy(archive->ap[nPMTIndex]->szUserDataOutputFileName, archive->ap[nPMTIndex]->szOutputFileName);
	szExtension = strstr(archive->ap[nPMTIndex]->szUserDataOutputFileName, ".mpg");
	lstrcpy(szExtension, ".user-data");
	archive->ap[nPMTIndex]->hUserDataOutputFile = CreateFile(archive->ap[nPMTIndex]->szUserDataOutputFileName, GENERIC_WRITE, FILE_SHARE_READ, (LPSECURITY_ATTRIBUTES) NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, (HANDLE) NULL);
	archive->ap[nPMTIndex]->nUserDataBytes = 0;

	archive->ap[nPMTIndex]->nCaptionCurrentOutputFileIndex[0] = archive->ap[nPMTIndex]->nCaptionCurrentOutputFileIndex[1] = -1;
	archive->ap[nPMTIndex]->fXDSActive = TRUE;
	for (nCaptionFile = 0; nCaptionFile < MAX_CAPTION_CHANNELS; nCaptionFile++)
	{
		lstrcpy(archive->ap[nPMTIndex]->szCaptionOutputFilename[nCaptionFile], archive->ap[nPMTIndex]->szOutputFileName);
		szExtension = strstr(archive->ap[nPMTIndex]->szCaptionOutputFilename[nCaptionFile], ".mpg");
		if (nCaptionFile < 4)
			wsprintf(szTemp, ".cc%d.txt", nCaptionFile + 1);
		else
			wsprintf(szTemp, ".text%d.txt", nCaptionFile - 4 + 1);
		lstrcpy(szExtension, szTemp);
		archive->ap[nPMTIndex]->hCaptionOutputFile[nCaptionFile] = CreateFile(archive->ap[nPMTIndex]->szCaptionOutputFilename[nCaptionFile], GENERIC_WRITE, FILE_SHARE_READ, (LPSECURITY_ATTRIBUTES) NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, (HANDLE) NULL);
		archive->ap[nPMTIndex]->nCaptionOutputPosition[nCaptionFile] = 0;
		archive->ap[nPMTIndex]->nCaptionProgramBytes[nCaptionFile] = 0;
	}

	// Write a text file with all the info about this new file
	// since if we crash, we'll need it to generate the XML
	lstrcpy(szTextInfoFile, archive->ap[nPMTIndex]->szOutputLocation);
	if (szTextInfoFile[lstrlen(szTextInfoFile) - 1] != '\\')
		lstrcat(szTextInfoFile, "\\");
	lstrcat(szTextInfoFile, archive->ap[nPMTIndex]->szChannelName);
	lstrcat(szTextInfoFile, ".events.txt");
	hTextInfoFile = CreateFile(szTextInfoFile, GENERIC_WRITE, FILE_SHARE_READ, (LPSECURITY_ATTRIBUTES) NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, (HANDLE) NULL);
	if (hTextInfoFile != INVALID_HANDLE_VALUE)
	{
		SetFilePointer(hTextInfoFile, 0, NULL, FILE_END);
		WriteFileWithCRLF(hTextInfoFile, szChannelName);
		WriteFileWithCRLF(hTextInfoFile, archive->ap[nPMTIndex]->szOutputFileName);
		WriteFileWithCRLF(hTextInfoFile, archive->ap[nPMTIndex]->szUserDataOutputFileName);
		WriteFileWithCRLF(hTextInfoFile, archive->ap[nPMTIndex]->szCurrentProgram);
		WriteFileWithCRLF(hTextInfoFile, archive->ap[nPMTIndex]->szCurrentDescription);
		wsprintf(szTemp, "%04d:%02d:%02d %02d:%02d:%02d",
			     archive->ap[nPMTIndex]->stCurrentStart.wYear, archive->ap[nPMTIndex]->stCurrentStart.wMonth, archive->ap[nPMTIndex]->stCurrentStart.wDay,
				 archive->ap[nPMTIndex]->stCurrentStart.wHour, archive->ap[nPMTIndex]->stCurrentStart.wMinute, archive->ap[nPMTIndex]->stCurrentStart.wSecond);
		WriteFileWithCRLF(hTextInfoFile, szTemp);
		wsprintf(szTemp, "%02d:%02d:%02d",
			     archive->ap[nPMTIndex]->stCurrentDuration.wHour, archive->ap[nPMTIndex]->stCurrentDuration.wMinute, archive->ap[nPMTIndex]->stCurrentDuration.wSecond);
		WriteFileWithCRLF(hTextInfoFile, szTemp);
		WriteFileWithCRLF(hTextInfoFile, "");
		CloseHandle(hTextInfoFile);
	}

	if (archive->ap[nPMTIndex]->nEPGType == EPG_TYPE_30_MIN)		
	{
		if (archive->ap[nPMTIndex]->fPreviouslyRemoteEPG)
			wsprintf(szShortStatus, "Start: %s - 30 minute recording - remote EPG disconnected", szChannelName);
		else
			wsprintf(szShortStatus, "Start: %s - 30 minute recording", szChannelName);
	}
	else
		wsprintf(szShortStatus, "Start: %s - %s", szChannelName, archive->ap[nPMTIndex]->szCurrentProgram);
	wsprintf(szLongStatus, "Output file %s", archive->ap[nPMTIndex]->szOutputFileName);
	ArchiveEventListWrite(EVENT_ICON_INFORMATION, szShortStatus, szLongStatus);
}

void RemoveATSCRatingFromProgram(char * szProgramName, char * szRating)
{
	char * szSeperator = strstr(szProgramName, " [");
	*szRating = '\0';
	if (szSeperator != NULL)
	{
		lstrcpy(szRating, &szSeperator[2]);
		*szSeperator = '\0';
		szSeperator = strstr(szRating, "]");
		if (szSeperator != NULL)
			*szSeperator = '\0';
	}
}

BOOL ReadEITSocket(SOCKET socket, char * szBuffer, int nSize)
{
	char c;
	int nCounter = 0;
	int nrecvRetVal;

	do
	{
		nrecvRetVal = recv(socket, &c, 1, 0);
		if ( (nrecvRetVal == 0) || (nrecvRetVal == SOCKET_ERROR) )
		{
			*szBuffer++ = '\0';
			return FALSE;
		}
		if (c == '\n')
		{
			*szBuffer++ = '\0';
			return TRUE;
		}
		if (c != '\r' && c != '\0')
		{
			*szBuffer++ = c;
			nCounter++;
			if (nCounter == nSize)
			{
				*szBuffer++ = '\0';
				return TRUE;
			}
		}
	} while (TRUE);
	return FALSE;			// should never get here
}
DWORD WINAPI ReceiveRemoteEPGDataThread(LPVOID pvarg)
{
	int nPMTIndex = (int)(LONG_PTR)pvarg;
	char szChannelName[128];
	char szStartDateTime[128];
	char szDuration[128];

	archive->ap[nPMTIndex]->fRemoteEPGThreadRunning = TRUE;

	do
	{
		if (!archive->ap[nPMTIndex]->fOldArchiveEPGServer)
		{
			if (ReadEITSocket(archive->ap[nPMTIndex]->EPGSocket, szChannelName, sizeof(szChannelName)) == FALSE)
				break;
		}
		else
		{
			if (!archive->ap[nPMTIndex]->fOldArchiveEPGGotChannelName)
			{
				if (ReadEITSocket(archive->ap[nPMTIndex]->EPGSocket, szChannelName, sizeof(szChannelName)) == FALSE)
					break;
				archive->ap[nPMTIndex]->fOldArchiveEPGGotChannelName = TRUE;
			}
		}
		if (ReadEITSocket(archive->ap[nPMTIndex]->EPGSocket, archive->ap[nPMTIndex]->szNextProgram, sizeof(archive->ap[nPMTIndex]->szNextProgram)) == FALSE)
			break;
		if (ReadEITSocket(archive->ap[nPMTIndex]->EPGSocket, archive->ap[nPMTIndex]->szNextDescription, sizeof(archive->ap[nPMTIndex]->szNextDescription)) == FALSE)
			break;
		if (ReadEITSocket(archive->ap[nPMTIndex]->EPGSocket, szStartDateTime, sizeof(szStartDateTime)) == FALSE)
			break;
		if (ReadEITSocket(archive->ap[nPMTIndex]->EPGSocket, szDuration, sizeof(szDuration)) == FALSE)
			break;

		// YYYY/MM/DD HH:MM:SS
		// 0123456789012345678
		memset(&archive->ap[nPMTIndex]->stNextStart, 0, sizeof(archive->ap[nPMTIndex]->stNextStart));
		szStartDateTime[4] = '\0';
		sscanf(szStartDateTime, "%d", (int *)&archive->ap[nPMTIndex]->stNextStart.wYear);
		szStartDateTime[7] = '\0';
		sscanf(&szStartDateTime[5], "%d", (int *)&archive->ap[nPMTIndex]->stNextStart.wMonth);
		szStartDateTime[10] = '\0';
		sscanf(&szStartDateTime[8], "%d", (int *)&archive->ap[nPMTIndex]->stNextStart.wDay);

		szStartDateTime[13] = '\0';
		sscanf(&szStartDateTime[11], "%d", (int *)&archive->ap[nPMTIndex]->stNextStart.wHour);
		szStartDateTime[16] = '\0';
		sscanf(&szStartDateTime[14], "%d", (int *)&archive->ap[nPMTIndex]->stNextStart.wMinute);
		if (!archive->ap[nPMTIndex]->fOldArchiveEPGServer)
			sscanf(&szStartDateTime[17], "%d", (int *)&archive->ap[nPMTIndex]->stNextStart.wSecond);
		else
			archive->ap[nPMTIndex]->stNextStart.wSecond = 0;

		// HH:MM:SS
		// 01234567
		memset(&archive->ap[nPMTIndex]->stNextDuration, 0, sizeof(archive->ap[nPMTIndex]->stNextDuration));
		szDuration[2] = '\0';
		sscanf(szDuration, "%d", (int *)&archive->ap[nPMTIndex]->stNextDuration.wHour);
		szDuration[5] = '\0';
		sscanf(&szDuration[3], "%d", (int *)&archive->ap[nPMTIndex]->stNextDuration.wMinute);
		if (!archive->ap[nPMTIndex]->fOldArchiveEPGServer)
			sscanf(&szDuration[6], "%d", (int *)&archive->ap[nPMTIndex]->stNextDuration.wSecond);
		else
			archive->ap[nPMTIndex]->stNextDuration.wSecond = 0;

		archive->ap[nPMTIndex]->fNewRemoteProgramDetected = TRUE;
	} while (TRUE);

	archive->ap[nPMTIndex]->fRemoteEPGThreadRunning = FALSE;

	return 0;
}

void SetupNewRemoteEPGProgramDetails(int nPMTIndex)
{
	lstrcpy(archive->ap[nPMTIndex]->szCurrentProgram, archive->ap[nPMTIndex]->szNextProgram);
	lstrcpy(archive->ap[nPMTIndex]->szCurrentDescription, archive->ap[nPMTIndex]->szNextDescription);
	memcpy(&archive->ap[nPMTIndex]->stCurrentStart, &archive->ap[nPMTIndex]->stNextStart, sizeof(SYSTEMTIME));
	memcpy(&archive->ap[nPMTIndex]->stCurrentDuration, &archive->ap[nPMTIndex]->stNextDuration, sizeof(SYSTEMTIME));
}

BOOL OpenRemoteEPGConnection(int nPMTIndex)
{
	int nEPGPort, nEPGChannel;
	int nStat;
	HANDLE hThread;
	DWORD dwThreadID;
	char * szColonPtr;
	char * szSlashPtr;
	SOCKADDR_IN sin;
	char szEPGServer[MAX_PATH];
	char szTemp[128];

	lstrcpy(szEPGServer, archive->ap[nPMTIndex]->szRemoteEPGServer);
	szColonPtr = strstr(szEPGServer, ":");
	if (szColonPtr == NULL)
		return FALSE;
	szSlashPtr = strstr(szEPGServer, "/");
	if (szSlashPtr == NULL)
	{
		szSlashPtr = strstr(szEPGServer, "\\");
		if (szSlashPtr == NULL)
			return FALSE;
		archive->ap[nPMTIndex]->fOldArchiveEPGServer = TRUE;
		archive->ap[nPMTIndex]->fOldArchiveEPGGotChannelName = FALSE;
	}
	*szColonPtr = '\0';
	*szSlashPtr = '\0';

	sscanf(szColonPtr + 1, "%d", &nEPGPort);
	sscanf(szSlashPtr + 1, "%d", &nEPGChannel);
	archive->ap[nPMTIndex]->EPGSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (archive->ap[nPMTIndex]->EPGSocket == INVALID_SOCKET)
		return FALSE;
	if (FillAddr(&sin, szEPGServer, nEPGPort) == FALSE)
		return FALSE;
	nStat = connect(archive->ap[nPMTIndex]->EPGSocket, (PSOCKADDR)&sin, sizeof(sin));
	if (nStat == SOCKET_ERROR)
		return FALSE;
	
	if (archive->ap[nPMTIndex]->fOldArchiveEPGServer == FALSE)
		wsprintf(szTemp, "%05d", nEPGChannel);
	else
		wsprintf(szTemp, "%04d", nEPGChannel);
	send(archive->ap[nPMTIndex]->EPGSocket, szTemp, lstrlen(szTemp), 0);

	hThread = CreateThread(NULL, 0, ReceiveRemoteEPGDataThread, (LPVOID)(LONG_PTR)nPMTIndex, 0, &dwThreadID);
	CloseHandle(hThread);
	
	return TRUE;
}

void GenerateInitialArchiveFiles()
{
	int nPMTIndex;

	for (nPMTIndex = 0; nPMTIndex < MAX_PAT_ENTRIES; nPMTIndex++)
	{
		if (v->pat.pmt[nPMTIndex].nPMTPID == 0)
			break;
		if (v->pat.pmt[nPMTIndex].nProgramNumber != 0)
		{
			if (archive->ap[nPMTIndex]->fEnabled == TRUE)
			{
				int i;
				char szJPGFolderName[MAX_PATH];

				for (i = 0; i < MAX_ASYNC_IO; i++)
					archive->ap[nPMTIndex]->pBlockBuffer[i] = LocalAlloc(LPTR, archive->nBlockSize * 1024);
				archive->ap[nPMTIndex]->nStatus = STATUS_NORMAL;
				archive->ap[nPMTIndex]->nGraphBytesMin = 1024 * 1024 * 1024;
				archive->ap[nPMTIndex]->nGraphBytesMax = 0;
				archive->ap[nPMTIndex]->dAllBytesWritten = 0.0;
				archive->ap[nPMTIndex]->nSyncLossCount = v->nSyncLossCount;
				archive->ap[nPMTIndex]->nContinuityErrorCount = GetProgramCCorTEICount(nPMTIndex, TRUE);
				archive->ap[nPMTIndex]->nTEIErrorCount = GetProgramCCorTEICount(nPMTIndex, FALSE);

				InitializeCriticalSection(&archive->ap[nPMTIndex]->csThumbnailList);

				switch(archive->ap[nPMTIndex]->nEPGType)
				{
				case EPG_TYPE_LOCAL_EPG:
					if (LocateCurrentProgram(nPMTIndex, 
										 archive->ap[nPMTIndex]->szCurrentProgram,
										 archive->ap[nPMTIndex]->szCurrentDescription,
										 &archive->ap[nPMTIndex]->stCurrentStart,
										 &archive->ap[nPMTIndex]->stCurrentDuration, FALSE) == TRUE)
					{
						RemoveATSCRatingFromProgram(archive->ap[nPMTIndex]->szCurrentProgram, archive->ap[nPMTIndex]->szCurrentRating);
					}
					break;
				case EPG_TYPE_30_MIN:
					archive->ap[nPMTIndex]->szCurrentProgram[0] = '\0';
					archive->ap[nPMTIndex]->szCurrentDescription[0] = '\0';
					GetSystemTime(&archive->ap[nPMTIndex]->stCurrentStart);
					memset(&archive->ap[nPMTIndex]->stCurrentDuration, 0, sizeof(archive->ap[nPMTIndex]->stCurrentDuration));
					archive->ap[nPMTIndex]->stCurrentDuration.wMinute = 30;
					break;
				case EPG_TYPE_REMOTE_EPG:
					if (OpenRemoteEPGConnection(nPMTIndex) == TRUE)
					{
						int nTimeout = 1000;

						// Wait for the event name to arrive on the socket
						do
						{
							if (archive->ap[nPMTIndex]->fNewRemoteProgramDetected)
								break;
							Sleep(10);
						} while (nTimeout-- > 0);
						if (archive->ap[nPMTIndex]->fNewRemoteProgramDetected)
						{
							SetupNewRemoteEPGProgramDetails(nPMTIndex);
							archive->ap[nPMTIndex]->fNewRemoteProgramDetected = FALSE;
						}
					}
					break;
				}

				// Make sure there's a JPG folder
				lstrcpy(szJPGFolderName, archive->ap[nPMTIndex]->szOutputLocation);
				if (szJPGFolderName[lstrlen(szJPGFolderName) - 1] != '\\')
					lstrcat(szJPGFolderName, "\\");
				lstrcat(szJPGFolderName, "jpg");
				CreateDirectory(szJPGFolderName, NULL);

				// Now open the initial output file
				OpenCurrentArchiveFile(nPMTIndex);
			}
		}
	}
}

void SetupDefaultChannelNames(HWND hDlg)
{
	int nPMTIndex;

	for (nPMTIndex = 0; nPMTIndex < MAX_PAT_ENTRIES; nPMTIndex++)
	{
		if (v->pat.pmt[nPMTIndex].nPMTPID == 0)
			break;
		if (v->pat.pmt[nPMTIndex].nProgramNumber != 0)
		{
			archive->ap[nPMTIndex]->szChannelName[0] = '\0';
			if (v->pChannelData[v->pat.pmt[nPMTIndex].nProgramNumber] != NULL)
			{
				if (lstrlen(v->pChannelData[v->pat.pmt[nPMTIndex].nProgramNumber]->szShortName))
					lstrcpy(archive->ap[nPMTIndex]->szChannelName, v->pChannelData[v->pat.pmt[nPMTIndex].nProgramNumber]->szShortName);
			}
		}
	}
	ListView_RedrawItems(GetDlgItem(hDlg, IDC_ARCHIVE_CHANNEL_LIST), 0, ListView_GetItemCount(GetDlgItem(hDlg, IDC_ARCHIVE_CHANNEL_LIST)));
}

BOOL CheckActiveArchiveChannels(HWND hWnd, BOOL fErrorsShown)
{
	int nPMTIndex;
	int nChannelsActive = 0;

	for (nPMTIndex = 0; nPMTIndex < MAX_PAT_ENTRIES; nPMTIndex++)
	{
		HANDLE hTestFile;
		char szTestFileName[MAX_PATH];

		if (v->pat.pmt[nPMTIndex].nPMTPID == 0)
			break;

		if (v->pat.pmt[nPMTIndex].nProgramNumber != 0)
		{
			if (archive->ap[nPMTIndex]->fEnabled == TRUE)
			{
				nChannelsActive++;

				// Check Output Folder's existance
				lstrcpy(szTestFileName, archive->ap[nPMTIndex]->szOutputLocation);
				if (szTestFileName[lstrlen(szTestFileName) - 1] != '\\')
					lstrcat(szTestFileName, "\\");
				lstrcat(szTestFileName, "test-file");
				hTestFile = CreateFile(szTestFileName, GENERIC_WRITE, FILE_SHARE_READ, (LPSECURITY_ATTRIBUTES) NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, (HANDLE) NULL);
				if (hTestFile == INVALID_HANDLE_VALUE)
				{
					char szTemp[1024];

					wsprintf(szTemp, "Unable to write to the output location set on channel %d", v->pat.pmt[nPMTIndex].nProgramNumber);
					if (fErrorsShown)
						MessageBox(hWnd, szTemp, gszAppName, MB_ICONSTOP);
					return FALSE;
				}
				CloseHandle(hTestFile);
				DeleteFile(szTestFileName);

				// Check EPG settings
				switch(archive->ap[nPMTIndex]->nEPGType)
				{
				case EPG_TYPE_LOCAL_EPG:
					{
						char szCurrentProgram[1024];

						if (LocateCurrentProgram(nPMTIndex, szCurrentProgram, NULL, NULL, NULL, FALSE) == FALSE)
						{
							char szTemp[128];
							wsprintf(szTemp, "Can't use Local EPG for channel %d as there is no EPG data for this channel/program", v->pat.pmt[nPMTIndex].nProgramNumber);
							if (fErrorsShown)
								MessageBox(hWnd, szTemp, gszAppName, MB_ICONSTOP);
							return FALSE;
						}
					}
					break;
				case EPG_TYPE_REMOTE_EPG:
					{
						char * szColon = strstr(archive->ap[nPMTIndex]->szRemoteEPGServer, ":");
						char * szSlash;
						if (szColon == NULL)
						{
							MessageBox(hWnd, "A colon is required in the EPG server to seperate address and port", gszAppName, MB_ICONSTOP);
							return FALSE;
						}
						if (*(szColon + 1) < '0' || *(szColon + 1) > '9')
						{
							MessageBox(hWnd, "Port number appears to be missing from EPG address", gszAppName, MB_ICONSTOP);
							return FALSE;
						}
						szSlash = strstr(archive->ap[nPMTIndex]->szRemoteEPGServer, "/");
						if (szSlash == NULL)
						{
							// How about a backslash? That's for the old archive EIT service
							szSlash = strstr(archive->ap[nPMTIndex]->szRemoteEPGServer, "\\");
							if (szSlash == NULL)
							{
								MessageBox(hWnd, "A slash is required in the EPG server to seperate port and EPG channel", gszAppName, MB_ICONSTOP);
								return FALSE;
							}
						}
						if (*(szSlash + 1) < '0' || *(szSlash + 1) > '9')
						{
							MessageBox(hWnd, "Channel number appears to be missing from EPG address", gszAppName, MB_ICONSTOP);
							return FALSE;
						}
					}
					break;
				}
			}
		}
	}

	if (!nChannelsActive)
	{
		MessageBox(hWnd, "No channels are enabled to record", gszAppName, MB_ICONSTOP);
		return FALSE;
	}
	return TRUE;
}

void SetCalculatedMemory(HWND hDlg)
{
	int nPMTIndex;
	int nBlockSize = GetDlgItemInt(hDlg, IDC_ARCHIVE_BLOCKSIZE, NULL, FALSE);
	int nActiveChannels = 0;
	char szTemp[128];
	
	for (nPMTIndex = 0; nPMTIndex < MAX_PAT_ENTRIES; nPMTIndex++)
	{
		if (v->pat.pmt[nPMTIndex].nPMTPID == 0)
			break;
		if (v->pat.pmt[nPMTIndex].nProgramNumber != 0)
		{
			if (archive->ap[nPMTIndex]->fEnabled)
			{
				nActiveChannels++;
			}
		}
	}

	wsprintf(szTemp, "KB - %d MB required for buffers", nActiveChannels * MAX_ASYNC_IO * nBlockSize / 1024);
	SetDlgItemText(hDlg, IDC_ARCHIVE_SETUP_MEMORY, szTemp);
}

INT_PTR CALLBACK ArchiveWaitEPGDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	return FALSE;
}

INT_PTR CALLBACK ArchiveSetupDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	static BOOL fFirstTime;
	static HWND hWaitingForEPGDialog;

	switch(uMsg)
	{
	case WM_INITDIALOG:
		{
			HWND hWndArchiveChannelList = GetDlgItem(hDlg, IDC_ARCHIVE_CHANNEL_LIST);
			int nColumnPosition = 0;
			int nPMTIndex;
			LV_COLUMN lvc; 
			char szTemp[128];

			archive->nListSelectedProgram = -1;
			archive->nListSelectedPMT = -1;
			LoadArchiveSettings();

			// Setup list view columns. 
			lvc.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM; 
			lvc.pszText = szTemp; 

			lvc.fmt = LVCFMT_CENTER; 
			lvc.cx = 50; 
			lstrcpy(szTemp, TEXT("Program"));
			ListView_InsertColumn(hWndArchiveChannelList, nColumnPosition++, &lvc); 

			lvc.fmt = LVCFMT_CENTER; 
			lvc.cx = 60; 
			lstrcpy(szTemp, TEXT("Enabled"));
			ListView_InsertColumn(hWndArchiveChannelList, nColumnPosition++, &lvc); 

			lvc.fmt = LVCFMT_CENTER; 
			lvc.cx = 60; 
			lstrcpy(szTemp, TEXT("EPG"));
			ListView_InsertColumn(hWndArchiveChannelList, nColumnPosition++, &lvc); 

			lvc.fmt = LVCFMT_LEFT; 
			lvc.cx = 100; 
			lstrcpy(szTemp, TEXT("Channel Name"));
			ListView_InsertColumn(hWndArchiveChannelList, nColumnPosition++, &lvc); 

			lvc.fmt = LVCFMT_LEFT; 
			lvc.cx = 150; 
			lstrcpy(szTemp, TEXT("Output Location"));
			ListView_InsertColumn(hWndArchiveChannelList, nColumnPosition++, &lvc); 

			ListView_SetExtendedListViewStyle(hWndArchiveChannelList, LVS_EX_FULLROWSELECT);
			
			// Load the list view
			for (nPMTIndex = 0; nPMTIndex < MAX_PAT_ENTRIES; nPMTIndex++)
			{
				if (v->pat.pmt[nPMTIndex].nPMTPID == 0)
					break;
				if (v->pat.pmt[nPMTIndex].nProgramNumber != 0)
				{
					LV_ITEM lvi;

					memset(&lvi, 0, sizeof(lvi));
					lvi.state = 0; 
					lvi.stateMask = 0; 
					lvi.pszText = LPSTR_TEXTCALLBACK;   // app. maintains text 
					lvi.mask = LVIF_TEXT | LVIF_PARAM | LVIF_STATE; 
					lvi.iItem = nPMTIndex; 
					lvi.iSubItem = 0; 
					lvi.lParam = (LPARAM) nPMTIndex;    // item data 
					ListView_InsertItem(hWndArchiveChannelList, &lvi);
				}
			}

			// Setup global stuff
			SetDlgItemInt(hDlg, IDC_ARCHIVE_BLOCKSIZE, archive->nBlockSize, FALSE);
			SetCalculatedMemory(hDlg);

			CheckDlgButton(hDlg, IDC_ARCHIVE_AUTO_DELETE, archive->nAutoRemoveOldFiles);
			SetDlgItemInt(hDlg, IDC_ARCHIVE_AUTO_DELETE_GB, archive->nAutoRemoveSizeGB, FALSE);
			EnableWindow(GetDlgItem(hDlg, IDC_ARCHIVE_AUTO_DELETE_GB), archive->nAutoRemoveOldFiles);
			SetDlgItemText(hDlg, IDC_ARCHIVE_AUTO_DELETE_MOVE_LOCATION, archive->szAutoRemoveLocation);
			EnableWindow(GetDlgItem(hDlg, IDC_ARCHIVE_AUTO_DELETE_DELETE), archive->nAutoRemoveOldFiles);
			EnableWindow(GetDlgItem(hDlg, IDC_ARCHIVE_AUTO_DELETE_MOVE), archive->nAutoRemoveOldFiles);
			EnableWindow(GetDlgItem(hDlg, IDC_ARCHIVE_AUTO_DELETE_MOVE_LOCATION), archive->nAutoRemoveOldFiles == 2);
			if (archive->nAutoRemoveOldFiles == 1)
				CheckDlgButton(hDlg, IDC_ARCHIVE_AUTO_DELETE_DELETE, BST_CHECKED);
			else
				CheckDlgButton(hDlg, IDC_ARCHIVE_AUTO_DELETE_MOVE, BST_CHECKED);

			if (archive->nWarningGB == 0)
				archive->nWarningGB = 50;
			if (archive->nErrorGB == 0)
				archive->nErrorGB = 10;
			SetDlgItemInt(hDlg, IDC_ARCHIVE_WARNING_GB, archive->nWarningGB, FALSE);
			SetDlgItemInt(hDlg, IDC_ARCHIVE_ERROR_GB, archive->nErrorGB, FALSE);
			CheckDlgButton(hDlg, IDC_ARCHIVE_RESTART, archive->fAutoRestart);

			fFirstTime = TRUE;
			hWaitingForEPGDialog = NULL;
			SetFocus(GetDlgItem(hDlg, IDOK));
		}
		break;
	case WM_DESTROY:
		v->fAutomaticRecordAll = FALSE;
		break;
	case WM_ACTIVATE:
		if (fFirstTime)
		{
			fFirstTime = FALSE;
			if (v->fAutomaticRecordAll)
				PostMessage(hDlg, WM_COMMAND, IDOK, 0);
		}
		break;
	case WM_CLOSE:
		EndDialog(hDlg, FALSE);
		break;
	case WM_COMMAND:
		switch(HIWORD(wParam))
		{
		case BN_CLICKED:
			switch(LOWORD(wParam))
			{
			case IDC_SAVE_SETTINGS:
			case IDOK:
				{
					BOOL fFixThumbnail = FALSE;

					if (LOWORD(wParam) != IDC_SAVE_SETTINGS)
					{
						if (v->fAutomaticRecordAll == TRUE)
						{
							if (CheckActiveArchiveChannels(hDlg, FALSE) == FALSE)
							{
								int nWaitCount = 100;

								if (hWaitingForEPGDialog == NULL)
								{
									hWaitingForEPGDialog = CreateDialog(v->hInstance, MAKEINTRESOURCE(IDD_ARCHIVE_WAIT_EPG), hDlg, ArchiveWaitEPGDlgProc);
									ShowWindow(hWaitingForEPGDialog, SW_SHOW);
								}

								while (nWaitCount--)
								{
									MSG msg;

									while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
									{
										TranslateMessage(&msg);
										DispatchMessage(&msg);
									}
									Sleep(10);
								}
								PostMessage(hDlg, WM_COMMAND, IDOK, 0);
								break;
							}
							else
							{
								if (hWaitingForEPGDialog != NULL)
									DestroyWindow(hWaitingForEPGDialog);
							}
						}
						else
						{
							if (CheckActiveArchiveChannels(hDlg, TRUE) == FALSE)
								break;
						}
					}

					archive->nBlockSize = GetDlgItemInt(hDlg, IDC_ARCHIVE_BLOCKSIZE, NULL, FALSE);
					if (archive->nBlockSize == 0)
					{
						MessageBox(hDlg, "Block size must be set to a non-zero value", gszAppName, MB_ICONSTOP);
						SetFocus(GetDlgItem(hDlg, IDC_ARCHIVE_BLOCKSIZE));
						break;
					}
					archive->nAutoRemoveOldFiles = IsDlgButtonChecked(hDlg, IDC_ARCHIVE_AUTO_DELETE);
					if (archive->nAutoRemoveOldFiles)
					{
						if (IsDlgButtonChecked(hDlg, IDC_ARCHIVE_AUTO_DELETE_MOVE))
						{
							archive->nAutoRemoveOldFiles = 2;
							GetDlgItemText(hDlg, IDC_ARCHIVE_AUTO_DELETE_MOVE_LOCATION, archive->szAutoRemoveLocation, sizeof(archive->szAutoRemoveLocation));
						}
					}
					archive->nAutoRemoveSizeGB = GetDlgItemInt(hDlg, IDC_ARCHIVE_AUTO_DELETE_GB, NULL, FALSE);
					if (archive->nAutoRemoveOldFiles && archive->nAutoRemoveSizeGB <= 0)
					{
						MessageBox(hDlg, "Auto remove parameter is incorrect - must be greater than zero", gszAppName, MB_ICONWARNING);
						SetFocus(GetDlgItem(hDlg, IDC_ARCHIVE_AUTO_DELETE_GB));
						break;
					}
					archive->nWarningGB = GetDlgItemInt(hDlg, IDC_ARCHIVE_WARNING_GB, NULL, FALSE);
					archive->nErrorGB = GetDlgItemInt(hDlg, IDC_ARCHIVE_ERROR_GB, NULL, FALSE);
					archive->fAutoRestart = IsDlgButtonChecked(hDlg, IDC_ARCHIVE_RESTART);

					if (v->nThumbnailProcessingThreadPriority == 3)
						fFixThumbnail = TRUE;
					if (v->nESParsingCounterReload != 60)
						fFixThumbnail = TRUE;
					if (v->fThumbnailThreadAnimated == TRUE)
						fFixThumbnail = TRUE;
					if (v->fAudioThumbnails == TRUE)
						fFixThumbnail = TRUE;
					if (v->nMaximumThumbnailThreads != 1)
						fFixThumbnail = TRUE;
					if (fFixThumbnail && LOWORD(wParam) != IDC_SAVE_SETTINGS)
					{
						if (MessageBox(hDlg, "The thumbnail generation settings are incorrect. Would you like these fixed?", gszAppName, MB_ICONQUESTION | MB_YESNO | MB_DEFBUTTON1) == IDYES)
						{
							SendMessage(v->hWndMainWindow, WM_COMMAND, ID_SETTINGS_THUMBNAILTHREADPRIORITY_NORMAL, 0);
							if (v->fThumbnailThreadAnimated == TRUE)
								SendMessage(v->hWndMainWindow, WM_COMMAND, ID_SETTINGS_THUMBNAILTHREADPRIORITY_ANIMATED, 0);
							if (v->fAudioThumbnails == TRUE)
								SendMessage(v->hWndMainWindow, WM_COMMAND, ID_SETTINGS_THUMBNAILTHREAD_ENABLEAUDIOTHUMBNAILS, 0);								
							v->nESParsingCounterReload = 60;
							v->nMaximumThumbnailThreads = 1;
						}
					}
					SaveArchiveSettings();
					if (LOWORD(wParam) != IDC_SAVE_SETTINGS)
					{
						v->fArchiveRunning = TRUE;
						EndDialog(hDlg, TRUE);
					}
				}
				break;
			case IDCANCEL:
				EndDialog(hDlg, FALSE);
				break;
			case IDC_ARCHIVE_AUTO_DELETE:
				{
					BOOL fEnableLocation = IsDlgButtonChecked(hDlg, IDC_ARCHIVE_AUTO_DELETE) && IsDlgButtonChecked(hDlg, IDC_ARCHIVE_AUTO_DELETE_MOVE);
					EnableWindow(GetDlgItem(hDlg, IDC_ARCHIVE_AUTO_DELETE_GB), IsDlgButtonChecked(hDlg, IDC_ARCHIVE_AUTO_DELETE));
					EnableWindow(GetDlgItem(hDlg, IDC_ARCHIVE_AUTO_DELETE_DELETE), IsDlgButtonChecked(hDlg, IDC_ARCHIVE_AUTO_DELETE));
					EnableWindow(GetDlgItem(hDlg, IDC_ARCHIVE_AUTO_DELETE_MOVE), IsDlgButtonChecked(hDlg, IDC_ARCHIVE_AUTO_DELETE));
					EnableWindow(GetDlgItem(hDlg, IDC_ARCHIVE_AUTO_DELETE_MOVE_LOCATION), fEnableLocation);
				}
				break;
			case IDC_ARCHIVE_AUTO_DELETE_DELETE:
			case IDC_ARCHIVE_AUTO_DELETE_MOVE:
				EnableWindow(GetDlgItem(hDlg, IDC_ARCHIVE_AUTO_DELETE_MOVE_LOCATION), IsDlgButtonChecked(hDlg, IDC_ARCHIVE_AUTO_DELETE_MOVE));
				break;
			case IDC_ARCHIVE_CHANNEL_ENABLED:
				archive->ap[archive->nListSelectedPMT]->fEnabled = IsDlgButtonChecked(hDlg, IDC_ARCHIVE_CHANNEL_ENABLED);
				ListView_RedrawItems(GetDlgItem(hDlg, IDC_ARCHIVE_CHANNEL_LIST), archive->nListSelectedProgram, archive->nListSelectedProgram);
				SetCalculatedMemory(hDlg);
				break;
			case IDC_ARCHIVE_CHANNEL_CURRENTEPG:
				archive->ap[archive->nListSelectedPMT]->nEPGType = EPG_TYPE_LOCAL_EPG;
				ListView_RedrawItems(GetDlgItem(hDlg, IDC_ARCHIVE_CHANNEL_LIST), archive->nListSelectedProgram, archive->nListSelectedProgram);
				EnableWindow(GetDlgItem(hDlg, IDC_ARCHIVE_CHANNEL_EPG_SERVER), FALSE);
				break;
			case IDC_ARCHIVE_CHANNEL_REMOTE_EPG:
				archive->ap[archive->nListSelectedPMT]->nEPGType = EPG_TYPE_REMOTE_EPG;
				ListView_RedrawItems(GetDlgItem(hDlg, IDC_ARCHIVE_CHANNEL_LIST), archive->nListSelectedProgram, archive->nListSelectedProgram);
				EnableWindow(GetDlgItem(hDlg, IDC_ARCHIVE_CHANNEL_EPG_SERVER), TRUE);
				SetFocus(GetDlgItem(hDlg, IDC_ARCHIVE_CHANNEL_EPG_SERVER));
				break;
			case IDC_ARCHIVE_CHANNEL_HALFHOUR:
				archive->ap[archive->nListSelectedPMT]->nEPGType = EPG_TYPE_30_MIN;
				ListView_RedrawItems(GetDlgItem(hDlg, IDC_ARCHIVE_CHANNEL_LIST), archive->nListSelectedProgram, archive->nListSelectedProgram);
				EnableWindow(GetDlgItem(hDlg, IDC_ARCHIVE_CHANNEL_EPG_SERVER), FALSE);
				break;
			case IDC_ARCHIVE_CHANNELS_DEFAULT_NAME:
				SetupDefaultChannelNames(hDlg);
				break;
			case IDC_ARCHIVE_SELECT_ALARM_SOUND:
				{
					OPENFILENAME ofn;
					char szEditAlarmSoundFilename[MAX_PATH] = {0};

					lstrcpy(szEditAlarmSoundFilename, archive->szAlarmSoundFilename);

					memset( &(ofn), 0, sizeof(ofn));
					ofn.lStructSize	= sizeof(ofn);
					ofn.hwndOwner = hDlg;
					ofn.lpstrFile = szEditAlarmSoundFilename;
					ofn.nMaxFile = MAX_PATH;
					ofn.lpstrFilter = TEXT("Wave Files (*.wav)\0*.wav\0All Files (*.*)\0*.*\0\0");	
					ofn.lpstrTitle = TEXT("Select Archive Alarm Sound");
					ofn.lpstrDefExt = TEXT("wav");
					ofn.Flags =  OFN_HIDEREADONLY;
					ofn.lpstrInitialDir = v->szRecordPIDFolder;				
					if (SourceHelper_myGetOpenFileName(&ofn) == TRUE)
						lstrcpy(archive->szAlarmSoundFilename, szEditAlarmSoundFilename);
				}
				break;
			case IDC_ARCHIVE_CLONE:
				{
					int nPMTIndex;

					if (archive->nListSelectedPMT == -1)
					{
						MessageBeep(0);
						break;
					}
					for (nPMTIndex = 0; nPMTIndex < MAX_PAT_ENTRIES; nPMTIndex++)
					{
						if (v->pat.pmt[nPMTIndex].nPMTPID == 0)
							break;
						if (v->pat.pmt[nPMTIndex].nProgramNumber != 0)
						{
							if (nPMTIndex != archive->nListSelectedPMT)
							{
								archive->ap[nPMTIndex]->fEnabled = archive->ap[archive->nListSelectedPMT]->fEnabled;
								archive->ap[nPMTIndex]->nEPGType = archive->ap[archive->nListSelectedPMT]->nEPGType;
								lstrcpy(archive->ap[nPMTIndex]->szOutputLocation, archive->ap[archive->nListSelectedPMT]->szOutputLocation);
							}
						}
					}
					ListView_RedrawItems(GetDlgItem(hDlg, IDC_ARCHIVE_CHANNEL_LIST), 0, ListView_GetItemCount(GetDlgItem(hDlg, IDC_ARCHIVE_CHANNEL_LIST)));
					SetCalculatedMemory(hDlg);
				}
				break;
			}
			break;
		case EN_CHANGE:
			switch(LOWORD(wParam))
			{
			case IDC_ARCHIVE_CHANNEL_EPG_SERVER:
				GetDlgItemText(hDlg, IDC_ARCHIVE_CHANNEL_EPG_SERVER, archive->ap[archive->nListSelectedPMT]->szRemoteEPGServer, sizeof(archive->ap[archive->nListSelectedPMT]->szRemoteEPGServer));
				break;
			case IDC_ARCHIVE_BLOCKSIZE:
				SetCalculatedMemory(hDlg);
				break;
			case IDC_ARCHIVE_CHANNEL_OUTPUT_LOCATION:
				GetDlgItemText(hDlg, IDC_ARCHIVE_CHANNEL_OUTPUT_LOCATION, archive->ap[archive->nListSelectedPMT]->szOutputLocation, sizeof(archive->ap[archive->nListSelectedPMT]->szOutputLocation));
				ListView_RedrawItems(GetDlgItem(hDlg, IDC_ARCHIVE_CHANNEL_LIST), archive->nListSelectedProgram, archive->nListSelectedProgram);
				break;
			case IDC_ARCHIVE_CHANNEL_OUTPUT_NAME:
				GetDlgItemText(hDlg, IDC_ARCHIVE_CHANNEL_OUTPUT_NAME, archive->ap[archive->nListSelectedPMT]->szChannelName, sizeof(archive->ap[archive->nListSelectedPMT]->szChannelName));
				ListView_RedrawItems(GetDlgItem(hDlg, IDC_ARCHIVE_CHANNEL_LIST), archive->nListSelectedProgram, archive->nListSelectedProgram);
				break;
			}
			break;
		}
		break;
	case WM_NOTIFY: 
		switch (((LPNMHDR) lParam)->code)
		{ 
		case LVN_GETDISPINFO:
			{
				NMLVDISPINFO * pnmv = (NMLVDISPINFO *)lParam;
				GetArchiveChannelListDispInfo((LV_DISPINFO *) lParam);
			}
			break;
		case LVN_ITEMCHANGED:
			{
				LPNMLISTVIEW pnmv = (LPNMLISTVIEW)lParam;
				if (pnmv->uNewState & LVIS_SELECTED)
				{
					if (archive->nListSelectedProgram == -1)
					{
						// First time - enable controls
						EnableWindow(GetDlgItem(hDlg, IDC_ARCHIVE_CHANNEL_ENABLED), TRUE);
						EnableWindow(GetDlgItem(hDlg, IDC_ARCHIVE_CHANNEL_OUTPUT_LOCATION), TRUE);
						EnableWindow(GetDlgItem(hDlg, IDC_ARCHIVE_CHANNEL_OUTPUT_NAME), TRUE);
						EnableWindow(GetDlgItem(hDlg, IDC_ARCHIVE_CHANNEL_CURRENTEPG), TRUE);
						EnableWindow(GetDlgItem(hDlg, IDC_ARCHIVE_CHANNEL_REMOTE_EPG), TRUE);
						EnableWindow(GetDlgItem(hDlg, IDC_ARCHIVE_CHANNEL_EPG_SERVER), TRUE);
						EnableWindow(GetDlgItem(hDlg, IDC_ARCHIVE_CHANNEL_HALFHOUR), TRUE);
					}
					archive->nListSelectedProgram = pnmv->iItem;
					archive->nListSelectedPMT = (int)pnmv->lParam;
					CheckDlgButton(hDlg, IDC_ARCHIVE_CHANNEL_ENABLED, archive->ap[archive->nListSelectedPMT]->fEnabled);
					SetDlgItemText(hDlg, IDC_ARCHIVE_CHANNEL_OUTPUT_LOCATION, archive->ap[archive->nListSelectedPMT]->szOutputLocation);
					SetDlgItemText(hDlg, IDC_ARCHIVE_CHANNEL_OUTPUT_NAME, archive->ap[archive->nListSelectedPMT]->szChannelName);
					SetDlgItemText(hDlg, IDC_ARCHIVE_CHANNEL_EPG_SERVER, archive->ap[archive->nListSelectedPMT]->szRemoteEPGServer);
					switch(archive->ap[archive->nListSelectedPMT]->nEPGType)
					{
					case EPG_TYPE_LOCAL_EPG:
						CheckDlgButton(hDlg, IDC_ARCHIVE_CHANNEL_CURRENTEPG, TRUE);
						CheckDlgButton(hDlg, IDC_ARCHIVE_CHANNEL_REMOTE_EPG, FALSE);
						CheckDlgButton(hDlg, IDC_ARCHIVE_CHANNEL_HALFHOUR, FALSE);
						EnableWindow(GetDlgItem(hDlg, IDC_ARCHIVE_CHANNEL_EPG_SERVER), FALSE);
						break;
					case EPG_TYPE_REMOTE_EPG:
						CheckDlgButton(hDlg, IDC_ARCHIVE_CHANNEL_CURRENTEPG, FALSE);
						CheckDlgButton(hDlg, IDC_ARCHIVE_CHANNEL_REMOTE_EPG, TRUE);
						CheckDlgButton(hDlg, IDC_ARCHIVE_CHANNEL_HALFHOUR, FALSE);
						EnableWindow(GetDlgItem(hDlg, IDC_ARCHIVE_CHANNEL_EPG_SERVER), TRUE);
						break;
					case EPG_TYPE_30_MIN:
						CheckDlgButton(hDlg, IDC_ARCHIVE_CHANNEL_CURRENTEPG, FALSE);
						CheckDlgButton(hDlg, IDC_ARCHIVE_CHANNEL_REMOTE_EPG, FALSE);
						CheckDlgButton(hDlg, IDC_ARCHIVE_CHANNEL_HALFHOUR, TRUE);
						EnableWindow(GetDlgItem(hDlg, IDC_ARCHIVE_CHANNEL_EPG_SERVER), FALSE);
						break;
					}
				}
			}
			break;
		}
		break;
	}

	return FALSE;
}

void CALLBACK FileIOCompletionRoutine(DWORD dwErrorCode, DWORD dwNumberOfBytesTransfered, LPOVERLAPPED lpOverlapped)
{
	int nPMTIndex = (int)(LONG_PTR)lpOverlapped->hEvent;
	int nAyncBufferIndex;
	char szTemp[128];

	for (nAyncBufferIndex = 0; nAyncBufferIndex < MAX_ASYNC_IO; nAyncBufferIndex++)
	{
		if (lpOverlapped == &archive->ap[nPMTIndex]->overlappedWrites[nAyncBufferIndex])
		{
			archive->ap[nPMTIndex]->hAsyncOutputFile[nAyncBufferIndex] = NULL;
			archive->ap[nPMTIndex]->nWriteBufferOffset[nAyncBufferIndex] = 0;
			memset(&archive->ap[nPMTIndex]->overlappedWrites[nAyncBufferIndex], 0, sizeof(OVERLAPPED));
			return;
		}
	}
	wsprintf(szTemp, "?Unable to locate overlapped I/O channel %d\n", v->pat.pmt[nPMTIndex].nProgramNumber);
	OutputDebugString(szTemp);
}

BOOL FlushBufferData(int nPMTIndex)
{
	archive->ap[nPMTIndex]->overlappedWrites[archive->ap[nPMTIndex]->nCurrentAsyncIO].Offset = archive->ap[nPMTIndex]->lnCurrentFileWritePtr.LowPart;
	archive->ap[nPMTIndex]->overlappedWrites[archive->ap[nPMTIndex]->nCurrentAsyncIO].OffsetHigh = archive->ap[nPMTIndex]->lnCurrentFileWritePtr.HighPart;

	archive->ap[nPMTIndex]->overlappedWrites[archive->ap[nPMTIndex]->nCurrentAsyncIO].hEvent = (HANDLE)(LONG_PTR)nPMTIndex;
	archive->ap[nPMTIndex]->hAsyncOutputFile[archive->ap[nPMTIndex]->nCurrentAsyncIO] = archive->ap[nPMTIndex]->hOutputFile;
	if (WriteFileEx(archive->ap[nPMTIndex]->hOutputFile,
					archive->ap[nPMTIndex]->pBlockBuffer[archive->ap[nPMTIndex]->nCurrentAsyncIO],
					archive->ap[nPMTIndex]->nWriteBufferOffset[archive->ap[nPMTIndex]->nCurrentAsyncIO],
					&archive->ap[nPMTIndex]->overlappedWrites[archive->ap[nPMTIndex]->nCurrentAsyncIO],
					FileIOCompletionRoutine) == FALSE)
	{
		int nGLE = GetLastError();
		char szTemp[128];
		wsprintf(szTemp, "WriteFileEx returned %d on channel %d\n", nGLE, v->pat.pmt[nPMTIndex].nProgramNumber);
		OutputDebugString(szTemp);
		archive->ap[nPMTIndex]->nWriteBufferOffset[archive->ap[nPMTIndex]->nCurrentAsyncIO] = 0;
		return FALSE;
	}
	EnterCriticalSection(&archive->ap[nPMTIndex]->csThumbnailList);	
	archive->ap[nPMTIndex]->lnCurrentFileWritePtr.QuadPart += archive->ap[nPMTIndex]->nWriteBufferOffset[archive->ap[nPMTIndex]->nCurrentAsyncIO];
	LeaveCriticalSection(&archive->ap[nPMTIndex]->csThumbnailList);

	archive->ap[nPMTIndex]->nCurrentAsyncIO++;
	if (archive->ap[nPMTIndex]->nCurrentAsyncIO >= MAX_ASYNC_IO)
		archive->ap[nPMTIndex]->nCurrentAsyncIO = 0;
	if (archive->ap[nPMTIndex]->nWriteBufferOffset[archive->ap[nPMTIndex]->nCurrentAsyncIO] != 0)
	{
		char szTemp[128];
		wsprintf(szTemp, "No async I/O channel %d nCurrentAsyncIO = %d\n", v->pat.pmt[nPMTIndex].nProgramNumber, archive->ap[nPMTIndex]->nCurrentAsyncIO);
		OutputDebugString(szTemp);
		return FALSE;
	}
	//archive->ap[nPMTIndex]->nWriteBufferOffset = 0;
	//if (dwWritten != (DWORD)archive->ap[nPMTIndex]->nWriteBufferOffset)
	//{
	//	return FALSE;
	//}

	return TRUE;
}

void WriteArchiveBlockBuffer(int nPMTIndex, BYTE * pData, int nLength)
{
	int nRemainingInBuffer = (archive->nBlockSize * 1024) - archive->ap[nPMTIndex]->nWriteBufferOffset[archive->ap[nPMTIndex]->nCurrentAsyncIO];
	int nRemainingThisPacket = nLength;

	if (nRemainingInBuffer >= nRemainingThisPacket)
	{
		memcpy(&archive->ap[nPMTIndex]->pBlockBuffer[archive->ap[nPMTIndex]->nCurrentAsyncIO][archive->ap[nPMTIndex]->nWriteBufferOffset[archive->ap[nPMTIndex]->nCurrentAsyncIO]],
			   pData,
			   nRemainingThisPacket);
		archive->ap[nPMTIndex]->nWriteBufferOffset[archive->ap[nPMTIndex]->nCurrentAsyncIO] += nRemainingThisPacket;
	}
	else
	{
		memcpy(&archive->ap[nPMTIndex]->pBlockBuffer[archive->ap[nPMTIndex]->nCurrentAsyncIO][archive->ap[nPMTIndex]->nWriteBufferOffset[archive->ap[nPMTIndex]->nCurrentAsyncIO]],
			   pData,
			   nRemainingInBuffer);
		archive->ap[nPMTIndex]->nWriteBufferOffset[archive->ap[nPMTIndex]->nCurrentAsyncIO] += nRemainingInBuffer;
		if (FlushBufferData(nPMTIndex) == TRUE)
		{
			memcpy(&archive->ap[nPMTIndex]->pBlockBuffer[archive->ap[nPMTIndex]->nCurrentAsyncIO][archive->ap[nPMTIndex]->nWriteBufferOffset[archive->ap[nPMTIndex]->nCurrentAsyncIO]],
				   &pData[nRemainingInBuffer],
				   nRemainingThisPacket - nRemainingInBuffer);
			archive->ap[nPMTIndex]->nWriteBufferOffset[archive->ap[nPMTIndex]->nCurrentAsyncIO] = nRemainingThisPacket - nRemainingInBuffer;
			archive->ap[nPMTIndex]->nStatus &= ~STATUS_NO_ASYNC_IO;
		}
		else
			archive->ap[nPMTIndex]->nStatus |= STATUS_NO_ASYNC_IO;
	}
}

void GetRecordedString(char * szOutput, double dTotal)
{
	if (dTotal / 1024.0 / 1024.0 / 1024.0 > 1000)
		sprintf(szOutput, "%.3f TB", dTotal / 1024.0 / 1024.0 / 1024.0 / 1024.0);
	else if (dTotal / 1024.0 / 1024.0 > 1000)
		sprintf(szOutput, "%.3f GB", dTotal / 1024.0 / 1024.0 / 1024.0);
	else			
		sprintf(szOutput, "%.1f MB", dTotal / 1024.0 / 1024.0);
}

void SetupArchiveStatusFonts(HDC hDC)
{
	archive->hSmallTextFont = CreateFont(-MulDiv(75, GetDeviceCaps(hDC, LOGPIXELSY), 720),
							   0,
							   0,
							   0,
							   400,
							   FALSE,
							   FALSE,
							   FALSE,
							   ANSI_CHARSET,
							   OUT_DEFAULT_PRECIS,
							   CLIP_DEFAULT_PRECIS,
							   ANTIALIASED_QUALITY,
							   FF_DONTCARE | VARIABLE_PITCH,
							   "Arial");	
	archive->hSmallTextFontBold = CreateFont(-MulDiv(90, GetDeviceCaps(hDC, LOGPIXELSY), 720),
							   0,
							   0,
							   0,
							   500,
							   FALSE,
							   FALSE,
							   FALSE,
							   ANSI_CHARSET,
							   OUT_DEFAULT_PRECIS,
							   CLIP_DEFAULT_PRECIS,
							   ANTIALIASED_QUALITY,
							   FF_DONTCARE | VARIABLE_PITCH,
							   "Arial");	
}

DWORD WINAPI BeepThread(LPVOID pvarg)
{
	int nErrorBeep = 0;
	int nErrorBit;
	DWORD i;
	DWORD dwLastErrorTime[sizeof(archive->nErrorBeep) * 8];
	
	archive->fBeepThreadRunning = TRUE;
	
	for (nErrorBit = 0; nErrorBit < sizeof(archive->nErrorBeep) * 8; nErrorBit++)
		dwLastErrorTime[nErrorBit] = 0;
	
	Sleep(1000);	// snooze while the data thread starts
	while (archive->fTerminateBeepThread == FALSE)
	{
		// See if the state has changed so we can send warnings
		if (archive->nErrorBeep != nErrorBeep)
		{
			DWORD dwBitCompare = 1;
			DWORD dwNotifyBits = 0;

			for (nErrorBit = 0; nErrorBit < sizeof(archive->nErrorBeep) * 8; nErrorBit++)
			{
				if (archive->nErrorBeep & dwBitCompare)
				{
					// Error bit is on - see if a timer is running
					if (dwLastErrorTime[nErrorBit])
					{
						// timer running. expired?
						if (GetTickCount() > dwLastErrorTime[nErrorBit] + (5 * 60 * 1000))
							dwLastErrorTime[nErrorBit] = 0;
					}
					if (!dwLastErrorTime[nErrorBit])
					{
						// timer not running so send email
						dwLastErrorTime[nErrorBit] = GetTickCount();
						dwNotifyBits |= dwBitCompare;
					}
				}
				dwBitCompare = dwBitCompare << 1;
			}
			if (dwNotifyBits)
			{
				char szError[1024] = {0};

				wsprintf(szError, "%s: ", v->szEmailFrom);
				if (dwNotifyBits & ERROR_BEEP_MPEG)
				{
					char szTemp[128];
					wsprintf(szTemp, "MPEG error channel %d ", v->pat.pmt[archive->nErrorPMTIndex].nProgramNumber);
					lstrcat(szError, szTemp);
				}
				if (dwNotifyBits & ERROR_BEEP_DISK_SPACE)
					lstrcat(szError, "Low space ");
				ArchiveEventListWrite(EVENT_ICON_WARNING, szError, "");
			}
			nErrorBeep = archive->nErrorBeep;
		}

		// Now actually do the beep
		if (archive->nErrorBeep & !archive->fSilent)
		{
			PlaySound(archive->szAlarmSoundFilename, NULL, SND_FILENAME | SND_SYNC);
			for (i = 0; i < 100 && archive->fTerminateBeepThread == FALSE; i++)
				Sleep(100);
		}
		else
			Sleep(100);
	}

	archive->fTerminateBeepThread = FALSE;
	archive->fBeepThreadRunning = FALSE;
	
	return 0;
}

BOOL LocateXMLTag(char * szXMLBuffer, char * szTag, char * szDestination, int nDestinationSize)
{
	int nOutputSize;
	char * szStartTagPtr, * szEndTagPtr;
	char szStartTag[128];
	char szEndTag[128];

	wsprintf(szStartTag, "<%s>", szTag);
	wsprintf(szEndTag, "</%s>", szTag);

	szStartTagPtr = strstr(szXMLBuffer, szStartTag);
	if (szStartTagPtr == NULL)
		return FALSE;
	szEndTagPtr = strstr(szStartTagPtr, szEndTag);
	if (szEndTagPtr == NULL)
		return FALSE;
	szStartTagPtr += lstrlen(szStartTag);
	nOutputSize = (int)(szEndTagPtr - szStartTagPtr);
	if (nOutputSize > nDestinationSize - 1)
		nOutputSize = nDestinationSize - 1;
	memcpy(szDestination, szStartTagPtr, nOutputSize);
	szDestination[nOutputSize] = '\0';

	return TRUE;
}
 
BOOL ParseArchiveXML(HANDLE hInputXMLFile, PARCHIVEDPROGRAMS thisarcprog)
{
	BOOL fRetVal = TRUE;
	BYTE * pXML;
	DWORD dwInputFileSize = GetFileSize(hInputXMLFile, NULL);
	HANDLE hMap;

	hMap = CreateFileMapping(hInputXMLFile, NULL, PAGE_READONLY | SEC_COMMIT, 0,  (DWORD)dwInputFileSize, NULL);
	if (hMap == NULL)
		return FALSE;

	pXML = (BYTE *)MapViewOfFile(hMap, FILE_MAP_READ, 0, 0, (DWORD)dwInputFileSize);
	if (pXML == NULL)
	{
		CloseHandle(hMap);
		return FALSE;
	}

	if (LocateXMLTag(pXML, "CHANNEL", thisarcprog->szChannel, sizeof(thisarcprog->szChannel)) == FALSE)
		fRetVal = FALSE;
	if (LocateXMLTag(pXML, "ACTUAL-START-DATE", thisarcprog->szStartDate, sizeof(thisarcprog->szStartDate)) == FALSE)
	{
		if (LocateXMLTag(pXML, "START-DATE", thisarcprog->szStartDate, sizeof(thisarcprog->szStartDate)) == FALSE)
			fRetVal = FALSE;
	}
	if (LocateXMLTag(pXML, "ACTUAL-START-TIME", thisarcprog->szStartTime, sizeof(thisarcprog->szStartTime)) == FALSE)
	{
		if (LocateXMLTag(pXML, "START-TIME", thisarcprog->szStartTime, sizeof(thisarcprog->szStartTime)) == FALSE)
			fRetVal = FALSE;
	}
	if (LocateXMLTag(pXML, "DURATION", thisarcprog->szDuration, sizeof(thisarcprog->szDuration)) == FALSE)
		fRetVal = FALSE;
	if (LocateXMLTag(pXML, "PROGRAM", thisarcprog->szProgram, sizeof(thisarcprog->szProgram)) == FALSE)
		fRetVal = FALSE;
	if (LocateXMLTag(pXML, "DESCRIPTION", thisarcprog->szDescription, sizeof(thisarcprog->szDescription)) == FALSE)
		fRetVal = FALSE;
	if (fRetVal == TRUE)
	{
		int nThumbnailIndex;
		char szThumbnailXMLData[8192];

		for (nThumbnailIndex = 0; nThumbnailIndex < 512; nThumbnailIndex++)
		{
			char szTargetTag[64];

			wsprintf(szTargetTag, "THUMBNAIL%d", nThumbnailIndex + 1);
			if (LocateXMLTag(pXML, szTargetTag, szThumbnailXMLData, sizeof(szThumbnailXMLData)) == TRUE)
			{
				LocateXMLTag(szThumbnailXMLData, "FILENAME", thisarcprog->at[nThumbnailIndex].szFilename, sizeof(thisarcprog->at[nThumbnailIndex].szFilename));
			}
			else
				break;
		}
	}

	UnmapViewOfFile(pXML);
	CloseHandle(hMap);

	return fRetVal;
}

BOOL MoveArchiveFile(char * szKillFilename)
{
	BOOL fRetVal;
	int nIndex;
	char szNewFile[MAX_PATH];
	char szTemp[MAX_PATH * 2];

	for (nIndex = lstrlen(szKillFilename); nIndex > 0; nIndex--)
	{
		if (szKillFilename[nIndex] == '\\')
			break;
	}
	if (nIndex == 0)
		return TRUE;
	lstrcpy(szNewFile, archive->szAutoRemoveLocation);
	if (szNewFile[lstrlen(szNewFile) - 1] != '\\')
		lstrcat(szNewFile, "\\");
	lstrcat(szNewFile, &szKillFilename[nIndex + 1]);

	wsprintf(szTemp, "Archiver: Moving %s to %s\n", szKillFilename, szNewFile);
	OutputDebugString(szTemp);
	DeleteFile(szNewFile);
	fRetVal = MoveFile(szKillFilename, szNewFile);
	if (!fRetVal)
	{
		int nLastError = GetLastError();
		char szError[1024];

		if (nLastError != 2)
		{
			wsprintf(szError, "Unable to move (GLE = %d) %s to %s",
					 nLastError, szKillFilename, szNewFile);
			ArchiveEventListWrite(EVENT_ICON_WARNING, szError, "");
		}
		else
			fRetVal = TRUE;	// deleted text file - that's OK
	}

	return fRetVal;
}

DWORD WINAPI RemoveOldestFilesThread(LPVOID lpv)
{
	int nDriveIndex;
	HANDLE hFind;
	ULARGE_INTEGER ulOldestFile;
	WIN32_FIND_DATA fd;
	char szOutputLocation[MAX_PATH];
	char szSearchPath[MAX_PATH];
	char szOldestXMLFile[MAX_PATH] = {0};

	lstrcpy(szOutputLocation, (char *)lpv);
	ulOldestFile.QuadPart = -1;

	lstrcpy(szSearchPath, szOutputLocation);
	if (szSearchPath[lstrlen(szSearchPath) - 1] != '\\')
		lstrcat(szSearchPath, "\\");
	lstrcat(szSearchPath, "*.xml");
	hFind = FindFirstFile(szSearchPath, &fd);
	if (hFind != INVALID_HANDLE_VALUE)
	{
		do
		{
			HANDLE hFile;
			ULARGE_INTEGER ulFileDate;
			char szFilename[MAX_PATH];

			if ((fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == FILE_ATTRIBUTE_DIRECTORY)
				continue;				

			lstrcpy(szFilename, szOutputLocation);
			if (szOutputLocation[lstrlen(szOutputLocation) - 1] != '\\')
				lstrcat(szFilename, "\\");
			lstrcat(szFilename, fd.cFileName);
			hFile = CreateFile(szFilename, GENERIC_READ, FILE_SHARE_READ, (LPSECURITY_ATTRIBUTES)NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, (HANDLE)NULL);
			if (hFile != INVALID_HANDLE_VALUE)
			{
				GetFileTime(hFile, (LPFILETIME)&ulFileDate, NULL, NULL);
				CloseHandle(hFile);
				if (ulFileDate.QuadPart < ulOldestFile.QuadPart)
				{
					ulOldestFile.QuadPart = ulFileDate.QuadPart;
					lstrcpy(szOldestXMLFile, szFilename);
				}
			}
		} while (FindNextFile(hFind, &fd) == TRUE);
		FindClose(hFind);

		// Now szOldestXMLFile has the oldest XML file
		if (lstrlen(szOldestXMLFile))
		{
			HANDLE hFile;

			hFile = CreateFile(szOldestXMLFile, GENERIC_READ, FILE_SHARE_READ, (LPSECURITY_ATTRIBUTES)NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, (HANDLE)NULL);
			if (hFile != INVALID_HANDLE_VALUE)
			{
				ARCHIVEDPROGRAMS deletearcprog;
				char szShortStatus[128 + MAX_PATH];
				char szLongStatus[] = {""};

				memset(&deletearcprog, 0, sizeof(deletearcprog));
				if (ParseArchiveXML(hFile, &deletearcprog) == TRUE)
				{
					int nThumbnailIndex;
					int nKillExtensionIndex;
					char * szExtension;
					char szKillFile[MAX_PATH];
					char * szKillExtensions[] = {".mpg", ".user-data",
												 ".cc1.txt", ".cc2.txt", ".cc3.txt", ".cc4.txt",
												 ".text1.txt", ".text2.txt", ".text3.txt", ".text4.txt",
												 "\0"};

					lstrcpy(szKillFile, szOldestXMLFile);
					szExtension = strstr(szKillFile, ".xml");
					for (nKillExtensionIndex = 0; szKillExtensions[nKillExtensionIndex][0] != '\0'; nKillExtensionIndex++)
					{
						lstrcpy(szExtension, szKillExtensions[nKillExtensionIndex]);
						if (archive->nAutoRemoveOldFiles == 2)
						{
							if (MoveArchiveFile(szKillFile) != FALSE)
								DeleteFile(szKillFile);
						}
						else
							DeleteFile(szKillFile);
					}

					for (nThumbnailIndex = 0; nThumbnailIndex < MAX_THUMBNAILS; nThumbnailIndex++)
					{
						if (lstrlen(deletearcprog.at[nThumbnailIndex].szFilename))
						{
							char szThumbnailFilename[MAX_PATH];

							lstrcpy(szThumbnailFilename, szOutputLocation);
							if (szThumbnailFilename[lstrlen(szThumbnailFilename) - 1] != '\\')
								lstrcat(szThumbnailFilename, "\\");
							lstrcat(szThumbnailFilename, "jpg\\");
							lstrcat(szThumbnailFilename, deletearcprog.at[nThumbnailIndex].szFilename);
							if (archive->nAutoRemoveOldFiles == 2)
							{
								if (MoveArchiveFile(szThumbnailFilename) != FALSE)
									DeleteFile(szThumbnailFilename);
							}
							else
								DeleteFile(szThumbnailFilename);
						}
					}
				}
				CloseHandle(hFile);
				if (archive->nAutoRemoveOldFiles == 2)
				{
					if (MoveArchiveFile(szOldestXMLFile) != FALSE)
						DeleteFile(szOldestXMLFile);
				}
				else
					DeleteFile(szOldestXMLFile);

				if (archive->nAutoRemoveOldFiles != 2)
					wsprintf(szShortStatus, "Free up: %s", szOldestXMLFile);
				else
					wsprintf(szShortStatus, "Moved: %s", szOldestXMLFile);
				ArchiveEventListWrite(EVENT_ICON_INFORMATION, szShortStatus, szLongStatus);
			}
		}
	}

	lstrcpy(szSearchPath, szOutputLocation);
	strupr(szSearchPath);
	nDriveIndex = szSearchPath[0] - 'A';
	if (nDriveIndex >= 0 && nDriveIndex <= 26)
		archive->bFileRemovalActive[nDriveIndex] = FALSE;

	return 0;
}

void UpdateOutputFreeSpace(HWND hWnd)
{
	int i;
	int nPMTIndex;
	char szTemp[128];

	for (i = 0; i < 26; i++)
		archive->bActiveDrive[i] = FALSE;

	for (nPMTIndex = 0; nPMTIndex < MAX_PAT_ENTRIES; nPMTIndex++)
	{
		if (v->pat.pmt[nPMTIndex].nPMTPID == 0)
			break;
		if (v->pat.pmt[nPMTIndex].nProgramNumber != 0)
		{
			if (archive->ap[nPMTIndex]->fEnabled == TRUE)
			{
				lstrcpy(szTemp, archive->ap[nPMTIndex]->szOutputLocation);
				strupr(szTemp);
				if (szTemp[0] >= 'A' && szTemp[1] <= 'Z')
					archive->bActiveDrive[szTemp[0] - 'A'] = TRUE;
			}
		}
	}

	for (i = 0; i < 26; i++)
	{
		archive->dDriveFreeSpace[i] = 0.0;
		if (archive->bActiveDrive[i] == TRUE)
		{
			ULARGE_INTEGER FreeBytesAvailable;
			ULARGE_INTEGER TotalNumberOfBytes;
			ULARGE_INTEGER TotalNumberOfFreeBytes;
			char szDrive[64];

			wsprintf(szDrive, "%c:", i + 'A');
			GetDiskFreeSpaceEx(szDrive, &FreeBytesAvailable, &TotalNumberOfBytes, &TotalNumberOfFreeBytes);
			archive->dDriveFreeSpace[i] = ((double)(__int64)FreeBytesAvailable.QuadPart / 1024.0 / 1024.0 / 1024.0);
		}
	}

	if (archive->nAutoRemoveOldFiles)
	{
		for (i = 0; i < 26; i++)
		{
			if (archive->bActiveDrive[i] == TRUE)
			{
				if (archive->dDriveFreeSpace[i] < (double)archive->nAutoRemoveSizeGB)
				{
					for (nPMTIndex = 0; nPMTIndex < MAX_PAT_ENTRIES; nPMTIndex++)
					{
						if (v->pat.pmt[nPMTIndex].nPMTPID == 0)
							break;
						if (v->pat.pmt[nPMTIndex].nProgramNumber != 0)
						{
							if (archive->ap[nPMTIndex]->fEnabled == TRUE)
							{
								lstrcpy(szTemp, archive->ap[nPMTIndex]->szOutputLocation);
								strupr(szTemp);
								if (i + 'A' == szTemp[0] && !archive->bFileRemovalActive[i])
								{
									HANDLE hThread;
									DWORD dwThreadID;

									archive->bFileRemovalActive[i] = TRUE;
									hThread = CreateThread(NULL, 0, RemoveOldestFilesThread, (LPVOID)archive->ap[nPMTIndex]->szOutputLocation, 0, &dwThreadID);
									CloseHandle(hThread);
									break;
								}
							}
						}
					}
				}
			}
		}
	}

}

int ArchiveThumbnailCount()
{
	int nArchiveThumbnailCount = 0;
	int nPMTIndex;

	// First make sure there's at least one thumbnail to display (at startup)
	for (nPMTIndex = 0; nPMTIndex < MAX_PAT_ENTRIES; nPMTIndex++)
	{
		if (archive->ap[nPMTIndex] == NULL)
			continue;
		if (archive->ap[nPMTIndex]->fEnabled == TRUE)
		{
			int nESIndex;

			for (nESIndex = 0; nESIndex < MAX_ESLIST_ENTRIES; nESIndex++)
			{
				if (v->pat.pmt[nPMTIndex].es[nESIndex].nESPID == 0)
					break;
				if (   v->pat.pmt[nPMTIndex].es[nESIndex].nStreamType == 0x01 // MPEG-1
					|| v->pat.pmt[nPMTIndex].es[nESIndex].nStreamType == 0x02 // MPEG-2
					|| v->pat.pmt[nPMTIndex].es[nESIndex].nStreamType == 0x10 // MPEG-4
					|| v->pat.pmt[nPMTIndex].es[nESIndex].nStreamType == 0x1b // H.264
					|| v->pat.pmt[nPMTIndex].es[nESIndex].nStreamType == 0x80 // DCII
					|| v->pat.pmt[nPMTIndex].es[nESIndex].nStreamType == 0xea // VC1
					)
				{
					if (v->pat.pmt[nPMTIndex].es[nESIndex].pRGBVideoFrame != NULL)
						nArchiveThumbnailCount++;
				}
			}
		}
	}

	return nArchiveThumbnailCount;
}

INT_PTR CALLBACK ArchiveRunDialogProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg)
	{
	case WM_INITDIALOG:
		{
			HDC hDC;
			HANDLE hThread;
			DWORD dwThreadID;
			char szTemp[MAX_PATH];

			v->hWndArchiveRun = hWnd;
			hDC = GetDC(hWnd);
			SetupArchiveStatusFonts(hDC);
			ReleaseDC(hWnd, hDC);
			
			archive->hGreenBrush = CreateSolidBrush(RGB(0x00, 0xff, 0x00));
			archive->hRedBrush = CreateSolidBrush(RGB(0xff, 0x00, 0x00));
			archive->hGreenPen = CreatePen(PS_SOLID, 1, RGB(0x00, 0xff, 0x00));
			archive->hYellowPen = CreatePen(PS_SOLID, 1, RGB(0xff, 0xff, 0x00));
			
			archive->hStatusIcon[EVENT_ICON_INFORMATION - 1] = LoadImage(v->hInstance, MAKEINTRESOURCE(IDI_LOG_INFO), IMAGE_ICON, 16, 16, 0);
			archive->hStatusIcon[EVENT_ICON_STOP - 1] = LoadImage(v->hInstance, MAKEINTRESOURCE(IDI_LOG_STOP), IMAGE_ICON, 16, 16, 0);
			archive->hStatusIcon[EVENT_ICON_WARNING - 1] = LoadImage(v->hInstance, MAKEINTRESOURCE(IDI_LOG_WARNING), IMAGE_ICON, 16, 16, 0);

			SetTimer(hWnd, 1, 250, NULL);
			SetTimer(hWnd, 2, 1000, NULL);

			if (lstrlen(v->szProfileName))
				wsprintf(szTemp, "Archive Status - %s", v->szProfileName);
			else
				lstrcpy(szTemp, "Archive Status - Default");
			SendMessage(hWnd, WM_SETTEXT, 0, (LPARAM)szTemp);

			if (v->nArchiveWindowW && v->nArchiveWindowH)
				SetWindowPos(hWnd, NULL, 
				             v->nArchiveWindowX, v->nArchiveWindowY,
							 v->nArchiveWindowW, v->nArchiveWindowH,
							 0);
			archive->fNeedNewThumbnail = TRUE;

			hThread = CreateThread(NULL, 0, BeepThread, (LPVOID)0, 0, &dwThreadID);
			CloseHandle(hThread);
#ifndef _DEBUG
			ShowWindow(GetDlgItem(hWnd, IDC_ARCHIVE_RUN_DEBUG), SW_HIDE);
			EnableWindow(GetDlgItem(hWnd, IDC_ARCHIVE_RUN_DEBUG), FALSE);
#endif _DEBUG
			CheckDlgButton(hWnd, IDC_ARCHIVE_SILENT, archive->fSilent);
			CheckDlgButton(hWnd, IDC_ARCHIVE_LOCAL_TIME, archive->fLocalTime);
		}
		break;
	case WM_CLOSE:
		{
			v->fArchiveTerminate = FALSE;
			if (v->fStopping == TRUE)
				v->fArchiveTerminate = TRUE;
			else
				if (MessageBox(hWnd, "This will terminate archiving. Are you sure you want to stop?", gszAppName, MB_ICONQUESTION | MB_YESNO | MB_DEFBUTTON2) == IDYES)
					v->fArchiveTerminate = TRUE;
			if (v->fArchiveTerminate)
				PostMessage(v->hWndMainWindow, WM_COMMAND, ID_RECORD_RECORDALLPROGRAMS, 0);
			else
				return FALSE;
		}
		break;
	case WM_DESTROY:
		{
			BOOL fRemoveThreadActive;
			RECT rcArchiveWindow;

			archive->fTerminateBeepThread = TRUE;
			while (archive->fBeepThreadRunning == TRUE)
				Sleep(50);

			do
			{
				int i;
				
				fRemoveThreadActive = FALSE;
				for (i = 0; i < 26; i++)
				{
					if (archive->bFileRemovalActive[i] == TRUE)
					{
						fRemoveThreadActive = TRUE;
						break;
					}
				}
				if (fRemoveThreadActive)
					Sleep(50);
			} while (fRemoveThreadActive);

			KillTimer(hWnd, 1);
			KillTimer(hWnd, 2);
			DeleteObject(archive->hSmallTextFont);
			DeleteObject(archive->hSmallTextFontBold);
			DeleteObject(archive->hGreenBrush);
			DeleteObject(archive->hRedBrush);
			DeleteObject(archive->hGreenPen);
			DeleteObject(archive->hYellowPen);
			
			DestroyIcon(archive->hStatusIcon[EVENT_ICON_INFORMATION - 1]);
			DestroyIcon(archive->hStatusIcon[EVENT_ICON_STOP - 1]);
			DestroyIcon(archive->hStatusIcon[EVENT_ICON_WARNING - 1]);

			GetWindowRect(hWnd, &rcArchiveWindow);
			v->nArchiveWindowX = rcArchiveWindow.left;
			v->nArchiveWindowY = rcArchiveWindow.top;
			v->nArchiveWindowW = rcArchiveWindow.right - rcArchiveWindow.left;
			v->nArchiveWindowH = rcArchiveWindow.bottom - rcArchiveWindow.top;

			//ImageList_Destroy(archive->ghimlEventSmall);
		}
		break;
	case WM_COMMAND:
		switch(LOWORD(wParam))
		{
#ifdef _DEBUG
		case IDC_ARCHIVE_RUN_DEBUG:
			ArchiveEventListWrite(EVENT_ICON_WARNING, "Debug", "debug long");
			break;
#endif _DEBUG
		case IDC_ARCHIVE_SILENT:
			archive->fSilent = IsDlgButtonChecked(hWnd, IDC_ARCHIVE_SILENT);
			SaveArchiveSettings();
			break;
		case IDC_ARCHIVE_LOCAL_TIME:
			archive->fLocalTime = IsDlgButtonChecked(hWnd, IDC_ARCHIVE_LOCAL_TIME);
			SaveArchiveSettings();
			InvalidateRect(hWnd, NULL, FALSE);
			break;
		case IDC_ARCHIVE_RUN_FORCE_NEW:
			archive->fForceNew = TRUE;
			break;
		case IDC_ARCHIVE_RUN_RESET_MIN_MAX:
			{
				int nPMTIndex;

				for (nPMTIndex = 0; nPMTIndex < MAX_PAT_ENTRIES; nPMTIndex++)
				{
					if (v->pat.pmt[nPMTIndex].nPMTPID == 0)
						break;
					if (v->pat.pmt[nPMTIndex].nProgramNumber != 0)
					{
						if (archive->ap[nPMTIndex]->fEnabled == TRUE)
						{
							archive->ap[nPMTIndex]->nGraphBytesMin = 1024 * 1024 * 1024;
							archive->ap[nPMTIndex]->nGraphBytesMax = 0;
						}
					}
				}
				archive->nMaxIOsPending = 0;
			}
			break;
		}
		break;
	case WM_ACTIVATE:
		PostMessage(v->hWndMainWindow, WM_ACTIVATE, 0, 0);
		break;
	case WM_TIMER:
		switch(wParam)
		{
		case 1:			// 4Hz
			{
				/*int nPMTIndex;

				for (nPMTIndex = 0; nPMTIndex < MAX_PAT_ENTRIES; nPMTIndex++)
				{
					int i;

					if (v->pat.pmt[nPMTIndex].nProgramNumber == 0)
						continue;
					if (v->pat.pmt[nPMTIndex].nPMTPID == 0)
						break;
					if (archive->ap[nPMTIndex]->fEnabled)
					{
						archive->ap[nPMTIndex]->nLastSampleGraphBytes = archive->ap[nPMTIndex]->nGraphBytes * 4;
						if (archive->ap[nPMTIndex]->nLastSampleGraphBytes == 0)
						{
							archive->ap[nPMTIndex]->nNoDataCounter++;
							if (archive->ap[nPMTIndex]->nNoDataCounter > 4)
								archive->ap[nPMTIndex]->nStatus |= STATUS_NO_DATA;
						}
						else
						{
							archive->ap[nPMTIndex]->nNoDataCounter = 0;
							archive->ap[nPMTIndex]->nStatus &= ~STATUS_NO_DATA;
						}
						archive->ap[nPMTIndex]->nGraphBytes = 0;
						archive->ap[nPMTIndex]->dGraphBytesTotal += archive->ap[nPMTIndex]->nLastSampleGraphBytes;
						archive->ap[nPMTIndex]->dGraphBytesTotalCount += 1.0;

						if (archive->ap[nPMTIndex]->nLastSampleGraphBytes > archive->ap[nPMTIndex]->nGraphBytesMax)
							archive->ap[nPMTIndex]->nGraphBytesMax = archive->ap[nPMTIndex]->nLastSampleGraphBytes;
						if (archive->ap[nPMTIndex]->nLastSampleGraphBytes && archive->ap[nPMTIndex]->nLastSampleGraphBytes < archive->ap[nPMTIndex]->nGraphBytesMin)
							archive->ap[nPMTIndex]->nGraphBytesMin = archive->ap[nPMTIndex]->nLastSampleGraphBytes;

						for (i = 0; i < MAX_HISTORIC_SAMPLES - 1; i++)
							archive->ap[nPMTIndex]->nHistoricLastSampleGraphBytes[i] = archive->ap[nPMTIndex]->nHistoricLastSampleGraphBytes[i + 1];
						archive->ap[nPMTIndex]->nHistoricLastSampleGraphBytes[MAX_HISTORIC_SAMPLES - 1] = archive->ap[nPMTIndex]->nLastSampleGraphBytes;
					}
				}*/
				InvalidateRect(hWnd, NULL, FALSE);
			}
			break;
		case 2: // 1 Hz
			{
				int nPMTIndex;

				for (nPMTIndex = 0; nPMTIndex < MAX_PAT_ENTRIES; nPMTIndex++)
				{
					if (v->pat.pmt[nPMTIndex].nProgramNumber == 0)
						continue;
					if (v->pat.pmt[nPMTIndex].nPMTPID == 0)
						break;
					if (archive->ap[nPMTIndex]->fEnabled)
					{
						archive->ap[nPMTIndex]->stCurrentFileRun.wSecond++;
						if (archive->ap[nPMTIndex]->stCurrentFileRun.wSecond > 59)
						{
							archive->ap[nPMTIndex]->stCurrentFileRun.wSecond = 0;
							//archive->ap[nPMTIndex]->nStatus = STATUS_NORMAL;	// gets reset quickly if errors exist
							archive->ap[nPMTIndex]->stCurrentFileRun.wMinute++;
							if (archive->ap[nPMTIndex]->stCurrentFileRun.wMinute > 59)
							{
								archive->ap[nPMTIndex]->stCurrentFileRun.wMinute = 0;
								archive->ap[nPMTIndex]->stCurrentFileRun.wHour++;
								if (archive->ap[nPMTIndex]->stCurrentFileRun.wHour > 23)
								{
									archive->ap[nPMTIndex]->stCurrentFileRun.wHour = 0;
									archive->ap[nPMTIndex]->stCurrentFileRun.wDay++;
								}
							}
						}
					}
				}

				archive->stTotalRun.wSecond++;
				if (archive->stTotalRun.wSecond > 59)
				{
					archive->stTotalRun.wSecond = 0;
					archive->stTotalRun.wMinute++;
					if (archive->stTotalRun.wMinute > 59)
					{
						archive->stTotalRun.wMinute = 0;
						archive->stTotalRun.wHour++;
						if (archive->stTotalRun.wHour > 23)
						{
							archive->stTotalRun.wHour = 0;
							archive->stTotalRun.wDay++;
						}
					}
				}

				if (archive->nThumbnailCounter++ >= 5)
				{
					archive->nCurrentThumbnailDisplay++;
					archive->fNeedNewThumbnail = TRUE;
					archive->nThumbnailCounter = 0;

					// Also might as well update the free space
					UpdateOutputFreeSpace(hWnd);
				}
			}
			break;
		}
		break;
	case WM_PAINT:
		{
			int nPMTIndex;
			int nCurrentY = 0;
			int nIOPending = 0;
			int nDrive;
			int nTotalAverageBitrate = 0;
			int nTotalAverageBitrateSamples = 0;
			int nEstimatedRemainingTimeY, nEstimatedRemainingTimeX;
			int nActiveChannels, nOKChannels;
			int nThumbnailWidth;
			int nMaximumIOs = 0;
			HDC hDC, hRealDC;
			HBITMAP memBM;
			double dMinimumFreeSpace;
			double dTotalDriveFreeSpace = 0;
			PAINTSTRUCT ps;
			RECT rc;
			SIZE sizeText;
			SYSTEMTIME stNow;
			LARGE_INTEGER ftNow;
			char szTemp[128];
			char szFreeSpaceString[256] = {0};
						
			GetSystemTime(&stNow);
			SystemTimeToFileTime(&stNow, (FILETIME*)&ftNow);

			GetClientRect(hWnd, &rc);
			hRealDC = BeginPaint(hWnd, &ps);
			hDC = CreateCompatibleDC(hRealDC);
			memBM = CreateCompatibleBitmap (hRealDC, rc.right, rc.bottom);
			SelectObject(hDC, memBM);
			SetBkMode(hDC, TRANSPARENT);

			for (nPMTIndex = 0; nPMTIndex < MAX_PAT_ENTRIES; nPMTIndex++)
			{
				if (v->pat.pmt[nPMTIndex].nProgramNumber == 0)
					continue;
				if (v->pat.pmt[nPMTIndex].nPMTPID == 0)
					break;
				if (archive->ap[nPMTIndex]->fEnabled == TRUE)
				{
					int nAsync;

					for (nAsync = 0; nAsync < MAX_ASYNC_IO; nAsync++)
					{
						if (archive->ap[nPMTIndex]->nWriteBufferOffset[nAsync])
							nIOPending++;

					}
					nMaximumIOs += MAX_ASYNC_IO;
				}
			}

			// First line shows I/O pending and free space usage
			if (nIOPending > archive->nMaxIOsPending)
				archive->nMaxIOsPending = nIOPending;
			wsprintf(szTemp, "I/Os %d (max %d/%d)", nIOPending, archive->nMaxIOsPending, nMaximumIOs);
			if (archive->nMaxIOsPending > nMaximumIOs / 2)
				SetTextColor(hDC, RGB(0xff, 0xff, 0x00));
			else
				SetTextColor(hDC, RGB(0x00, 0xff, 0x00));
			TextOut(hDC, 5, nCurrentY, szTemp, lstrlen(szTemp));

			wsprintf(szTemp, "%03d:%02d:%02d:%02d", 
					 archive->stTotalRun.wDay,
					 archive->stTotalRun.wHour,
					 archive->stTotalRun.wMinute,
					 archive->stTotalRun.wSecond);
			GetTextExtentPoint(hDC, szTemp, lstrlen(szTemp), &sizeText);
			nEstimatedRemainingTimeY = nCurrentY;
			nEstimatedRemainingTimeX = rc.right - sizeText.cx - 5;
			SetTextColor(hDC, RGB(0x00, 0xff, 0x00));
			TextOut(hDC, nEstimatedRemainingTimeX, nCurrentY, szTemp, lstrlen(szTemp));

			// Free space
			dMinimumFreeSpace = 1024.0 * 1024.0 * 1024.0;
			for (nDrive = 0; nDrive < 26; nDrive++)
			{
				if (archive->bActiveDrive[nDrive] == TRUE)
				{
					sprintf(szTemp, "%c: %.1f GB", nDrive + 'A', archive->dDriveFreeSpace[nDrive]);
					if (archive->bFileRemovalActive[nDrive])
						lstrcat(szTemp, "*");
					lstrcat(szTemp, " ");

					lstrcat(szFreeSpaceString, szTemp);
					dTotalDriveFreeSpace += archive->dDriveFreeSpace[nDrive];
					if (archive->dDriveFreeSpace[nDrive] < dMinimumFreeSpace)
						dMinimumFreeSpace = archive->dDriveFreeSpace[nDrive];					
				}
			}
			
			if (lstrlen(szFreeSpaceString))
			{
				if (dMinimumFreeSpace < (double)archive->nErrorGB)
				{
					SetTextColor(hDC, RGB(0xff, 0x00, 0x00));
					archive->nErrorBeep |= ERROR_BEEP_DISK_SPACE;
				}
				else
				{
					archive->nErrorBeep &= ~ERROR_BEEP_DISK_SPACE;
					if (dMinimumFreeSpace < (double)archive->nWarningGB)
						SetTextColor(hDC, RGB(0xff, 0xff, 0x00));
				}
				TextOut(hDC, 150, nCurrentY, szFreeSpaceString, lstrlen(szFreeSpaceString));
			}
			SetTextColor(hDC, RGB(0x00, 0xff, 0x00));
			nCurrentY += 20;
			nActiveChannels = nOKChannels = 0;

			for (nPMTIndex = 0; nPMTIndex < MAX_PAT_ENTRIES; nPMTIndex++)
			{
				char szDisplayChannelName[64];

				if (v->pat.pmt[nPMTIndex].nPMTPID == 0)
					break;
				if (v->pat.pmt[nPMTIndex].nProgramNumber == 0)
					continue;
				if (archive->ap[nPMTIndex]->fEnabled)
				{
					char szRecorded[64];
					
					nActiveChannels++;
					SelectObject(hDC, archive->hSmallTextFontBold);
					if (archive->ap[nPMTIndex]->nStatus == STATUS_NORMAL)
					{
						SetTextColor(hDC, RGB(0x00, 0xff, 0x00));	// green for OK
						lstrcpy(szTemp, "Normal");
						nOKChannels++;
					}
					else
					{
						SetTextColor(hDC, RGB(0xff, 0x00, 0x00));
						if (archive->ap[nPMTIndex]->nStatus & STATUS_SCRAMBLED)
							lstrcpy(szTemp, "Scrambled");
						else if (archive->ap[nPMTIndex]->nStatus & STATUS_NO_DATA)
							lstrcpy(szTemp, "No Data");
						else if (archive->ap[nPMTIndex]->nStatus & STATUS_NO_ASYNC_IO)
							lstrcpy(szTemp, "No I/Os");
						else
							lstrcpy(szTemp, "Invalid");
						archive->nErrorBeep |= ERROR_BEEP_MPEG;
						archive->nErrorPMTIndex = nPMTIndex;
					}
					TextOut(hDC, 5, nCurrentY, szTemp, lstrlen(szTemp));

					SelectObject(hDC, archive->hSmallTextFont);
					SetTextColor(hDC, RGB(0x00, 0xff, 0xff));
					lstrcpy(szDisplayChannelName, archive->ap[nPMTIndex]->szChannelName);
					do
					{
						SIZE sizeChannelName;

						wsprintf(szTemp, "%d [%s]", v->pat.pmt[nPMTIndex].nProgramNumber, szDisplayChannelName);
						GetTextExtentPoint(hDC, szTemp, lstrlen(szTemp), &sizeChannelName);
						if (sizeChannelName.cx + 80 <= 215)
							break;
						szDisplayChannelName[lstrlen(szDisplayChannelName) - 1] = '\0';
					} while (lstrlen(szDisplayChannelName));
					TextOut(hDC, 80, nCurrentY, szTemp, lstrlen(szTemp));

					GetRecordedString(szRecorded, archive->ap[nPMTIndex]->dCurrentFileWritten);
					wsprintf(szTemp, "Current: %s", szRecorded);
					GetTextExtentPoint(hDC, szTemp, lstrlen(szTemp), &sizeText);
					TextOut(hDC, rc.right - sizeText.cx - 5, nCurrentY, szTemp, lstrlen(szTemp));
			
					// Current file runtime
					wsprintf(szTemp, "%03d:%02d:%02d:%02d", 
						     archive->ap[nPMTIndex]->stCurrentFileRun.wDay,
							 archive->ap[nPMTIndex]->stCurrentFileRun.wHour,
							 archive->ap[nPMTIndex]->stCurrentFileRun.wMinute,
							 archive->ap[nPMTIndex]->stCurrentFileRun.wSecond);
					GetTextExtentPoint(hDC, szTemp, lstrlen(szTemp), &sizeText);
					TextOut(hDC, rc.right - sizeText.cx - 5, nCurrentY + 13, szTemp, lstrlen(szTemp));

					// Remaining time for this program
					{
						LARGE_INTEGER ftEventStart;
						DWORD64 dwDuration;
						LARGE_INTEGER ftTimeRemaining;
						SYSTEMTIME stRemaining;

						SystemTimeToFileTime(&archive->ap[nPMTIndex]->stCurrentStart, (FILETIME*)&ftEventStart);
						dwDuration = (DWORD64)10000000 * (DWORD64)archive->ap[nPMTIndex]->stCurrentDuration.wHour * (DWORD64)60 * (DWORD64)60
			                       + (DWORD64)10000000 * (DWORD64)archive->ap[nPMTIndex]->stCurrentDuration.wMinute * (DWORD64)60
								   + (DWORD64)10000000 * (DWORD64)archive->ap[nPMTIndex]->stCurrentDuration.wSecond;
						ftTimeRemaining.QuadPart = (ftEventStart.QuadPart + dwDuration) - ftNow.QuadPart;
						if (ftTimeRemaining.QuadPart < 0)
							ftTimeRemaining.QuadPart = 0;
						FileTimeToSystemTime((FILETIME*)&ftTimeRemaining, &stRemaining);
						wsprintf(szTemp, "%02d:%02d:%02d", 
								 stRemaining.wHour,
								 stRemaining.wMinute,
								 stRemaining.wSecond);
						GetTextExtentPoint(hDC, szTemp, lstrlen(szTemp), &sizeText);
						TextOut(hDC, rc.right - sizeText.cx - 5, nCurrentY + 25, szTemp, lstrlen(szTemp));
					}


					GetRecordedString(szRecorded, archive->ap[nPMTIndex]->dAllBytesWritten);
					wsprintf(szTemp, "All: %s", szRecorded);
					TextOut(hDC, 220, nCurrentY, szTemp, lstrlen(szTemp));
					
					// Output EPG name
					switch(archive->ap[nPMTIndex]->nEPGType)
					{
					case EPG_TYPE_30_MIN:
						lstrcpy(szTemp, "30 minute recordings");
						break;
					default:
						lstrcpy(szTemp, archive->ap[nPMTIndex]->szCurrentProgram);
						break;
					}
					while (TRUE)
					{
						GetTextExtentPoint(hDC, szTemp, lstrlen(szTemp), &sizeText);
						if (sizeText.cx < 320 - 5)
							break;
						szTemp[lstrlen(szTemp) - 1] = '\0';
					}
					TextOut(hDC, 5, nCurrentY + 13, szTemp, lstrlen(szTemp));
					
					// Output filename
					TextOut(hDC, 5, nCurrentY + 25, archive->ap[nPMTIndex]->szOutputFileName, lstrlen(archive->ap[nPMTIndex]->szOutputFileName));

					// Draw graph to indicate bytes coming through on the channel
					if (archive->ap[nPMTIndex]->nGraphBytesMax)
					{
						int nBytesRange;
						int i;
						int nGraphSamples = 0;
						int nGraphStart;
						double dFactor;
						RECT rectGraph;
						RECT rectGraphFrame;
						POINT pt[MAX_HISTORIC_SAMPLES];
						
						// Frame for the graph
						rectGraphFrame.top = nCurrentY + 40;
						rectGraphFrame.bottom = nCurrentY + 40 + 15 + 10;
						rectGraphFrame.left = 2;
						rectGraphFrame.right = rc.right - 5;
						FrameRect(hDC, &rectGraphFrame, archive->hGreenBrush);

						rectGraph.top = nCurrentY + 40 + 3;
						rectGraph.bottom = nCurrentY + 40 + 15 - 10;
						rectGraph.left = 55;
						rectGraph.right = rc.right - 5 - 55;
						nBytesRange = archive->ap[nPMTIndex]->nGraphBytesMax;
						dFactor = (double)nBytesRange / (double)(rectGraph.right - rectGraph.left);
						rectGraph.right = rectGraph.left + (int)((double)archive->ap[nPMTIndex]->nLastSampleGraphBytes / dFactor);
						if (rectGraph.right < rectGraph.left)
							rectGraph.right = rectGraph.left;
						if (archive->ap[nPMTIndex]->nStatus == STATUS_NORMAL)
							FillRect(hDC, &rectGraph, archive->hGreenBrush);
						else
							FillRect(hDC, &rectGraph, archive->hRedBrush);

						wsprintf(szTemp, "%d Kb", (archive->ap[nPMTIndex]->nGraphBytesMin * 8) / 1000);
						TextOut(hDC, 5, nCurrentY + 40, szTemp, lstrlen(szTemp));
						
						wsprintf(szTemp, "%d Kb", (archive->ap[nPMTIndex]->nGraphBytesMax * 8) / 1000);
						TextOut(hDC, 5, nCurrentY + 51, szTemp, lstrlen(szTemp));
						
						wsprintf(szTemp, "%d Kb", (archive->ap[nPMTIndex]->nLastSampleGraphBytes * 8) / 1000);
						TextOut(hDC, rc.right - 55, nCurrentY + 40, szTemp, lstrlen(szTemp));

						if (archive->ap[nPMTIndex]->dGraphBytesTotalCount > 0.0)
						{
							int nChannelKBAverage = (int)((archive->ap[nPMTIndex]->dGraphBytesTotal * 8) / archive->ap[nPMTIndex]->dGraphBytesTotalCount);
							wsprintf(szTemp, "%d Kb", nChannelKBAverage / 1000);
							TextOut(hDC, rc.right - 55, nCurrentY + 51, szTemp, lstrlen(szTemp));
							nTotalAverageBitrate += nChannelKBAverage / 1000;
							nTotalAverageBitrateSamples++;
						}

						nGraphStart = MAX_HISTORIC_SAMPLES - (rc.right - rectGraph.left - 55 - 5);
						for (i = nGraphStart; i < MAX_HISTORIC_SAMPLES; i++)
						{
							int nOffset;
							dFactor = (double)nBytesRange / (double)(15);
							nOffset = (int)((double)archive->ap[nPMTIndex]->nHistoricLastSampleGraphBytes[i] / dFactor);

							pt[nGraphSamples].x = nGraphSamples + rectGraph.left;
							pt[nGraphSamples].y = (rectGraph.bottom + 15) - nOffset;
							nGraphSamples++;
						}
						SelectObject(hDC, archive->hYellowPen);
						Polyline(hDC, pt, nGraphSamples);
					}

					// Onto the next archiving channel
					nCurrentY += 68;
					if (nCurrentY > rc.bottom)
						break;
				}
			}

			// Turn off beep if all OK
			if (nActiveChannels == nOKChannels)
				archive->nErrorBeep &= ~ERROR_BEEP_MPEG;

			// Now we know the average bitrate for all active chnnels, we can estimate running time left
			if (nTotalAverageBitrate && dTotalDriveFreeSpace)
			{
				int nRemainingSeconds, nDays, nHours, nMinutes;
				// dAverageBitrate is the average data in Kbps for all active channels
				double dAverageBitrate = (double)nTotalAverageBitrate / (double)nTotalAverageBitrateSamples;
				
				dAverageBitrate /= 8.0; // convert to KBps
				dAverageBitrate /= 1024.0; // convert to MBps
				dTotalDriveFreeSpace *= 1024.0; // convert total space from GB to MB
				nRemainingSeconds = (int)(dTotalDriveFreeSpace / (dAverageBitrate * (double)nTotalAverageBitrateSamples));
				nDays = nRemainingSeconds / 86400;
				nRemainingSeconds -= nDays * 86400;
				nHours = nRemainingSeconds / 3600;
				nRemainingSeconds -= nHours * 3600;
				nMinutes = nRemainingSeconds / 60;
				nRemainingSeconds -= nMinutes * 60;
				if (nDays)
					wsprintf(szTemp, "%dd%02dh%02dm", nDays, nHours, nMinutes);
				else
					wsprintf(szTemp, "%dh%02dm", nHours, nMinutes);
				GetTextExtentPoint(hDC, szTemp, lstrlen(szTemp), &sizeText);
				SetTextColor(hDC, RGB(0x00, 0xff, 0x00));
				TextOut(hDC, nEstimatedRemainingTimeX - sizeText.cx - 5, nEstimatedRemainingTimeY, szTemp, lstrlen(szTemp));
			}
			
			// Output the thumbnail at the bottom			
			EnterCriticalSection(&v->csThumbnails);
			if (archive->fNeedNewThumbnail)
			{
				BOOL fFoundNextThumbnail = FALSE;

				if (ArchiveThumbnailCount() == 0)
				{
					nThumbnailWidth = 0;
					goto ArchivePaint_DoneThumbnails;
				}

				archive->fNeedNewThumbnail = FALSE;
				for (nPMTIndex = archive->nCurrentThumbnailDisplay; nPMTIndex < MAX_PAT_ENTRIES; nPMTIndex++)
				{
					int nESIndex;
					
					// If we got to the end, start at the begining again
					if (v->pat.pmt[nPMTIndex].nPMTPID == 0 && v->pat.pmt[nPMTIndex].nProgramNumber == 0)
					{
						nPMTIndex = -1;
						archive->nCurrentThumbnailDisplay = 0;
						continue;
					}
					if (archive->ap[nPMTIndex] == NULL)
						continue;
					if (archive->ap[nPMTIndex]->fEnabled == TRUE)
					{
						for (nESIndex = 0; nESIndex < MAX_ESLIST_ENTRIES; nESIndex++)
						{
							if (v->pat.pmt[nPMTIndex].es[nESIndex].nESPID == 0)
								break;
							if (   v->pat.pmt[nPMTIndex].es[nESIndex].nStreamType == 0x01 // MPEG-1
								|| v->pat.pmt[nPMTIndex].es[nESIndex].nStreamType == 0x02 // MPEG-2
								|| v->pat.pmt[nPMTIndex].es[nESIndex].nStreamType == 0x10 // MPEG-4
								|| v->pat.pmt[nPMTIndex].es[nESIndex].nStreamType == 0x1b // H.264
								|| v->pat.pmt[nPMTIndex].es[nESIndex].nStreamType == 0x80 // DCII
								|| v->pat.pmt[nPMTIndex].es[nESIndex].nStreamType == 0xea // VC1
								)
							{
								if (v->pat.pmt[nPMTIndex].es[nESIndex].pRGBVideoFrame != NULL)
								{
									archive->nCurrentThumbnailDisplay = nPMTIndex;
									archive->nThumbnailWidth = v->pat.pmt[nPMTIndex].es[nESIndex].nVideoWidth;
									archive->nThumbnailHeight = v->pat.pmt[nPMTIndex].es[nESIndex].nVideoHeight;
									archive->pThumbnailRGB = v->pat.pmt[nPMTIndex].es[nESIndex].pRGBVideoFrame;
									fFoundNextThumbnail = TRUE;
									break;
								}
							}
						}
					}
					if (fFoundNextThumbnail)
						break;
				}
			}
			{
				char szChannelName[128] = {0};

				if (archive->ap[archive->nCurrentThumbnailDisplay] != NULL)
					wsprintf(szChannelName, "%d [%s]", 
							 v->pat.pmt[archive->nCurrentThumbnailDisplay].nProgramNumber,
							 archive->ap[archive->nCurrentThumbnailDisplay]->szChannelName);

				_ISDrawRGB(hDC,
						   archive->pThumbnailRGB,
						   archive->nThumbnailWidth, archive->nThumbnailHeight,
						   5, nCurrentY + 5,
						   archive->nThumbnailWidth, archive->nThumbnailHeight,
						   NULL);
				SetBkMode(hDC, TRANSPARENT);
				SetTextColor(hDC, RGB(0x00, 0xff, 0x00));
				TextOut(hDC, 5, nCurrentY + 5, szChannelName, lstrlen(szChannelName));

				nThumbnailWidth = archive->nThumbnailWidth;
ArchivePaint_DoneThumbnails:
				LeaveCriticalSection(&v->csThumbnails);
			}

			// Now draw the event list
			{
				int i;

				EnterCriticalSection(&archive->csEventList);
				SetTextColor(hDC, RGB(0x00, 0xff, 0x00));
				SelectObject(hDC, archive->hSmallTextFontBold);
				for (i = 0; i < MAX_EVENTS && nCurrentY < rc.bottom; i++)
				{			
					char szTemp2[32];

					if (archive->eventlist[i].nEventType == 0)
						break;
					
					DrawIconEx(hDC, 
						       nThumbnailWidth + 10, nCurrentY + 5,
							   archive->hStatusIcon[archive->eventlist[i].nEventType - 1],
							   16, 16,
							   0,
							   NULL,
							   DI_NORMAL);
					if (!archive->fLocalTime)
					{
						wsprintf(szTemp2, "%02d:%02d:%02d",
								 archive->eventlist[i].stEvent.wHour, archive->eventlist[i].stEvent.wMinute, archive->eventlist[i].stEvent.wSecond);
					}
					else
					{
						SYSTEMTIME stLocal;
						
						SystemTimeToTzSpecificLocalTime(NULL, &archive->eventlist[i].stEvent, &stLocal);
						wsprintf(szTemp2, "%02d:%02d:%02d",
								 stLocal.wHour, stLocal.wMinute, stLocal.wSecond);
					}
					wsprintf(szTemp, "%s %s", 
						     szTemp2,
							 archive->eventlist[i].szShort);
					TextOut(hDC, nThumbnailWidth + 10 + 16 + 2, nCurrentY + 5, szTemp, lstrlen(szTemp));
					nCurrentY += 16;
				}
				LeaveCriticalSection(&archive->csEventList);
			}

			BitBlt(hRealDC, 0, 25, rc.right, rc.bottom, hDC, 0, 0, SRCCOPY);
			DeleteObject(memBM);
			DeleteDC(hDC);
			EndPaint(hWnd, &ps);
		}
		break;
	case WM_SIZE:
		InvalidateRect(hWnd, NULL, FALSE);
		break;
	}

	return FALSE;
}

void WriteCaptionFileTime(int nPMTIndex, int cc_type)
{
	DWORD dwWritten;
	SYSTEMTIME st;
	char szTemp[256];

	GetSystemTime(&st);
	wsprintf(szTemp, "[%04d/%02d/%02d %02d:%02d:%02d] ",
		     st.wYear, st.wMonth, st.wDay,
			 st.wHour, st.wMinute, st.wSecond);

	WriteFile(archive->ap[nPMTIndex]->hCaptionOutputFile[archive->ap[nPMTIndex]->nCaptionCurrentOutputFileIndex[cc_type]], szTemp, lstrlen(szTemp), &dwWritten, NULL);
}

void WriteCCCharacter(int nPMTIndex, int cc_type, BYTE cc_byte)
{
	DWORD dwWritten;
	BYTE new_cc_byte = cc_byte;

	switch(new_cc_byte)
	{
	case '*':
		new_cc_byte = 'á';
		break;
	case '\\':
		new_cc_byte = 'é';
		break;
	case '^':
		new_cc_byte = 'í';
		break;
	case '_':
		new_cc_byte = 'ó';
		break;
	case '`':
		new_cc_byte = 'ú';
		break;
	case '{':
		new_cc_byte = 'ç';
		break;
	case '|':
		new_cc_byte = '÷';
		break;
	case '}':
		new_cc_byte = 'Ñ';
		break;
	case '~':
		new_cc_byte = 'ñ';
		break;
	case 0x7f:
		return;		// don't output that
	}

	WriteFile(archive->ap[nPMTIndex]->hCaptionOutputFile[archive->ap[nPMTIndex]->nCaptionCurrentOutputFileIndex[cc_type]], &new_cc_byte, 1, &dwWritten, NULL);
}

void WriteCCData(int nPMTIndex, int cc_valid, int cc_type, BYTE cc_data_1, BYTE cc_data_2)
{
	DWORD dwWritten;
	BYTE bCCBuffer[4];
	
	bCCBuffer[0] = cc_valid;
	bCCBuffer[1] = cc_type;
	bCCBuffer[2] = cc_data_1;
	bCCBuffer[3] = cc_data_2;
	
	WriteFile(archive->ap[nPMTIndex]->hUserDataOutputFile, bCCBuffer, 4, &dwWritten, NULL);
	archive->ap[nPMTIndex]->nUserDataBytes += dwWritten;

	if (cc_valid)
	{
		int type;

		// Determine what type of data we have
		type = GetCCDataType(cc_data_1 << 8 | cc_data_2);
		
		switch (type)
		{
		case TYPE_CHAR_STANDARD:
			if (cc_type == 1 && archive->ap[nPMTIndex]->fXDSActive)
				break;
			if (archive->ap[nPMTIndex]->nCaptionCurrentOutputFileIndex[cc_type] != -1)
			{
				if (cc_data_1 != 0)
				{
					if (archive->ap[nPMTIndex]->nCaptionOutputPosition[archive->ap[nPMTIndex]->nCaptionCurrentOutputFileIndex[cc_type]] == 0)
						WriteCaptionFileTime(nPMTIndex, cc_type);
					WriteCCCharacter(nPMTIndex, cc_type, cc_data_1);
					archive->ap[nPMTIndex]->nCaptionOutputPosition[archive->ap[nPMTIndex]->nCaptionCurrentOutputFileIndex[cc_type]]++;
					archive->ap[nPMTIndex]->nCaptionProgramBytes[archive->ap[nPMTIndex]->nCaptionCurrentOutputFileIndex[cc_type]]++;
				}
				if (cc_data_2 != 0)
				{
					if (archive->ap[nPMTIndex]->nCaptionOutputPosition[archive->ap[nPMTIndex]->nCaptionCurrentOutputFileIndex[cc_type]] == 0)
						WriteCaptionFileTime(nPMTIndex, cc_type);
					WriteCCCharacter(nPMTIndex, cc_type, cc_data_2);
					archive->ap[nPMTIndex]->nCaptionOutputPosition[archive->ap[nPMTIndex]->nCaptionCurrentOutputFileIndex[cc_type]]++;
					archive->ap[nPMTIndex]->nCaptionProgramBytes[archive->ap[nPMTIndex]->nCaptionCurrentOutputFileIndex[cc_type]]++;
				}
			}
			break;
		case TYPE_MISC1:
			if (cc_type == 1 && archive->ap[nPMTIndex]->fXDSActive)
				break;
			{
				int channel = ((cc_data_1 & 0x08) >> 3);

				// Save any Messages
				switch(cc_data_2 & 0xF)
				{
				case 0x0: // (RCL) Resume Caption Loading
				case 0x5: // (RU2) Roll-up captions, 2 rows
				case 0x6: // (RU3) Roll-up captions, 3 rows
				case 0x7: // (RU4) Roll-up captions, 4 rows
				case 0x9: // (RDC) Resume direct captioning
					archive->ap[nPMTIndex]->nCaptionCurrentOutputFileIndex[cc_type] = channel + (cc_type * 2);
					break;
				case 0x1: // (BS)  Backspace
				case 0x2: // (AOF) Reserved (formerly alarm off)
				case 0x3: // (AON) Reserved (formerly alarm on)
				case 0x4: // (DER) Delete to end of row
				case 0x8: // (FON) Flash on
				case 0xE: // (ENM) Erase nondisplayed memory
				case 0xF: // (EOC) End of caption (flip memories)
					break;
				case 0xA: // (TR)  Text restart
				case 0xB: // (RTD) Resume text display
					archive->ap[nPMTIndex]->nCaptionCurrentOutputFileIndex[cc_type] = channel + 4  + (cc_type * 2);
					break;
				case 0xC: // (EDM) Erase displayed memory
					break;
				case 0xD: // (CR)  Carriage return
					if (archive->ap[nPMTIndex]->nCaptionCurrentOutputFileIndex[cc_type] != -1)
					{
						if (archive->ap[nPMTIndex]->nCaptionOutputPosition[archive->ap[nPMTIndex]->nCaptionCurrentOutputFileIndex[cc_type]] > 0)
						{
							WriteFile(archive->ap[nPMTIndex]->hCaptionOutputFile[archive->ap[nPMTIndex]->nCaptionCurrentOutputFileIndex[cc_type]], "\r\n", 2, &dwWritten, NULL);
							archive->ap[nPMTIndex]->nCaptionProgramBytes[archive->ap[nPMTIndex]->nCaptionCurrentOutputFileIndex[cc_type]] += 2;
							archive->ap[nPMTIndex]->nCaptionOutputPosition[archive->ap[nPMTIndex]->nCaptionCurrentOutputFileIndex[cc_type]] = 0;
						}
					}
					break;
				}
			}
			break;
		case TYPE_MISC2:
			break;
		case TYPE_MIDROW:
			break;
		case TYPE_PREAMBLE_ADDRESS:
			if (cc_type == 1 && archive->ap[nPMTIndex]->fXDSActive)
				break;
			if (archive->ap[nPMTIndex]->nCaptionCurrentOutputFileIndex[cc_type] != -1)
			{
				if (archive->ap[nPMTIndex]->nCaptionOutputPosition[archive->ap[nPMTIndex]->nCaptionCurrentOutputFileIndex[cc_type]] > 0)
				{
					WriteFile(archive->ap[nPMTIndex]->hCaptionOutputFile[archive->ap[nPMTIndex]->nCaptionCurrentOutputFileIndex[cc_type]], "\r\n", 2, &dwWritten, NULL);
					archive->ap[nPMTIndex]->nCaptionOutputPosition[archive->ap[nPMTIndex]->nCaptionCurrentOutputFileIndex[cc_type]] = 0;
					archive->ap[nPMTIndex]->nCaptionProgramBytes[archive->ap[nPMTIndex]->nCaptionCurrentOutputFileIndex[cc_type]] += 2;
				}
			}
			break;
		case TYPE_ATTRIBUTE_1:
		case TYPE_ATTRIBUTE_2:
		case TYPE_ATTRIBUTE_3:
		case TYPE_CLOSED_GROUP_EXT:
		case TYPE_CHAR_EXT1:
		case TYPE_CHAR_EXT2:
		case TYPE_CHAR_EXT3:
			break;
		case TYPE_XDS:
			if (cc_data_1 == 0x0f)
			{
				// XDS end code - we're not doing XDS any more
				archive->ap[nPMTIndex]->fXDSActive = FALSE;
			}
			else
			{
				// XDS start or continuation code - we now block text
				// that's XDS data from being written as cc3/cc4 data
				archive->ap[nPMTIndex]->fXDSActive = TRUE;
			}		
			break; 
		default:
			break;
		}
	}
}

void OutputPendingUserData(int nPMTIndex)
{
	while (archive->ap[nPMTIndex]->gopbuffer[archive->ap[nPMTIndex]->nGOPOutputPos].fActive == TRUE)
	{
		DWORD dwWritten;
		BOOL fAlreadyWritten = FALSE;

		if (archive->ap[nPMTIndex]->gopbuffer[archive->ap[nPMTIndex]->nGOPOutputPos].nUserDataLength > 5)
		{
			set_buf(BM_ARCHIVE_THREAD, archive->ap[nPMTIndex]->gopbuffer[archive->ap[nPMTIndex]->nGOPOutputPos].bUserData, 0, FALSE);
			{
				DWORD ATSC_identifier = get_bits(BM_ARCHIVE_THREAD, 32);
				BYTE user_data_type_code = get_bits(BM_ARCHIVE_THREAD, 8);
				if (user_data_type_code == 0x03 && ATSC_identifier == 0x47413934)
				{
					// ATSC format
					BOOL process_em_data_flag = get_bits(BM_ARCHIVE_THREAD, 1);
					BOOL process_cc_data_flag = get_bits(BM_ARCHIVE_THREAD, 1);
					BOOL additional_data_flag = get_bits(BM_ARCHIVE_THREAD, 1);
					int cc_count = get_bits(BM_ARCHIVE_THREAD, 5);
					int em_data = get_bits(BM_ARCHIVE_THREAD, 8);
					int i;

					for (i = 0; i < cc_count; i++ )
					{
						int marker_bits = get_bits(BM_ARCHIVE_THREAD, 5);
						BOOL cc_valid = get_bits(BM_ARCHIVE_THREAD, 1);
						int cc_type = get_bits(BM_ARCHIVE_THREAD, 2);
						BYTE cc_data_1 = get_bits(BM_ARCHIVE_THREAD, 8) & 0x7f;
						BYTE cc_data_2 = get_bits(BM_ARCHIVE_THREAD, 8) & 0x7f;

						if (cc_type == 0 || cc_type == 1)
							WriteCCData(nPMTIndex, cc_valid, cc_type, cc_data_1, cc_data_2);
					}
					fAlreadyWritten = TRUE;
				}
				else
				{
					set_buf(BM_ARCHIVE_THREAD, archive->ap[nPMTIndex]->gopbuffer[archive->ap[nPMTIndex]->nGOPOutputPos].bUserData, 0, FALSE);
					if ((get_bits(BM_ARCHIVE_THREAD, 16) & 0xff7f) == 0x0301)
					{
						// SCTE 20 format
						{
							int cc_count = get_bits(BM_ARCHIVE_THREAD, 5);
							int i;

							for (i = 0; i < cc_count; i++)
							{
								int cc_priority = get_bits(BM_ARCHIVE_THREAD, 2);
								int field_number = get_bits(BM_ARCHIVE_THREAD, 2);
								int line_offset = get_bits(BM_ARCHIVE_THREAD, 5);
								int cc1 = get_bits(BM_ARCHIVE_THREAD, 8);
								int cc2 = get_bits(BM_ARCHIVE_THREAD, 8);
								int marker_bit = get_bits(BM_ARCHIVE_THREAD, 1);

								if (line_offset == 11)
								{
									WriteCCData(nPMTIndex, 1, (~field_number) & 1, ReverseBits(cc1) & 0x7f, ReverseBits(cc2) & 0x7f);
								}
							}
							fAlreadyWritten = TRUE;
						}
					}
					else
					{
						set_buf(BM_ARCHIVE_THREAD, archive->ap[nPMTIndex]->gopbuffer[archive->ap[nPMTIndex]->nGOPOutputPos].bUserData, 0, FALSE);
						if (get_bits(BM_ARCHIVE_THREAD, 32) == 0x53415544)
						{
							// Sci. Atl. format
							int reserved_04 = get_bits(BM_ARCHIVE_THREAD, 8);
							int reserved_e2 = get_bits(BM_ARCHIVE_THREAD, 8);
							int reserved_b1 = get_bits(BM_ARCHIVE_THREAD, 8);
							int cc1 = get_bits(BM_ARCHIVE_THREAD, 8);
							int cc2 = get_bits(BM_ARCHIVE_THREAD, 8);

							WriteCCData(nPMTIndex, 1, 0, cc1 & 0x7f, cc2 & 0x7f);
						}
						else
						{
							set_buf(BM_ARCHIVE_THREAD, archive->ap[nPMTIndex]->gopbuffer[archive->ap[nPMTIndex]->nGOPOutputPos].bUserData, 0, FALSE);
							if (get_bits(BM_ARCHIVE_THREAD, 16) == 0x0502)
							{
								// Dish Network format
								int junk = get_bits(BM_ARCHIVE_THREAD, 5 * 8);		// 5 bytes of junk
								int type = get_bits(BM_ARCHIVE_THREAD, 8);

								switch(type)
								{
								case 0x02:		// 2 byte caption - can be repeated
									{
										BYTE junk = get_bits(BM_ARCHIVE_THREAD, 8);
										BYTE cc_data_1 = get_bits(BM_ARCHIVE_THREAD, 8) & 0x7f;
										BYTE cc_data_2 = get_bits(BM_ARCHIVE_THREAD, 8) & 0x7f;
										BYTE repeater = get_bits(BM_ARCHIVE_THREAD, 8);
										WriteCCData(nPMTIndex, 1, 0, cc_data_1, cc_data_2);
										if (repeater == 0x04 && cc_data_1 < 0x20)
											WriteCCData(nPMTIndex, 1, 0, cc_data_1, cc_data_2);
									}
									break;
								case 0x04:		// 4 byte caption - not repeated
									{
										BYTE junk = get_bits(BM_ARCHIVE_THREAD, 8);
										BYTE cc_data_1 = get_bits(BM_ARCHIVE_THREAD, 8) & 0x7f;
										BYTE cc_data_2 = get_bits(BM_ARCHIVE_THREAD, 8) & 0x7f;
										WriteCCData(nPMTIndex, 1, 0, cc_data_1, cc_data_2);
										cc_data_1 = get_bits(BM_ARCHIVE_THREAD, 8) & 0x7f;
										cc_data_2 = get_bits(BM_ARCHIVE_THREAD, 8) & 0x7f;
										WriteCCData(nPMTIndex, 1, 0, cc_data_1, cc_data_2);
									}
									break;
								case 0x05:		// type 5 is used by P (Predictive) frames but we've already put
												// the pictures into display order, so no need to hold data
									{
										int i;
										for (i = 0; i < 6; i++)
										{
											BYTE junk = get_bits(BM_ARCHIVE_THREAD, 8);
										}
										{
											BYTE type = get_bits(BM_ARCHIVE_THREAD, 8);
											BYTE junk2 = get_bits(BM_ARCHIVE_THREAD, 8);
											BYTE cc_data_1 = get_bits(BM_ARCHIVE_THREAD, 8) & 0x7f;
											BYTE cc_data_2 = get_bits(BM_ARCHIVE_THREAD, 8) & 0x7f;
											WriteCCData(nPMTIndex, 1, 0, cc_data_1, cc_data_2);

											switch(type)
											{
											case 0x02:
												break;
											case 0x04:
												cc_data_1 = get_bits(BM_ARCHIVE_THREAD, 8) & 0x7f;
												cc_data_2 = get_bits(BM_ARCHIVE_THREAD, 8) & 0x7f;
												WriteCCData(nPMTIndex, 1, 0, cc_data_1, cc_data_2);
												break;
											}
										}
									}
									break;
								}
								fAlreadyWritten = TRUE;
							}
						}
					}
				}
			}
		}

		if (!fAlreadyWritten)
		{
			WriteFile(archive->ap[nPMTIndex]->hUserDataOutputFile,
					  archive->ap[nPMTIndex]->gopbuffer[archive->ap[nPMTIndex]->nGOPOutputPos].bUserData,
					  archive->ap[nPMTIndex]->gopbuffer[archive->ap[nPMTIndex]->nGOPOutputPos].nUserDataLength,
					  &dwWritten, NULL);
			archive->ap[nPMTIndex]->nUserDataBytes += dwWritten;
		}
		archive->ap[nPMTIndex]->gopbuffer[archive->ap[nPMTIndex]->nGOPOutputPos].fActive = FALSE;
		archive->ap[nPMTIndex]->nGOPOutputPos++;
	}
}

void BufferAndWriteUserData(int nPMTIndex, BYTE * pUserData, int nUserDataLength)
{
	if (archive->ap[nPMTIndex]->nPictureCodingType == 1)
	{
		// I-picture so reset the buffer
		if (archive->ap[nPMTIndex]->fUserDataArmed)
			OutputPendingUserData(nPMTIndex);
		memset(&archive->ap[nPMTIndex]->gopbuffer, 0, sizeof(ARCHIVEGOPBUFFER) * MAX_GOP_SIZE);
		archive->ap[nPMTIndex]->nGOPOutputPos = 0;
		archive->ap[nPMTIndex]->fUserDataArmed = TRUE;
	}
	if (archive->ap[nPMTIndex]->fUserDataArmed)
	{
		archive->ap[nPMTIndex]->gopbuffer[archive->ap[nPMTIndex]->nTemporalRererence].fActive = TRUE;
		memcpy(&archive->ap[nPMTIndex]->gopbuffer[archive->ap[nPMTIndex]->nTemporalRererence].bUserData, pUserData, nUserDataLength);
		archive->ap[nPMTIndex]->gopbuffer[archive->ap[nPMTIndex]->nTemporalRererence].nUserDataLength = nUserDataLength;
		OutputPendingUserData(nPMTIndex);
	}
}


void ExtractSubtitleData(int nPMTIndex, BYTE * pPESPacket, int nPESLength)
{
	set_buf(BM_ARCHIVE_THREAD, pPESPacket, 0, FALSE);
	{
		int data_identifier = get_bits(BM_ARCHIVE_THREAD, 8);
		int subtitle_stream_id = get_bits(BM_ARCHIVE_THREAD, 8);
		
		if (data_identifier != 0x20 || subtitle_stream_id != 0)
			return;
		nPESLength -= 2;
		do
		{
			int sync_byte = get_bits(BM_ARCHIVE_THREAD, 8);
			int segment_type;
			int page_id;
			int segment_length;

			if (sync_byte != 0x0f)
				break;
			segment_type = get_bits(BM_ARCHIVE_THREAD, 8);
			page_id = get_bits(BM_ARCHIVE_THREAD, 16);
			segment_length = get_bits(BM_ARCHIVE_THREAD, 16);
			nPESLength -= 6;

			switch(segment_type)
			{
			case 0x10:		// Page Composition Segment
				{
					int page_time_out = get_bits(BM_ARCHIVE_THREAD, 8);
					int page_version_number = get_bits(BM_ARCHIVE_THREAD, 4);
					int page_state = get_bits(BM_ARCHIVE_THREAD, 2);
					int reserved = get_bits(BM_ARCHIVE_THREAD, 2);
					int processed_length = 0;					
					nPESLength -= 2;
					segment_length -= 2;
					while (processed_length < segment_length)
					{	
						int region_id = get_bits(BM_ARCHIVE_THREAD, 8);
						int reserved = get_bits(BM_ARCHIVE_THREAD, 8);
						int region_horizontal_address = get_bits(BM_ARCHIVE_THREAD, 16);
						int region_vertical_address = get_bits(BM_ARCHIVE_THREAD, 16);
						nPESLength -= 6;
						processed_length += 6;
					}
				}
				break;
			case 0x11:		// Region Composition Segment
				{
					int region_id = get_bits(BM_ARCHIVE_THREAD, 8);
					int region_version_number = get_bits(BM_ARCHIVE_THREAD, 4);
					int region_fill_flag = get_bits(BM_ARCHIVE_THREAD, 1);
					int reserved1 = get_bits(BM_ARCHIVE_THREAD, 3);
					int region_width = get_bits(BM_ARCHIVE_THREAD, 16);
					int region_height = get_bits(BM_ARCHIVE_THREAD, 16);
					int region_level_of_compatibility = get_bits(BM_ARCHIVE_THREAD, 3);
					int region_depth = get_bits(BM_ARCHIVE_THREAD, 3);
					int reserved2 = get_bits(BM_ARCHIVE_THREAD, 2);
					int CLUT_id = get_bits(BM_ARCHIVE_THREAD, 8);
					int region_8_bit_pixel_code = get_bits(BM_ARCHIVE_THREAD, 8);
					int region_4_bit_pixel_code = get_bits(BM_ARCHIVE_THREAD, 4);
					int region_2_bit_pixel_code = get_bits(BM_ARCHIVE_THREAD, 2);
					int reserved3 = get_bits(BM_ARCHIVE_THREAD, 2);
					int processed_length = 0;
					nPESLength -= 10;
					segment_length -= 10;
					while (processed_length < segment_length)
					{	
						int object_id = get_bits(BM_ARCHIVE_THREAD, 16);
						int object_type = get_bits(BM_ARCHIVE_THREAD, 2);
						int object_provider_flag = get_bits(BM_ARCHIVE_THREAD, 2);
						int object_horizontal_position = get_bits(BM_ARCHIVE_THREAD, 12);
						int reserved4 = get_bits(BM_ARCHIVE_THREAD, 4);
						int object_vertical_position = get_bits(BM_ARCHIVE_THREAD, 12);
						nPESLength -= 6;
						processed_length += 6;
						if (object_type == 0x01 || object_type == 0x02)
						{	
							int foreground_pixel_code = get_bits(BM_ARCHIVE_THREAD, 8);
							int background_pixel_code = get_bits(BM_ARCHIVE_THREAD, 8);
							nPESLength -= 2;
							processed_length += 2;
						}	
					}	
				}
				break;
			case 0x12:		// CLUT Definition Segment
				{
					int CLUT_id = get_bits(BM_ARCHIVE_THREAD, 8);
					int CLUT_version_number = get_bits(BM_ARCHIVE_THREAD, 4);
					int reserved1 = get_bits(BM_ARCHIVE_THREAD, 4);
					int processed_length = 0;
					nPESLength -= 2;
					segment_length -= 2;
					while (processed_length < segment_length)
					{	
						int CLUT_entry_id = get_bits(BM_ARCHIVE_THREAD, 8);
						int CLUT2_bit_entry_CLUT_flag = get_bits(BM_ARCHIVE_THREAD, 1);
						int CLUT4_bit_entry_CLUT_flag = get_bits(BM_ARCHIVE_THREAD, 1);
						int CLUT8_bit_entry_CLUT_flag = get_bits(BM_ARCHIVE_THREAD, 1);
						int reserved2 = get_bits(BM_ARCHIVE_THREAD, 4);
						int full_range_flag = get_bits(BM_ARCHIVE_THREAD, 1);
						nPESLength -= 2;
						processed_length += 2;
						if (full_range_flag == 1)
						{	
							int Y_value = get_bits(BM_ARCHIVE_THREAD, 8);
							int Cr_value = get_bits(BM_ARCHIVE_THREAD, 8);
							int Cb_value = get_bits(BM_ARCHIVE_THREAD, 8);
							int T_value = get_bits(BM_ARCHIVE_THREAD, 8);
							nPESLength -= 4;
							processed_length += 4;
						}
						else
						{	
							int Y_value = get_bits(BM_ARCHIVE_THREAD, 6);
							int Cr_value = get_bits(BM_ARCHIVE_THREAD, 4);
							int Cb_value = get_bits(BM_ARCHIVE_THREAD, 4);
							int T_value = get_bits(BM_ARCHIVE_THREAD, 2);
							nPESLength -= 2;
							processed_length += 2;
						}
					}
				}
				break;
			case 0x13:		// Object Data Segment
				{
					int object_id = get_bits(BM_ARCHIVE_THREAD, 16);
					int object_version_number = get_bits(BM_ARCHIVE_THREAD, 4);
					int object_coding_method = get_bits(BM_ARCHIVE_THREAD, 2);
					int non_modifying_colour_flag = get_bits(BM_ARCHIVE_THREAD, 1);
					int reserved = get_bits(BM_ARCHIVE_THREAD, 1);
					int processed_length = 0;
					nPESLength -= 3;
					if (object_coding_method == 0)
					{	
						int top_field_data_block_length = get_bits(BM_ARCHIVE_THREAD, 16);
						int bottom_field_data_block_length = get_bits(BM_ARCHIVE_THREAD, 16);
						nPESLength -= 4;
						
						{
							int data_type = get_bits(BM_ARCHIVE_THREAD, 8);
							switch(data_type)
							{
							case 0x11:
								{
									int two_bit_pixel_code = get_bits(BM_ARCHIVE_THREAD, 2);
									int switch_1 = get_bits(BM_ARCHIVE_THREAD, 1);
									if (switch_1 == 1)
									{	
										int run_length_3_10 = get_bits(BM_ARCHIVE_THREAD, 3);
										int two_bit_pixel_code2 = get_bits(BM_ARCHIVE_THREAD, 2);
									}
									else
									{	
										int switch_2 = get_bits(BM_ARCHIVE_THREAD, 1);
										if (switch_2 == 0)
										{	
											int switch_3 = get_bits(BM_ARCHIVE_THREAD, 2);
											if (switch_3 == 2)
											{	
												int run_length_12_27 = get_bits(BM_ARCHIVE_THREAD, 4);
												int two_bit_pixel_code3 = get_bits(BM_ARCHIVE_THREAD, 2);
											}	
											if (switch_3 == 3)
											{	
												int run_length_29_284 = get_bits(BM_ARCHIVE_THREAD, 8);
												int two_bit_pixel_code4 = get_bits(BM_ARCHIVE_THREAD, 2);
											}	
										}	
									}	
								}
								break;
							}
						}
						/*while (processed_length < top_field_data_block_length)	
						{
							pixel_data_sub-block()	
						}
						while(processed_length < bottom_field_data_block_length)	
						{
							pixel_data_sub-block()	
						}
						if (!wordaligned())
						{
							int object8_stuff_bits = get_bits(BM_ARCHIVE_THREAD, 8);
							nPESLength -= 1;
						}*/
					}	
					if (object_coding_method == 1)
					{	
						int number_of_codes = get_bits(BM_ARCHIVE_THREAD, 8);
						int i;
						for (i = 1; i <= number_of_codes; i++)	
						{
							int character_code = get_bits(BM_ARCHIVE_THREAD, 16);
						}
						nPESLength -= 1 + (2 * number_of_codes);
					}	
				}
				break;
			default:		// Unknown - terminate processing
				return;
			}
			
		} while (nPESLength > 0);
	}
}

void ExtractVideoUserData(int nPMTIndex, BYTE * pPESPacket, int nPESLength)
{
	int nCurrentPos = 0;

	while (nCurrentPos < nPESLength - 4)
	{
		if (   pPESPacket[nCurrentPos + 0] == 0x00
			&& pPESPacket[nCurrentPos + 1] == 0x00
			&& pPESPacket[nCurrentPos + 2] == 0x01)
		{
			switch (pPESPacket[nCurrentPos + 3])
			{
			case 0xb2:
				{
					int nUserDataOffset = 0;
					BYTE bUserData[1024];

					nCurrentPos += 4;
					while (nCurrentPos < nPESLength - 4)
					{
						if (   pPESPacket[nCurrentPos + 0] == 0x00
							&& pPESPacket[nCurrentPos + 1] == 0x00
							&& pPESPacket[nCurrentPos + 2] == 0x01)
							break;
						bUserData[nUserDataOffset] = pPESPacket[nCurrentPos];
						nUserDataOffset++;
						nCurrentPos++;
					}
					if (nUserDataOffset)
						BufferAndWriteUserData(nPMTIndex, bUserData, nUserDataOffset);
				}
				break;
			case 0x00:	// picture start code
				{
					int new_temporal_reference;
					set_buf(BM_ARCHIVE_THREAD, &pPESPacket[nCurrentPos + 4], 0, FALSE);
					new_temporal_reference = get_bits(BM_ARCHIVE_THREAD, 10);
					if (new_temporal_reference < MAX_GOP_SIZE)
					{
						archive->ap[nPMTIndex]->nTemporalRererence = new_temporal_reference;
						archive->ap[nPMTIndex]->nPictureCodingType = get_bits(BM_ARCHIVE_THREAD, 3);
					}
				}
				break;
			}
		}
		nCurrentPos++;
	}
}

void BufferSubtitlePESData(int nPMTIndex, BYTE * pPacket)
{
	int nAdaptation = (pPacket[3] >> 4) & 0x03;
	BYTE * pWritePtr;
	int nWriteLen = 0;

	switch(nAdaptation)
	{
	case 0:		// not allowed
		break;	
	case 1:		// no adaptation
		pWritePtr = &pPacket[4];
		nWriteLen = 184;
		break;
	case 2:		// entirely adaptation
		break;
	case 3:		// adaptation + payload
		{
			int nAdaptationLen = pPacket[4];
			pWritePtr = &pPacket[5 + nAdaptationLen];
			nWriteLen = 188 - 5 - nAdaptationLen;
		}
		break;
	}

	if (!nWriteLen)
		return;
	if (nWriteLen < 0 || nWriteLen > 184)
		return;

	if ((pPacket[1] & 0x40) == 0x40)		// PES start?
	{
		BOOL fRetVal = FALSE;

		// If we have data from the last packet, finish that one
		if (archive->ap[nPMTIndex]->nSubtitleESFillPtr)
		{
			if (archive->ap[nPMTIndex]->nSubtitlePESLength == -1)
				archive->ap[nPMTIndex]->nSubtitlePESLength = archive->ap[nPMTIndex]->nSubtitleESFillPtr;
			ExtractSubtitleData(nPMTIndex, archive->ap[nPMTIndex]->bSubtitleESBuffer, archive->ap[nPMTIndex]->nSubtitlePESLength);
		}

		// Now start a new packet
		archive->ap[nPMTIndex]->nSubtitlePESLength = pWritePtr[4] << 8 | pWritePtr[5];
		if (nWriteLen - 14 > 0)
		{
			if (archive->ap[nPMTIndex]->nSubtitleESFillPtr + nWriteLen - 14 < ES_BUFFER_SIZE)
			{
				memcpy(archive->ap[nPMTIndex]->bSubtitleESBuffer, pWritePtr + 14, nWriteLen - 14);
				archive->ap[nPMTIndex]->nSubtitleESFillPtr = nWriteLen - 14;
			}
		}

		if (archive->ap[nPMTIndex]->nSubtitlePESLength != 0)
			archive->ap[nPMTIndex]->nSubtitlePESLength -= 8; // minus header
		else
			archive->ap[nPMTIndex]->nSubtitlePESLength = -1;
	}
	else			// continuation of prior packet
	{
		if (archive->ap[nPMTIndex]->nSubtitlePESLength)
		{
			if (nWriteLen > 0)
			{
				if (archive->ap[nPMTIndex]->nSubtitleESFillPtr + nWriteLen < ES_BUFFER_SIZE)
				{
					memcpy(archive->ap[nPMTIndex]->bSubtitleESBuffer + archive->ap[nPMTIndex]->nSubtitleESFillPtr, pWritePtr, nWriteLen);
					archive->ap[nPMTIndex]->nSubtitleESFillPtr += nWriteLen;
				}
			}
		}
	}
}

void BufferVideoPESData(int nPMTIndex, BYTE * pPacket)
{
	int nAdaptation = (pPacket[3] >> 4) & 0x03;
	BYTE * pWritePtr;
	int nWriteLen = 0;

	switch(nAdaptation)
	{
	case 0:		// not allowed
		break;	
	case 1:		// no adaptation
		pWritePtr = &pPacket[4];
		nWriteLen = 184;
		break;
	case 2:		// entirely adaptation
		break;
	case 3:		// adaptation + payload
		{
			int nAdaptationLen = pPacket[4];
			pWritePtr = &pPacket[5 + nAdaptationLen];
			nWriteLen = 188 - 5 - nAdaptationLen;
		}
		break;
	}

	if (!nWriteLen)
		return;
	if (nWriteLen < 0 || nWriteLen > 184)
		return;

	if ((pPacket[1] & 0x40) == 0x40)		// PES start?
	{
		BOOL fRetVal = FALSE;

		// If we have data from the last packet, finish that one
		if (archive->ap[nPMTIndex]->nVideoESFillPtr)
		{
			if (archive->ap[nPMTIndex]->nVideoPESLength == -1)
				archive->ap[nPMTIndex]->nVideoPESLength = archive->ap[nPMTIndex]->nVideoESFillPtr;
			ExtractVideoUserData(nPMTIndex, archive->ap[nPMTIndex]->bVideoESBuffer, archive->ap[nPMTIndex]->nVideoPESLength);
		}

		// Now start a new packet
		archive->ap[nPMTIndex]->nVideoPESLength = pWritePtr[4] << 8 | pWritePtr[5];
		if (nWriteLen - 14 > 0)
		{
			if (archive->ap[nPMTIndex]->nVideoESFillPtr + nWriteLen - 14 < ES_BUFFER_SIZE)
			{
				memcpy(archive->ap[nPMTIndex]->bVideoESBuffer, pWritePtr + 14, nWriteLen - 14);
				archive->ap[nPMTIndex]->nVideoESFillPtr = nWriteLen - 14;
			}
		}

		if (archive->ap[nPMTIndex]->nVideoPESLength != 0)
			archive->ap[nPMTIndex]->nVideoPESLength -= 8; // minus header
		else
			archive->ap[nPMTIndex]->nVideoPESLength = -1;
	}
	else			// continuation of prior packet
	{
		if (archive->ap[nPMTIndex]->nVideoPESLength)
		{
			if (nWriteLen > 0)
			{
				if (archive->ap[nPMTIndex]->nVideoESFillPtr + nWriteLen < ES_BUFFER_SIZE)
				{
					memcpy(archive->ap[nPMTIndex]->bVideoESBuffer + archive->ap[nPMTIndex]->nVideoESFillPtr, pWritePtr, nWriteLen);
					archive->ap[nPMTIndex]->nVideoESFillPtr += nWriteLen;
				}
			}
		}
	}
}

int ReadFromArchivePipe(BYTE * pBuffer, int nLength)
{
	int nRequestedLength = nLength;
	DWORD dwRead;

	while (nLength)
	{
		ReadFile(v->hArchiveReadPipe, pBuffer, nLength, &dwRead, NULL);
		if (dwRead == 0)
			return 0;
		pBuffer += dwRead;
		nLength -= dwRead;
	}

	return nRequestedLength;
}

void ArchivePIDData(BYTE * pPacket, int nPID)
{
	int nPMTIndex;
	int i;

	if (nPID == 0x1fff)
		return;

	if (nPID == 0)
	{
		// Received a PAT so output the appropriate PAT for each
		// recording stream plus a PMT too
		for (nPMTIndex = 0; nPMTIndex < MAX_PAT_ENTRIES; nPMTIndex++)
		{
			if (v->pat.pmt[nPMTIndex].nPMTPID == 0)
				break;
			if (v->pat.pmt[nPMTIndex].nProgramNumber != 0)
			{
				if (archive->ap[nPMTIndex]->fEnabled == TRUE)
				{
					WriteArchiveBlockBuffer(nPMTIndex, archive->ap[nPMTIndex]->pat, 188);
					archive->ap[nPMTIndex]->pat[3] = archive->ap[nPMTIndex]->pat[3] & 0xf0
						                           | ((archive->ap[nPMTIndex]->pat[3] & 0x0f) + 1) & 0x0f;

					if (v->pat.pmt[nPMTIndex].fCompleted)
					{
						archive->ap[nPMTIndex]->pmt[(188 * archive->ap[nPMTIndex]->nPMTPacketOutputCounter) + 3] = 
							archive->ap[nPMTIndex]->pmt[(188 * archive->ap[nPMTIndex]->nPMTPacketOutputCounter) + 3] & 0xf0 |
							archive->ap[nPMTIndex]->nPMTOutputContinuityCounter;
						archive->ap[nPMTIndex]->nPMTOutputContinuityCounter++;
						archive->ap[nPMTIndex]->nPMTOutputContinuityCounter &= 0x0f;
						WriteArchiveBlockBuffer(nPMTIndex, &archive->ap[nPMTIndex]->pmt[188 * archive->ap[nPMTIndex]->nPMTPacketOutputCounter], 188);
						archive->ap[nPMTIndex]->nPMTPacketOutputCounter++;
						if (archive->ap[nPMTIndex]->nPMTPacketOutputCounter == archive->ap[nPMTIndex]->nPMTPackets)
							archive->ap[nPMTIndex]->nPMTPacketOutputCounter = 0;
					}
				}
			}
		}
	}
	else
	{
		for (i = 0; i < MAX_ARCHIVE_PID_STREAMS; i++)
		{
			if (archive->nArchivePIDs[nPID][i] == 0)
				return;
			nPMTIndex = archive->nArchivePIDs[nPID][i] - 1;
			if (nPMTIndex < 0)
				continue;

			if (archive->ap[nPMTIndex]->fEnabled && archive->ap[nPMTIndex]->hOutputFile != INVALID_HANDLE_VALUE)
			{
				WriteArchiveBlockBuffer(nPMTIndex, pPacket, 188);
				archive->ap[nPMTIndex]->dCurrentFileWritten += 188.0;
				archive->ap[nPMTIndex]->dAllBytesWritten += 188.0;
				archive->ap[nPMTIndex]->nGraphBytes += 188;
				if ((pPacket[3] & 0xc0) != 0)
				{
					archive->ap[nPMTIndex]->nStatus |= STATUS_SCRAMBLED;
					continue;
				}
				archive->ap[nPMTIndex]->nStatus &= ~STATUS_SCRAMBLED;

				// Build PES packets for the video stream to get the user data
				if (archive->ap[nPMTIndex]->nVideoPID != 0x1fff)
				{
					if (archive->ap[nPMTIndex]->nVideoPID == nPID)
						BufferVideoPESData(nPMTIndex, pPacket);
				}
				// Build PES packets for the DVB subtitle stream
				if (archive->ap[nPMTIndex]->nSubtitlePID != 0x1fff)
				{
					if (archive->ap[nPMTIndex]->nSubtitlePID == nPID)
						BufferSubtitlePESData(nPMTIndex, pPacket);
				}
			}
		}
	}
}

void WriteXMLFile(int nPMTIndex)
{
	HANDLE hXMLFile;
	char * szExtensionPtr;
	char szXMLName[MAX_PATH];

	lstrcpy(szXMLName, archive->ap[nPMTIndex]->szOutputFileName);
	szExtensionPtr = GetExtensionPtr(szXMLName);
	lstrcpy(szExtensionPtr, ".xml");
	
	hXMLFile = CreateFile(szXMLName, GENERIC_WRITE, FILE_SHARE_READ, (LPSECURITY_ATTRIBUTES) NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, (HANDLE) NULL);
	if (hXMLFile != INVALID_HANDLE_VALUE)
	{
		DWORD dwWritten;
		int nThumbnailIndex;
		int nSyncLosses = 0;
		int nCCErrorCount = 0;
		int nTEICount = 0;
		char szTemp[1024];

		lstrcpy(szTemp, "<?xml version=\"1.0\" encoding=\"ISO-8859-1\" ?>\r\n");
		WriteFile(hXMLFile, szTemp, lstrlen(szTemp), &dwWritten, NULL);

		lstrcpy(szTemp, "<EVENT>\r\n");
		WriteFile(hXMLFile, szTemp, lstrlen(szTemp), &dwWritten, NULL);

		wsprintf(szTemp, "<CHANNEL>%s</CHANNEL>\r\n", archive->ap[nPMTIndex]->szChannelName);
		WriteFile(hXMLFile, szTemp, lstrlen(szTemp), &dwWritten, NULL);
		wsprintf(szTemp, "<SCHEDULED-START-DATE>%04d/%02d/%02d</SCHEDULED-START-DATE>\r\n", 
			     archive->ap[nPMTIndex]->stCurrentStart.wYear,
				 archive->ap[nPMTIndex]->stCurrentStart.wMonth,
				 archive->ap[nPMTIndex]->stCurrentStart.wDay);
		WriteFile(hXMLFile, szTemp, lstrlen(szTemp), &dwWritten, NULL);
		wsprintf(szTemp, "<SCHEDULED-START-TIME>%02d:%02d:%02d</SCHEDULED-START-TIME>\r\n",
				 archive->ap[nPMTIndex]->stCurrentStart.wHour,
				 archive->ap[nPMTIndex]->stCurrentStart.wMinute,
				 archive->ap[nPMTIndex]->stCurrentStart.wSecond);
		WriteFile(hXMLFile, szTemp, lstrlen(szTemp), &dwWritten, NULL);
		wsprintf(szTemp, "<ACTUAL-START-DATE>%04d/%02d/%02d</ACTUAL-START-DATE>\r\n", 
			     archive->ap[nPMTIndex]->stActualStart.wYear,
				 archive->ap[nPMTIndex]->stActualStart.wMonth,
				 archive->ap[nPMTIndex]->stActualStart.wDay);
		WriteFile(hXMLFile, szTemp, lstrlen(szTemp), &dwWritten, NULL);
		wsprintf(szTemp, "<ACTUAL-START-TIME>%02d:%02d:%02d</ACTUAL-START-TIME>\r\n",
				 archive->ap[nPMTIndex]->stActualStart.wHour,
				 archive->ap[nPMTIndex]->stActualStart.wMinute,
				 archive->ap[nPMTIndex]->stActualStart.wSecond);
		WriteFile(hXMLFile, szTemp, lstrlen(szTemp), &dwWritten, NULL);

		wsprintf(szTemp, "<DURATION>%02d:%02d:%02d</DURATION>\r\n",
				 archive->ap[nPMTIndex]->stCurrentDuration.wHour,
				 archive->ap[nPMTIndex]->stCurrentDuration.wMinute,
				 archive->ap[nPMTIndex]->stCurrentDuration.wSecond);
		WriteFile(hXMLFile, szTemp, lstrlen(szTemp), &dwWritten, NULL);
		wsprintf(szTemp, "<PROGRAM>%s</PROGRAM>\r\n", archive->ap[nPMTIndex]->szCurrentProgram);
		WriteFile(hXMLFile, szTemp, lstrlen(szTemp), &dwWritten, NULL);
		wsprintf(szTemp, "<DESCRIPTION>%s</DESCRIPTION>\r\n", archive->ap[nPMTIndex]->szCurrentDescription);
		WriteFile(hXMLFile, szTemp, lstrlen(szTemp), &dwWritten, NULL);
		wsprintf(szTemp, "<RATING>%s</RATING>\r\n", archive->ap[nPMTIndex]->szCurrentRating);
		WriteFile(hXMLFile, szTemp, lstrlen(szTemp), &dwWritten, NULL);

		if (v->nSyncLossCount > archive->ap[nPMTIndex]->nSyncLossCount)
			nSyncLosses = v->nSyncLossCount - archive->ap[nPMTIndex]->nSyncLossCount;
		wsprintf(szTemp, "<SYNC-LOSSES>%d</SYNC-LOSSES>\r\n", nSyncLosses);
		WriteFile(hXMLFile, szTemp, lstrlen(szTemp), &dwWritten, NULL);
		if (GetProgramCCorTEICount(nPMTIndex, TRUE) > archive->ap[nPMTIndex]->nContinuityErrorCount)
			nCCErrorCount = GetProgramCCorTEICount(nPMTIndex, TRUE) - archive->ap[nPMTIndex]->nContinuityErrorCount;
		wsprintf(szTemp, "<ES-CC-ERRORS>%d</ES-CC-ERRORS>\r\n", nCCErrorCount);
		WriteFile(hXMLFile, szTemp, lstrlen(szTemp), &dwWritten, NULL);
		if (GetProgramCCorTEICount(nPMTIndex, FALSE) > archive->ap[nPMTIndex]->nTEIErrorCount)
			nTEICount = GetProgramCCorTEICount(nPMTIndex, FALSE) - archive->ap[nPMTIndex]->nTEIErrorCount;
		wsprintf(szTemp, "<ES-TEI-ERRORS>%d</ES-TEI-ERRORS>\r\n", nTEICount);
		WriteFile(hXMLFile, szTemp, lstrlen(szTemp), &dwWritten, NULL);

		lstrcpy(szTemp, "<THUMBNAILS>\r\n");
		WriteFile(hXMLFile, szTemp, lstrlen(szTemp), &dwWritten, NULL);

		EnterCriticalSection(&archive->ap[nPMTIndex]->csThumbnailList);
		for (nThumbnailIndex = 0; nThumbnailIndex < MAX_THUMBNAILS; nThumbnailIndex++)
		{
			if (archive->ap[nPMTIndex]->tl[nThumbnailIndex]->fActive == FALSE)
				break;

			wsprintf(szTemp, "<THUMBNAIL%d>\r\n", nThumbnailIndex + 1);
			WriteFile(hXMLFile, szTemp, lstrlen(szTemp), &dwWritten, NULL);

			wsprintf(szTemp, "<FILENAME>%s</FILENAME>\r\n", archive->ap[nPMTIndex]->tl[nThumbnailIndex]->szFilename);
			WriteFile(hXMLFile, szTemp, lstrlen(szTemp), &dwWritten, NULL);
			wsprintf(szTemp, "<TIME>%02d:%02d:%02d</TIME>\r\n",
					 archive->ap[nPMTIndex]->tl[nThumbnailIndex]->st.wHour,
					 archive->ap[nPMTIndex]->tl[nThumbnailIndex]->st.wMinute,
					 archive->ap[nPMTIndex]->tl[nThumbnailIndex]->st.wSecond);
			WriteFile(hXMLFile, szTemp, lstrlen(szTemp), &dwWritten, NULL);
			wsprintf(szTemp, "<DATE>%04d/%02d/%02d</DATE>\r\n", 
					 archive->ap[nPMTIndex]->tl[nThumbnailIndex]->st.wYear,
					 archive->ap[nPMTIndex]->tl[nThumbnailIndex]->st.wMonth,
					 archive->ap[nPMTIndex]->tl[nThumbnailIndex]->st.wDay);
			WriteFile(hXMLFile, szTemp, lstrlen(szTemp), &dwWritten, NULL);
			wsprintf(szTemp, "<MPEG-OFFSET-H>%u</MPEG-OFFSET-H>\r\n",  archive->ap[nPMTIndex]->tl[nThumbnailIndex]->lnByteOffset.HighPart);
			WriteFile(hXMLFile, szTemp, lstrlen(szTemp), &dwWritten, NULL);
			wsprintf(szTemp, "<MPEG-OFFSET-L>%u	</MPEG-OFFSET-L>\r\n",  archive->ap[nPMTIndex]->tl[nThumbnailIndex]->lnByteOffset.LowPart);
			WriteFile(hXMLFile, szTemp, lstrlen(szTemp), &dwWritten, NULL);
			
			wsprintf(szTemp, "</THUMBNAIL%d>\r\n", nThumbnailIndex + 1);
			WriteFile(hXMLFile, szTemp, lstrlen(szTemp), &dwWritten, NULL);
			
			archive->ap[nPMTIndex]->tl[nThumbnailIndex]->fActive = FALSE;
		}
		LeaveCriticalSection(&archive->ap[nPMTIndex]->csThumbnailList);
		lstrcpy(szTemp, "</THUMBNAILS>\r\n");
		WriteFile(hXMLFile, szTemp, lstrlen(szTemp), &dwWritten, NULL);

		lstrcpy(szTemp, "</EVENT>\r\n");
		WriteFile(hXMLFile, szTemp, lstrlen(szTemp), &dwWritten, NULL);

		CloseHandle(hXMLFile);
	}
	else
	{
		char szTemp[128 + MAX_PATH];
		wsprintf(szTemp, "TSReader: Archive.c: Unable to open XML file %s\n", szXMLName);
		OutputDebugString(szTemp);
	}
}

void RestartArchiveChannel(int nPMTIndex)
{
	int nSyncLosses = 0;
	int nCCErrorCount = 0;
	int nTEICount = 0;
	char szTemp[1024];
	char szChannelName[256];

	CloseUserDataAndCaptionFiles(nPMTIndex);
	FlushBufferData(nPMTIndex);
	WriteXMLFile(nPMTIndex);

	if (v->nSyncLossCount > archive->ap[nPMTIndex]->nSyncLossCount)
		nSyncLosses = v->nSyncLossCount - archive->ap[nPMTIndex]->nSyncLossCount;
	if (GetProgramCCorTEICount(nPMTIndex, TRUE) > archive->ap[nPMTIndex]->nContinuityErrorCount)
		nCCErrorCount = GetProgramCCorTEICount(nPMTIndex, TRUE) - archive->ap[nPMTIndex]->nContinuityErrorCount;
	if (GetProgramCCorTEICount(nPMTIndex, FALSE) > archive->ap[nPMTIndex]->nTEIErrorCount)
		nTEICount = GetProgramCCorTEICount(nPMTIndex, FALSE) - archive->ap[nPMTIndex]->nTEIErrorCount;

	lstrcpy(szChannelName, archive->ap[nPMTIndex]->szChannelName);
	if (lstrlen(szChannelName) == 0)
		wsprintf(szChannelName, "%05d", v->pat.pmt[nPMTIndex].nProgramNumber);	
	wsprintf(szTemp, "Stop: %s - cc:%d,tei:%d,sync:%d %s",
		     szChannelName, 
			 nCCErrorCount, nTEICount, nSyncLosses,
			 archive->ap[nPMTIndex]->szCurrentProgram);
	ArchiveEventListWrite(EVENT_ICON_INFORMATION, szTemp, "");
	
	archive->ap[nPMTIndex]->hPriorOutputFile = archive->ap[nPMTIndex]->hOutputFile;
	archive->ap[nPMTIndex]->nSyncLossCount = v->nSyncLossCount;
	archive->ap[nPMTIndex]->nContinuityErrorCount = GetProgramCCorTEICount(nPMTIndex, TRUE);
	archive->ap[nPMTIndex]->nTEIErrorCount = GetProgramCCorTEICount(nPMTIndex, FALSE);
}

BOOL EPGDataWithin15SecondsPresent(int nPMTIndex)
{
	BOOL fRetVal = FALSE;
	int nServiceID = v->pat.pmt[nPMTIndex].nProgramNumber;
	PEITEVENT pCurrent;
	SYSTEMTIME stSystemTime;

	EnterCriticalSection(&v->csEIT);
	pCurrent = v->pEvents[nServiceID];
	if (pCurrent != NULL)
	{
		DWORD64 lnProgramStart, lnNow;
		DWORD64 lnDifference;
		DWORD64 lnMultiplier = 10000000;

		GetSystemTime(&stSystemTime);
		SystemTimeToFileTime(&stSystemTime, (FILETIME *)&lnNow);
		do
		{
			SystemTimeToFileTime(&pCurrent->stStartTime, (FILETIME *)&lnProgramStart);
			lnDifference = (lnProgramStart - lnNow) / lnMultiplier;
			{
				char szTemp[256];
				wsprintf(szTemp, "TSReader: %02d:%02d:%02d %02d:%02d:%02d %d %s\n",
					pCurrent->stStartTime.wHour, pCurrent->stStartTime.wMinute, pCurrent->stStartTime.wSecond,
					stSystemTime.wHour, stSystemTime.wMinute, stSystemTime.wSecond,
					(int)lnDifference,
					pCurrent->szEventName);
				OutputDebugString(szTemp);
			}
			
			if (lnDifference >= 0 && lnDifference <= 15)
			{
				fRetVal = TRUE;
				break;
			}
			pCurrent = (PEITEVENT)pCurrent->dwNextEvent;
		} while (pCurrent != NULL);
	}
	LeaveCriticalSection(&v->csEIT);
	return fRetVal;
}

void CheckEPG()
{
	SYSTEMTIME stNow;
	__int64 ftNow, ftTimeCheck;


	GetSystemTime(&stNow);
	stNow.wMilliseconds = 0;
	SystemTimeToFileTime(&stNow, (FILETIME *)&ftNow);
	SystemTimeToFileTime(&archive->stTimeCheck, (FILETIME *)&ftTimeCheck);
	if (ftNow > ftTimeCheck || archive->fForceNew)
	{
		int nPMTIndex;
		
		memcpy(&archive->stTimeCheck, &stNow, sizeof(SYSTEMTIME));

		for (nPMTIndex = 0; nPMTIndex < MAX_PAT_ENTRIES; nPMTIndex++)
		{
			if (v->pat.pmt[nPMTIndex].nPMTPID == 0)
				break;
			if (v->pat.pmt[nPMTIndex].nProgramNumber != 0)
			{
				if (archive->ap[nPMTIndex]->fEnabled == TRUE)
				{
ReCheckEPG:
					switch(archive->ap[nPMTIndex]->nEPGType)
					{
					case EPG_TYPE_30_MIN:
						if (archive->ap[nPMTIndex]->fPreviouslyLocalEPG == TRUE)
						{
							// See if there's EPG data for the channel now
							if (LocateCurrentProgram(nPMTIndex, NULL, NULL, NULL, NULL, FALSE) == TRUE)
							{
								archive->ap[nPMTIndex]->fPreviouslyLocalEPG = FALSE;
								archive->ap[nPMTIndex]->nEPGType = EPG_TYPE_LOCAL_EPG;
								archive->fForceNew = TRUE;
								goto ReCheckEPG;
							}
						}
						else if (archive->ap[nPMTIndex]->fPreviouslyRemoteEPG == TRUE)
						{
							if (!--archive->ap[nPMTIndex]->nRemoteEPGReconnectCounter)
							{
								if (OpenRemoteEPGConnection(nPMTIndex) == TRUE)
								{
									int nTimeout = 1000;

									// Wait for the event name to arrive on the socket
									do
									{
										if (archive->ap[nPMTIndex]->fNewRemoteProgramDetected)
											break;
										Sleep(10);
									} while (nTimeout-- > 0);
									archive->ap[nPMTIndex]->fPreviouslyRemoteEPG = FALSE;
									archive->ap[nPMTIndex]->nEPGType = EPG_TYPE_REMOTE_EPG;
									goto ReCheckEPG;
								}
								else
								{
									// Still can't connect - retry again in a minute
									archive->ap[nPMTIndex]->nRemoteEPGReconnectCounter = 60;
								}
							}
						}

						if (    stNow.wSecond == 0
							&& (stNow.wMinute == 0 || stNow.wMinute == 30)
							|| archive->fForceNew)
						{
							RestartArchiveChannel(nPMTIndex);
							archive->ap[nPMTIndex]->szCurrentProgram[0] = '\0';
							archive->ap[nPMTIndex]->szCurrentDescription[0] = '\0';
							GetSystemTime(&archive->ap[nPMTIndex]->stCurrentStart);
							memset(&archive->ap[nPMTIndex]->stCurrentDuration, 0, sizeof(archive->ap[nPMTIndex]->stCurrentDuration));							
							archive->ap[nPMTIndex]->stCurrentDuration.wMinute = 30;
							OpenCurrentArchiveFile(nPMTIndex);
						}
						break;
					case EPG_TYPE_LOCAL_EPG:
						{
							SYSTEMTIME stStart, stDuration;
							char szCurrentProgram[1024];
							char szCurrentDescription[4096];
							char szCurrentRating[64];

							if (LocateCurrentProgram(nPMTIndex, szCurrentProgram, szCurrentDescription, &stStart, &stDuration, FALSE) == TRUE)
							{
								if (lstrlen(szCurrentProgram) == 0)
									break;		// ignore null length programs
								RemoveATSCRatingFromProgram(szCurrentProgram, szCurrentRating);							
								if (memcmp(&archive->ap[nPMTIndex]->stCurrentStart, &stStart, sizeof(SYSTEMTIME)) != 0
									|| archive->fForceNew)
								{
									RestartArchiveChannel(nPMTIndex);
									lstrcpy(archive->ap[nPMTIndex]->szCurrentProgram, szCurrentProgram);
									lstrcpy(archive->ap[nPMTIndex]->szCurrentDescription, szCurrentDescription);
									memcpy(&archive->ap[nPMTIndex]->stCurrentStart, &stStart, sizeof(SYSTEMTIME));
									memcpy(&archive->ap[nPMTIndex]->stCurrentDuration, &stDuration, sizeof(SYSTEMTIME));
									lstrcpy(archive->ap[nPMTIndex]->szCurrentRating, szCurrentRating);
									OpenCurrentArchiveFile(nPMTIndex);
								}
							}
							else
							{
								// No EPG data for this channel - there was before
								// but there isn't now. Switch to 30 minute mode and
								// flag to see if we can switch back when the EPG returns
								// First check to see if there's an event starting within 15 seconds
								// if so, we'll wait until it starts
								if (EPGDataWithin15SecondsPresent(nPMTIndex) == TRUE)
									break;
								archive->ap[nPMTIndex]->nEPGType = EPG_TYPE_30_MIN;
								archive->ap[nPMTIndex]->fPreviouslyLocalEPG = TRUE;
								archive->fForceNew = TRUE;
								goto ReCheckEPG;
							}
						}
						break;
					case EPG_TYPE_REMOTE_EPG:
						{
							if (archive->ap[nPMTIndex]->fNewRemoteProgramDetected || archive->fForceNew)
							{
								RestartArchiveChannel(nPMTIndex);
								if (archive->ap[nPMTIndex]->fNewRemoteProgramDetected)
								{
									SetupNewRemoteEPGProgramDetails(nPMTIndex);
									archive->ap[nPMTIndex]->fNewRemoteProgramDetected = FALSE;
								}
								OpenCurrentArchiveFile(nPMTIndex);
							}
							else if (!archive->ap[nPMTIndex]->fRemoteEPGThreadRunning)
							{
								// There is an issue with the socket - switch to 30 minute mode
								// We check every 60 seconds to see if we can reconnect
								archive->ap[nPMTIndex]->nEPGType = EPG_TYPE_30_MIN;
								archive->ap[nPMTIndex]->fPreviouslyRemoteEPG = TRUE;
								archive->ap[nPMTIndex]->nRemoteEPGReconnectCounter = 61;
								archive->fForceNew = TRUE;
								goto ReCheckEPG;
							}
						}
						break;
					}
				}
			}
		}
		if (archive->fForceNew)
			archive->fForceNew = FALSE;
	}
}

void CheckOutstandingFileCloses()
{
	int nPMTIndex;

	for (nPMTIndex = 0; nPMTIndex < MAX_PAT_ENTRIES; nPMTIndex++)
	{
		if (v->pat.pmt[nPMTIndex].nPMTPID == 0)
			break;
		if (v->pat.pmt[nPMTIndex].nProgramNumber != 0)
		{
			if (archive->ap[nPMTIndex]->fEnabled == TRUE)
			{
				if (archive->ap[nPMTIndex]->hPriorOutputFile != NULL)
				{
					int nAsyncBufferIndex;

					for (nAsyncBufferIndex = 0; nAsyncBufferIndex < MAX_ASYNC_IO; nAsyncBufferIndex++)
					{
						if (archive->ap[nPMTIndex]->hAsyncOutputFile[nAsyncBufferIndex] == archive->ap[nPMTIndex]->hPriorOutputFile)
						{
							// still outstanding I/Os for this file
							continue;
						}
					}
					CloseHandle(archive->ap[nPMTIndex]->hPriorOutputFile);
					archive->ap[nPMTIndex]->hPriorOutputFile = NULL;
					{
						char szTemp[128];
						wsprintf(szTemp, "TSReader: Closed prior file on channel %d\n", v->pat.pmt[nPMTIndex].nProgramNumber);
						OutputDebugString(szTemp);
					}
				}
			}
		}
	}
}

void AllocateOrDeAllocateThumbnailBuffers(BOOL fAllocate)
{
	int nPMTIndex;

	for (nPMTIndex = 0; nPMTIndex < MAX_PAT_ENTRIES; nPMTIndex++)
	{
		if (v->pat.pmt[nPMTIndex].nPMTPID == 0)
			break;
		if (v->pat.pmt[nPMTIndex].nProgramNumber != 0)
		{
			if (archive->ap[nPMTIndex]->fEnabled == TRUE)
			{
				int i;

				for (i = MAX_THUMBNAILS - 1; i >= 0 ; i--)
				{
					if (fAllocate)
					{
						archive->ap[nPMTIndex]->tl[i] = LocalAlloc(LPTR, sizeof(THUMBNAILLIST));
						if (archive->ap[nPMTIndex]->tl[i] == NULL)
						{
							OutputDebugString("TSReader: Archive - can't allocate thumbnail buffer\n");
						}
					}
					else
					{
						LocalFree(archive->ap[nPMTIndex]->tl[i]);
						archive->ap[nPMTIndex]->tl[i] = NULL;
					}
				}
			}
		}
	}
}

void UpdateGraphData()
{
	int nPMTIndex;

	for (nPMTIndex = 0; nPMTIndex < MAX_PAT_ENTRIES; nPMTIndex++)
	{
		int i;

		if (v->pat.pmt[nPMTIndex].nProgramNumber == 0)
			continue;
		if (v->pat.pmt[nPMTIndex].nPMTPID == 0)
			break;
		if (archive->ap[nPMTIndex]->fEnabled)
		{
			archive->ap[nPMTIndex]->nLastSampleGraphBytes = archive->ap[nPMTIndex]->nGraphBytes * 4;
			if (archive->ap[nPMTIndex]->nLastSampleGraphBytes == 0)
			{
				archive->ap[nPMTIndex]->nNoDataCounter++;
				if (archive->ap[nPMTIndex]->nNoDataCounter > 4)
					archive->ap[nPMTIndex]->nStatus |= STATUS_NO_DATA;
			}
			else
			{
				archive->ap[nPMTIndex]->nNoDataCounter = 0;
				archive->ap[nPMTIndex]->nStatus &= ~STATUS_NO_DATA;
			}
			archive->ap[nPMTIndex]->nGraphBytes = 0;
			archive->ap[nPMTIndex]->dGraphBytesTotal += archive->ap[nPMTIndex]->nLastSampleGraphBytes;
			archive->ap[nPMTIndex]->dGraphBytesTotalCount += 1.0;

			if (archive->ap[nPMTIndex]->nLastSampleGraphBytes > archive->ap[nPMTIndex]->nGraphBytesMax)
				archive->ap[nPMTIndex]->nGraphBytesMax = archive->ap[nPMTIndex]->nLastSampleGraphBytes;
			if (archive->ap[nPMTIndex]->nLastSampleGraphBytes && archive->ap[nPMTIndex]->nLastSampleGraphBytes < archive->ap[nPMTIndex]->nGraphBytesMin)
				archive->ap[nPMTIndex]->nGraphBytesMin = archive->ap[nPMTIndex]->nLastSampleGraphBytes;

			for (i = 0; i < MAX_HISTORIC_SAMPLES - 1; i++)
				archive->ap[nPMTIndex]->nHistoricLastSampleGraphBytes[i] = archive->ap[nPMTIndex]->nHistoricLastSampleGraphBytes[i + 1];
			archive->ap[nPMTIndex]->nHistoricLastSampleGraphBytes[MAX_HISTORIC_SAMPLES - 1] = archive->ap[nPMTIndex]->nLastSampleGraphBytes;
		}
	}
}

DWORD WINAPI ArchiveGraphThread(LPVOID lpv)
{
	DWORD dwTickCount = GetTickCount();
	while (!archive->fTerminateGraphThread)
	{
		if (GetTickCount() > dwTickCount + 250)
		{
			dwTickCount = GetTickCount();
			UpdateGraphData();
		}
		Sleep(10);	
	}
	return 0;
}

DWORD WINAPI ArchiveThread(LPVOID lpv)
{
	int nPMTIndex;
	HANDLE hThread;
	DWORD dwThreadID;
	BYTE tspacket[188];

	archive->fTerminateGraphThread = FALSE;
	hThread = CreateThread(NULL, 0, ArchiveGraphThread, (LPVOID)0, 0, &dwThreadID);
	CloseHandle(hThread);

	BuildArchivePIDList();
	GenerateArchivePATs();
	GenerateInitialArchiveFiles();
	AllocateOrDeAllocateThumbnailBuffers(TRUE);

	while (ReadFromArchivePipe(tspacket, 188) != 0)
	{
		BOOL fSleep = FALSE;
		int nPID = (tspacket[1] << 8 | tspacket[2]) & 0x1fff;
		ArchivePIDData(tspacket, nPID);
		EnterCriticalSection(&archive->csPipeBytes);
		archive->nPipeBytes -= 188;
		if (archive->nPipeBytes == 0)
			fSleep = TRUE;
		LeaveCriticalSection(&archive->csPipeBytes);

		if (fSleep)
		{
			CheckEPG();
			CheckOutstandingFileCloses();
			SleepEx(5, TRUE);
		}
	}
	archive->fTerminateGraphThread = TRUE;
	
	for (nPMTIndex = 0; nPMTIndex < MAX_PAT_ENTRIES; nPMTIndex++)
	{
		if (v->pat.pmt[nPMTIndex].nPMTPID == 0)
			break;
		if (v->pat.pmt[nPMTIndex].nProgramNumber != 0)
		{
			if (archive->ap[nPMTIndex]->fEnabled == TRUE)
			{
				int i;
				BOOL fAllEmpty = FALSE;

				// Flush any outstanding buffers
				if (archive->ap[nPMTIndex]->nWriteBufferOffset[archive->ap[nPMTIndex]->nCurrentAsyncIO])
					FlushBufferData(nPMTIndex);

				// Wait for them to flush
				while (fAllEmpty == FALSE)
				{
					for (i = 0; i < MAX_ASYNC_IO; i++)
					{
						if (archive->ap[nPMTIndex]->nWriteBufferOffset[i] != 0)
							break;
					}
					if (i == MAX_ASYNC_IO)
						fAllEmpty = TRUE;
					else
						SleepEx(5, TRUE);
				}

				// Wait for the remote EPG to shutdown
				if (archive->ap[nPMTIndex]->nEPGType == EPG_TYPE_REMOTE_EPG)
				{
					if (archive->ap[nPMTIndex]->fRemoteEPGThreadRunning == TRUE)
					{
						int nTimeout = 100;
						closesocket(archive->ap[nPMTIndex]->EPGSocket);
						while (archive->ap[nPMTIndex]->fRemoteEPGThreadRunning && nTimeout-- > 0)
							Sleep(50);
					}
				}				

				for (i = 0; i < MAX_ASYNC_IO; i++)	
					LocalFree(archive->ap[nPMTIndex]->pBlockBuffer[i]);
				CloseUserDataAndCaptionFiles(nPMTIndex);
				CloseHandle(archive->ap[nPMTIndex]->hOutputFile);
				WriteXMLFile(nPMTIndex);
				DeleteCriticalSection(&archive->ap[nPMTIndex]->csThumbnailList);
			}
		}
	}

	//AllocateOrDeAllocateThumbnailBuffers(FALSE);
	
	OutputDebugString("ArchiveThread-\n");
	return 0;
}

void ArchiveProgramData(BYTE * pData, int nLength)
{
	if (v->fArchiveRunning && archive->fReadyForData)
	{
		DWORD dwWritten;
	
		WriteFile(v->hArchiveWritePipe, pData, nLength, &dwWritten, NULL);
		if (dwWritten != (DWORD)nLength)
		{
			OutputDebugString("*************ARCHIVE BUFFER WRITE PROBLEM**************\n");
		}
		EnterCriticalSection(&archive->csPipeBytes);
		archive->nPipeBytes += dwWritten;
		LeaveCriticalSection(&archive->csPipeBytes);
	}
}

void AllocateOrDeAllocateArchiveProgramBuffers(BOOL fAllocate)
{
	int nPMTIndex;

	for (nPMTIndex = 0; nPMTIndex < MAX_PAT_ENTRIES; nPMTIndex++)
	{
		if (v->pat.pmt[nPMTIndex].nPMTPID == 0)
			break;
		if (v->pat.pmt[nPMTIndex].nProgramNumber != 0)
		{
			if (fAllocate)
				archive->ap[nPMTIndex] = LocalAlloc(LPTR, sizeof(ARCHIVEPROGRAM));
			else
				LocalFree(archive->ap[nPMTIndex]);
		}
	}
}

BOOL StartArchivePrograms(HWND hWnd)
{
	archive = LocalAlloc(LPTR, sizeof(ARCHIVE));
	AllocateOrDeAllocateArchiveProgramBuffers(TRUE);
	if (DialogBox(v->hInstance, MAKEINTRESOURCE(IDD_ARCHIVE_SETUP), hWnd, ArchiveSetupDlgProc) == TRUE)
	{
		HANDLE hThread;
		DWORD dwThreadID;

		if (archive->fAutoRestart)
		{
			char szExecutable[MAX_PATH];
			char szCommandLine[MAX_PATH] = {""};
			char szTemp[MAX_PATH * 2];

			SourceHelper_GetTSReaderEXEDirectory(v->hInstance, szExecutable, sizeof(szExecutable));
			lstrcat(szExecutable, "\\TSReader_ArchiveMonitor.exe");

			if (lstrlen(v->szProfileName))
				lstrcpy(szCommandLine, v->szProfileName);
			lstrcat(szCommandLine, ",");
			lstrcat(szCommandLine, v->szSourceParametersPtr);
			wsprintf(szTemp, "\"%s\" %s", szExecutable, szCommandLine);
			WinExec(szTemp, SW_SHOW);
		}

		InitCCDecoder();
		InitializeCriticalSection(&archive->csEventList);
		InitializeCriticalSection(&archive->csPipeBytes);

		CreatePipe(&v->hArchiveReadPipe, &v->hArchiveWritePipe, NULL, 1024 * 1024 * 15);
		hThread = CreateThread(NULL, 0, ArchiveThread, (LPVOID)0, 0, &dwThreadID);
		CloseHandle(hThread);
		GetSystemTime(&archive->stTimeCheck);
		archive->stTimeCheck.wMilliseconds = 0;
		
		archive->hWndStatus = CreateDialog(v->hInstance, MAKEINTRESOURCE(IDD_ARCHIVE_RUN), hWnd, ArchiveRunDialogProc);
		ShowWindow(archive->hWndStatus, SW_SHOW);		
		archive->fReadyForData = TRUE;

		return TRUE;
	}

	AllocateOrDeAllocateArchiveProgramBuffers(FALSE);
	LocalFree(archive);
	return FALSE;
}

void StopArchivePrograms(HWND hWnd)
{
	v->fArchiveRunning = FALSE;

	CloseHandle(v->hArchiveWritePipe);
	Sleep(1000);

	DestroyWindow(archive->hWndStatus);
	DeleteCriticalSection(&archive->csEventList);
	DeleteCriticalSection(&archive->csPipeBytes);

	AllocateOrDeAllocateArchiveProgramBuffers(FALSE);

	if (archive->fAutoRestart)
	{
		if (v->hWndArchiveMonitor != NULL)
		{
			SendMessage(v->hWndArchiveMonitor, WM_USER + 2, 0, 0);
			v->hWndArchiveMonitor = NULL;
		}
	}
	LocalFree(archive);
}

BOOL GetArchiveThumbnailName(int nPMTIndex, char * szThumbnailName)
{
	int nThumbnailIndex;
	SYSTEMTIME st;
	char szChannelName[128];
	char szTemp[256];

	if (archive->ap[nPMTIndex]->fEnabled == FALSE)
		return FALSE;

	lstrcpy(szThumbnailName, archive->ap[nPMTIndex]->szOutputLocation);
	if (szThumbnailName[lstrlen(szThumbnailName) - 1] != '\\')
		lstrcat(szThumbnailName, "\\");
	lstrcat(szThumbnailName, "jpg\\");
	
	GetSystemTime(&st);
	lstrcpy(szChannelName, archive->ap[nPMTIndex]->szChannelName);
	if (lstrlen(szChannelName) == 0)
		wsprintf(szChannelName, "%05d", v->pat.pmt[nPMTIndex].nProgramNumber);
	wsprintf(szTemp, "%s_%04d%02d%02d_%02d%02d%02d.jpg", 
			 szChannelName,
			 st.wYear, st.wMonth, st.wDay,
			 st.wHour, st.wMinute, st.wSecond);
	lstrcat(szThumbnailName, szTemp);

	EnterCriticalSection(&archive->ap[nPMTIndex]->csThumbnailList);
	for (nThumbnailIndex = 0; nThumbnailIndex < MAX_THUMBNAILS; nThumbnailIndex++)
	{
		if (archive->ap[nPMTIndex]->tl[nThumbnailIndex] == NULL)
			continue;
		if (archive->ap[nPMTIndex]->tl[nThumbnailIndex]->fActive == FALSE)
		{
			memcpy(&archive->ap[nPMTIndex]->tl[nThumbnailIndex]->st, &st, sizeof(SYSTEMTIME));
			lstrcpy(archive->ap[nPMTIndex]->tl[nThumbnailIndex]->szFilename, szTemp);
			archive->ap[nPMTIndex]->tl[nThumbnailIndex]->fActive = TRUE;
			archive->ap[nPMTIndex]->tl[nThumbnailIndex]->lnByteOffset.QuadPart = archive->ap[nPMTIndex]->lnCurrentFileWritePtr.QuadPart;
			break;
		}
	}
	LeaveCriticalSection(&archive->ap[nPMTIndex]->csThumbnailList);
	if (nThumbnailIndex == MAX_THUMBNAILS)
		OutputDebugString("No space left for thumbnails\n");

	return TRUE;
}

void LoadArchiveXML(HWND hDlg)
{
	BOOL fOneOrMoreXMLBad = FALSE;
	HANDLE hFind;
	HWND hWndArchiveList = GetDlgItem(hDlg, IDC_ARCHIVE_VIEW_LIST);
	WIN32_FIND_DATA fd;
	char szSearchName[MAX_PATH];

	GetDlgItemText(hDlg, IDC_ARCHIVE_VIEW_BASE_FOLDER, v->szBaseArchiveFolder, sizeof(v->szBaseArchiveFolder));
	if (v->szBaseArchiveFolder[lstrlen(v->szBaseArchiveFolder) - 1] != '\\')
	{
		lstrcat(v->szBaseArchiveFolder, "\\");
		SetDlgItemText(hDlg, IDC_ARCHIVE_VIEW_BASE_FOLDER, v->szBaseArchiveFolder);
	}
	lstrcpy(szSearchName, v->szBaseArchiveFolder);
	lstrcat(szSearchName, "*.xml");

	SendMessage(hWndArchiveList, WM_SETREDRAW, FALSE, 0);
	if (arcprogs != NULL)
	{
		ListView_DeleteAllItems(hWndArchiveList);
		LocalFree(arcprogs);
	}

	hFind = FindFirstFile(szSearchName, &fd);
	if (hFind == INVALID_HANDLE_VALUE)
	{
		SetDlgItemText(hDlg, IDC_VIEW_ARCHIVE_LOAD_PROGRESS, "");
		MessageBox(hDlg, "Location doesn't contain any archive XML files", gszAppName, MB_ICONSTOP);
		return;
	}

	// Allocate initial buffers
	v->nARCHIVEDPROGRAMSMax = 512;
	v->nARCHIVEDPROGRAMSCount = 0;
	arcprogs = LocalAlloc(LPTR, sizeof(ARCHIVEDPROGRAMS) * v->nARCHIVEDPROGRAMSMax);

	do
	{
		HANDLE hInputXMLFile;
		char szTargetXMLFile[MAX_PATH];

		lstrcpy(szTargetXMLFile, v->szBaseArchiveFolder);
		lstrcat(szTargetXMLFile, fd.cFileName);
		hInputXMLFile = CreateFile(szTargetXMLFile, GENERIC_READ, FILE_SHARE_READ, (LPSECURITY_ATTRIBUTES)NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, (HANDLE)NULL);
		if (hInputXMLFile != INVALID_HANDLE_VALUE)
		{
			if (ParseArchiveXML(hInputXMLFile, &arcprogs[v->nARCHIVEDPROGRAMSCount]) == TRUE)
			{
				char * szExtension;
				LV_ITEM lvi;

				memset(&lvi, 0, sizeof(lvi));
				lvi.state = 0; 
				lvi.stateMask = 0; 
				lvi.pszText = LPSTR_TEXTCALLBACK;   // app. maintains text 
				lvi.mask = LVIF_TEXT | LVIF_PARAM | LVIF_STATE; 
				lvi.iItem = v->nARCHIVEDPROGRAMSCount; 
				lvi.iSubItem = 0; 
				lvi.lParam = (LPARAM) v->nARCHIVEDPROGRAMSCount;    // item data 
				ListView_InsertItem(hWndArchiveList, &lvi);
				lstrcpy(arcprogs[v->nARCHIVEDPROGRAMSCount].szMPEGFile, szTargetXMLFile);
				
				szExtension = strstr(arcprogs[v->nARCHIVEDPROGRAMSCount].szMPEGFile, ".xml");
				lstrcpy(szExtension, ".mpg");

				v->nARCHIVEDPROGRAMSCount++;
				if (v->nARCHIVEDPROGRAMSCount == v->nARCHIVEDPROGRAMSMax)
				{
					PARCHIVEDPROGRAMS new_arcprogs;

					v->nARCHIVEDPROGRAMSMax += 512;
					new_arcprogs = LocalAlloc(LPTR, sizeof(ARCHIVEDPROGRAMS) * v->nARCHIVEDPROGRAMSMax);
					memcpy(new_arcprogs, arcprogs, sizeof(ARCHIVEDPROGRAMS) * v->nARCHIVEDPROGRAMSCount);
					LocalFree(arcprogs);
					arcprogs = new_arcprogs;
				}
				SetDlgItemInt(hDlg, IDC_VIEW_ARCHIVE_LOAD_PROGRESS, v->nARCHIVEDPROGRAMSCount, FALSE);
			}
			else
				fOneOrMoreXMLBad = TRUE;

			CloseHandle(hInputXMLFile);
		}
	} while (FindNextFile(hFind, &fd) == TRUE);
	FindClose(hFind);
	SendMessage(hWndArchiveList, WM_SETREDRAW, TRUE, 0);
	SetDlgItemText(hDlg, IDC_VIEW_ARCHIVE_LOAD_PROGRESS, "");
	if (fOneOrMoreXMLBad)
		MessageBox(hDlg, "One or more of the XML files didn't contain the required tags or was otherwise unprocessable", gszAppName, MB_ICONWARNING);
}

void GetArchiveFileListDispInfo(LV_DISPINFO *pnmv) 
{ 
    // Provide the item or subitem's text, if requested. 
    if (pnmv->item.mask & LVIF_TEXT)
	{ 
        int nArchiveItemIndex = (int)(pnmv->item.lParam);
		switch(pnmv->item.iSubItem)
		{
		case 0:
			lstrcpy(pnmv->item.pszText, arcprogs[nArchiveItemIndex].szChannel);
			break;
		case 1:
			if (!v->fShowArchiveLocalTime)
				wsprintf(pnmv->item.pszText, "%s %s", arcprogs[nArchiveItemIndex].szStartDate, arcprogs[nArchiveItemIndex].szStartTime);
			else
			{
				SYSTEMTIME stUTC, stLocal;
				char szTemp[128];

				lstrcpy(szTemp, arcprogs[nArchiveItemIndex].szStartDate);
				szTemp[4] = '\0';
				sscanf(szTemp, "%d", (int *)&stUTC.wYear);
				szTemp[7] = '\0';
				sscanf(&szTemp[5], "%d", (int *)&stUTC.wMonth);
				sscanf(&szTemp[8], "%d", (int *)&stUTC.wDay);
				
				lstrcpy(szTemp, arcprogs[nArchiveItemIndex].szStartTime);
				szTemp[2] = '\0';
				sscanf(szTemp, "%d", (int *)&stUTC.wHour);
				szTemp[5] = '\0';
				sscanf(&szTemp[3], "%d", (int *)&stUTC.wMinute);
				sscanf(&szTemp[6], "%d", (int *)&stUTC.wSecond);
				SystemTimeToTzSpecificLocalTime(NULL, &stUTC, &stLocal);

				wsprintf(pnmv->item.pszText, "%04d/%02d/%02d %02d:%02d:%02d",
					     stLocal.wYear, stLocal.wMonth, stLocal.wDay,
						 stLocal.wHour, stLocal.wMinute, stLocal.wSecond);
	
			}
			break;
		case 2:
			lstrcpy(pnmv->item.pszText, arcprogs[nArchiveItemIndex].szDuration);
			break;
		case 3:
			lstrcpy(pnmv->item.pszText, arcprogs[nArchiveItemIndex].szProgram);
			break;
		}
	}
}

DWORD WINAPI ShowArchiveThumbnails(LPVOID lpv)
{
	HWND hDlg = (HWND)lpv;
	int nThumbnailIndex;
	int nMaximumThumbnails;

	v->fArchiveViewThumbnailThreadRunning = TRUE;

	for (nMaximumThumbnails = 0; nMaximumThumbnails < 512; nMaximumThumbnails++)
	{
		if (lstrlen(arcprogs[v->nSelectedArchiveProgram].at[nMaximumThumbnails].szFilename) == 0)
			break;
	}

	nThumbnailIndex = 0;
	do
	{
		DWORD dwMaxThumbnailWidth = 0;
		DWORD dwMaxThumbnailHeight = 0;
		int nCurrentThumbnailOffset;
		int nSleep;
		HDC hRealDC = GetDC(hDlg);
		HDC hDC = CreateCompatibleDC(hRealDC);
		HBITMAP memBM = CreateCompatibleBitmap(hRealDC, 600, 400);
		SelectObject(hDC, memBM);

		for (nCurrentThumbnailOffset = 0; nCurrentThumbnailOffset < 5 && !v->fTerminateArchiveViewThumbnailThread; nCurrentThumbnailOffset++)
		{
			HISSRC hSourceObject;
			HGLOBAL hThumbnail;
			DWORD dwThumbnailWidth, dwThumbnailHeight;
			char szJPGFile[MAX_PATH];
			
			lstrcpy(szJPGFile, v->szBaseArchiveFolder);
			lstrcat(szJPGFile, "jpg\\");
			lstrcat(szJPGFile, arcprogs[v->nSelectedArchiveProgram].at[nThumbnailIndex + nCurrentThumbnailOffset].szFilename);

			hSourceObject = _ISOpenFileSource(szJPGFile);
			if (hSourceObject != NULL)
			{
				BYTE * hDisplayThumbnail;
				DWORD dwHalfWidth;
				DWORD dwHalfHeight;
				char szThumbnailNumber[16];

				hThumbnail = _ISReadJPGToRGB(hSourceObject, &dwThumbnailWidth, &dwThumbnailHeight);
				_ISCloseSource(hSourceObject);
				dwHalfWidth = dwThumbnailWidth / 2;
				dwHalfHeight = dwThumbnailHeight / 2;
				hDisplayThumbnail = LocalAlloc(LPTR, (dwHalfWidth * dwHalfHeight) * 3);
				if (hDisplayThumbnail != NULL)
				{
					_ISDecimateRGB(hThumbnail, dwThumbnailWidth, dwThumbnailHeight,
								   hDisplayThumbnail, dwHalfWidth, dwHalfHeight);

					wsprintf(szThumbnailNumber, "%d", nThumbnailIndex + nCurrentThumbnailOffset + 1);
					_ISDrawTextOnRGB2(hDisplayThumbnail,
									  dwHalfWidth, dwHalfHeight,
									  szThumbnailNumber,
									  &v->logfontChannelFont,
									  1, 1,
									  RGB(0x00, 0xff, 0x00));				
					_ISDrawRGB(hDC,
							   hDisplayThumbnail,
							   dwHalfWidth, dwHalfHeight,
							   nCurrentThumbnailOffset * dwHalfHeight, 0,
							   dwHalfWidth, dwHalfHeight,
							   NULL);
					LocalFree(hDisplayThumbnail);
				}
				GlobalFree(hThumbnail);
				if (dwThumbnailWidth > dwMaxThumbnailWidth)
					dwMaxThumbnailWidth = dwThumbnailWidth;
				if (dwThumbnailHeight > dwMaxThumbnailHeight)
					dwMaxThumbnailHeight = dwThumbnailHeight;
			}					
		}
		BitBlt(hRealDC, 5, 280, (dwMaxThumbnailWidth / 2) * 5, (dwMaxThumbnailHeight / 2), hDC, 0, 0, SRCCOPY);
		DeleteObject(memBM);
		DeleteDC(hDC);
		ReleaseDC(hDlg, hRealDC);
		for (nSleep = 0; nSleep < 25 && !v->fTerminateArchiveViewThumbnailThread; nSleep++)
			Sleep(10);
		nThumbnailIndex++;
		if (nThumbnailIndex >= nMaximumThumbnails - 5)
			nThumbnailIndex = 0;
	} while (!v->fTerminateArchiveViewThumbnailThread);
	v->fArchiveViewThumbnailThreadRunning = FALSE;
	return 0;
}

void TerminateThumbnailPreviewThread()
{
	if (v->fArchiveViewThumbnailThreadRunning == TRUE)
	{
		v->fTerminateArchiveViewThumbnailThread = TRUE;
		while (v->fArchiveViewThumbnailThreadRunning == TRUE)
			Sleep(10);
		v->fTerminateArchiveViewThumbnailThread = FALSE;
	}
}

BOOL DeletedArchiveItem()
{
	if (lstrcmp(arcprogs[v->nSelectedArchiveProgram].szChannel, "---") == 0)
		return TRUE;
	return FALSE;
}

DWORD WINAPI LoadArchiveXMLThread(LPVOID pvarg)
{

	HWND hDlg = (HWND)pvarg;

	CursorWait(hDlg);
	LoadArchiveXML(hDlg);
	CursorNormal();
	EnableWindow(GetDlgItem(hDlg, IDC_ARCHIVE_VIEW_LOAD), TRUE);

	return 0;
}

INT_PTR CALLBACK ViewArchiveDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg)
	{
	case WM_INITDIALOG:
		{
			int nColumnPosition = 0;
			HICON hIcon;
			HWND hWndArchiveList = GetDlgItem(hDlg, IDC_ARCHIVE_VIEW_LIST);
			LV_COLUMN lvc;
			char szTemp[128];
			
			arcprogs = NULL;
			v->nSelectedArchiveProgram = -1;
			SetDlgItemText(hDlg, IDC_ARCHIVE_VIEW_BASE_FOLDER, v->szBaseArchiveFolder);
			CheckDlgButton(hDlg, IDC_VIEW_ARCHIVE_SHOW_LOCAL_TIME, v->fShowArchiveLocalTime);
			SetDlgItemText(hDlg, IDC_VIEW_ARCHIVE_LOAD_PROGRESS, "");
			SetDlgItemText(hDlg, IDC_VIEW_ARCHIVE_DESCRIPTION, "");

			// Setup list view columns. 
			lvc.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM; 
			lvc.pszText = szTemp; 

			lvc.fmt = LVCFMT_CENTER; 
			lvc.cx = 70; 
			lstrcpy(szTemp, TEXT("Channel"));
			ListView_InsertColumn(hWndArchiveList, nColumnPosition++, &lvc); 

			lvc.fmt = LVCFMT_CENTER; 
			lvc.cx = 120; 
			lstrcpy(szTemp, TEXT("Date/Time"));
			ListView_InsertColumn(hWndArchiveList, nColumnPosition++, &lvc); 

			lvc.fmt = LVCFMT_CENTER; 
			lvc.cx = 60; 
			lstrcpy(szTemp, TEXT("Duration"));
			ListView_InsertColumn(hWndArchiveList, nColumnPosition++, &lvc); 

			lvc.fmt = LVCFMT_LEFT; 
			lvc.cx = 180; 
			lstrcpy(szTemp, TEXT("Program"));
			ListView_InsertColumn(hWndArchiveList, nColumnPosition++, &lvc); 

			ListView_SetExtendedListViewStyle(hWndArchiveList, LVS_EX_FULLROWSELECT);

			SetFocus(GetDlgItem(hDlg, IDC_ARCHIVE_VIEW_BASE_FOLDER));

			hIcon = LoadImage(v->hInstance, MAKEINTRESOURCE(IDI_PLAY), IMAGE_ICON, 16, 16, 0);
			SendDlgItemMessage(hDlg, IDC_VIEW_ARCHIVE_PLAY, BM_SETIMAGE, IMAGE_ICON, (LPARAM)hIcon);
			hIcon = LoadImage(v->hInstance, MAKEINTRESOURCE(IDI_DELETE), IMAGE_ICON, 16, 16, 0);
			SendDlgItemMessage(hDlg, IDC_VIEW_ARCHIVE_DELETE, BM_SETIMAGE, IMAGE_ICON, (LPARAM)hIcon);

		}
		break;
	case WM_DESTROY:
		TerminateThumbnailPreviewThread();
		if (arcprogs != NULL)
		{
			ListView_DeleteAllItems(GetDlgItem(hDlg, IDC_ARCHIVE_VIEW_LIST));
			LocalFree(arcprogs);
		}
	case WM_NOTIFY:
		if (lParam == (LPARAM)NULL)
			break;
		switch (((LPNMHDR) lParam)->code)
		{ 
		case LVN_GETDISPINFO:
			{
				NMLVDISPINFO * pnmv = (NMLVDISPINFO *)lParam;
				GetArchiveFileListDispInfo((LV_DISPINFO *) lParam);
			}
			break;
		case LVN_ITEMCHANGED:
			{
				LPNMLISTVIEW pnmv = (LPNMLISTVIEW)lParam;
				if (pnmv->uNewState & LVIS_SELECTED)
				{
					DWORD dwThreadID;
					HANDLE hThread;

					TerminateThumbnailPreviewThread();
					v->nSelectedArchiveProgram = (int)pnmv->lParam;
					InvalidateRect(hDlg, NULL, TRUE);
					if (DeletedArchiveItem() == TRUE)
					{
						SetDlgItemText(hDlg, IDC_VIEW_ARCHIVE_DESCRIPTION, "");
						break;
					}
					SetDlgItemText(hDlg, IDC_VIEW_ARCHIVE_DESCRIPTION, arcprogs[v->nSelectedArchiveProgram].szDescription);
					hThread = CreateThread(NULL, 0, ShowArchiveThumbnails, (LPVOID)hDlg, 0, &dwThreadID);
					CloseHandle(hThread);
				}
			}
			break;
		case NM_DBLCLK:
		case NM_RETURN:
			PostMessage(hDlg, WM_COMMAND, IDC_VIEW_ARCHIVE_PLAY, 0);
			break;
		case NM_KEYDOWN:
			{
				LPNMKEY lpnmk = (LPNMKEY)lParam;
				switch(lpnmk->nVKey)
				{
				case VK_DELETE:
					SendMessage(hDlg, WM_COMMAND, IDC_VIEW_ARCHIVE_DELETE, 0);
					break;
				}
			}
			break;
		}
		break;
	case WM_CLOSE:
		if (IsWindowEnabled(GetDlgItem(hDlg, IDC_ARCHIVE_VIEW_LOAD)) == TRUE)
			EndDialog(hDlg, FALSE);
		else
			MessageBeep(0);
		break;
	case WM_ACTIVATE:
		SetFocus(GetDlgItem(hDlg, IDC_ARCHIVE_VIEW_LIST));
		break;
	case WM_COMMAND:
		switch(LOWORD(wParam))
		{
		case IDC_VIEW_ARCHIVE_BROWSE:
			{
				BROWSEINFO BrowsingInfo;
				char DirPath[MAX_PATH];
				char FolderName[MAX_PATH];
				LPITEMIDLIST ItemID;

				GetDlgItemText(hDlg, IDC_ARCHIVE_VIEW_BASE_FOLDER, FolderName, sizeof(FolderName));

				memset(&BrowsingInfo, 0, sizeof(BROWSEINFO));
				memset(DirPath, 0, MAX_PATH);

				BrowsingInfo.hwndOwner      = hDlg;
				BrowsingInfo.pszDisplayName = FolderName;
				BrowsingInfo.lpszTitle      = "Select input folder for archived files";
				BrowsingInfo.ulFlags = BIF_NEWDIALOGSTYLE;

				ItemID = SHBrowseForFolder(&BrowsingInfo);
				if (ItemID)
				{
					SHGetPathFromIDList(ItemID, DirPath);
					SetDlgItemText(hDlg, IDC_ARCHIVE_VIEW_BASE_FOLDER, DirPath);
				}
			}
			break;
		case IDCANCEL:
			PostMessage(hDlg, WM_CLOSE, 0, 0);
			break;
		case IDC_ARCHIVE_VIEW_LOAD:
			{
				DWORD dwThreadID;
				HANDLE hThread;

				TerminateThumbnailPreviewThread();
				SetDlgItemText(hDlg, IDC_VIEW_ARCHIVE_LOAD_PROGRESS, "Wait");
				EnableWindow(GetDlgItem(hDlg, IDC_ARCHIVE_VIEW_LOAD), FALSE);

				hThread = CreateThread(NULL, 0, LoadArchiveXMLThread, (LPVOID)hDlg, 0, &dwThreadID);
				CloseHandle(hThread);
			}
			break;
		case IDC_VIEW_ARCHIVE_SHOW_LOCAL_TIME:
			v->fShowArchiveLocalTime = IsDlgButtonChecked(hDlg, IDC_VIEW_ARCHIVE_SHOW_LOCAL_TIME);
			ListView_RedrawItems(GetDlgItem(hDlg, IDC_ARCHIVE_VIEW_LIST), 0, ListView_GetItemCount(GetDlgItem(hDlg, IDC_ARCHIVE_VIEW_LIST)));
			break;
		case IDC_VIEW_ARCHIVE_PLAY:
			if (v->nSelectedArchiveProgram != -1)
			{
				DWORD dwThreadID;
				HANDLE hThread;

				if (DeletedArchiveItem() == TRUE)
					break;

				TerminateThumbnailPreviewThread();
				lstrcpy(v->szArchiveVLCCommand, v->szVLCExeLocation);
				lstrcat(v->szArchiveVLCCommand, " \"");
				lstrcat(v->szArchiveVLCCommand, arcprogs[v->nSelectedArchiveProgram].szMPEGFile);
				lstrcat(v->szArchiveVLCCommand, "\"");

				hThread = CreateThread(NULL, 0, LaunchVLCThread, (LPVOID)v->szArchiveVLCCommand, 0, &dwThreadID);
				CloseHandle(hThread);
			}
			else
				MessageBeep(0);
			break;
		case IDC_ARCHIVE_VIEW_COPY_FILENAME:
			if (v->nSelectedArchiveProgram != -1)
			{
				if (!OpenClipboard(hDlg))
				{
					MessageBeep(0);
					break;
				}
				{
					char * szBuffer = GlobalAlloc(GMEM_DDESHARE, lstrlen(arcprogs[v->nSelectedArchiveProgram].szMPEGFile) + 1);
					lstrcpy(szBuffer, arcprogs[v->nSelectedArchiveProgram].szMPEGFile);

					GlobalUnlock(szBuffer);  
					EmptyClipboard();
					if (SetClipboardData(CF_TEXT, szBuffer) == NULL)
						OutputDebugString("Archive: copy to clipboard failed\n");
					CloseClipboard();
					GlobalFree(szBuffer);
				}
			}
			break;
		case IDC_VIEW_ARCHIVE_DELETE:
			if (v->nSelectedArchiveProgram != -1)
			{
				char szTemp[128];

				if (DeletedArchiveItem() == FALSE)
				{				
					wsprintf(szTemp, "Are you sure you want to delete '%s - %s %s'?",
							 arcprogs[v->nSelectedArchiveProgram].szProgram,
							 arcprogs[v->nSelectedArchiveProgram].szStartDate, arcprogs[v->nSelectedArchiveProgram].szStartTime);
					if (MessageBox(hDlg, szTemp, gszAppName, MB_YESNO | MB_DEFBUTTON2 | MB_ICONQUESTION) == IDYES)
					{
						int nThumbnailIndex;
						char * szExtension;
						HWND hWndListView = GetDlgItem(hDlg, IDC_ARCHIVE_VIEW_LIST);
						char szDeleteFile[MAX_PATH];
						
						TerminateThumbnailPreviewThread();
						CursorWait(hDlg);
						DeleteFile(arcprogs[v->nSelectedArchiveProgram].szMPEGFile);
						lstrcpy(szDeleteFile, arcprogs[v->nSelectedArchiveProgram].szMPEGFile);
						szExtension = strstr(szDeleteFile, ".mpg");
						lstrcpy(szExtension, ".xml");
						DeleteFile(szDeleteFile);
						lstrcpy(szExtension, ".user-data");
						DeleteFile(szDeleteFile);
						for (nThumbnailIndex = 0; nThumbnailIndex < 512; nThumbnailIndex++)
						{
							if (lstrlen(arcprogs[v->nSelectedArchiveProgram].at[nThumbnailIndex].szFilename) == 0)
								break;
							lstrcpy(szDeleteFile, v->szBaseArchiveFolder);
							lstrcat(szDeleteFile, "jpg\\");
							lstrcat(szDeleteFile, arcprogs[v->nSelectedArchiveProgram].at[nThumbnailIndex].szFilename);
							DeleteFile(szDeleteFile);
						}
						lstrcpy(arcprogs[v->nSelectedArchiveProgram].szChannel, "---");
						ListView_RedrawItems(hWndListView, v->nSelectedArchiveProgram, v->nSelectedArchiveProgram);
						{
							LVITEM lvi;
							lvi.mask = LVIF_STATE;
							lvi.iItem = v->nSelectedArchiveProgram + 1;
							lvi.iSubItem = 0;
							lvi.stateMask = LVIS_SELECTED;
							lvi.state = LVIS_SELECTED;
							ListView_SetItem(hWndListView, &lvi);
							ListView_EnsureVisible(hWndListView, lvi.iItem, FALSE);
						}
						CursorNormal();
					}
				}
			}
			else
				MessageBeep(0);
			SetFocus(GetDlgItem(hDlg, IDC_ARCHIVE_VIEW_LIST));
			break;

		}
		break;
	}

	return FALSE;
}

void ViewArchivedFiles(HWND hWnd)
{
	DialogBox(v->hInstance, MAKEINTRESOURCE(IDD_ARCHIVE_VIEW), hWnd, ViewArchiveDlgProc);
}

INT_PTR CALLBACK SaveEPGDataDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg)
	{
	case WM_INITDIALOG:
		SetDlgItemText(hDlg, IDC_SAVE_EPG_FOLDER, v->szEPGSaveFolder);
		SetFocus(GetDlgItem(hDlg, IDC_SAVE_EPG_FOLDER));
		break;
	case WM_CLOSE:
		EndDialog(hDlg, FALSE);
		break;
	case WM_COMMAND:
		switch(LOWORD(wParam))
		{
		case IDOK:
			GetDlgItemText(hDlg, IDC_SAVE_EPG_FOLDER, v->szEPGSaveFolder, sizeof(v->szEPGSaveFolder));
			v->hEPGSaveHandle = INVALID_HANDLE_VALUE;
			memset(&v->stEPGSaveCurrentDate, 0, sizeof(v->stEPGSaveCurrentDate));
			v->fEPGSaveFirstTime = TRUE;
			v->fEPGSaveEnabled = TRUE;
			EndDialog(hDlg, TRUE);
			break;
		case IDCANCEL:
			EndDialog(hDlg, FALSE);
			break;
		case IDC_SAVE_ALL_EPG_BROWSE:
			break;
		}
		break;
	}

	return FALSE;
}
