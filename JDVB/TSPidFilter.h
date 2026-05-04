
#ifndef PIDFILTER_H___
#define PIDFILTER_H___

#include <string.h>

class TSPacket;
class TSPacketQueue;
class TSPidFilterQueue;

typedef long HRESULT;

#define TSPID_PRIORITY_HIGH			4
#define TSPID_PRIORITY_NORMAL		3
#define TSPID_PRIORITY_LOW			2
#define TSPID_PRIORITY_LOWEST		1

#define PID_TYPE_PAT		    0x0
#define PID_TYPE_PMT		    0x1
#define PID_TYPE_AUDIO		    0x1C
#define PID_TYPE_AUDIO_MPEG1	0x4
#define PID_TYPE_AUDIO_MPEG2	0x8
#define PID_TYPE_AUDIO_AC3		0x10
#define PID_TYPE_VIDEO			0xE0
#define PID_TYPE_VIDEO_MPEG1	0x40
#define PID_TYPE_VIDEO_MPEG2	0x80
#define PID_TYPE_PCR			0x100

#define UNREFERENCED_PARAMETER(P)	(P)

#include <winerror.h>

/*********************************************************************************
 *                                                                               *
 * TSPidFilter.h  Transponder Stream Pid Filter                                  *
 *                                                                               *
 * Copyright (C) 2003      Ware4z                                                *                                                                               *
 *                                                                               *
 * This program is free software; you can redistribute it and/or                 *
 * modify it under the terms of the GNU General Public License                   *
 * as published by the Free Software Foundation; either version 2                *
 * of the License, or (at your option) any later version.                        *
 *                                                                               *
 *                                                                               *          
 * This program is distributed in the hope that it will be useful,               *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of                *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the                 *
 * GNU General Public License for more details.                                  *
 *                                                                               *
 *                                                                               *
 * You should have received a copy of the GNU General Public License             *
 * along with this program; if not, write to the Free Software                   *
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.    *
 * Or, point your browser to http://www.gnu.org/copyleft/gpl.html                *
 *                                                                               *
 *                                                                               *
 * The author can be reached at j_anderw@sbox.tugraz.at                          *
 *********************************************************************************/ 


/********************************************************************
 * Class TSPidFilter
 *
 * This class is an abstract base class for all kinds of work. In general
 * this class will get access to RAW MPEG-2 Transponder stream of any
 * concrete TSSource class. A TSPidFilter can do whatever it wants with
 * the received TSPackets. However after the processing has been done, it
 * should then release the packet. Releasing means, that the Packet is either
 * dispatched to another filter, or it is sent back to original TSPacketQueue
 * There are 3 types of actions a TSPidFilter can do with an TSPacket:
 * a) push it immediately back the TSPacketQueue (recommended behavior
 *    if the packet data is invalid or useless)
 * b) alter the contents and dispatch to another filter by calling
 *    the pointer to the next filter
 * c) store it internaly for further usage (if this done, then it should
 *     only store about a 1/20 since this can lead to an deadlock :
 *	  TSSource class waits for a new packet, but the queue is empty -> 
 *	  TSPidFilter waits for a new Packet -> but the TSPidFilterQueue does not
 *	  receive packets since the TSSource does not send anything -> therfore
 *	  TSPidFilter does not release the packets....)
 *
 * However, it should never ever dipatch to Packet twice to the 
 * TSPacketQueue or to the next TSPidFilter and dispatch to both locations
 * By doing this, TSPidFilters will receive the packets at the same, thus
 * making operation impossible (in the end the segmentation violation will
 * be the climax due to fact that the TSPacketQueue will delete all remaining
 * packets and so deleting a packet twice...)
 *
 * TSPidFilter Priorities:
 *
 * By adding a priority scheme to filters, filters with important task like
 * decryption , CAT filtering and etc can ensure that they receive first. 
 *
 * Currently, the suggested Priority layout is as following:
 *
 * TSPID_PRIORITY_HIGH: decryption filters(CSA), program information filters
 *                      CSA, PAT, PMT, CAT filters 
 * TSPID_PRIORITY_NORMAL: net streaming filters (UDP, RTP / TCP)
 *						  
 * TSPID_PRIORITY_LOW: filters for the playback engine
 *
 * TSPID_PRIORITY_LOWEST: file writing filters 
 * 
 *
 *
 *******************************************************************/


class TSPidFilter
{
protected:
	unsigned short Pid_; 
	char *Name_;
	unsigned long ProcessedPackets_;
	unsigned long DiscardedPackets_;
	unsigned char Priority_;
	TSPidFilter *Next_;
	TSPacketQueue *Queue_;
public:
	TSPidFilter(const unsigned short Pid, const char *Name)
	{
		Pid_ = Pid;
		if(Name_)
			Name_ = _strdup(Name);
		else
			Name_ = 0;
		ProcessedPackets_ = 0;
		DiscardedPackets_ = 0;
		Next_ = 0;
	}
	virtual ~TSPidFilter()
	{}
	//---------------------------------------------------------------
	/*
		getPid
		Description: this function returns the Pid for a given PidIndex
		This allows the usage of filters, which are interested in multiple
		streams (etc...)

	*/
	virtual const unsigned short getPid(const unsigned char PidIndex) 
	{
		UNREFERENCED_PARAMETER(PidIndex);	
		return Pid_;
	}
	//----------------------------------------------------------------
	/*
		setQueue
		Description: this function sets the default TSPacketQueue for a
		TSPidFilter. When a TSPidFilter detects an invalid TSPacket or 
		it is the last filter holding execution on that TSPacket it
		must push the TSPacket back to the TSPidFilter

		Note: This function is only supposed to be called by the TSSource
		      class
	*/
	virtual void setTSPacketQueue(TSPacketQueue *Queue)
	{Queue_ = Queue;}
	//---------------------------------------------------------------
	/*
		setNext
		Description: sets the Pointer to following filter

		Note: this function is only to be called by TSPidFilterQueue class
	*/
	virtual void setTSPidFilter(TSPidFilter *Next)
	{
		Next_ = Next;
	}
	//---------------------------------------------------------------
	TSPidFilter * getNextTSPidFilter() const
	{
		return Next_;
	}


	//---------------------------------------------------------------
	/*
		getPidCount
		Description: this function returns the amount of active pid filters
		available on this filter
		
		Note: filters that process more than one TS Pid, must override
		this function

	*/
	virtual const unsigned char getPidCount()
	{return 1;}

	//---------------------------------------------------------------
	/*
		getName
		Description: returns a name for the filter
		
		Note: filters that process more than one TSPid, must override
		this function in order to provide correct names
	
	*/
	virtual const char *getName(const unsigned char PidIndex)
	{
		UNREFERENCED_PARAMETER(PidIndex);	
		return Name_;
	}

	//---------------------------------------------------------------
	/*
		getNumOfProcessedPackets:
		returns the number of successfully processed packets
		Note: filters that process more than one TSPid, should override
		this function
	*/
	virtual const unsigned long getNumOfProcessedPackets(const unsigned char PidIndex) 
	{
		UNREFERENCED_PARAMETER(PidIndex);	
		return ProcessedPackets_;
	}
	//---------------------------------------------------------------
	/*
		getNumOfDiscardedPackets
		returns the number of packets which failed to process
		Note: filters that process more than TSPid, should override this function
	*/
	virtual const unsigned long getNumOfDiscardedPackets(const unsigned char PidIndex)
    {
		UNREFERENCED_PARAMETER(PidIndex);	
		return DiscardedPackets_;
	}

	//---------------------------------------------------------------
	/*
		dispatch

		Description: this function actually receives the TSPacket from the
		TSPidFilterQueue. Because it is pure virtual, the derrived class must
		implement this function. If the derrived class has finished the processing,
		it has to either deliver it back to TSPacketQueue or submit it to the next 
		TSPidFilter. If the TSPidFilter is already the last (Pointer to the Next Filter
        is NULL) than it MUST deliver the packet back to the queue
		Note: never ever push the packet back to TSPacketQueue or submit it the packet
		to the next filter
							

	*/
	virtual void dispatch(TSPacket *Packet) = 0; //pure virtual

	//---------------------------------------------------------------
	/*
		getPriority
		Description: this function returns the processing priority of the filter. The lower
		the value, the higher the priority. If a filter needs to a higher / lower
		than the normal priority, it should then override this function
	
		Note: filter which access more than one TSPid must override
		this function to customize individual filter behavior

		TSPID_PRIORITY_LOWEST should only be used by AVPidFilters
	
	*/

	virtual const int getPriority(const unsigned char PidIndex) 
	{ 
		UNREFERENCED_PARAMETER(PidIndex);	
		return TSPID_PRIORITY_NORMAL;
	}

	//---------------------------------------------------------------
	/*
		notifyTransponderChange
		this function will be called when Transponder change is done. 
		TSPidFilter who can accept this change should return S_OK, filters
		who cannot accept that should return E_FAIL (for i.e File writing
		filters or audio video filters)

		Note: filters that accept this will remain in the TSPidFilterQueue
		untill this transponder is reactivated. However, they will receive
		untill that event no further TSPackets. In addition, they should release
		any TSPackets (in case they are occupying some)

		@param Enable : a positive value indicates that the Transponder
		is reactivated while a negative value indicates that the Transponder
		is deactivated

	*/
	virtual HRESULT notifyTransponderChange(bool Enable)
	{
		UNREFERENCED_PARAMETER(Enable);	
		return S_OK;
	}

	/*
		isProcessingDone
		A true return value inidcates the TSPidFilter has finished doing its work, and
		that it no longer requires to be in TSPidFilterQueue (this is usefull when having
		filters, which only need to operate a certain time to gather the requested information
		like PAT, PMT, or SDT filter
	*/
	virtual const bool isProcessingDone()
	{
		return false;
	}


private:
	TSPidFilter(const TSPidFilter &Source)
	{
		UNREFERENCED_PARAMETER(Source);
	}

};
#endif