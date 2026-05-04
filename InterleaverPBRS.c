#ifdef PRO
#include <windows.h>
#include <commctrl.h>

#include "tsreader.h"

extern PVARIABLES v;

int GetInterLeaverInputPosition(int nWritePointer)
{
	int nResult = 0;

	while (--nWritePointer)
		nResult += nWritePointer * 17;

	return nResult;	
}

void InterleavePacket(BYTE * pOutputPacket, BYTE * pInputPacket)
{
	BYTE i;

	for (i = 0; i < 204; i++)
	{
		if (v->nInterleaveRow == 0)
			*(pOutputPacket++) = *(pInputPacket++);
		else
		{
			int nInputPos = GetInterLeaverInputPosition(v->nInterleaveRow);
			int nCellLength = v->nInterleaveRow * 17;
			int nOutputPos = nInputPos + nCellLength - 1;

			*(pOutputPacket++) = v->bInterleaver[nOutputPos];
			memmove(&v->bInterleaver[nInputPos + 1], &v->bInterleaver[nInputPos], nCellLength - 1);
			v->bInterleaver[nInputPos] = *(pInputPacket++);
		}
		v->nInterleaveRow = (v->nInterleaveRow + 1) % 12;
	}
}

void PBRSScramble(BYTE * pPacket)
{
	static WORD nPBRSReg = 0;
	BYTE i;

	for (i = 0; i < 188; i++)
	{
		BYTE j;
		register BYTE nInputByte = pPacket[i];
		register BYTE nOutputByte = 0;
		
		if (nInputByte == 0xb8 && !i)
		{
			nPBRSReg = 0x00a9;	// reset the PRBS register
			continue;			// don't bump PBRS for first sync
		}
		
		for (j = 7; j != 0xff; j--)
		{
			register BYTE nBit0;
			register BYTE nExtractBit;
			register BYTE nInputBit;
			register BYTE nOutputBit;

			nExtractBit = 1 << j;
			nInputBit = nInputByte & nExtractBit;
			if (nInputBit)
				nInputBit = 1;
			nBit0 = (nPBRSReg & 0x4000) >> 14 ^ (nPBRSReg & 0x2000) >> 13;
			nPBRSReg = ((nPBRSReg << 1) & 0x7ffe) | nBit0;			
			nOutputBit = nInputBit ^ nBit0;
			if (nOutputBit)
				nOutputByte |= nExtractBit;
		}

		if (i)	// don't scramble syncs
			pPacket[i] = nOutputByte; 
	}	
}
#endif PRO
