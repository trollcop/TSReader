#include <windows.h>
#include <commctrl.h>
#include "initguid.h"
#include "winioctl.h"
#include "setupapi.h"	// VC++ 5 one is out of date
#include <stdio.h>

#include "..\sources.h"
#ifdef BCM4500
#include "bcm4500.h"
#include "resource.h"
#endif BCM4500

PSOURCESTRUCT ss;
HANDLE hInstance;
BOOL fDontTune = FALSE;

// Hardware specific stuff
#include "dvbIoctl.h"

#define MaxTsfrDir MaxPlxDmaChannels
#define PAGE_SIZE 4096
#define QueuedExtras 2

#define ROLLOVER(x,y) if((x)==((y)-1))(x)=0; else (x)++;

HANDLE			ShutDown;
HANDLE			DvB[MaxTsfrDir]={INVALID_HANDLE_VALUE,INVALID_HANDLE_VALUE};
HANDLE			StDvB[MaxTsfrDir]={INVALID_HANDLE_VALUE,INVALID_HANDLE_VALUE};
HANDLE			LfhWaitForStsChgThread;
char			*DvBBuf[MaxTsfrDir];
OVERLAPPED		*OvOp[MaxTsfrDir];
int				MaxBufferSize[MaxTsfrDir],NumBufs[MaxTsfrDir];
int				Idx[MaxTsfrDir]={0,0};
int				OpsPending[MaxTsfrDir]={0,0};
int				OpsString[MaxTsfrDir]={0,0};
int				MaxOpsPending[MaxTsfrDir]={0,0};
int				MaxOpsString[MaxTsfrDir]={0,0};
int				WaitTime=1000;
int				NumHandles=0;
int				Dir[MaxTsfrDir];
int				Extra=QueuedExtras;

BOOL fNeedTuneDialog = TRUE;
int nFrequency;
int nPolarity;
int nSymbolRate;
int nLNBFrequency;
int n22KHz;
int nDiSEqCInput;
int nADVModulationMode;
int nCodeRate;
int nMaximumCards;
BOOL fTimestampPackets = FALSE;
char szLastTune[128] = {"n/a"};

#ifdef BCM4500
HANDLE g_hUSB = INVALID_HANDLE_VALUE;
#endif BCM4500

#ifndef BCM4500
	char gszSourceName[] = {"Linear Systems DVB-ASI cards"};
#else BCM4500
	char gszSourceName[] = {"Linear Systems DVB-ASI cards with BCM4500 tuner"};
#endif

BOOL __cdecl SourceHelper_ValidateSourceContainer(PSOURCESTRUCT pss);

// This is the I/O completion routine called by the operating system when I/O has 
// completed on a I/O request (read or write)
VOID CALLBACK FileIOCompletionRoutine(DWORD dwErrorCode, DWORD dwNumberOfBytesTransfered, LPOVERLAPPED lpOverlapped)
{
	lpOverlapped->Internal=dwNumberOfBytesTransfered;	// save the number of bytes in the OVERLAPPED structure
	lpOverlapped->Offset=dwErrorCode;	// save the error code
	lpOverlapped->hEvent=(void *)1;	 // this is used as a flag to show this structure has been COMPLETED !
}

HANDLE GetDeviceViaInterface(GUID* pGuid, DWORD instance, int ovr)
{
	SP_INTERFACE_DEVICE_DATA ifdata;
	PSP_INTERFACE_DEVICE_DETAIL_DATA ifDetail;
	DWORD ReqLen;
	HANDLE rv;
	// Get handle to relevant device information set
	HDEVINFO info = SetupDiGetClassDevs(pGuid, NULL, NULL, DIGCF_PRESENT | DIGCF_INTERFACEDEVICE);
	if(info==INVALID_HANDLE_VALUE)
	{
		return INVALID_HANDLE_VALUE;
	}

	// Get interface data for the requested instance
	ifdata.cbSize = sizeof(ifdata);
	if(!SetupDiEnumDeviceInterfaces(info, NULL, pGuid,instance, &ifdata))
	{
		SetupDiDestroyDeviceInfoList(info);
		return INVALID_HANDLE_VALUE;
	}

	// Get size of symbolic link name
	SetupDiGetDeviceInterfaceDetail(info, &ifdata, NULL, 0, &ReqLen, NULL);
	ifDetail = (PSP_INTERFACE_DEVICE_DETAIL_DATA)malloc(ReqLen);
	if( ifDetail==NULL)
	{
		SetupDiDestroyDeviceInfoList(info);
		return INVALID_HANDLE_VALUE;
	}

	// Get symbolic link name
	ifDetail->cbSize = sizeof(SP_INTERFACE_DEVICE_DETAIL_DATA);
	if( !SetupDiGetDeviceInterfaceDetail(info, &ifdata, ifDetail, ReqLen, NULL, NULL))
	{
		SetupDiDestroyDeviceInfoList(info);
		free( ifDetail);
		return INVALID_HANDLE_VALUE;
	}

	// Open file
	rv = CreateFile( ifDetail->DevicePath, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, ovr, NULL);
	free(ifDetail);
	SetupDiDestroyDeviceInfoList(info);
	return rv;
}

#ifdef BCM4500
BOOL CALLBACK LoadBCM4500DlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	static BOOL fFirstTime;

	switch(uMsg)
	{
	case WM_INITDIALOG:
		fFirstTime = TRUE;
		break;
	case WM_ACTIVATE:
		if (fFirstTime == TRUE)
		{
			fFirstTime = FALSE;
			PostMessage(hDlg, WM_USER + 1, 0, 0);
		}
		break;
	case WM_USER + 1:
		SetDlgItemText(hDlg, IDC_STATUS, "Setting up interface");
		SetupDNIF();
		SetDlgItemText(hDlg, IDC_STATUS, "Loading BCM-4500 firmware");
		if (WriteBCM4500Memory(hDlg) == FALSE)
			EndDialog(hDlg, FALSE);
		EndDialog(hDlg, TRUE);
		break;
	}
	return FALSE;
}

BOOL InitBCM4500()
{
	int nTimeout = 5;

	if (OpenUSBDriver() == FALSE)
		return FALSE;

	// Try sending a get status command to see if the hardware is alive
	do
	{
		BYTE * legacy_data;

		if (bcm4500_get_legacy_status(&legacy_data) == TRUE)
			break;	// board is ready to go
		if (DialogBox(hInstance, MAKEINTRESOURCE(IDD_LOAD_BCM4500), NULL, LoadBCM4500DlgProc) == FALSE)
		{
			MessageBox(ss->hDlgSIParser, "Problem with the BCM4500 firmware file", gszSourceName, MB_ICONSTOP);
			return FALSE;
		}
	} while (nTimeout--);

	return (nTimeout > 0) & 1;
}

#endif BCM4500

BOOL TSReader_Init(PSOURCESTRUCT pss)
{
	int i=0;
	BOOL fFirst=TRUE;

	if (SourceHelper_ValidateSourceContainer(pss) == FALSE)
		return FALSE;

	ss = pss;
	fTimestampPackets = ss->fTimestampPackets;

#ifdef BCM4500
	if (InitBCM4500() == FALSE)
		return FALSE;
#endif BCM4500

	nMaximumCards = -1;
	while(TRUE)
	{
		struct DVBCfg DvbCfg;
		ULONG n;
		HANDLE h;
		
		h=GetDeviceViaInterface((LPGUID)&DVBLINK_GUID,i,0);
		if(h==INVALID_HANDLE_VALUE)
		{
			if (i == 0)
				return FALSE;		// no device
			nMaximumCards = i;
			if (ss->nSourceIndex < nMaximumCards)
				break;
			MessageBox(NULL, "The selected Linear Systems interface is not present", gszSourceName, MB_ICONSTOP);
			return FALSE;
		}
		if(!DeviceIoControl(h,IOCTL_DVB_RD_CFG,NULL,0,&DvbCfg,sizeof(struct DVBCfg),&n,NULL))
		{
#ifdef DEBUG_MESSAGES
			OutputDebugString("Error in DeviceIoCcontrol IOCTL_DVB_RD_CFG\n");
#endif DEBUG_MESSAGES
		}
		i++;
#ifdef _DEBUGx
		DvbCfg.PlxDmaThreshold = 0x0;
		DvbCfg.FifoAf[RxDir] = 0x8;
		DvbCfg.FifoAf[TxDir] = 0xf8;
		DvbCfg.FifoAe[TxDir] = 0x80;
		if(!DeviceIoControl(h, IOCTL_DVB_SET_CFG, &DvbCfg, sizeof(DvbCfg), NULL, 0, &n, NULL))
		{
#ifdef DEBUG_MESSAGES
			OutputDebugString("Error in DeviceIoCcontrol IOCTL_DVB_SET_CFG\n");
#endif DEBUG_MESSAGES
		}
#endif
		CloseHandle(h);
	}
	ShutDown = CreateEvent(NULL,TRUE,FALSE,NULL);

	return TRUE;
}

BOOL TSReader_DeInit()
{
	CloseHandle(ShutDown);
#ifdef BCM4500
	if (g_hUSB != INVALID_HANDLE_VALUE)
		CloseHandle(g_hUSB);
#endif BCM4500

	return TRUE;
}

DWORD WINAPI DvB_ReceiveThread(LPVOID pPtr)
{
// DvBBuf[RxDir][] are data buffers for the read operations
// OvOp[RxDir][] are the OVERLAPPED structures! Read MS documentation !
// Idx[RxDir] is the index in the CIO queued buffer list of pending read operations

	int Rc;
	int i= RxDir;
	int k = 0;
	BOOL Started = FALSE;
	ULONG Seq = 0;
	int MaxNum = MaxBufferSize[i]/4;
	int errors = 0;
	int nRemainder = 0;
	int nTSBufferIndex = 0;
	int nCounter = 0;
	BOOL fFirstTime = TRUE;
	int nPacketLength = 188;

#ifdef LINEAR_SW_SYNC
	SourceHelper_StartSyncThread(ss, FALSE);
#endif LINEAR_SW_SYNC

	if (fTimestampPackets)
		nPacketLength = 196;

	while (TRUE)
	{ 
		//Retry after errors
		while(OvOp[i][Idx[i]].hEvent)	 // process all completed read operations
		{
			if(OvOp[i][Idx[i]].Offset)
				errors++;   // Count any errors that occured on that operation,
			if(OvOp[i][Idx[i]].hEvent==(void *)1) // Post initialization, read operation has completed
			{
				OpsString[i]++;
				if(OpsString[i] > MaxOpsString[i])
					MaxOpsString[i] = OpsString[i];
				OpsPending[i]--;
				if(OvOp[i][Idx[i]].Offset==0)	  // Valid data (no errors)
				{
					// This is where the data just read would now be available for processing ! 
					if(OvOp[i][Idx[i]].Internal != (DWORD)MaxBufferSize[i])
					{
						//MessageBox(NULL,"Read size error","Warning",MB_OK);
						//	Normally this condition will not occur
						errors++;
					}
					else
					{
						if (fFirstTime == FALSE)
							fFirstTime = TRUE;
						else
						{
#ifndef LINEAR_SW_SYNC
							int nTSPacketCount;
							int nNewOffset = 0;

							EnterCriticalSection(&ss->csPIDCounter);
							ss->nLastSecondByteCounter += OvOp[i][Idx[i]].Internal;
							LeaveCriticalSection(&ss->csPIDCounter);

							if (nRemainder != 0)
							{
								nNewOffset = nPacketLength - nRemainder;
								memcpy(&ss->tsb[nTSBufferIndex].pData[nRemainder],
									   DvBBuf[i]+Idx[i]*MaxBufferSize[i],
									   nNewOffset);
								if (fTimestampPackets)
								{
									if (ss->tsb[nTSBufferIndex].pTimestamps != NULL)
										memcpy(&ss->tsb[nTSBufferIndex].pTimestamps, DvBBuf[i]+Idx[i]*MaxBufferSize[i] + nNewOffset, 4);
									nNewOffset += 8;
								}
								nTSBufferIndex++;
								if (nTSBufferIndex == MAX_TS_BUFFERS)
								{
									//OutputDebugString("*");
									nTSBufferIndex = 0;
								}
								EnterCriticalSection(&ss->csTSBuffersInUse);
								ss->nTSBuffersInUse++;
								LeaveCriticalSection(&ss->csTSBuffersInUse);
							}
							
							nTSPacketCount = (OvOp[i][Idx[i]].Internal - nNewOffset) / nPacketLength;
							nRemainder = OvOp[i][Idx[i]].Internal - nNewOffset - (nTSPacketCount * nPacketLength);

							if (fTimestampPackets)
							{
								int nPacket;
								BYTE * pInputDataPtr = DvBBuf[i]+Idx[i]*MaxBufferSize[i] + nNewOffset;
								BYTE * pOutputDataPtr = ss->tsb[nTSBufferIndex].pData;

								for (nPacket = 0; nPacket < nTSPacketCount; nPacket++)
								{
									memcpy(pOutputDataPtr, pInputDataPtr, 188);
									pOutputDataPtr += 188;
									pInputDataPtr += 188;
									if (ss->tsb[nTSBufferIndex].pTimestamps != NULL)
										memcpy(&ss->tsb[nTSBufferIndex].pTimestamps[nPacket], pInputDataPtr, 4);
									pInputDataPtr += 4;
									pInputDataPtr += 4;
								}
								ss->tsb[nTSBufferIndex].nSize = nTSPacketCount * 188;
							}
							else
							{
								memcpy(ss->tsb[nTSBufferIndex].pData, DvBBuf[i]+Idx[i]*MaxBufferSize[i] + nNewOffset, nTSPacketCount * 188);
								ss->tsb[nTSBufferIndex].nSize = nTSPacketCount * 188;
							}
							
							nTSBufferIndex++;
							if (nTSBufferIndex == MAX_TS_BUFFERS)
								nTSBufferIndex = 0;
							EnterCriticalSection(&ss->csTSBuffersInUse);
							ss->nTSBuffersInUse++;
#ifdef DEBUG_MESSAGES
							if (ss->nTSBuffersInUse > MAX_TS_BUFFERS)
								OutputDebugString("|");
#endif DEBUG_MESSAGES
							LeaveCriticalSection(&ss->csTSBuffersInUse);

							if (nRemainder != 0)
							{
								ss->tsb[nTSBufferIndex].nSize = nPacketLength;
								memcpy(ss->tsb[nTSBufferIndex].pData,
									   DvBBuf[i]+Idx[i]*MaxBufferSize[i] + nNewOffset + (nTSPacketCount * nPacketLength),
									   nRemainder);
							}
#else LINEAR_SW_SYNC
						SourceHelper_SyncData(DvBBuf[i]+Idx[i]*MaxBufferSize[i], OvOp[i][Idx[i]].Internal);
#endif LINEAR_SW_SYNC
						}
					}
				}
				Started = TRUE;
			}
			OvOp[i][Idx[i]].hEvent = 0;             // used, if we did anything with it. Now initialize the OVERLAPPED structure for the next read.
			// Start read operation on all buffers that do not have a read operation pending already
			if(!ReadFileEx(DvB[i],DvBBuf[i]+Idx[i]*MaxBufferSize[i],MaxBufferSize[i],&OvOp[i][Idx[i]],FileIOCompletionRoutine))
			{
#ifdef DEBUG_MESSAGES
				OutputDebugString("Read error");
#endif DEBUG_MESSAGES
			}
			OpsPending[i]++;
			if(OpsPending[i] > MaxOpsPending[i])			
				ROLLOVER(Idx[i],NumBufs[i]);// Move to the next buffer in the read queue
		}

		//Waiting for a signal to shutdown, or for the asynchronous I/O operations to complete
		if((Rc = WaitForSingleObjectEx(ShutDown, WaitTime, TRUE)) == WAIT_OBJECT_0)
		{
			// The shutdown signal has arrived !!
			ULONG n;
			if(!DeviceIoControl(DvB[i],IOCTL_DVB_RX_PURGE,NULL,0,NULL,0,&n,NULL))	// re-initialize hardware
			{
#ifdef DEBUG_MESSAGES
				OutputDebugString("Error in DeviceIoCcontrol IOCTL_DVB_RX_PURGE\n");
#endif DEBUG_MESSAGES
				exit(1);
			}
			if(DvB[i]!= INVALID_HANDLE_VALUE)
				CancelIo(DvB[i]); // All the presently pending I/O operations should be cancelled with error status !
			break;
		}
		else if(Rc==WAIT_TIMEOUT)
		{
			if(DvB[i] != INVALID_HANDLE_VALUE)	// Rx has stopped, is recovery possible ?
			{
				ULONG n;
				//				if(Started)
				{
					CancelIo(DvB[i]);	 // Stop pending receives
					SleepEx(INFINITE,TRUE);	 // wait to complete
					if(!DeviceIoControl(DvB[i],IOCTL_DVB_RESET_REFRAME,NULL,0,NULL,0,&n,NULL))	// re-initialize hardware
					{
#ifdef DEBUG_MESSAGES
						OutputDebugString("Error in DeviceIoCcontrol IOCTL_DVB_RESET_REFRAME\n");
#endif DEBUG_MESSAGES
						exit(1);
					}
					Started=FALSE;
					//					printf("R");
				}
			}
		}
		
		OpsString[i]=0;
		// If it's not the shutdown, then some of the pending I/O operations have completed !!!
	} 

#ifdef LINEAR_SW_SYNC
	SourceHelper_StopSyncThread();
#endif LINEAR_SW_SYNC

	ss->fTerminateReadThread = FALSE;
	EnterCriticalSection(&ss->csTSBuffersInUse);
	ss->nTSBuffersInUse = -1000;
	LeaveCriticalSection(&ss->csTSBuffersInUse);

	ss->fReadThreadTerminated = TRUE;
	CloseHandle(ss->hReadDataThread);
	OutputDebugString("Linsys: DvB_ReceiveThread-\n");
	return 0;
}

BOOL TSReader_Stop() 
{
	int i;

	SetEvent(ShutDown);
	Sleep(10);
	while (ss->fReadThreadTerminated == FALSE)
		Sleep(50);

	for(i=0;i<NumHandles;i++)
	{
		VirtualFree(DvBBuf[Dir[i]],0,MEM_RELEASE);
		if(!CloseHandle(DvB[Dir[i]]))
		{
#ifdef DEBUG_MESSAGES
			OutputDebugString("CloseHandle fails");
#endif DEBUG_MESSAGES
		}
		if(!CloseHandle(StDvB[Dir[i]]))
		{
#ifdef DEBUG_MESSAGES
			OutputDebugString("CloseHandle fails");
#endif DEBUG_MESSAGES
		}
	}

	return TRUE;
}

BOOL TSReader_Start() 
{
	int TotLength=0;
	int MaxBufSize=0;
	int i;
	int BufTime;
	BOOL fAutoSize = FALSE;
#ifndef LINEAR_SW_SYNC
	BOOL fRxSyncEnbl = TRUE;
#else LINEAR_SW_SYNC
	BOOL fRxSyncEnbl = FALSE;
#endif LINEAR_SW_SYNC
	BOOL fDSMEnbl = FALSE;
	int MinWaitTime = 0;
	int m_Rate = 0;
	BOOL m_RlLow = FALSE;
	BOOL m_Rx204 = FALSE;

	ResetEvent(ShutDown);
	if(MinWaitTime==0)
		MinWaitTime = 65535;
	NumHandles=0;

	for(i=0;i<MaxTsfrDir;i++)	// open cards for each direction
	{
		ULONG n;
		HANDLE h,h1;
		struct DVBCfg Cfg;
		int instance;
		
		if (i == TxDir)
			continue;	// don't support that

		if(i==RxDir)
		{
			if (nMaximumCards != -1)
			{
				if (ss->nSourceIndex < nMaximumCards)
					instance = ss->nSourceIndex;
			}
		}
		h=GetDeviceViaInterface((LPGUID)&DVBLINK_GUID,instance,0);
		if (h == INVALID_HANDLE_VALUE)
			break;
		h1=h;
		
		//Just open all the device handles for the cards
		if(!DeviceIoControl(h1,IOCTL_DVB_RD_CFG,NULL,0,&Cfg,sizeof(Cfg),&n,NULL))
		{
#ifdef DEBUG_MESSAGES
			OutputDebugString("Error in DeviceIoCcontrol IOCTL_DVB_RD_CFG\n");
#endif DEBUG_MESSAGES
		}
		if(Cfg.DirSupported[i])	  // The card supports this direction
		{
			struct DVBStatus t;
			{
				struct DVBStats s;
				ULONG n;
				int fDvbFdCard=Cfg.DirSupported[TxDir];

				Cfg.OptionFlags &= ~DvbRxOptions;
				if(fAutoSize)	// implies
				{
					Cfg.OptionFlags|=RxAutoSize;
					fRxSyncEnbl = TRUE;							 
				}
//				Cfg.OptionFlags |= RxPidFilterEnable; 
				Cfg.OptionFlags &= ~RxPidFilterEnable; 
				if(fDSMEnbl)
					Cfg.OptionFlags |= RxDualSyncMode;
				else 
					Cfg.OptionFlags &= ~RxDualSyncMode;
				if(m_RlLow)
					Cfg.OptionFlags |= RLSetLow;
				else
					Cfg.OptionFlags &= ~RLSetLow;
				if(fDvbFdCard)  // The FD card
				{
					int R=0;
					
					if(fRxSyncEnbl)
						R=FD_Auto204to188;

					ClearRxMode(Cfg.OptionFlags);
					Cfg.OptionFlags |= SetRxMode(R);
#ifdef DEBUG_MESSAGES
					{
						char szTemp[100];
						wsprintf(szTemp, "RxMode %x, Options %x\n",R,Cfg.OptionFlags);
						OutputDebugString(szTemp);
					}
#endif DEBUG_MESSAGES
				}
				else
				{
					if(fRxSyncEnbl)
						Cfg.OptionFlags |= PktSyncEnbl;
					else
						Cfg.OptionFlags &= ~PktSyncEnbl;
					if(m_Rx204)
						Cfg.OptionFlags |= RxFrSz204;
					else
						Cfg.OptionFlags &= ~RxFrSz204;
				}
				if (fTimestampPackets)
				{
					if(Cfg.FpgaId > 4)
						Cfg.OptionFlags |= (RxRecoverTimeStamp | RxTimestampEnable);
					else
					{
						MessageBox(ss->hWndTSReader, "Can't enable timestamps - old FPGA", gszSourceName, MB_ICONWARNING);
						fTimestampPackets = FALSE;
					}
				}
				else
					Cfg.OptionFlags &= ~(RxRecoverTimeStamp | RxTimestampEnable);

				if(!DeviceIoControl(h1,IOCTL_DVB_SET_CFG,&Cfg,sizeof(Cfg),NULL,0,&n,NULL))
				{
#ifdef DEBUG_MESSAGES
					OutputDebugString("Error in DeviceIoCcontrol IOCTL_DVB_SET_CFG\n");
#endif DEBUG_MESSAGES
				}

				if(!DeviceIoControl(h1, IOCTL_DVB_RESET_RX, NULL, 0,NULL,0,&n,NULL))
				{
#ifdef DEBUG_MESSAGES
					OutputDebugString("Error in DeviceIoCcontrol IOCTL_DVB_RESET_REFRAME\n");
#endif DEBUG_MESSAGES
				}	

				// getting statistics from the driver
				if(!DeviceIoControl(h1,IOCTL_DVB_RD_ST,NULL,0,&s,sizeof(struct DVBStats),&n,NULL))
				{
#ifdef DEBUG_MESSAGES
					OutputDebugString("Error in DeviceIoCcontrol IOCTL_DVB_RD_ST\n");
#endif DEBUG_MESSAGES
				}
			}
			if(!DeviceIoControl(h1,IOCTL_DVB_RD_CFG,NULL,0,&Cfg,sizeof(Cfg),&n,NULL))
			{
#ifdef DEBUG_MESSAGES
				OutputDebugString("Error in DeviceIoCcontrol IOCTL_DVB_RD_CFG\n");
#endif DEBUG_MESSAGES
			}
			if(!DeviceIoControl(h1,IOCTL_DVB_GET_STATUS,NULL,0,&t,sizeof(struct DVBStatus),&n,NULL))
			{
#ifdef DEBUG_MESSAGES
				OutputDebugString("Error in DeviceIoCcontrol IOCTL_DVB_GET_STATUS\n");
#endif DEBUG_MESSAGES
				exit(1);
			}
			// Sort out which is the transmit & receive handles based on the configuration return
			h=GetDeviceViaInterface((LPGUID)&DVBLINK_GUID,instance,FILE_FLAG_OVERLAPPED);
			Dir[NumHandles++]=i;  // the direction for the card
			DvB[i]=h;	// All else keyed to direction
			StDvB[i]=h1;  // for statistics and control calls
			MaxBufferSize[i]=Cfg.MaxTransferSize[i];
			if(MaxBufSize<MaxBufferSize[i])
				MaxBufSize=MaxBufferSize[i];
			NumBufs[i]=Cfg.MaxBuffers[i]+Extra+2;
			TotLength+=NumBufs[i]*(MaxBufferSize[i]*sizeof(char)+sizeof(OVERLAPPED));
		}
		else
		{
#ifdef DEBUG_MESSAGES
			OutputDebugString("Dvb transfer direction not supported\n");
#endif DEBUG_MESSAGES
		}
	}
	if(NumHandles>2||NumHandles==0)
	{
#ifdef DEBUG_MESSAGES
		OutputDebugString("Invalid Number of DvB Devices\n");
#endif DEBUG_MESSAGES
	}
	
	TotLength *= 4;

	TotLength = ((TotLength/PAGE_SIZE)+300)*PAGE_SIZE;
	
	if(m_Rate>0)
		BufTime=(int)((double)MaxBufSize/((double)m_Rate/8.0));
	else
		BufTime=10000;
	WaitTime=(int)(BufTime*20.0*1000.0);
	if(WaitTime<MinWaitTime)
		WaitTime=MinWaitTime;

	{
		HANDLE hProcess=GetCurrentProcess();
		DWORD dwMinimumWorkingSetSize=TotLength;
		DWORD dwMaximumWorkingSetSize=TotLength;
		if(SetProcessWorkingSetSize(hProcess,dwMinimumWorkingSetSize,dwMaximumWorkingSetSize)==0)
		{
#ifdef DEBUG_MESSAGES
			OutputDebugString("SetProcessWorkingSetSize");
#endif DEBUG_MESSAGES
		}
		if(GetProcessWorkingSetSize(hProcess,&dwMinimumWorkingSetSize,&dwMaximumWorkingSetSize)==0)
		{
#ifdef DEBUG_MESSAGES
			OutputDebugString("GetProcessWorkingSetSize");
#endif DEBUG_MESSAGES
		}
	}

	for(i=0;i<NumHandles;i++)	// boards
	{
		int j=Dir[i];	// direction
		int k;

		DWORD Length= NumBufs[j]*MaxBufferSize[j]*sizeof(char)+NumBufs[j]*sizeof(OVERLAPPED);
		DvBBuf[j]=(char *)VirtualAlloc(NULL,Length,MEM_RESERVE,PAGE_READWRITE);
		DvBBuf[j]=(char *)VirtualAlloc(DvBBuf[j],Length,MEM_COMMIT,PAGE_READWRITE); 
		if(VirtualLock(DvBBuf[j],Length)==0)
		{
#ifdef DEBUG_MESSAGES
			OutputDebugString("VirtualLock");
#endif DEBUG_MESSAGES
		}
		OvOp[j]=(OVERLAPPED*)(DvBBuf[j]+NumBufs[j]*MaxBufferSize[j]*sizeof(char));

		memset(DvBBuf[j],0xaa,NumBufs[j]*MaxBufferSize[j]*sizeof(char));
		if(OvOp[j]==NULL||DvBBuf[j]==NULL)
		{
#ifdef DEBUG_MESSAGES
			OutputDebugString("No memory");
#endif DEBUG_MESSAGES
		}
		// Initialize overlapped structures for Thread I/O
		memset(OvOp[j],0,NumBufs[j]*sizeof(OVERLAPPED));
		for(k=0;k<NumBufs[j];k++)
		{
			OvOp[j][k].hEvent=(void *)2;
			OvOp[j][k].Internal=0;
		}
	}
	
	// Start the threads to do DVB reads & writes
	if(DvB[RxDir]!=INVALID_HANDLE_VALUE)
	{
		DWORD dwThreadID;

		ss->hReadDataThread = CreateThread(NULL, 0, DvB_ReceiveThread, (LPVOID)RxDir, CREATE_SUSPENDED, &dwThreadID);
		SourceHelper_SetWorkerThreadPriorities(FALSE);
		ResumeThread(ss->hReadDataThread);
	}

	return TRUE;
}

#ifdef BCM4500
BOOL GetBCM4500LockStatus()
{
	BYTE * LockData;

	switch(ss->nADVModulationMode)
	{
	case ADV_MOD_DVB_QPSK:
	case ADV_MOD_DCII_C_QPSK:
	case ADV_MOD_DCII_I_QPSK:
	case ADV_MOD_DCII_Q_QPSK:
	case ADV_MOD_DCII_C_OQPSK:
		if (bcm4500_get_legacy_status(&LockData))
		{
			ReadStatus();
			if ((LockData[2] & 0x09) == 0x09)
				return TRUE;
		}
		return FALSE;
	case ADV_MOD_TURBO_QPSK:
	case ADV_MOD_TURBO_8PSK:
	case ADV_MOD_TURBO_16QAM:
		if (bcm4500_get_turbo_status(&LockData))
		{
			ReadStatus();
			if ((LockData[2] & 0x02) == 0x02)
				return TRUE;
		}
		return FALSE;
	}
	return FALSE;
}
#endif BCM4500

BOOL TSReader_Tune()
{
#ifdef BCM4500
	BOOL StatusSuccess = FALSE;

	int nLBand;
	DWORD baud_offset_hz = 0;
	DWORD carrier_offset_hz = 0;
	DWORD dwLockTime;
	DWORD dwLockTimeout = 3000;
	BYTE byte2 = 0;
	BYTE byte3 = 0;
	BYTE ctl_flags1 = 0x08;	// ERR/SYNC/VALID all active high CLKINV
	BYTE ctl_flags2 = 0x00;
	BYTE ctl_flags3 = 0xeb;

	if (ss->nFrequency > ss->nLNBFrequency)
		nLBand = ss->nFrequency - ss->nLNBFrequency;
	else
		nLBand = ss->nLNBFrequency - ss->nFrequency;

	bcm4500_tune(nLBand * 1000000);	
	bcm4500_edit_symbol_rate_list(1, ss->nSymbolRate * 1000);
	if (ss->nSymbolRate < 10000)
		dwLockTimeout = 10000;

	switch(ss->nADVModulationMode)
	{
	case ADV_MOD_DVB_QPSK:
		switch(ss->nCodeRate)
		{
		case 0:		// 1/2
			byte2 |= 0x10;
			break;
		case 1:		// 2/3
			byte2 |= 0x20;
			break;
		case 2:		// 3/4
			byte2 |= 0x30;
			break;
		case 3:		// 5/6
			byte2 |= 0x40;
			break;
		case 4:		// 7/8
			byte2 |= 0x60;
			break;
		default:
			//byte2[7..4] = 0 for DVB scan
			break;
		}
		break;
	case ADV_MOD_DCII_C_QPSK:
	case ADV_MOD_DCII_I_QPSK:
	case ADV_MOD_DCII_Q_QPSK:
	case ADV_MOD_DCII_C_OQPSK:
		byte2 |= 0xf0;
		switch(ss->nCodeRate)
		{
		case 0:		// 5/11
			byte3 |= 0x10;
			break;
		case 1:		// 1/2
			byte3 |= 0x20;
			break;
		case 2:		// 3/5
			byte3 |= 0x30;
			break;
		case 3:		// 2/3
			byte3 |= 0x40;
			break;
		case 4:		// 3/4
			byte3 |= 0x50;
			break;
		case 5:		// 4/5
			byte3 |= 0x60;
			break;
		case 6:		// 5/6
			byte3 |= 0x70;
			break;
		case 7:		// 7/8
			byte3 |= 0x80;
			break;
		default:
			//byte3[7..4] = 0 for DCII scan
			break;
		}
		if (ss->nADVModulationMode == ADV_MOD_DCII_C_QPSK)
			;	// ctl_flags1[2..0] = 0;
		else if (ss->nADVModulationMode == ADV_MOD_DCII_I_QPSK)
			ctl_flags1 |= 0x02;
		else if (ss->nADVModulationMode == ADV_MOD_DCII_Q_QPSK)
			ctl_flags1 |= 0x06;
		else if (ss->nADVModulationMode == ADV_MOD_DCII_C_OQPSK)
			ctl_flags1 |= 0x01;
		break;
	case ADV_MOD_TURBO_QPSK:
		byte2 |= 0x01;	// set turbo mode
		ctl_flags2 |= 0x01;	// turbo MPEG header re-insert
		switch(ss->nCodeRate)
		{
		case 0:		// 1/4
			byte2 |= 0x90;
			break;
		case 1:		// 1/2
			//byte2[7..4] = 0 for Turbo QPSK 1/2
			break;
		case 2:		// 3/4
			byte2 |= 0x10;
			break;
		}
		break;
	case ADV_MOD_TURBO_8PSK:
		byte2 |= 0x01;	// set turbo mode
		ctl_flags2 |= 0x01;	// turbo MPEG header re-insert
		switch(ss->nCodeRate)
		{
		case 0:		// 2/3
			byte2 |= 0x20;
			break;
		case 1:		// 3/4 I
			byte2 |= 0x50;
			break;
		case 2:		// 3/4 II
			byte2 |= 0x70;
			break;
		case 3:		// 5/6
			byte2 |= 0x30;
			break;
		case 4:		// 8/9
			byte2 |= 0x40;
			break;
		}
		break;
	case ADV_MOD_TURBO_16QAM:
		byte2 |= 0x01;	// set turbo mode
		ctl_flags2 |= 0x01;	// turbo MPEG header re-insert
		byte2 |= 0x60;	// 3/4 QAM
		break;
	}

	byte2 |= 0x08;		// parallel data out
	byte2 |= 0x04;		// MPEG2
	byte3 |= 0x01;		// symbol rate #1
	bcm4500_acquire2(byte2, byte3, baud_offset_hz, carrier_offset_hz, ctl_flags1, ctl_flags2, ctl_flags3);

	dwLockTime = GetTickCount();	
	while (!StatusSuccess)
	{
		DWORD dwCountNow = GetTickCount() - dwLockTime;
		if (dwCountNow > dwLockTimeout)
		{
			OutputDebugString("Linsys4500: Tune Timeout\n");
			break;
		}

		StatusSuccess = GetBCM4500LockStatus();
		Sleep(1);
	}

	if (!StatusSuccess)
	{
		if (ss->fAutoXMLExport == FALSE)
		{
			MessageBox(ss->hDlgSIParser, "Failed to lock signal", gszSourceName, MB_ICONWARNING);
			return FALSE;
		}
	}

	return StatusSuccess;

#else BCM4500
	if (ss->fSerialReceiverControlEnabled && !fDontTune)
	{
		if (SourceHelper_TuneSerialControl(szLastTune) == FALSE)
			return FALSE;
	}
	return TRUE;
#endif BCM4500
}

BOOL DoTuneDialog(HWND hWnd)
{
	if (ss->fDontTune == TRUE)
		fNeedTuneDialog = FALSE;

	if (fNeedTuneDialog)
	{
		OutputDebugString("Linsys: TSReader_TuneDialog tuning dialog is required\n");
		fDontTune = FALSE;
		if (SourceHelper_ADVTuneDialog(hWnd) == FALSE)
			fDontTune = TRUE;
	}
	else
	{
		OutputDebugString("Linsys: TSReader_TuneDialog tune NOT required\n");
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

	return TRUE;
}

BOOL TSReader_TuneDialog(HWND hWnd)
{
	OutputDebugString("Linsys: TSReader_TuneDialog\n");

#ifdef BCM4500
	return DoTuneDialog(hWnd);
#else BCM4500
	if (ss->fSerialReceiverControlEnabled == TRUE)
		return DoTuneDialog(hWnd);
#endif BCM4500

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
#ifndef BCM4500
	if (szCommandLineParameters != NULL)
		lstrcpy(szCommandLineParameters, "None");
#else BCM4500
	if (szCommandLineParameters != NULL)
		lstrcpy(szCommandLineParameters, "freq pol sr mod fec lnbf 22khz {input}");
#endif BCM4500
	if (fCanBeStopped != NULL)
		*fCanBeStopped= FALSE;
	if (nMaxPIDs != NULL)
		*nMaxPIDs = 8192;
	if (dwCapabilities != NULL)
#ifdef BCM4500
		*dwCapabilities = 0;
#else BCM4500
#ifndef _DEBUG
		*dwCapabilities = CAPABILITIES_SERIAL_CONTROL
			            | CAPABILITIES_MULTICARD;
#else _DEBUG
		*dwCapabilities = CAPABILITIES_SERIAL_CONTROL
						| CAPABILITIES_TIMESTAMP
						| CAPABILITIES_MULTICARD;
#endif _DEBUG
#endif BCM4500

	return TRUE;
}

#ifndef BCM4500
BOOL TSReader_ParseCommandLine(PSOURCESTRUCT ss, char * szCommandLine, BOOL fQuiet)
{
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
	return TRUE;
}
#else BCM4500
BOOL TSReader_ParseCommandLine(PVARIABLES v, char * szCommandLine, BOOL fQuiet)
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

	return TRUE;
}
#endif BCM4500

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
	lstrcpy(szString, "n/a");
	return TRUE;
}

BOOL TSReader_GetTunerString(char * szString)
{
	lstrcpy(szString, "n/a");
	return TRUE;
}

BOOL TSReader_SendDiSEqC(BYTE * bCommand, int nLength)
{
	return TRUE;
}

int TSReader_GetSyncLossCount(BOOL fReset)
{
#ifdef LINEAR_SW_SYNC
	return SourceHelper_GetSyncLossCount(fReset);
#else LINEAR_SW_SYNC
	return 0;
#endif LINEAR_SW_SYNC
}

BOOL APIENTRY DllMain(HANDLE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
    switch( ul_reason_for_call )
	{
    case DLL_PROCESS_ATTACH:
		hInstance = hModule;
		break;
    case DLL_PROCESS_DETACH:
		break;
    }
    return TRUE;
}

