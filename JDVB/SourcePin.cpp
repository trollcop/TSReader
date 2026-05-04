#include "SourceFilter.h"
#include "TSPacket.h"
#include "TSPacketQueue.h"
#include "TSPidFilter.h"
#include <streams.h>

//--------------------------------------------------------------------------------
CSourceStreamPin::CSourceStreamPin (HRESULT *phr, CSource *pFilter, const wchar_t * PinName) :
	  CSourceStream(NAME("CSourceStreamPin"),phr,pFilter,PinName)
{
	Head_ = 0;
	Tail_ = 0;
	Queue_ = 0;
	BufferedPackets_ = 0;
	MediaTime_ = 0;
	Next_ = 0;
	Queue_ = 0;
}

	
//--------------------------------------------------------------------------------

CSourceStreamPin::~CSourceStreamPin()
{
	TSPacket *Current = 0;
	//emptied any reserved packets on closure
	while(Current)
	{
		Current = Head_->getNext ();
		if(Next_)
			Next_->dispatch (Head_);
		else
		{
#ifdef DEBUG
			ASSERT(Queue_);
#endif
			Queue_->push (Head_);
		}
		Head_ = Current;
	}
}

//---------------------------------------------------------------------------------

HRESULT CSourceStreamPin::GetMediaType(CMediaType *pMediaType)
{
	CAutoLock cAutoLock(m_pFilter->pStateLock());

    CheckPointer(pMediaType, E_POINTER);
	//TwinHan DVB Demultiplexer
	//pMediaType->SetType (&MEDIATYPE_Stream);
	//pMediaType->SetSubtype (&GUID_NULL);
	//pMediaType->SetFormatType (&GUID_NULL);


	//MPEG-2 Demultiplexer acceptable stuff
	pMediaType->SetType (&MEDIATYPE_Stream);
	pMediaType->SetSubtype (&MEDIASUBTYPE_MPEG2_TRANSPORT);
	//pMediaType->SetFormatType (&GUID_NULL);

	return NOERROR;
}

//--------------------------------------------------------------------------------
HRESULT CSourceStreamPin::CheckMediaType (const CMediaType *pMediaType)
{
	return S_OK;
}
//--------------------------------------------------------------------------------

HRESULT CSourceStreamPin::DecideBufferSize(IMemAllocator *pAlloc, ALLOCATOR_PROPERTIES *pRequest)
{
	HRESULT hr;
    CAutoLock cAutoLock(m_pFilter->pStateLock());

    CheckPointer(pAlloc, E_POINTER);
    CheckPointer(pRequest, E_POINTER);
    
    // Ensure a minimum number of buffers
    if (pRequest->cBuffers == 0)
    {
        pRequest->cBuffers = 1;
    }
    pRequest->cbBuffer = 188 *25; //FIXME
	pRequest->cBuffers = 1;
    ALLOCATOR_PROPERTIES Actual;
    hr = pAlloc->SetProperties(pRequest, &Actual);
    if (FAILED(hr)) 
    {
        return hr;
    }

    // Is this allocator unsuitable?
    if (Actual.cbBuffer < pRequest->cbBuffer) 
    {
        return E_FAIL;
    }

	ASSERT(Actual.cBuffers == 1);

    return S_OK;
}

//--------------------------------------------------------------------------------
 
HRESULT CSourceStreamPin::FillBuffer(IMediaSample *pSample)
{
    CheckPointer(pSample, E_POINTER);
	//CAutoLock cAutoLockShared(&m_cSharedState);
	
	if(pSample == 0 || Head_ == 0)
	{
		while(Head_ == 0)
			_sleep(100);

	}

	long BufferSize = pSample->GetSize ();
	BYTE * Buffer = NULL;
	pSample->GetPointer (&Buffer);
	TSPacket *Current = 0;
	long BufferPosition;

	for(BufferPosition = 0; BufferPosition < BufferSize / 188 ; BufferPosition++)
	{
		if(Head_ == 0)
			break;
		memcpy(&Buffer[BufferPosition * 188],(BYTE *)Head_->getData(), 188);
		BufferedPackets_--;
		Current = Head_->getNext ();
		if(Next_)
			Next_->dispatch (Head_);
		else
		{
#ifdef DEBUG
			ASSERT(Queue_);
#endif
			Queue_->push (Head_);
		}
		Head_ = Current;
	}

	 REFERENCE_TIME TIMEStart = 0xCCCCCCCCCCCCCCCC;
	 REFERENCE_TIME TIMEEnd = 0xCCCCCCCCCCCCCCCC;


	pSample->SetTime (&TIMEStart,&TIMEEnd);
	
	
	MediaTime_ += BufferPosition * 188;

	pSample->SetActualDataLength (BufferPosition * 188);
	pSample->SetDiscontinuity (false);
	pSample->SetPreroll (false);
	pSample->SetSyncPoint (false);
	

	return S_OK;

}
//-------------------------------------------------------------------
HRESULT CSourceStreamPin::insertSample (TSPacket *Packet)
{
	if(Packet == 0)
		return E_FAIL;
	CAutoLock cAutoLockShared(&m_cSharedState);
	

	if(Head_ == 0)
	{
		Head_ = Tail_ = Packet;
	}
	else
	{
		Tail_->setNext (Packet);
		Tail_ = Packet;
	}
	Tail_->setNext (0);

	BufferedPackets_++;
	return NOERROR;
}
//-------------------------------------------------------------------
HRESULT CSourceStreamPin::setNextTSPidFilter (TSPidFilter *Next)
{
	Next_ = Next;
	return S_OK;
}
//-------------------------------------------------------------------
HRESULT CSourceStreamPin::setTSPacketQueue (TSPacketQueue *Queue)
{
	Queue_ = Queue;
	return S_OK;
}