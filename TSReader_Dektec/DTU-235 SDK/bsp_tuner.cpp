/**
* \file $Id: bsp_tuner.c,v 1.9 2005/10/18 16:38:15 paulja Exp $
*
* \brief DRXBSP tuner dependant functions implementation.
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
#include "bsp_tuner.h"

/*------------------------------------------------------------------------------
DEFINES
------------------------------------------------------------------------------*/

/*------------------------------------------------------------------------------
STATIC VARIABLES
------------------------------------------------------------------------------*/

/*------------------------------------------------------------------------------
GLOBAL VARIABLES
------------------------------------------------------------------------------*/

/*------------------------------------------------------------------------------
STRUCTURES
------------------------------------------------------------------------------*/

/*------------------------------------------------------------------------------
FUNCTIONS
------------------------------------------------------------------------------*/

/**
* \fn DRXStatus_t DRXBSP_TUNER_Open( pTUNERInstance_t tuner )
* \brief Open a tuner.
* \return DRXStatus_t Return status.
* \retval DRX_STS_OK Opened tuner with success.
* \retval DRX_STS_ERROR Something went wrong.
* \retval DRX_STS_INVALID_ARG Invalid tuner instance.
*/
DRXStatus_t
DRXBSP_TUNER_Open( pTUNERInstance_t tuner, DtDevice *device )
{
   DRXStatus_t err;

   /*
      do restricted sanity checks of the tuner instance data structures,
      so other functions can rely on the parameters in the structures
    */


   if ( ( tuner == NULL )             ||
        ( tuner->myCommonAttr == NULL ) ||
        ( tuner->myFunct == NULL ) )

   {
      /* incomplete or no tuner structure */
      return DRX_STS_INVALID_ARG;
   }

   if ( tuner->myCommonAttr->selfCheck == &(tuner->myCommonAttr->selfCheck) )
   {
      /* tuner already opened */
      return DRX_STS_ERROR;
   }

   err = (*(tuner->myFunct->openFunc))( tuner, device );

   if ( err == DRX_STS_OK )
   {
      tuner->myCommonAttr->programmed = FALSE;
      tuner->myCommonAttr->selfCheck  = &tuner->myCommonAttr->selfCheck;
   } else {
      tuner->myCommonAttr->selfCheck  = NULL;
   }

   return ( err );
}

/*============================================================================*/

/**
* \fn DRXStatus_t DRXBSP_TUNER_Close( pTUNERInstance_t tuner )
* \brief Close a tuner.
* \return DRXStatus_t Return status.
* \retval DRX_STS_OK Closed tuner with success.
* \retval DRX_STS_ERROR Something went wrong.
* \retval DRX_STS_INVALID_ARG Invalid tuner instance.
*/
DRXStatus_t
DRXBSP_TUNER_Close( pTUNERInstance_t tuner )
{
   DRXStatus_t err;

   if ( ( tuner == NULL )             ||
        ( tuner->myCommonAttr == NULL ) ||
        ( tuner->myFunct == NULL ) )

   {
      /* incomplete or no tuner structure */
      return DRX_STS_INVALID_ARG;
   }

   if ( tuner->myCommonAttr->selfCheck != &(tuner->myCommonAttr->selfCheck) )
   {
      /* tuner not opened */
      return DRX_STS_ERROR;
   }

   err = (*(tuner->myFunct->closeFunc))( tuner );

   if ( err == DRX_STS_OK )
   {
      tuner->myCommonAttr->selfCheck = NULL;
   }

   return( err );
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
* \retval DRX_STS_INVALID_ARG Invalid tuner instance.
*/
DRXStatus_t
DRXBSP_TUNER_SetFrequency( pTUNERInstance_t tuner,
                           TUNERMode_t      mode,
                           DRXFrequency_t   centerFrequency, DtDevice *device )
{
   DRXStatus_t err;

   if ( ( tuner == NULL )             ||
        ( tuner->myCommonAttr == NULL ) ||
        ( tuner->myFunct == NULL ) )

   {
      /* incomplete or no tuner structure */
      return DRX_STS_INVALID_ARG;
   }

   if ( tuner->myCommonAttr->selfCheck != &(tuner->myCommonAttr->selfCheck) )
   {
      /* tuner not opened */
      return DRX_STS_ERROR;
   }

   err = (*(tuner->myFunct->setFrequencyFunc))( tuner, mode, centerFrequency, device );

   if ( err == DRX_STS_OK )
   {
      tuner->myCommonAttr->programmed = TRUE;
   } else {
      tuner->myCommonAttr->programmed = FALSE;
   }

   return (err);
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
* \retval DRX_STS_INVALID_ARG Invalid tuner instance.
*/
DRXStatus_t
DRXBSP_TUNER_GetFrequency( pTUNERInstance_t tuner,
                           pDRXFrequency_t  RFfrequency,
                           pDRXFrequency_t  IFfrequency )
{
   if ( ( tuner == NULL )             ||
        ( tuner->myCommonAttr == NULL ) ||
        ( tuner->myFunct == NULL ) )

   {
      /* incomplete or no tuner structure */
      return DRX_STS_INVALID_ARG;
   }

   if ( ( tuner->myCommonAttr->selfCheck != &(tuner->myCommonAttr->selfCheck) ) ||
         !tuner->myCommonAttr->programmed )
   {
      /* tuner not opened, or no frequency programmed yet */
      return DRX_STS_ERROR;
   }

   return( (*(tuner->myFunct->getFrequencyFunc))( tuner, RFfrequency, IFfrequency) );
}

/*============================================================================*/

/**
* \fn DRXStatus_t DRXBSP_TUNER_LockStatus( pTUNERInstance_t tuner,
      pTUNERLockStatus_t lockStat )
* \brief Get tuner locking status.
* \return DRXStatus_t Return status.
* \retval DRX_STS_OK Acquired locking status successfully.
* \retval DRX_STS_ERROR Something went wrong.
* \retval DRX_STS_INVALID_ARG Invalid tuner instance.
*/
DRXStatus_t
DRXBSP_TUNER_LockStatus( pTUNERInstance_t    tuner,
                         pTUNERLockStatus_t  lockStat, DtDevice *device )
{
   if ( ( tuner == NULL )             ||
        ( tuner->myCommonAttr == NULL ) ||
        ( tuner->myFunct == NULL ) )

   {
      /* incomplete or no tuner structure */
      return DRX_STS_INVALID_ARG;
   }

   if ( tuner->myCommonAttr->selfCheck != &(tuner->myCommonAttr->selfCheck) )
   {
      /* tuner not opened */
      return DRX_STS_ERROR;
   }

   return ( (*(tuner->myFunct->lockStatusFunc))( tuner, lockStat, device ) );
}

/*============================================================================*/

/**
* \fn DRXStatus_t DRXBSP_TUNER_DefaultI2CWriteRead(
               pTUNERInstance_t tuner,
               pI2CDeviceAddr_t wDevAddr,
               u16_t            wCount,
               pu8_t            wData,
               pI2CDeviceAddr_t rDevAddr,
               u16_t            rCount,
               pu8_t            rData);
* \brief Standard I2C read write function.
* \return DRXStatus_t Return status.
* \retval DRX_STS_OK Acquired locking status successfully.
* \retval DRX_STS_ERROR Something went wrong.
* \retval DRX_STS_INVALID_ARG Invalid tuner instance.
*
* Default implementation of I2CWriteRead function for tuners, via bsp_i2c.
*
*/
DRXStatus_t
DRXBSP_TUNER_DefaultI2CWriteRead( pTUNERInstance_t   tuner,
                                  pI2CDeviceAddr_t wDevAddr,
                                  u16_t            wCount,
                                  pu8_t            wData,
                                  pI2CDeviceAddr_t rDevAddr,
                                  u16_t            rCount,
                                  pu8_t            rData,
                                  DtDevice         *device)
{
   /* Default to BSP_I2C routine */
   return ( DRXBSP_I2C_WriteRead( wDevAddr, wCount, wData,
                                  rDevAddr, rCount, rData, device) );
}

/* End of file */
