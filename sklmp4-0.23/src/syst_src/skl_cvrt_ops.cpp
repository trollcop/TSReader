/********************************************************
 * Some code. Copyright (C) 2003 by Pascal Massimino.   *
 * All Rights Reserved.      (http://skal.planet-d.net) *
 * For Educational/Academic use ONLY. See 'LICENSE.TXT'.*
 ********************************************************/
/*
 * skl_cvrt_ops.cpp
 *
 *  (tedious) convertion between pixel format
 ********************************************************/

#include "skl.h"
#include "skl_2d/skl_btm_cvrt.h"

extern "C" {

//////////////////////////////////////////////////////////
// Copiers (common to "C" and ASM)

static SKL_CVRT_SIGNATURE(Copy_32_RawC)
{
  while(H-->0) {
    SKL_MEMCPY(Dst, Src, W*4);
    Dst += Dst_BpS; Src += Src_BpS;
  }
}
static SKL_CVRT_SIGNATURE(Copy_24_RawC)
{
  while(H-->0) {
    SKL_MEMCPY(Dst, Src, W*3);
    Dst += Dst_BpS; Src += Src_BpS;
  }
}
static SKL_CVRT_SIGNATURE(Copy_16_RawC)
{
  while(H-->0) {
    SKL_MEMCPY(Dst, Src, W*2);
    Dst += Dst_BpS; Src += Src_BpS;
  }
}
static SKL_CVRT_SIGNATURE(Copy_8_RawC)
{
  while(H-->0) {
    SKL_MEMCPY(Dst, Src, W);
    Dst += Dst_BpS; Src += Src_BpS;
  }
}

#if !defined(SKL_USE_ASM)

//////////////////////////////////////////////////////////
// some macros to help us out

#define M1  (Map + 0  )
#define M2  (Map + 256)
#define M3  (Map + 512)
#define M4  (Map + 768)
#define FROM_16(C)  ( M1[(C)&0xff] | M2[(C)>>8] )
#define FROM_24(S)  ( M1[(S)[0]] | M2[(S)[1]] | M3[(S)[2]] ); (S)+=3
#define FROM_32(S)  ( M1[(S)[0]] | M2[(S)[1]] | M3[(S)[2]] | M4[(S)[3]] ); (S)+=4

#define STORE_24(D,C)   \
  (D)[0] = ((C)>>0) & 0xff; (D)[1] = ((C)>> 8) & 0xff; (D)[2] = ((C)>>16) & 0xff; (D)+=3
  

#define PROLOG(T_OUT,T_IN)  \
  while(H-->0) {                                   \
    const T_IN *S = (const T_IN*)Src;              \
    T_OUT *D = (T_OUT*)Dst;                        \
    for(int x = -W; x<0; ++x) {

#define EPILOG                                     \
    } Dst += Dst_BpS; Src += Src_BpS; }



//////////////////////////////////////////////////////////
// RGB_8_To_*   (<=> color mappers)
//////////////////////////////////////////////////////////

static SKL_CVRT_SIGNATURE(RGB8_To_8_RawC)
{
  Dst += W; Src += W;
  PROLOG(SKL_BYTE, SKL_BYTE)
    D[x] = (SKL_BYTE)Map[S[x]];
  EPILOG
}

static SKL_CVRT_SIGNATURE(RGB8_To_16_RawC)
{
  Dst += 2*W; Src += W;
  PROLOG(SKL_UINT16, SKL_BYTE)
    D[x] = (SKL_UINT16)Map[S[x]];
  EPILOG
}

static SKL_CVRT_SIGNATURE(RGB8_To_24_RawC)
{
  Src += W;
  PROLOG(SKL_BYTE, SKL_BYTE)
    SKL_UINT32 C  = Map[S[x]];
    STORE_24(D,C);
  EPILOG
}

static SKL_CVRT_SIGNATURE(RGB8_To_32_RawC)
{
  Dst += 4*W; Src += W;
  PROLOG(SKL_UINT32, SKL_BYTE)
    D[x] = Map[S[x]];
  EPILOG
}

//////////////////////////////////////////////////////////
// RGB16_To_*
//////////////////////////////////////////////////////////

static SKL_CVRT_SIGNATURE(RGB16_To_8_RawC)
{
  Dst += W; Src += 2*W;
  PROLOG(SKL_BYTE, SKL_UINT16)
    SKL_UINT16 C = S[x];
    D[x] = FROM_16(C);
  EPILOG
}

static SKL_CVRT_SIGNATURE(RGB16_To_16_RawC)
{
  Dst += 2*W; Src += 2*W;
  PROLOG(SKL_UINT16, SKL_UINT16)
    SKL_UINT16 C = S[x];
    D[x] = FROM_16(C);
  EPILOG
}

static SKL_CVRT_SIGNATURE(RGB16_To_24_RawC)
{
  Src += 2*W; Src += 2*W;
  PROLOG(SKL_BYTE, SKL_UINT16)
    SKL_UINT16 C = S[x];
    SKL_UINT32 CC = FROM_16(C);
    STORE_24(D,CC);
  EPILOG
}

static SKL_CVRT_SIGNATURE(RGB16_To_32_RawC)
{
  Dst += 4*W; Src += 2*W;
  PROLOG(SKL_UINT32, SKL_UINT16)
    SKL_UINT16 C = S[x];
    D[x] = FROM_16(C);
  EPILOG
}

//////////////////////////////////////////////////////////
// RGB24_To_*
//////////////////////////////////////////////////////////

static SKL_CVRT_SIGNATURE(RGB24_To_8_RawC)
{
  Dst += W;
  PROLOG(SKL_BYTE, SKL_BYTE)
    SKL_UINT32 C = FROM_24(S);
    D[x] = (SKL_BYTE)C;
  EPILOG
}

static SKL_CVRT_SIGNATURE(RGB24_To_16_RawC)
{
  Dst += 2*W;
  PROLOG(SKL_UINT16, SKL_BYTE)
    SKL_UINT32 C = FROM_24(S);
    D[x] = (SKL_UINT16)C;
  EPILOG
}

static SKL_CVRT_SIGNATURE(RGB24_To_24_RawC)
{
  PROLOG(SKL_BYTE, SKL_BYTE)
    SKL_UINT32 C = FROM_24(S);
    STORE_24(D,C);
  EPILOG
}

static SKL_CVRT_SIGNATURE(RGB24_To_32_RawC)
{
  Dst += 4*W;
  PROLOG(SKL_UINT32, SKL_BYTE)
    SKL_UINT32 C = FROM_24(S);
    D[x] = C;
  EPILOG
}

//////////////////////////////////////////////////////////
// RGB32_To_*
//////////////////////////////////////////////////////////

static SKL_CVRT_SIGNATURE(RGB32_To_8_RawC)
{
  Dst += W;
  PROLOG(SKL_BYTE, SKL_BYTE)
    D[x] = (SKL_BYTE)FROM_32(S);
  EPILOG
}

static SKL_CVRT_SIGNATURE(RGB32_To_16_RawC)
{
  Dst += 2*W;
  PROLOG(SKL_UINT16, SKL_BYTE)
    D[x] = (SKL_UINT16)FROM_32(S);
  EPILOG
}

static SKL_CVRT_SIGNATURE(RGB32_To_24_RawC)
{
  PROLOG(SKL_BYTE, SKL_BYTE)
    SKL_UINT32 C = FROM_32(S);
    STORE_24(D,C);
  EPILOG
}
static SKL_CVRT_SIGNATURE(RGB32_To_32_RawC)
{
  Dst += 4*W;
  PROLOG(SKL_UINT32, SKL_BYTE)
    D[x] = (SKL_UINT32)FROM_32(S);
  EPILOG
}

//////////////////////////////////////////////////////////
// C-version
//////////////////////////////////////////////////////////

static const SKL_CVRT_FUNC Converters[4][4] = {
  { RGB8_To_8_RawC,  RGB8_To_16_RawC,  RGB8_To_24_RawC,  RGB8_To_32_RawC  },
  { RGB16_To_8_RawC, RGB16_To_16_RawC, RGB16_To_24_RawC, RGB16_To_32_RawC },
  { RGB24_To_8_RawC, RGB24_To_16_RawC, RGB24_To_24_RawC, RGB24_To_32_RawC },
  { RGB32_To_8_RawC, RGB32_To_16_RawC, RGB32_To_24_RawC, RGB32_To_32_RawC }
};
static const SKL_CVRT_FUNC Copiers[4] = {
  Copy_8_RawC, Copy_16_RawC, Copy_24_RawC, Copy_32_RawC
};

#endif   /* !SKL_USE_ASM */

//////////////////////////////////////////////////////////
// ASM-version
//////////////////////////////////////////////////////////

#if defined(SKL_USE_ASM)

extern SKL_CVRT_SIGNATURE(Skl_RGB8_To_8_ASM);
extern SKL_CVRT_SIGNATURE(Skl_RGB8_To_16_ASM);
extern SKL_CVRT_SIGNATURE(Skl_RGB8_To_24_ASM);
extern SKL_CVRT_SIGNATURE(Skl_RGB8_To_32_ASM);

extern SKL_CVRT_SIGNATURE(Skl_RGB16_To_8_ASM);
extern SKL_CVRT_SIGNATURE(Skl_RGB16_To_16_ASM);
extern SKL_CVRT_SIGNATURE(Skl_RGB16_To_24_ASM);
extern SKL_CVRT_SIGNATURE(Skl_RGB16_To_32_ASM);

extern SKL_CVRT_SIGNATURE(Skl_RGB24_To_8_ASM);
extern SKL_CVRT_SIGNATURE(Skl_RGB24_To_16_ASM);
extern SKL_CVRT_SIGNATURE(Skl_RGB24_To_24_ASM);
extern SKL_CVRT_SIGNATURE(Skl_RGB24_To_32_ASM);

extern SKL_CVRT_SIGNATURE(Skl_RGB32_To_8_ASM);
extern SKL_CVRT_SIGNATURE(Skl_RGB32_To_16_ASM);
extern SKL_CVRT_SIGNATURE(Skl_RGB32_To_24_ASM);
extern SKL_CVRT_SIGNATURE(Skl_RGB32_To_32_ASM);

static const SKL_CVRT_FUNC Converters[4][4] = {
  { Skl_RGB8_To_8_ASM, Skl_RGB8_To_16_ASM, Skl_RGB8_To_24_ASM, Skl_RGB8_To_32_ASM },
  { Skl_RGB16_To_8_ASM, Skl_RGB16_To_16_ASM, Skl_RGB16_To_24_ASM, Skl_RGB16_To_32_ASM },
  { Skl_RGB24_To_8_ASM, Skl_RGB24_To_16_ASM, Skl_RGB24_To_24_ASM, Skl_RGB24_To_32_ASM },
  { Skl_RGB32_To_8_ASM, Skl_RGB32_To_16_ASM, Skl_RGB32_To_24_ASM, Skl_RGB32_To_32_ASM }
};
static const SKL_CVRT_FUNC Copiers[4] = { 
  Copy_8_RawC, Copy_16_RawC, Copy_24_RawC, Copy_32_RawC
};

#endif  /* SKL_USE_ASM */

SKL_CVRT_FUNC Skl_Get_Cvrt_Convert_Ops(int In, int Out) {
  return Converters[In-1][Out-1];
}
SKL_CVRT_FUNC Skl_Get_Cvrt_Copy_Ops(int Quant) {
  return Copiers[Quant-1];
}


//////////////////////////////////////////////////////////

} /* extern "C" */
