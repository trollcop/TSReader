//------------------------------------------------------------------------------
// File: Graph.cpp
//
// Desc: Sample code for BDA graph building.
//
//
// Copyright (c) 2003, Conexant System, Inc. All rights reserved.
//------------------------------------------------------------------------------
#include "stdafx.h"
#include "graph.h"
#include <stdlib.h>
#include <stdio.h>
#include <TCHAR.h>

// 
// NOTE: In this sample, text strings are hard-coded for 
// simplicity and for readability.  For product code, you should
// use string tables and LoadString().
//

//
// An application can advertise the existence of its filter graph
// by registering the graph with a global Running Object Table (ROT).
// The GraphEdit application can detect and remotely view the running
// filter graph, allowing you to 'spy' on the graph with GraphEdit.
//
// To enable registration in this sample, define REGISTER_FILTERGRAPH.
//
#define REGISTER_FILTERGRAPH

// Constructor, initializes member variables
// and calls InitializeGraphBuilder
CBDAFilterGraph::CBDAFilterGraph() :
    m_fGraphBuilt(FALSE),
    m_fGraphRunning(FALSE),
	m_ulCarrierFrequency(12280000),      
	m_ulSymbolRate(22425),
	m_SignalPolarisation(0),
	m_ulBandwidth(6),
	m_ulQAM(64),
    m_KsTunerPropSet(NULL),
    m_KsDemodPropSet(NULL),
    m_pTunerPin(NULL),
    m_pDemodPin(NULL),
    m_dwGraphRegister(0)
{
	m_TunerFilterType = CLSID_DVBTNetworkProvider;

    if(FAILED(InitializeGraphBuilder()))
        m_fGraphFailure = TRUE;
    else
        m_fGraphFailure = FALSE;

	m_LNB_Data.DiSEqC_Port = 1;
	m_LNB_Data.LNB_POWER = 1;
	m_LNB_Data.ulLNBLOFLowBand = 11300;
	m_LNB_Data.ulLNBLOFHighBand = 11300;
	m_LNB_Data.ulLNBLOFHiLoSW = 0;
	m_LNB_Data.Tone_Data_Burst = 0;
	m_LNB_Data.f22K_Output = 0;
}


// Destructor
CBDAFilterGraph::~CBDAFilterGraph()
{
    StopGraph();

    if(m_fGraphBuilt || m_fGraphFailure)
    {
        TearDownGraph();
    }
}

void
CBDAFilterGraph::ReleaseInterfaces()
{
    OutputDebugString(TEXT("BDASample: CBDAFilterGraph::ReleaseInterfaces\n"));

    if (m_pITuningSpace)
    {
        m_pITuningSpace = NULL;
    }

    if (m_pTuningSpaceContainer)
    {
        m_pTuningSpaceContainer = NULL;
    }

    if (m_pITuner)
    {
        m_pITuner = NULL;
    }

    if (m_pFilterGraph)
    {
        m_pFilterGraph = NULL;
    }

    if (m_pIMediaControl)
    {
        m_pIMediaControl = NULL;
    }

    if (m_pVideoPin)
    {
        m_pVideoPin = NULL;
    }

    if (m_pAudioPin)
    {
        m_pAudioPin = NULL;
    }

	if (m_pITHPsiParser)
	{
		m_pITHPsiParser = NULL;
	}

     if ( m_KsDemodPropSet )
     {
        m_KsDemodPropSet = NULL;
     }

     if ( m_KsTunerPropSet )
     {
        m_KsTunerPropSet = NULL;
     }

     if ( m_pTunerPin )
     {
        m_pTunerPin = NULL;
     }

     if ( m_pDemodPin )
     {
        m_pDemodPin = NULL;
     }
}

// Instantiate graph object for filter graph building
HRESULT CBDAFilterGraph::InitializeGraphBuilder()
{
    HRESULT hr = S_OK;
    
    OutputDebugString(TEXT("BDASample: CBDAFilterGraph::InitializeGraphBuilder\n"));

    // we have a graph already
    if (m_pFilterGraph)
        return S_OK;

    // create the filter graph
    if (FAILED(hr = m_pFilterGraph.CoCreateInstance(CLSID_FilterGraph)))
    {
        OutputDebugString(TEXT("Couldn't CoCreate IGraphBuilder\n"));
        m_fGraphFailure = true;
        return hr;
    }
    
    return hr;
}

// BuildGraph sets up devices, adds and connects filters
HRESULT CBDAFilterGraph::BuildGraph(
    CComBSTR pNetworkType)   // the registered Tuning Space name
{
    HRESULT hr = E_FAIL;

    OutputDebugString(TEXT("BDASample: CBDAFilterGraph::BuildGraph\n"));

    // if we have already have a filter graph, tear it down
    if (m_fGraphBuilt) {
        if(m_fGraphRunning) {
            hr = StopGraph();
        }

        hr = TearDownGraph();
    }
	
    // Check tuner filter type
    if (FAILED(hr = CheckFilterType(KSCATEGORY_BDA_NETWORK_TUNER)))
    {
        OutputDebugString(TEXT("Check tuner device type fail\n"));
        //BuildGraphError();
        return hr;
    }

    // STEP 1: load network provider first so that it can configure other
    // filters, such as configuring the demux to sprout output pins.
    // We also need to submit a tune request to the Network Provider so it will
    // tune to a channel
    if (FAILED(hr = LoadNetworkProvider(pNetworkType)))
    {
        OutputDebugString(TEXT("Cannot load network provider\n"));
        BuildGraphError();
        return hr;
    }

    hr = m_pNetworkProvider->QueryInterface(__uuidof (ITuner), reinterpret_cast <void**> (&m_pITuner));
    if (FAILED(hr)) {
        OutputDebugString(TEXT("pNetworkProvider->QI: Can't QI for ITuner.\n"));
        BuildGraphError();
        return hr;
    }

	// create a tune request to initialize the network provider
	// before connecting other filters
	CComPtr <IDVBTuneRequest> pDVBTuneRequest;

	if (m_TunerFilterType == CLSID_DVBSNetworkProvider)
		hr = CreateDVBSTuneRequest(&pDVBTuneRequest);
	else if (m_TunerFilterType == CLSID_DVBTNetworkProvider)
		hr = CreateDVBTTuneRequest(&pDVBTuneRequest);
	else if (m_TunerFilterType == CLSID_DVBCNetworkProvider)
		hr = CreateDVBCTuneRequest(&pDVBTuneRequest);

	if (FAILED(hr))
	{
		OutputDebugString(TEXT("Cannot create tune request\n"));
		BuildGraphError();
		return hr;
	}

	//submit the tune request to the network provider
	hr = m_pITuner->put_TuneRequest(pDVBTuneRequest);
	if (FAILED(hr)) {
		OutputDebugString(TEXT("Cannot submit the tune request\n"));
		BuildGraphError();
		return hr;
	}

    // STEP2: Load tuner device and connect to network provider
    if (FAILED(hr = LoadFilter(KSCATEGORY_BDA_NETWORK_TUNER, 
                               &m_pTunerDemodDevice,
                               m_pNetworkProvider, 
                               TRUE)) || !m_pTunerDemodDevice)
	//if (FAILED(hr = LoadTunerFilter(KSCATEGORY_BDA_NETWORK_TUNER, 
     //                          m_pNetworkProvider, 
    //                           TRUE)))    
    {
        OutputDebugString(TEXT("Cannot load tuner device and connect network provider\n"));
        BuildGraphError();
        return hr;
    }
/*
    // Step3: Load capture device and connect to tuner/demod device
    if (FAILED(hr = LoadFilter(KSCATEGORY_BDA_RECEIVER_COMPONENT,
                               &m_pCaptureDevice,
                               m_pTunerDemodDevice, 
                               TRUE)))
    {
        OutputDebugString(TEXT("Cannot load capture device and connect tuner/demod\n"));
        BuildGraphError();
        return hr;
    }
*/
    
    //
    // this next call loads and connects filters associated with
    // the demultiplexor. if you want to manually load individual
    // filters such as audio and video decoders, use the code at
    // the bottom of this file
    //
#ifdef REGISTER_FILTERGRAPH
    hr = AddGraphToRot(m_pFilterGraph, &m_dwGraphRegister);
    if (FAILED(hr))
    {
      ///OutputDebugString(TEXT("Failed to register filter graph with ROT!  hr=0x%x"), hr);
        m_dwGraphRegister = 0;
    }
#endif

	// Step4: Load demux
    if (FAILED(hr = LoadDemux())) {
        OutputDebugString(TEXT("Cannot load demux\n"));
        BuildGraphError();
        return hr;
    }

    // Step5: Render demux pins
    //if (FAILED (hr = BuildAVSegment()))
    if (FAILED(hr = RenderDemux())) {
        OutputDebugString(TEXT("Cannot load demux\n"));
        BuildGraphError();
        return hr;
    }

    // Step6: get pointer to Tuner and Demod pins on Tuner/Demod filter
    // for setting the tuner and demod. property sets
    if (GetTunerDemodPropertySetInterfaces() == FALSE) {
        OutputDebugString(TEXT("Getting tuner/demod pin pointers failed\n"));
        BuildGraphError();
		return hr;
    }

	if (!THBDA_IOCTL_CHECK_INTERFACE_Fun()) {
        OutputDebugString(TEXT("THBDA_IOCTL_CHECK_INTERFACE_Fun failed\n"));
        BuildGraphError();
		return S_FALSE;
    }

	
	if (!THBDA_IOCTL_SET_LNB_DATA_Fun(&m_LNB_Data)) {
		OutputDebugString(TEXT("THBDA_IOCTL_SET_LNB_DATA_Fun failed\n"));
		BuildGraphError();
		return S_FALSE;
	}

    m_fGraphBuilt = true;
    m_fGraphFailure = false;
    
    return S_OK;
}


// Get the IPin addresses to the Tuner (Input0) and the
// Demodulator (MPEG2 Transport) pins on the Tuner/Demod filter.
// Then gets the IKsPropertySet interfaces for the pins.
//
BOOL CBDAFilterGraph::GetTunerDemodPropertySetInterfaces()
{
    if (!m_pTunerDemodDevice) {
        return FALSE;
    }

    m_pTunerPin = FindPinOnFilter(m_pTunerDemodDevice, "Input0");
    if (!m_pTunerPin) {
        OutputDebugString(TEXT("Cannot find Input0 pin on BDA Tuner/Demod filter!\n"));
        return FALSE;
    }

    m_pDemodPin = FindPinOnFilter(m_pTunerDemodDevice, "MPEG2 Transport");
    if (!m_pDemodPin) {
        OutputDebugString(TEXT("Cannot find MPEG2 Transport pin on BDA Tuner/Demod filter!\n"));
        return FALSE;
    }

    HRESULT hr = m_pTunerPin->QueryInterface(IID_IKsPropertySet,
                                   reinterpret_cast<void**>(&m_KsTunerPropSet));
    if (FAILED(hr)) {
        OutputDebugString(TEXT("QI of IKsPropertySet failed\n"));
        m_pTunerPin = NULL;
		return FALSE;
	}

    hr = m_pDemodPin->QueryInterface(IID_IKsPropertySet,
                                   reinterpret_cast<void**>(&m_KsDemodPropSet));
    if (FAILED(hr)) {
        OutputDebugString(TEXT("QI of IKsPropertySet failed\n"));
        m_pDemodPin = NULL;
		return FALSE;
	}

    return TRUE;
}

// Makes call to tear down the graph and sets graph failure
// flag to true
VOID
CBDAFilterGraph::BuildGraphError()
{
    OutputDebugString(TEXT("BDASample: CBDAFilterGraph::BuildGraphError\n"));

    TearDownGraph();
    m_fGraphFailure = true;
}


// This creates a new DVBS Tuning Space entry in the registry.
//
HRESULT CBDAFilterGraph::CreateDVBSTuningSpace(
    CComBSTR pNetworkType)
{
    HRESULT hr = S_OK;

    OutputDebugString(TEXT("BDASample: CBDAFilterGraph::CreateDVBSTuningSpace\n"));

    CComPtr <IDVBSTuningSpace> pIDVBTuningSpace;
    hr = pIDVBTuningSpace.CoCreateInstance(CLSID_DVBSTuningSpace);
    if (FAILED(hr) || !pIDVBTuningSpace) {
        OutputDebugString(TEXT("Failed to create system tuning space."));
        return FALSE;
    }

    // set the Frequency mapping
    WCHAR szFreqMapping[ 3 ]=L"-1";
    BSTR bstrFreqMapping = SysAllocString(szFreqMapping);
    if (bstrFreqMapping) {
        hr = pIDVBTuningSpace->put_FrequencyMapping(bstrFreqMapping); // not used
        SysFreeString(bstrFreqMapping);
    }

    // set the Friendly Name of the network type, shown as Name in the registry
    hr = pIDVBTuningSpace->put_UniqueName(pNetworkType);
    if (FAILED(hr)) {
        OutputDebugString(TEXT("BDASample: put_FriendlyName failed\n"));
        return hr;
    }

    // set the System type to terrestrial
    hr = pIDVBTuningSpace->put_SystemType(DVB_Satellite);
    if (FAILED(hr)) {
        OutputDebugString(TEXT("BDASample: put_SystemType failed\n"));
        return hr;
    }

    // set the Network Type to DVBS
	//CComBSTR clsid_dvbt = ("{216C62DF-6D7F-4e9a-8571-05F14EDB766A}");
    hr = pIDVBTuningSpace->put__NetworkType(CLSID_DVBSNetworkProvider);
    if (FAILED(hr)) {
        OutputDebugString(TEXT("BDASample: put_NetworkType failed\n"));
        return hr;
    }

    // set the Friendly Name, shown as Description in the registry
    WCHAR szFriendlyName[ 32 ]=L"Twinhan DVBS";
    BSTR bstrFriendlyName = SysAllocString(szFriendlyName);
    if (bstrFriendlyName) {
        hr = pIDVBTuningSpace->put_FriendlyName(bstrFriendlyName);
        SysFreeString(bstrFriendlyName);
    }

    // create DVBS Locator
    CComPtr <IDVBSLocator> pIDVBSLocator;
    hr = CoCreateInstance(CLSID_DVBSLocator,
                          NULL,
                          CLSCTX_INPROC_SERVER,
                          IID_IDVBSLocator,
                          reinterpret_cast<void**>(&pIDVBSLocator));
    if (FAILED(hr) || !pIDVBSLocator) {
        OutputDebugString(TEXT("BDASample: Failed to create system locator."));
        return hr;
    }

    // NOTE: The below parameter with the exception of
    //       setting the carrier frequency don't need to set.
    //       Thus they are set to -1.  The demodulator can
    //       handle these parameters without having to specifically
    //       set them in the hardware.

    // set the Carrier Frequency
    hr = pIDVBSLocator->put_CarrierFrequency(m_ulCarrierFrequency);
	//hr = pIDVBSLocator->put_SymbolRate(m_ulSymbolRate);
	//hr = pIDVBSLocator->put_SignalPolarisation(m_SignalPolarisation);
	
    
    // set the Bandwidth
    //hr = pIDVBSLocator->put_Bandwidth(-1);                            // not used 
    
    // set the Low Priority FEC type
    //hr = pIDVBSLocator->put_LPInnerFEC(BDA_FEC_METHOD_NOT_SET);       // not used
       
    // set the Low Priority code rate
    //hr = pIDVBSLocator->put_LPInnerFECRate(BDA_BCC_RATE_NOT_SET);     // not used
    
    // set the hieriarcy alpha
    //hr = pIDVBSLocator->put_HAlpha(BDA_HALPHA_NOT_SET);               // not used 
        
    // set the guard interval
    //hr = pIDVBSLocator->put_Guard(BDA_GUARD_NOT_SET);                 // not used 
        
    // set the transmission mode/FFT
    //hr = pIDVBSLocator->put_Mode(BDA_XMIT_MODE_NOT_SET);              // not used 
        
    // set whether the frequency is being used by another DVB-S broadcaster
    //hr = pIDVBSLocator->put_OtherFrequencyInUse(VARIANT_BOOL(FALSE)); // not used

    // set the inner FEC type
    hr = pIDVBSLocator->put_InnerFEC(BDA_FEC_METHOD_NOT_SET);         // not used
        
    // set the inner code rate
    hr = pIDVBSLocator->put_InnerFECRate(BDA_BCC_RATE_NOT_SET);       // not used 
        
    // set the outer FEC type
    hr = pIDVBSLocator->put_OuterFEC(BDA_FEC_METHOD_NOT_SET);         // not used 
        
    // set the outer code rate
    hr = pIDVBSLocator->put_OuterFECRate(BDA_BCC_RATE_NOT_SET);       // not used 
        
    // set the modulation type
    hr = pIDVBSLocator->put_Modulation(BDA_MOD_NOT_SET);              // not used 
       
    // set the symbol rate
    hr = pIDVBSLocator->put_SymbolRate(-1);                           // not used

    // set this a default locator
    hr = pIDVBTuningSpace->put_DefaultLocator(pIDVBSLocator);

    // create a tuning space container for the values we just set
    // so we can create in the registry.
    CComPtr <ITuningSpaceContainer> pTunerSpaceContainer;
    hr = CoCreateInstance(CLSID_SystemTuningSpaces,
                          NULL,
                          CLSCTX_INPROC_SERVER,
                          IID_ITuningSpaceContainer,
                          reinterpret_cast<void**>(&pTunerSpaceContainer));
    if (FAILED(hr) || !pTunerSpaceContainer) {
        OutputDebugString(TEXT("BDASample: Failed to create tuning space container"));
        return hr;
    }

    // get the number of tuning spaces that have been created
    LONG lCount;
    hr = pTunerSpaceContainer->get_Count(&lCount); 
    if (FAILED(hr)) {
        OutputDebugString(TEXT("BDASample: get_Count failed"));
        return hr;
    }

    // add the tuning spacing container values to the registry
    CComVariant varIndex(lCount+1);
    hr = pTunerSpaceContainer->Add(pIDVBTuningSpace, &varIndex);
    if (FAILED(hr)) {
        OutputDebugString(TEXT("BDASample: Failed to add new tuning space to registry"));
        return hr;
    }

    // create a copy of this tuning space for the 
    // class member variable.
    hr = pIDVBTuningSpace->Clone(&m_pITuningSpace);
    if (FAILED(hr)) {
        OutputDebugString(TEXT("BDASample: Clone failed"));
        m_pITuningSpace = NULL;
        return hr;
    }

    pIDVBTuningSpace     = NULL;
    pIDVBSLocator        = NULL;
	//pIBDA_FrequencyFilter  = NULL;
    pTunerSpaceContainer = NULL;

    return hr;
}

HRESULT CBDAFilterGraph::CreateDVBTTuningSpace(
    CComBSTR pNetworkType)
{
    HRESULT hr = S_OK;

    OutputDebugString(TEXT("BDASample: CBDAFilterGraph::CreateDVBTTuningSpace\n"));

    CComPtr <IDVBTuningSpace> pIDVBTuningSpace;
    hr = pIDVBTuningSpace.CoCreateInstance(CLSID_DVBTuningSpace);
    if (FAILED(hr) || !pIDVBTuningSpace) {
        OutputDebugString(TEXT("Failed to create system tuning space."));
        return FALSE;
    }

    // set the Frequency mapping
    WCHAR szFreqMapping[ 3 ]=L"-1";
    BSTR bstrFreqMapping = SysAllocString(szFreqMapping);
    if (bstrFreqMapping) {
        hr = pIDVBTuningSpace->put_FrequencyMapping(bstrFreqMapping); // not used
        SysFreeString(bstrFreqMapping);
    }

    // set the Friendly Name of the network type, shown as Name in the registry
    hr = pIDVBTuningSpace->put_UniqueName(pNetworkType);
    if (FAILED(hr)) {
        OutputDebugString(TEXT("BDASample: put_UniqueName failed\n"));
        return hr;
    }

    // set the System type to terrestrial
    hr = pIDVBTuningSpace->put_SystemType(DVB_Terrestrial);
    if (FAILED(hr)) {
        OutputDebugString(TEXT("BDASample: put_SystemType failed\n"));
        return hr;
    }

    // set the Network Type to DVBT
    hr = pIDVBTuningSpace->put__NetworkType(CLSID_DVBTNetworkProvider);
    if (FAILED(hr)) {
        OutputDebugString(TEXT("BDASample: put_NetworkType failed\n"));
        return hr;
    }

    // set the Friendly Name, shown as Description in the registry
    WCHAR szFriendlyName[ 32 ]=L"Twinhan DVBT";
    BSTR bstrFriendlyName = SysAllocString(szFriendlyName);
    if (bstrFriendlyName) {
        hr = pIDVBTuningSpace->put_FriendlyName(bstrFriendlyName);
        SysFreeString(bstrFriendlyName);
    }

    // create DVBT Locator
    CComPtr <IDVBTLocator> pIDVBTLocator;
    hr = CoCreateInstance(CLSID_DVBTLocator,
                          NULL,
                          CLSCTX_INPROC_SERVER,
                          IID_IDVBTLocator,
                          reinterpret_cast<void**>(&pIDVBTLocator));
    if (FAILED(hr) || !pIDVBTLocator) {
        OutputDebugString(TEXT("BDASample: Failed to create system locator."));
        return hr;
    }

    // NOTE: The below parameter with the exception of
    //       setting the carrier frequency don't need to set.
    //       Thus they are set to -1.  The demodulator can
    //       handle these parameters without having to specifically
    //       set them in the hardware.

    // set the Carrier Frequency
    hr = pIDVBTLocator->put_CarrierFrequency(m_ulCarrierFrequency);

    // set the Bandwidth
    hr = pIDVBTLocator->put_Bandwidth(-1);                            // not used 
    
    // set this a default locator
    hr = pIDVBTuningSpace->put_DefaultLocator(pIDVBTLocator);

    // create a tuning space container for the values we just set
    // so we can create in the registry.
    CComPtr <ITuningSpaceContainer> pTunerSpaceContainer;
    hr = CoCreateInstance(CLSID_SystemTuningSpaces,
                          NULL,
                          CLSCTX_INPROC_SERVER,
                          IID_ITuningSpaceContainer,
                          reinterpret_cast<void**>(&pTunerSpaceContainer));
    if (FAILED(hr) || !pTunerSpaceContainer) {
        OutputDebugString(TEXT("BDASample: Failed to create tuning space container"));
        return hr;
    }

    // get the number of tuning spaces that have been created
    LONG lCount;
    hr = pTunerSpaceContainer->get_Count(&lCount); 
    if (FAILED(hr)) {
        OutputDebugString(TEXT("BDASample: get_Count failed"));
        return hr;
    }

    // add the tuning spacing container values to the registry
    CComVariant varIndex(lCount+1);
    hr = pTunerSpaceContainer->Add(pIDVBTuningSpace, &varIndex);
    if (FAILED(hr)) {
        OutputDebugString(TEXT("BDASample: Failed to add new tuning space to registry"));
        return hr;
    }

    // create a copy of this tuning space for the 
    // class member variable.
    hr = pIDVBTuningSpace->Clone(&m_pITuningSpace);
    if (FAILED(hr)) {
        OutputDebugString(TEXT("BDASample: Clone failed"));
        m_pITuningSpace = NULL;
        return hr;
    }

    pIDVBTuningSpace     = NULL;
    pIDVBTLocator        = NULL;
	//pIBDA_FrequencyFilter  = NULL;
    pTunerSpaceContainer = NULL;

    return hr;
}

HRESULT CBDAFilterGraph::CreateDVBCTuningSpace(
    CComBSTR pNetworkType)
{
    HRESULT hr = S_OK;

    OutputDebugString(TEXT("BDASample: CBDAFilterGraph::CreateTuningSpace\n"));

    CComPtr <IDVBTuningSpace> pIDVBTuningSpace;
    hr = pIDVBTuningSpace.CoCreateInstance(CLSID_DVBTuningSpace);
    if (FAILED(hr) || !pIDVBTuningSpace) {
        OutputDebugString(TEXT("Failed to create system tuning space."));
        return FALSE;
    }

    // set the Frequency mapping
    WCHAR szFreqMapping[ 3 ]=L"-1";
    BSTR bstrFreqMapping = SysAllocString(szFreqMapping);
    if (bstrFreqMapping) {
        hr = pIDVBTuningSpace->put_FrequencyMapping(bstrFreqMapping); // not used
        SysFreeString(bstrFreqMapping);
    }

    // set the Friendly Name of the network type, shown as Name in the registry
    hr = pIDVBTuningSpace->put_UniqueName(pNetworkType);
    if (FAILED(hr)) {
        OutputDebugString(TEXT("BDASample: put_FriendlyName failed\n"));
        return hr;
    }

    // set the System type to terrestrial
    hr = pIDVBTuningSpace->put_SystemType(DVB_Cable);
    if (FAILED(hr)) {
        OutputDebugString(TEXT("BDASample: put_SystemType failed\n"));
        return hr;
    }

    // set the Network Type to DVBC
    hr = pIDVBTuningSpace->put__NetworkType(CLSID_DVBCNetworkProvider);
    if (FAILED(hr)) {
        OutputDebugString(TEXT("BDASample: put_NetworkType failed\n"));
        return hr;
    }

    // set the Friendly Name, shown as Description in the registry
    WCHAR szFriendlyName[ 32 ]=L"Twinhan DVBC";
    BSTR bstrFriendlyName = SysAllocString(szFriendlyName);
    if (bstrFriendlyName) {
        hr = pIDVBTuningSpace->put_FriendlyName(bstrFriendlyName);
        SysFreeString(bstrFriendlyName);
    }

    // create DVBC Locator
    CComPtr <IDVBCLocator> pIDVBCLocator;
    hr = CoCreateInstance(CLSID_DVBCLocator,
                          NULL,
                          CLSCTX_INPROC_SERVER,
                          IID_IDVBCLocator,
                          reinterpret_cast<void**>(&pIDVBCLocator));
    if (FAILED(hr) || !pIDVBCLocator) {
        OutputDebugString(TEXT("BDASample: Failed to create system locator."));
        return hr;
    }

    // NOTE: The below parameter with the exception of
    //       setting the carrier frequency don't need to set.
    //       Thus they are set to -1.  The demodulator can
    //       handle these parameters without having to specifically
    //       set them in the hardware.

    // set the Carrier Frequency
    hr = pIDVBCLocator->put_CarrierFrequency(m_ulCarrierFrequency);

    // set the Modulation
    hr = pIDVBCLocator->put_Modulation((ModulationType)0);
    
    // set this a default locator
    hr = pIDVBTuningSpace->put_DefaultLocator(pIDVBCLocator);

    // create a tuning space container for the values we just set
    // so we can create in the registry.
    CComPtr <ITuningSpaceContainer> pTunerSpaceContainer;
    hr = CoCreateInstance(CLSID_SystemTuningSpaces,
                          NULL,
                          CLSCTX_INPROC_SERVER,
                          IID_ITuningSpaceContainer,
                          reinterpret_cast<void**>(&pTunerSpaceContainer));
    if (FAILED(hr) || !pTunerSpaceContainer) {
        OutputDebugString(TEXT("BDASample: Failed to create tuning space container"));
        return hr;
    }

    // get the number of tuning spaces that have been created
    LONG lCount;
    hr = pTunerSpaceContainer->get_Count(&lCount); 
    if (FAILED(hr)) {
        OutputDebugString(TEXT("BDASample: get_Count failed"));
        return hr;
    }

    // add the tuning spacing container values to the registry
    CComVariant varIndex(lCount+1);
    hr = pTunerSpaceContainer->Add(pIDVBTuningSpace, &varIndex);
    if (FAILED(hr)) {
        OutputDebugString(TEXT("BDASample: Failed to add new tuning space to registry"));
        return hr;
    }

    // create a copy of this tuning space for the 
    // class member variable.
    hr = pIDVBTuningSpace->Clone(&m_pITuningSpace);
    if (FAILED(hr)) {
        OutputDebugString(TEXT("BDASample: Clone failed"));
        m_pITuningSpace = NULL;
        return hr;
    }

    pIDVBTuningSpace     = NULL;
    pIDVBCLocator        = NULL;
	//pIBDA_FrequencyFilter  = NULL;
    pTunerSpaceContainer = NULL;

    return hr;
}

// Loads the correct tuning space based on NETWORK_TYPE that got
// passed into BuildGraph()
HRESULT CBDAFilterGraph::LoadTuningSpace(
    CComBSTR pNetworkType)
{   
	HRESULT hr;
	ITuningSpace* pTuningSpace           = NULL;
    IEnumTuningSpaces* pEnumTuningSpaces = NULL;
    ITuningSpace** pTuningSpaceArray     = NULL;
    long lCount                          = 0;
    ULONG ulNumFetched = 0, i = 0;
    CComBSTR bstrTemp;

    OutputDebugString(TEXT("BDASample: CBDAFilterGraph::LoadTuningSpace\n"));

	// Get the tuning space collection
    hr = CoCreateInstance(CLSID_SystemTuningSpaces, NULL, 
                          CLSCTX_INPROC_SERVER, IID_ITuningSpaceContainer,
                          reinterpret_cast<void**>(&m_pTuningSpaceContainer));
    if (FAILED(hr)) {
        OutputDebugString(TEXT("Failed to create system tuning space."));
        return hr;
    }

	// Find the tuning space in the collection
    hr = m_pTuningSpaceContainer->get_EnumTuningSpaces(&pEnumTuningSpaces);
    if (FAILED(hr)) {
        OutputDebugString(TEXT("Failed to get tuning space enumerator."));
        return hr;
    }
    hr = m_pTuningSpaceContainer->get_Count(&lCount);
    if (FAILED(hr)) {
        OutputDebugString(TEXT("Failed to enumerate tuning spaces."));
        return hr;
    }

    // Read tuning space info into allocated array
    pTuningSpaceArray = new ITuningSpace*[lCount];
    hr = pEnumTuningSpaces->Next(lCount, pTuningSpaceArray, &ulNumFetched);
    if (FAILED(hr))  {
        OutputDebugString(TEXT("Failed to read tuning spaces."));
        return hr;
    }

    pEnumTuningSpaces = NULL;

    // Find the tuning space
    for (i = 0; i < ulNumFetched; i++) {
        hr = pTuningSpaceArray[i]->get_UniqueName(&bstrTemp);
        if (FAILED(hr)) {
            OutputDebugString(TEXT("Failed to read tuning space unique name."));
            return hr;
        }

        // Is this the correct tuning space?
        if (bstrTemp == pNetworkType) {
            OutputDebugString(TEXT("BDASample: Found the MYDVBT tuning space.\n"));
            hr = pTuningSpaceArray[i]->Clone(&pTuningSpace);
            break;
        }
    }

    pTuningSpaceArray = NULL;

    if (pTuningSpace == NULL) {
        OutputDebugString(TEXT("BDASample: Could not find MYDVBT tuning space.\n"));
        return E_FAIL;
    }

	// QI for IDVBTuningSpace
    hr = pTuningSpace->QueryInterface(IID_IDVBSTuningSpace, reinterpret_cast<void**>(&m_pITuningSpace));
    if (FAILED(hr)) {
        OutputDebugString(TEXT("Failed to QI for IDVBTuningSpace."));
        return hr;
    }

    pTuningSpace = NULL;

    delete pTuningSpaceArray;
    
    return hr;
}

HRESULT CBDAFilterGraph::LoadTuningSpace(GUID NetworkTypeGuid)
{   
	HRESULT hr;
	ITuningSpace* pTuningSpace           = NULL;
    IEnumTuningSpaces* pEnumTuningSpaces = NULL;
    ITuningSpace** pTuningSpaceArray     = NULL;
    long lCount                          = 0;
    ULONG ulNumFetched = 0, i = 0;
    //CComBSTR bstrTemp;
    GUID GUIDTemp;

    OutputDebugString(TEXT("BDASample: CBDAFilterGraph::LoadTuningSpace\n"));

	// Get the tuning space collection
    hr = CoCreateInstance(CLSID_SystemTuningSpaces, NULL, 
                          CLSCTX_INPROC_SERVER, IID_ITuningSpaceContainer,
                          reinterpret_cast<void**>(&m_pTuningSpaceContainer));
    if (FAILED(hr)) {
        OutputDebugString(TEXT("Failed to create system tuning space."));
        return hr;
    }

	// Find the tuning space in the collection
    hr = m_pTuningSpaceContainer->get_EnumTuningSpaces(&pEnumTuningSpaces);
    if (FAILED(hr)) {
        OutputDebugString(TEXT("Failed to get tuning space enumerator."));
        return hr;
    }
    hr = m_pTuningSpaceContainer->get_Count(&lCount);
    if (FAILED(hr)) {
        OutputDebugString(TEXT("Failed to enumerate tuning spaces."));
        return hr;
    }

    // Read tuning space info into allocated array
    pTuningSpaceArray = new ITuningSpace*[lCount];
    hr = pEnumTuningSpaces->Next(lCount, pTuningSpaceArray, &ulNumFetched);
    if (FAILED(hr))  {
        OutputDebugString(TEXT("Failed to read tuning spaces."));
        return hr;
    }

    pEnumTuningSpaces = NULL;

    // Find the tuning space
    for (i = 0; i < ulNumFetched; i++) {
        //hr = pTuningSpaceArray[i]->get_UniqueName(&bstrTemp);
        //if (FAILED(hr)) {
        //    ErrorMessageBox(TEXT("Failed to read tuning space unique name."));
        //    return hr;
        //}

        // Is this the correct tuning space?
        //if (bstrTemp == pNetworkType) {
        //    OutputDebugString(TEXT("BDASample: Found the MYDVBT tuning space.\n"));
        //    hr = pTuningSpaceArray[i]->Clone(&pTuningSpace);
        //    break;
        //}

        hr = pTuningSpaceArray[i]->get__NetworkType(&GUIDTemp);
        if (FAILED(hr)) {
            OutputDebugString(TEXT("Failed to read tuning space GUID."));
            return hr;
        }

        // Is this the correct tuning space?
        if (GUIDTemp == NetworkTypeGuid) {
            OutputDebugString(TEXT("Found the tuning space.\n"));
            hr = pTuningSpaceArray[i]->Clone(&pTuningSpace);
            break;
        }
    }

    if (pTuningSpaceArray) {
        delete pTuningSpaceArray;
        pTuningSpaceArray = NULL;
    }

    if (pTuningSpace == NULL) {
        OutputDebugString(TEXT("BDASample: Could not find MYDVBT tuning space.\n"));
        return E_FAIL;
    }

	// QI for IDVBTuningSpace
	if (m_TunerFilterType == CLSID_DVBSNetworkProvider)
		hr = pTuningSpace->QueryInterface(IID_IDVBSTuningSpace, reinterpret_cast<void**>(&m_pITuningSpace));
	else if (m_TunerFilterType == CLSID_DVBTNetworkProvider)
		hr = pTuningSpace->QueryInterface(IID_IDVBTuningSpace, reinterpret_cast<void**>(&m_pITuningSpace));
	else if (m_TunerFilterType == CLSID_DVBCNetworkProvider)
		hr = pTuningSpace->QueryInterface(IID_IDVBTuningSpace, reinterpret_cast<void**>(&m_pITuningSpace));

	if (FAILED(hr)) {
        OutputDebugString(TEXT("Failed to QI for IDVBTuningSpace."));
        return hr;
    }

    pTuningSpace = NULL;

    delete pTuningSpaceArray;
    
    return hr;
}


// Creates an DVBS Tune Request
HRESULT
CBDAFilterGraph::CreateDVBSTuneRequest(
    IDVBTuneRequest** pTuneRequest)
{
    HRESULT hr = S_OK;

    OutputDebugString(TEXT("BDASample: CBDAFilterGraph::CreateDVBSTuneRequest\n"));

    if (pTuneRequest == NULL)
    {
        OutputDebugString (TEXT("Invalid pointer\n"));
        return E_POINTER;
    }

    // Making sure we have a valid tuning space
    if (m_pITuningSpace == NULL)
    {
        OutputDebugString(TEXT("Tuning Space is NULL\n"));
        return E_FAIL;
    }

    //  Create an instance of the DVBS tuning space
    CComQIPtr <IDVBSTuningSpace> pDVBSTuningSpace (m_pITuningSpace);
    if (!pDVBSTuningSpace)
    {
        OutputDebugString(TEXT("Cannot QI for an IDVBSTuningSpace\n"));
        return E_FAIL;
    }

	hr = pDVBSTuningSpace->put_LowOscillator(m_LNB_Data.ulLNBLOFLowBand*1000);
    if (FAILED(hr))
    {
        OutputDebugString(TEXT("put_LowOscillator failed\n"));
        return hr;
    }

	hr = pDVBSTuningSpace->put_HighOscillator(m_LNB_Data.ulLNBLOFHighBand*1000);
    if (FAILED(hr))
    {
        OutputDebugString(TEXT("put_HighOscillator failed\n"));
        return hr;
    }

	hr = pDVBSTuningSpace->put_LNBSwitch(m_LNB_Data.ulLNBLOFHiLoSW*1000);
    if (FAILED(hr))
    {
        OutputDebugString(TEXT("put_LNBSwitch failed\n"));
        return hr;
    }

    //  Create an empty tune request.
    CComPtr <ITuneRequest> pNewTuneRequest;
    hr = pDVBSTuningSpace->CreateTuneRequest(&pNewTuneRequest);
    if (FAILED (hr))
    {
        OutputDebugString(TEXT("CreateTuneRequest: Can't create tune request.\n"));
        return hr;
    }

    //query for an IDVBTuneRequest interface pointer
    CComQIPtr <IDVBTuneRequest> pDVBSTuneRequest (pNewTuneRequest);
    if (!pDVBSTuneRequest)
    {
        OutputDebugString(TEXT("CreateDVBSTuneRequest: Can't QI for IDVBTuneRequest.\n"));
        return E_FAIL;
    }

    CComPtr <IDVBSLocator> pDVBSLocator;
    hr = pDVBSLocator.CoCreateInstance (CLSID_DVBSLocator);	
    if (FAILED(hr) || !pDVBSLocator) {
        OutputDebugString(TEXT("BDASample: Failed to create system locator."));
        return hr;
    }
    if (FAILED( hr))
    {
        OutputDebugString(TEXT("Cannot create the DVB-S locator failed\n"));
        return hr;
    }

    hr = pDVBSLocator->put_CarrierFrequency( m_ulCarrierFrequency );
    if (FAILED(hr))
    {
        OutputDebugString(TEXT("put_CarrierFrequency failed\n"));
        return hr;
    }
	hr = pDVBSLocator->put_SymbolRate(m_ulSymbolRate);
	
	if(m_SignalPolarisation)
	{
		hr= pDVBSLocator->put_SignalPolarisation(BDA_POLARISATION_LINEAR_H);
	}
	else
	{
		hr= pDVBSLocator->put_SignalPolarisation(BDA_POLARISATION_LINEAR_V);
	}

    hr = pDVBSTuneRequest->put_Locator (pDVBSLocator);
    if (FAILED (hr))
    {
        OutputDebugString(TEXT("Cannot put the locator\n"));
        return hr;
    }

    hr = pDVBSTuneRequest.QueryInterface (pTuneRequest);

    return hr;
}

// Creates an DVBT Tune Request
HRESULT
CBDAFilterGraph::CreateDVBTTuneRequest(
    IDVBTuneRequest** pTuneRequest)
{
    HRESULT hr = S_OK;

    OutputDebugString(TEXT("BDASample: CBDAFilterGraph::CreateDVBTTuneRequest\n"));

    if (pTuneRequest == NULL)
    {
        OutputDebugString (TEXT("Invalid pointer\n"));
        return E_POINTER;
    }

    // Making sure we have a valid tuning space
    if (m_pITuningSpace == NULL)
    {
        OutputDebugString(TEXT("Tuning Space is NULL\n"));
        return E_FAIL;
    }

    //  Create an instance of the DVBS tuning space
    CComQIPtr <IDVBTuningSpace> pDVBTTuningSpace (m_pITuningSpace);
    if (!pDVBTTuningSpace)
    {
        OutputDebugString(TEXT("Cannot QI for an IDVBTTuningSpace\n"));
        return E_FAIL;
    }

    //  Create an empty tune request.
    CComPtr <ITuneRequest> pNewTuneRequest;
    hr = pDVBTTuningSpace->CreateTuneRequest(&pNewTuneRequest);
    if (FAILED (hr))
    {
        OutputDebugString(TEXT("CreateTuneRequest: Can't create tune request.\n"));
        return hr;
    }

    //query for an IDVBTuneRequest interface pointer
    CComQIPtr <IDVBTuneRequest> pDVBTuneRequest (pNewTuneRequest);
    if (!pDVBTuneRequest)
    {
        OutputDebugString(TEXT("CreateDVBSTuneRequest: Can't QI for IDVBTuneRequest.\n"));
        return E_FAIL;
    }

    CComPtr <IDVBTLocator> pDVBTLocator;
    hr = pDVBTLocator.CoCreateInstance (CLSID_DVBTLocator);	
    if (FAILED(hr) || !pDVBTLocator) {
        OutputDebugString(TEXT("BDASample: Failed to create system locator."));
        return hr;
    }
    if (FAILED( hr))
    {
        OutputDebugString(TEXT("Cannot create the DVB-T locator failed\n"));
        return hr;
    }

    hr = pDVBTLocator->put_CarrierFrequency( m_ulCarrierFrequency );
    if (FAILED(hr))
    {
        OutputDebugString(TEXT("put_CarrierFrequency failed\n"));
        return hr;
    }
	hr = pDVBTLocator->put_Bandwidth(m_ulBandwidth);

    hr = pDVBTuneRequest->put_Locator (pDVBTLocator);
    if (FAILED (hr))
    {
        OutputDebugString(TEXT("Cannot put the locator\n"));
        return hr;
    }

    hr = pDVBTuneRequest.QueryInterface (pTuneRequest);

    return hr;
}

// Creates an DVBC Tune Request
HRESULT
CBDAFilterGraph::CreateDVBCTuneRequest(
    IDVBTuneRequest** pTuneRequest)
{
    HRESULT hr = S_OK;

    OutputDebugString(TEXT("BDASample: CBDAFilterGraph::CreateDVBTTuneRequest\n"));

    if (pTuneRequest == NULL)
    {
        OutputDebugString (TEXT("Invalid pointer\n"));
        return E_POINTER;
    }

    // Making sure we have a valid tuning space
    if (m_pITuningSpace == NULL)
    {
        OutputDebugString(TEXT("Tuning Space is NULL\n"));
        return E_FAIL;
    }

    //  Create an instance of the DVBS tuning space
    CComQIPtr <IDVBTuningSpace> pDVBTTuningSpace (m_pITuningSpace);
    if (!pDVBTTuningSpace)
    {
        OutputDebugString(TEXT("Cannot QI for an IDVBTTuningSpace\n"));
        return E_FAIL;
    }

    //  Create an empty tune request.
    CComPtr <ITuneRequest> pNewTuneRequest;
    hr = pDVBTTuningSpace->CreateTuneRequest(&pNewTuneRequest);
    if (FAILED (hr))
    {
        OutputDebugString(TEXT("CreateTuneRequest: Can't create tune request.\n"));
        return hr;
    }

    //query for an IDVBTuneRequest interface pointer
    CComQIPtr <IDVBTuneRequest> pDVBTuneRequest (pNewTuneRequest);
    if (!pDVBTuneRequest)
    {
        OutputDebugString(TEXT("CreateDVBSTuneRequest: Can't QI for IDVBTuneRequest.\n"));
        return E_FAIL;
    }

    CComPtr <IDVBCLocator> pDVBCLocator;
    hr = pDVBCLocator.CoCreateInstance (CLSID_DVBCLocator);	
    if (FAILED(hr) || !pDVBCLocator) {
        OutputDebugString(TEXT("BDASample: Failed to create system locator."));
        return hr;
    }
    if (FAILED( hr))
    {
        OutputDebugString(TEXT("Cannot create the DVB-T locator failed\n"));
        return hr;
    }

    hr = pDVBCLocator->put_CarrierFrequency( m_ulCarrierFrequency );
    if (FAILED(hr))
    {
        OutputDebugString(TEXT("put_CarrierFrequency failed\n"));
        return hr;
    }

	hr = pDVBCLocator->put_SymbolRate(m_ulSymbolRate);
    if (FAILED(hr))
    {
        OutputDebugString(TEXT("put_SymbolRate failed\n"));
        return hr;
    }

	hr = pDVBCLocator->put_Modulation((ModulationType)m_ulQAM);
    if (FAILED(hr))
    {
        OutputDebugString(TEXT("put_Modulation failed\n"));
        return hr;
    }

    hr = pDVBTuneRequest->put_Locator (pDVBCLocator);
    if (FAILED (hr))
    {
        OutputDebugString(TEXT("Cannot put the locator\n"));
        return hr;
    }

    hr = pDVBTuneRequest.QueryInterface (pTuneRequest);

    return hr;
}

/*
// Creates an DVBS Tuning Space
HRESULT
CBDAFilterGraph::CreateDVBSTuningSpace(
    IDVBSTuningSpace** pDVBSTuningSpace)
{
    HRESULT hr = S_OK;

    OutputDebugString(TEXT("BDASample: CBDAFilterGraph::CreateDVBSTuningSpace\n"));

    if (pDVBSTuningSpace == NULL)
    {
        OutputDebugString (TEXT("Invalid pointer\n"));
        return E_POINTER;
    }

    //  Create an empty tune request.
    CComPtr <ITuneRequest> pNewTuneRequest;
    hr = pDVBTTuningSpace->CreateTuneRequest(&pNewTuneRequest);
    if (FAILED (hr))
    {
        OutputDebugString(TEXT("CreateTuneRequest: Can't create tune request.\n"));
        return hr;
    }

    //query for an IDVBTuneRequest interface pointer
    CComQIPtr <IDVBTuneRequest> pDVBSTuneRequest (pNewTuneRequest);
    if (!pDVBSTuneRequest)
    {
        OutputDebugString(TEXT("CreateDVBSTuneRequest: Can't QI for IDVBTuneRequest.\n"));
        return E_FAIL;
    }

    CComPtr <IDVBSLocator> pDVBSLocator;
    hr = pDVBSLocator.CoCreateInstance (CLSID_DVBSLocator);	
    if (FAILED(hr) || !pDVBSLocator) {
        OutputDebugString(TEXT("BDASample: Failed to create system locator."));
        return hr;
    }
    if (FAILED( hr))
    {
        OutputDebugString(TEXT("Cannot create the DVB-S locator failed\n"));
        return hr;
    }

    hr = pDVBSLocator->put_CarrierFrequency( m_ulCarrierFrequency );
    if (FAILED(hr))
    {
        OutputDebugString(TEXT("put_CarrierFrequency failed\n"));
        return hr;
    }
	hr = pDVBSLocator->put_SymbolRate(m_ulSymbolRate);
	if(m_SignalPolarisation)
	{
		hr= pDVBSLocator->put_SignalPolarisation(BDA_POLARISATION_LINEAR_H);
	}
	else
	{
		hr= pDVBSLocator->put_SignalPolarisation(BDA_POLARISATION_LINEAR_V);
	}

    hr = pDVBSTuneRequest->put_Locator (pDVBSLocator);
    if (FAILED (hr))
    {
        OutputDebugString(TEXT("Cannot put the locator\n"));
        return hr;
    }

    hr = pDVBSTuneRequest.QueryInterface (pTuningSpace);

    return hr;
}
*/
// LoadNetworkProvider loads network provider
HRESULT CBDAFilterGraph::LoadNetworkProvider(
    CComBSTR pNetworkType)
{
    HRESULT  hr = S_OK;
    CComBSTR bstrNetworkType;
    CLSID    CLSIDNetworkType;

    OutputDebugString(TEXT("BDASample: CBDAFilterGraph::LoadNetworkProvider\n"));

    // obtain tuning space then load network provider
    if (m_pITuningSpace == NULL) {
        //hr = LoadTuningSpace(pNetworkType);
		hr = LoadTuningSpace(m_TunerFilterType);
        if (FAILED(hr)) {
            OutputDebugString(TEXT("BDASample: Cannot find MYDVB-S TuningSpace\n"));

            // so we create one.
			if (m_TunerFilterType == CLSID_DVBSNetworkProvider)
				hr = CreateDVBSTuningSpace("MYDVB-S");
			else if (m_TunerFilterType == CLSID_DVBTNetworkProvider)
				hr = CreateDVBTTuningSpace("MYDVB-T");
			else if (m_TunerFilterType == CLSID_DVBCNetworkProvider)
				hr = CreateDVBCTuningSpace("MYDVB-C");
            if (FAILED(hr)) {
                OutputDebugString(TEXT("Cannot load/create MYDVB-S tuning space\n"));
                return E_FAIL;
            }
        }
    }

    if (!m_pITuningSpace) {
        OutputDebugString(TEXT("Tuning space error!\n"));
        return E_FAIL;
    }

    // Get the current Network Type clsid
    hr = m_pITuningSpace->get_NetworkType(&bstrNetworkType);
    if (FAILED(hr)) {
        OutputDebugString(TEXT("ITuningSpace::Get Network Type failed\n"));
        return hr;
    }

    hr = CLSIDFromString(bstrNetworkType, &CLSIDNetworkType);
    if (FAILED(hr)) {
        OutputDebugString(TEXT("Couldn't get CLSIDFromString\n"));
        return hr;
    }

    // create the network provider based on the clsid obtained from the tuning space
    hr = CoCreateInstance(CLSIDNetworkType, NULL, CLSCTX_INPROC_SERVER,
                          IID_IBaseFilter, 
                          reinterpret_cast<void**>(&m_pNetworkProvider));
    if (FAILED(hr)) {
        OutputDebugString(TEXT("Couldn't CoCreate Network Provider\n"));
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
HRESULT CBDAFilterGraph::LoadFilter(
    REFCLSID clsid, 
    IBaseFilter** ppFilter,
    IBaseFilter* pConnectFilter, 
    BOOL fIsUpstream)
{
    HRESULT                hr = S_OK;
    BOOL                   fFoundFilter = FALSE;
    CComPtr <IMoniker>     pIMoniker;
    CComPtr <IEnumMoniker> pIEnumMoniker;

    OutputDebugString(TEXT("BDASample: CBDAFilterGraph::LoadFilter\n"));

    if (!m_pICreateDevEnum) {
        hr = m_pICreateDevEnum.CoCreateInstance(CLSID_SystemDeviceEnum);
        if (FAILED(hr)) {
            OutputDebugString(TEXT("LoadFilter(): Cannot CoCreate ICreateDevEnum"));
            return hr;
        }
    }

    // obtain the enumerator
    hr = m_pICreateDevEnum->CreateClassEnumerator(clsid, &pIEnumMoniker, 0);
    // the call can return S_FALSE if no moniker exists, so explicitly check S_OK
    if (FAILED(hr)) {
        OutputDebugString(TEXT("LoadFilter(): Cannot CreateClassEnumerator"));
        return hr;
    }
    if (S_OK != hr) { // Class not found
        OutputDebugString(TEXT("LoadFilter(): Class not found, CreateClassEnumerator returned S_FALSE"));
        return E_UNEXPECTED;
    }

    // next filter
    while (pIEnumMoniker->Next(1, &pIMoniker, 0) == S_OK)
    {
        // obtain filter's friendly name
        CComPtr <IPropertyBag> pBag;

        hr = pIMoniker->BindToStorage(NULL, 
                                      NULL, 
                                      IID_IPropertyBag,
                                      reinterpret_cast<void**>(&pBag));
        if (FAILED(hr)) {
            OutputDebugString (TEXT("BDASample: LoadFilter(): Cannot BindToStorage"));
            return hr;
        }

        CComVariant varBSTR;
        hr = pBag->Read(L"FriendlyName", &varBSTR, NULL);
        if (FAILED(hr)) {
            OutputDebugString (TEXT("BDASample: LoadFilter(): IPropertyBag->Read method failed"));
            pIMoniker = NULL;
            continue;
        }

        pBag = NULL;

        // bind the filter
        CComPtr <IBaseFilter> pFilter;

        hr = pIMoniker->BindToObject(NULL, 
                                     NULL, 
                                     IID_IBaseFilter,
                                     reinterpret_cast<void**>(&pFilter));

        if (FAILED(hr)) {
            pIMoniker = NULL;
            pFilter = NULL;
            continue;
        }

        hr = m_pFilterGraph->AddFilter (pFilter, varBSTR.bstrVal);
        if (FAILED(hr)) {
            OutputDebugString (TEXT("BDASample: Cannot add filter\n"));
            return hr;
        }

        //MessageBox (NULL, _T(""), _T(""), MB_OK);
        // test connections
        // to upstream filter
        if (pConnectFilter) {
            if (fIsUpstream) {
                hr = ConnectFilters(pConnectFilter, pFilter);
            }
            else {
                hr = ConnectFilters(pFilter, pConnectFilter);
            }

            if (SUCCEEDED(hr)) {
                //that's the filter we want
                fFoundFilter = TRUE;
                pFilter.QueryInterface(ppFilter);
                break;
            }
            else {
                fFoundFilter = FALSE;
                // that wasn't the the filter we wanted
                // so unload and try the next one
                hr = m_pFilterGraph->RemoveFilter(pFilter);
                if (FAILED(hr)) {
                    OutputDebugString(TEXT("BDASample: Failed unloading Filter\n"));
                    return hr;
                }
            }
        }
        else {
            fFoundFilter = TRUE;
            pFilter.QueryInterface(ppFilter);
            break;
        }

        pIMoniker = NULL;
        pFilter = NULL;

    } // while

    pIEnumMoniker = NULL;

    return S_OK;
}

// Loads the demux into the FilterGraph
HRESULT CBDAFilterGraph::LoadDemux()
{
    HRESULT hr = S_OK;
    
    hr = CoCreateInstance(CLSID_MPEG2Demultiplexer, 
                          NULL, 
                          CLSCTX_INPROC_SERVER,
                          IID_IBaseFilter, 
                          reinterpret_cast<void**>(&m_pDemux));
    if (FAILED(hr)) {
        OutputDebugString(TEXT("Could not CoCreateInstance CLSID_MPEG2Demultiplexer\n"));
        return hr;
    }

    hr = m_pFilterGraph->AddFilter(m_pDemux, L"Demux");
    if (FAILED(hr)) {
        OutputDebugString(TEXT("Unable to add demux filter to graph\n"));
        return hr;
    }

    return hr;
}

// Renders demux output pins.
HRESULT CBDAFilterGraph::RenderDemux()
{
    HRESULT             hr = S_OK;
    CComPtr <IPin>      pIPin;
    CComPtr <IPin>      pDownstreamPin;
    CComPtr <IEnumPins> pIEnumPins;
    PIN_DIRECTION       direction;

    OutputDebugString(TEXT("BDASample: CBDAFilterGraph::RenderDemux\n"));

    assert(m_pDemux);

    // connect the demux to the capture device
    //hr = ConnectFilters(m_pCaptureDevice, m_pDemux);
	hr = ConnectFilters(m_pTunerDemodDevice, m_pDemux);
    if (FAILED(hr)) {
        OutputDebugString(TEXT("Cannot connect demux to capture filter\n"));
        return hr;
    }

    // load transform information filter and connect it to the demux
    hr = LoadFilter(KSCATEGORY_BDA_TRANSPORT_INFORMATION, 
                    &m_pTIF, 
                    m_pDemux, 
                    TRUE);	
	if (FAILED(hr)) {
        OutputDebugString(TEXT("Cannot load TIF\n"));
        return hr;
    }

	hr = CoCreateInstance(CLSID_THPsiParser, NULL,
            CLSCTX_INPROC, IID_IBaseFilter,
            reinterpret_cast<void**>(&m_pTHPsiParser));
	if(SUCCEEDED(hr) && m_pTHPsiParser)
    {
        hr = m_pFilterGraph->AddFilter(m_pTHPsiParser, L"THPsiParser");
        if(FAILED(hr))
        {
            OutputDebugString(TEXT("Adding THPsiParser to the FilterGraph Failed\n"));
            return hr;
        }
    }
    else
    {
        OutputDebugString(TEXT("Loading THPsiParser Failed\n"));
        return hr;
    }

	hr = m_pTHPsiParser->QueryInterface(IID_ITHPsiParser, reinterpret_cast<void**>(&m_pITHPsiParser));
    if (FAILED(hr)) {
        OutputDebugString(TEXT("Failed to QI for m_pITHPsiParser."));
        return hr;
    }

    // render/connect the rest of the demux pins
    hr = m_pDemux->EnumPins(&pIEnumPins);
    if (FAILED(hr)) {
        OutputDebugString(TEXT("Cannot get the enumpins\n"));
        return hr;
    }

    while(pIEnumPins->Next(1, &pIPin, 0) == S_OK)
    {
        hr = pIPin->QueryDirection(&direction);
    
        if(direction == PINDIR_OUTPUT) {
            pIPin->ConnectedTo(&pDownstreamPin);
   
            if (pDownstreamPin == NULL) {
                m_pFilterGraph->Render(pIPin);
            }
   
           pDownstreamPin = NULL;
        }

        pIPin = NULL;
    }

    pIEnumPins = NULL;

    return hr;
}


// Removes each filter from the graph and un-registers
// the filter.
HRESULT
CBDAFilterGraph::TearDownGraph()
{
    HRESULT hr = S_OK;
    CComPtr <IBaseFilter> pFilter;
    CComPtr <IEnumFilters> pIFilterEnum;

    OutputDebugString(TEXT("BDASample: CBDAFilterGraph::TearDownGraph\n"));

    if(m_fGraphBuilt || m_fGraphFailure)
    {
        // unload manually added filters
        if ( m_pIPSink )
        {
            m_pFilterGraph->RemoveFilter(m_pIPSink);
            m_pIPSink = NULL;
        }

        if ( m_pTHPsiParser )
        {
            m_pFilterGraph->RemoveFilter(m_pTHPsiParser);
            m_pTHPsiParser = NULL;
        }

        if ( m_pTIF )
        {
            m_pFilterGraph->RemoveFilter(m_pTIF);
            m_pTIF = NULL;
        }

        if ( m_pDemux )
        {
            m_pFilterGraph->RemoveFilter(m_pDemux);
            m_pDemux = NULL;
        }

        if ( m_pNetworkProvider )
        {
            m_pFilterGraph->RemoveFilter(m_pNetworkProvider);
            m_pNetworkProvider = NULL;
        }

        if ( m_pTunerDemodDevice )
        {
            m_pFilterGraph->RemoveFilter(m_pTunerDemodDevice);
            m_pTunerDemodDevice = NULL;
        }

        if ( m_pCaptureDevice )
        {
            m_pFilterGraph->RemoveFilter(m_pCaptureDevice);
            m_pCaptureDevice = NULL;
        }

        // now go unload rendered filters
        hr = m_pFilterGraph->EnumFilters(&pIFilterEnum);

        if(FAILED(hr))
        {
            OutputDebugString(TEXT("TearDownGraph: cannot EnumFilters\n"));
            return E_FAIL;
        }

        pIFilterEnum->Reset();
                                
        while(pIFilterEnum->Next(1, &pFilter, 0) == S_OK) // addrefs filter
        {
            hr = m_pFilterGraph->RemoveFilter(pFilter);

            if (FAILED (hr))
                return hr;

            pIFilterEnum->Reset();
            pFilter = NULL;
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
    IBaseFilter* pFilterDownstream)
{
    HRESULT         hr = E_FAIL;

    CComPtr <IPin>  pIPinUpstream;

    PIN_INFO        PinInfoUpstream;
    PIN_INFO        PinInfoDownstream;

    OutputDebugString(TEXT("BDASample: CBDAFilterGraph::ConnectFilters\n"));
    
    // validate passed in filters
    assert(pFilterUpstream);

    assert(pFilterDownstream);

    // grab upstream filter's enumerator
    CComPtr <IEnumPins> pIEnumPinsUpstream;
    hr = pFilterUpstream->EnumPins(&pIEnumPinsUpstream);

    if(FAILED(hr))
    {
        OutputDebugString(TEXT("Cannot Enumerate Upstream Filter's Pins\n"));
        return hr;
    }

    // iterate through upstream filter's pins
    while (pIEnumPinsUpstream->Next (1, &pIPinUpstream, 0) == S_OK)
    {
        hr = pIPinUpstream->QueryPinInfo (&PinInfoUpstream);
        if(FAILED(hr))
        {
            OutputDebugString(TEXT("Cannot Obtain Upstream Filter's PIN_INFO\n"));
            return hr;
        }

        CComPtr <IPin> pPinDown;
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
                OutputDebugString(TEXT("Cannot enumerate pins on downstream filter!\n"));
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
                        OutputDebugString(TEXT("Failed in pIPinDownstream->ConnectedTo()!\n"));
                        continue;
                    }

                    if ((PINDIR_INPUT == PinInfoDownstream.dir) && (pPinUp == NULL))
                    {
                        assert(pIPinUpstream);
                        assert(pIPinDownstream);

                        if (SUCCEEDED (m_pFilterGraph->Connect(pIPinUpstream, pIPinDownstream)))
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

    OutputDebugString(TEXT("BDASample: CBDAFilterGraph::RunGraph\n"));

    if (m_pIMediaControl == NULL)
    {
        hr = m_pFilterGraph.QueryInterface (&m_pIMediaControl);
    }

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
            OutputDebugString(TEXT("Cannot run graph\n"));
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

    OutputDebugString(TEXT("BDASample: CBDAFilterGraph::StopGraph\n"));

    assert(m_pIMediaControl);

    // pause before stopping
    hr = m_pIMediaControl->Pause();

    // stop the graph
    hr = m_pIMediaControl->Stop();

    m_fGraphRunning = (FAILED (hr))?true:false;

    return hr;
}


#ifdef REGISTER_FILTERGRAPH

// Adds a DirectShow filter graph to the Running Object Table,
// allowing GraphEdit to "spy" on a remote filter graph.
HRESULT CBDAFilterGraph::AddGraphToRot(
    IUnknown *pUnkGraph, 
    DWORD *pdwRegister) 
{
    CComPtr <IMoniker>            pMoniker;
    CComPtr <IRunningObjectTable> pROT;
    WCHAR wsz[128];
    HRESULT hr;

    if (FAILED(GetRunningObjectTable(0, &pROT)))
        return E_FAIL;

    wsprintfW(wsz, L"FilterGraph %08x pid %08x\0", (DWORD_PTR) pUnkGraph, 
              GetCurrentProcessId());

    hr = CreateItemMoniker(L"!", wsz, &pMoniker);
    if (SUCCEEDED(hr)) 
        hr = pROT->Register(ROTFLAGS_REGISTRATIONKEEPSALIVE, pUnkGraph, 
                            pMoniker, pdwRegister);
        
    return hr;
}

// Removes a filter graph from the Running Object Table
void CBDAFilterGraph::RemoveGraphFromRot(
    DWORD pdwRegister)
{
    CComPtr <IRunningObjectTable> pROT;

    if (SUCCEEDED(GetRunningObjectTable(0, &pROT))) 
    {
        pROT->Revoke(pdwRegister);
    }

}

#endif

// Set the audio and video PIDs on the
// demux. filter.
HRESULT
CBDAFilterGraph::SetVideoAndAudioPIDs(
    LONG lNewVideoPID,
    LONG lNewAudioPID)
{
    HRESULT hr;

    //OutputDebugString(TEXT("BDASample: CBDAFilterGraph::SetVideoAndAudioPIDs\n"));

    if (!m_pVideoPin)
    {
        m_pVideoPin = FindPinOnFilter(m_pDemux, "2");
        if (!m_pVideoPin)
            return E_FAIL;
    }

    if (!m_pAudioPin)
    {
        m_pAudioPin = FindPinOnFilter(m_pDemux, "3");
        if (!m_pAudioPin)
            return E_FAIL;
    }

    if (!m_pIVideoPIDMap)
    {
        hr = m_pVideoPin->QueryInterface(IID_IMPEG2PIDMap, (void **) &m_pIVideoPIDMap);
        if (FAILED(hr))
        {
            MessagePrint("QI of IMPEGPIDMap on video pin failed\n");
            return E_FAIL;
        }

    }

    if (!m_pIAudioPIDMap)
    {
        hr = m_pAudioPin->QueryInterface(IID_IMPEG2PIDMap, (void **) &m_pIAudioPIDMap);
        if (FAILED(hr))
        {
            MessagePrint("QI of IMPEGPIDMap on audio pin failed\n");
            return E_FAIL;
        }
    }



    ULONG ulVideoPID[1] = {(ULONG) lNewVideoPID};
    ULONG ulAudioPID[1] = {(ULONG) lNewAudioPID};

    hr = m_pIVideoPIDMap->MapPID(1, ulVideoPID, MEDIA_ELEMENTARY_STREAM);
    if (FAILED(hr))
    {
        MessagePrint("mapping Video PID failed\n");
        return E_FAIL;
    }

    hr = m_pIAudioPIDMap->MapPID(1, ulAudioPID, MEDIA_ELEMENTARY_STREAM);
    if (FAILED(hr))
    {
        MessagePrint("mapping Audio PID failed\n");
        return E_FAIL;
    }

	MessagePrint("MapPID lNewVideoPID=%d, lNewAudioPID=%d", ulVideoPID[0], ulAudioPID[0]);


    m_pIVideoPIDMap = NULL;
    m_pIAudioPIDMap = NULL;
    
    return hr;
}

HRESULT
CBDAFilterGraph::ComposeCAPMT(int nCAPMT_ListMgt, int nCAPMT_CMD_ID, 
							    UINT PMT_PID, UINT SID,
								UINT* PIDAry, UINT nPIDNum, 
							    PBYTE pBuff_CAPMT, UINT *pbyBuffSize)
{
    HRESULT hr = 0;
	
	if (!m_pTHParserPin)
	{
        m_pTHParserPin = FindPinOnFilter(m_pDemux, "4");
        if (!m_pTHParserPin) {
			MessagePrint("BDASample: EnableCAM => Find m_pTHParserPin fail\n");
            return E_FAIL;
		}
    }

	if (!m_pITHParserPIDMap)
    {
        hr = m_pTHParserPin->QueryInterface(IID_IMPEG2PIDMap, (void **) &m_pITHParserPIDMap);
        if (FAILED(hr) || !m_pITHParserPIDMap)
        {
            MessagePrint("QI of IMPEGPIDMap on THParser pin failed\n");
            return E_FAIL;
        }
    }


	ULONG ulTHParserPID[1] = {(ULONG) PMT_PID};
	hr = m_pITHParserPIDMap->MapPID(1, ulTHParserPID, MEDIA_TRANSPORT_PAYLOAD);
    if (FAILED(hr))
    {
        MessagePrint("mapping THParser PID failed\n");
        return E_FAIL;
    }

	hr = m_pITHPsiParser->Set_SID(SID);
	if (FAILED(hr))
	{
		MessagePrint("Set_SID failed\n");
		return E_FAIL;
	}

	hr = m_pITHPsiParser->Set_CAPMT_List_Management(nCAPMT_ListMgt);
	if (FAILED(hr))
	{
		MessagePrint("Set_CAPMT_List_Management failed\n");
		return E_FAIL;
	}

	hr = m_pITHPsiParser->Set_ES_CAPMT_CMD_ID(nCAPMT_CMD_ID);
	if (FAILED(hr))
	{
		MessagePrint("Set_ES_CAPMT_CMD_ID failed\n");
		return E_FAIL;
	}

	hr = m_pITHPsiParser->SetCAPMT_2(PIDAry, nPIDNum, CAM_DEFAULT);
	if (FAILED(hr))
	{
		MessagePrint("SetCAPMT failed\n");
		return E_FAIL;
	}

	//Get PMT Data	
	BYTE nTry = 0;
	while ((*pbyBuffSize == 0) && (nTry < 3)) {
		nTry++;
		hr = m_pITHPsiParser->GetPMTData(pBuff_CAPMT, pbyBuffSize);
		if (FAILED(hr))
		{
			MessagePrint("GetPMTData failed\n");
			return E_FAIL;
		}	
	}
	MessagePrint("GetPMTData BuffSize=%d, (%x)  (%x)  (%x)", *pbyBuffSize, pBuff_CAPMT[0], pBuff_CAPMT[1], pBuff_CAPMT[2]);
		
	m_pITHParserPIDMap = NULL;
    return hr;
}

// Returns a pointer address of the name of a Pin on a 
// filter.
IPin*
CBDAFilterGraph::FindPinOnFilter(
    IBaseFilter* pBaseFilter,
    char* pPinName )
{
	HRESULT hr;
	IEnumPins *pEnumPin = NULL;
	ULONG CountReceived = 0;
	IPin *pPin = NULL, *pThePin = NULL;
	char String[80];
	char Mp2Str[] = {"MPEG2 Transport"};
	char* pString;
    PIN_INFO PinInfo;
	int length;

	// enumerate of pins on the filter 
	hr = pBaseFilter->EnumPins(&pEnumPin);
	if (hr == S_OK && pEnumPin)
	{
		pEnumPin->Reset();
		while (pEnumPin->Next( 1, &pPin, &CountReceived) == S_OK && pPin)
		{
			memset(String, NULL, sizeof(String));

			hr = pPin->QueryPinInfo(&PinInfo);
			if (hr == S_OK)
			{
				if (!strcmp(Mp2Str, pPinName)) {
					pThePin = pPin;	  // yes
					break;
				}

				length = wcslen (PinInfo.achName) + 1;
				pString = new char [length];
		
				// get the pin name 
				WideCharToMultiByte(CP_ACP, 0, PinInfo.achName, -1, pString, length,
									NULL, NULL);
		
				strcat (String, pString);

				// is there a match
				if (!strcmp(String, pPinName))
					pThePin = pPin;	  // yes
				else
					pPin = NULL;	  // no

				delete pString;

			}
			else
			{
				// need to release this pin
				pPin->Release();
			}


		}	// end if have pin

		// need to release the enumerator
		pEnumPin->Release();
	}

	// return address of pin if found on the filter
	return pThePin;

}


BOOL
CBDAFilterGraph::GetTunerStatus(BOOLEAN *pLocked, LONG *pQuality, LONG *pStrength)
{
	BOOLEAN bRet = false;
	HRESULT hr;
	BOOLEAN locked=false;
	LONG quality=0;
	LONG strength=0;

	CComQIPtr <IBDA_Topology> pBDATopology (m_pTunerDemodDevice);

	if (pBDATopology)
	{
		CComPtr <IUnknown> pControlNode;

		//if (SUCCEEDED(hr=pBDATopology->GetControlNode(0,1,0,&pControlNode)))
		if (SUCCEEDED(hr=pBDATopology->GetControlNode(0,1,0,&pControlNode)))
		{
			CComQIPtr <IBDA_SignalStatistics> pSignalStats (pControlNode);

			if (pSignalStats)
			{
				if (SUCCEEDED(hr=pSignalStats->get_SignalLocked(&locked)) &&
					SUCCEEDED(hr=pSignalStats->get_SignalQuality(&quality)) &&
					SUCCEEDED(hr=pSignalStats->get_SignalStrength(&strength)))
					bRet=true;
			}
		}
	}

	*pLocked = locked;
	*pQuality = quality;
	*pStrength = strength;

	return bRet;
}

BOOL CBDAFilterGraph::ChangeSetting(void)
{
	HRESULT hr = S_OK;

	OutputDebugString(TEXT("BDASample: ChangeSetting\n"));
	
	// create a tune request to initialize the network provider
    // before connecting other filters
    CComPtr <IDVBTuneRequest> pDVBTuneRequest;
    
	if (m_TunerFilterType == CLSID_DVBSNetworkProvider)
		hr = CreateDVBSTuneRequest(&pDVBTuneRequest);
	else if (m_TunerFilterType == CLSID_DVBTNetworkProvider)
		hr = CreateDVBTTuneRequest(&pDVBTuneRequest);
	else if (m_TunerFilterType == CLSID_DVBCNetworkProvider)
		hr = CreateDVBCTuneRequest(&pDVBTuneRequest);

	if (FAILED(hr))
	{
		OutputDebugString(TEXT("Cannot create tune request\n"));
		BuildGraphError();
		return hr;
	}

    //submit the tune request to the network provider
    hr = m_pITuner->put_TuneRequest(pDVBTuneRequest);
    if (FAILED(hr)) {
        OutputDebugString(TEXT("Cannot submit the tune request\n"));
        BuildGraphError();
        return hr;
    }

	if (!THBDA_IOCTL_SET_LNB_DATA_Fun(&m_LNB_Data)) {
		OutputDebugString(TEXT("THBDA_IOCTL_SET_LNB_DATA_Fun failed\n"));
		BuildGraphError();
		return S_FALSE;
	}

	OutputDebugString(TEXT("BDASample: ChangeSetting OK\n"));
	return hr;
}


VOID MessagePrint(LPTSTR szFormat, ...)
{
    static TCHAR szBuffer[2048];  // Large buffer
    const size_t NUMCHARS = sizeof(szBuffer) / sizeof(szBuffer[0]);
    const int LASTCHAR = NUMCHARS - 1;

    // Format the input string
    va_list pArgs;
    va_start(pArgs, szFormat);

    // Use a bounded buffer size to prevent buffer overruns.  Limit count to
    // character size minus one to allow for a NULL terminating character.
    _vsntprintf(szBuffer, NUMCHARS - 1, szFormat, pArgs);
    va_end(pArgs);

    // Ensure that the formatted string is NULL-terminated
    szBuffer[LASTCHAR] = TEXT('\0');

    // Display a message box with the formatted string
    OutputDebugString(szBuffer);
}

HRESULT CBDAFilterGraph::CheckFilterType(REFCLSID clsid)
{
    HRESULT                hr = S_OK;
    //BOOL                   fFoundFilter = FALSE;
    CComPtr <IMoniker>     pIMoniker;
    CComPtr <IEnumMoniker> pIEnumMoniker;
    CComPtr <IBaseFilter>  pTunerFilter; // for tuner device filter

    OutputDebugString(TEXT("BDASample: CBDAFilterGraph::CheckFilterType\n"));

    if (!m_pICreateDevEnum) {
        hr = m_pICreateDevEnum.CoCreateInstance(CLSID_SystemDeviceEnum);
        if (FAILED(hr)) {
            OutputDebugString(TEXT("LoadFilter(): Cannot CoCreate ICreateDevEnum"));
            return hr;
        }
    }

    // obtain the enumerator
    hr = m_pICreateDevEnum->CreateClassEnumerator(clsid, &pIEnumMoniker, 0);
    // the call can return S_FALSE if no moniker exists, so explicitly check S_OK
    if (FAILED(hr)) {
        OutputDebugString(TEXT("LoadFilter(): Cannot CreateClassEnumerator"));
        return hr;
    }
    if (S_OK != hr) { // Class not found
        //OutputDebugString(TEXT("LoadFilter(): Class not found, CreateClassEnumerator returned S_FALSE"));
        OutputDebugString(TEXT("Cannot find TwinHan BDA Tuner filter"));
        return E_UNEXPECTED;
    }

    // next filter
    while (pIEnumMoniker->Next(1, &pIMoniker, 0) == S_OK)
    {
        // obtain filter's friendly name
        CComPtr <IPropertyBag> pBag;

        hr = pIMoniker->BindToStorage(NULL, 
                                      NULL, 
                                      IID_IPropertyBag,
                                      reinterpret_cast<void**>(&pBag));
        if (FAILED(hr)) {
            OutputDebugString (TEXT("BDASample: LoadFilter(): Cannot BindToStorage"));
            return hr;
        }

        CComVariant varBSTR;
        hr = pBag->Read(L"FriendlyName", &varBSTR, NULL);
        if (FAILED(hr)) {
            OutputDebugString (TEXT("BDASample: LoadFilter(): IPropertyBag->Read method failed"));
            pIMoniker = NULL;
            continue;
        }

        pBag = NULL;

        // bind the filter
        CComPtr <IBaseFilter> pFilter;

        hr = pIMoniker->BindToObject(NULL, 
                                     NULL, 
                                     IID_IBaseFilter,
                                     reinterpret_cast<void**>(&pFilter));

        if (FAILED(hr)) {
            pIMoniker = NULL;
            pFilter = NULL;
            continue;
        }

        hr = m_pFilterGraph->AddFilter (pFilter, varBSTR.bstrVal);
        if (FAILED(hr)) {
            OutputDebugString (TEXT("BDASample: Cannot add filter\n"));
            return hr;
        }

        CComPtr <IBDA_Topology> pIBDATopology;
        //pTunerFilter->QueryInterface(__uuidof (IBDA_Topology), (void **) &pIBDATopology);
        pFilter->QueryInterface(__uuidof (IBDA_Topology), (void **) &pIBDATopology);
        ULONG ulcNodeDescriptors;
        BDANODE_DESCRIPTOR rgNodeDescriptors[10];
        pIBDATopology->GetNodeDescriptors(&ulcNodeDescriptors, 10, rgNodeDescriptors);

        for (ULONG i=0; i<ulcNodeDescriptors; i++) {
            if (rgNodeDescriptors[i].guidFunction == KSNODE_BDA_COFDM_DEMODULATOR) {
                m_TunerFilterType = CLSID_DVBTNetworkProvider;
                break;
            }
            else if (rgNodeDescriptors[i].guidFunction == KSNODE_BDA_QPSK_DEMODULATOR) {
                m_TunerFilterType = CLSID_DVBSNetworkProvider;
                break;
            }
            else if (rgNodeDescriptors[i].guidFunction == KSNODE_BDA_QAM_DEMODULATOR) {
                m_TunerFilterType = CLSID_DVBCNetworkProvider;
                break;
            }
            else if (rgNodeDescriptors[i].guidFunction == KSNODE_BDA_8VSB_DEMODULATOR) {
                m_TunerFilterType = CLSID_ATSCNetworkProvider;
                break;
            }
        }

        if (pIBDATopology)
            pIBDATopology.Release();

        if (pTunerFilter)
            pTunerFilter.Release();

        hr = m_pFilterGraph->RemoveFilter(pFilter);
        if (FAILED(hr)) {
            OutputDebugString(TEXT("BDASample: Failed unloading Filter\n"));
            return hr;
        }

        if (pFilter)
            pFilter.Release();

        pIMoniker = NULL;
        pFilter = NULL;

        break;
    } // while

    pIEnumMoniker = NULL;

    return S_OK;
}
