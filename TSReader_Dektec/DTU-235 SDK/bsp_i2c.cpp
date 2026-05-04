/******************************************************************************
 * FILENAME: $Id: bsp_i2c.c,v 1.7 2005/07/08 14:43:40 carlo Exp $
 *
 * DESCRIPTION:
 * Includes a Micronas-provided I2C implementation.
 * Also serves as the template for a target-specific I2C implementation.
 *
 * USAGE:
 * Outcomment and fill-out the template implementation
 * OR include target-specific I2C source files, surrounded by a compiler flag.
 *
 * NOTES:
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
 * AUTHOR:
 * Martin Sinot
 *
 *****************************************************************************/


/******************************************************************************
 *
 * If no BSP_I2C implementation is selected, use I2CECP as default
 *
 *****************************************************************************/
#include "stdafx.h"

/*
#if !defined(BSP_I2C_USE_I2CECP) && \
    !defined(BSP_I2C_USE_I2CUSB) && \
    !defined(BSP_I2C_USE_I2CLPT) && \
    !defined(BSP_I2C_USE_DUMMY)  && \
    !defined(BSP_I2C_USE_EPROM_GEN)
    
/* Default usage: Micronas I2CECP driver
#define BSP_I2C_USE_I2CECP 1
#endif

/******************************************************************************
 *
 * Include the selected (Micronas-provided) BSP_I2C implementation.
 * Note: not all of these implementations may have been supplied in the package.
 *
 *****************************************************************************/

/* ECP driver
#ifdef BSP_I2C_USE_I2CECP
#include "i2cecp.c.inc"
#include "i2cecp_stub.c.inc"
#endif

/* USB driver
#ifdef BSP_I2C_USE_I2CUSB
#include "i2cusb.c.inc"
#endif

/* LPT driver 
#ifdef BSP_I2C_USE_I2CLPT
#include "i2clpt.c.inc"
#endif

/* Dummy driver 
#ifdef BSP_I2C_USE_DUMMY
#include "i2cdummy.c.inc"
#endif

/* EPROM generator 
#ifdef BSP_I2C_USE_EPROM_GEN
#include "i2cepromgen.c.inc"
#endif
*/
/******************************************************************************
 *
 * Below, a template is provided for a custom BSP_I2C implementation
 *
 *****************************************************************************/

#if 1

/*
 * To create a custom BSP_I2C implementation:
 *
 * - Disable the default (Micronas-supplied) I2C interface,
 *    e.g. by removing (or out-commenting) all lines above this section.
 *
 * - Also remove (or out-comment) the '#if 0' (above)
 *     and the matching '#endif'(at the end of this file).
 *
 * - Implement the BSP_I2C functions for your target system.
 *    Empty function bodies and brief explanations of the required
 *    functionality are included below. Please refer to Driver Manual for
 *    further details of these functions.
 *
 */

/* Necessary includes go here */
#include "bsp_i2c.h"

/* Store errors from your I2C subsystem as-is in this variable */
//int DRX_I2C_Error_g; //Made local to single function that needs it (DRXBSP_I2C_WriteRead)


/******************************
 *
 * DRXStatus_t DRXBSP_I2C_Init (void)
 *
 * Prepare the platform's I2C subsystem for usage.
 * Should be called before any other I2C function is called.
 *
 * Output:
 * - DRX_STS_OK    system is now ready for use
 * - other         something wrong
 *
 ******************************/

DRXStatus_t DRXBSP_I2C_Init( void )
{
    /* TODO: initialize your I2C subsystem */
    return DRX_STS_OK;
}


/******************************
 *
 * DRXStatus_t DRXBSP_I2C_Term (void)
 *
 * Shut down the I2C subsystem of the target platform.
 * After this, I2C functions should no longer be called, until the system is
 * initialized again.
 *
 * Output:
 * - DRX_STS_OK     I2C subsystem shut down
 * - other          something wrong
 *
 ******************************/

DRXStatus_t DRXBSP_I2C_Term( void )
{
    /* TODO: terminate your I2C subsystem */
    return DRX_STS_OK;
}


/******************************
 *
 * DRXStatus_t DRXBSP_I2C_WriteRead(
 *             pI2CDeviceAddr_t wDevAddr,     -- Device to write to
 *             u16_t            wCount,       -- nr of bytes to write
 *             pu8_t            wData,        -- the data to write
 *             pI2CDeviceAddr_t rDevAddr,     -- Device to read from
 *             u16_t            rCount,       -- nr of bytes to read
 *             pu8_t            rData     )   -- buffer receiving the data
 *
 * Write data to I2C device, then receive back specified number of bytes.
 * Must also support write-only (when rDevAddr is NULL)
 *               and read-only  (when wDevAddr is NULL).
 *
 * Output:
 * - DRX_STS_OK           data successfully transferred
 *                        in that case: data received, if any, is in rData
 * - DRX_STS_INVALID_ARG  invalid arguments passed to this function
 * - DRX_STS_ERROR        something wrong in the transfer
 *
 ******************************/

DRXStatus_t DRXBSP_I2C_WriteRead( pI2CDeviceAddr_t wDevAddr,
                                  u16_t            wCount,
                                  pu8_t            wData,
                                  pI2CDeviceAddr_t rDevAddr,
                                  u16_t            rCount,
                                  pu8_t            rData,
                                  DtDevice         *device)
{   
    int DRX_I2C_Error_g = 0;
    int lwAddress, lrAddress;
    char *lwData, *lrData;
    int lwCount, lrCount;
    DTAPI_RESULT dtResult;
    // We must have data for either the read or the write.
    if (!wDevAddr && !rDevAddr)
    {
        return DRX_STS_INVALID_ARG;
    }
    // Check write data and convert to Dektec format.
    if (wDevAddr)
    {
        // Make sure we have valid data...
        if (!wData && wCount)
        {
            return DRX_STS_INVALID_ARG;
        }
        lwAddress = (int) (wDevAddr->i2cAddr);
        lwData = (char*) wData;
        lwCount = (int) wCount;
        // lwData cannot be null sending it to the Dektec.
        if( lwData == NULL )
        {
            lwData = new char['0'];
        }
    }
    // Check read data and convert to Dektec format.
    if (rDevAddr)
    {
        // Make sure we have valid data...
        if (!rData && rCount)
        {
            return DRX_STS_INVALID_ARG;
        }
        lrAddress = (int) (rDevAddr->i2cAddr);
        lrData = (char*) rData;
        lrCount = (int) rCount;   
    }
    // Write the data...
    if (wDevAddr)
    {
        dtResult = device->I2CWrite( lwAddress, lwData, lwCount );
        // Make sure write was succesful, which Dektec just assumes for the most part.
        if( dtResult != DTAPI_OK )
        {
           DRX_I2C_Error_g = -5;
        }
    }
    // Read the data...
    if (rDevAddr)
    {
        dtResult = device->I2CRead( lrAddress, lrData, lrCount );
        // Make sure read was succesful, which Dektec just assumes for the most part.
        if( dtResult != DTAPI_OK )
        {
            DRX_I2C_Error_g = -5;
        }
    }
    // Check for errors
    if ( DRX_I2C_Error_g )
    {
        return DRX_STS_ERROR;
    }
    return DRX_STS_OK;
}


/******************************
 *
 * char* DRXBSP_I2C_ErrorText( void )
 *
 * Returns a human readable error.
 * Counter part of numerical DRX_I2C_Error_g.
 *
 * Output:
 * - Pointer to string containing error description.
 *
 ******************************/

char* DRXBSP_I2C_ErrorText( void )
{
   /* TODO: explain your I2C subsystem-specific error code */
   //switch ( DRX_I2C_Error_g ) {
   //   case 0:
   //      return ("No error");
   //      break;
   //   /*
   //   case SOME_ERROR:
   //      return ("Some error");
   //      break;
   //   */
   //   default:
   //      return("Unknown error");
   //      break;
   //} /* switch */

   return("DRXBSP_I2C_ErrorText error") ;
}


#endif
