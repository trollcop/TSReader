//#define DEBUG_ROT

#include <windows.h>
#ifdef DEBUG_ROT
# include <atlstr.h>
#endif

#include "DVBCBDASource.h"
#include "../sources.h"

// {FC772AB0-0C7F-11D3-8FF2-00A0C9224CF5}
static const GUID BDASRC_CLSID_TIF =
{ 0xfc772ab0, 0x0c7f, 0x11d3,{ 0x8f, 0xf2, 0x00, 0xa0, 0xc9, 0x22, 0x4c, 0xf5 } };

void ErrorBox(LPCTSTR);

#ifdef DEBUG_ROT

HRESULT AddGraphToRot(
        IUnknown *pUnkGraph, 
        DWORD *pdwRegister
        ) ;

#endif

BDAGraph::BDAGraph():
	fGraphCreated(FALSE),
	fGraphRunning(FALSE),
	initial_carrier(226500),
	initial_symbolrate(5900),
	initial_modulation(BDA_MOD_256QAM),
	initial_ONID(-1),
	initial_SID(-1),
	initial_TSID(-1)
{
}

BDAGraph::~BDAGraph()
{
	if (fGraphCreated)
	{
		Destroy();
	}
}

void BDAGraph::SetInitialTune(LONG carrier, LONG symbolrate=5900, LONG modulation=BDA_MOD_256QAM,
							LONG ONID=-1, LONG SID=-1, LONG TSID=-1)
{
	initial_carrier = carrier;
	initial_symbolrate = symbolrate;
	initial_modulation = modulation;
	initial_ONID = ONID;
	initial_SID = SID;
	initial_TSID = TSID;
}

int BDAGraph::GetTuneStatus(LONG *TuneStatus, BOOLEAN *extra, BOOLEAN *pLocked, LONG *pQuality,
							LONG *pStrength)
{
	HRESULT hr;
	CComQIPtr <ITuner> lpTuner (pNetworkProvider);
	
	long lTuneStatus;

	if (!fGraphCreated)
		return -1;

	if (!lpTuner)
		return -1;

	if (FAILED((hr=lpTuner->get_SignalStrength(&lTuneStatus))))
	{
		return -1;
	}

	*TuneStatus=lTuneStatus;
	
	// extra information, not necessarily available

	BOOLEAN locked=false;
	LONG quality=-1;
	LONG strength=0;
	*extra=false;

	CComQIPtr <IBDA_Topology> pBDATopology (pTuneDev);

	if (pBDATopology)
	{
		CComPtr <IUnknown> pControlNode;

		if (SUCCEEDED(hr=pBDATopology->GetControlNode(0,1,1,&pControlNode)))
		{
			CComQIPtr <IBDA_SignalStatistics> pSignalStats (pControlNode);

			if (pSignalStats)
			{
				if (SUCCEEDED(hr=pSignalStats->get_SignalLocked(&locked)) &&
					SUCCEEDED(hr=pSignalStats->get_SignalQuality(&quality)) &&
					SUCCEEDED(hr=pSignalStats->get_SignalStrength(&strength)))
					*extra=true;
			}
		}
	}

	*pLocked = locked;
	*pQuality = quality;
	*pStrength = strength;

	return 0;
}

int BDAGraph::GetDeviceName(LPWSTR *pdevname)
{
	HRESULT hr;

	if (SUCCEEDED(hr=pTuneDev->QueryVendorInfo(pdevname)))
		return 0;		

	return 1;
}

int BDAGraph::Play()
{
	HRESULT hr;
	CComQIPtr <IMediaControl> pMediaControl (pGraphBuilder);

	if (!fGraphCreated)
		return 1;

	if (fGraphRunning)
		return 0;

	if (!pMediaControl)
		return 1;

	if (SUCCEEDED((hr=pMediaControl->Run())))
	{
		fGraphRunning = 1;		
	}
	else
	{
		pMediaControl->Stop();
		ErrorBox("Unable to run graph");
		return 1;
	}

	pTIF->Stop();

	return 0;
}

int BDAGraph::Stop()
{
	HRESULT hr;
	CComQIPtr <IMediaControl> pMediaControl (pGraphBuilder);

	if (!fGraphCreated)
		return 1;
	if (!fGraphRunning)
		return 0;

	if (pMediaControl)
	{
		if (SUCCEEDED((hr=pMediaControl->Pause())))
		{
			if (SUCCEEDED((hr=(pMediaControl->StopWhenReady()))))
			{
				fGraphRunning = 0;
				return 0;
			}
			ErrorBox("Unable to stop graph");
			return 1;
		}
		ErrorBox("Unable to pause graph");
		return 1;
	}

	return 1;
}

int BDAGraph::Create(NETWORK_TYPE netType, int (*callback_func)(double, BYTE*, long, LPVOID), 
					 LPVOID callback_data)
{
	HRESULT hr;

	if (fGraphCreated)
		return 1;

	// Create the graph builder object
	if (FAILED((hr=pGraphBuilder.CoCreateInstance(CLSID_FilterGraph))))
	{
		Destroy();
		return 1;
	}

	networkType = netType;

	// Tuning space, network provider

	if (LoadTuningSpace())
	{
		Destroy();
		return 1;
	}

	if (LoadNetworkProvider())
	{
		Destroy();
		return 1;
	}

	if (CreateDefaultTuneRequest())
	{
		Destroy();
		return 1;
	}

#ifdef DEBUG_ROT
	// debugging
	DWORD rotreg;
	AddGraphToRot(pGraphBuilder, &rotreg);
#endif

	// Create Tuner filter

	if (FAILED((hr=LoadFilter(KSCATEGORY_BDA_NETWORK_TUNER,
							&pTuneDev,
							pNetworkProvider,
							TRUE))))
	{
		ErrorBox("Error loading and connecting tuner device");
		Destroy();
		return 1;
	}

	// Create demod filter

	if (FAILED((hr=LoadFilter(KSCATEGORY_BDA_RECEIVER_COMPONENT,
							&pReceiverDev,
							pTuneDev,
							TRUE))))
	{
		ErrorBox("Error loading and connecting receiver device");
		Destroy();
		return 1;
	}

	// create infinite tee filter

	if (LoadInfiniteTee())
	{
		Destroy();
		return 1;
	}

	if (FAILED((hr=ConnectFilters(pReceiverDev, pInfiniteTee))))
	{
		ErrorBox("Error connecting infinite tee to receiver");
		Destroy();
		return 1;
	}
	// create demux

	if (LoadDemux())
	{
		Destroy();
		return 1;
	}

	if (FAILED((hr=ConnectFilters(pInfiniteTee, pDemux))))
	{
		// modified by Christian Zietz
		// BDA sources with no receiver component fail here, so
		// try loading them by connecting the tuner directly to
		// the infinite tee

		// first unload all filters ...
		pGraphBuilder->RemoveFilter(pDemux);
		pDemux = NULL;
		pGraphBuilder->RemoveFilter(pInfiniteTee);
		pInfiniteTee = NULL;
		pGraphBuilder->RemoveFilter(pReceiverDev);
		pReceiverDev = NULL;
		pGraphBuilder->RemoveFilter(pTuneDev);
		pTuneDev = NULL;
		// ... then start from scratch
		if (FAILED((hr=LoadFilter(KSCATEGORY_BDA_NETWORK_TUNER,
								&pTuneDev,
								pNetworkProvider,
								TRUE))))
		{	
			ErrorBox("Error loading and connecting tuner device");
			Destroy();
			return 1;
		}
		if (LoadInfiniteTee())
		{
			Destroy();
			return 1;
		}
		// this time connect tuner directly to tee
		if (FAILED((hr=ConnectFilters(pTuneDev, pInfiniteTee))))
		{
			ErrorBox("Error connecting infinite tee to tuner");
			Destroy();
			return 1;
		}		
		// now retry connecting tee to demux
		if (LoadDemux())
		{
			Destroy();
			return 1;
		}
		if (FAILED((hr=ConnectFilters(pInfiniteTee, pDemux))))
		{
			ErrorBox("Error connecting demux to infinite tee");
			Destroy();
			return 1;
		}
	}

	// create TIF

	if (LoadTIF())
	{
		Destroy();
		return 1;
	}

	if (FAILED((hr=ConnectFilters(pDemux, pTIF))))
	{
		ErrorBox("Error connecting TIF to demux");
		Destroy();	
		return 1;
	}

	// Create sample grabber filter

	if (LoadSampleGrabber())
	{
		Destroy();
		return 1;
	}

	// connect sample grabber

	if (FAILED((hr=ConnectFilters(pInfiniteTee, pSampleGrabber))))
	{
		ErrorBox("Error connecting sample grabber to infinite tee");
		Destroy();	
		return 1;
	}

	// set the callback

	grabbercallback.SetCallback(callback_func, callback_data);

	fGraphCreated = 1;

	return 0;
}

int BDAGraph::Destroy()
{
	if (!fGraphCreated)
		return 1;

	if (fGraphRunning)
			Stop();

	if (pSampleGrabber)
	{
		pGraphBuilder->RemoveFilter(pSampleGrabber);
		pSampleGrabber = NULL;
	}

	if (pTIF)
	{
		pGraphBuilder->RemoveFilter(pTIF);
		pTIF = NULL;
	}

	if (pDemux)
	{
		pGraphBuilder->RemoveFilter(pDemux);
		pDemux = NULL;
	}

	if (pInfiniteTee)
	{
		pGraphBuilder->RemoveFilter(pInfiniteTee);
		pInfiniteTee = NULL;
	}

	if (pReceiverDev)
	{
		pGraphBuilder->RemoveFilter(pReceiverDev);
		pReceiverDev = NULL;
	}

	if (pTuneDev)
	{
		pGraphBuilder->RemoveFilter(pTuneDev);
		pTuneDev = NULL;
	}

	fGraphCreated = 0;

	return 0;
}

int BDAGraph::TuneDVB(LONG carrier, LONG symbolrate, LONG modulation, LONG ONID, LONG SID, LONG TSID)
{
	if (!fGraphCreated)
		return 1;

	if (networkType != NT_DVBC)
		return 1;

	return CreateDVBCTuneRequest(carrier, symbolrate, modulation, ONID, SID, TSID);
}

int BDAGraph::LoadTuningSpace()
{
	HRESULT hr;
	CComPtr <IDVBTuningSpace> pDVBTuningSpace;

	if (FAILED((hr=pDVBTuningSpace.CoCreateInstance(CLSID_DVBTuningSpace))))
	{
		ErrorBox("Could not create a DVBTuningSpace object");
		return 1;
	}
	if (FAILED(hr=pDVBTuningSpace->put_FriendlyName(CComBSTR("TSReader BDASource Tuning Space"))) ||
		(FAILED(hr=pDVBTuningSpace->put_UniqueName(CComBSTR("TSReader BDASource Tuning Space"))) ||
		(FAILED(hr=pDVBTuningSpace->put__NetworkType(CLSID_DVBCNetworkProvider))) ||
		(FAILED(hr=pDVBTuningSpace->put_SystemType(DVB_Cable)))))
	{
		ErrorBox("Error setting DVB-C Tuning space attributes");
		return 1;
	}

	// generic ITuningSpace point to DVBTuningSpace

	if (FAILED((hr=pDVBTuningSpace.QueryInterface(&pTuningSpace))))
	{
		ErrorBox("Could not retrieve ITuningSpace interface from DVBTuningSpace");
		return 1;
	}
	return 0;
}

int BDAGraph::LoadNetworkProvider()
{
	HRESULT hr;

	// load the network provider based upon our tuning space

	CLSID providerCLSID;
	
	// fetch its CLSID
	if (FAILED((hr=pTuningSpace->get__NetworkType(&providerCLSID))))
	{
		ErrorBox("Error retrieving tuning space CLSID");
		return 1;
	}

	if (FAILED((hr=CoCreateInstance(providerCLSID, NULL, CLSCTX_INPROC_SERVER, 
								IID_IBaseFilter, reinterpret_cast<void**> (&pNetworkProvider)))))
	{
		ErrorBox("Error creating network provider");
		return 1;
	}

	if (FAILED((hr=pGraphBuilder->AddFilter(pNetworkProvider, L"BDA Network Provider"))))
	{
		ErrorBox("Error adding network provider to graph");
		return 1;
	}

	return 0;
}

int BDAGraph::LoadSampleGrabber()
{
	HRESULT hr;
	pSampleGrabber.CoCreateInstance(CLSID_SampleGrabber);

	if (!pSampleGrabber)
	{
		ErrorBox("Cannot load sample grabber");
		return 1;
	}

	if (FAILED((hr=pGraphBuilder->AddFilter(pSampleGrabber, L"Sample Grabber"))))
	{
		ErrorBox("Cannot add sample grabber to graph");
		return 1;
	}

	CComQIPtr <ISampleGrabber> pSampleGrabberI (pSampleGrabber);

	AM_MEDIA_TYPE sgType;
	ZeroMemory(&sgType, sizeof(sgType));

	sgType.majortype=GUID_NULL;
	sgType.subtype=GUID_NULL;
	sgType.formattype=GUID_NULL;

	if (FAILED((hr=pSampleGrabberI->SetMediaType(&sgType))))
	{
		ErrorBox("Cannot set sample grabber media type");
		return 1;
	}

	//set callback

	if (FAILED((hr=pSampleGrabberI->SetCallback(&grabbercallback, 1))))
	{
		ErrorBox("Cannot set sample grabber callback");
		return 1;
	}

	return 0;
}

int BDAGraph::LoadInfiniteTee()
{
	HRESULT hr;
	pInfiniteTee.CoCreateInstance(CLSID_InfTee);

	if (!pInfiniteTee)
	{
		ErrorBox("Cannot load infinite tee");
		return 1;
	}

	if (FAILED((hr=pGraphBuilder->AddFilter(pInfiniteTee, L"Infinite Tee"))))
	{
		ErrorBox("Cannot add infinite tee to graph");
		return 1;
	}

	return 0;
}

int BDAGraph::LoadDemux()
{
	HRESULT hr;
	pDemux.CoCreateInstance(CLSID_MPEG2Demultiplexer);

	if (!pDemux)
	{
		ErrorBox("Cannot load demux");
		return 1;
	}

	if (FAILED((hr=pGraphBuilder->AddFilter(pDemux, L"MPEG-2 Demux"))))
	{
		ErrorBox("Cannot add MPEG-2 demultiplexer to graph");
		return 1;
	}

	return 0;
}

int BDAGraph::LoadTIF()
{
	HRESULT hr;
	pTIF.CoCreateInstance(BDASRC_CLSID_TIF);

	if (!pTIF)
	{
		ErrorBox("Cannot load TIF");
		return 1;
	}

	if (FAILED((hr=pGraphBuilder->AddFilter(pTIF, L"TIF"))))
	{
		ErrorBox("Cannot add TIF to graph");
		return 1;
	}

	return 0;
}

int BDAGraph::CreateDefaultTuneRequest()
{
	switch (networkType)
	{
	case NT_DVBC:
		return CreateDVBCTuneRequest(initial_carrier, initial_symbolrate, initial_modulation,
			initial_ONID, initial_SID, initial_TSID);
	default:
		return 1;
	}
	return 0;
}

int BDAGraph::CreateDVBCTuneRequest(LONG carrier, LONG symbolrate, LONG modulation,
									LONG ONID, LONG SID, LONG TSID)
{
	HRESULT hr;

	CComQIPtr <IDVBTuningSpace> pDVBTuningSpace (pTuningSpace);
	if (!pDVBTuningSpace)
	{
		ErrorBox("Cannot retrieve DVB-C Tuning Space");
		return 1;
	}

	CComPtr <ITuneRequest> lpTuneRequest;
	if (FAILED((hr=pDVBTuningSpace->CreateTuneRequest(&lpTuneRequest))))
	{
		ErrorBox("Cannot create Tune Request");
		return 1;
	}

	CComQIPtr <IDVBTuneRequest> pDVBTuneRequest (lpTuneRequest);
	if (!pDVBTuneRequest)
	{
		ErrorBox("Cannot retrieve DVB-C Tune Request");
		return 1;
	}

	// create the locator
	CComPtr <IDVBCLocator> pDVBCLocator;
	if (FAILED((hr=pDVBCLocator.CoCreateInstance(CLSID_DVBCLocator))))
	{
		ErrorBox("Cannot create DVB-C locator");
		return 1;
	}

	if (FAILED((hr=pDVBCLocator->put_SymbolRate(symbolrate))) ||
		FAILED((hr=pDVBCLocator->put_Modulation((ModulationType)modulation))) ||
		FAILED((hr=pDVBCLocator->put_CarrierFrequency(carrier))))
	{
		ErrorBox("Error setting bandwidth/frequency/modulation");
		return 1;
	}

	if (FAILED((hr=pDVBTuneRequest->put_Locator(pDVBCLocator))))
	{
		ErrorBox("Error putting locator to tune request");
		return 1;
	}

	if (FAILED((hr=pDVBTuneRequest->put_ONID(ONID))) ||
		FAILED((hr=pDVBTuneRequest->put_SID(SID))) ||
		FAILED((hr=pDVBTuneRequest->put_TSID(TSID))))
	{
		ErrorBox("Error setting ONID/SID/TSID");
		return 1;
	}

	// finally submit it to the tuner
	CComQIPtr <ITuner> pTuner (pNetworkProvider);
	if (!pTuner)
	{
		ErrorBox("Error retrieving tuner interface");
		return 1;
	}

	if (FAILED((hr=pTuner->put_TuneRequest(pDVBTuneRequest))))
	{
		ErrorBox("Error submitting tune request");
		return 1;
	}

	return 0;
}

void ErrorBox(LPCTSTR errorText)
{
	MessageBox(NULL, errorText, "Error", MB_ICONERROR|MB_OK);
}

// LoadFilter, borrowed from the DX SDK BDASample


HRESULT BDAGraph::LoadFilter(
    REFCLSID clsid, 
    IBaseFilter** ppFilter,
    IBaseFilter* pConnectFilter, 
    BOOL fIsUpstream
    )
{
    HRESULT                 hr = S_OK;
    BOOL                    fFoundFilter = FALSE;
    CComPtr <IMoniker>      pIMoniker;
    CComPtr <IEnumMoniker>  pIEnumMoniker;
	CComPtr <ICreateDevEnum>pICreateDevEnum;

    hr = pICreateDevEnum.CoCreateInstance(CLSID_SystemDeviceEnum);
    if (FAILED (hr))
    {
        ErrorBox("LoadFilter(): Cannot CoCreate ICreateDevEnum");
        return hr;
    }

    // obtain the enumerator
    hr = pICreateDevEnum->CreateClassEnumerator(clsid, &pIEnumMoniker, 0);
    // the call can return S_FALSE if no moniker exists, so explicitly check S_OK
    if (FAILED (hr))
    {
        ErrorBox("LoadFilter(): Cannot CreateClassEnumerator");
        return hr;
    }
    if (S_OK != hr)  // Class not found
    {
        ErrorBox("LoadFilter(): Class not found, CreateClassEnumerator returned S_FALSE");
        return E_UNEXPECTED;
    }

    // next filter
    while(pIEnumMoniker->Next(1, &pIMoniker, 0) == S_OK)
    {
        // obtain filter's friendly name
        CComPtr <IPropertyBag>  pBag;
        hr = pIMoniker->BindToStorage(
                                    NULL, 
                                    NULL, 
                                    IID_IPropertyBag,
                                    reinterpret_cast<void**>(&pBag)
                                    );

        if(FAILED(hr))
        {
            OutputDebugString (TEXT("LoadFilter(): Cannot BindToStorage"));
            return hr;
        }

        CComVariant varBSTR;
        hr = pBag->Read(L"FriendlyName", &varBSTR, NULL);
        if(FAILED(hr))
        {
            OutputDebugString (TEXT("LoadFilter(): IPropertyBag->Read method failed"));
            pIMoniker = NULL;
            continue;
        }

        // bind the filter
        CComPtr <IBaseFilter>   pFilter;
        hr = pIMoniker->BindToObject(
                                    NULL, 
                                    NULL, 
                                    IID_IBaseFilter,
                                    reinterpret_cast<void**>(&pFilter)
                                    );

        if (FAILED(hr))
        {
            pIMoniker = NULL;
            pFilter = NULL;
            continue;
        }


        hr = pGraphBuilder->AddFilter (pFilter, varBSTR.bstrVal);

        if (FAILED(hr))
        {
            OutputDebugString (TEXT("Cannot add filter\n"));
            return hr;
        }

        //MessageBox (NULL, _T(""), _T(""), MB_OK);
        // test connections
        // to upstream filter
        if (pConnectFilter)
        {
            if(fIsUpstream)
            {
                hr = ConnectFilters (pConnectFilter, pFilter);
            }
            else
            {
                hr = ConnectFilters (pFilter, pConnectFilter);
            }

            if(SUCCEEDED(hr))
            {
                //that's the filter we want
                fFoundFilter = TRUE;
                pFilter.QueryInterface (ppFilter);
                break;
            }
            else
            {
                fFoundFilter = FALSE;
                // that wasn't the the filter we wanted
                // so unload and try the next one
                hr = pGraphBuilder->RemoveFilter(pFilter);

                if(FAILED(hr))
                {
                    OutputDebugString(TEXT("Failed unloading Filter\n"));
                    return hr;
                }
            }
        }
        else
        {
            fFoundFilter = TRUE;
            pFilter.QueryInterface (ppFilter);
            break;
        }

        pIMoniker = NULL;
        pFilter = NULL;
    } // while

	return fFoundFilter ? S_OK : E_FAIL;
}

// ConnectFilters, borrowed from the DX SDK BDASample

HRESULT
BDAGraph::ConnectFilters(
    IBaseFilter* pFilterUpstream, 
    IBaseFilter* pFilterDownstream
    )
{
    HRESULT         hr = E_FAIL;

    CComPtr <IPin>  pIPinUpstream;

    PIN_INFO        PinInfoUpstream;
    PIN_INFO        PinInfoDownstream;

    // grab upstream filter's enumerator
    CComPtr <IEnumPins> pIEnumPinsUpstream;
    hr = pFilterUpstream->EnumPins(&pIEnumPinsUpstream);

    if(FAILED(hr))
    {
        ErrorBox("Cannot Enumerate Upstream Filter's Pins\n");
        return hr;
    }

    // iterate through upstream filter's pins
    while (pIEnumPinsUpstream->Next (1, &pIPinUpstream, 0) == S_OK)
    {
        hr = pIPinUpstream->QueryPinInfo (&PinInfoUpstream);
        if(FAILED(hr))
        {
            ErrorBox("Cannot Obtain Upstream Filter's PIN_INFO\n");
            return hr;
        }

        CComPtr <IPin>  pPinDown;
        pIPinUpstream->ConnectedTo (&pPinDown);

        // bail if pins are connected
        // otherwise check direction and connect
        if ((PINDIR_OUTPUT == PinInfoUpstream.dir) && (pPinDown == NULL))
        {
            // grab downstream filter's enumerator
            CComPtr <IEnumPins> pIEnumPinsDownstream;
            hr = pFilterDownstream->EnumPins (&pIEnumPinsDownstream);
            if(FAILED(hr))
            {
                ErrorBox("Cannot enumerate pins on downstream filter!\n");
                return hr;
            }

            // iterate through downstream filter's pins
            CComPtr <IPin>  pIPinDownstream;
            while (pIEnumPinsDownstream->Next (1, &pIPinDownstream, 0) == S_OK)
            {
                // make sure it is an input pin
                hr = pIPinDownstream->QueryPinInfo(&PinInfoDownstream);
                if(SUCCEEDED(hr))
                {
                    CComPtr <IPin>  pPinUp;

                    // Determine if the pin is already connected.  Note that 
                    // VFW_E_NOT_CONNECTED is expected if the pin isn't yet connected.
                    hr = pIPinDownstream->ConnectedTo (&pPinUp);
                    if(FAILED(hr) && hr != VFW_E_NOT_CONNECTED)
                    {
                        ErrorBox("Failed in pIPinDownstream->ConnectedTo()!\n");
                        continue;
                    }

                    if ((PINDIR_INPUT == PinInfoDownstream.dir) && (pPinUp == NULL))
                    {
                        if (SUCCEEDED (pGraphBuilder->Connect(
                                        pIPinUpstream,
                                        pIPinDownstream))
                                        )
                        {
                            PinInfoDownstream.pFilter->Release();
                            PinInfoUpstream.pFilter->Release();
                            return S_OK;
                        }
                    }
                }

                PinInfoDownstream.pFilter->Release();
                pIPinDownstream = NULL;
            } // while next downstream filter pin

            //We are now back into the upstream pin loop
        } // if output pin

        pIPinUpstream = NULL;
        PinInfoUpstream.pFilter->Release();
    } // while next upstream filter pin

    return E_FAIL;
}

#ifdef DEBUG_ROT

HRESULT AddGraphToRot(
        IUnknown *pUnkGraph, 
        DWORD *pdwRegister
        ) 
{
    CComPtr <IMoniker>              pMoniker;
    CComPtr <IRunningObjectTable>   pROT;
    HRESULT hr;
    CStringW strw;

    if (FAILED(GetRunningObjectTable(0, &pROT)))
        return E_FAIL;

    strw.Format(L"FilterGraph %08x pid %08x", (DWORD_PTR)pUnkGraph, GetCurrentProcessId());

	hr = CreateItemMoniker(L"!", strw, &pMoniker);
    if (SUCCEEDED(hr)) 
        hr = pROT->Register(ROTFLAGS_REGISTRATIONKEEPSALIVE, pUnkGraph, 
                            pMoniker, pdwRegister);
        
    return hr;
}
#endif