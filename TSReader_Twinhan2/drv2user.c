#ifdef ALPSTUNER
/**********************************************************************
 * DRV2USER.C
 * User-defined source code to support DRV2004
 *
 * Copyright (C) 2002 NxtWave Communications, Inc.
 *
 * NOTE: This file must be edited to adapt the DRV2004 software to a 
 * user-specific platform and operating system.
 *
 * $Log:   //panxtbdc01/AppsEng/PVCS/Application Eval/archives/EVAL2004/Drv2004/drv2user.c-arc  $
 * 
 *    Rev 1.8   Apr 28 2003 17:18:04   raggarwa
 * Updated comments
 * 
 *    Rev 1.7   Feb 11 2003 14:48:28   raggarwa
 * Removed nested comments, for release 3.0.0
 * 
 *    Rev 1.6   Oct 01 2002 15:29:08   raggarwa
 * Updated comments
 * 
 *    Rev 1.5   Jun 18 2002 09:41:40   raggarwal
 * Alpha3 Release
 * 
 *    Rev 1.4   Mar 20 2002 13:14:38   raggarwal
 * Ended file with a carraige return
 * 
 *    Rev 1.3   Mar 08 2002 16:28:48   raggarwal
 * Alpha Release
 * 
 **********************************************************************/

/*
******************************************************************************
Includes
******************************************************************************
*/
/* Insert your headers here */
#define _WIN32_WINNT 0x400
#include <windows.h>

#include "drv2user.h"
#include "8vsbdriver/NxtCommon.h"
#include "TSReader_Twinhan2.h"


unsigned char MakeCheckSum(TunerData *pTunerData);
extern HANDLE hDevice2;


/*
******************************************************************************
Public Functions (not static)
******************************************************************************
*/

/* Notification Functions */


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
void NxtOnFatChannelLock( void *pContext )
{
} /* NxtOnFatChannelLock */


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
void NxtOnFatChannelLoss( void *pContext )
{
} /* NxtOnFatChannelLoss */

/* Operating System Interface */


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
void NxtSuspendThread( void *pContext, Data16 mSecTime )
{
	Sleep(mSecTime);
} /* NxtSuspendThread */


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
Data8 NxtInitCriticalSection( void *pContext, NxtCSHandle_t *pCs )
{
	return 0;
} /* NxtInitCriticalSection */


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
Data8 NxtDeleteCriticalSection( void *pContext, NxtCSHandle_t cs )
{
	return 0;
} /* NxtDeleteCriticalSection */


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
Data8 NxtRequestCriticalSection( void *pContext, NxtCSHandle_t cs )
{
	return 0;
} /* NxtRequestCriticalSection */


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
Data8 NxtWaitForCriticalSection( void *pContext, NxtCSHandle_t cs )
{
	return 0;
} /* NxtWaitForCriticalSection */


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
Data8 NxtLeaveCriticalSection( void *pContext, NxtCSHandle_t cs )
{
	return 0;
} /* NxtLeaveCriticalSection */


/**********************************************************************
 *
 * NxtSetHighPriority
 *
 * Called by DRV200X to raise the priority of the current thread above
 *	all other threads.
 *
 * Inputs:
 *	void *pContext - identifies which NXT200X instance called the function
 *
 * Returns:
 *	non-zero for failure
 *	0 for success
 *
 **********************************************************************/
Data8 NxtSetHighPriority( void *pContext )
{
	return 0;
} /* NxtSetHighPriority */


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
Data8 NxtSetNormalPriority( void *pContext )
{
	return 0;
} /* NxtSetNormalPriority */


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
/* NxtAllocateMemory -- MACRO defined in drv2user.h
void *NxtAllocateMemory(void *pContext, Data16 byteCount) {
}*/ /* NxtAllocateMemory */


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
/* NxtFreeMemory -- MACRO defined in drv2user.h
void NxtFreeMemory(void *pContext, void *pBuffer) {
}*/ /* NxtFreeMemory */

/* Hardware-Specific Functions */


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
 *	Data8 - the IIC dev 0 address of the referenced NXT2003 instance
 *
 **********************************************************************/
Data8 NxtGetDev0Addr( void *pContext )
{
	return 0x14;
} /* NxtGetDev0Addr */

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
Data8 NxtGetDev1Addr( void *pContext )
{
	return 0;		// zero used NxtGetDev0Addr's return address plus 2
} /* NxtGetDev1Addr */

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
Data8 iicRead(void *pContext, Data8 iicAddr, Data8 byteCount, Data8 *pBuffer )
{
	return 1;
} /* iicRead */


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
Data8 iicWrite( void *pContext, Data8 iicAddr, Data8 byteCount, Data8 *pBuffer) 
{
	return 1;
} /* iicWrite */


/**********************************************************************
 *
 * iicRegRead
 *
 * Called by DRV200X to read NXT200X registers
 *
 * Inputs:
 *	void *pContext - identifies which NXT200X instance
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


Data8 iicRegRead( void *pContext, Data8 iicAddr, Data8 regAddr, Data8 byteCount, 
				 Data8 *pBuffer )

{
	int i;
	BYTE * inBuf;
	BYTE * outBuf;
	TunerData tunerData;
	BYTE * pTunerData = (BYTE *)&tunerData;
	BYTE bBuf[256];
	int inBufLength, outBufLength;
	
	memset(&tunerData,0,sizeof(TunerData));

	for (i = 0; i < byteCount; i++)
	{
		BOOL StatusSuccess;
	    ULONG bytesReturned;

		pTunerData[0] = 0xaa;
		pTunerData[2] = 0x14;		// I2C read/write
		pTunerData[3] = 0x01;		// demod(1) tuner(0) select
		pTunerData[5] = regAddr;
		pTunerData[6] = 0;			// read
		MakeCheckSum(&tunerData);	
		inBuf = (BYTE *)&tunerData;
		inBufLength	= sizeof(tunerData);		
		outBuf = (LPVOID)bBuf;
		outBufLength = inBufLength;
		memset(outBuf, 0, outBufLength);
		
		StatusSuccess = DeviceIoControl(hDevice2, 
										(DWORD)DST_IOCTL_SET_INFO, 
										inBuf, 
										inBufLength + 1, 
										outBuf, 
										outBufLength, 
										&bytesReturned, 
										NULL);
		if (!StatusSuccess)
			return 1;
		*(pBuffer++) = outBuf[0];
	}
	
	return 0;
	return 0;
}


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
Data8 iicRegWrite( void *pContext, Data8 iicAddr, Data8 regAddr, Data8 byteCount, 
				  Data8 *pBuffer)
{
	int i;
	BYTE * inBuf;
	BYTE * outBuf;
	TunerData tunerData;
	BYTE * pTunerData = (BYTE *)&tunerData;
	BYTE bBuf[256];
	int inBufLength, outBufLength;
	
	memset(&tunerData,0,sizeof(TunerData));

	for (i = 0; i < byteCount; i++)
	{
		BOOL StatusSuccess;
	    ULONG bytesReturned;

		pTunerData[0] = 0xaa;
		pTunerData[2] = 0x14;		// I2C read/write
		pTunerData[3] = 0x01;		// demod(1) tuner(0) select
		pTunerData[5] = regAddr;
		pTunerData[6] = 1;			// write
		pTunerData[7] = *(pBuffer++);
		MakeCheckSum(&tunerData);	
		inBuf = (BYTE *)&tunerData;
		inBufLength	= sizeof(tunerData);		
		outBuf = (LPVOID)bBuf;
		outBufLength = inBufLength;
		memset(outBuf, 0, outBufLength);
		
		StatusSuccess = DeviceIoControl(hDevice2, 
										(DWORD)DST_IOCTL_SET_INFO, 
										inBuf, 
										inBufLength + 1, 
										outBuf, 
										outBufLength, 
										&bytesReturned, 
										NULL);
		if (!StatusSuccess)
			return 1;
	}
	
	return 0;
} /* iicRegWrite */


/**********************************************************************
 *
 * FatAgcData
 *
 * Pointer passed as a parameter to NxtSetFatAgcData.
 *
 * The data in the table below MUST be modified as necessary to
 * work with the user's custom AGC circuit.  NxtWave will provide
 * tables for well-known AGC circuits.  The table supplied with
 * drv2user.c is an example only, the table used for the NxtWave
 * evaluation tuner Philips 1236D.
 *
 * Format: Register, count, data, ..., data
 *
 * The table MAY include sections for NXT_AGC_SETUP_64QAM --
 *	 modifications to the standard data for 64QAM reception
 *
 * The table MAY include sections for NXT_AGC_SETUP_256QAM --
 *	 modifications to the standard data for 256QAM reception
 *
 * The table MAY include sections for NXT_FAT_AGC_SETUP_ADJ --
 *	 modifications to the standard data for VSB reception with Adjacents
 *
 * The table MUST end with NXT_AGC_SETUP_DONE
 *
 **********************************************************************/

Data16 fatAgcData[] = {

	//AGC_ADC_TARGET_POWER_LEVEL, 1, 0x6F,		// set ADC target level 
	AGC_ADC_TARGET_POWER_LEVEL, 1, 0x70,		// set ADC target level 
	//AGC_SDM_CONFIGURE, 1, 0x87,					// sdm1 inverted, all others disabled 
	AGC_SDM_CONFIGURE, 1, 0x07,					// sdm1 inverted, all others disabled 
	AGC_SDM1_INPUT_MSB, 2, 0x10, 0x00,			// SDM1 at mid range before update 
	AGC_SDMX_INPUT_MSB, 2, 0x69, 0x00,			// SDMX  
	AGC_ADC_POWER_LPF_FC, 1, 0x05,				// adc_power_lpf 
	AGC_GAIN_LOOP_BANDWIDTH, 1, 0x18,			// agc gain/distribution loop bw 
	//AGC_GAIN_LOOP_BANDWIDTH, 2, 0x00, 0x00,		// agc gain/distribution loop bw 

	AGC_ACCUMULATOR2_MSB, 2, 0x80, 0x00,		
	//AGC_KG1, 1, 0x00,							// k1 for gain loop 
	AGC_KG1, 1, 0x10,							// k1 for gain loop 
	AGC_SDM12_LPF_FC, 1, 0x44,					// sdm1 lpf  
	AGC_CONTROL, 1, 0x04,						// START AGC 

	NXT_FAT_AGC_SETUP_ADJ,						// optimized for VSB w/N-1 Adjacent 
	AGC_ADC_TARGET_POWER_LEVEL, 1, 0x70,		// set ADC target level 
	AGC_SDMX_INPUT_MSB, 2, 0x75, 0x00,			// SDMX  

	NXT_AGC_SETUP_64QAM,						// optimized for 64QAM 
	AGC_ADC_TARGET_POWER_LEVEL, 1, 0x74,		// set ADC target level 
	AGC_SDMX_INPUT_MSB, 2, 0x68, 0x00,			// SDMX  

	NXT_AGC_SETUP_256QAM,						// optimized for 256QAM 
	AGC_ADC_TARGET_POWER_LEVEL, 1, 0x74,		// set ADC target level 
	AGC_SDMX_INPUT_MSB, 2, 0x64, 0x00,			// SDMX  

	NXT_AGC_IF_THRESHOLD,						// IF thresholds for tuner 
	0xA8, 0xF5,									// adj ON threshold, broken into 2 bytes, MSB first 
	0x7A, 0xE0,									// adj OFF threshold 

	NXT_AGC_SETUP_DONE							// EOF 
};

#endif ALPSTUNER
