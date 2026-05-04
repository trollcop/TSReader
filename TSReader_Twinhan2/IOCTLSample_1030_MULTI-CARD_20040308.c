//
// Twinhan DTV driver IOControl call sample (2004/03/08)
//
// Description: 1. Fill TunerData fields to setup transponder
//              2. Get Card handle (For One-Card Only driver)
//              3. Get Multi-Card handles (For Multi-Card support driver)
//              4. Lock channel
//              5. Sart stream capture
//              6. Stop stream capture
//              7. Get locking flag and Quality/Level values
//              8. Get stream buffer position
//              9. Get stream buffer start address
//              10. SetDiSEqC() command
//              11. 22kHz-tone controlling (on/off)
//              12. CI interfance
//              13. Get MAC address


////////////////////////////////////////////////////////////////////////////////////////
// The IOControl call driver handle GUID.
// hDevice1:
DEFINE_GUID(GUID_DST_DEVICE1, 
            0xe18d1721, 0x70b5, 0x4de7, 0x83, 0xb3, 0x35, 0x43, 0x6e, 0x39, 0x54, 0xed);

// hDevice2:
DEFINE_GUID(GUID_DST_DEVICE2, 
            0x7377db61, 0xe3cd, 0x11d2, 0x8a, 0x3f, 0x0, 0x0, 0xf8, 0x78, 0x44, 0x22);

////////////////////////////////////////////////////////////////////////////////////////
// Stream buffer length:
     BufferLength = 3 * 4092 * 255
///////////////////////////////////////////////////////////////////////////////////////


// Tuner data define
typedef struct TunerDataStruct
{
	unsigned char	address;         // 0
	unsigned char	frequencyMSB;    // 1
	unsigned char	frequencyLSB;    // 2
	unsigned char	tunerStep;       // 3
	unsigned char	symbolRateHSB;   // 4
	unsigned char	symbolRateMSB;   // 5
	unsigned char	symbolRateLSB;   // 6
	unsigned char	flag;            // 7
	unsigned char	checkSum;        // 8
}TunerData;

////////////////////////////////////////////////////////////////////////////////////////
#define DST_DEVICE1 0xDDFF
#define DST_DEVICE2 0xAABB
////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////
// Stream buffer length:
//     BufferLength = 3 * 4092 * 255

////////////////////////////////////////////////////////////////////////////////////////
// Get Gard Type
#define DST_IOCTL_SET_INFO \
	CTL_CODE( DST_VIDEO, 0xA20, METHOD_BUFFERED, FILE_ANY_ACCESS )

int getCIInfo(HANDLE hDevice2)
{
    BOOL			StatusSuccess = TRUE;
	LPVOID			inBuf;
	LPVOID			outBuf; 
	DWORD			inBufLength;
	DWORD			outBufLength;
    ULONG			bytesReturned;
	BYTE			bBuf[256];

	if (hDevice2 == NULL) {
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
	
	StatusSuccess = DeviceIoControl(hDevice2, 
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
	//10: DTT Card(Digtal & Analog)
	//11: DTT Card(Digtal)
	char *chName = (char *)((BYTE *)outBuf + 2);
	chName[6] = 0;
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

	return 0;
}

m_Mode = getCIInfo(hDevice2);
////////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////////
// 2. Get Card handle (For One-Card Only driver)
// obtain the handle of the two Twinhan devices (GUID_DST_DEVICE1 and GUID_DST_DEVICE2)
//
    char szErrMsg[128];
 
    HANDLE hDevice1 = GetDeviceHandle(GUID_DST_DEVICE1, NULL, szErrMsg); 
    HANDLE hDevice2 = GetDeviceHandle(GUID_DST_DEVICE2, NULL, szErrMsg); 
 
 
    HANDLE GetDeviceHandle(
                        GUID ClassGuid,
                        PSP_INTERFACE_DEVICE_DETAIL_DATA functionClassDeviceData,
                        char* szErrMsg)
    {
        HDEVINFO                 hardwareDeviceInfo;
        SP_INTERFACE_DEVICE_DATA deviceInfoData;
        DWORD                    predictedLength = 0;
        DWORD                    requiredLength = 0;
        HANDLE                   hDev = INVALID_HANDLE_VALUE;
 
        //
        // Open a handle to the plug and play dev node.
        //
        hardwareDeviceInfo = SetupDiGetClassDevs((LPGUID)&ClassGuid,
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
               free(functionClassDeviceData);
               return (hDev);
           }
       
        }
        else if (ERROR_NO_MORE_ITEMS != GetLastError()) {
            free(functionClassDeviceData);
            SetupDiDestroyDeviceInfoList(hardwareDeviceInfo);
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
                          NULL);
        if (hDev == INVALID_HANDLE_VALUE) {
             free(functionClassDeviceData);
             return (hDev);
        }
    
        return (hDev);
    }
////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////
// 3. Get Multi-Card handles (For Multi-Card support driver)
#define		HW_MAX_COUNT			3
HANDLE	   m_hMulVideo[HW_MAX_COUNT];
HANDLE	   m_hMulAudio[HW_MAX_COUNT];

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

int	GetMaxCard(HANDLE hDevice2, int *DriverType)
{
	BOOL			StatusSuccess = FALSE;
	PVOID			cBuffer = NULL;
	DWORD			bytesReturned;
	PCIConfig2		pc;

	cBuffer = &pc;
	StatusSuccess = DeviceIoControl(hDevice2,
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

// open the first video driver
m_hMulVideo[0] = OpenMulVideoDriver(0);

// Get Max Card number
m_nMulNum = GetMaxCard(m_hMulVideo[0], NULL);

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

//0 : Old DST Card					--- 188bytes;
//1 : New DST Card(USALS)			--- 188bytes;
//2 : New DST Card(USALS, 3 Tuners) --- 204bytes;
//3 : DST CI Card					--- 204bytes;
//4 : DST CI Card(188 Bytes)		--- 188bytes;
//5 : DCT Card						--- 188bytes;
//6 : DCT CI Card					--- 204bytes;
//10: DTT Card(Digtal & Analog)
//11: DTT Card(Digtal)

for (i=0;i<m_nMulNum;i++) {
	int loop = 0;

	while (loop < 3) {
		m_nMulMode[i] = getCIInfo(m_hMulVideo[i]);
		if (m_nMulMode[i])
			break;
		loop++;
	};

	switch(m_nMulMode[i]) {
	    case 2:
	    	m_MulType[i] = HW_DST_FTA_CARD;
	    	break;
	    case 4:
	    	m_MulType[i] = HW_DST_CI_CARD;
	    	break;
	    case 6:
	    	m_MulType[i] = HW_DCT_CI_CARD;
	    	break;
	    case 10:
	    	m_MulType[i] = HW_DTT_AD_CARD;
	    	break;
	    case 11:
	    	m_MulType[i] = HW_DTT_FTA_CARD;
	    	break;
	    default:
	    	m_MulType[i] = HW_ERROR;
	    	break;
	}
}

m_nCurrentHW = 0; // First Card
//m_nCurrentHW = 1; // Second Card
//m_nCurrentHW = 2; // Third Card

m_hVideo = m_hMulVideo[m_nCurrentHW];
m_hAudio = m_hMulAudio[m_nCurrentHW];
m_Mode = m_nMulMode[m_nCurrentHW];

////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////
// 1. Fill TunerData fields to setup transponder

    // LOF select: according the LNB
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
	tunerData.frequencyMSB = (InNumber & 0x00007f00) >> 8;
	tunerData.frequencyLSB = InNumber & 0x000000ff;
	divRate = 4000000L / tunerStep;


/*****************************************************************************
Symbol Rate calculation:
symbolRateHSB¡GThe first byte of symbol rate;
symbolRateMSB¡GThe second byte of symbol rate;
symbolRateLSB¡G The third byte of symbol rate;

Symbol rate¡GSymbol Rate=Fm-clk/220*TEMP_M ;    unit (K sps);
             Fm-clk: Master clock (88M Hz); 

TEMP_M: 20 bit binary value,

symbolRateHSB= (TEMP_M>>12)
symbolRateMSB=(TEMP_M>>4)&0xff
symbolRateLSB=(TEMP_M&0xf)<<4

For example¡G
        Symbol rate : 28125 K sps; 
        Master Clock : 88M Hz;
       28125000/(88000000 / 220 ) = (335127) Dec  = 0x51017;

      So,  symbolRateHSB, symbolRateMSB, symbolRateLSB=0x51, 0x01, 0x70
*************************************************************************/
        TEMP_M=Symbol_Rate/(88000000 / 220 );
        tunerData.symbolRateHSB= (TEMP_M>>12);
        tunerData.symbolRateMSB=(TEMP_M>>4)&0xff;
        tunerData.symbolRateLSB=(TEMP_M&0xf)<<4;

	
	for (i=0;i<16;i++) {
		if (frequencyRef[i] == divRate)
			break;
	}
	
	if (i > 15)
		i = 15;
	
	tunerData.tunerStep = i;

      //H_V: 1 for Horizotal, 0 for Vertical
	tunerData.flag = 0;
	
	/*h/v chosen*/
	if (H_V)
		tunerData.flag |= 0x0040;

	MakeCheckSum(pTunerData);
////////////////////////////////////////////////////////////////////////////////////////




////////////////////////////////////////////////////////////////////////////////////////
// 4. Lock channel
// Start lock
#define DST_IOCTL_SCAN_START \
	CTL_CODE( DST_DEVICE2, 0xA40, METHOD_BUFFERED, FILE_ANY_ACCESS )

// Wait lock
#define DST_IOCTL_SCAN_WAITING \
	CTL_CODE( DST_DEVICE2, 0xA60, METHOD_BUFFERED, FILE_ANY_ACCESS )

// Stop lock
#define DST_IOCTL_SCAN_END \
	CTL_CODE( DST_DEVICE2, 0xA80, METHOD_BUFFERED, FILE_ANY_ACCESS )

// CI
#define DST_IOCTL_CA_WRITE \
	CTL_CODE( DST_VIDEO, 0xA81, METHOD_BUFFERED, FILE_ANY_ACCESS )

#define DST_IOCTL_CA_READ \
	CTL_CODE( DST_VIDEO, 0xA82, METHOD_BUFFERED, FILE_ANY_ACCESS )

// Reset RDC8820
#define DST_IOCTL_RDC8820_RESET \
	CTL_CODE( DST_VIDEO, 0xA84, METHOD_BUFFERED, FILE_ANY_ACCESS )


void resetHardware(HANDLE hDevice2)
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


    BYTE			bBuf[256];

	TunerData tunerData;
	
	inBufLength	= sizeof(tunerData);		
	outBuf = (LPVOID)bBuf;
	outBufLength = inBufLength;
    
    
    if ((m_Mode == 0) || (m_Mode == 1) || (m_Mode == 2)) {
        inBuf = &tunerData;
        inBufLength = sizeof(tunerData);
        
        StatusSuccess = DeviceIoControl(hDevice2, 
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
        
        StatusSuccess = FALSE;
        DWORD LockTime = GetTickCount();
        while (!StatusSuccess) {
            if ((GetTickCount() - LockTime) > 6000) {
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
        if (!StatusSuccess) {
            return false;
        }
        
        StatusSuccess = DeviceIoControl(hDevice2, 
                                        (DWORD)DST_IOCTL_SCAN_END, 
                                        inBuf, 
                                        inBufLength + 1, 
                                        outBuf, 
                                        outBufLength, 
                                        &bytesReturned, 
                                        NULL);
    }
    else if ((m_Mode == 3) || (m_Mode == 4)) {
    
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

			StatusSuccess = DeviceIoControl(hDevice2, 
											(DWORD)DST_IOCTL_CA_WRITE, 
											inBuf,
											inBufLength,
											NULL, 
											0, 
											&bytesReturned, 
											NULL);
			if (!StatusSuccess) {
				resetHardware(hDevice2);
				Sleep(600);
				return false;
			}

			StatusSuccess = DeviceIoControl(hDevice2, 
											(DWORD)DST_IOCTL_CA_READ, 
											NULL,
											0,
											inBuf, 
											inBufLength, 
											&bytesReturned, 
											NULL);
			if (!StatusSuccess) {
				resetHardware(hDevice2);
				Sleep(600);
				return false;
			}

			if ((InBuf[1] == tunerData.frequencyMSB) && (InBuf[2] == tunerData.frequencyLSB)) {
			}
			else {
				StatusSuccess = FALSE;
			}
    }
    else if (m_Mode == 5) {
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
		StatusSuccess = DeviceIoControl(hDevice2, 
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
		if (!StatusSuccess) {
			return false;
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
		if (StatusSuccess) {
		}
	}
	else if (m_Mode == 6) {
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
		
		StatusSuccess = DeviceIoControl(hDevice2, 
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
		
		StatusSuccess = DeviceIoControl(hDevice2, 
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
	else if (m_Mode == 10) {
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
			StatusSuccess = DeviceIoControl(hDevice2, 
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
			if (!StatusSuccess) {
				return false;
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

			StatusSuccess = DeviceIoControl(hDevice2, 
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
		}
	}
	else if (m_Mode == 11) {
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
		StatusSuccess = DeviceIoControl(hDevice2, 
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
		if (!StatusSuccess) {
			return false;
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
		if (StatusSuccess) {
		}
	}

////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////
// 5. Sart stream capture
#define DST_IOCTL_START_CAP \
	CTL_CODE( DST_DEVICE1, 0xB00, METHOD_BUFFERED, FILE_ANY_ACCESS )

    DeviceIoControl(hDevice1,
                    (DWORD)DST_IOCTL_START_CAP, 
                    NULL, 
                    0, 
                    &outBuf, 
                    sizeof(DWORD), 
                    &bytesReturned, 
                    NULL);
////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////
// 6. Stop stream capture
#define DST_IOCTL_STOP_CAP \
	CTL_CODE( DST_DEVICE1, 0xB10, METHOD_BUFFERED, FILE_ANY_ACCESS )

    DeviceIoControl(hDevice1,
                    (DWORD)DST_IOCTL_STOP_CAP, 
                    NULL, 
                    0, 
                    NULL, 
                    0, 
                    &bytesReturned, 
                    NULL);
////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////
// 7. Get locking flag and Quality/Level values
#define DST_IOCTL_SET_INFO \
	CTL_CODE( DST_DEVICE2, 0xA20, METHOD_BUFFERED, FILE_ANY_ACCESS )

    TunerData tunerData;
    
    inBuf = &tunerData;
    inBufLength	= sizeof(tunerData);
    
    tunerData.address = 0xaa;
	tunerData.frequencyMSB = 0;
	tunerData.frequencyLSB = 0x05;

	MakeCheckSum(pTunerData);
    
    StatusSuccess = DeviceIoControl(hDevice2, 
                                    (DWORD)DST_IOCTL_SET_INFO, 
                                    inBuf, // command
                                    inBufLength + 1, 
                                    outBuf, // return signal state
                                    outBufLength, 
                                    &bytesReturned, 
                                    NULL);

    BYTE  *pbuf = (BYTE *)outBuf;
	DWORD Temp = 0;

	Temp = pbuf[7];
	if (Temp & 0x10) {
		*LockFlag = false;
	}
	else {
		*LockFlag = true;
	}

	switch (m_Mode) {
	    case 0:
	    {
	    	Temp = pbuf[3];
	    	Temp = (Temp << 8) | pbuf[4];
	    	if ((Temp <= 0x37fa) && (Temp >= 0)) {
	    		double x = (double)Temp / (double)(0x37fa / 99);
	    		*Qu = 100 - (int)x;
	    	}
	    	else {
	    		*Qu = 0;
	    	}
        
	    	Temp = pbuf[6];
	    	if ((Temp <= 0x40) && (Temp >= 0)) {
	    		double x = (double)Temp / ((double)0x40 / (double)99);
	    		*Str = 100 - (int)x;
	    	}
	    	else {
	    		*Str = 0;
	    	}
	    }
	    break;
	    
	    case 1:
	    case 2:
	    case 3:
	    case 4:
	    {
	    	Temp = pbuf[3];
	    	Temp = (Temp << 8) | pbuf[4];
	    	if (*LockFlag) {
	    		if ((Temp >= 0) && (Temp <= 0x1200)) {
	    			*Qu = 98;
	    		}
	    		else if ((Temp > 0x1200) && (Temp <= 0x1900)) {
	    			double x = (double)(Temp - 0x1201) / (double)((0x1900 - 0x1201) / 57);
	    			*Qu = 40 + (58 - (int)x);
	    		}
	    		else if ((Temp > 0x1900) && (Temp <= 0x3500)) {
	    			double x = (double)(Temp - 0x1901) / (double)((0x3500 - 0x1901) / 38);
	    			*Qu = 39 - (int)x;
	    		}
	    		else {
	    			*Qu = 0;
	    		}
	    	}
	    	else {
	    		*Qu = 0;
	    	}
        
	    	Temp = pbuf[6];
	    	if (*LockFlag) {
	    		*Str = Temp;
	    	}
	    	else {
	    		if (Temp > 80) {
	    			*Str = 80;
	    		}
	    		else {
	    			*Str = Temp;
	    		}
	    	}
	    }
	    break;
	    
	    case 5:
	    // return data: 0x00,0x05,ErrorCounter_h,ErrorCounter_l,Strength_h,Strength_l,noise,Checksum;
	    {
	    	Temp = pbuf[5];
	    	if ((Temp & 0x04) == 0)
	    		*Str = 8;
	    	else {
	    		Temp = pbuf[6] + ((Temp & 0x03) << 8);
	    		double x = 100 * Temp;
	    		x = x / 1024;
	    		if ((int)x >= 90)
	    			*Str = 10;
	    		else if ((int)x <= 15)
	    			*Str = 90;
	    		else
	    			*Str = 100 - (int)x;
	    	}
        
	    	Temp = pbuf[5];
	    	if ((Temp & 0x80) == 0x80) {
	    		Temp = pbuf[3];
	    		Temp = pbuf[4] + (Temp << 8);
	    		if (Temp == 0) {
	    			if (pbuf[7] == 0)
	    				*Qu = 98;
	    			else
	    				*Qu = 90;
	    		}
	    		else if (Temp < 0xf0) {
	    			double x = 40 + ((0x00f0 - Temp) * 20) / 0xf0;
	    			*Qu = (int)x;
	    		}
	    		else if (Temp < 0xf00) {
	    			double x = 30 + ((0x0f00 - Temp) * 10) / 0xf00;
	    			*Qu = (int)x;
	    		}
	    		else {
	    			double x = 15 + ((0xffff - Temp) * 15) / 0xffff;
	    			*Qu = (int)x;
	    		}
	    	}
	    	else {
	    		*Qu = 0;
	    	}
	    }
	    break;
	    
	    case 6:
	    {
	    	Temp = pbuf[4];
	    	*Qu = Temp;
	    	Temp = pbuf[6];
	    	*Str = Temp;
	    }
	    break;
	    
	    case 10:
	    case 11:
            // data1---------0x00
            // data2---------0x00
            // data3---------Quality(H)
            // data4---------Quality(L)
            // data5---------Strength(H)
            // data6---------Strength(L)
		    Temp = pbuf[2];
		    if (Temp) {
		    	*LockFlag = true;
		    }
		    else {
		    	*LockFlag = false;
		    }
            
		    Temp = pbuf[4];
		    *Qu = Temp;
		    Temp = pbuf[5];
		    //Temp = pbuf[6];
		    *Str = Temp;
		    break;
	}
	
////////////////////////////////////////////////////////////////////////////////////////
/*
Twinhan's DST driver allocates a 3MB(BufferLength = 3 * 4092 * 255) Ring buffer for 
captured TS data in HOST memory space.  
DMA controller will fill data to this Ring buffer when  DST_IOCTL_START_CAP is called.

DST_IOCTL_GET_DATABUFF_ADDR returns the starting address of this DMA buffer. This address
 is fixed after driver is initialized.

DST_IOCTL_GET_DATABUFF_BLOCK returns the current offset which DMA controller will
fill captured data to.
Initially, it's 0. When a channel lock command is issued and it locks successfully, DMA engine
starts to capture TS data into host's DMA buffer and this offset will increase.
It will reset(wrap) to 0 when 3 * 4092 * 255 limit is reached.
*/

////////////////////////////////////////////////////////////////////////////////////////
// 8. GetWritePos(): Get stream buffer position
#define DST_IOCTL_GET_DATABUFF_BLOCK \
	CTL_CODE( DST_DEVICE1, 0xB20, METHOD_BUFFERED, FILE_ANY_ACCESS )

    DeviceIoControl(hDevice1, 
                    (DWORD)DST_IOCTL_GET_DATABUFF_BLOCK, 
                    NULL, 
                    0, 
                    outBuf, 
                    outBufLength, 
                    &bytesReturned, 
                    NULL);
////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////
// 9. GetDeviceBuffer(): Get stream buffer start address
#define DST_IOCTL_GET_DATABUFF_ADDR \
	CTL_CODE( DST_DEVICE1, 0xB40, METHOD_BUFFERED, FILE_ANY_ACCESS )

    DeviceIoControl(hDevice1, 
                    (DWORD)DST_IOCTL_GET_DATABUFF_ADDR, 
                    NULL, 
                    0, 
                    outBuf, 
                    outBufLength, 
                    &bytesReturned, 
                    NULL);
////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////
// 10. SetDiSEqC() command
#define DST_IOCTL_SET_INFO \
	CTL_CODE( DST_DEVICE2, 0xA20, METHOD_BUFFERED, FILE_ANY_ACCESS )

    TunerData tunerData;
    
    inBuf = &tunerData;
    inBufLength	= sizeof(tunerData);
    
    //The TnerData structure to send DiSEqC commands:
    //a. 3 bytes command:
    tunerData.tunerStep = 0x03;
    tunerData.symbolRateHSB = Frame;
    tunerData.symbolRateMSB = Address;
    tunerData.symbolRateLSB = Command;
    tunerData.flag = 0;
 
    //b. 4 bytes command:
    tunerData.tunerStep = 0x04;
    tunerData.symbolRateHSB = Frame;
    tunerData.symbolRateMSB = Address;
    tunerData.symbolRateLSB = Command;
    tunerData.flag = Data1;

    //c. 5 bytes command:
    tunerData.tunerStep = Frame;
    tunerData.symbolRateHSB = Address;
    tunerData.symbolRateMSB = Command;
    tunerData.symbolRateLSB = Data1;
    tunerData.flag = Data2;

    
    StatusSuccess = DeviceIoControl(hDevice2, 
                                    (DWORD)DST_IOCTL_SET_INFO, 
                                    inBuf, // command
                                    inBufLength + 1, 
                                    outBuf,
                                    outBufLength, 
                                    &bytesReturned, 
                                    NULL);
////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////
// 11. 22kHz-tone controlling (on/off)
// Stream buffer length:
// 22kHz-tone controlling (on/off)
// LNB Power controlling (on/off)

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

    tunerData.address = 0xaa;
	tunerData.frequencyMSB = 0;
	tunerData.frequencyLSB = 0x09;

	if (22KEnable)
		tunerData.tunerStep = 0x02;
	else
		tunerData.tunerStep = 0;

	tunerData.symbolRateHSB = 0xff;

       tunerData.symbolRateMSB = 0x01; // 0x00: LNB power off, 0x01: LNB power on

	MakeCheckSum(pTunerData);

    inBuf = &tunerData;
	inBufLength	= sizeof(tunerData);		
	outBuf = (LPVOID)bBuf;
	outBufLength = inBufLength;

    DeviceIoControl(hDevice2, 
                    (DWORD)DST_IOCTL_SET_INFO, 
                    inBuf, 
                    inBufLength + 1, 
                    outBuf, 
                    outBufLength, 
                    &bytesReturned, 
                    NULL);

////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////
// 12. CI interfance

#define ME0						1
#define ME1						2
#define MMI0					3
#define MMI1					4
#define MMI0_ClOSE				5
#define MMI1_CLOSE				6
#define NON_CI_INFO				0

#define PCMSG_NULL              0x00
#define PCMSG_APPLICATION_INFO  0x01
#define PCMSG_CA_INFO           0x02
#define PCMSG_CA_PMT            0x03
#define PCMSG_CA_PMT_REPLY      0x04
#define PCMSG_DATETIME_ENQ      0x05
#define PCMSG_DATETIME          0x06
#define PCMSG_ENQ               0x07
#define PCMSG_ANSWER            0x08
#define PCMSG_ENTER_MENU        0x09
#define PCMSG_MENU           	0x0A
#define PCMSG_MENU_ANSWER       0x0B
#define PCMSG_LIST              0x0C
#define PCMSG_GET_MMI           0x0D
#define PCMSG_CLOSE_MMI			0x0e


BOOL getCAMState(
    int    *CAM_Flag,
    int    *MMI_Flag,
    HANDLE hDevice2)
{
    BOOL   StatusSuccess = TRUE;
    LPVOID inBuf;
    LPVOID outBuf; 
    DWORD  inBufLength;
    DWORD  outBufLength;
    ULONG  bytesReturned;
    BYTE   bBuf[256];
    
    if (hDevice2 == NULL) {
        return false;
    }
    
    TunerData tunerData;
    memset(&tunerData, 0, sizeof(TunerData));
    
    tunerData.address = 0xaa;
    tunerData.frequencyMSB = 0;
    tunerData.frequencyLSB = 0x05;
    
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
    if (!StatusSuccess) {
        return false;
    }
    
    BYTE  *pbuf = (BYTE *)outBuf;
    DWORD Temp = 0;
    
    Temp = pbuf[5];
    if (Temp == 0xff) {
        *CAM_Flag = NON_CI_INFO;
        *MMI_Flag = NON_CI_INFO;
        return true;
    }
    
    if (Temp & 0x80)
        *CAM_Flag = ME0;
    else if (Temp & 0x40)
        *CAM_Flag = ME1;
    else
        *CAM_Flag = NON_CI_INFO;
    
    if (Temp & 0x20)
        *MMI_Flag = MMI0;
    else if (Temp & 0x10)
        *MMI_Flag = MMI1;
    else if (Temp & 0x08)
        *MMI_Flag = MMI0_ClOSE;
    else if (Temp & 0x04)
        *MMI_Flag = MMI1_CLOSE;
    else
        *MMI_Flag = NON_CI_INFO;
    
    return true;
}

#define	CAM_DEFAULT				0
#define	CAM_CONAX				1
#define	CAM_CRYPTOWORKS			2
#define CAM_ASTON				3

int CAM_Type = CAM_DEFAULT;

BOOL getAppInfo(
    App_Info *pAppInfo,
    HANDLE hDevice2)
{
    BOOL   StatusSuccess = TRUE;
    //LPVOID			inBuf;
    LPVOID outBuf; 
    DWORD  inBufLength = 9;
    DWORD  outBufLength = 256;
    ULONG  bytesReturned;
    BYTE   bBuf[256];
    char   *Str = NULL;
    
    if (hDevice2 == NULL) {
        return false;
    }
    
    BYTE Command[] = {0xaa, 0x07, 0x40, 0x00, 0x00, PCMSG_APPLICATION_INFO, 0x00, 0x00, 0xff};
    
    outBuf = (PVOID)bBuf;
    ZeroMemory((PVOID)bBuf, 256);
    MakeCheckSum2(Command);
    
    StatusSuccess = DeviceIoControl(hDevice2, 
                                    (DWORD)DST_IOCTL_CA_WRITE, 
                                    (PVOID)Command, 
                                    inBufLength, 
                                    NULL, 
                                    0, 
                                    &bytesReturned, 
                                    NULL);
    if (!StatusSuccess) {
        return false;
    }
    	
    Sleep(100);
    
    StatusSuccess = DeviceIoControl(hDevice2, 
                                    (DWORD)DST_IOCTL_CA_READ, 
                                    NULL,
                                    0,
                                    (PVOID)outBuf, 
                                    outBufLength, 
                                    &bytesReturned, 
                                    NULL);
    if (!StatusSuccess) {
        return false;
    }
    
    pAppInfo->app_type = bBuf[6];
    pAppInfo->application_manufacture = (bBuf[7] << 8) | bBuf[8];
    pAppInfo->manufacture_code = (bBuf[9] << 8) | bBuf[10];
    strcpy((char *)pAppInfo->application_info, (char *)(&(bBuf[11])));
    
    Str = strlwr(strdup(pAppInfo->application_info));

	if (NULL != strstr(Str,"conax")) {
		CAM_Type = CAM_CONAX;
	}

	if (NULL != strstr(Str,"cryptoworks")) {
		CAM_Type = CAM_CRYPTOWORKS;
	}

	if (NULL != strstr(Str,"aston")) {
		CAM_Type = CAM_ASTON;
	}
    
    return true;
}

BOOL initMMI(HANDLE hDevice2)
{
    BOOL  StatusSuccess = TRUE;
    //LPVOID			inBuf;
    DWORD inBufLength = 9;
    ULONG bytesReturned;
    
    if (hDevice2 == NULL) {
        return false;
    }
    
    BYTE EnterMenu[] = {0xaa, 0x07, 0x40, 0x00, 0x00, PCMSG_ENTER_MENU, 0x00, 0x00, 0xff};
    
    MakeCheckSum2(EnterMenu);
    
    StatusSuccess = DeviceIoControl(hDevice2, 
                                    (DWORD)DST_IOCTL_CA_WRITE, 
                                    (PVOID)EnterMenu, 
                                    inBufLength, 
                                    NULL, 
                                    0, 
                                    &bytesReturned, 
                                    NULL);
    if (!StatusSuccess) {
        return false;
    }
    
    return true;
}


BOOL getMMI(
    MMI_Info *pMMIInfo,
    int *pType,
    HANDLE hDevice2)
{
    BOOL   StatusSuccess = TRUE;
    //LPVOID inBuf;
    LPVOID outBuf; 
    DWORD  inBufLength = 9;
    DWORD  outBufLength = 256;
    ULONG  bytesReturned;
    BYTE   bBuf[256];
    
    if (hDevice2 == NULL) {
        return false;
    }
    
    BYTE GetMMI[] = {0xaa, 0x07, 0x40, 0x00, 0x00, PCMSG_GET_MMI, 0x00, 0x00, 0xff};
    
    outBuf = (PVOID)bBuf;
    ZeroMemory((PVOID)bBuf,
    		   256);
    MakeCheckSum2(GetMMI);
    
    StatusSuccess = DeviceIoControl(hDevice2, 
                                    (DWORD)DST_IOCTL_CA_WRITE, 
                                    (PVOID)GetMMI, 
                                    inBufLength, 
                                    NULL, 
                                    0, 
                                    &bytesReturned, 
                                    NULL);
    if (!StatusSuccess) {
        return false;
    }
    
    Sleep(100);
    
    StatusSuccess = DeviceIoControl(hDevice2, 
                                    (DWORD)DST_IOCTL_CA_READ, 
                                    NULL,
                                    0,
                                    (PVOID)outBuf, 
                                    outBufLength, 
                                    &bytesReturned, 
                                    NULL);
    if (!StatusSuccess) {
        return false;
    }
    
    int i = 0;
    BOOL noItemFlag = false;
    BYTE *pBuf = NULL;
    pBuf = bBuf + 6;
    switch (bBuf[3]) {
        case PCMSG_MENU:
            *pType = 1;
            
            pMMIInfo->ItemCount = *pBuf++;
            sprintf((char *)pMMIInfo->Header,"%s", (char *)pBuf);
            pBuf += strlen((char *)pBuf) + 1;
            sprintf((char *)pMMIInfo->SubHeader,"%s", (char *)pBuf);
            pBuf += strlen((char *)pBuf) + 1;
            sprintf((char *)pMMIInfo->ButtomLine,"%s", (char *)pBuf);
            
            if (pMMIInfo->ItemCount == 0xff)
                pMMIInfo->ItemCount = 0;
            
            if (pMMIInfo->ItemCount > 9)
                pMMIInfo->ItemCount = 9;
            
            for (i=0;i<pMMIInfo->ItemCount;i++) {
                pBuf += strlen((char *)pBuf) + 1;
                sprintf((char *)pMMIInfo->MenuItem[i], "%s", (char *)pBuf);
            }
            break;
        
        case PCMSG_LIST:
            *pType = 2;
            
            pMMIInfo->ItemCount = *pBuf++;
            sprintf((char *)pMMIInfo->Header, "%s", (char *)pBuf);
            pBuf += strlen((char *)pBuf) + 1;
            sprintf((char *)pMMIInfo->SubHeader, "%s", (char *)pBuf);
            pBuf += strlen((char *)pBuf) + 1;
            sprintf((char *)pMMIInfo->ButtomLine, "%s", (char *)pBuf);
            
            if ((pMMIInfo->ItemCount == 0xff) || (pMMIInfo->ItemCount == 0)) {
                noItemFlag = true;
                pMMIInfo->ItemCount = 1;
            }
            
            if (pMMIInfo->ItemCount > 9)
                pMMIInfo->ItemCount = 9;
            
            if (noItemFlag) {
                sprintf((char *)pMMIInfo->MenuItem[0], "%s", pMMIInfo->ButtomLine);
            }
            else {
                for (i=0;i<pMMIInfo->ItemCount;i++) {
                    pBuf += strlen((char *)pBuf) + 1;
                    sprintf((char *)pMMIInfo->MenuItem[i], "%s", (char *)pBuf);
                }
            }
            break;
            
        case PCMSG_ENQ:
            *pType = 3;
            
            pMMIInfo->Blind_Answer = *pBuf++;
            pMMIInfo->Answer_Text_Length = *pBuf++;
            sprintf((char *)pMMIInfo->Prompt, "%s", (char *)pBuf);
            pMMIInfo->EnqFlag = TRUE;
            break;
            
        default:
            *pType = 0;
            return false;
    }
    
    return true;
}

BOOL answer(
    MMI_Info *pMMIInfo,
    int      Type,
    HANDLE   hDevice2)
{
    BOOL   StatusSuccess = TRUE;
    LPVOID inBuf;
    DWORD  inBufLength = 0;
    ULONG  bytesReturned;

    if (hDevice2 == NULL) {
        return false;
    }

    BYTE MenuAnswer[] = {0xaa, 0x08, 0x40, 0x00, 0x00, PCMSG_MENU_ANSWER, 0x01, 0x00, 0x01, 0xff};
    BYTE EnqAnswer[256] = {0xaa,0x08,0x40,0x00,0x00, PCMSG_ANSWER, 0x01,0x00,0x01,0xff};
    
    //Answer Cancel, 
    //When Type == 1, pMMIInfo->Answer = 0;
    //When Type == 2, Automatic;
    //When Type == 3, pMMIInfo->Answer_Text_Length = 0;
    switch (Type) {
        case 1:
            MenuAnswer[8] = pMMIInfo->Answer;
            inBufLength = 12;
            MakeCheckSum2(MenuAnswer);
            inBuf = (PVOID)MenuAnswer;
            break;
            
        case 2:
            MenuAnswer[8] = 0;
            inBufLength = 12;
            MakeCheckSum2(MenuAnswer);
            inBuf = (PVOID)MenuAnswer;
            break;
            
        case 3:
            inBufLength = 10;
            if (pMMIInfo->Answer_Text_Length) {
                CopyMemory((PVOID)(EnqAnswer + 9),
                           (PVOID)(pMMIInfo->AnswerStr),
                           pMMIInfo->Answer_Text_Length);
                EnqAnswer[1] += pMMIInfo->Answer_Text_Length;
                EnqAnswer[6] += pMMIInfo->Answer_Text_Length;
                inBufLength += pMMIInfo->Answer_Text_Length;
            }
            else {
                EnqAnswer[8] = 0;
            }
            MakeCheckSum2(EnqAnswer);
            inBuf = (PVOID)EnqAnswer;
            break;
            
        default:
            return false;
    }
    
    StatusSuccess = DeviceIoControl(hDevice2, 
                                    (DWORD)DST_IOCTL_CA_WRITE, 
                                    (PVOID)inBuf, 
                                    inBufLength, 
                                    NULL, 
                                    0, 
                                    &bytesReturned, 
                                    NULL);
    if (!StatusSuccess) {
        return false;
    }
    
    return true;
}


BOOL closeMMI(HANDLE hDevice2)
{
    BOOL  StatusSuccess = TRUE;
    //LPVOID inBuf;
    DWORD inBufLength = 9;
    ULONG bytesReturned;

    BYTE CloseMenu[] = {0xaa, 0x07, 0x40, 0x00, 0x00, PCMSG_CLOSE_MMI, 0x00, 0x00, 0xff};
    MakeCheckSum2(CloseMenu);
    
    StatusSuccess = DeviceIoControl(hDevice2, 
                                    (DWORD)DST_IOCTL_CA_WRITE, 
                                    (PVOID)CloseMenu, 
                                    inBufLength, 
                                    NULL, 
                                    0, 
                                    &bytesReturned, 
                                    NULL);
    if (!StatusSuccess) {
        return false;
    }
    
    return true;
}


void Analyse_PMT(
    BYTE               *pBuf, // Buffer is started in program_number of TS_program_map_section()
    int                BufSize,
    DWORD              VPid,
    DWORD              APid,
    BYTE               *pStartPMTBuf,
    int                *pStartPMTSize,
    BYTE               *pStopPMTBuf,
    int                *pStopPMTSize)
{
    BYTE  *pData = pBuf;
    int   DataSize = BufSize;
    int   ReadPosition = 0;
    ULONG program_info_length = 0;
    BYTE  stream_type = 0;
    ULONG elementary_PID = 0;
    ULONG ES_info_length = 0;
    BOOL  Match_Flag = false;

    *pStartPMTSize = 0;
    *pStopPMTSize = 0;
    
    ReadPosition = 7;
    program_info_length = *(pData + ReadPosition++) & 0x0f;
    program_info_length = (program_info_length << 8) | *(pData + ReadPosition++);
    ReadPosition += program_info_length;
    
    while (ReadPosition < (DataSize - 4)) {
        stream_type = *(pData + ReadPosition++);
        elementary_PID = *(pData + ReadPosition++) & 0x1f;
        elementary_PID = (elementary_PID << 8) | *(pData + ReadPosition++);
        ES_info_length = *(pData + ReadPosition++) & 0x0f;
        ES_info_length = (ES_info_length << 8) | *(pData + ReadPosition++);
        
        // stream type:
        // 0x01    MPEG1 Video;
        // 0x02    MPEG2 Video;
        // 0x03    MPEG1 Audio;
        // 0x04    MPEG2 Audio;
        
        if ((stream_type == 0x02) || (stream_type == 0x01)) {
            if (VPid == elementary_PID) {
                Match_Flag = true;
            }
        }
        else if ((stream_type == 0x04) || (stream_type == 0x03)) {
            if (APid == elementary_PID) {
                Match_Flag = true;
            }
        }
        
        ReadPosition += ES_info_length;
    }
    
    if (Match_Flag) {
        ReadPosition = 7;
        program_info_length = *(pData + ReadPosition++) & 0x0f;
        program_info_length = (program_info_length << 8) | *(pData + ReadPosition++);
        
        pStartPMTBuf[(*pStartPMTSize)++] = 0x03;
        pStopPMTBuf[(*pStopPMTSize)++] = 0x03;
        CopyMemory((PVOID)(pStartPMTBuf + (*pStartPMTSize)),
                   (PVOID)pData,
                   3);
        (*pStartPMTSize) += 3;
        CopyMemory((PVOID)(pStopPMTBuf + (*pStopPMTSize)),
                   (PVOID)pData,
                   3);
        (*pStopPMTSize) += 3;
        
        if (program_info_length && (program_info_length < 256)) {
            pStartPMTBuf[(*pStartPMTSize)++] = *(pData + 7) & 0x0f;
            pStartPMTBuf[(*pStartPMTSize)++] = *(pData + 8) + 1;
            pStartPMTBuf[(*pStartPMTSize)++] = 0x01;
            CopyMemory((PVOID)(pStartPMTBuf + (*pStartPMTSize)),
                       (PVOID)(pData + ReadPosition),
                       program_info_length);
            (*pStartPMTSize) += program_info_length;
        }
        else {
            pStartPMTBuf[(*pStartPMTSize)++] = 0xf0;
            pStartPMTBuf[(*pStartPMTSize)++] = 0;
        }
        
        pStopPMTBuf[(*pStopPMTSize)++] = 0xf0;
        pStopPMTBuf[(*pStopPMTSize)++] = 0;
        
        ReadPosition += program_info_length;
        
        while (ReadPosition < (DataSize - 4)) {
            stream_type = *(pData + ReadPosition++);
            elementary_PID = *(pData + ReadPosition++) & 0x1f;
            elementary_PID = (elementary_PID << 8) | *(pData + ReadPosition++);
            ES_info_length = *(pData + ReadPosition++) & 0x0f;
            ES_info_length = (ES_info_length << 8) | *(pData + ReadPosition++);
            
            ReadPosition -= 5;
            
            if (CAM_Type == CAM_DEFAULT) {
                if ((VPid == elementary_PID) || (APid == elementary_PID)) {
                    CopyMemory((PVOID)(pStartPMTBuf + (*pStartPMTSize)),
                               (PVOID)(pData + ReadPosition),
                               3);
                    (*pStartPMTSize) += 3;
                    
                    CopyMemory((PVOID)(pStopPMTBuf + (*pStopPMTSize)),
                               (PVOID)(pData + ReadPosition),
                               3);
                    (*pStopPMTSize) += 3;
                    pStopPMTBuf[(*pStopPMTSize)++] = 0;
                    pStopPMTBuf[(*pStopPMTSize)++] = 0;
                    
                    ReadPosition += 5;
                    
                    int nLen = 0;
                    int descriptor_tag = 0;
                    int descriptor_length = 0;
                    int nESInfoLenPos = *pStartPMTSize;
                    int nESInfoLen = 0;
                    int CADescriptorLen = 0;
                    
                    if (ES_info_length != 0) {
                        while (nLen < (int)ES_info_length - 1) {
                            descriptor_tag = *(pData + ReadPosition + nLen++);
                            descriptor_length = *(pData + ReadPosition + nLen++);
                            
                            //descriptor tag is 09, the other is not need.
                            if (descriptor_tag == 0x09) {
                                if (nESInfoLenPos == *pStartPMTSize) {
                                    //tag and length 2 bytes, ca_pmt_cmd_id 1 byte
                                    nESInfoLen = CADescriptorLen = descriptor_length + 2 + 1;
                                    pStartPMTBuf[(*pStartPMTSize)++] = (BYTE)(CADescriptorLen >> 8);
                                    pStartPMTBuf[(*pStartPMTSize)++] = (BYTE)(CADescriptorLen & 0xff);
                                    pStartPMTBuf[(*pStartPMTSize)++] = 0x01;
                                    CopyMemory((PVOID)(pStartPMTBuf + (*pStartPMTSize)),
                                               (PVOID)(pData + ReadPosition + nLen - 2),
                                               CADescriptorLen - 1);
                                    (*pStartPMTSize) += CADescriptorLen - 1;
                                }
                                else {
                                    CADescriptorLen = descriptor_length + 2;
                                    nESInfoLen += CADescriptorLen;
                                    pStartPMTBuf[nESInfoLenPos] = (BYTE)(nESInfoLen >> 8);
                                    pStartPMTBuf[nESInfoLenPos+1] = (BYTE)(nESInfoLen & 0xff);
                                    CopyMemory((PVOID)(pStartPMTBuf + (*pStartPMTSize)),
                                               (PVOID)(pData + ReadPosition + nLen - 2),
                                               CADescriptorLen);
                                    (*pStartPMTSize) += CADescriptorLen;
                                }
                            }
                            nLen += descriptor_length;
                        }
                        
                        //Haven't CA_descripor
                        if (nESInfoLenPos == *pStartPMTSize) {
                            pStartPMTBuf[(*pStartPMTSize)++] = 0;
                            pStartPMTBuf[(*pStartPMTSize)++] = 0;
                        }
                    }
                    else {
                        pStartPMTBuf[(*pStartPMTSize)++] = 0;
                        pStartPMTBuf[(*pStartPMTSize)++] = 0;
                    }
                    ReadPosition += ES_info_length;
                }
                else {
                    ReadPosition += 5 + ES_info_length;
                }
            }
            
            if (CAM_Type == CAM_ASTON) {
				if ((VPid == elementary_PID) ||	(APid == elementary_PID)) {
					CopyMemory((PVOID)(pStartPMTBuf + (*pStartPMTSize)),
							   (PVOID)(pData + ReadPosition),
							   3);
					(*pStartPMTSize) += 3;

					CopyMemory((PVOID)(pStopPMTBuf + (*pStopPMTSize)),
							   (PVOID)(pData + ReadPosition),
							   3);
					(*pStopPMTSize) += 3;
					pStopPMTBuf[(*pStopPMTSize)++] = 0;
					pStopPMTBuf[(*pStopPMTSize)++] = 0;

					ReadPosition += 5;

					int nLen = 0;
					int descriptor_tag = 0;
					int descriptor_length = 0;
					int nValidTagPos = 0;
					int CADescriptorLen = 0;

					if (ES_info_length != 0) {
						while (nLen < (int)ES_info_length - 1) {
							descriptor_tag = *(pData + ReadPosition + nLen++);
							descriptor_length = *(pData + ReadPosition + nLen++);
							if ((descriptor_tag == 0x09) && (descriptor_length > 0x04)) {
								nValidTagPos = nLen - 2;
								break;
							}
							nLen += descriptor_length;
						}

						//descriptor tag is 09, the other is not need.
						int CADescriptorLen = *(pData + ReadPosition + nValidTagPos + 1);
						CADescriptorLen += 2 + 1;
						pStartPMTBuf[(*pStartPMTSize)++] = (BYTE)(CADescriptorLen >> 8);
						pStartPMTBuf[(*pStartPMTSize)++] = (BYTE)(CADescriptorLen & 0xff);
						pStartPMTBuf[(*pStartPMTSize)++] = 0x01;
						CopyMemory((PVOID)(pStartPMTBuf + (*pStartPMTSize)),
								   (PVOID)(pData + ReadPosition + nValidTagPos),
								   CADescriptorLen - 1);
						(*pStartPMTSize) += CADescriptorLen - 1;
					}
					else {
						pStartPMTBuf[(*pStartPMTSize)++] = 0;
						pStartPMTBuf[(*pStartPMTSize)++] = 0;
					}
					ReadPosition += ES_info_length;
				}
				else {
					ReadPosition += 5 + ES_info_length;
				}
			}

			if ((CAM_Type == CAM_CONAX) || (CAM_Type == CAM_CRYPTOWORKS)) {
				CopyMemory((PVOID)(pStartPMTBuf + (*pStartPMTSize)),
						   (PVOID)(pData + ReadPosition),
						   3);
				(*pStartPMTSize) += 3;

				CopyMemory((PVOID)(pStopPMTBuf + (*pStopPMTSize)),
						   (PVOID)(pData + ReadPosition),
						   3);
				(*pStopPMTSize) += 3;
				pStopPMTBuf[(*pStopPMTSize)++] = 0;
				pStopPMTBuf[(*pStopPMTSize)++] = 0;

				ReadPosition += 5;

				int nLen = 0;
				int descriptor_tag = 0;
				int descriptor_length = 0;
				int nPos = *pStartPMTSize;
				int CADescriptorLen = 0;

				if (ES_info_length != 0) {
					//descriptor tag is 0x09, the other is not need.
					while (nLen < (int)ES_info_length - 1) {
						descriptor_tag = *(pData + ReadPosition + nLen++);
						descriptor_length = *(pData + ReadPosition + nLen++);
						if (descriptor_tag == 0x09) {
							*pStartPMTSize = nPos;
							CADescriptorLen = descriptor_length + 2 + 1;
							pStartPMTBuf[(*pStartPMTSize)++] = (BYTE)(CADescriptorLen >> 8);
							pStartPMTBuf[(*pStartPMTSize)++] = (BYTE)(CADescriptorLen & 0xff);
							pStartPMTBuf[(*pStartPMTSize)++] = 0x01;
							CopyMemory((PVOID)(pStartPMTBuf + (*pStartPMTSize)),
									   (PVOID)(pData + ReadPosition + nLen - 2),
									   CADescriptorLen - 1);
							(*pStartPMTSize) += CADescriptorLen - 1;
						}
						nLen += descriptor_length;
					}
				}

				if ((nPos == *pStartPMTSize) || (ES_info_length == 0)) {
					pStartPMTBuf[(*pStartPMTSize)++] = *(pData + ReadPosition - 2) & 0xf0;
					pStartPMTBuf[(*pStartPMTSize)++] = 0;
				}

				ReadPosition += ES_info_length;
			}
            
        }
    }
}


// CAM start decrypt stream (send PMT info to CAM)
void SendPMT(
    BYTE   *pBuf, // PMT info buffer
    int    Size,  // PMT info size
    HANDLE hDevice2)
{
    BYTE *pBuffer = pBuf;
    int nSize = Size;
    int i = 0;
    DWORD bytesReturned = 0;

    if (nSize) {
        // set CI Information
        for (i=nSize-1; i>=0; i--) {
            pBuf[i+8] = pBuf[i];
        }
        nSize += 9;
        pBuf[0] = 0xaa;
        pBuf[1] = nSize - 2;
        pBuf[2] = 0x40;
        pBuf[3] = 0x03;
        pBuf[4] = 0;
        pBuf[5] = 0x03;
        pBuf[6] = nSize - 9;
        pBuf[7] = 0;
        pBuf[nSize - 1] = 0;
        for (i=1; i<nSize-1; i++) {
            pBuf[nSize-1] += pBuf[i];
        }
        pBuf[nSize-1] = ~pBuf[nSize-1] + 1;
        
        DeviceIoControl(hDevice2, 
                        (DWORD)DST_IOCTL_CA_WRITE, 
                        pBuf, 
                        nSize, 
                        NULL, 
                        0, 
                        &bytesReturned, 
                        NULL);
    }
}


// -------------------------------------------------------------------------------------
BYTE  *CI_Start_PMT_Buf = new BYTE[1024];
int	  CI_Start_PMT_Size = 0;
BYTE  *CI_Stop_PMT_Buf = new BYTE[1024];
int	  CI_Stop_PMT_Size = 0;

// Call Analyse_PMT to get pStartPMTBuf and pStopPMTBuf
Analyse_PMT(pBuf, BufSize, VPid, APid, CI_Start_PMT_Buf, CI_Start_PMT_Size, CI_Stop_PMT_Buf, CI_Stop_PMT_Size);

// Tell CAM to start decrypt stream
SendPMT(CI_Start_PMT_Buf, CI_Start_PMT_Size, hDevice2);

// Tell CAM to stop decrypt stream
SendPMT(CI_Stop_PMT_Buf, CI_Stop_PMT_Size, hDevice2);
// -------------------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////
// 13. Get MAC address
bool getMACAddr(char *chMACAddr, HANDLE hDevice2)
{
    BOOL			StatusSuccess = TRUE;
	LPVOID			inBuf;
	LPVOID			outBuf; 
	DWORD			inBufLength;
	DWORD			outBufLength;
    ULONG			bytesReturned;

	if (hDevice2 == NULL) {
		return false;
	}
	
	//Check MAC Address
	TunerData tunerData;
	memset(&tunerData,0,sizeof(TunerData));

	tunerData.address = 0xaa;
	tunerData.frequencyMSB = 0;
	tunerData.frequencyLSB = 0x0a;
	MakeCheckSum(&tunerData);

	inBuf = &tunerData;
	inBufLength	= sizeof(tunerData);		
	outBuf = &tunerData;
	outBufLength = inBufLength;
		
	StatusSuccess = DeviceIoControl(hDevice2, 
								    DST_IOCTL_SET_INFO, 
									inBuf, 
									inBufLength + 1, 
									outBuf, 
									outBufLength, 
									&bytesReturned, 
									NULL);
	if (!StatusSuccess) {
		return false;
	}

	BYTE *temp = (BYTE *)(&tunerData);
	char str[8];
	strcpy(chMACAddr,"");
	for (int i=1;i<8;i++) {
		if (i == 4)
			continue;
		byteToHex(temp[i],str);
		strcat(chMACAddr,str);
		if (i < 7)
			strcat(chMACAddr,"-");
	}

	return true;
}
////////////////////////////////////////////////////////////////////////////////////////