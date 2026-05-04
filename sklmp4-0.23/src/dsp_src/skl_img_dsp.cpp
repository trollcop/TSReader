/********************************************************
 * Some code. Copyright (C) 2003 by Pascal Massimino.   *
 * All Rights Reserved.      (http://skal.planet-d.net) *
 * For Educational/Academic use ONLY. See 'LICENSE.TXT'.*
 ********************************************************/
/*
 * skl_img_dsp.cpp
 *
 *   Low-level processing for data in various format
 *   => Image processing
 ********************************************************/

#include "skl.h"
#include "skl_syst/skl_dsp.h"

extern "C" {

//////////////////////////////////////////////////////////
// Raw-C version

extern SKL_UINT32 Skl_SAD_C(const SKL_BYTE *Src1, const SKL_BYTE *Src2,
                            SKL_INT32 W, SKL_INT32 H, SKL_INT32 BpS);
extern SKL_UINT32 Skl_SAD_4x4_C(const SKL_BYTE *Src1, const SKL_BYTE *Src2, SKL_INT32 BpS);
extern SKL_UINT32 Skl_SAD_8x8_C(const SKL_BYTE *Src1, const SKL_BYTE *Src2, SKL_INT32 BpS);
extern SKL_UINT32 Skl_SAD_16x8_Field_C(const SKL_BYTE *Src1, const SKL_BYTE *Src2, SKL_INT32 BpS);
extern SKL_UINT32 Skl_SAD_16x16_C(const SKL_BYTE *Src1, const SKL_BYTE *Src2, SKL_INT32 BpS);
extern SKL_UINT32 Skl_SAD_16x7_Self_C(const SKL_BYTE *Src1, SKL_INT32 BpS);

extern SKL_UINT32 Skl_SAD_Avrg_4x4_C(const SKL_BYTE *Dst, const SKL_BYTE *Src1, const SKL_BYTE *Src2, SKL_INT32 BpS);
extern SKL_UINT32 Skl_SAD_Avrg_8x8_C(const SKL_BYTE *Dst, const SKL_BYTE *Src1, const SKL_BYTE *Src2, SKL_INT32 BpS);
extern SKL_UINT32 Skl_SAD_Avrg_16x8_C(const SKL_BYTE *Dst, const SKL_BYTE *Src1, const SKL_BYTE *Src2, SKL_INT32 BpS);
extern SKL_UINT32 Skl_SAD_Avrg_16x16_C(const SKL_BYTE *Dst, const SKL_BYTE *Src1, const SKL_BYTE *Src2, SKL_INT32 BpS);

extern SKL_UINT32 Skl_SSD_4x4_C(const SKL_BYTE *Src1, const SKL_BYTE *Src2, SKL_INT32 BpS);
extern SKL_UINT32 Skl_SSD_8x8_C(const SKL_BYTE *Src1, const SKL_BYTE *Src2, SKL_INT32 BpS);
extern SKL_UINT32 Skl_SSD_16x8_Field_C(const SKL_BYTE *Src1, const SKL_BYTE *Src2, SKL_INT32 BpS);
extern SKL_UINT32 Skl_SSD_16x16_C(const SKL_BYTE *Src1, const SKL_BYTE *Src2, SKL_INT32 BpS);

extern SKL_UINT32 Skl_Hadamard_SAD_4x4_C(const SKL_BYTE *Src1, const SKL_BYTE *Src2, SKL_INT32 BpS);
extern SKL_UINT32 Skl_Hadamard_SAD_8x8_C(const SKL_BYTE *Src1, const SKL_BYTE *Src2, SKL_INT32 BpS);
extern SKL_UINT32 Skl_Hadamard_SAD_16x8_Field_C(const SKL_BYTE *Src1, const SKL_BYTE *Src2, SKL_INT32 BpS);
extern SKL_UINT32 Skl_Hadamard_SAD_16x16_C(const SKL_BYTE *Src1, const SKL_BYTE *Src2, SKL_INT32 BpS);

extern SKL_UINT32 Skl_Mean_C(const SKL_BYTE *Src1, SKL_INT32 W, SKL_INT32 H, SKL_INT32 BpS);
extern SKL_UINT32 Skl_Mean_4x4_C(const SKL_BYTE *Src1, SKL_INT32 BpS);
extern SKL_UINT32 Skl_Mean_8x8_C(const SKL_BYTE *Src1, SKL_INT32 BpS);
extern SKL_UINT32 Skl_Mean_16x16_C(const SKL_BYTE *Src1, SKL_INT32 BpS);

extern SKL_UINT32 Skl_Sqr_4x4_C(const SKL_BYTE *Src1, SKL_INT32 BpS);
extern SKL_UINT32 Skl_Sqr_8x8_C(const SKL_BYTE *Src1, SKL_INT32 BpS);
extern SKL_UINT32 Skl_Sqr_16x16_C(const SKL_BYTE *Src1, SKL_INT32 BpS);

extern SKL_UINT32 Skl_Sqr_Dev_16x16_C(const SKL_BYTE *Src1, SKL_INT32 BpS);
extern SKL_UINT32 Skl_Abs_Dev_16x16_C(const SKL_BYTE *Src1, SKL_INT32 BpS);
extern SKL_UINT32 Skl_Hadamard_Dev_16x16_C(const SKL_BYTE *Src1, SKL_INT32 BpS);

extern SKL_UINT32 Skl_Square_Dev_C(const SKL_BYTE *Src1, SKL_INT32 W, SKL_INT32 H, SKL_INT32 BpS);
extern SKL_UINT32 Skl_Abs_Dev_C(const SKL_BYTE *Src1, SKL_INT32 W, SKL_INT32 H, SKL_INT32 BpS);

extern void Skl_Smooth_18x18_To_8x8_C(SKL_BYTE *Dst, int Dst_BpS, const SKL_BYTE *Src, int Src_BpS);
extern void Skl_Gradx_18x18_To_8x8_C(SKL_INT8 *Dst, int Dst_BpS, const SKL_BYTE *Src, int Src_BpS);
extern void Skl_Grady_18x18_To_8x8_C(SKL_INT8 *Dst, int Dst_BpS, const SKL_BYTE *Src, int Src_BpS);
extern void Skl_Grad2_18x18_To_8x8_C(SKL_BYTE *Dst, int Dst_BpS, const SKL_BYTE *Src, int Src_BpS);

static SKL_IMG_DSP Img_DSP_C = { 
  "IMG--C-", Skl_Switch_None,

  Skl_SAD_4x4_C, Skl_SAD_8x8_C, Skl_SAD_16x8_Field_C, Skl_SAD_16x16_C, Skl_SAD_16x7_Self_C,
  Skl_SSD_4x4_C, Skl_SSD_8x8_C, Skl_SSD_16x8_Field_C, Skl_SSD_16x16_C, 
  Skl_Hadamard_SAD_4x4_C, Skl_Hadamard_SAD_8x8_C, Skl_Hadamard_SAD_16x8_Field_C, Skl_Hadamard_SAD_16x16_C,

  Skl_Mean_4x4_C, Skl_Mean_8x8_C, Skl_Mean_16x16_C,
  Skl_Sqr_4x4_C, Skl_Sqr_8x8_C, Skl_Sqr_16x16_C,

  Skl_Sqr_Dev_16x16_C,
  Skl_Abs_Dev_16x16_C,
  Skl_Hadamard_Dev_16x16_C,

  Skl_SAD_C, Skl_Mean_C, 
  Skl_Square_Dev_C, 
  Skl_Abs_Dev_C,

  Skl_SAD_Avrg_4x4_C,  Skl_SAD_Avrg_8x8_C,
  Skl_SAD_Avrg_16x8_C, Skl_SAD_Avrg_16x16_C,

  Skl_Smooth_18x18_To_8x8_C,
  Skl_Gradx_18x18_To_8x8_C, Skl_Grady_18x18_To_8x8_C, Skl_Grad2_18x18_To_8x8_C
};

//////////////////////////////////////////////////////////
// no x86-ASM version

#define Img_DSP_x86 Img_DSP_C

//////////////////////////////////////////////////////////
// MMX version

#ifdef SKL_USE_ASM

extern SKL_UINT32 Skl_SAD_4x4_MMX(const SKL_BYTE *Src1, const SKL_BYTE *Src2, SKL_INT32 BpS);
extern SKL_UINT32 Skl_SAD_8x8_MMX(const SKL_BYTE *Src1, const SKL_BYTE *Src2, SKL_INT32 BpS);
extern SKL_UINT32 Skl_SAD_16x8_Field_MMX(const SKL_BYTE *Src1, const SKL_BYTE *Src2, SKL_INT32 BpS);
extern SKL_UINT32 Skl_SAD_16x16_MMX(const SKL_BYTE *Src1, const SKL_BYTE *Src2, SKL_INT32 BpS);
extern SKL_UINT32 Skl_SSD_4x4_MMX(const SKL_BYTE *Src1, const SKL_BYTE *Src2, SKL_INT32 BpS);
extern SKL_UINT32 Skl_SSD_8x8_MMX(const SKL_BYTE *Src1, const SKL_BYTE *Src2, SKL_INT32 BpS);
extern SKL_UINT32 Skl_SSD_16x8_Field_MMX(const SKL_BYTE *Src1, const SKL_BYTE *Src2, SKL_INT32 BpS);
extern SKL_UINT32 Skl_SSD_16x16_MMX(const SKL_BYTE *Src1, const SKL_BYTE *Src2, SKL_INT32 BpS);
extern SKL_UINT32 Skl_Hadamard_SAD_4x4_MMX(const SKL_BYTE *Src1, const SKL_BYTE *Src2, SKL_INT32 BpS);
extern SKL_UINT32 Skl_Hadamard_SAD_8x8_MMX(const SKL_BYTE *Src1, const SKL_BYTE *Src2, SKL_INT32 BpS);
extern SKL_UINT32 Skl_Hadamard_SAD_16x8_Field_MMX(const SKL_BYTE *Src1, const SKL_BYTE *Src2, SKL_INT32 BpS);
extern SKL_UINT32 Skl_Hadamard_SAD_16x16_MMX(const SKL_BYTE *Src1, const SKL_BYTE *Src2, SKL_INT32 BpS);
extern SKL_UINT32 Skl_SAD_16x7_Self_MMX(const SKL_BYTE *Src1, SKL_INT32 BpS);
extern SKL_UINT32 Skl_Mean_4x4_MMX(const SKL_BYTE *Src1, SKL_INT32 BpS);
extern SKL_UINT32 Skl_Mean_8x8_MMX(const SKL_BYTE *Src1, SKL_INT32 BpS);
extern SKL_UINT32 Skl_Mean_16x16_MMX(const SKL_BYTE *Src1, SKL_INT32 BpS);
extern SKL_UINT32 Skl_Sqr_4x4_MMX(const SKL_BYTE *Src1, SKL_INT32 BpS);
extern SKL_UINT32 Skl_Sqr_8x8_MMX(const SKL_BYTE *Src1, SKL_INT32 BpS);
extern SKL_UINT32 Skl_Sqr_16x16_MMX(const SKL_BYTE *Src1, SKL_INT32 BpS);

extern void Skl_Smooth_18x18_To_8x8_MMX(SKL_BYTE *Dst, int Dst_BpS, const SKL_BYTE *Src, int Src_BpS);
extern void Skl_Gradx_18x18_To_8x8_MMX(SKL_INT8 *Dst, int Dst_BpS, const SKL_BYTE *Src, int Src_BpS);
extern void Skl_Grady_18x18_To_8x8_MMX(SKL_INT8 *Dst, int Dst_BpS, const SKL_BYTE *Src, int Src_BpS);
extern void Skl_Grad2_18x18_To_8x8_MMX(SKL_BYTE *Dst, int Dst_BpS, const SKL_BYTE *Src, int Src_BpS);

static SKL_IMG_DSP Img_DSP_MMX =  {
  "IMG-MMX", Skl_Switch_MMX,

  Skl_SAD_4x4_MMX,  Skl_SAD_8x8_MMX,  Skl_SAD_16x8_Field_MMX, Skl_SAD_16x16_MMX, Skl_SAD_16x7_Self_MMX,
  Skl_SSD_4x4_MMX,  Skl_SSD_8x8_MMX,  Skl_SSD_16x8_Field_MMX, Skl_SSD_16x16_MMX,
  Skl_Hadamard_SAD_4x4_MMX, Skl_Hadamard_SAD_8x8_MMX, Skl_Hadamard_SAD_16x8_Field_MMX, Skl_Hadamard_SAD_16x16_MMX,

  Skl_Mean_4x4_MMX, Skl_Mean_8x8_MMX, Skl_Mean_16x16_MMX,
  Skl_Sqr_4x4_MMX,  Skl_Sqr_8x8_MMX,  Skl_Sqr_16x16_MMX,

  Skl_Sqr_Dev_16x16_C,
  Skl_Abs_Dev_16x16_C,
  Skl_Hadamard_Dev_16x16_C,

  Skl_SAD_C, Skl_Mean_C, 
  Skl_Square_Dev_C,
  Skl_Abs_Dev_C,

  Skl_SAD_Avrg_4x4_C,  Skl_SAD_Avrg_8x8_C,
  Skl_SAD_Avrg_16x8_C, Skl_SAD_Avrg_16x16_C,
    
  Skl_Smooth_18x18_To_8x8_MMX,
  Skl_Gradx_18x18_To_8x8_MMX, Skl_Grady_18x18_To_8x8_MMX, Skl_Grad2_18x18_To_8x8_MMX
};

#else 
#define Img_DSP_MMX Img_DSP_C
#endif

//////////////////////////////////////////////////////////
// SSE version

#ifdef SKL_USE_ASM

extern SKL_UINT32 Skl_SAD_4x4_SSE(const SKL_BYTE *Src1, const SKL_BYTE *Src2, SKL_INT32 BpS);
extern SKL_UINT32 Skl_SAD_8x8_SSE(const SKL_BYTE *Src1, const SKL_BYTE *Src2, SKL_INT32 BpS);
extern SKL_UINT32 Skl_SAD_16x8_Field_SSE(const SKL_BYTE *Src1, const SKL_BYTE *Src2, SKL_INT32 BpS);
extern SKL_UINT32 Skl_SAD_16x16_SSE(const SKL_BYTE *Src1, const SKL_BYTE *Src2, SKL_INT32 BpS);
extern SKL_UINT32 Skl_SAD_16x7_Self_SSE(const SKL_BYTE *Src1, SKL_INT32 BpS);

extern SKL_UINT32 Skl_SAD_Avrg_8x8_SSE(const SKL_BYTE *Dst, const SKL_BYTE *Src1, const SKL_BYTE *Src2, SKL_INT32 BpS);
extern SKL_UINT32 Skl_SAD_Avrg_16x8_SSE(const SKL_BYTE *Dst, const SKL_BYTE *Src1, const SKL_BYTE *Src2, SKL_INT32 BpS);
extern SKL_UINT32 Skl_SAD_Avrg_16x16_SSE(const SKL_BYTE *Dst, const SKL_BYTE *Src1, const SKL_BYTE *Src2, SKL_INT32 BpS);

extern SKL_UINT32 Skl_Mean_8x8_SSE(const SKL_BYTE *Src1, SKL_INT32 BpS);
extern SKL_UINT32 Skl_Mean_4x4_SSE(const SKL_BYTE *Src1, SKL_INT32 BpS);
extern SKL_UINT32 Skl_Mean_16x16_SSE(const SKL_BYTE *Src1, SKL_INT32 BpS);

extern SKL_UINT32 Skl_Sqr_Dev_16x16_SSE(const SKL_BYTE *Src1, SKL_INT32 BpS);
extern SKL_UINT32 Skl_Abs_Dev_16x16_SSE(const SKL_BYTE *Src1, SKL_INT32 BpS);

static SKL_IMG_DSP Img_DSP_SSE = { 
  "IMG-SSE", Skl_Switch_MMX,

  Skl_SAD_4x4_SSE,  Skl_SAD_8x8_SSE,  Skl_SAD_16x8_Field_SSE, Skl_SAD_16x16_SSE, Skl_SAD_16x7_Self_SSE,
  Skl_SSD_4x4_MMX,  Skl_SSD_8x8_MMX,  Skl_SSD_16x8_Field_MMX, Skl_SSD_16x16_MMX, 
  Skl_Hadamard_SAD_4x4_MMX, Skl_Hadamard_SAD_8x8_MMX, Skl_Hadamard_SAD_16x8_Field_MMX, Skl_Hadamard_SAD_16x16_MMX,

  Skl_Mean_4x4_SSE, Skl_Mean_8x8_SSE, Skl_Mean_16x16_SSE,
  Skl_Sqr_4x4_MMX,  Skl_Sqr_8x8_MMX,  Skl_Sqr_16x16_MMX,

  Skl_Sqr_Dev_16x16_SSE,
  Skl_Abs_Dev_16x16_SSE,
  Skl_Hadamard_Dev_16x16_C,

  Skl_SAD_C, Skl_Mean_C, 
  Skl_Square_Dev_C,
  Skl_Abs_Dev_C,

  Skl_SAD_Avrg_4x4_C,    Skl_SAD_Avrg_8x8_SSE,
  Skl_SAD_Avrg_16x8_SSE, Skl_SAD_Avrg_16x16_SSE,

  Skl_Smooth_18x18_To_8x8_MMX,
  Skl_Gradx_18x18_To_8x8_MMX, Skl_Grady_18x18_To_8x8_MMX, Skl_Grad2_18x18_To_8x8_MMX
};

extern SKL_UINT32 Skl_Abs_Dev_16x16_SSE2(const SKL_BYTE *Src1, SKL_INT32 BpS);
extern SKL_UINT32 Skl_SAD_16x16_SSE2(const SKL_BYTE *Src1, const SKL_BYTE *Src2, SKL_INT32 BpS);
extern SKL_UINT32 Skl_SAD_16x8_Field_SSE2(const SKL_BYTE *Src1, const SKL_BYTE *Src2, SKL_INT32 BpS);
extern SKL_UINT32 Skl_SAD_16x7_Self_SSE2(const SKL_BYTE *Src1, SKL_INT32 BpS);
extern SKL_UINT32 Skl_Mean_16x16_SSE2(const SKL_BYTE *Src1, SKL_INT32 BpS);
extern SKL_UINT32 Skl_Sqr_16x16_SSE2(const SKL_BYTE *Src1, SKL_INT32 BpS);

static SKL_IMG_DSP Img_DSP_SSE2 = { 
  "IMG-SSE2", Skl_Switch_MMX,

  Skl_SAD_4x4_SSE,  Skl_SAD_8x8_SSE,  Skl_SAD_16x8_Field_SSE2, Skl_SAD_16x16_SSE2, Skl_SAD_16x7_Self_SSE2,
  Skl_SSD_4x4_MMX,  Skl_SSD_8x8_MMX,  Skl_SSD_16x8_Field_MMX, Skl_SSD_16x16_MMX, 
  Skl_Hadamard_SAD_4x4_MMX, Skl_Hadamard_SAD_8x8_MMX, Skl_Hadamard_SAD_16x8_Field_MMX, Skl_Hadamard_SAD_16x16_MMX,

  Skl_Mean_4x4_SSE, Skl_Mean_8x8_SSE, Skl_Mean_16x16_SSE2,
  Skl_Sqr_4x4_MMX,  Skl_Sqr_8x8_MMX,  Skl_Sqr_16x16_SSE2,

  Skl_Sqr_Dev_16x16_SSE,
  Skl_Abs_Dev_16x16_SSE2,
  Skl_Hadamard_Dev_16x16_C,

  Skl_SAD_C, Skl_Mean_C, 
  Skl_Square_Dev_C,
  Skl_Abs_Dev_C,

  Skl_SAD_Avrg_4x4_C,    Skl_SAD_Avrg_8x8_SSE,
  Skl_SAD_Avrg_16x8_SSE, Skl_SAD_Avrg_16x16_SSE,

  Skl_Smooth_18x18_To_8x8_MMX,
  Skl_Gradx_18x18_To_8x8_MMX, Skl_Grady_18x18_To_8x8_MMX, Skl_Grad2_18x18_To_8x8_MMX
};

#else 
#define Img_DSP_SSE Img_DSP_C
#define Img_DSP_SSE2 Img_DSP_C
#endif

//////////////////////////////////////////////////////////
// Reference version == C-version

#define Img_DSP_Ref Img_DSP_C

//////////////////////////////////////////////////////////
// no Alternative version

//////////////////////////////////////////////////////////
// Initialization
//////////////////////////////////////////////////////////

int Skl_Init_Img_DSP(SKL_IMG_DSP *Dsp, SKL_CPU_FEATURE Cpu)
{
  SKL_ASSERT(Dsp!=0);
  if (Cpu==SKL_CPU_DETECT) Cpu = Skl_Detect_CPU_Feature();

  if      (Cpu==SKL_CPU_C)    *Dsp = Img_DSP_C;
  else if (Cpu==SKL_CPU_X86)  *Dsp = Img_DSP_x86;
  else if (Cpu==SKL_CPU_MMX)  *Dsp = Img_DSP_MMX;
  else if (Cpu==SKL_CPU_SSE)  *Dsp = Img_DSP_SSE;
  else if (Cpu==SKL_CPU_SSE2) *Dsp = Img_DSP_SSE2;
  else if (Cpu==SKL_CPU_REF)  *Dsp = Img_DSP_Ref;
  else                        *Dsp = Img_DSP_C;

  if (Dsp->Switch_Off==0) Dsp->Switch_Off = Skl_Get_Switch(Cpu);
  return 1;
}

//////////////////////////////////////////////////////////

} /* extern "C" */
