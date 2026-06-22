#include <windows.h>
#include <commctrl.h>
#include "TSReader.h"
#include "resource.h"
#include "util.h"
#include "bcdmux.h"
#include "parser.h"

extern PVARIABLES v;

typedef struct _tagVC1Decode
{
	unsigned char * pY;
	unsigned char * pU;
	unsigned char * pV;
	int x, y;
	int interlaced;
} VC1DECODE, *PVC1DECODE;

typedef int (* td_VC1) (HANDLE hInPipe, PVC1DECODE hd);

/*DWORD WINAPI VC1DecoderThread(LPVOID lpv)
{
	PESPARSERINFO esparserinfo = (PESPARSERINFO)lpv;
	VC1DECODE hd = {0};
	HMODULE hDLL;
	int (* VC1) (HANDLE hInPipe, PVC1DECODE hd);

	v->fMPEG2DecoderThreadRunning[esparserinfo->nES] = TRUE;

	{
		char szTemp[128];
		wsprintf(szTemp, "TSReader: VC1DecoderThread+ %d/%d\n", esparserinfo->nProgramNumber, esparserinfo->nES);
		OutputDebugString(szTemp);
	}

	hDLL = LoadLibrary("TSReader_VC1.dll");
	if (hDLL == NULL)
	{
		OutputDebugString("TSReader: VC1DecoderThread: Unable to load TSReader_VC1.dll\n");
		goto windup_VC1_decode;
	}
	VC1 = (td_VC1)GetProcAddress(hDLL, "VC1");

	hd.pY = LocalAlloc(LMEM_FIXED, 1920 * 1088);
	hd.pU = LocalAlloc(LMEM_FIXED, (1920 * 1088) / 2);
	hd.pV = LocalAlloc(LMEM_FIXED, (1920 * 1088) / 2);
	
	hd.x = hd.y = 0;
//	__try
//	{
		VC1(v->hMPEGDecoderReadPipe[esparserinfo->nES], &hd);
//	} __except(EXCEPTION_EXECUTE_HANDLER )
//	{
//		// VC1 decoder crashed on us - so we put up a decoder crash picture
//		hd.x = hd.y = 0;
//	}
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

		// Add a PARSEDVC1VIDEO structure to keep the size
		if (v->pat.pmt[v->nESParsePMTIndex[esparserinfo->nES]].es[v->nESParseESIndex[esparserinfo->nES]].pParsedData == NULL)
		{
			PPARSEDVC1VIDEO pVC1 = LocalAlloc(LPTR, sizeof(PARSEDVC1VIDEO));
			pVC1->horizontal_size_value = hd.x;
			pVC1->vertical_size_value = hd.y;
			pVC1->interlaced = hd.interlaced;
			v->pat.pmt[v->nESParsePMTIndex[esparserinfo->nES]].es[v->nESParseESIndex[esparserinfo->nES]].pParsedData = (BYTE *)pVC1;
		}

		LocalFree(pImage);	
	}
	else
	{
		LoadVideoDecoderCrashThumbnail(v->nESParsePMTIndex[esparserinfo->nES], v->nESParseESIndex[esparserinfo->nES]);
	}

	LocalFree(hd.pY);
	LocalFree(hd.pU);
	LocalFree(hd.pV);
	CloseHandle(v->hMPEGDecoderReadPipe[esparserinfo->nES]);
	CloseHandle(v->hMPEGDecoderWritePipe[esparserinfo->nES]);
	FreeLibrary(hDLL);
windup_VC1_decode:
	v->fESParseDecoderCompletedLibMPEG[esparserinfo->nES] = TRUE;
	v->fMPEG2DecoderThreadRunning[esparserinfo->nES] = FALSE;
	{
		char szTemp[128];
		wsprintf(szTemp, "TSReader: VC1DecoderThread- %d/%d\n", esparserinfo->nProgramNumber, esparserinfo->nES);
		OutputDebugString(szTemp);
	}

	return 0;	
}
*/

int ReadFromVC1ESPipe(BYTE * pBuffer, int nLength, int nES)
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

//DWORD WINAPI VC1DecoderThread_DEBUG(LPVOID lpv)
DWORD WINAPI VC1DecoderThread(LPVOID lpv)
{
	PESPARSERINFO esparserinfo = (PESPARSERINFO)lpv;
	HANDLE hVC1File;
	HANDLE hOptionsFile;
	DWORD dwTickCount;
	DWORD dwWritten;
	BOOL fRetVal;
	int nTimeout;
	VC1DECODE hd = {0};
	PROCESS_INFORMATION ProcessInformation;
	STARTUPINFO StartupInfo;

	char szVC1ESFile[MAX_PATH];
	char szOptionsFile[MAX_PATH];
	char szYUVFile[MAX_PATH];
	char szTextOutputFile[MAX_PATH];
	char szDecoderName[MAX_PATH];
	char szExecutable[MAX_PATH];
	char szTemp[MAX_PATH * 3];
	char szCommandLine[MAX_PATH * 3];

	OutputDebugString("TSReader: VC1DecoderThread+\n");
	EnterCriticalSection(&v->esparserinfo[esparserinfo->nES].csThreadSignal);
	v->fMPEG2DecoderThreadRunning[esparserinfo->nES] = TRUE;
	LeaveCriticalSection(&v->esparserinfo[esparserinfo->nES].csThreadSignal);
	dwTickCount = GetTickCount();
	hd.x = hd.y = 0;

	// Come up with filenames
	SourceHelper_GetTSReaderEXEDirectory(v->hInstance, szVC1ESFile, sizeof(szVC1ESFile));
	lstrcat(szVC1ESFile, "\\");
	lstrcpy(szOptionsFile, szVC1ESFile);
	lstrcpy(szYUVFile, szVC1ESFile);
	lstrcpy(szTextOutputFile, szVC1ESFile);
	lstrcpy(szDecoderName, szVC1ESFile);

	wsprintf(szTemp, "%05d%08x.vc1", esparserinfo->nProgramNumber, dwTickCount);
	lstrcat(szVC1ESFile, szTemp);
	wsprintf(szTemp, "%05d%08x.txt", esparserinfo->nProgramNumber, dwTickCount);
	lstrcat(szOptionsFile, szTemp);
	wsprintf(szTemp, "%05d%08x.yuv", esparserinfo->nProgramNumber, dwTickCount);
	lstrcat(szYUVFile, szTemp);
	wsprintf(szTemp, "%05d%08x.out.txt", esparserinfo->nProgramNumber, dwTickCount);
	lstrcat(szTextOutputFile, szTemp);
	lstrcat(szDecoderName, "vc1-decoder.exe");

	// Create a second snapshot of the ES
	hVC1File = CreateFile(szVC1ESFile, GENERIC_WRITE, 0, (LPSECURITY_ATTRIBUTES) NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, (HANDLE) NULL);
	do
	{
		int nRead;
		BYTE buffer[1024];

		nRead = ReadFromVC1ESPipe(buffer, 1024, esparserinfo->nES);
		if (nRead != 1024)
			break;
		WriteFile(hVC1File, buffer, nRead, &dwWritten, NULL);
	} while (GetTickCount() - dwTickCount < 500);	
	CloseHandle(hVC1File);

	// Make the options.txt file
	hOptionsFile = CreateFile(szOptionsFile, GENERIC_WRITE, 0, (LPSECURITY_ATTRIBUTES) NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, (HANDLE) NULL);
	wsprintf(szTemp, "Bitstream File : %s\r\nOutput YUV : %s\r\nDebugMask : 0x0000000f\r\n",
		     szVC1ESFile, szYUVFile);
	WriteFile(hOptionsFile, szTemp, lstrlen(szTemp), &dwWritten, NULL);
	CloseHandle(hOptionsFile);

	// Run the decoder
	memset(&StartupInfo, 0, sizeof(StartupInfo));
	StartupInfo.cb = sizeof(STARTUPINFO);
	lstrcpy(szCommandLine, "/C \" \"");
	lstrcat(szCommandLine, szDecoderName);
	lstrcat(szCommandLine, "\" \"");
	lstrcat(szCommandLine, szOptionsFile);
	lstrcat(szCommandLine, "\" > \"");
	lstrcat(szCommandLine, szTextOutputFile);
	lstrcat(szCommandLine, "\" \"");

	GetSystemDirectory(szExecutable, sizeof(szExecutable));
	lstrcat(szExecutable, "\\cmd.exe");
	{
		char szDebugTemp[MAX_PATH * 2];
		wsprintf(szDebugTemp, "VC1: Starting decoder %s %s\n", szExecutable, szCommandLine);
		OutputDebugString(szDebugTemp);
	}
	fRetVal = CreateProcess(szExecutable, szCommandLine, NULL, NULL, FALSE, CREATE_NO_WINDOW | CREATE_NEW_PROCESS_GROUP, NULL, NULL, &StartupInfo, &ProcessInformation);
	if (!fRetVal)
	{
		char szDebugTemp[128];
		wsprintf(szDebugTemp, "VC1: CreateProcess failed with %d\n", GetLastError());
		OutputDebugString(szDebugTemp);
	}
	Sleep(100);

	// Now try to read the result file
	nTimeout = 50;	// 5 seconds
	while (nTimeout--)
	{
		int nTextOutputFileSize;
		BYTE * pTextOutputFile;
		DWORD dwRead;
		char * szSearch;

		HANDLE hTextOutputFile = CreateFile(szTextOutputFile, GENERIC_READ, 0, (LPSECURITY_ATTRIBUTES) NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, (HANDLE) NULL);
		if (hTextOutputFile == INVALID_HANDLE_VALUE)
		{
			Sleep(100);
			continue;
		}
		nTextOutputFileSize = GetFileSize(hTextOutputFile, NULL);
		pTextOutputFile = LocalAlloc(LPTR, nTextOutputFileSize);
		ReadFile(hTextOutputFile, pTextOutputFile, nTextOutputFileSize, &dwRead, NULL);
		CloseHandle(hTextOutputFile);

		// Get size
		szSearch = strstr((const char *)pTextOutputFile, "Width = ");
		if (szSearch != NULL)
			hd.x = atoi(&szSearch[8]);
		szSearch = strstr((const char *)pTextOutputFile, "Height = ");
		if (szSearch != NULL)
			hd.y = atoi(&szSearch[9]);
		szSearch = strstr((const char *)pTextOutputFile, "INTERLACE: ");
		if (szSearch != NULL)
			hd.interlaced = atoi(&szSearch[11]);
		LocalFree(pTextOutputFile);

		/*{
			char szTemp[MAX_PATH];
			wsprintf(szTemp, "VC1: Finished parsing %s\n", szTextOutputFile);
			OutputDebugString(szTemp);
		}*/
		break;
	}

	if (hd.x && hd.y)
	{
		BYTE * pImage = LocalAlloc(LPTR, hd.x * hd.y * 3);
		int nDestHeight, nDestWidth;
		int nSourceHeight = hd.y;
		int i;
		DWORD dwRead;
		HANDLE hYUVFile = CreateFile(szYUVFile, GENERIC_READ, 0, (LPSECURITY_ATTRIBUTES) NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, (HANDLE) NULL);

		hd.pY = LocalAlloc(LMEM_FIXED, 1920 * 1088);
		hd.pU = LocalAlloc(LMEM_FIXED, (1920 * 1088) / 2);
		hd.pV = LocalAlloc(LMEM_FIXED, (1920 * 1088) / 2);

		// Use the second frame in the YUV file - first one is often corrupt
		for (i = 0; i < 2; i++)
		{
			ReadFile(hYUVFile, hd.pY, hd.x * hd.y, &dwRead, NULL);
			ReadFile(hYUVFile, hd.pU, (hd.x / 2) * (hd.y / 2), &dwRead, NULL);
			ReadFile(hYUVFile, hd.pV, (hd.x / 2) * (hd.y / 2), &dwRead, NULL);
		}
		CloseHandle(hYUVFile);

		YUVtoRGB(pImage, hd.pY, hd.pU, hd.pV, hd.x, hd.y); 
		GetNewThumbnailSize(&nSourceHeight, &nDestHeight, &nDestWidth);
		GenerateSizedThumbnail(pImage, nDestWidth, nDestHeight, hd.x, nSourceHeight,
			                   v->nESParsePMTIndex[esparserinfo->nES], v->nESParseESIndex[esparserinfo->nES]);

		// Add a PARSEDVC1VIDEO structure to keep the size
		if (v->pat.pmt[v->nESParsePMTIndex[esparserinfo->nES]].es[v->nESParseESIndex[esparserinfo->nES]].pParsedData == NULL)
		{
			PPARSEDVC1VIDEO pVC1 = LocalAlloc(LPTR, sizeof(PARSEDVC1VIDEO));
			pVC1->horizontal_size_value = hd.x;
			pVC1->vertical_size_value = hd.y;
			pVC1->interlaced = hd.interlaced;
			v->pat.pmt[v->nESParsePMTIndex[esparserinfo->nES]].es[v->nESParseESIndex[esparserinfo->nES]].pParsedData = (BYTE *)pVC1;
		}
		if (v->fSaveThumbnails)
			DecoderThread_SaveThumbnail(NULL, esparserinfo->nES, hd.x, hd.y, pImage);
		if (v->fArchiveRunning)
			SaveArchiveThumbnail(NULL, esparserinfo->nES);
		if (v->hWndVideoMosaic != NULL)
			InvalidateRect(v->hWndVideoMosaic, NULL, FALSE);

		LocalFree(pImage);
		LocalFree(hd.pY);
		LocalFree(hd.pU);
		LocalFree(hd.pV);
	}
	else
	{
		LoadVideoDecoderCrashThumbnail(v->nESParsePMTIndex[esparserinfo->nES], v->nESParseESIndex[esparserinfo->nES]);
	}

	// Remove Files
	DeleteFile(szVC1ESFile);
	DeleteFile(szOptionsFile);
	DeleteFile(szYUVFile);
	DeleteFile(szTextOutputFile);

	// All done with the thread
	CloseHandle(v->hMPEGDecoderReadPipe[esparserinfo->nES]);
	CloseHandle(v->hMPEGDecoderWritePipe[esparserinfo->nES]);
	
	EnterCriticalSection(&v->esparserinfo[esparserinfo->nES].csThreadSignal);
	v->fESParseDecoderCompletedLibMPEG[esparserinfo->nES] = TRUE;
	v->fMPEG2DecoderThreadRunning[esparserinfo->nES] = FALSE;
	LeaveCriticalSection(&v->esparserinfo[esparserinfo->nES].csThreadSignal);
	
	OutputDebugString("TSReader: VC1DecoderThread-\n");
	
	return 0;
}
