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

#define USB_DOWNLOAD_CHUNK  512
#define NUM_USB_BUFFERS     10

// ------------------------------------------------------------------------------------------------
// ---- Types

typedef struct
{
  bool          Serviced;

  ULONG         Bytes_Read;
  OVERLAPPED    Overlapped;

  char          *Data;
} buffer;

// ------------------------------------------------------------------------------------------------

typedef struct
{
  WORD          Address;
  WORD          Length;

  BYTE          Data        [USB_DOWNLOAD_CHUNK];
} hexline;


// ------------------------------------------------------------------------------------------------
// ---- Prototypes

void    close_usb_buffers   ();                             // Free up all circular buffer memory & events
void    close_usb_driver    ();                             // Close the USB driver
HANDLE  create_pipe         (char*, UINT, UINT);            // Create a file pipe
void    download            (HANDLE);                       // Download the FX2 firmware
void    init_usb_buffers    ();                             // Initialise all circular buffer memory & events
void    init_usb_device     ();                             // Initialise the connected USB hardware
void    get_usb_dev_info    (GUID, char*, short*);          // Find the installed USB driver name
short   num_usb_devs        (short*);                       // Find the number of installed DigiTV USB devices
bool    open_usb_driver     (short);                        // Open the USB driver
void    reset_pipe          (HANDLE);                       // Reset the specified USB pipe
void    reset_usb_buffers   ();                             // Reset the data buffers
void    signal_usb_read     (HANDLE);                       // Signal an intention to read USB data
bool    transfer_data       (HANDLE, char *);               // Transfer data from the USB buffer to memory

void CALLBACK usb_read_complete (DWORD, DWORD, OVERLAPPED*);// USB data read completion routine
