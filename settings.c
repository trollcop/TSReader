#include <windows.h>
#include <commctrl.h>
#include "TSReader.h"
#include "resource.h"

extern PVARIABLES v;
extern char gszAppName[];

HWND hWndSettings[32];
int nMaxSettingsPages;
HFONT hTitleFont;

// From TSReader.c
void BrowseIPSaveFolder(HWND hDlg);
BOOL myChooseColor(HWND hDlg, DWORD * dwColor);
void ProcessPIDColorButtonClicks(HWND hDlg, WPARAM wParam);
void PaintPIDChartColorBoxes(HWND hDlg, int nTopOffset, int nLeftOffset);
void SetupEPGGridScrollbars(HWND hDlg);
void ProcessEPGScrollbarMessages(HWND hDlg, WPARAM wParam, LPARAM lParam);
void InitSerialControlDialog(HWND hDlg);
void UpdateFilenameFormatPreview(HWND hDlg);
int PopulateSourceList(HWND hDlg, HWND hWndLV);
void GetSourceDispInfo(LV_DISPINFO *pnmv);
void UpdateVLCConfigDisplay(HWND hDlg, int nOffset);
void VLCSettings_WM_COMMAND(HWND hDlg, WPARAM wParam, LPARAM lParam, BOOL fFromProfileEditor);

HTREEITEM SetupTreeView(HWND hDlg, char * lpszItem, LPARAM lParam, int nDialogResource, DLGPROC dlgproc)
{
	HTREEITEM hItem;
	TV_INSERTSTRUCT tvins; 
	LPTV_ITEM tvi = &tvins.item; 

	tvi->mask = TVIF_TEXT | TVIF_PARAM ; 
 
    // Set the text of the item. 
    tvi->pszText = lpszItem; 
    tvi->cchTextMax = lstrlen(lpszItem); 
  
    // Save the heading level in the item's application-defined 
    // data area. 
    tvi->lParam = (LPARAM)lParam; 
    tvi->cChildren = 0;
	tvins.hInsertAfter = NULL; 

    // Set the parent item based on the specified level. 
	tvins.hParent = TVI_ROOT;
	
    // Add the item to the tree-view control. 
    hItem = (HTREEITEM) SendMessage(GetDlgItem(hDlg, IDC_SETTINGS_TREE), TVM_INSERTITEM, 0, (LPARAM)(LPTV_INSERTSTRUCT)&tvins);  
    	
	// Create the dialog box for this item
	hWndSettings[lParam] = CreateDialog(v->hInstance, MAKEINTRESOURCE(nDialogResource), hDlg, dlgproc);
	SendDlgItemMessage(hWndSettings[lParam], IDC_TITLE, WM_SETFONT, (WPARAM)hTitleFont, MAKELONG(TRUE, 0));
	SetWindowPos(hWndSettings[lParam], NULL, 175, 10, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
	nMaxSettingsPages++;

	return hItem;
}

INT_PTR CALLBACK Settings_Threads(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg)
	{
	case WM_INITDIALOG:
		switch(v->nProcessPriority)
		{
		case REALTIME_PRIORITY_CLASS:
			CheckDlgButton(hDlg, IDC_PROCESS_REALTIME, BST_CHECKED);
			break;
		case HIGH_PRIORITY_CLASS:
			CheckDlgButton(hDlg, IDC_PROCESS_HIGH, BST_CHECKED);
			break;
		case ABOVE_NORMAL_PRIORITY_CLASS:
			CheckDlgButton(hDlg, IDC_PROCESS_ABOVE_NORMAL, BST_CHECKED);
			break;
		case NORMAL_PRIORITY_CLASS:
			CheckDlgButton(hDlg, IDC_PROCESS_NORMAL, BST_CHECKED);
			break;
		case BELOW_NORMAL_PRIORITY_CLASS:
			CheckDlgButton(hDlg, IDC_PROCESS_BELOW_NORMAL, BST_CHECKED);
			break;
		case IDLE_PRIORITY_CLASS:
			CheckDlgButton(hDlg, IDC_PROCESS_LOW, BST_CHECKED);
			break;
		}
		switch(v->ss.nInputThreadPriority)
		{
		case 0:
			CheckDlgButton(hDlg, IDC_DATA_INPUT_NORMAL, BST_CHECKED);
			break;
		case 1:
			CheckDlgButton(hDlg, IDC_DATA_INPUT_HIGH, BST_CHECKED);
			break;
		case 2:
			CheckDlgButton(hDlg, IDC_DATA_INPUT_LOW, BST_CHECKED);
			break;
		case 3:
			CheckDlgButton(hDlg, IDC_DATA_INPUT_CRITICAL, BST_CHECKED);
			break;
		}
		switch(v->nStreamProcessingThreadPriority)
		{
		case 0:
			CheckDlgButton(hDlg, IDC_STREAM_PROCESSING_NORMAL, BST_CHECKED);
			break;
		case 1:
			CheckDlgButton(hDlg, IDC_STREAM_PROCESSING_HIGH, BST_CHECKED);
			break;
		case 2:
			CheckDlgButton(hDlg, IDC_STREAM_PROCESSING_LOW, BST_CHECKED);
			break;
		}
		switch(v->nThumbnailProcessingThreadPriority)
		{
		case 0:
			CheckDlgButton(hDlg, IDC_THUMBNAIL_PROCESSING_NORMAL, BST_CHECKED);
			break;
		case 1:
			CheckDlgButton(hDlg, IDC_THUMBNAIL_PROCESSING_HIGH, BST_CHECKED);
			break;
		case 2:
			CheckDlgButton(hDlg, IDC_THUMBNAIL_PROCESSING_LOW, BST_CHECKED);
			break;
		case 3:
			CheckDlgButton(hDlg, IDC_THUMBNAIL_PROCESSING_DISABLED, BST_CHECKED);
			break;		
		}
		break;
	case WM_COMMAND:
		switch(LOWORD(wParam))
		{
		case IDOK:
			if (IsDlgButtonChecked(hDlg, IDC_PROCESS_REALTIME))					v->nProcessPriority = REALTIME_PRIORITY_CLASS;
			if (IsDlgButtonChecked(hDlg, IDC_PROCESS_HIGH))						v->nProcessPriority = HIGH_PRIORITY_CLASS;
			if (IsDlgButtonChecked(hDlg, IDC_PROCESS_ABOVE_NORMAL))				v->nProcessPriority = ABOVE_NORMAL_PRIORITY_CLASS;
			if (IsDlgButtonChecked(hDlg, IDC_PROCESS_NORMAL))					v->nProcessPriority = NORMAL_PRIORITY_CLASS;
			if (IsDlgButtonChecked(hDlg, IDC_PROCESS_BELOW_NORMAL))				v->nProcessPriority = BELOW_NORMAL_PRIORITY_CLASS;
			if (IsDlgButtonChecked(hDlg, IDC_PROCESS_LOW))						v->nProcessPriority = IDLE_PRIORITY_CLASS;

			if (IsDlgButtonChecked(hDlg, IDC_DATA_INPUT_NORMAL))				v->ss.nInputThreadPriority = 0;
			if (IsDlgButtonChecked(hDlg, IDC_DATA_INPUT_HIGH))					v->ss.nInputThreadPriority = 1;
			if (IsDlgButtonChecked(hDlg, IDC_DATA_INPUT_LOW))					v->ss.nInputThreadPriority = 2;
			if (IsDlgButtonChecked(hDlg, IDC_DATA_INPUT_CRITICAL))				v->ss.nInputThreadPriority = 3;

			if (IsDlgButtonChecked(hDlg, IDC_STREAM_PROCESSING_NORMAL))			v->nStreamProcessingThreadPriority = 0;
			if (IsDlgButtonChecked(hDlg, IDC_STREAM_PROCESSING_HIGH))			v->nStreamProcessingThreadPriority = 1;
			if (IsDlgButtonChecked(hDlg, IDC_STREAM_PROCESSING_LOW))			v->nStreamProcessingThreadPriority = 2;

			if (IsDlgButtonChecked(hDlg, IDC_THUMBNAIL_PROCESSING_NORMAL))		v->nThumbnailProcessingThreadPriority = 0;
			if (IsDlgButtonChecked(hDlg, IDC_THUMBNAIL_PROCESSING_HIGH))		v->nThumbnailProcessingThreadPriority = 1;
			if (IsDlgButtonChecked(hDlg, IDC_THUMBNAIL_PROCESSING_LOW))			v->nThumbnailProcessingThreadPriority = 2;
			if (IsDlgButtonChecked(hDlg, IDC_THUMBNAIL_PROCESSING_DISABLED))	v->nThumbnailProcessingThreadPriority = 3;
			break;
		}
		break;
	}
	return FALSE;
}

INT_PTR CALLBACK Settings_Thumbnails(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg)
	{
	case WM_INITDIALOG:
		CheckDlgButton(hDlg, IDC_THUMBNAIL_ANIMATED, v->fThumbnailThreadAnimated);
		CheckDlgButton(hDlg, IDC_THUMBNAIL_FULL_THUMBNAILS, v->fFullThumbnails);
		CheckDlgButton(hDlg, IDC_THUMBNAIL_AUDIO, v->fAudioThumbnails);
		CheckDlgButton(hDlg, IDC_THUMBNAIL_FULL_SIZE_SAVED, v->fSavedThumbnailsFullSize);
		CheckDlgButton(hDlg, IDC_THUMBNAIL_HIDE_ICONS, v->fHideThumbnailIcons);
		CheckDlgButton(hDlg, IDC_THUMBNAIL_LARGE, v->nThumbnailSize);
		CheckDlgButton(hDlg, IDC_THUMBNAIL_SAVE_ALL_SAME_NAME, v->fSaveAllThumbnailsSameName);
		CheckDlgButton(hDlg, IDC_THUMBNAIL_SHOW_SCRAMBLED, v->fShowScrambledChannels);
		CheckDlgButton(hDlg, IDC_THUMBNAIL_WAIT_CA, v->fWaitForCAThumbnail);
		SetDlgItemInt(hDlg, IDC_THUMBNAIL_REFRESH_RATE, v->nESParsingCounterReload, FALSE);
		break;
	case WM_COMMAND:
		switch(LOWORD(wParam))
		{
		case IDOK:
			v->fThumbnailThreadAnimated = IsDlgButtonChecked(hDlg, IDC_THUMBNAIL_ANIMATED);
			v->fFullThumbnails = IsDlgButtonChecked(hDlg, IDC_THUMBNAIL_FULL_THUMBNAILS);
			v->fAudioThumbnails = IsDlgButtonChecked(hDlg, IDC_THUMBNAIL_AUDIO);
			v->fSavedThumbnailsFullSize = IsDlgButtonChecked(hDlg, IDC_THUMBNAIL_FULL_SIZE_SAVED);
			v->fHideThumbnailIcons = IsDlgButtonChecked(hDlg, IDC_THUMBNAIL_HIDE_ICONS);
			v->nThumbnailSize = IsDlgButtonChecked(hDlg, IDC_THUMBNAIL_LARGE);
			v->fSaveAllThumbnailsSameName = IsDlgButtonChecked(hDlg, IDC_THUMBNAIL_SAVE_ALL_SAME_NAME);
			v->fShowScrambledChannels = IsDlgButtonChecked(hDlg, IDC_THUMBNAIL_SHOW_SCRAMBLED);
			v->fWaitForCAThumbnail = IsDlgButtonChecked(hDlg, IDC_THUMBNAIL_WAIT_CA);
			v->nESParsingCounterReload = GetDlgItemInt(hDlg, IDC_THUMBNAIL_REFRESH_RATE, NULL, FALSE);
			break;
		}
		break;
	}

	return FALSE;
}

INT_PTR CALLBACK Settings_EIT(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg)
	{
	case WM_INITDIALOG:
		CheckDlgButton(hDlg, IDC_EIT_KEEP_PAST_DATA, v->fKeepPastEITData);
		SetDlgItemInt(hDlg, IDC_EIT_SERVER_PORT, v->nEITServerPort, FALSE);
		CheckDlgButton(hDlg, IDC_EIT_SERVER_ENABLED, v->fEITServerEnabled);
		CheckDlgButton(hDlg, IDC_EPG_ONLY_EPG_CHANNELS, v->fShowEPGChannelsOnly);
		CheckDlgButton(hDlg, IDC_EPG_THIS_MUX_ONLY, v->fShowEPGThisMuxOnly);
		SetupEPGGridScrollbars(hDlg);
		break;
	case WM_COMMAND:
		switch(LOWORD(wParam))
		{
		case IDOK:
			v->fKeepPastEITData = IsDlgButtonChecked(hDlg, IDC_EIT_KEEP_PAST_DATA);
			v->nEITServerPort = GetDlgItemInt(hDlg, IDC_EIT_SERVER_PORT, NULL, FALSE);
			v->fEITServerEnabled = IsDlgButtonChecked(hDlg, IDC_EIT_SERVER_ENABLED);
			v->fShowEPGChannelsOnly = IsDlgButtonChecked(hDlg, IDC_EPG_ONLY_EPG_CHANNELS);
			v->fShowEPGThisMuxOnly = IsDlgButtonChecked(hDlg, IDC_EPG_THIS_MUX_ONLY);
			break;
		}
		break;
	case WM_HSCROLL:
		ProcessEPGScrollbarMessages(hDlg, wParam, lParam);
		break;
	}
	return FALSE;
}

INT_PTR CALLBACK Settings_IPDVB(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg)
	{
	case WM_INITDIALOG:
		CheckDlgButton(hDlg, IDC_IP_AUTO_EXPAND, v->fAutoExpandIPs);
		SetDlgItemText(hDlg, IDC_IP_SAVE_FOLDER, v->szIPSaveFolder);
		break;
	case WM_COMMAND:
		switch(LOWORD(wParam))
		{
		case IDOK:
			v->fAutoExpandIPs = IsDlgButtonChecked(hDlg, IDC_IP_AUTO_EXPAND);
			GetDlgItemText(hDlg, IDC_IP_SAVE_FOLDER, v->szIPSaveFolder, sizeof(v->szIPSaveFolder));
			break;
		case IDC_IP_SAVE_FOLDER_ELIPSES:
			BrowseIPSaveFolder(hDlg);
			break;
		}
		break;
	}
	return FALSE;
}

INT_PTR CALLBACK Settings_Parser(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg)
	{
	case WM_INITDIALOG:
		CheckDlgButton(hDlg, IDC_PARSER_COUNT_CONTINUITY, v->fCountContinuityErrors);
		CheckDlgButton(hDlg, IDC_PARSER_IGNORE_CRC, v->fIgnoreTableCRCErrors);
		CheckDlgButton(hDlg, IDC_PARSER_DCII_PMT_80, v->fIgnorePMT800x0ff6);
		CheckDlgButton(hDlg, IDC_PARSER_IGNORE_DVB_EIT_SDT, v->fIgnoreEIT);
		CheckDlgButton(hDlg, IDC_PARSER_IGNORE_PMT_65500, v->fIgnorePMT65500);
		CheckDlgButton(hDlg, IDC_PARSER_PLAIN_CA_DESCRIPTOR, v->fPlainCADescriptors);
		CheckDlgButton(hDlg, IDC_PARSER_SDT_CURRENT_MUX, v->fSDTOnlyForCurrentMux);
		CheckDlgButton(hDlg, IDC_PARSER_TIMESTAMP_PACKETS, v->ss.fTimestampPackets);
		CheckDlgButton(hDlg, IDC_AUTORESTART_PAT_CHANGE, v->fAutoRestartOnPATVersionChange);
		CheckDlgButton(hDlg, IDC_AUTORESTART_STOP, v->fAutoRestartOnDataStop);
		SetDlgItemInt(hDlg, IDC_AUTORESTART_STOP_DELAY, v->nAutoRestartOnDataStopDelay, FALSE);
		CheckDlgButton(hDlg, IDC_AUTORESTART_NO_TUNE_DIALOG, v->fAutoRestartNoTuneDialog);
		CheckDlgButton(hDlg, IDC_RESTART_NOBEEP, v->fAutoRestartNoBeep);
		SetDlgItemInt(hDlg, IDC_STREAMING_PIPE_SIZE, v->nStreamingPipeSize, FALSE);
		SetDlgItemInt(hDlg, IDC_THUMBNAIL_PIPE_SIZE, v->nThumbnailPipeSize, FALSE);
		break;
	case WM_COMMAND:
		switch(LOWORD(wParam))
		{
		case IDOK:
			v->fCountContinuityErrors = IsDlgButtonChecked(hDlg, IDC_PARSER_COUNT_CONTINUITY);
			v->fIgnoreTableCRCErrors = IsDlgButtonChecked(hDlg, IDC_PARSER_IGNORE_CRC);
			v->fIgnorePMT800x0ff6 = IsDlgButtonChecked(hDlg, IDC_PARSER_DCII_PMT_80);
			v->fIgnoreEIT = IsDlgButtonChecked(hDlg, IDC_PARSER_IGNORE_DVB_EIT_SDT);
			v->fIgnorePMT65500 = IsDlgButtonChecked(hDlg, IDC_PARSER_IGNORE_PMT_65500);
			v->fPlainCADescriptors = IsDlgButtonChecked(hDlg, IDC_PARSER_PLAIN_CA_DESCRIPTOR);
			v->fSDTOnlyForCurrentMux = IsDlgButtonChecked(hDlg, IDC_PARSER_SDT_CURRENT_MUX);
			v->ss.fTimestampPackets = IsDlgButtonChecked(hDlg, IDC_PARSER_TIMESTAMP_PACKETS);
			v->fAutoRestartOnPATVersionChange = IsDlgButtonChecked(hDlg, IDC_AUTORESTART_PAT_CHANGE);
			v->fAutoRestartOnDataStop = IsDlgButtonChecked(hDlg, IDC_AUTORESTART_STOP);
			v->nAutoRestartOnDataStopDelay = GetDlgItemInt(hDlg, IDC_AUTORESTART_STOP_DELAY, NULL, FALSE);
			v->fAutoRestartNoTuneDialog = IsDlgButtonChecked(hDlg, IDC_AUTORESTART_NO_TUNE_DIALOG);
			v->fAutoRestartNoBeep = IsDlgButtonChecked(hDlg, IDC_RESTART_NOBEEP);
			v->nStreamingPipeSize = GetDlgItemInt(hDlg, IDC_STREAMING_PIPE_SIZE, NULL, FALSE);
			v->nThumbnailPipeSize = GetDlgItemInt(hDlg, IDC_THUMBNAIL_PIPE_SIZE, NULL, FALSE);
			break;
		}
		break;
	}
	return FALSE;
}

INT_PTR CALLBACK Settings_Chart(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg)
	{
	case WM_INITDIALOG:
		{
			int nChartStyleIndex = 0;
			HWND hCombo = GetDlgItem(hDlg, IDC_CHART_STYLE);
			static char * szChartStyles[] = {
				"No Style",
				"Light Colors with Inset Border",
				"Light Colors with Shadow Border",
				"Light Colors with Line Border",
				"Light Colors with No Border",
				"Medium Colors with Inset Border",
				"Medium Colors with Shadow Border",
				"Medium Colors with Line Border",
				"Medium Colors with No Border",
				"Dark Colors with Inset Border",
				"Dark Colors with Shadow Border",
				"Dark Colors with Line Border",
				"Dark Colors with No Border",
				""};

			CheckDlgButton(hDlg, IDC_CHART_REALTIME, v->fRealtimeCharting);
			SetDlgItemInt(hDlg, IDC_CHART_REFRESH_RATE, v->nGraphRefreshRate, FALSE);
			while (szChartStyles[nChartStyleIndex][0] != '\0')
			{
				int nIndex = (int)SendMessage(hCombo, CB_ADDSTRING, 0, (LPARAM)szChartStyles[nChartStyleIndex]);
				if (nChartStyleIndex == v->nChartStyle)
					SendMessage(hCombo, CB_SETCURSEL, nIndex, 0);
				nChartStyleIndex++;
			}			
		}
		break;
	case WM_COMMAND:
		switch(LOWORD(wParam))
		{
		case IDOK:
			v->fRealtimeCharting = IsDlgButtonChecked(hDlg, IDC_CHART_REALTIME);
			v->nGraphRefreshRate = GetDlgItemInt(hDlg, IDC_CHART_REFRESH_RATE, NULL, FALSE);
			v->nChartStyle = (int)SendDlgItemMessage(hDlg, IDC_CHART_STYLE, CB_GETCURSEL, 0, 0);
			break;
		default:
			ProcessPIDColorButtonClicks(hDlg, wParam);
			break;
		}
		break;
	case WM_PAINT:
		PaintPIDChartColorBoxes(hDlg, 37, 180);
		break;
	}
	return FALSE;
}

INT_PTR CALLBACK Settings_UI(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg)
	{
	case WM_INITDIALOG:
		CheckDlgButton(hDlg, IDC_UI_AUTO_EXPAND_PMTS, v->fAutoExpandPMTs);
		CheckDlgButton(hDlg, IDC_UI_DECIMAL_PIDS, v->fDecimalPIDs);
		CheckDlgButton(hDlg, IDC_UI_HIDE_MINIMIZED, v->fHideWhenMinimized);
		CheckDlgButton(hDlg, IDC_UI_KEEP_SPECIAL_XML, v->fKeepSpecialXMLCharacters);
		CheckDlgButton(hDlg, IDC_UI_SHOW_NON_VIDEO_PCR, v->fShowNonVideoPCR);
		break;
	case WM_COMMAND:
		switch(LOWORD(wParam))
		{
		case IDOK:
			v->fAutoExpandPMTs = IsDlgButtonChecked(hDlg, IDC_UI_AUTO_EXPAND_PMTS);
			v->fDecimalPIDs = IsDlgButtonChecked(hDlg, IDC_UI_DECIMAL_PIDS);
			v->fHideWhenMinimized = IsDlgButtonChecked(hDlg, IDC_UI_HIDE_MINIMIZED);
			v->fKeepSpecialXMLCharacters = IsDlgButtonChecked(hDlg, IDC_UI_KEEP_SPECIAL_XML);
			v->fShowNonVideoPCR = IsDlgButtonChecked(hDlg, IDC_UI_SHOW_NON_VIDEO_PCR);
			break;
		}
		break;
	}
	return FALSE;
}

INT_PTR CALLBACK Settings_SerialReceiver(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg)
	{
	case WM_INITDIALOG:
		InitSerialControlDialog(hDlg);
		break;
	case WM_COMMAND:
		switch(LOWORD(wParam))
		{
		case IDOK:
			{
				int nIndex = (int)SendDlgItemMessage(hDlg, IDC_SERIAL_CONTROL_PORT, CB_GETCURSEL, 0, 0);
				if (nIndex == CB_ERR)
					break;
				SendDlgItemMessage(hDlg, IDC_SERIAL_CONTROL_PORT, CB_GETLBTEXT, nIndex, (LPARAM)v->szSerialReceiverPort);
				v->ss.fSerialReceiverControlEnabled = IsDlgButtonChecked(hDlg, IDC_SERIAL_CONTROL_ENABLED);
				nIndex = (int)SendDlgItemMessage(hDlg, IDD_SERIAL_CONTROL_RX_TYPES, LB_GETCURSEL, 0, 0);
				if (nIndex == LB_ERR)
					break;
//ppp				v->nSerialReceiverType = nIndex + 1;
			}
			break;
		}
		break;
	}

	return FALSE;
}

INT_PTR CALLBACK Settings_ControlServer(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg)
	{
	case WM_INITDIALOG:
		CheckDlgButton(hDlg, IDC_CONTROL_SERVER_ENABLED, v->fControlServerEnabled);
		SetDlgItemInt(hDlg, IDC_CONTROL_SERVER_PORT, v->nControlServerPort, FALSE);
		break;
	case WM_COMMAND:
		switch(LOWORD(wParam))
		{
		case IDOK:
			v->fControlServerEnabled = IsDlgButtonChecked(hDlg, IDC_CONTROL_SERVER_ENABLED);
			v->nControlServerPort = GetDlgItemInt(hDlg, IDC_CONTROL_SERVER_PORT, NULL, FALSE);
			break;
		}
		break;
	}
	return FALSE;
}

INT_PTR CALLBACK Settings_SplitFilenames(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg)
	{
	case WM_INITDIALOG:
		SetDlgItemText(hDlg, IDC_FORMAT_STRING, v->szSplitFormatString);
		break;
	case WM_COMMAND:
		switch(HIWORD(wParam))
		{
		case BN_CLICKED:
			switch(LOWORD(wParam))
			{
			case IDOK:
				GetDlgItemText(hDlg, IDC_FORMAT_STRING, v->szSplitFormatString, sizeof(v->szSplitFormatString));
				break;
			}
			break;
		case EN_CHANGE:
			if (LOWORD(wParam) == IDC_FORMAT_STRING)
				UpdateFilenameFormatPreview(hDlg);
			break;
		}
		break;
	}

	return FALSE;
}

INT_PTR CALLBACK Settings_DVHS(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg)
	{
	case WM_INITDIALOG:
		CheckDlgButton(hDlg, IDC_DVHS_AUTO_REC_STOP, v->fControlDVHSDeck);
		CheckDlgButton(hDlg, IDC_DVHS_AUTO_POWER, v->fPowerCycleDVHSDeck);
		CheckDlgButton(hDlg, IDC_DVHS_ATSC_PIDS, v->fDVHSForceATSC);
		break;
	case WM_COMMAND:
		switch(LOWORD(wParam))
		{
		case IDOK:
			v->fControlDVHSDeck = IsDlgButtonChecked(hDlg, IDC_DVHS_AUTO_REC_STOP);
			v->fPowerCycleDVHSDeck = IsDlgButtonChecked(hDlg, IDC_DVHS_AUTO_POWER);
			v->fDVHSForceATSC = IsDlgButtonChecked(hDlg, IDC_DVHS_ATSC_PIDS);
			break;
		}
		break;
	}
	return FALSE;
}

INT_PTR CALLBACK Settings_Source(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg)
	{
	case WM_INITDIALOG:
		{
			int nColumnPosition = 0;
			int nModuleIndex;
			int nIndex;
			HWND hWndSourceList = GetDlgItem(hDlg, IDC_SOURCE_LIST);
			LV_COLUMN lvc; 
			char szTemp[128];

			// Setup list view columns. 
			lvc.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM; 
			lvc.pszText = szTemp; 

			lvc.fmt = LVCFMT_LEFT; 
			lvc.cx = 150; 
			lstrcpy(szTemp, TEXT("Name"));
			ListView_InsertColumn(hWndSourceList, nColumnPosition++, &lvc); 

			lvc.fmt = LVCFMT_LEFT; 
			lvc.cx = 275; 
			lstrcpy(szTemp, TEXT("Description"));
			ListView_InsertColumn(hWndSourceList, nColumnPosition++, &lvc); 

			ListView_SetExtendedListViewStyle(hWndSourceList, LVS_EX_FULLROWSELECT);

			SetDlgItemText(hDlg, IDC_SOURCE_SELECTED_SOURCE, "");
			nModuleIndex = PopulateSourceList(hDlg, hWndSourceList);
			for (nIndex = 0; nIndex < nModuleIndex; nIndex++)
			{
				if (lstrcmp(v->sourcemodules[nIndex].szFilename, v->szSourceName) == 0)
				{
					ListView_SetItemState(hWndSourceList, nIndex, LVIS_FOCUSED | LVIS_SELECTED, LVIS_FOCUSED | LVIS_SELECTED);
					ListView_EnsureVisible(hWndSourceList, nIndex, FALSE);
					v->nSelectedSource = nIndex;
					SetDlgItemText(hDlg, IDC_SOURCE_SELECTED_SOURCE, v->sourcemodules[nIndex].szDisplayName);
					break;
				}
			}

			SetDlgItemInt(hDlg, IDC_SOURCE_DEVICE_NUMBER, v->ss.nSourceIndex, FALSE);
		}
		break;
	case WM_COMMAND:
		switch(LOWORD(wParam))
		{
		case IDOK:
			lstrcpy(v->szSourceName, v->sourcemodules[v->nSelectedSource].szFilename);
			v->ss.nSourceIndex = GetDlgItemInt(hDlg, IDC_SOURCE_DEVICE_NUMBER, NULL, FALSE);
			break;
		}
		break;
	case WM_NOTIFY: 
		switch (((LPNMHDR) lParam)->code)
		{ 
		case LVN_GETDISPINFO:
			GetSourceDispInfo((LV_DISPINFO *)lParam);
			break;
		case LVN_ITEMCHANGED:
			{
				LPNMLISTVIEW pnmv = (LPNMLISTVIEW)lParam;
				if (pnmv->uNewState & LVIS_SELECTED)
				{
					v->nSelectedSource = pnmv->iItem;
					SetDlgItemText(hDlg, IDC_SOURCE_SELECTED_SOURCE, v->sourcemodules[v->nSelectedSource].szDisplayName);
					EnableWindow(GetDlgItem(hDlg, IDC_SOURCE_DEVICE_NUMBER), v->sourcemodules[v->nSelectedSource].dwCapabilities & CAPABILITIES_MULTICARD);
					EnableWindow(GetDlgItem(hDlg, IDC_SOURCE_DEVICE_NUMBER_CAPTION), v->sourcemodules[v->nSelectedSource].dwCapabilities & CAPABILITIES_MULTICARD);
				}
			}
			break;
		}
		break;

	}
	return FALSE;
}

INT_PTR CALLBACK Settings_CA(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg)
	{
	case WM_INITDIALOG:
		{
			char szTemp[64];

			CheckDlgButton(hDlg, IDC_PLUGINS_PREFERED_CA_ID_ENABLED, v->fUsePreferedCAID);
			wsprintf(szTemp, "%x", v->nPrefereredCAID);
			SetDlgItemText(hDlg, IDC_PLUGINS_PREFERED_CA_ID, szTemp);
		}		
		break;
	case WM_COMMAND:
		switch(LOWORD(wParam))
		{
		case IDOK:
			{
				char szTemp[64];

				v->fUsePreferedCAID = IsDlgButtonChecked(hDlg, IDC_PLUGINS_PREFERED_CA_ID_ENABLED);
				GetDlgItemText(hDlg, IDC_PLUGINS_PREFERED_CA_ID, szTemp, sizeof(szTemp));
				sscanf(szTemp, "%x", &v->nPrefereredCAID);
			}		
			break;
		}
		break;
	}
	return FALSE;
}

INT_PTR CALLBACK Settings_Roku(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg)
	{
	case WM_INITDIALOG:
		SetDlgItemText(hDlg, IDC_ROKU_IP, v->szRokuIP);
		SetDlgItemText(hDlg, IDC_ROKU_USERNAME, v->szRokuUsername);
		SetDlgItemText(hDlg, IDC_ROKU_PASSWORD, v->szRokuPassword);
		SetDlgItemText(hDlg, IDC_ROKU_MPEGPSPLAY_LOCATION, v->szRokuMpegPSPlayLocation);
		CheckDlgButton(hDlg, IDC_ROKU_TRACE_DISABLED, v->fRokuTraceDisabled);
		break;
	case WM_COMMAND:
		switch(LOWORD(wParam))
		{
		case IDOK:
			GetDlgItemText(hDlg, IDC_ROKU_IP, v->szRokuIP, sizeof(v->szRokuIP));
			GetDlgItemText(hDlg, IDC_ROKU_USERNAME, v->szRokuUsername, sizeof(v->szRokuUsername));
			GetDlgItemText(hDlg, IDC_ROKU_PASSWORD, v->szRokuPassword, sizeof(v->szRokuPassword));
			GetDlgItemText(hDlg, IDC_ROKU_MPEGPSPLAY_LOCATION, v->szRokuMpegPSPlayLocation, sizeof(v->szRokuMpegPSPlayLocation));
			v->fRokuTraceDisabled = IsDlgButtonChecked(hDlg, IDC_ROKU_TRACE_DISABLED);
			break;
		}
		break;
	}
	return FALSE;
}

INT_PTR CALLBACK Settings_Stradis(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg)
	{
	case WM_INITDIALOG:
		CheckDlgButton(hDlg, IDC_STRADIS_FORCE_PAL, v->fForceStradisPAL);
		break;
	case WM_COMMAND:
		switch(LOWORD(wParam))
		{
		case IDOK:
			v->fForceStradisPAL = IsDlgButtonChecked(hDlg, IDC_STRADIS_FORCE_PAL);
			break;
		}
		break;
	}
	return FALSE;
}

INT_PTR CALLBACK Settings_XNS(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg)
	{
	case WM_INITDIALOG:
		SetDlgItemInt(hDlg, IDC_XNS_SERVER_PORT, v->nXNSServerPort, FALSE);
		break;
	case WM_COMMAND:
		switch(LOWORD(wParam))
		{
		case IDOK:
			v->nXNSServerPort = GetDlgItemInt(hDlg, IDC_XNS_SERVER_PORT, NULL, FALSE);
			break;
		}
		break;
	}
	return FALSE;
}

INT_PTR CALLBACK Settings_VLC(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg)
	{
	case WM_INITDIALOG:
		SetDlgItemText(hDlg, IDC_VLC_EXE_LOCATION, v->szVLCExeLocation);
		CheckDlgButton(hDlg, IDC_VLC_CONTROL_ENABLE, v->fVLCControl);
		CheckDlgButton(hDlg, IDC_VLC_CONTROL_SEND_LENGTH, v->fSendBogusHTTPSize);
		SetDlgItemInt(hDlg, IDC_VLC_PORT, v->nVLCPort, FALSE);
		v->nCurrentVLCEditConfig = 0;
		UpdateVLCConfigDisplay(hDlg, v->nCurrentVLCEditConfig);
		break;
	case WM_COMMAND:
		VLCSettings_WM_COMMAND(hDlg, wParam, lParam, TRUE);
		break;
	}
	return FALSE;
}

INT_PTR CALLBACK SettingsDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg)
	{
	case WM_INITDIALOG:
		{
			int nIndex;
			HDC hDC;
			HTREEITEM hFirst;
			HWND hWndTV = GetDlgItem(hDlg, IDC_SETTINGS_TREE);
			char szTitle[MAX_PATH];

			SendMessage(hDlg, WM_GETTEXT, (WPARAM)sizeof(szTitle), (LPARAM)szTitle);
			if (lstrlen(v->szProfileName))
			{
				lstrcat(szTitle, " - ");
				lstrcat(szTitle, v->szProfileName);
			}
			else
				lstrcat(szTitle, " - Default");
			SendMessage(hDlg, WM_SETTEXT, 0, (LPARAM)szTitle);

			hDC = GetDC(hDlg);
			hTitleFont = CreateFont(-MulDiv(14, GetDeviceCaps(hDC, LOGPIXELSY), 72),
									   0,
									   0,
									   0,
									   900,
									   FALSE,
									   FALSE,
									   FALSE,
									   ANSI_CHARSET,
									   OUT_DEFAULT_PRECIS,
									   CLIP_DEFAULT_PRECIS,
									   ANTIALIASED_QUALITY,
									   FF_DONTCARE | VARIABLE_PITCH,
									   "Arial");
			ReleaseDC(hDlg, hDC);

			nMaxSettingsPages = 0;
			nIndex = 0;
			hFirst = SetupTreeView(hDlg, "Source Module", nIndex++, IDD_SETTINGS_SOURCE, Settings_Source);
			SetupTreeView(hDlg, "Threads", nIndex++, IDD_SETTINGS_THREADS, Settings_Threads);
			SetupTreeView(hDlg, "Thumbnails", nIndex++, IDD_SETTINGS_THUMBNAILS, Settings_Thumbnails);
			SetupTreeView(hDlg, "EIT", nIndex++, IDD_SETTINGS_EIT, Settings_EIT);
			SetupTreeView(hDlg, "IP/DVB", nIndex++, IDD_SETTINGS_IPDVB, Settings_IPDVB);
			SetupTreeView(hDlg, "Parser", nIndex++, IDD_SETTINGS_PARSER, Settings_Parser);
			SetupTreeView(hDlg, "Chart", nIndex++, IDD_SETTINGS_CHART, Settings_Chart);
			SetupTreeView(hDlg, "User Interface", nIndex++, IDD_SETTINGS_UI, Settings_UI);
			SetupTreeView(hDlg, "Serial Receiver", nIndex++, IDD_SETTINGS_RECEIVER, Settings_SerialReceiver);
			SetupTreeView(hDlg, "Control Server", nIndex++, IDD_SETTINGS_CONTROL_SERVER, Settings_ControlServer);
			SetupTreeView(hDlg, "Split Filenames", nIndex++, IDD_SETTINGS_SPLIT_FILENAMES, Settings_SplitFilenames);
			SetupTreeView(hDlg, "D-VHS", nIndex++, IDD_SETTINGS_DVHS, Settings_DVHS);
			SetupTreeView(hDlg, "Conditional Access", nIndex++, IDD_SETTINGS_CA, Settings_CA);
			SetupTreeView(hDlg, "Roku HD-1000", nIndex++, IDD_SETTINGS_ROKU, Settings_Roku);
			SetupTreeView(hDlg, "Stradis", nIndex++, IDD_SETTINGS_STRADIS, Settings_Stradis);
			SetupTreeView(hDlg, "XNS Server", nIndex++, IDD_SETTINGS_XNS, Settings_XNS);
			SetupTreeView(hDlg, "VLC", nIndex++, IDD_SETTINGS_VLC, Settings_VLC);

			TreeView_SelectItem(hWndTV, hFirst);
			SetFocus(hWndTV);
		}
		break;
	case WM_DESTROY:
		{
			int i;
			for (i = 0; i < nMaxSettingsPages; i++)
				DestroyWindow(hWndSettings[i]);
			TreeView_DeleteAllItems(GetDlgItem(hDlg, IDC_SETTINGS_TREE));
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
				int i;
				for (i = 0; i < nMaxSettingsPages; i++)
					SendMessage(hWndSettings[i], WM_COMMAND, IDOK, 0);
				EndDialog(hDlg, TRUE);
			}
			break;
		case IDCANCEL:
			EndDialog(hDlg, FALSE);
			break;
		}
		break;
	case WM_NOTIFY:
		{
			LPNMHDR pnmh = (LPNMHDR)lParam;

			switch (pnmh->code)
		    {
			case TVN_SELCHANGED:
				{
					LPNM_TREEVIEW pnmtv = (LPNM_TREEVIEW)pnmh;
					HTREEITEM hti = pnmtv->itemNew.hItem;
					TVITEM tvi;
					int i;
					
					memset(&tvi, 0, sizeof(tvi));
					tvi.mask = TVIF_PARAM;
					tvi.hItem = hti;
					TreeView_GetItem(GetDlgItem(hDlg, IDC_SETTINGS_TREE), &tvi);
					for (i = 0; i < nMaxSettingsPages; i++)
					{
						if (i == tvi.lParam)
							ShowWindow(hWndSettings[i], SW_SHOWNORMAL);
						else
							ShowWindow(hWndSettings[i], SW_HIDE);
					}
				}
				break;
			}
		}
		break;
	}

	return FALSE;
}
