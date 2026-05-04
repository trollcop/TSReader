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


// ------------------------------------------------------------------------------------------------
// ---- Definitions

#define MAX_DEVICES             10                              // Maximum number of connected DigiTV devices

#define PCI_VARIANT             1                               // PCI Version of hardware installed 
#define USB_VARIANT             2                               // USB Version of hardware installed 
#define NO_HARDWARE             4                               // No hardware installed 


// ------------------------------------------------------------------------------------------------
// ---- Function Prototypes

int APIENTRY        WinMain         (HINSTANCE, HINSTANCE, LPSTR, int);

BOOL                InitInstance    (HINSTANCE, int);
LRESULT CALLBACK    Strength        (HWND, UINT, WPARAM, LPARAM);

DWORD               get_system_time (bool);                     // Get elapsed time in mS
void                safe_free       (void**);                   // Free  memory in a pointer-safe fashion
void                save_file       (HWND, char*);              // Opens a 'Save File' Dialog Box
void                start_drivers   ();                         // Start the drivers relevent to the device attached
void                start_process   (LPSTR, LPSTR, LPSTR);      // Start up any slave processes
void                update_locks    (HWND);
