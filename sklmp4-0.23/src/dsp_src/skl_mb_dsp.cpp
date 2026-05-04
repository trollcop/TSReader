/********************************************************
 * Some code. Copyright (C) 2003 by Pascal Massimino.   *
 * All Rights Reserved.      (http://skal.planet-d.net) *
 * For Educational/Academic use ONLY. See 'LICENSE.TXT'.*
 ********************************************************/
/*
 * skl_mb_dsp.cpp
 *
 *   Low-level processing for data in various format
 *   => Macro-block processing 
 ********************************************************/

#include "skl.h"
#include "skl_syst/skl_dsp.h"

extern "C" {

SKL_BYTE Skl_DSP_Clip[SKL_CLIP_MAX-SKL_CLIP_MIN] = {0};
void Skl_Init_DSP_Clip() {
  if (Skl_DSP_Clip[SKL_CLIP_MAX-SKL_CLIP_MIN-1]==0) {
    int i = SKL_CLIP_MIN;
    for( ; i<0; ++i)            Skl_DSP_Clip[i-SKL_CLIP_MIN] = 0;
    for( ; i<256; ++i)          Skl_DSP_Clip[i-SKL_CLIP_MIN] = i;
    for( ; i<SKL_CLIP_MAX; ++i) Skl_DSP_Clip[i-SKL_CLIP_MIN] = 255;
  }
}

//////////////////////////////////////////////////////////

// common signatures
#define BLK_FUNC(Name) void Name(SKL_BYTE *Dst, const SKL_INT16 *Src, const int BpS)
#define BLK2_FUNC(Name) void Name(SKL_BYTE *Dst, const SKL_BYTE *Src, const int BpS)
#define BLK3_FUNC(Name) void Name(SKL_INT16 *Dst, const SKL_BYTE *Src, const int BpS)
#define BLK4_FUNC(Name) void Name(SKL_INT16 *Dst, const SKL_BYTE *Src1, const SKL_BYTE *Src2, const int BpS)

//////////////////////////////////////////////////////////
// Raw-C version

extern BLK_FUNC(Skl_Copy_Upsampled_8x8_16To8_C);
extern BLK_FUNC(Skl_Add_Upsampled_8x8_16To8_C);

extern BLK3_FUNC(Skl_Copy_16x8_8To16_C);
extern BLK3_FUNC(Skl_Copy_8x8_8To16_C);
extern BLK3_FUNC(Skl_Diff_16x8_8To16_C);
extern BLK3_FUNC(Skl_Diff_8x8_8To16_C);
extern BLK4_FUNC(Skl_Diff_16x8_88To16_C);
extern BLK4_FUNC(Skl_Diff_8x8_88To16_C);

extern SKL_UINT32 Skl_SAD_16x7_Frame_C(const SKL_INT16 *Src);
extern SKL_UINT32 Skl_SAD_16x7_Field_C(const SKL_INT16 *Src);
extern void Skl_Reorder_Frame_16x16_C(SKL_INT16 *Src);

extern SKL_MB_FUNCS Skl_MB_Funcs_Add_Rnd0_C;
extern SKL_MB_FUNCS Skl_MB_Funcs_Copy_Rnd1_C,Skl_MB_Funcs_Copy_Rnd0_C;

extern void Skl_Make_Edges_C(SKL_BYTE * const * const YUV, const int Width, const int Height, const int BpS);
extern void Skl_HFilter_31_C(SKL_BYTE *Src1, SKL_BYTE *Src2, int Len);
extern void Skl_VFilter_31_C(SKL_BYTE *Src1, SKL_BYTE *Src2, const int BpS, int Len);

extern void Skl_Filter_18x18_To_8x8_C(SKL_INT16 *Dst, const SKL_BYTE *Src, const int BpS);
extern void Skl_Filter_Diff_18x18_To_8x8_C(SKL_INT16 *Dst, const SKL_BYTE *Src, const int BpS);

static void Init_C() {
  Skl_Init_DSP_Clip();
}

static SKL_MB_DSP Mb_DSP_C = { 
  "MB--C-", Skl_Switch_None, Init_C,

  Skl_Copy_Upsampled_8x8_16To8_C,
  Skl_Add_Upsampled_8x8_16To8_C,
  

  Skl_Copy_16x8_8To16_C,  Skl_Copy_8x8_8To16_C,
  Skl_Diff_16x8_8To16_C,  Skl_Diff_8x8_8To16_C,
  Skl_Diff_16x8_88To16_C, Skl_Diff_8x8_88To16_C,

  { &Skl_MB_Funcs_Copy_Rnd0_C, &Skl_MB_Funcs_Copy_Rnd1_C },
    &Skl_MB_Funcs_Add_Rnd0_C,

  Skl_SAD_16x7_Frame_C, Skl_SAD_16x7_Field_C, Skl_Reorder_Frame_16x16_C,

  Skl_Make_Edges_C,
  Skl_HFilter_31_C, Skl_VFilter_31_C,
  Skl_Filter_18x18_To_8x8_C, Skl_Filter_Diff_18x18_To_8x8_C,

  &Skl_Filter_2_C
};

//////////////////////////////////////////////////////////
// Ref version (almost the same than C, except for qpel)

extern SKL_MB_FUNCS Skl_MB_Funcs_Add_Rnd0_Ref;
extern SKL_MB_FUNCS Skl_MB_Funcs_Copy_Rnd1_Ref,Skl_MB_Funcs_Copy_Rnd0_Ref;

static SKL_MB_DSP Mb_DSP_Ref = { 
  "MB-Ref", Skl_Switch_None,

  0,

  Skl_Copy_Upsampled_8x8_16To8_C, Skl_Add_Upsampled_8x8_16To8_C,

  Skl_Copy_16x8_8To16_C,  Skl_Copy_8x8_8To16_C,
  Skl_Diff_16x8_8To16_C,  Skl_Diff_8x8_8To16_C,
  Skl_Diff_16x8_88To16_C, Skl_Diff_8x8_88To16_C,

  { &Skl_MB_Funcs_Copy_Rnd0_Ref, &Skl_MB_Funcs_Copy_Rnd1_Ref },
    &Skl_MB_Funcs_Add_Rnd0_Ref,

  Skl_SAD_16x7_Frame_C, Skl_SAD_16x7_Field_C, Skl_Reorder_Frame_16x16_C,

  Skl_Make_Edges_C,
  Skl_HFilter_31_C, Skl_VFilter_31_C,
  Skl_Filter_18x18_To_8x8_C, Skl_Filter_Diff_18x18_To_8x8_C,

  &Skl_Filter_2_C
};

//////////////////////////////////////////////////////////
// x86-ASM version

#ifdef SKL_USE_ASM

extern SKL_MB_FUNCS Skl_MB_Funcs_Add_Rnd0_x86;
extern SKL_MB_FUNCS Skl_MB_Funcs_Copy_Rnd1_x86,Skl_MB_Funcs_Copy_Rnd0_x86;

extern void Skl_Make_Edges_x86(SKL_BYTE * const * const YUV, const int Width, const int Height, const int BpS);
extern void Skl_HFilter_31_x86(SKL_BYTE *Src1, SKL_BYTE *Src2, int Len);
extern void Skl_VFilter_31_x86(SKL_BYTE *Src1, SKL_BYTE *Src2, const int BpS, int Len);

static SKL_MB_DSP Mb_DSP_x86 = { 
  "MB-x86", Skl_Switch_None,
  0,

  Skl_Copy_Upsampled_8x8_16To8_C, Skl_Add_Upsampled_8x8_16To8_C,

  Skl_Copy_16x8_8To16_C,  Skl_Copy_8x8_8To16_C,
  Skl_Diff_16x8_8To16_C,  Skl_Diff_8x8_8To16_C,
  Skl_Diff_16x8_88To16_C, Skl_Diff_8x8_88To16_C,

  { &Skl_MB_Funcs_Copy_Rnd0_x86, &Skl_MB_Funcs_Copy_Rnd1_x86 },
    &Skl_MB_Funcs_Add_Rnd0_x86,

  Skl_SAD_16x7_Frame_C, Skl_SAD_16x7_Field_C, Skl_Reorder_Frame_16x16_C,

  Skl_Make_Edges_x86,
  Skl_HFilter_31_x86, Skl_VFilter_31_x86,
  Skl_Filter_18x18_To_8x8_C, Skl_Filter_Diff_18x18_To_8x8_C,

  &Skl_Filter_2_C
};

#else 
#define Mb_DSP_x86 Mb_DSP_C
#endif

//////////////////////////////////////////////////////////
// MMX version

#ifdef SKL_USE_ASM

extern SKL_MB_FUNCS Skl_MB_Funcs_Add_Rnd0_MMX;
extern SKL_MB_FUNCS Skl_MB_Funcs_Copy_Rnd1_MMX,Skl_MB_Funcs_Copy_Rnd0_MMX;

extern BLK3_FUNC(Skl_Copy_16x8_8To16_MMX);
extern BLK3_FUNC(Skl_Copy_8x8_8To16_MMX);
extern BLK3_FUNC(Skl_Diff_16x8_8To16_MMX);
extern BLK3_FUNC(Skl_Diff_8x8_8To16_MMX);
extern BLK4_FUNC(Skl_Diff_16x8_88To16_MMX);
extern BLK4_FUNC(Skl_Diff_8x8_88To16_MMX);

extern BLK_FUNC(Skl_Copy_Upsampled_8x8_16To8_MMX);
extern BLK_FUNC(Skl_Add_Upsampled_8x8_16To8_MMX);

extern void Skl_Make_Edges_MMX(SKL_BYTE * const * const YUV, const int Width, const int Height, const int BpS);
extern void Skl_HFilter_31_MMX(SKL_BYTE *Src1, SKL_BYTE *Src2, int Len);

extern void Skl_Filter_18x18_To_8x8_MMX(SKL_INT16 *Dst, const SKL_BYTE *Src, const int BpS);
extern void Skl_Filter_Diff_18x18_To_8x8_MMX(SKL_INT16 *Dst, const SKL_BYTE *Src, const int BpS);

extern void Skl_Init_QP_MMX();

static SKL_MB_DSP Mb_DSP_MMX =  {
  "MB-MMX", Skl_Switch_MMX,
  Skl_Init_QP_MMX,

  Skl_Copy_Upsampled_8x8_16To8_MMX, Skl_Add_Upsampled_8x8_16To8_MMX,

  Skl_Copy_16x8_8To16_MMX,  Skl_Copy_8x8_8To16_MMX,
  Skl_Diff_16x8_8To16_MMX,  Skl_Diff_8x8_8To16_MMX,
  Skl_Diff_16x8_88To16_MMX, Skl_Diff_8x8_88To16_MMX,

  { &Skl_MB_Funcs_Copy_Rnd0_MMX, &Skl_MB_Funcs_Copy_Rnd1_MMX },
    &Skl_MB_Funcs_Add_Rnd0_MMX,

  Skl_SAD_16x7_Frame_C, Skl_SAD_16x7_Field_C, Skl_Reorder_Frame_16x16_C,

  Skl_Make_Edges_MMX,
  Skl_HFilter_31_MMX, Skl_VFilter_31_x86,
  Skl_Filter_18x18_To_8x8_MMX, Skl_Filter_Diff_18x18_To_8x8_MMX,

  &Skl_Filter_2_MMX
};

#else 
#define Mb_DSP_MMX Mb_DSP_C
#endif

//////////////////////////////////////////////////////////
// SSE version

#ifdef SKL_USE_ASM

extern SKL_MB_FUNCS Skl_MB_Funcs_Add_Rnd0_SSE;
extern SKL_MB_FUNCS Skl_MB_Funcs_Copy_Rnd1_SSE,Skl_MB_Funcs_Copy_Rnd0_SSE;

extern BLK_FUNC(Skl_Copy_Upsampled_8x8_16To8_SSE);
extern BLK_FUNC(Skl_Add_Upsampled_8x8_16To8_SSE);

extern void Skl_Make_Edges_SSE(SKL_BYTE * const * const YUV, const int Width, const int Height, const int BpS);

static SKL_MB_DSP Mb_DSP_SSE = {
  "MB-SSE", Skl_Switch_MMX,
  Skl_Init_QP_MMX,

  Skl_Copy_Upsampled_8x8_16To8_SSE, Skl_Add_Upsampled_8x8_16To8_SSE,

  Skl_Copy_16x8_8To16_MMX,  Skl_Copy_8x8_8To16_MMX,
  Skl_Diff_16x8_8To16_MMX,  Skl_Diff_8x8_8To16_MMX,
  Skl_Diff_16x8_88To16_MMX, Skl_Diff_8x8_88To16_MMX,

  { &Skl_MB_Funcs_Copy_Rnd0_SSE, &Skl_MB_Funcs_Copy_Rnd1_SSE },
    &Skl_MB_Funcs_Add_Rnd0_SSE,

  Skl_SAD_16x7_Frame_C, Skl_SAD_16x7_Field_C, Skl_Reorder_Frame_16x16_C,

  Skl_Make_Edges_SSE,
  Skl_HFilter_31_MMX, Skl_VFilter_31_x86,
  Skl_Filter_18x18_To_8x8_MMX, Skl_Filter_Diff_18x18_To_8x8_MMX,

  &Skl_Filter_2_SSE
};

extern SKL_MB_FUNCS Skl_MB_Funcs_Add_Rnd0_SSE2;
extern SKL_MB_FUNCS Skl_MB_Funcs_Copy_Rnd1_SSE2,Skl_MB_Funcs_Copy_Rnd0_SSE2;

static SKL_MB_DSP Mb_DSP_SSE2 = {
  "MB-SSE2", Skl_Switch_MMX,
  Skl_Init_QP_MMX,

  Skl_Copy_Upsampled_8x8_16To8_SSE, Skl_Add_Upsampled_8x8_16To8_SSE,

  Skl_Copy_16x8_8To16_MMX,  Skl_Copy_8x8_8To16_MMX,
  Skl_Diff_16x8_8To16_MMX,  Skl_Diff_8x8_8To16_MMX,
  Skl_Diff_16x8_88To16_MMX, Skl_Diff_8x8_88To16_MMX,

  { &Skl_MB_Funcs_Copy_Rnd0_SSE2, &Skl_MB_Funcs_Copy_Rnd1_SSE2 },
    &Skl_MB_Funcs_Add_Rnd0_SSE2,

  Skl_SAD_16x7_Frame_C, Skl_SAD_16x7_Field_C, Skl_Reorder_Frame_16x16_C,

  Skl_Make_Edges_SSE,
  Skl_HFilter_31_MMX, Skl_VFilter_31_x86,
  Skl_Filter_18x18_To_8x8_MMX, Skl_Filter_Diff_18x18_To_8x8_MMX,
  
  &Skl_Filter_2_SSE
};

#else 
#define Mb_DSP_SSE  Mb_DSP_C
#define Mb_DSP_SSE2 Mb_DSP_C
#endif

//////////////////////////////////////////////////////////
// Initialization
//////////////////////////////////////////////////////////

int Skl_Init_Mb_DSP(SKL_MB_DSP *Dsp, SKL_CPU_FEATURE Cpu)
{
  SKL_ASSERT(Dsp!=0);
  if (Cpu==SKL_CPU_DETECT) Cpu = Skl_Detect_CPU_Feature();

  if      (Cpu==SKL_CPU_C)    *Dsp = Mb_DSP_C;
  else if (Cpu==SKL_CPU_X86)  *Dsp = Mb_DSP_x86;
  else if (Cpu==SKL_CPU_MMX)  *Dsp = Mb_DSP_MMX;
  else if (Cpu==SKL_CPU_SSE)  *Dsp = Mb_DSP_SSE;
  else if (Cpu==SKL_CPU_SSE2) *Dsp = Mb_DSP_SSE2;
  else if (Cpu==SKL_CPU_REF)  *Dsp = Mb_DSP_Ref;
  else                        *Dsp = Mb_DSP_C;

  if (Dsp->Switch_Off==0) Dsp->Switch_Off = Skl_Get_Switch(Cpu);
  if (Dsp->Init!=0) Dsp->Init();
  return 1;
}

//////////////////////////////////////////////////////////

} /* extern "C" */
