// I2CTestTool.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "DSTIoctl.h"
#include "share.h"

#define		HW_MAX_COUNT			3

char ucDevName[32] = {"Unknow"};
char ucDevNameTest[32] = {"Unknow"};

// Calculate check sum ?
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

int TestReset(HANDLE hVideo)
{
    BOOL			StatusSuccess = TRUE;
	LPVOID			inBuf;
	LPVOID			outBuf; 
	DWORD			inBufLength;
	DWORD			outBufLength;
    ULONG			bytesReturned;
	BYTE			bBuf[256];

	if (hVideo == NULL) {
		return 0;
	}
	
	TunerData tunerData;
	memset(&tunerData,0,sizeof(TunerData));

	tunerData.address = 0xaa;
	tunerData.frequencyMSB = 0;
	tunerData.frequencyLSB = 0x0;
	
	MakeCheckSum(&tunerData);
	
	inBuf = &tunerData;
	inBufLength	= sizeof(tunerData);		
	outBuf = (LPVOID)bBuf;
	outBufLength = inBufLength;
	
	StatusSuccess = DeviceIoControl(hVideo, 
									(DWORD)DST_IOCTL_RDC8820_RESET, 
									inBuf, 
									inBufLength + 1, 
									outBuf, 
									outBufLength, 
									&bytesReturned, 
									NULL);


	if (!StatusSuccess) {
		return 0;
	}
     cout <<  "Reset ok!"  << endl;
		return 1;

}
int GetLockStatus(HANDLE hVideo)
{
    BOOL			StatusSuccess = TRUE;
	LPVOID			inBuf;
	LPVOID			outBuf; 
	DWORD			inBufLength;
	DWORD			outBufLength;
    ULONG			bytesReturned;
	BYTE			bBuf[256];

	if (hVideo == NULL) {
		return 0;
	}
	
	TunerData tunerData;
	memset(&tunerData,0,sizeof(TunerData));

	tunerData.address = 0xaa;
	tunerData.frequencyMSB = 0;
	tunerData.frequencyLSB = 0x05;
	
	MakeCheckSum(&tunerData);
	
	inBuf = &tunerData;
	inBufLength	= sizeof(tunerData);		
	outBuf = (LPVOID)bBuf;
	outBufLength = inBufLength;
	
	StatusSuccess = DeviceIoControl(hVideo, 
									(DWORD)DST_IOCTL_SET_INFO, 
									inBuf, 
									inBufLength + 1, 
									outBuf, 
									outBufLength, 
									&bytesReturned, 
									NULL);
	if (!StatusSuccess) {
		return 0;
	}

	char *chName = (char *)((BYTE *)outBuf + 1);
	printf("Lock status: %02.2x - %02.2x - %02.2x - %02.2x - %02.2x - %02.2x - %02.2x \n",  
(BYTE)chName[0],
(BYTE)chName[1],
(BYTE)chName[2],
(BYTE)chName[3],
(BYTE)chName[4],
(BYTE)chName[5],
(BYTE)chName[6]);
		return 1;
}


int getCIInfo(HANDLE hVideo, BOOL bShowStrFlag)
{
    BOOL			StatusSuccess = TRUE;
	LPVOID			inBuf;
	LPVOID			outBuf; 
	DWORD			inBufLength;
	DWORD			outBufLength;
    ULONG			bytesReturned;
	BYTE			bBuf[256];

	if (hVideo == NULL) {
		return 0;
	}
	
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
	
	StatusSuccess = DeviceIoControl(hVideo, 
									(DWORD)DST_IOCTL_SET_INFO, 
									inBuf, 
									inBufLength + 1, 
									outBuf, 
									outBufLength, 
									&bytesReturned, 
									NULL);
	if (!StatusSuccess) {
		return 0;
	}

	//0 : Old DST Card					--- 188bytes;
	//1 : New DST Card(USALS)			--- 188bytes;
	//2 : New DST Card(USALS, 3 Tuners) --- 204bytes;
	//3 : DST CI Card					--- 204bytes;
	//4 : DST CI Card(188 Bytes)		--- 188bytes;
	//5 : DCT Card						--- 188bytes;
	//6 : DCT CI Card					--- 204bytes;
	//10: DTT Card(Digtal & Analog)     --- 188bytes;
	//11: DTT Card(Digtal)              --- 188bytes;
    //12: DTT CI Card                   --- 188bytes;
    //14: ATSC Digital only             --- 188bytes;
    //15: ATSC A+D                      --- 188bytes;

	char *chName = (char *)((BYTE *)outBuf + 2);
	chName[6] = 0;
    
    if (bShowStrFlag) {
        cout << "Card Type: " << chName << endl;
        strcpy(ucDevName, chName);
    }

    strcpy(ucDevNameTest, chName);

	if (0 == strcmp(chName,"ST-MOT")) {
		return 1;
	}

	if (0 == strcmp(chName,"ST-03T")) {
		return 2;
	}

	if (0 == strcmp(chName,"DST-CI")) {
		return 3;
	}

	if (0 == strcmp(chName,"DSTMCI")) {	
		return 4;
	}

	if (0 == strcmp(chName,"DCTNEW")) {
		return 5;
	}

	if (0 == strcmp(chName,"DCT-CI")) {
		return 6;
	}

	if (0 == strcmp(chName,"DTTNXT")) {
		return 10;
	}

	if (0 == strcmp(chName,"DTTDIG")) {
		return 11;
	}

	if (0 == strcmp(chName,"DTT-CI")) {
		return 12;
	}

    if (0 == strcmp(chName,"ATSCDI")) {
		return 14;
	}

    if (0 == strcmp(chName,"ATSCAD")) {
		return 15;
	}

	return 0;
}

int bcd2hex(unsigned char bcd)
{
    return(((bcd&0xf0)>>4)*10+(bcd&0xf));
}

int getVerInfo(HANDLE hVideo)
{
                  BOOL			StatusSuccess = TRUE;
	LPVOID			inBuf;
	LPVOID			outBuf; 
	DWORD			inBufLength;
	DWORD			outBufLength;
                  ULONG			bytesReturned;
	BYTE			bBuf[256];	
    char *chName;

	if (hVideo == NULL) {
		return 0;
	}
	
	TunerData tunerData;
	memset(&tunerData,0,sizeof(TunerData));
	memset(&bBuf,0,sizeof(bBuf));
	tunerData.address = 0xaa;
	tunerData.frequencyMSB = 0;
	tunerData.frequencyLSB = 0x06;
	
	MakeCheckSum(&tunerData);
	
	inBuf = &tunerData;
	inBufLength= sizeof(tunerData);		
	outBuf = (LPVOID)bBuf;
	outBufLength = inBufLength;
	
	StatusSuccess = DeviceIoControl(hVideo, 
									(DWORD)DST_IOCTL_SET_INFO, 
									inBuf, 
									inBufLength + 1, 
									outBuf, 
									outBufLength, 
									&bytesReturned, 
									NULL);
	if (!StatusSuccess) {
		return 0;
	}

	chName = (char *)((BYTE *)outBuf + 2);
    	chName[6] = 0;
    cout <<  "Card ID:"  <<chName<< endl;
  /*                
    cout <<  (int) (char *)((BYTE *)outBuf)[0] << endl;
    cout <<  (int) (char *)((BYTE *)outBuf)[1]<< endl;
    cout <<  (int) (char *)((BYTE *)outBuf)[2]<< endl;
    cout <<  (int) (char *)((BYTE *)outBuf)[3]<< endl;
    cout <<  (int) (char *)((BYTE *)outBuf)[4]<< endl;
    cout <<  (int) (char *)((BYTE *)outBuf)[5]<< endl;
    */
                  /***********************************************************************************/

	tunerData.address = 0xaa;
	tunerData.frequencyMSB = 0;
	tunerData.frequencyLSB = 0x10;
    memset(&bBuf,0,sizeof(bBuf));
	
	MakeCheckSum(&tunerData);
	
	inBuf = &tunerData;
	inBufLength= sizeof(tunerData);		
	outBuf = (LPVOID)bBuf;
	outBufLength = inBufLength;
	
	StatusSuccess = DeviceIoControl(hVideo, 
									(DWORD)DST_IOCTL_SET_INFO, 
									inBuf, 
									inBufLength + 1, 
									outBuf, 
									outBufLength, 
									&bytesReturned, 
									NULL);
	if (!StatusSuccess) {
		return 0;
	}

	chName = (char *)((BYTE *)outBuf + 1);
                  //cout <<  (int) (char *)((BYTE *)outBuf + 1)[0]<< endl;
                  cout <<  "Ver:"  << bcd2hex(chName[0])/10<< "."<< bcd2hex(chName[0])%10<<"B" << bcd2hex(chName[1])<< endl;
                  cout <<  "Date:" << 2000+bcd2hex(chName[2])<<"-"<< bcd2hex(chName[3])<<"-"<< bcd2hex(chName[4])<< endl; 
                  cout <<  "Time:" << bcd2hex(chName[5])<<":" << bcd2hex(chName[6])<< endl;


                  /***********************************************************************************/
	tunerData.address = 0xaa;
	tunerData.frequencyMSB = 0;
	tunerData.frequencyLSB = 0x11;
        memset(&bBuf,0,sizeof(bBuf));
	
	MakeCheckSum(&tunerData);
	
	inBuf = &tunerData;
	inBufLength= sizeof(tunerData);		
	outBuf = (LPVOID)bBuf;
	outBufLength = inBufLength;
	
	StatusSuccess = DeviceIoControl(hVideo, 
									(DWORD)DST_IOCTL_SET_INFO, 
									inBuf, 
									inBufLength + 1, 
									outBuf, 
									outBufLength, 
									&bytesReturned, 
									NULL);
	if (!StatusSuccess) {
		//return 0;
	}

	chName = (char *)((BYTE *)outBuf + 1);
	chName[7] = 0;
                  //cout <<  (int) (char *)((BYTE *)outBuf + 1)[0]<< endl;
                  cout <<  "Model:"  << chName<< endl;

                  /***********************************************************************************/
	tunerData.address = 0xaa;
	tunerData.frequencyMSB = 0;
	tunerData.frequencyLSB = 0x12;
        memset(&bBuf,0,sizeof(bBuf));
	
	MakeCheckSum(&tunerData);
	
	inBuf = &tunerData;
	inBufLength= sizeof(tunerData);		
	outBuf = (LPVOID)bBuf;
	outBufLength = inBufLength;
	
	StatusSuccess = DeviceIoControl(hVideo, 
									(DWORD)DST_IOCTL_SET_INFO, 
									inBuf, 
									inBufLength + 1, 
									outBuf, 
									outBufLength, 
									&bytesReturned, 
									NULL);
	if (!StatusSuccess) {
		return 0;
	}

	chName = (char *)((BYTE *)outBuf + 1);
	chName[7] = 0;
                  //cout <<  (char *)((BYTE *)outBuf + 1)[0]<< endl;
                  cout <<  "Company:"  << chName<< endl;
               /***********************************************************************************/
	tunerData.address = 0xaa;
	tunerData.frequencyMSB = 0;
	tunerData.frequencyLSB = 0x13;
        memset(&bBuf,0,sizeof(bBuf));
	
	MakeCheckSum(&tunerData);
	
	inBuf = &tunerData;
	inBufLength= sizeof(tunerData);		
	outBuf = (LPVOID)bBuf;
	outBufLength = inBufLength;
	
	StatusSuccess = DeviceIoControl(hVideo, 
									(DWORD)DST_IOCTL_SET_INFO, 
									inBuf, 
									inBufLength + 1, 
									outBuf, 
									outBufLength, 
									&bytesReturned, 
									NULL);
	if (!StatusSuccess) {
		//return 0;
	}

	chName = (char *)((BYTE *)outBuf + 1);
	chName[7] = 0;
                  //cout <<  (int) (char *)((BYTE *)outBuf + 1)[0]<< endl;
                  cout <<  "Packet length:"  << (int)(unsigned char)chName[0]<< "   Daughter BD/FPGA:"<<(int)chName[1]<< endl;

	return 0;
}
HANDLE CheckSetupDev(GUID ClassGuid,PSP_INTERFACE_DEVICE_DETAIL_DATA functionClassDeviceData,char* szErrMsg)
{
    HDEVINFO                            hardwareDeviceInfo;
    SP_INTERFACE_DEVICE_DATA            deviceInfoData;
    DWORD                               predictedLength = 0;
    DWORD                               requiredLength = 0;
    HANDLE								hDev = INVALID_HANDLE_VALUE;

	//
    // Open a handle to the plug and play dev node.
    //
    hardwareDeviceInfo = SetupDiGetClassDevs ((LPGUID)&ClassGuid,
											  NULL,
											  NULL, // Define no
											  (DIGCF_PRESENT | // Only Devices present
											  DIGCF_INTERFACEDEVICE)); // Function class devices.
    if(0 == hardwareDeviceInfo) {
        return (hDev);
    }
    
    deviceInfoData.cbSize = sizeof (SP_INTERFACE_DEVICE_DATA);

    if (SetupDiEnumDeviceInterfaces (hardwareDeviceInfo,
                                     0,
                                     (LPGUID)&ClassGuid,
                                     0,
                                     &deviceInfoData)) {
       SetupDiGetInterfaceDeviceDetail (
                hardwareDeviceInfo,
                &deviceInfoData,
                NULL,					// probing so no output buffer yet
                0,						// probing so output buffer length of zero
                &requiredLength,
                NULL);					// not interested in the specific dev-node


       predictedLength = requiredLength;

       functionClassDeviceData = (PSP_INTERFACE_DEVICE_DETAIL_DATA)malloc(predictedLength);
       functionClassDeviceData->cbSize = sizeof(SP_INTERFACE_DEVICE_DETAIL_DATA);

       if (! SetupDiGetInterfaceDeviceDetail(hardwareDeviceInfo,
											 &deviceInfoData,
											 functionClassDeviceData,
											 predictedLength,
											 &requiredLength,
											 NULL)) {
           free (functionClassDeviceData);
           return (hDev);
       }
       
    }
    else if (ERROR_NO_MORE_ITEMS != GetLastError()) {
		free (functionClassDeviceData);
		SetupDiDestroyDeviceInfoList (hardwareDeviceInfo);
		return (hDev);
    }

    SetupDiDestroyDeviceInfoList (hardwareDeviceInfo);

    if (functionClassDeviceData == NULL) {
        return (hDev);
    }

	hDev = CreateFile(functionClassDeviceData->DevicePath, 
					  GENERIC_READ | GENERIC_WRITE,
					  0,
					  NULL, 
					  OPEN_EXISTING,
					  0, 
					  NULL );
	if ( hDev == INVALID_HANDLE_VALUE  ) {
	    free (functionClassDeviceData);
		return (hDev);
	}
	
	return (hDev);
}

HANDLE OpenVideoDriver()
{
	char szErrMsg[128];

	return CheckSetupDev(GUID_DST_VIDEODEV,NULL,szErrMsg);
}

HANDLE OpenAudioDriver()
{
	char szErrMsg[128];

	return CheckSetupDev(GUID_DST_AUDIODEV,NULL,szErrMsg); 	
}

HANDLE	OpenMulVideoDriver(int DriverNum)
{
	char strtemp[8];
	char strDeviceName [256] = "\\\\.\\ZVVid";

	_itoa(DriverNum, strtemp, 10);
	strcat(strDeviceName, strtemp);

	return CreateFile(strDeviceName, 
					  GENERIC_READ | GENERIC_WRITE,
					  0,
					  NULL, 
					  OPEN_EXISTING,
					  0, 
					  NULL );
}

HANDLE  OpenMulAudioDriver(int DriverNum)
{
	char strtemp[8];
	char strDeviceName [256] = "\\\\.\\ZVAud";

	_itoa(DriverNum, strtemp, 10);
	strcat(strDeviceName, strtemp);

	return CreateFile(strDeviceName, 
					  GENERIC_READ | GENERIC_WRITE,
					  0,
					  NULL, 
					  OPEN_EXISTING,
					  0, 
					  NULL );
}

int	GetMaxCard(HANDLE hVideo, int *DriverType)
{
	BOOL			StatusSuccess = FALSE;
	PVOID			cBuffer = NULL;
	DWORD			bytesReturned;
	PCIConfig2		pc;

	cBuffer = &pc;
	StatusSuccess = DeviceIoControl(hVideo,
									(DWORD)DST_GET_DEVICE_INFO, 
									NULL, 
									0, 
									cBuffer, 
									sizeof(PCIConfig2)+1, 
									&bytesReturned, 
									NULL);
	if (!StatusSuccess) {
		return 1;
    }

	if (bytesReturned == 15) {
		// old driver
		if (DriverType)
			*DriverType = 0;
	}

	if (bytesReturned == 17) {
		// multi-card support driver
		if (DriverType)
			*DriverType = 1;
	}

	return (int)pc.MaxCardNum;
}

BOOL IsNumberStr(char* str)
{
	int i = 0;
	while(str[i] != 0)
	{
		if( !(str[i] >= '0' && str[i] <='9'))
		{
			return FALSE;
		}
		i++;
	}
	return TRUE;
	
}

void StrEmpty(char* str)
{
	for(int i = 0; i <= 255 ; i ++)
	{
		str[i] = 0;
	}
	return;

}

bool setTone(
    bool K22Enable,
	bool PowerOnEnable,
    HANDLE hVideo)
{
    BOOL   StatusSuccess = TRUE;
    LPVOID inBuf;
    LPVOID outBuf; 
    DWORD  inBufLength;
    DWORD  outBufLength;
    ULONG  bytesReturned;
    BYTE   bBuf[256];
    
    if (hVideo == NULL) {
    	return false;
    }
    
    TunerData tunerData;
    memset(&tunerData, 0, sizeof(TunerData));
    
    tunerData.address = 0xaa;
    tunerData.frequencyMSB = 0;
    tunerData.frequencyLSB = 0x09;
    
    if (K22Enable)
    	tunerData.tunerStep = 0x02;
    else
    	tunerData.tunerStep = 0;
    
    tunerData.symbolRateHSB = 0xff; // Disable the Tone/Data Burst function
    
    tunerData.symbolRateMSB = 0x01; // Power On
    
    MakeCheckSum(&tunerData);
    
    inBuf = &tunerData;
    inBufLength	= sizeof(tunerData);		
    outBuf = (LPVOID)bBuf;
    outBufLength = inBufLength;
    
    StatusSuccess = DeviceIoControl(hVideo, 
                                    (DWORD)DST_IOCTL_SET_INFO, 
                                    inBuf, 
                                    inBufLength + 1, 
                                    outBuf, 
                                    outBufLength, 
                                    &bytesReturned, 
                                    NULL);
    if (!StatusSuccess) {
    	return false;
    }

    return true;
}

// Binry Float Divide
long BinaryFloatDiv(long n1, long n2, int precision)
{
	int i = 0;
    long result = 0;
    
	// division de N1 par N2 avec N1<N2
    while ( i <= precision ) //*n1>0
    {
		if ( n1 < n2 )
		{
			result *= 2;
			n1 *= 2;
		}
		else
		{
			result = result * 2 + 1;
			n1 = ( n1 - n2 ) * 2;
		}
		i++;
	}
    
    return result;
}

long ConvProgPara(long Freqency,
			      long SymbolRate,
				  int  H_V, 
				  long LNBValue1,
				  long LNBValue2,
				  TunerData *pTunerData)
{
	long inFrequency = 0;
	long inFrequency1 = 0;
	long inFrequency2 = 0;
	long LNB = 0;
	long LNB1 = 0;
	long LNB2 = 0;
	long tunerStep;
    long InNumber;
    int	 divRate = 0;
	int	 i = 0;

	/*CalFrequency*/
	long frequencyRef[16] = {
		2, 4, 8, 16, 32, 64, 128, 256, 24, 5, 10, 20, 40, 80, 160, 320 };

	if (Freqency > LNBValue1)
		inFrequency1 = Freqency - LNBValue1;
	else
		inFrequency1 = LNBValue1 - Freqency;
	if ((inFrequency1 > 950) && (inFrequency1 < 2150)) {
		LNB1 = LNBValue1;
	}

	if (Freqency > LNBValue2)
		inFrequency2 = Freqency - LNBValue2;
	else
		inFrequency2 = LNBValue2 - Freqency;
	if ((inFrequency2 > 950) && (inFrequency2 < 2150)) {
		LNB2 = LNBValue2;
	}

	if (LNB1 && !LNB2) {
		LNB = LNBValue1;
		inFrequency = inFrequency1;
	}
	else if (LNB2 && !LNB1) {
		LNB = LNBValue2;
		inFrequency = inFrequency2;
	}
	else if (LNB1 && LNB2) {
		if (((inFrequency1 > 600) ? (inFrequency1 - 600) : (600 - inFrequency1)) <=
			((inFrequency2 > 600) ? (inFrequency2 - 600) : (600 - inFrequency2))) {
			LNB = LNBValue1;
			inFrequency = inFrequency1;
		}
		else {
			LNB = LNBValue2;
			inFrequency = inFrequency2;
		}
	}
	else {
		return 0;
	}

	tunerStep = 1000000;    //250000
	
	InNumber = inFrequency * 1000 / ( tunerStep / 1000 );
	pTunerData->frequencyMSB = (InNumber & 0x00007f00) >> 8;
	pTunerData->frequencyLSB = InNumber & 0x000000ff;
	divRate = 4000000L / tunerStep;
	
	for (i=0;i<16;i++) {
		if (frequencyRef[i] == divRate)
			break;
	}
	
	if (i > 15)
		i = 15;
	
	pTunerData->tunerStep = i;

	/*CalSymbolRate*/
    long Result = BinaryFloatDiv(SymbolRate, 88000000, 20);

    pTunerData->symbolRateHSB = (Result >> 12) & 0xFF;
    pTunerData->symbolRateMSB = (Result >> 4) & 0xFF;
    pTunerData->symbolRateLSB = ((Result & 0x0F) << 4) & 0xF0;

	// adjust value
	pTunerData->flag = 0;
	
	/*h/v chosen*/
	if (H_V)
		pTunerData->flag |= 0x0040;

	// finally, calculate and set the check sum
	MakeCheckSum(pTunerData);

	return LNB;
}

BOOL setDiSEqC(unsigned char b1,
			   unsigned char b2, 
			   unsigned char b3,
			   unsigned char b4,
			   HANDLE hVideo)
{
    BOOL			StatusSuccess = TRUE;
	LPVOID			inBuf;
	LPVOID			outBuf; 
	DWORD			inBufLength;
	DWORD			outBufLength;
    ULONG			bytesReturned;
	BYTE			bBuf[256];

	if (hVideo == NULL) {
		return false;
	}

	TunerData tunerData;
	memset(&tunerData,0,sizeof(TunerData));

	tunerData.address = 0xaa;
	tunerData.frequencyMSB = 0;
	tunerData.frequencyLSB = 0x08;

	if (b4 == 0)
		tunerData.tunerStep = 0x03;
	else 
		tunerData.tunerStep = 0x04;
	tunerData.symbolRateHSB = b1;
	tunerData.symbolRateMSB = b2;
	tunerData.symbolRateLSB = b3;
	tunerData.flag = b4;

	MakeCheckSum(&tunerData);

	inBuf = &tunerData;
	inBufLength	= sizeof(tunerData);		
	outBuf = (LPVOID)bBuf;
	outBufLength = inBufLength;

	StatusSuccess = DeviceIoControl(hVideo, 
									(DWORD)DST_IOCTL_SET_INFO, 
									inBuf, 
									inBufLength + 1, 
									outBuf, 
									outBufLength, 
									&bytesReturned, 
									NULL);
	if (!StatusSuccess) {
		return false;
    }

	return true;
}

void resetHardware(HANDLE hVideo)
{
    BOOL			StatusSuccess = TRUE;
    ULONG			bytesReturned;

	StatusSuccess = DeviceIoControl(hVideo, 
									(DWORD)DST_IOCTL_RDC8820_RESET, 
									NULL, 
									0, 
									NULL, 
									0, 
									&bytesReturned, 
									NULL);
}

BOOL lockChannel(long Frequency,
				 long SymbolRate,
				 int  HV,
				 int  Tone,
				 int  DiSEqC,
				 int  LNBType,
				 long LNBValue1,
				 long LNBValue2,
				 int  Mode,
                 int  LNBPower,
				 HANDLE hVideo)
{
    BOOL			StatusSuccess = TRUE;
	LPVOID			inBuf;
	LPVOID			outBuf; 
	DWORD			inBufLength;
	DWORD			outBufLength;
    ULONG			bytesReturned;

	BYTE			InBuf[256];
	BYTE			bBuf[256];
//	lock_status		LockStatus;

	if ((hVideo==NULL) || (hVideo==INVALID_HANDLE_VALUE)) {
		return false;
	}

	TunerData tunerData;
	memset(&tunerData,0,sizeof(TunerData));

	//0 : Old DST Card					--- 188bytes;
	//1 : New DST Card(USALS)			--- 188bytes;
	//2 : New DST Card(USALS, 3 Tuners) --- 204bytes;
	//3 : DST CI Card					--- 204bytes;
	//4 : DST CI Card(188 Bytes)		--- 188bytes;
	//5 : DCT Card						--- 188bytes;
	//6 : DCT CI Card					--- 204bytes;
	//10: DTT Card(Digtal & Analog)     --- 188bytes;
	//11: DTT Card(Digtal)              --- 188bytes;
    //12: DTT CI Card                   --- 188bytes;
    //14: ATSC Digital only             --- 188bytes;
    //15: ATSC A+D                      --- 188bytes;

	//printf("Mode = %d", Mode);
	if ((Mode == 0) || (Mode == 1) || (Mode == 2) || (Mode == 3) || (Mode == 4)) {
		switch (LNBType) {
			case 0:	//Normal
				LNBValue1 = 5150;
				LNBValue2 = 5750;
				break;
			case 1:	//Universal
				LNBValue1 = 10600;
				LNBValue2 = 9750;
				break;
			case 2:	//Custom
				break;
			default:
				LNBValue1 = 5150;
				LNBValue2 = 5750;
				break;
		}

		long LNB = ConvProgPara(Frequency,
								SymbolRate * 1000,
								HV,
								LNBValue1,
								LNBValue2,
								&tunerData);

		if (LNB == 0) {
			return false;
		}

		bool K22Enable = false;
		unsigned char X1 = 0;
		unsigned char X2 = 0;
		unsigned char X3 = 0;
		unsigned char X4 = 0;

		if (DiSEqC) {
			X1 = 0xe0;
			X2 = 0x10;
			X3 = 0x38;
			
			X4 = 0xf0;
			// high low band; low band bit0 = 0;
			if (LNB == 10600) {
				X4 += 0x01;
			}
			// polarzation; if (V or L) bit1 = 0;
			if (HV) {
				X4 += 0x02;
			}

			//satellite position and option; bit2 and bit3;	
			switch (DiSEqC) {
			case 1:
				X4 &= 0xf3;			//0xf0,0xf1,0xf2,0xf3;
				break;
			case 2:
				X4 += 0x04;			//0xf4,0xf5,0xf6,0xf7;
				break;
			case 3:
				X4 += 0x08;			//0xf8,0xf9,0xfa,0xfb;
				break;
			case 4:
				X4 += 0x0c;			//0xfc,0xfd,0xfe,0xff;
				break;
			default:
				X4 = 0xf0;
			}
			
			setDiSEqC(X1,X2,X3,X4,hVideo);

//			Sleep(100);
		}

		if (LNBType == 1) {
			if (LNB == 10600) {
				K22Enable = true;
			}
			if (LNB == 9750) {
				K22Enable = false;
			}

//			if (!DiSEqC)
				//setTone(K22Enable, LNBPower, hVideo);
				setTone(K22Enable, LNBPower, hVideo);

//			Sleep(100);
		}
		else {
			if (Tone) {
				K22Enable = true;
			}
			else {
				K22Enable = false;
			}
				
			setTone(K22Enable, LNBPower, hVideo);
				
//			Sleep(100);
		}

		if ((Mode == 0) || (Mode == 1) || (Mode == 2)) {
			inBuf = &tunerData;
			inBufLength	= sizeof(tunerData);		
			outBuf = (LPVOID)bBuf;
			outBufLength = inBufLength;
			// The First step, scan start. I2c Write tuner data ...
			StatusSuccess = DeviceIoControl(hVideo, 
											(DWORD)DST_IOCTL_SCAN_START, 
											inBuf,
											inBufLength + 1,
											outBuf, 
											outBufLength, 
											&bytesReturned, 
											NULL);
			if (!StatusSuccess) {
				return false;
			}
			// The second step, waiting for Tuner Locking...
			StatusSuccess = FALSE;
			DWORD LockTime = GetTickCount();
			while (!StatusSuccess) {
				if ((GetTickCount() - LockTime) > 6000) {
					break;
				}
				StatusSuccess = DeviceIoControl(hVideo, 
												(DWORD)DST_IOCTL_SCAN_WAITING, 
												NULL, 
												0, 
												NULL, 
												0, 
												&bytesReturned, 
												NULL);
				Sleep(1);
			}
			if (!StatusSuccess) {
				return false;
			}
			// The third step
			StatusSuccess = DeviceIoControl(hVideo, 
											(DWORD)DST_IOCTL_SCAN_END, 
											inBuf, 
											inBufLength + 1, 
											outBuf, 
											outBufLength, 
											&bytesReturned, 
											NULL);
			if (StatusSuccess) {
			}
		}
		else {
			inBuf = (PVOID)InBuf;
			inBufLength = 256;
			InBuf[0] = 0xaa;
			InBuf[1] = 9;
			InBuf[2] = 0;
			InBuf[3] = tunerData.frequencyMSB;
			InBuf[4] = tunerData.frequencyLSB;
			InBuf[5] = tunerData.tunerStep;
			InBuf[6] = 0;
			InBuf[7] = (BYTE)((SymbolRate & 0xff00) >> 8);
			InBuf[8] = (BYTE)(SymbolRate & 0xff);
			InBuf[9] = tunerData.flag;
			InBuf[10] = 0;
			for (int l=1;l<10;l++) {
				InBuf[10] += InBuf[l];
			}
			InBuf[10] = ~InBuf[10] + 1;

			StatusSuccess = DeviceIoControl(hVideo, 
											(DWORD)DST_IOCTL_CA_WRITE, 
											inBuf,
											inBufLength,
											NULL, 
											0, 
											&bytesReturned, 
											NULL);
			if (!StatusSuccess) {
				resetHardware(hVideo);
				Sleep(600);
				return false;
			}

			StatusSuccess = DeviceIoControl(hVideo, 
											(DWORD)DST_IOCTL_CA_READ, 
											NULL,
											0,
											inBuf, 
											inBufLength, 
											&bytesReturned, 
											NULL);
			if (!StatusSuccess) {
				resetHardware(hVideo);
				Sleep(600);
				return false;
			}

			if ((InBuf[1] == tunerData.frequencyMSB) && (InBuf[2] == tunerData.frequencyLSB)) {
			}
			else {
				StatusSuccess = FALSE;
			}

//			Sleep(600);
		}
	}
	else if (Mode == 5) {
		tunerData.frequencyMSB = (BYTE)((Frequency & 0xff0000) >> 16);
		tunerData.frequencyLSB = (BYTE)((Frequency & 0x00ff00) >> 8);
		tunerData.tunerStep = (BYTE)(Frequency & 0x0000ff);
		
		tunerData.symbolRateHSB = (BYTE)(SymbolRate >> 8);
		tunerData.symbolRateMSB = (BYTE)(SymbolRate & 0x00ff);
		tunerData.symbolRateLSB = 0;
		
		if (HV == 256)
			tunerData.flag = 255;
		else
			tunerData.flag = (BYTE)HV;
		
		// finally, calculate and set the check sum
		MakeCheckSum(&tunerData);
		
		inBuf = &tunerData;
		inBufLength	= sizeof( tunerData );		
		outBuf = (LPVOID)bBuf;
		outBufLength = inBufLength;
		// The First step, scan start. I2c Write tuner data ...
		StatusSuccess = DeviceIoControl(hVideo, 
										(DWORD)DST_IOCTL_SCAN_START, 
										inBuf,
										inBufLength + 1,
										outBuf, 
										outBufLength, 
										&bytesReturned, 
										NULL);
		if (!StatusSuccess) {
			return false;
		}
		// The second step, waiting for Tuner Locking...
		StatusSuccess = FALSE;
		DWORD LockTime = GetTickCount();
		while (!StatusSuccess) {
			if ((GetTickCount() - LockTime) > 6000) {
				break;
			}
			
			StatusSuccess = DeviceIoControl(hVideo, 
											(DWORD)DST_IOCTL_SCAN_WAITING, 
											NULL, 
											0, 
											NULL, 
											0, 
											&bytesReturned, 
											NULL);
			Sleep(1);
		}
		if (!StatusSuccess) {
			return false;
		}
		// The third step
		StatusSuccess = DeviceIoControl(hVideo, 
										(DWORD)DST_IOCTL_SCAN_END, 
										inBuf, 
										inBufLength + 1, 
										outBuf, 
										outBufLength, 
										&bytesReturned, 
										NULL);
		if (StatusSuccess) {
		}
	}
	else if ((Mode==6) || (Mode==12)) {
		BYTE F1 = (BYTE)((Frequency & 0xff0000) >> 16);
		BYTE F2 = (BYTE)((Frequency & 0x00ff00) >> 8);
		BYTE F3 = (BYTE)(Frequency & 0xff);
		
		inBuf = (PVOID)InBuf;
		inBufLength = 256;
		InBuf[0] = 0xaa;
		InBuf[1] = 9;
		InBuf[2] = 0;
		InBuf[3] = F1;
		InBuf[4] = F2;
		InBuf[5] = F3;
		InBuf[6] = (BYTE)((SymbolRate &0xff0000) >> 16);
		InBuf[7] = (BYTE)((SymbolRate &0x00ff00) >> 8);
		InBuf[8] = (BYTE)(SymbolRate & 0xff);
		if (HV == 256) {
			InBuf[9] = 0;
		}
		else {
			InBuf[9] = (BYTE)(HV);
		}
		InBuf[10] = 0;
		for (int l=1;l<10;l++) {
			InBuf[10] += InBuf[l];
		}
		InBuf[10] = ~InBuf[10] + 1;
		
		StatusSuccess = DeviceIoControl(hVideo, 
										(DWORD)DST_IOCTL_CA_WRITE, 
										inBuf,
										inBufLength,
										NULL, 
										0, 
										&bytesReturned, 
										NULL);
		if (!StatusSuccess) {
			return false;
		}
		
		StatusSuccess = DeviceIoControl(hVideo, 
										(DWORD)DST_IOCTL_CA_READ, 
										NULL,
										0,
										inBuf, 
										inBufLength, 
										&bytesReturned, 
										NULL);
		if (!StatusSuccess) {
			return false;
		}
		
		if ((InBuf[1] == F1) && (InBuf[2] == F2) && (InBuf[3] == F3)) {
		}
		else {
			StatusSuccess = FALSE;
		}
	}
	else if (Mode == 10) {
		if (HV) {
			//Digital TV Locking
			tunerData.frequencyMSB = (BYTE)((Frequency & 0xff0000) >> 16);
			tunerData.frequencyLSB = (BYTE)((Frequency & 0x00ff00) >> 8);
			tunerData.tunerStep = (BYTE)(Frequency & 0x0000ff);
			tunerData.symbolRateHSB = 0;
			tunerData.symbolRateMSB = (BYTE)SymbolRate;		//BindWidth : 6,7,8
			tunerData.symbolRateLSB = 0;
			tunerData.flag = 0;

			// finally, calculate and set the check sum
			MakeCheckSum(&tunerData);
		
			inBuf = &tunerData;
			inBufLength	= sizeof( tunerData );		
			outBuf = (LPVOID)bBuf;
			outBufLength = inBufLength;
			// The First step, scan start. I2c Write tuner data ...
			StatusSuccess = DeviceIoControl(hVideo, 
											(DWORD)DST_IOCTL_SCAN_START, 
											inBuf,
											inBufLength + 1,
											outBuf, 
											outBufLength, 
											&bytesReturned, 
											NULL);
			if (!StatusSuccess) {
				return false;
			}
			// The second step, waiting for Tuner Locking...
			StatusSuccess = FALSE;
			DWORD LockTime = GetTickCount();
			while (!StatusSuccess) {
				if ((GetTickCount() - LockTime) > 6000) {
					break;
				}
				
				StatusSuccess = DeviceIoControl(hVideo, 
												(DWORD)DST_IOCTL_SCAN_WAITING, 
												NULL, 
												0, 
												NULL, 
												0, 
												&bytesReturned, 
												NULL);
				Sleep(1);
			}
			if (!StatusSuccess) {
				return false;
			}
			// The third step
			StatusSuccess = DeviceIoControl(hVideo, 
											(DWORD)DST_IOCTL_SCAN_END, 
											inBuf, 
											inBufLength + 1, 
											outBuf, 
											outBufLength, 
											&bytesReturned, 
											NULL);
			if (StatusSuccess) {
			}
		}
		else {
			//Analog TV Locking
			tunerData.frequencyMSB = (BYTE)((Frequency & 0xff0000) >> 16);
			tunerData.frequencyLSB = (BYTE)((Frequency & 0x00ff00) >> 8);
			tunerData.tunerStep = (BYTE)(Frequency & 0x0000ff);
			tunerData.symbolRateHSB = 0;
			tunerData.symbolRateMSB = 0;
			tunerData.symbolRateLSB = 0xff;
			tunerData.flag = 0;

			// finally, calculate and set the check sum
			MakeCheckSum(&tunerData);
		
			inBuf = &tunerData;
			inBufLength	= sizeof( tunerData );		
			outBuf = (LPVOID)bBuf;
			outBufLength = inBufLength;

			StatusSuccess = DeviceIoControl(hVideo, 
											(DWORD)ANALOG_SET_CHANNEL, 
											inBuf,
											inBufLength + 1,
											outBuf, 
											outBufLength, 
											&bytesReturned, 
											NULL);
			if (!StatusSuccess) {
				return false;
			}

/*			ZeroMemory((PVOID)(&LockStatus),
					   sizeof(LockStatus));

			StatusSuccess = DeviceIoControl(hVideo,
											(DWORD)ANALOG_GET_STATUS, 
											(LPVOID)&LockStatus, 
											sizeof(LockStatus), 
											(LPVOID)&LockStatus,  
											sizeof(LockStatus), 
											&bytesReturned, 
											NULL);
			if (!StatusSuccess) {
				return false;
			}

			if (LockStatus.HLOCK) {
			}
			else {
				StatusSuccess = FALSE;
			}
*/
		}
	}
	else if ((Mode==11) || (Mode==14) || (Mode==15)) {
		//Digital TV Locking
		tunerData.frequencyMSB = (BYTE)((Frequency & 0xff0000) >> 16);
		tunerData.frequencyLSB = (BYTE)((Frequency & 0x00ff00) >> 8);
		tunerData.tunerStep = (BYTE)(Frequency & 0x0000ff);
		tunerData.symbolRateHSB = 0;
		tunerData.symbolRateMSB = (BYTE)SymbolRate;		//BindWidth : 6,7,8
		tunerData.symbolRateLSB = 0;
		tunerData.flag = 0;
		
		// finally, calculate and set the check sum
		MakeCheckSum(&tunerData);
		
		inBuf = &tunerData;
		inBufLength	= sizeof( tunerData );		
		outBuf = (LPVOID)bBuf;
		outBufLength = inBufLength;
		// The First step, scan start. I2c Write tuner data ...
		StatusSuccess = DeviceIoControl(hVideo, 
			(DWORD)DST_IOCTL_SCAN_START, 
			inBuf,
			inBufLength + 1,
			outBuf, 
			outBufLength, 
			&bytesReturned, 
			NULL);
		if (!StatusSuccess) {
			return false;
		}
		// The second step, waiting for Tuner Locking...
		StatusSuccess = FALSE;
		DWORD LockTime = GetTickCount();
		while (!StatusSuccess) {
			if ((GetTickCount() - LockTime) > 6000) {
				break;
			}
			
			StatusSuccess = DeviceIoControl(hVideo, 
				(DWORD)DST_IOCTL_SCAN_WAITING, 
				NULL, 
				0, 
				NULL, 
				0, 
				&bytesReturned, 
				NULL);
			Sleep(1);
		}
		if (!StatusSuccess) {
			return false;
		}
		// The third step
		StatusSuccess = DeviceIoControl(hVideo, 
			(DWORD)DST_IOCTL_SCAN_END, 
			inBuf, 
			inBufLength + 1, 
			outBuf, 
			outBufLength, 
			&bytesReturned, 
			NULL);
		if (StatusSuccess) {
		}
	}

	if (!StatusSuccess) {
		return false;
	}

	return true;
}

BOOL GetMACAddr(HANDLE hVideo, UCHAR *Addr)
{
    BOOL			StatusSuccess = TRUE;
	LPVOID			inBuf;
	LPVOID			outBuf;
	DWORD			inBufLength;
	DWORD			outBufLength;
    ULONG			bytesReturned;
    UCHAR			cmdBuf[9] = { 0xaa,0x00,0x0a,0x00,0x0,0x0,0x00,0x00, 0xf6 };

	UCHAR			*szTemp;
	int				k;
	BOOL			ret = TRUE;
	EEprom_Info		eeprom_info;
	// 
	UCHAR			Flag = FALSE, sign = TRUE;

	if ((hVideo==NULL) || (hVideo==INVALID_HANDLE_VALUE)) {
	    return 1;
	}
	
	inBuf = cmdBuf;
	inBufLength	= 9;		
	outBuf = cmdBuf;
	outBufLength = inBufLength;
	for (k=0; k<3; k++) {		
		StatusSuccess = DeviceIoControl(hVideo, 
									(DWORD)DST_IOCTL_SET_INFO, 
									inBuf, 
									inBufLength, 
									outBuf, 
									outBufLength, 
									&bytesReturned, 
									NULL);
		if (!StatusSuccess) 
		{
#if DBG_GET_MAC
			fprintf(stdout, "!!! Failed get the Mac address! Try again...\n");
#endif
			Sleep(1);
			continue;
		}
		szTemp = (PUCHAR)outBuf;
		if ((0x00 == szTemp[1]) && (0x08 == szTemp[2]) && (0xca==szTemp[3])) {
#if DBG_GET_MAC
			fprintf(stdout, "Get Mac address ok.\n");
#endif
			szTemp = (unsigned char *)outBuf;
			Addr[0] = szTemp[1];
			Addr[1] = szTemp[2];
			Addr[2] = szTemp[3];
			Addr[3] = szTemp[5];
			Addr[4] = szTemp[6];
			Addr[5] = szTemp[7];
#if DBG_GET_MAC
			fprintf(stdout, "Your Mac address is %02x-%02x-%02x-%02x-%02x-%02x\n",
					Addr[0], Addr[1], Addr[2], Addr[3], Addr[4], Addr[5]);
#endif		
			Flag = TRUE;
			break;
		}
	}

	// 
	if (Flag == FALSE) {
		for (k=0; k<3; k++) {
			eeprom_info.address = 0x50+k;
			eeprom_info.data = 0;
			eeprom_info.flag = 1;

			inBuf = &eeprom_info;
			inBufLength = sizeof(EEprom_Info);
			outBuf = &eeprom_info;
			outBufLength = sizeof(EEprom_Info);
			StatusSuccess = DeviceIoControl(hVideo, 
										(DWORD)DST_GET_EEPROM_INFO, 
										inBuf, 
										inBufLength, 
										outBuf, 
										outBufLength, 
										&bytesReturned, 
										NULL);
			if (!StatusSuccess) {
				sign = FALSE;
				break;
			}
			szTemp = ( unsigned char * )outBuf;
			Addr[3+k] = szTemp[1];
		}
		if (sign==TRUE) {
			Addr[0] = 0x08;
			Addr[0] = 0x08;
			Addr[0] = 0x08;
			Flag = TRUE;
		}
	}

	if (Flag==TRUE)
		return TRUE;
	else 
		return FALSE;
}

int main(int argc, char* argv[])
{
    HANDLE	   m_hMulVideo[HW_MAX_COUNT];
	HANDLE	   m_hMulAudio[HW_MAX_COUNT];
    int		   m_nMulMode[HW_MAX_COUNT];
    int		   m_nCurrentHW = 1;
    int		   m_nMulNum = 1;

    int i = 0;

	for (i=0;i<HW_MAX_COUNT;i++) {
		m_hMulVideo[i] = NULL;
		m_hMulAudio[i] = NULL;
	}

    m_hMulVideo[0] = OpenVideoDriver();
	if (m_hMulVideo[0] == INVALID_HANDLE_VALUE) {
		m_hMulVideo[0] = NULL;
	}

	m_hMulAudio[0] = OpenAudioDriver();
	if (m_hMulAudio[0] == INVALID_HANDLE_VALUE) {
		m_hMulAudio[0] = NULL;
	}

	if (m_hMulVideo[0] && m_hMulAudio[0]) {
		int nReturn = 0;

		GetMaxCard(m_hMulVideo[0], &nReturn);

		if (nReturn) {
			CloseHandle(m_hMulVideo[0]);
			CloseHandle(m_hMulAudio[0]);
			m_hMulVideo[0] = NULL;
			m_hMulAudio[0] = NULL;
		}
	}

	if ((m_hMulVideo[0] == NULL) && 
		(m_hMulAudio[0] == NULL)) {
		// open the first video driver
		m_hMulVideo[m_nCurrentHW - 1] = OpenMulVideoDriver(m_nCurrentHW - 1);
		if (m_hMulVideo[m_nCurrentHW - 1] == INVALID_HANDLE_VALUE) {
			m_hMulVideo[m_nCurrentHW - 1] = NULL;
		}

		m_nMulNum = GetMaxCard(m_hMulVideo[m_nCurrentHW - 1], NULL);

		if (m_nMulNum > HW_MAX_COUNT)
			m_nMulNum = HW_MAX_COUNT;

		if (m_hMulVideo[m_nCurrentHW - 1])
			CloseHandle(m_hMulVideo[m_nCurrentHW - 1]);

		for (i=0;i<m_nMulNum;i++) {
			m_hMulVideo[i] = OpenMulVideoDriver(i);
			if (m_hMulVideo[i] == INVALID_HANDLE_VALUE) {
				m_hMulVideo[i] = NULL;
			}

			m_hMulAudio[i] = OpenMulAudioDriver(i);
			if (m_hMulAudio[i] == INVALID_HANDLE_VALUE) {
				m_hMulAudio[i] = NULL;
			}
		}
	}

    if (!m_hMulVideo[0]) {
        cout <<"Can not find card."<<endl;
        return 0;
    }

    int Count = 1000;

    BOOL bExit = FALSE ;
    char str[256];
	int iMenuSelect = 0;
    int I2CSuccess = 0;
    int I2CFail = 0;
    UCHAR MacFilterAddr[6];
    char buffer[32];

	while(bExit == FALSE)
	{
		for(int i = 0 ; i < 4; i ++)
		{
			cout << endl;
		}

        cout << "============================================" <<endl;
        cout << "=          I2C Test Tool V1.1.0.0          =" <<endl;
        cout << "=            Twinhan Technology            =" <<endl;
        cout << "=                2004/12/13                =" <<endl;
        cout << "============================================" <<endl;

        cout << "============================================" <<endl;
        getCIInfo(m_hMulVideo[0], 1);
        // Get MAC address
        MacFilterAddr[0] = L'\0';
        GetMACAddr(m_hMulVideo[0], MacFilterAddr);

        sprintf(buffer, "%02X-%02X-%02X-%02X-%02X-%02X", MacFilterAddr[0], MacFilterAddr[1], MacFilterAddr[2], MacFilterAddr[3], MacFilterAddr[4], MacFilterAddr[5]);
        cout << "MAC Address: ";
        cout << buffer << endl;
        cout << "============================================" <<endl;

		cout << "1. Start I2C test1                 " << endl;
        cout << "2. Start I2C test2 (Satellite only)" << endl;
		cout <<	"3. Change loop counts              " << endl;	
		cout << "4. Version Code"					  << endl;
		cout << "5. Reset test"						  << endl;
		cout << "6. Test ""DCT-CI"" channel lock"	  << endl;
		cout << "7. Test ""DCTNEW"" channel lock"	  << endl;
		cout << "8. Test Get lock status"	          << endl;
		cout << "9. Exit"		                      << endl;
		cout << "Please Select Menu(1-9):";
		
		StrEmpty(str);
		cin >> str;
		if(!IsNumberStr(str)) 
		{
			cout << "Error selecting,Please Select again" << endl;  
			continue;
		}
		iMenuSelect = atoi(str);
		
		if(iMenuSelect < 0 || iMenuSelect > 9)
		{
			cout << "Error selecting,Please Select again" << endl;
			continue;
		}
		switch(iMenuSelect)
		{
			case 1:
            {
                I2CSuccess = 0;
                I2CFail = 0;

                cout << "Start testing ..." << endl;

				for (i=0; i<Count; i++) {
                    m_nMulMode[0] = 0;
		            m_nMulMode[0] = getCIInfo(m_hMulVideo[0], 0);

                    //cout << "m_nMulMode[0] = " << m_nMulMode[0] << endl;
                    
                    if (m_nMulMode[0]) {
                        if (strcmp(ucDevName, ucDevNameTest) == 0)
                            I2CSuccess++;
                        else
                            I2CFail++;
                    }
                    else
                        I2CFail++;
	            }

                cout << endl;
                cout << "loop counts: " << Count << endl;
                cout << "I2C success times: " << I2CSuccess << endl;
                cout << "I2C fail times: " << I2CFail << endl;

				break;
            }
            case 2:
            {
                I2CSuccess = 0;
                I2CFail = 0;

                m_nMulMode[0] = 0;
		        m_nMulMode[0] = getCIInfo(m_hMulVideo[0], 0);
                if (m_nMulMode[0] > 5) {
                    cout << "This card have not support this test!" << endl;
                    continue;
                }

                cout << "Start testing ..." << endl;

				for (i=0; i<Count; i++) {
                    m_nMulMode[0] = 0;
		            m_nMulMode[0] = setTone(FALSE, TRUE, m_hMulVideo[0]);

                    //cout << "m_nMulMode[0] = " << m_nMulMode[0] << endl;
                    
                    if (m_nMulMode[0])
                        I2CSuccess++;
                    else
                        I2CFail++;
	            }

                cout << endl;
                cout << "loop counts: " << Count << endl;
                cout << "I2C success times: " << I2CSuccess << endl;
                cout << "I2C fail times: " << I2CFail << endl;

				break;
            }
            case 3:
            {
                cout << "Please enter loop counts: ";
		        cin >> Count;

				break;
            }
            case 4:
			{
				getVerInfo(m_hMulVideo[0]);
				break;		
			}
            case 5:
			{			
				TestReset(m_hMulVideo[0]);
				break;		
			}
			case 6:
			case 7:
			{
				long Freq = 0;
				long SR = 0;
				int  HV = 0;
				int  mode = 0;
                BOOL bFlag = FALSE;

				if (iMenuSelect == 6)
					mode = 6; // "DCT-CI"
				else
					mode = 5; // "DCTNEW"

                //mode = 0;
		        //mode = getCIInfo(m_hMulVideo[0]);

				cout << "Frequency(KHz): ";
				cin >> Freq;

				cout <<"SymbolRate(Ksps): ";
				cin >> SR;

				cout <<"QAMSize: ";
				cin >> HV;

				bFlag = lockChannel(Freq,
							SR,
							HV,
							0,
							0,
							0,
							0,
							0,
							mode,
							0,
							m_hMulVideo[0]);

                if (bFlag)
                    cout << endl << "Lock Success!!" << endl;
                else
                    cout << endl << "Lock Fail!!" << endl;

				break;		
			}

			case 8:
			{			
				GetLockStatus(m_hMulVideo[0]);
				break;		
			}
			case 9:			{			
				bExit = TRUE;
				break;		
			}


		}
	}

	return 0;
}
