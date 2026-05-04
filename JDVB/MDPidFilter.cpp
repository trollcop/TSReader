#include "MDPidFilter.h"
#include "TSPacket.h"
#include "TSPacketQueue.h"
#include "TSPidFilterQueue.h"

#include <string.h>
#include <winerror.h>

//-------------------------------------------------------------------
MDPidFilter::MDPidFilter (unsigned short Pid, char *FilterName, CallBackProcedure Procedure)
: TSPidFilter(Pid,FilterName)
{
	Procedure_ = Procedure;
	Pid_ = Pid;
	
	if(FilterName)
		Name_ = _strdup(FilterName);
	else
		Name_ = 0;

}

//-------------------------------------------------------------------
MDPidFilter::~MDPidFilter()
{


}

//-------------------------------------------------------------------

void MDPidFilter::dispatch (TSPacket *Packet)
{

	/*
		here the individual TSPidFilter receive their packets. they can decide
		to either store it in an array or modify it. If they are done with 
		their modifiying / whatever, they should call TSPidFilterQueue to reinsert it
		into the dispatcher so that other filters, sharing the same Pid get access
		to the Packets. Since this filter is just a simple forwarder, we just send
		it to the callback procedure and the leave

		Note: if a filter detects the Packet has errors, it can automatically reinsert it back to
		the TSPacketQueue. In that case, it should never call the FilterQueue to insert it back

		@param index: this is needed to select the correct pid filter. this parameter
		should not be modified in any case, because it affects processing
		

	*/

#ifdef DEBUG
	ASSERT(Queue_);
#endif

	char *data = Packet->getData ();
	int Pid = Packet->getPid ();
	if(Procedure_)
		(Procedure_)(Pid, 184, &data[4]);

	if(Next_)
		Next_->dispatch (Packet);
	else
		Queue_->push (Packet);

}

//-------------------------------------------------------------------
 const int MDPidFilter::getPriority (const unsigned char PidIndex)
{
	/*
		this function returns the priority of the filter
		if it has no special needs then the priority is -1

	*/
	return TSPID_PRIORITY_LOW;	
}