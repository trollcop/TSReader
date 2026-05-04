// ------------------------------------------------------------------------------------------------
// ---- This file and its contents are Copyright (C) Nebula Electronics Ltd 2005
// ---- 
// ---- The user may use this file and its contents without restriction, EXCEPT where intended for
// ---- commercial use. If this file or its contents are to be used in a commercial application, then 
// ---- prior written consent must first be obtained from Nebula Electronics Ltd. In this case, please  
// ---- email sales@nebule-electronics.com.
// ---- 
// ---- Although every effort has been made to ensure that this information contained in this file is
// ---- correct, it is supplied WITHOUT WARRANTY. No guarantee of fitness for use or merchantability
// ---- is implied or should be inferred.
// ------------------------------------------------------------------------------------------------

#include "stdafx.h"

#include <wininet.h> 
#include <stdio.h> 
#include <process.h> 
//#include <dshow.h> 
#include <vfw.h>
#include <tlhelp32.h>
#include <commdlg.h>
#include <commctrl.h>
#include <shellapi.h>
#include <direct.h>

#include "defs.h"
#include "device.h"
#include "record.h"
#include "tuner.h"

#include "pcidriver.h"
#include "usbdriver.h"

#include "SDK.h"


// ------------------------------------------------------------------------------------------------
// ---- Global Variables

HINSTANCE           hInst;                                          // The current instance

bool                Recording               = false;                // True when recording the transport stream

char                Save_File               [MAX_STRING_LENGTH];    // The name of the saved transport stream

short               COFDM_Type              = MT_352;               // The installed COFDM chip
short               Device_No               = 0;                    // The DigiTV device number of this instance
short               Num_Devices             = 0;                    // The number of connected DigiTV devices
short               Device_List             [MAX_DEVICES];          // A list of the device numbers for each active device

int                 Base_Variant;                                   // Which variant of the hardware is installed
int                 Variant;                                        // The current hardware interface mode


// ------------------------------------------------------------------------------------------------
// ---- Static Variables

static __int64      Reset_Time              = 0;                    // Base point from which times are calculated

static LARGE_INTEGER    Frequency;                                  // Performance counter frequency


// ------------------------------------------------------------------------------------------------
// ---- Static Variables

extern short        Tune_BWidth;                                    // Channel bandwidth


// ------------------------------------------------------------------------------------------------
// ---- WinMain

int APIENTRY WinMain (HINSTANCE     hInstance,
                      HINSTANCE     hPrevInstance,
                      LPSTR         lpCmdLine,
                      int           nCmdShow)
{
  INITCOMMONCONTROLSEX  CommCtl;


  CommCtl. dwSize = sizeof (INITCOMMONCONTROLSEX);
  CommCtl. dwICC  = ICC_DATE_CLASSES | ICC_INTERNET_CLASSES;
  hInst           = hInstance;

  QueryPerformanceFrequency (&Frequency);
  CoInitialize              (NULL);
  InitCommonControls        ();
  InitCommonControlsEx      (&CommCtl);

  if (strstr (lpCmdLine, "DEV"))
  {
    Device_No = atoi ((char*) (strstr (lpCmdLine, "DEV" ) + 3));
  } // if

  start_drivers  ();
  init_tuner     (true);

  DialogBox      (hInstance, (LPCTSTR) IDD_STRENGTH, NULL, (DLGPROC) Strength);

  CoUninitialize ();
  return         (0);
} // WinMain


// ------------------------------------------------------------------------------------------------
// ---- Dialog Box Handler

LRESULT CALLBACK Strength (HWND    hDlg,
                           UINT    message,
                           WPARAM  wParam,
                           LPARAM  lParam)
{
  char          Str             [MAX_STRING_LENGTH];

  int           Addr;
  int           Button;
  int           wmId;

  HWND          hAGC            = GetDlgItem (hDlg, IDC_IF_AGC   );
  HWND          hBER            = GetDlgItem (hDlg, IDC_BER      );
  HWND          hSNR            = GetDlgItem (hDlg, IDC_SNR      );
  HWND          hFreq           = GetDlgItem (hDlg, IDC_FREQUENCY);
  HWND          hSerial_No      = GetDlgItem (hDlg, IDC_SERIAL_NO);
  HWND          h7MHz           = GetDlgItem (hDlg, IDC_7MHZ     );
  HWND          h8MHz           = GetDlgItem (hDlg, IDC_8MHZ     );

  DWORD         Data;
  RECT          Rect;


  switch (message)
  {
    case WM_INITDIALOG : SetWindowLong (hAGC, GWL_STYLE, GetWindowLong (hAGC, GWL_STYLE) | PBS_SMOOTH);

                         SendMessage (hAGC,  PBM_SETRANGE, 0, MAKELPARAM (0, 511));
                         SendMessage (hBER,  PBM_SETRANGE, 0, MAKELPARAM (0, 100));
                         SendMessage (hSNR,  PBM_SETRANGE, 0, MAKELPARAM (0, 32 ));

                         SendMessage (h7MHz, BM_SETCHECK,  (Tune_BWidth == 7) ? BST_CHECKED : BST_UNCHECKED, 0);
                         SendMessage (h8MHz, BM_SETCHECK,  (Tune_BWidth == 8) ? BST_CHECKED : BST_UNCHECKED, 0);

                         if (Variant == PCI_VARIANT) Addr = 0x00;
                         else                        Addr = 0xf8;

                         Data = (read_eeprom (Addr    )        +
                                (read_eeprom (Addr + 1) <<  8) +
                                (read_eeprom (Addr + 2) << 16) +
                                (read_eeprom (Addr + 3) << 24));

                         sprintf       (Save_File, "");
                         sprintf       (Str, "%d", Data);
                         SetWindowText (hSerial_No, Str);

                         GetWindowRect (hDlg, &Rect);

                         Rect. left += (Device_No * 20);
                         Rect. top  += (Device_No * 20);

                         SetWindowPos  (hDlg, HWND_TOP, Rect. left, Rect. top, 0, 0, SWP_NOSIZE);

                         SetTimer (hDlg, 1, 200, NULL);
                         return   (FALSE);

    case WM_TIMER      : update_locks (hDlg);
                         break;

    case WM_NOTIFY     : break;

    case WM_COMMAND    : wmId   = HIWORD (wParam);
                         Button = LOWORD (wParam);

                         switch (wmId)
                         {
                           case BN_CLICKED    : switch (Button)
                                                {
                                                  case IDC_START  : EnableWindow (GetDlgItem (hDlg, IDC_START), false);
                                                                    EnableWindow (GetDlgItem (hDlg, IDC_STOP ), true );

                                                                    Recording = true;

                                                                    _beginthread (ts_record, 0, Save_File);
                                                                    break;

                                                  case IDC_STOP   : EnableWindow (GetDlgItem (hDlg, IDC_START), true );
                                                                    EnableWindow (GetDlgItem (hDlg, IDC_STOP ), false);

                                                                    Recording = false;
                                                                    break;

                                                  case IDC_BROWSE : save_file     (hDlg, Save_File);
                                                                    SetWindowText (GetDlgItem (hDlg, IDC_FILE), Save_File);
                                                                    break;

                                                  case IDC_7MHZ   : Tune_BWidth = 7;
                                                                    GetWindowText       (hFreq, Str, MAX_STRING_LENGTH);
                                                                    set_tuner_frequency ((float) atof (Str), Tune_BWidth);
                                                                    break;

                                                  case IDC_8MHZ   : Tune_BWidth = 8;
                                                                    GetWindowText       (hFreq, Str, MAX_STRING_LENGTH);
                                                                    set_tuner_frequency ((float) atof (Str), Tune_BWidth);
                                                                    break;

                                                  case IDCANCEL   :
                                                  case IDOK       : EndDialog (hDlg, LOWORD (wParam));
                                                                    return    (TRUE);
                                                } // switch
                                                break;

                           case EN_CHANGE     : GetWindowText       (hFreq, Str, MAX_STRING_LENGTH);
                                                set_tuner_frequency ((float) atof (Str), Tune_BWidth);
                                                break;
                         } // switch
                         break;
  } // switch

  return (FALSE);
} // Strength


// ------------------------------------------------------------------------------------------------
// ---- get_system_time

DWORD get_system_time (bool     Reset)
{
  __int64           Ticks;

  LARGE_INTEGER     Count;

  QueryPerformanceCounter   (&Count);

  Ticks = Count. QuadPart;

  if (Reset) Reset_Time = Ticks;

  return ((DWORD) (((Ticks - Reset_Time) * 1000) / Frequency. QuadPart));
} // get_system_time


// ------------------------------------------------------------------------------------------------
// ---- safe_free

void safe_free (void   **pObject)
{
  void      *pTemp  = (*pObject);

  *pObject = 0;

  if (pTemp) free (pTemp);
} // safe_free


// ------------------------------------------------------------------------------------------------
// ---- save_file

void save_file (HWND        hWnd,
                char        *pPath)
{
  OPENFILENAME  Info;

  Info. lStructSize       = sizeof (OPENFILENAME);
  Info. hwndOwner         = hWnd;
  Info. hInstance         = NULL;
  Info. lpstrCustomFilter = NULL;
  Info. nMaxCustFilter    = 0;
  Info. nFilterIndex      = 1;
  Info. lpstrFile         = pPath;
  Info. nMaxFile          = MAX_STRING_LENGTH;
  Info. lpstrFileTitle    = NULL;
  Info. nMaxFileTitle     = MAX_STRING_LENGTH;
  Info. lpstrInitialDir   = NULL;
  Info. lpstrTitle        = NULL;
  Info. Flags             = OFN_ENABLESIZING | OFN_EXPLORER | OFN_FILEMUSTEXIST;
  Info. lCustData         = NULL;
  Info. lpTemplateName    = NULL;

  Info. lpstrFilter = "Transport Stream Files\0*.ts\0\0";
  Info. lpstrDefExt = "ts";

  GetSaveFileName (&Info);
} // save_file


// ------------------------------------------------------------------------------------------------
// ---- start_drivers

void start_drivers ()
{
  char              Argument    [MAX_STRING_LENGTH];
  char              Command     [MAX_STRING_LENGTH];

  bool              Result_PCI;
  bool              Result_USB;

  short             Dev_Pos     [MAX_DEVICES];
  short             Pos;

 
  Num_Devices = num_usb_devs (Dev_Pos);

  if (Device_No == 0)
  {
    for (Pos = 1; Pos < Num_Devices; Pos ++)
    {
      sprintf       (Command,  "SDK.exe");
      sprintf       (Argument, "DEV %d", Pos);
      start_process (Command,  "SDK", Argument);
    } // for
  } // if

  Result_USB = open_usb_driver (Dev_Pos [Device_No]);
  Result_PCI = open_pci_driver ();

  if      (Result_USB) Base_Variant = USB_VARIANT;
  else if (Result_PCI) Base_Variant = PCI_VARIANT;
  else                 Base_Variant = NO_HARDWARE;

  if      (Result_USB) Variant = USB_VARIANT;
  else if (Result_PCI) Variant = PCI_VARIANT;
  else                 Variant = NO_HARDWARE;

  switch (Variant)
  {
    case PCI_VARIANT : init_pci_device (); break;
    case USB_VARIANT : init_usb_device (); break;
  } // switch

  if (read_i2c (NXT_CHIP_ID) == 0x0b) COFDM_Type = NXT_6000;
  else                                COFDM_Type = MT_352;
} // start_drivers


// ------------------------------------------------------------------------------------------------
// ---- start_process

void start_process (LPSTR   Path,
                    LPSTR   Command,
                    LPSTR   Argument)
{
  _spawnl (_P_NOWAIT, Path, Command, Argument, NULL);
} // start_process


// ------------------------------------------------------------------------------------------------
// ---- update_locks

void update_locks (HWND     hDlg)
{
  static HICON  hLED_Off        = NULL; 
  static HICON  hLED_On         = NULL;

  char          AGC_Str         [MAX_STRING_LENGTH];
  char          BER_Str         [MAX_STRING_LENGTH];
  char          SNR_Str         [MAX_STRING_LENGTH];

  COLORREF      AGC_Col;
  COLORREF      BER_Col;
  COLORREF      SNR_Col;

  HWND          hAGC_Lock       = GetDlgItem (hDlg, IDC_AGC_LOCK  );
  HWND          hTPS_Lock       = GetDlgItem (hDlg, IDC_TPS_LOCK  );
  HWND          hVIT_Lock       = GetDlgItem (hDlg, IDC_VIT_LOCK  );
  HWND          hFEC_Lock       = GetDlgItem (hDlg, IDC_FEC_LOCK  );
  HWND          hAGC            = GetDlgItem (hDlg, IDC_IF_AGC    );
  HWND          hBER            = GetDlgItem (hDlg, IDC_BER       );
  HWND          hSNR            = GetDlgItem (hDlg, IDC_SNR       );
  HWND          hAGC_Val        = GetDlgItem (hDlg, IDC_AGC_VAL   );
  HWND          hBER_Val        = GetDlgItem (hDlg, IDC_BER_VAL   );
  HWND          hSNR_Val        = GetDlgItem (hDlg, IDC_SNR_VAL   );

  HWND          hInfMode        = GetDlgItem (hDlg, IDC_INF_MODE  );
  HWND          hInfMod         = GetDlgItem (hDlg, IDC_INF_MOD   );

  HWND          hFreq           = GetDlgItem (hDlg, IDC_FREQUENCY );

  HICON         AGC_Lock;
  HICON         TPS_Lock;
  HICON         VIT_Lock;
  HICON         FEC_Lock;

  tune_info     Tune_Info;


  if (!hLED_Off) hLED_Off   = LoadIcon (hInst, (LPCTSTR) IDI_LED_OFF);
  if (!hLED_On ) hLED_On    = LoadIcon (hInst, (LPCTSTR) IDI_LED_ON );
                         
  channel_locked (&Tune_Info, false);

  if (Tune_Info. Mode) SetWindowText (hInfMode, "8K");
  else                 SetWindowText (hInfMode, "2K");

  switch (Tune_Info. Modulation)
  {
    case 0x00 : SetWindowText (hInfMod, "QPSK"   ); break;
    case 0x01 : SetWindowText (hInfMod, "QAM 16" ); break;
    case 0x02 : SetWindowText (hInfMod, "QAM 64" ); break;
    case 0x03 : SetWindowText (hInfMod, "Unknown"); break;
  } // switch

  sprintf       (AGC_Str, "%d%%",  (Tune_Info. AGC * 100) / 511);
  sprintf       (BER_Str, "%1.4f",  Tune_Info. BER);
  sprintf       (SNR_Str, "%ddB",   Tune_Info. SNR);

  SetWindowText (hAGC_Val, AGC_Str);
  SetWindowText (hBER_Val, BER_Str);
  SetWindowText (hSNR_Val, SNR_Str);

  SendMessage   (hAGC, PBM_SETPOS, Tune_Info. AGC,                  0);
  SendMessage   (hBER, PBM_SETPOS, (int) (Tune_Info. BER * 1000.0), 0);
  SendMessage   (hSNR, PBM_SETPOS, Tune_Info. SNR,                  0);

  AGC_Lock = (Tune_Info.AGC_Lock) ? hLED_On : hLED_Off;
  TPS_Lock = (Tune_Info.TPS_Lock) ? hLED_On : hLED_Off;
  VIT_Lock = (Tune_Info.VIT_Lock) ? hLED_On : hLED_Off;
  FEC_Lock = (Tune_Info.FEC_Lock) ? hLED_On : hLED_Off;

  SendMessage (hFEC_Lock, STM_SETIMAGE, IMAGE_ICON, (LPARAM) FEC_Lock);
  SendMessage (hVIT_Lock, STM_SETIMAGE, IMAGE_ICON, (LPARAM) VIT_Lock);
  SendMessage (hTPS_Lock, STM_SETIMAGE, IMAGE_ICON, (LPARAM) TPS_Lock);
  SendMessage (hAGC_Lock, STM_SETIMAGE, IMAGE_ICON, (LPARAM) AGC_Lock);

  if      (Tune_Info. AGC < 260 ) AGC_Col = RGB (0xff,0x00,0x00); // Red
  else if (Tune_Info. AGC < 350 ) AGC_Col = RGB (0xff,0x80,0x00); // Amber
  else                            AGC_Col = RGB (0x00,0xff,0x00); // Green

  if      (Tune_Info. BER > 0.06) BER_Col = RGB (0xff,0x00,0x00); // Red
  else if (Tune_Info. BER > 0.03) BER_Col = RGB (0xff,0x80,0x00); // Amber
  else                            BER_Col = RGB (0x00,0xff,0x00); // Green

  if      (Tune_Info. SNR <  17 ) SNR_Col = RGB (0xff,0x00,0x00); // Red
  else if (Tune_Info. SNR <  20 ) SNR_Col = RGB (0xff,0x80,0x00); // Amber
  else                            SNR_Col = RGB (0x00,0xff,0x00); // Green

  SendMessage (hAGC, PBM_SETBARCOLOR, 0, AGC_Col);
  SendMessage (hBER, PBM_SETBARCOLOR, 0, BER_Col);
  SendMessage (hSNR, PBM_SETBARCOLOR, 0, SNR_Col);
} // update_locks

