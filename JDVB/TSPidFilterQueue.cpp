#include "TSPidFilterQueue.h"
#include "TSPidFilter.h"
#include "TSPacket.h"
#include "TSPacketQueue.h"

#include <winerror.h>


//------------------------------------------------------------------
TSPidFilterQueue::TSPidFilterQueue (unsigned short Pid)
{
	Pid_ = Pid;
	Head_ = 0;
	Next_ = 0;
}
//-------------------------------------------------------------------
const unsigned short TSPidFilterQueue::getPid() const
{
	return Pid_;
}
//-------------------------------------------------------------------

TSPidFilterQueue::~TSPidFilterQueue ()
{
}

//-------------------------------------------------------------------
HRESULT TSPidFilterQueue::insertTSPidFilter (TSPidFilter *Filter)
{
	if(Filter == 0)
		return E_FAIL;

	if(Head_ == 0) //empty queue
	{
		Head_ = Filter;
		Head_->setTSPidFilter (0);
		return S_OK;
	}


	TSPidFilter *Current = Head_;

	while(Current)
	{
		if(Current == Filter)
			return S_OK; //requested filter already exists
		else
			Current = Current->getNextTSPidFilter ();
	}

	//duplicate check completed successfully, lets insert it correctly

	Current = Head_;

	int FilterPriority = 0;
	
	for(unsigned char index = 0; index <Filter->getPidCount (); index++)
	{
		if(Filter->getPid (index) == Pid_)
		{
			FilterPriority = Filter->getPriority (index);
			break;
		}

	}
	
	TSPidFilter *LastFilter = Head_;


	int CurrentPriority = 0;
	while(Current)
	{
		for(unsigned char index = 0; index < Current->getPidCount (); index++)
		{	
			if(Current->getPid (index) == Pid_)
			{
				CurrentPriority = Current->getPriority (index);
				break;
			}
		}
		if(FilterPriority > CurrentPriority)
		{
			LastFilter->setTSPidFilter (Filter);
			Filter->setTSPidFilter (Current);
			return S_OK;
		}
		else
		{
			LastFilter = Current;
			Current = Current->getNextTSPidFilter ();

		}
	}

	LastFilter->setTSPidFilter (Filter);

	return S_OK;

}

//------------------------------------------------------------------
HRESULT TSPidFilterQueue::removeTSPidFilter (TSPidFilter *Filter)
{
	if(Filter == 0 || Head_ == 0)
		return E_FAIL;
	
	if(Filter == Head_)
	{
		Head_ = 0;
		return S_OK;
	}
	TSPidFilter *Current = Head_;
	TSPidFilter *LastFilter = Head_;


	while(Current)
	{
		if(Current == Filter)
		{
			if(Current->getNextTSPidFilter () == 0)
				LastFilter->setTSPidFilter (0);
			else
				LastFilter->setTSPidFilter (Current->getNextTSPidFilter ());
		
			return S_OK;
		}
		LastFilter = Current;
		Current = Current->getNextTSPidFilter ();
	}

	return E_FAIL;
}
//-------------------------------------------------------------------
void TSPidFilterQueue::dispatch (TSPacket *Packet)
{
	if(Head_)
		Head_->dispatch (Packet);
}
//-------------------------------------------------------------------
const bool TSPidFilterQueue::isEmpty ()
{
	if(Head_)
		return false;
	else
		return true;
}
//-------------------------------------------------------------------
void TSPidFilterQueue::setNextTSPidFilterQueue (TSPidFilterQueue *Next)
{
	Next_ = Next;
}
//-------------------------------------------------------------------
TSPidFilterQueue * TSPidFilterQueue::getNextTSPidFilterQueue () const
{
	return Next_;
}