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
// ---- Prototypes

void    close_pci_driver    ();                     // Close the PCI driver
void    init_pci_device     ();                     // Initialise the connected PCI hardware
void    get_dev_info        (GUID, char*, short*);  // Find the installed Audio & Video driver names
bool    open_pci_driver     ();                     // Open the PCI driver
