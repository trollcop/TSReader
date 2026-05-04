#ifndef LITE
#include <windows.h>
#include <commctrl.h>
#include <limits.h>
#include "TSReader.h"
#include "util.h"

#include "resource.h"

// libmad
#include <mad.h>

extern PVARIABLES v;

// Variables
struct mad_stream	Stream[REAL_MAX_ES_PARSERS];
struct mad_frame	Frame[REAL_MAX_ES_PARSERS];
struct mad_synth	Synth[REAL_MAX_ES_PARSERS];
mad_timer_t			Timer[REAL_MAX_ES_PARSERS];

#define INPUT_BUFFER_SIZE	(4096)

BOOL MPEGAInit(int nES)
{
	// First the structures used by libmad must be initialized
	mad_stream_init(&Stream[nES]);
	mad_frame_init(&Frame[nES]);
	mad_synth_init(&Synth[nES]);
	mad_timer_reset(&Timer[nES]);

	return TRUE;
}

BOOL MPEGADeInit(int nES)
{

	mad_synth_finish(&Synth[nES]);
	mad_frame_finish(&Frame[nES]);
	mad_stream_finish(&Stream[nES]);

	return TRUE;
}

static signed short MadFixedToSshort(mad_fixed_t Fixed)
{
	// Clipping
	if(Fixed >= MAD_F_ONE)
		return(SHRT_MAX);
	if(Fixed <= -MAD_F_ONE)
		return(-SHRT_MAX);

	// Conversion
	Fixed=Fixed>>(MAD_F_FRACBITS-15);
	return((signed short)Fixed);
}

DWORD WINAPI MPEGAudioDecoderThread(LPVOID lpv)
{
	PESPARSERINFO esparserinfo = (PESPARSERINFO)lpv;
	int nSampleCount = 0;
	int nDestWidth = 240;
	int nDestHeight = 52;
	int fTimeoutCounter = 0;
	int nReadSize, nRemaining;
	int nSampleWriteIndex = 0;
	unsigned char * pReadStart;
	BYTE * pThumbnail;
	signed short * pSamples;
	BYTE InputBuffer[INPUT_BUFFER_SIZE+MAD_BUFFER_GUARD];

	EnterCriticalSection(&v->esparserinfo[esparserinfo->nES].csThreadSignal);
	v->fMPEG2DecoderThreadRunning[esparserinfo->nES] = TRUE;
	LeaveCriticalSection(&v->esparserinfo[esparserinfo->nES].csThreadSignal);
	
	switch(v->nThumbnailSize)
	{
	case 1:
		nDestWidth *= 2; nDestWidth /= 3;
		break;
	case 2:
		nDestWidth *= 2;
		break;
	}

	pThumbnail = LocalAlloc(LPTR, nDestWidth * nDestHeight * 3);
	pSamples = LocalAlloc(LMEM_FIXED, sizeof(signed short) * SAMPLES_REQUIRED);

	MPEGAInit(esparserinfo->nES);
	do
	{
		int nReadLength;
		int i;

		if (v->fRunning == FALSE)
			break;

		if(Stream[esparserinfo->nES].next_frame != NULL)
		{
			nRemaining = Stream[esparserinfo->nES].bufend - Stream[esparserinfo->nES].next_frame;
			memmove(InputBuffer, Stream[esparserinfo->nES].next_frame, nRemaining);
			pReadStart = InputBuffer + nRemaining;
			nReadSize = INPUT_BUFFER_SIZE - nRemaining;
			nReadLength = 0;
		}
		else
		{
			nReadSize=INPUT_BUFFER_SIZE;
			pReadStart=InputBuffer;
			nRemaining=0;
			nReadLength = ReadFromMPEG2ESPipe(pReadStart, nReadSize, esparserinfo->nES);
			if (nReadLength == 0)
				break;
		}
		
		mad_stream_buffer(&Stream[esparserinfo->nES], InputBuffer, nReadLength + nRemaining);
		Stream[esparserinfo->nES].error = 0;
		if(mad_frame_decode(&Frame[esparserinfo->nES], &Stream[esparserinfo->nES]))
		{
			if(Stream[esparserinfo->nES].error != MAD_ERROR_LOSTSYNC)
			{
				char szTemp[128];
				wsprintf(szTemp,"%s: recoverable frame level error (%s) %d\n",
						"TSReader",mad_stream_errorstr(&Stream[esparserinfo->nES]), esparserinfo->nProgramNumber);
				//OutputDebugString(szTemp);
			}
			//continue;
		}
		else
		{
			if(Stream[esparserinfo->nES].error == MAD_ERROR_BUFLEN)
				continue;
			else
			{
				char szTemp[128];
				wsprintf(szTemp,"%s: unrecoverable frame level error (%s) %d\n",
						"TSReader",mad_stream_errorstr(&Stream[esparserinfo->nES]), esparserinfo->nProgramNumber);
				//OutputDebugString(szTemp);
				//break;
			}
		}

		if (!Stream[esparserinfo->nES].sync)
		{
			fTimeoutCounter++;
			if (fTimeoutCounter > 10000)
				break;		// ?? no idea if this is right
			continue;
		}
		else
			fTimeoutCounter = 0;

		mad_timer_add(&Timer[esparserinfo->nES], Frame[esparserinfo->nES].header.duration);
		mad_synth_frame(&Synth[esparserinfo->nES], &Frame[esparserinfo->nES]);

		if (++nSampleCount < 4)
			continue;

		for(i=0; i < Synth[esparserinfo->nES].pcm.length; i++)
		{
			signed short Sample;

			// Left channel
			pSamples[nSampleWriteIndex++] = Sample = MadFixedToSshort(Synth[esparserinfo->nES].pcm.samples[0][i]);

			// Right channel. If the decoded stream is monophonic then
			// the right output channel is the same as the left one.
			if(MAD_NCHANNELS(&Frame[esparserinfo->nES].header)==2)
				Sample = MadFixedToSshort(Synth[esparserinfo->nES].pcm.samples[1][i]);
			pSamples[nSampleWriteIndex++] = Sample;
			if (nSampleWriteIndex == SAMPLES_REQUIRED)
				break;
		}
		if (nSampleWriteIndex == SAMPLES_REQUIRED)
			break;
	} while (TRUE);

	GenerateAudioThumbnail(pSamples, MAD_NCHANNELS(&Frame[esparserinfo->nES].header), nDestWidth, nDestHeight, pThumbnail,
		                   v->nESParsePMTIndex[esparserinfo->nES], v->nESParseESIndex[esparserinfo->nES]);

	LocalFree(pSamples);
	CloseHandle(v->hMPEGDecoderReadPipe[esparserinfo->nES]);
	CloseHandle(v->hMPEGDecoderWritePipe[esparserinfo->nES]);
	EnterCriticalSection(&v->esparserinfo[esparserinfo->nES].csThreadSignal);
	v->fESParseDecoderCompletedLibMPEG[esparserinfo->nES] = TRUE;
	v->fMPEG2DecoderThreadRunning[esparserinfo->nES] = FALSE;
	LeaveCriticalSection(&v->esparserinfo[esparserinfo->nES].csThreadSignal);
	MPEGADeInit(esparserinfo->nES);
	return 0;
}
#endif LITE
