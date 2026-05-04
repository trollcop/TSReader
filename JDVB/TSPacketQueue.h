
#ifndef TSPACKETQUEUE_H___
#define TSPACKETQUEUE_H___

/*********************************************************************************
 *                                                                               *
 * TSPacketQueue.h  Transport Packet Queue Provider                               *
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
 * Class TSPacketQueue
 *
 *	This class is just a simple queue of TSPacket. Consumer classes
 *  such as the TSSource fetch a packet to distiribute to their 
 *  responding TSPidFilters. When queue gets empty, there is an option
 *  to increase this again by calling increaseQueue
 *
 *
 *******************************************************************/


class TSPacket;
typedef long HRESULT;


class TSPacketQueue
{
protected:
  int TotalPacketQueueSize_;
  int CurrentPacketQueueSize_;
  TSPacket *HeadTSPacket_;
  TSPacket *TailTSPacket_;

public:
	//------------------------------------------------------------------
	TSPacketQueue(unsigned short PacketQueueSize);
	//------------------------------------------------------------------
	TSPacketQueue();
	//------------------------------------------------------------------
	virtual ~TSPacketQueue();
	//------------------------------------------------------------------
	HRESULT increaseQueue(unsigned short PacketQueue);
	//------------------------------------------------------------------
	HRESULT decreaseQueue(unsigned short PacketQueue);
	//------------------------------------------------------------------
	HRESULT pull(TSPacket **Packet);
	//------------------------------------------------------------------
	HRESULT push(TSPacket *Packet);
	//------------------------------------------------------------------
	const int getCurrentQueueSize() const;
};



#endif