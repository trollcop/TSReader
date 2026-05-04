#include "DirectShowTSTarget.h"
#include "AVPidFilter.h"
#include "CLSID.h"
#include "SourceFilter.h"
#define FILTER_RECREATE_RETRIES	0x3


//-------------------------------------------------------------------
DirectShowTSTarget::DirectShowTSTarget (const char *Name)
: TSTarget(Name)
{
	Mute_ = false;
	FullScreen_ = false;
	running_ = false;
	Volume_ = 0;
	GraphBuilder_ = 0;
	BasicAudio_ = 0;
	VideoWindow_ = 0;
	MediaControl_ = 0;
	//IBase Filters
	SourceFilter_= 0;
	DVBDemux_ = 0;
	VideoDecoder_ = 0;
	AudioDecoder_ = 0;
	AudioRenderer_ = 0;
	VideoRenderer_ = 0;
	//Interfaces
	ISourceFilter_ = 0;
	IMPEG2Demux_ = 0;
	PATFilter_ = 0;
	PMTFilter_ = 0;
	AudioFilter_ = 0;
	VideoFilter_ = 0;
	PCRFilter_ = 0;
}
//-------------------------------------------------------------------
DirectShowTSTarget::~DirectShowTSTarget ()
{
	
	if(MediaControl_)
		MediaControl_->Stop ();
	removeAllFilterFromGraph();
	SAFE_RELEASE(ISourceFilter_);
	SAFE_RELEASE(SourceFilter_);
	SAFE_RELEASE(DVBDemux_);
	SAFE_RELEASE(MediaControl_);
	SAFE_RELEASE(GraphBuilder_);
#ifdef DEBUG
	ASSERT(PATFilter_ == 0);
	ASSERT(PMTFilter_ == 0);
	ASSERT(AudioFilter_ == 0);
	ASSERT(VideoFilter_ == 0);
	ASSERT(PCRFilter_ == 0);
#endif

}
//-------------------------------------------------------------------
HRESULT DirectShowTSTarget::deleteTSTargetFilter ( const unsigned int PidType)
{
	switch(PidType)
	{
		case PID_TYPE_PAT:
		{
			if(PATFilter_ == 0)
				return E_FAIL;
			static_cast<AVPidFilter *>(PATFilter_)->setFilter (0);
			delete PATFilter_;
			PATFilter_ = 0;
			return S_OK;
		}
		case PID_TYPE_PMT:
		{
			if(PMTFilter_ == 0)
				return E_FAIL;
			static_cast<AVPidFilter *>(PMTFilter_)->setFilter (0);
			delete PMTFilter_;
			PMTFilter_ = 0;
			return S_OK;
		}
		case PID_TYPE_PCR:
		{
			if(PCRFilter_ == 0)
				return E_FAIL;
			static_cast<AVPidFilter *>(PCRFilter_)->setFilter (0);
			delete PCRFilter_;
			PCRFilter_ = 0;
			return S_OK;
		}
		case PID_TYPE_AUDIO:
		case PID_TYPE_AUDIO_MPEG1:
		case PID_TYPE_AUDIO_MPEG2:
		case PID_TYPE_AUDIO_AC3:
		{
			if(AudioFilter_ == 0)
				return E_FAIL;
			static_cast<AVPidFilter *>(AudioFilter_)->setFilter (0);
#ifdef DEBUG
			ASSERT(IMPEG2Demux_);
#endif
			delete AudioFilter_;
			AudioFilter_ = 0;
			return IMPEG2Demux_->DeleteOutputPin (L"Audio");
		}
		case PID_TYPE_VIDEO:
		case PID_TYPE_VIDEO_MPEG1:
		case PID_TYPE_VIDEO_MPEG2:
		{
			if(VideoFilter_ == 0)
				return E_FAIL;
			static_cast<AVPidFilter *>(VideoFilter_)->setFilter (0);
#ifdef DEBUG
			ASSERT(IMPEG2Demux_);
#endif
			delete VideoFilter_;
			VideoFilter_ = 0;
			return IMPEG2Demux_->DeleteOutputPin (L"Video");
		}
	}

	return E_FAIL;
}
//-------------------------------------------------------------------
HRESULT DirectShowTSTarget::createTSTargetFilter(const unsigned short Pid, const unsigned int PidType, const char *PidName, TSPidFilter **Filter)
{
	if(GraphBuilder_ == 0)
	{
		if(FAILED(CoCreateInstance(CLSID_FilterGraph, NULL, CLSCTX_INPROC_SERVER, IID_IGraphBuilder, (void **)&GraphBuilder_)))
			return E_FAIL;	
		if(GraphBuilder_)
			GraphBuilder_->QueryInterface (IID_IMediaControl, (void **)&MediaControl_);
	}

	if(SourceFilter_ == 0)
	{
		HRESULT hr = S_OK;
		IUnknown *pUnk = 0;
		SourceFilter_ = new CSourceFilter(pUnk,&hr);

		if(FAILED(hr))
		{
			delete SourceFilter_;	
			SourceFilter_ = 0;
			return E_FAIL;
		}
		hr = GraphBuilder_->AddFilter (SourceFilter_,L"Source Filter Version 0.1");
		if(FAILED(hr))
		{
			delete SourceFilter_;	
			SourceFilter_ = 0;
			return E_FAIL;
		}

		if(FAILED(queryInterface(SourceFilter_,IID_SOURCE_FILTER, (void **)&ISourceFilter_)))
			return E_FAIL;
	}

	if(DVBDemux_ == 0)
	{
		if(FAILED(createIBaseFilterAndAddToGraph(L"MS MPEG-2 Demultiplexer",CLSID_MPEG2Demultiplexer,&DVBDemux_)))
			return E_FAIL;

		if(FAILED(queryInterface(DVBDemux_,IID_IMpeg2Demultiplexer, (void **)&IMPEG2Demux_)))
			return E_FAIL;
		if(SourceFilter_ && DVBDemux_)
			if(FAILED(connectIBaseFilters(SourceFilter_,L"Stream 1",DVBDemux_,PN_MPEG2_DEMUX_IN)))
				return E_FAIL;

	}
	
	switch(PidType)
	{
		case PID_TYPE_PAT:
		{
			if(PATFilter_)
			{
				*Filter = PATFilter_;
				return S_OK;
			}
			ASSERT(Pid == 0);
			*Filter = PATFilter_ = new AVPidFilter(Pid,PidName);
			static_cast<AVPidFilter *>(PATFilter_)->setFilter (ISourceFilter_);
			return S_OK;
		}
		case PID_TYPE_PMT:
		{
			if(PMTFilter_)
			{
				if(PMTFilter_->getPid (0) != Pid)
					return E_FAIL;	//the old filter must be released first ->Memory Leak
				*Filter = PMTFilter_;
				return S_OK;
			}

			*Filter = PMTFilter_ = new AVPidFilter(Pid,PidName);
			static_cast<AVPidFilter *>(PMTFilter_)->setFilter (ISourceFilter_);
			return S_OK;
		}
		case PID_TYPE_PCR:
		{	
			if(PCRFilter_)
			{
				if(PCRFilter_->getPid (0) != Pid)
					return E_FAIL;

				*Filter = PCRFilter_;
				return S_OK;
			}


			*Filter = PCRFilter_ = new AVPidFilter(Pid,PidName);
			static_cast<AVPidFilter *>(*Filter)->setFilter (ISourceFilter_);
			return S_OK;
		}
		case PID_TYPE_AUDIO_MPEG1:
		case PID_TYPE_AUDIO_MPEG2:
		case PID_TYPE_AUDIO_AC3:
		{
			if(AudioFilter_)
			{	
				if(AudioFilter_->getPid (0) !=Pid)
					return E_FAIL;
				*Filter = AudioFilter_;
				return S_OK;
			}
			if(AudioDecoder_ == 0)
				if(FAILED(createIBaseFilterAndAddToGraph(L"Audio Decoder",CLSID_AudioDecoder,&AudioDecoder_)))
					return E_FAIL;
			IPin *SourcePin = 0;
			IPin *TargetPin = 0;
			HRESULT hr;
			if(FAILED(getPin(DVBDemux_,L"Audio",&SourcePin)))
				if(FAILED(createMPEG2DemuxPin(L"Audio",&SourcePin,PidType)))
					return E_FAIL;

			if(FAILED(mapMPEG2DemuxPin(SourcePin,Pid,MEDIA_ELEMENTARY_STREAM)))
			{			
				SourcePin->Release ();
				IMPEG2Demux_->DeleteOutputPin (L"Audio");			
				return E_FAIL;
			}
			if(FAILED(getPin(AudioDecoder_,PN_AUDIO_DECODER_IN,&TargetPin)))
			{
				IMPEG2Demux_->DeleteOutputPin (L"Audio");
				return E_FAIL;

			}
			hr = GraphBuilder_->Connect (SourcePin,TargetPin);
			SourcePin->Release ();
			TargetPin->Release ();
			if(FAILED(hr))
			{
				IMPEG2Demux_->DeleteOutputPin (L"Audio");
				return E_FAIL;
			}
			if(AudioRenderer_ == 0)
			{
				if(FAILED(createIBaseFilterAndAddToGraph(L"Audio Renderer",CLSID_AudioRenderer,&AudioRenderer_)))
					return E_FAIL;
				if(FAILED(connectIBaseFilters(AudioDecoder_,PN_AUDIO_DECODER_OUT,AudioRenderer_,PN_AUDIO_RENDERER_IN)))
					return E_FAIL;
			}
			IReferenceClock *RenderClock = 0;
			queryInterface(AudioRenderer_,IID_IBasicAudio,(void **)&BasicAudio_);
			
			if(SUCCEEDED(AudioRenderer_->QueryInterface (IID_IReferenceClock,(void **)&RenderClock)))
			{
				//using the MS MPEG-2 Demultiplexer is just fine
				//except that it takes property of the IReferenceClock
				//but that is needed to hear sound :-)
				//this took me 5 days to figure out ...
				IMediaFilter *pMediaFilter = 0;
				GraphBuilder_->QueryInterface(IID_IMediaFilter, (void**)&pMediaFilter);
				pMediaFilter->SetSyncSource (RenderClock);
				RenderClock->Release ();
				pMediaFilter->Release ();
			}
				

			*Filter = AudioFilter_ = new AVPidFilter(Pid,PidName);
			static_cast<AVPidFilter *>(AudioFilter_)->setFilter (ISourceFilter_);
			return S_OK;
		}
		case PID_TYPE_VIDEO:
		case PID_TYPE_VIDEO_MPEG1:
		case PID_TYPE_VIDEO_MPEG2:
		{
			if(VideoFilter_)
			{
				if(VideoFilter_->getPid (0) != Pid)
					return E_FAIL;

				*Filter = VideoFilter_;
				return S_OK;

			}
			if(VideoDecoder_ == 0)
				if(FAILED(createIBaseFilterAndAddToGraph(L"Video Decoder",CLSID_VideoDecoder,&VideoDecoder_)))
					return E_FAIL;
			IPin *SourcePin = 0;
			IPin *TargetPin = 0;
			HRESULT hr;
			if(FAILED(getPin(DVBDemux_,L"Video",&SourcePin)))
				if(FAILED(createMPEG2DemuxPin(L"Video",&SourcePin,PidType)))
					return E_FAIL;

			if(FAILED(mapMPEG2DemuxPin(SourcePin,Pid,MEDIA_ELEMENTARY_STREAM)))
			{
				SourcePin->Release ();
				IMPEG2Demux_->DeleteOutputPin (L"Video");
				return E_FAIL;
			}
			if(FAILED(getPin(VideoDecoder_,PN_VIDEO_DECODER_IN,&TargetPin)))
			{
				SourcePin->Release ();
				IMPEG2Demux_->DeleteOutputPin (L"Video");
				return E_FAIL;
			}
			hr = GraphBuilder_->Connect (SourcePin,TargetPin);
			SourcePin->Release ();
			TargetPin->Release ();
			if(FAILED(hr))
			{
				IMPEG2Demux_->DeleteOutputPin (L"Video");
				return E_FAIL;
			}
			if(VideoRenderer_ == 0)
				if(FAILED(createIBaseFilterAndAddToGraph(L"Video Renderer",CLSID_VideoRenderer,&VideoRenderer_)))
					return E_FAIL;
			if(FAILED(connectIBaseFilters(VideoDecoder_,PN_VIDEO_DECODER_OUT,VideoRenderer_,PN_VIDEO_RENDERER_IN)))
				return E_FAIL;

			VideoRenderer_->QueryInterface (IID_IVideoWindow,(void **)&VideoWindow_);
			
			*Filter = VideoFilter_ = new AVPidFilter(Pid,PidName);
			static_cast<AVPidFilter *>(VideoFilter_)->setFilter (ISourceFilter_);
			return S_OK;
		}
		default:
			return E_FAIL;
	}

}
//-------------------------------------------------------------------
HRESULT DirectShowTSTarget::togglePlayback()
{
	if(MediaControl_ == 0)
		return E_FAIL;

	HRESULT hr;

	if(running_)
		hr = MediaControl_->Pause ();
	else
		hr = MediaControl_->Run ();

	return hr;
}
//-------------------------------------------------------------------
HRESULT DirectShowTSTarget::toggleVolume()
{
	if(BasicAudio_ == 0)
		return E_FAIL;

	return BasicAudio_->put_Volume (Volume_);

}
//-------------------------------------------------------------------
HRESULT DirectShowTSTarget::toggleFullScreen(HWND hWnd)
{
if(!VideoWindow_)
		return E_FAIL;

	LONG lMode;
	static HWND hDrain=0;
	VideoWindow_->get_FullScreenMode (&lMode);

	if(lMode == OAFALSE)
	{
		VideoWindow_->get_MessageDrain((OAHWND *) &hDrain);

        // Set message drain to application main window
        VideoWindow_->put_MessageDrain((OAHWND) hWnd);

        // Switch to full-screen mode
        lMode = OATRUE;
        VideoWindow_->put_FullScreenMode(lMode);
	}
	else
	{
		lMode = OAFALSE;
        VideoWindow_->put_FullScreenMode(lMode);

        // Undo change of message drain
        VideoWindow_->put_MessageDrain((OAHWND) hDrain);

        // Reset video window
        VideoWindow_->SetWindowForeground(-1);

        // Reclaim keyboard focus for player application
        UpdateWindow(hWnd);
        SetForegroundWindow(hWnd);
        SetFocus(hWnd);   
	}

	return S_OK;

}
//-------------------------------------------------------------------
HRESULT DirectShowTSTarget::getProperties(HINSTANCE hInstOwner, HWND hWndOwner, HWND * Window, unsigned int * CommandId)
{

	return E_NOTIMPL;
}
//-------------------------------------------------------------------
HRESULT DirectShowTSTarget::setVideoProperties(HWND hWnd, RECT rect)
{
	if(VideoWindow_ == 0)
		return E_FAIL;

	if(FAILED(VideoWindow_->put_Owner ((OAHWND) hWnd)))
		return E_FAIL;

	if(FAILED(VideoWindow_->put_WindowStyle(WS_CHILD | WS_CLIPSIBLINGS | WS_CLIPCHILDREN)))
		return E_FAIL;

	return VideoWindow_->SetWindowPosition (rect.left ,rect.top,rect.right, rect.bottom );
}
//-------------------------------------------------------------------
HRESULT DirectShowTSTarget::setTargetFileName(const char * Name)
{

	return E_NOTIMPL;
}
//-------------------------------------------------------------------
HRESULT DirectShowTSTarget::createIBaseFilterAndAddToGraph(wchar_t * filter_name, CLSID filter_id, IBaseFilter ** Filter)
{
int retries = 0;
	HRESULT result;

	if(filter_name == NULL || GraphBuilder_ == NULL)
		return E_FAIL;

	while(retries++ < FILTER_RECREATE_RETRIES)
	{
		result = CoCreateInstance(filter_id,
								  NULL,
								  CLSCTX_INPROC,
								  IID_IBaseFilter,
								  (void **)Filter);
		if(result == S_OK)
		{
			 return GraphBuilder_->AddFilter (*Filter,filter_name);
		}
		
	}
	return E_FAIL;
}
//-------------------------------------------------------------------
HRESULT DirectShowTSTarget::queryInterface(IBaseFilter * Filter, REFIID ID, void ** Interface)
{
	return Filter->QueryInterface (ID,Interface);
}
//-------------------------------------------------------------------
HRESULT DirectShowTSTarget::removeAllFilterFromGraph()
{
	SAFE_RELEASE(ISourceFilter_);
	SAFE_RELEASE(IMPEG2Demux_);
	SAFE_RELEASE(DVBDemux_);
	if(SourceFilter_)
	{
		IPin *SourcePin = 0;
		if(SUCCEEDED(getPin(SourceFilter_,L"Stream 1",&SourcePin)))
		{
			SourcePin->Disconnect ();
			SourcePin->Release ();
		}
		SAFE_RELEASE(SourceFilter_);
	}
	
	SAFE_RELEASE(AudioDecoder_);
	SAFE_RELEASE(VideoDecoder_);
	SAFE_RELEASE(AudioRenderer_);
	SAFE_RELEASE(VideoRenderer_);
	SAFE_RELEASE(VideoWindow_);
	SAFE_RELEASE(BasicAudio_);
	return S_OK;
}
//-------------------------------------------------------------------
HRESULT DirectShowTSTarget::connectIBaseFilters(IBaseFilter * source_filter, wchar_t * source_pin, IBaseFilter * target_filter, wchar_t * target_pin)
{
	IPin * ISourcePin = NULL;
	IPin * ITargetPin = NULL;

	HRESULT result;
	if(source_filter == NULL || target_filter == NULL || GraphBuilder_ == NULL)
		return E_FAIL;

	if(FAILED(getPin(source_filter, source_pin, &ISourcePin)))
		return E_FAIL;

	if(FAILED(getPin(target_filter, target_pin, &ITargetPin)))
	{
		ISourcePin->Release ();
		return E_FAIL;
	}


	result = GraphBuilder_->Connect (ISourcePin, ITargetPin);
	SAFE_RELEASE(ISourcePin);
	SAFE_RELEASE(ITargetPin);

	return result;
}
//-------------------------------------------------------------------
HRESULT DirectShowTSTarget::createMPEG2DemuxPin(wchar_t * PinName, IPin ** Pin, unsigned char PinType)
{
	if(PinName == NULL || IMPEG2Demux_ == NULL)
		return E_FAIL;

	
	AM_MEDIA_TYPE media_type;

	ZeroMemory(&media_type, sizeof(AM_MEDIA_TYPE));
	if(PinType & PID_TYPE_AUDIO)
	{
		media_type.majortype = MEDIATYPE_Audio;
		
		if(PinType & PID_TYPE_AUDIO_MPEG1)
			media_type.subtype = MEDIASUBTYPE_MPEG1Audio;
		else if(PinType & PID_TYPE_AUDIO_MPEG2)
			media_type.subtype = MEDIASUBTYPE_MPEG2_AUDIO;
		else if(PinType & PID_TYPE_AUDIO_AC3)
			media_type.subtype = MEDIASUBTYPE_DOLBY_AC3;
	}
	else if(PinType & PID_TYPE_VIDEO)	
	{
		media_type.majortype = MEDIATYPE_Video;
		
		if(PinType & PID_TYPE_VIDEO_MPEG1)
			media_type.subtype = MEDIASUBTYPE_MPEG1Video;
		else if(PinType & PID_TYPE_VIDEO_MPEG2)
			media_type.subtype = MEDIASUBTYPE_MPEG2_VIDEO;
		else
			return E_FAIL;
	}
	return IMPEG2Demux_->CreateOutputPin(&media_type, PinName, Pin);
}
//-------------------------------------------------------------------
HRESULT DirectShowTSTarget::mapMPEG2DemuxPin(IPin * Pin, unsigned short Pid, MEDIA_SAMPLE_CONTENT Content)
{
	if(Pin == 0)
		return E_FAIL;
	IMPEG2PIDMap *pPidMap = NULL;
	if(FAILED(Pin->QueryInterface(IID_IMPEG2PIDMap, (void**)&pPidMap)))
		return E_FAIL;
	
	HRESULT hr = pPidMap->MapPID (1, (ULONG *)&Pid,  Content);
	pPidMap->Release ();
	
	return hr;
}
//-------------------------------------------------------------------
HRESULT DirectShowTSTarget::getPin(IBaseFilter *filter, wchar_t * pin_name, IPin ** pin)
{
	IEnumPins *pEnumPins = NULL;
	PIN_INFO PinInfo = {0};
	IPin * pCurrentEnumPin = NULL;
	
	if(pin_name == NULL)
		return E_FAIL;


	if(SUCCEEDED(filter->FindPin (pin_name,pin)))
		return S_OK;

	

	if(filter->EnumPins (&pEnumPins) != S_OK)
		return E_FAIL;
	
	pEnumPins->Reset ();
	while(pEnumPins->Next (1,&pCurrentEnumPin,NULL)== S_OK)
	{
		pCurrentEnumPin->QueryPinInfo (&PinInfo);
		if(wcscmp(pin_name,PinInfo.achName ) == 0 )
		{
			*pin = pCurrentEnumPin;
			pCurrentEnumPin->Release (); //TEST	
			PinInfo.pFilter->Release ();
			pEnumPins->Release ();
			return S_OK;
		}
		PinInfo.pFilter->Release ();		
		pCurrentEnumPin->Release ();
	}
	pEnumPins->Release ();
	
	return E_FAIL;
}
//-------------------------------------------------------------------