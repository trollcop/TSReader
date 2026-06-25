#include <windows.h>
#include <commctrl.h>
#include <time.h>
#include "TSReader.h"
#include "util.h"
#include "bcdmux.h"

#define PACKETSIZE		(2048)						// MPEG Program stream block size
#define CLOCKRATE		((__int64)27000000)			// MPEG System clock rate
//#define STREAMRATE		((__int64)2401587)			// Original HD stream rate 19.2 Mbps
#define STREAMRATE		((__int64)312500)
#define DEMUX			(((int)STREAMRATE * 8) / 50)// Demux value for HD content STREAMRATE / 50

#define RDBUFSIZE		(1024 * 1024 * 4)
#define WRBUFSIZE		(1024 * 1024 * 4)

#ifndef min
#define min(a,b) ((a)<(b)?(a):(b))
#define max(a,b) ((a)>(b)?(a):(b))
#endif

extern PVARIABLES v;
extern BOOL CheckForFileSplit(void);

// ------------------------------------------------------------------------------------

static void error(char* errorstr)
{
	dbg_printf("TSReaderMux: ERROR: %s\n", errorstr);
}

static void warning(char* errorstr)
{
	dbg_printf("TSReaderMux: WARNING: %s\n", errorstr);
}

// ------------------------------------------------------------------------------------

BOOL WriteFileAndUpdateCounters(HANDLE hFile, LPCVOID lpBuffer, DWORD nNumberOfBytesToWrite, LPDWORD lpNumberOfBytesWritten, LPOVERLAPPED lpOverlapped)
{

	BOOL fRetVal = WriteFile(hFile, lpBuffer, nNumberOfBytesToWrite, lpNumberOfBytesWritten, lpOverlapped);
	if (fRetVal)
	{
		v->dTotalRecorded += (double)*lpNumberOfBytesWritten;
		v->dThisFileRecorded += (double)*lpNumberOfBytesWritten;
	}
	return fRetVal;
}


// ------------------------------------------------------------------------------------

int bitpos[BM_MAX] = {0, 0, 0, 0, 0, 0};
unsigned int bitval[BM_MAX] = {0, 0, 0, 0, 0, 0};
unsigned char * bitbuf[BM_MAX] = {NULL, NULL, NULL, NULL, NULL, NULL};
unsigned int bitmask[] = {
	0x0,0x1,0x3,0x7,0xf,0x1f,0x3f,0x7f,0xff,
	0x1ff,0x3ff,0x7ff,0xfff,0x1fff,0x3fff,0x7fff,0xffff,
	0x1ffff,0x3ffff,0x7ffff,0xfffff,0x1fffff,0x3fffff,0x7fffff,0xffffff,
	0x1ffffff,0x3ffffff,0x7ffffff,0xfffffff,0x1fffffff,0x3fffffff,0x7fffffff,0xffffffff};

void set_buf(int index, unsigned char* buf, int bufsize, BOOL clear)
{
	bitpos[index] = 0;
	bitbuf[index] = buf;
	bitval[index] = (bitbuf[index][0] << 24) | (bitbuf[index][1] << 16) | (bitbuf[index][2] << 8) | bitbuf[index][3];
	if (clear)
		memset(bitbuf[index], 0, bufsize);
}

int buf_size(int index)
{
	return bitpos[index] >> 3;
}

void set_bits(int index, unsigned int val, int bits)
{
	val &= bitmask[bits];

	while (bits > 0)
	{
		int bitsleft = (8 - (bitpos[index] & 7));
		if (bits >= bitsleft)
		{
			bitbuf[index][bitpos[index] >> 3] |= val >> (bits - bitsleft);
			bitpos[index] += bitsleft;
			bits -= bitsleft;
			val &= bitmask[bits];
		}
		else
		{
			bitbuf[index][bitpos[index] >> 3] |= val << (bitsleft - bits);
			bitpos[index] += bits;
			bits = 0;
		}
	}
}

unsigned int get_bits(int index, int bits)
{
	unsigned int val;
	int left = 32 - (bitpos[index] & 31);

	if (bits < left)
	{
		val = (bitval[index] >> (left - bits)) & bitmask[bits];
		bitpos[index] += bits;
	}
	else
	{
		int pos;

		val = (bitval[index] & bitmask[left]) << (bits - left);
		bitpos[index] += left;
		bits -= left;

		pos = bitpos[index] >> 3;
		bitval[index] = (bitbuf[index][pos] << 24) | (bitbuf[index][pos + 1] << 16) | (bitbuf[index][pos + 2] << 8) | bitbuf[index][pos + 3];
		
		if (bits > 0)
		{
			val |= (bitval[index] >> (32 - bits)) & bitmask[bits];
			bitpos[index] += bits;
		}
	}

	return val;
}

unsigned int get_bit_pos(int index)
{
	return bitpos[index];
}

unsigned int get_byte_pos(int index)
{
	return bitpos[index] >> 3;
}

void set_bit_pos(int index, unsigned int newpos)
{
	unsigned int pos;

	bitpos[index] = newpos;
	pos = bitpos[index] >> 5;
	bitval[index] = (bitbuf[index][pos] << 24) | (bitbuf[index][pos + 1] << 16) | (bitbuf[index][pos + 2] << 8) | bitbuf[index][pos + 3];
}

// ------------------------------------------------------------------------------------

void set_pts(unsigned char* buf, __int64 PTS)
{
	set_buf(BM_PS_MUXER, buf, 5, TRUE);

	set_bits(BM_PS_MUXER, 2, 4);								// '0010'							4
	set_bits(BM_PS_MUXER, (unsigned int)(PTS >> 30), 3);		// PTS [32..30]						3
	set_bits(BM_PS_MUXER, 1, 1);								// marker bit						1
	set_bits(BM_PS_MUXER, (unsigned int)(PTS >> 15), 15);	// PTS [29..15]						15
	set_bits(BM_PS_MUXER, 1, 1);								// marker bit						1
	set_bits(BM_PS_MUXER, (unsigned int)PTS, 15);			// PTS [14..0]						15
	set_bits(BM_PS_MUXER, 1, 1);								// marker bit						1
}

BOOL write_pack(__int64 time)
{
	DWORD dwWritten;
	__int64 ext_time;
	unsigned char buf[64];
	
	set_buf(BM_PS_MUXER, buf, 64, TRUE);

	ext_time = time % 300;
	time = time / 300;

	set_bits(BM_PS_MUXER, 0x000001ba, 32);					// pack id								32
	set_bits(BM_PS_MUXER, 1, 2);								// 0x01									2
	set_bits(BM_PS_MUXER, (unsigned int)(time >> 30), 3);	// system_clock_reference_base			3
	set_bits(BM_PS_MUXER, 1, 1);								// marker_bit							1
	set_bits(BM_PS_MUXER, (unsigned int)(time >> 15), 15);	// system_clock_reference_base			15
	set_bits(BM_PS_MUXER, 1, 1);								// marker_bit							1
	set_bits(BM_PS_MUXER, (unsigned int)time, 15);			// system_clock_reference_base1			15
	set_bits(BM_PS_MUXER, 1, 1);								// marker_bit							1
	set_bits(BM_PS_MUXER, (unsigned int)ext_time, 9);		// system_clock_reference_extension		9
	set_bits(BM_PS_MUXER, 1, 1);								// marker_bit							1
	set_bits(BM_PS_MUXER, DEMUX, 22);						// program_mux_rate						22
	set_bits(BM_PS_MUXER, 1, 1);								// marker_bit							1
	set_bits(BM_PS_MUXER, 1, 1);								// marker_bit							1
	set_bits(BM_PS_MUXER, 31, 5);							// reserved								5
	set_bits(BM_PS_MUXER, 0, 3);								// pack_stuffing_length					3

	return WriteFileAndUpdateCounters(v->hRecordFile, buf, buf_size(BM_PS_MUXER), &dwWritten, NULL);
}

int write_system(int videostream, int vidbscl, int vidbsize, int audiostream, int audbscl, int audbsize)
{
	int len;
	DWORD dwWritten;
	unsigned char buf[64];
	set_buf(BM_PS_MUXER, buf, 64, TRUE);

	len = 6;
	if (videostream != 0)
		len += 3;
	if (audiostream != 0)
		len += 3;

	set_bits(BM_PS_MUXER, 0x000001bb, 32);					// system id							32
	set_bits(BM_PS_MUXER, len, 16);							// header_length						16
	set_bits(BM_PS_MUXER, 1, 1);								// marker_bit							1
	set_bits(BM_PS_MUXER, DEMUX, 22);						// rate_bound							22
	set_bits(BM_PS_MUXER, 1, 1);								// marker_bit							1
	set_bits(BM_PS_MUXER, 1, 6);								// audio_bound							6
	set_bits(BM_PS_MUXER, 0, 1);								// fixed_flag							1
	set_bits(BM_PS_MUXER, 0, 1);								// CSPS_flag							1
	set_bits(BM_PS_MUXER, 1, 1);								// system_audio_lock_flag				1
	set_bits(BM_PS_MUXER, 1, 1);								// system_video_lock_flag				1
	set_bits(BM_PS_MUXER, 1, 1);								// marker_bit							1
	set_bits(BM_PS_MUXER, 1, 5);								// video_bound							5
	set_bits(BM_PS_MUXER, 0, 1);								// packet_rate_restriction_flag			1
	set_bits(BM_PS_MUXER, 127, 7);							// reserved_byte						7

	if (videostream != 0)
	{
		set_bits(BM_PS_MUXER, videostream, 8);
		set_bits(BM_PS_MUXER, 0x3, 2);
		set_bits(BM_PS_MUXER, vidbscl, 1);
		set_bits(BM_PS_MUXER, vidbsize, 13);
	}

	if (audiostream != 0)
	{
		set_bits(BM_PS_MUXER, audiostream, 8);
		set_bits(BM_PS_MUXER, 0x3, 2);
		set_bits(BM_PS_MUXER, audbscl, 1);
		set_bits(BM_PS_MUXER, audbsize, 13);
	}

	WriteFileAndUpdateCounters(v->hRecordFile, buf, buf_size(BM_PS_MUXER), &dwWritten, NULL);
	return (int)dwWritten;
}

static BOOL pad_buffer(size_t pad)
{
	size_t i;
	DWORD dwWritten;
	unsigned char padbyte = 0xff;
	char buf[6];

	pad -= 6;

	buf[0] = '\x0'; buf[1] = '\x0'; buf[2] = '\x1'; buf[3] = '\xbe';
	buf[4] = (pad >> 8) & 0xff;
	buf[5] = pad & 0xff;

	if (WriteFileAndUpdateCounters(v->hRecordFile, buf, 6, &dwWritten, NULL) == FALSE)
		return FALSE;

	for (i = 0; i < pad; i++) {
		if (WriteFileAndUpdateCounters(v->hRecordFile, &padbyte, 1, &dwWritten, NULL) == FALSE)
			return FALSE;
	}

	return TRUE;
}

int make_pes_header(unsigned char* buf, int streamid, int len, __int64 PTS, __int64 DTS)
{
	int hdrlen = 0;
	int PTS_DTS_flags = 0;
	if (PTS != -1)
	{
		if (DTS != -1)
		{
			PTS_DTS_flags = 3;
			hdrlen += 10;
		}
		else
		{
			PTS_DTS_flags = 2;
			hdrlen += 5;
		}
	}

	set_buf(BM_PS_MUXER, buf, 9 + hdrlen, TRUE);

	set_bits(BM_PS_MUXER, 0x000001, 24);						// packet_start_code_prefix				24
	set_bits(BM_PS_MUXER, (unsigned int)streamid, 8);		// directory_stream_id					8
	set_bits(BM_PS_MUXER, len, 16);							// PES_packet_length					16
	set_bits(BM_PS_MUXER, 0x2, 2);							// '10'									2
	set_bits(BM_PS_MUXER, 0, 2);								// PES_scrambling_control				2
	set_bits(BM_PS_MUXER, 1, 1);								// PES_priority							1
	set_bits(BM_PS_MUXER, 0, 1);								// data_alignment_indicator				1
	set_bits(BM_PS_MUXER, 0, 1);								// copyright							1
	set_bits(BM_PS_MUXER, 0, 1);								// original_or_copy						1
	set_bits(BM_PS_MUXER, PTS_DTS_flags, 2);					// PTS_DTS_flags						2
	set_bits(BM_PS_MUXER, 0, 1);								// ESCR_flag							1
	set_bits(BM_PS_MUXER, 0, 1);								// ES_rate_flag							1
	set_bits(BM_PS_MUXER, 0, 1);								// DSM_trick_mode_flag					1
	set_bits(BM_PS_MUXER, 0, 1);								// additional_copy_info_flag			1
	set_bits(BM_PS_MUXER, 0, 1);								// PES_CRC_flag							1
	set_bits(BM_PS_MUXER, 0, 1);								// PES_extension_flag					1
	set_bits(BM_PS_MUXER, hdrlen, 8);						// PES_header_data_length				8
	
	if (PTS_DTS_flags == 2)
	{
		set_bits(BM_PS_MUXER, 2, 4);								// '0010'							4
		set_bits(BM_PS_MUXER, (unsigned int)(PTS >> 30), 3);		// PTS [32..30]						3
		set_bits(BM_PS_MUXER, 1, 1);								// marker bit						1
		set_bits(BM_PS_MUXER, (unsigned int)(PTS >> 15), 15);	// PTS [29..15]						15
		set_bits(BM_PS_MUXER, 1, 1);								// marker bit						1
		set_bits(BM_PS_MUXER, (unsigned int)PTS, 15);			// PTS [14..0]						15
		set_bits(BM_PS_MUXER, 1, 1);								// marker bit						1
	}
	else if (PTS_DTS_flags == 3)
	{
		set_bits(BM_PS_MUXER, 3, 4);								// '0011'							4
		set_bits(BM_PS_MUXER, (unsigned int)(PTS >> 30), 3);		// PTS [32..30]						3
		set_bits(BM_PS_MUXER, 1, 1);								// marker bit						1
		set_bits(BM_PS_MUXER, (unsigned int)(PTS >> 15), 15);	// PTS [29..15]						15
		set_bits(BM_PS_MUXER, 1, 1);								// marker bit						1
		set_bits(BM_PS_MUXER, (unsigned int)PTS, 15);			// PTS [14..0]						15
		set_bits(BM_PS_MUXER, 1, 1);								// marker bit						1
		set_bits(BM_PS_MUXER, 1, 4);								// '0001'							4
		set_bits(BM_PS_MUXER, (unsigned int)(DTS >> 30), 3);		// DTS [32..30]						3
		set_bits(BM_PS_MUXER, 1, 1);								// marker bit						1
		set_bits(BM_PS_MUXER, (unsigned int)(DTS >> 15), 15);	// DTS [29..15]						15
		set_bits(BM_PS_MUXER, 1, 1);								// marker bit						1
		set_bits(BM_PS_MUXER, (unsigned int)DTS, 15);			// DTS [14..0]						15
		set_bits(BM_PS_MUXER, 1, 1);								// marker bit						1
	}

	return buf_size(BM_PS_MUXER);
}

BOOL write_file_header(int vidstreamid, int audstreamid)
{ 
	int nSystemBlockLen;

	// Write the first pack header
	if (!write_pack(0))
	{
		error("Couldn't write system pack header!");
		return FALSE;
	}

	// Write the system block
	nSystemBlockLen = write_system(vidstreamid, 1, 4096, audstreamid, 0, 58); // 4096k for video packets 58 for audio??
	if (!nSystemBlockLen)	
	{
		error("Couldn't write system header!");
		return FALSE;
	}

	// Pad out to even multiple of PACKETSIZE
	if (!pad_buffer(PACKETSIZE - nSystemBlockLen))
	{
		error("Couldn't write pad buffer!");
		return FALSE;
	}

	return TRUE;
}

// ------------------------------------------------------------------------------------

#define VIDEO 0
#define AUDIO 1

// Packet arrays
unsigned char * packetbuf[2];
BOOL foundfirst[2];
BOOL skipbad[2];
BYTE streamid[2];
int packetpos[2];
BYTE audiosync[2];
BYTE audiosyncmask[2];

// Set the starting time value
__int64 PCR;
__int64 first_video_PCR;
BOOL doing_iframe;

int PS__StartWriting(void)
{
	//if (v->nRecordVideoPID != v->nRecordPCRPID)
	//{
	//	MessageBox(NULL, "The PCR PID is seperate from the video PID - this isn't supported yet", NULL, MB_ICONSTOP);
	//	return 1;
	//}
	
	PCR = -1;

	// Get the video PID
	packetbuf[VIDEO] = LocalAlloc(LPTR, 1024 * 1024);
	packetpos[VIDEO] = 0;
	foundfirst[VIDEO] = 0;
	skipbad[VIDEO] = FALSE;
	streamid[VIDEO] = 0xE0;	// MPEG2 video stream id

	// Get the audio PID
	packetbuf[AUDIO] = LocalAlloc(LPTR, 1024 * 1024);
	packetpos[AUDIO] = 0;
	foundfirst[AUDIO] = FALSE;
	skipbad[AUDIO] = FALSE;
	switch(v->nRecordAudioType[0])
	{
	case 0x03:		// MPEG-1 audio
	case 0x04:		// MPEG-2 audio
		streamid[AUDIO] = 0xC0;
		audiosync[0] = 0xff; audiosync[1] = 0xf8;
		audiosyncmask[0] = 0xff; audiosyncmask[1] = 0xf8;
		break;
	case 0x06:
	case 0x81:		// AC-3 audio
		streamid[AUDIO] = 0xBD;	// AC3, etc. streamid
		audiosync[0] = 0x0b; audiosync[1] = 0x77;
		audiosyncmask[0] = 0xff; audiosyncmask[1] = 0xff;
		break;
	}
		
	// Write the first pack header
	if (!write_file_header(streamid[VIDEO], streamid[AUDIO]))
		return 1;

	return 0;	
}

int PS__StopWriting(void)
{
	DWORD dwWritten;

	// Mark last valid stream pos
	unsigned char end[4] = {0x00, 0x00, 0x01, 0xb9};
	WriteFileAndUpdateCounters(v->hRecordFile, &end, 4, &dwWritten, NULL);

	// Delete video/audio buffers
	LocalFree(packetbuf[VIDEO]);
	LocalFree(packetbuf[AUDIO]);
	
	return 0;
}

int PS__TranslateToProgramStream(BYTE * pPacketData, int nLength)
{
	int nPacketOffset;
	int curstream;

	for (nPacketOffset = 0; nPacketOffset < nLength; nPacketOffset += 188)
	{		
		BOOL start;
		BOOL errorbit;
		BOOL random;
		int pid;
		int adapt_len;
		int adaption;
		BYTE * buf = pPacketData + nPacketOffset;

		if (CheckForFileSplit() == TRUE)
		{
			// Set the starting time value
			PCR = -1;

			// Write the first pack header
			if (!write_file_header(streamid[VIDEO], streamid[AUDIO]))
					return 1;
		}

		// Get pid
		pid = (((buf[1] & 0x1F) << 8) | buf[2]) & 0x1FFF;
		if (pid != v->nRecordAudioPID[0] && pid != v->nRecordVideoPID && pid != v->nRecordPCRPID)
			continue;

		// Get the pos and buf
		if (pid == v->nRecordVideoPID)
			curstream = 0;
		else
			curstream = 1;

		// Get start code
		start = (buf[1] & 0x40) != 0;
		if (start && curstream == 1)
		{
			;
		}
		if (!start && skipbad[curstream])
			continue;

		// Get error
		errorbit = (buf[1] & 0x80) != 0;
		if (errorbit)
		{
			skipbad[curstream] = TRUE;
			continue;
		}
		
		// Get adaption header info
		adaption = (buf[3] & 0x30) >> 4;
		adapt_len = 0;

		// Get adaption header size
		if (adaption == 0)
		{
			warning("Bad adaption code (code was 0)!");
			skipbad[AUDIO] = skipbad[VIDEO] = TRUE;
			continue;
		}
		else if (adaption == 0x2)
			adapt_len = 184;
		else if (adaption == 0x3)
		{
			adapt_len = buf[4] + 1;
			if (adapt_len > 184)
			{
				warning("Invalid adapt len (was > 183)!");
				skipbad[AUDIO] = skipbad[VIDEO] = TRUE;
			}
		}

		// HBO is slick, it doesn't bother to sync AC3 packets with PES elementary stream packets.. so
		// we have to swizzle them together!  (ARGHH!)
		if (pid == v->nRecordAudioPID[0] && start)
		{
			// Is there an AC3 packet start 0b77 code in this packet??
			BOOL sync_found = FALSE;
			unsigned char * p = buf + 4 + adapt_len;
			while (p <= buf + 186)
			{
				if ((p[0] & audiosyncmask[0]) == audiosync[0] && (p[1] & audiosyncmask[1]) == audiosync[1])
				{
					sync_found = TRUE;
					break;
				}
				p++;
			}

			// Couldn't find an AC3 sync start in this packet.. don't make a PES packet!
			if (!sync_found)
			{
//					int pos = ftell(fin);
					warning("Audio packet sync not found in start frame");
//					return 1;
				adapt_len += 9 + buf[4 + adapt_len + 8];	
				start = FALSE;
			}
		}

		// Get PCR
		if (pid == v->nRecordPCRPID && v->nRecordPCRPID != v->nRecordVideoPID)
		{
			start = 1;
		}
		if (start && (adaption & 0x2) && (buf[5] & 0x10))
		{
			__int64 PCR_base = ((__int64)buf[6] << 25) | ((__int64)buf[7] << 17) | 
				  ((__int64)buf[8] << 9) | ((__int64)buf[9] << 1) | ((__int64)buf[10] >> 7);
			__int64 PCR_ext = ((__int64)(buf[10] & 0x1) << 8) | ((__int64)buf[11]);
			PCR = PCR_base * 300 + PCR_ext;
			if (v->nRecordVideoPID != v->nRecordPCRPID)
				continue;
		}

		// Get random
		random = start;	// Oops.. random bit seems to be used only from some broadcasters???

		// Found a random access point (now we can start a frame/audio packet..)
		if (random)
		{
			// Check to see if this is an i_frame (group of picture start)
			if (pid == v->nRecordVideoPID)
			{
				// Look for the Group of Pictures packet.. indicates this is an I-Frame packet..
				unsigned int strid = 0;
				int i;

				doing_iframe = FALSE;
				for (i = 4 + adapt_len; i < 188; i++)
				{
					strid = (strid << 8) | buf[i];
					if (strid == 0x000001B8) // group_start_code
					{
						// found a Group of Pictures header, subsequent picture must be an I-frame
						doing_iframe = TRUE;
					}
					else if (strid == 0x00000100) // picture_start_code
					{
						// picture_header, let's see if it's an I-frame
						if (i<187)
						{
							// check if picture_coding_type == 1
							if ((buf[i+2] & (0x7 << 3)) == (1 << 3))
							{
								// found an I-frame picture
								doing_iframe = TRUE;
							}
						}
					}

					if (doing_iframe)
					{
						if (!foundfirst[curstream])
						{
							foundfirst[curstream] = TRUE;
							first_video_PCR = PCR;
						}
						break;
					}
				}
			}
			else if (pid == v->nRecordAudioPID[0] && !foundfirst[AUDIO] && foundfirst[VIDEO])  // Set audio found first ONLY after first video frame found..
				foundfirst[curstream] |= TRUE;

			// If we were skipping a bad packet, start fresh on this new PES packet..
			if (skipbad[curstream] == TRUE)
			{
				skipbad[curstream] = FALSE;
				packetpos[curstream] = 0;
			}
		}

		// Write a 2048 byte program stream packet..
		if (start && packetpos[curstream] > 0 && foundfirst[curstream] && !skipbad[curstream])
		{
			// Save the substream id block so we can added it to subsequent blocks
			unsigned char ac3_substream_id[4];
			int ac3len = 0;
			int written;
			int pos;

			if (pid == v->nRecordAudioPID[0])
			{
				int spos, dpos;
				int plen;
				int pstart;

				// Make sure we start with 0x0b77
				if (   (packetbuf[curstream][9 + packetbuf[curstream][8]] & audiosyncmask[0]) != audiosync[0]
					|| (packetbuf[curstream][9 + packetbuf[curstream][8] + 1] & audiosyncmask[1]) != audiosync[1])
				{
					spos = 9 + packetbuf[curstream][8];
					dpos = 9 + packetbuf[curstream][8];
					while (   spos <= packetpos[curstream] - 2 
						   && !((packetbuf[curstream][spos] & audiosyncmask[0]) == audiosync[0]
						   && (packetbuf[curstream][spos + 1] & audiosyncmask[1]) == audiosync[1]))
						spos++;

					if (!( (packetbuf[curstream][spos] & audiosyncmask[0]) == audiosync[0]
						&& (packetbuf[curstream][spos + 1] & audiosyncmask[1]) == audiosync[1]))
					{
						warning("Couldn't sync audio packet 1!");
						skipbad[curstream] = TRUE;
						continue;
					}

					while (spos < packetpos[curstream])
					{
						packetbuf[curstream][dpos] = packetbuf[curstream][spos];
						spos++;
						dpos++;
					}
					packetpos[curstream] = dpos;
				}

				// Check the next packet to make sure IT starts with a 0x0b77
				plen = 0;
				plen = 9 + buf[4 + adapt_len + 8];
				pstart = 4 + adapt_len + plen;
				if (   (buf[pstart] & audiosyncmask[0]) != audiosync[0] || (buf[pstart + 1] & audiosyncmask[1]) != audiosync[1])
				{
					spos = pstart;
					while (spos < 188 - 2 
						   && !((buf[spos] & audiosyncmask[0]) == audiosync[0]
						   && (buf[spos + 1] & audiosyncmask[1]) == audiosync[1]))
					{
						packetbuf[curstream][packetpos[curstream]] = buf[spos];
						packetpos[curstream]++;
						spos++;
					}

					if (!( (buf[spos] & audiosyncmask[0]) == audiosync[0]
						&& (buf[spos + 1] & audiosyncmask[1]) == audiosync[1]))
					{
						error("Couldn't sync audio packet 2!");
						skipbad[curstream] = TRUE;
						continue;
					}

					adapt_len = spos - 4 - plen;
					dpos = spos - 1;
					spos = pstart - 1;
					while (spos >= pstart - plen)
					{
						buf[dpos] = buf[spos];
						spos--;
						dpos--;
					}
				}

				// Make a four byte ac3 streamid
				switch(v->nRecordAudioType[0])
				{
				case 0x03:		// MPEG-1 audio
				case 0x04:		// MPEG-2 audio
					ac3len = 0;
					break;
				case 0x81:		// AC-3 audio
					ac3_substream_id[0] = 0x80;	// Four byte AC3 CODE??
					ac3_substream_id[1] = 0x01;
					ac3_substream_id[2] = 0x00;	// WHY???  OH WHY??
					ac3_substream_id[3] = 0x02;
					ac3len = 4;
					break;
				}
			}

			written = 0;	// Bytes we've written to output file
			pos = 0;		// Position in PES packet buffer
			for (;;)
			{
				DWORD dwWritten;
				int hdrsize;
				int i;
				__int64 packet_time;

				// Get total length of this pack
				int len = min(14 + ac3len + packetpos[curstream] - pos, PACKETSIZE);

				// Figure out stuffing (if we have less than 16 bytes left)
				BYTE stuffing = 0;
				if (len < PACKETSIZE && PACKETSIZE - len < 16)
				{
					stuffing = (BYTE)(PACKETSIZE - len);
					len += stuffing;
				}

				// Write out pack header
				packet_time = ((__int64)v->dTotalRecorded * CLOCKRATE / STREAMRATE);
				if (!write_pack(packet_time))
				{
					error("Couldn't write pack header!");
					return 1;
				}

				/* dbg_printf("packet_time = %d (0x%08x)\n", (int)packet_time, (int)packet_time); */

				if (pid == v->nRecordAudioPID[0])
					packetbuf[curstream][pos + 3] = streamid[AUDIO];
				else
					packetbuf[curstream][pos + 3] = streamid[VIDEO];

				// Packet length..
				// Subtract pack size (14) and pes id and len (6) from lenth
				packetbuf[curstream][pos + 4] = ((len - 6 - 14) >> 8) & 0xFF; 
				packetbuf[curstream][pos + 5] = (len - 6 - 14) & 0xFF;

				// Add any stuffing bytes to header extra len
				hdrsize = 9 + packetbuf[curstream][pos + 8];
				packetbuf[curstream][pos + 8] += stuffing;					// Add stuffing to header bytes

				// Write out id, sreamid, len
				if (!WriteFileAndUpdateCounters(v->hRecordFile, packetbuf[curstream] + pos, hdrsize, &dwWritten, NULL))	// Write pes id, streamid, and len
				{
					error("Failed to write output file!");
					return 1;
				}
				
				// Write stuffing
				for (i = 0; i < stuffing; i++)				// Write any stuffing bytes
				{
					unsigned char stuff = 0xff;
					if (!WriteFileAndUpdateCounters(v->hRecordFile, &stuff, 1, &dwWritten, NULL))
					{
						error("Failed to write output file!");
						return 1;
					}
				}

				// Write ac3 streamid
				if (ac3len != 0)
				{
					if (!WriteFileAndUpdateCounters(v->hRecordFile, ac3_substream_id, ac3len, &dwWritten, NULL))
					{
						error("Failed to write output file!");
						return 1;
					}
				}

				// Write rest of data len minus headersize (9) stuffing, and pack size (14)
				if (!WriteFileAndUpdateCounters(v->hRecordFile, packetbuf[curstream] + pos + hdrsize, len - hdrsize - 14 - stuffing - ac3len, &dwWritten, NULL))	// Write data bytes
				{
					error("Failed to write output file!");
					return 1;
				}
				written += len;

				// Add len minus stuff we added like the pack (14) and the stuffing.
				pos += len - 14 - stuffing - ac3len;
				if (pos == packetpos[curstream])
					break;

				// Add pes header for next packet
				pos -= 9;
				make_pes_header(packetbuf[curstream] + pos, (pid == v->nRecordVideoPID ? streamid[VIDEO] : streamid[AUDIO]), 0, -1, -1);
			}

			// Write padding
			if ((written % PACKETSIZE) != 0)
			{
				int left = PACKETSIZE - (written % PACKETSIZE);

				// Pad out to PACKETSIZE bytes
				if (!pad_buffer(left))
				{
					error("Couldn't write pad buffer!");
					return 1;
				}
			}

			packetpos[curstream] = 0;
		}

		// Add the payload for this packet to the current buffer
		if (foundfirst[curstream] && (184 - adapt_len) > 0)
		{
			memcpy(packetbuf[curstream] + packetpos[curstream], buf + 4 + adapt_len, 184 - adapt_len);
			packetpos[curstream] += 184 - adapt_len;
		}
	}

	return 0;
}


