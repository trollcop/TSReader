/*
Copyright (c) David R. Cattley (dcattley@msn.com). All rights reserved.

Module Name:

    DVBSSource.h

Abstract:

	DVBS BDA Source for TSReader (www.coolstf.com)

Author:

    David R. Cattley (dcattley@msn.com)

Revision History:

	01-Feb-2005 - Created
*/
#pragma	once
#include "BDAFilterGraph.h"

#define	_TSREADER_SOURCEHELPER_SYNC	1	// Define this if you have it!

extern HINSTANCE g_hInstance;

typedef struct {
	unsigned long ChannelFrequency;//
		unsigned long ulLNBLOFLowBand;
	unsigned long ulLNBLOFHighBand;
	unsigned long SymbolRate;
	//unsigned char QamMode;
	unsigned char Polarity;
	UCHAR           LNB_POWER;              // LNB_POWER_ON | LNB_POWER_OFF
	UCHAR           HZ_22K;                 // HZ_22K_OFF | HZ_22K_ON
	UCHAR           Tone_Data_Burst;        // Data_Burst_ON | Tone_Burst_ON |Tone_Data_Disable
	UCHAR           DiSEqC_Port;            // DiSEqC_NULL | DiSEqC_A | DiSEqC_B | DiSEqC_C | DiSEqC_D

	unsigned char motor[5];

	unsigned char ir_code;
	unsigned char lock;
	unsigned char strength;
	unsigned char quality;

	unsigned char reserved[256];
} QBOXDVBSCMD, *PQBOXDVBSCMD;
typedef enum
{
    KSPROPERTY_CTRL_TUNER,
} KSPROPERTY_UTICA;
// {BDB891DA-1E7A-46c4-8246-7ABDE36DDB57}
//DEFINE_GUID(<<name>>, 
//0xbdb891da, 0x1e7a, 0x46c4, 0x82, 0x46, 0x7a, 0xbd, 0xe3, 0x6d, 0xdb, 0x57);
//#define STATIC_KSPROPERTYSET_QBOXControl \
//    4ab8455a, 0xd56b, 0x4c6c, 0x83, 0x74, 0x62, 0xc3, 0x7c, 0xf1, 0xdd, 0x9b
//DEFINE_GUIDSTRUCT( "C6EFE5EB-855A-4f1b-B7AA-87B5E1DC4113", KSPROPERTYSET_QBOXControl );
//#define KSPROPERTYSET_QBOXControl DEFINE_GUIDNAMED( KSPROPERTYSET_QBOXControl )

#define STATIC_KSPROPERTYSET_QBOXControl \
    0xbdb891da, 0x1e7a, 0x46c4, 0x82, 0x46, 0x7a, 0xbd, 0xe3, 0x6d, 0xdb, 0x57
DEFINE_GUIDSTRUCT( "C6EFE5EB-855A-4f1b-B7AA-87B5E1DC4113", KSPROPERTYSET_QBOXControl );
#define KSPROPERTYSET_QBOXControl DEFINE_GUIDNAMED( KSPROPERTYSET_QBOXControl )

class CDVBSSource
	: public ISampleGrabberCB
{
	// CTOR & DTOR
public:
	CDVBSSource()
		: m_pSS(NULL)
		, m_pGraph(NULL)
		, m_nTSBufferIndex(0)
		, m_fNeedTuneDialog(TRUE)
		, m_nFrequency(0)
	{
		ZeroMemory(m_szLastTune, sizeof(m_szLastTune));
		hIOMutex= CreateMutex (NULL, FALSE, NULL);
	}

	~CDVBSSource()
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
			int nConversionCount;

			SourceHelper_ConvertPolarity((char *)lpszCommandLine);
			pSS->nDiSEqCInput = 0;
			nConversionCount = sscanf(lpszCommandLine,
									  "%d %d %d %d %d %d", 
									  &m_nFrequency,
									  &m_nPolarity,
									  &m_nSymbolRate,
									  &m_nLNBFrequency,
									  &m_n22KHz,
									  &m_nDiSEqCInput);
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
						   "DVB-S BDA Source",
						   MB_OK | MB_ICONSTOP);
				return FALSE;
			}

			m_fNeedTuneDialog = FALSE;
		}

		return TRUE;
	}
void WINAPI DebugString(LPCSTR format, ...)
{
va_list arglist;
char buffer[1024];

va_start (arglist,format);
vsprintf(buffer, format, arglist);
va_end (arglist);

strcat(buffer, "\n");

OutputDebugString (buffer);
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
				DWORD dwNetworkType = NT_DVBC;

				LONG lKey;
				HKEY hkMainReg;
				static char szKeyName[] = "Software\\COOL.STF\\TSReader\\DVBSSource";
				static char szSourceName[] = {"DVBSSource"};

				lKey = RegOpenKeyEx(HKEY_CURRENT_USER, 
								 szKeyName,
								 0,
								 KEY_QUERY_VALUE,
								 &hkMainReg);
				if (lKey == ERROR_SUCCESS)
				{
					DWORD dwDataSize = sizeof(DWORD);
					DWORD dwType;

					RegQueryValueEx(hkMainReg, "NetworkType", NULL, &dwType, (BYTE *)&dwNetworkType, &dwDataSize);
					RegCloseKey(hkMainReg);
				}

				hr = m_pGraph->BuildGraph((NETWORK_TYPE)dwNetworkType);

				if (FAILED(hr))
				{
					delete m_pGraph;
					m_pGraph = NULL;
				}
				else{
					//OutputDebugString(TEXT(" Init(PSOURCESTRUCT pSS)\n"));
					m_pKsCtrl = m_pGraph->ReturnFilter();
					if(m_pKsCtrl==NULL)
						OutputDebugString(TEXT(" Init(PSOURCESTRUCT pSS)m_pKsCtrl==NULL\n"));
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
		if(m_pKsCtrl)
			{
			delete m_pKsCtrl;
			}
		return SourceHelper_StopSyncThread();
	}

void LockDVBSChannel()
{
	HRESULT hr;
	QBOXDVBSCMD temp_cmd;
       if(m_pKsCtrl==NULL)
       	{
	   	OutputDebugString(TEXT("m_pKsCtrl==NULL\n"));
		return;
       	}
	memcpy(&temp_cmd,&m_QBoxCmd,sizeof(QBOXDVBSCMD));
	OutputDebugString(TEXT("LockDVBSChannel1\n"));
	WaitForSingleObject( hIOMutex, INFINITE );
	OutputDebugString(TEXT("LockDVBSChannel2\n"));
	hr = m_pKsCtrl->Set(KSPROPERTYSET_QBOXControl,
                        KSPROPERTY_CTRL_TUNER,
                        NULL,
                        0,
                        &temp_cmd,
                        sizeof( QBOXDVBSCMD ));

	if(FAILED(hr))
	{
		//ErrorMessageBox("Can't Submit tuner request!");
		OutputDebugString(TEXT("Can't Submit tuner request!\n"));
	}
	OutputDebugString(TEXT("LockDVBSChannel3\n"));
	ReleaseMutex( hIOMutex);
	OutputDebugString(TEXT("LockDVBSChannel4\n"));
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
	QBOXDVBSCMD temp_cmd;
	ULONG BytesRead;
	HRESULT hr ;
		//
		// Calculate the physial channel.
		//
              // HRESULT hr =   m_pGraph->StopGraph();
		//if (FAILED(hr))
		//	return FALSE;

	char szPolarity[4] = {"H/L"};

	if (m_pSS->nPolarity == 0)
		lstrcpy(szPolarity, "V/R");
	wsprintfA(m_szLastTune, "%d MHz %s %d DVB QPSK", m_pSS->nFrequency, szPolarity, m_pSS->nSymbolRate);
	
	OutputDebugString(TEXT("Tune1\n"));
	m_QBoxCmd.ChannelFrequency=m_pSS->nFrequency;
	m_QBoxCmd.SymbolRate=m_pSS->nSymbolRate;
	m_QBoxCmd.HZ_22K = m_pSS->n22KHz;
	if(m_pSS->nPolarity == -1)
	{
		m_QBoxCmd.LNB_POWER = 0;
	}
	else
	{
		m_QBoxCmd.LNB_POWER = 1;
		m_QBoxCmd.Polarity = m_pSS->nPolarity;
	}
	m_QBoxCmd.ulLNBLOFHighBand = m_pSS->nLNBFrequency;
	m_QBoxCmd.ulLNBLOFLowBand = m_pSS->nLNBFrequency;
	
	if(m_pSS->nDiSEqCInput < 5)
		m_QBoxCmd.DiSEqC_Port = m_pSS->nDiSEqCInput;
	else if(m_pSS->nDiSEqCInput == 5)
		m_QBoxCmd.Tone_Data_Burst = 1;
	else if(m_pSS->nDiSEqCInput == 6)
		m_QBoxCmd.Tone_Data_Burst = 2;
	
		OutputDebugString(TEXT("Tune2\n"));
		LockDVBSChannel();
		OutputDebugString(TEXT("Tune3\n"));
		
		OutputDebugString(TEXT("Tune4\n"));
	
		OutputDebugString(TEXT("Tune5\n"));
			Sleep(100);
		memcpy(&temp_cmd,&m_QBoxCmd,sizeof(QBOXDVBSCMD));
		OutputDebugString(TEXT("Tune6\n"));
		WaitForSingleObject( hIOMutex, INFINITE );
              temp_cmd.lock = 0;
		hr = m_pKsCtrl->Get(KSPROPERTYSET_QBOXControl,
	                        KSPROPERTY_CTRL_TUNER,
	                        &temp_cmd,
	                        sizeof( QBOXDVBSCMD ),
	                        &temp_cmd,
	                        sizeof( QBOXDVBSCMD ),
	                        &BytesRead );
		if(FAILED(hr))
		{
			//ErrorMessageBox("Can't Get tuner Status!");
			OutputDebugString(TEXT("Can't Get tuner Status!\n"));
			return FALSE;
		}
		OutputDebugString(TEXT("Tune7\n"));
		ReleaseMutex( hIOMutex);


		//
		// Check if the tuner locked or not.
		//
	
             DebugString("lock %d  stenth  %d  qulity  %d",temp_cmd.lock,temp_cmd.strength,temp_cmd.quality);
		if(temp_cmd.lock!=1)
			temp_cmd.lock = 0;
		if (!(SUCCEEDED(hr) && temp_cmd.lock))
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

                            OutputDebugString(TEXT("Failed to lock signal\n"));
				MessageBoxA(m_pSS->hWndTSReader, "Failed to lock signal", szSourceName, MB_ICONWARNING);

			}
			OutputDebugString(TEXT("Failed to lock signal111111\n"));
			return FALSE;
		}
		// Start the graph.
		//
		
		hr = m_pGraph->RunGraph();
			if (FAILED(hr))
			{
			OutputDebugString(TEXT("hr = m_pGraph->RunGraph();\n"));
			return FALSE;
			}
			
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
		OutputDebugString(TEXT("@@@@Start\n"));
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

			return SourceHelper_DVBSTuneDialog(hWndParent);
		}
		else
		{
			m_pSS->nFrequency = m_nFrequency;
			m_pSS->nPolarity = m_nPolarity;
			m_pSS->nSymbolRate = m_nSymbolRate;
			m_pSS->nLNBFrequency = m_nLNBFrequency;
			m_pSS->n22KHz = m_n22KHz;
			m_pSS->nDiSEqCInput = m_nDiSEqCInput;
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

	//
	// This function is called at a 1Hz rate and returns a signal report string which
	// TSReader then displays in it's main window
	//
	BOOL GetSignalString(LPSTR lpszSignalString)
	{
		char szLockStatus[128];
		QBOXDVBSCMD temp_cmd;
		ULONG BytesRead;

		if (!m_pGraph)
			return FALSE;

		lstrcpyA(lpszSignalString, "n/a");

		CComPtr<IBDA_SignalStatistics> signalStatistics;
		HRESULT hr = m_pGraph->GetControlNode(&signalStatistics);

		memcpy(&temp_cmd, &m_QBoxCmd, sizeof(QBOXDVBSCMD));

		WaitForSingleObject( hIOMutex, INFINITE );

		hr = m_pKsCtrl->Get(KSPROPERTYSET_QBOXControl,
	                        KSPROPERTY_CTRL_TUNER,
	                        &temp_cmd,
	                        sizeof( QBOXDVBSCMD ),
	                        &temp_cmd,
	                        sizeof( QBOXDVBSCMD ),
	                        &BytesRead );
		if(FAILED(hr))
		{
			//ErrorMessageBox("Can't Get tuner Status!");
			return FALSE;
		}

		ReleaseMutex( hIOMutex);


		if (FAILED(hr))
			return FALSE;

		if (temp_cmd.strength > 10 && temp_cmd.lock)
			lstrcpy(szLockStatus, "Locked");
		else
			lstrcpy(szLockStatus, "Unlocked");
				
		wsprintfA(lpszSignalString, "%s Quality %d%% Signal %d%%", szLockStatus, temp_cmd.quality, temp_cmd.strength);

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
	int m_nPolarity;
	int m_nSymbolRate;
	int m_nLNBFrequency;
	int m_n22KHz;
	int m_nDiSEqCInput;

	CHAR m_szLastTune[128];

	QBOXDVBSCMD	m_QBoxCmd;
	HANDLE    hIOMutex;
	IKsPropertySet* m_pKsCtrl;
};
