//
// Twinhan DTV driver IOControl call sample (2003/11/25)
//
// Description: 1. Fill TunerData fields to setup transponder
//              2. Lock channel
//              3. Sart stream capture
//              4. Stop stream capture
//              5. Get locking flag and Quality/Level values
//              6. Get stream buffer position
//              7. Get stream buffer start address
//              8. SetDiSEqC() command
//              9. 22kHz-tone controlling (on/off)


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
/////////////////////////////////////////////////////////////////////////////////////////
//obtain the handle of the two Twinhan devices (GUID_DST_DEVICE1 and GUID_DST_DEVICE2)
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
// Get Gard Type
#define DST_IOCTL_SET_INFO \
	CTL_CODE( DST_VIDEO, 0xA20, METHOD_BUFFERED, FILE_ANY_ACCESS )

int getCIInfo(HANDLE hVideo)
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
// 2. Lock channel
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

////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////
// 3. Sart stream capture
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
// 4. Stop stream capture
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
// 5. Get locking flag and Quality/Level values
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
		    Temp = pbuf[6];
		    *Str = Temp;
		    break;
	}
	
////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////
// 6. GetWritePos(): Get stream buffer position
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
// 7. GetDeviceBuffer(): Get stream buffer start address
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
// 8. SetDiSEqC() command
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


