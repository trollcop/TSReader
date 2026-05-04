/*
Copyright (c) David R. Cattley (dcattley@msn.com). All rights reserved.

Module Name:

    ATSCSource.h

Abstract:

	ATSC BDA Source for TSReader (www.coolstf.com)

Author:

    David R. Cattley (dcattley@msn.com)

Revision History:

	01-Feb-2005 - Created
*/
#pragma	once
#include "BDAFilterGraph.h"

#define	_TSREADER_SOURCEHELPER_SYNC	1	// Define this if you have it!

extern HINSTANCE g_hInstance;

class CATSCSource
	: public ISampleGrabberCB
{
	// CTOR & DTOR
public:
	CATSCSource()
		: m_pSS(NULL)
		, m_pGraph(NULL)
		, m_nTSBufferIndex(0)
		, m_fNeedTuneDialog(TRUE)
		, m_nFrequency(0)
	{
		ZeroMemory(m_szLastTune, sizeof(m_szLastTune));
	}

	~CATSCSource()
	{
	}

	// TSReader Source Implementation
public:
	//
	// This is where you get a chance to parse the command-line and save the parameters which
	// can be used later in the TuneDialog function rather than presenting a tune dialog.
	// This function is only called with the full-version of TSReader.
	//
	BOOL ParseCommandLine(PSOURCESTRUCT pSS, LPCSTR lpszCommandLine, BOOL fQuiet)
	{
		m_fNeedTuneDialog = TRUE;

		if (lpszCommandLine && lstrlenA(lpszCommandLine))
		{
			int nConversionCount = sscanf(lpszCommandLine, "%d", &m_nFrequency);

			if (nConversionCount < 1)
			{
				if (!fQuiet)
				{
					CHAR szSourceName[256];
					LoadStringA(g_hInstance,  IDS_SOURCE_DESCRIPTION,  szSourceName,  sizeof(szSourceName));
					MessageBoxA(
						NULL,
						"Usage for this source: freq\n"
						"\n"
						"freq = frequency to tune in MHz or prefix with 0 for channel number, e.g. 022 for channel 22",
						szSourceName,
						MB_OK | MB_ICONSTOP
						);
				}

				return FALSE;
			}

			if (*lpszCommandLine == '0')
			{
#ifndef QAM
				m_nFrequency = SourceHelper_GetFrequencyFromATSCChannel(m_nFrequency);
#else QAM
				m_nFrequency = SourceHelper_GetFrequencyFromQAMChannel(m_nFrequency);
#endif QAM
			}

			m_fNeedTuneDialog = FALSE;
		}
		else
		{
			m_fNeedTuneDialog = TRUE;
		}

		return TRUE;
	}

	//
	// This function is called when TSReader first loads up the source. Use it to find and initialize
	// your hardware. Return FALSE if your hardware didn't initialize.
	//
	BOOL Init(PSOURCESTRUCT pSS)
	{
		//
		// Save the SOURCESTRUCT
		//
		m_pSS = pSS;

		//
		// Initialize COM
		//
	    HRESULT hr = CoInitialize(NULL);
		if (SUCCEEDED(hr))
		{
			//
			// Create and initialize the graph.
			//
			m_pGraph = new CBDAFilterGraph();

			if (m_pGraph)
			{
				DWORD dwNetworkType = NT_ATSC;
				hr = m_pGraph->BuildGraph((NETWORK_TYPE)dwNetworkType);
				if (FAILED(hr))
				{
					delete m_pGraph;
					m_pGraph = NULL;
				}
			}

			if (FAILED(hr))
			{
				CoUninitialize();
			}
		}

		return SUCCEEDED(hr) && SourceHelper_StartSyncThread(pSS, FALSE);
	}

	//
	// Opposite of the Init() function
	//
	BOOL DeInit(void)
	{
		if (m_pGraph)
		{
			HRESULT hr = m_pGraph->TearDownGraph();
			delete m_pGraph;
			m_pGraph = NULL;
			CoUninitialize();
		}

		return SourceHelper_StopSyncThread();
	}

	//
	// This is where you setup the tuner to receive the transport stream
	// If you're a source with CAPABILITIES_SERIAL_CONTROL you should
	// call into the SourceHelper_TuneSerialControl() function to have it
	// tune the serially controlled receiver.
	//
	// Return TRUE if you locked, FALSE if no lock and -1 if you're a terrestrial tuner that detects analog signals
	BOOL Tune(void)
	{
		HRESULT hr;

		//
		// Calculate the physial channel.
		//
#ifndef QAM
		LONG physicalChannel = SourceHelper_GetATSCChannelFromFrequency(m_pSS->nFrequency);
#else QAM
		LONG physicalChannel = SourceHelper_GetQAMChannelFromFrequency(m_pSS->nFrequency);
#endif QAM

		//
		// Format and save a tune string.
		//
		wsprintfA(m_szLastTune, "Channel %d (%d Mhz)", physicalChannel, m_pSS->nFrequency);

#ifndef QAM
		//
		// Tell the graph to change channels.
		//
		hr = m_pGraph->ChangeChannel(physicalChannel, -1, -1);
		if (FAILED(hr))
			return FALSE;

 		//
		// Get the IBDA_SignalStatistics interface on the graph control node.
		//
		CComPtr<IBDA_SignalStatistics> pSignalStatistics;
		hr = m_pGraph->GetControlNode(&pSignalStatistics);
		if (FAILED(hr))
			return FALSE;
#endif QAM
		//
		// Start the graph.
		//
		hr = m_pGraph->RunGraph();
		if (FAILED(hr))
			return FALSE;

		//
		// Check if the tuner locked or not.
		//
#ifdef QAM
		hr = m_pGraph->ChangeChannel(physicalChannel, -1, -1);
		if (hr)
		{
			if (m_pSS->fQuietMode == FALSE)
			{
				CHAR szSourceName[256];
				LoadStringA(g_hInstance,  IDS_SOURCE_DESCRIPTION,  szSourceName,  sizeof(szSourceName));
				MessageBoxA(m_pSS->hWndTSReader, "Failed to lock signal", szSourceName, MB_ICONWARNING);
			}
		}
		return hr == 0;
#else QAM
		BOOLEAN signalLocked = FALSE;
		DWORD dwCurrentTime = GetTickCount();
		while ((GetTickCount() < dwCurrentTime + 3000) && SUCCEEDED(hr) && !signalLocked)
		{
			hr = pSignalStatistics->get_SignalLocked(&signalLocked);
			//
			// Give the tuner a chance to actually tune.
			//
			Sleep(100);
		}

		if (!(SUCCEEDED(hr) && signalLocked))
		{
			if (m_pSS->fQuietMode == FALSE)
			{
				CHAR szSourceName[256];
				LoadStringA(
					g_hInstance, 
					IDS_SOURCE_DESCRIPTION, 
					szSourceName, 
					sizeof(szSourceName)
					);

#ifndef NOSTATUS
				MessageBoxA(m_pSS->hWndTSReader, "Failed to lock signal.\n\nIf this happens on all channels and you know there's an 8VSB station that's\nreceiveable your BDA source might not return it's status correctly.\n\nLaunch TSReader with Ctrl pressed down and choose the ATSCBDASourceNS.dll\nsource from the list.", szSourceName, MB_ICONWARNING);
#else NOSTATUS
				MessageBoxA(m_pSS->hWndTSReader, "Failed to lock signal", szSourceName, MB_ICONWARNING);
#endif NOSTATUS
			}
			return FALSE;
		}
#endif QAM
		return TRUE;
	}

	//
	// This function is called by TSReader when it wants to start the source so it can receive
	// data. This gives you a chance to setup a thread to move the data
	BOOL Start(void)
	{
		if (!m_pGraph)
			return FALSE;

		m_pSS->hReadDataThread = NULL; /* don't need this*/
		m_pSS->fTerminateReadThread = FALSE;
		
		//
		// Setup the Sample Grabber callback.
		//
		CComPtr<ISampleGrabber> sampleGrabber;
		HRESULT hr = m_pGraph->GetSampleGrabber(&sampleGrabber);

		AM_MEDIA_TYPE mediaType = {0};
		hr = FAILED(hr) ? hr : sampleGrabber->GetConnectedMediaType(&mediaType);

		hr = FAILED(hr) ? hr : sampleGrabber->SetBufferSamples(false);
		hr = FAILED(hr) ? hr : sampleGrabber->SetCallback(this, 1 /* Call BufferCB */);

		hr = FAILED(hr) ? hr : m_pGraph->RunGraph();

		return SUCCEEDED(hr);
	}

	//
	// Opposite of the start function - close down the thread before returning
	//
	BOOL Stop(void)
	{
		m_pSS->fTerminateReadThread = TRUE;

		if (m_pGraph)
		{
			//
			// Stop the graph.
			//
			HRESULT hr = m_pGraph->StopGraph();

			//
			// Shutdown the Sample Grabber callback.
			//
			CComPtr<ISampleGrabber> sampleGrabber;

			hr = FAILED(hr) ? hr : m_pGraph->GetSampleGrabber(&sampleGrabber);
			hr = FAILED(hr) ? hr : sampleGrabber->SetCallback(NULL, 1 /* Call BufferCB */);
		}

		m_pSS->fReadThreadTerminated = TRUE;
		m_pSS->fTerminateReadThread = FALSE;

		EnterCriticalSection(&m_pSS->csTSBuffersInUse);
		m_pSS->nTSBuffersInUse = -1000;
		LeaveCriticalSection(&m_pSS->csTSBuffersInUse);

		return TRUE;
	}


	//
	// TSReader calls this function to get the tuner parameters. You can build your own dialog
	// or use one of the TSReader standard ones exported in TSReader_SourceHelper.dll
	//
	BOOL TuneDialog(HWND hWndParent)
	{
		if (m_pSS->fDontTune == TRUE)
		{
			m_fNeedTuneDialog = FALSE;
		}

		if (m_fNeedTuneDialog)
		{
			m_pSS->fQuietMode = FALSE;

#ifndef QAM
			return SourceHelper_ATSCTuneDialog(hWndParent);
#else QAM
			return SourceHelper_QAMTuneDialog(hWndParent);
#endif QAM
		}
		else
		{
			m_pSS->nFrequency = m_nFrequency;
			m_fNeedTuneDialog = TRUE;
		}

		return TRUE;
	}

	//
	// This function is used for sources that have demuxes. TSReader will call this function as 
	// it needs access to a PID. For devices that return the entire transport stream, this function
	// can be ignored.
	//
	BOOL PIDManagement(BOOL fAdd, int nPID, BOOL fTemporary)
	{
		return TRUE;
	}

	//
	// If using an interface with a demux, this function tells TSReader if the PID is currently
	// active (turned on in the demux) or not.
	//
	BOOL IsPIDActive(int nPID)
	{
		/* No DeMux */
		return TRUE;
	}

	//
	// This function will be used in the future for CI-CAMs
	//
	BOOL SetChannel(int nChannel)
	{
		/* not supported */
		return TRUE;
	}

	double QAM_SNR(int DATA)
	{
		double snr;

		if((DATA>1021)&&(DATA<=1023))
			snr = 00.0 ;
		else if((DATA>773)&&(DATA<=1021))
			snr = 10.0 ;
		else if((DATA>769)&&(DATA<=773))
			snr = 18.0 ;
		else if((DATA>765)&&(DATA<=769))
			snr =18.5 ;
		else if((DATA>761)&&(DATA<=765))
			snr =19.0 ;
		else if((DATA>758)&&(DATA<=761))
			snr =19.5 ;
		else if((DATA>755)&&(DATA<=758))
			snr =20.0 ;
		else if(DATA==755)
			snr =20.5 ;
		else if(DATA==754)
			snr =21.0 ;
		else if((DATA>750)&&(DATA<=753))
			snr =21.5 ;
		else if((DATA>746)&&(DATA<=750))
			snr =22.0 ;
		else if((DATA>742)&&(DATA<=746))
			snr =22.5 ;
		else if((DATA>738)&&(DATA<=742))
			snr =23.0 ;
		else if((DATA>733)&&(DATA<=738))
			snr =23.5 ;
		else if((DATA>728)&&(DATA<=733))
			snr =24.0 ;
		else if((DATA>723)&&(DATA<=728))
			snr =24.5 ;
		else if((DATA>715)&&(DATA<=723))
			snr =25.0 ;
		else if((DATA>707)&&(DATA<=715))
			snr =25.5 ;
		else if((DATA>700)&&(DATA<=707))
			snr =26.0 ;
		else if((DATA>692)&&(DATA<=700))
			snr =26.5 ;
		else if((DATA>686)&&(DATA<=692))
			snr =27.0 ;
		else if((DATA>684)&&(DATA<=686))
			snr =27.1 ;
		else if((DATA>682)&&(DATA<=684))
			snr =27.2 ;
		else if((DATA>679)&&(DATA<=682))
			snr =27.3 ;
		else if((DATA>676)&&(DATA<=679))
			snr =27.4 ;
		else if((DATA>674)&&(DATA<=676))
			snr =27.5 ;
		else if((DATA>672)&&(DATA<=674))
			snr =27.6 ;
		else if((DATA>670)&&(DATA<=672))
			snr =27.7 ;
		else if((DATA>666)&&(DATA<=670))
			snr =27.8 ;
		else if((DATA>663)&&(DATA<=666))
			snr =27.9 ;
		else if((DATA>660)&&(DATA<=663))
			snr =28.0 ;
		else if((DATA>657)&&(DATA<=660))
			snr =28.1 ;
		else if((DATA>654)&&(DATA<=657))
			snr =28.2 ;
		else if((DATA>650)&&(DATA<=654))
			snr =28.3 ;
		else if((DATA>647)&&(DATA<=650))
			snr =28.4 ;
		else if((DATA>645)&&(DATA<=647))
			snr =28.5 ;
		else if((DATA>643)&&(DATA<=645))
			snr =28.6 ;
		else if((DATA>640)&&(DATA<=643))
			snr =28.7 ;
		else if((DATA>637)&&(DATA<=640))
			snr =28.8 ;
		else if((DATA>634)&&(DATA<=637))
			snr =28.9 ;
		else if((DATA>631)&&(DATA<=634))
			snr =29.0 ;
		else if((DATA>628)&&(DATA<=631))
			snr =29.1 ;
		else if((DATA>625)&&(DATA<=628))
			snr =29.2 ;
		else if((DATA>621)&&(DATA<=625))
			snr =29.3 ;
		else if((DATA>617)&&(DATA<=621))
			snr =29.4 ;
		else if((DATA>613)&&(DATA<=617))
			snr =29.5 ;
		else if((DATA>609)&&(DATA<=613))
			snr =29.6 ;
		else if((DATA>605)&&(DATA<=609))
			snr =29.7 ;
		else if((DATA>602)&&(DATA<=605))
			snr =29.8 ;
		else if((DATA>598)&&(DATA<=602))
			snr =29.9 ;
		else if((DATA>594)&&(DATA<=598))
			snr =30.0 ;
		else if((DATA>589)&&(DATA<=594))
			snr =30.1 ;
		else if((DATA>585)&&(DATA<=589))
			snr =30.2 ;
		else if((DATA>581)&&(DATA<=585))
			snr =30.3 ;
		else if((DATA>577)&&(DATA<=581))
			snr =30.4 ;
		else if((DATA>572)&&(DATA<=577))
			snr =30.5 ;
		else if((DATA>569)&&(DATA<=572))
			snr =30.6 ;
		else if((DATA>564)&&(DATA<=569))
			snr =30.7 ;
		else if((DATA>560)&&(DATA<=564))
			snr =30.8 ;
		else if((DATA>556)&&(DATA<=560))
			snr =30.9 ;
		else if((DATA>551)&&(DATA<=556))
			snr =31.0 ;
		else if((DATA>547)&&(DATA<=551))
			snr =31.1 ;
		else if((DATA>542)&&(DATA<=547))
			snr =31.2 ;
		else if((DATA>537)&&(DATA<=542))
			snr =31.3 ;
		else if((DATA>532)&&(DATA<=537))
			snr =31.4 ;
		else if((DATA>527)&&(DATA<=532))
			snr =31.5 ;
		else if((DATA>522)&&(DATA<=527))
			snr =31.6 ;
		else if((DATA>517)&&(DATA<=522))
			snr =31.7 ;
		else if((DATA>512)&&(DATA<=517))
			snr =31.8 ;
		else if((DATA>507)&&(DATA<=512))
			snr =31.9 ;
		else if((DATA>501)&&(DATA<=507))
			snr =32.0 ;
		else if((DATA>495)&&(DATA<=501))
			snr =32.1 ;
		else if((DATA>490)&&(DATA<=495))
			snr =32.2 ;
		else if((DATA>484)&&(DATA<=490))
			snr =32.3 ;
		else if((DATA>478)&&(DATA<=484))
			snr =32.4 ;
		else if((DATA>472)&&(DATA<=478))
			snr =32.5 ;
		else if((DATA>467)&&(DATA<=472))
			snr =32.6 ;
		else if((DATA>461)&&(DATA<=467))
			snr =32.7 ;
		else if((DATA>455)&&(DATA<=461))
			snr =32.8 ;
		else if((DATA>448)&&(DATA<=455))
			snr =32.9 ;
		else if((DATA>442)&&(DATA<=448))
			snr =33.0 ;
		else if((DATA>437)&&(DATA<=442))
			snr =33.1 ;
		else if((DATA>431)&&(DATA<=437))
			snr =33.2 ;
		else if((DATA>425)&&(DATA<=431))
			snr =33.3 ;
		else if((DATA>419)&&(DATA<=425))
			snr =33.4 ;
		else if((DATA>412)&&(DATA<=419))
			snr =33.5 ;
		else if((DATA>407)&&(DATA<=412))
			snr =33.6 ;
		else if((DATA>400)&&(DATA<=407))
			snr =33.7 ;
		else if((DATA>393)&&(DATA<=400))
			snr =33.8 ;
		else if((DATA>388)&&(DATA<=393))
			snr =33.9 ;
		else if((DATA>381)&&(DATA<=388))
			snr =34.0 ;
		else if((DATA>375)&&(DATA<=381))
			snr =34.1 ;
		else if((DATA>369)&&(DATA<=375))
			snr =34.2 ;
		else if((DATA>362)&&(DATA<=369))
			snr =34.3 ;
		else if((DATA>356)&&(DATA<=362))
			snr =34.4 ;
		else if((DATA>350)&&(DATA<=356))
			snr =34.5 ;
		else if((DATA>343)&&(DATA<=350))
			snr =34.6 ;
		else if((DATA>337)&&(DATA<=343))
			snr =34.7 ;
		else if((DATA>331)&&(DATA<=337))
			snr =34.8 ;
		else if((DATA>324)&&(DATA<=331))
			snr =34.9 ;
		else if(DATA<=324)
			snr =35.0 ;
		else 
			snr =0.0 ;

		return snr;
	}

	//
	// This function is called at a 1Hz rate and returns a signal report string which
	// TSReader then displays in it's main window
	//
	BOOL GetSignalString(LPSTR lpszSignalString)
	{
#ifdef QAM
		DWORD dwRawSNR = m_pGraph->GetQAMSNR();
		if (dwRawSNR != -1)
			sprintf(lpszSignalString, "SNR: %.1f dB (%d)", QAM_SNR(dwRawSNR), dwRawSNR);
		else
			lstrcpy(lpszSignalString, "n/a");
		return TRUE;
#endif QAM
#ifndef NOSTATUS
		if (!m_pGraph)
			return FALSE;

		lstrcpyA(lpszSignalString, "n/a");

		CComPtr<IBDA_SignalStatistics> signalStatistics;
		HRESULT hr = m_pGraph->GetControlNode(&signalStatistics);

		BOOLEAN present = FALSE;
		BOOLEAN locked = FALSE;
		LONG quality = 0;
		LONG strength = 0;

		hr = FAILED(hr) ? hr : signalStatistics->get_SignalPresent(&present);
		hr = FAILED(hr) ? hr : signalStatistics->get_SignalLocked(&locked);
		hr = FAILED(hr) ? hr : signalStatistics->get_SignalQuality(&quality);
		hr = FAILED(hr) ? hr : signalStatistics->get_SignalStrength(&strength);

		if (FAILED(hr))
			return FALSE;

		char szTemp[64];
		if (present && locked)
		{
			lstrcpyA(szTemp, "Locked");
		}
		else if (present)
		{
			lstrcpyA(szTemp, "Present");
		}
		else
		{
			lstrcpyA(szTemp, "Absent");
		}
		if (quality > 100)
			quality = 0;
		wsprintfA(lpszSignalString,  "%s: %3d%% (%2d.%0.3d dBm)", szTemp, quality, (strength / 1000), (strength % 1000));

#else NOSTATUS
		lstrcpyA(lpszSignalString, "n/a");
#endif NOSTATUS

		return TRUE;
	}

	//
	// This function is called at a 1Hz rate and returns a tune report string which
	// TSReader then displays in it's main window
	//
	BOOL GetTunerString(LPSTR lpszTunerString)
	{
		lstrcpyA(lpszTunerString, m_szLastTune);
		return TRUE;
	}

	//
	// This function is called by TSReader when it needs to send a DiSEqC command.
	// Only sources that have CAPABILITIES_DISEQC_POSITIONER will get called.
	//
	BOOL SendDiSEqC(BYTE * bCommand, int nLength)
	{
		/* not supported */
		return TRUE;
	}

	//
	// ISampleGrabberCB implementation.
	//
public:
	STDMETHODIMP_(ULONG) AddRef() { return 1; }
    STDMETHODIMP_(ULONG) Release() { return 2; }

    STDMETHODIMP QueryInterface(REFIID riid, void **ppvObject)
    {
        if (NULL == ppvObject) return E_POINTER;
        if (riid == __uuidof(IUnknown))
        {
            *ppvObject = static_cast<IUnknown*>(this);
             return S_OK;
        }
        if (riid == __uuidof(ISampleGrabberCB))
        {
            *ppvObject = static_cast<ISampleGrabberCB*>(this);
             return S_OK;
        }
        return E_NOTIMPL;
    }

    STDMETHODIMP SampleCB(double Time, IMediaSample *pSample)
    {
        return E_NOTIMPL;
    }

    STDMETHODIMP BufferCB(double Time, LPBYTE lpBuffer, long BufferLen)
    {
#ifdef	_TSREADER_SOURCEHELPER_SYNC
		BOOL fSuccess = SourceHelper_SyncData(lpBuffer, BufferLen);
#else
		long remaining = BufferLen;
		LPBYTE buffer = lpBuffer;

		while(remaining && !m_pSS->fTerminateReadThread)
		{
			long chunk = min(remaining, TS_BUFFER_SIZE);

			EnterCriticalSection(&m_pSS->csPIDCounter);
			m_pSS->nLastSecondByteCounter += chunk;
			LeaveCriticalSection(&m_pSS->csPIDCounter);

			CopyMemory(m_pSS->tsb[m_nTSBufferIndex].pData, buffer, chunk);
			m_pSS->tsb[m_nTSBufferIndex++].nSize = chunk;
			m_nTSBufferIndex %= MAX_TS_BUFFERS;

			EnterCriticalSection(&m_pSS->csTSBuffersInUse);
			m_pSS->nTSBuffersInUse++;
			LeaveCriticalSection(&m_pSS->csTSBuffersInUse);
			
			remaining -= chunk;
		}
#endif

        return S_OK;
    }


protected:
	PSOURCESTRUCT m_pSS;
	CBDAFilterGraph* m_pGraph;
	int m_nTSBufferIndex;
    
	bool m_fNeedTuneDialog;
	int m_nFrequency;

	CHAR m_szLastTune[128];
};
