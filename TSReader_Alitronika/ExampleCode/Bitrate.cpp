#define STATUS_TIMER	60
#define STATUS_MS		100

...

BOOL CmyRecordView::OnInitDialog()
{
	...
	pBoard->InitBitrateFilter(STATUS_MS);
	SetTimer(STATUS_TIMER, STATUS_MS, 0);
}


void CmyRecordView::OnTimer(UINT nIDEvent)
{
	...
	int Bitrate = pBoard->GetInputBitrate();
	m_Str.Format("%.2f", Bitrate/1e6);
	m_BrStatic.SetWindowText(m_Str);

	// Now read the SDRAM FIFO counter in the Progress Control.
	// The indication is inverted to show the FREE FIFO space.
	m_SdramFillCtrl.SetPos(100 * (SDRAMFIFO_SZ - Registers.m_RecFifoContent) / SDRAMFIFO_SZ);
}
