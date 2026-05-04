#include <windows.h>

typedef void (* td_QuickParseUserData) (BYTE * pData, int user_data_len, int nESParsePMTIndex, int nESParseESIndex, int nES);

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

int h264decoder_main(HANDLE hInPipe, PH264DECODE hd);

int H264(HANDLE hInPipe, PH264DECODE hd)
{
	return h264decoder_main(hInPipe, hd);
}
