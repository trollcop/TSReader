/**********************************************************************
 * DRV2CNTX.H
 * Include File for DRV2CNTX.C
 *
 * Copyright (C) 2002 NxtWave Communications, Inc.
 *
 * $Log:   //panxtbdc01/AppsEng/PVCS/Application Eval/archives/EVAL2005/Drv2005/drv2cntx.h-arc  $
 * 
 *    Rev 1.13   Sep 05 2002 11:49:34   raggarwa
 * Merged main trunk and nxtenna
 * 
 *    Rev 1.12.1.0   Jun 27 2002 17:31:34   raggarwal
 * Added Nxtenna settings
 * 
 *    Rev 1.12   Jun 17 2002 10:53:04   raggarwal
 * Alpha3 Release
 * 
 * 
 *    Rev 1.10   May 17 2002 16:05:08   raggarwal
 * Alpha2 Release
 * 
 *    Rev 1.3   Mar 08 2002 16:28:50   raggarwal
 * Alpha Release
 * 
 **********************************************************************/

#ifndef DRV2CNTX_H
#define DRV2CNTX_H

/*
******************************************************************************
Defines
******************************************************************************
*/
#define INVALID_CONTEXT ((NxtDCB *)(-1))

/*
******************************************************************************
Public Types
******************************************************************************
*/
/* FAT AGC Polarities */
typedef struct {
	NxtPolarity_t	sdm1_pol;
	NxtPolarity_t	sdm2_pol;
	NxtPolarity_t	sdmX_pol;
	NxtPolarity_t	sdmA_pol;
} NxtFatAgcPol_t;

/* MPEG Format */
typedef struct {
	NxtPolarity_t dataEnablePolarity;
	NxtPolarity_t pktSyncPolarity;
	NxtPolarity_t errorPolarity;
	NxtPolarity_t clockPolarity;
	Bool bGatedOutputEnable;
	Bool bContinuousRateEnable;
	Bool bParallelOutputEnable;
	Bool bHeaderEnable;
} NxtMpegFormat_t;

/* BERT settings */
typedef struct {
	NxtBertSource_t bertSource;
	NxtBertPnSequence_t bertAlgorithm;
	NxtPolarity_t PNSequence;
	NxtBertHeaderRemove_t removeBytes;
	Data8 winSize;
} NxtBertFormat_t;

typedef struct {
	NxtControl_t	pgaOutput;
	NxtControl_t	fdcAgcOutput;
	NxtControl_t	fatAgcOutput;
	NxtControl_t	mpegOutput;
	NxtControl_t	podOutput;
} NxtOutputControl_t;

#ifndef NXT2004
/* FDC differential decoder settings */
typedef struct {
	NxtPolarity_t clockPolarity;
	NxtPolarity_t dataPolarity;
	NxtPolarity_t decoderMode;
} NxtFdcDifdecMode_t;
#endif

/* NXT200X Device Control Block */
typedef struct {
	Bool				bInitialized;
	Bool				bAsicInitialized;
	Data8				nxtDev0Addr;
	Data8				nxtDev1Addr;
	NxtFatAgcPol_t		fatAgcPolarity;
	Bool				bDualAgcLoop;
	NxtMpegFormat_t		mpegFormat;
	Data16				fatIfAdjOn;		/* FAT IF input ON threshold */
	Data16				fatIfAdjOff;	/* FAT IF input OFF threshold */
	Data16				fatRfAdjOn;		/* FAT RF input ON threshold */
	Data16				fatRfAdjOff;	/* FAT RF input OFF threshold */
#ifndef NXT2004
	double				fdcIFfreq;		/* FDC IF freq */
	NxtFdcSymbolRate_t	symbolRate;		/* FDC symbol rate */
	NxtPolarity_t		fdcAgcPolarity;	/* FDC AGC polarity */
	NxtFdcDifdecMode_t	fdcDifdecMode;	/* FDC differential decoder mode */
#endif
	NxtBertFormat_t		bertFormat;		/* user configurable BERT settings */
	NxtOutputControl_t	outputControl;
	Bool				bFatStartScriptDone;
	Bool				bFatAdjacent;
	Data8				irqMask;	
	NxtCSHandle_t       hCSNxtComm;
	NxtCSHandle_t       hCSNxtFatTrack;	/* active tracking semaphore for FAT */
#ifndef NXT2004
	NxtCSHandle_t		hCSNxtFdcTrack;	/* active tracking semaphore for FDC */
	Bool				bAbortFdcFlag;	/* abort flag for FDC */
	Data16				*pFdcAgcData;	/* out of band AGC setup script */
#endif
#ifdef NXT2005
	Data16				*pRdcUgcData;	/* RDC UGC configuration data */
	NxtControlMode_t	rdcMode;		/* RDC automatic/manual mode */
#endif
	Bool				bAbortFatFlag;	/* abort flag for FAT */
	Data16				*pFatAgcData;	/* standard VSB AGC setup script */
	Data16				*pAgc256Qam;	/* 256 QAM AGC setup modifications */
	Data16				*pAgc64Qam;		/* 64 QAM AGC setup modifications */
	Data16				*pFatAgcAdj;	/* Adjacent channel AGC setup modifications */

	NxtennaMapping_t	nxtennaMap[16];	/* map from 16 positions to 4-bit values */
	Data8				nxtennaGain;	/* number of gain settings */ 
	NxtennaPolBit_t		nxtennaPolBit;	/* Nxtenna polarization bit */

	void                *pContext;		/* identifies the context, i.e. instance of this device */
	void                *pNext;			/* implements a single-linked list of DCBs */
} NxtDCB;

/*
******************************************************************************
Public Functions
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
NxtDCB *GetDCB(void *pContext);


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
NxtDCB *FindDCB(void *pContext, NxtDCB **ppPrevious);


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
void DeleteDCB(void *pContext);

#endif

