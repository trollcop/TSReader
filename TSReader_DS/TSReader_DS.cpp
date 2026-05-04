#include <windows.h>
#include <commctrl.h>

#include <dshow.h>
#include <commctrl.h>
#include <commdlg.h>
#include <stdio.h>
#include <tchar.h>
#include <atlbase.h>
#include <bdaiface.h>
#include <dvdmedia.h>
#include <mmreg.h>

#include <Streams.h>

#include "imvdc.h"
#include "../cseqhdr.h"
#include "../TSReader.h"
#include "../resource.h"

PVARIABLES v;
#define STRADIS_READ_SIZE 188 * 512

BOOL bMono = FALSE;
BOOL bAC3 = FALSE;
BOOL bNoMixer = FALSE;
BOOL bLowRes = FALSE;
int nVideoWidth = 720;
HWND ghApp;
double fPlaybackRate;
unsigned short topoffset = 0;
enum PLAYSTATE {Stopped, Paused, Running, Init};
PLAYSTATE g_psCurrent = Stopped;
char gszAppName[] = {"TSReader DirectShow Interface"};

#define JIF(x) if (FAILED(hr=(x))) \
    {char szTemp[100]; wsprintf(szTemp, TEXT("FAILED(hr=0x%x) in ") TEXT(#x) TEXT("\n"), hr); OutputDebugString(szTemp); return hr;}
#define SAFE_RELEASE(x) { if (x) x->Release(); x = NULL; }
#define WM_GRAPHNOTIFY  WM_USER+13
#define DEFAULT_AUDIO_WIDTH     240
#define DEFAULT_AUDIO_HEIGHT    120

const CLSID CLSID_vOC_BL = {0x4fccc8de, 0x433a, 0x469e, 0xb7, 0xde, 0xb7, 0xe3, 0xab, 0x62, 0x28, 0x65};
DEFINE_GUID(IID_IGenericSat, 0xf8f1b7e5, 0x400b, 0x4c15, 0xb7, 0x77, 0xe1, 0x84, 0x73, 0x6, 0x1, 0x57);
const GUID IID_IGenericSat = {0xf8f1b7e5, 0x400b, 0x4c15, 0xb7, 0x77, 0xe1, 0x84, 0x73, 0x6, 0x1, 0x57};
const CLSID CLSID_MoonDecoder = {0xE32C3B01, 0xC81B, 0x4D01, 0x8A, 0xD4, 0x2B, 0x93, 0xF7, 0xFA,0x54, 0x4C};
const GUID IID_IMPEGVideoDecoderControl = {0xaa4e6d3b, 0x68f9, 0x4ca8, 0xaa, 0xbe, 0x22, 0x87, 0xa5, 0x29, 0x15, 0xb4};
const CLSID CLSID_Elecard_Decoder = {0xF50B3F13, 0x19C4, 0x11CF, 0xAA, 0x9A, 0x02, 0x60, 0x8C, 0x9B, 0xAB, 0xA2};
const CLSID CLSID_Sigma = {0xCB51EFC2, 0x40D6, 0x11D3, 0xB2, 0x65, 0x00, 0xA0, 0xC9, 0xA3, 0xA5, 0x6F};
const CLSID CLSID_Stradis = {0x8AF05226, 0xA5AB, 0x46F5, 0xA5, 0x2F, 0xDA, 0x1A, 0x5B, 0xC4, 0x66, 0xEC};

    MIDL_INTERFACE("f8f1b7e5-400b-4c15-b777-e18473060157")
    IGenericSat : public IUnknown
    {
    public:
        BEGIN_INTERFACE
		virtual HRESULT STDMETHODCALLTYPE SetPipe(
			HANDLE hDataPipe) = 0;	
		virtual HRESULT STDMETHODCALLTYPE SetVPID(
			unsigned short pid) = 0;
		virtual HRESULT STDMETHODCALLTYPE SetAPID(
			unsigned short pid) = 0;
		virtual HRESULT STDMETHODCALLTYPE SetPCR(
			unsigned short pid) = 0;
		virtual HRESULT STDMETHODCALLTYPE SetAC3(
			unsigned short flag) = 0;
		virtual HRESULT STDMETHODCALLTYPE set_Parameter(
			unsigned short id,
			short val) = 0;
		virtual HRESULT STDMETHODCALLTYPE set_String(
			unsigned short id,
			LPSTR val) = 0;
		virtual HRESULT STDMETHODCALLTYPE get_Parameter(
			unsigned short id,
			short *val) = 0;
		END_INTERFACE
	};

// DirectShow interfaces
IGraphBuilder *pGB = NULL;
IMediaControl *pMC = NULL;
IMediaEventEx *pME = NULL;
IVideoWindow  *pVW = NULL;
IBasicAudio   *pBA = NULL;
IBasicVideo   *pBV = NULL;
IMediaSeeking *pMS = NULL;
IMediaPosition *pMP = NULL;
IVideoFrameStep *pFS = NULL;
IGenericSat     *pGS = NULL;
IReferenceClock *pRC = NULL;
IMPEGVideoDecoderControl *pMDC = NULL;
IDirectDrawVideo *pDDV = NULL;

IBaseFilter *pBL = NULL;   // BL source filter
IBaseFilter *pMD = NULL;   // Moon decoder
IBaseFilter *pDM = NULL;   // Mpeg2 demultiplexer
IBaseFilter *pOM = NULL;   // Overlay Mixer
IBaseFilter *pEMD = NULL;  // Elecard decoder

void ShowFilters(HWND hWnd)
{
    IEnumFilters *pEnum = NULL;
    IBaseFilter *pFilter;
    ULONG cFetched;
	char result[500]="";

	if (pGB == NULL)
	{
		MessageBox(hWnd, "No valid Filter Graph", gszAppName, MB_OK);
		return;
	}
    pGB->EnumFilters(&pEnum);
    while(pEnum->Next(1, &pFilter, &cFetched) == S_OK)
    {
        FILTER_INFO FilterInfo;
        char szName[256];
        
        pFilter->QueryFilterInfo(&FilterInfo);
        WideCharToMultiByte(CP_ACP, 0, FilterInfo.achName, -1, szName, 256, 0, 0);
		strcat(result, szName);
		strcat(result, "\n");

        FilterInfo.pGraph->Release();
        pFilter->Release();
    }
    pEnum->Release();
	MessageBox(hWnd, result, gszAppName, MB_OK);
}

HRESULT CreateFilter(REFCLSID clsid, IBaseFilter **ppFilter, char *s)
{
    HRESULT hr;
    hr = CoCreateInstance(clsid, NULL, CLSCTX_INPROC_SERVER, IID_IBaseFilter, (void **) ppFilter);
    if(FAILED(hr))
    {
		char szTemp[100];
        wsprintf(szTemp, "CreateFilter: Failed to create filter (%s)!  hr=0x%x\n", s, hr);
		OutputDebugString(szTemp);
        if (ppFilter)
            *ppFilter = NULL;
        return hr;
    }

    return S_OK;
}

// creates an MPEG2 media type that the decoder likes.
CMediaType *CreateMPEG2VideoType(int w, int h, int level, int profile)
{
	CMediaType *mt = new CMediaType(&MEDIATYPE_Video);

//	RECT src={0,0,w,h};
//	RECT dst={0,0,0,0};

	mt->InitMediaType();
	mt->SetType(&MEDIATYPE_Video);
	mt->SetSubtype(&MEDIASUBTYPE_MPEG2_VIDEO);
	mt->SetFormatType(&FORMAT_MPEG2_VIDEO);

	// Set up the format section of the mediatype to be the right size
	MPEG2VIDEOINFO *pm2vi = (MPEG2VIDEOINFO *) mt->ReallocFormatBuffer(sizeof(MPEG2VIDEOINFO) + MAX_SEQ_HDR_LEN);
	if (pm2vi == NULL)
	{
		return NULL;
	}

	// make sure all fields are initially zero
	ZeroMemory((void *)pm2vi, sizeof(MPEG2VIDEOINFO)+MAX_SEQ_HDR_LEN);

	pm2vi->dwStartTimeCode = 0L;
	pm2vi->dwProfile = AM_MPEG2Profile_Main;
	pm2vi->dwLevel = AM_MPEG2Level_Main;
	pm2vi->dwFlags = 0L;

	CSeqHdrData *seq_hdr = new CSeqHdrData(TRUE);
	if (nVideoWidth)
	{
		seq_hdr->horizontal_size = nVideoWidth; 
	}
	else
	{
		seq_hdr->horizontal_size = 720; // The highest possible resolution for MP@ML
	}
	seq_hdr->vertical_size = 480;
	seq_hdr->bit_rate = (double)15000000;
	seq_hdr->vbv_buffer_size = 229736*8/16384; // size in bytes / 16 kbit
	//seq_hdr->aspectratio = 2;
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

CMediaType * CreateMPEG1AudioType()
{
	CMediaType *mt = new CMediaType(&MEDIATYPE_Audio);

	mt->SetSubtype(&MEDIASUBTYPE_MPEG1Payload);
	mt->SetFormatType(&FORMAT_WaveFormatEx);

    MPEG1WAVEFORMAT *wave = (MPEG1WAVEFORMAT *) mt->AllocFormatBuffer(sizeof(MPEG1WAVEFORMAT));
    if (NULL == wave)
	{
		return(NULL);
    }
    ZeroMemory(wave, sizeof(MPEG1WAVEFORMAT));
	wave->wfx.cbSize = 22;
	wave->wfx.wFormatTag = WAVE_FORMAT_MPEG;
	wave->wfx.nSamplesPerSec = 48000; //48000;
	if (bMono)
		wave->wfx.nChannels = 1;
	else
		wave->wfx.nChannels = 2;

	wave->fwHeadLayer = ACM_MPEG_LAYER3;
	wave->fwHeadMode = ACM_MPEG_STEREO;

	return mt;
}

CMediaType * CreateAC3AudioType()
{
	CMediaType *mt = new CMediaType(&MEDIATYPE_Audio);

	mt->SetSubtype(&MEDIASUBTYPE_DOLBY_AC3);
	mt->SetFormatType(&FORMAT_WaveFormatEx);

    MPEG1WAVEFORMAT *wave = (MPEG1WAVEFORMAT *) mt->AllocFormatBuffer(sizeof(MPEG1WAVEFORMAT));
    if (NULL == wave)
		return(NULL);

    ZeroMemory(wave, sizeof(MPEG1WAVEFORMAT));
	wave->wfx.cbSize = 22;
	wave->wfx.wFormatTag = WAVE_FORMAT_MPEG;
	wave->wfx.nSamplesPerSec = 48000;
	wave->wfx.nChannels = 2;

	wave->fwHeadLayer = ACM_MPEG_LAYER3;
	wave->fwHeadMode = ACM_MPEG_STEREO;

	return mt;

}


BOOL MakePins()
{
	IMpeg2Demultiplexer *pDemux = NULL;
	HRESULT hr;
	IPin *videopin=NULL;
	IPin *audiopin=NULL;
	IPin *ac3pin=NULL;

	CMediaType *video = CreateMPEG2VideoType(720, 480, 2, 2);
	CMediaType *audio = CreateMPEG1AudioType();
	CMediaType *ac3   = CreateAC3AudioType();
	if (video==NULL)
	{
		MessageBox(NULL, "video format is null", "", MB_OK);
		return FALSE;
	}
	if (audio==NULL)
	{
		MessageBox(NULL, "audio format is null", "", MB_OK);
		return FALSE;
	}
	if (ac3==NULL)
	{
		MessageBox(NULL, "ac3 format is null", "", MB_OK);
		return FALSE;
	}

	if (pDM)
	{
		hr = pDM->QueryInterface(IID_IMpeg2Demultiplexer, (void**)&pDemux);
		if (!FAILED(hr))
		{
			hr = pDemux->CreateOutputPin((AM_MEDIA_TYPE *)video, L"BL video", &videopin);
			if (FAILED(hr))
			{
				MessageBox(NULL, "Unable to create video pin", "Error", MB_OK);
			}
			if (bAC3)
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
		//pReceivePin->Release();
	}

	return TRUE;
}

HRESULT InitVideoWindow(int nMultiplier, int nDivider)
{
    int nTitleHeight  = GetSystemMetrics(SM_CYCAPTION);
    int nBorderWidth  = GetSystemMetrics(SM_CXBORDER);
    int nBorderHeight = GetSystemMetrics(SM_CYBORDER);
    HRESULT hr = S_OK;
    LONG lHeight, lWidth;
    RECT rect;

	if (!pBV)
        return S_OK;

    // Read the default video size
    hr = pBV->GetVideoSize(&lWidth, &lHeight);
    if (hr == E_NOINTERFACE)
        return S_OK;

    // Account for requests of normal, half, or double size
    lWidth  = lWidth  * nMultiplier / nDivider;
    lHeight = lHeight * nMultiplier / nDivider;
    SetWindowPos(ghApp, NULL, 0, 0, lWidth, lHeight, SWP_NOMOVE | SWP_NOOWNERZORDER);

    // Account for size of title bar and borders for exact match
    // of window client area to default video size
    //SetWindowPos(ghApp, NULL, 0, 0, lWidth + 2*nBorderWidth,
    //        lHeight + nTitleHeight + 2*nBorderHeight,
    //        SWP_NOMOVE | SWP_NOOWNERZORDER);

    GetClientRect(ghApp, &rect);
    JIF(pVW->SetWindowPosition(rect.left, rect.top, rect.right, rect.bottom));

	if (topoffset)
	{
		pBV->put_DestinationTop(-topoffset);
		LONG dHeight;
		pBV->get_DestinationHeight(&dHeight);
		pBV->put_DestinationHeight(dHeight+topoffset);
	}

    return hr;
}

//
// Some video renderers support stepping media frame by frame with the
// IVideoFrameStep interface.  See the interface documentation for more
// details on frame stepping.
//
BOOL GetFrameStepInterface(void)
{
    HRESULT hr;
    IVideoFrameStep *pFSTest = NULL;

    // Get the frame step interface, if supported
    hr = pGB->QueryInterface(__uuidof(IVideoFrameStep), (PVOID *)&pFSTest);
    if (FAILED(hr))
        return FALSE;

    // Check if this decoder can step
    hr = pFSTest->CanStep(0L, NULL);
    if (hr == S_OK)
    {
        pFS = pFSTest;  // Save interface to global variable for later use
        return TRUE;
    }
    else
    {
        pFSTest->Release();
        return FALSE;
    }
}

HRESULT StartPlayback()
{
    USES_CONVERSION;
    HRESULT hr;
	DWORD DMFlags;

    // Get the interface for DirectShow's GraphBuilder
    JIF(CoCreateInstance(CLSID_FilterGraph, NULL, CLSCTX_INPROC_SERVER,  IID_IGraphBuilder, (void **)&pGB));
    if(SUCCEEDED(hr))
    {
		IPin *pBLOut = NULL;
		IPin *pDemuxIn = NULL;

		JIF(CreateFilter(CLSID_vOC_BL, &pBL, "vOC_BL"));
        JIF(pGB->AddFilter(pBL, L"BL Source"));
		
		JIF(CreateFilter(CLSID_MoonDecoder, &pMD, "Moonlight audio decoder"));
		JIF(pGB->AddFilter(pMD, L"Moonlight audio decoder"));
		
		JIF(CreateFilter(CLSID_MPEG2Demultiplexer, &pDM, "MPEG2 Demux"));
		JIF(pGB->AddFilter(pDM, L"MPEG2 Demux"));
		if (!bNoMixer)
		{
			JIF(CreateFilter(CLSID_OverlayMixer, &pOM, "Overlay Mixer"));
			JIF(pGB->AddFilter(pOM, L"Overlay Mixer"));
		}
		
		JIF(CreateFilter(CLSID_Elecard_Decoder, &pEMD, "Elecard MPEG2 decoder"));
		JIF(pGB->AddFilter(pEMD, L"Elecard MPEG2 decoder"));

		JIF(pEMD->QueryInterface(IID_IMPEGVideoDecoderControl, (void **)&pMDC));
		if (bLowRes)
		{
			DWORD DMFlags;
			pMDC->get_DecodeFlags(&DMFlags);
			DMFlags |= MVDC_DECODE_QUARTER_RESOLUTION;
			pMDC->put_DecodeFlags(DMFlags);
		} 
		pMDC->get_DecodeFlags(&DMFlags);
		DMFlags |= MVDC_DECODE_BOB;
		pMDC->put_DecodeFlags(DMFlags);

        //pGB->SetDefaultSyncSource();
		JIF(pBL->QueryInterface(IID_IGenericSat, (void **)&pGS));
		pGS->SetVPID(v->nRecordVideoPID);
		pGS->SetAPID(v->nRecordAudioPID);
		pGS->SetPCR(v->nRecordPCRPID);

		pGS->SetAC3(bAC3);
		if (bMono)
			pGS->set_Parameter(1, 1);
		if (nVideoWidth)
			pGS->set_Parameter(2, nVideoWidth);
		//pGS->SetTuner(freq,lnbf,sr,pol,f22k);
		pGS->SetPipe(v->hStradisReadPipe);

		MakePins();

		IEnumPins *pEnum=NULL;
		pEnum=NULL;
		pBL->EnumPins(&pEnum);
		pBLOut = NULL;
		if (pEnum)
		{
			PIN_INFO info;

			while(pEnum->Next(1, &pBLOut, 0) == S_OK)
			{
				pBLOut->QueryPinInfo(&info);
				//pGB->Render(pBLOut);
				//pBLOut->Release();
			}
			pEnum->Release();
		}

		pEnum=NULL;
		pDM->EnumPins(&pEnum);
		if (pEnum)
		{
			PIN_INFO info;
			IPin *DMin;

			while(pEnum->Next(1, &DMin, 0) == S_OK)
			{
				DMin->QueryPinInfo(&info);
				if (info.dir==PINDIR_INPUT)
				{
					pBLOut->Connect(DMin, NULL);
				}
			}
			pEnum->Release();
		}
		pBLOut->Release();
		pDM->Release();

		pEnum=NULL;
		pDM->EnumPins(&pEnum);
		if (pEnum)
		{
			PIN_INFO info;
			IPin *pin;
			ULONG pid;
			IMPEG2PIDMap *pMap=NULL; 

			while(pEnum->Next(1, &pin, 0) == S_OK)
			{
				pid = 0L;
				pin->QueryPinInfo(&info);
				if (info.dir==PINDIR_OUTPUT)
				{
					if (wcscmp(info.achName, L"BL video")==0)
					{
						pid = v->nRecordVideoPID;
					}
					else
					{
						pid = v->nRecordAudioPID;
					}
					hr = pin->QueryInterface(IID_IMPEG2PIDMap, (void **)&pMap);
					if (!FAILED(hr))
					{
						hr = pMap->MapPID(1, &pid, MEDIA_ELEMENTARY_STREAM); 
						if (FAILED(hr))
						{
							MessageBox(NULL, "pid map failed", "Error", MB_OK);
						}
						pMap->Release();
					}				
					pGB->Render(pin);
				}
				pin->Release();
			}
			pEnum->Release();
		}

		// QueryInterface for DirectShow interfaces
        JIF(pGB->QueryInterface(IID_IMediaControl, (void **)&pMC));
        JIF(pGB->QueryInterface(IID_IMediaEventEx, (void **)&pME));
        JIF(pGB->QueryInterface(IID_IMediaSeeking, (void **)&pMS));
        JIF(pGB->QueryInterface(IID_IMediaPosition, (void **)&pMP));
		
        // Query for video interfaces, which may not be relevant for audio files
        JIF(pGB->QueryInterface(IID_IVideoWindow, (void **)&pVW));
        JIF(pGB->QueryInterface(IID_IBasicVideo, (void **)&pBV));

        // Query for audio interfaces, which may not be relevant for video-only files
        JIF(pGB->QueryInterface(IID_IBasicAudio, (void **)&pBA));

		// Get the reference clock interface from the MPEG2 Demultiplexer
		//JIF(pDM->QueryInterface(IID_IReferenceClock, (void **)&pRC));
		//pDM->SetSyncSource(pRC);

        // Have the graph signal event via window callbacks for performance
        JIF(pME->SetNotifyWindow((OAHWND)ghApp, WM_GRAPHNOTIFY, 0));

		// Find the elecard interface
        JIF(pVW->put_Owner((OAHWND)ghApp));
        JIF(pVW->put_WindowStyle(WS_CHILD | WS_CLIPSIBLINGS | WS_CLIPCHILDREN));

        JIF(InitVideoWindow(1, 1));
        GetFrameStepInterface();

        // Let's get ready to rumble!
        //CheckSizeMenu(ID_FILE_SIZE_NORMAL);
        ShowWindow(ghApp, SW_SHOWNORMAL);
        UpdateWindow(ghApp);
        SetForegroundWindow(ghApp);
        SetFocus(ghApp);
        fPlaybackRate = 1.0;

#ifdef REGISTER_FILTERGRAPH
        hr = AddGraphToRot(pGB, &g_dwGraphRegister);
        if (FAILED(hr))
        {
            Msg(TEXT("Failed to register filter graph with ROT!  hr=0x%x"), hr);
            g_dwGraphRegister = 0;
        }
#endif

        // Run the graph to play the media file
        JIF(pMC->Run());
        g_psCurrent=Running;

        SetFocus(ghApp);
		//JIF(pMC->Pause());
		//Sleep(2000);
		//JIF(pMC->Run());
    }

    return hr;
}

void CloseInterfaces(void)
{
    HRESULT hr;

    // Relinquish ownership (IMPORTANT!) after hiding video window
    if(pVW)
    {
        hr = pVW->put_Visible(OAFALSE);
        hr = pVW->put_Owner(NULL);
    }

    // Disable event callbacks
    if (pME)
        hr = pME->SetNotifyWindow((OAHWND)NULL, 0, 0);

#ifdef REGISTER_FILTERGRAPH
    if (g_dwGraphRegister)
    {
        RemoveGraphFromRot(g_dwGraphRegister);
        g_dwGraphRegister = 0;
    }
#endif

	pGB->RemoveFilter(pBL);
	pGB->RemoveFilter(pOM);
	pGB->RemoveFilter(pEMD);
	pGB->RemoveFilter(pMD);
	pGB->RemoveFilter(pDM);
	if (!bNoMixer)
		pGB->RemoveFilter(pOM);

    // Release and zero DirectShow interfaces
	SAFE_RELEASE(pDDV);
	SAFE_RELEASE(pMDC);
	SAFE_RELEASE(pEMD);
	SAFE_RELEASE(pOM);
	SAFE_RELEASE(pRC);
	SAFE_RELEASE(pMD);
	SAFE_RELEASE(pDM);
	SAFE_RELEASE(pGS);
    SAFE_RELEASE(pME);
    SAFE_RELEASE(pMS);
    SAFE_RELEASE(pMP);
    SAFE_RELEASE(pMC);
    SAFE_RELEASE(pBA);
    SAFE_RELEASE(pBV);
    SAFE_RELEASE(pVW);
    SAFE_RELEASE(pFS);
    SAFE_RELEASE(pGB);

	SAFE_RELEASE(pBL);

IGraphBuilder *pGB = NULL;


}

void CloseClip()
{
    HRESULT hr;

    // Stop media playback
    if(pMC)
        hr = pMC->Stop();

    // Clear global flags
    g_psCurrent = Stopped;
	if (bLowRes && pMDC)
	{
		DWORD DMFlags;
		pMDC->get_DecodeFlags(&DMFlags);
		DMFlags = MVDC_DECODE_FULL_RESOLUTION;
		pMDC->put_DecodeFlags(DMFlags);
	}
    // Free DirectShow interfaces
    CloseInterfaces();

    // No current media state
    g_psCurrent = Init;

    // Reset the player window
    DestroyWindow(ghApp);
}

HRESULT HandleGraphEvent(void)
{
    LONG evCode, evParam1, evParam2;
    HRESULT hr=S_OK;

    // Make sure that we don't access the media event interface
    // after it has already been released.
    if (!pME)
	    return S_OK;

    // Process all queued events
    while(SUCCEEDED(pME->GetEvent(&evCode, (LONG_PTR *) &evParam1, (LONG_PTR *) &evParam2, 0)))
    {
        // Free memory associated with callback, since we're not using it
        hr = pME->FreeEventParams(evCode, evParam1, evParam2);
	}

    return hr;
}

long FAR PASCAL MainWndProc(HWND hWnd, UINT msg, WPARAM wParam,	LPARAM lParam)
{
	static BOOL fClosingFromEscape = FALSE;

	switch (msg)
	{
	case WM_CREATE:
		{
			break;
		}
	case WM_CLOSE:
		{
			PostMessage(v->hDlgSIParser, WM_COMMAND, ID_PLAYBACK_DIRECTSHOW, 0);
			break;

			//return DefWindowProc(hWnd, msg, wParam, lParam);
		}

	case WM_DESTROY:
		break;

	case WM_CHAR:
		switch(wParam)
		{
		case VK_ESCAPE:
			PostMessage(hWnd, WM_CLOSE, 0, 0);
			break;
		case 'F':
		case 'f':
			ShowFilters(hWnd);
			break;
		}
		break;

	case WM_GRAPHNOTIFY:
		HandleGraphEvent();
		break;

	case WM_ERASEBKGND:
		{
			PAINTSTRUCT ps;
			HDC hDC = BeginPaint(hWnd, &ps);
			FillRect(hDC, &ps.rcPaint, (HBRUSH )GetStockObject(BLACK_BRUSH));
			EndPaint(hWnd, &ps);
			return TRUE;
		}

	default:
		return DefWindowProc(hWnd, msg, wParam, lParam);
	}	

	return 0L;
}

BOOL SetupStradis(PVARIABLES pv)
{
	WNDCLASS  wc;
	DWORD dwFlags;
	RECT rcMainWindow;

    CoInitialize(NULL);

	v = pv;		// save for us
	GetWindowRect(v->hDlgSIParser, &rcMainWindow);

	// Setup the window classes
	wc.style =			CS_VREDRAW | CS_HREDRAW;
	wc.lpfnWndProc =	MainWndProc;
	wc.cbClsExtra =		0;
	wc.cbWndExtra =		0;
	wc.hInstance =		v->hInstance;
	wc.hIcon =			NULL;
	wc.hCursor =		LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground =	(HBRUSH) (COLOR_WINDOW); 
	wc.lpszMenuName =	NULL;//MAKEINTRESOURCE(ID_STANDARD_MENU);
	wc.lpszClassName =	"TSReader_DirectShowClass";
	RegisterClass(&wc);
	

	dwFlags = WS_BORDER | WS_SYSMENU;
	ghApp = CreateWindow("TSReader_DirectShowClass",
						   "TSReader DS Display",
						   dwFlags,
						   rcMainWindow.left, rcMainWindow.top,
						   720, 480,
						   0,
						   0,
						   v->hInstance,
						   0);
	if (ghApp)
		StartPlayback(); 

	return TRUE;
}

void ShutdownStradis()
{
	CloseClip();
	CoUninitialize();
	UnregisterClass("TSReader_DirectShowClass", v->hInstance);

}
