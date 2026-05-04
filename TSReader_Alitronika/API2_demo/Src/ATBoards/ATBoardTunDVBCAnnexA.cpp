/*! Time-stamp: <@(#)ATBoardTunDVBCAnnexA.cpp   07/05/2007 - 15:46:04   P.Hoogervorst>
*********************************************************************
*  @file   : ATBoardTunDVBCAnnexA.cpp
*
*          : Device supported: AT70, AT730, AT700, AT730
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
#include "ATBoardTunDVBCAnnexA.h"

/**********************************************************************************************************/
// defines

/**********************************************************************************************************/
// functions
// constructor
CATBoardTunDVBCAnnexA::CATBoardTunDVBCAnnexA(void)
{
}

// destructor
CATBoardTunDVBCAnnexA::~CATBoardTunDVBCAnnexA(void)
{
}


/*
 * Do the one time initialize and reset of the tuner. Is done once at startup.
 */
BOOL CATBoardTunDVBCAnnexA::Init()
{
	CAtRegisters RegAcc(m_pAtBoard);
	ATREGISTRY &Registers = RegAcc;

	// get registers from device and store them in the ATBoard object
	m_pAtBoard->GetRegisters();

	// reset tuner module
	SET_BITMASK(Registers.m_RecordConfig, AT_RCONF_TUNRST);
	m_pAtBoard->UpdateRegisters();
	CLR_BITMASK(Registers.m_RecordConfig, AT_RCONF_TUNRST);
	m_pAtBoard->UpdateRegisters();

	// reset tuner controller
	m_pAtBoard->DvbSource(SRC_RESET, NULL);
	// init tuner controller
	m_pAtBoard->DvbSource(SRC_INIT, NULL);

	return TRUE;
}

/*
 * Set the tuner.
 */
BOOL CATBoardTunDVBCAnnexA::Set(u32 nFreq, u32 nSymRate, src_spectral_inversion_t Invert, src_modulation_t Modulation)
{
	dvb_frontend_parameters Info;

	// set parameters
	Info.frequency = nFreq;
	Info.inversion = Invert;
	Info.u.qam.modulation = Modulation;
	Info.u.qam.symbol_rate= nSymRate;

	// send parameters to device
	return m_pAtBoard->DvbSource(SRC_SET_FRONTEND, &Info);
}

/*
 * Get status from the tuner
 */
BOOL CATBoardTunDVBCAnnexA::Get(BOOL &bInSync, BOOL &bCarrierDetect, u16 &nSignalStrength, u32 &nBer, src_spectral_inversion_t &Invert, src_modulation_t &Modulation)
{
	// set to defaults
	bInSync = FALSE;
	bCarrierDetect = FALSE;
	nSignalStrength = 0;
	nBer = 0;
	Invert = INVERSION_OFF;
	Modulation = QAM_64;
	
	// get status
	src_status_t eStatus;
	if (!m_pAtBoard->DvbSource(SRC_READ_STATUS, &eStatus))
		return FALSE;

	if (eStatus & SRC_HAS_SYNC)
		bInSync = TRUE;
	else
		bInSync = FALSE;

	if (eStatus & SRC_HAS_CARRIER)
		bCarrierDetect = TRUE;
	else
		bCarrierDetect = FALSE;

	// get signal strength
	if (!m_pAtBoard->DvbSource(SRC_READ_SIGNAL_STRENGTH, &nSignalStrength))
		return FALSE;

	// get bit error rate (ber)
	if (!m_pAtBoard->DvbSource(SRC_READ_BER, &nBer))
		return FALSE;

	dvb_frontend_parameters Info;
	if (!m_pAtBoard->DvbSource(SRC_GET_FRONTEND, &Info))
		return FALSE;

	// get spectral inversion (retrieve spectral inversion when it is set to automatic)
	Invert = Info.inversion;
	
	// get modulation (retrieve modulation when it is set to automatic)
	Modulation = Info.u.qam.modulation;

	return TRUE;
}
