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
#include "sdk.h"   
#include "tuner.h"   

#include "pcidriver.h"


// ------------------------------------------------------------------------------------------------
// ---- External Variables


// ------------------------------------------------------------------------------------------------
// ---- Global Variables

HANDLE              hAudio;                     // Handle to the Audio minidriver
HANDLE              hVideo;                     // Handle to the Video minidriver


// ------------------------------------------------------------------------------------------------
// ---- Static Variables


// ------------------------------------------------------------------------------------------------
// ---- open_pci_driver

bool open_pci_driver ()
{
  char  Audio_Device  [1024];
  char  Video_Device  [1024];
 

  get_dev_info (DEVINTERFACE_PCI_VIDEO, Video_Device, 0);
  get_dev_info (DEVINTERFACE_PCI_AUDIO, Audio_Device, 0);

  hAudio  = CreateFile (Audio_Device,0,
                        FILE_SHARE_READ | FILE_SHARE_WRITE,
                        NULL, OPEN_EXISTING, 0, NULL);

  hVideo  = CreateFile (Video_Device, 0,
                        FILE_SHARE_READ | FILE_SHARE_WRITE,
                        NULL, OPEN_EXISTING, 0, NULL);
 
  if ((hAudio != INVALID_HANDLE_VALUE) && 
      (hVideo != INVALID_HANDLE_VALUE)) return (true );
  else                                  return (false);
} // open_pci_driver


// ------------------------------------------------------------------------------------------------
// ---- close_pci_driver

void close_pci_driver ()
{
  close_tuner ();

  if (hAudio != INVALID_HANDLE_VALUE) CloseHandle (hAudio); 
  if (hVideo != INVALID_HANDLE_VALUE) CloseHandle (hVideo); 
} // close_pci_driver


// ------------------------------------------------------------------------------------------------
// ---- init_pci_device

void init_pci_device ()
{
} // init_pci_device


// ------------------------------------------------------------------------------------------------
// ---- get_dev_info

void get_dev_info (GUID     Device_GUID,
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

    if (memcmp (Buffer, "Neb", 3) == 0)
    {
      Success = true;
      break;
    } // if

    Pos ++;
  } // while

  if (Success & (Buffersize > 0))
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
} // get_dev_info


