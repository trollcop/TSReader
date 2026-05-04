/*
Copyright (c) David R. Cattley (dcattley@msn.com). All rights reserved.
Portions Copyright (c) 2000-2002, Microsoft Corporation.  All rights reserved.

Module Name:

    BDAFilterGraph.h

Abstract:

	ATSC BDA Source for TSReader (www.coolstf.com) with portions derived from
	the DirectX SDK BDA Sample graph.h

Author:

    David R. Cattley (dcattley@msn.com)

Revision History:

	01-Feb-2005 - Created
*/
#pragma	once

//-----------------------------------------------------------------------------
// index of tuning spaces 
enum NETWORK_TYPE 
{
	NT_ATSC = 0x0003,
	NT_DVBT = 0x0008,
	NT_DVBC = 0x0009
};

class CBDAFilterGraph
{
private:
    CComPtr <ITuningSpace>   m_pITuningSpace;

    CComPtr <IScanningTuner> m_pITuner;

    CComPtr <IGraphBuilder>  m_pFilterGraph;         // for current graph
    CComPtr <IMediaControl>  m_pIMediaControl;       // for controlling graph state
    CComPtr <ICreateDevEnum> m_pICreateDevEnum;      // for enumerating system devices

    CComPtr <IBaseFilter>    m_pNetworkProvider;     // for network provider filter
    CComPtr <IBaseFilter>    m_pTunerDevice;         // for tuner device filter
    CComPtr <IBaseFilter>    m_pCaptureDevice;       // for capture device filter
	CComPtr <IBaseFilter>    m_pSampleGrabber;       // for monitoring Transport Stream
    CComPtr <IBaseFilter>    m_pDemux;               // for demux filter
    CComPtr <IBaseFilter>    m_pTIF;                 // for transport information filter

    //required for an ATSC network when creating a tune request
    LONG                     m_lMajorChannel;
    LONG                     m_lMinorChannel;
    LONG                     m_lPhysicalChannel;

    //registration number for the RunningObjectTable
    DWORD                    m_dwGraphRegister;

    NETWORK_TYPE             m_NetworkType;

    HRESULT InitializeGraphBuilder();
    HRESULT LoadTuningSpace();
    HRESULT LoadNetworkProvider();
	HRESULT LoadSampleGrabber();
    HRESULT LoadDemux();
    HRESULT RenderDemux();

    HRESULT LoadFilter(
        REFCLSID clsid, 
        IBaseFilter** ppFilter,
        IBaseFilter* pConnectFilter, 
        BOOL fIsUpstream
        );

    HRESULT ConnectFilters(
        IBaseFilter* pFilterUpstream, 
        IBaseFilter* pFilterDownstream
        );

    HRESULT CreateATSCTuneRequest(
        LONG lPhysicalChannel,
        LONG lMajorChannel, 
        LONG lMinorChannel,
        IATSCChannelTuneRequest**   pTuneRequest
        );

public:
	CComPtr <IBaseFilter>	 m_tvTuner;

    bool            m_fGraphBuilt;
    bool            m_fGraphRunning;
    bool            m_fGraphFailure;

    CBDAFilterGraph();   
    ~CBDAFilterGraph();

    HRESULT BuildGraph(
        NETWORK_TYPE NetworkType
        );

    HRESULT RunGraph();
    HRESULT StopGraph();
    HRESULT TearDownGraph();

	template<class Q>
	HRESULT GetSampleGrabber(Q** ppSampleGrabber)
	{
		if (!m_pSampleGrabber)
			return E_FAIL;

		return m_pSampleGrabber.QueryInterface(ppSampleGrabber);
	}

	template<class Q>
	HRESULT 
	GetControlNode(Q** ppControlNode)
	{
		CComQIPtr<IBDA_Topology> pBDATopology(m_pTunerDevice);

		if (!pBDATopology)
			return E_FAIL;
        
		CComPtr<IUnknown> pUnk;
		HRESULT hr = pBDATopology->GetControlNode(0,1,0,&pUnk);

		if (SUCCEEDED(hr))
		{
			hr = pUnk.QueryInterface(ppControlNode);
		}

		return hr;
	}

    HRESULT ChangeChannel(
        LONG lPhysicalChannel,
        LONG lMajorChannel, 
        LONG lMinorChannel
        );
    

	// Adds/removes a DirectShow filter graph from the Running Object Table,
    // allowing GraphEdit to "spy" on a remote filter graph if enabled.
    HRESULT AddGraphToRot(
        IUnknown *pUnkGraph, 
        DWORD *pdwRegister
        );

    void RemoveGraphFromRot(
        DWORD pdwRegister
        );

    LONG GetMajorChannel()		{ return m_lMajorChannel;    };
    LONG GetPhysicalChannel()	{ return m_lPhysicalChannel; };
    LONG GetMinorChannel()		{ return m_lMinorChannel;    };

	BOOL DevFilterProperty(IBaseFilter* pFilter,REFGUID pGuid,int fGetSet,int nIndex, PBYTE pInBuffer, int nInSize, PBYTE pOutBuffer, int nOutSize);
	HRESULT TunerIface(DWORD fGetSet,DWORD fCommand,DWORD* pdwValue);
	HRESULT CBDAFilterGraph::ScanSignal(int lPhyCh, LONG * plSignal);
	BOOL CBDAFilterGraph::LoadTuner();
	IBaseFilter *CBDAFilterGraph::BindFilterToObject(REFCLSID clsid, char *filtername);
	DWORD CBDAFilterGraph::GetQAMSNR();
};
