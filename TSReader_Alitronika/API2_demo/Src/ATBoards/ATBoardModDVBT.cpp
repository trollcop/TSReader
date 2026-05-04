/*! Time-stamp: <@(#)ATBoardModDVBT.cpp   07/05/2007 - 15:46:04   P.Hoogervorst>
*********************************************************************
*  @file   : ATBoardModDVBT.cpp
*
*          : Device supported: AT2800, AT3800, AT2780
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
#include "ATBoardModDVBT.h"

/**********************************************************************************************************/
// defines

/**********************************************************************************************************/
// functions
// constructor
CATBoardModDVBT::CATBoardModDVBT(void)
{
}

// destructor
CATBoardModDVBT::~CATBoardModDVBT(void)
{
}

/*
 * Do the one time initialize of the modulator. Is done once at startup.
 */
BOOL CATBoardModDVBT::Init()
{
	dvb_at2800_parameters DvbTOfdmParams;

	// init modulator
	if (!m_pAtBoard->DvbSource(SRC_INIT, NULL))
		return FALSE;
	
	// restore the DVB-T parameters
	if (!m_pAtBoard->DvbSource(SRC_RESTORE_MOD_DVB, &DvbTOfdmParams))
		return FALSE;

	// set the parameters
	return Set(	DvbTOfdmParams.constellation,
				DvbTOfdmParams.transmission_mode,
				DvbTOfdmParams.guard_interval,
				DvbTOfdmParams.code_rate_HP,
				DvbTOfdmParams.bandwidth,
				DvbTOfdmParams.inversion,
				(BOOL)DvbTOfdmParams.dvb_h_en,
				(BOOL)DvbTOfdmParams.dvb_h_HP_time_slice,
				(BOOL)DvbTOfdmParams.dvb_h_HP_MPE_FEC,
				(BOOL)DvbTOfdmParams.dvb_h_indepth_int,
				DvbTOfdmParams.dvb_h_cell_id);
}

/*
* Initialize play for derived class. If the initialization of the play must be changed or expanded by a derived class,
* The derived class should do that in this function
*/
BOOL CATBoardModDVBT::_InitPlay	   (u32			nFileBitrate,
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
BOOL CATBoardModDVBT::Set(	src_modulation_t			Modulation,
							src_transmit_mode_t			TranMode,
							src_guard_interval_t		GuardInt,
							src_code_rate_t				FECCodeRate,
							src_bandwidth_t				Bandwidth,
							src_spectral_inversion_t	Inversion,
							BOOL						bDvbHEn,
							BOOL						bDvbHTimeSlice,
							BOOL						bDvbH_MPE_FEC,
							BOOL						bDvbHIndepthInt,
							u16							nDvbHCellId)
{
	dvb_at2800_parameters DvbTOfdmParams;

	DvbTOfdmParams.constellation		= Modulation;
	DvbTOfdmParams.transmission_mode	= TranMode;
	DvbTOfdmParams.guard_interval		= GuardInt;
	DvbTOfdmParams.code_rate_HP			= FECCodeRate;
	DvbTOfdmParams.code_rate_LP			= FEC_7_8;			// non hierarchical mode: always FEC_7_8
	DvbTOfdmParams.bandwidth			= Bandwidth;
	DvbTOfdmParams.hierarchy_information= HIERARCHY_NONE;	// DVBT modulator only supports non hierarchical mode
	DvbTOfdmParams.inversion			= Inversion;
	DvbTOfdmParams.dvb_h_en				= (u8)bDvbHEn;
	DvbTOfdmParams.dvb_h_HP_time_slice	= (u8)bDvbHTimeSlice;
	DvbTOfdmParams.dvb_h_LP_time_slice	= (u8)FALSE;		// non hierarchical mode: always FALSE
	DvbTOfdmParams.dvb_h_HP_MPE_FEC		= (u8)bDvbH_MPE_FEC;
	DvbTOfdmParams.dvb_h_LP_MPE_FEC		= (u8)FALSE;		// non hierarchical mode: always FALSE
	DvbTOfdmParams.dvb_h_indepth_int	= (u8)bDvbHIndepthInt;
	DvbTOfdmParams.dvb_h_cell_id		= nDvbHCellId;

	// set the parameters
	if (!m_pAtBoard->DvbSource(SRC_SET_MOD_PARAMS, &DvbTOfdmParams))
		return FALSE;
	
	// store the parameters
	return m_pAtBoard->DvbSource(SRC_STORE_MOD_DVB, &DvbTOfdmParams);
}

/*
* Get the modulation status of the modulator
*/
BOOL CATBoardModDVBT::Get(BOOL &bInSync, BOOL &b204PacketSize, BOOL &bModOFlow, BOOL &bModUFlow)
{
	// set to defaults
	bInSync = FALSE;
	b204PacketSize = FALSE;
	bModOFlow = FALSE;
	bModUFlow = FALSE;

	src_status_at2800_t eStatus;

	// get the modulation status
	if (!m_pAtBoard->DvbSource(SRC_GET_MOD_STAT, &eStatus))
		return FALSE;

	if (eStatus & DST_HAS_HP_PCR_TS_INSYNC)
		bInSync = TRUE;
	else
		bInSync = FALSE;

	if (eStatus & DST_HAS_HP_PCR_TS_204PKT)
		b204PacketSize = TRUE;
	else
		b204PacketSize = FALSE;

	if (eStatus & DST_HAS_HP_PCR_TS_OFLOW)
		bModOFlow = TRUE;
	else
		bModOFlow = FALSE;

	if (eStatus & DST_HAS_MOD_UFLOW)
		bModUFlow = TRUE;
	else
		bModUFlow = FALSE;
	
	return TRUE;
}

