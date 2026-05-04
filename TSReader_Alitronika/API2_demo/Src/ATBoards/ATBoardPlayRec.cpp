/*! Time-stamp: <@(#)ATBoardPlayRec.cpp   07/05/2007 - 15:46:04   P.Hoogervorst>
*********************************************************************
*  @file   : ATBoardPlayRec.cpp
*
*  Project : ATDemoApp. Testappliction for Alitronika devices.
*			  Supports Linux and windows operating System.
*
*			: Devices supported (USB and PCI)
*				-AT20, AT200
*				-AT30, AT30R1, AT300
*				-AT4, AT40, AT40X, AT40R1, AT400
*
*  Package : 
*
*  Company : Engineering Spirit
*
*  Author  : P.Hoogervorst                              Date: 07/05/2007
*
*  Purpose : Implementation of methods for class 
*
*********************************************************************
* Version History:
*
* V 0.10  07/05/2007  BN : First Revision
*
*********************************************************************
*/

/**********************************************************************************************************/
// includes
#include "../Sys/SysFunc.h"

#include "ATBoardPlayRec.h"

/**********************************************************************************************************/
// defines

/**********************************************************************************************************/
// functions
// constructor
CATBoardPlayRec::CATBoardPlayRec(void)
{
	m_pAtBoard = NULL;
	m_pDevName = NULL;
}

// destructor
CATBoardPlayRec::~CATBoardPlayRec(void)
{
	BoardClose();
}

/*
 * open an ATboard
 */
BOOL CATBoardPlayRec::BoardOpen(SBoardOpenParams BoardOpenParams)
{
	CAtDeviceList iDevices;
	CAtBoardManager *pMan = CAtBoardManager::Instance();

	// Get the device list.
	pMan->GetDeviceList(iDevices);

	// Set the board used status in the ATDeviceList
	// only if board is not a demo board

	if (BoardOpenParams.m_BoardPos >= 0)
		iDevices.SetBoardUsed(&BoardOpenParams, TRUE);

	m_pAtBoard = pMan->GetBoard(&BoardOpenParams);

	if (m_pAtBoard == NULL) 
		return FALSE;

	// program the default FPGA file
	ProgFpga(PROG_FPGA_DEFAULT, IOMODE_DVB);

	return TRUE;
}

/*
 * close an ATboard
 */
BOOL CATBoardPlayRec::BoardClose(void)
{
	CAtDeviceList iDevices;
	CAtBoardManager *pMan = CAtBoardManager::Instance();

	// no board valid
	if (m_pAtBoard == NULL) 
		return FALSE;
	
	pMan->ReleaseBoard(m_pAtBoard);
	
	// Get the device list.
	pMan->GetDeviceList(iDevices);

	// Reset the board used status in the ATDeviceList
	// only if board is not a demo board
	if (m_pAtBoard->m_BoardOpenParams.m_BoardPos >= 0)
		iDevices.SetBoardUsed(&m_pAtBoard->m_BoardOpenParams, FALSE);

	m_pAtBoard = NULL;

	return TRUE;
}

/*
 * Get the pointer to the current opened ATboard
 */
CATBoard* CATBoardPlayRec::GetCurBoard(void)
{
	// return pointer to board
	return m_pAtBoard;
}

/*
 * initialize play
 */
BOOL CATBoardPlayRec::InitPlay(u32			nFileBitrate,
							   BOOL			bORemux,
							   u32			nOutputBitrate,
							   EIOSel		OSel,
							   EIOMode		OMode,
							   EIOSpiMode	OSpiMode,
							   ETSPSize		OTsPSize,
							   BOOL			bOHTP,
							   BOOL			bOCTP,
							   BYTE			nBurstSize)
{
	// program the default FPGA file
	if (!ProgFpga(PROG_FPGA_PLAY, OMode))
		return FALSE;

	// set registers
	CAtRegisters RegAcc(m_pAtBoard);
	ATREGISTRY &Registers = RegAcc;
	
	// get registers from device and store them in the ATBoard object
	m_pAtBoard->GetRegisters();

	// set mode 
	switch(OMode)	
	{
	default:
	case IOMODE_DVB:
		SET_BITMASK(Registers.m_PlayConfig, AT_PCONF_DVB);
		CLR_BITMASK(Registers.m_PlayConfig, AT_PCONF_SMP);
		CLR_BITMASK(Registers.m_PlayConfig, AT_PCONF_RAW);
		break;
	case IOMODE_SMPE:
		CLR_BITMASK(Registers.m_PlayConfig, AT_PCONF_DVB);
		SET_BITMASK(Registers.m_PlayConfig, AT_PCONF_SMP);
		CLR_BITMASK(Registers.m_PlayConfig, AT_PCONF_RAW);
		break;
	case IOMODE_RAW:
		SET_BITMASK(Registers.m_PlayConfig, AT_PCONF_DVB);
		CLR_BITMASK(Registers.m_PlayConfig, AT_PCONF_SMP);
		SET_BITMASK(Registers.m_PlayConfig, AT_PCONF_RAW);
		break;
	}

	// input selection bit1 must be zero st play
	CLR_BITMASK(Registers.m_RecordConfig, AT_RCONF_ISEL1);

	// set output selection
	switch (OSel)
	{
	case IOSEL_NON:
		CLR_BITMASK(Registers.m_PlayConfig, AT_PCONF_SER);
		CLR_BITMASK(Registers.m_PlayConfig, AT_PCONF_PAR);
		break;
	default:
	case IOSEL_SER:
		SET_BITMASK(Registers.m_PlayConfig, AT_PCONF_SER);
		CLR_BITMASK(Registers.m_PlayConfig, AT_PCONF_PAR);
		break;
	case IOSEL_SPI:
		CLR_BITMASK(Registers.m_PlayConfig, AT_PCONF_SER);
		SET_BITMASK(Registers.m_PlayConfig, AT_PCONF_PAR);
		break;
	case IOSEL_SERSPI:
		SET_BITMASK(Registers.m_PlayConfig, AT_PCONF_SER);
		SET_BITMASK(Registers.m_PlayConfig, AT_PCONF_PAR);
		break;
	}

	// set SPI mode
	switch(OSpiMode)
	{
	case SPIMODE_FCLO_188:
		CLR_BITMASK(Registers.m_PlayConfig, AT_PCONF_SPI0);
		CLR_BITMASK(Registers.m_PlayConfig, AT_PCONF_SPI1);
		break;
	default:
	case SPIMODE_VCLO_188:
		SET_BITMASK(Registers.m_PlayConfig, AT_PCONF_SPI0);
		CLR_BITMASK(Registers.m_PlayConfig, AT_PCONF_SPI1);
		break;
	case SPIMODE_FCLO_204:
		CLR_BITMASK(Registers.m_PlayConfig, AT_PCONF_SPI0);
		SET_BITMASK(Registers.m_PlayConfig, AT_PCONF_SPI1);
		break;
	case SPIMODE_VCLO_204:
		SET_BITMASK(Registers.m_PlayConfig, AT_PCONF_SPI0);
		SET_BITMASK(Registers.m_PlayConfig, AT_PCONF_SPI1);
		break;
	}

	// set transport stream packet size
	switch(OTsPSize)
	{
	default:
	case TSPSIZE_188:
		CLR_BITMASK(Registers.m_PlayConfig, AT_PCONF_TPS0);
		CLR_BITMASK(Registers.m_PlayConfig, AT_PCONF_TPS1);
		CLR_BITMASK(Registers.m_PlayConfig, AT_PCONF_188_16);
		break;
	case TSPSIZE_188P16:
		CLR_BITMASK(Registers.m_PlayConfig, AT_PCONF_TPS0);
		CLR_BITMASK(Registers.m_PlayConfig, AT_PCONF_TPS1);
		SET_BITMASK(Registers.m_PlayConfig, AT_PCONF_188_16);
		break;
	case TSPSIZE_204:
		CLR_BITMASK(Registers.m_PlayConfig, AT_PCONF_TPS0);
		SET_BITMASK(Registers.m_PlayConfig, AT_PCONF_TPS1);
		CLR_BITMASK(Registers.m_PlayConfig, AT_PCONF_188_16);
		break;
	}

	// set re multiplexing 
	if (bORemux)
	{
		SET_BITMASK(Registers.m_PlayConfig, AT_PCONF_PCRREST);
		SET_BITMASK(Registers.m_PlayConfig, AT_PCONF_BRREMUX);
		CLR_BITMASK(Registers.m_PlayConfig, AT_PCONF_CTP);
	}
	else
	{
		CLR_BITMASK(Registers.m_PlayConfig, AT_PCONF_PCRREST);
		CLR_BITMASK(Registers.m_PlayConfig, AT_PCONF_BRREMUX);
		CLR_BITMASK(Registers.m_PlayConfig, AT_PCONF_CTP);
	}

	// set hardware generated transport stream bit
	if (bOHTP)
		SET_BITMASK(Registers.m_PlayConfig, AT_PCONF_HTP);
	else
		CLR_BITMASK(Registers.m_PlayConfig, AT_PCONF_HTP);

	// set hardware generated counter transport stream bit (only when HTP is enabled)
	if (bOCTP && bOHTP)
		SET_BITMASK(Registers.m_PlayConfig, AT_PCONF_CTP);
	else
		CLR_BITMASK(Registers.m_PlayConfig, AT_PCONF_CTP);

	// set file bit rate
	Registers.m_PlayBitrate = nFileBitrate;

	// set output bit rate
	Registers.m_OutputBitrate = nOutputBitrate;

	// set burst size
	Registers.m_BurstSize = (u32)nBurstSize;

	// set registers to device
	m_pAtBoard->UpdateRegisters();
	
	// call possible overrides of derived class
	_InitPlay(nFileBitrate, bORemux, nOutputBitrate, OSel, OMode, OSpiMode, OTsPSize, bOHTP, bOCTP, nBurstSize);

	return TRUE;
}

/*!
 * Play a file
 *
 * @param pFilename : The name of the file to play
 *
 * @return BOOL  : TRUE on success, else FALSE
 */
BOOL CATBoardPlayRec::PlayFile(char * pFilename)
{
	BOOL bRet = FALSE;
	s32 iFileDescrValue;
	s32 iReturnValue;
	static char PlayBuffer[TSDATA_BUFFERSIZE];

	CAtRegisters RegAcc(m_pAtBoard);
	ATREGISTRY &Registers = RegAcc;
	m_pAtBoard->GetRegisters();

	// start
	// open the file
	iFileDescrValue = SysFileOpenRead (pFilename);
	if (iFileDescrValue < 0)	// file open failed
	{
		bRet = FALSE;
		goto _PlayExitFunc;
	}

	// reset play
	SET_BITMASK(Registers.m_PlayConfig, AT_PCONF_PRST);
	m_pAtBoard->UpdateRegisters();
	CLR_BITMASK(Registers.m_PlayConfig, AT_PCONF_PRST);
	m_pAtBoard->UpdateRegisters();

	// start play
	if (!m_pAtBoard->StartPlaying())
	{
		bRet = FALSE;
		goto _PlayFileClose;
	}

	// send file
	do
	{
		// read data from file
		iReturnValue = SysFileRead (iFileDescrValue, PlayBuffer, TSDATA_BUFFERSIZE);

		if (iReturnValue < 0)	// error occured
		{
			bRet = FALSE;
			goto _PlayStop;
		}

		// send data to device. Retry if SendPlayPacketDirect failed. 
		while (!m_pAtBoard->SendPlayPacketDirect((u8*)PlayBuffer, iReturnValue))
			SysSleep(1);

		// enable play after first block send (else sync errors occur)
		if (!TST_BITMASK(Registers.m_PlayConfig, AT_PCONF_PENA))
		{	
			SET_BITMASK(Registers.m_PlayConfig, AT_PCONF_PENA);
			m_pAtBoard->UpdateRegisters();
		}

	} while (iReturnValue != 0);	// keep sending until complete file is send

	// stop 
	bRet = TRUE;

_PlayStop:
	// stop playing
	if (!m_pAtBoard->StopPlaying())
		bRet = FALSE;

	// disable play
	CLR_BITMASK(Registers.m_PlayConfig, AT_PCONF_PENA);
	m_pAtBoard->UpdateRegisters();

_PlayFileClose:
	// close file
	SysFileClose(iFileDescrValue);

_PlayExitFunc:

	// return
	return bRet;
}

/*
 * Initialize recording
 */
BOOL CATBoardPlayRec::InitRec(	EIOMode		IMode,
								EIOSel		ISel,
								BOOL		bILoop,
								BOOL		bIPassThroughSer,
								BOOL		bIPassThroughSpi,
								BOOL		bITimeStamp,
								EIOSpiMode	ISpiMode)
{
	// program the default FPGA file
	if (!ProgFpga(PROG_FPGA_REC, IMode))
		return FALSE;

	// set registers
	CAtRegisters RegAcc(m_pAtBoard);
	ATREGISTRY &Registers = RegAcc;

	// get registers from device and store them in the ATBoard object
	m_pAtBoard->GetRegisters();

	// set mode 
	switch(IMode)	
	{
	default:
	case IOMODE_DVB:
		SET_BITMASK(Registers.m_RecordConfig, AT_RCONF_DVB);
		CLR_BITMASK(Registers.m_RecordConfig, AT_RCONF_SMP);
		CLR_BITMASK(Registers.m_RecordConfig, AT_RCONF_RAW);
		break;
	case IOMODE_SMPE:
		CLR_BITMASK(Registers.m_RecordConfig, AT_RCONF_DVB);
		SET_BITMASK(Registers.m_RecordConfig, AT_RCONF_SMP);
		CLR_BITMASK(Registers.m_RecordConfig, AT_RCONF_RAW);
		break;
	case IOMODE_RAW:
		SET_BITMASK(Registers.m_RecordConfig, AT_RCONF_DVB);
		CLR_BITMASK(Registers.m_RecordConfig, AT_RCONF_SMP);
		SET_BITMASK(Registers.m_RecordConfig, AT_RCONF_RAW);
		break;
	}

	// enable loop through
	if (bILoop)
		SET_BITMASK(Registers.m_RecordConfig, AT_RCONF_LEN);
	else
		CLR_BITMASK(Registers.m_RecordConfig, AT_RCONF_LEN);

	// enable pass through to serial output
	if (bIPassThroughSer)
		SET_BITMASK(Registers.m_PlayConfig, AT_PCONF_SER);
	else
		CLR_BITMASK(Registers.m_PlayConfig, AT_PCONF_SER);

	// enable pass through to SPI output
	if (bIPassThroughSpi)
		SET_BITMASK(Registers.m_PlayConfig, AT_PCONF_PAR);
	else
		CLR_BITMASK(Registers.m_PlayConfig, AT_PCONF_PAR);

	// enable time stamping
	if (bITimeStamp)
		SET_BITMASK(Registers.m_RecordConfig, AT_RCONF_ETS);
	else
		CLR_BITMASK(Registers.m_RecordConfig, AT_RCONF_ETS);

	// set input selection
	switch (ISel)
	{
	default:
	case IOSEL_SER:
		SET_BITMASK(Registers.m_RecordConfig, AT_RCONF_ISEL0);
		SET_BITMASK(Registers.m_RecordConfig, AT_RCONF_ISEL1);
		break;
	case IOSEL_SPI:
		CLR_BITMASK(Registers.m_RecordConfig, AT_RCONF_ISEL0);
		SET_BITMASK(Registers.m_RecordConfig, AT_RCONF_ISEL1);
		break;
	}

	// set SPI mode
	switch(ISpiMode)
	{
	case SPIMODE_FCLO_188:
		CLR_BITMASK(Registers.m_RecordConfig, AT_RCONF_SPI);
		break;
	default:
	case SPIMODE_VCLO_188:
		SET_BITMASK(Registers.m_RecordConfig, AT_RCONF_SPI);
		break;
	}

	// set registers to device
	m_pAtBoard->UpdateRegisters();

	// call possible overrides of derived class
	_InitRec(IMode, ISel, bILoop, bIPassThroughSer, bIPassThroughSpi, bITimeStamp, ISpiMode);

	return TRUE;
}

/*
 * Record a file
 */
BOOL CATBoardPlayRec::RecFile(char * pFilename, u32 nFileSize)
{
	BOOL bRet = FALSE;
	s32 iFileDescrValue;
	s32 iReturnValue;
	u32 nReadRequestSize;
	u32 nReadReturnSize;
	u32 nTotalReadSize = nFileSize*1000000UL;
	static char RecBuffer[TSDATA_BUFFERSIZE];

	CAtRegisters RegAcc(m_pAtBoard);
	ATREGISTRY &Registers = RegAcc;
	m_pAtBoard->GetRegisters();

	// start
	// open the file
	iFileDescrValue = SysFileOpenWrite (pFilename);
	if (iFileDescrValue < 0)	// file open failed
	{
		bRet = FALSE;
		goto _RecExitFunc;
	}

	// reset rec
	SET_BITMASK(Registers.m_RecordConfig, AT_RCONF_RRST);
	m_pAtBoard->UpdateRegisters();
	CLR_BITMASK(Registers.m_RecordConfig, AT_RCONF_RRST);
	m_pAtBoard->UpdateRegisters();

	// start play
	if (!m_pAtBoard->StartRecording())
	{
		bRet = FALSE;
		goto _RecFileClose;
	}

	// enable recording
	SET_BITMASK(Registers.m_RecordConfig, AT_RCONF_RENA);
	m_pAtBoard->UpdateRegisters();

	// send file
	do
	{
		if (nTotalReadSize > TSDATA_BUFFERSIZE)
			nReadRequestSize = TSDATA_BUFFERSIZE;
		else
			nReadRequestSize = nTotalReadSize;
		
		// get data from device
		if(!m_pAtBoard->GetRecordPacketDirect((u8*)RecBuffer, nReadRequestSize, &nReadReturnSize))
		{
			bRet = FALSE;
			goto _RecStop;
		}

		// write data to file
		iReturnValue = SysFileWrite(iFileDescrValue, RecBuffer, nReadReturnSize);

		// error occurred at writing
		if (iReturnValue < 0)
		{
			bRet = FALSE;
			goto _RecStop;
		}

		// if there was no data available: go to sleep.
		if (nReadReturnSize == 0)
			SysSleep(1);

		nTotalReadSize -= nReadReturnSize;

	} while (nTotalReadSize > 0);	// keep sending until complete file is send

	// stop 
	bRet = TRUE;

_RecStop:
	// stop playing
	if (!m_pAtBoard->StopRecording())
		bRet = FALSE;

	// disable play
	CLR_BITMASK(Registers.m_RecordConfig, AT_RCONF_RENA);
	m_pAtBoard->UpdateRegisters();

_RecFileClose:
	// close file
	SysFileClose(iFileDescrValue);

_RecExitFunc:

	// return
	return bRet;
}

/*
 * get status of the input transport stream
 */
BOOL CATBoardPlayRec::GetRecStatus(ETSPSize &TsPSize, BOOL &bInSync, BOOL &bCarrierDetect, BOOL &bLocked, u32 &nBitrate)
{
	// set variables to default
	TsPSize = TSPSIZE_188;
	bInSync = FALSE;
	bCarrierDetect = FALSE;
	bLocked = FALSE;
	nBitrate = 0;

	CAtRegisters RegAcc(m_pAtBoard);
	ATREGISTRY &Registers = RegAcc;

	// get registers from device and store them in the ATBoard object
	m_pAtBoard->GetRegisters();

	// get input transport stream packet size
	if (TST_BITMASK(Registers.m_Status, AT_STAT_RTPS0))
		TsPSize = TSPSIZE_204;
	else
		TsPSize = TSPSIZE_188;

	// check transport stream sync bit
	if (TST_BITMASK(Registers.m_Status, AT_STAT_RSYNC))
		bInSync = TRUE;
	else
		bInSync = FALSE;

	// check transport stream carrier detect bit
	if (TST_BITMASK(Registers.m_Status, AT_STAT_RCD))
		bCarrierDetect = TRUE;
	else
		bCarrierDetect = FALSE;

	// check transport stream lock bit
	if (TST_BITMASK(Registers.m_Status, AT_STAT_RLOCK))
		bLocked = TRUE;
	else
		bLocked = FALSE;

	// get bit rate
	if (TST_BITMASK(Registers.m_Status, AT_STAT_RBRTIMELOCK) || TST_BITMASK(Registers.m_Status, AT_STAT_RBRPCRLOCK))
	{
		// If the bit rate must be obtained, get registers again because at the first read of the registers,
		// the Registers.m_BitrateBytesCount and Registers.m_BitrateTimeInterval values are 0.
		m_pAtBoard->GetRegisters();

		__int64 i64BitRate = (__int64)27000000 * Registers.m_BitrateBytesCount * 8;
		i64BitRate /= (__int64)Registers.m_BitrateTimeInterval;
		nBitrate = (u32)i64BitRate;
	}

	return TRUE;
}

/*
* Programs the FPGA file to the device
*/
BOOL CATBoardPlayRec::ProgFpga(EProgFpgaMode ProgFpgaMode, EIOMode Mode)
{
	// get device information
	ATDEVICEINFO DevInfo;
	if (!m_pAtBoard->GetDeviceInfo(&DevInfo))
		return FALSE;
	
	// if board is demo board: no FPGA file needs to loaded
	if (DevInfo.DeviceType == ATDEMO)
		return TRUE;

	const FPGAModeEnum FPGAModeArray[PROG_FPGA_ENUM_SIZE][IOMODE_SIZE] =
	{
		// PROG_FPGA_DEFAULT
		{PLAY_DVB,	PLAY_SMPTE,	PLAY_RAW},
		// PROG_FPGA_PLAY
		{PLAY_DVB,	PLAY_SMPTE,	PLAY_RAW},
		// PROG_FPGA_REC
		{REC_DVB,	REC_SMPTE,	REC_RAW},
		// PROG_FPGA_MODULATOR
		{MODULATOR,	MODULATOR,	MODULATOR}
	};

	// set FpgaMode to requested value
//	FPGAModeEnum FpgaMode = FPGAModeArray[ProgFpgaMode][Mode];

	// The default FPGA file is always the play. If the board is record only
	// program the play file
	if (ProgFpgaMode==PROG_FPGA_DEFAULT && DevInfo.DeviceCanPlay == FALSE)
		ProgFpgaMode = PROG_FPGA_REC;

	// if a modulator is attached, use the MODULATOR FPGA file (the modulators do not have a 
	// separate record and play FPGA file)
	if (DevInfo.DeviceHasDVB_C_Mod || DevInfo.DeviceHasDVB_S_Mod || DevInfo.DeviceHasDVB_T_Mod)
		ProgFpgaMode = PROG_FPGA_MOD;

	// check if the device can play
	if (ProgFpgaMode==PROG_FPGA_PLAY && DevInfo.DeviceCanPlay == FALSE)
		return FALSE;

	// check if the device can record
	if (ProgFpgaMode==PROG_FPGA_REC && DevInfo.DeviceCanRecord == FALSE)
		return FALSE;
	

	FPGAModeEnum FpgaMode = FPGAModeArray[ProgFpgaMode][Mode];

	// program the FPGA
	if (!m_pAtBoard->ProgramFPGA(FpgaMode))
		return FALSE;

	return TRUE;
}
