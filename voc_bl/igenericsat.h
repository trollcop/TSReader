#ifndef __IGENERICSAT__
#define __IGENERICSAT__

#ifdef __cplusplus
extern "C" {
#endif

	// {F8F1B7E5-400B-4c15-B777-E18473060157}
	DEFINE_GUID(IID_IGenericSat, 
	0xf8f1b7e5, 0x400b, 0x4c15, 0xb7, 0x77, 0xe1, 0x84, 0x73, 0x6, 0x1, 0x57);

    DECLARE_INTERFACE_(IGenericSat, IUnknown)
    {
        STDMETHOD(SetPipe) (THIS_
			HANDLE hPipe
        ) PURE;

        STDMETHOD(SetVPID) (THIS_
			unsigned short pid
        ) PURE;

        STDMETHOD(SetAPID) (THIS_
			unsigned short pid
        ) PURE;

        STDMETHOD(SetPCR) (THIS_
			unsigned short pid
        ) PURE;

        STDMETHOD(SetAC3) (THIS_
			unsigned short flag
        ) PURE;

        STDMETHOD(put_Parameter) (THIS_
			unsigned short id,
			short val
        ) PURE;

        STDMETHOD(put_String) (THIS_
			unsigned short id,
			LPSTR val
        ) PURE;

        STDMETHOD(get_Parameter) (THIS_
			unsigned short id,
			short *val
        ) PURE;
    };

#ifdef __cplusplus
}
#endif

#endif // __IGENERICSAT__

