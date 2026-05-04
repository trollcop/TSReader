/******************************************************************************
 * FILENAME: $Id: drx_dap_wasi.c,v 1.6 2005/10/13 10:49:52 jasper Exp $
 *
 * DESCRIPTION:
 * Part of DRX driver.
 * Data access protocol: Wide Access Sequential Interface (wasi)
 * Wide access, because the full 32 bit address needs to be given
 * Sequential, because of I2C.
 * These functions know how the chip's memory and registers are to be accessed,
 * but nothing more.
 *
 * These functions should not need adapting to a new platform.
 *
 * USAGE:
 * -
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

#include "stdafx.h"
#include <string.h>  /* memcpy, needed in DRXDAP_WASI_WriteBlock() */
#include "drx_dap_wasi.h"

/*===========================================================================*/

/* Function prototypes */
static DRXStatus_t DRXDAP_WASI_WriteBlock (
        pI2CDeviceAddr_t devAddr,       /* address of I2C device        */
        DRXaddr_t        addr,          /* address of register/memory   */
        u16_t            datasize,      /* size of data                 */
        pu8_t            data,          /* data to send                 */
        DRXflags_t       flags,         /* special device flags         */
        DtDevice *device);

static DRXStatus_t DRXDAP_WASI_ReadBlock (
        pI2CDeviceAddr_t devAddr,       /* address of I2C device        */
        DRXaddr_t        addr,          /* address of register/memory   */
        u16_t            datasize,      /* size of data                 */
        pu8_t            data,          /* data to send                 */
        DRXflags_t       flags,         /* special device flags         */
        DtDevice *device);

static DRXStatus_t DRXDAP_WASI_WriteReg8 (
        pI2CDeviceAddr_t devAddr,       /* address of I2C device        */
        DRXaddr_t        addr,          /* address of register          */
        u8_t             data,          /* data to write                */
        DRXflags_t       flags, DtDevice *device);        /* special device flags         */

static DRXStatus_t DRXDAP_WASI_ReadReg8 (
        pI2CDeviceAddr_t devAddr,       /* address of I2C device        */
        DRXaddr_t        addr,          /* address of register          */
        pu8_t            data,          /* buffer to receive data       */
        DRXflags_t       flags, DtDevice *device);        /* special device flags         */

static DRXStatus_t DRXDAP_WASI_ReadModifyWriteReg8 (
        pI2CDeviceAddr_t devAddr,       /* address of I2C device        */
        DRXaddr_t        waddr,         /* address of register          */
        DRXaddr_t        raddr,         /* address to read back from    */
        u8_t            datain,         /* data to send                 */
        pu8_t           dataout, DtDevice *device);       /* data to receive back         */

static DRXStatus_t DRXDAP_WASI_WriteReg16 (
        pI2CDeviceAddr_t devAddr,       /* address of I2C device        */
        DRXaddr_t        addr,          /* address of register          */
        u16_t            data,          /* data to write                */
        DRXflags_t       flags, DtDevice *device);        /* special device flags         */

static DRXStatus_t DRXDAP_WASI_ReadReg16 (
        pI2CDeviceAddr_t devAddr,       /* address of I2C device        */
        DRXaddr_t        addr,          /* address of register          */
        pu16_t           data,          /* buffer to receive data       */
        DRXflags_t       flags, DtDevice *device);        /* special device flags         */

static DRXStatus_t DRXDAP_WASI_ReadModifyWriteReg16 (
        pI2CDeviceAddr_t devAddr,       /* address of I2C device        */
        DRXaddr_t        waddr,         /* address of register          */
        DRXaddr_t        raddr,         /* address to read back from    */
        u16_t            datain,        /* data to send                 */
        pu16_t           dataout, DtDevice *device);      /* data to receive back         */

static DRXStatus_t DRXDAP_WASI_WriteReg32 (
        pI2CDeviceAddr_t devAddr,       /* address of I2C device        */
        DRXaddr_t        addr,          /* address of register          */
        u32_t            data,          /* data to write                */
        DRXflags_t       flags, DtDevice *device);        /* special device flags         */

static DRXStatus_t DRXDAP_WASI_ReadReg32 (
        pI2CDeviceAddr_t devAddr,       /* address of I2C device        */
        DRXaddr_t        addr,          /* address of register          */
        pu32_t           data,          /* buffer to receive data       */
        DRXflags_t       flags, DtDevice *device);        /* special device flags         */

static DRXStatus_t DRXDAP_WASI_ReadModifyWriteReg32 (
        pI2CDeviceAddr_t devAddr,       /* address of I2C device        */
        DRXaddr_t        waddr,         /* address of register          */
        DRXaddr_t        raddr,         /* address to read back from    */
        u32_t            datain,        /* data to send                 */
        pu32_t           dataout, DtDevice *device);      /* data to receive back         */

/* The vesrion structure of this protocol implementation */
char drxDapWasiModuleName[]  = "WASI Data Access Protocol";
char drxDapWasiVesrionText[] = "";

DRXVersion_t drxDapWasiVersion = {
  DRX_MODULE_DAP,             /**< type identifier of the module */
  drxDapWasiModuleName,       /**< name or description of module */

  0,                          /**< major version number */
  0,                          /**< minor version number */
  0,                          /**< patch version number */
  drxDapWasiVesrionText       /**< version as text string */
};

/* The structure containing the protocol interface */
DRXAccessFunc_t drxDapWasiFunct_g = {
   &drxDapWasiVersion,
   DRXDAP_WASI_WriteBlock,               /* Supported */
   DRXDAP_WASI_ReadBlock,                /* Supported */
   DRXDAP_WASI_WriteReg8,                /* Not supported */
   DRXDAP_WASI_ReadReg8,                 /* Not supported */
   DRXDAP_WASI_ReadModifyWriteReg8,      /* Not supported */
   DRXDAP_WASI_WriteReg16,               /* Supported */
   DRXDAP_WASI_ReadReg16,                /* Supported */
   DRXDAP_WASI_ReadModifyWriteReg16,     /* Supported */
   DRXDAP_WASI_WriteReg32,               /* Supported */
   DRXDAP_WASI_ReadReg32,                /* Supported */
   DRXDAP_WASI_ReadModifyWriteReg32      /* Not supported */
};

/*===========================================================================*/

/* Functions not supported by protocol*/

static DRXStatus_t DRXDAP_WASI_WriteReg8 (
        pI2CDeviceAddr_t devAddr,       /* address of I2C device        */
        DRXaddr_t        addr,          /* address of register          */
        u8_t             data,          /* data to write                */
        DRXflags_t       flags,         /* special device flags         */
        DtDevice *device)
{
   return DRX_STS_ERROR;
}

static DRXStatus_t DRXDAP_WASI_ReadReg8 (
        pI2CDeviceAddr_t devAddr,       /* address of I2C device        */
        DRXaddr_t        addr,          /* address of register          */
        pu8_t            data,          /* buffer to receive data       */
        DRXflags_t       flags,         /* special device flags         */
        DtDevice *device)
{
   return DRX_STS_ERROR;
}

static DRXStatus_t DRXDAP_WASI_ReadModifyWriteReg8 (
        pI2CDeviceAddr_t devAddr,       /* address of I2C device        */
        DRXaddr_t        waddr,         /* address of register          */
        DRXaddr_t        raddr,         /* address to read back from    */
        u8_t            datain,         /* data to send                 */
        pu8_t           dataout,         /* special device flags         */
        DtDevice *device)
{
   return DRX_STS_ERROR;
}

static DRXStatus_t DRXDAP_WASI_ReadModifyWriteReg32 (
        pI2CDeviceAddr_t devAddr,       /* address of I2C device        */
        DRXaddr_t        waddr,         /* address of register          */
        DRXaddr_t        raddr,         /* address to read back from    */
        u32_t            datain,        /* data to send                 */
        pu32_t           dataout,         /* special device flags         */
        DtDevice *device)
{
   return DRX_STS_ERROR;
}

/*===========================================================================*/

/******************************
 *
 * DRXStatus_t DRXDAP_WASI_ReadBlock (
 *      pI2CDeviceAddr_t devAddr,      -- address of I2C device
 *      DRXaddr_t        addr,         -- address of chip register/memory
 *      u16_t            datasize,     -- number of bytes to read
 *      pu8_t            data,         -- data to receive
 *      DRXflags_t       flags)        -- special device flags
 *
 * Read block data from chip address. Because the chip is word oriented,
 * the number of bytes to read must be even.
 *
 * Make sure that the buffer to receive the data is large enough.
 *
 * Although this function expects an even number of bytes, it is still byte
 * oriented, and the data read back is NOT translated to the endianness of
 * the target platform.
 *
 * Output:
 * - DRX_STS_OK     if reading was successful
 *                  in that case: data read is in *data.
 * - DRX_STS_ERROR  if anything went wrong
 *
 ******************************/

static DRXStatus_t DRXDAP_WASI_ReadBlock (
    pI2CDeviceAddr_t devAddr,
    DRXaddr_t        addr,
    u16_t            datasize,
    pu8_t            data,
    DRXflags_t       flags,
    DtDevice         *device)
{
    u8_t buf[4];
    u16_t bufx;
    DRXStatus_t rc;

#if  DRXDAP_MAX_WCHUNKSIZE ==  DRXDAP_MAX_WCHUNKSIZE_MIN
    /* In the smallest chunk size, 10-bit addresses are not allowed */
    if (IS_I2C_10BIT (devAddr->i2cAddr))
    {
        return DRX_STS_INVALID_ARG;
    }
#endif

    datasize &= ~1;     /* Rounded down to even number */
    if (!data && datasize)
    {
        return DRX_STS_INVALID_ARG;
    }

    addr  &= ~DRXDAP_WASI_FLAGS;
    /* ReadModifyWrite & mode flag bits are not allowed */
    flags &= (~DRXDAP_WASI_RMW & ~DRXDAP_WASI_MODEFLAGS);
#if DRXDAP_SINGLE_MASTER
    flags |= DRXDAP_WASI_SINGLE_MASTER;
#endif
    addr |= flags;

    do {
        u16_t todo = (datasize <  DRXDAP_MAX_RCHUNKSIZE ? datasize :  DRXDAP_MAX_RCHUNKSIZE);

        bufx = 0;

        buf[bufx++] = (u8_t) ((addr >>  0) & 0xFF);
        buf[bufx++] = (u8_t) ((addr >> 16) & 0xFF);
        buf[bufx++] = (u8_t) ((addr >> 24) & 0xFF);
        buf[bufx++] = (u8_t) ((addr >>  8) & 0xFF);

#if DRXDAP_SINGLE_MASTER
        /*
         * In single master mode, split the read and write actions.
         * No special action is needed for write chunks here.
         */
        rc = DRXBSP_I2C_WriteRead (devAddr, bufx, buf, 0, 0, 0, device);
        if (rc == DRX_STS_OK)
        {
            rc = DRXBSP_I2C_WriteRead (0, 0, 0, devAddr, todo, data, device);
        }
#else
    /* In multi master mode, do everything in one RW action */
        rc = DRXBSP_I2C_WriteRead (devAddr, bufx, buf, devAddr, todo, data, device);
#endif
        data += todo;
        addr += (todo >> 1);
        datasize -= todo;
    } while (datasize && rc == DRX_STS_OK);

    return rc;
}




/******************************
 *
 * DRXStatus_t DRXDAP_WASI_ReadModifyWriteReg16 (
 *      pI2CDeviceAddr_t devAddr,   -- address of I2C device
 *      DRXaddr_t        waddr,     -- address of chip register/memory
 *      DRXaddr_t        raddr,     -- chip address to read back from
 *      u16_t            wdata,     -- data to send
 *      pu16_t           rdata)     -- data to receive back
 *
 * Write 16-bit data, then read back the original contents of that location.
 *
 * Before sending data, the data is converted to little endian. The
 * data received back is converted back to the target platform's endianness.
 *
 * WARNING: This function is only guaranteed to work if there is one
 * master on the I2C bus.
 *
 * Output:
 * - DRX_STS_OK     if reading was successful
 *                  in that case: read back data is at *rdata
 * - DRX_STS_ERROR  if anything went wrong
 *
 ******************************/

static DRXStatus_t DRXDAP_WASI_ReadModifyWriteReg16 (
    pI2CDeviceAddr_t devAddr,
    DRXaddr_t        waddr,
    DRXaddr_t        raddr,
    u16_t            wdata,
    pu16_t           rdata,
    DtDevice         *device)
{
    DRXStatus_t rc;

    if (rdata == NULL)
    {
        return DRX_STS_INVALID_ARG;
    }

    rc = DRXDAP_WASI_WriteReg16 (devAddr, waddr, wdata, DRXDAP_WASI_RMW, device);
    if (rc != DRX_STS_OK)
    {
        return rc;
    }
    return DRXDAP_WASI_ReadReg16 (devAddr, raddr, rdata, 0, device);
}




/******************************
 *
 * DRXStatus_t DRXDAP_WASI_ReadReg16 (
 *     pI2CDeviceAddr_t devAddr, -- address of I2C device
 *     DRXaddr_t        addr,    -- address of chip register/memory
 *     pu16_t           data,    -- data to receive
 *     DRXflags_t       flags)   -- special device flags
 *
 * Read one 16-bit register or memory location. The data received back is
 * converted back to the target platform's endianness.
 *
 * Output:
 * - DRX_STS_OK     if reading was successful
 *                  in that case: read data is at *data
 * - DRX_STS_ERROR  if anything went wrong
 *
 ******************************/

static DRXStatus_t DRXDAP_WASI_ReadReg16 (
    pI2CDeviceAddr_t devAddr,
    DRXaddr_t        addr,
    pu16_t           data,
    DRXflags_t       flags,
    DtDevice *device)
{
    u8_t buf[sizeof (*data)];
    DRXStatus_t rc;

    if (!data)
    {
        return DRX_STS_INVALID_ARG;
    }
    rc = DRXDAP_WASI_ReadBlock (devAddr, addr, sizeof (*data), buf, flags, device);
    *data = buf[0] + (((u16_t) buf[1]) << 8);
    return rc;
}




/******************************
 *
 * DRXStatus_t DRXDAP_WASI_ReadReg32 (
 *     pI2CDeviceAddr_t devAddr, -- address of I2C device
 *     DRXaddr_t        addr,    -- address of chip register/memory
 *     pu32_t           data,    -- data to receive
 *     DRXflags_t       flags)   -- special device flags
 *
 * Read one 32-bit register or memory location. The data received back is
 * converted back to the target platform's endianness.
 *
 * Output:
 * - DRX_STS_OK     if reading was successful
 *                  in that case: read data is at *data
 * - DRX_STS_ERROR  if anything went wrong
 *
 ******************************/

static DRXStatus_t DRXDAP_WASI_ReadReg32 (
    pI2CDeviceAddr_t devAddr,
    DRXaddr_t        addr,
    pu32_t           data,
    DRXflags_t       flags,
    DtDevice *device)
{
    u8_t buf[sizeof (*data)];
    DRXStatus_t rc;

    if (!data)
    {
        return DRX_STS_INVALID_ARG;
    }
    rc = DRXDAP_WASI_ReadBlock (devAddr, addr, sizeof (*data), buf, flags, device);
    *data = (((u32_t) buf[0]) <<  0) +
            (((u32_t) buf[1]) <<  8) +
            (((u32_t) buf[2]) << 16) +
            (((u32_t) buf[3]) << 24);
    return rc;
}




/******************************
 *
 * DRXStatus_t DRXDAP_WASI_WriteBlock (
 *      pI2CDeviceAddr_t devAddr,    -- address of I2C device
 *      DRXaddr_t        addr,       -- address of chip register/memory
 *      u16_t            datasize,   -- number of bytes to read
 *      pu8_t            data,       -- data to receive
 *      DRXflags_t       flags)      -- special device flags
 *
 * Write block data to chip address. Because the chip is word oriented,
 * the number of bytes to write must be even.
 *
 * Although this function expects an even number of bytes, it is still byte
 * oriented, and the data being written is NOT translated from the endianness of
 * the target platform.
 *
 * Output:
 * - DRX_STS_OK     if writing was successful
 * - DRX_STS_ERROR  if anything went wrong
 *
 ******************************/

static DRXStatus_t DRXDAP_WASI_WriteBlock (
    pI2CDeviceAddr_t devAddr,
    DRXaddr_t        addr,
    u16_t            datasize,
    pu8_t            data,
    DRXflags_t       flags,
    DtDevice         *device)
{
    u8_t buf[ DRXDAP_MAX_WCHUNKSIZE];
    DRXStatus_t st;
    u16_t overhead_size = sizeof (addr) + (IS_I2C_10BIT (devAddr->i2cAddr) ? 2 : 1);
    u16_t blocksize = ( DRXDAP_MAX_WCHUNKSIZE - overhead_size) & ~1;

    if (overhead_size >  DRXDAP_MAX_WCHUNKSIZE_MIN)
    {
        return DRX_STS_INVALID_ARG;
    }

    flags &= DRXDAP_WASI_FLAGS;
    flags &= ~DRXDAP_WASI_MODEFLAGS;
#if DRXDAP_SINGLE_MASTER
    flags |= DRXDAP_WASI_SINGLE_MASTER;
#endif

    datasize &= ~1;     /* Rounded down to even number */
    if (datasize && !data)
    {
        return DRX_STS_INVALID_ARG;
    }

    do
    {
        //Sleep( 100 );
        u16_t todo;
        u16_t bufx = 0;
        addr  &= ~DRXDAP_WASI_FLAGS;
        addr |= flags;

        todo = (blocksize < datasize ? blocksize : datasize);
        buf[bufx++] = (u8_t) ((addr >>  0) & 0xFF);
        buf[bufx++] = (u8_t) ((addr >> 16) & 0xFF);
        buf[bufx++] = (u8_t) ((addr >> 24) & 0xFF);
        buf[bufx++] = (u8_t) ((addr >>  8) & 0xFF);
        /*
        In single master mode blocksize can be 0. In such a case this I2C
        sequense will be visible: (1) write address {i2c addr,
        4 bytes chip address} (2) write data {i2c addr, 4 bytes data }
        (3) write address (4) write data etc...
        Addres must be rewriten because HI is reset after data transport and
        expects an address.
        */
        if (!todo)
        {
            /* write address */
            st = DRXBSP_I2C_WriteRead (devAddr, (u16_t)(bufx + todo), buf, NULL, 0, NULL, device);
            bufx = 0;
            todo = (4 < datasize ? 4 : datasize);
        }
        memcpy (&buf[bufx], data, todo);
        /* write (address if can do and) data */
        st = DRXBSP_I2C_WriteRead (devAddr, (u16_t)(bufx + todo), buf, NULL, 0, NULL, device);
        datasize -= todo;
        data += todo;
        addr += (todo >> 1);
    } while (datasize);
    return st;
}




/******************************
 *
 * DRXStatus_t DRXDAP_WASI_WriteReg16 (
 *     pI2CDeviceAddr_t devAddr, -- address of I2C device
 *     DRXaddr_t        addr,    -- address of chip register/memory
 *     u16_t            data,    -- data to send
 *     DRXflags_t       flags)   -- special device flags
 *
 * Write one 16-bit register or memory location. The data being written is
 * converted from the target platform's endianness to little endian.
 *
 * Output:
 * - DRX_STS_OK     if writing was successful
 * - DRX_STS_ERROR  if anything went wrong
 *
 ******************************/

static DRXStatus_t DRXDAP_WASI_WriteReg16 (
    pI2CDeviceAddr_t devAddr,
    DRXaddr_t        addr,
    u16_t            data,
    DRXflags_t       flags,
    DtDevice *device)
{
    u8_t buf[sizeof (data)];

    buf[0] = (u8_t) ( (data >> 0 ) & 0xFF );
    buf[1] = (u8_t) ( (data >> 8 ) & 0xFF );

    return DRXDAP_WASI_WriteBlock (devAddr, addr, sizeof (data), buf, flags, device);
}




/******************************
 *
 * DRXStatus_t DRXDAP_WASI_WriteReg32 (
 *     pI2CDeviceAddr_t devAddr, -- address of I2C device
 *     DRXaddr_t        addr,    -- address of chip register/memory
 *     u32_t            data,    -- data to send
 *     DRXflags_t       flags)   -- special device flags
 *
 * Write one 32-bit register or memory location. The data being written is
 * converted from the target platform's endianness to little endian.
 *
 * Output:
 * - DRX_STS_OK     if writing was successful
 * - DRX_STS_ERROR  if anything went wrong
 *
 ******************************/

static DRXStatus_t DRXDAP_WASI_WriteReg32 (
    pI2CDeviceAddr_t devAddr,
    DRXaddr_t        addr,
    u32_t            data,
    DRXflags_t       flags,
    DtDevice *device)
{
    u8_t buf[sizeof (data)];

    buf[0] = (u8_t) ( (data >> 0 ) & 0xFF );
    buf[1] = (u8_t) ( (data >> 8 ) & 0xFF );
    buf[2] = (u8_t) ( (data >> 16) & 0xFF );
    buf[3] = (u8_t) ( (data >> 24) & 0xFF );

    return DRXDAP_WASI_WriteBlock (devAddr, addr, sizeof (data), buf, flags, device);
}

