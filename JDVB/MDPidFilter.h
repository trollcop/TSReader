

#ifndef MDPIDFILTER_H___
#define MDPIDFILTER_H___

/*********************************************************************************
 *                                                                               *
 * MDPidFilter.h   TSPidFilter for MD Plugins                                    *
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
 *     Version: 0.0.1
 *
 *********************************************************************************/ 



/********************************************************************
 * Class MDPidFilter
 *
 * This class only dispatches received TSPackets to the registered callback
 * function of an MultiDec Plugin. After that, the the Filter simply dispatches
 * back to the TSPidFilterQueue
 *
 *******************************************************************/


#include "TSPidFilter.h"

typedef void (__cdecl *CallBackProcedure)(int Pid, int Length, char *Data);


class MDPidFilter : public TSPidFilter
{
protected:
	CallBackProcedure Procedure_;
	unsigned short Pid_;
	char *Name_;

public:

	MDPidFilter(unsigned short Pid, char *FilterName, CallBackProcedure Procedure);

	virtual ~MDPidFilter();
	void dispatch(TSPacket *Packet);

	const int getPriority(const unsigned char PidIndex);

};

#endif