#include <windows.h>
#include <commctrl.h>
#include <dshow.h>
#include <uuids.h>
#include <streams.h>
#include <asyncio.h>
#include <string.h>
#include <asyncrdr.h>
#include "TSReader_DVHS.h"

#include "../TSReader.h"

PVARIABLES v;

#define STRADIS_READ_SIZE 188 * 256
CMemReader *rdr;
IGraphBuilder *pFG = NULL;
IBaseFilter *pTSP2SP = NULL;
BYTE * pbMem;
BOOL fDVHSThreadComplete;
IPin *m_sourcePin;
IPin *m_TSPInPin;
IAMExtDevice            *m_pIAMExtDevice;
IAMExtTransport         *m_pIAMExtTransport;
IAMTimecodeReader       *m_pIAMTCReader;

HRESULT PlayFileWait(IGraphBuilder *pFG, BOOL fControlDeck, BOOL fPowerCycleDeck)  // Start the filtergraph and wait till done...
{
    IMediaControl *pMC;
    IMediaEvent *pME;
    HRESULT hr = pFG->QueryInterface(IID_IMediaControl, (void **)&pMC);
    if (FAILED(hr))
		return hr;

    hr = pFG->QueryInterface(IID_IMediaEvent, (void **)&pME);
    if (FAILED(hr))
	{
        pMC->Release();
        return hr;
    }

    OAEVENT oEvent;
    hr = pME->GetEventHandle(&oEvent);
    if (SUCCEEDED(hr))
	{
        hr = pMC->Run();
    }
    if (SUCCEEDED(hr))
	{
        LONG levCode;
		BOOL fPowerOn = FALSE;

		if (fPowerCycleDeck && m_pIAMExtDevice != NULL)
		{
			int nCount = 0;

			fPowerOn = TRUE;
			do
			{
				long lPowerMode;

				m_pIAMExtDevice->put_DevicePower(ED_POWER_ON);
				m_pIAMExtDevice->get_DevicePower(&lPowerMode);
#ifdef DEBUG_MESSAGES
				{
					char szTemp[100];
					wsprintf(szTemp, "ED_POWER_ON: lPowerMode = %d nCount = %d\n", lPowerMode, nCount);
					OutputDebugString(szTemp);
				}
#endif DEBUG_MESSAGES
				if (lPowerMode == ED_POWER_ON)
					break;
				Sleep(100);
			} while (nCount++ < 20);
		}

		if (fControlDeck && m_pIAMExtTransport != NULL)
		{
			int nCount = 0;

			if (fPowerOn)
				Sleep(250);
			do
			{
				long lRecordMode;

				m_pIAMExtTransport->put_Mode(ED_MODE_RECORD);
				m_pIAMExtTransport->get_Mode(&lRecordMode);
#ifdef DEBUG_MESSAGES
				{
					char szTemp[100];
					wsprintf(szTemp, "ED_MODE_RECORD: lRecordMode = %d nCount = %d\n", lRecordMode, nCount);
					OutputDebugString(szTemp);
				}
#endif DEBUG_MESSAGES
				if (lRecordMode == ED_MODE_RECORD)
					break;
				Sleep(100);
			} while (nCount++ < 20);
		}

        hr = pME->WaitForCompletion(INFINITE, &levCode);

		if (fControlDeck && m_pIAMExtTransport != NULL)
		{
			int nCount = 0;
			do
			{
				long lStopMode;

				m_pIAMExtTransport->put_Mode(ED_MODE_STOP);
				m_pIAMExtTransport->get_Mode(&lStopMode);
#ifdef DEBUG_MESSAGES
				{
					char szTemp[100];
					wsprintf(szTemp, "ED_MODE_STOP: lStopMode = %d nCount = %d\n", lStopMode, nCount);
					OutputDebugString(szTemp);
				}
#endif DEBUG_MESSAGES
				if (lStopMode == ED_MODE_STOP && nCount >= 5)
					break;
				Sleep(100);
			} while (nCount++ < 20);
		}

		if (fPowerCycleDeck && m_pIAMExtDevice != NULL)
		{
			int nCount = 0;

			Sleep(500);

			do
			{
				long lPowerMode;

				m_pIAMExtDevice->put_DevicePower(ED_POWER_OFF);
				m_pIAMExtDevice->get_DevicePower(&lPowerMode);
#ifdef DEBUG_MESSAGES
				{
					char szTemp[100];
					wsprintf(szTemp, "ED_POWER_OFF: lPowerMode = %d nCount = %d\n", lPowerMode, nCount);
					OutputDebugString(szTemp);
				}
#endif DEBUG_MESSAGES
				if (lPowerMode == ED_POWER_OFF && nCount >= 5)
					break;
				Sleep(100);
			} while (nCount++ < 20);
		}
    }

	hr = pMC->Stop();
    pMC->Release();
    pME->Release();
    
	return hr;
}

DEFINE_GUID(CLSID_TSP2SP, 
0x9d509fe5, 0x1cbd, 0x44d4, 0x93, 0x35, 0x8c, 0x87, 0xe8, 0x1f, 0xda, 0xb5);

BOOL GetTapeControl(IBaseFilter * m_pDeviceFilter)
{
	HRESULT hr = S_OK;

    hr = m_pDeviceFilter->QueryInterface(IID_IAMExtTransport, (void **) &m_pIAMExtTransport);
	if (FAILED(hr))
	{
		OutputDebugString(" Failed to  QI IAMExtTransport.\n");
		return FALSE;
	}
  
    hr = m_pDeviceFilter->QueryInterface(IID_IAMExtDevice, (void **) &m_pIAMExtDevice);
	if (FAILED(hr))
	{
		OutputDebugString(" Failed to  QI IAMExtDevice.\n");
		return FALSE;
	}

    hr = m_pDeviceFilter->QueryInterface(IID_IAMTimecodeReader, (void **) &m_pIAMTCReader);
	if (FAILED(hr))
	{
		OutputDebugString(" Failed to  QI IAMTimecodeReader.\n");
		return FALSE;
	}

	if (m_pIAMExtTransport == NULL)
		return FALSE;

	return TRUE;
}

HRESULT SelectAndRender(CMemReader *pReader, IGraphBuilder **ppFG)
{
	HRESULT hr = S_OK;
	ICaptureGraphBuilder2 *pBuilder = NULL;

	// Create the Capture Graph Builder.
	hr = CoCreateInstance(CLSID_CaptureGraphBuilder2, NULL, CLSCTX_INPROC_SERVER, IID_ICaptureGraphBuilder2, (void **)&pBuilder);
    if (FAILED(hr))
		return hr;

	pBuilder->SetFiltergraph(*ppFG);
    if (FAILED(hr))
		return hr;

    /*  Get the various interfaces we need */

	// Create TSP2SP Filter...
	hr = CoCreateInstance(CLSID_TSP2SP, NULL, CLSCTX_INPROC_SERVER, IID_IBaseFilter, reinterpret_cast<PVOID *>(&pTSP2SP));
    if (FAILED(hr))
	{
		lstrcpy(v->szDVHSRecordFailureReason, "Couldn't load TSP2SP.AX filter - is it installed and registered?");
		return hr;
	}
	
    /*  Add reader filter */
    hr = (*ppFG)->AddFilter(pReader, NULL);
    if (FAILED(hr))
		return hr;


    /*  Add TSP2SP filter */
    hr = (*ppFG)->AddFilter(pTSP2SP, NULL);
    if (FAILED(hr))
		return hr;


	IBaseFilter	*		m_pDeviceFilter = NULL;
    ICreateDevEnum*     pCreateDevEnum = NULL;
    IEnumMoniker *      pEnumMoniker = NULL;
    IMoniker *          pMoniker = NULL;
    ULONG               nFetched = 0;
	TCHAR               m_DeviceName[260];
	CHAR				FoundDVHS = FALSE;
    
	// Create Device Enumerator
    hr = CoCreateInstance(CLSID_SystemDeviceEnum, NULL, CLSCTX_INPROC_SERVER, IID_ICreateDevEnum, reinterpret_cast<PVOID *>(&pCreateDevEnum));
    if (FAILED(hr)) {return hr;}
    
    // Create the enumerator of the monikers for the specified Device Class & reset them 
    hr = pCreateDevEnum->CreateClassEnumerator(CLSID_VideoInputDeviceCategory, &pEnumMoniker, 0);
    if(SUCCEEDED(hr) && pEnumMoniker)
        pEnumMoniker->Reset();
    else 
	{
        lstrcpy(v->szDVHSRecordFailureReason, "D-VHS deck enumeration interface failed - device connected?");
        return E_FAIL;
    }

    // Loop through "CLSID_VideoInputDeviceCategory" looking for "Microsoft AV/C Tape Subunit Device"
    while(SUCCEEDED(pEnumMoniker->Next( 1, &pMoniker, &nFetched )) && pMoniker)    
    {
        IPropertyBag *pPropBag;
        hr = pMoniker->BindToStorage( 0, 0, IID_IPropertyBag, (void **)&pPropBag );


        //Friendly name
        VARIANT varFriendlyName;
        varFriendlyName.vt = VT_BSTR;
        hr = pPropBag->Read( L"FriendlyName", &varFriendlyName, 0 );
        WideCharToMultiByte( CP_ACP, 0, varFriendlyName.bstrVal, -1, m_DeviceName, sizeof(m_DeviceName), 0, 0 );    
        VariantClear( &varFriendlyName );

        //if(!lstrcmp(TEXT("Panasonic MPEG2TS Tape Subunit Device"), m_DeviceName))
        if(!lstrcmp(TEXT("Microsoft AV/C Tape Subunit Device"), m_DeviceName))
        {
            hr = pMoniker->BindToObject(0, 0, IID_IBaseFilter, (void**)&m_pDeviceFilter );
            hr = (*ppFG)->AddFilter(m_pDeviceFilter, L"Filter");
			FoundDVHS = TRUE;
            //break;
        }
        pPropBag->Release();
        pMoniker->Release();

		if (FoundDVHS)
			break;

    }//end of while

    pEnumMoniker->Release();
    pCreateDevEnum->Release();

	if (!FoundDVHS)
	{
		lstrcpy(v->szDVHSRecordFailureReason, "D-VHS deck not found - driver installed?");
		return E_FAIL;
	}
    
	/*  Connect our filters */
	if (GetTapeControl(m_pDeviceFilter) == FALSE)
	{
		lstrcpy(v->szDVHSRecordFailureReason, "D-VHS deck control interface not found - device connected?");
		return E_FAIL;
	}

	m_sourcePin = pReader->GetPin(0);
    hr = pBuilder->FindPin(pTSP2SP, PINDIR_INPUT, NULL, NULL, TRUE, 0, &m_TSPInPin);
	if (FAILED(hr))
		return hr;
	hr = (*ppFG)->ConnectDirect( m_sourcePin, m_TSPInPin, NULL);
	if (FAILED(hr))
		return hr;
	hr = pBuilder->RenderStream( NULL, NULL, pTSP2SP, NULL, m_pDeviceFilter);
	if (FAILED(hr))
		return hr;

    pBuilder->Release();
	return hr; //Done
}

HRESULT MakeGraph(IGraphBuilder **ppFG)
{
	HRESULT hr = CoCreateInstance(CLSID_FilterGraph, NULL, CLSCTX_INPROC_SERVER, IID_IGraphBuilder, (void **)ppFG);
	if (FAILED(hr))
		return hr;

	return 1;
}

DWORD WINAPI StradisDataThread(LPVOID lpv)
{
	PVARIABLES v = (PVARIABLES)lpv;

    CMediaType mt;
    mt.majortype = MEDIATYPE_Stream;
	mt.subtype = MEDIASUBTYPE_None;

	CoInitialize(NULL);

	HRESULT hr = S_OK;
    pbMem = new BYTE[STRADIS_READ_SIZE];
	LONGLONG m_Length = ((LONGLONG)144095232 * 480);  // Length = 8 Hours HiPix Files...
	
	hr = MakeGraph(&pFG);
    if (FAILED(hr))
	{
        lstrcpy(v->szDVHSRecordFailureReason, "Failed to make filter graph");
		PostMessage(v->hDlgSIParser, WM_USER + 4, 0, 0);
	}
	else
	{
		CMemStream Stream(&v->hStradisReadPipe, pbMem, m_Length, pFG, &v->dTotalRecorded, &v->nPipeBytes, v->csPipeBytes); 
		rdr = new CMemReader(&Stream, &mt, &hr);
		if (FAILED(hr) || rdr == NULL)
		{
			delete rdr;
			lstrcpy(v->szDVHSRecordFailureReason, "Could not create filter");
			PostMessage(v->hDlgSIParser, WM_USER + 4, 0, 0);
			goto Windup;
		}

		//  Make sure we don't accidentally go away!
		rdr->AddRef();
		hr = SelectAndRender(rdr, &pFG);
		if (FAILED(hr))
		{
			delete rdr;
			if (lstrlen(v->szDVHSRecordFailureReason) == 0)
				lstrcpy(v->szDVHSRecordFailureReason, "Failed to Render Graph");
			PostMessage(v->hDlgSIParser, WM_USER + 4, 0, 0);
			goto Windup;
		}

		//  Play the graph
		hr = PlayFileWait(pFG, v->fControlDVHSDeck, v->fPowerCycleDVHSDeck);
		if (FAILED(hr))
		{
			delete rdr;
			lstrcpy(v->szDVHSRecordFailureReason, "Failed to play graph");
			PostMessage(v->hDlgSIParser, WM_USER + 4, 0, 0);
			goto Windup;
		}
			
		pTSP2SP->Release();
		rdr->Release();
		if (pFG)
		{
			ULONG ulRelease = pFG->Release();
			if (ulRelease != 0)
				OutputDebugString("Filter graph count not 0!\n");
		}

		//delete rdr;
	}

Windup:
	delete pbMem;
    CoUninitialize();
	fDVHSThreadComplete = TRUE;
	return 0;
}

BOOL SetupStradis(PVARIABLES pv)
{
	HANDLE hThread;
	DWORD dwThreadID;

	v = pv;		// save for us

	// Get a thread going to read the data written by the main
	// parser thread
	fDVHSThreadComplete = FALSE;
	hThread = CreateThread(NULL, 0, StradisDataThread, (LPVOID)v, 0, &dwThreadID);
	CloseHandle(hThread);
				
	return TRUE;
}

void ShutdownStradis()
{
	while (!fDVHSThreadComplete)
		Sleep(10);
}

