
#ifndef AVPIDFILTER_H___
#define AVPIDFILTER_H___

/*********************************************************************************
 *                                                                               *
 * AVPidFilter.h AV Dispatching Filter                                           *
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
 * The author can be reached at j_anderw@sbox.tugraz.at    
 *
 *   Version: 0.0.1
 *********************************************************************************/ 


/********************************************************************
 * Class AVPidFilter
 *
 * This class delivers TSPacket to our CSourceFilter class. The 
 * CSourceStreamPin class then stores it internally and releases
 * the packets after it has put them into the IMediaSample class
 *
 *******************************************************************/


#include "TSPidFilter.h"
#include "InterfaceSourceFilter.h"

class AVPidFilter : public TSPidFilter
{
protected:
ISourceFilter *Filter_;

public: 
	AVPidFilter(const unsigned short Pid, const char *PidName);

	virtual ~AVPidFilter();
	
	void setFilter(ISourceFilter *Filter);
	
	const int getPriority(const unsigned char PidIndex);
	void dispatch(TSPacket *Packet);
	
	void setTSPacketQueue(TSPacketQueue *Queue);

	void setTSPidFilter(TSPidFilter *Next);

};
#endif