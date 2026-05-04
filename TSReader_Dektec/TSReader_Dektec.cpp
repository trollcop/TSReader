#include <windows.h>
#include <commctrl.h>
#include "initguid.h"
#include "winioctl.h"
#include "setupapi.h"	// VC++ 5 one is out of date
#include <stdio.h>

#include "..\sources.h"
#include "DTAPI.h"

//#define RX_BUFFER_PACKET_COUNT 348
#define RX_BUFFER_PACKET_COUNT 697
#define RX_BUFFER_SIZE (RX_BUFFER_PACKET_COUNT * nPacketLength)
PSOURCESTRUCT ss;
HANDLE hInstance;
char * szCmdLinePtr;

BOOL fNeedTuneDialog = TRUE;
int nFrequency;
#ifndef DTU234
int nPolarity;
int nSymbolRate;
int nLNBFrequency;
int n22KHz;
int nDiSEqCInput;
int nADVModulationMode;
int nCodeRate;
int nFIFOOverrunCounter;
BOOL fTimestampPackets = FALSE;
#else DTU234
HMODULE hSencoreDLL;
BOOL fBoard1049InitDone = FALSE;
CRITICAL_SECTION csSignal;

typedef BOOL (__stdcall * td_B1049DoesExist) (int nCard);
typedef BOOL (__stdcall * td_B1049Initialize) (int nCard);
typedef BOOL (__stdcall * td_B1049Uninitialize) (int nCard);
typedef int  (__stdcall * td_B1049SetTunerFrequencyHz) (int nCard, int nFrequency);
typedef int  (__stdcall * td_B1049GetTunerFrequencyHz) (int nCard);
typedef BOOL (__stdcall * td_B1049DemodSetAcquire) (int nCard, short nMode);
typedef BOOL (__stdcall * td_B1049DemodGetStatus1) (int nCard);
typedef int  (__stdcall * td_B1049GetDemodStatusEstSNR) (int nCard);
typedef BOOL (__stdcall * td_B1049GetDemodStatusOLock) (int nCard);
typedef BOOL (__stdcall * td_B1049GetDemodStatusFLock) (int nCard);
typedef BOOL (__stdcall * td_B1049GetDemodStatusRLock) (int nCard);
typedef int  (__stdcall * td_B1049GetChannelLevel) (int nCard);
typedef void (__stdcall * td_B1049Versions) (int nCard, long FAR* Ocx, long FAR* Driver, long FAR* Xilinx); 

BOOL (__stdcall * B1049DoesExist) (int nCard);
BOOL (__stdcall * B1049Initialize) (int nCard);
BOOL (__stdcall * B1049Uninitialize) (int nCard);
int  (__stdcall * B1049SetTunerFrequencyHz) (int nCard, int nFrequency);
int  (__stdcall * B1049GetTunerFrequencyHz) (int nCard);
BOOL (__stdcall * B1049DemodSetAcquire) (int nCard, short nMode);
BOOL (__stdcall * B1049DemodGetStatus1) (int nCard);
int  (__stdcall * B1049GetDemodStatusEstSNR) (int nCard);
BOOL (__stdcall * B1049GetDemodStatusOLock) (int nCard);
BOOL (__stdcall * B1049GetDemodStatusFLock) (int nCard);
BOOL (__stdcall * B1049GetDemodStatusRLock) (int nCard);
int  (__stdcall * B1049GetChannelLevel) (int nCard);
void (__stdcall * B1049Versions) (int nCard, long FAR* Ocx, long FAR* Driver, long FAR* Xilinx); 
#endif DTU234

#ifdef DTU235
HMODULE hSencoreDLL;
BOOL fBoard1077InitDone = FALSE;
CRITICAL_SECTION csSignal;
BOOL fSpectrumInversion;
int nBandwidth;

typedef BOOL (__stdcall * td_B1077Initialize) (int CardIndex);
typedef BOOL (__stdcall * td_B1077Uninitialize) (int CardIndex);
typedef BOOL (__stdcall * td_B1077DoesExist) (int CardIndex);
typedef void (__stdcall * td_B1077SetChannelBandwidth) (int CardIndex, UCHAR mode);
typedef void (__stdcall * td_B1077SetChannelClassification) (int CardIndex, UCHAR mode);
typedef void (__stdcall * td_B1077SetChannelCoderate) (int CardIndex, UCHAR mode);
typedef void (__stdcall * td_B1077SetChannelConstellation) (int CardIndex, UCHAR mode);
typedef void (__stdcall * td_B1077SetChannelGuard) (int CardIndex, UCHAR mode);
typedef void (__stdcall * td_B1077SetChannelHierarchy) (int CardIndex, UCHAR mode);
typedef void (__stdcall * td_B1077SetChannelMirror) (int CardIndex, UCHAR mode);
typedef void (__stdcall * td_B1077SetChannelPriority) (int CardIndex, UCHAR mode);
typedef void (__stdcall * td_B1077SetFftMode) (int CardIndex, UCHAR mode);
typedef BOOL (__stdcall * td_B1077SetChannelParameters) (int CardIndex);
typedef void (__stdcall * td_B1077SetTunerFrequencyHz) (int CardIndex, long Frequency);
typedef double (__stdcall * td_B1077GetChannelLevel) (int CardIndex);
typedef int (__stdcall * td_B1077GetLockStatus) (int CardIndex);

BOOL (__stdcall * B1077Initialize) (int CardIndex);
BOOL (__stdcall * B1077Uninitialize) (int CardIndex);
BOOL (__stdcall * B1077DoesExist) (int CardIndex);
void (__stdcall * B1077SetChannelBandwidth) (int CardIndex, UCHAR mode);
void (__stdcall * B1077SetChannelClassification) (int CardIndex, UCHAR mode);
void (__stdcall * B1077SetChannelCoderate) (int CardIndex, UCHAR mode);
void (__stdcall * B1077SetChannelConstellation) (int CardIndex, UCHAR mode);
void (__stdcall * B1077SetChannelGuard) (int CardIndex, UCHAR mode);
void (__stdcall * B1077SetChannelHierarchy) (int CardIndex, UCHAR mode);
void (__stdcall * B1077SetChannelMirror) (int CardIndex, UCHAR mode);
void (__stdcall * B1077SetChannelPriority) (int CardIndex, UCHAR mode);
void (__stdcall * B1077SetFftMode) (int CardIndex, UCHAR mode);
BOOL (__stdcall * B1077SetChannelParameters) (int CardIndex);
void (__stdcall * B1077SetTunerFrequencyHz) (int CardIndex, long Frequency);
double (__stdcall * B1077GetChannelLevel) (int CardIndex);
int (__stdcall * B1077GetLockStatus) (int CardIndex);

#endif DTU235

BOOL fDontTune = FALSE;
char szLastTune[128] = {"n/a"};
char szLastSignalReport[128] = {"n/a"};        

#ifndef USB
char gszSourceName[] = {"Dektec ASI/SPI (PCI)"};
DtDevice theCard;
#else USB
#ifndef DTU234
#ifdef DTU235
 char gszSourceName[] = {"Sencore DTU-235"};
#else DTU235
char gszSourceName[] = {"Dektec DTU-225/245"};
#endif DTU235
#else DTU234
#ifndef QAM
 char gszSourceName[] = {"Sencore DTU-234 8VSB"};
#else QAM
 char gszSourceName[] = {"Sencore DTU-234 QAM"};
#endif QAM
#endif DTU234
DtDevice theCard;
#endif USB

TsInpChannel theInput;
int		g_nBoardType = 0;					// Type of board to connect to
int		g_nBoardNum = 1;					// Board number to connect to
char	g_szLastError[256];					// Last error message

extern "C" {
	BOOL __cdecl SourceHelper_ValidateSourceContainer(PSOURCESTRUCT pss);
}

void DebugTrace(char* szError)
{
#ifndef USB
	lstrcpy(g_szLastError, "Dektec: ");
#else USB
#ifndef DTU234
#ifdef DTU235
	lstrcpy(g_szLastError, "Sencore_DTU235: ");
#else DTU235
	lstrcpy(g_szLastError, "Dektec_USB: ");
#endif DTU235
#else DTU234
	lstrcpy(g_szLastError, "Sencore_DTU234: ");
#endif DTU234
#endif USB
	lstrcat(g_szLastError, szError);
	lstrcat(g_szLastError, "\n");
	OutputDebugString(g_szLastError);
}

bool AttachToBoard(PciCard& rCard, TsInpChannel& rInputCh)
{
	bool bResult = true;

#ifndef USB
	try
	{
		int i = 0;
		char szTemp[256];

		//.-.-.-.-.-.-.-.-.-.-.-.-.- Determine number of cards -.-.-.-.-.-.-.-.-.-.-.-.-.-
		int iNumOfFuncs = 64;
		DTAPI_RESULT  dr;
		DtHwFuncDesc HwFuncs[64];
//		DtapiHwFunc HwFuncs[64];

		if (DTAPI_OK != DtapiHwFuncScan(iNumOfFuncs, iNumOfFuncs, HwFuncs))
		//if ( DTAPI_OK != DtapiPciScan(iNumOfFuncs, iNumOfFuncs, HwFuncs) )
		{
			strcpy(szTemp, "DtapiPciScan failed");
			DebugTrace(szTemp);
			throw bResult;
		}
		
		//-.-.-.-.-.-.-.-.-.-.-.-.-.-.- Find the Nth DTA-XXX -.-.-.-.-.-.-.-.-.-.-.-.-.-.-
		int nBoard = 0;

		for (i=0; i<iNumOfFuncs; i++)
		{
			if (HwFuncs[i].m_StreamType == DTAPI_TS_OVER_IP)
				continue;

			if (g_nBoardType==0 && (  HwFuncs[i].m_DvcDesc.m_TypeNumber == 120
								   || HwFuncs[i].m_DvcDesc.m_TypeNumber == 122
								   || HwFuncs[i].m_DvcDesc.m_TypeNumber == 124
								   || HwFuncs[i].m_DvcDesc.m_TypeNumber == 140
								   || HwFuncs[i].m_DvcDesc.m_TypeNumber == 145
								   || HwFuncs[i].m_DvcDesc.m_TypeNumber == 160
								   || HwFuncs[i].m_DvcDesc.m_TypeNumber == 2145))
			{
				// No board type specified so any input board is OK
				nBoard++;
			}
			else if ( g_nBoardType == HwFuncs[i].m_DvcDesc.m_TypeNumber)
			{
				nBoard++;
			}

			if (g_nBoardNum == nBoard)
			{
				// Found the target board
				if ( 0 == g_nBoardType )
				{
					// No specific type was specified, so store the type found
					g_nBoardType = HwFuncs[i].m_DvcDesc.m_TypeNumber;
				}
				break;
			}
		}

		if ( 0 == nBoard )
		{
			// No board found at all
			if ( 0 == g_nBoardType )
			{
				// No specific type specified
				wsprintf(szTemp, "No input board in the system");
			}
			else
			{
				// Specific type specified
				wsprintf(szTemp, "No DTA-%d in the system", g_nBoardType);
			}

			DebugTrace(szTemp);
			throw bResult;

		}
		else if ( g_nBoardNum != nBoard )
		{ 
			// Could not find specified board
			if ( 0 == g_nBoardType )
			{
				// No specific type specified
				wsprintf(szTemp, "Could not find input board #%d in the system", g_nBoardNum);
			}
			else
			{
				// Specific type specified
				wsprintf(szTemp, "Could not find DTA-%d #%d in the system", g_nBoardType, g_nBoardNum);
			}
			
			DebugTrace(szTemp);
			throw bResult;
		}
		//.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.- Attach to Card -.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.
		dr = rCard.AttachToSlot(HwFuncs[i].m_DvcDesc.m_PciBusNumber, HwFuncs[i].m_DvcDesc.m_SlotNumber);
		if (dr != DTAPI_OK)
		{
			if (dr == DTAPI_E_DRIVER_INCOMP)
			{
				int nDriverVersionMajor(-1), nDriverVersionMinor(-1);
				int nDriverVersionBugFix(-1), nDriverVersionBuild(-1);

				::DtapiGetDeviceDriverVersion(nDriverVersionMajor, nDriverVersionMinor,
											  nDriverVersionBugFix, nDriverVersionBuild);
				wsprintf(szTemp, "The version of the Dta1xx device driver (V%d.%d.%d %d) "
								"is not compatible\nwith this version of DtRecord.\n"
								"Please install the latest version of the Dta1xx driver.", 
								nDriverVersionMajor, nDriverVersionMinor,
								nDriverVersionBugFix, nDriverVersionBuild);

			}
			else
			{
				wsprintf(szTemp, "Failed to attach to the DTA-%d on Bus: %d and Slot: %d",  g_nBoardType,
						HwFuncs[i].m_DvcDesc.m_PciBusNumber, HwFuncs[i].m_DvcDesc.m_SlotNumber);
			}
			DebugTrace(szTemp);
			throw bResult;
		}

		// On ASI cards, set to input mode (might be bi-directional)
		dr = rCard.SetIoConfig(HwFuncs[i].m_Port, DTAPI_IOCONFIG_INPUT);

		//.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.- Attach to Channel -.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.
		dr = rInputCh.AttachToPort(&rCard, HwFuncs[i].m_Port, false);
		if (dr != DTAPI_OK)
		{
			DebugTrace("Can't attach to the channel");
			throw bResult;
		}
	}
	catch (...)
	{
		bResult = false;
	}
#else USB
	char szTemp[256];

	try
	{
		int i;
		int nEntries = 64;
		int nSourceIndex = ss->nSourceIndex;
		DtDeviceDesc DvcDescArr[64];

		if (DTAPI_OK != DtapiDeviceScan(nEntries, nEntries, DvcDescArr))
		{
			strcpy(szTemp, "DtapiDeviceScan failed");
			DebugTrace(szTemp);
			throw bResult;
		}

		wsprintf(szTemp, "Dektec USB: Devices from DtapiDeviceScan == %d\n", nEntries);
		OutputDebugString(szTemp);
		for (i = 0; i < nEntries; i++)
		{
			wsprintf(szTemp, "Dektec USB: Category: %d TypeNumber: %d\n", DvcDescArr[i].m_Category, DvcDescArr[i].m_TypeNumber);
			OutputDebugString(szTemp);
			if (DvcDescArr[i].m_Category == DTAPI_CAT_USB)
			{
#ifndef DTU234
#ifdef DTU325
				if (DvcDescArr[i].m_TypeNumber == 235)
#else DTU235
				if (DvcDescArr[i].m_TypeNumber == 225 || DvcDescArr[i].m_TypeNumber == 245)
#endif DTU235
#else DTU234
				if (DvcDescArr[i].m_TypeNumber == 234)
#endif DTU234
				{
					if (nSourceIndex == 0)
						break;
					nSourceIndex--;				
				}
			}
		}
		if (i == nEntries)
		{
			strcpy(szTemp, "Unable to locate any Dektec USB devices");
			DebugTrace(szTemp);
			throw bResult;
		}
		//.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.- Attach to Card -.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.
		DTAPI_RESULT  dr;
		dr = rCard.AttachToSerial(DvcDescArr[i].m_Serial);
		if (dr != DTAPI_OK)
		{
			strcpy(szTemp, "Unable to attach to Dektec by serial number");
			DebugTrace(szTemp);
			throw bResult;
		}
		if (rInputCh.Attach(&rCard) != DTAPI_OK)
		{
			DebugTrace("Can't attach to the channel");
			throw bResult;
		}
	}
	catch (...)
	{
		bResult = false;
	}
#endif USB
	return bResult;
}

DWORD WINAPI DektecReceiveThread(LPVOID pPtr)
{
	int nTSBufferIndex = 0;
	int nPacketLength = 188;
	int nFIFOCheckCounter = 1000000;
	int nMaxFIFOSize = 0;
	DTAPI_RESULT dr;
	BYTE * pBuffer;
	//BYTE bTimestampingRXBuffer[RX_BUFFER_PACKET_COUNT * 192];

	DebugTrace("ReceiveThread+");

	theInput.GetMaxFifoSize(nMaxFIFOSize);
	pBuffer = (BYTE *)LocalAlloc(LPTR, nMaxFIFOSize);
	
	SourceHelper_StartSyncThread(ss, FALSE);

//	if (fTimestampPackets == TRUE)
//		nPacketLength = 192;

	dr = theInput.SetRxControl(DTAPI_RXCTRL_RCV);
	if (DTAPI_OK != dr)
	{
		DebugTrace("Unable to start receive");
	}
	else
	{
		while (!ss->fTerminateReadThread)
		{
			int nFIFOLoad = 0;
			//int nReceiveBufferSize;

			theInput.GetFifoLoad(nFIFOLoad);
			if (nFIFOLoad < 64 * 1024)
			{
				Sleep(1);
#ifdef DTU234
				// Check signal
				if (nFIFOCheckCounter++ > 64)
				{
					EnterCriticalSection(&csSignal);
					B1049DemodGetStatus1(g_nBoardNum - 1);
					if (B1049GetDemodStatusRLock(g_nBoardNum - 1) && B1049GetDemodStatusFLock(g_nBoardNum - 1))
					{
						float fSNR;
						fSNR = (float)B1049GetDemodStatusEstSNR(g_nBoardNum - 1) / 256.0f;
						sprintf(szLastSignalReport, "Locked SNR %.1f dB", fSNR);
					}
					else
						lstrcpy(szLastSignalReport, "Unlocked");
					LeaveCriticalSection(&csSignal);
					nFIFOCheckCounter = 0;
				}
#endif DTU234
#ifdef DTU235
				{
				}
#endif DTU235

				continue;
			}
			nFIFOLoad &= ~3;	// ensure divisible by 4
			dr = theInput.Read((char *)pBuffer, nFIFOLoad);
			SourceHelper_SyncData(pBuffer, nFIFOLoad);
			
			/*nReceiveBufferSize = RX_BUFFER_SIZE;
			if (fTimestampPackets == TRUE)
			{
				int nPacket;
				int nPackets = nReceiveBufferSize / 192;
				BYTE * pInputDataPtr = bTimestampingRXBuffer;
				BYTE * pOutputDataPtr = ss->tsb[nTSBufferIndex].pData;

				dr = theInput.Read((char *)bTimestampingRXBuffer, nReceiveBufferSize);
				for (nPacket = 0; nPacket < nPackets; nPacket++)
				{
					if (ss->tsb[nTSBufferIndex].pTimestamps != NULL)
						memcpy(&ss->tsb[nTSBufferIndex].pTimestamps[nPacket], pInputDataPtr, 4);
					pInputDataPtr += 4;
					memcpy(pOutputDataPtr, pInputDataPtr, 188);
					pOutputDataPtr += 188;
					pInputDataPtr += 188;
				}
				ss->tsb[nTSBufferIndex].nSize = nPackets * 188;
			}
			else
			{
				// Do it the quick way without timestamps
				dr = theInput.Read((char *)ss->tsb[nTSBufferIndex].pData, nReceiveBufferSize);
				if (DTAPI_OK != dr)
				{
					char szTemp[128];
					wsprintf(szTemp, "Read error with return %04x", dr);
					DebugTrace(szTemp);
					break;
				}
				ss->tsb[nTSBufferIndex].nSize = nReceiveBufferSize;
			}

			EnterCriticalSection(&ss->csPIDCounter);
			ss->nLastSecondByteCounter += ss->tsb[nTSBufferIndex].nSize;
			LeaveCriticalSection(&ss->csPIDCounter);
			nTSBufferIndex++;
			if (nTSBufferIndex == MAX_TS_BUFFERS)
				nTSBufferIndex = 0;
			EnterCriticalSection(&ss->csTSBuffersInUse);
			ss->nTSBuffersInUse++;
			LeaveCriticalSection(&ss->csTSBuffersInUse);
			ss->tsb[nTSBufferIndex].nSize = 0;
			*/

#ifndef DTU234
#ifndef DTU235
			// Check for FIFO overrun
			if (nFIFOCheckCounter++ > 256)
			{
				int nStatus = 0;
				int nLatched = 0;
				theInput.GetFlags(nStatus, nLatched);
				if (0 != (DTAPI_RX_FIFO_OVF & nLatched))
				{
					nFIFOOverrunCounter++;
					DebugTrace("FIFO overrun has occured");
					//break;
				}
				nFIFOCheckCounter = 0;
			}
#endif DTU235
#endif DTU234
		}
	}

	LocalFree(pBuffer);
	theInput.SetRxControl(DTAPI_RXCTRL_IDLE);
	SourceHelper_StopSyncThread();

	ss->fTerminateReadThread = FALSE;
	EnterCriticalSection(&ss->csTSBuffersInUse);
	ss->nTSBuffersInUse = -1000;
	LeaveCriticalSection(&ss->csTSBuffersInUse);

	ss->fReadThreadTerminated = TRUE;
	CloseHandle(ss->hReadDataThread);
	DebugTrace("ReceiveThread-");

	return 0;
}

BOOL TSReader_Init(PSOURCESTRUCT pss)
{
	if (SourceHelper_ValidateSourceContainer(pss) == FALSE)
		return FALSE;

	ss = pss;
	//fTimestampPackets = ss->fTimestampPackets;

	g_nBoardType = 0;		// reset board type since any is acceptable
	g_nBoardNum = ss->nSourceIndex + 1;	// board number we want

	if (!AttachToBoard(theCard, theInput))
	{
		DebugTrace("AttachToBoard() failed");
		return FALSE;
	}

#ifdef DTU234
	InitializeCriticalSection(&csSignal);
	if (!fBoard1049InitDone)
	{
		SetCapture(ss->hWndTSReader);
		SetCursor(LoadCursor(NULL, IDC_WAIT));

		hSencoreDLL = LoadLibrary("Board1049.dll");
		if (hSencoreDLL == NULL)
		{
			DebugTrace("Unable to load Board1049.dll");
			return FALSE;
		}
		B1049DoesExist = (td_B1049DoesExist)GetProcAddress(hSencoreDLL, "B1049DoesExist");
		B1049Initialize = (td_B1049Initialize)GetProcAddress(hSencoreDLL, "B1049Initialize");
		B1049Uninitialize = (td_B1049Uninitialize)GetProcAddress(hSencoreDLL, "B1049Uninitialize");
		B1049SetTunerFrequencyHz = (td_B1049SetTunerFrequencyHz)GetProcAddress(hSencoreDLL, "B1049SetTunerFrequencyHz");
		B1049GetTunerFrequencyHz = (td_B1049GetTunerFrequencyHz)GetProcAddress(hSencoreDLL, "B1049GetTunerFrequencyHz");
		B1049DemodSetAcquire = (td_B1049DemodSetAcquire)GetProcAddress(hSencoreDLL, "B1049DemodSetAcquire");
		B1049DemodGetStatus1 = (td_B1049DemodGetStatus1)GetProcAddress(hSencoreDLL, "B1049DemodGetStatus1");
		B1049GetDemodStatusEstSNR = (td_B1049GetDemodStatusEstSNR)GetProcAddress(hSencoreDLL, "B1049GetDemodStatusEstSNR");
		B1049GetDemodStatusOLock = (td_B1049GetDemodStatusOLock)GetProcAddress(hSencoreDLL, "B1049GetDemodStatusOLock");
		B1049GetDemodStatusFLock = (td_B1049GetDemodStatusFLock)GetProcAddress(hSencoreDLL, "B1049GetDemodStatusFLock");
		B1049GetDemodStatusRLock = (td_B1049GetDemodStatusRLock)GetProcAddress(hSencoreDLL, "B1049GetDemodStatusRLock");
		B1049GetChannelLevel = (td_B1049GetChannelLevel)GetProcAddress(hSencoreDLL, "B1049GetChannelLevel");
		B1049Versions = (td_B1049Versions)GetProcAddress(hSencoreDLL, "B1049Versions");

		if (B1049DoesExist(g_nBoardNum - 1) == FALSE)
		{
			OutputDebugString("B1049DoesExist failed\n");
			return FALSE;
		}
		if (B1049Initialize(g_nBoardNum - 1) == FALSE)
		{
			OutputDebugString("B1049Initialize failed\n");
			return FALSE;
		}
		fBoard1049InitDone = TRUE;

		ReleaseCapture();
		SetCursor(LoadCursor(NULL, IDC_ARROW));
	}
#endif DTU234
#ifdef DTU235
	InitializeCriticalSection(&csSignal);
	if (!fBoard1077InitDone)
	{
		SetCapture(ss->hWndTSReader);
		SetCursor(LoadCursor(NULL, IDC_WAIT));

		hSencoreDLL = LoadLibrary("Board1077.dll");
		if (hSencoreDLL == NULL)
		{
			DebugTrace("Unable to load Board1077.dll");
			return FALSE;
		}
		
		B1077Initialize = (td_B1077Initialize)GetProcAddress(hSencoreDLL, "B1077Initialize");
		B1077Uninitialize = (td_B1077Uninitialize)GetProcAddress(hSencoreDLL, "B1077Uninitialize");
		B1077DoesExist = (td_B1077DoesExist)GetProcAddress(hSencoreDLL, "B1077DoesExist");
		B1077SetChannelBandwidth = (td_B1077SetChannelBandwidth)GetProcAddress(hSencoreDLL, "B1077SetChannelBandwidth");
		B1077SetChannelClassification = (td_B1077SetChannelClassification)GetProcAddress(hSencoreDLL, "B1077SetChannelClassification");
		B1077SetChannelConstellation = (td_B1077SetChannelConstellation)GetProcAddress(hSencoreDLL, "B1077SetChannelConstellation");
		B1077SetChannelGuard = (td_B1077SetChannelGuard)GetProcAddress(hSencoreDLL, "B1077SetChannelGuard");
		B1077SetChannelHierarchy = (td_B1077SetChannelHierarchy)GetProcAddress(hSencoreDLL, "B1077SetChannelHierarchy");
		B1077SetChannelMirror = (td_B1077SetChannelMirror)GetProcAddress(hSencoreDLL, "B1077SetChannelMirror");
		B1077SetChannelPriority = (td_B1077SetChannelPriority)GetProcAddress(hSencoreDLL, "B1077SetChannelPriority");
		B1077SetFftMode = (td_B1077SetFftMode)GetProcAddress(hSencoreDLL, "B1077SetFftMode");
		B1077SetChannelParameters = (td_B1077SetChannelParameters)GetProcAddress(hSencoreDLL, "B1077SetChannelParameters");
		B1077SetTunerFrequencyHz = (td_B1077SetTunerFrequencyHz)GetProcAddress(hSencoreDLL, "B1077SetTunerFrequencyHz");
		B1077GetChannelLevel = (td_B1077GetChannelLevel)GetProcAddress(hSencoreDLL, "B1077GetChannelLevel");
		B1077GetLockStatus = (td_B1077GetLockStatus)GetProcAddress(hSencoreDLL, "B1077GetLockStatus");

		if (B1077DoesExist(g_nBoardNum - 1) == FALSE)
		{
			OutputDebugString("B1077DoesExist failed\n");
			return FALSE;
		}
		if (B1077Initialize(g_nBoardNum - 1) == FALSE)
		{
			OutputDebugString("B1077Initialize failed\n");
			return FALSE;
		}
		fBoard1077InitDone = TRUE;

		ReleaseCapture();
		SetCursor(LoadCursor(NULL, IDC_ARROW));
	}
#endif DTU235


	/*if (fTimestampPackets == TRUE)
	{
		if (DTAPI_OK != theInput.SetRxMode(DTAPI_RXMODE_ST188 | DTAPI_RX_TIMESTAMP))
		{
			DebugTrace("theInput.SetRxMode() failed");
			return FALSE;
		}
	}
	else*/
	{
		if (DTAPI_OK != theInput.SetRxMode(DTAPI_RXMODE_ST188))
		{
			DebugTrace("theInput.SetRxMode() failed");
			return FALSE;
		}
	}

	// Do not start recording yet, Start after printing final messages
	// So for now set receive control to idle.
	if (DTAPI_OK != theInput.SetRxControl(DTAPI_RXCTRL_IDLE))
	{
		DebugTrace("theInput.SetRxControl() failed");
		return FALSE;
	}

	if (DTAPI_OK != theInput.ClearFifo())
	{
		DebugTrace("theInput.ClearFifo() failed");
		return FALSE;
	}

	return TRUE;
}

BOOL TSReader_DeInit()
{
	// Detach from input channel and then from PCI card
	theInput.Detach(DTAPI_INSTANT_DETACH);
	theCard.Detach();
#ifdef DTU234
	DeleteCriticalSection(&csSignal);
#endif DTU234
#ifdef DTU235
	DeleteCriticalSection(&csSignal);
#endif DTU235
	return TRUE;
}

BOOL TSReader_Start() 
{
	DWORD dwThreadID;

#ifndef DTU234
#ifndef DTU235
	nFIFOOverrunCounter = 0;
#endif DTU235
#endif DTU234

	ss->hReadDataThread = CreateThread(NULL, 0, DektecReceiveThread, (LPVOID)0, CREATE_SUSPENDED, &dwThreadID);
	SourceHelper_SetWorkerThreadPriorities(FALSE);
	ResumeThread(ss->hReadDataThread);
	return TRUE;
}

BOOL TSReader_Stop()
{
	DebugTrace("enter Stop()");

	ss->fTerminateReadThread = TRUE;
	while (ss->fReadThreadTerminated == FALSE)
		Sleep(50);
	
	DebugTrace("leave Stop()");
	return TRUE;
}

#ifdef DTU234
void SetupLastTune()
{
#ifndef QAM
	wsprintf(szLastTune, "Channel %d (%d MHz)", SourceHelper_GetATSCChannelFromFrequency(ss->nFrequency), ss->nFrequency);
#else QAM
	wsprintf(szLastTune, "Channel %d (%d MHz)", SourceHelper_GetQAMChannelFromFrequency(ss->nFrequency), ss->nFrequency);
#endif QAM
}
#endif DTU234

#ifdef DTU235
void SetupLastTune()
{
	sprintf(szLastTune, "%.3f MHz", ss->nFrequency / 1000.0);
}
#endif DTU235

BOOL TSReader_Tune()
{
#ifndef DTU234
#ifndef DTU235
	if (ss->fSerialReceiverControlEnabled && !fDontTune)
	{
		if (SourceHelper_TuneSerialControl(szLastTune) == FALSE)
			return FALSE;
	}
#else DTU235
	{
		int nBandwidthSelections[] = {2, 1, 0, 0};	// 6, 7, 8 MHz plus one extra to stop errors
		DWORD dwTimeout;
		
		SetupLastTune();

		B1077SetChannelBandwidth(g_nBoardNum - 1, nBandwidthSelections[ss->nBandwidth & 0x03]);
		B1077SetChannelClassification(g_nBoardNum - 1, 255);
		B1077SetChannelCoderate(g_nBoardNum - 1, 255);
		B1077SetChannelConstellation(g_nBoardNum - 1, 255);
		B1077SetChannelGuard(g_nBoardNum - 1, 255);
		B1077SetChannelHierarchy(g_nBoardNum - 1, 255);
		B1077SetChannelMirror(g_nBoardNum - 1, 255);
		B1077SetChannelPriority(g_nBoardNum - 1, 1);	// high priority?
		B1077SetFftMode(g_nBoardNum - 1, 255);
		if (B1077SetChannelParameters(g_nBoardNum - 1) == FALSE)
		{
			DebugTrace("B1077SetChannelParameters failed");
			return FALSE;
		}
		B1077SetTunerFrequencyHz(g_nBoardNum - 1, ss->nFrequency * 1000);
		dwTimeout = GetTickCount();
		while (GetTickCount() < dwTimeout + 1000)
		{
			B1077GetChannelLevel(g_nBoardNum - 1);
			if (B1077GetLockStatus(g_nBoardNum - 1) == 2)
				return TRUE;
			Sleep(100);
		}
		if (ss->fQuietMode == FALSE)
			MessageBox(ss->hWndTSReader, "Failed to lock signal", gszSourceName, MB_ICONWARNING);
		return FALSE;
	}
#endif DTU235
#else DTU234
	DWORD dwTimeout;
#ifndef QAM
	int nLoopCount = 1;
#else QAM
	int nLoopCount = 2;
#endif QAM
	int nLoop;

	SetupLastTune();

#ifndef QAM
	B1049DemodSetAcquire(g_nBoardNum - 1, 0);	// 8VSB
#else QAM
	B1049DemodSetAcquire(g_nBoardNum - 1, 3);	// 256QAM-B
#endif QAM
	B1049SetTunerFrequencyHz(g_nBoardNum - 1, ss->nFrequency * 1000000);
	Sleep(100);

	for (nLoop = 0; nLoop < nLoopCount; nLoop++)
	{
#ifdef QAM
		if (nLoop == 1)
			B1049DemodSetAcquire(g_nBoardNum - 1, 2);	// 64QAM-B
#endif QAM
		dwTimeout = GetTickCount();
		while (GetTickCount() < dwTimeout + 1000)
		{
			B1049GetChannelLevel(g_nBoardNum - 1);
			B1049DemodGetStatus1(g_nBoardNum - 1);
			if (B1049GetDemodStatusRLock(g_nBoardNum - 1) && B1049GetDemodStatusFLock(g_nBoardNum - 1))
				return TRUE;
			Sleep(100);
		}
	}
	if (ss->fQuietMode == FALSE)
		MessageBox(ss->hWndTSReader, "Failed to lock signal", gszSourceName, MB_ICONWARNING);
	return FALSE;
#endif DTU234
	return TRUE;
}

BOOL TSReader_TuneDialog(HWND hWnd)
{
	DebugTrace("TSReader_TuneDialog");

#ifndef DTU234
#ifndef DTU235
	if (ss->fSerialReceiverControlEnabled == TRUE)
	{
		if (ss->fDontTune == TRUE)
			fNeedTuneDialog = FALSE;

		if (fNeedTuneDialog)
		{
			fDontTune = FALSE;
			DebugTrace("TSReader_TuneDialog tuning dialog is required");
			if (SourceHelper_ADVTuneDialog(hWnd) == FALSE)
				fDontTune = TRUE;
		}
		else
		{
			DebugTrace("TSReader_TuneDialog tune NOT required");
			ss->nFrequency = nFrequency;
			ss->nPolarity = nPolarity;
			ss->nSymbolRate = nSymbolRate;
			ss->nLNBFrequency = nLNBFrequency;
			ss->n22KHz = n22KHz;
			ss->nDiSEqCInput = nDiSEqCInput;
			ss->nCodeRate = nCodeRate;
			ss->nADVModulationMode = nADVModulationMode;
			fNeedTuneDialog = TRUE;
		}
	}
#else DTU235
	if (ss->fDontTune == TRUE)
	{
		fNeedTuneDialog = FALSE;
		OutputDebugString("TuneDialog() set fNeedTuneDialog = FALSE\n");
	}

	{
		char szTemp[100];
		wsprintf(szTemp, "fNeedTuneDialog = %d\n", fNeedTuneDialog);
		OutputDebugString(szTemp);
	}
	if (fNeedTuneDialog)
	{
		if (SourceHelper_DVBTTuneDialog(hWnd) == FALSE)
			return FALSE;
	}
	else
	{
		ss->nFrequency = nFrequency;
		ss->fSpectrumInversion = fSpectrumInversion;
		ss->nBandwidth = nBandwidth;
		if (ss->fQuietMode == FALSE)
		{
			fNeedTuneDialog = TRUE;
			OutputDebugString("TuneDialog() set fNeedTuneDialog = TRUE\n");
		}
	}
#endif DTU235
#else DTU234
	DebugTrace("TSReader_TuneDialog (8VSB)\n");

	if (ss->fDontTune == TRUE)
		fNeedTuneDialog = FALSE;

	if (fNeedTuneDialog)
	{
		OutputDebugString("Sencore: TSReader_TuneDialog tuning dialog is required\n");
		ss->fQuietMode = FALSE;
#ifndef QAM
		if (SourceHelper_ATSCTuneDialog(hWnd) == FALSE)
			return FALSE;
#else QAM
		if (SourceHelper_QAMTuneDialog(hWnd) == FALSE)
			return FALSE;
#endif QAM
	}
	else
	{
		OutputDebugString("Sencore: TSReader_TuneDialog tune NOT required\n");
		ss->nFrequency = nFrequency;
		fNeedTuneDialog = TRUE;
	}
#endif DTU234

	return TRUE;
}

BOOL TSReader_PIDManagement(BOOL fAdd, int nPID, BOOL fTemporary)
{
	return TRUE;
}

BOOL TSReader_GetDescription(char * szDescription, char * szCommandLineParameters, BOOL * fCanBeStopped, int * nMaxPIDs, DWORD * dwCapabilities)
{
	if (szDescription != NULL)
		lstrcpy(szDescription, gszSourceName);
	if (fCanBeStopped != NULL)
		*fCanBeStopped= FALSE;
	if (nMaxPIDs != NULL)
		*nMaxPIDs = 8192;
#ifndef DTU234
	if (szCommandLineParameters != NULL)
		lstrcpy(szCommandLineParameters, "None");
	if (dwCapabilities != NULL)
		*dwCapabilities = CAPABILITIES_SERIAL_CONTROL
						| CAPABILITIES_MULTICARD;
#else DTU234
	if (szCommandLineParameters != NULL)
		lstrcpy(szCommandLineParameters, "freq");	
	if (dwCapabilities != NULL)
		*dwCapabilities = CAPABILITIES_MULTICARD;
#endif DTU234

	return TRUE;
}

BOOL TSReader_ParseCommandLine(PSOURCESTRUCT ss, char * szCommandLine, BOOL fQuiet)
{
#ifndef DTU234
#ifndef DTU235
	if (ss->fSerialReceiverControlEnabled)
	{
		if (lstrlen(szCommandLine))
		{
			int nConversionCount = 0;

			SourceHelper_ConvertPolarity(szCommandLine);
			ss->nDiSEqCInput = 1;
			nConversionCount = sscanf(szCommandLine,
									  "%d %d %d %d %d %d %d %d", 
									  &nFrequency,
									  &nPolarity,
									  &nSymbolRate,
									  &nADVModulationMode,
									  &nCodeRate,
									  &nLNBFrequency,
									  &n22KHz,
									  &nDiSEqCInput);
			if (nConversionCount < 7)
			{
				if (!fQuiet)
					MessageBox(NULL,
						   "Usage for this source: freq pol sr mod fec lnbf 22khz {input}\n"
						   "\n"
						   "freq = frequency to tune\n"
						   "pol = 0 vertical/RHCP 1 horizontal/LHCP\n"
						   "sr = symbol rate\n"
						   "mod = modulation mode (see readme)\n"
						   "fec = FEC code rate (see readme)\n"
						   "lnbf = LNB frequency\n"
						   "22k = 22KHz tone enable\n"
						   "input = select DiSEqC input number (1-4) - optional",
						   gszSourceName,
						   MB_OK | MB_ICONSTOP);
				return FALSE;
			}
			fNeedTuneDialog = FALSE;
		}
		else
			fNeedTuneDialog = TRUE;
	}
#else DTU235
	fNeedTuneDialog = TRUE;
	if (lstrlen(szCommandLine))
	{
		int nConversionCount;

		nConversionCount = sscanf(szCommandLine,
								  "%d %d %d", 
								  &nFrequency,
								  &fSpectrumInversion,
								  &nBandwidth);
		if (nConversionCount < 3)
		{
			if (!fQuiet)
				MessageBox(NULL,
					   "Usage for this source: freq inversion bandwidth\n"
					   "\n"
					   "freq = frequency to tune in KHz\n"
					   "inversion = inverted spectrum (0 or 1)\n"
					   "bandwidth = bandwidth of signal (0 = 6, 1 = 7, 2 = 8 MHz)",
					   gszSourceName,
					   MB_OK | MB_ICONSTOP);
			return FALSE;
		}
		fNeedTuneDialog = FALSE;
	}
	else
		fNeedTuneDialog = TRUE;
#endif DTU235
#else DTU234
	fNeedTuneDialog = TRUE;
	if (lstrlen(szCommandLine))
	{
		int nConversionCount;

		nConversionCount = sscanf(szCommandLine,
								  "%d", 
								  &nFrequency);
		if (nConversionCount < 1)
		{
			if (!fQuiet)
				MessageBox(NULL,
					   "Usage for this source: freq\n"
					   "\n"
					   "freq = frequency to tune in MHz or prefix with 0 for channel number, e.g. 022 for channel 22",
					   gszSourceName,
					   MB_OK | MB_ICONSTOP);
			return FALSE;
		}
		if (*szCommandLine == '0')
#ifndef QAM
			nFrequency = SourceHelper_GetFrequencyFromATSCChannel(nFrequency);
#else QAM
			nFrequency = SourceHelper_GetFrequencyFromQAMChannel(nFrequency);
#endif QAM
		fNeedTuneDialog = FALSE;
	}
	else
		fNeedTuneDialog = TRUE;
#endif DTU234

	return TRUE;
}

BOOL TSReader_IsPIDActive(int nPID)
{
	return TRUE;
}

BOOL TSReader_SetChannel(int nChannel)
{
	return TRUE;
}

BOOL TSReader_GetSignalString(char * szString)
{
#ifndef DTU234
#ifdef DTU235
	EnterCriticalSection(&csSignal);
	lstrcpy(szString, szLastSignalReport);
	LeaveCriticalSection(&csSignal);
#else DTU235
	wsprintf(szString, "FIFO Overruns: %d", nFIFOOverrunCounter);
#endif DTU235
#else DTU234
	EnterCriticalSection(&csSignal);
	lstrcpy(szString, szLastSignalReport);
	LeaveCriticalSection(&csSignal);
#endif DTU234
	return TRUE;
}

BOOL TSReader_GetTunerString(char * szString)
{
#ifndef DTU234
#ifdef DTU235
	lstrcpy(szString, szLastTune);
#else DTU235
	lstrcpy(szString, "n/a");
#endif DTU235
#else DTU234
	lstrcpy(szString, szLastTune);
#endif DTU234
	return TRUE;
}

BOOL TSReader_SendDiSEqC(BYTE * bCommand, int nLength)
{
	return TRUE;
}

int TSReader_GetSyncLossCount(BOOL fReset)
{
	return SourceHelper_GetSyncLossCount(fReset);
}

BOOL APIENTRY DllMain(HANDLE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
    switch( ul_reason_for_call )
	{
    case DLL_PROCESS_ATTACH:
		hInstance = hModule;
		break;
    case DLL_PROCESS_DETACH:
#ifdef DTU234
		if (fBoard1049InitDone)
		{
			if (hSencoreDLL != NULL)
			{
				FreeLibrary(hSencoreDLL);
				hSencoreDLL = NULL;
			}
			fBoard1049InitDone = FALSE;
		}
#endif DTU234
#ifdef DTU235
		if (fBoard1077InitDone)
		{
			if (hSencoreDLL != NULL)
			{
				FreeLibrary(hSencoreDLL);
				hSencoreDLL = NULL;
			}
			fBoard1077InitDone = FALSE;
		}
#endif DTU235
		break;
    }
    return TRUE;
}
