static CATBoard *pBoard;
static u32 LnbCrossoverFreq		= 11700000;		// all frequencies in Hz
static u32 LnbLocOscHighFreq	= 10600000;
static u32 LnbLocOscLowFreq		=  9750000;
static u32 nLnbExtVolt			= 1; // LNB: 1 volt extra
...

/*
 *	Tune the Satellite tuner to the frequency
 *	and set the transponder's Symbol rate.
 *	Parameters:
 *		nFreq in Hz.
 *		nSymbRate in symbols/sec
 *		PolarityVert TRUE for Vertical polarity
 */
void SetDVBsourceStation(u32 nFreq, u32 nSymbRate, u32 PolarityVert)
{
	dvb_frontend_parameters parms;

	memset(parms, 0, sizeof(parms));
	src_sec_voltage_t LnbVoltage = PolarityVert ? SEC_VOLTAGE_13 : SEC_VOLTAGE_18;

	pBoard->DvbSource(SRC_SET_VOLTAGE, &LnbVoltage);
	pBoard->DvbSource(SRC_ENABLE_HIGH_LNB_VOLTAGE, (void*)nLnbExtVolt);
	
	src_sec_tone_mode_t eBand;
	
	/*
	 *	Correct the receiver frequency with the LNB's active local oscillator.
	 */
	if (nFreq >= LnbCrossoverFreq)
	{
		parms.frequency = static_cast<u32>(nFreq - LnbLocOscHighFreq);
		eBand = SEC_TONE_ON;
	}
	else
	{
		parms.frequency = static_cast<u32>(nFreq - LnbLocOscLowFreq);
		eBand = SEC_TONE_OFF;
	}

	/*
	 *	Set the LNB to the requested High/Low band
	 */
	pBoard->DvbSource(SRC_SET_TONE, (void*)SEC_TONE_OFF);
	pBoard->DvbSource(SRC_SET_TONE, (void*)eBand);

	parms.inversion = INVERSION_AUTO;

	parms.u.qpsk.fec_inner = FEC_AUTO;
	parms.u.qpsk.symbol_rate = nSymbRate;

	pBoard->DvbSource(SRC_SET_FRONTEND, &parms);
}

/*
 *	Read the status of the Satellite tuner
 */
void GetDVBsourceInfo()
{
	src_status_t status;
	u16 nSigStrength, nNoise, nBitrateError;
	struct dvb_frontend_parameters info;
	CString sTxt;

	/*
	 *	On/Off status indicators
	 */
	pBoard->DvbSource(SRC_READ_STATUS, &status);

	m_VitLed.Set(!(status & SRC_HAS_VITERBI));		// Viterbi LED control
	m_SignalLed.Set(!(status & SRC_HAS_SIGNAL));	// Signal LED control
	m_CarrLed.Set(!(status & SRC_HAS_CARRIER));		// Carrier detected LED control
	m_SyncLed.Set(!(status & SRC_HAS_SYNC));		// Transport stream sync lock LED control
	m_LockLed.Set(!(status & SRC_HAS_LOCK));		// All required status is OK
	
	/*
	 *	Signal to Noise indication
	 */
	pBoard->DvbSource(SRC_READ_SNR, &nNoise);
	m_NoiseProgress.SetPos(nNoise);
	
	/*
	 *	Bitrate errors
	 */
	pBoard->DvbSource(SRC_READ_BER, &nBitrateError);
	sTxt.Format("%.3lf",(nBitrateError*100.0)/0xffff);
	m_ctrlBER.SetWindowText(sTxt);

	/*
	 *	Signal strength
	 */
	pBoard->DvbSource(SRC_READ_SIGNAL_STRENGTH, &nSigStrength);
	m_StrengthBar.SetPos(nSigStrength);

	/*
	 *	Get the actual tuned frequency
	 *	The Low/High band base frequencies are taken from the LNB data.
	 */
	pBoard->DvbSource(SRC_GET_FRONTEND, &info);
	sTxt.Format("%lf [MHz]", info.frequency/1000000.0+(m_Freq >= iLnb.dBandSwGhz ? 
												iLnb.dLoHiGHz : iLnb.dLoLoGHz));
	m_ctrlCurrFreq.SetWindowText(sTxt);
}
