// Satellite.h: interface for the Satellite class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SATELLITE_H__608551DB_8975_480F_AE87_322AFFBE4EE4__INCLUDED_)
#define AFX_SATELLITE_H__608551DB_8975_480F_AE87_322AFFBE4EE4__INCLUDED_

//#include <windows.h>
#include "seconst.h"
#include "seerror.h"
#include "seprot.h"

#define PACKETBUFFER 48128 //13300
#define VIDEOBUFFER  9400 //9400
#define AUDIOBUFFER  1504 //3008

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#pragma pack(1)
typedef struct _PACKET {
	SE_UINT   iBoardNumber;
	SE_HANDLE seHandle;
	PSE_VOID  pvUserData;
	PSE_VOID  pvData;
	SE_UINT   uiDataBlockLength;
	SE_INT    iPacketStatus;
} PACKET;


class Satellite  
{
public:
	STDMETHOD(put_String)(unsigned short id, LPSTR val);
	BOOL bStarting;
	short width;
	short mono;
	STDMETHOD(get_Parameter)(unsigned short id, short *val);
	STDMETHOD(put_Parameter)(unsigned short id, short val);
	unsigned short ac3;
	STDMETHOD(SetAC3)(unsigned short flag);
	STDMETHODIMP SetAPID(unsigned short pid);
	STDMETHODIMP SetPCR(unsigned short pid);
	STDMETHODIMP SetVPID(unsigned short pid);
	STDMETHODIMP SetPipe(HANDLE hDataPipe);
	unsigned short GetAPID(void);
	unsigned short GetVPID(void);
	int DeInit();
	int Init();
	SE_ULONG GetData(char *buff, unsigned long maxlen);
	int Stop();
	int Start();
	Satellite();
	virtual ~Satellite();
	SE_USHORT vpid;
	SE_USHORT apid;
	SE_USHORT pcr;
	BOOL quit;
	BOOL twinhanquit;
	CRITICAL_SECTION csBufferCritical;
	int nOutstandingData;
	HANDLE m_hDataPipe;

private:
	SE_USHORT prognum;
};

#endif // !defined(AFX_SATELLITE_H__608551DB_8975_480F_AE87_322AFFBE4EE4__INCLUDED_)
