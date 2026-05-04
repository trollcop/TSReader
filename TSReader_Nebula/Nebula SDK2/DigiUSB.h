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

#ifndef _BULKUSB_USER_H
#define _BULKUSB_USER_H

#include <initguid.h>

DEFINE_GUID (DEVINTERFACE_USB, 0x873fdf, 0x61a8, 0x11d1, 0xaa, 0x5e, 0x0, 0xc0, 0x4f, 0xb1, 0x72, 0x8b);

#define BULKUSB_IOCTL_INDEX                 0x0000
#define IOCTL_BULKUSB_GET_CONFIG_DESCRIPTOR CTL_CODE (FILE_DEVICE_UNKNOWN, BULKUSB_IOCTL_INDEX,     METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_BULKUSB_RESET_DEVICE          CTL_CODE (FILE_DEVICE_UNKNOWN, BULKUSB_IOCTL_INDEX + 1, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_BULKUSB_RESET_PIPE            CTL_CODE (FILE_DEVICE_UNKNOWN, BULKUSB_IOCTL_INDEX + 2, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_BULKUSB_DOWNLOAD              CTL_CODE (FILE_DEVICE_UNKNOWN, BULKUSB_IOCTL_INDEX + 3, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_BULKUSB_UPLOAD                CTL_CODE (FILE_DEVICE_UNKNOWN, BULKUSB_IOCTL_INDEX + 4, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_BULKUSB_DETACH                CTL_CODE (FILE_DEVICE_UNKNOWN, BULKUSB_IOCTL_INDEX + 5, METHOD_BUFFERED, FILE_ANY_ACCESS)
#endif

