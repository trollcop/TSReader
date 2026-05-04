/********************************************************
 * Some code. Copyright (C) 2003 by Pascal Massimino.   *
 * All Rights Reserved.      (http://skal.planet-d.net) *
 * For Educational/Academic use ONLY. See 'LICENSE.TXT'.*
 ********************************************************/
/*
 *  skl_mb_qpel_c.cpp
 *
 *  Quarter-pixel interpolation
 *
 ********************************************************/

#ifndef SKL_AUTO_INCLUDE

#include "skl.h"
#include "skl_syst/skl_dsp.h"

extern "C" {

//////////////////////////////////////////////////////////
//
//  Quarter-pixel mess starts here. This is as tedious
//  as you can imagine, but copy-paste rules! :)
//
//////////////////////////////////////////////////////////

#define SKL_AUTO_INCLUDE

  // 16x? filters

#define SIZE  16
#define TABLE FIR_Tab_16
#define STORE(d,s)  (d) = (s)

#define RND 1
#define FUNC_H      Skl_H_Pass_16_Copy_Rnd1_C
#define FUNC_V      Skl_V_Pass_16_Copy_Rnd1_C
#define FUNC_HA     Skl_H_Pass_Avrg_16_Copy_Rnd1_C
#define FUNC_VA     Skl_V_Pass_Avrg_16_Copy_Rnd1_C
#define FUNC_HA_UP  Skl_H_Pass_Avrg_Up_16_Copy_Rnd1_C
#define FUNC_VA_UP  Skl_V_Pass_Avrg_Up_16_Copy_Rnd1_C

#include "./skl_mb_qpel_c.cpp"   /* self-include ourself */

#define RND 0
#define FUNC_H      Skl_H_Pass_16_Copy_Rnd0_C
#define FUNC_V      Skl_V_Pass_16_Copy_Rnd0_C
#define FUNC_HA     Skl_H_Pass_Avrg_16_Copy_Rnd0_C
#define FUNC_VA     Skl_V_Pass_Avrg_16_Copy_Rnd0_C
#define FUNC_HA_UP  Skl_H_Pass_Avrg_Up_16_Copy_Rnd0_C
#define FUNC_VA_UP  Skl_V_Pass_Avrg_Up_16_Copy_Rnd0_C

#include "./skl_mb_qpel_c.cpp"
#undef STORE

  // note: Rounding is always 0 for B-frame dst mixing
#define STORE(d,s)  (d) = ( (s)+(d)+1 ) >> 1

#define RND 0
#define FUNC_H      Skl_H_Pass_16_Add_Rnd0_C
#define FUNC_V      Skl_V_Pass_16_Add_Rnd0_C
#define FUNC_HA     Skl_H_Pass_Avrg_16_Add_Rnd0_C
#define FUNC_VA     Skl_V_Pass_Avrg_16_Add_Rnd0_C
#define FUNC_HA_UP  Skl_H_Pass_Avrg_Up_16_Add_Rnd0_C
#define FUNC_VA_UP  Skl_V_Pass_Avrg_Up_16_Add_Rnd0_C

#include "./skl_mb_qpel_c.cpp"
#undef STORE


#undef SIZE
#undef TABLE

  // 8x? filters

#define SIZE  8
#define TABLE FIR_Tab_8
#define STORE(d,s)  (d) = (s)

#define RND 1
#define FUNC_H      Skl_H_Pass_8_Copy_Rnd1_C
#define FUNC_V      Skl_V_Pass_8_Copy_Rnd1_C
#define FUNC_HA     Skl_H_Pass_Avrg_8_Copy_Rnd1_C
#define FUNC_VA     Skl_V_Pass_Avrg_8_Copy_Rnd1_C
#define FUNC_HA_UP  Skl_H_Pass_Avrg_Up_8_Copy_Rnd1_C
#define FUNC_VA_UP  Skl_V_Pass_Avrg_Up_8_Copy_Rnd1_C

#include "./skl_mb_qpel_c.cpp"

#define RND 0
#define FUNC_H      Skl_H_Pass_8_Copy_Rnd0_C
#define FUNC_V      Skl_V_Pass_8_Copy_Rnd0_C
#define FUNC_HA     Skl_H_Pass_Avrg_8_Copy_Rnd0_C
#define FUNC_VA     Skl_V_Pass_Avrg_8_Copy_Rnd0_C
#define FUNC_HA_UP  Skl_H_Pass_Avrg_Up_8_Copy_Rnd0_C
#define FUNC_VA_UP  Skl_V_Pass_Avrg_Up_8_Copy_Rnd0_C

#include "./skl_mb_qpel_c.cpp"
#undef STORE

  // note: Rounding is always 0 for B-frame dst mixing
#define STORE(d,s)  (d) = ( (s)+(d)+1 ) >> 1

#define RND 0
#define FUNC_H      Skl_H_Pass_8_Add_Rnd0_C
#define FUNC_V      Skl_V_Pass_8_Add_Rnd0_C
#define FUNC_HA     Skl_H_Pass_Avrg_8_Add_Rnd0_C
#define FUNC_VA     Skl_V_Pass_Avrg_8_Add_Rnd0_C
#define FUNC_HA_UP  Skl_H_Pass_Avrg_Up_8_Add_Rnd0_C
#define FUNC_VA_UP  Skl_V_Pass_Avrg_Up_8_Add_Rnd0_C

#include "./skl_mb_qpel_c.cpp"
#undef STORE

#undef SIZE
#undef TABLE

#undef SKL_AUTO_INCLUDE

}   // extern "C"

#endif /* !SKL_AUTO_INCLUDE */

//////////////////////////////////////////////////////////
// horizontal passes
//////////////////////////////////////////////////////////

#ifdef SKL_AUTO_INCLUDE

#define CLIP_STORE(D,C) \
  if (C<0) C = 0; else if (C>(255<<5)) C = 255; else C = C>>5;  \
  STORE(D, C)

void FUNC_H(SKL_BYTE *Dst, const SKL_BYTE *Src, int H, int BpS)
{
#if (SIZE==16)
  while(H-->0) {
    int C;
    C = 16-RND +14*Src[0] +23*Src[1] - 7*Src[2] + 3*Src[3] -   Src[4];
    CLIP_STORE(Dst[ 0],C);
    C = 16-RND - 3*(Src[0]-Src[4]) +19*Src[1] +20*Src[2] - 6*Src[3] - Src[5];
    CLIP_STORE(Dst[ 1],C);
    C = 16-RND + 2*Src[0] - 6*(Src[1]+Src[4]) +20*(Src[2]+Src[3]) + 3*Src[5] - Src[6];
    CLIP_STORE(Dst[ 2],C);
    C = 16-RND - (Src[0]+Src[7 ]) + 3*(Src[ 1]+Src[ 6])-6*(Src[ 2]+Src[ 5]) + 20*(Src[ 3]+Src[ 4]);
    CLIP_STORE(Dst[ 3],C);
    C = 16-RND - (Src[1]+Src[8 ]) + 3*(Src[ 2]+Src[ 7])-6*(Src[ 3]+Src[ 6]) + 20*(Src[ 4]+Src[ 5]);
    CLIP_STORE(Dst[ 4],C);
    C = 16-RND - (Src[2]+Src[9 ]) + 3*(Src[ 3]+Src[ 8])-6*(Src[ 4]+Src[ 7]) + 20*(Src[ 5]+Src[ 6]);
    CLIP_STORE(Dst[ 5],C);
    C = 16-RND - (Src[3]+Src[10]) + 3*(Src[ 4]+Src[ 9])-6*(Src[ 5]+Src[ 8]) + 20*(Src[ 6]+Src[ 7]);
    CLIP_STORE(Dst[ 6],C);
    C = 16-RND - (Src[4]+Src[11]) + 3*(Src[ 5]+Src[10])-6*(Src[ 6]+Src[ 9]) + 20*(Src[ 7]+Src[ 8]);
    CLIP_STORE(Dst[ 7],C);
    C = 16-RND - (Src[5]+Src[12]) + 3*(Src[ 6]+Src[11])-6*(Src[ 7]+Src[10]) + 20*(Src[ 8]+Src[ 9]);
    CLIP_STORE(Dst[ 8],C);
    C = 16-RND - (Src[6]+Src[13]) + 3*(Src[ 7]+Src[12])-6*(Src[ 8]+Src[11]) + 20*(Src[ 9]+Src[10]);
    CLIP_STORE(Dst[ 9],C);
    C = 16-RND - (Src[7]+Src[14]) + 3*(Src[ 8]+Src[13])-6*(Src[ 9]+Src[12]) + 20*(Src[10]+Src[11]);
    CLIP_STORE(Dst[10],C);
    C = 16-RND - (Src[8]+Src[15]) + 3*(Src[ 9]+Src[14])-6*(Src[10]+Src[13]) + 20*(Src[11]+Src[12]);
    CLIP_STORE(Dst[11],C);
    C = 16-RND - (Src[9]+Src[16]) + 3*(Src[10]+Src[15])-6*(Src[11]+Src[14]) + 20*(Src[12]+Src[13]);
    CLIP_STORE(Dst[12],C);
    C = 16-RND - Src[10] +3*Src[11] -6*(Src[12]+Src[15]) + 20*(Src[13]+Src[14]) +2*Src[16];
    CLIP_STORE(Dst[13],C);
    C = 16-RND - Src[11] +3*(Src[12]-Src[16]) -6*Src[13] + 20*Src[14] + 19*Src[15];
    CLIP_STORE(Dst[14],C);
    C = 16-RND - Src[12] +3*Src[13] -7*Src[14] + 23*Src[15] + 14*Src[16];
    CLIP_STORE(Dst[15],C);
    Src += BpS;
    Dst += BpS;
  }
#else
  while(H-->0) {
    int C;
    C = 16-RND +14*Src[0] +23*Src[1] - 7*Src[2] + 3*Src[3] -   Src[4];
    CLIP_STORE(Dst[0],C);
    C = 16-RND - 3*(Src[0]-Src[4]) +19*Src[1] +20*Src[2] - 6*Src[3] - Src[5];
    CLIP_STORE(Dst[1],C);
    C = 16-RND + 2*Src[0] - 6*(Src[1]+Src[4]) +20*(Src[2]+Src[3]) + 3*Src[5] - Src[6];
    CLIP_STORE(Dst[2],C);
    C = 16-RND - (Src[0]+Src[7]) + 3*(Src[1]+Src[6])-6*(Src[2]+Src[5]) + 20*(Src[3]+Src[4]);
    CLIP_STORE(Dst[3],C);
    C = 16-RND - (Src[1]+Src[8]) + 3*(Src[2]+Src[7])-6*(Src[3]+Src[6]) + 20*(Src[4]+Src[5]);
    CLIP_STORE(Dst[4],C);
    C = 16-RND - Src[2] +3*Src[3] -6*(Src[4]+Src[7]) + 20*(Src[5]+Src[6]) +2*Src[8];
    CLIP_STORE(Dst[5],C);
    C = 16-RND - Src[3] +3*(Src[4]-Src[8]) -6*Src[5] + 20*Src[6] + 19*Src[7];
    CLIP_STORE(Dst[6],C);
    C = 16-RND - Src[4] +3*Src[5] -7*Src[6] + 23*Src[7] + 14*Src[8];
    CLIP_STORE(Dst[7],C);
    Src += BpS;
    Dst += BpS;
  }
#endif
}
#undef CLIP_STORE

#define CLIP_STORE(i,C) \
  if (C<0) C = 0; else if (C>(255<<5)) C = 255; else C = C>>5;  \
  C = (C+Src[i]+1-RND) >> 1;  \
  STORE(Dst[i], C)

void FUNC_HA(SKL_BYTE *Dst, const SKL_BYTE *Src, int H, int BpS)
{
#if (SIZE==16)
  while(H-->0) {
    int C;
    C = 16-RND +14*Src[0] +23*Src[1] - 7*Src[2] + 3*Src[3] -   Src[4];
    CLIP_STORE(0,C);
    C = 16-RND - 3*(Src[0]-Src[4]) +19*Src[1] +20*Src[2] - 6*Src[3] - Src[5];
    CLIP_STORE( 1,C);
    C = 16-RND + 2*Src[0] - 6*(Src[1]+Src[4]) +20*(Src[2]+Src[3]) + 3*Src[5] - Src[6];
    CLIP_STORE( 2,C);
    C = 16-RND - (Src[0]+Src[7 ]) + 3*(Src[ 1]+Src[ 6])-6*(Src[ 2]+Src[ 5]) + 20*(Src[ 3]+Src[ 4]);
    CLIP_STORE( 3,C);
    C = 16-RND - (Src[1]+Src[8 ]) + 3*(Src[ 2]+Src[ 7])-6*(Src[ 3]+Src[ 6]) + 20*(Src[ 4]+Src[ 5]);
    CLIP_STORE( 4,C);
    C = 16-RND - (Src[2]+Src[9 ]) + 3*(Src[ 3]+Src[ 8])-6*(Src[ 4]+Src[ 7]) + 20*(Src[ 5]+Src[ 6]);
    CLIP_STORE( 5,C);
    C = 16-RND - (Src[3]+Src[10]) + 3*(Src[ 4]+Src[ 9])-6*(Src[ 5]+Src[ 8]) + 20*(Src[ 6]+Src[ 7]);
    CLIP_STORE( 6,C);
    C = 16-RND - (Src[4]+Src[11]) + 3*(Src[ 5]+Src[10])-6*(Src[ 6]+Src[ 9]) + 20*(Src[ 7]+Src[ 8]);
    CLIP_STORE( 7,C);
    C = 16-RND - (Src[5]+Src[12]) + 3*(Src[ 6]+Src[11])-6*(Src[ 7]+Src[10]) + 20*(Src[ 8]+Src[ 9]);
    CLIP_STORE( 8,C);
    C = 16-RND - (Src[6]+Src[13]) + 3*(Src[ 7]+Src[12])-6*(Src[ 8]+Src[11]) + 20*(Src[ 9]+Src[10]);
    CLIP_STORE( 9,C);
    C = 16-RND - (Src[7]+Src[14]) + 3*(Src[ 8]+Src[13])-6*(Src[ 9]+Src[12]) + 20*(Src[10]+Src[11]);
    CLIP_STORE(10,C);
    C = 16-RND - (Src[8]+Src[15]) + 3*(Src[ 9]+Src[14])-6*(Src[10]+Src[13]) + 20*(Src[11]+Src[12]);
    CLIP_STORE(11,C);
    C = 16-RND - (Src[9]+Src[16]) + 3*(Src[10]+Src[15])-6*(Src[11]+Src[14]) + 20*(Src[12]+Src[13]);
    CLIP_STORE(12,C);
    C = 16-RND - Src[10] +3*Src[11] -6*(Src[12]+Src[15]) + 20*(Src[13]+Src[14]) +2*Src[16];
    CLIP_STORE(13,C);
    C = 16-RND - Src[11] +3*(Src[12]-Src[16]) -6*Src[13] + 20*Src[14] + 19*Src[15];
    CLIP_STORE(14,C);
    C = 16-RND - Src[12] +3*Src[13] -7*Src[14] + 23*Src[15] + 14*Src[16];
    CLIP_STORE(15,C);
    Src += BpS;
    Dst += BpS;
  }
#else
  while(H-->0) {
    int C;
    C = 16-RND +14*Src[0] +23*Src[1] - 7*Src[2] + 3*Src[3] -   Src[4];
    CLIP_STORE(0,C);
    C = 16-RND - 3*(Src[0]-Src[4]) +19*Src[1] +20*Src[2] - 6*Src[3] - Src[5];
    CLIP_STORE(1,C);
    C = 16-RND + 2*Src[0] - 6*(Src[1]+Src[4]) +20*(Src[2]+Src[3]) + 3*Src[5] - Src[6];
    CLIP_STORE(2,C);
    C = 16-RND - (Src[0]+Src[7]) + 3*(Src[1]+Src[6])-6*(Src[2]+Src[5]) + 20*(Src[3]+Src[4]);
    CLIP_STORE(3,C);
    C = 16-RND - (Src[1]+Src[8]) + 3*(Src[2]+Src[7])-6*(Src[3]+Src[6]) + 20*(Src[4]+Src[5]);
    CLIP_STORE(4,C);
    C = 16-RND - Src[2] +3*Src[3] -6*(Src[4]+Src[7]) + 20*(Src[5]+Src[6]) +2*Src[8];
    CLIP_STORE(5,C);
    C = 16-RND - Src[3] +3*(Src[4]-Src[8]) -6*Src[5] + 20*Src[6] + 19*Src[7];
    CLIP_STORE(6,C);
    C = 16-RND - Src[4] +3*Src[5] -7*Src[6] + 23*Src[7] + 14*Src[8];
    CLIP_STORE(7,C);
    Src += BpS;
    Dst += BpS;
  }
#endif
}
#undef CLIP_STORE

#define CLIP_STORE(i,C) \
  if (C<0) C = 0; else if (C>(255<<5)) C = 255; else C = C>>5;  \
  C = (C+Src[i+1]+1-RND) >> 1;  \
  STORE(Dst[i], C)

void FUNC_HA_UP(SKL_BYTE *Dst, const SKL_BYTE *Src, int H, int BpS)
{
#if (SIZE==16)
  while(H-->0) {
    int C;
    C = 16-RND +14*Src[0] +23*Src[1] - 7*Src[2] + 3*Src[3] -   Src[4];
    CLIP_STORE(0,C);
    C = 16-RND - 3*(Src[0]-Src[4]) +19*Src[1] +20*Src[2] - 6*Src[3] - Src[5];
    CLIP_STORE( 1,C);
    C = 16-RND + 2*Src[0] - 6*(Src[1]+Src[4]) +20*(Src[2]+Src[3]) + 3*Src[5] - Src[6];
    CLIP_STORE( 2,C);
    C = 16-RND - (Src[0]+Src[7 ]) + 3*(Src[ 1]+Src[ 6])-6*(Src[ 2]+Src[ 5]) + 20*(Src[ 3]+Src[ 4]);
    CLIP_STORE( 3,C);
    C = 16-RND - (Src[1]+Src[8 ]) + 3*(Src[ 2]+Src[ 7])-6*(Src[ 3]+Src[ 6]) + 20*(Src[ 4]+Src[ 5]);
    CLIP_STORE( 4,C);
    C = 16-RND - (Src[2]+Src[9 ]) + 3*(Src[ 3]+Src[ 8])-6*(Src[ 4]+Src[ 7]) + 20*(Src[ 5]+Src[ 6]);
    CLIP_STORE( 5,C);
    C = 16-RND - (Src[3]+Src[10]) + 3*(Src[ 4]+Src[ 9])-6*(Src[ 5]+Src[ 8]) + 20*(Src[ 6]+Src[ 7]);
    CLIP_STORE( 6,C);
    C = 16-RND - (Src[4]+Src[11]) + 3*(Src[ 5]+Src[10])-6*(Src[ 6]+Src[ 9]) + 20*(Src[ 7]+Src[ 8]);
    CLIP_STORE( 7,C);
    C = 16-RND - (Src[5]+Src[12]) + 3*(Src[ 6]+Src[11])-6*(Src[ 7]+Src[10]) + 20*(Src[ 8]+Src[ 9]);
    CLIP_STORE( 8,C);
    C = 16-RND - (Src[6]+Src[13]) + 3*(Src[ 7]+Src[12])-6*(Src[ 8]+Src[11]) + 20*(Src[ 9]+Src[10]);
    CLIP_STORE( 9,C);
    C = 16-RND - (Src[7]+Src[14]) + 3*(Src[ 8]+Src[13])-6*(Src[ 9]+Src[12]) + 20*(Src[10]+Src[11]);
    CLIP_STORE(10,C);
    C = 16-RND - (Src[8]+Src[15]) + 3*(Src[ 9]+Src[14])-6*(Src[10]+Src[13]) + 20*(Src[11]+Src[12]);
    CLIP_STORE(11,C);
    C = 16-RND - (Src[9]+Src[16]) + 3*(Src[10]+Src[15])-6*(Src[11]+Src[14]) + 20*(Src[12]+Src[13]);
    CLIP_STORE(12,C);
    C = 16-RND - Src[10] +3*Src[11] -6*(Src[12]+Src[15]) + 20*(Src[13]+Src[14]) +2*Src[16];
    CLIP_STORE(13,C);
    C = 16-RND - Src[11] +3*(Src[12]-Src[16]) -6*Src[13] + 20*Src[14] + 19*Src[15];
    CLIP_STORE(14,C);
    C = 16-RND - Src[12] +3*Src[13] -7*Src[14] + 23*Src[15] + 14*Src[16];
    CLIP_STORE(15,C);
    Src += BpS;
    Dst += BpS;
  }
#else
  while(H-->0) {
    int C;
    C = 16-RND +14*Src[0] +23*Src[1] - 7*Src[2] + 3*Src[3] -   Src[4];
    CLIP_STORE(0,C);
    C = 16-RND - 3*(Src[0]-Src[4]) +19*Src[1] +20*Src[2] - 6*Src[3] - Src[5];
    CLIP_STORE(1,C);
    C = 16-RND + 2*Src[0] - 6*(Src[1]+Src[4]) +20*(Src[2]+Src[3]) + 3*Src[5] - Src[6];
    CLIP_STORE(2,C);
    C = 16-RND - (Src[0]+Src[7]) + 3*(Src[1]+Src[6])-6*(Src[2]+Src[5]) + 20*(Src[3]+Src[4]);
    CLIP_STORE(3,C);
    C = 16-RND - (Src[1]+Src[8]) + 3*(Src[2]+Src[7])-6*(Src[3]+Src[6]) + 20*(Src[4]+Src[5]);
    CLIP_STORE(4,C);
    C = 16-RND - Src[2] +3*Src[3] -6*(Src[4]+Src[7]) + 20*(Src[5]+Src[6]) +2*Src[8];
    CLIP_STORE(5,C);
    C = 16-RND - Src[3] +3*(Src[4]-Src[8]) -6*Src[5] + 20*Src[6] + 19*Src[7];
    CLIP_STORE(6,C);
    C = 16-RND - Src[4] +3*Src[5] -7*Src[6] + 23*Src[7] + 14*Src[8];
    CLIP_STORE(7,C);
    Src += BpS;
    Dst += BpS;
  }
#endif
}
#undef CLIP_STORE

//////////////////////////////////////////////////////////
// vertical passes
//////////////////////////////////////////////////////////
// Note: for vertical passes, width (W) needs only be 8 or 16.

#define CLIP_STORE(D,C) \
  if (C<0) C = 0; else if (C>(255<<5)) C = 255; else C = C>>5;  \
  STORE(D, C)

void FUNC_V(SKL_BYTE *Dst, const SKL_BYTE *Src, int H, int BpS)
{
#if (SIZE==16)
  while(H-->0) {
    int C;
    C = 16-RND +14*Src[BpS*0] +23*Src[BpS*1] - 7*Src[BpS*2] + 3*Src[BpS*3] -   Src[BpS*4];
    CLIP_STORE(Dst[BpS* 0],C);
    C = 16-RND - 3*(Src[BpS*0]-Src[BpS*4]) +19*Src[BpS*1] +20*Src[BpS*2] - 6*Src[BpS*3] - Src[BpS*5];
    CLIP_STORE(Dst[BpS* 1],C);
    C = 16-RND + 2*Src[BpS*0] - 6*(Src[BpS*1]+Src[BpS*4]) +20*(Src[BpS*2]+Src[BpS*3]) + 3*Src[BpS*5] - Src[BpS*6];
    CLIP_STORE(Dst[BpS* 2],C);
    C = 16-RND - (Src[BpS*0]+Src[BpS*7 ]) + 3*(Src[BpS* 1]+Src[BpS* 6])-6*(Src[BpS* 2]+Src[BpS* 5]) + 20*(Src[BpS* 3]+Src[BpS* 4]);
    CLIP_STORE(Dst[BpS* 3],C);
    C = 16-RND - (Src[BpS*1]+Src[BpS*8 ]) + 3*(Src[BpS* 2]+Src[BpS* 7])-6*(Src[BpS* 3]+Src[BpS* 6]) + 20*(Src[BpS* 4]+Src[BpS* 5]);
    CLIP_STORE(Dst[BpS* 4],C);
    C = 16-RND - (Src[BpS*2]+Src[BpS*9 ]) + 3*(Src[BpS* 3]+Src[BpS* 8])-6*(Src[BpS* 4]+Src[BpS* 7]) + 20*(Src[BpS* 5]+Src[BpS* 6]);
    CLIP_STORE(Dst[BpS* 5],C);
    C = 16-RND - (Src[BpS*3]+Src[BpS*10]) + 3*(Src[BpS* 4]+Src[BpS* 9])-6*(Src[BpS* 5]+Src[BpS* 8]) + 20*(Src[BpS* 6]+Src[BpS* 7]);
    CLIP_STORE(Dst[BpS* 6],C);
    C = 16-RND - (Src[BpS*4]+Src[BpS*11]) + 3*(Src[BpS* 5]+Src[BpS*10])-6*(Src[BpS* 6]+Src[BpS* 9]) + 20*(Src[BpS* 7]+Src[BpS* 8]);
    CLIP_STORE(Dst[BpS* 7],C);
    C = 16-RND - (Src[BpS*5]+Src[BpS*12]) + 3*(Src[BpS* 6]+Src[BpS*11])-6*(Src[BpS* 7]+Src[BpS*10]) + 20*(Src[BpS* 8]+Src[BpS* 9]);
    CLIP_STORE(Dst[BpS* 8],C);
    C = 16-RND - (Src[BpS*6]+Src[BpS*13]) + 3*(Src[BpS* 7]+Src[BpS*12])-6*(Src[BpS* 8]+Src[BpS*11]) + 20*(Src[BpS* 9]+Src[BpS*10]);
    CLIP_STORE(Dst[BpS* 9],C);
    C = 16-RND - (Src[BpS*7]+Src[BpS*14]) + 3*(Src[BpS* 8]+Src[BpS*13])-6*(Src[BpS* 9]+Src[BpS*12]) + 20*(Src[BpS*10]+Src[BpS*11]);
    CLIP_STORE(Dst[BpS*10],C);
    C = 16-RND - (Src[BpS*8]+Src[BpS*15]) + 3*(Src[BpS* 9]+Src[BpS*14])-6*(Src[BpS*10]+Src[BpS*13]) + 20*(Src[BpS*11]+Src[BpS*12]);
    CLIP_STORE(Dst[BpS*11],C);
    C = 16-RND - (Src[BpS*9]+Src[BpS*16]) + 3*(Src[BpS*10]+Src[BpS*15])-6*(Src[BpS*11]+Src[BpS*14]) + 20*(Src[BpS*12]+Src[BpS*13]);
    CLIP_STORE(Dst[BpS*12],C);
    C = 16-RND - Src[BpS*10] +3*Src[BpS*11] -6*(Src[BpS*12]+Src[BpS*15]) + 20*(Src[BpS*13]+Src[BpS*14]) +2*Src[BpS*16];
    CLIP_STORE(Dst[BpS*13],C);
    C = 16-RND - Src[BpS*11] +3*(Src[BpS*12]-Src[BpS*16]) -6*Src[BpS*13] + 20*Src[BpS*14] + 19*Src[BpS*15];
    CLIP_STORE(Dst[BpS*14],C);
    C = 16-RND - Src[BpS*12] +3*Src[BpS*13] -7*Src[BpS*14] + 23*Src[BpS*15] + 14*Src[BpS*16];
    CLIP_STORE(Dst[BpS*15],C);
    Src += 1;
    Dst += 1;
  }
#else
  while(H-->0) {
    int C;
    C = 16-RND +14*Src[BpS*0] +23*Src[BpS*1] - 7*Src[BpS*2] + 3*Src[BpS*3] -   Src[BpS*4];
    CLIP_STORE(Dst[BpS*0],C);
    C = 16-RND - 3*(Src[BpS*0]-Src[BpS*4]) +19*Src[BpS*1] +20*Src[BpS*2] - 6*Src[BpS*3] - Src[BpS*5];
    CLIP_STORE(Dst[BpS*1],C);
    C = 16-RND + 2*Src[BpS*0] - 6*(Src[BpS*1]+Src[BpS*4]) +20*(Src[BpS*2]+Src[BpS*3]) + 3*Src[BpS*5] - Src[BpS*6];
    CLIP_STORE(Dst[BpS*2],C);
    C = 16-RND - (Src[BpS*0]+Src[BpS*7]) + 3*(Src[BpS*1]+Src[BpS*6])-6*(Src[BpS*2]+Src[BpS*5]) + 20*(Src[BpS*3]+Src[BpS*4]);
    CLIP_STORE(Dst[BpS*3],C);
    C = 16-RND - (Src[BpS*1]+Src[BpS*8]) + 3*(Src[BpS*2]+Src[BpS*7])-6*(Src[BpS*3]+Src[BpS*6]) + 20*(Src[BpS*4]+Src[BpS*5]);
    CLIP_STORE(Dst[BpS*4],C);
    C = 16-RND - Src[BpS*2] +3*Src[BpS*3] -6*(Src[BpS*4]+Src[BpS*7]) + 20*(Src[BpS*5]+Src[BpS*6]) +2*Src[BpS*8];
    CLIP_STORE(Dst[BpS*5],C);
    C = 16-RND - Src[BpS*3] +3*(Src[BpS*4]-Src[BpS*8]) -6*Src[BpS*5] + 20*Src[BpS*6] + 19*Src[BpS*7];
    CLIP_STORE(Dst[BpS*6],C);
    C = 16-RND - Src[BpS*4] +3*Src[BpS*5] -7*Src[BpS*6] + 23*Src[BpS*7] + 14*Src[BpS*8];
    CLIP_STORE(Dst[BpS*7],C);
    Src += 1;
    Dst += 1;
  }
#endif
}
#undef CLIP_STORE

#define CLIP_STORE(i,C) \
  if (C<0) C = 0; else if (C>(255<<5)) C = 255; else C = C>>5;  \
  C = (C+Src[BpS*i]+1-RND) >> 1;  \
  STORE(Dst[BpS*i], C)

void FUNC_VA(SKL_BYTE *Dst, const SKL_BYTE *Src, int H, int BpS)
{
#if (SIZE==16)
  while(H-->0) {
    int C;
    C = 16-RND +14*Src[BpS*0] +23*Src[BpS*1] - 7*Src[BpS*2] + 3*Src[BpS*3] -   Src[BpS*4];
    CLIP_STORE(0,C);
    C = 16-RND - 3*(Src[BpS*0]-Src[BpS*4]) +19*Src[BpS*1] +20*Src[BpS*2] - 6*Src[BpS*3] - Src[BpS*5];
    CLIP_STORE( 1,C);
    C = 16-RND + 2*Src[BpS*0] - 6*(Src[BpS*1]+Src[BpS*4]) +20*(Src[BpS*2]+Src[BpS*3]) + 3*Src[BpS*5] - Src[BpS*6];
    CLIP_STORE( 2,C);
    C = 16-RND - (Src[BpS*0]+Src[BpS*7 ]) + 3*(Src[BpS* 1]+Src[BpS* 6])-6*(Src[BpS* 2]+Src[BpS* 5]) + 20*(Src[BpS* 3]+Src[BpS* 4]);
    CLIP_STORE( 3,C);
    C = 16-RND - (Src[BpS*1]+Src[BpS*8 ]) + 3*(Src[BpS* 2]+Src[BpS* 7])-6*(Src[BpS* 3]+Src[BpS* 6]) + 20*(Src[BpS* 4]+Src[BpS* 5]);
    CLIP_STORE( 4,C);
    C = 16-RND - (Src[BpS*2]+Src[BpS*9 ]) + 3*(Src[BpS* 3]+Src[BpS* 8])-6*(Src[BpS* 4]+Src[BpS* 7]) + 20*(Src[BpS* 5]+Src[BpS* 6]);
    CLIP_STORE( 5,C);
    C = 16-RND - (Src[BpS*3]+Src[BpS*10]) + 3*(Src[BpS* 4]+Src[BpS* 9])-6*(Src[BpS* 5]+Src[BpS* 8]) + 20*(Src[BpS* 6]+Src[BpS* 7]);
    CLIP_STORE( 6,C);
    C = 16-RND - (Src[BpS*4]+Src[BpS*11]) + 3*(Src[BpS* 5]+Src[BpS*10])-6*(Src[BpS* 6]+Src[BpS* 9]) + 20*(Src[BpS* 7]+Src[BpS* 8]);
    CLIP_STORE( 7,C);
    C = 16-RND - (Src[BpS*5]+Src[BpS*12]) + 3*(Src[BpS* 6]+Src[BpS*11])-6*(Src[BpS* 7]+Src[BpS*10]) + 20*(Src[BpS* 8]+Src[BpS* 9]);
    CLIP_STORE( 8,C);
    C = 16-RND - (Src[BpS*6]+Src[BpS*13]) + 3*(Src[BpS* 7]+Src[BpS*12])-6*(Src[BpS* 8]+Src[BpS*11]) + 20*(Src[BpS* 9]+Src[BpS*10]);
    CLIP_STORE( 9,C);
    C = 16-RND - (Src[BpS*7]+Src[BpS*14]) + 3*(Src[BpS* 8]+Src[BpS*13])-6*(Src[BpS* 9]+Src[BpS*12]) + 20*(Src[BpS*10]+Src[BpS*11]);
    CLIP_STORE(10,C);
    C = 16-RND - (Src[BpS*8]+Src[BpS*15]) + 3*(Src[BpS* 9]+Src[BpS*14])-6*(Src[BpS*10]+Src[BpS*13]) + 20*(Src[BpS*11]+Src[BpS*12]);
    CLIP_STORE(11,C);
    C = 16-RND - (Src[BpS*9]+Src[BpS*16]) + 3*(Src[BpS*10]+Src[BpS*15])-6*(Src[BpS*11]+Src[BpS*14]) + 20*(Src[BpS*12]+Src[BpS*13]);
    CLIP_STORE(12,C);
    C = 16-RND - Src[BpS*10] +3*Src[BpS*11] -6*(Src[BpS*12]+Src[BpS*15]) + 20*(Src[BpS*13]+Src[BpS*14]) +2*Src[BpS*16];
    CLIP_STORE(13,C);
    C = 16-RND - Src[BpS*11] +3*(Src[BpS*12]-Src[BpS*16]) -6*Src[BpS*13] + 20*Src[BpS*14] + 19*Src[BpS*15];
    CLIP_STORE(14,C);
    C = 16-RND - Src[BpS*12] +3*Src[BpS*13] -7*Src[BpS*14] + 23*Src[BpS*15] + 14*Src[BpS*16];
    CLIP_STORE(15,C);
    Src += 1;
    Dst += 1;
  }
#else
  while(H-->0) {
    int C;
    C = 16-RND +14*Src[BpS*0] +23*Src[BpS*1] - 7*Src[BpS*2] + 3*Src[BpS*3] -   Src[BpS*4];
    CLIP_STORE(0,C);
    C = 16-RND - 3*(Src[BpS*0]-Src[BpS*4]) +19*Src[BpS*1] +20*Src[BpS*2] - 6*Src[BpS*3] - Src[BpS*5];
    CLIP_STORE(1,C);
    C = 16-RND + 2*Src[BpS*0] - 6*(Src[BpS*1]+Src[BpS*4]) +20*(Src[BpS*2]+Src[BpS*3]) + 3*Src[BpS*5] - Src[BpS*6];
    CLIP_STORE(2,C);
    C = 16-RND - (Src[BpS*0]+Src[BpS*7]) + 3*(Src[BpS*1]+Src[BpS*6])-6*(Src[BpS*2]+Src[BpS*5]) + 20*(Src[BpS*3]+Src[BpS*4]);
    CLIP_STORE(3,C);
    C = 16-RND - (Src[BpS*1]+Src[BpS*8]) + 3*(Src[BpS*2]+Src[BpS*7])-6*(Src[BpS*3]+Src[BpS*6]) + 20*(Src[BpS*4]+Src[BpS*5]);
    CLIP_STORE(4,C);
    C = 16-RND - Src[BpS*2] +3*Src[BpS*3] -6*(Src[BpS*4]+Src[BpS*7]) + 20*(Src[BpS*5]+Src[BpS*6]) +2*Src[BpS*8];
    CLIP_STORE(5,C);
    C = 16-RND - Src[BpS*3] +3*(Src[BpS*4]-Src[BpS*8]) -6*Src[BpS*5] + 20*Src[BpS*6] + 19*Src[BpS*7];
    CLIP_STORE(6,C);
    C = 16-RND - Src[BpS*4] +3*Src[BpS*5] -7*Src[BpS*6] + 23*Src[BpS*7] + 14*Src[BpS*8];
    CLIP_STORE(7,C);
    Src += 1;
    Dst += 1;
  }
#endif
}
#undef CLIP_STORE

#define CLIP_STORE(i,C) \
  if (C<0) C = 0; else if (C>(255<<5)) C = 255; else C = C>>5;  \
  C = (C+Src[BpS*i+BpS]+1-RND) >> 1;  \
  STORE(Dst[BpS*i], C)

void FUNC_VA_UP(SKL_BYTE *Dst, const SKL_BYTE *Src, int H, int BpS)
{
#if (SIZE==16)
  while(H-->0) {
    int C;
    C = 16-RND +14*Src[BpS*0] +23*Src[BpS*1] - 7*Src[BpS*2] + 3*Src[BpS*3] -   Src[BpS*4];
    CLIP_STORE(0,C);
    C = 16-RND - 3*(Src[BpS*0]-Src[BpS*4]) +19*Src[BpS*1] +20*Src[BpS*2] - 6*Src[BpS*3] - Src[BpS*5];
    CLIP_STORE( 1,C);
    C = 16-RND + 2*Src[BpS*0] - 6*(Src[BpS*1]+Src[BpS*4]) +20*(Src[BpS*2]+Src[BpS*3]) + 3*Src[BpS*5] - Src[BpS*6];
    CLIP_STORE( 2,C);
    C = 16-RND - (Src[BpS*0]+Src[BpS*7 ]) + 3*(Src[BpS* 1]+Src[BpS* 6])-6*(Src[BpS* 2]+Src[BpS* 5]) + 20*(Src[BpS* 3]+Src[BpS* 4]);
    CLIP_STORE( 3,C);
    C = 16-RND - (Src[BpS*1]+Src[BpS*8 ]) + 3*(Src[BpS* 2]+Src[BpS* 7])-6*(Src[BpS* 3]+Src[BpS* 6]) + 20*(Src[BpS* 4]+Src[BpS* 5]);
    CLIP_STORE( 4,C);
    C = 16-RND - (Src[BpS*2]+Src[BpS*9 ]) + 3*(Src[BpS* 3]+Src[BpS* 8])-6*(Src[BpS* 4]+Src[BpS* 7]) + 20*(Src[BpS* 5]+Src[BpS* 6]);
    CLIP_STORE( 5,C);
    C = 16-RND - (Src[BpS*3]+Src[BpS*10]) + 3*(Src[BpS* 4]+Src[BpS* 9])-6*(Src[BpS* 5]+Src[BpS* 8]) + 20*(Src[BpS* 6]+Src[BpS* 7]);
    CLIP_STORE( 6,C);
    C = 16-RND - (Src[BpS*4]+Src[BpS*11]) + 3*(Src[BpS* 5]+Src[BpS*10])-6*(Src[BpS* 6]+Src[BpS* 9]) + 20*(Src[BpS* 7]+Src[BpS* 8]);
    CLIP_STORE( 7,C);
    C = 16-RND - (Src[BpS*5]+Src[BpS*12]) + 3*(Src[BpS* 6]+Src[BpS*11])-6*(Src[BpS* 7]+Src[BpS*10]) + 20*(Src[BpS* 8]+Src[BpS* 9]);
    CLIP_STORE( 8,C);
    C = 16-RND - (Src[BpS*6]+Src[BpS*13]) + 3*(Src[BpS* 7]+Src[BpS*12])-6*(Src[BpS* 8]+Src[BpS*11]) + 20*(Src[BpS* 9]+Src[BpS*10]);
    CLIP_STORE( 9,C);
    C = 16-RND - (Src[BpS*7]+Src[BpS*14]) + 3*(Src[BpS* 8]+Src[BpS*13])-6*(Src[BpS* 9]+Src[BpS*12]) + 20*(Src[BpS*10]+Src[BpS*11]);
    CLIP_STORE(10,C);
    C = 16-RND - (Src[BpS*8]+Src[BpS*15]) + 3*(Src[BpS* 9]+Src[BpS*14])-6*(Src[BpS*10]+Src[BpS*13]) + 20*(Src[BpS*11]+Src[BpS*12]);
    CLIP_STORE(11,C);
    C = 16-RND - (Src[BpS*9]+Src[BpS*16]) + 3*(Src[BpS*10]+Src[BpS*15])-6*(Src[BpS*11]+Src[BpS*14]) + 20*(Src[BpS*12]+Src[BpS*13]);
    CLIP_STORE(12,C);
    C = 16-RND - Src[BpS*10] +3*Src[BpS*11] -6*(Src[BpS*12]+Src[BpS*15]) + 20*(Src[BpS*13]+Src[BpS*14]) +2*Src[BpS*16];
    CLIP_STORE(13,C);
    C = 16-RND - Src[BpS*11] +3*(Src[BpS*12]-Src[BpS*16]) -6*Src[BpS*13] + 20*Src[BpS*14] + 19*Src[BpS*15];
    CLIP_STORE(14,C);
    C = 16-RND - Src[BpS*12] +3*Src[BpS*13] -7*Src[BpS*14] + 23*Src[BpS*15] + 14*Src[BpS*16];
    CLIP_STORE(15,C);
    Src += 1;
    Dst += 1;
  }
#else
  while(H-->0) {
    int C;
    C = 16-RND +14*Src[BpS*0] +23*Src[BpS*1] - 7*Src[BpS*2] + 3*Src[BpS*3] -   Src[BpS*4];
    CLIP_STORE(0,C);
    C = 16-RND - 3*(Src[BpS*0]-Src[BpS*4]) +19*Src[BpS*1] +20*Src[BpS*2] - 6*Src[BpS*3] - Src[BpS*5];
    CLIP_STORE(1,C);
    C = 16-RND + 2*Src[BpS*0] - 6*(Src[BpS*1]+Src[BpS*4]) +20*(Src[BpS*2]+Src[BpS*3]) + 3*Src[BpS*5] - Src[BpS*6];
    CLIP_STORE(2,C);
    C = 16-RND - (Src[BpS*0]+Src[BpS*7]) + 3*(Src[BpS*1]+Src[BpS*6])-6*(Src[BpS*2]+Src[BpS*5]) + 20*(Src[BpS*3]+Src[BpS*4]);
    CLIP_STORE(3,C);
    C = 16-RND - (Src[BpS*1]+Src[BpS*8]) + 3*(Src[BpS*2]+Src[BpS*7])-6*(Src[BpS*3]+Src[BpS*6]) + 20*(Src[BpS*4]+Src[BpS*5]);
    CLIP_STORE(4,C);
    C = 16-RND - Src[BpS*2] +3*Src[BpS*3] -6*(Src[BpS*4]+Src[BpS*7]) + 20*(Src[BpS*5]+Src[BpS*6]) +2*Src[BpS*8];
    CLIP_STORE(5,C);
    C = 16-RND - Src[BpS*3] +3*(Src[BpS*4]-Src[BpS*8]) -6*Src[BpS*5] + 20*Src[BpS*6] + 19*Src[BpS*7];
    CLIP_STORE(6,C);
    C = 16-RND - Src[BpS*4] +3*Src[BpS*5] -7*Src[BpS*6] + 23*Src[BpS*7] + 14*Src[BpS*8];
    CLIP_STORE(7,C);
    Src += 1;
    Dst += 1;
  }
#endif
}
#undef CLIP_STORE

#undef RND
#undef FUNC_H
#undef FUNC_V
#undef FUNC_HA
#undef FUNC_VA
#undef FUNC_HA_UP
#undef FUNC_VA_UP

#endif /* SKL_AUTO_INCLUDE */

//////////////////////////////////////////////////////////
