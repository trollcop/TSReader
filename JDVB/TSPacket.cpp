#include "TSPacket.h"


#include <string.h>

unsigned char TSPacket::PACKET_SIZE = 188;



//-------------------------------------------------------------------
TSPacket::TSPacket()
{
	Data_ = new char [PACKET_SIZE];
	Next_ = 0;
}
//-------------------------------------------------------------------
TSPacket::~TSPacket()
{
	delete [] Data_;
	delete Next_;


}
//-------------------------------------------------------------------
TSPacket::TSPacket(const TSPacket & Source)
{
	Data_ = new char [PACKET_SIZE];

	memcpy(Data_,Source.Data_,PACKET_SIZE);

}
//-------------------------------------------------------------------
bool TSPacket::operator ==(const TSPacket &Source)
{
	return !(memcmp(Data_, Source.Data_ ,PACKET_SIZE));
}
//-------------------------------------------------------------------
void TSPacket::update(const char *Data)
{
	if(Data == 0)
		return;

    memcpy(Data_,Data,PACKET_SIZE);
}
//-------------------------------------------------------------------
char * TSPacket::getData()
{
	return Data_;
}
//-------------------------------------------------------------------
unsigned short TSPacket::getPid()
{
	return (unsigned short) (((Data_[1] & 0x1f) << 8) | (unsigned short) (Data_[2] & 0xff));
}
//-------------------------------------------------------------------
unsigned char  TSPacket::getAdaptionLength()
{
	return Data_[4]; 
}
//-------------------------------------------------------------------
bool TSPacket::hasPayload()
{
	//FIXME
	return true;
}
//-------------------------------------------------------------------
unsigned char TSPacket::getScramblingType()
{
	return (unsigned char )(Data_[3] & 0xC0)>>6;
}
//-------------------------------------------------------------------
void TSPacket::setNext (TSPacket *Next)
{
	Next_ = Next;
}
//-------------------------------------------------------------------
