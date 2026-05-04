#define _WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <streams.h>
#include <atlbase.h>
#include <xprtdefs.h>
#include "dump.h"

#include "dctrecord.h"
#include "..\sources.h"

#define SAFE_RELEASE(i) { if (i) i->Release(); i = NULL; }

extern PSOURCESTRUCT ss;
extern CAPDEVICES gDevices[MAX_DEVICES];
extern int gDeviceCount;
extern char szLastTune[256];

// check output pin for providing MPEG2TS (raw or strided)
static int GetPinType(IMoniker *pMoniker)
{
    HRESULT hr = S_OK;
    
    CComPtr <IPin> pPin = NULL;
    CComPtr <IBaseFilter> pCapFilter = NULL;
    CComPtr <IGraphBuilder> pGraph = NULL;
    hr = pGraph.CoCreateInstance(CLSID_FilterGraph, 0, CLSCTX_INPROC);

    if (FAILED(hr))
	return -1;

    hr = pMoniker->BindToObject(0, 0, IID_IBaseFilter, (void**)&pCapFilter);
    if (FAILED(hr))
	return -1;

    hr = pGraph->AddFilter(pCapFilter, L"Capture Filter");
    if (FAILED(hr))
	return -1;

    CComPtr <ICaptureGraphBuilder2> pBuilder;
    hr = pBuilder.CoCreateInstance(CLSID_CaptureGraphBuilder2, 0, CLSCTX_INPROC);
    if (FAILED(hr))
	return -1;

    AM_MEDIA_TYPE *mt;
    hr = pBuilder->FindPin(pCapFilter, PINDIR_OUTPUT, &PIN_CATEGORY_CAPTURE, NULL, true, 0, &pPin);
    if (SUCCEEDED(hr)) {
	IEnumMediaTypes *pEnum;
	pPin->EnumMediaTypes(&pEnum);
	ULONG c;
	pEnum->Next(1, &mt, &c);
    }

    if ((mt->majortype == MEDIATYPE_Stream) && (mt->subtype == MEDIASUBTYPE_MPEG2_TRANSPORT || mt->subtype == MEDIASUBTYPE_MPEG2_TRANSPORT_STRIDE))
	return 0;

    return 1;
}

// find all capture sources w/MPEG2TS output capability and store their friendly names for later lookup
HRESULT EnumCaptureSources(void)
{
    HRESULT hr;

    // create system device enumerator
    CComPtr <ICreateDevEnum> pDevEnum = NULL;
    CComPtr <IEnumMoniker> pClassEnum = NULL;
    IMoniker *pMoniker = NULL;
    ULONG cFetched = 0;
    int found = 0;

    hr = pDevEnum.CoCreateInstance(CLSID_SystemDeviceEnum, 0, CLSCTX_INPROC);
    if (FAILED(hr))
	return E_FAIL;

    hr = pDevEnum->CreateClassEnumerator(CLSID_VideoInputDeviceCategory, &pClassEnum, 0);
    if (pClassEnum == NULL)
	return E_FAIL;

    // Enumerate all items associated with the moniker
    while (pClassEnum->Next(1, &pMoniker, &cFetched) == S_OK) {

	VARIANT varName = {0};
	CComPtr <IPropertyBag> pPropBag = NULL;
	if (!pMoniker)
	    continue;

	// Associate moniker with a file
	hr = pMoniker->BindToStorage(0, 0, IID_IPropertyBag, (void **)&pPropBag);
	if (FAILED(hr))
	    continue;

	if (!pPropBag)
	    continue;

	// Read filter name from property bag
	varName.vt = VT_BSTR;
	hr = pPropBag->Read(L"FriendlyName", &varName, 0);
	if (FAILED(hr))
	    continue;

	TCHAR str[MAX_PATH];
	WideCharToMultiByte(CP_ACP, 0, varName.bstrVal, -1, str, sizeof(str), 0, 0);
	VariantClear(&varName);

	int type = GetPinType(pMoniker);
	if (type == 0) {
	    lstrcpy(gDevices[found].DeviceName, str);
	    found++;
	}
	SAFE_RELEASE(pMoniker);
    }

    gDeviceCount = found;
    return S_OK;
}

static HRESULT GetCaptureDevice(IBaseFilter **device)
{
    HRESULT hr;
    IMoniker *pMoniker = NULL;
    IBaseFilter *capture_device = NULL;
    ULONG cFetched = 0;

    // create system device enumerator
    CComPtr <ICreateDevEnum> pDevEnum = NULL;

    hr = pDevEnum.CoCreateInstance(CLSID_SystemDeviceEnum, NULL, CLSCTX_INPROC);
    if (FAILED(hr))
	return E_FAIL;

    CComPtr <IEnumMoniker> pClassEnum = NULL;
    hr = pDevEnum->CreateClassEnumerator(CLSID_VideoInputDeviceCategory, &pClassEnum, 0);
    if (FAILED(hr))
	return E_FAIL;

    if (pClassEnum == NULL)
	return E_FAIL;

    while (true) {
	if (S_OK == (pClassEnum->Next (1, &pMoniker, &cFetched))) {
	    CComPtr <IPropertyBag> pBag;
	    hr = pMoniker->BindToStorage(NULL, NULL, IID_IPropertyBag, reinterpret_cast<void**>(&pBag));
	    if (FAILED(hr))
		return hr;

	    CComVariant varBSTR;
	    hr = pBag->Read(L"FriendlyName", &varBSTR, NULL);
	    if (FAILED(hr)) {
		pMoniker = NULL;
		return E_FAIL;
	    }

	    TCHAR str[MAX_PATH];
	    WideCharToMultiByte(CP_ACP, 0, varBSTR.bstrVal, -1, str, sizeof(str), 0, 0);
	    VariantClear(&varBSTR);

	    // try to connect to the device given by the index
	    if (strcmp(gDevices[ss->nSourceIndex].DeviceName, str) != 0) {
		continue;
	    } else {
		hr = pMoniker->BindToObject(0, 0, IID_IBaseFilter, (void**)&capture_device);
		if (FAILED(hr))
		    return E_FAIL;
		strncpy(szLastTune, gDevices[ss->nSourceIndex].DeviceName, sizeof(szLastTune));
		break;
	    }
	} else {
	    break;
	}
    }

    if (capture_device == NULL)
	return E_FAIL;

    *device = capture_device;
    return S_OK;
}

HRESULT GetUnconnectedPin(IBaseFilter *pFilter, PIN_DIRECTION PinDir, IPin **ppPin)
{
    *ppPin = 0;
    IEnumPins *pEnum = 0;
    IPin *pPin = 0;
    HRESULT hr = pFilter->EnumPins(&pEnum);

    if (FAILED(hr))
        return hr;

    while (pEnum->Next(1, &pPin, NULL) == S_OK) {
        PIN_DIRECTION ThisPinDir;
        pPin->QueryDirection(&ThisPinDir);
        if (ThisPinDir == PinDir) {
            IPin *pTmp = 0;
            hr = pPin->ConnectedTo(&pTmp);

	    // Already connected, not the pin we want.
	    if (SUCCEEDED(hr)) {
		pTmp->Release();
	    }else {
		// Unconnected, this is the pin we want.
		pEnum->Release();
		*ppPin = pPin;
		return S_OK;
	    }
	}
        pPin->Release();
    }
    pEnum->Release();

    // Did not find a matching pin.
    return E_FAIL;
}

int dctrecord(void)
{
    HRESULT hr = 0;

    // create a filter graph
    CComPtr<IFilterGraph> pGraph;
    hr = pGraph.CoCreateInstance(CLSID_FilterGraph);

    // QI the filter graph for the other useful interfaces
    CComQIPtr< IGraphBuilder, &IID_IGraphBuilder > pBuilder(pGraph);
    CComQIPtr< IMediaControl, &IID_IMediaControl > pControl(pGraph);

    // add the dump filter
    CComPtr<IBaseFilter> pDump;
    pDump = new CDumpFilter(NULL, &hr);

    // add a source filter for it
    CComPtr<IBaseFilter> pSource;

    // try to find the MPEG2TS source
    hr = GetCaptureDevice(&pSource);

    if (hr != S_OK) {
	OutputDebugString("Firewire: Capture device inaccessible after enum() - device removed?\n");
        return 0;
    }

    IPin *pSourceOut = NULL;
    hr =  GetUnconnectedPin(pSource, PINDIR_OUTPUT, &pSourceOut);
    hr = pBuilder->AddFilter(pSource, L"TunerFilter");
    hr = pBuilder->AddFilter(pDump, L"Dump");

    // get the other input and output pins
    IPin *pDumpIn = NULL;
    hr = GetUnconnectedPin(pDump, PINDIR_INPUT, &pDumpIn);

    // connect
    hr = pBuilder->Connect(pSourceOut, pDumpIn);

    pSource->SetSyncSource(NULL);
    pDump->SetSyncSource(NULL);

    hr = pControl->Pause();
    hr = pControl->Run();
    ASSERT(!FAILED(hr));

    while (!ss->fTerminateReadThread)
	Sleep(10);

    hr = pControl->Stop();
    hr = pBuilder->Abort();

    return 0;
}
