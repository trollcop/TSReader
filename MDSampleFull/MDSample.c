// MDSample.c - sample plugin for the MultiDec API

#include <windows.h>
#include <time.h>
#include "MDSample.h"
#include "resource.h"

// Global variables
HANDLE hOutput;
HINSTANCE hTSReaderInstance;
HWND hTSReaderWnd;
BOOL fRunning;
int nMyDLLID;
DWORD dwOutput;
char szThisFilterName[] = {"TSReader Full TS Sample"};
TSTART_FILTER Filter;

// On_Start
// This function gets called when TSReader loads the plugin
// Saves important stuff like TSReader's hWnd

void On_Start(HINSTANCE hInstance, HWND hWnd, BOOL bLogSet, int nDLLID, char * szHotKey)
{
	hTSReaderInstance = hInstance;
	hTSReaderWnd = hWnd;
	nMyDLLID = nDLLID;
	fRunning = FALSE;
}

// On_Channel_Change
// This function gets called when the user changes channel in TSReader
// by clicking on a PMT entry

void On_Channel_Change(TPROGRAM CurrentProgram)
{
}

// On_Send_Dll_ID_Name
// Called by TSReader when loading the plugin to get
// it's name

void On_Send_Dll_ID_Name(char * szName) 
{
	lstrcpy(szName, szThisFilterName);
}

// On_Exit
// Called by TSReader when the plugin is being unloaded
void On_Exit(HINSTANCE hInstance, HWND hWnd, BOOL bLogSet)
{
}

// On_Filter_Receive
void On_Filter_Receive(int nMyFilter, int nLength, unsigned char * pBuffer)
{
	if (fRunning)
	{
		DWORD dwWritten;

		WriteFile(hOutput, pBuffer, nLength, &dwWritten, NULL);
		dwOutput += dwWritten;
	}
}

// On_Menu_Select
// TSReader calls this routine when a menu item from the EXTERN menu
// in this DLL is selected by the user. The menu message must have a range
// of 40000 through 41000 but really should start at 40500 to prevent
// interference with TSReader's menus
void On_Menu_Select(unsigned int nMenuID)
{
	switch(nMenuID)
	{
	case ID_CAPTURE_TS:
		if (fRunning == FALSE)
		{
			hOutput = CreateFile("c:\\TSReaderPlugin.ts", GENERIC_WRITE, 0, (LPSECURITY_ATTRIBUTES)NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, (HANDLE)NULL);
			if (hOutput == INVALID_HANDLE_VALUE)
			{
				MessageBox(hTSReaderWnd, "Unable to open c:\\TSReaderPlugin.ts", szThisFilterName, MB_ICONSTOP);
				break;
			}
			dwOutput = 0;
			fRunning = TRUE;
			ZeroMemory(&Filter.Name, sizeof(Filter.Name));
			strncpy(Filter.Name, "TSReader Full TS", sizeof(Filter.Name));
			Filter.DLL_ID = nMyDLLID;
			Filter.Filter_ID = 0;	// filled in by TSReader
			Filter.Pid = 0x9fff;	// special for full TS
			Filter.Irq_Call_Adresse = (DWORD)On_Filter_Receive;			
			SendMessage(hTSReaderWnd, WM_USER, MDAPI_START_FILTER, (LPARAM)&Filter);			
		}
		else
		{
			char szTemp[128];

			SendMessage(hTSReaderWnd, WM_USER, MDAPI_STOP_FILTER, Filter.Running_ID);
			fRunning = FALSE;
			CloseHandle(hOutput);
			wsprintf(szTemp, "Wrote %d MB to c:\\TSReaderPlugins.ts", dwOutput / 1024 / 1024);
			MessageBox(hTSReaderWnd, szTemp, szThisFilterName, MB_ICONINFORMATION);
		}
		break;
	}
}




