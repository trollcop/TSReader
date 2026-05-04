/**
* \file $Id: tuner5byte.h,v 1.2 2005/07/20 10:08:50 paulja Exp $
*
* \brief Tuner interface for 5 byte tuners
*
* \author Paul Janssen
*/

/*
* $(c) 2005 Micronas GmbH. All rights reserved.
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

#ifndef __TUNER5BYTE_H__
#define __TUNER5BYTE_H__
/*------------------------------------------------------------------------------
INCLUDES
------------------------------------------------------------------------------*/
#include "bsp_tuner.h"

#ifdef __cplusplus
extern "C" {
#endif

/*------------------------------------------------------------------------------
DEFINES
------------------------------------------------------------------------------*/


/*------------------------------------------------------------------------------
TYPEDEFS
------------------------------------------------------------------------------*/

typedef struct {

   DRXFrequency_t maxFreq; /* Maximum RF center frequency (kHz) for which these       */
                           /* control bytes are still valid,                          */
                           /*   set to ((u32_t)-1) for last entry in array            */
                           /*                                                         */
   u8_t           CB1;     /* First  control byte                                     */
   u8_t           CB2;     /* Second control byte                                     */

} TUNER5BYTEControlUnit_t, *pTUNER5BYTEControlUnit_t;


typedef struct {

   TUNERMode_t         modes;              /* modes supported by this control table   */

   char               *description;        /* description of this control mode, e.g.: */
                                           /*   bandswitch, auxiliary, analog, ...    */

   DRXFrequency_t      refFreqNumerator;   /* Reference frequency, numerator, in kHz  */
   u32_t               refFreqDenominator; /* Reference frequency, denominator        */

   DRXFrequency_t      outputFreq;         /* IF output frequency, in kHz             */

   pTUNER5BYTEControlUnit_t ControlUnits;
        /* Pointer to an array of control units,
           one for each 'frequency band' that is defined in the tuner datasheet
        */

} TUNER5BYTEControlTable_t, *pTUNER5BYTEControlTable_t;


typedef struct {
   u8_t            lockMask;            /* Mask to apply to check tuner lock          */
   u8_t            lockValue;           /* Value that indicates tuner lock            */

   pTUNER5BYTEControlTable_t controlTable;
        /* Pointer to an array  control tables.
           Multiple control tables can be used to for different modes
           of the same tuner, e.g. bandswitch control, auxiliary control,
           analog modem, etc.
        */
   u16_t controlTables;      /* number of entries in the ControlTable array           */

} TUNER5BYTEData_t, *pTUNER5BYTEData_t;

/*------------------------------------------------------------------------------
ENUM
------------------------------------------------------------------------------*/

/*------------------------------------------------------------------------------
STRUCTS
------------------------------------------------------------------------------*/


/*------------------------------------------------------------------------------
Exported FUNCTIONS
------------------------------------------------------------------------------*/

extern DRXStatus_t TUNER_5BYTE_Open ( pTUNERInstance_t tuner, DtDevice *device );

extern DRXStatus_t TUNER_5BYTE_Close( pTUNERInstance_t tuner );

extern DRXStatus_t TUNER_5BYTE_SetFrequency( pTUNERInstance_t tuner,
                                             TUNERMode_t      mode,
                                             DRXFrequency_t   frequency,
                                             DtDevice *device);

extern DRXStatus_t TUNER_5BYTE_GetFrequency( pTUNERInstance_t tuner,
                                             pDRXFrequency_t  RFfrequency,
                                             pDRXFrequency_t  IFfrequency );

extern DRXStatus_t TUNER_5BYTE_LockStatus(   pTUNERInstance_t   tuner,
                                             pTUNERLockStatus_t lockStat,
                                             DtDevice *device);

/*-------------------------------------------------------------------------
Exported GLOBAL VARIABLES
-------------------------------------------------------------------------*/
extern TUNERFunc_t TUNER5BYTEFunctions_g;

/*------------------------------------------------------------------------------
THE END
------------------------------------------------------------------------------*/
#ifdef __cplusplus
}
#endif
#endif   /* __TUNER5BYTE_H__ */

/* End of file */
