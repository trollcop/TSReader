/*! Time-stamp: <@(#)ATBoardModRfIf.h   07/05/2007 - 15:46:04   P.Hoogervorst>
*********************************************************************
*  @file   : ATBoardModRfIf.h
*
*  Project : ATDemoApp. Testappliction for Alitronika devices.
*			 Supports Linux and windows operating System.
*
*          : Device supported: AT2700, AT2800, AT3800, AT2780.
*
*  Package : 
*
*  Company : Engineering Spirit
*
*  Author  : P.Hoogervorst                              Date: 07/05/2007
*
*  Purpose : Implementation of methods for class 
*
*********************************************************************
* Version History:
*
* V 0.10  07/05/2007  BN : First Revision
*
*********************************************************************
*/

#ifndef __ATBOARD_MOD_RFIF_H__
#define __ATBOARD_MOD_RFIF_H__

/**********************************************************************************************************/
// includes
#include "ATBoardPlayRec.h"

class CAtBoardModRfIf : public CATBoardPlayRec
{
public:
	CAtBoardModRfIf(void);
public:
	virtual ~CAtBoardModRfIf(void);

public:

	/*!
	* Initialize and restore the RF and IF settings
	*
	* @return BOOL  : TRUE on success, else FALSE
	*/
	BOOL InitRfIf();

	/*!
	* Set the Modulator RF and IF settings.
	*
	* Valid settings:
	* When IF output is enabled:	IF range can be set to 35-37MHz and 69-71MHz. The IF frequency resolution is 1Hz
	* When RF output is enabled:	RF range can be set to 50-1000MHz. The RF frequency resolution is 1Hz
	* When RF output is enabled:	The output level can be set between -35dBm to +2dBm. The RF level stepsize is 0.5dBm.
	*								The RF level is in 0.1dBm units so a value of -215 equals to -21.5dBm.
	*
    * @param bRfIfSel :			selects if RF output is selected or IF output. 1: RF is selected, 0: IF is selected
    * @param iIfFrequency :		(absolute) IF frequency (in Hz)
    * @param nRfFrequency :		(absolute) RF frequency (in Hz)
    * @param iRfoutputLevel :	(absolute) output level for the modulators. (in 0.1dBm, 0.5dBm step size)
	*
	* @return BOOL  : TRUE on success, else FALSE
	*/
	BOOL SetRfIf(	BOOL	bRfIfSel,
					s32		iIfFrequency,
					u32		nRfFrequency,
					s32		iRfoutputLevel);

	/*!
	* Get the RF and IF status of the modulator
	*
	* @param bRfLocked : TRUE: RF output is locked to the requested frequency, FALSE: RF output is not locked
	*
	* @return BOOL  : TRUE on success, else FALSE
	*/
	BOOL GetRfIf(	BOOL	&bRfLocked); 
};

#endif //__ATBOARD_MOD_RFIF_H__
