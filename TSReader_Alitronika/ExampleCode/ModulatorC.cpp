static CATBoard *pBoard;

...

/****************************************************************************************************/
// set modulation parameters

/*
 *	Initializes the Cable modulator.
 *	This fuction must be called before any other call to the modulator
 */
void InitCableModulator()
{
	pBoard->DvbSource(SRC_INIT, NULL);
}

/*
 *	Set the modulator parameters.
 *	The Cable modulator supports four different modes: J83-annexA, J83-annexB, J83-annexC and DVB-C
 *	Every mode has its own capabilities:
 *	J83-annexA:
 *		modulation:		QAM16, QAM32, QAM64
 *		filter_rolloff:	FLT_ALPHA_15,
 *		interleaver:	INTLEAV_I12_J17,
 *		inversion:		INVERSION_OFF, INVERSION_ON,
 *		symbol_rate:	0-6950000 MSymbols/s
 *		annexb_enable:	FALSE.
 *	J83-annexB:
 *		modulation:		QAM64, QAM256
 *		filter_rolloff:	FLT_ALPHA_18 for QAM64, FLT_ALPHA_12 for QAM256
 *		interleaver:	INTLEAV_I128_J1, INTLEAV_I128_J2, INTLEAV_I128_J3, INTLEAV_I128_J4, INTLEAV_I128_J5, INTLEAV_I128_J6, INTLEAV_I128_J7, INTLEAV_I128_J8, INTLEAV_I64_J2, INTLEAV_I32_J4, INTLEAV_I16_J8, INTLEAV_I8_J16
 *		inversion:		INVERSION_OFF, INVERSION_ON,
 *		symbol_rate:	5056941 MSymbols/s for QAM64, 5360537 MSymbols/s for QAM256
 *		annexb_enable:	TRUE.
 *	J83-annexC:
 *		modulation:		QAM64
 *		filter_rolloff:	FLT_ALPHA_13,
 *		interleaver:	INTLEAV_I12_J17,
 *		inversion:		INVERSION_OFF, INVERSION_ON,
 *		symbol_rate:	0-5300000 MSymbols/s
 *		annexb_enable:	FALSE.
 *	DVB-C:
 *		modulation:		QAM16, QAM32, QAM64, QAM128, QAM256
 *		filter_rolloff:	FLT_ALPHA_15,
 *		interleaver:	INTLEAV_I12_J17,
 *		inversion:		INVERSION_OFF, INVERSION_ON,
 *		symbol_rate:	0-6950000 MSymbols/s
 *		annexb_enable:	FALSE.
 */
void SetCableModulatorParams()
{
	dvb_at2700_parameters params;

	memset(params, 0, sizeof(params));
	
	params.modulation		= QAM_16;			// set modulation. (caps: QAM_16, QAM_32, QAM_64, QAM_128, QAM_256)
	params.filter_rolloff	= FLT_ALPHA_15;		// set filter roll-off. (caps: FLT_ALPHA_12, FLT_ALPHA_13, FLT_ALPHA_15, FLT_ALPHA_18)
	params.interleaver		= INTLEAV_I12_J17;	// set interleaver mode. (caps: INTLEAV_I128_J1, INTLEAV_I128_J2, INTLEAV_I128_J3, INTLEAV_I128_J4, INTLEAV_I128_J5, INTLEAV_I128_J6, INTLEAV_I128_J7, INTLEAV_I128_J8, INTLEAV_I64_J2, INTLEAV_I32_J4, INTLEAV_I16_J8, INTLEAV_I8_J16, INTLEAV_I12_J17)
	params.inversion		= INVERSION_OFF;	// set inversion.
	params.symbol_rate		= 6000000;			// set symbol rate. (in MSymbols/s)
	params.annexb_enable	= FALSE;			// enable annexB mode.

	pBoard->DvbSource(SRC_SET_MOD_PARAMS, &params);
}

/*
 *	Read the status of the modulator
 */
void GetCableModulatorStatus()
{
	src_status_at2700 eStatus;

	/*
	 *	Get modulator status
	 *
	 *	All CH1, CH2 and CH3 flags can be ignored because the Cable modulator does not
	 *	support multiple parallel TS streams. Only one (CH0) channel is supported.
	 */
	pBoard->DvbSource(SRC_GET_MOD_STAT, &eStatus);

 	if (eStatus & DST_HAS_PCR_TS_INSYNC_CH0)				// check if modulator is in sync
 		m_LedInSyncPcr.Set(CLedControl::LED_GREEN);
 	else
 		m_LedInSyncPcr.Set(CLedControl::LED_RED);

 	if (eStatus & DST_HAS_PCR_TS_204PKT_CH0)				// check transport stream packet size
 		m_Led204PckPcr.Set(CLedControl::LED_GREEN);
 	else
 		m_Led204PckPcr.Set(CLedControl::LED_GRAY);

 	if (eStatus & DST_HAS_PCR_TS_OFLOW_CH0)				// check if overflow has occured
 		m_LedTsOFlowPcr.Set(CLedControl::LED_RED);
 	else
 		m_LedTsOFlowPcr.Set(CLedControl::LED_GRAY);
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
void GetCableModulatorRfIfInfo()
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
void SetCableModulatorRfIf()
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
void GetCableModulatorRfIfStatus()
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
