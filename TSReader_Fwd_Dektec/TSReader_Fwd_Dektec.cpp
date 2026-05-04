#include <windows.h>
#include <commctrl.h>
#include "initguid.h"
#include "winioctl.h"
#include "setupapi.h"	// VC++ 5 one is out of date
#include <stdio.h>

#include "..\forwarder.h"

#include "DTAPI.h"

static char gszModuleName[] = {"Dektec ASI Forwarder Module"};

#define DEKTEC_WRITE_COUNT 32
#define PACKET_BUFFER_MAX 32

DtDevice Dvc;
DtOutpChannel TsOut;
int nWritePacketOffset;
int gnPacketLength;
int nTransmitPacketCounter;
BYTE bBuffer[PACKET_BUFFER_MAX * 204];

BOOL TSReader_Fwd_Init(HWND hWnd, int nPacketLength, int nBitRate)
{
	OutputDebugString("+TSReader_Fwd_Init\n");

	gnPacketLength = nPacketLength;
	nWritePacketOffset = 0;
	nTransmitPacketCounter = 0;

	// Find the card and attach to it
	if (Dvc.AttachToType(140) != DTAPI_OK)
	{
		MessageBox(hWnd, "Can't attach to the Dektec 140 board", gszModuleName, MB_ICONSTOP);
		return FALSE;
	}
	if (TsOut.Attach(&Dvc) != DTAPI_OK)
	{
		MessageBox(hWnd, "Can't attach to the output channel", gszModuleName, MB_ICONSTOP);
		return FALSE;
	}

	// Dektec's API wants 188 byte rate even if using 204 byte packets
	// Which is what TSR gives us - the real rate, so adjust if needed
	if (nPacketLength != 188)
	{
		double dRSRate = (double)nBitRate * (188.0 / 204.0);
		nBitRate = (int)dRSRate;
	}
	TsOut.SetTsRateBps(DTAPI_TXCLOCK_INTERNAL, nBitRate);

	// Setup output packet length
	if (nPacketLength != 188)
		TsOut.SetTxMode(DTAPI_TXMODE_204, 0);
	else
		TsOut.SetTxMode(DTAPI_TXMODE_188, 0);

	TsOut.SetTxControl(DTAPI_TXCTRL_HOLD);

	OutputDebugString("-TSReader_Fwd_Init\n");
	return TRUE;
}

BOOL TSReader_Fwd_DeInit()
{
	OutputDebugString("+TSReader_Fwd_DeInit\n");
	if (nTransmitPacketCounter > 32)
		TsOut.SetTxControl(DTAPI_TXCTRL_HOLD);
	TsOut.Detach (DTAPI_INSTANT_DETACH);
	OutputDebugString("-TSReader_Fwd_DeInit\n");
	return TRUE;
}

BOOL TSReader_Fwd_Data(BYTE * pBuffer, int nLength)
{
	memcpy(&bBuffer[nWritePacketOffset * gnPacketLength], pBuffer, nLength);
	nWritePacketOffset++;
	if (nWritePacketOffset == PACKET_BUFFER_MAX)
	{
		// Got PACKET_BUFFER_MAX packets so let's send them to the Dektec API
		TsOut.Write((char *)bBuffer, nWritePacketOffset * gnPacketLength);
		nWritePacketOffset = 0;

		if (nTransmitPacketCounter < DEKTEC_WRITE_COUNT)
		{
			nTransmitPacketCounter++;
		}
		else if (nTransmitPacketCounter == DEKTEC_WRITE_COUNT)
		{
			TsOut.SetTxControl(DTAPI_TXCTRL_SEND);
			nTransmitPacketCounter++;
		}
	}
	return TRUE;
}

BOOL TSReader_Fwd_GetDescription(char * szDeviceNameBuffer)
{
	lstrcpy(szDeviceNameBuffer, "Dektec ASI");
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

/*
// Attach PciCard and output channel objects to hardware
printf("No DTA-100 in system. Quitting...\n");
return -3;
}
if (TsOut.Attach(&Dvc) != DTAPI_OK) {
printf("Can't attach output channel.\nQuitting...\n");
return -4; // Detach is executed from DtDevice destructor
}
// Initialise bit rate and packet mode
char Buf[BUFSIZE];
int Load = 0;
int NumBytes = fread(Buf, 1, BUFSIZE, fp);
while (Load<INILOAD && NumBytes!=0) {
TsOut.Write(Buf, NumBytes);
Load += NumBytes;
NumBytes = fread(Buf, 1, BUFSIZE, fp);
}
// Start transmission
TsOut.SetTxControl(DTAPI_TXCTRL_SEND);
// Main loop
while (NumBytes != 0) {
TsOut.Write(Buf, NumBytes);
NumBytes = fread(Buf, 1, BUFSIZE, fp);
}
TsOut.Detach (DTAPI_INSTANT_DETACH);
  */