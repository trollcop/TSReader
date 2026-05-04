#ifdef BCM4500

#include <windows.h>
#include <winioctl.h>
#include <commctrl.h>

#include "c:\cypress\usb\drivers\ezusbdrv\ezusbsys.h"
#include "USBIoctlStructs.h"

#include "bcm4500.h"
#include "resource.h"

#define HCTL1   0xa0
#define APSTAT1 0xa2
#define APSTAT2 0xa4
#define HABADR  0xa6
#define HABDATA 0xa7
#define HABSTAT 0xa8

#define outI2CWRITE 19
#define outI2CWRITEEEPROM 20
#define outI2CREAD 21
#define outWriteBCM4500RAM 23
#define outDNHardware 0
#define outDNHardware2 1

BYTE apstat1;
BYTE apstat2;
BYTE habstat;
BYTE hab[64];

extern HANDLE g_hUSB;
extern BOOL g_fAbortI2CRead;
extern HANDLE hInstance;

BOOL i2c_read(BYTE opcode, BYTE * buffer)
{
	int nBytesSent, nBytesReceived;
	int nResult;
	BULK_TRANSFER_CONTROL bulkControl;
	BYTE bOut[4];
	
	bOut[0] = outI2CREAD;		// I2C read command
	bOut[1] = 0x20;				// address
	bOut[2] = 1;				// byte count
	bOut[3] = opcode;			// register
	bulkControl.pipeNum = 0;
	nResult = DeviceIoControl(g_hUSB, IOCTL_EZUSB_BULK_WRITE, &bulkControl, sizeof(BULK_TRANSFER_CONTROL), bOut, sizeof(bOut), &nBytesSent, NULL);

	bulkControl.pipeNum = 1;
	nResult = DeviceIoControl(g_hUSB, IOCTL_EZUSB_BULK_READ, &bulkControl, sizeof(BULK_TRANSFER_CONTROL), buffer, 1, &nBytesReceived, NULL);
	if ( (nResult) && (g_fAbortI2CRead == FALSE) )
		return TRUE;
	return FALSE;
}

BOOL i2c_write(BYTE opcode, BYTE val)
{
	int nBytesSent;
	int nResult;
	BULK_TRANSFER_CONTROL bulkControl;
	BYTE bOut[6];
		
	bOut[0] = outI2CWRITE;		// I2C write command
	bOut[1] = 0x20;				// address
	bOut[2] = 1;				// byte count
	bOut[3] = opcode;			// register
	bOut[4] = val;				// value
	bulkControl.pipeNum = 0;
	nResult = DeviceIoControl(g_hUSB, IOCTL_EZUSB_BULK_WRITE, &bulkControl, sizeof(BULK_TRANSFER_CONTROL), bOut, 5, &nBytesSent, NULL);
	if (nResult)
		return TRUE;
	return FALSE;
}

WORD WriteAndReadDNIF(WORD wOut)
{
	int nBytesSent, nBytesReceived;
	BOOL bResult;
	BULK_TRANSFER_CONTROL bulkControl;
	BYTE bOut[3];
	BYTE bIn[2];
	WORD wRetVal;

	bOut[0] = outDNHardware;
	bOut[1] = wOut >> 8;
	bOut[2] = wOut & 0xff;;
	bulkControl.pipeNum = 0;
	bResult = DeviceIoControl(g_hUSB, IOCTL_EZUSB_BULK_WRITE, &bulkControl, sizeof(BULK_TRANSFER_CONTROL), bOut, sizeof(bOut), &nBytesSent, NULL);
	
	bulkControl.pipeNum = 1;
	bResult = DeviceIoControl(g_hUSB, IOCTL_EZUSB_BULK_READ, &bulkControl, sizeof(BULK_TRANSFER_CONTROL), bIn, sizeof(bIn), &nBytesReceived, NULL);
	wRetVal = bIn[0] << 8 | bIn[1];

	return wRetVal;
}

void DoDNIF(WORD wOut)
{
	/*
	char szTemp[100];
	wsprintf(szTemp, "Linsys4500: DNIF: Sent 0x%04x Received 0x%04x\n", wOut, WriteAndReadDNIF(wOut));
	OutputDebugString(szTemp);
	*/
	WriteAndReadDNIF(wOut);
}

void SetupDNIF()
{	
	DoDNIF(0x8000);

	DoDNIF(0x0200);
	DoDNIF(0x0200);
	DoDNIF(0x0100);
	DoDNIF(0x0100);
	DoDNIF(0x0100);
	DoDNIF(0x0100);
	DoDNIF(0x0100);
	DoDNIF(0x0100);
	Sleep(100);

	DoDNIF(0x0201);	// 3V on after this
	DoDNIF(0x0203); // CLK driven after this
	Sleep(100);

	DoDNIF(0x0104);	// 
	Sleep(100);

	DoDNIF(0x0100);	// I2C relay off
	DoDNIF(0x0102);	// I2C relay on
	DoDNIF(0x0103);	// I2C relay on and bus on
}

BOOL WriteBCM4500Memory(HWND hDlg)
{
	HANDLE hFile;
	DWORD dwRead;
	int i;
	BOOL fRetVal = TRUE;
	char szFirmwareFile[MAX_PATH];

	GetModuleFileName(hInstance, szFirmwareFile, sizeof(szFirmwareFile));
	for (i = lstrlen(szFirmwareFile); i > 0; i--)
	{
		if (szFirmwareFile[i] == '\\')
		{
			szFirmwareFile[i + 1] = 0;
			break;
		}
	}
	lstrcat(szFirmwareFile, "apfirmware.bin1");

	hFile = CreateFile(szFirmwareFile, GENERIC_READ, 0, (LPSECURITY_ATTRIBUTES) NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, (HANDLE) NULL);
	if (hFile != INVALID_HANDLE_VALUE)
	{
		BOOL status;
		BYTE bOut[64];
		BYTE * pBuffer = LocalAlloc(LPTR, 0x4000);

		ReadFile(hFile, pBuffer, 0x4000, &dwRead, NULL);
		if (dwRead == 0x4000)
		{
			int nAddr = 0;
			int nRemaining = 0x4000;
			int nThisTime;
			int nProgress = 0;

			SendDlgItemMessage(hDlg, IDC_BCM4500_PROGRESS, PBM_SETRANGE32, 0, 0x4000);

			status = i2c_write(HCTL1, 1);	// turn on reset
			do
			{
				int nBytesSent;
				BULK_TRANSFER_CONTROL bulkControl;
				//char szTemp[100];

				nThisTime = nRemaining;
				if (nThisTime > 59)
					nThisTime = 59;
				nProgress += nThisTime;
				SendDlgItemMessage(hDlg, IDC_BCM4500_PROGRESS, PBM_SETPOS, nProgress, 0);

				//wsprintf(szTemp, "Linsys4500: 0x%04x to 0x%04x\n", nAddr, nAddr + nThisTime);
				//OutputDebugString(szTemp);

				bOut[0] = outWriteBCM4500RAM;
				bOut[1] = 0x20; // I2C addr
				bOut[2] = nThisTime;	// count
				bOut[3] = (nAddr >> 8) & 0xff;
				bOut[4] = nAddr & 0xff;
				memcpy(&bOut[5], &pBuffer[nAddr], nThisTime);
				bulkControl.pipeNum = 0;
				DeviceIoControl(g_hUSB, IOCTL_EZUSB_BULK_WRITE, &bulkControl, sizeof(BULK_TRANSFER_CONTROL), bOut, nThisTime + 5, &nBytesSent, NULL);

				nAddr += nThisTime;
				nRemaining -= nThisTime;
			} while (nRemaining);
		}
		else
		{
			OutputDebugString("Linsys4500: Failed to read 0x4000 bytes\n");
			fRetVal = FALSE;
		}

		LocalFree(pBuffer);
		CloseHandle(hFile);

		status = i2c_write(HCTL1, 0);	// turn off reset
		Sleep(100);
	}
	else
		fRetVal = FALSE;

	return fRetVal;
}

BOOL OpenUSBDriver()
{
	int nPipeCount;
	int nBytes;
	char szDeviceName[MAX_PATH];
	UCHAR InterfaceInfo[1024];

	lstrcpy(szDeviceName, "\\\\.\\Ezusb-0");
	g_hUSB = CreateFile(szDeviceName, GENERIC_WRITE, FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
	if (g_hUSB == INVALID_HANDLE_VALUE)
	{
		OutputDebugString("Linsys4500: Unable to open the EZUSB device driver\n");
		return FALSE;
	}

	// Get Pipe info
	DeviceIoControl(g_hUSB, IOCTL_Ezusb_GET_PIPE_INFO, NULL, 0, &InterfaceInfo, sizeof(InterfaceInfo), &nBytes, NULL);
	nPipeCount = ((PUSBD_INTERFACE_INFORMATION) InterfaceInfo)->NumberOfPipes;
	if (nPipeCount != 6)
	{
		CloseHandle(g_hUSB);
		g_hUSB = INVALID_HANDLE_VALUE;
		OutputDebugString("Linsys4500: Pipe count mismatch - firmware loaded?\n");
		return FALSE;
	}

	return TRUE;
}

BOOL bcm4500_ap_running()
{
	apstat1 = 0;
	if (i2c_read(APSTAT1, &apstat1) == FALSE)
		return FALSE;

	return (apstat1 & 0x08);
}

BOOL bcm4500_hab_available()
{
	habstat = 0;
	if (i2c_read(HABSTAT, &habstat) == FALSE)
		return FALSE;
	if (habstat & 0x01)
		return FALSE;
	return TRUE;
}

BOOL bcm4500_ok_to_send()
{
	if (!bcm4500_ap_running())
		return FALSE;
	if (!bcm4500_hab_available())
		return FALSE;
	if (!i2c_read(APSTAT2, &apstat2))
		return FALSE;

	return (apstat2 & 0x80);
}

BOOL hab_write(BYTE n, BYTE * buffer)
{
	BOOL status;
	BYTE i;

	if (!bcm4500_ok_to_send())
		return FALSE;

	status = i2c_write(HABADR, 0);
	for (i = 0; (status == TRUE) && (i < n); i++)
		status = i2c_write(HABDATA, buffer[i]);

	if (status)
		status = i2c_write(HABSTAT, 0x03);

	return status;
}

BOOL hab_read(BYTE n, BYTE * buffer)
{
	BOOL status;
	BYTE i;

	if (!bcm4500_ap_running())
		return FALSE;

	for (i = 0; i < 5; i++)
	{
		if (bcm4500_hab_available())
			break;
		else
			Sleep(500);
	}
	if (i == 5)
		return FALSE;

	status = i2c_write(HABADR, 0);
	for (i = 0; (status == TRUE) && (i < n); i++)
		status = i2c_read(HABDATA, &buffer[i]);

	return status;
}

BOOL bcm4500_get_legacy_status(BYTE ** buffer)
{
	hab[0] = 0x01;
	hab[1] = 0x10;
	hab[2] = 0x00;

	if (!hab_write(3, hab))
		return FALSE;

	if (!hab_read(47, hab))
		return FALSE;

	if (hab[0] != 0x81)
		return FALSE;

	*buffer = &hab[0];
	return TRUE;
}

BOOL bcm4500_get_turbo_status(BYTE ** buffer)
{
	hab[0] = 0x01;
	hab[1] = 0x11;
	hab[2] = 0x00;

	if (!hab_write(3, hab))
		return FALSE;
	if (!hab_read(57, hab))
		return FALSE;

	if (hab[0] != 0x81)
		return FALSE;

	*buffer = &hab[0];
	return TRUE;

}

BOOL bcm4500_tune(unsigned long freq_hz)
{
	hab[0] = 0x06;
	hab[1] = 0x0b;
	hab[2] = 0x17;
	hab[3] = (BYTE)(freq_hz >> 24) & 0xff;
	hab[4] = (BYTE)(freq_hz >> 16) & 0xff;
	hab[5] = (BYTE)(freq_hz >> 8) & 0xff;
	hab[6] = (BYTE)freq_hz & 0xff;
	hab[7] = 0x00;

	if (!hab_write(8, hab))
		return FALSE;
	if (!hab_read(1, hab))
		return FALSE;
	
	return (hab[0] == 0x86);
}

BOOL bcm4500_acquire(BYTE byte2, BYTE byte3)
{
	hab[0] = 0x03;
	hab[1] = 0x0f;
	hab[2] = byte2;
	hab[3] = byte3;
	hab[4] = 0x00;

	if (!hab_write(5, hab))
		return FALSE;

	if (!hab_read(1, hab))
		return FALSE;
	
	return (hab[0] == 0x83);
}

BOOL bcm4500_acquire2(BYTE byte2, BYTE byte3, DWORD baud_offset_hz, DWORD carrier_offset_hz,
					  BYTE ctl_flags1, BYTE ctl_flags2, BYTE ctl_flags3)
{
	hab[0] = 0x03;
	hab[1] = 0x0f;
	hab[2] = byte2;
	hab[3] = byte3 | 0x08;
	hab[4] = (BYTE)(baud_offset_hz >> 24) & 0xff;
	hab[5] = (BYTE)(baud_offset_hz >> 16) & 0xff;
	hab[6] = (BYTE)(baud_offset_hz >> 8) & 0xff;
	hab[7] = (BYTE)baud_offset_hz & 0xff;
	hab[8] = (BYTE)(carrier_offset_hz >> 24) & 0xff;
	hab[9] = (BYTE)(carrier_offset_hz >> 16) & 0xff;
	hab[10] = (BYTE)(carrier_offset_hz >> 8) & 0xff;
	hab[11] = (BYTE)carrier_offset_hz & 0xff;
	hab[12] = ctl_flags1;
	hab[13] = ctl_flags2;
	hab[14] = ctl_flags3;
	hab[15] = 0x00;

	if (!hab_write(16, hab))
		return FALSE;

	if (!hab_read(1, hab))
		return FALSE;
	
	return (hab[0] == 0x83);
}

BOOL bcm4500_edit_symbol_rate_list(BYTE sym_idx, DWORD sym_rate_hz)
{
	BYTE addr_lo;

	switch(sym_idx)
	{
	case 0:
		addr_lo = 0x4b;
		break;
	case 1:
		addr_lo = 0x4f;
		break;
	case 2:
		addr_lo = 0x53;
		break;
	case 3:
		addr_lo = 0x57;
		break;
	case 4:
		addr_lo = 0x5b;
		break;
	case 5:
		addr_lo = 0x5f;
		break;
	case 6:
		addr_lo = 0x63;
		break;
	case 7:
		addr_lo = 0x67;
		break;
	default:
		return FALSE;
	}

	hab[0] = 0x07;
	hab[1] = 0x09;
	hab[2] = 0x39;
	hab[3] = addr_lo;
	hab[4] = (BYTE)(sym_rate_hz >> 24) & 0xff;
	hab[5] = (BYTE)(sym_rate_hz >> 16) & 0xff;
	hab[6] = (BYTE)(sym_rate_hz >> 8) & 0xff;
	hab[7] = (BYTE)sym_rate_hz & 0xff;
	hab[8] = 0;

	if (!hab_write(9, hab))
		return FALSE;

	if (!hab_read(1, hab))
		return FALSE;
	
	return (hab[0] == 0x87);
}

void GetLegacyCodeRate(int nRate, char * szDescription)
{
	switch(nRate)
	{
	case 0:
		lstrcpy(szDescription, "1/2");
		break;
	case 1:
		lstrcpy(szDescription, "2/3");
		break;
	case 2:
		lstrcpy(szDescription, "3/4");
		break;
	case 3:
		lstrcpy(szDescription, "5/6");
		break;
	case 4:
		lstrcpy(szDescription, "6/7");
		break;
	case 5:
		lstrcpy(szDescription, "7/8");
		break;
	case 8:
		lstrcpy(szDescription, "DCII 5/11");
		break;
	case 9:
		lstrcpy(szDescription, "DCII 1/2");
		break;
	case 10:
		lstrcpy(szDescription, "DCII 3/5");
		break;
	case 11:
		lstrcpy(szDescription, "DCII 2/3");
		break;
	case 12:
		lstrcpy(szDescription, "DCII 3/4");
		break;
	case 13:
		lstrcpy(szDescription, "DCII 4/5");
		break;
	case 14:
		lstrcpy(szDescription, "DCII 5/6");
		break;
	case 15:
		lstrcpy(szDescription, "DCII 7/8");
		break;
	default:
		lstrcpy(szDescription, "Error");
		break;
	}
}

DWORD ReadDWORD(BYTE * buffer, int nOffset)
{
	DWORD dwRetVal = buffer[nOffset] << 24 | buffer[nOffset + 1] << 16 | buffer[nOffset + 2] << 8 | buffer[nOffset + 3];
	return dwRetVal;
}

void GetTurboCodeRateA(int nRate, char * szDescription)
{
	switch(nRate)
	{
	case 0x00:
		lstrcpy(szDescription, "2/3");
		break;
	case 0x10:
		lstrcpy(szDescription, "5/6");
		break;
	case 0x20:
		lstrcpy(szDescription, "8/9");
		break;
	case 0x30:
		lstrcpy(szDescription, "1/2");
		break;
	case 0x40:
		lstrcpy(szDescription, "3/4");
		break;
	case 0x50:
		lstrcpy(szDescription, "3/4-II");
		break;
	case 0x60:
		lstrcpy(szDescription, "3/4-I");
		break;
	case 0x70:
		lstrcpy(szDescription, "QPSK 3/4");
		break;
	case 0x90:
		lstrcpy(szDescription, "QPSK 1/4");
		break;
	default:
		lstrcpy(szDescription, "Reserved");
		break;
	}
}

void GetTurboCodeRateB(int nRate, char * szDescription)
{
	switch(nRate)
	{
	case 0x00:
		lstrcpy(szDescription, "QPSK 1/2");
		break;
	case 0x10:
		lstrcpy(szDescription, "QPSK 3/4");
		break;
	case 0x20:
		lstrcpy(szDescription, "8PSK 2/3");
		break;
	case 0x30:
		lstrcpy(szDescription, "8PSK 5/6");
		break;
	case 0x40:
		lstrcpy(szDescription, "8PSK 8/9");
		break;
	case 0x50:
		lstrcpy(szDescription, "8PSK 3/4-I");
		break;
	case 0x60:
		lstrcpy(szDescription, "16QAM 3/4");
		break;
	case 0x70:
		lstrcpy(szDescription, "8PSK 3/4-II");
		break;
	case 0x90:
		lstrcpy(szDescription, "QPSK 1/4");
		break;
	default:
		lstrcpy(szDescription, "Reserved");
		break;
	}
}

void ReadStatus()
{
	BYTE * legacy_data;
	BYTE * turbo_data;
	char szTemp[200];

	if (bcm4500_get_legacy_status(&legacy_data))
	{
		DWORD dwSymbolRate;
		DWORD dwSymbolRateOffset, dwSymbolRateError, dwCarrierOffset, dwCarrierError;
		char szCodeRate[10];
		char szFECRotationPhase[30], szSpectralInversion[30];
		char szModMode[20];
		char szOutput[1024] = {0};

		lstrcat(szOutput, "Legacy status: ");
		if (legacy_data[2] & 0x01)
			lstrcat(szOutput, "RS Lock ");
		if (legacy_data[2] & 0x08)
			lstrcat(szOutput, "Viterbi Lock ");
		if (legacy_data[2] & 0x10)
			lstrcat(szOutput, "BERT Lock ");		
		lstrcat(szOutput, "\n");

		GetLegacyCodeRate(legacy_data[32] & 0x0f, szCodeRate);
		dwSymbolRate = ReadDWORD(legacy_data, 12);
		switch(legacy_data[32] & 0xc0)
		{
		case 0x00:
			lstrcpy(szFECRotationPhase, "0");
			break;
		case 0x40:
			lstrcpy(szFECRotationPhase, "90");
			break;
		case 0xc0:
			lstrcpy(szFECRotationPhase, "180");
			break;
		case 0x80:
			lstrcpy(szFECRotationPhase, "270");
			break;
		}
		switch(legacy_data[32] & 0x30)
		{
		case 0x00:
			lstrcpy(szSpectralInversion, "Normal (I/Q)");
			break;
		case 0x10:
			lstrcpy(szSpectralInversion, "I Channel Invert");
			break;
		case 0x20:
			lstrcpy(szSpectralInversion, "Q Channel Invert");
			break;
		case 0x30:
			lstrcpy(szSpectralInversion, "Not Used");
			break;
		}
		wsprintf(szTemp, "Symbol rate: %d Code rate: %s FEC Rotation Phase = %s Spectral Inversion = %s\n", dwSymbolRate, szCodeRate, 
						 szFECRotationPhase, szSpectralInversion);
		lstrcat(szOutput, szTemp);

		dwSymbolRateOffset = ReadDWORD(legacy_data, 16);
		dwSymbolRateError = ReadDWORD(legacy_data, 20);
		dwCarrierOffset = ReadDWORD(legacy_data, 24);
		dwCarrierError = ReadDWORD(legacy_data, 28);
		wsprintf(szTemp, "dwSymbolRateOffset = %d dwSymbolRateError = %d dwCarrierOffset = %d dwCarrierError = %d\n",
			     dwSymbolRateOffset, dwSymbolRateError, dwCarrierOffset, dwCarrierError);
		lstrcat(szOutput, szTemp);

		switch(legacy_data[5] & 0xf0)
		{
		case 0x00:
			lstrcpy(szModMode, "QPSK DVB");
			break;
		case 0x10:
			lstrcpy(szModMode, "QPSK DSS");
			break;
		case 0x20:
			lstrcpy(szModMode, "QPSK DCII");
			break;
		case 0x40:
			lstrcpy(szModMode, "OQPSK DCII");
			break;
		default:
			lstrcpy(szModMode, "Reserved");
			break;
		}
		wsprintf(szTemp, "3:par/ser 2:pn/mpeg 10:mode = %01x Mod Mode = %s\n", legacy_data[5] & 0x0f, szModMode);
		lstrcat(szOutput, szTemp);
		
		OutputDebugString(szOutput);
	}
	else
		OutputDebugString("Linsys4500: bcm4500_get_legacy_status returned FALSE;\n");

	if (bcm4500_get_turbo_status(&turbo_data))
	{
		DWORD dwTunerFrequency, dwSymbolRate, dwSymbolRateOffset, dwSymbolRateError;
		DWORD dwCarrierOffset, dwCarrierError;
		char szCodeRateA[20], szCodeRateB[20];
		char szOutput[1024] = {0};

		lstrcat(szOutput, "Turbo status: ");
		if (turbo_data[2] & 0x01)
			lstrcat(szOutput, "Receiver lock ");
		if (turbo_data[2] & 0x02)
			lstrcat(szOutput, "FEC lock ");
		if (turbo_data[2] & 0x10)
			lstrcat(szOutput, "BERT lock ");
		lstrcat(szOutput, "\n");
		
		dwTunerFrequency = ReadDWORD(turbo_data, 6);
		dwSymbolRate = ReadDWORD(turbo_data, 12);
		dwSymbolRateOffset = ReadDWORD(turbo_data, 16);
		dwSymbolRateError = ReadDWORD(turbo_data, 20);
		wsprintf(szTemp, "dwTunerFrequency = %d dwSymbolRate = %d dwSymbolRateOffset = %d dwSymbolRateError = %d\n",
			     dwTunerFrequency, dwSymbolRate, dwSymbolRateOffset, dwSymbolRateError);
		lstrcat(szOutput, szTemp);

		GetTurboCodeRateA(turbo_data[3] & 0xf0, szCodeRateA);
		GetTurboCodeRateB(turbo_data[5] & 0xf0, szCodeRateB);
		dwCarrierOffset = ReadDWORD(turbo_data, 24);
		dwCarrierError = ReadDWORD(turbo_data, 28);
		wsprintf(szTemp, "Code Rate-A: %s Code Rate-B: %s dwCarrierOffset = %d dwCarrierError = %d\n", 
			     szCodeRateA, szCodeRateB, dwCarrierOffset, dwCarrierError);
		lstrcat(szOutput, szTemp);
		wsprintf(szTemp, "3:par/ser 2:pn/mpeg 10:mode = %01x\n", turbo_data[5] & 0x0f);
		lstrcat(szOutput, szTemp);
		
		OutputDebugString(szOutput);
	}
	else
		OutputDebugString("Linsys4500: bcm4500_get_turbo_status returned FALSE;\n");
}

#endif BCM4500
