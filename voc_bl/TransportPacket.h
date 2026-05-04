// TransportPacket.h: interface for the CTransportPacket class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_TRANSPORTPACKET_H__9134F3AC_6042_41A7_9C11_AC91EDAE0B83__INCLUDED_)
#define AFX_TRANSPORTPACKET_H__9134F3AC_6042_41A7_9C11_AC91EDAE0B83__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#pragma pack(1)
	typedef struct _TP {
		unsigned char sync;
		unsigned short pid;
		struct {
			unsigned char  contcnt: 4;
			unsigned char  adaptation: 2;
			unsigned char  scramble: 2;
		} flag;
		unsigned char rest[184];
	} TP;

	typedef struct _PAT {
		unsigned char id;
		unsigned short len;
		unsigned short streamid;
		unsigned char version;
		unsigned char sectionnum;
		unsigned char lastsection;
	} PAT;

	typedef struct _PATENTRY {
		unsigned short prognum;
		unsigned short pid;
	} PATENTRY;

	typedef struct _ADAPTATION {
		unsigned char len;
	} ADAPTATION;

	typedef struct _PMT {
		unsigned char id;
		unsigned short len;
		unsigned short prognum;
		unsigned char  version;
		unsigned char  sectionnum;
		unsigned char  lastsection;
		unsigned short pcr;
		unsigned short proglen;
	} PMT;

	typedef struct _PMTENTRY {
		unsigned char streamtype;
		unsigned short pid;
		unsigned short len;
	} PMTENTRY;


class CTransportPacket  
{
public:
	int GetPayLoadStart();
	PMTENTRY * GetPMTEntry(int i);
	PMT * GetPMTPayload();
	void AddPMTEntry(unsigned short prog, unsigned short vp, unsigned short ap, unsigned short pp);
	int m_packet_type;
	void GetBuffer(unsigned char *b);
	void AddPATEntry(unsigned short prog, unsigned short p);
	unsigned int GetCRC();
	PATENTRY * GetPATEntry(int i);
	void * GetPayload();
	unsigned short GetPID();
	int SyncOK();
	CTransportPacket(unsigned char *p);
	TP m_packet;
	CTransportPacket(unsigned short tpid=0);
	virtual ~CTransportPacket();
	void CalcCRC();
	void SetContCount(int cnt);

private:
};

#endif // !defined(AFX_TRANSPORTPACKET_H__9134F3AC_6042_41A7_9C11_AC91EDAE0B83__INCLUDED_)
