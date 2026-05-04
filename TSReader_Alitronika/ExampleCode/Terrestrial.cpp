static CATBoard *pBoard;

...

/*
 *	Tune the Terrestrial tuner to the frequency
 *	and set the transponder's Bandwidth.
 *	Parameters:
 *		nFreq in Hz.
 *		nBandWidth in MHz, either 7 or 8.
 */
void SetTerrestrialStation(u32 nFreq, u32 nBandWidth)
{
	dvb_frontend_parameters parms;

	memset(parms, 0, sizeof(parms));
	parms.frequency = nFreq;
	parms.inversion = INVERSION_AUTO;
	parms.u.ofdm.bandwidth = nBandWidth == 8 ? BANDWIDTH_8_MHZ : BANDWIDTH_7_MHZ;

	pBoard->DvbSource(SRC_SET_FRONTEND, &parms);
}

/*
 *	Read the status of the Terrestrial tuner
 */
void GetTerrestrialInfo()
{
	src_status_t status;
	u16 nSigStrength, nNoise, nBitrateError;
	struct dvb_frontend_parameters Info;
	CString sTxt;

	/*
	 *	On/Off status indicators
	 */
	pBoard->DvbSource(SRC_READ_STATUS, &status);

	m_VitLed.Set(!(status & SRC_HAS_VITERBI));		// Viterbi LED control
	m_SignalLed.Set(!(status & SRC_HAS_SIGNAL));	// Signal LED control
	m_CarrLed.Set(!(status & SRC_HAS_CARRIER));		// Carrier detected LED control
	m_SyncLed.Set(!(status & SRC_HAS_SYNC));		// Transport stream sync lock LED control

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
	 */
	pBoard->DvbSource(SRC_GET_FRONTEND, &Info);
	sTxt.Format("%6.3f [MHz]", Info.frequency/1000000.0);
	m_ctrlCurrFreq.SetWindowText(sTxt);	// in [MHz].

	/*
	 *	Get the current constellation
	 */
	switch(Info.u.ofdm.constellation)
	{
	case QPSK:
		sTxt.Format("QPSK");
		break;
	case  QAM_16:
		sTxt.Format("QAM 16");
		break;
	case  QAM_32:
		sTxt.Format("QAM 32");
		break;
	case  QAM_64:
		sTxt.Format("QAM 64");
		break;
	case  QAM_128:
		sTxt.Format("QAM 128");
		break;
	case  QAM_256:
		sTxt.Format("QAM 256");
		break;
	}
	m_txtConstellation.SetWindowText(sTxt);
	
	/*
	 *	Get the guard interval
	 */
	sTxt.Empty();
	switch(Info.u.ofdm.guard_interval)
	{
	case  GUARD_INTERVAL_1_32:
		sTxt.Format("1/32");
		break;
	case  GUARD_INTERVAL_1_16:
		sTxt.Format("1/16");
		break;
	case  GUARD_INTERVAL_1_8:
		sTxt.Format("1/8");
		break;
	case  GUARD_INTERVAL_1_4:
		sTxt.Format("1/4");
		break;
	}
	m_txtGuardInterval.SetWindowText(sTxt);
	
	/*
	 *	Get the current hierarchy
	 */
	sTxt.Empty();
	switch(Info.u.ofdm.hierarchy_information)
	{
	case  HIERARCHY_NONE:
		sTxt.Format("None");
		break;
	case  HIERARCHY_1:
		sTxt.Format("1");
		break;
	case  HIERARCHY_2:
		sTxt.Format("2");
		break;
	case  HIERARCHY_4:
		sTxt.Format("4");
		break;
	}
	m_txtHierarchy.SetWindowText(sTxt);

	/*
	 *	Get the current High and Low code rate
	 */
	sTxt.Empty();
	CStatic *pS[2] = {&m_txtHPSR, &m_txtLPSR};
	src_code_rate_t *pCr[2] = {&Info.u.ofdm.code_rate_HP, &Info.u.ofdm.code_rate_LP};
	for (int i=0; i<2; ++i)
	{
		switch(*pCr[i])
		{
		case FEC_AUTO:
			sTxt.Format("Auto");
			break;
		case FEC_1_2:
			sTxt.Format("1/2");
			break;
		case FEC_2_3:
			sTxt.Format("2/3");
			break;
		case FEC_3_4:
			sTxt.Format("3/4");
			break;
		case FEC_4_5:
			sTxt.Format("4/5");
			break;
		case FEC_5_6:
			sTxt.Format("5/6");
			break;
		case FEC_6_7:
			sTxt.Format("6/7");
			break;
		case FEC_7_8:
			sTxt.Format("7/8");
			break;
		default:
			sTxt.Empty();
		}
		pS[i]->SetWindowText(sTxt);
	}

	/*
	 *	Get the current transmission mode
	 */
	sTxt.Empty();
	switch(Info.u.ofdm.transmission_mode)
	{
	case TRANSMISSION_MODE_2K:
		sTxt.Format("2k");
		break;
	case TRANSMISSION_MODE_8K:
		sTxt.Format("8k");
		break;
	default://TRANSMISSION_MODE_AUTO
		sTxt.Format("Auto");
	}
	m_txtTMode.SetWindowText(sTxt);
}
