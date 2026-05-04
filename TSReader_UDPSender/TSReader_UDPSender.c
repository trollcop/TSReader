#include <stdio.h>
#include <pcap.h>

char errbuf[PCAP_ERRBUF_SIZE];
pcap_t *fp = NULL;

int UDPSender_GetDevices(int nIndex, char * szName, char * szDescription)
{
	pcap_if_t *alldevs = NULL, *d;
	int  i = 0;

	//if (pcap_findalldevs(&alldevs, errbuf) == -1)
	if (pcap_findalldevs_ex(PCAP_SRC_IF_STRING, NULL, &alldevs, errbuf) == -1)
	{
		char szTemp[256];

		sprintf(szTemp, "UDPSender: Error in pcap_findalldevs_ex: %s\n", errbuf);
		OutputDebugString(szTemp);
		return -1;
	}

	for(d = alldevs; d; d = d->next)
	{
		if (nIndex == i++)
		{
			lstrcpy(szName, d->name);
			lstrcpy(szDescription, d->description);
			pcap_freealldevs(alldevs);
			return TRUE;
		}
	}
	pcap_freealldevs(alldevs);
	return FALSE;
}

int UDPSender_OpenDevice(char * szName)
{
	if ( (fp = pcap_open(szName,
						100, //snaplen
						PCAP_OPENFLAG_PROMISCUOUS, //flags
						1000, //read timeout
						NULL, // remote authentication
						errbuf)
						) == NULL)
	{
		OutputDebugString("UDPSender: Error opening adapter\n");
		return -1;
	}

	return TRUE;
}

int UDPSender_CloseDevice()
{
	if (fp != NULL)
	{
		pcap_close(fp);
		fp = NULL;
	}
	return TRUE;
}

int UDPSender_SendPacket(BYTE * pData, int nLength)
{
	if (pcap_sendpacket(fp, pData, nLength) != 0)
    {
		char szTemp[256];
        sprintf(szTemp ,"UDPSender: Error sending the packet: %d\n", pcap_geterr(fp));
        return FALSE;
    }
	return TRUE;
}
