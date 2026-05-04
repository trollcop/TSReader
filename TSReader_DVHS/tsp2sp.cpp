#include <windows.h>
#include <streams.h>
#include <initguid.h>
#include <olectl.h>
#if (1100 > _MSC_VER)
#include <olectlid.h>
#endif
#include <pullpin.h>
#include "TSP2SP.h"

#pragma warning(disable:4238)  // nonstd extension used: class rvalue used as lvalue

// setup data

const AMOVIESETUP_MEDIATYPE sudPinTypes =
{
    &MEDIATYPE_NULL,       // Major type
    &MEDIASUBTYPE_NULL      // Minor type
};

const AMOVIESETUP_PIN psudPins[] =
{
    {
        L"Input",           // String pin name
        FALSE,              // Is it rendered
        FALSE,              // Is it an output
        FALSE,              // Allowed none
        FALSE,              // Allowed many
        &CLSID_NULL,        // Connects to filter
        L"Output",          // Connects to pin
        1,                  // Number of types
        &sudPinTypes },     // The pin details
      { L"Output",          // String pin name
        FALSE,              // Is it rendered
        TRUE,               // Is it an output
        FALSE,              // Allowed none
        FALSE,              // Allowed many
        &CLSID_NULL,        // Connects to filter
        L"Input",           // Connects to pin
        1,                  // Number of types
        &sudPinTypes        // The pin details
    }
};


const AMOVIESETUP_FILTER sudTSP2SP =
{
    &CLSID_TSP2SP,        // Filter CLSID
    L"MPEG2 Transport to Source Packets",      // Filter name
    MERIT_DO_NOT_USE,       // Its merit
    2,                      // Number of pins
    psudPins                // Pin details
};


// List of class IDs and creator functions for the class factory. This
// provides the link between the OLE entry point in the DLL and an object
// being created. The class factory will call the static CreateInstance

CFactoryTemplate g_Templates[1] = {

    { L"Video TSP2SP"
    , &CLSID_TSP2SP
    , CTSP2SP::CreateInstance
    , NULL
    , &sudTSP2SP }

};
int g_cTemplates = sizeof(g_Templates) / sizeof(g_Templates[0]);


//
// Constructor
//
CTSP2SP::CTSP2SP(TCHAR *tszName,LPUNKNOWN punk,HRESULT *phr) :
    CTransformFilter(tszName, punk, CLSID_TSP2SP)
{
    ASSERT(tszName);
    ASSERT(phr);

	m_lastCount = 3000;
	m_lastOffset = 0;
	m_lastCountDelta = 0;
	m_lastOffsetDelta = 0;
	m_CountDelta = 0;
	m_OffsetDelta = 0;
	m_lastClock = 0;
	m_packets = 0;
	m_fValidStamp = false;
	
	*phr = S_OK;
} // TSP2SP


//
// CreateInstance
//
// Provide the way for COM to create a CTSP2SP object
//
CUnknown * WINAPI CTSP2SP::CreateInstance(LPUNKNOWN punk, HRESULT *phr) {

    CTSP2SP *pNewObject = new CTSP2SP(NAME("TSP2SP"), punk, phr);
    if (pNewObject == NULL) {
        *phr = E_OUTOFMEMORY;
    }
    return pNewObject;

} // CreateInstance


//
// NonDelegatingQueryInterface
//
// Reveals ITSP2SP and ISpecifyPropertyPages
//
STDMETHODIMP CTSP2SP::NonDelegatingQueryInterface(REFIID riid, void **ppv)
{
    CheckPointer(ppv,E_POINTER);
    return CTransformFilter::NonDelegatingQueryInterface(riid, ppv);

} // NonDelegatingQueryInterface

HRESULT CTSP2SP::CheckTransform(const CMediaType *mtIn,const CMediaType *mtOut)
{
	return S_OK;
}

HRESULT CTSP2SP::Transform(IMediaSample *pSource, IMediaSample *pDest)
{
    HRESULT hr = S_OK;

	int i,j;
	DWORD dwCount;
	DWORD dwOffset;
	long newdatasize = 0;
	MPEG2_TRANSPORT * pTsp = NULL;
	MPEG2_TRANSPORT_STRIDE * pStride = NULL;
    BYTE *pSourceBuffer, *pDestBuffer, *pBuff;
    long lSourceSize = pSource->GetActualDataLength();
	long lSampleCount = lSourceSize/sizeof(MPEG2_TRANSPORT);
#ifdef DEBUG
	long lDestSize	= pDest->GetSize();
	ASSERT(lDestSize == sizeof(MPEG2_TRANSPORT_STRIDE) * 256);
#endif
	pSource->GetPointer(&pSourceBuffer);
    pDest->GetPointer(&pDestBuffer);
	pTsp = (MPEG2_TRANSPORT *)pSourceBuffer;
	pStride = (MPEG2_TRANSPORT_STRIDE *)pDestBuffer;
	ASSERT(pTsp);
	ASSERT(pDest);

	for(i = 0, j = 0; i < lSampleCount; i++,m_packets++)
	{
		ASSERT ( pTsp[i].pTransportHeader[0] == 0x47);
		if((pTsp[i].pTransportHeader[3] & 0x20) // Adaptation field present
			&& (pTsp[i].AdaptationField.pAdaptationLength > 0) // length > 0
			&& (pTsp[i].AdaptationField.pAdaptationFlags & 0x10)) // and a PCR
		{
			DWORD m_lastCountDelta = m_CountDelta;
			DWORD m_lastOffsetDelta = m_OffsetDelta;
			__int64 clock = (pTsp[i].AdaptationField.pPCR[0] << 25);
			clock += (pTsp[i].AdaptationField.pPCR[1] << 17);
			clock += (pTsp[i].AdaptationField.pPCR[2] << 9);
			clock += (pTsp[i].AdaptationField.pPCR[3] << 1);
			clock += ((pTsp[i].AdaptationField.pPCR[4] & 0x80) >> 7);
			clock *= 300;
			clock += ((pTsp[i].AdaptationField.pPCR[4] & 0x1) << 8);
			clock += (pTsp[i].AdaptationField.pPCR[5]);
			if(m_lastClock != 0)
			{ // we have a clock difference, time to update packets
				__int64 delta = clock - m_lastClock;
				__int64 cn = ((delta * 8000) / 27000000);
				m_CountDelta = (DWORD)cn;
				__int64 tn = delta - ((cn * 27000000)/8000);
				__int64 on = ((tn * 24576000) / 27000000);
				m_OffsetDelta = (DWORD)on;
				DWORD dcpp = m_CountDelta / m_packets;
				DWORD dopp = (((m_CountDelta % m_packets) * 3072) + m_OffsetDelta) / m_packets;
				if(m_fValidStamp && ( abs(dcpp-m_lastCountDelta) > 450 )) // if change in delivery rate is too large
				{
					pTsp[i].pTransportHeader[1] |= 0x80; // Set Transport error flag
					m_CountDelta = m_lastCountDelta;     // Keep old rate
					m_OffsetDelta = m_lastOffsetDelta;
				}
				else
				{
					m_CountDelta = dcpp;
					m_OffsetDelta = dopp;
				}
				m_fValidStamp = true;
			}
			m_lastClock = clock;
			m_packets = 0;
		}
		if(m_fValidStamp)
		{
			m_lastOffset += m_OffsetDelta;
			if(m_lastOffset >= 3072)
			{
				m_lastOffset -= 3072;
				m_lastCount ++;
			}
			m_lastCount += m_CountDelta;
			if(m_lastCount >= 8000)
				m_lastCount -= 8000;
			dwCount = m_lastCount;
			dwOffset = m_lastOffset;

			pBuff = (BYTE *) &dwCount;
			pStride[j].pHeader[0] = ((pBuff[1] & 0x10) >> 4);
			pStride[j].pHeader[1] = ((pBuff[1] & 0x0F) << 4);
			pStride[j].pHeader[1] += ((pBuff[0] & 0xF0) >> 4);
			pStride[j].pHeader[2] = ((pBuff[0] & 0x0F) << 4);
			pBuff = (BYTE *) &dwOffset;
			pStride[j].pHeader[2] += ((pBuff[1] & 0x0F));
			pStride[j].pHeader[3] = ((pBuff[0]));
			memcpy(&(pStride[j].tsp),&(pTsp[i]),sizeof(MPEG2_TRANSPORT));
			j++;
			newdatasize += sizeof(MPEG2_TRANSPORT_STRIDE);
		}
	}
	
    // Copy the sample times

    REFERENCE_TIME TimeStart, TimeEnd;
    if (S_OK == pSource->GetTime(&TimeStart, &TimeEnd)) {
        pDest->SetTime(&TimeStart, &TimeEnd);
    }

    LONGLONG MediaStart, MediaEnd;
    if (pSource->GetMediaTime(&MediaStart,&MediaEnd) == S_OK) {
        pDest->SetMediaTime(&MediaStart,&MediaEnd);
    }

    // Copy the Sync point property

    hr = pSource->IsSyncPoint();
    if (hr == S_OK) {
        pDest->SetSyncPoint(TRUE);
    }
    else if (hr == S_FALSE) {
        pDest->SetSyncPoint(FALSE);
    }
    else {  // an unexpected error has occured...
        return E_UNEXPECTED;
    }

    // Copy the media type

    AM_MEDIA_TYPE *pMediaType;
    pSource->GetMediaType(&pMediaType);
    pDest->SetMediaType(pMediaType);
    DeleteMediaType(pMediaType);

    // Copy the preroll property

    hr = pSource->IsPreroll();
    if (hr == S_OK) {
        pDest->SetPreroll(TRUE);
    }
    else if (hr == S_FALSE) {
        pDest->SetPreroll(FALSE);
    }
    else {  // an unexpected error has occured...
        return E_UNEXPECTED;
    }

    // Copy the discontinuity property

    hr = pSource->IsDiscontinuity();
    if (hr == S_OK) {
	pDest->SetDiscontinuity(TRUE);
    }
    else if (hr == S_FALSE) {
        pDest->SetDiscontinuity(FALSE);
    }
    else {  // an unexpected error has occured...
        return E_UNEXPECTED;
    }

    pDest->SetActualDataLength(newdatasize);

	if(newdatasize == 0)
		return S_FALSE;
	return S_OK;
}

//
// CheckInputType
//
// Check the input type is OK, return an error otherwise
//
HRESULT CTSP2SP::CheckInputType(const CMediaType *mtIn)
{
   	return S_OK;

} // CheckInputType


//
// DecideBufferSize
//
// Tell the output pin's allocator what size buffers we
// require. Can only do this when the input is connected
//
HRESULT CTSP2SP::DecideBufferSize(IMemAllocator *pAlloc,ALLOCATOR_PROPERTIES *pProperties)
{
    ASSERT(pAlloc);
    ASSERT(pProperties);
    HRESULT hr = S_OK;

    pProperties->cBuffers = 3;
    pProperties->cbBuffer = sizeof(MPEG2_TRANSPORT_STRIDE) * 256;
    pProperties->cbAlign  = 1;
    pProperties->cbPrefix = 0;

    // Ask the allocator to reserve us some sample memory, NOTE the function
    // can succeed (that is return S_OK) but still not have allocated the
    // memory that we requested, so we must check we got whatever we wanted

    ALLOCATOR_PROPERTIES Actual;
    hr = pAlloc->SetProperties(pProperties,&Actual);
    if (FAILED(hr)) {
        return hr;
    }

    if (pProperties->cBuffers != Actual.cBuffers ||
            pProperties->cbBuffer != Actual.cbBuffer) {
                return E_FAIL;
    }
    return S_OK;

} // DecideBufferSize


//
// GetMediaType
//
// I support one type, namely the type of the input pin
// We must be connected to support the single output type
//
HRESULT CTSP2SP::GetMediaType(int iPosition, CMediaType *pMediaType)
{
    // Is the input pin connected

    if (m_pInput->IsConnected() == FALSE) {
        return E_UNEXPECTED;
    }

    // This should never happen

    if (iPosition < 0) {
        return E_INVALIDARG;
    }

    // Do we have more items to offer

    if (iPosition > 0) {
        return VFW_S_NO_MORE_ITEMS;
    }

    *pMediaType = m_pInput->CurrentMediaType();
    return S_OK;

} // GetMediaType

CBasePin * CTSP2SP::GetPin(int n)
{
    HRESULT hr = S_OK;

    // Create an input pin if necessary

    if(m_pInput == NULL) {

        m_pInput = new CTSP2SPInputPin(NAME("TSP input"),
            this,              // Owner filter
            &hr,               // Result code
            L"TransportPackets In");      // Pin name


        //  Can't fail
        ASSERT(SUCCEEDED(hr));
        if(m_pInput == NULL) {
            return NULL;
        }
        m_pOutput = (CTransformOutputPin *)
            new CTransformOutputPin(NAME("SP output"),
            this,            // Owner filter
            &hr,             // Result code
            L"SourcePackets Out");   // Pin name


        // Can't fail
        ASSERT(SUCCEEDED(hr));
        if(m_pOutput == NULL) {
            delete m_pInput;
            m_pInput = NULL;
        }
    }

    // Return the appropriate pin

    if(n == 0) {
        return m_pInput;
    }
    else
        if(n == 1) {
        return m_pOutput;
    }
    else {
        return NULL;
    }
}

//
// DllRegisterServer
//
// Handle registration of this filter
//
STDAPI DllRegisterServer()
{
    return AMovieDllRegisterServer2( TRUE );

} // DllRegisterServer


//
// DllUnregisterServer
//
STDAPI DllUnregisterServer()
{
    return AMovieDllRegisterServer2( FALSE );

} // DllUnregisterServer

// CPullPin
CTSPAsyncPin::CTSPAsyncPin(CTSP2SPInputPin *pOwner)
{ m_pOwner = pOwner; }

HRESULT CTSPAsyncPin::Receive(IMediaSample *pSample)
{ return m_pOwner->Receive(pSample); }

HRESULT CTSPAsyncPin::EndOfStream(void)
{ return m_pOwner->EndOfStream(); }

HRESULT CTSPAsyncPin::BeginFlush(void)
{ return m_pOwner->BeginFlush(); }

HRESULT CTSPAsyncPin::EndFlush(void)
{ return m_pOwner->EndFlush(); }

void CTSPAsyncPin::OnError(HRESULT hr)
{ return; }

HRESULT CTSPAsyncPin::DecideAllocator(IMemAllocator* pAlloc,ALLOCATOR_PROPERTIES * pProps)
{
    HRESULT hr = S_OK;
    ALLOCATOR_PROPERTIES Request;
    IAsyncReader * pReader;
    Request.cBuffers = 3;
    Request.cbBuffer = sizeof(MPEG2_TRANSPORT) * 256;
    Request.cbAlign  = 1;
    Request.cbPrefix = 0;

	pReader = GetReader();
    if(pReader)
	{
        hr = pReader->RequestAllocator(pAlloc,&Request,&m_pAlloc);
		pReader->Release();
	}
    else 
        hr = E_FAIL;
    return hr;
}

//CTransfortInputPin
CTSP2SPInputPin::CTSP2SPInputPin(TCHAR *pObjectName,CTransformFilter *pTransformFilter,
	HRESULT *phr,LPCWSTR pName) : CTransformInputPin(pObjectName,pTransformFilter,phr,pName)
{
	m_pAsync = new CTSPAsyncPin(this);
	if(m_pAsync == NULL)
		*phr = E_OUTOFMEMORY;
}

HRESULT CTSP2SPInputPin::CheckConnect(IPin *pPin)
{
	return m_pAsync->Connect(pPin,NULL,true);
}

HRESULT CTSP2SPInputPin::BreakConnect(void)
{
	return m_pAsync->Disconnect();
}

HRESULT CTSP2SPInputPin::Active(void)
{
	return m_pAsync->Active();
}

HRESULT CTSP2SPInputPin::Inactive(void)
{
	return m_pAsync->Inactive();
}

