#ifdef ALPSTUNER
/**********************************************************************
 * DRV200X.C
 * Public Interface to DRV200X
 *
 * Copyright (C) 2002 NxtWave Communications, Inc.
 *
 * $Log:   //panxtbdc01/AppsEng/PVCS/Application Eval/archives/EVAL2005/Drv2005/drv200x.c-arc  $
 * 
 *    Rev 1.51   May 20 2003 17:50:54   raggarwa
 * Fixed comments in NxtGetBertData and NxtSetAdcInputGain
 * 
 *    Rev 1.50   May 01 2003 16:29:22   raggarwa
 * Fixed the comments for NxtSetMpegMode( )
 * 
 *    Rev 1.49   Apr 28 2003 16:39:32   raggarwa
 * Changed to rev 4.0.0
 * 
 *    Rev 1.48   Apr 22 2003 13:43:52   raggarwa
 * Release 3.0.12
 * 
 *    Rev 1.47   Mar 12 2003 17:07:32   raggarwa
 * Loaded FW 4.0.1 - fix for 49 packets message
 * 
 *    Rev 1.46   Feb 28 2003 13:49:28   raggarwa
 * Changed rev number to 3.0.3;
 * moved storage of DCB settings to prior to calling regConfigSmoother( )
 * 
 *    Rev 1.45   Feb 11 2003 13:20:22   raggarwa
 * Removed nested comments, for release 3.0.0
 * 
 *    Rev 1.44   Feb 06 2003 10:59:44   raggarwa
 * Changed to rev 3.0.0
 * 
 *    Rev 1.43   Jan 21 2003 16:56:50   raggarwa
 * Updated comments for NxtSetAdcInputGain() API
 * 
 *    Rev 1.42   Jan 16 2003 17:27:52   raggarwa
 * Updated comments of NxtOutputControl()
 * 
 *    Rev 1.41   Dec 09 2002 19:12:46   raggarwa
 * Changed to rev 2.0.0
 * 
 *    Rev 1.40   Dec 09 2002 11:04:06   sreichgo
 * Changed IIC_XFER_MAX_TRIES from 10 to 30 to eliminate TUNER ERROR problems.
 * 
 *    Rev 1.39   Dec 04 2002 17:15:30   raggarwa
 * Changed rev to 1.0.2 - changed smoother registers as multibyte
 * 
 *    Rev 1.38   Nov 06 2002 11:34:56   raggarwa
 * Changed driver version to 1.0.1
 * 
 *    Rev 1.37   Oct 22 2002 15:31:02   ttsang
 * added RF threshold parsing
 * 
 *    Rev 1.36   Sep 05 2002 12:09:20   raggarwa
 * Changed rev to 1.0.0 to reflect nxtenna integration into API/driver
 * 
 *    Rev 1.35   Jul 02 2002 10:23:26   raggarwal
 * Changed rev to 0.0.11
 * 
 *    Rev 1.34   Jun 26 2002 10:57:52   raggarwal
 * Changed NxtSetAdcInputGain( ) and NxtGetBertData( ) to mask _FDC_ types for 2004
 * 
 *    Rev 1.33   Jun 25 2002 15:49:40   raggarwal
 * Updated comments for NxtGpioControl( )
 * 
 *    Rev 1.32   Jun 24 2002 16:02:58   raggarwal
 * Removed communication semaphore give-up during NTSC and IIC Xfer service completion polling, per $6.4 of Nxtenna API HLD
 * 
 *    Rev 1.31   Jun 11 2002 16:12:12   raggarwal
 * Alpha3 Release
 * 
 *    Rev 1.30   May 17 2002 16:04:52   raggarwal
 * Alpha2 Release
 * 
 *    Rev 1.16   Mar 08 2002 16:27:30   raggarwal
 * Alpha Release
 * 
 **********************************************************************/


/*
******************************************************************************
Driver Version - UPDATE THIS IF VERSION CHANGES!!!
******************************************************************************
*/
#define DRV_MAJOR_VERSION	4
#define DRV_CUST_VERSION	0
#define	DRV_MINOR_VERSION	0


/*
******************************************************************************
Includes
******************************************************************************
*/
#include "NxtCommon.h"
#include "drv2cntx.h"
#include "drv2reg.h"


/*
******************************************************************************
Defines
******************************************************************************
*/

/* Timing Offset values used for signal status */
#define FAT_TIMING_CHECKS		3
#define FAT_TIMING_CHECK_DELAY	5
#define FAT_POOR_TIMING_OFFSET  2000

/* Delay for polled service completion */
#define SERVICE_POLL_DELAY	10

/* Maximum number of polls for service completion */
#define NTSC_MAX_TRIES		10
#define IIC_XFER_MAX_TRIES	30

#define	NXT_START_CHANNEL_MASK	0xF000

/*
******************************************************************************
Private Types
******************************************************************************
*/


/*
******************************************************************************
Global variables
******************************************************************************
*/


/*
******************************************************************************
Private (static) Function Prototypes
******************************************************************************
*/

/*
******************************************************************************
Public Functions (not static)
******************************************************************************
*/

/**********************************************************************/
/*                          Initialization APIs                       */
/**********************************************************************/

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
DRV2_API Data16 NxtExitDriver(void *pContext) {

	Data16 retVal = NXT_NO_ERROR;
	NxtDCB *pDCB;
	NxtDCB *pDummy;

	/* find the referenced DCB */
	pDCB = FindDCB(pContext, &pDummy);

	if (pDCB == INVALID_CONTEXT) {	/* referenced DCB never initialized */
		retVal = NXT_ERR_INIT;
	}
	else {

		/* stop the driver */
		retVal = regStop(pDCB, NXT_STOP_FAT);
#ifndef NXT2004
		retVal = regStop(pDCB, NXT_STOP_FDC);
#endif
		
		/* return system resources */
		NxtDeleteCriticalSection(pContext, pDCB->hCSNxtComm);
		NxtDeleteCriticalSection(pContext, pDCB->hCSNxtFatTrack);
#ifndef NXT2004
		NxtDeleteCriticalSection(pContext, pDCB->hCSNxtFdcTrack);
#endif
		/* return the DCB */
		DeleteDCB(pContext);
	}
	
	return retVal;
} /* NxtExitDriver */


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
					Data16 *pResult) {

	Data16 retVal = NXT_NO_ERROR;
	NxtDCB *pDCB;
	NxtDCB *pDummy;
	Data8	coreCon;
	Data16	local = (startOptions & NXT_START_CHANNEL_MASK);
	
	*pResult = 0x00;

	/* find the referenced DCB */
	pDCB = FindDCB(pContext, &pDummy);

	if (pDCB == INVALID_CONTEXT) {	/* referenced DCB never initialized */
		retVal = NXT_ERR_INIT;
	}
	else {

		/* wait for comm semaphore */
		NxtWaitForCriticalSection(pContext, pDCB->hCSNxtComm);

		/* check for power-down */
		getRegister(pDCB, MISC_POWER_CONTROL, 1, &coreCon);
			
		/* release comm semaphore */
		NxtLeaveCriticalSection(pContext, pDCB->hCSNxtComm);

		/* check for pdwn FAT */
		if (local & NXT_CONFIG_CABLE) {
			/* NXT_CONFIG_CABLE really tests for FAT */
			if (coreCon & MISC_POWER_DN_FAT) {
				retVal = NXT_ERR_MODE;
			}
		}

#ifndef NXT2004			
		/* check for pdwn FDC */
		if (local & NXT_CONFIG_FDC) {
			if (coreCon & MISC_POWER_DN_FDC) {
				retVal = NXT_ERR_MODE;
			}
		}
#endif

		if (retVal == NXT_NO_ERROR) {
			retVal = regStart(pDCB, startOptions, pResult);
		}
	}

	return retVal;
} /* NxtStart */

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
					NxtAcqOptions_t stopOptions) {

	Data16 retVal = NXT_NO_ERROR;
	NxtDCB *pDCB;
	NxtDCB *pDummy;

	/* find the referenced DCB */
	pDCB = FindDCB(pContext, &pDummy);

	if (pDCB == INVALID_CONTEXT) {	/* referenced DCB never initialized */
		retVal = NXT_ERR_INIT;
	}
	else {
		retVal = regStop(pDCB, stopOptions);
	}

	return retVal;
} /* NxtStop */


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
 *	Bool *pbFatLocked - TRUE if frame/mpeg lock achieved in FAT channel
 *
 * Returns:
 *	Data16
 *
 *
 **********************************************************************/
DRV2_API Data16 NxtGetFatLockStatus(void *pContext,
					Bool *pbFatLocked ) {

	Data16 retVal = NXT_NO_ERROR;
	NxtDCB *pDCB;
	NxtDCB *pDummy;

	/* find the referenced DCB */
	pDCB = FindDCB(pContext, &pDummy);

	if (pDCB == INVALID_CONTEXT) {	/* referenced DCB never initialized */
		retVal = NXT_ERR_INIT;
	}
	else {

		retVal = regGetFatLockStatus(pDCB, pbFatLocked);
	}

	return retVal;
} /* NxtGetFatLockStatus */


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
 *	Data8 *pErrCount - number of Reed-Solomon errors accumulated since 
 *                     the last read.
 *
 * Returns:
 *	Data16
 *
 * Notes:
 *	The output value is only valid if the current channel is locked.
 *
 **********************************************************************/
DRV2_API Data16 NxtGetRSErrCount(void *pContext,
					Data8 *pErrCount) {

	Data16 retVal = NXT_NO_ERROR;
	NxtDCB *pDCB;
	NxtDCB *pDummy;
	Bool bLocked;

	/* find the referenced DCB */
	pDCB = FindDCB(pContext, &pDummy);

	if (pDCB == INVALID_CONTEXT) {	/* referenced DCB never initialized */
		retVal = NXT_ERR_INIT;
	}
	else {

		retVal = regGetFatLockStatus(pDCB, &bLocked);
		if (retVal == NXT_NO_ERROR) {
			if (!bLocked) {
				retVal = NXT_ERR_NO_LOCK;
				*pErrCount = 0;
			}
			else {
				retVal = regGetRSErrors(pDCB, pErrCount);
			} /* locked */
		} /* no error testing lock */
	}

	return retVal;
} /* NxtGetRSErrCount */


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
					Data8 *pBuffer ) {

	Data16 retVal = NXT_NO_ERROR;
	NxtDCB *pDCB;
	NxtDCB *pDummy;
	Data8	hwStatus;
	Data16	ramBase;
	Data8	loadControl[3]; /* MSB, LSB, control */
	Data16	blockStart;
	Data16	bytesLeft;
	Data8	blockSize;
	Data16	bufferIndex;

	/* find the referenced DCB */
	pDCB = FindDCB(pContext, &pDummy);

	if (pDCB == INVALID_CONTEXT) {	/* referenced DCB never initialized */
		retVal = NXT_ERR_INIT;
	}
	else {

		/* range check */
		if (ucOrAgc == NXT_READ_AGC_RAM) {

			/* AGC_RAM_SIZE bytes of AGC start at address 0 */
			if (startAddress + byteCount > AGC_RAM_SIZE) {
				retVal = NXT_ERR_RANGE;
			}
		}
		else {

			/* UC RAM start address depends on ROM DISABLE state */
			/* wait for comm semaphore */
			NxtWaitForCriticalSection(pContext, pDCB->hCSNxtComm);
			retVal = getRegister(pDCB, MISC_ASIC_HW_STATUS_1, 1, &hwStatus);
			/* release comm semaphore */
			NxtLeaveCriticalSection(pContext, pDCB->hCSNxtComm);

			if (retVal == NXT_NO_ERROR) {
				if (hwStatus & INT_ROM_DISABLED) {
					ramBase = RAM_BASE_ROM_DISABLED;
				}
				else {
					ramBase = RAM_BASE_ROM_ENABLED;
				}

				if ((startAddress < ramBase) || 
					(startAddress + byteCount > ramBase + UC_RAM_SIZE)) {
					retVal = NXT_ERR_RANGE;
				} else {
					if (startAddress + byteCount > DI_RAM_END + 1) {
						retVal = NXT_ERR_RANGE;
					}
				}
			}
		}
		
		/* wait for comm semaphore */
		NxtWaitForCriticalSection(pContext, pDCB->hCSNxtComm);

		if (retVal == NXT_NO_ERROR) {

			/* get current control byte - preserve reset state */
			retVal = getRegister(pDCB, UC_AGC_PGM_DNLD_CTL, 1, &loadControl[2]);
			loadControl[2] &= UC_AGC_PDC_RESET;
			loadControl[2] |= UC_AGC_PDC_NO_CRC | UC_AGC_PDC_READ_EN;
			if (ucOrAgc == NXT_READ_AGC_RAM) {
				loadControl[2] |= UC_AGC_PDC_AGC_LOAD;
			}
		}

		/* loop transferring maximum block size till done */
		blockStart = startAddress;
		bytesLeft = byteCount;
		bufferIndex = 0;

		while ((retVal == NXT_NO_ERROR) && (bytesLeft)) {

			/* how many bytes get sent this time */
			if (bytesLeft <= MAX_XFER_BLOCK_SIZE) {
				blockSize = (Data8)bytesLeft;
			}
			else {
				blockSize = MAX_XFER_BLOCK_SIZE;
			}

			/* transfer blockSize to blockStart from bufferIndex */
			/* write RAM access control */
			loadControl[0] = (Data8)((blockStart>>8) & 0xFF);
			loadControl[1] = (Data8)(blockStart & 0xFF);
			loadControl[2] |= UC_AGC_PDC_XFER_EN;

			retVal = setRegister(pDCB, UC_AGC_START_ADDR_MSB, 3, loadControl);

			if (retVal == NXT_NO_ERROR) {

				/* transfer data */
				retVal = getRegister(pDCB, 
										UC_AGC_DATA_TRANSFER, 
										blockSize, 
										&pBuffer[bufferIndex]);

				/* dummy write to end the transfer */
				setRegister(pDCB, UC_AGC_DATA_TRANSFER, 1, loadControl);

				/* disable transfer mode */
				loadControl[2] &= ~UC_AGC_PDC_XFER_EN;
				setRegister(pDCB, UC_AGC_PGM_DNLD_CTL, 1, &loadControl[2]);
			}

			/* adjust blockStart, bytesLeft, bufferIndex */
			blockStart += blockSize;
			bufferIndex += blockSize;
			bytesLeft -= blockSize;
		}

		/* release comm semaphore */
		NxtLeaveCriticalSection(pContext, pDCB->hCSNxtComm);
	}
	
	return retVal;
} /* NxtReadRAM */

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
					Data8 *pBuffer) {
	
	Data16 retVal;
	NxtDCB *pDCB;
	NxtDCB *pDummy;

	/* find the referenced DCB */
	pDCB = FindDCB(pContext, &pDummy);

	if (pDCB == INVALID_CONTEXT) {	/* referenced DCB never initialized */
		retVal = NXT_ERR_INIT;
	}
	else {
		/* wait for comm semaphore */
		NxtWaitForCriticalSection(pContext, pDCB->hCSNxtComm);

		retVal = getRegister(pDCB, deviceRegister, byteCount, pBuffer);

		/* release comm semaphore */
		NxtLeaveCriticalSection(pContext, pDCB->hCSNxtComm);
	}

	return retVal;
} /* NxtGetRegister */

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
					Data16 *pFatSQI) {

	Data16 retVal = NXT_NO_ERROR;
	NxtDCB *pDCB;
	NxtDCB *pDummy;
	Bool bLocked;
	
	/* find the referenced DCB */
	pDCB = FindDCB(pContext, &pDummy);
	
	if (pDCB == INVALID_CONTEXT) {	/* referenced DCB never initialized */
		retVal = NXT_ERR_INIT;
	}
	else {
		
		retVal = regGetFatLockStatus(pDCB, &bLocked);
		
		if (retVal == NXT_NO_ERROR) {
			if (!bLocked) {
				retVal = NXT_ERR_NO_LOCK;
				*pFatSQI = 0;
			}
			else {
				retVal = getFatSQI(pDCB, pFatSQI);
			}	/* got lock */
		}
	}

	return retVal;
} /* NxtGetFatSQI */


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
					Data8 *pFatSignalMetric) {

	Data16 retVal = NXT_NO_ERROR;
	NxtDCB *pDCB;
	NxtDCB *pDummy;
	Bool	bLocked = TRUE;
	Data16	sqi;
	Data8	gain[2];
	Data8	pdet = 0;
	Data16	power;
	double	scale;
	Data32	timingOffset;
	Data32	averageOffset = 0;
	Data8	i;
	
	/* find the referenced DCB */
	pDCB = FindDCB(pContext, &pDummy);
	
	if (pDCB == INVALID_CONTEXT)
	{	/* referenced DCB never initialized */
		retVal = NXT_ERR_INIT;
	}
	else
	{		
		retVal = regGetFatLockStatus(pDCB, &bLocked);
		
		if (retVal == NXT_NO_ERROR)
		{
			if (!bLocked)
			{
				/* not locked - check input power */
				
				/* wait for comm semaphore */
				NxtWaitForCriticalSection(pContext, pDCB->hCSNxtComm);
				
				retVal = getRegister(pDCB, AGC_SDM1_INPUT_MSB, 2, gain);
				if (NXT_NO_ERROR == retVal)
				{
					retVal = getRegister(pDCB, AGC_ADC_POWER_DETECT_MSB, 1, &pdet);
				}
				
				/* release comm semaphore */
				NxtLeaveCriticalSection(pContext, pDCB->hCSNxtComm);
				
				power = (DATA8_TO_DATA16(gain));
				power = ((~power)+4) & 0xFFFF;
				scale = ((double)pdet)/8.0 - (9.0/4.0); /* 9dB (0) to 13dB (1) */

				if (scale < 0)
					scale = 0;
				else if (scale > 1)
					scale = 1;

				if (power < FAT_POWER_THRESHOLD)
				{
					*pFatSignalState = NXT_SIG_NO_SIGNAL;
					*pFatSignalMetric = 0;
				}
				else 
				{
					/* got power - check average timing */
					*pFatSignalState = NXT_SIG_WEAK;
					for (i=0; (retVal == NXT_NO_ERROR)&&(i<FAT_TIMING_CHECKS); i++)
					{
						NxtSuspendThread(pContext, FAT_TIMING_CHECK_DELAY);
						retVal = getFatTimingOffset(pDCB, &timingOffset);
						if (timingOffset < 0)
						{
							averageOffset -= timingOffset/FAT_TIMING_CHECKS;
						}
						else
						{
							averageOffset += timingOffset/FAT_TIMING_CHECKS;
						}
					}

					if (averageOffset > FAT_POOR_TIMING_OFFSET)
					{
						*pFatSignalMetric = 0;
					}
					else if (power > FAT_POWER_LOCK)
					{
						*pFatSignalMetric = (Data8)(scale * 10);
					}
					else if (power > FAT_POWER_MID)
					{
						*pFatSignalMetric = (Data8) (scale*(8+2*(float)(power - FAT_POWER_MID)/ (FAT_POWER_LOCK - FAT_POWER_MID)));
					}
					else if (power > FAT_POWER_BAUD)
					{
						*pFatSignalMetric = (Data8) (scale*(4+4*(float)(power - FAT_POWER_BAUD)/ (FAT_POWER_MID - FAT_POWER_BAUD)));
					}
					else
					{
						*pFatSignalMetric = (Data8) (scale*(1+3*(float)(power - FAT_POWER_THRESHOLD)/ (FAT_POWER_BAUD - FAT_POWER_THRESHOLD)));
					}
				}
			}
			else
			{
				/* got lock -- determine state, metric based on sqi */
				retVal = getFatSQI(pDCB, &sqi);
				if (NXT_NO_ERROR == retVal)
				{
					if (sqi <= FAT_SQI_STRONG)
					{
						*pFatSignalState = NXT_SIG_MODERATE;
						if (sqi < FAT_SQI_MODERATE)
						{
							*pFatSignalMetric = 0;
						}
						else
						{
							*pFatSignalMetric = (Data8)(10*(float)(sqi - FAT_SQI_MODERATE)/	(FAT_SQI_STRONG - FAT_SQI_MODERATE));
						}
					}
					else if (sqi <= FAT_SQI_VERY_STRONG)
					{
						*pFatSignalState = NXT_SIG_STRONG;
						*pFatSignalMetric =  (Data8)(10*(float)(sqi - FAT_SQI_STRONG)/ (FAT_SQI_VERY_STRONG - FAT_SQI_STRONG));
					}
					else
					{
						*pFatSignalState = NXT_SIG_VERY_STRONG;
						*pFatSignalMetric =  (Data8)(10*(float)(sqi - FAT_SQI_VERY_STRONG)/ (FAT_SQI_MAX - FAT_SQI_VERY_STRONG));
					}
				}
			}	/* locked - status based on sqi */
		}
	}

	return retVal;
} /* NxtGetFatSignalStatus */



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
					Data32 *pFatFreqOffset) {
	Data16 retVal;
	NxtDCB *pDCB;
	NxtDCB *pDummy;
	Bool	bLocked;
	Data32	carrierOffset = 0;
	Data32	pilotOffset = 0;

	/* find the referenced DCB */
	pDCB = FindDCB(pContext, &pDummy);

	if (pDCB == INVALID_CONTEXT) {	/* referenced DCB never initialized */
		retVal = NXT_ERR_INIT;
	}
	else {
		/* offset only valid if locked */

		retVal = regGetFatLockStatus(pDCB, &bLocked);

		if ((retVal == NXT_NO_ERROR)&&(!bLocked)) {
			retVal = NXT_ERR_NO_LOCK;
		}

		if (retVal == NXT_NO_ERROR) {
			retVal = getFatPilotOffset(pDCB, &pilotOffset);
		}

		if (retVal == NXT_NO_ERROR) {
			retVal = getFatCarrierOffset(pDCB, &carrierOffset);
		}

		*pFatFreqOffset = pilotOffset + carrierOffset;
	}

	return retVal;
} /* NxtGetFatFreqOffset */


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
					NxtModFormat_t *pFatModFormat) {

	Data16 retVal = NXT_NO_ERROR;
	NxtDCB *pDCB;
	NxtDCB *pDummy;

	/* find the referenced DCB */
	pDCB = FindDCB(pContext, &pDummy);

	if (pDCB == INVALID_CONTEXT) {	/* referenced DCB never initialized */
		retVal = NXT_ERR_INIT;
	}
	else {

		retVal = getFatModFormat(pDCB, pFatModFormat);
	}

	return retVal;
} /* NxtGetFatModFormat */


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
					Bool bWait) {

	Data16 retVal = NXT_NO_ERROR;
	NxtDCB *pDCB;
	NxtDCB *pDummy;
	NxtModFormat_t modFormat;
	Bool	bLock = FALSE;
	Data8   buffer;

	/* find the referenced DCB */
	pDCB = FindDCB(pContext, &pDummy);

	if (pDCB == INVALID_CONTEXT) {	/* referenced DCB never initialized */
		retVal = NXT_ERR_INIT;
	}
	else {

		/* get current mod format */
		retVal = getFatModFormat(pDCB, &modFormat);

		/* wait for comm semaphore */
		NxtWaitForCriticalSection(pContext, pDCB->hCSNxtComm);
#ifndef NXT2004
		if (NXT_BERT_INPUT_FDC == pDCB->bertFormat.bertSource) {
			/* BERT source is FDC */
			retVal |= regGetBertData(pDCB, pErrorCount, bWait);
		} else 
#endif		
		{

			/* BERT source is not FDC */
			if (NXT_NO_ERROR == retVal) {

				if (NXT_8VSB == modFormat || NXT_16VSB == modFormat) {

					retVal = getRegister(pDCB, EQ_DFS_STATE, 1, &buffer);

					if (NXT_NO_ERROR == retVal) {
						if (!(buffer & EQ_DFS_ACQ)) {
							/* BERT stream is locked */
							retVal = regGetBertData(pDCB, pErrorCount, bWait);
						} else {
							retVal = NXT_ERR_NO_LOCK;
						}
					}

				}/* if VSB==mod */
				else if (NXT_64QAM == modFormat || NXT_256QAM == modFormat) {
					/* mod format is QAM, check the input type */
					switch (pDCB->bertFormat.bertSource) {
						case NXT_BERT_INPUT_TRELLIS:
							retVal = getRegister(pDCB, FEC_TD_MODE_CTRL, 1, &buffer);
							if (NXT_NO_ERROR == retVal && (buffer & FEC_TD_LOCK))
								bLock = TRUE;
							break;
						case NXT_BERT_INPUT_DI:
						case NXT_BERT_INPUT_FAT:
							retVal = getRegister(pDCB, FEC_QAMFSD_STATUS, 1, &buffer);
							if (NXT_NO_ERROR == retVal) {
								buffer &= FEC_QAM_DET_STATE;
								if ((FEC_QAM_LOCK == buffer) || 
									(FEC_QAM_VERIFY == buffer))
									bLock = TRUE;
							}
							break;
					}/* switch */

					if(bLock) {
						retVal = regGetBertData(pDCB, pErrorCount, bWait);
					}/* if (bLock) */
					else {
						retVal = NXT_ERR_NO_LOCK;
					}

				}/* else if */
			}/* if */

		}/* else */

		/* release comm semaphore */
		NxtLeaveCriticalSection(pContext, pDCB->hCSNxtComm);

	}
	return retVal;
}/* NxtGetBertData */


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
DRV2_API Data16 NxtGetDriverVersion(CodeVersion_s *pCodeVersion) {

	pCodeVersion->major = DRV_MAJOR_VERSION;
	pCodeVersion->custom = DRV_CUST_VERSION;
	pCodeVersion->minor = DRV_MINOR_VERSION;

	return NXT_NO_ERROR;
} /* NxtGetDriverVersion */


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
					AsicVersion_s *pAsicVersion) {

	Data16 retVal = NXT_NO_ERROR;
	NxtDCB *pDCB;
	NxtDCB *pDummy;
	Data8	buffer[DEV_ID_LENGTH];

	/* find the referenced DCB */
	pDCB = FindDCB(pContext, &pDummy);

	if (pDCB == INVALID_CONTEXT) {	/* referenced DCB never initialized */
		retVal = NXT_ERR_INIT;
	}
	else {
				
		/* wait for comm semaphore */
		NxtWaitForCriticalSection(pContext, pDCB->hCSNxtComm);
		
		retVal = getRegister(pDCB, MISC_DEV_ID, DEV_ID_LENGTH, buffer);
		
		/* release comm semaphore */
		NxtLeaveCriticalSection(pContext, pDCB->hCSNxtComm);

		pAsicVersion->device = buffer[DEV_ID_DEVICE];
		pAsicVersion->fab = buffer[DEV_ID_FAB];
		pAsicVersion->month = buffer[DEV_ID_MONTH];
		pAsicVersion->year[0] = buffer[DEV_ID_YEAR_MSB];
		pAsicVersion->year[1] = buffer[DEV_ID_YEAR_LSB];
	}

	return retVal;
} /* NxtGetAsicVersion */


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
					CodeVersion_s *pCodeVersion) {

	Data16 retVal = NXT_NO_ERROR;
	NxtDCB *pDCB;
	NxtDCB *pDummy;
	Data8	hwStatus;
	Data8	buffer[CODE_VERSION_LENGTH];

	/* set default output values */
	pCodeVersion->major = 0;
	pCodeVersion->custom = 0;
	pCodeVersion->minor = 0;

	/* find the referenced DCB */
	pDCB = FindDCB(pContext, &pDummy);

	if (pDCB == INVALID_CONTEXT) {	/* referenced DCB never initialized */
		retVal = NXT_ERR_INIT;
	}
	else {
				
		/* wait for comm semaphore */
		NxtWaitForCriticalSection(pContext, pDCB->hCSNxtComm);

		/* check for ROM DISABLED */
		retVal = getRegister(pDCB, MISC_ASIC_HW_STATUS_1, 1, &hwStatus);
		if (NXT_NO_ERROR == retVal) {
			if (hwStatus & INT_ROM_DISABLED) {
				retVal = NXT_ERR_MODE;
			}
			else {
				retVal = getRegister(pDCB, MISC_FIRMWARE_CONTROL, 1, &hwStatus);
				if (NXT_NO_ERROR == retVal) {
					if (hwStatus & MISC_FW_ENABLE_RAM_DNLD) {

						/* RAM is enabled, return the hard-coded value */
						pCodeVersion->major = 0;
						pCodeVersion->custom = 0;
						pCodeVersion->minor = 1;
					} else {

						/* RAM is disabled */
						retVal = getRegister(pDCB, MISC_ROM_MASK_VERSION, 
											CODE_VERSION_LENGTH, buffer);

						pCodeVersion->major = buffer[CODE_VERSION_MAJOR];
						pCodeVersion->custom = buffer[CODE_VERSION_CUSTOM];
						pCodeVersion->minor = buffer[CODE_VERSION_MINOR];
					}
				}
			}
		}
		/* release comm semaphore */
		NxtLeaveCriticalSection(pContext, pDCB->hCSNxtComm);
	}

	return retVal;
} /* NxtGetRomVersion */

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
					CodeVersion_s *pCodeVersion) {

	Data16	retVal = NXT_NO_ERROR;
	NxtDCB *pDCB;
	NxtDCB *pDummy;
	Data8	hwStatus;
	Data8	buffer[CODE_VERSION_LENGTH];

	/* set default output values */
	pCodeVersion->major = 0;
	pCodeVersion->custom = 0;
	pCodeVersion->minor = 0;

	/* find the referenced DCB */
	pDCB = FindDCB(pContext, &pDummy);

	if (pDCB == INVALID_CONTEXT) {	/* referenced DCB never initialized */
		retVal = NXT_ERR_INIT;
	}
	else {
				
		/* wait for comm semaphore */
		NxtWaitForCriticalSection(pContext, pDCB->hCSNxtComm);


		/* check for RAM enabled */
		retVal = getRegister(pDCB, MISC_FIRMWARE_CONTROL, 1, &hwStatus);
		if (NXT_NO_ERROR == retVal) {
			if (hwStatus & MISC_FW_ENABLE_RAM_DNLD) {
		/* check for ROM DISABLED */
/*		retVal = getRegister(pDCB, MISC_ASIC_HW_STATUS_1, 1, &hwStatus);
		if (NXT_NO_ERROR == retVal) {
			if (hwStatus & INT_ROM_DISABLED) { */
				/* check for micro in reset */
				retVal = getRegister(pDCB, UC_AGC_PGM_DNLD_CTL, 1, &hwStatus);
				if ((NXT_NO_ERROR == retVal) && (hwStatus & UC_AGC_PDC_RESET)) {
					retVal = NXT_ERR_RESET;
				}		
				else if (NXT_NO_ERROR == retVal) {
					/* read RAM version */
					retVal = getRegister(pDCB, MISC_ROM_MASK_VERSION, 
						CODE_VERSION_LENGTH, buffer);
					
					pCodeVersion->major = buffer[CODE_VERSION_MAJOR];
					pCodeVersion->custom = buffer[CODE_VERSION_CUSTOM];
					pCodeVersion->minor = buffer[CODE_VERSION_MINOR];
				}
			}
			else {
				retVal = NXT_ERR_MODE;
			}
		}

		/* release comm semaphore */
		NxtLeaveCriticalSection(pContext, pDCB->hCSNxtComm);
	}	/* good context */

	return retVal;
} /* NxtGetRamVersion */


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
 *	Bool	*pbNtsc - TRUE if NTSC is present, FALSE otherwise
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
					Bool *pbNtsc) {

	Data16	retVal = NXT_NO_ERROR;
	NxtDCB *pDCB;
	NxtDCB *pDummy;
	Data8	opMode;
	Data8	tries = 0;

	/* find the referenced DCB */
	pDCB = FindDCB(pContext, &pDummy);

	if (pDCB == INVALID_CONTEXT) {	/* referenced DCB never initialized */
		retVal = NXT_ERR_INIT;
	}
	else {
		/* stop in-band acquisition */
		retVal = regStop(pDCB, NXT_STOP_FAT);

		/* assume no NTSC present */
		*pbNtsc = FALSE;

		/* initiate NTSC detection */
		if (NXT_NO_ERROR == retVal) {

			/* wait for comm semaphore */
			NxtWaitForCriticalSection(pContext, pDCB->hCSNxtComm);

			retVal = getRegister(pDCB, UC_SERVICES, 1, &opMode);
			opMode |= UC_SERVICES_NTSC_DET_SVC;
			retVal |= setRegister(pDCB, UC_SERVICES, 1, &opMode);

			/* if interrupt is enabled just return */
			if (!(pDCB->irqMask & NXT_INT_NTSC)) {

				/* wait for detection to complete */
				while (	(tries < NTSC_MAX_TRIES) && 
						(NXT_NO_ERROR == retVal) && 
						(opMode & UC_SERVICES_NTSC_DET_SVC)) {

					NxtSuspendThread(pContext, SERVICE_POLL_DELAY);

					retVal = getRegister(pDCB, UC_SERVICES, 1, &opMode);
	
					tries++;
				}

				/* check for error conditions */
				if (tries >= NTSC_MAX_TRIES) {
					retVal = NXT_ERR_TIMEOUT;
				}
				else if (NXT_NO_ERROR == retVal) {
					*pbNtsc = ((opMode & UC_SERVICES_NTSC_STATUS) 
								== UC_SERVICES_NTSC_STATUS);
				}
			}
			/* release comm semaphore */
			NxtLeaveCriticalSection(pContext, pDCB->hCSNxtComm);
		}
	}

	return retVal;
} /* NxtGetNtscStatus */



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
					NxtBypass_t bypass) {

	Data16 retVal = NXT_NO_ERROR;
	NxtDCB *pDCB;
	NxtDCB *pDummy;
	Data8	buffer;

	/* find the referenced DCB */
	pDCB = FindDCB(pContext, &pDummy);

	if (pDCB == INVALID_CONTEXT) {	/* referenced DCB never initialized */
		retVal = NXT_ERR_INIT;
	}
	else {
		/* wait for comm semaphore */
		NxtWaitForCriticalSection(pContext, pDCB->hCSNxtComm);
		
		/* Read MISC_ASIC_HW_STATUS_1 */
		retVal = getRegister(pDCB, MISC_ASIC_HW_STATUS_1, 1, &buffer);

		/* Modify MISC_ASIC_HW_STATUS_1 to effect NxtIicBypass */
		if (bypass == NXT_IIC_BYPASS) {
			buffer |= MISC_IIC_BYPASS;
		}
		else if (bypass == NXT_IIC_UC_CONTROL) {
			buffer &= ~MISC_IIC_BYPASS;
		}
		else {
			retVal = NXT_ERR_RANGE;
		}

		if (retVal == NXT_NO_ERROR) {
			/* Write MISC_ASIC_HW_STATUS_1 */
			retVal = setRegister(pDCB, MISC_ASIC_HW_STATUS_1, 1, &buffer);
		}

		/* release comm semaphore */
		NxtLeaveCriticalSection(pContext, pDCB->hCSNxtComm);
	}

	return retVal;
} /* NxtIicBypass */

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
DRV2_API Data16 NxtIicXfer( void *pContext,
					NxtIicXferMode_t mode,
					Data8 speed,
					Data8 byteCount,	
					Data8 deviceAddress,
					Data8 *pBuffer) {
	
	Data16	retVal = NXT_NO_ERROR;
	NxtDCB *pDCB;
	NxtDCB *pDummy;
	Data8	tries = 0;
	Data8	opMode;
	
	/* find the referenced DCB */
	pDCB = FindDCB(pContext, &pDummy);
	
	if (pDCB == INVALID_CONTEXT) {	/* referenced DCB never initialized */
		retVal = NXT_ERR_INIT;
	}
	else {
			
		/* wait for comm semaphore */
		NxtWaitForCriticalSection(pContext, pDCB->hCSNxtComm);

		/* test for max byte count and legal mode and speed */
		if ((byteCount > (IIC_XFER_DATA_MAX-IIC_XFER_DATA_0+1)) ||
			((mode != NXT_IIC_READ) && (mode != NXT_IIC_WRITE)) ||
			(speed & 0x80)) {
			retVal = NXT_ERR_RANGE;
		}
		
		/* initiate IIC transfer */
		if (NXT_NO_ERROR == retVal) {

			/* mask speed just to be sure */
			speed &= UC_CTRL_SPEED_MASK;
			retVal = setRegister(	pDCB, 
									UC_CONTROL, 
									1, 
									&speed);

			/* set UC_GP_4 for IIC xfer(0x00) and byteCount */
			byteCount = byteCount & UC_XFER_SIZE_MASK;
			retVal |= setRegister(	pDCB, 
									UC_GP_4, 
									1, 
									&byteCount);
			
			if (NXT_IIC_READ == mode) {
				deviceAddress |= 0x01;
			}
			else {
				deviceAddress &= 0xFE;
				retVal |= setRegister(	pDCB, 
										IIC_XFER_DATA_0, 
										byteCount, 
										pBuffer);
			}
			retVal |= setRegister(	pDCB, 
									IIC_XFER_ADDR, 
									1, 
									&deviceAddress);
			
			
			if (NXT_NO_ERROR == retVal) {
				
				retVal |= getRegister(pDCB, UC_SERVICES, 1, &opMode);
				opMode |= UC_SERVICES_DATA_XFER;
				retVal |= setRegister(pDCB, UC_SERVICES, 1, &opMode);

				/* if interrupt is enabled just return */
				if (!(pDCB->irqMask & NXT_INT_XFER)) {

					/* wait for transfer to complete */
					while (	(tries < IIC_XFER_MAX_TRIES) && 
						(NXT_NO_ERROR == retVal) && 
						(opMode & UC_SERVICES_DATA_XFER)) {

						NxtSuspendThread(pContext, SERVICE_POLL_DELAY);
						
						retVal = getRegister(pDCB, UC_SERVICES, 1, &opMode);
						
						tries++;
					}
					
					/* check for error conditions */
					if (tries >= IIC_XFER_MAX_TRIES) {
						retVal = NXT_ERR_TIMEOUT;
					}
					else if (NXT_NO_ERROR == retVal) {
						if (opMode & UC_SERVICES_DATA_XFER_ERR){
							retVal = NXT_ERR_IIC_XFER;
						}
						else if (NXT_IIC_READ == mode) {
							/* get the data */
							retVal |= getRegister(	pDCB, 
													IIC_XFER_DATA_0, 
													byteCount, 
													pBuffer);
						} /* NXT_IIC_READ */
					} /* NXT_NO_ERROR */
				} /* NXT_INT_XFER */
			} /* NXT_NO_ERROR */
		} /* NXT_NO_ERROR */
				
		/* release comm semaphore */
		NxtLeaveCriticalSection(pContext, pDCB->hCSNxtComm);
	}
	
	return retVal;
} /* NxtIicXfer */


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
					NxtPolarity_t clockPolarity) {

	Data16 retVal = NXT_NO_ERROR;
	NxtDCB *pDCB;
	NxtDCB *pDummy;
	Data8	buffer;

	/* find the referenced DCB */
	pDCB = FindDCB(pContext, &pDummy);

	if (pDCB == INVALID_CONTEXT) {	/* referenced DCB never initialized */
		retVal = NXT_ERR_INIT;
	}
	else {

		/* range check all inputs */
		if ((dataEnablePolarity != NON_INVERTED && 
			 dataEnablePolarity != INVERTED) ||
			(pktSyncPolarity != NON_INVERTED && 
			 pktSyncPolarity != INVERTED) ||
			(errorPolarity != NON_INVERTED && 
			 errorPolarity != INVERTED) ||
			(clockPolarity != NON_INVERTED && 
			 clockPolarity != INVERTED)) {

			retVal = NXT_ERR_RANGE;
		}
		else {

			/* wait for comm semaphore */
			NxtWaitForCriticalSection(pContext, pDCB->hCSNxtComm);

			/* read MISC_MOD_CTRL_OUT_FMT */
			retVal = getRegister(pDCB, MISC_MOD_CTRL_OUT_FMT, 1, &buffer);

			/* modify all polarity bits */
			buffer &= ~OUT_FMT_POL_MASK;
			if (dataEnablePolarity == INVERTED) {
				buffer |= OUT_FMT_POL_DAT_EN;
			}
			if (pktSyncPolarity == INVERTED) {
				buffer |= OUT_FMT_POL_PKT_SY;
			}
			if (errorPolarity == INVERTED) {
				buffer |= OUT_FMT_POL_ERROR;
			}
			if (clockPolarity == NON_INVERTED) {
				buffer |= OUT_FMT_POL_CLOCK;
			}

			if (retVal == NXT_NO_ERROR) {
				/* write MISC_MOD_CTRL_OUT_FMT */
				retVal = setRegister(pDCB, MISC_MOD_CTRL_OUT_FMT, 1, &buffer);
			}

			/* release comm semaphore */
			NxtLeaveCriticalSection(pContext, pDCB->hCSNxtComm);

			/* save current polarity settings */
			pDCB->mpegFormat.dataEnablePolarity = dataEnablePolarity;
			pDCB->mpegFormat.pktSyncPolarity = pktSyncPolarity;
			pDCB->mpegFormat.errorPolarity = errorPolarity;
			pDCB->mpegFormat.clockPolarity = clockPolarity;
		}
	}

	return retVal;
} /* NxtSetMpegPolarity */


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
					Bool bHeaderEnable) {

	Data16	retVal = NXT_NO_ERROR;
	NxtDCB *pDCB;
	NxtDCB *pDummy;
	Data8	format, mpegMode0, mpegMode1, mpegOutFmt;
	NxtModFormat_t modFormat;
	/* find the referenced DCB */
	pDCB = FindDCB(pContext, &pDummy);

	if (pDCB == INVALID_CONTEXT) {	/* referenced DCB never initialized */
		retVal = NXT_ERR_INIT;
	}
	else {

		/* range check all inputs */
		if ((bGatedOutputEnable != TRUE && 
			 bGatedOutputEnable != FALSE) ||
			(bContinuousRateEnable != TRUE &&
			 bContinuousRateEnable != FALSE) ||
			(bParallelOutputEnable != TRUE && 
			 bParallelOutputEnable != FALSE) ||
			(bHeaderEnable != TRUE && 
			 bHeaderEnable != FALSE)) {

			retVal = NXT_ERR_RANGE;
		}
		else {

			/* wait for comm semaphore */
			NxtWaitForCriticalSection(pContext, pDCB->hCSNxtComm);

			/* read MISC_MOD_CTRL_OUT_FMT */
			retVal = getRegister(pDCB, MISC_MOD_CTRL_OUT_FMT, 1, &format);

			if (NXT_NO_ERROR == retVal) 
			{
				/* modify output format bits */
				format &= ~OUT_FMT_DATA_MASK;

				if (bGatedOutputEnable) {
					format |= OUT_FMT_DATA_GATED;
				}
				if (!bParallelOutputEnable) {
					format |= OUT_FMT_DATA_SERIAL;
				}
				/* write MISC_MOD_CTRL_OUT_FMT */
				retVal = setRegister(pDCB, MISC_MOD_CTRL_OUT_FMT, 1, &format);
			}
			if (NXT_NO_ERROR == retVal) {
				if (bHeaderEnable) {
					/* normal operation */
					mpegMode0 = FEC_MPEG_MODE_0_STD;
					mpegMode1 = FEC_MPEG_MODE_1_STD;
				}
				else {
					/* bit-error-rate test mode */
					mpegMode0 = FEC_MPEG_MODE_0_BER;
					mpegMode1 = FEC_MPEG_MODE_1_BER;
				}
				retVal = setRegister(pDCB, FEC_MPEG_MODE_0, 1, &mpegMode0);
				retVal |= setRegister(pDCB, FEC_MPEG_MODE_1, 1, &mpegMode1);
			}

			if (NXT_NO_ERROR == retVal) {
				retVal = getRegister(pDCB, SMOOTHER_CONTROL, 1, &mpegOutFmt);
				if (NXT_NO_ERROR == retVal) {
					if (bContinuousRateEnable) {
						/* enable feedback control loop */
						mpegOutFmt |= SMOOTHER_CTL_ENABLE_LOOP;
					} else {
						mpegOutFmt &= ~SMOOTHER_CTL_ENABLE_LOOP;
					}
					retVal = setRegister(pDCB, SMOOTHER_CONTROL, 1, &mpegOutFmt);
				}
			}

			/* release comm semaphore */
			NxtLeaveCriticalSection(pContext, pDCB->hCSNxtComm);

			/* save current format settings  regardless of success or  
			** failure, so that a soft reset will put the chip in the
			** proper mode. */
			pDCB->mpegFormat.bParallelOutputEnable = bParallelOutputEnable;
			pDCB->mpegFormat.bContinuousRateEnable = bContinuousRateEnable;
			pDCB->mpegFormat.bGatedOutputEnable    = bGatedOutputEnable;
			pDCB->mpegFormat.bHeaderEnable         = bHeaderEnable;

			/* re-initialize smoother core */
			retVal |= getFatModFormat(pDCB, &modFormat);
			if (NXT_NO_ERROR == retVal) {
				retVal |= regConfigSmoother(pDCB, modFormat);
			}

		}
	}

	return retVal;
} /* NxtSetMpegMode */



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
					Data16 *pFatAgcData) {
	Data16 retVal = NXT_NO_ERROR;
	NxtDCB *pDCB;
	NxtDCB *pDummy;
	Data8	i;

	/* find the referenced DCB */
	pDCB = FindDCB(pContext, &pDummy);

	if (pDCB == INVALID_CONTEXT) {	/* referenced DCB never initialized */
		retVal = NXT_ERR_INIT;
	}
	else {
		/* set the AGC data pointer */
		pDCB->pFatAgcData = pFatAgcData;
		pDCB->bFatStartScriptDone = FALSE;

		/* initialize the pointers to NULL */
		pDCB->pAgc256Qam = NULL;
		pDCB->pFatAgcAdj = NULL;
		pDCB->pAgc64Qam = NULL;
		/* default the threshold values */
		pDCB->fatIfAdjOn = IF_INPUT_ADJACENT_ON;
		pDCB->fatIfAdjOff = IF_INPUT_ADJACENT_OFF;

		pDCB->fatRfAdjOn = RF_INPUT_ADJACENT_ON;
		pDCB->fatRfAdjOff = RF_INPUT_ADJACENT_OFF;

		/* search for AGC modifications */
		if (pFatAgcData != (Data16 *)NULL) {
			for (i=0; pDCB->pFatAgcData[i] != NXT_AGC_SETUP_DONE; i++) {
				if (pDCB->pFatAgcData[i] == NXT_AGC_SETUP_256QAM) {
					if ((pDCB->pFatAgcData[i+1] != NXT_AGC_SETUP_DONE) &&
						(pDCB->pFatAgcData[i+1] != NXT_AGC_SETUP_64QAM) &&
						(pDCB->pFatAgcData[i+1] != NXT_FAT_AGC_SETUP_ADJ) &&
						(pDCB->pFatAgcData[i+1] != NXT_AGC_IF_THRESHOLD) &&
						(pDCB->pFatAgcData[i+1] != NXT_AGC_RF_THRESHOLD)) {

						pDCB->pAgc256Qam = pFatAgcData + i + 1;
					}
				} else if (pDCB->pFatAgcData[i] == NXT_FAT_AGC_SETUP_ADJ) {
					if ((pDCB->pFatAgcData[i+1] != NXT_AGC_SETUP_DONE) &&
						(pDCB->pFatAgcData[i+1] != NXT_AGC_SETUP_64QAM) &&
						(pDCB->pFatAgcData[i+1] != NXT_AGC_SETUP_256QAM) &&
						(pDCB->pFatAgcData[i+1] != NXT_AGC_IF_THRESHOLD) &&
						(pDCB->pFatAgcData[i+1] != NXT_AGC_RF_THRESHOLD)) {

						pDCB->pFatAgcAdj = pFatAgcData + i + 1;
					}
				} else if (pDCB->pFatAgcData[i] == NXT_AGC_SETUP_64QAM) {
					if ((pDCB->pFatAgcData[i+1] != NXT_AGC_SETUP_DONE) &&
						(pDCB->pFatAgcData[i+1] != NXT_FAT_AGC_SETUP_ADJ) &&
						(pDCB->pFatAgcData[i+1] != NXT_AGC_SETUP_256QAM) &&
						(pDCB->pFatAgcData[i+1] != NXT_AGC_IF_THRESHOLD) &&
						(pDCB->pFatAgcData[i+1] != NXT_AGC_RF_THRESHOLD)) {

						pDCB->pAgc64Qam = pFatAgcData + i + 1;
					}
				} else if (pDCB->pFatAgcData[i] == NXT_AGC_IF_THRESHOLD) {
					if ((pDCB->pFatAgcData[i+1] != NXT_AGC_SETUP_DONE) &&
						(pDCB->pFatAgcData[i+1] != NXT_FAT_AGC_SETUP_ADJ) && 
						(pDCB->pFatAgcData[i+1] != NXT_AGC_SETUP_256QAM) &&
						(pDCB->pFatAgcData[i+1] != NXT_AGC_SETUP_64QAM) &&
						(pDCB->pFatAgcData[i+1] != NXT_AGC_RF_THRESHOLD)) {
						/* save in DCB */
						pDCB->fatIfAdjOn	= DATA8_TO_DATA16(pFatAgcData + i + 1);
						pDCB->fatIfAdjOff	= DATA8_TO_DATA16(pFatAgcData + i + 3);
						/* assume there will always be 4B of info, if at all */
						i += 4;
					}
				} else if (pDCB->pFatAgcData[i] == NXT_AGC_RF_THRESHOLD) {
					if ((pDCB->pFatAgcData[i+1] != NXT_AGC_SETUP_DONE) &&
						(pDCB->pFatAgcData[i+1] != NXT_FAT_AGC_SETUP_ADJ) &&
						(pDCB->pFatAgcData[i+1] != NXT_AGC_SETUP_256QAM) &&
						(pDCB->pFatAgcData[i+1] != NXT_AGC_SETUP_64QAM) &&
						(pDCB->pFatAgcData[i+1] != NXT_AGC_IF_THRESHOLD)) {
						/* save in DCB */
						pDCB->fatRfAdjOn	= DATA8_TO_DATA16(pFatAgcData + i + 1);
						pDCB->fatRfAdjOff	= DATA8_TO_DATA16(pFatAgcData + i + 3);
						/* assume there will always be 4B of info, if at all */
						i += 4;
					}
				}

			}
		}
	}
	return retVal;
}/* NxtSetFatAgcData */

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
					NxtPolarity_t sdmAPolarity) {

	Data16 retVal = NXT_NO_ERROR;
	NxtDCB *pDCB;
	NxtDCB *pDummy;
	Data8	buffer;

	/* find the referenced DCB */
	pDCB = FindDCB(pContext, &pDummy);

	if (pDCB == INVALID_CONTEXT) {	/* referenced DCB never initialized */
		retVal = NXT_ERR_INIT;
	}
	else {

		/* range check all inputs */
		if ((sdm1Polarity != NON_INVERTED &&
			 sdm1Polarity != INVERTED) ||
			(sdm2Polarity != NON_INVERTED &&
			 sdm2Polarity != INVERTED) ||
			(sdmXPolarity != NON_INVERTED &&
			 sdmXPolarity != INVERTED) ||
			(sdmAPolarity != NON_INVERTED &&
			 sdmAPolarity != INVERTED)) {

			retVal = NXT_ERR_RANGE;
		}
		else {

			/* wait for comm semaphore */
			NxtWaitForCriticalSection(pContext, pDCB->hCSNxtComm);

			/* read AGC_SDM_CONFIGURE */
			retVal = getRegister(pDCB, AGC_SDM_CONFIGURE, 1, &buffer);

			/* modify all polarity bits */
			buffer &= ~AGC_SDM_POL_MASK;
			if (sdm1Polarity == INVERTED) {
				buffer |= AGC_SDM1_INVERT;
			}
			if (sdm2Polarity == INVERTED) {
				buffer |= AGC_SDM2_INVERT;
			}
			if (sdmXPolarity == INVERTED) {
				buffer |= AGC_SDMX_INVERT;
			}
			if (sdmAPolarity == INVERTED) {
				buffer |= AGC_SDMA_INVERT;
			}

			if (retVal == NXT_NO_ERROR) {
				/* write AGC_SDM_CONFIGURE */
				retVal = setRegister(pDCB, AGC_SDM_CONFIGURE, 1, &buffer);
			}

			/* release comm semaphore */
			NxtLeaveCriticalSection(pContext, pDCB->hCSNxtComm);

			/* save current polarity settings */
			pDCB->fatAgcPolarity.sdm1_pol = sdm1Polarity;
			pDCB->fatAgcPolarity.sdm2_pol = sdm2Polarity;
			pDCB->fatAgcPolarity.sdmX_pol = sdmXPolarity;
			pDCB->fatAgcPolarity.sdmA_pol = sdmAPolarity;
		}
	}

	return retVal;
} /* NxtSetFatAgcSdmPolarity */


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
					Data8 *pBuffer) {

	Data16 retVal = NXT_NO_ERROR;
	NxtDCB *pDCB;
	NxtDCB *pDummy;

	/* find the referenced DCB */
	pDCB = FindDCB(pContext, &pDummy);

	if (pDCB == INVALID_CONTEXT) {	/* referenced DCB never initialized */
		retVal = NXT_ERR_INIT;
	}
	else {
		retVal = regLoadRAM(pDCB, loadType, startAddress, byteCount, pBuffer);

	} 
	
	return retVal;
} /* NxtLoadRAM */


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
					Data8 byteCount,
					Data8 *pBuffer) {
	
	Data16 retVal;
	NxtDCB *pDCB;
	NxtDCB *pDummy;

	/* find the referenced DCB */
	pDCB = FindDCB(pContext, &pDummy);

	if (pDCB == INVALID_CONTEXT) {	/* referenced DCB never initialized */
		retVal = NXT_ERR_INIT;
	}
	else {
		/* wait for comm semaphore */
		NxtWaitForCriticalSection(pContext, pDCB->hCSNxtComm);

		retVal = setRegister(pDCB, deviceRegister, byteCount, pBuffer);

		/* release comm semaphore */
		NxtLeaveCriticalSection(pContext, pDCB->hCSNxtComm);
	}

	return retVal;
} /* NxtSetRegister */

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
 *	Data16 *pData - for NXT_GPIO_ASSIGN, contains the assignment mask
 *	             - for NXT_GPIO_SET_IO, contains the Input/Output mask
 *	             - for NXT_GPIO_WRITE, contains data to write
 *
 * Outputs:
 *	Data16 *pData - for NXT_GPIO_READ, contains read data
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
DRV2_API Data16 NxtGpioControl(void *pContext,
					NxtGpioMode_t mode,
					Data16 *pData) {
	Data16	retVal = NXT_NO_ERROR;
	NxtDCB *pDCB;
	NxtDCB *pDummy;
	Data16	localData;
	Data8	regData;
	Data8	coreCon;
	/* add masks for read, write, etc. */
	/* find the referenced DCB */
	pDCB = FindDCB(pContext, &pDummy);
	
	if (pDCB == INVALID_CONTEXT) {	/* referenced DCB never initialized */
		retVal = NXT_ERR_INIT;
	}
	else {

		/* GPIO_MASK depends on the chip */
#ifdef NXT2003
		localData = *pData & ~NXT2003_GPIO_MASK;
#endif

#ifdef NXT2004
		localData = *pData & ~NXT2004_GPIO_MASK;
#endif

#ifdef NXT2005
		localData = *pData & ~NXT2005_GPIO_MASK;
#endif
		/* wait for comm semaphore */
		NxtWaitForCriticalSection(pContext, pDCB->hCSNxtComm);
		
		switch (mode) {
		case NXT_GPIO_ASSIGN:
			/* only lower eight pins are assignable */
			localData &= ~NXT_GPIO_ASSIGN_MASK;
			regData = (Data8) localData;
			retVal = setRegister(pDCB, MISC_GPIO_ACCESS_SELECT, 1, &regData);
			break;
		case NXT_GPIO_SET_IO:
			/* upper byte */
			regData = (Data8) (localData >> 8);
			retVal = setRegister(pDCB, MISC_TEST_IO_OEN, 1, &regData);
			/* lower byte */
			regData = (Data8) (localData & 0xFF);
			retVal |= setRegister(pDCB, MISC_GPIO_OEN, 1, &regData);
			break;
		case NXT_GPIO_READ:
			localData = 0x0000;
			retVal = getRegister(pDCB, MISC_TEST_IO_DATA, 1, &regData);
			if (retVal == NXT_NO_ERROR) {	
				localData = (Data16)(regData << 8);
				/* localData &= 0xFF00; */
				retVal = getRegister(pDCB, MISC_GPIO_MONITOR, 1, &regData);
				if (retVal == NXT_NO_ERROR) {
					localData |= regData;
				}
			}
			*pData = localData;
			break;
		case NXT_GPIO_WRITE:
			/* if writing, test for low-power mode error */
			retVal = getRegister(pDCB, MISC_POWER_CONTROL, 1, &coreCon);
			if (coreCon & MISC_POWER_DN_FAT) {
				retVal = NXT_ERR_MODE;
			} else {
				regData = (Data8) (localData & 0xFF);
				retVal = setRegister(pDCB, MISC_GPIO_OEN, 1, &regData);
				if (retVal == NXT_NO_ERROR) {
					regData = (Data8) ((localData >> 8) & 0xFF);
					retVal = setRegister(pDCB, MISC_TEST_IO_DATA, 1, &regData);
				}
			}
			break;
		default:
			/* mode not legal */
			retVal = NXT_ERR_RANGE;
			break;
		}/* mode */

		/* release comm semaphore */
		NxtLeaveCriticalSection(pContext, pDCB->hCSNxtComm);
	}
	
	return retVal;
} /* NxtGpioControl */


/**********************************************************************
 *
 * NxtCoreControl
 *
 * Optionally resets/powers-down selected core sections.
 *
 * Inputs:
 *	void *pContext - NULL, or user-defined ID for multiple NXTs
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
					NxtCoreControl_t coreControl) {

	Data16 retVal = NXT_NO_ERROR;
	NxtDCB *pDCB;
	NxtDCB *pDummy;

	/* find the referenced DCB */
	pDCB = FindDCB(pContext, &pDummy);

	if (pDCB == INVALID_CONTEXT) {	/* referenced DCB never initialized */
		retVal = NXT_ERR_INIT;
	}
	else {
		retVal = regCoreControl(pDCB, coreControl);
	}

	return retVal;
} /* NxtCoreControl */


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
					NxtAdcGainControl_t adcGainCtl) {

	Data16 retVal = NXT_NO_ERROR;
	NxtDCB *pDCB;
	NxtDCB *pDummy;
	Data8	buffer;

	/* find the referenced DCB */
	pDCB = FindDCB(pContext, &pDummy);

	if (pDCB == INVALID_CONTEXT) {	/* referenced DCB never initialized */
		retVal = NXT_ERR_INIT;
	}
	else {

		/* range check all inputs */
		if ( adcGainCtl != NXT_FAT_ADC_GAIN_1V
			 && adcGainCtl != NXT_FAT_ADC_GAIN_2V 
#ifndef NXT2004
			 && adcGainCtl != NXT_FDC_ADC_GAIN_1V 
			 && adcGainCtl != NXT_FDC_ADC_GAIN_2V
#endif
			 ) {
			
			retVal = NXT_ERR_RANGE;

		} else {

			retVal = getRegister(pDCB, MISC_ELARA_SERIAL_CMD_3, 1, &buffer);

			if (NXT_NO_ERROR == retVal) {

				switch (adcGainCtl) {
#ifndef NXT2004
				case NXT_FDC_ADC_GAIN_2V:
						buffer &= ~MISC_FDC_ADC_GAIN_1V;
						break;
					case NXT_FDC_ADC_GAIN_1V:
						buffer |= MISC_FDC_ADC_GAIN_1V;
						break;
#endif
					case NXT_FAT_ADC_GAIN_2V:
						buffer &= ~MISC_FAT_ADC_GAIN_1V;
						break;
					case NXT_FAT_ADC_GAIN_1V:
						buffer |= MISC_FAT_ADC_GAIN_1V;
						break;
					default:
						break;
				} /* switch */

				retVal = setRegister(pDCB, MISC_ELARA_SERIAL_CMD_3, 1, &buffer);
			} /* if */

		} /* else */

	} /* else */

	return retVal;
																				
}/* NxtSetAdcInputGain */

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
					NxtControl_t podOutput) {

	Data16	retVal = NXT_NO_ERROR;
	NxtDCB *pDCB;
	NxtDCB *pDummy;
	Data8	control;
	
	/* find the referenced DCB */
	pDCB = FindDCB(pContext, &pDummy);

	if (pDCB == INVALID_CONTEXT) {	/* referenced DCB never initialized */
		retVal = NXT_ERR_INIT;
	}
	else {

		/* wait for comm semaphore */
		NxtWaitForCriticalSection(pContext, pDCB->hCSNxtComm);

		/* read MISC_MOD_CTRL_OUT_FMT */
		retVal = getRegister(pDCB, MISC_POWER_CONTROL, 1, &control);

		if (NXT_NO_ERROR == retVal) 
		{
			/* modify output format bits */
			control &= MISC_POWER_CTL_MASK;

			if (fatAgcOutput) {
				control |= FAT_AGC_OUT_ENABLE;
			}
			if (mpegOutput) {
				control |= MPEG_OUT_ENABLE;
			}
#ifndef NXT2004
			if (podOutput) {
				control |= POD_OUT_ENABLE;
			}
			if (fdcAgcOutput) {
				control |= FDC_AGC_OUT_ENABLE;
			}
#endif
#ifdef NXT2005
			if (pgaOutput) {
				control |= PGA_OUT_ENABLE;
			}
#endif

			/* write MISC_POWER_CONTROL */
			retVal = setRegister(pDCB, MISC_POWER_CONTROL, 1, &control);
		}
		/* release comm semaphore */
		NxtLeaveCriticalSection(pContext, pDCB->hCSNxtComm);

		/* save current format settings in DCB */
		pDCB->outputControl.pgaOutput = pgaOutput;
		pDCB->outputControl.fdcAgcOutput = fdcAgcOutput;
		pDCB->outputControl.fatAgcOutput = fatAgcOutput;
		pDCB->outputControl.mpegOutput = mpegOutput;
		pDCB->outputControl.podOutput = podOutput;
	}

	return retVal;
}/* NxtOutputControl */


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
 *	NxtBerterRemove_t removeBytes - number of bytes to remove in header
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
					Data8 winSize) {
	Data16	retVal = NXT_NO_ERROR;
	NxtDCB *pDCB;
	NxtDCB *pDummy;
	Data8	buffer;

	/* find the referenced DCB */
	pDCB = FindDCB(pContext, &pDummy);

	if (pDCB == INVALID_CONTEXT) {	/* referenced DCB never initialized */
		retVal = NXT_ERR_INIT;
	}
	else {

		/* range check all inputs */
		if ((bertSource != NXT_BERT_INPUT_FAT &&
			 bertSource != NXT_BERT_INPUT_DI &&
#ifndef NXT2004
			 bertSource != NXT_BERT_INPUT_FDC &&
#endif
			 bertSource != NXT_BERT_INPUT_TRELLIS)	||

			(bertAlgorithm != NXT_BERT_PN_15 &&
			 bertAlgorithm != NXT_BERT_PN_23)	||
			 
			(PNSequence != NON_INVERTED && 
			 PNSequence != INVERTED)	|| 

			(removeBytes != NXT_BERT_RM_0 &&
			 removeBytes != NXT_BERT_RM_1 &&
			 removeBytes != NXT_BERT_RM_3 &&
			 removeBytes != NXT_BERT_RM_4)	||

			 (winSize > 0x1F))							{

			retVal = NXT_ERR_RANGE;
		}
		else {

			/* make sure BERT is turned off */
			retVal = NxtStopBert(pContext);			
			
			/* wait for comm semaphore */
			NxtWaitForCriticalSection(pContext, pDCB->hCSNxtComm);

			if (NXT_NO_ERROR == retVal) {

				retVal = getRegister(pDCB, BERT_CTL_A, 1, &buffer);

				buffer &= ~BERT_CTL_A_MASK;

				/* modify the configuration bits */
				buffer |= bertSource;
				buffer |= bertAlgorithm;
				buffer |= 0x04 * PNSequence;/* the bit is in D2 location */
				buffer |= removeBytes;
				retVal |= setRegister(pDCB, BERT_CTL_A, 1, &buffer);
			
				/* set window size */
				retVal |= setRegister(pDCB, BERT_WIN_SIZE, 1, &winSize);
			
				/* write number of sync windows and number of acceptable errors
				** in a window before lock can be declared */
				buffer = (NXT_BERT_SYNC_LOSS | NXT_BERT_SYNC_ACQ);
				retVal |= setRegister(pDCB, BERT_SYNC_ACQ_LOSS, 1, &buffer);
				
				/* write BERT synchronizer error threshold */
				buffer = NXT_BERT_SYNC_THOLD;
				retVal |= setRegister(pDCB, BERT_SYNC_THOLD, 1, &buffer);
		
				if (retVal == NXT_NO_ERROR) {
					/* save the settings in DCB */
					pDCB->bertFormat.bertSource		= bertSource;
					pDCB->bertFormat.bertAlgorithm	= bertAlgorithm;
					pDCB->bertFormat.PNSequence		= PNSequence;
					pDCB->bertFormat.removeBytes	= removeBytes;
					pDCB->bertFormat.winSize		= winSize;
				}
			}		
			
			/* release comm semaphore */
			NxtLeaveCriticalSection(pContext, pDCB->hCSNxtComm);
		}

	}

	return retVal;
} /* NxtConfigInternalBert */

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
DRV2_API Data16 NxtStartBert(void *pContext) {
	
	Data16 retVal;
	NxtDCB *pDCB;
	NxtDCB *pDummy;
	Data8	buffer;

	pDCB = FindDCB(pContext, &pDummy);
	if (pDCB == INVALID_CONTEXT) {	/* referenced DCB never initialized */

		retVal = NXT_ERR_INIT;
	}
	else {
		/* wait for comm semaphore */
		NxtWaitForCriticalSection(pContext, pDCB->hCSNxtComm);

		retVal = getRegister(pDCB, BERT_CTL_A, 1, &buffer);
		
		if (NXT_NO_ERROR == retVal) {

			buffer |= BERT_CTL_A_ON;
			retVal = setRegister(pDCB, BERT_CTL_A, 1, &buffer);
		}

		/* release comm semaphore */
		NxtLeaveCriticalSection(pContext, pDCB->hCSNxtComm);
	}

	return retVal;
} /* NxtStartBert */

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
DRV2_API Data16 NxtStopBert(void *pContext) {

	Data16 retVal;
	NxtDCB *pDCB;
	NxtDCB *pDummy;
	Data8	buffer;

	pDCB = FindDCB(pContext, &pDummy);
	if (pDCB == INVALID_CONTEXT) {	/* referenced DCB never initialized */

		retVal = NXT_ERR_INIT;
	}
	else {
		/* wait for comm semaphore */
		NxtWaitForCriticalSection(pContext, pDCB->hCSNxtComm);

		retVal = getRegister(pDCB, BERT_CTL_A, 1, &buffer);
		
		if (NXT_NO_ERROR == retVal) {

			buffer &= ~BERT_CTL_A_ON;
			retVal = setRegister(pDCB, BERT_CTL_A, 1, &buffer);
		}

		/* release comm semaphore */
		NxtLeaveCriticalSection(pContext, pDCB->hCSNxtComm);
	}

	return retVal;
} /* NxtStopBert */

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
					Data8 interruptMask) {
	Data16	retVal;
	NxtDCB *pDCB;
	NxtDCB *pDummy;
	
	/* find the referenced DCB */
	pDCB = FindDCB(pContext, &pDummy);
	
	if (pDCB == INVALID_CONTEXT) {	/* referenced DCB never initialized */
		retVal = NXT_ERR_INIT;
	}
	else {

		/* wait for comm semaphore */
		NxtWaitForCriticalSection(pContext, pDCB->hCSNxtComm);

		retVal = setRegister(pDCB, UC_IRQ_MASK, 1, &interruptMask);

		if (NXT_NO_ERROR == retVal) {
			/* save interrupt mask */
			pDCB->irqMask = interruptMask;
		}

		/* release comm semaphore */
		NxtLeaveCriticalSection(pContext, pDCB->hCSNxtComm);
	}
	return retVal;
} /* NxtSetInterruptMask */



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
					Data8 *pStatusResult) {
	Data16	retVal;
	NxtDCB *pDCB;
	NxtDCB *pDummy;
	Data8	regData;
	Data8	lInterruptSource;	/* local used to read source register */
	
	/* find the referenced DCB */
	pDCB = FindDCB(pContext, &pDummy);
	
	if (pDCB == INVALID_CONTEXT) {	/* referenced DCB never initialized */
		retVal = NXT_ERR_INIT;
	}
	else {

		/* wait for comm semaphore */
		NxtWaitForCriticalSection(pContext, pDCB->hCSNxtComm);

		*pInterruptSource = 0;
		*pStatusResult = 0;
		retVal = getRegister(pDCB, UC_IRQ_SOURCE, 1, &lInterruptSource);

		if (retVal == NXT_NO_ERROR)
		{
			if (0 == lInterruptSource) 
			{
				retVal |= NXT_ERR_OTHER;	
			}
			else while (lInterruptSource != 0) 
			{
				/* report all interrupts, but only the last lock or loss event */
				if (lInterruptSource & ((NXT_INT_FAT_LOCK | NXT_INT_FAT_LOSS) | 
					(NXT_INT_FDC_LOCK | NXT_INT_FDC_LOSS))) 
				{
					*pInterruptSource &= ~((NXT_INT_FAT_LOCK | NXT_INT_FAT_LOSS) | 
						(NXT_INT_FDC_LOCK | NXT_INT_FDC_LOSS));
				}
				*pInterruptSource |= lInterruptSource;

				if (lInterruptSource & (NXT_INT_NTSC | NXT_INT_XFER)) 
				{
					/* get Xfer error status and NTSC result */
					retVal |= getRegister(pDCB, UC_SERVICES, 1, &regData);
					
					if (regData & UC_SERVICES_NTSC_STATUS) {
						*pStatusResult |= NXT_NTSC_DETECTED;
					}
					
					if (regData & UC_SERVICES_DATA_XFER_ERR) {
						*pStatusResult |= NXT_XFER_ERROR;
					}
				}
				/* guarantee that the source register is clear before returning */
				retVal |= getRegister(pDCB, UC_IRQ_SOURCE, 1, &lInterruptSource);
			}/* while */
		}

		/* release comm semaphore */
		NxtLeaveCriticalSection(pContext, pDCB->hCSNxtComm);
	}
	return retVal;
} /* NxtGetInterruptSource */


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
					Data8 *pBuffer) {

	Data16	retVal;
	NxtDCB *pDCB;
	NxtDCB *pDummy;
	
	/* find the referenced DCB */
	pDCB = FindDCB(pContext, &pDummy);
	
	if (pDCB == INVALID_CONTEXT) {	/* referenced DCB never initialized */
		retVal = NXT_ERR_INIT;
	}
	else {

		/* wait for comm semaphore */
		NxtWaitForCriticalSection(pContext, pDCB->hCSNxtComm);

		retVal = getRegister(pDCB, 
					IIC_XFER_DATA_0, 
					byteCount, 
					pBuffer);

		/* release comm semaphore */
		NxtLeaveCriticalSection(pContext, pDCB->hCSNxtComm);
	}

	return retVal;
} /* NxtReadIicXferData */

#endif ALPSTUNER
