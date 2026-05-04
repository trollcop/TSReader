#include <windows.h>
#include <commctrl.h>
#include "initguid.h"
#include "winioctl.h"
#include "setupapi.h"	// VC++ 5 one is out of date
#include <stdio.h>

#include "..\forwarder.h"

#include "dvbIoctl.h"
#define MaxTsfrDir MaxPlxDmaChannels
#define QueuedExtras 2
#define PAGE_SIZE 4096
#define Thread    __declspec( thread )
#define ROLLOVER(x,y) if((x)==((y)-1))(x)=0; else (x)++;

typedef struct _DvbStuffing
{
	double DesiredRate,PacketSize,RateError,InterFrame;
	int InterByte,FT0,FT1;
	BOOL fNoIb;
} DvbStuffing;

static char gszModuleName[] = {"Linear Systems Forwarder Module"};

BOOL fFirstTime;
BOOL fWriteThreadTerminated;
int NumHandles=0;
int Dir[MaxTsfrDir];
int SyncPacketSize;
int Extra=QueuedExtras;
int MaxBufferSize[MaxTsfrDir],NumBufs[MaxTsfrDir];
int Idx[MaxTsfrDir]={0,0};
int OpsPending[MaxTsfrDir]={0,0};
int OpsString[MaxTsfrDir]={0,0};
int MaxOpsPending[MaxTsfrDir]={0,0};
int MaxOpsString[MaxTsfrDir]={0,0};
int nWriteBufferOffset;
int nCurrentElement;
int nElementRemaining;

HANDLE ShutDown;
HANDLE DvB[MaxTsfrDir]={INVALID_HANDLE_VALUE,INVALID_HANDLE_VALUE};
OVERLAPPED *OvOp[MaxTsfrDir];
char *DvBBuf[MaxTsfrDir];

// This is the I/O completion routine called by the operating system when I/O has
// completed on a I/O request (read or write)
VOID CALLBACK FileIOCompletionRoutine(
	DWORD dwErrorCode,                // completion code
	DWORD dwNumberOfBytesTransfered,  // number of bytes transferred
	LPOVERLAPPED lpOverlapped)         // pointer to structure with I/O information
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
		OutputDebugString("No HDEVINFO available for this GUID\n");
		return INVALID_HANDLE_VALUE;
	}

	// Get interface data for the requested instance
	ifdata.cbSize = sizeof(ifdata);
	if(!SetupDiEnumDeviceInterfaces(info, NULL, pGuid,instance, &ifdata))
	{
		OutputDebugString("No SP_INTERFACE_DEVICE_DATA available for this GUID instance\n");
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
	rv = CreateFile(ifDetail->DevicePath, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, ovr, NULL);

	free(ifDetail);
	SetupDiDestroyDeviceInfoList(info);
	return rv;
}

DWORD WINAPI DvB_TransmitThread(LPVOID lpv)
{
	// DvBBuf[TxDir][] are data buffers for the read and write operations
	// OvOp[TxDir][] are the OVERLAPPED structures! Read MS documentation!
	// Idx[TxDir] is the index in the CIO queued buffer list of pending write operations
	int Rc;
	int i= TxDir;	
	static int l;
	int k=0;
	ULONG Seq=0;
	int MaxNum=MaxBufferSize[i]/4;
	int SyncSize=SyncPacketSize/4;

	l=1;
	while(TRUE)
	{
		// Retry after errors
		if(DvB[i] != INVALID_HANDLE_VALUE)  // there is a transmitter card
		{
			while(OvOp[i][Idx[i]].hEvent)
			{
				if(OvOp[i][Idx[i]].hEvent == (void *)1) // Post initialization, actual data has been transmitted !
				{
					OpsString[i]++;
					if(OpsString[i]>MaxOpsString[i])
						MaxOpsString[i]=OpsString[i];
					OpsPending[i]--;
				}
				// used, if we did anything with it. Now initialize the OVERLAPPED structure for the next read
				OvOp[i][Idx[i]].hEvent=0;
				
				// Start write operation on all buffers that do not have a write operation pending already
				if(!WriteFileEx(DvB[i], DvBBuf[i]+Idx[i]*MaxBufferSize[i], MaxBufferSize[i], &OvOp[i][Idx[i]], FileIOCompletionRoutine))
				{
					OutputDebugString("Write error\n");
				}
				OpsPending[i]++;
				if(OpsPending[i]>MaxOpsPending[i])
					MaxOpsPending[i]=OpsPending[i];
				// Move to the next buffer in the write queue
				ROLLOVER(Idx[i],NumBufs[i]);
			}
		}
		if((Rc=WaitForSingleObjectEx(ShutDown, 5, TRUE))==WAIT_OBJECT_0)
		// Waiting for a signal to shutdown, or for the asynchronous I/O operations to complete
		{
			if(DvB[i] != INVALID_HANDLE_VALUE)
				CancelIo(DvB[i]); // All the presently pending I/O operations should be cancelled with error status !
			break;
		}
		OpsString[i]=0;
	} 

	fWriteThreadTerminated = TRUE;
	return 0;
}

void CalcStuffingFromRate(DvbStuffing *s)
{
	double TS=27.0e6*8.0*(double)s->PacketSize/s->DesiredRate-(double)s->PacketSize-2.0;
	int TSI=(int)TS;
	double R1t0=(TS-(double)TSI)/((double)TSI+1.0-TS);
	s->InterByte = (int)(27.e6/(s->DesiredRate/8.0) - 1.0);
	if(s->fNoIb)s->InterByte=0;
	if((R1t0<1.0||(int)((255.0/R1t0)+.5)>=1)&&((int)((255.0*R1t0)+.5))>=1)
	{
		double difmin=1.e6;
		s->InterFrame=TSI-(s->PacketSize-1.0)*(double)s->InterByte;
		if(R1t0>1.0)  // FT1 <= 255
		{
			int i,j=(int)((255.0/R1t0)+.5);
			for(i=1;i<=j;i++)	// increment FT0
			{
				int k=(int)((double)i*R1t0+.5);
				double TTS=(double)(i*TSI+k*(TSI+1))/(double)(i+k);
				double dif=TTS-TS;
				if(dif<0.0) dif*= (-1.);
				if(dif< difmin)
				{
					difmin=dif;
					s->FT0=i;
					s->FT1=k;
				}
			}
		}
		else	 // FT0 <= 255
		{
			int i,j=(int)((255.0*R1t0)+.5);
			for(i=1;i<=j;i++)	// increment FT1
			{
				int k=(int)((double)i/R1t0+.5);
				double TTS=(double)(k*TSI+i*(TSI+1))/(double)(i+k);
				double dif=TTS-TS;
				if(dif<0.0) dif*= (-1.);
				if(dif< difmin)
				{
					difmin=dif;
					s->FT0=k;
					s->FT1=i;
				}
			}
		}
		s->RateError= 1.0e6 * (difmin/TS);
	}
	else
	{
		double Rt;
		s->InterFrame=8.0*27.e6*(double)s->PacketSize/s->DesiredRate-s->PacketSize-2-(s->PacketSize-1)*s->InterByte;
//		if(R1t0<1.0)
		Rt= (8.0*(double)s->PacketSize*(27.0e6/((double)s->PacketSize+2.0+(double)((s->PacketSize-1)*(s->InterByte)) + s->InterFrame)));
		s->RateError=(s->DesiredRate-Rt)/s->DesiredRate;
		s->RateError *= 1.e6;
		s->FT0=1;
		s->FT1=0;
	}	
}

BOOL TSReader_Fwd_Init(HWND hWnd, int nPacketLength, int nBitRate)
{
	int TotLength = 0, MaxBufSize = 0;
	int i;
	BOOL m_FL204;
	DvbStuffing stuffing;

	ShutDown = CreateEvent(NULL, TRUE, FALSE, NULL);

	NumHandles = 0;
	if (nPacketLength == 188)
		m_FL204 = FALSE;
	else
		m_FL204 = TRUE;
	SyncPacketSize=(m_FL204)?204:188;


	stuffing.DesiredRate = (double)nBitRate;
	stuffing.PacketSize = ((m_FL204)?204.0:188.0);
	stuffing.fNoIb = 1;
	CalcStuffingFromRate(&stuffing);

	for(i=0;i< MaxTsfrDir; i++)	// open cards for each direction
	{
		ULONG n;
		HANDLE h,h1;
		struct DVBCfg Cfg;
		int instance;

		if (i == TxDir)
			instance = 0;	

		h=GetDeviceViaInterface((LPGUID)&DVBLINK_GUID,instance,0);
		if (h == INVALID_HANDLE_VALUE)
			break;
		h1=h;

		// Just open all the device handles for the cards
		if(!DeviceIoControl(h1,IOCTL_DVB_RD_CFG,NULL,0,&Cfg,sizeof(Cfg),&n,NULL))
		{
			OutputDebugString("Error in DeviceIoCcontrol IOCTL_DVB_RD_CFG\n");
		}
		if(Cfg.DirSupported[i])	  // The card supports this direction
		{
			struct DVBStats s;
			struct DVBStatus t;
			ULONG n;

			if(i==TxDir)  // Transmit
			{
				int fDvbFdCard=Cfg.DirSupported[RxDir];

				Cfg.OptionFlags &= ~DvbTxOptions;
				if(fDvbFdCard)  // The FD card
				{
					int T=0;
					ClearTxMode(Cfg.OptionFlags);
					if(m_FL204)
						T=1;
					Cfg.OptionFlags |= SetTxMode(T);
				}
				else
				{
					if(m_FL204)
						Cfg.OptionFlags |= FrSz204;
				}
				
				// Illustrating set stuffing, should close & re-open device handle if seriously changing stuffing !
				Cfg.FT0 = stuffing.FT0;
				Cfg.FT1 = stuffing.FT1;
				Cfg.Stuffing = stuffing.InterByte & 0xff | (((int)stuffing.InterFrame)<<8);

				if(!DeviceIoControl(h1,IOCTL_DVB_SET_CFG,&Cfg,sizeof(Cfg),NULL,0,&n,NULL))
				{
					OutputDebugString("Error in DeviceIoCcontrol IOCTL_DVB_SET_CFG\n");
				}
				if(!DeviceIoControl(h1,IOCTL_DVB_RESET_TX,NULL,0,NULL,0,&n,NULL))
				{
					OutputDebugString("Error in DeviceIoCcontrol IOCTL_DVB_RESET_REFRAME\n");
				}	
				if(!DeviceIoControl(h1,IOCTL_DVB_RD_ST,NULL,0,&s,sizeof(struct DVBStats),&n,NULL))
					// getting statistics from the driver
				{
					OutputDebugString("Error in DeviceIoCcontrol IOCTL_DVB_RD_ST");
				}
			}
			
			if(!DeviceIoControl(h1,IOCTL_DVB_RD_CFG,NULL,0,&Cfg,sizeof(Cfg),&n,NULL))
			{
				OutputDebugString("Error in DeviceIoCcontrol IOCTL_DVB_RD_CFG\n");
			}
			if(!DeviceIoControl(h1,IOCTL_DVB_GET_STATUS,NULL,0,&t,sizeof(struct DVBStatus),&n,NULL))
			{
				OutputDebugString("Error in DeviceIoCcontrol IOCTL_DVB_GET_STATUS\n");
			}
			
			// Sort out which is the transmit & receive handles based on the configuration return
			h=GetDeviceViaInterface((LPGUID)&DVBLINK_GUID,instance,FILE_FLAG_OVERLAPPED);
			Dir[NumHandles++]=i;  // the direction for the card
			DvB[i]=h;	// All else keyed to direction
			MaxBufferSize[i]=Cfg.MaxTransferSize[i];
			if(TRUE)
				MaxBufferSize[i]=(Cfg.MaxTransferSize[i]/SyncPacketSize)*SyncPacketSize;
			if(MaxBufSize<MaxBufferSize[i])
				MaxBufSize=MaxBufferSize[i];
			NumBufs[i]=Cfg.MaxBuffers[i]+Extra+2;
			TotLength+=NumBufs[i]*(MaxBufferSize[i]*sizeof(char)+sizeof(OVERLAPPED));
		}
		else
			OutputDebugString("Dvb transfer direction not supported\n");
	}
	if(NumHandles>2||NumHandles==0)
	{
		OutputDebugString("Invalid Number of DvB Devices\n");
	}
	TotLength = ((TotLength/PAGE_SIZE)+300)*PAGE_SIZE;

	for(i=0;i<NumHandles;i++)	// boards
	{
		int j=Dir[i];	// direction
		int k;

		DvBBuf[j]=(char *)malloc(NumBufs[j]*MaxBufferSize[j]*sizeof(char));
		OvOp[j]=(OVERLAPPED *)malloc(NumBufs[j]*sizeof(OVERLAPPED));

		// Initialize overlapped structures for Thread I/O
		memset(OvOp[j],0,NumBufs[j]*sizeof(OVERLAPPED));
		for(k=0;k<NumBufs[j];k++)
		{
			OvOp[j][k].hEvent=(void *)0;	// means empty
			OvOp[j][k].Internal=0;
		}
	}
	
	// Start the threads to do DVB reads & writes
	if(DvB[TxDir] != INVALID_HANDLE_VALUE)
	{
		DWORD dwThreadID;
		HANDLE hReadDataThread;
		
		fWriteThreadTerminated = FALSE;

		hReadDataThread = CreateThread(NULL, 0, DvB_TransmitThread, (LPVOID)TxDir, CREATE_SUSPENDED, &dwThreadID);
		ResumeThread(hReadDataThread);

	}

	nWriteBufferOffset = 0;
	nCurrentElement = 0;
	nElementRemaining = MaxBufferSize[TxDir];
	fFirstTime = TRUE;
	return TRUE;
}

BOOL TSReader_Fwd_DeInit()
{
	SetEvent(ShutDown);
	while (!fWriteThreadTerminated)
		Sleep(50);
	CloseHandle(ShutDown);
	return TRUE;
}

BOOL TSReader_Fwd_Data(BYTE * pBuffer, int nLength)
{
	int j = Dir[TxDir];	// direction

	if (nWriteBufferOffset + nLength >= NumBufs[j] * MaxBufferSize[j])
	{
		// About to wrap around the end of the ring buffer
		int nShortLength = (NumBufs[j] * MaxBufferSize[j]) - nWriteBufferOffset;
		if (nShortLength > 0)
			memcpy(&DvBBuf[j][nWriteBufferOffset], pBuffer, nShortLength);
		OvOp[j][nCurrentElement].hEvent = (void *)2;	// buffer ready to TX
		nCurrentElement = 0;
		nWriteBufferOffset = 0;
		memcpy(&DvBBuf[j][nWriteBufferOffset], &pBuffer[nShortLength], nLength - nShortLength);
		nWriteBufferOffset = nLength - nShortLength;
		nElementRemaining -= nLength - nShortLength;
	}
	else
	{
		memcpy(&DvBBuf[j][nWriteBufferOffset], pBuffer, nLength);
		nWriteBufferOffset += nLength;
		nElementRemaining -= nLength;
		if (nElementRemaining <= 0)
		{
			if (fFirstTime == TRUE)
			{
				nCurrentElement++;
				if (nCurrentElement == NumBufs[j] / 2)
				{
					int x;
					for (x = 0; x < NumBufs[j] / 2; x++)
						OvOp[j][x].hEvent = (void *)2;	// buffer ready to TX
					fFirstTime = FALSE;
				}
			}
			else
				OvOp[j][nCurrentElement++].hEvent = (void *)2;	// buffer ready to TX
			nElementRemaining = MaxBufferSize[j] + nElementRemaining;
		}
	}
	return TRUE;
}

BOOL TSReader_Fwd_GetDescription(char * szDeviceNameBuffer)
{
	lstrcpy(szDeviceNameBuffer, "Linear Systems ASI");
	return TRUE;
}

BOOL APIENTRY DllMain(HANDLE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
    switch( ul_reason_for_call )
	{
    case DLL_PROCESS_ATTACH:
		break;
    case DLL_PROCESS_DETACH:
		break;
    }
    return TRUE;
}

