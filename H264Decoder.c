#include <windows.h>
#include <commctrl.h>
#include "TSReader.h"
#include "resource.h"
#include "util.h"
#include "bcdmux.h"
#include "parser.h"

extern PVARIABLES v;

typedef struct _tagH264Decode
{
	unsigned char * pY;
	unsigned char * pU;
	unsigned char * pV;
	int x, y;
	int interlaced;
	int maximum_pictures;
	HWND hWndST;
	td_QuickParseUserData UserFunction;
} H264DECODE, *PH264DECODE;

typedef int (* td_H264) (HANDLE hInPipe, PH264DECODE hd);

//DWORD WINAPI H264DecoderThread_DEBUG(LPVOID lpv)
DWORD WINAPI H264DecoderThread(LPVOID lpv)
{
	PESPARSERINFO esparserinfo = (PESPARSERINFO)lpv;
	H264DECODE hd = {0};
	HMODULE hDLL;
	td_H264 H264 = NULL;

	char szTemp[128];
	wsprintf(szTemp, "TSReader: H264DecoderThread+ %d/%d\n", esparserinfo->nProgramNumber, esparserinfo->nES);
	OutputDebugString(szTemp);

	hDLL = LoadLibrary("TSReader_ForVid.dll");
	if (hDLL == NULL)
	{
		OutputDebugString("TSReader: H264DecoderThread: Unable to load H.264 decoder DLL (TSReader_ForVid.dll)\n");
		goto windup_h264_decode;
	}
	H264 = (td_H264)GetProcAddress(hDLL, "ForVid");

	hd.pY = LocalAlloc(LMEM_FIXED, 1920 * 1088);
	hd.pU = LocalAlloc(LMEM_FIXED, (1920 * 1088) / 2);
	hd.pV = LocalAlloc(LMEM_FIXED, (1920 * 1088) / 2);
	hd.hWndST = v->hWndST;
	hd.UserFunction = QuickParseUserData;
	hd.maximum_pictures = v->nMaximumH264Pictures;
	
	EnterCriticalSection(&v->esparserinfo[esparserinfo->nES].csThreadSignal);
	v->fMPEG2DecoderThreadRunning[esparserinfo->nES] = TRUE;
	LeaveCriticalSection(&v->esparserinfo[esparserinfo->nES].csThreadSignal);
	
	__try
	{
		H264(v->hMPEGDecoderReadPipe[esparserinfo->nES], &hd);
	} __except(EXCEPTION_EXECUTE_HANDLER )
	{
		// H.264 decoder crashed on us - so we put up a decoder crash picture
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
		GenerateSizedThumbnail(pImage, nDestWidth, nDestHeight, hd.x, nSourceHeight,
			                   v->nESParsePMTIndex[esparserinfo->nES], v->nESParseESIndex[esparserinfo->nES]);

		// Add a PARSEDH264VIDEO structure to keep the size
		if (v->pat.pmt[v->nESParsePMTIndex[esparserinfo->nES]].es[v->nESParseESIndex[esparserinfo->nES]].pParsedData == NULL)
		{
			PPARSEDH264VIDEO pH264 = LocalAlloc(LPTR, sizeof(PARSEDH264VIDEO));
			pH264->horizontal_size_value = hd.x;
			pH264->vertical_size_value = hd.y;
			pH264->interlaced = hd.interlaced;
			v->pat.pmt[v->nESParsePMTIndex[esparserinfo->nES]].es[v->nESParseESIndex[esparserinfo->nES]].pParsedData = (BYTE *)pH264;
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
	if (!(v->pat.pmt[v->nESParsePMTIndex[esparserinfo->nES]].es[v->nESParseESIndex[esparserinfo->nES]].nBlacklisted))
		CloseHandle(v->hMPEGDecoderReadPipe[esparserinfo->nES]);
	CloseHandle(v->hMPEGDecoderWritePipe[esparserinfo->nES]);
	FreeLibrary(hDLL);
windup_h264_decode:
	EnterCriticalSection(&v->esparserinfo[esparserinfo->nES].csThreadSignal);
	v->fESParseDecoderCompletedLibMPEG[esparserinfo->nES] = TRUE;
	v->fMPEG2DecoderThreadRunning[esparserinfo->nES] = FALSE;
	LeaveCriticalSection(&v->esparserinfo[esparserinfo->nES].csThreadSignal);

	wsprintf(szTemp, "TSReader: H264DecoderThread- %d/%d\n", esparserinfo->nProgramNumber, esparserinfo->nES);
	OutputDebugString(szTemp);

	return 0;	
}

#ifdef _DEBUG
int ReadFromH264ESPipe(BYTE * pBuffer, int nLength, int nES)
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

DWORD WINAPI H264DecoderThread_DEBUG(LPVOID lpv)
//DWORD WINAPI H264DecoderThread(LPVOID lpv)
{
	PESPARSERINFO esparserinfo = (PESPARSERINFO)lpv;
	HANDLE hDebug = INVALID_HANDLE_VALUE;
	char szDebugName[MAX_PATH];

	OutputDebugString("TSReader: DEBUG: H264DecoderThread+\n");
	wsprintf(szDebugName, "c:\\MPEG-ES\\%08x.h264", GetTickCount());
	hDebug = CreateFile(szDebugName, GENERIC_WRITE, 0, (LPSECURITY_ATTRIBUTES) NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, (HANDLE) NULL);
	do
	{
		int nRead;
		DWORD dwWritten;
		BYTE buffer[1024];
		nRead = ReadFromH264ESPipe(buffer, 1024, esparserinfo->nES);
		if (nRead != 1024)
			break;
		WriteFile(hDebug, buffer, nRead, &dwWritten, NULL);
	} while (TRUE);
	
	CloseHandle(hDebug);
	CloseHandle(v->hMPEGDecoderReadPipe[esparserinfo->nES]);
	CloseHandle(v->hMPEGDecoderWritePipe[esparserinfo->nES]);
	
	EnterCriticalSection(&v->esparserinfo[esparserinfo->nES].csThreadSignal);	
	v->fESParseDecoderCompletedLibMPEG[esparserinfo->nES] = TRUE;
	v->fMPEG2DecoderThreadRunning[esparserinfo->nES] = FALSE;
	LeaveCriticalSection(&v->esparserinfo[esparserinfo->nES].csThreadSignal);

	OutputDebugString("TSReader: DEBUG: H264DecoderThread-\n");
	
	return 0;
}
#endif _DEBUG
