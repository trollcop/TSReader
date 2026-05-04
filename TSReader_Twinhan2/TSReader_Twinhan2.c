#ifndef DVBS
#ifndef DVBT
#ifndef DVBC
#ifndef EIGHTVSB
#ifndef NOTUNE
------ARRRGH - DEFINES ARE WRONG--------
#endif
#endif
#endif
#endif
#endif

#include <windows.h>
#include <commctrl.h>
#include <setupapi.h>
#include <initguid.h>
#include <stdio.h>
#ifdef EIGHTVSB
#include <math.h>
#endif EIGHTVSB

#include "..\sources.h"
#include "TSReader_Twinhan2.h"

const int nBufferLength = 3 * 4092 * 255;

PSOURCESTRUCT ss;
HANDLE hDevice1, hDevice2;
HANDLE hPipeRead;
BYTE * pDriverBuffer;
BOOL fPowerOn;
BOOL fPipeThreadTerminated = FALSE;
BOOL fNeedTuneDialog = TRUE;
int nFrequency;
#ifdef DVBS
int nPolarity;
int nSymbolRate;
int nLNBFrequency;
int n22KHz;
int nDiSEqCInput;
#endif DVBS
#ifdef DVBT
int nBandwidth;
BOOL fSpectrumInversion;
#endif DVBT
#ifdef DVBC
int nSymbolRate;
int nQAM;
int nBandwidth;
BOOL fSpectrumInversion;
#endif DVBC
int nCardType;
int nRetuneCount;
CRITICAL_SECTION csSignal;
CRITICAL_SECTION csHardware;

#ifdef TWINHAN1020
#define SATELLITE
 #ifdef DSS
  char gszSourceName[] = {"Twinhan 1020a DSS"};
 #else DSS
  char gszSourceName[] = {"Twinhan 1020/1020a/102g DVB-S"};
 #endif DSS
#endif TWINHAN1020
  
#ifdef TWINHAN1030
#define SATELLITE
 #ifndef PINNACLE
  char gszSourceName[] = {"Twinhan 1030/1030a/1032 DVB-S"};
#else PINNACLE
  char gszSourceName[] = {"Pinnacle Sat CI DVB-S"};
#endif PINNACLE
#endif TWINHAN1030

#ifdef DVBT
 #ifndef CI_DVBT
  char gszSourceName[] = {"Twinhan DTT DVB-T"};
 #else CI_DVBT
  char gszSourceName[] = {"Twinhan DTT-CI DVB-T"};
 #endif CI_DVBT
#endif DVBT

#ifdef DVBC
 #ifndef CI_DVBC
  char gszSourceName[] = {"Twinhan DCT DVB-C"};
 #else CI_DVBC
  char gszSourceName[] = {"Twinhan DCT-CI DVB-C"};
 #endif CI_DVBC
#endif DVBC

#ifdef NOTUNE
  char gszSourceName[] = {"Twinhan non-tuning"};
#endif NOTUNE

#ifdef EIGHTVSB
  char gszSourceName[] = {"Twinhan 8VSB (VP3250/VP3220)"};
  BOOL fNXT2004InitDone = FALSE;
#endif EIGHTVSB

char szLastTune[128] = {"n/a"};
char szLastSignalReport[128] = {"n/a"};

BOOL __cdecl SourceHelper_ValidateSourceContainer(PSOURCESTRUCT pss);

// Hardware specific stuff
unsigned char MakeCheckSum(TunerData *pTunerData)
{
	unsigned char *pByte = (unsigned char *)pTunerData;
	unsigned char chksum = 0;
	int i;

	for (i = 1; i < 8; i++ )
		chksum += pByte[i];

	chksum = ~chksum+1;
	pByte[8] = chksum;
	return chksum;
} 

HANDLE GetDeviceHandle(GUID ClassGuid,
                       PSP_INTERFACE_DEVICE_DETAIL_DATA functionClassDeviceData)
{
    HDEVINFO                 hardwareDeviceInfo;
    SP_INTERFACE_DEVICE_DATA deviceInfoData;
    DWORD                    predictedLength = 0;
    DWORD                    requiredLength = 0;
    HANDLE                   hDev = INVALID_HANDLE_VALUE;

    // Open a handle to the plug and play dev node.
    hardwareDeviceInfo = SetupDiGetClassDevs((LPGUID)&ClassGuid,
                                         NULL,
                                         NULL, // Define no
                                         (DIGCF_PRESENT | // Only Devices present
                                         DIGCF_INTERFACEDEVICE)); // Function class devices.
    if(0 == hardwareDeviceInfo) 
	{
		OutputDebugString("Twinhan2: GetDeviceHandle SetupDiGetClassDevs() failed\n");
        return (hDev);
    }

    deviceInfoData.cbSize = sizeof (SP_INTERFACE_DEVICE_DATA);

    if (SetupDiEnumDeviceInterfaces (hardwareDeviceInfo,
                                 0,
                                 (LPGUID)&ClassGuid,
                                 0,
                                 &deviceInfoData))
    {
        SetupDiGetInterfaceDeviceDetail(
            hardwareDeviceInfo,
            &deviceInfoData,
            NULL,               // probing so no output buffer yet
            0,                  // probing so output buffer length of zero
            &requiredLength,
            NULL);              // not interested in the specific dev-node


       predictedLength = requiredLength;

       functionClassDeviceData =
            (PSP_INTERFACE_DEVICE_DETAIL_DATA)malloc(predictedLength);
       functionClassDeviceData->cbSize = sizeof(SP_INTERFACE_DEVICE_DETAIL_DATA);

       if (!SetupDiGetInterfaceDeviceDetail(hardwareDeviceInfo,
                                        &deviceInfoData,
                                        functionClassDeviceData,
                                        predictedLength,
                                        &requiredLength,
                                        NULL))
       {
			OutputDebugString("Twinhan2: GetDeviceHandle SetupDiGetInterfaceDeviceDetail() failed\n");
			free(functionClassDeviceData);
			return (hDev);
       }
   
    }
    else if (ERROR_NO_MORE_ITEMS != GetLastError())
	{
		OutputDebugString("Twinhan2: GetDeviceHandle SetupDiEnumDeviceInterfaces() failed\n");
        free(functionClassDeviceData);
        SetupDiDestroyDeviceInfoList(hardwareDeviceInfo);
        return (hDev);
    }

    SetupDiDestroyDeviceInfoList(hardwareDeviceInfo);
    if (functionClassDeviceData == NULL)
	{
		OutputDebugString("Twinhan2: GetDeviceHandle SetupDiDestroyDeviceInfoList() failed\n");
        return (hDev);
    }

    hDev = CreateFile(functionClassDeviceData->DevicePath, 
                      GENERIC_READ | GENERIC_WRITE,
                      0,
                      NULL, 
                      OPEN_EXISTING,
                      0, 
                      NULL);
    if (hDev == INVALID_HANDLE_VALUE)
	{
		OutputDebugString("Twinhan2: GetDeviceHandle CreateFile(functionClassDeviceData->DevicePath) failed\n");
         free(functionClassDeviceData);
         return (hDev);
    }

    return (hDev);
}

int GetCardType()
{
    BOOL			StatusSuccess = TRUE;
	LPVOID			inBuf;
	LPVOID			outBuf; 
	DWORD			inBufLength;
	DWORD			outBufLength;
    ULONG			bytesReturned;
	BYTE			bBuf[256];
	char *chName;
	
	TunerData tunerData;
	memset(&tunerData,0,sizeof(TunerData));

	tunerData.address = 0xaa;
	tunerData.frequencyMSB = 0;
	tunerData.frequencyLSB = 0x06;
	MakeCheckSum(&tunerData);
	
	inBuf = &tunerData;
	inBufLength	= sizeof(tunerData);		
	outBuf = (LPVOID)bBuf;
	outBufLength = inBufLength;
	
	StatusSuccess = DeviceIoControl(hDevice2, 
									(DWORD)DST_IOCTL_SET_INFO, 
									inBuf, 
									inBufLength + 1, 
									outBuf, 
									outBufLength, 
									&bytesReturned, 
									NULL);
	if (!StatusSuccess)
	{
		OutputDebugString("Twinhan2: GetCardType() IOCTL failed\n");
		return -1;
	}

	//0 : Old DST Card					--- 188bytes;
	//1 : New DST Card(USALS)			--- 188bytes;
	//2 : New DST Card(USALS, 3 Tuners) --- 204bytes;
	//3 : DST CI Card					--- 204bytes;
	//4 : DST CI Card(188 Bytes)		--- 188bytes;
	//5 : DCT Card						--- 188bytes;
	//6 : DCT CI Card					--- 204bytes;
	//10: DTT Card(Digtal & Analog)
	//11: DTT Card(Digtal)
	chName = (char *)((BYTE *)outBuf + 2);
	chName[6] = 0;

	{
		char szDebug[128];
		wsprintf(szDebug, "Twinhan2: API says card type is \"%s\"\n", chName);
		OutputDebugString(szDebug);
	}

	if (0 == strcmp(chName,"ST-MOT"))
		return 1;
	else if (0 == strcmp(chName,"ST-020"))
		return 1;
	else if (0 == strcmp(chName,"ST-03T"))
		return 2;
	else if (0 == strcmp(chName,"DST-CI"))
		return 3;
	else if (0 == strcmp(chName,"DSTMCI"))	
		return 4;
	else if (0 == strcmp(chName,"DCTNEW"))
		return 5;
	else if (0 == strcmp(chName,"DCT-CI"))
		return 6;
	else if (0 == strcmp(chName,"DTTNXT"))
		return 10;
	else if (0 == strcmp(chName,"DTTDIG"))
		return 11;
	else if (0 == strcmp(chName, "ATSCAD"))
		return 12;

	return 0;
}

DWORD GetDriverBufferAddress()
{
	BYTE outBuf[4];
	DWORD outBufLength = sizeof(outBuf);
	DWORD bytesReturned;
	DWORD dwRetVal;

    DeviceIoControl(hDevice1, 
                    (DWORD)DST_IOCTL_GET_DATABUFF_ADDR, 
                    NULL, 
                    0, 
                    outBuf, 
                    outBufLength, 
                    &bytesReturned, 
                    NULL);

	memcpy(&dwRetVal, outBuf, sizeof(DWORD));
	return dwRetVal;
}

int GetDriverWritePtr()
{
	BYTE outBuf[4];
	DWORD outBufLength = sizeof(outBuf);
	DWORD bytesReturned;
	int nRetVal;

	EnterCriticalSection(&csHardware);
    nRetVal = DeviceIoControl(hDevice1, 
                    (DWORD)DST_IOCTL_GET_DATABUFF_BLOCK, 
                    NULL, 
                    0, 
                    outBuf, 
                    outBufLength, 
                    &bytesReturned, 
                    NULL);
	if (!nRetVal)
		OutputDebugString("Twinhan2: GetDriverWritePtr() IOCTL failed\n");

	memcpy(&nRetVal, outBuf, sizeof(nRetVal));

	LeaveCriticalSection(&csHardware);
	return nRetVal;
}

// 20,000,000 for 20MSps
int GetTwinhanSymbolRate(int nInputSR)
{
	BYTE nRetVal[3];

	_asm
	{
  		mov	eax,nInputSR
  		push	00000014h
  		push	053EC600h
  		push	eax
  		call	L10001420
  		mov	edx,eax
  		mov	ecx,[esp+08h]
  		sar	edx,0Ch
  		mov	nRetVal[0],dl
  		mov	edx,eax
  		sar	edx,04h
  		shl	al,04h
  		mov	nRetVal[1],dl		
  		mov	nRetVal[2],al		
  		jmp    SR_done

 L10001420:
  		mov	ecx,[esp+0Ch]		
  		xor	eax,eax			
  		test	ecx,ecx		
  		jl 	L1000144C
  		mov	edx,[esp+08h]	
  		push	esi
  		lea	esi,[ecx+01h]
  		mov	ecx,[esp+08h]	
 L10001436:
  		cmp	ecx,edx
  		jge	L10001440
  		add	eax,eax
  		add	ecx,ecx
  		jmp	L10001448
 L10001440:
  		sub	ecx,edx
  		lea	eax,[eax+eax+01h]
  		shl	ecx,1
 L10001448:
  		dec	esi
  		jnz	L10001436
  		pop	esi
 L1000144C:
  		retn	000Ch

SR_done:
	}

	//TEMP_M=Symbol_Rate/(88000000 / 220 );

	return nRetVal[0] << 16 | nRetVal[1] << 8 | nRetVal[2];
}

void SelectDiSEqCInput(int nInput)
{
	BYTE bPositionByte[] = {0xf0, 0xf4, 0xf8, 0xfc};

	{
		char szDebug[128];
		wsprintf(szDebug, "Twinhan2: SelectDiSEqCInput(%d)\n", nInput);
		OutputDebugString(szDebug);
	}
	nInput--;
	if ((nInput >= 0) && (nInput <= 3) )
	{
		DWORD bytesReturned;
		int StatusSuccess;
		TunerData tunerData;   
		char * inBuf = (char *)&tunerData;
		int inBufLength	= sizeof(tunerData);
		BYTE bOutBuf[sizeof(tunerData)];
		memset(&tunerData, 0, sizeof(tunerData));

		tunerData.address = 0xab;
		tunerData.tunerStep = 0x04;
		tunerData.frequencyLSB = 0x08;
		tunerData.symbolRateHSB = 0xe0;
		tunerData.symbolRateMSB = 0x10;
		tunerData.symbolRateLSB = 0x38;
		tunerData.flag = bPositionByte[nInput];
		MakeCheckSum(&tunerData);

		StatusSuccess = DeviceIoControl(hDevice2, 
										(DWORD)DST_IOCTL_SET_INFO, 
										inBuf, // command
										inBufLength + 1, 
										bOutBuf,
										sizeof(bOutBuf), 
										&bytesReturned, 
										NULL);
	}										
}

#ifdef TWINHAN1030
void ResetHardware()
{

    BOOL			StatusSuccess = TRUE;
    ULONG			bytesReturned;

	StatusSuccess = DeviceIoControl(hDevice2, 
									(DWORD)DST_IOCTL_RDC8820_RESET, 
									NULL, 
									0, 
									NULL, 
									0, 
									&bytesReturned, 
									NULL);
}
#endif TWINHAN1030

#ifdef EIGHTVSB

// following part for get snr in ATSC device
typedef __int16 Data16;

#define NXT_NO_ERROR TRUE
#define NXT_ERR_NO_LOCK FALSE
#define EQ_SNR					0x00A0
#define EQ_CL_CONTROL			0x00A1
#define		CL_STAT_PTR_MASK	0x03
#define		CL_STAT_CV			0x00	/* cluster variance */
//#define		CL_STAT_CL			0x01	/* carrier loop */
//#define		CL_STAT_LF			0x02	/* loop filter */
//#define		CL_STAT_I_Q			0x03	/* i, q error etc. */

#define EQ_CL_STAT_3			0x00A6
#define EQ_CL_STAT_2			0x00A7
#define EQ_CL_STAT_1			0x00A8
#define DATA8_TO_DATA16(x)  (((Data16) (*(x)))<<8) | (((Data16) (*((x)+1))) & 0x00ff)

// Calculate checksum
BYTE CmdBytesCheckSum(BYTE *pByte)
{
	BYTE chksum = 0;
    int i;
	
	for(i=1; i<8; i++)
		chksum += pByte[i];
	
	chksum = ~chksum + 1;
	pByte[8] = chksum;
	return chksum;
}

// regGetFatLockStatus()  :  ¶ÁÈ¡tunerµ±Ç°µÄËø¶¨×´Ì¬£¬¿ÉÍ¨¹ý0x05ÃüÁî»ñµÃ£»
// getRegister()  :  ¶ÁÈ¡tuner½âµ÷Ð¾Æ¬ÏàÓ¦µÄ¼Ä´æÆ÷Öµ£¬¿ÉÍ¨¹ý0x14ÃüÁîÊµÏÖ£»
// setRegister()  :  Ïòtuner½âµ÷Ð¾Æ¬ÏàÓ¦µÄ¼Ä´æÆ÷Ð´ÈëÏàÓ¦µÄÊý¾Ý£¬¿ÉÍ¨¹ý0x14ÃüÁîÊµÏÖ£»
BOOL regGetFatLockStatus(BOOL *bLockedStatus)
{
	BOOL StatusSuccess;
	DWORD dwBytesRet;
	BYTE szInBuf[9] = {0xaa, 0, 5};

	CmdBytesCheckSum(szInBuf);
	StatusSuccess = DeviceIoControl(hDevice2, 
									(DWORD)DST_IOCTL_SET_INFO, 
									szInBuf, 
									9, 
									szInBuf, 
									9, 
									&dwBytesRet, 
									NULL);
	*bLockedStatus = (szInBuf[7] & 0x10) ? FALSE : TRUE;
	return StatusSuccess;
}

// ¶ÁÈ¡tuner½âµ÷Ð¾Æ¬ÏàÓ¦µÄ¼Ä´æÆ÷Öµ£¬¿ÉÍ¨¹ý0x14ÃüÁîÊµÏÖ
BOOL getRegister(UINT nCmd, UINT nParam, BYTE *pBuf)
{
	BOOL StatusSuccess;
	DWORD dwBytesRet;
	BYTE szInBuf[9] = {0xaa, 0, 0x14, 1};

	szInBuf[4] = nCmd>>8 & 0xff;
	szInBuf[5] = nCmd & 0xff;
	szInBuf[6] = 0;

	CmdBytesCheckSum(szInBuf);
	StatusSuccess = DeviceIoControl(hDevice2, 
									(DWORD)DST_IOCTL_SET_INFO, 
									szInBuf, 
									9, 
									szInBuf, 
									9, 
									&dwBytesRet, 
									NULL);
	*pBuf = szInBuf[1];
	return StatusSuccess;
}

// Ïòtuner½âµ÷Ð¾Æ¬ÏàÓ¦µÄ¼Ä´æÆ÷Ð´ÈëÏàÓ¦µÄÊý¾Ý£¬¿ÉÍ¨¹ý0x14ÃüÁîÊµÏÖ£»
BOOL setRegister(UINT nCmd, UINT nParam, BYTE *pBuf)
{
	BOOL StatusSuccess;
	DWORD dwBytesRet;
	BYTE szInBuf[9] = {0xaa, 0, 0x14, 1, 0, 0, 1};

	szInBuf[4] = nCmd>>8 & 0xff;
	szInBuf[5] = nCmd & 0xff;
	szInBuf[6] = 1;
	szInBuf[7] = *pBuf;

	CmdBytesCheckSum(szInBuf);
	StatusSuccess = DeviceIoControl(hDevice2, 
									(DWORD)DST_IOCTL_SET_INFO, 
									szInBuf, 
									9, 
									szInBuf, 
									9, 
									&dwBytesRet, 
									NULL);
	return StatusSuccess;
}

BOOL getFatSQI(//NxtDCB *pDCB,
			Data16 *pFatSQI) {

	BOOL	retVal;
	BYTE	buffer[2];

	/* setup to read cluster variance */
	retVal = getRegister(EQ_CL_CONTROL, 1, buffer);
	if (retVal == NXT_NO_ERROR) {
		buffer[0] &= ~CL_STAT_PTR_MASK;
		buffer[0] |= CL_STAT_CV;
		retVal = setRegister(EQ_CL_CONTROL, 1, buffer);
	}

	/* read 16 bit cluster variance */
	if (retVal == NXT_NO_ERROR) {
		retVal = getRegister(EQ_CL_STAT_3, 1, buffer);
		retVal = getRegister(EQ_CL_STAT_2, 1, &buffer[1]);
	}

	if (retVal == NXT_NO_ERROR) {
		*pFatSQI = (Data16)(0x7FFF) - (DATA8_TO_DATA16(buffer));
	}

	return retVal;
} /* getFatSQI */

BOOL NxtGetFatSQI(void *pContext,
					Data16 *pFatSQI) {

	BOOL retVal = NXT_NO_ERROR;
	//NxtDCB *pDummy;
	BOOL bLocked;
	
	retVal = regGetFatLockStatus(&bLocked);
	
	if (retVal == NXT_NO_ERROR) {
		if (!bLocked) {
			retVal = NXT_ERR_NO_LOCK;
			*pFatSQI = 0;
		}
		else {
			retVal = getFatSQI(pFatSQI);
		}	/* got lock */
	}

	return retVal;
} /* NxtGetFatSQI */

// return value :
// TRUE --success, SNR was stored in memory pointed to pSnrValue
// FALSE -- failure
BOOL GetEqSnr(double* pSnrValue)
{
	const double CONST_POWER_VSB8	= 41977441.0;

	double		const_power;
	long		snrVal;
	long		snr_wnw;
	long		snr_window_len;
	BYTE		i2c_data;
	Data16		sqi;
	BOOL		retVal;

	struct snr_control
	{
		int  SNR_CONTROL;
		BOOL SNR_FRAME;
	} sc;

	EnterCriticalSection(&csHardware);
	retVal  = (BOOL)NxtGetFatSQI((void*)NULL, &sqi);
	retVal	= (BOOL)getRegister(EQ_SNR, 1 , &i2c_data);

	if (retVal == NXT_NO_ERROR)
	{
		snrVal = 32767 - sqi;

		sc.SNR_CONTROL = i2c_data & 6;
		sc.SNR_FRAME   = (i2c_data & 1) == 1 ? TRUE : FALSE;
		snr_wnw        = (i2c_data >> 6) & 0x03;

		switch(sc.SNR_CONTROL)
		{
		case 0:	 
			{
				const_power = CONST_POWER_VSB8;
				
				switch(snr_wnw) 
				{
				case 0: snr_window_len = sc.SNR_FRAME ? 256 :  512;	break;
				case 1: snr_window_len = sc.SNR_FRAME ? 512 : 1024;	break;
				case 2: snr_window_len = sc.SNR_FRAME ? 768 : 2048;	break;
				case 3: snr_window_len = sc.SNR_FRAME ? 800 : 4096;	break;
				}
				
				*pSnrValue = 10.0 * log10((const_power * snr_window_len) / (2097152.0  * (double)snrVal ));
			}
			break;
		case 2:		
		case 4:		
		case 6:     
			{
				*pSnrValue = 0.0;	
			}
			break;
		default:
			*pSnrValue = 0.0;
			break;
		}
	}

	// fix SNR range
	if(*pSnrValue > 100 || *pSnrValue < 0)
		*pSnrValue = 0.0;

	LeaveCriticalSection(&csHardware);
    return retVal;
}
#endif EIGHTVSB

void GetTunerStatus(BOOL * LockFlag, int * Quality, int * Strength, double * dBER)
{
#ifndef NOTUNE
	int StatusSuccess;
	int inBufLength, outBufLength;
	int bytesReturned;
	BYTE * inBuf;
	BYTE outBuf[256];
    BYTE  *pbuf = (BYTE *)outBuf;
	DWORD Temp = 0;
    TunerData tunerData;
   	memset(&tunerData, 0, sizeof(tunerData));

	*dBER = 0.0;

    inBuf = (BYTE *)&tunerData;
    inBufLength	= sizeof(tunerData);
    
    tunerData.address = 0xaa;
	tunerData.frequencyMSB = 0;
	tunerData.frequencyLSB = 0x05;
	MakeCheckSum(&tunerData);
    
	outBufLength = sizeof(outBuf);
	EnterCriticalSection(&csHardware);
    StatusSuccess = DeviceIoControl(hDevice2, 
                                    (DWORD)DST_IOCTL_SET_INFO, 
                                    inBuf, // command
                                    inBufLength + 1, 
                                    outBuf, // return signal state
                                    outBufLength, 
                                    &bytesReturned, 
                                    NULL);
	LeaveCriticalSection(&csHardware);
	Temp = pbuf[7];
	if (Temp & 0x10)
		*LockFlag = FALSE;
	else
		*LockFlag = TRUE;

    Temp = pbuf[3];
	Temp = (Temp << 8) | pbuf[4];
    if (*LockFlag)
	{
        if (((int)Temp >= 0) && ((int)Temp <= 0x1200))
		{
            *Quality = 98;
		}
		else if ((Temp > 0x1200) && (Temp <= 0x1900))
		{
			double x = (double)(Temp - 0x1201) / (double)((0x1900 - 0x1201) / 57);
			*Quality = 40 + (58 - (int)x);
		}
		else if ((Temp > 0x1900) && (Temp <= 0x3500))
		{
			double x = (double)(Temp - 0x1901) / (double)((0x3500 - 0x1901) / 38);
			*Quality = 39 - (int)x;
		}
		else
			*Quality = 0;
	}
	else
	{
		*Quality = 0;
	}

	Temp = pbuf[6];
	if (*LockFlag)
		*Strength = Temp;
	else
	{
		if (Temp > 80)
			*Strength = 80;
		else
			*Strength = Temp;
	}
#else NOTUNE
	*LockFlag = 1;
#endif NOTUNE
#ifdef EIGHTVSB
	*dBER = 0.0;
	GetEqSnr(dBER);
#endif EIGHTVSB
}

#ifdef DVBS
BOOL TuneDVBS(int Frequency, BOOL H_V, int LNBFreq, int SymbolRate)
{
	int inFrequency;
	int StatusSuccess;
	int tunerStep;
	int InNumber;
	int divRate;
	int j;
	int inBufLength;
	int bytesReturned;
	BYTE * inBuf, * outBuf;
#ifdef TWINHAN1020
	DWORD dwLockTimeout = 3000;
	DWORD LockTime;
	DWORD dwSymbolRateValue;
	int outBufLength;
	BYTE bBuf[sizeof(TunerData)];
#endif TWINHAN1020
#ifdef TWINHAN1030
	BYTE bBuf[256];
#endif TWINHAN1030
	TunerData tunerData;
    long frequencyRef[16] = {2, 4, 8, 16, 32, 64, 128, 256, 24, 5, 10, 20, 40, 80, 160, 320 };

	memset(&tunerData, 0, sizeof(tunerData));

	if (Frequency > LNBFreq)
		inFrequency = Frequency - LNBFreq;
	else
		inFrequency = LNBFreq - Frequency;
	{
		char szDebug[128];
		wsprintf(szDebug, "Twinhan2: Tune: %d MHz l-band = %d SR = %d LNB = %d Pol = %d\n",
			Frequency, inFrequency, SymbolRate, LNBFreq, H_V);
		OutputDebugString(szDebug);
	}

	tunerStep = 1000000;    //250000	
	InNumber = inFrequency * 1000 / ( tunerStep / 1000 );
	tunerData.frequencyMSB = (InNumber & 0x00007f00) >> 8;
	tunerData.frequencyLSB = InNumber & 0x000000ff;
	divRate = 4000000L / tunerStep;
	
	for (j = 0; j < 16; j++)
	{
		if (frequencyRef[j] == divRate)
			break;
	}	
	if (j > 15)
		j = 15;
	tunerData.tunerStep = j;

	outBuf = (BYTE *)bBuf;

#ifdef TWINHAN1020
#ifdef DSS
	tunerData.flag = 0x0C;
#else DSS
	tunerData.flag = 0x04;
#endif DSS
	if (H_V)
		tunerData.flag |= 0x40;

	inBuf = (BYTE *)&tunerData;
	inBufLength = sizeof(tunerData);
	outBufLength = inBufLength;    

	dwSymbolRateValue = GetTwinhanSymbolRate(SymbolRate * 1000);
	tunerData.symbolRateHSB = (BYTE)(dwSymbolRateValue >> 16);
	tunerData.symbolRateMSB = (BYTE)(dwSymbolRateValue >> 8);
	tunerData.symbolRateLSB = (BYTE)(dwSymbolRateValue & 0xff);
	tunerData.address = 0xaa;
	MakeCheckSum(&tunerData);
	
	StatusSuccess = DeviceIoControl(hDevice2, 
									(DWORD)DST_IOCTL_SCAN_START, 
									inBuf,
									inBufLength + 1,
									outBuf, 
									outBufLength, 
									&bytesReturned, 
									NULL);
	if (!StatusSuccess)
	{
		char szTemp[100];
		wsprintf(szTemp, "Twinhan2: DST_IOCTL_SCAN_START failed with %d\n", StatusSuccess);
		OutputDebugString(szTemp);
		return FALSE;
	}

	StatusSuccess = FALSE;
	LockTime = GetTickCount();
	while (!StatusSuccess)
	{
		DWORD dwCountNow = GetTickCount() - LockTime;
		if (dwCountNow > dwLockTimeout)
		{
			OutputDebugString("Twinhan2: Tune Timeout\n");
			break;
		}

		StatusSuccess = DeviceIoControl(hDevice2, 
										(DWORD)DST_IOCTL_SCAN_WAITING, 
										NULL, 
										0, 
										NULL, 
										0, 
										&bytesReturned, 
										NULL);
		Sleep(1);
	}
	if (!StatusSuccess)
	{
		OutputDebugString("Twinhan2: Tune Fail\n");
		return FALSE;
	}

	DeviceIoControl(hDevice2, 
					(DWORD)DST_IOCTL_SCAN_END, 
					inBuf, 
					inBufLength + 1, 
					outBuf, 
					outBufLength, 
					&bytesReturned, 
					NULL);

	if (StatusSuccess)
	{
		OutputDebugString("Twinhan2: Tune OK\n");
		return TRUE;
	}
#endif TWINHAN1020

#ifdef TWINHAN1030
	tunerData.flag = 0x00;
	if (H_V)
		tunerData.flag |= 0x40;

	inBuf = (PVOID)bBuf;
	inBufLength = 256;
	memset(inBuf, 0, inBufLength);
	inBuf[0] = 0xaa;
	inBuf[1] = 9;
	inBuf[2] = 0;
	inBuf[3] = tunerData.frequencyMSB;
	inBuf[4] = tunerData.frequencyLSB;
	inBuf[5] = tunerData.tunerStep;
	inBuf[6] = 0;
	inBuf[7] = (BYTE)((SymbolRate & 0xff00) >> 8);
	inBuf[8] = (BYTE)(SymbolRate & 0xff);
	inBuf[9] = tunerData.flag;
	inBuf[10] = 0;
	for (j = 1; j < 10; j++)
		inBuf[10] += inBuf[j];
	inBuf[10] = ~inBuf[10] + 1;

	StatusSuccess = DeviceIoControl(hDevice2, 
									(DWORD)DST_IOCTL_CA_WRITE, 
									inBuf,
									inBufLength,
									NULL, 
									0, 
									&bytesReturned, 
									NULL);
	if (!StatusSuccess)
	{
		OutputDebugString("Twinhan2: 1030 CA_WRITE failed - resetting\n");
		ResetHardware();
		Sleep(600);
		return FALSE;
	}

	StatusSuccess = DeviceIoControl(hDevice2, 
									(DWORD)DST_IOCTL_CA_READ, 
									NULL,
									0,
									inBuf, 
									inBufLength, 
									&bytesReturned, 
									NULL);
	if (!StatusSuccess)
	{
		OutputDebugString("Twinhan2: 1030 CA_READ failed - resetting\n");
		ResetHardware();
		Sleep(600);
		return FALSE;
	}

	if ((inBuf[1] == tunerData.frequencyMSB) && (inBuf[2] == tunerData.frequencyLSB))
		return TRUE;

#endif TWINHAN1030

	return FALSE;
}
#endif DVBS

#ifdef EIGHTVSB
BOOL TuneATSC(int nFrequency)
{
	int nSymbolRate = 0;
	int StatusSuccess;
	int inBufLength, outBufLength;
	int bytesReturned;
	DWORD LockTime;
	BYTE * inBuf, * outBuf;
	BYTE bBuf[sizeof(TunerData)];
	TunerData tunerData;

	//ATSC Locking
	memset(&tunerData, 0, sizeof(tunerData));
	tunerData.address = 0xaa;
	tunerData.frequencyMSB = (BYTE)((nFrequency & 0xff0000) >> 16);
	tunerData.frequencyLSB = (BYTE)((nFrequency & 0x00ff00) >> 8);
	tunerData.tunerStep = (BYTE)(nFrequency & 0x0000ff);
	tunerData.symbolRateHSB = 0;
	tunerData.symbolRateMSB = (BYTE)nSymbolRate;		//BindWidth : 6,7,8
	tunerData.symbolRateLSB = 0;
	tunerData.flag = 0;		
	MakeCheckSum(&tunerData);
		
	inBuf = (BYTE *)&tunerData;
	inBufLength	= sizeof( tunerData );		
	outBuf = (LPVOID)bBuf;
	outBufLength = inBufLength;

	// The First step, scan start. I2c Write tuner data ...
	StatusSuccess = DeviceIoControl(hDevice2, 
									(DWORD)DST_IOCTL_SCAN_START, 
									inBuf,
									inBufLength + 1,
									outBuf, 
									outBufLength, 
									&bytesReturned, 
									NULL);
	if (!StatusSuccess)
	{
		OutputDebugString("Twinhan2: ATSC tuner DST_IOCTL_SCAN_START failed\n");
		return FALSE;
	}

	// The second step, waiting for Tuner Locking...
	StatusSuccess = FALSE;
	LockTime = GetTickCount();
	while (!StatusSuccess)
	{
		if ((GetTickCount() - LockTime) > 3000)
			break;
			
		StatusSuccess = DeviceIoControl(hDevice2, 
										(DWORD)DST_IOCTL_SCAN_WAITING, 
										NULL, 
										0, 
										NULL, 
										0, 
										&bytesReturned, 
										NULL);
		Sleep(5);
	}
	if (!StatusSuccess)
	{
		OutputDebugString("Twinhan2: ATSC tuner DST_IOCTL_SCAN_WAITING failed\n");
		return FALSE;
	}

	// The third step
	StatusSuccess = DeviceIoControl(hDevice2, 
									(DWORD)DST_IOCTL_SCAN_END, 
									inBuf, 
									inBufLength + 1, 
									outBuf, 
									outBufLength, 
									&bytesReturned, 
									NULL);
	if (StatusSuccess)
		return TRUE;

	OutputDebugString("Twinhan2: ATSC tuner DST_IOCTL_SCAN_END failed\n");
	return FALSE;
}
#endif EIGHTVSB

#ifdef DVBC
BOOL TuneDVBC(int nFrequency, int nSymbolRate, int nQAM)
{
	int HV;
    BOOL StatusSuccess = TRUE;
	int inBufLength;
    ULONG bytesReturned;
	BYTE * inBuf;
#ifndef CI_DVBC
	int outBufLength;
	DWORD dwLockTime;
	BYTE * outBuf;
	TunerData tunerData;
	BYTE bBuf[sizeof(TunerData)];
#else CI_DVBC
	int l;
	BYTE InBuf[256];
	BYTE F1, F2, F3;
#endif CI_DVBC

	switch(nQAM)
	{
	case 0:
		HV = 16;
		break;
	case 1:
		HV = 32;
		break;
	case 2:
		HV = 64;
		break;
	case 3:
		HV = 128;
		break;
	default:
		HV = 255;
		break;
	}

#ifdef CI_DVBC
	F1 = (BYTE)((nFrequency & 0xff0000) >> 16);
	F2 = (BYTE)((nFrequency & 0x00ff00) >> 8);
	F3 = (BYTE)(nFrequency & 0xff);

	inBuf = (PVOID)InBuf;
	inBufLength = 256;
	InBuf[0] = 0xaa;
	InBuf[1] = 9;
	InBuf[2] = 0;
	InBuf[3] = F1;
	InBuf[4] = F2;
	InBuf[5] = F3;
	InBuf[6] = (BYTE)((nSymbolRate & 0xff0000) >> 16);
	InBuf[7] = (BYTE)((nSymbolRate & 0x00ff00) >> 8);
	InBuf[8] = (BYTE)(nSymbolRate & 0xff);
	if (HV == 255)
		InBuf[9] = 0;
	else
		InBuf[9] = (BYTE)(HV);
	InBuf[10] = 0;
	for (l = 1; l < 10; l++)
		InBuf[10] += InBuf[l];
	InBuf[10] = ~InBuf[10] + 1;
	
	StatusSuccess = DeviceIoControl(hDevice2, 
									(DWORD)DST_IOCTL_CA_WRITE, 
									inBuf,
									inBufLength,
									NULL, 
									0, 
									&bytesReturned, 
									NULL);
	if (!StatusSuccess)
		return FALSE;
	
	StatusSuccess = DeviceIoControl(hDevice2, 
									(DWORD)DST_IOCTL_CA_READ, 
									NULL,
									0,
									inBuf, 
									inBufLength, 
									&bytesReturned, 
									NULL);
	if (!StatusSuccess)
		return FALSE;

	if ((InBuf[1] == F1) && (InBuf[2] == F2) && (InBuf[3] == F3))
	{
		return TRUE;
	}

	return FALSE;
#else CI_DVBC
	memset(&tunerData,0,sizeof(TunerData));

	tunerData.frequencyMSB = (BYTE)((nFrequency & 0xff0000) >> 16);
	tunerData.frequencyLSB = (BYTE)((nFrequency & 0x00ff00) >> 8);
	tunerData.tunerStep = (BYTE)(nFrequency & 0x0000ff);
	tunerData.symbolRateHSB = (BYTE)(nSymbolRate >> 8);
	tunerData.symbolRateMSB = (BYTE)(nSymbolRate & 0x00ff);
	tunerData.symbolRateLSB = 0;		
	tunerData.flag = (BYTE)HV;
		
	// finally, calculate and set the check sum
	MakeCheckSum(&tunerData);		
	inBuf = (BYTE *)&tunerData;
	inBufLength	= sizeof( tunerData );		
	outBuf = (LPVOID)bBuf;
	outBufLength = inBufLength;
	
	// The First step, scan start. I2c Write tuner data ...
	StatusSuccess = DeviceIoControl(hDevice2, 
									(DWORD)DST_IOCTL_SCAN_START, 
									inBuf,
									inBufLength + 1,
									outBuf, 
									outBufLength, 
									&bytesReturned, 
									NULL);
	if (!StatusSuccess)
	{
		OutputDebugString("Twinhan2 DVBC: DST_IOCTL_SCAN_START failed\n");
		return FALSE;
	}
	
	// The second step, waiting for Tuner Locking...
	StatusSuccess = FALSE;
	dwLockTime = GetTickCount();
	while (!StatusSuccess)
	{
		if ((GetTickCount() - dwLockTime) > 6000)
			break;
		
		StatusSuccess = DeviceIoControl(hDevice2, 
										(DWORD)DST_IOCTL_SCAN_WAITING, 
										NULL, 
										0, 
										NULL, 
										0, 
										&bytesReturned, 
										NULL);
		Sleep(5);
	}
	{
		char szTemp[128];
		wsprintf(szTemp, "Twinhan2 DVBC: DST_IOCTL_SCAN_WAITING returned %d\n", StatusSuccess);
		OutputDebugString(szTemp);
	}
	if (!StatusSuccess)
	{
		return FALSE;
	}
	
	// The third step
	StatusSuccess = DeviceIoControl(hDevice2, 
									(DWORD)DST_IOCTL_SCAN_END, 
									inBuf, 
									inBufLength + 1, 
									outBuf, 
									outBufLength, 
									&bytesReturned, 
									NULL);
	{
		char szTemp[128];
		wsprintf(szTemp, "Twinhan2 DVBC: DST_IOCTL_SCAN_END returned %d\n", StatusSuccess);
		OutputDebugString(szTemp);
	}
	if (StatusSuccess)
	{
		Sleep(500);
		return TRUE;
	}

	return FALSE;

#endif CI_DVBC
}
#endif DVBC

#ifdef DVBT
BOOL TuneDVBT(int Frequency, int nBandwidth)
{
	//int nMultiplier = 100;
	int SymbolRate = 0;
	int StatusSuccess;
	int inBufLength, outBufLength;
	int bytesReturned;
	int l = 0;
	DWORD LockTime;
	BYTE * inBuf, * outBuf;
	BYTE bBuf[256];
	TunerData tunerData;

	switch(nBandwidth)
	{
	case 0:
		SymbolRate = 6;
		break;
	case 1:
		SymbolRate = 7;
		break;
	case 2:
		SymbolRate = 8;
		break;
	}

#ifdef CI_DVBT
	inBuf = (PVOID)bBuf;
	inBufLength = 256;
	inBuf[0] = 0xaa;
	inBuf[1] = 9;
	inBuf[2] = 0;
	inBuf[3] = (BYTE)((Frequency & 0xff0000) >> 16);
	inBuf[4] = (BYTE)((Frequency & 0x00ff00) >> 8);
	inBuf[5] = (BYTE)(Frequency & 0xff);
	inBuf[6] = 0;
	inBuf[7] = 0;
	inBuf[8] = (BYTE)(SymbolRate & 0xff);
	inBuf[9] = 0;
	inBuf[10] = 0;
	for (l = 1; l < 10; l++)
		inBuf[10] += inBuf[l];
	inBuf[10] = ~inBuf[10] + 1;
	
	StatusSuccess = DeviceIoControl(hDevice2, 
									(DWORD)DST_IOCTL_CA_WRITE, 
									inBuf,
									inBufLength,
									NULL, 
									0, 
									&bytesReturned, 
									NULL);
	if (!StatusSuccess)
	{
		OutputDebugString("Twinhan DTT-CI: DST_IOCTL_CA_WRITE failed\n");
		return FALSE;
	}
	
	StatusSuccess = DeviceIoControl(hDevice2, 
									(DWORD)DST_IOCTL_CA_READ, 
									NULL,
									0,
									inBuf, 
									inBufLength, 
									&bytesReturned, 
									NULL);
	if (!StatusSuccess)
	{
		OutputDebugString("Twinhan DTT-CI: DST_IOCTL_CA_READ failed\n");
		return FALSE;
	}

	/*{
		int x;
		char szTemp[256];
		
		lstrcpy(szTemp, "Twinhan DTT-CI: ");

		for (x = 0; x < 11; x++)
		{
			char szTemp2[16];
			wsprintf(szTemp2, "%02x ", inBuf[x]);
			lstrcat(szTemp, szTemp2);
		}
		lstrcat(szTemp, "\n");
		OutputDebugString(szTemp);
	}*/

	if ((inBuf[1] == 0) && (inBuf[2] == 0))
	{
		OutputDebugString("Twinhan DTT-CI: inbuf indicates no lock\n");
		return FALSE;
	}

	return TRUE;
#else CI_DVBT
	//Digital TV Locking
	tunerData.frequencyMSB = (BYTE)((Frequency & 0xff0000) >> 16);
	tunerData.frequencyLSB = (BYTE)((Frequency & 0x00ff00) >> 8);
	tunerData.tunerStep = (BYTE)(Frequency & 0x0000ff);
	tunerData.symbolRateHSB = 0;
	tunerData.symbolRateMSB = (BYTE)SymbolRate;		//BindWidth : 6,7,8
	tunerData.symbolRateLSB = 0;
	tunerData.flag = 0;
		
	MakeCheckSum(&tunerData);
		
	inBuf = (BYTE *)&tunerData;
	inBufLength	= sizeof( tunerData );		
	outBuf = (LPVOID)bBuf;
	outBufLength = inBufLength;

	// The First step, scan start. I2c Write tuner data ...
	StatusSuccess = DeviceIoControl(hDevice2, 
									(DWORD)DST_IOCTL_SCAN_START, 
									inBuf,
									inBufLength + 1,
									outBuf, 
									outBufLength, 
									&bytesReturned, 
									NULL);
	if (!StatusSuccess)
		return FALSE;

	// The second step, waiting for Tuner Locking...
	StatusSuccess = FALSE;
	LockTime = GetTickCount();
	while (!StatusSuccess)
	{
		if ((GetTickCount() - LockTime) > 3000)
			break;
			
		StatusSuccess = DeviceIoControl(hDevice2, 
										(DWORD)DST_IOCTL_SCAN_WAITING, 
										NULL, 
										0, 
										NULL, 
										0, 
										&bytesReturned, 
										NULL);
		Sleep(5);
	}
	if (!StatusSuccess)
			return FALSE;

	// The third step
	StatusSuccess = DeviceIoControl(hDevice2, 
									(DWORD)DST_IOCTL_SCAN_END, 
									inBuf, 
									inBufLength + 1, 
									outBuf, 
									outBufLength, 
									&bytesReturned, 
									NULL);
	if (StatusSuccess)
		return TRUE;

	return FALSE;
#endif CI_DVBT
}
#endif DVBT

#ifdef DVBS
void SetTuner22KHzAndPower(BOOL f22KHz, BOOL fPower)
{
	BYTE * inBuf, outBuf[sizeof(TunerData)];
	int inBufLength, outBufLength;
	int bytesReturned;
	int nRetVal;
    TunerData tunerData;
	
	{
		char szDebug[128];
		wsprintf(szDebug, "Twinhan2: SetTuner22KHzAndPower(22KHz = %d, Power = %d)\n", f22KHz, fPower);
		OutputDebugString(szDebug);
	}

	memset(&tunerData, 0, sizeof(tunerData));
	tunerData.address = 0xaa;
	if (fPower)
		tunerData.symbolRateMSB = 0x01;
	else
		tunerData.symbolRateMSB = 0x00; 
	tunerData.frequencyLSB = 0x09;
	if (f22KHz)
		tunerData.tunerStep = 0x02;
	else
		tunerData.tunerStep = 0;
	tunerData.symbolRateHSB = 0xff;
	MakeCheckSum(&tunerData);
 
    inBuf = (BYTE *)&tunerData;
	inBufLength = sizeof(tunerData);  
	outBufLength = sizeof(outBuf);
 
    nRetVal = DeviceIoControl(hDevice2, 
                    (DWORD)DST_IOCTL_SET_INFO, 
                    inBuf, 
                    inBufLength + 1, 
                    outBuf, 
                    outBufLength, 
                    &bytesReturned, 
                    NULL);
	if (!nRetVal)
	{
		OutputDebugString("Twinhan2: SetTuner22KHzAndPower() DST_IOCTL_SET_INFO IOCTL failed\n");
		return;
	}
	Sleep(250);
}
#endif DVBS

void StreamCapture(BOOL fEnable)
{
	int bytesReturned;
	DWORD outBuf;
    BOOL StatusSuccess = TRUE;

	EnterCriticalSection(&csHardware);
	if (fEnable)
	{
		StatusSuccess = DeviceIoControl(hDevice1,
						(DWORD)DST_IOCTL_START_CAP, 
						NULL, 
						0, 
						&outBuf, 
						sizeof(DWORD), 
						&bytesReturned, 
						NULL);
		memcpy(&pDriverBuffer, &outBuf, sizeof(DWORD));
		if (!StatusSuccess)
		{
			OutputDebugString("Twinhan2: StreamCapture() DST_IOCTL_START_CAP IOCTL failed\n");
		}
	}
	else
	{
		StatusSuccess = DeviceIoControl(hDevice1,
						(DWORD)DST_IOCTL_STOP_CAP, 
						NULL, 
						0, 
						NULL, 
						0, 
						&bytesReturned, 
						NULL);
		if (!StatusSuccess)
		{
			OutputDebugString("Twinhan2: StreamCapture() DST_IOCTL_STOP_CAP IOCTL failed\n");
		}
	}
	LeaveCriticalSection(&csHardware);
}

DWORD WINAPI ReadTwinhan2Thread(LPVOID lpv)
{
	int nReadPtr = 0;
	int nTunerStatusTimer = 250;
	//HANDLE hDebug = CreateFile("c:\\tsreader.ts", GENERIC_WRITE, 0, (LPSECURITY_ATTRIBUTES) NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, (HANDLE) NULL);

	ss->fReadThreadTerminated = FALSE;
	ss->fTerminateReadThread = FALSE;

#ifdef DSS	
	SourceHelper_StartSyncThread(ss, TRUE);
#else DSS
	SourceHelper_StartSyncThread(ss, FALSE);
#endif
	StreamCapture(TRUE);

	while (!ss->fTerminateReadThread)
	{
		int nWritePtr = GetDriverWritePtr();

		if (nReadPtr != nWritePtr)
		{
			BYTE * pCurrentBuffer = pDriverBuffer + nReadPtr;
			int nLength = nWritePtr - nReadPtr;

			if (nLength <= 0)
			{
				//WriteFile(hDebug, pCurrentBuffer, nBufferLength - nReadPtr, &dwWritten, NULL);
				SourceHelper_SyncData(pCurrentBuffer, nBufferLength - nReadPtr);
				nReadPtr = 0;
			}
			else
			{
				//WriteFile(hDebug, pCurrentBuffer, nBufferLength - nReadPtr, &dwWritten, NULL);
				SourceHelper_SyncData(pCurrentBuffer, nLength);
				nReadPtr += nLength;
			}
		}
		else
		{
			Sleep(1);
#ifndef NOTUNE
			if (nTunerStatusTimer++ > 250)
			{
				BOOL fLock, nQuality, nStrength;
				double dBER;
				char szLockStatus[16];

				nTunerStatusTimer = 0;
				GetTunerStatus(&fLock, &nQuality, &nStrength, &dBER);
				if (fLock)
					lstrcpy(szLockStatus, "Locked");
				else
					lstrcpy(szLockStatus, "Unlocked");
				
				EnterCriticalSection(&csSignal);
#ifndef EIGHTVSB
				wsprintf(szLastSignalReport, "%s Quality %d%% Signal %d%%", szLockStatus, nQuality, nStrength);
#else EIGHTVSB
				sprintf(szLastSignalReport, "%s SNR %.1f dB ", szLockStatus, dBER);
#endif EIGHTVSB
				LeaveCriticalSection(&csSignal);
				if (fLock == 0)
				{
					nRetuneCount++;
#ifdef DVBS
					TuneDVBS(ss->nFrequency, ss->nPolarity, ss->nLNBFrequency, ss->nSymbolRate);
#endif DVBS
#ifdef DVBT
					TuneDVBT(ss->nFrequency, ss->nBandwidth);
#endif DVBT
#ifdef DVBC
					TuneDVBC(ss->nFrequency, ss->nSymbolRate, ss->nQAM);
#endif DVBC
#ifdef EIGHTVSB
					TuneATSC(ss->nFrequency * 1000);
#endif EIGHTVSB
				}
			}
#endif NOTUNE
		}
	}

	SourceHelper_StopSyncThread();
	//CloseHandle(hDebug);
	ss->fReadThreadTerminated = TRUE;
	ss->fTerminateReadThread = FALSE;
	EnterCriticalSection(&ss->csTSBuffersInUse);
	ss->nTSBuffersInUse = -1000;
	LeaveCriticalSection(&ss->csTSBuffersInUse);
	CloseHandle(ss->hReadDataThread);
	OutputDebugString("Twinhan2: -ReadTwinhan2Thread\n");
	return 0;
}

BOOL TSReader_Start()
{
	DWORD dwThreadID;

	InitializeCriticalSection(&csHardware);

	nRetuneCount = 0;
	ss->hReadDataThread = CreateThread(NULL, 0, ReadTwinhan2Thread, (LPVOID)0, CREATE_SUSPENDED, &dwThreadID);
	SourceHelper_SetWorkerThreadPriorities(FALSE);
	ResumeThread(ss->hReadDataThread);

	return TRUE;
}

BOOL TSReader_Stop()
{
	OutputDebugString("Twinhan2: Wait for read thread terminate\n");
	ss->fTerminateReadThread = TRUE;
	while (ss->fReadThreadTerminated == FALSE)
		Sleep(50);
	OutputDebugString("Twinhan2: Read thread terminated\n");

	StreamCapture(FALSE);
	DeleteCriticalSection(&csHardware);
	
	OutputDebugString("Twinhan2: TSReader_Stop() complete\n");
	return TRUE;
}

BOOL OpenTwinhanDriver()
{
	hDevice1 = GetDeviceHandle(GUID_DST_DEVICE1, NULL);
	if (hDevice1 == INVALID_HANDLE_VALUE)
	{
		OutputDebugString("Twinhan2: OpenTwinhanDriver, GetDeviceHandle(GUID_DST_DEVICE1) failed\n");
		return FALSE;
	}

    hDevice2 = GetDeviceHandle(GUID_DST_DEVICE2, NULL); 
	if (hDevice2 == INVALID_HANDLE_VALUE)
	{
		CloseHandle(hDevice1);
		OutputDebugString("Twinhan2: OpenTwinhanDriver, GetDeviceHandle(GUID_DST_DEVICE2) failed\n");
		return FALSE;
	}

	return TRUE;
}

BOOL TSReader_Init(PSOURCESTRUCT pss)
{
	if (SourceHelper_ValidateSourceContainer(pss) == FALSE)
		return FALSE;
	InitializeCriticalSection(&csSignal);

	ss = pss;

	if (OpenTwinhanDriver() == FALSE)
		return FALSE;
	nCardType = GetCardType();
	{
		char szTemp[100];
		wsprintf(szTemp, "Twinhan2: card type = %d\n", nCardType);
		OutputDebugString(szTemp);
	}

	return nCardType != -1;
}

void CloseTwinhanDriver()
{
	CloseHandle(hDevice1);
	CloseHandle(hDevice2);
	OutputDebugString("Twinhan2: CloseTwinhanDriver()\n");
}

BOOL TSReader_DeInit()
{
	OutputDebugString("Twinhan2: TSReader_DeInit()\n");
	CloseTwinhanDriver();
	DeleteCriticalSection(&csSignal);
	return TRUE;
}

void SetupLastTune()
{
#ifdef DVBS
	char szPolarity[4] = {"H/L"};
	char szModulation[16] = {0};

	lstrcpy(szLastSignalReport, "n/a");

	if (ss->nPolarity == 0)
		lstrcpy(szPolarity, "V/R");
#ifndef DSS
	lstrcpy(szModulation, "DVB QPSK");
#else DSS
	lstrcpy(szModulation, "DSS QPSK");
#endif DSS
	wsprintf(szLastTune, "%d MHz %s %d %s", ss->nFrequency, szPolarity, ss->nSymbolRate, szModulation);
#endif DVBS
#ifdef DVBT
	sprintf(szLastTune, "%.3f MHz", ss->nFrequency / 1000.0);
#endif DVBT
#ifdef DVBC
	sprintf(szLastTune, "%.1f MHz", ss->nFrequency / 1000.0);
#endif DVBC
#ifdef EIGHTVSB
	wsprintf(szLastTune, "Channel %d (%d MHz)", SourceHelper_GetATSCChannelFromFrequency(ss->nFrequency), ss->nFrequency);
#endif EIGHTVSB
}

BOOL TSReader_Tune()
{
#ifndef NOTUNE	
	SetupLastTune();
#ifdef DVBS
	if (fPowerOn == FALSE)
	{
		SetTuner22KHzAndPower(ss->n22KHz, TRUE);
		fPowerOn = TRUE;
	}

	switch(ss->nDiSEqCInput)
	{
	case 0:
		break;
	case 1:
	case 2:
	case 3:
	case 4:
		Sleep(50);
		SelectDiSEqCInput(ss->nDiSEqCInput);
		if (fPowerOn == TRUE)
		{
			Sleep(50);
			SetTuner22KHzAndPower(ss->n22KHz, TRUE);
		}
		break;
	case 5:
	case 6:
		break;
	}

	if (TuneDVBS(ss->nFrequency, ss->nPolarity, ss->nLNBFrequency, ss->nSymbolRate) == TRUE)
		return TRUE;
#endif DVBS
#ifdef DVBT
	if (TuneDVBT(ss->nFrequency, ss->nBandwidth) == TRUE)
		return TRUE;
#endif DVBT
#ifdef DVBC
	if (TuneDVBC(ss->nFrequency, ss->nSymbolRate, ss->nQAM) == TRUE)
		return TRUE;
#endif DVBC
#ifdef EIGHTVSB
	if (TuneATSC(ss->nFrequency * 1000) == TRUE)
		return TRUE;
#endif EIGHTVSB
	if (ss->fQuietMode == FALSE)
	{
		OutputDebugString("Twinahn2: Failed to lock signal\n");
		MessageBox(ss->hWndTSReader, "Failed to lock signal", gszSourceName, MB_ICONWARNING);
	}
	return FALSE;
#endif NOTUNE

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
#ifdef DVBS
	if (szCommandLineParameters != NULL)
		lstrcpy(szCommandLineParameters, "freq pol sr lnbf 22khz {input}");
	if (dwCapabilities != NULL)
		*dwCapabilities = CAPABILITIES_DISEQC
						| CAPABILITIES_POWER
						| CAPABILITIES_DISEQC_POSITIONER;
#endif DVBS
#ifdef DVBT
	if (szCommandLineParameters != NULL)
		lstrcpy(szCommandLineParameters, "freq bandwidth");
	if (dwCapabilities != NULL)
		*dwCapabilities = 0;
#endif DVBT
#ifdef NOTUNE
	if (szCommandLineParameters != NULL)
		lstrcpy(szCommandLineParameters, "None");
	if (dwCapabilities != NULL)
		*dwCapabilities = 0;
#endif NOTUNE
#ifdef DVBC
	if (szCommandLineParameters != NULL)
		lstrcpy(szCommandLineParameters, "freq sr QAM inversion bandwidth");
	if (dwCapabilities != NULL)
		*dwCapabilities = 0;
#endif DVBC
#ifdef CI_SUPPORT
	if (dwCapabilities != NULL)
		*dwCapabilities |= CAPABILITIES_CI_CAM;
#endif CI_SUPPORT
	
	if (fCanBeStopped != NULL)
		*fCanBeStopped = FALSE;
	if (nMaxPIDs != NULL)
		*nMaxPIDs = 8192;
	return TRUE;
}

BOOL TSReader_TuneDialog(HWND hWnd)
{
	OutputDebugString("Twinhan2: TSReader_TuneDialog\n");

	if (ss->fDontTune == TRUE)
		fNeedTuneDialog = FALSE;

	if (fNeedTuneDialog)
#ifndef NOTUNE
	{
		OutputDebugString("Twinhan2: TSReader_TuneDialog tuning dialog is required\n");
#ifdef DVBS
 #ifndef DSS
		if (SourceHelper_DVBSTuneDialog(hWnd) == FALSE)
 #else DSS
		if (SourceHelper_DSSTuneDialog(hWnd) == FALSE)
 #endif DSS
#endif DVBS
#ifdef DVBT
		if (SourceHelper_DVBTTuneDialog(hWnd) == FALSE)
#endif DVBT
#ifdef DVBC
		if (SourceHelper_DVBCTuneDialog(hWnd) == FALSE)
#endif DVBC
#ifdef EIGHTVSB
		if (SourceHelper_ATSCTuneDialog(hWnd) == FALSE)
#endif EIGHTVSB
			return FALSE;
	}
	else
	{
		OutputDebugString("Twinhan2: TSReader_TuneDialog tune NOT required\n");
		ss->nFrequency = nFrequency;
#ifdef DVBS
		ss->nPolarity = nPolarity;
		ss->nSymbolRate = nSymbolRate;
		ss->nLNBFrequency = nLNBFrequency;
		ss->n22KHz = n22KHz;
		ss->nDiSEqCInput = nDiSEqCInput;
#endif DVBS
#ifdef DVBT
		ss->nBandwidth = nBandwidth;
#endif DVBT
#ifdef DVBC
		ss->nQAM = nQAM;
		ss->nSymbolRate = nSymbolRate;
#endif DVBC
		fNeedTuneDialog = TRUE;
	}
#else NOTUNE
	{
		if (MessageBox(hWnd,
					   "The current API from Twinhan doesn't tune quite right. To use this source you\nneed to first tune the mux with another program and then run TSreader.\n\nDo you want to continue?",
					   gszSourceName, MB_YESNO | MB_ICONQUESTION | MB_DEFBUTTON1) == IDNO)
		{
			return FALSE;
		}
	}
#endif NOTUNE

	return TRUE;
}

BOOL TSReader_ParseCommandLine(PSOURCESTRUCT ss, char * szCommandLine, BOOL fQuiet)
{
	if (lstrlen(szCommandLine))
	{
		int nConversionCount = 0;
#ifdef DVBS
		SourceHelper_ConvertPolarity(szCommandLine);
		ss->nDiSEqCInput = 0;
		nConversionCount = sscanf(szCommandLine,
								  "%d %d %d %d %d %d", 
								  &nFrequency,
								  &nPolarity,
								  &nSymbolRate,
								  &nLNBFrequency,
								  &n22KHz,
								  &nDiSEqCInput);
		if (nConversionCount < 5)
		{
			if (!fQuiet)
				MessageBox(NULL,
					   "Usage for this source: freq pol sr lnbf 22khz {input}\n"
					   "\n"
					   "freq = frequency to tune\n"
					   "pol = 0 vertical/RHCP 1 horizontal/LHCP\n"
					   "sr = symbol rate\n"
					   "lnbf = LNB frequency\n"
					   "22k = 22KHz tone enable\n"
					   "input = select DiSEqC input number (1-4) - optional",
					   gszSourceName,
					   MB_OK | MB_ICONSTOP);
			return FALSE;
		}
#endif DVBS
#ifdef DVBT
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
#endif DVBT
#ifdef DVBC
		nConversionCount = sscanf(szCommandLine,
								  "%d %d %d %d %d", 
								  &nFrequency,
								  &nSymbolRate,
								  &nQAM,
								  &fSpectrumInversion,
								  &nBandwidth);
		if (nConversionCount < 5)
		{
			if (!fQuiet)
				MessageBox(NULL,
					   "Usage for this source: freq sr QAM inversion bandwidth\n"
					   "\n"
					   "freq = frequency to tune in KHz\n"
					   "sr = symbol rate\n"
					   "QAM = QAM Mode (0=QAM-16 1=QAM-32 2=QAM-64 3=QAM-128 4=QAM-256)\n"
					   "inversion = inverted spectrum (0 or 1)\n"
					   "bandwidth = bandwidth of signal (0 = 6, 1 = 7, 2 = 8 MHz)",
					   gszSourceName,
					   MB_OK | MB_ICONSTOP);
			return FALSE;
		}
#endif DVBC
#ifdef EIGHTVSB
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
			nFrequency = SourceHelper_GetFrequencyFromATSCChannel(nFrequency);
#endif EIGHTVSB
		fNeedTuneDialog = FALSE;
	}
	else
		fNeedTuneDialog = TRUE;

	return TRUE;
}

/*
  
 

7 IQ
6 H/V
5 S
4 QPSK/BPSK
3 DVB/DSS
2 P2
1 X1
0 X0

 

X1,X0,I/Q,S,P2 : don’t care;

*For old version,“(200103A)”:

I/Q:0?normal ,1?changed ;

S:if Symbol Rate>8000 Ksps , S =1,else S=0?

P2:Frequency>1530,P2=0,Frequency<=1530,P2=1?

X1X0: 00:      F22/DiSEqC Pin output 0

                                 01:      F22/DiSEqC Pin output 1

                                10:      F22/DiSEqC Pin output 22Khz

                                11:      high impedance;

H/V:polarization selector  0: V,1: H;

DVB/DSS:0?DVB Standard,1?DSS;

QPSK/BPSK:0?QPSK,1?BPSK;

======================================================================

Symbol Rate calculation:

Sym-h:The first byte of symbol rate;

Sym-m:The second byte of symbol rate;

Sym-l: The third byte of symbol rate;

Symbol rate:Symbol Rate=Fm-clk/220*m ;    unit (K sps);

Fm-clk: Master clock (88M Hz); 

 m: 20 bit binary value;

 

sym-h= (m>>12)

 sym-m=(m>>4)&0xff

 sym-l=(m&0xf)<<4

 

For example:

        Symbol rate : 28125 K sps; 

        Master Clock : 88M Hz;

28125000/(88000000 / 220 ) = (335127) Dec  = 0x51017;

So, 

        sym-h, sym-m, sym-l=0x51, 0x01, 0x70


  */
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
	EnterCriticalSection(&csSignal);
	lstrcpy(szString, szLastSignalReport);
	LeaveCriticalSection(&csSignal);
	return TRUE;
}

BOOL TSReader_GetTunerString(char * szString)
{
	lstrcpy(szString, szLastTune);
	return TRUE;
}

BOOL TSReader_SendDiSEqC(BYTE * bCommand, int nLength)
{
#ifdef SATELLITE
	if (bCommand != NULL && nLength)
	{
		DWORD bytesReturned;
		int StatusSuccess;
		TunerData tunerData;   
		char * inBuf = (char *)&tunerData;
		int inBufLength	= sizeof(tunerData);
		BYTE bOutBuf[sizeof(tunerData)];
		memset(&tunerData, 0, sizeof(tunerData));

#ifdef TWINHAN1020
		if (fPowerOn == FALSE)
		{
			SetTuner22KHzAndPower(ss->n22KHz, TRUE);
			fPowerOn = TRUE;
			Sleep(1000);
		}
#endif TWINHAN1020

		tunerData.address = 0xab;
		tunerData.tunerStep = nLength;
		tunerData.frequencyLSB = 0x08;
		tunerData.symbolRateHSB = bCommand[0];
		if (nLength > 1)
			tunerData.symbolRateMSB = bCommand[1];
		if (nLength > 2)
			tunerData.symbolRateLSB = bCommand[2];
		if (nLength > 3)
			tunerData.flag = bCommand[3];
		MakeCheckSum(&tunerData);

		StatusSuccess = DeviceIoControl(hDevice2, 
										(DWORD)DST_IOCTL_SET_INFO, 
										inBuf, // command
										inBufLength + 1, 
										bOutBuf,
										sizeof(bOutBuf), 
										&bytesReturned, 
										NULL);

		// Stall while the firmware sends the DiSEqC command. 12.5 ms per byte
		// plus 15 ms after all bytes
		Sleep((nLength * 13) + 15 + 10);

		// In case we're locked, set the 22KHz and polarity back the way it was before the DiSEqC command
		SetTuner22KHzAndPower(ss->n22KHz, TRUE);
	}
#endif SATELLITE
	return TRUE;
}

int TSReader_GetSyncLossCount(BOOL fReset)
{
	return SourceHelper_GetSyncLossCount(fReset);
}

int TSReader_GetRetuneCount(BOOL fReset)
{
	if (fReset)
		nRetuneCount = 0;
	return nRetuneCount;
}

BOOL APIENTRY DllMain(HANDLE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
    switch( ul_reason_for_call )
	{
    case DLL_PROCESS_ATTACH:
		fPowerOn = FALSE;
		break;
    case DLL_PROCESS_DETACH:
#ifdef DVBS
		if (fPowerOn)
		{
			if (OpenTwinhanDriver() == TRUE)
			{
				SetTuner22KHzAndPower(FALSE, FALSE);
				CloseTwinhanDriver();
			}
		}
#endif DVBS
		break;
    }
    return TRUE;
}
