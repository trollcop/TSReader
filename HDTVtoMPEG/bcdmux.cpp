// ***************************************************************************
// bcdemux.exe - Copyright (C) 2002 - Ben Cooley
//
// bcdemux demultiplexes an ATSC transport stream and converts it to an MPEG2
// DVD compatible video file.  It can select any program in the TS file, and 
// can automatically read a sequence of TS files and convert them to any number
// of MPG files.
//
// Changelog:
//
// 1.0 - 8-1-2002 -		First version
//
// 1.01 - 8-1-2002 -	Fixed a problem with some TS streams (ABC) not having the RANDOM bit in
//						the adaption header set.  Now just looks for the 'start' bit.  I think
//						this will get A-B frames at the beginning of a video, but it seems to 
//						work.
//
// 1.02 - 13-1-2002 -   Fixed problem again with some TS streams not having an adaption header on
//                      TS packets marked 'START'.  This was causing some of the streams not to 
//                      convert correctly.
//
// 1.03 - 16-1-2002 -	Fixed problem with AC3 audio stream from HBO streams.  AC3 packets from HBO
//						aren't aligned to the MPEG 0xBD packets.  Bit of code realigns them.
//
// 1.04 - 19-1-2002 -	FINALLY fixed problem with AC3 audio stream from HBO streams.  AC3 packets from HBO
//						aren't aligned to the MPEG 0xBD packets, and they don't start in TS start packets 
//						occasionally.  Solution is to just skip packet starts where there is no AC3 packet
//						sync.  Seems to work pretty well.
//
//						Fixed pack PTS timestamps so that the time displays in DVD players are accurate.
//						One minute is now really one minute.  There's still a problem with some DVD players
//						picking up the PTS in the video frames, but we'll save that for later.
//
//						Integrated this file with both the HDTVtoMPEG2 projects and the bcdmux projects. 
//						That way changes to this file automatically are reflected in both projects.
//
// 1.05 - 2-18-2002 -	Added channel detection, support for outputting files greater than 2GB, and better
//						error correction.  Also changed the program to start a new file after a selectable
//						size has been reached.
//
// 1.06 - 2-19-2002 -	Forced all files to be cut on I-Frame boundries.  Now files start on I-Frame's with
//						GOP (group of picture) packets.
//
// 1.07 - 2-23-2002 - GR: Recognise AccessDTV's strange sync byte mangling (0x47, 0x72, 0x29).
//						Packet alignment per file support (.adtv files not ts packet aligned).
//						Support for progress bar of total conversion.
//						Improved I-frame detection, detect if picture packets are I-frames in case GOPs are missing.
//
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <io.h>
#include <memory.h>

#include "bcdmux.h"

#define PACKETSIZE		(2048)						// MPEG Program stream block size
#define CLOCKRATE		((__int64)27000000)			// MPEG System clock rate
#define STREAMRATE		((__int64)2401587)			// Original HD stream rate 19.2 Mbps
#define FILE_TIME		(CLOCKRATE * (__int64)60)	// Time for each file
#define DEMUX			(((int)STREAMRATE * 8) / 50)// Demux value for HD content STREAMRATE / 50

#define RDBUFSIZE		(1024 * 1024 * 4)
#define WRBUFSIZE		(1024 * 1024 * 4)

#ifndef min
#define min(a,b) ((a)<(b)?(a):(b))
#define max(a,b) ((a)>(b)?(a):(b))
#endif

// ------------------------------------------------------------------------------------

#ifdef HDTVtoMPEG2

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

static void error(char* errorstr, char* extra = NULL)
{
	char buf[128];
	sprintf(buf, errorstr, extra);

	::MessageBox( NULL, buf, "Error", MB_ICONSTOP | MB_OK);
}

static void warning(char* errorstr, char* extra = NULL)
{
	char buf[128];
	sprintf(buf, errorstr, extra);

//	We'll add a log dialog box someday perhaps..
//	::MessageBox( NULL, buf, "Error", MB_ICONSTOP | MB_OK);
}

#else

static void error(char* errorstr, char* extra = NULL)
{
	printf("ERROR: ");
	printf(errorstr, extra);
}

static void warning(char* errorstr, char* extra = NULL)
{
	printf("WARNING: ");
	printf(errorstr, extra);
}

#endif

// ------------------------------------------------------------------------------------

struct FILE64
{
	int handle;
	bool read;
	unsigned char* buffer;
	__int64 filepos, filelen;
	int size, pos, len;
};

static FILE64* fopen64(const char* name, const char* mode)
{
	FILE64* f = new FILE64;
	
	// Just check first char..
	if (mode[0] == 'r')
	{
		f->handle = _open(name, _O_RDONLY | _O_SEQUENTIAL | _O_BINARY);
		if (f->handle < 0)
		{
			delete f;
			return NULL;
		}
		f->read = true;
		f->size = RDBUFSIZE;
		f->buffer = new unsigned char[f->size];

		_lseeki64(f->handle, 0, SEEK_END);
		f->filelen = _telli64(f->handle);
		_lseeki64(f->handle, 0, SEEK_SET);
	}
	else
	{
		f->handle = _open(name, _O_WRONLY | _O_CREAT | _O_TRUNC | _O_BINARY | _O_SEQUENTIAL, _S_IREAD | _S_IWRITE);
		if (f->handle < 0)
		{
			delete f;
			return NULL;
		}
		f->read = false;
		f->size = WRBUFSIZE;
		f->buffer = new unsigned char[f->size];
		f->filelen = 0;
	}

	f->filepos = 0;
	f->pos = f->len = 0;

	return f;
}

static void fillbuf(FILE64* f)
{
	f->filepos += f->len;
	f->len = _read(f->handle, f->buffer, f->size);
	f->pos = 0;
}

static bool flushbuf(FILE64* f)
{
	if (_write(f->handle, f->buffer, f->len) != f->len)
		return false;
	f->filepos += f->len;
	f->filelen += f->len;
	f->pos = f->len = 0;
	return true;
}

static __int64 ftell64(FILE64* f)
{
	return f->filepos + (__int64)f->pos;
}

static __int64 fseek64(FILE64* f, __int64 pos, int set)
{
	_lseeki64(f->handle, pos, SEEK_SET);
	f->filepos = pos;
	f->pos = f->len = 0;
	return pos;
}

static __int64 filelength64(FILE64* f)
{
	return f->filelen;
}

static int fread64(void* buf, int elsize, int elnum, FILE64* f)
{
	int size = elsize;
	if (elnum > 1)
		size *= elnum;
	
	int read = 0;

	if (size < f->len - f->pos)
	{
		memcpy(buf, f->buffer + f->pos, size);
		f->pos += size;
		read = size;
	}
	else
	{
		memcpy(buf, f->buffer + f->pos, f->len - f->pos);
		read += f->len - f->pos;

		fillbuf(f);

		if (f->len > size - read)
		{
			memcpy((unsigned char*)buf + read, f->buffer, size - read);
			f->pos += size - read;
			read = size;
		}
		else
		{
			memcpy((unsigned char*)buf + read, f->buffer, f->len);
			f->pos += f->len;
			read += f->len;
		}
	}

	if (elnum == 1 && read == size)
		return 1;
	else if (elsize == 1)
		return read;
	else
		return read / elsize;
}

static int fwrite64(void* buf, int elsize, int elnum, FILE64* f)
{
	int size = elsize;
	if (elnum > 1)
		size *= elnum;
	
	int written = 0;

	if (size <= f->size - f->pos)
	{
		memcpy(f->buffer + f->pos, buf, size);
		f->pos += size;
		f->len = f->pos;
		written = size;
	}
	else
	{
		memcpy(f->buffer + f->pos, buf, f->size - f->pos);
		written += f->size - f->pos;
		f->pos += f->size - f->pos;
		f->len = f->pos;

		if (flushbuf(f))
		{
			memcpy(f->buffer, (unsigned char*)buf + written, size - written);
			f->pos += size - written;
			f->len = f->pos;
			written += size - written;
		}
	}


	if (elnum == 1 && written == size)
		return 1;
	else
		return written / elsize;
}

static void fclose64(FILE64* f)
{
	if (!f->read)
		flushbuf(f);
	_close(f->handle);
	delete f->buffer;
	delete f;
}



// ------------------------------------------------------------------------------------

int bitpos = 0;
unsigned int bitval = 0;
unsigned char* bitbuf = NULL;
unsigned int bitmask[] = {
	0x0,0x1,0x3,0x7,0xf,0x1f,0x3f,0x7f,0xff,
	0x1ff,0x3ff,0x7ff,0xfff,0x1fff,0x3fff,0x7fff,0xffff,
	0x1ffff,0x3ffff,0x7ffff,0xfffff,0x1fffff,0x3fffff,0x7fffff,0xffffff,
	0x1ffffff,0x3ffffff,0x7ffffff,0xfffffff,0x1fffffff,0x3fffffff,0x7fffffff,0xffffffff};

static inline void set_buf(unsigned char* buf, int bufsize, bool clear = true)
{
	bitpos = 0;
	bitbuf = buf;
	bitval = (bitbuf[0] << 24) | (bitbuf[1] << 16) | (bitbuf[2] << 8) | bitbuf[3];
	if (clear)
		memset(bitbuf, 0, bufsize);
}

static inline int buf_size()
{
	return bitpos >> 3;
}

static inline void set_bits(unsigned int val, int bits)
{
	val &= bitmask[bits];

	while (bits > 0)
	{
		int bitsleft = (8 - (bitpos & 7));
		if (bits >= bitsleft)
		{
			bitbuf[bitpos >> 3] |= val >> (bits - bitsleft);
			bitpos += bitsleft;
			bits -= bitsleft;
			val &= bitmask[bits];
		}
		else
		{
			bitbuf[bitpos >> 3] |= val << (bitsleft - bits);
			bitpos += bits;
			bits = 0;
		}
	}
}

static inline unsigned int get_bit_pos()
{
	return bitpos;
}

static inline unsigned int get_byte_pos()
{
	return bitpos >> 3;
}

static inline void set_bit_pos(unsigned int newpos)
{
	bitpos = newpos;

	unsigned int pos = bitpos >> 5;
	bitval = (bitbuf[pos] << 24) | (bitbuf[pos + 1] << 16) | (bitbuf[pos + 2] << 8) | bitbuf[pos + 3];
}

static inline void set_byte_pos(unsigned int newpos)
{
	set_bit_pos(newpos << 3);
}

static inline unsigned int get_bits(int bits)
{
	unsigned int val;
	int left = 32 - (bitpos & 31);

	if (bits < left)
	{
		val = (bitval >> (left - bits)) & bitmask[bits];
		bitpos += bits;
	}
	else
	{
		val = (bitval & bitmask[left]) << (bits - left);
		bitpos += left;
		bits -= left;

		int pos = bitpos >> 3;
		bitval = (bitbuf[pos] << 24) | (bitbuf[pos + 1] << 16) | (bitbuf[pos + 2] << 8) | bitbuf[pos + 3];
		
		if (bits > 0)
		{
			val |= (bitval >> (32 - bits)) & bitmask[bits];
			bitpos += bits;
		}
	}

	return val;
}

// ------------------------------------------------------------------------------------

__int64 align_to_next_packet(FILE64* f)
{
	unsigned char buf[188*20];

	__int64 start = ftell64(f);
	__int64 pos = 0;

	if (fread64(buf, 188*20, 1, f) == 1)
	{
		bool found = false;
		while (!found && (pos < 188))
		{
			found = true;
			for (int i = 0; i < 188*20; i += 188)
			{
				unsigned char c = buf[pos+i];
				// Check sync byte
				if ((c != 0x47) && (c != 0x72) && (c != 0x29))
				{
					// this offset failed, try next
					found = false;
					pos++;
					break;
				}
			}
		}
	}

	if (pos == 188)
		pos = 0;		// failed to find anything!!!!!?

	fseek64(f, start+pos, SEEK_SET);

	return pos;
}

// ------------------------------------------------------------------------------------
/*
bool scan_channel_info(const char* filename, int maxscan, int maxchannels, ATSC_CHANNEL_INFO* channels, int& numchannels)
{
	numchannels = 0;

	memset(channels, 0, sizeof(ATSC_CHANNEL_INFO) * maxchannels);

	FILE64* f = fopen64(filename, "rb");
	if (!f)
	{
		warning("Couldn't open file for channel scan!\n");
		return false;
	}

	// align to first packet
	align_to_next_packet(f);

	unsigned char buf[188];

	unsigned char tablebuf[1024];
	unsigned int tablepos = 0;
	
	bool reading = false;

	while (fread64(buf, 188, 1, f) == 1 && ftell64(f) < maxscan)
	{
		// Check sync byte
		if ((buf[0] != 0x47) && (buf[0] != 0x72) && (buf[0] != 0x29))
			continue;

		// Get error
		bool errorbit = (buf[1] & 0x80) != 0;
		if (errorbit)
			continue;

		// Get pid
		int pid = (((buf[1] & 0x1F) << 8) | buf[2]) & 0x1FFF;

		// Skip this block
		if (pid < 0x1000 || pid >= 0x1FFF)
			continue;

		// Get adaption header info
		int adapt_len = 0;
		int adaption = (buf[3] & 0x30) >> 4;
		if (adaption == 0)
			continue;
		else if (adaption == 0x2)
			adapt_len = 184;
		else if (adaption == 0x3)
			adapt_len = buf[4] + 1;
		if (adapt_len > 184)
			continue;

		// Get pointer length
		int pointer_len = buf[4 + adapt_len] + 1;

		// Get payload start indicator
		bool start;
		start = (buf[1] & 0x40) != 0;

		if (start && reading)
		{
			memcpy(tablebuf + tablepos, buf + 4 + adapt_len + 1, pointer_len - 1);

			unsigned int pos = 0;
			//while (pos < tablepos)
			{
				set_buf(tablebuf + pos, tablepos - pos, false);

				unsigned char section_id	= get_bits(8);
											  get_bits(4);
				unsigned int section_len	= get_bits(12);
				unsigned int transport_id	= get_bits(16);
											  get_bits(2);
				unsigned int version_num	= get_bits(5);
				unsigned int current_next	= get_bits(1);
				unsigned int section_num	= get_bits(8);
				unsigned int last_section	= get_bits(8);
				unsigned int protocol_ver	= get_bits(8);

				switch (section_id)
				{
				  case 0xC7:
					{
						break;
					}
				  case 0xC8:
					{
						int num_ch = get_bits(8);

						for (int i = 0; i < num_ch; i++)
						{
							char *name = channels[numchannels].name;
							name[0] =								(char)get_bits(16);
							name[1] =								(char)get_bits(16);
							name[2] =								(char)get_bits(16);
							name[3] =								(char)get_bits(16);
							name[4] =								(char)get_bits(16);
							name[5] =								(char)get_bits(16);
							name[6] =								(char)get_bits(16);
							name[7] =								'\0';
							
																	get_bits(4);
							channels[numchannels].major_channel =	get_bits(10);
							channels[numchannels].minor_channel =	get_bits(10);

							unsigned int modulation =				get_bits(8);
							unsigned int carrier =					get_bits(32);
							unsigned int channel_TSID =				get_bits(16);
							unsigned int program_number =			get_bits(16);
											
							// Ignore these bits for now //			get_bits(32);	// Hidden, encrypted, etc. bits

																	get_bits(6);

							unsigned int ch_desclen	=				get_bits(10);

							unsigned int pos = 0;
							while (pos < ch_desclen)
							{
								unsigned int desc_id =				get_bits(8);
								unsigned int desc_len =				get_bits(8);

								switch (desc_id)
								{
								  case 0xA1:	// Stream descriptors
									{
																	get_bits(16); // Ignore PCR_PID 
										int num_elem =				get_bits(8);
										for (int j = 0; j < num_elem; j++)
										{
											unsigned int stream_type =	get_bits(8);
																		get_bits(3);
											unsigned int pid =			get_bits(13);
											unsigned int language =		get_bits(24);
											
											switch (stream_type)	// MPEG-2 video..
											{
											  case 0x02:
												{
													channels[numchannels].videopid = pid;
													break;
												}
											  case 0x81:
												{
													if (channels[numchannels].audiopid != 0 && numchannels + 1 < maxchannels)
													{
														memcpy(channels + numchannels + 1, channels + numchannels, sizeof(ATSC_CHANNEL_INFO));
														numchannels++;
													}
													channels[numchannels].audiopid = pid;
													break;
												}
											}
										}
										break;
									}
								}

								pos += desc_len + 2;
							}

							if (modulation == 4)	// Only if ATSC digital channel.. Ignore ANALOG channels.
								numchannels++;
						}

						fclose64(f);

						return true;
					}
				}

				pos += 3 + section_len;
			}

			tablepos = 0;
		}
		else if (start)
			reading = true;
	
		// Add the payload for this packet to the current buffer
		if (reading && (184 - adapt_len) > 0)
		{
			if (tablepos + 184 - adapt_len - pointer_len > 1024)
			{
				warning("Bad program section length (> 1024)\n");
				continue;
			}
			memcpy(tablebuf + tablepos, buf + 4 + adapt_len + pointer_len, 184 - adapt_len - pointer_len);
			tablepos += 184 - adapt_len - pointer_len;
		}
	}	

	fclose64(f);

	return true;
}
*/
// ------------------------------------------------------------------------------------

static void set_pts(unsigned char* buf, __int64 PTS)
{
	set_buf(buf, 5);

	set_bits(2, 4);								// '0010'							4
	set_bits((unsigned int)(PTS >> 30), 3);		// PTS [32..30]						3
	set_bits(1, 1);								// marker bit						1
	set_bits((unsigned int)(PTS >> 15), 15);	// PTS [29..15]						15
	set_bits(1, 1);								// marker bit						1
	set_bits((unsigned int)PTS, 15);			// PTS [14..0]						15
	set_bits(1, 1);								// marker bit						1
}

static bool write_pack(FILE64* fout, __int64 time)
{
	unsigned char buf[64];
	set_buf(buf, 64);

	__int64 ext_time = time % 300;
	time = time / 300;

	set_bits(0x000001ba, 32);					// pack id								32
	set_bits(1, 2);								// 0x01									2
	set_bits((unsigned int)(time >> 30), 3);	// system_clock_reference_base			3
	set_bits(1, 1);								// marker_bit							1
	set_bits((unsigned int)(time >> 15), 15);	// system_clock_reference_base			15
	set_bits(1, 1);								// marker_bit							1
	set_bits((unsigned int)time, 15);			// system_clock_reference_base1			15
	set_bits(1, 1);								// marker_bit							1
	set_bits((unsigned int)ext_time, 9);		// system_clock_reference_extension		9
	set_bits(1, 1);								// marker_bit							1
	set_bits(DEMUX, 22);						// program_mux_rate						22
	set_bits(1, 1);								// marker_bit							1
	set_bits(1, 1);								// marker_bit							1
	set_bits(31, 5);							// reserved								5
	set_bits(0, 3);								// pack_stuffing_length					3

	return fwrite64(buf, buf_size(), 1, fout) == 1;
}

static bool write_system(FILE64* fout, 
	int videostream, int vidbscl, int vidbsize,
	int audiostream, int audbscl, int audbsize)
{
	unsigned char buf[64];
	set_buf(buf, 64);

	int len = 6;
	if (videostream != 0)
		len += 3;
	if (audiostream != 0)
		len += 3;

	set_bits(0x000001bb, 32);					// system id							32
	set_bits(len, 16);							// header_length						16
	set_bits(1, 1);								// marker_bit							1
	set_bits(DEMUX, 22);						// rate_bound							22
	set_bits(1, 1);								// marker_bit							1
	set_bits(1, 6);								// audio_bound							6
	set_bits(0, 1);								// fixed_flag							1
	set_bits(0, 1);								// CSPS_flag							1
	set_bits(1, 1);								// system_audio_lock_flag				1
	set_bits(1, 1);								// system_video_lock_flag				1
	set_bits(1, 1);								// marker_bit							1
	set_bits(1, 5);								// video_bound							5
	set_bits(0, 1);								// packet_rate_restriction_flag			1
	set_bits(127, 7);							// reserved_byte						7

	if (videostream != 0)
	{
		set_bits(videostream, 8);
		set_bits(0x3, 2);
		set_bits(vidbscl, 1);
		set_bits(vidbsize, 13);
	}

	if (audiostream != 0)
	{
		set_bits(audiostream, 8);
		set_bits(0x3, 2);
		set_bits(audbscl, 1);
		set_bits(audbsize, 13);
	}

	return fwrite64(buf, buf_size(), 1, fout) == 1;
}

static bool pad_buffer(FILE64* fout, int pad)
{
	pad -= 6;

	char buf[6];
	buf[0] = '\x0'; buf[1] = '\x0'; buf[2] = '\x1'; buf[3] = '\xbe';
	buf[4] = pad >> 8; buf[5] = pad & 0xff;

	if (fwrite64(buf, 6, 1, fout) != 1)
		return false;

	unsigned char padbyte = 0xff;
	for (int i = 0; i < pad; i++)
	{
		if (fwrite64(&padbyte, 1, 1, fout) != 1)
			return false;
	}

	return true;
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

	set_buf(buf, 9 + hdrlen);

	set_bits(0x000001, 24);						// packet_start_code_prefix				24
	set_bits((unsigned int)streamid, 8);		// directory_stream_id					8
	set_bits(len, 16);							// PES_packet_length					16
	set_bits(0x2, 2);							// '10'									2
	set_bits(0, 2);								// PES_scrambling_control				2
	set_bits(1, 1);								// PES_priority							1
	set_bits(0, 1);								// data_alignment_indicator				1
	set_bits(0, 1);								// copyright							1
	set_bits(0, 1);								// original_or_copy						1
	set_bits(PTS_DTS_flags, 2);					// PTS_DTS_flags						2
	set_bits(0, 1);								// ESCR_flag							1
	set_bits(0, 1);								// ES_rate_flag							1
	set_bits(0, 1);								// DSM_trick_mode_flag					1
	set_bits(0, 1);								// additional_copy_info_flag			1
	set_bits(0, 1);								// PES_CRC_flag							1
	set_bits(0, 1);								// PES_extension_flag					1
	set_bits(hdrlen, 8);						// PES_header_data_length				8
	
	if (PTS_DTS_flags == 2)
	{
		set_bits(2, 4);								// '0010'							4
		set_bits((unsigned int)(PTS >> 30), 3);		// PTS [32..30]						3
		set_bits(1, 1);								// marker bit						1
		set_bits((unsigned int)(PTS >> 15), 15);	// PTS [29..15]						15
		set_bits(1, 1);								// marker bit						1
		set_bits((unsigned int)PTS, 15);			// PTS [14..0]						15
		set_bits(1, 1);								// marker bit						1
	}
	else if (PTS_DTS_flags == 3)
	{
		set_bits(3, 4);								// '0011'							4
		set_bits((unsigned int)(PTS >> 30), 3);		// PTS [32..30]						3
		set_bits(1, 1);								// marker bit						1
		set_bits((unsigned int)(PTS >> 15), 15);	// PTS [29..15]						15
		set_bits(1, 1);								// marker bit						1
		set_bits((unsigned int)PTS, 15);			// PTS [14..0]						15
		set_bits(1, 1);								// marker bit						1
		set_bits(1, 4);								// '0001'							4
		set_bits((unsigned int)(DTS >> 30), 3);		// DTS [32..30]						3
		set_bits(1, 1);								// marker bit						1
		set_bits((unsigned int)(DTS >> 15), 15);	// DTS [29..15]						15
		set_bits(1, 1);								// marker bit						1
		set_bits((unsigned int)DTS, 15);			// DTS [14..0]						15
		set_bits(1, 1);								// marker bit						1
	}

	return buf_size();
}

static bool write_file_header(FILE64* fout, int vidstreamid, int audstreamid, int vidpid, int audpid)
{ 
	// Write the first pack header
	if (!write_pack(fout, 0))
	{
		error("Couldn't write system pack header!\n");
		return false;
	}

	// Write the system block
	if (audpid != -1)
	{
		if (!write_system(fout, vidstreamid, 1, 4096, audstreamid, 0, 58))	// 4096k for video packets 58 for audio??
		{
			error("Couldn't write system header!\n");
			return false;
		}
	}
	else
	{
		if (!write_system(fout, vidstreamid, 1, 4096, 0, 0, 0))		// 4096k for video packets
		{
			error("Couldn't write system header!\n");
			return false;
		}
	}

	// Pad out to even multiple of PACKETSIZE
	int beg_len = (int)ftell64(fout);
	if (!pad_buffer(fout, PACKETSIZE - beg_len))
	{
		error("Couldn't write pad buffer!\n");
		return false;
	}

	return true;
}

static int get_hex(char* str)
{
	if (strncmp(str, "0x", 2) != 0)
		return atol(str);

	int v = 0;	
	char *p = str + 2;
	while (*p && *p != ' ')
	{
		if (*p >= '0' && *p <= '9')
			v = (v << 4) | (*p - '0');
		else if (*p >= 'a' && *p <= 'f')
			v = (v << 4) | ((*p - 'a') + 10);
		else if (*p >= 'A' && *p <= 'F')
			v = (v << 4) | ((*p - 'A') + 10);
		p++;
	}

	return v;
}

static bool inc_file_counter(char* filename)
{
	char *p = filename + strlen(filename) - 1;
	bool found_digits = false;
	while (p != filename)
	{
		if (found_digits && (*p < '0' || *p > '9'))
			break;
		else if (*p >= '0' && *p <= '9')
		{
			found_digits |= true;
			if (*p == '9')
				*p = '0';
			else
			{
				*p += 1;
				break;
			}
		}
		p--;
	}
	return found_digits;
}

// ------------------------------------------------------------------------------------

typedef bool (*convert_callback)(int ifilenum, char* ifilename, char* ofilename,
	__int64 curfilesize, __int64 curfilepos, __int64 curtotalsize, __int64 curtotalpos, void* data);

#define curbuf (packetbuf[curstream])
#define curpos (packetpos[curstream])
#define curpackets (bufpackets[curstream])
#define curfoundfirst (foundfirst[curstream])
#define curskipbad (skipbad[curstream])
#define curcontinuity (streamcont[curstream])
#define curstreamid (streamid[curstream])

#define vidbuf (packetbuf[0])
#define vidpos (packetpos[0])
#define vidpackets (bufpackets[0])
#define vidfoundfirst (foundfirst[0])
#define vidskipbad (skipbad[0])
#define vidcontinuity (streamcont[0])
#define vidstreamid (streamid[0])

#define audbuf (packetbuf[1])
#define audpos (packetpos[1])
#define audpackets (bufpackets[1])
#define audfoundfirst (foundfirst[1])
#define audskipbad (skipbad[1])
#define audcontinuity (streamcont[1])
#define audstreamid (streamid[1])

int convert_files(char* ifilelist[], int numifiles, char* ofilepattern, 
		int vidpid, int audpid, int maxfilesize, convert_callback callback, void* data)
{
	__int64 curfilesize = 0;
	__int64 curfilepos = 0;
	__int64 maxfilesize64 = (__int64)maxfilesize * (__int64)(1024 * 1024);
	bool cont = true;
	int tspackets = 0;

	// Packet arrays
	unsigned char* packetbuf[2];
	int packetpos[2];
	int bufpackets[2];
	bool foundfirst[2];
	bool skipbad[2];
	int streamcont[2];
	int streamid[2];

	// Open the first source file
	char ifilename[256];
	strcpy(ifilename, ifilelist[0]);
	FILE64 *fin = fopen64(ifilename, "rb");
	if (!fin)
	{
		error("Couldn't open source file %s!\n", ifilename);
		return 1;
	}
	curfilesize = filelength64(fin);		// These files are less than 1GB I think..
	int curifile = 0;

	// align to first packet
	curfilepos = align_to_next_packet(fin);

	// Get the video PID
	vidbuf = new unsigned char[1024 * 1024];
	vidpos = 0;
	vidfoundfirst = 0;
	vidskipbad = false;
	vidcontinuity = -1;
	vidpackets = 0;
	vidstreamid = 0xE0;	// MPEG2 video stream id

	// Get the audio PID
	if (audpid <= 0)
		audpid = -1;
	audbuf = new unsigned char[1024 * 1024];
	audpos = 0;
	audfoundfirst = false;
	audskipbad = false;
	audcontinuity = -1;
	audpackets = 0;
	audstreamid = 0xBD;	// AC3, etc. streamid

	// Open the destination file
	char ofilename[256];
	strcpy(ofilename, ofilepattern);
	FILE64 *fout = fopen64(ofilename, "wb");
	if (!fout)
	{
		error("Couldn't open dest file %s!\n", ofilename);
		return 1;
	}
	
	// Setup num files from max files
	int numfiles = 1;
	
	// Set the starting time value
	__int64 packet_time = 0;
	__int64 file_time = 0;
	__int64 PCR = -1;
	__int64 first_video_PCR;
	bool doing_iframe;
	int frame = 0;

	// Write the first pack header
	if (!write_file_header(fout, vidstreamid, audstreamid, vidpid, audpid))
		return 1;
	
#ifndef HDTVtoMPEG2
	// Write out 'processing'..
	printf("Converting file %s to file %s\n"
		"Video stream 0x%x Audio stream 0x%x\n"
		"Please wait", ifilename, ofilename, vidpid, audpid);
#endif

	unsigned char buf[188];
	int curstream = 0;

	// Here's the deal.  We simply read each packet in the transport file, decide if it is one of the PID's
	// we're interested in, detect the 'start' bit and 'random access' bit in the header, then once we find
	// one, simply add the payloads of all subsequent packets with that id into a buffer.  Once we get the
	// next packet for a PID with a 'start bit' for either stream, we dump the previous packet, to the 
	// MPEG2 program file with a pack header, 2048 bytes at a time, then pad the final pack to 2048 bytes.

	// We have to get the PCR timestamps from the transport stream and use them to set the timestamps for
	// the pack headers in the MPEG stream.  We just do a one to one copy when we find one, and use the 
	// number of bytes read times the demux rate to do any pack header that doesn't have a timestamp.

	// As for AC3, we have to add that little 4 byte substreamid, AC3 id value before the AC3 payload for
	// each MPEG2 AC3 2048 byte packet.

	// For demux rate, we just use the ATSC 19.2Mbps rate, since that's pretty close to what the file is.
	// For the decode buffer sizes, we err on the side of caution.  Perhaps they're too big, but I'm not
	// sure.

	// Finally, once we get to the end of the file, we add the MPEG stop packet id, and we're done.
	// Whahla.. we have a nice DVD compatible super high definition standard MPEG video file, all set for
	// compression into DivX or what-have-you.

	int bytesReadInPacket = 0;
	for (;;)
	{
		// Call callback
		if (((tspackets) & 0x3f) == 0 && callback)
		{
			// quick hacked total calculations. ideally sum up file sizes of all inputs
			__int64 curtotalsize, curtotalpos;
			if (numifiles > 0)
			{
				curtotalsize = (__int64)numifiles * curfilesize;
				curtotalpos = (__int64)curifile * curfilesize + curfilepos;
			}
			else
			{
				curtotalsize = curfilesize;
				curtotalpos = curfilepos;
			}
			cont = callback(curifile, ifilename, ofilename, curfilesize, curfilepos, curtotalsize, curtotalpos, data);
		}

		tspackets++;
		
		// Cancelled?
		if (!cont)
		{
			fclose64(fin);
			break;
		}
		
		// Try to read packet..
		int bytesRead;
		if ((bytesRead = fread64(buf+bytesReadInPacket, 1, 188-bytesReadInPacket, fin)) != 188-bytesReadInPacket)
		{
			if (bytesRead < 0)
				bytesRead = 0;
			bytesReadInPacket += bytesRead;

			// Close this file
			fclose64(fin);

			// Open the next file in the sequence..
			curifile++;
			if (numifiles == -1)	// Indicates we're going to just increment the file num
			{
				if (inc_file_counter(ifilename))
					fin = fopen64(ifilename, "rb");
				else
					fin = NULL;
			}
			else					// Get next file in input list
			{
				if (curifile >= numifiles)
					break;
				strcpy(ifilename, ifilelist[curifile]);
				fin = fopen64(ifilename, "rb");
			}
			if (!fin)
				break;	// No more files??  GIVE UP!
			curfilesize = filelength64(fin);		// These files are less than 1GB I think..
			file_time += FILE_TIME;
			numfiles++;

			// check packet alignment of new file
			curfilepos = align_to_next_packet(fin);
			if (curfilepos == ((188-bytesReadInPacket)%188))
			{
				// alignment matches, allow to finish reading partial packet
				curfilepos = 0;
				fseek64(fin, curfilepos, SEEK_SET);
			}
			else
			{
				// alignment does not match, discard partial packet
				bytesReadInPacket = 0;
			}

#ifndef HDTVtoMPEG2
			printf("\nReading input file %s\n", ifilename);
			printf("Please Wait");
#endif
			continue;
		}
		else
		{
			curfilepos += bytesRead;
			bytesReadInPacket = 0;
		}

		// Check max files (Only cut files on I-Frames -- i.e. video packets containing Group of Picture id)
		if (ftell64(fout) >= maxfilesize64 && doing_iframe)
		{
			// Mark last valid stream pos
			unsigned char end[4] = {0,0,1,0xb9};
			fwrite64(&end, 4, 1, fout);

			// Close files
			fclose64(fout);

			// Try next file?
			if (inc_file_counter(ofilename))
			{
				// Open the destination file
				fout = fopen64(ofilename, "wb");
				if (!fout)
				{
					error("Couldn't open dest file %s!\n", ofilename);
					return 1;
				}

				// Set the starting time value
				PCR = -1;
				frame = 0;
				file_time = 0;

				// Write the first pack header
				if (!write_file_header(fout, vidstreamid, audstreamid, vidpid, audpid))
						return 1;

#ifndef HDTVtoMPEG2
				printf("\nWriting next output file %s\n", ofilename);
				printf("Please Wait");
#endif
			}
			else
				break;
		}

		// Check sync byte
		if ((buf[0] != 0x47) && (buf[0] != 0x72) && (buf[0] != 0x29))
		{
			__int64 pos = ftell64(fin);
			warning("Bad transport packet (no sync byte 0x47)!\n");
			audskipbad = vidskipbad = true;
			continue;
		}

		// Get pid
		int pid = (((buf[1] & 0x1F) << 8) | buf[2]) & 0x1FFF;

		// Skip this block
		if (pid != audpid && pid != vidpid)
			continue;

		// Get the pos and buf
		if (pid == vidpid)
			curstream = 0;
		else
			curstream = 1;

		// Get start code
		bool start;
		start = (buf[1] & 0x40) != 0;
		if (!start && curskipbad)
			continue;

		// Get error
		bool errorbit = (buf[1] & 0x80) != 0;
		if (errorbit)
		{
			warning("Error bit set in packet\n");
			curskipbad = true;
			continue;
		}

		// Get continuity
		int continuity = (buf[3] & 0xF);
		if (curcontinuity != -1)
		{
			if (continuity != ((curcontinuity + 1) & 0xF))
			{
				warning("Bad continuity code in packet\n");
				curskipbad = true;
				continue;
			}
			curcontinuity = continuity;
		}
			
		// Get adaption header info
		int adaption = (buf[3] & 0x30) >> 4;
		int adapt_len = 0;

		// Get adaption header size
		if (adaption == 0)
		{
			warning("Bad adaption code (code was 0)!\n");
			audskipbad = vidskipbad = true;
			continue;
		}
		else if (adaption == 0x2)
			adapt_len = 184;
		else if (adaption == 0x3)
		{
			adapt_len = buf[4] + 1;
			if (adapt_len > 184)
			{
				warning("Invalid adapt len (was > 183)!\n");
				audskipbad = vidskipbad = true;
			}
		}

		// HBO is slick, it doesn't bother to sync AC3 packets with PES elementary stream packets.. so
		// we have to swizzle them together!  (ARGHH!)
		if (pid == audpid && start)
		{
			// Is there an AC3 packet start 0b77 code in this packet??
			bool sync_found = false;
			unsigned char *p = buf + 4 + adapt_len;
			while (p <= buf + 186)
			{
				if (p[0] == 0x0b && p[1] == 0x77)
				{
					sync_found = true;
					break;
				}
				p++;
			}

			// Couldn't find an AC3 sync start in this packet.. don't make a PES packet!
			if (!sync_found)
			{
//					int pos = ftell(fin);
//					error("AC3 packet sync not found in start frame");
//					return 1;
				adapt_len += 9 + buf[4 + adapt_len + 8];	
				start = false;
			}
		}

		// Get PCR
		if (start && (adaption & 0x2) && (buf[5] & 0x10))
		{
			__int64 PCR_base = ((__int64)buf[6] << 25) | ((__int64)buf[7] << 17) | 
				  ((__int64)buf[8] << 9) | ((__int64)buf[9] << 1) | ((__int64)buf[10] >> 7);
			__int64 PCR_ext = ((__int64)(buf[10] & 0x1) << 8) | ((__int64)buf[11]);
			PCR = PCR_base * 300 + PCR_ext;
		}

		// Get random
		bool random = start;	// Oops.. random bit seems to be used only from some broadcasters???
//		bool random = false;
//		if (start && (adaption & 0x2))
//			random = (buf[5] & 0x40) != 0;		// BUG: SOME TS STREAMS DON'T HAVE THE RANDOM BIT (ABC!! ALIAS)

		// Found a random access point (now we can start a frame/audio packet..)
		if (random)
		{
			// Check to see if this is an i_frame (group of picture start)
			if (pid == vidpid)
			{
				// Look for the Group of Pictures packet.. indicates this is an I-Frame packet..
				doing_iframe = false;
				unsigned int strid = 0;
				for (int i = 4 + adapt_len; i < 188; i++)
				{
					strid = (strid << 8) | buf[i];
					if (strid == 0x000001B8) // group_start_code
					{
						// found a Group of Pictures header, subsequent picture must be an I-frame
						doing_iframe = true;
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
								doing_iframe = true;
							}
						}
					}

					if (doing_iframe)
					{
						if (!curfoundfirst)
						{
							curfoundfirst = true;
							first_video_PCR = PCR;
						}
						break;
					}
				}
			}
			else if (pid == audpid && !audfoundfirst && vidfoundfirst)  // Set audio found first ONLY after first video frame found..
				curfoundfirst |= true;

			// If we were skipping a bad packet, start fresh on this new PES packet..
			if (curskipbad == true)
			{
				curskipbad = false;
				curpos = 0;
			}

			// Get the continuity code of this packet
			curcontinuity = continuity;
		}

		// Write a 2048 byte program stream packet..
		if (start && curpos > 0 && curfoundfirst && !curskipbad)
		{
			// Save the substream id block so we can added it to subsequent blocks
			unsigned char ac3_substream_id[4];
			int ac3len = 0;
			if (pid == audpid && audstreamid == 0xBD)
			{
				int spos, dpos;

				// Make sure we start with 0x0b77
				if (curbuf[9 + curbuf[8]] != 0x0b || curbuf[9 + curbuf[8] + 1] != 0x77)
				{
					spos = 9 + curbuf[8];
					dpos = 9 + curbuf[8];
					while (spos <= curpos - 2 && !(curbuf[spos] == 0x0b && curbuf[spos + 1] == 0x77))
						spos++;

					if (!(curbuf[spos] == 0x0b && curbuf[spos + 1] == 0x77))
					{
						warning("Couldn't sync AC3 packet!\n");
						curskipbad = true;
						continue;
					}

					while (spos < curpos)
					{
						curbuf[dpos] = curbuf[spos];
						spos++;
						dpos++;
					}
					curpos = dpos;
				}

				// Check the next packet to make sure IT starts with a 0x0b77
				int plen = 0;
//					if (buf[4 + adapt_len] == 0 && buf[4 + adapt_len + 1] == 0 &&		// Starting with an mpeg header?
//						buf[4 + adapt_len + 2] == 1 && buf[4 + adapt_len + 3] == 0xBD)
						plen = 9 + buf[4 + adapt_len + 8];
				int pstart = 4 + adapt_len + plen;
				if (buf[pstart] != 0x0b || buf[pstart + 1] != 0x77)
				{
					spos = pstart;
					while (spos < 188 - 2 && !(buf[spos] == 0x0b && buf[spos + 1] == 0x77))
					{
						curbuf[curpos] = buf[spos];
						curpos++;
						spos++;
					}

					if (!(buf[spos] == 0x0b && buf[spos + 1] == 0x77))
					{
						error("Couldn't sync AC3 packet!\n");
						curskipbad = true;
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
				ac3_substream_id[0] = 0x80;	// Four byte AC3 CODE??
				ac3_substream_id[1] = 0x01;
				ac3_substream_id[2] = 0x00;	// WHY???  OH WHY??
				ac3_substream_id[3] = 0x02;
				ac3len = 4;
			}

			int written = 0;	// Bytes we've written to output file
			int pos = 0;		// Position in PES packet buffer
			for (;;)
			{
				__int64 fpos = ftell64(fout);
				if ((fpos % (__int64)PACKETSIZE) != 0)
				{
					error("Packet's not falling on packetsize boundries!\n");
					return 1;
				}

				// Get total length of this pack
				int len = min(14 + ac3len + curpos - pos, PACKETSIZE);

				// Figure out stuffing (if we have less than 16 bytes left)
				int stuffing = 0;
				if (len < PACKETSIZE && PACKETSIZE - len < 16)
				{
					stuffing = PACKETSIZE - len;
					len += stuffing;
				}

				// Write out pack header
				packet_time = (ftell64(fin) * CLOCKRATE / STREAMRATE) + file_time;
				if (!write_pack(fout, packet_time))
				{
					error("Couldn't write pack header!\n");
					return 1;
				}

				if (pid == audpid)
					curbuf[pos + 3] = audstreamid;
				else
					curbuf[pos + 3] = vidstreamid;

				// Packet length..
				// Subtract pack size (14) and pes id and len (6) from lenth
				curbuf[pos + 4] = (len - 6 - 14) >> 8; curbuf[pos + 5] = (len - 6 - 14) & 0xFF;

				// Add any stuffing bytes to header extra len
				int hdrsize = 9 + curbuf[pos + 8];
				curbuf[pos + 8] += stuffing;					// Add stuffing to header bytes

				// Write out id, sreamid, len
				if (fwrite64(curbuf + pos, hdrsize, 1, fout) != 1)	// Write pes id, streamid, and len
				{
					error("Failed to write output file!\n");
					return 1;
				}
				
				// Write stuffing
				for (int i = 0; i < stuffing; i++)				// Write any stuffing bytes
				{
					unsigned char stuff = 0xff;
					if (fwrite64(&stuff, 1, 1, fout) != 1)
					{
						error("Failed to write output file!\n");
						return 1;
					}
				}

				// Write ac3 streamid
				if (ac3len != 0)
				{
					if (fwrite64(ac3_substream_id, ac3len, 1, fout) != 1)
					{
						error("Failed to write output file!\n");
						return 1;
					}
				}

				// Write rest of data len minus headersize (9) stuffing, and pack size (14)
				if (fwrite64(curbuf + pos + hdrsize, len - hdrsize - 14 - stuffing - ac3len, 1, fout) != 1)	// Write data bytes
				{
					error("Failed to write output file!\n");
					return 1;
				}
				written += len;

				// Add len minus stuff we added like the pack (14) and the stuffing.
				pos += len - 14 - stuffing - ac3len;
				if (pos == curpos)
					break;

				// Add pes header for next packet
				pos -= 9;
				make_pes_header(curbuf + pos, (pid == vidpid ? vidstreamid : audstreamid), 0, -1, -1);
			}

			// Write padding
			if ((written % PACKETSIZE) != 0)
			{
				int left = PACKETSIZE - (written % PACKETSIZE);

				// Pad out to PACKETSIZE bytes
				if (!pad_buffer(fout, left))
				{
					error("Couldn't write pad buffer!\n");
					return 1;
				}
			}

			curpos = 0;
			curcontinuity = -1;
			curpackets++;

#ifndef HDTVtoMPEG2
			// Print out a "." every 30 frames
			if (pid == vidpid)
			{
				if (curpackets % 30 == 0)
					printf(".");
			}
#endif
		}

		// Add the payload for this packet to the current buffer
		if (curfoundfirst && (184 - adapt_len) > 0)
		{
			memcpy(curbuf + curpos, buf + 4 + adapt_len, 184 - adapt_len);
			curpos += 184 - adapt_len;
		}
	}

	// Mark last valid stream pos
	unsigned char end[4] = {0,0,1,0xb9};
	fwrite64(&end, 4, 1, fout);

	// Close files
	fclose64(fout);

	// Delete video/audio buffers
	delete vidbuf;
	delete audbuf;

#ifndef HDTVtoMPEG2
	// All done!
	printf("\n\nNumber of video packets %d audio packets %d\nFinished\n", vidpackets, audpackets);
#endif
	
	return 0;
}

#ifndef HDTVtoMPEG2

int main(int argc, char* argv[])
{
	printf("bcdmux 1.04 - by Ben Cooley\n"
		   "    Converts HiPix or other MPEG2 TS file into standard DVD MPEG2 files.\n"
		   "    usage: bcdmux \"sourcefile\" videopid audiopid \"destfile\" <numfiles>\n"
		   "\n");

	if (argc < 4)
	{
		printf("      sourcefile: First MPEG2 ATSC transport stream file in seq (from HiPix)\n"
			   "        videopid: Usually 0x11 for video program 1, 0x21 for program 2, etc.\n"
			   "         audioid: Usually 0x14 for audio program 1, 0x24 for main audio 2\n"
			   "        destfile: MPEG2 file to output stream to.  Should have an .mpg ext\n"
			   "        numfiles: Num input files for each output file (usually 3-4)\n"
			   "\n"
			   "     note: If output file doesn't have digits in name, only one output\n"
			   "           file will be made. (i.e. needs zeros in Sting0000.mpg)\n"
			   "\n"
			   "  example: bcdmux \"Sting on Leno.ts.0000\" 0x11 0x14 \"Sting0000.mpg\" 3\n"
			   "\n"
			   "  This converts three sequentially numbered files to Sting0000.mpg\n");
		return 0;
	}
	
	// Input files
	int numifiles = -1;				// Indicates we should automatically increment vile number
	char *ifiles[1];
	ifiles[0] = argv[1];

	// Get the video PID
	int vid_pid = get_hex(argv[2]);

	// Get the audio PID
	int aud_pid = get_hex(argv[3]);

	// Get dest file pattern
	char* dfile = argv[4];

	// Get max files
	int maxfiles = 3; // Default three minutes worth of files.. nearly a gig
	if (argc > 5)
		maxfiles = atoi(argv[5]);

	// Convert files
	return convert_files(ifiles, numifiles, dfile, vid_pid, aud_pid, maxfiles, NULL, NULL);
}

#endif