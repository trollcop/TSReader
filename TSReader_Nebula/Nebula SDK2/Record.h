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
// ---- Types

// ------------------------------------------------------------------------------------------------

typedef struct
{
  KSPROPERTY    Property;

  DWORD         Num_Bytes;
  BYTE          Data        [PCI_BUFFER_SIZE];
} pci_data;

// ------------------------------------------------------------------------------------------------

typedef struct
{
  BYTE          Data        [USB_BUFFER_SIZE];

  DWORD         Num_Bytes;
} usb_data;



// ------------------------------------------------------------------------------------------------
// ---- Prototypes

int     get_data_pci            (byte*);                    // Get a data packet from the PCI card
int     get_data_usb            (byte*);                    // Get a data packet from the USB device
void	ts_record               (LPVOID);                   // Thread that reads & records the live Transport Stream data
void    write_ts                (int, byte*, int, bool*);   // Write the transport stream to the specified file
