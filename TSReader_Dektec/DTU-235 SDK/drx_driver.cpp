/**
* \file $Id: drx_driver.c,v 1.15 2005/09/27 15:36:52 paulja Exp $
*
* \brief Generic DRX functionality, DRX driver core.
*
* \author Paul Janssen
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

/*-----------------------------------------------------------------------------
INCLUDE FILES
----------------------------------------------------------------------------*/

#include "stdafx.h"
#include <string.h>

#include "drx_driver.h"
#include "drx_driver_version.h"
#include "bsp_host.h"


/*-----------------------------------------------------------------------------
DEFINES
----------------------------------------------------------------------------*/

/*============================================================================*/
/*=== MICROCODE RELATED DEFINES ==============================================*/
/*============================================================================*/

/**
* \def UCODE_MAGIC_WORD
* \brief Magic word for checking correct Endianess of microcode data.
*
*/
#define UCODE_MAGIC_WORD         ((((char)'H')<<8)+((char)'L'))
/**
* \def UCODE_CRC_FLAG
* \brief CRC flag in ucode header, flags field.
*
*/
#define UCODE_CRC_FLAG           (0x0001)
/**
* \def UCODE_COMPRESSION_FLAG
* \brief Compression flag in ucode header, flags field.
*
*/
#define UCODE_COMPRESSION_FLAG   (0x0002)

/*============================================================================*/
/*=== CHANNEL SCAN RELATED DEFINES ===========================================*/
/*============================================================================*/

/**
* \def DRX_SCAN_MAX_PROGRESS
* \brief Maximum progress indication.
*
* Progress indication will run from 0 upto DRX_SCAN_MAX_PROGRESS during scan.
*
*/
#define DRX_SCAN_MAX_PROGRESS 1000

/**
* \def DRX_SCAN_8MHZ_STEP
* \brief Distance in kHz between two centre frequencies in 8Mhz grid
*/
#define DRX_SCAN_8MHZ_STEP 8000

/**
* \def DRX_SCAN_7MHZ_STEP
* \brief Distance in kHz between two centre frequencies in 7Mhz grid
*/
#define DRX_SCAN_7MHZ_STEP 7000

/**
* \def DRX_SCAN_6MHZ_STEP
* \brief Distance in kHz between two centre frequencies in 6Mhz grid
*/
#define DRX_SCAN_6MHZ_STEP 6000

/*-----------------------------------------------------------------------------
GLOBAL VARIABLES
----------------------------------------------------------------------------*/

/*============================================================================*/
/*============================================================================*/
/*== Instance administration =================================================*/
/*============================================================================*/
/*============================================================================*/

/* NULL terminated array of pointer to demod instance controled by driver. */
//static pDRXDemodInstance_t *drxDriverDemodsInstancesList = NULL;


/*-----------------------------------------------------------------------------
STRUCTURES
----------------------------------------------------------------------------*/

typedef struct {
   u32_t addr;
   u16_t size;
   u16_t flags; /* bit[15..2]=reserved,
                   bit[1]= compression on/off
                   bit[0]= CRC on/off */
   u16_t CRC;
} DRXUCodeBlockHdr_t, *pDRXUCodeBlockHdr_t;

/*-----------------------------------------------------------------------------
FUNCTIONS
----------------------------------------------------------------------------*/

/*============================================================================*/
/*============================================================================*/
/*== Channel Scan Functions ==================================================*/
/*============================================================================*/
/*============================================================================*/

/**
* \fn DRXStatus_t ScanWaitForLock()
* \brief Wait for lock while scanning.
* \param demod Pointer to demodulator instance.
* \param lockStat Pointer to bool indicating if end result is lock or not.
* \return DRXStatus_t.
*
* Wait until timeout, DEMOD_LOCK or NEVER_LOCK.
*
* Assume lock function returns : at least DRX_NOT_LOCKED and DRX_DEMOD_LOCK.
* In case DRX_NEVER_LOCK is returned the poll-wait will be aborted.
*
* Assume BSP has a clock function to retreive a millisecond ticker value.
*
*/
static DRXStatus_t
ScanWaitForLock( pDRXDemodInstance_t demod,
                 pBool_t             isLocked,
                 DtDevice *device)
{
   Bool_t           doneWaiting = FALSE;
   DRXLockStatus_t  lockState = DRX_NOT_LOCKED;
   DRXLockStatus_t  desiredLockState = DRX_NOT_LOCKED;
   u32_t            timeoutValue = 0;
   u32_t            startTimeLockStage = 0;
   u32_t            currentTime = 0;
   u32_t            timerValue = 0;

   /* Check arguments */
   if ( isLocked == NULL )
   {
      return ( DRX_STS_INVALID_ARG);
   }

   *isLocked = FALSE;
   timeoutValue = (u32_t) demod->myCommonAttr->scanDemodLockTimeout;
   desiredLockState = demod->myCommonAttr->scanDesiredLock;

   startTimeLockStage=DRXBSP_HST_Clock();

   /* Start polling loop, checking for lock & timeout */
   while ( doneWaiting == FALSE ) {

      if ( DRX_Ctrl( demod, DRX_CTRL_LOCK_STATUS, &lockState, device ) != DRX_STS_OK )
      {
         return (DRX_STS_ERROR);
      }
      currentTime=DRXBSP_HST_Clock();

      /* Check for time out in this lock stage */
      timerValue = currentTime - startTimeLockStage;
      if ( timerValue > timeoutValue )
      {
         /* Maximum timeout for this lock stage reached, return no_lock */
         doneWaiting = TRUE;
      }
      else
      {
         if ( lockState >= desiredLockState )
         {
                  *isLocked = TRUE;
                  doneWaiting = TRUE;
         }
         else if ( lockState == DRX_NEVER_LOCK )
         {
                  doneWaiting = TRUE;
         } /* if ( lockState != oldLockState ) .. */
      } /* if ( timerValue > timeoutValue ) */
   } /* while */

   return (DRX_STS_OK);
}

/*============================================================================*/

/**
* \fn DRXStatus_t ScanPrepareNextScan()
* \brief Determine next frequency to scan.
* \param demod Pointer to demodulator instance.
* \return DRXStatus_t.
* \retval DRX_STS_OK Succes.
* \retval DRX_STS_ERROR Something went wrong.
* \retval DRX_STS_INVALID_ARG Invalid frequency plan.
*
* Helper function for CtrlScanNext() function.
* Compute next frequency & index in frequency plan.
* Check if scan is ready.
*
*/
static DRXStatus_t
ScanPrepareNextScan ( pDRXDemodInstance_t  demod )
{
   pDRXCommonAttr_t     commonAttr=NULL;
   u16_t                tableIndex = 0;
   u16_t                frequencyPlanSize = 0;
   pDRXFrequencyPlan_t  frequencyPlan = NULL;
   DRXFrequency_t       nextFrequency = 0;
   DRXFrequency_t       frequencyStep = 0;
   DRXFrequency_t       tunerMinFrequency = 0;
   DRXFrequency_t       tunerMaxFrequency = 0;

   /* Check parameters */
   if ( demod ->myCommonAttr->scanParam == NULL)
   {
      return (DRX_STS_ERROR);
   }

   commonAttr        = (pDRXCommonAttr_t)demod->myCommonAttr;
   tableIndex        = commonAttr->scanFreqPlanIndex;
   frequencyPlan     = commonAttr->scanParam->frequencyPlan;
   nextFrequency     = commonAttr->scanNextFrequency;
   tunerMinFrequency = commonAttr->tunerMinFreqRF;
   tunerMaxFrequency = commonAttr->tunerMaxFreqRF;

   do {
      /* Determine frequency step of channel to scan */
      switch ( frequencyPlan[tableIndex].bandwidth ) {
         case DRX_BANDWIDTH_8MHZ:
            frequencyStep = DRX_SCAN_8MHZ_STEP;
            break;
         case DRX_BANDWIDTH_7MHZ:
            frequencyStep = DRX_SCAN_7MHZ_STEP;
            break;
         case DRX_BANDWIDTH_6MHZ:
            frequencyStep = DRX_SCAN_6MHZ_STEP;
            break;
         default:
            /* Frequency plan content is invalid */
            return (DRX_STS_INVALID_ARG);
            break;
      };

      /* Search next frequency to scan */
      nextFrequency += frequencyStep;
      if ( nextFrequency > frequencyPlan[tableIndex].last )
      {
         /* reached end of this band */
         tableIndex++;
         frequencyPlanSize = commonAttr->scanParam->frequencyPlanSize;
         if ( tableIndex >= frequencyPlanSize )
         {
            /* reached end of frequency plan */
            commonAttr->scanReady = TRUE;
         } else {
            nextFrequency = frequencyPlan[tableIndex].first;
         }
      }
      if ( nextFrequency > (tunerMaxFrequency) )
      {
         /* reached end of tuner range */
         commonAttr->scanReady = TRUE;
      }
   } while( ( nextFrequency < tunerMinFrequency ) &&
            ( commonAttr->scanReady == FALSE ) );

   /* Store new values */
   commonAttr->scanFreqPlanIndex = tableIndex;
   commonAttr->scanNextFrequency = nextFrequency;

   return (DRX_STS_OK);
}

/*============================================================================*/

/**
* \fn DRXStatus_t CtrlScanInit()
* \brief Initialize for channel scan.
* \param demod Pointer to demodulator instance.
* \param scanParam Pointer to scan parameters.
* \return DRXStatus_t.
* \retval DRX_STS_OK Initialized for scan.
* \retval DRX_STS_ERROR No overlap between frequency plan and tuner range.
* \retval DRX_STS_INVALID_ARG Wrong parameters.
*
* After calling this routine scanning will start at the first centre frequency
* of the frequency plan that is within the tuner range.
*
*/
static DRXStatus_t
CtrlScanInit( pDRXDemodInstance_t  demod,
               pDRXScanParam_t scanParam )
{
   pDRXCommonAttr_t commonAttr=NULL;
   DRXFrequency_t maxTunerFreq = 0;
   DRXFrequency_t minTunerFreq = 0;
   u16_t nrChannelsInPlan = 0;
   u16_t i = 0;

   commonAttr = (pDRXCommonAttr_t)demod -> myCommonAttr;
   commonAttr->scanActive = TRUE;

   /* invalidate a previous SCAN_INIT */
   commonAttr->scanParam = NULL;
   commonAttr->scanNextFrequency = 0;

   /* Check parameters */
   if ( ( demod->myTuner == NULL) ||

        ( scanParam == NULL) ||

        ( scanParam->numTries == 0) ||

        ( scanParam->frequencyPlan == NULL) ||

        ( scanParam->frequencyPlanSize == 0 )
       )
   {
      commonAttr->scanActive = FALSE;
      return (DRX_STS_INVALID_ARG);
   } /* if */

   /* Check frequency plan contents */
   maxTunerFreq = commonAttr->tunerMaxFreqRF;
   minTunerFreq = commonAttr->tunerMinFreqRF;
   for( i = 0; i < (scanParam->frequencyPlanSize); i++ )
   {
      DRXFrequency_t width = 0;
      DRXFrequency_t step = 0;
      DRXFrequency_t firstFreq = scanParam->frequencyPlan[i].first;
      DRXFrequency_t lastFreq  = scanParam->frequencyPlan[i].last;
      DRXFrequency_t minFreq = 0;
      DRXFrequency_t maxFreq = 0;

      if ( firstFreq > lastFreq )
      {
            /* First centre frequency is higher then last centre frequency */
            commonAttr->scanActive = FALSE;
            return (DRX_STS_INVALID_ARG);
      } /* if */

      width = lastFreq - firstFreq;
      switch ( scanParam->frequencyPlan[i].bandwidth ) {
         case DRX_BANDWIDTH_6MHZ:
            step = DRX_SCAN_6MHZ_STEP;
            break;
         case DRX_BANDWIDTH_7MHZ:
            step = DRX_SCAN_7MHZ_STEP;
            break;
         case DRX_BANDWIDTH_8MHZ:
            step = DRX_SCAN_8MHZ_STEP;
            break;
         default:
            /* Invalid bandwidth parameter in frequency plan */
            commonAttr->scanActive = FALSE;
            return (DRX_STS_INVALID_ARG);
            break;
      }; /* switch */

      if ( (width % step) != 0 )
      {
            /* Difference between last and first centre frequency is not
               an integer number of bandwidths */
            commonAttr->scanActive = FALSE;
            return (DRX_STS_INVALID_ARG);
      } /* if */

      /* Check if frequency plan entry intersects with tuner range */
      if ( lastFreq >= minTunerFreq )
      {
         if ( firstFreq <= maxTunerFreq )
         {
            if (  firstFreq >= minTunerFreq )
            {
               minFreq = firstFreq;
            } else {
               u32_t n=0;

               n=( minTunerFreq - firstFreq )/step;
               if ( (( minTunerFreq - firstFreq )%step) != 0 )
               {
                  n++;
               }
               minFreq = firstFreq + n*step;
            }
            if ( lastFreq <= maxTunerFreq )
            {
               maxFreq = lastFreq;
            } else {
               u32_t n=0;

               n=( lastFreq - maxTunerFreq )/step;
               if ( (( lastFreq - maxTunerFreq )%step) !=0 )
               {
                  n++;
               }
               maxFreq = lastFreq - n*step;
            }
         }
      }

      /* Keep track of total number of channels within tuner range
         in this frequency plan. */
      if ( (minFreq!=0) && (maxFreq!=0) )
      {
         nrChannelsInPlan += (u16_t)(( (maxFreq-minFreq) / step ) +1 );

         /* Determine first frequency (within tuner range) to scan */
         if ( commonAttr->scanNextFrequency == 0 )
         {
            commonAttr->scanNextFrequency = minFreq;
            commonAttr->scanFreqPlanIndex = i;
         }
      }

   }/* for ( ... ) */

   if ( nrChannelsInPlan == 0 )
   {
      /* Tuner range and frequency plan ranges do not overlap */
      commonAttr->scanActive = FALSE;
      return (DRX_STS_ERROR);
   }

   /* Store parameters */
   commonAttr->scanReady = FALSE;
   commonAttr->scanMaxChannels = nrChannelsInPlan;
   commonAttr->scanChannelsScanned = 0;
   commonAttr->scanParam = scanParam; /* SCAN_NEXT is now allowed */

   commonAttr->scanActive = FALSE;

   return (DRX_STS_OK);
}

/*============================================================================*/

/**
* \fn DRXStatus_t CtrlScanNext()
* \brief Scan for next channel .
* \param demod Pointer to demodulator instance.
* \param scanParam Pointer to scan parameters.
* \return DRXStatus_t.
* \retval DRX_STS_OK Channel found.
* \retval DRX_STS_BUSY Tried specified number of channels, need to scan more.
* \retval DRX_STS_READY Reached end of scan range.
* \retval DRX_STS_ERROR Something went wrong.
* \retval DRX_STS_INVALID_ARG Wrong parameters.
*
*/
static DRXStatus_t
CtrlScanNext( pDRXDemodInstance_t  demod,
              pu16_t               scanProgress,
              DtDevice *device)
{
   pDRXCommonAttr_t  commonAttr=NULL;
   pBool_t scanReady = NULL;
   u16_t maxProgress = DRX_SCAN_MAX_PROGRESS;
   u32_t numTries = 0;
   u32_t i = 0;

   /* Check scan parameters */
   *scanProgress = 0;
   commonAttr = (pDRXCommonAttr_t)demod -> myCommonAttr;
   commonAttr->scanActive = TRUE;
   if ( (commonAttr->scanParam == NULL) ||
        ( commonAttr->scanMaxChannels == 0 )
      )
   {
      /* CtrlScanInit() was not called succesfully before CtrlScanNext() */
      commonAttr->scanActive = FALSE;
      return (DRX_STS_ERROR);
   }

   *scanProgress = (u16_t)(((commonAttr->scanChannelsScanned)*
                           ((u32_t)(maxProgress)))/
                           (commonAttr->scanMaxChannels));

   /* Scan */
   numTries = commonAttr->scanParam->numTries;
   scanReady = &(commonAttr->scanReady);
   for ( i = 0; ( (i < numTries) && ( (*scanReady) == FALSE) ); i++)
   {
      DRXChannel_t         scanChannel;
      DRXStatus_t          status      = DRX_STS_ERROR;
      Bool_t               isLocked    = FALSE;
      pDRXFrequencyPlan_t  freqPlan    = NULL;


      /* Next channel to scan */
      freqPlan =
        &(commonAttr->scanParam->frequencyPlan[commonAttr->scanFreqPlanIndex]);
      scanChannel.frequency      = commonAttr->scanNextFrequency;
      scanChannel.bandwidth      = freqPlan->bandwidth;
      scanChannel.mirror         = (DRXMirror_t) DRX_AUTO;
      scanChannel.constellation  = (DRXConstellation_t) DRX_AUTO;
      scanChannel.hierarchy      = (DRXHierarchy_t) DRX_AUTO;
      scanChannel.priority       = (DRXPriority_t) DRX_PRIORITY_HIGH;
      scanChannel.coderate       = (DRXCoderate_t) DRX_AUTO;
      scanChannel.guard          = (DRXGuard_t) DRX_AUTO;
      scanChannel.fftmode        = (DRXFftmode_t) DRX_AUTO;
      scanChannel.classification = (DRXClassification_t) DRX_AUTO;


      /* Scan (try) that frequency */
      status = DRX_Ctrl( demod, DRX_CTRL_SET_CHANNEL, &scanChannel, device );
      if ( status != DRX_STS_OK )
      {
         commonAttr->scanActive = FALSE;
         return (status);
      }

      /* Determine frequency to scan next time
         side effects:
            commonAttr->scanFreqPlanIndex
            commonAttr->scanNextFrequency
            commonAttr->scanReady
      */
      status = ScanPrepareNextScan( demod );
      if ( status != DRX_STS_OK )
      {
         commonAttr->scanActive = FALSE;
         return (status);
      }

      /* Keep track of progress */
      (commonAttr->scanChannelsScanned) ++;
      *scanProgress = (u16_t)(((commonAttr->scanChannelsScanned)*
                              ((u32_t)(maxProgress)))/
                             (commonAttr->scanMaxChannels));


      status = ScanWaitForLock( demod, &isLocked, device );
      if ( status != DRX_STS_OK )
      {
         commonAttr->scanActive = FALSE;
         return (status);
      }

      if ( isLocked == TRUE )
      {
         /* Channel found */
         commonAttr->scanActive = FALSE;
         return (DRX_STS_OK);
      }
   } /* for ( i = 0; i < ( ... numTries); i++) */

   if ( (*scanReady) == TRUE )
   {
      commonAttr->scanActive = FALSE;
      return (DRX_STS_READY);
   }

   commonAttr->scanActive = FALSE;

   return (DRX_STS_BUSY);
}

/*============================================================================*/
/*============================================================================*/
/*===Microcode related functions==============================================*/
/*============================================================================*/
/*============================================================================*/

/**
* \fn u16_t UCodeRead16( pu8_t addr)
* \brief Read a 16 bits word, expect big endian data.
* \return u16_t The data read.
*/
static u16_t
UCodeRead16( pu8_t addr)
{
   /* Works fo any host processor */

   u16_t word=0;

   word = addr[0];
   word <<= 8;
   word |= addr[1];

   return ( word );
}

/*============================================================================*/

/**
* \fn u32_t UCodeRead32( pu8_t addr)
* \brief Read a 32 bits word, expect big endian data.
* \return u32_t The data read.
*/
static u32_t
UCodeRead32( pu8_t addr)
{
   /* Works fo any host processor */

   u32_t word=0;

   word = addr[0];
   word <<= 8;
   word |= addr[1];
   word <<= 8;
   word |= addr[2];
   word <<= 8;
   word |= addr[3];

   return ( word );
}

/*============================================================================*/

/**
* \fn u16_t UCodeComputeCRC (pu8_t blockData, u16_t nrWords)
* \brief Compute CRC of block of microcode data.
* \param blockData Pointer to microcode data.
* \param nrWords Size of microcode block (number of 16 bits words).
* \return u16_t The computed CRC residu.
*/
static u16_t
UCodeComputeCRC (pu8_t blockData, u16_t nrWords)
{
   u16_t i = 0;
   u8_t  j = 0;
   u32_t CRCWord=0;
   u32_t carry=0;

   while (i < nrWords) {
     CRCWord |= (u32_t) UCodeRead16(blockData);
     for (j = 0; j < 16; j++)
     {
       CRCWord <<= 1;
       if (carry != 0)
          CRCWord ^= 0x80050000UL;
       carry = CRCWord & 0x80000000UL;
     }
     i++;
     blockData+=(sizeof(u16_t));
  }
  return ((u16_t) (CRCWord >> 16));
}

/*============================================================================*/

/**
* \fn DRXStatus_t CtrlUCode()
* \brief Handle microcode upload or verify.
* \param devAddr Address of device.
* \param mcInfo  Pointer to information about microcode data.
* \param action  Either UCODE_UPLOAD or UCODE_VERIFY
* \return DRXStatus_t.
*/
static DRXStatus_t
CtrlUCode( pDRXDemodInstance_t demod,
           pDRXUCodeInfo_t  mcInfo,
           DRXUCodeAction_t action,
           DtDevice *device)
{
   u16_t  i = 0;
   u16_t  mcNrOfBlks = 0;
   u16_t  mcMagicWord = 0;
   pu8_t  mcData = NULL;
   pI2CDeviceAddr_t devAddr = NULL;

   devAddr = demod -> myI2CDevAddr;

   /* Check arguments */
   if ( ( mcInfo == NULL ) ||
        ( mcInfo->mcData == NULL ) ||
        ( mcInfo->mcSize == 0 ) )
   {
      return DRX_STS_INVALID_ARG;
   }

   mcData = mcInfo->mcData;

   /* Check data */
   mcMagicWord = UCodeRead16( mcData );
   mcData += sizeof( u16_t );
   mcNrOfBlks = UCodeRead16( mcData );
   mcData += sizeof( u16_t );

   if ( ( mcMagicWord != UCODE_MAGIC_WORD ) ||
        ( mcNrOfBlks == 0 ) )
   {
      /* wrong endianess or wrong data ? */
      return DRX_STS_INVALID_ARG;
   }

   /* Process microcode blocks */
   for( i = 0 ; i<mcNrOfBlks ; i++ )
   {
      Sleep( 1000 );
      DRXUCodeBlockHdr_t blockHdr;
      u16_t mcBlockNrBytes = 0;

      /* Process block header */
      blockHdr.addr = UCodeRead32( mcData );
      mcData += sizeof(u32_t);
      blockHdr.size = UCodeRead16( mcData );
      mcData += sizeof(u16_t);
      blockHdr.flags = UCodeRead16( mcData );
      mcData += sizeof(u16_t);
      blockHdr.CRC = UCodeRead16( mcData );
      mcData += sizeof(u16_t);

      /* Check block data */
      if ( ( blockHdr.size == 0 ) ||
           ( (( blockHdr.flags & UCODE_CRC_FLAG ) != 0) &&
             ( blockHdr.CRC != UCodeComputeCRC( mcData, blockHdr.size)) ) ||
           (( blockHdr.flags & UCODE_COMPRESSION_FLAG ) != 0)
         )
      {
         /* Wrong data ! */
         return DRX_STS_INVALID_ARG;
      }

      mcBlockNrBytes = blockHdr.size * sizeof(u16_t);

      //Sleep( 100 );
      /* Perform the desired action */
      switch ( action ) {
         /*===================================================================*/
         case UCODE_UPLOAD :
            {
               /* Upload microcode */
               if ( demod->myAccessFunct->writeBlockFunc(
                               devAddr,
                               (DRXaddr_t) blockHdr.addr,
                               mcBlockNrBytes,
                               mcData,
                               0x0000,
                               device) != DRX_STS_OK)
               {
                  return (DRX_STS_OK);
               } /* if */
            };
            break;

         /*===================================================================*/
         case UCODE_VERIFY :
            {
               int result = 0;
               pu8_t  mcBlockData = NULL;

               /* Prepare for ucode download */
               mcBlockData = (pu8_t) DRXBSP_HST_Malloc( (size_t) mcBlockNrBytes );
               if ( mcBlockData == NULL )
               {
                  return DRX_STS_ERROR;
               }

               /* Download microcode block */
               if ( demod->myAccessFunct->readBlockFunc(
                              devAddr,
                              (DRXaddr_t) blockHdr.addr,
                              mcBlockNrBytes,
                              mcBlockData,
                              0x0000,
                              device) != DRX_STS_OK)
               {
                  DRXBSP_HST_Free( mcBlockData );
                  return (DRX_STS_ERROR);
               }

               /* Verify microcode block */
               result = memcmp( mcData, mcBlockData, mcBlockNrBytes);
               DRXBSP_HST_Free( mcBlockData );
               // Return error status (DRX_STS_ERROR) if the verify fails.
               if ( result != 0 )
               {
                  return (DRX_STS_ERROR);
               };
               
            };
            break;

         /*===================================================================*/
         default:
            return DRX_STS_INVALID_ARG;
            break;

      } /* switch ( action ) */


      /* Next block */
      mcData += mcBlockNrBytes;

   } /* for( i = 0 ; i<mcNrOfBlks ; i++ ) */

   return (DRX_STS_OK);
}

/*============================================================================*/

/**
* \fn DRXStatus_t CtrlVersion()
* \brief Build list of version information.
* \param demod A pointer to a demodulator instance.
* \param versionList Pointer to pinter of linked list of versions.
* \return DRXStatus_t.
*/
static DRXStatus_t
CtrlVersion( pDRXDemodInstance_t demod,
             pDRXVersionList_t   *versionList, DtDevice *device )
{
   static char drxDriverCoreModuleName[]  = "DriverCoreModule";
   static char drxDriverCoreVersionText[] =
         DRX_VERSIONSTRING( VERSION_MAJOR, VERSION_MINOR, VERSION_PATCH );

   static DRXVersion_t drxDriverCoreVersion = {
      DRX_MODULE_DRIVERCORE,      /**< type identifier of the module */
      drxDriverCoreModuleName,    /**< name or description of module */
      VERSION_MAJOR,              /**< major version number */
      VERSION_MINOR,              /**< minor version number */
      VERSION_PATCH,              /**< patch version number */
      drxDriverCoreVersionText    /**< version as text string */
   };

   static DRXVersionList_t drxDriverCoreVersionList = {
      &drxDriverCoreVersion,
      NULL
   };

   pDRXVersionList_t demodVersionList = NULL;
   DRXStatus_t returnStatus = DRX_STS_ERROR;

   /* Check arguments */
   if ( versionList == NULL )
   {
      return ( DRX_STS_INVALID_ARG );
   }

   /* Get version info list from demod */
   returnStatus = (*(demod->myDemodFunct->ctrlFunc))(
                           demod,
                           DRX_CTRL_VERSION,
                           (void *) &demodVersionList, device );

   /* Always fill in the information of the driver SW . */
   drxDriverCoreVersion.moduleType  = DRX_MODULE_DRIVERCORE;
   drxDriverCoreVersion.moduleName  = drxDriverCoreModuleName;
   drxDriverCoreVersion.vMajor      = VERSION_MAJOR;
   drxDriverCoreVersion.vMinor      = VERSION_MINOR;
   drxDriverCoreVersion.vPatch      = VERSION_PATCH;
   drxDriverCoreVersion.vString     =
         DRX_VERSIONSTRING( VERSION_MAJOR, VERSION_MINOR, VERSION_PATCH );

   drxDriverCoreVersionList.version = &drxDriverCoreVersion;
   drxDriverCoreVersionList.next    = NULL;

   if (( returnStatus == DRX_STS_OK ) && ( demodVersionList != NULL ))
   {
      /* Append versioninfo from driver to versioninfo from demod  */
      /* Return version info in "bottom-up" order. This way, multiple
         devices can be handled without using malloc. */
      pDRXVersionList_t currentListElement = demodVersionList;
      while ( currentListElement->next != NULL )
      {
         currentListElement = currentListElement->next;
      }
      currentListElement->next = &drxDriverCoreVersionList;

      *versionList = demodVersionList;
   }
   else
   {
      /* Just return versioninfo from driver */
      *versionList = &drxDriverCoreVersionList;
   }

   return (DRX_STS_OK);
}

/*============================================================================*/
/*============================================================================*/
/*== Driver helper functions =================================================*/
/*============================================================================*/
/*============================================================================*/

/**
* \fn u32_t CountInList(pDRXDemodInstance_t demod,pDRXDemodInstance_t listOfDemods [])
* \brief Count occurences of demod pointer in listOfDemods
* \param demod A pointer to a demodulator instance.
* \param listOfDemods Pointer to a linked list of pointers to demods.
* \return Number of occurences.
*/
static u32_t
CountInList( pDRXDemodInstance_t demod,
             pDRXDemodInstance_t listOfDemods [] )
{
   u32_t result = 0;

   if ( ( demod != NULL ) || ( listOfDemods != NULL ) )
   {
      u32_t i = 0;
      while (listOfDemods[i] != NULL)
      {
         if (listOfDemods[i] == demod)
         {
            result++;
         }
         i++;
      }
   }

   return (result);
}

/*============================================================================*/

/**
* \fn Bool_t AllInstancesUnique( pDRXDemodInstance_t listOfDemods [] )
* \brief Check if all pointers in listOfDemods are unique.
* \param listOfDemods Pointer to a linked list of pointers to demods.
* \return TRUE if all pointers are unique, FALSE otherwise.
*/
static Bool_t
AllInstancesUnique( pDRXDemodInstance_t listOfDemods [] )
{
   Bool_t result = FALSE;

   if ( listOfDemods != NULL )
   {
      pDRXDemodInstance_t *demod=listOfDemods;

      while ( (*demod) != NULL )
      {
         if ( CountInList( (*demod), listOfDemods ) > 1 )
         {
            break;
         }
         demod++;
      }

      if ( (*demod) == NULL )
      {
         /* No demod occures more then once */
         result = TRUE;
      }
   }

   return (result);
}

/*============================================================================*/
/*============================================================================*/
/*== Exported functions ======================================================*/
/*============================================================================*/
/*============================================================================*/



/**
* \fn DRXStatus_t DRX_Init(  pDRXDemodInstance_t demods[]  )
* \brief Initialize driver.
* \param demods A non-empty, NULL terminated array of pointers to demodulator instances.
* \return DRXStatus_t Return status.
* \retval DRX_STS_OK Initialization completed.
* \retval DRX_STS_INVALID_ARG The array demods is empty or has invalid content.
*
* Initialize driver. Check if this has already been done, if so abort. Checks
* if each pointer in the demods array is unique, if not abort.
*
*/

DRXStatus_t
DRX_Init(  pDRXDemodInstance_t demods[]  )
{
   //if ( drxDriverDemodsInstancesList != NULL )
   //{
   //   /* DRX_Init() was already called. */
   //   return (DRX_STS_ERROR);
   //}

   if ( ( demods == NULL ) ||
        ( demods[0] == NULL ) ||
        ( AllInstancesUnique( demods ) == FALSE ) )
   {
      return (DRX_STS_INVALID_ARG);
   }

   //drxDriverDemodsInstancesList = &(demods[0]);

   return DRX_STS_OK;
}

/*============================================================================*/

/**
* \fn DRXStatus_t DRX_Term( void )
* \brief Terminate driver.
* \return DRXStatus_t Return status.
* \retval DRX_STS_OK Terminated driver successful.
* \retval DRX_STS_ERROR Error on closing one or more demodulators.
*
* Close all unclosed demodulators.
*
*/

DRXStatus_t
DRX_Term( void )
{
   pDRXDemodInstance_t *demod = NULL;
   DRXStatus_t closingStatus = DRX_STS_OK;

   //if ( drxDriverDemodsInstancesList == NULL )
   //{
   //   /* DRX_Init() not corectly called before calling DRX_Term() */
   //   return (DRX_STS_ERROR);
   //}

   /* Close all demods if necesarry. */
   //demod = drxDriverDemodsInstancesList;
   //while ( (*demod) != NULL )
   //{
   //   if ( ( (*demod)->myDemodFunct != NULL ) &&
   //        ( (*demod)->myCommonAttr != NULL ) &&
   //        ( (*demod)->myI2CDevAddr != NULL ) &&
   //        ( (*demod)->myCommonAttr->isOpened == TRUE ) )
   //   {
   //      if ( DRX_Close( (*demod) ) != DRX_STS_OK )
   //      {
   //         closingStatus = DRX_STS_ERROR;
   //      };
   //   }
   //   demod++;
   //};

   ///* Invalidate driver initialisation. */
   //drxDriverDemodsInstancesList = NULL;

   return (closingStatus);
}

/*============================================================================*/

/**
* \fn DRXStatus_t DRX_Open(pDRXDemodInstance_t demod)
* \brief Open a demodulator instance.
* \param demod A pointer to a demodulator instance.
* \return DRXStatus_t Return status.
* \retval DRX_STS_OK Opened demod instance with succes.
* \retval DRX_STS_INVALID_ARG Demod instance has invalid content.
* \retval DRX_STS_ERROR Driver not initialized or unable to initialize demod.
*
*/

DRXStatus_t
DRX_Open(pDRXDemodInstance_t demod, DtDevice *device)
{
   DRXStatus_t status = DRX_STS_OK;

   //if ( drxDriverDemodsInstancesList == NULL )
   //{
   //   return (DRX_STS_ERROR);
   //}

   if ( ( demod == NULL ) ||
        ( demod->myDemodFunct == NULL ) ||
        ( demod->myCommonAttr == NULL ) ||
        ( demod->myI2CDevAddr == NULL ) ||
        ( demod->myCommonAttr->isOpened == TRUE ) /*||
        ( CountInList( demod, drxDriverDemodsInstancesList) == 0 )*/ )
   {
      return (DRX_STS_INVALID_ARG);
   }

   status = (*(demod->myDemodFunct->openFunc))( demod, device );

   if ( status == DRX_STS_OK )
   {
      demod->myCommonAttr->isOpened = TRUE;
   }

   return ( status );
}

/*============================================================================*/

/**
* \fn Status_t DRX_Close( pDRXDemodInstance_t demod)
* \brief Close device.
* \param demod A pointer to a demodulator instance.
* \return DRXStatus_t Return status.
* \retval DRX_STS_OK Closed demod instance with succes.
* \retval DRX_STS_INVALID_ARG Demod instance has invalid content.
* \retval DRX_STS_ERROR Driver not initialized or error during close demod.
*
* Free resources occupied by device instance.
* Put device into sleep mode.
*/

DRXStatus_t
DRX_Close(pDRXDemodInstance_t demod, DtDevice *device)
{
   DRXStatus_t status = DRX_STS_OK;

   //if ( drxDriverDemodsInstancesList == NULL )
   //{
   //   return (DRX_STS_ERROR);
   //}

   if ( ( demod == NULL ) ||
        ( demod->myCommonAttr == NULL ) ||
        ( demod->myCommonAttr->isOpened == FALSE ) /*||
        ( CountInList( demod, drxDriverDemodsInstancesList) == 0 )*/ )
   {
      return (DRX_STS_INVALID_ARG);
   }

   status = (*(demod->myDemodFunct->closeFunc))( demod, device );

   if ( status == DRX_STS_OK )
   {
      demod->myCommonAttr->isOpened = FALSE;
   }

   return ( status );
}

/*============================================================================*/

/**
* \fn Status_t DRX_Ctrl( pDRXDemodInstance_t demod, DRXCtrlIndex_t ctrl, void *ctrlData)
* \brief Control device.
* \param demod A pointer to a demodulator instance.
* \param ctrl Reference to desired control function.
* \param ctrlData Pointer to data structure for control function.
* \return DRXStatus_t Return status.
* \retval DRX_STS_OK Control function completed successful.
* \retval DRX_STS_INVALID_ARG Demod instance or ctrlData has invalid content.
* \retval DRX_STS_ERROR Driver not initialized or error during control demod.
*
* Data needed or returned by the control function is stored in ctrlData.
*
*/

DRXStatus_t
DRX_Ctrl(pDRXDemodInstance_t demod, DRXCtrlIndex_t ctrl, void *ctrlData, DtDevice *device)
{
   DRXStatus_t status = DRX_STS_ERROR;

   //if ( drxDriverDemodsInstancesList == NULL )
   //{
   //   return (DRX_STS_ERROR);
   //}

   if ( ( demod == NULL ) ||
        /*( CountInList( demod, drxDriverDemodsInstancesList) == 0 ) ||*/
        ( demod->myCommonAttr == NULL ) ||
        ( ( demod->myCommonAttr->isOpened == FALSE ) &&
          ( ctrl != DRX_CTRL_PROBE_DEVICE ) ) )
   {
      return (DRX_STS_INVALID_ARG);
   }

   /* Fixed control functions */
   switch ( ctrl ) {
      /*======================================================================*/
      case DRX_CTRL_NOP:
         /* No operation */
         return (DRX_STS_OK);
         break;

      /*======================================================================*/
      case DRX_CTRL_VERSION:
         {
            return CtrlVersion( demod, (pDRXVersionList_t *) ctrlData, device );
         }
         break;

      /*======================================================================*/
      default :
         /* Do nothing */
         break;
   }

   /* Virtual functions */
   /* First try calling function from derived class */
   status = (*(demod->myDemodFunct->ctrlFunc))( demod, ctrl, ctrlData, device );
   if ( status == DRX_STS_FUNC_NOT_AVAILABLE )
   {
      /* Now try calling a the base class function */
      switch ( ctrl ) {
         /*======================================================================*/
         case DRX_CTRL_LOAD_UCODE:
            {
               return CtrlUCode ( demod,
                                  (pDRXUCodeInfo_t) ctrlData,
                                  UCODE_UPLOAD, device);
            }
            break;

         /*======================================================================*/
         case DRX_CTRL_VERIFY_UCODE:
            {
               return CtrlUCode ( demod,
                                  (pDRXUCodeInfo_t) ctrlData,
                                  UCODE_VERIFY, device);
            }
            break;

         /*======================================================================*/
         case DRX_CTRL_SCAN_INIT:
            {
               return CtrlScanInit( demod, (pDRXScanParam_t) ctrlData );
            }
            break;

         /*======================================================================*/
         case DRX_CTRL_SCAN_NEXT:
            {
               return CtrlScanNext( demod, (pu16_t) ctrlData, device );
            }
            break;

         /*======================================================================*/
         default :
            return (DRX_STS_FUNC_NOT_AVAILABLE);
      }
   } else {
      return (status);
   } /* if ( status == DRX_STS_FUNC_NOT_AVAILABLE ) */

   return (DRX_STS_OK);
}


/*============================================================================*/

/* END OF FILE */
