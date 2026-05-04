
#ifndef PMTPIDFILTER_H___
#define PMTPIDFILTER_H___

/*********************************************************************************
 *                                                                               *
 * PMTPidFilter.h  Program Map Table Pid Filter                                  *
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
 * Class PMTPidFilter
 *
 * This class implements a PMT Pid Filter. It shall filter all incoming
 * packets and set up a PMT structure.
 *
 *
 *******************************************************************/


#include "TSPidFilter.h"

typedef long HRESULT;


typedef struct __PMT__
{
	unsigned short AudioPid;
	unsigned short VideoPid;
	unsigned short TeleTextPid;
	unsigned short ECMPid;
	unsigned short PCRPid;

}PMT;

typedef struct __PMTTable___
{
	unsigned char TableId;
	unsigned char BitField1;
	unsigned char SectionLength_lo;
	unsigned char ProgrammNumber_hi;
	unsigned char ProgrammNumber_lo;
	unsigned char BitField2;
	unsigned char SectionNumber;
	unsigned char LastSectionNumber;
	unsigned char BitField3;
	unsigned char PCR_PID_lo;
	unsigned char BitField4;
	unsigned char ProgrammInfoLength_lo;

}PMTTable;

#define PMT_TABLE_LEN	12
#define TID_PMT			0x2

typedef struct __PMTInfo__
{
	unsigned char StreamType;
	unsigned char BitField1;
	unsigned char Elementary_PID_lo;
	unsigned char BitField2;
	unsigned char ES_Info_Length;
}PMTInfo;

#define PMT_CA_info_LEN 6
struct pmt_ca_info_struct {
	unsigned char stream_type;
    unsigned char len;
	unsigned char CA_Ident_hi;
	unsigned char CA_Ident_lo;
	unsigned char ECM_hi;
	unsigned char ECM_lo;
};



#define HILO(x) (x##_hi << 8 | x##_lo)


class PMTPidFilter : public TSPidFilter
{
protected:
PMT *Info_;
unsigned short Pid_;
char *Name_;
bool done_;
bool initialized_;
public:

	PMTPidFilter(unsigned short Pid, char *Name);

	virtual ~PMTPidFilter();

	void dispatch(TSPacket *Packet);

	const int getPriority(const unsigned char PidIndex);

	const bool isProcessingDone() const;


	const PMT *getPMTInfo() const;

protected:

	HRESULT extractPMTInfos(TSPacket *Packet);

	void extractCADescriptor(char * ptr, int info_length);

};



#endif