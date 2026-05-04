/**********************************************************************
 * NXTENNA.H
 * Public Include File for Nxtenna API
 *
 * Copyright (C) 2002 NxtWave Communications, Inc.
 *
 * $Log:   //panxtbdc01/AppsEng/PVCS/Application Eval/archives/EVAL2005/Include_new/drvNxtenna.h-arc  $
 * 
 *    Rev 1.1   Jan 30 2003 12:11:40   sreichgo
 * Changed lines 67 and 90 for ANSI compiler error.
 * 
 *    Rev 1.0   Jan 03 2003 11:43:00   raggarwa
 * Initial revision.
 * 
 *    Rev 1.0   Nov 04 2002 14:03:20   raggarwa
 * Initial revision.
 * 
 *    Rev 1.1   Aug 20 2002 18:44:36   reichgot
 * Added NXTENNA_FIELD_* definitions.
 * 
 *    Rev 1.0   Jun 27 2002 17:38:26   raggarwal
 * Initial revision.
 * 
 **********************************************************************/

#ifndef NXTENNA_H
#define NXTENNA_H

/*
******************************************************************************
Includes
******************************************************************************
*/

/*
******************************************************************************
Defines
******************************************************************************
*/

/*
Nxtenna Firmware state flags
*/
#define NXTENNA_DRIVER_ENABLED		0x01
#define NXTENNA_SEEKING_OPTIMUM		0x02
#define NXTENNA_TRACKING_ENABLED	0x04
#define NXTENNA_INITIALIZING		0x08
#define NXTENNA_TRACKING_ACTIVE		0x10

/*
Nxtenna Log data types
*/
#define NXTENNA_LOG					0
#define NXTENNA_LOG_NOW				1
#define	NXTENNA_FIELD_BENCHMARK		2
#define	NXTENNA_FIELD_START			3
#define	NXTENNA_FIELD_SEEK			4


/*
******************************************************************************
Public Types
******************************************************************************
*/

/* 
Mapping from 16 clockwise positions (array index) to the 4-bit 
value transmitted on the EIA/CEA-909 interface.
*/
typedef Data8 NxtennaMapping_t;
/*
typedef struct {
	Data8 map[16];
} NxtennaMapping_t;
*/

/*
Use of the EIA/CEA-909 interface polarization bit.
*/
typedef enum {
	POL_BIT_NOT_USED = 0x00,
	POL_BIT_USED = 0x01,		/* for polarization */
	POL_BIT_PHASE_SHIFTER = 0x02/* NxtWave proprietary mode */
	/* add new values here */
} NxtennaPolBit_t;

/*
Metric types
*/
typedef enum {
	METRIC_SIG_STRENGTH,	/* range 0..100% */
	METRIC_CHANNEL_QUAL 	/* 16-bit relative value - indicates multipath */
	/* add new values here */
} NxtennaMetric_t;

/*
******************************************************************************
Public Functions
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
 *	void *pContext	- NULL, or user-defined ID for multiple NXT200xs
 *	Bool bEnable	- TRUE to enable driver, FALSE to disable (default)
 *
 * Returns:
 *	Data16
 *
 * Notes:
 *
 **********************************************************************/
DRV2_API Data16 NxtennaEnableDriver(void *pContext, Bool bEnable);

/**********************************************************************
 *
 * NxtennaSetConfig
 *
 * Configures antenna specifics in the NXT200x
 *
 * Inputs:
 *	void *pContext			- NULL, or user-defined ID for multiple NXT200xs
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
									  NxtennaPolBit_t polBit);




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
 *	void *pContext	- NULL, or user-defined ID for multiple NXT2002s
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
									   Data8 channel);


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
 *	void *pContext			- NULL, or user-defined ID for multiple NXT200xs
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
									  NxtennaPolBit_t *pPolBit);


/**********************************************************************
 *
 * NxtennaGetSetting
 *
 * Gets last values set by the 909 driver.
 *
 * Inputs:
 *	void *pContext	- NULL, or user-defined ID for multiple NXT2002s
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
									   Data8 *pChannel);

/**********************************************************************
 *
 * NxtennaGetMetric
 *
 * Gets one of several metrics used for antenna control.
 *
 * Inputs:
 *	void *pContext	- NULL, or user-defined ID for multiple NXT2002s
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
									  Data16 *pMetric);

/**********************************************************************
 *
 * NxtennaGetState
 *
 * Gets current state of the Nxtenna firmware.
 *
 * Inputs:
 *	void *pContext	- NULL, or user-defined ID for multiple NXT2002s
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
									 Data8 *pState);

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
 *	void *pContext	- NULL, or user-defined ID for multiple NXT2002s
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
DRV2_API Data16 NxtennaSeekOptimum(void *pContext);

/**********************************************************************
 *
 * NxtennaEnableTracking
 *
 * Enables or disables constant tracking for optimum setting.
 *
 * Inputs:
 *	void *pContext	- NULL, or user-defined ID for multiple NXT2002s
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
										   Bool bEnable);


#endif

