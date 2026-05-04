/**********************************************************************
 * DRV2CRC.H
 * Include File for DRV2CRC.C
 *
 * Copyright (C) 2002 NxtWave Communications, Inc.
 *
 * $Log:   V:/Applications EVAL Projects/archives/EVAL2005/Drv2005/drv2crc.h-arc  $
 * 
 *    Rev 1.1   Mar 08 2002 16:29:36   raggarwal
 * Alpha Release
 * 
 **********************************************************************/

#ifndef DRV2CRC_H
#define DRV2CRC_H

/*
******************************************************************************
Defines
******************************************************************************
*/

/*
******************************************************************************
Public Types
******************************************************************************
*/

/*
******************************************************************************
Public Functions
******************************************************************************
*/


/**********************************************************************
 *
 * NxtCrc
 *
 * CRC-CCIT algorithm derived from public sources
 *
 * Inputs:
 *	Data16 crc - current crc value, start with zero.
 *	Data8 c - next character to fold into the crc.
 *
 * Returns:
 *	Data16 crc - updated crc value
 *
 **********************************************************************/
Data16 NxtCrc(Data16 crc,
			  Data8 c);

#endif

