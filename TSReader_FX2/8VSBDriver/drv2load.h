/**********************************************************************
 * DRV2LOAD.H
 * Header file for drv2load.c
 *
 * Copyright (C) 2002 NxtWave Communications, Inc.
 *
 * $Log:   V:/Applications EVAL Projects/archives/EVAL2005/Drv2005/drv2load.h-arc  $
 * 
 *    Rev 1.3   May 17 2002 16:06:00   raggarwal
 * Alpha2 Release
 * 
 *    Rev 1.1   Mar 08 2002 16:29:30   raggarwal
 * Alpha Release
 * 
 **********************************************************************/

#ifndef DRV2LOAD_H
#define DRV2LOAD_H

/*
******************************************************************************
Defines
******************************************************************************
*/



/*
******************************************************************************
Public Types
******************************************************************************
*/

typedef struct {
	Data16	byteCount;	/* byte count of RAM code */
	Bool	bEnableROM;	/* indicates if ROM enable required */
	Data8  *pBuffer;	/* pointer to RAM code */
} NxtCodeDescriptor_s;


/*
******************************************************************************
Public Data
******************************************************************************
*/
extern Data8 defaultCode[];
extern NxtCodeDescriptor_s defaultCodeDscp;


#endif


