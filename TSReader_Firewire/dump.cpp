//------------------------------------------------------------------------------
// Dump Input pin to TSReader
//
// Copyright (c) 2006 Igoru Hamasaki
//------------------------------------------------------------------------------

#define _WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <streams.h>
#include <initguid.h>
#include "dumpuids.h"
#include "dump.h"
#include "..\sources.h"

extern PSOURCESTRUCT ss;

CDumpFilter::CDumpFilter(LPUNKNOWN pUnk, HRESULT *phr) :
    CBaseFilter(NAME("CDumpFilter"), pUnk, &m_Lock, CLSID_Dump),
    m_pPin(NULL),
    nTSBufferIndex(0)
{
    m_pPin = new CDumpInputPin(this, NULL, &m_Lock, &m_ReceiveLock, phr);
}

CDumpInputPin::CDumpInputPin(CDumpFilter *pDump, LPUNKNOWN pUnk, CCritSec *pLock, CCritSec *pReceiveLock, HRESULT *phr) :
    CRenderedInputPin(NAME("CDumpInputPin"), pDump, pLock, phr, L"Input"),
    m_pReceiveLock(pReceiveLock),
    m_pDump(pDump)
{
}

// Receive from DShow and push to TSReader
STDMETHODIMP CDumpInputPin::Receive(IMediaSample *pSample)
{
    CheckPointer(pSample,E_POINTER);

    CAutoLock lock(m_pReceiveLock);
    PBYTE pbData;
    LONG lDataLength = 0;

    // Copy the data to the file
    HRESULT hr = pSample->GetPointer(&pbData);
    if (FAILED(hr))
        return hr;
    
    lDataLength = pSample->GetActualDataLength();
    m_pDump->Write(pbData, lDataLength);

    return S_OK;
}

// Write buffer to TSReader
void CDumpFilter::Write(PBYTE pbData, LONG lDataLength)
{
	/*
    memcpy(ss->tsb[nTSBufferIndex].pData, pbData, lDataLength);
    EnterCriticalSection(&ss->csPIDCounter);
    ss->nLastSecondByteCounter += lDataLength;
    LeaveCriticalSection(&ss->csPIDCounter);

    ss->tsb[nTSBufferIndex].nSize = lDataLength;
    nTSBufferIndex++;
    if (nTSBufferIndex == MAX_TS_BUFFERS)
		nTSBufferIndex = 0;
    EnterCriticalSection(&ss->csTSBuffersInUse);
    ss->nTSBuffersInUse++;
    LeaveCriticalSection(&ss->csTSBuffersInUse);
	*/
	SourceHelper_SyncData(pbData, lDataLength);
}
