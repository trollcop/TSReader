#include <windows.h>
#include <commctrl.h>
#include "initguid.h"
#include "winioctl.h"
#include "setupapi.h"	// VC++ 5 one is out of date
#include <stdio.h>

#include "..\forwarder.h"

#include "ATDV_API.H"
//#include <algorithm>

CATBoard *pBoard;
CAtBoardManager *pMan;

#define TS_BUFFER_SIZE 204 * 40
int nTSBufferOffset;
int nFirstOutput;
BYTE bTSBuffer[TS_BUFFER_SIZE];

static char * szValidDevices[] = {
	"AT4USB",
	"AT40USB",			// Mini ASI I/O
	"AT40R1USB",
	"AT400USB",
	"ATDEMO"
};
static char gszModuleName[] = {"Alitronika Forwarder Module"};

#define SET_BITMASK(var, pat) var |= pat
#define CLR_BITMASK(var, pat) var &= ~pat

BOOL TSReader_Fwd_Init(HWND hWnd, int nPacketLength, int nBitRate)
{
    char * pFriendlyName = NULL;
    char * pDevicesBoardName = NULL;
    CAtDeviceList iDevices;
    
	pMan = CAtBoardManager::Instance();
	
	pBoard = NULL;

    //Update device list.
    pMan->GetDeviceList(iDevices);

    //Get the first board name from the device list.
    pDevicesBoardName =  iDevices.GetFirstBoardName();
	if (pDevicesBoardName == NULL)
		OutputDebugString("TSReader Alitronika: GetFirstBoardName() returned NULL\n");

    // Loop through all devices in the list.
    // Stop the loop when the board name = NULL indicating no board found 
    while(pDevicesBoardName)
    {
        //Get the friendly name of the board
        pFriendlyName = iDevices.GetFriendlyName(pDevicesBoardName);
		{
			char szTemp[128];
			wsprintf(szTemp, "TSReader Alitronika: Device found %s\n", pFriendlyName);
			OutputDebugString(szTemp);
		}

        //Check if the boardname is 
        if(iDevices.IsBoardNameValid(pDevicesBoardName))
        {
			int nDeviceType;

			for (nDeviceType = 0; lstrcmp(szValidDevices[nDeviceType], "ATDEMO") != 0; nDeviceType++)
			{
				if (lstrcmp(szValidDevices[nDeviceType], pFriendlyName) == 0)
				{
					// We've found an interface with the right name.
					iDevices.SetBoardUsed(pDevicesBoardName, TRUE);
					pBoard = pMan->GetBoard(pDevicesBoardName);
					break;
				}
			}
        }
		if (pBoard != NULL)
			break;

        //Get the next board name from the device list
        pDevicesBoardName = iDevices.GetNextBoardName();
    }

	if (pBoard == NULL)
	{
		MessageBox(hWnd, "Unable to locate a suitable Alitronika interface", gszModuleName, MB_ICONSTOP);
		return FALSE;
	}
	
	// Upload the correct FPGA file
	pBoard->ProgramFPGA(PLAY_DVB);

	CAtRegisters RegAcc(pBoard);
	ATREGISTRY &Registers = RegAcc;

	// get registers from device and store them in the ATBoard object
	pBoard->GetRegisters();

	// set mode 
	enum
	{
		IOMODE_DVB = 0,
		IOMODE_SMPE,
		IOMODE_RAW
	};
	int OMode = IOMODE_DVB;
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
	enum
	{
		IOSEL_NON = 0,
		IOSEL_SER,
		IOSEL_SPI,
		IOSEL_SERSPI
	};
	int OSel = IOSEL_SER;
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
	enum
	{
		SPIMODE_FCLO_188 = 0,
		SPIMODE_VCLO_188,
		SPIMODE_FCLO_204,
		SPIMODE_VCLO_204
	};
	int OSpiMode = SPIMODE_FCLO_188;
	if (nPacketLength != 188)
		OSpiMode = SPIMODE_FCLO_204;
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
	enum
	{
		TSPSIZE_188 = 0,
		TSPSIZE_188P16,
		TSPSIZE_204
	};
	int OTsPSize = TSPSIZE_188;
	if (nPacketLength != 188)
		OTsPSize = TSPSIZE_204;
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
	int bORemux = FALSE;
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
	int bOHTP = FALSE;
	if (bOHTP)
		SET_BITMASK(Registers.m_PlayConfig, AT_PCONF_HTP);
	else
		CLR_BITMASK(Registers.m_PlayConfig, AT_PCONF_HTP);

	// set hardware generated counter transport stream bit (only when HTP is enabled)
	BOOL bOCTP = FALSE;
	if (bOCTP && bOHTP)
		SET_BITMASK(Registers.m_PlayConfig, AT_PCONF_CTP);
	else
		CLR_BITMASK(Registers.m_PlayConfig, AT_PCONF_CTP);

	// set file bit rate
	Registers.m_PlayBitrate = nBitRate;

	// set output bit rate
	Registers.m_OutputBitrate = nBitRate;

	// set burst size
	int nBurstSize = nPacketLength * 10;
	Registers.m_BurstSize = (u32)nBurstSize;

	// set registers to device
	pBoard->UpdateRegisters();

	nTSBufferOffset = 0;
	nFirstOutput = 0;
	return TRUE;
}

BOOL TSReader_Fwd_DeInit()
{
	if (nFirstOutput == 2)
	{
		CAtRegisters RegAcc(pBoard);
		ATREGISTRY &Registers = RegAcc;
		pBoard->GetRegisters();

		pBoard->StopPlaying();

		// disable play
		CLR_BITMASK(Registers.m_PlayConfig, AT_PCONF_PENA);
		pBoard->UpdateRegisters();
	}
	OutputDebugString("Alitronika Fwd: DeInit() finished\n");
	return TRUE;
}

BOOL TSReader_Fwd_Data(BYTE * pBuffer, int nLength)
{
	// We get one packet at a time. Save the packets until we've got about
	// 10 of them and then we can start sending
	if (nLength + nTSBufferOffset < TS_BUFFER_SIZE)
	{
		memcpy(&bTSBuffer[nTSBufferOffset], pBuffer, nLength);
		nTSBufferOffset += nLength;
		return TRUE;
	}

	if (nFirstOutput == 0)
	{
		CAtRegisters RegAcc(pBoard);
		ATREGISTRY &Registers = RegAcc;
		pBoard->GetRegisters();

		// reset play
		SET_BITMASK(Registers.m_PlayConfig, AT_PCONF_PRST);
		pBoard->UpdateRegisters();
		CLR_BITMASK(Registers.m_PlayConfig, AT_PCONF_PRST);
		pBoard->UpdateRegisters();

		// start play
		pBoard->StartPlaying();
	}

	while (!pBoard->SendPlayPacketDirect((u8*)bTSBuffer, nTSBufferOffset))
		Sleep(1);
	nTSBufferOffset = 0;
	// Don't forget the current packet!
	memcpy(&bTSBuffer[nTSBufferOffset], pBuffer, nLength);
	nTSBufferOffset += nLength;

	// enable play after first block send (else sync errors occur)
	if (nFirstOutput == 2)
	{
		CAtRegisters RegAcc(pBoard);
		ATREGISTRY &Registers = RegAcc;
		pBoard->GetRegisters();

		SET_BITMASK(Registers.m_PlayConfig, AT_PCONF_PENA);
		pBoard->UpdateRegisters();
		nFirstOutput++;
	}
	else if (nFirstOutput < 2)
		nFirstOutput++;

	
	return TRUE;
}
 
BOOL TSReader_Fwd_GetDescription(char * szDeviceNameBuffer)
{
	lstrcpy(szDeviceNameBuffer, "Alitronika AT4/AT40/AT400");
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

