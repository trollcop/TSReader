/**
* \file $Id: drx3973d.h,v 1.21 2005/09/30 09:25:37 jasper Exp $
*
* \brief DRX3973D specific header files
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
#ifndef __DRX3973D_H__
#define __DRX3973D_H__

/*-------------------------------------------------------------------------
INCLUDES
-------------------------------------------------------------------------*/

#include "drx_driver.h"

#ifdef __cplusplus
extern "C" {
#endif

/*-------------------------------------------------------------------------
DEFINES
-------------------------------------------------------------------------*/

/* Revision types -------------------------------------------------------*/
/* Current default: type A & type A2 */

/*
#ifdef DRXD_TYPE_B
#undef DRXD_TYPE_B
#define DRXD_TYPE_B 1
#else
#define DRXD_TYPE_B 0
#endif  // #ifdef DRXD_TYPE_B
*/

// #undef DRXD_TYPE_A

#define DRXD_TYPE_A (!DRXD_TYPE_B)

#if (DRXD_TYPE_A==1)

#ifdef DRXD_TYPE_A1
#undef DRXD_TYPE_A1
#error "DRXD_TYPE_A1 no longer supported."
#else
#define DRXD_TYPE_A1 0
#endif  /* #ifdef DRXD_TYPE_A1 */

#undef DRXD_TYPE_A2
#define DRXD_TYPE_A2 (!DRXD_TYPE_A1)

#else /* (DRXD_TYPE_A==0) */

#undef DRXD_TYPE_A2
#undef DRXD_TYPE_A1
#define DRXD_TYPE_A2 0
#define DRXD_TYPE_A1 0

#endif  /* #if (DRXD_TYPE_A==1) */

/* End of revision types ------------------------------------------------*/


/* Type id's of drxd family */
#define DRX3973D_TYPE_ID (0x3973000DUL)
#define DRX3974D_TYPE_ID (0x3974000DUL)
#define DRX3975D_TYPE_ID (0x3975000DUL)
#define DRX3976D_TYPE_ID (0x3976000DUL)
#define DRX3977D_TYPE_ID (0x3977000DUL)
#define DRX3978D_TYPE_ID (0x3978000DUL)

/* Power modes for DRX3973D */
#define DRX3973D_POWER_UP           DRX_POWER_UP
#define DRX3973D_POWER_DOWN_CORE    DRX_POWER_MODE_1
#define DRX3973D_POWER_DOWN_PLL     DRX_POWER_MODE_2
#define DRX3973D_POWER_DOWN         DRX_POWER_DOWN

/* Lock states for DRX3973D */
#define DRX3973D_DEMOD_LOCK         DRX_LOCK_STATE_1
#define DRX3973D_FEC_LOCK           DRX_LOCK_STATE_2

/*-------------------------------------------------------------------------
TYPEDEFS
-------------------------------------------------------------------------*/

/*============================================================================*/
/*============================================================================*/
/*== CTRL CFG related data structures ========================================*/
/*============================================================================*/
/*============================================================================*/

#define DRX3973D_CTRL_CFG_BASE          ((int)DRX_CTRL_CFG_MAX)
typedef enum {
   DRX3973D_CFG_POSTVITERBI   = DRX3973D_CTRL_CFG_BASE,
   DRX3973D_CFG_PREVITERBI     ,
   DRX3973D_CFG_SYSCLK2PIN17   ,
   DRX3973D_CFG_IF_AGC         ,
   DRX3973D_CFG_RF_AGC         ,
   DRX3973D_CFG_PGA            ,
   DRX3973D_CFG_SC_CMD         ,
   DRX3973D_CFG_IMP_NOISE      ,

   DRX3973D_CFG_MAX /* dummy, never to be used */

} DRX3973DCfgType_t, *pDRX3973DCfgType_t;

/* configure preViterbi */
typedef struct {
   Bool_t bitsAsUnit ; /* if FALSE then unit will be symbols */
   u16_t  nrUnits ;    /* Nr of Bits or Nr of Symbols depending on BitsAsUnit */
}DRX3973DCfgPreViterbi_t, *pDRX3973DCfgPreViterbi_t;

/* configure IF AGC */
typedef enum {
   DRX3973D_AGC_CTRL_AUTO = 0,
   DRX3973D_AGC_CTRL_USER,
   DRX3973D_AGC_CTRL_OFF
} DRX3973DAgcCtrlMode_t, *pDRX3973DAgcCtrlMode_t;

typedef struct {
   DRX3973DAgcCtrlMode_t ctrlMode;
   u16_t outputLevel;     /* range [0, ... , 1023], 1/n of fullscale range */
   u16_t settleLevel;     /* range [0, ... , 1023], 1/n of fullscale range */
   u16_t minOutputLevel;  /* range [0, ... , 1023], 1/n of fullscale range */
   u16_t maxOutputLevel;  /* range [0, ... , 1023], 1/n of fullscale range */
   u16_t speed;           /* range [0, ... , 1023], 1/n of fullscale range */
}DRX3973DCfgAgc_t, *pDRX3973DCfgAgc_t;

/* configure: SC Command */
typedef struct {
   u16_t cmd;        /**< Command number */
   u16_t subcmd;     /**< Sub-command parameter*/
   u16_t param0;     /**< General purpous param */
   u16_t param1;     /**< General purpous param */
   u16_t param2;     /**< General purpous param */
   u16_t param3;     /**< General purpous param */
   u16_t param4;     /**< General purpous param */
} DRX3973DScCmd_t, *pDRX3973DScCmd_t;


#define DRX3973D_FE_CTRL_MAX  1023
#define DRX3973D_FE_CTRL_MIN  0

/*============================================================================*/
/*============================================================================*/
/*== CTRL related data structures ============================================*/
/*============================================================================*/
/*============================================================================*/

/* NONE */

/*========================================*/


/**
* \enum DRX3973DCSCDState_t
* \brief Correct System Clock Deviation states.
*/
typedef enum {
   DRX3973D_CSCD_INIT=0,
   DRX3973D_CSCD_SET
} DRX3973DCSCDState_t, pDRX3973DCSCDState_t;

/**
* /struct DRX3973DData_t
* DRX3973D specific attributes.
*
* Global data container for DRX3973D specific data.
*
*/
typedef struct {
   /* clocks */
   DRXFrequency_t expectedSysClockFreq;  /**< Systemclock frequency.  (kHz) */

   /* HI configuration */
   u16_t HICfgTimingDiv;         /**< HI Configure() parameter 2 */
   u16_t HICfgBridgeDelay;       /**< HI Configure() parameter 3 */
   u16_t HICfgWakeUpKey;         /**< HI Configure() parameter 4 */
   u16_t HICfgCtrl;              /**< HI Configure() parameter 5 */
   /* FS setting */
   u32_t orgFeFsAddIncr;         /**< Frequency shifter setting  */
   /* Chip ID */
   Bool_t consumerDevice;        /**< consumer/pro device flag */
   /* Needed for restore (SetChannel, PowerMode, RF AGC control) */
   u16_t EcOcRegOcMpgSio;        /**< shadow register */
   u16_t EcOcRegOcModeLop;       /**< shadow register */
   u16_t EcOcRegOcModeHip;       /**< shadow register */
   u16_t EcOcRegIprInvMpg;       /**< shadow register */
   u16_t FeAgRegAgPwd;           /**< shadow register */
   u16_t FeAgRegAgAgcSio;        /**< shadow register */
   /* UIO configuartion */
   DRXUIOMode_t  hiUioMode;      /**< current UIO mode */
   DRXUIOMode_t  scUioMode;      /**< current UIO mode */
   /* AGC control */
   DRX3973DCfgAgc_t IfAgcCfg;
   DRX3973DCfgAgc_t RfAgcCfg;

   /* Correct oscillator deviation state */
   DRX3973DCSCDState_t CSCDState;/**< state of osc freq deviation correction */

   DRXBandwidth_t curBandwidth;  /**< current bandwidth selected */

   Bool_t ignoreLockSigQuality;

} DRX3973DData_t, *pDRX3973DData_t;


/*-------------------------------------------------------------------------
Exported FUNCTIONS
-------------------------------------------------------------------------*/

extern DRXStatus_t DRX3973D_Open(pDRXDemodInstance_t  demod, DtDevice *device);
extern DRXStatus_t DRX3973D_Close(pDRXDemodInstance_t demod, DtDevice *device);
extern DRXStatus_t DRX3973D_Ctrl(pDRXDemodInstance_t  demod,
                                 DRXCtrlIndex_t       ctrl,
                                 void                 *ctrlData,
                                 DtDevice *device);

/*-------------------------------------------------------------------------
Exported GLOBAL VARIABLES
-------------------------------------------------------------------------*/
extern DRXDemodFunc_t      DRX3973DFunctions_g;
extern DRX3973DData_t      DRX3973DData_g;
extern I2CDeviceAddr_t     DRX3973DDefaultAddr_g;
extern DRXCommonAttr_t     DRX3973DDefaultCommAttr_g;
extern DRXDemodInstance_t  DRX3973DDefaultDemod_g;

/*-------------------------------------------------------------------------
THE END
-------------------------------------------------------------------------*/
#ifdef __cplusplus
}
#endif
#endif /* __DRX3973D_H__ */
