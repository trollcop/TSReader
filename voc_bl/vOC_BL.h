// vOC_BL.h: interface for the vOC_BL class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_VOC_BL_H__397B5A24_6C25_4D32_878A_62137B408D50__INCLUDED_)
#define AFX_VOC_BL_H__397B5A24_6C25_4D32_878A_62137B408D50__INCLUDED_

#include "Satellite.h"	// Added by ClassView
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

// {4FCCC8DE-433A-469e-B7DE-B7E3AB622865}
DEFINE_GUID(CLSID_vOC_BL, 
0x4fccc8de, 0x433a, 0x469e, 0xb7, 0xde, 0xb7, 0xe3, 0xab, 0x62, 0x28, 0x65);


//------------------------------------------------------------------------------
// Forward Declarations
//------------------------------------------------------------------------------
// The class managing the output pin
class CvOC_BLStream;


class vOC_BL : public CSource, public IGenericSat
{
public:
	STDMETHOD(put_String)(unsigned short id, LPSTR val);
	STDMETHOD(get_Parameter)(unsigned short id, short *val);
	STDMETHOD(put_Parameter)(unsigned short id, short val);
	STDMETHOD(SetAC3)(unsigned short flag);
    DECLARE_IUNKNOWN;


	STDMETHOD(SetPipe)(HANDLE hDataPipe);
	STDMETHOD(SetAPID)(unsigned short pid);
	STDMETHOD(SetVPID)(unsigned short pid);
	STDMETHOD(SetPCR)(unsigned short pid);
	STDMETHOD(NonDelegatingQueryInterface)(REFIID riid, void **ppv);

    // The only allowed way to create Bouncing balls!
    static CUnknown * WINAPI CreateInstance(LPUNKNOWN lpunk, HRESULT *phr);

private:

    // It is only allowed to to create these objects with CreateInstance
    vOC_BL(LPUNKNOWN lpunk, HRESULT *phr);
	~vOC_BL();

}; // CBouncingBall

//------------------------------------------------------------------------------
// Class CvOC_BLStream
//
// This class implements the stream which is used to output dvb mpeg2
// data from the source filter. It inherits from DirectShows's base
// CSourceStream class.
//------------------------------------------------------------------------------
class CvOC_BLStream : public CSourceStream
{

public:
	HRESULT Run(void);
	HRESULT CompleteConnect(IPin *pReceivePin);
	Satellite *m_Satellite;

    CvOC_BLStream(HRESULT *phr, vOC_BL *pParent, LPCWSTR pPinName);
    ~CvOC_BLStream();

    // plots a ball into the supplied video frame
    HRESULT FillBuffer(IMediaSample *pms);

    // Ask for buffers of the size appropriate to the agreed media type
    HRESULT DecideBufferSize(IMemAllocator *pIMemAlloc,
                             ALLOCATOR_PROPERTIES *pProperties);

    // Set the agreed media type, and set up the necessary ball parameters
    HRESULT SetMediaType(const CMediaType *pMediaType);

    // Because we calculate the ball there is no reason why we
    // can't calculate it in any one of a set of formats...
    HRESULT CheckMediaType(const CMediaType *pMediaType);
    HRESULT GetMediaType(int iPosition, CMediaType *pmt);

    // Resets the stream time to zero
    HRESULT OnThreadCreate(void);
    HRESULT OnThreadDestroy(void);

    // Quality control notifications sent to us
    STDMETHODIMP Notify(IBaseFilter * pSender, Quality q);

private:
	CMediaType * CreateAC3AudioType();
	CMediaType * CreateMPEG1AudioType(void);
	CMediaType *CreateMPEG2VideoType(int w=720, int h=576, int level=2, int profile=2);

    CCritSec m_cSharedState;	        // Lock on m_rtSampleTime and m_Ball
    vOC_BL *m_vOCBL;	                // The current source object

}; // CvOC_BLStream
	

#endif // !defined(AFX_VOC_BL_H__397B5A24_6C25_4D32_878A_62137B408D50__INCLUDED_)
