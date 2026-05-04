#include <windows.h>

#include "skl_utils.h"
#include "skl_syst/skl_mpg4.h"
#include "skl_syst/skl_dyn_load.h"
#include "skl_syst/skl_exception.h"
#include "skl_syst/skl_mem_trc.h"
#include "skl_syst/skl_dsp.h"
#include "skl_syst/skl_cpu_specs.h"

typedef struct _tagMPEG4Decode
{
	unsigned char * pY;
	unsigned char * pU;
	unsigned char * pV;
	int x, y;
} MPEG4DECODE, *PMPEG4DECODE;

SKL_MP4_NEW_DEC MP4_New_Decoder;
SKL_MP4_DELETE_DEC MP4_Delete_Decoder;

SKL_CPU_FEATURE Cpu;

int ReadFromMPEG4ESPipe(HANDLE hInPipe, BYTE * pBuffer, int nLength)
{
	int nRequestedLength = nLength;
	DWORD dwRead;

	while (nLength)
	{
		ReadFile(hInPipe, pBuffer, nLength, &dwRead, NULL);
		if (dwRead == 0)
			return 0;
		pBuffer += dwRead;
		nLength -= dwRead;
	}

	return nRequestedLength;
}

int MPEG4(HANDLE hInPipe, PMPEG4DECODE hd)
{
	MP4_New_Decoder = (SKL_MP4_NEW_DEC)SKL_LOAD_DYN_SYMBOL(SKL_MP4_DLL, Skl_MP4_New_Decoder);
	MP4_Delete_Decoder = (SKL_MP4_DELETE_DEC)SKL_LOAD_DYN_SYMBOL(SKL_MP4_DLL, Skl_MP4_Delete_Decoder);
	if (MP4_New_Decoder==0 || MP4_Delete_Decoder==0)  // problem with DLL
		return -1;

	SKL_MP4_DEC *Dec = MP4_New_Decoder();
	if (Dec==0)
		return -2;
	Dec->Set_CPU( Cpu );

	long Fullness, Left_To_Read, Size, Left;
	SKL_BYTE *Buf, *mBuf = 0;
	const int MAX_BUF = 400000;   // TODO: BAD
	SKL_BYTE Buf0[MAX_BUF+4] = {0};     // +4 = sentinel (just in case)
	int Err = 0;
	
	Size = 1024 * 1024 * 100;
	Buf = Buf0;
	Left_To_Read = Size;
	Fullness = 0;
	Left = Size;
	while(Left>=0)
	{
		if (Fullness<MAX_BUF/2)
		{
			if (Fullness>0)
				memmove( Buf0, Buf, Fullness);
			Buf = Buf0;
			const int Free = MAX_BUF-Fullness;
			size_t More = (Left_To_Read>Free) ? Free : Left_To_Read;
			if (More>0)
			{
				More = ReadFromMPEG4ESPipe(hInPipe, Buf+Fullness, More);
				Fullness += More;
				Left_To_Read -= More;
			}
	    }

		int Read = Dec->Decode(Buf, Fullness);
		if (Read>Fullness)
			throw SKL_EXCEPTION( "Aieeee! Buffer read overflow" );

		if (Left==0 && Read==0)
			Left = -1;   // very last frame
		else
			Left -= Read;
		Buf       += Read;
		Fullness  -= Read;

		if (!Dec->Is_Frame_Ready())
			continue;
      
		SKL_MP4_PIC Pic;
		Dec->Consume_Frame(&Pic);
		hd->x = Pic.Width;
		hd->y = Pic.Height;	
		BYTE *YSrc = Pic.Y;
		BYTE *YDest = hd->pY;
		int k = Pic.Height;
		while(k-->0)
		{
			memcpy(YDest, YSrc, Pic.Width);
			YSrc += Pic.BpS;
			YDest += Pic.Width;
			
		}
		k = Pic.Height/2;
		BYTE *USrc = Pic.U;
		BYTE *UDest = hd->pU;
		BYTE *VSrc = Pic.V;
		BYTE *VDest = hd->pV;
		while(k-->0)
		{
			memcpy(UDest, USrc, Pic.Width / 2);
			USrc += Pic.BpS;
			UDest += Pic.Width / 2;
		
			memcpy(VDest, VSrc, Pic.Width / 2);
			VSrc += Pic.BpS;
			VDest += Pic.Width / 2;
		}
		break;
	}

	MP4_Delete_Decoder(Dec);	
	return 0;
}
