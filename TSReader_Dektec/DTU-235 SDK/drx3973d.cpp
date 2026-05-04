/**
* \file $Id: drx3973d.c,v 1.96 2005/10/26 10:22:46 paulja Exp $
*
* \brief DRX3973D specific implementation of DRX driver.
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
#include "drx_dap_wasi.h"     /* data access protocol */
#include "drx3973d.h"
#include "drx3973d_map.h"

/*-----------------------------------------------------------------------------
ENUMS
----------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------
DEFINES
----------------------------------------------------------------------------*/

/**
* \def DRX3973D_DEF_I2C_ADDR
* \brief Default I2C addres of a demodulator instance.
*/
#define DRX3973D_DEF_I2C_ADDR ((0x70)<<1)

/**
* \def DRX3973D_DEF_DEMOD_DEV_ID
* \brief Default device identifier of a demodultor instance.
*/
#define DRX3973D_DEF_DEMOD_DEV_ID   (1)


/**
* \def DRX3973D_SCAN_TIMEOUT
* \brief Timeout value for waiting on demod lock during channel scan (millisec).
*/
#define DRX3973D_SCAN_TIMEOUT    604

/**
* \def DRX3973D_DAP
* \brief Name of structure containing all data access protocol functions.
*/
#define DRX3973D_DAP drxDapWasiFunct_g

/**
* \def HI_I2C_DELAY
* \brief HI timing delay for I2C timing (in nano seconds)
*
*  Used to compute HI_CFG_DIV, should be 250 ns.
*
*/
#define HI_I2C_DELAY    84

/**
* \def HI_I2C_BRIDGE_DELAY
* \brief HI timing delay for I2C timing (in nano seconds)
*
*  Used to compute HI_CFG_BDL, should be 1000 ns.
*  Set to 1550 to enable working with murata tuner.
*
*/
#define HI_I2C_BRIDGE_DELAY   750

/**
* \def EQ_TD_TPS_PWR_XXXX
* \brief TD_TPS_PWR constants
*
*  Used in SetChannel.
*
*/
#define EQ_TD_TPS_PWR_UNKNOWN          0x00C0   /* Unknown configurations */
#define EQ_TD_TPS_PWR_QPSK             0x016A
#define EQ_TD_TPS_PWR_QAM16_ALPHAN     0x0195
#define EQ_TD_TPS_PWR_QAM16_ALPHA1     0x0195
#define EQ_TD_TPS_PWR_QAM16_ALPHA2     0x011E
#define EQ_TD_TPS_PWR_QAM16_ALPHA4     0x01CE
#define EQ_TD_TPS_PWR_QAM64_ALPHAN     0x019F
#define EQ_TD_TPS_PWR_QAM64_ALPHA1     0x019F
#define EQ_TD_TPS_PWR_QAM64_ALPHA2     0x00F8
#define EQ_TD_TPS_PWR_QAM64_ALPHA4     0x014D


/**
* \def DRX3973D_DEF_XXXX
* \brief Shadow register default values
*
*  Needed for AGC IF/RF control.
*
*/
#define DRX3973D_DEF_AG_PWD_CONSUMER 0x000E
#define DRX3973D_DEF_AG_PWD_PRO 0x0000
#define DRX3973D_DEF_AG_AGC_SIO 0x0000

/**
* \def DRX3973D_OSCDEV_DO_SCAN
* \brief Instruct SC to do a scan for oscillator frequency deviation.
*
* This defines the search range for a clock deviation. Has to be even.
* If set to n the search range will be from -n/2 ... n/2 times the specified
* step size.
*/
#define DRX3973D_OSCDEV_DO_SCAN  (16)

/**
* \def DRX3973D_OSCDEV_DONT_SCAN
* \brief Instruct SC to skip the scan for oscillator frequency deviation.
*
* Search range is set to 0.
*/
#define DRX3973D_OSCDEV_DONT_SCAN  (0)

/**
* \def DRX3973D_OSCDEV_STEP
* \brief Step size for scan for oscillator frequency deviation.
*
* This defines the stepsize used during the search.
* 275 is approx. a 25 ppm step when tuning to an 8 Mhz channel (worst case).
* 220 is approx. a 20 ppm step when tuning to an 8 Mhz channel (worst case).
*/
#define DRX3973D_OSCDEV_STEP  (275)

/**
* \def DRX3973D_BANDWIDTH_8MHZ_IN_HZ
* \brief Defines for bandwidths in Hz (6,7 & 8Mhz).
*/
/* (64/7)*(8/8)*1000000 Hz */
#define DRX3973D_BANDWIDTH_8MHZ_IN_HZ  (0x8B8249L)
/* (64/7)*(7/8)*1000000 */
#define DRX3973D_BANDWIDTH_7MHZ_IN_HZ  (0x7A1200L)
/* (64/7)*(6/8)*1000000 */
#define DRX3973D_BANDWIDTH_6MHZ_IN_HZ  (0x68A1B6L)

/*============================================================================*/
/*=== WORKAROUNDS ============================================================*/
/*============================================================================*/

/**
* \def USE_LC_INIT
* \brief If defined compile with usage of InitLC routine, otherwise without
*
* LCInit() is needed by LC code version 0.1.32.
* LCInit() is NOT needed by LC code version 0.1.67 or higher.
* TODO remove
*
*/
/*
#define USE_LC_INIT
*/

/**
* \def COMPILE_FOR_QT
* \brief If defined compile for QT.
*
* Several settings are different for QT environment.
* TODO remove
*
*/
/*
#define COMPILE_FOR_QT
*/

/**
* \def DONT_USE_SEMAPHORES
* \brief If defined , do not use semaphores for HI or SC communication.
*
* Due to problem with read-modify-write access originaly spec-ed semaphores
* cannot be used. The semaphores may not reside in HI or SC data RAM.
*
*/
#define DONT_USE_SEMAPHORES

/*============================================================================*/
/*=== REGISTER ACCESS MACROS =================================================*/
/*============================================================================*/

#ifdef DRX3973DDRIVER_DEBUG
#include <stdio.h>
#define CHK_ERROR( s ) \
        do{ \
            if ( (s) != DRX_STS_OK ) \
            { \
               fprintf(stderr, \
                       "ERROR[\n file    : %s\n line    : %d\n]\n", \
                       __FILE__,__LINE__); \
               goto rw_error; }; \
            } \
        while (0)
#else
#define CHK_ERROR( s ) \
        do{ \
            if ( (s) != DRX_STS_OK ) \
            { \
               goto rw_error; }; \
            } \
        while (0)
#endif

#define WR16( dev, addr, val, flags, device) \
        CHK_ERROR( DRX3973D_DAP.writeReg16Func( (dev), \
                                                (addr), \
                                                (val), \
                                                (flags), \
                                                (device)) )

#define RR16( dev, addr, val, flags, device) \
        CHK_ERROR( DRX3973D_DAP.readReg16Func( (dev), \
                                               (addr), \
                                               (val), \
                                               (flags), \
                                                (device)) )

#define RMWR16( dev, addr, wval, rval, device) \
        CHK_ERROR( DRX3973D_DAP.readModifyWriteReg16Func( (dev), \
                                             (addr), \
                                             (HI_RA_RAM_SLV0_READBACK__A), \
                                             (wval), \
                                             (rval), \
                                                (device)) )

#define BCWR16( dev, addr, val, device ) \
        CHK_ERROR( DRX3973D_DAP.writeReg16Func( (dev), \
                                                (addr), \
                                                (val), \
                                                DRXDAP_WASI_BROADCAST, \
                                                (device)) )

#define WRBLOCK( dev, addr, size, data, device ) \
        CHK_ERROR( DRX3973D_DAP.writeBlockFunc( (dev), \
                                                (addr), \
                                                (size), \
                                                (data), \
                                                (0), \
                                                (device)) )

/*============================================================================*/
/*=== HI COMMAND RELATED DEFINES =============================================*/
/*============================================================================*/

/* TODO find a sensible nr of retries based upon i2c speed */
// This is usually 1000
#define DRX3973D_MAX_RETRIES (250)

/*-----------------------------------------------------------------------------
STATIC VARIABLES
----------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------
GLOBAL VARIABLES
----------------------------------------------------------------------------*/
/**
* \var DRX3973D_Func_g
* \brief The driver functions of the drx3973D
*/
DRXDemodFunc_t DRX3973DFunctions_g  =
{
   DRX3975D_TYPE_ID,
   DRX3973D_Open,
   DRX3973D_Close,
   DRX3973D_Ctrl
};

/**
* \var DRX3973DData_g
* \brief Extended attributes for a demodulator ot the DRX3973D family.
*
* Device specific attributes for DRX3973D family. They are initialised
* when calling the DRX_Open() function. They must be allocated before
* calling the DRX_Open() function.
*
*/
DRX3973DData_t DRX3973DData_g =
{
   0,                               /* expected sysClockFreq */
   /* HI configuration */
   0,                               /* HI Configure() parameter 2 */
   0,                               /* HI Configure() parameter 3 */
   0,                               /* HI Configure() parameter 4 */
   0,                               /* HI Configure() parameter 5 */
   /* GET/SET Channel */
   0,                               /* frequency shifter setting  */
   /* Chip Id */
   FALSE,                           /* consumer device flag */
   /* OC configuration */
   0,                               /* shadow EcOcRegOcMpgSio */
   0,                               /* shadow EcOcRegOcModeLop */
   0,                               /* shadow EcOcRegOcModeHip */
   0,                               /* shadow EcOcRegIprInvMpg */
   0,                               /* shadow FeAgRegAgPwd */
   0,                               /* shadow FeAgRegAgAgcSio */
   /* UIO configuartion */
   DRX_UIO_MODE_DISABLE,            /* initial UIO mode HI */
   DRX_UIO_MODE_DISABLE,            /* initial UIO mode SC */
   /* AGC control */
   {             /* IF AGC settings */
      DRX3973D_AGC_CTRL_AUTO,       /* ctrlMode */
      0,                            /* level */
      0,                            /* min */
      0,                            /* max */
      0                             /* speed */
   },
   {             /* RF AGC settings */
      DRX3973D_AGC_CTRL_AUTO,       /* ctrlMode */
      0,                            /* level */
      0,                            /* min */
      0,                            /* max */
      0                             /* speed */
   },
   DRX3973D_CSCD_INIT,              /* system clock deviation FSM */
   (DRXBandwidth_t) 0,                               /* current bandwidth selection */
   FALSE                            /* ignore lock status for SigQuality */
};

/**
* \var DRX3973DDefaultAddr_g
* \brief Default I2C address and device identifier.
*/
I2CDeviceAddr_t DRX3973DDefaultAddr_g = {
      DRX3973D_DEF_I2C_ADDR,     /* i2c address */
      DRX3973D_DEF_DEMOD_DEV_ID  /* device id */
};

/**
* \var DRX3973DDefaultCommAttr_g
* \brief Default common attributes of a drx3973d demodulator instance.
*/
DRXCommonAttr_t DRX3973DDefaultCommAttr_g = {
   (pu8_t)NULL,   /* ucode ptr */
   0,             /* ucode size */
   TRUE,          /* ucode verify switch */
   0,             /* IF in kHz in case no tuner instance is used */
   48000L,        /* system clock frequency in kHz */
   24000L,        /* oscillator frequency kHz*/
   0L,            /* oscillator deviation in ppm, signed */
   FALSE,         /* If TRUE mirror frequency spectrum */

   /* MPEG output configuration */
   TRUE,          /* If TRUE, enable MPEG ouput */
   TRUE,          /* If TRUE, insert RS byte */
   TRUE,          /* If TRUE, parallel out otherwise serial */
   FALSE,         /* If TRUE, invert DATA signals */
   FALSE,         /* If TRUE, invert ERR signal */
   FALSE,         /* If TRUE, invert STR signals */
   FALSE,         /* If TRUE, invert VAL signals */
   FALSE,         /* If TRUE, invert CLK signals */

   FALSE,               /* isOpened */

   /* Scan */
   NULL,                /* no scan params yet */
   0,                   /* current scan index */
   0,                   /* next scan frequency */
   FALSE,               /* scan ready flag */
   0L,                  /* max channels to scan */
   0L,                  /* nr of channels scanned */
   0,                   /* millisec to wait for demod lock */
   DRX3973D_DEMOD_LOCK, /* desired lock */
   FALSE,               /* scan routines active or not */

   /* Power management */
   DRX_POWER_DOWN,

   /* Tuner */
   1,                   /* nr of I2C port to wich tuner is */
   0L,                  /* minimum RF input frequency, in kHz */
   0L                   /* maximum RF input frequency, in kHz */
};


/**
* \var DRX3973DDefaultDemod_g
* \brief Default drx3973d demodulator instance.
*/
DRXDemodInstance_t DRX3973DDefaultDemod_g = {
   &DRX3973DFunctions_g,          /* demod functions */
   &DRX3973D_DAP,                 /* data access protocol functions */
   NULL,                          /* tuner instance */
   &DRX3973DDefaultAddr_g,        /* i2c address & device id */
   &DRX3973DDefaultCommAttr_g,    /* demod common attributes */
   &DRX3973DData_g                /* demod device specific attributes */
};

/*-----------------------------------------------------------------------------
STRUCTURES
----------------------------------------------------------------------------*/

typedef struct {
   u16_t cmd;
   u16_t param1;
   u16_t param2;
   u16_t param3;
   u16_t param4;
   u16_t param5;
} DRX3973DHiCmd_t, *pDRX3973DHiCmd_t;

/*-----------------------------------------------------------------------------
FUNCTIONS
----------------------------------------------------------------------------*/
/* Some prototypes */
static DRXStatus_t CtrlI2CBridge( pDRXDemodInstance_t demod,
                                  pBool_t             bridgeClosed, DtDevice *device );

static DRXStatus_t CtrlUIOWrite( pDRXDemodInstance_t demod,
                                 pDRXUIOData_t       UIOData, DtDevice *device);

static DRXStatus_t CtrlLockStatus( pI2CDeviceAddr_t  devAddr,
                                   pDRXLockStatus_t  lockStat, DtDevice *device  );

static DRXStatus_t HI_CfgCommand(const pDRXDemodInstance_t   demod, DtDevice *device);

static DRXStatus_t
CtrlSetCfgIfAgc( pDRXDemodInstance_t   demod, pDRX3973DCfgAgc_t cfg, DtDevice *device );

static DRXStatus_t
CtrlSetCfgRfAgc( pDRXDemodInstance_t   demod, pDRX3973DCfgAgc_t cfg, DtDevice *device );

static DRXStatus_t
HI_Command(const pI2CDeviceAddr_t devAddr,
           const pDRX3973DHiCmd_t cmd,
           pu16_t result, DtDevice *device);

/*=============================================================================
  =============================================================================
  ===== HELPER FUNCTIONS ======================================================
  =============================================================================
  ===========================================================================*/

/*============================================================================*/

/**
* \fn void WaitMilliSec()
* \brief Wait (loop) at least n millisec
* \param n millisec to wait
*
*/
static DRXStatus_t WaitMilliSec(u16_t n)
{
   u32_t start=0;
   u32_t current=0;
   u32_t delta=0;

   start = DRXBSP_HST_Clock();

   do{
      current = DRXBSP_HST_Clock();
      delta = current - start;
   } while( delta < n );

   return (DRX_STS_OK);
}

/*============================================================================*/


/**
* \fn void Mult32(u32_t a, u32_t b, pu32_t h, pu32_t l)
* \brief 32bitsx32bits signed multiplication
* \param a 32 bits multiplicant
* \param b 32 bits multiplier
* \param h pointer to high part 64 bits result
* \param l pointer to low part 64 bits result
*/

static void  Mult32(u32_t a, u32_t b, ps32_t h, pu32_t l)
{

   u8_t i = 0;

   *l =0;
   *h =0;
   for ( i=0 ; i<32 ; i++)
   {
      if ( a & 1)
      {
         *h += b;
      }
      /* shift [h:l] one right */
      (*l) >>= 1;
      if ( (*h) & 1)
      {
         *l |= 0x80000000UL;
      }
      (*h) >>=1;
      /* shift multiplicant one right */
      a >>=1;
   }
}

/*============================================================================*/

static u32_t Frac28(u32_t N, u32_t D)
/*
   This function is used to avoid floating-point calculations as they may
   not be present on the target platform.

   Frac28 performs an unsigned 28/28 bits division to 32-bit fixed point
   fraction used for setting the Frequency Shifter registers.
   N and D can hold numbers up to width: 28-bits.
   The 4 bits integer part and the 28 bits fractional part are calculated.

   Effectually calculates: (1<<28)*N/D

   N: 0...(1<<28)-1 = 268435454
   D: 0...(1<<28)-1
   Q: 0...(1<<32)-1
*/
{
   u8_t   i=0;
   u32_t Q1=0;
   u32_t R0=0;

   R0 = (N%D)<<4; /* 32-28 == 4 shifts possible at max */
   Q1 = N/D;      /* integer part, only the 4 least significant bits
                     will be visible in the result */

   /* division using radix 16, 7 nibbles in the result */
   for (i=0; i<7; i++) {
      Q1 = (Q1 << 4) | R0/D;
      R0 = (R0%D)<<4;
   }
   /* rounding */
   if ((R0>>3) >= D) Q1++;

   return Q1;
}

/*============================================================================*/

/**
* \fn u32_t FracTimes1e6( u16_t N, u32_t D)
* \brief Compute: (N/D) * 1000000.
* \param N nominator 16-bits.
* \param D denominator 32-bits.
* \return u32_t
* \retval ((N/D) * 1000000), 32 bits
*
* No check on D=0!
*/
static u32_t
FracTimes1e6( u16_t N, u32_t D)
{
   u32_t remainder = 0;
   u32_t frac = 0;

   /*
      frac = (N * 1000000) / D
      To let it fit in a 32 bits computation:
      frac = (N * (1000000 >> 4)) / (D >> 4)
      This would result in a problem in case D < 16 (div by 0).
      So we do it more elaborate as shown below.
   */

   frac =  ( ((u32_t)N) * (1000000UL >> 4) ) / D ;
   frac <<=  4 ;
   remainder  =  ( ((u32_t)N) * (1000000UL >> 4) ) % D ;
   remainder <<= 4;
   frac += remainder / D;
   remainder  = remainder % D ;
   if( (remainder * 2) > D )
   {
      frac++;
   }

   return ( frac );
}

/*============================================================================*/

/**
* \fn u32_t Log10Times100( u32_t x)
* \brief Compute: 100*log10(x)
* \param x 32 bits
* \return 100*log10(x)
*
* 100*log10(x)
* = 100*(log2(x)/log2(10)))
* = (100*(2^15)*log2(x))/((2^15)*log2(10))
* = ((200*(2^15)*log2(x))/((2^15)*log2(10)))/2
* = ((200*(2^15)*(log2(x/y)+log2(y)))/((2^15)*log2(10)))/2
* = ((200*(2^15)*log2(x/y))+(200*(2^15)*log2(y)))/((2^15)*log2(10)))/2
*
* where y = 2^k and 1<= (x/y) < 2
*/

u32_t Log10Times100( u32_t x)
{
   static const u8_t scale=15;
   static const u8_t indexWidth=5;
   /*
   log2lut[n] = (1<<scale) * 200 * log2( 1.0 + ( (1.0/(1<<INDEXWIDTH)) * n ))
   0 <= n < ((1<<INDEXWIDTH)+1)
   */

   static const u32_t log2lut[] = {
      0, /* 0.000000 */
      290941, /* 290941.300628 */
      573196, /* 573196.476418 */
      847269, /* 847269.179851 */
      1113620, /* 1113620.489452 */
      1372674, /* 1372673.576986 */
      1624818, /* 1624817.752104 */
      1870412, /* 1870411.981536 */
      2109788, /* 2109787.962654 */
      2343253, /* 2343252.817465 */
      2571091, /* 2571091.461923 */
      2793569, /* 2793568.696416 */
      3010931, /* 3010931.055901 */
      3223408, /* 3223408.452106 */
      3431216, /* 3431215.635215 */
      3634553, /* 3634553.498355 */
      3833610, /* 3833610.244726 */
      4028562, /* 4028562.434393 */
      4219576, /* 4219575.925308 */
      4406807, /* 4406806.721144 */
      4590402, /* 4590401.736809 */
      4770499, /* 4770499.491025 */
      4947231, /* 4947230.734179 */
      5120719, /* 5120719.018555 */
      5291081, /* 5291081.217197 */
      5458428, /* 5458427.996830 */
      5622864, /* 5622864.249668 */
      5784489, /* 5784489.488298 */
      5943398, /* 5943398.207380 */
      6099680, /* 6099680.215452 */
      6253421, /* 6253420.939751 */
      6404702, /* 6404701.706649 */
      6553600, /* 6553600.000000 */
   };


   u8_t  i = 0;
   u32_t y = 0;
   u32_t d = 0;
   u32_t k = 0;
   u32_t r = 0;

   if (x==0) return (0);

   /* Scale x (normalize) */
   /* computing y in log(x/y) = log(x) - log(y) */
   if ( (x & (((u32_t)(-1))<<(scale+1)) ) == 0 )
   {
      for (k = scale; k>0 ; k--)
      {
        if (x & ((1UL)<<scale)) break;
        x <<= 1;
      }
   } else {
      for (k = scale; k<31 ; k++)
      {
        if ((x & (((u32_t)(-1))<<(scale+1)))==0) break;
        x >>= 1;
      }
   }
   /*
     Now x has binary point between bit[scale] and bit[scale-1]
     and 1.0 <= x < 2.0 */

   /* correction for divison: log(x) = log(x/y)+log(y) */
   y = k * ((1UL << scale) * 200);

   /* remove integer part */
   x &= ((1UL << scale)-1);
   /* get index */
   i = (u8_t) (x >> (scale -indexWidth));
   /* compute delta (x-a) */
   d = x & ((1UL << (scale-indexWidth))-1);
   /* compute log, multiplication ( d* (.. )) must be within range ! */
   y += log2lut[i] + (( d*( log2lut[i+1]-log2lut[i] ))>>(scale-indexWidth));
   /* Conver to log10() */
   y /= 108853; /* (log2(10) << scale) */
   r = (y>>1UL);
   /* rounding */
   if (y&1UL) r++;

   return (r);

}

/*=============================================================================
  ===== Atomic data access related stuff ======================================
  ===========================================================================*/

#define HI_TR_FUNC_ADDR HI_IF_RAM_USR_BEGIN__A

static
DRXStatus_t InitAtomicRead ( pI2CDeviceAddr_t devAddr, DtDevice *device )
{
   static u8_t instructions[] =
   {
     0x26, 0x00,  /* 0         -> ring.rdy;           */
     0x60, 0x04,  /* r0rami.dt -> ring.xba;           */
     0x61, 0x04,  /* r0rami.dt -> ring.xad;           */
     0xE3, 0x07,  /* HI_RA_RAM_USR_BEGIN -> ring.iad; */
     0x40, 0x00,  /* (long immediate)                 */
     0x64, 0x04,  /* r0rami.dt -> ring.len;           */
     0x65, 0x04,  /* r0rami.dt -> ring.ctl;           */
     0x26, 0x00,  /* 0         -> ring.rdy;           */
     0x38, 0x00   /* 0         -> jumps.ad;           */
   };

   WRBLOCK( devAddr, HI_TR_FUNC_ADDR , sizeof(instructions), instructions, device);

   return DRX_STS_OK;

 rw_error:
   return (DRX_STS_ERROR);
}

/**
* \fn DRXStatus_t AtomicReadBlock()
* \brief Atomic read of n bytes
*
* Flags are ignored for now ...
*/

#define HI_TR_WRITE      0x9
#define HI_TR_READ       0xA
#define HI_TR_READ_WRITE 0xB
#define HI_TR_BROADCAST  0x4

static
DRXStatus_t AtomicReadBlock (
    pI2CDeviceAddr_t devAddr,
    DRXaddr_t        addr,
    u16_t            datasize,
    pu8_t            data,
    DRXflags_t       flags,
    DtDevice         *device)
{
   DRX3973DHiCmd_t hiCmd;

   u16_t dummy=0;
   u16_t i=0;

   /* Parameter check */
   if ( ( data == NULL ) ||
        ( devAddr == NULL ) ||
        ( (datasize%2)!=0 )
      )
   {
      return (DRX_STS_INVALID_ARG);
   }

   /* Instruct HI to read n bytes */
   hiCmd.cmd    = HI_RA_RAM_SRV_CMD_EXECUTE;
   hiCmd.param1 = (u16_t) (HI_TR_FUNC_ADDR & 0xFFFF);
   hiCmd.param2 = (u16_t)(addr >> 16);
   hiCmd.param3 = (u16_t)(addr & 0xFFFF);
   hiCmd.param4 = (u16_t)((datasize/2) - 1);
   hiCmd.param5 = HI_TR_READ;

   CHK_ERROR( HI_Command( devAddr, &hiCmd, &dummy, device) );

   for (i = 0; i < (datasize/2); i++)
   {
      u16_t word;

      RR16 (devAddr, (HI_RA_RAM_USR_BEGIN__A + i), &word, 0, device);
      data[2*i]       = (u8_t) (word & 0xFF);
      data[(2*i) + 1] = (u8_t) (word >> 8 );
   }

   return DRX_STS_OK;

 rw_error:
   return (DRX_STS_ERROR);
}

/*============================================================================*/

/**
* \fn DRXStatus_t AtomicReadReg32()
* \brief Atomic read of 32 bits words
*/

static
DRXStatus_t AtomicReadReg32 (
    pI2CDeviceAddr_t devAddr,
    DRXaddr_t        addr,
    pu32_t           data,
    DRXflags_t       flags,
    DtDevice         *device)
{
    u8_t buf[sizeof (*data)];
    DRXStatus_t rc = DRX_STS_ERROR;
    u32_t word = 0;

    if (!data)
    {
        return DRX_STS_INVALID_ARG;
    }

    rc = AtomicReadBlock (devAddr, addr, sizeof (*data), buf, flags, device);

    word = (u32_t)buf[3];
    word <<= 8;
    word |= (u32_t)buf[2];
    word <<= 8;
    word |= (u32_t)buf[1];
    word <<= 8;
    word |= (u32_t)buf[0];

    *data = word;

    return rc;
}

/*============================================================================*/

#if 0
/**
* \fn DRXStatus_t TunetI2CWriteRead()
* \brief I2C communication for tuner via micro
*/
DRXStatus_t TunerI2CWriteRead( pTUNERInstance_t   tuner,
                                      pI2CDeviceAddr_t wDevAddr,
                                      u16_t            wCount,
                                      pu8_t            wData,
                                      pI2CDeviceAddr_t rDevAddr,
                                      u16_t            rCount,
                                      pu8_t            rData)
{
   pDRXDemodInstance_t demod;
   DRXI2CData_t i2cData = { 1, wDevAddr, wCount, wData, rDevAddr, rCount, rData };

   demod = (pDRXDemodInstance_t) (tuner->myCommonAttr->myUserData);

   return ( DRX_Ctrl( demod, DRX_CTRL_I2C_READWRITE, &i2cData ) );
}
#endif


/*=============================================================================
  ===== Reset related funtions ================================================
  ===========================================================================*/

/**
* \fn DRXStatus_t StopAllProcessors( const pI2CDeviceAddr_t devAddr )
* \brief Stop all processors except the HI.
* \param devAddr pointer to device address.
* \return DRXStatus_t Return status.
*/
static DRXStatus_t
StopAllProcessors( const pI2CDeviceAddr_t devAddr, DtDevice *device )
{

   /* Do not stop HI controller!
      Otherwise there will be no communication with the device */
   /* Do inverse broadcast, write to all blocks except the HI block
                                                       and CC block */
   BCWR16( devAddr, HI_COMM_EXEC__A, SC_COMM_EXEC_CTL_STOP, device );

   return (DRX_STS_OK);

 rw_error:
   return (DRX_STS_ERROR);
}

/*============================================================================*/

/**
* \fn DRXStatus_t EnableAndResetMB( const pI2CDeviceAddr_t devAddr )
* \brief Enable and reset monitor bus.
* \param devAddr pointer to device address.
* \return DRXStatus_t Return status.
*/
static DRXStatus_t
EnableAndResetMB( const pI2CDeviceAddr_t devAddr, DtDevice *device )
{
#if (DRXD_TYPE_A)
   /* disable? monitor bus observe @ EC_OC */
   WR16( devAddr, EC_OC_REG_OC_MON_SIO__A, 0x0000, 0x0000, device);
#endif

   /* Do inverse broadcast, followed by explicit write to HI */
   BCWR16( devAddr, HI_COMM_MB__A, 0x0000, device );
   WR16( devAddr, HI_COMM_MB__A, 0x0000, 0x0000, device);

   return (DRX_STS_OK);

 rw_error:
   return (DRX_STS_ERROR);
}

/*============================================================================*/

/**
* \fn DRXStatus_t ResetCE_FR( const pI2CDeviceAddr_t devAddr )
* \brief Reset CE FR.
* \param devAddr pointer to device address.
* \return DRXStatus_t Return status.
* \retval DRX_STS_OK Success.
* \retval DRX_STS_ERROR Failure.
*
* Due to bug in HW the default values of these registers have
* to be programmed by the host.
*
*/
#if (DRXD_TYPE_A)

static DRXStatus_t
ResetCEFR( const pI2CDeviceAddr_t devAddr, DtDevice *device )
{

   static u8_t resetData[] =
   {  0x52,0x00, /* CE_REG_FR_TREAL00__A */
      0x00,0x00, /* CE_REG_FR_TIMAG00__A */
      0x52,0x00, /* CE_REG_FR_TREAL01__A */
      0x00,0x00, /* CE_REG_FR_TIMAG01__A */
      0x52,0x00, /* CE_REG_FR_TREAL02__A */
      0x00,0x00, /* CE_REG_FR_TIMAG02__A */
      0x52,0x00, /* CE_REG_FR_TREAL03__A */
      0x00,0x00, /* CE_REG_FR_TIMAG03__A */
      0x52,0x00, /* CE_REG_FR_TREAL04__A */
      0x00,0x00, /* CE_REG_FR_TIMAG04__A */
      0x52,0x00, /* CE_REG_FR_TREAL05__A */
      0x00,0x00, /* CE_REG_FR_TIMAG05__A */
      0x52,0x00, /* CE_REG_FR_TREAL06__A */
      0x00,0x00, /* CE_REG_FR_TIMAG06__A */
      0x52,0x00, /* CE_REG_FR_TREAL07__A */
      0x00,0x00, /* CE_REG_FR_TIMAG07__A */
      0x52,0x00, /* CE_REG_FR_TREAL08__A */
      0x00,0x00, /* CE_REG_FR_TIMAG08__A */
      0x52,0x00, /* CE_REG_FR_TREAL09__A */
      0x00,0x00, /* CE_REG_FR_TIMAG09__A */
      0x52,0x00, /* CE_REG_FR_TREAL10__A */
      0x00,0x00, /* CE_REG_FR_TIMAG10__A */
      0x52,0x00, /* CE_REG_FR_TREAL11__A */
      0x00,0x00, /* CE_REG_FR_TIMAG11__A */

      0x52,0x00, /* CE_REG_FR_MID_TAP__A */

      0x0B,0x00, /* CE_REG_FR_SQS_G00__A */
      0x0B,0x00, /* CE_REG_FR_SQS_G01__A */
      0x0B,0x00, /* CE_REG_FR_SQS_G02__A */
      0x0B,0x00, /* CE_REG_FR_SQS_G03__A */
      0x0B,0x00, /* CE_REG_FR_SQS_G04__A */
      0x0B,0x00, /* CE_REG_FR_SQS_G05__A */
      0x0B,0x00, /* CE_REG_FR_SQS_G06__A */
      0x0B,0x00, /* CE_REG_FR_SQS_G07__A */
      0x0B,0x00, /* CE_REG_FR_SQS_G08__A */
      0x0B,0x00, /* CE_REG_FR_SQS_G09__A */
      0x0B,0x00, /* CE_REG_FR_SQS_G10__A */
      0x0B,0x00, /* CE_REG_FR_SQS_G11__A */
      0x0B,0x00, /* CE_REG_FR_SQS_G12__A */

      0xFF,0x01, /* CE_REG_FR_RIO_G00__A */
      0x90,0x01, /* CE_REG_FR_RIO_G01__A */
      0x14,0x01, /* CE_REG_FR_RIO_G02__A */
      0xC8,0x00, /* CE_REG_FR_RIO_G03__A */
      0xA0,0x00, /* CE_REG_FR_RIO_G04__A */
      0x85,0x00, /* CE_REG_FR_RIO_G05__A */
      0x72,0x00, /* CE_REG_FR_RIO_G06__A */
      0x64,0x00, /* CE_REG_FR_RIO_G07__A */
      0x59,0x00, /* CE_REG_FR_RIO_G08__A */
      0x50,0x00, /* CE_REG_FR_RIO_G09__A */
      0x49,0x00, /* CE_REG_FR_RIO_G10__A */

      0x10,0x00, /* CE_REG_FR_MODE__A     */
      0x78,0x00, /* CE_REG_FR_SQS_TRH__A  */
      0x00,0x00, /* CE_REG_FR_RIO_GAIN__A */
      0x00,0x02, /* CE_REG_FR_BYPASS__A   */
      0x0D,0x00, /* CE_REG_FR_PM_SET__A   */
      0x07,0x00, /* CE_REG_FR_ERR_SH__A   */
      0x04,0x00, /* CE_REG_FR_MAN_SH__A   */
      0x06,0x00  /* CE_REG_FR_TAP_SH__A   */
   };

   WRBLOCK( devAddr, CE_REG_FR_TREAL00__A, sizeof(resetData), resetData, device );


   return (DRX_STS_OK);

 rw_error:
   return (DRX_STS_ERROR);
}

#endif /* DRXD_TYPE_A */

/*============================================================================*/

/**
* \fn DRXStatus_t InitCC( const pDRXDemodInstance_t demod )
* \brief Initialize clock controler
* \param demod pointer to demod data.
* \return DRXStatus_t Return status.
* \retval DRX_STS_OK Success.
* \retval DRX_STS_ERROR Failure.
*
* Two pads are available for clock input.
* Only one pad is bounded to XI and XO pins.
* Both pads can form, together with an external Xtal, an Xtal oscillator.
* One pad can do this with an Xtal from 4 upto 20 MHz. This is prefered.
* Another pad can do this with an Xtal of 48 Mhz. This needs different bounding
* and a metal change. Only needed if to other solution fails.
* Both pads can accept a signal from an external oscillator from at least 4 MHz
* upto 48 MHz.
*/
static DRXStatus_t
InitCC( const pDRXDemodInstance_t demod, DtDevice *device )
{
   const DRXFrequency_t divider_base = 4000;
   u16_t divider = 0;
#ifndef COMPILE_FOR_QT
   pI2CDeviceAddr_t devAddr = demod -> myI2CDevAddr;
#endif
   DRXFrequency_t oscFreq = demod->myCommonAttr->oscClockFreq;
   pDRX3973DData_t extAttr= (pDRX3973DData_t) demod -> myExtAttr;

   /* compute clock divider */
   divider = (u16_t) (oscFreq/divider_base);

   /* handle non 4-fold clocks, asymetric tolerance range:
      approx. -1.4 Mhz ... +3.2Mhz */
   if ( oscFreq%divider_base > 2600 )
   {
      divider++;
   }
   if ( divider > CC_REG_REF_DIVIDE_D10 )
   {
      if ( oscFreq%divider_base > 3200 )
      {
         /* out of range */
         return (DRX_STS_ERROR);
      } else {
         /* still within upper tolerance range */
         divider--;
      }
   }

   (extAttr->expectedSysClockFreq) = oscFreq*12/divider;
   /* rounding */
   if ( (2*((oscFreq*12)%divider)) > divider )
   {
      (extAttr->expectedSysClockFreq)++;
   }

   if(oscFreq == 48000)
   {
      (extAttr->expectedSysClockFreq) = oscFreq;
   }

#ifndef COMPILE_FOR_QT
   /* 4-20 MHz pad */
   WR16( devAddr, CC_REG_OSC_MODE__A, CC_REG_OSC_MODE_M20, 0x0000, device);

   if ( oscFreq != 48000 )
   {
      /* active PLL, pump=1.2, outen = 0 */
      WR16( devAddr, CC_REG_PLL_MODE__A, ( CC_REG_PLL_MODE_BYPASS_PLL |
                                        CC_REG_PLL_MODE_PUMP_CUR_12 ) , 0, device);
      /* clock divider */
      WR16( devAddr, CC_REG_REF_DIVIDE__A, divider, 0, device);
   }
   else
   {
      /* 4-20 MHz pad, PLL bypassed */
      WR16( devAddr, CC_REG_PLL_MODE__A, 0x2 , 0, device);
   }

   /* when power down , clock & PLL down, osc up  */
   /* trigger CC to take over the new settings */
   WR16( devAddr, CC_REG_UPDATE__A, CC_REG_UPDATE_KEY , 0, device);
#endif

   return (DRX_STS_OK);

#ifndef COMPILE_FOR_QT
 rw_error:
   return (DRX_STS_ERROR);
#endif

}

/*============================================================================*/

/**
* \fn DRXStatus_t InitFE( const pDRXDemodInstance_t demod )
* \brief
* \param demod pointer to demod data.
* \return DRXStatus_t Return status.
*\retval DRX_STS_OK Success.
*\retval DRX_STS_ERROR Failure.
*/
static DRXStatus_t
InitFE( const pDRXDemodInstance_t demod, DtDevice *device )
{
   pI2CDeviceAddr_t  devAddr=NULL;
   pDRX3973DData_t   extAttr=NULL;

#if (DRXD_TYPE_A)

   static u8_t resetData1[] =
   {  0x00,0x00, /* FE_AD_REG_PD__A          */
      0x01,0x00, /* FE_AD_REG_INVEXT__A      */
      0x00,0x00  /* FE_AD_REG_CLKNEG__A      */
   };

   static u8_t resetData2[] =
   {  0x10,0x00, /* FE_AG_REG_DCE_AUR_CNT__A */
      0x10,0x00  /* FE_AG_REG_DCE_RUR_CNT__A */
   };

   static u8_t resetData3[] =
   {  0x0E,0x00, /* FE_AG_REG_ACE_AUR_CNT__A */
      0x00,0x00  /* FE_AG_REG_ACE_RUR_CNT__A */
   };

   static u8_t resetData4[] =
   {  0x04,0x00, /* FE_AG_REG_EGC_FLA_RGN__A */
      0x1F,0x00, /* FE_AG_REG_EGC_SLO_RGN__A */
      0x00,0x00, /* FE_AG_REG_EGC_JMP_PSN__A */
      0x00,0x00, /* FE_AG_REG_EGC_FLA_INC__A */
      0x00,0x00  /* FE_AG_REG_EGC_FLA_DEC__A */
   };

   static u8_t resetData5[] =
   {  0xFF,0x01, /* FE_AG_REG_GC1_AGC_MAX__A */
      0x00,0xFE  /* FE_AG_REG_GC1_AGC_MIN__A */
   };

   static u8_t resetData6[] =
   {  0x00,0x00, /* FE_AG_REG_IND_WIN__A     */
      0x05,0x00, /* FE_AG_REG_IND_THD_LOL__A */
      0x0F,0x00, /* FE_AG_REG_IND_THD_HIL__A */
      0x00,0x00, /* FE_AG_REG_IND_DEL__A     don't care */
      0x1E,0x00, /* FE_AG_REG_IND_PD1_WRI__A */
      0x09,0x00, /* FE_AG_REG_PDA_AUR_CNT__A */
      0x00,0x00, /* FE_AG_REG_PDA_RUR_CNT__A */
      0x00,0x00, /* FE_AG_REG_PDA_AVE_DAT__A don't care  */
      0x00,0x00, /* FE_AG_REG_PDC_RUR_CNT__A */
      0x10,0x00, /* FE_AG_REG_PDC_SET_LVL__A */
      0x02,0x00, /* FE_AG_REG_PDC_FLA_RGN__A */
      0x00,0x00, /* FE_AG_REG_PDC_JMP_PSN__A don't care  */
      0xFF,0xFF, /* FE_AG_REG_PDC_FLA_STP__A */
      0xFF,0xFF, /* FE_AG_REG_PDC_SLO_STP__A */
      0x00,0x1F, /* FE_AG_REG_PDC_PD2_WRI__A don't care  */
      0x00,0x00, /* FE_AG_REG_PDC_MAP_DAT__A don't care  */
      0x02,0x00, /* FE_AG_REG_PDC_MAX__A     */
      0x0C,0x00, /* FE_AG_REG_TGA_AUR_CNT__A */
      0x00,0x00, /* FE_AG_REG_TGA_RUR_CNT__A */
      0x00,0x00, /* FE_AG_REG_TGA_AVE_DAT__A don't care  */
      0x00,0x00, /* FE_AG_REG_TGC_RUR_CNT__A */
      0x19,0x00, /* FE_AG_REG_TGC_SET_LVL__A */
      0x15,0x00, /* FE_AG_REG_TGC_FLA_RGN__A */
      0x00,0x00, /* FE_AG_REG_TGC_JMP_PSN__A don't care  */
      0x01,0x00, /* FE_AG_REG_TGC_FLA_STP__A */
      0x0A,0x00, /* FE_AG_REG_TGC_SLO_STP__A */
      0x00,0x00, /* FE_AG_REG_TGC_MAP_DAT__A don't care  */
      0x10,0x00, /* FE_AG_REG_FGA_AUR_CNT__A */
      0x10,0x00, /* FE_AG_REG_FGA_RUR_CNT__A */
   };

   static u8_t resetData7[] =
   {  0x00,0x00, /* FE_AG_REG_BGC_FGC_WRI__A */
      0x00,0x00  /* FE_AG_REG_BGC_CGC_WRI__A */
   };

   static u8_t resetData8[] =
   {  0x05,0x00, /* FE_FD_REG_SCL__A         */
      0x03,0x00, /* FE_FD_REG_MAX_LEV__A     */
      0x05,0x00  /* FE_FD_REG_NR__A          */
   };

   static u8_t resetData9[] =
   {  0x16,0x00, /* FE_CF_REG_SCL__A         */
      0x04,0x00, /* FE_CF_REG_MAX_LEV__A     */
      0x06,0x00, /* FE_CF_REG_NR__A          */
      0x00,0x00, /* FE_CF_REG_IMP_VAL__A     */
      0x01,0x00  /* FE_CF_REG_MEAS_VAL__A    */
   };

   static u8_t resetData10[] =
   {  0x00,0x08, /* FE_CU_REG_FRM_CNT_RST__A */
      0x00,0x00  /* FE_CU_REG_FRM_CNT_STR__A */
   };


   devAddr = demod -> myI2CDevAddr;
   extAttr = (pDRX3973DData_t) demod -> myExtAttr;

   WRBLOCK( devAddr, FE_AD_REG_PD__A            , sizeof(resetData1), resetData1, device );
   WRBLOCK( devAddr, FE_AG_REG_DCE_AUR_CNT__A   , sizeof(resetData2), resetData2, device );
   WRBLOCK( devAddr, FE_AG_REG_ACE_AUR_CNT__A   , sizeof(resetData3), resetData3, device );
   WRBLOCK( devAddr, FE_AG_REG_EGC_FLA_RGN__A   , sizeof(resetData4), resetData4, device );
   WRBLOCK( devAddr, FE_AG_REG_GC1_AGC_MAX__A   , sizeof(resetData5), resetData5, device );
   WRBLOCK( devAddr, FE_AG_REG_IND_WIN__A       , sizeof(resetData6), resetData6, device );
   WRBLOCK( devAddr, FE_AG_REG_BGC_FGC_WRI__A   , sizeof(resetData7), resetData7, device );
   WRBLOCK( devAddr, FE_FD_REG_SCL__A           , sizeof(resetData8), resetData8, device );
   WRBLOCK( devAddr, FE_CF_REG_SCL__A           , sizeof(resetData9), resetData9, device );
   WRBLOCK( devAddr, FE_CU_REG_FRM_CNT_RST__A   , sizeof(resetData10), resetData10, device );


   /* with or without PGA  */
   if ( ( demod->myDemodFunct->typeId == DRX3973D_TYPE_ID ) ||
        ( demod->myDemodFunct->typeId == DRX3974D_TYPE_ID ) ||
        ( demod->myDemodFunct->typeId == DRX3977D_TYPE_ID ) )
   {
      /* with PGA */
      WR16( devAddr, FE_AG_REG_AG_PGA_MODE__A   , 0x0004, 0x0000, device);
   } else {
      /* withou PGA */
      WR16( devAddr, FE_AG_REG_AG_PGA_MODE__A   , 0x0001, 0x0000, device);
   }
   WR16( devAddr, FE_AG_REG_AG_AGC_SIO__A,  (extAttr -> FeAgRegAgAgcSio), 0x0000, device );
   WR16( devAddr, FE_AG_REG_AG_PWD__A        ,(extAttr -> FeAgRegAgPwd), 0x0000, device );
   WR16( devAddr, FE_AG_REG_CDR_RUR_CNT__A, 0x0010, 0x0000, device );
   WR16( devAddr, FE_AG_REG_FGM_WRI__A    ,     48, 0x0000, device );
   /* Activate measurement, activate scale */
   WR16( devAddr, FE_FD_REG_MEAS_VAL__A , 0x0001, 0x0000, device );

   WR16( devAddr, FE_CU_REG_COMM_EXEC__A, 0x0001, 0x0000, device );
   WR16( devAddr, FE_CF_REG_COMM_EXEC__A, 0x0001, 0x0000, device );
   WR16( devAddr, FE_IF_REG_COMM_EXEC__A, 0x0001, 0x0000, device );
   WR16( devAddr, FE_FD_REG_COMM_EXEC__A, 0x0001, 0x0000, device );
   WR16( devAddr, FE_FS_REG_COMM_EXEC__A, 0x0001, 0x0000, device );
   WR16( devAddr, FE_AD_REG_COMM_EXEC__A     , 0x0001, 0x0000, device);
   WR16( devAddr, FE_AG_REG_COMM_EXEC__A     , 0x0001, 0x0000, device);
   WR16( devAddr, FE_AG_REG_AG_MODE_LOP__A   , 0x895E, 0x0000, device);

#endif /* DRXD_TYPE_A */

#if (DRXD_TYPE_B)

   devAddr = demod -> myI2CDevAddr;
   extAttr = (pDRX3973DData_t) demod -> myExtAttr;

   WR16( devAddr, FE_AD_REG_PD__A            ,0x0000 , 0x0000, device );
   WR16( devAddr, FE_AD_REG_CLKNEG__A        ,0x0000 , 0x0000, device );
   WR16( devAddr, FE_AG_REG_AG_PWD__A        ,0x0000 , 0x0000, device );
   WR16( devAddr, FE_AG_REG_BGC_FGC_WRI__A   ,0x0000 , 0x0000, device );
   WR16( devAddr, FE_AG_REG_BGC_CGC_WRI__A   ,0x0000 , 0x0000, device );
   WR16( devAddr, FE_AG_REG_AG_MODE_LOP__A   ,0x000a , 0x0000, device );
   WR16( devAddr, FE_AG_REG_IND_PD1_WRI__A   ,35     , 0x0000, device );
   WR16( devAddr, FE_AG_REG_IND_WIN__A       ,0      , 0x0000, device );
   WR16( devAddr, FE_AG_REG_IND_THD_LOL__A   ,8      , 0x0000, device );
   WR16( devAddr, FE_AG_REG_IND_THD_HIL__A   ,8      , 0x0000, device );
   WR16( devAddr, FE_CF_REG_IMP_VAL__A       ,0      , 0x0000, device );
   /* with or without PGA  */
   if ( ( demod->myDemodFunct->typeId == DRX3973D_TYPE_ID ) ||
        ( demod->myDemodFunct->typeId == DRX3974D_TYPE_ID ) ||
        ( demod->myDemodFunct->typeId == DRX3977D_TYPE_ID ) )
   {
      /* with PGA */
      WR16( devAddr, FE_AG_REG_AG_PGA_MODE__A   , 0x0000, 0x0000, device);
   } else {
      /* withou PGA */
      WR16( devAddr, FE_AG_REG_AG_PGA_MODE__A   , 0x0001, 0x0000, device);
   }
   WR16( devAddr, FE_AG_REG_AG_AGC_SIO__A,(extAttr -> FeAgRegAgAgcSio), 0x0000, device );/*added HS 23-05-2005*/
   WR16( devAddr, FE_AG_REG_AG_PWD__A    ,(extAttr -> FeAgRegAgPwd), 0x0000, device );
   WR16( devAddr, FE_COMM_EXEC__A        ,0x0001 , 0x0000 );

   /* RF-AGC setup */
   WR16( devAddr, FE_AG_REG_PDA_AUR_CNT__A , 0x9,    0x0000, device );
   WR16( devAddr, FE_AG_REG_PDC_FLA_RGN__A , 0x2,    0x0000, device );
   WR16( devAddr, FE_AG_REG_PDC_FLA_STP__A , 0xFFFF, 0x0000, device );
   WR16( devAddr, FE_AG_REG_PDC_SLO_STP__A , 0xFFFF, 0x0000, device );
   WR16( devAddr, FE_AG_REG_PDC_MAX__A     , 0x2,    0x0000, device );
   WR16( devAddr, FE_AG_REG_TGA_AUR_CNT__A , 0xC,    0x0000, device );
   WR16( devAddr, FE_AG_REG_TGC_SET_LVL__A , 0x17,   0x0000, device );
   WR16( devAddr, FE_AG_REG_TGC_FLA_RGN__A , 0xA,    0x0000, device );
   WR16( devAddr, FE_AG_REG_TGC_FLA_STP__A , 0x1,    0x0000, device );
   WR16( devAddr, FE_AG_REG_TGC_SLO_STP__A , 0x5,    0x0000, device );

#endif  /* DRXD_TYPE_B */

   return (DRX_STS_OK);

 rw_error:
   return (DRX_STS_ERROR);
}


/*============================================================================*/

/**
* \fn DRXStatus_t InitFT( const pDRXDemodInstance_t demod )
* \brief Initilize FT
* \param demod pointer to demod data.
* \return DRXStatus_t Return status.
*\retval DRX_STS_OK Success.
*\retval DRX_STS_ERROR Failure.
*/
static DRXStatus_t
InitFT( const pDRXDemodInstance_t demod, DtDevice *device )
{
   pI2CDeviceAddr_t devAddr=NULL;
   devAddr = demod -> myI2CDevAddr;

   WR16( devAddr, FT_REG_COMM_EXEC__A, 0x0001, 0x0000, device );

   return (DRX_STS_OK);

 rw_error:
   return (DRX_STS_ERROR);
}


/*============================================================================*/

/**
* \fn DRXStatus_t InitCP( const pDRXDemodInstance_t demod )
* \brief Initialize CP
* \param demod pointer to demod data.
* \return DRXStatus_t Return status.
*\retval DRX_STS_OK Success.
*\retval DRX_STS_ERROR Failure.
*/
static DRXStatus_t
InitCP( const pDRXDemodInstance_t demod, DtDevice *device )
{
   pI2CDeviceAddr_t devAddr=NULL;

#if (DRXD_TYPE_A)

   static u8_t resetData1[] =
   {  0x07,0x00, /* CP_REG_BR_SPL_OFFSET__A  */
      0x0A,0x00  /* CP_REG_BR_STR_DEL__A     */
   };

   static u8_t resetData2[] =
   {  0x00,0x00, /* CP_REG_RT_ANG_INC0__A    */
      0x00,0x00, /* CP_REG_RT_ANG_INC1__A    */
      0x03,0x00, /* CP_REG_RT_DETECT_ENA__A  */
      0x03,0x00  /* CP_REG_RT_DETECT_TRH__A  */
   };

   static u8_t resetData3[] =
   {  0x32,0x00, /* CP_REG_AC_NEXP_OFFS__A   */
      0x62,0x00, /* CP_REG_AC_AVER_POW__A    */
      0x82,0x00, /* CP_REG_AC_MAX_POW__A     */
      0x26,0x00, /* CP_REG_AC_WEIGHT_MAN__A  */
      0x0F,0x00  /* CP_REG_AC_WEIGHT_EXP__A  */
   };

   static u8_t resetData4[] =
   {  0x02,0x00, /* CP_REG_AC_AMP_MODE__A    */
      0x01,0x00  /* CP_REG_AC_AMP_FIX__A     */
   };

   devAddr = demod -> myI2CDevAddr;

   WRBLOCK( devAddr, CP_REG_BR_SPL_OFFSET__A , sizeof(resetData1), resetData1, device );
   WRBLOCK( devAddr, CP_REG_RT_ANG_INC0__A   , sizeof(resetData2), resetData2, device );
   WRBLOCK( devAddr, CP_REG_AC_NEXP_OFFS__A  , sizeof(resetData3), resetData3, device );
   WRBLOCK( devAddr, CP_REG_AC_AMP_MODE__A   , sizeof(resetData4), resetData4, device );

   WR16( devAddr, CP_REG_INTERVAL__A     , 0x0005, 0x0000, device);
   WR16( devAddr, CP_REG_RT_EXP_MARG__A      , 0x0004, 0x0000, device);
   WR16( devAddr, CP_REG_AC_ANG_MODE__A      , 0x0003, 0x0000, device);

   WR16( devAddr, CP_REG_COMM_EXEC__A        , 0x0001, 0x0000, device);

#endif /* DRXD_TYPE_A */

#if (DRXD_TYPE_B)

   devAddr = demod -> myI2CDevAddr;

   WR16( devAddr, CP_REG_BR_SPL_OFFSET__A    ,0x0008  ,0x0000, device);
   WR16( devAddr, CP_COMM_EXEC__A            ,0x0001  ,0x0000, device);

#endif   /* DRXD_TYPE_B */

   return (DRX_STS_OK);

 rw_error:
   return (DRX_STS_ERROR);
}

/*============================================================================*/

/**
* \fn DRXStatus_t InitCE( const pDRXDemodInstance_t demod )
* \brief Inmitialize CE.
* \param demod pointer to demod data.
* \return DRXStatus_t Return status.
*\retval DRX_STS_OK Success.
*\retval DRX_STS_ERROR Failure.
*/
static DRXStatus_t
InitCE( const pDRXDemodInstance_t demod, DtDevice *device )
{
   pI2CDeviceAddr_t devAddr=NULL;

#if (DRXD_TYPE_A)

   static u8_t resetData1[] =
   {  0x62,0x00, /* CE_REG_AVG_POW__A        */
      0x78,0x00, /* CE_REG_MAX_POW__A        */
      0x62,0x00, /* CE_REG_ATT__A            */
      0x17,0x00  /* CE_REG_NRED__A           */
   };

   static u8_t resetData2[] =
   {  0x07,0x00, /* CE_REG_NE_ERR_SELECT__A  */
      0xEB,0xFF  /* CE_REG_NE_TD_CAL__A      */
   };

   static u8_t resetData3[] =
   {  0x06,0x00, /* CE_REG_NE_MIXAVG__A      */
      0x00,0x00  /* CE_REG_NE_NUPD_OFS__A    */
   };

   static u8_t resetData4[] =
   {  0x00,0x00, /* CE_REG_PE_NEXP_OFFS__A   */
      0x00,0x00  /* CE_REG_PE_TIMESHIFT__A   */
   };

   static u8_t resetData5[] =
   {  0x00,0x01, /* CE_REG_TP_A0_TAP_NEW__A       */
      0x01,0x00, /* CE_REG_TP_A0_TAP_NEW_VALID__A */
      0x0E,0x00  /* CE_REG_TP_A0_MU_LMS_STEP__A   */
   };

   static u8_t resetData6[] =
   {  0x00,0x00, /* CE_REG_TP_A1_TAP_NEW__A        */
      0x01,0x00, /* CE_REG_TP_A1_TAP_NEW_VALID__A  */
      0x0A,0x00  /* CE_REG_TP_A1_MU_LMS_STEP__A    */
   };

   static u8_t resetData7[] =
   {  0x12,0x00, /* CE_REG_FI_SHT_INCR__A          */
      0x0C,0x00  /* CE_REG_FI_EXP_NORM__A          */
   };

   static u8_t resetData8[] =
   {  0x00,0x00, /* CE_REG_IR_INPUTSEL__A          */
      0x00,0x00, /* CE_REG_IR_STARTPOS__A          */
      0xFF,0x00  /* CE_REG_IR_NEXP_THRES__A        */
   };


   devAddr = demod -> myI2CDevAddr;

   WRBLOCK( devAddr, CE_REG_AVG_POW__A          , sizeof(resetData1), resetData1 , device);
   WRBLOCK( devAddr, CE_REG_NE_ERR_SELECT__A    , sizeof(resetData2), resetData2 , device);
   WRBLOCK( devAddr, CE_REG_NE_MIXAVG__A        , sizeof(resetData3), resetData3 , device);
   WRBLOCK( devAddr, CE_REG_PE_NEXP_OFFS__A     , sizeof(resetData4), resetData4 , device);
   WRBLOCK( devAddr, CE_REG_TP_A0_TAP_NEW__A    , sizeof(resetData5), resetData5 , device);
   WRBLOCK( devAddr, CE_REG_TP_A1_TAP_NEW__A    , sizeof(resetData6), resetData6 , device);
   WRBLOCK( devAddr, CE_REG_FI_SHT_INCR__A      , sizeof(resetData7), resetData7 , device);
   WRBLOCK( devAddr, CE_REG_IR_INPUTSEL__A      , sizeof(resetData8), resetData8 , device);

   WR16( devAddr, CE_REG_TI_NEXP_OFFS__A        ,0x0000, 0x0000, device);
   WR16( devAddr, CE_REG_COMM_EXEC__A           ,0x0001, 0x0000, device); /* start ce */

#endif /* DRXD_TYPE_A */

#if (DRXD_TYPE_B)

   devAddr = demod -> myI2CDevAddr;

   WR16( devAddr, CE_REG_TI_PHN_ENABLE__A       ,0x0001, 0x0000, device);
   WR16( devAddr, CE_REG_FR_PM_SET__A           ,0x000D, 0x0000, device);

   WR16( devAddr, CE_REG_COMM_EXEC__A           ,0x0001, 0x0000, device); /* start ce */

#endif   /* DRXD_TYPE_B */

   return (DRX_STS_OK);

 rw_error:
   return (DRX_STS_ERROR);
}

/*============================================================================*/

/**
* \fn DRXStatus_t InitEQ( const pDRXDemodInstance_t demod )

* \brief
* \param demod pointer to demod data.
* \return DRXStatus_t Return status.
*\retval DRX_STS_OK Success.
*\retval DRX_STS_ERROR Failure.
*/
static DRXStatus_t
InitEQ( const pDRXDemodInstance_t demod, DtDevice *device )
{
   pI2CDeviceAddr_t devAddr=NULL;

#if (DRXD_TYPE_A)

   static u8_t resetData1[] =
   {  0x1E,0x00, /* EQ_REG_OT_QNT_THRES0__A        */
      0x1F,0x00, /* EQ_REG_OT_QNT_THRES1__A        */
      0x06,0x00, /* EQ_REG_OT_CSI_STEP__A          */
      0x02,0x00  /* EQ_REG_OT_CSI_OFFSET__A        */
   };
   devAddr = demod -> myI2CDevAddr;

   WRBLOCK( devAddr, EQ_REG_OT_QNT_THRES0__A    , sizeof(resetData1), resetData1, device );

   WR16( devAddr, EQ_REG_TD_REQ_SMB_CNT__A            ,0x0200, 0x0000, device );
   WR16( devAddr, EQ_REG_IS_CLIP_EXP__A               ,0x001F, 0x0000, device );
   WR16( devAddr, EQ_REG_SN_OFFSET__A                  ,-7    , 0x0000, device );
   WR16( devAddr, EQ_REG_RC_SEL_CAR__A                ,0x0002, 0x0000, device );
   WR16( devAddr, EQ_REG_COMM_EXEC__A                 ,0x0001, 0x0000, device );

#endif /* DRXD_TYPE_A */

#if (DRXD_TYPE_B)

   devAddr = demod -> myI2CDevAddr;

   WR16( devAddr, EQ_REG_COMM_EXEC__A                 ,0x0001, 0x0000, device );

#endif   /* DRXD_TYPE_B */

   return (DRX_STS_OK);

 rw_error:
   return (DRX_STS_ERROR);
}

/*============================================================================*/

/**
* \fn DRXStatus_t InitEC( const pDRXDemodInstance_t demod )
* \brief
* \param demod pointer to demod data.
* \return DRXStatus_t Return status.
*\retval DRX_STS_OK Success.
*\retval DRX_STS_ERROR Failure.
*/
static DRXStatus_t
InitEC( const pDRXDemodInstance_t demod, DtDevice *device )
{
   pI2CDeviceAddr_t devAddr=NULL;
   pDRX3973DData_t  extAttr=NULL;

#if (DRXD_TYPE_A)

   static u8_t resetData1[] =
   {  0x1F,0x00, /* EC_SB_REG_CSI_HI__A            */
      0x1E,0x00, /* EC_SB_REG_CSI_LO__A            */
      0x01,0x00, /* EC_SB_REG_SMB_TGL__A           */
      0x7F,0x00, /* EC_SB_REG_SNR_HI__A            */
      0x7F,0x00, /* EC_SB_REG_SNR_MID__A           */
      0x7F,0x00  /* EC_SB_REG_SNR_LO__A            */
   };

   static u8_t resetData2[] =
   {  0x00,0x10, /* EC_RS_REG_REQ_PCK_CNT__A       */
      (EC_RS_REG_VAL_PCK&0xFF),(EC_RS_REG_VAL_PCK>>8)
                 /* EC_RS_REG_VAL__A               */
   };

   static u8_t resetData3[] =
   {  0x03,0x00, /* EC_OC_REG_TMD_TOP_MODE__A      */
      0xF4,0x01, /* EC_OC_REG_TMD_TOP_CNT__A       */
      0xC0,0x03, /* EC_OC_REG_TMD_HIL_MAR__A       */
      0x40,0x00, /* EC_OC_REG_TMD_LOL_MAR__A       */
      0x03,0x00  /* EC_OC_REG_TMD_CUR_CNT__A       */
   };

   static u8_t resetData4[] =
   {  0x06,0x00, /* EC_OC_REG_AVR_ASH_CNT__A       */
      0x02,0x00  /* EC_OC_REG_AVR_BSH_CNT__A       */
   };

   static u8_t resetData5[] =
   {  0x07,0x00, /* EC_OC_REG_RCN_MODE__A          */
      0x00,0x00, /* EC_OC_REG_RCN_CRA_LOP__A       */
      0xc0,0x00, /* EC_OC_REG_RCN_CRA_HIP__A       */
      0x00,0x10, /* EC_OC_REG_RCN_CST_LOP__A       */
      0x00,0x00, /* EC_OC_REG_RCN_CST_HIP__A       */
      0xFF,0x01, /* EC_OC_REG_RCN_SET_LVL__A       */
      0x0A,0x00  /* EC_OC_REG_RCN_GAI_LVL__A       */
   };

   static u8_t resetData6[] =
   {  0xFF,0xFF, /* EC_OC_REG_RCN_CLP_LOP__A       */
      0xFF,0x00  /* EC_OC_REG_RCN_CLP_HIP__A       */
   };


   devAddr = demod -> myI2CDevAddr;
   extAttr = (pDRX3973DData_t) demod -> myExtAttr;

   WRBLOCK( devAddr, EC_SB_REG_CSI_HI__A        , sizeof(resetData1), resetData1, device );
   WRBLOCK( devAddr, EC_RS_REG_REQ_PCK_CNT__A   , sizeof(resetData2), resetData2, device );
   WRBLOCK( devAddr, EC_OC_REG_TMD_TOP_MODE__A  , sizeof(resetData3), resetData3, device );
   WRBLOCK( devAddr, EC_OC_REG_AVR_ASH_CNT__A   , sizeof(resetData4), resetData4, device );
   WRBLOCK( devAddr, EC_OC_REG_RCN_MODE__A      , sizeof(resetData5), resetData5, device );
   WRBLOCK( devAddr, EC_OC_REG_RCN_CLP_LOP__A   , sizeof(resetData6), resetData6, device );

   WR16( devAddr, EC_SB_REG_CSI_OFS__A          , 0x0001, 0x0000, device);
   WR16( devAddr, EC_VD_REG_FORCE__A            , 0x0002, 0x0000, device);
   WR16( devAddr, EC_VD_REG_REQ_SMB_CNT__A      , 0x0001, 0x0000, device);
   WR16( devAddr, EC_VD_REG_RLK_ENA__A          , 0x0001, 0x0000, device);
   WR16( devAddr, EC_OD_REG_SYNC__A             , 0x0664 , 0x0000, device);
   WR16( devAddr, EC_OC_REG_OC_MON_SIO__A       , 0x0000, 0x0000, device);
   WR16( devAddr, EC_OC_REG_SNC_ISC_LVL__A      , 0x0422, 0x0000, device);
   if ( (extAttr->consumerDevice) == TRUE )
   {
      /* Output zero on monitorbus pads, power saving */
      WR16( devAddr, EC_OC_REG_OCR_MON_UOS__A       ,
                  ( EC_OC_REG_OCR_MON_UOS_DAT_0_ENABLE |
                    EC_OC_REG_OCR_MON_UOS_DAT_1_ENABLE |
                    EC_OC_REG_OCR_MON_UOS_DAT_2_ENABLE |
                    EC_OC_REG_OCR_MON_UOS_DAT_3_ENABLE |
                    EC_OC_REG_OCR_MON_UOS_DAT_4_ENABLE |
                    EC_OC_REG_OCR_MON_UOS_DAT_5_ENABLE |
                    EC_OC_REG_OCR_MON_UOS_DAT_6_ENABLE |
                    EC_OC_REG_OCR_MON_UOS_DAT_7_ENABLE |
                    EC_OC_REG_OCR_MON_UOS_DAT_8_ENABLE |
                    EC_OC_REG_OCR_MON_UOS_DAT_9_ENABLE |
                    EC_OC_REG_OCR_MON_UOS_VAL_ENABLE   |
                    EC_OC_REG_OCR_MON_UOS_CLK_ENABLE ), 0x0000, device);
      WR16( devAddr, EC_OC_REG_OCR_MON_WRI__A,
                        EC_OC_REG_OCR_MON_WRI_INIT, 0x0000, device);
   }

   WR16( devAddr, EC_SB_REG_COMM_EXEC__A       ,  0x0001, 0x0000, device);
   WR16( devAddr, EC_VD_REG_COMM_EXEC__A        , 0x0001, 0x0000, device);
   WR16( devAddr, EC_OD_REG_COMM_EXEC__A        , 0x0001, 0x0000, device);
   WR16( devAddr, EC_RS_REG_COMM_EXEC__A        , 0x0001, 0x0000, device);
   WR16( devAddr, EC_OC_REG_COMM_EXEC__A        , 0x0001, 0x0000, device);

#endif /* DRXD_TYPE_A */

#if (DRXD_TYPE_B)

   devAddr = demod -> myI2CDevAddr;
   extAttr = (pDRX3973DData_t) demod -> myExtAttr;

   WR16( devAddr, EC_SB_REG_CSI_OFS0__A         ,0x0001 , 0x0000, device);
   WR16( devAddr, EC_SB_REG_CSI_OFS1__A         ,0x0001 , 0x0000, device);
   WR16( devAddr, EC_SB_REG_CSI_OFS2__A         ,0x0001 , 0x0000, device);
   WR16( devAddr, EC_SB_REG_CSI_LO__A           ,0x000c , 0x0000, device);
   WR16( devAddr, EC_SB_REG_CSI_HI__A           ,0x0018 , 0x0000, device);
   WR16( devAddr, EC_SB_REG_SNR_HI__A           ,0x007f , 0x0000, device);
   WR16( devAddr, EC_SB_REG_SNR_MID__A          ,0x007f , 0x0000, device);
   WR16( devAddr, EC_SB_REG_SNR_LO__A           ,0x007f , 0x0000, device);

   WR16( devAddr, EC_OC_REG_DTO_CLKMODE__A      ,0x0002 , 0x0000, device);
   WR16( devAddr, EC_OC_REG_DTO_PER__A          ,0x0006 , 0x0000, device);
   WR16( devAddr, EC_OC_REG_DTO_BUR__A          ,0x0001 , 0x0000, device);
   WR16( devAddr, EC_OC_REG_RCR_CLKMODE__A      ,0x0000 , 0x0000, device);
   WR16( devAddr, EC_OC_REG_RCN_GAI_LVL__A      ,0x000D , 0x0000, device);
   WR16( devAddr, EC_OC_REG_OC_MPG_SIO__A       ,0x0000 , 0x0000, device);

   WR16( devAddr, EC_OD_REG_SYNC__A             ,0x0664 , 0x0000, device);
   WR16( devAddr, EC_RS_REG_REQ_PCK_CNT__A      ,0x1000  , 0x0000, device);
   WR16( devAddr, EC_COMM_EXEC__A               ,0x0001 , 0x0000, device);

#endif  /* DRXD_TYPE_B */

   return (DRX_STS_OK);

 rw_error:
   return (DRX_STS_ERROR);
}

/*============================================================================*/

/**
* \fn DRXStatus_t ResetEC( const pDRXDemodInstance_t demod )
* \brief
* \param demod pointer to demod data.
* \return DRXStatus_t Return status.
*\retval DRX_STS_OK Success.
*\retval DRX_STS_ERROR Failure.
*/

#if DRXD_TYPE_A

static DRXStatus_t
ResetEC( const pDRXDemodInstance_t demod, DtDevice *device )
{
   pI2CDeviceAddr_t devAddr=NULL;
   pDRX3973DData_t  extAttr=NULL;

   static u8_t resetData3[] =
   {  0x03,0x00, /* EC_OC_REG_TMD_TOP_MODE__A      */
      0xF4,0x01, /* EC_OC_REG_TMD_TOP_CNT__A       */
      0xC0,0x03, /* EC_OC_REG_TMD_HIL_MAR__A       */
      0x40,0x00, /* EC_OC_REG_TMD_LOL_MAR__A       */
      0x03,0x00  /* EC_OC_REG_TMD_CUR_CNT__A       */
   };

   static u8_t resetData4[] =
   {  0x06,0x00, /* EC_OC_REG_AVR_ASH_CNT__A       */
      0x02,0x00  /* EC_OC_REG_AVR_BSH_CNT__A       */
   };

   static u8_t resetData5[] =
   {  0x07,0x00, /* EC_OC_REG_RCN_MODE__A          */
      0x00,0x00, /* EC_OC_REG_RCN_CRA_LOP__A       */
      0xc0,0x00, /* EC_OC_REG_RCN_CRA_HIP__A       */
      0x00,0x10, /* EC_OC_REG_RCN_CST_LOP__A       */
      0x00,0x00, /* EC_OC_REG_RCN_CST_HIP__A       */
      0xFF,0x01, /* EC_OC_REG_RCN_SET_LVL__A       */
      0x0A,0x00  /* EC_OC_REG_RCN_GAI_LVL__A       */
   };

   static u8_t resetData6[] =
   {  0xFF,0xFF, /* EC_OC_REG_RCN_CLP_LOP__A       */
      0xFF,0x00  /* EC_OC_REG_RCN_CLP_HIP__A       */
   };

   devAddr = demod -> myI2CDevAddr;
   extAttr = (pDRX3973DData_t) demod -> myExtAttr;

   WR16( devAddr, EC_OC_REG_COMM_EXEC__A        , 0x0000, 0x0000, device);
   WR16( devAddr, EC_OD_REG_COMM_EXEC__A        , 0x0000, 0x0000, device);

   WRBLOCK( devAddr, EC_OC_REG_TMD_TOP_MODE__A  , sizeof(resetData3), resetData3, device );
   WRBLOCK( devAddr, EC_OC_REG_AVR_ASH_CNT__A   , sizeof(resetData4), resetData4, device );
   WRBLOCK( devAddr, EC_OC_REG_RCN_MODE__A      , sizeof(resetData5), resetData5, device );
   WRBLOCK( devAddr, EC_OC_REG_RCN_CLP_LOP__A   , sizeof(resetData6), resetData6, device );

   WR16( devAddr, EC_OD_REG_SYNC__A             , 0x0664 , 0x0000, device);
   WR16( devAddr, EC_OC_REG_OC_MON_SIO__A       , 0x0000, 0x0000, device);
   WR16( devAddr, EC_OC_REG_SNC_ISC_LVL__A      , 0x0422, 0x0000, device);
   if ( (extAttr->consumerDevice) == TRUE )
   {
      /* Output zero on monitorbus pads, power saving */
      WR16( devAddr, EC_OC_REG_OCR_MON_UOS__A       ,
                  ( EC_OC_REG_OCR_MON_UOS_DAT_0_ENABLE |
                    EC_OC_REG_OCR_MON_UOS_DAT_1_ENABLE |
                    EC_OC_REG_OCR_MON_UOS_DAT_2_ENABLE |
                    EC_OC_REG_OCR_MON_UOS_DAT_3_ENABLE |
                    EC_OC_REG_OCR_MON_UOS_DAT_4_ENABLE |
                    EC_OC_REG_OCR_MON_UOS_DAT_5_ENABLE |
                    EC_OC_REG_OCR_MON_UOS_DAT_6_ENABLE |
                    EC_OC_REG_OCR_MON_UOS_DAT_7_ENABLE |
                    EC_OC_REG_OCR_MON_UOS_DAT_8_ENABLE |
                    EC_OC_REG_OCR_MON_UOS_DAT_9_ENABLE |
                    EC_OC_REG_OCR_MON_UOS_VAL_ENABLE   |
                    EC_OC_REG_OCR_MON_UOS_CLK_ENABLE ), 0x0000, device);
      WR16( devAddr, EC_OC_REG_OCR_MON_WRI__A,
                        EC_OC_REG_OCR_MON_WRI_INIT, 0x0000, device);
   }

   WR16( devAddr, EC_OD_REG_COMM_EXEC__A        , 0x0001, 0x0000, device);
   WR16( devAddr, EC_OC_REG_COMM_EXEC__A        , 0x0001, 0x0000, device);

   return (DRX_STS_OK);

 rw_error:
   return (DRX_STS_ERROR);
}
#endif   /* DRXD_TYPE_A */

/*============================================================================*/

/**
* \fn DRXStatus_t InitSC( const pDRXDemodInstance_t demod )
* \brief
* \param demod pointer to demod data.
* \return DRXStatus_t Return status.
*\retval DRX_STS_OK Success.
*\retval DRX_STS_ERROR Failure.
*/
static DRXStatus_t
InitSC( const pDRXDemodInstance_t demod, DtDevice *device )
{
   pI2CDeviceAddr_t devAddr=NULL;
   devAddr = demod -> myI2CDevAddr;

   /*=================================*/
   /* Reset SC                        */
   /*=================================*/
   WR16( devAddr, SC_CT_REG_COMM_EXEC__A,    0      ,0x0000, device);
   WR16( devAddr, SC_CT_REG_COMM_STATE__A,   0      ,0x0000, device);

#ifdef COMPILE_FOR_QT
   WR16( devAddr, SC_RA_RAM_BE_OPT_DELAY__A,      0x100 ,0x0000, device);
#endif

   /* SC is not started, this is done in SetChannels() */
   return (DRX_STS_OK);

 rw_error:
   return (DRX_STS_ERROR);
}

/*============================================================================*/

#ifdef USE_LC_INIT
/**
* \fn DRXStatus_t InitLC( const pDRXDemodInstance_t demod )
* \brief
* \param demod pointer to demod data.
* \return DRXStatus_t Return status.
*\retval DRX_STS_OK Success.
*\retval DRX_STS_ERROR Failure.
*/
static DRXStatus_t
InitLC( const pDRXDemodInstance_t demod, DtDevice *device )
{
   pI2CDeviceAddr_t devAddr=NULL;

   /* DRXD_TYPE_A & DRXD_TYPE_B settings */

   static u8_t resetData1[] =
   {  0xE6,0xFF, /* LC_RA_RAM_PROC_DELAY_IF__A  */
      0xE3,0xFF, /* LC_RA_RAM_PROC_DELAY_FS__A  */
#ifdef COMPILE_FOR_QT
      /* LC version 0.1.32 */
      0x00,0x01, /* LC_RA_RAM_LOCK_TH_CRMM__A   */
      0x00,0x05  /* LC_RA_RAM_LOCK_TH_SRMM__A   */
#else
      /* LC version 0.1.67 and higher  */
      0x64,0x00, /* LC_RA_RAM_LOCK_TH_CRMM__A   */
      0x64,0x00  /* LC_RA_RAM_LOCK_TH_SRMM__A   */
#endif
   };

   static u8_t resetData2[] =
   {  0x0F,0x00, /* LC_RA_RAM_MODE_FILTER__A        */
      0x00,0x40, /* LC_RA_RAM_WEIGHT_CP_CRMM__A     */
      0x00,0x40, /* LC_RA_RAM_WEIGHT_CE_CRMM__A     */
      0x00,0x04, /* LC_RA_RAM_GAIN_PHASE__A         */
      0x00,0x04, /* LC_RA_RAM_GAIN_DELAY__A         */
      0xFF,0x7F, /* LC_RA_RAM_THRESHOLD_CRMM__A     */
      0xFF,0x7F, /* LC_RA_RAM_THRESHOLD_SRMM__A     */
      0x00,0x00, /* LC_RA_RAM_OFFSET_ADJUST_CRMM__A */
      0x00,0x00, /* LC_RA_RAM_OFFSET_ADJUST_SRMM__A */
      0x00,0x00, /* LC_RA_RAM_OFFSET_PHASE__A       */
      0x10,0x00  /* LC_RA_RAM_MAX_ABS_EXP__A        */
   };

   static u8_t resetData3[] =
   {  0x04,0x00, /* LC_RA_RAM_FILTER_CRMM_A__A  */
      0x01,0x00  /* LC_RA_RAM_FILTER_CRMM_B__A  */
   };

   static u8_t resetData4[] =
   {  0x04,0x00, /* LC_RA_RAM_FILTER_SRMM_A__A  */
      0x01,0x00  /* LC_RA_RAM_FILTER_SRMM_B__A  */
   };

   static u8_t resetData5[] =
   {  0x04,0x00, /* LC_RA_RAM_FILTER_PHASE_A__A */
      0x01,0x00  /* LC_RA_RAM_FILTER_PHASE_B__A */
   };

   static u8_t resetData6[] =
   {  0x04,0x00, /* LC_RA_RAM_FILTER_DELAY_A__A */
      0x01,0x00  /* LC_RA_RAM_FILTER_DELAY_B__A */
   };


   devAddr = demod -> myI2CDevAddr;

   /*=================================*/
   /* Reset LC                        */
   /*=================================*/
   WR16( devAddr, LC_CT_REG_COMM_EXEC__A,    0      ,0x0000, device);
   WR16( devAddr, LC_CT_REG_COMM_STATE__A,   0      ,0x0000, device);

   WR16( devAddr, LC_RA_RAM_MODE_ADJUST__A,  0      , 0x0000, device);

   /*=================================*/
   /* Start LC                        */
   /*=================================*/
   WR16( devAddr, LC_CT_REG_COMM_EXEC__A,     1      , 0x0000, device);

   /*
   turn off

   Allow for overriding defaults.
   In real application all defaults will be correct, so this initialization
   is not needed.
   A cold boot will do when starting the LC for real (after pilot detection).
   */

   /* wait */
   CHK_ERROR( WaitMilliSec(1) );
   WR16( devAddr, LC_CT_REG_COMM_EXEC__A,     0      , 0x0000, device);


   WRBLOCK( devAddr, LC_RA_RAM_PROC_DELAY_IF__A , sizeof(resetData1), resetData1, device );
   WRBLOCK( devAddr, LC_RA_RAM_MODE_FILTER__A   , sizeof(resetData2), resetData2, device );
   WRBLOCK( devAddr, LC_RA_RAM_FILTER_CRMM_A__A , sizeof(resetData3), resetData3, device );
   WRBLOCK( devAddr, LC_RA_RAM_FILTER_SRMM_A__A , sizeof(resetData4), resetData4, device );
   WRBLOCK( devAddr, LC_RA_RAM_FILTER_PHASE_A__A, sizeof(resetData5), resetData5, device );
   WRBLOCK( devAddr, LC_RA_RAM_FILTER_DELAY_A__A, sizeof(resetData6), resetData6, device );


   return (DRX_STS_OK);

 rw_error:
   return (DRX_STS_ERROR);
}
#endif /* USE_LC_INIT */

/*============================================================================*/

/**
* \fn DRXStatus_t InitHI( const pDRXDemodInstance_t demod )
* \brief Initialise and configurate HI.
* \param demod pointer to demod data.
* \return DRXStatus_t Return status.
*\retval DRX_STS_OK Success.
*\retval DRX_STS_ERROR Failure.
*
* Needs to know Psys (System Clock period) and Posc (Osc Clock period)
* Need to store configuration in driver because of the way I2C
* bridging is controlled.
*
*/
static DRXStatus_t
InitHI( const pDRXDemodInstance_t demod, DtDevice *device )
{
   pDRX3973DData_t   extAttr=NULL;
   pDRXCommonAttr_t  commonAttr=NULL;

   extAttr    = (pDRX3973DData_t)  demod -> myExtAttr;
   commonAttr = (pDRXCommonAttr_t) demod -> myCommonAttr;


#ifdef COMPILE_FOR_QT
   extAttr -> HICfgTimingDiv   = 0x0000;
   extAttr -> HICfgBridgeDelay = 0x001f;
#else
   /* Timing div, 250ns/Psys */
   /* Timing div, = ( delay (nano seconds) * sysclk (kHz) )/ 1000 */
   extAttr -> HICfgTimingDiv =
      (u16_t)((commonAttr->sysClockFreq/1000)* HI_I2C_DELAY)/1000 ;
   /* Bridge delay, uses oscilator clock */
   /* Delay = ( delay (nano seconds) * oscclk (kHz) )/ 1000 */
   extAttr -> HICfgBridgeDelay =
      (u16_t)((commonAttr->oscClockFreq/1000)*
               HI_I2C_BRIDGE_DELAY)/1000 ;
#endif
   if(commonAttr->oscClockFreq == 48000 )
   {
      extAttr -> HICfgTimingDiv   = 0x0000;
      extAttr -> HICfgBridgeDelay = 0x001f;
   }

   extAttr -> HICfgWakeUpKey = (demod -> myI2CDevAddr -> i2cAddr);
   /* port/bridge/power down ctrl */
   extAttr -> HICfgCtrl = HI_RA_RAM_SRV_CFG_ACT_SLV0_ON;
   CHK_ERROR( HI_CfgCommand( demod, device ) );

   return (DRX_STS_OK);

 rw_error:
   return (DRX_STS_ERROR);
}

/*=============================================================================
  ===== SC command related functions ==========================================
  ===========================================================================*/
static DRXStatus_t
ScCommand( pI2CDeviceAddr_t devAddr, pDRX3973DScCmd_t cmd, DtDevice *device )
{
   u16_t curCmd = 0;
   u16_t errCode = 0;
   u16_t retryCnt = 0;

   /* Check param */
   if ( cmd == NULL )
   {
      return (DRX_STS_INVALID_ARG);
   }

   /* Wait until sc is ready to receive command */
   retryCnt =0;
   do{
      RR16( devAddr, SC_RA_RAM_CMD__A, &curCmd, 0x0000, device );
      retryCnt++;
   } while ( ( curCmd!= 0 ) && ( retryCnt < DRX3973D_MAX_RETRIES) );
   if ( retryCnt >= DRX3973D_MAX_RETRIES )
   {
      return (DRX_STS_ERROR);
   }

   /* Write sub-command */
   switch ( cmd->cmd ) {
      /* All commands using sub-cmd */
      case SC_RA_RAM_CMD_PROC_START:
      case SC_RA_RAM_CMD_SET_PREF_PARAM:
      case SC_RA_RAM_CMD_PROGRAM_PARAM:
         WR16( devAddr, SC_RA_RAM_CMD_ADDR__A, cmd->subcmd , 0x0000, device );
         break;
      default:
         /* Do nothing */
         break;
   } /* switch (cmd->cmd) */

   /* Write needed parameters and the command */
   switch ( cmd->cmd ) {
      /* All commands using 5 parameters */
      /* All commands using 4 parameters */
      /* All commands using 3 parameters */
      /* All commands using 2 parameters */
      case SC_RA_RAM_CMD_PROC_START:
      case SC_RA_RAM_CMD_SET_PREF_PARAM:
      case SC_RA_RAM_CMD_PROGRAM_PARAM:
         WR16( devAddr, SC_RA_RAM_PARAM1__A, cmd->param1 , 0x0000, device );
      /* All commands using 1 parameters */
      case SC_RA_RAM_CMD_SET_ECHO_TIMING:
      case SC_RA_RAM_CMD_USER_IO:
         WR16( devAddr, SC_RA_RAM_PARAM0__A, cmd->param0 , 0x0000, device );
      /* All commands using 0 parameters */
      case SC_RA_RAM_CMD_GET_OP_PARAM:
      case SC_RA_RAM_CMD_NULL:
         /* Write command */
         WR16( devAddr, SC_RA_RAM_CMD__A, cmd->cmd , 0x0000, device );
         break;
      default:
         /* Unknown command */
         return (DRX_STS_INVALID_ARG);
         break;
   } /* switch (cmd->cmd) */


   /* Wait until sc is ready processing command */
   retryCnt =0;
   do{
      RR16( devAddr, SC_RA_RAM_CMD__A, &curCmd, 0x0000, device );
      retryCnt++;
   } while ( ( curCmd!= 0 )  && ( retryCnt < DRX3973D_MAX_RETRIES) );
   if ( retryCnt >= DRX3973D_MAX_RETRIES )
   {
      return (DRX_STS_ERROR);
   }

   /* Check for illegal cmd */
   RR16( devAddr, SC_RA_RAM_CMD_ADDR__A   , &errCode , 0x0000, device );
   if ( errCode == 0xFFFF )
   {
      /* illegal command */
      return (DRX_STS_INVALID_ARG);
   }

   /* Retreive results parameters from SC */
   switch ( cmd->cmd ) {
      /* All commands yielding 5 results */
      /* All commands yielding 4 results */
      /* All commands yielding 3 results */
      /* All commands yielding 2 results */
      /* All commands yielding 1 result */
      case SC_RA_RAM_CMD_USER_IO:
      case SC_RA_RAM_CMD_GET_OP_PARAM:
         RR16( devAddr, SC_RA_RAM_PARAM0__A, &(cmd->param0) , 0x0000, device );
      /* All commands yielding 0 results */
      case SC_RA_RAM_CMD_SET_ECHO_TIMING:
      case SC_RA_RAM_CMD_SET_TIMER:
      case SC_RA_RAM_CMD_PROC_START:
      case SC_RA_RAM_CMD_SET_PREF_PARAM:
      case SC_RA_RAM_CMD_PROGRAM_PARAM:
      case SC_RA_RAM_CMD_NULL:
         break;
      default:
         /* Unknown command */
         return (DRX_STS_INVALID_ARG);
         break;
   } /* switch (cmd->cmd) */

   return (DRX_STS_OK);

 rw_error:
   return (DRX_STS_ERROR);
}

/*=============================================================================
  ===== HI command related functions ==========================================
  ===========================================================================*/
#ifndef DONT_USE_SEMAPHORES

static DRXStatus_t
HI_SemaphoreP(pI2CDeviceAddr_t devAddr)
{
   u16_t semaphore=0;
   u16_t nrRetries1 = 0;
   u16_t nrRetries2 = 0;

   do {
      /* Avoid deadlock */
      if ( nrRetries1 > DRX3973D_MAX_RETRIES )
      {
         goto rw_error;
      };
      nrRetries1++;

      /* Wait until semaphore contains 0 */
      do {

         RR16( devAddr, HI_RA_RAM_SRV_SEM__A, &semaphore, 0, device);

         /* Avoid deadlock */
         if ( nrRetries2 > DRX3973D_MAX_RETRIES )
         {
            goto rw_error;
         };
         nrRetries2++;

      } while ( semaphore != HI_RA_RAM_SRV_SEM_FREE );

      /* Read-modify-write 1 */
      RMWR16( devAddr, HI_RA_RAM_SRV_SEM__A,
                       HI_RA_RAM_SRV_SEM_CLAIMED,
                       &semaphore, device );
      /* Loop while semaphore was already aquired be another process */
   } while ( semaphore == HI_RA_RAM_SRV_SEM_CLAIMED );

   return (DRX_STS_OK);

 rw_error:
   return (DRX_STS_ERROR);
}

#endif /* #ifndef DONT_USE_SEMAPHORES */
/*============================================================================*/

#ifndef DONT_USE_SEMAPHORES
static DRXStatus_t
HI_SemaphoreV(pI2CDeviceAddr_t devAddr)
{
   WR16( devAddr, HI_RA_RAM_SRV_SEM__A, HI_RA_RAM_SRV_SEM_FREE, 0, device);

   return (DRX_STS_OK);

 rw_error:
   return (DRX_STS_ERROR);
}
#endif /* #ifndef DONT_USE_SEMAPHORES */

/*============================================================================*/

/* TODO fill in all commands  */
static DRXStatus_t
HI_Command(const pI2CDeviceAddr_t devAddr,
           const pDRX3973DHiCmd_t cmd,
           pu16_t result, DtDevice *device)
{
   u16_t  waitCmd=0;
   u16_t  nrRetries = 0;
   Bool_t powerdown_cmd = FALSE;

#ifndef DONT_USE_SEMAPHORES
   /* Set semaphore */
   if ( HI_SemaphoreP( devAddr) != DRX_STS_OK )
   {
      goto rw_error;
   };
#endif /* #ifndef DONT_USE_SEMAPHORES */

   /* Write parameters */
   switch ( cmd->cmd ) {
      case HI_RA_RAM_SRV_CMD_NULL:
         /* No parameters */
         break;

      case HI_RA_RAM_SRV_CMD_UIO:
         WR16(devAddr, HI_RA_RAM_SRV_UIO_KEY__A, HI_RA_RAM_SRV_UIO_KEY_ACT, 0, device);
         WR16(devAddr, HI_RA_RAM_SRV_UIO_SEL__A, cmd->param2, 0, device);
         WR16(devAddr, HI_RA_RAM_SRV_UIO_SET__A, cmd->param3, 0, device);
         break;

      case HI_RA_RAM_SRV_CMD_RESET:
         WR16(devAddr, HI_RA_RAM_SRV_RST_KEY__A, HI_RA_RAM_SRV_RST_KEY_ACT, 0, device);
         break;

      case HI_RA_RAM_SRV_CMD_CONFIG:
         /* TODO can be implemented as a block move */
         WR16(devAddr, HI_RA_RAM_SRV_CFG_KEY__A, HI_RA_RAM_SRV_RST_KEY_ACT, 0, device);
         WR16(devAddr, HI_RA_RAM_SRV_CFG_DIV__A, cmd->param2, 0, device);
         WR16(devAddr, HI_RA_RAM_SRV_CFG_BDL__A, cmd->param3, 0, device);
         WR16(devAddr, HI_RA_RAM_SRV_CFG_WUP__A, cmd->param4, 0, device);
         WR16(devAddr, HI_RA_RAM_SRV_CFG_ACT__A, cmd->param5, 0, device);
         if ( ((cmd->param5) & HI_RA_RAM_SRV_CFG_ACT_PWD_EXE) ==
              HI_RA_RAM_SRV_CFG_ACT_PWD_EXE )
         {
            /* Detect power down to ommit the rest of the HI cmd protocol */
            powerdown_cmd = TRUE;
         }
         break;

      case HI_RA_RAM_SRV_CMD_COPY:
         return (DRX_STS_INVALID_ARG);
         break;

      case HI_RA_RAM_SRV_CMD_TRANSMIT:
         return (DRX_STS_INVALID_ARG);
         break;

      case HI_RA_RAM_SRV_CMD_EXECUTE:
         /* TODO use proper names forthese egisters */
         WR16(devAddr, HI_RA_RAM_SRV_CFG_KEY__A, cmd->param1, 0, device);
         WR16(devAddr, HI_RA_RAM_SRV_CFG_DIV__A, cmd->param2, 0, device);
         WR16(devAddr, HI_RA_RAM_SRV_CFG_BDL__A, cmd->param3, 0, device);
         WR16(devAddr, HI_RA_RAM_SRV_CFG_WUP__A, cmd->param4, 0, device);
         WR16(devAddr, HI_RA_RAM_SRV_CFG_ACT__A, cmd->param5, 0, device);

         break;

      default:
         return (DRX_STS_INVALID_ARG);
         break;
   }

   /* Write command */
   WR16(devAddr, HI_RA_RAM_SRV_CMD__A, cmd->cmd, 0, device);
   if ( (cmd->cmd) == HI_RA_RAM_SRV_CMD_RESET )
   {
      /* Allow for HI to reset */
      WaitMilliSec(1);
   }

#ifdef COMPILE_FOR_QT
      WaitMilliSec(100);
#endif

   if ( powerdown_cmd == FALSE )
   {
      /* Wait until command rdy */
      do
      {
         nrRetries++;
         if ( nrRetries > DRX3973D_MAX_RETRIES )
         {
            goto rw_error;
         };

         RR16(devAddr, HI_RA_RAM_SRV_CMD__A, &waitCmd, 0, device);
      } while ( waitCmd != 0 );

      /* Read result */
      RR16(devAddr, HI_RA_RAM_SRV_RES__A, result, 0, device);

#ifndef DONT_USE_SEMAPHORES
      /* Clear semaphore */
      if ( HI_SemaphoreV( devAddr) != DRX_STS_OK )
      {
         goto rw_error;
      };
#endif /* #ifndef DONT_USE_SEMAPHORES */
   } /* if ( powerdown_cmd == TRUE ) */

   return (DRX_STS_OK);

 rw_error:
   return (DRX_STS_ERROR);
}

/*============================================================================*/

/**
* \fn DRXStatus_t HI_CfgCommand()
* \brief Configure HI with settings stored in the demod structure.
* \param demod Demodulator.
* \return DRXStatus_t.
*
* This routine was created because to much orthogonal settings have
* been put into one HI API function (configure). Especially the I2C bridge
* enable/disable should not need re-configuration of the HI.
*
*/
static DRXStatus_t
HI_CfgCommand(const pDRXDemodInstance_t   demod, DtDevice *device)
{
   pDRX3973DData_t extAttr=NULL;
   DRX3973DHiCmd_t hiCmd;
   u16_t result=0;

   extAttr = (pDRX3973DData_t)demod -> myExtAttr;

   /* Clipping, keep values within max range */
   if ( (extAttr -> HICfgTimingDiv) > HI_RA_RAM_SRV_CFG_DIV__M )
   {
      extAttr -> HICfgTimingDiv = HI_RA_RAM_SRV_CFG_DIV__M;
   }
   if ( (extAttr -> HICfgBridgeDelay) > HI_RA_RAM_SRV_CFG_BDL__M )
   {
      extAttr -> HICfgBridgeDelay = HI_RA_RAM_SRV_CFG_BDL__M;
   }

   hiCmd.cmd    = HI_RA_RAM_SRV_CMD_CONFIG;
   hiCmd.param1 = HI_RA_RAM_SRV_CFG_KEY_ACT;
   hiCmd.param2 = extAttr -> HICfgTimingDiv;
   hiCmd.param3 = extAttr -> HICfgBridgeDelay;
   hiCmd.param4 = extAttr -> HICfgWakeUpKey;
   hiCmd.param5 = extAttr -> HICfgCtrl;

   CHK_ERROR( HI_Command( demod -> myI2CDevAddr, &hiCmd, &result, device) );

   /* Reset power down flag (set one call only) */
   extAttr -> HICfgCtrl &= (~(HI_RA_RAM_SRV_CFG_ACT_PWD_EXE));

   return (DRX_STS_OK);

 rw_error:
   return (DRX_STS_ERROR);
}

/*=============================================================================
  ===== SetChannel() ==========================================================
  ===========================================================================*/

/**
* \fn DRXStatus_t CorrectSysClockDeviation()
* \brief Handle sysclock deviation caused by oscillator deviation.
* \param demod Demodulator.
* \return DRXStatus_t.
*
* SC scans upto n ppm from sysclock, n is determined by stepsize and count
* parameters of SC.
* The result is stored in FE_IF_REG_INCR.
* The driver can compute the actual sysClock from this value and use it for
* subsequent calls to SetChannel(), to speed up the channelswitch.
*
*/
#if DRXD_TYPE_A

static DRXStatus_t
CorrectSysClockDeviation( pDRXDemodInstance_t   demod, DtDevice *device)
{
   pDRX3973DData_t  extAttr=NULL;

   extAttr = (pDRX3973DData_t)demod -> myExtAttr;
   switch( extAttr->CSCDState )
   {
      /*=====================================================================*/
      case DRX3973D_CSCD_INIT:
      {
         pI2CDeviceAddr_t devAddr=NULL;

         devAddr = demod -> myI2CDevAddr;

         /* Setup SC to determine sysclock deviation */
         WR16 ( devAddr, SC_RA_RAM_SAMPLE_RATE_COUNT__A,
                DRX3973D_OSCDEV_DO_SCAN  , 0x0000, device );
         WR16 ( devAddr, SC_RA_RAM_SAMPLE_RATE_STEP__A,
                DRX3973D_OSCDEV_STEP  , 0x0000, device );
         extAttr->CSCDState = DRX3973D_CSCD_SET;
         break;
      }
      /*=====================================================================*/
      case DRX3973D_CSCD_SET:
      {
         DRXLockStatus_t  lockStatus=DRX_NOT_LOCKED;
         pI2CDeviceAddr_t devAddr=NULL;

         devAddr = demod -> myI2CDevAddr;

         /* If lock is sufficient compute & store deviation and disbale
            SC sysclock deviation scan */
         CHK_ERROR( CtrlLockStatus( devAddr, &lockStatus, device) );
         if ( lockStatus >= DRX3973D_DEMOD_LOCK )
         {
            u32_t incr=0;
            u32_t bandwidth=0;
            s32_t h=0;
            u32_t l=0;
            u32_t sysClockInHz=0;
            DRXFrequency_t sysClockFreq=0; /* in kHz */
            pDRXCommonAttr_t commonAttr=NULL;

            commonAttr = (pDRXCommonAttr_t) demod -> myCommonAttr;

            /* Retrieve bandwidth and incr */
            CHK_ERROR( AtomicReadReg32( devAddr, FE_IF_REG_INCR0__A, &incr, 0x0000, device) );

            switch( extAttr->curBandwidth )
            {
               case DRX_BANDWIDTH_8MHZ    :
                  bandwidth = DRX3973D_BANDWIDTH_8MHZ_IN_HZ;
                  break;
               case DRX_BANDWIDTH_7MHZ    :
                  bandwidth = DRX3973D_BANDWIDTH_7MHZ_IN_HZ;
                  break;
               case DRX_BANDWIDTH_6MHZ    :
                  bandwidth = DRX3973D_BANDWIDTH_6MHZ_IN_HZ;
                  break;
               case DRX_BANDWIDTH_UNKNOWN : /* fall through */
               default                    :
                  return (DRX_STS_ERROR);
                  break;
            }

            /* Compute new sysclock value
               sysClockFreq = (((incr + 2^23)*bandwidth)/2^21)/1000 */
            incr += (1<<23);
            Mult32(incr,bandwidth,&h,&l);
            /* sysClockFreq=([h:l]/2^21)/1000 */
            sysClockInHz =( (l>>21)+(h<<11) );
            sysClockFreq= (DRXFrequency_t)(sysClockInHz/1000);
            /* rounding */
            if ( ( sysClockInHz%1000 ) > 500 )
            {
               sysClockFreq++;
            }
            commonAttr->sysClockFreq = sysClockFreq;

            /* Compute clock deviation in ppm */
            commonAttr->oscClockDeviation = (s16_t) (
              ((sysClockFreq - (extAttr->expectedSysClockFreq))*1000000L)/
              (extAttr->expectedSysClockFreq) );

            /* Disable SC scan */
            WR16 ( devAddr, SC_RA_RAM_SAMPLE_RATE_COUNT__A,
                     DRX3973D_OSCDEV_DONT_SCAN  , 0x0000, device );

            /* Keep correcting every subsequent channelswitch .. */
         } /* if ( ... ) */

         break;
      }

      /*=====================================================================*/
      default:
         return (DRX_STS_ERROR);
         break;
   }

   return (DRX_STS_OK);

 rw_error:
   return (DRX_STS_ERROR);
}

#endif

/**
* \fn DRXStatus_t SetChannel()
* \brief Select a new transmission channel.
* \param demod Demodulator.
* \param channel Pointer to channel data.
* \return DRXStatus_t.
*/
static DRXStatus_t
CtrlSetChannel( pDRXDemodInstance_t   demod,
                pDRXChannel_t         channel,
                DtDevice              *device )
{
   u16_t  transmissionParams = 0;
   u16_t  operationMode = 0;

#if (DRXD_TYPE_A)
   u16_t  qpskTdTpsPwr  = 0;
   u16_t  qam16TdTpsPwr = 0;
   u16_t  qam64TdTpsPwr = 0;

   u16_t  qpskSnCeGain  = 0;
   u16_t  qam16SnCeGain = 0;
   u16_t  qam64SnCeGain = 0;

   u16_t  qpskIsGainMan  = 0;
   u16_t  qam16IsGainMan = 0;
   u16_t  qam64IsGainMan = 0;

   u16_t  qpskIsGainExp  = 0;
   u16_t  qam16IsGainExp = 0;
   u16_t  qam64IsGainExp = 0;
#endif /* DRXD_TYPE_A */

   u32_t  feIfIncr = 0;
   Bool_t mirrorFreqSpect = FALSE;
   u32_t  bandwidth = 0;

   DRXFrequency_t  offsetFreq       = 0;
   DRXFrequency_t  intermediateFreq = 0;
   u32_t           addInc           = 0;

   DRX3973DScCmd_t  scCmd;
   pDRXCommonAttr_t commonAttr=NULL;
   pDRX3973DData_t  extAttr=NULL;
   pI2CDeviceAddr_t devAddr=NULL;

   TUNERMode_t    tunerMode      = 0;

   devAddr = demod -> myI2CDevAddr;
   extAttr = (pDRX3973DData_t)demod -> myExtAttr;
   commonAttr = (pDRXCommonAttr_t) demod -> myCommonAttr;

   /*== check arguments ======================================================*/

   if ( channel == NULL )
   {
      return DRX_STS_INVALID_ARG;
   }

   switch ( channel->bandwidth ) {
      case DRX_BANDWIDTH_8MHZ    : /* fall through */
      case DRX_BANDWIDTH_7MHZ    : /* fall through */
      case DRX_BANDWIDTH_6MHZ    :
         /* ok */
         break;
      case DRX_BANDWIDTH_UNKNOWN : /* fall through */
      default                    :
         return (DRX_STS_INVALID_ARG);
         break;
   }

   switch ( channel->mirror ) {
         /* ok */
      case DRX_MIRROR_YES     : /* fall through */
      case DRX_MIRROR_NO      : /* fall through */
      case DRX_MIRROR_AUTO    :
         break;
      case DRX_MIRROR_UNKNOWN : /* fall through */
      default                 :
         return (DRX_STS_INVALID_ARG);
         break;
   }

   switch ( channel->constellation ) {
      case DRX_CONSTELLATION_AUTO   : /* fall through */
      case DRX_CONSTELLATION_QPSK   : /* fall through */
      case DRX_CONSTELLATION_QAM16  : /* fall through */
      case DRX_CONSTELLATION_QAM64  :
         /* ok */
         break;
      case DRX_CONSTELLATION_BPSK   : /* fall through */
      case DRX_CONSTELLATION_PSK8   : /* fall through */
      case DRX_CONSTELLATION_QAM32  : /* fall through */
      case DRX_CONSTELLATION_QAM128 : /* fall through */
      case DRX_CONSTELLATION_QAM256 : /* fall through */
      case DRX_CONSTELLATION_QAM512 : /* fall through */
      case DRX_CONSTELLATION_QAM1024: /* fall through */
      case DRX_CONSTELLATION_UNKNOWN: /* fall through */
      default                       :
         return (DRX_STS_INVALID_ARG);
         break;
   }

   switch ( channel->hierarchy ) {
      case DRX_HIERARCHY_AUTO    : /* fall through */
      case DRX_HIERARCHY_NONE    :
         /* ok */
         break;
      case DRX_HIERARCHY_ALPHA1  : /* fall through */
      case DRX_HIERARCHY_ALPHA2  : /* fall through */
      case DRX_HIERARCHY_ALPHA4  :
         /* ok, check priority */
         switch ( channel->priority) {
            case DRX_PRIORITY_LOW     : /* fall through */
            case DRX_PRIORITY_HIGH    :
               /* ok */
               break;
            case DRX_PRIORITY_UNKNOWN : /* fall through */
            default                   :
               return (DRX_STS_INVALID_ARG);
               break;
         }
         break;
      case DRX_HIERARCHY_UNKNOWN : /* fall through */
      default                    :
         return (DRX_STS_INVALID_ARG);
         break;
   }

   switch ( channel->coderate ) {
      case DRX_CODERATE_AUTO   : /* fall through */
      case DRX_CODERATE_1DIV2  : /* fall through */
      case DRX_CODERATE_2DIV3  : /* fall through */
      case DRX_CODERATE_3DIV4  : /* fall through */
      case DRX_CODERATE_5DIV6  : /* fall through */
      case DRX_CODERATE_7DIV8  :
         /* ok */
         break;
      case DRX_CODERATE_UNKNOWN: /* fall through */
      default              :
         return (DRX_STS_INVALID_ARG);
         break;
   }

   switch ( channel->guard ) {
      case DRX_GUARD_1DIV32  : /* fall through */
      case DRX_GUARD_1DIV16  : /* fall through */
      case DRX_GUARD_1DIV8   : /* fall through */
      case DRX_GUARD_1DIV4   : /* fall through */
      case DRX_GUARD_AUTO    :
         /* ok */
         break;
      case DRX_GUARD_UNKNOWN : /* fall through */
      default                :
         return (DRX_STS_INVALID_ARG);
         break;
   }

   switch ( channel->fftmode) {
      case DRX_FFTMODE_2K      : /* fall through */
      case DRX_FFTMODE_8K      : /* fall through */
      case DRX_FFTMODE_AUTO    :
         /* ok */
         break;
      case DRX_FFTMODE_UNKNOWN : /* fall through */
      default                  :
         return (DRX_STS_INVALID_ARG);
         break;
   }

   switch ( channel->classification) {
      case DRX_CLASSIFICATION_AUTO      :
         /* ok */
         break;
      case DRX_CLASSIFICATION_GAUSS     : /* fall through */
      case DRX_CLASSIFICATION_HVY_GAUSS : /* fall through */
      case DRX_CLASSIFICATION_COCHANNEL : /* fall through */
      case DRX_CLASSIFICATION_STATIC    : /* fall through */
      case DRX_CLASSIFICATION_MOVING    : /* fall through */
      case DRX_CLASSIFICATION_ZERODB    : /* fall through */
      case DRX_CLASSIFICATION_UNKNOWN   : /* fall through */
      default                       :
         return (DRX_STS_INVALID_ARG);
         break;
   }

#if DRXD_TYPE_A
   /*== Correct syslock deviation ============================================*/
   CHK_ERROR( CorrectSysClockDeviation( demod, device ) );
#endif
#if DRXD_TYPE_B
   /* Disable SC scan for clock deviation */
   WR16 ( devAddr, SC_RA_RAM_SAMPLE_RATE_COUNT__A,
          DRX3973D_OSCDEV_DONT_SCAN  , 0x0000 );
#endif

   /*== Store configuration ==================================================*/
   /* MPEG output configuration */
   RR16 ( devAddr, EC_OC_REG_OC_MPG_SIO__A ,
                   &(extAttr->EcOcRegOcMpgSio)  , 0x0000, device );
   RR16 ( devAddr, EC_OC_REG_OC_MODE_LOP__A,
                   &(extAttr->EcOcRegOcModeLop) , 0x0000, device );
   RR16 ( devAddr, EC_OC_REG_OC_MODE_HIP__A,
                   &(extAttr->EcOcRegOcModeHip) , 0x0000, device );
   RR16 ( devAddr, EC_OC_REG_IPR_INV_MPG__A,
                   &(extAttr->EcOcRegIprInvMpg) , 0x0000, device );


#if DRXD_TYPE_A
   /* Stop relevant processors off the device */
   WR16( devAddr, SC_COMM_EXEC__A, SC_COMM_EXEC_CTL_STOP, 0, device );
   WR16( devAddr, LC_COMM_EXEC__A, SC_COMM_EXEC_CTL_STOP, 0, device );
#endif
#if DRXD_TYPE_B
   /* Stop all processors except HI & CC & FE */
   WR16( devAddr, SC_COMM_EXEC__A, SC_COMM_EXEC_CTL_STOP, 0, device );
   WR16( devAddr, LC_COMM_EXEC__A, SC_COMM_EXEC_CTL_STOP, 0, device );
   WR16( devAddr, FT_COMM_EXEC__A, SC_COMM_EXEC_CTL_STOP, 0, device );
   WR16( devAddr, CP_COMM_EXEC__A, SC_COMM_EXEC_CTL_STOP, 0, device );
   WR16( devAddr, CE_COMM_EXEC__A, SC_COMM_EXEC_CTL_STOP, 0, device );
   WR16( devAddr, EQ_COMM_EXEC__A, SC_COMM_EXEC_CTL_STOP, 0, device );
   WR16( devAddr, EC_COMM_EXEC__A, SC_COMM_EXEC_CTL_STOP, 0, device );
#endif

#ifdef COMPILE_FOR_QT
   WR16( devAddr, 0x14010000, 0x0000, 0 );
   WR16( devAddr, 0x14010000, 0x0001, 0 );
#endif

   /*== SAW switch via UIO 1 (HI) ============================================*/
   if ( (extAttr->hiUioMode) == DRX_UIO_MODE_FIRMWARE )
   {
      DRXUIOData_t uio1 = { DRX_UIO1, FALSE };
      DRXStatus_t  sts = DRX_STS_ERROR;

      switch ( channel->bandwidth ) {
         case DRX_BANDWIDTH_8MHZ    :
            uio1.value = TRUE;
            break;
         case DRX_BANDWIDTH_7MHZ    : /* fall through */
         case DRX_BANDWIDTH_6MHZ    :
            uio1.value = FALSE;
            break;
         case DRX_BANDWIDTH_UNKNOWN :
            return (DRX_STS_INVALID_ARG);
            break;
         default:
            return (DRX_STS_INVALID_ARG);
            break;
      }
      /* dirty trick for code reuse ... */
      extAttr->hiUioMode = DRX_UIO_MODE_READWRITE;
      sts = CtrlUIOWrite( demod, &uio1, device);
      extAttr->hiUioMode = DRX_UIO_MODE_FIRMWARE;
      if ( sts != DRX_STS_OK)
      {
         return (sts);
      }
   }

   /*== Tune (1)==============================================================*/

   if ( demod->myTuner !=NULL )
   {
      DRXFrequency_t tunerFreq      = 0;

      /* Start with TUNER_MODE_SWITCH, reprogram tuner later with
         TUNER_MODE_LOCK */
      tunerMode = TUNER_MODE_DIGITAL | TUNER_MODE_SWITCH;
      switch ( channel->bandwidth ) {
         case DRX_BANDWIDTH_8MHZ    :
            tunerMode |= TUNER_MODE_8MHZ;
            break;
         case DRX_BANDWIDTH_7MHZ    :
            tunerMode |= TUNER_MODE_7MHZ;
            break;
         case DRX_BANDWIDTH_6MHZ    :
            tunerMode |= TUNER_MODE_6MHZ;
            break;
         default:
            return (DRX_STS_INVALID_ARG);
            break;
      }

      if ( commonAttr->tunerPortNr == 1 )
      {
         Bool_t bridgeClosed=TRUE;

         CHK_ERROR( CtrlI2CBridge( demod, &bridgeClosed, device ) );
      }
      CHK_ERROR( DRXBSP_TUNER_SetFrequency( demod->myTuner,
                                            tunerMode,
                                            channel->frequency, device ) );
      if ( commonAttr->tunerPortNr == 1 )
      {
         Bool_t bridgeClosed=FALSE;

         CHK_ERROR( CtrlI2CBridge( demod, &bridgeClosed, device ) );
      }

      /* Get actual frequency set by tuner and compute offset */
      CHK_ERROR( DRXBSP_TUNER_GetFrequency( demod->myTuner,
                                            &tunerFreq,
                                            &intermediateFreq ) );
      offsetFreq = tunerFreq - (channel->frequency);
   }
   else
   {
      /* no tuner instance defined, use fixed intermediate frequency */
      offsetFreq = 0;
      intermediateFreq = commonAttr->intermediateFreq;
   }/* if ( demod->myTuner !=NULL ) */


   /*== Re-initialise device =================================================*/


#if DRXD_TYPE_A
   /* Initialize device */
   CHK_ERROR( ResetEC( demod, device ) );
   CHK_ERROR( InitEC( demod, device ) );
#ifdef USE_LC_INIT
   CHK_ERROR( InitLC( demod, device ) );
#endif
   CHK_ERROR( InitSC( demod, device ) );
#endif /* DRXD_TYPE_A */

#if DRXD_TYPE_B
   /* Initialize device */
   CHK_ERROR( InitFT( demod ) );
   CHK_ERROR( InitCP( demod ) );
   CHK_ERROR( InitCE( demod ) );
   CHK_ERROR( InitEQ( demod ) );
   CHK_ERROR( InitEC( demod ) );
   CHK_ERROR( InitSC( demod ) );
#ifdef USE_LC_INIT
   CHK_ERROR( InitLC( demod ) );
#endif
#endif  /* DRXD_TYPE_B */

   /* Restore current IF & RF AGC settings */
   CtrlSetCfgIfAgc( demod, &(extAttr -> IfAgcCfg), device );
   CtrlSetCfgRfAgc( demod, &(extAttr -> RfAgcCfg), device );

   /*== Write channel settings to device =====================================*/

   /* TODO classification not in ucode */

   /* Mirror */
   /* Start with hardware setting, invert if signal is mirrored */
   mirrorFreqSpect = ( commonAttr->mirrorFreqSpect );
   switch ( channel->mirror ) {
      case DRX_MIRROR_YES     :
         mirrorFreqSpect = (!mirrorFreqSpect);
         break;
      case DRX_MIRROR_NO      :
         /* do nothing */
         break;
      case DRX_MIRROR_AUTO    :
         /* do nothing, device will always detect mirroring */
         break;
      case DRX_MIRROR_UNKNOWN : /* fall through */
      default                 :
         return (DRX_STS_INVALID_ARG);
         break;
   }

   /* mode */
   switch ( channel->fftmode) {
      case DRX_FFTMODE_AUTO    :
         operationMode |= SC_RA_RAM_OP_AUTO_MODE__M;
         /* fall trhough , try first guess DRX_FFTMODE_2K */
      case DRX_FFTMODE_2K      :
         transmissionParams |= SC_RA_RAM_OP_PARAM_MODE_2K;
#if (DRXD_TYPE_A)
         WR16( devAddr, EC_SB_REG_TR_MODE__A,   EC_SB_REG_TR_MODE_2K, 0x0000, device );
         qpskSnCeGain  = 97;
         qam16SnCeGain = 71;
         qam64SnCeGain = 65;

#endif /* DRXD_TYPE_A */
         break;
      case DRX_FFTMODE_8K      :
         transmissionParams |= SC_RA_RAM_OP_PARAM_MODE_8K;
#if (DRXD_TYPE_A)
         WR16( devAddr, EC_SB_REG_TR_MODE__A,   EC_SB_REG_TR_MODE_8K, 0x0000, device );
         qpskSnCeGain  = 99;
         qam16SnCeGain = 83;
         qam64SnCeGain = 67;
#endif /* DRXD_TYPE_A */
         break;
      case DRX_FFTMODE_UNKNOWN :
         /* What to do ? */
         return (DRX_STS_INVALID_ARG);
         break;
      default:
         return (DRX_STS_INVALID_ARG);
         break;
   }

   /* guard */
   switch ( channel->guard ) {
      case DRX_GUARD_AUTO    :
         operationMode |= SC_RA_RAM_OP_AUTO_GUARD__M;
         /* fall trhough , try first guess DRX_GUARD_1DIV32 */
      case DRX_GUARD_1DIV32  :
         transmissionParams |= SC_RA_RAM_OP_PARAM_GUARD_32;
         break;
      case DRX_GUARD_1DIV16  :
         transmissionParams |= SC_RA_RAM_OP_PARAM_GUARD_16;
         break;
      case DRX_GUARD_1DIV8   :
         transmissionParams |= SC_RA_RAM_OP_PARAM_GUARD_8;
         break;
      case DRX_GUARD_1DIV4   :
         transmissionParams |= SC_RA_RAM_OP_PARAM_GUARD_4;
         break;
      case DRX_GUARD_UNKNOWN :
         return (DRX_STS_INVALID_ARG);
         break;
      default:
         return (DRX_STS_INVALID_ARG);
         break;
   }

   /* hierarchy */
   switch ( channel->hierarchy ) {
      case DRX_HIERARCHY_AUTO    :
         operationMode |= SC_RA_RAM_OP_AUTO_HIER__M;
         /* fall trhough , try first guess SC_RA_RAM_OP_PARAM_HIER_NO */

      case DRX_HIERARCHY_NONE    :
         transmissionParams |= SC_RA_RAM_OP_PARAM_HIER_NO;
#if (DRXD_TYPE_A)
         WR16( devAddr, EQ_REG_OT_ALPHA__A,   0x0000, 0x0000, device );
         WR16( devAddr, EC_SB_REG_ALPHA__A,   0x0000, 0x0000, device );

         qpskTdTpsPwr  = EQ_TD_TPS_PWR_QPSK;
         qam16TdTpsPwr = EQ_TD_TPS_PWR_QAM16_ALPHAN;
         qam64TdTpsPwr = EQ_TD_TPS_PWR_QAM64_ALPHAN;

         qpskIsGainMan  = SC_RA_RAM_EQ_IS_GAIN_QPSK_MAN__PRE;
         qam16IsGainMan = SC_RA_RAM_EQ_IS_GAIN_16QAM_MAN__PRE;
         qam64IsGainMan = SC_RA_RAM_EQ_IS_GAIN_64QAM_MAN__PRE;

         qpskIsGainExp  = SC_RA_RAM_EQ_IS_GAIN_QPSK_EXP__PRE;
         qam16IsGainExp = SC_RA_RAM_EQ_IS_GAIN_16QAM_EXP__PRE;
         qam64IsGainExp = SC_RA_RAM_EQ_IS_GAIN_64QAM_EXP__PRE;
#endif /* DRXD_TYPE_A */
         break;

      case DRX_HIERARCHY_ALPHA1  :
         transmissionParams |= SC_RA_RAM_OP_PARAM_HIER_A1;
#if (DRXD_TYPE_A)
         WR16( devAddr, EQ_REG_OT_ALPHA__A,   0x0001, 0x0000, device );
         WR16( devAddr, EC_SB_REG_ALPHA__A,   0x0001, 0x0000, device );

         qpskTdTpsPwr  = EQ_TD_TPS_PWR_UNKNOWN;
         qam16TdTpsPwr = EQ_TD_TPS_PWR_QAM16_ALPHA1;
         qam64TdTpsPwr = EQ_TD_TPS_PWR_QAM64_ALPHA1;

         qpskIsGainMan  = SC_RA_RAM_EQ_IS_GAIN_UNKNOWN_MAN__PRE;
         qam16IsGainMan = SC_RA_RAM_EQ_IS_GAIN_16QAM_MAN__PRE;
         qam64IsGainMan = SC_RA_RAM_EQ_IS_GAIN_64QAM_MAN__PRE;

         qpskIsGainExp  = SC_RA_RAM_EQ_IS_GAIN_UNKNOWN_EXP__PRE;
         qam16IsGainExp = SC_RA_RAM_EQ_IS_GAIN_16QAM_EXP__PRE;
         qam64IsGainExp = SC_RA_RAM_EQ_IS_GAIN_64QAM_EXP__PRE;
#endif /* DRXD_TYPE_A */
         break;

      case DRX_HIERARCHY_ALPHA2  :
         transmissionParams |= SC_RA_RAM_OP_PARAM_HIER_A2;
#if (DRXD_TYPE_A)
         WR16( devAddr, EQ_REG_OT_ALPHA__A,   0x0002, 0x0000, device );
         WR16( devAddr, EC_SB_REG_ALPHA__A,   0x0002, 0x0000, device );

         qpskTdTpsPwr  = EQ_TD_TPS_PWR_UNKNOWN;
         qam16TdTpsPwr = EQ_TD_TPS_PWR_QAM16_ALPHA2;
         qam64TdTpsPwr = EQ_TD_TPS_PWR_QAM64_ALPHA2;

         qpskIsGainMan  = SC_RA_RAM_EQ_IS_GAIN_UNKNOWN_MAN__PRE;
         qam16IsGainMan = SC_RA_RAM_EQ_IS_GAIN_16QAM_A2_MAN__PRE;
         qam64IsGainMan = SC_RA_RAM_EQ_IS_GAIN_64QAM_A2_MAN__PRE;

         qpskIsGainExp  = SC_RA_RAM_EQ_IS_GAIN_UNKNOWN_EXP__PRE;
         qam16IsGainExp = SC_RA_RAM_EQ_IS_GAIN_16QAM_A2_EXP__PRE;
         qam64IsGainExp = SC_RA_RAM_EQ_IS_GAIN_64QAM_A2_EXP__PRE;
#endif /* DRXD_TYPE_A */
         break;

      case DRX_HIERARCHY_ALPHA4  :
         transmissionParams |= SC_RA_RAM_OP_PARAM_HIER_A4;
#if (DRXD_TYPE_A)
         WR16( devAddr, EQ_REG_OT_ALPHA__A,   0x0003, 0x0000, device );
         WR16( devAddr, EC_SB_REG_ALPHA__A,   0x0003, 0x0000, device );

         qpskTdTpsPwr  = EQ_TD_TPS_PWR_UNKNOWN;
         qam16TdTpsPwr = EQ_TD_TPS_PWR_QAM16_ALPHA4;
         qam64TdTpsPwr = EQ_TD_TPS_PWR_QAM64_ALPHA4;

         qpskIsGainMan  = SC_RA_RAM_EQ_IS_GAIN_UNKNOWN_MAN__PRE;
         qam16IsGainMan = SC_RA_RAM_EQ_IS_GAIN_16QAM_A4_MAN__PRE;
         qam64IsGainMan = SC_RA_RAM_EQ_IS_GAIN_64QAM_A4_MAN__PRE;

         qpskIsGainExp  = SC_RA_RAM_EQ_IS_GAIN_UNKNOWN_EXP__PRE;
         qam16IsGainExp = SC_RA_RAM_EQ_IS_GAIN_16QAM_A4_EXP__PRE;
         qam64IsGainExp = SC_RA_RAM_EQ_IS_GAIN_64QAM_A4_EXP__PRE;
#endif /* DRXD_TYPE_A */
         break;
      case DRX_HIERARCHY_UNKNOWN :
         return (DRX_STS_INVALID_ARG);
         break;

      default:
         return (DRX_STS_INVALID_ARG);
         break;
   }

   /* constellation */
   switch ( channel->constellation ) {

      case DRX_CONSTELLATION_AUTO   :
         operationMode |= SC_RA_RAM_OP_AUTO_CONST__M;
         /* fall trhough , try first guess DRX_CONSTELLATION_QPSK */

      case DRX_CONSTELLATION_QPSK   :
         transmissionParams |= SC_RA_RAM_OP_PARAM_CONST_QPSK;
#if (DRXD_TYPE_A)
         WR16( devAddr, EQ_REG_OT_CONST__A,   0x0000, 0x0000, device );
         WR16( devAddr, EC_SB_REG_CONST__A, EC_SB_REG_CONST_QPSK, 0x0000, device);
         WR16( devAddr, EC_SB_REG_SCALE_MSB__A,  0x0010, 0x0000, device );
         WR16( devAddr, EC_SB_REG_SCALE_BIT2__A, 0x0000, 0x0000, device );
         WR16( devAddr, EC_SB_REG_SCALE_LSB__A, 0x0000, 0x0000, device );

         WR16( devAddr, EQ_REG_TD_TPS_PWR_OFS__A, qpskTdTpsPwr, 0x0000, device );
         WR16( devAddr, EQ_REG_SN_CEGAIN__A, qpskSnCeGain, 0x0000, device );
         WR16( devAddr, EQ_REG_IS_GAIN_MAN__A, qpskIsGainMan, 0x0000, device );
         WR16( devAddr, EQ_REG_IS_GAIN_EXP__A, qpskIsGainExp, 0x0000, device );
#endif /* DRXD_TYPE_A */
         break;

      case DRX_CONSTELLATION_QAM16  :
         transmissionParams |= SC_RA_RAM_OP_PARAM_CONST_QAM16;
#if (DRXD_TYPE_A)
         WR16( devAddr, EQ_REG_OT_CONST__A,   0x0001, 0x0000, device );
         WR16( devAddr, EC_SB_REG_CONST__A, EC_SB_REG_CONST_16QAM, 0x0000, device);
         WR16( devAddr, EC_SB_REG_SCALE_MSB__A,  0x0010, 0x0000, device );
         WR16( devAddr, EC_SB_REG_SCALE_BIT2__A, 0x0004, 0x0000, device );
         WR16( devAddr, EC_SB_REG_SCALE_LSB__A, 0x0000, 0x0000, device );

         WR16( devAddr, EQ_REG_TD_TPS_PWR_OFS__A, qam16TdTpsPwr, 0x0000, device );
         WR16( devAddr, EQ_REG_SN_CEGAIN__A, qam16SnCeGain, 0x0000, device );
         WR16( devAddr, EQ_REG_IS_GAIN_MAN__A, qam16IsGainMan, 0x0000, device );
         WR16( devAddr, EQ_REG_IS_GAIN_EXP__A, qam16IsGainExp, 0x0000, device );
#endif /* DRXD_TYPE_A */
         break;

      case DRX_CONSTELLATION_QAM64  :
         transmissionParams |= SC_RA_RAM_OP_PARAM_CONST_QAM64;
#if (DRXD_TYPE_A)
         WR16( devAddr, EQ_REG_OT_CONST__A,   0x0002, 0x0000, device );
         WR16( devAddr, EC_SB_REG_CONST__A, EC_SB_REG_CONST_64QAM, 0x0000, device);
         WR16( devAddr, EC_SB_REG_SCALE_MSB__A,  0x0020, 0x0000, device );
         WR16( devAddr, EC_SB_REG_SCALE_BIT2__A, 0x0008, 0x0000, device );
         WR16( devAddr, EC_SB_REG_SCALE_LSB__A, 0x0002, 0x0000, device );

         WR16( devAddr, EQ_REG_TD_TPS_PWR_OFS__A, qam64TdTpsPwr, 0x0000, device );
         WR16( devAddr, EQ_REG_SN_CEGAIN__A, qam64SnCeGain, 0x0000, device );
         WR16( devAddr, EQ_REG_IS_GAIN_MAN__A, qam64IsGainMan, 0x0000, device );
         WR16( devAddr, EQ_REG_IS_GAIN_EXP__A, qam64IsGainExp, 0x0000, device );
#endif /* DRXD_TYPE_A */
         break;

      case DRX_CONSTELLATION_BPSK   : /* fall through */
      case DRX_CONSTELLATION_PSK8   : /* fall through */
      case DRX_CONSTELLATION_QAM32  : /* fall through */
      case DRX_CONSTELLATION_QAM128 : /* fall through */
      case DRX_CONSTELLATION_QAM256 : /* fall through */
      case DRX_CONSTELLATION_QAM512 : /* fall through */
      case DRX_CONSTELLATION_QAM1024: /* fall through */
      case DRX_CONSTELLATION_UNKNOWN:
         return (DRX_STS_INVALID_ARG);
         break;

      default:
         return (DRX_STS_INVALID_ARG);
         break;
   }

   /* Priority (only for hierarchical channels) */
   switch ( channel->priority) {
      case DRX_PRIORITY_LOW     :
         transmissionParams |= SC_RA_RAM_OP_PARAM_PRIO_LO;
         WR16( devAddr, EC_SB_REG_PRIOR__A,   EC_SB_REG_PRIOR_LO, 0x0000, device );
         break;
      case DRX_PRIORITY_HIGH    :
         transmissionParams |= SC_RA_RAM_OP_PARAM_PRIO_HI;
         WR16( devAddr, EC_SB_REG_PRIOR__A,   EC_SB_REG_PRIOR_HI, 0x0000, device );
         break;
      case DRX_PRIORITY_UNKNOWN :
         return (DRX_STS_INVALID_ARG);
         break;
      default:
         return (DRX_STS_INVALID_ARG);
         break;
   }

   /* coderate */
   switch ( channel->coderate ) {
      case DRX_CODERATE_AUTO   :
         operationMode |= SC_RA_RAM_OP_AUTO_RATE__M;
         /* fall trhough , try first guess DRX_CODERATE_1DIV2 */

      case DRX_CODERATE_1DIV2  :
         transmissionParams |= SC_RA_RAM_OP_PARAM_RATE_1_2;
#if (DRXD_TYPE_A)
         WR16( devAddr, EC_VD_REG_SET_CODERATE__A,
                        EC_VD_REG_SET_CODERATE_C1_2, 0x0000, device );
#endif /* DRXD_TYPE_A */
         break;
      case DRX_CODERATE_2DIV3  :
         transmissionParams |= SC_RA_RAM_OP_PARAM_RATE_2_3;
#if (DRXD_TYPE_A)
         WR16( devAddr, EC_VD_REG_SET_CODERATE__A,
                        EC_VD_REG_SET_CODERATE_C2_3, 0x0000, device );
#endif /* DRXD_TYPE_A */
         break;
      case DRX_CODERATE_3DIV4  :
         transmissionParams |= SC_RA_RAM_OP_PARAM_RATE_3_4;
#if (DRXD_TYPE_A)
         WR16( devAddr, EC_VD_REG_SET_CODERATE__A,
                        EC_VD_REG_SET_CODERATE_C3_4, 0x0000, device );
#endif /* DRXD_TYPE_A */
         break;
      case DRX_CODERATE_5DIV6  :
         transmissionParams |= SC_RA_RAM_OP_PARAM_RATE_5_6;
#if (DRXD_TYPE_A)
         WR16( devAddr, EC_VD_REG_SET_CODERATE__A,
                        EC_VD_REG_SET_CODERATE_C5_6, 0x0000, device );
#endif /* DRXD_TYPE_A */
         break;
      case DRX_CODERATE_7DIV8  :
         transmissionParams |= SC_RA_RAM_OP_PARAM_RATE_7_8;
#if (DRXD_TYPE_A)
         WR16( devAddr, EC_VD_REG_SET_CODERATE__A,
                        EC_VD_REG_SET_CODERATE_C7_8, 0x0000, device );
#endif /* DRXD_TYPE_A */
         break;

      case DRX_CODERATE_UNKNOWN:
         return (DRX_STS_INVALID_ARG);
         break;

      default:
         return (DRX_STS_INVALID_ARG);
         break;
   }

   /* bandwidth */
   /* store bandwidth for GetChannel() */
   extAttr->curBandwidth = channel->bandwidth;

   /* SAW filter selection: normaly not necesarry, but if wanted
      the application can select a SAW filter via the driver by using UIOs */
   /* First determine real bandwidth (Hz) */
   /* Also set delay for impulse noise cruncher */
   switch ( channel->bandwidth ) {
      case DRX_BANDWIDTH_8MHZ    :
         bandwidth = DRX3973D_BANDWIDTH_8MHZ_IN_HZ;
         WR16( devAddr, FE_AG_REG_IND_DEL__A , 50 , 0x0000, device );
         break;
      case DRX_BANDWIDTH_7MHZ    :
         bandwidth = DRX3973D_BANDWIDTH_7MHZ_IN_HZ;
         WR16( devAddr, FE_AG_REG_IND_DEL__A , 59 , 0x0000, device );
         break;
      case DRX_BANDWIDTH_6MHZ    :
         bandwidth = DRX3973D_BANDWIDTH_6MHZ_IN_HZ;
         WR16( devAddr, FE_AG_REG_IND_DEL__A , 71 , 0x0000, device );
         break;
      case DRX_BANDWIDTH_UNKNOWN :
         return (DRX_STS_INVALID_ARG);
         break;
      default:
         return (DRX_STS_INVALID_ARG);
         break;
   }

   /* Now compute FE_IF_REG_INCR
       ((( SysFreq/BandWidth)/2)/2) -1) * 2^23)
      =>
       ((SysFreq / BandWidth) * (2^21) ) - (2^23)
   */
   /* (SysFreq / BandWidth) * (2^28)  */
   /* assert ( MAX(sysClk)/MIN(bandwidth) < 16 )
      => assert( MAX(sysClk) < 16*MIN(bandwidth) )
      => assert( 109714272 > 48000000 ) = TRUE so Frac 28 can be used  */
   feIfIncr = Frac28( (u32_t)(commonAttr->sysClockFreq *1000), bandwidth );
   /* (SysFreq / BandWidth) * (2^21)  */
   feIfIncr = feIfIncr >> 7 ;
   /* ((SysFreq / BandWidth) * (2^21) ) - (2^23)  */
   feIfIncr = feIfIncr - (1<<23);

   WR16( devAddr, FE_IF_REG_INCR0__A,
      (u16_t)(feIfIncr & FE_IF_REG_INCR0__M ), 0x0000, device);
   WR16( devAddr, FE_IF_REG_INCR1__A,
      (u16_t)((feIfIncr >> FE_IF_REG_INCR0__W) & FE_IF_REG_INCR1__M ), 0x0000, device);
   /* Bandwidth setting done */

   /* Mirror & frequency offset */
   /*
      Program add_incr of frequency shifter (28 bits, fixed point fraction)
      Sign of the increment is dependent on the tuner signal:
         - mirrored -> positive sign
         - not mirrored -> negative sign
      add_incr = 2^28 * (intermediate frequency tuner / SysClk frequency )
   */
   /* Frac28 is unsigned ! */
   addInc =  Frac28( (u32_t)(intermediateFreq + offsetFreq),
                     (u32_t)(commonAttr->sysClockFreq) );
   /* remove integer part */
   addInc &= 0x0FFFFFFFL;
   /* Negate, 2s complement to mirror frequency spectrum */
   if ( mirrorFreqSpect )
   {
      addInc = 0x10000000L - addInc;
   }
   /* write low part & high part to device */
   WR16( devAddr, FE_FS_REG_ADD_INC_LOP__A,
                  (u16_t)(addInc & FE_FS_REG_ADD_INC_LOP__M ), 0x0000, device);
   WR16( devAddr, FE_FS_REG_ADD_INC_HIP__A,
                  (u16_t)((addInc>>16) & FE_FS_REG_ADD_INC_HIP__M ), 0x0000, device);

   /* Compute original FS without tuner offset correction for use in GetChannel() */
   addInc =  Frac28( (u32_t)(intermediateFreq),
                     (u32_t)(commonAttr->sysClockFreq) );
   /* remove integer part */
   addInc &= 0x0FFFFFFFL;
   /* Negate, 2s complement to mirror frequency spectrum */
   if ( mirrorFreqSpect )
   {
      addInc = 0x10000000L - addInc;
   }
   extAttr->orgFeFsAddIncr = addInc;

   /*== Restore configuration ================================================*/
   /* MPEG output configuration */
   WR16 ( devAddr, EC_OC_REG_OC_MPG_SIO__A ,
                   extAttr->EcOcRegOcMpgSio  , 0x0000, device );
   WR16 ( devAddr, EC_OC_REG_OC_MODE_LOP__A,
                   extAttr->EcOcRegOcModeLop , 0x0000, device );
   WR16 ( devAddr, EC_OC_REG_OC_MODE_HIP__A,
                   extAttr->EcOcRegOcModeHip , 0x0000, device );
   WR16 ( devAddr, EC_OC_REG_IPR_INV_MPG__A,
                   extAttr->EcOcRegIprInvMpg , 0x0000, device );


   /*== Tune (2)==============================================================*/
   /*
   if ( demod->myTuner !=NULL )
   {
      tunerMode = TUNER_MODE_DIGITAL | TUNER_MODE_LOCK;
      switch ( channel->bandwidth ) {
         case DRX_BANDWIDTH_8MHZ    :
            tunerMode |= TUNER_MODE_8MHZ;
            break;
         case DRX_BANDWIDTH_7MHZ    :
            tunerMode |= TUNER_MODE_7MHZ;
            break;
         case DRX_BANDWIDTH_6MHZ    :
            tunerMode |= TUNER_MODE_6MHZ;
            break;
         default:
            return (DRX_STS_INVALID_ARG);
            break;
      }

      if ( commonAttr->tunerPortNr == 1 )
      {
         Bool_t bridgeClosed=TRUE;

         CHK_ERROR( CtrlI2CBridge( demod, &bridgeClosed ) );
      }
      CHK_ERROR( DRXBSP_TUNER_SetFrequency( demod->myTuner,
                                            tunerMode,
                                            channel->frequency ) );
      if ( commonAttr->tunerPortNr == 1 )
      {
         Bool_t bridgeClosed=FALSE;

         CHK_ERROR( CtrlI2CBridge( demod, &bridgeClosed ) );
      }
   }
   */
   /*== Start SC, write channel settings to SC ===============================*/

   /* Enable SC after setting all other parameters */
   WR16( devAddr, SC_CT_REG_COMM_EXEC__A,     1      , 0x0000, device);

   /* Write SC parameter registers, set all AUTO flags in operation mode */
   scCmd.param0 = transmissionParams;
   scCmd.param1 = ( SC_RA_RAM_OP_AUTO_MODE__M  |
                    SC_RA_RAM_OP_AUTO_GUARD__M |
                    SC_RA_RAM_OP_AUTO_CONST__M |
                    SC_RA_RAM_OP_AUTO_HIER__M  |
                    SC_RA_RAM_OP_AUTO_RATE__M  );
   scCmd.subcmd = 0x0000 ;
   scCmd.cmd    = SC_RA_RAM_CMD_SET_PREF_PARAM;
   CHK_ERROR( ScCommand( devAddr, &scCmd, device) );

   /* Start correct processes to get in lock */
   scCmd.param0 = SC_RA_RAM_SW_EVENT_RUN_NMASK__M;
   scCmd.param1 = SC_RA_RAM_LOCKTRACK_MIN;
   scCmd.subcmd = SC_RA_RAM_PROC_LOCKTRACK;
   scCmd.cmd    = SC_RA_RAM_CMD_PROC_START;
   CHK_ERROR( ScCommand( devAddr, &scCmd, device) );

   return (DRX_STS_OK);

 rw_error:
   return (DRX_STS_ERROR);
}

/*=============================================================================
  ===== GetChannel() ==========================================================
  ===========================================================================*/
/**
* \fn DRXStatus_t GetChannel()
* \brief Retreive parameters of current transmission channel.
* \param demod   Pointer to demod instance.
* \param channel Pointer to channel data.
* \return DRXStatus_t.
*/
static DRXStatus_t
CtrlGetChannel( pDRXDemodInstance_t   demod,
                pDRXChannel_t         channel,
                DtDevice *device )
{
   u16_t transmissionParams = 0;
   DRX3973DScCmd_t  scCmd;
   pI2CDeviceAddr_t devAddr=NULL;
   pDRXCommonAttr_t commonAttr=NULL;
   pDRX3973DData_t  extAttr=NULL;
   DRXLockStatus_t  lockStatus=DRX_NOT_LOCKED;
   u16_t FeFsRegAddIncHip = 0;
   u32_t FeFsRegAddInc = 0;
   s32_t deltaFeFsRegAddInc = 0;
   s32_t FeFsOffset = 0;
   s32_t h=0;
   u32_t l=0;
   Bool_t mirroredFreqSpect = FALSE;

   /*== check arguments ======================================================*/

   if ( channel == NULL )
   {
      return DRX_STS_INVALID_ARG;
   }

   devAddr = demod -> myI2CDevAddr;
   extAttr = (pDRX3973DData_t)demod -> myExtAttr;
   commonAttr = (pDRXCommonAttr_t) demod -> myCommonAttr;

   /* Ask SC for current settings settings */
   scCmd.subcmd = 0x0000 ;
   scCmd.cmd    = SC_RA_RAM_CMD_GET_OP_PARAM;
   CHK_ERROR( ScCommand( devAddr, &scCmd, device) );
   transmissionParams = scCmd.param0;

   if ( demod->myTuner != NULL )
   {
      DRXFrequency_t intermediateFreq = 0;

      /* Get frequency */
      CHK_ERROR( DRXBSP_TUNER_GetFrequency( demod->myTuner,
                                            &(channel->frequency),
                                            &intermediateFreq ) );
   }
   else
   {
      channel->frequency = 0;
   }

   /* Compute frequency offset, skip this if not in lock

      offset in kHz = ( (SysClkFreq )/(2^28) ) * delta(FeFsRegAddInc)
      where:
      delta(FeFsRegAddInc) = current FeFsRegAddInc - original FeFsRegAddInc

      Control code:

      double dOffset =0.0;
      dOffset = (commonAttr->sysClockFreq)/(268435456.0);
      dOffset = dOffset * ( (double) deltaFeFsRegAddInc );
   */
   CHK_ERROR( AtomicReadReg32( devAddr, FE_FS_REG_ADD_INC_LOP__A, &FeFsRegAddInc, 0x0000, device) );
   CHK_ERROR( CtrlLockStatus( devAddr, &lockStatus, device) );
   if ( lockStatus == DRX_LOCKED )
   {
      deltaFeFsRegAddInc = ((s32_t)FeFsRegAddInc) - extAttr->orgFeFsAddIncr;
      /* [h:l] = sysClkFreq  * delta(FesRegAddInc) , where sysClkFreq in kHz */
      Mult32( commonAttr->sysClockFreq, deltaFeFsRegAddInc, &h, &l );
      /* FeFsOffset = [h:l] / (2^28) */
      FeFsOffset = (l>>28) + (h<<4); /* in kHz */
      channel->frequency -= FeFsOffset;
   }

   /* TODO classification not in ucode */
   channel->classification = DRX_CLASSIFICATION_UNKNOWN;

   /* mirror */
   RR16( devAddr, FE_FS_REG_ADD_INC_HIP__A, &FeFsRegAddIncHip, 0x0000, device);
   mirroredFreqSpect =( ( FeFsRegAddIncHip & 0x0800 ) != 0x0800 );
   mirroredFreqSpect ^=( commonAttr->mirrorFreqSpect );
   if ( mirroredFreqSpect )
   {
      channel->mirror = DRX_MIRROR_YES;
   } else {
      channel->mirror = DRX_MIRROR_NO;
   }

   /* mode */
   switch ( transmissionParams & SC_RA_RAM_OP_PARAM_MODE__M ) {
      case SC_RA_RAM_OP_PARAM_MODE_2K:
         channel->fftmode = DRX_FFTMODE_2K;
         break;
      case SC_RA_RAM_OP_PARAM_MODE_8K:
         channel->fftmode = DRX_FFTMODE_8K;
         break;
      default:
         return (DRX_STS_ERROR);
         break;
   }

   /* guard */
   switch ( transmissionParams & SC_RA_RAM_OP_PARAM_GUARD__M) {
      case SC_RA_RAM_OP_PARAM_GUARD_32:
         channel->guard = DRX_GUARD_1DIV32;
         break;
      case SC_RA_RAM_OP_PARAM_GUARD_16:
         channel->guard = DRX_GUARD_1DIV16;
         break;
      case SC_RA_RAM_OP_PARAM_GUARD_8:
         channel->guard = DRX_GUARD_1DIV8;
         break;
      case SC_RA_RAM_OP_PARAM_GUARD_4:
         channel->guard = DRX_GUARD_1DIV4;
         break;
      default:
         return (DRX_STS_ERROR);
         break;
   }

   /* hierarchy */
   switch ( transmissionParams & SC_RA_RAM_OP_PARAM_HIER__M ) {
      case SC_RA_RAM_OP_PARAM_HIER_NO:
         channel->hierarchy = DRX_HIERARCHY_NONE;
         break;
      case SC_RA_RAM_OP_PARAM_HIER_A1:
         channel->hierarchy = DRX_HIERARCHY_ALPHA1;
         break;
      case SC_RA_RAM_OP_PARAM_HIER_A2:
         channel->hierarchy = DRX_HIERARCHY_ALPHA2;
         break;
      case SC_RA_RAM_OP_PARAM_HIER_A4:
         channel->hierarchy = DRX_HIERARCHY_ALPHA4;
         break;
      default:
         return (DRX_STS_ERROR);
         break;
   }

   /* constellation */
   switch ( transmissionParams & SC_RA_RAM_OP_PARAM_CONST__M ) {

      case SC_RA_RAM_OP_PARAM_CONST_QPSK:
         channel->constellation = DRX_CONSTELLATION_QPSK;
         break;
      case SC_RA_RAM_OP_PARAM_CONST_QAM16:
         channel->constellation = DRX_CONSTELLATION_QAM16;
         break;
      case SC_RA_RAM_OP_PARAM_CONST_QAM64:
         channel->constellation = DRX_CONSTELLATION_QAM64;
         break;
      default:
         return (DRX_STS_ERROR);
         break;
   }

   /* Priority (only for hierarchical channels) */
   switch ( transmissionParams & SC_RA_RAM_OP_PARAM_PRIO__M) {
      case SC_RA_RAM_OP_PARAM_PRIO_LO:
         channel->priority = DRX_PRIORITY_LOW;
         break;
      case SC_RA_RAM_OP_PARAM_PRIO_HI:
         channel->priority = DRX_PRIORITY_HIGH;
         break;
      default:
         return (DRX_STS_ERROR);
         break;
   }

   /* coderate */
   switch ( transmissionParams & SC_RA_RAM_OP_PARAM_RATE__M) {
      case SC_RA_RAM_OP_PARAM_RATE_1_2  :
         channel->coderate = DRX_CODERATE_1DIV2;
         break;
      case SC_RA_RAM_OP_PARAM_RATE_2_3  :
         channel->coderate = DRX_CODERATE_2DIV3;
         break;
      case SC_RA_RAM_OP_PARAM_RATE_3_4  :
         channel->coderate = DRX_CODERATE_3DIV4;
         break;
      case SC_RA_RAM_OP_PARAM_RATE_5_6  :
         channel->coderate = DRX_CODERATE_5DIV6;
         break;
      case SC_RA_RAM_OP_PARAM_RATE_7_8  :
         channel->coderate = DRX_CODERATE_7DIV8;
         break;
      default:
         return (DRX_STS_ERROR);
         break;
   }

   /* Bandwidth */
   channel->bandwidth = extAttr->curBandwidth;

   return (DRX_STS_OK);

 rw_error:
   return (DRX_STS_ERROR);
}

/*============================================================================*/

/*=============================================================================
  ===== SigQuality() ==========================================================
  ===========================================================================*/

/**
* \fn DRXStatus_t CtrlSigQuality()
* \brief Retreive signal quality form device.
* \param devmod Pointer to demodulator instance.
* \param sigQuality Pointer to signal quality data.
* \return DRXStatus_t.
* \retval DRX_STS_OK sigQuality contains valid data.
* \retval DRX_STS_INVALID_ARG sigQuality is NULL.
* \retval DRX_STS_ERROR Erroneous data, sigQuality contains invalid data.

*/
static DRXStatus_t
CtrlSigQuality( pDRXDemodInstance_t demod,
                pDRXSigQuality_t    sigQuality,
                DtDevice *device )
{
   u16_t EcVdRegErrBitCnt = 0 ;
   u16_t EcVdRegInBitCnt  = 0 ;
   u16_t EcRsRegErrBitCnt = 0 ;
   u16_t EcRsRegInPckCnt = 0 ;
   u16_t EcRsRegErrPckCnt = 0 ;

   u16_t EqRegTdSqrErrI   = 0 ;
   u16_t EqRegTdSqrErrQ   = 0 ;
   u16_t EqRegTdSqrErrExp = 0 ;
   u16_t EqRegTdTpsPwrOfs = 0 ;
   u16_t EqRegTdReqSmbCnt = 0 ;

   u32_t tpsCnt = 0;
   u32_t SqrErrIQ = 0;
   u32_t a=0,b=0,c=0;

   u32_t iMER = 0;

   DRX3973DScCmd_t  scCmd;
   DRXLockStatus_t  lockStatus;

   const u32_t rsPacketLength = 204*8; /* fixed! */
   u32_t postViterbiDenominator = 0;

   pDRX3973DData_t  extAttr=NULL;
   pI2CDeviceAddr_t devAddr=NULL;

   devAddr = demod -> myI2CDevAddr;
   extAttr = (pDRX3973DData_t)demod -> myExtAttr;

   /* Check arguments */
   if ( sigQuality == NULL )
   {
      return (DRX_STS_INVALID_ARG);
   }

   if(!( extAttr->ignoreLockSigQuality))
   {
      CHK_ERROR( CtrlLockStatus( devAddr, &lockStatus, device) );
      if ( lockStatus != DRX_LOCKED )
      {
         return (DRX_STS_ERROR);
      }
   }

  /* EC_VD_REG_REQ_SMB_CNT__A and/or EC_VD_REG_REQ_BIT_CNT__A can be written
      to set nr of symbols or bits over which
      to measure EC_VD_REG_ERR_BIT_CNT__A . See CtrlSetCfg(). */

   /* EC_RS_REG_REQ_PCK_CNT__A can be written to set nr of packets over which
      to measure EC_RS_REG_ERR_BIT_CNT__A. See CtrlSetCfg(). */

   /* Read registers for post/preViterbi BER calculation */
   RR16( devAddr, EC_VD_REG_ERR_BIT_CNT__A, &EcVdRegErrBitCnt  , 0x0000, device );
   RR16( devAddr, EC_VD_REG_IN_BIT_CNT__A , &EcVdRegInBitCnt   , 0x0000, device );
   RR16( devAddr, EC_RS_REG_ERR_BIT_CNT__A, &EcRsRegErrBitCnt  , 0x0000, device );
   RR16( devAddr, EC_RS_REG_IN_PCK_CNT__A , &EcRsRegInPckCnt   , 0x0000, device );
   RR16( devAddr, EC_RS_REG_ERR_PCK_CNT__A, &EcRsRegErrPckCnt  , 0x0000, device );

   /* Read registers for MER calculation */
   /* TODO ERR_I, ERR_Q and ERR_EXP should be retreived via SC
           otherwise race conditions
   */
   RR16( devAddr, EQ_REG_TD_TPS_PWR_OFS__A , &EqRegTdTpsPwrOfs  , 0x0000, device );
   RR16( devAddr, EQ_REG_TD_SQR_ERR_I__A   , &EqRegTdSqrErrI    , 0x0000, device );
   RR16( devAddr, EQ_REG_TD_SQR_ERR_Q__A   , &EqRegTdSqrErrQ    , 0x0000, device );
   RR16( devAddr, EQ_REG_TD_REQ_SMB_CNT__A , &EqRegTdReqSmbCnt  , 0x0000, device );
   RR16( devAddr, EQ_REG_TD_SQR_ERR_EXP__A , &EqRegTdSqrErrExp  , 0x0000, device );

   scCmd.subcmd = 0x0000 ;
   scCmd.cmd    = SC_RA_RAM_CMD_GET_OP_PARAM;
   CHK_ERROR( ScCommand( devAddr, &scCmd, device) );

   /* Check input data for post/preViterbi BER calculation */
   /* Check denominators on zero */
   if ( ( EcVdRegInBitCnt == 0 ) || ( EcRsRegInPckCnt == 0 ) )
   {
      return (DRX_STS_ERROR);
   };

   /* Check if denominators are smaller then nominators: if so error */
   postViterbiDenominator = ((u32_t)EcRsRegInPckCnt) * rsPacketLength;
   if ( ( EcVdRegInBitCnt < EcVdRegErrBitCnt ) ||
           ( postViterbiDenominator < ((u32_t)EcRsRegErrBitCnt) )  )
   {
      return (DRX_STS_ERROR);
   };

   /* Check input data for MER */
   if ( EqRegTdReqSmbCnt == 0) /* Measurement over 0 symbols */
   {
      return (DRX_STS_ERROR);
   };

   /* Compute desired results */

   /* PreViterbi is computed in steps of 10^(-6) */
   sigQuality->preViterbiBER  = FracTimes1e6( EcVdRegErrBitCnt,
                                              ((u32_t)EcVdRegInBitCnt) );

   /* PostViterbi is computed in steps of 10^(-6) */
   sigQuality->postViterbiBER  = FracTimes1e6( EcRsRegErrBitCnt,
                                               postViterbiDenominator );

   /* BER values are computed in steps of 10^(-6) */
   sigQuality->scaleFactorBER  = ((u32_t)1000000);

   /* Number of errorneous packets */
   sigQuality->packetError  = EcRsRegErrPckCnt;

   /* MER calculation (in 0.1 dB) without math.h */

   if (( EqRegTdTpsPwrOfs == 0 ) || (EqRegTdReqSmbCnt == 0 ))
   {
      iMER = 0;
   }
   else if ( ( EqRegTdSqrErrI + EqRegTdSqrErrQ ) == 0 )
   {
      /* perfect signal */
      iMER = 600;
   }
   else
   {
      SqrErrIQ = ((u32_t)EqRegTdSqrErrI+(u32_t)EqRegTdSqrErrQ)
                     << EqRegTdSqrErrExp;
      if (((scCmd.param0)&SC_RA_RAM_OP_PARAM_MODE__M)
                                       ==SC_RA_RAM_OP_PARAM_MODE_2K)
      {
         tpsCnt = 17;
      } else {
         tpsCnt = 68;
      }

      /* IMER = 100 * log10 ( x )
         where x = (EqRegTdTpsPwrOfs^2 * EqRegTdReqSmbCnt * tpsCnt)/SqrErrIQ

         => IMER = a + b -c
         where a = 100 * log10 (EqRegTdTpsPwrOfs^2)
               b = 100 * log10 (EqRegTdReqSmbCnt * tpsCnt)
               c = 100 * log10 (SqrErrIQ)
      */

      /* log(x) x = 9bits * 9bits -> 18 bits  */
      a = Log10Times100( EqRegTdTpsPwrOfs*EqRegTdTpsPwrOfs );
      /* log(x) x = 16bits * 7bits -> 23 bits  */
      b = Log10Times100( EqRegTdReqSmbCnt*tpsCnt );
      /* log(x) x = (16bits + 16bits) << 15  -> 32 bits  */
      c = Log10Times100( SqrErrIQ );

      iMER = a + b;
      /* No negative MER, clip to zero */
      if ( iMER > c )
      {
         iMER -= c;
      } else {
         iMER = 0;
      }
   }

   sigQuality->MER = (u16_t) iMER;

   return (DRX_STS_OK);

 rw_error:
   return (DRX_STS_ERROR);
}

/*=============================================================================
  ===== SigStrength() =========================================================
  ===========================================================================*/

/**
* \fn DRXStatus_t CtrlSigStrength()
* \brief Retrieve signal strength.
* \param devmod Pointer to demodulator instance.
* \param sigQuality Pointer to signal strength data; range 0, .. , 1023.
* \return DRXStatus_t.
* \retval DRX_STS_OK sigStrength contains valid data.
* \retval DRX_STS_INVALID_ARG sigStrength is NULL.
* \retval DRX_STS_ERROR Erroneous data, sigStrength contains invalid data.

*/
static DRXStatus_t
CtrlSigStrength( pI2CDeviceAddr_t    devAddr,
                 pu16_t              sigStrength, DtDevice *device )
{
   u16_t AgRegGc1AgcDat   = 0 ;

   /* Check arguments */
   if ( sigStrength == NULL )
   {
      return (DRX_STS_INVALID_ARG);
   }

   RR16( devAddr, FE_AG_REG_GC1_AGC_DAT__A,  &AgRegGc1AgcDat    , 0x0000, device );

   /* Signal strength indication */
   *sigStrength = (FE_AG_REG_GC1_AGC_DAT__M - AgRegGc1AgcDat);


   return (DRX_STS_OK);

 rw_error:
   return (DRX_STS_ERROR);
}

/*===========================================================================
======= CtrlLockStatus() ====================================================
===========================================================================*/

/**
* \fn DRXStatus_t CtrlLockStatus()
* \brief Retreive lock status .
* \param devAddr Pointer to demodulator device address.
* \param lockStat Pointer to lock status structure.
* \return DRXStatus_t.
**/
static DRXStatus_t
CtrlLockStatus( pI2CDeviceAddr_t  devAddr,
                pDRXLockStatus_t  lockStat, DtDevice *device )
{
   u16_t ScRaRamLock = 0;
   const u16_t mpeg_lock_mask  = ( SC_RA_RAM_LOCK_MPEG__M |
                                   SC_RA_RAM_LOCK_FEC__M  |
                                   SC_RA_RAM_LOCK_DEMOD__M );
   const u16_t fec_lock_mask   = ( SC_RA_RAM_LOCK_FEC__M  |
                                   SC_RA_RAM_LOCK_DEMOD__M );
   const u16_t demod_lock_mask =   SC_RA_RAM_LOCK_DEMOD__M ;

   RR16( devAddr, SC_RA_RAM_LOCK__A, &ScRaRamLock, 0x0000, device );

   if ( (ScRaRamLock & mpeg_lock_mask) == mpeg_lock_mask )
   {
      *lockStat = DRX_LOCKED;
   } else if ( (ScRaRamLock & fec_lock_mask) == fec_lock_mask )
   {
      *lockStat = DRX3973D_FEC_LOCK;
   } else if ( (ScRaRamLock & demod_lock_mask) == demod_lock_mask )
   {
      *lockStat = DRX3973D_DEMOD_LOCK;
   } else
   {
      *lockStat = DRX_NOT_LOCKED;
   }

   return (DRX_STS_OK);

 rw_error:
   return (DRX_STS_ERROR);
}

/*=============================================================================
  ===== CtrlTPSInfo() =========================================================
  ===========================================================================*/

/**
* \fn DRXStatus_t CtrlTPSInfo()
* \brief Retreive TPS information .
* \param demod Pointer to demodulator instance.
* \param TPSInfo Pointer to TPSInfo structure.
* \return DRXStatus_t.
*
*/

static DRXStatus_t
CtrlTPSInfo( pDRXDemodInstance_t    demod,
             pDRXTPSInfo_t          TPSInfo, DtDevice *device )
{
   pI2CDeviceAddr_t devAddr = NULL;
   pDRX3973DData_t  extAttr = NULL;
   u16_t            value = 0;

   devAddr = demod -> myI2CDevAddr;
   extAttr = (pDRX3973DData_t)demod -> myExtAttr;

   /* fft mode */
   RR16( devAddr, EQ_REG_TD_TPS_TR_MODE__A, &value, 0x0000, device);
   switch( value & EQ_REG_TD_TPS_TR_MODE__M ) {
      case EQ_REG_TD_TPS_TR_MODE_2K:
         TPSInfo->fftmode = DRX_FFTMODE_2K ;
         break;
      case EQ_REG_TD_TPS_TR_MODE_8K:
         TPSInfo->fftmode = (DRXFftmode_t) DRX_FFTMODE_8K ;
         break;
      default:
         TPSInfo->fftmode = (DRXFftmode_t) DRX_UNKNOWN ;
         break;
   } /* switch ( ... ) */

   /* guard */
   RR16( devAddr, EQ_REG_TD_TPS_GUARD__A, &value, 0x0000, device);
   switch( value & EQ_REG_TD_TPS_GUARD__M ) {
      case EQ_REG_TD_TPS_GUARD_32:
         TPSInfo->guard = DRX_GUARD_1DIV32 ;
         break;
      case EQ_REG_TD_TPS_GUARD_16:
         TPSInfo->guard = DRX_GUARD_1DIV16 ;
         break;
      case EQ_REG_TD_TPS_GUARD_08:
         TPSInfo->guard = DRX_GUARD_1DIV8 ;
         break;
      case EQ_REG_TD_TPS_GUARD_04:
         TPSInfo->guard = DRX_GUARD_1DIV4 ;
         break;
      default:
         TPSInfo->guard = (DRXGuard_t) DRX_UNKNOWN ;
         break;
   } /* switch ( ... ) */

   /* constellation */
   RR16( devAddr, EQ_REG_TD_TPS_CONST__A, &value, 0x0000, device);
   switch( value & EQ_REG_TD_TPS_CONST__M ) {
      case EQ_REG_TD_TPS_CONST_QPSK:
         TPSInfo->constellation = DRX_CONSTELLATION_QPSK ;
         break;
      case EQ_REG_TD_TPS_CONST_16QAM:
         TPSInfo->constellation = DRX_CONSTELLATION_QAM16 ;
         break;
      case EQ_REG_TD_TPS_CONST_64QAM:
         TPSInfo->constellation = DRX_CONSTELLATION_QAM64 ;
         break;
      default:
         TPSInfo->constellation = (DRXConstellation_t) DRX_UNKNOWN ;
         break;
   } /* switch ( ... ) */

   /* hierarchy */
   RR16( devAddr, EQ_REG_TD_TPS_HINFO__A, &value, 0x0000, device);
   switch( value & EQ_REG_TD_TPS_HINFO__M ) {
      case EQ_REG_TD_TPS_HINFO_NH:
         TPSInfo->hierarchy = DRX_HIERARCHY_NONE ;
         break;
      case EQ_REG_TD_TPS_HINFO_H1:
         TPSInfo->hierarchy = DRX_HIERARCHY_ALPHA1;
         break;
      case EQ_REG_TD_TPS_HINFO_H2:
         TPSInfo->hierarchy = DRX_HIERARCHY_ALPHA2;
         break;
      case EQ_REG_TD_TPS_HINFO_H4:
         TPSInfo->hierarchy = DRX_HIERARCHY_ALPHA4;
         break;
      default:
         TPSInfo->hierarchy = (DRXHierarchy_t) DRX_UNKNOWN ;
         break;
   } /* switch ( ... ) */

   /* high priority coderate */
   RR16( devAddr, EQ_REG_TD_TPS_CODE_HP__A, &value, 0x0000, device);
   switch( value & EQ_REG_TD_TPS_CODE_HP__M ) {
      case EQ_REG_TD_TPS_CODE_HP_1_2:
         TPSInfo->highCoderate = DRX_CODERATE_1DIV2 ;
         break;
      case EQ_REG_TD_TPS_CODE_HP_2_3:
         TPSInfo->highCoderate = DRX_CODERATE_2DIV3 ;
         break;
      case EQ_REG_TD_TPS_CODE_HP_3_4:
         TPSInfo->highCoderate = DRX_CODERATE_3DIV4 ;
         break;
      case EQ_REG_TD_TPS_CODE_HP_5_6:
         TPSInfo->highCoderate = DRX_CODERATE_5DIV6 ;
         break;
      case EQ_REG_TD_TPS_CODE_HP_7_8:
         TPSInfo->highCoderate = DRX_CODERATE_7DIV8 ;
         break;
      default:
         TPSInfo->highCoderate = (DRXCoderate_t) DRX_UNKNOWN ;
         break;
   } /* switch ( ... ) */

   /* low priority coderate */
   RR16( devAddr, EQ_REG_TD_TPS_CODE_LP__A, &value, 0x0000, device);
   switch( value & EQ_REG_TD_TPS_CODE_LP__M ) {
      case EQ_REG_TD_TPS_CODE_LP_1_2:
         TPSInfo->lowCoderate = DRX_CODERATE_1DIV2 ;
         break;
      case EQ_REG_TD_TPS_CODE_LP_2_3:
         TPSInfo->lowCoderate = DRX_CODERATE_2DIV3 ;
         break;
      case EQ_REG_TD_TPS_CODE_LP_3_4:
         TPSInfo->lowCoderate = DRX_CODERATE_3DIV4 ;
         break;
      case EQ_REG_TD_TPS_CODE_LP_5_6:
         TPSInfo->lowCoderate = DRX_CODERATE_5DIV6 ;
         break;
      case EQ_REG_TD_TPS_CODE_LP_7_8:
         TPSInfo->lowCoderate = DRX_CODERATE_7DIV8 ;
         break;
      default:
         TPSInfo->lowCoderate = (DRXCoderate_t) DRX_UNKNOWN ;
         break;
   } /* switch ( ... ) */

   /* frame */
   RR16( devAddr, EQ_REG_TD_TPS_FRM_NMB__A, &value, 0x0000, device);
   switch( value & EQ_REG_TD_TPS_FRM_NMB__M ) {
      case EQ_REG_TD_TPS_FRM_NMB_1:
         TPSInfo->frame = DRX_TPS_FRAME1 ;
         break;
      case EQ_REG_TD_TPS_FRM_NMB_2:
         TPSInfo->frame = DRX_TPS_FRAME2 ;
         break;
      case EQ_REG_TD_TPS_FRM_NMB_3:
         TPSInfo->frame = DRX_TPS_FRAME3 ;
         break;
      case EQ_REG_TD_TPS_FRM_NMB_4:
         TPSInfo->frame = DRX_TPS_FRAME4;
         break;
      default:
         TPSInfo->frame = (DRXTPSFrame_t) DRX_UNKNOWN ;
         break;
   } /* switch ( ... ) */

   /* length */
   RR16( devAddr, EQ_REG_TD_TPS_LEN__A, &value, 0x0000, device);
   TPSInfo->length = (u8_t) (value & EQ_REG_TD_TPS_LEN__M);

   /* cell identifiers */
   RR16( devAddr, EQ_REG_TD_TPS_CELL_ID_LO__A, &(TPSInfo->cellId), 0x0000, device);
   TPSInfo->cellId &= EQ_REG_TD_TPS_CELL_ID_LO__M;
   RR16( devAddr, EQ_REG_TD_TPS_CELL_ID_HI__A, &value, 0x0000, device);
   value &= EQ_REG_TD_TPS_CELL_ID_HI__M;
   TPSInfo->cellId |= (value << 8);


   return (DRX_STS_OK);

 rw_error:
    return (DRX_STS_ERROR);
}

/*============================================================================*/

/**
* \fn DRXStatus_t CtrlConstel()
* \brief Retreive a constellation point via I2C.
* \param demod Pointer to demodulator instance.
* \param complexNr Pointer to the structure in which to store the
                   constellation point.
* \return DRXStatus_t.

Pre-condition: Device must be started and in lock.
This precondition is checked in this routine.
*/
static DRXStatus_t
CtrlConstel( pI2CDeviceAddr_t  devAddr,
             pDRXComplex_t     complexNr, DtDevice *device )
{
   u16_t eqRegCommMb = 0;
   u16_t nrRetries = 0;
   u16_t ready = 0;
   u16_t im = 0;
   u16_t re = 0;
   DRXLockStatus_t  lockStat = DRX_NOT_LOCKED;

   /* check arguments */
   if (complexNr == NULL )
   {
      return (DRX_STS_INVALID_ARG);
   }

   /* Check for demod lock or higher */
   CHK_ERROR( CtrlLockStatus( devAddr, &lockStat, device ));
   if ( lockStat == DRX_NOT_LOCKED )
   {
      /* No demod lock or higher, so no constellation available */
      return (DRX_STS_ERROR);
   }

   /* Config MB (Monitor bus) */
   /* TODO: Store current state MB ? */

   /* according to EB: only change observe bits, leave ctrl bits */
   RR16( devAddr, EQ_REG_COMM_MB__A, &eqRegCommMb  , 0x0000, device );
   eqRegCommMb &= (~ ( EQ_REG_COMM_MB_OBS__M + EQ_REG_COMM_MB_OBS_MUX__M));
   eqRegCommMb |= (EQ_REG_COMM_MB_OBS_ON + EQ_REG_COMM_MB_OBS_MUX_EQ_RC );
   WR16( devAddr, EQ_REG_COMM_MB__A, eqRegCommMb   , 0x0000, device );

   /* Instruct device to start sampling MB */
   /* TODO remove hard constants */
   WR16( devAddr, EC_OC_REG_OCR_MON_RDX__A, 0,  0x0000, device );

   /* Wait until rdy */
   do {
      RR16( devAddr, EC_OC_REG_OCR_MON_RDX__A, &ready,  0x0000, device );
      nrRetries++;
   } while ( ( ready != 1 ) && ( nrRetries < DRX3973D_MAX_RETRIES ) );
   if ( nrRetries >= DRX3973D_MAX_RETRIES )
   {
      return (DRX_STS_ERROR);
   }

   /* Interpret I and Q */
   RR16( devAddr, EC_OC_REG_OCR_MON_RD0__A, &re, 0x0000, device );
   RR16( devAddr, EC_OC_REG_OCR_MON_RD1__A, &im, 0x0000, device );
   re &= 0x3FF;
   im &= 0x3FF;

   /* sign extension, 10th bit is sign bit */
   if ( (re & 0x0200) == 0x0200 )
   {
      re |= 0xFC00;
   }
   if ( (im & 0x0200) == 0x0200 )
   {
      im |= 0xFC00;
   }
   complexNr->re = ((s16_t)re) ;
   complexNr->im = ((s16_t)im) ;

   /* TODO: Restore previous state MB ? */

   return (DRX_STS_OK);

 rw_error:
    return (DRX_STS_ERROR);
}

/*============================================================================*/

/**
* \fn DRXStatus_t CtrlI2CBridge()
* \brief Open or close the I2C bridge.
* \param demod Pointer to demodulator instance.
* \param bridgeClosed Pointer to bool indication if bridge is closed not.
* \return DRXStatus_t.

*/
static DRXStatus_t
CtrlI2CBridge( pDRXDemodInstance_t demod,
               pBool_t             bridgeClosed, DtDevice *device )
{
   pDRX3973DData_t extAttr=NULL;

   /* check arguments */
   if (bridgeClosed == NULL )
   {
      return (DRX_STS_INVALID_ARG);
   }

   extAttr = (pDRX3973DData_t)demod -> myExtAttr;

   extAttr->HICfgCtrl &= (~HI_RA_RAM_SRV_CFG_ACT_BRD__M);
   if (*bridgeClosed == TRUE)
   {
      extAttr->HICfgCtrl |= HI_RA_RAM_SRV_CFG_ACT_BRD_ON;
   }
   else
   {
      extAttr->HICfgCtrl |= HI_RA_RAM_SRV_CFG_ACT_BRD_OFF;
   }

   return ( HI_CfgCommand( demod, device ) );
}

/*============================================================================*/

/**
* \fn DRXStatus_t CtrlUIOCfg()
* \brief Configure modus oprandi UIO.
* \param demod Pointer to demodulator instance.
* \param UIOCfg Pointer to a configuration setting for a certain UIO.
* \return DRXStatus_t.
*/
static DRXStatus_t
CtrlUIOCfg( pDRXDemodInstance_t demod,
            pDRXUIOCfg_t         UIOCfg, DtDevice *device)
{
   pDRX3973DData_t extAttr=NULL;

   if ( UIOCfg == NULL )
   {
      return DRX_STS_INVALID_ARG;
   }

   extAttr = (pDRX3973DData_t)demod -> myExtAttr;

   switch ( UIOCfg->uio ) {
      /*======================================================================*/
      case DRX_UIO1 :
         /* DRX_UIO1: UIO at HI */
         switch ( UIOCfg->mode )
         {
            /*= supported modes =*/
            case DRX_UIO_MODE_FIRMWARE: /* falltrough */
            case DRX_UIO_MODE_READWRITE:
               extAttr->hiUioMode = UIOCfg->mode;
               break;
            case DRX_UIO_MODE_DISABLE:
               {
                  /* Config user IO hardware as read */
                  DRX3973DHiCmd_t hiCmd;
                  u16_t value=0;

                  hiCmd.cmd=HI_RA_RAM_SRV_CMD_UIO;
                  hiCmd.param2=HI_RA_RAM_SRV_UIO_SEL_UIO;
                  hiCmd.param3=HI_RA_RAM_SRV_UIO_SET_DIR_IN;
                  CHK_ERROR( HI_Command( demod->myI2CDevAddr, &hiCmd, &value, device) );
                  extAttr->hiUioMode = UIOCfg->mode;
               }
               break;
            /*= unsupported modes =*/
            default:
               return DRX_STS_INVALID_ARG;
               break;
         } /* switch ( UIOCfg->mode ) */
         break;

      /*======================================================================*/
      case DRX_UIO2 :
         /* DRX_UIO2: IRQN at SC */
         switch ( UIOCfg->mode )
         {
            /*= supported modes =*/
            case DRX_UIO_MODE_READWRITE:
               extAttr->scUioMode = UIOCfg->mode;
               break;
            case DRX_UIO_MODE_DISABLE:
               {
                  /* Config user IO hardware as read */
                  DRX3973DScCmd_t scCmd;

                  scCmd.cmd=SC_RA_RAM_CMD_USER_IO;
                  scCmd.param0=2;
                  CHK_ERROR( ScCommand( demod->myI2CDevAddr, &scCmd, device) );
                  extAttr->scUioMode = UIOCfg->mode;
               }
               break;
            /*= unsupported modes =*/
            case DRX_UIO_MODE_FIRMWARE: /* falltrough */
            default:
               return DRX_STS_INVALID_ARG;
               break;
         } /* switch ( UIOCfg->mode ) */
         break;

      /*======================================================================*/
      default:
         return DRX_STS_INVALID_ARG;
   } /* switch ( UIOCfg->uio ) */

   return (DRX_STS_OK);

 rw_error:
    return (DRX_STS_ERROR);
}

/*============================================================================*/

/**
* \fn DRXStatus_t CtrlUIORead()
* \brief Read from a UIO.
* \param demod Pointer to demodulator instance.
* \param UIOData Pointer to data container for a certain UIO.
* \return DRXStatus_t.
*/
static DRXStatus_t
CtrlUIORead( pDRXDemodInstance_t demod,
             pDRXUIOData_t       UIOData, DtDevice *device)
{
   pDRX3973DData_t extAttr=NULL;

   if ( UIOData == NULL )
   {
      return DRX_STS_INVALID_ARG;
   }

   extAttr = (pDRX3973DData_t)demod -> myExtAttr;

   switch ( UIOData->uio ) {
      case DRX_UIO1 :
      /*======================================================================*/
         /* DRX_UIO1: UIO at HI */
         {
            DRX3973DHiCmd_t hiCmd;
            u16_t value=0;

            if ( extAttr->hiUioMode != DRX_UIO_MODE_READWRITE )
            {
               return DRX_STS_ERROR;
            }
            hiCmd.cmd=HI_RA_RAM_SRV_CMD_UIO;
            hiCmd.param2=HI_RA_RAM_SRV_UIO_SEL_UIO;
            hiCmd.param3=HI_RA_RAM_SRV_UIO_SET_DIR_IN;
            CHK_ERROR( HI_Command( demod->myI2CDevAddr, &hiCmd, &value, device) );
            if ( value == 0 )
            {
               UIOData->value = FALSE;
            } else {
               UIOData->value = TRUE;
            }
         }

         break;

      /*======================================================================*/
      case DRX_UIO2 :
         /* DRX_UIO2: IRQN at SC */
         {
            DRX3973DScCmd_t scCmd;

            if ( extAttr->scUioMode != DRX_UIO_MODE_READWRITE )
            {
               return DRX_STS_ERROR;
            }
            scCmd.cmd=SC_RA_RAM_CMD_USER_IO;
            scCmd.param0=2;
            CHK_ERROR( ScCommand( demod->myI2CDevAddr, &scCmd, device) );
            if ( scCmd.param0 == 0 )
            {
               UIOData->value = FALSE;
            } else {
               UIOData->value = TRUE;
            }
         }

         break;

      /*======================================================================*/
      default:
         return DRX_STS_INVALID_ARG;
   } /* switch ( UIOData->uio ) */

   return (DRX_STS_OK);

 rw_error:
    return (DRX_STS_ERROR);
}

/*============================================================================*/

/**
* \fn DRXStatus_t CtrlUIOWrite()
* \brief Write to a UIO.
* \param demod Pointer to demodulator instance.
* \param UIOData Pointer to data container for a certain UIO.
* \return DRXStatus_t.
*/
static DRXStatus_t
CtrlUIOWrite( pDRXDemodInstance_t demod,
              pDRXUIOData_t       UIOData, DtDevice *device)
{
   pDRX3973DData_t extAttr=NULL;

   if ( UIOData == NULL )
   {
      return DRX_STS_INVALID_ARG;
   }

   extAttr = (pDRX3973DData_t)demod -> myExtAttr;

   switch ( UIOData->uio ) {
      case DRX_UIO1 :
      /*======================================================================*/
         /* DRX_UIO1: UIO at HI */
         {
            DRX3973DHiCmd_t hiCmd;
            u16_t value=0;

            if ( extAttr->hiUioMode != DRX_UIO_MODE_READWRITE )
            {
               return DRX_STS_ERROR;
            }
            hiCmd.cmd=HI_RA_RAM_SRV_CMD_UIO;
            hiCmd.param2=HI_RA_RAM_SRV_UIO_SEL_UIO;
            if ( UIOData->value == TRUE )
            {
               hiCmd.param3= HI_RA_RAM_SRV_UIO_SET_DIR_OUT |
                             HI_RA_RAM_SRV_UIO_SET_OUT_HI ;
            } else {
               hiCmd.param3= HI_RA_RAM_SRV_UIO_SET_DIR_OUT |
                             HI_RA_RAM_SRV_UIO_SET_OUT_LO ;
            }
            CHK_ERROR( HI_Command( demod->myI2CDevAddr, &hiCmd, &value, device) );
         }
         break;

      /*======================================================================*/
      case DRX_UIO2 :
         /* DRX_UIO2: IRQN at SC */
         {
            DRX3973DScCmd_t scCmd;

            if ( extAttr->scUioMode != DRX_UIO_MODE_READWRITE )
            {
               return DRX_STS_ERROR;
            }
            scCmd.cmd=SC_RA_RAM_CMD_USER_IO;
            if ( UIOData->value == TRUE )
            {
               scCmd.param0= 1 ;
            } else {
               scCmd.param0= 0 ;
            }
            CHK_ERROR( ScCommand( demod->myI2CDevAddr, &scCmd, device ) );
         }
         break;

      /*======================================================================*/
      default:
         return DRX_STS_INVALID_ARG;
   } /* switch ( UIOData->uio ) */

   return (DRX_STS_OK);

 rw_error:
    return (DRX_STS_ERROR);
}

/*============================================================================*/

/**
* \fn DRXStatus_t CtrlSetStandard()
* \brief Set modulation standard to be used.
* \param standard Modulation standard.
* \return DRXStatus_t.
*
* Accepts DVBT only. Returns error otherwise.
*
*/
static DRXStatus_t
CtrlSetStandard( pDRXStandard_t standard )
{
   /* check arguments */
   if ( standard == NULL )
   {
      return (DRX_STS_INVALID_ARG);
   }

   if ( (*standard) == DRX_STANDARD_DVBT )
   {
      return ( DRX_STS_OK );
   }
   else
   {
      return ( DRX_STS_ERROR );
   }
}

/*============================================================================*/

/**
* \fn DRXStatus_t CtrlGetStandard()
* \brief Get modulation standard currently used to demodulate.
* \param standard Modulation standard.
* \return DRXStatus_t.
*
* Returns DVBT only.
*
*/
static DRXStatus_t
CtrlGetStandard( pDRXStandard_t standard )
{
   /* check arguments */
   if ( standard == NULL )
   {
      return (DRX_STS_INVALID_ARG);
   }

   (*standard) = DRX_STANDARD_DVBT ;

   return ( DRX_STS_OK );
}

/*============================================================================*/

/**
* \fn DRXStatus_t CtrlPowerMode()
* \brief Set the power mode of the device to the specified power mode
* \param demod Pointer to demodulator instance.
* \param power mode.
* \return DRXStatus_t.
*
* Will be called by DRX3973D_Open().
* Special treatment for when called from DRX3973D_Open():
*  * Do not look at currentPowerMode
*  * Always powerup device
*  * Do not backup or restore registers
*
*/
static DRXStatus_t
CtrlPowerMode( pDRXDemodInstance_t demod,
               pDRXPowerMode_t     mode,
               DtDevice             *device )
{
   pI2CDeviceAddr_t devAddr=NULL;
   pDRX3973DData_t  extAttr=NULL;
   pDRXCommonAttr_t commonAttr=NULL;
   u16_t CcRegPwdMode=0;

   devAddr = demod -> myI2CDevAddr;
   extAttr = (pDRX3973DData_t)demod -> myExtAttr;
   commonAttr = (pDRXCommonAttr_t)demod -> myCommonAttr;

   /* Check arguments */
   if ( mode == NULL )
   {
      return (DRX_STS_INVALID_ARG);
   }

   /* If already in requested power mode, do nothing */
   if ( ( commonAttr->currentPowerMode == *mode ) &&
        ( commonAttr->isOpened == TRUE ) )
   {
      return (DRX_STS_OK);
   }

   // Check if device needs to be powered up
   if ( ( commonAttr->currentPowerMode != DRX_POWER_UP ) ||
        ( commonAttr->isOpened == FALSE ) )
   {
      // Need powered up device in order to change power mode
      // CHK_ERROR( DRXBSP_I2C_WriteRead( devAddr, 0, NULL, NULL, 0, NULL ) );
      
      // We're having issues writing null, so we just won't.  Take that Micronas!
      u8_t myVal = 0;
      pu8_t test;
      test = &myVal;
      
      CHK_ERROR( DRXBSP_I2C_WriteRead( devAddr, 1, test, NULL, 0, NULL, device) );

      // Wait for clocks to become stable, worst case: wakeup from deep sleep
      WaitMilliSec(6);
   }

   // commonAttr->isOpened = true;
   /* Translate power down mode to device powerdown mode */
   switch ( *mode )
   {
      case DRX3973D_POWER_UP:
         /* Power up device, no power saving:
            + all functional blocks are working
            + Pro version: 48 Mhz clock is offered on a pin
            + I2C bridge can be used
            + RAM & register content is not affected
            */
         CcRegPwdMode = CC_REG_PWD_MODE_UP;
         break;
      case DRX3973D_POWER_DOWN_CORE:

          /* Power down systemclock on device,
          - all functional blocks except the CC stop working
          + Pro version: 48 Mhz clock is offered on a pin
          + I2C bridge can be used if closed before this call
          + RAM & register content is not affected
          */
       CcRegPwdMode = CC_REG_PWD_MODE_DOWN_CLK;
         break;
      case DRX3973D_POWER_DOWN_PLL:
        /* Power down PLL on device,
           - all functional blocks except the CC stop working
           - Pro version: NO 48 Mhz clock is offered on a pin
           + I2C bridge can be used if closed before this call
           + RAM & register content is not affected
           */
        CcRegPwdMode = CC_REG_PWD_MODE_DOWN_PLL;
         break;
      case DRX3973D_POWER_DOWN:
         /* Power down OSC,
            -- ALL functional blocks stop working
            - Pro version: NO 48 Mhz clock is offered on a pin
            - I2C bridge can NOT be used if closed before this call
            + RAM & register content is not affected
            */
         CcRegPwdMode = CC_REG_PWD_MODE_DOWN_OSC;
         break;
      default:
         /* Unknow sleep mode */
         return (DRX_STS_ERROR);
         break;
   }


   if ( ( commonAttr->currentPowerMode == DRX_POWER_UP ) &&
        ( commonAttr->isOpened == TRUE ) )
   {
      /* Swicthing from POWER_UP to some power saving mode. */

      /* Backup some register settings */
      RR16( devAddr, EC_OC_REG_OC_MPG_SIO__A ,
                                          &(extAttr->EcOcRegOcMpgSio), 0x0000, device );
      /* FE_AG_REG_AG_AGC_SIO__A & FE_AG_REG_AG_PWD__A
         are already shadow registers */
   }

   if ( *mode == DRX_POWER_UP )
   {
      /* Restore some register settings */
      if ( commonAttr->isOpened == TRUE )
      {
         WR16( devAddr, EC_OC_REG_OC_MPG_SIO__A ,
                                           extAttr->EcOcRegOcMpgSio, 0x0000, device );
         WR16( devAddr, FE_AG_REG_AG_AGC_SIO__A,
                                           extAttr->FeAgRegAgAgcSio, 0x0000, device );
         WR16( devAddr, FE_AG_REG_AG_PWD__A, extAttr->FeAgRegAgPwd, 0x0000, device );
         WR16( devAddr, FE_AD_REG_PD__A, (~FE_AD_REG_PD_INIT), 0x0000, device );
      }
   } 
   else
   {
      /* Power down to requested mode */

      /* Set pins with possible pull-ups connected to them in input mode */
      WR16( devAddr, FE_AG_REG_AG_AGC_SIO__A,
                             FE_AG_REG_AG_AGC_SIO_INIT, 0x0000, device );
      WR16( devAddr, EC_OC_REG_OC_MPG_SIO__A ,
                             EC_OC_REG_OC_MPG_SIO_INIT, 0x0000, device );

      /* Analog power down */
      WR16( devAddr, FE_AG_REG_AG_PWD__A     , FE_AG_REG_AG_PWD_INIT, 0x0000, device);
      /* ADC power down */
      WR16( devAddr, FE_AD_REG_PD__A         , FE_AD_REG_PD_INIT    , 0x0000, device);

      WR16( devAddr, CC_REG_PWD_MODE__A, CcRegPwdMode, 0x0000, device);
      WR16( devAddr, CC_REG_UPDATE__A  , CC_REG_UPDATE_KEY, 0x0000, device);

      /* Power down device */
      extAttr -> HICfgCtrl |= HI_RA_RAM_SRV_CFG_ACT_PWD_EXE;
      CHK_ERROR( HI_CfgCommand( demod, device ) );
   }

   commonAttr->currentPowerMode = *mode;

   return ( DRX_STS_OK );

 rw_error:
    return (DRX_STS_ERROR);
}

/*============================================================================*/

/**
* \fn DRXStatus_t CtrlVersionInfo()
* \brief Report version of microcode and if possible version of device
* \param demod Pointer to demodulator instance.
* \param versionList Pointer to pointer of linked list of versions.
* \return DRXStatus_t.
*
* Using static structures so no allocation of memory is needed.
* Filling in all the fields each time, cause you don't know if they are
* changed by the application.
*
*/
static DRXStatus_t
CtrlVersionInfo( pDRXDemodInstance_t demod,
             pDRXVersionList_t   *versionList, DtDevice *device )
{

   pI2CDeviceAddr_t devAddr = demod -> myI2CDevAddr;

   static char ucodeName[]  = "Microcode";
   static char ucodeVersionText[] = "00.0.000";
   static DRXVersion_t ucodeVersion = {
      DRX_MODULE_MICROCODE,   /**< type identifier of the module */
      ucodeName,              /**< name or description of module */
      0,                      /**< major version number */
      0,                      /**< minor version number */
      0,                      /**< patch version number */
      ucodeVersionText        /**< version as text string */
   };
   static DRXVersionList_t ucodeVersionListElement = {
         &ucodeVersion,
         NULL };

   u16_t ucodeMajorMinor = 0; /* BCD xx:Ma:Ma:Mi */
   u16_t ucodePatch = 0;      /* BCD xx:Pa:Pa:Pa */

   RR16( devAddr, SC_IF_RAM_VERSION_MA_MI__A  , &ucodeMajorMinor, 0x0000, device);
   RR16( devAddr, SC_IF_RAM_VERSION_PATCH__A  , &ucodePatch, 0x0000, device);

   ucodeVersion.moduleType = DRX_MODULE_MICROCODE;
   ucodeVersion.moduleName = ucodeName;

   /* Translate BCD to numbers and string */
   ucodeVersion.vMinor     =   (ucodeMajorMinor & 0xF);
   ucodeVersion.vString[3] =   (ucodeMajorMinor & 0xF)+'0';
   ucodeMajorMinor         >>= 4;
   ucodeVersion.vMajor     =   (ucodeMajorMinor & 0xF);
   ucodeVersion.vString[2] =   '.';
   ucodeVersion.vString[1] =   (ucodeMajorMinor & 0xF)+'0';
   ucodeMajorMinor         >>= 4;
   ucodeVersion.vMajor     +=  (10* (ucodeMajorMinor & 0xF));
   ucodeVersion.vString[0] =   (ucodeMajorMinor & 0xF)+'0';
   ucodeVersion.vPatch     =   (ucodePatch & 0xF);
   ucodeVersion.vString[7] =   (ucodePatch & 0xF)+'0';
   ucodePatch              >>= 4;
   ucodeVersion.vPatch     +=  (10*(ucodePatch & 0xF));
   ucodeVersion.vString[6] =   (ucodePatch & 0xF)+'0';
   ucodePatch              >>= 4;
   ucodeVersion.vPatch     +=  (100*(ucodePatch & 0xF));
   ucodeVersion.vString[5] =   (ucodePatch & 0xF)+'0';
   ucodeVersion.vString[4] =   '.';

   ucodeVersionListElement.version = &ucodeVersion;
   ucodeVersionListElement.next = NULL;

   *versionList = &ucodeVersionListElement;

   return ( DRX_STS_OK );

 rw_error:
   return (DRX_STS_ERROR);
}

/*============================================================================*/

/**
* \fn DRXStatus_t CtrlProbeDevice()
* \brief Probe device, check if it is present
* \param demod Pointer to demodulator instance.
* \return DRXStatus_t.
*
* This funtion can be caled before open() and after close().
* The current implementation has as a side affect that, in case the device is
* currently not "opened", the HI will be reset.
*
*/
static DRXStatus_t
CtrlProbeDevice( pDRXDemodInstance_t demod, DtDevice *device )
{
   if ( demod->myCommonAttr->isOpened == FALSE )
   {
      pI2CDeviceAddr_t  devAddr=NULL;
#if !( VI2C_APPL || COMPILE_FOR_QT )
      DRXPowerMode_t    powerMode=DRX_POWER_UP;
#endif
      DRX3973DHiCmd_t   hiCmd;
      u16_t             result=0;

      devAddr    = demod -> myI2CDevAddr;

      /* Wake-up device */
#if ( VI2C_APPL || COMPILE_FOR_QT )
      demod->myCommonAttr->currentPowerMode = DRX_POWER_UP;
#else
      CHK_ERROR( CtrlPowerMode( demod, &powerMode, device ));
#endif
      /* Soft reset the HI, feedback from device */
      hiCmd.cmd = HI_RA_RAM_SRV_CMD_RESET;
      CHK_ERROR( HI_Command( devAddr, &hiCmd, &result, device) );

      /* Init HI for power down, feedback from device */
      CHK_ERROR( InitHI( demod, device ) );

      /* Device was not opened, powerdown device, feedback from device */
#if !( VI2C_APPL || COMPILE_FOR_QT )
      powerMode=DRX_POWER_DOWN;
      CHK_ERROR( CtrlPowerMode( demod, &powerMode, device ));
#endif
   }
   /* else device is already opened, just return OK */

   return ( DRX_STS_OK );

 rw_error:
   return (DRX_STS_ERROR);
}


static DRXStatus_t
CtrlI2CWriteRead( pDRXDemodInstance_t demod,
                  pDRXI2CData_t       i2cData )
{
#if 0
   printf("CtrlI2CWriteRead\n");
   printf("port %d\n", i2cData->portNr);
   printf("wDevAddr %p\n", (void *)i2cData->wDevAddr);
   printf("wCount %d\n", i2cData->wCount);
   printf("wData %p\n", (void *)i2cData->wData);
   printf("rDevAddr %p\n", (void *)i2cData->rDevAddr);
   printf("rCount %d\n", i2cData->rCount);
   printf("rData %p\n", (void *)i2cData->rData);

   return (DRX_STS_OK);
#endif

   return (DRX_STS_ERROR);
}
/*============================================================================*/
/*============================================================================*/
/*== CTRL Set/Get Config related functions ===================================*/
/*============================================================================*/
/*============================================================================*/

/**
* \fn DRXStatus_t CtrlSetCfgMPEGOutput()
* \brief Set 'some' configuration of the device.
* \param devmod  Pointer to demodulator instance.
* \param cfgData Pointer to nr of RS packets per measurement (u16_t).
* \return DRXStatus_t.
*
*  Configure MPEG output parameters.
*
*/
static DRXStatus_t
CtrlSetCfgMPEGOutput( pDRXDemodInstance_t   demod,
                      pDRXCfgMPEGOutput_t   cfgData, DtDevice *device )
{
   pI2CDeviceAddr_t devAddr=NULL;
   u16_t EcOcRegIprInvMpg = 0;
   u16_t EcOcRegOcModeLop = EC_OC_REG_OC_MODE_LOP_DTO_CTR_SRC_DYNAMIC;
   u16_t EcOcRegOcModeHip = 0;
   u16_t EcOcRegOcMpgSio  = 0;

   devAddr = demod -> myI2CDevAddr;

   if ( cfgData->enableMPEGOutput == TRUE )
   {
      EcOcRegOcMpgSio &= (~(EC_OC_REG_OC_MPG_SIO__M));
   }
   else
   {
      EcOcRegOcMpgSio |= EC_OC_REG_OC_MPG_SIO__M;
   }

   if ( cfgData->insertRSByte == TRUE )
   {
      EcOcRegOcModeLop &= (~(EC_OC_REG_OC_MODE_LOP_PAR_ENA__M));
      EcOcRegOcModeHip &= (~EC_OC_REG_OC_MODE_HIP_MPG_PAR_VAL__M);
      EcOcRegOcModeHip |= EC_OC_REG_OC_MODE_HIP_MPG_PAR_VAL_ENABLE;
   }
   else
   {
      EcOcRegOcModeLop |= EC_OC_REG_OC_MODE_LOP_PAR_ENA_DISABLE;
      EcOcRegOcModeHip &= (~EC_OC_REG_OC_MODE_HIP_MPG_PAR_VAL__M);
      EcOcRegOcModeHip |= EC_OC_REG_OC_MODE_HIP_MPG_PAR_VAL_DISABLE;
   }

   if ( cfgData->enableParallel == TRUE )
   {
      EcOcRegOcModeLop &= (~(EC_OC_REG_OC_MODE_LOP_MPG_TRM_MDE__M));
   }
   else
   {
      EcOcRegOcModeLop |= EC_OC_REG_OC_MODE_LOP_MPG_TRM_MDE_SERIAL;
   }

   if ( cfgData->invertDATA == TRUE )
   {
     EcOcRegIprInvMpg |= 0x00FF;
   }
   else
   {
     EcOcRegIprInvMpg &= (~(0x00FF));
   }

   if ( cfgData->invertERR == TRUE )
   {
     EcOcRegIprInvMpg |= 0x0100;
   }
   else
   {
     EcOcRegIprInvMpg &= (~(0x0100));
   }

   if ( cfgData->invertSTR == TRUE )
   {
     EcOcRegIprInvMpg |= 0x0200;
   }
   else
   {
     EcOcRegIprInvMpg &= (~(0x0200));
   }

   if ( cfgData->invertVAL == TRUE )
   {
     EcOcRegIprInvMpg |= 0x0400;
   }
   else
   {
     EcOcRegIprInvMpg &= (~(0x0400));
   }

   if ( cfgData->invertCLK == TRUE )
   {
     EcOcRegIprInvMpg |= 0x0800;
   }
   else
   {
     EcOcRegIprInvMpg &= (~(0x0800));
   }

   WR16 ( devAddr, EC_OC_REG_IPR_INV_MPG__A, EcOcRegIprInvMpg , 0x0000, device );
   WR16 ( devAddr, EC_OC_REG_OC_MODE_LOP__A, EcOcRegOcModeLop , 0x0000, device );
   WR16 ( devAddr, EC_OC_REG_OC_MODE_HIP__A, EcOcRegOcModeHip , 0x0000, device );
   WR16 ( devAddr, EC_OC_REG_OC_MPG_SIO__A , EcOcRegOcMpgSio  , 0x0000, device );

   return (DRX_STS_OK);

 rw_error:
   return (DRX_STS_ERROR);
}

/*============================================================================*/

/**
* \fn DRXStatus_t CtrlGetCfgMPEGOutput()
* \brief Get MPEG output configuration of the device.
* \param devmod  Pointer to demodulator instance.
* \param cfgData Pointer to nr of RS packets per measurement (u16_t).
* \return DRXStatus_t.
*
*  Retrieve MPEG output configuartion.
*
*/
static DRXStatus_t
CtrlGetCfgMPEGOutput( pDRXDemodInstance_t   demod,
                      pDRXCfgMPEGOutput_t   cfgData, DtDevice *device )
{
   pI2CDeviceAddr_t devAddr=NULL;
   u16_t EcOcRegIprInvMpg = 0;
   u16_t EcOcRegOcModeLop = 0;
   u16_t EcOcRegOcMpgSio  = 0;

   devAddr = demod -> myI2CDevAddr;

   RR16 ( devAddr, EC_OC_REG_OC_MPG_SIO__A , &EcOcRegOcMpgSio  , 0x0000, device );
   RR16 ( devAddr, EC_OC_REG_OC_MODE_LOP__A, &EcOcRegOcModeLop , 0x0000, device );
   RR16 ( devAddr, EC_OC_REG_IPR_INV_MPG__A, &EcOcRegIprInvMpg , 0x0000, device );

   if ( (EcOcRegOcMpgSio & EC_OC_REG_OC_MPG_SIO__M)==EC_OC_REG_OC_MPG_SIO__M )
   {
      cfgData->enableMPEGOutput = FALSE;
   }
   else
   {
      cfgData->enableMPEGOutput = TRUE;
   }

   if ( (EcOcRegOcModeLop & EC_OC_REG_OC_MODE_LOP_PAR_ENA_DISABLE) ==
        EC_OC_REG_OC_MODE_LOP_PAR_ENA_DISABLE )
   {
      cfgData->insertRSByte = FALSE;
   }
   else
   {
      cfgData->insertRSByte = TRUE;
   }

   if ( (EcOcRegOcModeLop & EC_OC_REG_OC_MODE_LOP_MPG_TRM_MDE_SERIAL) ==
         EC_OC_REG_OC_MODE_LOP_MPG_TRM_MDE_SERIAL )
   {
      cfgData->enableParallel = FALSE;
   }
   else
   {
      cfgData->enableParallel = TRUE;
   }

   if ( (EcOcRegIprInvMpg & 0x00FF) == 0x00FF )
   {
      cfgData->invertDATA = TRUE;
   }
   else
   {
      cfgData->invertDATA = FALSE;
   }

   if ( (EcOcRegIprInvMpg & 0x0100) == 0x0100 )
   {
     cfgData->invertERR = TRUE;
   }
   else
   {
     cfgData->invertERR = FALSE;
   }

   if ( ( EcOcRegIprInvMpg & 0x0200 ) == 0x0200 )
   {
      cfgData->invertSTR = TRUE;
   }
   else
   {
      cfgData->invertSTR = FALSE;
   }

   if ( ( EcOcRegIprInvMpg & 0x0400 ) == 0x0400 )
   {
      cfgData->invertVAL = TRUE;
   }
   else
   {
      cfgData->invertVAL = FALSE;
   }

   if ( ( EcOcRegIprInvMpg & 0x0800 ) == 0x0800 )
   {
      cfgData->invertCLK = TRUE;
   }
   else
   {
      cfgData->invertCLK = FALSE;
   }

   return (DRX_STS_OK);

 rw_error:
   return (DRX_STS_ERROR);
}

/*============================================================================*/

/**
* \fn DRXStatus_t CtrlSetCfgPostViterbi()
* \brief Set 'some' configuration of the device.
* \param devmod  Pointer to demodulator instance.
* \param cfgData Pointer to nr of RS packets per measurement (u16_t).
* \return DRXStatus_t.
*
* Set number of RS packets processed per measurement.
* This setting wil be lost and set to 0xFF after a reset of EC(_RS)
*
*/
static DRXStatus_t
CtrlSetCfgPostViterbi( pDRXDemodInstance_t   demod,
                       void*                 cfgData, DtDevice *device )
{
   /* TODO precondition: device is running and in lock ? */
   WR16( demod->myI2CDevAddr, EC_RS_REG_REQ_PCK_CNT__A, *((pu16_t)cfgData), 0x0000, device);

   return (DRX_STS_OK);

 rw_error:
   return (DRX_STS_ERROR);
}

/*============================================================================*/

/**
* \fn DRXStatus_t CtrlSetCfgPreViterbi()
* \brief Set 'some' configuration of the device.
* \param devmod  Pointer to demodulator instance.
* \param cfgData Pointer to config params for preViterbi measurement.
* \return DRXStatus_t.
*
* Set number of symbols or number of bits processed per measurement.
* Range number of bits: 0x0000 - 0xFFFF
* Range number of symbols depends on FFT mode and constellation.
* ( Carriers/symbol: 1512 for 2K, 6048 for 8K FFT.)
* ( Bits/carrier: 2 for QPSK, 3 for QAM16, 6 for QAM64)
*
* Mode | Const | Max symbols (before overflow in EC_VD_REG_IN_BIT_CNT__A)
*------------------------------------------------------------
*  2   |  QPSK |  21  = ((2^16)-1)/3024
*  2   |  QAM16|  10  = ((2^16)-1)/6048
*  2   |  QAM64|  5   = ((2^16)-1)/12096
*  8   |  QPSK |  5   = ((2^16)-1)/12096
*  8   |  QAM16|  2   = ((2^16)-1)/24192
*  8   |  QAM64|  1   = ((2^16)-1)/48384
*
* if EC_VD_REG_REQ_SMB_CNT__A is set to zero then EC_VD_REG_REQ_BIT_CNT__A
* will be used, otherwise EC_VD_REG_REQ_SMB_CNT__A will be used.
*
*/
static DRXStatus_t
CtrlSetCfgPreViterbi( pDRXDemodInstance_t   demod,
                      void*                 cfgData )
{
   /* TODO */
   /* TODO precondition: device is running and in lock ? */
   /* Check maxima in case of per Symbol measurement */

   return (DRX_STS_OK);
#if 0
 rw_error:
   return (DRX_STS_ERROR);
#endif
}

/*============================================================================*/

/**
* \fn DRXStatus_t CtrlSetSysClk2Pin17()
* \brief Let PIN17 of DRX3973D output the SysClk.
* \param devmod  Pointer to demodulator instance.
* \return DRXStatus_t.
*
*/

static DRXStatus_t
CtrlSetSysClk2Pin17( pDRXDemodInstance_t   demod, DtDevice *device )
{

   pI2CDeviceAddr_t devAddr=NULL;

   devAddr = demod -> myI2CDevAddr;

   WR16 ( devAddr,
          EC_OC_REG_OC_MPG_SIO__A ,
          ( EC_OC_REG_OC_MPG_SIO__M ^ EC_OC_REG_OC_MPG_SIO_MPG_SIO_11_INPUT),
          0x0000, device );
   WR16 ( devAddr,
          EC_OC_REG_OC_MODE_HIP__A,
          EC_OC_REG_OC_MODE_HIP_MPG_BUS_SRC_MONITOR,
          0x0000, device );
   WR16 ( devAddr,
          EC_OC_REG_OCR_INV_MON__A,
          EC_OC_REG_OCR_INV_MON_INIT,
          0x0000, device );

   return (DRX_STS_OK);

 rw_error:
   return (DRX_STS_ERROR);
}

/*============================================================================*/

/**
* \fn DRXStatus_t CtrlSetCfgIfAgc()
* \brief Configure IF AGC control.
* \param demod  Pointer to demodulator instance.
* \param cfg    Pointer to configuration data.
*
* User controlled IF AGC
*
*  IF AGC can be directly controled by writing a value to the DAC input.
*  In this case use:
*  ctrlMode=DRX3973D_AGC_CTRL_USER, Level=<DAC input value>
*  The rest of the DRX3973DCfgAgc_t structure fields are don't cares.
*
* Device controled IF AGC
*
*  IF AGC can be controled by the device. The device needs some parameters to
*  perform this control:
*  CtrlMode=DRX3973D_AGC_CTRL_AUTO
*  Level: desired IF settle level
*  MinOuputLevel: minimum output level to DAC
*  MaxOuputLevel: maximum output level to DAC
*  Speed: dBmV per time unit
*
* \return DRXStatus_t.
*
*/
static DRXStatus_t
CtrlSetCfgIfAgc( pDRXDemodInstance_t   demod, pDRX3973DCfgAgc_t cfg, DtDevice *device )
{
   pI2CDeviceAddr_t devAddr=NULL;
   pDRX3973DData_t extAttr=NULL;

   devAddr = demod -> myI2CDevAddr;
   extAttr    = (pDRX3973DData_t) demod -> myExtAttr;

   if ( cfg->ctrlMode == DRX3973D_AGC_CTRL_USER )
   {
      /* User control */

      u16_t AgModeLop=0;
      u16_t level=0;

      /*==== Check ====*/
      if ( cfg->outputLevel > DRX3973D_FE_CTRL_MAX )
      {
         return (DRX_STS_INVALID_ARG);
      }

      /*==== Mode ====*/

      RR16 ( devAddr,
             FE_AG_REG_AG_MODE_LOP__A,
             &AgModeLop,
             0x0000, device );
      AgModeLop &= (~FE_AG_REG_AG_MODE_LOP_MODE_4__M);
      AgModeLop |= FE_AG_REG_AG_MODE_LOP_MODE_4_STATIC;
      WR16 ( devAddr,
             FE_AG_REG_AG_MODE_LOP__A,
             AgModeLop,
             0x0000, device );

      /*==== Ouput level ====*/

      level = ( ( cfg->outputLevel ) & FE_AG_REG_PM1_AGC_WRI__M );
      WR16 ( devAddr,
             FE_AG_REG_PM1_AGC_WRI__A,
             level,
             0x0000, device );
   }
   else if ( cfg->ctrlMode == DRX3973D_AGC_CTRL_AUTO )
   {
      /* Automatic control */

      u16_t AgModeLop=0;
      u16_t level=0;
      u16_t slope=0;
      u16_t offset=0;
      u16_t fastIncrDec=0;
      u16_t slowIncrDec=0;
      u16_t rurCount=0;
      u16_t invRurCount=0;
      u16_t speed = (cfg->speed);
      u16_t fineSteps = 0;
      u16_t fineSpeed = 0;
      const u16_t maxRur = 8;
      static const u16_t slowIncrDecLUT[]={ 3, 4, 4, 5, 6 };
      static const u16_t fastIncrDecLUT[]={ 14, 15, 15, 16,
                                            17, 18, 18, 19,
                                            20, 21, 22, 23,
                                            24, 26, 27, 28,
                                            29, 31};


      /*==== Check ====*/

      if ( ( (cfg->maxOutputLevel) < (cfg->minOutputLevel) ) ||
           ( (cfg->maxOutputLevel) > DRX3973D_FE_CTRL_MAX ) ||
           ( (cfg->speed) > DRX3973D_FE_CTRL_MAX ) ||
           ( (cfg->settleLevel) > DRX3973D_FE_CTRL_MAX )
         )
      {
         return (DRX_STS_INVALID_ARG);
      }

      /*==== Mode ====*/

      RR16 ( devAddr,
             FE_AG_REG_AG_MODE_LOP__A,
             &AgModeLop,
             0x0000, device );
      AgModeLop &= (~FE_AG_REG_AG_MODE_LOP_MODE_4__M);
      AgModeLop |= FE_AG_REG_AG_MODE_LOP_MODE_4_DYNAMIC;
      WR16 ( devAddr,
             FE_AG_REG_AG_MODE_LOP__A,
             AgModeLop,
             0x0000, device );

      /*==== Settle level ====*/

      level = ( (( cfg->settleLevel )>>1) & FE_AG_REG_EGC_SET_LVL__M );
      WR16 ( devAddr,
             FE_AG_REG_EGC_SET_LVL__A,
             level,
             0x0000, device );

      /*==== Min/max ====*/

      slope = ( ( cfg->maxOutputLevel ) - ( cfg->minOutputLevel ) )/2;
      offset = ( ( cfg->maxOutputLevel ) + ( cfg->minOutputLevel ) )/2;
      offset -= 511;
      WR16 ( devAddr,
             FE_AG_REG_GC1_AGC_RIC__A,
             slope,
             0x0000, device );
      WR16 ( devAddr,
             FE_AG_REG_GC1_AGC_OFF__A,
             offset,
             0x0000, device );

      /*==== Speed ====*/

      fineSteps  = (DRX3973D_FE_CTRL_MAX+1)/(maxRur+1);
      fineSpeed  = speed - ((speed/fineSteps)*fineSteps);
      invRurCount= speed / fineSteps;
      if ( invRurCount > maxRur )
      {
         rurCount   = 0;
         fineSpeed += fineSteps;
      } else {
         rurCount   = maxRur - invRurCount;
      }

      /*
        fastInc = default * (2^(fineSpeed/fineSteps) )
            => range[default...2*default>
        slowInc = default * (2^(fineSpeed/fineSteps) )
      */
      fastIncrDec = fastIncrDecLUT[ fineSpeed/((fineSteps/(14+1))+1) ]; /*rounding*/
      slowIncrDec = slowIncrDecLUT[ fineSpeed/(fineSteps/(3+1)) ];
      WR16 ( devAddr,
             FE_AG_REG_EGC_RUR_CNT__A,
             rurCount,
             0x0000, device );
      WR16 ( devAddr,
             FE_AG_REG_EGC_FAS_INC__A,
             fastIncrDec,
             0x0000, device );
      WR16 ( devAddr,
             FE_AG_REG_EGC_FAS_DEC__A,
             fastIncrDec,
             0x0000, device );
      WR16 ( devAddr,
             FE_AG_REG_EGC_SLO_INC__A,
             slowIncrDec,
             0x0000, device );
      WR16 ( devAddr,
             FE_AG_REG_EGC_SLO_DEC__A,
             slowIncrDec,
             0x0000, device );
   }
   else
   {
      /* No OFF mode for IF control */
      return (DRX_STS_INVALID_ARG);
   }

   /* Store current configuration */
   extAttr->IfAgcCfg = *cfg;

   return (DRX_STS_OK);

 rw_error:
   return (DRX_STS_ERROR);
}

/*============================================================================*/

/**
* \fn DRXStatus_t CtrlGetCfgIfAgc()
* \brief Retrieve IF AGC control configuaration.
* \param demod  Pointer to demodulator instance.
* \param cfg    Pointer to configuration data.
*
* \return DRXStatus_t.
*
*/
static DRXStatus_t
CtrlGetCfgIfAgc( pDRXDemodInstance_t   demod, pDRX3973DCfgAgc_t cfg, DtDevice *device )
{
   pI2CDeviceAddr_t devAddr=NULL;
   pDRX3973DData_t extAttr=NULL;

   devAddr = demod -> myI2CDevAddr;
   extAttr    = (pDRX3973DData_t) demod -> myExtAttr;

   *cfg = (extAttr -> IfAgcCfg);
   if ( cfg->ctrlMode == DRX3973D_AGC_CTRL_AUTO )
   {
      RR16 ( devAddr, FE_AG_REG_GC1_AGC_DAT__A, &(cfg->outputLevel), 0x0000 , device);
   }

   return (DRX_STS_OK);

 rw_error:
   return (DRX_STS_ERROR);
}

/*============================================================================*/

/**
* \fn DRXStatus_t CtrlSetCfgRFAgc()
* \brief Configure RF AGC control.
* \param demod  Pointer to demodulator instance.
* \param cfg    Pointer to configuration data.
*
* Under construction
*
*  Currently only two choices are available:
*  * DEFAULT: Switch off RF AGC (use CtrlMode=DRX3973D_AGC_CTRL_OFF)
*  * Switch on fixed RF AGC (use CtrlMode=DRX3973D_AGC_CTRL_AUTO)
*
* The other other fields of the DRX3973DCfgAgc_t are currently not used.
*
* \return DRXStatus_t.
*
*/
static DRXStatus_t
CtrlSetCfgRfAgc( pDRXDemodInstance_t   demod, pDRX3973DCfgAgc_t cfg, DtDevice *device )
{
   pI2CDeviceAddr_t devAddr=NULL;
   pDRX3973DData_t extAttr=NULL;

   devAddr = demod -> myI2CDevAddr;
   extAttr    = (pDRX3973DData_t) demod -> myExtAttr;

   switch ( cfg->ctrlMode )
   {
      case DRX3973D_AGC_CTRL_USER:
      {
         /* User control */

         u16_t AgModeLop=0;
         u16_t level=0;

         /*==== Check ====*/
         if ( cfg->outputLevel > DRX3973D_FE_CTRL_MAX )
         {
            return (DRX_STS_INVALID_ARG);
         }

         /*==== Ouput level ====*/

         level = ( cfg->outputLevel );
         if (level == DRX3973D_FE_CTRL_MAX )
         {
            level++;
         }

         WR16 ( devAddr,
                FE_AG_REG_PM2_AGC_WRI__A,
                level,
                0x0000, device );

         /*==== Mode ====*/

         /* Powerdown PD2, WRI source */
         (extAttr -> FeAgRegAgPwd) &= ~(FE_AG_REG_AG_PWD_PWD_PD2__M);
         (extAttr -> FeAgRegAgPwd) |= FE_AG_REG_AG_PWD_PWD_PD2_DISABLE;
         WR16 ( devAddr,
                FE_AG_REG_AG_PWD__A,
                (extAttr -> FeAgRegAgPwd),
                0x0000, device );

         RR16 ( devAddr,
                FE_AG_REG_AG_MODE_LOP__A,
                &AgModeLop,
                0x0000, device );
         AgModeLop &= (~( FE_AG_REG_AG_MODE_LOP_MODE_5__M |
                          FE_AG_REG_AG_MODE_LOP_MODE_E__M));
         AgModeLop |= ( FE_AG_REG_AG_MODE_LOP_MODE_5_STATIC |
                        FE_AG_REG_AG_MODE_LOP_MODE_E_STATIC );
         WR16 ( devAddr,
               FE_AG_REG_AG_MODE_LOP__A,
               AgModeLop,
               0x0000, device );

         break;
      }

      case DRX3973D_AGC_CTRL_AUTO:
      {
         u16_t AgModeLop=0;

         /* Automatic control */
         /* Powerup PD2, AGC2 as output, TGC source */
         (extAttr -> FeAgRegAgPwd) &= ~(FE_AG_REG_AG_PWD_PWD_PD2__M);
         (extAttr -> FeAgRegAgPwd) |= FE_AG_REG_AG_PWD_PWD_PD2_DISABLE;
         WR16 ( devAddr,
                FE_AG_REG_AG_PWD__A,
                (extAttr -> FeAgRegAgPwd),
                0x0000, device );

         RR16 ( devAddr,
                FE_AG_REG_AG_MODE_LOP__A,
                &AgModeLop,
                0x0000, device );
         AgModeLop &= (~( FE_AG_REG_AG_MODE_LOP_MODE_5__M |
                          FE_AG_REG_AG_MODE_LOP_MODE_E__M));
         AgModeLop |= ( FE_AG_REG_AG_MODE_LOP_MODE_5_STATIC |
                        FE_AG_REG_AG_MODE_LOP_MODE_E_DYNAMIC );
         WR16 ( devAddr,
               FE_AG_REG_AG_MODE_LOP__A,
               AgModeLop,
               0x0000, device );

         break;
      }

      case DRX3973D_AGC_CTRL_OFF:
      {
         u16_t AgModeLop=0;

         /* No RF AGC control */
         /* Powerdown PD2, WRI source */
         (extAttr -> FeAgRegAgPwd) &= ~(FE_AG_REG_AG_PWD_PWD_PD2__M);
         (extAttr -> FeAgRegAgPwd) |= FE_AG_REG_AG_PWD_PWD_PD2_ENABLE;
         WR16 ( devAddr,
                FE_AG_REG_AG_PWD__A,
                (extAttr -> FeAgRegAgPwd),
                0x0000, device );

         RR16 ( devAddr,
                FE_AG_REG_AG_MODE_LOP__A,
                &AgModeLop,
                0x0000, device );
         AgModeLop &= (~( FE_AG_REG_AG_MODE_LOP_MODE_5__M |
                          FE_AG_REG_AG_MODE_LOP_MODE_E__M));
         AgModeLop |= ( FE_AG_REG_AG_MODE_LOP_MODE_5_STATIC |
                        FE_AG_REG_AG_MODE_LOP_MODE_E_STATIC );
         WR16 ( devAddr,
               FE_AG_REG_AG_MODE_LOP__A,
               AgModeLop,
               0x0000, device );
         WR16 ( devAddr,
               FE_AG_REG_PM2_AGC_WRI__A,
               FE_AG_REG_PM2_AGC_WRI_INIT,
               0x0000, device );

         break;
      }
      default:
      {
         return (DRX_STS_INVALID_ARG);
         break;
      }
   }

   /* Store current configuration */
   extAttr->RfAgcCfg = *cfg;

   return (DRX_STS_OK);

 rw_error:
   return (DRX_STS_ERROR);
}

/*============================================================================*/

/**
* \fn DRXStatus_t CtrlGetCfgRfAgc()
* \brief Retrieve RF AGC control configuaration.
* \param demod  Pointer to demodulator instance.
* \param cfg    Pointer to configuration data.
*
* \return DRXStatus_t.
*
*/
static DRXStatus_t
CtrlGetCfgRfAgc( pDRXDemodInstance_t   demod, pDRX3973DCfgAgc_t cfg )
{
   pDRX3973DData_t extAttr=NULL;

   extAttr    = (pDRX3973DData_t) demod -> myExtAttr;

   *cfg = (extAttr -> RfAgcCfg);

   return (DRX_STS_OK);
}

/*============================================================================*/

/**
* \fn DRXStatus_t CtrlSetCfgImpulsiveNoise()
* \brief Turn on/off impulsive noise killer.
* \param demod  Pointer to demodulator instance.
* \param cfg    Pointer to boolean, FALSE for OFF, TRUE for ON.
*
* \return DRXStatus_t.
*
*/
static DRXStatus_t
CtrlSetCfgImpulsiveNoise( pDRXDemodInstance_t   demod, pBool_t killerSwitch, DtDevice *device )
{
   pI2CDeviceAddr_t devAddr = NULL;
   u16_t            val = 0;

   devAddr    = demod -> myI2CDevAddr;
   val= (u16_t)(*killerSwitch == TRUE );
   /* Switch impulsive noise killer on or off */
   WR16( devAddr, FE_CF_REG_IMP_VAL__A, val, 0x0000, device);

   return (DRX_STS_OK);

 rw_error:
   return (DRX_STS_ERROR);

}

/*============================================================================*/

/**
* \fn DRXStatus_t CtrlGetCfgImpulsiveNoise()
* \brief Retrieve impulsive noise killer setting.
* \param demod  Pointer to demodulator instance.
* \param cfg    Pointer to boolean, FALSE for OFF, TRUE for ON.
*
* \return DRXStatus_t.
*
*/
static DRXStatus_t
CtrlGetCfgImpulsiveNoise( pDRXDemodInstance_t   demod, pBool_t killerSwitch, DtDevice *device )
{
   pI2CDeviceAddr_t devAddr = NULL;
   u16_t            val = 0;

   devAddr    = demod -> myI2CDevAddr;

   RR16( devAddr, FE_CF_REG_IMP_VAL__A, &val, 0x0000, device);
   *killerSwitch = (Bool_t)(val==1);

   return (DRX_STS_OK);

 rw_error:
   return (DRX_STS_ERROR);

}

/*============================================================================*/

/**
* \fn DRXStatus_t CtrlSetCfgPGA()
* \brief Configure PGA, OFF (==bypass) or ON.
* \param demod  Pointer to demodulator instance.
* \param cfg    Pointer to boolean, FALSE for OFF, TRUE for ON.
*
* \return DRXStatus_t.
*
*/
static DRXStatus_t
CtrlSetCfgPGA( pDRXDemodInstance_t demod, pBool_t pgaSwitch, DtDevice *device )
{
   u16_t AgModeLop = 0;
   u16_t AgPgaMode = 0;
   u16_t BgcCgcWri = 0;
   u16_t IndPd1Wri = 0;

   pI2CDeviceAddr_t devAddr = NULL;

   devAddr    = demod -> myI2CDevAddr;

   if ( *pgaSwitch == TRUE )
   {
      /* PGA on */
      /* FE_AG_REG_AG_MODE_HIP__A stays 0, hardware init value */

      RR16( devAddr, FE_AG_REG_AG_MODE_LOP__A, &AgModeLop, 0x0000, device);
      AgModeLop&=(~(FE_AG_REG_AG_MODE_LOP_MODE_C__M));
      AgModeLop|= FE_AG_REG_AG_MODE_LOP_MODE_C_DYNAMIC;
      WR16( devAddr, FE_AG_REG_AG_MODE_LOP__A, AgModeLop, 0x0000, device);

      AgPgaMode=FE_AG_REG_AG_PGA_MODE_INIT;
      WR16( devAddr, FE_AG_REG_AG_PGA_MODE__A, AgPgaMode, 0x0000, device);

      BgcCgcWri = 1; /* 10 dB amplification */
      WR16( devAddr, FE_AG_REG_BGC_CGC_WRI__A, BgcCgcWri, 0x0000, device);

      IndPd1Wri = 27;
      WR16( devAddr, FE_AG_REG_IND_PD1_WRI__A, IndPd1Wri, 0x0000, device);
   } else {
      /* PGA off, bypass */
      /* FE_AG_REG_AG_MODE_HIP__A stays 0, hardware init value */

      RR16( devAddr, FE_AG_REG_AG_MODE_LOP__A, &AgModeLop, 0x0000, device);
      AgModeLop&=(~(FE_AG_REG_AG_MODE_LOP_MODE_C__M));
      AgModeLop|= FE_AG_REG_AG_MODE_LOP_MODE_C_STATIC ;
      WR16( devAddr, FE_AG_REG_AG_MODE_LOP__A, AgModeLop, 0x0000, device);

      AgPgaMode=FE_AG_REG_AG_PGA_MODE_PFN_PCN_AFY_REN;
      WR16( devAddr, FE_AG_REG_AG_PGA_MODE__A, AgPgaMode, 0x0000, device);

      BgcCgcWri = 0; /* 0 dB amplification */
      WR16( devAddr, FE_AG_REG_BGC_CGC_WRI__A, BgcCgcWri, 0x0000, device);

      IndPd1Wri = 35;
      WR16( devAddr, FE_AG_REG_IND_PD1_WRI__A, IndPd1Wri, 0x0000, device);
   }

   return (DRX_STS_OK);

 rw_error:
   return (DRX_STS_ERROR);

}

/*============================================================================*/

/**
* \fn DRXStatus_t CtrlGetCfgPGA()
* \brief Retrieve config PGA, OFF (==bypass) or ON.
* \param demod  Pointer to demodulator instance.
* \param cfg    Pointer to boolean, FALSE for OFF, TRUE for ON.
*
* \return DRXStatus_t.
*
*/
static DRXStatus_t
CtrlGetCfgPGA( pDRXDemodInstance_t demod, pBool_t pgaSwitch, DtDevice *device )
{
   u16_t AgPgaMode = 0;
   pI2CDeviceAddr_t devAddr = NULL;

   devAddr    = demod -> myI2CDevAddr;

   RR16( devAddr, FE_AG_REG_AG_PGA_MODE__A, &AgPgaMode, 0x0000, device);
   *pgaSwitch = (Bool_t) ( AgPgaMode==FE_AG_REG_AG_PGA_MODE_INIT );

   return (DRX_STS_OK);

 rw_error:
   return (DRX_STS_ERROR);

}

/*============================================================================*/

/**
* \fn DRXStatus_t CtrlSetCfg()
* \brief Set 'some' configuration of the device.
* \param devmod Pointer to demodulator instance.
* \param config Pointer to configuration parameters (type and data).
* \return DRXStatus_t.

*/
static DRXStatus_t
CtrlSetCfg( pDRXDemodInstance_t   demod,
            pDRXCfg_t             config, DtDevice *device )
{


   if ( config == NULL )
   {
      return (DRX_STS_INVALID_ARG);
   }

   /* Configurations that do not need config data */

   switch ( config->cfgType )
   {
      case DRX3973D_CFG_SYSCLK2PIN17 :
         return CtrlSetSysClk2Pin17( demod, device );
         break;
      default:
         /* Do Nothing , unknowns will be detected in the next switch */
         break;
   }

   /* Configurations that need config data */

   if ( config->cfgData == NULL )
   {
      return (DRX_STS_INVALID_ARG);
   }

   switch ( config->cfgType )
   {
      case DRX_CFG_MPEG_OUTPUT:
         return CtrlSetCfgMPEGOutput( demod,
                                      (pDRXCfgMPEGOutput_t) config->cfgData, device );
         break;

      case DRX3973D_CFG_POSTVITERBI :
         return CtrlSetCfgPostViterbi( demod, config->cfgData, device );
         break;

      case DRX3973D_CFG_PREVITERBI :
         return CtrlSetCfgPreViterbi( demod,
                                      (pDRX3973DCfgPreViterbi_t)config->cfgData );
         break;

      case DRX3973D_CFG_IF_AGC :
         return CtrlSetCfgIfAgc( demod,
                                 (pDRX3973DCfgAgc_t)config->cfgData, device );
         break;

      case DRX3973D_CFG_RF_AGC :
         return CtrlSetCfgRfAgc( demod,
                                 (pDRX3973DCfgAgc_t)config->cfgData, device );
         break;

      case DRX3973D_CFG_SC_CMD :
         return ScCommand( demod->myI2CDevAddr,
                           (pDRX3973DScCmd_t)config->cfgData, device );

      case DRX3973D_CFG_IMP_NOISE :
         return CtrlSetCfgImpulsiveNoise( demod, (pBool_t)config->cfgData, device );
         break;

      case DRX3973D_CFG_PGA :
         return CtrlSetCfgPGA( demod, (pBool_t)config->cfgData, device );
         break;

      default:
         return (DRX_STS_INVALID_ARG);
         break;
   }

   return (DRX_STS_OK);
}

/*============================================================================*/

/**
* \fn DRXStatus_t CtrlGetCfg()
* \brief Get 'some' configuration of the device.
* \param devmod Pointer to demodulator instance.
* \param config Pointer to configuration parameters (type and data).
* \return DRXStatus_t.

*/
static DRXStatus_t
CtrlGetCfg( pDRXDemodInstance_t   demod,
            pDRXCfg_t             config, DtDevice *device )
{

   if ( config == NULL ||
        config->cfgData == NULL
       )
   {
      return (DRX_STS_INVALID_ARG);
   }

   switch ( config->cfgType )
   {
      case DRX_CFG_MPEG_OUTPUT:
         return CtrlGetCfgMPEGOutput( demod,
                                      (pDRXCfgMPEGOutput_t) config->cfgData, device );
         break;

      case DRX3973D_CFG_IF_AGC:
         return CtrlGetCfgIfAgc( demod,
                                      (pDRX3973DCfgAgc_t)config->cfgData, device );
         break;

      case DRX3973D_CFG_RF_AGC:
         return CtrlGetCfgRfAgc( demod,
                                      (pDRX3973DCfgAgc_t)config->cfgData );
         break;

      case DRX3973D_CFG_IMP_NOISE :
         return CtrlGetCfgImpulsiveNoise( demod, (pBool_t)config->cfgData, device );
         break;

      case DRX3973D_CFG_PGA :
         return CtrlGetCfgPGA( demod, (pBool_t)config->cfgData, device );
         break;

      default:
         return (DRX_STS_INVALID_ARG);
         break;
   }

   return (DRX_STS_OK);
}

/*=============================================================================
  =============================================================================
  ===== EXPORTED FUNCTIONS ====================================================
  =============================================================================
  ===========================================================================*/


/**
* \fn DRX3973D_Open()
* \brief Open demodulator instance 'demod'.
* \return Status_t Return status.
*
* Pre conditions (checked by DRX_Open()):
* demod                         != NULL
* demod->myDemodFunct           != NULL
* demod->myCommonAttr           != NULL
* demod->myI2CDevAddr           != NULL
* demod->myCommonAttr->isOpened != TRUE
*
*/
DRXStatus_t DRX3973D_Open(pDRXDemodInstance_t demod, DtDevice *device)
{
   DRXUCodeInfo_t ucodeInfo;
   DRXPowerMode_t   powerMode=DRX3973D_POWER_UP;
   pI2CDeviceAddr_t devAddr=NULL;
   pDRX3973DData_t extAttr=NULL;
   pDRXCommonAttr_t commonAttr=NULL;
   DRXCfgMPEGOutput_t cfgMPEGOutput;
   DRX3973DHiCmd_t hiCmd;
   u16_t result=0;
   u16_t scFlags = 0;
   DRXFrequency_t deviation = 0;
   DRXFrequency_t devA = 0;   /* helper variable */
   DRXFrequency_t devB = 0;   /* helper variable */

   /* Check arguments */
   if (demod -> myExtAttr == NULL )
   {
      return ( DRX_STS_INVALID_ARG);
   }

   devAddr    = demod -> myI2CDevAddr;
   extAttr    = (pDRX3973DData_t) demod -> myExtAttr;
   commonAttr = (pDRXCommonAttr_t) demod -> myCommonAttr;

   /* Power up */
#if !( VI2C_APPL || COMPILE_FOR_QT )
   
   CHK_ERROR( CtrlPowerMode( demod, &powerMode, device ));
#else
   commonAttr->currentPowerMode = DRX_POWER_UP;
#endif


   /* Soft reset the HI */
   hiCmd.cmd    = HI_RA_RAM_SRV_CMD_RESET;
   CHK_ERROR( HI_Command( devAddr, &hiCmd, &result, device) );
#if (DRXD_TYPE_A)
   /* HI firmware patch for UIO readout, avoid clearing of result register */
   WR16( devAddr, 0x43012D, 0x047f, 0x0000, device );
#endif

   /* Stop all processors off the device */
   CHK_ERROR( StopAllProcessors( devAddr, device ) );

   /* Clock controler, set ossilator and system clocks */
   CHK_ERROR( InitCC( demod, device ) );

   /* Handle clock deviation */
   devA = ((DRXFrequency_t)(commonAttr->oscClockDeviation)) *
           (extAttr->expectedSysClockFreq);
   deviation = ( devA /((DRXFrequency_t)1000000L)); /* deviation in kHz */
   /* rounding, signed */
   if ( devA > 0 )
   {
      devB=((DRXFrequency_t)2);
   } else {
      devB=((DRXFrequency_t)-2);
   }
   if ( (devB*(devA%1000000L)>1000000L ) )
   {
      /* add +1 or -1 */
      deviation += (devB/2);
   }

   (commonAttr->sysClockFreq) = (extAttr->expectedSysClockFreq) + deviation;

   /* Host interface, config I2C, I2C bridging, powermodes */
   CHK_ERROR( InitHI( demod, device ) );
   /* Upload code to HI that enables atomic reads of multiple words by host */
   CHK_ERROR( InitAtomicRead( devAddr, device ) );

   /* Monitor bus reset and enable */
   CHK_ERROR( EnableAndResetMB( devAddr, device ) );

#if (DRXD_TYPE_A)
   CHK_ERROR( ResetCEFR( devAddr, device ) );
#endif

   if ( commonAttr->microcode != NULL )
   {
      /* Download microcode */
      /* Dirty trick to use common ucode upload & verify,
         pretend device is already open */
      commonAttr->isOpened = TRUE;
      ucodeInfo.mcData = commonAttr->microcode;
      ucodeInfo.mcSize = commonAttr->microcodeSize;
      CHK_ERROR( DRX_Ctrl( demod, DRX_CTRL_LOAD_UCODE, &ucodeInfo, device) );
      if ( commonAttr->verifyMicrocode == TRUE )
      {
         CHK_ERROR( DRX_Ctrl ( demod, DRX_CTRL_VERIFY_UCODE, &ucodeInfo, device) );
      }
      commonAttr->isOpened = FALSE;
   }

   /* Check chip ID (needs running SC) */
   CHK_ERROR( InitSC( demod, device ) );
   WR16( devAddr, SC_CT_REG_COMM_EXEC__A,     1      , 0x0000, device);
   RR16( devAddr, SC_RA_RAM_CONFIG__A   ,  &scFlags  , 0x0000, device);
   WR16( devAddr, SC_CT_REG_COMM_EXEC__A,     0      , 0x0000, device);
   switch ( scFlags & SC_RA_RAM_CONFIG_ID__M ) {
      case SC_RA_RAM_CONFIG_ID_PRO:
         extAttr -> consumerDevice = FALSE;
         break;
      case SC_RA_RAM_CONFIG_ID_CONSUMER:
         extAttr -> consumerDevice = TRUE;
         break;
      default:
         return ( DRX_STS_ERROR );
         break;
   } /* switch */

   /* with or without PGA  */
   if ( ( demod->myDemodFunct->typeId == DRX3973D_TYPE_ID ) ||
        ( demod->myDemodFunct->typeId == DRX3974D_TYPE_ID ) ||
        ( demod->myDemodFunct->typeId == DRX3977D_TYPE_ID ) )
   {
      /* with PGA */
      extAttr -> FeAgRegAgPwd          = DRX3973D_DEF_AG_PWD_PRO;
   } else {
      /* withou PGA */
      extAttr -> FeAgRegAgPwd          = DRX3973D_DEF_AG_PWD_CONSUMER;
   }
   extAttr -> FeAgRegAgAgcSio       = DRX3973D_DEF_AG_AGC_SIO;

   /* Initialize device */
   CHK_ERROR( InitFE( demod, device ) );
   CHK_ERROR( InitFT( demod, device ) );
   CHK_ERROR( InitCP( demod, device ) );
   CHK_ERROR( InitCE( demod, device ) );
   CHK_ERROR( InitEQ( demod, device ) );
   CHK_ERROR( InitEC( demod, device ) );
   CHK_ERROR( InitSC( demod, device ) );
#ifdef USE_LC_INIT
   CHK_ERROR( InitLC( demod, device ) );
#endif

   /* Default IF AGC settings */
   extAttr -> IfAgcCfg.ctrlMode        = DRX3973D_AGC_CTRL_AUTO;
   extAttr -> IfAgcCfg.settleLevel     = 140;
   extAttr -> IfAgcCfg.minOutputLevel  = 0;
   extAttr -> IfAgcCfg.maxOutputLevel  = 1023;
   extAttr -> IfAgcCfg.speed           = 904;
   CtrlSetCfgIfAgc( demod, &(extAttr -> IfAgcCfg), device );

   /* Default RF AGC settings */
   extAttr -> RfAgcCfg.ctrlMode        = DRX3973D_AGC_CTRL_OFF;
   CtrlSetCfgRfAgc( demod, &(extAttr -> RfAgcCfg), device );

   /* Open tuner instance */
   if ( demod->myTuner != NULL )
   {
      demod->myTuner->myCommonAttr->myUserData = (void *)demod;

      if ( commonAttr->tunerPortNr == 1 )
      {
         Bool_t bridgeClosed=TRUE;

         CHK_ERROR( CtrlI2CBridge( demod, &bridgeClosed, device ) );
      }
      /* Dirty trick to use common ucode upload & verify,
         pretend device is already open */
      commonAttr->isOpened = TRUE;
      CHK_ERROR( DRXBSP_TUNER_Open( demod -> myTuner, device ) );
      commonAttr->isOpened = FALSE;
      if ( commonAttr->tunerPortNr == 1 )
      {
         Bool_t bridgeClosed=FALSE;

         CHK_ERROR( CtrlI2CBridge( demod, &bridgeClosed, device ) );
      }

      /* Overwrite max and min RF frequency values with those of the
         tuner instance */
      commonAttr->tunerMinFreqRF = ((demod->myTuner)->myCommonAttr->minFreqRF);
      commonAttr->tunerMaxFreqRF = ((demod->myTuner)->myCommonAttr->maxFreqRF);
   };

   /* Configure initial MPEG output */
   cfgMPEGOutput.enableMPEGOutput = commonAttr->enableMPEGOutput;
   cfgMPEGOutput.insertRSByte     = commonAttr->insertRSByte;
   cfgMPEGOutput.enableParallel   = commonAttr->enableParallel;
   cfgMPEGOutput.invertDATA       = commonAttr->invertDATA;
   cfgMPEGOutput.invertERR        = commonAttr->invertERR;
   cfgMPEGOutput.invertSTR        = commonAttr->invertSTR;
   cfgMPEGOutput.invertVAL        = commonAttr->invertVAL;
   cfgMPEGOutput.invertCLK        = commonAttr->invertCLK;
   CHK_ERROR( CtrlSetCfgMPEGOutput( demod, &cfgMPEGOutput, device) );

   /* Initialize scan parameters */
   commonAttr -> scanDemodLockTimeout =  DRX3973D_SCAN_TIMEOUT;
   commonAttr -> scanDesiredLock =  DRX3973D_DEMOD_LOCK;

   /* Initialise oscillator deviation correction state machine */
   extAttr->CSCDState = DRX3973D_CSCD_INIT;

   /* current bandwidth selection */
   extAttr->curBandwidth = DRX_BANDWIDTH_UNKNOWN;

   return ( DRX_STS_OK );

 rw_error:
   commonAttr->isOpened = FALSE;
   return (DRX_STS_ERROR);
}

/*============================================================================*/
/**
* \fn DRX3973D_Close()
* \brief glgl
* \return Status_t Return status.
*/
DRXStatus_t DRX3973D_Close(pDRXDemodInstance_t demod, DtDevice *device)
{
#if !( VI2C_APPL || COMPILE_FOR_QT )
   DRXPowerMode_t   powerMode=DRX3973D_POWER_DOWN;
#endif
   pI2CDeviceAddr_t devAddr=NULL;
   pDRX3973DData_t  extAttr=NULL;
   pDRXCommonAttr_t commonAttr=NULL;

   devAddr = demod -> myI2CDevAddr;
   extAttr = (pDRX3973DData_t) demod -> myExtAttr;
   commonAttr = (pDRXCommonAttr_t) demod -> myCommonAttr;

   /* Power up */
#if !( VI2C_APPL || COMPILE_FOR_QT )
   powerMode=DRX3973D_POWER_UP;
   CHK_ERROR( CtrlPowerMode( demod, &powerMode, device ));
#endif

   if ( demod->myTuner != NULL )
   {
      if ( commonAttr->tunerPortNr == 1 )
      {
         Bool_t bridgeClosed=TRUE;

         CHK_ERROR( CtrlI2CBridge( demod, &bridgeClosed, device ) );
      }
      CHK_ERROR( DRXBSP_TUNER_Close( demod -> myTuner ) );
      if ( commonAttr->tunerPortNr == 1 )
      {
         Bool_t bridgeClosed=FALSE;

         CHK_ERROR( CtrlI2CBridge( demod, &bridgeClosed, device ) );
      }
   };

   /* Power down */
#if !( VI2C_APPL || COMPILE_FOR_QT )
   powerMode=DRX3973D_POWER_DOWN;
   CHK_ERROR( CtrlPowerMode( demod, &powerMode, device ));
#endif

   return DRX_STS_OK;

 rw_error:
   return (DRX_STS_ERROR);
}

/*============================================================================*/
/**
* \fn DRX3973D_Ctrl()
* \brief DRX3973D specific control function
* \return Status_t Return status.
*/
DRXStatus_t DRX3973D_Ctrl(pDRXDemodInstance_t demod, DRXCtrlIndex_t ctrl, void *ctrlData, DtDevice *device)
{
   pI2CDeviceAddr_t devAddr=NULL;

   devAddr = demod -> myI2CDevAddr;

   switch ( ctrl ) {
      /*======================================================================*/
      case DRX_CTRL_SET_CHANNEL:
         {
            return CtrlSetChannel ( demod,
                                   (pDRXChannel_t) ctrlData, device );
         }
         break;

      /*======================================================================*/
      case DRX_CTRL_GET_CHANNEL:
         {
            return CtrlGetChannel ( demod,
                                   (pDRXChannel_t) ctrlData, device );
         }
         break;

      /*======================================================================*/
      case DRX_CTRL_SIG_QUALITY:
         {
            return CtrlSigQuality ( demod,
                                    (pDRXSigQuality_t) ctrlData, device);
         }
         break;

      /*======================================================================*/
      case DRX_CTRL_SIG_STRENGTH:
         {
            return CtrlSigStrength ( devAddr,
                                     (pu16_t) ctrlData, device);
         }
         break;

      /*======================================================================*/
      case DRX_CTRL_CONSTEL:
         {
            return CtrlConstel ( devAddr,
                                 (pDRXComplex_t) ctrlData, device);
         }
         break;

      /*======================================================================*/
      case DRX_CTRL_SET_CFG:
         {
            return CtrlSetCfg ( demod,
                                (pDRXCfg_t) ctrlData, device);
         }
         break;

      /*======================================================================*/
      case DRX_CTRL_GET_CFG:
         {
            return CtrlGetCfg ( demod,
                                (pDRXCfg_t) ctrlData, device);
         }
         break;

      /*======================================================================*/
      case DRX_CTRL_I2C_BRIDGE:
         {
            return CtrlI2CBridge( demod, (pBool_t) ctrlData, device );
         }
         break;

      /*======================================================================*/
      case DRX_CTRL_LOCK_STATUS:
         {
            return CtrlLockStatus( devAddr, (pDRXLockStatus_t) ctrlData, device );
         }
         break;

      /*======================================================================*/
      case DRX_CTRL_TPS_INFO:
         {
            return CtrlTPSInfo( demod, (pDRXTPSInfo_t) ctrlData, device );
         }
         break;

      /*======================================================================*/
      case DRX_CTRL_UIO_CFG:
         {
            return CtrlUIOCfg( demod, (pDRXUIOCfg_t) ctrlData, device );
         }
         break;

      /*======================================================================*/
      case DRX_CTRL_UIO_READ:
         {
            return CtrlUIORead( demod, (pDRXUIOData_t) ctrlData, device );
         }
         break;

      /*======================================================================*/
      case DRX_CTRL_UIO_WRITE:
         {
            return CtrlUIOWrite( demod, (pDRXUIOData_t) ctrlData, device );
         }
         break;

      /*======================================================================*/
      case DRX_CTRL_SET_STANDARD:
         {
            return CtrlSetStandard( (pDRXStandard_t) ctrlData );
         }
         break;

      /*======================================================================*/
      case DRX_CTRL_GET_STANDARD:
         {
            return CtrlGetStandard( (pDRXStandard_t) ctrlData );
         }
         break;

      /*======================================================================*/
      case DRX_CTRL_POWER_MODE:
         {
            return CtrlPowerMode( demod, (pDRXPowerMode_t) ctrlData, device );
         }
         break;

      /*======================================================================*/
      case DRX_CTRL_VERSION:
         {
            return CtrlVersionInfo( demod, (pDRXVersionList_t *) ctrlData, device );
         }
         break;

      /*======================================================================*/
      case DRX_CTRL_PROBE_DEVICE:
         {
            return CtrlProbeDevice( demod, device );
         }
         break;

      /*======================================================================*/
      case DRX_CTRL_I2C_READWRITE:
         {
            return CtrlI2CWriteRead( demod, (pDRXI2CData_t) ctrlData );
         }
         break;

      default:
         return (DRX_STS_FUNC_NOT_AVAILABLE);
         break;
   }

   return (DRX_STS_OK);
}


/* END OF FILE */
