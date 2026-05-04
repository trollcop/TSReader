/**
* \file $Id: drx_driver.h,v 1.24 2005/10/21 09:02:17 paulja Exp $
*
* \brief DRX driver API
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
#ifndef __DRXDRIVER_H__
#define __DRXDRIVER_H__
/*-------------------------------------------------------------------------
INCLUDES
-------------------------------------------------------------------------*/
#include "bsp_types.h"
#include "bsp_i2c.h"
#include "bsp_tuner.h"
#include "bsp_host.h"

#ifdef __cplusplus
extern "C" {
#endif
/*-------------------------------------------------------------------------
TYPEDEFS
-------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------
DEFINES
-------------------------------------------------------------------------*/

/**************
 *
 * This section configures the DRX Data Access Protocols (DAPs).
 *
 **************/

/**
* \def DRXDAP_SINGLE_MASTER
* \brief Enable I2C single or I2C multimaster mode on host.
*
* Set to 1 to enable single master mode
* Set to 0 or undefine to enable multi master mode
*
* The actual DAP implementation may be restricted to only one of the modes.
* A compiler warning or error will be generated if the DAP implementation
* overides or cannot handle the mode defined below.
*
*/
#define DRXDAP_SINGLE_MASTER 1


/**
* \def DRXDAP_MAX_WCHUNKSIZE
* \brief Defines maximum chunksize of an i2c write action by host.
*
* This indicates the maximum size of data the I2C device driver is able to
* write at a time. This includes I2C device address and register addressing.
*
* This maximum size may be restricted by the actual DAP implementation.
* A compiler warning or error will be generated if the DAP implementation
* overides or cannot handle the chunksize defined below.
*
* Beware that the DAP uses  DRXDAP_MAX_WCHUNKSIZE to create a temporary data
* buffer. Do not undefine or choose too large, unless your system is able to
* handle a stack buffer of that size.
*
*/
// #define  DRXDAP_MAX_WCHUNKSIZE 254
#define  DRXDAP_MAX_WCHUNKSIZE 62

/**
* \def DRXDAP_MAX_RCHUNKSIZE
* \brief Defines maximum chunksize of an i2c read action by host.
*
* This indicates the maximum size of data the I2C device driver is able to read
* at a time. Minimum value is 2. Also, the read chunk size must be even.
*
* This maximum size may be restricted by the actual DAP implementation.
* A compiler warning or error will be generated if the DAP implementation
* overides or cannot handle the chunksize defined below.
*
*/
// #define  DRXDAP_MAX_RCHUNKSIZE 254
#define  DRXDAP_MAX_RCHUNKSIZE 62


/**************
 *
 * This section describes drxdriver defines.
 *
 **************/

/**
* \def DRX_UNKNOWN
* \brief Generic UNKNOWN value for DRX enumerated types.
*
* Used to indicate that the parameter value is unknown or not yet initalized.
*/
#define DRX_UNKNOWN (254)

/**
* \def DRX_AUTO
* \brief Generic AUTO value for DRX enumerated types.
*
* Used to instruct the driver to automatically determine the value of the parameter.
*/
#define DRX_AUTO    (255)

/*-------------------------------------------------------------------------
MACROS
-------------------------------------------------------------------------*/
/* Macros to stringify the version number */
#define DRX_VERSIONSTRING( MAJOR, MINOR, PATCH ) \
         DRX_VERSIONSTRING_HELP(MAJOR)"." \
         DRX_VERSIONSTRING_HELP(MINOR)"." \
         DRX_VERSIONSTRING_HELP(PATCH)
#define DRX_VERSIONSTRING_HELP( NUM ) #NUM

/*-------------------------------------------------------------------------
ENUM
-------------------------------------------------------------------------*/

/**
* \enum DRXStandard_t
* \brief Modulation standards.
*/
typedef enum DRXStandard_t {
    DRX_STANDARD_DVBT    = 0,            /**< Terrestrial DVB-T. */
    DRX_STANDARD_8VSB,                   /**< Terrestrial 8VSB. */
    DRX_STANDARD_NTSC,                   /**< Terrestrial analog NTSC. */
    DRX_STANDARD_PAL,                    /**< Terrestrial ANALOG PAL. */
    DRX_STANDARD_ITU_A,                  /**< Cable ITU ANNEX A. */
    DRX_STANDARD_ITU_B,                  /**< Cable ITU ANNEX B. */
    DRX_STANDARD_ITU_C,                  /**< Cable ITU ANNEX C. */
    DRX_STANDARD_ITU_D,                  /**< Cable ITU ANNEX D. */
    DRX_STANDARD_UNKNOWN = DRX_UNKNOWN,  /**< Standard unknown. */
    DRX_STANDARD_AUTO    = DRX_AUTO      /**< Autodetect standard. */
} DRXStandard_t, *pDRXStandard_t;

/**
* \enum DRXBandwidth_t
* \brief Channel bandwidth or channel spacing.
*/
typedef enum DRXBandwidth_t {
    DRX_BANDWIDTH_8MHZ    = 0,            /**< Bandwidth 8 MHz. */
    DRX_BANDWIDTH_7MHZ,                   /**< Bandwidth 7 MHz. */
    DRX_BANDWIDTH_6MHZ,                   /**< Bandwidth 6 MHz. */
    DRX_BANDWIDTH_UNKNOWN = DRX_UNKNOWN   /**< Bandwidth unknown. */
} DRXBandwidth_t, *pDRXBandwidth_t;

/**
* \enum DRXMirror_t
* \brief Indicate if channel spectrum is mirrored or not.
*/
typedef enum {
   DRX_MIRROR_YES     = 0,            /**< Spectrum is mirrored. */
   DRX_MIRROR_NO,                     /**< Spectrum is not mirrored. */
   DRX_MIRROR_UNKNOWN = DRX_UNKNOWN,  /**< It is unknown if spectrum is mirrored. */
   DRX_MIRROR_AUTO   = DRX_AUTO      /**< Autodetect if spectrum is mirrored. */
} DRXMirror_t, *pDRXMirror_t;

/**
* \enum DRXConstellation_t
* \brief Constellation type of the channel.
*/
typedef enum {
   DRX_CONSTELLATION_BPSK    = 0,            /**< Modulation is BPSK. */
   DRX_CONSTELLATION_QPSK,                   /**< Constellation is QPSK. */
   DRX_CONSTELLATION_PSK8,                   /**< Constellation is PSK8. */
   DRX_CONSTELLATION_QAM16,                  /**< Constellation is QAM16. */
   DRX_CONSTELLATION_QAM32,                  /**< Constellation is QAM32. */
   DRX_CONSTELLATION_QAM64,                  /**< Constellation is QAM64. */
   DRX_CONSTELLATION_QAM128,                 /**< Constellation is QAM128. */
   DRX_CONSTELLATION_QAM256,                 /**< Constellation is QAM256. */
   DRX_CONSTELLATION_QAM512,                 /**< Constellation is QAM512. */
   DRX_CONSTELLATION_QAM1024,                /**< Constellation is QAM1024. */
   DRX_CONSTELLATION_UNKNOWN = DRX_UNKNOWN,  /**< Constellation unknown. */
   DRX_CONSTELLATION_AUTO    = DRX_AUTO      /**< Autodetect constellation. */
} DRXConstellation_t, *pDRXConstellation_t;

/**
* \enum DRXHierarchy_t
* \brief Hierarchy of the channel.
*/
typedef enum {
   DRX_HIERARCHY_NONE    = 0,           /**< No hierarchical transmission. */
   DRX_HIERARCHY_ALPHA1,                /**< Hierarchical transmission with alpha=1. */
   DRX_HIERARCHY_ALPHA2,                /**< Hierarchical transmission with alpha=2. */
   DRX_HIERARCHY_ALPHA4,                /**< Hierarchical transmission with alpha=4. */
   DRX_HIERARCHY_UNKNOWN = DRX_UNKNOWN, /**< Hierarchicy unknown. */
   DRX_HIERARCHY_AUTO    = DRX_AUTO     /**< Autodetect hierarchy. */
} DRXHierarchy_t, *pDRXHierarchy_t;

/**
* \enum DRXPriority_t
* \brief Channel priority in case of hierarchical transmission.
*/
typedef enum {
   DRX_PRIORITY_LOW     = 0,           /**< Low priority channel. */
   DRX_PRIORITY_HIGH,                  /**< High priority channel. */
   DRX_PRIORITY_UNKNOWN = DRX_UNKNOWN  /**< Priority unknown. */
} DRXPriority_t, *pDRXPriority_t;

/**
* \enum DRXCoderate_t
* \brief Channel priority in case of hierarchical transmission.
*/
typedef enum {
   DRX_CODERATE_1DIV2   = 0,            /**< Code rate 1/2nd. */
   DRX_CODERATE_2DIV3,                  /**< Code rate 2/3nd. */
   DRX_CODERATE_3DIV4,                  /**< Code rate 3/4nd. */
   DRX_CODERATE_5DIV6,                  /**< Code rate 5/6nd. */
   DRX_CODERATE_7DIV8,                  /**< Code rate 7/8nd. */
   DRX_CODERATE_UNKNOWN = DRX_UNKNOWN,  /**< Code rate unknown. */
   DRX_CODERATE_AUTO    = DRX_AUTO      /**< Autodetect code rate. */
} DRXCoderate_t, *pDRXCoderate_t;

/**
* \enum DRXGuard_t
* \brief Guard interval of a channel.
*/
typedef enum {
   DRX_GUARD_1DIV32   = 0,            /**< Guard interval 1/32nd. */
   DRX_GUARD_1DIV16,                  /**< Guard interval 1/16th. */
   DRX_GUARD_1DIV8,                   /**< Guard interval 1/8th. */
   DRX_GUARD_1DIV4,                   /**< Guard interval 1/4th. */
   DRX_GUARD_UNKNOWN  = DRX_UNKNOWN,  /**< Guard interval unknown. */
   DRX_GUARD_AUTO     = DRX_AUTO      /**< Autodetect guard interval. */
} DRXGuard_t, *pDRXGuard_t;

/**
* \enum DRXFftmode_t
* \brief FFT mode.
*/
typedef enum {
   DRX_FFTMODE_2K      = 0,            /**< 2K FFT mode. */
   DRX_FFTMODE_8K,                     /**< 8K FFT mode. */
   DRX_FFTMODE_UNKNOWN = DRX_UNKNOWN,  /**< FFT mode unknown. */
   DRX_FFTMODE_AUTO    = DRX_AUTO      /**< Autodetect FFT mode. */
} DRXFftmode_t, *pDRXFftmode_t;

/**
* \enum DRXClassification_t
* \brief Channel classification.
*/
typedef enum {
   DRX_CLASSIFICATION_GAUSS     = 0,            /**< Gaussion noise. */
   DRX_CLASSIFICATION_HVY_GAUSS,                /**< Heavy Gaussion noise. */
   DRX_CLASSIFICATION_COCHANNEL,                /**< Co-channel. */
   DRX_CLASSIFICATION_STATIC,                   /**< Static echo. */
   DRX_CLASSIFICATION_MOVING,                   /**< Moving echo. */
   DRX_CLASSIFICATION_ZERODB,                   /**< Zero dB echo. */
   DRX_CLASSIFICATION_UNKNOWN   = DRX_UNKNOWN,  /**< Channel classification unknown. */
   DRX_CLASSIFICATION_AUTO      = DRX_AUTO      /**< Autodetect channel classification. */
} DRXClassification_t, *pDRXClassification_t;

/**
* \enum DRXTPSFrame_t
* \brief Frame number in current super-frame.
*/
typedef enum {
   DRX_TPS_FRAME1     = 0,                /**< TPS frame 1. */
   DRX_TPS_FRAME2,                        /**< TPS frame 2. */
   DRX_TPS_FRAME3,                        /**< TPS frame 3. */
   DRX_TPS_FRAME4,                        /**< TPS frame 4. */
   DRX_TPS_FRAME_UNKNOWN = DRX_UNKNOWN   /**< TPS frame unknown. */
} DRXTPSFrame_t, *pDRXTPSFrame_t;

/**
* \enum DRXCtrlIndex_t
* \brief Indices of the control functions.
*/
#define DRX_CTRL_BASE          (0x0000)
typedef enum {
   DRX_CTRL_NOP                 = DRX_CTRL_BASE ,
   DRX_CTRL_PROBE_DEVICE        ,

   DRX_CTRL_LOAD_UCODE          ,
   DRX_CTRL_VERIFY_UCODE        ,
   DRX_CTRL_SET_CHANNEL         ,
   DRX_CTRL_GET_CHANNEL         ,
   DRX_CTRL_LOCK_STATUS         ,
   DRX_CTRL_SIG_QUALITY         ,
   DRX_CTRL_SIG_STRENGTH        ,
   DRX_CTRL_CONSTEL             ,
   DRX_CTRL_SCAN_INIT           ,
   DRX_CTRL_SCAN_NEXT           ,
   DRX_CTRL_TPS_INFO            ,
   DRX_CTRL_SET_CFG             ,
   DRX_CTRL_GET_CFG             ,
   DRX_CTRL_VERSION             ,
   DRX_CTRL_I2C_BRIDGE          ,
   DRX_CTRL_SET_STANDARD        ,
   DRX_CTRL_GET_STANDARD        ,
   DRX_CTRL_SET_OOB             ,
   DRX_CTRL_GET_OOB             ,
   DRX_CTRL_I2C_READWRITE       ,

   /* Professional */
   DRX_CTRL_MB_CFG              ,
   DRX_CTRL_MB_READ             ,
   DRX_CTRL_MB_WRITE            ,
   DRX_CTRL_MB_CONSTEL          ,
   DRX_CTRL_MB_MER              ,

   /* Misc */
   DRX_CTRL_UIO_CFG             ,
   DRX_CTRL_UIO_READ            ,
   DRX_CTRL_UIO_WRITE           ,
   DRX_CTRL_READ_EVENTS         ,
   DRX_CTRL_HDL_EVENTS          ,
   DRX_CTRL_POWER_MODE          ,

   DRX_CTRL_MAX /* dummy, never to be used */

} DRXCtrlIndex_t, *pDRXCtrlIndex_t;

/**
* \enum DRXUCodeAction_t
* \brief Used to indicate if firmware has to be uploaded or verified.
*/
typedef enum {
   UCODE_UPLOAD,
   UCODE_VERIFY
} DRXUCodeAction_t, *pDRXUCodeAction_t;


/**
* \enum DRXLockStatus_t
* \brief Used to reflect current lock status of demodulator.
*
* The generic lock states have device dependent semantics.
*/
typedef enum{
   DRX_NEVER_LOCK = 0,        /**< Device will never lock on this signal */
   DRX_NOT_LOCKED,            /**< Device has no lock at all */
   DRX_LOCK_STATE_1,          /**< Generic lock state */
   DRX_LOCK_STATE_2,          /**< Generic lock state */
   DRX_LOCK_STATE_3,          /**< Generic lock state */
   DRX_LOCK_STATE_4,          /**< Generic lock state */
   DRX_LOCK_STATE_5,          /**< Generic lock state */
   DRX_LOCK_STATE_6,          /**< Generic lock state */
   DRX_LOCK_STATE_7,          /**< Generic lock state */
   DRX_LOCK_STATE_8,          /**< Generic lock state */
   DRX_LOCK_STATE_9,          /**< Generic lock state */
   DRX_LOCKED                 /**< Device is in lock */
} DRXLockStatus_t, *pDRXLockStatus_t;

/**
* \enum DRXUIO_t
* \brief Used to address a User IO (UIO).
*/
typedef enum{
   DRX_UIO1  ,
   DRX_UIO2  ,
   DRX_UIO3  ,
   DRX_UIO4  ,
   DRX_UIO5  ,
   DRX_UIO6  ,
   DRX_UIO7  ,
   DRX_UIO8  ,
   DRX_UIO9  ,
   DRX_UIO10 ,
   DRX_UIO11 ,
   DRX_UIO12 ,
   DRX_UIO13 ,
   DRX_UIO14 ,
   DRX_UIO15 ,
   DRX_UIO16 ,
   DRX_UIO17 ,
   DRX_UIO18 ,
   DRX_UIO19 ,
   DRX_UIO20 ,
   DRX_UIO21 ,
   DRX_UIO22 ,
   DRX_UIO23 ,
   DRX_UIO24 ,
   DRX_UIO25 ,
   DRX_UIO26 ,
   DRX_UIO27 ,
   DRX_UIO28 ,
   DRX_UIO29 ,
   DRX_UIO30 ,
   DRX_UIO31 ,
   DRX_UIO32 ,
   DRX_UIO_MAX = DRX_UIO32
} DRXUIO_t, *pDRXUIO_t;

/**
* \enum DRXUIOMode_t
* \brief Used to configure the modus oprandi of a UIO.
*/
typedef enum{
   DRX_UIO_MODE_DISABLE = 0, /**< UIO is not used,pin is configured as input */
   DRX_UIO_MODE_READWRITE,   /**< UIO is used for read/write by application */
   DRX_UIO_MODE_FIRMWARE     /**< UIO is controled and used by firmware */
} DRXUIOMode_t, *pDRXUIOMode_t;

/**
* \enum DRXOOBDirection_t
* \brief Used to select upstream or downstream OOB control.
*/
typedef enum  {
  DRX_OOB_DOWNSTREAM = 0,  /**< Downstream out-of-band */
  DRX_OOB_UPSTREAM         /**< Upstream out-of-band */
} DRXOOBDirection_t, *pDRXOOBDirection_t;


/**
* \enum DRXOOBStandard_t
* \brief Used to select OOB standard.
*
* Based on ANSI 55-1 and 55-2
*/
typedef enum {
   DRX_STANDARD_OOB_A = 0,  /**< ANSI 55-2 A both up- and down stream */
   DRX_STANDARD_OOB_B,      /**< ANSI 55-2 B both up- and down stream */
   DRX_STANDARD_OOB_C,      /**< ANSI 55-2 C only upstream */
   DRX_STANDARD_OOB_STD,    /**< ANSI 55-1 standard both up- and down stream */
   DRX_STANDARD_OOB_STD_ALT,    /**< ANSI 55-1 standard alt*/
   DRX_STANDARD_OOB_EXT     /**< ANSI 55-1 extended only upstream */
} DRXOOBStandard_t, *pDRXOOBStandard_t;

/**
* \enum DRXOOBBitrate_t
* \brief Used to select bitrates for OOB up and downstream channels.
*
* Based on ANSI 55-1 and 55-2
* TODO : ANSI 55-1 ext upstream bitrates
*/
typedef enum {
   DRX_BITRATE_256KBS = 0, /**< ANSI 55-1 std upstream , 55-2 A upstream*/
   DRX_BITRATE_1544KBS,    /**< ANSI 55-2 A downstream, 55-2 B upstrteam */
   DRX_BITRATE_2048KBS,    /**< ANSI 55-1 downstream */
   DRX_BITRATE_3088KBS     /**< ANSI 55-2 B downstream, 55-2 C upstream */
} DRXOOBBitrate_t, *pDRXOOBBitrate_t;

/*-------------------------------------------------------------------------
STRUCTS
-------------------------------------------------------------------------*/

/*============================================================================*/
/*============================================================================*/
/*== CTRL CFG related data structures ========================================*/
/*============================================================================*/
/*============================================================================*/

typedef enum {
   DRX_CFG_MPEG_OUTPUT = 0,
   /* ...  generic configuration functions */
   DRX_CTRL_CFG_MAX /* dummy, never to be used */
} DRXCfgType_t, *pDRXCfgType_t;

/*============================================================================*/
/*============================================================================*/
/*== CTRL related data structures ============================================*/
/*============================================================================*/
/*============================================================================*/

/* DRX_CTRL_LOAD_UCODE, DRX_CTRL_VERIFY_UCODE */
typedef struct {
   pu8_t    mcData;
   u16_t    mcSize;
} DRXUCodeInfo_t, *pDRXUCodeInfo_t;

/*========================================*/

/* DRX_CTRL_SET_CHANNEL, DRX_CTRL_GET_CHANNEL */

/**
* \struct DRXChannel_t
* The set of parameters describing a single channel.
*/
typedef struct {
   DRXFrequency_t       frequency;      /**< in kHz */
   DRXBandwidth_t       bandwidth;
   DRXMirror_t          mirror;
   DRXConstellation_t   constellation;
   DRXHierarchy_t       hierarchy;
   DRXPriority_t        priority;
   DRXCoderate_t        coderate;
   DRXGuard_t           guard;
   DRXFftmode_t         fftmode;
   DRXClassification_t  classification;
} DRXChannel_t, *pDRXChannel_t;


/*========================================*/

/* DRX_CTRL_SIG_QUALITY */
typedef struct {
   u16_t MER;              /**< in steps of 0.1 dB */
   u32_t preViterbiBER ;   /**< in steps of 1/scaleFactorBER */
   u32_t postViterbiBER ;  /**< in steps of 1/scaleFactorBER */
   u32_t scaleFactorBER;   /**< scale factor for BER */
   u16_t packetError ;     /**< number of packet errors */
}DRXSigQuality_t, *pDRXSigQuality_t;

/*========================================*/

/* DRX_CTRL_CONSTEL */

/**
* \struct DRXComplex_t
* A complex number.
*/
typedef struct {
   s16_t im; /**< Imaginair part. */
   s16_t re; /**< Real part. */
} DRXComplex_t, *pDRXComplex_t;


/*========================================*/

/* used in DRX_CTRL_SCAN_INIT's param structure */
typedef struct {
    DRXFrequency_t first;     /**< First centre frequency in this band */
    DRXFrequency_t last;      /**< Last centre frequency in this band */
    DRXBandwidth_t bandwidth; /**< Bandwidth within this frequency band */
    u16_t          chNumber;  /**< First channel number in this band, or first index in chNames */
    char         **chNames;   /**< List of channel names in this band (optional) */
} DRXFrequencyPlan_t, *pDRXFrequencyPlan_t;

/* DRX_CTRL_SCAN_INIT */
typedef struct {
   pDRXFrequencyPlan_t frequencyPlan;     /**< Frequency plan (array)*/
   u16_t               frequencyPlanSize; /**< Number of bands */
   u32_t               numTries;          /**< Max channels tried */
} DRXScanParam_t, *pDRXScanParam_t;

/*========================================*/

/* DRX_CTRL_TPS_INFO */

typedef struct {
  DRXFftmode_t       fftmode;
  DRXGuard_t         guard;
  DRXConstellation_t constellation;
  DRXHierarchy_t     hierarchy;
  DRXCoderate_t      highCoderate;
  DRXCoderate_t      lowCoderate;
  DRXTPSFrame_t      frame;
  u8_t               length;
  u16_t              cellId;
}DRXTPSInfo_t, *pDRXTPSInfo_t;

/*========================================*/

/* DRX_CTRL_SET_POWER_MODE */

typedef enum {
   DRX_POWER_UP = 0,     /**< Generic */
   DRX_POWER_MODE_1,     /**< Device specific */
   DRX_POWER_MODE_2,     /**< Device specific  */
   DRX_POWER_MODE_3,     /**< Device specific  */
   DRX_POWER_MODE_4,     /**< Device specific  */
   DRX_POWER_MODE_5,     /**< Device specific  */
   DRX_POWER_MODE_6,     /**< Device specific  */
   DRX_POWER_MODE_7,     /**< Device specific  */
   DRX_POWER_MODE_8,     /**< Device specific  */
   DRX_POWER_MODE_9,     /**< Device specific  */
   DRX_POWER_DOWN = 255  /**< Generic */
}DRXPowerMode_t, *pDRXPowerMode_t;

/*========================================*/

/* DRX_CTRL_VERSION */

typedef enum
{
  /* Add, remove or edit as necessary!
     Also maintain DRX_STR_MODULE(x) below! */
  DRX_MODULE_DEVICE,
  DRX_MODULE_MICROCODE,
  DRX_MODULE_DRIVERCORE,
  DRX_MODULE_DEVICEDRIVER,
  DRX_MODULE_DAP,
  DRX_MODULE_BSP_I2C,
  DRX_MODULE_BSP_TUNER,
  DRX_MODULE_BSP_HOST,

  DRX_MODULE_UNKNOWN

} DRXModule_t, *pDRXModule_t;


/* type to store a single version structure */

typedef struct {

  DRXModule_t  moduleType;    /**< type identifier of the module */
  char        *moduleName;    /**< name or description of module */

  u16_t        vMajor;        /**< major version number */
  u16_t        vMinor;        /**< minor version number */
  u16_t        vPatch;        /**< patch version number */
  char        *vString;       /**< version as text string */

} DRXVersion_t, *pDRXVersion_t;


/* type to make a linked-list of version structures */

typedef struct DRXVersionList_s {

  pDRXVersion_t            version;
  struct DRXVersionList_s *next;

} DRXVersionList_t, *pDRXVersionList_t;

/*========================================*/

/* DRX_CTRL_UIO_CFG */
typedef struct {
   DRXUIO_t      uio;
   DRXUIOMode_t  mode;
} DRXUIOCfg_t, *pDRXUIOCfg_t;

/*========================================*/

/* DRX_CTRL_UIO_READ, DRX_CTRL_UIO_WRITE */
typedef struct {
   DRXUIO_t uio;
   Bool_t   value;
} DRXUIOData_t, *pDRXUIOData_t;

/*========================================*/

/* DRX_CTRL_SET_OOB */
typedef struct {
   DRXOOBDirection_t    direction;
   DRXOOBStandard_t     standard;
   DRXFrequency_t       frequency;
   u32_t                powerLevel;    /* upstream ,RF ,in units of 0.5 dBmV */
   DRXConstellation_t   constellation; /* upstream */
   DRXOOBBitrate_t      bitrate;       /* upstream */
} DRXOOB_t, *pDRXOOB_t;


/*========================================*/

/* DRX_CTRL_SET_CFG */

/* CTRL CFG base data structure */
typedef struct {
   DRXCfgType_t cfgType ;
   void*        cfgData ;
} DRXCfg_t, *pDRXCfg_t;

/*========================================*/

/* CTRL CFG MPEG ouput */
/**
* \struct DRXCfgMPEGOutput_t
* \brief Configuartion parameters for MPEG output control.
*/

typedef struct {
   Bool_t      enableMPEGOutput;      /**< If TRUE, enable MPEG output */
   Bool_t      insertRSByte;          /**< If TRUE, insert RS byte */
   Bool_t      enableParallel;        /**< If TRUE, parallel out otherwise serial */
   Bool_t      invertDATA;            /**< If TRUE, invert DATA signals */
   Bool_t      invertERR;             /**< If TRUE, invert ERR signal */
   Bool_t      invertSTR;             /**< If TRUE, invert STR signals */
   Bool_t      invertVAL;             /**< If TRUE, invert VAL signals */
   Bool_t      invertCLK;             /**< If TRUE, invert CLK signals */
} DRXCfgMPEGOutput_t, *pDRXCfgMPEGOutput_t;

/*========================================*/

/* DRX_CTRL_I2C_READWRITE */
/**
* \struct DRXI2CData_t
* \brief Data for I2C via 2nd or 3rd or etc I2C port.
*
* If portNr is equal to primairy portNr BSPI2C will be used.
*
*/
typedef struct {
   u16_t            portNr;
   pI2CDeviceAddr_t wDevAddr;
   u16_t            wCount;
   pu8_t            wData;
   pI2CDeviceAddr_t rDevAddr;
   u16_t            rCount;
   pu8_t            rData;
} DRXI2CData_t, *pDRXI2CData_t;

/*============================================================================*/
/*============================================================================*/
/*== Data access structures ==================================================*/
/*============================================================================*/
/*============================================================================*/

/* Address on device */
typedef u32_t DRXaddr_t;

/* Protocol specific flags */
typedef u32_t DRXflags_t;

/* Write block of data to device */
typedef DRXStatus_t (*DRXWriteBlockFunc_t) (
         pI2CDeviceAddr_t devAddr,       /* address of I2C device        */
         DRXaddr_t        addr,          /* address of register/memory   */
         u16_t            datasize,      /* size of data in bytes        */
         pu8_t            data,          /* data to send                 */
         DRXflags_t       flags,
         DtDevice *device);

/* Read block of data from device */
typedef DRXStatus_t (*DRXReadBlockFunc_t) (
         pI2CDeviceAddr_t devAddr,       /* address of I2C device        */
         DRXaddr_t        addr,          /* address of register/memory   */
         u16_t            datasize,      /* size of data in bytes        */
         pu8_t            data,          /* receive buffer               */
         DRXflags_t       flags,
         DtDevice *device);

/* Write 8-bits value to device */
typedef DRXStatus_t (*DRXWriteReg8Func_t) (
         pI2CDeviceAddr_t devAddr,       /* address of I2C device        */
         DRXaddr_t        addr,          /* address of register/memory   */
         u8_t             data,          /* data to send                 */
         DRXflags_t       flags,
         DtDevice *device);

/* Read 8-bits value to device */
typedef DRXStatus_t (*DRXReadReg8Func_t) (
         pI2CDeviceAddr_t devAddr,       /* address of I2C device        */
         DRXaddr_t        addr,          /* address of register/memory   */
         pu8_t            data,          /* receive buffer               */
         DRXflags_t       flags,
         DtDevice *device);

/* Read modify write 8-bits value to device */
typedef DRXStatus_t (*DRXReadModifyWriteReg8Func_t) (
         pI2CDeviceAddr_t devAddr,       /* address of I2C device       */
         DRXaddr_t        waddr,         /* write address of register   */
         DRXaddr_t        raddr,         /* read  address of register   */
         u8_t             wdata,         /* data to write               */
         pu8_t            rdata,         /* data to read                */
         DtDevice         *device);

/* Write 16-bits value to device */
typedef DRXStatus_t (*DRXWriteReg16Func_t) (
         pI2CDeviceAddr_t devAddr,       /* address of I2C device        */
         DRXaddr_t        addr,          /* address of register/memory   */
         u16_t            data,          /* data to send                 */
         DRXflags_t       flags,
         DtDevice *device);

/* Read 16-bits value to device */
typedef DRXStatus_t (*DRXReadReg16Func_t) (
         pI2CDeviceAddr_t devAddr,       /* address of I2C device        */
         DRXaddr_t        addr,          /* address of register/memory   */
         pu16_t           data,          /* receive buffer               */
         DRXflags_t       flags,
         DtDevice *device);

/* Read modify write 16-bits value to device */
typedef DRXStatus_t (*DRXReadModifyWriteReg16Func_t) (
         pI2CDeviceAddr_t devAddr,       /* address of I2C device       */
         DRXaddr_t        waddr,         /* write address of register   */
         DRXaddr_t        raddr,         /* read  address of register   */
         u16_t            wdata,         /* data to write               */
         pu16_t           rdata,
         DtDevice *device);        /* data to read                */

/* Write 32-bits value to device */
typedef DRXStatus_t (*DRXWriteReg32Func_t) (
         pI2CDeviceAddr_t devAddr,       /* address of I2C device        */
         DRXaddr_t        addr,          /* address of register/memory   */
         u32_t            data,          /* data to send                 */
         DRXflags_t       flags,
         DtDevice *device);

/* Read 32-bits value to device */
typedef DRXStatus_t (*DRXReadReg32Func_t) (
         pI2CDeviceAddr_t devAddr,       /* address of I2C device        */
         DRXaddr_t        addr,          /* address of register/memory   */
         pu32_t           data,          /* receive buffer               */
         DRXflags_t       flags,
         DtDevice *device);

/* Read modify write 32-bits value to device */
typedef DRXStatus_t (*DRXReadModifyWriteReg32Func_t) (
         pI2CDeviceAddr_t devAddr,       /* address of I2C device       */
         DRXaddr_t        waddr,         /* write address of register   */
         DRXaddr_t        raddr,         /* read  address of register   */
         u32_t            wdata,         /* data to write               */
         pu32_t           rdata,
         DtDevice *device);        /* data to read                */

/**
* \struct DRXAccessFunc_t
* \brief Interface to an access protocol.
*/
typedef struct {
   pDRXVersion_t                   protocolVersion;
   DRXWriteBlockFunc_t             writeBlockFunc;
   DRXReadBlockFunc_t              readBlockFunc;
   DRXWriteReg8Func_t              writeReg8Func;
   DRXReadReg8Func_t               readReg8Func;
   DRXReadModifyWriteReg8Func_t    readModifyWriteReg8Func;
   DRXWriteReg16Func_t             writeReg16Func;
   DRXReadReg16Func_t              readReg16Func;
   DRXReadModifyWriteReg16Func_t   readModifyWriteReg16Func;
   DRXWriteReg32Func_t             writeReg32Func;
   DRXReadReg32Func_t              readReg32Func;
   DRXReadModifyWriteReg32Func_t   readModifyWriteReg32Func;
} DRXAccessFunc_t, *pDRXAccessFunc_t;


/*============================================================================*/
/*============================================================================*/
/*== Demod instance data structures ==========================================*/
/*============================================================================*/
/*============================================================================*/

typedef struct DRXDemodInstance_s *pDRXDemodInstance_t;

/**
* \struct DRXCommonAttr_t
* \brief Set of common attributes, shared by all DRX devices.
*/
typedef struct {
   /* Microcode (firmware) attributes */
   pu8_t          microcode;             /**< Pointer to array containing microcode. */
   u16_t          microcodeSize;         /**< Size of the microcode array in bytes. */
   Bool_t         verifyMicrocode;       /**< Use microcode verify or not.*/

   /* Clocks and tuner attributes */
   DRXFrequency_t intermediateFreq;      /**< IF, in case no tuner instance is used. (kHz) */
   DRXFrequency_t sysClockFreq;          /**< Systemclock frequency.  (kHz) */
   DRXFrequency_t oscClockFreq;          /**< Oscillator clock frequency.  (kHz) */
   s16_t          oscClockDeviation;     /**< Oscillator clock deviation.  (ppm) */
   Bool_t         mirrorFreqSpect;       /**< Mirror the IF frequency spectrum or not. */

   /* Initial MPEG output attributes */
   Bool_t      enableMPEGOutput;      /**< If TRUE, enable MPEG output */
   Bool_t      insertRSByte;          /**< If TRUE, insert RS byte */
   Bool_t      enableParallel;        /**< If TRUE, parallel out otherwise serial */
   Bool_t      invertDATA;            /**< If TRUE, invert DATA signals */
   Bool_t      invertERR;             /**< If TRUE, invert ERR signal */
   Bool_t      invertSTR;             /**< If TRUE, invert STR signals */
   Bool_t      invertVAL;             /**< If TRUE, invert VAL signals */
   Bool_t      invertCLK;             /**< If TRUE, invert CLK signals */

   Bool_t      isOpened;              /**< if TRUE instance is already opened. */

   /* Channel scan */
   pDRXScanParam_t scanParam;         /**< scan parameters */
   u16_t           scanFreqPlanIndex; /**< next index in freq plan */
   DRXFrequency_t  scanNextFrequency; /**< next freq to scan */
   Bool_t          scanReady;         /**< scan ready flag */
   u32_t           scanMaxChannels;   /**< number of channels in freqplan */
   u32_t           scanChannelsScanned; /**< number of channels scanned */
   u16_t           scanDemodLockTimeout; /**< millisecs to wait for demod lock */
   DRXLockStatus_t scanDesiredLock;   /**< lock requirement for channel found */
   /* scanActive can be used by SetChannel to decide how to program the tuner,
      fast or slow (but stable). Usually fast during scan. */
   Bool_t          scanActive;        /**< TRUE when scan routines are active */

   /* Power management */
   DRXPowerMode_t  currentPowerMode;  /**< current power management mode */

   /* Tuner */
   u8_t            tunerPortNr;       /**< nr of I2C port to wich tuner is */
   DRXFrequency_t  tunerMinFreqRF;    /**< minimum RF input frequency, in kHz */
   DRXFrequency_t  tunerMaxFreqRF;    /**< maximum RF input frequency, in kHz */

} DRXCommonAttr_t, *pDRXCommonAttr_t;


/*
* Generic functions for DRX devices.
*/
typedef DRXStatus_t (*DRXOpenFunc_t)  (pDRXDemodInstance_t demod, DtDevice *device);
typedef DRXStatus_t (*DRXCloseFunc_t) (pDRXDemodInstance_t demod, DtDevice *device);
typedef DRXStatus_t (*DRXCtrlFunc_t)  (pDRXDemodInstance_t demod, DRXCtrlIndex_t ctrl, void *ctrlData, DtDevice *device);

/**
* \struct DRXDemodFunc_t
* \brief A stucture containing all functions of a demodulator.
*/
typedef struct {
   u32_t          typeId;                /**< Device type identifier. */
   DRXOpenFunc_t  openFunc;              /**< Pointer to Open() function. */
   DRXCloseFunc_t closeFunc;             /**< Pointer to Close() function. */
   DRXCtrlFunc_t  ctrlFunc;              /**< Pointer to Ctrl() function. */
} DRXDemodFunc_t, *pDRXDemodFunc_t;

/**
* \struct DRXDemodInstance_t
* \brief Top structure of demodulator instance.
*/
typedef struct DRXDemodInstance_s {
   /* type specific demodulator data */
   pDRXDemodFunc_t   myDemodFunct;      /**< demodulator functions */
   pDRXAccessFunc_t  myAccessFunct;     /**< data access protocol functions */
   pTUNERInstance_t  myTuner;           /**< tuner instance, if NULL then baseband */
   pI2CDeviceAddr_t  myI2CDevAddr;      /**< i2c address and device identifier */
   pDRXCommonAttr_t  myCommonAttr;      /**< common DRX attributes */
   void*             myExtAttr;         /**< device specific attributes */
   /* generic demodulator data */
} DRXDemodInstance_t;



/*-------------------------------------------------------------------------
MACROS
Conversion from enum values to human readable form.
-------------------------------------------------------------------------*/

/* standard */

#define DRX_STR_STANDARD(x) ( \
   ( x == DRX_STANDARD_DVBT             )  ? "DVB-T"            : \
   ( x == DRX_STANDARD_8VSB             )  ? "8VSB"             : \
   ( x == DRX_STANDARD_AUTO             )  ? "Auto"             : \
   ( x == DRX_STANDARD_UNKNOWN          )  ? "Unknown"          : \
                                            "(Invalid)"  )

/* channel */

#define DRX_STR_BANDWIDTH(x) ( \
   ( x == DRX_BANDWIDTH_8MHZ           )  ? "8 MHz"            : \
   ( x == DRX_BANDWIDTH_7MHZ           )  ? "7 MHz"            : \
   ( x == DRX_BANDWIDTH_6MHZ           )  ? "6 MHz"            : \
   ( x == DRX_BANDWIDTH_UNKNOWN        )  ? "Unknown"          : \
                                            "(Invalid)"  )
#define DRX_STR_FFTMODE(x) ( \
   ( x == DRX_FFTMODE_2K               )  ? "2k"               : \
   ( x == DRX_FFTMODE_8K               )  ? "8k"               : \
   ( x == DRX_FFTMODE_AUTO             )  ? "Auto"             : \
   ( x == DRX_FFTMODE_UNKNOWN          )  ? "Unknown"          : \
                                            "(Invalid)"  )
#define DRX_STR_GUARD(x) ( \
   ( x == DRX_GUARD_1DIV32             )  ? "1/32nd"           : \
   ( x == DRX_GUARD_1DIV16             )  ? "1/16th"           : \
   ( x == DRX_GUARD_1DIV8              )  ? "1/8th"            : \
   ( x == DRX_GUARD_1DIV4              )  ? "1/4th"            : \
   ( x == DRX_GUARD_AUTO               )  ? "Auto"             : \
   ( x == DRX_GUARD_UNKNOWN            )  ? "Unknown"          : \
                                            "(Invalid)"  )
#define DRX_STR_CONSTELLATION(x) ( \
   ( x == DRX_CONSTELLATION_BPSK       )  ?  "BPSK"            : \
   ( x == DRX_CONSTELLATION_QPSK       )  ?  "QPSK"            : \
   ( x == DRX_CONSTELLATION_PSK8       )  ?  "PSK8"            : \
   ( x == DRX_CONSTELLATION_QAM16      )  ?  "QAM16"           : \
   ( x == DRX_CONSTELLATION_QAM32      )  ?  "QAM32"           : \
   ( x == DRX_CONSTELLATION_QAM64      )  ?  "QAM64"           : \
   ( x == DRX_CONSTELLATION_QAM128     )  ?  "QAM128"          : \
   ( x == DRX_CONSTELLATION_QAM256     )  ?  "QAM256"          : \
   ( x == DRX_CONSTELLATION_QAM512     )  ?  "QAM512"          : \
   ( x == DRX_CONSTELLATION_QAM1024    )  ?  "QAM1024"         : \
   ( x == DRX_CONSTELLATION_AUTO       )  ?  "Auto"            : \
   ( x == DRX_CONSTELLATION_UNKNOWN    )  ?  "Unknown"         : \
                                             "(Invalid)" )
#define DRX_STR_CODERATE(x) ( \
   ( x == DRX_CODERATE_1DIV2           )  ?  "1/2nd"           : \
   ( x == DRX_CODERATE_2DIV3           )  ?  "2/3rd"           : \
   ( x == DRX_CODERATE_3DIV4           )  ?  "3/4th"           : \
   ( x == DRX_CODERATE_5DIV6           )  ?  "5/6th"           : \
   ( x == DRX_CODERATE_7DIV8           )  ?  "7/8th"           : \
   ( x == DRX_CODERATE_AUTO            )  ?  "Auto"            : \
   ( x == DRX_CODERATE_UNKNOWN         )  ?  "Unknown"         : \
                                             "(Invalid)" )
#define DRX_STR_HIERARCHY(x) ( \
   ( x == DRX_HIERARCHY_NONE           )  ?  "None"            : \
   ( x == DRX_HIERARCHY_ALPHA1         )  ?  "Alpha=1"         : \
   ( x == DRX_HIERARCHY_ALPHA2         )  ?  "Alpha=2"         : \
   ( x == DRX_HIERARCHY_ALPHA4         )  ?  "Alpha=4"         : \
   ( x == DRX_HIERARCHY_AUTO           )  ?  "Auto"            : \
   ( x == DRX_HIERARCHY_UNKNOWN        )  ?  "Unknown"         : \
                                             "(Invalid)" )
#define DRX_STR_PRIORITY(x) ( \
   ( x == DRX_PRIORITY_LOW             )  ?  "Low"             : \
   ( x == DRX_PRIORITY_HIGH            )  ?  "High"            : \
   ( x == DRX_PRIORITY_UNKNOWN         )  ?  "Unknown"         : \
                                             "(Invalid)" )
#define DRX_STR_MIRROR(x) ( \
   ( x == DRX_MIRROR_NO                )  ?  "Normal"          : \
   ( x == DRX_MIRROR_YES               )  ?  "Mirrored"        : \
   ( x == DRX_MIRROR_AUTO              )  ?  "Auto"            : \
   ( x == DRX_MIRROR_UNKNOWN           )  ?  "Unknown"         : \
                                             "(Invalid)" )
#define DRX_STR_CLASSIFICATION(x) ( \
   ( x == DRX_CLASSIFICATION_GAUSS     )  ?  "Gaussion"        : \
   ( x == DRX_CLASSIFICATION_HVY_GAUSS )  ?  "Heavy Gaussion"  : \
   ( x == DRX_CLASSIFICATION_COCHANNEL )  ?  "Co-channel"      : \
   ( x == DRX_CLASSIFICATION_STATIC    )  ?  "Static echo"     : \
   ( x == DRX_CLASSIFICATION_MOVING    )  ?  "Moving echo"     : \
   ( x == DRX_CLASSIFICATION_ZERODB    )  ?  "Zero dB echo"    : \
   ( x == DRX_CLASSIFICATION_UNKNOWN   )  ?  "Unknown"         : \
   ( x == DRX_CLASSIFICATION_AUTO      )  ?  "Auto"            : \
                                             "(Invalid)" )
/* lock status */

#define DRX_STR_LOCKSTATUS(x) ( \
   ( x == DRX_NEVER_LOCK               )  ?  "Never"           : \
   ( x == DRX_NOT_LOCKED               )  ?  "No"              : \
   ( x == DRX_LOCKED                   )  ?  "Locked"          : \
   ( x == DRX_LOCK_STATE_1             )  ?  "Lock state 1"    : \
   ( x == DRX_LOCK_STATE_2             )  ?  "Lock state 2"    : \
   ( x == DRX_LOCK_STATE_3             )  ?  "Lock state 3"    : \
   ( x == DRX_LOCK_STATE_4             )  ?  "Lock state 4"    : \
   ( x == DRX_LOCK_STATE_5             )  ?  "Lock state 5"    : \
   ( x == DRX_LOCK_STATE_6             )  ?  "Lock state 6"    : \
   ( x == DRX_LOCK_STATE_7             )  ?  "Lock state 7"    : \
   ( x == DRX_LOCK_STATE_8             )  ?  "Lock state 8"    : \
   ( x == DRX_LOCK_STATE_9             )  ?  "Lock state 9"    : \
                                             "(Invalid)" )

/* version information , modules */
#define DRX_STR_MODULE(x) ( \
   ( x == DRX_MODULE_DEVICE            )  ?  "Device"                : \
   ( x == DRX_MODULE_MICROCODE         )  ?  "Microcode"             : \
   ( x == DRX_MODULE_DRIVERCORE        )  ?  "Drivercore"            : \
   ( x == DRX_MODULE_DEVICEDRIVER      )  ?  "Devicedriver"          : \
   ( x == DRX_MODULE_BSP_I2C           )  ?  "BSP I2C"               : \
   ( x == DRX_MODULE_BSP_TUNER         )  ?  "BSP Tuner"             : \
   ( x == DRX_MODULE_BSP_HOST          )  ?  "BSP Host"              : \
   ( x == DRX_MODULE_DAP               )  ?  "Data Access Protocol"  : \
   ( x == DRX_MODULE_UNKNOWN           )  ?  "Unknown"               : \
                                             "(Invalid)" )

#define DRX_STR_POWER_MODE(x) ( \
   ( x == DRX_POWER_UP                 )  ?  "DRX_POWER_UP    " : \
   ( x == DRX_POWER_MODE_1             )  ?  "DRX_POWER_MODE_1" : \
   ( x == DRX_POWER_MODE_2             )  ?  "DRX_POWER_MODE_2" : \
   ( x == DRX_POWER_MODE_3             )  ?  "DRX_POWER_MODE_3" : \
   ( x == DRX_POWER_MODE_4             )  ?  "DRX_POWER_MODE_4" : \
   ( x == DRX_POWER_MODE_5             )  ?  "DRX_POWER_MODE_5" : \
   ( x == DRX_POWER_MODE_6             )  ?  "DRX_POWER_MODE_6" : \
   ( x == DRX_POWER_MODE_7             )  ?  "DRX_POWER_MODE_7" : \
   ( x == DRX_POWER_MODE_8             )  ?  "DRX_POWER_MODE_8" : \
   ( x == DRX_POWER_MODE_9             )  ?  "DRX_POWER_MODE_9" : \
   ( x == DRX_POWER_DOWN               )  ?  "DRX_POWER_DOWN  " : \
                                             "(Invalid)" )

#define DRX_STR_OOB_DIRECTION(x) ( \
   ( x == DRX_OOB_DOWNSTREAM           )  ?  "Downstream" : \
   ( x == DRX_OOB_UPSTREAM             )  ?  "Upstream  " : \
                                             "(Invalid)" )

#define DRX_STR_OOB_STANDARD(x) ( \
   ( x == DRX_STANDARD_OOB_A           )  ?  "ANSI 55-2 A  " : \
   ( x == DRX_STANDARD_OOB_B           )  ?  "ANSI 55-2 B  " : \
   ( x == DRX_STANDARD_OOB_C           )  ?  "ANSI 55-2 C  " : \
   ( x == DRX_STANDARD_OOB_STD         )  ?  "ANSI 55-1 std" : \
   ( x == DRX_STANDARD_OOB_EXT         )  ?  "ANSI 55-2 ext" : \
                                             "(Invalid)" )

#define DRX_STR_OOB_BITRATE(x) ( \
   ( x == DRX_BITRATE_256KBS           )  ?  "256  Kbs" : \
   ( x == DRX_BITRATE_1544KBS          )  ?  "1544 Kbs" : \
   ( x == DRX_BITRATE_2048KBS          )  ?  "2048 Kbs" : \
   ( x == DRX_BITRATE_3088KBS          )  ?  "3088 Kbs" : \
                                             "(Invalid)" )

/*-------------------------------------------------------------------------
Exported FUNCTIONS
-------------------------------------------------------------------------*/

DRXStatus_t DRX_Init( pDRXDemodInstance_t demods[] );

DRXStatus_t DRX_Term( void );

DRXStatus_t DRX_Open(pDRXDemodInstance_t demod, DtDevice *device);

DRXStatus_t DRX_Close(pDRXDemodInstance_t demod, DtDevice *device);

DRXStatus_t DRX_Ctrl(pDRXDemodInstance_t demod, DRXCtrlIndex_t ctrl, void *ctrlData, DtDevice *device);

/*-------------------------------------------------------------------------
THE END
-------------------------------------------------------------------------*/
#ifdef __cplusplus
}
#endif
#endif /* __DRXDRIVER_H__ */
