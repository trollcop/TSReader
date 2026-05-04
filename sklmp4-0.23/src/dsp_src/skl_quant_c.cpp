/********************************************************
 * Some code. Copyright (C) 2003 by Pascal Massimino.   *
 * All Rights Reserved.      (http://skal.planet-d.net) *
 * For Educational/Academic use ONLY. See 'LICENSE.TXT'.*
 ********************************************************/
/*
 *  skl_quant_c.cpp
 *
 *  quantization/dequantization
 *
 ********************************************************/

#include <math.h>
#include "skl.h"
#include "skl_syst/skl_dsp.h"

extern "C" void Skl_Dct16_C( SKL_INT16 *In );
extern "C" void Skl_IDct16_C( SKL_INT16 *In );
extern "C" void Skl_IDct16_Sparse_C( SKL_INT16 *In );
extern "C" void Skl_IDct16_Put_C( SKL_INT16 *In, SKL_BYTE *Dst, int BpS );
extern "C" void Skl_IDct16_Add_C( SKL_INT16 *In, SKL_BYTE *Dst, int BpS );

extern "C" {

  // DIV_RND: operator `//` of the norm 
#define DIV_RND(x,y,b)  ( (x)<0 ? (((x) - (b)) / (y)) : (((x) + (b)) / (y)) )
#define FIX(c,n,b)  ((1<<(n))/(c) + (b))
#define FDIV(c,m,b,n) (((int)((c)+(b))*(m)) >> (n))

//////////////////////////////////////////////////////////
// MPEG-4 quant
//////////////////////////////////////////////////////////

  // Note: using variable precision is ~10% slower, but accurate almost
  // everywhere (even down to the unrealistic M[i]=1!)
  // TODO: implement this table with 'switch(q)' ??
static int Shifts[31] = { 
  15, 16, 17, 17, 18, 18, 18, 18, 
  19, 19, 19, 19, 19, 19, 19, 19, 
  20, 20, 20, 20, 20, 20, 20, 20, 
  20, 20, 20, 20, 20, 20, 20
};

static void Init_Quantizer_MPEG4_C(SKL_QUANTIZER Q, const SKL_BYTE M[64],
                                   const SKL_BYTE *Scale_Map, int For_Intra)
{
  for(int k=1; k<32; ++k) {
    const int q = (Scale_Map==0) ? (k<<1) : Scale_Map[k-1];
    for(int i=0; i<64; ++i) {
      SKL_ASSERT(M[i]>0);
      const int S = q*M[i];
      const int Fix = Shifts[k-1];
      Q[0][k-1][0][i] = FIX(S, Fix,1);  // quant
      Q[0][k-1][1][i] = For_Intra ? ((3*S+4)>>3) : 0;
      Q[1][k-1][0][i] = S;              // dequant
      Q[1][k-1][1][i] = For_Intra ? 0 : (S>>1);
    }
  }
}

static void Quant_Intra_MPEG4_C(SKL_INT16 *Out,
                                const SKL_INT16 *In,
                                const SKL_QUANTIZER Q,
                                SKL_INT32 q, SKL_INT32 DC_q)
{
  SKL_ASSERT(q>0 && q<32);
  Out[0] = DIV_RND(In[0], DC_q, DC_q>>1);
  const SKL_INT16 * const M = Q[0][q-1][0];
  const SKL_INT16 * const B = Q[0][q-1][1];
  const int Fix = Shifts[q-1];
  for(int i=1; i<64; ++i) {
    if      (In[i]<0) Out[i] = -FDIV( (-In[i])<<4, M[i], B[i], Fix );
    else if (In[i]>0) Out[i] =  FDIV( ( In[i])<<4, M[i], B[i], Fix );
    else              Out[i] = 0;
  }
}

static SKL_INT32 Quant_Inter_MPEG4_C(SKL_INT16 *Out,
                                     const SKL_INT16 *In,
                                     const SKL_QUANTIZER Q,
                                     SKL_INT32 q)
{
  SKL_INT32 Sum = 0;
  SKL_ASSERT(q>0 && q<32);
  const SKL_INT16 * const M = Q[0][q-1][0];
  // no bias for inter: const SKL_INT16 * const B = Q[0][q-1][1];
  const int Fix = Shifts[q-1];
  for(int i=0; i<64; ++i) {
    if      (In[i]<0) Sum -= ( Out[i] = -FDIV( (-In[i])<<4, M[i], 0, Fix ) );
    else if (In[i]>0) Sum += ( Out[i] =  FDIV( ( In[i])<<4, M[i], 0, Fix ) );
    else Out[i] = 0;
  }
  return Sum;
}

//////////////////////////////////////////////////////////
// MPEG4 Dequant
//////////////////////////////////////////////////////////

static void Dequant_Intra_MPEG4_C(SKL_INT16 *Out,
                                  const SKL_INT16 *In,
                                  const SKL_QUANTIZER Q,
                                  SKL_INT32 q, SKL_INT32 DC_q)
{
  SKL_ASSERT(q>0 && q<32);
  SKL_INT32 v;

  v = In[0]*DC_q;
  Out[0] = (v<-2048) ? -2048 : (v>2047) ? 2047 : (SKL_INT16)v;

  const SKL_INT16 * const M = Q[1][q-1][0] + 64;
  // no bias: const SKL_INT16 * const B = Q[1][q-1][1] + 64;
  for(int i=-63; i<0; ++i) {
    int v = In[64+i];
    if (v==0) Out[64+i] = 0;
    else {
      if (v<0) {
        v = -M[i]*v;
        Out[64+i] = (v>(2048<<4)) ? -2048 :-(SKL_INT16)(v>>4);
      }
      else {
        v =  M[i]*v;
        Out[64+i] = (v>(2047<<4)) ?  2047 : (SKL_INT16)(v>>4);
      }
    }
  }
}

static void Dequant_Inter_MPEG4_C(SKL_INT16 *Out,
                                  const SKL_INT16 *In,
                                  const SKL_QUANTIZER Q,
                                  SKL_INT32 q, int Rows)
{
  SKL_ASSERT(q>0 && q<32);
  const SKL_INT16 * M = Q[1][q-1][0];
  const SKL_INT16 * B = Q[1][q-1][1];

  int Sum = 0;
  int R = Rows;
  for(int i=-8; i<0; ++i, R>>=1) {
    In  += 8;
    Out += 8;
    M += 8;
    B += 8;
    if (R & 1) {
      for(int j=-8; j<0; ++j) {
        int v = In[j];
        if (v==0) Out[j] = 0;
        else {
          if (v<0) {
            v = (-M[j]*v+B[j]);
            v = (v>(2048<<4)) ? -2048 : -(v>>4);
          }
          else {
            v = ( M[j]*v+B[j]);
            v = (v>(2047<<4)) ?  2047 :  (v>>4);
          }
          Out[j] = v;
          Sum ^= v;
        }
      }
    }
    else SKL_BZERO(Out-8, 8*sizeof(*Out));
  }
  if ( !(Sum&1) )
    Out[-1] ^= 1;   // mismatch control on coeff #63
}

//////////////////////////////////////////////////////////
// H.263 way for quantizing
//////////////////////////////////////////////////////////

static void Init_Quantizer_H263_C(SKL_QUANTIZER Q, const SKL_BYTE M[64],
                                  const SKL_BYTE *Scale_Map, int For_Intra)
{
  SKL_ASSERT(Scale_Map==0);
  for(int k=1; k<32; ++k) {
    const int q = (k<<1);
    const int b = (q>>1);
    for(int i=0; i<64; ++i) {
      Q[0][k-1][0][i] = FIX(q, 15, 1);  // 15bits of fixed-prec is enough for all q
      Q[0][k-1][1][i] = (b&1) ? 1 : 0;
      Q[1][k-1][0][i] = q;
      Q[1][k-1][1][i] = (b&1) ? b : b-1;
    }
  }
}

static void Quant_Intra_H263_C(SKL_INT16 *Out,
                               const SKL_INT16 *In,
                               const SKL_QUANTIZER Q,
                               SKL_INT32 q, SKL_INT32 DC_q)
{
  SKL_ASSERT(q>0 && q<32);
  Out[0] = DIV_RND(In[0], DC_q, DC_q>>1);
  const int M = Q[0][q-1][0][0];
  for(int i=1; i<64; ++i) {
    const int B = Q[0][q-1][1][i];
    SKL_ASSERT(B>=0);
    if      (In[i]<-B) Out[i] =-FDIV(-In[i], M,-B, 15 );
    else if (In[i]> B) Out[i] = FDIV( In[i], M,-B, 15 );
    else               Out[i] = 0;
  }
}

SKL_INT32 Quant_Inter_H263_C(SKL_INT16 *Out,
                             const SKL_INT16 *In,
                             const SKL_QUANTIZER Q,
                             SKL_INT32 q)
{
  SKL_ASSERT(q>0 && q<32);
  SKL_INT32 Sum = 0;
  const int M = Q[0][q-1][0][0];
  for(int i=0; i<64; ++i) {
    const int B = Q[0][q-1][1][i];
    SKL_ASSERT(B>=0);
    if      (In[i]<-B) Sum -= ( Out[i] =-FDIV(-In[i], M,-B, 15 ) );
    else if (In[i]> B) Sum += ( Out[i] = FDIV( In[i], M,-B, 15 ) );
    else                        Out[i] = 0;
  }
  return Sum;
}

//////////////////////////////////////////////////////////
// H263 dequant

static void Dequant_Intra_H263_C(SKL_INT16 *Out,
                                const SKL_INT16 *In,
                                const SKL_QUANTIZER Q,
                                SKL_INT32 q, SKL_INT32 DC_q)
{
  SKL_ASSERT(q>0 && q<32);
  const SKL_INT32 M = Q[1][q-1][0][0];
  const SKL_INT32 B = Q[1][q-1][1][0];
  int v;

  v = In[0]*DC_q;
  Out[0] = (v<-2048) ? -2048 : (v>2047) ? 2047 : (SKL_INT16)v;

  for(int i=-63; i<0; ++i) {
    int v = In[64+i];
    if (v==0) Out[64+i] = 0;
    else if (v<0) { 
      v = v*M - B;
      Out[64+i] = (v<-2048) ? -2048 : (SKL_INT16)v;
    }
    else {
      v = v*M + B; 
      Out[64+i] = (v> 2047) ?  2047 : (SKL_INT16)v;
    }
  }
}

static void Dequant_Inter_H263_C(SKL_INT16 *Out,
                                 const SKL_INT16 *In,
                                 const SKL_QUANTIZER Q,
                                 SKL_INT32 q, int Rows)
{
  SKL_ASSERT(q>0 && q<32);
  const SKL_INT32 M = Q[1][q-1][0][0];
  const SKL_INT32 B = Q[1][q-1][1][0];
  int R = Rows;
  for(int i=-8; i<0; ++i, R>>=1) {
    In += 8;
    Out += 8;
    if (R & 1)
    {
      for(int j=-8; j<0; ++j) {
        int v = In[j];
        if (v==0) Out[j] = 0;
        else if (v<0) {
          v = v*M - B;
          Out[j] = (v<-2048) ? -2048 : (SKL_INT16)v;
        }
        else {
          v = v*M + B;
          Out[j] = (v>2047) ? 2047 : (SKL_INT16)v;
        }
      }
    }
    else SKL_BZERO(Out-8, 8*sizeof(*Out));
  }
}

//////////////////////////////////////////////////////////

static void Zero_C(SKL_INT16 C[64]) {
  SKL_UINT32 *P = (SKL_UINT32 *)&C[64];
  for(int i=-32; i<0; i+=4) {
    P[i   ] = 0;
    P[i+ 1] = 0;
    P[i+ 2] = 0;
    P[i+ 3] = 0;
  }
}

static void Zero16_C(SKL_INT16 C[16]) { 
  *(SKL_UINT32*)(C+ 0) = 0;
  *(SKL_UINT32*)(C+ 2) = 0;
  *(SKL_UINT32*)(C+ 4) = 0;
  *(SKL_UINT32*)(C+ 6) = 0;
  *(SKL_UINT32*)(C+ 8) = 0;
  *(SKL_UINT32*)(C+10) = 0;
  *(SKL_UINT32*)(C+12) = 0;
  *(SKL_UINT32*)(C+14) = 0;
}

//////////////////////////////////////////////////////////

SKL_QUANT_DSP Skl_Quant_MPEG4_Dsp_C =
{
  "MPEG4-C", Skl_Switch_None,
  Init_Quantizer_MPEG4_C,

  Quant_Intra_MPEG4_C,          Quant_Inter_MPEG4_C,
  Dequant_Intra_MPEG4_C,        Dequant_Inter_MPEG4_C,

  Zero_C, Zero16_C,
  Skl_Dct16_C, Skl_IDct16_C,
  Skl_IDct16_Sparse_C, Skl_IDct16_Put_C, Skl_IDct16_Add_C, 
  Skl_IDct16_Sparse_C, Skl_IDct16_Put_C, Skl_IDct16_Add_C
};

SKL_QUANT_DSP Skl_Quant_H263_Dsp_C =
{
  "H263-C", Skl_Switch_None,
  Init_Quantizer_H263_C,

  Quant_Intra_H263_C,           Quant_Inter_H263_C,
  Dequant_Intra_H263_C,         Dequant_Inter_H263_C,

  Zero_C, Zero16_C,
  Skl_Dct16_C, Skl_IDct16_C,
  Skl_IDct16_Sparse_C, Skl_IDct16_Put_C, Skl_IDct16_Add_C, 
  Skl_IDct16_Sparse_C, Skl_IDct16_Put_C, Skl_IDct16_Add_C
};

//////////////////////////////////////////////////////////

#undef DIV_RND
#undef FIX
#undef FDIV

}   // extern "C"
