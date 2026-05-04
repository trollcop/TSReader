
#ifndef TSPIDFILTERQUEUE_H___
#define TSPIDFILTERQUEUE_H___

/*********************************************************************************
 *                                                                               *
 * TSPidFilterQueue.h     Queue holding TSPidFilters                             *
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
 * Class TSPidFilterQueue
 *
 *	This class is just a simple Queue holding pointers to TSPidFilters
 *  sharing the same Pid 
 *  In addition, it implements an priority scheme (for i.e
 *  to feed Decryption Filters first and then let the UDP Filters 
 *  stream the decrypted stuff over the net...)
 *
 * A TSPidFilter is inserted by calling registerTSPidFilter. This class
 * then stores the TSPidFilter in an local array then dispatches the 
 * data to appropiate filters
 *
 * When a TSSource class wants a TSPidFilterQueue to process
 * new data, it will call the TSPidFilterQueue class the following way
 *
 * 	void dispatch(TSPacket *Packet);
 *   TSPacket             ...  Packet data
 *
 * The Queue delivers the Packet the first filter in the queue, the 
 * filters are then responsible for delivering the data to the next
 * filter when they have finished processing
 *
 *********************************************************************/


class TSPidFilter;
class TSPacket;

typedef long HRESULT;

#define UNREFERENCED_PARAMETER(P)	(P)



class TSPidFilterQueue
{
protected:
	TSPidFilter *Head_;
	unsigned short Pid_;
	TSPidFilterQueue *Next_;
public:
	//------------------------------------------------------------------
	TSPidFilterQueue(unsigned short Pid);
	//------------------------------------------------------------------
	virtual ~TSPidFilterQueue();
	//------------------------------------------------------------------
	HRESULT insertTSPidFilter(TSPidFilter *Filter);
	//------------------------------------------------------------------
	HRESULT removeTSPidFilter(TSPidFilter *Filter);
	//------------------------------------------------------------------
	void dispatch(TSPacket *Packet);
	//------------------------------------------------------------------
	const unsigned short getPid() const;
	//------------------------------------------------------------------
	const bool isEmpty();

	void setNextTSPidFilterQueue(TSPidFilterQueue *Queue);

	TSPidFilterQueue * getNextTSPidFilterQueue() const;
	
private:
	TSPidFilterQueue(const TSPidFilterQueue &Source)
	{
		UNREFERENCED_PARAMETER(Source);
	}

};
#endif