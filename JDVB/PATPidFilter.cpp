#include "PATPidFilter.h"
#include "TSPacket.h"
#include "TSPacketQueue.h"
#include "TSPidFilterQueue.h"


#include <winerror.h>


//-------------------------------------------------------------------
PATPidFilter::PATPidFilter (unsigned short Pid, char *PidName)
: TSPidFilter(Pid,PidName)
{
	Head_ = 0;
	initialized_ = false;
	done_ = false;
}
//-------------------------------------------------------------------
PATPidFilter::~PATPidFilter ()
{
	PATInfo *Current = Head_;

	while(Head_)
	{
		Current = Head_->Next ;
		delete Head_;
		Head_ = Current;
	}
}
//-------------------------------------------------------------------
const int PATPidFilter::getPriority (const unsigned char PidIndex)
{
	return TSPID_PRIORITY_HIGH;

}
//-------------------------------------------------------------------
void PATPidFilter::insertPAT (unsigned short SIDPid,unsigned short PMTPid, unsigned short TPId)
{

	PATInfo *Current = Head_;

	if(PMTPid == 0 || SIDPid == 0 || TPId == 0)
		return;


	while(Current)
	{
		if(Current->PMTPid == PMTPid && Current->SIDPid == SIDPid && Current->TPId == TPId)
			return;
		if(Current->Next == 0)
			break;
		else
			Current = Current->Next ;
	}

	if(Current == 0)
		Head_ = Current = new PATInfo();
	else
	{
		Current->Next = new PATInfo();
		Current = Current->Next ;
	}

	Current->Next = 0;
	Current->PMTPid = PMTPid;
	Current->SIDPid = SIDPid;
	Current->TPId = TPId;
		
}
//-------------------------------------------------------------------
void PATPidFilter::dispatch (TSPacket *Packet)
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

	char * ptr = Packet->getData ();
	int sec_len = 0;
	static unsigned char loop = 0;
	PATTable *pat_t = 0;
	PATProgramm *pat_p;
	ptr += 5;

	pat_t = (PATTable *)ptr;


	if(pat_t->TableId != TID_PAT)
	{
		Queue_->push(Packet);
		DiscardedPackets_++;
		return;
	}
		
	sec_len = ((pat_t->BitField1 & 0x0F) << 8) + pat_t->SectionLength_lo; 
		
	if(sec_len < 12)
	{
		Queue_->push (Packet);
		DiscardedPackets_++;
		return;
	}

	if(pat_t->SectionNumber == loop)
	{
		if(loop == pat_t->last_section_number && initialized_)
			done_ = true;

			if(initialized_ == false)
				initialized_ = true;
		
			int program_count = (sec_len - 8) / 4; //CRC_LEN

			ptr +=4;

			for(int index = 0; index <program_count; index++)
			{
				pat_p = (PATProgramm *)ptr;

				insertPAT(HILO(pat_p->ProgrammNumber),
						 ((pat_p->BitField1 & 0x1f) << 8) + pat_p->NetworkPid_lo,
						 HILO(pat_t->TransportStreamId));

				ptr += 4;
			}
			loop++;
		}
		done_ = initialized_; 


		if(Next_)
			Next_->dispatch (Packet);
		else
			Queue_->push (Packet);
		return;


}
//-------------------------------------------------------------------
const bool PATPidFilter::isProcessingDone () const
{
	return done_;
}
//-------------------------------------------------------------------
const PATInfo * PATPidFilter::getPATInfo () const
{
	return Head_;
}