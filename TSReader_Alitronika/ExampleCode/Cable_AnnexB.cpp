static CATBoard *pBoard;

...

/*
 *	ANNEX B  Cable tuner
 *
 *	Tune the Cable tuner to the frequency
 *
 *	Parameters:
 *		nFreq in Hz.
 */
void SetCableStation_AnnexB(u32 nFreq)
{
	dvb_frontend_parameters Info;

	memset(Info, 0, sizeof(Info));
	Info.frequency = nFreq;
	Info.u.qam.modulation = QAM_64;		// Or QAM_256

	pBoard->DvbSource(SRC_SET_FRONTEND, &Info);
}

/*
 *	Read the status of the Annex B Cable tuner
 */
void GetCableInfo_AnnexB()
{
	src_status_dvbcb_t eStatus;
	u16 nSigStrength, nNoise, nBitrateError;
	struct dvb_frontend_parameters Info;
	CString sTxt;

	/*
	 *	On/Off status indicators
	 */

	pBoard->DvbSource(SRC_READ_STATUS, &eStatus);
	m_ledFatLock.Set(!(eStatus & SRC_HAS_FAT_LOCK));			// Forward App Chan Lock
	m_ledQamSyncLock.Set(!(eStatus & SRC_HAS_QAMSYNC_LOCK));	// QAM in lock
	m_ledMpegLock.Set(!(eStatus & SRC_HAS_MPEG_LOCK));			// Transport stream Lock

	/*
	 *	Signal to noise ratio
	 */

	u16 nSigSnr;
	pBoard->DvbSource(SRC_READ_SNR, &nSigSnr);
	if(!m_bIgnoreTimer)
	{
		m_ctrlSignalSnr.SetPos(nSigSnr);
	}

	/*
	 *	Reed Solomon error count
	 */

	u8 nRSErrCnt;
	pBoard->DvbSource(SRC_READ_BER, &nRSErrCnt);
	sTxt.Format("%u", nRSErrCnt);
	m_txtBER.SetWindowText(sTxt);

	/*
	 *	Get QAM mode
	 */

	dvb_frontend_parameters Info;
	pBoard->DvbSource(SRC_GET_FRONTEND, &Info);
	sTxt.Empty();
	switch(Info.u.qam.modulation)
	{
	default:
	case QAM_64:
		sTxt.Format("QAM 64");
		break;
	case QAM_256:
		sTxt.Format("QAM 256");
		break;
	}
	m_txtModulation.SetWindowText(sTxt);
}
