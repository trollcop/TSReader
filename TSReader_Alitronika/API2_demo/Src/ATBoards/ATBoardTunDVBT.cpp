/*! Time-stamp: <@(#)ATBoardTunDVBT.cpp   07/05/2007 - 15:46:04   P.Hoogervorst>
*********************************************************************
*  @file   : ATBoardTunDVBT.cpp
*
*          : Device supported: AT8, AT80, AT82, AT800, AT820, AT1800
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
#include "ATBoardTunDVBT.h"

/**********************************************************************************************************/
// defines

/**********************************************************************************************************/
// functions
// constructor
CATBoardTunDVBT::CATBoardTunDVBT(void)
{
}

// destructor
CATBoardTunDVBT::~CATBoardTunDVBT(void)
{
}


/*
 * Do the one time initialize and reset of the tuner. Is done once at startup.
 */
BOOL CATBoardTunDVBT::Init()
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
BOOL CATBoardTunDVBT::Set(u32 nFreq, src_bandwidth_t BandWidth, BOOL bBandWidthGroup)
{
	dvb_frontend_parameters Info;

	// set parameters
	Info.frequency = nFreq;
	Info.inversion = INVERSION_AUTO;	// always AUTO
	Info.u.ofdm.bandwidth = BandWidth;
	Info.u.ofdm.bwMode67 = bBandWidthGroup;

	// send parameters to device
	return m_pAtBoard->DvbSource(SRC_SET_FRONTEND, &Info);
}

/*
 * Get status from the tuner
 */
BOOL CATBoardTunDVBT::Get(	BOOL &bInSync,
							BOOL &bCarrierDet,
							BOOL &bSigDet,
							BOOL &bViterbiDet,
							u16 &nSignalStrength,
							u16 &nSnr,
							u32 &nBer,
							src_modulation_t &Modulation,
							src_transmit_mode_t &ModTransmitMode,
							src_guard_interval_t &ModGuard,
							src_hierarchy_t &ModHierarch,
							src_code_rate_t &ModHPFecCodeRate,
							src_code_rate_t &ModLPFecCodeRate)
{
	// set to defaults
	bInSync = FALSE;
	bCarrierDet = FALSE;
	bSigDet = FALSE;
	bViterbiDet = FALSE;
	nSignalStrength = 0;
	nSnr = 0;
	nBer = 0;
	Modulation = QPSK;
	ModTransmitMode = TRANSMISSION_MODE_2K;
	ModGuard = GUARD_INTERVAL_1_4;
	ModHierarch = HIERARCHY_NONE;
	ModHPFecCodeRate = FEC_1_2;
	ModLPFecCodeRate = FEC_1_2; 
	
	// get status
	src_status_t eStatus;
	if (!m_pAtBoard->DvbSource(SRC_READ_STATUS, &eStatus))
		return FALSE;

	if (eStatus & SRC_HAS_SYNC)
		bInSync = TRUE;
	else
		bInSync = FALSE;

	if (eStatus & SRC_HAS_CARRIER)
		bCarrierDet = TRUE;
	else
		bCarrierDet = FALSE;

	if (eStatus & SRC_HAS_SIGNAL)
		bSigDet = TRUE;
	else
		bSigDet = FALSE;

	if (eStatus & SRC_HAS_VITERBI)
		bViterbiDet = TRUE;
	else
		bViterbiDet = FALSE;

	// get signal strength
	if (!m_pAtBoard->DvbSource(SRC_READ_SIGNAL_STRENGTH, &nSignalStrength))
		return FALSE;

	// get signal to noise ratio
	if (!m_pAtBoard->DvbSource(SRC_READ_SNR, &nSnr))
		return FALSE;

	// get bit error rate (BER)
	if (!m_pAtBoard->DvbSource(SRC_READ_BER, &nBer))
		return FALSE;

	// get modulation status
	dvb_frontend_parameters Info;
	if (!m_pAtBoard->DvbSource(SRC_GET_FRONTEND, &Info))
		return FALSE;

	// get modulation
	Modulation = Info.u.ofdm.constellation;
	// get guard interval
	ModGuard = Info.u.ofdm.guard_interval;
	// get hierarchical information
	ModHierarch = Info.u.ofdm.hierarchy_information;
	// get low priority stream FEC code rate
	ModLPFecCodeRate = Info.u.ofdm.code_rate_LP;
	// get high priority stream FEC code rate
	ModHPFecCodeRate = Info.u.ofdm.code_rate_HP;
	// get transmission mode
	ModTransmitMode = Info.u.ofdm.transmission_mode;

	return TRUE;
}


