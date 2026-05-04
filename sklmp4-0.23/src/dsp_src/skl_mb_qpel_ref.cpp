/********************************************************
 * Some code. Copyright (C) 2003 by Pascal Massimino.   *
 * All Rights Reserved.      (http://skal.planet-d.net) *
 * For Educational/Academic use ONLY. See 'LICENSE.TXT'.*
 ********************************************************/
/*
 *  skl_mb_qpel_ref.cpp
 *
 *  Quarter-pixel interpolation reference impl
 *
 ********************************************************/

#ifndef SKL_AUTO_INCLUDE

#include "skl.h"
#include "skl_syst/skl_dsp.h"

extern "C" {

//////////////////////////////////////////////////////////
//
//  Quarter-pixel mess starts here. This is as tedious
//  as you can imagine :(
//
//////////////////////////////////////////////////////////

static const SKL_INT16 FIR_Tab_8[9][8] = {
  { 14, -3,  2, -1,  0,  0,  0,  0 }
, { 23, 19, -6,  3, -1,  0,  0,  0 }
, { -7, 20, 20, -6,  3, -1,  0,  0 }
, {  3, -6, 20, 20, -6,  3, -1,  0 }
, { -1,  3, -6, 20, 20, -6,  3, -1 }
, {  0, -1,  3, -6, 20, 20, -6,  3 }
, {  0,  0, -1,  3, -6, 20, 20, -7 }
, {  0,  0,  0, -1,  3, -6, 19, 23 }
, {  0,  0,  0,  0, -1,  2, -3, 14 }
};

static const SKL_INT16 FIR_Tab_16[17][16] = {
  { 14, -3,  2, -1,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 }
, { 23, 19, -6,  3, -1,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 }
, { -7, 20, 20, -6,  3, -1,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 }
, {  3, -6, 20, 20, -6,  3, -1,  0,  0,  0,  0,  0,  0,  0,  0,  0 }
, { -1,  3, -6, 20, 20, -6,  3, -1,  0,  0,  0,  0,  0,  0,  0,  0 }
, {  0, -1,  3, -6, 20, 20, -6,  3, -1,  0,  0,  0,  0,  0,  0,  0 }
, {  0,  0, -1,  3, -6, 20, 20, -6,  3, -1,  0,  0,  0,  0,  0,  0 }
, {  0,  0,  0, -1,  3, -6, 20, 20, -6,  3, -1,  0,  0,  0,  0,  0 }
, {  0,  0,  0,  0, -1,  3, -6, 20, 20, -6,  3, -1,  0,  0,  0,  0 }
, {  0,  0,  0,  0,  0, -1,  3, -6, 20, 20, -6,  3, -1,  0,  0,  0 }
, {  0,  0,  0,  0,  0,  0, -1,  3, -6, 20, 20, -6,  3, -1,  0,  0 }
, {  0,  0,  0,  0,  0,  0,  0, -1,  3, -6, 20, 20, -6,  3, -1,  0 }
, {  0,  0,  0,  0,  0,  0,  0,  0, -1,  3, -6, 20, 20, -6,  3, -1 }
, {  0,  0,  0,  0,  0,  0,  0,  0,  0, -1,  3, -6, 20, 20, -6,  3 }
, {  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, -1,  3, -6, 20, 20, -7 }
, {  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, -1,  3, -6, 19, 23 }
, {  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, -1,  2, -3, 14 }
};

//////////////////////////////////////////////////////////
// Implementation

#define SKL_AUTO_INCLUDE

  // 16x? filters

#define SIZE  16
#define TABLE FIR_Tab_16
#define STORE(d,s)  (d) = (s)

#define RND 1
#define FUNC_H      Skl_H_Pass_16_Copy_Rnd1_Ref
#define FUNC_V      Skl_V_Pass_16_Copy_Rnd1_Ref
#define FUNC_HA     Skl_H_Pass_Avrg_16_Copy_Rnd1_Ref
#define FUNC_VA     Skl_V_Pass_Avrg_16_Copy_Rnd1_Ref
#define FUNC_HA_UP  Skl_H_Pass_Avrg_Up_16_Copy_Rnd1_Ref
#define FUNC_VA_UP  Skl_V_Pass_Avrg_Up_16_Copy_Rnd1_Ref

#include "./skl_mb_qpel_ref.cpp"   /* self-include ourself */

#define RND 0
#define FUNC_H      Skl_H_Pass_16_Copy_Rnd0_Ref
#define FUNC_V      Skl_V_Pass_16_Copy_Rnd0_Ref
#define FUNC_HA     Skl_H_Pass_Avrg_16_Copy_Rnd0_Ref
#define FUNC_VA     Skl_V_Pass_Avrg_16_Copy_Rnd0_Ref
#define FUNC_HA_UP  Skl_H_Pass_Avrg_Up_16_Copy_Rnd0_Ref
#define FUNC_VA_UP  Skl_V_Pass_Avrg_Up_16_Copy_Rnd0_Ref

#include "./skl_mb_qpel_ref.cpp"
#undef STORE

  // note: Rounding is always 0 for B-frame dst mixing
#define STORE(d,s)  (d) = ( (s)+(d)+1 ) >> 1

#define RND 0
#define FUNC_H      Skl_H_Pass_16_Add_Rnd0_Ref
#define FUNC_V      Skl_V_Pass_16_Add_Rnd0_Ref
#define FUNC_HA     Skl_H_Pass_Avrg_16_Add_Rnd0_Ref
#define FUNC_VA     Skl_V_Pass_Avrg_16_Add_Rnd0_Ref
#define FUNC_HA_UP  Skl_H_Pass_Avrg_Up_16_Add_Rnd0_Ref
#define FUNC_VA_UP  Skl_V_Pass_Avrg_Up_16_Add_Rnd0_Ref

#include "./skl_mb_qpel_ref.cpp"
#undef STORE


#undef SIZE
#undef TABLE

  // 8x? filters

#define SIZE  8
#define TABLE FIR_Tab_8
#define STORE(d,s)  (d) = (s)

#define RND 1
#define FUNC_H      Skl_H_Pass_8_Copy_Rnd1_Ref
#define FUNC_V      Skl_V_Pass_8_Copy_Rnd1_Ref
#define FUNC_HA     Skl_H_Pass_Avrg_8_Copy_Rnd1_Ref
#define FUNC_VA     Skl_V_Pass_Avrg_8_Copy_Rnd1_Ref
#define FUNC_HA_UP  Skl_H_Pass_Avrg_Up_8_Copy_Rnd1_Ref
#define FUNC_VA_UP  Skl_V_Pass_Avrg_Up_8_Copy_Rnd1_Ref

#include "./skl_mb_qpel_ref.cpp"

#define RND 0
#define FUNC_H      Skl_H_Pass_8_Copy_Rnd0_Ref
#define FUNC_V      Skl_V_Pass_8_Copy_Rnd0_Ref
#define FUNC_HA     Skl_H_Pass_Avrg_8_Copy_Rnd0_Ref
#define FUNC_VA     Skl_V_Pass_Avrg_8_Copy_Rnd0_Ref
#define FUNC_HA_UP  Skl_H_Pass_Avrg_Up_8_Copy_Rnd0_Ref
#define FUNC_VA_UP  Skl_V_Pass_Avrg_Up_8_Copy_Rnd0_Ref

#include "./skl_mb_qpel_ref.cpp"
#undef STORE

  // note: Rounding is always 0 for B-frame dst mixing
#define STORE(d,s)  (d) = ( (s)+(d)+1 ) >> 1

#define RND 0
#define FUNC_H      Skl_H_Pass_8_Add_Rnd0_Ref
#define FUNC_V      Skl_V_Pass_8_Add_Rnd0_Ref
#define FUNC_HA     Skl_H_Pass_Avrg_8_Add_Rnd0_Ref
#define FUNC_VA     Skl_V_Pass_Avrg_8_Add_Rnd0_Ref
#define FUNC_HA_UP  Skl_H_Pass_Avrg_Up_8_Add_Rnd0_Ref
#define FUNC_VA_UP  Skl_V_Pass_Avrg_Up_8_Add_Rnd0_Ref

#include "./skl_mb_qpel_ref.cpp"
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

void FUNC_H(SKL_BYTE *Dst, const SKL_BYTE *Src, int H, int BpS)
{
  while(H-->0) {
    int i, k;
    int Sums[SIZE] = { 0 };
    for(i=0; i<=SIZE; ++i)
      for(k=0; k<SIZE; ++k)
        Sums[k] += TABLE[i][k] * Src[i];

    for(i=0; i<SIZE; ++i) {
      int C = ( Sums[i] + 16-RND ) >> 5;
      if (C<0) C = 0; else if (C>255) C = 255;
      STORE(Dst[i], C);
    }
    Src += BpS;
    Dst += BpS;
  }
}

void FUNC_HA(SKL_BYTE *Dst, const SKL_BYTE *Src, int H, int BpS)
{
  while(H-->0) {
    int i, k;
    int Sums[SIZE] = { 0 };
    for(i=0; i<=SIZE; ++i)
      for(k=0; k<SIZE; ++k)
        Sums[k] += TABLE[i][k] * Src[i];

    for(i=0; i<SIZE; ++i) {
      int C = ( Sums[i] + 16-RND ) >> 5;
      if (C<0) C = 0; else if (C>255) C = 255;
      C = (C+Src[i]+1-RND) >> 1;
      STORE(Dst[i], C);
    }
    Src += BpS;
    Dst += BpS;
  }
}

void FUNC_HA_UP(SKL_BYTE *Dst, const SKL_BYTE *Src, int H, int BpS)
{
  while(H-->0) {
    int i, k;
    int Sums[SIZE] = { 0 };
    for(i=0; i<=SIZE; ++i)
      for(k=0; k<SIZE; ++k)
        Sums[k] += TABLE[i][k] * Src[i];

    for(i=0; i<SIZE; ++i) {
      int C = ( Sums[i] + 16-RND ) >> 5;
      if (C<0) C = 0; else if (C>255) C = 255;
      C = (C+Src[i+1]+1-RND) >> 1;
      STORE(Dst[i], C);
    }
    Src += BpS;
    Dst += BpS;
  }
}

//////////////////////////////////////////////////////////
// vertical passes
//////////////////////////////////////////////////////////
// Note: for vertical passes, width (W) needs only be 8 or 16.

void FUNC_V(SKL_BYTE *Dst, const SKL_BYTE *Src, int W, int BpS)
{
  while(W-->0) {
    int i, k;
    int Sums[SIZE] = { 0 };
    const SKL_BYTE *S = Src++;
    for(i=0; i<=SIZE; ++i) {
      for(k=0; k<SIZE; ++k)
        Sums[k] += TABLE[i][k] * S[0];
      S += BpS;
    }

    SKL_BYTE *D = Dst++;
    for(i=0; i<SIZE; ++i) {
      int C = ( Sums[i] + 16-RND )>>5;
      if (C<0) C = 0; else if (C>255) C = 255;
      STORE(D[0], C);
      D += BpS;
    }
  }
}

void FUNC_VA(SKL_BYTE *Dst, const SKL_BYTE *Src, int W, int BpS)
{
  while(W-->0) {
    int i, k;
    int Sums[SIZE] = { 0 };
    const SKL_BYTE *S = Src;
    for(i=0; i<=SIZE; ++i) {
      for(k=0; k<SIZE; ++k)
        Sums[k] += TABLE[i][k] * S[0];
      S += BpS;
    }

    SKL_BYTE *D = Dst;
    S = Src;
    for(i=0; i<SIZE; ++i) {
      int C = ( Sums[i] + 16-RND )>>5;
      if (C<0) C = 0; else if (C>255) C = 255;
      C = ( C+S[0]+1-RND ) >> 1;
      STORE(D[0], C);
      D += BpS;
      S += BpS;
    }
    Src++;
    Dst++;
  }
}

void FUNC_VA_UP(SKL_BYTE *Dst, const SKL_BYTE *Src, int W, int BpS)
{
  while(W-->0) {
    int i, k;
    int Sums[SIZE] = { 0 };
    const SKL_BYTE *S = Src;
    for(i=0; i<=SIZE; ++i) {
      for(k=0; k<SIZE; ++k)
        Sums[k] += TABLE[i][k] * S[0];
      S += BpS;
    }

    SKL_BYTE *D = Dst;
    S = Src + BpS;
    for(i=0; i<SIZE; ++i) {
      int C = ( Sums[i] + 16-RND )>>5;
      if (C<0) C = 0; else if (C>255) C = 255;
      C = ( C+S[0]+1-RND ) >> 1;
      STORE(D[0], C);
      D += BpS;
      S += BpS;
    }
    Dst++;
    Src++;
  }
}

#undef RND
#undef FUNC_H
#undef FUNC_V
#undef FUNC_HA
#undef FUNC_VA
#undef FUNC_HA_UP
#undef FUNC_VA_UP

#endif /* SKL_AUTO_INCLUDE */

//////////////////////////////////////////////////////////
