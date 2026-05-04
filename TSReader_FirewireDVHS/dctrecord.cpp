#include <windows.h>
#include <streams.h>
#include <initguid.h>

#include <atlbase.h>

#include <xprtdefs.h>
#include "dumpuids.h"
#include "dump.h"

#include "sources.h"

extern PSOURCESTRUCT ss;
extern char gszSourceName[];

HRESULT FindFilterInterface(
    IGraphBuilder *pGraph, // Pointer to the Filter Graph Manager.
    REFGUID iid,           // IID of the interface to retrieve.
    void **ppUnk)          // Receives the interface pointer.
{
    if (!pGraph || !ppUnk) return E_POINTER;

    HRESULT hr = E_FAIL;
    IEnumFilters *pEnum = NULL;
    IBaseFilter *pF = NULL;
    if (FAILED(pGraph->EnumFilters(&pEnum)))
    {
        return E_FAIL;
    }
    // Query every filter for the interface.
    while (S_OK == pEnum->Next(1, &pF, 0))
    {
        hr = pF->QueryInterface(iid, ppUnk);
        pF->Release();
        if (SUCCEEDED(hr))
        {
            break;
        }
    }
    pEnum->Release();
    return hr;
}



HRESULT findTuner(IBaseFilter	**tunerFilter, char *filtername, REFCLSID inCLSID)
{
    IBaseFilter	*		m_pDeviceFilter = NULL;
    ICreateDevEnum*     pCreateDevEnum = NULL;
    IEnumMoniker *      pEnumMoniker = NULL;
    IMoniker *          pMoniker = NULL;
    ULONG               nFetched = 0;
    TCHAR               m_DeviceName[260];
    CHAR				FoundDVHS = FALSE;
    HRESULT hr;
    
    // Create Device Enumerator
    hr = CoCreateInstance(CLSID_SystemDeviceEnum, NULL, CLSCTX_INPROC_SERVER, IID_ICreateDevEnum, reinterpret_cast<PVOID *>(&pCreateDevEnum));
    if (FAILED(hr)) {return hr;}
    
    // Create the enumerator of the monikers for the specified Device Class & reset them 
    hr = pCreateDevEnum->CreateClassEnumerator(inCLSID, &pEnumMoniker, 0);
    if(SUCCEEDED(hr) && pEnumMoniker)
        pEnumMoniker->Reset();
    else   {
        //printf(" Failed to CreateClassEnumerator.") ;
        return hr;
    }

    // Loop through "CLSID_VideoInputDeviceCategory" looking for "Microsoft AV/C Tape Subunit Device"
    while(SUCCEEDED(pEnumMoniker->Next( 1, &pMoniker, &nFetched )) && pMoniker && nFetched)    
    {
        IPropertyBag *pPropBag;
        hr = pMoniker->BindToStorage( 0, 0, IID_IPropertyBag, (void **)&pPropBag );


        //Friendly name
        VARIANT varFriendlyName;
        varFriendlyName.vt = VT_BSTR;
        hr = pPropBag->Read( L"FriendlyName", &varFriendlyName, 0 );
        WideCharToMultiByte( CP_ACP, 0, varFriendlyName.bstrVal, -1, m_DeviceName, sizeof(m_DeviceName), 0, 0 );    
        VariantClear( &varFriendlyName );

//        if(!lstrcmp(TEXT("Microsoft AV/C Tape Subunit Device"), m_DeviceName))
        if(!lstrcmp(TEXT(filtername), m_DeviceName))
        {
            //printf("Found: %s\n", m_DeviceName) ;        
            hr = pMoniker->BindToObject(0, 0, IID_IBaseFilter, (void**)&m_pDeviceFilter );
            FoundDVHS = TRUE;
            break;
        }
        //        else
            
        pPropBag->Release();
        pMoniker->Release();

    }//end of while

    pEnumMoniker->Release();
    pCreateDevEnum->Release();
    if (FoundDVHS == TRUE)
    {   
        *tunerFilter = m_pDeviceFilter;
        return S_OK;
    }
	else
        return E_FAIL;

}

/*BOOL LocateTapeDevice()
{
	HRESULT hr;

	//hr = findTuner(&pSource, "Panasonic MPEG2TS Tape Subunit Device",  // avx-1 uses this
    //               CLSID_VideoInputDeviceCategory);	
	hr = findTuner(&pSource, "Microsoft AV/C Tape Subunit Device",  // avx-1 uses this
                   CLSID_VideoInputDeviceCategory);

    if (hr != S_OK)
    {
        OutputDebugString("firedvhs: Could not find firewire Capture Device\n");
        return FALSE;
    }

	return TRUE;
}*/

HRESULT SendCmdToDevice(IAMExtTransport *pTransport, int channel)
{
    HRESULT hr;
    // Set up the ATN search command.
    BYTE AvcCmd[512] = 
    { 
        0x00,   // ctype = "control"
        0x48,   // subunit_type, subunit_id
        0x7C,   // opcode (ATN)
        0x67,   // operand 0 = "search"
        0x04,   // operand 1 = ATN
        0x00,   // operand 2 = ATN
        0xAC,   // operand 3 = ATN
        0xFF,   //  operand 4 = D-VCR medium type.
    };
    
    long cbCmd = 8;
    hr = pTransport->GetTransportBasicParameters(ED_RAW_EXT_DEV_CMD, 
                                                 &cbCmd,
                                                 (LPOLESTR*) &AvcCmd);
    return S_OK;
}

HRESULT GetUnconnectedPin(
    IBaseFilter *pFilter,   // Pointer to the filter.
    PIN_DIRECTION PinDir,   // Direction of the pin to find.
    IPin **ppPin)           // Receives a pointer to the pin.
{
    *ppPin = 0;
    IEnumPins *pEnum = 0;
    IPin *pPin = 0;
    HRESULT hr = pFilter->EnumPins(&pEnum);
    if (FAILED(hr))
    {
        return hr;
    }
    while (pEnum->Next(1, &pPin, NULL) == S_OK)
    {
        PIN_DIRECTION ThisPinDir;
        pPin->QueryDirection(&ThisPinDir);
        if (ThisPinDir == PinDir)
        {
            IPin *pTmp = 0;
            hr = pPin->ConnectedTo(&pTmp);
            if (SUCCEEDED(hr))  // Already connected, not the pin we want.
            {
                pTmp->Release();
            }
            else  // Unconnected, this is the pin we want.
            {
                pEnum->Release();
                *ppPin = pPin;
                return S_OK;
            }
        }
        pPin->Release();
    }
    pEnum->Release();
    // Did not find a matching pin.
    return E_FAIL;
}

int dctrecord()
{
    HRESULT hr = 0;
    
    // create a filter graph
    CComPtr< IFilterGraph > pGraph;
    hr = pGraph.CoCreateInstance( CLSID_FilterGraph );

    // QI the filter graph for the other useful interfaces
    CComQIPtr< IGraphBuilder, &IID_IGraphBuilder > pBuilder( pGraph );
    CComQIPtr< IMediaSeeking, &IID_IMediaSeeking > pSeeking( pGraph );
    CComQIPtr< IMediaControl, &IID_IMediaControl > pControl( pGraph );
    CComQIPtr< IMediaFilter, &IID_IMediaFilter > pMediaFilter( pGraph );
    CComQIPtr< IMediaEvent, &IID_IMediaEvent > pEvent( pGraph );

    // add a the dump filter
    IBaseFilter *pDump;
    CDump *cDump = new CDump(NULL, &hr);
    pDump = cDump->m_pFilter;
    
    IFileSinkFilter *pSink= NULL;
    pDump->QueryInterface(IID_IFileSinkFilter, (void**)&pSink);
    hr = pSink->SetFileName(L"test", NULL);
//    hr = pSink->SetFileName((LPCOLESTR)filename, NULL);
    if (hr != S_OK)
    {
        cDump->Release();
		OutputDebugString("firedvhs: pSink->SetFileName() failed\n");
        return -1;
    }    
    
    // add a source filter for it
	CComPtr< IBaseFilter > pSource;
	hr = findTuner(&pSource, "Microsoft AV/C Tape Subunit Device",  // avx-1 uses this
                   CLSID_VideoInputDeviceCategory);

    if (hr != S_OK)
    {
        MessageBox(ss->hWndTSReader, "Unable to locate Firewire Capture Device", gszSourceName, MB_ICONSTOP);
        return 0;
    }

    CComPtr<IBaseFilter>		pTuner;
    CComPtr<IAMExtDevice>       m_pDevice;
    CComPtr<IAMExtTransport>    m_pTransport;
    CComPtr<IAMTimecodeReader>  m_pTimecode;   
    if (0 && pTuner)
    {        
        pTuner->QueryInterface(&m_pDevice);        
        if (m_pDevice)
        {
            // Don't query for these unless there is an external device.
            pTuner->QueryInterface(&m_pTransport);
            pTuner->QueryInterface(&m_pTimecode);
            
            LONG lDeviceType = 0;
            hr = m_pDevice->GetCapability(ED_DEVCAP_DEVICE_TYPE, &lDeviceType, 0);
            if (SUCCEEDED(hr) && (lDeviceType == ED_DEVTYPE_TUNER) && (m_pTransport != 0))
            {
                SendCmdToDevice(m_pTransport, 123);
                //printf("Found a device\r\n");
                //            cDump->Release();
                //            return 0;
            }
        }
    }
    
    IPin * pSourceOut = NULL;
    hr =  GetUnconnectedPin(pSource, PINDIR_OUTPUT, &pSourceOut);    
    hr = pBuilder->AddFilter( pSource, L"TunerFilter" );
    hr = pBuilder->AddFilter( pDump, L"Dump" );

    // get the other input and output pins
    IPin * pDumpIn = NULL;
    hr =  GetUnconnectedPin(pDump, PINDIR_INPUT, &pDumpIn);

    // connect
    hr = pBuilder->Connect( pSourceOut, pDumpIn );

    // make sure it looks right in the debug output
    //DumpGraph( pGraph, 0 );
    
//    REFERENCE_TIME Duration = 0;
  //  hr = pSeeking->GetDuration( &Duration );

    pSource->SetSyncSource(NULL);
    pDump->SetSyncSource(NULL);
	
    hr = pControl->Pause(); // pause stream
    hr = pControl->Run();  // start stream
    ASSERT( !FAILED( hr ) );

	while (!ss->fTerminateReadThread)
		Sleep(10);

    hr = pBuilder->Abort();
    hr = pControl->StopWhenReady(); //
    
    cDump->Release();
    //delete cDump;

    return 0;
}
