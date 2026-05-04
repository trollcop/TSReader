

#ifndef TSPACKET_H___
#define TSPACKET_H___

/*********************************************************************************
 *                                                                               *
 * TSPacket.h  Tranport Stream Packet Class                                      *
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
 * Class TSPacket
 *
 * This class represents a RAW Tranport Stream Packet.
 * It includes query function to facilitate information about the
 * packet (getPid...) . In addition it stores a pointer to an other
 * TSPacket (aides to job of the TSPacketQueue or of any other
 * TSPidFilter, which wants to temporarly store TSPackets)
 *
 *
 *******************************************************************/



class TSPacket
{
protected:
	char * Data_;
	static unsigned char PACKET_SIZE;
	TSPacket *Next_;
public:
	TSPacket();
	virtual ~TSPacket();
	bool operator ==(const TSPacket &Source);
	void update(const char *Data);
	char * getData();

	unsigned short getPid();
	unsigned char  getAdaptionLength();

	bool hasPayload();

	unsigned char getScramblingType();


	TSPacket(const TSPacket & Source);

	void setNext(TSPacket *Next);
	
	TSPacket *getNext() 
	{return Next_;}


};


#endif