// TransportPacket.cpp: implementation of the CTransportPacket class.
//
//////////////////////////////////////////////////////////////////////

#include <windows.h>
#include <memory.h>
//#include <winsock2.h>
#include "TransportPacket.h"
#include "CRC.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CTransportPacket::CTransportPacket(unsigned short tpid)
{
	unsigned short p=tpid+0x4000;

	m_packet.pid = htons(p);
	m_packet.sync = 0x47;
	m_packet.flag.adaptation = 1;
	m_packet.flag.scramble = 0;
	m_packet.flag.contcnt = 0; 
	m_packet_type = 0;  // undetermined
	memset(m_packet.rest, 0xFF, sizeof(m_packet.rest));
	*m_packet.rest=0;  // pointer field
}

CTransportPacket::~CTransportPacket()
{

}

CTransportPacket::CTransportPacket(unsigned char *p)
{
	memcpy(&m_packet, p, sizeof(TP));
}

int CTransportPacket::SyncOK()
{
	if (m_packet.sync==0x47)
		return 1;
	else
		return 0;
}

unsigned short CTransportPacket::GetPID()
{
	unsigned short pid;
	pid = ntohs(m_packet.pid);
	pid &= 0x1FFF;

	return pid;
}

void * CTransportPacket::GetPayload()
{
	int adaptlen;
	ADAPTATION *padaptation;

	if ((m_packet.flag.adaptation==2) || (m_packet.flag.adaptation==3)) 
	{
		padaptation = (ADAPTATION *)&m_packet.rest[0];
		adaptlen = padaptation->len+1; 
	}
	else
		adaptlen = 1;

	return(&m_packet.rest[adaptlen]);
}

PATENTRY * CTransportPacket::GetPATEntry(int i)
{
	int adaptlen;
	ADAPTATION *padaptation;

	if ((m_packet.flag.adaptation==2) || (m_packet.flag.adaptation==3))
	{
		padaptation = (ADAPTATION *)&m_packet.rest[0];
		adaptlen = padaptation->len+1; 
	}
	else 
		adaptlen = 1;

	return((PATENTRY *)&m_packet.rest[adaptlen+sizeof(PAT)+i*sizeof(PATENTRY)]);

}

void CTransportPacket::CalcCRC()
{
	CCRC *crc = new CCRC();
	unsigned int c;
	unsigned int *pcrc;
	unsigned short len;

	PAT *ppat=(PAT *)GetPayload();
	len = ntohs(ppat->len);
	len &= 0x0FFF;
	//len -= 9;		// flags, crc;

	unsigned char *pb = ((unsigned char *)&m_packet);
	unsigned char *b = &pb[5];

	c = crc->Calc(b,len-1);
	pcrc = (unsigned int *)&b[len-1];
	*pcrc = htonl(c);
	free(crc);
}

unsigned int CTransportPacket::GetCRC()
{
	unsigned short len;
	unsigned int crc;
	int n;
	unsigned char *b;

	PAT *ppat=(PAT *)GetPayload();
	len = ntohs(ppat->len);
	len &= 0x0FFF;
	len -= 9;		// flags, crc;

	if (m_packet_type==1)
	{
		n = len/sizeof(PATENTRY);
		b = (unsigned char *)GetPATEntry(n);
	}
	else
	{
		n = len/sizeof(PMTENTRY);
		b = (unsigned char *)GetPMTEntry(n);
	}
	crc = *((unsigned int *)b);

	crc=0;

	return crc;

}

void CTransportPacket::AddPATEntry(unsigned short prog, unsigned short p)
{
	//this makes the packet a PAT packet
	m_packet_type = 1;

	PAT *ppat = (PAT *)GetPayload();
	PATENTRY *pent = (PATENTRY *)GetPATEntry(0);
	ppat->lastsection = 0;
	ppat->sectionnum  = 0;
	ppat->streamid    = htons(0x7B);
	ppat->version     = 0xC1;
	ppat->id          = 0;
	unsigned short len = sizeof(PATENTRY)+9;
	len |= 0xB000;
	ppat->len         = htons(len);

	pent->pid = htons(p|0xE000);
	pent->prognum = htons(prog);
	CalcCRC();
}

void CTransportPacket::GetBuffer(unsigned char *b)
{
	memcpy(b, &m_packet, 188);
}

void CTransportPacket::AddPMTEntry(unsigned short prog, unsigned short vp, unsigned short ap, unsigned short pp)
{
	unsigned short tmp;

	//this makes the packet a PMT packet
	m_packet_type = 2;

	PMT *ppmt = (PMT *)GetPayload();
	ppmt->id = 2;
	ppmt->lastsection = 0;
	ppmt->sectionnum = 0;
	ppmt->pcr = htons(pp|0xE000);
	ppmt->prognum = htons(prog);
	ppmt->version = 0xC1;
	ppmt->proglen = htons(0xF000);
	tmp = sizeof(PMTENTRY)*2;
	tmp += sizeof(PMT);
	tmp += 4-3;
	tmp |= 0xB000;
	ppmt->len = htons(tmp);
	PMTENTRY *pent = (PMTENTRY *)GetPMTEntry(0);
	pent->len = htons(0xF000);
	pent->pid = htons(vp|0xE000);
	pent->streamtype = 0x02;
	pent = (PMTENTRY *)GetPMTEntry(1);
	pent->len = htons(0xF000);
	pent->pid = htons(ap|0xE000);
	pent->streamtype = 0x04;
}

PMT * CTransportPacket::GetPMTPayload()
{
	int adaptlen;
	ADAPTATION *padaptation;

	if ((m_packet.flag.adaptation==2) || (m_packet.flag.adaptation==3))
	{
		padaptation = (ADAPTATION *)&m_packet.rest[0];
		adaptlen = padaptation->len; 
	}
	else
		adaptlen = 0;

	return((PMT*)&m_packet.rest[adaptlen+1]);
}

PMTENTRY * CTransportPacket::GetPMTEntry(int n)
{
	int adaptlen;
	ADAPTATION *padaptation;
	PMTENTRY *p;
	int i;
	int len;
	int plen;

	if ((m_packet.flag.adaptation==2) || (m_packet.flag.adaptation==3))
	{
		padaptation = (ADAPTATION *)&m_packet.rest[0];
		adaptlen = padaptation->len+1; 
	}
	else 
		adaptlen = 1;
	len = adaptlen+sizeof(PMT);
	p = (PMTENTRY *)&m_packet.rest[len];
	for (i=0; i<n; ++i)
	{
		plen = ntohs(p->len)&0x0FFF;
		len += sizeof(PMTENTRY)+plen;
		p = (PMTENTRY *)&m_packet.rest[len];
	}

	
	return(p);

}

void CTransportPacket::SetContCount(int cnt) {
	m_packet.flag.contcnt = cnt;
}

int CTransportPacket::GetPayLoadStart()
{
	unsigned short flag;
	unsigned short res;

	flag = ntohs(m_packet.pid);
	res = flag & 0x4000;
	
	if (res==0x4000)
		return 1;
	else 
		return 0;
}
