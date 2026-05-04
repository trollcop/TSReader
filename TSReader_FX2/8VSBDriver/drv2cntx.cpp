#ifdef ALPSTUNER
/**********************************************************************
 * DRV2CNTX.C
 * Context management for DRV200X
 *
 * Copyright (C) 2002 NxtWave Communications, Inc.
 *
 * $Log:   //panxtbdc01/AppsEng/PVCS/Application Eval/archives/EVAL2005/Drv2005/drv2cntx.c-arc  $
 * 
 *    Rev 1.17   Feb 27 2003 15:14:24   raggarwa
 * Added nullification of bInit flags; PR 65/2003
 * 
 *    Rev 1.16   Sep 05 2002 11:49:28   raggarwa
 * Merged main trunk and nxtenna
 * 
 *    Rev 1.15.1.1   Aug 14 2002 14:30:50   reichgot
 * Changed default mapping to straight 1-1, 2-2, etc.
 * 
 *    Rev 1.15.1.0   Jun 27 2002 17:30:40   raggarwal
 * Added Nxtenna settings
 * 
 *    Rev 1.15   Jun 17 2002 10:52:08   raggarwal
 * Alpha3 Release
 * 
 *    Rev 1.13   May 17 2002 16:05:00   raggarwal
 * Alpha2 Release
 * 
 *    Rev 1.4   Mar 08 2002 16:28:52   raggarwal
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
Static variables
******************************************************************************
*/

/* One Device Control Block is guaranteed --
 * this is the root of a list if multiple are needed.
 * Hardware defaults are for MEV, TUNER 1, EVAL-ATV.
 */
static NxtDCB dcb0 = {
	FALSE,					/* bInitialized */
	FALSE,					/* bAsicInitialized */
	0,						/* dev_0 Addr */
	0x04,					/* dev_1 Addr */
	{
		INVERTED,			/* sdm1_pol */
		INVERTED,			/* sdm2_pol */
		NON_INVERTED,		/* sdmX_pol */
		INVERTED			/* sdmA_pol */
	},	/* fatAgcPolarity */

	FALSE,					/* default to Single Loop AGC */
	{
		NON_INVERTED,		/* dataEnablePolarity */
		NON_INVERTED,		/* pktSyncPolarity */
		NON_INVERTED,		/* errorPolarity */
		NON_INVERTED,		/* clockPolarity */
		TRUE,				/* bGatedOutputEnable */
		TRUE,				/* bContinuousRateEnable */
		TRUE,				/* bParallelOutputEnable */
		TRUE				/* bHeaderEnable */
	},	/* mpegFormat */
	IF_INPUT_ADJACENT_ON,	/* FAT IF input ON threshold */
	IF_INPUT_ADJACENT_OFF,	/* FAT IF input OFF threshold */
	RF_INPUT_ADJACENT_ON,	/* FAT RF input ON threshold */
	RF_INPUT_ADJACENT_OFF,	/* FAT RF input OFF threshold */
#ifndef NXT2004
	44.0,					/* FDC downconverter freq */
	NXT_FDC_SYMBOL_RATE_1024,/* FDC symbol rate */
	NON_INVERTED,			/* FDC AGC polarity */
	{
		NON_INVERTED,		/* FDC decoder clock polarity */
		NON_INVERTED,		/* FDC decoder data polarity */
		INVERTED			/* FDC decoder mode */
	},	/* fdcDifdecMode */
#endif
	{
		NXT_BERT_INPUT_FAT,	/* BERT data source */
		NXT_BERT_PN_15,		/* BERT PN Sequence type */
		NON_INVERTED,		/* PN sequence polarity */
		NXT_BERT_RM_1,		/* bytecount to remove */
		0x1F				/* window size */
	},	/* bertFormat */
	{
		NXT_ENABLE,
		NXT_ENABLE,
		NXT_ENABLE,
		NXT_ENABLE,
		NXT_TRISTATE
	},	/* outputControl */
	FALSE,					/* bFatStartScriptDone */
	FALSE,					/* bFatAdjacent */
	0,						/* irqMask */
	(NxtCSHandle_t)0,		/* hCSNxtComm */
	(NxtCSHandle_t)0,		/* hCSNxtFatTrack */
#ifndef NXT2004
	(NxtCSHandle_t)0,		/* hCSNxtFdcTrack */
	FALSE,					/* AbortFdcFlag */
	(Data16*)NULL,			/* pFdcAgcData */
#endif
#ifdef NXT2005
	(Data16*)NULL,			/* pRdcUgcData */
	NXT_AUTO,				/* rdcMode */
#endif
	FALSE,					/* AbortFatFlag */
	(Data16*)NULL,			/* pFatAgcData */

	(Data16*)NULL,			/* pAgc256Qam */
	(Data16*)NULL,			/* pAgc64Qam */
	(Data16*)NULL,			/* pFatAgcAdj */

	{ 	
		0, 1, 2, 3,
		4, 5, 6, 7,
		8, 9, 10, 11,
		12, 13, 14, 15	
	},						/* nxtennaMap[16] */
	3,						/* nxtennaGain */
	POL_BIT_NOT_USED,		/* nxtennaPolBit */

	(void*)INVALID_CONTEXT,	/* *pContext */
	(void*)INVALID_CONTEXT	/* *pNext */
};

/*
******************************************************************************
Public Functions (not static)
******************************************************************************
*/

/**********************************************************************
 *
 * GetDCB
 *
 * Finds a matching DCB, or allocates a new one for future match.
 *
 * Inputs:
 *	void *pContext - NULL, or user-defined ID for multiple NXT200Xs
 *
 * Returns:
 *	NxtDCB * - pointer to a DCB, or INVALID_CONTEXT if memory allocation failed.
 *
 * Notes:
 *
 **********************************************************************/
NxtDCB *GetDCB(void *pContext) {
	NxtDCB *pDCB;
	NxtDCB *pLastDCB;

	/* attempt to find a matching DCB */
	pDCB = FindDCB(pContext, &pLastDCB);

	/* did we find one? */
	if ( pDCB == INVALID_CONTEXT ) {
		/* no match found -- is the root in use yet? */
		if ( pLastDCB->pContext == INVALID_CONTEXT ) {
			/* no -- give back the root */
			pDCB = &dcb0;
		}
		else {
			/* we're past the root, so allocate one */
			pDCB = (NxtDCB *)NxtAllocateMem(pContext, sizeof(NxtDCB));
			/* link it on the list */
			pDCB->bInitialized = FALSE;
			pDCB->bAsicInitialized = FALSE;
			pLastDCB->pNext = pDCB;
		}
		/* mark this DCB in use */
		pDCB->pContext = pContext;
		/* make it the end of the list */
		pDCB->pNext = INVALID_CONTEXT;
	}
	return pDCB;
}

/**********************************************************************
 *
 * FindDCB
 *
 * Finds a matching DCB among initialized contexts
 *
 * Inputs:
 *	void *pContext - NULL, or user-defined ID for multiple NXT200Xs
 *
 * Outputs:
 *	NxtDCB **ppLastDCB - pointer to previous DCB in single linked list
 *
 * Returns:
 *	NxtDCB * - pointer to matching DCB
 *
 **********************************************************************/
NxtDCB *FindDCB(void *pContext, NxtDCB **ppLastDCB)
{
	NxtDCB *pDCB = &dcb0;

	*ppLastDCB = INVALID_CONTEXT;
    /* iterate until we match or hit the end of the list */
	do {
		if ( pDCB->pContext == pContext ) {
			break;
		}
		else {
			*ppLastDCB = pDCB;
			pDCB = (NxtDCB*)(pDCB->pNext);
		}
	} while ( pDCB != INVALID_CONTEXT );

	return pDCB;
}


/**********************************************************************
 *
 * DeleteDCB
 *
 * Unlinks DCB from list and returns memory
 *
 * Inputs:
 *	void *pContext - NULL, or user-defined ID for multiple NXT200Xs
 *
 * Returns:
 *	void
 *
 **********************************************************************/
void DeleteDCB(void *pContext)
{
	NxtDCB *pDCB;
	NxtDCB *pPreviousDCB;

	/* attempt to find a matching DCB */
	pDCB = FindDCB(pContext, &pPreviousDCB);

	/* did we find one? */
	if ( pDCB != INVALID_CONTEXT ) {
		/* fixup the links and free the DCB */
		if ( pDCB != &dcb0 ) {   /* not the root */
			pPreviousDCB->pNext = pDCB->pNext;
			/* clean up data in case this one gets re-allocated */
			pDCB->bInitialized = FALSE;
			pDCB->bAsicInitialized = FALSE;
			pDCB->pNext = INVALID_CONTEXT;
			pDCB->pContext = INVALID_CONTEXT;
			NxtFreeMem(pContext, pDCB);
		}
		else { /* delete the root by compressing the list */
			if ( pDCB->pNext != INVALID_CONTEXT ) { /* copy the 1st dynamic DCB to the root */
				pPreviousDCB = (NxtDCB*)(pDCB->pNext); /* temp to hold for free after copy */
				memcpy(pDCB, pDCB->pNext, sizeof(NxtDCB));
				pPreviousDCB->bInitialized = FALSE;
				pPreviousDCB->bAsicInitialized = FALSE;
				pPreviousDCB->pNext = INVALID_CONTEXT;
				pPreviousDCB->pContext = INVALID_CONTEXT;
				NxtFreeMem(pDCB->pContext, pPreviousDCB);
			}
			else { /* the list is now empty */
				pDCB->bInitialized = FALSE;
				pDCB->bAsicInitialized = FALSE;
				pDCB->pNext = INVALID_CONTEXT;
				pDCB->pContext = INVALID_CONTEXT;
			}
		}
	}
}

#endif ALPSTUNER