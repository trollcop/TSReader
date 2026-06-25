// General stuff
BOOL CloseSerialPort(void);
BOOL OpenSerialPort(int nBaudRate, int nByteSize, int nParity, int nStopBits, BOOL fDTR);
int SendSerial(unsigned char * lpByte, DWORD dwBytesToWrite);

// Tandberg Alteia Plus
BOOL SetAlteiaChannel(int nTSID, int nNID, int nSID);
BOOL TuneAlteia(int nFrequency, int nSymbolRate, char * szFEC, int nMode, int nRFInput);
BOOL GetAlteiaTunerStatus(void);
void SetVoltageAndToneAlteia(int nPolarity, BOOL f22KHz, int nLNBFrequency);

// Motorola DSR-4800
void Select4800Channel(int nChannel);
void Tune4800(int nFrequency, int nSymbolRate, int nMode, int nRFInput, int nFEC);
BOOL Get4800TunerStatus(void);
void SetVoltageAndTone4800(int nPolarity, BOOL f22KHz, int nLNBFrequency);

// Newtec DVB-2063
BOOL TuneNewtec(int nFrequency, int nSymbolRate, char * szFECAndModulation, int nRFInput);
void SetVoltageAndToneNewtec(int nPolarity, BOOL f22KHz);
BOOL GetNewtecTunerStatus(void);
