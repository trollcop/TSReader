#ifndef UDPPIDFILTER_H___
#define UDPPIDFILTER_H___


#include "TSPidFilter.h"
#include <winsock2.h>

typedef long HRESULT;

typedef struct __rtpbits__
{
  unsigned int Version:2;           /* version: 2 */
  unsigned int Padding:1;           /* is there padding appended: 0 */
  unsigned int Extension:1;           /* number of extension headers: 0 */
  unsigned int CSRC:4;          /* number of CSRC identifiers: 0 */
  unsigned int Marker:1;           /* marker: 0 */
  unsigned int PayloadType:7;          /* payload type: 33 for MPEG2 TS - RFC 1890 */
  unsigned int Sequence:16;   /* sequence number: random */
}RTPBits;

typedef struct __rtpheader__ 
{	/* in network byte order */
  RTPBits Bits;
  int TimeStamp;	/* start: random */
  int SSRC;		/* random */
}RTPHeader;







class UDPPidFilter : public TSPidFilter
{
protected:
	SOCKET Socket_;
	unsigned long PacketsDelivered_;
	unsigned long PacketsLost_;
	unsigned short Port_;
	unsigned short Pid_;
	char *Name_;
	char *Address_;
	RTPHeader rtp_header_;

public:
	UDPPidFilter(unsigned short Pid, char *Name,
				 char *Address, unsigned short Port);

	virtual ~UDPPidFilter();
	
	void dispatch(TSPacket *Packet);

	const unsigned short getPort() const;
	const char * getAddress() const;
	
	void setPayloadType(int PayloadType);	



protected:
	HRESULT sendPacket(TSPacket *Packet);

	HRESULT createSocket();
private:
	UDPPidFilter(const UDPPidFilter &Source) 
		: TSPidFilter(0xFFFF,0)
	{}

};

extern HWND g_UDPPidDlg;


LRESULT CALLBACK UDPPidFilterProcedure(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);




#endif