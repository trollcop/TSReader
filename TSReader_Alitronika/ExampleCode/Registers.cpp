static CATBoard *pBoard;

...

/**
 *	This function set the mode to DVB
 *	see \ref ATREGISTRY
 */

void SetRecModeDVB()
{
	CAtRegisters RegAcc(pBoard);
	ATREGISTRY &Registers = RegAcc;
	DWORD &RecordConfig = RegAcc.m_RecordConfig;
	DWORD &PlayConfig =	RegAcc.m_PlayConfig;

	pBoard->GetRegisters();

	RecordConfig &= ~(AT_RCONF_SMP);	// SMPTE bit off
	RecordConfig |= AT_RCONF_DVB;		// DVB bit on

	...

	pBoard->UpdateRegisters();
}
