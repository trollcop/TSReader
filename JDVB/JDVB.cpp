// JDVB.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "JDVB.h"
#include "Programm.h"
#include "DVBSource.h"
#include "Global.h"
#include "MDAPI.h"
#include "MDPluginManager.h"
#include "DirectShowTSTarget.h"
#include "ChannelSetting.h"

#include <stdio.h>

#include <streams.h>
#include <winsock2.h>

#define MAX_LOADSTRING 100

// Global Variables:


MDPluginManager * g_PluginManager = 0;
TSTarget *g_defaultVideoOutDevice = 0;
int g_cProgrammIndex = 0;	
HINSTANCE hInst;								// current instance
HWND hWnd;
TCHAR szTitle[MAX_LOADSTRING];					// The title bar text
TCHAR szWindowClass[MAX_LOADSTRING];			// the main window class name

// Forward declarations of functions included in this code module:
ATOM				MyRegisterClass(HINSTANCE hInstance);
BOOL				InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK	About(HWND, UINT, WPARAM, LPARAM);

int APIENTRY _tWinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPTSTR    lpCmdLine,
                     int       nCmdShow)
{
 	// TODO: Place code here.
	
	if(FAILED(CoInitialize(NULL)))
		exit (-1);

	WSADATA wsData;
	if(WSAStartup(0x202,&wsData) != 0)
	{
		WSACleanup();
		exit(-2);
	}

	MSG msg;
	HACCEL hAccelTable;

	// Initialize global strings
	LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadString(hInstance, IDC_JDVB, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);
	HRESULT hr;
	g_DVBSource = new DVBSource();
	g_DVBSource->setTransponder (12692l,22000,1); //TW1
	g_defaultVideoOutDevice = new DirectShowTSTarget("Default Video Out Device");
	//--------------------------------------------------------------
	

	// Perform application initialization:
	if (!InitInstance (hInstance, nCmdShow)) 
	{
		return FALSE;
	}
	g_defaultVideoOutDevice = new DirectShowTSTarget("Default Playback Device");
	//hr = g_DVBSource->scanTransponder ();
	Programm *TW1Programm = 0;
	ChannelSetting *l_ChannelSetting = new ChannelSetting();
	hr = l_ChannelSetting->read (&TW1Programm,"ChannelSetting.txt");
	hr = g_DVBSource->importProgramm (TW1Programm);
	//if(FAILED(g_DVBSource->findProgrammByName (12692l,22000,1,"TW1",&TW1Programm)))
	//	return -1;
	
//	hr = l_ChannelSetting->write (TW1Programm,-1,"ChannelSetting.txt","ASTRA 19.2 E","Twinhan DVB");
	

	
	

	hr = TW1Programm->insert (g_defaultVideoOutDevice);
	
	RECT rect;
	GetClientRect(hWnd,&rect);
	hr = g_defaultVideoOutDevice->setVideoProperties (hWnd,rect);
	hr = g_defaultVideoOutDevice->togglePlayback ();
	hAccelTable = LoadAccelerators(hInstance, (LPCTSTR)IDC_JDVB);
	//ChannelSetting *l_ChannelSetting = new ChannelSetting();

	//g_PluginManager = new MDPluginManager(g_DVBSource,hInst,hWnd);
	//FIXME
	//hr = g_PluginManager->loadMDPlugins ("Local");


	while (GetMessage(&msg, NULL, 0, 0)) 
	{
		//
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg) ||!IsWindow(g_UDPPidDlg) || !IsDialogMessage(g_UDPPidDlg,&msg) ||
			!IsWindow(g_DVBSourceDlg) || !IsDialogMessage(g_DVBSourceDlg, &msg) ||
			!IsWindow(g_cProgramm) || !IsDialogMessage(g_cProgramm, &msg)) 
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}
	g_defaultVideoOutDevice->togglePlayback ();
	//TW1Programm->remove (g_defaultVideoOutDevice);
	delete g_DVBSource;
	

//	delete g_defaultVideoOutDevice;
	WSACleanup();
	CoUninitialize();

	return (int) msg.wParam;
	

}



//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
//  COMMENTS:
//
//    This function and its usage are only necessary if you want this code
//    to be compatible with Win32 systems prior to the 'RegisterClassEx'
//    function that was added to Windows 95. It is important to call this function
//    so that the application will get 'well formed' small icons associated
//    with it.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX); 

	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= (WNDPROC)WndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= LoadIcon(hInstance, (LPCTSTR)IDI_JDVB);
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= (LPCTSTR)IDC_JDVB;
	wcex.lpszClassName	= szWindowClass;
	wcex.hIconSm		= LoadIcon(wcex.hInstance, (LPCTSTR)IDI_SMALL);

	return RegisterClassEx(&wcex);
}

//
//   FUNCTION: InitInstance(HANDLE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{

   hInst = hInstance; // Store instance handle in our global variable

   hWnd = CreateWindow(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, NULL, NULL, hInstance, NULL);

   if (!hWnd)
   {
      return FALSE;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}

//
//  FUNCTION: WndProc(HWND, unsigned, WORD, LONG)
//
//  PURPOSE:  Processes messages for the main window.
//
//  WM_COMMAND	- process the application menu
//  WM_PAINT	- Paint the main window
//  WM_DESTROY	- post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int wmId, wmEvent;
	PAINTSTRUCT ps;
	HDC hdc;

	switch (message) 
	{
		case WM_USER:
		{
			// MD API Plugins exchange information via WM_USER messages
			// wParam = Type of Message eg. MDAPI_START_FILTER
			// lParam = Pointer to Argument of calling Plugin
			g_PluginManager->dispatch (wParam,NULL,lParam);
			break;
		}
	
		case WM_COMMAND:
		{
			wmId    = LOWORD(wParam); 
			wmEvent = HIWORD(wParam); 
		// Parse the menu selections:
			if(wmId >= 40000 && wmId <= 41000)
			{
				// The MultiDec API allows custom user menu id's to be
				// between 40000 & 41000
				// Plugin Developers should always check if the
				// requested id has not already been occupied by an other
				// plugin (developer)
				//MessageBox(hWnd,"CALLING dispatchMDAPIMessages","Information",MB_OK);
				g_PluginManager->dispatch (MDAPI_ON_MENU_SELECT,reinterpret_cast<WPARAM*>(&wmId),NULL);
				break;
			}
			switch (wmId)
			{
				case IDM_ABOUT:
					DialogBox(hInst, (LPCTSTR)IDD_ABOUTBOX, hWnd, (DLGPROC)About);
					break;
				case IDM_OPTIONS_SHOWPROGRAMM:
				{
					unsigned int commandId;
					//g_defaultVideoOutDevice->getProgrammProperties (g_cProgrammIndex,hInst,hWnd,&g_cProgramm,&commandId);
					//ShowWindow(g_cProgramm, SW_SHOW);
					//g_defaultVideoOutDevice->setVideoOwner (g_cProgrammIndex,g_cProgramm);
					//UpdateWindow(g_cProgramm);
					//ShowWindow(g_cProgramm, SW_SHOW);
					break;
				}
				case IDM_OPTIONS_VIEWUDPFILTER:
				{
			
					//if(g_UDPPidDlg == 0)
					//	g_UDPPidDlg = CreateDialog(hInst,(LPCTSTR)IDD_UDPPID,hWnd,(DLGPROC)UDPPidFilterProcedure);
					//ShowWindow(g_UDPPidDlg,SW_SHOW);
					//SetTimer(g_UDPPidDlg,IDT_UDP_TIMER,500,(TIMERPROC)NULL);

					break;
				}
				case IDM_OPTIONS_DVBSOURCEFILTER:
				{
					if(g_DVBSourceDlg == 0)
						g_DVBSourceDlg = CreateDialog(hInst,(LPCTSTR)IDD_DVB_SOURCE,hWnd,(DLGPROC)DVBSourceDlgProcedure);
			
					ShowWindow(g_DVBSourceDlg,SW_SHOW);
					SetTimer(g_DVBSourceDlg,IDT_DVB_SOURCE_TIMER,1000,(TIMERPROC)NULL);
					break;
				}
				case IDM_EXIT:
					DestroyWindow(hWnd);
					break;
			default:
				return DefWindowProc(hWnd, message, wParam, lParam);
			}
		break;
		}
		case WM_PAINT:
				hdc = BeginPaint(hWnd, &ps);
		// TODO: Add any drawing code here...
				EndPaint(hWnd, &ps);
				break;
		case WM_DESTROY:
			PostQuitMessage(0);
			break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

// Message handler for about box.
LRESULT CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_INITDIALOG:
		return TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL) 
		{
			EndDialog(hDlg, LOWORD(wParam));
			return TRUE;
		}
		break;
	}
	return FALSE;
}
