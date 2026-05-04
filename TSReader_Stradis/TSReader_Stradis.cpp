#include <windows.h>
#include <commctrl.h>
#include "../TSReader.h"

PVARIABLES v;
BOOL fStradisThreadDone;
#define STRADIS_READ_SIZE 188 * 51

#ifndef CSDK

/// Old 1.2 C++ API

#include "StradisDecoder.h"
CStradisDecoder* pDecoder = NULL;
CStradisDecoderSettings* pSettings = NULL;

DWORD WINAPI StradisDataThread(LPVOID lpv)
{
	DWORD dwRead;
	BYTE * buffer = (BYTE *)LocalAlloc(LPTR, STRADIS_READ_SIZE);
	HANDLE hRecordFile = INVALID_HANDLE_VALUE;
	DWORD dwBStotal = pDecoder->RateBuffer_GetBufferSize(CStradisDecoder::BUFFER_CURRENT);
	char szTemp[128];
	
	OutputDebugString("+StradisDataThread\n");
	do
	{
		DWORD dwOffset = 0;
		DWORD dwBS = pDecoder->RateBuffer_GetCombinedFullness();
		if (dwBS + STRADIS_READ_SIZE * 4 < dwBStotal)
		//if (TRUE)
		{
			int nPipeReturn;

			// make sure it fits in the demux buffer
			if (ReadFile(v->hStradisReadPipe, buffer, STRADIS_READ_SIZE, &dwRead, NULL) == FALSE)
			{
				OutputDebugString("ReadStradis: ReadFile for pipe failed\n");
				break;
			}
			EnterCriticalSection(&v->csPipeBytes);
			v->nPipeBytes -= dwRead;
			LeaveCriticalSection(&v->csPipeBytes);

			nPipeReturn = pDecoder->Demux_Transmission(CStradisDecoder::BUFFER_CURRENT, buffer, dwRead);
			if (nPipeReturn)
			{
				Sleep(5);
				continue;

				/*// The API has indicated that we must restart the decoder
				pDecoder->Demux_Command(CStradisDecoder::DEMUX_STOP, 0);
				pDecoder->Demux_Init(pSettings);
				pDecoder->Demux_Setup(CStradisDecoder::BUFFER_CURRENT, pSettings);
				continue;*/
			}
		}
		else
		{
			Sleep(1);
		}
	} while (v->fStradisActive);

	LocalFree(buffer);
	wsprintf(szTemp, "-StradisDataThread. v->fStradisActive = %d\n", v->fStradisActive);
	OutputDebugString(szTemp);
	fStradisThreadDone = TRUE;
	return 0;
}

BOOL SetupStradis(PVARIABLES pv)
{
	HANDLE hThread;
	DWORD dwThreadID;
	char szTemp[128];

	v = pv;		// save for us
	fStradisThreadDone = FALSE;

	// Initialize the Transmission System here
	pDecoder = new CStradisDecoder(0);
	if (pDecoder)
	{
		int i, nAudioPIDIndex;

		if (v->fStradisPAL == TRUE || v->fForceStradisPAL == TRUE)
			pSettings = new CStradisDecoderSettings(CStradisDecoder::DENC_PAL);
		else
			pSettings = new CStradisDecoderSettings(CStradisDecoder::DENC_NTSC);

		if (pDecoder->Demux_Init(pSettings))
		{
			OutputDebugString("Stradis: Demux_Init failed\n");
			goto ErrorExit;
		}

		pDecoder->ClosedCaption_Enable(CStradisDecoder::CC_ENABLE_OUTPUT|CStradisDecoder::CC_ENABLE_PASS_TO_OUTPUT);

		pSettings->m_StreamType = CStradisDecoder::TRANSPORT_STREAM;
		nAudioPIDIndex = 0;
		for (i = 0; i < MAX_AUDIO_STREAMS && i < 4; i++)
		{
			if (v->nRecordAudioPID[i] == 0x1fff)
				break;		
			pSettings->m_pAudioPid[nAudioPIDIndex++] = v->nRecordAudioPID[i];
		}
		pSettings->m_nVideoPid = v->nRecordVideoPID;		// Fill in the video PID of the transport stream
		pSettings->m_nClockPid = v->nRecordPCRPID;		// Fill in the clock pid of the transport stream

		wsprintf(szTemp, "Stradis: VPID = 0x%04x APID = 0x%04x PPID = 0x%04x\n",
			pSettings->m_nVideoPid, pSettings->m_pAudioPid[0], pSettings->m_nClockPid);
		OutputDebugString(szTemp);

		pSettings->m_bClockRecover = true;	// Do clock recovery from the clock PID
		pSettings->m_dwVBV = 400000;		// Set this to the appropriate amount of data to collect before starting the decoder
											// This amount will vary depending on the video bit-rate
		pSettings->m_dwCollectTime = 500;
		pSettings->m_bShiftData = true;

		// Set up the current buffer
		if (pDecoder->Demux_Setup(CStradisDecoder::BUFFER_CURRENT, pSettings) == -1)
		{
			OutputDebugString("Stradis: Demux_Setup failed\n");
			goto ErrorExit;
		}

		// Get a thread going to read the data written by the main
		// parser thread
		hThread = CreateThread(NULL, 0, StradisDataThread, (LPVOID)0, 0, &dwThreadID);
		SetThreadPriority(hThread, THREAD_PRIORITY_HIGHEST);
		CloseHandle(hThread);
	}

ErrorExit:				
	return TRUE;
}

void ShutdownStradis()
{
	int nCount = 0;

	do
	{
		if (fStradisThreadDone == TRUE)
			break;
		Sleep(50);
	} while (nCount++ < 30);

	if (pDecoder->RateBuffer_GetState() == CStradisDecoder::RATEBUFFER_PLAY)
	{
		pDecoder->Demux_SetEndOfStream();
		pDecoder->Demux_Complete();
		pDecoder->RateBuffer_WaitForEndEvent();
	}
	else
	{
		pDecoder->Demux_Command(CStradisDecoder::DEMUX_STOP, 0);
	}

	Sleep(100);

	if (pSettings) delete pSettings;
	if (pDecoder) delete pDecoder;
}

#else CSDK

#include "StradisDecoderC.h"

STRADISDECODERSETTINGS * phSettings;
STRADISDECODER hDecoder;

DWORD WINAPI StradisDataThread(LPVOID lpv)
{
	DWORD dwRead;
	BYTE * buffer = (BYTE *)LocalAlloc(LPTR, STRADIS_READ_SIZE);
	HANDLE hRecordFile = INVALID_HANDLE_VALUE;
	DWORD dwBStotal = StradisDecoder_RateBuffer_GetBufferSize(hDecoder, BUFFER_CURRENT);
	char szTemp[128];
	
	OutputDebugString("+StradisDataThread\n");
	do
	{
		DWORD dwOffset = 0;
		DWORD dwBS = StradisDecoder_RateBuffer_GetCombinedFullness(hDecoder);
		if (dwBS + STRADIS_READ_SIZE * 4 < dwBStotal)
		//if (TRUE)
		{
			int nPipeReturn;

			// make sure it fits in the demux buffer
			if (ReadFile(v->hStradisReadPipe, buffer, STRADIS_READ_SIZE, &dwRead, NULL) == FALSE)
			{
				OutputDebugString("ReadStradis: ReadFile for pipe failed\n");
				break;
			}
			EnterCriticalSection(&v->csPipeBytes);
			v->nPipeBytes -= dwRead;
			LeaveCriticalSection(&v->csPipeBytes);
			nPipeReturn = StradisDecoder_Demux_Transmission(hDecoder, BUFFER_CURRENT, buffer, dwRead);
			if (nPipeReturn)
			{
				Sleep(5);
				continue;

				/*// The API has indicated that we must restart the decoder
				pDecoder->Demux_Command(CStradisDecoder::DEMUX_STOP, 0);
				pDecoder->Demux_Init(pSettings);
				pDecoder->Demux_Setup(CStradisDecoder::BUFFER_CURRENT, pSettings);
				continue;*/
			}
		}
		else
		{
			Sleep(1);
		}
	} while (v->fStradisActive);

	LocalFree(buffer);
	wsprintf(szTemp, "-StradisDataThread. v->fStradisActive = %d\n", v->fStradisActive);
	OutputDebugString(szTemp);
	fStradisThreadDone = TRUE;
	return 0;
}

BOOL SetupStradis(PVARIABLES pv)
{
	HANDLE hThread;
	DWORD dwThreadID;

	v = pv;		// save for us
	fStradisThreadDone = FALSE;

	hDecoder = StradisDecoder_Create(0);
	if (hDecoder != INVALID_STRADIS_HANDLE)
	{
		int i, nAudioPIDIndex;

		phSettings = (STRADISDECODERSETTINGS * )malloc(sizeof(STRADISDECODERSETTINGS));
		if (v->fStradisPAL == TRUE || v->fForceStradisPAL == TRUE)
			*phSettings = StradisDecoderSettings_Create(DENC_PAL);
		else
			*phSettings = StradisDecoderSettings_Create(DENC_NTSC);
		if (StradisDecoder_Demux_Init(hDecoder, *phSettings))
		{
			OutputDebugString("Stradis: Demux_Init failed\n");
			goto ErrorExit;
		}

		StradisDecoder_ClosedCaption_Enable(hDecoder, CC_ENABLE_OUTPUT | CC_ENABLE_PASS_TO_OUTPUT);
		StradisDecoderSettings_Set_m_StreamType(*phSettings, TRANSPORT_STREAM);

		nAudioPIDIndex = 0;
		for (i = 0; i < MAX_AUDIO_STREAMS && i < 4; i++)
		{
			if (v->nRecordAudioPID[i] == 0x1fff)
				break;		
			StradisDecoderSettings_Set_m_pAudioPid(*phSettings, nAudioPIDIndex++, v->nRecordAudioPID[i]);
		}
		StradisDecoderSettings_Set_m_nVideoPid(*phSettings, v->nRecordVideoPID);
		StradisDecoderSettings_Set_m_nClockPid(*phSettings, v->nRecordPCRPID);
		StradisDecoderSettings_Set_m_bClockRecover(*phSettings, TRUE);
		StradisDecoderSettings_Set_m_dwVBV(*phSettings, 400000);
		StradisDecoderSettings_Set_m_dwCollectTime(*phSettings, 500);
		StradisDecoderSettings_Set_m_bShiftData(*phSettings, TRUE);

		// Set up the current buffer
		if (StradisDecoder_Demux_Setup(hDecoder, BUFFER_CURRENT, *phSettings) == -1)
		{
			OutputDebugString("Stradis: Demux_Setup failed\n");
			goto ErrorExit;
		}

		// Get a thread going to read the data written by the main
		// parser thread
		hThread = CreateThread(NULL, 0, StradisDataThread, (LPVOID)0, 0, &dwThreadID);
		SetThreadPriority(hThread, THREAD_PRIORITY_HIGHEST);
		CloseHandle(hThread);

	}

ErrorExit:				
	return TRUE;
}

void ShutdownStradis()
{
	int nCount = 0;

	do
	{
		if (fStradisThreadDone == TRUE)
			break;
		Sleep(50);
	} while (nCount++ < 30);

	if (StradisDecoder_RateBuffer_GetState(hDecoder) == RATEBUFFER_PLAY)
	{
		StradisDecoder_Demux_SetEndOfStream(hDecoder);
		StradisDecoder_Demux_Complete(hDecoder);
		StradisDecoder_RateBuffer_WaitForEndEvent(hDecoder, 2000);
	}
	else
	{
		StradisDecoder_Demux_Command(hDecoder, DEMUX_STOP, 0);
	}

	Sleep(100);

	StradisDecoderSettings_Delete(*phSettings);
	StradisDecoder_Delete(hDecoder);
	free(phSettings);
}
#endif CSDK