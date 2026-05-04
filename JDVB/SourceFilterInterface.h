#ifndef SOURCEFILTERINTERFACE_H___
#define SOURCEFILTERINTERFACE_H___

#ifdef __cplusplus
extern "C" {
#endif

#include <windows.h>

class TSPacket;
class TSPacketQueue;
class TSPidFilter;


// {C24D1867-D148-4421-A57E-F87F7C397434}
DEFINE_GUID(IID_SOURCE_FILTER, 
0xc24d1867, 0xd148, 0x4421, 0xa5, 0x7e, 0xf8, 0x7f, 0x7c, 0x39, 0x74, 0x34);

static CLSID CLSID_SourceFilter =  
{0x52eecb1f, 0xb9f3, 0x497b, {0x83, 0x60, 0xf0, 0xd5, 0x54, 0x4b, 0xa2, 0xcd}};


#define g_wszSourceFilter	L"Source Filter Version 1.0"

DECLARE_INTERFACE_(ISourceFilter, IUnknown)
{
	STDMETHOD(processPid)(TSPacket *Packet) PURE;
	
	STDMETHOD(setTSPacketQueue)(TSPacketQueue *Queue) PURE;

	STDMETHOD(setNextTSPidFilter)(TSPidFilter *Filter) PURE;

};






#ifdef __cplusplus
}
#endif

#endif