#include <windows.h>
#include <commctrl.h>
#include <stdlib.h>     /* standard includes */
#include <stdio.h>
#include <string.h>
#include "..\parser.h"

#include "fvd_mp4.h"

typedef struct _tagH264Decode
{
	unsigned char * pY;
	unsigned char * pU;
	unsigned char * pV;
	int x, y;
	int interlaced;
	int maximum_pictures;
	HWND hWndST;
	td_QuickParseUserData UserFunction;
} H264DECODE, *PH264DECODE;


int ReadFromMPEG4ESPipe(HANDLE hInPipe, BYTE * pBuffer, int nLength)
{
	int nRequestedLength = nLength;
	DWORD dwRead;

	while (nLength)
	{
		ReadFile(hInPipe, pBuffer, nLength, &dwRead, NULL);
		if (dwRead == 0)
			return 0;
		return dwRead;
		pBuffer += dwRead;
		nLength -= dwRead;
	}

	return nRequestedLength;
}

//#define FILE_OUTPUT
int ForVid(HANDLE hInPipe, PH264DECODE hd)
{
	const int MAX_BUF = 1000000;   // TODO: BAD
	long Fullness = 0;
	long Size = 1024 * 1024 * 30;
	long Left_To_Read = Size;
	long Left = Size;
	int Err = 0;
	int nFrameCount = hd->maximum_pictures;
	FVD_VIDEO_DEC * Dec = 0;
	FVD_BYTE *Buf0 = (FVD_BYTE *)LocalAlloc(LPTR, MAX_BUF + 4);
	FVD_BYTE *Buf;
	FVD_BYTE *mBuf = 0;
	FVD_PIC Pic;  // container for picture's information and sample
	char szStatus[256] = {""};
#ifdef FILE_OUTPUT
	HANDLE hDebug = INVALID_HANDLE_VALUE;
	char szDebugName[MAX_PATH];

	OutputDebugString("TSReader: DEBUG: H264DecoderThread+\n");
	wsprintf(szDebugName, "c:\\MPEG-ES\\%08x.h264", GetTickCount());
	hDebug = CreateFile(szDebugName, GENERIC_WRITE, 0, (LPSECURITY_ATTRIBUTES) NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, (HANDLE) NULL);
#endif FILE_OUTPUT

	Buf = Buf0;
	Dec = Fvd_New_Decoder(FVD_MPEG4_H264);
	//Fvd_Dec_Set_Param(Dec, FVD_CMD_MT_METHOD, 2, 0); 
	//Fvd_Dec_Set_Param(Dec, FVD_CMD_CPU, FVD_CPU_DETECT, 0);
	Fvd_Dec_Set_Param(Dec, FVD_CMD_NO_FILTER, 1,  0);
	while(Left >= 0)
	{
		if (Fullness < MAX_BUF / 2)
		{
			if (Fullness > 0)
				memmove(Buf0, Buf, Fullness);
			Buf = Buf0;
			const int Free = MAX_BUF - Fullness;
			size_t More = (Left_To_Read > Free) ? Free : Left_To_Read;
			if (More > 0)
			{
				More = ReadFromMPEG4ESPipe(hInPipe, Buf+Fullness, More);
#ifdef FILE_OUTPUT
				{
					DWORD dwWritten;
					WriteFile(hDebug, Buf+Fullness, More, &dwWritten, NULL);
					FlushFileBuffers(hDebug);
				}
#endif FILE_OUTPUT
				Fullness += More;
				Left_To_Read -= More;
			}
	    }

		int Read = Fvd_Dec_Decode(Dec, Buf, Fullness);
		if (Read > Fullness)
		{
			OutputDebugString("Aieeee! Buffer read overflow\n");
			break;
		}

		if (Left == 0 && Read == 0)
			Left = -1;   // very last frame
		else
			Left -= Read;
		Buf += Read;
		Fullness -= Read;

		const BYTE * User_Data = (const BYTE *)Fvd_Dec_Get_Param( Dec, FVD_CMD_GET_USER_DATA_REG);
		if (User_Data!=0)
		{
			const int Len = Fvd_Dec_Get_Param( Dec, FVD_CMD_GET_USER_DATA_REG_LEN);
			if (User_Data[0] == 0xb5)	// itu_t_t35_country_code for ATSC
			{
				//hd->UserFunction((BYTE *)&User_Data[3], Len - 3);
			}
			//else
			//	hd->UserFunction(NULL, 0);			// giant kludge to get the user indicator on

		}

		User_Data = (const BYTE *)Fvd_Dec_Get_Param( Dec, FVD_CMD_GET_USER_DATA);
		if (User_Data!=0)
		{
			const int Len = Fvd_Dec_Get_Param( Dec, FVD_CMD_GET_USER_DATA_REG);
			if (User_Data[0] == 0xb5)	// itu_t_t35_country_code for ATSC
			{
			//	hd->UserFunction((BYTE *)&User_Data[3], Len - 3);
			}
		}

		if (!Fvd_Dec_Has_Pending_Frames(Dec))
			continue;

		Fvd_Dec_Consume_Frame(Dec, &Pic);
		switch(Pic.Coding)
		{
			case 1:
				lstrcat(szStatus, "I1");
				break;
			case 2:
				lstrcat(szStatus, "I2");
				break;
			case 3:
				lstrcat(szStatus, "P");
				break;
			case 4:
				lstrcat(szStatus, "B");
				break;
		}
		if (--nFrameCount && Pic.Coding != 1 && Pic.Coding != 2)
		//if (--nFrameCount)
			continue;

		{
			char szTemp[256];
			wsprintf(szTemp, "Forvid: H.264 frames composition: %s\n", szStatus);
			OutputDebugString(szTemp);
		}
		hd->x = Pic.Width;
		hd->y = Pic.Height;	
		hd->interlaced = Pic.Interlaced;

		BYTE *YSrc = Pic.Y;
		BYTE *YDest = hd->pY;
		int k = Pic.Height;
		while(k-- > 0)
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
		while(k-- > 0)
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

	if (Left == 0)
	{
		OutputDebugString("TSReader: ForVid.cpp - no picture after 30 MB\n");
	}
#ifdef FILE_OUTPUT
	CloseHandle(hDebug);
#endif FILE_OUTPUT
	Fvd_Delete_Decoder( Dec );
	LocalFree(Buf0);
	return 0;
}

// This DLL is for TSR Pro, but this little kludge and a rename to TSReader_H264.dll will 
// allow operation with TSR Standard.

int H264(HANDLE hInPipe, PH264DECODE hd)
{
	return ForVid(hInPipe, hd);
}
