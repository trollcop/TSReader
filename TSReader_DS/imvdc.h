//
//  Copyright (c) 2000  ELECARD.  All Rights Reserved.
//

// A custom interface to allow the user to adjust the contrast level.

#ifndef __IMPEGVideoDecoderControl__
#define __IMPEGVideoDecoderControl__

#ifdef __cplusplus
extern "C" {
#endif

    #define MVDC_DECODE_FORCE_QC	0
    #define MVDC_DECODE_FORCE_I		1
    #define MVDC_DECODE_FORCE_IP	3
    #define MVDC_DECODE_FORCE_IPB	7
	#define	MVDC_DECODE_FORCE_MASK		0x0000000F

    #define MVDC_DECODE_BOB		0x00010000
    #define MVDC_DECODE_WEAVE		0x00000000

	#define MVDC_DECODE_RESOLUTION_MASK	0x00300000 
	#define MVDC_DECODE_RESOLUTION_SHIFT	20
	#define MVDC_DECODE_FULL_RESOLUTION	0x00000000
	#define MVDC_DECODE_HALF_VERTICAL_RESOLUTION	0x00100000
	#define MVDC_DECODE_HALF_HORIZONTAL_RESOLUTION	0x00200000
	#define MVDC_DECODE_QUARTER_RESOLUTION	0x00300000

    #define MVDC_COLORCONTROL_BRIGHTNESS	0

    // {aa4e6d3b-68f9-4ca8-aabe-2287a52915b4}
    DEFINE_GUID(IID_IMPEGVideoDecoderControl, 
    0xaa4e6d3b, 0x68f9, 0x4ca8, 0xaa, 0xbe, 0x22, 0x87, 0xa5, 0x29, 0x15, 0xb4);

    DECLARE_INTERFACE_(IMPEGVideoDecoderControl, IUnknown)
    {
        STDMETHOD(get_DecodeFlags) (THIS_
            DWORD *dwFlags
        ) PURE;
        STDMETHOD(put_DecodeFlags) (THIS_
            DWORD dwFlags
        ) PURE;
        STDMETHOD(get_InputMediaType) (THIS_
			AM_MEDIA_TYPE *pmt
        ) PURE;
        STDMETHOD(get_CurrentPicture) (THIS_
            BYTE *pBuffer, DWORD *pdwSize 
        ) PURE;

        STDMETHOD(set_OverlayBitmap) (THIS_
            void *pBmp, int X, int Y 
        ) PURE;

		STDMETHOD(get_DecodeStats)(THIS_
			DWORD *pdwTotal, DWORD *pdwSkipped
		) PURE;
    };


    // {aa4e6d3c-68f9-4ca8-aabe-2287a52915b4}
    DEFINE_GUID(IID_IMPEGVideoDecoderControl2, 
    0xaa4e6d3c, 0x68f9, 0x4ca8, 0xaa, 0xbe, 0x22, 0x87, 0xa5, 0x29, 0x15, 0xb4);

    DECLARE_INTERFACE_(IMPEGVideoDecoderControl2, IMPEGVideoDecoderControl)
    {
        STDMETHOD(get_ColorControl)(THIS_
		    DWORD dwCCIndex, DWORD *pdwCCValue
        ) PURE;

        STDMETHOD(set_ColorControl)(THIS_
            DWORD dwCCIndex, DWORD dwCCValue
        ) PURE;
	};

#ifdef __cplusplus
}
#endif

#endif // __IMPEGVideoDecoderControl__

