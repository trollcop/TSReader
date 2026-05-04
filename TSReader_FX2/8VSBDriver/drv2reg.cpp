#ifdef ALPSTUNER
/**********************************************************************
 * DRV2REG.C
 * Handles the register-based interface to NXT200X
 *
 * Copyright (C) 2002 NxtWave Communications, Inc.
 *
 * $Log:   //panxtbdc01/AppsEng/PVCS/Application Eval/archives/EVAL2005/Drv2005/Drv2reg.c-arc  $
 * 
 *    Rev 1.69   Apr 24 2003 11:38:30   raggarwa
 * Changed Smoother target to 0 for burst mode, PR 43/2005
 * 
 *    Rev 1.68   Mar 26 2003 17:03:16   raggarwa
 * renamed some variables;
 * put FDC variables in conditional compile area;
 * put RDC function in conditional compile area
 * 
 *    Rev 1.67   Mar 26 2003 09:45:26   raggarwa
 * Changed commenting style for ANSI compliance
 * 
 *    Rev 1.66   Mar 25 2003 10:45:22   raggarwa
 * Changed the way code is downloaded - CRC is done at the end of code download
 * 
 *    Rev 1.65   Mar 20 2003 16:49:08   raggarwa
 * Changed location of stop acquisition, to when micro is not in reset
 * 
 *    Rev 1.64   Mar 19 2003 10:16:04   raggarwa
 * Fixed bugs in Core Control function
 * 
 *    Rev 1.63   Feb 28 2003 13:44:34   raggarwa
 * Added RDC initialization;
 * Added a function for sending RDC taps;
 * updated comments
 * 
 *    Rev 1.62   Feb 11 2003 14:42:56   raggarwa
 * Changed // type comments, for release 3.0.0
 * 
 *    Rev 1.61   Feb 10 2003 13:26:38   raggarwa
 * Fixed MPEG clock polarity check in regCoreControl(), PR# 19/2005
 * 
 *    Rev 1.60   Feb 03 2003 12:04:52   raggarwa
 * Added <= check for RF ON thold during active tracking, PR 21/2005
 * 
 *    Rev 1.59   Jan 20 2003 13:40:48   raggarwa
 * Added semaphore in regActiveTracking(); removed NxtSuspendThread()  in regStop( ) PR 108/2003
 * 
 *    Rev 1.58   Dec 20 2002 11:46:46   raggarwa
 * Got rid of compiler warnings
 * 
 *    Rev 1.57   Dec 17 2002 14:02:22   raggarwa
 * OR'ed 0x01 for iicRegRead, PR 100/2003
 * 
 *    Rev 1.56   Dec 09 2002 21:30:30   sreichgo
 * Added separate ACQUISITION_DELAY values for 8VSB, 64QAM, and 256QAM.  This means they all don't wait for
 * the slowest.
 * 
 * Increased XFER_SERVICE_MAX_TRIES from 100 to 200 ms
 * to accommodate new firmware that may wait 100 ms for lock.
 * 
 * Changed delay after setting adjacent AGC values from
 * ACQUISITION_DELAY to START_SCRIPT_DELAY.
 * 
 * Added acquisition delay back into regConfig, and removed
 * it from regConfigCable.
 * 
 *    Rev 1.55   Dec 06 2002 16:00:26   raggarwa
 * Added <= check for RF ON thold
 * 
 *    Rev 1.54   Dec 04 2002 18:28:26   raggarwa
 * Fixed conditional compile problem in regCoreControl, PR 96/2003
 * 
 *    Rev 1.53   Dec 04 2002 16:53:44   sreichgo
 * Added smoother core to multibyte map to avoid any critical section/arbitration problems.
 * 
 *    Rev 1.52   Nov 27 2002 13:03:16   raggarwa
 * Changed acquisition delay to 500 ms
 * 
 *    Rev 1.51   Oct 28 2002 11:58:34   raggarwa
 * Added NXT_AGC_RF_THRESHOLD check in regAgcSetup( )
 * 
 *    Rev 1.50   Oct 01 2002 16:10:20   raggarwa
 * Added a delay after abort flag is cleared during acquisition stopping
 * 
 *    Rev 1.49   Sep 24 2002 10:06:28   raggarwa
 * Changed tries to Data16 in regGetBertData, PR 67/2003
 * 
 *    Rev 1.48   Sep 05 2002 11:55:00   raggarwa
 * Merged main trunk and nxtenna
 * 
 *    Rev 1.47   Aug 22 2002 18:18:50   raggarwal
 * Fixed a CS bug in regActiveTracking( )
 *    Rev 1.46   Aug 15 2002 10:16:38   raggarwal
 * Removed reset cores before power-up/down of FAT and FDC cores, PR 51/2003
 *    Rev 1.45   Aug 06 2002 09:43:56   raggarwal
 * Changed to not reset FAT cores during FAT power down. This saves FDC VGA from going out of control.
 * 
 *    Rev 1.44   Jul 02 2002 10:24:56   raggarwal
 * Masked FDC symbol rate while writing to UC_GP_0 in startFatAcq( )
 * 
 *    Rev 1.43.1.1   Aug 06 2002 09:50:54   raggarwal
 * Fixed a bug in timeout check in regNxtennaSvc()
 * 
 *    Rev 1.43.1.0   Jun 27 2002 17:32:56   raggarwal
 * Added regNxtennaSvc( )
 * 
 *    Rev 1.43   Jun 24 2002 16:04:12   raggarwal
 * Added USMOD enable during regInitAsic()
 * 
 *    Rev 1.42   Jun 19 2002 10:49:10   raggarwal
 * Alpha3 Release
 * 
 *    Rev 1.41   Jun 19 2002 10:27:08   raggarwal
 * Alpha3 Release
 * 
 *    Rev 1.40   Jun 17 2002 11:06:32   raggarwal
 * Alpha3 Release
 * 
 *    Rev 1.37   May 17 2002 16:06:10   raggarwal
 * Alpha2 Release
 * 
 *    Rev 1.13   Mar 08 2002 16:28:38   raggarwal
 * Alpha Release
 * 
 **********************************************************************/

#include <windows.h>

/*
******************************************************************************
Includes
******************************************************************************
*/
#include "NxtCommon.h"
#include "drv2cntx.h"
#include "drv2crc.h"
#include "drv2reg.h"
#include "drv2load.h"

/*
******************************************************************************
Defines
******************************************************************************
*/
#define LOCK_DEBOUNCE_COUNT 3
#define LOCK_DEBOUNCE_DELAY 25

#define START_SCRIPT_DELAY	100
#define ACQUISITION_DELAY_8VSB		300
#define ACQUISITION_DELAY_64QAM		300
#define ACQUISITION_DELAY_256QAM	500

#define STOP_ACQ_MAX_TRIES	10
#define STOP_ACQ_DELAY		5

#define TRACKING_TIMEOUT	100

#define BAUD_RATE_VSB		10762238.0
#define BAUD_RATE_QAM64		5056941
#define BAUD_RATE_QAM256	5360537
#define BAUD_RATE_FDC_772	 772000
#define BAUD_RATE_FDC_1024	1024000
#define BAUD_RATE_FDC_1544	1544000
#define PI					3.141592653589

#define CAR_FRQ_SCALE_FACTOR(br) ((Data32) (((1 << 28)/(double)br)*2.0 * PI))
#define PIL_FRQ_SCALE_FACTOR(br) ((Data32) (((1 << 29)/(double)br)*2.0 * PI))

#define MASTER_CLOCK		50280000

#define MULTIBYTE_REGISTERS	(sizeof(multibyteRegister)/sizeof(multibyteReg_s))

#ifdef EIGHTVSB
#define XFER_SERVICE_MAX_TRIES	750
#endif
#ifdef QAM
#define XFER_SERVICE_MAX_TRIES	1500
#endif QAM

/* Delay and maximum number of polls for BERT data */
#define BERT_DATA_POLL_DELAY	1000
#define	BERT_DATA_MAX_TRIES		1390	/* for slowest data rate, max win size */

#define RDC_TAPS_SIZE			32

/*
******************************************************************************
Private Types
******************************************************************************
*/

/*
******************************************************************************
Static variables
******************************************************************************
*/

static const char multibyteTestDev0[256] = {
/*  	0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F  */
/* 0 */	0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0,
/* 1 */	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
/* 2 */	0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 2, 0, 0, 0, 1, 2,
/* 3 */	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
/* 4 */	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
/* 5 */	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
/* 6 */	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
/* 7 */	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
/* 8 */	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
/* 9 */	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
/* A */	0, 0, 1, 2, 3, 4, 1, 2, 3, 4, 0, 0, 0, 0, 0, 0,
/* B */	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
/* C */	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
/* D */	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
/* E */	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
/* F */	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};/* multibyteTestDev0[] */


static const char multibyteTestDev1[256] = {
/*  	0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F  */
/* 0 */	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
/* 1 */	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
/* 2 */	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
/* 3 */	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
/* 4 */	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 2, 0, 1,
/* 5 */	2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
/* 6 */	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 2,
/* 7 */	3, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
/* 8 */	1, 1, 1, 2, 3, 1, 2, 3, 1, 1, 2, 1, 1, 1, 1, 1,
/* 9 */	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
/* A */	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
/* B */	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
/* C */	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
/* D */	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
/* E */	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
/* F */	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};/* multibyteTestDev1[] */

/*
******************************************************************************
Public Functions
******************************************************************************
*/

/**********************************************************************
 *
 * regInitASIC
 *
 * NxtInitASIC200X for register-based interface
 *
 * Inputs:
 *	NxtDCB *pDCB - pointer to current DCB -- ASSUMED FOUND!!!
 *
 * Returns:
 *	NXT_NO_ERROR
 *	NXT_ERR_COMM
 *
 **********************************************************************/
Data16 regInitASIC(NxtDCB *pDCB) {

	Data16	retVal = NXT_NO_ERROR;
	Data16	startAddress;
	Data8	hwStatus;
	Data8	outFmt;

	/* check for download required */
	if (defaultCodeDscp.pBuffer != NULL) {
		
		/* wait for comm semaphore */
		NxtWaitForCriticalSection(pDCB->pContext, pDCB->hCSNxtComm);

		if (defaultCodeDscp.bEnableROM != TRUE && 
			NXT_NO_ERROR == retVal) {

			/* disable ROM */
			retVal = getRegister(pDCB, MISC_ASIC_HW_STATUS_1, 1, &hwStatus);

			if (NXT_NO_ERROR == retVal) {
			
				hwStatus |= INT_ROM_DISABLED;
				retVal = setRegister(pDCB, MISC_ASIC_HW_STATUS_1, 1, &hwStatus);
			
			}
		}/* disable ROM required */

		/* release comm semaphore */
		NxtLeaveCriticalSection(pDCB->pContext, pDCB->hCSNxtComm);

		if (NXT_NO_ERROR == retVal) {

			/* start address depends on ROM DISABLED bit */
			retVal = getRamBase(pDCB, &startAddress);

			/* load current code */
			if (NXT_NO_ERROR == retVal) {
				retVal = regLoadRAM(pDCB,
							   NXT_LOAD_CODE,
							   startAddress,
							   defaultCodeDscp.byteCount,
							   defaultCodeDscp.pBuffer);
			}
		}
	} /* pBuffer != NULL */

#ifndef NXT2004

	/* Enable ROM-ZS to use FDC script */
	retVal |= enableRomFdc(pDCB);

	/* stop FDC acquisition */
	stopFdcAcquisition(pDCB);

#endif

	/* disable FAT acquisition */
	stopFatAcquisition(pDCB);

	/* release micro */
	unresetMicro(pDCB);
		
	/* reset cores */
	regCoreControl(pDCB, CORE_SOFT_RESET_ALL);


#ifndef NXT2004
	/* Power up FDC */
	if (NXT_NO_ERROR == retVal) {
		regCoreControl(pDCB, CORE_POWER_UP_FDC);
	}
#endif

#ifdef NXT2005
	/* Power up RDC */
	if (NXT_NO_ERROR == retVal) {
		regCoreControl(pDCB, CORE_POWER_UP_RDC);
	}
#endif

	/* wait for comm semaphore */
	NxtWaitForCriticalSection(pDCB->pContext, pDCB->hCSNxtComm);
	
	/* enable Output Format Smoother */
	retVal |= getRegister(pDCB, MISC_ASIC_HW_STATUS_1, 1, &hwStatus);
	if (NXT_NO_ERROR == retVal) {
		hwStatus |= MPEG_OUT_SMOOTHER_EN;
		retVal = setRegister(pDCB, MISC_ASIC_HW_STATUS_1, 1, &hwStatus);
	}
	/* set MPEG clock polarity to NON_INVERTED */
	if (NXT_NO_ERROR == retVal) {
		retVal = getRegister(pDCB, MISC_MOD_CTRL_OUT_FMT, 1, &outFmt);
		if (NXT_NO_ERROR == retVal) {
			outFmt |= OUT_FMT_POL_CLOCK;
			retVal = setRegister(pDCB, MISC_MOD_CTRL_OUT_FMT, 1, &outFmt);
		}
	}

#ifdef NXT2005
	/* enable upstream modulator */
	retVal |= getRegister(pDCB, USMOD_CONTROL, 1, &hwStatus);
	if (NXT_NO_ERROR == retVal) {
		hwStatus |= USMOD_CTL_MOD_ENABLE;
		retVal |= setRegister(pDCB, USMOD_CONTROL, 1, &hwStatus);
	}
	/* set modulator's output data format */
	retVal |= getRegister(pDCB, USMOD_IFACE_CONTROL, 1, &hwStatus);
	if (NXT_NO_ERROR == retVal) {
		hwStatus |= USMOD_OUT_DATA_FMT;
		retVal |= setRegister(pDCB, USMOD_IFACE_CONTROL, 1, &hwStatus);
	}
#endif
	
	/* release comm semaphore */
	NxtLeaveCriticalSection(pDCB->pContext, pDCB->hCSNxtComm);

	retVal |= NxtennaSetConfig(pDCB->pContext, pDCB->nxtennaMap, 
		pDCB->nxtennaGain, pDCB->nxtennaPolBit);

	/* set the ASIC Initialized flag */
	if (NXT_NO_ERROR == retVal)
		pDCB->bAsicInitialized = TRUE;

	return retVal;

} /* regInitASIC */


/**********************************************************************
 *
 * regStart
 *
 * NxtStart for register-based interface.
 * Configures the AGC and starts acquisition for in-band modes,
 * starts acquisition for out-of-band modes.
 *
 * Inputs:
 *	NxtDCB *pDCB - pointer to current DCB -- ASSUMED FOUND!!!
 *	NxtAcqOptions_t - stores the modulation format and start options
 *
 * Outputs:
 *	Data16 *pResult - failed bits in startOptions are turned to zero
 *
 * Returns:
 *	NXT_NO_ERROR
 *	NXT_ERR_COMM
 *
 **********************************************************************/
Data16 regStart(NxtDCB *pDCB,
			NxtAcqOptions_t startOptions,
			Data16 *pResult) {

	Data16	retVal = NXT_NO_ERROR;
	Data8	ifLevelReg[2];
	Data8	rfLevelReg[2];
	Data8	agcControl;
	Data16	mode;
	Bool	bDualLoop;
	
	if (startOptions & NXT_ACTIVE_TRACKING_FAT) {
		regActiveTracking(pDCB, TRUE);
		*pResult |= NXT_ACTIVE_TRACKING_FAT;
	} 
#ifndef NXT2004
	else if (startOptions & NXT_ACTIVE_TRACKING_FDC) {
		regActiveTracking(pDCB, FALSE);
		*pResult |= NXT_ACTIVE_TRACKING_FDC;
	}
#endif
	mode = startOptions & NXT_START_CHANNEL_MASK;
	
	switch (mode) {

	case NXT_CONFIG_8VSB:

		/* attempt to acquire with multipath optimization */
		retVal = regConfig(pDCB, startOptions, pResult);
		
		if (!(startOptions & NXT_START_ADJ_MASK)) {
			pDCB->bFatAdjacent = FALSE;

			if (NXT_NO_ERROR == retVal) {

				/* wait for comm semaphore */
				NxtWaitForCriticalSection(pDCB->pContext, pDCB->hCSNxtComm);
			
				/* check IF level for adjacent */
				retVal = getRegister(pDCB, AGC_SDM1_INPUT_MSB, 2, ifLevelReg);

				/* determine if this is a dual loop system */
				if (NXT_NO_ERROR == retVal) {
					retVal = getRegister(pDCB, AGC_CONTROL, 1, &agcControl);
					bDualLoop = (0 != (agcControl & AGC_CONTROL_DUAL));

					if (bDualLoop && (NXT_NO_ERROR == retVal)) {
						/* check RF level for adjacent */
						retVal = getRegister(pDCB, AGC_SDM2_INPUT_MSB, 2, rfLevelReg);
					}
				}

				/* release comm semaphore */
				NxtLeaveCriticalSection(pDCB->pContext, pDCB->hCSNxtComm);

				if (NXT_NO_ERROR == retVal) {
					/* test case for single loop or dual loop */
					pDCB->bFatAdjacent = 
						((DATA8_TO_DATA16(ifLevelReg)) > pDCB->fatIfAdjOn);

					if (bDualLoop) {	/* dual loop AGC */
						/* also check RF AGC gain */
						pDCB->bFatAdjacent = pDCB->bFatAdjacent && 
								((DATA8_TO_DATA16(rfLevelReg)) <= pDCB->fatRfAdjOn);
					}

					if (pDCB->bFatAdjacent) {

						/* probably an adjacent channel */
						if (pDCB->pFatAgcAdj != (Data16 *)NULL) {
						
							retVal = regAgcSetup(pDCB, pDCB->pFatAgcAdj);
							pDCB->bFatAdjacent = TRUE;
							pDCB->bFatStartScriptDone = FALSE;
							NxtSuspendThread(pDCB->pContext, START_SCRIPT_DELAY);
						}
					}
				}
			}
		}

		if (retVal == NXT_NO_ERROR) {
			retVal = regNotifyFatLockLoss(pDCB);
		}
		break;

	case NXT_CONFIG_64QAM:
	case NXT_CONFIG_256QAM:
		retVal = regConfig(pDCB, startOptions, pResult);
		if (retVal == NXT_NO_ERROR) {
			retVal = regNotifyFatLockLoss(pDCB);
		}
		break;

	case NXT_CONFIG_CABLE:
		retVal = regConfigCable(pDCB, startOptions, pResult);
		
		if (retVal == NXT_NO_ERROR) {
			retVal = regNotifyFatLockLoss(pDCB);
		}
		
		break;

#ifndef NXT2004
	case NXT_CONFIG_FDC:

		/* NxtConfigFdc() should already have been called */
		
		/* start FDC acquisition */
		retVal = startFdcAcquisition(pDCB);
		
		if (retVal == NXT_NO_ERROR) {
			retVal = regNotifyFdcLockLoss(pDCB);
		}
		
		break;
#endif

	default:
		break;
	} /* mode */


	return retVal;

}/* regStart */

/**********************************************************************
 *
 * regStop
 *
 * Stops acquisition and tracking.
 *
 * Inputs:
 *	void *pContext - NULL, or user-defined ID for multiple NXT200Xs
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
Data16 regStop(NxtDCB *pDCB, NxtAcqOptions_t stopOptions) {

	Data16	retVal = NXT_NO_ERROR;
	Data8	buffer = (Data8) (stopOptions & 0xFF);

	buffer &= NXT_STOP_OPTIONS_MASK;

	if (0x00 == buffer) {

		retVal = NXT_ERR_RANGE;

	} else {

		/* first stop the driver */
		if (buffer & NXT_STOP_FAT) {
			pDCB->bAbortFatFlag = TRUE;
			/*NxtSuspendThread(pDCB->pContext, 2*TRACKING_TIMEOUT);*/
		}
#ifndef NXT2004
		if (buffer & NXT_STOP_FDC) {
			pDCB->bAbortFdcFlag = TRUE;
			/*NxtSuspendThread(pDCB->pContext, 2*TRACKING_TIMEOUT);*/
		}
#endif
		if (buffer & NXT_STOP_ASYNC_MASK) {

			if ((buffer & NXT_STOP_FAT) && 
				(!NxtRequestCriticalSection(pDCB->pContext, pDCB->hCSNxtFatTrack))) {
				/* no inband active tracking thread running, release cs */
				NxtLeaveCriticalSection(pDCB->pContext, pDCB->hCSNxtFatTrack);
				pDCB->bAbortFatFlag = FALSE;
			}

#ifndef NXT2004
			if ((buffer & NXT_STOP_FDC) && 
				(!NxtRequestCriticalSection(pDCB->pContext, pDCB->hCSNxtFdcTrack))) {
				/* no out-of-band active tracking thread running, release CS */
				NxtLeaveCriticalSection(pDCB->pContext, pDCB->hCSNxtFdcTrack);
				pDCB->bAbortFdcFlag = FALSE;
			}
#endif

			/* if a thread was running, it must clear bAbortFlag */
		}
		else {
			if (buffer & NXT_STOP_FAT) {
				/* wait for tracking semaphore */
				NxtWaitForCriticalSection(pDCB->pContext, pDCB->hCSNxtFatTrack);

				/* release tracking semaphore */
				NxtLeaveCriticalSection(pDCB->pContext, pDCB->hCSNxtFatTrack);

				pDCB->bAbortFatFlag = FALSE;
			}
#ifndef NXT2004
			if (buffer & NXT_STOP_FDC) {
				/* wait for tracking semaphore */
				NxtWaitForCriticalSection(pDCB->pContext, pDCB->hCSNxtFdcTrack);

				/* release tracking semaphore */
				NxtLeaveCriticalSection(pDCB->pContext, pDCB->hCSNxtFdcTrack);

				pDCB->bAbortFdcFlag = FALSE;
			}
#endif
		}

		/* now stop FAT/FDC acquisitions */
		if (buffer & NXT_STOP_FAT) {
			stopFatAcquisition(pDCB);
		}

#ifndef NXT2004
		if (buffer & NXT_STOP_FDC) {
			stopFdcAcquisition(pDCB);
		}	
#endif

	}

	return retVal;
} /* regStop */

/**********************************************************************
 *
 * regAgcSetup
 *
 * Runs AGC setup script.
 *
 * Inputs:
 *	NxtDCB	*pDCB - pointer to current DCB -- ASSUMED FOUND!!!
 *	Data8	*pSetupData - pointer to setup script data
 *
 * Returns:
 *	Data16 
 *
 **********************************************************************/
Data16 regAgcSetup(NxtDCB *pDCB,
			Data16 *pSetupData ) {

	Data8	i,j;
	Data8	buffer[4] = {0};
	Data16	retVal = NXT_NO_ERROR;

	/* wait for comm semaphore */
	NxtWaitForCriticalSection(pDCB->pContext, pDCB->hCSNxtComm);

	for ( i=0; 
		  (pSetupData[i] != NXT_AGC_SETUP_DONE) &&
		  (pSetupData[i] != NXT_AGC_IF_THRESHOLD) &&
		  (pSetupData[i] != NXT_AGC_RF_THRESHOLD) &&
		  (pSetupData[i] != NXT_AGC_SETUP_256QAM) &&
		  (pSetupData[i] != NXT_AGC_SETUP_64QAM) &&

		  (pSetupData[i] != NXT_FAT_AGC_SETUP_ADJ); 
		  i += pSetupData[i+1] + 2) 
	{
		
		/* make sure the non-first elements in buffer[] are zero */
		for ( j=1;j!=4;j++)
			buffer[j] = 0;

		for (j=0;
			 j!=pSetupData[i+1];
			 j+=1)
		{
			buffer[j] = (Data8)pSetupData[i+2+j];
		}
		retVal |= setRegister( pDCB, 
							   pSetupData[i], 
							   (Data8)pSetupData[i+1], 
							   buffer);
	}

	/* release comm semaphore */
	NxtLeaveCriticalSection(pDCB->pContext, pDCB->hCSNxtComm);

	return retVal;
} /* regAgcSetup */

/**********************************************************************
 *
 * regConfig
 *
 * configures NXT200X for a single in-band modulation format
 * should not be called for QPSK channel
 *
 * Inputs:
 *	NxtDCB *pDCB - pointer to current DCB -- ASSUMED FOUND!!!
 *	NxtAcqOptions_t configMode - user's configuration mode selection
 *
 * Outputs:
 *	Data16 *pResult 
 *
 * Returns:
 *	NXT_NO_ERROR
 *	NXT_ERR_COMM
 *
 **********************************************************************/
Data16 regConfig(NxtDCB *pDCB,
					  NxtAcqOptions_t startOptions,
					  Data16 *pResult) {

	Data16	retVal = NXT_NO_ERROR;
	Data8	ucGp0;
	static	NxtAcqOptions_t lastMode = NXT_DIRECT_TUNE;	/* init to invalid mode */
	Data8	coreControl, coreConTemp;
	Data16	configMode;	/* stores the modulation channel */
	NxtModFormat_t modFmt;
	Data16	acquisitionDelay = 0;

	configMode = startOptions & NXT_START_CHANNEL_MASK;

	/* stop acquisition */
	retVal = stopFatAcquisition(pDCB);

	/* check for mode change */
	if (configMode != lastMode) {
		/* run AGC setup for new mode */
		pDCB->bFatStartScriptDone = FALSE;
		lastMode = (NxtAcqOptions_t)configMode;
	}

	/* determine configuration */
    ucGp0 = 0;  /* initialize: NO ext ram, YES 8VSB, 
				** YES cochannel, NO Alt timing, 8VSB mode */

	if (startOptions & NXT_START_NO_CO_MASK) {
		ucGp0 |= UC_GP_0_NO_CO;
	}

	switch(configMode) {

	case NXT_CONFIG_8VSB:
		modFmt = NXT_8VSB;
		acquisitionDelay = ACQUISITION_DELAY_8VSB;
		ucGp0 |= UC_GP_0_8VSB;
		/* run the standard AGC setup script plus adjacent script, if applicable */
		if (!pDCB->bFatStartScriptDone) {
			if (pDCB->pFatAgcData != (Data16 *)NULL) {

				/* wait for comm semaphore */
				NxtWaitForCriticalSection(pDCB->pContext, pDCB->hCSNxtComm);
				
				/* reset the AGC */
				retVal |= getRegister(pDCB, MISC_RESET_CONTROL, 1, &coreControl);
				coreConTemp = coreControl;
				coreControl |= SOFT_RESET_FAT_AGC;
				retVal |= setRegister(pDCB, MISC_RESET_CONTROL, 1, &coreControl);
				retVal |= setRegister(pDCB, MISC_RESET_CONTROL, 1, &coreConTemp);

				/* release comm semaphore */
				NxtLeaveCriticalSection(pDCB->pContext, pDCB->hCSNxtComm);

				retVal |= regAgcSetup(pDCB, pDCB->pFatAgcData);
				
			}
			if (startOptions & NXT_START_ADJ_MASK) {
				if (pDCB->pFatAgcAdj != (Data16 *)NULL) {
					retVal |= regAgcSetup(pDCB, pDCB->pFatAgcAdj);
					*pResult |= NXT_START_ADJ_MASK;
				}
			}
			NxtSuspendThread(pDCB->pContext, START_SCRIPT_DELAY);
			pDCB->bFatStartScriptDone = TRUE;
		}
		break;

	case NXT_CONFIG_64QAM:
		modFmt = NXT_64QAM;
		acquisitionDelay = ACQUISITION_DELAY_64QAM;
		ucGp0 |= UC_GP_0_64QAM;
		/* run the standard AGC setup script plus 64QAM script, if any */
		if (!pDCB->bFatStartScriptDone) {
			if (pDCB->pFatAgcData != (Data16 *)NULL) {

				/* wait for comm semaphore */
				NxtWaitForCriticalSection(pDCB->pContext, pDCB->hCSNxtComm);

				/* reset the AGC */
				retVal |= getRegister(pDCB, MISC_RESET_CONTROL, 1, &coreControl);
				coreConTemp = coreControl;
				coreControl |= SOFT_RESET_FAT_AGC;
				retVal |= setRegister(pDCB, MISC_RESET_CONTROL, 1, &coreControl);
				retVal |= setRegister(pDCB, MISC_RESET_CONTROL, 1, &coreConTemp);

				/* release comm semaphore */
				NxtLeaveCriticalSection(pDCB->pContext, pDCB->hCSNxtComm);

				retVal |= regAgcSetup(pDCB, pDCB->pFatAgcData);
			}
			if (pDCB->pAgc64Qam != (Data16 *)NULL) {

				retVal |= regAgcSetup(pDCB, pDCB->pAgc64Qam);
			}
			NxtSuspendThread(pDCB->pContext, START_SCRIPT_DELAY);
			pDCB->bFatStartScriptDone = TRUE;
		}
		break;

	case NXT_CONFIG_256QAM:
		modFmt = NXT_256QAM;
		acquisitionDelay = ACQUISITION_DELAY_256QAM;
		ucGp0 |= UC_GP_0_256QAM;
		/* run the standard AGC setup script plus 64QAM script, if any */
		if (!pDCB->bFatStartScriptDone) {
			if (pDCB->pFatAgcData != (Data16 *)NULL) {

				/* wait for comm semaphore */
				NxtWaitForCriticalSection(pDCB->pContext, pDCB->hCSNxtComm);
				
				/* reset the AGC */
				retVal |= getRegister(pDCB, MISC_RESET_CONTROL, 1, &coreControl);
				coreConTemp = coreControl;
				coreControl |= SOFT_RESET_FAT_AGC;
				retVal |= setRegister(pDCB, MISC_RESET_CONTROL, 1, &coreControl);
				retVal |= setRegister(pDCB, MISC_RESET_CONTROL, 1, &coreConTemp);

				/* release comm semaphore */
				NxtLeaveCriticalSection(pDCB->pContext, pDCB->hCSNxtComm);

				retVal |= regAgcSetup(pDCB, pDCB->pFatAgcData);
			}
			if (pDCB->pAgc256Qam != (Data16 *)NULL) {

				retVal |= regAgcSetup(pDCB, pDCB->pAgc256Qam);
			}
			NxtSuspendThread(pDCB->pContext, START_SCRIPT_DELAY);
			pDCB->bFatStartScriptDone = TRUE;
		}
		break;

	default:
		retVal |= NXT_ERR_RANGE;
		break;

	}/* switch */

	if (retVal == NXT_NO_ERROR) {
		/* configure smoother */
		retVal |= regConfigSmoother(pDCB, modFmt);
	}

	
	/* restart acquisition */
	if (retVal == NXT_NO_ERROR) {
		retVal |= startFatAcquisition(pDCB, ucGp0);
		
		/* set the bits in pResult */
		if (NXT_NO_ERROR == retVal) {
			*pResult |= configMode;
			if (ucGp0 & UC_GP_0_NO_CO) {
				*pResult |= NXT_START_NO_CO_MASK;
			}
		}

		retVal |= unresetMicro(pDCB);

		/* delay for acquisition */
		NxtSuspendThread(pDCB->pContext, acquisitionDelay);
	}

	return retVal;
} /* regConfig */


/**********************************************************************
 *
 * regConfigCable
 *
 * Finds proper configuration among 64QAM, 256QAM, or 8VSB,
 * in that order
 *
 * Inputs:
 *	NxtDCB *pDCB - pointer to current DCB -- ASSUMED FOUND!!!
 *
 * Outputs:
 *	NxtScriptParameter_p pScript - config parameters stored here
 *
 * Returns:
 *	NXT_NO_ERROR
 *	NXT_ERR_COMM
 *
 **********************************************************************/
Data16 regConfigCable(NxtDCB *pDCB,
						   NxtAcqOptions_t startOptions,
						   Data16 *pResult) {

	Data16	retVal;
	Bool	bLocked;
	Data16	lValue = (Data16)startOptions;

	lValue &= ~NXT_START_CHANNEL_MASK;

	/* attempt 64QAM */
	retVal = regConfig(pDCB, (NxtAcqOptions_t)(lValue | NXT_CONFIG_64QAM), pResult);
	if (retVal == NXT_NO_ERROR) {
		retVal = regGetFatLockStatus(pDCB, &bLocked);
		if ((retVal == NXT_NO_ERROR) && !bLocked) {
			/* attempt 256QAM */
			retVal = regConfig(pDCB, (NxtAcqOptions_t)(lValue | NXT_CONFIG_256QAM), pResult);
			if (retVal == NXT_NO_ERROR) {
				retVal = regGetFatLockStatus(pDCB, &bLocked);
			}
			if ((retVal == NXT_NO_ERROR) && !bLocked) {
				/* attempt 8VSB */
				retVal = regConfig(pDCB, (NxtAcqOptions_t)(lValue | NXT_CONFIG_8VSB), pResult);
			}
		}
	}

	return retVal;
} /* regConfigCable */

/**********************************************************************
 *
 * regActiveTracking
 *
 * Infinite loop that sends locked/lost notifications - run as a task!
 * One function does for all channels - VSB, QAM, QPSK
 * 
 * Inputs:
 *	NxtDCB *pDCB - pointer to current DCB -- ASSUMED FOUND!!!
 *	Bool bTrackFat -	TRUE  if FAT,
 *						FALSE if FDC
 *
 **********************************************************************/
void regActiveTracking(NxtDCB *pDCB, Bool bTrackFat) {

	Bool	fatLockState;
	Bool	fatLastLockState = TRUE;
	Bool	bDualLoop	= 0;
	Bool	bFatAdjOn;
	Bool	bFatAdjOff;
	Data8	agcControl;
	Data8	ifLevelReg[2];
	Data8	rfLevelReg[2];
	Data16	fnRet;
	NxtModFormat_t	modFmt;	
#ifndef NXT2004
	Bool	fdcLockState;
	Bool	fdcLastLockState = TRUE;
#endif

	if (TRUE == bTrackFat) {
	
		/* wait for tracking semaphore */
		NxtWaitForCriticalSection(pDCB->pContext, pDCB->hCSNxtFatTrack);

		do {
			NxtSuspendThread(pDCB->pContext, TRACKING_TIMEOUT);

			fnRet = regGetFatLockStatus(pDCB, &fatLockState);
			if (fnRet == NXT_NO_ERROR) {

				/* check for change of lock state */
				if (fatLastLockState & !fatLockState) {
					NxtOnFatChannelLoss(pDCB->pContext);
				}
				else if (!fatLastLockState & fatLockState) {
					NxtOnFatChannelLock(pDCB->pContext);
				}

				fatLastLockState = fatLockState;
			}
			
			/* get modulation format to see if adjacent test needed */
			fnRet = getFatModFormat(pDCB, &modFmt);

			if ((NXT_NO_ERROR == fnRet) && (NXT_8VSB == modFmt)) {

				/* wait for comm semaphore */
				NxtWaitForCriticalSection(pDCB->pContext, pDCB->hCSNxtComm);
			
				/* check IF level for adjacent */
				fnRet = getRegister(pDCB, AGC_SDM1_INPUT_MSB, 2, ifLevelReg);

				/* determine if this is a dual loop system */
				if (NXT_NO_ERROR == fnRet) {
					fnRet = getRegister(pDCB, AGC_CONTROL, 1, &agcControl);
					bDualLoop = (0 != (agcControl & AGC_CONTROL_DUAL));

					if (bDualLoop && (NXT_NO_ERROR == fnRet)) {
						/* check RF level for adjacent */
						fnRet = getRegister(pDCB, AGC_SDM2_INPUT_MSB, 2, rfLevelReg);
					}
				}

				/* release comm semaphore */
				NxtLeaveCriticalSection(pDCB->pContext, pDCB->hCSNxtComm);

				if (NXT_NO_ERROR == fnRet) {

					bFatAdjOn = ((DATA8_TO_DATA16(ifLevelReg)) > pDCB->fatIfAdjOn);
					if (bDualLoop) {
						bFatAdjOn = bFatAdjOn && 
							((DATA8_TO_DATA16(rfLevelReg)) <= pDCB->fatRfAdjOn);
					}

					bFatAdjOff = ((DATA8_TO_DATA16(ifLevelReg)) < pDCB->fatIfAdjOff);
					if (bDualLoop) {
						bFatAdjOff = bFatAdjOff || 
							((DATA8_TO_DATA16(rfLevelReg)) > pDCB->fatRfAdjOff);
					}


					
					if (bFatAdjOn && !pDCB->bFatAdjacent) {
						/* probably an adjacent channel */
						if (pDCB->pFatAgcAdj != (Data16 *)NULL) {

							pDCB->bFatAdjacent = TRUE;			
							regAgcSetup(pDCB, pDCB->pFatAgcAdj);
							pDCB->bFatStartScriptDone = FALSE;
						}
					}
					else if (bFatAdjOff && pDCB->bFatAdjacent) {
						/* no adjacent channel */
						if (pDCB->pFatAgcData != (Data16 *)NULL) {

							pDCB->bFatAdjacent = FALSE;			
							regAgcSetup(pDCB, pDCB->pFatAgcData);
							pDCB->bFatStartScriptDone = FALSE;
						}
					}
				}
			}
			

		} while (!pDCB->bAbortFatFlag);

		pDCB->bAbortFatFlag = FALSE;
			
		/* release tracking semaphore */
		NxtLeaveCriticalSection(pDCB->pContext, pDCB->hCSNxtFatTrack);
	
	} else {/* track FDC */

#ifndef NXT2004
		/* wait for tracking semaphore */
		NxtWaitForCriticalSection(pDCB->pContext, pDCB->hCSNxtFdcTrack);

		do {
			NxtSuspendThread(pDCB->pContext, TRACKING_TIMEOUT);
			/* get FDC lock status */
			fnRet = regGetFdcLockStatus(pDCB, &fdcLockState);
			if (fnRet == NXT_NO_ERROR) {

				/* check for change of lock state */
				if (fdcLastLockState & !fdcLockState) {
					NxtOnFdcChannelLoss(pDCB->pContext);
				}
				else if (!fdcLastLockState & fdcLockState) {
					NxtOnFdcChannelLock(pDCB->pContext);
				}
				fdcLastLockState = fdcLockState;
			}
		}while (!pDCB->bAbortFdcFlag);
		pDCB->bAbortFdcFlag = FALSE;
		
		/* release tracking semaphore */
		NxtLeaveCriticalSection(pDCB->pContext, pDCB->hCSNxtFdcTrack);

#endif
	}

} /* regActiveTracking */


/**********************************************************************
 *
 * regGetFatLockStatus
 *
 * Obtains FAT (in-band) lock status
 *
 * Inputs:
 *	NxtDCB *pDCB - pointer to current DCB -- ASSUMED FOUND!!!
 *
 * Outputs:
 *	Bool *pFatLockStatus -- TRUE if locked, FALSE otherwise
 *
 * Returns:
 *	NXT_NO_ERROR
 *	NXT_ERROR_COMM
 *
 **********************************************************************/
Data16 regGetFatLockStatus(NxtDCB *pDCB,
							 Bool *pFatLockStatus) {

	Data16	retVal = NXT_NO_ERROR;
	Data8	lockRegister;

	/* wait for comm semaphore */
	NxtWaitForCriticalSection(pDCB->pContext, pDCB->hCSNxtComm);
	
	/* check lock flag */
	retVal = getRegister(pDCB, UC_GP_1, 1, &lockRegister);

	if (lockRegister & UC_GP_1_FAT_LOCK) {

		*pFatLockStatus = TRUE;

	} else {

		*pFatLockStatus = FALSE;
	}
	
	/* release comm semaphore */
	NxtLeaveCriticalSection(pDCB->pContext, pDCB->hCSNxtComm);	
	
	return retVal;
} /* regGetFatLockStatus */

#ifndef NXT2004
/**********************************************************************
 *
 * regGetFdcLockStatus
 *
 * Obtains FDC (out of band) lock status
 *
 * Inputs:
 *	NxtDCB *pDCB - pointer to current DCB -- ASSUMED FOUND!!!
 *
 * Outputs:
 *	Bool *pFdcLockStatus -- TRUE if locked, FALSE otherwise
 *
 * Returns:
 *	NXT_NO_ERROR
 *	NXT_ERROR_COMM
 *
 **********************************************************************/
Data16 regGetFdcLockStatus(NxtDCB *pDCB,
							 Bool *pFdcLockStatus) {

	Data16	retVal = NXT_NO_ERROR;
	Data8	lockRegister;
		
	/* wait for comm semaphore */
	NxtWaitForCriticalSection(pDCB->pContext, pDCB->hCSNxtComm);
	
	/* check lock flag */
	retVal = getRegister(pDCB, UC_GP_1, 1, &lockRegister);

	if (lockRegister & UC_GP_1_FDC_LOCK) {

		*pFdcLockStatus = TRUE;

	} else {

		*pFdcLockStatus = FALSE;
	}
	
	/* release comm semaphore */
	NxtLeaveCriticalSection(pDCB->pContext, pDCB->hCSNxtComm);

	return retVal;
} /* regGetFdcLockStatus */
#endif

/**********************************************************************
 *
 * regGetRSErrors
 *
 * Obtains the number of accumulated Reed Solomon errors in FAT channel
 *
 * Inputs:
 *	NxtDCB *pDCB - pointer to current DCB -- ASSUMED FOUND!!!
 *
 * Outputs:
 *	Bool *pErrCount - number of errors accumulated since the last read
 *
 * Returns:
 *	NXT_NO_ERROR
 *	NXT_ERROR_COMM
 *
 **********************************************************************/
Data16 regGetRSErrors(NxtDCB *pDCB,
						   Data8 *pErrCount) {
	Data8	zero = 0;
	Data16	retVal;

	/* wait for comm semaphore */
	NxtWaitForCriticalSection(pDCB->pContext, pDCB->hCSNxtComm);

	retVal = getRegister(pDCB, FEC_RS_ERROR_MIRROR, 1, pErrCount);
	retVal |= setRegister(pDCB, FEC_RS_ERROR_MIRROR, 1, &zero);

	/* release comm semaphore */
	NxtLeaveCriticalSection(pDCB->pContext, pDCB->hCSNxtComm);

	return retVal;
} /* regGetRSErrors */

/**********************************************************************
 *
 * regNotifyFatLockLoss
 *
 * Sends notification on lock or loss in FAT channel
 *
 * Inputs:
 *	NxtDCB *pDCB - pointer to current DCB -- ASSUMED FOUND!!!
 *
 * Returns:
 *	NXT_NO_ERROR
 *	NXT_ERROR_COMM
 *
 **********************************************************************/
Data16 regNotifyFatLockLoss(NxtDCB *pDCB) {

	Data16	retVal;
	Bool	bLocked;

	retVal = regGetFatLockStatus(pDCB, &bLocked);
	if (bLocked) {
		NxtOnFatChannelLock(pDCB->pContext);
	}
	else {
		NxtOnFatChannelLoss(pDCB->pContext);
	}

	return retVal;
} /* regNotifyFatLockLoss */

#ifndef NXT2004
/**********************************************************************
 *
 * regNotifyFdcLockLoss
 *
 * Sends notification on lock or loss in FDC channel
 *
 * Inputs:
 *	NxtDCB *pDCB - pointer to current DCB -- ASSUMED FOUND!!!
 *
 * Returns:
 *	NXT_NO_ERROR
 *	NXT_ERROR_COMM
 *
 **********************************************************************/
Data16 regNotifyFdcLockLoss(NxtDCB *pDCB) {

	Data16	retVal;
	Bool	bLocked;

	retVal = regGetFdcLockStatus(pDCB, &bLocked);
	if (bLocked) {
		NxtOnFdcChannelLock(pDCB->pContext);
	}
	else {
		NxtOnFdcChannelLoss(pDCB->pContext);
	}

	return retVal;
} /* regNotifyFdcLockLoss */
#endif

/**********************************************************************
 *
 * startFatAcquisition
 *
 * Sets Nxt200X for desired acquisition mode and releases Stop bit.
 *
 * Inputs:
 *	NxtDCB *pDCB - pointer to current DCB -- ASSUMED FOUND!!!
 *	Data8 ucGp0	- desired acquisition mode
 *
 * Returns:
 *	NXT_NO_ERROR
 *	NXT_ERR_COMM
 *
 **********************************************************************/
Data16 startFatAcquisition(NxtDCB *pDCB,
			Data8 ucGp0) {
	Data16	retVal;
	Data8	agcAccumulator[2];
	Data8	startRegister;
	Data8	buffer;

	/* wait for comm semaphore */
	NxtWaitForCriticalSection(pDCB->pContext, pDCB->hCSNxtComm);

	/* set acquisition mode */
	retVal = getRegister(pDCB, UC_GP_0, 1, &buffer);
	buffer &= UC_FDC_SYMBOL_MASK;
	ucGp0 |= buffer;
	retVal |= setRegister(pDCB, UC_GP_0, 1, &ucGp0);

	/* stop AGC without resetting */
	retVal |= getRegister(pDCB, AGC_CONTROL, 1, &startRegister);
	startRegister &= ~AGC_CONTROL_GO;
	retVal |= setRegister(pDCB, AGC_CONTROL, 1, &startRegister);

	/* center the AGC accumulator */
	agcAccumulator[0] = 0x80;
	agcAccumulator[1] = 0x00;
	retVal |= setRegister(pDCB, AGC_ACCUMULATOR1_MSB, 2, agcAccumulator);
	retVal |= setRegister(pDCB, AGC_ACCUMULATOR2_MSB, 2, agcAccumulator);

	/* start AGC */
	startRegister |= AGC_CONTROL_GO;
	retVal |= setRegister(pDCB, AGC_CONTROL, 1, &startRegister);

	/* enable inband acquisition */
	retVal |= getRegister(pDCB, UC_ACQ_CONTROL, 1, &startRegister);
	startRegister &= ~UC_ACQ_FAT_STOP;
	if (NXT_NO_ERROR == retVal) {
		retVal = setRegister(pDCB, UC_ACQ_CONTROL, 1, &startRegister);
	}

	/* release comm semaphore */
	NxtLeaveCriticalSection(pDCB->pContext, pDCB->hCSNxtComm);

	return retVal;
} /* startFatAcquisistion */

#ifndef NXT2004
/**********************************************************************
 *
 * startFdcAcquisition
 *
 * Sets Nxt200X for QPSK acquisition mode and releases Stop bit.
 *
 * Inputs:
 *	NxtDCB *pDCB - pointer to current DCB -- ASSUMED FOUND!!!
 *
 * Returns:
 *	NXT_NO_ERROR
 *	NXT_ERR_COMM
 *
 **********************************************************************/
Data16 startFdcAcquisition(NxtDCB *pDCB) {
	Data16	retVal = NXT_NO_ERROR;
	Data8	startRegister;
	Data8	coreControl;
	Data8	coreConTemp;




	if (pDCB->pFdcAgcData != (Data16 *)NULL) {
		/* wait for comm semaphore */
		NxtWaitForCriticalSection(pDCB->pContext, pDCB->hCSNxtComm);
		
		/* reset the external AGC */
		retVal |= getRegister(pDCB, MISC_RESET_CONTROL_2, 1, &coreControl);
		coreConTemp = coreControl;
		coreControl |= SOFT_RESET_FDC_XAGC;
		retVal |= setRegister(pDCB, MISC_RESET_CONTROL_2, 1, &coreControl);
		retVal |= setRegister(pDCB, MISC_RESET_CONTROL_2, 1, &coreConTemp);

		/* release comm semaphore */
		NxtLeaveCriticalSection(pDCB->pContext, pDCB->hCSNxtComm);
		
		/* set FDC AGC Data */
		retVal |= regAgcSetup(pDCB, pDCB->pFdcAgcData);
	}

	/* wait for comm semaphore */
	NxtWaitForCriticalSection(pDCB->pContext, pDCB->hCSNxtComm);
	/* enable FDC acquisition */
	retVal |= getRegister(pDCB, UC_ACQ_CONTROL, 1, &startRegister);
	startRegister &= ~UC_ACQ_FDC_STOP;
	if (NXT_NO_ERROR == retVal) {
		retVal = setRegister(pDCB, UC_ACQ_CONTROL, 1, &startRegister);
	}

	/* release comm semaphore */
	NxtLeaveCriticalSection(pDCB->pContext, pDCB->hCSNxtComm);

	return retVal;
} /* startFdcAcquisistion */
#endif

/**********************************************************************
 *
 * stopFatAcquisition
 *
 * Sets Stop bit to abort in-band acquisition.
 *
 * Inputs:
 *	NxtDCB *pDCB - pointer to current DCB -- ASSUMED FOUND!!!
 *
 * Returns:
 *	NXT_NO_ERROR
 *	NXT_ERR_COMM
 *
 **********************************************************************/
Data16 stopFatAcquisition(NxtDCB *pDCB) {
	Data16	retVal;
	Data8	stopRegister;
	Data8	buffer	= 0;
	Data8	tries	= 0;

	/* wait for comm semaphore */
	NxtWaitForCriticalSection(pDCB->pContext, pDCB->hCSNxtComm);

	/* set stop bit */
	retVal = getRegister(pDCB, UC_ACQ_CONTROL, 1, &stopRegister);
	stopRegister |= UC_ACQ_FAT_STOP;
	retVal |= setRegister(pDCB, UC_ACQ_CONTROL, 1, &stopRegister);

	/* check if micro is in reset */
	if (NXT_NO_ERROR == retVal) {
		retVal = getRegister(pDCB, UC_AGC_PGM_DNLD_CTL, 1, &buffer);
	}

	if (NXT_NO_ERROR == retVal && !(buffer & UC_AGC_PDC_RESET) ) {

		/* wait for stop bit to be acknowledged */
		retVal |= getRegister(pDCB, UC_GP_1, 1, &stopRegister);

		while ( !(stopRegister & UC_GP_1_FAT_PAUSED) && (tries < STOP_ACQ_MAX_TRIES)) {

			/* release comm semaphore */
			NxtLeaveCriticalSection(pDCB->pContext, pDCB->hCSNxtComm);

			NxtSuspendThread(pDCB->pContext, STOP_ACQ_DELAY);

			/* wait for comm semaphore */
			NxtWaitForCriticalSection(pDCB->pContext, pDCB->hCSNxtComm);

			retVal |= getRegister(pDCB, UC_GP_1, 1, &stopRegister);
			tries++;
		}
	} else {
		/* micro is in reset */
		retVal |= NXT_ERR_RESET;
	}

	/* release comm semaphore */
	NxtLeaveCriticalSection(pDCB->pContext, pDCB->hCSNxtComm);

	return retVal;
} /* stopFatAcquisition */

#ifndef NXT2004
/**********************************************************************
 *
 * stopFdcAcquisition
 *
 * Sets Stop bit to abort out-of-band acquisition.
 *
 * Inputs:
 *	NxtDCB *pDCB - pointer to current DCB -- ASSUMED FOUND!!!
 *
 * Returns:
 *	NXT_NO_ERROR
 *	NXT_ERR_COMM
 *
 **********************************************************************/
Data16 stopFdcAcquisition(NxtDCB *pDCB) {
	Data16	retVal;
	Data8	stopRegister;
	Data8	buffer	= 0;
	Data8	tries	= 0;

	/* wait for comm semaphore */
	NxtWaitForCriticalSection(pDCB->pContext, pDCB->hCSNxtComm);

	/* set stop bit */
	retVal = getRegister(pDCB, UC_ACQ_CONTROL, 1, &stopRegister);
	stopRegister |= UC_ACQ_FDC_STOP;
	retVal |= setRegister(pDCB, UC_ACQ_CONTROL, 1, &stopRegister);

	/* check if micro is in reset */
	if (NXT_NO_ERROR == retVal) {
		retVal = getRegister(pDCB, UC_AGC_PGM_DNLD_CTL, 1, &buffer);
	}

	if (NXT_NO_ERROR == retVal && !(buffer & UC_AGC_PDC_RESET) ) {
		/* wait for stop bit to be acknowledged */
		retVal |= getRegister(pDCB, UC_GP_1, 1, &stopRegister);
		while ( !(stopRegister & UC_GP_1_FDC_PAUSED) && (tries < STOP_ACQ_MAX_TRIES)) {

			/* release comm semaphore */
			NxtLeaveCriticalSection(pDCB->pContext, pDCB->hCSNxtComm);

			NxtSuspendThread(pDCB->pContext, STOP_ACQ_DELAY);

			/* wait for comm semaphore */
			NxtWaitForCriticalSection(pDCB->pContext, pDCB->hCSNxtComm);

			retVal |= getRegister(pDCB, UC_GP_1, 1, &stopRegister);
			tries++;
		} /* while */
	} else {
		/* micro is in reset */
		retVal |= NXT_ERR_RESET;
	}

	/* release comm semaphore */
	NxtLeaveCriticalSection(pDCB->pContext, pDCB->hCSNxtComm);

	return retVal;
} /* stopFdcAcquisition */
#endif

/**********************************************************************
 *
 * multibyteRegReadWrite
 *
 * Tests for violation of multibyte register requirement, fixes if needed
 *
 * Inputs:
 *	void *pContext	- void *pContext - NULL, or user-defined ID for multiple NXT200Xs
 *  Data8 devID		- Device ID, required for multibyteTest[]
 *	Data8 nxtAddr	- Nxt200X IIC address, with read/write bit set
 *	Data8 regAddr	- Base register number
 *	Data8 dev0Addr	- IIC Dev 0 address, required for setting UC flags
 *	Data8 byteCount - number of contiguous registers to read
 *	Data8 *pBuffer	- holds write data
 *
 * Outputs:
 *	Data8 *pBuffer	- holds read data if multibyte fixup required
 *	Data16 *pStatus - result of multibyte fixup attempt if required
 *
 * Returns:
 *	TRUE if user's read/write must be modified to satisfy multibyte reqs
 *
 **********************************************************************/
Bool multibyteRegReadWrite( void *pContext,
						    Data8 devID,
						    Data8 nxtAddr,
						    Data8 regAddr,
							Data8 dev0Addr,
							Data8 byteCount, 
							Data8 *pBuffer,
							Data16 *pStatus ) {

	int		i;
	Bool	retVal = FALSE;
	const char	*multibyteTest;	/* copy of multibyteTestDev0 or multibyteTestDec1 
								 * depending upon nxtAddr */
	Data8	lastAddr;		/* last register requested by the user */
	Data8	localAddr;		/* first register to read/write each access in this fn */
	Data8	localCount;		/* number of bytes to read/write each access in this fn */
	Data8	copyIndex;		/* user's buffer index for each access */
	Data8	mbSvcData;		/* temp data for register setup */
	Data8	localBuf[5];	/* big enough for largest multibyte register */
	Data8	localIndex;		/* where to copy from localBuf for a read */
	Data8	copyCount;		/* how many to copy from localBuf for a read */
	Data8	irqData;		/* interrupt mask */
	Data8	tries = 0;		/* polling loop counter for service completion */

	/* assume no error to start */
	*pStatus = NXT_NO_ERROR;

	/* copy multibyteTestDev# to multibyteTest */
	if (devID == 0x00) { /* dev 0 */
		multibyteTest = multibyteTestDev0;
	}
	else if (devID == 0x01) { /* dev 1 */
		multibyteTest = multibyteTestDev1;
	}
	
	lastAddr = regAddr + byteCount -1;
	
	/* simple test for any multibyte access */
	for ( i = regAddr; i <= lastAddr; i++ ) {
		if (multibyteTest[i]) {
			/* non-zero element of array */
			if (iicRegRead(pContext, dev0Addr|0x01, (Data8)UC_AGC_PGM_DNLD_CTL, 
				1, &mbSvcData)) {
				/* iicRegRead failed */
				*pStatus = NXT_ERR_COMM;
				retVal = TRUE; /* force *pStatus to stand */
			} else if (!(mbSvcData & UC_AGC_PDC_RESET)) {/* not in reset */
				retVal = TRUE;
				break;
			}
		}
	} /* for */
	

	if (retVal && (NXT_NO_ERROR == *pStatus)) { /* fix multibyte */

		/* disallow any partial multibyte writes */
		if (!(nxtAddr & 0x01)) { /* write */
			if ((multibyteTest[regAddr] > 1) ||
				( (multibyteTest[lastAddr] != 0) && 
				  (multibyteTest[lastAddr] < multibyteTest[lastAddr+1])) ) {
				*pStatus = NXT_ERR_RANGE;
			}
		} /* write */

		if (NXT_NO_ERROR == *pStatus) { /* handle all accesses */

			for (localAddr = regAddr, copyIndex = 0;
			     localAddr <= lastAddr;
				 localAddr += localCount) { /* for all bytes */

				localCount = 0;
				if ( multibyteTest[localAddr] ) { /* multibyte access */

					/* do we have to back-up to catch the first in multibyte? */
					if (multibyteTest[localAddr] > 1) {
						localIndex = multibyteTest[localAddr] - 1;
						localAddr -= localIndex;
					}
					else {
						localIndex = 0;
					}

					/* find the end of the multibyte access */
					do {
						
						localCount++;
					} while ( multibyteTest[localAddr + localCount - 1] < 
							  multibyteTest[localAddr + localCount] );

					/* use the multibyte service to get the job done */
					mbSvcData = localCount;

					if (devID == 0x01) { /* dev 1 */
						mbSvcData |= UC_DATA_XFER_DEV1;
					}
					if (devID == 0x00) { /*dev 0 */
						mbSvcData |= UC_DATA_XFER_DEV0;
					}
					if (iicRegWrite(pContext, dev0Addr, (Data8)UC_GP_5, 
						1, &localAddr)) {
						*pStatus = NXT_ERR_COMM;
					}
					if (!(nxtAddr & 0x01)) { /* write */
						mbSvcData |= UC_MULTI_WRITE_SVC;
						if (iicRegWrite(pContext, dev0Addr, (Data8)UC_GP_6, 
							localCount, &pBuffer[copyIndex])) {
							*pStatus = NXT_ERR_COMM;
						}
						if (NXT_NO_ERROR == *pStatus) {
							/* set dev#, multibyte data xfer type and byte count */
							if (iicRegWrite(pContext, dev0Addr, (Data8)UC_GP_4, 
								1, &mbSvcData)) {
								*pStatus = NXT_ERR_COMM;
							}
							/* set data xfer service */
							mbSvcData = UC_SERVICES_DATA_XFER;
							if (iicRegWrite(pContext, dev0Addr, 
								(Data8)UC_SERVICES, 1, &mbSvcData)) {
								*pStatus = NXT_ERR_COMM;
							}
						}
						/* poll for data_xfer completion bit only if 
						   IRQ mask is not set */
						if (iicRegRead(pContext, dev0Addr|0x01, 
							(Data8)UC_IRQ_MASK, 1, &irqData)) {
							*pStatus = NXT_ERR_COMM;
						} else 	if (!(irqData & NXT_INT_XFER)) {
							do {

								/* wait for data xfer to complete */
								if (iicRegRead(pContext, dev0Addr|0x01, 
									(Data8)UC_SERVICES, 1, &mbSvcData)) {
									*pStatus = NXT_ERR_COMM;
								}

								if (mbSvcData & UC_SERVICES_DATA_XFER) {
									NxtSuspendThread(pContext, 1);
									tries ++;
								}
							} while ( (tries < XFER_SERVICE_MAX_TRIES) &&
									(NXT_NO_ERROR == *pStatus) && 
									(mbSvcData & UC_SERVICES_DATA_XFER));
							/* check for error conditions */
							if (tries >= XFER_SERVICE_MAX_TRIES) {
								retVal = NXT_ERR_TIMEOUT;
							}
							else if (mbSvcData & UC_SERVICES_DATA_XFER_ERR){
								retVal = NXT_ERR_IIC_XFER;
							}
						}

						copyIndex += localCount;
					} /* write */
					else { /* read */
						if (iicRegWrite(pContext, dev0Addr, (Data8)UC_GP_4, 
							1, &mbSvcData)) {
							/* mbSvcData has dev# and byte count info */
							*pStatus = NXT_ERR_COMM;
						}
						if (NXT_NO_ERROR == *pStatus) {
							mbSvcData = UC_SERVICES_DATA_XFER;
							if (iicRegWrite(pContext, dev0Addr, 
								(Data8)UC_SERVICES, 1, &mbSvcData)) {
								*pStatus = NXT_ERR_COMM;
							}
						}
						
						/* poll for data_xfer completion bit only if 
						   IRQ mask is not set */
						if (iicRegRead(pContext, dev0Addr|0x01, 
							(Data8)UC_IRQ_MASK, 1, &irqData)) {
							*pStatus = NXT_ERR_COMM;
						} else 	if (!(irqData & NXT_INT_XFER)) {
							do {

								/* wait for data xfer to complete */
								if (iicRegRead(pContext, dev0Addr|0x01, 
									(Data8)UC_SERVICES, 1, &mbSvcData)) {
									*pStatus = NXT_ERR_COMM;
								}

								if (mbSvcData & UC_SERVICES_DATA_XFER) {
									NxtSuspendThread(pContext, 1);
									tries ++;
								}
							} while ( (tries < XFER_SERVICE_MAX_TRIES) &&
									(NXT_NO_ERROR == *pStatus) && 
									(mbSvcData & UC_SERVICES_DATA_XFER));
							/* check for error conditions */
							if (tries >= XFER_SERVICE_MAX_TRIES) {
								retVal = NXT_ERR_TIMEOUT;
							} 
							/* check for data_xfer service error */
							else if (mbSvcData & UC_SERVICES_DATA_XFER_ERR){
								retVal = NXT_ERR_IIC_XFER;
							}
						}

						if (iicRegRead(pContext, dev0Addr|0x01, (Data8)UC_GP_6, 
							localCount, localBuf)) {
							*pStatus = NXT_ERR_COMM;
						}
						
						if (lastAddr < localAddr + localCount - 1) {
							copyCount = lastAddr - localAddr + 1 - localIndex;
						}
						else {
							copyCount = localCount - localIndex;
						}
						memcpy(&pBuffer[copyIndex], &localBuf[localIndex], 
							copyCount);

						copyIndex += copyCount;
					} /* read */

				}  /* multibyte access */
				else { /* standard IIC access */

					/* find the end of the standard access */
					do {
						
						localCount++;
					} while ( (copyIndex + localCount < byteCount) && 
							  (!multibyteTest[localAddr + localCount]) );

					if (nxtAddr & 0x01) { /* read */
						if (iicRegRead(pContext, nxtAddr, localAddr, localCount, 
							&pBuffer[copyIndex])) {
							*pStatus = NXT_ERR_COMM;
						}
					} /* read */
					else { /* write */
						if (iicRegWrite(pContext, nxtAddr, localAddr, localCount, 
							&pBuffer[copyIndex])) {
							*pStatus = NXT_ERR_COMM;
						}
					} /* write */

					copyIndex += localCount;
				} /* standard IIC access */
			}  /* for all bytes */
		} /* handle all accesses */
	} /* fix multibyte */

	return retVal;
} /* multibyteRegReadWrite */

/**********************************************************************
 *
 * getRegister
 *
 * Reads a NXT200X register with critical section protection
 *
 * Inputs:
 *	NxtDCB *pDCB - pointer to current DCB -- ASSUMED FOUND!!!
 *	Data16 devReg - device ID and register number
 *	Data8 byteCount - number of contiguous registers to read
 *
 * Outputs:
 *	Data8 *pBuffer - user-supplied data buffer
 *
 * Returns:
 *	NXT_NO_ERROR
 *	NXT_ERR_COMM
 *	NXT_ERR_RANGE
 *
 **********************************************************************/
Data16 getRegister(NxtDCB *pDCB,
			Data16 devReg,
			Data8 byteCount,
			Data8 *pBuffer) {

	Data16	retVal = NXT_NO_ERROR;
	Data16	temp;
	Data8	regAddr;
	Data8	devID;
	Data8	devAddr;

	/* extract devID */
	temp = devReg;
	temp = temp >> 8;
	devID = (Data8) temp;
	if (devID == 0x00) {
		devAddr = pDCB->nxtDev0Addr;
	}
	else {
		if (devID == 0x01) {
			devAddr = pDCB->nxtDev1Addr;
		}
	}

	/* set the LSB for Read, per IIC spec */
	devAddr = devAddr | 0x01;

	/* extract regAddr */
	temp = devReg;
	regAddr = (Data8) (temp & 0xFF);


	/* range check address + count */
	if ((regAddr + byteCount) > 256) {
		/* check for RAM transfer -- range already approved */
		if (regAddr != UC_AGC_DATA_TRANSFER) {
			retVal = NXT_ERR_RANGE;
		}
	}

	if (NXT_NO_ERROR == retVal) {

		/* test for multibyte read requirement */
		if ( (regAddr == UC_AGC_DATA_TRANSFER) ||
			 !multibyteRegReadWrite( pDCB->pContext,
									 devID,
									 devAddr, 
									 regAddr,
									 pDCB->nxtDev0Addr,
									 byteCount, 
									 pBuffer, 
									 &retVal) ) {

			/* IIC read registers */
			if (iicRegRead(pDCB->pContext, devAddr, regAddr, 
				byteCount, pBuffer)) {
				retVal = NXT_ERR_COMM;
			}
		}
	}

	return retVal;
} /* getRegister */


/**********************************************************************
 *
 * setRegister
 *
 * Writes a NXT200X register with critical section protection
 *
 * Inputs:
 *	NxtDCB *pDCB - pointer to current DCB -- ASSUMED FOUND!!!
 *	Data16 devReg - device ID and register number
 *	Data8 byteCount - number of contiguous registers to write
 *	Data8 pBuffer[] - user-supplied data buffer
 *
 * Returns:
 *	NXT_NO_ERROR
 *	NXT_ERR_COMM
 *	NXT_ERR_RANGE
 *
 **********************************************************************/
Data16 setRegister(NxtDCB *pDCB,
			Data16 devReg,
			Data8 byteCount,
			Data8 *pBuffer) {

	Data16	retVal = NXT_NO_ERROR;
	Data16	temp;
	Data8	regAddr;
	Data8	devID;
	Data8	devAddr;

	/* extract devID */
	temp = devReg;
	temp = temp >> 8;
	devID = (Data8) temp;
	if (devID == 0x00) {
		devAddr = pDCB->nxtDev0Addr;
	}
	else {
		if (devID == 0x01) {
			devAddr = pDCB->nxtDev1Addr;
		}
	}

	/* set the LSB for Write, per IIC spec */
	devAddr = devAddr & 0xFE;

	/* extract regAddr */
	temp = devReg;
	regAddr = (Data8) (temp & 0xFF);


	/* range check address + count */
	if ((regAddr + byteCount) > 256) {
		/* check for RAM transfer -- range already approved */
		if (regAddr != UC_AGC_DATA_TRANSFER) {
			retVal = NXT_ERR_RANGE;
		}
	}

	if (NXT_NO_ERROR == retVal) {

		if ((regAddr == UC_AGC_DATA_TRANSFER) ||
			 !multibyteRegReadWrite( pDCB->pContext,
									 devID,
									 devAddr, 
									 regAddr,
									 pDCB->nxtDev0Addr,
									 byteCount, 
									 pBuffer, 
									 &retVal) ) {

			/* IIC write registers */
			if (iicRegWrite(pDCB->pContext, devAddr, regAddr, 
				byteCount, pBuffer)) {
				retVal = NXT_ERR_COMM;
			}
		}
	}

	return retVal;
} /* setRegister */

/**********************************************************************
 *
 * resetMicro
 *
 * Places and holds the microcontroller in a reset state
 *
 * Inputs:
 *	NxtDCB *pDCB - pointer to current DCB -- ASSUMED FOUND!!!
 *
 * Returns:
 *	NXT_NO_ERROR
 *	NXT_ERR_COMM
 *
 **********************************************************************/
Data16 resetMicro(NxtDCB *pDCB) {

	Data16	retVal;
	Data8	ctlReg;

	/* wait for comm semaphore */
	NxtWaitForCriticalSection(pDCB->pContext, pDCB->hCSNxtComm);

	/* get UC_AGC_PGM_DNLD_CTL register */
	retVal = getRegister(pDCB, UC_AGC_PGM_DNLD_CTL, 1, &ctlReg);

	if (retVal == NXT_NO_ERROR) {
		/* set the micro in reset */
		ctlReg |= UC_AGC_PDC_RESET;
		retVal = setRegister(pDCB, UC_AGC_PGM_DNLD_CTL, 1, &ctlReg);
	}

	/* release comm semaphore */
	NxtLeaveCriticalSection(pDCB->pContext, pDCB->hCSNxtComm);

	return retVal;
} /* resetMicro */


/**********************************************************************
 *
 * unresetMicro
 *
 * Releases the microcontroller from the reset state
 *
 * Inputs:
 *	NxtDCB *pDCB - pointer to current DCB -- ASSUMED FOUND!!!
 *
 * Returns:
 *	NXT_NO_ERROR
 *	NXT_ERR_COMM
 *
 **********************************************************************/
Data16 unresetMicro(NxtDCB *pDCB) {

	Data16	retVal;
	Data8	ctlReg;

	/* wait for comm semaphore */
	NxtWaitForCriticalSection(pDCB->pContext, pDCB->hCSNxtComm);

	/* get UC_AGC_PGM_DNLD_CTL regster */
	retVal = getRegister(pDCB, UC_AGC_PGM_DNLD_CTL, 1, &ctlReg);

	if (retVal == NXT_NO_ERROR) {
		/* clear the reset bit */
		ctlReg &= ~UC_AGC_PDC_RESET;
		retVal = setRegister(pDCB, UC_AGC_PGM_DNLD_CTL, 1, &ctlReg);
	}

	/* release comm semaphore */
	NxtLeaveCriticalSection(pDCB->pContext, pDCB->hCSNxtComm);

	retVal |= NxtennaSetConfig(pDCB->pContext, pDCB->nxtennaMap,
					pDCB->nxtennaGain, pDCB->nxtennaPolBit);

	return retVal;
} /* unresetMicro */

/**********************************************************************
 *
 * getFatSQI
 *
 * Obtains the cluster variance, a measure of signal quality
 *
 * Inputs:
 *	NxtDCB *pDCB - pointer to current DCB -- ASSUMED FOUND!!!
 *
 * Outputs:
 *	Data16 *pFatSQI - register data copied to user-provided buffer
 *
 * Returns:
 *	NXT_NO_ERROR
 *	NXT_ERR_COMM
 *
 **********************************************************************/
Data16 getFatSQI(NxtDCB *pDCB,
			Data16 *pFatSQI) {

	Data16	retVal;
	Data8	buffer[2];

	/* wait for comm semaphore */
	NxtWaitForCriticalSection(pDCB->pContext, pDCB->hCSNxtComm);

	/* setup to read cluster variance */
	retVal = getRegister(pDCB, EQ_CL_CONTROL, 1, buffer);
	if (retVal == NXT_NO_ERROR) {
		buffer[0] &= ~CL_STAT_PTR_MASK;
		buffer[0] |= CL_STAT_CV;
		retVal = setRegister(pDCB, EQ_CL_CONTROL, 1, buffer);
	}

	/* read 16 bit cluster variance */
	if (retVal == NXT_NO_ERROR) {
		retVal = getRegister(pDCB, EQ_CL_STAT_3, 2, buffer);
	}

	/* release comm semaphore */
	NxtLeaveCriticalSection(pDCB->pContext, pDCB->hCSNxtComm);

	if (retVal == NXT_NO_ERROR) {
		*pFatSQI = (Data16)(0x7FFF) - (DATA8_TO_DATA16(buffer));
	}

	return retVal;
} /* getFatSQI */

#ifndef NXT2004
/**********************************************************************
 *
 * getFdcSQI
 *
 * Obtains the cluster variance, a measure of signal quality
 *
 * Inputs:
 *	NxtDCB *pDCB - pointer to current DCB -- ASSUMED FOUND!!!
 *
 * Outputs:
 *	double *pFdcSQI - Cluster Variance calculated value
 *
 * Returns:
 *	NXT_NO_ERROR
 *	NXT_ERR_COMM
 *
 **********************************************************************/
Data16 getFdcSQI(NxtDCB *pDCB,
			double *pFdcSQI) {
	Data16	retVal;
	Data8	buffer[4];
	Data8	eqControl;
	Data32	winSize = 1;
	Data32	cvAcc;

	
	NxtWaitForCriticalSection(pDCB->pContext, pDCB->hCSNxtComm);
	
	/* read the 30 bit CV */
	retVal = getRegister(pDCB, FDC_CV_ESTIMATE_3, 4, buffer);

	/* read window size */
	retVal = getRegister(pDCB, FDC_EQ_CONTROL, 1, &eqControl);
	
	NxtLeaveCriticalSection(pDCB->pContext, pDCB->hCSNxtComm);
	
	if (retVal == NXT_NO_ERROR) {
		
		/* convert data8 to data32 */
		/* clear the left 2 bits */
		cvAcc = ((DATA8_TO_DATA32(buffer)) << 2) >> 2;
		
		/* multiply by 2^-18 */
		cvAcc = (Data32) ( cvAcc / (double) 0x40000 );

		eqControl = eqControl & FDC_CVEST_WIN;
		switch (eqControl) {
			case FDC_CVEST_WIN_256:
				winSize = 256; break;
			case FDC_CVEST_WIN_512:
				winSize = 512; break;
			case FDC_CVEST_WIN_2048:
				winSize = 2048; break;	
			case FDC_CVEST_WIN_1024:
			default:
				winSize = 1024; break;
		}

		*pFdcSQI =  (double) cvAcc / (2.0 * winSize);
	}
	return retVal;
}/* getFdcSQI */
#endif

/**********************************************************************
 *
 * getFatPilotOffset
 *
 * Register level NxtGetFatPilotOffset.
 *
 * Inputs:
 *	NxtDCB *pDCB - pointer to current DCB -- ASSUMED FOUND!!!
 *
 * Outputs:
 *	Data32 *pFatPilotOffset - frequency offset, Hz
 *
 * Returns:
 *	NXT_NO_ERROR
 *	NXT_ERR_RESET
 *	NXT_ERR_INIT
 *	NXT_ERR_COMM
 *
 * Notes:
 *	This function assumes that frame lock is achieved.
 *
 **********************************************************************/
Data16 getFatPilotOffset(NxtDCB *pDCB,
		   Data32 *pFatPilotOffset) {

	Data16	retVal;
	Data8	buffer[4];
	Data32	baudRate;
	Data32	Pilot_LF_ACC;

	NxtModFormat_t modFmt;

	/* pilot offset calculation depends on modulation format */
	retVal = getFatModFormat(pDCB, &modFmt);
	if (retVal == NXT_NO_ERROR) {
		switch (modFmt) {
		case NXT_64QAM:
			baudRate =  BAUD_RATE_QAM64  * 2;
			break;
		case NXT_256QAM:
			baudRate =  BAUD_RATE_QAM256 * 2;
			break;
		case NXT_8VSB:
		default:
			baudRate =  (Data32)BAUD_RATE_VSB;
			break;
		}

		/* wait for comm semaphore */
		NxtWaitForCriticalSection(pDCB->pContext, pDCB->hCSNxtComm);

		/* read 32 bit pilot offset */
		retVal = getRegister(pDCB, FE_PILOT_FREQ_OFFSET_3, 4, buffer);

		/* release comm semaphore */
		NxtLeaveCriticalSection(pDCB->pContext, pDCB->hCSNxtComm);

		Pilot_LF_ACC = ( (  ((Data32)buffer[0] << 24)
						  | ((Data32)buffer[1] << 16)
						  | ((Data32)buffer[2] <<  8)
						  | ((Data32)buffer[3]      )
						 ) << 4		/* clear bits [d31:d28] */
					   ) >> 4;		/* set the values of bits [d31:d28] 
									to the value of [d27] */

		*pFatPilotOffset =  (Data32)( - ((double)Pilot_LF_ACC / 536870912.0) /* double(1 << 29) */
		                         * 0.159154943092						/* (1.0 / (2.0 * PI)) */
								 * baudRate
							   );
	}

	return retVal;
} /* getFatPilotOffset */


/**********************************************************************
 *
 * getFatCarrierOffset
 *
 * Obtains the value, in Hz, of IF carrier frequency offset from nominal.
 *
 * Inputs:
 *	NxtDCB *pDCB - pointer to current DCB -- ASSUMED FOUND!!!
 *
 * Outputs:
 *	Data32 *pCarrierOffset - carrier offset, Hz
 *
 * Returns:
 *	NXT_NO_ERROR
 *	NXT_ERR_COMM
 *
 * Notes:
 *	This function assumes that frame lock is achieved.
 *
 **********************************************************************/
Data16 getFatCarrierOffset(NxtDCB *pDCB,
			 Data32 *pCarrierOffset) {

	Data16	retVal;
	NxtModFormat_t	modFmt;
	Data8	buffer[4];
	Data32	baudRate;
	Data32	cl_offset;

	/* carrier offset calculation depends on modulation format */
	retVal = getFatModFormat(pDCB, &modFmt);
	if (retVal == NXT_NO_ERROR) {
		switch (modFmt) {
		case NXT_64QAM:
			baudRate =  BAUD_RATE_QAM64;
			break;
		case NXT_256QAM:
			baudRate =  BAUD_RATE_QAM256;
			break;
		case NXT_8VSB:
		default:
			baudRate = (Data32)( -(BAUD_RATE_VSB) );
			break;
		}

		/* wait for comm semaphore */
		NxtWaitForCriticalSection(pDCB->pContext, pDCB->hCSNxtComm);

		/* setup to read carrier offset (loop filter) */
		retVal = getRegister(pDCB, EQ_CL_CONTROL, 1, buffer);
		buffer[0] = (buffer[0] & ~CL_STAT_PTR_MASK) | CL_STAT_LF;
		if (retVal == NXT_NO_ERROR) {
			retVal = setRegister(pDCB, EQ_CL_CONTROL, 1, buffer);
		}

		/* read 32 bit timing offset */
		if (retVal == NXT_NO_ERROR) {
			retVal = getRegister(pDCB, EQ_CL_STAT_3, 4, buffer);
		}

		/* release comm semaphore */
		NxtLeaveCriticalSection(pDCB->pContext, pDCB->hCSNxtComm);

		cl_offset = ((Data32)buffer[0] <<24)
				  | ((Data32)buffer[1] <<16)
				  | ((Data32)buffer[2] << 8)
				  | ((Data32)buffer[3]     );


		*pCarrierOffset = (Data32)(  ((double)cl_offset / 268435456.0) /* double(1 << 28) */
									* 0.159154943092                 /* (1.0 / (2.0 * PI)) */
									* baudRate
								);
	}

	return retVal;
} /* getFatCarrierOffset */

#ifndef NXT2004
/**********************************************************************
 *
 * getFdcCarrierOffset
 *
 * Obtains the value, in Hz, of FDC IF carrier frequency offset from nominal.
 *
 * Inputs:
 *	NxtDCB *pDCB - pointer to current DCB -- ASSUMED FOUND!!!
 *
 * Outputs:
 *	Data32 *pCarrierOffset - carrier offset, Hz
 *
 * Returns:
 *	NXT_NO_ERROR
 *	NXT_ERR_COMM
 *
 * Notes:
 *	This function assumes that frame lock is achieved.
 *
 **********************************************************************/
Data16 getFdcCarrierOffset(NxtDCB *pDCB,
			 Data32 *pCarrierOffset) {
	Data16	retVal;
	Data8	buffer[4];
	Data32	baudRate; 
	Data32	cl_offset;

	/* read symbol rate from DCB */
	switch(pDCB->symbolRate) {
		case NXT_FDC_SYMBOL_RATE_772:
			baudRate = BAUD_RATE_FDC_772;
		case NXT_FDC_SYMBOL_RATE_1544:
			baudRate = BAUD_RATE_FDC_1544;
		case NXT_FDC_SYMBOL_RATE_1024:
		default:
			baudRate = BAUD_RATE_FDC_1024;
	}/* pDCB->symbolRate */

	/* wait for comm semaphore */
	NxtWaitForCriticalSection(pDCB->pContext, pDCB->hCSNxtComm);

	/* read 32 bit timing offset */
	retVal = getRegister(pDCB, FDC_CARRIER_OFFSET_3, 4, buffer);

	/* release comm semaphore */
	NxtLeaveCriticalSection(pDCB->pContext, pDCB->hCSNxtComm);

	/* carrier offset calculation */
	if (retVal == NXT_NO_ERROR) {
		cl_offset = ((Data32)buffer[0] <<24)
				  | ((Data32)buffer[1] <<16)
				  | ((Data32)buffer[2] << 8)
				  | ((Data32)buffer[3]     );


		*pCarrierOffset = (Data32)(  ((double)cl_offset / 8589934592.0) /* double(1 << 33) */
									* 0.159154943092                 /* (1.0 / (2.0 * PI)) */
									* baudRate
								);
	}

	return retVal;

}/* getFdcCarrierOffset */
#endif

/**********************************************************************
 *
 * getFatTimingOffset
 *
 * Obtains the timing baud rate offset in FAT channel
 *
 * Inputs:
 *	NxtDCB *pDCB - pointer to current DCB -- ASSUMED FOUND!!!
 *
 * Outputs:
 *	Data32 *pFatTimingOffset - accumulation of timing recovery loop
 *
 * Returns:
 *	NXT_NO_ERROR
 *	NXT_ERR_COMM
 *
 **********************************************************************/
Data16 getFatTimingOffset(NxtDCB*  pDCB,
			Data32*  pFatTimingOffset) 
{
	Data8	baudBuffer[4];
	Data8	rateBuffer[5];
	Data8	pilot		= 0;
	Data8	timingShift = 0;
	Data32	timingAcc   = 0;
	Data32	divisor		= 1;
	Data32	baudRate	= 0;
	Data64	rateNom     = 0;
	Data16	retVal;
	NxtModFormat_t	modFmt;

	/* timing offset calculation depends on modulation format */
	retVal = getFatModFormat(pDCB, &modFmt);
	if (NXT_NO_ERROR == retVal) 
	{
		switch (modFmt) 
		{
		case NXT_64QAM:	  
			divisor  = 2; 
			baudRate = BAUD_RATE_QAM64  * 2;  
			break;
		case NXT_256QAM:  
			divisor  = 2; 
			baudRate = BAUD_RATE_QAM256 * 2;  
			break;
		case NXT_8VSB:
		default:		  
			divisor  = 1; 
			baudRate = (Data32)(BAUD_RATE_VSB);
		};
	}
	if (NXT_NO_ERROR == retVal) 
	{
		/* wait for comm semaphore */
		NxtWaitForCriticalSection(pDCB->pContext, pDCB->hCSNxtComm);

		/* read 32 bit timing offset */
		retVal  = getRegister(pDCB, FE_TIMING_BAUD_RATE_OFFSET_3, 4, baudBuffer);

		/* read 38 bit sampling frequency ratio */
		retVal |= getRegister(pDCB, FE_TIMING_RATE_NOM_4, 5, rateBuffer);

		/* determine timing shift */
		retVal |= getRegister(pDCB, FE_TIMING_PILOT_CONTROL, 1, &pilot);

		/* release comm semaphore */
		NxtLeaveCriticalSection(pDCB->pContext, pDCB->hCSNxtComm);

	}
	if (NXT_NO_ERROR == retVal) 
	{
		timingAcc = ( (	 ((Data32)baudBuffer[0] << 24)
					   | ((Data32)baudBuffer[1] << 16)
					   | ((Data32)baudBuffer[2] <<  8)
					   |  (Data32)baudBuffer[3]
					  ) << 4
					) >> 4;


		rateNom =     (((Data64)rateBuffer[0] & 0x3F) << 32) 
					| (((Data64)rateBuffer[1] & 0xFF) << 24) 
					| (((Data64)rateBuffer[2] & 0xFF) << 16)
					| (((Data64)rateBuffer[3] & 0xFF) << 8) 
					|  ((Data64)rateBuffer[4] & 0xFF);

		switch (pilot & FE_TIMING_FEEDBACK_GAIN) 
		{
		case FE_TIMING_FEEDBACK_1X:	 timingShift = 4;  break;
		case FE_TIMING_FEEDBACK_4X:	 timingShift = 2;  break;
		case FE_TIMING_FEEDBACK_16D: timingShift = 8;  break;
		case FE_TIMING_FEEDBACK_4D:	 timingShift = 6;  break;
		default: break;
		}


		*pFatTimingOffset = (Data32)( ( (    (	(MASTER_CLOCK / 4.0) * 
												(double)0x40000000 ) 
									  / ( (double)rateNom + 
										  (timingAcc >> timingShift) ) 
								   ) - baudRate
								 ) / divisor
							   );
	}
	else
	{
		*pFatTimingOffset = (Data32)(-1);
	}

	return retVal;
} /* getFatTimingOffset */


#ifndef NXT2004
/**********************************************************************
 *
 * getFdcTimingOffset
 *
 * Obtains the timing baud rate offset in FDC channel
 *
 * Inputs:
 *	NxtDCB *pDCB - pointer to current DCB -- ASSUMED FOUND!!!
 *
 * Outputs:
 *	Data32 *pFdcTimingOffset - accumulation of timing recovery loop
 *
 * Returns:
 *	NXT_NO_ERROR
 *	NXT_ERR_COMM
 *
 **********************************************************************/
Data16 getFdcTimingOffset(NxtDCB*  pDCB,
			Data32*  pFdcTimingOffset) 
{
	Data8	errBuffer[3];
	Data8	rateBuffer[5];
	Data32	timingAcc   = 0;
	Data32	baudRate;
	Data16	rateNom     = 0;
	Data16	retVal;
	
	/* get the FDC symbol rate */
	baudRate = pDCB->symbolRate;
	
	/* wait for comm semaphore */
	NxtWaitForCriticalSection(pDCB->pContext, pDCB->hCSNxtComm);

	/* read 24 bit error */
	retVal  = getRegister(pDCB, FDC_TIMING_OFFSET_2, 3, errBuffer);

	/* read 16 bit NCO */
	retVal |= getRegister(pDCB, FDC_NCO_PERIOD_MSB, 2, rateBuffer);

	/* release comm semaphore */
	NxtLeaveCriticalSection(pDCB->pContext, pDCB->hCSNxtComm);


	if (NXT_NO_ERROR == retVal) 
	{
		timingAcc = (	 ((Data32)errBuffer[0] << 16)
					   | ((Data32)errBuffer[1] <<  8)
					   | ((Data32)errBuffer[2])
					);


		rateNom =   (	  ((Data16)rateBuffer[0] << 8) 
						| ((Data16)rateBuffer[1])
					);


		*pFdcTimingOffset = (Data32)( ( (MASTER_CLOCK / 8.0)  
									/ (2.0 * ((double)rateNom /(double)0x2000 + 
									           timingAcc/(double)0x40000000) ) 
								   ) - baudRate
							   );
	}
	else
	{
		*pFdcTimingOffset = (Data32)(-1);
	}

	return retVal;
} /* getFdcTimingOffset */
#endif

/**********************************************************************
 *
 * getFatModFormat
 *
 * Obtains the current in-band modulation format used by the NXT200X
 *
 * Inputs:
 *	NxtDCB *pDCB - pointer to current DCB -- ASSUMED FOUND!!!
 *
 * Outputs:
 *	NxtModFormat_t *pFatModFormat - current modulation format.
 *
 * Returns:
 *	NXT_NO_ERROR
 *	NXT_ERR_INIT
 *	NXT_ERR_COMM
 *
 * Notes:
 *	Modulation format setting is valid regardless of the state of the
 *	NXT200X ASIC -- this is not an indication of the type of signal
 *	being received, but of the type that is (or would be) being attempted
 *	for reception by the ASIC.
 **********************************************************************/
Data16 getFatModFormat(NxtDCB *pDCB,
						 NxtModFormat_t *pFatModFormat) {

	Data16	retVal = NXT_NO_ERROR;
	Data8	buffer;

	/* wait for comm semaphore */
	NxtWaitForCriticalSection(pDCB->pContext, pDCB->hCSNxtComm);

	retVal = getRegister(pDCB, MISC_MOD_CTRL_OUT_FMT, 1, &buffer);

	/* release comm semaphore */
	NxtLeaveCriticalSection(pDCB->pContext, pDCB->hCSNxtComm);

	switch (buffer & MOD_FMT_MASK) {
	case MOD_FMT_8VSB:
		*pFatModFormat = NXT_8VSB;
		break;
	case MOD_FMT_16VSB:
		*pFatModFormat = NXT_16VSB;
		break;
	case MOD_FMT_64QAM:
		*pFatModFormat = NXT_64QAM;
		break;
	case MOD_FMT_256QAM:
		*pFatModFormat = NXT_256QAM;
		break;
	}

	return retVal;
} /* getFatModFormat */


/**********************************************************************
 *
 * regCoreControl
 *
 * Register level core control.
 *
 * Inputs:
 *	NxtDCB *pDCB - pointer to current DCB -- ASSUMED FOUND!!!
 *	NxtCoreControl_t coreControl - picks core section to control
 *
 * Returns:
 *	Data16
 *
 * Notes:
 *	FAT and FDC acquisitions are stopped in this API.  Use NxtStart()
 *	to restart acquisition after any core control.
 *
 **********************************************************************/
Data16 regCoreControl(NxtDCB *pDCB,
						   NxtCoreControl_t coreControl) {

	Data16	retVal = NXT_NO_ERROR;
	Data8	buffer;
	Data8	readValue;
	Data8	writeValue;
	Data8	fatSdmConfig;	/* FAT saved polarities, etc. */
	Data8	mpegFmt0;		/* MPEG settings */
	Data8	mpegFmt1;		/* more MPEG settings */
	Data8	mpegMode[2];
	Data8	eqTestMux;
	Data8	zero = 0;
	NxtModFormat_t	modFmt;
#ifndef NXT2004
	Data8	fdcSdmConfig;	/* FDC AGC polarity */
#endif


	/* stop any running threads, stop Acquisitions */
	if(coreControl & 0x10)
		regStop(pDCB, NXT_STOP_FAT);
#ifndef NXT2004
	if(coreControl & 0x20)
		regStop(pDCB, NXT_STOP_FDC);
#endif

	/* wait for comm semaphore */
	NxtWaitForCriticalSection(pDCB->pContext, pDCB->hCSNxtComm);

	/* before resetting save current settings */
	retVal |= getRegister(pDCB, MISC_MOD_CTRL_OUT_FMT, 1, &mpegFmt0);
	retVal |= getRegister(pDCB, SMOOTHER_CONTROL, 1, &mpegFmt1);
	retVal |= getRegister(pDCB, AGC_SDM_CONFIGURE, 1, &fatSdmConfig);
	retVal |= getRegister(pDCB, EQ_TEST_MUX_SELECT, 1, &eqTestMux);
#ifndef NXT2004
	retVal |= getRegister(pDCB, FDC_AGC_MODE, 1, &fdcSdmConfig);
#endif
	/* readValue, writeValue in MISC_POWER_CONTROL and MISC_RESET_CONTROL 
	depends on coreControl */
	switch (coreControl) {
		case CORE_POWER_DOWN_ALL:
			/* soft reset required before power down of digital core */
/*			writeValue = SOFT_RESET_ALL;
			retVal |= setRegister(pDCB, MISC_RESET_CONTROL, 1, &writeValue);
#ifndef NXT2004
			retVal |= getRegister(pDCB, MISC_RESET_CONTROL_2, 1, &buffer);
			buffer |= SOFT_RESET_2_ALL;
			retVal |= setRegister(pDCB, MISC_RESET_CONTROL_2, 1, &buffer);
#endif
*/			writeValue = MISC_POWER_DN_FAT;
#ifndef NXT2004
			writeValue |= MISC_POWER_DN_FDC;
#endif
#ifdef NXT2005
			writeValue |= MISC_POWER_DN_RDC;
#endif

			retVal = getRegister(pDCB, MISC_POWER_CONTROL, 1, &readValue);
			writeValue |= readValue;
			retVal |= setRegister(pDCB, MISC_POWER_CONTROL, 1, &writeValue);

			/* power down analog section */
			retVal |= getRegister(pDCB, MISC_ELARA_SERIAL_CMD_2, 1, &readValue);
			writeValue = readValue | MISC_ELARA_PDWN_FS;
			writeValue |= MISC_ELARA_PDWN_ADC1;
			writeValue |= MISC_ELARA_PDWN_ADC0;
			retVal |= setRegister(pDCB, MISC_ELARA_SERIAL_CMD_2, 1, &writeValue);
			/* trigger data transfer to Elara */
			retVal |= getRegister(pDCB, MISC_ELARA_SERIAL_CMD_0, 1, &readValue);
			writeValue = readValue;
			retVal |= setRegister(pDCB, MISC_ELARA_SERIAL_CMD_0, 1, &writeValue);
			break;

		case CORE_POWER_UP_ALL:
			/* soft reset required before power up of digital core */
/*			buffer = SOFT_RESET_ALL;
			retVal |= setRegister(pDCB, MISC_RESET_CONTROL, 1, &buffer);
#ifndef NXT2004
			retVal |= getRegister(pDCB, MISC_RESET_CONTROL_2, 1, &buffer);
			buffer |= SOFT_RESET_2_ALL;
			retVal |= setRegister(pDCB, MISC_RESET_CONTROL_2, 1, &buffer);
#endif
*/			retVal |= getRegister(pDCB, MISC_POWER_CONTROL, 1, &buffer);
			buffer &= ~MISC_POWER_CTL_MASK;
#ifndef NXT2004
			buffer |= POD_OUT_ENABLE;
#endif
			retVal |= setRegister(pDCB, MISC_POWER_CONTROL, 1, &buffer);

			/* power up analog section */
			retVal |= getRegister(pDCB, MISC_ELARA_SERIAL_CMD_2, 1, &readValue);
			writeValue = readValue &~ MISC_ELARA_PDWN_FS;
#ifndef NXT2004
			writeValue &= ~MISC_ELARA_PDWN_ADC1;
#endif
			writeValue &= ~MISC_ELARA_PDWN_ADC0;
			retVal |= setRegister(pDCB, MISC_ELARA_SERIAL_CMD_2, 1, &writeValue);
			/* trigger data transfer to Elara */
			retVal |= getRegister(pDCB, MISC_ELARA_SERIAL_CMD_0, 1, &readValue);
			writeValue = readValue;
			retVal |= setRegister(pDCB, MISC_ELARA_SERIAL_CMD_0, 1, &writeValue);
			
			/* soft unreset required after power up */
/*			retVal |= setRegister(pDCB, MISC_RESET_CONTROL, 1, &zero);
#ifndef NXT2004
			retVal |= getRegister(pDCB, MISC_RESET_CONTROL_2, 1, &buffer);
			buffer &= ~SOFT_RESET_2_ALL;
			retVal |= setRegister(pDCB, MISC_RESET_CONTROL_2, 1, &buffer);
#endif
*/			break;

		case CORE_POWER_DOWN_FAT:

			retVal |= getRegister(pDCB, MISC_POWER_CONTROL, 1, &buffer);
			buffer |= MISC_POWER_DN_FAT;
			retVal |= setRegister(pDCB, MISC_POWER_CONTROL, 1, &buffer);

			/* power down analog section */
			retVal |= getRegister(pDCB, MISC_ELARA_SERIAL_CMD_2, 1, &buffer);
			buffer |= MISC_ELARA_PDWN_ADC0;
			retVal |= setRegister(pDCB, MISC_ELARA_SERIAL_CMD_2, 1, &buffer);
			/* trigger data transfer to Elara */
			retVal |= getRegister(pDCB, MISC_ELARA_SERIAL_CMD_0, 1, &readValue);
			writeValue = readValue;
			retVal |= setRegister(pDCB, MISC_ELARA_SERIAL_CMD_0, 1, &writeValue);
			break;

		case CORE_POWER_UP_FAT:

			retVal = getRegister(pDCB, MISC_POWER_CONTROL, 1, &readValue);
			writeValue = readValue & ~MISC_POWER_DN_FAT;
			retVal |= setRegister(pDCB, MISC_POWER_CONTROL, 1, &writeValue);
			
			/* power up analog section */
			retVal |= getRegister(pDCB, MISC_ELARA_SERIAL_CMD_2, 1, &readValue);
			writeValue = readValue & ~MISC_ELARA_PDWN_ADC0;
			retVal |= setRegister(pDCB, MISC_ELARA_SERIAL_CMD_2, 1, &writeValue);
			/* trigger bytes transfer to Elara */
			retVal |= getRegister(pDCB, MISC_ELARA_SERIAL_CMD_0, 1, &readValue);
			writeValue = readValue;
			retVal |= setRegister(pDCB, MISC_ELARA_SERIAL_CMD_0, 1, &writeValue);

			break;

#ifndef NXT2004
		case CORE_POWER_DOWN_FDC:

			retVal |= getRegister(pDCB, MISC_POWER_CONTROL, 1, &buffer);
			buffer |= MISC_POWER_DN_FDC;
			retVal |= setRegister(pDCB, MISC_POWER_CONTROL, 1, &buffer);

			/* power down analog section */
			retVal |= getRegister(pDCB, MISC_ELARA_SERIAL_CMD_2, 1, &buffer);
			buffer |= MISC_ELARA_PDWN_FS;
			buffer |= MISC_ELARA_PDWN_ADC1;
			retVal |= setRegister(pDCB, MISC_ELARA_SERIAL_CMD_2, 1, &buffer);
			/* trigger bytes transfer to Elara */
			retVal |= getRegister(pDCB, MISC_ELARA_SERIAL_CMD_0, 1, &readValue);
			writeValue = readValue;
			retVal |= setRegister(pDCB, MISC_ELARA_SERIAL_CMD_0, 1, &writeValue);
			break;

		case CORE_POWER_UP_FDC:

			retVal |= getRegister(pDCB, MISC_POWER_CONTROL, 1, &buffer);
			buffer &= ~MISC_POWER_DN_FDC;
			buffer |= POD_OUT_ENABLE;
			retVal |= setRegister(pDCB, MISC_POWER_CONTROL, 1, &buffer);

			/* power up analog section */
			retVal |= getRegister(pDCB, MISC_ELARA_SERIAL_CMD_2, 1, &readValue);
			writeValue = readValue &~ MISC_ELARA_PDWN_FS;
			writeValue &= ~MISC_ELARA_PDWN_ADC1;
			retVal |= setRegister(pDCB, MISC_ELARA_SERIAL_CMD_2, 1, &writeValue);
			/* trigger bytes transfer to Elara */
			retVal |= getRegister(pDCB, MISC_ELARA_SERIAL_CMD_0, 1, &readValue);
			writeValue = readValue;
			retVal |= setRegister(pDCB, MISC_ELARA_SERIAL_CMD_0, 1, &writeValue);

			retVal |= getRegister(pDCB, MISC_RESET_CONTROL, 1, &buffer);
			/* make sure BERT is unreset */
			buffer &= ~SOFT_RESET_BERT;
			/* make sure ADC FIFOs are unreset */
			buffer &= ~SOFT_RESET_ADC_FIFO;
			retVal |= setRegister(pDCB, MISC_RESET_CONTROL, 1, &buffer);
			break;
#endif

#ifdef NXT2005
		case CORE_POWER_DOWN_RDC:
			/* soft reset required before power down */
			/*retVal |= getRegister(pDCB, MISC_RESET_CONTROL, 1, &buffer);
			buffer |= SOFT_RESET_RDC;
			retVal |= setRegister(pDCB, MISC_RESET_CONTROL, 1, &buffer);*/

			retVal |= getRegister(pDCB, MISC_POWER_CONTROL, 1, &buffer);
			buffer |= MISC_POWER_DN_RDC;
			retVal |= setRegister(pDCB, MISC_POWER_CONTROL, 1, &buffer);
			break;

		case CORE_POWER_UP_RDC:
			/* soft reset required before power up */
			/*retVal |= getRegister(pDCB, MISC_RESET_CONTROL, 1, &buffer);
			buffer |= SOFT_RESET_RDC;
			retVal |= setRegister(pDCB, MISC_RESET_CONTROL, 1, &buffer);*/

			retVal |= getRegister(pDCB, MISC_POWER_CONTROL, 1, &buffer);
			buffer &= ~MISC_POWER_DN_RDC;
			buffer |= PGA_OUT_ENABLE;
			buffer |= POD_OUT_ENABLE;
			retVal |= setRegister(pDCB, MISC_POWER_CONTROL, 1, &buffer);
			
			/* un-do soft-reset flag */
			/*retVal |= getRegister(pDCB, MISC_RESET_CONTROL, 1, &buffer);
			buffer &= ~SOFT_RESET_RDC;
			retVal |= setRegister(pDCB, MISC_RESET_CONTROL, 1, &buffer);*/
			break;
#endif

		case CORE_SOFT_RESET_ALL:
			writeValue = SOFT_RESET_ALL;
			retVal |= setRegister(pDCB, MISC_RESET_CONTROL, 1, &writeValue);
			/* un-do all soft-reset flags */
			retVal |= setRegister(pDCB, MISC_RESET_CONTROL, 1, &zero);

#ifndef NXT2004
			retVal |= getRegister(pDCB, MISC_RESET_CONTROL_2, 1, &buffer);
			buffer |= SOFT_RESET_2_ALL;
			retVal |= setRegister(pDCB, MISC_RESET_CONTROL_2, 1, &buffer);
			/* un-do all soft-reset flags */
			retVal |= getRegister(pDCB, MISC_RESET_CONTROL_2, 1, &buffer);
			buffer &= ~SOFT_RESET_2_ALL;
			retVal |= setRegister(pDCB, MISC_RESET_CONTROL_2, 1, &buffer);
#endif
			break;

		case CORE_SOFT_RESET_FAT_AGC:
			writeValue = SOFT_RESET_FAT_AGC;
			retVal |= getRegister(pDCB, MISC_RESET_CONTROL, 1, &buffer);
			buffer |= SOFT_RESET_FAT_AGC;
			retVal |= setRegister(pDCB, MISC_RESET_CONTROL, 1, &buffer);
			/* un-do all soft-reset flags */
			retVal |= getRegister(pDCB, MISC_RESET_CONTROL, 1, &buffer);
			buffer &= ~SOFT_RESET_FAT_AGC;
			retVal |= setRegister(pDCB, MISC_RESET_CONTROL, 1, &buffer);
			break;

#ifndef NXT2004

		case CORE_SOFT_RESET_FDC_XAGC:
			retVal |= getRegister(pDCB, MISC_RESET_CONTROL_2, 1, &buffer);
			buffer |= SOFT_RESET_FDC_XAGC;
			retVal |= setRegister(pDCB, MISC_RESET_CONTROL_2, 1, &buffer);
			/* un-do soft-reset flag */
			retVal |= getRegister(pDCB, MISC_RESET_CONTROL_2, 1, &buffer);
			buffer &= ~SOFT_RESET_FDC_XAGC;
			retVal |= setRegister(pDCB, MISC_RESET_CONTROL_2, 1, &buffer);
			break;

		case CORE_SOFT_RESET_FDC:
			retVal |= getRegister(pDCB, MISC_RESET_CONTROL_2, 1, &buffer);
			buffer |= SOFT_RESET_FDC;
			retVal |= setRegister(pDCB, MISC_RESET_CONTROL_2, 1, &buffer);
			/* un-do soft-reset flag */
			retVal |= getRegister(pDCB, MISC_RESET_CONTROL_2, 1, &buffer);
			buffer &= ~SOFT_RESET_FDC;
			retVal |= setRegister(pDCB, MISC_RESET_CONTROL_2, 1, &buffer);
			break;
#endif

#ifdef NXT2005
		case CORE_SOFT_RESET_RDC:
			retVal |= getRegister(pDCB, MISC_RESET_CONTROL, 1, &buffer);
			buffer |= SOFT_RESET_RDC;
			retVal |= setRegister(pDCB, MISC_RESET_CONTROL, 1, &buffer);
			/* un-do soft-reset flag */
			retVal |= getRegister(pDCB, MISC_RESET_CONTROL, 1, &buffer);
			buffer &= ~SOFT_RESET_RDC;
			retVal |= setRegister(pDCB, MISC_RESET_CONTROL, 1, &buffer);
			break;
#endif

		case CORE_SOFT_RESET_SMOOTHER:
			/* smoother reset is done in regConfigSmoother() */
			break;

		case CORE_SOFT_RESET_BERT:
			retVal |= getRegister(pDCB, MISC_RESET_CONTROL, 1, &buffer);
			buffer |= SOFT_RESET_BERT;
			retVal |= setRegister(pDCB, MISC_RESET_CONTROL, 1, &buffer);
			/* un-do soft-reset flag */
			retVal |= getRegister(pDCB, MISC_RESET_CONTROL, 1, &buffer);
			buffer &= ~SOFT_RESET_BERT;
			retVal |= setRegister(pDCB, MISC_RESET_CONTROL, 1, &buffer);
			break;
		default:
			retVal |= NXT_ERR_RANGE;
			break;
	}/* coreControl */

	/* restore settings as previously set by API */
	if (coreControl & 0x10) {
	/*if ((CORE_SOFT_RESET_ALL== coreControl) ||
		(CORE_SOFT_RESET_FAT_AGC == coreControl)) */
		/* fat agc polarity */
		fatSdmConfig &= ~AGC_SDM_POL_MASK;
		if (pDCB->fatAgcPolarity.sdm1_pol == INVERTED) {
			fatSdmConfig |= AGC_SDM1_INVERT;
		}
		if (pDCB->fatAgcPolarity.sdm2_pol == INVERTED) {
			fatSdmConfig |= AGC_SDM2_INVERT;
		}
		if (pDCB->fatAgcPolarity.sdmX_pol == INVERTED) {
			fatSdmConfig |= AGC_SDMX_INVERT;
		}
		if (pDCB->fatAgcPolarity.sdmA_pol == INVERTED) {
			fatSdmConfig |= AGC_SDMA_INVERT;
		}
		retVal |= setRegister(pDCB, AGC_SDM_CONFIGURE, 1, &fatSdmConfig);
	}

#ifndef NXT2004
	if (coreControl & 0x20) {
		/* fdc sdm polarity */
		fdcSdmConfig &= ~FDC_AGC_INVERT;
		if (pDCB->fdcAgcPolarity == INVERTED) {
			fdcSdmConfig |= FDC_AGC_INVERT;
		}
		retVal |= setRegister(pDCB, FDC_AGC_MODE, 1, &fdcSdmConfig);

		/* restore FDC diff decoder settings */
		retVal |= regSetFdcDecoderMode(pDCB, 
					pDCB->fdcDifdecMode.clockPolarity,
					pDCB->fdcDifdecMode.dataPolarity,
					pDCB->fdcDifdecMode.decoderMode);

		/* restore FDC IF frequency */
		retVal |= writeFdcIF(pDCB);
	}
#endif
	
	/*if (coreControl & 0x10) {*/
	if ((CORE_SOFT_RESET_ALL==coreControl) ||
		(CORE_SOFT_RESET_SMOOTHER==coreControl)) {
		/* mpeg polarity & format */		
		mpegFmt0 &= ~OUT_FMT_POL_MASK;
		mpegFmt0 &= ~OUT_FMT_DATA_MASK;
		if (pDCB->mpegFormat.dataEnablePolarity == INVERTED) {
			mpegFmt0 |= OUT_FMT_POL_DAT_EN;
		}
		if (pDCB->mpegFormat.pktSyncPolarity == INVERTED) {
			mpegFmt0 |= OUT_FMT_POL_PKT_SY;
		}
		if (pDCB->mpegFormat.errorPolarity == INVERTED) {
			mpegFmt0 |= OUT_FMT_POL_ERROR;
		}
		if (pDCB->mpegFormat.clockPolarity == NON_INVERTED) {
			mpegFmt0 |= OUT_FMT_POL_CLOCK;
		}

		if (pDCB->mpegFormat.bGatedOutputEnable) {
			mpegFmt0 |= OUT_FMT_DATA_GATED;
		}
		if (!pDCB->mpegFormat.bParallelOutputEnable) {
			mpegFmt0 |= OUT_FMT_DATA_SERIAL;
		}
		retVal |= setRegister(pDCB, MISC_MOD_CTRL_OUT_FMT, 1, &mpegFmt0);


		/* MPEG bContinuousRate enable */
		if (pDCB->mpegFormat.bContinuousRateEnable) {
			mpegFmt1 |= SMOOTHER_CTL_ENABLE_LOOP;
		}
		retVal |= setRegister(pDCB, SMOOTHER_CONTROL, 1, &mpegFmt1);
	}

	/*if (coreControl & 0x10) {*/
	if(CORE_SOFT_RESET_ALL==coreControl) {
		/* FAT BERT mode */
		if (pDCB->mpegFormat.bHeaderEnable) {
			/* normal operation */
			mpegMode[0] = FEC_MPEG_MODE_0_STD;
			mpegMode[1] = FEC_MPEG_MODE_1_STD;
		}
		else {
			/* bit-error-rate test mode */
			mpegMode[0] = FEC_MPEG_MODE_0_BER;
			mpegMode[1] = FEC_MPEG_MODE_1_BER;
		}
		retVal |= setRegister(pDCB, FEC_MPEG_MODE_0, 2, mpegMode);
		
		/* EQ test mux */
		retVal |= setRegister(pDCB, EQ_TEST_MUX_SELECT, 1, &eqTestMux);
	}

#ifdef NXT2005
	if ((coreControl & 0x40) && (pDCB->bAsicInitialized)) {
		/* restore RDC UGC settings */
		if (NULL != pDCB->pRdcUgcData)
			retVal |= regAgcSetup(pDCB, pDCB->pRdcUgcData);
	}
	/* enable upstream modulator */
	retVal |= getRegister(pDCB, USMOD_CONTROL, 1, &buffer);
	if (NXT_NO_ERROR == retVal) {
		buffer |= USMOD_CTL_MOD_ENABLE;
		retVal |= setRegister(pDCB, USMOD_CONTROL, 1, &buffer);
	}
	/* set modulator's output data format */
	retVal |= getRegister(pDCB, USMOD_IFACE_CONTROL, 1, &buffer);
	if (NXT_NO_ERROR == retVal) {
		buffer |= USMOD_OUT_DATA_FMT;
		retVal |= setRegister(pDCB, USMOD_IFACE_CONTROL, 1, &buffer);
	}
#endif
	/* release comm semaphore */
	NxtLeaveCriticalSection(pDCB->pContext, pDCB->hCSNxtComm);

	if (coreControl & 0x10) {
		/* restore Smoother settings, based on last mod format */
		switch (mpegFmt0 & MOD_FMT_MASK) {
			
			case MOD_FMT_16VSB:
				modFmt = NXT_16VSB;
				break;
			case MOD_FMT_64QAM:
				modFmt = NXT_64QAM;
				break;
			case MOD_FMT_256QAM:
				modFmt = NXT_256QAM;
				break;
			case MOD_FMT_8VSB:
			default:
				modFmt = NXT_8VSB;
				break;
		}

		retVal |= regConfigSmoother(pDCB, modFmt);
	}

	if (coreControl & 0x10) {
		if ((NXT_BERT_INPUT_FAT == pDCB->bertFormat.bertSource) ||
			(NXT_BERT_INPUT_TRELLIS == pDCB->bertFormat.bertSource) ||
			(NXT_BERT_INPUT_DI == pDCB->bertFormat.bertSource)) {
			/* restore internal BERT settings */
			retVal |= NxtConfigInternalBert(pDCB->pContext, pDCB->bertFormat.bertSource,
						pDCB->bertFormat.bertAlgorithm, pDCB->bertFormat.PNSequence,
						pDCB->bertFormat.removeBytes, pDCB->bertFormat.winSize);
		}
	}
#ifndef NXT2004
	if ((coreControl & 0x20) &&
		(NXT_BERT_INPUT_FDC == pDCB->bertFormat.bertSource)) {

		/* restore internal BERT settings */
		retVal |= NxtConfigInternalBert(pDCB->pContext, pDCB->bertFormat.bertSource,
					pDCB->bertFormat.bertAlgorithm, pDCB->bertFormat.PNSequence,
					pDCB->bertFormat.removeBytes, pDCB->bertFormat.winSize);
	}
#endif

	if (coreControl & 0x10) {
		/* make sure AGC start script gets run when NxtStart is used */
		pDCB->bFatStartScriptDone = FALSE;
	}


	return retVal;
} /* regCoreControl */

/**********************************************************************
 *
 * regLoadRAM
 *
 * Register level NxtLoadRam
 *
 * Inputs:
 *	NxtDCB *pDCB - pointer to current DCB -- ASSUMED FOUND!!!
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
 *	The microcontroller is held in reset during and after this operation.
 *	Use NxtStart() to restart acquisition following RAM loading.
 *
 **********************************************************************/
Data16 regLoadRAM(NxtDCB *pDCB,
			NxtLoadType_t loadType,
			Data16 startAddress,
			Data16 byteCount,
			Data8 *pBuffer) {

	Data16	retVal = NXT_NO_ERROR;
	Data16	ramBase;
	Data8	loadControl[3]; /* MSB, LSB, control */
	Data16	blockStart;
	Data16	bytesLeft;
	Data8	blockSize;
	Data16	bufferIndex;
	Data16	blockCrc;
	Data8	crcBuffer[2];
	Data8	fwControl;
	Data8	dummy		= 0;
	Data8	tries;

    static	Data8	lBuffer[MAX_XFER_BLOCK_SIZE];

	if (loadType == NXT_LOAD_AGC) {

		/* range within AGC table space */
		if (startAddress + byteCount > AGC_RAM_SIZE) {
			retVal = NXT_ERR_RANGE;
		}
	} 
	else {

		/* UC RAM start address depends on ROM DISABLE state */
		retVal = getRamBase(pDCB, &ramBase);

		if (NXT_NO_ERROR == retVal) {
			if (startAddress < ramBase) {
				retVal = NXT_ERR_RANGE;
			}
			else if (startAddress + byteCount > ramBase + UC_RAM_SIZE) {
				retVal = NXT_ERR_RANGE;
			}
			/* code and script files have additional constraint*/
			if (loadType == NXT_LOAD_CODE){
				if (startAddress != ramBase) {
					retVal = NXT_ERR_RANGE;
				}
			}
		}
	}

	stopFatAcquisition(pDCB);
#ifndef NXT2004
	stopFdcAcquisition(pDCB);
#endif
	
	/* reset the microcontroller */
	if (retVal == NXT_NO_ERROR) {
		retVal = resetMicro(pDCB);
	}

	/* wait for comm semaphore */
	NxtWaitForCriticalSection(pDCB->pContext, pDCB->hCSNxtComm);

	if (retVal == NXT_NO_ERROR) {
		
		/* setup load control forcing reset */
		loadControl[2] = UC_AGC_PDC_RESET | UC_AGC_PDC_XFER_EN;
		if (loadType == NXT_LOAD_AGC) {
			loadControl[2] |= UC_AGC_PDC_AGC_LOAD;
		}
	}

	/* loop transferring maximum block size till done */

	tries = 0;

	while ((retVal == NXT_NO_ERROR) && (tries <= MAX_DNLD_TRIES)) {

		blockStart = startAddress;
		bytesLeft = byteCount;
		bufferIndex = 0;
		/* write RAM access control */
		loadControl[0] = (Data8)((blockStart>>8) & 0xFF);
		loadControl[1] = (Data8)(blockStart & 0xFF);
		loadControl[2] |= UC_AGC_PDC_XFER_EN;

		retVal = setRegister(pDCB, UC_AGC_START_ADDR_MSB, 3, loadControl);
		blockCrc = 0;

		while ((retVal == NXT_NO_ERROR) && (bytesLeft)) {

			/* how many bytes get sent this time */
			if (bytesLeft <= MAX_XFER_BLOCK_SIZE) {
				blockSize = (Data8)bytesLeft;
			}
			else {
				blockSize = MAX_XFER_BLOCK_SIZE;
			}

			/* transfer blockSize from blockStart to bufferIndex */
			memcpy(lBuffer, &pBuffer[bufferIndex], blockSize);
			/* transfer data */
			retVal = setRegister(pDCB, 
						UC_AGC_DATA_TRANSFER, 
						blockSize, 
						lBuffer);
			if (retVal == NXT_NO_ERROR) {
				/* calculate block crc */
				for(dummy = 0; dummy < blockSize; dummy++) {
					blockCrc = NxtCrc(blockCrc, pBuffer[bufferIndex + dummy]);
				}
			}
			/* adjust blockStart, bytesLeft, bufferIndex */
			blockStart += blockSize;
			bufferIndex += blockSize;
			bytesLeft -= blockSize;
		}/* while */

		/* send block crc */
		crcBuffer[0] = (Data8)(blockCrc>>8) & 0xFF;
		crcBuffer[1] = (Data8)blockCrc & 0xFF;
		retVal = setRegister(pDCB, UC_AGC_DATA_TRANSFER, 2, crcBuffer);

		/* dummy read to end the transfer */
		retVal |= getRegister(pDCB, UC_AGC_DATA_TRANSFER, 1, &dummy);

		/* disable transfer mode */
		if (retVal == NXT_NO_ERROR) {
			loadControl[2] &= ~UC_AGC_PDC_XFER_EN;
			retVal = setRegister(pDCB, UC_AGC_PGM_DNLD_CTL, 
								 1, &loadControl[2]);
		}

		/* read back crc check result */
		if (retVal == NXT_NO_ERROR) {
			
			retVal = getRegister(pDCB, UC_AGC_PGM_DNLD_CTL, 1, &dummy);
			dummy &= UC_AGC_PDC_CRC_FAIL;
		}
		if ((retVal == NXT_NO_ERROR) && dummy) {
			/* crc failed */
			OutputDebugString("CRC Failure\n");
			if (tries++ >= MAX_DNLD_TRIES) {
				retVal = NXT_ERR_COMM;
			}
		}
		else {
			tries = MAX_DNLD_TRIES + 1; /* to signal end of while function */
		}
	}/* while */

	/* Enable downloaded RAM code */
	retVal |= getRegister(pDCB, MISC_FIRMWARE_CONTROL, 1, &fwControl);
	if (NXT_NO_ERROR == retVal) {
		fwControl |= MISC_FW_ENABLE_RAM_DNLD;
		retVal = setRegister(pDCB, MISC_FIRMWARE_CONTROL, 1, &fwControl);
	}

	/* release comm semaphore */
	NxtLeaveCriticalSection(pDCB->pContext, pDCB->hCSNxtComm);

	pDCB->bFatStartScriptDone = FALSE;

	/* release micro from reset with acquisition disabled */
	unresetMicro(pDCB);
	stopFatAcquisition(pDCB);
#ifndef NXT2004
	stopFdcAcquisition(pDCB);
#endif

	return retVal;
} /* regLoadRAM */

#ifndef NXT2004
/**********************************************************************
 * enableRomFdc
 *
 * Enables ROM to use the FDC script
 * 
 * Inputs:
 *	NxtDCB *pDCB - pointer to current DCB -- ASSUMED FOUND!!!
 *
 * Returns:
 *	Data16
 *
 * Outputs:
 *	None
 **********************************************************************/
Data16 enableRomFdc(NxtDCB *pDCB) {
	
	Data16	retVal;
	Data8	fwControl;

	/* wait for comm semaphore */
	NxtWaitForCriticalSection(pDCB->pContext, pDCB->hCSNxtComm);	

	retVal = getRegister(pDCB, MISC_FIRMWARE_CONTROL, 1, &fwControl);

	if (NXT_NO_ERROR == retVal) {
		
		/* Enable ROM-ZS to use FDC script */
		fwControl |= MISC_FW_ENABLE_FDC;

		retVal = setRegister(pDCB, MISC_FIRMWARE_CONTROL, 1, &fwControl);
	}

	/* release comm semaphore */
	NxtLeaveCriticalSection(pDCB->pContext, pDCB->hCSNxtComm);

	return retVal;

} /* enableRomFdc */
#endif

/**********************************************************************
 *
 * getRamBase
 *
 * Determines the base address for RAM based on ROM DISABLED bit
 *
 * Inputs:
 *	NxtDCB *pDCB - pointer to current DCB -- ASSUMED FOUND!!!
 *
 * Returns:
 *	Data16
 *
 * Outputs:
 *	Data16 *pRamBase
 *
 **********************************************************************/
Data16 getRamBase(NxtDCB *pDCB, Data16 *pRamBase) {

	Data16	retVal;
	Data8	hwStatus;

	/* wait for comm semaphore */
	NxtWaitForCriticalSection(pDCB->pContext, pDCB->hCSNxtComm);

	retVal = getRegister(pDCB, MISC_ASIC_HW_STATUS_1, 1, &hwStatus);

	/* release comm semaphore */
	NxtLeaveCriticalSection(pDCB->pContext, pDCB->hCSNxtComm);

	if (retVal == NXT_NO_ERROR) {

		if (hwStatus & INT_ROM_DISABLED) {
			*pRamBase = RAM_BASE_ROM_DISABLED;
		}
		else {
			*pRamBase = RAM_BASE_ROM_ENABLED;
		}
	}
	
	return retVal;
} /* getRamBase */

#ifndef NXT2004
/**********************************************************************
 *
 * writeFdcIF
 *
 * Normalizes the FDC IF frequency and writes it to the FDC register core
 * using microcontroller services
 *
 * Inputs:
 *	NxtDCB *pDCB - pointer to current DCB -- ASSUMED FOUND!!!
 *
 * Returns:
 *	Data16
 *
 * Outputs:
 *	None
 *
 **********************************************************************/
Data16 writeFdcIF(NxtDCB *pDCB) {

	Data16	retVal = NXT_NO_ERROR;
	Data16	freqIF_reg; /* register compatible IF freq */
	Data8	svcData;
	Data8	regData[2];
	Data8	tries = 0;	/* loop counter to poll for service completion */

	/* calculate freqIF_reg */
	freqIF_reg = (Data16)(0.5 +
							((2 * NXT_CRYSTAL_FREQ - pDCB->fdcIFfreq) / 
							NXT_CRYSTAL_FREQ) * 
							(double)0x2000
						);

	freqIF_reg &= 0x1FFF;

	/* write IF freq in FDC_IF_FREQ using microcontroller services */	
	
	/* compose data to be written */
	regData[0] = (Data8) (freqIF_reg >> 8);
	regData[1] = (Data8) (freqIF_reg & 0xFF);

	retVal = setRegister(pDCB, UC_GP_5, FDC_IF_FREQ_SIZE, regData);

	if (NXT_NO_ERROR == retVal) {

		/* set data xfer type and byte count */
		svcData = FDC_IF_FREQ_SIZE;	
		svcData |= UC_FDC_IF_DATA_XFER;
		
		retVal = setRegister(pDCB, UC_GP_4, 1, &svcData);

		if (NXT_NO_ERROR == retVal) {

			/* set data xfer service */
			svcData = UC_SERVICES_DATA_XFER;
			retVal = setRegister(pDCB, UC_SERVICES, 1, &svcData);

			/* poll for data_xfer completion bit only if IRQ mask is not set */
			if (!(pDCB->irqMask & NXT_INT_XFER)) {
				do {

					/* wait for data xfer to complete */
					retVal = getRegister(pDCB, UC_SERVICES, 1, &svcData);

					if (svcData & UC_SERVICES_DATA_XFER) {
						NxtSuspendThread(pDCB->pContext, 1);
						tries ++;
					}
				} while ( (tries < XFER_SERVICE_MAX_TRIES) &&
						(NXT_NO_ERROR == retVal) && 
						(svcData & UC_SERVICES_DATA_XFER));
				/* check for error conditions */
				if (tries >= XFER_SERVICE_MAX_TRIES) {
					retVal = NXT_ERR_TIMEOUT;
				}
				/* check data_xfer service error */
				else if (svcData & UC_SERVICES_DATA_XFER_ERR){
					
					retVal = NXT_ERR_IIC_XFER;
				}
			} /* !NXT_INT_XFER */
		}
	}

	return retVal;

}/* writeFdcIF */

/**********************************************************************
 *
 * regSetFdcDecoderMode
 *
 * Register level NxtSetFdcDecoderMode
 *
 * Inputs:
 *	NxtDCB *pDCB - pointer to current DCB -- ASSUMED FOUND!!!
 *	NxtPolarity_t clockPolarity - INVERTED or NON_INVERTED
 *	NxtPolarity_t dataPolarity - INVERTED or NON_INVERTED
 *	NxtPolarity_t decoderMode - INVERTED for Alternative Mode
 *								NON_INVERTED for Normal Mode
 *
 * Returns:
 *	Data16
 *
 * Notes:
 *	Clock polarity defaults to NON_INVERTED if this function is not called.
 *	Data polarity defaults to NON_INVERTED if this function is not called.
 *	Decoder Mode defaults to Alternative Mode (INVERTED)
 *
 **********************************************************************/
Data16 regSetFdcDecoderMode(NxtDCB *pDCB,
			NxtPolarity_t clockPolarity,
			NxtPolarity_t dataPolarity,
			NxtPolarity_t decoderMode) {

	Data16 retVal = NXT_NO_ERROR;
	Data8	buffer;

	/* range check all inputs */
	if ((clockPolarity != NON_INVERTED && 
		 clockPolarity != INVERTED) ||
		(dataPolarity != NON_INVERTED && 
		 dataPolarity != INVERTED) ||
		(decoderMode != NON_INVERTED && 
		 decoderMode != INVERTED)) {

		retVal = NXT_ERR_RANGE;
	}
	else {
		

		/* read FDC_CONTROL */
		retVal = getRegister(pDCB, FDC_CONTROL, 1, &buffer);

		if (retVal == NXT_NO_ERROR) {
			/* modify all polarity bits */
			buffer &= ~FDC_CTL_POL_MASK;

			if (clockPolarity == INVERTED) {
				buffer |= FDC_CTL_CRX_INVERT;
			}
			if (dataPolarity == INVERTED) {
				buffer |= FDC_CTL_DRX_INVERT;
			}
			if (decoderMode == NON_INVERTED) {
				buffer |= FDC_CTL_DIFEC_NORMAL;
			}
			/* write FDC_CONTROL */
			retVal = setRegister(pDCB, FDC_CONTROL, 1, &buffer);
		}


		/* save current settings */
		pDCB->fdcDifdecMode.clockPolarity = clockPolarity;
		pDCB->fdcDifdecMode.dataPolarity = dataPolarity;
		pDCB->fdcDifdecMode.decoderMode = decoderMode;
	}

	return retVal;

}/* regSetFdcDecoderMode */
#endif

/**********************************************************************
 *
 * regGetBertData
 *
 * Register level NxtGetBertData.
 *
 * Inputs:
 *	NxtDCB *pDCB - pointer to current DCB -- ASSUMED FOUND!!!
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
 *
 * Notes:
 *	The user can specify if they want to wait till the error data is
 *	ready. If the user doesn't specify that, and the data is not ready
 *	then the function will return error.
 *
 **********************************************************************/
Data16 regGetBertData(NxtDCB *pDCB, Data32 *pErrorCount, Bool bWait) {

	Data16	retVal = NXT_NO_ERROR;
	Data16	tries = 0;	
	Data8   buffer;
	Data8	status;
	Data8	bertErr[4];


	retVal = getRegister(pDCB, BERT_CTL_A, 1, &buffer);
	if (NXT_NO_ERROR == retVal) {

		/* check if BERT is running */
		if (buffer & BERT_CTL_A_ON) {
		
			/* BERT is running */
			retVal = getRegister(pDCB, BERT_CTL_B, 1, &buffer);
	
			if (NXT_NO_ERROR == retVal) {
			
				/* check for loss of lock */
				if (buffer & BERT_CTL_LSS_LCK) {
					/* loss of lock */
					retVal = NXT_ERR_NO_LOCK;
				} else {

					/* check for acquiring/locked */
					retVal = getRegister(pDCB, BERT_STATUS, 1, &status);

					if (NXT_NO_ERROR == retVal && 
						status & BERT_STATUS_LOCKED) {

						/* locked */					
						if (buffer & BERT_CTL_NEW_ERR) {

							/* read error data */
							retVal = getRegister(pDCB, 
										BERT_ERR_CNTR_3, 4, bertErr);

							*pErrorCount = DATA8_TO_DATA32(bertErr);
						} else {
						
							if (TRUE == bWait) {
								/* poll for new error data */
								while ( (retVal == NXT_NO_ERROR) &&
										!(buffer & BERT_CTL_NEW_ERR) &&
										(tries < BERT_DATA_MAX_TRIES) ) {

									/* release comm semaphore */
									NxtLeaveCriticalSection(pDCB->pContext, 
														pDCB->hCSNxtComm);

									NxtSuspendThread(pDCB->pContext, 
											BERT_DATA_POLL_DELAY);

									/* wait for comm semaphore */
									NxtWaitForCriticalSection(pDCB->pContext, 
														pDCB->hCSNxtComm);

									retVal = getRegister(pDCB, 
												BERT_CTL_B, 1, &buffer);
									tries++;
								} /* while */

								if (NXT_NO_ERROR == retVal) {
									/* read error */
									retVal = getRegister(pDCB, 
												BERT_ERR_CNTR_3, 4, bertErr);
									*pErrorCount = DATA8_TO_DATA32(bertErr);
								}
							} else {
								retVal = NXT_ERR_NOT_READY;
							}
						}
						
					} else {

						/* BERT is not locked, still trying to acquire */
						retVal = NXT_ERR_NO_LOCK;	
					}
				}
			}
		
		} else {

			/* BERT not running */
			retVal = NXT_ERR_MODE;
		}

	}

	return retVal;

} /* regGetBertData */

/**********************************************************************
 *
 * regConfigSmoother
 *
 * Configures the Smoother core based on the modulation format and
 * mpeg modes selected by the user.
 *
 * Inputs:
 *	NxtDCB *pDCB - pointer to current DCB -- ASSUMED FOUND!!!
 *	NxtModFormat_t mode - current modulation format
 *
 * Returns:
 *	Data16
 *
 * Outputs:
 *	None
 *
 * Notes:
 *	This function will only work if mpeg output smoother is enabled in
 *	the MISC core.
 *
 **********************************************************************/
Data16 regConfigSmoother(NxtDCB *pDCB, NxtModFormat_t mode) {

	Data32	nco_2;
	Data16	retVal = NXT_NO_ERROR;
	Data8	ctlData;
	Data8	regData;
	Data8	ncoData[3];
	Data8	buffer;

	if (mode != NXT_8VSB &&
		mode != NXT_64QAM &&
		mode != NXT_256QAM) {

		retVal = NXT_ERR_RANGE;

	} else {

		/* wait for comm semaphore */
		NxtWaitForCriticalSection(pDCB->pContext, pDCB->hCSNxtComm);

		/* make sure the smoother is stopped */
		retVal = getRegister(pDCB, SMOOTHER_CONTROL, 1, &ctlData);
		if (NXT_NO_ERROR == retVal) {
			ctlData &= ~SMOOTHER_CTL_GO_AGC;
			retVal = setRegister(pDCB, SMOOTHER_CONTROL, 1, &ctlData);
		}

		/* reset the core */
		retVal |= getRegister(pDCB, MISC_RESET_CONTROL, 1, &buffer);
		buffer |= SOFT_RESET_SMOOTHER;
		retVal |= setRegister(pDCB, MISC_RESET_CONTROL, 1, &buffer);
		/* un-do soft-reset flag */
		retVal |= getRegister(pDCB, MISC_RESET_CONTROL, 1, &buffer);
		buffer &= ~SOFT_RESET_SMOOTHER;
		retVal |= setRegister(pDCB, MISC_RESET_CONTROL, 1, &buffer);

		if (NXT_NO_ERROR == retVal) {

			retVal = getRegister(pDCB, SMOOTHER_CONTROL, 1, &ctlData);

			ctlData &= ~SMOOTHER_CTL_MASK;
			
			if (NXT_8VSB == mode) {
				/* enable error filter */
				ctlData &= ~SMOOTHER_CTL_DIS_FILTER;
			} else {
				/* disable error filter for 64QAM and 256QAM */
				ctlData |= SMOOTHER_CTL_DIS_FILTER;
			}

			/* check for loop mode - burst or continuous */
			/*if (!(ctlData & SMOOTHER_CTL_ENABLE_LOOP)) {*/
			if (!(pDCB->mpegFormat.bContinuousRateEnable)) {
				/* resume data output */
				ctlData |= SMOOTHER_CTL_RESUME_DATA_OP;
			}

			if (pDCB->mpegFormat.bContinuousRateEnable) {
				/* enable feedback control loop */
				ctlData |= SMOOTHER_CTL_ENABLE_LOOP;
			} else {
				ctlData &= ~SMOOTHER_CTL_ENABLE_LOOP;
			}
			retVal |= setRegister(pDCB, SMOOTHER_CONTROL, 1, &ctlData);


			/* set smoother target level */
			if (pDCB->mpegFormat.bContinuousRateEnable) {
				/* continuous mode */
				if (NXT_8VSB == mode) {
					regData = SMOOTHER_BUF_FILL_LEVEL_VSB / 2;
				} else {
					/* for 64QAM and 256QAM */
					regData = SMOOTHER_BUF_FILL_LEVEL_QAM / 2;
				}
			} else {
				/* burst mode */
				regData = 0x00;
			}

			retVal |= setRegister(pDCB, SMOOTHER_TARGET, 1, &regData);


			/* set nominal NCO_2 */
			switch (mode) {

				case NXT_64QAM:
					if(TRUE == pDCB->mpegFormat.bHeaderEnable) {
						nco_2 = SMOOTHER_NCO2_64QAM_SYNC_EN;
					} else {
						nco_2 = SMOOTHER_NCO2_64QAM_SYNC_DIS;
					}
					break;

				case NXT_256QAM:
					if(TRUE == pDCB->mpegFormat.bHeaderEnable) {
						nco_2 = SMOOTHER_NCO2_256QAM_SYNC_EN;
					} else {
						nco_2 = SMOOTHER_NCO2_256QAM_SYNC_DIS;
					}
					break;

				case NXT_8VSB:
				default:
					if(TRUE == pDCB->mpegFormat.bHeaderEnable) {
						nco_2 = SMOOTHER_NCO2_8VSB_SYNC_EN;
					} else {
						nco_2 = SMOOTHER_NCO2_8VSB_SYNC_DIS;
					}
					break;
			}/* switch */
	
			/* check for loop mode - burst or continuous */
			/*if (!(ctlData & SMOOTHER_CTL_ENABLE_LOOP)) { */
			if (!(pDCB->mpegFormat.bContinuousRateEnable)) {
				/* set nco_2 to max */
				nco_2 = SMOOTHER_NCO2_MAX;
			}

			nco_2 &= 0xFFFFFF;
			ncoData[0] = (Data8) (nco_2 >> 16);
			ncoData[1] = (Data8) ((nco_2 & 0xFFFF) >> 8);
			ncoData[2] = (Data8) (nco_2 & 0xFF);

			retVal |= setRegister(pDCB, SMOOTHER_NOMINAL_NCO_2, 3, ncoData);


			retVal |= getRegister(pDCB, SMOOTHER_BANDWIDTH, 1, &regData);
			regData &= ~SMOOTHER_BW_MASK;
			
			/* set zeta */
			regData |= SMOOTHER_ZETA;
			
			/* set smoother bandwidth */
			if (NXT_8VSB == mode) {
				regData |= SMOOTHER_BW_VSB;
			} else {
				/* for 64QAM and 256QAM */
				regData |= SMOOTHER_BW_QAM;
			}
			
			retVal |= setRegister(pDCB, SMOOTHER_BANDWIDTH, 1, &regData);


			/* set the go bit, only if all settings were successful */
			if (NXT_NO_ERROR == retVal) {
				
				retVal = getRegister(pDCB, SMOOTHER_CONTROL, 1, &regData);
				
				if (NXT_NO_ERROR == retVal) {

					regData |= SMOOTHER_CTL_GO_AGC;
					
					retVal = setRegister(pDCB, SMOOTHER_CONTROL, 1, &regData);
				}
			}
		}

		/* release comm semaphore */
		NxtLeaveCriticalSection(pDCB->pContext, pDCB->hCSNxtComm);
	
	}

	return retVal;

}/* regConfigSmoother */

#ifdef NXT2005
/**********************************************************************
 *
 * regSendRdcTaps()
 *
 * Loads the PSF coefficients into the USMOD. PSF coefficients depend
 * on the value of alpha
 *
 * Inputs:
 *	NxtDCB *pDCB - pointer to current DCB -- ASSUMED FOUND!!!
 *	NxtRdcAlpha_t alpha - value of alpha
 *
 * Returns:
 *	Data16
 *
 * Outputs:
 *	None
 *
 * Notes:
 *	Caller should secure the comm semaphore. This function assumes 
 *	comm semaphore is already in place
 *
 **********************************************************************/
Data16 regSendRdcTaps(NxtDCB *pDCB, 
					  NxtRdcAlpha_t alpha) {
	
	Data16 retVal = NXT_NO_ERROR;
	int i;
	Data8 taps_25[RDC_TAPS_SIZE] = {	
		0xFF,0xF6,0x00,0x06,
		0x00,0x0A,0xFF,0xF1,
		0xFF,0xFD,0x00,0x12,
		0xFF,0xF2,0xFF,0xFA,
		0x00,0x29,0xFF,0xDD,
		0xFF,0xB8,0x00,0x7D,
		0x00,0x66,0xFE,0xBA,
		0xFF,0x85,0x04,0xA8	};

	Data8 taps_30[RDC_TAPS_SIZE] = {	
		0xFF,0xFE,0xFF,0xFA,
		0x00,0x0A,0xFF,0xFD,
		0xFF,0xF3,0x00,0x10,
		0x00,0x03,0xFF,0xEB,
		0x00,0x17,0xFF,0xFD,
		0xFF,0xC1,0x00,0x55,
		0x00,0x6B,0xFE,0xDD,
		0xFF,0x72,0x04,0x8D	};

	Data8 taps_50[RDC_TAPS_SIZE] = {	
		0xFF,0xFC,0x00,0x04,
		0x00,0x00,0xFF,0xFC,
		0x00,0x08,0xFF,0xF9,
		0xFF,0xFF,0x00,0x09,
		0xFF,0xEE,0x00,0x13,
		0x00,0x05,0xFF,0xE5,
		0x00,0x4C,0xFF,0x79,
		0xFF,0x41,0x04,0x13	};
	
	Data8 data = 0x80;
	retVal = setRegister(pDCB, USMOD_COEFF_ADDR, 1, &data);
	
	switch (alpha) {
	case NXT_RDC_ALPHA_25:
		/* write the values to the register */
		for (i = 0; i < RDC_TAPS_SIZE; i++) {
			retVal |= setRegister(pDCB, USMOD_COEFF_DATA, 1, taps_25+i);
		}
		break;
	case NXT_RDC_ALPHA_30:
		/* write the values to the register */
		for (i = 0; i < RDC_TAPS_SIZE; i++) {
			retVal |= setRegister(pDCB, USMOD_COEFF_DATA, 1, taps_30+i);
		}
		break;
	case NXT_RDC_ALPHA_50:
		/* write the values to the register */
		for (i = 0; i < RDC_TAPS_SIZE; i++) {
			retVal |= setRegister(pDCB, USMOD_COEFF_DATA, 1, taps_50+i);
		}
		break;
	}

	return retVal;

}/* regSendRdcTaps */
#endif

/**********************************************************************
 *
 * regNxtennaSvc
 *
 * Performs the specified Nxtenna data transfer service.
 *
 * Inputs:
 *	NxtDCB *pDCB - pointer to current DCB -- ASSUMED FOUND!!!
 *	Data8 uc_gp4 - value to be written to UC_GP_4
 *	Data8 uc_gp5 - value to be written to UC_GP_5
 *	Data8* pUc_gp6 - pointer to values in UC_GP_6 (and beyond)
 *			if uc_gp4 == UC_NXTENNA_WRITE
 *	Data8 bytes - number of bytes to transfer
 *
 * Returns:
 *	Data16
 *
 * Outputs:
 *	Data8* pUc_gp6 - pointer to values in UC_GP_6 (and beyond)
 *			if uc_gp4 == UC_NXTENNA_READ
 *
 * Notes:
 *	
 *
 **********************************************************************/
Data16 regNxtennaSvc(NxtDCB *pDCB, 
					 Data8 uc_gp4, 
					 Data8 uc_gp5, 
					 Data8* pUc_gp6,
					 Data8 bytes) {
	
	Data16	retVal = NXT_NO_ERROR;
	Data8	buffer;
	Data8	tries = 0;

	if (uc_gp4 == UC_NXTENNA_WRITE || uc_gp4 == UC_NXTENNA_READ) {
		/* wait for comm semaphore */
		NxtWaitForCriticalSection(pDCB->pContext, pDCB->hCSNxtComm);
		
		/* specify the data_xfer service */
		buffer = uc_gp4;
		retVal = setRegister(pDCB, UC_GP_4, 1, &buffer);

		buffer = uc_gp5;
		/* write to UC_GP_5 */
		retVal |= setRegister(pDCB, UC_GP_5, 1, &buffer);
		if (uc_gp4 == UC_NXTENNA_WRITE) {
			retVal = setRegister(pDCB, UC_GP_6, bytes, pUc_gp6);
		}
		if (NXT_NO_ERROR == retVal) {
			/* set the service flag */
			buffer = UC_SERVICES_DATA_XFER;
			retVal = setRegister(pDCB, UC_SERVICES, 1, &buffer);
		}
		
		if (uc_gp4 == UC_NXTENNA_WRITE) {
			
			/* poll for data_xfer completion bit, if IRQ mask is not set */
			if (!(pDCB->irqMask & NXT_INT_XFER)) {
				do {

					/* wait for data xfer to complete */
					retVal = getRegister(pDCB, UC_SERVICES, 1, &buffer);

					if (buffer & UC_SERVICES_DATA_XFER) {
						NxtSuspendThread(pDCB->pContext, 1);
						tries ++;
					}
				} while ( (tries < XFER_SERVICE_MAX_TRIES) &&
						(NXT_NO_ERROR == retVal) && 
						(buffer & UC_SERVICES_DATA_XFER));

				/* check for error conditions */
				if (tries >= XFER_SERVICE_MAX_TRIES) {
					retVal = NXT_ERR_TIMEOUT;
				}
				/* check data_xfer service error */
				else if (buffer & UC_SERVICES_DATA_XFER_ERR){
					
					retVal = NXT_ERR_MODE;
				}
			} /* !NXT_INT_XFER */
		} else {
			/* poll for data_xfer completion bit */
			do {
				/* wait for data xfer to complete */
				retVal |= getRegister(pDCB, UC_SERVICES, 1, &buffer);

				if (buffer & UC_SERVICES_DATA_XFER) {
					NxtSuspendThread(pDCB->pContext, 1);
					tries ++;
				}
			} while ( (tries < XFER_SERVICE_MAX_TRIES) &&
					(NXT_NO_ERROR == retVal) && 
					(buffer & UC_SERVICES_DATA_XFER));

			if (NXT_NO_ERROR == retVal && tries < XFER_SERVICE_MAX_TRIES) {
				
				/* check data_xfer service error */
				if (!(buffer & UC_SERVICES_DATA_XFER_ERR)) {
					
					retVal = getRegister(pDCB, UC_GP_6, bytes, pUc_gp6);					
				} 
			} else	if (tries >= XFER_SERVICE_MAX_TRIES) {

						retVal = NXT_ERR_TIMEOUT;
					} else {
						retVal = NXT_ERR_MODE;
					}
		}

		/* release comm semaphore */
		NxtLeaveCriticalSection(pDCB->pContext, pDCB->hCSNxtComm);

	} else {
		retVal = NXT_ERR_MODE;
	}

	return retVal;
}/* regNxtennaSvc */

#endif ALPSTUNER
