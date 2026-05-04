/*
Copyright (c) David R. Cattley (dcattley@msn.com). All rights reserved.
Portions Copyright (c) 2000-2002, Microsoft Corporation.  All rights reserved.

Module Name:

    BDAFilterGraph.cpp

Abstract:

	ATSC BDA Source for TSReader (www.coolstf.com) with portions derived from
	the DirectX SDK BDA Sample graph.cpp

Author:

    David R. Cattley (dcattley@msn.com)

Revision History:

	01-Feb-2005 - Created
*/

#include "stdafx.h"
#include "resource.h"
#include "BDAFilterGraph.h"

//
// An application can advertise the existence of its filter graph
// by registering the graph with a global Running Object Table (ROT).
// The GraphEdit application can detect and remotely view the running
// filter graph, allowing you to 'spy' on the graph with GraphEdit.
//
// To enable registration in this sample, define REGISTER_FILTERGRAPH.
//
#define REGISTER_FILTERGRAPH

// We use channel 46 internally for testing.  Change this constant to any value.
#define DEFAULT_PHYSICAL_CHANNEL    46L

static const GUID PROPSETID_NIMTUNER_PROPERTIES = 
{ 0x87f6acbc, 0xbe46, 0x4ea5, { 0x90, 0x12, 0x1d, 0x21, 0x1c, 0x47, 0x3f, 0x71 } };

typedef enum
{
	FCV1236D_LG3302,
	LGTDVSH002F_LG3302,
	LGTDVSH062F_LG3303
}TunerID_t;

typedef enum
{
	QAM64_MODE	= 0x00,
	QAM256_MODE	= 0x01,
	VSBMODE		= 0x03,
}DTVDemodeMode_t;

typedef enum 
{
	INIM_DMODE_TUNER_ID			= 0,		// Get Only
	INIM_DMODE_SWRESET			= 1,		// Set Only
	INIM_DMODE_MODE				= 2,		// Set Only
	INIM_DMODE_SSINVERSION		= 3,		// Set Only
	INIM_DMODE_SNR_REGISTER		= 4			// Get Only

} KSPROPERTY_NIMTUNER_PROPERTIES;

// Constructor, initializes member variables
// and calls InitializeGraphBuilder
CBDAFilterGraph::CBDAFilterGraph() :
    m_fGraphBuilt(FALSE),
    m_fGraphRunning(FALSE),
    m_NetworkType(NT_ATSC),
    m_lMajorChannel(-1), 
    m_lMinorChannel(-1),
    m_lPhysicalChannel(DEFAULT_PHYSICAL_CHANNEL),
    m_dwGraphRegister (0)
{
    if(FAILED(InitializeGraphBuilder()))
        m_fGraphFailure = TRUE;
    else
        m_fGraphFailure = FALSE;
}


// Destructor
CBDAFilterGraph::~CBDAFilterGraph()
{
    if(m_fGraphRunning)
    {
        StopGraph();
    }

    if(m_fGraphBuilt || m_fGraphFailure)
    {
        TearDownGraph();
    }
}


// Instantiate graph object for filter graph building
HRESULT
CBDAFilterGraph::InitializeGraphBuilder()
{
    HRESULT hr = S_OK;
    
    // we have a graph already
    if (m_pFilterGraph)
        return S_OK;

    // create the filter graph
    if (FAILED (hr = m_pFilterGraph.CoCreateInstance (CLSID_FilterGraph)))
    {
        OutputDebugString("ATSCBDASource: Couldn't CoCreate IGraphBuilder\n");
        m_fGraphFailure = true;
        return hr;
    }
    
    return hr;
}


// BuildGraph sets up devices, adds and connects filters
HRESULT
CBDAFilterGraph::BuildGraph(NETWORK_TYPE NetType)
{
    HRESULT hr = S_OK;
    m_NetworkType = NetType;

    // if we have already have a filter graph, tear it down
    if(m_fGraphBuilt)
    {
        if(m_fGraphRunning)
        {
            hr = StopGraph ();
        }

        hr = TearDownGraph ();
    }

#ifdef REGISTER_FILTERGRAPH
    hr = AddGraphToRot (m_pFilterGraph, &m_dwGraphRegister);
    if (FAILED(hr))
    {
        ///OutputDebugString("ATSCBDASource: Failed to register filter graph with ROT!\n");
        m_dwGraphRegister = 0;
    }
#endif

    // STEP 1: load network provider first so that it can configure other
    // filters, such as configuring the demux to sprout output pins.
    // We also need to submit a tune request to the Network Provider so it will
    // tune to a channel
    if(FAILED (hr = LoadNetworkProvider()))
    {
        OutputDebugString("ATSCBDASource: Cannot load network provider\n");
        TearDownGraph();
        m_fGraphFailure = true;
        return hr;
    }

	hr = m_pNetworkProvider.QueryInterface(&m_pITuner);
    if(FAILED (hr))
    {
        OutputDebugString("ATSCBDASource: pNetworkProvider->QI: Can't QI for ITuner.\n");
        TearDownGraph();
        m_fGraphFailure = true;
        return hr;
    }

    // create a tune request to initialize the network provider
    // before connecting other filters
    CComPtr<IATSCChannelTuneRequest> pATSCTuneRequest;
	hr = CreateATSCTuneRequest(m_lPhysicalChannel, m_lMajorChannel, m_lMinorChannel, &pATSCTuneRequest);
	if (FAILED(hr))
    {
        OutputDebugString("ATSCBDASource: Cannot create tune request\n");
        TearDownGraph();
        m_fGraphFailure = true;
        return hr;
    }

    //submit the tune request to the network provider
    hr = m_pITuner->put_TuneRequest(pATSCTuneRequest);
    if(FAILED(hr))
    {
        OutputDebugString("ATSCBDASource: Cannot submit the tune request\n");
        TearDownGraph();
        m_fGraphFailure = true;
        return hr;
    }


    // STEP2: Load tuner device and connect to network provider
	hr = LoadFilter(KSCATEGORY_BDA_NETWORK_TUNER, &m_pTunerDevice, m_pNetworkProvider, TRUE);
	if (FAILED(hr))
    {
        OutputDebugString("ATSCBDASource: Cannot load tuner device and connect network provider\n");
        TearDownGraph();
        m_fGraphFailure = true;
        return hr;
    }

    // Step3: Load capture device and connect to tuner device
	hr = LoadFilter(KSCATEGORY_BDA_RECEIVER_COMPONENT, &m_pCaptureDevice, m_pTunerDevice, TRUE);
	if(FAILED(hr))
    {
        OutputDebugString("ATSCBDASource: Cannot load capture device and connect tuner\n");
        TearDownGraph();
        m_fGraphFailure = true;
        return hr;
    }

	// Step4: Load Sample Grabber
	hr = LoadSampleGrabber();
	if (FAILED(hr))
	{
        OutputDebugString("ATSCBDASource: Cannot load Sample Grabber\n");
        TearDownGraph();
        m_fGraphFailure = true;
        return hr;
	}

    // Step5: Load demux
	hr = LoadDemux();
	if(FAILED(hr))
    {
        OutputDebugString("ATSCBDASource: Cannot load demux\n");
        TearDownGraph();
        m_fGraphFailure = true;
        return hr;
    }

    //
    // this next call loads and connects filters associated with
    // the demultiplexor. if you want to manually load individual
    // filters such as audio and video decoders, use the code at
    // the bottom of this file
    //

    // Step6: Render demux pins
	hr = RenderDemux();
	if(FAILED(hr))
    {
        OutputDebugString("ATSCBDASource: Cannot load demux\n");
        TearDownGraph();
        m_fGraphFailure = true;
        return hr;
    }

    m_fGraphBuilt = true;
    m_fGraphFailure = false;
    
    return S_OK;
}


// Loads the correct tuning space based on NETWORK_TYPE that got
// passed into BuildGraph()
HRESULT
CBDAFilterGraph::LoadTuningSpace()
{   
    CComPtr <ITuningSpaceContainer>  pITuningSpaceContainer;

    // get the tuningspace container for all the tuning spaces from SYSTEM_TUNING_SPACES
    HRESULT hr = pITuningSpaceContainer.CoCreateInstance(CLSID_SystemTuningSpaces);
    if (FAILED (hr))
    {
        OutputDebugString("ATSCBDASource: Could not CoCreate SystemTuningSpaces\n");
        return hr;
    }

    CComVariant var (m_NetworkType);

    hr = pITuningSpaceContainer->get_Item(var, &m_pITuningSpace);

    if(FAILED(hr))
    {
        OutputDebugString("ATSCBDASource: Unable to retrieve Tuning Space\n");
    }

    return hr;
}

#ifdef QAM
BOOL CBDAFilterGraph::DevFilterProperty(IBaseFilter* pFilter,REFGUID pGuid,int fGetSet,int nIndex, PBYTE pInBuffer, int nInSize, PBYTE pOutBuffer, int nOutSize)
{
	IKsPropertySet* pKPS=NULL;
	DWORD dwSupport;
	BOOL bOk = FALSE;

	if(!pFilter)
	{
		return FALSE;
	}
	if (FAILED(pFilter->QueryInterface(IID_IKsPropertySet, (void**)&pKPS)))
	{
		return bOk;
	}

	if(fGetSet)
	{
		if (pKPS->QuerySupported(pGuid, nIndex, &dwSupport) == S_OK)
		{
			if (dwSupport & KSPROPERTY_SUPPORT_SET)
			{
				if (pKPS->Set(pGuid, nIndex,pInBuffer, nInSize, pOutBuffer, nOutSize) == S_OK)
				{
					bOk = TRUE;
				}
			}
		}
	}
	else
	{
		DWORD dwReturned;

		if (pKPS->QuerySupported(pGuid, nIndex, &dwSupport) == S_OK)
		{
			if (dwSupport & KSPROPERTY_SUPPORT_GET)
			{
				if (pKPS->Get(pGuid, nIndex,pInBuffer, nInSize, pOutBuffer, nOutSize, &dwReturned) == S_OK)
				{
					bOk = TRUE;
				}
			}
		}
	}
	if(pKPS)
	{
		pKPS->Release();
	}
	return bOk;
}

HRESULT CBDAFilterGraph::TunerIface(DWORD fGetSet,DWORD fCommand,DWORD* pdwValue)
{
	HRESULT hr = E_FAIL;

	if(m_tvTuner)  
	{
		hr = DevFilterProperty( m_tvTuner
					,PROPSETID_NIMTUNER_PROPERTIES 
					,fGetSet
					,fCommand
					,(BYTE*)pdwValue
					,sizeof(DWORD)
					,(BYTE*)pdwValue
					,sizeof(DWORD));
	}
	return hr;
}

DWORD CBDAFilterGraph::GetQAMSNR()
{
	DWORD lComData = -1;

	if (m_tvTuner)
	{
		TunerIface(0,INIM_DMODE_SNR_REGISTER,&lComData);
	}
	return lComData;
}

HRESULT CBDAFilterGraph::ScanSignal(int lPhyCh, LONG * plSignal)
{
	DWORD lComData;
	IAMTVTuner *pTvTuner = NULL;
	HRESULT hr;

	hr = m_tvTuner->QueryInterface(IID_IAMTVTuner,(void **)&pTvTuner);
	if (pTvTuner)
	{
		int i;

		AMTunerModeType Mode;

		hr = pTvTuner->get_Mode(&Mode);
		if (Mode != 16)
		{
			hr = pTvTuner->put_Mode((tagAMTunerModeType)16);			// for ATSC( or Digital mode )
			OutputDebugString("ATSCBDASource QAM: Switched to digital mode\n");
		}

		//TunerIface(0,INIM_DMODE_TUNER_ID,&lComData);
		//if(lComData==LGTDVSH062F_LG3303)
		hr = pTvTuner->AutoTune(lPhyCh, plSignal);
		for (i = 0; i < 2; i++)
		{
			*plSignal = 0;

			// Try QAM256 non-invered
			lComData = 0;
			TunerIface(1,INIM_DMODE_SSINVERSION,&lComData);
			lComData = QAM256_MODE;
			TunerIface(1,INIM_DMODE_MODE,&lComData);
			//hr = pTvTuner->AutoTune(lPhyCh, plSignal);
			pTvTuner->SignalPresent(plSignal);
			if(*plSignal)
			{
				OutputDebugString("ATSCBDASource QAM: locked 256 non inverted\n");
				return 0;	// locked
			}

			// Try QAM256 invered
			lComData = 1;
			TunerIface(1,INIM_DMODE_SSINVERSION,&lComData);
			lComData = QAM256_MODE;
			TunerIface(1,INIM_DMODE_MODE,&lComData);
			pTvTuner->SignalPresent(plSignal);
			if(*plSignal)
			{
				OutputDebugString("ATSCBDASource QAM: locked 256 inverted\n");
				return 0;	// locked
			}

			// Try QAM64 non-inverted
			lComData = 0;
			TunerIface(1,INIM_DMODE_SSINVERSION,&lComData);
			lComData = QAM64_MODE;
			TunerIface(1,INIM_DMODE_MODE,&lComData);
			pTvTuner->SignalPresent(plSignal);
			if(*plSignal)
			{
				OutputDebugString("ATSCBDASource QAM: locked 64 non inverted\n");
				return 0;	// locked
			}

			// Try QAM64 inverted
			lComData = 1;
			TunerIface(1,INIM_DMODE_SSINVERSION,&lComData);
			lComData = QAM64_MODE;
			TunerIface(1,INIM_DMODE_MODE,&lComData);
			pTvTuner->SignalPresent(plSignal);
			if(*plSignal)
			{
				OutputDebugString("ATSCBDASource QAM-GT: locked 64 inverted\n");
				return 0;	// locked
			}

			Sleep(50);
		}
	}
	return 1;	// shouldn't get here, so no lock
}

IBaseFilter *CBDAFilterGraph::BindFilterToObject(REFCLSID clsid, char *filtername)
{
	ICreateDevEnum	*pCreateDevEnum=NULL;
	IEnumMoniker	*pEm;
	ULONG		cFetched;
	IMoniker	*pM;
	HRESULT		hr;
	IBaseFilter     *basefilter = NULL;
//	EC_TVFG		tagEcTvfg;
	char 		achFriendlyName[160];

	while(1)
	{
		hr = CoCreateInstance(CLSID_SystemDeviceEnum, NULL, CLSCTX_INPROC_SERVER, IID_ICreateDevEnum, (void**)&pCreateDevEnum);
		if(!SUCCEEDED(hr))
		{
			break;
		}
		hr = pCreateDevEnum->CreateClassEnumerator(clsid, &pEm, 0);
		pCreateDevEnum->Release();
		if(!SUCCEEDED(hr))
		{
			break;
		}
		
		pEm->Reset();
		while(pEm->Next(1, &pM, &cFetched) == S_OK)
		{
			IPropertyBag *pBag;
			achFriendlyName[0] = 0;
			hr = pM->BindToStorage(0, 0, IID_IPropertyBag, (void **)&pBag);
			if(SUCCEEDED(hr))
			{
				VARIANT var;
				var.vt = VT_BSTR;
				hr = pBag->Read(L"FriendlyName", &var, NULL);
				if (hr == NOERROR)
				{	
					WideCharToMultiByte(CP_ACP, 0, var.bstrVal, -1,	achFriendlyName, 80, NULL, NULL);
					SysFreeString(var.bstrVal);
				}
				pBag->Release();
				if (strcmp(achFriendlyName,filtername) == 0)
				{
					pM->BindToObject(0, 0, IID_IBaseFilter, (void**)&basefilter);
					pM->Release();
					break;
				}
			}
			pM->Release();
		}
		pEm->Release();
		break;
	}
	return basefilter;
}

BOOL CBDAFilterGraph::LoadTuner()
{ 
	//
	// Load GT tuner filter
	//
#ifndef CREATOR
	char strTunerDeviceFriendlyName[128] = {"USB HDTV-GT Tuner"};
#else CREATOR
	char strTunerDeviceFriendlyName[128] = {"USB HDTV-CR Tuner"};
#endif CREATOR
	if(!m_tvTuner )
	{
		HRESULT hr;
		WORD wstrFiltername[128];

		//m_tvTuner = (IBaseFilter*)BindFilterToObject(CLSID_WdmStreamingTVTunerDevices,strTunerDeviceFriendlyName);
		m_tvTuner = (IBaseFilter*)BindFilterToObject(AM_KSCATEGORY_TVTUNER,strTunerDeviceFriendlyName);
		if (m_tvTuner == NULL)
		{
			return FALSE;
		}
		MultiByteToWideChar(CP_ACP,0,strTunerDeviceFriendlyName,-1,wstrFiltername,128);
		hr = m_pFilterGraph->AddFilter(m_tvTuner,wstrFiltername);		// add to filtergraph
	}
	return TRUE;
}

#endif QAM

// Creates an ATSC Tune Request
HRESULT
CBDAFilterGraph::CreateATSCTuneRequest(
        LONG lPhysicalChannel,
        LONG lMajorChannel, 
        LONG lMinorChannel,
        IATSCChannelTuneRequest**   pTuneRequest
    )
{
    HRESULT hr = S_OK;

    if (pTuneRequest == NULL)
    {
        OutputDebugString("ATSCBDASource: Invalid pointer\n");
        return E_POINTER;
    }

    // Making sure we have a valid tuning space
    if (m_pITuningSpace == NULL)
    {
        OutputDebugString("ATSCBDASource: Tuning Space is NULL\n");
        return E_FAIL;
    }

    //  Create an instance of the ATSC tuning space
    CComQIPtr <IATSCTuningSpace> pATSCTuningSpace (m_pITuningSpace);
    if (!pATSCTuningSpace)
    {
        OutputDebugString("ATSCBDASource: Cannot QI for an IATSCTuningSpace\n");
        return E_FAIL;
    }

    //  Create an empty tune request.
    CComPtr <ITuneRequest> pNewTuneRequest;
    hr = pATSCTuningSpace->CreateTuneRequest(&pNewTuneRequest);

    if (FAILED (hr))
    {
        OutputDebugString("ATSCBDASource: CreateTuneRequest: Can't create tune request.\n");
        return hr;
    }

    //query for an IATSCChannelTuneRequest interface pointer
    CComQIPtr <IATSCChannelTuneRequest> pATSCTuneRequest (pNewTuneRequest);
    if (!pATSCTuneRequest)
    {
        OutputDebugString("ATSCBDASource: CreateATSCTuneRequest: Can't QI for IATSCChannelTuneRequest.\n");
        return E_FAIL;
    }

    //  Set the initial major and minor channels
    hr = pATSCTuneRequest->put_Channel(lMajorChannel);
    if(FAILED(hr))
    {
        OutputDebugString("ATSCBDASource: put_Channel failed\n");
        return hr;
    }

    hr = pATSCTuneRequest->put_MinorChannel(lMinorChannel);
    if(FAILED(hr))
    {
        OutputDebugString("ATSCBDASource: put_MinorChannel failed\n");
        return hr;
    }

    CComPtr <IATSCLocator> pATSCLocator;
    hr = pATSCLocator.CoCreateInstance (CLSID_ATSCLocator);
    if (FAILED( hr))
    {
        OutputDebugString("ATSCBDASource: Cannot create the ATSC locator failed\n");
        return hr;
    }

    //  Set the initial physical channel.
    //
    hr = pATSCLocator->put_PhysicalChannel (lPhysicalChannel);
    if (FAILED( hr))
    {
        OutputDebugString("ATSCBDASource: Cannot put the physical channel\n");
        return hr;
    }

    hr = pATSCTuneRequest->put_Locator (pATSCLocator);
    if (FAILED (hr))
    {
        OutputDebugString("ATSCBDASource: Cannot put the locator\n");
        return hr;
    }

    hr = pATSCTuneRequest.QueryInterface (pTuneRequest);

    return hr;
}


// LoadNetworkProvider loads network provider
HRESULT
CBDAFilterGraph::LoadNetworkProvider()
{
    HRESULT     hr = S_OK;
    CComBSTR    bstrNetworkType;
    CLSID       CLSIDNetworkType;

    // obtain tuning space then load network provider
    if(m_pITuningSpace == NULL)
    {
        hr = LoadTuningSpace();
        if(FAILED(hr))
        {
            OutputDebugString("ATSCBDASource: Cannot load TuningSpace\n");
            return hr;
        }
    }

    // Get the current Network Type clsid
    hr = m_pITuningSpace->get_NetworkType(&bstrNetworkType);
    if (FAILED (hr))
    {
        OutputDebugString("ATSCBDASource: ITuningSpace::Get Network Type failed\n");
        return hr;
    }

    hr = CLSIDFromString(bstrNetworkType, &CLSIDNetworkType);
    if (FAILED (hr))
    {
        OutputDebugString("ATSCBDASource: Couldn't get CLSIDFromString\n");
        return hr;
    }

    // create the network provider based on the clsid obtained from the tuning space
    hr = CoCreateInstance(CLSIDNetworkType, NULL, CLSCTX_INPROC_SERVER,
                          IID_IBaseFilter, 
                          reinterpret_cast<void**>(&m_pNetworkProvider));
    if (FAILED (hr))
    {
        OutputDebugString("ATSCBDASource: Couldn't CoCreate Network Provider\n");
        return hr;
    }

    //add the Network Provider filter to the graph
    hr = m_pFilterGraph->AddFilter(m_pNetworkProvider, L"Network Provider");

    return hr;
}


// enumerates through registered filters
// instantiates the the filter object and adds it to the graph
// it checks to see if it connects to upstream filter
// if not,  on to the next enumerated filter
// used for tuner, capture, MPE Data Filters and decoders that
// could have more than one filter object
// if pUpstreamFilter is NULL don't bother connecting
HRESULT
CBDAFilterGraph::LoadFilter(
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

    if (!m_pICreateDevEnum)
    {
        hr = m_pICreateDevEnum.CoCreateInstance(CLSID_SystemDeviceEnum);
        if (FAILED (hr))
        {
            OutputDebugString("ATSCBDASource: LoadFilter(): Cannot CoCreate ICreateDevEnum");
            return hr;
        }
    }

    // obtain the enumerator
    hr = m_pICreateDevEnum->CreateClassEnumerator(clsid, &pIEnumMoniker, 0);
    // the call can return S_FALSE if no moniker exists, so explicitly check S_OK
    if (FAILED (hr))
    {
        OutputDebugString("ATSCBDASource: LoadFilter(): Cannot CreateClassEnumerator");
        return hr;
    }
    if (S_OK != hr)  // Class not found
    {
        OutputDebugString("ATSCBDASource: LoadFilter(): Class not found, CreateClassEnumerator returned S_FALSE");
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


        hr = m_pFilterGraph->AddFilter (pFilter, varBSTR.bstrVal);

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
                hr = m_pFilterGraph->RemoveFilter(pFilter);

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
    return S_OK;
}

HRESULT
CBDAFilterGraph::LoadSampleGrabber()
{
	CComPtr<ISampleGrabber> sampleGrabber;
	HRESULT hr = sampleGrabber.CoCreateInstance(CLSID_SampleGrabber, NULL, CLSCTX_INPROC_SERVER);
	if (FAILED(hr))
	{
        OutputDebugString("ATSCBDASource: Could not CoCreateInstance CLSID_SampleGrabber\n");
		return hr;
	}

	AM_MEDIA_TYPE sgType;
	ZeroMemory(&sgType, sizeof(sgType));

	sgType.majortype=GUID_NULL;
	sgType.subtype=GUID_NULL;
	sgType.formattype=GUID_NULL;

	hr = sampleGrabber->SetMediaType(&sgType);
	if (FAILED(hr))
	{
        OutputDebugString("ATSCBDASource: Could not set media type on Sample Grabber.\n");
		return hr;
	}

	hr = sampleGrabber.QueryInterface(&m_pSampleGrabber);
	if (FAILED(hr))
	{
        OutputDebugString("ATSCBDASource: Unable to get filter interface to Sample Grabber\n");
        return hr;
	}

	hr = m_pFilterGraph->AddFilter(m_pSampleGrabber, L"Sample Grabber");
    if(FAILED(hr))
    {
        OutputDebugString("ATSCBDASource: Unable to add Sample Grabber filter to graph\n");
        return hr;
    }

	return hr;
}

// loads the demux into the FilterGraph
HRESULT
CBDAFilterGraph::LoadDemux()
{
    HRESULT hr = S_OK;
    
    hr = CoCreateInstance(
                        CLSID_MPEG2Demultiplexer, 
                        NULL, 
                        CLSCTX_INPROC_SERVER,
                        IID_IBaseFilter, 
                        reinterpret_cast<void**>(&m_pDemux)
                        );
    if (FAILED (hr))
    {
        OutputDebugString("ATSCBDASource: Could not CoCreateInstance CLSID_MPEG2Demultiplexer\n");
        return hr;
    }

    hr = m_pFilterGraph->AddFilter(m_pDemux, L"Demux");
    if(FAILED(hr))
    {
        OutputDebugString("ATSCBDASource: Unable to add demux filter to graph\n");
        return hr;
    }

    return hr;
}


// renders demux output pins
HRESULT
CBDAFilterGraph::RenderDemux()
{
    HRESULT             hr = S_OK;

    if (!m_pDemux)
    {
        return E_FAIL;
    }

    // connect the Sample Grabber to the capture device
    hr = ConnectFilters(m_pCaptureDevice, m_pSampleGrabber);

    if (FAILED (hr))
    {
        OutputDebugString("ATSCBDASource: Cannot connect Sample Grabber to capture filter\n");
        return hr;
    }

    // connect the demux to the Sample Grabber
    hr = ConnectFilters(m_pSampleGrabber, m_pDemux);
    if (FAILED (hr))
    {
        OutputDebugString("ATSCBDASource: Cannot connect demux to Sample Grabber\n");
        return hr;
    }

    // load transform information filter and connect it to the demux
    hr = LoadFilter (
                    KSCATEGORY_BDA_TRANSPORT_INFORMATION, 
                    &m_pTIF, 
                    m_pDemux, 
                    TRUE
                    );
    if (FAILED (hr))
    {
        OutputDebugString("ATSCBDASource: Cannot load TIF\n");
        return hr;
    }

    return hr;
}


// removes each filter from the graph
HRESULT
CBDAFilterGraph::TearDownGraph()
{
    HRESULT hr = S_OK;
    CComPtr <IBaseFilter> pFilter;
    CComPtr <IEnumFilters> pIFilterEnum;

    m_pITuningSpace = NULL;

    if(m_fGraphBuilt || m_fGraphFailure)
    {
        // unload manually added filters
#ifdef QAM
		m_pFilterGraph->RemoveFilter(m_tvTuner);
#endif QAM
        m_pFilterGraph->RemoveFilter(m_pTIF);
		m_pFilterGraph->RemoveFilter(m_pSampleGrabber);
        m_pFilterGraph->RemoveFilter(m_pDemux);
        m_pFilterGraph->RemoveFilter(m_pNetworkProvider);
        m_pFilterGraph->RemoveFilter(m_pTunerDevice);
        m_pFilterGraph->RemoveFilter(m_pCaptureDevice);

        m_pTIF = NULL;
        m_pDemux = NULL;
		m_pSampleGrabber = NULL;
        m_pNetworkProvider = NULL;
        m_pTunerDevice = NULL;
        m_pCaptureDevice = NULL;
#ifdef QAM
		//m_pFilterGraph = NULL;
#endif QAM

        // now go unload rendered filters
        hr = m_pFilterGraph->EnumFilters(&pIFilterEnum);
        if(FAILED(hr))
        {
            OutputDebugString("ATSCBDASource: TearDownGraph: cannot EnumFilters\n");
            return E_FAIL;
        }

        pIFilterEnum->Reset();

        while(pIFilterEnum->Next(1, &pFilter, 0) == S_OK) // addrefs filter
        {
            hr = m_pFilterGraph->RemoveFilter(pFilter);

            if (FAILED (hr))
                return hr;

            pIFilterEnum->Reset();
            pFilter.Release ();
        }
    }

#ifdef REGISTER_FILTERGRAPH
    if (m_dwGraphRegister)
    {
        RemoveGraphFromRot(m_dwGraphRegister);
        m_dwGraphRegister = 0;
    }
#endif

    m_fGraphBuilt = FALSE;
    return S_OK;
}


// ConnectFilters is called from BuildGraph
// to enumerate and connect pins
HRESULT
CBDAFilterGraph::ConnectFilters(
    IBaseFilter* pFilterUpstream, 
    IBaseFilter* pFilterDownstream
    )
{
    HRESULT         hr = E_FAIL;

    CComPtr <IPin>  pIPinUpstream;


    PIN_INFO        PinInfoUpstream;
    PIN_INFO        PinInfoDownstream;

    // validate passed in filters
    ASSERT (pFilterUpstream);
    ASSERT (pFilterDownstream);

    // grab upstream filter's enumerator
    CComPtr <IEnumPins> pIEnumPinsUpstream;
    hr = pFilterUpstream->EnumPins(&pIEnumPinsUpstream);

    if(FAILED(hr))
    {
        OutputDebugString("ATSCBDASource: Cannot Enumerate Upstream Filter's Pins\n");
        return hr;
    }

    // iterate through upstream filter's pins
    while (pIEnumPinsUpstream->Next (1, &pIPinUpstream, 0) == S_OK)
    {
        hr = pIPinUpstream->QueryPinInfo (&PinInfoUpstream);
        if(FAILED(hr))
        {
            OutputDebugString("ATSCBDASource: Cannot Obtain Upstream Filter's PIN_INFO\n");
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
                OutputDebugString("ATSCBDASource: Cannot enumerate pins on downstream filter!\n");
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
                        OutputDebugString("ATSCBDASource: Failed in pIPinDownstream->ConnectedTo()!\n");
                        continue;
                    }

                    if ((PINDIR_INPUT == PinInfoDownstream.dir) && (pPinUp == NULL))
                    {
                        if (SUCCEEDED (m_pFilterGraph->Connect(
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


// RunGraph checks to see if a graph has been built
// if not it calls BuildGraph
// RunGraph then calls MediaCtrl-Run
HRESULT
CBDAFilterGraph::RunGraph()
{
    // check to see if the graph is already running
    if(m_fGraphRunning)
        return S_OK;

    HRESULT hr = S_OK;
    if (m_pIMediaControl == NULL)
        hr = m_pFilterGraph.QueryInterface (&m_pIMediaControl);

    if (SUCCEEDED (hr))
    {
        // run the graph
        hr = m_pIMediaControl->Run();
        if(SUCCEEDED(hr))
        {
            m_fGraphRunning = true;
        }
        else
        {
            // stop parts of the graph that ran
            m_pIMediaControl->Stop();
            OutputDebugString("ATSCBDASource: Cannot run graph\n");
        }
    }

    return hr;
}


// StopGraph calls MediaCtrl - Stop
HRESULT
CBDAFilterGraph::StopGraph()
{
    // check to see if the graph is already stopped
    if(m_fGraphRunning == false)
        return S_OK;

    HRESULT hr = S_OK;

    ASSERT (m_pIMediaControl);
    // pause before stopping
    hr = m_pIMediaControl->Pause();

    // stop the graph
    hr = m_pIMediaControl->Stop();

    m_fGraphRunning = (FAILED (hr))?true:false;
    return hr;
}



HRESULT
CBDAFilterGraph::ChangeChannel(
        LONG lPhysicalChannel,
        LONG lMajorChannel, 
        LONG lMinorChannel
        )
{
    HRESULT hr = S_OK;
    m_lMajorChannel = lMajorChannel;
    m_lMinorChannel = lMinorChannel;
    m_lPhysicalChannel = lPhysicalChannel;

    if (!m_pNetworkProvider)
    {
        OutputDebugString("ATSCBDASource: The FilterGraph is not yet built.\n");
        return E_FAIL;
    }

#ifdef QAM
		if (LoadTuner() == TRUE)
		{
			LONG lSignal;
			return ScanSignal(lPhysicalChannel, &lSignal);
			
		}
#endif QAM

    // create tune request
    CComPtr <IATSCChannelTuneRequest> pTuneRequest;
    hr = CreateATSCTuneRequest(
                            m_lPhysicalChannel, 
                            lMajorChannel, 
                            lMinorChannel,
                            &pTuneRequest
                            );
    if(SUCCEEDED(hr))
    {
        hr = m_pITuner->put_TuneRequest (pTuneRequest);
        if (FAILED (hr))
            OutputDebugString("ATSCBDASource: Cannot submit tune request\n");
    }
    else
    {
        OutputDebugString("ATSCBDASource: Cannot Change Channels\n");
    }
    return hr;
}


#ifdef REGISTER_FILTERGRAPH

// Adds a DirectShow filter graph to the Running Object Table,
// allowing GraphEdit to "spy" on a remote filter graph.
HRESULT CBDAFilterGraph::AddGraphToRot(
        IUnknown *pUnkGraph, 
        DWORD *pdwRegister
        ) 
{
    CComPtr <IMoniker>              pMoniker;
    CComPtr <IRunningObjectTable>   pROT;
    WCHAR wsz[128];
    HRESULT hr;

    if (FAILED(GetRunningObjectTable(0, &pROT)))
        return E_FAIL;

	StringCbPrintfW(wsz, sizeof(wsz), L"FilterGraph %p pid %08x\0", pUnkGraph, GetCurrentProcessId());

    hr = CreateItemMoniker(L"!", wsz, &pMoniker);
    if (SUCCEEDED(hr)) 
        hr = pROT->Register(ROTFLAGS_REGISTRATIONKEEPSALIVE, pUnkGraph, 
                            pMoniker, pdwRegister);
        
    return hr;
}

// Removes a filter graph from the Running Object Table
void CBDAFilterGraph::RemoveGraphFromRot(
        DWORD pdwRegister
        )
{
    CComPtr <IRunningObjectTable> pROT;

    if (SUCCEEDED(GetRunningObjectTable(0, &pROT))) 
        pROT->Revoke(pdwRegister);

}

#endif


//
// USE THE CODE BELOW IF YOU WANT TO MANUALLY LOAD AND
// CONNECT A/V DECODERS TO THE DEMUX OUTPUT PINS
//

/*
To use this code:
1) in LoadAudioDecoder() and LoadVideoDecoder(), fill in decoder specific information (clsid)
2) goto BuildGraph() and replace RenderDemux() with BuildAVSegment()
*/

/*
// Builds the Audio, Video segment of the digital TV graph.
// Demux -> AV Decoder -> OVMixer -> Video Renderer
HRESULT
CBDAFilterGraph::BuildAVSegment()
{
    HRESULT hr = E_FAIL;

    // connect the demux to the capture device
    hr = ConnectFilters(m_pCaptureDevice, m_pDemux);

    hr = LoadVideoDecoder();

    if(SUCCEEDED(hr) && m_pVideoDecoder)
    {
        // Connect the demux & video decoder
        hr = ConnectFilters(m_pDemux, m_pVideoDecoder);

        if(FAILED(hr))
        {
            ErrorMessageBox("Connecting Demux & Video Decoder Failed\n");
            goto err;
        }
    }
    else
    {
        //ErrorMessageBox("Unable to load Video Decoder\n");
        goto err;
    }

    //Audio
    hr = LoadAudioDecoder();

    if(SUCCEEDED(hr) && m_pAudioDecoder)
    {
        hr = ConnectFilters(m_pDemux, m_pAudioDecoder);

        if(FAILED(hr))
        {
            ErrorMessageBox("Connecting Deumx & Audio Decoder Failed\n");
            goto err;
        }
    }
    else
    {
        ErrorMessageBox("Unable to load Audio Decoder\n");
        goto err;
    }

    // Create the OVMixer & Video Renderer for the video segment
    hr = CoCreateInstance(CLSID_OverlayMixer, NULL, CLSCTX_INPROC_SERVER,
            IID_IBaseFilter, reinterpret_cast<void**>(&m_pOVMixer));

    if(SUCCEEDED(hr) && m_pOVMixer)
    {
        hr = m_pFilterGraph->AddFilter(m_pOVMixer, L"OVMixer");

        if(FAILED(hr))
        {
            ErrorMessageBox("Adding OVMixer to the FilterGraph Failed\n");
            goto err;
        }
    }
    else
    {
        ErrorMessageBox("Loading OVMixer Failed\n");
        goto err;
    }

    hr = CoCreateInstance(CLSID_VideoRenderer, NULL, CLSCTX_INPROC_SERVER,
            IID_IBaseFilter, reinterpret_cast<void**>(&m_pVRenderer));

    if(SUCCEEDED(hr) && m_pVRenderer)
    {
        hr = m_pFilterGraph->AddFilter(m_pVRenderer, L"Video Renderer");

        if(FAILED(hr))
        {
            ErrorMessageBox("Adding Video Renderer to the FilterGraph Failed\n");
            goto err;
        }
    }
    else
    {
        ErrorMessageBox("Loading Video Renderer Failed\n");
        goto err;
    }

    // Split AV Decoder? Then add Default DirectSound Renderer to the filtergraph
    if(m_pVideoDecoder != m_pAudioDecoder)
    {
        hr = CoCreateInstance(CLSID_DSoundRender, NULL,
                        CLSCTX_INPROC_SERVER, IID_IBaseFilter,
                        reinterpret_cast<void**>(&m_pDDSRenderer));

        if(SUCCEEDED(hr) && m_pDDSRenderer)
        {
            hr = m_pFilterGraph->AddFilter(m_pDDSRenderer, L"Sound Renderer");

            if(FAILED(hr))
            {
                ErrorMessageBox("Adding DirectSound Device to the FilterGraph Failed\n");
                goto err;
            }
        }
        else
        {
            ErrorMessageBox("Loading DirectSound Device Failed\n");
            goto err;
        }
    }

    hr = ConnectFilters(m_pVideoDecoder, m_pOVMixer);

    if(FAILED(hr))
    {
        ErrorMessageBox("Connecting Capture & OVMixer Failed\n");
        goto err;
    }

    hr = ConnectFilters(m_pOVMixer, m_pVRenderer);

    if(FAILED(hr))
    {
        ErrorMessageBox("Connecting OVMixer & Video Renderer Failed\n");
        goto err;
    }

    // Split AV Decoder & if you need audio too ?? then connect Audio decoder to Sound Renderer
    if(m_pVideoDecoder != m_pAudioDecoder)
    {
        hr = ConnectFilters(m_pAudioDecoder, m_pDDSRenderer);

        if(FAILED(hr))
        {
            ErrorMessageBox("Connecting AudioDecoder & DirectSound Device Failed\n");
            goto err;
        }
    }

err:
    return hr;
}

// placeholders for real decoders
DEFINE_GUID(CLSID_FILL_IN_NAME_AUDIO_DECODER, 0xFEEDFEED, 0x0000, 0x0000, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00);
DEFINE_GUID(CLSID_FILL_IN_NAME_VIDEO_DECODER, 0xFEEDFEED, 0x0000, 0x0000, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00);

HRESULT
CBDAFilterGraph::LoadVideoDecoder()
{
    HRESULT hr = E_FAIL;

    hr = CoCreateInstance(CLSID_FILL_IN_NAME_VIDEO_DECODER, NULL,
            CLSCTX_INPROC_SERVER, IID_IBaseFilter,
            reinterpret_cast<void**>(&m_pVideoDecoder));

    if(SUCCEEDED(hr) && m_pVideoDecoder)
    {
        hr = m_pFilterGraph->AddFilter(m_pVideoDecoder, L"Video Decoder");

        if(FAILED(hr))
        {
            ErrorMessageBox("Unable to add Video Decoder filter to graph\n");
        }
    }

    return hr;
}


HRESULT
CBDAFilterGraph::LoadAudioDecoder()
{
    HRESULT hr = E_FAIL;

    hr = CoCreateInstance(CLSID_FILL_IN_NAME_AUDIO_DECODER, NULL,
            CLSCTX_INPROC_SERVER, IID_IBaseFilter,
            reinterpret_cast<void**>(&m_pAudioDecoder));

    if(SUCCEEDED(hr) && m_pAudioDecoder)
    {
        hr = m_pFilterGraph->AddFilter(m_pAudioDecoder, L"Audio Decoder");

        if(FAILED(hr))
        {
            ErrorMessageBox("Unable to add Audio filter to graph\n");
        }
    }

    return hr;
}

// ErrorMessageBox
//
// Opens a Message box with a error message in it.  The user can     
// select the OK button to continue.
//
VOID
ErrorMessageBox(LPTSTR szFormat, ...)
{
    static TCHAR szBuffer[2048];  // Large buffer
    const size_t NUMCHARS = sizeof(szBuffer) / sizeof(szBuffer[0]);
    const int LASTCHAR = NUMCHARS - 1;

    // Format the input string
    va_list pArgs;
    va_start(pArgs, szFormat);

    // Use a bounded buffer size to prevent buffer overruns.  Limit count to
    // character size minus one to allow for a NULL terminating character.

	StringCbVPrintf(szBuffer, sizeof(szBuffer), szFormat, pArgs);
    va_end(pArgs);

    // Ensure that the formatted string is NULL-terminated
    szBuffer[LASTCHAR] = TEXT('\0');

    // Display a message box with the formatted string
    MessageBox(NULL, szBuffer, TEXT("Error!\0"), MB_OK|MB_ICONEXCLAMATION|MB_TASKMODAL);
}
*/

