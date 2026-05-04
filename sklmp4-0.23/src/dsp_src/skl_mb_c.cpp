/********************************************************
 * Some code. Copyright (C) 2003 by Pascal Massimino.   *
 * All Rights Reserved.      (http://skal.planet-d.net) *
 * For Educational/Academic use ONLY. See 'LICENSE.TXT'.*
 ********************************************************/
/*
 *  skl_mb_c.cpp
 *
 *  Macro-block processing
 *
 ********************************************************/

//////////////////////////////////////////////////////////

#include "skl.h"
#include "skl_syst/skl_dsp.h"

extern "C" {

void Skl_Init_MB_Clip() {}

#define COPY(d,s)    (d) = SKL_DSP_CLIP((s))  // <0 ? 0 : (s)>255 ? 255 : (s)
#define ADD(d,s)     COPY( (d), (s)+(d) )

//////////////////////////////////////////////////////////
// 8b <-> 8b transfer
//////////////////////////////////////////////////////////

extern "C" SKL_MB_FUNCS Skl_MB_Funcs_Add_C;
extern "C" SKL_MB_FUNCS Skl_MB_Funcs_Copy_C;


#define PRELUDE(H,S)          \
  for(int j=(H); j>0; j--) {    \
    for(int i=-(S); i<0; ++i) {

#define EPILOG                \
    }                         \
    Dst += BpS; Src += BpS;   \
  }
#define MIX(x)  Dst[(x)] = (Dst[(x)]+v +1)>>1

#define MB_FUNC(Name) void Name(SKL_BYTE *Dst, const SKL_BYTE *Src, const int BpS)
#define MB_FUNC_QP(Name) void Name(SKL_BYTE *Dst, const SKL_BYTE *Src, \
                                   SKL_BYTE *Tmp_X, SKL_BYTE *Tmp_Y,  \
                                   const int BpS)

//////////////////////////////////////////////////////////
//
//  Half-pixel add
//
//////////////////////////////////////////////////////////

  // 16x8 Half-Pixel additive ops

static MB_FUNC(Skl_Add_16x8_FF_C) {
  PRELUDE(8,16)
    const SKL_UINT32 v = Src[16+i];
    MIX(16+i);
  EPILOG
}

  // 8x8 Half-Pixel additive ops

static MB_FUNC(Skl_Add_8x8_FF_C) { 
  PRELUDE(8,8)
    const SKL_UINT32 v = Src[8+i]; MIX(8+i); 
  EPILOG
}

  // 8x4 Half-Pixel additive ops

static MB_FUNC(Skl_Add_8x4_FF_C) { 
  PRELUDE(4,8) 
    const SKL_UINT32 v = Src[8+i]; MIX(8+i); 
  EPILOG
}

static MB_FUNC(Skl_Add_16x8_FH_Rnd0_C) {
  PRELUDE(8,16) 
    const SKL_UINT32 v = (Src[16+i] + Src[16+1+i] + 1)>>1;
    MIX(16+i);
  EPILOG
}
static MB_FUNC(Skl_Add_16x8_HF_Rnd0_C) {
  PRELUDE(8,16) 
    const SKL_UINT32 v = (Src[16+i] + Src[16+BpS+i] + 1)>>1;
    MIX(16+i);
  EPILOG
}
static MB_FUNC(Skl_Add_16x8_HH_Rnd0_C) {
  PRELUDE(8,16) 
    const SKL_UINT32 v = (  Src[16+i] + Src[16+1+i] 
                        + Src[16+BpS+i] + Src[16+1+BpS+i]+ 2) >> 2;
    MIX(16+i);
  EPILOG
}

static MB_FUNC(Skl_Add_8x8_FH_Rnd0_C) {
  PRELUDE(8,8) 
    const SKL_UINT32 v = (Src[8+i] + Src[8+1+i] + 1)>>1; MIX(8+i);
  EPILOG
}
static MB_FUNC(Skl_Add_8x8_HF_Rnd0_C) {
  PRELUDE(8,8) 
    const SKL_UINT32 v = (Src[8+i] + Src[8+BpS+i] + 1)>>1; MIX(8+i);
  EPILOG
}
static MB_FUNC(Skl_Add_8x8_HH_Rnd0_C) {
  PRELUDE(8,8)
    const SKL_UINT32 v = (Src[8+i] + Src[8+1+i] 
                        + Src[8+BpS+i] + Src[8+1+BpS+i]+ 2) >> 2;
    MIX(8+i);
  EPILOG
}

static MB_FUNC(Skl_Add_8x4_FH_Rnd0_C) {
  PRELUDE(4,8) 
    const SKL_UINT32 v = (Src[8+i] + Src[8+1+i] + 1)>>1; MIX(8+i);
  EPILOG
}
static MB_FUNC(Skl_Add_8x4_HF_Rnd0_C) {
  PRELUDE(4,8) 
    const SKL_UINT32 v = (Src[8+i] + Src[8+BpS+i] + 1)>>1; MIX(8+i);
  EPILOG
}
static MB_FUNC(Skl_Add_8x4_HH_Rnd0_C) {
  PRELUDE(4,8)
    const SKL_UINT32 v = (  Src[8+i] + Src[8+1+i] 
                            + Src[8+BpS+i] + Src[8+1+BpS+i]+ 2) >> 2;
    MIX(8+i);
  EPILOG
}


//////////////////////////////////////////////////////////
//
//  Half-pixel copy
//
//////////////////////////////////////////////////////////

  // 16x8 Half-Pixel copy ops

static MB_FUNC(Skl_Copy_16x8_FF_C) {
  for(int j=8; j>0; j--) {
    memcpy(Dst, Src, 16*sizeof(*Dst));
    Dst += BpS;
    Src += BpS;
  }
}
static MB_FUNC(Skl_Copy_16x8_FH_Rnd1_C) {
  PRELUDE(8,16)
      Dst[16+i] = (Src[16+i] + Src[16+1+i]) >>1;
  EPILOG
}
static MB_FUNC(Skl_Copy_16x8_HF_Rnd1_C) {
    PRELUDE(8,16)
      Dst[16+i] = (Src[16+i] + Src[16+BpS+i]) >>1;
    EPILOG
}
static MB_FUNC(Skl_Copy_16x8_HH_Rnd1_C) {
    PRELUDE(8,16) 
      Dst[16+i] = (Src[16+i] + Src[16+1+i] + 
                   Src[16+BpS+i] + Src[16+1+BpS+i] + 1) >> 2;
    EPILOG
}

  // 8x8 Half-Pixel copy ops

static MB_FUNC(Skl_Copy_8x8_FF_C) {
  for(int j=8; j>0; j--) {
    memcpy(Dst, Src, 8*sizeof(*Dst));
    Dst += BpS;
    Src += BpS;
  }
}
static MB_FUNC(Skl_Copy_8x8_FH_Rnd1_C) {
    PRELUDE(8,8)
      Dst[8+i] = (Src[8+i] + Src[8+1+i]) >>1;
    EPILOG
}

static MB_FUNC(Skl_Copy_8x8_HF_Rnd1_C) {
    PRELUDE(8,8)
      Dst[8+i] = (Src[8+i] + Src[8+BpS+i]) >>1;
    EPILOG
}
static MB_FUNC(Skl_Copy_8x8_HH_Rnd1_C) {
    PRELUDE(8,8)
      Dst[8+i] = ( Src[8+i] + Src[8+1+i] 
                  + Src[8+BpS+i] + Src[8+1+BpS+i]+ 1) >> 2;
    EPILOG
}

  // 8x4 Half-Pixel copy ops

static MB_FUNC(Skl_Copy_8x4_FF_C) {
  for(int j=4; j>0; j--) {
    memcpy(Dst, Src, 8*sizeof(*Dst));
    Dst += BpS;
    Src += BpS;
  }
}
static MB_FUNC(Skl_Copy_8x4_FH_Rnd1_C) {
    PRELUDE(4,8)
      Dst[8+i] = (Src[8+i] + Src[8+1+i]) >>1;
    EPILOG
}
static MB_FUNC(Skl_Copy_8x4_HF_Rnd1_C) {
    PRELUDE(4,8)
      Dst[8+i] = (Src[8+i] + Src[8+BpS+i]) >>1;
    EPILOG
}
static MB_FUNC(Skl_Copy_8x4_HH_Rnd1_C) {
    PRELUDE(4,8)
      Dst[8+i] = ( Src[8+i] + Src[8+1+i] 
                  + Src[8+BpS+i] + Src[8+1+BpS+i]+ 1) >> 2;
    EPILOG
}


    // the same, with rounding = 0


static MB_FUNC(Skl_Copy_16x8_FH_Rnd0_C) {
    PRELUDE(8,16)
      Dst[16+i] = (Src[16+i] + Src[16+1+i]+1) >>1;
    EPILOG
}
static MB_FUNC(Skl_Copy_16x8_HF_Rnd0_C) {
    PRELUDE(8,16) 
      Dst[16+i] = (Src[16+i] + Src[16+BpS+i]+1) >>1;
    EPILOG
}
static MB_FUNC(Skl_Copy_16x8_HH_Rnd0_C) {
    PRELUDE(8,16) 
      Dst[16+i] = (Src[16+i] + Src[16+1+i] + 
                   Src[16+BpS+i] + Src[16+1+BpS+i] + 2) >> 2;
    EPILOG
}

static MB_FUNC(Skl_Copy_8x8_FH_Rnd0_C) {
    PRELUDE(8,8)
      Dst[8+i] = (Src[8+i] + Src[8+1+i] + 1) >>1;
    EPILOG
}
static MB_FUNC(Skl_Copy_8x8_HF_Rnd0_C) {
    PRELUDE(8,8)
      Dst[8+i] = (Src[8+i] + Src[8+BpS+i] + 1) >>1;
    EPILOG
}
static MB_FUNC(Skl_Copy_8x8_HH_Rnd0_C) {
    PRELUDE(8,8)
      Dst[8+i] = ( Src[8+i] + Src[8+1+i] 
                  + Src[8+BpS+i] + Src[8+1+BpS+i]+ 2) >> 2;
    EPILOG
}

static MB_FUNC(Skl_Copy_8x4_FH_Rnd0_C) {
    PRELUDE(4,8)
      Dst[8+i] = (Src[8+i] + Src[8+1+i] + 1) >>1;
    EPILOG
}
static MB_FUNC(Skl_Copy_8x4_HF_Rnd0_C) {
    PRELUDE(4,8)
      Dst[8+i] = (Src[8+i] + Src[8+BpS+i] + 1) >>1;
    EPILOG
}
static MB_FUNC(Skl_Copy_8x4_HH_Rnd0_C) {
    PRELUDE(4,8)
      Dst[8+i] = ( Src[8+i] + Src[8+1+i] 
                  + Src[8+BpS+i] + Src[8+1+BpS+i]+ 2) >> 2;
    EPILOG
}

    // HV Filter for 16x16 block

static MB_FUNC(Skl_Copy_16x16_FH_Rnd0_C) {
    PRELUDE(16,16)
      Dst[16+i] = (Src[16+i] + Src[16+1+i]+1) >>1;
    EPILOG
}
static MB_FUNC(Skl_Copy_16x16_HF_Rnd0_C) {
    PRELUDE(16,16) 
      Dst[16+i] = (Src[16+i] + Src[16+BpS+i]+1) >>1;
    EPILOG
}
static MB_FUNC(Skl_Copy_16x16_HH_Rnd0_C) {
    PRELUDE(16,16) 
      Dst[16+i] = (Src[16+i] + Src[16+1+i] + 
                   Src[16+BpS+i] + Src[16+1+BpS+i] + 2) >> 2;
    EPILOG
}

const SKL_HV_FILTER Skl_Filter_2_C = { 
  Skl_Copy_16x16_FH_Rnd0_C, Skl_Copy_16x16_HF_Rnd0_C, Skl_Copy_16x16_HH_Rnd0_C
};

//////////////////////////////////////////////////////////
//
//  Half-pixel SAD
//
//////////////////////////////////////////////////////////

#define ABS(x) ((x)<0 ? -(x) : (x))

void Skl_SAD_HP_16x16_Rnd0_C(const SKL_BYTE *Cur, const SKL_BYTE *Src,
                             int BpS, SKL_UINT32 Sad[3])
{
  SKL_UINT32 Sad_H = 0, Sad_V = 0, Sad_HV = 0;
  for(int j=16; j>0; j--) {
    for(int i=-16; i<0; i++) {
      SKL_INT32 v1 = Src[16+i]     + Src[16+i+1]     + 1;
      SKL_INT32 v2 = Src[16+i+BpS] + Src[16+i+1+BpS] + 1 + v1;
      v1 = (v1>>1) - Cur[16+i];
      v2 = (v2>>2) - Cur[16+i];
      Sad_H  += ABS(v1);
      Sad_HV += ABS(v2);
      SKL_INT32 v3 = Src[16+i+BpS] + Src[16+i]     + 1;
      v3 = (v3>>1) - Cur[16+i];
      Sad_V  += ABS(v3);
    }
    Cur += BpS;
    Src += BpS;
  }
  Sad[0] = Sad_H;
  Sad[1] = Sad_V;
  Sad[2] = Sad_HV;
}

void Skl_SAD_HP_16x16_Rnd1_C(const SKL_BYTE *Cur, const SKL_BYTE *Src,
                             int BpS, SKL_UINT32 Sad[3])
{
  SKL_UINT32 Sad_H = 0, Sad_V = 0, Sad_HV = 0;
  for(int j=16; j>0; j--) {
    for(int i=-16; i<0; i++) {
      SKL_INT32 v1 = Src[16+i]     + Src[16+i+1];
      SKL_INT32 v2 = Src[16+i+BpS] + Src[16+i+1+BpS] + 1 + v1;
      v1 = (v1>>1) - Cur[16+i];
      v2 = (v2>>2) - Cur[16+i]; 
      Sad_H  += ABS(v1);
      Sad_HV += ABS(v2);
      SKL_INT32 v3 = Src[16+i+BpS] + Src[16+i];
      v3 = (v3>>1) - Cur[16+i];
      Sad_V  += ABS(v3);
    }
    Cur += BpS;
    Src += BpS;
  }
  Sad[0] = Sad_H;
  Sad[1] = Sad_V;
  Sad[2] = Sad_HV;
}

void Skl_SAD_HP_8x8_Rnd0_C(const SKL_BYTE *Cur, const SKL_BYTE *Src,
                           int BpS, SKL_UINT32 Sad[3])
{
  SKL_UINT32 Sad_H = 0, Sad_V = 0, Sad_HV = 0;
  for(int j=8; j>0; j--) {
    for(int i=-8; i<0; i++) {
      SKL_INT32 v1 = Src[8+i]     + Src[8+i+1]     + 1;
      SKL_INT32 v2 = Src[8+i+BpS] + Src[8+i+1+BpS] + 1 + v1;
      v1 = (v1>>1) - Cur[8+i];
      v2 = (v2>>2) - Cur[8+i];
      Sad_H  += ABS(v1);
      Sad_HV += ABS(v2);
      SKL_INT32 v3 = Src[8+i+BpS] + Src[8+i]       + 1;
      v3 = (v3>>1) - Cur[8+i];
      Sad_V  += ABS(v3);
    }
    Cur += BpS;
    Src += BpS;
  }
  Sad[0] = Sad_H;
  Sad[1] = Sad_V;
  Sad[2] = Sad_HV;
}

void Skl_SAD_HP_8x8_Rnd1_C(const SKL_BYTE *Cur, const SKL_BYTE *Src,
                           int BpS, SKL_UINT32 Sad[3])
{
  SKL_UINT32 Sad_H = 0, Sad_V = 0, Sad_HV = 0;
  for(int j=8; j>0; j--) {
    for(int i=-8; i<0; i++) {
      SKL_INT32 v1 = Src[8+i]     + Src[8+i+1];
      SKL_INT32 v2 = Src[8+i+BpS] + Src[8+i+1+BpS] + 1 + v1;
      v1 = (v1>>1) - Cur[8+i];
      v2 = (v2>>2) - Cur[8+i];
      Sad_H  += ABS(v1);
      Sad_HV += ABS(v2);
      SKL_INT32 v3 = Src[8+i+BpS] + Src[8+i];
      v3 = (v3>>1) - Cur[8+i];
      Sad_V  += ABS(v3);
    }
    Cur += BpS;
    Src += BpS;
  }
  Sad[0] = Sad_H;
  Sad[1] = Sad_V;
  Sad[2] = Sad_HV;
}

#undef SAD

//////////////////////////////////////////////////////////
// C-version

extern SKL_MB_QP_PASS_SIGNATURE(Skl_H_Pass_16_Copy_Rnd1_C);
extern SKL_MB_QP_PASS_SIGNATURE(Skl_V_Pass_16_Copy_Rnd1_C);
extern SKL_MB_QP_PASS_SIGNATURE(Skl_H_Pass_Avrg_16_Copy_Rnd1_C);
extern SKL_MB_QP_PASS_SIGNATURE(Skl_V_Pass_Avrg_16_Copy_Rnd1_C);
extern SKL_MB_QP_PASS_SIGNATURE(Skl_H_Pass_Avrg_Up_16_Copy_Rnd1_C);
extern SKL_MB_QP_PASS_SIGNATURE(Skl_V_Pass_Avrg_Up_16_Copy_Rnd1_C);
extern SKL_MB_QP_PASS_SIGNATURE(Skl_H_Pass_8_Copy_Rnd1_C);
extern SKL_MB_QP_PASS_SIGNATURE(Skl_V_Pass_8_Copy_Rnd1_C);
extern SKL_MB_QP_PASS_SIGNATURE(Skl_H_Pass_Avrg_8_Copy_Rnd1_C);
extern SKL_MB_QP_PASS_SIGNATURE(Skl_V_Pass_Avrg_8_Copy_Rnd1_C);
extern SKL_MB_QP_PASS_SIGNATURE(Skl_H_Pass_Avrg_Up_8_Copy_Rnd1_C);
extern SKL_MB_QP_PASS_SIGNATURE(Skl_V_Pass_Avrg_Up_8_Copy_Rnd1_C);

extern SKL_MB_QP_PASS_SIGNATURE(Skl_H_Pass_16_Copy_Rnd0_C);
extern SKL_MB_QP_PASS_SIGNATURE(Skl_V_Pass_16_Copy_Rnd0_C);
extern SKL_MB_QP_PASS_SIGNATURE(Skl_H_Pass_Avrg_16_Copy_Rnd0_C);
extern SKL_MB_QP_PASS_SIGNATURE(Skl_V_Pass_Avrg_16_Copy_Rnd0_C);
extern SKL_MB_QP_PASS_SIGNATURE(Skl_H_Pass_Avrg_Up_16_Copy_Rnd0_C);
extern SKL_MB_QP_PASS_SIGNATURE(Skl_V_Pass_Avrg_Up_16_Copy_Rnd0_C);
extern SKL_MB_QP_PASS_SIGNATURE(Skl_H_Pass_8_Copy_Rnd0_C);
extern SKL_MB_QP_PASS_SIGNATURE(Skl_V_Pass_8_Copy_Rnd0_C);
extern SKL_MB_QP_PASS_SIGNATURE(Skl_H_Pass_Avrg_8_Copy_Rnd0_C);
extern SKL_MB_QP_PASS_SIGNATURE(Skl_V_Pass_Avrg_8_Copy_Rnd0_C);
extern SKL_MB_QP_PASS_SIGNATURE(Skl_H_Pass_Avrg_Up_8_Copy_Rnd0_C);
extern SKL_MB_QP_PASS_SIGNATURE(Skl_V_Pass_Avrg_Up_8_Copy_Rnd0_C);

extern SKL_MB_QP_PASS_SIGNATURE(Skl_H_Pass_16_Add_Rnd0_C);
extern SKL_MB_QP_PASS_SIGNATURE(Skl_V_Pass_16_Add_Rnd0_C);
extern SKL_MB_QP_PASS_SIGNATURE(Skl_H_Pass_Avrg_16_Add_Rnd0_C);
extern SKL_MB_QP_PASS_SIGNATURE(Skl_V_Pass_Avrg_16_Add_Rnd0_C);
extern SKL_MB_QP_PASS_SIGNATURE(Skl_H_Pass_Avrg_Up_16_Add_Rnd0_C);
extern SKL_MB_QP_PASS_SIGNATURE(Skl_V_Pass_Avrg_Up_16_Add_Rnd0_C);
extern SKL_MB_QP_PASS_SIGNATURE(Skl_H_Pass_8_Add_Rnd0_C);
extern SKL_MB_QP_PASS_SIGNATURE(Skl_V_Pass_8_Add_Rnd0_C);
extern SKL_MB_QP_PASS_SIGNATURE(Skl_H_Pass_Avrg_8_Add_Rnd0_C);
extern SKL_MB_QP_PASS_SIGNATURE(Skl_V_Pass_Avrg_8_Add_Rnd0_C);
extern SKL_MB_QP_PASS_SIGNATURE(Skl_H_Pass_Avrg_Up_8_Add_Rnd0_C);
extern SKL_MB_QP_PASS_SIGNATURE(Skl_V_Pass_Avrg_Up_8_Add_Rnd0_C);

SKL_MB_FUNCS Skl_MB_Funcs_Copy_Rnd1_C = {
  { Skl_Copy_16x8_FF_C, Skl_Copy_16x8_FH_Rnd1_C, Skl_Copy_16x8_HF_Rnd1_C, Skl_Copy_16x8_HH_Rnd1_C }
, { Skl_Copy_8x8_FF_C,  Skl_Copy_8x8_FH_Rnd1_C,  Skl_Copy_8x8_HF_Rnd1_C,  Skl_Copy_8x8_HH_Rnd1_C  }
, { Skl_Copy_8x4_FF_C,  Skl_Copy_8x4_FH_Rnd1_C,  Skl_Copy_8x4_HF_Rnd1_C,  Skl_Copy_8x4_HH_Rnd1_C  }
,   Skl_H_Pass_16_Copy_Rnd1_C, Skl_H_Pass_Avrg_16_Copy_Rnd1_C, Skl_H_Pass_Avrg_Up_16_Copy_Rnd1_C
,   Skl_H_Pass_16_Copy_Rnd1_C, Skl_H_Pass_Avrg_16_Copy_Rnd1_C, Skl_H_Pass_Avrg_Up_16_Copy_Rnd1_C
,   Skl_V_Pass_16_Copy_Rnd1_C, Skl_V_Pass_Avrg_16_Copy_Rnd1_C, Skl_V_Pass_Avrg_Up_16_Copy_Rnd1_C
,   Skl_H_Pass_8_Copy_Rnd1_C,  Skl_H_Pass_Avrg_8_Copy_Rnd1_C,  Skl_H_Pass_Avrg_Up_8_Copy_Rnd1_C
,   Skl_H_Pass_8_Copy_Rnd1_C,  Skl_H_Pass_Avrg_8_Copy_Rnd1_C,  Skl_H_Pass_Avrg_Up_8_Copy_Rnd1_C
,   Skl_V_Pass_8_Copy_Rnd1_C,  Skl_V_Pass_Avrg_8_Copy_Rnd1_C,  Skl_V_Pass_Avrg_Up_8_Copy_Rnd1_C
,   Skl_SAD_HP_16x16_Rnd1_C, Skl_SAD_HP_8x8_Rnd1_C
};

SKL_MB_FUNCS Skl_MB_Funcs_Copy_Rnd0_C = {
  { Skl_Copy_16x8_FF_C, Skl_Copy_16x8_FH_Rnd0_C, Skl_Copy_16x8_HF_Rnd0_C, Skl_Copy_16x8_HH_Rnd0_C }
, { Skl_Copy_8x8_FF_C,  Skl_Copy_8x8_FH_Rnd0_C,  Skl_Copy_8x8_HF_Rnd0_C,  Skl_Copy_8x8_HH_Rnd0_C  }
, { Skl_Copy_8x4_FF_C,  Skl_Copy_8x4_FH_Rnd0_C,  Skl_Copy_8x4_HF_Rnd0_C,  Skl_Copy_8x4_HH_Rnd0_C  }
,   Skl_H_Pass_16_Copy_Rnd0_C, Skl_H_Pass_Avrg_16_Copy_Rnd0_C, Skl_H_Pass_Avrg_Up_16_Copy_Rnd0_C
,   Skl_H_Pass_16_Copy_Rnd0_C, Skl_H_Pass_Avrg_16_Copy_Rnd0_C, Skl_H_Pass_Avrg_Up_16_Copy_Rnd0_C
,   Skl_V_Pass_16_Copy_Rnd0_C, Skl_V_Pass_Avrg_16_Copy_Rnd0_C, Skl_V_Pass_Avrg_Up_16_Copy_Rnd0_C
,   Skl_H_Pass_8_Copy_Rnd0_C,  Skl_H_Pass_Avrg_8_Copy_Rnd0_C,  Skl_H_Pass_Avrg_Up_8_Copy_Rnd0_C
,   Skl_H_Pass_8_Copy_Rnd0_C,  Skl_H_Pass_Avrg_8_Copy_Rnd0_C,  Skl_H_Pass_Avrg_Up_8_Copy_Rnd0_C
,   Skl_V_Pass_8_Copy_Rnd0_C,  Skl_V_Pass_Avrg_8_Copy_Rnd0_C,  Skl_V_Pass_Avrg_Up_8_Copy_Rnd0_C
,   Skl_SAD_HP_16x16_Rnd0_C, Skl_SAD_HP_8x8_Rnd0_C
};

SKL_MB_FUNCS Skl_MB_Funcs_Add_Rnd0_C = {
  { Skl_Add_16x8_FF_C, Skl_Add_16x8_FH_Rnd0_C, Skl_Add_16x8_HF_Rnd0_C, Skl_Add_16x8_HH_Rnd0_C }
, { Skl_Add_8x8_FF_C,  Skl_Add_8x8_FH_Rnd0_C,  Skl_Add_8x8_HF_Rnd0_C,  Skl_Add_8x8_HH_Rnd0_C  }
, { Skl_Add_8x4_FF_C,  Skl_Add_8x4_FH_Rnd0_C,  Skl_Add_8x4_HF_Rnd0_C,  Skl_Add_8x4_HH_Rnd0_C  }
,   Skl_H_Pass_16_Add_Rnd0_C,  Skl_H_Pass_Avrg_16_Add_Rnd0_C,  Skl_H_Pass_Avrg_Up_16_Add_Rnd0_C
,   Skl_H_Pass_16_Copy_Rnd0_C, Skl_H_Pass_Avrg_16_Copy_Rnd0_C, Skl_H_Pass_Avrg_Up_16_Copy_Rnd0_C
,   Skl_V_Pass_16_Add_Rnd0_C,  Skl_V_Pass_Avrg_16_Add_Rnd0_C,  Skl_V_Pass_Avrg_Up_16_Add_Rnd0_C
,   Skl_H_Pass_8_Add_Rnd0_C,   Skl_H_Pass_Avrg_8_Add_Rnd0_C,   Skl_H_Pass_Avrg_Up_8_Add_Rnd0_C
,   Skl_H_Pass_8_Copy_Rnd0_C,  Skl_H_Pass_Avrg_8_Copy_Rnd0_C,  Skl_H_Pass_Avrg_Up_8_Copy_Rnd0_C
,   Skl_V_Pass_8_Add_Rnd0_C,   Skl_V_Pass_Avrg_8_Add_Rnd0_C,   Skl_V_Pass_Avrg_Up_8_Add_Rnd0_C
,   Skl_SAD_HP_16x16_Rnd0_C, Skl_SAD_HP_8x8_Rnd0_C
};

//////////////////////////////////////////////////////////
// Ref version with basic Q-Pel funcs

extern SKL_MB_QP_PASS_SIGNATURE(Skl_H_Pass_16_Copy_Rnd1_Ref);
extern SKL_MB_QP_PASS_SIGNATURE(Skl_V_Pass_16_Copy_Rnd1_Ref);
extern SKL_MB_QP_PASS_SIGNATURE(Skl_H_Pass_Avrg_16_Copy_Rnd1_Ref);
extern SKL_MB_QP_PASS_SIGNATURE(Skl_V_Pass_Avrg_16_Copy_Rnd1_Ref);
extern SKL_MB_QP_PASS_SIGNATURE(Skl_H_Pass_Avrg_Up_16_Copy_Rnd1_Ref);
extern SKL_MB_QP_PASS_SIGNATURE(Skl_V_Pass_Avrg_Up_16_Copy_Rnd1_Ref);
extern SKL_MB_QP_PASS_SIGNATURE(Skl_H_Pass_8_Copy_Rnd1_Ref);
extern SKL_MB_QP_PASS_SIGNATURE(Skl_V_Pass_8_Copy_Rnd1_Ref);
extern SKL_MB_QP_PASS_SIGNATURE(Skl_H_Pass_Avrg_8_Copy_Rnd1_Ref);
extern SKL_MB_QP_PASS_SIGNATURE(Skl_V_Pass_Avrg_8_Copy_Rnd1_Ref);
extern SKL_MB_QP_PASS_SIGNATURE(Skl_H_Pass_Avrg_Up_8_Copy_Rnd1_Ref);
extern SKL_MB_QP_PASS_SIGNATURE(Skl_V_Pass_Avrg_Up_8_Copy_Rnd1_Ref);

extern SKL_MB_QP_PASS_SIGNATURE(Skl_H_Pass_16_Copy_Rnd0_Ref);
extern SKL_MB_QP_PASS_SIGNATURE(Skl_V_Pass_16_Copy_Rnd0_Ref);
extern SKL_MB_QP_PASS_SIGNATURE(Skl_H_Pass_Avrg_16_Copy_Rnd0_Ref);
extern SKL_MB_QP_PASS_SIGNATURE(Skl_V_Pass_Avrg_16_Copy_Rnd0_Ref);
extern SKL_MB_QP_PASS_SIGNATURE(Skl_H_Pass_Avrg_Up_16_Copy_Rnd0_Ref);
extern SKL_MB_QP_PASS_SIGNATURE(Skl_V_Pass_Avrg_Up_16_Copy_Rnd0_Ref);
extern SKL_MB_QP_PASS_SIGNATURE(Skl_H_Pass_8_Copy_Rnd0_Ref);
extern SKL_MB_QP_PASS_SIGNATURE(Skl_V_Pass_8_Copy_Rnd0_Ref);
extern SKL_MB_QP_PASS_SIGNATURE(Skl_H_Pass_Avrg_8_Copy_Rnd0_Ref);
extern SKL_MB_QP_PASS_SIGNATURE(Skl_V_Pass_Avrg_8_Copy_Rnd0_Ref);
extern SKL_MB_QP_PASS_SIGNATURE(Skl_H_Pass_Avrg_Up_8_Copy_Rnd0_Ref);
extern SKL_MB_QP_PASS_SIGNATURE(Skl_V_Pass_Avrg_Up_8_Copy_Rnd0_Ref);

extern SKL_MB_QP_PASS_SIGNATURE(Skl_H_Pass_16_Add_Rnd0_Ref);
extern SKL_MB_QP_PASS_SIGNATURE(Skl_V_Pass_16_Add_Rnd0_Ref);
extern SKL_MB_QP_PASS_SIGNATURE(Skl_H_Pass_Avrg_16_Add_Rnd0_Ref);
extern SKL_MB_QP_PASS_SIGNATURE(Skl_V_Pass_Avrg_16_Add_Rnd0_Ref);
extern SKL_MB_QP_PASS_SIGNATURE(Skl_H_Pass_Avrg_Up_16_Add_Rnd0_Ref);
extern SKL_MB_QP_PASS_SIGNATURE(Skl_V_Pass_Avrg_Up_16_Add_Rnd0_Ref);
extern SKL_MB_QP_PASS_SIGNATURE(Skl_H_Pass_8_Add_Rnd0_Ref);
extern SKL_MB_QP_PASS_SIGNATURE(Skl_V_Pass_8_Add_Rnd0_Ref);
extern SKL_MB_QP_PASS_SIGNATURE(Skl_H_Pass_Avrg_8_Add_Rnd0_Ref);
extern SKL_MB_QP_PASS_SIGNATURE(Skl_V_Pass_Avrg_8_Add_Rnd0_Ref);
extern SKL_MB_QP_PASS_SIGNATURE(Skl_H_Pass_Avrg_Up_8_Add_Rnd0_Ref);
extern SKL_MB_QP_PASS_SIGNATURE(Skl_V_Pass_Avrg_Up_8_Add_Rnd0_Ref);

//////////////////////////////////////////////////////////

SKL_MB_FUNCS Skl_MB_Funcs_Copy_Rnd1_Ref = {
  { Skl_Copy_16x8_FF_C, Skl_Copy_16x8_FH_Rnd1_C, Skl_Copy_16x8_HF_Rnd1_C, Skl_Copy_16x8_HH_Rnd1_C }
, { Skl_Copy_8x8_FF_C,  Skl_Copy_8x8_FH_Rnd1_C,  Skl_Copy_8x8_HF_Rnd1_C,  Skl_Copy_8x8_HH_Rnd1_C  }
, { Skl_Copy_8x4_FF_C,  Skl_Copy_8x4_FH_Rnd1_C,  Skl_Copy_8x4_HF_Rnd1_C,  Skl_Copy_8x4_HH_Rnd1_C  }
,   Skl_H_Pass_16_Copy_Rnd1_Ref, Skl_H_Pass_Avrg_16_Copy_Rnd1_Ref, Skl_H_Pass_Avrg_Up_16_Copy_Rnd1_Ref
,   Skl_H_Pass_16_Copy_Rnd1_Ref, Skl_H_Pass_Avrg_16_Copy_Rnd1_Ref, Skl_H_Pass_Avrg_Up_16_Copy_Rnd1_Ref
,   Skl_V_Pass_16_Copy_Rnd1_Ref, Skl_V_Pass_Avrg_16_Copy_Rnd1_Ref, Skl_V_Pass_Avrg_Up_16_Copy_Rnd1_Ref
,   Skl_H_Pass_8_Copy_Rnd1_Ref,  Skl_H_Pass_Avrg_8_Copy_Rnd1_Ref,  Skl_H_Pass_Avrg_Up_8_Copy_Rnd1_Ref
,   Skl_H_Pass_8_Copy_Rnd1_Ref,  Skl_H_Pass_Avrg_8_Copy_Rnd1_Ref,  Skl_H_Pass_Avrg_Up_8_Copy_Rnd1_Ref
,   Skl_V_Pass_8_Copy_Rnd1_Ref,  Skl_V_Pass_Avrg_8_Copy_Rnd1_Ref,  Skl_V_Pass_Avrg_Up_8_Copy_Rnd1_Ref
,   Skl_SAD_HP_16x16_Rnd1_C, Skl_SAD_HP_8x8_Rnd1_C
};
SKL_MB_FUNCS Skl_MB_Funcs_Copy_Rnd0_Ref = {
  { Skl_Copy_16x8_FF_C, Skl_Copy_16x8_FH_Rnd0_C, Skl_Copy_16x8_HF_Rnd0_C, Skl_Copy_16x8_HH_Rnd0_C }
, { Skl_Copy_8x8_FF_C,  Skl_Copy_8x8_FH_Rnd0_C,  Skl_Copy_8x8_HF_Rnd0_C,  Skl_Copy_8x8_HH_Rnd0_C  }
, { Skl_Copy_8x4_FF_C,  Skl_Copy_8x4_FH_Rnd0_C,  Skl_Copy_8x4_HF_Rnd0_C,  Skl_Copy_8x4_HH_Rnd0_C  }
,   Skl_H_Pass_16_Copy_Rnd0_Ref, Skl_H_Pass_Avrg_16_Copy_Rnd0_Ref, Skl_H_Pass_Avrg_Up_16_Copy_Rnd0_Ref
,   Skl_H_Pass_16_Copy_Rnd0_Ref, Skl_H_Pass_Avrg_16_Copy_Rnd0_Ref, Skl_H_Pass_Avrg_Up_16_Copy_Rnd0_Ref
,   Skl_V_Pass_16_Copy_Rnd0_Ref, Skl_V_Pass_Avrg_16_Copy_Rnd0_Ref, Skl_V_Pass_Avrg_Up_16_Copy_Rnd0_Ref
,   Skl_H_Pass_8_Copy_Rnd0_Ref,  Skl_H_Pass_Avrg_8_Copy_Rnd0_Ref,  Skl_H_Pass_Avrg_Up_8_Copy_Rnd0_Ref
,   Skl_H_Pass_8_Copy_Rnd0_Ref,  Skl_H_Pass_Avrg_8_Copy_Rnd0_Ref,  Skl_H_Pass_Avrg_Up_8_Copy_Rnd0_Ref
,   Skl_V_Pass_8_Copy_Rnd0_Ref,  Skl_V_Pass_Avrg_8_Copy_Rnd0_Ref,  Skl_V_Pass_Avrg_Up_8_Copy_Rnd0_Ref
,   Skl_SAD_HP_16x16_Rnd0_C, Skl_SAD_HP_8x8_Rnd0_C
};
SKL_MB_FUNCS Skl_MB_Funcs_Add_Rnd0_Ref = {
  { Skl_Add_16x8_FF_C, Skl_Add_16x8_FH_Rnd0_C, Skl_Add_16x8_HF_Rnd0_C, Skl_Add_16x8_HH_Rnd0_C }
, { Skl_Add_8x8_FF_C,  Skl_Add_8x8_FH_Rnd0_C,  Skl_Add_8x8_HF_Rnd0_C,  Skl_Add_8x8_HH_Rnd0_C  }
, { Skl_Add_8x4_FF_C,  Skl_Add_8x4_FH_Rnd0_C,  Skl_Add_8x4_HF_Rnd0_C,  Skl_Add_8x4_HH_Rnd0_C  }
,   Skl_H_Pass_16_Add_Rnd0_Ref,  Skl_H_Pass_Avrg_16_Add_Rnd0_Ref,  Skl_H_Pass_Avrg_Up_16_Add_Rnd0_Ref
,   Skl_H_Pass_16_Copy_Rnd0_Ref, Skl_H_Pass_Avrg_16_Copy_Rnd0_Ref, Skl_H_Pass_Avrg_Up_16_Copy_Rnd0_Ref
,   Skl_V_Pass_16_Add_Rnd0_Ref,  Skl_V_Pass_Avrg_16_Add_Rnd0_Ref,  Skl_V_Pass_Avrg_Up_16_Add_Rnd0_Ref
,   Skl_H_Pass_8_Add_Rnd0_Ref,   Skl_H_Pass_Avrg_8_Add_Rnd0_Ref,   Skl_H_Pass_Avrg_Up_8_Add_Rnd0_Ref
,   Skl_H_Pass_8_Copy_Rnd0_Ref,  Skl_H_Pass_Avrg_8_Copy_Rnd0_Ref,  Skl_H_Pass_Avrg_Up_8_Copy_Rnd0_Ref 
,   Skl_V_Pass_8_Add_Rnd0_Ref,   Skl_V_Pass_Avrg_8_Add_Rnd0_Ref,   Skl_V_Pass_Avrg_Up_8_Add_Rnd0_Ref
,   Skl_SAD_HP_16x16_Rnd0_C, Skl_SAD_HP_8x8_Rnd0_C
};

//////////////////////////////////////////////////////////
// x86-ASM version

#ifdef SKL_USE_ASM

extern MB_FUNC(Skl_Add_8x8_FF_x86);
extern MB_FUNC(Skl_Add_8x8_FH_Rnd0_x86);
extern MB_FUNC(Skl_Add_8x8_HF_Rnd0_x86);
extern MB_FUNC(Skl_Add_8x8_HH_Rnd0_x86);
extern MB_FUNC(Skl_Add_16x8_FF_x86);
extern MB_FUNC(Skl_Add_16x8_FH_Rnd0_x86);
extern MB_FUNC(Skl_Add_16x8_HF_Rnd0_x86);
extern MB_FUNC(Skl_Add_16x8_HH_Rnd0_x86);
extern MB_FUNC(Skl_Copy_8x8_FF_x86);
extern MB_FUNC(Skl_Copy_8x8_FH_Rnd1_x86);
extern MB_FUNC(Skl_Copy_8x8_HF_Rnd1_x86);
extern MB_FUNC(Skl_Copy_8x8_HH_Rnd1_x86);
extern MB_FUNC(Skl_Copy_8x8_FH_Rnd0_x86);
extern MB_FUNC(Skl_Copy_8x8_HF_Rnd0_x86);
extern MB_FUNC(Skl_Copy_8x8_HH_Rnd0_x86);
extern MB_FUNC(Skl_Copy_16x8_FF_x86);
extern MB_FUNC(Skl_Copy_16x8_FH_Rnd1_x86);
extern MB_FUNC(Skl_Copy_16x8_HF_Rnd1_x86);
extern MB_FUNC(Skl_Copy_16x8_HH_Rnd1_x86);
extern MB_FUNC(Skl_Copy_16x8_FH_Rnd0_x86);
extern MB_FUNC(Skl_Copy_16x8_HF_Rnd0_x86);
extern MB_FUNC(Skl_Copy_16x8_HH_Rnd0_x86);

extern SKL_MB_FUNCS Skl_MB_Funcs_Add_x86;
extern SKL_MB_FUNCS Skl_MB_Funcs_Copy_x86;

SKL_MB_FUNCS Skl_MB_Funcs_Copy_Rnd1_x86 = {
  { Skl_Copy_16x8_FF_x86, Skl_Copy_16x8_FH_Rnd1_x86, Skl_Copy_16x8_HF_Rnd1_x86, Skl_Copy_16x8_HH_Rnd1_x86 }
, { Skl_Copy_8x8_FF_x86,  Skl_Copy_8x8_FH_Rnd1_x86,  Skl_Copy_8x8_HF_Rnd1_x86,  Skl_Copy_8x8_HH_Rnd1_x86 }
, { Skl_Copy_8x4_FF_C,    Skl_Copy_8x4_FH_Rnd1_C,    Skl_Copy_8x4_HF_Rnd1_C,    Skl_Copy_8x4_HH_Rnd1_C }
,   Skl_H_Pass_16_Copy_Rnd1_C, Skl_H_Pass_Avrg_16_Copy_Rnd1_C, Skl_H_Pass_Avrg_Up_16_Copy_Rnd1_C
,   Skl_H_Pass_16_Copy_Rnd1_C, Skl_H_Pass_Avrg_16_Copy_Rnd1_C, Skl_H_Pass_Avrg_Up_16_Copy_Rnd1_C
,   Skl_V_Pass_16_Copy_Rnd1_C, Skl_V_Pass_Avrg_16_Copy_Rnd1_C, Skl_V_Pass_Avrg_Up_16_Copy_Rnd1_C
,   Skl_H_Pass_8_Copy_Rnd1_C,  Skl_H_Pass_Avrg_8_Copy_Rnd1_C,  Skl_H_Pass_Avrg_Up_8_Copy_Rnd1_C
,   Skl_H_Pass_8_Copy_Rnd1_C,  Skl_H_Pass_Avrg_8_Copy_Rnd1_C,  Skl_H_Pass_Avrg_Up_8_Copy_Rnd1_C
,   Skl_V_Pass_8_Copy_Rnd1_C,  Skl_V_Pass_Avrg_8_Copy_Rnd1_C,  Skl_V_Pass_Avrg_Up_8_Copy_Rnd1_C
,   Skl_SAD_HP_16x16_Rnd1_C, Skl_SAD_HP_8x8_Rnd1_C
};
SKL_MB_FUNCS Skl_MB_Funcs_Copy_Rnd0_x86 = {
  { Skl_Copy_16x8_FF_x86, Skl_Copy_16x8_FH_Rnd0_x86, Skl_Copy_16x8_HF_Rnd0_x86, Skl_Copy_16x8_HH_Rnd0_x86 }
, { Skl_Copy_8x8_FF_x86,  Skl_Copy_8x8_FH_Rnd0_x86,  Skl_Copy_8x8_HF_Rnd0_x86,  Skl_Copy_8x8_HH_Rnd0_x86 }
, { Skl_Copy_8x4_FF_C,    Skl_Copy_8x4_FH_Rnd0_C,    Skl_Copy_8x4_HF_Rnd0_C,    Skl_Copy_8x4_HH_Rnd0_C }
,   Skl_H_Pass_16_Copy_Rnd0_C, Skl_H_Pass_Avrg_16_Copy_Rnd0_C, Skl_H_Pass_Avrg_Up_16_Copy_Rnd0_C
,   Skl_H_Pass_16_Copy_Rnd0_C, Skl_H_Pass_Avrg_16_Copy_Rnd0_C, Skl_H_Pass_Avrg_Up_16_Copy_Rnd0_C
,   Skl_V_Pass_16_Copy_Rnd0_C, Skl_V_Pass_Avrg_16_Copy_Rnd0_C, Skl_V_Pass_Avrg_Up_16_Copy_Rnd0_C
,   Skl_H_Pass_8_Copy_Rnd0_C,  Skl_H_Pass_Avrg_8_Copy_Rnd0_C,  Skl_H_Pass_Avrg_Up_8_Copy_Rnd0_C
,   Skl_H_Pass_8_Copy_Rnd0_C,  Skl_H_Pass_Avrg_8_Copy_Rnd0_C,  Skl_H_Pass_Avrg_Up_8_Copy_Rnd0_C
,   Skl_V_Pass_8_Copy_Rnd0_C,  Skl_V_Pass_Avrg_8_Copy_Rnd0_C,  Skl_V_Pass_Avrg_Up_8_Copy_Rnd0_C
,   Skl_SAD_HP_16x16_Rnd0_C, Skl_SAD_HP_8x8_Rnd0_C
};
SKL_MB_FUNCS Skl_MB_Funcs_Add_Rnd0_x86 = {
  { Skl_Add_16x8_FF_x86, Skl_Add_16x8_FH_Rnd0_x86, Skl_Add_16x8_HF_Rnd0_x86, Skl_Add_16x8_HH_Rnd0_x86 }
, { Skl_Add_8x8_FF_x86,  Skl_Add_8x8_FH_Rnd0_x86,  Skl_Add_8x8_HF_Rnd0_x86,  Skl_Add_8x8_HH_Rnd0_x86 }
, { Skl_Add_8x4_FF_C,    Skl_Add_8x4_FH_Rnd0_C,    Skl_Add_8x4_HF_Rnd0_C,    Skl_Add_8x4_HH_Rnd0_C }
,   Skl_H_Pass_16_Add_Rnd0_C,  Skl_H_Pass_Avrg_16_Add_Rnd0_C,  Skl_H_Pass_Avrg_Up_16_Add_Rnd0_C
,   Skl_H_Pass_16_Copy_Rnd0_C, Skl_H_Pass_Avrg_16_Copy_Rnd0_C, Skl_H_Pass_Avrg_Up_16_Copy_Rnd0_C
,   Skl_V_Pass_16_Add_Rnd0_C,  Skl_V_Pass_Avrg_16_Add_Rnd0_C,  Skl_V_Pass_Avrg_Up_16_Add_Rnd0_C
,   Skl_H_Pass_8_Add_Rnd0_C,   Skl_H_Pass_Avrg_8_Add_Rnd0_C,   Skl_H_Pass_Avrg_Up_8_Add_Rnd0_C
,   Skl_H_Pass_8_Copy_Rnd0_C,  Skl_H_Pass_Avrg_8_Copy_Rnd0_C,  Skl_H_Pass_Avrg_Up_8_Copy_Rnd0_C
,   Skl_V_Pass_8_Add_Rnd0_C,   Skl_V_Pass_Avrg_8_Add_Rnd0_C,   Skl_V_Pass_Avrg_Up_8_Add_Rnd0_C
,   Skl_SAD_HP_16x16_Rnd0_C, Skl_SAD_HP_8x8_Rnd0_C
};


//////////////////////////////////////////////////////////
// MMX version

extern SKL_MB_FUNCS Skl_MB_Funcs_Add_MMX;
extern SKL_MB_FUNCS Skl_MB_Funcs_Copy_MMX;

extern MB_FUNC(Skl_Add_8x4_FF_MMX);
extern MB_FUNC(Skl_Add_8x4_FH_Rnd0_MMX);
extern MB_FUNC(Skl_Add_8x4_HF_Rnd0_MMX);
extern MB_FUNC(Skl_Add_8x4_HH_Rnd0_MMX);
extern MB_FUNC(Skl_Add_8x8_FF_MMX);
extern MB_FUNC(Skl_Add_8x8_FH_Rnd0_MMX);
extern MB_FUNC(Skl_Add_8x8_HF_Rnd0_MMX);
extern MB_FUNC(Skl_Add_8x8_HH_Rnd0_MMX);
extern MB_FUNC(Skl_Add_16x8_FF_MMX);
extern MB_FUNC(Skl_Add_16x8_FH_Rnd0_MMX);
extern MB_FUNC(Skl_Add_16x8_HF_Rnd0_MMX);
extern MB_FUNC(Skl_Add_16x8_HH_Rnd0_MMX);
extern MB_FUNC(Skl_Copy_8x4_FF_MMX);
extern MB_FUNC(Skl_Copy_8x4_FH_Rnd1_MMX);
extern MB_FUNC(Skl_Copy_8x4_HF_Rnd1_MMX);
extern MB_FUNC(Skl_Copy_8x4_HH_Rnd1_MMX);
extern MB_FUNC(Skl_Copy_8x4_FH_Rnd0_MMX);
extern MB_FUNC(Skl_Copy_8x4_HF_Rnd0_MMX);
extern MB_FUNC(Skl_Copy_8x4_HH_Rnd0_MMX);
extern MB_FUNC(Skl_Copy_8x8_FF_MMX);
extern MB_FUNC(Skl_Copy_8x8_FH_Rnd1_MMX);
extern MB_FUNC(Skl_Copy_8x8_HF_Rnd1_MMX);
extern MB_FUNC(Skl_Copy_8x8_HH_Rnd1_MMX);
extern MB_FUNC(Skl_Copy_8x8_FH_Rnd0_MMX);
extern MB_FUNC(Skl_Copy_8x8_HF_Rnd0_MMX);
extern MB_FUNC(Skl_Copy_8x8_HH_Rnd0_MMX);
extern MB_FUNC(Skl_Copy_16x8_FF_MMX);
extern MB_FUNC(Skl_Copy_16x8_FH_Rnd1_MMX);
extern MB_FUNC(Skl_Copy_16x8_HF_Rnd1_MMX);
extern MB_FUNC(Skl_Copy_16x8_HH_Rnd1_MMX);
extern MB_FUNC(Skl_Copy_16x8_FH_Rnd0_MMX);
extern MB_FUNC(Skl_Copy_16x8_HF_Rnd0_MMX);
extern MB_FUNC(Skl_Copy_16x8_HH_Rnd0_MMX);

extern SKL_MB_QP_PASS_SIGNATURE(Skl_H_Pass_16_Copy_Rnd1_MMX);
extern SKL_MB_QP_PASS_SIGNATURE(Skl_V_Pass_16_Copy_Rnd1_MMX);
extern SKL_MB_QP_PASS_SIGNATURE(Skl_H_Pass_Avrg_16_Copy_Rnd1_MMX);
extern SKL_MB_QP_PASS_SIGNATURE(Skl_V_Pass_Avrg_16_Copy_Rnd1_MMX);
extern SKL_MB_QP_PASS_SIGNATURE(Skl_H_Pass_Avrg_Up_16_Copy_Rnd1_MMX);
extern SKL_MB_QP_PASS_SIGNATURE(Skl_V_Pass_Avrg_Up_16_Copy_Rnd1_MMX);
extern SKL_MB_QP_PASS_SIGNATURE(Skl_H_Pass_8_Copy_Rnd1_MMX);
extern SKL_MB_QP_PASS_SIGNATURE(Skl_V_Pass_8_Copy_Rnd1_MMX);
extern SKL_MB_QP_PASS_SIGNATURE(Skl_H_Pass_Avrg_8_Copy_Rnd1_MMX);
extern SKL_MB_QP_PASS_SIGNATURE(Skl_V_Pass_Avrg_8_Copy_Rnd1_MMX);
extern SKL_MB_QP_PASS_SIGNATURE(Skl_H_Pass_Avrg_Up_8_Copy_Rnd1_MMX);
extern SKL_MB_QP_PASS_SIGNATURE(Skl_V_Pass_Avrg_Up_8_Copy_Rnd1_MMX);

extern SKL_MB_QP_PASS_SIGNATURE(Skl_H_Pass_16_Copy_Rnd0_MMX);
extern SKL_MB_QP_PASS_SIGNATURE(Skl_V_Pass_16_Copy_Rnd0_MMX);
extern SKL_MB_QP_PASS_SIGNATURE(Skl_H_Pass_Avrg_16_Copy_Rnd0_MMX);
extern SKL_MB_QP_PASS_SIGNATURE(Skl_V_Pass_Avrg_16_Copy_Rnd0_MMX);
extern SKL_MB_QP_PASS_SIGNATURE(Skl_H_Pass_Avrg_Up_16_Copy_Rnd0_MMX);
extern SKL_MB_QP_PASS_SIGNATURE(Skl_V_Pass_Avrg_Up_16_Copy_Rnd0_MMX);
extern SKL_MB_QP_PASS_SIGNATURE(Skl_H_Pass_8_Copy_Rnd0_MMX);
extern SKL_MB_QP_PASS_SIGNATURE(Skl_V_Pass_8_Copy_Rnd0_MMX);
extern SKL_MB_QP_PASS_SIGNATURE(Skl_H_Pass_Avrg_8_Copy_Rnd0_MMX);
extern SKL_MB_QP_PASS_SIGNATURE(Skl_V_Pass_Avrg_8_Copy_Rnd0_MMX);
extern SKL_MB_QP_PASS_SIGNATURE(Skl_H_Pass_Avrg_Up_8_Copy_Rnd0_MMX);
extern SKL_MB_QP_PASS_SIGNATURE(Skl_V_Pass_Avrg_Up_8_Copy_Rnd0_MMX);

extern SKL_MB_QP_PASS_SIGNATURE(Skl_H_Pass_16_Add_Rnd0_MMX);
extern SKL_MB_QP_PASS_SIGNATURE(Skl_V_Pass_16_Add_Rnd0_MMX);
extern SKL_MB_QP_PASS_SIGNATURE(Skl_H_Pass_Avrg_16_Add_Rnd0_MMX);
extern SKL_MB_QP_PASS_SIGNATURE(Skl_V_Pass_Avrg_16_Add_Rnd0_MMX);
extern SKL_MB_QP_PASS_SIGNATURE(Skl_H_Pass_Avrg_Up_16_Add_Rnd0_MMX);
extern SKL_MB_QP_PASS_SIGNATURE(Skl_V_Pass_Avrg_Up_16_Add_Rnd0_MMX);
extern SKL_MB_QP_PASS_SIGNATURE(Skl_H_Pass_8_Add_Rnd0_MMX);
extern SKL_MB_QP_PASS_SIGNATURE(Skl_V_Pass_8_Add_Rnd0_MMX);
extern SKL_MB_QP_PASS_SIGNATURE(Skl_H_Pass_Avrg_8_Add_Rnd0_MMX);
extern SKL_MB_QP_PASS_SIGNATURE(Skl_V_Pass_Avrg_8_Add_Rnd0_MMX);
extern SKL_MB_QP_PASS_SIGNATURE(Skl_H_Pass_Avrg_Up_8_Add_Rnd0_MMX);
extern SKL_MB_QP_PASS_SIGNATURE(Skl_V_Pass_Avrg_Up_8_Add_Rnd0_MMX);

SKL_MB_FUNCS Skl_MB_Funcs_Copy_Rnd1_MMX = {
  { Skl_Copy_16x8_FF_MMX, Skl_Copy_16x8_FH_Rnd1_MMX, Skl_Copy_16x8_HF_Rnd1_MMX, Skl_Copy_16x8_HH_Rnd1_MMX }
, { Skl_Copy_8x8_FF_MMX,  Skl_Copy_8x8_FH_Rnd1_MMX,  Skl_Copy_8x8_HF_Rnd1_MMX,  Skl_Copy_8x8_HH_Rnd1_MMX }
, { Skl_Copy_8x4_FF_MMX,  Skl_Copy_8x4_FH_Rnd1_MMX,  Skl_Copy_8x4_HF_Rnd1_MMX,  Skl_Copy_8x4_HH_Rnd1_MMX  }
,   Skl_H_Pass_16_Copy_Rnd1_MMX, Skl_H_Pass_Avrg_16_Copy_Rnd1_MMX, Skl_H_Pass_Avrg_Up_16_Copy_Rnd1_MMX
,   Skl_H_Pass_16_Copy_Rnd1_MMX, Skl_H_Pass_Avrg_16_Copy_Rnd1_MMX, Skl_H_Pass_Avrg_Up_16_Copy_Rnd1_MMX
,   Skl_V_Pass_16_Copy_Rnd1_MMX, Skl_V_Pass_Avrg_16_Copy_Rnd1_MMX, Skl_V_Pass_Avrg_Up_16_Copy_Rnd1_MMX
,   Skl_H_Pass_8_Copy_Rnd1_MMX,  Skl_H_Pass_Avrg_8_Copy_Rnd1_MMX,  Skl_H_Pass_Avrg_Up_8_Copy_Rnd1_MMX
,   Skl_H_Pass_8_Copy_Rnd1_MMX,  Skl_H_Pass_Avrg_8_Copy_Rnd1_MMX,  Skl_H_Pass_Avrg_Up_8_Copy_Rnd1_MMX
,   Skl_V_Pass_8_Copy_Rnd1_MMX,  Skl_V_Pass_Avrg_8_Copy_Rnd1_MMX,  Skl_V_Pass_Avrg_Up_8_Copy_Rnd1_MMX
,   Skl_SAD_HP_16x16_Rnd1_C, Skl_SAD_HP_8x8_Rnd1_C
};
SKL_MB_FUNCS Skl_MB_Funcs_Copy_Rnd0_MMX = {
  { Skl_Copy_16x8_FF_MMX, Skl_Copy_16x8_FH_Rnd0_MMX, Skl_Copy_16x8_HF_Rnd0_MMX, Skl_Copy_16x8_HH_Rnd0_MMX }
, { Skl_Copy_8x8_FF_MMX,  Skl_Copy_8x8_FH_Rnd0_MMX,  Skl_Copy_8x8_HF_Rnd0_MMX,  Skl_Copy_8x8_HH_Rnd0_MMX }
, { Skl_Copy_8x4_FF_MMX,  Skl_Copy_8x4_FH_Rnd0_MMX,  Skl_Copy_8x4_HF_Rnd0_MMX,  Skl_Copy_8x4_HH_Rnd0_MMX  }
,   Skl_H_Pass_16_Copy_Rnd0_MMX, Skl_H_Pass_Avrg_16_Copy_Rnd0_MMX, Skl_H_Pass_Avrg_Up_16_Copy_Rnd0_MMX
,   Skl_H_Pass_16_Copy_Rnd0_MMX, Skl_H_Pass_Avrg_16_Copy_Rnd0_MMX, Skl_H_Pass_Avrg_Up_16_Copy_Rnd0_MMX
,   Skl_V_Pass_16_Copy_Rnd0_MMX, Skl_V_Pass_Avrg_16_Copy_Rnd0_MMX, Skl_V_Pass_Avrg_Up_16_Copy_Rnd0_MMX
,   Skl_H_Pass_8_Copy_Rnd0_MMX,  Skl_H_Pass_Avrg_8_Copy_Rnd0_MMX,  Skl_H_Pass_Avrg_Up_8_Copy_Rnd0_MMX
,   Skl_H_Pass_8_Copy_Rnd0_MMX,  Skl_H_Pass_Avrg_8_Copy_Rnd0_MMX,  Skl_H_Pass_Avrg_Up_8_Copy_Rnd0_MMX
,   Skl_V_Pass_8_Copy_Rnd0_MMX,  Skl_V_Pass_Avrg_8_Copy_Rnd0_MMX,  Skl_V_Pass_Avrg_Up_8_Copy_Rnd0_MMX
,   Skl_SAD_HP_16x16_Rnd0_C, Skl_SAD_HP_8x8_Rnd0_C
};
SKL_MB_FUNCS Skl_MB_Funcs_Add_Rnd0_MMX = {
  { Skl_Add_16x8_FF_MMX, Skl_Add_16x8_FH_Rnd0_MMX, Skl_Add_16x8_HF_Rnd0_MMX, Skl_Add_16x8_HH_Rnd0_MMX }
, { Skl_Add_8x8_FF_MMX,  Skl_Add_8x8_FH_Rnd0_MMX,  Skl_Add_8x8_HF_Rnd0_MMX,  Skl_Add_8x8_HH_Rnd0_MMX }
, { Skl_Add_8x4_FF_MMX,  Skl_Add_8x4_FH_Rnd0_MMX,  Skl_Add_8x4_HF_Rnd0_MMX,  Skl_Add_8x4_HH_Rnd0_MMX  }
,   Skl_H_Pass_16_Add_Rnd0_MMX,  Skl_H_Pass_Avrg_16_Add_Rnd0_MMX,  Skl_H_Pass_Avrg_Up_16_Add_Rnd0_MMX
,   Skl_H_Pass_16_Copy_Rnd0_MMX, Skl_H_Pass_Avrg_16_Copy_Rnd0_MMX, Skl_H_Pass_Avrg_Up_16_Copy_Rnd0_MMX
,   Skl_V_Pass_16_Add_Rnd0_MMX,  Skl_V_Pass_Avrg_16_Add_Rnd0_MMX,  Skl_V_Pass_Avrg_Up_16_Add_Rnd0_MMX
,   Skl_H_Pass_8_Add_Rnd0_MMX,   Skl_H_Pass_Avrg_8_Add_Rnd0_MMX,   Skl_H_Pass_Avrg_Up_8_Add_Rnd0_MMX
,   Skl_H_Pass_8_Copy_Rnd0_MMX,  Skl_H_Pass_Avrg_8_Copy_Rnd0_MMX,  Skl_H_Pass_Avrg_Up_8_Copy_Rnd0_MMX
,   Skl_V_Pass_8_Add_Rnd0_MMX,   Skl_V_Pass_Avrg_8_Add_Rnd0_MMX,   Skl_V_Pass_Avrg_Up_8_Add_Rnd0_MMX
,   Skl_SAD_HP_16x16_Rnd0_C, Skl_SAD_HP_8x8_Rnd0_C
};
extern MB_FUNC(Skl_H_Pass_2Taps_MMX);
extern MB_FUNC(Skl_V_Pass_2Taps_MMX);
extern MB_FUNC(Skl_HV_Pass_2Taps_MMX);

const SKL_HV_FILTER Skl_Filter_2_MMX = {
  Skl_H_Pass_2Taps_MMX, Skl_V_Pass_2Taps_MMX, Skl_HV_Pass_2Taps_MMX
};


//////////////////////////////////////////////////////////
// tables sent to ASM code

extern SKL_INT16 Skl_MMX_Expand[256][4];

  // 17 tables, 2K each => 34K
  // Mirroring can be acheived composing 11 basic tables
  // (for instance: (23,19,-6,3)=(20,20,-6,3)+(3,-1,0,0)
  // Using Symmetries (and bswap) could reduce further
  // the memory to 7 tables (->14K).

extern SKL_INT16 Skl_FIR_1_0_0_0[256][4];
extern SKL_INT16 Skl_FIR_3_1_0_0[256][4];
extern SKL_INT16 Skl_FIR_6_3_1_0[256][4];
extern SKL_INT16 Skl_FIR_14_3_2_1[256][4];
extern SKL_INT16 Skl_FIR_20_6_3_1[256][4];
extern SKL_INT16 Skl_FIR_20_20_6_3[256][4];
extern SKL_INT16 Skl_FIR_23_19_6_3[256][4];
extern SKL_INT16 Skl_FIR_7_20_20_6[256][4];
extern SKL_INT16 Skl_FIR_6_20_20_6[256][4];
extern SKL_INT16 Skl_FIR_6_20_20_7[256][4];
extern SKL_INT16 Skl_FIR_3_6_20_20[256][4];
extern SKL_INT16 Skl_FIR_3_6_19_23[256][4];
extern SKL_INT16 Skl_FIR_1_3_6_20[256][4];
extern SKL_INT16 Skl_FIR_1_2_3_14[256][4];
extern SKL_INT16 Skl_FIR_0_1_3_6[256][4];
extern SKL_INT16 Skl_FIR_0_0_1_3[256][4];
extern SKL_INT16 Skl_FIR_0_0_0_1[256][4];

extern void Skl_Init_QP_MMX();

//////////////////////////////////////////////////////////

SKL_INT16 Skl_FIR_1_0_0_0[256][4];
SKL_INT16 Skl_FIR_3_1_0_0[256][4];
SKL_INT16 Skl_FIR_6_3_1_0[256][4];
SKL_INT16 Skl_FIR_14_3_2_1[256][4];
SKL_INT16 Skl_FIR_20_6_3_1[256][4];
SKL_INT16 Skl_FIR_20_20_6_3[256][4];
SKL_INT16 Skl_FIR_23_19_6_3[256][4];
SKL_INT16 Skl_FIR_7_20_20_6[256][4];
SKL_INT16 Skl_FIR_6_20_20_6[256][4];
SKL_INT16 Skl_FIR_6_20_20_7[256][4];
SKL_INT16 Skl_FIR_3_6_20_20[256][4];
SKL_INT16 Skl_FIR_3_6_19_23[256][4];
SKL_INT16 Skl_FIR_1_3_6_20[256][4];
SKL_INT16 Skl_FIR_1_2_3_14[256][4];
SKL_INT16 Skl_FIR_0_1_3_6[256][4];
SKL_INT16 Skl_FIR_0_0_1_3[256][4];
SKL_INT16 Skl_FIR_0_0_0_1[256][4];

SKL_INT16 Skl_MMX_Expand[256][4];
static void Init_FIR_Table(SKL_INT16 Tab[][4],
                           int A, int B, int C, int D)
{
  for(int i=0; i<256; ++i) {
    Tab[i][0] = i*A;
    Tab[i][1] = i*B;
    Tab[i][2] = i*C;
    Tab[i][3] = i*D;
  }
}

void Skl_Init_QP_MMX()
{
  for(int i=0; i<256; ++i) {
    Skl_MMX_Expand[i][0] = i;
    Skl_MMX_Expand[i][1] = i;
    Skl_MMX_Expand[i][2] = i;
    Skl_MMX_Expand[i][3] = i;
  }
  Init_FIR_Table(Skl_FIR_1_0_0_0,   -1,  0,  0,  0);
  Init_FIR_Table(Skl_FIR_3_1_0_0,    3, -1,  0,  0);
  Init_FIR_Table(Skl_FIR_6_3_1_0,   -6,  3, -1,  0);
  Init_FIR_Table(Skl_FIR_14_3_2_1,  14, -3,  2, -1);
  Init_FIR_Table(Skl_FIR_20_6_3_1,  20, -6,  3, -1);
  Init_FIR_Table(Skl_FIR_20_20_6_3, 20, 20, -6,  3);
  Init_FIR_Table(Skl_FIR_23_19_6_3, 23, 19, -6,  3);
  Init_FIR_Table(Skl_FIR_7_20_20_6, -7, 20, 20, -6);
  Init_FIR_Table(Skl_FIR_6_20_20_6, -6, 20, 20, -6);
  Init_FIR_Table(Skl_FIR_6_20_20_7, -6, 20, 20, -7);
  Init_FIR_Table(Skl_FIR_3_6_20_20,  3, -6, 20, 20);
  Init_FIR_Table(Skl_FIR_3_6_19_23,  3, -6, 19, 23);
  Init_FIR_Table(Skl_FIR_1_3_6_20,  -1,  3, -6, 20);
  Init_FIR_Table(Skl_FIR_1_2_3_14,  -1,  2, -3, 14);
  Init_FIR_Table(Skl_FIR_0_1_3_6,    0, -1,  3, -6);
  Init_FIR_Table(Skl_FIR_0_0_1_3,    0,  0, -1,  3);
  Init_FIR_Table(Skl_FIR_0_0_0_1,    0,  0,  0, -1);
}


//////////////////////////////////////////////////////////
// SSE version

extern SKL_MB_FUNCS Skl_MB_Funcs_Add_SSE;
extern SKL_MB_FUNCS Skl_MB_Funcs_Copy_SSE;

extern MB_FUNC(Skl_Add_8x4_FF_SSE);
extern MB_FUNC(Skl_Add_8x4_FH_Rnd0_SSE);
extern MB_FUNC(Skl_Add_8x4_HF_Rnd0_SSE);
extern MB_FUNC(Skl_Add_8x4_HH_Rnd0_SSE);
extern MB_FUNC(Skl_Add_8x8_FF_SSE);
extern MB_FUNC(Skl_Add_8x8_FH_Rnd0_SSE);
extern MB_FUNC(Skl_Add_8x8_HF_Rnd0_SSE);
extern MB_FUNC(Skl_Add_8x8_HH_Rnd0_SSE);
extern MB_FUNC(Skl_Add_16x8_FF_SSE);
extern MB_FUNC(Skl_Add_16x8_FH_Rnd0_SSE);
extern MB_FUNC(Skl_Add_16x8_HF_Rnd0_SSE);
extern MB_FUNC(Skl_Add_16x8_HH_Rnd0_SSE);
extern MB_FUNC(Skl_Copy_8x4_FH_Rnd1_SSE);
extern MB_FUNC(Skl_Copy_8x4_HF_Rnd1_SSE);
extern MB_FUNC(Skl_Copy_8x4_HH_Rnd1_SSE);
extern MB_FUNC(Skl_Copy_8x4_FH_Rnd0_SSE);
extern MB_FUNC(Skl_Copy_8x4_HF_Rnd0_SSE);
extern MB_FUNC(Skl_Copy_8x4_HH_Rnd0_SSE);
extern MB_FUNC(Skl_Copy_8x8_FH_Rnd1_SSE);
extern MB_FUNC(Skl_Copy_8x8_HF_Rnd1_SSE);
extern MB_FUNC(Skl_Copy_8x8_HH_Rnd1_SSE);
extern MB_FUNC(Skl_Copy_8x8_FH_Rnd0_SSE);
extern MB_FUNC(Skl_Copy_8x8_HF_Rnd0_SSE);
extern MB_FUNC(Skl_Copy_8x8_HH_Rnd0_SSE);
extern MB_FUNC(Skl_Copy_16x8_FH_Rnd1_SSE);
extern MB_FUNC(Skl_Copy_16x8_HF_Rnd1_SSE);
extern MB_FUNC(Skl_Copy_16x8_HH_Rnd1_SSE);
extern MB_FUNC(Skl_Copy_16x8_FH_Rnd0_SSE);
extern MB_FUNC(Skl_Copy_16x8_HF_Rnd0_SSE);
extern MB_FUNC(Skl_Copy_16x8_HH_Rnd0_SSE);


SKL_MB_FUNCS Skl_MB_Funcs_Copy_Rnd1_SSE = {
  { Skl_Copy_16x8_FF_MMX, Skl_Copy_16x8_FH_Rnd1_SSE, Skl_Copy_16x8_HF_Rnd1_SSE, Skl_Copy_16x8_HH_Rnd1_SSE }
, { Skl_Copy_8x8_FF_MMX,  Skl_Copy_8x8_FH_Rnd1_SSE,  Skl_Copy_8x8_HF_Rnd1_SSE,  Skl_Copy_8x8_HH_Rnd1_SSE }
, { Skl_Copy_8x4_FF_MMX,  Skl_Copy_8x4_FH_Rnd1_SSE,  Skl_Copy_8x4_HF_Rnd1_SSE,  Skl_Copy_8x4_HH_Rnd1_SSE  }
,   Skl_H_Pass_16_Copy_Rnd1_MMX, Skl_H_Pass_Avrg_16_Copy_Rnd1_MMX, Skl_H_Pass_Avrg_Up_16_Copy_Rnd1_MMX
,   Skl_H_Pass_16_Copy_Rnd1_MMX, Skl_H_Pass_Avrg_16_Copy_Rnd1_MMX, Skl_H_Pass_Avrg_Up_16_Copy_Rnd1_MMX
,   Skl_V_Pass_16_Copy_Rnd1_MMX, Skl_V_Pass_Avrg_16_Copy_Rnd1_MMX, Skl_V_Pass_Avrg_Up_16_Copy_Rnd1_MMX
,   Skl_H_Pass_8_Copy_Rnd1_MMX,  Skl_H_Pass_Avrg_8_Copy_Rnd1_MMX,  Skl_H_Pass_Avrg_Up_8_Copy_Rnd1_MMX
,   Skl_H_Pass_8_Copy_Rnd1_MMX,  Skl_H_Pass_Avrg_8_Copy_Rnd1_MMX,  Skl_H_Pass_Avrg_Up_8_Copy_Rnd1_MMX 
,   Skl_V_Pass_8_Copy_Rnd1_MMX,  Skl_V_Pass_Avrg_8_Copy_Rnd1_MMX,  Skl_V_Pass_Avrg_Up_8_Copy_Rnd1_MMX
,   Skl_SAD_HP_16x16_Rnd1_C, Skl_SAD_HP_8x8_Rnd1_C
};
SKL_MB_FUNCS Skl_MB_Funcs_Copy_Rnd0_SSE = {
  { Skl_Copy_16x8_FF_MMX, Skl_Copy_16x8_FH_Rnd0_SSE, Skl_Copy_16x8_HF_Rnd0_SSE, Skl_Copy_16x8_HH_Rnd0_SSE }
, { Skl_Copy_8x8_FF_MMX,  Skl_Copy_8x8_FH_Rnd0_SSE,  Skl_Copy_8x8_HF_Rnd0_SSE,  Skl_Copy_8x8_HH_Rnd0_SSE }
, { Skl_Copy_8x4_FF_MMX,  Skl_Copy_8x4_FH_Rnd0_SSE,  Skl_Copy_8x4_HF_Rnd0_SSE,  Skl_Copy_8x4_HH_Rnd0_SSE  }
,   Skl_H_Pass_16_Copy_Rnd0_MMX, Skl_H_Pass_Avrg_16_Copy_Rnd0_MMX, Skl_H_Pass_Avrg_Up_16_Copy_Rnd0_MMX
,   Skl_H_Pass_16_Copy_Rnd0_MMX, Skl_H_Pass_Avrg_16_Copy_Rnd0_MMX, Skl_H_Pass_Avrg_Up_16_Copy_Rnd0_MMX
,   Skl_V_Pass_16_Copy_Rnd0_MMX, Skl_V_Pass_Avrg_16_Copy_Rnd0_MMX, Skl_V_Pass_Avrg_Up_16_Copy_Rnd0_MMX
,   Skl_H_Pass_8_Copy_Rnd0_MMX,  Skl_H_Pass_Avrg_8_Copy_Rnd0_MMX,  Skl_H_Pass_Avrg_Up_8_Copy_Rnd0_MMX
,   Skl_H_Pass_8_Copy_Rnd0_MMX,  Skl_H_Pass_Avrg_8_Copy_Rnd0_MMX,  Skl_H_Pass_Avrg_Up_8_Copy_Rnd0_MMX 
,   Skl_V_Pass_8_Copy_Rnd0_MMX,  Skl_V_Pass_Avrg_8_Copy_Rnd0_MMX,  Skl_V_Pass_Avrg_Up_8_Copy_Rnd0_MMX
,   Skl_SAD_HP_16x16_Rnd0_C, Skl_SAD_HP_8x8_Rnd0_C
};
SKL_MB_FUNCS Skl_MB_Funcs_Add_Rnd0_SSE = {
  { Skl_Add_16x8_FF_SSE, Skl_Add_16x8_FH_Rnd0_SSE, Skl_Add_16x8_HF_Rnd0_SSE, Skl_Add_16x8_HH_Rnd0_SSE }
, { Skl_Add_8x8_FF_SSE,  Skl_Add_8x8_FH_Rnd0_SSE,  Skl_Add_8x8_HF_Rnd0_SSE,  Skl_Add_8x8_HH_Rnd0_SSE }
, { Skl_Add_8x4_FF_SSE,  Skl_Add_8x4_FH_Rnd0_SSE,  Skl_Add_8x4_HF_Rnd0_SSE,  Skl_Add_8x4_HH_Rnd0_SSE  }
,   Skl_H_Pass_16_Add_Rnd0_MMX,  Skl_H_Pass_Avrg_16_Add_Rnd0_MMX,  Skl_H_Pass_Avrg_Up_16_Add_Rnd0_MMX
,   Skl_H_Pass_16_Copy_Rnd0_MMX, Skl_H_Pass_Avrg_16_Copy_Rnd0_MMX, Skl_H_Pass_Avrg_Up_16_Copy_Rnd0_MMX
,   Skl_V_Pass_16_Add_Rnd0_MMX,  Skl_V_Pass_Avrg_16_Add_Rnd0_MMX,  Skl_V_Pass_Avrg_Up_16_Add_Rnd0_MMX
,   Skl_H_Pass_8_Add_Rnd0_MMX,   Skl_H_Pass_Avrg_8_Add_Rnd0_MMX,   Skl_H_Pass_Avrg_Up_8_Add_Rnd0_MMX
,   Skl_H_Pass_8_Copy_Rnd0_MMX,  Skl_H_Pass_Avrg_8_Copy_Rnd0_MMX,  Skl_H_Pass_Avrg_Up_8_Copy_Rnd0_MMX 
,   Skl_V_Pass_8_Add_Rnd0_MMX,   Skl_V_Pass_Avrg_8_Add_Rnd0_MMX,   Skl_V_Pass_Avrg_Up_8_Add_Rnd0_MMX
,   Skl_SAD_HP_16x16_Rnd0_C, Skl_SAD_HP_8x8_Rnd0_C
};

//////////////////////////////////////////////////////////

extern MB_FUNC(Skl_H_Pass_2Taps_SSE);
extern MB_FUNC(Skl_V_Pass_2Taps_SSE);
extern MB_FUNC(Skl_HV_Pass_2Taps_SSE);

const SKL_HV_FILTER Skl_Filter_2_SSE = {
  Skl_H_Pass_2Taps_SSE, Skl_V_Pass_2Taps_SSE, Skl_HV_Pass_2Taps_SSE
};

//////////////////////////////////////////////////////////
// SSE2 version

extern SKL_MB_FUNCS Skl_MB_Funcs_Add_SSE2;
extern SKL_MB_FUNCS Skl_MB_Funcs_Copy_SSE2;

extern MB_FUNC(Skl_Add_16x8_FF_SSE2);
extern MB_FUNC(Skl_Add_16x8_FH_Rnd0_SSE2);
extern MB_FUNC(Skl_Add_16x8_HF_Rnd0_SSE2);
extern MB_FUNC(Skl_Add_16x8_HH_Rnd0_SSE2);
extern MB_FUNC(Skl_Copy_16x8_FF_SSE2);
extern MB_FUNC(Skl_Copy_16x8_FH_Rnd0_SSE2);
extern MB_FUNC(Skl_Copy_16x8_HF_Rnd0_SSE2);
extern MB_FUNC(Skl_Copy_16x8_HH_Rnd0_SSE2);
extern MB_FUNC(Skl_Copy_16x8_FH_Rnd1_SSE2);
extern MB_FUNC(Skl_Copy_16x8_HF_Rnd1_SSE2);
extern MB_FUNC(Skl_Copy_16x8_HH_Rnd1_SSE2);

SKL_MB_FUNCS Skl_MB_Funcs_Copy_Rnd1_SSE2 = {
  { Skl_Copy_16x8_FF_SSE2, Skl_Copy_16x8_FH_Rnd1_SSE2, Skl_Copy_16x8_HF_Rnd1_SSE2, Skl_Copy_16x8_HH_Rnd1_SSE2 }
, { Skl_Copy_8x8_FF_MMX,   Skl_Copy_8x8_FH_Rnd1_SSE,   Skl_Copy_8x8_HF_Rnd1_SSE,   Skl_Copy_8x8_HH_Rnd1_SSE }
, { Skl_Copy_8x4_FF_MMX,   Skl_Copy_8x4_FH_Rnd1_SSE,   Skl_Copy_8x4_HF_Rnd1_SSE,   Skl_Copy_8x4_HH_Rnd1_SSE }
,   Skl_H_Pass_16_Copy_Rnd1_MMX, Skl_H_Pass_Avrg_16_Copy_Rnd1_MMX, Skl_H_Pass_Avrg_Up_16_Copy_Rnd1_MMX
,   Skl_H_Pass_16_Copy_Rnd1_MMX, Skl_H_Pass_Avrg_16_Copy_Rnd1_MMX, Skl_H_Pass_Avrg_Up_16_Copy_Rnd1_MMX
,   Skl_V_Pass_16_Copy_Rnd1_MMX, Skl_V_Pass_Avrg_16_Copy_Rnd1_MMX, Skl_V_Pass_Avrg_Up_16_Copy_Rnd1_MMX
,   Skl_H_Pass_8_Copy_Rnd1_MMX,  Skl_H_Pass_Avrg_8_Copy_Rnd1_MMX,  Skl_H_Pass_Avrg_Up_8_Copy_Rnd1_MMX
,   Skl_H_Pass_8_Copy_Rnd1_MMX,  Skl_H_Pass_Avrg_8_Copy_Rnd1_MMX,  Skl_H_Pass_Avrg_Up_8_Copy_Rnd1_MMX 
,   Skl_V_Pass_8_Copy_Rnd1_MMX,  Skl_V_Pass_Avrg_8_Copy_Rnd1_MMX,  Skl_V_Pass_Avrg_Up_8_Copy_Rnd1_MMX
,   Skl_SAD_HP_16x16_Rnd1_C, Skl_SAD_HP_8x8_Rnd1_C
};
SKL_MB_FUNCS Skl_MB_Funcs_Copy_Rnd0_SSE2 = {
  { Skl_Copy_16x8_FF_SSE2, Skl_Copy_16x8_FH_Rnd0_SSE2, Skl_Copy_16x8_HF_Rnd0_SSE2, Skl_Copy_16x8_HH_Rnd0_SSE2 }
, { Skl_Copy_8x8_FF_MMX,   Skl_Copy_8x8_FH_Rnd0_SSE,   Skl_Copy_8x8_HF_Rnd0_SSE,   Skl_Copy_8x8_HH_Rnd0_SSE }
, { Skl_Copy_8x4_FF_MMX,   Skl_Copy_8x4_FH_Rnd0_SSE,   Skl_Copy_8x4_HF_Rnd0_SSE,   Skl_Copy_8x4_HH_Rnd0_SSE }
,   Skl_H_Pass_16_Copy_Rnd0_MMX, Skl_H_Pass_Avrg_16_Copy_Rnd0_MMX, Skl_H_Pass_Avrg_Up_16_Copy_Rnd0_MMX
,   Skl_H_Pass_16_Copy_Rnd0_MMX, Skl_H_Pass_Avrg_16_Copy_Rnd0_MMX, Skl_H_Pass_Avrg_Up_16_Copy_Rnd0_MMX
,   Skl_V_Pass_16_Copy_Rnd0_MMX, Skl_V_Pass_Avrg_16_Copy_Rnd0_MMX, Skl_V_Pass_Avrg_Up_16_Copy_Rnd0_MMX
,   Skl_H_Pass_8_Copy_Rnd0_MMX,  Skl_H_Pass_Avrg_8_Copy_Rnd0_MMX,  Skl_H_Pass_Avrg_Up_8_Copy_Rnd0_MMX
,   Skl_H_Pass_8_Copy_Rnd0_MMX,  Skl_H_Pass_Avrg_8_Copy_Rnd0_MMX,  Skl_H_Pass_Avrg_Up_8_Copy_Rnd0_MMX
,   Skl_V_Pass_8_Copy_Rnd0_MMX,  Skl_V_Pass_Avrg_8_Copy_Rnd0_MMX,  Skl_V_Pass_Avrg_Up_8_Copy_Rnd0_MMX
,   Skl_SAD_HP_16x16_Rnd0_C, Skl_SAD_HP_8x8_Rnd0_C
};
SKL_MB_FUNCS Skl_MB_Funcs_Add_Rnd0_SSE2 = {
  { Skl_Add_16x8_FF_SSE2, Skl_Add_16x8_FH_Rnd0_SSE2, Skl_Add_16x8_HF_Rnd0_SSE2, Skl_Add_16x8_HH_Rnd0_SSE2 }
, { Skl_Add_8x8_FF_MMX,   Skl_Add_8x8_FH_Rnd0_SSE,   Skl_Add_8x8_HF_Rnd0_SSE,   Skl_Add_8x8_HH_Rnd0_SSE }
, { Skl_Add_8x4_FF_MMX,   Skl_Add_8x4_FH_Rnd0_SSE,   Skl_Add_8x4_HF_Rnd0_SSE,   Skl_Add_8x4_HH_Rnd0_SSE }
,   Skl_H_Pass_16_Add_Rnd0_MMX,  Skl_H_Pass_Avrg_16_Add_Rnd0_MMX,  Skl_H_Pass_Avrg_Up_16_Add_Rnd0_MMX
,   Skl_H_Pass_16_Copy_Rnd0_MMX, Skl_H_Pass_Avrg_16_Copy_Rnd0_MMX, Skl_H_Pass_Avrg_Up_16_Copy_Rnd0_MMX
,   Skl_V_Pass_16_Add_Rnd0_MMX,  Skl_V_Pass_Avrg_16_Add_Rnd0_MMX,  Skl_V_Pass_Avrg_Up_16_Add_Rnd0_MMX
,   Skl_H_Pass_8_Add_Rnd0_MMX,   Skl_H_Pass_Avrg_8_Add_Rnd0_MMX,   Skl_H_Pass_Avrg_Up_8_Add_Rnd0_MMX
,   Skl_H_Pass_8_Copy_Rnd0_MMX,  Skl_H_Pass_Avrg_8_Copy_Rnd0_MMX,  Skl_H_Pass_Avrg_Up_8_Copy_Rnd0_MMX
,   Skl_V_Pass_8_Add_Rnd0_MMX,   Skl_V_Pass_Avrg_8_Add_Rnd0_MMX,   Skl_V_Pass_Avrg_Up_8_Add_Rnd0_MMX
,   Skl_SAD_HP_16x16_Rnd0_C, Skl_SAD_HP_8x8_Rnd0_C
};

#endif    /* SKL_USE_ASM */

//////////////////////////////////////////////////////////
// Upsampling (3,1) filter

static inline void Filter_31(SKL_BYTE *Dst1, SKL_BYTE *Dst2,
                             const SKL_INT16 *Src1, const SKL_INT16 *Src2)
{
    /* Src[] is assumed to be >=0. So we can use ">>2" instead of "/2" */
  SKL_INT16 a = (3*Src1[0]+  Src2[0]+2) >> 2;
  SKL_INT16 b = (  Src1[0]+3*Src2[0]+2) >> 2;
  COPY( Dst1[0], a );
  COPY( Dst2[0], b );

}

static inline void Filter_9331(SKL_BYTE *Dst1, SKL_BYTE *Dst2,
                               const SKL_INT16 *Src1, const SKL_INT16 *Src2)
{
    /* Src[] is assumed to be >=0. So we can use ">>4" instead of "/16" */
  SKL_INT16 a = (9*Src1[0]+  3*Src1[1]+ 3*Src2[0] + 1*Src2[1] + 8) >> 4;
  SKL_INT16 b = (3*Src1[0]+  9*Src1[1]+ 1*Src2[0] + 3*Src2[1] + 8) >> 4;
  SKL_INT16 c = (3*Src1[0]+  1*Src1[1]+ 9*Src2[0] + 3*Src2[1] + 8) >> 4;
  SKL_INT16 d = (1*Src1[0]+  3*Src1[1]+ 3*Src2[0] + 9*Src2[1] + 8) >> 4;
  COPY( Dst1[0], a );
  COPY( Dst1[1], b );
  COPY( Dst2[0], c );
  COPY( Dst2[1], d );
}

void Skl_Copy_Upsampled_8x8_16To8_C(SKL_BYTE *Dst, const SKL_INT16 *Src, const int BpS)
{
  int x, y;

  COPY( Dst[0], Src[0] );
  for(x=0; x<7; ++x) Filter_31(Dst+2*x+1, Dst+2*x+2, Src+x, Src+x+1);
  COPY( Dst[15], Src[7] );
  Dst += BpS;
  for(y=0; y<7; ++y) {
    SKL_BYTE *const Dst2 = Dst + BpS;
    Filter_31(Dst, Dst2, Src, Src+8);
    for(x=0; x<7; ++x)
      Filter_9331(Dst+2*x+1, Dst2+2*x+1, Src+x, Src+x+8);
    Filter_31(Dst+15, Dst2+15, Src+7, Src+7+8);
    Src += 8; 
    Dst += 2*BpS;
  }
  COPY( Dst[0], Src[0] );
  for(x=0; x<7; ++x) Filter_31(Dst+2*x+1, Dst+2*x+2, Src+x, Src+x+1);
  COPY( Dst[15], Src[7] );
}

static inline void Filter_Add_31(SKL_BYTE *Dst1, SKL_BYTE *Dst2,
                             const SKL_INT16 *Src1, const SKL_INT16 *Src2)
{
    /* Here, we must use "/4", since Src[] is in [-256, 255] */
  SKL_INT16 a = (3*Src1[0]+  Src2[0] + 2) / 4;
  SKL_INT16 b = (  Src1[0]+3*Src2[0] + 2) / 4;
  ADD(Dst1[0], a);
  ADD(Dst2[0], b);
}

static inline void Filter_Add_9331(SKL_BYTE *Dst1, SKL_BYTE *Dst2,
                                   const SKL_INT16 *Src1, const SKL_INT16 *Src2)
{
  SKL_INT16 a = (9*Src1[0]+  3*Src1[1]+ 3*Src2[0] + 1*Src2[1] + 8) / 16;
  SKL_INT16 b = (3*Src1[0]+  9*Src1[1]+ 1*Src2[0] + 3*Src2[1] + 8) / 16;
  SKL_INT16 c = (3*Src1[0]+  1*Src1[1]+ 9*Src2[0] + 3*Src2[1] + 8) / 16;
  SKL_INT16 d = (1*Src1[0]+  3*Src1[1]+ 3*Src2[0] + 9*Src2[1] + 8) / 16;
  ADD(Dst1[0], a);
  ADD(Dst1[1], b);
  ADD(Dst2[0], c);
  ADD(Dst2[1], d);
}

void Skl_Add_Upsampled_8x8_16To8_C(SKL_BYTE *Dst, const SKL_INT16 *Src, const int BpS)
{
  int x, y;

  ADD(Dst[0], Src[0]);
  for(x=0; x<7; ++x) Filter_Add_31(Dst+2*x+1, Dst+2*x+2, Src+x, Src+x+1);
  ADD(Dst[15], Src[7]);
  Dst += BpS;
  for(y=0; y<7; ++y) {
    SKL_BYTE *const Dst2 = Dst + BpS;
    Filter_Add_31(Dst, Dst2, Src, Src+8);
    for(x=0; x<7; ++x)
      Filter_Add_9331(Dst+2*x+1, Dst2+2*x+1, Src+x, Src+x+8);
    Filter_Add_31(Dst+15, Dst2+15, Src+7, Src+7+8);
    Src += 8; 
    Dst += 2*BpS;
  }
  ADD(Dst[0], Src[0]);
  for(x=0; x<7; ++x) Filter_Add_31(Dst+2*x+1, Dst+2*x+2, Src+x, Src+x+1);
  ADD(Dst[15], Src[7]);
}

void Skl_HFilter_31_C(SKL_BYTE *Src1, SKL_BYTE *Src2, int Nb_Blks)
{
  Nb_Blks *= 8;
  while(Nb_Blks-->0) {
    SKL_BYTE a = ( 3*Src1[0] + 1*Src2[0] + 2 ) >> 2;
    SKL_BYTE b = ( 1*Src1[0] + 3*Src2[0] + 2 ) >> 2;
    *Src1++ = a;
    *Src2++ = b;
  }
}

void Skl_VFilter_31_C(SKL_BYTE *Src1, SKL_BYTE *Src2,
                      const int BpS, int Nb_Blks)
{
  Nb_Blks *= 8;
  while(Nb_Blks-->0) {
    SKL_BYTE a = ( 3*Src1[0] + 1*Src2[0] + 2 ) >> 2;
    SKL_BYTE b = ( 1*Src1[0] + 3*Src2[0] + 2 ) >> 2;
    *Src1 = a;
    *Src2 = b;
    Src1 += BpS;
    Src2 += BpS;
  }
}

//////////////////////////////////////////////////////////
// (3,1) downsampling

void Skl_Filter_18x18_To_8x8_C(SKL_INT16 *Dst, const SKL_BYTE *Src, const int BpS)
{
  SKL_INT16 *T, Tmp[18*8];
  int i, j;

  T = Tmp;
  Src -= BpS;
  for(j=-1; j<17; j++) {
    for(i=0; i<8; ++i)
      T[i] = Src[2*i-1] + 3*Src[2*i+0] + 3*Src[2*i+1] + Src[2*i+2];
    T += 8;
    Src += BpS;
  }
  T = Tmp + 8;
  for(j=0; j<8; j++) {
    for(i=0; i<8; ++i)
      Dst[i] = ( T[-8+i] + 3*T[0+i] + 3*T[8+i] + T[16+i] + 32 ) / 64;
    Dst += 8;
    T += 16;
  }
}

void Skl_Filter_Diff_18x18_To_8x8_C(SKL_INT16 *Dst, const SKL_BYTE *Src, const int BpS)
{
  SKL_INT16 *T, Tmp[18*8];
  int i, j;

  T = Tmp;
  Src -= BpS;
  for(j=-1; j<17; j++) {
    for(i=0; i<8; ++i)
      T[i] = Src[2*i-1] + 3*Src[2*i+0] + 3*Src[2*i+1] + Src[2*i+2];
    T += 8;
    Src += BpS;
  }
  T = Tmp;
  for(j=0; j<8; j++) {
    for(i=0; i<8; ++i)
      Dst[i] -= ( T[i] + 3*T[8+i] + 3*T[16+i] + T[24+i] + 32 ) / 64;
    Dst += 8;
    T += 16;
  }
}

//////////////////////////////////////////////////////////
// 8b to 16b transfer ops

void Skl_Copy_16x8_8To16_C(SKL_INT16 *Dst, const SKL_BYTE *Src, const int BpS)
{
  for(int y=8; y>0; --y) {
    Dst[0] = (SKL_INT16)Src[0]; Dst[64+0] = (SKL_INT16)Src[0+8];
    Dst[1] = (SKL_INT16)Src[1]; Dst[64+1] = (SKL_INT16)Src[1+8];
    Dst[2] = (SKL_INT16)Src[2]; Dst[64+2] = (SKL_INT16)Src[2+8];
    Dst[3] = (SKL_INT16)Src[3]; Dst[64+3] = (SKL_INT16)Src[3+8];
    Dst[4] = (SKL_INT16)Src[4]; Dst[64+4] = (SKL_INT16)Src[4+8];
    Dst[5] = (SKL_INT16)Src[5]; Dst[64+5] = (SKL_INT16)Src[5+8];
    Dst[6] = (SKL_INT16)Src[6]; Dst[64+6] = (SKL_INT16)Src[6+8];
    Dst[7] = (SKL_INT16)Src[7]; Dst[64+7] = (SKL_INT16)Src[7+8];
    Src += BpS; Dst += 8;
  }
}

void Skl_Copy_8x8_8To16_C(SKL_INT16 *Dst, const SKL_BYTE *Src, const int BpS)
{
  for(int y=8; y>0; --y) {
    Dst[0] = (SKL_INT16)Src[0];
    Dst[1] = (SKL_INT16)Src[1];
    Dst[2] = (SKL_INT16)Src[2];
    Dst[3] = (SKL_INT16)Src[3];
    Dst[4] = (SKL_INT16)Src[4];
    Dst[5] = (SKL_INT16)Src[5];
    Dst[6] = (SKL_INT16)Src[6];
    Dst[7] = (SKL_INT16)Src[7];
    Src += BpS; Dst += 8;
  }
}
void Skl_Diff_16x8_8To16_C(SKL_INT16 *Dst, const SKL_BYTE *Src, const int BpS)
{
  for(int y=8; y>0; --y) {
    Dst[0] -= (SKL_INT16)Src[0]; Dst[64+0] -= (SKL_INT16)Src[0+8];
    Dst[1] -= (SKL_INT16)Src[1]; Dst[64+1] -= (SKL_INT16)Src[1+8];
    Dst[2] -= (SKL_INT16)Src[2]; Dst[64+2] -= (SKL_INT16)Src[2+8];
    Dst[3] -= (SKL_INT16)Src[3]; Dst[64+3] -= (SKL_INT16)Src[3+8];
    Dst[4] -= (SKL_INT16)Src[4]; Dst[64+4] -= (SKL_INT16)Src[4+8];
    Dst[5] -= (SKL_INT16)Src[5]; Dst[64+5] -= (SKL_INT16)Src[5+8];
    Dst[6] -= (SKL_INT16)Src[6]; Dst[64+6] -= (SKL_INT16)Src[6+8];
    Dst[7] -= (SKL_INT16)Src[7]; Dst[64+7] -= (SKL_INT16)Src[7+8];
    Src += BpS; Dst += 8;
  }
}

void Skl_Diff_8x8_8To16_C(SKL_INT16 *Dst, const SKL_BYTE *Src, const int BpS)
{
  for(int y=8; y>0; --y) {
    Dst[0] -= (SKL_INT16)Src[0];
    Dst[1] -= (SKL_INT16)Src[1];
    Dst[2] -= (SKL_INT16)Src[2];
    Dst[3] -= (SKL_INT16)Src[3];
    Dst[4] -= (SKL_INT16)Src[4];
    Dst[5] -= (SKL_INT16)Src[5];
    Dst[6] -= (SKL_INT16)Src[6];
    Dst[7] -= (SKL_INT16)Src[7];
    Src += BpS; Dst += 8;
  }
}

void Skl_Diff_16x8_88To16_C(SKL_INT16 *Dst,
                            const SKL_BYTE *Src1, const SKL_BYTE *Src2, 
                            const int BpS)
{
  for(int y=8; y>0; --y) {
    Dst[   0] = (SKL_INT16)Src1[0]  -(SKL_INT16)Src2[0];
    Dst[   1] = (SKL_INT16)Src1[1]  -(SKL_INT16)Src2[1];
    Dst[   2] = (SKL_INT16)Src1[2]  -(SKL_INT16)Src2[2];
    Dst[   3] = (SKL_INT16)Src1[3]  -(SKL_INT16)Src2[3];
    Dst[   4] = (SKL_INT16)Src1[4]  -(SKL_INT16)Src2[4];
    Dst[   5] = (SKL_INT16)Src1[5]  -(SKL_INT16)Src2[5];
    Dst[   6] = (SKL_INT16)Src1[6]  -(SKL_INT16)Src2[6];
    Dst[   7] = (SKL_INT16)Src1[7]  -(SKL_INT16)Src2[7];
    Dst[64+0] = (SKL_INT16)Src1[8+0]  -(SKL_INT16)Src2[8+0];
    Dst[64+1] = (SKL_INT16)Src1[8+1]  -(SKL_INT16)Src2[8+1];
    Dst[64+2] = (SKL_INT16)Src1[8+2]  -(SKL_INT16)Src2[8+2];
    Dst[64+3] = (SKL_INT16)Src1[8+3]  -(SKL_INT16)Src2[8+3];
    Dst[64+4] = (SKL_INT16)Src1[8+4]  -(SKL_INT16)Src2[8+4];
    Dst[64+5] = (SKL_INT16)Src1[8+5]  -(SKL_INT16)Src2[8+5];
    Dst[64+6] = (SKL_INT16)Src1[8+6]  -(SKL_INT16)Src2[8+6];
    Dst[64+7] = (SKL_INT16)Src1[8+7]  -(SKL_INT16)Src2[8+7];
    Src1 += BpS; Src2 += BpS; Dst += 8;
  }
}

void Skl_Diff_8x8_88To16_C(SKL_INT16 *Dst,
                           const SKL_BYTE *Src1, const SKL_BYTE *Src2,
                           const int BpS)
{
  for(int y=8; y>0; --y) {
    Dst[   0] = (SKL_INT16)Src1[0]  -(SKL_INT16)Src2[0];
    Dst[   1] = (SKL_INT16)Src1[1]  -(SKL_INT16)Src2[1];
    Dst[   2] = (SKL_INT16)Src1[2]  -(SKL_INT16)Src2[2];
    Dst[   3] = (SKL_INT16)Src1[3]  -(SKL_INT16)Src2[3];
    Dst[   4] = (SKL_INT16)Src1[4]  -(SKL_INT16)Src2[4];
    Dst[   5] = (SKL_INT16)Src1[5]  -(SKL_INT16)Src2[5];
    Dst[   6] = (SKL_INT16)Src1[6]  -(SKL_INT16)Src2[6];
    Dst[   7] = (SKL_INT16)Src1[7]  -(SKL_INT16)Src2[7];
    Src1 += BpS; Src2 += BpS; Dst += 8;
  }
}

// 8b to 8b transfer ops

void Skl_Move_16x8_C(SKL_BYTE *Dst, const SKL_BYTE *Src, const int BpS)
{
  for(int y=8; y>0; --y) {
    memcpy( Dst, Src, 16 );
    Dst += BpS;
    Src += BpS;
  }
}
void Skl_Move_8x8_C(SKL_BYTE *Dst, const SKL_BYTE *Src, const int BpS)
{
  for(int y=8; y>0; --y) {
    memcpy( Dst, Src, 8 );
    Dst += BpS;
    Src += BpS;
  }
}

#undef COPY
#undef ADD
#undef MIX
#undef PRELUDE
#undef EPILOG

//////////////////////////////////////////////////////////
// Edge replication

void Skl_Make_Edges_C(SKL_BYTE * const * const YUV,
                      const int Width, const int Height, const int BpS)
{
  int k;

  SKL_BYTE *sY = YUV[0];
  SKL_BYTE *sU = YUV[1];
  SKL_BYTE *sV = YUV[2];
  for(k=Height/2; k>0; --k) {
    memset(sY   -16,     sY[0          ], 16);
    memset(sY+Width,     sY[Width-1    ], 16);
    memset(sY   -16+BpS, sY[0      +BpS], 16);
    memset(sY+Width+BpS, sY[Width-1+BpS], 16);
    sY += 2*BpS;

    memset(sU      -8, sU[0        ], 8);
    memset(sU+Width/2, sU[Width/2-1], 8);
    memset(sV      -8, sV[0        ], 8);
    memset(sV+Width/2, sV[Width/2-1], 8);
    sU += BpS;
    sV += BpS;
  }

  SKL_BYTE *dY_Top = YUV[0] - 16 - 16*BpS;
  SKL_BYTE *sY_Top = YUV[0] - 16;

  SKL_BYTE *dY_Bot = YUV[0] - 16 + Height*BpS;
  SKL_BYTE *sY_Bot = YUV[0] - 16 + Height*BpS - BpS;

  SKL_BYTE *dC_Top = YUV[1] - 8 - 8*BpS;
  SKL_BYTE *sC_Top = YUV[1] - 8;
  SKL_BYTE *dC_Bot = YUV[1] - 8 + (Height/2)*BpS;
  SKL_BYTE *sC_Bot = YUV[1] - 8 + (Height/2)*BpS - BpS;

  for(k=8;k>0;k--)
  {
    memcpy(dY_Top    , sY_Top, BpS);
    memcpy(dY_Top+BpS, sY_Top, BpS);
    memcpy(dC_Top    , sC_Top, BpS);
    memcpy(dY_Bot    , sY_Bot, BpS);
    memcpy(dY_Bot+BpS, sY_Bot, BpS);
    memcpy(dC_Bot    , sC_Bot, BpS);
    dY_Top += 2*BpS;
    dC_Top +=   BpS;
    dY_Bot += 2*BpS;
    dC_Bot +=   BpS;
  }
}

//////////////////////////////////////////////////////////
// Frame/Field ops (rather unused)
//////////////////////////////////////////////////////////

#define ABS(x) ((x)<0 ? -(x) : (x))
#define SAD(a,b)  \
    Tmp = Src[0*64+(a)*8+j+8] - Src[0*64+(b)*8+j+8]; Sum += ABS(Tmp); \
    Tmp = Src[1*64+(a)*8+j+8] - Src[1*64+(b)*8+j+8]; Sum += ABS(Tmp);
#define SAD2(a,b)  SAD((a),(b)); SAD((a)+16, (b)+16);

SKL_UINT32 Skl_SAD_16x7_Frame_C(const SKL_INT16 *Src)
{
  SKL_UINT32 Sum = 0;
  for(int j=-8; j<0; ++j) {
    SKL_INT32 Tmp;
    SAD2(0,1); SAD2(1,2); SAD2(2,3); SAD2(3,4);
    SAD2(4,5); SAD2(5,6); SAD2(6,7);
  }
  return Sum;
}

SKL_UINT32 Skl_SAD_16x7_Field_C(const SKL_INT16 *Src)
{
  SKL_UINT32 Sum = 0;
  SKL_INT32 Tmp;
  for(int j=-8; j<0; ++j) {
    SAD2(0,2); SAD2(2,4); 
    SAD2(2,4); SAD2(3,5);
    SAD2(4,6); SAD2(5,7);

    SAD(6,0+16); SAD(7,1+16);
  }
  return Sum;
}
#undef SAD2
#undef SAD
#undef ABS
 
void Skl_Reorder_Frame_16x16_C(SKL_INT16 *Src)  // Src[4*64]
{
    // the permutation: [01234567|89abcdef] -> [0248ace|13579bdf]
    // decomposes into the 4 elementary cycles:
    //   1->2->4->8 ->1      3->6->c->9 ->3
    //   7->e->d->b ->7      5->10      ->5
    //  + the 2 fixed point 0/f, of course.

  SKL_INT16 Line[8];
#define CIRCULATE(a,b,c,d)                   \
  memcpy(Line, Src+(a)*8, sizeof(Line));     \
  memcpy(Src+(a)*8, Src+(b)*8, sizeof(Line));\
  memcpy(Src+(b)*8, Src+(c)*8, sizeof(Line));\
  memcpy(Src+(c)*8, Src+(d)*8, sizeof(Line));\
  memcpy(Src+(d)*8, Line, sizeof(Line))

#define SWAP(a,b)                            \
  memcpy(Line, Src+(a)*8, sizeof(Line));     \
  memcpy(Src+(a)*8, Src+(b)*8, sizeof(Line));\
  memcpy(Src+(b)*8, Line, sizeof(Line))

#define BLK 8   // jump to next block below

  CIRCULATE(1,2,4,8 +BLK);
  CIRCULATE(3,6,12 +BLK,9 +BLK);
  CIRCULATE(7,14 +BLK,13 +BLK, 11 +BLK);
  SWAP(5,10 +BLK);

  Src += 64;
  CIRCULATE(1,2,4,8 +BLK);
  CIRCULATE(3,6,12 +BLK,9 +BLK);
  CIRCULATE(7,14 +BLK,13 +BLK, 11 +BLK);
  SWAP(5,10 +BLK);

#undef BLK
#undef SWAP
#undef CIRCULATE
}

//////////////////////////////////////////////////////////

}   // extern "C"
