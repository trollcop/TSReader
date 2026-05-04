#include <windows.h>
#include <commctrl.h>
#include <time.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <io.h>
#include <memory.h>

#include "bcdmux.h"
#include "TSReader.h"

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

extern PVARIABLES v;

// ------------------------------------------------------------------------------------

static void error(char* errorstr, char* extra = NULL)
{
	char buf[128];
	OutputDebugString("ERROR: ");
	sprintf(buf, errorstr, extra);
	OutputDebugString(buf);
}

static void warning(char* errorstr, char* extra = NULL)
{
	char buf[128];
	OutputDebugString("WARNING: ");
	sprintf(buf, errorstr, extra);
	OutputDebugString(buf);
}

// ------------------------------------------------------------------------------------

BOOL WriteFileAndUpdateCounters(HANDLE hFile, LPCVOID lpBuffer, DWORD nNumberOfBytesToWrite, LPDWORD lpNumberOfBytesWritten, LPOVERLAPPED lpOverlapped)
{

	BOOL fRetVal = WriteFile(hFile, lpBuffer, nNumberOfBytesToWrite, lpNumberOfBytesWritten, lpOverlapped);

	v->dTotalRecorded += (double)*lpNumberOfBytesWritten;
	v->dThisFileRecorded += (double)*lpNumberOfBytesWritten;

	return fRetVal;
}


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

BOOL write_pack(__int64 time)
{
	DWORD dwWritten;
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

	return WriteFileAndUpdateCounters(v->hRecordFile, buf, buf_size(), &dwWritten, NULL);
}

int write_system(int videostream, int vidbscl, int vidbsize, int audiostream, int audbscl, int audbsize)
{
	DWORD dwWritten;
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

	WriteFileAndUpdateCounters(v->hRecordFile, buf, buf_size(), &dwWritten, NULL);
	return (int)dwWritten;
}

static bool pad_buffer(int pad)
{
	DWORD dwWritten;

	pad -= 6;

	char buf[6];
	buf[0] = '\x0'; buf[1] = '\x0'; buf[2] = '\x1'; buf[3] = '\xbe';
	buf[4] = pad >> 8; buf[5] = pad & 0xff;

	if (WriteFileAndUpdateCounters(v->hRecordFile, buf, 6, &dwWritten, NULL) == FALSE)
		return false;

	unsigned char padbyte = 0xff;
	for (int i = 0; i < pad; i++)
	{
		if (WriteFileAndUpdateCounters(v->hRecordFile, &padbyte, 1, &dwWritten, NULL) == FALSE)
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

static bool write_file_header(int vidstreamid, int audstreamid)
{ 
	// Write the first pack header
	if (!write_pack(0))
	{
		error("Couldn't write system pack header!\n");
		return false;
	}

	// Write the system block
	int nSystemBlockLen = write_system(vidstreamid, 1, 4096, audstreamid, 0, 58); // 4096k for video packets 58 for audio??
	if (!nSystemBlockLen)	
	{
		error("Couldn't write system header!\n");
		return false;
	}

	// Pad out to even multiple of PACKETSIZE
	if (!pad_buffer(PACKETSIZE - nSystemBlockLen))
	{
		error("Couldn't write pad buffer!\n");
		return false;
	}

	return true;
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

__int64 curfilesize = 0;
int maxfilesize = 1000;
__int64 maxfilesize64 = (__int64)maxfilesize * (__int64)(1024 * 1024);
bool cont = true;

// Packet arrays
unsigned char* packetbuf[2];
int packetpos[2];
int bufpackets[2];
bool foundfirst[2];
bool skipbad[2];
int streamcont[2];
int streamid[2];

	// Set the starting time value
	__int64 packet_time = 0;
	__int64 file_time = 0;
	__int64 PCR = -1;
	__int64 first_video_PCR;
	bool doing_iframe;
	int frame = 0;
	// Setup num files from max files
	int numfiles = 1;
	int bytesReadInPacket = 0;

int PS__StartWriting()
{
	// Get the video PID
	vidbuf = new unsigned char[1024 * 1024];
	vidpos = 0;
	vidfoundfirst = 0;
	vidskipbad = false;
	vidcontinuity = -1;
	vidpackets = 0;
	vidstreamid = 0xE0;	// MPEG2 video stream id

	// Get the audio PID
	audbuf = new unsigned char[1024 * 1024];
	audpos = 0;
	audfoundfirst = false;
	audskipbad = false;
	audcontinuity = -1;
	audpackets = 0;
	audstreamid = 0xBD;	// AC3, etc. streamid
		
	// Write the first pack header
	if (!write_file_header(vidstreamid, audstreamid))
		return 1;

	return 0;	
}

int PS__StopWriting()
{
	DWORD dwWritten;

	// Mark last valid stream pos
	unsigned char end[4] = {0,0,1,0xb9};
	WriteFileAndUpdateCounters(v->hRecordFile, &end, 4, &dwWritten, NULL);

	// Delete video/audio buffers
	delete vidbuf;
	delete audbuf;
	
	return 0;
}

BOOL CheckForFileSplit();

int x(BYTE * pPacketData, int nLength)
{
	int nPacketOffset;
	int curstream;

	for (nPacketOffset = 0; nPacketOffset < nLength; nPacketOffset += 188)
	{		
		BYTE * buf = pPacketData + nPacketOffset;

		if (CheckForFileSplit() == TRUE)
		{
			// Set the starting time value
			PCR = -1;
			frame = 0;
			file_time = 0;

			// Write the first pack header
			if (!write_file_header(vidstreamid, audstreamid))
					return 1;
		}

		// Check sync byte
		if ((buf[0] != 0x47) && (buf[0] != 0x72) && (buf[0] != 0x29))
		{
			warning("Bad transport packet (no sync byte 0x47)!\n");
			audskipbad = vidskipbad = true;
			continue;
		}

		// Get pid
		int pid = (((buf[1] & 0x1F) << 8) | buf[2]) & 0x1FFF;

		// Skip this block
		if (pid != v->nRecordAudioPID && pid != v->nRecordVideoPID)
			continue;

		// Get the pos and buf
		if (pid == v->nRecordVideoPID)
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
		if (pid == v->nRecordAudioPID && start)
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
			if (pid == v->nRecordVideoPID)
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
			else if (pid == v->nRecordAudioPID && !audfoundfirst && vidfoundfirst)  // Set audio found first ONLY after first video frame found..
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
			if (pid == v->nRecordAudioPID && audstreamid == 0xBD)
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
				DWORD dwWritten;

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
				packet_time = ((__int64)v->dTotalRecorded * CLOCKRATE / STREAMRATE) + file_time;
				if (!write_pack(packet_time))
				{
					error("Couldn't write pack header!\n");
					return 1;
				}

				if (pid == v->nRecordAudioPID)
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
				if (!WriteFileAndUpdateCounters(v->hRecordFile, curbuf + pos, hdrsize, &dwWritten, NULL))	// Write pes id, streamid, and len
				{
					error("Failed to write output file!\n");
					return 1;
				}
				
				// Write stuffing
				for (int i = 0; i < stuffing; i++)				// Write any stuffing bytes
				{
					unsigned char stuff = 0xff;
					if (!WriteFileAndUpdateCounters(v->hRecordFile, &stuff, 1, &dwWritten, NULL))
					{
						error("Failed to write output file!\n");
						return 1;
					}
				}

				// Write ac3 streamid
				if (ac3len != 0)
				{
					if (!WriteFileAndUpdateCounters(v->hRecordFile, ac3_substream_id, ac3len, &dwWritten, NULL))
					{
						error("Failed to write output file!\n");
						return 1;
					}
				}

				// Write rest of data len minus headersize (9) stuffing, and pack size (14)
				if (!WriteFileAndUpdateCounters(v->hRecordFile, curbuf + pos + hdrsize, len - hdrsize - 14 - stuffing - ac3len, &dwWritten, NULL))	// Write data bytes
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
				make_pes_header(curbuf + pos, (pid == v->nRecordVideoPID ? vidstreamid : audstreamid), 0, -1, -1);
			}

			// Write padding
			if ((written % PACKETSIZE) != 0)
			{
				int left = PACKETSIZE - (written % PACKETSIZE);

				// Pad out to PACKETSIZE bytes
				if (!pad_buffer(left))
				{
					error("Couldn't write pad buffer!\n");
					return 1;
				}
			}

			curpos = 0;
			curcontinuity = -1;
			curpackets++;
		}

		// Add the payload for this packet to the current buffer
		if (curfoundfirst && (184 - adapt_len) > 0)
		{
			memcpy(curbuf + curpos, buf + 4 + adapt_len, 184 - adapt_len);
			curpos += 184 - adapt_len;
		}
	}

	return 0;
}


