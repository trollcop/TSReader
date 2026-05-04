#include <windows.h>
#include "resource.h"

HINSTANCE ghInstance;
char * gszTSReaderSourceParameters;
char gszMainClass[] = {"TSReaderArchiveMonitor"};
char gszAppName[] = {"TSReader Archive Monitor"};
char gszTSReaderClass[256] = {"TSReaderMain"};
char gszProfileName[256] = {""};

LRESULT FAR PASCAL MainWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_CREATE:
		{
			HICON hTrayIcon;
			NOTIFYICONDATA tnid; 
			
			// Add icon to tray 
			hTrayIcon = LoadIcon(ghInstance, MAKEINTRESOURCE(IDI_MAIN));
			tnid.cbSize = sizeof(NOTIFYICONDATA); 
			tnid.hWnd = hWnd; 
			tnid.uID = 1; 
			tnid.uFlags = NIF_MESSAGE | NIF_ICON | NIF_TIP; 
			tnid.uCallbackMessage = WM_USER + 1; 
			tnid.hIcon = hTrayIcon; 
			strcpy(tnid.szTip, gszAppName);
			Shell_NotifyIcon(NIM_ADD, &tnid); 
			DestroyIcon(hTrayIcon);

			// Tell TSReader our hWnd so it can tell us to quit if the archiver
			// gets stopped by the user
			SendMessage(FindWindow(gszTSReaderClass, NULL), WM_USER + 11, 0, (LPARAM)hWnd);

			// Start a timer to poll for TSReader
			SetTimer(hWnd, 1, 1000, NULL);
			break;
		}
	case WM_DESTROY:
		{
			NOTIFYICONDATA tnid; 

			// Remove from the tray
 			tnid.cbSize = sizeof(NOTIFYICONDATA); 
			tnid.hWnd = hWnd; 
			tnid.uID = 1; 
			Shell_NotifyIcon(NIM_DELETE, &tnid); 

			PostQuitMessage(0);
			break;
		}
	case WM_USER + 1:		// the user clicked our icon
		{
			UINT uMouseMsg = (UINT)lParam;

			if (uMouseMsg == WM_RBUTTONDOWN)
			{
				SetForegroundWindow(hWnd);
				if (MessageBox(hWnd, "Sure you want to quit monitoring TSReader?", gszAppName, MB_YESNO | MB_ICONWARNING) == IDYES)
				{
					KillTimer(hWnd, 1);
					DestroyWindow(hWnd);
				}
			}
			return FALSE;
		}
	case WM_USER + 2:		// TSR stopped archiving (deliberately)
		KillTimer(hWnd, 1);
		DestroyWindow(hWnd);
		break;
	case WM_TIMER:
		{
			HWND hWndTSReader; 

			hWndTSReader = FindWindow(gszTSReaderClass, NULL);
			if (hWndTSReader == NULL)
			{
				// Oops - TSReader isn't there! Restart and quit
				int i;
				BOOL fRetVal;
				PROCESS_INFORMATION ProcessInformation;
				STARTUPINFO StartupInfo;
				char szExecutable[MAX_PATH];
				char szCommandLine[MAX_PATH] = {""};
				char szTemp[MAX_PATH * 2];

				KillTimer(hWnd, 1);

				memset(&StartupInfo, 0, sizeof(StartupInfo));
				StartupInfo.cb = sizeof(STARTUPINFO);

				GetModuleFileName(ghInstance, szExecutable, sizeof(szExecutable));
				for (i = lstrlen(szExecutable); i > 0; i--)
				{
					if (szExecutable[i] == '\\')
					{
						szExecutable[i + 1] = 0;
						break;
					}
				}
				lstrcat(szExecutable, "TSReaderPro.exe");

				lstrcat(szCommandLine, "-n ");
				if (lstrlen(gszProfileName))
				{
					wsprintf(szTemp, "-L \"%s\" ", gszProfileName);
					lstrcat(szCommandLine, szTemp);
				}
				lstrcat(szCommandLine, gszTSReaderSourceParameters);
				wsprintf(szTemp, "\"%s\" %s", szExecutable, szCommandLine);
				WinExec(szTemp, SW_SHOW);
				//fRetVal = CreateProcess(szExecutable, szCommandLine, NULL, NULL, FALSE, CREATE_NEW_PROCESS_GROUP, NULL, NULL, &StartupInfo, &ProcessInformation);

				DestroyWindow(hWnd);
			}
			return FALSE;
		}
	default:
		return (DefWindowProc(hWnd, uMsg, wParam, lParam));
	}
	return 0L;
}

BOOL NEAR InitApplication(HANDLE hInstance)
{
	WNDCLASS wndclass;

	// register window class
	wndclass.style =         CS_DBLCLKS;
	wndclass.lpfnWndProc =   MainWndProc;
	wndclass.cbClsExtra =    0;
	wndclass.cbWndExtra =    0;
	wndclass.hInstance =     hInstance;
	wndclass.hIcon =         NULL;
	wndclass.hCursor =       LoadCursor(NULL, IDC_ARROW);
	wndclass.hbrBackground = (HBRUSH) (COLOR_BACKGROUND);
	wndclass.lpszMenuName =  NULL;
	wndclass.lpszClassName = gszMainClass;

	return(RegisterClass(&wndclass));
}

HWND NEAR InitInstance(HANDLE hInstance, int nCmdShow)
{
	HWND hWnd;
	DWORD dwStyle;

	// create the window
	dwStyle = WS_OVERLAPPED | WS_CAPTION | WS_MINIMIZEBOX | WS_SYSMENU;
	hWnd = CreateWindow(gszMainClass, gszAppName,
						   dwStyle,
						   CW_USEDEFAULT, CW_USEDEFAULT,
						   CW_USEDEFAULT, CW_USEDEFAULT,
						   NULL, NULL, hInstance, NULL);

	if (NULL == hWnd)
		return (NULL);

	return (hWnd);

}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpszCmdLine, int nCmdShow)
{
	HWND hWnd;
	MSG msg;
	char * szComma;

	// Command line has the profile name comma and then the command-line parameters for TSR
	szComma = strstr(lpszCmdLine, ",");
	if (szComma == NULL)
	{
		char szTemp[MAX_PATH];

		wsprintf(szTemp, "Incorrect command-line\n\n%s\n", lpszCmdLine);
		MessageBox(NULL, szTemp, gszAppName, MB_OK | MB_ICONSTOP);
		return 1;
	}
	*szComma = '\0';
	if (lstrlen(lpszCmdLine))
	{
		lstrcat(gszTSReaderClass, ".");
		lstrcat(gszTSReaderClass, lpszCmdLine);
		lstrcpy(gszProfileName, lpszCmdLine);
	}
	gszTSReaderSourceParameters = szComma + 1;

	ghInstance = hInstance;
	if (InitApplication(hInstance) == FALSE)
	{
		MessageBox(NULL, "CreateWindow failed in InitApplication", gszAppName, MB_OK | MB_ICONSTOP);
		return 1;
	}

	hWnd = InitInstance(hInstance, SW_HIDE);
	if (hWnd == NULL)
	{
		MessageBox(NULL, "CreateWindow failed in InitInstance", gszAppName, MB_OK | MB_ICONSTOP);
		return 1;
	}

	// Process message loop
	while (GetMessage(&msg, NULL, 0, 0) == TRUE)
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	
	return ((int)msg.wParam);
}
