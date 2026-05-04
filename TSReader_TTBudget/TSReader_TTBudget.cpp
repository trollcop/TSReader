#include "stdafx.h"
#include <DVBDLLInit.h>
#include "TSReader_TTBudget.h"

#include "..\CI-CAM.h"

#include "../SampleSource/inc/sources.h"

PSOURCESTRUCT ss;
char * szCmdLinePtr;
BOOL fNeedTuneDialog = TRUE;
int nFrequency;
BOOL fFirstDiSEqC;
CDVBFrontend::CHANNEL_TYPE ch;

CDVBComnIF CI;
BOOL fCIInited;
int nSetProgram;
int nCIProgram;

#ifdef DVBS
int nSymbolRate;
int nPolarity;
int nLNBFrequency;
int n22KHz;
int nDiSEqCInput;
#ifndef DVBS2
char gszSourceName[] = {"TechnoTrend Budget DVB-S"};
#else DVBS2
int nADVModulationMode;
int nCodeRate;
char gszSourceName[] = {"TechnoTrend Budget DVB-S2"};
#endif DVBS2
#endif DVBS

#ifdef DVBT
int fSpectrumInversion;
int nBandwidth;
BOOL fAntennaPower = FALSE;
char gszSourceName[] = {"TechnoTrend Budget DVB-T"};
#endif DVBT

#ifdef DVBC
int nSymbolRate;
int nQAM;
int fSpectrumInversion;
int nBandwidth;
char gszSourceName[] = {"TechnoTrend Budget DVB-C"};
#endif DVBC

CDVBFrontend m_FE;
CDVBBoardControl m_BoardControl;
CRITICAL_SECTION csSignal;
#ifdef DVBT
BOOL fFirstTime;
#endif DVBT
char szLastTune[128] = {"n/a"};
char szLastSignalReport[128] = {"n/a"};

extern "C" {
	BOOL __cdecl SourceHelper_ValidateSourceContainer(PSOURCESTRUCT pss);
}

// Hardware specific stuff
#define RX_SIZE 99828
DWORD WINAPI ReadTTThread(LPVOID lpv)
{
	int nTSBufferIndex = 0;
	int retlen;
	int nPollCounter = 25;
	__int64 nLoopCount = 0;

	ss->fReadThreadTerminated = FALSE;
	ss->fTerminateReadThread = FALSE;

	while (!ss->fTerminateReadThread)
	{
		if (nCIProgram != -1)
		{
			DVB_ERROR error;
			if (nSetProgram != -1)
			{
				error = CI.ClearProgram(nSetProgram);
				nSetProgram = -1;
			}
			//error = CI.ReadPSIFast(nProgram); // SID
			error = CI.SetProgram(nCIProgram, NULL);
			{
				char szTemp[128];
				wsprintf(szTemp, "TTBudget CI: CI.SetProgram(%d, NULL) returned %d\n", nCIProgram, error);
				OutputDebugString(szTemp);
			}
			if (error == 0)
				nSetProgram = nCIProgram;
			nCIProgram = -1;
		}

		retlen = m_BoardControl.TSRecordRead(ss->tsb[nTSBufferIndex].pData, RX_SIZE);
		if (retlen > 0)
		{
			if (nLoopCount++ > 10)
			{				
				EnterCriticalSection(&ss->csPIDCounter);
				ss->nLastSecondByteCounter += retlen;
				LeaveCriticalSection(&ss->csPIDCounter);

				ss->tsb[nTSBufferIndex].nSize = retlen;
				nTSBufferIndex++;
				if (nTSBufferIndex == MAX_TS_BUFFERS)
					nTSBufferIndex = 0;
				EnterCriticalSection(&ss->csTSBuffersInUse);
				ss->nTSBuffersInUse++;
				LeaveCriticalSection(&ss->csTSBuffersInUse);
			}
		}
		else
		{
			if (nPollCounter++ > 25)
			{
				BOOL lock = FALSE;
				CDVBFrontend::SIGNAL_TYPE Signal;

				DVB_ERROR error = m_FE.GetState(lock, &Signal);
				if(error == DVB_ERR_NONE)
				{
					EnterCriticalSection(&csSignal);
					if (lock)
						sprintf(szLastSignalReport, "Locked SNR %.1f dB", Signal.SNRdB / 10.0f);
					else
						lstrcpy(szLastSignalReport, "Unlocked SNR 0.0 dB");
					LeaveCriticalSection(&csSignal);
#ifdef DVBS
					if (!lock)
						m_FE.SetChannel(ch);
#endif DVBS
				}
				nPollCounter = 0;
			}
			Sleep(20);
		}
	}

	CloseHandle(ss->hReadDataThread);
	ss->fReadThreadTerminated = TRUE;
	ss->fTerminateReadThread = FALSE;
	OutputDebugString("ReadTTThread-\n");
	return 0;
}

BOOL TSReader_Start()
{
	DWORD dwThreadID;

	if (!fCIInited)
	{
		DVB_ERROR error = CI.Open();
		if (error == DVB_ERR_NONE)
			fCIInited = TRUE;
	}

	ss->hReadDataThread = CreateThread(NULL, 0, ReadTTThread, (LPVOID)0, CREATE_SUSPENDED, &dwThreadID);
	SourceHelper_SetWorkerThreadPriorities(FALSE);
	m_BoardControl.EnableDataDMA(TRUE);	
	m_BoardControl.TSRecordOnOff(TRUE);
	ResumeThread(ss->hReadDataThread);

	return TRUE;
}

BOOL TSReader_Stop()
{
	ss->fTerminateReadThread = TRUE;
	while (ss->fReadThreadTerminated == FALSE)
		Sleep(50);

	m_BoardControl.TSRecordOnOff(FALSE);
	m_BoardControl.EnableDataDMA(FALSE);

	EnterCriticalSection(&ss->csTSBuffersInUse);
	ss->nTSBuffersInUse = -1000;
	LeaveCriticalSection(&ss->csTSBuffersInUse);

	return TRUE;
}

BOOL TSReader_Init(PSOURCESTRUCT pss)
{
	int nDevice, nDevices;

	if (SourceHelper_ValidateSourceContainer(pss) == FALSE)
		return FALSE;
	InitializeCriticalSection(&csSignal);

	ss = pss;
	fFirstDiSEqC = TRUE;
	nSetProgram = -1;
	nCIProgram = -1;

#ifdef DVBT
	if (fFirstTime == TRUE)
	{
		fFirstTime = FALSE;
#endif DVBT
		nDevices = CDVBCommon::GetNumberOfDevices();
		for (nDevice = 0; nDevice < nDevices; nDevice++)
		{
			if(CDVBCommon::OpenDevice(nDevice))
			{
				m_FE.Init();
#ifdef DVBS
#ifndef DVBS2
				if ((m_FE.GetType() == CDVBFrontend::FE_SATELLITE)
				&& !(m_FE.GetCapabilities() & HAS_DVBS2FE))
#else DVBS2
				if ((m_FE.GetType() == CDVBFrontend::FE_SATELLITE)
				&& (m_FE.GetCapabilities() & HAS_DVBS2FE))
#endif DVBS2
#endif DVBS
#ifdef DVBT
				if(m_FE.GetType() == CDVBFrontend::FE_TERRESTRIAL)
#endif DVBT
#ifdef DVBC
				if(m_FE.GetType() == CDVBFrontend::FE_CABLE)
#endif DVBC
				{
					if (ss->nSourceIndex == 0)
						break;
					ss->nSourceIndex--;
				}
				CDVBCommon::CloseDevice();			
			}
		}

		if (nDevice == nDevices)
		{
			MessageBox(NULL, "Unable to locate a suitable PCI card", gszSourceName, MB_ICONSTOP);
			return FALSE;
		}
#ifdef DVBT
	}
	if (m_FE.GetCapabilities() & HAS_ANT_PWR)
		fAntennaPower = TRUE;
	else 
		fAntennaPower = FALSE;
#endif DVBT

	return TRUE;
}

BOOL TSReader_DeInit()
{
	if (fCIInited)
	{
		if (nSetProgram != -1)
		{
			CI.ClearProgram(nSetProgram);
			nSetProgram = -1;
		}
	}

#ifndef DVBT
//	CDVBCommon::CloseDevice();
#endif DVBT
	DeleteCriticalSection(&csSignal);

	return TRUE;
}

void SetupLastTune()
{
#ifdef DVBS
	char szPolarity[4] = {"H/L"};
	char szModulation[16] = {0};

	if (ss->nPolarity == 0)
		lstrcpy(szPolarity, "V/R");
	lstrcpy(szModulation, "DVB QPSK");
#ifdef DVBS2
	if (ss->nADVModulationMode == ADV_MOD_DVBS2)
		lstrcpy(szModulation, "DVB-S2");
#endif DVBS2

	wsprintf(szLastTune, "%d MHz %s %d %s", ss->nFrequency, szPolarity, ss->nSymbolRate, szModulation);
#endif DVBS
#ifdef DVBT
	sprintf(szLastTune, "%.3f MHz", ss->nFrequency / 1000.0);
#endif DVBT
#ifdef DVBC
	sprintf(szLastTune, "%.1f MHz", ss->nFrequency / 1000.0);
#endif DVBC

	EnterCriticalSection(&csSignal);
	lstrcpy(szLastSignalReport, "n/a");
	LeaveCriticalSection(&csSignal);
}


#define DISEQC_HIGH_NIBLE	0xF0
#define DISEQC_LOW_BAND		0x00
#define DISEQC_HIGH_BAND	0x01
#define DISEQC_VERTICAL		0x00
#define DISEQC_HORIZONTAL	0x02
#define DISEQC_POSITION_A	0x00
#define DISEQC_POSITION_B	0x04
#define DISEQC_OPTION_A		0x00
#define DISEQC_OPTION_B		0x08

BOOL TSReader_Tune()
{
	SetupLastTune();

#ifdef DVBS
	// DiSEqC commands first
	switch(ss->nDiSEqCInput)
	{
	case 0:
		break;
	case 1:
	case 2:
	case 3:
	case 4:
		{
			BYTE DiPar[4];
			DVB_ERROR err;

			DiPar[0] = 0xE0; DiPar[1] = 0x10; DiPar[2] = 0x38;
			DiPar[3] = DISEQC_HIGH_NIBLE;
			DiPar[3] |= (ss->nPolarity == 0) ? DISEQC_VERTICAL : DISEQC_HORIZONTAL;
			DiPar[3] |= ss->n22KHz ? DISEQC_HIGH_BAND : DISEQC_LOW_BAND;
			switch(ss->nDiSEqCInput) 
			{	case 1: DiPar[3] |= DISEQC_POSITION_A | DISEQC_OPTION_A; break;
				case 2: DiPar[3] |= DISEQC_POSITION_B | DISEQC_OPTION_A; break;
				case 3: DiPar[3] |= DISEQC_POSITION_A | DISEQC_OPTION_B; break;
				case 4: DiPar[3] |= DISEQC_POSITION_B | DISEQC_OPTION_B; break;
			}
			for(int i=0; i<3; i++)
			{
				err = m_FE.SendDiSEqCMsg(DiPar, 4, 0xff);
				DiPar[0] = 0xE1; // Repetition
				Sleep(120); // Time between Diseqc-Sequences
			}
		}
		break;
	case 5:
		{
			//m_FE.SendDiSEqCMsg(1, FALSE, FALSE, FALSE, FALSE, 10);
			BYTE DiPar[4];
			DVB_ERROR err;

			err = m_FE.SendDiSEqCMsg(DiPar, 0, 0);
		}
		break;
	case 6:
		{
			//m_FE.SendDiSEqCMsg(1, TRUE, TRUE, FALSE, FALSE, 10);
			BYTE DiPar[4];
			DVB_ERROR err;

			err = m_FE.SendDiSEqCMsg(DiPar, 0, 1);
		}
		break;
	}

	// Tune frontend
	ch.dvb_s.dwFreq = ss->nFrequency * 1000;
	ch.dvb_s.dwSymbRate = ss->nSymbolRate * 1000;
	ch.dvb_s.dwLOF = ss->nLNBFrequency * 1000;
	ch.dvb_s.bF22kHz = ss->n22KHz;
	switch(ss->nPolarity)
	{
	case 1:
		ch.dvb_s.LNB_Power = CDVBFrontend::POL_HORZ;
		break;
	case 0:
		ch.dvb_s.LNB_Power = CDVBFrontend::POL_VERT;
		break;
	case -1:
		ch.dvb_s.LNB_Power = CDVBFrontend::POWER_OFF;
		break;
	}
	ch.dvb_s.Inversion = CDVBFrontend::SI_AUTO;
	ch.dvb_s.Viterbi = CDVBFrontend::VR_AUTO;		
#ifdef DVBS2
	if (ss->nADVModulationMode == ADV_MOD_DVB_QPSK)
		ch.dvb_s.Mode = CDVBFrontend::MODE_DVB_S;
	else if (ss->nADVModulationMode == ADV_MOD_DVBS2)
		ch.dvb_s.Mode = CDVBFrontend::MODE_DVB_S2;
	else
	{
		MessageBox(ss->hWndTSReader, "This device supports DVB-S and DVB-S2 only", gszSourceName, MB_ICONSTOP);
		return FALSE;
	}

#endif DVBS2
		
#endif DVBS
#ifdef DVBT
	ch.dvb_t.dwFreq = ss->nFrequency;

	if (fAntennaPower)
		m_FE.SetAntennaPower(ss->nPolarity);
	if (m_FE.GetCapabilities() & HAS_SI_AUTO)
		ch.dvb_t.Inversion = CDVBFrontend::SI_AUTO;
	else
	{
		if (ss->fSpectrumInversion == 0)
			ch.dvb_t.Inversion = CDVBFrontend::SI_OFF;
		else
			ch.dvb_t.Inversion = CDVBFrontend::SI_ON;
	}
	if(m_FE.GetCapabilities() & HAS_BW_AUTO)
		ch.dvb_t.BandWidth = CDVBFrontend::BW_AUTO;
	else
	{
		CDVBFrontend::BANDWITH_TYPE bwmin, bwmax;
		m_FE.GetBandwidthRange(bwmin, bwmax);
		if(bwmin == bwmax)
			ch.dvb_t.BandWidth = bwmin;
		else
		{
			switch(ss->nBandwidth)
			{
			case 0:
				ch.dvb_t.BandWidth = CDVBFrontend::BW_6MHz;
				break;
			case 1:
				ch.dvb_t.BandWidth = CDVBFrontend::BW_7MHz;
				break;
			case 2:
				ch.dvb_t.BandWidth = CDVBFrontend::BW_8MHz;
				break;
			}
		}
	}
	ch.dvb_t.bScan = FALSE;
/*		{
			char szTemp[2048];
			wsprintf(szTemp, "ch.dvb_t.dwFreq = %d\nch.dvb_t.Inversion = %d\nch.dvb_t.BandWidth = %d\nch.dvb_t.bScan = %d\n",
					 (int)ch.dvb_t.dwFreq, (int)ch.dvb_t.Inversion, (int)ch.dvb_t.BandWidth, (int)ch.dvb_t.bScan);
			MessageBox(NULL, szTemp, "Rod's bug tracker", MB_ICONINFORMATION);
		}*/

#endif DVBT	
#ifdef DVBC
		ch.dvb_c.dwFreq = ss->nFrequency;
		ch.dvb_c.dwSymbRate = ss->nSymbolRate * 1000;

		if(m_FE.GetCapabilities() & HAS_SI_AUTO)
			ch.dvb_c.Inversion = CDVBFrontend::SI_AUTO;
		else
		{
			if (ss->fSpectrumInversion == 0)
				ch.dvb_c.Inversion = CDVBFrontend::SI_OFF;
			else
				ch.dvb_c.Inversion = CDVBFrontend::SI_ON;
		}

		ch.dvb_c.Qam = (CDVBFrontend::QAM_TYPE)ss->nQAM;

		if(m_FE.GetCapabilities() & HAS_BW_AUTO)
			ch.dvb_c.BandWidth = CDVBFrontend::BW_AUTO;
		else
		{
			CDVBFrontend::BANDWITH_TYPE bwmin, bwmax;
			m_FE.GetBandwidthRange(bwmin, bwmax);
			if(bwmin == bwmax)
				ch.dvb_c.BandWidth = bwmin;
			else
			{
				switch(ss->nBandwidth)
				{
				case 0:
					ch.dvb_c.BandWidth = CDVBFrontend::BW_6MHz;
					break;
				case 1:
					ch.dvb_c.BandWidth = CDVBFrontend::BW_7MHz;
					break;
				case 2:
					ch.dvb_c.BandWidth = CDVBFrontend::BW_8MHz;
					break;
				}
			}
		}

/*	{
		char szTemp[2048];
		wsprintf(szTemp, "TT: ch.dvb_c.dwFreq = %d ch.dvb_c.dwSymbRate = %d\n"
						 "TT: ch.dvb_c.Inversion = %d ch.dvb_c.Qam = %d ch.dvb_c.BandWidth = %d\n",
						 ch.dvb_c.dwFreq, ch.dvb_c.dwSymbRate,
						 ch.dvb_c.Inversion, ch.dvb_c.Qam, ch.dvb_c.BandWidth);
		OutputDebugString(szTemp);
	}*/
#endif DVBC

	m_FE.SetChannel(ch);
	// Snooze after locking -- this way we'll get some
	// decent data back from the BER calculator which
	// obviously needs to be locked onto the signal for
	// a while before it can be calculated correctly.
	Sleep(250);

	// Make sure we locked
	BOOL lock = FALSE;
	CDVBFrontend::SIGNAL_TYPE Signal;
	BYTE quality = 0;

	if(m_FE.GetState(lock, &Signal) == DVB_ERR_NONE)
	{
		if(Signal.BER < 10e-4)
			quality = 75; // 75%
		else if(Signal.BER < 10e-3)
			quality = 50; // 50%
		else if(Signal.BER < 10e-2)
			quality = 25; // 25%
		else // (Signal.BER >= 10e-2)
			quality = 0; // 0%

		if(lock)
		{
			//CString t;
			//quality += 25;
			//t.Format("Signal.BER: %f (%e), Quality: %u%%, Level: %u%%\n", Signal.BER, Signal.BER, quality, Signal.SNR100);
			//OutputDebugString(t);
		}
		else
		{
			if (ss->fQuietMode == FALSE)
				MessageBox(ss->hWndTSReader, "Failed to lock signal", gszSourceName, MB_OK | MB_ICONWARNING);
			return FALSE;
		}
	}
	else
	{
		if (ss->fQuietMode == FALSE)
			MessageBox(ss->hWndTSReader, "Unable to get frontend status", gszSourceName, MB_OK | MB_ICONWARNING);
		return FALSE;
	}

	return TRUE;
}

BOOL TSReader_TuneDialog(HWND hWnd)
{
	if (ss->fDontTune == TRUE)
	{
		fNeedTuneDialog = FALSE;
		OutputDebugString("TuneDialog() set fNeedTuneDialog = FALSE\n");
	}

	{
		char szTemp[100];
		wsprintf(szTemp, "fNeedTuneDialog = %d\n", fNeedTuneDialog);
		OutputDebugString(szTemp);
	}
	if (fNeedTuneDialog)
	{
#ifdef DVBS
#ifndef DVBS2
		if (SourceHelper_DVBSTuneDialog(hWnd) == FALSE)
			return FALSE;
#else DVBS2
		if (SourceHelper_ADVTuneDialog(hWnd) == FALSE)
			return FALSE;
#endif DVBS2
#endif DVBS
#ifdef DVBT
		if (SourceHelper_DVBTTuneDialog(hWnd) == FALSE)
			return FALSE;
#endif DVBT
#ifdef DVBC
		if (SourceHelper_DVBCTuneDialog(hWnd) == FALSE)
			return FALSE;
#endif DVBC
	}
	else
	{
		ss->nFrequency = nFrequency;
#ifdef DVBS
		ss->nSymbolRate = nSymbolRate;
		ss->nPolarity = nPolarity;
		ss->nLNBFrequency = nLNBFrequency;
		ss->n22KHz = n22KHz;
		ss->nDiSEqCInput = nDiSEqCInput;
#ifdef DVBS2
		ss->nCodeRate = nCodeRate;
		ss->nADVModulationMode = nADVModulationMode;
#endif DVBS2
#endif DVBS
#ifdef DVBT
		ss->fSpectrumInversion = fSpectrumInversion;
		ss->nBandwidth = nBandwidth;
#endif DVBT
#ifdef DVBC
		ss->nSymbolRate = nSymbolRate;
		ss->nQAM = nQAM;
		ss->fSpectrumInversion = fSpectrumInversion;
		ss->nBandwidth = nBandwidth;
#endif DVBC
		if (ss->fQuietMode == FALSE)
		{
			fNeedTuneDialog = TRUE;
			OutputDebugString("TuneDialog() set fNeedTuneDialog = TRUE\n");
		}
	}

	return TRUE;
}

BOOL TSReader_PIDManagement(BOOL fAdd, int nPID, BOOL fTemporary)
{
	return TRUE;
}

BOOL TSReader_GetDescription(char * szDescription, char * szCommandLineParameters, BOOL * fCanBeStopped, int * nMaxPIDs, DWORD * dwCapabilities)
{
	if (szDescription != NULL)
		lstrcpy(szDescription, gszSourceName);
	if (szCommandLineParameters != NULL)
#ifdef DVBS
#ifndef DVBS2
		lstrcpy(szCommandLineParameters, "freq pol sr lnbf 22khz {input}");
#else DVBS2
		lstrcpy(szCommandLineParameters, "freq pol sr lnbf 22khz mode fec {input}");	
#endif DVBS2
	if (dwCapabilities != NULL)
		*dwCapabilities = CAPABILITIES_DISEQC
		                | CAPABILITIES_TONEBURST
						| CAPABILITIES_POWER
						| CAPABILITIES_DISEQC_POSITIONER
						| CAPABILITIES_MULTICARD
						| CAPABILITIES_DISEQC
#ifdef DVBS2
						| CAPABILITIES_ADV_SATELLITE
#endif DVBS2
						| CAPABILITIES_CI_CAM;
#endif DVBS
#ifdef DVBT
		lstrcpy(szCommandLineParameters, "freq inversion bandwidth");
	if (dwCapabilities != NULL)
		*dwCapabilities = CAPABILITIES_MULTICARD;
	if (fAntennaPower)
		*dwCapabilities |= CAPABILITIES_ACTIVE_ANTENNA;
#endif DVBT
#ifdef DVBC
		lstrcpy(szCommandLineParameters, "freq sr QAM inversion bandwidth");
	if (dwCapabilities != NULL)
		*dwCapabilities = CAPABILITIES_MULTICARD;
#endif DVBC
	if (fCanBeStopped != NULL)
		*fCanBeStopped = FALSE;
	if (nMaxPIDs != NULL)
		*nMaxPIDs = 8192;

	return TRUE;
}

#ifdef DVBC
BOOL TSReader_ParseCommandLine(PSOURCESTRUCT ss, char * szCommandLine, BOOL fQuiet)
{
	fNeedTuneDialog = TRUE;
	OutputDebugString("TSReader_ParseCommandLine() set fNeedTuneDialog = TRUE\n");

	if (lstrlen(szCommandLine))
	{
		int nConversionCount;

		SourceHelper_ConvertPolarity(szCommandLine);
		ss->nDiSEqCInput = 1;
		nConversionCount = sscanf(szCommandLine,
								  "%d %d %d %d %d", 
								  &nFrequency,
								  &nSymbolRate,
								  &nQAM,
								  &fSpectrumInversion,
								  &nBandwidth);
		if (nConversionCount < 5)
		{
			if (!fQuiet)
				MessageBox(NULL,
					   "Usage for this source: freq sr QAM inversion bandwidth\n"
					   "\n"
					   "freq = frequency to tune in KHz\n"
					   "sr = symbol rate\n"
					   "QAM = QAM Mode (0=QAM-16 1=QAM-32 2=QAM-64 3=QAM-128 4=QAM-256)\n"
					   "inversion = inverted spectrum (0 or 1)\n"
					   "bandwidth = bandwidth of signal (0 = 6, 1 = 7, 2 = 8 MHz)",
					   gszSourceName,
					   MB_OK | MB_ICONSTOP);
			return FALSE;
		}
		else
		{
			fNeedTuneDialog = FALSE;
			OutputDebugString("NO TUNE DIALOG\n");
		}
	}
	else
	{
		fNeedTuneDialog = TRUE;
		OutputDebugString("TUNE DIALOG\n");
	}

	return TRUE;
}
#endif DVBC

#ifdef DVBS
BOOL TSReader_ParseCommandLine(PSOURCESTRUCT ss, char * szCommandLine, BOOL fQuiet)
{
	fNeedTuneDialog = TRUE;
	OutputDebugString("TSReader_ParseCommandLine() set fNeedTuneDialog = TRUE\n");

	if (lstrlen(szCommandLine))
	{
		int nConversionCount;

		SourceHelper_ConvertPolarity(szCommandLine);
		ss->nDiSEqCInput = 1;
#ifndef DVBS2
		nConversionCount = sscanf(szCommandLine,
								  "%d %d %d %d %d %d", 
								  &nFrequency,
								  &nPolarity,
								  &nSymbolRate,
								  &nLNBFrequency,
								  &n22KHz,
								  &nDiSEqCInput);
		if (nConversionCount < 5)
		{
			if (!fQuiet)
				MessageBox(NULL,
					   "Usage for this source: freq pol sr lnbf 22khz {input}\n"
					   "\n"
					   "freq = frequency to tune\n"
					   "pol = 0 vertical/RHCP 1 horizontal/LHCP\n"
					   "sr = symbol rate\n"
					   "lnbf = LNB frequency\n"
					   "22k = 22KHz tone enable\n"
					   "input = select DiSEqC input number (1-4) - optional",
					   gszSourceName,
					   MB_OK | MB_ICONSTOP);
			return FALSE;
		}
#else DVBS2
		nConversionCount = sscanf(szCommandLine,
								  "%d %d %d %d %d %d %d %d", 
								  &nFrequency,
								  &nPolarity,
								  &nSymbolRate,
								  &nLNBFrequency,
								  &n22KHz,
								  &nADVModulationMode,
								  &nCodeRate,
								  &nDiSEqCInput);
		if (nConversionCount < 7)
		{
			if (!fQuiet)
				MessageBox(NULL,
					   "Usage for this source: freq pol sr lnbf 22khz mode fec {input}\n"
					   "\n"
					   "freq = frequency to tune\n"
					   "pol = 0 vertical/RHCP 1 horizontal/LHCP\n"
					   "sr = symbol rate\n"
					   "lnbf = LNB frequency\n"
					   "22k = 22KHz tone enable\n"
					   "mode = modulation mode (see readme)\n"
					   "FEC = code rate selection (see readme)\n"
					   "input = select DiSEqC input number (1-4) - optional",
					   gszSourceName,
					   MB_OK | MB_ICONSTOP);
			return FALSE;
		}
#endif DVBS2
		else
		{
			fNeedTuneDialog = FALSE;
			OutputDebugString("NO TUNE DIALOG\n");
		}
	}
	else
	{
		fNeedTuneDialog = TRUE;
		OutputDebugString("TUNE DIALOG\n");
	}

	return TRUE;
}
#endif DVBS
#ifdef DVBT
BOOL TSReader_ParseCommandLine(PSOURCESTRUCT ss, char * szCommandLine, BOOL fQuiet)
{
	fNeedTuneDialog = TRUE;
	if (lstrlen(szCommandLine))
	{
		int nConversionCount;

		nConversionCount = sscanf(szCommandLine,
								  "%d %d %d", 
								  &nFrequency,
								  &fSpectrumInversion,
								  &nBandwidth);
		if (nConversionCount < 3)
		{
			if (!fQuiet)
				MessageBox(NULL,
					   "Usage for this source: freq inversion bandwidth\n"
					   "\n"
					   "freq = frequency to tune in KHz\n"
					   "inversion = inverted spectrum (0 or 1)\n"
					   "bandwidth = bandwidth of signal (0 = 6, 1 = 7, 2 = 8 MHz)",
					   gszSourceName,
					   MB_OK | MB_ICONSTOP);
			return FALSE;
		}
		fNeedTuneDialog = FALSE;
	}
	else
		fNeedTuneDialog = TRUE;

	return TRUE;
}
#endif DVBT

BOOL TSReader_IsPIDActive(int nPID)
{
	return TRUE;
}

BOOL TSReader_SetChannel(int nChannel)
{
	return TRUE;
}

BOOL TSReader_SendDiSEqC(BYTE * bCommand, int nLength)
{
	if (bCommand != NULL && nLength)
	{
		if (fFirstDiSEqC == TRUE)
		{
			fFirstDiSEqC = FALSE;
			m_FE.SendDiSEqCMsg(bCommand, nLength, FALSE);
			Sleep(150 * nLength);
		}
		m_FE.SendDiSEqCMsg(bCommand, nLength, FALSE);
	}
	return TRUE;
}

BOOL TSReader_GetSignalString(char * szString)
{
	EnterCriticalSection(&csSignal);
	lstrcpy(szString, szLastSignalReport);
	LeaveCriticalSection(&csSignal);
	return TRUE;
}

BOOL TSReader_GetTunerString(char * szString)
{
	lstrcpy(szString, szLastTune);
	return TRUE;
}

BOOL TSReader_InitMMI()
{
	return FALSE;
}

BOOL TSReader_CloseMMI()
{
	return FALSE;
}

BOOL TSReader_GetMMI(MMI_Info *pMMIInfo, int *pType)
{
	return FALSE;
}

BOOL TSReader_AnswerMMI(MMI_Info *pMMIInfo, int Type)
{
	return FALSE;
}

BOOL TSReader_GetAppInfo(App_Info *pAppInfo, int * CAM_Type)
{
	return FALSE;
}

BOOL TSReader_SendCAPMT(BYTE *pBuf, int Size)
{
	if (fCIInited)
		nCIProgram = pBuf[1] << 8 | pBuf[2];		// pBuf->CA_PMT
	return TRUE;
}

BOOL APIENTRY DllMain(HANDLE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
    switch( ul_reason_for_call )
	{
    case DLL_PROCESS_ATTACH:
		fCIInited = FALSE;
#ifdef DVBT
		fFirstTime = TRUE;
#endif DVBT
		break;
    case DLL_PROCESS_DETACH:
		if (fCIInited)
		{
			CI.Close();
			fCIInited = FALSE;
		}
#ifdef DVBT
		if (fFirstTime == FALSE)
			CDVBCommon::CloseDevice();
#endif DVBT
		break;
    }
    return TRUE;
}
