/******************************************************************************
 * FILENAME: $Id: drx_dap_wasi.h,v 1.2 2005/01/27 13:22:43 paulja Exp $
 *
 * DESCRIPTION:
 * Part of DRX API
 * Data access protocol: Wide Access Sequential Interface (wasi)
 * Wide access, because the full 32 bit address needs to be given
 * Sequential, because of I2C.
 *
 * USAGE:
 * Include.
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
 * Paul Janssen
 *
 *****************************************************************************/

/*-------- compilation control switches -------------------------------------*/

#ifndef __DRX_DAP_WASI_H__
#define __DRX_DAP_WASI_H__

/*-------- Required includes ------------------------------------------------*/

#include "drx_driver.h"

/*-------- Defines, configuring the API -------------------------------------*/

/*
 * Chunk/mode checking
 */
#ifndef DRXDAP_SINGLE_MASTER
#define DRXDAP_SINGLE_MASTER 1
#endif

/*
* Comments about DRXDAP_MAX_WCHUNKSIZE in single or multi master mode:
*
* In single master mode, data can be written by sending the register address
* first, then two or four bytes of data in the next packet.
* Because the device address plus a register address equals five bytes,
* the mimimum chunk size must be five.
* If ten-bit I2C device addresses are used, the minimum chunk size must be six,
* because the I2C device address will then occupy two bytes when writing.
*
* Data in single master mode is transferred as follows:
* <S> <devW>  a0  a1  a2  a3  <P>
* <S> <devW>  d0  d1 [d2  d3] <P>
* ..
* or
* ..
* <S> <devW>  a0  a1  a2  a3  <P>
* <S> <devR> --- <P>
*
* In multi-master mode, the data must immediately follow the address (an I2C
* stop resets the internal address), and hence the minimum chunk size is
* 1 <I2C address> + 4 (register address) + 2 (data to send) = 7 bytes (8 if
* 10-bit I2C device addresses are used).
*/

#if !defined( DRXDAP_MAX_WCHUNKSIZE)
#define  DRXDAP_MAX_WCHUNKSIZE 62
//#define  DRXDAP_MAX_WCHUNKSIZE 254
#endif

#if DRXDAP_SINGLE_MASTER
#define  DRXDAP_MAX_WCHUNKSIZE_MIN 5
#else
#define  DRXDAP_MAX_WCHUNKSIZE_MIN 7
#endif

#if  DRXDAP_MAX_WCHUNKSIZE <  DRXDAP_MAX_WCHUNKSIZE_MIN
#if DRXDAP_SINGLE_MASTER
#error  DRXDAP_MAX_WCHUNKSIZE must be at least 5 in single master mode
*;   /* illegal statement to force compiler error */
#else
#error  DRXDAP_MAX_WCHUNKSIZE must be at least 7 in multi master mode
*;   /* illegal statement to force compiler error */
#endif
#endif

#if !defined( DRXDAP_MAX_RCHUNKSIZE)
#define  DRXDAP_MAX_RCHUNKSIZE 62
//#define  DRXDAP_MAX_RCHUNKSIZE 254
#endif

#if  DRXDAP_MAX_RCHUNKSIZE < 2
#error  DRXDAP_MAX_RCHUNKSIZE must be at least 2
*;   /* illegal statement to force compiler error */
#endif

#if  DRXDAP_MAX_RCHUNKSIZE & 1
#error  DRXDAP_MAX_RCHUNKSIZE must be even
*;   /* illegal statement to force compiler error */
#endif

/*-------- Public API functions ---------------------------------------------*/

#ifdef __cplusplus
extern "C" {
#endif


extern DRXAccessFunc_t drxDapWasiFunct_g;

#define DRXDAP_WASI_RMW           0x10000000
#define DRXDAP_WASI_BROADCAST     0x20000000
#define DRXDAP_WASI_CLEARCRC      0x80000000
#define DRXDAP_WASI_SINGLE_MASTER 0xC0000000
#define DRXDAP_WASI_MODEFLAGS     0xC0000000
#define DRXDAP_WASI_FLAGS         0xF0000000

#ifdef __cplusplus
}
#endif


#endif         /* __DRX_DAP_WASI_H__ */


