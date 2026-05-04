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
#include "setupapi.h"
#include <winioctl.h>
#include <stdio.h>
//#include <dshow.h>
#include <ks.h>

#include "defs.h"
#include "device.h"
#include "digiusb.h"
#include "sdk.h"   
#include "tuner.h"   

#include "usbdriver.h"


// ------------------------------------------------------------------------------------------------
// ---- Global Variables

HANDLE       hUSB_Read_Con;                         // Handle to the USB minidriver's control read pipe
HANDLE       hUSB_Write_Con;                        // Handle to the USB minidriver's control write pipe
HANDLE       hUSB_Data;                             // Handle to the USB minidriver's data pipe

buffer       Buffer         [NUM_USB_BUFFERS];      // USB transfer buffer list

// ------------------------------------------------------------------------------------------------
// ---- External Variables

extern HWND  hWnd;                                  // The handle of the display window


// ------------------------------------------------------------------------------------------------
// ---- Static Variables

static DWORD Data_Pos       = 1;                    //


// ------------------------------------------------------------------------------
// -- close_usb_buffers

void close_usb_buffers ()
{
  short         Pos;

  buffer        *pBuffer;


  for (Pos = 0; Pos < NUM_USB_BUFFERS; Pos ++)
  {
    pBuffer  = &(Buffer [Pos]);
    LocalFree ((void**) &(pBuffer -> Data));
  } // for
} // close_usb_buffers


// ------------------------------------------------------------------------------------------------
// ---- close_usb_driver

void close_usb_driver ()
{
  if (hUSB_Read_Con  != INVALID_HANDLE_VALUE) CloseHandle (hUSB_Read_Con );
  if (hUSB_Write_Con != INVALID_HANDLE_VALUE) CloseHandle (hUSB_Write_Con);
  if (hUSB_Data      != INVALID_HANDLE_VALUE) CloseHandle (hUSB_Data     );

  close_usb_buffers ();
} // close_usb_driver


// ------------------------------------------------------------------------------------------------
// ---- create_pipe

HANDLE create_pipe (char    *pName,
                    UINT    Overlapped,
                    UINT    Flags)
{
  return CreateFile (pName, GENERIC_WRITE | GENERIC_READ,
                     FILE_SHARE_WRITE     | FILE_SHARE_READ, NULL,
                     Flags, Overlapped, NULL);
} // create_pipe


// ------------------------------------------------------------------------------------------------
// ---- download

void download (HANDLE   hUSB)
{
  char                  Error           [MAX_STRING_LENGTH];
  char                  Filename        [MAX_STRING_LENGTH];

  unsigned long         Bytes_Read      = 0;
  unsigned long         Bytes_Written   = 0;
  unsigned long         Result          = 0;

  hexline               Hex_Line;

  FILE                  *pHex_File;


  Hex_Line. Address = 0x0000;
  Hex_Line. Length  = USB_DOWNLOAD_CHUNK;

  sprintf (Filename, "C:\\Program Files\\Nebula\\DigiTV\\DigiUSB.bix");

  pHex_File = fopen (Filename, "rb");

  if (pHex_File)
  {
    while (!feof (pHex_File))
    {
      fread (Hex_Line. Data, USB_DOWNLOAD_CHUNK, 1, pHex_File);
       
      DeviceIoControl (hUSB, IOCTL_BULKUSB_DOWNLOAD,
				       &Hex_Line, sizeof (hexline), NULL, 0, &Bytes_Written, NULL);

      Hex_Line. Address += USB_DOWNLOAD_CHUNK;
    } // while

    Result = DeviceIoControl (hUSB, IOCTL_BULKUSB_RESET_DEVICE,
				              NULL, 0, NULL, 0, &Bytes_Written, NULL);
    fclose (pHex_File);
  } // if
  else
  {
    sprintf    (Error, "No DigiTV USB firmware found:\n\n%s\n\nPlease make sure that the file DigiUSB.bix is in the parent directory of the executable.", Filename);
    MessageBox (GetDesktopWindow (), Error, "No DigiTV Firmware", MB_ICONEXCLAMATION);
    //exit       (1);
  } // else
} // download


// ------------------------------------------------------------------------------
// -- init_usb_buffers

void init_usb_buffers ()
{
  short         Pos;

  buffer        *pBuffer;


  Data_Pos = 1;

  for (Pos = 0; Pos < NUM_USB_BUFFERS; Pos ++)
  {
    pBuffer  = &(Buffer [Pos]);
    ZeroMemory (&(pBuffer -> Overlapped), sizeof (OVERLAPPED));

    pBuffer -> Overlapped. hEvent = 0; 
    pBuffer -> Data               = (char*) LocalAlloc (LPTR, USB_READ_SIZE);
    pBuffer -> Serviced           = true;
  } // for
} // init_usb_buffers


// ------------------------------------------------------------------------------------------------
// ---- init_usb_device

void init_usb_device ()
{
  BOOL                  Result;

  unsigned long         Bytes_Read          = 0;


  Result = DeviceIoControl (hUSB_Read_Con,  IOCTL_BULKUSB_RESET_PIPE, NULL, 0, NULL, 0, &Bytes_Read, NULL);
  Result = DeviceIoControl (hUSB_Write_Con, IOCTL_BULKUSB_RESET_PIPE, NULL, 0, NULL, 0, &Bytes_Read, NULL);
  Result = DeviceIoControl (hUSB_Data,      IOCTL_BULKUSB_RESET_PIPE, NULL, 0, NULL, 0, &Bytes_Read, NULL);

  init_usb_buffers ();
} // init_usb_device


// ------------------------------------------------------------------------------------------------
// ---- get_usb_dev_info

void get_usb_dev_info (GUID     Device_GUID,
                       char     *pFileName,
                       short    *pPos)
{
  HDEVINFO                            hDevInfo;
  SP_DEVINFO_DATA                     DevInfoData;
  SP_DEVICE_INTERFACE_DATA            DevInterData;
  SP_DEVICE_INTERFACE_DETAIL_DATA     *pDevDetailData   = NULL;
  ULONG                               Length            = 0;
  DWORD                               Buffersize        = 0;
  DWORD                               DataT;

  bool                                Success           = false;
  char                                Buffer            [1024];

  int                                 Pos;


  if (pPos) Pos = (*pPos);
  else      Pos = 0;

  hDevInfo = SetupDiGetClassDevs ((LPGUID) &Device_GUID, NULL, NULL,
                                   DIGCF_PRESENT | DIGCF_DEVICEINTERFACE);

  DevInfoData. cbSize = sizeof (SP_DEVINFO_DATA);

  while (SetupDiEnumDeviceInfo (hDevInfo, Pos, &DevInfoData))
  {
    Buffersize = 1024;

    SetupDiGetDeviceRegistryProperty (hDevInfo,
                                      &DevInfoData,
                                      SPDRP_DEVICEDESC,
                                      &DataT,
                                      (PBYTE) Buffer,
                                      Buffersize,
                                      &Buffersize);

    if (memcmp (Buffer, "Neb", 3) == 0) break;

    Pos ++;
  } // while

  if (Buffersize > 0)
  {
    DevInterData. cbSize = sizeof (SP_DEVICE_INTERFACE_DATA);

    SetupDiEnumDeviceInterfaces     (hDevInfo, 0, (LPGUID) &Device_GUID, Pos, &DevInterData);
    SetupDiGetDeviceInterfaceDetail (hDevInfo, &DevInterData, NULL, 0, &Length, NULL);

    pDevDetailData           =        (SP_DEVICE_INTERFACE_DETAIL_DATA*) LocalAlloc (LPTR, Length);
    pDevDetailData -> cbSize = sizeof (SP_DEVICE_INTERFACE_DETAIL_DATA);

    SetupDiGetDeviceInterfaceDetail (hDevInfo, &DevInterData, pDevDetailData, Length, &Length, NULL);

    strcpy    (pFileName, pDevDetailData -> DevicePath);
    LocalFree ((void**) &pDevDetailData);
  } // if
  else
    sprintf (pFileName, "");

  SetupDiDestroyDeviceInfoList (hDevInfo);
} // get_usb_dev_info


// ------------------------------------------------------------------------------------------------
// ---- num_usb_devs

short num_usb_devs (short   *pDev_Pos)
{
  HDEVINFO                            hDevInfo;
  SP_DEVINFO_DATA                     DevInfoData;
  SP_DEVICE_INTERFACE_DETAIL_DATA     *pDevDetailData   = NULL;
  ULONG                               Length            = 0;
  DWORD                               Buffersize        = 0;
  DWORD                               DataT;
  GUID                                Device_GUID       = DEVINTERFACE_USB;

  bool                                Success           = false;
  char                                Buffer            [1024];

  int                                 Count             = 0;
  int                                 Pos               = 0;

  hDevInfo = SetupDiGetClassDevs ((LPGUID) &Device_GUID, NULL, NULL,
                                   DIGCF_PRESENT | DIGCF_DEVICEINTERFACE);

  DevInfoData. cbSize = sizeof (SP_DEVINFO_DATA);

  while (SetupDiEnumDeviceInfo (hDevInfo, Pos, &DevInfoData))
  {
    Buffersize = 1024;

    SetupDiGetDeviceRegistryProperty (hDevInfo, &DevInfoData, SPDRP_DEVICEDESC,
                                      &DataT, (PBYTE) Buffer, Buffersize, &Buffersize);

    if (memcmp (Buffer, "Neb", 3) == 0)
    {
      if (Count < MAX_DEVICES)
      {
        pDev_Pos [Count] = Pos;
        Count ++;
      } // if
    } // if

    Pos ++;
  } // while

  SetupDiDestroyDeviceInfoList (hDevInfo);

  return (Count);
} // num_usb_devs


// ------------------------------------------------------------------------------------------------
// ---- open_usb_driver

bool open_usb_driver (short     Dev_Pos)
{
  char      USB_Device      [1024];
  char      USB_ReadCon     [1024];
  char      USB_WriteCon    [1024];
  char      USB_Data        [1024];

  HANDLE    hUSB;


  get_usb_dev_info (DEVINTERFACE_USB, USB_Device, &Dev_Pos);

  sprintf (USB_WriteCon, "%s\\PIPE00", USB_Device);                      
  sprintf (USB_ReadCon,  "%s\\PIPE01", USB_Device);                      
  sprintf (USB_Data,     "%s\\PIPE02", USB_Device);                      

  hUSB = create_pipe (USB_Device, NULL, OPEN_EXISTING);

  if (hUSB != INVALID_HANDLE_VALUE)
  {
    hUSB_Read_Con = create_pipe (USB_ReadCon, NULL, OPEN_EXISTING);

    if (hUSB_Read_Con == INVALID_HANDLE_VALUE)
    {
      download    (hUSB);
      Sleep       (1000);
      CloseHandle (hUSB);

      do
      {
        Sleep (500);
        get_usb_dev_info (DEVINTERFACE_USB, USB_Device, &Dev_Pos);
      }
      while (strlen (USB_Device) == 0);

    } // if
    else
      CloseHandle (hUSB_Read_Con);

    sprintf (USB_WriteCon, "%s\\PIPE00", USB_Device);                      
    sprintf (USB_ReadCon,  "%s\\PIPE01", USB_Device);                      
    sprintf (USB_Data,     "%s\\PIPE02", USB_Device);                      

    hUSB_Read_Con  = create_pipe (USB_ReadCon,  NULL, OPEN_EXISTING);
    hUSB_Write_Con = create_pipe (USB_WriteCon, NULL, OPEN_EXISTING);
    hUSB_Data      = create_pipe (USB_Data,     FILE_FLAG_OVERLAPPED, OPEN_EXISTING);

    reset_pipe (hUSB_Read_Con );
    reset_pipe (hUSB_Write_Con);
    reset_pipe (hUSB_Data     );
  } // if
  else
  {
    return (false);
  }

  if (hUSB_Data == INVALID_HANDLE_VALUE) return (false);
  else                                   return (true );
} // open_usb_driver



// ------------------------------------------------------------------------------
// -- reset_pipe

void reset_pipe (HANDLE    hPipe)
{
  unsigned long     Bytes   = 0;


  if (hPipe != INVALID_HANDLE_VALUE)
  {
    DeviceIoControl (hPipe, IOCTL_BULKUSB_RESET_PIPE, NULL, 0, NULL, 0, &Bytes, NULL);
  } // if
} // reset_pipe



// ------------------------------------------------------------------------------
// -- reset_usb_buffers

void reset_usb_buffers ()
{
  short         Pos;

  buffer        *pBuffer;


  Data_Pos = 1;

  for (Pos = 0; Pos < NUM_USB_BUFFERS; Pos ++)
  {
    pBuffer  = &(Buffer [Pos]);
    ZeroMemory (&(pBuffer -> Overlapped), sizeof (OVERLAPPED));

    pBuffer -> Overlapped. hEvent = 0; 
    pBuffer -> Serviced           = true;
  } // for
} // reset_usb_buffers


// ------------------------------------------------------------------------------
// -- signal_usb_read

void signal_usb_read (HANDLE    hRead)
{
  short         Pos;

  buffer        *pBuffer;
 

  for (Pos = 0; Pos < NUM_USB_BUFFERS; Pos ++)
  {
    pBuffer  = &(Buffer [Pos]);

    if (pBuffer -> Serviced)
    {
      pBuffer -> Serviced           = false;
      pBuffer -> Overlapped. hEvent = (HANDLE) 1;
      pBuffer -> Overlapped. Offset = Data_Pos;

      ReadFileEx (hRead, pBuffer -> Data, USB_READ_SIZE, &(pBuffer -> Overlapped), usb_read_complete);

      Data_Pos ++;
    } // if
  } // for
} // signal_usb_read


// ------------------------------------------------------------------------------
// -- transfer_data

bool transfer_data (HANDLE      hRead,
                    char        *pData)
{
  short     Pos;

  DWORD     Min_Pos     = (DWORD) (-1);
  DWORD     Position    = 0;

  buffer    *pBuffer;
  buffer    *pTransfer  = NULL;
 

  for (Pos = 0; Pos < NUM_USB_BUFFERS; Pos ++)
  {
    pBuffer  = &(Buffer [Pos]);

    if (pBuffer -> Overlapped. hEvent == 0)
    {
      Position = (UINT) pBuffer -> Overlapped. Offset;

      if (Position < Min_Pos)
      {
        pTransfer = pBuffer;
        Min_Pos   = Position;
      } // if
    } // if
  } // for

  if (pTransfer != NULL)
  {
    memcpy (pData, pTransfer -> Data, USB_READ_SIZE);
    pTransfer -> Serviced = true;
    return (true);
  } // if
  else
    return (false);
} // transfer_data


// ------------------------------------------------------------------------------
// -- usb_read_complete

void CALLBACK usb_read_complete (DWORD       Code,
                                 DWORD       Bytes_Read,
                                 OVERLAPPED  *pOverlapped)
{
  pOverlapped -> hEvent = 0;
} // usb_read_complete
