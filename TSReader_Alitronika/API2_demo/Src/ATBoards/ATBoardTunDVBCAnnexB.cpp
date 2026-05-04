/*! Time-stamp: <@(#)ATBoardTunDVBCAnnexB.cpp   07/05/2007 - 15:46:04   P.Hoogervorst>
*********************************************************************
*  @file   : ATBoardTunDVBCAnnexB.cpp
*
*          : Device supported: AT72, AT720
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
#include "ATBoardTunDVBCAnnexB.h"

/**********************************************************************************************************/
// defines

/**********************************************************************************************************/
// functions
// constructor
CATBoardTunDVBCAnnexB::CATBoardTunDVBCAnnexB(void)
{
}

// destructor
CATBoardTunDVBCAnnexB::~CATBoardTunDVBCAnnexB(void)
{
}


/*
 * Do the one time initialize and reset of the tuner. Is done once at startup.
 */
BOOL CATBoardTunDVBCAnnexB::Init()
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
BOOL CATBoardTunDVBCAnnexB::Set(u32 nFreq, src_modulation_t Modulation)
{
	dvb_frontend_parameters Info;

	// set parameters
	Info.frequency = nFreq;
	Info.u.qam.modulation = Modulation;

	// send parameters to device
	return m_pAtBoard->DvbSource(SRC_SET_FRONTEND, &Info);
}

/*
 * Get status from the tuner
 */
BOOL CATBoardTunDVBCAnnexB::Get(BOOL &bInSync, BOOL &bCarrierDetect, BOOL &bTsLock, u16 &nSnr, u32 &nCorBer, u32 &nUnCorBer)
{
	// set to defaults
	bInSync = FALSE;
	bCarrierDetect = FALSE;
	bTsLock = FALSE;
	nSnr = 0;
	nCorBer = 0;
	nUnCorBer = 0;
	
	// get status
	src_status_dvbcb_t eStatus;
	if (!m_pAtBoard->DvbSource(SRC_READ_STATUS, &eStatus))
		return FALSE;

	if (eStatus & SRC_HAS_QAMSYNC_LOCK)
		bInSync = TRUE;
	else
		bInSync = FALSE;

	if (eStatus & SRC_HAS_FAT_LOCK)
		bCarrierDetect = TRUE;
	else
		bCarrierDetect = FALSE;

	if (eStatus & SRC_HAS_MPEG_LOCK)
		bTsLock = TRUE;
	else
		bTsLock = FALSE;

	// get signal to noise ratio
	if (!m_pAtBoard->DvbSource(SRC_READ_SNR, &nSnr))
		return FALSE;

	// get corrected bit error rate
	if (!m_pAtBoard->DvbSource(SRC_READ_BER, &nUnCorBer))
		return FALSE;

	// get uncorrected bit error rate
	if (!m_pAtBoard->DvbSource(SRC_READ_UNCORECTED_BER, &nCorBer))
		return FALSE;

		return TRUE;
}
