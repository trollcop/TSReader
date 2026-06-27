/* _ISource.h replacement header.
   Declares functions implemented in isource_impl.c using stb_image + GDI. */

#ifndef _ISOURCE_H_REPLACEMENT
#define _ISOURCE_H_REPLACEMENT

#include <Windows.h>

typedef struct { char szPath[MAX_PATH]; } * HISSRC;
typedef struct { char szPath[MAX_PATH]; } * HISDEST;

void    _ISInitialize(const char * key);

HISSRC  _ISOpenFileSource(const char * filename);
void    _ISCloseSource(HISSRC h);

HISDEST _ISOpenFileDest(const char * filename);
void    _ISCloseDest(HISDEST h);

void *  _ISReadBMPToRGB(HISSRC h, DWORD * pw, DWORD * ph);
void *  _ISReadJPGToRGB(HISSRC h, DWORD * pw, DWORD * ph);

int     _ISWriteRGBToJPG(HISDEST hd, void * pData, int nWidth, int nHeight, int nQuality, int nFlags);

void    _ISDrawRGB(HDC hDC, void * pData, int srcW, int srcH,
                   int dstX, int dstY, int dstW, int dstH, void * reserved);

void    _ISOverlayRGBTrans(void * pDst, int dstW, int dstH,
                           void * pSrc, int srcW, int srcH,
                           int x, int y, double alpha, DWORD transColor);

void    _ISResizeRGB(void * pSrc, int srcW, int srcH,
                     void * pDst, int dstW, int dstH);

void    _ISDecimateRGB(void * pSrc, int srcW, int srcH,
                       void * pDst, int dstW, int dstH);

void    _ISDrawTextOnRGB2(void * pImage, int nWidth, int nHeight,
                          const char * szText, LOGFONTA * pLogFont,
                          int nX, int nY, DWORD dwColor);

BYTE *  _ISHBITMAPToRGB(HBITMAP hBitmap, int * pWidth, int * pHeight,
                         HDC hDC, void * reserved);

HBITMAP _ISLoadResourceBitmap(HINSTANCE hInst, const char * szName, void * reserved);

#endif /* _ISOURCE_H_REPLACEMENT */
