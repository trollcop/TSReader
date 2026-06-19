#include <windows.h>
#include <commctrl.h>
#include <limits.h>
#include "TSReader.h"
#include "util.h"

#include "resource.h"

// liba52
#include <a52.h>
#include <audio_out.h>
#include <audio_out_internal.h>
#include <mm_accel.h>

extern PVARIABLES v;

//static int disable_dynrng = 0;
//static int disable_adjust = 0;
//static sample_t gain = 1;

static a52_state_t * state[REAL_MAX_ES_PARSERS];
static ao_instance_t * output[REAL_MAX_ES_PARSERS];
static signed short *pSamples[REAL_MAX_ES_PARSERS];
static int nSampleWriteIndex[REAL_MAX_ES_PARSERS];
static int count[REAL_MAX_ES_PARSERS];
static BYTE *pThumbnail[REAL_MAX_ES_PARSERS];
static uint8_t buf[REAL_MAX_ES_PARSERS][3840];
static uint8_t * bufptr[REAL_MAX_ES_PARSERS];
static uint8_t * bufpos[REAL_MAX_ES_PARSERS];

#define BUFFER_SIZE 10 * 8192

BOOL AC3Init(int nES)
{
	int i;
    ao_driver_t * drivers;

    drivers = ao_drivers ();
	for (i = 0; drivers[i].open != NULL; i++)
	{
		if (lstrcmp(drivers[i].name, "null") == 0)
			break;
	}
	if (drivers[i].open == NULL)
	{
		OutputDebugString("Can't locate null output driver\n");
		return FALSE;
	}
    output[nES] = ao_open(drivers[i].open);
    if (output[nES] == NULL)
	{
		OutputDebugString("Can not open output\n");
		return FALSE;
    }

    state[nES] = a52_init(MM_ACCEL_DJBFFT);
    if (state[nES] == NULL)
	{
		OutputDebugString("A52 init failed\n");
		return FALSE;
    }

	return TRUE;
}

BOOL AC3DeInit(int nES)
{
    a52_free (state[nES]);
    ao_close (output[nES]);
	return TRUE;
}

BOOL a52_decode_data(uint8_t * start, uint8_t * end, int nES, int nDestWidth, int nDestHeight)
{    
    // sample_rate and flags are static because this routine could
    // exit between the a52_syncinfo() and the ao_setup(), and we want
    // to have the same values when we get back !
    static int sample_rate[REAL_MAX_ES_PARSERS];
    static int flags[REAL_MAX_ES_PARSERS];
    int bit_rate;
    int len;

    while (TRUE)
	{
		len = end - start;
		if (!len)
			break;
		if (len > bufpos[nES] - bufptr[nES])
			len = bufpos[nES] - bufptr[nES];
		memcpy (bufptr[nES], start, len);
		bufptr[nES] += len;
		start += len;
		if (bufptr[nES] == bufpos[nES])
		{
			if (bufpos[nES] == buf[nES] + 7)
			{
				int length;

				length = a52_syncinfo (buf[nES], &flags[nES], &sample_rate[nES], &bit_rate);
				if (!length)
				{
					for (bufptr[nES] = buf[nES]; bufptr[nES] < buf[nES] + 6; bufptr[nES]++)
						bufptr[nES][0] = bufptr[nES][1];
					continue;
				}
				bufpos[nES] = buf[nES] + length;
			}
			else
			{
				sample_t level, bias;
				int i;

				if (ao_setup (output[nES], sample_rate[nES], &flags[nES], &level, &bias))
					goto error;
				flags[nES] |= A52_ADJUST_LEVEL;
				if (a52_frame (state[nES], buf[nES], &flags[nES], &level, bias))
					goto error;
				for (i = 0; i < 1; i++)
				{
					sample_t * samples;

					if (a52_block (state[nES]))
						goto error;
					samples = a52_samples (state[nES]);
					//if (ao_play (output[nES], flags[nES], samples))
					//	goto error;
					//else
					{
						if (++count[nES] > 1)
						{
							int j;
							int16_t int16_samples[256 * 2];

							float2s16_2 (samples, int16_samples);
							for (j = 0; j < 256 * 2; j += 2)
							{
								pSamples[nES][nSampleWriteIndex[nES]++] = int16_samples[j];
								pSamples[nES][nSampleWriteIndex[nES]++] = int16_samples[j + 1];
								if (nSampleWriteIndex[nES] == SAMPLES_REQUIRED)
									break;
							}

							if (nSampleWriteIndex[nES] == SAMPLES_REQUIRED)
							{
								int nAudioChannels = 2;
								GenerateAudioThumbnail(pSamples[nES], nAudioChannels, nDestWidth, nDestHeight, pThumbnail[nES],
									                   v->nESParsePMTIndex[nES], v->nESParseESIndex[nES]);
								{
									char szTemp[128];
									wsprintf(szTemp, "TSReader: Completed AC-3 parser for program %d ES thread %d\n",
										     v->pat.pmt[v->nESParsePMTIndex[nES]].nProgramNumber, nES);
									OutputDebugString(szTemp);
								}
								return TRUE;
							}
						}
					}
				}
error:
				bufptr[nES] = buf[nES];
				bufpos[nES] = buf[nES] + 7;
				continue;
		    }
		}
    }
	return FALSE;
}

DWORD WINAPI AC3AudioDecoderThread(LPVOID lpv)
{
	PESPARSERINFO esparserinfo = (PESPARSERINFO)lpv;
	int nLoop = 0;
	int nDestWidth;
	int nDestHeight;
	BYTE bBuffer[BUFFER_SIZE];

	EnterCriticalSection(&v->esparserinfo[esparserinfo->nES].csThreadSignal);
	v->fMPEG2DecoderThreadRunning[esparserinfo->nES] = TRUE;
	LeaveCriticalSection(&v->esparserinfo[esparserinfo->nES].csThreadSignal);

	nDestWidth = 240;
	nDestHeight = 52;
	
	switch(v->nThumbnailSize)
	{
	case 1:
		nDestWidth *= 2; nDestWidth /= 3;
		break;
	case 2:
		nDestWidth *= 2;
		break;
	}
	
	pThumbnail[esparserinfo->nES] = LocalAlloc(LPTR, nDestWidth * nDestHeight * 3);
	pSamples[esparserinfo->nES] = LocalAlloc(LMEM_FIXED, sizeof(signed short) * SAMPLES_REQUIRED);

	if (AC3Init(esparserinfo->nES) != FALSE)
	{
		bufptr[esparserinfo->nES] = buf[esparserinfo->nES];
		bufpos[esparserinfo->nES] = buf[esparserinfo->nES] + 7;
		count[esparserinfo->nES] = 0;
		nSampleWriteIndex[esparserinfo->nES] = 0;
		do
		{
			int nReadLength;
			int nESPacketLength;
			int nReturned;

			if (v->fRunning == FALSE)
				break;
			nReturned = ReadFromMPEG2ESPipe((BYTE *)&nReadLength, sizeof(nReadLength), esparserinfo->nES);
			if (nReadLength == 0 || nReturned == 0)
				break;

			nESPacketLength = ReadFromMPEG2ESPipe(bBuffer, nReadLength, esparserinfo->nES);
			if (nESPacketLength == 0)
				break;
			if (a52_decode_data(bBuffer, bBuffer + nESPacketLength, esparserinfo->nES, nDestWidth, nDestHeight) == TRUE)
				break;
			if (++nLoop > 1000)
				break;
		} while (TRUE);
		AC3DeInit(esparserinfo->nES);
	}
	else
		LocalFree(pThumbnail[esparserinfo->nES]);

	LocalFree(pSamples[esparserinfo->nES]);
	CloseHandle(v->hMPEGDecoderReadPipe[esparserinfo->nES]);
	CloseHandle(v->hMPEGDecoderWritePipe[esparserinfo->nES]);
	EnterCriticalSection(&v->esparserinfo[esparserinfo->nES].csThreadSignal);
	v->fESParseDecoderCompletedLibMPEG[esparserinfo->nES] = TRUE;
	v->fMPEG2DecoderThreadRunning[esparserinfo->nES] = FALSE;
	LeaveCriticalSection(&v->esparserinfo[esparserinfo->nES].csThreadSignal);
	{
		char szTemp[1024];
		wsprintf(szTemp, "TSReader: Completed AC-3 audio for program %d ES thread %d\n",
				 esparserinfo->nProgramNumber, esparserinfo->nES);
		OutputDebugString(szTemp);
	}
	return 0;
}
