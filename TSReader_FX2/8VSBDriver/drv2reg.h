/**********************************************************************
 * DRV2REG.H
 * Include File for DRV2REG.C -- register interface functions
 *
 * Copyright (C) 2002 NxtWave Communications, Inc.
 *
 * $Log:   //panxtbdc01/AppsEng/PVCS/Application Eval/archives/EVAL2005/Drv2005/Drv2reg.h-arc  $ 
 * 
 *    Rev 1.17   Feb 28 2003 13:43:18   raggarwa
 * Added a function for sending RDC taps
 * 
 *    Rev 1.16   Sep 05 2002 11:49:42   raggarwa
 * Merged main trunk and nxtenna
 * 
 *    Rev 1.15.1.0   Jun 27 2002 17:33:32   raggarwal
 * Added regNxtennaSvc( )
 * 
 *    Rev 1.15   Jun 17 2002 11:07:12   raggarwal
 * Alpha3 Release
 * 
 *    Rev 1.13   May 17 2002 16:06:18   raggarwal
 * Alpha2 Release
 * 
 *    Rev 1.4   Mar 08 2002 16:28:48   raggarwal
 * Alpha Release
 * 
 **********************************************************************/

#ifndef DRV2REG_H
#define DRV2REG_H


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
Data16 regInitASIC(NxtDCB *pDCB);

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
 * Returns:
 *	NXT_NO_ERROR
 *	NXT_ERR_COMM
 *
 **********************************************************************/
Data16 regStart(NxtDCB *pDCB,
			NxtAcqOptions_t startOptions,
			Data16 *result);

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
Data16 regStop(NxtDCB *pDCB, 
			NxtAcqOptions_t stopOptions);


/**********************************************************************
 *
 * regAgcSetup
 *
 * Runs AGC setup script
 *
 * Inputs:
 *	NxtDCB	*pDCB - pointer to current DCB -- ASSUMED FOUND!!!
 *	Data16	*pSetupData - pointer to setup script data
 *
 * Returns:
 *	(Data16)Data16 
 *
 **********************************************************************/
Data16 regAgcSetup( NxtDCB *pDCB,
			Data16 *pSetupData );

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
 *	Data16 *pResult - pointer to startOptions that were successful
 *
 * Returns:
 *	NXT_NO_ERROR
 *	NXT_ERR_COMM
 *
 **********************************************************************/
Data16 regConfig(NxtDCB *pDCB,
			NxtAcqOptions_t startOptions,
			Data16 *pResult);


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
			Data16 *pResult);

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
void regActiveTracking(NxtDCB *pDCB, 
		Bool bTrackFat);


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
			Bool *pLockStatus);


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
			Bool *pFdcLockStatus);


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
 *	Bool *pErrCount - number of errors accumulated in 50 mS
 *
 * Returns:
 *	NXT_NO_ERROR
 *	NXT_ERROR_COMM
 *
 **********************************************************************/
Data16 regGetRSErrors(NxtDCB *pDCB,
			Data8 *pErrCount);


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
Data16 regNotifyFatLockLoss(NxtDCB *pDCB);

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
Data16 regNotifyFdcLockLoss(NxtDCB *pDCB);

/**********************************************************************
 *
 * startFatAcquisition
 *
 * Sets NXT200X for desired in-band acquisition mode and releases Stop bit.
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
			Data8 ucGp0);

/**********************************************************************
 *
 * startFdcAcquisition
 *
 * Sets NXT200X for QPSK acquisition mode and releases Stop bit.
 *
 * Inputs:
 *	NxtDCB *pDCB - pointer to current DCB -- ASSUMED FOUND!!!
 *
 * Returns:
 *	NXT_NO_ERROR
 *	NXT_ERR_COMM
 *
 **********************************************************************/
Data16 startFdcAcquisition(NxtDCB *pDCB);

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
Data16 stopFatAcquisition(NxtDCB *pDCB);

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
Data16 stopFdcAcquisition(NxtDCB *pDCB);
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
							Data16 *pStatus );

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
			Data8 *pBuffer);


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
			Data8 *pBuffer);

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
Data16 resetMicro(NxtDCB *pDCB);


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
Data16 unresetMicro(NxtDCB *pDCB);

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
			Data16 *pFatSQI);

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
 *	double *pFdcSQI - register data copied to user-provided buffer
 *
 * Returns:
 *	NXT_NO_ERROR
 *	NXT_ERR_COMM
 *
 **********************************************************************/
Data16 getFdcSQI(NxtDCB *pDCB,
			double *pFdcSQI);
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
			Data32 *pFatPilotOffset);

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
			Data32 *pCarrierOffset);

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
			Data32 *pCarrierOffset);
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
Data16 getFatTimingOffset(NxtDCB *pDCB,
			Data32 *pFatTimingOffset);

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
Data16 getFdcTimingOffset(NxtDCB *pDCB,
			Data32 *pFdcTimingOffset);
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
 *
 **********************************************************************/
Data16 getFatModFormat(NxtDCB *pDCB,
			NxtModFormat_t *pFatModFormat);


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
			NxtCoreControl_t coreControl);

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
			Data8 *pBuffer);

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
Data16 enableRomFdc(NxtDCB *pDCB);
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
Data16 getRamBase(NxtDCB *pDCB, 
			Data16 *pRamBase);

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
Data16 writeFdcIF(NxtDCB *pDCB);

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
			NxtPolarity_t decoderMode);
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
Data16 regGetBertData(NxtDCB *pDCB, 
			Data32 *pErrorCount, 
			Bool bWait);


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
Data16 regConfigSmoother(NxtDCB *pDCB, 
			NxtModFormat_t mode);

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
 *	
 *
 **********************************************************************/
Data16 regSendRdcTaps(NxtDCB *pDCB, 
			NxtRdcAlpha_t alpha);
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
					 Data8 bytes);

#endif

