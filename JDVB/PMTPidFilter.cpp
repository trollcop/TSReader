#include "PMTPidFilter.h"
#include "TSPacket.h"
#include "TSPacketQueue.h"
#include "TSPidFilterQueue.h"


#include <string.h>
#include <winerror.h>






//-------------------------------------------------------------------
PMTPidFilter::PMTPidFilter (unsigned short Pid, char *Name)
: TSPidFilter(Pid,Name)
{
	done_ = false;
	initialized_ = false;
	Info_ = new PMT();
	memset(Info_,0,sizeof(PMT));
}
//-------------------------------------------------------------------
PMTPidFilter::~PMTPidFilter ()
{

}
//-------------------------------------------------------------------
const PMT * PMTPidFilter::getPMTInfo () const
{
	return Info_;
}

//-------------------------------------------------------------------
void PMTPidFilter::dispatch (TSPacket *Packet)
{

#ifdef DEBUG
	ASSERT(Packet);
	ASSERT(Queue_);
#endif


	if(FAILED(extractPMTInfos(Packet)) || Next_ == 0)
	{	
		Queue_->push (Packet);
	}
	else
		Next_->dispatch (Packet);
}

//-------------------------------------------------------------------
const int PMTPidFilter::getPriority (const unsigned char PidIndex)
{
	return TSPID_PRIORITY_HIGH;
}
//-------------------------------------------------------------------
const bool PMTPidFilter::isProcessingDone() const
{
	return done_;
}
//-------------------------------------------------------------------
HRESULT PMTPidFilter::extractPMTInfos (TSPacket *Packet)
{
	//TODO:
	//Implement PMT parsing
	//should return E_FAIL when packet data is invalid
	int loop = 0;
	int sec_len;
	char *ptr = Packet->getData ();
	ptr += 5 + Packet->getAdaptionLength ();
	unsigned short info_length = 0;
	PMTTable * pmt_t = (PMTTable *)ptr;
	
	if(pmt_t->TableId != TID_PMT)
		return E_FAIL;


	sec_len = ((pmt_t->BitField1 & 0x0F) << 8 ) + pmt_t->SectionLength_lo ;

	info_length = ((pmt_t->BitField4 & 0x0F) << 8) + pmt_t->ProgrammInfoLength_lo ;

	if(sec_len < info_length + 12)
	{
		done_ = true;
		return S_OK;
	}

	if(pmt_t->SectionNumber == loop)
	{
		if(loop == pmt_t->LastSectionNumber && initialized_)
			done_ = true;

		loop++;		
		
		initialized_ = true;


		Info_->PCRPid = ((pmt_t->BitField3 & 0x1F >> 1) << 8) + pmt_t->PCR_PID_lo ;
		unsigned short program_nr = HILO(pmt_t->ProgrammNumber); 

		ptr += PMT_TABLE_LEN;

		extractCADescriptor(ptr,info_length);


		if(info_length > 0)
			ptr += info_length;
		int stream_len = (sec_len - info_length -12);

		PMTInfo * pmt_i;
		unsigned char stream_type;
		unsigned short pid;
		while(stream_len > 0)
		{
			pmt_i = (PMTInfo *)ptr;		
			ptr +=5;
			
			stream_type = pmt_i->StreamType;
			pid = ((pmt_i->BitField1 & 0x1f) << 8) + pmt_i->Elementary_PID_lo ;
			info_length = ((pmt_i->BitField2 & 0x0F) << 8) + pmt_i->ES_Info_Length ;
		
			extractCADescriptor(ptr,info_length);


			switch(stream_type)
			{
				case 2:
				{
					if(Info_->VideoPid == 0)
						Info_->VideoPid = pid;
					break;
				}
				case 3:
				case 4:
				{
					if(Info_->AudioPid == 0)
						Info_->AudioPid = pid;
					break;
				}
				case 6:
				{
					if(Info_->TeleTextPid == 0)
						Info_->TeleTextPid = pid;
					break;
				}
				default:
					break;
			}
			stream_len -= 5 + info_length;
			ptr +=info_length;				
		}
	}
	done_ = initialized_;

	return S_OK;
}
//-------------------------------------------------------------------
void PMTPidFilter::extractCADescriptor(char * ptr, int info_length)
{
	struct pmt_ca_info_struct *ca_info;

	int i;
	unsigned char *MyPtr;
	unsigned short CA_Typ;
	unsigned short CA_ECM;
	if ( info_length < 6 ) 
		return;
	i=0;
	MyPtr=(unsigned char *)ptr;
	while ( i+PMT_CA_info_LEN  <= info_length )
	{
		ca_info=(struct pmt_ca_info_struct *)(MyPtr);
		switch ( ca_info-> stream_type ) 
		{
			case 0x09 :
			{
				if ( ca_info->len >= 4 )
				{
					CA_Typ = ( ca_info->CA_Ident_hi<<8)+ca_info->CA_Ident_lo;
					if(Info_->ECMPid == 0)
						Info_->ECMPid = (( ca_info->ECM_hi&0x1f)<<8)+ca_info->ECM_lo;
			        break;
				}
			}
			 case 0x0a :
			{        // Crypt-Land
				break;
			}
		
		}
		
		MyPtr+=(ca_info->len+2);
		i+=(ca_info->len+2);
	}

}





