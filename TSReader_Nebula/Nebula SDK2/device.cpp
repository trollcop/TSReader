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

#include "setupapi.h"
#include "defs.h"
#include "device.h"

#include "digiusb.h"
#include "record.h"
#include "sdk.h"
#include "tuner.h"

#include "pcidriver.h"
#include "usbdriver.h"


// ------------------------------------------------------------------------------------------------
// ---- Globals

fusion_register     Register;                           //


// ------------------------------------------------------------------------------------------------
// ---- Externals

extern short        COFDM_Type;                         // The installed COFDM chip

extern int          Variant;                            // Which variant of the hardware is installed

extern HANDLE       hAudio;                             // Handle to the Audio minidriver
extern HANDLE       hVideo;                             // Handle to the Video minidriver
extern HANDLE       hUSB_Read_Con;                      // Handle to the USB minidriver's control read pipe
extern HANDLE       hUSB_Write_Con;                     // Handle to the USB minidriver's control write pipe
extern HANDLE       hUSB_Data;                          // Handle to the USB minidriver's data pipe

extern buffer       Buffer          [];                 // USB transfer buffer list


// ------------------------------------------------------------------------------------------------
// ---- Statics

static byte         I2C_Device      = I2C_COFDM_ADDR;   // Current destination for i2c transfers

static float        Tuner_frequency = 0;                // Current tuner frequency


// ------------------------------------------------------------------------------------------------
// ---- Function Bodies
// ------------------------------------------------------------------------------------------------

// ------------------------------------------------------------------------------------------------
// ---- get_tuner_frequency

float get_tuner_frequency ()
{
  return (Tuner_frequency);
} // get_tuner_frequency


// ------------------------------------------------------------------------------------------------
// ---- read_eeprom

reg_data read_eeprom (reg_address      Address)
{
  reg_data  Data;


  set_i2c_device  (I2C_EEPROM_ADDR);                // Set the current i2c device to the EEPROM

  Data = read_i2c (Address);                        // Read the byte from the EEPROM

  set_i2c_device  (I2C_COFDM_ADDR);                 // Set the current i2c device back to the COFDM

  return          (Data);
} // read_eeprom


// ------------------------------------------------------------------------------------------------
// ---- read_i2c

reg_data read_i2c (reg_address      Address)
{
  short     Count   = 0;

  reg_data  Data;


  switch (Variant)
  {
    case PCI_VARIANT : write_i2c_addr (Address);                         // Setup the i2c register address
                       write_reg      (VIDEO, REG_INT_STAT, I2C_DONE);   // Clear the I2C_DONE flag in INT_STAT

                       Data  = I2C_Device << 24;                         // Construct the i2c address
                       Data |= Address    << 16;                         // Construct the i2c sub-address
                       Data |= (1         << 24);                        // I2c read mode (make sure bit 24 is set)
                       Data |= I2C_FLAGS;

                       write_reg     (VIDEO, REG_I2C_CON, Data);
                       wait_i2c_done ();

                       Data = (read_reg (VIDEO, REG_I2C_CON) & 0x0000ff00) >> 8;
                       break;

    case USB_VARIANT : switch (I2C_Device)
                       {
                         case I2C_COFDM_ADDR  : Data = read_usb_i2c (USB_READ_COFDM,  Address); break;
                         case I2C_EEPROM_ADDR : Data = read_usb_i2c (USB_READ_EEPROM, Address); break;
                       } // switch
                       break;
  } // switch

  return (Data);
} // read_i2c


// ------------------------------------------------------------------------------------------------
// ---- read_reg

reg_data read_reg (short            Function,
                   reg_address      Address)
{
  DWORD     Property_Get;
  DWORD     Property_Set;

  Register. Address = Address;

  switch (Function)
  {
    case AUDIO : Property_Set = AUDIO_SET_ADDR;
                 Property_Get = AUDIO_READ_DATA; break;
    case VIDEO : Property_Set = VIDEO_SET_ADDR;
                 Property_Get = VIDEO_READ_DATA; break;
  } // switch

  write_property (Function, Property_Set, (void*) &Register);
  read_property  (Function, Property_Get, (void*) &Register, sizeof (fusion_register));

  return (Register. Data);
} // read_reg


// ------------------------------------------------------------------------------------------------
// ---- read_property

BOOL read_property (short               Function,
                    DWORD               Property,
                    void                *pData,
                    int                 Size)
{
  BOOL                  Result      = false;
  unsigned long         Bytes_Read  = 0;
  unsigned long         BytesToRead = 0;
  unsigned long         PipeNum     = 0;

  fusion_register       *pRegister  = (fusion_register*) pData;

  HANDLE                hDevice;


  switch (Variant)
  {
    case PCI_VARIANT : hDevice   = (Function == VIDEO) ? hVideo : hAudio;

                       pRegister -> Property. Set   = PROPSETID_VIDCAP_CUSTOMBT848;
                       pRegister -> Property. Id    = Property;
                       pRegister -> Property. Flags = KSPROPERTY_TYPE_GET;

                       Result = DeviceIoControl (hDevice, IOCTL_KS_PROPERTY,
                                                 pData, Size,
                                                 pData, Size,
                                                 &Bytes_Read, NULL);
                       break;
 
    case USB_VARIANT : if (Function == USB) ReadFile (hUSB_Read_Con, pData, Size, &Bytes_Read, NULL);
                       break;
  } // switch

  return (Result);
} // read_property


// ------------------------------------------------------------------------------------------------
// ---- read_usb_data

BOOL read_usb_data (void    *pData)
{
  bool      Success;

  int       Num_Bytes   = USB_READ_SIZE;


  signal_usb_read (hUSB_Data);
  Success = transfer_data (hUSB_Data, (char*) pData);

  ((usb_data*) pData) -> Num_Bytes = Num_Bytes;

  return (Success);
} // read_usb_data


// ------------------------------------------------------------------------------------------------
// ---- read_usb_i2c

reg_data read_usb_i2c (short           Function,
                       reg_address     Address)
{
  usb_register  USB_Reg;


  USB_Reg. Command   = (byte) Function;
  USB_Reg. Address   = (byte) Address;
  USB_Reg. Num_Bytes = 1;

  write_property (USB, 0, &USB_Reg);
  Sleep          (10);
  read_property  (USB, 0, &USB_Reg, sizeof (USB_Reg));

  return (USB_Reg. Data [0]);
} // read_usb_i2c


// ------------------------------------------------------------------------------------------------
// ---- read_usb_remote

DWORD read_usb_remote ()
{
  DWORD         Data;

  usb_register  USB_Reg;


  USB_Reg. Command   = USB_READ_REMOTE;
  USB_Reg. Address   = 0;
  USB_Reg. Num_Bytes = 4;
  USB_Reg. Data [0]  = 0;
  USB_Reg. Data [1]  = 0;
  USB_Reg. Data [2]  = 0;
  USB_Reg. Data [3]  = 0;

  write_property (USB, 0, &USB_Reg);
  Sleep          (10);
  read_property  (USB, 0, &USB_Reg, sizeof (USB_Reg));

  Data =  ((DWORD) (USB_Reg. Data [0]))        +
         (((DWORD) (USB_Reg. Data [1])) << 8 ) + 
         (((DWORD) (USB_Reg. Data [2])) << 16) + 
         (((DWORD) (USB_Reg. Data [3])) << 24);

  return (Data);
} // read_usb_remote


// ------------------------------------------------------------------------------------------------
// ---- set_i2c_device

void set_i2c_device (byte  Device)
{
  switch (Device)
  {
    case I2C_COFDM_ADDR   : I2C_Device = I2C_COFDM_ADDR;

                            if (COFDM_Type == NXT_6000) write_i2c (NXT_TUNER_I2C_EN, 0x00);
                            break;

    case I2C_TDED_ADDR    : if (COFDM_Type == NXT_6000)
                            {
                              I2C_Device = I2C_COFDM_ADDR;
                              write_i2c (NXT_TUNER_I2C_EN, 0x01);
                            } // if

                            I2C_Device = I2C_TDED_ADDR;
                            break;

    case I2C_EEPROM_ADDR  : I2C_Device = I2C_EEPROM_ADDR;
                            break;
  } // switch
} // set_i2c_device


// ------------------------------------------------------------------------------------------------
// ---- set_tuner_frequency

void set_tuner_frequency (float  Frequency,
                          short  Bandwidth)
{
  int   Band        = 0;
  int   Tuner_Ctl   = 0;
  int   PLL_Value   = (int) (((Frequency + 36.16666) / 0.16666) + 0.5);
  int   Tuner_Band  = 0;


  Tuner_frequency = Frequency;
  Tuner_Band      = Bandwidth;

  switch (Tuner_Band)
  {
    case 7 : if (COFDM_Type == NXT_6000)
             {
               write_i2c (NXT_OFDM_TRL_RATE1, 0x00);        // Channel Bandwidth = 7MHz Fadc = 20.48MHz
               write_i2c (NXT_OFDM_TRL_RATE2, 0x64);        //               "
             } // if
             else
             {
               write_i2c (ZAR_TRL_NOMINAL_RATE_0, 0x00);    // Channel Bandwidth = 7MHz Fadc = 20.48MHz
               write_i2c (ZAR_TRL_NOMINAL_RATE_1, 0x64);    //               "
             } // else

             Band = 0x0000;
             break;

    case 8 : if (COFDM_Type == NXT_6000)
             {
               write_i2c (NXT_OFDM_TRL_RATE1, 0x49);        // Channel Bandwidth = 8MHz Fadc = 20.48MHz
               write_i2c (NXT_OFDM_TRL_RATE2, 0x72);        //               "
             } // if
             else
             {
               write_i2c (ZAR_TRL_NOMINAL_RATE_0, 0x49);    // Channel Bandwidth = 8MHz Fadc = 20.48MHz
               write_i2c (ZAR_TRL_NOMINAL_RATE_1, 0x72);    //               "
             } // else

             Band = 0x0004;
             break;
  } // switch

  if      (Frequency < 470) Tuner_Ctl = 0x8502 | Band;
  else if (Frequency > 823) Tuner_Ctl = 0x8588 | Band;
  else                      Tuner_Ctl = 0x8508 | Band;

  write_tuner ((PLL_Value << 16) | Tuner_Ctl);

  if (Variant == USB_VARIANT) reset_pipe (hUSB_Data);
} // set_tuner_frequency


// ------------------------------------------------------------------------------------------------
// ---- wait_i2c_done

void wait_i2c_done ()
{
  short     Count   = 0;

  reg_data  Data;

  do
  {
    Data  = read_reg (VIDEO, REG_INT_STAT);
    Count ++;
    Sleep (1);
  } 
  while (!(Data & I2C_DONE) && (Count < 100));
} // wait_i2c_done


// ------------------------------------------------------------------------------------------------
// ---- write_i2c

void write_i2c (reg_address      Address,
                reg_data         New_Data)
{
  reg_data  Data;


  switch (Variant)
  {
    case PCI_VARIANT : write_reg (VIDEO, REG_INT_STAT, I2C_DONE);      // Clear the I2C_DONE flag in INT_STAT

                       Data  = I2C_Device << 24;                       // Construct the i2c address
                       Data |= Address    << 16;                       // Construct the i2c sub-address
                       Data |= New_Data   <<  8;                       // Construct the i2c data
                       Data &= ~(1        << 24);                      // I2c write mode (make sure bit 24 is reset)
                       Data |= (I2C_FLAGS | I2C_THREE_BYTES);

                       write_reg     (VIDEO, REG_I2C_CON, Data);
                       wait_i2c_done ();
                       break;

    case USB_VARIANT : switch (I2C_Device)
                       {
                         case I2C_COFDM_ADDR  : write_usb_i2c (USB_WRITE_COFDM,  Address, New_Data, 1); break;
                       } // switch
                       break;
  } // switch
} // write_i2c


// ------------------------------------------------------------------------------------------------
// ---- write_i2c_addr

void write_i2c_addr (reg_address      Address)
{
  reg_data  Data;


  write_reg (VIDEO, REG_INT_STAT, I2C_DONE);        // Clear the I2C_DONE flag in INT_STAT

  Data  = I2C_Device << 24;                         // Construct the i2c address
  Data |= Address    << 16;                         // Construct the i2c sub-address
  Data &= ~(1        << 24);                        // I2c write mode (make sure bit 24 is reset)
  Data |= I2C_FLAGS | I2C_NO_STOP;

  write_reg     (VIDEO, REG_I2C_CON, Data);
  wait_i2c_done ();
} // write_i2c_addr


// ------------------------------------------------------------------------------------------------
// ---- write_property

void write_property (short               Function,
                     DWORD               Property,
                     void                *pData)
{
  unsigned int      Result;
  unsigned long     Bytes_Read;

  fusion_register   *pRegister  = (fusion_register* ) pData;

  HANDLE            hDevice;


  switch (Variant)
  {
    case PCI_VARIANT : hDevice    = (Function == VIDEO) ? hVideo : hAudio;

                       pRegister -> Property. Set   = PROPSETID_VIDCAP_CUSTOMBT848;
                       pRegister -> Property. Id    = Property;
                       pRegister -> Property. Flags = KSPROPERTY_TYPE_SET;

                       Result = DeviceIoControl (hDevice, IOCTL_KS_PROPERTY,
                                                 pData, sizeof  (fusion_register),
                                                 pData, sizeof  (fusion_register),
                                                  &Bytes_Read,
                                                NULL);
                       break;

    case USB_VARIANT : if (Function == USB) WriteFile (hUSB_Write_Con, pData, sizeof (usb_register), &Bytes_Read, NULL);
                       break;
  } // switch
} // write_property


// ------------------------------------------------------------------------------------------------
// ---- write_reg

void write_reg (short           Function,
                reg_address     Address,
                reg_data        Data)
{
  DWORD     Property;


  Register. Address  = Address;
  Register. Data     = Data;

  switch (Function)
  {
    case AUDIO : Property = AUDIO_WRITE_DATA;  break;
    case VIDEO : Property = VIDEO_WRITE_DATA;  break;
  } // switch

  write_property (Function, Property, &Register);
} // write_reg


// ------------------------------------------------------------------------------------------------
// ---- write_tuner

void write_tuner (reg_data         New_Data)
{
  int       PLL_Value   = (New_Data & 0xffff0000) >> 16;
  int       Tuner_Ctl   =  New_Data & 0x0000ffff;

  reg_data  Data;

  byte      Byte [4];


  if (COFDM_Type == NXT_6000) set_i2c_device (I2C_TDED_ADDR);            // Set the current i2c device to the TDED tuner

  switch (Variant)
  {
    case PCI_VARIANT : if (COFDM_Type == NXT_6000)
                       {
                         Byte [0] = (byte) ((New_Data & 0xff000000) >> 24);
                         Byte [1] = (byte) ((New_Data & 0x00ff0000) >> 16);
                         Byte [2] = (byte) ((New_Data & 0x0000ff00) >> 8);
                         Byte [3] = (byte) ((New_Data & 0x000000ff));

                         write_reg (VIDEO, REG_INT_STAT, I2C_DONE);        // Clear the I2C_DONE flag in INT_STAT

                         Data  = I2C_TDED_ADDR << 24;                      // Construct the i2c address
                         Data |= Byte [0]      << 16;                      // Construct the 1st data byte
                         Data &= ~(1           << 24);                     // I2c write mode (make sure bit 24 is reset)
                         Data |= (I2C_FLAGS | I2C_NO_STOP);

                         write_reg      (VIDEO, REG_I2C_CON, Data);        // Send the 1st data byte
                         wait_i2c_done  ();

                         Data  = Byte [1] << 24;                           // Construct the 2nd data byte
                         Data |= (I2C_FLAGS | I2C_NO_STOP | I2C_NO_START); //
                         write_reg      (VIDEO, REG_I2C_CON, Data);        // Send it
                         wait_i2c_done  ();

                         Data  = Byte [2] << 24;                           // Construct the 3rd data byte
                         Data |= (I2C_FLAGS | I2C_NO_STOP | I2C_NO_START); //
                         write_reg      (VIDEO, REG_I2C_CON, Data);        // Send it
                         wait_i2c_done  ();

                         Data  = Byte [3] << 24;                           // Construct the 4th data byte
                         Data |= (I2C_FLAGS | I2C_NO_START);               //
                         write_reg      (VIDEO, REG_I2C_CON, Data);        // Send it
                         wait_i2c_done  ();
                       } // if

                       else
                       {
                         write_i2c (ZAR_CHAN_START_0,  PLL_Value       & 0xff); 
                         write_i2c (ZAR_CHAN_START_1, (PLL_Value >> 8) & 0x7f); 
                         write_i2c (ZAR_CONT_0,        Tuner_Ctl       & 0xff); 
                         write_i2c (ZAR_CONT_1,       (Tuner_Ctl >> 8) & 0xff); 
                         write_i2c (ZAR_TUNER_GO,      0x01); 
                       } // else
                       break;

    case USB_VARIANT : if (COFDM_Type == NXT_6000)
                       {
                         write_usb_i2c (USB_WRITE_TUNER, 0, New_Data, 4);
                       } // if

                       else
                       {
                         write_i2c (ZAR_CHAN_START_0,  PLL_Value       & 0xff); 
                         write_i2c (ZAR_CHAN_START_1, (PLL_Value >> 8) & 0x7f); 
                         write_i2c (ZAR_CONT_0,        Tuner_Ctl       & 0xff); 
                         write_i2c (ZAR_CONT_1,       (Tuner_Ctl >> 8) & 0xff); 
                         write_i2c (ZAR_TUNER_GO,      0x01); 
                       } // else
                       break;
  } // switch

  set_i2c_device (I2C_COFDM_ADDR);                      // Set the current i2c device back to the COFDM
} // write_tuner


// ------------------------------------------------------------------------------------------------
// ---- write_usb_i2c

void write_usb_i2c (short           Function,
                    reg_address     Address,
                    reg_data        Data,
                    short           Count)
{
  short         Pos;
  short         Rev_Pos;

  reg_data      Mask    = 0x000000ff;

  usb_register  USB_Reg;

  USB_Reg. Command   = (byte) Function;
  USB_Reg. Address   = (byte) Address;
  USB_Reg. Num_Bytes = (byte) Count;

  for (Pos = 0; Pos < Count; Pos ++)
  {
    Rev_Pos = (Count - 1) - Pos;
    Mask    = 0x000000ff << (Rev_Pos * 8);

    USB_Reg. Data [Pos] = (byte) ((Data & Mask) >> (Rev_Pos * 8));
  } // for

  write_property (USB, 0, &USB_Reg);
} // write_usb_i2c


// ------------------------------------------------------------------------------------------------
// ---- write_usb_remote

void write_usb_remote (byte     Command,
                       DWORD    Value)
{
  usb_register  USB_Reg;


  USB_Reg. Command   = Command;
  USB_Reg. Address   = 0;
  USB_Reg. Num_Bytes = 4;
  USB_Reg. Data [0]  = (byte)  (Value & 0x000000ff);
  USB_Reg. Data [1]  = (byte) ((Value & 0x0000ff00) >> 8 );
  USB_Reg. Data [2]  = (byte) ((Value & 0x00ff0000) >> 16);
  USB_Reg. Data [3]  = (byte) ((Value & 0xff000000) >> 24);

  write_property (USB, 0, &USB_Reg);
} // write_usb_remote


