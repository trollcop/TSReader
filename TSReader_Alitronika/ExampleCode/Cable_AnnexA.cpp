static CATBoard *pBoard;

...

/*
 *	ANNEX A  Cable tuner
 *
 *	Tune the Cable tuner to the frequency
 *
 *	NOTE: Once the RF signal is lost and re-applied, the Cable tuner
 *	does NOT automatically lock on the new signal.
 *	SetCalbleStation must be called again to establish a lock.
 *	This can be automated by calling SetCableStation_AnnexA periodically
 *	when the SRC_HAS_CARRIER status is FALSE.
 *	Parameters:
 *		nFreq in Hz.
 */
void SetCableStation_AnnexA(u32 nFreq)
{
	dvb_frontend_parameters Info;

	memset(Info, 0, sizeof(Info));
	Info.frequency = nFreq;
	Info.inversion = INVERSION_AUTO;
	Info.u.qam.modulation = QAM_AUTO;
	Info.u.qam.symbol_rate= m_nSymbolRate;	

	pBoard->DvbSource(SRC_SET_FRONTEND, &Info);
}

/*
 *	Read the status of the Cable tuner
 */
void GetCableInfo_AnnexA()
{
	src_status_t status;
	u16 nSigStrength, nNoise, nBitrateError;
	struct dvb_frontend_parameters Info;
	CString sTxt;

	/*
	 *	On/Off status indicators
	 */
	pBoard->DvbSource(SRC_READ_STATUS, &status);

	m_CarrLed.Set(!(status & SRC_HAS_CARRIER));		// Carrier detected LED control
	m_SyncLed.Set(!(status & SRC_HAS_SYNC));		// Transport stream sync lock LED control

	/*
	 *	Signal strength
	 */
	pBoard->DvbSource(SRC_READ_SIGNAL_STRENGTH, &nSigStrength);
	m_StrengthBar.SetPos(nSigStrength);

	/*
	 *	Get the actual tuned frequency
	 */
	pBoard->DvbSource(SRC_GET_FRONTEND, &Info);
	sTxt.Format("%6.3f MHz", Info.frequency/1000000.0);
	m_ctrlCurrFreq.SetWindowText(sTxt);	// in [MHz].

	/*
	 *	Get the demodulator info
	 */

	sTxt.Empty();
	sTxt.Format("%li Ks/S",Info.u.qam.symbol_rate);
	m_txtSymbolRate.SetWindowText(sTxt);

	sTxt.Empty();
	sTxt.Format("%s",(Info.inversion == INVERSION_ON ? "On" :
					 (Info.inversion == INVERSION_OFF? "Off":
					  "Auto")));
	m_txtIQSwap.SetWindowText(sTxt);

	sTxt.Empty();
	switch(Info.u.qam.modulation)
	{
	case QAM_16:
		sTxt.Format("QAM 16");
		break;
	case QAM_32:
		sTxt.Format("QAM 32");
		break;
	case QAM_64:
		sTxt.Format("QAM 64");
		break;
	case QAM_128:
		sTxt.Format("QAM 128");
		break;
	case QAM_256:
		sTxt.Format("QAM 256");
		break;
	default:
		sTxt.Format("Auto");
	}
	m_txtModulation.SetWindowText(sTxt);
}
