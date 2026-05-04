/*! Time-stamp: <@(#)ATBoardModDVBC.cpp   07/05/2007 - 15:46:04   P.Hoogervorst>
*********************************************************************
*  @file   : ATBoardModDVBC.cpp
*
*          : Device supported: AT2700, AT2780
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
#include "ATBoardModDVBC.h"

/**********************************************************************************************************/
// defines

/**********************************************************************************************************/
// functions
// constructor
CATBoardModDVBC::CATBoardModDVBC(void)
{
}

// destructor
CATBoardModDVBC::~CATBoardModDVBC(void)
{
}

/*
 * Do the one time initialize of the modulator. Is done once at startup.
 */
BOOL CATBoardModDVBC::Init()
{
	dvb_at2700_parameters DvbCParams;

	// init modulator
	if (!m_pAtBoard->DvbSource(SRC_INIT, NULL))
		return FALSE;
	
	// restore the DVB-T parameters
	if (!m_pAtBoard->DvbSource(SRC_RESTORE_MOD_DVB, &DvbCParams))
		return FALSE;

	// set the parameters
	return Set(	DvbCParams.modulation,
				DvbCParams.filter_rolloff,
				DvbCParams.interleaver,
				DvbCParams.inversion,
				DvbCParams.symbol_rate,
				(BOOL)DvbCParams.annexb_enable);
}

/*
* Initialize play for derived class. If the initialization of the play must be changed or expanded by a derived class,
* The derived class should do that in this function
*/
BOOL CATBoardModDVBC::_InitPlay	   (u32			nFileBitrate,
									BOOL		bORemux,
									u32			nOutputBitrate,
									EIOSel		OSel,
									EIOMode		OMode,
									EIOSpiMode	OSpiMode,
									ETSPSize	OTsPSize,
									BOOL		bOHTP,
									BOOL		bOCTP,
									BYTE		nBurstSize)
{

	// set registers
	CAtRegisters RegAcc(m_pAtBoard);
	ATREGISTRY &Registers = RegAcc;

	// get registers from device and store them in the ATBoard object
	m_pAtBoard->GetRegisters();

	// clear loop through
	CLR_BITMASK(Registers.m_RecordConfig, AT_RCONF_LEN);

	// enable remultiplexing
	SET_BITMASK(Registers.m_PlayConfig, AT_PCONF_PCRREST);
	SET_BITMASK(Registers.m_PlayConfig, AT_PCONF_BRREMUX);
	CLR_BITMASK(Registers.m_PlayConfig, AT_PCONF_CTP);

	// set registers to device
	m_pAtBoard->UpdateRegisters();

	return TRUE;
}

/*
* Set the modulation settings.
*/
BOOL CATBoardModDVBC::Set(	src_modulation_t			Modulation,
							src_filter_rolloff_t		FltRollOff,
							src_interleaver_t			Interleaver,
							src_spectral_inversion_t	Inversion,
							u32							nSymbolRate,
							BOOL						bAnnexBEn)
{
	dvb_at2700_parameters DvbCParams;

	DvbCParams.modulation			= Modulation;
	DvbCParams.filter_rolloff		= FltRollOff;
	DvbCParams.interleaver			= Interleaver;
	DvbCParams.inversion			= Inversion;
	DvbCParams.symbol_rate			= nSymbolRate;
	DvbCParams.annexb_enable		= (u8)bAnnexBEn;

	// override some fixed settings for AnnexB mode
	if (bAnnexBEn)
	{
		if (Modulation == QAM_64)
			DvbCParams.symbol_rate = ANNEXB_QAM64_SYMRATE;
		else
			DvbCParams.symbol_rate = ANNEXB_QAM256_SYMRATE;

		DvbCParams.filter_rolloff = FLT_ALPHA_18;
	}

	// set the parameters
	if (!m_pAtBoard->DvbSource(SRC_SET_MOD_PARAMS, &DvbCParams))
		return FALSE;
	
	// store the parameters
	return m_pAtBoard->DvbSource(SRC_STORE_MOD_DVB, &DvbCParams);
}

/*
* Get the modulation status of the modulator
*/
BOOL CATBoardModDVBC::Get(BOOL &bInSync, BOOL &b204PacketSize, BOOL &bModOFlow)
{
	// set to defaults
	bInSync = FALSE;
	b204PacketSize = FALSE;
	bModOFlow = FALSE;

	src_status_at2700_t eStatus;

	// get the modulation status
	if (!m_pAtBoard->DvbSource(SRC_GET_MOD_STAT, &eStatus))
		return FALSE;

	if (eStatus & DST_HAS_PCR_TS_INSYNC_CH0)
		bInSync = TRUE;
	else
		bInSync = FALSE;

	if (eStatus & DST_HAS_PCR_TS_204PKT_CH0)
		b204PacketSize = TRUE;
	else
		b204PacketSize = FALSE;

	if (eStatus & DST_HAS_PCR_TS_OFLOW_CH0)
		bModOFlow = TRUE;
	else
		bModOFlow = FALSE;

	return TRUE;
}

