#include <windows.h>
#include <commctrl.h>
#include <time.h>
#include "TSReader.h"
#include "bcdmux.h"
#include "resource.h"
#include "reed-solomon.h"

extern PVARIABLES v;
extern char gszAppName[];
extern char gszKeyName[];

char gszSAPAddress[] = {"224.0.0.255"};
DWORD dwMulticastAddress;
DWORD dwMulticastAddressMask; 

// Forward declarations
void BuildOutputPAT(BYTE * pat, int nPMTIndex);

// Declarations
#define MAX_FORWARD_PID_STREAMS 32
#define MAX_FORWARD_PACKET_SIZE 188 * 7
#define MAX_FORWARDER_SOCKETS 8
 
// Internal structs
typedef struct _tagForwarderProgram
{
	BOOL fEnabled;
	
	int nPort;
	int nOutputByteCounter;
	int nSAPMessageLength;

	double dAllBytesWritten;

	char szDestination[MAX_PATH];

	BYTE pat[188];
	BYTE bOutputBuffer[MAX_FORWARD_PACKET_SIZE];
	BYTE bSAPMessage[1024];

	SOCKET sock[MAX_FORWARDER_SOCKETS];
	struct sockaddr_in server[MAX_FORWARDER_SOCKETS];
	struct sockaddr_in client;

} FORWARDERPROGRAM, *PFORWARDERPROGRAM;

typedef struct _tagForwarder
{
	BOOL fSAPEnabled;
	BOOL fMuxEnabled;
	BOOL fMuxDropNULLs;
	BOOL fThreadTerminated;
	BOOL fIncludeCATables;

	int nListSelectedProgram;
	int nListSelectedPMT;
	int nPipeBytes;
	int nMuxPort;
	int nMulticastTTL;

	HWND hWndStatus;
	HFONT hSmallTextFont;
	HFONT hSmallBoldTextFont;
	HBRUSH hGreenBrush;
	HBRUSH hRedBrush;
	HPEN hGreenPen;

	CRITICAL_SECTION csPipeBytes;

	SOCKET SAPsock;
	struct sockaddr_in SAPserver;
	struct sockaddr_in SAPclient;

	char szInterfaceAddress[MAX_PATH];
	char szSAPGroupName[MAX_PATH];
	char szMuxDestination[MAX_PATH];

	PFORWARDERPROGRAM fp[MAX_PAT_ENTRIES + 2];
	BYTE nForwardPIDs[8192][MAX_FORWARD_PID_STREAMS];

} FORWARDER, *PFORWARDER;

PFORWARDER forwarder;

void LoadForwarderSettings()
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
	lstrcat(szKeyName, "\\Forwarder");

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

					wsprintf(szValueName, "%05d_Enabled", v->pat.pmt[nPMTIndex].nProgramNumber);
					dwDataSize = sizeof(forwarder->fp[nPMTIndex]->fEnabled);
					RegQueryValueEx(hkMainReg, szValueName, NULL, &dwType, (BYTE *)&forwarder->fp[nPMTIndex]->fEnabled, &dwDataSize);

					wsprintf(szValueName, "%05d_Destination", v->pat.pmt[nPMTIndex].nProgramNumber);
					dwDataSize = sizeof(forwarder->fp[nPMTIndex]->szDestination);
					RegQueryValueEx(hkMainReg, szValueName, NULL, &dwType, (BYTE *)forwarder->fp[nPMTIndex]->szDestination, &dwDataSize);
				}
			}

			// Load other settings
			dwDataSize = sizeof(forwarder->fIncludeCATables);
			RegQueryValueEx(hkMainReg, "IncludeCATables", NULL, &dwType, (BYTE *)&forwarder->fIncludeCATables, &dwDataSize);				
			dwDataSize = sizeof(forwarder->szInterfaceAddress);
			RegQueryValueEx(hkMainReg, "InterfaceAddress", NULL, &dwType, (BYTE *)forwarder->szInterfaceAddress, &dwDataSize);	
			dwDataSize = sizeof(forwarder->szSAPGroupName);
			RegQueryValueEx(hkMainReg, "SAPGroupName", NULL, &dwType, (BYTE *)forwarder->szSAPGroupName, &dwDataSize);	
			dwDataSize = sizeof(forwarder->fSAPEnabled);
			RegQueryValueEx(hkMainReg, "SAPEnabled", NULL, &dwType, (BYTE *)&forwarder->fSAPEnabled, &dwDataSize);	
			dwDataSize = sizeof(forwarder->szMuxDestination);
			RegQueryValueEx(hkMainReg, "MuxDestination", NULL, &dwType, (BYTE *)forwarder->szMuxDestination, &dwDataSize);	
			dwDataSize = sizeof(forwarder->fMuxEnabled);
			RegQueryValueEx(hkMainReg, "MuxEnabled", NULL, &dwType, (BYTE *)&forwarder->fMuxEnabled, &dwDataSize);
			dwDataSize = sizeof(forwarder->fMuxDropNULLs);
			RegQueryValueEx(hkMainReg, "MuxDropNULLs", NULL, &dwType, (BYTE *)&forwarder->fMuxDropNULLs, &dwDataSize);		
			dwDataSize = sizeof(forwarder->nMulticastTTL);
			RegQueryValueEx(hkMainReg, "MulticastTTL", NULL, &dwType, (BYTE *)&forwarder->nMulticastTTL, &dwDataSize);		
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
					forwarder->fp[nPMTIndex]->fEnabled = TRUE;
					forwarder->fp[nPMTIndex]->fEnabled = FALSE;
					wsprintf(forwarder->fp[nPMTIndex]->szDestination, "224.0.0.%d:1234", nPMTIndex);
				}
			}
		}
		RegCloseKey(hkMainReg);
	}
}

void SaveForwarderSettings()
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
	lstrcat(szKeyName, "\\Forwarder");

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

				wsprintf(szValueName, "%05d_Enabled", v->pat.pmt[nPMTIndex].nProgramNumber);
				RegSetValueEx(hkMainReg, szValueName, 0, REG_DWORD, (BYTE *)&forwarder->fp[nPMTIndex]->fEnabled, sizeof(DWORD));
				wsprintf(szValueName, "%05d_Destination", v->pat.pmt[nPMTIndex].nProgramNumber);
				RegSetValueEx(hkMainReg, szValueName, 0, REG_SZ, (BYTE *)forwarder->fp[nPMTIndex]->szDestination, lstrlen(forwarder->fp[nPMTIndex]->szDestination) + 1);
			}
		}

		// Save global settings
		RegSetValueEx(hkMainReg, "IncludeCATables", 0, REG_DWORD, (BYTE *)&forwarder->fIncludeCATables, sizeof(DWORD));
		RegSetValueEx(hkMainReg, "InterfaceAddress", 0, REG_SZ, (BYTE *)forwarder->szInterfaceAddress, lstrlen(forwarder->szInterfaceAddress) + 1);
		RegSetValueEx(hkMainReg, "SAPGroupName", 0, REG_SZ, (BYTE *)forwarder->szSAPGroupName, lstrlen(forwarder->szSAPGroupName) + 1);
		RegSetValueEx(hkMainReg, "SAPEnabled", 0, REG_DWORD, (BYTE *)&forwarder->fSAPEnabled, sizeof(DWORD));
		RegSetValueEx(hkMainReg, "MuxDestination", 0, REG_SZ, (BYTE *)forwarder->szMuxDestination, lstrlen(forwarder->szMuxDestination) + 1);
		RegSetValueEx(hkMainReg, "MuxEnabled", 0, REG_DWORD, (BYTE *)&forwarder->fMuxEnabled, sizeof(DWORD));
		RegSetValueEx(hkMainReg, "MuxDropNULLs", 0, REG_DWORD, (BYTE *)&forwarder->fMuxDropNULLs, sizeof(DWORD));
		RegSetValueEx(hkMainReg, "MulticastTTL", 0, REG_DWORD, (BYTE *)&forwarder->nMulticastTTL, sizeof(DWORD));

		RegCloseKey(hkMainReg);
	}
}

void GetForwarderChannelListDispInfo(LV_DISPINFO *pnmv) 
{ 
    // Provide the item or subitem's text, if requested. 
    if (pnmv->item.mask & LVIF_TEXT)
	{ 
        int nPMTIndex = (int)(pnmv->item.lParam);
		switch(pnmv->item.iSubItem)
		{
		case 0:
			wsprintf(pnmv->item.pszText, "%d", v->pat.pmt[nPMTIndex].nProgramNumber);
			if (v->pChannelData[v->pat.pmt[nPMTIndex].nProgramNumber] != NULL)
			{
				if (lstrlen(v->pChannelData[v->pat.pmt[nPMTIndex].nProgramNumber]->szShortName))
				{
					lstrcat(pnmv->item.pszText, " ");
					lstrcat(pnmv->item.pszText, v->pChannelData[v->pat.pmt[nPMTIndex].nProgramNumber]->szShortName);
				}
			}
			break;
		case 1:
			if (forwarder->fp[nPMTIndex]->fEnabled)
				lstrcpy(pnmv->item.pszText, "Enabled");
			else
				pnmv->item.pszText[0] = '\0';
			break;
		case 2:
			lstrcpy(pnmv->item.pszText, forwarder->fp[nPMTIndex]->szDestination);
			break;
		}
	}
}

INT_PTR CALLBACK ForwarderSetupDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg)
	{
	case WM_INITDIALOG:
		{
			HWND hWndForwarderChannelList = GetDlgItem(hDlg, IDC_FORWARDER_CHANNEL_LIST);
			int nColumnPosition = 0;
			int nPMTIndex;
			LV_COLUMN lvc; 
			char szTemp[128];
			int nCount = 0;

			forwarder->nListSelectedProgram = -1;
			forwarder->nListSelectedPMT = -1;
			LoadForwarderSettings();
			if (forwarder->nMulticastTTL == 0)
				forwarder->nMulticastTTL = 1;

			// Setup list view columns. 
			lvc.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM; 
			lvc.pszText = szTemp; 

			lvc.fmt = LVCFMT_LEFT; 
			lvc.cx = 150; 
			lstrcpy(szTemp, TEXT("Program"));
			ListView_InsertColumn(hWndForwarderChannelList, nColumnPosition++, &lvc); 

			lvc.fmt = LVCFMT_CENTER; 
			lvc.cx = 60; 
			lstrcpy(szTemp, TEXT("Status"));
			ListView_InsertColumn(hWndForwarderChannelList, nColumnPosition++, &lvc); 

			lvc.fmt = LVCFMT_LEFT; 
			lvc.cx = 210; 
			lstrcpy(szTemp, TEXT("Destination"));
			ListView_InsertColumn(hWndForwarderChannelList, nColumnPosition++, &lvc); 

			ListView_SetExtendedListViewStyle(hWndForwarderChannelList, LVS_EX_FULLROWSELECT);

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
					ListView_InsertItem(hWndForwarderChannelList, &lvi);
				}
			}

			{
				HOSTENT *pHost; 
				HWND hCombo = GetDlgItem(hDlg, IDC_INTERFACE_ADDRESS);
				int  nAdapter = 0; 
				char szHostname[MAX_PATH] = {0}; 

				SendMessage(hCombo, CB_ADDSTRING, 0, (LPARAM)"127.0.0.1");

				gethostname(szHostname, sizeof(szHostname)); 
				pHost = gethostbyname(szHostname); 
				while (pHost->h_addr_list[nAdapter]) 
				{
					int nItem;
					char szTemp[128];

					wsprintf(szTemp, "%d.%d.%d.%d", 
						(BYTE)pHost->h_addr_list[nAdapter][0],
						(BYTE)pHost->h_addr_list[nAdapter][1],
						(BYTE)pHost->h_addr_list[nAdapter][2],
						(BYTE)pHost->h_addr_list[nAdapter][3]);
					nItem = (int)SendMessage(hCombo, CB_ADDSTRING, 0, (LPARAM)szTemp);
					if (lstrcmp(szTemp, forwarder->szInterfaceAddress) == 0)
						SendMessage(hCombo, CB_SETCURSEL, nItem, 0);
					nAdapter++;
				}
			}

			CheckDlgButton(hDlg, IDC_FORWARDER_INCLUDE_CA, forwarder->fIncludeCATables);
			CheckDlgButton(hDlg, IDC_FORWARDER_SAP_ENABLED, forwarder->fSAPEnabled);
			SetDlgItemText(hDlg, IDC_FORWARDER_SAP_GROUP, forwarder->szSAPGroupName);
			CheckDlgButton(hDlg, IDC_FORWARDER_MUX_ENABLED, forwarder->fMuxEnabled);
			SetDlgItemText(hDlg, IDC_FORWARDER_MUX_DESTINATION, forwarder->szMuxDestination);
			CheckDlgButton(hDlg, IDC_FORWARDER_MUX_NO_NULLS, forwarder->fMuxDropNULLs);
			SetDlgItemInt(hDlg, IDC_FORWARDER_MULTICAST_TTL, forwarder->nMulticastTTL, FALSE);

			SetFocus(GetDlgItem(hDlg, IDOK));

			if (v->fAutomaticForwarding)
			{
				v->fAutomaticForwarding = FALSE;
				PostMessage(hDlg, WM_COMMAND, IDOK, 0);
			}
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
			case IDOK:
				{
					int nItem;
					int nPMTIndex;

					// Make sure the desination addresses are correct
					for (nPMTIndex = 0; nPMTIndex < MAX_PAT_ENTRIES; nPMTIndex++)
					{
						if (v->pat.pmt[nPMTIndex].nPMTPID == 0)
							break;
						if (v->pat.pmt[nPMTIndex].nProgramNumber != 0)
						{
							if (forwarder->fp[nPMTIndex]->fEnabled)
							{
								if (strstr(forwarder->fp[nPMTIndex]->szDestination, ":") == NULL)
								{
									char szTemp[128];
									wsprintf(szTemp, "The destination address for program %d is not formatted correctly - use addr:port notation", v->pat.pmt[nPMTIndex].nProgramNumber);
									MessageBox(hDlg, szTemp, gszAppName, MB_ICONSTOP);
									return FALSE;
								}
							}
						}
					}
											
					nItem = (int)SendDlgItemMessage(hDlg, IDC_INTERFACE_ADDRESS, CB_GETCURSEL, 0, 0);
					if (nItem == CB_ERR)
						forwarder->szInterfaceAddress[0] = '\0';
					else
						SendDlgItemMessage(hDlg, IDC_INTERFACE_ADDRESS, CB_GETLBTEXT, nItem, (LPARAM)forwarder->szInterfaceAddress);
					forwarder->fIncludeCATables = IsDlgButtonChecked(hDlg, IDC_FORWARDER_INCLUDE_CA);
					forwarder->fSAPEnabled = IsDlgButtonChecked(hDlg, IDC_FORWARDER_SAP_ENABLED);
					GetDlgItemText(hDlg, IDC_FORWARDER_SAP_GROUP, forwarder->szSAPGroupName, sizeof(forwarder->szSAPGroupName));

					forwarder->fMuxEnabled = IsDlgButtonChecked(hDlg, IDC_FORWARDER_MUX_ENABLED);
					GetDlgItemText(hDlg, IDC_FORWARDER_MUX_DESTINATION, forwarder->szMuxDestination, sizeof(forwarder->szMuxDestination));
					if (strstr(forwarder->szMuxDestination, ":") == NULL && forwarder->fMuxEnabled)
					{
						MessageBox(hDlg, "The mux destination is not formatted correctly - use addr:port notation", gszAppName, MB_ICONSTOP);
						SetFocus(GetDlgItem(hDlg, IDC_FORWARDER_MUX_DESTINATION));
						break;
					}
					forwarder->fMuxDropNULLs = IsDlgButtonChecked(hDlg, IDC_FORWARDER_MUX_NO_NULLS);
					forwarder->nMulticastTTL = GetDlgItemInt(hDlg, IDC_FORWARDER_MULTICAST_TTL, NULL, FALSE);
					if (forwarder->nMulticastTTL < 1 || forwarder->nMulticastTTL > 255)
						forwarder->nMulticastTTL = 1;
					SaveForwarderSettings();
					EndDialog(hDlg, TRUE);
				}
				break;
			case IDCANCEL:
				EndDialog(hDlg, FALSE);
				break;
			case IDC_FORWARDER_CHANNEL_ENABLED:
				forwarder->fp[forwarder->nListSelectedPMT]->fEnabled = IsDlgButtonChecked(hDlg, IDC_FORWARDER_CHANNEL_ENABLED);
				ListView_RedrawItems(GetDlgItem(hDlg, IDC_FORWARDER_CHANNEL_LIST), forwarder->nListSelectedProgram, forwarder->nListSelectedProgram);
				break;
			}
			break;
		case EN_CHANGE:
			switch(LOWORD(wParam))
			{
			case IDC_FORWARDER_CHANNEL_DESTINATION:
				GetDlgItemText(hDlg, IDC_FORWARDER_CHANNEL_DESTINATION, forwarder->fp[forwarder->nListSelectedPMT]->szDestination, sizeof(forwarder->fp[forwarder->nListSelectedPMT]->szDestination));
				ListView_RedrawItems(GetDlgItem(hDlg, IDC_FORWARDER_CHANNEL_LIST), forwarder->nListSelectedProgram, forwarder->nListSelectedProgram);
				break;
			}
		}
		break;
	case WM_NOTIFY: 
		switch (((LPNMHDR) lParam)->code)
		{ 
		case LVN_GETDISPINFO:
			{
				NMLVDISPINFO * pnmv = (NMLVDISPINFO *)lParam;
				GetForwarderChannelListDispInfo((LV_DISPINFO *) lParam);
			}
			break;
		case LVN_ITEMCHANGED:
			{
				LPNMLISTVIEW pnmv = (LPNMLISTVIEW)lParam;
				if (pnmv->uNewState & LVIS_SELECTED)
				{
					if (forwarder->nListSelectedProgram == -1)
					{
						// First time - enable controls
						EnableWindow(GetDlgItem(hDlg, IDC_FORWARDER_CHANNEL_ENABLED), TRUE);
						EnableWindow(GetDlgItem(hDlg, IDC_FORWARDER_CHANNEL_DESTINATION), TRUE);
						EnableWindow(GetDlgItem(hDlg, IDC_FORWARDER_CHANNEL_PORT), TRUE);
					}
					forwarder->nListSelectedProgram = pnmv->iItem;
					forwarder->nListSelectedPMT = (int)pnmv->lParam;
					CheckDlgButton(hDlg, IDC_FORWARDER_CHANNEL_ENABLED, forwarder->fp[forwarder->nListSelectedPMT]->fEnabled);
					SetDlgItemText(hDlg, IDC_FORWARDER_CHANNEL_DESTINATION, forwarder->fp[forwarder->nListSelectedPMT]->szDestination);
				}
			}
			break;
		}
		break;
	}

	return FALSE;
}

int ReadFromForwarderPipe(HANDLE hPipe, BYTE * pBuffer, int nLength)
{
	int nRequestedLength = nLength;
	DWORD dwRead;

	while (nLength)
	{
		ReadFile(hPipe, pBuffer, nLength, &dwRead, NULL);
		if (dwRead == 0)
			return 0;
		pBuffer += dwRead;
		nLength -= dwRead;
	}

	return nRequestedLength;
}

void GenerateForwarderPATs()
{
	int nPMTIndex;

	for (nPMTIndex = 0; nPMTIndex < MAX_PAT_ENTRIES; nPMTIndex++)
	{
		if (v->pat.pmt[nPMTIndex].nPMTPID == 0)
			break;
		if (v->pat.pmt[nPMTIndex].nProgramNumber != 0)
		{
			if (forwarder->fp[nPMTIndex]->fEnabled == TRUE)
			{
				BuildOutputPAT(forwarder->fp[nPMTIndex]->pat, nPMTIndex);
			}
		}
	}
}

void AddPMTEntryToForwarderPIDList(int nPID, int nPMTIndex)
{
	int i;

	for (i = 0; i < MAX_FORWARD_PID_STREAMS; i++)
	{
		if (forwarder->nForwardPIDs[nPID][i] == nPMTIndex + 1)
			return;	// already got this

		if (forwarder->nForwardPIDs[nPID][i] == 0)
		{
			forwarder->nForwardPIDs[nPID][i] = nPMTIndex + 1;
			return;
		}
	}
	MessageBox(v->hWndMainWindow, "OUT OF ROOM in AddPMTEntryToForwarderPIDList - tell support@tsreader.co.uk", gszAppName, MB_ICONSTOP);
}

void BuildForwarderPIDList()
{
	// This initializes the nForwardPIDs[] list
	// Each nForwardPIDs entry that's being recorded
	// contains a pointer +1 to the PMT index (forwarder->fp too)
	// so when we receive data from the main thread we can tell
	// really quick which program this recording goes to
	int nPMTIndex;

	for (nPMTIndex = 0; nPMTIndex < MAX_PAT_ENTRIES; nPMTIndex++)
	{
		if (v->pat.pmt[nPMTIndex].nPMTPID == 0)
			break;
		if (v->pat.pmt[nPMTIndex].nProgramNumber != 0)
		{
			if (forwarder->fp[nPMTIndex]->fEnabled == TRUE)
			{
				int nESIndex;
				BOOL fAlreadyGotPCR = FALSE;

				for (nESIndex = 0; nESIndex < MAX_ESLIST_ENTRIES; nESIndex++)
				{
					if (v->pat.pmt[nPMTIndex].es[nESIndex].nESPID == 0)
						break;
					if (v->pat.pmt[nPMTIndex].es[nESIndex].nESPID == v->pat.pmt[nPMTIndex].nPCRPID)
						fAlreadyGotPCR = TRUE;
					AddPMTEntryToForwarderPIDList(v->pat.pmt[nPMTIndex].es[nESIndex].nESPID, nPMTIndex);
					if (forwarder->fIncludeCATables)
					{
						// Check for a CA descriptors at the ES level
						BYTE * pDescriptorData = v->pat.pmt[nPMTIndex].es[nESIndex].pDescriptors;
						int nDescriptorsLength = v->pat.pmt[nPMTIndex].es[nESIndex].nDescriptorsLength;

						if (pDescriptorData)
						{
							int nCurrentIndex = 0;
							do
							{
								if (pDescriptorData[nCurrentIndex] == 9)
								{
									// CA descriptor - add the PID to our list
									int nECMPID = ((pDescriptorData[nCurrentIndex + 4] << 8) + pDescriptorData[nCurrentIndex + 5]) & 0x1fff;
									AddPMTEntryToForwarderPIDList(nECMPID, nPMTIndex);
								}
								nCurrentIndex += (BYTE)pDescriptorData[nCurrentIndex + 1]; // descriptor length
								nCurrentIndex += 2;	// descriptor tag and length
							} while (nCurrentIndex < nDescriptorsLength);
						}
					}
				}

				// Plus the PCR if it's on a seperate PID
				if (!fAlreadyGotPCR)
					AddPMTEntryToForwarderPIDList(v->pat.pmt[nPMTIndex].nPCRPID, nPMTIndex);
				
				// Plus the PMT 
				AddPMTEntryToForwarderPIDList(v->pat.pmt[nPMTIndex].nPMTPID, nPMTIndex);

				// Additional CA stuff
				if (forwarder->fIncludeCATables)
				{
					int nDescriptorIndex;
					BYTE * pDescriptorData = v->pat.pmt[nPMTIndex].pProgramInfo;
					int nDescriptorsLength = v->pat.pmt[nPMTIndex].nProgramInfoLength;
					int nCurrentIndex = 0;

					// The CAT itself
					AddPMTEntryToForwarderPIDList(0x0001, nPMTIndex);

					// The EMM PIDs indexed by the CAT
					for (nDescriptorIndex = 0; nDescriptorIndex < MAX_CAT_DESCRIPTORS; nDescriptorIndex++)
					{
						if (v->cat.pDescriptor[nDescriptorIndex] == NULL)
							break;
						if (v->cat.pDescriptor[nDescriptorIndex][0] == 9)
						{
							int nEMMPID = ((v->cat.pDescriptor[nDescriptorIndex][4] << 8) + v->cat.pDescriptor[nDescriptorIndex][5]) & 0x1fff;
							AddPMTEntryToForwarderPIDList(nEMMPID, nPMTIndex);
						}
					}

					// Check the PMT for CA descriptors in case the ECM is indexed there
					if (pDescriptorData != NULL)
					{
						do
						{
							if (pDescriptorData[nCurrentIndex] == 9)
							{
								int nECMPID = ((pDescriptorData[nCurrentIndex + 4] << 8) + pDescriptorData[nCurrentIndex + 5]) & 0x1fff;
								AddPMTEntryToForwarderPIDList(nECMPID, nPMTIndex);
							}

							nCurrentIndex += (BYTE)pDescriptorData[nCurrentIndex + 1]; // descriptor length
							nCurrentIndex += 2;	// descriptor tag and length
						} while (nCurrentIndex < nDescriptorsLength);
					}
				}
			}
		}
	}
}

void WriteForwarderBlockBuffer(int nPMTIndex, BYTE * pData, int nLength)
{
	if (nLength + forwarder->fp[nPMTIndex]->nOutputByteCounter > MAX_FORWARD_PACKET_SIZE)
	{
		int nSocket;
		int nOutputBytes = forwarder->fp[nPMTIndex]->nOutputByteCounter;

		for (nSocket = 0; nSocket < MAX_FORWARDER_SOCKETS; nSocket++)\
		{
			int nSend;

			if (forwarder->fp[nPMTIndex]->sock[nSocket] == 0)
				break;
			nSend = sendto(forwarder->fp[nPMTIndex]->sock[nSocket],
						   forwarder->fp[nPMTIndex]->bOutputBuffer,
						   nOutputBytes,
						   0,
						   (struct sockaddr *)&forwarder->fp[nPMTIndex]->server[nSocket],
						   sizeof(forwarder->fp[nPMTIndex]->server[nSocket]) );
			if (nSend != nOutputBytes)
			{
#ifdef _DEBUG
				OutputDebugString(".");
#endif _DEBUG
			}
		}
		forwarder->fp[nPMTIndex]->nOutputByteCounter = 0;
	}
	memcpy(&forwarder->fp[nPMTIndex]->bOutputBuffer[forwarder->fp[nPMTIndex]->nOutputByteCounter],
		   pData,
		   nLength);
	forwarder->fp[nPMTIndex]->nOutputByteCounter += nLength;
	forwarder->fp[nPMTIndex]->dAllBytesWritten += nLength;
}

void ForwardPIDData(BYTE * pPacket, int nPID)
{
	int nPMTIndex;
	int i;

	if (forwarder->fMuxEnabled)
	{
		if (!(forwarder->fMuxDropNULLs && nPID == 0x1fff))
			WriteForwarderBlockBuffer(MAX_PAT_ENTRIES, pPacket, 188);
	}

	if (nPID == 0x1fff)
		return;

	if (nPID == 0)
	{
		// Received a PAT so output the appropriate PAT for each
		// recording stream
		for (nPMTIndex = 0; nPMTIndex < MAX_PAT_ENTRIES; nPMTIndex++)
		{
			if (v->pat.pmt[nPMTIndex].nPMTPID == 0)
				break;
			if (v->pat.pmt[nPMTIndex].nProgramNumber != 0)
			{
				if (forwarder->fp[nPMTIndex]->fEnabled == TRUE)
				{
					int nCurrentContinuity = forwarder->fp[nPMTIndex]->pat[3] & 0x0f;
					WriteForwarderBlockBuffer(nPMTIndex, forwarder->fp[nPMTIndex]->pat, 188);
					nCurrentContinuity++;
					nCurrentContinuity &= 0x0f;
					forwarder->fp[nPMTIndex]->pat[3] &= 0xf0;
					forwarder->fp[nPMTIndex]->pat[3] |= nCurrentContinuity;
				}
			}
		}
	}
	else
	{
		for (i = 0; i < MAX_FORWARD_PID_STREAMS; i++)
		{
			if (forwarder->nForwardPIDs[nPID][i] == 0)
				return;
			nPMTIndex = forwarder->nForwardPIDs[nPID][i] - 1;
			if (nPMTIndex < 0)
				continue;
			if (forwarder->fp[nPMTIndex]->fEnabled)
				WriteForwarderBlockBuffer(nPMTIndex, pPacket, 188);
		}
	}
}

void BuildIndividualSocket(int nPMTIndex, char * szDestination)
{
	BOOL fFlag = TRUE;
	int nRet;
	int nTransmitUDPBufferSize = 256 * 1024;
	int nSocket;
	int nPort;
	char * szColon;
	struct ip_mreq stIpMreq;
	char szNewDestination[MAX_PATH];

	lstrcpy(szNewDestination, szDestination);
	szColon = strstr(szNewDestination, ":");
	if (szColon == NULL)
		return;
	*szColon = '\0';
	sscanf(szColon + 1, "%d", &nPort);
	if (nPort == 0)
		return;

	for (nSocket = 0; nSocket < MAX_FORWARDER_SOCKETS; nSocket++)
	{
		if (forwarder->fp[nPMTIndex]->sock[nSocket] == 0)
			break;
	}
	if (nSocket == MAX_FORWARDER_SOCKETS)
		return;		// out of space

	forwarder->fp[nPMTIndex]->sock[nSocket] = socket(AF_INET, SOCK_DGRAM, 0);
	if ((int)forwarder->fp[nPMTIndex]->sock[nSocket] < 0)
	{
		OutputDebugString("TSReader: Forwarder: socket() failed\n");
		return;
	}

	memset(&forwarder->fp[nPMTIndex]->client, 0, sizeof(forwarder->fp[nPMTIndex]->client));
	forwarder->fp[nPMTIndex]->client.sin_family = AF_INET;
	if (lstrlen(forwarder->szInterfaceAddress))	
		forwarder->fp[nPMTIndex]->client.sin_addr.s_addr = inet_addr(forwarder->szInterfaceAddress);
	else
		forwarder->fp[nPMTIndex]->client.sin_addr.s_addr = htonl(INADDR_ANY);
	forwarder->fp[nPMTIndex]->client.sin_port = htons(0);
	
	nRet = bind(forwarder->fp[nPMTIndex]->sock[nSocket], (struct sockaddr *)&forwarder->fp[nPMTIndex]->client, sizeof(forwarder->fp[nPMTIndex]->client));
	if (nRet < 0)
	{
		OutputDebugString("TSReader: Forwarder: bind() failed\n");
		//return FALSE;
	}

	memset(&forwarder->fp[nPMTIndex]->server[nSocket], 0, sizeof(forwarder->fp[nPMTIndex]->server[nSocket]));
	forwarder->fp[nPMTIndex]->server[nSocket].sin_family = AF_INET;
//	if (nPMTIndex == MAX_PAT_ENTRIES)
//	{
//		lstrcpy(forwarder->fp[nPMTIndex]->szDestination, forwarder->szMuxDestination);
//		forwarder->fp[nPMTIndex]->nPort = forwarder->nMuxPort;
//	}
	forwarder->fp[nPMTIndex]->server[nSocket].sin_addr.s_addr = inet_addr(szNewDestination);
	forwarder->fp[nPMTIndex]->server[nSocket].sin_port = htons(nPort);

	nRet = setsockopt(forwarder->fp[nPMTIndex]->sock[nSocket], SOL_SOCKET, SO_SNDBUF, (char *)&nTransmitUDPBufferSize, sizeof(nTransmitUDPBufferSize));
	if (nRet == SOCKET_ERROR)
	{
		OutputDebugString("TSReader: Forwarder: setsockopt(SO_SNDBUF) failed\n");
		return;
	}

	if ((forwarder->fp[nPMTIndex]->server[nSocket].sin_addr.s_addr & dwMulticastAddressMask) == dwMulticastAddress)
	{
		stIpMreq.imr_multiaddr.s_addr = inet_addr(szNewDestination); // group addr 
		if (lstrlen(forwarder->szInterfaceAddress))
			stIpMreq.imr_interface.s_addr = inet_addr(forwarder->szInterfaceAddress);
		else
			stIpMreq.imr_interface.s_addr = INADDR_ANY; // use default 
		nRet = setsockopt(forwarder->fp[nPMTIndex]->sock[nSocket], IPPROTO_IP, IP_ADD_MEMBERSHIP, (char FAR *)&stIpMreq, sizeof (struct ip_mreq));
		if (nRet == SOCKET_ERROR)
		{
			OutputDebugString("TSReader: Forwarder: setsockopt(IP_ADD_MEMBERSHIP) failed\n");
			return;
		}

		// Set the multicast TTL
		nRet = setsockopt(forwarder->fp[nPMTIndex]->sock[nSocket], IPPROTO_IP, IP_MULTICAST_TTL, (char *)&forwarder->nMulticastTTL, sizeof(forwarder->nMulticastTTL));
		if (nRet == SOCKET_ERROR)
		{
			OutputDebugString("TSReader: Forwarder: setsockopt(IP_MULTICAST_TTL) failed\n");
			return;
		}
	}
}

void BuildSockets(int nPMTIndex, char * szDestination)
{
	char szDestinationCopy[MAX_PATH];
	char * szCurrentInputDestination = szDestinationCopy;

	lstrcpy(szDestinationCopy, szDestination);
	do
	{
		char * szNextComma = strstr(szCurrentInputDestination, ",");
		char szCurrentDestination[MAX_PATH];
		
		if (szNextComma == NULL)
		{
			if (lstrlen(szCurrentInputDestination) == 0)
				break;
		}
		else
		{
			*szNextComma = '\0';
		}
		lstrcpy(szCurrentDestination, szCurrentInputDestination);
		BuildIndividualSocket(nPMTIndex, szCurrentDestination);
		if (szNextComma == NULL)
			break;
		szCurrentInputDestination = szNextComma + 1;
	} while (*szCurrentInputDestination != '\0');
}

void OpenForwarderSockets()
{
	int nPMTIndex;

	for (nPMTIndex = 0; nPMTIndex < MAX_PAT_ENTRIES; nPMTIndex++)
	{
		if (v->pat.pmt[nPMTIndex].nPMTPID == 0)
			break;
		if (v->pat.pmt[nPMTIndex].nProgramNumber != 0)
		{
			if (forwarder->fp[nPMTIndex]->fEnabled == TRUE)
			{
				BuildSockets(nPMTIndex, forwarder->fp[nPMTIndex]->szDestination); 
			}
		}
	}
	if (forwarder->fMuxEnabled)
		BuildSockets(MAX_PAT_ENTRIES, forwarder->szMuxDestination);
}

void ShutdownIndividualSocket(int nPMTIndex)
{
	BOOL fCloseSocket = TRUE;
	int nRet;
	int nSocket;

	for (nSocket = 0; nSocket < MAX_FORWARDER_SOCKETS; nSocket++)
	{
		if (forwarder->fp[nPMTIndex]->sock[nSocket] == 0)
			break;
		if ((forwarder->fp[nPMTIndex]->server[nSocket].sin_addr.s_addr & dwMulticastAddressMask) == dwMulticastAddress)
		{
			struct ip_mreq stIpMreq;

			stIpMreq.imr_multiaddr.s_addr = forwarder->fp[nPMTIndex]->server[nSocket].sin_addr.s_addr; // group addr  
			if (lstrlen(forwarder->szInterfaceAddress))
				stIpMreq.imr_interface.s_addr = inet_addr(forwarder->szInterfaceAddress);
			else
				stIpMreq.imr_interface.s_addr = INADDR_ANY; // use default 
			nRet = setsockopt(forwarder->fp[nPMTIndex]->sock[nSocket], IPPROTO_IP, IP_DROP_MEMBERSHIP, (char *)&stIpMreq, sizeof(stIpMreq));
			if (nRet == SOCKET_ERROR)
			{
				OutputDebugString("TSReader: Forwarder: setsockopt() IP_DROP_MEMBERSHIP failed\n");
				fCloseSocket = FALSE;
			}
		}
		if (fCloseSocket == TRUE)
			closesocket(forwarder->fp[nPMTIndex]->sock[nSocket]);
	}
}

void ShutdownForwarderSockets()
{
	int nPMTIndex;

	for (nPMTIndex = 0; nPMTIndex < MAX_PAT_ENTRIES; nPMTIndex++)
	{
		if (v->pat.pmt[nPMTIndex].nPMTPID == 0)
			break;
		if (v->pat.pmt[nPMTIndex].nProgramNumber != 0)
		{
			if (forwarder->fp[nPMTIndex]->fEnabled == TRUE)
				ShutdownIndividualSocket(nPMTIndex);
		}
	}
	if (forwarder->fMuxEnabled)
		ShutdownIndividualSocket(MAX_PAT_ENTRIES);
}

void BuildIndividualSAPMessage(int nPMTIndex)
{
	int i;
	char szTemp[1024];
	char szToolName[128];
	char szChannelName[256] = {""};

	static char szApplicationType[] = {"application/sdp"};
	static char szSDPMask[] = {"v=0\r\n"
				               "o=- %d 26 IN IP4 224.0.0.255\r\n"
							   "s=%s\r\n"
							   "t=0 0\r\n"
							   "c=IN IP4 %s /1\r\n"
							   "m=video %d udp 33\r\n"
							   "a=tool:%s\r\n"
							   "a=type:broadcast\r\n"
							   "a=x-plgroup:%s\r\n"};

	set_buf(BM_USER_THREAD, forwarder->fp[nPMTIndex]->bSAPMessage, sizeof(forwarder->fp[nPMTIndex]->bSAPMessage), TRUE);
	set_bits(BM_USER_THREAD, 1, 3);		// version number
	set_bits(BM_USER_THREAD, 0, 1);		// address type: IPV4
	set_bits(BM_USER_THREAD, 0, 1);		// reserved
	set_bits(BM_USER_THREAD, 0, 1);		// message-type: announcement
	set_bits(BM_USER_THREAD, 0, 1);		// payload not encrypted
	set_bits(BM_USER_THREAD, 0, 1);		// compression: not compressed
	set_bits(BM_USER_THREAD, 0, 8);		// authentication length
	set_bits(BM_USER_THREAD, 0xfcfd, 16);		// message identifier hash
	set_bits(BM_USER_THREAD, 0x61, 8);		// originating source 1
	set_bits(BM_USER_THREAD, 0x6d, 8);		// originating source 2
	set_bits(BM_USER_THREAD, 0x5f, 8);		// originating source 3
	set_bits(BM_USER_THREAD, 0x6f, 8);		// originating source 4
	for (i = 0; i < sizeof(szApplicationType); i++)
		set_bits(BM_USER_THREAD, szApplicationType[i], 8);		// 
	
	wsprintf(szToolName, "%s", gszAppName, GetTSRVersion(NULL));
	if (nPMTIndex == MAX_PAT_ENTRIES)
		lstrcpy(szChannelName, "Entire Mux");
	else
	{
		if (v->pChannelData[v->pat.pmt[nPMTIndex].nProgramNumber] != NULL)
		{
			if (lstrlen(v->pChannelData[v->pat.pmt[nPMTIndex].nProgramNumber]->szShortName))
				lstrcpy(szChannelName, v->pChannelData[v->pat.pmt[nPMTIndex].nProgramNumber]->szShortName);
		}
	}
	if (!lstrlen(szChannelName))
		wsprintf(szChannelName, "Program %d", v->pat.pmt[nPMTIndex].nProgramNumber);
	wsprintf(szTemp, szSDPMask,
			 v->pat.pmt[nPMTIndex].nProgramNumber,     
			 szChannelName,
			 forwarder->fp[nPMTIndex]->szDestination,
			 forwarder->fp[nPMTIndex]->nPort,
			 szToolName,
			 forwarder->szSAPGroupName);
	for (i = 0; i < lstrlen(szTemp); i++)
		set_bits(BM_USER_THREAD, szTemp[i], 8);
	forwarder->fp[nPMTIndex]->nSAPMessageLength = get_byte_pos(BM_USER_THREAD);
}

void SAPSetup()
{
	int nRet;
	int nPMTIndex;
	struct ip_mreq stIpMreq;

	if (!forwarder->fSAPEnabled)
		return;

	forwarder->SAPsock = socket(AF_INET, SOCK_DGRAM, 0);
	if ((int)forwarder->SAPsock < 0)
	{
		OutputDebugString("TSReader: Forwarder: socket() failed\n");
		return;
	}

	memset(&forwarder->SAPclient, 0, sizeof(forwarder->SAPclient));
	forwarder->SAPclient.sin_family = AF_INET;
	if (lstrlen(forwarder->szInterfaceAddress))	
		forwarder->SAPclient.sin_addr.s_addr = inet_addr(forwarder->szInterfaceAddress);
	else	
		forwarder->SAPclient.sin_addr.s_addr = htonl(INADDR_ANY);
	forwarder->SAPclient.sin_port = htons(0);
	nRet = bind(forwarder->SAPsock, (struct sockaddr *)&forwarder->SAPclient, sizeof(forwarder->SAPclient));
	if (nRet < 0)
	{
		OutputDebugString("TSReader: Forwarder: bind() failed\n");
		//return FALSE;
	}

	memset(&forwarder->SAPserver, 0, sizeof(forwarder->SAPserver));
	forwarder->SAPserver.sin_family = AF_INET;
	forwarder->SAPserver.sin_addr.s_addr = inet_addr(gszSAPAddress);
	forwarder->SAPserver.sin_port = htons(9875);

	stIpMreq.imr_multiaddr.s_addr = inet_addr(gszSAPAddress); // group addr 
	if (lstrlen(forwarder->szInterfaceAddress))
		stIpMreq.imr_interface.s_addr = inet_addr(forwarder->szInterfaceAddress);
	else
		stIpMreq.imr_interface.s_addr = INADDR_ANY; // use default 
	nRet = setsockopt(forwarder->SAPsock, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char FAR *)&stIpMreq, sizeof (struct ip_mreq));
	if (nRet == SOCKET_ERROR)
	{
		OutputDebugString("TSReader: Forwarder: setsockopt() failed\n");
		return;
	}

	// Build the SAP messages
	for (nPMTIndex = 0; nPMTIndex < MAX_PAT_ENTRIES; nPMTIndex++)
	{
		if (v->pat.pmt[nPMTIndex].nPMTPID == 0)
			break;
		if (v->pat.pmt[nPMTIndex].nProgramNumber == 0)
			continue;
		if (forwarder->fp[nPMTIndex]->fEnabled)
			BuildIndividualSAPMessage(nPMTIndex);
	}	
	if (forwarder->fMuxEnabled)
		BuildIndividualSAPMessage(MAX_PAT_ENTRIES);
}

void SAPShutdown()
{
	BOOL fCloseSocket = TRUE;
	int nRet;
	int nPMTIndex;
	struct ip_mreq stIpMreq;

	if (!forwarder->fSAPEnabled)
		return;

	for (nPMTIndex = 0; nPMTIndex < MAX_PAT_ENTRIES; nPMTIndex++)
	{
		if (v->pat.pmt[nPMTIndex].nPMTPID == 0)
			break;
		if (v->pat.pmt[nPMTIndex].nProgramNumber == 0)
			continue;
		if (forwarder->fp[nPMTIndex]->fEnabled)
		{
			forwarder->fp[nPMTIndex]->bSAPMessage[0] |= 0x04;
			sendto(forwarder->SAPsock,
							   forwarder->fp[nPMTIndex]->bSAPMessage,
							   forwarder->fp[nPMTIndex]->nSAPMessageLength,
							   0,
							   (struct sockaddr *)&forwarder->SAPserver,
							   sizeof(forwarder->SAPserver) );
		}
	}
	if (forwarder->fMuxEnabled)
	{
		forwarder->fp[MAX_PAT_ENTRIES]->bSAPMessage[0] |= 0x04;
		sendto(forwarder->SAPsock,
						   forwarder->fp[MAX_PAT_ENTRIES]->bSAPMessage,
						   forwarder->fp[MAX_PAT_ENTRIES]->nSAPMessageLength,
						   0,
						   (struct sockaddr *)&forwarder->SAPserver,
						   sizeof(forwarder->SAPserver) );
	}
	Sleep(500);

	stIpMreq.imr_multiaddr.s_addr = inet_addr(gszSAPAddress); // group addr  
	if (lstrlen(forwarder->szInterfaceAddress))
		stIpMreq.imr_interface.s_addr = inet_addr(forwarder->szInterfaceAddress);
	else
		stIpMreq.imr_interface.s_addr = INADDR_ANY; // use default 
	nRet = setsockopt(forwarder->SAPsock, IPPROTO_IP, IP_DROP_MEMBERSHIP, (char *)&stIpMreq, sizeof(stIpMreq));
	if (nRet == SOCKET_ERROR)
	{
		OutputDebugString("TSReader: Forwarder: setsockopt() IP_DROP_MEMBERSHIP failed\n");
		fCloseSocket = FALSE;
	}
	if (fCloseSocket == TRUE)
		closesocket(forwarder->SAPsock);
}

void SendSAP()
{
	int nPMTIndex;

	if (!forwarder->fSAPEnabled)
		return;

	for (nPMTIndex = 0; nPMTIndex < MAX_PAT_ENTRIES; nPMTIndex++)
	{
		if (v->pat.pmt[nPMTIndex].nPMTPID == 0)
			break;
		if (v->pat.pmt[nPMTIndex].nProgramNumber == 0)
			continue;
		if (forwarder->fp[nPMTIndex]->fEnabled)
		{
			sendto(forwarder->SAPsock,
							   forwarder->fp[nPMTIndex]->bSAPMessage,
							   forwarder->fp[nPMTIndex]->nSAPMessageLength,
							   0,
							   (struct sockaddr *)&forwarder->SAPserver,
							   sizeof(forwarder->SAPserver) );
		}
	}
	if (forwarder->fMuxEnabled)
	{
		sendto(forwarder->SAPsock,
						   forwarder->fp[MAX_PAT_ENTRIES]->bSAPMessage,
						   forwarder->fp[MAX_PAT_ENTRIES]->nSAPMessageLength,
						   0,
						   (struct sockaddr *)&forwarder->SAPserver,
						   sizeof(forwarder->SAPserver) );
	}
}

DWORD WINAPI UDPForwarderThread(LPVOID lpv)
{
	BYTE tspacket[188];
	DWORD dwLastTick;

	forwarder->fThreadTerminated = FALSE;
	BuildForwarderPIDList();
	GenerateForwarderPATs();
	OpenForwarderSockets();
	SAPSetup();
	
	dwLastTick = GetTickCount() - 10000;
	while (ReadFromForwarderPipe(v->hForwarderReadPipe, tspacket, 188) != 0)
	{
		BOOL fSleep = FALSE;
		int nPID = (tspacket[1] << 8 | tspacket[2]) & 0x1fff;
		
		ForwardPIDData(tspacket, nPID);
		EnterCriticalSection(&forwarder->csPipeBytes);
		forwarder->nPipeBytes -= 188;
		if (forwarder->nPipeBytes == 0)
			fSleep = TRUE;
		LeaveCriticalSection(&forwarder->csPipeBytes);
		if (fSleep)
		{
			Sleep(25);
			if (GetTickCount() >= dwLastTick + 2500)
			{
				dwLastTick = GetTickCount();
				SendSAP();
			}
		}		
	}

	SAPShutdown();
	ShutdownForwarderSockets();

	forwarder->fThreadTerminated = TRUE;
	OutputDebugString("TSReader: UDPForwarderThread() exit\n");
	return 0;
}


void AllocateOrDeAllocateForwarderProgramBuffers(BOOL fAllocate)
{
	int nPMTIndex;

	for (nPMTIndex = 0; nPMTIndex < MAX_PAT_ENTRIES; nPMTIndex++)
	{
		if (v->pat.pmt[nPMTIndex].nPMTPID == 0)
			break;
		if (v->pat.pmt[nPMTIndex].nProgramNumber == 0)
			continue;
		if (fAllocate)
			forwarder->fp[nPMTIndex] = LocalAlloc(LPTR, sizeof(FORWARDERPROGRAM));
		else
			LocalFree(forwarder->fp[nPMTIndex]);
	}

	if (fAllocate)
		forwarder->fp[MAX_PAT_ENTRIES] = LocalAlloc(LPTR, sizeof(FORWARDERPROGRAM));
	else
		LocalFree(forwarder->fp[MAX_PAT_ENTRIES]);
}

void DrawForwardingStream(int nPMTIndex, int nSocket, HDC hDC, int * nCurrentY, int nColumn2, int nColumn3, SIZE * sizeText, RECT * rc)
{
	char szDestinationAddr[128];
	char szTemp[1024];

	if (nPMTIndex == MAX_PAT_ENTRIES)
	{
		lstrcpy(szTemp, "Entire Mux");
	}
	else
	{
		wsprintf(szTemp, "%d", v->pat.pmt[nPMTIndex].nProgramNumber);
		if (v->pChannelData[v->pat.pmt[nPMTIndex].nProgramNumber] != NULL)
		{
			if (lstrlen(v->pChannelData[v->pat.pmt[nPMTIndex].nProgramNumber]->szShortName))
			{
				lstrcat(szTemp, " ");
				lstrcat(szTemp, v->pChannelData[v->pat.pmt[nPMTIndex].nProgramNumber]->szShortName);
			}
		}
	}
	TextOut(hDC, 3, *nCurrentY, szTemp, lstrlen(szTemp));

	wsprintf(szDestinationAddr, "%d.%d.%d.%d",
	         forwarder->fp[nPMTIndex]->server[nSocket].sin_addr.S_un.S_un_b.s_b1,
	         forwarder->fp[nPMTIndex]->server[nSocket].sin_addr.S_un.S_un_b.s_b2,
	         forwarder->fp[nPMTIndex]->server[nSocket].sin_addr.S_un.S_un_b.s_b3,
	         forwarder->fp[nPMTIndex]->server[nSocket].sin_addr.S_un.S_un_b.s_b4);
	wsprintf(szTemp, "%s:%d", szDestinationAddr, ntohs(forwarder->fp[nPMTIndex]->server[nSocket].sin_port));
	TextOut(hDC, nColumn2, *nCurrentY, szTemp, lstrlen(szTemp));

	if (forwarder->fp[nPMTIndex]->dAllBytesWritten / 1024.0 / 1024.0 / 1024.0 > 1000)
		sprintf(szTemp, "%.3f TB", forwarder->fp[nPMTIndex]->dAllBytesWritten / 1024.0 / 1024.0 / 1024.0 / 1024.0);
	else if (forwarder->fp[nPMTIndex]->dAllBytesWritten / 1024.0 / 1024.0 > 1000)
		sprintf(szTemp, "%.3f GB", forwarder->fp[nPMTIndex]->dAllBytesWritten / 1024.0 / 1024.0 / 1024.0);
	else			
		sprintf(szTemp, "%.1f MB", forwarder->fp[nPMTIndex]->dAllBytesWritten/ 1024.0 / 1024.0);
	TextOut(hDC, nColumn3, *nCurrentY, szTemp, lstrlen(szTemp));
	
	MoveToEx(hDC, 1, *nCurrentY + sizeText->cy + 1, NULL);
	LineTo(hDC, rc->right - 1, *nCurrentY + sizeText->cy + 1);
	*nCurrentY += sizeText->cy + 3;
}

INT_PTR CALLBACK ForwarderRunDialogProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg)
	{
	case WM_INITDIALOG:
		{
			HDC hDC = GetDC(hWnd);
			char szTemp[MAX_PATH];

			forwarder->hGreenBrush = CreateSolidBrush(RGB(0x00, 0xff, 0x00));
			forwarder->hRedBrush = CreateSolidBrush(RGB(0xff, 0x00, 0x00));
			forwarder->hGreenPen = CreatePen(PS_SOLID, 1, RGB(0x00, 0xff, 0x00));
			forwarder->hSmallTextFont = CreateFont(-MulDiv(95, GetDeviceCaps(hDC, LOGPIXELSY), 720),
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
			forwarder->hSmallBoldTextFont = CreateFont(-MulDiv(95, GetDeviceCaps(hDC, LOGPIXELSY), 720),
									   0,
									   0,
									   0,
									   700,
									   FALSE,
									   FALSE,
									   FALSE,
									   ANSI_CHARSET,
									   OUT_DEFAULT_PRECIS,
									   CLIP_DEFAULT_PRECIS,
									   ANTIALIASED_QUALITY,
									   FF_DONTCARE | VARIABLE_PITCH,
									   "Arial");	
			ReleaseDC(hWnd, hDC);

			if (lstrlen(v->szProfileName))
				wsprintf(szTemp, "Forwarder Status - %s", v->szProfileName);
			else
				lstrcpy(szTemp, "Forwarder Status - Default");
			SendMessage(hWnd, WM_SETTEXT, 0, (LPARAM)szTemp);

			if (v->nForwarderWindowW && v->nForwarderWindowH)
				SetWindowPos(hWnd, NULL, 
				             v->nForwarderWindowX, v->nForwarderWindowY,
							 v->nForwarderWindowW, v->nForwarderWindowH,
							 0);
			SetTimer(hWnd, 1, 1000, NULL);
		}
		break;
	case WM_DESTROY:
		{
			RECT rcForwarderWindow;

			GetWindowRect(hWnd, &rcForwarderWindow);
			v->nForwarderWindowX = rcForwarderWindow.left;
			v->nForwarderWindowY = rcForwarderWindow.top;
			v->nForwarderWindowW = rcForwarderWindow.right - rcForwarderWindow.left;
			v->nForwarderWindowH = rcForwarderWindow.bottom - rcForwarderWindow.top;
			KillTimer(hWnd, 1);

			DeleteObject(forwarder->hSmallTextFont);
			DeleteObject(forwarder->hSmallBoldTextFont);
			DeleteObject(forwarder->hGreenBrush);
			DeleteObject(forwarder->hRedBrush);
			DeleteObject(forwarder->hGreenPen);
		}
		break;
	case WM_CLOSE:
		PostMessage(v->hWndMainWindow, WM_COMMAND, ID_FORWARD_FORWARDTOUDP, 0);
		break;
	case WM_SIZE:
		InvalidateRect(hWnd, NULL, FALSE);
		break;
	case WM_PAINT:
		{
			int nPMTIndex;
			int nCurrentY = 1;
			int nColumn2, nColumn3;
			HDC hDC, hRealDC;
			HBITMAP memBM;
			PAINTSTRUCT ps;
			RECT rc;
			SIZE sizeText;
			static char szChannel[] = {"Program"};
			static char szDestination[] = {"Destination"};
			static char szSent[] = {"Data Sent"};
						
			GetClientRect(hWnd, &rc);

			nColumn2 = rc.right / 2;
			nColumn3 = nColumn2 + (rc.right / 4);
			hRealDC = BeginPaint(hWnd, &ps);
			hDC = CreateCompatibleDC(hRealDC);
			memBM = CreateCompatibleBitmap (hRealDC, rc.right, rc.bottom);
			SelectObject(hDC, memBM);
			SetBkMode(hDC, TRANSPARENT);
			SelectObject(hDC, forwarder->hGreenPen);

			SetTextColor(hDC, RGB(0, 255, 0));

			SelectObject(hDC, forwarder->hSmallBoldTextFont);					
			GetTextExtentPoint(hDC, "X", 1, &sizeText);
			TextOut(hDC, 3, nCurrentY, szChannel, lstrlen(szChannel));
			TextOut(hDC, nColumn2, nCurrentY, szDestination, lstrlen(szDestination));
			TextOut(hDC, nColumn3, nCurrentY, szSent, lstrlen(szSent));
			MoveToEx(hDC, 1, nCurrentY + sizeText.cy, NULL);
			LineTo(hDC, rc.right - 1, nCurrentY + sizeText.cy);
			nCurrentY += sizeText.cy + 2;

			SelectObject(hDC, forwarder->hSmallTextFont);					
			GetTextExtentPoint(hDC, "X", 1, &sizeText);
			for (nPMTIndex = 0; nPMTIndex < MAX_PAT_ENTRIES; nPMTIndex++)
			{
				if (v->pat.pmt[nPMTIndex].nPMTPID == 0)
					break;
				if (v->pat.pmt[nPMTIndex].nProgramNumber == 0)
					continue;
				if (forwarder->fp[nPMTIndex]->fEnabled == TRUE)
				{
					int nSocket;

					for (nSocket = 0; nSocket < MAX_FORWARDER_SOCKETS; nSocket++)
					{
						if (forwarder->fp[nPMTIndex]->sock[nSocket] == 0)
							break;
						DrawForwardingStream(nPMTIndex, nSocket, hDC, &nCurrentY, nColumn2, nColumn3, &sizeText, &rc);
					}
				}
			}
			if (forwarder->fMuxEnabled)
			{
				int nSocket;

				for (nSocket = 0; nSocket < MAX_FORWARDER_SOCKETS; nSocket++)
				{
					if (forwarder->fp[MAX_PAT_ENTRIES]->sock[nSocket] == 0)
						break;
					DrawForwardingStream(MAX_PAT_ENTRIES, nSocket, hDC, &nCurrentY, nColumn2, nColumn3, &sizeText, &rc);
				}
			}

			MoveToEx(hDC, 0, 0, NULL);
			LineTo(hDC, rc.right - 1, 0);
			LineTo(hDC, rc.right - 1, nCurrentY - 2);
			LineTo(hDC, 0, nCurrentY - 2);
			LineTo(hDC, 0, 0);

			MoveToEx(hDC, nColumn2 - 2, 0, NULL);
			LineTo(hDC, nColumn2 - 2, nCurrentY - 2);
			MoveToEx(hDC, nColumn3 - 2, 0, NULL);
			LineTo(hDC, nColumn3 - 2, nCurrentY - 2);

			BitBlt(hRealDC, 0, 0, rc.right, rc.bottom, hDC, 0, 0, SRCCOPY);
			DeleteObject(memBM);
			DeleteDC(hDC);
			EndPaint(hWnd, &ps);
		}
		break;
	case WM_TIMER:
		InvalidateRect(hWnd, NULL, FALSE);
		break;
	}
	return FALSE;
}

BOOL StartUDPForwarder(HWND hWnd)
{
	forwarder = LocalAlloc(LPTR, sizeof(FORWARDER));
	AllocateOrDeAllocateForwarderProgramBuffers(TRUE);
	if (DialogBox(v->hInstance, MAKEINTRESOURCE(IDD_FORWARDER_SETUP), hWnd, ForwarderSetupDlgProc) == TRUE)
	{
		HANDLE hThread;
		DWORD dwThreadID;

		dwMulticastAddress = inet_addr("224.0.0.0");
		dwMulticastAddressMask = inet_addr("240.0.0.0");

		InitializeCriticalSection(&forwarder->csPipeBytes);
		CreatePipe(&v->hForwarderReadPipe, &v->hForwarderWritePipe, NULL, 1024 * 1024 * 15);
		hThread = CreateThread(NULL, 0, UDPForwarderThread, (LPVOID)0, 0, &dwThreadID);
		CloseHandle(hThread);
		forwarder->hWndStatus = CreateDialog(v->hInstance, MAKEINTRESOURCE(IDD_FORWARDER_RUN), hWnd, ForwarderRunDialogProc);
		ShowWindow(forwarder->hWndStatus, SW_SHOW);		
		v->fForwarderEnabled = TRUE;
		return TRUE;
	}

	AllocateOrDeAllocateForwarderProgramBuffers(FALSE);
	LocalFree(forwarder);
	return FALSE;
}

BOOL StopUDPForwarder(HWND hWnd)
{
	v->fForwarderEnabled = FALSE;

	CloseHandle(v->hForwarderWritePipe);
	while (forwarder->fThreadTerminated == FALSE)
		Sleep(50);

	DestroyWindow(forwarder->hWndStatus);
	DeleteCriticalSection(&forwarder->csPipeBytes);

	AllocateOrDeAllocateForwarderProgramBuffers(FALSE);
	LocalFree(forwarder);

	return TRUE;
}

void ForwardProgramData(BYTE * pBuffer, int nLength)
{
	if (v->fForwarderEnabled)
	{
		DWORD dwWritten;
	
		WriteFile(v->hForwarderWritePipe, pBuffer, nLength, &dwWritten, NULL);
		if (dwWritten != (DWORD)nLength)
		{
			OutputDebugString("*************FORWARDER BUFFER WRITE PROBLEM**************\n");
		}
		EnterCriticalSection(&forwarder->csPipeBytes);
		forwarder->nPipeBytes += dwWritten;
		LeaveCriticalSection(&forwarder->csPipeBytes);
		v->nForwarderByteCounter += dwWritten;

	}

	if (v->fwd.nForwarderModulesActive)
	{
		int nIndex;
		for (nIndex = 0; nIndex < MAX_FWD_DLLS; nIndex++)
		{
			if (v->fwd.fActive[nIndex])
			{
				DWORD dwWritten;
				WriteFile(v->fwd.hWritePipe[nIndex], pBuffer, nLength, &dwWritten, NULL);
				v->nForwarderByteCounter += dwWritten;
			}
		}
	}

	if (v->nForwarderByteCounter > 1024 * 1024)
	{
		v->nForwarderByteCounter -= 1024 * 1024;
		v->nForwarderActivityPosition++;
		if (v->nForwarderActivityPosition> 3)
			v->nForwarderActivityPosition = 0;
	}
}

void PBRSScramble(int nIndex, BYTE * pPacket)
{
	BYTE i;

	for (i = 0; i < 188; i++)
	{
		BYTE j;
		register BYTE nInputByte = pPacket[i];
		register BYTE nOutputByte = 0;
		
		if (nInputByte == 0xb8 && !i)
		{
			v->fwd.nPBRSReg[nIndex] = 0x00a9;	// reset the PRBS register
			continue;			// don't bump PBRS for first sync
		}
		
		for (j = 7; j != 0xff; j--)
		{
			register BYTE nBit0;
			register BYTE nExtractBit;
			register BYTE nInputBit;
			register BYTE nOutputBit;

			nExtractBit = 1 << j;
			nInputBit = nInputByte & nExtractBit;
			if (nInputBit)
				nInputBit = 1;
			nBit0 = (v->fwd.nPBRSReg[nIndex] & 0x4000) >> 14 ^ (v->fwd.nPBRSReg[nIndex] & 0x2000) >> 13;
			v->fwd.nPBRSReg[nIndex] = ((v->fwd.nPBRSReg[nIndex] << 1) & 0x7ffe) | nBit0;			
			nOutputBit = nInputBit ^ nBit0;
			if (nOutputBit)
				nOutputByte |= nExtractBit;
		}

		if (i)	// don't scramble syncs
			pPacket[i] = nOutputByte; 
	}	
}

int GetInterLeaverInputPosition(int nWritePointer)
{
	int nResult = 0;

	while (--nWritePointer)
		nResult += nWritePointer * 17;

	return nResult;	
}

void InterleavePacket(int nIndex, BYTE * pOutputPacket, BYTE * pInputPacket)
{
	BYTE i;

	for (i = 0; i < 204; i++)
	{
		if (v->fwd.nInterleaveRow[nIndex] == 0)
			*(pOutputPacket++) = *(pInputPacket++);
		else
		{
			int nInputPos = GetInterLeaverInputPosition(v->fwd.nInterleaveRow[nIndex]);
			int nCellLength = v->fwd.nInterleaveRow[nIndex] * 17;
			int nOutputPos = nInputPos + nCellLength - 1;

			*(pOutputPacket++) = v->fwd.bInterleaver[nIndex][nOutputPos];
			memmove(&v->fwd.bInterleaver[nIndex][nInputPos + 1], &v->fwd.bInterleaver[nIndex][nInputPos], nCellLength - 1);
			v->fwd.bInterleaver[nIndex][nInputPos] = *(pInputPacket++);
		}
		v->fwd.nInterleaveRow[nIndex] = (v->fwd.nInterleaveRow[nIndex] + 1) % 12;
	}
}

DWORD WINAPI ForwarderModuleThread(LPVOID lpv)
{
	int nIndex = (int)(LONG_PTR)lpv;
	BYTE packet_buffer[256];
	BYTE * pPacket = &packet_buffer[51];

	v->fwd.fThreadTerminated[nIndex] = FALSE;
	if (v->nPacketSizeFlag == FORWARDER_PACKET_204_RS)
	{
		int symsize = 8;
		int gfpoly = 0x11d;
		int fcr = 1;
		int prim = 1;
		int nroots = 16;
		int pad = 0;

		v->fwd.RShandle[nIndex] = init_rs_char(symsize, gfpoly, fcr, prim, nroots, pad);
		if (v->fwd.RShandle[nIndex] != NULL)
		{
			memset(v->fwd.bInterleaver[nIndex], 0xff, sizeof(v->fwd.bInterleaver[nIndex]));
			v->fwd.nSyncCounter[nIndex] = 0;
			v->fwd.nInterleaveRow[nIndex] = 0;
			v->fwd.nPBRSReg[nIndex] = 0;
		}
	}

	memset(packet_buffer, 0, sizeof(packet_buffer));
	while (!v->fwd.fTerminateThread[nIndex])
	{
		int nCount = ReadFromForwarderPipe(v->fwd.hReadPipe[nIndex], pPacket, 188);
		if (nCount)
		{
			switch(v->nPacketSizeFlag)
			{
			case FORWARDER_PACKET_188:
				v->fwd.functions[nIndex].Fwd_Data(pPacket, 188);
				break;
			case FORWARDER_PACKET_204:
				v->fwd.functions[nIndex].Fwd_Data(pPacket, 204);
				break;
			case FORWARDER_PACKET_204_RS:
				{
					BYTE bInterleavedPacket[204];
					
					if (v->nPacketOptions & FORWARDER_PACKET_SYNCINVERT)
					{
						if (!v->fwd.nSyncCounter[nIndex])
							pPacket[0] = 0xb8;
						v->fwd.nSyncCounter[nIndex] = (v->fwd.nSyncCounter[nIndex] + 1) % 8;
					}
					
					if (v->nPacketOptions & FORWARDER_PACKET_RANDOMIZER)
						PBRSScramble(nIndex, &packet_buffer[51]);

					// RS encode the packet
					encode_rs_char(v->fwd.RShandle[nIndex], packet_buffer, &packet_buffer[51 + 188]);
					
					if (v->nPacketOptions & FORWARDER_PACKET_INTERLEAVE)
					{						
						InterleavePacket(nIndex, bInterleavedPacket, &packet_buffer[51]);
						v->fwd.functions[nIndex].Fwd_Data(bInterleavedPacket, 204);
					}
					else
					{
						v->fwd.functions[nIndex].Fwd_Data(pPacket, 204);
					}
				}
				break;
			}
		}
	}

	if (v->fwd.RShandle[nIndex] != NULL)
	{
		free_rs_char(v->fwd.RShandle[nIndex]);
		v->fwd.RShandle[nIndex] = NULL;
	}

	v->fwd.functions[nIndex].Fwd_DeInit();

	v->fwd.fTerminateThread[nIndex] = FALSE;
	v->fwd.fThreadTerminated[nIndex] = TRUE;

	return 0;
}


void ForwarderModuleStartStop(HWND hWnd, int nIndex)
{
	if (v->fwd.fActive[nIndex])
	{
		v->fwd.fTerminateThread[nIndex] = TRUE;
		CloseHandle(v->fwd.hWritePipe[nIndex]);
		while (v->fwd.fThreadTerminated[nIndex] == TRUE)
			Sleep(50);
		v->fwd.fThreadTerminated[nIndex] = FALSE;

		v->fwd.fActive[nIndex] = FALSE;
		CheckMenuItem(GetMenu(hWnd), ID_FORWARD_DLL_BASE + nIndex, MF_BYCOMMAND | MF_UNCHECKED);
		v->fwd.nForwarderModulesActive--;
	}
	else
	{
		int nPacketLength = 188;
		int nBitRate = 0;

		if (v->nPacketSizeFlag != FORWARDER_PACKET_188)
			nPacketLength = 204;

		if (v->dDisplayMuxRate > 0)
		{
			EnterCriticalSection(&v->ss.csPIDCounter);
			nBitRate = (int)((v->dDisplayMuxRate / (double)v->nMuxRateCounter) * 8.0);
			LeaveCriticalSection(&v->ss.csPIDCounter);
			if (nPacketLength != 188)
			{
				double dRSRate = (double)nBitRate * (204.0/188.0);
				nBitRate = (int)dRSRate;
			}
		}
		else
		{
			MessageBox(hWnd, "TSReader hasn't calculated a bitrate for this mux so it cannot be forwarded.", gszAppName, MB_ICONSTOP);
			return;
		}

		if (v->fwd.functions[nIndex].Fwd_Init(hWnd, nPacketLength, nBitRate) == TRUE)
		{
			HANDLE hThread;
			DWORD dwThreadID;

			InitializeCriticalSection(&v->fwd.csPipeBytes[nIndex]);
			CreatePipe(&v->fwd.hReadPipe[nIndex], &v->fwd.hWritePipe[nIndex], NULL, 1024 * 1024 * 15);
			hThread = CreateThread(NULL, 0, ForwarderModuleThread, (LPVOID)(LONG_PTR)nIndex, 0, &dwThreadID);
			CloseHandle(hThread);

			CheckMenuItem(GetMenu(hWnd), ID_FORWARD_DLL_BASE + nIndex, MF_BYCOMMAND | MF_CHECKED);
			v->fwd.fActive[nIndex] = TRUE;
			v->fwd.nForwarderModulesActive++;
		}
	}
}
