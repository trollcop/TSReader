// Satellite.cpp: implementation of the Satellite class.
//
//////////////////////////////////////////////////////////////////////
#include <windows.h>
//#include <streams.h>
#include <initguid.h>
#include <olectl.h>
#if (1100 > _MSC_VER)
#include <olectlid.h>
#endif

#include "TransportPacket.h"
#include "Satellite.h"
#include <stdio.h>
enum PLAYSTATE {Init, FirstRun, ReadytoRun, ShouldRun, Running, Stopped};

#define MAX_BUFFERS 1000

int inHere = 0;
PACKET pb[MAX_BUFFERS];
int incnt = 0;
int outcnt = 0;
long cnt = 0;
int patreceived = 0;
PLAYSTATE playstate = Init;
unsigned long m_audio_frequency;
unsigned long m_audio_bit_rate;
int m_audio_channels;
int m_Layer;
int m_height;
int m_width;
short m_videoWratio;
short m_videoHratio;
BOOL m_bAudioInfo;
BOOL m_bVideoInfo;
short m_vpid;
short m_apid;

char tmp[50];

FILE *dump=NULL;
unsigned int bitrates[3][16] =
{{0,32,64,96,128,160,192,224,256,288,320,352,384,416,448,0},
 {0,32,48,56,64,80,96,112,128,160,192,224,256,320,384,0},
 {0,32,40,48,56,64,80,96,112,128,160,192,224,256,320,0}};

unsigned long freq[4] = {441, 480, 320, 0};

void __cdecl StreamCallBack(PSE_VOID  pvUserData, PSE_VOID  pvData, SE_UINT   uiDataBlockLength)
{
	if (playstate != Running)
	{
		CTransportPacket *rec = new CTransportPacket((unsigned char *)pvData);

		if (!m_bVideoInfo && rec->GetPID()==m_vpid)
		{
			if (/* rec->GetPayLoadStart() TODO: determine start*/ 1)
			{
				int found = 0;
				unsigned int c = 0;
				unsigned char *cbuff = (unsigned char *)pvData;
				while (found ==0 && c+4 < uiDataBlockLength)
				{
					unsigned char *b;

					b = (unsigned char *)cbuff+c;
					if ( b[0] == 0x00 && b[1] == 0x00 && b[2] == 0x01
						 && b[3] == 0xb3) found = 1;
					else 
					{
						c++;
					}
				}
				c+=4;
				unsigned char *headr = (unsigned char *)cbuff+c;
				if (found == 1)
				{
					m_width	 = ((headr[1] &0xF0) >> 4) | (headr[0] << 4);
					m_height = ((headr[1] &0x0F) << 8) | (headr[2]);
					int sw = (int)((headr[3]&0xF0) >> 4) ;

					switch( sw )
					{
					case 1:
						m_videoWratio=1;
						m_videoHratio=1;
						break;
					case 2:
						m_videoWratio=4;
						m_videoHratio=3;
						break;

					case 3:
						m_videoWratio=16;
						m_videoHratio=9;
						break;
					case 4:
						m_videoWratio=2;
						m_videoHratio=1;
						break;
					default:
						m_videoWratio=4;
						m_videoHratio=3;
						break;
					}
					m_bVideoInfo = TRUE;
					if (m_bAudioInfo)
					{
						if (playstate==ShouldRun)
							playstate=Running;
						else
							playstate=ReadytoRun;
					}
				}
			}
		} 
		else if (!m_bAudioInfo && rec->GetPID() == m_apid)
		{
			if (/* rec->GetPayLoadStart() TODO: determine start*/ 1)
			{
				unsigned char *cbuff = (unsigned char *)pvData;
				int found = 0;
				int fr;
				unsigned int c = 0;
				while (!found && c < uiDataBlockLength)
				{
					unsigned char *b = (unsigned char *)cbuff+c;

					if ( b[0] == 0xff && (b[1] & 0xf8) == 0xf8)
						found = 1;
					else 
						c++;
				}
				if (found == 1 && c+3< uiDataBlockLength)
				{
					unsigned char *headr = (unsigned char *)cbuff+c;

					m_Layer = (headr[1] & 0x06) >> 1;
					m_audio_bit_rate = bitrates[(3-m_Layer)][(headr[2] >> 4 )]*1000;
					fr = (headr[2] & 0x0c ) >> 2;
					m_audio_frequency = freq[fr]*100;
					m_bAudioInfo = TRUE;
					if (m_bVideoInfo)
					{
						if (playstate==ShouldRun)
							playstate=Running;
						else
							playstate=ReadytoRun;
					}
				}
			}
		}
		free(rec);
	}
	else
	{
		EnterCriticalSection((CRITICAL_SECTION *)pvUserData);
		incnt++;
		if (incnt >= MAX_BUFFERS)
			incnt=0; //incnt%=1000;
		if (incnt == outcnt)
			OutputDebugString("Buffer overflow\n");

		pb[incnt].pvData = pvData;
		pb[incnt].pvUserData = pvUserData;
		pb[incnt].uiDataBlockLength = uiDataBlockLength;
		LeaveCriticalSection((CRITICAL_SECTION *)pvUserData);
	}
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

Satellite::Satellite()
{
	nOutstandingData = 0;
	InitializeCriticalSection(&csBufferCritical);
	
	vpid = 1160;
	apid = 1120;
	pcr = 0x1FFF;
	ac3=0;
	mono=0;
	width=0;
}

Satellite::~Satellite()
{
	DeleteCriticalSection(&csBufferCritical);
}

#define STRADIS_READ_SIZE 188 * 32
DWORD WINAPI ReadTwinhanThread(LPVOID lpv)
{
	Satellite * pBaseClass = (Satellite *)lpv;
	DWORD dwRead;
	BYTE * buffer;

	while (!pBaseClass->quit)
	{
		buffer = (BYTE *)LocalAlloc(LPTR, STRADIS_READ_SIZE);

		if (ReadFile(pBaseClass->m_hDataPipe, buffer, STRADIS_READ_SIZE, &dwRead, NULL) == FALSE)
		{
			LocalFree(buffer);
			break;
		}
		StreamCallBack(&pBaseClass->csBufferCritical, buffer, dwRead);
	}

	//pBaseClass->fTerminalSignalThread = TRUE;
	pBaseClass->twinhanquit = 1;
	return 0;
}

int Satellite::Start()
{
	OutputDebugString("Start\n");

	quit = 0;
	twinhanquit = 0;
	DWORD dwThreadID;
	HANDLE hThread = CreateThread(NULL, 0, ::ReadTwinhanThread, (LPVOID)this, 0, &dwThreadID);
	CloseHandle(hThread);
	
	if (playstate==ReadytoRun)
		playstate=Running;
	else
		playstate=ShouldRun;

	return 1;
}

int Satellite::Stop()
{
	OutputDebugString("Stop\n");
	
	quit = 1;
	while (twinhanquit == 0)
		Sleep(10);
	
	EnterCriticalSection(&csBufferCritical);
	while (incnt != outcnt)
	{
		++outcnt;
		if (outcnt>=MAX_BUFFERS)
			outcnt=0; 
	}
	LeaveCriticalSection(&csBufferCritical);
	incnt = 0;
	outcnt = 0;
	cnt = 0;
	playstate = Stopped;
	return 1;
}

SE_ULONG Satellite::GetData(char *buff, unsigned long maxlen)
{
	SE_ULONG se_len=0;
	static cont_cnt=0;
	static zero_cnt = 0;
	static BOOL fFirstTime = TRUE;
	
	EnterCriticalSection(&csBufferCritical);

	if (incnt == outcnt)
	{
		LeaveCriticalSection(&csBufferCritical);
		zero_cnt++;
		if (zero_cnt == 250)
		{
			Sleep(1);
			zero_cnt = 0;
		}
		return 0;
	}

	zero_cnt = 0;

	while (incnt != outcnt) 
	{
		int outcnt_new = outcnt;

		++outcnt_new;
		if (outcnt_new >= MAX_BUFFERS)
			outcnt_new = 0; //outcnt%=1000;

		unsigned int packetlen = pb[outcnt_new].uiDataBlockLength;
		if (se_len + packetlen > maxlen)
			break;
		outcnt = outcnt_new;
		LeaveCriticalSection(&csBufferCritical);

		CTransportPacket *rec = new CTransportPacket((unsigned char *)pb[outcnt].pvData);

		if ( (rec->GetPID()==0) || (fFirstTime) )
		{
			fFirstTime = FALSE;
			CTransportPacket *pat = new CTransportPacket();
			pat->AddPATEntry(1, 0x20);
			pat->SetContCount(cont_cnt);
			pat->CalcCRC();
			pat->GetBuffer((unsigned char *)&buff[se_len]);
			se_len += 188;

			CTransportPacket *pmt = new CTransportPacket(0x0020);
			if (pcr == 0x1FFF)
				pcr=vpid;
			pmt->AddPMTEntry(1, vpid, apid, pcr);
			pmt->SetContCount(cont_cnt);
			pmt->CalcCRC();
			pmt->GetBuffer((unsigned char *)&buff[se_len]);
			se_len += 188;

			free(pat);
			free(pmt);

			cont_cnt++;
			if (cont_cnt > 16)
				cont_cnt=0;
		}
		else
		{
			memcpy(&buff[se_len], pb[outcnt].pvData, pb[outcnt].uiDataBlockLength);
			se_len += pb[outcnt].uiDataBlockLength;
		}
		free(rec);
		LocalFree(pb[outcnt].pvData);
		EnterCriticalSection(&csBufferCritical);
		nOutstandingData -= packetlen;					
	}
	LeaveCriticalSection(&csBufferCritical);
	bStarting = FALSE;

	return se_len;
}

int Satellite::Init()
{
	Start();
	return TRUE;
}

int Satellite::DeInit()
{
	Stop();
	return TRUE;
}

unsigned short Satellite::GetVPID()
{
	return vpid;
}

unsigned short Satellite::GetAPID()
{
	return apid;
}

STDMETHODIMP Satellite::SetPipe(HANDLE hDataPipe)
{
	m_hDataPipe = hDataPipe;
	return NOERROR;
}

STDMETHODIMP Satellite::SetVPID(unsigned short pid)
{
	vpid = pid;
	m_vpid = vpid;
	return NOERROR;
}

STDMETHODIMP Satellite::SetAPID(unsigned short pid)
{
	apid = pid;
	m_apid = apid;
	return NOERROR;
}

STDMETHODIMP Satellite::SetPCR(unsigned short pid)
{
	pcr = pid;
	return NOERROR;
}

STDMETHODIMP Satellite::SetAC3(unsigned short flag)
{
	ac3=flag;
	return NOERROR;
}

STDMETHODIMP Satellite::put_Parameter(unsigned short id, short val)
{
	switch(id)
	{
	case 1: 
		mono=val;
		break;
	case 2:
		width=val;
		break;
	}
	return NOERROR;
}

STDMETHODIMP Satellite::get_Parameter(unsigned short id, short *val)
{
	switch(id)
	{
	case 20:
		*val = m_width;
		break;
	case 21:
		*val = m_height;
		break;
	case 22:
		*val = m_videoWratio;
		break;
	case 23:
		*val = m_videoHratio;
		break;
	case 28: 
		*val = m_audio_channels;
		break;
	case 30:
		*val = m_bVideoInfo+m_bAudioInfo;
		break;
	}

	return NOERROR;
}

STDMETHODIMP Satellite::put_String(unsigned short id, LPSTR val)
{
	return NOERROR;
}

