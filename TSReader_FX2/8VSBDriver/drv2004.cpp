#ifdef ALPSTUNER
/**********************************************************************
 * DRV2004.C
 * Public Interface to DRV2004
 *
 * Copyright (C) 2002 NxtWave Communications, Inc.
 *
 * $Log:   //panxtbdc01/AppsEng/PVCS/Application Eval/archives/EVAL2004/Drv2004/drv2004.c-arc  $
 * 
 *    Rev 1.10   Oct 01 2002 14:00:22   raggarwa
 * Corrected dev1 default address
 * 
 *    Rev 1.9   Jun 18 2002 09:33:06   raggarwal
 * Alpha3 Release
 * 
 *    Rev 1.8   May 17 2002 16:34:12   raggarwal
 * Alpha2 Release
 * 
 *    Rev 1.3   Mar 08 2002 16:28:50   raggarwal
 * Alpha Release
 * 
 **********************************************************************/

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

extern int fNXT2004InitDone;


DRV2_API Data16 NxtInit2004Driver(void *pContext) {

	Data16  retVal = NXT_NO_ERROR;
	NxtDCB *pDCB;
	Data8	iicAddr;

	/* find or create the referenced DCB */
	pDCB = GetDCB(pContext);

	if (pDCB == INVALID_CONTEXT) {	
		/* memory allocation error */
		retVal = NXT_ERR_MEMORY;
	}
	else { 
		if (!pDCB->bInitialized) {

			/* obtain OS resources to support the DCB */
			if (NxtInitCriticalSection(pContext, &(pDCB->hCSNxtComm ))) {
				retVal = NXT_ERR_OS;
			}

			else if (NxtInitCriticalSection(pContext, &(pDCB->hCSNxtFatTrack ))) {
				retVal = NXT_ERR_OS;
			}
		
		}
	}

	if (retVal == NXT_NO_ERROR) {

		if (!pDCB->bInitialized) {

			/* get the IIC dev0 address */
			pDCB->nxtDev0Addr = NxtGetDev0Addr(pContext);

			/* get the IIC dev1 address */
			iicAddr = NxtGetDev1Addr(pContext);
			
			if (0 == iicAddr) {
				/* 0 means dev1 is at default location */
				pDCB->nxtDev1Addr = (Data8) (pDCB->nxtDev0Addr + 0x02);
			} else {
				pDCB->nxtDev1Addr = iicAddr;
			}

			/* wait for comm semaphore */
			NxtWaitForCriticalSection(pDCB->pContext, pDCB->hCSNxtComm);

			/* write IIC slave 1 address in MISC core */
			retVal = setRegister(pDCB, MISC_IIC_SLAVE_ADDR_1, 1, &iicAddr);

			/* release comm semaphore */
			NxtLeaveCriticalSection(pDCB->pContext, pDCB->hCSNxtComm);

			pDCB->bInitialized = TRUE;
		}
		
		/* initialize the ASIC */
		if (!fNXT2004InitDone)
			retVal |= regInitASIC(pDCB);
	}

	return retVal;
} /* NxtInit2004Driver */


/**********************************************************************
 *
 * NxtInit2004ASIC
 *
 * Initializes NXT2004 ASIC and reloads current firmware
 *
 * Inputs:
 *	void *pContext	- NULL, or user-defined ID for multiple NXT2003s
 *
 * Returns:
 *	Data16
 *
 * Notes: 
 *	This should be used after a hardware reset is performed.
 *	Acquisition is stopped following this call.
 *
 **********************************************************************/
DRV2_API Data16 NxtInit2004ASIC(void *pContext) {

	Data16 retVal;
	NxtDCB	*pDCB;
	NxtDCB	*pDummy;

	/* find the referenced DCB */
	pDCB = FindDCB(pContext, &pDummy);

	if (pDCB == INVALID_CONTEXT) {	/* referenced DCB never initialized */
		retVal = NXT_ERR_INIT;
	}
	else {
		retVal = regInitASIC(pDCB);
	}
	return retVal;
} /* NxtInit2004ASIC */


/**********************************************************************/
/*                          Acquisition APIs                          */
/**********************************************************************/

/**********************************************************************/
/*                              Query APIs                            */
/**********************************************************************/

/**********************************************************************/
/*                          Configuration APIs                        */
/**********************************************************************/

/**********************************************************************/
/*                       Interrupt-Related APIs                       */
/**********************************************************************/


#endif ALPSTUNER