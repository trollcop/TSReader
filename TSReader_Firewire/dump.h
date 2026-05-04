//------------------------------------------------------------------------------
// Dump Input pin to TSReader
//
// Copyright (c) 2006 Igoru Hamasaki
//------------------------------------------------------------------------------

class CDumpInputPin;
class CDumpFilter;

//  Pin object
class CDumpInputPin :
    public CRenderedInputPin
{
protected:
    CDumpFilter* const m_pDump;           // Main renderer object
    CCritSec *const m_pReceiveLock;    // Sample critical section

public:
    CDumpInputPin(CDumpFilter *pDump, LPUNKNOWN pUnk, CCritSec *pLock, CCritSec *pReceiveLock, HRESULT *phr);

    STDMETHODIMP EndOfStream(void) { CAutoLock lock(m_pReceiveLock); return CRenderedInputPin::EndOfStream(); }
    STDMETHODIMP ReceiveCanBlock() { return S_OK; }
    HRESULT BreakConnect() { return CRenderedInputPin::BreakConnect(); }
    HRESULT CheckMediaType(const CMediaType *mt) { return S_OK; }

    // Do something with this media sample
    STDMETHODIMP Receive(IMediaSample *pSample);
};

// Main filter object
class CDumpFilter : 
    public CBaseFilter, 
    public IFileSinkFilter
{
protected:
    CDumpInputPin *m_pPin;
    CCritSec m_Lock;
    CCritSec m_ReceiveLock;
    int nTSBufferIndex;

public:
    DECLARE_IUNKNOWN
    // STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void ** ppv);

    CDumpFilter(LPUNKNOWN pUnk, HRESULT *phr);

    // Pin enumeration
    CBasePin *GetPin(int n) { if (n == 0) return m_pPin; return NULL; }
    int GetPinCount() { return 1; }

    // IFileSinkFilter
    STDMETHODIMP SetFileName(LPCOLESTR pszFileName,const AM_MEDIA_TYPE *pmt) { return S_OK; }
    STDMETHODIMP GetCurFile(LPOLESTR * ppszFileName,AM_MEDIA_TYPE *pmt) { return S_OK; }
    // TSReader-specific code
    void Write(PBYTE pbData, LONG lDataLength);
};
