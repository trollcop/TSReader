
#ifndef SDTPIDFILTER_H___
#define SDTPIDFILTER_H___

/*********************************************************************************
 *                                                                               *
 * SDTPidFilter.h TSPidFilter scanning the SDT                                   *
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



/********************************************************************
 * Class SDTPidFilter
 *
 * This class implements the parsing of the Service Descriptor Table
 * It will hook onto the SDT Pid and gather together the requrired 
 * information. In order to track process, it will return status
 * information available with isProcessingDone() function (returns
 * true when finished)
 *
 *******************************************************************/

#include "TSPidFilter.h"

typedef long HRESULT;

#define PID_SDT		0x11
#define TID_SDT_ACT 0x42
#define TID_SDT_OTH 0x46

#define SDT_LEN 11
#define SDT_DESCR_LEN 5
#define DESCR_GEN_LEN 2
#define DESCR_SERVICE 0x48

#define HILO(x) (x##_hi << 8 | x##_lo)

struct sdt_descr_t {
 		unsigned char service_id_hi;
 		unsigned char	service_id_lo;
 		unsigned char  BitField1;
 		// u_char	eit_present_following_flag	: 1;
 		//u_char	eit_schedule_flag		: 1;
 		//u_char					: 6;
 		unsigned char  BitField2;
 		// u_char	descriptors_loop_length_hi	: 4;
 		// u_char	free_ca_mode			: 1;
 		// u_char	running_status			: 3;
 		unsigned char	descriptors_loop_length_lo;
 	};


struct sdt_t {
	unsigned char	table_id;
	unsigned char  BitField1;
	//u_char	section_length_hi		: 4;
	//u_char					: 3;
	//u_char	section_syntax_indicator	: 1;
	unsigned char	section_length_lo;
	unsigned char	transport_stream_id_hi;
	unsigned char	transport_stream_id_lo;
	unsigned char  BitField2;
	// u_char	current_next_indicator		: 1;
	// u_char	version_number			: 5;
	// u_char					: 2;
	unsigned char	section_number;
	unsigned char	last_section_number;
	unsigned char	original_network_id_hi;
	unsigned char	original_network_id_lo;
	unsigned char	buffer;
};

struct descr_gen_struct {
	unsigned char descriptor_tag;
	unsigned char descriptor_length;
};
typedef struct descr_gen_struct descr_gen_t;


#define GET_DESCRIPTOR_LENGTH(x) (((descr_gen_t *) x)->descriptor_length)
#define GET_DESCRIPTOR_TAG(x) (((descr_gen_t *) x)->descriptor_tag)



typedef struct __SDT__
{
	unsigned short SIDPid;
	unsigned char CAId;
	unsigned short TPId;
	char *Name;
	char *Provider;
	struct __SDT__ *Next;

}SDT;

class SDTPidFilter : public TSPidFilter
{
protected:
bool done_;
bool initialized_;
SDT *Head_;
TSPacket *HeadPacket_;
TSPacket *TailPacket_;
unsigned char PacketCount_;
public: 
	SDTPidFilter(unsigned short Pid, char * PidName);

	virtual ~SDTPidFilter();
	
	void dispatch(TSPacket *Packet);

	const int getPriority(const unsigned char PidIndex);

	const bool isProcessingDone() const;

	const SDT * getSDTInfo() const;

protected:

	void extractSDT(TSPacket *Packet);

	HRESULT insertSDT(unsigned short SIDPid, unsigned char CAId, unsigned short TPId, char * Provider, char *Name); 


};
#endif