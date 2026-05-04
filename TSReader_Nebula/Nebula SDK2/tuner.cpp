#include "stdafx.h"

#include <winioctl.h>
#include <atlbase.h>
#include <process.h> 
#include <stdio.h>
//#include <dshow.h>
#include <ks.h>

#include "setupapi.h"
#include "defs.h"
#include "device.h"

#include "sdk.h"
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

#include "tuner.h"

#include "PCIDriver.h"
#include "USBDriver.h"


// ------------------------------------------------------------------------------------------------
// ---- Global Variables

short               Tune_BWidth     = 8;                    // Channel bandwidth
short               Tune_Gate       = 800;                  // Time (mS) to wait for a tuning lock


// ------------------------------------------------------------------------------------------------
// ---- Externals

extern  short       COFDM_Type;                             // The installed COFDM chip
extern  short       Language;                               // The current language preference
extern  short       Max_Disp_Channels;                      // The number of currently tuned channels

extern  int         Variant;                                // Which variant of the hardware is installed


// ------------------------------------------------------------------------------------------------
// ---- Statics

static  short        Channel_No          = 0;                // The current channel number


// ------------------------------------------------------------------------------------------------
// ---- Function Bodies
// ------------------------------------------------------------------------------------------------


// ------------------------------------------------------------------------------------------------
// ---- close_tuner

void close_tuner ()
{
} // close_tuner


// ------------------------------------------------------------------------------------------------
// ---- channel_locked

bool channel_locked (tune_info  *pTune,
                     bool       Wait)
{
  byte      Pos         = 0;

  reg_data  Status      = 0;

  DWORD     Elapsed_mS;


  get_system_time (true);

  do 
  {
    Elapsed_mS = get_system_time (false);
    Sleep (1);

    pTune -> AGC_Lock = false;
    pTune -> TPS_Lock = false;
    pTune -> VIT_Lock = false;
    pTune -> FEC_Lock = false;

    if (COFDM_Type == NXT_6000)
    {
      Status            = read_i2c (NXT_OFDM_CORE_STAT );
      pTune -> AGC_Lock = (Status & 0x10) ? true : false;
      pTune -> TPS_Lock = (Status & 0x40) ? true : false;
    } // if
    else
    {
      Status            = read_i2c (ZAR_STATUS_0);
      pTune -> AGC_Lock = (Status & 0x01) ? true : false;
      pTune -> TPS_Lock = (Status & 0x02) ? true : false;
    } // else

    if (pTune -> AGC_Lock && pTune -> TPS_Lock)
    {
      if (COFDM_Type == NXT_6000)
      {
        Status            = read_i2c (NXT_VIT_SYNC_STATUS);
        pTune -> VIT_Lock =  (Status & 0x80) ? true : false;
      } // if
      else
      {
        pTune -> VIT_Lock =  (Status & 0x08) ? true : false;
      } // else
    } // if

    if (pTune -> VIT_Lock) 
    {
      if (COFDM_Type == NXT_6000)
      {
        Status            = read_i2c (NXT_RS_COR_STAT);
        pTune -> FEC_Lock = ((Status & 0x03) == 0x03) ? true : false;
      } // if 
      else
      {
        pTune -> FEC_Lock = (Status & 0x20) ? true : false;
      } // else
    } // if
  } 
  while ((Wait && (Elapsed_mS < (DWORD) Tune_Gate) &&
         (!pTune -> VIT_Lock || !pTune -> FEC_Lock)) ||

         (Wait && (Elapsed_mS < (DWORD) Tune_Gate * 2) &&
        ((!pTune -> AGC_Lock || !pTune -> TPS_Lock) ||
         (!pTune -> VIT_Lock || !pTune -> FEC_Lock))));

  if (pTune -> AGC_Lock && pTune -> TPS_Lock && 
      pTune -> VIT_Lock && pTune -> FEC_Lock)
  {
    pTune -> SNR = get_snr ();
    pTune -> BER = get_ber ();
    pTune -> AGC = get_agc ();

    if (COFDM_Type == NXT_6000)
    {
      pTune -> Modulation = (short) read_i2c (NXT_TPS_RCVD2) & 0x03;
      pTune -> Mode       = (short) read_i2c (NXT_TPS_RCVD4) & 0x01;
    } // if
    else
    {
      pTune -> Modulation = (short) (read_i2c (ZAR_TPS_CURRENT_1) & 0x60) >> 5;
      pTune -> Mode       = (short)  read_i2c (ZAR_TPS_CURRENT_0) & 0x01;
    } // else

    return (true);
  } // if
  else
  {
    pTune -> SNR        = 0;
    pTune -> BER        = 0;
    pTune -> AGC        = 0;
    pTune -> Mode       = 0;
    pTune -> Modulation = 0x03;

    return (false);
  } // if
} // channel_locked


// ------------------------------------------------------------------------------------------------
// ---- get_agc

short get_agc ()
{
  short     AGC;

  if (COFDM_Type == NXT_6000)
  {
    AGC = (short) (511 - (read_i2c (NXT_AGC_GAIN_1) + 
                        ((read_i2c (NXT_AGC_GAIN_2) & 0x03) << 8)));
  } // if
  else
  {
    AGC = (short) (511 - ((((read_i2c (ZAR_AGC_GAIN_2) & 0xc0) >> 6) + 
                           ((read_i2c (ZAR_AGC_GAIN_3) & 0xff) << 2)) >> 3));
  } // else

  return (AGC);
} // get_agc


// ------------------------------------------------------------------------------------------------
// ---- get_ber

float get_ber ()
{
  static double     Last_Rate   = 0;

  int               Count;

  double            Rate;


  if (COFDM_Type == NXT_6000)
  {
    Count = (read_i2c (NXT_VIT_BER_1) << 8) | 
             read_i2c (NXT_VIT_BER_0);
    Rate  = (double) Count / (double) (NXT_BER_TIME * 256);

    write_i2c (NXT_VIT_CORE_INTSTAT, 0x18);                               // Clear BER Done interrupts
  } // if
  else
  {
    if (read_i2c (ZAR_INTERRUPT_1) & 0x80)
    {
      Count = ((int) read_i2c (ZAR_RS_ERR_CNT_2) << 16) |
              ((int) read_i2c (ZAR_RS_ERR_CNT_1) <<  8) |
               (int) read_i2c (ZAR_RS_ERR_CNT_0);
      Rate  = (double) Count / (double) (ZAR_BER_TIME * 1024);
    } // if
    else
      Rate  = Last_Rate;
  } // else

  Last_Rate = Rate;

  return ((float) Rate);
} // get_ber


// ------------------------------------------------------------------------------------------------
// ---- get_digital_channel

short get_digital_channel ()
{
  return (Channel_No);
} // get_digital_channel


// ------------------------------------------------------------------------------------------------
// ---- get_snr

byte get_snr ()
{
  if (COFDM_Type == NXT_6000) return ((byte) (read_i2c (NXT_OFDM_CHC_SNR) / 8));
  else                        return ((byte) (read_i2c (ZAR_SNR)          / 8));
} // get_snr


// ------------------------------------------------------------------------------------------------
// ---- init_tuner

void init_tuner (bool  Complete)
{
  if (Complete)
  {
    reset_cofdm ();                                           // Reset the COFDM

    if (COFDM_Type == NXT_6000)
    {
      write_i2c (NXT_VIT_CORE_CTL,        0x80);              // Allow VIT Core Resync
      write_i2c (NXT_VIT_BERTIME_2,       0x00);              // BER Timer = 0x000200 * 256 = 131072 bits
      write_i2c (NXT_VIT_BERTIME_1,       0x02);              //
      write_i2c (NXT_VIT_BERTIME_0,       0x00);              //
      write_i2c (NXT_VIT_CORE_INTEN,      0x98);              // Enable BER interrupts
      write_i2c (NXT_VIT_CORE_CTL,        0x82);              // Enable BER measurement

      write_i2c (NXT_OFDM_CORE_CTL,       0x20);              // Enable OFDM Core
      write_i2c (NXT_OFDM_CORE_MODEGUARD, 0x00);              // Mode = 2K
      write_i2c (NXT_OFDM_SYR_CTL,        0x04);              // Enable long echo
      write_i2c (NXT_OFDM_SCR_CTL,        0x30);              // SYR Decay
      write_i2c (NXT_OFDM_PPM_CTL1,       0x30);              // 256 FFT bins
      write_i2c (NXT_OFDM_ITB_FRQ1,       0x06);              // Set IF frequency to 36.16667 MHz
      write_i2c (NXT_OFDM_ITB_FRQ2,       0x31);              //               "
      write_i2c (NXT_OFDM_AGC_CTL,        0x18);              // AGC Control
      write_i2c (NXT_ANALOG_CTRL_0,       0x20);              // Set interal ADC active
      write_i2c (NXT_DMD_RAQ,             0xb0);              // FEC Reaquisition = 50mS, OFDM Reaquisition = 200mS
      write_i2c (NXT_DIAG_CONFIG,         0x10);              // Set TST bus to normal mode (for LEDs), Parellel Mode
      write_i2c (NXT_DIAG_MODE,           0x01);              // Clock polarity inverted
      write_i2c (NXT_TS_FORMAT,           0x00);              // All signals active high, gated clock
    } // if
    else
    {
      write_i2c (ZAR_CLOCK_CTL,           0x38);              // 20.48 MHz Clock, Power up, clock on rising edge
      write_i2c (ZAR_CONFIG,              0x2d);              // Enable MPEG output, normal ordering
      write_i2c (ZAR_RESET,               0x80);              // Reset the MT352

      Sleep     (1);

      write_i2c (ZAR_AGC_CTL,             0xa0);              // Initialise the AGC
      write_i2c (ZAR_ADC_CTL_1,           0x40);              // Initialise the ADC
      write_i2c (ZAR_ACQ_CTL,             0x50);              // Auto search for unknown signal parameters
      write_i2c (ZAR_AGC_TARGET,          0x20);              // Limit the maximum AGC voltage
      write_i2c (ZAR_RS_ERR_PER_0,        0x01);              // BER Timer = 0x01 * 1024 * 1632 = 1671168 bits
      write_i2c (ZAR_RS_ERR_PER_1,        0x00);              //               "
      write_i2c (ZAR_SNR_SELECT_0,        0x00);              // SNR measurement is averaged over all carriers
      write_i2c (ZAR_SNR_SELECT_1,        0x20);              //               "
      write_i2c (ZAR_INPUT_FREQ_0,        0x05);              // Set IF frequency to 36.16667 MHz
      write_i2c (ZAR_INPUT_FREQ_1,        0x31);              //               "
      write_i2c (ZAR_SCAN_CTL,            0x0f);              // No check of tuner status, Report low signals
      write_i2c (ZAR_CAPT_RANGE,          0x32);              // Allow +- 250 Khz frequency capture range
    } // else
  } // if
} // init_tuner


// ------------------------------------------------------------------------------------------------
// ---- reset_cofdm

void reset_cofdm () 
{
  reg_data      Enable = read_reg (VIDEO, REG_GPIO_OUT_EN);
  reg_data      Data   = read_reg (VIDEO, REG_GPIO_DATA);
  reg_data      Data_T;
  reg_data      Data_F;

  Enable |= 0x08;                                           // Set up the masks so we don't
  Data_T  = Data | 0x00000008;                              //   overwrite what is already on 
  Data_F  = Data & 0xfffffff7;                              //   the GPIO ports

  write_reg (VIDEO, REG_GPIO_OUT_EN, Enable);               // Make sure the Reset line is an output
  write_reg (VIDEO, REG_GPIO_DATA,   Data_T);               // Set the Reset line high
  write_reg (VIDEO, REG_GPIO_DATA,   Data_F);               // Set the Reset line low
  Sleep     (100);                                          // Wait for 100mS

  write_reg (VIDEO, REG_GPIO_DATA,   Data_T);               // Set the Reset line high
} // reset_cofdm
