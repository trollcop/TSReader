//#define HAMASAKI
//#define _DEBUG
#include <windows.h>
#include <commctrl.h>
#include <setupapi.h>
#include <initguid.h>
#include <stdio.h>
#include <math.h>

#include "CyAPI.h"

#include "..\sources.h"
#include "resource.h"
#include "hardware.h"

#ifdef ALPSTUNER
#include "drv2user.h"
#include "8VSBDriver/drv2004.h"
#endif ALPSTUNER

PSOURCESTRUCT ss;
BOOL fNeedTuneDialog = TRUE;
int nTunerStatusTimer;
CRITICAL_SECTION csSignal;
HANDLE hInstance;

CCyUSBDevice * USBDevice = NULL;
CCyBulkEndPoint * MPEGInEpt = NULL;   
OVERLAPPED outOvLap;
OVERLAPPED inOvLap; 

char szLastSignalReport[128] = {"n/a"};        
char szLastTune[128] = {"n/a"};

#ifdef EIGHTPSK
 #ifndef DVBTECH
 char gszSourceName[] = {"DTVWorks 8PSK"};
 #else DVBTECH
 char gszSourceName[] = {"DVBTech 8PSK"};
 #endif DVBTECH
#endif EIGHTPSK
#ifdef EIGHTVSB
char gszSourceName[] = {"DTVWorks 8VSB"};
#endif EIGHTVSB
#ifdef QAM
char gszSourceName[] = {"DTVWorks QAM"};
#endif QAM
#ifdef SPI
 #ifndef HORIZON
  #ifndef HAMASAKI
   char gszSourceName[] = {"DTVWorks DVB-SPI"};
  #else HAMASAKI
   char gszSourceName[] = {"Hamasaki ISDB Special"};
#endif HAMASAKI
char gszSPIKeyName[] = {"Software\\DTVWorks\\SPIInterface"};
 #else HORZION
char gszSourceName[] = {"Horizon TSR-S1"};
 #endif HORIZON
#endif SPI
#ifdef CIELPLUS_SKY
char gszSourceName[] = {"CielPlus Sky Interface"};
char gszCielPlusSkyKeyName[] = {"Software\\CielPlus\\SkyInterface"};
#endif CIELPLUS_SKY
#ifdef CIELPLUS_5000
char gszSourceName[] = {"CielPlus Dish Interface"};
#endif CIELPLUS_5000
#ifdef DVBS
#ifndef DSS
char gszSourceName[] = {"DTVWorks DVB-S"};
#else DSS
char gszSourceName[] = {"DTVWorks DVB-S (DSS mode)"};
#endif DSS
#endif DVBS

#ifdef SATELLITE_SOURCE
int nFrequency;
int nPolarity;
int nSymbolRate;
int nLNBFrequency;
int n22KHz;
int nDiSEqCInput;
int nADVModulationMode;
int nCodeRate;
BOOL fPowerOn;
#endif SATELLITE_SOURCE

#ifdef ALPSTUNER
int nFrequency;
BOOL fCheckMode;
BOOL fNXT2004InitDone;
 #ifdef _DEBUG
int nRegNameLen;
 #endif _DEBUG
#endif ALPSTUNER
BOOL fDoneDeInit = FALSE;

#ifdef SPI
BOOL fSPISyncMode = FALSE;
BOOL fDontAskMode = FALSE;
BOOL fDontTune = FALSE;
BOOL fFullSyncAsyncControlFirmware;
BOOL fInvertSPIClock = FALSE;
#endif SPI

#ifdef CIELPLUS_SKY
int nReceiverType = 0;
BOOL fDontAskMode = FALSE;
char szRFSequence[64];
#include "skyir.h"
#define SKY_RECEIVER_PACE 0
#define SKY_RECEIVER_AMSTRAD 1
#endif CIELPLUS_SKY

#ifdef CIELPLUS_5000
char szRFSequence[64];
#endif CIELPLUS_5000

extern "C" {
	BOOL __cdecl SourceHelper_ValidateSourceContainer(PSOURCESTRUCT pss);
}

BOOL SendUSB(BYTE * bOut, int nSize)
{
	LONG nBytesSent;

	UCHAR  * outContext = USBDevice->BulkOutEndPt->BeginDataXfer(bOut, nSize, &outOvLap); 
	USBDevice->BulkOutEndPt->WaitForXfer(&outOvLap, 1000);
	return (USBDevice->BulkOutEndPt->FinishDataXfer(bOut, nBytesSent, &outOvLap,outContext));
}

int ReceiveUSB(BYTE * bIn, int nSize)
{
	LONG nBytesReceived;

	UCHAR  * inContext = USBDevice->BulkInEndPt->BeginDataXfer(bIn, nSize, &inOvLap); 
	USBDevice->BulkInEndPt->WaitForXfer(&inOvLap, 1000); 
	USBDevice->BulkInEndPt->FinishDataXfer(bIn, nBytesReceived, &inOvLap,inContext);

	return nBytesReceived;
}

#ifdef CIELPLUS_SKY
void SendSkyRF(BYTE * pCommand)
{
	BYTE bOut[8];

	bOut[0] = outSendSkyIr;
	memcpy(&bOut[1], pCommand, 7);
	SendUSB(bOut, sizeof(bOut));
	Sleep(750);		// snooze while the command is sent
}
#endif CIELPLUS_SKY

#ifdef ALPSTUNER
 #ifdef _DEBUG
void DecodeNXT2004RegisterName(int nRegister, int nAddress, char * szReg)
{
	if (nAddress == 0x14)
	{
		switch(nRegister)
		{
		case 0x00:
			lstrcpy(szReg, "MISC_DEV_ID");
			break;
		case 0x01:
			lstrcpy(szReg, "MISC_FAB_ID");
			break;
		case 0x02:
			lstrcpy(szReg, "MISC_MONTH_PG");
			break;
		case 0x03:
			lstrcpy(szReg, "MISC_YEAR_OF_PG_LSB");
			break;
		case 0x04:
			lstrcpy(szReg, "MISC_YEAR_OF_PG_MSB");
			break;
		case 0x08:
			lstrcpy(szReg, "MISC_RESET_CONTROL");
			break; 
		case 0x0a:
			lstrcpy(szReg, "MISC_MOD_CONTROL_AND_OUTPUT_FORMAT");
			break;
		case 0x10:
			lstrcpy(szReg, "MISC_ASIC_HARDWARE_STATUS_1");
			break;
		case 0x11:
			lstrcpy(szReg, "MISC_ASIC_HARDWARE_STATUS_0");
			break;
		case 0x12:
			lstrcpy(szReg, "MISC_POWER_CONTROL");
			break;
		case 0x13:
			lstrcpy(szReg, "MISC_GPIO_ACCESS_SELECT");
			break;
		case 0x14:
			lstrcpy(szReg, "MISC_GPIO_OUTPUT_ENABLE");
			break;
		case 0x15:
			lstrcpy(szReg, "MISC_GPIO_MONITOR");
			break;
		case 0x19:
			lstrcpy(szReg, "MISC_FIRMWARE_CTL");
			break;
		case 0x1b:
			lstrcpy(szReg, "MISC_ELARA_SERIAL_CMD_2");
			break;
		case 0x1d:
			lstrcpy(szReg, "MISC_ELARA_SERIAL_CMD_0");
			break;
		case 0x1e:
			lstrcpy(szReg, "MISC_IIC_DEV_1_ADDR");
			break;
		case 0x20:
			lstrcpy(szReg, "UC_CONTROL");
			break;
		case 0x21:
			lstrcpy(szReg, "UC_SERVICES");
			break;
		case 0x22:
			lstrcpy(szReg, "UC_ACQUISITION_CONTROL");
			break;
		case 0x25:
			lstrcpy(szReg, "UC_IRQ_MASK");
			break;
		case 0x26:
			lstrcpy(szReg, "UC_IRQ_SOURCE");
			break;
		case 0x29:
			lstrcpy(szReg, "UC_AGC_MEMORY_LOAD_START_ADDRESS_MSB");
			break;
		case 0x2a:
			lstrcpy(szReg, "UC_AGC_MEMORY_LOAD_START_ADDRESS_LSB");
			break;
		case 0x2b:
			lstrcpy(szReg, "UC_AGC_PROGRAM_DOWNLOAD_CONTROL");
			break;
		case 0x2c:
			lstrcpy(szReg, "UC_AGC_DATA_TRANSFER");
			break;
		case 0x30:
			lstrcpy(szReg, "UC_GP_0");
			break;
		case 0x31:
			lstrcpy(szReg, "UC_GP_1");
			break;
		case 0x33:
			lstrcpy(szReg, "UC_GP_3");
			break;
		case 0x34:
			lstrcpy(szReg, "UC_GP_4");
			break;
		case 0x35:
			lstrcpy(szReg, "UC_GP_5");
			break;
		case 0x36:
			lstrcpy(szReg, "UC_GP_6");
			break;
		case 0x37:
			lstrcpy(szReg, "UC_GP_7");
			break;
		case 0x38:
			lstrcpy(szReg, "UC_GP_8");
			break;
		case 0x39:
			lstrcpy(szReg, "UC_GP_9");
			break;
		case 0x3a:
			lstrcpy(szReg, "UC_GP_10");
			break;
		case 0x3b:
			lstrcpy(szReg, "UC_GP_11");
			break;
		case 0x3c:
			lstrcpy(szReg, "UC_GP_12");
			break;
		case 0x3d:
			lstrcpy(szReg, "UC_GP_13");
			break;
		case 0x3e:
			lstrcpy(szReg, "UC_GP_14");
			break;
		case 0x41:
			lstrcpy(szReg, "AGC_CONTROL");
			break;
		case 0x42:
			lstrcpy(szReg, "AGC_ADC_TARGET_POWER_LEVEL");
			break;
		case 0x43:
			lstrcpy(szReg, "AGC_ADC_POWER_LPF_FC");
			break;
		case 0x44:
			lstrcpy(szReg, "AGC_ADC_POWER_DETECT_MSB");
			break;
		case 0x45:
			lstrcpy(szReg, "AGC_ADC_POWER_DETECT_LSB");
			break;
		case 0x46:
			lstrcpy(szReg, "AGC_GAIN_LOOP_BANDWIDTH");
			break;
		case 0x47:
			lstrcpy(szReg, "AGC_GAIN_DISTRIBUTION_LOOP_BANDWIDTH");
			break;
		case 0x48:
			lstrcpy(szReg, "AGC_LOOP_DAMPING_RATIOS");
			break;
		case 0x49:
			lstrcpy(szReg, "AGC_ACCUMULATOR1_MSB");
			break;
		case 0x4a:
			lstrcpy(szReg, "AGC_ACCUMULATOR1_LSB");
			break;
		case 0x4b:
			lstrcpy(szReg, "AGC_ACCUMULATOR2_MSB");
			break;
		case 0x4c:
			lstrcpy(szReg, "AGC_ACCUMULATOR2_LSB");
			break;
		case 0x4d:
			lstrcpy(szReg, "AGC_KG1");
			break;
		case 0x4e:
			lstrcpy(szReg, "AGC_KG2");
			break;
		case 0x4f:
			lstrcpy(szReg, "AGC_KD1");
			break;
		case 0x50:
			lstrcpy(szReg, "AGC_KD2");
			break;
		case 0x51:
			lstrcpy(szReg, "AGC_PDET_TARGET_POWER_LEVEL");
			break;
		case 0x52:
			lstrcpy(szReg, "AGC_PDET_PARAMETERS");
			break;
		case 0x53:
			lstrcpy(szReg, "AGC_PDET_POWER_MSB");
			break;
		case 0x54:
			lstrcpy(szReg, "AGC_PDET_POWER_LSB");
			break;
		case 0x55:
			lstrcpy(szReg, "AGC_SDM12_LPF_FC");
			break;
		case 0x56:
			lstrcpy(szReg, "AGC_SDMXA_LPF_FC");
			break;
		case 0x57:
			lstrcpy(szReg, "AGC_SDM_CONFIGURE");
			break;
		case 0x58:
			lstrcpy(szReg, "AGC_SDM1_INPUT_MSB");
			break;
		case 0x59:
			lstrcpy(szReg, "AGC_SDM1_INPUT_LSB");
			break;
		case 0x5a:
			lstrcpy(szReg, "AGC_SDM2_INPUT_MSB");
			break;
		case 0x5b:
			lstrcpy(szReg, "AGC_SDM2_INPUT_LSB");
			break;
		case 0x5c:
			lstrcpy(szReg, "AGC_SDMX_INPUT_MSB");
			break;
		case 0x5d:
			lstrcpy(szReg, "AGC_SDMX_INPUT_LSB");
			break;
		case 0x62:
			lstrcpy(szReg, "FE_TIMING_K1_MSB");
			break;
		case 0x63:
			lstrcpy(szReg, "FE_TIMING_K1_LSB");
			break;
		case 0x64:
			lstrcpy(szReg, "FE_TIMING_K2_MSB");
			break;
		case 0x65:
			lstrcpy(szReg, "FE_TIMING_K2_LSB");
			break;
		case 0x66:
			lstrcpy(szReg, "FE_TIMING_RATE_NOM_4");
			break;
		case 0x67:
			lstrcpy(szReg, "FE_TIMING_RATE_NOM_3");
			break;
		case 0x68:
			lstrcpy(szReg, "FE_TIMING_RATE_NOM_2");
			break;
		case 0x69:
			lstrcpy(szReg, "FE_TIMING_RATE_NOM_1");
			break;
		case 0x6a:
			lstrcpy(szReg, "FE_TIMING_RATE_NOM_0");
			break;
		case 0x6b:
			lstrcpy(szReg, "FE_TIMING_BAUD_RATE_OFFSET_3");
			break;
		case 0x6c:
			lstrcpy(szReg, "FE_TIMING_BAUD_RATE_OFFSET_2");
			break;
		case 0x6d:
			lstrcpy(szReg, "FE_TIMING_BAUD_RATE_OFFSET_1");
			break;
		case 0x6e:
			lstrcpy(szReg, "FE_TIMING_BAUD_RATE_OFFSET_0");
			break;
		case 0x6f:
			lstrcpy(szReg, "FE_PILOT_K1_MSB");
			break;
		case 0x70:
			lstrcpy(szReg, "FE_PILOT_K1_LSB");
			break;
		case 0x71:
			lstrcpy(szReg, "FE_PILOT_K2_MSB");
			break;
		case 0x72:
			lstrcpy(szReg, "FE_PILOT_K2_LSB");
			break;
		case 0x73:
			lstrcpy(szReg, "FE_PILOT_FREQUENCY_OFFSET_3");
			break;
		case 0x74:
			lstrcpy(szReg, "FE_PILOT_FREQUENCY_OFFSET_2");
			break;
		case 0x75:
			lstrcpy(szReg, "FE_PILOT_FREQUENCY_OFFSET_1");
			break;
		case 0x76:
			lstrcpy(szReg, "FE_PILOT_FREQUENCY_OFFSET_0");
			break;
		case 0x9b:
			lstrcpy(szReg, "EQ_NCONFIRM");
			break;
		case 0x9c:
			lstrcpy(szReg, "EQ_NLOCK");
			break;
		case 0x9d:
			lstrcpy(szReg, "EQ_BINARY_CONSTANT");
			break;
		case 0x9e:
			lstrcpy(szReg, "EQ_PN_CONSTANT");
			break;
		case 0x9f:
			lstrcpy(szReg, "EQ_DFS_STATE");
			break;
		case 0xa0:
			lstrcpy(szReg, "EQ_SNR");
			break;
		case 0xa1:
			lstrcpy(szReg, "EQ_CL_CONTROL");
			break;
		case 0xa6:
			lstrcpy(szReg, "EQ_CL_STAT_3");
			break;
		case 0xa7:
			lstrcpy(szReg, "EQ_CL_STAT_2");
			break;
		case 0xa8:
			lstrcpy(szReg, "EQ_CL_STAT_1");
			break;
		case 0xa9:
			lstrcpy(szReg, "EQ_CL_STAT_0");
			break;
		case 0xcc:
			lstrcpy(szReg, "EQ_TEST_MUX_SELECT");
			break;
		case 0xd1:
			lstrcpy(szReg, "FEC_TD_WINLEN");
			break;
		case 0xd2:
			lstrcpy(szReg, "FEC_TD_CONFIDENCE_COUNT");
			break;
		case 0xd3:
			lstrcpy(szReg, "FEC_TD_MAX_COUNT");
			break;
		case 0xd4:
			lstrcpy(szReg, "FEC_TD_METRIC_THRESHOLD_MSB");
			break;
		case 0xd5:
			lstrcpy(szReg, "FEC_TD_METRIC_THRESHOLD_LSB");
			break;
		case 0xd6:
			lstrcpy(szReg, "FEC_TD_INPUT_METRIC_LSB");
			break;
		case 0xd7:
			lstrcpy(szReg, "FEC_TD_INPUT_METRIC_MSB");
			break;
		case 0xd8:
			lstrcpy(szReg, "FEC_TD_OUTPUT_METRIC_LSB");
			break;
		case 0xd9:
			lstrcpy(szReg, "FEC_TD_OUTPUT_METRIC_MSB");
			break;
		case 0xdb:
			lstrcpy(szReg, "FEC_QAMFSD_STATUS");
			break;
		case 0xdc:
			lstrcpy(szReg, "FEC_QAMFSD_NVERLOCK");
			break;
		case 0xdd:
			lstrcpy(szReg, "FEC_QAMFSD_FLTR_COUNT");
			break;
		case 0xde:
			lstrcpy(szReg, "FEC_QAMFSD_INTERLEAVER_MODE");
			break;
		case 0xdf:
			lstrcpy(szReg, "FEC_QAMFSD_TRAILER_MSB");
			break;
		case 0xe0:
			lstrcpy(szReg, "FEC_QAMFSD_TRAILER_LSB");
			break;
		case 0xe1:
			lstrcpy(szReg, "FEC_QAMFSD_BAND_SYNCS_TOTAL_COUNT");
			break;
		case 0xe2:
			lstrcpy(szReg, "FEC_DI_MODE");
			break;
		case 0xe3:
			lstrcpy(szReg, "FEC_DI_CONFIG");
			break;
		case 0xe4:
			lstrcpy(szReg, "FEC_RS_MODE");
			break;
		case 0xe5:
			lstrcpy(szReg, "FEC_RS_NFRAME");
			break;
		case 0xe6:
			lstrcpy(szReg, "FEC_RS_ERROR_COUNT_MSB");
			break;
		case 0xe7:
			lstrcpy(szReg, "FEC_RS_ERROR_COUNT_LSB");
			break;
		case 0xe8:
			lstrcpy(szReg, "FEC_RS_UNCORRECTABLE_ERROR_COUNT");
			break;
		case 0xe9:
			lstrcpy(szReg, "FEC_MPEG_MODE_0");
			break;
		case 0xea:
			lstrcpy(szReg, "FEC_MPEG_MODE_1");
			break;
		case 0xeb:
			lstrcpy(szReg, "FEC_MPEG_STATUS");
			break;
		case 0xed:
			lstrcpy(szReg, "FEC_MPEG_NVERLOCK");
			break;
		case 0xee:
			lstrcpy(szReg, "FEC_MPEG_NPKTS_MSB");
			break;
		case 0xef:
			lstrcpy(szReg, "FEC_MPEG_NPKTS_LSB");
			break;
		case 0xf0:
			lstrcpy(szReg, "FEC_MPEG_NUMBER_OF_PACKET_ERRORS");
			break;
		case 0xf1:
			lstrcpy(szReg, "FEC_MPEG_BAD_CHECKSUM");
			break;
		default:
			lstrcpy(szReg, "RESERVED");
			break;
		}
	}
	else if (nAddress == 0x16)
	{
		switch(nRegister)
		{
		case 0x30:
			lstrcpy(szReg, "BERT_CTL_A");
			break;
		case 0x31:
			lstrcpy(szReg, "BERT_CTL_B");
			break;
		case 0x32:
			lstrcpy(szReg, "BERT_STATUS");
			break;
		case 0x33:
			lstrcpy(szReg, "BERT_WIN_SIZE");
			break;
		case 0x34:
			lstrcpy(szReg, "BERT_ERR_CNTR_3");
			break;
		case 0x35:
			lstrcpy(szReg, "BERT_ERR_CNTR_2");
			break;
		case 0x36:
			lstrcpy(szReg, "BERT_ERR_CNTR_1");
			break;
		case 0x37:
			lstrcpy(szReg, "BERT_ERR_CNTR_0");
			break;
		case 0x38:
			lstrcpy(szReg, "BERT_SYNC_THOLD");
			break;
		case 0x39:
			lstrcpy(szReg, "BERT_SYNC_ACQ_LOSS");
			break;
		case 0x80:
			lstrcpy(szReg, "SMOOTHER_CONTROL");
			break;
		case 0x81:
			lstrcpy(szReg, "SMOOTHER_TARGET");
			break;
		case 0x82:
			lstrcpy(szReg, "SMOOTHER_NOM_NCO_2");
			break;
		case 0x83:
			lstrcpy(szReg, "SMOOTHER_NOM_NCO_1");
			break;
		case 0x84:
			lstrcpy(szReg, "SMOOTHER_NOM_NCO_0");
			break;
		case 0x88:
			lstrcpy(szReg, "SMOOTHER_BANDWIDTH");
			break;
		case 0x89:
			lstrcpy(szReg, "SMOOTHER_MPEG_ERR_CNTR_1");
			break;
		case 0x8a:
			lstrcpy(szReg, "SMOOTHER_MPEG_ERR_CNTR_0");
			break;
		default:
			lstrcpy(szReg, "RESERVED");
			break;
		}
	}
	if (fCheckMode == FALSE)
	{
		while (lstrlen(szReg) < nRegNameLen)
			lstrcat(szReg, " ");
	}
}
 #endif _DEBUG
#endif ALPSTUNER

#ifdef SATELLITE_SOURCE
BYTE ReadIntersil()
{
	BYTE bOut[2];
	BYTE bIn[1];

	bOut[0] = outReadIntersil;
	bOut[1] = 0x10;

	if (!SendUSB(bOut, sizeof(bOut)))
		OutputDebugString("DTVWorks: Write for ReadIntersil failed\n");

	ReceiveUSB(bIn, sizeof(bIn));
	return bIn[0];
}

void WriteIntersil(BYTE bValue)
{
	BYTE bOut[3];

	bOut[0] = outWriteIntersil;
	bOut[1] = 0x10;
	bOut[2] = bValue;

	if (!SendUSB(bOut, sizeof(bOut)))
		OutputDebugString("DTVWorks: WriteIntersil failed\n");
}

void Setup22KHz(BOOL f22KHz)
{
	BYTE bOut[2];

	bOut[0] = out22KHzOnOff;
	bOut[1] = f22KHz;

	if (!SendUSB(bOut, sizeof(bOut)))
		OutputDebugString("DTVWorks: Write 22KHz tone control failed\n");
}

void SetTuner22KHzAndPower(BOOL f22KHz, BOOL fPower, int nPolarity)
{
	if (fPower == FALSE)
	{
		Setup22KHz(FALSE);
		WriteIntersil(ISEL1);
	}
	else
	{
		switch(nPolarity)
		{
		case 0:	// vertical
			WriteIntersil(ISEL1 | EN1);
			Setup22KHz(f22KHz);
			break;
		case 1:	// horizontal
			WriteIntersil(ISEL1 | VSEL1 | EN1);
			Setup22KHz(f22KHz);
			break;
		default: // power off
			WriteIntersil(ISEL1);
			break;
		}
	}
}

void SelectDiSEqCInput(int nInput)
{
	BYTE bPositionByte[] = {0xc0, 0xc4, 0xc8, 0xcc};

	{
		char szDebug[128];
		wsprintf(szDebug, "DTVWorks: SelectDiSEqCInput(%d)\n", nInput);
		OutputDebugString(szDebug);
	}
	nInput--;
	if ((nInput >= 0) && (nInput <= 3) )
	{
		BYTE bOut[6];

		bOut[0] = outDiSEqC;
		bOut[1] = 4;
		bOut[2] = 0xe0;		// master to slave no response
		bOut[3] = 0x10;		// address
		bOut[4] = 0x38;		// switch port
		bOut[5] = bPositionByte[nInput];
		SendUSB(bOut, sizeof(bOut));

		// Stall while the firmware sends the DiSEqC command. 12.5 ms per byte
		// plus 15 ms after all bytes
		Sleep((bOut[1] * 13) + 15 + 10);
	}
}
/*
void SelectDiSEqCInput(int nInput)
{
	BYTE bPositionByte[] = {0xc0, 0xc4, 0xc8, 0xcc};

	{
		char szDebug[128];
		wsprintf(szDebug, "DTVWorks: SelectDiSEqCInput(%d)\n", nInput);
		OutputDebugString(szDebug);
	}
	nInput--;
	if ((nInput >= 0) && (nInput <= 3) )
	{
		BYTE bOut[6];

		bOut[0] = outDiSEqC;
		bOut[1] = 3;
		bOut[2] = 0xe2;		// master to slave no response
		bOut[3] = 0x17;		// address
		bOut[4] = 0x26;		// switch port
		SendUSB(bOut, sizeof(bOut));

		// Stall while the firmware sends the DiSEqC command. 12.5 ms per byte
		// plus 15 ms after all bytes
		Sleep((bOut[1] * 13) + 15 + 10);
	}
}
*/
#endif SATELLITE_SOURCE

#ifdef EIGHTPSK
BOOL LastTuneDCIIOQPSK()
{
	BYTE bOut[1];
	BYTE bIn[2];

	bOut[0] = outGetConfiguration;
	if (!SendUSB(bOut, sizeof(bOut)))
		OutputDebugString("DTVWorks: Write for GetConfiguration failed\n");
	ReceiveUSB(bIn, sizeof(bIn));
	if (bIn[1] & 0x08)
		return TRUE;
	return FALSE;
}

WORD WriteAndReadDNIF(WORD wOut)
{
	WORD wRetVal;
	BYTE bOut[3];
	BYTE bIn[2] = {0, 0};

	bOut[0] = outDNHardware;
	bOut[1] = wOut >> 8;
	bOut[2] = wOut & 0xff;
	if (!SendUSB(bOut, sizeof(bOut)))
		OutputDebugString("?DNHardware out failed\n");
	else
		ReceiveUSB(bIn, sizeof(bIn));
	wRetVal = bIn[0] << 8 | bIn[1];
	return wRetVal;
}

void DoDNIF(HWND hDlgProgress, WORD wOut)
{
	WriteAndReadDNIF(wOut);
	SendMessage(hDlgProgress, PBM_STEPIT, 0, 0);
}

void SetupDNIF(HWND hDlg)
{
	HWND hDlgProgress = GetDlgItem(hDlg, IDC_BCM4500_PROGRESS);

	DoDNIF(hDlgProgress, 0x8000);

	DoDNIF(hDlgProgress, 0x0200);
	DoDNIF(hDlgProgress, 0x0200);
	DoDNIF(hDlgProgress, 0x0100);
	DoDNIF(hDlgProgress, 0x0100);
	DoDNIF(hDlgProgress, 0x0100);
	DoDNIF(hDlgProgress, 0x0100);
	DoDNIF(hDlgProgress, 0x0100);
	DoDNIF(hDlgProgress, 0x0100);
	Sleep(100);

	DoDNIF(hDlgProgress, 0x0201);	// 3V on after this
	DoDNIF(hDlgProgress, 0x0203); // CLK driven after this
	Sleep(100);

	DoDNIF(hDlgProgress, 0x0104);	// 
	Sleep(100);

	DoDNIF(hDlgProgress, 0x0100);	// I2C relay off
	DoDNIF(hDlgProgress, 0x0102);	// I2C relay on
	DoDNIF(hDlgProgress, 0x0103);	// I2C relay on and bus on
}

BOOL WriteBCM4500Memory(HWND hDlg)
{
	HANDLE hFile;
	DWORD dwRead;
	int i;
	BOOL fRetVal = TRUE;
	char szFirmwareFile[MAX_PATH];

	GetModuleFileName((HMODULE)hInstance, szFirmwareFile, sizeof(szFirmwareFile));
	for (i = lstrlen(szFirmwareFile); i > 0; i--)
	{
		if (szFirmwareFile[i] == '\\')
		{
			szFirmwareFile[i + 1] = 0;
			break;
		}
	}
#ifndef DVBTECH
	lstrcat(szFirmwareFile, "oasis_8psk_firmware.bin");
#else DVBTECH
	lstrcat(szFirmwareFile, "dvbtech_8psk_firmware.bin");
#endif DVBTECH
	//lstrcat(szFirmwareFile, "new-fw.bin");

	hFile = CreateFile(szFirmwareFile, GENERIC_READ, 0, (LPSECURITY_ATTRIBUTES) NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, (HANDLE) NULL);
	if (hFile != INVALID_HANDLE_VALUE)
	{
		BYTE bOut[64];
		BYTE * pBuffer = (BYTE *)LocalAlloc(LPTR, 0x4000);

		ReadFile(hFile, pBuffer, 0x4000, &dwRead, NULL);
		if (dwRead == 0x4000)
		{
			int nAddr = 0;
			int nRemaining = 0x4000;
			int nThisTime;
			int nProgress = 0;

			SendDlgItemMessage(hDlg, IDC_BCM4500_PROGRESS, PBM_SETRANGE32, 0, 0x4000);

			do
			{
				nThisTime = nRemaining;
				if (nThisTime > 60)
					nThisTime = 60;
				nProgress += nThisTime;
				SendDlgItemMessage(hDlg, IDC_BCM4500_PROGRESS, PBM_SETPOS, nProgress, 0);

				//wsprintf(szTemp, "DTVWorks: 0x%04x to 0x%04x\n", nAddr, nAddr + nThisTime);
				//OutputDebugString(szTemp);

				bOut[0] = outWriteBCM4500RAM;
				bOut[1] = nThisTime;	// count
				bOut[2] = (nAddr >> 8) & 0xff;
				bOut[3] = nAddr & 0xff;
				memcpy(&bOut[4], &pBuffer[nAddr], nThisTime);
				SendUSB(bOut, nThisTime + 4);
				nAddr += nThisTime;
				nRemaining -= nThisTime;
			} while (nRemaining);
		}
		else
		{
			OutputDebugString("DTVWorks: Failed to read 0x4000 bytes\n");
			fRetVal = FALSE;
		}

		LocalFree(pBuffer);
		CloseHandle(hFile);
		Sleep(100);
	}
	else
		fRetVal = FALSE;

	return fRetVal;
}

BOOL CALLBACK LoadBCM4500DlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	static BOOL fFirstTime;
	static BOOL fFirmwareOnly;

	switch(uMsg)
	{
	case WM_INITDIALOG:
		fFirmwareOnly = (BOOL)lParam;
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
		if (fFirmwareOnly == FALSE)
		{
			SetDlgItemText(hDlg, IDC_STATUS, "Setting up interface");
			SendDlgItemMessage(hDlg, IDC_BCM4500_PROGRESS, PBM_SETRANGE32, 0, 30);
			SendDlgItemMessage(hDlg, IDC_BCM4500_PROGRESS, PBM_SETSTEP, 1, 0);
			SetupDNIF(hDlg);
			SetupDNIF(hDlg);
		}
		SetDlgItemText(hDlg, IDC_STATUS, "Loading firmware");
		if (WriteBCM4500Memory(hDlg) == FALSE)
		{
			EndDialog(hDlg, FALSE);
			break;
		}	
		EndDialog(hDlg, TRUE);
		break;
	}
	return FALSE;
}
#endif EIGHTPSK

#ifdef ALPSTUNER
int TunerSetChan(int nFrequency)
{
	int nTunerFrequency;
	int nRetVal;
	BYTE bTunerBytes[4];

	nTunerFrequency = ((nFrequency + 44) * 10000) / 625;
	bTunerBytes[0] = (nTunerFrequency >> 8) & 0xff;		// programable divider MSB
	bTunerBytes[1] = nTunerFrequency & 0xff;	// programable divider LSB
	bTunerBytes[2] = 0x85;						// control data

	if (nFrequency < 162)
		bTunerBytes[3] = 0x01;					// charge pump current and port setting
	else if (nFrequency < 426)
		bTunerBytes[3] = 0x02;
	else if (nFrequency < 782)
		bTunerBytes[3] = 0x08;
	else
		bTunerBytes[3] = 0x88;

	nRetVal = NxtIicXfer(NULL, NXT_IIC_WRITE, NXT_IIC_SPEED_STANDARD, 4, 0xC2, bTunerBytes);
	return nRetVal;
}

#define STOP_ERROR 1
#define BYPASS_ERROR 2
#define TUNER_ERROR 3
#define UNBYPASS_ERROR 4
#define START_ERROR 5
#define TUNER_NTSC 6

int TuneFAT(int nFrequency, int nQAMMode)
{
	int tuneStatus;
	Data16 drvErr;
	Data16 result;
	int tuneErr; 
	
	// Stop current acquisition before tuning a new channel 
	drvErr = NxtStop(NULL, NXT_STOP_FAT);
	if (NXT_NO_ERROR != drvErr)
	{
		tuneStatus = STOP_ERROR;
	}
	else
	{
		// switch the host IIC to the auxiliary IIC 
		drvErr = NxtIicBypass(NULL, NXT_IIC_UC_CONTROL);
		if (NXT_NO_ERROR != drvErr)
		{
			tuneStatus = BYPASS_ERROR;
		}
		else
		{
			// use the tuner driver to set the channel 
			tuneErr = TunerSetChan(nFrequency);
			if (tuneErr)
			{
				tuneStatus = TUNER_ERROR;
			}
			else
			{
				// Switch the host IIC off the auxiliary IIC 
				drvErr = NxtIicBypass(NULL, NXT_IIC_UC_CONTROL);
				if (NXT_NO_ERROR != drvErr)
				{
					tuneStatus = UNBYPASS_ERROR;
				}
				else 
				{
					// Optionally seek the best antenna position 
					// drvErr = NxtennaSeekOptimum(NULL );
					// start digital channel acquisition 
#ifdef EIGHTVSB
					drvErr = NxtStart(NULL, NXT_CONFIG_8VSB, &result); 
#endif EIGHTVSB
#ifdef QAM
					drvErr = NxtStart(NULL, (NxtAcqOptions_t)nQAMMode, &result);  
#endif QAM
#ifdef EIGHTVSB
					if (NXT_NO_ERROR == drvErr && NXT_CONFIG_8VSB == result)
#endif EIGHTVSB
#ifdef QAM
					if (NXT_NO_ERROR == drvErr && (nQAMMode && result) )
#endif QAM
					{
						BOOL fLocked;
						NxtGetFatLockStatus(NULL, &fLocked);
						tuneStatus = !(fLocked == TRUE);

						if (fLocked == FALSE)
						{
							BOOL fNTSC;
							NxtGetNtscStatus(NULL, &fNTSC);
							if (fNTSC == TRUE)
								tuneStatus = TUNER_NTSC;
						}
					}
					else
					{
						tuneStatus = START_ERROR;
					}
				}
			}
		}
	}

	return tuneStatus;
}
#endif ALPSTUNER

#ifdef SATELLITE_SOURCE
void SetupDiSEqC()
{
	switch(ss->nDiSEqCInput)
	{
	case 0:
		{
			int i;
			for (i = 0; i < 1; i++)
			{
				BYTE bOut[2];
				BYTE bIn[2];

				bOut[0] = outDishSwitch;
				bOut[1] = 0x65;
				SendUSB(bOut, sizeof(bOut));
				
				do
				{
					Sleep(5);
					bOut[0] = outGetConfiguration;
					if (!SendUSB(bOut, sizeof(bOut)))
						OutputDebugString("DTVWorks: Write for GetConfiguarion failed\n");
					ReceiveUSB(bIn, sizeof(bIn));
				} while (bIn[1] & 2);
			}
		}
		break;
	case 1:
	case 2:
	case 3:
	case 4:
		Sleep(750);		// allow time for switch to reset
		SelectDiSEqCInput(ss->nDiSEqCInput);
		Sleep(100);
		break;
	case 5:	// Tone A
	case 6:	// Tone B
		{
			BYTE bOut[2];

			Sleep(750);
			bOut[0] = outToneBurst;
			bOut[1] = ss->nDiSEqCInput == 6;
			SendUSB(bOut, sizeof(bOut));
			Sleep(50);
		}
		break;
	default:
		{
			if (ss->nDiSEqCInput >= 7 && ss->nDiSEqCInput <= 20)
			{
				// Dish Network Switch
				BYTE bOut[2];
				static BYTE bDishBytes[] = {
					0x34, // SW21 Dish 1
					0x65, // SW21 Dish 2
					0x46, // SW42 Dish 1
					0x17, // SW42 Dish 2
					0x68, // SW44 Dish 2
					0x39, // SW64 Dish 1A
					0x1A, // SW64 Dish 1B
					0x4B, // SW64 Dish 2A
					0x5C, // SW64 Dish 2B
					0x0D, // SW64 Dish 3A
					0x2E, // SW64 Dish 3B
					0x72, // Twin LNB 1
					0x23, // Twin LNB 2
					0x51}; // Quad LNB 2

				Sleep(750);
				bOut[0] = outDishSwitch;
				bOut[1] = bDishBytes[ss->nDiSEqCInput - 7];
				SendUSB(bOut, sizeof(bOut));
				Sleep(125);
			}
		}
		break;
	}
}

#endif SATELLITE_SOURCE

void SetupLastTune()
{
	szLastSignalReport[0] = '\0';
#ifdef EIGHTPSK
	char szPolarity[4] = {"H/L"};
	char szModulation[16] = {0};

	if (ss->nPolarity == 0)
		lstrcpy(szPolarity, "V/R");
	switch(ss->nADVModulationMode)
	{
	case ADV_MOD_DVB_QPSK:
		lstrcpy(szModulation, "DVB QPSK");
		break;
	case ADV_MOD_TURBO_QPSK:
		lstrcpy(szModulation, "Turbo QPSK");
		break;
	case ADV_MOD_TURBO_8PSK:
		lstrcpy(szModulation, "8PSK");
		break;
	case ADV_MOD_TURBO_16QAM:
		lstrcpy(szModulation, "16QAM");
		break;
	case ADV_MOD_DCII_C_QPSK:
		lstrcpy(szModulation, "DC2C QPSK");
		break;
	case ADV_MOD_DCII_I_QPSK:
		lstrcpy(szModulation, "DC2I QPSK");
		break;
	case ADV_MOD_DCII_Q_QPSK:
		lstrcpy(szModulation, "DC2Q QPSK");
		break;
	case ADV_MOD_DCII_C_OQPSK:
		lstrcpy(szModulation, "DC2 OQPSK");
		break;
	}
	wsprintf(szLastTune, "%d MHz %s %d %s", ss->nFrequency, szPolarity, ss->nSymbolRate, szModulation);
#endif EIGHTPSK

#ifdef DVBS
	char szPolarity[4] = {"H/L"};
	char szModulation[16] = {0};

	if (ss->nPolarity == 0)
		lstrcpy(szPolarity, "V/R");
	switch(ss->nADVModulationMode)
	{
	case ADV_MOD_DVB_QPSK:
#ifndef DSS
		lstrcpy(szModulation, "DVB QPSK");
#else DSS
		lstrcpy(szModulation, "DSS QPSK");
#endif DSS
		break;
	}
	wsprintf(szLastTune, "%d MHz %s %d %s", ss->nFrequency, szPolarity, ss->nSymbolRate, szModulation);
#endif DVBS

#ifdef ALPSTUNER	
#ifdef QAM
	wsprintf(szLastTune, "Channel %d (%d MHz)", SourceHelper_GetQAMChannelFromFrequency(ss->nFrequency), ss->nFrequency);
#endif QAM
#ifdef EIGHTVSB
	wsprintf(szLastTune, "Channel %d (%d MHz)", SourceHelper_GetATSCChannelFromFrequency(ss->nFrequency), ss->nFrequency);
#endif EIGHTVSB
#endif ALPSTUNER
}

#ifdef CIELPLUS_SKY
void SendIRSequence(char * szSequence)
{
	int i;

	SendSkyRF(sky_sky);
	for (i = 0; i < lstrlen(szSequence); i++)
	{
		switch(szSequence[i])
		{
		case '0':
			SendSkyRF(sky_0);
			break;
		case '1':
			SendSkyRF(sky_1);
			break;
		case '2':
			SendSkyRF(sky_2);
			break;
		case '3':
			SendSkyRF(sky_3);
			break;
		case '4':
			SendSkyRF(sky_4);
			break;
		case '5':
			SendSkyRF(sky_5);
			break;
		case '6':
			SendSkyRF(sky_6);
			break;
		case '7':
			SendSkyRF(sky_7);
			break;
		case '8':
			SendSkyRF(sky_8);
			break;
		case '9':
			SendSkyRF(sky_9);
			break;
		case 'P':	// Power
			SendSkyRF(sky_power);
			break;
		case 'G':	// Guide
			SendSkyRF(sky_tv_guide);
			break;
		case 'B':	// Box Office
			SendSkyRF(sky_box_office);
			break;
		case 'S':	// Services
			SendSkyRF(sky_services);
			break;
		case 'I':	// Interactive
			SendSkyRF(sky_interactive);
			break;
		case 'H':	// Help
			SendSkyRF(sky_help);
			break;
		case 'b':	// Back Up
			SendSkyRF(sky_back_up);
			break;
		case 'i':	// info
			SendSkyRF(sky_info);
			break;
		case 'R':	// Red
			SendSkyRF(sky_red);
			break;
		case 'g':	// Green
			SendSkyRF(sky_green);
			break;
		case 'Y':	// Yellow
			SendSkyRF(sky_yellow);
			break;
		case 'l':	// Blue
			SendSkyRF(sky_blue);
			break;
		case 'U':	// Up
			SendSkyRF(sky_up);
			break;
		case '<':	// Left
			SendSkyRF(sky_left);
			break;
		case 's':	// Select
			SendSkyRF(sky_select);
			break;
		case '>':	// Right
			SendSkyRF(sky_right);
			break;
		case 'D':	// Down
			SendSkyRF(sky_down);
			break;
		case '+':	// +Channel
			SendSkyRF(sky_ch_up);
			break;
		case '-':	// -Channel
			SendSkyRF(sky_ch_down);
			break;
		}
	}
}

BOOL CALLBACK CielPlusSkyDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg)
	{
	case WM_INITDIALOG:
		{
			switch(nReceiverType)
			{
			case SKY_RECEIVER_PACE:
				CheckDlgButton(hDlg, IDC_CIEL_PLUS_PACE, BST_CHECKED);
				break;
			case SKY_RECEIVER_AMSTRAD:
				CheckDlgButton(hDlg, IDC_CIEL_PLUS_AMSTRAD, BST_CHECKED);
				break;
			}			
			SetFocus(GetDlgItem(hDlg, IDOK));
		}
		break;
	case WM_COMMAND:
		switch(LOWORD(wParam))
		{
		case IDCANCEL:
			EndDialog(hDlg, FALSE);
			break;
		case IDOK:
			fDontAskMode = IsDlgButtonChecked(hDlg, IDC_CIEL_PLUS_DONT_ASK);
			EndDialog(hDlg, TRUE);
			break;
		case IDC_CIEL_PLUS_PACE:
			if (IsDlgButtonChecked(hDlg, IDC_CIEL_PLUS_PACE))
				nReceiverType = SKY_RECEIVER_PACE;
			break;
		case IDC_CIEL_PLUS_AMSTRAD:
			if (IsDlgButtonChecked(hDlg, IDC_CIEL_PLUS_AMSTRAD))
				nReceiverType = SKY_RECEIVER_AMSTRAD;
			break;
		case IDC_SKY_1:
			SendSkyRF(sky_1);
			break;
		case IDC_SKY_2:
			SendSkyRF(sky_2);
			break;
		case IDC_SKY_3:
			SendSkyRF(sky_3);
			break;
		case IDC_SKY_4:
			SendSkyRF(sky_4);
			break;
		case IDC_SKY_5:
			SendSkyRF(sky_5);
			break;
		case IDC_SKY_6:
			SendSkyRF(sky_6);
			break;
		case IDC_SKY_7:
			SendSkyRF(sky_7);
			break;
		case IDC_SKY_8:
			SendSkyRF(sky_8);
			break;
		case IDC_SKY_9:
			SendSkyRF(sky_9);
			break;
		case IDC_SKY_0:
			SendSkyRF(sky_0);
			break;
		case IDC_SKY_PWR:
			SendSkyRF(sky_power);
			break;
		case IDC_SKY_GUIDE:
			SendSkyRF(sky_tv_guide);
			break;
		case IDC_SKY_BOX:
			SendSkyRF(sky_box_office);
			break;
		case IDC_SKY_SVCS:
			SendSkyRF(sky_services);
			break;
		case IDC_SKY_INT:
			SendSkyRF(sky_interactive);
			break;
		case IDC_SKY_HELP:
			SendSkyRF(sky_help);
			break;
		case IDC_SKY_BU:
			SendSkyRF(sky_back_up);
			break;
		case IDC_SKY_i:
			SendSkyRF(sky_info);
			break;
		case IDC_SKY_RED:
			SendSkyRF(sky_red);
			break;
		case IDC_SKY_GRN:
			SendSkyRF(sky_green);
			break;
		case IDC_SKY_YEL:
			SendSkyRF(sky_yellow);
			break;
		case IDC_SKY_BLU:
			SendSkyRF(sky_blue);
			break;
		case IDC_SKY_UP:
			SendSkyRF(sky_up);
			break;
		case IDC_SKY_DN:
			SendSkyRF(sky_down);
			break;
		case IDC_SKY_LEFT:
			SendSkyRF(sky_left);
			break;
		case IDC_SKY_RIGHT:
			SendSkyRF(sky_right);
			break;
		case IDC_SKY_SEL:
			SendSkyRF(sky_select);
			break;
		case IDC_SKY_CHUP:
			SendSkyRF(sky_ch_up);
			break;
		case IDC_SKY_CHDN:
			SendSkyRF(sky_ch_down);
			break;
		}
		break;
	case WM_CLOSE:
		EndDialog(hDlg, FALSE);
		break;
	}

	return FALSE;
}

void LoadCielPlusSkySettings()
{
	DWORD dwDisposition;
	DWORD dwDataSize;
	DWORD dwType;
	LONG lKey;
	HKEY hkMainReg;

	lKey = RegCreateKeyEx(HKEY_CURRENT_USER,
		                  gszCielPlusSkyKeyName,
						  0,
						  gszSourceName,
					      REG_OPTION_NON_VOLATILE,
						  KEY_ALL_ACCESS,
						  NULL,
						  &hkMainReg,
						  &dwDisposition);
	if (lKey == ERROR_SUCCESS)
	{
		if (dwDisposition != REG_CREATED_NEW_KEY)
		{
			dwDataSize = sizeof(nReceiverType);
			RegQueryValueEx(hkMainReg, "ReceiverType", NULL, &dwType, (BYTE *)&nReceiverType, &dwDataSize);
			dwDataSize = sizeof(fDontAskMode);
			RegQueryValueEx(hkMainReg, "DontAskMode", NULL, &dwType, (BYTE *)&fDontAskMode, &dwDataSize);
		}
		RegCloseKey(hkMainReg);
	}
}

void SaveCielPlusSkySettings()
{
	LONG lKey;
	HKEY hkMainReg;
	DWORD dwDisposition;

	lKey = RegCreateKeyEx(HKEY_CURRENT_USER,
		                  gszCielPlusSkyKeyName,
						  0,
						  gszSourceName,
					      REG_OPTION_NON_VOLATILE,
						  KEY_ALL_ACCESS,
						  NULL,
						  &hkMainReg,
						  &dwDisposition);
	if (lKey == ERROR_SUCCESS)
	{
		RegSetValueEx(hkMainReg, "ReceiverType", 0, REG_DWORD, (BYTE *)&nReceiverType, sizeof(DWORD));
		RegSetValueEx(hkMainReg, "DontAskMode", 0, REG_DWORD, (BYTE *)&fDontAskMode, sizeof(DWORD));
		RegCloseKey(hkMainReg);
	}
}
#endif CIELPLUS_SKY

#ifdef CIELPLUS_5000
void SendDishRF(int nCommand)
{
	BYTE bOut[3];

	bOut[0] = 28;
	bOut[1] = nCommand >> 8;
	bOut[2] = nCommand & 0xff;
	SendUSB(bOut, sizeof(bOut));
	Sleep(600);
}
#endif CIELPLUS_5000

//#define _DEBUG_DISEQC
#ifdef _DEBUG_DISEQC

BOOL TSReader_SendDiSEqC(BYTE * bCommand, int nLength);

BOOL CALLBACK DebugDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	static BOOL f22k = FALSE;
	static BOOL fHorz = FALSE;

	switch(uMsg)
	{
	case WM_INITDIALOG:
		CheckDlgButton(hDlg, IDC_14V, BST_CHECKED);
		SetTuner22KHzAndPower(f22k, TRUE, fHorz);
		break;
	case WM_CLOSE:
		EndDialog(hDlg, FALSE);
		break;
	case WM_COMMAND:
		switch(LOWORD(wParam))
		{
		case IDCANCEL:
			EndDialog(hDlg, FALSE);
			break;
		case IDC_22K:
			f22k = IsDlgButtonChecked(hDlg, IDC_22K);
			SetTuner22KHzAndPower(f22k, TRUE, fHorz);
			break;
		case IDC_14V:
		case IDC_18V:
			fHorz = IsDlgButtonChecked(hDlg, IDC_14V);
			SetTuner22KHzAndPower(f22k, TRUE, fHorz);
			break;
		case IDOK:
			{
				int i;
				int nByteCounter = 0;
				BYTE bByteSequence[128];
				char szSendString[256];

				GetDlgItemText(hDlg, IDC_DISEQC_COMMAND, szSendString, sizeof(szSendString));
				for (i = 0; i < lstrlen(szSendString); i += 2)
				{
					unsigned int nByte;
					char szTemp[4];

					szTemp[0] = szSendString[i];
					szTemp[1] = szSendString[i + 1];
					szTemp[2] = '\0';
					sscanf(szTemp, "%x", &nByte);
					bByteSequence[nByteCounter++] = nByte;
					if (szSendString[i + 2] == ' ')
						i++;
				}
				if (nByteCounter)
				{
					ss->n22KHz = f22k;
					ss->nPolarity = fHorz;
					TSReader_SendDiSEqC(bByteSequence, nByteCounter);
				}
			}
			break;
		}
		break;
	}

	return FALSE;
}
#endif _DEBUG_DISEQC

BOOL TSReader_Tune()
{
#ifdef EIGHTPSK
	BOOL StatusSuccess = FALSE;
	int nLBand;
	int nBytesReceived;
	DWORD dwLockTime;
	DWORD dwLockTimeout = 3000;
	DWORD dwFrequency, dwSymbolRate;
	BYTE bOut[16];
	BYTE bIn[1];

	if (ss->nADVModulationMode == ADV_MOD_DVBS2)
	{
		MessageBox(ss->hWndTSReader, "DVB-S2 is not supported by this device", gszSourceName, MB_ICONSTOP);
		return FALSE;
	}

	SetupLastTune();
	if (LastTuneDCIIOQPSK() == TRUE && ss->nADVModulationMode != ADV_MOD_DCII_C_OQPSK)
	{
		OutputDebugString("DTVWorks: Reload firmware\n");
		DialogBoxParam((HINSTANCE)hInstance, MAKEINTRESOURCE(IDD_LOAD_BCM4500), NULL, LoadBCM4500DlgProc, (LPARAM)FALSE);
	}

	if (fPowerOn == FALSE)
	{
		// Turn on power but no 22KHz yet
		SetTuner22KHzAndPower(FALSE, TRUE, ss->nPolarity);
		fPowerOn = TRUE;
		Sleep(50);
	}

	SetupDiSEqC();

	// Now turn on 22KHz as appropriate
	SetTuner22KHzAndPower(ss->n22KHz, TRUE, ss->nPolarity);

#ifdef _DEBUG_DISEQC
	DialogBox((HINSTANCE)hInstance, MAKEINTRESOURCE(IDD_DEBUG), NULL, DebugDlgProc);
#endif _DEBUG_DISEQC

	// Calculate l-band, freq in hz and SR in symbols
	if (ss->nFrequency > ss->nLNBFrequency)
		nLBand = ss->nFrequency - ss->nLNBFrequency;
	else
		nLBand = ss->nLNBFrequency - ss->nFrequency;
	dwFrequency = nLBand * 1000000;
	dwSymbolRate = ss->nSymbolRate * 1000;

	// Send the tune command
	bOut[0] = outTune;
	bOut[1] = (BYTE)(dwFrequency >> 24);
	bOut[2] = (BYTE)(dwFrequency >> 16);
	bOut[3] = (BYTE)(dwFrequency >> 8);
	bOut[4] = (BYTE)(dwFrequency & 0xff);
	bOut[5] = (BYTE)(dwSymbolRate >> 24);
	bOut[6] = (BYTE)(dwSymbolRate >> 16);
	bOut[7] = (BYTE)(dwSymbolRate >> 8);
	bOut[8] = (BYTE)(dwSymbolRate & 0xff);
	bOut[9] = ss->nADVModulationMode;
	bOut[10] = ss->nCodeRate;
	SendUSB(bOut, 11);
	
	/*{
		BYTE byte2 = 0x0c;		// 0x08 = parallel data out 0x04 = MPEG2
		BYTE byte3 = 0x01;		// symbol rate #1
		BYTE ctl_flags1 = 0x10;	// ERR/SYNC/VALID all active high CLKINV/CLKSUP
		BYTE ctl_flags2 = 0x01;
		BYTE ctl_flags3 = 0xeb;

		
		byte2 |= 0x80;	// DSS scan
		ctl_flags3 &= ~0x04;
		
		bOut[9] = ADV_MOD_DSS_QPSK;
		bOut[11] = byte2;
		bOut[12] = byte3;
		bOut[13] = ctl_flags1;
		bOut[14] = ctl_flags2;
		bOut[15] = ctl_flags3;

		SendUSB(bOut, 16);
	}*/

	// Get status back 
	nBytesReceived = ReceiveUSB(bIn, sizeof(bIn));
	if (nBytesReceived != sizeof(bIn))
	{
		char szTemp[128];
		wsprintf(szTemp, "DTVWorks: Wrong size back after outTune command - got %d bytes\n", nBytesReceived);
		OutputDebugString(szTemp);
		return FALSE;
	}
	if (bIn[0] != 0)
	{
		char szTemp[128];
		wsprintf(szTemp, "DTVWorks: outTune command returned 0x%02x\n", bIn[0]);
		OutputDebugString(szTemp);
		return FALSE;
	}

	dwLockTime = GetTickCount();	
	while (!StatusSuccess)
	{
		BYTE bOut[1];
		BYTE bIn[3];

		DWORD dwCountNow = GetTickCount() - dwLockTime;
		if (dwCountNow > dwLockTimeout)
		{
			OutputDebugString("DTVWorks: Tune Timeout\n");
			break;
		}

		bOut[0] = outLockStatus;
		SendUSB(bOut, sizeof(bOut));
		ReceiveUSB(bIn, sizeof(bIn));
		StatusSuccess = bIn[0];
		if (!StatusSuccess)
			Sleep(1);
	}

	if (!StatusSuccess)
	{
		if (ss->fQuietMode == FALSE)
		{
			MessageBox(ss->hWndTSReader, "Failed to lock signal", gszSourceName, MB_ICONWARNING);
			return FALSE;
		}
	}

	return StatusSuccess;
#endif EIGHTPSK

#ifdef ALPSTUNER
	int nTuneFATStatus;

	SetupLastTune();
#ifdef EIGHTVSB
	nTuneFATStatus = TuneFAT(ss->nFrequency, 0);
#endif EIGHTVSB
#ifdef QAM
	// Try 64QAM first
	nTuneFATStatus = TuneFAT(ss->nFrequency, NXT_CONFIG_64QAM);
	if (!nTuneFATStatus)
	{
		//OutputDebugString("64QAM ");
		return TRUE;
	}
	if (nTuneFATStatus == TUNER_NTSC)
		return -1;
	
	// Now try 256QAM 
	nTuneFATStatus = TuneFAT(ss->nFrequency, NXT_CONFIG_256QAM);
	if (!nTuneFATStatus)
	{
		//OutputDebugString("256QAM ");
		return TRUE;
	}
	if (nTuneFATStatus == TUNER_NTSC)
		return -1;
	
	// 256QAM one more time (can take forever to lock)
	nTuneFATStatus = TuneFAT(ss->nFrequency, NXT_CONFIG_256QAM);
#endif QAM

	if (nTuneFATStatus)
	{
		if (ss->fQuietMode == FALSE)
			MessageBox(ss->hWndTSReader, "Failed to lock signal", gszSourceName, MB_ICONWARNING);
		if (nTuneFATStatus == TUNER_NTSC)
			return -1;
		return FALSE;
	}
	//OutputDebugString("256QAM ");
	return TRUE;
#endif ALPSTUNER

#ifdef SPI
#ifndef HORIZON

	/*
	case outSwitchSyncAsync:
		// For older TSReader which selects between
		// only two modes
		switch(EP1OUTBUF[1])
		{
		case 0:
			FIFOPINPOLAR = 0x04;
			IFCONFIG = 0xCB;	// slave FIFO interface to 48MHz internal with IFCLK off IFCLK normal
			IOA |= 0x08;			// async mode
			break;
		case 1:
			// Sync mode
			FIFOPINPOLAR = 0x00;
			IFCONFIG = 0x43;	// slave FIFO interface to 48MHz external with IFCLK off IFCLK not inverted
			IOA &= ~0x08;			// sync mode
			break;
		}
	case outSwitchSyncAsyncExtended:
		// For latest TSR's with full mode support
		FIFOPINPOLAR = EP1OUTBUF[1];
		IFCONFIG = EP1OUTBUF[2];	// slave FIFO interface to 48MHz internal with IFCLK off IFCLK normal
		if (EP1OUTBUF[3])
			IOA |= 0x08;			// async mode
		else
			IOA &= ~0x08;			// sync mode
		break;
#endif //SPI

  */
	// Select sync mode. Two ways depending on the firmware revision
	if (!fFullSyncAsyncControlFirmware)
	{
		// Old way where just the mode could be selected
		BYTE bOut[2];
		bOut[0] = outSwitchSyncAsync;
		bOut[1] = fSPISyncMode;
		SendUSB(bOut, 2);
	}
	else
	{
		BYTE bOut[2];
		bOut[0] = outSwitchSyncAsync;
		bOut[1] = fSPISyncMode;
		if (fInvertSPIClock)
			bOut[1] &= 2;
		SendUSB(bOut, 2);
	}
#endif HORIZON
	if (ss->fSerialReceiverControlEnabled && !fDontTune)
	{
		if (SourceHelper_TuneSerialControl(szLastTune) == FALSE)
			return FALSE;
	}
	return TRUE;
#endif SPI

#ifdef DVBS
	BOOL StatusSuccess = FALSE;
	int nLBand;
	int nBytesReceived;
	DWORD dwLockTime;
	DWORD dwLockTimeout = 3000;
	DWORD dwFrequency, dwSymbolRate;
	BYTE bOut[6];
	BYTE bIn[8];

	SetupLastTune();
	if (fPowerOn == FALSE)
	{
		// Turn on power but no 22KHz yet
		SetTuner22KHzAndPower(FALSE, TRUE, ss->nPolarity);
		fPowerOn = TRUE;
		Sleep(50);
	}

	SetupDiSEqC();

	// Now turn on 22KHz as appropriate
	SetTuner22KHzAndPower(ss->n22KHz, TRUE, ss->nPolarity);

	// Calculate l-band, freq in hz and SR in symbols
	if (ss->nFrequency > ss->nLNBFrequency)
		nLBand = ss->nFrequency - ss->nLNBFrequency;
	else
		nLBand = ss->nLNBFrequency - ss->nFrequency;
	dwFrequency = nLBand;
	dwSymbolRate = ss->nSymbolRate;

	// Send the tune command
	bOut[0] = outTune;
	bOut[1] = (BYTE)(dwFrequency >> 8);
	bOut[2] = (BYTE)(dwFrequency & 0xff);
	bOut[3] = (BYTE)(dwSymbolRate >> 8);
	bOut[4] = (BYTE)(dwSymbolRate & 0xff);
#ifndef DSS
	bOut[5] = ADV_MOD_DVB_QPSK;
#else DSS
	bOut[5] = ADV_MOD_DSS_QPSK;
#endif DSS
	SendUSB(bOut, 11);

	// Get status back 
	nBytesReceived = ReceiveUSB(bIn, sizeof(bIn));
	if (nBytesReceived != sizeof(bIn))
	{
		char szTemp[128];
		wsprintf(szTemp, "DTVWorks: Wrong size back after outTune command - got %d byte(s)\n", nBytesReceived);
		OutputDebugString(szTemp);
		return FALSE;
	}
	if (bIn[0] != 0)
	{
		char szTemp[128];
		wsprintf(szTemp, "DTVWorks: outTune command returned 0x%02x", bIn[0]);
		OutputDebugString(szTemp);
		return FALSE;
	}

	dwLockTime = GetTickCount();	
	while (!StatusSuccess)
	{
		BYTE bOut[1];
		BYTE bIn[3];

		DWORD dwCountNow = GetTickCount() - dwLockTime;
		if (dwCountNow > dwLockTimeout)
		{
			OutputDebugString("DTVWorks: Tune Timeout\n");
			break;
		}

		bOut[0] = outLockStatus;
		SendUSB(bOut, sizeof(bOut));
		ReceiveUSB(bIn, sizeof(bIn));
		StatusSuccess = bIn[0];
		if (!StatusSuccess)
			Sleep(1);
	}

	if (!StatusSuccess)
	{
		if (ss->fQuietMode == FALSE)
		{
			MessageBox(ss->hWndTSReader, "Failed to lock signal", gszSourceName, MB_ICONWARNING);
			return FALSE;
		}
	}

	return StatusSuccess;
#endif DVBS
#ifdef CIELPLUS_SKY
	{
		BYTE bOut[3];
		BYTE bFIFOPinpolar;

		switch(nReceiverType)
		{
		case SKY_RECEIVER_PACE:
			bFIFOPinpolar = 0x04;
			break;
		case SKY_RECEIVER_AMSTRAD:
			bFIFOPinpolar = 0x00;
			break;
		}

		bOut[0] = 15;
		bOut[1] = 0xcb;
		bOut[2] = bFIFOPinpolar;
		SendUSB(bOut, 3);

		if (lstrlen(szRFSequence))
		{
			SendIRSequence(szRFSequence);
			Sleep(2000);	// give it time to tune there...
		}
	}
	return TRUE;
#endif CIELPLUS_SKY
#ifdef CIELPLUS_5000
	if (lstrlen(szRFSequence))
	{
		int i;

		for (i = 0; i < lstrlen(szRFSequence); i++)
		{
			switch(szRFSequence[i])
			{
			case '0':
				SendDishRF(0xBBFF);
				break;
			case '1':
				SendDishRF(0xEFFF);
				break;
			case '2':
				SendDishRF(0xEBFF);
				break;
			case '3':
				SendDishRF(0xE7FF);
				break;
			case '4':
				SendDishRF(0xDFFF);
				break;
			case '5':
				SendDishRF(0xDBFF);
				break;
			case '6':
				SendDishRF(0xD7FF);
				break;
			case '7':
				SendDishRF(0xCFFF);
				break;
			case '8':
				SendDishRF(0xCBFF);
				break;
			case '9':
				SendDishRF(0xC7FF);
				break;
			}
		}
		Sleep(500);	// give it time to tune there...
	}
	return TRUE;
#endif CIELPLUS_5000
}

void EnableDMA(BOOL fEnable)
{
	BYTE bOut[2];

	bOut[0] = outDMAControl;
	bOut[1] = fEnable;

	if (!SendUSB(bOut, sizeof(bOut)))
		OutputDebugString("DTVWorks: EnableDMA failed\n");
}

void CheckLockStatus()
{
#ifdef DVBS
	if (nTunerStatusTimer++ > 10)
	{
		int nErrCnt;
		double dBER;
		double dDivisor = 2097152.0;
		BYTE bOut[1];
		BYTE bIn[3];
		char szLockStatus[16];

		bOut[0] = outLockStatus;
		SendUSB(bOut, sizeof(bOut));
		ReceiveUSB(bIn, sizeof(bIn));

		nErrCnt = bIn[1] << 8 | bIn[2];
		if (nErrCnt > 8192)
			nErrCnt = 8192;
		dBER = (double)nErrCnt / dDivisor;

		if (bIn[0])
			lstrcpy(szLockStatus, "Locked");
		else
			lstrcpy(szLockStatus, "Unlocked");

		EnterCriticalSection(&csSignal);			
		sprintf(szLastSignalReport, "%s: BER %0.1E", szLockStatus, dBER);
		LeaveCriticalSection(&csSignal);
		nTunerStatusTimer = 0;
	}
#endif DVBS
#ifdef EIGHTPSK
	if (nTunerStatusTimer++ > 100)
	{
		double fSNR;
		BYTE bOut[1];
		BYTE bIn[3];
		char szLockStatus[16];

		bOut[0] = outLockStatus;
		SendUSB(bOut, sizeof(bOut));
		ReceiveUSB(bIn, sizeof(bIn));

		fSNR = double(bIn[1] << 8 | bIn[2]) / 256.0;
		if (bIn[0])
			lstrcpy(szLockStatus, "Locked");
		else
			lstrcpy(szLockStatus, "Unlocked");
		EnterCriticalSection(&csSignal);			
		sprintf(szLastSignalReport, "%s: SNR %.1f dB", szLockStatus, fSNR);
		LeaveCriticalSection(&csSignal);
		nTunerStatusTimer = 0;
	}
#endif EIGHTPSK
#ifdef ALPSTUNER
	if (nTunerStatusTimer++ > 100)
	{
		int nRetVal;
		int nFatSQI;
		int winSize;
		BOOL fLocked;
		double dClusterVar;
		double dSNRdB;
		//NxtSignalState_t FatSignalState;
		//Data8 FatSignalMetric;
		NxtModFormat_t FatModFormat;
		BYTE data;

		//char szTemp[256];
		//char szSignalState[24];
		char szModFormat[8];

		NxtGetFatLockStatus(NULL, &fLocked);

		/*nRetVal = NxtGetFatSignalStatus(NULL, &FatSignalState, &FatSignalMetric);
		switch(FatSignalState)
		{
		case NXT_SIG_NO_SIGNAL:
			lstrcpy(szSignalState, "NXT_SIG_NO_SIGNAL");
			break;
		case NXT_SIG_WEAK:
			lstrcpy(szSignalState, "NXT_SIG_WEAK");
			break;
		case NXT_SIG_MODERATE:
			lstrcpy(szSignalState, "NXT_SIG_MODERATE");
			break;
		case NXT_SIG_STRONG:
			lstrcpy(szSignalState, "NXT_SIG_STRONG");
			break;
		case NXT_SIG_VERY_STRONG:
			lstrcpy(szSignalState, "NXT_SIG_VERY_STRONG");
			break;
		default:
			lstrcpy(szSignalState, "BAD RETURN VALUE");
			break;
		}

		wsprintf(szTemp, "DTVWorks: NxtGetFatSignalStatus returned %d with FatSignalState = %s and FatSignalMetric = %d SQI=%d\n",
						 nRetVal, szSignalState, FatSignalMetric, nFatSQI);
		OutputDebugString(szTemp);
*/
		nRetVal = NxtGetFatSQI(NULL, &nFatSQI);
		dClusterVar = 32767.0 - (double)nFatSQI;
		NxtGetRegister(NULL, 0xA0, 1, &data);
		winSize = ((data >> 6) & 0x03);
		winSize = (1<<9) * (1 << winSize);
		NxtGetFatModFormat(NULL, &FatModFormat);
		switch(FatModFormat)
		{
		case NXT_256QAM:
			lstrcpy(szModFormat, "256QAM");
			dSNRdB = 10.0 * log10(18.852014541 * (double)winSize / dClusterVar);
			break;
		case NXT_64QAM:
			lstrcpy(szModFormat, "64QAM");
			dSNRdB = 10.0 * log10(18.630226135 * (double)winSize / dClusterVar);
			break;
		case NXT_16VSB:
			lstrcpy(szModFormat, "16VSB");
			dSNRdB = 10.0 * log10(20.238192081 * (double)winSize / dClusterVar);
			break;
		case NXT_8VSB:
			lstrcpy(szModFormat, "8VSB");
			dSNRdB = 10.0 * log10(20.016403675 * (double)winSize / dClusterVar);
			break;
		}
		nTunerStatusTimer = 0;

		EnterCriticalSection(&csSignal);			
		if (fLocked)
			sprintf(szLastSignalReport, "Locked %s SNR %.1f dB", szModFormat, dSNRdB);
		else
			lstrcpy(szLastSignalReport, "Unlocked");
		LeaveCriticalSection(&csSignal);
	}
#endif ALPSTUNER
#ifdef SPI
	if (ss->fSerialReceiverControlEnabled)
	{
		char szTemp[128];
		if (SourceHelper_SerialControlGetSignal(szTemp) == TRUE)
		{
			EnterCriticalSection(&csSignal);			
			lstrcpy(szLastSignalReport, szTemp);
			LeaveCriticalSection(&csSignal);
		}
	}
#endif SPI
}

#define READ_FROM_FX2_SIZE 128 * 1024
//#define _DEBUGx
DWORD WINAPI ReadFX2Thread(LPVOID lpv)
{
	int nReadPtr = 0;
	int nCurrentBuffer;
	BOOL fFirstPacket = TRUE;
	BOOL fRestart = TRUE;
#ifdef _DEBUGx
	HANDLE hDebug;
	hDebug = CreateFile("c:\\tsreader.ts", GENERIC_WRITE, 0, (LPSECURITY_ATTRIBUTES)NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, (HANDLE)NULL);
#endif _DEBUGx

	OutputDebugString("DTVWorks: +ReadFX2Thread\n");

	nTunerStatusTimer = 65535;
	ss->fReadThreadTerminated = FALSE;
	ss->fTerminateReadThread = FALSE;
#ifdef DSS
	SourceHelper_StartSyncThread(ss, TRUE);
#else DSS
	SourceHelper_StartSyncThread(ss, FALSE);
#endif DSS

RestartFX2DataThread:
	fRestart = FALSE;
	LONG nTransferLength = READ_FROM_FX2_SIZE;

#ifdef SATELLITE_SOURCE
	if (ss->nSymbolRate < 10000)
	{
		nTransferLength /= 4;
		if (ss->nSymbolRate < 5000)
			nTransferLength /= 2;
	}
#endif SATELLITE_SOURCE
	MPEGInEpt->SetXferSize(nTransferLength);

	// Setup the asynchronous transfer buffers
	int nQueueSize = 32;
	PUCHAR *buffers = new PUCHAR[nQueueSize];
	PUCHAR *contexts = new PUCHAR[nQueueSize];
	OVERLAPPED * inMPEGOvLap = new OVERLAPPED[nQueueSize];

	for (nCurrentBuffer = 0; nCurrentBuffer < nQueueSize; nCurrentBuffer++)
	{ 
       buffers[nCurrentBuffer] = new UCHAR[nTransferLength];
	   inMPEGOvLap[nCurrentBuffer].Internal = inMPEGOvLap[nCurrentBuffer].InternalHigh = 0;
	   inMPEGOvLap[nCurrentBuffer].Offset = inMPEGOvLap[nCurrentBuffer].OffsetHigh = 0;
	   inMPEGOvLap[nCurrentBuffer].hEvent = CreateEvent(NULL, false, false, NULL);
	}

	// Queue-up the first batch of transfer requests
	EnableDMA(TRUE);
	for (nCurrentBuffer = 0; nCurrentBuffer < nQueueSize; nCurrentBuffer++)
	   contexts[nCurrentBuffer] = MPEGInEpt->BeginDataXfer(buffers[nCurrentBuffer], nTransferLength, &inMPEGOvLap[nCurrentBuffer]);

	nCurrentBuffer = 0;
	//OutputDebugString("DTVWorks: enter main read loop\n");
	while (!ss->fTerminateReadThread)
	{
		LONG nReceiveLength = 0;
		
		if (!MPEGInEpt->WaitForXfer(&inMPEGOvLap[nCurrentBuffer], 2500))
		{
			char szTemp[128];
			wsprintf(szTemp, "DTVWorks: WaitForXfer() timed out buffer = %d ***********\n", nCurrentBuffer);
			OutputDebugString(szTemp);
			fRestart = TRUE;
			break;
		}

		if (MPEGInEpt->FinishDataXfer(buffers[nCurrentBuffer], nReceiveLength, &inMPEGOvLap[nCurrentBuffer],
			contexts[nCurrentBuffer]))
		{
			if (fFirstPacket == TRUE)
				fFirstPacket = FALSE;
			else
			{
				SourceHelper_SyncData(buffers[nCurrentBuffer], nReceiveLength);
#ifdef _DEBUGx
				{
					DWORD dwWritten = 0;
					WriteFile(hDebug, buffers[nCurrentBuffer], nReceiveLength, &dwWritten, NULL);
				}
#endif _DEBUGx
			}
		}
		else
			Sleep(1);

	    contexts[nCurrentBuffer] = MPEGInEpt->BeginDataXfer(buffers[nCurrentBuffer], nTransferLength, &inMPEGOvLap[nCurrentBuffer]);
		nCurrentBuffer++;
		if (nCurrentBuffer == nQueueSize)
			nCurrentBuffer = 0;
		CheckLockStatus();
	}
	OutputDebugString("DTVWorks: left main read loop\n");
	nTunerStatusTimer = 65535; CheckLockStatus();

	MPEGInEpt->Abort();
	for (nCurrentBuffer = 0; nCurrentBuffer < nQueueSize; nCurrentBuffer++)
	{
		// Wait for all the queued requests to be cancelled 
		LONG nReceiveLength = 0;

		MPEGInEpt->FinishDataXfer(buffers[nCurrentBuffer], nReceiveLength, &inMPEGOvLap[nCurrentBuffer], contexts[nCurrentBuffer]);
		CloseHandle(inMPEGOvLap[nCurrentBuffer].hEvent);
		delete [] buffers[nCurrentBuffer];
	}

	delete [] buffers;
	delete [] contexts;
	delete [] inMPEGOvLap;

	EnableDMA(FALSE);
	
	if (fRestart == TRUE && !ss->fTerminateReadThread)
		goto RestartFX2DataThread;

	SourceHelper_StopSyncThread();
	ss->fReadThreadTerminated = TRUE;
	ss->fTerminateReadThread = FALSE;
	EnterCriticalSection(&ss->csTSBuffersInUse);
	ss->nTSBuffersInUse = -1000;
	LeaveCriticalSection(&ss->csTSBuffersInUse);
	CloseHandle(ss->hReadDataThread);
#ifdef _DEBUGx
	CloseHandle(hDebug);
#endif _DEBUGx
	OutputDebugString("DTVWorks: -ReadFX2Thread\n");
	return 0;
}

BOOL TSReader_Start()
{
	DWORD dwThreadID;

	OutputDebugString("DTVWorks: enter Start()\n");

	if (ss->fSerialReceiverControlEnabled)
		SourceHelper_SerialControlStart();

	ss->hReadDataThread = CreateThread(NULL, 0, ReadFX2Thread, (LPVOID)0, CREATE_SUSPENDED, &dwThreadID);
	SourceHelper_SetWorkerThreadPriorities(FALSE);
SetThreadPriority(ss->hReadDataThread, THREAD_PRIORITY_TIME_CRITICAL);
OutputDebugString("DTVWorks: ReadFX2Thread at THREAD_PRIORITY_TIME_CRITICAL\n");
	ResumeThread(ss->hReadDataThread);

	OutputDebugString("DTVWorks: leave Start()\n");
	return TRUE;
}

BOOL TSReader_Stop()
{
	OutputDebugString("DTVWorks: enter Stop()\n");

	ss->fTerminateReadThread = TRUE;
	while (ss->fReadThreadTerminated == FALSE)
		Sleep(50);

	if (ss->fSerialReceiverControlEnabled)
		SourceHelper_SerialControlStop();

#ifdef CIELPLUS_SKY
	SendSkyRF(sky_sky);
#endif CIELPLUS_SKY
	OutputDebugString("DTVWorks: leave Stop()\n");
	return TRUE;
}

BOOL OpenUSBDriver()
{
	int d = 0;
	GUID OASISguid;
	char szFoundDevices[512] = {0};

	OASISguid.Data1 = 0xFA58C45D;
	OASISguid.Data2 = 0x5B19; OASISguid.Data3 = 0x428b;
	OASISguid.Data4[0] = 0xA2; OASISguid.Data4[1] = 0xD1; OASISguid.Data4[2] = 0x27; OASISguid.Data4[3] = 0x07;
	OASISguid.Data4[4] = 0x85; OASISguid.Data4[5] = 0x6D; OASISguid.Data4[6] = 0x7E; OASISguid.Data4[7] = 0x19;
	
	USBDevice = new CCyUSBDevice(NULL, OASISguid);
    int nDevices = USBDevice->DeviceCount(); 
	if (nDevices == 0)
	{
#ifndef DVBTECH
 #ifndef HORIZON
		MessageBox(NULL, "Unable to locate any DTVWorks interfaces", gszSourceName, MB_ICONSTOP);
 #else HORIZON
		MessageBox(NULL, "Unable to locate any Horizon interfaces", gszSourceName, MB_ICONSTOP);
 #endif HORIZON
#else DVBTECH
		MessageBox(NULL, "Unable to locate any DVBTech interfaces", gszSourceName, MB_ICONSTOP);
#endif DVBTECH
		return FALSE;
	}

    do
	{
		BYTE bOut[1];
		BYTE bIn[2] = {0, 0};
		char szTemp[128];

		USBDevice->Open(d);   // Open automatically  calls Close() if necessary 

		int vID = USBDevice->VendorID; 
        int pID  = USBDevice->ProductID;

		if (vID != 0x14ac)	// must be a DTVWorks/COOLSTF product
			goto LoopForNextUSBDevice;

		if (USBDevice->BulkOutEndPt == NULL)
		{
			OutputDebugString("DTVWorks: Couldn't find control out endpoint\n");
			return FALSE;
		}

		bOut[0] = outGetConfiguration;
		if (!SendUSB(bOut, sizeof(bOut)))
			OutputDebugString("DTVWorks: Write for GetConfiguration failed\n");
		ReceiveUSB(bIn, sizeof(bIn));
		{
			char szTemp[128];
			wsprintf(szTemp, "DTVWorks: VID = %04x PID = %04x bIn[0] = %02x bIn[1] = %02x\n", vID, pID, bIn[0], bIn[1]);
			OutputDebugString(szTemp);
		}
		if (pID == (TARGET_PRODUCT_ID | (ss->nSourceIndex << 12)))
			break;
		
		// Not one of the types we want - but keep track
		wsprintf(szTemp, "DTVWorks %s (Device ID %d)\n", USBDevice->DeviceName, pID >> 12 & 0x0f);
		if (vID == 0x14ac)
			lstrcat(szFoundDevices, szTemp);
LoopForNextUSBDevice:
		USBDevice->Close();
		d++;
    } while (d < nDevices);

	if (d == nDevices)
	{
		char szTargetName[128];
		char szTemp[1024];

		wsprintf(szTargetName, "%s (Device ID %d)", gszSourceName, ss->nSourceIndex);
		wsprintf(szTemp, "Located these interface(s):\n\n%s\nbut none of these are the model you've selected:\n\n%s", szFoundDevices, szTargetName);
		MessageBox(NULL, szTemp, gszSourceName, MB_ICONSTOP);
		return FALSE;
	}

	if (USBDevice->EndPoints == NULL)
	{
		OutputDebugString("DTVWorks: NULL endpoint from the USB device\n");
		return FALSE;
	}

	return TRUE;
}

#ifdef ALPSTUNER
BOOL InitNXT2004Driver()
{
	Data16 drvErr;

	drvErr = NxtInit2004Driver(NULL);
	if (NXT_NO_ERROR != drvErr)
		return FALSE;
	fNXT2004InitDone = TRUE;

	// set FAT AGC data
	drvErr = NxtSetFatAgcData(NULL, fatAgcData);
	if (NXT_NO_ERROR != drvErr)
		return FALSE;

	// set FAT AGC Polarity
	drvErr = NxtSetFatAgcSdmPolarity(NULL, INVERTED, INVERTED, NON_INVERTED, INVERTED);
	if (NXT_NO_ERROR != drvErr)
		return FALSE;

	// set MPEG modes
 /*DRV2_API Data16 NxtSetMpegMode(void *pContext,
					Bool bGatedOutputEnable,
					Bool bContinuousRateEnable,
					Bool bParallelOutputEnable,
					Bool bHeaderEnable)	*/
#ifdef EIGHTVSB
	drvErr = NxtSetMpegMode(NULL, TRUE, TRUE, FALSE, TRUE);
#endif EIGHTVSB
#ifdef QAM
	drvErr = NxtSetMpegMode(NULL, TRUE, FALSE, FALSE, TRUE);
#endif QAM
	if (NXT_NO_ERROR != drvErr)
		return FALSE;

	// set MPEG polarities
/*DRV2_API Data16 NxtSetMpegPolarity(void *pContext,
					NxtPolarity_t dataEnablePolarity,
					NxtPolarity_t pktSyncPolarity,
					NxtPolarity_t errorPolarity,
					NxtPolarity_t clockPolarity)*/
#ifdef EIGHTVSB
	drvErr = NxtSetMpegPolarity(NULL, NON_INVERTED, NON_INVERTED, NON_INVERTED, NON_INVERTED);
#endif EIGHTVSB
#ifdef QAM
	drvErr = NxtSetMpegPolarity(NULL, NON_INVERTED, NON_INVERTED, NON_INVERTED, NON_INVERTED);
#endif QAM
	if (NXT_NO_ERROR != drvErr)
		return FALSE;
	
	return TRUE;
}
#endif ALPSTUNER

BOOL InitHardware()
{
	int nTimeout = 5;

	if (OpenUSBDriver() == FALSE)
		return FALSE;

#ifdef EIGHTPSK
	// Try sending a get status command to see if the hardware is alive
	do
	{
		BYTE bOut[1];
		BYTE bIn[2];

		bOut[0] = outGetConfiguration;
		if (!SendUSB(bOut, sizeof(bOut)))
			OutputDebugString("DTVWorks: Write for GetConfiguration failed\n");
		ReceiveUSB(bIn, sizeof(bIn));
		if ((bIn[1] & 4) == 4)
			break;	// board is ready to go
		if (DialogBoxParam((HINSTANCE)hInstance, MAKEINTRESOURCE(IDD_LOAD_BCM4500), NULL, LoadBCM4500DlgProc, (LPARAM)FALSE) == FALSE)
		{
			MessageBox(ss->hWndTSReader, "Unable to open the firmware file", gszSourceName, MB_ICONSTOP);
			return FALSE;
		}
		Sleep(100);
	} while (nTimeout--);

	return (nTimeout > 0) & 1;
#endif EIGHTPSK

#ifdef ALPSTUNER
	if (InitNXT2004Driver() == FALSE)
		return FALSE;
	if (NxtCoreControl(NULL, CORE_POWER_UP_ALL))
		return FALSE;
#endif ALPSTUNER

	return TRUE;
}

void CloseEP1OvelapppedEvents()
{
	CloseHandle(outOvLap.hEvent); outOvLap.hEvent = NULL;
	CloseHandle(inOvLap.hEvent);  inOvLap.hEvent = NULL;
}

BOOL TSReader_Init(PSOURCESTRUCT pss)
{
	int nIndex;
	int nEndPointCount;
	BYTE bOut[1];
	BYTE bIn[2];

#ifdef SPI
	fFullSyncAsyncControlFirmware = FALSE;
#endif SPI
	fDoneDeInit = FALSE;

	if (SourceHelper_ValidateSourceContainer(pss) == FALSE)
		return FALSE;
	OutputDebugString("DTVWorks: Init\n");
	InitializeCriticalSection(&csSignal);

	ss = pss;
#ifdef SATELLITE_SOURCE
	fPowerOn = FALSE;
#endif SATELLITE_SOURCE

	outOvLap.hEvent = CreateEvent(NULL, false, false, "CYUSB_OUT"); 
	inOvLap.hEvent = CreateEvent(NULL, false, false, "CYUSB_IN"); 

	if (InitHardware() == FALSE)
		return FALSE;

	// Find the USB pipe we use for data transfer (endpoint IN 2)
	nEndPointCount = USBDevice->EndPointCount();  
	for (nIndex = 1; nIndex < nEndPointCount; nIndex++)
	{
#ifndef HAMASAKI
		if (USBDevice->EndPoints[nIndex]->Address == 0x82)
#else HAMASAKI
		if (USBDevice->EndPoints[nIndex]->Address == 0x86)
#endif HAMASAKI
		{
			MPEGInEpt = (CCyBulkEndPoint *)USBDevice->EndPoints[nIndex];
			break;
		}
	}
	if (nIndex == nEndPointCount)
	{
		OutputDebugString("DTVWorks: Couldn't find MPEG in endpoint\n");
		CloseEP1OvelapppedEvents();
		return FALSE;
	}

	// Make sure we have a USB 2.0 connection
	bOut[0] = outGetConfiguration;
	if (!SendUSB(bOut, sizeof(bOut)))
		OutputDebugString("DTVWorks: Write for GetConfiguarion failed\n");
	ReceiveUSB(bIn, sizeof(bIn));
	if ((bIn[1] & 1) == 0)
	{
		MessageBox(NULL, "Interface is not connected to a USB 2.0 port", gszSourceName, MB_ICONSTOP);
		return FALSE;
	}
#ifdef SPI
	if ((bIn[1] & 0x80) == 0x80)
		fFullSyncAsyncControlFirmware = TRUE;
//	fFullSyncAsyncControlFirmware = FALSE;
#endif SPI

	return TRUE;
}

BOOL TSReader_DeInit()
{
	OutputDebugString("DTVWorks: +DeInit\n");

	if (fDoneDeInit)
		return TRUE;

#ifdef ALPSTUNER
	if (fNXT2004InitDone == TRUE)
		NxtExitDriver(NULL);
#endif ALPSTUNER

	CloseEP1OvelapppedEvents();

	if (USBDevice->EndPoints != NULL)
	{
		delete USBDevice; USBDevice = NULL;
	}

	DeleteCriticalSection(&csSignal);
	
	OutputDebugString("DTVWorks: -DeInit\n");
	fDoneDeInit = TRUE;
	return TRUE;
}

#ifdef SPI
#ifndef HORIZON
BOOL CALLBACK SPIModeDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg)
	{
	case WM_INITDIALOG:
		if (fSPISyncMode == FALSE)
			CheckDlgButton(hDlg, IDC_SPI_ASYNC, BST_CHECKED);
		else
			CheckDlgButton(hDlg, IDC_SPI_SYNC, BST_CHECKED);
		if (!fFullSyncAsyncControlFirmware)
			ShowWindow(GetDlgItem(hDlg, IDC_SPI_INVERT_CLOCK), SW_HIDE);
		else
			CheckDlgButton(hDlg, IDC_SPI_INVERT_CLOCK, fInvertSPIClock);
		SetFocus(GetDlgItem(hDlg, IDOK));
		break;
	case WM_CLOSE:
		EndDialog(hDlg, FALSE);
		break;
	case WM_COMMAND:
		switch(LOWORD(wParam))
		{
		case IDOK:
			fSPISyncMode = IsDlgButtonChecked(hDlg, IDC_SPI_SYNC);
			fDontAskMode = IsDlgButtonChecked(hDlg, IDC_SPI_DONT_ASK);
			if (fFullSyncAsyncControlFirmware)
				fInvertSPIClock = IsDlgButtonChecked(hDlg, IDC_SPI_INVERT_CLOCK);
			EndDialog(hDlg, TRUE);
			break;
		case IDCANCEL:
			EndDialog(hDlg, FALSE);
			break;
		}
		break;
	}

	return FALSE;
}

void LoadSPISettings()
{
	DWORD dwDisposition;
	DWORD dwDataSize;
	DWORD dwType;
	LONG lKey;
	HKEY hkMainReg;

	lKey = RegCreateKeyEx(HKEY_CURRENT_USER,
		                  gszSPIKeyName,
						  0,
						  gszSourceName,
					      REG_OPTION_NON_VOLATILE,
						  KEY_ALL_ACCESS,
						  NULL,
						  &hkMainReg,
						  &dwDisposition);
	if (lKey == ERROR_SUCCESS)
	{
		if (dwDisposition != REG_CREATED_NEW_KEY)
		{
			dwDataSize = sizeof(fSPISyncMode);
			RegQueryValueEx(hkMainReg, "SPISyncMode", NULL, &dwType, (BYTE *)&fSPISyncMode, &dwDataSize);
			dwDataSize = sizeof(fDontAskMode);
			RegQueryValueEx(hkMainReg, "DontAskMode", NULL, &dwType, (BYTE *)&fDontAskMode, &dwDataSize);
			if (fFullSyncAsyncControlFirmware)
			{
				dwDataSize = sizeof(fInvertSPIClock);
				RegQueryValueEx(hkMainReg, "InvertSPIClock", NULL, &dwType, (BYTE *)&fInvertSPIClock, &dwDataSize);
			}
		}
		RegCloseKey(hkMainReg);
	}
}

void SaveSPISettings()
{
	LONG lKey;
	HKEY hkMainReg;
	DWORD dwDisposition;

	lKey = RegCreateKeyEx(HKEY_CURRENT_USER,
		                  gszSPIKeyName,
						  0,
						  gszSourceName,
					      REG_OPTION_NON_VOLATILE,
						  KEY_ALL_ACCESS,
						  NULL,
						  &hkMainReg,
						  &dwDisposition);
	if (lKey == ERROR_SUCCESS)
	{
		RegSetValueEx(hkMainReg, "SPISyncMode", 0, REG_DWORD, (BYTE *)&fSPISyncMode, sizeof(DWORD));
		RegSetValueEx(hkMainReg, "DontAskMode", 0, REG_DWORD, (BYTE *)&fDontAskMode, sizeof(DWORD));
		if (fFullSyncAsyncControlFirmware)
			RegSetValueEx(hkMainReg, "InvertSPIClock", 0, REG_DWORD, (BYTE *)&fInvertSPIClock, sizeof(DWORD));
		RegCloseKey(hkMainReg);
	}
}
#endif HORIZON
#endif SPI


BOOL TSReader_TuneDialog(HWND hWnd)
{
#ifdef EIGHTPSK
	OutputDebugString("DTVWorks: TSReader_TuneDialog (8PSK)\n");

	if (ss->fDontTune == TRUE)
		fNeedTuneDialog = FALSE;

	if (fNeedTuneDialog)
	{
		ss->fQuietMode = FALSE;
		OutputDebugString("DTVWorks: TSReader_TuneDialog tuning dialog is required\n");
		if (SourceHelper_ADVTuneDialog(hWnd) == FALSE)
			return FALSE;
	}
	else
	{
		OutputDebugString("DTVWorks: TSReader_TuneDialog tune NOT required\n");
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
#endif EIGHTPSK

#ifdef EIGHTVSB
	OutputDebugString("DTVWorks: TSReader_TuneDialog (8VSB)\n");

	if (ss->fDontTune == TRUE)
		fNeedTuneDialog = FALSE;

	if (fNeedTuneDialog)
	{
		OutputDebugString("DTVWorks: TSReader_TuneDialog tuning dialog is required\n");
		ss->fQuietMode = FALSE;
		if (SourceHelper_ATSCTuneDialog(hWnd) == FALSE)
			return FALSE;
	}
	else
	{
		OutputDebugString("DTVWorks: TSReader_TuneDialog tune NOT required\n");
		ss->nFrequency = nFrequency;
		fNeedTuneDialog = TRUE;
	}
#endif EIGHTVSB

#ifdef QAM
	OutputDebugString("DTVWorks: TSReader_TuneDialog (QAM)\n");

	if (ss->fDontTune == TRUE)
		fNeedTuneDialog = FALSE;

	if (fNeedTuneDialog)
	{
		OutputDebugString("DTVWorks: TSReader_TuneDialog tuning dialog is required\n");
		ss->fQuietMode = FALSE;
		if (SourceHelper_QAMTuneDialog(hWnd) == FALSE)
			return FALSE;
	}
	else
	{
		OutputDebugString("DTVWorks: TSReader_TuneDialog tune NOT required\n");
		ss->nFrequency = nFrequency;
		fNeedTuneDialog = TRUE;
	}
#endif QAM

#ifdef DVBS
	OutputDebugString("DTVWorks: TSReader_TuneDialog\n");

	if (ss->fDontTune == TRUE)
		fNeedTuneDialog = FALSE;

	if (fNeedTuneDialog)
	{
		ss->fQuietMode = FALSE;
		OutputDebugString("DTVWorks: TSReader_TuneDialog tuning dialog is required\n");
#ifndef DSS
		if (SourceHelper_DVBSTuneDialog(hWnd) == FALSE)
#else DSS
		if (SourceHelper_DSSTuneDialog(hWnd) == FALSE)
#endif DSS
			return FALSE;
	}
	else
	{
		OutputDebugString("DTVWorks: TSReader_TuneDialog tune NOT required\n");
		ss->nFrequency = nFrequency;
		ss->nPolarity = nPolarity;
		ss->nSymbolRate = nSymbolRate;
		ss->nLNBFrequency = nLNBFrequency;
		ss->n22KHz = n22KHz;
		ss->nDiSEqCInput = nDiSEqCInput;
		fNeedTuneDialog = TRUE;
	}
#endif DVBS
#ifdef SPI
	if (fNeedTuneDialog)
	{
#ifndef HORIZON
		LoadSPISettings();
		if (fDontAskMode == FALSE)
		{
			if (DialogBox((HINSTANCE)hInstance, MAKEINTRESOURCE(IDD_SPI_MODE), ss->hWndTSReader, SPIModeDlgProc) == FALSE)
				return FALSE;
		}
		SaveSPISettings();
#endif HORIZON
		if (ss->fSerialReceiverControlEnabled)
		{
			fDontTune = FALSE;
			if (SourceHelper_ADVTuneDialog(hWnd) == FALSE)
				fDontTune = TRUE;
		}
	}
#endif SPI

#ifdef CIELPLUS_SKY
	LoadCielPlusSkySettings();
	if (!lstrlen(szRFSequence))
	{
		if (fDontAskMode == FALSE)
		{
			if (DialogBox((HINSTANCE)hInstance, MAKEINTRESOURCE(IDD_CIELPLUS), ss->hWndTSReader, CielPlusSkyDlgProc) == FALSE)
				return FALSE;
			SaveCielPlusSkySettings();
		}
	}
#endif CIELPLUS_SKY
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
		*fCanBeStopped = FALSE;
	if (nMaxPIDs != NULL)
		*nMaxPIDs = 8192;
#ifdef EIGHTPSK
	if (szCommandLineParameters != NULL)
		lstrcpy(szCommandLineParameters, "freq pol sr lnbf 22khz mode fec {input}");	
	if (dwCapabilities != NULL)
		*dwCapabilities = CAPABILITIES_POWER
		                | CAPABILITIES_DISEQC
						| CAPABILITIES_TONEBURST
						| CAPABILITIES_DISEQC_POSITIONER
						| CAPABILITIES_DISH_SWITCH
#ifdef DVBTECH
		                | CAPABILITIES_MULTICARD
#endif DVBTECH
						| CAPABILITIES_ADV_SATELLITE;
#endif EIGHTPSK

#ifdef ALPSTUNER
	if (szCommandLineParameters != NULL)
		lstrcpy(szCommandLineParameters, "freq");	
	if (dwCapabilities != NULL)
		*dwCapabilities = 0;
#endif ALPSTUNER

#ifdef SPI
	if (szCommandLineParameters != NULL)
#ifndef HORIZON
		lstrcpy(szCommandLineParameters, "async=0/sync=1");	
#else HORIZON
		lstrcpy(szCommandLineParameters, "None");	
#endif HORIZON
	if (dwCapabilities != NULL)
		*dwCapabilities = CAPABILITIES_SERIAL_CONTROL
		                | CAPABILITIES_MULTICARD;
#endif SPI

#ifdef CIELPLUS_SKY
	if (szCommandLineParameters != NULL)
		lstrcpy(szCommandLineParameters, "channel");	
	if (dwCapabilities != NULL)
		*dwCapabilities = CAPABILITIES_TUNE_BY_CHANNEL;
#endif CIELPLUS_SKY

#ifdef CIELPLUS_5000
	if (szCommandLineParameters != NULL)
		lstrcpy(szCommandLineParameters, "channel");	
	if (dwCapabilities != NULL)
		*dwCapabilities = CAPABILITIES_TUNE_BY_CHANNEL;
#endif CIELPLUS_5000

#ifdef DVBS
	if (szCommandLineParameters != NULL)
		lstrcpy(szCommandLineParameters, "freq pol sr lnbf 22khz {input}");	
	if (dwCapabilities != NULL)
		*dwCapabilities = CAPABILITIES_POWER
		                | CAPABILITIES_DISEQC
						| CAPABILITIES_TONEBURST
						| CAPABILITIES_DISEQC_POSITIONER
						| CAPABILITIES_DISH_SWITCH;
#endif DVBS
	return TRUE;
}

BOOL TSReader_ParseCommandLine(PSOURCESTRUCT ss, char * szCommandLine, BOOL fQuiet)
{
#ifdef EIGHTPSK
	if (lstrlen(szCommandLine))
	{
		int nConversionCount = 0;

		SourceHelper_ConvertPolarity(szCommandLine);
		ss->nDiSEqCInput = 0;
		nConversionCount = sscanf(szCommandLine,
								  "%d %d %d %d %d %d %d %d", 
								  &nFrequency,
								  &nPolarity,
								  &nSymbolRate,
								  &nLNBFrequency,
								  &n22KHz,
								  &nADVModulationMode,
								  &nCodeRate,
								  &nDiSEqCInput);
		if (nConversionCount < 7)
		{
			if (!fQuiet)
				MessageBox(NULL,
					   "Usage for this source: freq pol sr lnbf 22khz mode fec {input}\n"
					   "\n"
					   "freq = frequency to tune\n"
					   "pol = 0 vertical/RHCP 1 horizontal/LHCP\n"
					   "sr = symbol rate\n"
					   "lnbf = LNB frequency\n"
					   "22k = 22KHz tone enable\n"
					   "mode = modulation mode (see readme)\n"
					   "FEC = code rate selection (see readme)\n"
					   "input = select DiSEqC input number (1-4) - optional",
					   gszSourceName,
					   MB_OK | MB_ICONSTOP);
			return FALSE;
		}
		fNeedTuneDialog = FALSE;
	}
	else
		fNeedTuneDialog = TRUE;
#endif EIGHTPSK

#ifdef ALPSTUNER
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
#ifdef EIGHTVSB
			nFrequency = SourceHelper_GetFrequencyFromATSCChannel(nFrequency);
#endif EIGHTVSB
#ifdef QAM
			nFrequency = SourceHelper_GetFrequencyFromQAMChannel(nFrequency);
#endif QAM
		fNeedTuneDialog = FALSE;
	}
	else
		fNeedTuneDialog = TRUE;
#endif ALPSTUNER

#ifdef DVBS
	if (lstrlen(szCommandLine))
	{
		int nConversionCount = 0;
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
		fNeedTuneDialog = FALSE;
	}
	else
		fNeedTuneDialog = TRUE;

#endif DVBS
#ifdef SPI
	fNeedTuneDialog = TRUE;
#ifndef HORIZON
	if (lstrlen(szCommandLine))
	{
		int nConversionCount;

		nConversionCount = sscanf(szCommandLine,
								  "%d", 
								  &fSPISyncMode);
		if (nConversionCount < 1)
		{
			if (!fQuiet)
				MessageBox(NULL,
					   "Usage for this source: SyncMode\n"
					   "\n"
					   "SyncMode = 0 for up to 60 Mbps or 1 for above 50 Mbps",
					   gszSourceName,
					   MB_OK | MB_ICONSTOP);
			return FALSE;
		}
		fNeedTuneDialog = FALSE;
	}
#endif HORIZON
#endif SPI

#ifdef CIELPLUS_SKY
	szRFSequence[0] = '\0';
	if (lstrlen(szCommandLine))
		lstrcpy(szRFSequence, szCommandLine);
#endif CIELPLUS_SKY

#ifdef CIELPLUS_5000
	szRFSequence[0] = '\0';
	if (lstrlen(szCommandLine))
		lstrcpy(szRFSequence, szCommandLine);
#endif CIELPLUS_5000
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
#ifdef SATELLITE_SOURCE
	if (nLength == 0 || bCommand == NULL)
	{
		char szTemp[128];
		wsprintf(szTemp, "FX2: Intersil = 0x%02x\n", ReadIntersil());
		OutputDebugString(szTemp);
	}
	else
	{
		int nIndex;
		BYTE bOut[128];

		if ((ReadIntersil() & EN1) == 0)
			WriteIntersil(ISEL1 | EN1);		// 12v on

		bOut[0] = outDiSEqC;
		bOut[1] = nLength;

		for (nIndex = 0; nIndex < nLength; nIndex++)
			bOut[nIndex + 2] = bCommand[nIndex];

		SendUSB(bOut, 2 + nLength);

		// Stall while the firmware sends the DiSEqC command. 12.5 ms per byte
		// plus 15 ms after all bytes
		Sleep((nLength * 13) + 15 + 10);

		// In case we're locked, set the 22KHz and polarity back the way it was before the DiSEqC command
		SetTuner22KHzAndPower(ss->n22KHz, TRUE, ss->nPolarity);
	}
#endif SATELLITE_SOURCE
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
		hInstance = (HINSTANCE)hModule;
#ifdef ALPSTUNER
 #ifdef _DEBUG
		{
			int i;
			fCheckMode = TRUE;
			for (i = 0; i < 256; i++)
			{
				char szTemp[128];

				DecodeNXT2004RegisterName(i, 0x14, szTemp);
				if (lstrlen(szTemp) > nRegNameLen)
					nRegNameLen = lstrlen(szTemp);
				DecodeNXT2004RegisterName(i, 0x16, szTemp);
				if (lstrlen(szTemp) > nRegNameLen)
					nRegNameLen = lstrlen(szTemp);
			}
			nRegNameLen++;
			fCheckMode = FALSE;
			fNXT2004InitDone = FALSE;
		}
 #endif _DEBUG
#endif ALPSTUNER
		break;
    case DLL_PROCESS_DETACH:
		break;
    }
    return TRUE;
}




/*
Dish Network RF

  ON always 435.6 us
  sequence 55.5 ms

  ON for 1 bit
  OFF for 6.1ms

*/
