// {9D509FE5-1CBD-44d4-9335-8C87E81FDAB5}
DEFINE_GUID(CLSID_TSP2SP, 
0x9d509fe5, 0x1cbd, 0x44d4, 0x93, 0x35, 0x8c, 0x87, 0xe8, 0x1f, 0xda, 0xb5);

class CTSP2SPInputPin ;
class CTSPAsyncPin ;

class CTSPAsyncPin : public CPullPin
{
private:
	CTSP2SPInputPin *m_pOwner;
public:
	CTSPAsyncPin(CTSP2SPInputPin *pOwner);
	HRESULT Receive(IMediaSample *pSample);
	HRESULT EndOfStream(void);
	HRESULT BeginFlush(void);
	HRESULT EndFlush(void);
	void OnError(HRESULT hr);
	HRESULT DecideAllocator(IMemAllocator* pAlloc,ALLOCATOR_PROPERTIES * pProps);
};

class CTSP2SPInputPin : public CTransformInputPin
{
private:
	CTSPAsyncPin *m_pAsync;
public:
	CTSP2SPInputPin(
		TCHAR *pObjectName,
		CTransformFilter *pTransformFilter,
		HRESULT *phr,
		LPCWSTR pName);
	HRESULT CheckConnect(IPin *pPin);
	HRESULT BreakConnect(void);
	HRESULT Active(void);
	HRESULT Inactive(void);
};

class CTSP2SP : public CTransformFilter
{

public:

    static CUnknown * WINAPI CreateInstance(LPUNKNOWN punk, HRESULT *phr);

    // Reveals ITSP2SP & ISpecifyPropertyPages
    STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void ** ppv);

    DECLARE_IUNKNOWN;

    HRESULT Transform(IMediaSample *pIn, IMediaSample *pOut);
    HRESULT CheckInputType(const CMediaType *mtIn);
    HRESULT GetMediaType(int iPosition, CMediaType *pMediaType);
    HRESULT DecideBufferSize(IMemAllocator *pAlloc,
                             ALLOCATOR_PROPERTIES *pProperties);
	HRESULT CheckTransform(const CMediaType *mtIn,const CMediaType *mtOut);
	CBasePin * GetPin(int n);

private:

    // Constructor
    CTSP2SP(TCHAR *tszName, LPUNKNOWN punk, HRESULT *phr);

	DWORD m_lastCount;
	DWORD m_lastOffset;
	DWORD m_lastCountDelta;
	DWORD m_lastOffsetDelta;
	DWORD m_CountDelta;
	DWORD m_OffsetDelta;
	__int64 m_lastClock;
	DWORD m_packets;
	bool m_fValidStamp;

}; // CTSP2SP

typedef struct _MPEG2_TRANSPORT {
	BYTE pTransportHeader[4];
	union
	{
		struct
		{
			BYTE pAdaptationLength;
			BYTE pAdaptationFlags;
			BYTE pPCR[6];
		} AdaptationField;
		BYTE pTransportData[184];
	};
} MPEG2_TRANSPORT;

typedef struct _MPEG2_TRANSPORT_STRIDE {
    BYTE pHeader[4];
	MPEG2_TRANSPORT tsp;
} MPEG2_TRANSPORT_STRIDE;


