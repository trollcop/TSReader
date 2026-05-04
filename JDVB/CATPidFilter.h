
#ifndef CATPIDFILTER_H___
#define CATPIDFILTER_H___

/*********************************************************************************
 *                                                                               *
 * CATPidFilter.h Conditional Access Table Scanner Filter                        *
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
 * Class CATPidFilter
 *
 * This class scans the Condidional Access Table (Pid 1) and searches
 * for new CAT Table information
 *
 * Note: this filter will stop parsing when it has found efficient
 * information to process CAT sections (it will then deliver the packets
 * to the next filter or just push it back to the TSPacketQueue)
 *
 *******************************************************************/





#include "TSPidFilter.h"

typedef long HRESULT;


typedef struct __CATInfo__
{
	unsigned short CA_ID;
	unsigned short EMM;
	struct __CATInfo__ *Next;

}CATInfo;


class CATPidFilter : public TSPidFilter
{

protected:

	CATInfo *Header_;
	int PacketCount;
	int CATInfoCount_;

public:

	CATPidFilter(unsigned short Pid, char *Name);

	virtual ~CATPidFilter();

	void dispatch(TSPacket *Packet);

	int getPriority();

	const int getCATInfoCount() const;

	const CATInfo *getCATInfo () const;

protected:

	HRESULT extractCATInfos(TSPacket *Packet);

	HRESULT insertCATInfo(unsigned short CA_ID, unsigned short EMM);

};
#endif