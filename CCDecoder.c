#include <windows.h>
#include <commctrl.h>
#include "TSReader.h"

#ifdef PRO
//#define CC_TRACE

#include "resource.h"
#include "bcdmux.h"
#include "CCDecoder.h"
#include "util.h"

#define CC_DUMP_RAW_USER		0x00000001
#define CC_DUMP_EIA_608			0x00000002
#define CC_DUMP_EIA_708			0x00000004

extern PVARIABLES v;
extern char gszAppName[];
extern char gszEPGGridClass[];
extern char szLiteWarning[];

PCLOSEDCAPTIONVARIABLES ccv = NULL;

static char gszCCDisplayClass[] = {"TSReaderCCDisplayClass"};

// CC 608 storage
WCHAR stndChar[128];
WCHAR e1Char[16];
WCHAR e2Char[32];
WCHAR e3Char[32];

char codeMessage1[512];
char codeMessage2[512];	
char codeMessage3[1024];

#include "CCDecoder_Strings.h"
#endif PRO

void bs_init(bs_t* b, uint8_t* buf, int size)
{
	b->start = buf;
	b->p = buf;
	b->end = buf + size;
	b->bits_left = 8;
}

uint32_t bs_byte_aligned(bs_t* b) { if (b->bits_left == 8) { return 1; } else { return 0; } }

//uint32_t bs_eof(bs_t* b) { if (b->p >= b->end) { return 1; } else { return 0; } }
uint32_t bs_eof(bs_t* b) { return 0; }

uint32_t bs_read_u1(bs_t* b)
{
	uint32_t r = 0;
	if (bs_eof(b)) { return 0; }
	
	b->bits_left--;
	r = ((*(b->p)) >> b->bits_left) & 0x01;

	if (b->bits_left == 0) { b->p ++; b->bits_left = 8; }

	return r;
}

uint32_t bs_read_u(bs_t* b, int n)
{
	uint32_t r = 0;
	int i;
	for (i = 0; i < n; i++)
	{
		r |= ( bs_read_u1(b) << ( n - i - 1 ) );
	}
	return r;
}

uint32_t bs_read_f(bs_t* b, int n) { return bs_read_u(b, n); }

uint32_t bs_read_u8(bs_t* b) { return bs_read_u(b, 8); }

uint32_t bs_read_ue(bs_t* b)
{
	int32_t r = 0;
	int i = 0;

    while( bs_read_u1(b) == 0 && i < 32 && !bs_eof(b) )
    {
        i++;
    }
	r = bs_read_u(b, i);
	r += (1 << i) - 1;
	return r;
}

int32_t bs_read_se(bs_t* b) 
{
	int32_t r = bs_read_ue(b);
	if (r & 0x01)
	{
		r = (r+1)/2;
	}
	else
	{
		r = -(r/2);
	}
	return r;
}

void read_scaling_list(bs_t* b, int* scalingList, int sizeOfScalingList, int useDefaultScalingMatrixFlag )
{
	int j;

	int lastScale = 8;
	int nextScale = 8;
	for( j = 0; j < sizeOfScalingList; j++ ) {
		if( nextScale != 0 ) {
			int delta_scale = bs_read_se(b);
			nextScale = ( lastScale + delta_scale + 256 ) % 256;
			useDefaultScalingMatrixFlag = ( j == 0 && nextScale == 0 );
		}
		scalingList[ j ] = ( nextScale == 0 ) ? lastScale : nextScale;
		lastScale = scalingList[ j ];
	}
}

void read_hrd_parameters(sps_t* sps, bs_t* b)
{
	int SchedSelIdx;

	sps->hrd.cpb_cnt_minus1 = bs_read_ue(b);
	sps->hrd.bit_rate_scale = bs_read_u(b,4);
	sps->hrd.cpb_size_scale = bs_read_u(b,4);
	for( SchedSelIdx = 0; SchedSelIdx <= sps->hrd.cpb_cnt_minus1; SchedSelIdx++ ) {
		sps->hrd.bit_rate_value_minus1[ SchedSelIdx ] = bs_read_ue(b);
		sps->hrd.cpb_size_value_minus1[ SchedSelIdx ] = bs_read_ue(b);
		sps->hrd.cbr_flag[ SchedSelIdx ] = bs_read_u1(b);
	}
	sps->hrd.initial_cpb_removal_delay_length_minus1 = bs_read_u(b,5);
	sps->hrd.cpb_removal_delay_length_minus1 = bs_read_u(b,5);
	sps->hrd.dpb_output_delay_length_minus1 = bs_read_u(b,5);
	sps->hrd.time_offset_length = bs_read_u(b,5);
}

void read_vui_parameters(sps_t* sps, bs_t* b)
{
	sps->vui.aspect_ratio_info_present_flag = bs_read_u1(b);
	if( sps->vui.aspect_ratio_info_present_flag ) {
		sps->vui.aspect_ratio_idc = bs_read_u8(b);
		if( sps->vui.aspect_ratio_idc == SAR_Extended ) {
			sps->vui.sar_width = bs_read_u(b,16);
			sps->vui.sar_height = bs_read_u(b,16);
		}
	}
	sps->vui.overscan_info_present_flag = bs_read_u1(b);
	if( sps->vui.overscan_info_present_flag )
		sps->vui.overscan_appropriate_flag = bs_read_u1(b);
	sps->vui.video_signal_type_present_flag = bs_read_u1(b);
	if( sps->vui.video_signal_type_present_flag ) {
		sps->vui.video_format = bs_read_u(b,3);
		sps->vui.video_full_range_flag = bs_read_u1(b);
		sps->vui.colour_description_present_flag = bs_read_u1(b);
		if( sps->vui.colour_description_present_flag ) {
			sps->vui.colour_primaries = bs_read_u8(b);
			sps->vui.transfer_characteristics = bs_read_u8(b);
			sps->vui.matrix_coefficients = bs_read_u8(b);
		}
	}
	sps->vui.chroma_loc_info_present_flag = bs_read_u1(b);
	if( sps->vui.chroma_loc_info_present_flag ) {
		sps->vui.chroma_sample_loc_type_top_field = bs_read_ue(b);
		sps->vui.chroma_sample_loc_type_bottom_field = bs_read_ue(b);
	}
	sps->vui.timing_info_present_flag = bs_read_u1(b);
	if( sps->vui.timing_info_present_flag ) {
		sps->vui.num_units_in_tick = bs_read_u(b,32);
		sps->vui.time_scale = bs_read_u(b,32);
		sps->vui.fixed_frame_rate_flag = bs_read_u1(b);
	}
	sps->vui.nal_hrd_parameters_present_flag = bs_read_u1(b);
	if( sps->vui.nal_hrd_parameters_present_flag )
		read_hrd_parameters(sps, b);
	sps->vui.vcl_hrd_parameters_present_flag = bs_read_u1(b);
	if( sps->vui.vcl_hrd_parameters_present_flag )
		read_hrd_parameters(sps, b);
	if( sps->vui.nal_hrd_parameters_present_flag || sps->vui.vcl_hrd_parameters_present_flag )
		sps->vui.low_delay_hrd_flag = bs_read_u1(b);
	sps->vui.pic_struct_present_flag = bs_read_u1(b);
	sps->vui.bitstream_restriction_flag = bs_read_u1(b);
	if( sps->vui.bitstream_restriction_flag ) {
		sps->vui.motion_vectors_over_pic_boundaries_flag = bs_read_u1(b);
		sps->vui.max_bytes_per_pic_denom = bs_read_ue(b);
		sps->vui.max_bits_per_mb_denom = bs_read_ue(b);
		sps->vui.log2_max_mv_length_horizontal = bs_read_ue(b);
		sps->vui.log2_max_mv_length_vertical = bs_read_ue(b);
		sps->vui.num_reorder_frames = bs_read_ue(b);
		sps->vui.max_dec_frame_buffering = bs_read_ue(b);
	}
}

#ifdef PRO
void Scroll608Page(int nPage, BYTE bEvenField)
{
	int nRow, nCol;
	int nWritePage = nPage + (bEvenField * 2);

	for (nRow = 0; nRow < CAPTION_ROWS - 1; nRow++)
		memcpy(&ccv->b608DisplayBuffer[nWritePage][nRow], &ccv->b608DisplayBuffer[nWritePage][nRow + 1], CAPTION_COLUMNS * sizeof(WCHAR));
	for (nCol = 0; nCol < CAPTION_COLUMNS; nCol++)
		ccv->b608DisplayBuffer[nWritePage][nRow][nCol] = 0x20;
	ccv->n608CursorRow[nWritePage]--;
}

void Write608Character(WCHAR c, BYTE bEvenField)
{
	if (ccv->n608WriteChannel != -1)
	{
		int nWritePage = ccv->n608WriteChannel + (bEvenField * 2);

		ccv->b608DisplayBuffer[nWritePage][ccv->n608CursorRow[nWritePage]][ccv->n608CursorColumn[nWritePage]] = c;
		if (ccv->n608CursorColumn[nWritePage]++ == CAPTION_COLUMNS - 1)
		{
			ccv->n608CursorColumn[nWritePage] = 0;
			if (ccv->n608CursorRow[nWritePage]++ == CAPTION_ROWS - 1)
				Scroll608Page(ccv->n608WriteChannel, bEvenField);
		}
	}
}

void Scroll708Page(int nService)
{
	int nRow, nCol;

	for (nRow = 0; nRow < CAPTION_ROWS - 1; nRow++)
		memcpy(&ccv->b708DisplayBuffer[nService][nRow], &ccv->b708DisplayBuffer[nService][nRow + 1], CAPTION_COLUMNS * sizeof(WCHAR));
	for (nCol = 0; nCol < CAPTION_COLUMNS; nCol++)
		ccv->b708DisplayBuffer[nService][nRow][nCol] = 0x20;
	ccv->n708CursorRow[nService]--;
}

WCHAR Translate708CharacterSet(BYTE c)
{
	switch(c)
	{
	default:
		return c;
	case 0xff:		// musical note
		return 0x266a;
	}
	return 0x20;
}

void Write708Character(int nService, BYTE c)
{
	ccv->b708DisplayBuffer[nService][ccv->n708CursorRow[nService]][ccv->n708CursorColumn[nService]] = Translate708CharacterSet(c);
	if (ccv->n708CursorColumn[nService]++ == CAPTION_COLUMNS - 1)
	{
		ccv->n708CursorColumn[nService] = 0;
		if (ccv->n708CursorRow[nService]++ == CAPTION_ROWS - 1)
			Scroll708Page(nService);
	}
}

void InitCCDecoder()
{
	int csi;

	// Intialize standard character set
	for (csi = 32; csi < 128; csi++)
		stndChar[csi] = csi;
	
	stndChar[0x2a] = 0xe1;	// lower-case a, acute accent
	stndChar[0x5c] = 0xe9;	// lower-case e, acute accent
	stndChar[0x5e] = 0xed;	// lower-case i, acute accent
	stndChar[0x5f] = 0xf3;	// lower-case o, acute accent
	stndChar[0x60] = 0xfa;	// lower-case u, acute accent
	stndChar[0x7b] = 0xe7;	// lower-case c with cedilla
	stndChar[0x7c] = 0xf7;	// division symbol
	stndChar[0x7d] = 0xd1;	// upper-case enya (N-tilde)
	stndChar[0x7e] = 0xf1;	// lower-case enya (n-tilde)
	stndChar[0x7f] = 0x25a0;	// solid block

	// Standard Extended Character Set
	e1Char[0] = 0xae; 	// Registered mark
	e1Char[1] = 0xb0; 	// Degree
	e1Char[2] = 0xbd; 	// 1/2
	e1Char[3] = 0xbf; 	// upside down ?
	e1Char[4] = 0x99; 	// TM mark
	e1Char[5] = 0xa2; 	// cent symbol
	e1Char[6] = 0xa3; 	// English pound symbol
	e1Char[7] = 0x266a;	// Musical notes
	e1Char[8] = 0xe0; 	// a with accent mark
	e1Char[9] = 0x20; 	// Transparent space (using normal space)
	e1Char[10] = 0xe8; 	// e with accent mark
	e1Char[11] = 0xe2; 	// a with ^
	e1Char[12] = 0xea; 	// e with ^
	e1Char[13] = 0xee; 	// i with ^
	e1Char[14] = 0xf4; 	// o with ^
	e1Char[15] = 0xfb; 	// u with ^

	// 2nd Extended Character Set
	// Spanish
	e2Char[0] = 0xc1;	// A with acute accent
	e2Char[1] = 0xc9;	// E with acute accent
	e2Char[2] = 0xd3;	// O with acute accent
	e2Char[3] = 0xda;	// U with acute accent
	e2Char[4] = 0xdc;	// U with diaeresis or umlat
	e2Char[5] = 0xfc;	// u with diaeresis or umlat
	e2Char[6] = 0x60;	// Opening single quote (`)
	e2Char[7] = 0xa1;	// inverted exclamation mark

	// Misc
	e2Char[8] = 0x2a;	// Asterisk (*)
	e2Char[9] = 0x27;	// Plain single quote (')
	e2Char[10] = 0x5F;	// EM Dash (_)
	e2Char[11] = 0xa9;	// Copyright mark
	e2Char[12] = 0x99;	// Service Mark (SM)  - NOTE: using TM instead!!!
	e2Char[13] = 0x95;	// Round Bullet;
	e2Char[14] = 0x201c;	// Opening quote (using ")
	e2Char[15] = 0x201d;	// Closing quote (using ")

	// French
	e2Char[16] = 0xc0;	// A with grave accent
	e2Char[17] = 0xc2;	// A with circumflex accent
	e2Char[18] = 0xc7;	// C with cedilla
	e2Char[19] = 0xc8;	// E with grave accent
	e2Char[20] = 0xca;	// E with circumflex accent
	e2Char[21] = 0xcb;	// E with diaeresis or umlaut mark
	e2Char[22] = 0xeb;	// e with diaeresis or umlaut mark
	e2Char[23] = 0xce;	// I with circumflex accent
	e2Char[24] = 0xcf;	// I with diaeresis or umlaut mark
	e2Char[25] = 0xef;	// i with diaeresis or umlaut mark
	e2Char[26] = 0xd4;	// O with cicumflex
	e2Char[27] = 0xd9;	// U with grave accent
	e2Char[28] = 0xf9;	// u with grave accent
	e2Char[29] = 0xdb;	// U with circumflex
	e2Char[30] = 0xab;	// opening guillemets
	e2Char[31] = 0xbb;	// closing guillemets

	// 3rd Extended character set
	// Portugese
	e3Char[0] = 0xc3;	// A with tilde
	e3Char[1] = 0xe3;	// a with tilde
	e3Char[2] = 0xcd;	// I with accute accent
	e3Char[3] = 0xcc;	// I with grave accent
	e3Char[4] = 0xec;	// i with grave accent
	e3Char[5] = 0xd2;	// O with grave accent
	e3Char[6] = 0xf2;	// o with grave accent
	e3Char[7] = 0xd5;	// O with tilde
	e3Char[8] = 0xf5;	// o with tilde
	e3Char[9] = 0x7b;	// opening brace ({)
	e3Char[10] = 0x7d;	// closing brace (})
	e3Char[11] = 0x5c;	// backslash (\)
	e3Char[12] = 0x5e;	// caret (^)
	e3Char[13] = 0x5f;	// underbar (_)
	e3Char[14] = 0x7c;	// pipe (|)
	e3Char[15] = 0x7e;	// tilde (~)

	// German
	e3Char[16] = 0xc4;	// A with diaeresis or umlaut mark
	e3Char[17] = 0xe4;	// a with diaeresis or umlaut mark
	e3Char[18] = 0xd6;	// O with diaeresis or umlaut mark
	e3Char[19] = 0xf6;	// o with diaeresis or umlaut mark
	e3Char[20] = 0xdf;	// small sharp s --- ????
	e3Char[21] = 0xa5;	// Yen
	e3Char[22] = 0xa4;	// non-specific currency sign
	e3Char[23] = 0x7c;	// Vertical bar (using | - needs to extend to upper/lower characters)

	// Danish
	e3Char[24] = 0xc5;	// A with a ring
	e3Char[25] = 0xe5;	// a with a ring
	e3Char[26] = 0xd8;	// O with a slash
	e3Char[27] = 0xf8;	// o with a slash
	e3Char[28] = 0x250c;// upper left corner --- no character
	e3Char[29] = 0x2510;// upper right corner --- no character
	e3Char[30] = 0x2514;// lower left corner --- no character
	e3Char[31] = 0x2518;// lower right corner --- no character
}

int GetCCDataType(int code)
{
	int codeU = (code >> 8) & 0xFF;
	int codeL = code & 0xFF;
	int controlCode1;

	// ========== Standard Character ==========
	if ( ((codeU >= 0x20) && (codeU <= 0x7F)) || codeU == 0)
		return TYPE_CHAR_STANDARD;

	// ========== Miscellaneous Control Codes ==========
	if ( ((codeU == 0x14) || (codeU == 0x1C) || (codeU == 0x15) || (codeU == 0x1D)) && 
	((codeL >= 0x20) && (codeL <= 0x2F)))
		return TYPE_MISC1;

	if ( ((codeU == 0x17) || (codeU == 0x1F)) && ((codeL >= 0x21) && (codeL <= 0x23)))
		return TYPE_MISC2;

	// ========== Mid-Row Codes ==========
	if ( ((codeU == 0x11) || (codeU == 0x19)) && ((codeL >= 0x20) && (codeL <= 0x2F)))
		return TYPE_MIDROW;

	// ========== PreAmble Address Codes ==========
	controlCode1 = code & 0xF760; // Strip out Channel, Colors, Underline 
	switch (controlCode1)
	{
	case 0x1140:
	case 0x1160:
	case 0x1240:
	case 0x1260:
	case 0x1540:
	case 0x1560:
	case 0x1640:
	case 0x1660: 
	case 0x1740:
	case 0x1760:
	case 0x1040:
	case 0x1340: 
	case 0x1360:
	case 0x1440: 
	case 0x1460: 
		return TYPE_PREAMBLE_ADDRESS;
	}

	// ========== Attribute Codes ==========
	if ( ((codeU == 0x10) || (codeU == 0x18)) && ((codeL >= 0x20) && (codeL <= 0x27)))
		return TYPE_ATTRIBUTE_1;
	if ( (code == 0x172D) || (code == 0x1F2D))
		return TYPE_ATTRIBUTE_2;	
	if ( (code==0x172E) || (code==0x172F) || (code==0x1F2E) || (code==0x1F2F) )
		return TYPE_ATTRIBUTE_3;

	// ========== Optional Closed Group Extensions ==========
	if ( ((codeU == 0x17) || (codeU == 0x1F)) && ((codeL >= 0x24) && (codeL <= 0x2A)))
		return TYPE_CLOSED_GROUP_EXT;

	// ========== Standard extended characters ==========
	if ( ((codeU == 0x11) || (codeU == 0x19)) &&  ((codeL >= 0x30) && (codeL <= 0x3F)))
		return TYPE_CHAR_EXT1;

	// ========== 2nd Extended Character set ==========
	if ( ((codeU == 0x12) || (codeU == 0x1A)) && ((codeL >= 0x20) && (codeL <= 0x3F)))
		return TYPE_CHAR_EXT2;

	// ========== 3rd Extended Character set ==========
	if ( ((codeU == 0x13) || (codeU == 0x1B)) && ((codeL >= 0x20) && (codeL <= 0x3F)))
		return TYPE_CHAR_EXT3;

	// ========== Extended Data service ==========
	if ((codeU >= 0x01) && (codeU <= 0x0F))
		return TYPE_XDS;

	// ========== UNKNOWN TYPE  ==========
	return TYPE_UNKNOWN;
}

void unknownType(int cc)
{

}

void standardChar(int cc, BYTE bEvenField)
{
	ccv->currentChar[0] = (cc >> 8) & 0x7f;
	ccv->currentChar[1] = (cc & 0xff) & 0x7f;

	if (ccv->currentChar[0] != 0)
	{			
		Write608Character(stndChar[ccv->currentChar[0]], bEvenField);
		if (ccv->currentChar[1] != 0)	
			Write608Character(stndChar[ccv->currentChar[1]], bEvenField);
		InvalidateRect(v->hWndCCDisplay, NULL, FALSE);
	}
}

void cntrlCodeMisc(int type, int cc, BYTE cc_even_field)
{
	int codeU = cc >> 8;
	int codeL = cc & 0xFF;
	int channel = ((codeU & 0x08) >> 3) + 1;
	int tvField = (codeU & 0x01) + 1;	// Not valid for TYPE_MISC2

	// Save any Messages
#ifdef CC_TRACE
	sprintf(codeMessage1, "Misc Control Code");
#endif 
	if (type == TYPE_MISC1)
	{
		switch(codeL & 0xf)
		{
		case 0:		// (RCL) Resume Caption Loading
		case 5:		// (RU2) Roll-up captions, 2 rows
		case 6:		// (RU3) Roll-up captions, 3 rows
		case 7:		// (RU4) Roll-up captions, 4 rows
		case 9:		// (RDC) Resume direct captioning
			ccv->n608WriteChannel = channel - 1;
			break;
		case 1:		// (BS)  Backspace
		case 2:		// (AOF) Reserved (formerly alarm off)
		case 3:		// (AON) Reserved (formerly alarm on)
		case 4:		// (DER) Delete to end of row
		case 8:		// (FON) Flash on
			break;
		case 10:	// (TR)  Text restart
		case 11:	// (RTD) Resume text display
			ccv->n608WriteChannel = channel - 1 + 4;
			break;
		case 12:	// (EDM) Erase displayed memory
			break;
		case 13:	// (CR)  Carriage return
			if (ccv->n608WriteChannel != -1)
			{
				if (ccv->n608CursorColumn[ccv->n608WriteChannel] > 0)
				{
					ccv->n608CursorColumn[ccv->n608WriteChannel] = 0;
					if (ccv->n608CursorRow[ccv->n608WriteChannel]++ == CAPTION_ROWS - 1)
						Scroll608Page(ccv->n608WriteChannel, cc_even_field);
				}				
			}
			break;
		}
#ifdef CC_TRACE
		sprintf(codeMessage2, "Channel %d, Field %d, %s", channel, tvField, misc1Code[codeL & 0xF]);
#endif CC_TRACE
	}
	else
	{
#ifdef CC_TRACE
		sprintf(codeMessage2, "Channel %d, %s", channel, misc2Code[(codeL-0x21) & 0x3]);
#endif CC_TRACE
	}
}

void cntrlCodeMidRow(int cc) 
{
	int codeU = cc >> 8;
	int codeL = cc & 0xFF;
	int channel = ((codeU & 0x08) >> 3) + 1;

	ccv->n608WriteChannel = channel - 1;

#ifdef CC_TRACE
	sprintf(codeMessage1, "MidRow Control Code"); 
	sprintf(codeMessage2, "Channel %d, %s, %s", 
		channel, midRowCode[(codeL >> 1) & 0x07], underlineCode[codeL & 0x01]);
#endif CC_TRACE
}

void cntrlCodePreAmble(int cc, BOOL bEvenField)
{
	int row;
	int codeU = cc >> 8;
	int codeL = cc & 0xFF;
	int channel = ((codeU & 0x08) >> 3) + 1;
	int controlCode1 = cc & 0xF760;	// Strip out Channel, Colors, Underline 

	ccv->n608WriteChannel = channel - 1;

	switch(controlCode1)
	{
	case 0x1140: row =  1; break;
	case 0x1160: row =  2; break;
	case 0x1240: row =  3; break;
	case 0x1260: row =  4; break;
	case 0x1540: row =  5; break;
	case 0x1560: row =  6; break;
	case 0x1640: row =  7; break;
	case 0x1660: row =  8; break;
	case 0x1740: row =  9; break;
	case 0x1760: row = 10; break;
	case 0x1040: row = 11; break;
	case 0x1340: row = 12; break;
	case 0x1360: row = 13; break;
	case 0x1440: row = 14; break;
	case 0x1460: row = 15; break;
	}

	{
		int nWritePage = ccv->n608WriteChannel + (bEvenField * 2);
		if (ccv->n608CursorColumn[nWritePage])
			Write608Character(' ', bEvenField);
		InvalidateRect(v->hWndCCDisplay, NULL, FALSE);
	}


#ifdef CC_TRACE
	sprintf(codeMessage1, "Preamble Control Code"); 
	sprintf(codeMessage2, "Channel %d, %s, %s, Row %d", 
		channel, preambleCode[(codeL >> 1) & 0x0F], underlineCode[codeL & 0x01], row);
#endif CC_TRACE
}

void cntrlCodeAttribute (int type, int cc)
{
	int codeU = cc >> 8;
	int codeL = cc & 0xFF;
	int channel = ((codeU & 0x08) >> 3) + 1;

	ccv->n608WriteChannel = channel - 1;

#ifdef CC_TRACE
	sprintf(codeMessage1, "Attribute Control Code");
	if (type == TYPE_ATTRIBUTE_1)
		sprintf(codeMessage2, "Channel %d, Background: %s %s", 	channel, transparentCode[codeL & 0x01], attributeCode[(codeL >> 1) & 0x07]);
	else if (type == TYPE_ATTRIBUTE_2)
		sprintf(codeMessage2, "Channel %d,  Background: Transparent", channel); 
	else if (type == TYPE_ATTRIBUTE_3)
		sprintf(codeMessage2, "Channel %d, Foreground: Black %s", channel, underlineCode[codeL & 0x01]);
#endif CC_TRACE
}

void cntrlCodeClosedGroup(int cc)
{
	int codeU = cc >> 8;
	int codeL = cc & 0xFF;
	int channel = ((codeU & 0x08) >> 3) + 1;
	int cgc = (codeL<0x24 || codeL>0x2A) ? 7 : (codeL-0x24);

	ccv->n608WriteChannel = channel - 1;

#ifdef CC_TRACE
	sprintf(codeMessage1, "Closed Group Extension Control Code");
	sprintf(codeMessage2, "Channel %d, %s",channel, closedGroupCode[cgc]);
#endif CC_TRACE
}

void cntrlCodeExtChar(int type, int cc, BYTE bEvenField) 
{
	int codeU = cc >> 8;
	int codeL = cc & 0xFF;
	int channel = ((codeU & 0x08) >> 3) + 1;

	ccv->n608WriteChannel = channel - 1;

	// Determine the character 
	if (type == TYPE_CHAR_EXT1)
	{
		ccv->currentChar[0] = e1Char[(codeL-0x30) & 0x0f];
	}
	else if (type == TYPE_CHAR_EXT2)
	{
		ccv->currentChar[0] = e2Char[(codeL-0x20) & 0x1f];
	}
	else if (type == TYPE_CHAR_EXT3)
	{
		ccv->currentChar[0] = e3Char[(codeL-0x20) & 0x1f];
	}

#ifdef CC_TRACE
	sprintf(codeMessage1,"Extended Character");
	sprintf(codeMessage2,"(%c)", ccv->currentChar[0]);
#endif CC_TRACE
	if (ccv->n608CursorColumn[ccv->n608WriteChannel])
		ccv->n608CursorColumn[ccv->n608WriteChannel]--;
	Write608Character(ccv->currentChar[0], bEvenField);
	InvalidateRect(v->hWndCCDisplay, NULL, FALSE);
}

void xdsCapture(int cc)
{
	int controlCode = cc >> 8;
	int type = cc & 0xff;

	// Control codes 1, 3, 5, 7, 9, 11, 13 are start codes.  Restart the process.
	if ((controlCode % 2) && (controlCode <= 13))
	{
		ccv->xdsPosition = 0;
		ccv->xdsLastControlCode = controlCode;
		ccv->xdsLastType = type;
		ccv->xdsState = XDS_COLLECT_DATA;
		return;
	}

	// Check if we are continuing an unknown packet or packet that differs from current one.
	// Reset if we are doing so.  (Legally, this is an acceptable situation but it is currently 
	// not implemented.)
	// Continuation types are: 2,4,6,8,10,12,14
	if ( ((controlCode % 2) == 0) && (ccv->xdsState != XDS_COLLECT_DATA) )
	{
		ccv->xdsState = XDS_COLLECT_DATA;
#ifdef CC_TRACE
		sprintf(codeMessage2,"%s %s", xdsClassType[controlCode], "ERROR: Continuation of unknown packet");
		sprintf(codeMessage3,"");
#endif CC_TRACE
		return;
	}

	// If all the data has been collected (packet END encountered) then process the packet
	// otherwise, save data in the buffer and construct the packet.
	if (controlCode == 0x0F && (ccv->xdsState == XDS_COLLECT_DATA))
	{
		if (ccv->xdsPosition)
		{
			int nXDSIndex = ccv->xdsLastControlCode << 8 | ccv->xdsLastType;
			memset(ccv->bXDSBuffer[nXDSIndex], 0, sizeof(ccv->bXDSBuffer[nXDSIndex]));
			memcpy(ccv->bXDSBuffer[nXDSIndex], ccv->xdsBuffer, ccv->xdsPosition);
			ccv->bXDSLength[nXDSIndex] = ccv->xdsPosition;
		}
		ccv->xdsState = XDS_IDLE;
		InvalidateRect(v->hWndCCDisplay, NULL, FALSE);
		return;
	}
}

#ifdef CC_TRACE
void printData(int pType, int code )
{
	char szTemp[256];

	if ((ccv->previousType == TYPE_CHAR_STANDARD) && (pType != TYPE_CHAR_STANDARD))
	{
		OutputDebugString("\n");
	}

	switch (pType) 
	{
	case TYPE_CHAR_STANDARD:
		if (ccv->currentChar[0] != 0)
		{
			sprintf(szTemp, "%c%c",stndChar[ccv->currentChar[0]], stndChar[ccv->currentChar[1]]);
			OutputDebugString(szTemp);
		}
		else 
			pType = ccv->previousType;
		break;
	case TYPE_MISC1:
	case TYPE_MISC2:
	case TYPE_MIDROW:
	case TYPE_PREAMBLE_ADDRESS:
	case TYPE_ATTRIBUTE_1:
	case TYPE_ATTRIBUTE_2:
	case TYPE_ATTRIBUTE_3:
	case TYPE_CLOSED_GROUP_EXT:
		sprintf(szTemp, "<0x%04x: %s, %s>\n", code, codeMessage1, codeMessage2);
		OutputDebugString(szTemp);
		break;

	case TYPE_CHAR_EXT1:
	case TYPE_CHAR_EXT2:
	case TYPE_CHAR_EXT3:
		sprintf(szTemp, "<0x%04x: %s, %s>\n", code, codeMessage1, codeMessage2);
		OutputDebugString(szTemp);
		break;

	case TYPE_XDS:
		sprintf(szTemp, "<0x%04x (XDS): %s>\n", code, codeMessage2);
		OutputDebugString(szTemp);
		if (codeMessage3[0] != 0) 
		{
			sprintf(szTemp, "<XDS Data:%s>\n",codeMessage3);
			OutputDebugString(szTemp);
		}
		break;

	case TYPE_UNKNOWN:
		sprintf(szTemp, "<0x%04x: %s, %s>\n", code, codeMessage1, codeMessage2);
		OutputDebugString(szTemp);
		break;
	}
	ccv->previousType = pType;
}
#endif CC_TRACE

void AddBufferHexASCII(char * szXDSDecode, BYTE * bXDSBuffer, BYTE nXDSIndex)
{
	int nOffset;
	char szTemp[256] = {""};
	char szTemp2[64];

	for (nOffset = 0; nOffset < nXDSIndex; nOffset++)
	{
		wsprintf(szTemp2, "%02x ", bXDSBuffer[nOffset]);
		lstrcat(szTemp, szTemp2);
	}

	for (nOffset = 0; nOffset < nXDSIndex; nOffset++)
	{
		BYTE bDisplay = bXDSBuffer[nOffset];

		if (bDisplay < ' ')
			bDisplay = ' ';
		wsprintf(szTemp2, "%c", bDisplay);
		lstrcat(szTemp, szTemp2);
	}
	lstrcat(szXDSDecode, szTemp);
}

void convertXdsToStandardChar(BYTE * pBuffer, int start)
{
	int i;

	// Max XDS size is 32 Characters
	for (i = start; i < 32; i++)
	{
		if (pBuffer[i] >= 0x20 && pBuffer[i] <= 0x7F)
			pBuffer[i] = (BYTE)stndChar[pBuffer[i]];
	}
}

void miscClass01(char * szXDSDecode, BYTE * pXDSData)
{
	int minute = pXDSData[0] & 0x3F;
	int hour   = pXDSData[1] & 0x1F;
	int date   = pXDSData[2] & 0x1F;
	int month  = pXDSData[3] & 0x0F;
	int day    = pXDSData[4] & 0x07;
	int year   = pXDSData[5] & 0x3F;
	char szTemp[256];

	int T      = (pXDSData[3] & 0x10) >> 4;	// Tape Delay
	int Z      = (pXDSData[3] & 0x20) >> 5;  // Reset seconds to zero
	int D      = (pXDSData[1] & 0x20) >> 5;  // Daylight savings
	int L      = (pXDSData[2] & 0x20) >> 5;  // Leap Year date info (Feb28/Feb29)

	year += 1990;

	wsprintf(szTemp, "Time of Day. Date: %s %02d/%02d/%04d Time: %02d:%02d Tape Delay: %d Zero out time: %d DayLight Savings: %d Leap Year Info (Feb 28/Feb 29): %d",
		   dayString[day], month, date, year, hour, minute, T, Z, D, L);
	lstrcat(szXDSDecode, szTemp);
}

void miscClass02(char * szXDSDecode, BYTE * pXDSData)
{
	int minute = pXDSData[0] & 0x3F;
	int hour   = pXDSData[1] & 0x1F;
	int date   = pXDSData[2] & 0x1F;
	int month  = pXDSData[3] & 0x0F;
	int length_m = pXDSData[4] & 0x3F;
	int length_h = pXDSData[5] & 0x3F;
	char szTemp[256];

	wsprintf(szTemp, "Impulse Capture ID. Date: %02d/%02d Time: %02d:%02d Length of program: %02d:%02d",
		     month, date, hour, minute, length_h, length_m);
	lstrcat(szXDSDecode, szTemp);
}

void miscClass03(char * szXDSDecode, BYTE * pXDSData) 
{
	int F = (pXDSData[0] >> 5) & 0x01;
	int N = pXDSData[0] & 0x1F;
	char szTemp[256];

	wsprintf(szTemp, "Supplemental Data Character Location. Field: %d Line number: %d",
		     F + 1, N);
	lstrcat(szXDSDecode, szTemp);
}

void miscClass04(char * szXDSDecode, BYTE * pXDSData)
{
	int D = (pXDSData[0] >> 5) & 0x01;
	int h = pXDSData[0] & 0x1F;
	char szTemp[256];

	wsprintf(szTemp, "Local Time Zone & DST. DayLight Savings: %d. Time Zone Offset: %d", D, h);
	lstrcat(szXDSDecode, szTemp);
}

void miscClass64(char * szXDSDecode, BYTE * pXDSData)
{
	int lo = pXDSData[0] & 0x3F;
	int hi = pXDSData[1] & 0x3F;
	int channel = (hi << 6) | lo;
	char szTemp[256];

	wsprintf(szTemp, "Out-of-Band Channel Number. Channel: %d", channel);
	lstrcat(szXDSDecode, szTemp);
}

void miscClass65(char * szXDSDecode, BYTE * pXDSData)
{
	int lo = pXDSData[0] & 0x3F;
	int hi = pXDSData[1] & 0x0F;
	int channel = (hi << 6) | lo;
	char szTemp[256];

	wsprintf(szTemp, "Channel Map Pointer. Channel: %d (0x%03x)", channel, channel);
	lstrcat(szXDSDecode, szTemp);
}

void miscClass66(char * szXDSDecode, BYTE * pXDSData)
{
	int lo = pXDSData[0] & 0x3F;
	int hi = pXDSData[1] & 0x0F;
	int channel = (hi << 6) | lo;
	int Version = pXDSData[2] & 0x3F;
	char szTemp[256];

	printf("Channel Map Header Packet. Number of Channels: %d Map Table Version:  %d",
		   channel, Version);
	lstrcat(szXDSDecode, szTemp);
}

void miscClass67(char * szXDSDecode, BYTE * pXDSData)
{
	int userChannelLow  = pXDSData[0] & 0x3F;
	int userChannelHigh = pXDSData[1] & 0x0F;
	int remapped = (pXDSData[1] >> 5) & 0x1; 
	int tuneChannelLow  = 0;
	int tuneChannelHigh = 0;
	int idStart = 2;
	int i=0, j=0;
	char id[16];
	char szTemp[256];

	if (remapped)
	{
		idStart = 4;
		tuneChannelLow  = pXDSData[2] & 0x3F;
		tuneChannelHigh = pXDSData[3] & 0x0F;
	}

	// Create the ID if it exists
	for (i = idStart; i < (idStart + 6); i++)
	{
		if ((pXDSData[i] >= 32) && (pXDSData[i] <= 128))
			id[j++] = (BYTE)stndChar[pXDSData[i]];
		else
		{
			id[j++] = 0;
			break;
		}
	}

	{
		int userChannel = (userChannelHigh << 6) | (userChannelLow);
		int tuneChannel = (tuneChannelHigh << 6) | (tuneChannelLow);
		
		wsprintf(szTemp, "Channel Map Packet. User Channel: %d Remapped: %s (%d) ",
			     userChannel, remapped ? "YES":"NO", remapped);
		lstrcat(szXDSDecode, szTemp);
		if (remapped)
		{
			wsprintf(szTemp, "Tune Channel: %d ", tuneChannel);
			lstrcat(szXDSDecode, szTemp);
		}
		if (id[0])
			wsprintf(szTemp, "Channel ID: %s", id);
		else
			wsprintf(szTemp, "Channel ID: No Channel ID Sent");		
		lstrcat(szXDSDecode, szTemp);
	}
}

void miscClassUnknown(int type, char * szXDSDecode, BYTE * pXDSData, BYTE bXDSIndex)
{
	char szTemp[256];

	wsprintf(szTemp, "Unknown type 0x%x ", type);
	lstrcat(szXDSDecode, szTemp);
	AddBufferHexASCII(szXDSDecode, pXDSData, bXDSIndex);
}

void miscClass(int type, char * szXDSDecode, BYTE * pXDSData, BYTE pXDSIndex) 
{
	switch(type)
	{
	case 1:
		miscClass01(szXDSDecode, pXDSData); 
		break;
	case 2:
		miscClass02(szXDSDecode, pXDSData); 
		break;
	case 3:
		miscClass03(szXDSDecode, pXDSData); 
		break;
	case 4:
		miscClass04(szXDSDecode, pXDSData); 
		break;
	case 64:
		miscClass64(szXDSDecode, pXDSData); 
		break;
	case 65:
		miscClass65(szXDSDecode, pXDSData); 
		break;
	case 66:
		miscClass66(szXDSDecode, pXDSData);
		break;
	case 67:
		miscClass67(szXDSDecode, pXDSData);
		break;
	default:
		miscClassUnknown(type, szXDSDecode, pXDSData, pXDSIndex); 
		break;
	}
}

void publicClass01(char * szXDSDecode, BYTE * pXDSData)
{
	int i, j;
	char category[16];
	char fips[16];
	char county[16];
	char period[16];
	char szTemp[256];

	j = 0;
	for (i=0; i<3; i++)
		category[j++] = (BYTE)stndChar[pXDSData[i]];
	category[3] = 0;

	j = 0;
	for (i=3; i<6; i++)
		fips[j++] = (BYTE)stndChar[pXDSData[i]];
	fips[3] = 0;

	j = 0;
	for (i=6; i<9; i++) 
		county[j++] = (BYTE)stndChar[pXDSData[i]];
	county[3] = 0;

	period[0] = (BYTE)stndChar[pXDSData[9]];
	period[1] = (BYTE)stndChar[pXDSData[10]];
	period[2] = 0;

	// Determine the category code
	for (i=0; i<25; i++)
	{
		if (strncmp(category, categoryCode[i], 3) == 0) 
			break;
	}
	{
		int catCode = i;

		wsprintf(szTemp, "National Weather Service Code. Event: %s (%s) FIPS: %s County: %s Valid Period: %s (15 minute segments)",
			     category, categoryCode[catCode], fips, county, period);
		lstrcat(szXDSDecode, szTemp);
	}
}

void publicClass02(char * szXDSDecode, BYTE * pXDSData)
{
	BYTE szNewBuffer[32];
	char szTemp[256];

	memcpy(szNewBuffer, pXDSData, sizeof(szNewBuffer));
	convertXdsToStandardChar(szNewBuffer, 0);
	wsprintf(szTemp, "National Weather Service Message: %s", szNewBuffer);
	lstrcat(szXDSDecode, szTemp);
}

void publicClassUnknown(int type, char * szXDSDecode, BYTE * pXDSData, BYTE bXDSIndex)
{
	char szTemp[256];

	wsprintf(szTemp, "Unknown type 0x%x ", type);
	lstrcat(szXDSDecode, szTemp);
	AddBufferHexASCII(szXDSDecode, pXDSData, bXDSIndex);
}

void publicClass(int type, char * szXDSDecode, BYTE * pXDSData, BYTE bXDSIndex)
{
	switch (type)
	{
	case 1:
		publicClass01(szXDSDecode, pXDSData);
		break;
	case 2:
		publicClass02(szXDSDecode, pXDSData);
		break;
	default:
		publicClassUnknown(type, szXDSDecode, pXDSData, bXDSIndex);
		break;
	}
}

void currentClass01(char * szXDSDecode, BYTE * bXDSData)
{
	int minute = bXDSData[0] & 0x3F;
	int hour   = bXDSData[1] & 0x1F;
	int date   = bXDSData[2] & 0x1F;
	int month  = bXDSData[3] & 0x0F;
	int t      = (bXDSData[3] & 0x10) >> 4;	// Tape Delay
	char szTemp[128];

	wsprintf(szTemp,
		     "Program Identification Number. Time (UTC): %02d:%02d Date (M/D): %02d/%02d Tape Delay: %s (%d)",
		     hour, minute, month, date, t ? "Yes":"No", t);
	lstrcat(szXDSDecode, szTemp);
}

void currentClass02(char * szXDSDecode, BYTE * bXDSData, BYTE bXDSLength)
{
	// Length & elapsed time
	int length_m = bXDSData[0] & 0x3F;
	int length_h = bXDSData[1] & 0x3F;
	int ET_m = bXDSData[2] & 0x3F;
	int ET_h = bXDSData[3] & 0x3F;
	int ET_s = bXDSData[4] & 0x3F;
	char szTemp[256];

	wsprintf(szTemp, "Program Length: %02d:%02d. ", length_h, length_m);
	lstrcat(szXDSDecode, szTemp);

	if (bXDSLength > 4)
	{
		wsprintf(szTemp, "Elapsed Time: %02d:%02d:%02d", ET_h, ET_m, ET_s);
		lstrcat(szXDSDecode, szTemp);
	}
	else if (bXDSLength > 2)
	{
		wsprintf(szTemp, "Elapsed Time: %02d:%02d", ET_h, ET_m);
		lstrcat(szXDSDecode, szTemp);
	}
}

void currentClass03(char * szXDSDecode, BYTE * bXDSData, int nXDSLength) 
{
	BYTE szNewBuffer[36];
	char szTemp[256];

	memset(szNewBuffer, 0, sizeof(szNewBuffer));
	memcpy(szNewBuffer, bXDSData, nXDSLength);
	convertXdsToStandardChar(szNewBuffer, 0);
	wsprintf(szTemp, "Program Name: %s", szNewBuffer);
	lstrcat(szXDSDecode, szTemp);
}

void currentClass04(char * szXDSDecode, BYTE * bXDSData) 
{
	int i=0;

	lstrcat(szXDSDecode, "Program Types: ");
	while (bXDSData[i] >= 0x20 && bXDSData[i] <= 0x7F)
	{
		char szTemp[256];

		wsprintf(szTemp, "%s ", programType[bXDSData[i] - 0x20]);
		lstrcat(szXDSDecode, szTemp);
		i++;
	}
}

void currentClass05(char * szXDSDecode, BYTE * bXDSData)
{
	int r = bXDSData[0] & 0x07;
	int g = bXDSData[1] & 0x07;

	int a0 = (bXDSData[0] >> 3) & 0x01;
	int a1 = (bXDSData[0] >> 4) & 0x01;
	int a2 = (bXDSData[0] >> 5) & 0x01;
	int a3 = (bXDSData[1] >> 3) & 0x01;
	int a = (a3 << 3) | (a2 << 2) | (a1 << 1) | a0;

	int D = a2;
	int L = a3; 
	int S = (bXDSData[1] >> 4) & 0x01;
	int V = (bXDSData[1] >> 5) & 0x01;

	int rSystem = 7;	// Error Message

	char szTemp[256];

	// Determine the Rating System used
	if ((a & 0x3) == 0)
		rSystem = 0;
	if ((a & 0x3) == 1)
		rSystem = 1;
	if ((a & 0x3) == 2)
		rSystem = 2;
	if (a == 3)
		rSystem = 3;
	if (a == 7) 
		rSystem = 4;
	if (a == 11)
		rSystem = 5;
	if (a == 15)
		rSystem = 6;

	wsprintf(szTemp, "Program Rating. System: %s (%d). ", ratingSystem[rSystem], a);
	lstrcat(szXDSDecode, szTemp);
	szTemp[0] = '\0';

	switch(rSystem)
	{
	case 0:
	case 2:
		wsprintf(szTemp, "Rating: %s (%d)", rating[r], r);
		break;
	case 1:
		wsprintf(szTemp, "Guidelines (g): %s (%d). Guidelines V(%d), S(%d), L(%d), D(%d)", guidelines[g], g, V, S, L, D);
		if (V && (g == 2))
			lstrcat(szTemp, "[Fantasy Violence]");
		if (V && ((g==4) || (g==5) || (g==6)))
			lstrcat(szTemp, "[Violence]");
		if (S && ((g==4) || (g==5) || (g==6)))
			lstrcat(szTemp, "[Sexual Situations]");
		if (L && ((g==4) || (g==5) || (g==6)))
			lstrcat(szTemp, "[Adult Language]");
		if (D && ((g==4) || (g==5)))
			lstrcat(szTemp, "[Sexually Suggestive Dialog]");
		break;
	case 3:
		wsprintf(szTemp, "Canadian English Rating: %s (%d)\n", CERating[g], g);
		break;
	case 4:
		wsprintf(szTemp, "Canadian French Rating: %s (%d)\n", CFRating[g], g);
		break;
	}
	if (lstrlen(szTemp))
		lstrcat(szXDSDecode, szTemp);
}

void currentClass06(char * szXDSDecode, BYTE * bXDSData)
{
	int mainLanguage = (bXDSData[0] >> 3) & 0x07;
	int mainType = bXDSData[0] & 0x07;
	int secondaryLanguage = (bXDSData[1] >> 3) & 0x07;
	int secondaryType = bXDSData[1] & 0x07;
	char szTemp[256];

	wsprintf(szTemp, "Audio Services. Primary (Language/Type): %s, %s Secondary (Language/Type): %s, %s",
		     audioLanguage[mainLanguage], audioMainType[mainType], audioLanguage[secondaryLanguage], audioSecondaryType[secondaryType]);
	lstrcat(szXDSDecode, szTemp);
}

void currentClass07(char * szXDSDecode, BYTE * bXDSData, BYTE bXDSLength)
{
	int i = 0;
	char szTemp[256];
	
	lstrcat(szXDSDecode, "Caption Services. ");

	for (i = 0; i < bXDSLength; i++)
	{
		int language = (bXDSData[i] >> 3) & 0x07;
		int type = bXDSData[i] & 0x07;
		wsprintf(szTemp, "Caption #%d: Language: %s Caption Type: %s ",
			     i + 1, audioLanguage[language], captionService[type]);
		lstrcat(szXDSDecode, szTemp);
	}
}

void currentClass08(char * szXDSDecode, BYTE * bXDSData)
{
	// todo
	int ASB = bXDSData[0] & 0x1;		// Analog Source Bit
	int APS = (bXDSData[0] >> 1) & 0x3;	// Analog Protection System
	int CGMS = (bXDSData[0] >> 3) & 0x3;
	char *CGMS_message[4];
	char *APS_message[4];

	CGMS_message[0] = "Copying is permitted without restriction";
	CGMS_message[1] = "Condition not to be used";
	CGMS_message[2] = "One generation of copies may be made";
	CGMS_message[3] = "No copying is permitted";

	APS_message[0] = "No APS (Analog Protection System)";
	APS_message[1] = "Pseudo Sync Pulse (PSP) On; Split Burst Off";
	APS_message[2] = "Pseudo Sync Pulse (PSP) On; 2 line Split Burst On";
	APS_message[3] = "Pseudo Sync Pulse (PSP) On; 4 line Split Burst On";

	printf("Copy Control System Information Packet\n");
	if (CGMS == 3)
	{
		printf("Message: %s (3)\n", CGMS_message[3]); 
		printf("         %s (%d)\n", APS_message[APS], APS);
	}
	else 
		printf("Message: %s (%d)\n", CGMS_message[CGMS], CGMS);
}

void currentClass09(char * szXDSDecode, BYTE * bXDSData, BYTE bXDSLength)
{
	int start = bXDSData[0] & 0x3F;
	int end   = bXDSData[1] & 0x3F;
	int q     = bXDSData[2] & 0x1;
	int lineStart = start + 22;
	int lineEnd   = 262 - end;
	char szTemp[256];

	wsprintf(szTemp, "Start: %d (Starting Line: %03d) End: %d (Ending Line: %03d) Aspect Ratio: 320/%d ",
		     start, lineStart, end, lineEnd, (lineEnd-lineStart));
	lstrcat(szXDSDecode, szTemp);
	if (bXDSLength > 2)
	{
		if (q)
			wsprintf(szTemp, "q: Squeezed (%d)", q);
		else 
			wsprintf(szTemp, "q: Normal (%d)", q); 
		lstrcat(szXDSDecode, szTemp);
	}
}

void currentClass12(char * szXDSDecode, BYTE * bXDSData)
{
	int i;
	int ratingValue = bXDSData[5] & 0x7;
	int advisoryValue = (bXDSData[5] & 0x8) >> 3;
	int length_m = bXDSData[6] & 0x3F;
	int length_h = bXDSData[7] & 0x3F;
	int ET_m = bXDSData[8] & 0x3F;
	int ET_h = bXDSData[9] & 0x3F;
	char szTemp[256];

	lstrcat(szXDSDecode, "Composite Packet-1. Program Type: ");
	for (i=0; i<5; i++) 
	{
		if (bXDSData[i] >= 0x20 && bXDSData[i] <= 0x7F)
		{
			wsprintf(szTemp, "%s ", programType[bXDSData[i - 0x20]]);
			lstrcat(szXDSDecode, szTemp);
		}
	}

	// There is a mistake in the specification.
	// Rating is 2 bytes.  They have only allocated 1 Byte for it.
	// I assume they are using the orignal specification where
	// Rating was a single byte.
	//lstrcat(szXDSDecode, "Program Rating: ");
	if (advisoryValue)
		wsprintf(szTemp, "Advisory, Rating = %s ", rating[ratingValue]);
	else
		wsprintf(szTemp, "No Advisory, Rating = %s ", rating[ratingValue]);
	lstrcat(szXDSDecode, szTemp);

	// Length/Elapsed Time
	wsprintf(szTemp, "Program Length: %02d:%02d Elapsed Time: %02d:%02d ", 
		     length_h, length_m, ET_h, ET_m);
	lstrcat(szXDSDecode, szTemp);

	// Title
	wsprintf(szTemp, "Title: %s", bXDSData + 10);	// Title starts in the 11th byte
	lstrcat(szXDSDecode, szTemp);
}


void currentClass13(char * szXDSDecode, BYTE * bXDSData)
{
	//todo
	// Start Time
	int minute = bXDSData[0] & 0x3F;
	int hour   = bXDSData[1] & 0x1F;
	int date   = bXDSData[2] & 0x1F;
	int month  = bXDSData[3] & 0x0F;
	int t      = (bXDSData[3] & 0x10) >> 4;	// Tape Delay
	int mainLanguage = (bXDSData[4] >> 3) & 0x07;
	int mainType = bXDSData[4] & 0x07;
	int secondaryLanguage = (bXDSData[5] >> 3) & 0x07;
	int secondaryType = bXDSData[5] & 0x07;
	int language1 = (bXDSData[6] >> 3) & 0x07;
	int language2 = (bXDSData[7] >> 3) & 0x07;
	int type1 = bXDSData[6] & 0x07;
	int type2 = bXDSData[7] & 0x07;
	char callLetters[16];

	printf("Composite Packet-1\n");
	printf("Program Identification Number (Scheduled Start Time)\n");
	printf("    Time (UTC): %d:%02d\n", hour, minute);
	printf("    Date (M/D): %02d/%02d\n", month, date);
	printf("    Tape Delay: %s (%d)\n", t ? "Yes":"No", t);

	// Audio Services
	printf("Audio Services\n");
	printf("    Primary   (Language/Type): %s, %s\n", 
	audioLanguage[mainLanguage], audioMainType[mainType]);
	printf("    Secondary (Language/Type): %s, %s\n", 
	audioLanguage[secondaryLanguage], audioSecondaryType[secondaryType]);


	// Caption Services
	printf("Caption Services\n");
	printf("    Language 1:     %s\n", audioLanguage[language1]);
	printf("    Caption Type 1: %s\n", captionService[type1]);
	printf("    Language 2:     %s\n", audioLanguage[language2]);
	printf("    Caption Type 2: %s\n", captionService[type2]);


	// Call Letters/Native Channel
	callLetters[0] = (BYTE)stndChar[bXDSData[8]];
	callLetters[1] = (BYTE)stndChar[bXDSData[9]];
	callLetters[2] = (BYTE)stndChar[bXDSData[10]];
	callLetters[3] = (BYTE)stndChar[bXDSData[11]];
	callLetters[4] = (BYTE)stndChar[bXDSData[12]];
	callLetters[5] = (BYTE)stndChar[bXDSData[13]];
	callLetters[6] = 0;
	printf("Call Letters (Station ID) / Native Channel\n");
	printf("    Station ID: %s\n", callLetters);

	//convertXdsToStandardChar(14);	// Fix character set
	printf("Network Name (Affiliation)\n");
	printf("    Name: %s\n",(bXDSData + 14));
}

void currentClass16to23(int type, char * szXDSDecode, BYTE * bXDSData)
{
	BYTE szNewBuffer[32];
	char szTemp[256];

	memcpy(szNewBuffer, bXDSData, sizeof(szNewBuffer));
	convertXdsToStandardChar(szNewBuffer, 0);
	wsprintf(szTemp, "Program Description (Row %d): %s", type - 0x0F, szNewBuffer);
	lstrcat(szXDSDecode, szTemp);
}

void currentClassUnknown(int type, char * szXDSDecode, BYTE * bXDSData, BYTE bXDSIndex)
{
	char szTemp[256];

	wsprintf(szTemp, "Unknown type 0x%x ", type);
	lstrcat(szXDSDecode, szTemp);
	AddBufferHexASCII(szXDSDecode, bXDSData, bXDSIndex);
}

void currentClass(int type, char * szXDSDecode, BYTE * pXDSData, BYTE bXDSLength)
{
	switch(type)
	{
	case  1:
		currentClass01(szXDSDecode, pXDSData);
		break;
	case  2:
		currentClass02(szXDSDecode, pXDSData, bXDSLength);
		break;
	case  3:
		currentClass03(szXDSDecode, pXDSData, bXDSLength);
		break;
	case  4:
		currentClass04(szXDSDecode, pXDSData);
		break;
	case  5:
		currentClass05(szXDSDecode, pXDSData);
		break;
	case  6:
		currentClass06(szXDSDecode, pXDSData);
		break;
	case  7:
		currentClass07(szXDSDecode, pXDSData, bXDSLength);
		break;
	case  8:
		currentClass08(szXDSDecode, pXDSData);
		break;
	case  9:
		currentClass09(szXDSDecode, pXDSData, bXDSLength);
		break;
	case 12:
		currentClass12(szXDSDecode, pXDSData);
		break;
	case 13:
		currentClass13(szXDSDecode, pXDSData);
		break;
	case 16:
	case 17:
	case 18: 
	case 19:
	case 20:
	case 21: 
	case 22:
	case 23:
		currentClass16to23(type, szXDSDecode, pXDSData);
		break;
	default:
		currentClassUnknown(type, szXDSDecode, pXDSData, bXDSLength);
		break;
	}
}

void channelClass01(char * szXDSDecode, BYTE * pXDSData)
{
	BYTE szNewBuffer[32];
	char szTemp[256];

	memcpy(szNewBuffer, pXDSData, sizeof(szNewBuffer));
	convertXdsToStandardChar(szNewBuffer, 0);
	wsprintf(szTemp, "Network Name: %s", szNewBuffer);
	lstrcat(szXDSDecode, szTemp);
}

void channelClass02(char * szXDSDecode, BYTE * pXDSData)
{
	BYTE szNewBuffer[32];
	char szTemp[256];

	memcpy(szNewBuffer, pXDSData, sizeof(szNewBuffer));
	convertXdsToStandardChar(szNewBuffer, 0);
	wsprintf(szTemp, "Callsign: %s", szNewBuffer);
	lstrcat(szXDSDecode, szTemp);
}

void channelClass03(char * szXDSDecode, BYTE * pXDSData)
{
	int minutes = pXDSData[0] & 0x3F;
	int hours   = pXDSData[1] & 0x1F;
	char szTemp[256];

	wsprintf(szTemp, "Tape Delay: Time: %dh %dm", hours, minutes);
	lstrcat(szXDSDecode, szTemp);
}

void channelClass04(char * szXDSDecode, BYTE * pXDSData)
{
	int tsid0 = pXDSData[0] & 0x0F;
	int tsid1 = pXDSData[1] & 0x0F;
	int tsid2 = pXDSData[2] & 0x0F;
	int tsid3 = pXDSData[3] & 0x0F;
	int TSID = (tsid3 << 12) | (tsid2 << 8) | (tsid1 << 4) | (tsid0);
	char szTemp[256];

	wsprintf(szTemp, "Transmission Signal Identifier (TSID): 0x%04x", TSID);
	lstrcat(szXDSDecode, szTemp);
}

void channelClassUnknown(int type, char * szXDSDecode, BYTE * pXDSData, BYTE bXDSIndex)
{
	char szTemp[256];

	wsprintf(szTemp, "Unknown type 0x%x ", type);
	lstrcat(szXDSDecode, szTemp);
	AddBufferHexASCII(szXDSDecode, pXDSData, bXDSIndex);
}

void channelClass(int type, char * szXDSDecode, BYTE * pXDSData, BYTE bXDSIndex)
{
	switch(type)
	{
	case 1:
		channelClass01(szXDSDecode, pXDSData);
		break;
	case 2:
		channelClass02(szXDSDecode, pXDSData); 
		break;
	case 3:
		channelClass03(szXDSDecode, pXDSData); 
		break;
	case 4:
		channelClass04(szXDSDecode, pXDSData); 
		break;
	default:
		channelClassUnknown(type, szXDSDecode, pXDSData, bXDSIndex); 
		break;
	}
}

void cc608(int cc, BYTE bEvenField)
{
	int type;

	if (bEvenField)
		ccv->nCaptionAreaCount[COUNTER_CC608_F2] += 2;
	else
		ccv->nCaptionAreaCount[COUNTER_CC608_F1] += 2;

//#define CC_RAW_OUTPUT
#ifdef CC_RAW_OUTPUT
	{
		char szTemp[128];
		char szField[4];
		int cc1_print = (cc >> 8) & 0x7f;
		int cc2_print = cc & 0x7f;
		if (cc1_print < ' ')
			cc1_print = ' ';
		if (cc2_print < ' ')
			cc2_print = ' ';
		if (bEvenField)
			lstrcpy(szField, "F2");
		else
			lstrcpy(szField, "F1");
		wsprintf(szTemp, "%s %02x %02x %c%c\n", szField, (cc >> 8) & 0x7f, cc & 0x7f, cc1_print, cc2_print);
		OutputDebugString(szTemp);
	}
#endif CC_RAW_OUTPUT

	//if (bEvenField && cc)
	if (cc)
	{
		if (ccv->xdsState == XDS_COLLECT_DATA)
		{
			BYTE next_value = (cc >> 8 & 0x7f);
			if (next_value < 0x10)
			{
				xdsCapture(cc);
				return;
			}
			else if (next_value < 0x20)
			{
				ccv->xdsState = XDS_IDLE;
			}
			else
			{
				ccv->xdsBuffer[ccv->xdsPosition++] = (cc >> 8) & 0x7f;
				ccv->xdsBuffer[ccv->xdsPosition++] = cc & 0x7f;
				if (ccv->xdsPosition > 32)
				{
					ccv->xdsState = XDS_IDLE;
				}
			}
			return;
		}
	}

	// Determine what type of data we have
	type = GetCCDataType(cc);
	switch (type)
	{
	case TYPE_CHAR_STANDARD:
		standardChar(cc, bEvenField); 
		break;
	case TYPE_MISC1:		
	case TYPE_MISC2:
		cntrlCodeMisc(type, cc, bEvenField);
		break;
	case TYPE_MIDROW:
		cntrlCodeMidRow(cc);
		break;
	case TYPE_PREAMBLE_ADDRESS:
		cntrlCodePreAmble(cc, bEvenField);
		break;
	case TYPE_ATTRIBUTE_1:
	case TYPE_ATTRIBUTE_2:
	case TYPE_ATTRIBUTE_3:
		cntrlCodeAttribute(type, cc);
		break;
	case TYPE_CLOSED_GROUP_EXT:
		cntrlCodeClosedGroup(cc); 
		break;
	case TYPE_CHAR_EXT1:
	case TYPE_CHAR_EXT2:
	case TYPE_CHAR_EXT3:
		cntrlCodeExtChar(type, cc, bEvenField); 
		break;
	case TYPE_XDS:
		xdsCapture(cc); 
		break; 
	default:
		unknownType(cc); 
		break;
	}
#ifdef CC_TRACE
	printData(type, cc);
#endif CC_TRACE
}

void Parse708Escape(BYTE * bEscapeSequence, int service_number)
{


}

static int b708EscapeByteCounter[] = {
		0, // 0x80 CWx Set Current Window
		0, // 0x81 CWx
		0, // 0x82 CWx
		0, // 0x83 CWx
		0, // 0x84 CWx
		0, // 0x85 CWx
		0, // 0x86 CWx
		0, // 0x87 CWx
		0, // 0x88 CLW Clear Windows
		1, // 0x89 DSW Display Windows
		1, // 0x8a HDW Hide Windows
		1, // 0x8b TGW Toggle Windows
		1, // 0x8c DLW Delete Windows
		1, // 0x8d DLY Delay
		0, // 0x8e DLC Delay Cancel
		0, // 0x8f RST Reset
		2, // 0x90 SPA Set Pen Attributes
		3, // 0x91 SPC Set Pen Color
		2, // 0x92 SPL Set Pen Location
		0, // 0x93
		0, // 0x94
		0, // 0x95
		0, // 0x96
		4, // 0x97 SWA Set Window Attribute
		6, // 0x98 DFx Define Window
		6, // 0x99 DFx
		6, // 0x9a DFx
		6, // 0x9b DFx
		6, // 0x9c DFx
		6, // 0x9d DFx
		6, // 0x9e DFx
		6  // 0x9f DFx
};

void Parse708Block(BYTE * b708Block, int service_number, int block_size)
{
	int i;

	ccv->f708ServiceActive[service_number] = TRUE;
	for (i = 0; i < block_size; i++)
	{
		switch(ccv->n708BlockStatus[service_number])
		{
		case CC708_BLOCK_CHARACTER:
			if (b708Block[i] >= 0x80 && b708Block[i] <= 0x9f)
			{
				ccv->b708EscapeBuffer[service_number][0] = b708Block[i];
				ccv->n708EscapeOffset[service_number] = 1;
				ccv->n708EscapeRemaining[service_number] = b708EscapeByteCounter[b708Block[i] - 0x80];
				if (ccv->n708EscapeRemaining[service_number])
					ccv->n708BlockStatus[service_number] = CC708_BLOCK_ESCAPE;
				else
					Parse708Escape(ccv->b708EscapeBuffer[service_number], service_number);
			}
			else if (b708Block[i] < 0x20)
			{
				switch(b708Block[i])
				{
				case 13:		// CR
					if (ccv->n708CursorColumn[service_number] > 0)
					{
						ccv->n708CursorColumn[service_number] = 0;
						if (ccv->n708CursorRow[service_number]++ == CAPTION_ROWS - 1)
							Scroll708Page(service_number);
					}				
					break;
				case 8:		// Backspace
					if (ccv->n708CursorColumn[service_number])
						ccv->n708CursorColumn[service_number]--;
					break;
				}
			} else
			{				
				// write the byte
				switch(b708Block[i])
				{
				case 0xff:
					break;
				default:
					Write708Character(service_number, b708Block[i]);
					break;
				}
			}
			break;
		case CC708_BLOCK_ESCAPE:
			ccv->b708EscapeBuffer[service_number][ccv->n708EscapeOffset[service_number]++] = b708Block[i];
			ccv->n708EscapeRemaining[service_number]--;
			if (ccv->n708EscapeRemaining[service_number] == 0)
			{
				Parse708Escape(ccv->b708EscapeBuffer[service_number], service_number);
				ccv->n708BlockStatus[service_number] = CC708_BLOCK_CHARACTER;
			}
			break;
		}
	}
}

void Parse708Packet(BYTE * b708Packet, int nPacketLength)
{
	while (nPacketLength > 0 )
	{
		int block_size = b708Packet[0] & 0x1f;
		int service_number = b708Packet[0] >> 5;

		if (service_number == 7 && block_size != 0)
		{
			int extended_service_number = b708Packet[1] >> 2;
			service_number = extended_service_number;
			b708Packet++;
			nPacketLength--;
		}
		b708Packet++; // now pointing at the data
		nPacketLength--;

		if (service_number != 0)
			Parse708Block(b708Packet, service_number, block_size);
		b708Packet += block_size;
		nPacketLength -= block_size; 
	}
}

void cc708(BYTE cc1, BYTE cc2, BYTE cc_type)
{
	BYTE c1 = cc1;
	BYTE c2 = cc2;

cc708_retry_parse:
	switch(ccv->n708PacketState)
	{
	case CC708_PACKET_IDLE:
		if (cc_type == 3)
		{
			int nSequence = cc1 >> 6;
			int nLength = cc1 & 0x3f;

			// Check sequence
			if (ccv->n708PacketSequence == -1)
				ccv->n708PacketSequence = nSequence;
			else
			{
				ccv->n708PacketSequence++;
				ccv->n708PacketSequence &= 0x03;
				if (nSequence != ccv->n708PacketSequence)
				{
					ccv->n708SequenceErrors++;
					ccv->n708PacketSequence = nSequence;
				}
			}

			// Start packet buffering
			ccv->n708CurrentPacketLength = (nLength * 2) - 1 - 1; //(less the 1 below)
			ccv->n708PacketWriteOffset = 0;
			ccv->b708Packet[ccv->n708PacketWriteOffset++] = cc2;
			ccv->n708PacketState = CC708_PACKET_RECEIVE;
		}
		else
		{
			// type 2 shouldn't occur when out of a packet
			if (ccv->n708PacketSequence != -1)
			{
				ccv->n708DataErrors++;
			}
		}
		break;
	case CC708_PACKET_RECEIVE:
		if (cc_type == 2)
		{
			ccv->b708Packet[ccv->n708PacketWriteOffset++] = cc1;
			ccv->b708Packet[ccv->n708PacketWriteOffset++] = cc2;
			ccv->n708CurrentPacketLength -= 2;
			if (ccv->n708CurrentPacketLength == 0)
			{
				Parse708Packet(ccv->b708Packet, ccv->n708PacketWriteOffset);
				ccv->n708PacketState = CC708_PACKET_IDLE;
			}
			else if (ccv->n708CurrentPacketLength < 0)
			{
				ccv->n708DataErrors++;
				ccv->n708PacketState = CC708_PACKET_IDLE;
			}
		}
		else
		{
				ccv->n708DataErrors++;
				ccv->n708PacketState = CC708_PACKET_IDLE;
				goto cc708_retry_parse;		
		}
		break;
	}
}

void DumpUserDataBuffer(BYTE * pUserData, int nUserDataLength)
{
	int i = 0;
	char szTemp[2048] = {""};
	BYTE cPicture;

	switch(ccv->nPictureCodingType)
	{
	default:
		cPicture = '?';
		break;
	case MPEG2_PICTURE_I:
		cPicture = 'I';
		break;
	case MPEG2_PICTURE_P:
		cPicture = 'P';
		break;
	case MPEG2_PICTURE_B:
		cPicture = 'B';
		break;
	}

	while (i < nUserDataLength)
	{
		int nCharacterCounter = 0;
		DWORD dwWritten;
		char szHexDump[1024] = {""};
		char szASCIIDump[1024] = {""};

		for (; i < nUserDataLength && nCharacterCounter < 16; i++)
		{
			char szTemp2[8];
			char data;

			wsprintf(szTemp2, "%02x ", pUserData[i]);
			lstrcat(szHexDump, szTemp2);

			data = pUserData[i] & 0x7f;
			if ((data) < 0x20)
				data = '.';
			wsprintf(szTemp2, "%c", data);
			lstrcat(szASCIIDump, szTemp2);
			nCharacterCounter++;
		}
		while (lstrlen(szHexDump) < 56)
			lstrcat(szHexDump, " ");

		wsprintf(szTemp, "%c-%02d: %s %s\r\n", cPicture, ccv->nTemporalReference, szHexDump, szASCIIDump);
		//OutputDebugString(szTemp);
		WriteFile(v->hCCLogFile, szTemp, lstrlen(szTemp), &dwWritten, NULL);
	}
}

void SendIPCaptions()
{
	// Once for EIA-608 captions
	if (ccv->nPictureCodingType == MPEG2_PICTURE_I)		// I-frame?
	{
		if (ccv->nPFrame608Count)		// any prior P-frame?
		{
			int i;

			for (i = 0; i < ccv->nPFrame608Count; i += 3)
			{
				cc608(ccv->bPFrameCC608[i + 0] << 8 | ccv->bPFrameCC608[i + 1], ccv->bPFrameCC608[i + 2]);
			}
			ccv->nPFrame608Count = 0;
		}
	}
	else if (ccv->nPictureCodingType == MPEG2_PICTURE_P)		// P-frame?
	{
		if (ccv->nIFrame608Count)		// any prior I-frame?
		{
			int i;

			for (i = 0; i < ccv->nIFrame608Count; i += 3)
			{
				cc608(ccv->bIFrameCC608[i + 0] << 8 | ccv->bIFrameCC608[i + 1], ccv->bIFrameCC608[i + 2]);
			}
			ccv->nIFrame608Count = 0;
		}
		if (ccv->nPFrame608Count)		// any prior P-frame?
		{
			int i;

			for (i = 0; i < ccv->nPFrame608Count; i += 3)
			{
				cc608(ccv->bPFrameCC608[i + 0] << 8 | ccv->bPFrameCC608[i + 1], ccv->bPFrameCC608[i + 2]);
			}
			ccv->nPFrame608Count = 0;
		}
	}

	// Now EIA-708
	if (ccv->nPictureCodingType == MPEG2_PICTURE_I)		// I-frame?
	{
		if (ccv->nPFrame708Count)		// any prior P-frame?
		{
			int i;

			for (i = 0; i < ccv->nPFrame708Count; i += 3)
			{
				cc708(ccv->bPFrameCC708[i + 0], ccv->bPFrameCC708[i + 1], ccv->bPFrameCC708[i + 2]);
			}
			ccv->nPFrame708Count = 0;
		}
	}
	else if (ccv->nPictureCodingType == MPEG2_PICTURE_P)		// P-frame?
	{
		if (ccv->nIFrame708Count)		// any prior I-frame?
		{
			int i;

			for (i = 0; i < ccv->nIFrame708Count; i += 3)
			{
				cc708(ccv->bIFrameCC708[i + 0], ccv->bIFrameCC708[i + 1], ccv->bIFrameCC708[i + 2]);
			}
			ccv->nIFrame708Count = 0;
		}
		if (ccv->nPFrame708Count)		// any prior P-frame?
		{
			int i;

			for (i = 0; i < ccv->nPFrame708Count; i += 3)
			{
				cc708(ccv->bPFrameCC708[i + 0], ccv->bPFrameCC708[i + 1], ccv->bPFrameCC708[i + 2]);
			}
			ccv->nPFrame708Count = 0;
		}
	}
}

void BufferATSC708User(BYTE cc_data_1, BYTE cc_data_2, BYTE cc_type)
{
	switch(ccv->nPictureCodingType)
	{
	case MPEG2_PICTURE_I:		// I-frame?
		ccv->bIFrameCC708[ccv->nIFrame708Count++] = cc_data_1;
		ccv->bIFrameCC708[ccv->nIFrame708Count++] = cc_data_2;
		ccv->bIFrameCC708[ccv->nIFrame708Count++] = cc_type;
		break;
	case MPEG2_PICTURE_P:		// P-frame?
		ccv->bPFrameCC708[ccv->nPFrame708Count++] = cc_data_1;
		ccv->bPFrameCC708[ccv->nPFrame708Count++] = cc_data_2;
		ccv->bPFrameCC708[ccv->nPFrame708Count++] = cc_type;
		break;
	case MPEG2_PICTURE_B:		// B-frame?
		cc708(cc_data_1, cc_data_2, cc_type);
		break;
	}

	if (ccv->nIFrame708Count == MAX_BUFFERED_PICTURE_DATA)
		ccv->nIFrame708Count = 0;
	if (ccv->nPFrame708Count == MAX_BUFFERED_PICTURE_DATA)
		ccv->nPFrame708Count = 0;
}

void Buffer608Data(BYTE cc_data_1, BYTE cc_data_2, BYTE cc_type)
{
	switch(ccv->nPictureCodingType)
	{
	case MPEG2_PICTURE_I:		// I-frame?
		ccv->bIFrameCC608[ccv->nIFrame608Count++] = cc_data_1;
		ccv->bIFrameCC608[ccv->nIFrame608Count++] = cc_data_2;
		ccv->bIFrameCC608[ccv->nIFrame608Count++] = cc_type;
		break;
	case MPEG2_PICTURE_P:		// P-frame?
		ccv->bPFrameCC608[ccv->nPFrame608Count++] = cc_data_1;
		ccv->bPFrameCC608[ccv->nPFrame608Count++] = cc_data_2;
		ccv->bPFrameCC608[ccv->nPFrame608Count++] = cc_type;
		break;
	case MPEG2_PICTURE_B:		// B-frame?
		cc608(cc_data_1 << 8 | cc_data_2, cc_type);
		break;
	}

	if (ccv->nIFrame608Count == MAX_BUFFERED_PICTURE_DATA)
		ccv->nIFrame608Count = 0;
	if (ccv->nPFrame608Count == MAX_BUFFERED_PICTURE_DATA)
		ccv->nPFrame608Count = 0;
}

void DumpEIA608Raw(char * szSystem, int cc1, int cc2, int field_number)
{
	DWORD dwWritten;
	char szTemp[256];
	char cCC1 = cc1;
	char cCC2 = cc2;

	if (cc1 == 0x80 && cc2 == 0x80)
		return;

	if (cCC1 < ' ' || cCC1 == 0x7f)
		cCC1 = '.';
	if (cCC2 < ' ' || cCC2 == 0x7f)
		cCC2 = '.';

	wsprintf(szTemp, " %s: EIA-608: field = %d cc1 = %02x(%c) cc2 = %02x(%c)\r\n",
			 szSystem, field_number, cc1, cCC1, cc2, cCC2);
	OutputDebugString(szTemp);
	WriteFile(v->hCCLogFile, szTemp, lstrlen(szTemp), &dwWritten, NULL);
}

void DumpEIA708Raw(char * szSystem, int cc1, int cc2, int field_number)
{
	DWORD dwWritten;
	char szTemp[256];
	char cCC1 = cc1;
	char cCC2 = cc2;

	if (cCC1 < ' ' || cCC1 == 0x7f)
		cCC1 = '.';
	if (cCC2 < ' ' || cCC2 == 0x7f)
		cCC2 = '.';

	wsprintf(szTemp, " %s: EIA-708: type = %d cc1 = %02x(%c) cc2 = %02x(%c)\r\n",
			 szSystem, field_number, cc1, cCC1, cc2, cCC2);
	WriteFile(v->hCCLogFile, szTemp, lstrlen(szTemp), &dwWritten, NULL);
}

void CCParseSystemSCTE20()
{
	static char szSystem[] = {"SCTE-20"};

	SendIPCaptions();
	{
		int cc_count = get_bits(BM_CC_THREAD, 5);
		int i;

		for (i = 0; i < cc_count; i++)
		{
			int cc_priority = get_bits(BM_CC_THREAD, 2);
			int field_number = get_bits(BM_CC_THREAD, 2);
			int line_offset = get_bits(BM_CC_THREAD, 5);
			int cc1 = get_bits(BM_CC_THREAD, 8);
			int cc2 = get_bits(BM_CC_THREAD, 8);
			int marker_bit = get_bits(BM_CC_THREAD, 1);

			if (line_offset == 11)
			{
				//if (v->fCCLogActive && (v->nCCDumpOptions & CC_DUMP_EIA_608))
					DumpEIA608Raw(szSystem, ReverseBits(cc1 & 0x7f), ReverseBits(cc2 & 0x7f), field_number);

				if (field_number == 1)
				{
					Buffer608Data(ReverseBits(cc1) & 0x7f, ReverseBits(cc2) & 0x7f, 0);
				}
				else if (field_number == 2)
				{
					Buffer608Data(ReverseBits(cc1) & 0x7f, ReverseBits(cc2) & 0x7f, 1);
				}
				else if (field_number == 3)
				{
					Buffer608Data(ReverseBits(cc1) & 0x7f, ReverseBits(cc2) & 0x7f, 0);
				}
			}
			//if (v->fCCLogActive)
			//	OutputDebugString("\n");
		}
	}
}

void CCParseSystemCCUBE()
{
	int junk = get_bits(BM_CC_THREAD, 5 * 8);		// 5 bytes of junk
	int type = get_bits(BM_CC_THREAD, 8);
	static char szSystem[] = {"C-Cube"};

	switch(type)
	{
	default:
		ccv->dwSystemType |= CC_SYSTEM_UNDEF;
		break;
	case 0x02:		// 2 byte caption - can be repeated
		{
			int junk = get_bits(BM_CC_THREAD, 8);
			int cc_data_1 = get_bits(BM_CC_THREAD, 8) & 0x7f;
			int cc_data_2 = get_bits(BM_CC_THREAD, 8) & 0x7f;
			int repeater = get_bits(BM_CC_THREAD, 8);
			cc608(cc_data_1 << 8 | cc_data_2, 0);
			if (v->fCCLogActive && (v->nCCDumpOptions & CC_DUMP_EIA_608))
				DumpEIA608Raw(szSystem, cc_data_1, cc_data_2, 0);
			if (repeater == 0x04 && cc_data_1 < 0x20)
			{
				if (v->fCCLogActive && (v->nCCDumpOptions & CC_DUMP_EIA_608))
					DumpEIA608Raw(szSystem, cc_data_1, cc_data_2, 0);
				cc608(cc_data_1 << 8 | cc_data_2, 0);
			}
		}
		break;
	case 0x04:		// 4 byte caption - not repeated
		{
			int junk = get_bits(BM_CC_THREAD, 8);
			int cc_data_1 = get_bits(BM_CC_THREAD, 8) & 0x7f;
			int cc_data_2 = get_bits(BM_CC_THREAD, 8) & 0x7f;
			cc608(cc_data_1 << 8 | cc_data_2, 0);
			if (v->fCCLogActive && (v->nCCDumpOptions & CC_DUMP_EIA_608))
				DumpEIA608Raw(szSystem, cc_data_1, cc_data_2, 0);
			cc_data_1 = get_bits(BM_CC_THREAD, 8) & 0x7f;
			cc_data_2 = get_bits(BM_CC_THREAD, 8) & 0x7f;
			cc608(cc_data_1 << 8 | cc_data_2, 0);
			if (v->fCCLogActive && (v->nCCDumpOptions & CC_DUMP_EIA_608))
				DumpEIA608Raw(szSystem, cc_data_1, cc_data_2, 0);
		}
		break;
	case 0x05:		// type 5 is used by P (Predictive) frames, so we have to hold each one until the next
					// P frame is received
		{
			int junk1 = get_bits(BM_CC_THREAD, 8 * 6);
			int type = get_bits(BM_CC_THREAD, 8);
			int junk2 = get_bits(BM_CC_THREAD, 8);
			int cc_data_1 = get_bits(BM_CC_THREAD, 8) & 0x7f;
			int cc_data_2 = get_bits(BM_CC_THREAD, 8) & 0x7f;

			if (v->fCCLogActive && (v->nCCDumpOptions & CC_DUMP_EIA_608))
				DumpEIA608Raw(szSystem, cc_data_1, cc_data_2, 0);

			if (ccv->bPFrameCC608[0] != 0)
				cc608(ccv->bPFrameCC608[0] << 8 | ccv->bPFrameCC608[1], 0);
			if (ccv->bPFrameCC608[2] != 0)
				cc608(ccv->bPFrameCC608[2] << 8 | ccv->bPFrameCC608[3], 0);
			ccv->bPFrameCC608[0] = cc_data_1;
			ccv->bPFrameCC608[1] = cc_data_2;
			ccv->bPFrameCC608[2] = 0;
			switch(type)
			{
			case 0x02:
				break;
			case 0x04:
				ccv->bPFrameCC608[2] = get_bits(BM_CC_THREAD, 8) & 0x7f;
				ccv->bPFrameCC608[3] = get_bits(BM_CC_THREAD, 8) & 0x7f;
				break;
			default:
				break;
			}
		}
		break;
	}
}

void CCParseSystemATSC()
{
	BOOL process_em_data_flag = get_bits(BM_CC_THREAD, 1);
	BOOL process_cc_data_flag = get_bits(BM_CC_THREAD, 1);
	BOOL additional_data_flag = get_bits(BM_CC_THREAD, 1);
	int cc_count = get_bits(BM_CC_THREAD, 5);
	int em_data = get_bits(BM_CC_THREAD, 8);
	int i;
	static char szSystem[] = {"ATSC"};

	SendIPCaptions();
	for (i = 0; i < cc_count; i++ )
	{
		int marker_bits = get_bits(BM_CC_THREAD, 5);
		BOOL cc_valid = get_bits(BM_CC_THREAD, 1);
		int cc_type = get_bits(BM_CC_THREAD, 2);
		BYTE cc_data_1 = get_bits(BM_CC_THREAD, 8);
		BYTE cc_data_2 = get_bits(BM_CC_THREAD, 8);

		if (cc_valid)
		{
			if (cc_type == 0 || cc_type == 1)
			{
				Buffer608Data(cc_data_1  & 0x7f, cc_data_2 & 0x7f, cc_type);
				if (v->fCCLogActive && (v->nCCDumpOptions & CC_DUMP_EIA_608))
					DumpEIA608Raw(szSystem, cc_data_1, cc_data_2, cc_type);
			}
			else
			{
				BufferATSC708User(cc_data_1, cc_data_2, cc_type);
				ccv->nCaptionAreaCount[COUNTER_CC708] += 2;
				if (v->fCCLogActive && (v->nCCDumpOptions & CC_DUMP_EIA_708))
					DumpEIA708Raw(szSystem, cc_data_1, cc_data_2, cc_type);
			}
		}
	}
}

void CCParseSystemSciAtl()
{
	int reserved_04 = get_bits(BM_CC_THREAD, 8);
	int reserved_e2 = get_bits(BM_CC_THREAD, 8);
	int reserved_b1 = get_bits(BM_CC_THREAD, 8);
	int cc1 = get_bits(BM_CC_THREAD, 8);
	int cc2 = get_bits(BM_CC_THREAD, 8);
	static char szSystem[] = {"Sci.Atl."};

	if (reserved_04 == 0x04)
	{
		SendIPCaptions();
		Buffer608Data(cc1 & 0x7f, cc2 & 0x7f, 0);
		if (v->fCCLogActive && (v->nCCDumpOptions & CC_DUMP_EIA_608))
			DumpEIA608Raw(szSystem, cc1, cc2, 0);
		{
			int reserved_b2 = get_bits(BM_CC_THREAD, 8);
			if (reserved_b2 == 0xb2)
			{
				cc1 = get_bits(BM_CC_THREAD, 8);
				cc2 = get_bits(BM_CC_THREAD, 8);
				Buffer608Data(cc1 & 0x7f, cc2 & 0x7f, 1);
				if (v->fCCLogActive && (v->nCCDumpOptions & CC_DUMP_EIA_608))
					DumpEIA608Raw(szSystem, cc1, cc2, 1);
			}
		}
	}
}

void CCParseSystemDIRECTV()
{
	int cc1, cc2;
	static char szSystem[] = {"DIRECTV"};

	get_bits(BM_CC_THREAD, 8);	// 0x10
	get_bits(BM_CC_THREAD, 8);	// 0x03
	get_bits(BM_CC_THREAD, 8);	// 0x09

	SendIPCaptions();

	cc1 = get_bits(BM_CC_THREAD, 8);
	cc2 = get_bits(BM_CC_THREAD, 8);
	Buffer608Data(cc1  & 0x7f, cc2 & 0x7f, 0);								
	if (v->fCCLogActive && (v->nCCDumpOptions & CC_DUMP_EIA_608))
		DumpEIA608Raw(szSystem, cc1, cc2, 0);

	get_bits(BM_CC_THREAD, 8);	// 0x03
	get_bits(BM_CC_THREAD, 8);	// 0x0a
	cc1 = get_bits(BM_CC_THREAD, 8);
	cc2 = get_bits(BM_CC_THREAD, 8);
	Buffer608Data(cc1  & 0x7f, cc2 & 0x7f, 1);								
	if (v->fCCLogActive && (v->nCCDumpOptions & CC_DUMP_EIA_608))
		DumpEIA608Raw(szSystem, cc1, cc2, 1);
}

void CCParseSystem32133f()
{
	int reserved = get_bits(BM_CC_THREAD, 1);
	int process_cc_data_flag = get_bits(BM_CC_THREAD, 1);
	int zero_bit = get_bits(BM_CC_THREAD, 1);
	int cc_count = get_bits(BM_CC_THREAD, 5);
	int i;
	static char szSystem[] = {"321/33f"};

	SendIPCaptions();
	reserved = get_bits(BM_CC_THREAD, 8);
	for (i = 0; i < cc_count; i++)
	{
		int one_bit = get_bits(BM_CC_THREAD, 1);
		int reserved = get_bits(BM_CC_THREAD, 4);
		int cc_valid = get_bits(BM_CC_THREAD, 1);
		int cc_type = get_bits(BM_CC_THREAD, 2);
		int cc1 = get_bits(BM_CC_THREAD, 8);
		int cc2 = get_bits(BM_CC_THREAD, 8);

		if (cc_valid)
		{
			if (cc_type == 0 || cc_type == 1)
			{
				Buffer608Data(cc1 & 0x7f, cc2 & 0x7f, cc_type);
				if (v->fCCLogActive && (v->nCCDumpOptions & CC_DUMP_EIA_608))
					DumpEIA608Raw(szSystem, cc1, cc2, cc_type);
			}
			else
			{
				BufferATSC708User(cc1, cc2, cc_type);
				ccv->nCaptionAreaCount[COUNTER_CC708] += 2;
				if (v->fCCLogActive && (v->nCCDumpOptions & CC_DUMP_EIA_708))
					DumpEIA708Raw(szSystem, cc1, cc2, cc_type);
			}
		}
	}
}

void ParseUserData(BYTE * pUserData, int nUserDataLength)
{
	if (v->fCCLogActive && (v->nCCDumpOptions & CC_DUMP_RAW_USER))
		DumpUserDataBuffer(pUserData, nUserDataLength);

	ccv->nUserDataCount += nUserDataLength;

	set_buf(BM_CC_THREAD, pUserData, 0, FALSE);
	{
		DWORD ATSC_identifier = get_bits(BM_CC_THREAD, 32);
		BYTE user_data_type_code = get_bits(BM_CC_THREAD, 8);
		if (user_data_type_code == 0x03 && ATSC_identifier == 0x47413934)
		{
			// ATSC format
			ccv->dwSystemType |= CC_SYSTEM_ATSC;
			if (!(v->nCCStreamMask & CC_SYSTEM_ATSC))
				CCParseSystemATSC();
		}
		else
		{
			set_buf(BM_CC_THREAD, pUserData, 0, FALSE);
			if (get_bits(BM_CC_THREAD, 16) == 0x0502)
			{
				// C-Cubed format
				ccv->dwSystemType |= CC_SYSTEM_CCUBE;
				if (!(v->nCCStreamMask & CC_SYSTEM_CCUBE))
					CCParseSystemCCUBE();
			}
			else
			{
				set_buf(BM_CC_THREAD, pUserData, 0, FALSE);
				if ((get_bits(BM_CC_THREAD, 16) & 0xff7f) == 0x0301)
				{
					// SCTE 20 format
					ccv->dwSystemType |= CC_SYSTEM_SCTE_20;
					if (!(v->nCCStreamMask & CC_SYSTEM_SCTE_20))
						CCParseSystemSCTE20();
				}
				else
				{
					set_buf(BM_CC_THREAD, pUserData, 0, FALSE);
					if (get_bits(BM_CC_THREAD, 32) == 0x53415544)	// SAUD = Sci Atl User Data!
					{
						ccv->dwSystemType |= CC_SYSTEM_SCIATL;
						if (!(v->nCCStreamMask & CC_SYSTEM_SCIATL))
							CCParseSystemSciAtl();
					}
					else
					{
						int type;
						set_buf(BM_CC_THREAD, pUserData, 0, FALSE);
						type = get_bits(BM_CC_THREAD, 16);
						if (type == 0x0321 || type == 0x033f)
						{
							ccv->dwSystemType |= CC_SYSTEM_321_33F;
							if (!(v->nCCStreamMask & CC_SYSTEM_321_33F))
								CCParseSystem32133f();
						}
						else
						{
							set_buf(BM_CC_THREAD, pUserData, 0, FALSE);
							if ((get_bits(BM_CC_THREAD, 16)) == 0x0602)
							{
								int i;

								if (nUserDataLength >= 15)
								{
									// Throw away unknown stuff
									for (i = 0; i < 13; i++)
										get_bits(BM_CC_THREAD, 8);

									if (get_bits(BM_CC_THREAD, 8) == 0x07)
									{
										ccv->dwSystemType |= CC_SYSTEM_DIRECTV;
										if (!(v->nCCStreamMask & CC_SYSTEM_DIRECTV))
											CCParseSystemDIRECTV();
									}
								}
							}
							else
							{
								set_buf(BM_CC_THREAD, pUserData, 0, FALSE);
								if (get_bits(BM_CC_THREAD, 32) == 0x47413934)
								{
									int user_data_start_code = get_bits(BM_CC_THREAD, 8);
									if (user_data_start_code == 0x04 || user_data_start_code == 0x05)
									{
										int marker_bits = get_bits(BM_CC_THREAD, 3);
										if (marker_bits == 0x07)
										{
											// SCTE-21 data - but we don't parse that right now
											ccv->dwSystemType |= CC_SYSTEM_SCTE_21;
										}
									}
									else
									{
										ccv->dwSystemType |= CC_SYSTEM_UNDEF;
									}
								}
							}							
						}
					}
				}
			}
		}
	}
}

void ExtractH264UserData(BYTE * pPESPacket, int nPESLength)
{
	int nCurrentPos = 0;
	sps_t sps_storage;
	sps_t* sps = &sps_storage;

	memset(sps, 0, sizeof(sps_t));
	while (nCurrentPos < nPESLength - 4)
	{
		if (   pPESPacket[nCurrentPos + 0] == 0x00
			&& pPESPacket[nCurrentPos + 1] == 0x00
			&& pPESPacket[nCurrentPos + 2] == 0x01)
		{
			switch (pPESPacket[nCurrentPos + 3] & 0x1f)
			{
			case 6:		// Supplemental enhancement information (SEI)
				{
					int nSEIOffset = nCurrentPos + 4;

					while (nSEIOffset < nPESLength - 2)
					{
						int payloadSize = 0;
						int payloadType = 0;

						if (   pPESPacket[nSEIOffset + 0] == 0x00
							&& pPESPacket[nSEIOffset + 1] == 0x00
							&& pPESPacket[nSEIOffset + 2] == 0x01)
						{
							break;
						}
						if (pPESPacket[nSEIOffset] == 0xff)
						{
							payloadType += 255;
							nSEIOffset++;
						}
						payloadType += pPESPacket[nSEIOffset++];
						if (pPESPacket[nSEIOffset] == 0xff)
						{
							payloadSize += 255;
							nSEIOffset++;
						}
						payloadSize += pPESPacket[nSEIOffset++];
						if (payloadSize == 0)
							break;
						if (payloadType == 4)
						{
							if (pPESPacket[nSEIOffset] == 0xb5)	// itu_t_t35_country_code for ATSC
							{
								//if (   pPESPacket[nSEIOffset + 1] == 0x00 && pPESPacket[nSEIOffset + 2] == 0x31		// ATSC registration
								//	|| pPESPacket[nSEIOffset + 1] == 0xaa && pPESPacket[nSEIOffset + 2] == 0x55)	// Some Modulus video
								{
									// Save the packet because we don't yet know the picture
									// type for this picture
									if (ccv->nH264UserBufferSize)
									{
										int a=1;	// shouldn't happen
									}
									memcpy(ccv->bH264UserBuffer, &pPESPacket[nSEIOffset + 3], payloadSize - 3);
									ccv->nH264UserBufferSize = payloadSize - 3;
								}
							}
						}
						nSEIOffset += payloadSize;
					}
					nCurrentPos = nSEIOffset - 1;
				}
				break;
			case 1:		// Coded slice of a non-IDR picture
			case 5:		// Coded slice of an IDR picture
			case 19:	// Coded slice of an auxiliary coded picture without partitioning
				{
					uint32_t first_mb_in_slice;
					uint32_t slice_type;
					//uint32_t pic_parameter_set_id;
					//uint32_t frame_num;
					bs_t b;
					
					memset(&b, 0, sizeof(b));
					bs_init(&b, &pPESPacket[nCurrentPos + 4], nPESLength - (nCurrentPos + 4));
					
					first_mb_in_slice = bs_read_ue(&b);
					slice_type = bs_read_ue(&b);
					if (slice_type > 4)
						slice_type -= 5;
					//pic_parameter_set_id = bs_read_ue(&b);
					//frame_num = bs_read_u(&b, sps->log2_max_frame_num_minus4 + 4 ); // was u(v)

					switch(slice_type)
					{
					case SH_SLICE_TYPE_P:
						ccv->nPictureCodingType = MPEG2_PICTURE_P;
						break;
					case SH_SLICE_TYPE_B:
						ccv->nPictureCodingType = MPEG2_PICTURE_B;
						break;
					case SH_SLICE_TYPE_I:
						ccv->nPictureCodingType = MPEG2_PICTURE_I;
						break;
					case SH_SLICE_TYPE_SP:
						ccv->nPictureCodingType = MPEG2_PICTURE_P;
						break;
					case SH_SLICE_TYPE_SI:
						ccv->nPictureCodingType = MPEG2_PICTURE_I;
						break;
					}
					if (ccv->nH264UserBufferSize)
					{
						ParseUserData(ccv->bH264UserBuffer, ccv->nH264UserBufferSize);
						ccv->nH264UserBufferSize = 0;
					}
				}
				break;
			/*case 7:		// Sequence parameter set
				{
					int i;
					bs_t b;
					
					memset(&b, 0, sizeof(b));
					bs_init(&b, &pPESPacket[nCurrentPos + 4], nPESLength - (nCurrentPos + 4));

					sps->profile_idc = bs_read_u8(&b);
					sps->constraint_set0_flag = bs_read_u1(&b);
					sps->constraint_set1_flag = bs_read_u1(&b);
					sps->constraint_set2_flag = bs_read_u1(&b);
					sps->constraint_set3_flag = bs_read_u1(&b);
					sps->reserved_zero_4bits = bs_read_u(&b,4);  // all 0's
					sps->level_idc = bs_read_u8(&b);
					sps->seq_parameter_set_id = bs_read_ue(&b);
					if( sps->profile_idc == 100 || sps->profile_idc == 110 ||
						sps->profile_idc == 122 || sps->profile_idc == 144 ) {
						sps->chroma_format_idc = bs_read_ue(&b);
						if( sps->chroma_format_idc == 3 )
							sps->residual_colour_transform_flag = bs_read_u1(&b);
						sps->bit_depth_luma_minus8 = bs_read_ue(&b);
						sps->bit_depth_chroma_minus8 = bs_read_ue(&b);
						sps->qpprime_y_zero_transform_bypass_flag = bs_read_u1(&b);
						sps->seq_scaling_matrix_present_flag = bs_read_u1(&b);
						if( sps->seq_scaling_matrix_present_flag )
							for( i = 0; i < 8; i++ ) {
								sps->seq_scaling_list_present_flag[ i ] = bs_read_u1(&b);
								if( sps->seq_scaling_list_present_flag[ i ] )
									if( i < 6 )
										read_scaling_list(&b, sps->ScalingList4x4[ i ], 16,
													  sps->UseDefaultScalingMatrix4x4Flag[ i ]);
									else
										read_scaling_list(&b, sps->ScalingList8x8[ i - 6 ], 64,
													  sps->UseDefaultScalingMatrix8x8Flag[ i - 6 ] );
							}
					}
					sps->log2_max_frame_num_minus4 = bs_read_ue(&b);
					sps->pic_order_cnt_type = bs_read_ue(&b);
					if( sps->pic_order_cnt_type == 0 )
						sps->log2_max_pic_order_cnt_lsb_minus4 = bs_read_ue(&b);
					else if( sps->pic_order_cnt_type == 1 ) {
						sps->delta_pic_order_always_zero_flag = bs_read_u1(&b);
						sps->offset_for_non_ref_pic = bs_read_se(&b);
						sps->offset_for_top_to_bottom_field = bs_read_se(&b);
						sps->num_ref_frames_in_pic_order_cnt_cycle = bs_read_ue(&b);
						for( i = 0; i < sps->num_ref_frames_in_pic_order_cnt_cycle; i++ )
							sps->offset_for_ref_frame[ i ] = bs_read_se(&b);
					}
					sps->num_ref_frames = bs_read_ue(&b);
					sps->gaps_in_frame_num_value_allowed_flag = bs_read_u1(&b);
					sps->pic_width_in_mbs_minus1 = bs_read_ue(&b);
					sps->pic_height_in_map_units_minus1 = bs_read_ue(&b);
					sps->frame_mbs_only_flag = bs_read_u1(&b);
					if( !sps->frame_mbs_only_flag )
						sps->mb_adaptive_frame_field_flag = bs_read_u1(&b);
					sps->direct_8x8_inference_flag = bs_read_u1(&b);
					sps->frame_cropping_flag = bs_read_u1(&b);
					if( sps->frame_cropping_flag ) {
						sps->frame_crop_left_offset = bs_read_ue(&b);
						sps->frame_crop_right_offset = bs_read_ue(&b);
						sps->frame_crop_top_offset = bs_read_ue(&b);
						sps->frame_crop_bottom_offset = bs_read_ue(&b);
					}
					sps->vui_parameters_present_flag = bs_read_u1(&b);
					if( sps->vui_parameters_present_flag )
						read_vui_parameters(sps, &b);
					//read_rbsp_trailing_bits(h, b);
				}
				break;*/
			}
		}
		nCurrentPos++;
	}
}

void ExtractMPEG2UserData(BYTE * pPESPacket, int nPESLength)
{
	int nCurrentPos = 0;

	while (nCurrentPos < nPESLength - 4)
	{
		if (   pPESPacket[nCurrentPos + 0] == 0x00
			&& pPESPacket[nCurrentPos + 1] == 0x00
			&& pPESPacket[nCurrentPos + 2] == 0x01)
		{
			switch (pPESPacket[nCurrentPos + 3])
			{
			case 0xb2:
				{
					int nUserDataOffset = 0;
					BYTE bUserData[1024];

					nCurrentPos += 4;
					while (nCurrentPos < nPESLength - 4)
					{
						if (   pPESPacket[nCurrentPos + 0] == 0x00
							&& pPESPacket[nCurrentPos + 1] == 0x00
							&& pPESPacket[nCurrentPos + 2] == 0x01)
							break;
						bUserData[nUserDataOffset] = pPESPacket[nCurrentPos];
						nUserDataOffset++;
						if (nUserDataOffset >= sizeof(bUserData))
						{
							nUserDataOffset = 0;
							break;
						}
						nCurrentPos++;
					}
					if (nUserDataOffset)
						ParseUserData(bUserData, nUserDataOffset);
				}
				break;
			case 0x00:	// picture start code
				{
					int nNewTemporalReference = (pPESPacket[nCurrentPos + 4] << 8 | pPESPacket[nCurrentPos + 5]) >> 6;
					if (nNewTemporalReference < MAX_GOP_SIZE)
					{
						ccv->nTemporalReference = nNewTemporalReference;
						ccv->nPictureCodingType = (pPESPacket[nCurrentPos + 5] >> 3) & 7;
					}
					nCurrentPos++;
				}
				break;
			default:
				// Some other - skip past the start code to search for the next
				nCurrentPos++;
				break;
			}
		}
		else
		{
			// Not a start code - onto the next byte in the PES packet
			nCurrentPos++;
		}
	}
}

int ReadFromCCPipe(BYTE * pBuffer, int nLength)
{
	int nRequestedLength = nLength;
	DWORD dwRead;

	while (nLength)
	{
		ReadFile(v->hCCReadPipe, pBuffer, nLength, &dwRead, NULL);
		if (dwRead == 0)
			return 0;
		EnterCriticalSection(&v->csCCPipeBytes);
		v->nCCPipeBytes -= dwRead;
		LeaveCriticalSection(&v->csCCPipeBytes);
		pBuffer += dwRead;
		nLength -= dwRead;
	}

	return nRequestedLength;
}

#define MAX_VIDEO_PES_SIZE 5 * 1024 * 1024
DWORD WINAPI CaptionDecoderThread(LPVOID lpv)
{
	BYTE * pPESPacket = LocalAlloc(LPTR, MAX_VIDEO_PES_SIZE);
	BOOL fH264Mode = (BOOL)lpv;

	InitCCDecoder();

	while(v->nCaptionPID != -1)
	{
		int nPESLength;

		EnterCriticalSection(&v->csCCPipeBytes);
		if (v->nCCPipeBytes < 4)
		{
			LeaveCriticalSection(&v->csCCPipeBytes);
			Sleep(10);
			continue;
		}
		LeaveCriticalSection(&v->csCCPipeBytes);
		
		if (ReadFromCCPipe((BYTE *)&nPESLength, sizeof(nPESLength)) != sizeof(nPESLength))
			break;

		if (nPESLength > MAX_VIDEO_PES_SIZE)
		{
			OutputDebugString("CC PES too big***************************\n");
			break;
		}
		if (ReadFromCCPipe(pPESPacket, nPESLength) != nPESLength)
			break;
		if (!fH264Mode)
			ExtractMPEG2UserData(pPESPacket, nPESLength);
		else
			ExtractH264UserData(pPESPacket, nPESLength);
	}

	LocalFree(pPESPacket);
	DeleteCriticalSection(&v->csCCPipeBytes);
	v->fCCThreadRunning = FALSE;

	return 0;
}

void FormatCCByteCounter(char * szString, int nCount)
{
	if (nCount < 1000)
		wsprintf(szString, "%d", nCount);
	else if (nCount < 1000 * 1000)
		sprintf(szString, "%.3f KB", (float)nCount / 1024.0f);
	else 
		sprintf(szString, "%.3f MB", (float)nCount / 1024.0f / 1024.0f);
}

void CCTextOut(HDC hDC, int X, int Y, WCHAR * lpwString, int nLength)
{
	ExtTextOutW(hDC, X, Y, ETO_OPAQUE, NULL, lpwString, nLength, NULL);
}


void CCPaint(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	int nRow, nColumn;
	int nXDSIndex;
	int nCurrentX, nCurrentY;
	int i;

	static int nCCStreamMask[] = {
		CC_SYSTEM_ATSC,
		CC_SYSTEM_CCUBE,
		CC_SYSTEM_SCTE_20,
		CC_SYSTEM_SCIATL,
		CC_SYSTEM_DIRECTV,
		CC_SYSTEM_SCTE_21,
		CC_SYSTEM_321_33F,
		0
	};

	static char * szCCSystemDisplay[] = {
		"ATSC",
		"CCube",
		"SCTE-20",
		"SciAtl",
		"DIRECTV",
		"SCTE-21",
		"321/33f"};

	HDC hDC, hRealDC;
	HBITMAP memBM;
	RECT rc;
	PAINTSTRUCT ps;
	SIZE sizeText;
	RECT rcBorder;
	char szCCLabel[64];
	char szTemp[256];
	char szTotalUserString[64];
	char szCCCounterString[4][64];

	GetClientRect(hWnd, &rc);
	hRealDC = BeginPaint(hWnd, &ps);
	hDC = CreateCompatibleDC(hRealDC);
    memBM = CreateCompatibleBitmap (hRealDC, rc.right, rc.bottom);
    SelectObject(hDC, memBM);

	FormatCCByteCounter(szTotalUserString, ccv->nUserDataCount);
	for (i = 0; i < 3; i++)
		FormatCCByteCounter(szCCCounterString[i], ccv->nCaptionAreaCount[i]);

	SelectObject(hDC, ccv->hCaptionSmallFont);
	SetTextColor(hDC, RGB(0xff, 0xff, 0xff));
	SetBkMode(hDC, TRANSPARENT);

	if (ccv->nCaptionAreaCount[COUNTER_CC708])
	{
		// Show 708 switch icon if 708 captions present
		DrawIconEx(hDC, 2, 4, ccv->hInfoIcon, 16, 16, 0, NULL, DI_NORMAL);
		if (!ccv->fDisplay708)
			lstrcpy(szTemp, "click for EIA-708");
		else
			lstrcpy(szTemp, "click for EIA-608");
		TextOut(hDC, 19, 3, szTemp, lstrlen(szTemp));
	}

	if (ccv->fDisplay708)
	{
		wsprintf(szTemp, "Errors: Seq: %d Data: %d", ccv->n708SequenceErrors, ccv->n708DataErrors);
		GetTextExtentPoint(hDC, szTemp, lstrlen(szTemp), &sizeText);
		TextOut(hDC, rc.right - sizeText.cx, 3, szTemp, lstrlen(szTemp));
	}
	
	wsprintf(szTemp, "Byte counters: User: %s, Line 21 Field-1: %s Field-2: %s, DTVCC: %s, System: ",
		     szTotalUserString, szCCCounterString[0], szCCCounterString[1],
			 szCCCounterString[2]);

	for (i = 0; nCCStreamMask[i] != 0; i++)
	{
		if (ccv->dwSystemType & nCCStreamMask[i])
		{
			if (v->nCCStreamMask & nCCStreamMask[i])
				lstrcat(szTemp, "(");
			lstrcat(szTemp, szCCSystemDisplay[i]);
			if (v->nCCStreamMask & nCCStreamMask[i])
				lstrcat(szTemp, ")");
			lstrcat(szTemp, " ");
		}
	}

	if (ccv->dwSystemType & CC_SYSTEM_UNDEF)
		lstrcat(szTemp, "Unknown");
	GetTextExtentPoint(hDC, szTemp, lstrlen(szTemp), &sizeText);
	TextOut(hDC, rc.right / 2 - sizeText.cx / 2, 3, szTemp, lstrlen(szTemp));
	
	SelectObject(hDC, ccv->hCaptionFixedFont);
	GetTextExtentPoint(hDC, "X", 1, &sizeText);

	if (!ccv->fDisplay708)
	{
		int nPage;
		int nPageMaxX, nPageMaxY;

		for (nPage = 0; nPage < CC608_PAGES; nPage++)
		{
			int nXOffset;
			int nYOffset;

			nXOffset = (nPage % 4) * sizeText.cx * (CAPTION_COLUMNS + 1) + 1;
			nYOffset = (nPage / 4) * sizeText.cy * (CAPTION_ROWS + 1) + 1 + (sizeText.cy + 10);

			rcBorder.left = 2 + nXOffset - 1;
			rcBorder.right = rcBorder.left + (sizeText.cx * CAPTION_COLUMNS) + 2;
			rcBorder.top = nYOffset + 1;
			rcBorder.bottom = (rcBorder.top + sizeText.cy * (CAPTION_ROWS + 1)) + 1;
			FrameRect(hDC, &rcBorder, GetStockObject(WHITE_BRUSH));
			if (nPage == CC608_PAGES-1)
			{
				nPageMaxX = rcBorder.right;
				nPageMaxY = rcBorder.bottom;
			}

			rcBorder.left = 2 + nXOffset - 1;
			rcBorder.right = rcBorder.left + (sizeText.cx * CAPTION_COLUMNS) + 1;
			rcBorder.top = nYOffset + 1;
			rcBorder.bottom = rcBorder.top + sizeText.cy + 1;
			FillRect(hDC, &rcBorder, GetStockObject(WHITE_BRUSH));

			switch(nPage)
			{
			case 0:
			case 1:
			case 2:
			case 3:
				wsprintf(szCCLabel, "CC%d", nPage + 1);
				break;
			case 4:
			case 5:
			case 6:
			case 7:
				wsprintf(szCCLabel, "Text %d", nPage - 3);
				break;
			}
			SelectObject(hDC, ccv->hCaptionSmallFont);
			SetTextColor(hDC, RGB(0x00, 0x00, 0x00));
			SetBkMode(hDC, TRANSPARENT);
			TextOut(hDC, nXOffset + 2, nYOffset + 1, szCCLabel, lstrlen(szCCLabel));

			SelectObject(hDC, GetStockObject(WHITE_PEN));
			MoveToEx(hDC, nXOffset + 1, nYOffset + sizeText.cy, NULL);
			LineTo(hDC, nXOffset + (sizeText.cx * CAPTION_COLUMNS) + 1, nYOffset + sizeText.cy);

			SelectObject(hDC, ccv->hCaptionFixedFont);
			SetTextColor(hDC, RGB(0xff, 0xff, 0xff));
			for (nRow = 0; nRow < CAPTION_ROWS; nRow++)
			{
				int nY = 2 + nYOffset + (nRow * sizeText.cy) + sizeText.cy;
				int nBufferOffset = 0;
				int nStartX = -1;
				WCHAR bOutputBuffer[CAPTION_COLUMNS];
				BYTE b608CurrentAttribute;

				nColumn = 0;
				while (nColumn < CAPTION_COLUMNS)
				{
					if (nStartX == -1)
					{
						nStartX = 2 + nXOffset + (nColumn * sizeText.cx);
						b608CurrentAttribute = ccv->b608DisplayAttributes[nPage][nRow][nColumn];
					}
					if (b608CurrentAttribute != ccv->b608DisplayAttributes[nPage][nRow][nColumn])
					{
						if (nBufferOffset)
						{
							CCTextOut(hDC, nStartX, nY, bOutputBuffer, nBufferOffset);
							nBufferOffset = 0;
							nStartX = -1;
							nColumn++;
							continue;
						}
					}
					bOutputBuffer[nBufferOffset++] = ccv->b608DisplayBuffer[nPage][nRow][nColumn];
					nColumn++;
				}
				if (nBufferOffset)
					CCTextOut(hDC, nStartX, nY, bOutputBuffer, nBufferOffset);
			}
		}

		// Now we can do the XDS
		rcBorder.top = nPageMaxY + 2;
		rcBorder.bottom = rc.bottom - 1;
		rcBorder.left = 2;
		rcBorder.right = rc.right - 2;
		FrameRect(hDC, &rcBorder, GetStockObject(WHITE_BRUSH));
		rcBorder.top = nPageMaxY + 2;
		rcBorder.bottom = rcBorder.top + sizeText.cy;
		rcBorder.left = 2;
		rcBorder.right = rc.right  - 2;
		FillRect(hDC, &rcBorder, GetStockObject(WHITE_BRUSH));

		SelectObject(hDC, ccv->hCaptionSmallFont);
		SetTextColor(hDC, RGB(0x00, 0x00, 0x00));
		SetBkMode(hDC, TRANSPARENT);
		lstrcpy(szCCLabel, "XDS");
		TextOut(hDC, 2, nPageMaxY + 2 + 1, szCCLabel, lstrlen(szCCLabel));

		nCurrentX = 4;
		nCurrentY = nPageMaxY + sizeText.cy + 2 + 3;

		for (nXDSIndex = 0; nXDSIndex < 256 * 16; nXDSIndex++)
		{
			if (ccv->bXDSLength[nXDSIndex])
			{
				BYTE bXDSCode = nXDSIndex >> 8;
				BYTE bXDSType = nXDSIndex & 0xff;
				char szXDSDecode[1024];

				switch (bXDSCode)
				{
				case 0x01:
					lstrcpy(szXDSDecode, "Current:  ");
					currentClass(bXDSType, szXDSDecode, ccv->bXDSBuffer[nXDSIndex], ccv->bXDSLength[nXDSIndex]);
					break;
				case 0x03:
					lstrcpy(szXDSDecode, "Future:   ");
					currentClass(bXDSType, szXDSDecode, ccv->bXDSBuffer[nXDSIndex], ccv->bXDSLength[nXDSIndex]);
					break;
				case 0x05:
					lstrcpy(szXDSDecode, "Channel:  ");
					channelClass(bXDSType, szXDSDecode, ccv->bXDSBuffer[nXDSIndex], ccv->bXDSLength[nXDSIndex]);
					break;
				case 0x07:
					lstrcpy(szXDSDecode, "Misc:     ");
					miscClass(bXDSType, szXDSDecode, ccv->bXDSBuffer[nXDSIndex], ccv->bXDSLength[nXDSIndex]);
					break;
				case 0x09:
					lstrcpy(szXDSDecode, "Public:   ");
					publicClass(bXDSType, szXDSDecode, ccv->bXDSBuffer[nXDSIndex], ccv->bXDSLength[nXDSIndex]);
					break;
				default:
					//                    "0x0/0x0: "
					wsprintf(szXDSDecode, "0x%1x/0x%02x: ", bXDSCode, bXDSType);
					AddBufferHexASCII(szXDSDecode, ccv->bXDSBuffer[nXDSIndex], ccv->bXDSLength[nXDSIndex]);
					break;
				}
				SelectObject(hDC, ccv->hCaptionFixedFont);
				SetTextColor(hDC, RGB(0xff, 0xff, 0xff));
				TextOut(hDC, nCurrentX, nCurrentY, szXDSDecode, lstrlen(szXDSDecode));
				nCurrentY += sizeText.cy;
				if (nCurrentY + sizeText.cy >= rc.bottom)
					break;
			}
		}
	}
	else
	{
		// Show EIA708 captions
		int nService;
		int nDisplayNumber = -1;

		for (nService = 0; nService < CC708_SERVICES; nService++)
		{
			int nXOffset;
			int nYOffset;

			if (!ccv->f708ServiceActive[nService])
				continue;

			nDisplayNumber++;
			if (nDisplayNumber > 11)
				break;	// too many

			nXOffset = (nDisplayNumber % 4) * sizeText.cx * (CAPTION_COLUMNS + 1) + 1;
			nYOffset = (nDisplayNumber / 4) * sizeText.cy * (CAPTION_ROWS + 1) + 1 + (sizeText.cy + 10);

			rcBorder.left = 2 + nXOffset - 1;
			rcBorder.right = rcBorder.left + (sizeText.cx * CAPTION_COLUMNS) + 1;
			rcBorder.top = nYOffset + 1;
			rcBorder.bottom = (rcBorder.top + sizeText.cy * (CAPTION_ROWS + 1)) + 1;
			FrameRect(hDC, &rcBorder, GetStockObject(WHITE_BRUSH));

			rcBorder.left = 2 + nXOffset - 1;
			rcBorder.right = rcBorder.left + (sizeText.cx * CAPTION_COLUMNS) + 1;
			rcBorder.top = nYOffset + 1;
			rcBorder.bottom = rcBorder.top + sizeText.cy + 1;
			FillRect(hDC, &rcBorder, GetStockObject(WHITE_BRUSH));

			wsprintf(szCCLabel, "Service %d", nService);
			SelectObject(hDC, ccv->hCaptionSmallFont);
			SetTextColor(hDC, RGB(0x00, 0x00, 0x00));
			SetBkMode(hDC, TRANSPARENT);
			TextOut(hDC, nXOffset + 2, nYOffset + 1, szCCLabel, lstrlen(szCCLabel));

			SelectObject(hDC, GetStockObject(WHITE_PEN));
			MoveToEx(hDC, nXOffset + 1, nYOffset + sizeText.cy, NULL);
			LineTo(hDC, nXOffset + (sizeText.cx * CAPTION_COLUMNS) + 1, nYOffset + sizeText.cy);

			SelectObject(hDC, ccv->hCaptionFixedFont);
			SetTextColor(hDC, RGB(0xff, 0xff, 0xff));
			for (nRow = 0; nRow < CAPTION_ROWS; nRow++)
			{
				int nY = 2 + nYOffset + (nRow * sizeText.cy) + sizeText.cy;
				int nBufferOffset = 0;
				int nStartX = -1;
				WCHAR bOutputBuffer[CAPTION_COLUMNS];
				BYTE b708CurrentAttribute;

				nColumn = 0;
				while (nColumn < CAPTION_COLUMNS)
				{
					if (nStartX == -1)
					{
						nStartX = 2 + nXOffset + (nColumn * sizeText.cx);
						b708CurrentAttribute = ccv->b708DisplayAttributes[nService][nRow][nColumn];
					}
					if (b708CurrentAttribute != ccv->b708DisplayAttributes[nService][nRow][nColumn])
					{
						if (nBufferOffset)
						{
							CCTextOut(hDC, nStartX, nY, bOutputBuffer, nBufferOffset);
							nBufferOffset = 0;
							nStartX = -1;
							nColumn++;
							continue;
						}
					}
					bOutputBuffer[nBufferOffset++] = ccv->b708DisplayBuffer[nService][nRow][nColumn];
					nColumn++;
				}
				if (nBufferOffset)
					CCTextOut(hDC, nStartX, nY, bOutputBuffer, nBufferOffset);
			}
		}
	}
		
	BitBlt(hRealDC, 0, 0, rc.right, rc.bottom, hDC, 0, 0, SRCCOPY);
	DeleteObject(memBM);
	DeleteDC(hDC);
	EndPaint(hWnd, &ps);
}

void EraseCC708Screen(int nService)
{
	int i;
	WCHAR * buffer = &ccv->b708DisplayBuffer[nService][0][0];

	for (i = 0; i < CAPTION_ROWS * CAPTION_COLUMNS; i++)
		*buffer++ = 0x20;
	memset(ccv->b708DisplayAttributes[nService], 0, CAPTION_ROWS * CAPTION_COLUMNS);
	ccv->n708CursorRow[nService] = 0;
	ccv->n708CursorColumn[nService] = 0;
}

void EraseCC608Screen(int nPage)
{
	int i;
	WCHAR * buffer = &ccv->b608DisplayBuffer[nPage][0][0];

	for (i = 0; i < CAPTION_ROWS * CAPTION_COLUMNS; i++)
		*buffer++ = 0x20;
	//memset(ccv->b608DisplayBuffer[nPage], ' ', CAPTION_ROWS * CAPTION_COLUMNS);
	memset(ccv->b608DisplayAttributes[nPage], 0, CAPTION_ROWS * CAPTION_COLUMNS);
	ccv->n608CursorRow[nPage] = 0;
	ccv->n608CursorColumn[nPage] = 0;
}


BOOL CALLBACK CCMaskStreamsDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg)
	{
	case WM_INITDIALOG:
		if ((v->nCCStreamMask & CC_SYSTEM_ATSC))			CheckDlgButton(hDlg, IDC_CC_MASK_1, BST_CHECKED);
		if ((v->nCCStreamMask & CC_SYSTEM_CCUBE))			CheckDlgButton(hDlg, IDC_CC_MASK_2, BST_CHECKED);
		if ((v->nCCStreamMask & CC_SYSTEM_SCTE_20))			CheckDlgButton(hDlg, IDC_CC_MASK_3, BST_CHECKED);
		if ((v->nCCStreamMask & CC_SYSTEM_SCIATL))			CheckDlgButton(hDlg, IDC_CC_MASK_4, BST_CHECKED);
		if ((v->nCCStreamMask & CC_SYSTEM_DIRECTV))			CheckDlgButton(hDlg, IDC_CC_MASK_5, BST_CHECKED);
		if ((v->nCCStreamMask & CC_SYSTEM_321_33F))			CheckDlgButton(hDlg, IDC_CC_MASK_6, BST_CHECKED);
		SetFocus(GetDlgItem(hDlg, IDOK));
		break;
	case WM_CLOSE:
		EndDialog(hDlg, FALSE);
		break;
	case WM_COMMAND:
		switch(LOWORD(wParam))
		{
		case IDOK:
			{
				v->nCCStreamMask = 0;
				if (IsDlgButtonChecked(hDlg, IDC_CC_MASK_1))		v->nCCStreamMask |= CC_SYSTEM_ATSC;
				if (IsDlgButtonChecked(hDlg, IDC_CC_MASK_2))		v->nCCStreamMask |= CC_SYSTEM_CCUBE;
				if (IsDlgButtonChecked(hDlg, IDC_CC_MASK_3))		v->nCCStreamMask |= CC_SYSTEM_SCTE_20;
				if (IsDlgButtonChecked(hDlg, IDC_CC_MASK_4))		v->nCCStreamMask |= CC_SYSTEM_SCIATL;
				if (IsDlgButtonChecked(hDlg, IDC_CC_MASK_5))		v->nCCStreamMask |= CC_SYSTEM_DIRECTV;
				if (IsDlgButtonChecked(hDlg, IDC_CC_MASK_6))		v->nCCStreamMask |= CC_SYSTEM_321_33F;
				EndDialog(hDlg, TRUE);
			}
			break;
		case IDCANCEL:
			EndDialog(hDlg, FALSE);
			break;
		}
		break;
	}

	return FALSE;
}

UINT FAR PASCAL CCDumpStreamsHook(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam) 
{ 
	switch(uMsg)
	{ 
	// WM_INITDIALOG is received after commdlg 
	//is processed. 
	case WM_INITDIALOG:
		{
			if ((v->nCCDumpOptions & CC_DUMP_RAW_USER))		CheckDlgButton(hDlg, IDC_CC_DUMP_RAW_USER, BST_CHECKED);
			if ((v->nCCDumpOptions & CC_DUMP_EIA_608))		CheckDlgButton(hDlg, IDC_CC_DUMP_EIA_608, BST_CHECKED);
			if ((v->nCCDumpOptions & CC_DUMP_EIA_708))		CheckDlgButton(hDlg, IDC_CC_DUMP_EIA_708, BST_CHECKED);
			ShowWindow(GetDlgItem(hDlg, IDC_CC_DUMP_WARNING), !v->nCCDumpOptions);
		}
		return TRUE; 
	case WM_COMMAND:
		switch (HIWORD(wParam))
		{
		case BN_CLICKED:
			v->nCCDumpOptions = 0;
			if (IsDlgButtonChecked(hDlg, IDC_CC_DUMP_RAW_USER))		v->nCCDumpOptions |= CC_DUMP_RAW_USER;
			if (IsDlgButtonChecked(hDlg, IDC_CC_DUMP_EIA_608))		v->nCCDumpOptions |= CC_DUMP_EIA_608;
			if (IsDlgButtonChecked(hDlg, IDC_CC_DUMP_EIA_708))		v->nCCDumpOptions |= CC_DUMP_EIA_708;
			ShowWindow(GetDlgItem(hDlg, IDC_CC_DUMP_WARNING), !v->nCCDumpOptions);
			break;
		}
		return FALSE;
	}
	return FALSE; 
} 

LRESULT FAR PASCAL CCDisplayDlgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch(msg)
	{
	case WM_CREATE:
		{
			HDC hDC;
			LOGFONT logFont;

			ccv->n608WriteChannel = 0;
			ccv->nPriorUserDataCount = -1;
			ccv->dwSystemType = 0;
			ccv->n708PacketSequence = -1;

			hDC = GetDC(hWnd);
			memset(&logFont, 0, sizeof(logFont));
			logFont.lfHeight =         -MulDiv(80, GetDeviceCaps(hDC, LOGPIXELSY), 720);
			logFont.lfWidth =          0;
			logFont.lfEscapement =     0;
			logFont.lfOrientation =    0;
			logFont.lfWeight =         500;
			logFont.lfItalic =         FALSE;
			logFont.lfUnderline =      FALSE;
			logFont.lfStrikeOut =      0;
			logFont.lfCharSet =        DEFAULT_CHARSET;
			logFont.lfOutPrecision =   OUT_DEFAULT_PRECIS;
			logFont.lfClipPrecision =  CLIP_DEFAULT_PRECIS;
			logFont.lfQuality =        DEFAULT_QUALITY;
			logFont.lfPitchAndFamily = FIXED_PITCH;
			lstrcpy(logFont.lfFaceName, TEXT("Courier New"));
			ccv->hCaptionFixedFont = CreateFontIndirect(&logFont);
			
			ccv->hCaptionSmallFont = CreateFont(-MulDiv(80, GetDeviceCaps(hDC, LOGPIXELSY), 720),
									   0,
									   0,
									   0,
									   900,
									   FALSE,
									   FALSE,
									   FALSE,
									   ANSI_CHARSET,
									   OUT_DEFAULT_PRECIS,
									   CLIP_DEFAULT_PRECIS,
									   ANTIALIASED_QUALITY,
									   FF_DONTCARE | VARIABLE_PITCH,
									   "Arial");				
			ReleaseDC(hWnd, hDC);
			ccv->hInfoIcon = LoadIcon(v->hInstance, MAKEINTRESOURCE(IDI_LOG_INFO));

			{
				int nPage, nService;

				for (nPage = 0; nPage < CC608_PAGES; nPage++)
					EraseCC608Screen(nPage);
				for (nService = 0; nService < CC708_SERVICES; nService++)
					EraseCC708Screen(nService);
			}
			SetTimer(hWnd, 1, 1000, NULL);
		}
		break;
	case WM_LBUTTONDOWN:
		if (ccv->nCaptionAreaCount[COUNTER_CC708])
		{
			char szTitle[256];
			
			if (ccv->fDisplay708 == 0)
			{
				wsprintf(szTitle, "%s EIA-708 Display - Program %s", gszAppName, ccv->szChannelDescription);
				ccv->fDisplay708 = 1;
			}
			else
			{
				wsprintf(szTitle, "%s EIA-608 Display - Program %s", gszAppName, ccv->szChannelDescription);
				ccv->fDisplay708 = 0;
			}
			SendMessage(hWnd, WM_SETTEXT, 0, (LPARAM)szTitle);
			InvalidateRect(hWnd, NULL, FALSE);
		}
		break;
	case WM_TIMER:
		if (ccv->nUserDataCount != ccv->nPriorUserDataCount)
		{
			ccv->nPriorUserDataCount = ccv->nUserDataCount;
			InvalidateRect(hWnd, NULL, FALSE);
		}
		break;
	case WM_PAINT:
		CCPaint(hWnd, wParam, lParam);
		break;
	case WM_SIZE:
		InvalidateRect(hWnd, NULL, FALSE);
		break;
	case WM_CLOSE:
		DestroyWindow(hWnd);
		break;
	case WM_DESTROY:
		v->nCaptionPID = -1;
		CloseHandle(v->hCCWritePipe);
		while (v->fCCThreadRunning)
			Sleep(50);
		CloseHandle(v->hCCReadPipe);
		DeleteObject(ccv->hCaptionFixedFont);
		DeleteObject(ccv->hCaptionSmallFont);
		DestroyIcon(ccv->hInfoIcon);
		LocalFree(ccv);
		SetForegroundWindow(v->hWndMainWindow);
		KillTimer(hWnd, 1);
		if (v->fCCLogActive)
		{
			v->fCCLogActive = FALSE;
			CloseHandle(v->hCCLogFile);
		}
		break;
	case WM_KEYUP:
		switch(wParam)
		{
		case VK_ESCAPE:
			PostMessage(hWnd, WM_CLOSE, 0, 0);
			break;
		case ' ':
			{
				int nPage, nService;
				int nXDSIndex;

				EnterCriticalSection(&v->csCCPipeBytes);
				Sleep(50);

				for (nPage = 0; nPage < CC608_PAGES; nPage++)
					EraseCC608Screen(nPage);
				for (nService = 0; nService < CC708_SERVICES; nService++)
					EraseCC708Screen(nService);
				for (nXDSIndex = 0; nXDSIndex < 16 * 256; nXDSIndex++)
					ccv->bXDSLength[nXDSIndex] = 0;
				InvalidateRect(v->hWndCCDisplay, NULL, FALSE);
				LeaveCriticalSection(&v->csCCPipeBytes);
			}
			break;
		case VK_F1:
			MessageBox(hWnd, "Keys available in the CC Display:\n"
				             "\n"
							 "L\t\tToggle logging of CC dump\n"
							 "M\t\tMask multiple streams\n"
							 "\n",
							 gszAppName, MB_ICONINFORMATION);
			break;
		case 'l':
		case 'L':
			if (v->fCCLogActive == FALSE)
			{
				// Just turned them on - ask for file and options
				OPENFILENAME ofn;
				char szOutputFile[MAX_PATH] = {0};

				memset( &(ofn), 0, sizeof(ofn));
				ofn.lStructSize	= sizeof(ofn);
				ofn.hwndOwner = hWnd;
				ofn.lpstrFile = szOutputFile;
				ofn.nMaxFile = MAX_PATH;
				ofn.lpstrFilter = TEXT("Text Files (*.txt)\0*.txt\0All Files (*.*)\0*.*\0\0");	
				ofn.lpstrTitle = TEXT("Save Caption Data Log");
				ofn.lpstrDefExt = TEXT("txtl");
				ofn.lpstrInitialDir = v->szRecordCCLogFolder;
				ofn.Flags =  OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT | OFN_EXPLORER | OFN_ENABLEHOOK | OFN_ENABLETEMPLATE;
				ofn.hInstance = v->hInstance;
				ofn.lpfnHook = CCDumpStreamsHook;
				ofn.lpTemplateName = MAKEINTRESOURCE(IDD_CC_DUMP_STREAMS_HOOK);
				if (myGetSaveFileName(&ofn) == TRUE)
				{
					v->hCCLogFile = CreateFile(szOutputFile, GENERIC_WRITE, 0, (LPSECURITY_ATTRIBUTES) NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, (HANDLE) NULL);
					if (v->hCCLogFile != INVALID_HANDLE_VALUE)
					{
						static char szTitle[] = {"TSReader Caption Log Data\r\n\r\n"};
						DWORD dwWritten;
						WriteFile(v->hCCLogFile, szTitle, lstrlen(szTitle), &dwWritten, NULL);
						v->fCCLogActive = TRUE;
					}
				}
			}
			else
			{
				// Turned off - close file
				v->fCCLogActive = FALSE;				
				CloseHandle(v->hCCLogFile);
			}
			break;
		case 'm':
		case 'M':
			DialogBox(v->hInstance, MAKEINTRESOURCE(IDD_CC_MASK_STREAMS), hWnd, CCMaskStreamsDlgProc);
			break;
		}
		break;
	default:
		return DefWindowProc(hWnd, msg, wParam, lParam);
	}

	return FALSE;
}

void CreateCaptionDisplayWindow(HWND hWndParent, int nCaptionPID)
{
	DWORD dwStyle;
	ATOM rcreturn;
	WNDCLASS  wc;
	char szTitle[256];
	char szCaptionPID[32];

	// Setup the window classes
	memset(&wc, 0, sizeof(wc));
	wc.style =			CS_VREDRAW | CS_HREDRAW | CS_DBLCLKS;
	wc.lpfnWndProc =	CCDisplayDlgProc;
	wc.cbClsExtra =		0;
	wc.cbWndExtra =		0;
	wc.hInstance =		v->hInstance;
	wc.hIcon =			LoadIcon(v->hInstance, MAKEINTRESOURCE(IDI_DVBSMALL_LOGO));
	wc.hCursor =		LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground =	GetStockObject(BLACK_BRUSH); 
	wc.lpszMenuName =	NULL;//MAKEINTRESOURCE(IDR_TEST);
	wc.lpszClassName =	gszCCDisplayClass;
	rcreturn = RegisterClass(&wc);

	wsprintf(szCaptionPID, v->szOutputPIDFlags, nCaptionPID);
	if (v->pChannelData[v->nCaptionChannel] != NULL)
		wsprintf(ccv->szChannelDescription, "%d (%s) VPID %s", v->nCaptionChannel, v->pChannelData[v->nCaptionChannel]->szShortName, szCaptionPID);
	else
		wsprintf(ccv->szChannelDescription, "%d VPID %s", v->nCaptionChannel, szCaptionPID);
	wsprintf(szTitle, "%s EIA-608 Display - Program %s", gszAppName, ccv->szChannelDescription);
	dwStyle = WS_OVERLAPPED | WS_CAPTION | WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_SYSMENU | WS_SIZEBOX;
	v->hWndCCDisplay = CreateWindow(gszCCDisplayClass,
						   szTitle,
						   dwStyle,
						   CW_USEDEFAULT, CW_USEDEFAULT,
						   930, 640,
						   hWndParent,
						   NULL,
						   v->hInstance,
						   0);
	if (v->hWndCCDisplay == NULL)
	{
		char szTemp[256];

		wsprintf(szTemp, "CreateWindow for the CC display failed with GetLastError() = %d", GetLastError());
		MessageBox(hWndParent, szTemp, gszAppName, MB_OK);
	}
	else
		ShowWindow(v->hWndCCDisplay, SW_SHOW);

	return;
}

void ClosedCaptionDecoderToggle(HWND hWnd)
{
	BOOL fH264Mode = FALSE;

	if (v->nCaptionPID != -1)
	{
		// Already running
		SetForegroundWindow(v->hWndCCDisplay);
	}
	else
	{
		int nCaptionPID;
		int i;
		HANDLE hThread;
		DWORD dwThreadID;

		if (v->nSelectedProgram == -1)
		{
			MessageBox(hWnd, "Before using this function, please select the program you wish to view captions from.", gszAppName, MB_ICONSTOP);
			return;
		}
		nCaptionPID = -1;
		v->fCCLogActive = FALSE;
		for (i = 0; i < MAX_ESLIST_ENTRIES && nCaptionPID == -1; i++)
		{
			if (v->pat.pmt[v->nSelectedProgram].es[i].nESPID == 0)
				break;
			switch(v->pat.pmt[v->nSelectedProgram].es[i].nStreamType)
			{
			case 0x01:		// MPEG-1
			case 0x02:		// MPEG-2
			case 0x80:		// DCII
				nCaptionPID = v->pat.pmt[v->nSelectedProgram].es[i].nESPID;
				break;
			case 0x1b:		// H.264
				fH264Mode = TRUE;
				nCaptionPID = v->pat.pmt[v->nSelectedProgram].es[i].nESPID;
				break;
			}
		}
		if (nCaptionPID == -1)
		{
			MessageBox(hWnd, "No video stream found in this program", gszAppName, MB_ICONSTOP);
			return;
		}
		if (v->fPIDScrambled[nCaptionPID])
		{
			MessageBox(hWnd, "Video stream is scrambled", gszAppName, MB_ICONSTOP);
			return;
		}

		ccv = LocalAlloc(LPTR, sizeof(CLOSEDCAPTIONVARIABLES));
		ccv->xdsState = XDS_IDLE;
		ccv->xdsPosition = 0;
		ccv->xdsLastControlCode = 0;
		ccv->xdsLastType = 0;
		ccv->previousType = -1;

		v->nCaptionChannel = v->pat.pmt[v->nSelectedProgram].nProgramNumber;
		CreateCaptionDisplayWindow(v->hWndMainWindow, nCaptionPID);
		InitializeCriticalSection(&v->csCCPipeBytes);
		v->nCCPipeBytes = 0;		
		CreatePipe(&v->hCCReadPipe, &v->hCCWritePipe, NULL, 10 * 1024 * 1024);
		v->nCaptionPID = nCaptionPID;
		hThread = CreateThread(NULL, 0, CaptionDecoderThread, (LPVOID)fH264Mode, 0, &dwThreadID);
		CloseHandle(hThread);
		v->fCCThreadRunning = TRUE;
	}
}

void InputCCData(BYTE * pPESPacket, int nPESLength, int nChartIndex)
{
	DWORD dwWritten;

	if (nPESLength <= 0)
		return;

	EnterCriticalSection(&v->csCCPipeBytes);
	v->nCCPipeBytes += sizeof(nPESLength);
	WriteFile(v->hCCWritePipe, &nPESLength, sizeof(nPESLength), &dwWritten, NULL);
	if (dwWritten != sizeof(nPESLength))
		OutputDebugString("CC Pipe write problem 1\n");
	
	v->nCCPipeBytes += nPESLength;
	WriteFile(v->hCCWritePipe, pPESPacket, nPESLength, &dwWritten, NULL);
	if ((int)dwWritten != nPESLength)
		OutputDebugString("CC Pipe write problem 2\n");
	LeaveCriticalSection(&v->csCCPipeBytes);
}

#endif PRO

