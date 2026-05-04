/**
* \file $Id: tuner5byte.c,v 1.6 2005/10/18 16:38:15 paulja Exp $
*
* \brief DRXBSP tuner implementation for 5byte tuners.
*
* \author Carlo Delhez
*/

/*
* $(c) 2004-2005 Micronas GmbH. All rights reserved.
*
* This software and related documentation (the 'Software') are intellectual
* property owned by Micronas and are copyright of Micronas, unless specifically
* noted otherwise.
*
* Any use of the Software is permitted only pursuant to the terms of the
* license agreement, if any, which accompanies, is included with or applicable
* to the Software ('License Agreement') or upon express written consent of
* Micronas. Any copying, reproduction or redistribution of the Software in
* whole or in part by any means not in accordance with the License Agreement
* or as agreed in writing by Micronas is expressly prohibited.
*
* THE SOFTWARE IS WARRANTED, IF AT ALL, ONLY ACCORDING TO THE TERMS OF THE
* LICENSE AGREEMENT. EXCEPT AS WARRANTED IN THE LICENSE AGREEMENT THE SOFTWARE
* IS DELIVERED 'AS IS' AND MICRONAS HEREBY DISCLAIMS ALL WARRANTIES AND
* CONDITIONS WITH REGARD TO THE SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES
* AND CONDITIONS OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, QUIT
* ENJOYMENT, TITLE AND NON-INFRINGEMENT OF ANY THIRD PARTY INTELLECTUAL
* PROPERTY OR OTHER RIGHTS WHICH MAY RESULT FROM THE USE OR THE INABILITY
* TO USE THE SOFTWARE.
*
* IN NO EVENT SHALL MICRONAS BE LIABLE FOR INDIRECT, INCIDENTAL, CONSEQUENTIAL,
* PUNITIVE, SPECIAL OR OTHER DAMAGES WHATSOEVER INCLUDING WITHOUT LIMITATION,
* DAMAGES FOR LOSS OF BUSINESS PROFITS, BUSINESS INTERRUPTION, LOSS OF BUSINESS
* INFORMATION, AND THE LIKE, ARISING OUT OF OR RELATING TO THE USE OF OR THE
* INABILITY TO USE THE SOFTWARE, EVEN IF MICRONAS HAS BEEN ADVISED OF THE
* POSSIBILITY OF SUCH DAMAGES, EXCEPT PERSONAL INJURY OR DEATH RESULTING FROM
* MICRONAS' NEGLIGENCE.                                                        $
*
*/
/*------------------------------------------------------------------------------
INCLUDE FILES
------------------------------------------------------------------------------*/

#include "stdafx.h"
#include "tuner5byte.h"

/*------------------------------------------------------------------------------
DEFINES
------------------------------------------------------------------------------*/

/* DEMOD_MAX_FREQ_OFFSET defines the margin on centerFrequency for deciding
   which TCU-entry to use; margin determined by frequency mismatch in
   transmitted or received signal that can be handled by demod
*/

#define DEMOD_MAX_FREQ_OFFSET 500 /* kHz */

/*------------------------------------------------------------------------------
STATIC VARIABLES
------------------------------------------------------------------------------*/

/*------------------------------------------------------------------------------
GLOBAL VARIABLES
------------------------------------------------------------------------------*/
TUNERFunc_t TUNER5BYTEFunctions_g = {
   TUNER_5BYTE_Open,
   TUNER_5BYTE_Close,
   TUNER_5BYTE_SetFrequency,
   TUNER_5BYTE_GetFrequency,
   TUNER_5BYTE_LockStatus,
   DRXBSP_TUNER_DefaultI2CWriteRead
};

/*------------------------------------------------------------------------------
STRUCTURES
------------------------------------------------------------------------------*/

/*------------------------------------------------------------------------------
FUNCTIONS
------------------------------------------------------------------------------*/

/**
* \fn DRXStatus_t TUNER_5BYTE_Open( pTUNERInstance_t tuner )
* \brief Open a tuner.
* \return DRXStatus_t Return status.
* \retval DRX_STS_OK Opened tuner with success.
* \retval DRX_STS_ERROR Something went wrong.
*/
DRXStatus_t
TUNER_5BYTE_Open( pTUNERInstance_t tuner, DtDevice *device )
{
   DRXStatus_t err;
   u8_t status;
   pTUNER5BYTEControlTable_t TCT;
   u16_t i, nTCT;

   pTUNER5BYTEData_t extAttr = (pTUNER5BYTEData_t) tuner->myExtAttr;

   nTCT = extAttr->controlTables;
   if ( ( nTCT == 0 ) || ( extAttr->controlTable == NULL ) )
   {
      /* no TCT present */
      return DRX_STS_ERROR;
   }

   for ( i=0; i < nTCT; i++ )
   {
      TCT = &(extAttr->controlTable[i]);

      if ( ( TCT->refFreqNumerator   == ( DRXFrequency_t )0 ) ||
           ( TCT->refFreqDenominator == 0 ) )
      {
         /* zero component in reference frequency, cannot do calculations */
         return DRX_STS_ERROR;
      }
      if ( TCT->ControlUnits == NULL )
      {
         /* no TCU present */
         return DRX_STS_ERROR;
      }
   }

   /* check presence of tuner by reading status */

   err = tuner->myFunct->i2cWriteReadFunc( tuner, NULL, 0, 0, &(tuner->myI2CDevAddr), 1, &status, device );
   if ( err != DRX_STS_OK )
   {
      return( err );
   }

   return DRX_STS_OK;
}

/*============================================================================*/

/**
* \fn DRXStatus_t DRXBSP_TUNER_Close( pTUNERInstance_t tuner )
* \brief Close a tuner.
* \return DRXStatus_t Return status.
* \retval DRX_STS_OK Closed tuner with success.
* \retval DRX_STS_ERROR Something went wrong.
*/
DRXStatus_t
TUNER_5BYTE_Close( pTUNERInstance_t tuner )
{
   return( DRX_STS_OK );
}

/*============================================================================*/

/**
* \fn DRXStatus_t DRXBSP_TUNER_SetFrequency( pTUNERInstance_t tuner,
                                             TUNERMode_t mode,
                                             DRXFrequency_t centerFrequency )
* \brief Program tuner at given center frequency for given mode.
* \return DRXStatus_t Return status.
* \retval DRX_STS_OK Programmed tuner successfully.
* \retval DRX_STS_ERROR Something went wrong.
*/
DRXStatus_t
TUNER_5BYTE_SetFrequency( pTUNERInstance_t tuner,
                           TUNERMode_t      mode,
                           DRXFrequency_t   centerFrequency, DtDevice *device )
{
   u16_t i;
   u16_t telegramsSent = 0;
   pTUNER5BYTEData_t extAttr = (pTUNER5BYTEData_t) tuner->myExtAttr;

   if ( ( centerFrequency < ( tuner->myCommonAttr->minFreqRF - DEMOD_MAX_FREQ_OFFSET ) ) ||
        ( centerFrequency > ( tuner->myCommonAttr->maxFreqRF + DEMOD_MAX_FREQ_OFFSET ) ) )
   {
      /* frequency out of range for this tuner */
      return DRX_STS_ERROR;
   }

   /* Clear sub-mode flags */
   mode &= ~TUNER_MODE_SUBALL;

   /* Check if sub-mode flag needs to be added */
   if ( tuner->myCommonAttr->subModes > 1 )
   {
      /* More than one submode; verify validity of sub-mode */
      if ( ( tuner->myCommonAttr->subMode  >= TUNER_MODE_SUB_MAX ) ||
           ( tuner->myCommonAttr->subMode  >= tuner->myCommonAttr->subModes )  )
      {
         /* invalid submode */
         return DRX_STS_ERROR;
      }
      /* set sub-mode flag */
      mode |= ( TUNER_MODE_SUB0 << tuner->myCommonAttr->subMode );
   }

   /* Check all control tables for desired mode: multiple packets may be sent */
   for ( i=0; i < extAttr->controlTables ; i++ )
   {
      pTUNER5BYTEControlTable_t TCT = &extAttr->controlTable[i];

      if ( ( TCT->modes & mode ) == mode )
      {
         /* this controlTable supports -at least- the requested mode bits     */

         DRXFrequency_t oscillatorFrequency;
         u32_t divider;
         u8_t  tunerTelegram[4];
         pTUNER5BYTEControlUnit_t TCU;
         DRXStatus_t err;

         oscillatorFrequency = centerFrequency + TCT->outputFreq;
         divider = ( oscillatorFrequency * TCT->refFreqDenominator +
                     ( TCT->refFreqNumerator >> 1 ) ) / TCT->refFreqNumerator;

         if ( divider >= (1<<15) )
         {
            /* divider out-of-range, only 15-bit value supported */
            return DRX_STS_ERROR;
         }

         tunerTelegram[0] = (u8_t)( ( divider >> 8 ) & 0x7F );
         tunerTelegram[1] = (u8_t)(   divider        & 0xFF );

         TCU = TCT->ControlUnits;
         while ( ( centerFrequency - DEMOD_MAX_FREQ_OFFSET ) > TCU->maxFreq )
         {
            TCU++;
         }
         tunerTelegram[2] = TCU->CB1;
         tunerTelegram[3] = TCU->CB2;

         //tuner->myI2CDevAddr = I2CDeviceAddr_t0xE0;
         err = tuner->myFunct->i2cWriteReadFunc( tuner, &tuner->myI2CDevAddr, 4, tunerTelegram, NULL, 0, NULL, device );
         
         if ( err != DRX_STS_OK )
         {
            return( err );
         }

         telegramsSent++;

         /* preserve information for GetFrequency call */
         tuner->myCommonAttr->RFfrequency = -TCT->outputFreq +
          ( divider * TCT->refFreqNumerator + ( TCT->refFreqDenominator >> 1 ) )
                  / TCT->refFreqDenominator;

         tuner->myCommonAttr->IFfrequency = TCT->outputFreq;
      }
   }

   if ( telegramsSent == 0 ) {
      /* no match found in control tables for the mode requested */
      return DRX_STS_ERROR;
   }
   return( DRX_STS_OK );
}

/*============================================================================*/

/**
* \fn DRXStatus_t DRXBSP_TUNER_GetFrequency( pTUNERInstance_t tuner,
                                             pDRXFrequency_t  RFfrequency,
                                             pDRXFrequency_t  IFfrequency )
* \brief Get tuned center frequency.
* \return DRXStatus_t Return status.
* \retval DRX_STS_OK Returned frequency successfully.
* \retval DRX_STS_ERROR Something went wrong.
*/
DRXStatus_t
TUNER_5BYTE_GetFrequency( pTUNERInstance_t tuner,
                           pDRXFrequency_t  RFfrequency,
                           pDRXFrequency_t  IFfrequency )
{
   if ( RFfrequency != NULL )
   {
      *RFfrequency = tuner->myCommonAttr->RFfrequency;
   }
   if ( IFfrequency != NULL )
   {
      *IFfrequency = tuner->myCommonAttr->IFfrequency;
   }
   return( DRX_STS_OK );
}

/*============================================================================*/

/**
* \fn DRXStatus_t DRXBSP_TUNER_LockStatus( pTUNERInstance_t tuner,
      pTUNERLockStatus_t lockStat )
* \brief Get tuner locking status.
* \return DRXStatus_t Return status.
* \retval DRX_STS_OK Acquired locking status successfully.
* \retval DRX_STS_ERROR Something went wrong.
*/
DRXStatus_t
TUNER_5BYTE_LockStatus( pTUNERInstance_t    tuner,
                         pTUNERLockStatus_t  lockStat, DtDevice *device )
{
   DRXStatus_t err;
   u8_t status;
   pTUNER5BYTEData_t extAttr = (pTUNER5BYTEData_t) tuner->myExtAttr;

   /* read tuner status */
   err = tuner->myFunct->i2cWriteReadFunc( tuner, NULL, 0, 0, &tuner->myI2CDevAddr, 1, &status, device );
   if ( err != DRX_STS_OK )
   {
      return( err );
   }
   /* report lock */
   if ( lockStat != NULL )
   {
      *lockStat = TUNER_NOT_LOCKED;
      if ( ( status & extAttr->lockMask ) == extAttr->lockValue )
      {
         *lockStat = TUNER_LOCKED;
      }
   }
   return DRX_STS_OK;
}

/* End of file */
