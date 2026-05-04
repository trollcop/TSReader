#include "CSAPidFilter.h"

#include "TSPacket.h"
#include "TSPacketQueue.h"
#include "TSPidFilterQueue.h"
#include "TSPidFilter.h"

#include <winerror.h>


/*
	Status: UNIMPLEMENTED
*/



const int CSAPidFilter::MAX_BUFFERED_PACKETS = 30;


//-------------------------------------------------------------------
CSAPidFilter::CSAPidFilter (unsigned short Pid, char *PidName)
: TSPidFilter(Pid,PidName)
{
	OddKeyQueue_ = new TSPacketQueue();
	EvenKeyQueue_ = new TSPacketQueue();
	OriginalQueue_ = 0;
}
//-------------------------------------------------------------------
CSAPidFilter::~CSAPidFilter ()
{
	if(OriginalQueue_ == 0)
		return;

	//bad thing redispatching the queue on destructor
	HRESULT hr = S_OK;
	TSPacket *BufferedPacket = 0;
	
	while(1)
	{
		if(FAILED(EvenKeyQueue_->pull (&BufferedPacket)))
			break;
		OriginalQueue_->push(BufferedPacket);
	}
	delete EvenKeyQueue_;
	BufferedPacket = 0;
	
	while(1)
	{
		if(FAILED(OddKeyQueue_->pull (&BufferedPacket)))
			break;
		OriginalQueue_->push(BufferedPacket);
	}
	delete OddKeyQueue_;	

}
//-------------------------------------------------------------------
void CSAPidFilter::dispatch (TSPacket *Packet)
{
	if(Packet == 0)
		return;

#ifdef DEBUG
	ASSERT(Queue_);
#endif

	unsigned char ScrambleType = Packet->getScramblingType ();

	switch (ScrambleType)
	{

		case 0: //packet is not scrambled
		case 1: //reserved
		{
			DiscardedPackets_ ++;
			if(Next_)
				Next_->dispatch (Packet);
			else
				Queue_->push (Packet);
			return;
		}
		case 2: //even key scrambled
		{
			if(EvenKeyValid_)
			{
				if(FAILED(decrypt(Packet,EvenKey_)))
				{
					// decryption failed -> lets insert it into our
					// EvenKeyQueue -> if the EvenKeyQueue is already to big
					// then dump one Packet
					//FIXME
					if(Next_)
						Next_->dispatch (Packet);
					else
						Queue_->push (Packet);
					return;
				}
				else
				{
					//successfuly decrypted :-)
					
					if(Next_)
						Next_->dispatch (Packet);
					else
						Queue_->push (Packet); //DAMN
					return;
				}
			}
			else
			{
				//baby we need a key
				//lets insert it into our EvenKeyQueue -> if the queue
				//gets to big, then just dump one packet
				//FIXME
				if(Next_)
					Next_->dispatch (Packet);
				else	
					Queue_->push (Packet);
				return;
			}
		
		}
		case 3: //odd key scrambled
		{
			if(OddKeyValid_)
			{
				if(FAILED(decrypt(Packet,OddKey_)))
				{
					// decryption failed -> lets insert it into our
					// OddKeyQueue -> if the OddKeyQueue is already to big
					// then dump one Packet

				}
				else
				{
					//successfuly decrypted :-)
			
					if(Next_)
						Next_->dispatch (Packet);
					else
						Queue_->push (Packet); //DAMN decrypted for nobody
					return;
				}
			}
			else
			{
				//baby we need a key
				//lets insert it into our OddKeyQueue -> if the queue
				//gets to big, then just dump one packet
				//FIXME
				if(Next_)
					Next_->dispatch (Packet);
				else
					Queue_->push (Packet);
				return;
			}


		}
	}



}
//-------------------------------------------------------------------
const int CSAPidFilter::getPriority (const unsigned char PidIndex)
{
	return TSPID_PRIORITY_HIGH;
}
//-------------------------------------------------------------------
HRESULT CSAPidFilter::decrypt (TSPacket *Packet , char *Key)
{
	return E_NOTIMPL;

}
//-------------------------------------------------------------------
