#include "UDPPidFilter.h"

#include "TSPacket.h"
#include "TSPacketQueue.h"
#include "TSPidFilterQueue.h"
#include "Global.h"
#include "Resource.h"

#include <winerror.h>
#include <stdio.h>

#include <ws2tcpip.h>
#include <stdlib.h> /* rand() */

HWND g_UDPPidDlg = 0;

UDPPidFilter *g_cUDPFilter = 0;

//-------------------------------------------------------------------
UDPPidFilter::UDPPidFilter (unsigned short Pid,char *Name,
							char *Address, unsigned short Port)
							: TSPidFilter(Pid,Name)
{
	Socket_ = -1;
	if(Address == 0)
		return;

	PacketsDelivered_ = 0;
	PacketsLost_ = 0;

	if(FAILED(createSocket()))
		return;

	
	rtp_header_.Bits.CSRC = 0;
	rtp_header_.Bits.Extension = 0;
	rtp_header_.Bits.Marker = 0;
	rtp_header_.Bits.Padding = 0;
	rtp_header_.Bits.PayloadType = 33; //FIXME MPEG-2 TS
	rtp_header_.Bits.Sequence = rand() & 65535;
	rtp_header_.Bits.Version = 2; //FIXME hardcoded version 2
	rtp_header_.SSRC = rand();
	rtp_header_.TimeStamp = rand();

	

	struct sockaddr_in server;
	struct sockaddr_in local;
    struct hostent *hp;
	unsigned int addr;
	Port_ = Port;
	if (isalpha(Address[0])) 
	{   /* server address is a name */
        hp = gethostbyname(Address);
  
	}
    else  { /* Convert nnn.nnn address to a usable one */
       addr = inet_addr(Address);
        //hp = gethostbyaddr(Address,strlen(Address),AF_INET);
    }

	/*if (hp == NULL )
	{
		delete hp;
		return;
	}
	*/
	memset(&server,0,sizeof(server));
    memcpy(&(server.sin_addr),(const void *)&addr,4);
	Address_ = _strdup(Address);
    server.sin_family = AF_INET;
    server.sin_port = htons(Port);

	memset(&local,0,sizeof(local));
	local.sin_port = htons(Port);
	local.sin_family = AF_INET;
	local.sin_addr.S_un.S_addr = INADDR_ANY;


	int iLoop = 1;

	if(setsockopt(Socket_,SOL_SOCKET,SO_REUSEADDR, (const char *)&iLoop,sizeof(int) ) != 0)
	{
		closesocket(Socket_);
		Socket_ = -1;
		return;
	}

	char cTtl = 1; //FIXME
				   //multicast is limited to local network
	if(setsockopt(Socket_, IPPROTO_IP, IP_MULTICAST_TTL, &cTtl, sizeof(char)) != 0)
	{
		closesocket(Socket_);
		Socket_ = -1;
		return;
	}
	char cLoop = 1;
	if(setsockopt(Socket_, IPPROTO_IP, IP_MULTICAST_LOOP, &cLoop, sizeof(char)) != 0)
	{
		closesocket(Socket_);
		Socket_ = -1;
		return;
	}


	if (connect(Socket_,(struct sockaddr*)&server,sizeof(server)) == SOCKET_ERROR)
	{
        closesocket(Socket_);
		Socket_ = -1;
    }


}
//-------------------------------------------------------------------
UDPPidFilter::~UDPPidFilter ()
{
	if(Socket_ < 0)
		return;

	closesocket(Socket_);
}
//-------------------------------------------------------------------
void UDPPidFilter::dispatch (TSPacket *Packet)
{

#ifdef DEBUG
	ASSERT(Packet);
	ASSERT(Queue_);
#endif


	if(Packet->getPid () != Pid_)
	{
		Queue_->push (Packet);
		return;//there seems to be a problem -> lets push it back
	}

	if(FAILED(sendPacket(Packet)))
		DiscardedPackets_++;
	else
		ProcessedPackets_++;

	if(Next_)
		Next_->dispatch (Packet);
	else
		Queue_->push (Packet);	//avoid memory leaks

}

//-------------------------------------------------------------------
HRESULT UDPPidFilter::sendPacket (TSPacket *Packet)
{
	static char dest_buf[184 + sizeof(RTPHeader)];

	
	if(!Packet->hasPayload ())
		return E_FAIL;

	unsigned char AdaptionLength = Packet->getAdaptionLength ();

	char *PacketData = Packet->getData ();

	PacketData += 4 + AdaptionLength;



	int *rtp_cast  = (int *)&rtp_header_;

	int *dest_cast  = (int *)&dest_buf;

	dest_cast[0] = htonl(rtp_cast[1]);
	dest_cast[0] = htonl(rtp_cast[1]);

	//memcpy(dest_cast +2 ,
	//char *TSData = Packet->getData ();

	int Result = send(Socket_,Packet->getData (),188,0);
	
	if(Result == SOCKET_ERROR)
		return WSAGetLastError();
	else
		return S_OK;
}
//-------------------------------------------------------------------
HRESULT UDPPidFilter::createSocket ()
{
	Socket_ = socket(AF_INET,SOCK_DGRAM,0);

	if(Socket_ < 0)
		return E_FAIL;
	else
		return S_OK;
}


//-------------------------------------------------------------------
const unsigned short UDPPidFilter::getPort () const
{
	return Port_;
}

//-------------------------------------------------------------------
const char * UDPPidFilter::getAddress () const
{
	return Address_;
}

//-------------------------------------------------------------------

/********************************************************************
 * UDP Pid Filter Win32 Dialog Procedure
 *******************************************************************/
// Message handler for about box.
LRESULT CALLBACK UDPPidFilterProcedure(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	TCHAR Info [300];
	switch (message)
	{
		case WM_INITDIALOG:
			return true;

		case WM_TIMER:
		{
			switch(wParam)
			{
			case IDT_UDP_TIMER:
				{
					if(g_cUDPFilter == 0)
						break;
					sprintf(Info,"%u",g_cUDPFilter->getNumOfProcessedPackets (0));	//FIXME
                    SetDlgItemText(g_UDPPidDlg,IDC_PACKETS_DELIVERED,Info);
					sprintf(Info,"%u",g_cUDPFilter->getNumOfDiscardedPackets (0));  //FIMXE
					SetDlgItemText(g_UDPPidDlg,IDC_PACKETS_LOST, Info);
					sprintf(Info,"%s",g_cUDPFilter->getName (0)); //FIXME
					SetDlgItemText(g_UDPPidDlg,IDC_CONTENT_TYPE, Info);
                    sprintf(Info,"%u",g_cUDPFilter->getPort ());
					SetDlgItemText(g_UDPPidDlg,IDC_TARGET_PORT, Info);
					const char * Address = g_cUDPFilter->getAddress ();
					sprintf(Info,"%s", Address);
					SetDlgItemText(g_UDPPidDlg,IDC_TARGET_ADDRESS, Info);

				}

			}	
		break;
		}


	//SetDlgItemText
	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK)
		{
			EndDialog(hDlg, LOWORD(wParam));
			g_UDPPidDlg = 0;
			KillTimer(hWnd,IDT_UDP_TIMER);
			return TRUE;
		}
		break;
	}
	return FALSE;






}
