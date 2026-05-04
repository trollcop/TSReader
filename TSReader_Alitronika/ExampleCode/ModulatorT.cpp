static CATBoard *pBoard;

...

/****************************************************************************************************/
// set modulation parameters

/*
 *	Initializes the terrestrial modulator.
 *	This fuction must be called before any other call to the modulator
 */
void InitTerrestrialModulator()
{
	pBoard->DvbSource(SRC_INIT, NULL);
}


/*
 *	Set the modulator parameters
 */
void SetTerrestrialModulatorParams()
{
	dvb_at2800_parameters params;

	memset(params, 0, sizeof(params));
	
	params.bandwidth				= BANDWIDTH_8_7_MHZ;	// set bandwidth. (caps: BANDWIDTH_8_7_MHZ, BANDWIDTH_7_6_MHZ, BANDWIDTH_6_MHZ, BANDWIDTH_5_MHZ)
	params.code_rate_HP				= FEC_1_2;				// set code rate for high priority transport stream. (caps: FEC_1_2, FEC_2_3, FEC_3_4, FEC_5_6, FEC_7_8)
	params.code_rate_LP				= FEC_1_2;				// not supported, always FEC_1_2.
	params.constellation			= QPSK;					// set modulation. (caps: QPSK, QAM_16, QAM_64)
	params.transmission_mode		= TRANSMISSION_MODE_2K;	// set transmission mode. (caps DVB-T: TRANSMISSION_MODE_2K, TRANSMISSION_MODE_8K; caps DVB-H: TRANSMISSION_MODE_2K, TRANSMISSION_MODE_4K, TRANSMISSION_MODE_8K)
	params.guard_interval			= GUARD_INTERVAL_1_4;	// set guard interval. (caps: GUARD_INTERVAL_1_4, GUARD_INTERVAL_1_8, GUARD_INTERVAL_1_16, GUARD_INTERVAL_1_32)
	params.hierarchy_information	= HIERARCHY_NONE;		// not supported, always HIERARCHY_NONE.
	params.inversion				= INVERSION_OFF;		// set inversion.
	params.dvb_h_en					= FALSE;				// DVB-H extension: enable DVB-H mode
	params.dvb_h_cell_id			= 1200;					// DVB-H extension: set cell ID number
	params.dvb_h_indepth_int		= TRUE;					// DVB-H extension: enable indepth interleaver
	params.dvb_h_HP_time_slice		= TRUE;					// DVB-H extension: enable time slicing
	params.dvb_h_HP_MPE_FEC			= TRUE;					// DVB-H extension: enable MPE FEC
	params.dvb_h_LP_time_slice		= FALSE;				// DVB-H extension: not supported, always false
	params.dvb_h_LP_MPE_FEC			= FALSE;				// DVB-H extension: not supported, always false

	pBoard->DvbSource(SRC_SET_MOD_PARAMS, &params);
}

/*
 *	Read the status of the modulator
 */
void GetTerrestrialModulatorStatus()
{
	src_status_at2800 eStatus;

	/*
	 *	Get modulator status
	 *
	 *	All LP flags can be ignored because the terrestrial modulator does not
	 *	support hierarchical mode.
	 */
	pBoard->DvbSource(SRC_GET_MOD_STAT, &eStatus);

 	if (eStatus & DST_HAS_HP_PCR_TS_INSYNC)				// check if modulator is in sync
 		m_LedInSyncPcr.Set(CLedControl::LED_GREEN);
 	else
 		m_LedInSyncPcr.Set(CLedControl::LED_RED);

 	if (eStatus & DST_HAS_HP_PCR_TS_204PKT)				// check transport stream packet size
 		m_Led204PckPcr.Set(CLedControl::LED_GREEN);
 	else
 		m_Led204PckPcr.Set(CLedControl::LED_GRAY);

 	if (eStatus & DST_HAS_HP_PCR_TS_OFLOW)				// check if overflow has occured
 		m_LedTsOFlowPcr.Set(CLedControl::LED_RED);
 	else
 		m_LedTsOFlowPcr.Set(CLedControl::LED_GRAY);

 	if (eStatus & DST_HAS_MOD_UFLOW)					// check if underflow has occured
 		m_LedModUFlow.Set(CLedControl::LED_RED);
 	else
 		m_LedModUFlow.Set(CLedControl::LED_GRAY);
}

/****************************************************************************************************/
// set IF and RF parameters

typedef struct _SIfRangeList
{
	u32		m_nIfFreqSel;
	u32		m_nMinFreq;
	u32		m_nMidFreq;
	u32		m_nMaxFreq;
} SIfRangeList;

static SIfRangeList *pIfRangeList;
static u32			nModRFFreqMin;
static u32			nModRFFreqMax;
static u32			nRFLevelMin;
static u32			nRFLevelMan;
static u32			nRFLevelStepSize;

/*
 *	Get some information about the RF and IF capabilities.
 */
void GetTerrestrialModulatorRfIfInfo()
{
	dvb_frontend_info DvbFrontInfo;

	/*
	 *	Get the information structure
	 */
	pBoard->DvbSource(SRC_GET_INFO, &DvbFrontInfo);

	/*
	 *	below: fields that are important for the RF and IF part
	 */
	nModRFFreqMin		= DvbFrontInfo.frequency_min;		// minimum RF frequency (in Hz)
	nModRFFreqMax		= DvbFrontInfo.frequency_max;		// maximum RF frequency (in Hz)
	nRFLevelMin			= DvbFrontInfo.rflevel_min;			// minimum RF output level (in 0.1dBm)
	nRFLevelMan			= DvbFrontInfo.rflevel_max;			// maximum RF output level (in 0.1dBm)
	nRFLevelStepSize	= DvbFrontInfo.rflevel_stepsize;	// RF output level step size (in 0.1dBm)

	pIfRangeList		= new SIfRangeList[DvbFrontInfo.nrIffreqs];	// construct a table with all the IF ranges and its capabilities.
	for (u32 i = 0; i < DvbFrontInfo.nrIffreqs; i++)
	{
		pIfRangeList[i].m_nIfFreqSel	= i;
		pIfRangeList[i].m_nMinFreq		= DvbFrontInfo.iffreqs[i]-(DvbFrontInfo.ifrange/2);	// minimum IF frequency (in Hz)
		pIfRangeList[i].m_nMidFreq		= DvbFrontInfo.iffreqs[i];							// midrange IF frequency (in Hz)
		pIfRangeList[i].m_nMaxFreq		= DvbFrontInfo.iffreqs[i]+(DvbFrontInfo.ifrange/2);	// maximum RF frequency (in Hz)
	}
}

/*
 *	Set the IF or RF frequency.
 *	The IF/RF output can be used in two modes. The first mode is IF only mode, the second is RF mode.
 *	IF only mode:
 *	In the IF only mode, the RF output is switched off. The IF output can be set to the frequency
 *	ranges defined in the pIfRangeList.
 *	RF mode:
 *	In the RF mode, the RF output is switched on. The RF output frequency is set, the value in the IF
 *	frequency field is ignored. 
 */
void SetTerrestrialModulatorRfIf()
{
	dvb_if_rf_parameters IfRfParams;
	
	// set RF and IF parameters
	if (IfOnlyMode)		// IF only mode
	{
		IfRfParams.RfIfSel			= TRUE;			// enable IF only mode
		IfRfParams.IfFrequency		= 70000000;		// set IF frequency to 70MHz, or any other value specified in the pIfRangeList
		IfRfParams.RfFrequency		= 0;			// RF output frequency is ignored
		IfRfParams.RfoutputLevel	= 0;			// RF output level is ignored
	}
	else				// RF mode
	{
		IfRfParams.RfIfSel			= FALSE;		// disable IF only mode
		IfRfParams.IfFrequency		= 0;			// IF output frequency is ignored
		IfRfParams.RfFrequency		= 500000000;	// set RF frequency to 500MHz, or any other value between nModRFFreqMin and nModRFFreqMax
		IfRfParams.RfoutputLevel	= 10;			// set RF output level to 1dBm, or any other value between nRFLevelMin and nRFLevelMax
	}

	// send parameter list
	pBoard->DvbSource(SRC_SET_MOD_IF_RF_PARAMS, &IfRfParams);
}

/*
 *	Get the RF frequency locked status.
 */
void GetTerrestrialModulatorRfIfStatus()
{
	dvb_if_rf_parameters IfRfParams;

	// get the RF/IF parameters
	pBoard->DvbSource(SRC_GET_MOD_IF_RF_STAT, &IfRfParams);

	// get the RF locked bit
	if (IfRfParams.RfLocked)
		m_LedRfLocked.Set(CLedControl::LED_GREEN);
	else
		m_LedRfLocked.Set(CLedControl::LED_RED);
}
