

#ifndef PATPIDFILTER_H___
#define PATPIDFILTER_H___

/*********************************************************************************
 *                                                                               *
 * PATPidFilter.h PidFilter for scanning PAT's                                   *
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
 *     Version: 0.0.2
 *
 *********************************************************************************/ 

#include "TSPidFilter.h"


typedef struct __PATInfo__
{
	unsigned short SIDPid;
	unsigned short PMTPid;
	unsigned short TPId;
	struct __PATInfo__ *Next;
}PATInfo;

typedef struct __PATTable__ 
{
	unsigned char TableId;
	unsigned char BitField1;
	unsigned char SectionLength_lo;
	unsigned char TransportStreamId_hi;
	unsigned char TransportStreamId_lo;
	unsigned char BitField2;
	unsigned char SectionNumber;
	unsigned char last_section_number;
}PATTable;

typedef struct __PATProgramm__
{
	unsigned char ProgrammNumber_hi;
	unsigned char ProgrammNumber_lo;
	unsigned char BitField1;
	unsigned char NetworkPid_lo;
}PATProgramm;

#define TID_PAT		0x0
#define PID_PAT		0x0
#define HILO(x) (x##_hi << 8 | x##_lo)


class PATPidFilter : public TSPidFilter
{
protected:
	PATInfo *Head_;
	bool initialized_;
	bool done_;
public:
	PATPidFilter(unsigned short Pid, char *PidName);

	virtual ~PATPidFilter();

	void dispatch(TSPacket *Packet);

	const int getPriority(const unsigned char PidIndex);

	const bool isProcessingDone() const;

	const PATInfo * getPATInfo() const;


protected:
	void insertPAT(unsigned short SIDPid, unsigned short PMTPid, unsigned short TPId);

};
#endif