//
//  Copyright (c) 1999  ELECARD.  All Rights Reserved.
//

typedef unsigned char uchar;
typedef unsigned short uint16;
typedef unsigned int uint32;

#define MAX_SEQ_HDR_LEN		192	/* 164 actually... */

#define PICTURE_START_CODE 0x100L
#define SLICE_MIN_START    0x101L
#define SLICE_MAX_START    0x1AFL
#define USER_START_CODE    0x1B2L
#define SEQ_START_CODE     0x1B3L
#define EXT_START_CODE     0x1B5L
#define SEQ_END_CODE       0x1B7L
#define GOP_START_CODE     0x1B8L
#define ISO_END_CODE       0x1B9L
#define PACK_START_CODE    0x1BAL
#define SYSTEM_START_CODE  0x1BBL

/* chroma_format */
#define CHROMA420 1
#define CHROMA422 2
#define CHROMA444 3

/* extension start code IDs */
#define SEQ_ID       1
#define DISP_ID      2
#define QUANT_ID     3
#define SEQSCAL_ID   5
#define PANSCAN_ID   7
#define CODING_ID    8
#define SPATSCAL_ID  9
#define TEMPSCAL_ID 10

class COBitstream;
class CHeader;

class CSeqHdrData {
	CHeader *header;
	COBitstream *bs;

	int mpeg1;
public:
	/* intra / non_intra quantization matrices */
	int intra_q[64], inter_q[64];
	int chrom_intra_q[64], chrom_inter_q[64];

	/* sequence specific data (sequence header) */
	int horizontal_size, vertical_size; /* frame size (pels) */
	int aspectratio; /* aspect ratio information (pel or display) */
	int frame_rate_code; /* coded value of frame rate */
	double frame_rate; /* frames per second */
	double bit_rate; /* bits per second */
	int vbv_buffer_size; /* size of VBV buffer (* 16 kbit) */
	int constrparms; /* constrained parameters flag (MPEG-1 only) */
	int load_iquant, load_niquant; /* use non-default quant. matrices */
	int load_ciquant,load_cniquant;

	/* sequence specific data (sequence extension) */
	int profile, level; /* syntax / parameter constraints */
	int prog_seq; /* progressive sequence */
	int chroma_format;
	int low_delay; /* no B pictures, skipped pictures */

	/* sequence specific data (sequence display extension) */
	int video_format; /* component, PAL, NTSC, SECAM or MAC */
	int color_primaries; /* source primary chromaticity coordinates */
	int transfer_characteristics; /* opto-electronic transfer char. (gamma) */
	int matrix_coefficients; /* Eg,Eb,Er / Y,Cb,Cr matrix coefficients */
	int display_horizontal_size, display_vertical_size; /* display size */

public:
	CSeqHdrData(int NTSC=0,int mpeg1=0);
	~CSeqHdrData();

	int check();
	uint32 create_seq_hdr(uchar *p);
};

class CHeader {
	COBitstream *bs;
	int frametotc (int frame,double frame_rate);
public:
		CHeader()
		{
			bs = NULL;
		}
		CHeader(COBitstream *p)
		{
			bs = p;
		}
		void set_bitstream(COBitstream *p)
		{
			bs = p;
		}
	void putseqhdr(int horizontal_size,int vertical_size,int aspectratio,
		int frame_rate_code,double bit_rate,int vbv_buffer_size,
		int constrparms,int load_iquant,int *intra_q,int load_niquant,
		int *inter_q);
	void putseqext(int profile,int level,int prog_seq,int chroma_format,
		int horizontal_size,int vertical_size,double bit_rate,
		int vbv_buffer_size);
	void putseqdispext(int video_format,int color_primaries,
		int transfer_characteristics,int matrix_coefficients,
		int display_horizontal_size,int display_vertical_size);
};

class COBitstream {
	uint32 bit_buffer;
	int bit_count;
	uchar *pout, *pbase;
	uint32 total_bytes;
public:
		COBitstream()
		{
			bit_buffer = 0;
			bit_count = 0;
			total_bytes = 0;
			pout = pbase = NULL;
		}
		void set_buffer(uchar *p)
		{
			bit_buffer = 0;
			bit_count = 0;
			pbase = pout = p;
		}
		double get_total_bits()
		{
			return (double)get_bytes_written()*8.0+(double)bit_count;
		}
		uint32 get_bytes_written()
		{
			return total_bytes;
		}
	void write(uint32 bits,int n);
	void startcode(uint32 code);
	void write1(uint32 bit);
	void bytepad();
	void flush();
};
