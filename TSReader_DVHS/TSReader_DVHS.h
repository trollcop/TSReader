//
//  Define an internal filter that wraps the base CBaseReader stuff
//

class CMemStream : public CAsyncStream
{
public:
	CMemStream(HANDLE *hfile, LPBYTE pbData, LONGLONG llLength, IGraphBuilder *pFG, double * pdTotalRecorded, int * pnPipeBytes, CRITICAL_SECTION csPipeBytes) :
		m_hfile(*hfile),
        m_pbData(pbData),
        m_llLength(llLength),
		m_pFG(pFG),
		m_pdTotalRecorded(pdTotalRecorded),
		m_pnPipeBytes(pnPipeBytes),
		m_csPipeBytes(csPipeBytes)
    {
		m_fFirstTime = TRUE;
    }

    HRESULT SetPointer(LONGLONG llPos)
    {
		return S_OK;
    }

    HRESULT Read(PBYTE pbBuffer, DWORD dwBytesToRead, BOOL bAlign, LPDWORD pdwBytesRead)
    {
		CAutoLock lck(&m_csLock);

        DWORD dwReadLength;
		DWORD dwBytesRead;
		DWORD dwOffset = 0;

		if (m_fFirstTime)
		{
			m_fFirstTime = FALSE;
			Sleep(250);
		}
		dwReadLength = dwBytesToRead;
		do
		{
			if (!ReadFile(m_hfile,(LPVOID)m_pbData, dwReadLength, &dwBytesRead, NULL))
			{
				*pdwBytesRead = 0;				
				IMediaEventSink *pME;
				LONG_PTR levCode = NULL;
				LONG_PTR levCode2 = NULL;

				HRESULT hr = m_pFG->QueryInterface(IID_IMediaEventSink, (void **)&pME);
				if (FAILED(hr)) 
				{
					OutputDebugString("\nFailed IMediaEventSink!!!!");
					return hr;
				}
				hr = pME->Notify(EC_USERABORT, levCode, levCode2);
				pME->Release();

				return E_FAIL ;
			}
			CopyMemory((PVOID)(pbBuffer + dwOffset), (PVOID)(m_pbData), dwBytesRead);
			dwReadLength -= dwBytesRead;
			dwOffset += dwBytesRead;
		} while (dwReadLength);

		EnterCriticalSection(&m_csPipeBytes);
		*m_pnPipeBytes -= dwBytesToRead;
        LeaveCriticalSection(&m_csPipeBytes);
		*pdwBytesRead = dwBytesToRead;
		*m_pdTotalRecorded += (double)dwBytesToRead;

		return S_OK;
    }
    
	LONGLONG Size(LONGLONG *pSizeAvailable)
    {        
        *pSizeAvailable = m_llLength;
        return m_llLength;
    }
    DWORD Alignment()
    {
        return 1;
    }
    void Lock()
    {
        m_csLock.Lock();
    }
    void Unlock()
    {
        m_csLock.Unlock();
    }

private:   
	CCritSec       m_csLock;
    const PBYTE    m_pbData;
    const LONGLONG m_llLength;
	HANDLE			m_hfile;
	IGraphBuilder  *m_pFG;
	BOOL m_fFirstTime;
	double * m_pdTotalRecorded;
	int * m_pnPipeBytes;
	CRITICAL_SECTION m_csPipeBytes;
};

class CMemReader : public CAsyncReader
{
public:

    //  We're not going to be CoCreate'd so we don't need registration
    //  stuff etc
    STDMETHODIMP Register()
    {
        return S_OK;
    }
    STDMETHODIMP Unregister()
    {
        return S_OK;
    }
    CMemReader(CMemStream *pStream, CMediaType *pmt, HRESULT *phr) :
        CAsyncReader(TEXT("Mem Reader"), NULL, pStream, phr)
    {
        m_mt = *pmt;
    }
};

