
#ifndef CSAPIDFILTER_H___
#define CSAPIDFILTER_H___

/*********************************************************************************
 *                                                                               *
 * CSAPidFilter.h Common Scrambling Algorithm Filter                             *
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
 *
 *  Version: 0.0.1
 *
 *********************************************************************************/ 




/********************************************************************
 * Class CSAPidFilter
 *
 * This class decrypts scrambled stream via the Common Scrambling Algorithm
 * To do this, it receives key via updateKey function and then decrypts it
 * When there is no key available yet, it stores the Packet internally. If
 * the internal queue gets too big, the first packet is released
 *
 *
 *******************************************************************/



#include "TSPidFilter.h"


class CSAPidFilter : public TSPidFilter
{
protected:
char EvenKey_[8];
char OddKey_[8];
bool EvenKeyValid_;
bool OddKeyValid_;
TSPacketQueue *OddKeyQueue_;
TSPacketQueue *EvenKeyQueue_;
TSPacketQueue *OriginalQueue_;
static const int MAX_BUFFERED_PACKETS;

public:
	CSAPidFilter(unsigned short Pid, char *PidName);

	virtual ~CSAPidFilter();

	void dispatch(TSPacket *Packet);

	const int getPriority(const unsigned char PidIndex);

	void updateKey(char *Key, bool Even);

protected:

	HRESULT decrypt(TSPacket *Packet, char *Key);


};
#endif