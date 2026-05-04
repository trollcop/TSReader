/*! Time-stamp: <@(#)ATBoardModRfIf.cpp   07/05/2007 - 15:46:04   P.Hoogervorst>
*********************************************************************
*  @file   : ATBoardModRfIf.cpp
*
*  Project : ATDemoApp. Testappliction for Alitronika devices.
*			 Supports Linux and windows operating System.
*
*          : Device supported: AT2700, AT2800, AT3800, AT2780.
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

/**********************************************************************************************************/
// includes
#include "ATBoardModRfIf.h"

/**********************************************************************************************************/
// defines

/**********************************************************************************************************/
// functions
// constructor
CAtBoardModRfIf::CAtBoardModRfIf(void)
{
}

// destructor
CAtBoardModRfIf::~CAtBoardModRfIf(void)
{
}

/*
* Initialize and restore the RF and IF settings
*/
BOOL CAtBoardModRfIf::InitRfIf()
{
	dvb_if_rf_parameters IfRfParams;
	
	// get the stored RF and IF parameters
	if (!m_pAtBoard->DvbSource(SRC_RESTORE_MOD_RFIF, &IfRfParams))
		return FALSE;

	// set the restored values
	return SetRfIf((BOOL)IfRfParams.RfIfSel, IfRfParams.IfFrequency, IfRfParams.RfFrequency, IfRfParams.RfoutputLevel);
}

/*
* Set the Modulator RF and IF settings.
*/
BOOL CAtBoardModRfIf::SetRfIf(	BOOL	bRfIfSel,
								s32		iIfFrequency,
								u32		nRfFrequency,
								s32		iRfoutputLevel)
{
	dvb_if_rf_parameters IfRfParams;

	IfRfParams.RfIfSel = (u8)bRfIfSel;
	IfRfParams.IfFrequency = iIfFrequency;
	IfRfParams.RfFrequency = nRfFrequency;
	IfRfParams.RfoutputLevel = iRfoutputLevel;

	// Set the Modulator RF and IF parameters
	if (!m_pAtBoard->DvbSource(SRC_SET_MOD_IF_RF_PARAMS, &IfRfParams))
		return FALSE;

	// store the values
	return m_pAtBoard->DvbSource(SRC_STORE_MOD_RFIF, &IfRfParams);
}

/*
* Get the RF and IF status of the modulator
*/
BOOL CAtBoardModRfIf::GetRfIf(	BOOL	&bRfLocked)
{
	// set to default
	bRfLocked = FALSE;
	
	dvb_if_rf_parameters IfRfParams;
	
	// get the status
	if (!m_pAtBoard->DvbSource(SRC_GET_MOD_IF_RF_STAT, &IfRfParams))
		return FALSE;

	bRfLocked = (BOOL)IfRfParams.RfLocked;

	return TRUE;
}
