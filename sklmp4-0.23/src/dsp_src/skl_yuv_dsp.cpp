/********************************************************
 * Some code. Copyright (C) 2003 by Pascal Massimino.   *
 * All Rights Reserved.      (http://skal.planet-d.net) *
 * For Educational/Academic use ONLY. See 'LICENSE.TXT'.*
 ********************************************************/
/*
 * skl_yuv_dsp.cpp
 *
 *   Low-level RGB<->YUV convertion
 ********************************************************/

#include "skl.h"
#include "skl_syst/skl_dsp.h"

extern "C" {

//////////////////////////////////////////////////////////

#define YUV_FUNC_DCL(NAME) void (NAME)(SKL_BYTE *Y, SKL_BYTE *U, SKL_BYTE *V, \
                                       const int Dst_BpS, \
                                       const SKL_BYTE *Src, \
                                       const int Src_BpS, \
                                       const int Width, const int Height)
#define RGB_FUNC_DCL(NAME) void (NAME)(SKL_BYTE *RGB, \
                                       const int Dst_BpS, \
                                       const SKL_BYTE *Y, const SKL_BYTE *U, const SKL_BYTE *V, \
                                       const int Src_BpS, \
                                       const int Width, const int Height)

extern "C" YUV_FUNC_DCL(Skl_RGB32_To_YUV_C);
extern "C" YUV_FUNC_DCL(Skl_RGB24_To_YUV_C);
extern "C" YUV_FUNC_DCL(Skl_RGB565_To_YUV_C);
extern "C" void Skl_Init_RGB_To_YUV_Table(int Transfer_Type);

SKL_UINT16 Skl_RGB_To_YUV_Tab[3][256][4];

//////////////////////////////////////////////////////////
// Raw-C version

extern "C" SKL_YUV_DSP Skl_YUV_DSP_C;

void Skl_Init_RGB_To_YUV_Table(int Transfer_Type)
{
  int i;
  for(i=0; i<256; ++i)
  {
      // Y
    Skl_RGB_To_YUV_Tab[0][i][3] =                    0;
    Skl_RGB_To_YUV_Tab[0][i][2] =  66*i + 128+  16*256;
    Skl_RGB_To_YUV_Tab[0][i][1] = -38*i + 128+ 128*256;
    Skl_RGB_To_YUV_Tab[0][i][0] = 112*i + 128+ 128*256;

      // U
    Skl_RGB_To_YUV_Tab[1][i][3] =     0;
    Skl_RGB_To_YUV_Tab[1][i][2] = 129*i;
    Skl_RGB_To_YUV_Tab[1][i][1] =  74*i;
    Skl_RGB_To_YUV_Tab[1][i][0] =  94*i;

      // V
    Skl_RGB_To_YUV_Tab[2][i][3] =     0;
    Skl_RGB_To_YUV_Tab[2][i][2] =  25*i;
    Skl_RGB_To_YUV_Tab[2][i][1] = 112*i;
    Skl_RGB_To_YUV_Tab[2][i][0] =  18*i;
  }
}

//////////////////////////////////////////////////////////
// x86-ASM version

#define Skl_YUV_DSP_x86 Skl_YUV_DSP_C

//////////////////////////////////////////////////////////
// MMX version

#ifdef SKL_USE_ASM

extern YUV_FUNC_DCL(Skl_RGB565_To_YUV_MMX);
extern RGB_FUNC_DCL(Skl_YUV_To_RGB565_MMX);
extern YUV_FUNC_DCL(Skl_RGB32_To_YUV_MMX);
extern RGB_FUNC_DCL(Skl_YUV_To_RGB32_MMX);
extern RGB_FUNC_DCL(Skl_YUV_To_RGB24_MMX);

extern SKL_INT16 Skl_YUV_Tab32_MMX[3][256][4];  // 6k
//extern SKL_INT16 Skl_YUV_Tab24_MMX[3][256][4];  // 6k

SKL_INT16 Skl_YUV_Tab32_MMX[3][256][4];
//SKL_INT16 Skl_YUV_Tab24_MMX[3][256][4];
SKL_UINT32 Skl_RGB32_To_565_MMX[3][258][2];

static void Init_Transfer_Matrix(float M[3][3], float Offsets[3])
{
    // [R,G,B] = M x ( [Y,U,V] - [Offsets] )

  for(int i=0; i<256; ++i)
  {
    float y = 1.f*i - Offsets[0];
    float u = 1.f*i - Offsets[1];
    float v = 1.f*i - Offsets[2];

    Skl_YUV_Tab32_MMX[0][i][2] = (int)( .5 + M[0][1]*u );
    Skl_YUV_Tab32_MMX[0][i][1] = (int)( .5 + M[1][1]*u );
    Skl_YUV_Tab32_MMX[0][i][0] = (int)( .5 + M[2][1]*u );
    Skl_YUV_Tab32_MMX[0][i][3] = 0;
    Skl_YUV_Tab32_MMX[1][i][2] = (int)( .5 + M[0][2]*v );
    Skl_YUV_Tab32_MMX[1][i][1] = (int)( .5 + M[1][2]*v );
    Skl_YUV_Tab32_MMX[1][i][0] = (int)( .5 + M[2][2]*v );
    Skl_YUV_Tab32_MMX[1][i][3] = 0;
    Skl_YUV_Tab32_MMX[2][i][2] = (int)( .5 + M[0][0]*y );
    Skl_YUV_Tab32_MMX[2][i][1] = (int)( .5 + M[1][0]*y );
    Skl_YUV_Tab32_MMX[2][i][0] = (int)( .5 + M[2][0]*y );
    Skl_YUV_Tab32_MMX[2][i][3] = 0;
  }
}

//////////////////////////////////////////////////////////
//  YUV <=> RGB
//
// R = Y +            + 1.40200.Cr 
// G = Y - 0.34414.Cb - 0.71414.Cr
// B = Y + 1.77200.Cb
//
// Y  = 0.29900.R + 0.58700.G + 0.11400.B
// Cb =-0.16874.R - 0.33126.G + 0.50000.B + 1/2
// Cr = 0.50000.R - 0.41869.G - 0.08131.B + 1/2
//
//  This representation maps RGB=[0..255]^3 toward 
//  YCbCr = [0..255]^3. However, for jpg-like encoding, 
//  we often have YCbCr ranging in [-128,127]^3.
//////////////////////////////////////////////////////////

static int Init_YUV_RGB_MMX(int Transfer_Type)
{
    // [0]: R = Y +       a.U
    // [1]: G = Y + c.V + b.U
    // [2]: B = Y + d.V

  static float Mo[][3][3] = {
  { { 1.f,  1.539844f,  0.        },
    { 1.f, -0.457390f, -0.182861f },
    { 1.f,  0.,         1.814373f } }, // ITU-R Rec. 709 (1990)
  { { 1.f,  1.368750f,  0.        },
    { 1.f, -0.695972f, -0.324457f },
    { 1.f,  0.,         1.740266f } }, // ITU-R Rec. 624-4 System M
  { { 1.f,  1.370703f,  0.        },
    { 1.f, -0.698200f, -0.336460f },
    { 1.f,  0.,         1.732443f } }, // ITU-R Rec. 624-4 System B-zG / SMPTE 170M
  { { 1.f,  1.540827f,  0.        },
    { 1.f, -0.465986f, -0.221559f },
    { 1.f,  0.,         1.785241f } } //  SMPTE 240M (1987)
  };
  static float To[3] = { 16.f, 128.f, 128.f };

  float (*M[8])[3][3] = { 0, &Mo[0],  // forbidden, Rec709,
                          0, 0,             // Unspecified, Reserved
                         &Mo[1], &Mo[2],
                         &Mo[2], &Mo[3] };

  if (Transfer_Type<0 || Transfer_Type>=8 || M[Transfer_Type]==0)
    return 0;

  Init_Transfer_Matrix(*M[Transfer_Type], To);

    // init RGB32 -> RGB565 on-the-fly conversion tables
  for(int c=0; c<256; ++c) {
    SKL_UINT32 R, G, B;
    R = (c&0xf8)<<8;
    G = (c&0xfc)<<3;
    B = (c&0xf8)>>3;
    Skl_RGB32_To_565_MMX[0][c][0] = B;
    Skl_RGB32_To_565_MMX[0][c][1] = B << 16;
    Skl_RGB32_To_565_MMX[1][c][0] = G;
    Skl_RGB32_To_565_MMX[1][c][1] = G << 16;
    Skl_RGB32_To_565_MMX[2][c][0] = R;
    Skl_RGB32_To_565_MMX[2][c][1] = R << 16;
  }
    // read overflow protection...
  Skl_RGB32_To_565_MMX[0][257][0] = 0;
  Skl_RGB32_To_565_MMX[0][257][1] = 0;
  Skl_RGB32_To_565_MMX[1][257][0] = 0;
  Skl_RGB32_To_565_MMX[1][257][1] = 0;
  Skl_RGB32_To_565_MMX[2][257][0] = 0;
  Skl_RGB32_To_565_MMX[2][257][1] = 0;

  Skl_Init_RGB_To_YUV_Table(Transfer_Type);
  return 1;
}

static SKL_YUV_DSP Skl_YUV_DSP_MMX =  {
  "YUV-MMX", Skl_Switch_MMX,
  Init_YUV_RGB_MMX,
  Skl_RGB565_To_YUV_C, Skl_YUV_To_RGB565_MMX,
  Skl_RGB24_To_YUV_C,  Skl_YUV_To_RGB24_MMX,
  Skl_RGB32_To_YUV_C,  Skl_YUV_To_RGB32_MMX,
};

#else 
#define Skl_YUV_DSP_MMX Skl_YUV_DSP_C
#endif

//////////////////////////////////////////////////////////
// SSE version

#define Skl_YUV_DSP_SSE  Skl_YUV_DSP_MMX
#define Skl_YUV_DSP_SSE2 Skl_YUV_DSP_MMX

//////////////////////////////////////////////////////////
// Reference version

extern "C" SKL_YUV_DSP Skl_YUV_DSP_Ref;

//////////////////////////////////////////////////////////
// Alternative version

#define Skl_YUV_DSP_Alt Skl_YUV_DSP_C

//////////////////////////////////////////////////////////
// Initialization
//////////////////////////////////////////////////////////

int Skl_Init_YUV_DSP(SKL_YUV_DSP *Dsp, SKL_CPU_FEATURE Cpu)
{
  SKL_ASSERT(Dsp!=0);
  if (Cpu==SKL_CPU_DETECT) Cpu = Skl_Detect_CPU_Feature();

  if      (Cpu==SKL_CPU_C)    *Dsp = Skl_YUV_DSP_C;
  else if (Cpu==SKL_CPU_X86)  *Dsp = Skl_YUV_DSP_x86;
  else if (Cpu==SKL_CPU_MMX)  *Dsp = Skl_YUV_DSP_MMX;
  else if (Cpu==SKL_CPU_SSE)  *Dsp = Skl_YUV_DSP_SSE;
  else if (Cpu==SKL_CPU_SSE2) *Dsp = Skl_YUV_DSP_SSE2;
  else if (Cpu==SKL_CPU_REF)  *Dsp = Skl_YUV_DSP_Ref;
  else                        *Dsp = Skl_YUV_DSP_C;

  if (Dsp->Switch_Off==0) Dsp->Switch_Off = Skl_Get_Switch(Cpu);
  if (Dsp->Init!=0) Dsp->Init(1);  // default init

  return 1;
}

//////////////////////////////////////////////////////////

} /* extern "C" */
