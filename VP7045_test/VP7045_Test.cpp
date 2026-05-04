// 7020_test.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include <windows.h>
#include <malloc.h>
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <initguid.h>
#include <devioctl.h>

#include "VP7045_test.h"
#include "Dev_IOCTL.h"

HANDLE	OpenDriver(char* DriverName);
BOOL Get_FX2_REG(HANDLE hDevice, UINT Index, PUCHAR pValue);
BOOL Set_FX2_REG(HANDLE hDevice, UINT Index, UCHAR Value);
BOOL Get_Tuner_REG(HANDLE hDevice, UINT Index, PUCHAR pValue);
BOOL Set_Tuner_REG(HANDLE hDevice, UINT Index, UCHAR Value);
BOOL Get_RC_VAL(HANDLE hDevice, PUCHAR pValue);
BOOL Lock_Tuner(HANDLE hDevice, UINT dwFreq, UINT dwBW);
DWORD Start_Cap(HANDLE hDevice);
BOOL  Stop_Cap(HANDLE hDevice);
BOOL GetDMACurrentPosition(HANDLE hDevice, PDWORD pCurrentPosition);
BOOL Get_Signal_Q_S(HANDLE hDevice, PDWORD pnQuality, PDWORD pnStrength, PDWORD pnLock_Status);
BOOL Get_FW_Version(HANDLE hDevice, PCHAR psVersion);
BOOL Get_Driver_Version(HANDLE hDevice, PCHAR psVersion);
BOOL Get_Vender_Str(HANDLE hDevice, PCHAR psVenderStr);
BOOL Get_Product_Str(HANDLE hDevice, PCHAR psProductStr);
BOOL Set_Tuner_Power(HANDLE hDevice, UCHAR ucStatus);
BOOL Get_USB_SPEED(HANDLE hDevice, PUCHAR pUSB_Speed);
BOOL Set_PLD_PID(HANDLE hDevice, PBYTE pPLD_buf);
BOOL Get_PLD_PID(HANDLE hDevice, PBYTE pPLD_buf);
BOOL Set_PLD_Status(HANDLE hDevice, UCHAR ucStatus);
DWORD MAKE_REMOTEBUF(HANDLE hDevice);
BOOL Start_RC(HANDLE hDevice);
BOOL Stop_RC(HANDLE hDevice, PREMOTE_EVENT pevRC);
BOOL Get_MAC(HANDLE hDevice, PUCHAR pVal);
BOOL Get_RC(HANDLE hDevice, PUCHAR pVal);
BOOL Get_Driver_Info(HANDLE hDevice, P_DriverInfo pDrvInfo);

#define MAX_TRY_NO 20

int main(int argc, char* argv[])
{		
	HANDLE h_Dev = NULL;
	BOOLEAN bResult	= FALSE;
	int     nBytes  = 0;
	PBYTE   pTS_Buf = NULL;
	
	int		i = 0;
	DWORD dwWritePosition = 0, dwReadPosition = 0, dwValidBufSize = 0;
	ULONG nFileSize = 5*1024*1024;

	if ((h_Dev = OpenDriver("THUSB-T-000")) == INVALID_HANDLE_VALUE) {
		printf("Error: Fail to open USB device !! \n");
		getchar();
		return FALSE;
	}		

#if 1	// Get Driver & F/W information
	DriverInfo DrvInfo;
	Get_Driver_Info(h_Dev, &DrvInfo);
	printf("Drive Major Version: %x, Minor Version: %x \n", DrvInfo.Version_Major, DrvInfo.Version_Minor);
	printf("FW    Major Version: %x, Minor Version: %x \n", DrvInfo.FW_Version_Major, DrvInfo.FW_Version_Minor);
	printf("Date_Time: %s, Company: %s\n", DrvInfo.Date_Time, DrvInfo.Company);
	printf("SupportHWInfo: %s\n", DrvInfo.SupportHWInfo);

	CHAR psVender[8];
	Get_Vender_Str(h_Dev, psVender);

	CHAR psProduct[8];
	Get_Product_Str(h_Dev, psProduct);

	CHAR psDrvVersion[5];
	Get_Driver_Version(h_Dev, psDrvVersion);	

	CHAR psFWVersion[5];
	Get_FW_Version(h_Dev, psFWVersion);	

	UCHAR USB_Speed;
	Get_USB_SPEED(h_Dev, &USB_Speed);

	UCHAR psMAC[10];	
	Get_MAC(h_Dev, psMAC);

	printf("Vender: %s,  Product: %s,  Driver Version: %s \n", psVender, psProduct, psDrvVersion);
	printf("F/W Version: %s,  USB Speed: %d \n", psFWVersion, USB_Speed);
	printf("MAC Address: ");
	for (i=1; i<8; i++)
		printf("%02x, ", psMAC[i]);
	printf("\n");
#endif	


#if 0  // Set PLD PLD values
	BYTE pPLD_buf[17] = {11, 00, 2-2, 2-1, 3-2, 3-1, 4-2, 4-1, 5-2, 5-1, 6-2, 6-1, 7-2, 7-1, 8-2, 8-1, 8};
	                  //{  Fix,  PID - 2,  PID - 3,  PID - 4,  PID - 5,  PID - 6,  PID - 7,  PID - 8,  Size}
	UINT pld_reg2 = 0;
	Set_PLD_PID(h_Dev, pPLD_buf);
	Get_PLD_PID(h_Dev, pPLD_buf);
	printf("PLD PID: ");
	for (int index=0; index<17; index++)
		printf("0x%x, ", pPLD_buf[index]);
#endif


#if 0	// Get Remote control value	
	//printf("call ACTTV_IOCTL_GET_KEYCODE!!\n");
	DWORD BytesReturned=0;
	REMOTE_EVENT evRC;
	UCHAR ucRCVal = 0xFF;

	evRC.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);	
	if(!DeviceIoControl(h_Dev, ACTTV_IOCTL_GET_KEYCODE ,
							(LPVOID)&evRC, sizeof(REMOTE_EVENT),     // Input
							NULL, 0,                    // Output
							&BytesReturned, NULL))
	{
		printf("ACTTV_IOCTL_GET_KEYCODE Failed!\n");
	}

	while (ucRCVal != 0x16) {  //PowerOff key value		
		ResetEvent(evRC.hEvent);
		printf("Wait driver to signal RC event %x..(Press PowerOff to terminal)\n", evRC.hEvent);
		WaitForSingleObject(evRC.hEvent, 3000);		
		Get_RC(h_Dev, &ucRCVal);
		if (ucRCVal != 0x44)
			printf("Get RC Value = 0x%x \n", ucRCVal);
	}

	Stop_RC(h_Dev, &evRC);
	CloseHandle(evRC.hEvent);
#endif


#if 1	// Lock Tuner

	Set_Tuner_Power(h_Dev, Tuner_Power_ON);

	//Get Tuner ID
	UCHAR tuner_reg = 0x0;
	//while (1)
	{
	Get_Tuner_REG(h_Dev, 0x7F, &tuner_reg);
	//Sleep(100);
	}
	printf("Tuner ID=%d \n", tuner_reg);

	long nFreq = 593000, nBW = 6;
	printf("Lock => Frequency:");
	scanf("%d", &nFreq);getchar();
	printf("Lock => BandWith:");
	scanf("%d", &nBW);getchar();

	if (!Lock_Tuner(h_Dev, nFreq, nBW)) {
		CloseHandle (h_Dev);		
		printf("Err: Lock tuner fail !! \n");
		getchar();
		return FALSE;
	}
	else {
		printf("Lock tuner success !! \n");		
	}
#endif	


#if 1 //Get Signal Strength & Quality & RC value
	//while (1) 
	{
		DWORD nStrength = 0, nQuality = 0, nLockStatus = 0;
		UCHAR rc_val = 0x0;
		Get_Signal_Q_S(h_Dev, &nQuality, &nStrength, &nLockStatus);
		printf("LockStatus=%d, Singal Strength=%d, Quality=%d \n", nLockStatus, nStrength, nQuality);
		//Sleep(1000);
	}
#endif


#if 0 //Record file by IOCTL
	printf("Record File => Enter the size (M):");
	scanf("%d", &nFileSize);getchar();
	nFileSize = nFileSize*1024*1024;

	DWORD tStart, tEnd;
	//Sleep(3000);
	FILE *fStream = fopen("rec.ts", "wb");	
	pTS_Buf = (PBYTE)Start_Cap(h_Dev);

	GetDMACurrentPosition(h_Dev, &dwWritePosition);
	dwReadPosition = dwWritePosition;
	ULONG nGetSize = 0, nTryNo = 0;
	printf("Start recording...\n");
	tStart = GetTickCount();
    while ((nGetSize < nFileSize) && (nTryNo < MAX_TRY_NO))
    {
		GetDMACurrentPosition(h_Dev, &dwWritePosition);
		if (dwReadPosition <= dwWritePosition) {
            dwValidBufSize = dwWritePosition - dwReadPosition;
    	}
    	else {
    		dwValidBufSize = DATABUF_SIZE - dwReadPosition;
    	}        
        if (dwValidBufSize == 0) {
			Sleep(100);
			nTryNo++;
            continue;
		}
		nTryNo = 0; //reset
		fwrite((pTS_Buf+dwReadPosition), 1, dwValidBufSize, fStream);
		nGetSize += dwValidBufSize;
		dwReadPosition += dwValidBufSize;
        if (dwReadPosition >= DATABUF_SIZE)
            dwReadPosition = 0;
	}
	tEnd = GetTickCount();	
	//printf("Time=%d\n", (tEnd-tStart));
	Stop_Cap(h_Dev);
	fclose(fStream);	
#endif	

	// Close the handle
	CloseHandle (h_Dev);
	printf("Enter any key to end ..");
	getchar();
	return TRUE;
}




BOOL Get_Driver_Version(HANDLE hDevice, PCHAR psVersion)
{
	BOOLEAN bResult	= FALSE;
	LPVOID	pInBuf = NULL;
	BYTE	bOutBuf[256];
	LPVOID	pOutBuf = bOutBuf;
	int     nBytes  = 0;

	TUNER_DATA tuner_data;
	tuner_data.address = FW_VERSION_READ;
	tuner_data.frequencyMSB = 0;
    tuner_data.frequencyLSB = 0;
    tuner_data.tunerStep = 0;
    tuner_data.symbolRateHSB = 0;
    tuner_data.symbolRateMSB = 0;
    tuner_data.symbolRateLSB = 0;
    tuner_data.flag = 0;
	tuner_data.checkSum = 0;
	pInBuf = &tuner_data;
	
	bResult = FALSE;
	bResult = DeviceIoControl (hDevice,
								IOCTL_TH_USB_GET_DRIVER_VERSION,
								(LPVOID)pInBuf,
								9,
								pOutBuf,
								5,
								(unsigned long *)&nBytes,
								NULL);	
	memcpy(psVersion, (PCHAR)pOutBuf+1, 4);
	psVersion[4] = '\0';
	return bResult;

}

BOOL Get_FW_Version(HANDLE hDevice, PCHAR psVersion)
{
	BOOLEAN bResult	= FALSE;
	LPVOID	pInBuf = NULL;
	BYTE	bOutBuf[256];
	LPVOID	pOutBuf = bOutBuf;
	int     nBytes  = 0;

	TUNER_DATA tuner_data;
	tuner_data.address = FW_VERSION_READ;
	tuner_data.frequencyMSB = 0;
    tuner_data.frequencyLSB = 0;
    tuner_data.tunerStep = 0;
    tuner_data.symbolRateHSB = 0;
    tuner_data.symbolRateMSB = 0;
    tuner_data.symbolRateLSB = 0;
    tuner_data.flag = 0;
	tuner_data.checkSum = 0;
	pInBuf = &tuner_data;
	
	bResult = FALSE;
	bResult = DeviceIoControl (hDevice,
								IOCTL_TH_USB_GET_FW_VERSION,
								(LPVOID)pInBuf,
								9,
								pOutBuf,
								5,
								(unsigned long *)&nBytes,
								NULL);	
	memcpy(psVersion, (PCHAR)pOutBuf+1, 4);
	psVersion[4] = '\0';
	return bResult;

}

BOOL Get_Vender_Str(HANDLE hDevice, PCHAR psVenderStr)
{
	BOOLEAN bResult	= FALSE;
	LPVOID	pInBuf = NULL;
	BYTE	bOutBuf[256];
	LPVOID	pOutBuf = bOutBuf;
	int     nBytes  = 0;

	TUNER_DATA tuner_data;
	tuner_data.address = VENDER_STRING_READ;
	tuner_data.frequencyMSB = 0;
    tuner_data.frequencyLSB = 0;
    tuner_data.tunerStep = 0;
    tuner_data.symbolRateHSB = 0;
    tuner_data.symbolRateMSB = 0;
    tuner_data.symbolRateLSB = 0;
    tuner_data.flag = 0;
	tuner_data.checkSum = 0;
	pInBuf = &tuner_data;
	
	bResult = FALSE;
	bResult = DeviceIoControl (hDevice,
								IOCTL_TH_USB_GET_VENDER_STRING,
								(LPVOID)pInBuf,
								9,
								pOutBuf,
								8,
								(unsigned long *)&nBytes,
								NULL);	
	memcpy(psVenderStr, (PCHAR)pOutBuf+1, 7);
	psVenderStr[7] = '\0';
	return bResult;
}

BOOL Get_Product_Str(HANDLE hDevice, PCHAR psProductStr)
{
	BOOLEAN bResult	= FALSE;
	LPVOID	pInBuf = NULL;
	BYTE	bOutBuf[256];
	LPVOID	pOutBuf = bOutBuf;
	int     nBytes  = 0;

	TUNER_DATA tuner_data;
	tuner_data.address = PRODUCT_STRING_READ;
	tuner_data.frequencyMSB = 0;
    tuner_data.frequencyLSB = 0;
    tuner_data.tunerStep = 0;
    tuner_data.symbolRateHSB = 0;
    tuner_data.symbolRateMSB = 0;
    tuner_data.symbolRateLSB = 0;
    tuner_data.flag = 0;
	tuner_data.checkSum = 0;
	pInBuf = &tuner_data;
	
	bResult = FALSE;
	bResult = DeviceIoControl (hDevice,
								IOCTL_TH_USB_GET_PRODUCT_STRING,
								(LPVOID)pInBuf,
								9,
								pOutBuf,
								8,
								(unsigned long *)&nBytes,
								NULL);	
	memcpy(psProductStr, (PCHAR)pOutBuf+1, 7);
	psProductStr[7] = '\0';
	return bResult;
}

BOOL Get_Signal_Q_S(HANDLE hDevice, PDWORD pnQuality, PDWORD pnStrength, PDWORD pnLock_Status)
{
	BOOLEAN bResult	= FALSE;
	LPVOID	pInBuf = NULL;
	BYTE	bOutBuf[256];
	LPVOID	pOutBuf = bOutBuf;
	int     nBytes  = 0;

	TUNER_DATA tuner_data;
	tuner_data.address = TUNER_SIGNAL_READ;
	tuner_data.frequencyMSB = TUNER_REG_QUALITY_2;
    tuner_data.frequencyLSB = TUNER_REG_QUALITY_1;
    tuner_data.tunerStep = TUNER_REG_QUALITY_0;
    tuner_data.symbolRateHSB = TUNER_REG_STRENGTH;
    tuner_data.symbolRateMSB = 0;
    tuner_data.symbolRateLSB = 0;
    tuner_data.flag = 0;
	tuner_data.checkSum = 0;
	pInBuf = &tuner_data;
	
	bResult = FALSE;
	bResult = DeviceIoControl (hDevice,
								IOCTL_TH_USB_GET_SIGNAL_Q_S,
								(LPVOID)pInBuf,
								9,
								pOutBuf,
								3,
								(unsigned long *)&nBytes,
								NULL);	
	*pnQuality = ((PCHAR)pOutBuf)[0];
	*pnStrength = ((PCHAR)pOutBuf)[1];
	*pnLock_Status = ((PCHAR)pOutBuf)[2];
	return bResult;

}

BOOL GetDMACurrentPosition(HANDLE hDevice, PDWORD pCurrentPosition)
{
	BOOL   StatusSuccess = TRUE;
	DWORD  bytesReturned = 0;
	DWORD  outBufLength = sizeof(DWORD);

	// Get current buffer position
    StatusSuccess = DeviceIoControl(hDevice,
                                    (DWORD)IOCTL_TH_USB_GET_WPTR_OFFSET, 
                                    NULL, 
                                    0, 
                                    (LPVOID)pCurrentPosition, 
                                    outBufLength, 
                                    &bytesReturned, 
                                    NULL);
	return StatusSuccess;
}

DWORD Start_Cap(HANDLE hDevice)
{
	
	DWORD  dwDMABuffAddr = 0;
	DWORD  bytesReturned = 0;
	if (!DeviceIoControl(hDevice,
							(DWORD)IOCTL_TH_USB_START_CAP, 
							NULL,
							0,
							(LPVOID)&dwDMABuffAddr, 
							sizeof(DWORD), 
							&bytesReturned, 
							NULL)) {
        return NULL;
    }

	return dwDMABuffAddr;
}

BOOL Stop_Cap(HANDLE hDevice)
{
	
	DWORD  dwDMABuffAddr = 0;
	DWORD  bytesReturned = 0;
	if (!DeviceIoControl(hDevice,
							(DWORD)IOCTL_TH_USB_STOP_CAP, 
							NULL,
							0,
							NULL, 
							0, 
							&bytesReturned, 
							NULL)) {
        return FALSE;
    }

	return TRUE;
}


BOOL Lock_Tuner(HANDLE hDevice, UINT dwFreq, UINT dwBW)
{
	BOOLEAN bResult	= FALSE;
	LPVOID	pInBuf = NULL;
	int     nTry, nBytes  = 0;

	TUNER_DATA tuner_data;
	tuner_data.address = LOCK_TUNER_COMMAND;
	tuner_data.frequencyMSB = (BYTE)((dwFreq & 0xff0000) >> 16);
    tuner_data.frequencyLSB = (BYTE)((dwFreq & 0x00ff00) >> 8);
    tuner_data.tunerStep = (BYTE)(dwFreq & 0x0000ff);
    tuner_data.symbolRateHSB = 0;
    tuner_data.symbolRateMSB = (BYTE)dwBW; //BindWidth : 6,7,8
    tuner_data.symbolRateLSB = 0;
    tuner_data.flag = 0;
	tuner_data.checkSum = 0;
	pInBuf = &tuner_data;

	nTry = 0;
	bResult = FALSE;
	bResult = DeviceIoControl (hDevice,
								IOCTL_TH_USB_SCAN_START,
								(LPVOID)pInBuf,
								9,
								NULL,
								0,
								(unsigned long *)&nBytes,
								NULL);	

	return bResult;

}

BOOL Get_Tuner_REG(HANDLE hDevice, UINT Index, PUCHAR pValue)
{
	BOOLEAN bResult	= FALSE;
	int     nBytes  = 0;
	BYTE	bOutBuf[256];
	LPVOID	pOutBuf = bOutBuf;
	BYTE	bInBuf[256];	
	LPVOID	pInBuf = bInBuf;
	memset(bInBuf, 0, 10);
	bInBuf[0] = TUNER_REG_READ;
	bInBuf[1] = (Index & 0xFF00);  //reg_addr1
	bInBuf[2] = (Index & 0x00FF);  //reg_addr2
	                  
	
	bResult = DeviceIoControl (hDevice,
								IOCTL_TH_USB_RW_FX2_REG,
								(LPVOID)pInBuf,
								4,
								pOutBuf,
								4,
								(unsigned long *)&nBytes,
								NULL);	
	if (bResult==FALSE)	{
		printf("IOCTL Error: Get_Tuner_REG failed! \n");		
	}
	else {
		*pValue = bOutBuf[1];
	}
	return bResult;
}

BOOL Set_Tuner_REG(HANDLE hDevice, UINT Index, UCHAR Value)
{
	BOOLEAN bResult	= FALSE;
	int     nBytes  = 0;
	BYTE	bOutBuf[256];
	LPVOID	pOutBuf = bOutBuf;
	BYTE	bInBuf[256];	
	LPVOID	pInBuf = bInBuf;
	memset(bInBuf, 0, 10);
	bInBuf[0] = TUNER_REG_WRITE;
	bInBuf[1] = (Index & 0xFF00);  //reg_addr1
	bInBuf[2] = (Index & 0x00FF);  //reg_addr2
	bInBuf[3] = Value;             //reg_val
	
	bResult = DeviceIoControl (hDevice,
								IOCTL_TH_USB_RW_FX2_REG,
								(LPVOID)pInBuf,
								4,
								pOutBuf,
								4,
								(unsigned long *)&nBytes,
								NULL);	
	if (bResult==FALSE)	{
		printf("IOCTL Error: Set_Tuner_REG failed! \n");		
	}
	return bResult;
}

BOOL Get_FX2_REG(HANDLE hDevice, UINT Index, PUCHAR pValue)
{
	BOOLEAN bResult	= FALSE;
	int     nBytes  = 0;
	BYTE	bOutBuf[256];
	LPVOID	pOutBuf = bOutBuf;
	BYTE	bInBuf[256];	
	LPVOID	pInBuf = bInBuf;
	memset(bInBuf, 0, 10);
	bInBuf[0] = FX2_REG_READ;
	bInBuf[1] = (Index & 0xFF00);  //reg_addr1
	bInBuf[2] = (Index & 0x00FF);  //reg_addr2
	
	bResult = DeviceIoControl (hDevice,
								IOCTL_TH_USB_RW_FX2_REG,
								(LPVOID)pInBuf,
								4,
								pOutBuf,
								4,
								(unsigned long *)&nBytes,
								NULL);	
	if (bResult==FALSE)	{
		printf("IOCTL Error: Get_FX2_REG failed! \n");		
	}
	else {
		*pValue = bOutBuf[1];
	}
	return bResult;
}

BOOL Set_FX2_REG(HANDLE hDevice, UINT Index, UCHAR Value)
{
	BOOLEAN bResult	= FALSE;
	int     nBytes  = 0;
	BYTE	bOutBuf[256];
	LPVOID	pOutBuf = bOutBuf;
	BYTE	bInBuf[256];	
	LPVOID	pInBuf = bInBuf;
	memset(bInBuf, 0, 10);
	bInBuf[0] = FX2_REG_WRITE;
	bInBuf[1] = (Index & 0xFF00);  //reg_addr1
	bInBuf[2] = (Index & 0x00FF);  //reg_addr2
	bInBuf[3] = Value;             //reg_val
	                  
	
	bResult = DeviceIoControl (hDevice,
								IOCTL_TH_USB_RW_FX2_REG,
								(LPVOID)pInBuf,
								4,
								pOutBuf,
								4,
								(unsigned long *)&nBytes,
								NULL);	
	if (bResult==FALSE)	{
		printf("IOCTL Error: Set_FX2_REG failed! \n");		
	}
	return bResult;
}

BOOL Set_PLD_PID(HANDLE hDevice, PBYTE pPLD_buf)
{
	BOOLEAN bResult	= FALSE;
	int     nBytes  = 0;
	BYTE	bInBuf[256];
	memcpy(bInBuf, pPLD_buf, 17);
	
	bResult = DeviceIoControl (hDevice,
								IOCTL_TH_USB_SET_PLD_PID,
								(LPVOID)bInBuf,
								17,
								NULL,
								0,
								(unsigned long *)&nBytes,
								NULL);	
	if (bResult==FALSE)	{
		printf("IOCTL Error: Set_PLD_PID failed! \n");		
	}
	return bResult;
}

BOOL Get_PLD_PID(HANDLE hDevice, PBYTE pPLD_buf)
{
	BOOLEAN bResult	= FALSE;
	int     nBytes  = 0;
	BYTE	bOutBuf[256];
	LPVOID	pOutBuf = bOutBuf;
		
	bResult = DeviceIoControl (hDevice,
								IOCTL_TH_USB_GET_PLD_PID,
								NULL,
								0,
								pOutBuf,
								17,
								(unsigned long *)&nBytes,
								NULL);	
	if (bResult==FALSE)	{
		printf("IOCTL Error: Get_PLD_PID failed! \n");		
	}

	memcpy(pPLD_buf, bOutBuf, 17);
	return bResult;
}

BOOL Get_RC_VAL(HANDLE hDevice, PUCHAR pValue)
{
	BOOLEAN bResult	= FALSE;
	int     nBytes  = 0;
	BYTE	bOutBuf[256];
	LPVOID	pOutBuf = bOutBuf;
	                  	
	bResult = DeviceIoControl (hDevice,
								IOCTL_TH_USB_GET_RC_VAL,
								NULL,
								0,
								pOutBuf,
								1,
								(unsigned long *)&nBytes,
								NULL);	
	if (bResult==FALSE)	{
		printf("IOCTL Error: Get_FX2_REG failed! \n");		
	}
	else {
		*pValue = bOutBuf[0];
	}
	return bResult;
}

BOOL Get_USB_SPEED(HANDLE hDevice, PUCHAR pUSB_Speed)
{
	BOOLEAN bResult	= FALSE;
	int     nBytes  = 0;
	BYTE	bOutBuf[256];
	LPVOID	pOutBuf = bOutBuf;
	                  
	
	bResult = DeviceIoControl (hDevice,
								IOCTL_TH_USB_GET_USB_SPEED,
								NULL,
								0,
								pOutBuf,
								1,
								(unsigned long *)&nBytes,
								NULL);	
	if (bResult==FALSE)	{
		printf("IOCTL Error: Get_USB_SPEED failed! \n");		
	}
	else {
		*pUSB_Speed = bOutBuf[0];
	}
	return bResult;
}

HANDLE	OpenDriver(char* DriverName)
{
	char strDeviceName [256] = "\\\\.\\";

	strcat(strDeviceName, DriverName);

	return CreateFile(strDeviceName, 
					  GENERIC_READ | GENERIC_WRITE,
					  0,
					  NULL, 
					  OPEN_EXISTING,
					  0, 
					  NULL );
}



BOOL Set_Tuner_Power(HANDLE hDevice, UCHAR ucStatus)
{
	BOOLEAN bResult	= FALSE;
	int     nBytes  = 0;		
	BYTE	bInBuf[256];	
	memset(bInBuf, 0, 10);
	bInBuf[0] = SET_TUNER_POWER;
	bInBuf[1] = ucStatus; 
	
	bResult = DeviceIoControl (hDevice,
								IOCTL_TH_USB_SET_TUNER_POWER,
								(LPVOID)bInBuf,
								4,
								NULL,
								0,
								(unsigned long *)&nBytes,
								NULL);	
	if (bResult==FALSE)	{
		printf("IOCTL Error: SET_TUNER_POWER failed! \n");		
	}

	return bResult;
}

BOOL Set_PLD_Status(HANDLE hDevice, UCHAR ucStatus)
{
	BOOLEAN bResult	= FALSE;
	int     nBytes  = 0;	
	BYTE	bInBuf[256];	
	LPVOID	pInBuf = bInBuf;
	memset(bInBuf, 0, 10);
	bInBuf[0] = SET_PLD_STATUS;
	bInBuf[1] = ucStatus;  
	
	bResult = DeviceIoControl (hDevice,
								IOCTL_TH_USB_SET_PLD_STATUS,
								(LPVOID)pInBuf,
								4,
								NULL,
								0,
								(unsigned long *)&nBytes,
								NULL);	
	if (bResult==FALSE)	{
		printf("IOCTL Error: SET_PLD_STATUS failed! \n");		
	}

	return bResult;
}


DWORD MAKE_REMOTEBUF(HANDLE hDevice)
{
	DWORD  dwRCBuffAddr = 0;
	DWORD  bytesReturned = 0;
	if (!DeviceIoControl(hDevice,
							(DWORD)ACTTV_IOCTL_MAKE_REMOTEBUF, 
							NULL,
							0,
							(LPVOID)&dwRCBuffAddr, 
							sizeof(DWORD), 
							&bytesReturned, 
							NULL)) {
        return NULL;
    }

	return dwRCBuffAddr;
}

BOOL Start_RC(HANDLE hDevice)
{
	BOOLEAN bResult	= FALSE;
	int     nBytes  = 0;		                  
		
	bResult = DeviceIoControl (hDevice,
								DST_START_RC,
								(LPVOID)NULL,
								0,
								NULL,
								0,
								(unsigned long *)&nBytes,
								NULL);	
	if (bResult==FALSE)	{
		printf("IOCTL Error: DST_START_RC failed! \n");		
	}

	return bResult;
}


BOOL Stop_RC(HANDLE hDevice, PREMOTE_EVENT pevRC)
{
	BOOLEAN bResult	= FALSE;
	int     nBytes  = 0;	
			
	bResult = DeviceIoControl (hDevice,
								DST_STOP_RC,
								(LPVOID)pevRC, 
								sizeof(REMOTE_EVENT),
								NULL,
								0,
								(unsigned long *)&nBytes,
								NULL);	
	if (bResult==FALSE)	{
		printf("IOCTL Error: DST_STOP_RC failed! \n");		
	}

	return bResult;
}

BOOL Get_MAC(HANDLE hDevice, PUCHAR pVal)
{
	BOOLEAN bResult	= FALSE;
	BYTE	bInBuf[256];
	LPVOID	pInBuf = bInBuf;
	BYTE	bOutBuf[256];
	LPVOID	pOutBuf = bOutBuf;
	int     nBytes  = 0;

	memset(bInBuf, 0, 10);
	bInBuf[2] = 0x0A;	
	bResult = DeviceIoControl (hDevice,
								DST_SET_INFO,
								(LPVOID)pInBuf,
								9,
								pOutBuf,
								10,
								(unsigned long *)&nBytes,
								NULL);		

	if (bResult==FALSE)	{
		printf("IOCTL Error: DST_SET_INFO failed! \n");		
	}
	
	memcpy(pVal, bOutBuf, 10);

	return bResult;
}

BOOL Get_RC(HANDLE hDevice, PUCHAR pVal)
{
	BOOLEAN bResult	= FALSE;
	BYTE	bInBuf[256];
	LPVOID	pInBuf = bInBuf;
	BYTE	bOutBuf[256];
	LPVOID	pOutBuf = bOutBuf;
	int     nBytes  = 0;
	
	bResult = DeviceIoControl (hDevice,
								DST_GET_RC,
								NULL,
								0,
								pOutBuf,
								1,
								(unsigned long *)&nBytes,
								NULL);		

	if (bResult==FALSE)	{
		printf("IOCTL Error: DST_GET_RC failed! \n");		
	}
	
	memcpy(pVal, bOutBuf, 1);

	return bResult;
}

BOOL Get_Driver_Info(HANDLE hDevice, P_DriverInfo pDrvInfo)
{
	BOOLEAN bResult	= FALSE;	
	BYTE	bOutBuf[512];
	LPVOID	pOutBuf = bOutBuf;
	int     nBytes  = 0;
	
	bResult = FALSE;
	bResult = DeviceIoControl (hDevice,
								IOCTL_TH_USB_GET_DRIVER_INFO,
								NULL,
								0,
								pOutBuf,
								sizeof(DriverInfo),
								(unsigned long *)&nBytes,
								NULL);	
	
	if (bResult==FALSE)	{
		printf("IOCTL Error: Get_Driver_Info failed! \n");		
	}
	else {
		memcpy(pDrvInfo, bOutBuf, sizeof(DriverInfo));
	}
	return bResult;

}