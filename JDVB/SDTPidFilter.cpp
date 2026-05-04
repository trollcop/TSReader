#include "SDTPidFilter.h"
#include "TSPacket.h"
#include "TSPacketQueue.h"
#include "TSPidFilterQueue.h"


#include <stdio.h> /* _strdup */
#include <stdlib.h> /* exit */
#include <windows.h>
#include <winerror.h> /* E_FAIL S_OK, SUCCEEDED */
#include <wxdebug.h> /* ASSERT */

//-------------------------------------------------------------------
SDTPidFilter::SDTPidFilter (unsigned short Pid, char *PidName)
: TSPidFilter(Pid,PidName)
{
	Head_ = 0;
	done_ = false;
	initialized_ = false;
	HeadPacket_ = 0;
	TailPacket_ = 0;
	PacketCount_ = 0;
}
//-------------------------------------------------------------------
SDTPidFilter::~SDTPidFilter ()
{
	if(Head_ == 0)
		return;

	SDT * Current;

	do
	{
		Current = Head_->Next;
		delete Head_->Name;
		delete Head_;
		Head_ = Current;
	}
	while(Head_);


}
//-------------------------------------------------------------------
const int SDTPidFilter::getPriority (const unsigned char PidIndex) 
{
	return TSPID_PRIORITY_HIGH;
}
//-------------------------------------------------------------------
HRESULT SDTPidFilter::insertSDT (unsigned short SIDPid, unsigned char CAId, unsigned short TPId, char *Provider, char *Name)
{
	SDT * Current = Head_;

	while(Current)
	{
		if(Current->SIDPid == SIDPid && 
		   Current->CAId == CAId &&
		   Current->TPId == TPId)
		   return E_FAIL;
		
		if(Current->Next == 0)
			break;
		else
			Current = Current->Next;
	}

	if(Current == 0)
		Head_ = Current = new SDT();
	else
	{
		Current->Next = new SDT();
		Current = Current->Next ;
	}

	Current->CAId = CAId;
	Current->Next = 0;
	Current->SIDPid = SIDPid;
	Current->TPId = TPId;
	if(*Name)
		Current->Name = _strdup(Name);
	else
		Current->Name = 0;

	if(*Provider)
		Current->Provider = _strdup(Provider);
	else
		Current->Provider = 0;


	return S_OK;
}
//-------------------------------------------------------------------
const bool SDTPidFilter::isProcessingDone () const 
{
	return done_;
}
//-------------------------------------------------------------------
void SDTPidFilter::dispatch (TSPacket *Packet)
{

#ifdef DEBUG
	ASSERT(Packet);
	ASSERT(Queue_);
#endif

	if(Packet->getPid () != Pid_)
	{
		Queue_->push (Packet);
		return;
	}

	if(done_)
	{
		if(Next_)
			Next_->dispatch (Packet);
		else
			Queue_->push (Packet);
		return;
	}


	if(PacketCount_ < 20)
	{
		PacketCount_++;
		if(HeadPacket_ == 0)
			HeadPacket_ = TailPacket_ = Packet;
		else
		{
			TailPacket_->setNext (Packet);
			TailPacket_ = Packet;
			TailPacket_->setNext (0);
		}
		return;
	}
	else //extract the information and then dispatch them
	{
		TSPacket *Temp;
		extractSDT(HeadPacket_);
		while(HeadPacket_)
		{
			Temp = HeadPacket_->getNext ();
			if(Next_)
				Next_->dispatch (Packet);
			else
				Queue_->push (Packet);

			HeadPacket_ = Temp;
		}
		PacketCount_ = 0;
		HeadPacket_ = TailPacket_ = Packet;
	}
}
//-------------------------------------------------------------------
void SDTPidFilter::extractSDT (TSPacket *Packet)
{
	char *ptr = 0;
	char *buf = 0;	
	struct sdt_t *sdt = 0;
	struct sdt_descr_t *sdt_descr = 0;
	int sec_len = 0xFFFF;
	static int Loop = 0;
	int loop_len = 0;
	int descr_len = 0;
	static int position = 0;
	unsigned int ts_id = 0;
	unsigned short SID_Id = 0;
	unsigned char CA_ID = 0;
	char Name[50] = {0};
	char Provider[50] = {0};
	TSPacket *Current = Packet;
			
 	while (true)
	{
		if(ptr == 0)
		{
			ptr = Current->getData ();
			ptr +=5;// + Current->getAdaptionLength ();
			buf = ptr;
			sdt=(struct sdt_t *)ptr;
			if(sdt->table_id == TID_SDT_ACT || sdt->table_id == TID_SDT_OTH)
			{
				if(sdt->section_number == Loop)
				{
					initialized_ = true;
					sec_len = ((sdt->BitField1&0x03)<<8)+sdt->section_length_lo;
					ts_id = HILO (sdt->transport_stream_id);
					ptr += SDT_LEN;		
					if(sdt->last_section_number == Loop)
						done_ = true;
					Loop++;
				}
				else if(done_)
				{				
					//the SDT has already been processed
					//therefore the Loop is bigger than the requested
					
					ASSERT(sdt->last_section_number < Loop);
					return;
				}
				else
				{
					//we received a section which was not in our
					//estimated order
					//we skip this packet
					//since we parse the contents in order
					ptr += 184;
				}
			}
		}

		if(ptr - buf > 183)
		{
			Current = Current->getNext ();
			ptr = 0;
			if(Current == 0)
				break;
			continue;
		}
		sdt_descr = (struct sdt_descr_t *)ptr;
		SID_Id = HILO (sdt_descr->service_id);
		CA_ID = (sdt_descr->BitField2>>4)&1;
		loop_len = ((sdt_descr->BitField2&0x0f) << 8 ) + sdt_descr->descriptors_loop_length_lo;
		ptr += SDT_DESCR_LEN;
		descr_len = 0;
		while (descr_len < loop_len) 
		{
			char * buff = ptr; 			
			//parse_sdt_descr(ptr, loop_len - descr_len, j);
			switch(GET_DESCRIPTOR_TAG(ptr))
			{
				case DESCR_SERVICE:
				{
					if(GET_DESCRIPTOR_LENGTH(ptr) == 0)
					{
						ptr += 2;
						descr_len += 2;
						//position += 2;
						break;
					}
						buff += DESCR_GEN_LEN + 1;
					if(*buff > 0)
					{
						Provider[*buff] = '\0';
						memcpy(Provider,&buff[1],*buff);
						buff += *buff +1; //skip ServiceProvider String
					}
					else
					{
						buff += 2; //FIXME
						Provider[0] = '\0';
					}	
					descr_len += GET_DESCRIPTOR_LENGTH(ptr) + DESCR_GEN_LEN;
					ptr += GET_DESCRIPTOR_LENGTH(ptr) + DESCR_GEN_LEN;
				//	position += GET_DESCRIPTOR_LENGTH(ptr) + DESCR_GEN_LEN;
					if(*buff > 0)
					{
						Name[*buff] = '\0';
						memcpy(Name,&buff[1],*buff);
					}
					else
						Name[0] = '\0';
					insertSDT(SID_Id, CA_ID,ts_id,Provider,Name);
					break;
				}
				default:
				{
					//an invalid descriptor was found
					while(ptr[0] != 0x48 && descr_len < loop_len)
					{
						if(ptr[5] == 0x48)
						{
							descr_len = loop_len;
							break;
						}
						//position++;
						ptr +=1;
						descr_len++;
					}
					
				} /* </default> */
 			
			} /* </switch> */
 		 			 	
		} /* while (descr_len < loop_len) */
	}	/* while (position < sec_len) */
	
	
}
//-------------------------------------------------------------------
const SDT * SDTPidFilter::getSDTInfo () const
{
	return Head_;
}
//-------------------------------------------------------------------