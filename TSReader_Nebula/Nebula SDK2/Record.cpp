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

#include <winioctl.h>
#include <atlbase.h>
#include <process.h> 
#include <stdio.h>
#include <fcntl.h>
#include <io.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "setupapi.h"
#include "defs.h"
#include "device.h"
#include "record.h"

#include "sdk.h"
#include "tuner.h"

#include "PCIDriver.h"
#include "USBDriver.h"


// ------------------------------------------------------------------------------------------------
// ---- Externals

extern bool         Recording;                                  // True when recording the transport stream

extern char         Save_File               [];                 // The name of the saved transport stream

extern int          Variant;                                    // Which variant of the hardware is installed


// ------------------------------------------------------------------------------------------------
// ---- get_data_pci

int    get_data_pci (byte      *pBuffer)
{
  BOOL      Success;

  int       Pos         = 0;
  int       Size;


  ((pci_data*) pBuffer) -> Num_Bytes = 0;

  do
  {
    Success = read_property (AUDIO, AUDIO_TS_DATA, pBuffer, sizeof (pci_data));

    if (!Success) Sleep (10);
    Pos   ++;
  }
  while (!Success && (Pos < 50) && Recording);

  Size = ((pci_data*) pBuffer) -> Num_Bytes;

  return (Size);
} // get_data_pci


// ------------------------------------------------------------------------------------------------
// ---- get_data_usb

int    get_data_usb (byte      *pBuffer)
{
  BOOL      Success;

  int       Pos         = 0;
  int       Size;


  do
  {
    Success = read_usb_data (pBuffer);

    if (!Success) SleepEx (10, TRUE);
    Pos   ++;
  }
  while (!Success && (Pos < 50) && Recording);

  Size = ((usb_data*) pBuffer) -> Num_Bytes;

  return (Size);
} // get_data_usb


// ------------------------------------------------------------------------------------------------
// ---- ts_record

void    ts_record (LPVOID  lpParameter)
{
  bool           First           = true;

  byte           *pBuffer        = NULL;
  byte           *pData          = NULL;

  int            Bytes_Read      = 0;
  int            hTSFile         = -1;


  reset_usb_buffers ();

  switch (Variant)
  {
    case PCI_VARIANT : pBuffer = (byte*) malloc (sizeof (pci_data));
                       pData   = ((pci_data*) pBuffer) -> Data;
                       break;

    case USB_VARIANT : pBuffer = (byte*) malloc (sizeof (usb_data));
                       pData   = ((usb_data*) pBuffer) -> Data;
                       break;
  } // switch

  hTSFile = _open (Save_File, _O_CREAT | _O_RDWR | _O_TRUNC | _O_SEQUENTIAL | _O_BINARY, _S_IREAD | _S_IWRITE);

  while (Recording && pBuffer)
  {
    switch (Variant)
    {
      case PCI_VARIANT : Bytes_Read = get_data_pci (pBuffer); break;
      case USB_VARIANT : Bytes_Read = get_data_usb (pBuffer); break;
    } // switch

    write_ts (hTSFile, pData, Bytes_Read, &First);
  } // while

  if (pBuffer     ) free   (pBuffer);
  if (hTSFile >= 0) _close (hTSFile);
} // ts_live_thread


// ----------------------------------------------------------------------------
// ---- write_ts

void write_ts (int      Handle,
               byte     *pBuffer,
               int      Length,
               bool     *pFirst)
{
  int       Pos         = 0;


  if (Handle >= 0)
  {
    for (Pos = 0; (Pos < Length) && *pFirst; Pos ++)
    {
      if ((pBuffer [Pos      ] == 0x47 ) && 
          (pBuffer [Pos + 188] == 0x47)) break;
    } // for

    _write (Handle, &(pBuffer [Pos]), Length - Pos);
  } // if

  *pFirst = false;
} // write_ts 
