/******************************************************************************
 * FILENAME: $Id: i2c_test.c,v 1.3 2005/02/23 12:37:02 paulja Exp $
 *
 * DESCRIPTION:
 * Test program to verify correct functioning of bsp_i2c implementation.
 *
 * USAGE:
 * Compile with flag BSP_I2C_USE_I2CECP=1 if this test runs on the I2C/ECP
 * Protocol Driver. Undefine this flag to run the test on the target platform.
 *
 * NOTES:
 * $(c) 2003-2005 Micronas GmbH. All rights reserved.
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

#include "stdafx.h"
#include <stdarg.h>
#include <stdio.h>

#include "drx_dap_wasi.h"

/* Define this to use main */
#define USE_MAIN

/* I2C test parameters - adapt to your board */
#define I2C_DEVICE              (0xE0)    /* Change to address of device */
#define TEST_MEM_ADDR           0x820112  /* Change to address of memory area */
#define TEST_MEM_ADDR_READBACK  0x420018  /* Change to readback address */


/* Macro's to replace drx_access.h interface with drx_dap_wasi.h interface */

#define DRX_Write( dev, addr, size, data, flags, device ) \
        drxDapWasiFunct_g.writeBlockFunc( (dev), (addr), (size), (data), (flags), device )

#define DRX_Read( dev, addr, size, data, flags, device ) \
        drxDapWasiFunct_g.readBlockFunc( (dev), (addr), (size), (data), (flags), device )

#define DRX_WriteReg16( dev, addr, data, flags, device ) \
        drxDapWasiFunct_g.writeReg16Func( (dev), (addr), (data), (flags), device )

#define DRX_ReadReg16( dev, addr, data, flags, device ) \
        drxDapWasiFunct_g.readReg16Func( (dev), (addr), (data), (flags), device )

#define DRX_ReadModifyWriteReg16( dev, waddr, raddr, datain, dataout, device ) \
        drxDapWasiFunct_g.readModifyWriteReg16Func( (dev), (waddr), (raddr), (datain), (dataout), device )

#define DRX_WriteReg32( dev, addr, data, flags, device ) \
        drxDapWasiFunct_g.writeReg32Func( (dev), (addr), (data), (flags), device )

#define DRX_ReadReg32( dev, addr, data, flags, device ) \
        drxDapWasiFunct_g.readReg32Func( (dev), (addr), (data), (flags), device )


/*
 * Print error function.
 * Implement your own version.
 */
int xprint (char* fmt, ...)
{
    int rc;
    va_list arg;

    va_start (arg, fmt);
    rc = vfprintf (stderr, fmt, arg);
    va_end (arg);
    return rc;
}


int xgetchar()
{
   return getchar();
}


/*
 * Test return values
 */
typedef enum {
    TEST_OK,                 /* Test OK                                             */
    TEST_INIT_FAILED,        /* I2C subsystem not initialised                       */
    TEST_ARG_FAILED,         /* Argument check failed                               */
    TEST_WRITE_I2C_FAILED,   /* Error in I2C transmission during write              */
    TEST_READ_I2C_FAILED,    /* Error in I2C transmission during read               */
    TEST_RMW_I2C_FAILED,     /* Error in I2C transmission during read-modify-write  */
    TEST_READ_FAILED,        /* Read error unrelated to I2C                         */
    TEST_RMW_FAILED,         /* Read-modify-write error unrelated to I2C            */
    TEST_READ_ERROR_FAILED,  /* Error in reading not properly detected              */
    TEST_WRITE_ERROR_FAILED, /* Error in writing not properly detected              */
    TEST_TERMINATE_FAILED    /* I2C subsystem not properly terminated               */
} TestError_t;



/******************************
 *
 * Test_API
 *
 * This function tests all functions of the API.
 * No printfs should be used in this function, so that it can be used
 * on an embedded platform.
 *
 * Returns TEST_OK if all tests passed, or some other TestError_t value
 * indicating failure. In that case, testnr indicates which test failed.
 *
 * If an error is returned, then the caller should inspect DRX_I2C_Error_g
 * to see if the error is I2C related. This variable is platform dependent,
 * but should be zero for a successful I2C transmission.
 *
 ******************************/

typedef TestError_t (*Test_API_Func_t)(pI2CDeviceAddr_t, DRXaddr_t, DRXaddr_t, DtDevice *device);


TestError_t Test_API_Init (pI2CDeviceAddr_t dev, DRXaddr_t addr, DRXaddr_t addr_rb, DtDevice *device)
{
    /* Init library            */
    /* Coverage : DRXBSP_I2C_Init */
    if (DRXBSP_I2C_Init () != DRX_STS_OK)
    {
        return TEST_INIT_FAILED;
    }
    return TEST_OK;
}

TestError_t Test_API_DRXBSP_RW_Param (pI2CDeviceAddr_t dev, DRXaddr_t addr, DRXaddr_t addr_rb, DtDevice *device)
{
    /* Check DRXBSP_I2C_WriteRead with faulty parameters */
    /* Coverage: DRXBSP_I2C_WriteRead error checking     */
    u8_t dummy[4];
    if (DRXBSP_I2C_WriteRead (dev, 4, 0, dev, 4, dummy, device) != DRX_STS_INVALID_ARG)
    {
        return TEST_ARG_FAILED;
    }
    if (DRXBSP_I2C_WriteRead (dev, 4, dummy, dev, 4, 0, device) != DRX_STS_INVALID_ARG)
    {
        return TEST_ARG_FAILED;
    }
    if (DRXBSP_I2C_WriteRead (0, 4, dummy, 0, 4, 0, device) != DRX_STS_INVALID_ARG)
    {
        return TEST_ARG_FAILED;
    }
    return TEST_OK;
}

TestError_t Test_API_DRX_Write (pI2CDeviceAddr_t dev, DRXaddr_t addr, DRXaddr_t addr_rb, DtDevice *device)
{
    /* Write 8 bytes to specified address */
    /* Coverage: DRXBSP_I2C_WriteRead, DRX_Write */
    u8_t bufout[8];
    int i;

    for (i = 0; i < 8; i++)
    {
        bufout[i] = (u8_t)(i | 0x80);
    }
    if (DRX_Write (dev, addr, 8, bufout, 0, device) != DRX_STS_OK)
    {
        return TEST_WRITE_I2C_FAILED;
    }
    return TEST_OK;
}

TestError_t Test_API_DRX_Read (pI2CDeviceAddr_t dev, DRXaddr_t addr, DRXaddr_t addr_rb, DtDevice *device)
{
    /* Read 8 bytes from specified address */
    /* Coverage: DRXBSP_I2C_WriteRead, DRX_Read */
    u8_t bufin[8];
    int i;

    for (i = 0; i < 8; i++)
    {
        bufin[i] = 0;
    }
    if (DRX_Read (dev, addr, 8, bufin, 0, device) != DRX_STS_OK)
    {
        return TEST_READ_I2C_FAILED;
    }
    for (i = 0; i < 8; i++)
    {
        if (bufin[i] != (u8_t)(i | 0x80))
        {
            return TEST_READ_FAILED;
        }
    }
    return TEST_OK;
}

TestError_t Test_API_DRX_Write2 (pI2CDeviceAddr_t dev, DRXaddr_t addr, DRXaddr_t addr_rb, DtDevice *device)
{
    /* Write 8 bytes to specified address */
    /* (different values)                 */
    /* Coverage: DRXBSP_I2C_WriteRead, DRX_Write */
    u8_t bufout[8];
    int i;

    for (i = 0; i < 8; i++)
    {
        bufout[i] = (u8_t)(0x7F - i);
    }
    if (DRX_Write (dev, addr, 8, bufout, 0, device) != DRX_STS_OK)
    {
        return TEST_WRITE_I2C_FAILED;
    }
    return TEST_OK;
}

TestError_t Test_API_DRX_Read2 (pI2CDeviceAddr_t dev, DRXaddr_t addr, DRXaddr_t addr_rb, DtDevice *device)
{
    /* Read 8 bytes from specified address    */
    /* (verifies that registers are writable) */
    /* Coverage: DRXBSP_I2C_WriteRead, DRX_Read       */
    u8_t bufin[8];
    int i;

    for (i = 0; i < 8; i++)
    {
        bufin[i] = 0;
    }
    if (DRX_Read (dev, addr, 8, bufin, 0, device) != DRX_STS_OK)
    {
        return TEST_READ_I2C_FAILED;
    }
    for (i = 0; i < 8; i++)
    {
        if (bufin[i] != (u8_t)(0x7F - i))
        {
            return TEST_READ_FAILED;
        }
    }
    return TEST_OK;
}

TestError_t Test_API_DRX_Write_Odd (pI2CDeviceAddr_t dev, DRXaddr_t addr, DRXaddr_t addr_rb, DtDevice *device)
{
    /* Write 7 bytes to specified address         */
    /* then read back 8 bytes                     */
    /* Coverage: DRX_Write handling of odd counts */
    u8_t bufio[8];
    int i;

    for (i = 0; i < 8; i++)
    {
        bufio[i] = (u8_t)(i | 0x80);
    }
    if (DRX_Write (dev, addr, 7, bufio, 0, device) != DRX_STS_OK)
    {
        return TEST_WRITE_I2C_FAILED;
    }
    for (i = 0; i < 8; i++)
    {
        bufio[i] = 0;
    }
    if (DRX_Read (dev, addr, 8, bufio, 0, device) != DRX_STS_OK)
    {
        return TEST_READ_I2C_FAILED;
    }
    /* First six bytes must be the new data */
    for (i = 0; i < 6; i++)
    {
        if (bufio[i] != (u8_t)(i | 0x80))
        {
            return TEST_READ_FAILED;
        }
    }
    /* Next bytes must be the old data of the previous test */
    for (i = 6; i < 8; i++)
    {
        if (bufio[i] != (u8_t)(0x7F - i))
        {
            return TEST_READ_FAILED;
        }
    }
    return TEST_OK;
}

TestError_t Test_API_DRX_Read_Odd (pI2CDeviceAddr_t dev, DRXaddr_t addr, DRXaddr_t addr_rb, DtDevice *device)
{
    /* Test 9  : read back 7 bytes                      */
    /* Coverage: DRX_Read handling of odd counts        */
    u8_t bufio[8];
    int i;

    for (i = 0; i < 8; i++)
    {
        bufio[i] = 0;
    }
    if (DRX_Read (dev, addr, 7, bufio, 0, device) != DRX_STS_OK)
    {
        return TEST_READ_I2C_FAILED;
    }
    /* First six bytes must be the new data */
    for (i = 0; i < 6; i++)
    {
        if (bufio[i] != (u8_t)(i | 0x80))
        {
            return TEST_READ_FAILED;
        }
    }
    /* Next bytes must be zero (not overwritten) */
    for (i = 6; i < 8; i++)
    {
        if (bufio[i] != 0)
        {
            return TEST_READ_FAILED;
        }
    }
    return TEST_OK;
}

TestError_t Test_API_DRX_Read_Param (pI2CDeviceAddr_t dev, DRXaddr_t addr, DRXaddr_t addr_rb, DtDevice *device)
{
    /* Check DRX_Read with faulty parameters */
    /* Coverage: DRX_Read error checking     */
    if (DRX_Read (dev, addr, 4, 0, 0, device) != DRX_STS_INVALID_ARG)
    {
        return TEST_ARG_FAILED;
    }
    return TEST_OK;
}

TestError_t Test_API_DRX_Write_Param (pI2CDeviceAddr_t dev, DRXaddr_t addr, DRXaddr_t addr_rb, DtDevice *device)
{
    /* Check DRX_Write with faulty parameters */
    /* Coverage: DRX_Write error checking     */
    if (DRX_Write (dev, addr, 4, 0, 0, device) != DRX_STS_INVALID_ARG)
    {
        return TEST_ARG_FAILED;
    }
    return TEST_OK;
}

TestError_t Test_API_DRX_ReadModifyWriteReg16_Param (pI2CDeviceAddr_t dev, DRXaddr_t addr, DRXaddr_t addr_rb, DtDevice *device)
{
    /* Check DRX_ReadModifyWriteReg16 with faulty argument */
    /* Coverage: DRX_ReadModifyWriteReg16 error checking   */
    if (DRX_ReadModifyWriteReg16 (dev, addr, addr_rb, 0, 0, device) != DRX_STS_INVALID_ARG)
    {
        return TEST_ARG_FAILED;
    }
    return TEST_OK;
}

TestError_t Test_API_DRX_ReadReg16 (pI2CDeviceAddr_t dev, DRXaddr_t addr, DRXaddr_t addr_rb, DtDevice *device)
{
    /* Check DRX_ReadReg16 with faulty argument */
    /* Coverage: DRX_ReadReg16 error checking   */
    if (DRX_ReadReg16 (dev, addr, 0, 0, device) != DRX_STS_INVALID_ARG)
    {
        return TEST_ARG_FAILED;
    }
    return TEST_OK;
}

TestError_t Test_API_DRX_ReadReg32 (pI2CDeviceAddr_t dev, DRXaddr_t addr, DRXaddr_t addr_rb, DtDevice *device)
{
    /* Check DRX_ReadReg32 with faulty argument */
    /* Coverage: DRX_ReadReg16 error checking   */
    if (DRX_ReadReg32 (dev, addr, 0, 0, device) != DRX_STS_INVALID_ARG)
    {
        return TEST_ARG_FAILED;
    }
    return TEST_OK;
}

TestError_t Test_API_DRX_ReadReg16_I2C (pI2CDeviceAddr_t dev, DRXaddr_t addr, DRXaddr_t addr_rb, DtDevice *device)
{
    /* Check DRX_ReadReg16 with invalid I2C address */
    /* Coverage: DRX_ReadReg16, DRX_Read            */
    u16_t data;

    if (DRX_ReadReg16 (&dev[1], addr, &data, 0, device) != DRX_STS_ERROR
#ifdef BSP_I2C_USE_I2CECP
        || DRX_I2C_Error_g != I2CECP_STATUS_I2C_NAK
#endif
        )
    {
        return TEST_READ_ERROR_FAILED;
    }
    return TEST_OK;
}

TestError_t Test_API_DRX_ReadModifyWriteReg16_I2C (pI2CDeviceAddr_t dev, DRXaddr_t addr, DRXaddr_t addr_rb, DtDevice *device)
{
    /* Check DRX_ReadModifyWriteReg16 with invalid I2C address */
    /* Coverage: DRX_ReadModifyWriteReg16                                */
    u16_t data;

    if (DRX_ReadModifyWriteReg16 (&dev[1], addr, addr_rb, 0, &data, device) != DRX_STS_ERROR
#ifdef BSP_I2C_USE_I2CECP
        || DRX_I2C_Error_g != I2CECP_STATUS_I2C_NAK
#endif
        )
    {
        return TEST_WRITE_ERROR_FAILED;
    }
    return TEST_OK;
}

TestError_t Test_API_DRX_WriteReg16_I2C (pI2CDeviceAddr_t dev, DRXaddr_t addr, DRXaddr_t addr_rb, DtDevice *device)
{
    /* Check DRX_WriteReg16 with invalid I2C address */
    /* Coverage: DRX_WriteReg16, DRX_Write           */
    if (DRX_WriteReg16 (&dev[1], addr, 0, 0, device) != DRX_STS_ERROR
#ifdef BSP_I2C_USE_I2CECP
        || DRX_I2C_Error_g != I2CECP_STATUS_I2C_NAK
#endif
        )
    {
        return TEST_WRITE_ERROR_FAILED;
    }
    return TEST_OK;
}

TestError_t Test_API_DRX_ReadReg32_I2C (pI2CDeviceAddr_t dev, DRXaddr_t addr, DRXaddr_t addr_rb, DtDevice *device)
{
    /* Check DRX_ReadReg32 with invalid I2C address */
    /* Coverage: DRX_ReadReg32, DRX_Read            */
    u32_t data;

    if (DRX_ReadReg32 (&dev[1], addr, &data, 0, device) != DRX_STS_ERROR
#ifdef BSP_I2C_USE_I2CECP
        || DRX_I2C_Error_g != I2CECP_STATUS_I2C_NAK
#endif
        )
    {
        return TEST_READ_ERROR_FAILED;
    }
    return TEST_OK;
}

TestError_t Test_API_DRX_WriteReg32_I2C (pI2CDeviceAddr_t dev, DRXaddr_t addr, DRXaddr_t addr_rb, DtDevice *device)
{
    /* Check DRX_WriteReg32 with invalid I2C address */
    /* Coverage: DRX_WriteReg32, DRX_Write           */
    if (DRX_WriteReg32 (&dev[1], addr, 0, 0, device) != DRX_STS_ERROR
#ifdef BSP_I2C_USE_I2CECP
        || DRX_I2C_Error_g != I2CECP_STATUS_I2C_NAK
#endif
        )
    {
        return TEST_WRITE_ERROR_FAILED;
    }
    return TEST_OK;
}

TestError_t Test_API_DRX_ReadWriteReg16 (pI2CDeviceAddr_t dev, DRXaddr_t addr, DRXaddr_t addr_rb, DtDevice *device)
{
    /* Write 16-bit register and read back     */
    /* Coverage: DRX_WriteReg16, DRX_ReadReg16 */
    u16_t datain = 0;

    if (DRX_WriteReg16 (dev, addr, 0x1234, 0, device) != DRX_STS_OK)
    {
        return TEST_WRITE_I2C_FAILED;
    }
    if (DRX_WriteReg16 (dev, addr + 1, 0, 0, device) != DRX_STS_OK)
    {
        return TEST_WRITE_I2C_FAILED;
    }
    if (DRX_ReadReg16 (dev, addr, &datain, 0, device) != DRX_STS_OK)
    {
        return TEST_READ_I2C_FAILED;
    }
    if (datain != 0x1234)
    {
        return TEST_READ_FAILED;
    }
    return TEST_OK;
}

TestError_t Test_API_DRX_ReadWriteReg32 (pI2CDeviceAddr_t dev, DRXaddr_t addr, DRXaddr_t addr_rb, DtDevice *device)
{
    /* Write 32-bit register and read back     */
    /* Coverage: DRX_WriteReg32, DRX_ReadReg32 */
    u32_t datain = 0;

    if (DRX_WriteReg32 (dev, addr, 0x23456789, 0, device) != DRX_STS_OK)
    {
        return TEST_WRITE_I2C_FAILED;
    }
    if (DRX_WriteReg32 (dev, addr + 2, 0, 0, device) != DRX_STS_OK)
    {
        return TEST_WRITE_I2C_FAILED;
    }
    if (DRX_ReadReg32 (dev, addr, &datain, 0, device) != DRX_STS_OK)
    {
        return TEST_READ_I2C_FAILED;
    }
    if (datain != 0x23456789)
    {
        return TEST_READ_FAILED;
    }
    return TEST_OK;
}

TestError_t Test_API_DRX_ReadModifyWriteReg16 (pI2CDeviceAddr_t dev, DRXaddr_t addr, DRXaddr_t addr_rb, DtDevice *device)
{
    /* Check read-modify-write            */
    /* Coverage: DRX_ReadModifyWriteReg16 */
    u16_t datain = 0;

    if (DRX_WriteReg16 (dev, addr, 0x4321, 0, device) != DRX_STS_OK)
    {
        return TEST_WRITE_I2C_FAILED;
    }
    if (DRX_WriteReg16 (dev, addr + 1, 0, 0, device) != DRX_STS_OK)
    {
        return TEST_WRITE_I2C_FAILED;
    }
    if (DRX_ReadModifyWriteReg16 (dev, addr, addr_rb, 0x8765, &datain, device) != DRX_STS_OK)
    {
        return TEST_RMW_I2C_FAILED;
    }
    if (datain != 0x4321)
    {
        return TEST_RMW_FAILED;
    }
    if (DRX_ReadReg16 (dev, addr, &datain, 0, device) != DRX_STS_OK)
    {
        return TEST_READ_I2C_FAILED;
    }
    if (datain != 0x8765)
    {
        return TEST_READ_FAILED;
    }
    return TEST_OK;
}

TestError_t Test_API_Terminate (pI2CDeviceAddr_t dev, DRXaddr_t addr, DRXaddr_t addr_rb, DtDevice *device)
{
    /* Test 23 : shut down I2C subsystem */
    /* Coverage: DRXBSP_I2C_Term         */
    if (DRXBSP_I2C_Term () != DRX_STS_OK)
    {
        return TEST_TERMINATE_FAILED;
    }

    return TEST_OK;
}




#ifdef USE_MAIN


int main (DtDevice *device)
{
    u32_t ver = 0; /* DRX_Lib_Version (); */
    TestError_t test_result;
    int testnr;
    int nsuccess;
    static Test_API_Func_t Test_API[] = {
        NULL,
        Test_API_Init,
        Test_API_DRXBSP_RW_Param,
        Test_API_DRX_Write,
        Test_API_DRX_Read,
        Test_API_DRX_Write2,
        Test_API_DRX_Read2,
        Test_API_DRX_Write_Odd,
        Test_API_DRX_Read_Odd,
        Test_API_DRX_Read_Param,
        Test_API_DRX_Write_Param,
        Test_API_DRX_ReadModifyWriteReg16_Param,
        Test_API_DRX_ReadReg16,
        Test_API_DRX_ReadReg32,
        Test_API_DRX_ReadReg16_I2C,
        Test_API_DRX_ReadModifyWriteReg16_I2C,
        Test_API_DRX_WriteReg16_I2C,
        Test_API_DRX_ReadReg32_I2C,
        Test_API_DRX_WriteReg32_I2C,
        Test_API_DRX_ReadWriteReg16,
        Test_API_DRX_ReadWriteReg32,
        Test_API_DRX_ReadModifyWriteReg16,
        Test_API_Terminate
    };
    static I2CDeviceAddr_t testdev[2] = { { I2C_DEVICE }, { I2C_DEVICE + 8} };

    xprint ("Version = %d.%d.%04d\n",
            (int)((ver >> 20) & 0xFFF),
            (int)((ver >> 16) & 0xF),
            (int)(ver & 0xFFFF));
#if DRXDAP_SINGLE_MASTER
    xprint ("Compiled in single master mode\n");
#else
    xprint ("Compiled in multi master mode\n");
#endif
    xprint ("Write chunk size = %u bytes\n", (unsigned)  DRXDAP_MAX_WCHUNKSIZE);
    xprint ("Read chunk size  = %u bytes\n", (unsigned)  DRXDAP_MAX_RCHUNKSIZE);
    for (testnr = 1, nsuccess = 1;
         testnr < (sizeof (Test_API) / sizeof (Test_API[0]));
         testnr++)
    {
        xprint ("\rTest %d: ", testnr);
        test_result = Test_API[testnr] (&testdev[0], TEST_MEM_ADDR, TEST_MEM_ADDR_READBACK, device);
        switch (test_result)
        {
        case TEST_OK:
            nsuccess++;
            break;
        case TEST_INIT_FAILED:
            xprint ("I2C subsystem not initialised\n");
            testnr = sizeof (Test_API) / sizeof (Test_API[0]) - 1;
            break;
        case TEST_ARG_FAILED:
            xprint ("Invalid arguments in function not detected\n");
            break;
        case TEST_WRITE_I2C_FAILED:
            xprint ("I2C error in write\n   %s\n",DRXBSP_I2C_ErrorText());
            break;
        case TEST_READ_I2C_FAILED:
            xprint ("I2C error in read\n   %s\n", DRXBSP_I2C_ErrorText());
            break;
        case TEST_RMW_I2C_FAILED:
            xprint ("I2C error in read-modify-write\n   %s\n", DRXBSP_I2C_ErrorText());
            break;
        case TEST_READ_FAILED:
            xprint ("Error in readback (non I2C-related)\n");
            break;
        case TEST_RMW_FAILED:
            xprint ("Error in read-modify-write readback (non I2C-related)\n");
            break;
        case TEST_READ_ERROR_FAILED:
            xprint ("I2C error in read expected\n   %s\n", DRXBSP_I2C_ErrorText());
            break;
        case TEST_WRITE_ERROR_FAILED:
            xprint ("I2C error in write expected\n   %s\n", DRXBSP_I2C_ErrorText());
            break;
        case TEST_TERMINATE_FAILED:
            xprint ("I2C subsystem not properly shutdown\n");
            break;
        default:
            xprint ("Unknown error!\n");
            break;
        }
    }

    if (nsuccess == testnr)
    {
        xprint ("\rAll tests passed\n");
    }
    xprint ("\r%d test%s failed\n", testnr - nsuccess,
        (testnr - nsuccess == 1 ? "" : "s"));

    xprint ("\nPress enter to exit program.\n");
    xgetchar();

    return 0;
}

#endif  /* USE_MAIN */

