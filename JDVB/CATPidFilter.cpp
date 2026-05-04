#include "CATPidFilter.h"
#include "TSPacket.h"
#include "TSPacketQueue.h"
#include "TSPidFilterQueue.h"

#include <winerror.h>


/*
	Status: UNIMPLEMENTED
*/


//-------------------------------------------------------------------
CATPidFilter::CATPidFilter (unsigned short Pid, char *Name)
: TSPidFilter(Pid,Name)
{

}
//-------------------------------------------------------------------
CATPidFilter::~CATPidFilter ()
{


}
//-------------------------------------------------------------------
int CATPidFilter::getPriority ()
{
	return -1;
}
//-------------------------------------------------------------------
void CATPidFilter::dispatch (TSPacket *Packet)
{
	if(Packet == 0)
		return;

#ifdef DEBUG
	ASSERT(Queue_);
#endif


	if(Pid_ != Packet->getPid ())
	{
		Queue_->push (Packet);
		return;
	}

	if(Next_)
		Next_->dispatch (Packet);
	else
		Queue_->push (Packet);

}
//-------------------------------------------------------------------
HRESULT CATPidFilter::extractCATInfos (TSPacket *Packet)
{
	//FIXME
	//here we should extract the CAT infos from the stream
	return E_NOTIMPL;
}
//-------------------------------------------------------------------
const int CATPidFilter::getCATInfoCount () const
{
	return CATInfoCount_ ;
}
//-------------------------------------------------------------------
const CATInfo * CATPidFilter::getCATInfo () const
{
	return Header_;
}
//-------------------------------------------------------------------
HRESULT CATPidFilter::insertCATInfo (unsigned short CA_ID, unsigned short EMM)
{
	//FIXME
	//here we insert a new cat we the requested cat has not already been inserted
	return E_NOTIMPL;
}