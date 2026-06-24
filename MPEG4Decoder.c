#include <windows.h>
#include <commctrl.h>
#include "TSReader.h"
#include "resource.h"
#include "util.h"
#include "bcdmux.h"

extern PVARIABLES v;

typedef struct _tagMPEG4Decode
{
	unsigned char * pY;
	unsigned char * pU;
	unsigned char * pV;
	int x, y;
} MPEG4DECODE, *PMPEG4DECODE;

typedef int (*td_MPEG4) (HANDLE hInPipe, PMPEG4DECODE hd);

#ifdef USE_FFMPEG
DWORD WINAPI MPEG4DecoderThread2(LPVOID lpv)
#else
DWORD WINAPI MPEG4DecoderThread(LPVOID lpv)
#endif
{
	td_MPEG4 MPEG4 = NULL;

	PESPARSERINFO esparserinfo = (PESPARSERINFO)lpv;
	MPEG4DECODE hd = {0};
	HMODULE hDLL;

	OutputDebugString("TSReader: MPEG4DecoderThread+\n");

	hDLL = LoadLibrary("TSReader_MPEG4.dll");
	if (hDLL == NULL)
	{
		OutputDebugString("TSReader: MPEG4DecoderThread: Unable to load TSReader_MPEG4.dll\n");
		goto windup_MPEG4_decode;
	}
	MPEG4 = (td_MPEG4)GetProcAddress(hDLL, "MPEG4");

	hd.pY = LocalAlloc(LMEM_FIXED, 1920 * 1088);
	hd.pU = LocalAlloc(LMEM_FIXED, (1920 * 1088) / 2);
	hd.pV = LocalAlloc(LMEM_FIXED, (1920 * 1088) / 2);
	
	EnterCriticalSection(&v->esparserinfo[esparserinfo->nES].csThreadSignal);
	v->fMPEG2DecoderThreadRunning[esparserinfo->nES] = TRUE;
	LeaveCriticalSection(&v->esparserinfo[esparserinfo->nES].csThreadSignal);

	__try
	{
		MPEG4(v->hMPEGDecoderReadPipe[esparserinfo->nES], &hd);
	} __except(EXCEPTION_EXECUTE_HANDLER )
	{
		// MPEG-4 decoder crashed on us - so we put up a decoder crash picture
		hd.x = hd.y = 0;
	}
	if (hd.x && hd.y)
	{
		// got an image - convert it to RGB
		BYTE * pImage = LocalAlloc(LPTR, hd.x * hd.y * 3);
		int nDestHeight, nDestWidth;
		int nSourceHeight = hd.y;

		YUVtoRGB(pImage, hd.pY, hd.pU, hd.pV, hd.x, hd.y); 
		GetNewThumbnailSize(&nSourceHeight, &nDestHeight, &nDestWidth);
		GenerateSizedThumbnail(pImage, nDestWidth, nDestHeight, hd.x, nSourceHeight, v->nESParsePMTIndex[esparserinfo->nES], v->nESParseESIndex[esparserinfo->nES]);

		// Add a PARSEDMPEG4VIDEO structure to keep the size
		if (v->pat.pmt[v->nESParsePMTIndex[esparserinfo->nES]].es[v->nESParseESIndex[esparserinfo->nES]].pParsedData == NULL)
		{
			PPARSEDMPEG4VIDEO pMPEG4 = LocalAlloc(LPTR, sizeof(PARSEDMPEG4VIDEO));
			pMPEG4->horizontal_size_value = hd.x;
			pMPEG4->vertical_size_value = hd.y;
			v->pat.pmt[v->nESParsePMTIndex[esparserinfo->nES]].es[v->nESParseESIndex[esparserinfo->nES]].pParsedData = (BYTE *)pMPEG4;
		}
		if (v->fSaveThumbnails)
			DecoderThread_SaveThumbnail(NULL, esparserinfo->nES, hd.x, hd.y, pImage);
		if (v->fArchiveRunning)
			SaveArchiveThumbnail(NULL, esparserinfo->nES);
		if (v->hWndVideoMosaic != NULL)
			InvalidateRect(v->hWndVideoMosaic, NULL, FALSE);

		LocalFree(pImage);
	}
	else
	{
		LoadVideoDecoderCrashThumbnail(v->nESParsePMTIndex[esparserinfo->nES], v->nESParseESIndex[esparserinfo->nES]);
	}

	LocalFree(hd.pY);
	LocalFree(hd.pU);
	LocalFree(hd.pV);
	FreeLibrary(hDLL);
windup_MPEG4_decode:
	CloseHandle(v->hMPEGDecoderReadPipe[esparserinfo->nES]);
	CloseHandle(v->hMPEGDecoderWritePipe[esparserinfo->nES]);
	EnterCriticalSection(&v->esparserinfo[esparserinfo->nES].csThreadSignal);
	v->fESParseDecoderCompletedLibMPEG[esparserinfo->nES] = TRUE;
	v->fMPEG2DecoderThreadRunning[esparserinfo->nES] = FALSE;
	LeaveCriticalSection(&v->esparserinfo[esparserinfo->nES].csThreadSignal);
	OutputDebugString("TSReader: MPEG4DecoderThread-\n");

	return 0;	
}

/*
int ReadFromMPEG4ESPipe(BYTE * pBuffer, int nLength)
{
	int nRequestedLength = nLength;
	DWORD dwRead;

	while (nLength)
	{
		ReadFile(v->hMPEGDecoderReadPipe[nES], pBuffer, nLength, &dwRead, NULL);
		if (dwRead == 0)
			return 0;
		pBuffer += dwRead;
		nLength -= dwRead;
	}

	return nRequestedLength;
}

DWORD WINAPI MPEG4DecoderThread(LPVOID lpv)
{
	HANDLE hDebug = INVALID_HANDLE_VALUE;
	DWORD dwTotalWritten = 0;
	char szDebugName[MAX_PATH];

	OutputDebugString("TSReader: DEBUG: MPEG4DecoderThread+\n");
	wsprintf(szDebugName, "c:\\MPEG-ES\\%08x.m4v", GetTickCount());
	hDebug = CreateFile(szDebugName, GENERIC_WRITE, 0, (LPSECURITY_ATTRIBUTES) NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, (HANDLE) NULL);
	do
	{
		int nRead;
		DWORD dwWritten;
		BYTE buffer[1024];
		nRead = ReadFromMPEG4ESPipe(buffer, 1024);
		if (nRead != 1024)
			break;
		WriteFile(hDebug, buffer, nRead, &dwWritten, NULL);
		dwTotalWritten += dwWritten;
	} while (dwTotalWritten < 1024 * 1024);
	
	CloseHandle(hDebug);
	CloseHandle(v->hMPEGDecoderReadPipe[esparserinfo->nES]);
	CloseHandle(v->hMPEGDecoderWritePipe[esparserinfo->nES]);
	v->fESParseDecoderCompletedLibMPEG = TRUE;
	v->fMPEG2DecoderThreadRunning = FALSE;
	OutputDebugString("TSReader: DEBUG: MPEG4DecoderThread-\n");
	
	return 0;
}
*/
