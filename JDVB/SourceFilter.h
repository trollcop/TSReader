#ifndef SOURCEFILTER_H___
#define SOURCEFILTER_H___


#include "SourceFilterInterface.h"
#include <windows.h>
#include <streams.h>
#include <string.h>
// {52EECB1F-B9F3-497b-8360-F0D5544BA2CD}

#define MPEG2VIDEO 0x1
#define MPEG2AUDIO 0x2
#define MPEG1VIDEO 0x4
#define MPEG1AUDIO 0x8
#define MAX_PIN	   0x5

class CSourceFilter;
class TSPidFilter;
class TSPacketQueue;
class TSPacket;
/*********************************************************************************
 * CSourceStreamAudio
 *
 * Description: implements CSourceStream pin on audio basis
 *
 ********************************************************************************/
class CSourceStreamPin : public CSourceStream
{
protected:
	CRefTime m_rtSampleTime;	        // The time stamp for each sample
	CCritSec m_cSharedState;            // Protects our internal state
	TSPacketQueue *Queue_;
	TSPidFilter *Next_;
	TSPacket *Head_;
	TSPacket *Tail_;
	long BufferedPackets_;
	REFERENCE_TIME MediaTime_;


public:
	DECLARE_IUNKNOWN;
	CSourceStreamPin (HRESULT *phr, CSource *pFilter, const wchar_t * PinName);
	

	virtual ~CSourceStreamPin();
	//the following methods have to overwritten
	//by the overriding class

	HRESULT GetMediaType(CMediaType *pMediaType);
    HRESULT DecideBufferSize(IMemAllocator *pAlloc, ALLOCATOR_PROPERTIES *pRequest);
    HRESULT FillBuffer(IMediaSample *pSample);
	HRESULT CheckMediaType (const CMediaType *pMediaType);
	HRESULT insertSample(TSPacket *Packet);
	HRESULT setTSPacketQueue(TSPacketQueue *Queue);
	HRESULT setNextTSPidFilter(TSPidFilter *Next);
	//FIXME
	// Quality control
	// Not implemented because we aren't going in real time.
	// If the file-writing filter slows the graph down, we just do nothing, which means
	// wait until we're unblocked. No frames are ever dropped.
    STDMETHODIMP Notify(IBaseFilter *pSelf, Quality q)
    {
        return E_FAIL;
    }

};









/*********************************************************************************
 * CSourceFilter
 *
 * implements CSource interface
 *
 *
 ********************************************************************************/

class CSourceFilter : public CSource,
					  public ISourceFilter  //SourceFilter Interface
					 
{

	friend class CSourceStreamPin;

protected:
	
	

	//constructor is private !
	CSourceStreamPin * m_pPin;
	
public:
	CSourceFilter(IUnknown *pUnk, HRESULT *phr);

	DECLARE_IUNKNOWN;

	static CUnknown * WINAPI CreateInstance(IUnknown *pUnk, HRESULT *phr);


	virtual ~CSourceFilter();

	
	
	STDMETHODIMP FindPin(const wchar_t * PinName,IPin ** Pin);



	STDMETHODIMP processPid(TSPacket *Packet);
	STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void **ppv);


	STDMETHODIMP setTSPacketQueue(TSPacketQueue *Queue);

	STDMETHODIMP setNextTSPidFilter(TSPidFilter *Next);




};


#endif