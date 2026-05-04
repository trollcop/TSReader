#include "TSPacket.h"
#include "TSPacketQueue.h"

#include <winerror.h>

//-------------------------------------------------------------------
TSPacketQueue::TSPacketQueue(unsigned short PacketQueueSize)
{
	TotalPacketQueueSize_ = 0;
	HeadTSPacket_ = 0;
	TailTSPacket_ = 0;
	CurrentPacketQueueSize_ = 0;
	increaseQueue(PacketQueueSize);
}
//-------------------------------------------------------------------
TSPacketQueue::TSPacketQueue()
{
	TotalPacketQueueSize_ = 0;
	HeadTSPacket_ = 0;
	TailTSPacket_ = 0;
	CurrentPacketQueueSize_ = 0;

}
//-------------------------------------------------------------------
TSPacketQueue::~TSPacketQueue()
{
	//while(SUCCEEDED(decreaseQueue(0xFFFF)))
	//	;

}
//-------------------------------------------------------------------
HRESULT TSPacketQueue::increaseQueue(unsigned short PacketQueue)
{
	TSPacket * Current = 0;
	for(int index = 0; index < PacketQueue; index++)
	{
		Current = new TSPacket();
		TotalPacketQueueSize_++;
		CurrentPacketQueueSize_++;
		if(HeadTSPacket_ == 0)
		{
			HeadTSPacket_ = TailTSPacket_ = Current;
			Current->setNext (0);
			continue;
		}
		
		TailTSPacket_->setNext (Current);
		TailTSPacket_ = Current;
	}

	return S_OK;
}
//-------------------------------------------------------------------
HRESULT TSPacketQueue::decreaseQueue(unsigned short PacketQueue)
{
	if(HeadTSPacket_ == 0)
		return E_FAIL;

	TSPacket *Current = 0;
	while(HeadTSPacket_ != 0 && PacketQueue-- > 0)
	{
		Current = HeadTSPacket_;
		HeadTSPacket_ = HeadTSPacket_->getNext ();
		delete Current;
		PacketQueue--;
		TotalPacketQueueSize_--;
		CurrentPacketQueueSize_--;

	}
	if(HeadTSPacket_ == 0)
		TailTSPacket_ = 0;

	if(PacketQueue == 0)
		return S_OK;
	else 
		return E_FAIL;
}
//-------------------------------------------------------------------
HRESULT TSPacketQueue::pull(TSPacket **Packet)
{
	if(HeadTSPacket_ == 0 || TailTSPacket_ == 0)
	{
		return E_FAIL;
	}

	*Packet = HeadTSPacket_;
	
	if(HeadTSPacket_ == TailTSPacket_)
		HeadTSPacket_ = TailTSPacket_ = 0; //last packet
	else
		HeadTSPacket_ = HeadTSPacket_->getNext ();

	CurrentPacketQueueSize_--;

	return S_OK;
}
//-------------------------------------------------------------------
HRESULT TSPacketQueue::push(TSPacket *Packet)
{
	if(Packet == 0)
		return E_FAIL;

	if(TailTSPacket_)
	{
		TailTSPacket_->setNext (Packet);
		TailTSPacket_ = Packet;
	}
	else //queue was empty
	{
		HeadTSPacket_ = TailTSPacket_ = Packet;
		CurrentPacketQueueSize_ = 0;
	}
	CurrentPacketQueueSize_++;
	TailTSPacket_->setNext (0);
	return S_OK;
}

//-------------------------------------------------------------------
const int TSPacketQueue::getCurrentQueueSize() const
{
	return CurrentPacketQueueSize_; 
}