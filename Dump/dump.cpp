//------------------------------------------------------------------------------
// File: Dump.cpp
//
// Desc: DirectShow sample code - implementation of a renderer that dumps
//       the samples it receives into a text file.
//
// Copyright (c) 1992-2001  Microsoft Corporation.  All rights reserved.
//------------------------------------------------------------------------------


// Summary
//
// We are a generic renderer that can be attached to any data stream that
// uses IMemInputPin data transport. For each sample we receive we write
// its contents including its properties into a dump file. The file we
// will write into is specified when the dump filter is created. GraphEdit
// creates a file open dialog automatically when it sees a filter being
// created that supports the IFileSinkFilter interface.
//
//
// Implementation
//
// Pretty straightforward really, we have our own input pin class so that
// we can override Receive, all that does is to write the properties and
// data into a raw data file (using the Write function). We don't keep
// the file open when we are stopped so the flags to the open function
// ensure that we open a file if already there otherwise we create it.
//
//
// Demonstration instructions
//
// Start GraphEdit, which is available in the SDK DXUtils folder. Drag and drop
// an MPEG, AVI or MOV file into the tool and it will be rendered. Then go to
// the filters in the graph and find the filter (box) titled "Video Renderer"
// This is the filter we will be replacing with the dump renderer. Then click
// on the box and hit DELETE. After that go to the Graph menu and select the
// "Insert Filters", from the dialog box find and select the "Dump Filter".
//
// You will be asked to supply a filename where you would like to have the
// data dumped, the data we receive in this filter is dumped in text form.
// Then dismiss the dialog. Back in the graph layout find the output pin of
// the filter that used to be connected to the input of the video renderer
// you just deleted, right click and do "Render". You should see it being
// connected to the input pin of the dump filter you just inserted.
//
// Click Pause and Run and then a little later stop on the GraphEdit frame and
// the data being passed to the renderer will be dumped into a file. Stop the
// graph and dump the filename that you entered when inserting the filter into
// the graph, the data supplied to the renderer will be displayed as raw data
//
//
// Files
//
// dump.cpp             Main implementation of the dump renderer
// dump.def             What APIs the DLL will import and export
// dump.h               Class definition of the derived renderer
// dump.rc              Version information for the sample DLL
// dumpuids.h           CLSID for the dump filter
// makefile             How to build it...
//
//
// Base classes used
//
// CBaseFilter          Base filter class supporting IMediaFilter
// CRenderedInputPin    An input pin attached to a renderer
// CUnknown             Handle IUnknown for our IFileSinkFilter
// CPosPassThru         Passes seeking interfaces upstream
// CCritSec             Helper class that wraps a critical section
//
//

#include <windows.h>
#include <commdlg.h>
#include <streams.h>
#include <initguid.h>
#include <stdio.h>

#include "dumpuids.h"
#include "dump.h"

#include <commctrl.h>

// Setup data

const AMOVIESETUP_MEDIATYPE sudPinTypes =
{
    &MEDIATYPE_NULL,            // Major type
    &MEDIASUBTYPE_NULL          // Minor type
};

const AMOVIESETUP_PIN sudPins =
{
    L"Input",                   // Pin string name
    FALSE,                      // Is it rendered
    FALSE,                      // Is it an output
    FALSE,                      // Allowed none
    FALSE,                      // Likewise many
    &CLSID_NULL,                // Connects to filter
    L"Output",                  // Connects to pin
    1,                          // Number of types
    &sudPinTypes                // Pin information
};

const AMOVIESETUP_FILTER sudDump =
{
    &CLSID_Dump,                // Filter CLSID
    L"TSReader_B2C2_Dump",      // String name
    MERIT_DO_NOT_USE,           // Filter merit
    1,                          // Number pins
    &sudPins                    // Pin details
};


//
//  Object creation stuff
//
CFactoryTemplate g_Templates[]= {
    L"TSReader_B2C2_Dump", &CLSID_Dump, CDump::CreateInstance, NULL, &sudDump
};
int g_cTemplates = sizeof(g_Templates) / sizeof(g_Templates[0]);

// Constructor

CDumpFilter::CDumpFilter(CDump *pDump,
                         LPUNKNOWN pUnk,
                         CCritSec *pLock,
                         HRESULT *phr) :
    CBaseFilter(NAME("CDumpFilter"), pUnk, pLock, CLSID_Dump),
    m_pDump(pDump)
{
}


//
// GetPin
//
CBasePin * CDumpFilter::GetPin(int n)
{
    if (n == 0)
	{
        return m_pDump->m_pPin;
    }
	else
	{
        return NULL;
    }
}


//
// GetPinCount
//
int CDumpFilter::GetPinCount()
{
    return 1;
}


//
// Stop
//
// Overriden to close the dump file
//
STDMETHODIMP CDumpFilter::Stop()
{
    CAutoLock cObjectLock(m_pLock);
    CBaseFilter::Stop();
	m_pDump->ss->fReadThreadTerminated = TRUE;
    return S_OK;
}


//
// Pause
//
// Overriden to open the dump file
//
STDMETHODIMP CDumpFilter::Pause()
{
    CAutoLock cObjectLock(m_pLock);
    return CBaseFilter::Pause();
}


//
// Run
//
// Overriden to open the dump file
//
STDMETHODIMP CDumpFilter::Run(REFERENCE_TIME tStart)
{
    CAutoLock cObjectLock(m_pLock);
    return CBaseFilter::Run(tStart);
}


//
//  Definition of CDumpInputPin
//
CDumpInputPin::CDumpInputPin(CDump *pDump,
                             LPUNKNOWN pUnk,
                             CBaseFilter *pFilter,
                             CCritSec *pLock,
                             CCritSec *pReceiveLock,
                             HRESULT *phr) :

    CRenderedInputPin(NAME("CDumpInputPin"),
                  pFilter,                   // Filter
                  pLock,                     // Locking
                  phr,                       // Return code
                  L"Input"),                 // Pin name
    m_pReceiveLock(pReceiveLock),
    m_pDump(pDump),
    m_tLast(0)
{
}


//
// CheckMediaType
//
// Check if the pin can support this specific proposed type and format
//
HRESULT CDumpInputPin::CheckMediaType(const CMediaType *)
{
    return S_OK;
}



//
// BreakConnect
//
// Break a connection
//
HRESULT CDumpInputPin::BreakConnect()
{
    if (m_pDump->m_pPosition != NULL)
	{
        m_pDump->m_pPosition->ForceRefresh();
    }
    //return CRenderedInputPin::BreakConnect();
    return S_OK;
}


//
// ReceiveCanBlock
//
// We don't hold up source threads on Receive
//
STDMETHODIMP CDumpInputPin::ReceiveCanBlock()
{
    return S_FALSE;
}


//
// Receive
//
// Do something with this media sample
//
STDMETHODIMP CDumpInputPin::Receive(IMediaSample *pSample)
{
    CAutoLock lock(m_pReceiveLock);
    PBYTE pbData;

    // Copy the data to the file
    HRESULT hr = pSample->GetPointer(&pbData);
    if (FAILED(hr))
	{
        return hr;
    }
    return m_pDump->Write(pbData, pSample->GetActualDataLength());
}


//
// DumpStringInfo
//
// Write to the file as text form
//
HRESULT CDumpInputPin::WriteStringInfo(IMediaSample *pSample)
{
    return NOERROR;
}


//
// EndOfStream
//
STDMETHODIMP CDumpInputPin::EndOfStream(void)
{
    CAutoLock lock(m_pReceiveLock);
    return CRenderedInputPin::EndOfStream();

} // EndOfStream


//
// NewSegment
//
// Called when we are seeked
//
STDMETHODIMP CDumpInputPin::NewSegment(REFERENCE_TIME tStart,
                                       REFERENCE_TIME tStop,
                                       double dRate)
{
    m_tLast = 0;
    return S_OK;

} // NewSegment


//
//  CDump class
//
CDump::CDump(LPUNKNOWN pUnk, HRESULT *phr) :
    CUnknown(NAME("CDump"), pUnk),
    m_pFilter(NULL),
    m_pPin(NULL),
    m_pPosition(NULL),
    m_hFile(INVALID_HANDLE_VALUE),
    m_pFileName(0)
{
    m_pFilter = new CDumpFilter(this, GetOwner(), &m_Lock, phr);
    if (m_pFilter == NULL)
	{
        *phr = E_OUTOFMEMORY;
        return;
    }

    m_pPin = new CDumpInputPin(this,GetOwner(),
                               m_pFilter,
                               &m_Lock,
                               &m_ReceiveLock,
                               phr);
    if (m_pPin == NULL) {
        *phr = E_OUTOFMEMORY;
        return;
    }

	nTSBufferIndex = 0;
}


//
// GetCurFile
//
// Implemented for IFileSinkFilter support
//
STDMETHODIMP CDump::GetCurFile(LPOLESTR * ppszFileName,AM_MEDIA_TYPE *pmt)
{
    if(pmt)
	{
        ZeroMemory(pmt, sizeof(*pmt));
        pmt->majortype = MEDIATYPE_NULL;
        pmt->subtype = MEDIASUBTYPE_NULL;
    }
    return S_OK;

} // GetCurFile


// Destructor

CDump::~CDump()
{
    delete m_pPin;
    delete m_pFilter;
    delete m_pPosition;
    delete m_pFileName;
}


//
// CreateInstance
//
// Provide the way for COM to create a dump filter
//
CUnknown * WINAPI CDump::CreateInstance(LPUNKNOWN punk, HRESULT *phr)
{
    CDump *pNewObject = new CDump(punk, phr);
    if (pNewObject == NULL)
	{
        *phr = E_OUTOFMEMORY;
    }
    return pNewObject;

} // CreateInstance


//
// NonDelegatingQueryInterface
//
// Override this to say what interfaces we support where
//
STDMETHODIMP CDump::NonDelegatingQueryInterface(REFIID riid, void ** ppv)
{
    CheckPointer(ppv,E_POINTER);
    CAutoLock lock(&m_Lock);

    // Do we have this interface

    if (riid == IID_IFileSinkFilter)
	{
        return GetInterface((IFileSinkFilter *) this, ppv);
    } 
    else if (riid == IID_IBaseFilter || riid == IID_IMediaFilter || riid == IID_IPersist)
	{
	    return m_pFilter->NonDelegatingQueryInterface(riid, ppv);
    } 
    else if (riid == IID_IMediaPosition || riid == IID_IMediaSeeking) 
	{
        if (m_pPosition == NULL)
		{

            HRESULT hr = S_OK;
            m_pPosition = new CPosPassThru(NAME("Dump Pass Through"),
                                           (IUnknown *) GetOwner(),
                                           (HRESULT *) &hr, m_pPin);
            if (m_pPosition == NULL)
			{
                return E_OUTOFMEMORY;
            }

            if (FAILED(hr))
			{
                delete m_pPosition;
                m_pPosition = NULL;
                return hr;
            }
        }

        return m_pPosition->NonDelegatingQueryInterface(riid, ppv);
    } 

    return CUnknown::NonDelegatingQueryInterface(riid, ppv);

} // NonDelegatingQueryInterface

//
// Write
//
// Write stuff to the file
//

HRESULT CDump::Write(PBYTE pbData, LONG lData)
{
	if (ss->tsb[nTSBufferIndex].nSize + lData >= TS_BUFFER_SIZE)
	{
		EnterCriticalSection(&ss->csPIDCounter);
		ss->nLastSecondByteCounter += ss->tsb[nTSBufferIndex].nSize;
		LeaveCriticalSection(&ss->csPIDCounter);

		nTSBufferIndex++;
		if (nTSBufferIndex == MAX_TS_BUFFERS)
		{
			nTSBufferIndex = 0;
			ss->fIgnoreContinuity = FALSE;
		}
		EnterCriticalSection(&ss->csTSBuffersInUse);
		ss->nTSBuffersInUse++;
		LeaveCriticalSection(&ss->csTSBuffersInUse);
		ss->tsb[nTSBufferIndex].nSize = 0;
	}

	memcpy(ss->tsb[nTSBufferIndex].pData + ss->tsb[nTSBufferIndex].nSize, pbData, lData);
	ss->tsb[nTSBufferIndex].nSize += lData;

    return S_OK;
}

//
// SetFileName
//
// Implemented for IFileSinkFilter support
//
STDMETHODIMP CDump::SetFileName(LPCOLESTR pszFileName,const AM_MEDIA_TYPE *pmt)
{
	int nSS;
	int nPID = 0;

	sscanf((char *)pszFileName, "%x", &nSS);
	ss = (PSOURCESTRUCT)nSS;

	{
		char szTemp[128];
		wsprintf(szTemp, "dump.cpp: %s\n", (char *)pszFileName);
		OutputDebugString(szTemp);
	}
	ss->fIgnoreContinuity = TRUE;

    return S_OK;
} // SetFileName

//
// DllRegisterSever
//
// Handle the registration of this filter
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
