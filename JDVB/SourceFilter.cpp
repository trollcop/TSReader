#include "SourceFilter.h"
#include "TSPacket.h"
#include "TSPacketQueue.h"
#include "TSPidFilter.h"
#include <streams.h>




//--------------------------------------------------------------------------------

/*********************************************************************************
 * CSourceFilter
 *
 ********************************************************************************/
CSourceFilter::CSourceFilter(IUnknown *pUnk, HRESULT *phr) :
CSource(NAME("SourceFilter"),pUnk,CLSID_SourceFilter)
{
	HRESULT hr;
	m_pPin = new CSourceStreamPin(&hr,this,L"Stream 1");


}
//--------------------------------------------------------------------------------

CSourceFilter::~CSourceFilter()
{
	delete m_pPin;

}

//--------------------------------------------------------------------------------
	//public constructor
CUnknown * WINAPI CSourceFilter::CreateInstance(IUnknown *pUnk, HRESULT *phr)
{
	CSourceFilter *pNewFilter = new CSourceFilter(pUnk, phr );

	if (phr)
	{
		if (pNewFilter == NULL) 
			*phr = E_OUTOFMEMORY;
		else
			*phr = S_OK;
	}
    return pNewFilter;
}

//--------------------------------------------------------------------------------

HRESULT CSourceFilter::processPid(TSPacket *Packet)
{
	//FIXME
	return m_pPin->insertSample (Packet);
}

//--------------------------------------------------------------------------------
HRESULT CSourceFilter::NonDelegatingQueryInterface(REFIID riid, void **ppv)
{
	if(riid == IID_SOURCE_FILTER)
		return GetInterface((ISourceFilter *) this, ppv);
	else
		return CSource::NonDelegatingQueryInterface (riid,ppv);


}
//---------------------------------------------------------------------------------
STDMETHODIMP CSourceFilter::FindPin(const wchar_t * PinName,IPin ** Pin)
{
	//FIXME
	if(wcscmp(PinName, L"Stream 1") == 0)
	{
		*Pin = m_pPin;
		m_pPin->AddRef ();
		return NOERROR;
	}

	return E_FAIL;
}

//----------------------------------------------------------------------------------
HRESULT CSourceFilter::setTSPacketQueue(TSPacketQueue *Queue)
{
	return m_pPin->setTSPacketQueue (Queue);
}
//----------------------------------------------------------------------------------
HRESULT CSourceFilter::setNextTSPidFilter(TSPidFilter *Next)
{
	return m_pPin->setNextTSPidFilter (Next);
}
