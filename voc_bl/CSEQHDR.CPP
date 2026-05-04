//
//  Copyright (c) 1999  ELECARD.  All Rights Reserved.
//

#include <streams.h>
#include <math.h>
#include "../cseqhdr.h"

/* zig-zag scan */
const int zig_zag_scan[64] =
{
  0,1,8,16,9,2,3,10,17,24,32,25,18,11,4,5,
  12,19,26,33,40,48,41,34,27,20,13,6,7,14,21,28,
  35,42,49,56,57,50,43,36,29,22,15,23,30,37,44,51,
  58,59,52,45,38,31,39,46,53,60,61,54,47,55,62,63
};

/* Constructor - initialize to resonable defaults
 */
CSeqHdrData::CSeqHdrData(int NTSC,int mpeg1)
{

	if (mpeg1)
	{
		if (NTSC)
		{
			// MPEG-1 NTSC params 
			horizontal_size = 352;
			vertical_size = 240;
			aspectratio = 8;
			frame_rate_code = 4;
			bit_rate = 1152000;
			vbv_buffer_size = 20;
			low_delay = 0;
			constrparms = 1;
			profile = 4;
			level = 8;
			prog_seq = 1;
			chroma_format = 1;
			video_format = 1;
			color_primaries = 5;
			transfer_characteristics = 5;
			matrix_coefficients = 5;
			display_horizontal_size = 352;
			display_vertical_size = 240;
		}
		else
		{
			// MPEG-1 PAL params 
			horizontal_size = 352;
			vertical_size = 288;
			aspectratio = 8;
			frame_rate_code = 3;
			bit_rate = 1152000;
			vbv_buffer_size = 20;
			low_delay = 0;
			constrparms = 1;
			profile = 4;
			level = 8;
			prog_seq = 1;
			chroma_format = 1;
			video_format = 1;
			color_primaries = 5;
			transfer_characteristics = 5;
			matrix_coefficients = 5;
			display_horizontal_size = 352;
			display_vertical_size = 288;
		}

        }
	else
	{
		if (NTSC)
		{
			// NTSC params
			horizontal_size = 704;
			vertical_size = 480;
			aspectratio = 2;		// aspect_ratio_information 1=square pel, 2=4:3, 3=16:9, 4=2.11:1 
			frame_rate_code = 4;	// frame_rate_code 1=23.976, 2=24, 3=25, 4=29.97, 5=30 frames/sec. 
			bit_rate = 5000000;		// bit_rate (bits/s)
			vbv_buffer_size = 112;	// vbv_buffer_size (in multiples of 16 kbit)
			low_delay = 0;
			constrparms = 0;
			profile = 4;			// Profile ID: Simple = 5, Main = 4, SNR = 3, Spatial = 2, High = 1 
			level = 8;				// Level ID:   Low = 10, Main = 8, High 1440 = 6, High = 4
			prog_seq = 0;
			chroma_format = 1;		// chroma_format: 1=4:2:0, 2=4:2:2, 3=4:4:4
			video_format = 2;		// video_format: 0=comp., 1=PAL, 2=NTSC, 3=SECAM, 4=MAC, 5=unspec.
			color_primaries = 5;
			transfer_characteristics = 5;
			matrix_coefficients = 5;
			display_horizontal_size = 704;
			display_vertical_size = 480;
		}
		else
		{
			// PAL params
			horizontal_size = 704;
			vertical_size = 576;
			aspectratio = 2;		// aspect_ratio_information 1=square pel, 2=4:3, 3=16:9, 4=2.11:1 
			frame_rate_code = 3;	// frame_rate_code 1=23.976, 2=24, 3=25, 4=29.97, 5=30 frames/sec. 
			bit_rate = 5000000;		// bit_rate (bits/s)
			vbv_buffer_size = 112;	// vbv_buffer_size (in multiples of 16 kbit)
			low_delay = 0;
			constrparms = 0;
			profile = 4;			// Profile ID: Simple = 5, Main = 4, SNR = 3, Spatial = 2, High = 1 
			level = 8;				// Level ID:   Low = 10, Main = 8, High 1440 = 6, High = 4
			prog_seq = 0;
			chroma_format = 1;		// chroma_format: 1=4:2:0, 2=4:2:2, 3=4:4:4
			video_format = 1;		// video_format: 0=comp., 1=PAL, 2=NTSC, 3=SECAM, 4=MAC, 5=unspec.
			color_primaries = 5;
			transfer_characteristics = 5;
			matrix_coefficients = 5;
			display_horizontal_size = 704;
			display_vertical_size = 576;
		}      
        }

	load_iquant = 0;
	load_niquant = 0;

	header = new CHeader;
	bs = NULL;
}

CSeqHdrData::~CSeqHdrData()
{
	if (header)
		delete header;
	if (bs)
		delete bs;
}

/* prepare for encoding
 */
CSeqHdrData::check()
{
	int block_count_tab[3] = {6,8,12};
	double ratetab[8]=
	  {24000.0/1001.0,24.0,25.0,30000.0/1001.0,30.0,50.0,60000.0/1001.0,60.0};

	/* make flags boolean (x!=0 -> x=1) */
	low_delay = !!low_delay;
	constrparms = !!constrparms;
	prog_seq = !!prog_seq;

	frame_rate = ratetab[frame_rate_code-1];
	constrparms = 0;

	return 0;
}

void CHeader::putseqhdr(int horizontal_size,int vertical_size,int aspectratio,
	int frame_rate_code,double bit_rate,int vbv_buffer_size,
	int constrparms,int load_iquant,int *intra_q,int load_niquant,
	int *inter_q)
{
  int i;

  bs->startcode(SEQ_START_CODE); /* sequence_header_code */
  bs->write(horizontal_size,12); /* horizontal_size_value */
  bs->write(vertical_size,12); /* vertical_size_value */
  bs->write(aspectratio,4); /* aspect_ratio_information */
  bs->write(frame_rate_code,4); /* frame_rate_code */
  bs->write((int)ceil(bit_rate/400.0),18); /* bit_rate_value */
  bs->write(1,1); /* marker_bit */
  bs->write(vbv_buffer_size,10); /* vbv_buffer_size_value */
  bs->write(constrparms,1); /* constrained_parameters_flag */

  bs->write(load_iquant,1); /* load_intra_quantizer_matrix */
  if (load_iquant)
    for (i=0; i<64; i++)  /* matrices are always downloaded in zig-zag order */
      bs->write(intra_q[zig_zag_scan[i]],8); /* intra_quantizer_matrix */

  bs->write(load_niquant,1); /* load_non_intra_quantizer_matrix */
  if (load_niquant)
    for (i=0; i<64; i++)
      bs->write(inter_q[zig_zag_scan[i]],8); /* non_intra_quantizer_matrix */
}

/* generate sequence extension (6.2.2.3, 6.3.5) header (MPEG-2 only) */
void CHeader::putseqext(int profile,int level,int prog_seq,int chroma_format,
	int horizontal_size,int vertical_size,double bit_rate,
	int vbv_buffer_size)
{
  bs->startcode(EXT_START_CODE); /* extension_start_code */
  bs->write(SEQ_ID,4); /* extension_start_code_identifier */
  bs->write((profile<<4)|level,8); /* profile_and_level_indication */
  bs->write(prog_seq,1); /* progressive sequence */
  bs->write(chroma_format,2); /* chroma_format */
  bs->write(horizontal_size>>12,2); /* horizontal_size_extension */
  bs->write(vertical_size>>12,2); /* vertical_size_extension */
  bs->write(((int)ceil(bit_rate/400.0))>>18,12); /* bit_rate_extension */
  bs->write(1,1); /* marker_bit */
  bs->write(vbv_buffer_size>>10,8); /* vbv_buffer_size_extension */
  bs->write(0,1); /* low_delay  -- currently not implemented */
  bs->write(0,2); /* frame_rate_extension_n */
  bs->write(0,5); /* frame_rate_extension_d */
}

/* generate sequence display extension (6.2.2.4, 6.3.6)
 *
 * content not yet user setable
 */
void CHeader::putseqdispext(int video_format,int color_primaries,
	int transfer_characteristics,int matrix_coefficients,
	int display_horizontal_size,int display_vertical_size)
{
  bs->startcode(EXT_START_CODE); /* extension_start_code */
  bs->write(DISP_ID,4); /* extension_start_code_identifier */
  bs->write(video_format,3); /* video_format */
  bs->write(1,1); /* colour_description */
  bs->write(color_primaries,8); /* colour_primaries */
  bs->write(transfer_characteristics,8); /* transfer_characteristics */
  bs->write(matrix_coefficients,8); /* matrix_coefficients */
  bs->write(display_horizontal_size,14); /* display_horizontal_size */
  bs->write(1,1); /* marker_bit */
  bs->write(display_vertical_size,14); /* display_vertical_size */
}

void COBitstream::write(uint32 bits,int n)
{
	register int bc;
	register uint32 bb;

	bc = bit_count+n;
	bb = (bit_buffer<<n)|(bits&(~((~0L)<<n)));
	while (bc>=8)
	{
		bc -= 8;
		*pout++ = (uchar)(bb>>bc)&0xFF;
		total_bytes++;
	}
	bit_count = bc;
	bit_buffer = bb;
}

void COBitstream::startcode(uint32 code)
{
	bytepad();
	pout[0] = (uchar)(code>>24)&0xFF;
	pout[1] = (uchar)(code>>16)&0xFF;
	pout[2] = (uchar)(code>>8)&0xFF;
	pout[3] = (uchar)code&0xFF;
	pout += 4;
	total_bytes+=4;
}
	
void COBitstream::write1(uint32 bit)
{
	bit_buffer=(bit_buffer<<1)|(bit&1);
	if (bit_count>=7)
	{
		bit_count = 0;
		*pout++ = (uchar)bit_buffer&0xFF;
		total_bytes++;
	}
	else
		bit_count++;
}

void COBitstream::bytepad()
{
	register uint32 bb;

	if (bit_count)
	{
		bb = bit_buffer << (8-bit_count);
		bit_count = 0;
		*pout++ = (uchar)bb&0xFF;
		total_bytes++;
	}
}

void COBitstream::flush()
{
	register uint32 bb;
	if (bit_count)
	{
		bb = bit_buffer << (8-bit_count);
		*pout++ = (uchar)bb&0xFF;
		total_bytes++;
		bit_count = 0;
	}
}

uint32 CSeqHdrData::create_seq_hdr(uchar *p)
{
	bs = new COBitstream;
	header->set_bitstream(bs);
	bs->set_buffer(p);

	header->putseqhdr(horizontal_size,vertical_size,aspectratio,
	  frame_rate_code,bit_rate,vbv_buffer_size,constrparms,
	  load_iquant,intra_q,load_niquant,inter_q);
	if (!mpeg1)
	{
		header->putseqext(profile,level,prog_seq,chroma_format,
		  horizontal_size,vertical_size,bit_rate,vbv_buffer_size);
		header->putseqdispext(video_format,color_primaries,
		  transfer_characteristics,matrix_coefficients,
		  display_horizontal_size,display_vertical_size);
	}

	bs->flush();

	return bs->get_bytes_written();
}