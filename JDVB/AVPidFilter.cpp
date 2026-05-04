#include "AVPidFilter.h"
#include "TSPacket.h"
#include "TSPacketQueue.h"
#include "TSPidFilterQueue.h"


//---------------------------------------------------------------
AVPidFilter::AVPidFilter (const unsigned short Pid, const char *PidName)
: TSPidFilter(Pid,PidName)
{
	Filter_ = 0;
}
//---------------------------------------------------------------
AVPidFilter::~AVPidFilter ()
{
}
//---------------------------------------------------------------
void AVPidFilter::dispatch (TSPacket *Packet)
{

	if(Packet == 0)
		return;
	
#ifdef DEBUG
	ASSERT(Queue_);
#endif

	if(Packet->getPid () != Pid_)
	{
		Queue_->push (Packet);
		DiscardedPackets_++;
		return;
	}
	

	if(Filter_)
	{
		Filter_->processPid (Packet);
		ProcessedPackets_++;
	}
	else
	{
		if(Next_)
			Next_->dispatch (Packet);
		else
		{
#ifdef DEBUG
			ASSERT(Queue_);
#endif
			Queue_->push (Packet);
		}
		DiscardedPackets_++;
	}
}

//---------------------------------------------------------------
const int AVPidFilter::getPriority (unsigned char PidIndex)
{
	return TSPID_PRIORITY_LOW;
}
//---------------------------------------------------------------
void AVPidFilter::setFilter (ISourceFilter *Filter)
{
	Filter_ = Filter;
}
//---------------------------------------------------------------
//this function is usally called when registering
//the TSPacketQueue by the TSSource class.
void AVPidFilter::setTSPacketQueue(TSPacketQueue *Queue)
{
	if(Filter_)
		Filter_->setTSPacketQueue (Queue);

	Queue_ = Queue;

}
//----------------------------------------------------------------
//this function has to be overriden to provide the
//CSourceStreamPin class the information of an the next TSPidFilter
//in the chain
void AVPidFilter::setTSPidFilter(TSPidFilter *Next)
{
	if(Filter_)
		Filter_->setNextTSPidFilter (Next);

	Next_ = Next;


}
