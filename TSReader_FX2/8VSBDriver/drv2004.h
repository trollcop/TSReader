/**********************************************************************
 * DRV2004.H
 * Public Include File for DRV2004 users
 *
 * Copyright (C) 2002 NxtWave Communications, Inc.
 *
 * $Log:   //panxtbdc01/AppsEng/PVCS/Application Eval/archives/EVAL2004/Include/drv2004.h-arc  $
 * 
 *    Rev 1.19   May 20 2003 18:15:14   raggarwa
 * Updated comments for NxtGetBertData
 * 
 *    Rev 1.18   May 01 2003 16:28:36   raggarwa
 * Fixed the function prototype for NxtSetMpegMode( )
 * 
 *    Rev 1.17   Feb 11 2003 13:35:22   raggarwa
 * Updated for release 3.0.0
 * 
 *    Rev 1.16   Jan 30 2003 12:10:32   sreichgo
 * Changed line 238 for ANSI compiler error.
 * 
 *    Rev 1.15   Jan 21 2003 17:02:48   raggarwa
 * Updated comments for NxtSetAdcInputGain() and NxtOutputControl() APIs
 * 
 *    Rev 1.14   Nov 07 2002 17:42:44   raggarwa
 * Changed AGC token defines, PR 46/2004
 * 
 *    Rev 1.13   Oct 01 2002 14:08:12   raggarwa
 * Updated comments
 * 
 *    Rev 1.12   Jun 26 2002 10:56:30   raggarwal
 * Removed ADC_1 from NxtSetAdcControl_t;
 * Removed NXT_BERT_INPUT_FDC from NxtInternalBert_t
 * 
 *    Rev 1.11   Jun 18 2002 09:33:46   raggarwal
 * Alpha3 Release
 * 
 *    Rev 1.10   May 17 2002 16:32:34   raggarwal
 * Alpha2 Release
 * 
 *    Rev 1.5   Mar 08 2002 16:09:48   raggarwal
 * Alpha Release
 * 
 * 
 **********************************************************************/

#ifndef DRV2004_H
#define DRV2004_H

/*
******************************************************************************
Defines
******************************************************************************
*/

/* The following directives are used when creating a Windows DLL from the
 * DRV2004 API.  Typical application software should disregard these directives.
 */
#ifdef DRV2004_DLL
	#ifdef DRV2DLL_EXPORTS
		#define DRV2_API __declspec(dllexport)
	#else
		#define DRV2_API __declspec(dllimport)
	#endif
#else
	#define DRV2_API
#endif

#define DATA8_TO_DATA16(x)  (((Data16) (*(x)))<<8) | (((Data16) (*((x)+1))) & 0x00ff)
#define DATA8_TO_DATA32(x)  ((((Data32) DATA8_TO_DATA16(x))<<16) | (((Data32) DATA8_TO_DATA16((x)+2))) & 0x0000ffff)
#define NEGATE(x) ((~x)+1)
#define SIGN_EXT(x,len) (((x) & ((Data32)1 << (len-1))) ? ((x) | ((Data32)-1 << (len-1) )) : ((Data32)x))

#define	NXT_CRYSTAL_FREQ	25.14	/* MHz */

#define AGC_RAM_SIZE		192

/* Download/Upload maximum block transfer size */
#define MAX_XFER_BLOCK_SIZE	60

/* Maximum number of download attempts with crc error */
#define MAX_DNLD_TRIES		3

#define RAM_BASE_ROM_ENABLED	0x1000
#define RAM_BASE_ROM_DISABLED	0x0000
#define	UC_RAM_SIZE				0xA000
#define DI_RAM_END				0xFDFF

/* Interrupt Mask/Source Values */
#define NXT_INT_FAT_LOCK		0x80
#define NXT_INT_FAT_LOSS		0x40
#define NXT_INT_NTSC			0x20
#define NXT_INT_XFER			0x10
#define NXT_INT_FDC_LOCK		0x02
#define NXT_INT_FDC_LOSS		0x01

/* Interrupt Result/Status Values */
#define NXT_NTSC_DETECTED		0x01
#define NXT_XFER_ERROR			0x02

/* Acquisition Start/Stop Masks*/
#define	NXT_START_NO_CO_MASK	0x0400	/* bit to disable co-channel */
#define	NXT_START_ADJ_MASK		0x0800	/* bit to enable adjacent detection */
#define	NXT_START_CHANNEL_MASK	0xF000
#define	NXT_STOP_OPTIONS_MASK	0x00E0
#define	NXT_STOP_ASYNC_MASK		0x0020

/* AGC Setup Script Token Values */
#define NXT_AGC_SETUP_DONE		-1	/* end of agc setup data */
#define NXT_AGC_SETUP_64QAM		-2	/* start of 64Qam modifications */
#define NXT_AGC_SETUP_256QAM	-3	/* start of 256Qam modifications */
#define	NXT_FAT_AGC_SETUP_ADJ	-4	/* start of fat adj channel modifications */
#define NXT_AGC_IF_THRESHOLD	-5	/* start of FAT IF input thresholds */
#define NXT_AGC_RF_THRESHOLD	-6	/* start of FAT RF input thresholds */


/* API Return Error Codes */
#define	NXT_NO_ERROR        0
#define NXT_ERR_INIT        1	/* driver not initialized						*/
#define	NXT_ERR_RANGE       2	/* input parameter out-of-range					*/
#define	NXT_ERR_RESET       4	/* chip held in reset - output data invalid		*/
#define	NXT_ERR_MODE        8	/* current mode invalid for requested API		*/
#define	NXT_ERR_NO_LOCK    16	/* no frame/mpeg lock - output data invalid		*/
#define	NXT_ERR_MEMORY     32	/* failed to allocate memory for device context */
#define	NXT_ERR_OS         64	/* operating system call returned error			*/
#define	NXT_ERR_COMM      128	/* communication failure with NXT2005			*/
#define	NXT_ERR_OTHER     256	/* other error (unsupported, etc.)				*/
#define	NXT_ERR_TIMEOUT   512	/* service timed out							*/
#define	NXT_ERR_IIC_XFER  1024	/* Nxt2005 reported IIC Xfer failure			*/
#define	NXT_ERR_NOT_READY 2048	/* chip/data not ready for requested API		*/

/*
******************************************************************************
Public Types
******************************************************************************
*/


/* Acquisition Options */
typedef enum {
	NXT_DIRECT_TUNE					= 0,
	/* Start Options */
	NXT_CONFIG_8VSB					= 0x8000,
	NXT_CONFIG_8VSB_NO_CO			= 0x8400,
	NXT_CONFIG_8VSB_ADJ				= 0x8800,
	NXT_CONFIG_8VSB_ADJ_NO_CO		= 0x8C00,
	NXT_CONFIG_64QAM				= 0x4000,
	NXT_CONFIG_256QAM				= 0x2000,
	NXT_CONFIG_CABLE				= 0xE000,	/* 64QAM, 256QAM, 8VSB in that order */
	NXT_ACTIVE_TRACKING_FAT			= 0x0200,

	/* Stop Options */
	NXT_STOP_FAT					= 0x0080,
	NXT_STOP_FAT_ASYNC				= 0x00A0
} NxtAcqOptions_t;


/* Read (upload) type */
typedef enum {
	NXT_READ_UC_RAM,
	NXT_READ_AGC_RAM
} NxtReadType_t;

/* Download Type */
typedef enum {
	NXT_LOAD_CODE = 0,
	NXT_LOAD_AGC = 2,
	NXT_LOAD_OTHER = 3
} NxtLoadType_t;

/* Signal Status */
typedef enum {
	NXT_SIG_NO_SIGNAL,
	NXT_SIG_WEAK,
	NXT_SIG_MODERATE,
	NXT_SIG_STRONG,
	NXT_SIG_VERY_STRONG
} NxtSignalState_t;

/* In-band Modulation Formats */
typedef enum {
	NXT_256QAM,
	NXT_64QAM,
	NXT_16VSB,
	NXT_8VSB
} NxtModFormat_t;

/* IIC Bypass Modes */
typedef enum {
	NXT_IIC_BYPASS,
	NXT_IIC_UC_CONTROL
} NxtBypass_t;

/* IIC Transfer Modes */
typedef enum {
	NXT_IIC_READ,
	NXT_IIC_WRITE
} NxtIicXferMode_t;

/* IIC Transfer Speeds */
enum {
	NXT_IIC_SPEED_FASTEST	= 0x00,	/* approx. 238 KHz */
	NXT_IIC_SPEED_STANDARD	= 0x03,	/* approx. 100 KHz */
	NXT_IIC_SPEED_SLOWEST	= 0x7F	/* approx. 4.8 KHz */
};

/* Polarities */
typedef enum {
	NON_INVERTED,
	INVERTED
} NxtPolarity_t;

/* Code Version */
typedef struct {
	Data8 major;
	Data8 custom;
	Data8 minor;
} CodeVersion_s;

/* ASIC Version */
typedef struct {
	Data8 device;
	Data8 fab;
	Data8 month;
	Data8 year[2];
} AsicVersion_s;

/* GPIO Control Modes */
typedef enum {
	NXT_GPIO_ASSIGN			= 0xC0,
	NXT_GPIO_SET_IO			= 0x80,
	NXT_GPIO_WRITE			= 0x40,
	NXT_GPIO_READ			= 0x00
} NxtGpioMode_t;

/* Controls used for NxtCoreControl() */
typedef enum {
	CORE_POWER_DOWN_ALL	= 0x70,		/* power down all cores */
	CORE_POWER_UP_ALL	= 0x71,		/* power up all cores */
	CORE_POWER_UP_FAT	= 0x12,		/* power up FAT */
	CORE_POWER_DOWN_FAT	= 0x13,		/* power down FAT */
	CORE_SOFT_RESET_ALL		= 0x78,	/* soft reset all cores - cleared by driver */
	CORE_SOFT_RESET_BERT	= 0x39,	/* soft reset BERT - cleared by driver */
	CORE_SOFT_RESET_SMOOTHER= 0x1B,	/* soft reset MPEG output smoother - cleared by driver */
	CORE_SOFT_RESET_FAT_AGC	= 0x1C	/* soft reset FAT AGC - cleared by driver */
} NxtCoreControl_t;

/* ADC gain control types */
typedef enum {
	NXT_FAT_ADC_GAIN_1V,
	NXT_FAT_ADC_GAIN_2V
} NxtAdcGainControl_t;

/* Internal BERT input types */
typedef enum {
	NXT_BERT_INPUT_FAT		= 0x00,
	NXT_BERT_INPUT_DI		= 0x08,
	NXT_BERT_INPUT_TRELLIS	= 0x18
} NxtBertSource_t;

/* Internal BERT PN sequence algorithms */
typedef enum {
	NXT_BERT_PN_15 = 0x02,
	NXT_BERT_PN_23 = 0x00
} NxtBertPnSequence_t;

/* Internal BERT header remove byte count */
typedef enum {
	NXT_BERT_RM_0 = 0x00,	/* remover no bytes */
	NXT_BERT_RM_1 = 0x20,	/* remover 1 byte */
	NXT_BERT_RM_3 = 0x40,	/* remover 3 bytes */
	NXT_BERT_RM_4 = 0x60	/* remover 4 bytes */
} NxtBertHeaderRemove_t;

/* Control mode enable/disable/tristate */
typedef enum {
	NXT_DISABLE = 0,
	NXT_ENABLE	= 1,
	NXT_TRISTATE= 0
} NxtControl_t;


/*
******************************************************************************
Public Functions
******************************************************************************
*/

/**********************************************************************/
/*                          Initialization APIs                       */
/**********************************************************************/

/**********************************************************************
 *
 * NxtInit2004Driver
 *
 * Initializes NXT2004 driver and ASIC.
 *
 * Inputs:
 *	void *pContext	- NULL, or user-defined ID for multiple NXT2004s
 *
 * Returns:
 *	Data16
 *
 * Notes:
 *
 *	Acquisition is stopped following this call.
 *
 **********************************************************************/
DRV2_API Data16 NxtInit2004Driver(void *pContext);


/**********************************************************************
 *
 * NxtInit2004ASIC
 *
 * Initializes NXT2004 ASIC and reloads current firmware
 *
 * Inputs:
 *	void *pContext	- NULL, or user-defined ID for multiple NXT2004s
 *
 * Returns:
 *	Data16
 *
 * Notes: 
 *	This should be used after a hardware reset is performed.
 *	Acquisition is stopped following this call.
 *
 **********************************************************************/
DRV2_API Data16 NxtInit2004ASIC(void *pContext);

/**********************************************************************
 *
 * NxtExitDriver
 *
 * Stops the NXT200X, background tasks, and returns system resources.
 *
 * Inputs:
 *	void *pContext - NULL, or user-defined ID for multiple NXT200Xs
 *
 * Returns:
 *	Data16
 *
 **********************************************************************/
DRV2_API Data16 NxtExitDriver(void *pContext);


/**********************************************************************/
/*                          Acquisition APIs                          */
/**********************************************************************/


/**********************************************************************
 *
 * NxtStart
 *
 * Initiates acquisition or tracking
 *
 * Inputs:
 *	void *pContext - NULL, or user-defined ID for multiple NXT200Xs
 *	NxtAcqOptions_t startOptions - Acquisition modes and co-channel 
 *						disable, adj enable options
 *
 * Outputs:
 *	Data16 *pResult - Indicates success modes and options
 *				copy of startOptions if all modes and options
 *				selected were successful
 *
 * Returns:
 *	Data16
 *
 * Notes:
 *	Notification functions will be called to report lock status.
 *	NXT_ACTIVE_TRACKING_ mode should be called from a background task!
 * 
 *	A NXT_ERR_MODE error is returned if the NXT200X is in low-power
 *	mode; no start function should be executed.
 *
 **********************************************************************/
DRV2_API Data16 NxtStart(void *pContext,
					NxtAcqOptions_t startOptions,
					Data16 *pResult);

/**********************************************************************
 *
 * NxtStop
 *
 * Stops acquisition and tracking.
 *
 * Inputs:
 *	void *pContext - NULL, or user-defined ID for multiple NXT200Xs
 *	NxtAcqOptions_t  stopOptions - 	Channel to be stopped,
 *							if function should return asynchronously
 *
 * Returns:
 *	Data16
 *
 * Notes:
 *	If calling within a notification function, use ASYNC option
 *  If ASYNC option is selected, it returns before background tasks
 *	have ended.
 *
 **********************************************************************/
DRV2_API Data16 NxtStop(void *pContext, 
					NxtAcqOptions_t stopOptions);


/**********************************************************************/
/*                              Query APIs                            */
/**********************************************************************/

/**********************************************************************
 *
 * NxtGetFatLockStatus
 *
 * Obtains the current frame/mpeg lock status in FAT channel.
 *
 * Inputs:
 *	void *pContext - NULL, or user-defined ID for multiple NXT200Xs
 *
 * Outputs:
 *	Bool *pbFatLocked - TRUE if frame/mpeg lock achieved
 *
 * Returns:
 *	Data16
 *
 *
 **********************************************************************/
DRV2_API Data16 NxtGetFatLockStatus(void *pContext,
					Bool *pbFatLocked );

/**********************************************************************
 *
 * NxtGetRSErrCount
 *
 * Obtains the number of uncorrectable Reed-Solomon errors in FAT 
 * channel accumulated since the last read.
 *
 * Inputs:
 *	void *pContext - NULL, or user-defined ID for multiple NXT200Xs
 *
 * Outputs:
 *	Data8 *pErrCount - number of Reed-Solomon errors since last read
 *
 * Returns:
 *	Data16
 *
 * Notes:
 *	The output value is only valid if the current channel is locked.
 *
 **********************************************************************/
DRV2_API Data16 NxtGetRSErrCount(void *pContext,
					Data8 *pErrCount);

/**********************************************************************
 *
 * NxtReadRAM
 *
 * Obtains a copy of data from microcontroller or AGC RAM
 *
 * Inputs:
 *	void *pContext - NULL, or user-defined ID for multiple NXT200Xs
 *	NxtReadType_t ucOrAgc - selects microcontroller or AGC RAM
 *	Data16 startAddress - first address to read
 *	Data16 byteCount - number of bytes to read
 *
 * Outputs:
 *	Data8 *pBuffer - RAM data copied to user-provided buffer
 *
 * Returns:
 *	Data16
 *
 **********************************************************************/
DRV2_API Data16 NxtReadRAM(void *pContext,
					NxtReadType_t ucOrAgc,
					Data16 startAddress,
					Data16 byteCount,
					Data8 *pBuffer );

/**********************************************************************
 *
 * NxtGetRegister
 *
 * Obtains a copy of register data from NXT200X
 *
 * Inputs:
 *	void *pContext - NULL, or user-defined ID for multiple NXT200Xs
 *	Data16 deviceRegister - first NXT200X register to read
 *	Data8 byteCount - number of bytes to read
 *
 * Outputs:
 *	Data8 *pBuffer - register data copied to user-provided buffer
 *
 * Returns:
 *	Data16
 *
 * Notes:
 *	Use this API instead of the IIC driver equivalent to get critical
 *	section protection.
 *
 *	deviceRegister is a 16 bit value- the upper byte is the IIC	device 
 *	ID (00 or 01) and the lower byte is the register offset. These two 
 *	bytes are appended to generate 16-bit deviceRegister.
 *
 **********************************************************************/
DRV2_API Data16 NxtGetRegister(void *pContext,
					Data16 deviceRegister,
					Data8 byteCount,
					Data8 *pBuffer);

/**********************************************************************
 *
 * NxtGetFatSQI
 *
 * Obtains the cluster variance, a measure of signal quality in FAT channel
 *
 * Inputs:
 *	void *pContext - NULL, or user-defined ID for multiple NXT200Xs
 *
 * Outputs:
 *	Data16 *pFatSQI - register data copied to user-provided buffer
 *
 * Returns:
 *	Data16
 *
 * Notes:
 *	SQI is only valid if frame/mpeg lock has been achieved.
 *
 **********************************************************************/
DRV2_API Data16 NxtGetFatSQI(void *pContext,
					  Data16 *pFatSQI);

/**********************************************************************
 *
 * NxtGetFatSignalStatus
 *
 * Obtains a state indicating signal power, and a measure of quality 
 * for FAT channel
 *
 * Inputs:
 *	void *pContext - NULL, or user-defined ID for multiple NXT200Xs
 *
 * Outputs:
 *	NxtSignalState_t *pFatSignalState - a state indicating signal power
 *	Data8 *pFatSignalMetric - value 0-10, relative quality in given state
 *
 * Returns:
 *	Data16
 *
 **********************************************************************/
DRV2_API Data16 NxtGetFatSignalStatus(void *pContext,
					NxtSignalState_t *pFatSignalState,
					Data8 *pFatSignalMetric);

/**********************************************************************
 *
 * NxtGetFatFreqOffset
 *
 * Obtains the value, in Hz, of FAT IF frequency offset from nominal.
 *
 * Inputs:
 *	void *pContext - NULL, or user-defined ID for multiple NXT200Xs
 *
 * Outputs:
 *	Data32 *pFatFreqOffset - frequency offset, Hz
 *
 * Returns:
 *	Data16
 *
 * Notes:
 *	Offset is only valid if frame/mpeg lock has been achieved.
 *
 **********************************************************************/
DRV2_API Data16 NxtGetFatFreqOffset(void *pContext,
					Data32 *pFatFreqOffset);


/**********************************************************************
 *
 * NxtGetFatModFormat
 *
 * Obtains the current modulation format used by the NXT200X
 *
 * Inputs:
 *	void *pContext - NULL, or user-defined ID for multiple NXT200Xs
 *
 * Outputs:
 *	NxtModFormat_t *pFatModFormat - current modulation format.
 *
 * Returns:
 *	Data16
 *
 * Notes:
 *	Modulation format setting is valid regardless of the state of the
 *	NXT200X ASIC -- this is not an indication of the type of signal
 *	being received, but of the type that is (or would be) being attempted
 *	for reception by the ASIC.
 *
 **********************************************************************/
DRV2_API Data16 NxtGetFatModFormat(void *pContext,
					NxtModFormat_t *pFatModFormat);


/**********************************************************************
 *
 * NxtGetBertData
 *
 * Obtains the accumulated error count in BERT.
 *
 * Inputs:
 *	void *pContext - NULL, or user-defined ID for multiple NXT200Xs
 *	Bool bWait - Wait till there in new BERT data
 *
 * Outputs:
 *	Data32 *pErrorCount -- BERT data accumulated since the last read.
 *
 * Returns:
 *	NXT_NO_ERROR - No error
 *	NXT_ERR_MODE - BERT is not running
 *	NXT_ERR_NOT_READY - No new error data in the registers
 *		If bWait is specified as TRUE, this error code will never be returned.
 *	NXT_ERR_NO_LOCK - BERT is not locked, 
 *				OR BERT is not synchronized,
 *				OR equalizer is in "acquire" state (if mod format is VSB),
 *				OR FEC is not locked (if mod format is QAM)
 *						
 * Notes:
 *	The user can specify if they want to wait till the error data is
 *	ready. If the user doesn't specify that, and the data is not ready
 *	then the function will return error.	
 *
 **********************************************************************/
DRV2_API Data16 NxtGetBertData(void *pContext,
					Data32 *pErrorCount,
					Bool bWait);


/**********************************************************************
 *
 * NxtGetDriverVersion
 *
 * Obtains the version of DRV200X currently executing.
 *
 * Inputs:
 *	none -- one DRV200X instance serves all contexts!
 *
 * Outputs:
 *	CodeVersion_s *pCodeVersion -- structure containing version info
 *
 * Returns:
 *	NXT_NO_ERROR
 *
 **********************************************************************/
DRV2_API Data16 NxtGetDriverVersion(CodeVersion_s *pCodeVersion);


/**********************************************************************
 *
 * NxtGetAsicVersion
 *
 * Obtains the version of the referenced NXT200X ASIC.
 *
 * Inputs:
 *	void *pContext - NULL, or user-defined ID for multiple NXT200Xs
 *
 * Outputs:
 *	AsicVersion_s *pAsicVersion -- structure containing version info
 *
 * Returns:
 *	Data16
 *
 **********************************************************************/
DRV2_API Data16 NxtGetAsicVersion(void *pContext,
					AsicVersion_s *pAsicVersion);


/**********************************************************************
 *
 * NxtGetRomVersion
 *
 * Obtains the version of the ROM Code in the referenced NXT200X ASIC.
 *
 * Inputs:
 *	void *pContext - NULL, or user-defined ID for multiple NXT200Xs
 *
 * Outputs:
 *	CodeVersion_s *pCodeVersion -- structure containing version info
 *
 * Returns:
 *	Data16
 *
 **********************************************************************/
DRV2_API Data16 NxtGetRomVersion(void *pContext,
					CodeVersion_s *pCodeVersion);

/**********************************************************************
 *
 * NxtGetRamVersion
 *
 * Obtains the version of the RAM Code running in the referenced NXT200X.
 *
 * Inputs:
 *	void *pContext - NULL, or user-defined ID for multiple NXT200Xs
 *
 * Outputs:
 *	CodeVersion_s *pCodeVersion -- structure containing version info
 *
 * Returns:
 *	Data16
 *
 * Notes:
 *	This API is only valid if the NXT200X is actually executing RAM Code.
 *
 **********************************************************************/
DRV2_API Data16 NxtGetRamVersion(void *pContext,
					CodeVersion_s *pCodeVersion);

/**********************************************************************
 *
 * NxtGetNtscStatus
 *
 * Determines if the current channel has an NTSC signal present
 *
 * Inputs:
 *	void *pContext - NULL, or user-defined ID for multiple NXT200Xs
 *
 * Outputs:
 *	Bool *pbNtsc - TRUE if NTSC is present, FALSE otherwise
 *
 * Returns:
 *	Data16
 *
 * Notes:
 *	The NTSC detection feature cannot be used while acquiring a 
 *	digital signal.  Using NxtGetNtscStatus while acquiring or 
 *	locked on a digital signal will stop acquisition of digital
 *	data.  Use NxtStart() to re-start acquisition.
 *	
 **********************************************************************/
DRV2_API Data16 NxtGetNtscStatus(void *pContext,
					Bool *pbNtsc);


/**********************************************************************/
/*                          Configuration APIs                        */
/**********************************************************************/


/**********************************************************************
 *
 * NxtIicBypass
 *
 * Sets the NXT200X IIC Bypass Switch position
 *
 * Inputs:
 *	void *pContext - NULL, or user-defined ID for multiple NXT200Xs
 *	NxtBypass_t bypass - NXT_IIC_BYPASS or NXT_IIC_UC_CONTROL
 *
 * Returns:
 *	Data16
 *
 * Notes:
 *	The user must insure any microcontroller use of the auxiliary 
 *  IIC bus has completed before using this API.
 *
 **********************************************************************/
DRV2_API Data16 NxtIicBypass(void *pContext,
					NxtBypass_t bypass);

/**********************************************************************
 *
 * NxtIicXfer
 *
 * Initiates IIC data transfer controlled by the NXT200X microcontroller
 *
 * Inputs:
 *	void *pContext - NULL, or user-defined ID for multiple NXT200Xs
 *	NxtIicXferMode_t mode - NXT_IIC_READ or NXT_IIC_WRITE
 *	Data8 speed - value from 0x00 (fastest) to 0xFF (slowest); see notes
 *	Data8 byteCount - number of bytes to transfer
 *	Data8 deviceAddress - IIC device address (LSB ignored)
 *	Data8 *pBuffer - for NXT_IIC_WRITE, buffer contains write data
 *
 * Outputs:
 *	Data8 *pBuffer - for NXT_IIC_READ, buffer contains read data
 *
 * Returns:
 *	Data16
 *
 * Notes:
 *	The following speed values are pre-defined for convenience:
 *		NXT_IIC_SPEED_FASTEST = 0x00	- approximately 238 KHz
 *		NXT_IIC_SPEED_STANDARD = 0x03	- approximately 100 KHz
 *		NXT_IIC_SPEED_SLOWEST = 0x7F	- approximately 4.9 KHz
 *	The user is free to choose any Data8 value <= 0x7F.  Note that the 
 *	speed settings do not relate to the bus speed in a linear fashion.
 *
 *	This API is inhibited if NxtIicBypass() has set the NXT_IIC_BYPASS
 *	mode.
 *
 *	This API does not interfere with NXT200X digital signal acquisition.
 *
 *	If the NXT_INT_XFER is enabled, this API returns immediately.  Use
 *	NxtReadIicXferData() to complete a NXT_IIC_READ operation, following
 *	the transfer completion interrupt.
 *
 *	Due to I/O constraints, this API will not operate properly if the 
 *	NXT200X is in low-power mode.  The transfer will be attempted, but
 *	an error will result.
 *
 **********************************************************************/
DRV2_API Data16 NxtIicXfer(	void *pContext,
					NxtIicXferMode_t mode,
					Data8 speed,
					Data8 byteCount,	
					Data8 deviceAddress,
					Data8 *pBuffer);

/**********************************************************************
 *
 * NxtSetMpegPolarity
 *
 * Sets the polarity of MPEG outputs
 *
 * Inputs:
 *	void *pContext - NULL, or user-defined ID for multiple NXT200Xs
 *	NxtPolarity_t dataEnablePolarity - data enable INVERTED or NON_INVERTED
 *	NxtPolarity_t pktSyncPolarity - packet sync INVERTED or NON_INVERTED
 *	NxtPolarity_t errorPolarity - MPEG error INVERTED or NON_INVERTED
 *	NxtPolarity_t clockPolarity - byte/bit clock INVERTED or NON_INVERTED
 *
 * Returns:
 *	Data16
 *
 * Notes:
 *	All outputs default to NON_INVERTED if this call is never made.
 *
 **********************************************************************/
DRV2_API Data16 NxtSetMpegPolarity(void *pContext,
					NxtPolarity_t dataEnablePolarity,
					NxtPolarity_t pktSyncPolarity,
					NxtPolarity_t errorPolarity,
					NxtPolarity_t clockPolarity);


/**********************************************************************
 *
 * NxtSetMpegMode
 *
 * Sets the MPEG output format
 *
 * Inputs:
 *	void *pContext - NULL, or user-defined ID for multiple NXT200Xs
 *	Bool bGatedOutputEnable - TRUE for gated output, FALSE for clocked
 *  Bool bContinuousRateEnable - TRUE enables continuous rate, FALSE disables
 *	Bool bParallelOutputEnable - TRUE for parallel, FALSE for serial
 *	Bool bHeaderEnable - TRUE enables MPEG header, etc.,  FALSE disables
 *
 * Returns:
 *	Data16
 *
 * Notes:
 *	bGatedOutputEnable, bParallelOutputEnable and bHeaderEnable values 
 *	default to TRUE, and bContinuousRateEnable defaults to FALSE if this 
 *	call is never made.
 *
 *	bHeaderEnable affects not only the MPEG header, but also whether the
 *	checksum is replaced with a sync byte, and whether the packet error
 *	flag is set in the output packet.  The purpose of bHeaderEnable = FALSE
 *	is to allow bit-error-rate testing.
 *
 **********************************************************************/
DRV2_API Data16 NxtSetMpegMode(void *pContext,
					Bool bGatedOutputEnable,
					Bool bContinuousRateEnable,
					Bool bParallelOutputEnable,
					Bool bHeaderEnable);

/**********************************************************************
 *
 * NxtSetFatAgcData
 *
 * Sets FAT AGC data.
 *
 * Inputs:
 *	void *pContext - NULL, or user-defined ID for multiple NXT200Xs
 *  Data16 *pFatAgcData - pointer to FAT AGC data
 *
 * Returns:
 *	Data16
 *
 * Notes:
 *	
 *
 **********************************************************************/
DRV2_API Data16 NxtSetFatAgcData(void *pContext,
					Data16 *pFatAgcData);

/**********************************************************************
 *
 * NxtSetFatAgcSdmPolarity
 *
 * Sets the AGC output polarity separately for each SDM output
 *
 * Inputs:
 *	void *pContext - NULL, or user-defined ID for multiple NXT200Xs
 *	NxtPolarity_t sdm1Polarity - INVERTED or NON_INVERTED for SDM 1 output
 *	NxtPolarity_t sdm2Polarity - INVERTED or NON_INVERTED for SDM 2 output
 *	NxtPolarity_t sdmXPolarity - INVERTED or NON_INVERTED for SDM X output
 *	NxtPolarity_t sdmAPolarity - INVERTED or NON_INVERTED for SDM A output
 *
 * Returns:
 *	Data16
 *
 * Notes:
 *	SDM1, SDM2 and SDMA polarities default to INVERTED and SDMX polarity defaults
 *	to NON_INVERTED if this function is not called.
 *
 **********************************************************************/
DRV2_API Data16 NxtSetFatAgcSdmPolarity(void *pContext,
					NxtPolarity_t sdm1Polarity,
					NxtPolarity_t sdm2Polarity,
					NxtPolarity_t sdmXPolarity,
					NxtPolarity_t sdmAPolarity);

/**********************************************************************
 *
 * NxtLoadRAM
 *
 * Copies from a user-supplied buffer to AGC or microcontroller RAM
 *
 * Inputs:
 *	void *pContext - NULL, or user-defined ID for multiple NXT200Xs
 *	NxtLoadType_t loadType - code, AGC, or other kind of download.
 *	Data16 startAddress - first RAM address to be written
 *	Data16 byteCount - number of RAM addresses to write
 *	Data8 *pBuffer - user-supplied buffer with RAM content to write
 *
 * Returns:
 *	Data16
 *
 * Notes:
 *	See API User's Guide for valid range information.
 *	The microcontroller is held in reset during this operation,
 *	and is released from reset at the end of the operation.
 *	Acquisition is stopped during the operation. Use NxtStart() to 
 *	restart acquisition following RAM loading.
 *
 **********************************************************************/
DRV2_API Data16 NxtLoadRAM(void *pContext,
					NxtLoadType_t loadType,
					Data16 startAddress,
					Data16 byteCount,
					Data8 *pBuffer);


/**********************************************************************
 *
 * NxtSetRegister
 *
 * Copies data to one or more contiguous registers in the referenced ASIC.
 *
 * Inputs:
 *	void *pContext - NULL, or user-defined ID for multiple NXT200Xs
 *	Data16 deviceRegister - device and register info about first register to write
 *	Data8 byteCount - number of registers to write, 0 - 255 (in range)
 *	Data8 *pBuffer - user-supplied data to write to registers
 *
 * Returns:
 *	Data16
 *
 * Notes:
 *	Use this function instead of the IIC driver equivalent in order to
 *	include critical-section protection.
 *
 *	deviceRegister is a 16 bit value- the upper byte is the IIC	device 
 *	ID (00 or 01) and the lower byte is the register offset. These two 
 *	bytes are appended to generate 16-bit deviceRegister.
 *
 **********************************************************************/
DRV2_API Data16 NxtSetRegister(void *pContext,
					Data16 deviceRegister,
					Data8  byteCount,
					Data8 *pBuffer);

/**********************************************************************
 *
 * NxtGpioControl
 *
 * Controls the general purpose I/O pins on the Nxt200X
 *
 * Inputs:
 *	void *pContext - NULL, or user-defined ID for multiple NXT200Xs
 *	NxtGpioMode_t mode - one of the following values:
 *		NXT_GPIO_ASSIGN - assigns GPIO pins for User or Nxt200X control
 *		NXT_GPIO_SET_IO - sets User-Controlled pins as Input or Output
 *		NXT_GPIO_READ - reads User-Controlled Inputs
 *		NXT_GPIO_WRITE - writes User-Controlled Outputs
 *	Data8 *pData - for NXT_GPIO_ASSIGN, contains the assignment mask
 *	             - for NXT_GPIO_SET_IO, contains the Input/Output mask
 *	             - for NXT_GPIO_WRITE, contains data to write
 *
 * Outputs:
 *	Data8 *pData - for NXT_GPIO_READ, contains read data
 *
 * Returns:
 *	Data16
 *
 * Notes:
 *	The use of this API is optional; if it is never called, all GPIO
 *	pins remain assigned to Nxt200X control.  Refer to the User's
 *	guide for default Nxt200X assignment, and full details on the use
 *	of this API.
 *
 *	Due to I/O constraints, the NXT_GPIO_WRITE operation will not 
 *	operate correctly if the NXT200X is in low-power mode.  An error
 *	NXT_ERR_MODE is returned if low-power mode is detected.
 *
 **********************************************************************/
DRV2_API Data16 NxtGpioControl(	void *pContext,
					NxtGpioMode_t mode,
					Data16 *pData);

/**********************************************************************
 *
 * NxtCoreControl
 *
 * Optionally resets/powers-down selected core sections.
 *
 * Inputs:
 *	void *pContext - NULL, or user-defined ID for multiple NXT200Xs
 *	NxtCoreControl_t coreControl - picks core section to control
 *
 * Returns:
 *	Data16
 *
 * Notes:
 *	Acquisition is stopped in this API.  Use NxtStart()
 *	to restart acquisition after any core control.
 *
 **********************************************************************/
DRV2_API Data16 NxtCoreControl(void *pContext,
					NxtCoreControl_t coreControl);

/**********************************************************************
 *
 * NxtSetAdcInputGain
 *
 * Sets the peak-to-peak input gain in FAT_ADC and FDC_ADC
 *
 * Inputs:
 *	void *pContext - NULL, or user-defined ID for multiple NXT200Xs
 *	NxtAdcGainControl_t adcGainCtl - one of	NxtAdcGainControl_t
 *
 * Returns:
 *	Data16
 *
 * Notes:
 *	This function should be called to set the FAT_ADC and FDC_ADC gain
 *	input. The default is 2V peak-to-peak for both FAT_ADC and FDC_ADC.
 *
 *	Only one of FAT_ADC and FDC_ADC can be set in one function call.
 *	
 **********************************************************************/
DRV2_API Data16 NxtSetAdcInputGain(void *pContext, 
					NxtAdcGainControl_t adcGainCtl);

/**********************************************************************
 *
 * NxtOutputControl
 *
 * Enables/tristates the output of various cores
 *
 * Inputs:
 *	void *pContext - NULL, or user-defined ID for multiple NXT200Xs
 *	NxtControl_t pgaOutput - NXT_ENABLE or NXT_TRISTATE
 *	NxtControl_t fdcAgcOutput - NXT_ENABLE or NXT_TRISTATE
 *	NxtControl_t fatAgcOutput - NXT_ENABLE or NXT_TRISTATE
 *	NxtControl_t mpegOutput - NXT_ENABLE or NXT_TRISTATE
 *	NxtControl_t podOutput - NXT_ENABLE or NXT_TRISTATE
 *
 * Returns:
 *	Data16
 *
 * Notes:
 *	The use of this API is optional. All values default to Enabled if
 *	this API is never called.
 *
 *	Use NXT_TRISTATE to tristate an output. Use NXT_ENABLE to enable an output. 
 *	NXT_DISABLE is treated same as NXT_TRISTATE.
 *	
 **********************************************************************/
DRV2_API Data16 NxtOutputControl(void *pContext, 
					NxtControl_t pgaOutput,
					NxtControl_t fdcAgcOutput,
					NxtControl_t fatAgcOutput,
					NxtControl_t mpegOutput,
					NxtControl_t podOutput);

/**********************************************************************
 *
 * NxtConfigInternalBert
 *
 * Configures the internal BERT
 *
 * Inputs:
 *	void *pContext - NULL, or user-defined ID for multiple NXT200Xs
 *	NxtBertSource_t bertSource - the source on BER data
 *	NxtBertPnSequence_t bertAlgorithm - the PN sequence algorithm
 *	NxtPolarity_t PNSequence - the PN sequence polarity
 *	NxtBertHeaderRemove_t removeBytes - number of bytes to remove in header
 *	Data8 winSize - BERT window size. max allowed size is 2^31
 *
 * Returns:
 *	Data16
 *
 * Notes:
 *	This function should be called before NxtStartBert is called.
 *	BERT is turned off following this call. Use NxtStartBert to 
 *	restart BERT.
 *	
 **********************************************************************/
DRV2_API Data16 NxtConfigInternalBert(void *pContext, 
					NxtBertSource_t bertSource, 
					NxtBertPnSequence_t bertAlgorithm, 
					NxtPolarity_t PNSequence, 
					NxtBertHeaderRemove_t removeBytes, 
					Data8 winSize);

/**********************************************************************
 *
 * NxtStartBert
 *
 * Starts BERT
 *
 * Inputs:
 *	void *pContext - NULL, or user-defined ID for multiple NXT200Xs
 *	
 * Outputs:
 *	None
 *
 * Returns:
 *	Data16
 *
 * Notes:
 *	
 *
 *
 **********************************************************************/
DRV2_API Data16 NxtStartBert(void *pContext);

/**********************************************************************
 *
 * NxtStopBert
 *
 * Stops BERT
 *
 * Inputs:
 *	void *pContext - NULL, or user-defined ID for multiple NXT200Xs
 *
 * Outputs:
 *	None
 *
 * Returns:
 *	Data16
 *
 * Notes:
 *	
 *
 *
 **********************************************************************/
DRV2_API Data16 NxtStopBert(void *pContext);


/**********************************************************************/
/*                       Interrupt-Related APIs                       */
/**********************************************************************/

/**********************************************************************
 *
 * NxtSetInterruptMask
 *
 * Determines which interrupt sources may generate a hardware interrupt
 *
 * Inputs:
 *	void *pContext - NULL, or user-defined ID for multiple NXT200Xs
 *	Data8 interruptMask - bit-mapped interrupt mask to enable interrupts
 *
 * Returns:
 *	Data16
 *
 * Notes:
 *	The default condition is that no interrupts are enabled.
 *	
 *	If multiple interrupt sources are to be enabled, the caller must OR 
 *	their values together 
 *
 **********************************************************************/
DRV2_API Data16 NxtSetInterruptMask(void *pContext,
					Data8 interruptMask);

/**********************************************************************
 *
 * NxtGetInterruptSource
 *
 * Tells which interrupt source has generated a hardware interrupt
 *
 * Inputs:
 *	void *pContext - NULL, or user-defined ID for multiple NXT200Xs
 *
 * Outputs:
 *	Data8 *pInterruptSource - bit-mapped interrupt source identifiers
 *	Data8 *pStatusResult 	- for NXT_INT_NTSC and NXT_INT_XFER, indicates
 *								the result of the operation:
 *								0 = no NTSC detect / no transfer error
 *								NXT_NTSC_DETECTED = NTSC present
 *								NXT_XFER_ERROR = transfer error occurred
 *
 * Returns:
 *	Data16
 *
 * Notes:
 *	The interrupt source register is automatically cleared when this 
 *	function is called.
 *	DO NOT call this function from interrupt-level code, or a deadlock
 *	may occur.
 *
 **********************************************************************/
DRV2_API Data16 NxtGetInterruptSource(void *pContext,
					Data8 *pInterruptSource,
					Data8 *pStatusResult);


/**********************************************************************
 *
 * NxtReadIicXferData
 *
 * Retrieves data from NxtIicXfer NXT_IIC_READ after an interrupt.
 *
 * Inputs:
 *	void *pContext - NULL, or user-defined ID for multiple NXT200Xs
 *	Data8 byteCount - number of bytes to copy
 *
 * Outputs:
 *	Data8 *pBuffer - buffer to receive byteCount bytes
 *
 * Returns:
 *	Data16
 *
 * Notes:
 *	This function will only return valid information after:
 *	  1) The previous NxtIicXfer() operation was NXT_IIC_READ,
 *	  2) NxtGetInterruptSource() has indicated a NXT_INT_XFER source, and
 *    3) NxtGetInterruptSource() has indicated no error in the transfer.
 *	These conditions are assumed -- there is no error checking for this.
 *
 **********************************************************************/
DRV2_API Data16 NxtReadIicXferData(void *pContext,
					Data8 byteCount,
					Data8 *pBuffer);

#endif

