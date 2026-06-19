#include <windows.h>
#include <commctrl.h>
#include "TSReader.h"
#include "resource.h"
#include "util.h"
#include "bcdmux.h"
#include "parser.h"

// Stuff for libmpeg
#include <stdio.h>
#include <stdlib.h>
#include "inttypes.h"
#include "mpeg2.h"
#include "mpeg2convert.h"

extern PVARIABLES v;

#ifdef _DEBUG
void save_ppm(int width, int height, uint8_t * buf, int num, char * szDebugName)
{
    char filename[MAX_PATH];
    FILE * ppmfile;

    sprintf (filename, "%s-%02d.ppm", szDebugName, num);
    ppmfile = fopen (filename, "wb");
    if (!ppmfile)
	{
		fprintf (stderr, "Could not open file \"%s\".\n", filename);
		exit (1);
    }
    fprintf (ppmfile, "P6\n%d %d\n255\n", width, height);
    fwrite (buf, 3 * width, height, ppmfile);
    fclose (ppmfile);
}
#endif _DEBUG

#define SIZE_TO_LIBMPEG2 4096
DWORD WINAPI MPEG2DecoderThread(LPVOID lpv)
{
	PESPARSERINFO esparserinfo = (PESPARSERINFO)lpv;
	int nPacketLength;
	int nPictureCount = 0;
	int nWriteBuffer = 0;
	int nMaximumPictures = v->nMaximumMPEGPictures;
	BYTE * buffer[2];
    mpeg2_state_t state;
	char szStatus[256] = {0};
    mpeg2dec_t * mpeg2dec;
    const mpeg2_info_t * info;

	EnterCriticalSection(&v->esparserinfo[esparserinfo->nES].csThreadSignal);
	v->fMPEG2DecoderThreadRunning[esparserinfo->nES] = TRUE;
	LeaveCriticalSection(&v->esparserinfo[esparserinfo->nES].csThreadSignal);

	//wsprintf(szDebugName, "c:\\MPEG-ES\\%08x.m2v", GetTickCount());
	//hDebug = CreateFile(szDebugName, GENERIC_WRITE, 0, (LPSECURITY_ATTRIBUTES) NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, (HANDLE) NULL);

	buffer[0] = LocalAlloc(LMEM_FIXED, 1024 * 1024);
	buffer[1] = LocalAlloc(LMEM_FIXED, 1024 * 1024);
	mpeg2_accel(MPEG2_ACCEL_DETECT);		
	mpeg2dec = mpeg2_init();
	info = mpeg2_info(mpeg2dec);
	mpeg2_reset(mpeg2dec, TRUE);

	if (v->pat.pmt[v->nESParsePMTIndex[esparserinfo->nES]].es[v->nESParseESIndex[esparserinfo->nES]].nStreamType == 0x80)
		nMaximumPictures = v->nMaximumDCIIPictures;

	do
	{
		BYTE * pPESPacket;
		int nReadLength;
		int nReturned;

		if (v->fRunning == FALSE)
			break;

		nReturned = ReadFromMPEG2ESPipe((BYTE *)&nReadLength, sizeof(nReadLength), esparserinfo->nES);
		if (nReadLength == 0 || nReturned == 0)
			break;

		nPacketLength = ReadFromMPEG2ESPipe(buffer[nWriteBuffer], nReadLength, esparserinfo->nES);
		if (nPacketLength != nReadLength)
			break;
		{
			//DWORD dwWritten;
			//WriteFile(hDebug, buffer[nWriteBuffer], nPacketLength, &dwWritten, NULL);
		}
		pPESPacket = buffer[nWriteBuffer];
		do
		{
			BOOL fCrashed = TRUE;

			if (v->fRunning == FALSE)
				break;

			__try
			{
				state = mpeg2_parse(mpeg2dec);
				fCrashed = FALSE;
			} __except(EXCEPTION_EXECUTE_HANDLER)
			{
				// Don't really do anything if we crash
			}
			if (fCrashed)
				goto MPEG2Crash;

			switch (state)
			{
			default:
				break;
			case STATE_PICTURE:
				if (info != NULL)
					QuickParseUserData((BYTE *)info->user_data, info->user_data_len,
					                   v->nESParsePMTIndex[esparserinfo->nES], v->nESParseESIndex[esparserinfo->nES],
									   esparserinfo->nES);
				break;
			case STATE_BUFFER:
				{
					int nThisPass;

					nThisPass = nPacketLength;
					if (nThisPass > SIZE_TO_LIBMPEG2)
						nThisPass = SIZE_TO_LIBMPEG2;

					mpeg2_buffer(mpeg2dec, pPESPacket, pPESPacket + nThisPass);
					pPESPacket += nThisPass;
					nPacketLength -= nThisPass;
					if (nPacketLength == 0)
					{
						nWriteBuffer++;
						if (nWriteBuffer > 1)
							nWriteBuffer = 0;
					}
				}
				break;
			case STATE_SEQUENCE:
				mpeg2_convert(mpeg2dec, mpeg2convert_rgb24, NULL);
				break;
			case STATE_SLICE:
			case STATE_END:
			case STATE_INVALID_END:
				nPictureCount++;
				if ( (info->display_fbuf) && (info->current_picture) )
				{
					if ((info->current_picture->flags & PIC_MASK_CODING_TYPE) == PIC_FLAG_CODING_TYPE_I)
						lstrcat(szStatus, "I");
					else if ((info->current_picture->flags & PIC_MASK_CODING_TYPE) == PIC_FLAG_CODING_TYPE_B)
						lstrcat(szStatus, "B");
					else if ((info->current_picture->flags & PIC_MASK_CODING_TYPE) == PIC_FLAG_CODING_TYPE_P)
						lstrcat(szStatus, "P");

					if (nPictureCount <= nMaximumPictures)
					{
						if  (   (!v->fThumbnailThreadAnimated) && ((info->current_picture->flags & PIC_MASK_CODING_TYPE) != PIC_FLAG_CODING_TYPE_I)
							 || (!v->fThumbnailThreadAnimated) && (nPictureCount <= 45) && ((info->current_picture->flags & PIC_MASK_CODING_TYPE) != PIC_FLAG_CODING_TYPE_I) )
							break;
					}
					//if ( (!v->fThumbnailThreadAnimated) && (nPictureCount < nMaximumPictures) )
					//	break;
					if (info->sequence == 0)
						break;
					{				
						if (info->sequence->height <= 1100)
						{
							int nDestHeight, nDestWidth;
							int nSourceHeight = info->sequence->height;

							GetNewThumbnailSize(&nSourceHeight, &nDestHeight, &nDestWidth);
							GenerateSizedThumbnail(info->display_fbuf->buf[0], nDestWidth, nDestHeight, info->sequence->width, nSourceHeight,
								                   v->nESParsePMTIndex[esparserinfo->nES], v->nESParseESIndex[esparserinfo->nES]);

							if ( ((info->current_picture->flags & PIC_MASK_CODING_TYPE) == PIC_FLAG_CODING_TYPE_I)
								 || (nPictureCount > nMaximumPictures) )
							{
								if (v->fSaveThumbnails)
									DecoderThread_SaveThumbnail(szStatus, esparserinfo->nES, info->sequence->width, info->sequence->height,
									                                 info->display_fbuf->buf[0]);
								if (v->fArchiveRunning)
									SaveArchiveThumbnail(szStatus, esparserinfo->nES);
								if (v->hWndVideoMosaic != NULL)
									InvalidateRect(v->hWndVideoMosaic, NULL, FALSE);
								mpeg2_close(mpeg2dec);

								CloseHandle(v->hMPEGDecoderReadPipe[esparserinfo->nES]);
								CloseHandle(v->hMPEGDecoderWritePipe[esparserinfo->nES]);
								LocalFree(buffer[0]);
								LocalFree(buffer[1]);
								//CloseHandle(hDebug);
								{
									char szTemp[1024];
									wsprintf(szTemp, "TSReader: Completed MPEG-2 video for program %d ES thread %d (%s)\n",
										     esparserinfo->nProgramNumber, esparserinfo->nES, szStatus);
									OutputDebugString(szTemp);
								}
								EnterCriticalSection(&v->esparserinfo[esparserinfo->nES].csThreadSignal);
								v->fESParseDecoderCompletedLibMPEG[esparserinfo->nES] = TRUE;
								v->fMPEG2DecoderThreadRunning[esparserinfo->nES] = FALSE;
								LeaveCriticalSection(&v->esparserinfo[esparserinfo->nES].csThreadSignal);
								return 0;
							}
						}
					}
				}
				break;
			}		
		} while (nPacketLength > 0);
	} while (TRUE);

MPEG2Crash:

	OutputDebugString("MPEG2DecoderThread: Should never get here - decoder crashed\n");
	//CloseHandle(hDebug);
	LocalFree(buffer[0]);
	LocalFree(buffer[1]);
	if (v->fRunning)
		LoadVideoDecoderCrashThumbnail(v->nESParsePMTIndex[esparserinfo->nES], v->nESParseESIndex[esparserinfo->nES]);

	EnterCriticalSection(&v->esparserinfo[esparserinfo->nES].csThreadSignal);
	v->fESParseDecoderCompletedLibMPEG[esparserinfo->nES] = TRUE;
	v->fMPEG2DecoderThreadRunning[esparserinfo->nES] = FALSE;
	LeaveCriticalSection(&v->esparserinfo[esparserinfo->nES].csThreadSignal);
	return 0;
}
