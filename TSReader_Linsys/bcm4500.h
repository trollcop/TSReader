BOOL OpenUSBDriver();
BOOL bcm4500_get_legacy_status(BYTE ** buffer);
BOOL bcm4500_get_turbo_status(BYTE ** buffer);
BOOL bcm4500_tune(unsigned long freq_hz);
BOOL bcm4500_acquire(BYTE byte2, BYTE byte3);
BOOL bcm4500_edit_symbol_rate_list(BYTE sym_idx, DWORD sym_rate_hz);
BOOL bcm4500_acquire2(BYTE byte2, BYTE byte3, DWORD baud_offset_hz, DWORD carrier_offset_hz,
					  BYTE ctl_flags1, BYTE ctl_flags2, BYTE ctl_flags3);

BOOL WriteBCM4500Memory(HWND hDlg);
void SetupDNIF();
void ReadStatus();

