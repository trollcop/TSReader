// vOC_BL.cpp: implementation of the vOC_BL class.
//
//////////////////////////////////////////////////////////////////////
//#define WINVER 0x501 
#include <streams.h>
#include <olectl.h>
#include <initguid.h>
#include <strmif.h>
#include <dvdmedia.h>
#include <bdaiface.h>
#include <Mmreg.h>

#include <stdio.h>

#include "igenericsat.h"
#include "vOC_BL.h"
#include "Satellite.h"
#include "../CSEQHDR.H"

//#define M(x) MessageBox(NULL, x, "", MB_OK)
 
#define M(x)
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
const AMOVIESETUP_MEDIATYPE sudOpPinTypes =
{
    &MEDIATYPE_Stream,				// Major type
    &MEDIASUBTYPE_MPEG2_TRANSPORT     // Minor type
};

const AMOVIESETUP_PIN sudOpPin =
{
    L"Output",              // Pin string name
    FALSE,                  // Is it rendered
    TRUE,                   // Is it an output
    FALSE,                  // Can we have none
    FALSE,                  // Can we have many
    &CLSID_NULL,            // Connects to filter
    NULL,                   // Connects to pin
    1,                      // Number of types
    &sudOpPinTypes };       // Pin details

const AMOVIESETUP_FILTER sudvOC_BLax =
{
    &CLSID_vOC_BL,    // Filter CLSID
    L"vOC BL DVB Source",       // String name
    MERIT_DO_NOT_USE,       // Filter merit
    1,                      // Number pins
    &sudOpPin               // Pin details
};


// COM global table of objects in this dll

CFactoryTemplate g_Templates[] = {
  { L"vOC BL DVB Source"
  , &CLSID_vOC_BL
  , vOC_BL::CreateInstance
  , NULL
  , &sudvOC_BLax }
};
int g_cTemplates = sizeof(g_Templates) / sizeof(g_Templates[0]);


//
// DllRegisterServer
//
// Exported entry points for registration and unregistration
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


//
// CreateInstance
//
// The only allowed way to create this filter
//
CUnknown * WINAPI vOC_BL::CreateInstance(LPUNKNOWN lpunk, HRESULT *phr)
{
    CUnknown *punk = new vOC_BL(lpunk, phr);
    if (punk == NULL)
        *phr = E_OUTOFMEMORY;
    return punk;

} // CreateInstance

//
// Constructor
//
// Initialise a vOC_BL object so that we have a pin.
//
vOC_BL::vOC_BL(LPUNKNOWN lpunk, HRESULT *phr) : CSource(NAME("vOC BL DVB Source"), lpunk, CLSID_vOC_BL)
{
    CAutoLock cAutoLock(&m_cStateLock);

    m_paStreams    = (CSourceStream **) new CvOC_BLStream*[1];
    if (m_paStreams == NULL)
	{
        *phr = E_OUTOFMEMORY;
		MessageBox(NULL, "Constructor error 1", "Error", MB_OK);
		return;
    }

    m_paStreams[0] = new CvOC_BLStream(phr, this, L"vOC BL DVB Source");
    if (m_paStreams[0] == NULL)
	{
        *phr = E_OUTOFMEMORY;
		MessageBox(NULL, "Constructor error 2", "Error", MB_OK);
		return;
    }

} // (Constructor)

vOC_BL::~vOC_BL()
{
	OutputDebugString("*********** vOC_BL::~vOC_BL() ***********\n");
	int a=1;
}

//
// Constructor
//
CvOC_BLStream::CvOC_BLStream(HRESULT *phr, vOC_BL *pParent, LPCWSTR pPinName) : CSourceStream(NAME("PS Stream"),phr, pParent, pPinName)
{
	CAutoLock cAutoLock(&m_cSharedState);

	m_Satellite = new Satellite();
	if (m_Satellite == NULL)
		*phr = E_OUTOFMEMORY;


} // (Constructor)


//
// Destructor
//
CvOC_BLStream::~CvOC_BLStream()
{
	OutputDebugString("*********** CvOC_BLStream::~CvOC_BLStream ***********\n");

    CAutoLock cAutoLock(&m_cSharedState);
	if (m_Satellite)
		delete m_Satellite;

} // (Destructor)


//
// FillBuffer
//
// Plots a ball into the supplied video buffer
//
HRESULT CvOC_BLStream::FillBuffer(IMediaSample *pms)
{
    BYTE *pData;
    long lDataLen;
	long len;

	//CAutoLock cAutoLockShared(&m_cSharedState);

	pms->GetPointer(&pData);
	lDataLen=pms->GetSize();
	len=m_Satellite->GetData((char *)pData, lDataLen);
	if (len!=lDataLen)
		pms->SetActualDataLength(len);
	
    return NOERROR;

} // FillBuffer


//
// Notify
//
// Alter the repeat rate according to quality management messages sent from
// the downstream filter (often the renderer).  Wind it up or down according
// to the flooding level - also skip forward if we are notified of Late-ness
//
STDMETHODIMP CvOC_BLStream::Notify(IBaseFilter * pSender, Quality q)
{
    return NOERROR;

} // Notify


//
// GetMediaType
//
// I _prefer_ 5 formats - 8, 16 (*2), 24 or 32 bits per pixel and
// I will suggest these with an image size of 320x240. However
// I can accept any image size which gives me some space to bounce.
//
// A bit of fun:
//      8 bit displays get red balls
//      16 bit displays get blue
//      24 bit see green
//      And 32 bit see yellow
//
// Prefered types should be ordered by quality, zero as highest quality
// Therefore iPosition =
// 0	return a 32bit mediatype
// 1	return a 24bit mediatype
// 2	return 16bit RGB565
// 3	return a 16bit mediatype (rgb555)
// 4	return 8 bit palettised format
// (iPosition > 4 is invalid)
//
HRESULT CvOC_BLStream::GetMediaType(int iPosition, CMediaType *pmt)
{
    CAutoLock cAutoLock(m_pFilter->pStateLock());
    if (iPosition < 0)
        return E_INVALIDARG;

    // Have we run off the end of types
	if (iPosition > 0)
        return VFW_S_NO_MORE_ITEMS;

	pmt->bFixedSizeSamples = TRUE;
    pmt->SetTemporalCompression(FALSE);
	pmt->SetType(&MEDIATYPE_Stream);
    pmt->SetSubtype( &MEDIASUBTYPE_MPEG2_TRANSPORT );
    pmt->SetFormatType(&FORMAT_None);
    pmt->SetSampleSize(31100);


    return NOERROR;

} // GetMediaType


//
// CheckMediaType
//
// Accept anything.  If it does not work, it is probably used wrong
HRESULT CvOC_BLStream::CheckMediaType(const CMediaType *pMediaType)
{
	return S_OK;
} // CheckMediaType


//
// DecideBufferSize
//
// This will always be called after the format has been sucessfully
// negotiated. So we have a look at m_mt to see what size image we agreed.
// Then we can ask for buffers of the correct size to contain them.
//

HRESULT CvOC_BLStream::DecideBufferSize(IMemAllocator *pAlloc,ALLOCATOR_PROPERTIES *pProperties)
{
    CAutoLock cAutoLock(m_pFilter->pStateLock());

    ASSERT(pAlloc);
    ASSERT(pProperties);
    HRESULT hr = NOERROR;

    pProperties->cBuffers = 16;
    pProperties->cbBuffer = PACKETBUFFER;

    ASSERT(pProperties->cbBuffer);

    // Ask the allocator to reserve us some sample memory, NOTE the function
    // can succeed (that is return NOERROR) but still not have allocated the
    // memory that we requested, so we must check we got whatever we wanted

    ALLOCATOR_PROPERTIES Actual;
    hr = pAlloc->SetProperties(pProperties,&Actual);
    if (FAILED(hr))
        return hr;

    // Is this allocator unsuitable

    if (Actual.cbBuffer < pProperties->cbBuffer)
        return E_FAIL;

    // Make sure that we have only 1 buffer (we erase the ball in the
    // old buffer to save having to zero a 200k+ buffer every time
    // we draw a frame)

    //ASSERT( Actual.cBuffers == 1 );
    return NOERROR;

} // DecideBufferSize


//
// SetMediaType
//
// Called when a media type is agreed between filters
//
HRESULT CvOC_BLStream::SetMediaType(const CMediaType *pMediaType)
{
    CAutoLock cAutoLock(m_pFilter->pStateLock());

    // Pass the call up to my base class
    HRESULT hr = CSourceStream::SetMediaType(pMediaType);

    return hr;

} // SetMediaType

//
// OnThreadCreate
//
// As we go active reset the stream time to zero
//
HRESULT CvOC_BLStream::OnThreadCreate()
{
    CAutoLock cAutoLockShared(&m_cSharedState);
	OutputDebugString("OnThreadCreate\n");
	m_Satellite->Init();
    return NOERROR;

} // OnThreadCreate

HRESULT CvOC_BLStream::OnThreadDestroy()
{
    CAutoLock cAutoLockShared(&m_cSharedState);
	m_Satellite->DeInit();
    return NOERROR;

} // OnThreadCreate



HRESULT CvOC_BLStream::CompleteConnect(IPin *pReceivePin)
{
	HRESULT res = CSourceStream::CompleteConnect(pReceivePin);
	PIN_INFO pin_info;
	IBaseFilter *pFilter=NULL;
	IMpeg2Demultiplexer *pDemux=NULL;
	CLSID clsid=GUID_NULL;
	HRESULT hr;
	IPin *videopin=NULL;
	IPin *audiopin=NULL;
	IPin *ac3pin=NULL;

return res;
    CAutoLock cAutoLock(m_pFilter->pStateLock());

	pin_info.pFilter = NULL;
	CMediaType *video = CreateMPEG2VideoType();
	CMediaType *audio = CreateMPEG1AudioType();
	CMediaType *ac3   = CreateAC3AudioType();
	if (video==NULL)
	{
		MessageBox(NULL, "video format is null", "", MB_OK);
		return res;
	}
	if (audio==NULL)
	{
		MessageBox(NULL, "audio format is null", "", MB_OK);
		return res;
	}
	if (ac3==NULL)
	{
		MessageBox(NULL, "ac3 format is null", "", MB_OK);
		return res;
	}

	if (res==NOERROR)
	{
		hr = pReceivePin->QueryPinInfo(&pin_info);
		if (hr==S_OK)
		{
			pFilter = pin_info.pFilter;
		} 
		else
		{
			return res;
		}
		if (pFilter)
		{
			pFilter->GetClassID(&clsid);
			if (clsid==CLSID_MPEG2Demultiplexer)
			{
				hr = pFilter->QueryInterface(IID_IMpeg2Demultiplexer, (void**)&pDemux);
				if (!FAILED(hr))
				{
					hr = pDemux->CreateOutputPin((AM_MEDIA_TYPE *)video, L"BL video", &videopin);
					if (FAILED(hr))
					{
						MessageBox(NULL, "Unable to create video pin", "Error", MB_OK);
					}
					if (m_Satellite->ac3) 
					{
						hr = pDemux->CreateOutputPin((AM_MEDIA_TYPE *)ac3, L"BL audio", &audiopin);
					}
					else 
					{
						hr = pDemux->CreateOutputPin((AM_MEDIA_TYPE *)audio, L"BL audio", &audiopin);
					}
	
					if (FAILED(hr))
					{
						MessageBox(NULL, "Unable to create audio pin", "Error", MB_OK);
					}
					else 
					{
						if (audiopin==NULL)
						{
							MessageBox(NULL, "No audio pin", "", MB_OK);
						}
					}
					//pDemux->Release();
				}
			}
			//pReceivePin->Release();
		}
	}
	else
	{
		MessageBox(NULL, "Error connecting", "", MB_OK);
	}

	return res;
}

// creates an MPEG2 media type that the decoder likes.
CMediaType *CvOC_BLStream::CreateMPEG2VideoType(int w, int h, int level, int profile)
{
	CMediaType *mt = new CMediaType(&MEDIATYPE_Video);

	RECT src={0,0,w,h};
	RECT dst={0,0,0,0};

	mt->InitMediaType();
	mt->SetType(&MEDIATYPE_Video);
	mt->SetSubtype(&MEDIASUBTYPE_MPEG2_VIDEO);
	mt->SetFormatType(&FORMAT_MPEG2_VIDEO);

/*
    MPEG2VIDEOINFO *mpeg2vinfo = (MPEG2VIDEOINFO *) mt->AllocFormatBuffer(sizeof(MPEG2VIDEOINFO));
    if (NULL == mpeg2vinfo) {
		return(NULL);
    }
    ZeroMemory(mpeg2vinfo, sizeof(MPEG2VIDEOINFO));

	mpeg2vinfo->hdr.dwBitRate=15000000;
	mpeg2vinfo->hdr.dwBitErrorRate=0;
	mpeg2vinfo->hdr.AvgTimePerFrame=333667;
	mpeg2vinfo->hdr.dwInterlaceFlags=0;
	mpeg2vinfo->hdr.dwCopyProtectFlags=0;
	mpeg2vinfo->hdr.dwPictAspectRatioX=4;
	mpeg2vinfo->hdr.dwPictAspectRatioY=3;
	mpeg2vinfo->hdr.dwControlFlags=0;
	mpeg2vinfo->hdr.dwReserved1=0;
	mpeg2vinfo->hdr.dwReserved2=0;

	mpeg2vinfo->hdr.bmiHeader.biSize=40;
	mpeg2vinfo->hdr.bmiHeader.biWidth=w;
	mpeg2vinfo->hdr.bmiHeader.biHeight=h;
	mpeg2vinfo->hdr.bmiHeader.biPlanes=0;
	mpeg2vinfo->hdr.bmiHeader.biBitCount=0;
	mpeg2vinfo->hdr.bmiHeader.biCompression=0;
	mpeg2vinfo->hdr.bmiHeader.biSizeImage=0;
	mpeg2vinfo->hdr.bmiHeader.biXPelsPerMeter=2000;
	mpeg2vinfo->hdr.bmiHeader.biYPelsPerMeter=53031;
	mpeg2vinfo->hdr.bmiHeader.biClrUsed=0;
	mpeg2vinfo->hdr.bmiHeader.biClrImportant=0;
	mpeg2vinfo->dwStartTimeCode=455832;
	mpeg2vinfo->dwProfile=profile;
	mpeg2vinfo->dwLevel=level;
	mpeg2vinfo->dwFlags=0;

	mpeg2vinfo->dwLevel = level;
	mpeg2vinfo->dwProfile = profile;
	mpeg2vinfo->cbSequenceHeader = 0;

	memcpy(&mpeg2vinfo->hdr.rcSource, &src, sizeof(RECT));
	memcpy(&mpeg2vinfo->hdr.rcTarget, &dst, sizeof(RECT));

//	mt->SetTemporalCompression(FALSE);
//	mt->SetSampleSize(18800);
//	mt->SetVariableSize();


*/
	// Set up the format section of the mediatype to be the right size
	MPEG2VIDEOINFO *pm2vi = (MPEG2VIDEOINFO *) mt->ReallocFormatBuffer(sizeof(MPEG2VIDEOINFO)+MAX_SEQ_HDR_LEN);
	if (pm2vi == NULL) 
		return NULL;

	// make sure all fields are initially zero
	ZeroMemory((void *)pm2vi, sizeof(MPEG2VIDEOINFO)+MAX_SEQ_HDR_LEN);

	pm2vi->dwStartTimeCode = 0L;
	pm2vi->dwProfile = AM_MPEG2Profile_Main;
	pm2vi->dwLevel = AM_MPEG2Level_Main;
	pm2vi->dwFlags = 0L;

	CSeqHdrData *seq_hdr = new CSeqHdrData(TRUE);
	if (m_Satellite->width)
	{
		seq_hdr->horizontal_size = m_Satellite->width; 
	}
	else
	{
		seq_hdr->horizontal_size = 720; // The highest possible resolution for MP@ML
	}
	seq_hdr->vertical_size = 576;
	seq_hdr->bit_rate = (double)15000000;
	seq_hdr->vbv_buffer_size = 229736*8/16384; // size in bytes / 16 kbit
	seq_hdr->check();
	pm2vi->cbSequenceHeader = seq_hdr->create_seq_hdr((uchar*)&pm2vi->dwSequenceHeader);

	pm2vi->hdr.rcSource.top = pm2vi->hdr.rcTarget.top = 0;
	pm2vi->hdr.rcSource.bottom = pm2vi->hdr.rcTarget.bottom = 0;
	pm2vi->hdr.rcSource.left = pm2vi->hdr.rcTarget.left = 0;
	pm2vi->hdr.rcSource.right = pm2vi->hdr.rcTarget.right = 0;

	pm2vi->hdr.dwBitRate = 15000000; // The highest bitrate we can	find in ATSC broadcasts
	pm2vi->hdr.dwBitErrorRate = 0;
	pm2vi->hdr.AvgTimePerFrame = 40000;

	pm2vi->hdr.dwInterlaceFlags = 1; // It is really interlaced. Let decoder decide what to do.
	pm2vi->hdr.dwCopyProtectFlags = 0;
	pm2vi->hdr.dwPictAspectRatioX = 4; // Decoder cares
	pm2vi->hdr.dwPictAspectRatioY = 3;
	pm2vi->hdr.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	pm2vi->hdr.bmiHeader.biWidth = seq_hdr->horizontal_size;
	pm2vi->hdr.bmiHeader.biHeight = seq_hdr->vertical_size;
	pm2vi->hdr.bmiHeader.biPlanes = 1;
	pm2vi->hdr.bmiHeader.biBitCount = 8;
	pm2vi->hdr.bmiHeader.biCompression = 0;
	pm2vi->hdr.bmiHeader.biSizeImage = 0;
	pm2vi->hdr.bmiHeader.biXPelsPerMeter = 0;
	pm2vi->hdr.bmiHeader.biYPelsPerMeter = 0;
	pm2vi->hdr.bmiHeader.biClrUsed = 0;
	pm2vi->hdr.bmiHeader.biClrImportant = 0;

	return mt;
}
/*
CMediaType * CvOC_BLStream::CreateMPEG1AudioType()
{
	CMediaType *mt = new CMediaType(&MEDIATYPE_Audio);

	mt->SetSubtype(&MEDIASUBTYPE_MPEG1Payload);
	mt->SetFormatType(&FORMAT_WaveFormatEx);

    MPEG1WAVEFORMAT *wave = (MPEG1WAVEFORMAT *) mt->AllocFormatBuffer(sizeof(MPEG1WAVEFORMAT));
    if (NULL == wave) {
		return(NULL);
    }
    ZeroMemory(wave, sizeof(MPEG1WAVEFORMAT));
	wave->wfx.cbSize = 22;
	wave->wfx.wFormatTag = WAVE_FORMAT_MPEG;
	wave->wfx.nSamplesPerSec = 48000;
	wave->wfx.nChannels = 2;
//	wave->wfx.nAvgBytesPerSec = 64000;
//	wave->wfx.nBlockAlign = 256;
//	wave->wfx.wBitsPerSample = 0;
//	wave->dwHeadBitrate = 256000;
	wave->fwHeadLayer = ACM_MPEG_LAYER2;
	wave->fwHeadMode = ACM_MPEG_STEREO;


	mt->SetTemporalCompression(FALSE);
	mt->SetSampleSize(0);

	return mt;
}
*/
CMediaType * CvOC_BLStream::CreateMPEG1AudioType()
{
	CMediaType *mt = new CMediaType(&MEDIATYPE_Audio);

	//mt->SetSubtype(&MEDIASUBTYPE_DOLBY_AC3);
	mt->SetSubtype(&MEDIASUBTYPE_MPEG1Payload);
	mt->SetFormatType(&FORMAT_WaveFormatEx);

    MPEG1WAVEFORMAT *wave = (MPEG1WAVEFORMAT *) mt->AllocFormatBuffer(sizeof(MPEG1WAVEFORMAT));
    if (NULL == wave)
		return(NULL);

    ZeroMemory(wave, sizeof(MPEG1WAVEFORMAT));
	wave->wfx.cbSize = 22;
	wave->wfx.wFormatTag = WAVE_FORMAT_MPEG;
	wave->wfx.nSamplesPerSec = 48000; //48000;
	if (m_Satellite->mono)
		wave->wfx.nChannels = 1;
	else
		wave->wfx.nChannels = 2;
//	wave->wfx.nAvgBytesPerSec = 64000;
//	wave->wfx.nBlockAlign = 256;
//	wave->wfx.wBitsPerSample = 0;
//	wave->dwHeadBitrate = 256000;
	wave->fwHeadLayer = ACM_MPEG_LAYER3;
	wave->fwHeadMode = ACM_MPEG_STEREO;
	//mt->SetTemporalCompression(FALSE);
	//mt->SetSampleSize(0);

	return mt;
}


STDMETHODIMP vOC_BL::NonDelegatingQueryInterface(REFIID riid, void **ppv)
{
    CheckPointer(ppv,E_POINTER);

    if (riid == IID_IGenericSat)
        return GetInterface((IGenericSat *) this, ppv);
	else
        return CSource::NonDelegatingQueryInterface(riid, ppv);
}

STDMETHODIMP vOC_BL::SetVPID(unsigned short pid)
{
	CvOC_BLStream *p=(CvOC_BLStream *)m_paStreams[0];
	return p->m_Satellite->SetVPID(pid);

}

STDMETHODIMP vOC_BL::SetAPID(unsigned short pid)
{
	CvOC_BLStream *p=(CvOC_BLStream *)m_paStreams[0];
	return p->m_Satellite->SetAPID(pid);

}

STDMETHODIMP vOC_BL::SetPCR(unsigned short pid)
{
	CvOC_BLStream *p=(CvOC_BLStream *)m_paStreams[0];
	return p->m_Satellite->SetPCR(pid);

}

STDMETHODIMP vOC_BL::SetPipe(HANDLE hDataPipe)
{
	CvOC_BLStream *p=(CvOC_BLStream *)m_paStreams[0];
	return p->m_Satellite->SetPipe(hDataPipe);
}

CMediaType * CvOC_BLStream::CreateAC3AudioType()
{
	CMediaType *mt = new CMediaType(&MEDIATYPE_Audio);

	mt->SetSubtype(&MEDIASUBTYPE_DOLBY_AC3);
//	mt->SetSubtype(&MEDIASUBTYPE_MPEG1Payload);
	mt->SetFormatType(&FORMAT_WaveFormatEx);

    MPEG1WAVEFORMAT *wave = (MPEG1WAVEFORMAT *) mt->AllocFormatBuffer(sizeof(MPEG1WAVEFORMAT));
    if (NULL == wave)
		return(NULL);

    ZeroMemory(wave, sizeof(MPEG1WAVEFORMAT));
	wave->wfx.cbSize = 22;
	wave->wfx.wFormatTag = WAVE_FORMAT_MPEG;
	wave->wfx.nSamplesPerSec = 48000;
	wave->wfx.nChannels = 2;
//	wave->wfx.nAvgBytesPerSec = 64000;
//	wave->wfx.nBlockAlign = 256;
//	wave->wfx.wBitsPerSample = 0;
//	wave->dwHeadBitrate = 256000;

	wave->fwHeadLayer = ACM_MPEG_LAYER3;
	wave->fwHeadMode = ACM_MPEG_STEREO;


	//mt->SetTemporalCompression(FALSE);
	//mt->SetSampleSize(0);

	return mt;

}

STDMETHODIMP vOC_BL::SetAC3(unsigned short flag)
{
	CvOC_BLStream *p=(CvOC_BLStream *)m_paStreams[0];
	return p->m_Satellite->SetAC3(flag);

}

STDMETHODIMP vOC_BL::put_Parameter(unsigned short id, short val)
{
	CvOC_BLStream *p=(CvOC_BLStream *)m_paStreams[0];
	return p->m_Satellite->put_Parameter(id, val);

}

STDMETHODIMP vOC_BL::get_Parameter(unsigned short id, short *val)
{
	CvOC_BLStream *p=(CvOC_BLStream *)m_paStreams[0];
	return p->m_Satellite->get_Parameter(id, val);

}


HRESULT CvOC_BLStream::Run()
{
	return CSourceStream::Run();
}

STDMETHODIMP vOC_BL::put_String(unsigned short id, LPSTR val)
{
	CvOC_BLStream *p=(CvOC_BLStream *)m_paStreams[0];
	return p->m_Satellite->put_String(id, val);
}
