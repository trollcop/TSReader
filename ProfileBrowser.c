#ifdef PRO

#include <windows.h>
#include <commctrl.h>
#include <ole2.h>
#include <shlguid.h>
#include <shlobj.h>

#include "TSReader.h"
#include "resource.h"

extern PVARIABLES v;
extern char gszAppName[];
extern char szLiteWarning[];
extern char gszKeyName[];
extern char gszMainClass[];
char gszProfileClassName[] = {"TSReader.Profile"};
char gszProfilesKeyName[] = {"Software\\COOL.STF"};

typedef struct _tagProfileList
{
	char szProfileName[MAX_PATH];
	char szSourceName[MAX_PATH];
	int nDeviceNumber;
} PROFILELIST, *PPROFILELIST;

PPROFILELIST pl;

// Forward declarations
BOOL CALLBACK AboutDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
void CursorNormal();
void CursorWait(HWND hWnd);
int PopulateSourceList(HWND hDlg, HWND hWndLV);
void GetSourceDispInfo(LV_DISPINFO *pnmv);
void LoadSettings();
void SaveSettings();

// Stuff in settings.c
BOOL CALLBACK SettingsDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);

// In TSReader.c
BOOL CALLBACK CheckNewVersionDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);

// Code
void LoadProfileSettings()
{
	DWORD dwDisposition;
	DWORD dwDataSize;
	DWORD dwType;
	LONG lKey;
	HKEY hkMainReg;
	char szTemp[128] = {0};
	char szKeyName[MAX_PATH];

	lstrcpy(szKeyName, gszKeyName);
	lstrcat(szKeyName, "\\Profiles");

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
		if (dwDisposition != REG_CREATED_NEW_KEY)
		{
			dwDataSize = sizeof(v->fProfilesMaximized);
			RegQueryValueEx(hkMainReg, "Maximized", NULL, &dwType, (BYTE *)&v->fProfilesMaximized, &dwDataSize);	

			// Try reading position/size
			dwDataSize = sizeof(v->nInitialProfileXPos);
			RegQueryValueEx(hkMainReg, "XPosition", NULL, &dwType, (BYTE *)&v->nInitialProfileXPos, &dwDataSize);	
			dwDataSize = sizeof(v->nInitialProfileYPos);
			RegQueryValueEx(hkMainReg, "YPosition", NULL, &dwType, (BYTE *)&v->nInitialProfileYPos, &dwDataSize);	
			dwDataSize = sizeof(v->nInitialProfileXSize);
			RegQueryValueEx(hkMainReg, "XSize", NULL, &dwType, (BYTE *)&v->nInitialProfileXSize, &dwDataSize);	
			dwDataSize = sizeof(v->nInitialProfileYSize);
			RegQueryValueEx(hkMainReg, "YSize", NULL, &dwType, (BYTE *)&v->nInitialProfileYSize, &dwDataSize);	
			if ( (v->nInitialProfileXPos < 0) || (v->nInitialProfileYPos < 0) )
				v->nInitialProfileXPos = v->nInitialProfileYPos = v->nInitialProfileXSize = v->nInitialProfileYSize = CW_USEDEFAULT;

			dwDataSize = sizeof(v->nSortProfileColumn);
			RegQueryValueEx(hkMainReg, "SortColumn", NULL, &dwType, (BYTE *)&v->nSortProfileColumn, &dwDataSize);	
			dwDataSize = sizeof(v->fProfileColumnAscending);
			RegQueryValueEx(hkMainReg, "ColumnAscending", NULL, &dwType, (BYTE *)&v->fProfileColumnAscending, &dwDataSize);	
			dwDataSize = sizeof(v->nProfileListViewState);
			RegQueryValueEx(hkMainReg, "ListViewState", NULL, &dwType, (BYTE *)&v->nProfileListViewState, &dwDataSize);	

		}
		else
		{
			// First time
			v->nInitialProfileXPos = v->nInitialProfileYPos = v->nInitialProfileXSize = v->nInitialProfileYSize = CW_USEDEFAULT;
			v->fProfileColumnAscending = FALSE;
			v->nSortProfileColumn = 0;
			v->nProfileListViewState = ID_VIEW_PROFILE_DETAILS;
		}
		RegCloseKey(hkMainReg);
	}
}

void SaveProfileSettings(HWND hWnd)
{
	LONG lKey;
	HKEY hkMainReg;
	DWORD dwDisposition;
	char szKeyName[MAX_PATH];

	lstrcpy(szKeyName, gszKeyName);
	lstrcat(szKeyName, "\\Profiles");

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
		RECT rect;

		GetWindowRect(hWnd, &rect);
		if ( (rect.left > 0) && (rect.top > 0) )
		{
			if (v->fProfilesMaximized == FALSE)
			{
				int nSize;

				RegSetValueEx(hkMainReg, "XPosition", 0, REG_DWORD, (BYTE *)&rect.left, sizeof(DWORD));
				RegSetValueEx(hkMainReg, "YPosition", 0, REG_DWORD, (BYTE *)&rect.top, sizeof(DWORD));
				nSize = rect.right - rect.left;
				RegSetValueEx(hkMainReg, "XSize", 0, REG_DWORD, (BYTE *)&nSize, sizeof(DWORD));
				nSize = rect.bottom - rect.top;
				RegSetValueEx(hkMainReg, "YSize", 0, REG_DWORD, (BYTE *)&nSize, sizeof(DWORD));
			}
		}
		RegSetValueEx(hkMainReg, "Maximized", 0, REG_DWORD, (BYTE *)&v->fProfilesMaximized, sizeof(DWORD));
		RegSetValueEx(hkMainReg, "ListViewState", 0, REG_DWORD, (BYTE *)&v->nProfileListViewState, sizeof(DWORD));
		RegSetValueEx(hkMainReg, "SortColumn", 0, REG_DWORD, (BYTE *)&v->nSortProfileColumn, sizeof(DWORD));
		RegSetValueEx(hkMainReg, "ColumnAscending", 0, REG_DWORD, (BYTE *)&v->fProfileColumnAscending, sizeof(DWORD));

		RegCloseKey(hkMainReg);
	}
}

// ProfileListCompareFunc - sorts the list view control. It is a 
//     comparison function. 
// Returns a negative value if the first item should precede the 
//     second item, a positive value if the first item should 
//     follow the second item, and zero if the items are equivalent. 
// lParam1 and lParam2 - item data for the two items (in this 
//     case, pointers to application-defined MYITEM structures) 
// lParamSort - value specified by the LVM_SORTITEMS message 
//     (in this case, the index of the column to sort) 

int CALLBACK ProfileListCompareFunc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort) 
{ 
	int nIndex1, nIndex2;
	int iCmp;
	char szText1[MAX_PATH] = {0};
	char szText2[MAX_PATH] = {0};

	if (v->fProfileColumnAscending == TRUE)
	{
		nIndex1 = (int) lParam1; 
		nIndex2 = (int) lParam2; 
	}
	else
	{
		nIndex1 = (int) lParam2; 
		nIndex2 = (int) lParam1; 
	}

	switch(lParamSort)
	{
	case 0:		// sort on profile name
		{
			char * szProfileSeperator = strstr(pl[nIndex1].szProfileName, "-");
			if (szProfileSeperator == NULL)
				lstrcpy(szText1, "Default");
			else
				lstrcpy(szText1, szProfileSeperator + 1);

			szProfileSeperator = strstr(pl[nIndex2].szProfileName, "-");
			if (szProfileSeperator == NULL)
				lstrcpy(szText2, "Default");
			else
				lstrcpy(szText2, szProfileSeperator + 1);
		}
		break;
	case 1:
		{
			char * szSourcePtr;
			static char szSearchString[] = {"TSReader_"};

			szSourcePtr = strstr(pl[nIndex1].szSourceName, szSearchString);
			if (szSourcePtr != NULL)
				lstrcpy(szText1, szSourcePtr + sizeof(szSearchString) - 1);
			szSourcePtr = strstr(pl[nIndex2].szSourceName, szSearchString);
			if (szSourcePtr != NULL)
				lstrcpy(szText2, szSourcePtr + sizeof(szSearchString) - 1);
		}
		break;
	}

    // Compare the specified column. 
    iCmp = lstrcmp(szText1, szText2);
	return iCmp;
 
    // Return the result if nonzero, or compare the 
    // first column otherwise. 
    //return (iCmp != 0) ? iCmp : lstrcmp(pItem1->aCols[0], pItem2->aCols[0]); 
} 

void LoadProfiles(HWND hWnd)
{
	HKEY hkMainReg;
	DWORD dwDisposition;
	LONG lKey;

	if (pl != NULL)
		LocalFree(pl);
	v->nProfileMax = 256;
	v->nProfileCount = 0;
	pl = LocalAlloc(LPTR, sizeof(PROFILELIST) * v->nProfileMax);

	lKey = RegCreateKeyEx(HKEY_CURRENT_USER,
		                  gszProfilesKeyName,
						  0,
						  gszAppName,
					      REG_OPTION_NON_VOLATILE,
						  KEY_ALL_ACCESS,
						  NULL,
						  &hkMainReg,
						  &dwDisposition);
	if (lKey == ERROR_SUCCESS)
	{
		DWORD dwIndex = 0;
		
		while (TRUE)
		{
			char szFoundKeyName[MAX_PATH] = {0};

			if (RegEnumKey(hkMainReg, dwIndex, szFoundKeyName, sizeof(szFoundKeyName)) == ERROR_SUCCESS)
			{
				if (memcmp(szFoundKeyName, "TSReader", 8) == 0)
				{
					HKEY hkProfile;
					char szProfileKeyName[MAX_PATH];
					
					lstrcpy(szProfileKeyName, gszProfilesKeyName);
					lstrcat(szProfileKeyName, "\\");
					lstrcat(szProfileKeyName, szFoundKeyName);
					lKey = RegOpenKeyEx(HKEY_CURRENT_USER,
										szProfileKeyName,
										0,
										KEY_ALL_ACCESS,
										&hkProfile);
					if (lKey == ERROR_SUCCESS)
					{
						DWORD dwType, dwDataSize;
						HMODULE hSource;
						LV_ITEM lvi;

						lstrcpy(pl[v->nProfileCount].szProfileName, szFoundKeyName);
						dwDataSize = sizeof(pl[v->nProfileCount].szSourceName);
						RegQueryValueEx(hkProfile, "SourceName", NULL, &dwType, (BYTE *)pl[v->nProfileCount].szSourceName, &dwDataSize);						
						pl[v->nProfileCount].nDeviceNumber = -1;	// signals no multiple devices
						// Load the source to see if this supports multiple devices
						hSource = LoadLibrary(pl[v->nProfileCount].szSourceName);
						if (hSource != NULL)
						{
							BOOL (* GetDescription) (char * szDescription, char * szCommandLineParameters, BOOL * fCanBeStopped, int * nMaxPIDs, DWORD * dwCapabilities);
							GetDescription = (td_GetDescription)GetProcAddress(hSource, "TSReader_GetDescription");
							if (GetDescription != NULL)
							{
								DWORD dwSourceCapabilities;

								GetDescription(NULL, NULL, NULL, NULL, &dwSourceCapabilities);
								if (dwSourceCapabilities & CAPABILITIES_MULTICARD)
								{
									dwDataSize = sizeof(pl[v->nProfileCount].nDeviceNumber);
									RegQueryValueEx(hkProfile, "SourceIndex", NULL, &dwType, (BYTE *)&pl[v->nProfileCount].nDeviceNumber, &dwDataSize);
								}

							}
							FreeLibrary(hSource);
						}
					
						memset(&lvi, 0, sizeof(lvi));
						lvi.state = 0; 
						lvi.stateMask = 0; 
						lvi.pszText = LPSTR_TEXTCALLBACK;   // app. maintains text 
						lvi.mask = LVIF_TEXT | LVIF_PARAM | LVIF_STATE; 
						lvi.iItem = v->nProfileCount; 
						lvi.iSubItem = 0; 
						lvi.lParam = (LPARAM) v->nProfileCount;    // item data 
						ListView_InsertItem(v->hWndProfileLV, &lvi);
						v->nProfileCount++;
						if (v->nProfileCount == v->nProfileMax)
						{
							MessageBox(hWnd, "Out of space for profiles - email support@tsreader.co.uk", gszAppName, MB_ICONSTOP);
							break;
						}
						RegCloseKey(hkProfile);
					}
				}
			}
			else
				break;
			dwIndex++;
		};

		RegCloseKey(hkMainReg);		
	}
	ListView_SortItems(v->hWndProfileLV, ProfileListCompareFunc, (LPARAM)v->nSortProfileColumn); 	
}

void GetProfileDispInfo(LV_DISPINFO *pnmv) 
{ 
    // Provide the item or subitem's text, if requested. 
    if (pnmv->item.mask & LVIF_TEXT)
	{ 
        int nProfileIndex = (int)(pnmv->item.lParam);
		switch(pnmv->item.iSubItem)
		{
		case 0:	// profile name
			{
				char * szProfileSeperator = strstr(pl[nProfileIndex].szProfileName, "-");
				if (szProfileSeperator == NULL)
					lstrcpy(pnmv->item.pszText, "Default");
				else
					lstrcpy(pnmv->item.pszText, szProfileSeperator + 1);
			}
			break;
		case 1:	// source module
			{
				char * szSourcePtr;
				static char szSearchString[] = {"TSReader_"};

				szSourcePtr = strstr(pl[nProfileIndex].szSourceName, szSearchString);
				if (szSourcePtr != NULL)
					lstrcpy(pnmv->item.pszText, szSourcePtr + sizeof(szSearchString) - 1);
				else
					pnmv->item.pszText[0] = '\0';
			}
			break;
		case 2:	// device no.
			if (pl[nProfileIndex].nDeviceNumber == -1)
				pnmv->item.pszText[0] = '\0';
			else
				wsprintf(pnmv->item.pszText, "%d", pl[nProfileIndex].nDeviceNumber);
			break;
		case 3: // currently active
			{
				HWND hWndOtherTSReader;
				char szProfileName[256];
				char szTemp[256];

				char * szProfileSeperator = strstr(pl[nProfileIndex].szProfileName, "-");
				if (szProfileSeperator == NULL)
					szProfileName[0] = '\0';
				else
					lstrcpy(szProfileName, szProfileSeperator + 1);

				lstrcpy(szTemp, gszMainClass);
				if (lstrlen(szProfileName))
				{
					lstrcat(szTemp, ".");
					lstrcat(szTemp, szProfileName);
				}
				hWndOtherTSReader = FindWindow(szTemp, NULL);
				if (hWndOtherTSReader != NULL)
					lstrcpy(pnmv->item.pszText, "Yes");
				else
					pnmv->item.pszText[0] = '\0';
			}
			break;
		}
	}
}

void SetProfileLVState(HWND hWnd)
{
    DWORD dwStyle = GetWindowLong(v->hWndProfileLV, GWL_STYLE); 
	DWORD dwView;
	HMENU hMenu = GetMenu(hWnd);

	CheckMenuRadioItem(hMenu, ID_VIEW_LARGEICONS, ID_VIEW_PROFILE_DETAILS, v->nProfileListViewState, MF_BYCOMMAND);
	SendMessage(v->hWndProfileTB, TB_CHECKBUTTON, ID_VIEW_LARGEICONS, MAKELONG(FALSE, 0));
	SendMessage(v->hWndProfileTB, TB_CHECKBUTTON, ID_VIEW_SMALLICONS, MAKELONG(FALSE, 0));
	SendMessage(v->hWndProfileTB, TB_CHECKBUTTON, ID_VIEW_PROFILE_LIST, MAKELONG(FALSE, 0));
	SendMessage(v->hWndProfileTB, TB_CHECKBUTTON, ID_VIEW_PROFILE_DETAILS, MAKELONG(FALSE, 0));

	switch(v->nProfileListViewState)
	{
	case ID_VIEW_LARGEICONS:
		dwView = LVS_ICON;
		SendMessage(v->hWndProfileTB, TB_CHECKBUTTON, ID_VIEW_LARGEICONS, MAKELONG(TRUE, 0));
		break;
	case ID_VIEW_SMALLICONS:
		dwView = LVS_SMALLICON;
		SendMessage(v->hWndProfileTB, TB_CHECKBUTTON, ID_VIEW_SMALLICONS, MAKELONG(TRUE, 0));
		break;
	case ID_VIEW_PROFILE_LIST:
		dwView = LVS_LIST;
		SendMessage(v->hWndProfileTB, TB_CHECKBUTTON, ID_VIEW_PROFILE_LIST, MAKELONG(TRUE, 0));
		break;
	case ID_VIEW_PROFILE_DETAILS:
		dwView = LVS_REPORT;
		SendMessage(v->hWndProfileTB, TB_CHECKBUTTON, ID_VIEW_PROFILE_DETAILS, MAKELONG(TRUE, 0));
		break;
	}

    if ((dwStyle & LVS_TYPEMASK) != dwView) 
	{
        SetWindowLong(v->hWndProfileLV, GWL_STYLE, (dwStyle & ~LVS_TYPEMASK) | dwView); 
		ListView_Arrange(v->hWndProfileLV, LVA_DEFAULT);
	}
}

int GetSelectedProfile(char * szProfileName, int nMaxLength)
{
	int nCount, i;
	LV_ITEM lvItem;

	nCount = ListView_GetItemCount(v->hWndProfileLV);
	if (nCount > 0)
	{
		for (i = 0; i < nCount; i++)
		{
			lvItem.mask = LVIF_TEXT | LVIF_STATE | LVIF_PARAM;
			lvItem.iItem = i;
			lvItem.iSubItem = 0;
			lvItem.pszText = szProfileName;
			lvItem.cchTextMax = nMaxLength;
			lvItem.stateMask = LVIS_SELECTED;
			ListView_GetItem(v->hWndProfileLV, &lvItem);
			if ((lvItem.state & LVIS_SELECTED) > 0)
			{
				return i;
			}
		}
	}
	return -1;
}

// CreateLink - uses the shell's IShellLink and IPersistFile interfaces  
//   to create and store a shortcut to the specified object. 
// Returns the result of calling the member functions of the interfaces. 
// lpszPathObj - address of a buffer containing the path of the object 
// lpszPathLink - address of a buffer containing the path where the 
//   shell link is to be stored 
// lpszDesc - address of a buffer containing the description of the 
//   shell link 
// lpszWorkingDirectory - where the application is run
// lpszArguments - command line arguments

HRESULT CreateShortcut(LPCSTR lpszPathObj, LPSTR lpszPathLink, LPSTR lpszDesc, LPSTR lpszWorkingDirectory, LPSTR lpszArguments) 
{ 
    HRESULT hres; 
    IShellLink* psl; 
 
    CoInitialize(NULL);

	// Get a pointer to the IShellLink interface. 
    hres = CoCreateInstance(&CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER, &IID_IShellLink, &psl); 
    if (SUCCEEDED(hres))
	{ 
        IPersistFile* ppf; 
 
        // Set the path to the shortcut target, and add the 
        // description. 
        psl->lpVtbl->SetPath(psl, lpszPathObj); 
        psl->lpVtbl->SetDescription(psl, lpszDesc); 
		psl->lpVtbl->SetWorkingDirectory(psl, lpszWorkingDirectory);
		psl->lpVtbl->SetArguments(psl, lpszArguments);
				
       // Query IShellLink for the IPersistFile interface for saving the 
       // shortcut in persistent storage. 
        hres = psl->lpVtbl->QueryInterface(psl, &IID_IPersistFile, &ppf); 
 
        if (SUCCEEDED(hres))
		{ 
            WORD wsz[MAX_PATH]; 
 
            // Ensure that the string is ANSI. 
            MultiByteToWideChar(CP_ACP, 0, lpszPathLink, -1, wsz, MAX_PATH); 
 
            // Save the link by calling IPersistFile::Save. 
            hres = ppf->lpVtbl->Save(ppf, wsz, TRUE); 
            ppf->lpVtbl->Release(ppf); 
        } 
        psl->lpVtbl->Release(psl); 
    }
	
	CoUninitialize();
    return hres; 
} 

void CreateWin32DesktopShortcut(HWND hWnd, HINSTANCE hResources, char * szProfileName)
{
	int i;
	HKEY hCU;
	DWORD lpType;
	ULONG ulSize = MAX_PATH;
	char * szEXEPtr;
	char szDesktop[MAX_PATH];
	char szTSReaderFolder[MAX_PATH];
	char szTSReaderEXE[MAX_PATH];
	char szEXEArguments[MAX_PATH];

	// Get the directory for Windows Desktop. This is
	// stored in the Registry under HKEY_CURRENT_USER\Software\
	// Microsoft\Windows\CurrentVersion\Explorer\Shell Folders\Desktop.
	if (RegOpenKeyEx(HKEY_CURRENT_USER, 
					 "Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Shell Folders", 
					 0,KEY_QUERY_VALUE,
					 &hCU) == ERROR_SUCCESS)
	{
		RegQueryValueEx( hCU, "Desktop", NULL, &lpType, (unsigned char *)&szDesktop, &ulSize);
		RegCloseKey(hCU);
	}
	else
	{
		MessageBox(hWnd, "Unable to create desktop shortcut", gszAppName, MB_OK | MB_ICONWARNING);
		return;
	}

	// Setup the link name
	lstrcat(szDesktop, "\\");
	lstrcat(szDesktop, szProfileName);
	lstrcat(szDesktop, ".LNK");
	
	// Execution folder
	GetModuleFileName(hResources, szTSReaderFolder, sizeof(szTSReaderFolder));
	for (i = lstrlen(szTSReaderFolder); i >= 0; i--)
	{
		if (szTSReaderFolder[i] == '\\')
		{
			szTSReaderFolder[i] = '\0';
			szEXEPtr = &szTSReaderFolder[i + 1];
			break;
		}
	}
	lstrcpy(szTSReaderEXE, szTSReaderFolder);
	lstrcat(szTSReaderEXE, "\\");
	lstrcat(szTSReaderEXE, szEXEPtr);

	if (lstrcmp(szProfileName, "Default") == 0)
		szEXEArguments[0] = '\0';
	else
		wsprintf(szEXEArguments, "-L \"%s\"", szProfileName);
	CreateShortcut(szTSReaderEXE, szDesktop, szProfileName, szTSReaderFolder, szEXEArguments);
}	

LONG DeleteProfile(char * szProfileName)
{
	TCHAR szSourceKey[MAX_PATH];
	TCHAR szSubKeys[MAX_PATH];

	lstrcpy(szSourceKey, gszProfilesKeyName);
	lstrcat(szSourceKey, "\\TSReader-");
	lstrcat(szSourceKey, szProfileName);

	wsprintf(szSubKeys, "%s\\Archive", szSourceKey);
	RegDeleteKey(HKEY_CURRENT_USER, szSubKeys);
	wsprintf(szSubKeys, "%s\\Forwarder", szSourceKey);
	RegDeleteKey(HKEY_CURRENT_USER, szSubKeys);

	return RegDeleteKey(HKEY_CURRENT_USER, szSourceKey);
}

BOOL CopyProfile(TCHAR * szOldProfileName, TCHAR * szNewProfileName)
{
	TCHAR szSourceKey[MAX_PATH];
	TCHAR szDestKey[MAX_PATH];
	HKEY hSource, hDest;
	DWORD dwDisposition;
	int i;
	BOOL fRetVal = TRUE;
	REGSAM samDesired = KEY_ALL_ACCESS;

	lstrcpy(szSourceKey, gszProfilesKeyName);
	if (lstrlen(szOldProfileName) == 0)
		lstrcat(szSourceKey, "\\TSReader");
	else
	{
		lstrcat(szSourceKey, "\\TSReader-");
		lstrcat(szSourceKey, szOldProfileName);
	}

	lstrcpy(szDestKey, gszProfilesKeyName);
	lstrcat(szDestKey, "\\TSReader-");
	lstrcat(szDestKey, szNewProfileName);

	if (RegOpenKeyEx(HKEY_CURRENT_USER, 
					 szSourceKey,
					 0,
					 samDesired,
					 &hSource) == ERROR_SUCCESS)
	{
		if (RegCreateKeyEx(HKEY_CURRENT_USER,
						   szDestKey,
						   0,
						   TEXT("ZTERMClass"),
		                   REG_OPTION_NON_VOLATILE,
						   samDesired,
						   NULL,
						   &hDest,
						   &dwDisposition) == ERROR_SUCCESS)
		{
			for (i = 0; ; i++)
			{
				TCHAR szValueName[MAX_PATH];
				TCHAR szValue[MAX_PATH];
				DWORD dwValueName = sizeof(szValueName);
				DWORD dwValue = sizeof(szValue);
				DWORD dwKeyType;

				if (RegEnumValue(hSource,
								 i,
								 szValueName,
								 &dwValueName,
								 NULL,
								 &dwKeyType,
								 (unsigned char *)szValue,
								 &dwValue) == ERROR_SUCCESS)
				{
					RegSetValueEx(hDest,
						          szValueName,
								  0,
								  dwKeyType,
								  (unsigned char *)szValue,
								  dwValue);
				}
				else
					break;
			}
			RegCloseKey(hDest);
		}
		else
			fRetVal = FALSE;
		RegCloseKey(hSource);
	}
	else
		fRetVal = FALSE;

	return fRetVal;
}

LONG RenameProfile(HWND hWnd, TCHAR * szOldProfileName, TCHAR * szNewProfileName)
{
	if (CopyProfile(szOldProfileName, szNewProfileName) == FALSE)
	{
		MessageBox(hWnd, TEXT("Unable to rename profile"), gszAppName, MB_ICONSTOP);
		return 1;
	}
	return DeleteProfile(szOldProfileName);
}

void EnableOrDisableOK(HWND hDlg)
{
	BOOL fEnabled = FALSE;
	char szTemp[MAX_PATH] = {0};

	if (v->nSelectedSource != -1)
	{
		GetDlgItemText(hDlg, IDC_PROFILE_NEW_NAME, szTemp, sizeof(szTemp));
		if (lstrlen(szTemp))
		{
			strlwr(szTemp);
			if (strcmp(szTemp, "default") != 0)
				fEnabled = TRUE;
		}
	}
	EnableWindow(GetDlgItem(hDlg, IDOK), fEnabled);
}

BOOL CALLBACK NewProfileDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg)
	{
	case WM_INITDIALOG:
		{
			int nColumnPosition = 0;
			int nModuleIndex;
			HWND hWndSourceList = GetDlgItem(hDlg, IDC_PROFILE_NEW_LIST);
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
			lvc.cx = 265; 
			lstrcpy(szTemp, TEXT("Description"));
			ListView_InsertColumn(hWndSourceList, nColumnPosition++, &lvc);
			ListView_SetExtendedListViewStyle(hWndSourceList, LVS_EX_FULLROWSELECT);
			nModuleIndex = PopulateSourceList(hDlg, hWndSourceList);
			v->nSelectedSource = -1;

			v->szNewProfileSource[0] = '\0';
			v->szNewProfileName[0] = '\0';

			EnableOrDisableOK(hDlg);
			SetFocus(GetDlgItem(hDlg, IDC_PROFILE_NEW_NAME));
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
					int nIndex;
					
					GetDlgItemText(hDlg, IDC_PROFILE_NEW_NAME, v->szNewProfileName, sizeof(v->szNewProfileName));
					for (nIndex = 0; nIndex < v->nProfileCount; nIndex++)
					{
						char * szSeperator = strstr(pl[nIndex].szProfileName, "-");
						if (szSeperator != NULL)
						{
							if (lstrcmp(v->szNewProfileName, szSeperator + 1) == 0)
							{
								MessageBox(hDlg, "A profile by this name already exists", gszAppName, MB_ICONSTOP);
								SetFocus(GetDlgItem(hDlg, IDC_PROFILE_NEW_NAME));
								return FALSE;
							}
						}
					}

					lstrcpy(v->szNewProfileSource, v->sourcemodules[v->nSelectedSource].szFilename);
					EndDialog(hDlg, TRUE);
				}
				break;
			case IDCANCEL:
				EndDialog(hDlg, FALSE);
				break;
			}
			break;
		case EN_CHANGE:
			EnableOrDisableOK(hDlg);
			break;
		}
		break;
	case WM_NOTIFY: 
		switch (((LPNMHDR) lParam)->code)
		{ 
		case LVN_GETDISPINFO:
			{
				NMLVDISPINFO * pnmv = (NMLVDISPINFO *)lParam;
				GetSourceDispInfo((LV_DISPINFO *) lParam);
			}
			break;
		case LVN_ITEMCHANGED:
			{
				LPNMLISTVIEW pnmv = (LPNMLISTVIEW)lParam;
				if (pnmv->uNewState & LVIS_SELECTED)
				{
					v->nSelectedSource = pnmv->iItem;
					EnableOrDisableOK(hDlg);
				}
			}
			break;
		case NM_DBLCLK:
			{
				NMLVDISPINFO * pnmv = (NMLVDISPINFO *)lParam;
				PostMessage(hDlg, WM_COMMAND, IDOK, 0);
			}
			break;
		}
		break;
	}

	return FALSE;
}

void ProfileSheetContextMenu(HWND hWnd, LPARAM lParam)
{
	HMENU hMenu, hMenuTrackPopup;
	TCHAR szProfile[MAX_PATH];

	if (GetSelectedProfile(szProfile, sizeof(szProfile)) == -1)
		return;

	// Get the menu for the pop-up menu from the resource file. 
	hMenu = LoadMenu(v->hInstance, MAKEINTRESOURCE(IDR_PROFILES_POPUP)); 
	if (!hMenu) 
		return; 

	// Get the first submenu in it for TrackPopupMenu. 
	hMenuTrackPopup = GetSubMenu(hMenu, 0);

	if (lstrcmp(szProfile, "Default") == 0)
	{
		EnableMenuItem(hMenuTrackPopup, ID_FILE_DELETE, MF_GRAYED | MF_DISABLED | MF_BYCOMMAND);
		EnableMenuItem(hMenuTrackPopup, ID_IDRPROFILESPOPUP_RENAME, MF_GRAYED | MF_DISABLED | MF_BYCOMMAND);
	}

	// Draw the "floating" pop-up menu, and track it. 
	TrackPopupMenu(hMenuTrackPopup, 
				   TPM_LEFTALIGN | TPM_RIGHTBUTTON, 
				   LOWORD(lParam), HIWORD(lParam), 
				   0, hWnd, NULL);

	// Destroy the menu. 
	DestroyMenu(hMenu); 
}

BOOL OKToDestroyOtherWindows(HWND hWnd)
{
	int nProfile;
	BOOL fWindowsActive = FALSE;

	if (v->fProfileBrowserShellMode)
	{
		for (nProfile = 0; nProfile < v->nProfileCount; nProfile++)
		{
			HWND hWndOtherTSReader;
			char szTemp[256];
			char szProfileName[256];

			char * szProfileSeperator = strstr(pl[nProfile].szProfileName, "-");
			if (szProfileSeperator == NULL)
				szProfileName[0] = '\0';
			else
				lstrcpy(szProfileName, szProfileSeperator + 1);
			lstrcpy(szTemp, gszMainClass);
			if (lstrlen(szProfileName))
			{
				lstrcat(szTemp, ".");
				lstrcat(szTemp, szProfileName);
			}
			hWndOtherTSReader = FindWindow(szTemp, NULL);
			if (hWndOtherTSReader != NULL)
			{
				fWindowsActive = TRUE;
				break;
			}
		}

		if (fWindowsActive)
		{
			if (MessageBox(hWnd, "Closing the profile browser will close all current TSReader processes. OK to do this?", gszAppName, MB_ICONWARNING | MB_YESNO | MB_DEFBUTTON1) == IDNO)
			{
				return FALSE;
			}

			for (nProfile = 0; nProfile < v->nProfileCount; nProfile++)
			{
				HWND hWndOtherTSReader;
				char szTemp[256];
				char szProfileName[256];

				char * szProfileSeperator = strstr(pl[nProfile].szProfileName, "-");
				if (szProfileSeperator == NULL)
					szProfileName[0] = '\0';
				else
					lstrcpy(szProfileName, szProfileSeperator + 1);
				lstrcpy(szTemp, gszMainClass);
				if (lstrlen(szProfileName))
				{
					lstrcat(szTemp, ".");
					lstrcat(szTemp, szProfileName);
				}
				hWndOtherTSReader = FindWindow(szTemp, NULL);
				if (hWndOtherTSReader != NULL)
					SendMessage(hWndOtherTSReader, WM_CLOSE, 0, 0);
			}
		}
	}

	return TRUE;
}

LRESULT FAR PASCAL ProfileWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg)
	{
	case WM_CREATE:
		{
			int nColumnPosition = 0;
			//HICON hiconItem;
			LV_COLUMN lvc; 
			HICON hiconItem;
			REBARBANDINFO rbBand;
			char szTemp[128];
			static TBBUTTON tbButtons [] = 
			{
				{0, ID_FILE_NEW_PROFILE,     TBSTATE_ENABLED, TBSTYLE_BUTTON,     0L, 0},
				{1, ID_FILE_DELETE,          TBSTATE_ENABLED, TBSTYLE_BUTTON,     0L, 0},
				{2, ID_FILE_EDIT,            TBSTATE_ENABLED, TBSTYLE_BUTTON,     0L, 0},
				{0, 0,                       0,               TBSTYLE_SEP,        0L, 0},
				{3, ID_VIEW_LARGEICONS,      TBSTATE_ENABLED, TBSTYLE_CHECKGROUP, 0L, 0},
				{4, ID_VIEW_SMALLICONS,      TBSTATE_ENABLED, TBSTYLE_CHECKGROUP, 0L, 0},
				{5, ID_VIEW_PROFILE_LIST,    TBSTATE_ENABLED, TBSTYLE_CHECKGROUP, 0L, 0},
				{6, ID_VIEW_PROFILE_DETAILS, TBSTATE_ENABLED, TBSTYLE_CHECKGROUP, 0L, 0},
				{0, 0,                       0,               TBSTYLE_SEP,        0L, 0},
				{7, ID_HELP_ABOUT,           TBSTATE_ENABLED, TBSTYLE_BUTTON,     0L, 0}
			};

			pl = NULL;
			v->fContinueAfterProfileBrowser = FALSE;
			v->szCopyProfileName[0] = '\0';

			// Profile List View
			v->hWndProfileLV = CreateWindowEx(WS_EX_WINDOWEDGE,
									WC_LISTVIEW,
									NULL,
									WS_VISIBLE | WS_CHILD | WS_BORDER | LVS_REPORT | LVS_EDITLABELS | LVS_SINGLESEL,
									0, 0,
									100, 100,
									hWnd,
									NULL,
									v->hInstance,
									NULL);

			v->himlSmall = ImageList_Create(GetSystemMetrics(SM_CXSMICON), 
										 GetSystemMetrics(SM_CYSMICON), TRUE, 1, 1); 

			hiconItem = LoadImage(v->hInstance, MAKEINTRESOURCE(IDI_DVBSMALL_LOGO), IMAGE_ICON, 16, 16, 0); 
			ImageList_AddIcon(v->himlSmall, hiconItem); 
			DestroyIcon(hiconItem); 

			v->himlLarge = ImageList_Create(GetSystemMetrics(SM_CXICON), 
										 GetSystemMetrics(SM_CYICON), TRUE, 1, 1); 
			hiconItem = LoadIcon(v->hInstance, MAKEINTRESOURCE(IDI_DVBSMALL_LOGO)); 
			ImageList_AddIcon(v->himlLarge, hiconItem); 
			DestroyIcon(hiconItem); 

			// Assign the image list to the list view control. 
			ListView_SetImageList(v->hWndProfileLV, v->himlLarge, LVSIL_NORMAL); 
			ListView_SetImageList(v->hWndProfileLV, v->himlSmall, LVSIL_SMALL); 


			lvc.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM; 
			lvc.pszText = szTemp; 

			// Add the columns. 
			lvc.fmt = LVCFMT_LEFT; 
			lvc.cx = 180; 
			lstrcpy(szTemp, TEXT("Profile Name"));
			ListView_InsertColumn(v->hWndProfileLV, nColumnPosition++, &lvc); 

			lvc.fmt = LVCFMT_LEFT; 
			lvc.cx = 180; 
			lstrcpy(szTemp, TEXT("Source"));
			ListView_InsertColumn(v->hWndProfileLV, nColumnPosition++, &lvc); 

			lvc.fmt = LVCFMT_CENTER; 
			lvc.cx = 80; 
			lstrcpy(szTemp, TEXT("Device No."));
			ListView_InsertColumn(v->hWndProfileLV, nColumnPosition++, &lvc); 

			lvc.fmt = LVCFMT_CENTER; 
			lvc.cx = 110; 
			lstrcpy(szTemp, TEXT("Currently Active"));
			ListView_InsertColumn(v->hWndProfileLV, nColumnPosition++, &lvc); 

			// Create the rebar
			v->hWndProfileRB = CreateWindowEx(0L,
			 						REBARCLASSNAME,
									NULL,
									WS_VISIBLE | WS_BORDER | WS_CHILD | WS_CLIPCHILDREN |
									WS_CLIPSIBLINGS | CCS_NODIVIDER | CCS_NOPARENTALIGN |
									RBS_VARHEIGHT | RBS_BANDBORDERS,
									0,
									0,
									800,
									28,
									hWnd,
									(HMENU)NULL,
									v->hInstance,
									NULL );

			//Create the toolbar.
			v->hWndProfileTB = CreateToolbarEx(v->hWndProfileRB, 
									 WS_CHILD | TBSTYLE_FLAT | TBSTYLE_TOOLTIPS | WS_CLIPCHILDREN |
									 WS_CLIPSIBLINGS | CCS_NODIVIDER | CCS_NORESIZE | WS_VISIBLE,
									 1,							// toolbar ID
									 7,                         // number of bitmaps
									 v->hInstance,				// mod instance
									 IDB_PROFILE_TOOLBAR,					// resource ID for bitmap
									 tbButtons,					// address of buttons
									 10,							// number of buttons
									 16, 16,						// width & height of buttons
									 16, 16,						// width & height of bitmaps
									 sizeof (TBBUTTON));			// structure size

			// Insert a band for the standard toolbar
			memset(&rbBand, 0, sizeof(REBARBANDINFO));
			rbBand.cbSize = sizeof(REBARBANDINFO);
			rbBand.fMask = RBBIM_COLORS |					 // clrFore and clrBack are valid
						   RBBIM_CHILD |                     // hwndChild is valid
						   RBBIM_CHILDSIZE |                 // cxMinChild and cyMinChild are valid
						   RBBIM_STYLE;                      // fStyle is valid

			rbBand.clrFore = GetSysColor(COLOR_BTNTEXT);
			rbBand.clrBack = GetSysColor(COLOR_BTNFACE);
			rbBand.fStyle = RBBS_NOVERT |  // Do not display in vertical orientation.
 							RBBS_CHILDEDGE |
							RBBS_GRIPPERALWAYS;
			rbBand.hwndChild = v->hWndProfileTB;
			rbBand.cxMinChild = 187;
			rbBand.cyMinChild = 24;
			SendMessage(v->hWndProfileRB, RB_INSERTBAND, (WPARAM) -1, (LPARAM)(LPREBARBANDINFO)&rbBand);

			SetProfileLVState(hWnd);
			PostMessage(hWnd, WM_COMMAND, ID_VIEW_REFRESH, 0);

			//LoadProfiles(hWnd);
			SetFocus(v->hWndProfileLV);
		}
		break;
	case WM_SIZE:
		{
			int nWidth = LOWORD(lParam);  // width of client area 
			int nHeight = HIWORD(lParam); // height of client area 
			RECT rectRB;

			memset(&rectRB, 0, sizeof(rectRB));
			
			if (wParam == SIZE_MAXIMIZED)
				v->fProfilesMaximized = TRUE;
			else
				v->fProfilesMaximized = FALSE;

			if (v->hWndProfileRB != NULL)
			{
				GetClientRect(v->hWndProfileRB, &rectRB);
				SendMessage(v->hWndProfileRB, uMsg, wParam, lParam);
				SendMessage(v->hWndProfileTB, uMsg, wParam, lParam);
				MoveWindow(v->hWndProfileRB,
						   0,
						   0,
						   nWidth,
						   rectRB.bottom - rectRB.top,
						   TRUE);		
				rectRB.bottom += 10;
			}
			MoveWindow(v->hWndProfileLV, 0, rectRB.bottom, nWidth, nHeight - rectRB.bottom, TRUE);
		}
		break;
	case WM_DESTROY:
		if (pl != NULL)
			LocalFree(pl);
		SaveProfileSettings(hWnd);
		ImageList_Destroy(v->himlSmall);
		ImageList_Destroy(v->himlLarge);
		PostQuitMessage(0);
		break;
	case WM_CLOSE:
		if (OKToDestroyOtherWindows(hWnd) == TRUE)
			DestroyWindow(hWnd);
		break;
	case WM_COMMAND:
		switch(LOWORD(wParam))
		{
		case ID_HELP_CHECKFORNEWVERSION:
			DialogBox(v->hInstance, MAKEINTRESOURCE(IDD_CHECK_NEW_VERSION), hWnd, CheckNewVersionDlgProc);
			break;
		case ID_FILE_NEW_PROFILE:
			if (DialogBox(v->hInstance, MAKEINTRESOURCE(IDD_PROFILE_NEW), hWnd, NewProfileDlgProc) == TRUE)
			{
				HKEY hkProfile;
				LONG lKey;
				char szProfileKeyName[MAX_PATH];

				if (CopyProfile("", v->szNewProfileName) == FALSE)
				{
					MessageBox(hWnd, "Unable to create new profile", gszAppName, MB_ICONSTOP);
					break;
				}
				
				lstrcpy(szProfileKeyName, gszProfilesKeyName);
				lstrcat(szProfileKeyName, "\\TSReader-");
				lstrcat(szProfileKeyName, v->szNewProfileName);
				lKey = RegOpenKeyEx(HKEY_CURRENT_USER,
									szProfileKeyName,
									0,
									KEY_ALL_ACCESS,
									&hkProfile);
				if (lKey == ERROR_SUCCESS)
				{
					RegSetValueEx(hkProfile, "SourceName", 0, REG_SZ, (BYTE *)v->szNewProfileSource, lstrlen(v->szNewProfileSource) + 1);
					RegCloseKey(hkProfile);
				}

				SendMessage(hWnd, WM_COMMAND, ID_VIEW_REFRESH, 0);
			}
			break;
		case ID_FILE_DELETE:
			{
				int nIndex;
				char szProfileName[MAX_PATH] = {0};
				
				nIndex = GetSelectedProfile(szProfileName, sizeof(szProfileName));
				if (nIndex != -1)
				{
					if (lstrcmp(szProfileName, "Default") == 0)
						MessageBox(hWnd, "The default profile cannot be deleted", gszAppName, MB_ICONSTOP);
					else
					{
						if (v->fProfileBrowserShellMode)
						{
							HWND hWndOtherTSReader;
							char szTemp[256];

							lstrcpy(szTemp, gszMainClass);
							lstrcat(szTemp, ".");
							lstrcat(szTemp, v->szProfileName);
							hWndOtherTSReader = FindWindow(szTemp, NULL);
							if (hWndOtherTSReader != NULL)
							{
								MessageBox(hWnd, "Profile is currently active - close it before attempting to delete", gszAppName, MB_ICONSTOP);
								break;
							}
						}

						if (DeleteProfile(szProfileName) == ERROR_SUCCESS)
						{
							pl[nIndex].szProfileName[0] = '\0';
							ListView_DeleteItem(v->hWndProfileLV, nIndex);
						}
						else
						{
							MessageBox(hWnd, "Unable to delete profile", gszAppName, MB_ICONSTOP);
							SetFocus(v->hWndProfileLV);
						}
					}
				}
				else
				{
					MessageBox(hWnd, "Please select the profile you wish to delete.", gszAppName, MB_ICONINFORMATION);
					SetFocus(v->hWndProfileLV);
				}
			}
			break;
		case ID_FILE_EDIT:		// todo
			{
				char szProfileName[MAX_PATH] = {0};
				if (GetSelectedProfile(szProfileName, sizeof(szProfileName)) != -1)
				{
					if (lstrcmp(szProfileName, "Default") == 0)
						v->szProfileName[0] = '\0';
					else
						lstrcpy(v->szProfileName, szProfileName);

					if (v->fProfileBrowserShellMode)
					{
						HWND hWndOtherTSReader;
						char szTemp[256];

						lstrcpy(szTemp, gszMainClass);
						if (lstrlen(v->szProfileName))
						{
							lstrcat(szTemp, ".");
							lstrcat(szTemp, v->szProfileName);
						}
						hWndOtherTSReader = FindWindow(szTemp, NULL);
						if (hWndOtherTSReader != NULL)
						{
							MessageBox(hWnd, "Profile is currently active - close it before attempting to edit", gszAppName, MB_ICONSTOP);
							break;
						}
					}

					LoadSettings();
					if (DialogBox(v->hInstance, MAKEINTRESOURCE(IDD_SETTINGS), hWnd, SettingsDlgProc) == TRUE)
					{
						SaveSettings();
						SendMessage(hWnd, WM_COMMAND, ID_VIEW_REFRESH, 0);
					}
				}
				else
				{
					MessageBox(hWnd, "Please select the profile you wish to edit.", gszAppName, MB_ICONINFORMATION);
					SetFocus(v->hWndProfileLV);
				}
			}
			break;
		case ID_FILE_LAUNCHPROFILE:
			{
				char szProfileName[MAX_PATH] = {0};
				if (GetSelectedProfile(szProfileName, sizeof(szProfileName)) != -1)
				{
					HWND hWndOtherTSReader;
					char szTemp[256];

					if (lstrcmp(szProfileName, "Default") == 0)
						v->szProfileName[0] = '\0';
					else
						lstrcpy(v->szProfileName, szProfileName);					
					lstrcpy(szTemp, gszMainClass);
					if (lstrlen(v->szProfileName))
					{
						lstrcat(szTemp, ".");
						lstrcat(szTemp, v->szProfileName);
					}
					hWndOtherTSReader = FindWindow(szTemp, NULL);

					if (!v->fProfileBrowserShellMode)
					{
						if (hWndOtherTSReader != NULL)
						{
							if (SendMessage(hWndOtherTSReader, WM_USER + 12, 0, 0))
								ShowWindow(hWndOtherTSReader, SW_RESTORE);
							SetForegroundWindow(hWndOtherTSReader);
							v->fContinueAfterProfileBrowser = FALSE;
						}
						else
						{
							v->fContinueAfterProfileBrowser = TRUE;
						}
						DestroyWindow(hWnd);
					}
					else
					{
						char szCommandLine[256];

						if (hWndOtherTSReader != NULL)
						{
							if (SendMessage(hWndOtherTSReader, WM_USER + 12, 0, 0))
								ShowWindow(hWndOtherTSReader, SW_RESTORE);
							SetForegroundWindow(hWndOtherTSReader);
						}
						else
						{
							char szTemp[256] = {0};

							GetModuleFileName(v->hInstance, szCommandLine, sizeof(szCommandLine));
							if (lstrlen(v->szProfileName))
							{
								wsprintf(szTemp, " -L \"%s\"", v->szProfileName);
								lstrcat(szCommandLine, szTemp);
							}
							if (WinExec(szCommandLine, SW_SHOW) <= 31)
								MessageBox(hWnd, "Unable to start TSReader profile running", gszAppName, MB_ICONSTOP);
						}
					}
				}
				else
				{
					MessageBox(hWnd, "Please select the profile you wish to launch.", gszAppName, MB_ICONINFORMATION);
					SetFocus(v->hWndProfileLV);
				}
			}
			break;
		case ID_FILE_CREATEDESKTOPSHORTCUT:
			{
				char szProfileName[MAX_PATH] = {0};
				if (GetSelectedProfile(szProfileName, sizeof(szProfileName)) != -1)
					CreateWin32DesktopShortcut(hWnd, v->hInstance, szProfileName);
				else
				{
					MessageBox(hWnd, "Please select the profile you wish to create a shortcut for.", gszAppName, MB_ICONINFORMATION);
					SetFocus(v->hWndProfileLV);
				}
			}
			break;
		case ID_FILE_EXIT:
			if (OKToDestroyOtherWindows(hWnd) == TRUE)
				DestroyWindow(hWnd);
			break;
		case IDC_EDIT_COPY:
			{
				TCHAR szProfileName[MAX_PATH];

				if (GetSelectedProfile(szProfileName, sizeof(szProfileName)) != -1)
					lstrcpy(v->szCopyProfileName, szProfileName);
				else
					MessageBeep(0);
			}
			break;
		case IDC_EDIT_PASTE:
			if (v->szCopyProfileName[0] == '\0')
				MessageBeep(0);
			else
			{
				LV_ITEM lvItem;
				TCHAR szNewProfileName[MAX_PATH];
				TCHAR szCheckProfileName[MAX_PATH];
				int i;

				lstrcpy(szNewProfileName, v->szCopyProfileName);
				if (lstrcmp(v->szCopyProfileName, "Default") == 0)
					v->szCopyProfileName[0] = '\0';
				do
				{
					int nCount = ListView_GetItemCount(v->hWndProfileLV);

					if (nCount > 0)
					{
						for (i = 0; i < nCount; i++)
						{
							lvItem.mask = LVIF_TEXT | LVIF_PARAM;
							lvItem.iItem = i;
							lvItem.iSubItem = 0;
							lvItem.pszText = szCheckProfileName;
							lvItem.cchTextMax = sizeof(szCheckProfileName);
							ListView_GetItem(v->hWndProfileLV, &lvItem);

							if (lstrcmp(szCheckProfileName, szNewProfileName) == 0)
							{
								TCHAR szTemp[MAX_PATH];

								// Not unique so change name to "Copy of" until we get it unique
								lstrcpy(szTemp, TEXT("Copy of "));
								lstrcat(szTemp, szNewProfileName);
								lstrcpy(szNewProfileName, szTemp);
								if (lstrlen(szNewProfileName) > MAX_PATH - 9)
								{
									MessageBox(hWnd, TEXT("Unable to generate unique profile name - paste aborted"), gszAppName, MB_ICONSTOP);
									return FALSE;
								}
								i = nCount;
								break;
							}
						}
						if (i == nCount)
							break;
					}
				} while (TRUE);

				CursorWait(hWnd);
				CopyProfile(v->szCopyProfileName, szNewProfileName);
				SendMessage(v->hWndProfileLV, WM_SETREDRAW, (WPARAM)FALSE, 0);
				ListView_DeleteAllItems(v->hWndProfileLV);
				LoadProfiles(hWnd);
				SendMessage(v->hWndProfileLV, WM_SETREDRAW, (WPARAM)TRUE, 0);
				CursorNormal();
			}
			break;
		case ID_VIEW_LARGEICONS:
		case ID_VIEW_SMALLICONS:
		case ID_VIEW_PROFILE_LIST:
		case ID_VIEW_PROFILE_DETAILS:
			v->nProfileListViewState = LOWORD(wParam);
			SetProfileLVState(hWnd);
			break;
		case ID_VIEW_REFRESH:
			CursorWait(hWnd);
			SendMessage(v->hWndProfileLV, WM_SETREDRAW, (WPARAM)FALSE, 0);
			ListView_DeleteAllItems(v->hWndProfileLV);
			LoadProfiles(hWnd);
			SendMessage(v->hWndProfileLV, WM_SETREDRAW, (WPARAM)TRUE, 0);	
			CursorNormal();
			break;
		case ID_HELP_ABOUT:
			DialogBox(v->hInstance, MAKEINTRESOURCE(IDD_ABOUT), hWnd, AboutDlgProc);
			break;
		case ID_IDRPROFILESPOPUP_RENAME:
			{
				int nCount, i;
				LV_ITEM lvItem;
				TCHAR szProfileName[MAX_PATH];

				nCount = ListView_GetItemCount(v->hWndProfileLV);
				if (nCount > 0)
				{
					for (i = 0; i < nCount; i++)
					{
						lvItem.mask = LVIF_STATE | LVIF_TEXT;
						lvItem.iItem = i;
						lvItem.iSubItem = 0;
						lvItem.pszText = szProfileName;
						lvItem.cchTextMax = sizeof(szProfileName);
						lvItem.stateMask = LVIS_SELECTED;
						ListView_GetItem(v->hWndProfileLV, &lvItem);
						if ((lvItem.state & LVIS_SELECTED) > 0)
						{
							PostMessage(hWnd, WM_USER + 1, i, 0);
							break;
						}
					}
				}
				break;
			}
			break;
		}
		break;
	case WM_NOTIFY: 
		switch (((LPNMHDR) lParam)->code)
		{
		case LVN_BEGINLABELEDIT:
			{
				NMLVDISPINFO * pdi = (NMLVDISPINFO *)lParam;
				char szProfileName[MAX_PATH];
			
				if (GetSelectedProfile(szProfileName, sizeof(szProfileName)) == -1)
					return TRUE;
				if (lstrcmp(szProfileName, "Default") == 0)
					return TRUE;
				v->fProfileLabelEditActive = TRUE;
			}
			return 0;
		case LVN_ENDLABELEDIT:
			{
				NMLVDISPINFO * pdi = (NMLVDISPINFO *)lParam;

				v->fProfileLabelEditActive = FALSE;
				if (pdi->item.pszText != NULL)
				{
					LV_ITEM lvItem;
					char szOldProfileName[MAX_PATH];
					char szCompareOld[MAX_PATH], szCompareNew[MAX_PATH];

					if (pdi->item.pszText[0] == '\0')
					{
						MessageBox(hWnd, "You must specify a profile name", gszAppName, MB_ICONSTOP);
						return FALSE;
					}

					lvItem.mask = LVIF_TEXT | LVIF_PARAM;
					lvItem.iItem = pdi->item.iItem;
					lvItem.iSubItem = 0;
					lvItem.pszText = szOldProfileName;
					lvItem.cchTextMax = sizeof(szOldProfileName);
					if (ListView_GetItem(v->hWndProfileLV, &lvItem) == TRUE)
					{
						int i;
						int nIndex = lvItem.lParam;
						int nCount = ListView_GetItemCount(v->hWndProfileLV);
						char szCheckProfileName[MAX_PATH];

						if (nCount > 0)
						{
							for (i = 0; i < nCount; i++)
							{
								lvItem.mask = LVIF_TEXT | LVIF_PARAM;
								lvItem.iItem = i;
								lvItem.iSubItem = 0;
								lvItem.pszText = szCheckProfileName;
								lvItem.cchTextMax = sizeof(szCheckProfileName);
								ListView_GetItem(v->hWndProfileLV, &lvItem);
								if (lstrcmp(lvItem.pszText, pdi->item.pszText) == 0)
								{
									v->fProfileLabelEditActive = FALSE;
									MessageBox(hWnd, TEXT("Can't rename profile - a profile with that name already exists"), gszAppName, MB_ICONSTOP);
									return FALSE;
								}
							}
						}

						lstrcpy(szCompareOld, szOldProfileName);
						lstrcpy(szCompareNew, pdi->item.pszText);
						strlwr(szCompareOld);
						strlwr(szCompareNew);
						if (lstrcmp(szCompareOld, szCompareNew) == 0)
						{
							v->fProfileLabelEditActive = FALSE;
							return FALSE;
						}
						CursorWait(hWnd);
						if (RenameProfile(hWnd, szOldProfileName, pdi->item.pszText) == ERROR_SUCCESS)
						{
							wsprintf(pl[nIndex].szProfileName, "TSReader-%s", pdi->item.pszText);
							ListView_RedrawItems(v->hWndProfileLV, pdi->item.iItem, pdi->item.iItem);
						}
						CursorNormal();
					}
					return 0;
				}
				break;
			}
		case NM_DBLCLK:
			PostMessage(hWnd, WM_COMMAND, ID_FILE_LAUNCHPROFILE, 0);
			break;
		case LVN_COLUMNCLICK: 
			{
				#define pnm ((NM_LISTVIEW *) lParam)
				// Ascending or decending
				if (pnm->iSubItem != v->nSortProfileColumn)
				{
					v->nSortProfileColumn = pnm->iSubItem;
					v->fProfileColumnAscending = FALSE;
				}
				else
					v->fProfileColumnAscending = !v->fProfileColumnAscending;
				ListView_SortItems(pnm->hdr.hwndFrom, ProfileListCompareFunc, (LPARAM) (pnm->iSubItem)); 
				#undef pnm 
			}
			break; 
		case LVN_GETDISPINFO:
			{
				NMLVDISPINFO * pnmv = (NMLVDISPINFO *)lParam;
				GetProfileDispInfo((LV_DISPINFO *) lParam);
			}
			break;
		case TTN_NEEDTEXT:
			{
				LPTOOLTIPTEXT lpToolTipText;
				lpToolTipText = (LPTOOLTIPTEXT)lParam;

				switch (lpToolTipText->hdr.idFrom)
				{
				case ID_FILE_NEW_PROFILE:
					lpToolTipText->lpszText = TEXT("New Profile...");
					break;
				case ID_FILE_DELETE:
					lpToolTipText->lpszText = TEXT("Delete Profile");
					break;
				case ID_FILE_EDIT:
					lpToolTipText->lpszText = TEXT("Profile Configuration...");
					break;
				case ID_VIEW_LARGEICONS:
					lpToolTipText->lpszText = TEXT("Large Icons");
					break;
				case ID_VIEW_SMALLICONS:
					lpToolTipText->lpszText = TEXT("Small Icons");
					break;
				case ID_VIEW_PROFILE_LIST:
					lpToolTipText->lpszText = TEXT("List");
					break;
				case ID_VIEW_PROFILE_DETAILS:
					lpToolTipText->lpszText = TEXT("Details");
					break;
				case ID_HELP_ABOUT:
					lpToolTipText->lpszText = TEXT("About...");
					break;
				}
			}
			break;
		}
		break;
	case WM_CONTEXTMENU: 
		if ((HWND)wParam == v->hWndProfileLV)
			ProfileSheetContextMenu(hWnd, lParam);
		break;
	case WM_USER + 1:		// This is used when the user right clicks on a profile
							// and selects Rename. Because we might have come from a
							// popup menu, we have to wait until the menu is destroyed
							// or focus will change incorrectly
		SetFocus(v->hWndProfileLV);
		ListView_EditLabel(v->hWndProfileLV, wParam);
		break;
	default:
		return(DefWindowProc(hWnd, uMsg, wParam, lParam));
	}
	return 0L;
}

BOOL InitProfileApplication(HANDLE hInstance)
{
	WNDCLASS wndclass;
	
	// register profile window class
	wndclass.style =         CS_DBLCLKS;
	wndclass.lpfnWndProc =   ProfileWndProc;
	wndclass.cbClsExtra =    0;
	wndclass.cbWndExtra =    0;
	wndclass.hInstance =     hInstance;
	wndclass.hIcon =         LoadIcon(hInstance, MAKEINTRESOURCE(IDD_DVB_LOGO));
	wndclass.hCursor =       LoadCursor(NULL, IDC_ARROW);
	wndclass.hbrBackground = (HBRUSH) (COLOR_WINDOW);
	wndclass.lpszMenuName =  MAKEINTRESOURCE(IDR_PROFILES);
	wndclass.lpszClassName = gszProfileClassName;

	return(RegisterClass(&wndclass));
}

HWND InitProfileInstance(HANDLE hInstance, int nCmdShow)
{
	HWND hProfileWnd;
	DWORD dwStyle;
	char szTitle[MAX_PATH];

	LoadProfileSettings();

	// create the Profile window
	dwStyle = WS_OVERLAPPED | WS_CAPTION | WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_SYSMENU | WS_SIZEBOX;
	wsprintf(szTitle, "%s Profile Browser", gszAppName);
	hProfileWnd = CreateWindow(gszProfileClassName,
		                   szTitle,
						   dwStyle,
						   v->nInitialProfileXPos, v->nInitialProfileYPos, 
						   v->nInitialProfileXSize, v->nInitialProfileYSize,
						   NULL,
						   NULL,
						   hInstance,
						   NULL);
	if (hProfileWnd == NULL)
		return NULL;

	SetForegroundWindow(hProfileWnd);
	if (v->fProfilesMaximized == TRUE)
		nCmdShow = SW_MAXIMIZE;
	ShowWindow(hProfileWnd, nCmdShow);

	return hProfileWnd;
}

BOOL ShowProfileBrowser()
{
	if (InitProfileApplication(v->hInstance) == FALSE)
		return FALSE;

	v->hWndProfile = InitProfileInstance(v->hInstance, SW_SHOW);
	if (v->hWndProfile == NULL)
		return FALSE;
	v->fProfileLabelEditActive = FALSE;

	{
		MSG msg;
		HACCEL hAccel = LoadAccelerators(v->hInstance, MAKEINTRESOURCE(IDR_PROFILES));

		while (GetMessage(&msg, NULL, 0, 0))
		{
			if (v->fProfileLabelEditActive == TRUE)
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
			else if (!TranslateAccelerator(v->hWndProfile, hAccel, &msg))
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}
		DestroyAcceleratorTable(hAccel);
	}
	UnregisterClass(gszProfileClassName, v->hInstance);

	return v->fContinueAfterProfileBrowser;
}
#endif PRO
