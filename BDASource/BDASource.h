#ifndef BDASOURCE_H

#define BDASOURCE_H

#include <streams.h>
#include <mmreg.h>
#include <msacm.h>
#include <fcntl.h>
#include <io.h>
#include <stdio.h>
#include <ks.h>
#include <ksmedia.h>
#include <bdatypes.h>
#include <bdamedia.h>
#include <bdaiface.h>
#include <uuids.h>
#include <tuner.h>
#include <commctrl.h>
#include <atlbase.h>
#include <qedit.h>

enum NETWORK_TYPE
{
	NT_DVBT,
	NT_DVBS
};

// interface for sample grabbing

class GrabberCallback : public ISampleGrabberCB
{
public:
    STDMETHODIMP_(ULONG) AddRef() { return 1; }
    STDMETHODIMP_(ULONG) Release() { return 2; }

    STDMETHODIMP QueryInterface(REFIID riid, void **ppvObject)
    {
        if (NULL == ppvObject) return E_POINTER;
        if (riid == __uuidof(IUnknown))
        {
            *ppvObject = static_cast<IUnknown*>(this);
             return S_OK;
        }
        if (riid == __uuidof(ISampleGrabberCB))
        {
            *ppvObject = static_cast<ISampleGrabberCB*>(this);
             return S_OK;
        }
        return E_NOTIMPL;
    }

    STDMETHODIMP SampleCB(double Time, IMediaSample *pSample)
    {
        return E_NOTIMPL;
    }

    STDMETHODIMP BufferCB(double Time, BYTE *pBuffer, long BufferLen)
    {
		(*m_callback)(Time, pBuffer, BufferLen, m_callbackdata);

        return S_OK;
    }

	void SetCallback(int (*callback_func)(double, BYTE*, long, LPVOID), LPVOID data)
	{
		m_callback=callback_func;
		m_callbackdata=data;
	}

private:
	int (*m_callback)(double, BYTE*, long, LPVOID);
	LPVOID m_callbackdata;
};

class BDAGraph
{
public:

	BDAGraph();
	~BDAGraph();
	int Create(NETWORK_TYPE, int (*callback_func)(double, BYTE*, long, LPVOID), LPVOID);
	void SetInitialTune(LONG, LONG, LONG, LONG, LONG);
	int Destroy();
	int Play();
	int Stop();
	int GetTuneStatus(LONG*, BOOLEAN*,BOOLEAN*,LONG*,LONG*);
	int GetDeviceName(LPWSTR*);
	int TuneDVB(LONG, LONG, LONG, LONG, LONG);
	int IsCreated()
	{
		return fGraphCreated;
	}
	int IsRunning()
	{
		return fGraphRunning;
	}


protected:

	int LoadTuningSpace();
	int LoadNetworkProvider();
	int LoadSampleGrabber();
	int LoadInfiniteTee();
	int LoadDemux();
	int LoadTIF();

	int CreateDefaultTuneRequest();
	int CreateDVBTTuneRequest(LONG, LONG, LONG, LONG, LONG); 

	HRESULT ConnectFilters (IBaseFilter*, IBaseFilter*);
	HRESULT LoadFilter(REFCLSID, IBaseFilter**, IBaseFilter* , BOOL);

	int fGraphCreated;
	int fGraphRunning;

	LONG initial_carrier;
	LONG initial_bandwidth;
	LONG initial_ONID;
	LONG initial_SID;
	LONG initial_TSID;

	CComPtr <IGraphBuilder> pGraphBuilder;
	CComPtr <ITuningSpace> pTuningSpace;
	CComPtr <IBaseFilter> pNetworkProvider;
	
	CComPtr <IBaseFilter> pTuneDev;
	CComPtr <IBaseFilter> pReceiverDev;
	CComPtr <IBaseFilter> pSampleGrabber;
	CComPtr <IBaseFilter> pInfiniteTee;
	CComPtr <IBaseFilter> pDemux;
	CComPtr <IBaseFilter> pTIF;

	NETWORK_TYPE networkType;

	GrabberCallback grabbercallback; // the callback object

};

#endif // BDASOURCE_H