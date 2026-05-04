/********************************************************
 * Some code. Copyright (C) 2003 by Pascal Massimino.   *
 * All Rights Reserved.      (http://skal.planet-d.net) *
 * For Educational/Academic use ONLY. See 'LICENSE.TXT'.*
 ********************************************************/
/*
 *  skl_quant_ref.cpp
 *
 *  reference quantization/dequantization
 *
 ********************************************************/

#include <math.h>
#include "skl.h"
#include "skl_syst/skl_dsp.h"

extern "C" {

#define DIV(x,y,b)  ( ((x)<0) ? -((-(x)+(b)) / (y)) : ((x)+(b)) / (y) )
#define ABS(x)      ( (x)<0 ? -(x) : (x) )

//////////////////////////////////////////////////////////
// MPEG-4 way for quantizing
//////////////////////////////////////////////////////////

static void Init_Quantizer_MPEG4_Ref(SKL_QUANTIZER Q, const SKL_BYTE M[64],
                                     const SKL_BYTE *Scale_Map, int For_Intra)
{
  for(int k=1; k<32; ++k) {
    const int q = (Scale_Map==0) ? (k<<1) : Scale_Map[k-1];
    for(int i=0; i<64; ++i) {
      const int S = q*M[i];
      Q[0][k-1][0][i] = S;    // quant
      Q[0][k-1][1][i] = For_Intra ? ((3*S+4)>>3) : 0;
      Q[1][k-1][0][i] = S;    // dequant
      Q[1][k-1][1][i] = For_Intra ? 0 : (S>>1);
    }
  }
}

static void Quant_Intra_MPEG4_Ref(SKL_INT16 *Out,
                                  const SKL_INT16 *In,
                                  const SKL_QUANTIZER Q,
                                  SKL_INT32 q, SKL_INT32 DC_q)
{
  Out[0] = DIV(In[0], DC_q, DC_q>>1);

  SKL_ASSERT(q>0 && q<32);
  const SKL_INT16 * const Qs = Q[0][q-1][0];
  const SKL_INT16 * const Qb = Q[0][q-1][1];
  for(int i=1; i<64; ++i) {
    if (In[i]!=0) Out[i] = DIV( (In[i]<<4), Qs[i], Qb[i]);
    else Out[i] = 0;
  }
}

static SKL_INT32 Quant_Inter_MPEG4_Ref(SKL_INT16 *Out,
                                       const SKL_INT16 *In,
                                       const SKL_QUANTIZER Q,
                                       SKL_INT32 q)
{
  SKL_ASSERT(q>0 && q<32);
  const SKL_INT16 * const Qs = Q[0][q-1][0];
  const SKL_INT16 * const Qb = Q[0][q-1][1];
  SKL_INT32 Sum = 0;
  for(int i=0; i<64; ++i) {
    if (In[i]!=0) Out[i] = DIV( (In[i]<<4), Qs[i], Qb[i]);
    else          Out[i] = 0;
    Sum += ABS(Out[i]);
  }
  return Sum;
}

//////////////////////////////////////////////////////////
// MPEG4 Dequant
//////////////////////////////////////////////////////////

static void Dequant_Intra_MPEG4_Ref(SKL_INT16 *Out,
                                    const SKL_INT16 *In,
                                    const SKL_QUANTIZER Q,
                                    SKL_INT32 q, SKL_INT32 DC_q)
{
  SKL_ASSERT(q>0 && q<32);
  const SKL_INT16 * const Qs = Q[1][q-1][0];
  const SKL_INT16 * const Qb = Q[1][q-1][1];

  SKL_INT32 v;
  v = In[0]*DC_q;
  Out[0] = (v<-2048) ? -2048 : (v>2047) ? 2047 : (SKL_INT16)v;

  for(int i=1; i<64; ++i) {
    if (In[i]==0)
      Out[i] = 0;
    else {
      if (In[i]<0) v = -(SKL_INT32)((-In[i]*Qs[i]+Qb[i]) >> 4);
      else         v =  (SKL_INT32)(( In[i]*Qs[i]+Qb[i]) >> 4);
      Out[i] = (v<-2048) ? -2048 : (v>2047) ? 2047 : (SKL_INT16)v;
    }
  }
}

static void Dequant_Inter_MPEG4_Ref(SKL_INT16 *Out,
                                    const SKL_INT16 *In,
                                    const SKL_QUANTIZER Q,
                                    SKL_INT32 q, int Rows)
{
  SKL_ASSERT(q>0 && q<32);
  const SKL_INT16 * Qs = Q[1][q-1][0];
  const SKL_INT16 * Qb = Q[1][q-1][1];

  int Sum = 0;
  int i, j;
  for(i=0; i<8; ++i) {
    if (Rows & (1<<i)) {
      for(j=0; j<8; ++j)
      {
        int v = (int)In[j];
        if (v==0) Out[j] = 0;
        else {
          if (v<0) v = -((-v*Qs[j]+Qb[j]) >> 4);
          else     v =  (( v*Qs[j]+Qb[j]) >> 4);
          Out[j] = (v<-2048) ? -2048 : (v>2047) ? 2047 : (SKL_INT16)v;
          Sum ^= Out[j];
        }
      }
    }
    else for(j=0; j<8; ++j) Out[j] = 0;
    Qs += 8;
    Qb += 8;
    In += 8;
    Out += 8;
  }
  if ( !(Sum&1) )
    Out[-1] ^= 1;   // mismatch control on coeff #63
}

//////////////////////////////////////////////////////////
// H.263 way of quantizing
//////////////////////////////////////////////////////////

static void Init_Quantizer_H263_Ref(SKL_QUANTIZER Q, const SKL_BYTE M[64],
                                    const SKL_BYTE *Scale_Map, int For_Intra)
{
  SKL_ASSERT(Scale_Map==0);
  for(int k=1; k<32; ++k) {
    const int q = (k<<1);
    const int b = (q>>1);
    for(int i=0; i<64; ++i) {
      Q[0][k-1][0][i] = q;    // quant
      Q[0][k-1][1][i] = (b&1) ? 1 : 0;
      Q[1][k-1][0][i] = q;    // dequant
      Q[1][k-1][1][i] = (b&1) ? b : b-1;
    }
  }
}

static void Quant_Intra_H263_Ref(SKL_INT16 *Out,
                                 const SKL_INT16 *In,
                                 const SKL_QUANTIZER Q,
                                 SKL_INT32 q, SKL_INT32 DC_q)
{
  Out[0] = DIV(In[0], DC_q, DC_q>>1);

  SKL_ASSERT(q>0 && q<32);
  const SKL_INT16 * const Qs = Q[0][q-1][0];
  const SKL_INT16 * const Qb = Q[0][q-1][1];
  for(int i=1; i<64; ++i) {
    if      (In[i]> Qb[i]) Out[i] = (In[i]-Qb[i]) / Qs[i];
    else if (In[i]<-Qb[i]) Out[i] = (In[i]+Qb[i]) / Qs[i];
    else                   Out[i] = 0;
  }
}

static SKL_INT32 Quant_Inter_H263_Ref(SKL_INT16 *Out,
                                      const SKL_INT16 *In,
                                      const SKL_QUANTIZER Q,
                                      SKL_INT32 q)
{
  SKL_ASSERT(q>0 && q<32);
  const SKL_INT16 * const Qs = Q[0][q-1][0];
  const SKL_INT16 * const Qb = Q[0][q-1][1];
  SKL_INT32 Sum = 0;
  for(int i=0; i<64; ++i) {
    if      (In[i]> Qb[i]) Out[i] = (In[i]-Qb[i]) / Qs[i];
    else if (In[i]<-Qb[i]) Out[i] = (In[i]+Qb[i]) / Qs[i];
    else                   Out[i] = 0;
    Sum += ABS(Out[i]);
  }
  return Sum;
}


//////////////////////////////////////////////////////////
// H263 Dequant
//////////////////////////////////////////////////////////

static void Dequant_Intra_H263_Ref(SKL_INT16 *Out,
                                   const SKL_INT16 *In,
                                   const SKL_QUANTIZER Q,
                                   SKL_INT32 q, SKL_INT32 DC_q)
{
  SKL_ASSERT(q>0 && q<32);
  const SKL_INT16 * const Qs = Q[1][q-1][0];
  const SKL_INT16 * const Qb = Q[1][q-1][1];
  SKL_INT32 v;

  v = In[0]*DC_q;
  Out[0] = (v<-2048) ? -2048 : (v>2047) ? 2047 : (SKL_INT16)v;

  for(int i=1; i<64; ++i) {
    if (In[i]==0) Out[i] = 0;
    else {
      if (In[i]<0) v = In[i]*Qs[i] - Qb[i];
      else         v = In[i]*Qs[i] + Qb[i];
      Out[i] = (v<-2048) ? -2048 : (v>2047) ? 2047 : (SKL_INT16)v;
    }
  }
}

static void Dequant_Inter_H263_Ref(SKL_INT16 *Out,
                                   const SKL_INT16 *In,
                                   const SKL_QUANTIZER Q,
                                   SKL_INT32 q, int Rows)
{
  SKL_ASSERT(q>0 && q<32);
  const SKL_INT16 * Qs = Q[1][q-1][0];
  const SKL_INT16 * Qb = Q[1][q-1][1];
  int i, j;
  for(i=0; i<8; ++i) {
    if (Rows & (1<<i)) {
      for(j=0; j<8; ++j) {
        int v = (int)In[j];
        if (v==0) Out[j] = 0;
        else {
          if (v<0) v = v*Qs[j] - Qb[j];
          else     v = v*Qs[j] + Qb[j];
          Out[j] = (v<-2048) ? -2048 : (v>2047) ? 2047 : (SKL_INT16)v;
        }
      }
    }
    else for(j=0; j<8; ++j) Out[j] = 0;
    Qs += 8;
    Qb += 8;
    In += 8;
    Out += 8;
  }
}

//////////////////////////////////////////////////////////
// MPEG2 Dequant (no MPEG1 oddification)
//////////////////////////////////////////////////////////

static void Init_Quantizer_MPEG2_Ref(SKL_QUANTIZER Q, const SKL_BYTE M[64],
                                     const SKL_BYTE *Scale_Map, int For_Intra)
{
  for(int k=1; k<32; ++k) {
    const int q = (Scale_Map==0) ? k : Scale_Map[k-1];
    for(int i=0; i<64; ++i) {
      const int S = q*M[i];
      Q[1][k-1][0][i] = 2*S;     // dequant
      Q[1][k-1][1][i] = S;  // only used for inter
    }
  }
}

#if 0
//  MPEG1 dequant as it should be (with oddification, and no mismatch ctrl).

static void Dequant_Intra_MPEG1_Ref(SKL_INT16 *Out,
                                    const SKL_INT16 *In,
                                    const SKL_QUANTIZER Q,
                                    SKL_INT32 q, SKL_INT32 DC_q)
{
  SKL_ASSERT(q>0 && q<32);
  const SKL_INT16 * const Qs = Q[1][q-1][0];
  SKL_INT32 v;

  v = In[0]*DC_q;
  Out[0] = (v<-2048) ? -2048 : (v>2047) ? 2047 : (SKL_INT16)v;
  for(int i=1; i<64; ++i) {
    if (In[i]==0) Out[i] = 0;
    else {
      if (In[i]<0) { v = ((-In[i]*Qs[i])>>5); if (v) v = -((v-1)|1); }
      else         { v = (( In[i]*Qs[i])>>5); if (v) v =   (v-1)|1;  }
      Out[i] = (v<-2048) ? -2048 : (v>2047) ? 2047 : (SKL_INT16)v;
    }
  }
}

static void Dequant_Inter_MPEG1_Ref(SKL_INT16 *Out,
                                    const SKL_INT16 *In,
                                    const SKL_QUANTIZER Q,
                                    SKL_INT32 q, int Rows)
{
  SKL_ASSERT(q>0 && q<32);
  const SKL_INT16 * Qs = Q[1][q-1][0];
  const SKL_INT16 * Qb = Q[1][q-1][1];

  int i, j;
  for(i=0; i<8; ++i) {
    if (Rows & (1<<i)) {
      for(j=0; j<8; ++j) {
        int v = *In++;
        if (v==0) *Out = 0;
        else {
          if (v<0) { v = ((-v*Qs[j]+Qb[j])>>5); if (v) v = -((v-1)|1); }
          else     { v = (( v*Qs[j]+Qb[j])>>5); if (v) v =   (v-1)|1;  }
          v = (v<-2048) ? -2048 : (v>2047) ? 2047 : v;
          Out[j] = (SKL_INT16)v;
        }
      }
    }
    else for(j=0; j<8; ++j) Out[j] = 0;
    Qs += 8;
    Qb += 8;
    In += 8;
    Out += 8;
  }
}
#endif

static void Dequant_Intra_MPEG2_Ref(SKL_INT16 *Out,
                                    const SKL_INT16 *In,
                                    const SKL_QUANTIZER Q,
                                    SKL_INT32 q, SKL_INT32 DC_q)
{
  SKL_ASSERT(q>0 && q<32);
  const SKL_INT16 * const Qs = Q[1][q-1][0];
  SKL_INT32 v;

  v = In[0]*DC_q;
  Out[0] = (v<-2048) ? -2048 : (v>2047) ? 2047 : (SKL_INT16)v;

  int Sum = 0;
  for(int i=1; i<64; ++i) {
    if (In[i]==0) Out[i] = 0;
    else {
      if (In[i]<0) v = -((-In[i]*Qs[i])>>5);
      else         v =  (( In[i]*Qs[i])>>5);
      v = (v<-2048) ? -2048 : (v>2047) ? 2047 : v;
      Out[i] = (SKL_INT16)v;
      Sum ^= v;
    }
  }
  if (!(Sum&1)) Out[63] ^= 1;   // mismatch control
}

static void Dequant_Inter_MPEG2_Ref(SKL_INT16 *Out,
                                    const SKL_INT16 *In,
                                    const SKL_QUANTIZER Q,
                                    SKL_INT32 q, int Rows)
{
  SKL_ASSERT(q>0 && q<32);
  const SKL_INT16 * Qs = Q[1][q-1][0];
  const SKL_INT16 * Qb = Q[1][q-1][1];

  int Sum = 0;
  int i, j;
  for(i=0; i<8; ++i) {
    if (Rows & (1<<i)) {
      for(j=0; j<8; ++j) {
        int v = In[j];
        if (v==0) Out[j] = 0;
        else {
          if (v<0) v = -((-v*Qs[j]+Qb[j])>>5);
          else     v =  (( v*Qs[j]+Qb[j])>>5);
          v = (v<-2048) ? -2048 : (v>2047) ? 2047 : v;
          Out[j] = (SKL_INT16)v;
          Sum ^= v;
        }
      }
    }
    else for(j=0; j<8; ++j) Out[j] = 0;
    Qs += 8;
    Qb += 8;
    In += 8;
    Out += 8;
  }
  if (!(Sum&1))
    Out[-1] ^= 1;   // mismatch control
}

#undef DIV
#undef ABS

//////////////////////////////////////////////////////////

static void Zero_Ref(SKL_INT16 C[64]) { SKL_BZERO(C, 64*sizeof(C[0])); }
static void Zero16_Ref(SKL_INT16 C[16]) { SKL_BZERO(C, 16*sizeof(C[0])); }

//////////////////////////////////////////////////////////
// exported methods

extern "C" void Skl_Dct16_Ref( SKL_INT16 *In );
extern "C" void Skl_IDct16_Ref( SKL_INT16 *In );
extern "C" void Skl_IDct16_Sparse_Ref( SKL_INT16 *In );
extern "C" void Skl_IDct16_Put_Ref( SKL_INT16 *In, SKL_BYTE *Dst, int BpS );
extern "C" void Skl_IDct16_Add_Ref( SKL_INT16 *In, SKL_BYTE *Dst, int BpS );

SKL_QUANT_DSP Skl_Quant_MPEG4_Dsp_Ref =
{
  "MPEG4-Ref", Skl_Switch_None,
  Init_Quantizer_MPEG4_Ref,
  Quant_Intra_MPEG4_Ref,        Quant_Inter_MPEG4_Ref,
  Dequant_Intra_MPEG4_Ref,      Dequant_Inter_MPEG4_Ref, 

  Zero_Ref, Zero16_Ref,
  Skl_Dct16_Ref, Skl_IDct16_Ref,
  Skl_IDct16_Sparse_Ref, Skl_IDct16_Put_Ref, Skl_IDct16_Add_Ref, 
  Skl_IDct16_Sparse_Ref, Skl_IDct16_Put_Ref, Skl_IDct16_Add_Ref
};

SKL_QUANT_DSP Skl_Quant_MPEG2_Dsp_Ref =
{
  "MPEG2-Ref", Skl_Switch_None,
  Init_Quantizer_MPEG2_Ref,
  0, 0,
  Dequant_Intra_MPEG2_Ref, Dequant_Inter_MPEG2_Ref, 

  Zero_Ref, Zero16_Ref,
  0, Skl_IDct16_Ref,
  Skl_IDct16_Sparse_Ref, Skl_IDct16_Put_Ref, Skl_IDct16_Add_Ref, 
  Skl_IDct16_Sparse_Ref, Skl_IDct16_Put_Ref, Skl_IDct16_Add_Ref
};

SKL_QUANT_DSP Skl_Quant_H263_Dsp_Ref =
{
  "H263-Ref", Skl_Switch_None,
  Init_Quantizer_H263_Ref,
  Quant_Intra_H263_Ref,         Quant_Inter_H263_Ref,
  Dequant_Intra_H263_Ref,       Dequant_Inter_H263_Ref,

  Zero_Ref, Zero16_Ref,
  Skl_Dct16_Ref, Skl_IDct16_Ref,
  Skl_IDct16_Sparse_Ref, Skl_IDct16_Put_Ref, Skl_IDct16_Add_Ref, 
  Skl_IDct16_Sparse_Ref, Skl_IDct16_Put_Ref, Skl_IDct16_Add_Ref
};

//////////////////////////////////////////////////////////

}   // extern "C"
