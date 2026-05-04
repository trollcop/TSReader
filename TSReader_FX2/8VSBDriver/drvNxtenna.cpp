#ifdef ALPSTUNER
/**********************************************************************
 * NXTENNA.C
 * Public Interface to Nxtenna
 *
 * Copyright (C) 2002 NxtWave Communications, Inc.
 *
 * $Log:   //panxtbdc01/AppsEng/PVCS/Application Eval/archives/EVAL2005/Drv2005/drvNxtenna.c-arc  $
 * 
 *    Rev 1.3   Oct 16 2002 11:24:54   raggarwa
 * Updated comments
 * 
 *    Rev 1.2   Aug 14 2002 14:27:38   reichgot
 * Changed mapping in getConfig and setConfig
 * 
 *    Rev 1.0   Jun 27 2002 17:34:50   raggarwal
 * Initial revision.
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
 * NxtennaEnableDriver
 *
 * Enables or disables the EIA/CEA-909 Driver in the Nxt200x
 *
 * Inputs:
 *	void *pContext	- NULL, or user-defined ID for multiple NXT200Xs
 *	Bool bEnable	- TRUE to enable driver, FALSE to disable (default)
 *
 * Returns:
 *	Data16
 *
 * Notes:
 *
 **********************************************************************/
DRV2_API Data16 NxtennaEnableDriver(void *pContext, 
					Bool bEnable) {
	
	Data16	retVal = NXT_NO_ERROR;
	NxtDCB *pDCB;
	Data8	uc_gp4, uc_gp5, uc_gp6;
	Data8	bytes;

	/* find or create the referenced DCB */
	pDCB = GetDCB(pContext);

	if (pDCB == INVALID_CONTEXT) {	
		/* memory allocation error */
		retVal = NXT_ERR_MEMORY;
	}
	else {
		uc_gp4 = UC_NXTENNA_WRITE;
		uc_gp5 = NXTENNA_ENABLE_DRIVER;
		bytes  = 1;

		/* write to UC_GP_6 */
		if (bEnable) {
			uc_gp6 = NXTENNA_ENABLE;
		} else {
			uc_gp6 = NXTENNA_DISABLE;
		}
		retVal = regNxtennaSvc(pDCB, uc_gp4, uc_gp5, &uc_gp6, 1);
	}
	return retVal;

}/* NxtennaEnableDriver */

/**********************************************************************
 *
 * NxtennaSetConfig
 *
 * Configures antenna specifics in the NXT200x
 *
 * Inputs:
 *	void *pContext			- NULL, or user-defined ID for multiple NXT200Xs
 *
 *	NxtennaMapping_t *pMap	- points to mapping from logical to physical
 *							position settings. Default is plain binary
 *							mapping to 16 clockwise positions.
 *
 *	Data8 gainSettings		- range 0..3 to indicate 1..4 possible gain 
 *							settings. Default is 4 settings.
 *
 *	NxtennaPolBit_t polBit	- determines the use of the polarization bit in the 
 *							909 interface. Default is POL_BIT_NOT_USED.
 *
 * Returns:
 *	Data16
 *
 * Notes: 
 *	The use of this API is optional.  If this API is never called
 *	then the default configuration is assumed.  If this API is called
 *	then all arguments must have valid values.
 *
 **********************************************************************/
DRV2_API Data16 NxtennaSetConfig(void *pContext,
					NxtennaMapping_t *pMap,
					Data8 gainSettings,
					NxtennaPolBit_t polBit) {
	
	Data16	retVal = NXT_NO_ERROR;
	NxtDCB *pDCB;
	Data8	uc_gp4,	uc_gp5,	map[9];
	Data8	i;

	/* find or create the referenced DCB */
	pDCB = GetDCB(pContext);

	if (pDCB == INVALID_CONTEXT) {	
		/* memory allocation error */
		retVal = NXT_ERR_MEMORY;
	}
	else {

		/* specify the data_xfer service */
		uc_gp4 = UC_NXTENNA_WRITE;

		/* write to UC_GP_5 */
		uc_gp5 = NXTENNA_ANT_CONFIG;

		/* write to UC_GP_6 */
		/*
		map[0]  = *(pMap + 0) & 0x0F;
		map[0] |= *(pMap + 1) & 0x0F;

		map[1]  = *(pMap + 2) & 0x0F;
		map[1] |= *(pMap + 3) & 0x0F;

		map[2]  = *(pMap + 4) & 0x0F;
		map[2] |= *(pMap + 5) & 0x0F;
		
		map[7]  = *(pMap +14) & 0x0F;
		map[7] |= *(pMap +15) & 0x0F;
		*/

		for(i=0; i<8; i++) {
			map[i]  = (*(pMap + 2*i) & 0x0F)<<4;
			map[i] |= *(pMap + 2*i+1) & 0x0F;
		}

		/* write to UC_GP_14 */
		map[8] = (gainSettings & 0x03) << 6;
		map[8] |= (polBit << 3);
		map[8] &= ~0x07;
	
		retVal = regNxtennaSvc(pDCB, uc_gp4, uc_gp5, map, 9);
		
		/* save in DCB */
		pDCB->nxtennaGain = gainSettings;
		pDCB->nxtennaPolBit = polBit;
		for(i=0; i<16; i++)
			pDCB->nxtennaMap[i] = *(pMap + i);

	}	
	return retVal;

}/* NxtennaSetConfig */



/**********************************************************************/
/*                          Manual Control APIs                       */
/**********************************************************************/


/**********************************************************************
 *
 * NxtennaSetSetting
 *
 * Sets position, gain, polarization, and channel number manually.
 *
 * Inputs:
 *	void *pContext	- NULL, or user-defined ID for multiple NXT200Xs
 *	Data8 position	- 4 significant bits (range 0-15)
 *	Data8 gain		- 2 significant bits (range 0-3)
 *	Data8 pol		- 1 significant bit (0 or 1)
 *	Data8 channel	- 7 significant bits (range 0-127)
 *
 * Returns:
 *	Data16
 *
 * Notes:
 *	No range checking is performed on settings, they are simply
 *	bit-packed to form a 14-bit EIA/CEA-909 message.
 *
 **********************************************************************/
DRV2_API Data16 NxtennaSetSetting(void *pContext,
					Data8 position,
					Data8 gain,
					Data8 pol,
					Data8 channel) {
	
	Data16	retVal = NXT_NO_ERROR;
	NxtDCB *pDCB;
	Data8	uc_gp4,	uc_gp5,	setting[2];

	/* find or create the referenced DCB */
	pDCB = GetDCB(pContext);

	if (pDCB == INVALID_CONTEXT) {	
		/* memory allocation error */
		retVal = NXT_ERR_MEMORY;
	}
	else {
		
		/* specify the data_xfer service */
		uc_gp4 = UC_NXTENNA_WRITE;

		/* write to UC_GP_5 */
		uc_gp5 = NXTENNA_ANT_SETTING;

		/* write to UC_GP_6 */
		setting[0] = (position & 0x0F) << 2;
		setting[0]|= (pol & 0x01) << 1;
		setting[0]|= (gain & 0x03) >> 1;
		setting[1] = (gain & 0x03) << 7;
		setting[1]|= (channel & 0x7F);

		retVal = regNxtennaSvc(pDCB, uc_gp4, uc_gp5, setting, 2);
	}
	return retVal;

}/* NxtennaSetSetting */


/**********************************************************************/
/*                              Query APIs                            */
/**********************************************************************/

/**********************************************************************
 *
 * NxtennaGetConfig
 *
 * Reads antenna configuration from the NXT200x
 *
 * Inputs:
 *	void *pContext			- NULL, or user-defined ID for multiple NXT200Xs
 *
 * Outputs:
 *	NxtennaMapping_t *pMap	- points to mapping from logical to physical
 *							position settings.
 *
 *	Data8 *pGainSettings	- range 0..3 to indicate 1..4 possible gain 
 *							settings.
 *
 *	NxtennaPolBit_t *pPolBit	- determines the use of the polarization bit in the 
 *							909 interface.
 *
 * Returns:
 *	Data16
 *
 *
 **********************************************************************/
DRV2_API Data16 NxtennaGetConfig(void *pContext,
				  NxtennaMapping_t *pMap,
				  Data8 *pGainSettings,
				  NxtennaPolBit_t *pPolBit) {
	
	Data16	retVal = NXT_NO_ERROR;
	NxtDCB *pDCB;
	Data8	uc_gp4, uc_gp5,	map[9];
	Data8	i;

	/* find or create the referenced DCB */
	pDCB = GetDCB(pContext);

	if (pDCB == INVALID_CONTEXT) {	
		/* memory allocation error */
		retVal = NXT_ERR_MEMORY;
	}
	else {
		
		/* specify the data_xfer service */
		uc_gp4 = UC_NXTENNA_READ;

		/* write to UC_GP_5 */
		uc_gp5 = NXTENNA_ANT_CONFIG;

		retVal = regNxtennaSvc(pDCB, uc_gp4, uc_gp5, map, 9);

		if (NXT_NO_ERROR == retVal) {

			/* decrypt UC_GP_6..13 */
			for(i=0; i<8; i++) {
				pMap[2*i] = (map[i] & 0xF0) >> 4;
				pMap[2*i+1] = map[i] & 0x0F;
			}
			/* decrypt UC_GP_14 */
			map[8] &= ~0x07;
			*pPolBit = (NxtennaPolBit_t)((map[8] & 0x38) >> 3);
			*pGainSettings = (map[8] & 0xC0) >> 6;
		}
	}
	return retVal;

}/* NxtennaGetConfig */


/**********************************************************************
 *
 * NxtennaGetSetting
 *
 * Gets last values set by the 909 driver.
 *
 * Inputs:
 *	void *pContext	- NULL, or user-defined ID for multiple NXT200Xs
 *
 * Outputs:
 *	Data8 *pPosition	- 4 significant bits (range 0-15)
 *	Data8 *pGain		- 2 significant bits (range 0-3)
 *	Data8 *pPol			- 1 significant bit (0 or 1)
 *	Data8 *pChannel		- 7 significant bits (range 0-127)
 *
 * Returns:
 *	Data16
 *
 * Notes:
 *	Output values are not valid if the 909 driver is not enabled, 
 *	indicated by NXT_ERR_MODE return value.
 *
 **********************************************************************/
DRV2_API Data16 NxtennaGetSetting(void *pContext,
					Data8 *pPosition,
					Data8 *pGain,
					Data8 *pPol,
					Data8 *pChannel) {
	
	Data16	retVal = NXT_NO_ERROR;
	NxtDCB *pDCB;
	Data8	uc_gp4, uc_gp5, setting[2];

	/* find or create the referenced DCB */
	pDCB = GetDCB(pContext);

	if (pDCB == INVALID_CONTEXT) {	
		/* memory allocation error */
		retVal = NXT_ERR_MEMORY;
	}
	else {
		
		/* specify the data_xfer service */
		uc_gp4 = UC_NXTENNA_READ;

		/* write to UC_GP_5 */
		uc_gp5 = NXTENNA_ANT_SETTING;

		retVal = regNxtennaSvc(pDCB, uc_gp4, uc_gp5, setting, 2);

		if (NXT_NO_ERROR == retVal) {
			/* decrypt UC_GP_6,7 */
			*pPosition = (setting[0] & 0x3C) >> 2;
			*pPol = (setting[0] & 0x02) >> 1;
			*pGain = ((setting[0] & 0x01) << 1)|((setting[1] & 0x80) >> 7);
			*pChannel = setting[1] & 0x7F;
		}
	}
	return retVal;

}/* NxtennaGetSetting */

/**********************************************************************
 *
 * NxtennaGetMetric
 *
 * Gets one of several metrics used for antenna control.
 *
 * Inputs:
 *	void *pContext	- NULL, or user-defined ID for multiple NXT200Xs
 *	NxtennaMetric_t	metric	- identifies metric being requested
 *
 * Outputs:
 *	Data16 *pMetric	- calculated metric value in 16 bits
 *
 * Returns:
 *	Data16
 *
 * Notes:
 *	Use of this API may cause loss of lock, depending on the metric
 *	being requested.  It is the user's responsibility to call this
 *	API at the appropriate time to avoid disruption of service.  If
 *	service is disrupted then the user must use NxtStart( ) to restart
 *	channel acquisition.
 *
 **********************************************************************/
DRV2_API Data16 NxtennaGetMetric(void *pContext,
					NxtennaMetric_t metric,
					Data16 *pMetric) {
	
	Data16	retVal = NXT_NO_ERROR;
	NxtDCB *pDCB;
	Data8	uc_gp4, uc_gp5, setting[2];
	Data8	acqCtrl, buffer;

	/* find or create the referenced DCB */
	pDCB = GetDCB(pContext);

	if (pDCB == INVALID_CONTEXT) {	
		/* memory allocation error */
		retVal = NXT_ERR_MEMORY;
	}
	else {
		
		/* specify the data_xfer service */
		uc_gp4 = UC_NXTENNA_READ;

		switch (metric) {
		case METRIC_SIG_STRENGTH:
			uc_gp5 = NXTENNA_SIGNAL_METRIC;
			break;
		case METRIC_CHANNEL_QUAL:
			uc_gp5 = NXTENNA_CHANNEL_METRIC;		
			retVal = NxtGetRegister(NULL, UC_ACQ_CONTROL, 1, &acqCtrl);
			buffer = acqCtrl | UC_ACQ_FAT_STOP ;
			retVal|= NxtSetRegister(NULL, UC_ACQ_CONTROL, 1, &buffer);
			break;
		default:
			retVal = NXT_ERR_MODE;
			break;
		}
		if (NXT_NO_ERROR == retVal) {
			retVal = regNxtennaSvc(pDCB, uc_gp4, uc_gp5, setting, 2);

			if (NXT_NO_ERROR == retVal) {
				if (METRIC_CHANNEL_QUAL == metric) {
					retVal = NxtSetRegister(NULL, UC_ACQ_CONTROL, 1, &acqCtrl);
				}
				*pMetric = DATA8_TO_DATA16(setting);
			}
		}
	}
	return retVal;

}/* NxtennaGetMetric */

/**********************************************************************
 *
 * NxtennaGetState
 *
 * Gets current state of the Nxtenna firmware.
 *
 * Inputs:
 *	void *pContext	- NULL, or user-defined ID for multiple NXT200Xs
 *
 * Outputs:
 *	Data8 *pState	- bit-map of Nxtenna state flags.  See #defines in
 *					  nxtenna.h.
 *
 * Returns:
 *	Data16
 *
 **********************************************************************/
DRV2_API Data16 NxtennaGetState(void *pContext,
					Data8 *pState){
	
	Data16	retVal = NXT_NO_ERROR;
	NxtDCB *pDCB;
	Data8	uc_gp4, uc_gp5;

	/* find or create the referenced DCB */
	pDCB = GetDCB(pContext);

	if (pDCB == INVALID_CONTEXT) {	
		/* memory allocation error */
		retVal = NXT_ERR_MEMORY;
	}
	else {
		
		/* specify the data_xfer service */
		uc_gp4 = UC_NXTENNA_READ;
		
		/* write to UC_GP_5 */
		uc_gp5 = NXTENNA_DRIVER_STATE;

		retVal = regNxtennaSvc(pDCB, uc_gp4, uc_gp5, pState, 1);

	}
	return retVal;

}/* NxtennaGetState */

/**********************************************************************/
/*                          Automatic Control APIs                    */
/**********************************************************************/

/**********************************************************************
 *
 * NxtennaSeekOptimum
 *
 * Sets optimum antenna setting under static conditions.
 *
 * Inputs:
 *	void *pContext	- NULL, or user-defined ID for multiple NXT200Xs
 *
 * Returns:
 *	Data16
 *
 * Notes:
 *	The EIA/CEA-909 driver must be enabled using NxtennaEnableDriver( )
 *	before using this API, or NXT_ERR_MODE will be returned.
 *
 *	This function call returns immediately, although the seek operation
 *	may take up to 1 second to complete.  To determine whether the seek
 *	has completed, use NxtennaGetState( ).
 *
 **********************************************************************/
DRV2_API Data16 NxtennaSeekOptimum(void *pContext){
	
	Data16	retVal = NXT_NO_ERROR;
	NxtDCB *pDCB;
	Data8	uc_gp4, uc_gp5;

	/* find or create the referenced DCB */
	pDCB = GetDCB(pContext);

	if (pDCB == INVALID_CONTEXT) {	
		/* memory allocation error */
		retVal = NXT_ERR_MEMORY;
	}
	else {
		
		/* specify the data_xfer service */
		uc_gp4 = UC_NXTENNA_WRITE;

		uc_gp5 = NXTENNA_STATIC_SETTING;

		retVal = regNxtennaSvc(pDCB, uc_gp4, uc_gp5, NULL, 0);
	}	
	return retVal;

}/* NxtennaSeekOptimum */

/**********************************************************************
 *
 * NxtennaEnableTracking
 *
 * Enables or disables constant tracking for optimum setting.
 *
 * Inputs:
 *	void *pContext	- NULL, or user-defined ID for multiple NXT200Xs
 *	Bool bEnable	- TRUE to enable, FALSE to disable tracking
 *
 * Returns:
 *	Data16
 *
 * Notes:
 *	The EIA/CEA-909 driver must be enabled using NxtennaEnableDriver( )
 *	before using this API, or NXT_ERR_MODE will be returned.
 *
 *	It is not necessary to use NxtennaSeekOptimum( ) if tracking is
 *	going to be used.  The tracking function in the firmware will cause
 *	an initial seek for optimum setting if no reception is possible at
 *	the current setting.
 *
 **********************************************************************/
DRV2_API Data16 NxtennaEnableTracking(void *pContext,
					Bool bEnable){
	
	Data16	retVal = NXT_NO_ERROR;
	NxtDCB *pDCB;
	Data8	uc_gp4, uc_gp5, uc_gp6;

	/* find or create the referenced DCB */
	pDCB = GetDCB(pContext);

	if (pDCB == INVALID_CONTEXT) {	
		/* memory allocation error */
		retVal = NXT_ERR_MEMORY;
	}
	else {
		/* specify the data_xfer service */
		uc_gp4 = UC_NXTENNA_WRITE;

		uc_gp5 = NXTENNA_TRACKING_REQUEST;

		uc_gp6 = (Data8)bEnable;

		retVal = regNxtennaSvc(pDCB, uc_gp4, uc_gp5, &uc_gp6, 1);
	}
	return retVal;

}/* NxtennaEnableTracking */



#endif ALPSTUNER
