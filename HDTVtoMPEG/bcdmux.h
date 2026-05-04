// ***************************************************************************
// bcdemux.exe - Copyright (C) 2002 - Ben Cooley
//
// bcdemux demultiplexes an ATSC transport stream and converts it to an MPEG2
// DVD compatible video file.  It can select any program in the TS file, and 
// can automatically read a sequence of TS files and convert them to any number
// of MPG files.

#ifndef _BCDMUX_H
#define _BCDMUX_H

#define ATSC_AC3_2CHANNEL 1
#define ATSC_AC3_5CHANNEL 2

struct ATSC_CHANNEL_INFO
{
	char name[8];
	int major_channel;
	int minor_channel;
	int width;
	int height;
	int audio;
	int videopid;
	int audiopid;
};

// Scans the TS file for channel info
bool scan_channel_info(const char* filename, int maxscan, int maxchannels, 
					   ATSC_CHANNEL_INFO* channels, int& numchannels);

// Typedef for callback function called during conversion
typedef bool (*convert_callback)(int ifilenum, char* ifilename, char* ofilename,
	__int64 curfilesize, __int64 curfilepos, __int64 curtotalsize, __int64 curtotalpos, void* data);

// Convert file function
int convert_files(char* ifilelist[], int numifiles, char* ofilepattern,
				  int vidpid, int audpid, int maxfilesize, convert_callback callback, void* data);

#endif