#include "Transponder.h"
#include "TSPidFilterQueue.h"
#include "TSPidFilter.h"
#include "TSSource.h"
#include "TSPacket.h"
#include "Programm.h"

#include <winerror.h>


//-------------------------------------------------------------------
Transponder::Transponder (const unsigned long Frequency, const unsigned long SymbolRate, const bool Vertical)
{
	Frequency_ = Frequency;
	SymbolRate_ = SymbolRate;
	Vertical_ = Vertical;
	Next_ = 0;
	Queue_ = 0;
	HeadProgramm_ = 0;
	ProcessedPackets_ = 0;
	DiscardedPackets_ = 0;
}
//-------------------------------------------------------------------
Transponder::~Transponder ()
{
	TSPidFilterQueue *CurrentQueue = Queue_;
	while(CurrentQueue)
	{
		CurrentQueue = Queue_->getNextTSPidFilterQueue ();
		delete Queue_;
		Queue_ = CurrentQueue;
	}
	Programm *CurrentProgramm = HeadProgramm_;
	while(CurrentProgramm)
	{
		CurrentProgramm  = HeadProgramm_->getNextProgramm ();
		delete HeadProgramm_;
		HeadProgramm_ = CurrentProgramm;
	}
}
//-------------------------------------------------------------------
HRESULT Transponder::insertProgramm (Programm *Prog)
{
	if(Prog == 0)
		return E_FAIL;

	Programm *newProgramm = Prog;
	Programm *Current = HeadProgramm_;


	while(Current)
	{
		if(Current == newProgramm)
			return E_FAIL;

		if(Current->isEqual (newProgramm))
			return E_FAIL;
		
		if(Current->getNextProgramm () == 0)
			break;
		Current = Current->getNextProgramm ();
		
	}
	if(HeadProgramm_ == 0)
	{
		HeadProgramm_ = Prog;
		HeadProgramm_->setNextProgramm (0);
		HeadProgramm_->setLastProgramm (0);
	}
	else
	{
		Current->setNextProgramm (Prog);
		Prog->setLastProgramm (Current);
		Prog->setNextProgramm (0);
	}
		
	return S_OK;
}
//-------------------------------------------------------------------
Transponder * Transponder::getNextTransponder () const
{
	return Next_;
}
//-------------------------------------------------------------------
HRESULT Transponder::deleteProgramm (Programm *Prog)
{
	if(Prog == 0 || HeadProgramm_)
		return E_FAIL;

	Programm *Current = HeadProgramm_;

	do
	{
		if(Current == Prog)
		{
			if(Current->getLastProgramm ())
			{
				Current->getLastProgramm ()->setNextProgramm (Current->getNextProgramm ());
				if(Current->getNextProgramm ())
					Current->getNextProgramm ()->setLastProgramm (Current->getLastProgramm ());
			}
			else
			{
				if(Current->getNextProgramm ())
					HeadProgramm_ = Current->getNextProgramm ();
				else
					HeadProgramm_ = 0;
			}
			delete Current;
			return S_OK;
		}
		Current = Current->getNextProgramm ();
	}while(Current);

	return E_FAIL;
}
//-------------------------------------------------------------------
HRESULT Transponder::registerTSPidFilter (const unsigned short Pid, TSPidFilter *Filter)
{
	if(Filter == 0)
		return E_FAIL;
	TSPidFilterQueue * Queue = 0;
	if(SUCCEEDED(findQueue(Pid,&Queue)))
	{
		Queue->insertTSPidFilter (Filter);
		return S_OK;
	}
	Queue = new TSPidFilterQueue(Pid);
	Queue->setNextTSPidFilterQueue (Queue_);
	Queue_ = Queue;
	Queue_->insertTSPidFilter (Filter);
		
	return S_OK;
}
//-------------------------------------------------------------------
HRESULT Transponder::unregisterTSPidFilter (TSPidFilter *Filter)
{
	if(Filter == 0)
		return E_FAIL;

	for(unsigned char PidIndex = 0; PidIndex < Filter->getPidCount (); PidIndex++)
	{
		TSPidFilterQueue *Queue = 0;
		if(FAILED(findQueue(Filter->getPid(PidIndex),&Queue)))
			continue;
		
		Queue->removeTSPidFilter (Filter);

		if(Queue->isEmpty ())
			removeQueue(Queue);
			
		
	}
	return S_OK;
}
//-------------------------------------------------------------------
HRESULT Transponder::deleteAllProgramm ()
{
	if(HeadProgramm_ == 0)
		return E_FAIL;

	Programm *CurrentProgramm = HeadProgramm_;
	while(CurrentProgramm)
	{
		CurrentProgramm  = HeadProgramm_->getNextProgramm ();
		delete HeadProgramm_;
		HeadProgramm_ = CurrentProgramm;
	}
	HeadProgramm_ = 0;
	return S_OK;
}

//-------------------------------------------------------------------
const bool Transponder::dispatch (TSPacket *Packet)
{
	if(Packet == 0)
		return false;

	TSPidFilterQueue *Queue = 0;
	if(FAILED(findQueue(Packet->getPid (),&Queue)))
	{
		DiscardedPackets_++;
		return false;
	}
	
	ProcessedPackets_++;
	Queue->dispatch (Packet);
	return true;
}
//-------------------------------------------------------------------
const unsigned long Transponder::getNumOfDiscardedPackets () const
{
	return DiscardedPackets_;
}
//-------------------------------------------------------------------
const unsigned long Transponder::getNumOfProcessedPackets () const
{
	return ProcessedPackets_;
}
//-------------------------------------------------------------------
HRESULT Transponder::findProgrammByName (const char *Name, Programm **Prog)
{
	if(HeadProgramm_ == 0 || Name == 0 || Prog == 0)
		return E_FAIL;

	Programm * Current = HeadProgramm_;

	do
	{
		if(Current->getName ())
		{
			if(strcmp(Current->getName (),Name) == 0)
			{
				*Prog = Current;
				return S_OK;
			}
		}
		Current = Current->getNextProgramm ();
	}
	while(Current);
	return E_FAIL;
}
//-------------------------------------------------------------------
HRESULT Transponder::findProgrammByPMT (const unsigned short PMTPid, Programm **Prog)
{
	if(Prog == 0 || HeadProgramm_ == 0)
		return E_FAIL;

	Programm * Current = HeadProgramm_;
	do
	{
		if(Current->getPMTPid () == PMTPid)
		{
			*Prog = Current;
			return S_OK;
		}

		Current = Current->getNextProgramm ();
	}
	while(Current);

	return E_FAIL;
}
//-------------------------------------------------------------------
HRESULT Transponder::findProgrammBySID (const unsigned short SIDPid, Programm **Prog)
{
	if(Prog == 0 || HeadProgramm_ == 0)
		return E_FAIL;

	Programm *Current = HeadProgramm_;

	do
	{
		if(Current->getSIDPid () == SIDPid)
		{
			*Prog = Current;
			return S_OK;
		}
		Current = Current->getNextProgramm ();
	}
	while(Current);

	return E_FAIL;
}
//-------------------------------------------------------------------
const unsigned long Transponder::getFrequency () const
{
	return Frequency_;
}
//-------------------------------------------------------------------
const unsigned long Transponder::getSymbolRate () const
{
	return SymbolRate_;
}

//-------------------------------------------------------------------
const bool Transponder::isVerticalPolarization () const
{
	return Vertical_;
}
//-------------------------------------------------------------------
HRESULT Transponder::notifyTransponderChange (const bool Select)
{
	
	return E_NOTIMPL;
}
//-------------------------------------------------------------------
void Transponder::setNextTransponder (Transponder *Next)
{
	Next_ = Next;
}
//-------------------------------------------------------------------
HRESULT Transponder::removeQueue (TSPidFilterQueue *Queue)
{
	if(Queue == 0)
		return E_FAIL;

	if(Queue_ == Queue)
	{
		delete Queue_;
		Queue_ = 0;
		return S_OK;
	}

	TSPidFilterQueue *Current = Queue_;

	while(Current)
	{
		if(Current->getNextTSPidFilterQueue () == Queue)
		{
			Current->setNextTSPidFilterQueue (Queue->getNextTSPidFilterQueue ());
			delete Queue;
			return S_OK;
		}

	}

	return E_FAIL;

}
//-------------------------------------------------------------------
HRESULT Transponder::findQueue(const unsigned short Pid, TSPidFilterQueue **Queue)
{
	if(Queue == 0 || Queue_ == 0)
		return E_FAIL;

	TSPidFilterQueue *Current = Queue_;

	do
	{
		if(Current->getPid () == Pid)
		{
			*Queue = Current;
			return S_OK;
		}


		Current = Current->getNextTSPidFilterQueue ();
	}
	while(Current);
	
	return E_FAIL;

}