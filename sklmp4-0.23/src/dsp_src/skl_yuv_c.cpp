/********************************************************
 * Some code. Copyright (C) 2003 by Pascal Massimino.   *
 * All Rights Reserved.      (http://skal.planet-d.net) *
 * For Educational/Academic use ONLY. See 'LICENSE.TXT'.*
 ********************************************************/
/*
 *  skl_yuv_c.cpp
 *
 *    YUV <-> RGB
 *
 ********************************************************/

#include "skl.h"
#include "skl_syst/skl_dsp.h"
#include "skl_2d/skl_btm_cvrt.h"

extern "C" {

extern SKL_YUV_DSP Skl_YUV_DSP_C;
extern SKL_YUV_DSP Skl_YUV_DSP_Ref;

#define RGB_TO_YUV_FUNC_DCL(Name)   \
void Name(SKL_BYTE *Y, SKL_BYTE *U, SKL_BYTE *V, \
                 const int Dst_BpS,               \
                 const SKL_BYTE *RGB, const int Src_BpS, \
                 const int Width, const int Height)

#define YUV_TO_RGB_FUNC_DCL(Name)   \
void Name(SKL_BYTE *RGB, const int Dst_BpS, \
                 const SKL_BYTE *Y, const SKL_BYTE *U, const SKL_BYTE *V, \
                 const int Src_BpS, \
                 const int Width, const int Height)

extern "C" RGB_TO_YUV_FUNC_DCL(Skl_RGB32_To_YUV_C);
extern "C" RGB_TO_YUV_FUNC_DCL(Skl_RGB24_To_YUV_C);
extern "C" RGB_TO_YUV_FUNC_DCL(Skl_RGB565_To_YUV_C);

//////////////////////////////////////////////////////////
// Ref-impl
//////////////////////////////////////////////////////////

#define RGB32_TO_RGB(V)  r=((V)>>16)&0xff; g=((V)>>8)&0xff; b=((V)>>0)&0xff; 
#define RGB24_TO_RGB(V)  r=(V)[0]; g=(V)[1]; b=(V)[2]
#define RGB565_TO_RGB(V)  r=((V)>>8)&0xf8; g=((V)>>3)&0xfc; b=((V)<<3)&0xf8

#define RGB_TO_Y(r,g,b)   ( 66*(r)+129*(g) + 25*(b) + (128+ 16*256))
#define RGB_TO_U(r,g,b)   (-38*(r)- 74*(g) +112*(b) + (128+128*256))
#define RGB_TO_V(r,g,b)   (112*(r)- 94*(g) - 18*(b) + (128+128*256))

#define SAT(x) ( (x)<0 ? 0 : (x)>255 ? 255 : (x) )
#define TO_RGB32(Y) ((SAT(Y+Bo)<<16)&0xff0000) | ((SAT(Y-Go)<<8)&0x00ff00) | (SAT(Y+Ro)&0x0000ff)
#define TO_RGB24(D,Y) \
  D[0] = SAT(Y[0]+Bo); D[1] = SAT(Y[0]-Go); D[2] = SAT(Y[0]+Ro); \
  D[3] = SAT(Y[1]+Bo); D[4] = SAT(Y[1]-Go); D[5] = SAT(Y[1]+Ro); \

#define TO_RGB565(Y) ((SAT(Y+Bo)<<8)&0xf800) | ((SAT(Y-Go)<<3)&0x07e0) | ((SAT(Y+Ro)>>3))


RGB_TO_YUV_FUNC_DCL(Skl_RGB32_To_YUV_Ref)
{
  for(int y=0; y<Height/2; y++)
  {
    SKL_BYTE *pDst1 = Y;
    SKL_BYTE *pDst2 = Y+Dst_BpS;
    const SKL_UINT32 *pSrc1 = (SKL_UINT32*)RGB;
    const SKL_UINT32 *pSrc2 = (SKL_UINT32*)(RGB + Src_BpS);
    for(int x=0; x<Width/2; x++)
    {
      int r,g,b, u,v;

      RGB32_TO_RGB(pSrc1[2*x+0]);
      pDst1[2*x+0] = RGB_TO_Y(r,g,b) >> 8;
      u = RGB_TO_U(r,g,b);
      v = RGB_TO_V(r,g,b);
      RGB32_TO_RGB(pSrc1[2*x+1]);
      pDst1[2*x+1] = RGB_TO_Y(r,g,b) >> 8;
      u += RGB_TO_U(r,g,b);
      v += RGB_TO_V(r,g,b);

      RGB32_TO_RGB(pSrc2[2*x+0]);
      pDst2[2*x+0] = RGB_TO_Y(r,g,b) >> 8;
      u += RGB_TO_U(r,g,b);
      v += RGB_TO_V(r,g,b);
      RGB32_TO_RGB(pSrc2[2*x+1]);
      pDst2[2*x+1] = RGB_TO_Y(r,g,b) >> 8;
      u += RGB_TO_U(r,g,b);
      v += RGB_TO_V(r,g,b);

      U[x] = u>>10;
      V[x] = v>>10;
    }
    RGB += 2*Src_BpS;
    Y   += 2*Dst_BpS;
    U   += Dst_BpS;
    V   += Dst_BpS;
  }
}

static YUV_TO_RGB_FUNC_DCL(YUV_To_RGB32_Ref)
{
  for(int y=0; y<Height/2; y++)
  {
    SKL_UINT32 *pDst1 = (SKL_UINT32*)RGB;
    SKL_UINT32 *pDst2 = (SKL_UINT32*)(RGB+Dst_BpS);
    const SKL_BYTE *pYSrc1 = Y;
    const SKL_BYTE *pYSrc2 = Y + Src_BpS;
    for(int x=0; x<Width/2; x++)
    {
      const int Uo = U[x]-128;
      const int Vo = V[x]-128;
      const int Ro = (91181*Uo)>>16;
      const int Go = (22553*Vo + 46801*Uo)>>16;
      const int Bo = (116129*Vo)>>16;

      pDst1[0] = TO_RGB32(pYSrc1[0]);
      pDst1[1] = TO_RGB32(pYSrc1[1]);
      pDst1+=2;
      pYSrc1+=2;
      pDst2[0] = TO_RGB32(pYSrc2[0]);
      pDst2[1] = TO_RGB32(pYSrc2[1]);
      pDst2+=2;
      pYSrc2+=2;
    }
    RGB  += 2*Dst_BpS;
    Y    += 2*Src_BpS;
    U    += Src_BpS;
    V    += Src_BpS;
  }
}

RGB_TO_YUV_FUNC_DCL(Skl_RGB24_To_YUV_Ref)
{ 
  for(int y=0; y<Height/2; y++)
  {
    SKL_BYTE *pDst1 = Y;
    SKL_BYTE *pDst2 = Y+Dst_BpS;
    const SKL_BYTE *pSrc1 = RGB;
    const SKL_BYTE *pSrc2 = RGB + Src_BpS;
    for(int x=0; x<Width/2; x++)
    {
      int r,g,b, u,v;

      RGB24_TO_RGB(&pSrc1[6*x+0]);
      pDst1[2*x+0] = RGB_TO_Y(r,g,b) >> 8;
      u = RGB_TO_U(r,g,b);
      v = RGB_TO_V(r,g,b);
      RGB24_TO_RGB(&pSrc1[6*x+3]);
      pDst1[2*x+1] = RGB_TO_Y(r,g,b) >> 8;
      u += RGB_TO_U(r,g,b);
      v += RGB_TO_V(r,g,b);

      RGB24_TO_RGB(&pSrc2[6*x+0]);
      pDst2[2*x+0] = RGB_TO_Y(r,g,b) >> 8;
      u += RGB_TO_U(r,g,b);
      v += RGB_TO_V(r,g,b);
      RGB24_TO_RGB(&pSrc2[6*x+3]);
      pDst2[2*x+1] = RGB_TO_Y(r,g,b) >> 8;
      u += RGB_TO_U(r,g,b);
      v += RGB_TO_V(r,g,b);

      U[x] = u>>10;
      V[x] = v>>10;
    }
    RGB += 2*Src_BpS;
    Y   += 2*Dst_BpS;
    U   += Dst_BpS;
    V   += Dst_BpS;
  }
}

static YUV_TO_RGB_FUNC_DCL(YUV_To_RGB24_Ref)
{
  for(int y=0; y<Height/2; y++)
  {
    SKL_BYTE *pDst1 = RGB;
    SKL_BYTE *pDst2 = RGB+Dst_BpS;
    const SKL_BYTE *pYSrc1 = Y;
    const SKL_BYTE *pYSrc2 = Y + Src_BpS;
    for(int x=0; x<Width/2; x++)
    {
      const int Uo = U[x]-128;
      const int Vo = V[x]-128;
      const int Ro = (91181*Uo)>>16;
      const int Go = (22553*Vo + 46801*Uo)>>16;
      const int Bo = (116129*Vo)>>16;

      TO_RGB24(pDst1,   pYSrc1)
      pDst1 +=6;
      pYSrc1+=2;
      TO_RGB24(pDst2,   pYSrc2)
      pDst2 +=6;
      pYSrc2+=2;
    }
    RGB  += 2*Dst_BpS;
    Y    += 2*Src_BpS;
    U    += Src_BpS;
    V    += Src_BpS;
  }
}

RGB_TO_YUV_FUNC_DCL(Skl_RGB565_To_YUV_Ref)
{
  for(int y=0; y<Height/2; y++)
  {
    SKL_BYTE *pDst1 = Y;
    SKL_BYTE *pDst2 = Y+Dst_BpS;
    const SKL_UINT16 *pSrc1 = (SKL_UINT16*)RGB;
    const SKL_UINT16 *pSrc2 = (SKL_UINT16*)(RGB + Src_BpS);
    for(int x=0; x<Width/2; x++)
    {
      int r,g,b, u,v;

      RGB565_TO_RGB(pSrc1[2*x+0]);
      pDst1[2*x+0] = RGB_TO_Y(r,g,b) >> 8;
      u = RGB_TO_U(r,g,b);
      v = RGB_TO_V(r,g,b);
      RGB565_TO_RGB(pSrc1[2*x+1]);
      pDst1[2*x+1] = RGB_TO_Y(r,g,b) >> 8;
      u += RGB_TO_U(r,g,b);
      v += RGB_TO_V(r,g,b);

      RGB565_TO_RGB(pSrc2[2*x+0]);
      pDst2[2*x+0] = RGB_TO_Y(r,g,b) >> 8;
      u += RGB_TO_U(r,g,b);
      v += RGB_TO_V(r,g,b);
      RGB565_TO_RGB(pSrc2[2*x+1]);
      pDst2[2*x+1] = RGB_TO_Y(r,g,b) >> 8;
      u += RGB_TO_U(r,g,b);
      v += RGB_TO_V(r,g,b);

      U[x] = u>>10;
      V[x] = v>>10;
    }
    RGB += 2*Src_BpS;
    Y   += 2*Dst_BpS;
    U   += Dst_BpS;
    V   += Dst_BpS;
  }
}

static YUV_TO_RGB_FUNC_DCL(YUV_To_RGB565_Ref)
{
  for(int y=0; y<Height/2; y++)
  {
    SKL_UINT16 *pDst1 = (SKL_UINT16*)RGB;
    SKL_UINT16 *pDst2 = (SKL_UINT16*)(RGB+Dst_BpS);
    const SKL_BYTE *pYSrc1 = Y;
    const SKL_BYTE *pYSrc2 = Y + Src_BpS;
    for(int x=0; x<Width/2; x++)
    {
      const int Uo = U[x]-128;
      const int Vo = V[x]-128;
      const int Ro = (91181*Uo)>>16;
      const int Go = (22553*Vo + 46801*Uo)>>16;
      const int Bo = (116129*Vo)>>16;

      pDst1[0] = TO_RGB565(pYSrc1[0]);
      pDst1[1] = TO_RGB565(pYSrc1[1]);
      pDst1+=2;
      pYSrc1+=2;
      pDst2[0] = TO_RGB565(pYSrc2[0]);
      pDst2[1] = TO_RGB565(pYSrc2[1]);
      pDst2+=2;  
      pYSrc2+=2;
    }
    RGB  += 2*Dst_BpS;
    Y    += 2*Src_BpS;
    U    += Src_BpS;
    V    += Src_BpS;
  }
}


static int Init_YUV_RGB_Ref(int Transfer_Type) {
  if (Transfer_Type<0 || Transfer_Type>=8)
    return 0;
  return 1;
}

//////////////////////////////////////////////////////////

SKL_YUV_DSP Skl_YUV_DSP_Ref = { 
  "YUV-Ref", Skl_Switch_None,

  Init_YUV_RGB_Ref,
  Skl_RGB565_To_YUV_Ref, YUV_To_RGB565_Ref,
  Skl_RGB24_To_YUV_Ref, YUV_To_RGB24_Ref,
  Skl_RGB32_To_YUV_Ref, YUV_To_RGB32_Ref,
};

//////////////////////////////////////////////////////////
// Raw-C impl.
//////////////////////////////////////////////////////////

static SKL_UINT16 CTable_565[3][256+2*384]; // 6k
static SKL_UINT32 CTable_32[3][256+2*384];  // 12k
static SKL_UINT32 Ycc_Cr_To_R[256];         // 1k
static SKL_UINT32 Ycc_Cb_To_G[256];
static SKL_UINT32 Ycc_Cr_To_G[256];
static SKL_UINT32 Ycc_Cb_To_B[256];

extern "C" void Skl_Init_RGB_To_YUV_Table(int Transfer_Type);
extern SKL_UINT16 Skl_RGB_To_YUV_Tab[3][256][4];


#define PROLOG(Size)                  \
    Y += Width;                       \
    U += Width/2; V += Width/2;       \
    RGB += Width*Size;                \
    for(int j=Height/2; j>0; --j) {   \
      const SKL_BYTE *Y2 = Y + Src_BpS; \
      SKL_BYTE *RGB2 = RGB + Dst_BpS; \
      for(int i=-Width/2; i<0; i++) { \

#define EPILOG        \
    }                 \
    RGB += 2*Dst_BpS; \
    Y   += 2*Src_BpS; \
    U   +=   Src_BpS; \
    V   +=   Src_BpS; \
  }

#define GET_CC(x)                         \
  const SKL_UINT32 cb = U[x];             \
  const SKL_UINT32 cr = V[x];             \
  const SKL_UINT32 R = Ycc_Cr_To_R[cr];   \
  const SKL_UINT32 G = Ycc_Cb_To_G[cb] + Ycc_Cr_To_G[cr]; \
  const SKL_UINT32 B = Ycc_Cb_To_B[cb]

#define PUT_Y(T,x,YSrc, RGB_Dst, CTab)  { \
  const SKL_UINT32 y = YSrc[x]; \
  ((T*)RGB_Dst)[x] = ( CTab[0][R+y] | CTab[1][G+y] | CTab[2][B+y] ); \
}

#define PUT_Y3(x,YSrc, RGB_Dst, CTab)  { \
  const SKL_UINT32 y = YSrc[x]; \
  const SKL_UINT32 rgb = ( CTab[0][R+y] | CTab[1][G+y] | CTab[2][B+y] ); \
  RGB_Dst[3*(x)+2] = (rgb >> 0) & 0xff;  \
  RGB_Dst[3*(x)+1] = (rgb >> 8) & 0xff;  \
  RGB_Dst[3*(x)+0] = (rgb >>16) & 0xff;  \
}

//////////////////////////////////////////////////////////

RGB_TO_YUV_FUNC_DCL(Skl_RGB565_To_YUV_C) {
  for(int y=0; y<Height/2; y++)
  {
    SKL_BYTE *pDst1 = Y;
    SKL_BYTE *pDst2 = Y+Dst_BpS;
    const SKL_UINT16 *pSrc1 = (SKL_UINT16*)RGB;
    const SKL_UINT16 *pSrc2 = (SKL_UINT16*)(RGB + Src_BpS);
    for(int x=0; x<Width/2; x++)
    {
      int r,g,b, u,v;

      RGB565_TO_RGB(pSrc1[2*x+0]);
      pDst1[2*x+0] = (Skl_RGB_To_YUV_Tab[0][r][2] + Skl_RGB_To_YUV_Tab[1][g][2] +  Skl_RGB_To_YUV_Tab[2][b][2])>>8;
      u  = Skl_RGB_To_YUV_Tab[0][r][1] - Skl_RGB_To_YUV_Tab[1][g][1] + Skl_RGB_To_YUV_Tab[2][b][1];
      v  = Skl_RGB_To_YUV_Tab[0][r][0] - Skl_RGB_To_YUV_Tab[1][g][0] - Skl_RGB_To_YUV_Tab[2][b][0];
      RGB565_TO_RGB(pSrc1[2*x+1]);
      pDst1[2*x+1] = (Skl_RGB_To_YUV_Tab[0][r][2] + Skl_RGB_To_YUV_Tab[1][g][2] +  Skl_RGB_To_YUV_Tab[2][b][2])>>8;
      u += Skl_RGB_To_YUV_Tab[0][r][1] - Skl_RGB_To_YUV_Tab[1][g][1] + Skl_RGB_To_YUV_Tab[2][b][1];
      v += Skl_RGB_To_YUV_Tab[0][r][0] - Skl_RGB_To_YUV_Tab[1][g][0] - Skl_RGB_To_YUV_Tab[2][b][0];

      RGB565_TO_RGB(pSrc2[2*x+0]);
      pDst2[2*x+0] = (Skl_RGB_To_YUV_Tab[0][r][2] + Skl_RGB_To_YUV_Tab[1][g][2] +  Skl_RGB_To_YUV_Tab[2][b][2])>>8;
      u += Skl_RGB_To_YUV_Tab[0][r][1] - Skl_RGB_To_YUV_Tab[1][g][1] + Skl_RGB_To_YUV_Tab[2][b][1];
      v += Skl_RGB_To_YUV_Tab[0][r][0] - Skl_RGB_To_YUV_Tab[1][g][0] - Skl_RGB_To_YUV_Tab[2][b][0];
      RGB565_TO_RGB(pSrc2[2*x+1]);
      pDst2[2*x+1] = (Skl_RGB_To_YUV_Tab[0][r][2] + Skl_RGB_To_YUV_Tab[1][g][2] +  Skl_RGB_To_YUV_Tab[2][b][2])>>8;
      u += Skl_RGB_To_YUV_Tab[0][r][1] - Skl_RGB_To_YUV_Tab[1][g][1] + Skl_RGB_To_YUV_Tab[2][b][1];
      v += Skl_RGB_To_YUV_Tab[0][r][0] - Skl_RGB_To_YUV_Tab[1][g][0] - Skl_RGB_To_YUV_Tab[2][b][0];

      U[x] = u>>10;
      V[x] = v>>10;
    }
    RGB += 2*Src_BpS;
    Y   += 2*Dst_BpS;
    U   += Dst_BpS;
    V   += Dst_BpS;
  }
}

static YUV_TO_RGB_FUNC_DCL(YUV_To_RGB565_C)
{
  PROLOG(2)
   GET_CC(i);
   PUT_Y(SKL_UINT16,2*i, Y , RGB , CTable_565); PUT_Y(SKL_UINT16,2*i+1, Y , RGB , CTable_565);
   PUT_Y(SKL_UINT16,2*i, Y2, RGB2, CTable_565); PUT_Y(SKL_UINT16,2*i+1, Y2, RGB2, CTable_565);
  EPILOG
}


RGB_TO_YUV_FUNC_DCL(Skl_RGB24_To_YUV_C) {
  for(int y=0; y<Height/2; y++)
  {
    SKL_BYTE *pDst1 = Y;
    SKL_BYTE *pDst2 = Y+Dst_BpS;
    const SKL_BYTE *pSrc1 = RGB;
    const SKL_BYTE *pSrc2 = RGB + Src_BpS;
    for(int x=0; x<Width/2; x++)
    {
      int r,g,b, u,v;

      RGB24_TO_RGB(&pSrc1[6*x+0]);
      pDst1[2*x+0] = (Skl_RGB_To_YUV_Tab[0][r][2] + Skl_RGB_To_YUV_Tab[1][g][2] +  Skl_RGB_To_YUV_Tab[2][b][2])>>8;
      u  = Skl_RGB_To_YUV_Tab[0][r][1] - Skl_RGB_To_YUV_Tab[1][g][1] + Skl_RGB_To_YUV_Tab[2][b][1];
      v  = Skl_RGB_To_YUV_Tab[0][r][0] - Skl_RGB_To_YUV_Tab[1][g][0] - Skl_RGB_To_YUV_Tab[2][b][0];
      RGB24_TO_RGB(&pSrc1[6*x+3]);
      pDst1[2*x+1] = (Skl_RGB_To_YUV_Tab[0][r][2] + Skl_RGB_To_YUV_Tab[1][g][2] +  Skl_RGB_To_YUV_Tab[2][b][2])>>8;
      u += Skl_RGB_To_YUV_Tab[0][r][1] - Skl_RGB_To_YUV_Tab[1][g][1] + Skl_RGB_To_YUV_Tab[2][b][1];
      v += Skl_RGB_To_YUV_Tab[0][r][0] - Skl_RGB_To_YUV_Tab[1][g][0] - Skl_RGB_To_YUV_Tab[2][b][0];

      RGB24_TO_RGB(&pSrc2[6*x+0]);
      pDst2[2*x+0] = (Skl_RGB_To_YUV_Tab[0][r][2] + Skl_RGB_To_YUV_Tab[1][g][2] +  Skl_RGB_To_YUV_Tab[2][b][2])>>8;
      u += Skl_RGB_To_YUV_Tab[0][r][1] - Skl_RGB_To_YUV_Tab[1][g][1] + Skl_RGB_To_YUV_Tab[2][b][1];
      v += Skl_RGB_To_YUV_Tab[0][r][0] - Skl_RGB_To_YUV_Tab[1][g][0] - Skl_RGB_To_YUV_Tab[2][b][0];
      RGB24_TO_RGB(&pSrc2[6*x+3]);
      pDst2[2*x+1] = (Skl_RGB_To_YUV_Tab[0][r][2] + Skl_RGB_To_YUV_Tab[1][g][2] +  Skl_RGB_To_YUV_Tab[2][b][2])>>8;
      u += Skl_RGB_To_YUV_Tab[0][r][1] - Skl_RGB_To_YUV_Tab[1][g][1] + Skl_RGB_To_YUV_Tab[2][b][1];
      v += Skl_RGB_To_YUV_Tab[0][r][0] - Skl_RGB_To_YUV_Tab[1][g][0] - Skl_RGB_To_YUV_Tab[2][b][0];

      U[x] = u>>10;
      V[x] = v>>10;
    }
    RGB += 2*Src_BpS;
    Y   += 2*Dst_BpS;
    U   += Dst_BpS;
    V   += Dst_BpS;
  }
}

static YUV_TO_RGB_FUNC_DCL(YUV_To_RGB24_C)
{
  PROLOG(3)
   GET_CC(i);
   PUT_Y3(2*i, Y , RGB , CTable_32); PUT_Y3(2*i+1, Y , RGB , CTable_32);
   PUT_Y3(2*i, Y2, RGB2, CTable_32); PUT_Y3(2*i+1, Y2, RGB2, CTable_32);
  EPILOG
}


RGB_TO_YUV_FUNC_DCL(Skl_RGB32_To_YUV_C) {
  for(int y=0; y<Height/2; y++)
  {
    SKL_BYTE *pDst1 = Y;
    SKL_BYTE *pDst2 = Y+Dst_BpS;
    const SKL_UINT32 *pSrc1 = (SKL_UINT32*)RGB;
    const SKL_UINT32 *pSrc2 = (SKL_UINT32*)(RGB + Src_BpS);
    for(int x=0; x<Width/2; x++)
    {
      int r,g,b, u,v;

      RGB32_TO_RGB(pSrc1[2*x+0]);
      pDst1[2*x+0] = (Skl_RGB_To_YUV_Tab[0][r][2] + Skl_RGB_To_YUV_Tab[1][g][2] +  Skl_RGB_To_YUV_Tab[2][b][2])>>8;
      u  = Skl_RGB_To_YUV_Tab[0][r][1] - Skl_RGB_To_YUV_Tab[1][g][1] + Skl_RGB_To_YUV_Tab[2][b][1];
      v  = Skl_RGB_To_YUV_Tab[0][r][0] - Skl_RGB_To_YUV_Tab[1][g][0] - Skl_RGB_To_YUV_Tab[2][b][0];
      RGB32_TO_RGB(pSrc1[2*x+1]);
      pDst1[2*x+1] = (Skl_RGB_To_YUV_Tab[0][r][2] + Skl_RGB_To_YUV_Tab[1][g][2] +  Skl_RGB_To_YUV_Tab[2][b][2])>>8;
      u += Skl_RGB_To_YUV_Tab[0][r][1] - Skl_RGB_To_YUV_Tab[1][g][1] + Skl_RGB_To_YUV_Tab[2][b][1];
      v += Skl_RGB_To_YUV_Tab[0][r][0] - Skl_RGB_To_YUV_Tab[1][g][0] - Skl_RGB_To_YUV_Tab[2][b][0];

      RGB32_TO_RGB(pSrc2[2*x+0]);
      pDst2[2*x+0] = (Skl_RGB_To_YUV_Tab[0][r][2] + Skl_RGB_To_YUV_Tab[1][g][2] +  Skl_RGB_To_YUV_Tab[2][b][2])>>8;
      u += Skl_RGB_To_YUV_Tab[0][r][1] - Skl_RGB_To_YUV_Tab[1][g][1] + Skl_RGB_To_YUV_Tab[2][b][1];
      v += Skl_RGB_To_YUV_Tab[0][r][0] - Skl_RGB_To_YUV_Tab[1][g][0] - Skl_RGB_To_YUV_Tab[2][b][0];
      RGB32_TO_RGB(pSrc2[2*x+1]);
      pDst2[2*x+1] = (Skl_RGB_To_YUV_Tab[0][r][2] + Skl_RGB_To_YUV_Tab[1][g][2] +  Skl_RGB_To_YUV_Tab[2][b][2])>>8;
      u += Skl_RGB_To_YUV_Tab[0][r][1] - Skl_RGB_To_YUV_Tab[1][g][1] + Skl_RGB_To_YUV_Tab[2][b][1];
      v += Skl_RGB_To_YUV_Tab[0][r][0] - Skl_RGB_To_YUV_Tab[1][g][0] - Skl_RGB_To_YUV_Tab[2][b][0];

      U[x] = u>>10;
      V[x] = v>>10;
    }
    RGB += 2*Src_BpS;
    Y   += 2*Dst_BpS;
    U   += Dst_BpS;
    V   += Dst_BpS;
  }
}

static YUV_TO_RGB_FUNC_DCL(YUV_To_RGB32_C)
{
  PROLOG(4)
   GET_CC(i);
   PUT_Y(SKL_UINT32,2*i, Y , RGB , CTable_32); PUT_Y(SKL_UINT32,2*i+1, Y , RGB , CTable_32);
   PUT_Y(SKL_UINT32,2*i, Y2, RGB2, CTable_32); PUT_Y(SKL_UINT32,2*i+1, Y2, RGB2, CTable_32);
  EPILOG
}

//////////////////////////////////////////////////////////
// -- Entry point
//////////////////////////////////////////////////////////

//
//   ISO 14496-2 - Table 6-9 (inverted)
// 
// these 4 floats C[0..3] are the Cb/Cr coefficients in the Ycc->RGB matrix:
//
// R = Y +         + C[0].Cr 
// G = Y + C[1].Cb + C[2].Cr
// B = Y + C[3].Cb
//

static const float Ycc_Cb_Cr_Coeffs_Table[8][4] = 
{
  {1.539844f, -0.182861f, -0.457390f, 1.814373f}, // forbidden
  {1.539844f, -0.182861f, -0.457390f, 1.814373f}, // ITU-R Rec. 709 (1990)
  {       .0,         .0,         .0,        .0}, // Unspecified video
  {       .0,         .0,         .0,        .0}, // Reserved
  {1.368750f, -0.324457f, -0.695972f, 1.740266f}, // ITU-R Rec. 624-4 System M
  {1.370703f, -0.336460f, -0.698200f, 1.732443f}, // ITU-R Rec. 624-4 System B-G
  {1.370703f, -0.336460f, -0.698200f, 1.732443f}, // SMPTE 170M
  {1.540827f, -0.221559f, -0.465986f, 1.785241f}  // SMPTE 240M (1987)
};

//////////////////////////////////////////////////////////

static int Init_YUV_RGB_C(int Transfer_Type)
{
  SKL_ASSERT(Transfer_Type>0 && Transfer_Type<8);
  if (Transfer_Type<0 || Transfer_Type>=8 ||
      Ycc_Cb_Cr_Coeffs_Table[Transfer_Type][0]==0.)
    return 0;

  const float *Coeffs = Ycc_Cb_Cr_Coeffs_Table[Transfer_Type];

  int i;
  for(i=-348; i<256+384; ++i)
  {
    const int C = SAT(i);
    CTable_32[0][i+384]  = (C << 16);  // R
    CTable_32[1][i+384]  = (C <<  8);  // G
    CTable_32[2][i+384]  = (C <<  0);  // B
    CTable_565[0][i+384] = (C << 8) & 0xf800;  // R
    CTable_565[1][i+384] = (C << 3) & 0x07e0;  // G
    CTable_565[2][i+384] = (C >> 3) & 0x001f;  // B
  }
  for(i=0; i<256; ++i)
  {
    Ycc_Cr_To_R[i] = (int)( Coeffs[0]*(i-128) )  +384;
    Ycc_Cb_To_G[i] = (int)( Coeffs[1]*(i-128) )  +384;
    Ycc_Cr_To_G[i] = (int)( Coeffs[2]*(i-128) );
    Ycc_Cb_To_B[i] = (int)( Coeffs[3]*(i-128) )  +384;
  }

  Skl_Init_RGB_To_YUV_Table(Transfer_Type);
  
  return 1;
}

#undef RGB_TO_Y
#undef RGB_TO_U
#undef RGB_TO_V

#undef RGB565_TO_RGB
#undef RGB24_TO_RGB

#undef SAT
#undef TO_RGB32
#undef TO_RGB24
#undef TO_RGB565

//////////////////////////////////////////////////////////

SKL_YUV_DSP Skl_YUV_DSP_C = { 
  "YUV--C-", Skl_Switch_None,

  Init_YUV_RGB_C,
  Skl_RGB565_To_YUV_C, YUV_To_RGB565_C,
  Skl_RGB24_To_YUV_C,  YUV_To_RGB24_C,
  Skl_RGB32_To_YUV_C,  YUV_To_RGB32_C,
};


//////////////////////////////////////////////////////////

}   // extern "C"
