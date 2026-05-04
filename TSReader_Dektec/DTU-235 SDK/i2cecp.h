/******************************************************************************
 * FILENAME: $Id: i2cecp.h,v 1.2 2005/03/14 15:03:32 paulja Exp $
 *
 * DESCRIPTION:
 * Part of I2C/ECP implemenation of bsp_i2c module.
 * Contains the API functions
 *
 * USAGE:
 * Include.
 *
 * NOTES:
 * $(c) 2001-2005 Micronas GmbH. All rights reserved.
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

#ifndef __I2CAPI_H
#define __I2CAPI_H

#ifdef VXD_KERNEL
#error This file must not be included in a driver!
#endif

#ifdef  __cplusplus
extern "C" {
#endif

/*
 * I2CECP status
 */
enum {
    I2CECP_STATUS_OK              =   0,
    I2CECP_STATUS_IO_PENDING      =  -1,
    I2CECP_STATUS_ILLEGAL_COMMAND =  -2,
    I2CECP_STATUS_I2C_BUSY        =  -3,
    I2CECP_STATUS_I2C_TIMEOUT     =  -4,
    I2CECP_STATUS_I2C_NAK         =  -5,
    I2CECP_STATUS_ECP_BUSY        =  -6,
    I2CECP_STATUS_ECP_NOBUF       =  -7,
    I2CECP_STATUS_ECP_ABORT       =  -8,
    I2CECP_STATUS_LOCKED          =  -9,
    I2CECP_STATUS_CANCELLED       = -10,
    I2CECP_STATUS_NOT_INITIALIZED = -11,
};


/*
 * I2C commands
 */
enum I2CCommand {
    I2C_Start,          /* I2C_Start            - generate start condition */
    I2C_RepStart,       /* I2C_RepStart         - generate repeated start condition */
    I2C_Stop,           /* I2C_Stop             - generate stop condition */
    I2C_ReadN,          /* I2C_ReadN n          - read n bytes of data */
    I2C_WriteN,         /* I2C_WriteN n data... - write n bytes of data */
    I2C_WriteBits       /* I2C_WriteBits n long - write n bits of long data (skip MSB)*/
};


/*
 * Driver configuration
 */
#define DRIVER_HAS_ECP      0x0001          /* ECP port detected */
#define DRIVER_HAS_INT      0x0002          /* IRQ functions correctly */
#define DRIVER_HAS_DMA      0x0004          /* DMA functions correctly */
#define DRIVER_PIO_FORCED   0x0008          /* Driver uses PIO to transfer ECP data */

/*
 * Driver mode
 */
enum DriverMode {
    DM_GetMode,
    DM_I2CECP,
    DM_Paraclip
};


/* Administrative functions */
BOOL  WINAPI I2CECP_IsPresent        (void);
int   WINAPI I2CECP_GetVersion       (void);
char* WINAPI I2CECP_GetVersionString (void);
int   WINAPI I2CECP_Config           (void);
int   WINAPI I2CECP_GetMode          (void);
int   WINAPI I2CECP_GetNrInt         (void);
int   WINAPI I2CECP_WaitForInterrupt (void);
int   WINAPI I2CECP_Lock             (void);
int   WINAPI I2CECP_Unlock           (void);

/* I2C functions */
int  WINAPI I2CECP_SendI2C           (LPBYTE bI2CCommand, unsigned uSize);
int  WINAPI I2CECP_ReceiveI2C        (LPBYTE bI2CBuffer, unsigned uSize);
int  WINAPI I2CECP_I2C               (LPBYTE bI2CCommand, unsigned uCmdSize,
                                      LPBYTE bI2CResult, unsigned uResultSize);
void WINAPI I2CECP_ResetI2C          (void);
int  WINAPI I2CECP_SendI2CPulse      (LPBYTE bI2CPulseTrain, unsigned uSize);
int  WINAPI I2CECP_SendI2CPulseN     (LPBYTE bI2CPulseTrain, unsigned uSize);

/* ECP functions */
int  WINAPI I2CECP_OpenECP           (void);
int  WINAPI I2CECP_CloseECP          (void);
int  WINAPI I2CECP_AbortECP          (void);
int  WINAPI I2CECP_GetECP            (LPBYTE bBuffer, unsigned uSize,
                                      HANDLE* uHandle, unsigned uTimeout);
int  WINAPI I2CECP_WaitECP           (HANDLE uHandle, unsigned uTimeout);

unsigned WINAPI I2CECP_GetCalibrate (void);
unsigned WINAPI I2CECP_GetDelay (void);
unsigned WINAPI I2CECP_SetDelay (unsigned);

int WINAPI I2CECP_FakeInt (void);

/*  Driver configuration */
#define I2CECP_CONFIG_MODE          0x01
#define I2CECP_CONFIG_DMA           0x02
#define I2CECP_CONFIG_LOGLEVEL      0x04
#define I2CECP_CONFIG_PORT          0x08
#define I2CECP_CONFIG_DRIVER_PATH   0x10

#define I2CECP_CONFIG_WIN95 (I2CECP_CONFIG_MODE | \
                             I2CECP_CONFIG_PORT | \
                             I2CECP_CONFIG_DRIVER_PATH)

#define I2CECP_CONFIG_WINNT (I2CECP_CONFIG_MODE | \
                             I2CECP_CONFIG_DMA | \
                             I2CECP_CONFIG_LOGLEVEL | \
                             I2CECP_CONFIG_PORT)

typedef struct _I2CECPDriverConfigData {
    DWORD dwSize;       /* Size of structure. */
                        /* Must be set to sizeof (I2CECPDriverConfig) by application */
                        /* (indication of version of structure) */
    DWORD dwFlags;      /*  Requested configuration modification */
                        /*  (unused if reading info) */
    DWORD dwMode;       /*  Driver mode */
    DWORD dwDMA;        /*  DMA channel (NT only) */
    DWORD dwLogLevel;   /*  Event log level (NT only) */
    DWORD dwNrPorts;    /*  Number of printer ports (read only) */
    char  cPort[64];    /*  Port modification */
    char  cDriverPath[MAX_PATH];  /* Driver path (Win95 only) */
} I2CECPDriverConfigData;

#define I2CECP_CONFERR_OK            0  /* Config access OK */
#define I2CECP_CONFERR_ERROR        -1  /* Config access failed */
#define I2CECP_CONFERR_NEED_REBOOT   1  /* Computer must be rebooted for modifications */


int  WINAPI I2CECP_GetDriverConfig   (I2CECPDriverConfigData* data);
int  WINAPI I2CECP_SetDriverConfig   (I2CECPDriverConfigData* data);

/* temp solution to start/stop driver */
int  WINAPI I2CECP_StartDriver();
int  WINAPI I2CECP_StopDriver();

#ifdef  __cplusplus
}
#endif

#endif  /* __I2CAPI_H */


