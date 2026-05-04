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

#define NXT_BER_TIME            0x200                       // NXT 6000 BER timer = 131072  (0x200 *  256) bits
#define ZAR_BER_TIME            0x660                       // MT 352   BER timer = 1671168 (0x660 * 1024) bits


// ------------------------------------------------------------------------------------------------
// ---- Types

typedef struct
{
  bool      AGC_Lock;
  bool      TPS_Lock;
  bool      VIT_Lock;
  bool      FEC_Lock;

  short     AGC;
  short     SNR;

  short     Mode;
  short     Modulation;

  float     BER; 
} tune_info;


// ------------------------------------------------------------------------------------------------
// ---- Prototypes

void        close_tuner         ();                         // Close all tuner objects
bool        channel_locked      (tune_info*, bool);         // Returns true if the current channel is locked
short       get_agc             ();                         // Read the AGC for the current multiplex
float       get_ber             ();                         // Read the Bit Error Rate (BER) for the current multiplex
byte        get_snr             ();                         // Read the SNR for the current multiplex
void        init_tuner          (bool);                     // Initialise the COFDM & Tuner
void        reset_cofdm         ();	                        // Reset the COFDM
