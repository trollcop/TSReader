/**********************************************************************
 * DRV2USER.H
 * User-defined data types and functions for DRV2004
 *
 * Copyright (C) 2002 NxtWave Communications, Inc.
 *
 * $Log:   //panxtbdc01/AppsEng/PVCS/Application Eval/archives/EVAL2004/Include/drv2user.h-arc  $
 * 
 *    Rev 1.9   Nov 07 2002 18:23:58   raggarwa
 * Changed smoother bandwidth for QAM to reduce jitter, PR 48/2004
 * 
 *    Rev 1.8   Oct 01 2002 14:29:16   raggarwa
 * Updated comments
 * 
 *    Rev 1.7   Jun 18 2002 09:33:56   raggarwal
 * Alpha3 Release
 * 
 *    Rev 1.6   May 17 2002 16:32:20   raggarwal
 * Alpha2 Release
 * 
 *    Rev 1.4   Mar 08 2002 16:25:36   raggarwal
 * Alpha Release
 * 
 **********************************************************************/

#ifndef DRV2USER_H
#define DRV2USER_H

/*
******************************************************************************
Defines
******************************************************************************
*/
#ifndef TRUE
	#define TRUE	1
#endif
#ifndef FALSE
	#define FALSE	0
#endif
#ifndef NULL
	#define NULL	0
#endif

/* MACRO-based definitions for memory alloc/free */
#define NxtAllocateMem(pContext, byteCount)		malloc(byteCount)
#define NxtFreeMem(pContext, pBuffer)			free(pBuffer)


/* Adjacent channel detection thresholds -- see API manual for description */
#define IF_INPUT_ADJACENT_ON	0xA8F5	/* > 66% of full scale (0xFFFF) */
#define IF_INPUT_ADJACENT_OFF	0x7AE0	/* < 48% of full scale */
#define RF_INPUT_ADJACENT_ON	0xB332	/* < 70% of full scale */
#define RF_INPUT_ADJACENT_OFF	0xE665	/* > 90% of full scale */

/* Power values used for FAT signal status */
#define FAT_POWER_THRESHOLD 0x0000	/* slightly more than value with no input */
#define FAT_POWER_BAUD		0x0001	/* lowest value with stable timing */
#define FAT_POWER_MID		0x0002	/* about half way to frame lock */
#define FAT_POWER_LOCK		0x0003	/* lowest value with frame lock */

/* SQI values used for FAT signal status */
#define FAT_SQI_MODERATE		0x77F8	/* 10 dB - just get lock */
#define FAT_SQI_STRONG			0x7D72	/* 15 dB - stop getting any errors */
#define FAT_SQI_VERY_STRONG		0x7FAE	/* 24 dB - very strong signal (adjust to taste) */
#define FAT_SQI_MAX				0x7FFF	/* you can never get this */

/* BERT synchronizer Error, Acq and Loss thresholds */
#define NXT_BERT_SYNC_THOLD		0x01	/* 1% */
#define NXT_BERT_SYNC_LOSS		0x03	/* corresponds to 3 windows */
#define NXT_BERT_SYNC_ACQ		0x30	/* corresponds to 3 windows */

/* MPEG output smoother settings */
#define SMOOTHER_BUF_FILL_LEVEL_VSB	224		/* bytes - 0 to 510, in multiples of 2 */
#define	SMOOTHER_BUF_FILL_LEVEL_QAM	192		/* bytes - 0 to 510, in multiples of 2 */
#define SMOOTHER_ZETA				0x10	/* corresponds to z=0.75 */
#define SMOOTHER_BW_VSB				0x01	/* corresponds to n=17, w_n=1.50 Hz */
#define SMOOTHER_BW_QAM				0x04	/* corresponds to n=14, w_n= */

/*
******************************************************************************
Public Types
******************************************************************************
*/
typedef unsigned char	Data8;
typedef int				Data16;
typedef int				Data32;
typedef	long			Data64;
typedef int				Bool;

typedef void*			NxtCSHandle_t;

/*
******************************************************************************
Public Data
******************************************************************************
*/
extern Data16 fatAgcData[];

/*
******************************************************************************
Public Functions
******************************************************************************
*/

/**********************************************************************
 *
 * NxtOnFatChannelLock
 *
 * Called by DRV200X on detection of a Lock condition
 *
 * Inputs:
 *	void *pContext - identifies which NXT200X instance called the function
 *
 * Notes:
 *	This is a callback function that is called from NxtStart().  The 
 *	user may define any action that should take place when lock is 
 *	detected.
 *
 **********************************************************************/
void NxtOnFatChannelLock( void *pContext );

/**********************************************************************
 *
 * NxtOnFatChannelLoss
 *
 * Called by DRV200X on detection of a Loss-Of-Lock condition
 *
 * Inputs:
 *	void *pContext - identifies which NXT200X instance called the function
 *
 * Notes:
 *	This is a callback function that is called from NxtStart().  The 
 *	user may define any action that should take place when loss is
 *	detected.
 *
 **********************************************************************/
void NxtOnFatChannelLoss( void *pContext );


/**********************************************************************
 *
 * NxtSuspendThread
 *
 * Called by DRV200X to insert a delay in the current thread
 *
 * Inputs:
 *	void *pContext - identifies which NXT200X instance called the function
 *	Data16 mSecTime - number of milliseconds to sleep
 *
 * Notes:
 *	This should be implemented as an operating system "sleep" with
 *	1 mS resolution.
 *
 **********************************************************************/
void NxtSuspendThread( void *pContext, Data16 mSecTime );


/**********************************************************************
 *
 * NxtInitCriticalSection
 *
 * Called by DRV200X to create a mutual exclusion object
 *
 * Inputs:
 *	void *pContext - identifies which NXT200X instance called the function
 *
 * Outputs:
 *	NxtCSHandle_t *pCs - handle assigned to the new critical section object
 *
 * Returns:
 *	non-zero for failure
 *	0 for success
 *
 **********************************************************************/
Data8 NxtInitCriticalSection( void *pContext, NxtCSHandle_t *pCs );


/**********************************************************************
 *
 * NxtDeleteCriticalSection
 *
 * Called by DRV200X to delete a mutual exclusion object
 *
 * Inputs:
 *	void *pContext - identifies which NXT200X instance called the function
 *	NxtCSHandle_t cs - handle of the critical section object to be deleted
 *
 * Returns:
 *	non-zero for failure
 *	0 for success
 *
 **********************************************************************/
Data8 NxtDeleteCriticalSection( void *pContext, NxtCSHandle_t cs );

/**********************************************************************
 *
 * NxtRequestCriticalSection
 *
 * Called by DRV200X on entry to a critical section, when no blocking
 *	is allowed.
 *
 * Inputs:
 *	void *pContext - identifies which NXT200X instance called the function
 *	NxtCSHandle_t cs - handle of the critical section being requested
 *
 * Returns:
 *	non-zero for failure -- critical section is currently in use
 *	0 for success -- critical section not held by another thread
 *
 * Notes:
 *	This function must return immediately, indicating whether or not
 *	the calling function has exclusive access to the critical section.
 *	Implement as a critical section entry with no wait.
 *
 **********************************************************************/
Data8 NxtRequestCriticalSection( void *pContext, NxtCSHandle_t cs );

/**********************************************************************
 *
 * NxtWaitForCriticalSection
 *
 * Called by DRV200X on entry to a critical section, when blocking
 *	is required.
 *
 * Inputs:
 *	void *pContext - identifies which NXT200X instance called the function
 *	NxtCSHandle_t cs - handle of the critical section being requested
 *
 * Returns:
 *	non-zero for failure
 *	0 for success
 *
 * Notes:
 *	This function must block indefinitely, until the requested critical
 *	section is available.
 *	Implement as a critical section entry with infinite wait.
 *
 **********************************************************************/
Data8 NxtWaitForCriticalSection(void *pContext, NxtCSHandle_t cs );

/**********************************************************************
 *
 * NxtLeaveCriticalSection
 *
 * Called by DRV200X on exit from a critical section
 *
 * Inputs:
 *	void *pContext - identifies which NXT200X instance called the function
 *	NxtCSHandle_t cs - handle of the critical section being released
 *
 * Returns:
 *	non-zero for failure
 *	0 for success
 *
 **********************************************************************/
Data8 NxtLeaveCriticalSection(void *pContext, NxtCSHandle_t cs );

/**********************************************************************
 *
 * NxtSetHighPriority
 *
 * Called by DRV200X to raise the priority of the current thread above
 * all other threads.
 *
 * Inputs:
 *	void *pContext - identifies which NXT200X instance called the function
 *
 * Returns:
 *	non-zero for failure
 *	0 for success
 *
 **********************************************************************/
Data8 NxtSetHighPriority(void *pContext );


/**********************************************************************
 *
 * NxtSetNormalPriority
 *
 * Called by DRV200X to restore the normal priority of the DRV200X
 *	thread.
 *
 * Inputs:
 *	void *pContext - identifies which NXT200X instance called the function
 *
 * Returns:
 *	non-zero for failure
 *	0 for success
 *
 **********************************************************************/
Data8 NxtSetNormalPriority(void *pContext );


/**********************************************************************
 *
 * NxtAllocateMem
 *
 * Called by DRV200X to dynamically allocate memory.
 *
 * Inputs:
 *	void *pContext - identifies which NXT200X instance called the function
 *	Data16 byteCount - number of bytes of memory to allocate
 *
 * Returns:
 *	void* - pointer to allocated memory, NULL for failure
 *
 * Notes:
 *	This function is only used in DRV2CNTX.C to allocate device control
 *	blocks for multiple NXT200x instances.
 *
 **********************************************************************/
/* NxtAllocateMem -- MACRO defined above
void *NxtAllocateMem(void *pContext, Data16 byteCount);
*/


/**********************************************************************
 *
 * NxtFreeMem
 *
 * Called by DRV200X to release dynamically allocated memory.
 *
 * Inputs:
 *	void *pContext - identifies which NXT200X instance called the function
 *	void *pBuffer - pointer to allocated memory to be released
 *
 * Notes:
 *	This function is only used in DRV2CNTX.C to release device control
 *	blocks for multiple NXT200x instances.
 *
 **********************************************************************/
/* NxtFreeMem -- MACRO defined above
void NxtFreeMem(void *pContext, void *pBuffer);
*/


/**********************************************************************
 *
 * NxtGetDev0Addr
 *
 * Called by DRV200X to learn the IIC address of device 0 of the referenced ASIC
 *
 * Inputs:
 *	void *pContext - identifies which NXT200X instance
 *
 * Returns:
 *	Data8 - the IIC dev 0 address of the referenced NXT200X instance
 *
 **********************************************************************/
Data8 NxtGetDev0Addr(void *pContext );


/**********************************************************************
 *
 * NxtGetDev1Addr
 *
 * Called by DRV200X to learn the IIC address of device 1 of the referenced ASIC
 *
 * Inputs:
 *	void *pContext - identifies which NXT200X instance
 *
 * Returns:
 *	Data8 - the IIC dev 1 address of the referenced NXT200X instance
 *
 **********************************************************************/
Data8 NxtGetDev1Addr(void *pContext );


/**********************************************************************
 *
 * iicRead (optional)
 *
 * Called by tuner control software
 *
 * Inputs:
 *	void *pContext - identifies which NXT200X instance
 *	Data8 iicAddr - device address
 *	Data8 byteCount - number of bytes to read
 *
 * Outputs:
 *	Data8 *pBuffer - read data bytes are copied to this buffer
 *
 * Returns:
 *	0 for success
 *	non-zero for failure
 *
 **********************************************************************/
Data8 iicRead(void *pContext,
		Data8 iicAddr, 
		Data8 byteCount, 
		Data8 *pBuffer );


/**********************************************************************
 *
 * iicWrite (optional)
 *
 * Called by tuner control software
 *
 * Inputs:
 *	void *pContext - identifies which NXT200X instance
 *	Data8 iicAddr - device address
 *	Data8 byteCount - number of bytes to write
 *	Data8 *pBuffer - write data bytes are copied from this buffer
 *
 * Returns:
 *	0 for success
 *	non-zero for failure
 *
 **********************************************************************/
Data8 iicWrite(void *pContext,
		Data8 iicAddr, 
		Data8 byteCount, 
		Data8 *pBuffer);


/**********************************************************************
 *
 * iicRegRead
 *
 * Called by DRV200X to read NXT200X registers
 *
 * Inputs:
 *	void *pContext - identifies which NXT200X instance
 *	Data8 iicAddr - device address
 *	Data8 regAddr - NXT200X register to read
 *	Data8 byteCount - number of bytes to read
 *
 * Outputs:
 *	Data8 *pBuffer - read data bytes are copied to this buffer
 *
 * Returns:
 *	0 for success
 *	non-zero for failure
 *
 **********************************************************************/
Data8 iicRegRead(void *pContext,
		Data8 iicAddr, 
		Data8 regAddr, 
		Data8 byteCount, 
		Data8 *pBuffer );


/**********************************************************************
 *
 * iicRegWrite
 *
 * Called by DRV200X to write to NXT200X registers
 *
 * Inputs:
 *	void *pContext - identifies which NXT200X instance
 *	Data8 iicAddr - device address
 *	Data8 regAddr - NXT200X register to write
 *	Data8 byteCount - number of bytes to write
 *	Data8 *pBuffer - write data bytes are copied from this buffer
 *
 * Returns:
 *	0 for success
 *	non-zero for failure
 *
 **********************************************************************/
Data8 iicRegWrite(void *pContext,
		Data8 iicAddr, 
		Data8 regAddr, 
		Data8 byteCount, 
		Data8 *pBuffer);
#endif

