#include <Windows.h>
#include <CommCtrl.h>
#include <limits.h>
#include "TSReader.h"
#include "util.h"

#include "resource.h"

#include "libfaad/include/neaacdec.h"

extern PVARIABLES v;

#define BUFFER_SIZE 20480

static BYTE *pThumbnail[REAL_MAX_ES_PARSERS];
static signed short *pSamples[REAL_MAX_ES_PARSERS];
static int nSampleWriteIndex[REAL_MAX_ES_PARSERS];
DWORD WINAPI AACAudioDecoderThread(LPVOID lpv);

DWORD WINAPI AACAudioDecoderThread(LPVOID lpv)
{
	PESPARSERINFO esparserinfo = (PESPARSERINFO)lpv;
	int nDestWidth;
	int nDestHeight;
	BOOL fGeneratedThumbnail = FALSE;
	NeAACDecHandle hAac;
	NeAACDecConfigurationPtr conf;
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
	nSampleWriteIndex[esparserinfo->nES] = 0;

	// Open the library
	hAac = NeAACDecOpen();
	if (hAac != NULL)
	{
		int nReadLength;
		unsigned long samplerate;
		long err;
		int nWriteOffset = 0;
		int nLoop = 0;
		unsigned char channels;

		// Set to stereo downmix
		conf =	NeAACDecGetCurrentConfiguration(hAac);
		conf->downMatrix = 1;
		conf->defObjectType = MAIN;
		NeAACDecSetConfiguration(hAac, conf);

		do
		{
			nReadLength = ReadFromMPEG2ESPipe(bBuffer, BUFFER_SIZE / 10, esparserinfo->nES);
			if (nReadLength != 0)
				break;
			Sleep(10);
		} while (nLoop++ < 100);
		if (nLoop == 100)
			goto aac_windup;
		nWriteOffset = nReadLength;
		
		// Initialise the library using one of the initialization functions
		err = NeAACDecInit(hAac, bBuffer, nReadLength, &samplerate, &channels);
		if (err != 0)
			goto aac_windup;

		// Loop until decoding finished
		nLoop = 0;
		do
		{
			NeAACDecFrameInfo hInfo;
			signed short * samplebuffer;

			if (BUFFER_SIZE - nWriteOffset > 0)
			{
				int nReadSize = BUFFER_SIZE - nWriteOffset;
				nReadLength = ReadFromMPEG2ESPipe(&bBuffer[nWriteOffset], nReadSize, esparserinfo->nES);
				if (nReadLength == 0)
					break;
				nWriteOffset += nReadLength;
			}

			// Decode the frame in buffer
			samplebuffer = NeAACDecDecode(hAac, &hInfo, bBuffer, nWriteOffset);
			if ((hInfo.error == 0) && (hInfo.samples > 0))
			{
				int j;

				memcpy(bBuffer, &bBuffer[hInfo.bytesconsumed], BUFFER_SIZE - hInfo.bytesconsumed);
				nWriteOffset -= hInfo.bytesconsumed;

				for (j = 0; j < 256 * 2; j += 2)
				{
					pSamples[esparserinfo->nES][nSampleWriteIndex[esparserinfo->nES]++] = samplebuffer[j];
					pSamples[esparserinfo->nES][nSampleWriteIndex[esparserinfo->nES]++] = samplebuffer[j + 1];
					if (nSampleWriteIndex[esparserinfo->nES] == SAMPLES_REQUIRED)
						break;
				}

				if (nSampleWriteIndex[esparserinfo->nES] == SAMPLES_REQUIRED)
				{
					int nAudioChannels = channels;
					PPARSEDAACAUDIO pAAC;

					GenerateAudioThumbnail(pSamples[esparserinfo->nES], nAudioChannels, nDestWidth, nDestHeight, pThumbnail[esparserinfo->nES],
									       v->nESParsePMTIndex[esparserinfo->nES], v->nESParseESIndex[esparserinfo->nES]);
					fGeneratedThumbnail = TRUE;
					dbg_printf("TSReader: Completed AAC parser for program %d ES thread %d\n", v->pat.pmt[v->nESParsePMTIndex[esparserinfo->nES]].nProgramNumber, esparserinfo->nES);
					
					pAAC = LocalAlloc(LPTR, sizeof(PARSEDAACAUDIO));
					pAAC->channels = hInfo.channels;
					pAAC->samplerate = hInfo.samplerate;
					pAAC->sbr = hInfo.sbr;
					pAAC->object_type = hInfo.object_type;
					pAAC->header_type = hInfo.header_type;
					pAAC->num_front_channels = hInfo.num_front_channels;
					pAAC->num_side_channels = hInfo.num_side_channels;
					pAAC->num_back_channels = hInfo.num_back_channels;
					pAAC->num_lfe_channels = hInfo.num_lfe_channels;
					if (v->pat.pmt[v->nESParsePMTIndex[esparserinfo->nES]].es[v->nESParseESIndex[esparserinfo->nES]].pParsedData)
						LocalFree(v->pat.pmt[v->nESParsePMTIndex[esparserinfo->nES]].es[v->nESParseESIndex[esparserinfo->nES]].pParsedData);
					v->pat.pmt[v->nESParsePMTIndex[esparserinfo->nES]].es[v->nESParseESIndex[esparserinfo->nES]].pParsedData = (BYTE *)pAAC;
					break;
				}
			}
			else if (hInfo.error != 0)
			{
				const char *error = NeAACDecGetErrorMessage(hInfo.error);

				dbg_printf("TSReader: AAC: %02X%02X%02X%02X%02X%02X%02X%02X%02X loop=%d %s\n", bBuffer[0], bBuffer[1], bBuffer[2], bBuffer[3], bBuffer[4], bBuffer[5], bBuffer[6], bBuffer[7], bBuffer[8], nLoop, error);
				nLoop += 50;
			}
		} while (nLoop++ < 100);		
		NeAACDecClose(hAac);
	}

aac_windup:
	if (!fGeneratedThumbnail)
		LocalFree(pThumbnail[esparserinfo->nES]);
	LocalFree(pSamples[esparserinfo->nES]);
	CloseHandle(v->hMPEGDecoderReadPipe[esparserinfo->nES]);
	CloseHandle(v->hMPEGDecoderWritePipe[esparserinfo->nES]);
	EnterCriticalSection(&v->esparserinfo[esparserinfo->nES].csThreadSignal);
	v->fESParseDecoderCompletedLibMPEG[esparserinfo->nES] = TRUE;
	v->fMPEG2DecoderThreadRunning[esparserinfo->nES] = FALSE;
	LeaveCriticalSection(&v->esparserinfo[esparserinfo->nES].csThreadSignal);
	dbg_printf("TSReader: Completed AAC audio for program %d ES thread %d\n", esparserinfo->nProgramNumber, esparserinfo->nES);

	return 0;
}
