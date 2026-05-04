/********************************************************
 * Some code. Copyright (C) 2003 by Pascal Massimino.   *
 * All Rights Reserved.      (http://skal.planet-d.net) *
 * For Educational/Academic use ONLY. See 'LICENSE.TXT'.*
 ********************************************************/
/*
 * skl_quant_dsp.cpp
 *
 *   Quantization/Dequantization init
 *
 ********************************************************/

#include "skl.h"
#include "skl_syst/skl_dsp.h"

extern "C" {

//////////////////////////////////////////////////////////
// Initialization
//////////////////////////////////////////////////////////

extern SKL_QUANT_DSP Skl_Quant_MPEG4_Dsp_Ref;
extern SKL_QUANT_DSP Skl_Quant_MPEG4_Dsp_C;
extern SKL_QUANT_DSP Skl_Quant_H263_Dsp_Ref;
extern SKL_QUANT_DSP Skl_Quant_H263_Dsp_C;
extern SKL_QUANT_DSP Skl_Quant_MPEG2_Dsp_Ref;
#define Skl_Quant_MPEG2_Dsp_C Skl_Quant_MPEG2_Dsp_Ref
/* No special C-version of MPEG2. We can't do much more than the Ref impl... */

//////////////////////////////////////////////////////////
// MMX/SSE
//////////////////////////////////////////////////////////

#ifdef SKL_USE_ASM

  // We put C-part of the MMX/SSE code here, so that the
  // real stuff is only another pure ASM file.


#define INTRA_SIGNATURE SKL_INT16 *Out, const SKL_INT16 *In,  \
                        const SKL_QUANTIZER Q, SKL_INT32 q, SKL_INT32 DC_q
#define INTER_SIGNATURE SKL_INT16 *Out, const SKL_INT16 *In,  \
                        const SKL_QUANTIZER Q, SKL_INT32 q

extern void Skl_Dequant_Intra_MPEG4_MMX( INTRA_SIGNATURE );
extern void Skl_Dequant_Inter_MPEG4_MMX( INTER_SIGNATURE, int Rows );
extern void Skl_Dequant_Intra_MPEG4_SSE2( INTRA_SIGNATURE );
extern void Skl_Dequant_Inter_MPEG4_SSE2( INTER_SIGNATURE, int Rows );
//extern void Skl_Dequant_Intra_MPEG2_MMX( INTRA_SIGNATURE );
//extern void Skl_Dequant_Inter_MPEG2_MMX( INTER_SIGNATURE, int Rows );
extern void Skl_Dequant_Intra_H263_MMX( INTRA_SIGNATURE );
extern void Skl_Dequant_Inter_H263_MMX( INTER_SIGNATURE, int Rows );
extern void Skl_Dequant_Intra_H263_SSE( INTRA_SIGNATURE );
extern void Skl_Dequant_Inter_H263_SSE( INTER_SIGNATURE, int Rows );
extern void Skl_Dequant_Intra_H263_SSE2( INTRA_SIGNATURE );
extern void Skl_Dequant_Inter_H263_SSE2( INTER_SIGNATURE, int Rows );

extern void      Skl_Quant_Intra_MPEG4_MMX( INTRA_SIGNATURE );
extern SKL_INT32 Skl_Quant_Inter_MPEG4_MMX( INTER_SIGNATURE );
extern void      Skl_Quant_Intra_H263_MMX( INTRA_SIGNATURE );
extern SKL_INT32 Skl_Quant_Inter_H263_MMX( INTER_SIGNATURE );

extern "C" void Skl_Dct16_MMX( SKL_INT16 *In );
extern "C" void Skl_IDct16_MMX( SKL_INT16 *In );
extern "C" void Skl_Dct16_SSE( SKL_INT16 *In );
extern "C" void Skl_IDct16_SSE( SKL_INT16 *In );
extern "C" void Skl_Dct16_SSE2( SKL_INT16 *In );
extern "C" void Skl_IDct16_SSE2( SKL_INT16 *In );
extern "C" void Skl_IDct16_Sparse_SSE2( SKL_INT16 *In );
extern "C" void Skl_IDct16_Put_SSE2( SKL_INT16 *In, SKL_BYTE *Dst, int BpS );
extern "C" void Skl_IDct16_Add_SSE2( SKL_INT16 *In, SKL_BYTE *Dst, int BpS );

extern "C" void Skl_IDct16_Sparse_SSE( SKL_INT16 *In );
extern "C" void Skl_IDct16_Sparse_MMX( SKL_INT16 *In );
extern "C" void Skl_IDct16_Put_SSE( SKL_INT16 *In, SKL_BYTE *Dst, int BpS );
extern "C" void Skl_IDct16_Put_MMX( SKL_INT16 *In, SKL_BYTE *Dst, int BpS );
extern "C" void Skl_IDct16_Add_SSE( SKL_INT16 *In, SKL_BYTE *Dst, int BpS );
extern "C" void Skl_IDct16_Add_MMX( SKL_INT16 *In, SKL_BYTE *Dst, int BpS );
extern "C" void Skl_IDct16_Sparse_8x4_SSE( SKL_INT16 *In );
extern "C" void Skl_IDct16_Sparse_8x4_MMX( SKL_INT16 *In );
extern "C" void Skl_IDct16_Put_8x4_SSE( SKL_INT16 *In, SKL_BYTE *Dst, int BpS );
extern "C" void Skl_IDct16_Put_8x4_MMX( SKL_INT16 *In, SKL_BYTE *Dst, int BpS );
extern "C" void Skl_IDct16_Add_8x4_SSE( SKL_INT16 *In, SKL_BYTE *Dst, int BpS );
extern "C" void Skl_IDct16_Add_8x4_MMX( SKL_INT16 *In, SKL_BYTE *Dst, int BpS );

extern void Skl_Quant_Zero_MMX(SKL_INT16 C[64]);
extern void Skl_Quant_Zero_SSE2(SKL_INT16 C[64]);
extern void Skl_Quant_Zero16_MMX(SKL_INT16 C[16]);
extern void Skl_Quant_Zero16_SSE2(SKL_INT16 C[16]);

#define FIX(x,b)  ((1<<(b))/(x) + 1)

//////////////////////////////////////////////////////////
// MPEG4

static void Init_Quantizer_MPEG4_MMX_SSE(SKL_QUANTIZER Q, const SKL_BYTE M[64],
                                         const SKL_BYTE *Scale_Map, int For_Intra)
{
  int Shift = (For_Intra ? 2 : 1);  // <- for internal reasons
  int Bits = 16;
  if (Scale_Map!=0) { Shift >>= 1; Bits=15; }
  for(int k=1; k<32; ++k) {
    const int q = (Scale_Map==0) ? k : Scale_Map[k-1];
    for(int i=0; i<64; ++i) {
      const int S = q*M[i];
      Q[0][k-1][0][i] = FIX(S,Bits);  // quant
      Q[0][k-1][1][i] = For_Intra ? ((3*S+4)>>3) : 0;
      Q[1][k-1][0][i] = S << Shift;   // dequant
      Q[1][k-1][1][i] = For_Intra ? 0 : (S>>1); // Note:Bias actually hardcoded in ASM
    }
  }
}

static SKL_QUANT_DSP Skl_Quant_MPEG4_Dsp_MMX =
{
  "MPEG4-MMX", Skl_Switch_MMX,
  Init_Quantizer_MPEG4_MMX_SSE,

  Skl_Quant_Intra_MPEG4_MMX,    Skl_Quant_Inter_MPEG4_MMX,
  Skl_Dequant_Intra_MPEG4_MMX,  Skl_Dequant_Inter_MPEG4_MMX,

  Skl_Quant_Zero_MMX, Skl_Quant_Zero16_MMX,
  Skl_Dct16_MMX, Skl_IDct16_MMX,
  Skl_IDct16_Sparse_MMX, Skl_IDct16_Put_MMX, Skl_IDct16_Add_MMX,
  Skl_IDct16_Sparse_8x4_MMX, Skl_IDct16_Put_8x4_MMX, Skl_IDct16_Add_8x4_MMX
};

static SKL_QUANT_DSP Skl_Quant_MPEG4_Dsp_SSE =
{
  "MPEG4-SSE", Skl_Switch_MMX,
  Init_Quantizer_MPEG4_MMX_SSE,

  Skl_Quant_Intra_MPEG4_MMX,    Skl_Quant_Inter_MPEG4_MMX,
  Skl_Dequant_Intra_MPEG4_MMX,  Skl_Dequant_Inter_MPEG4_MMX,

  Skl_Quant_Zero_MMX, Skl_Quant_Zero16_MMX,
  Skl_Dct16_SSE, Skl_IDct16_SSE,
  Skl_IDct16_Sparse_SSE, Skl_IDct16_Put_SSE, Skl_IDct16_Add_SSE,
  Skl_IDct16_Sparse_8x4_SSE, Skl_IDct16_Put_8x4_SSE, Skl_IDct16_Add_8x4_SSE
};

static SKL_QUANT_DSP Skl_Quant_MPEG4_Dsp_SSE2 =
{
  "MPEG4-SSE2", Skl_Switch_MMX,
  Init_Quantizer_MPEG4_MMX_SSE,

  Skl_Quant_Intra_MPEG4_MMX,    Skl_Quant_Inter_MPEG4_MMX,
  Skl_Dequant_Intra_MPEG4_SSE2, Skl_Dequant_Inter_MPEG4_SSE2,

  Skl_Quant_Zero_SSE2, Skl_Quant_Zero16_SSE2,
  Skl_Dct16_SSE2, Skl_IDct16_SSE2,
  Skl_IDct16_Sparse_SSE2, Skl_IDct16_Put_SSE2, Skl_IDct16_Add_SSE2,
  Skl_IDct16_Sparse_SSE2, Skl_IDct16_Put_SSE2, Skl_IDct16_Add_SSE2
};

//////////////////////////////////////////////////////////
// MPEG2

static void Init_Quantizer_MPEG2_MMX_SSE(SKL_QUANTIZER Q, const SKL_BYTE M[64],
                                         const SKL_BYTE *Scale_Map, int For_Intra)
{
  for(int k=1; k<32; ++k) {
    const int q = (Scale_Map==0) ? k : Scale_Map[k-1];
    for(int i=0; i<64; ++i) {
      const int S = q*M[i];
      Q[1][k-1][0][i] = For_Intra ? (S<<1) : S; // dequant
      Q[1][k-1][1][i] = For_Intra ? 0 : S;
    }
  }
}

static SKL_QUANT_DSP Skl_Quant_MPEG2_Dsp_MMX =
{
  "MPEG2-MMX", Skl_Switch_MMX,
  Init_Quantizer_MPEG2_MMX_SSE,

  0, 0,
  Skl_Dequant_Intra_MPEG4_MMX, Skl_Dequant_Inter_MPEG4_MMX,

  Skl_Quant_Zero_MMX, Skl_Quant_Zero16_MMX,
  0, Skl_IDct16_MMX,

  Skl_IDct16_Sparse_MMX, Skl_IDct16_Put_MMX, Skl_IDct16_Add_MMX,
  Skl_IDct16_Sparse_8x4_MMX, Skl_IDct16_Put_8x4_MMX, Skl_IDct16_Add_8x4_MMX
};

static SKL_QUANT_DSP Skl_Quant_MPEG2_Dsp_SSE =
{
  "MPEG2-SSE", Skl_Switch_MMX,
  Init_Quantizer_MPEG2_MMX_SSE,

  0, 0,
  Skl_Dequant_Intra_MPEG4_MMX, Skl_Dequant_Inter_MPEG4_MMX,

  Skl_Quant_Zero_MMX, Skl_Quant_Zero16_MMX,
  0, Skl_IDct16_SSE,

  Skl_IDct16_Sparse_SSE, Skl_IDct16_Put_SSE, Skl_IDct16_Add_SSE,
  Skl_IDct16_Sparse_8x4_SSE, Skl_IDct16_Put_8x4_SSE, Skl_IDct16_Add_8x4_SSE
};

#define Skl_Quant_MPEG2_Dsp_SSE2 Skl_Quant_MPEG2_Dsp_SSE      // nothing special so far...

//////////////////////////////////////////////////////////
// H263

static void Init_Quantizer_H263_MMX_SSE(SKL_QUANTIZER Q, const SKL_BYTE M[64],
                                        const SKL_BYTE *Scale_Map, int For_Intra)
{
    // Note: 16b of fixed-point is not enough for q=1 => special loop in ASM
  for(int k=1; k<32; ++k) {
    const int q = (Scale_Map==0) ? (k<<1) : Scale_Map[k-1];
    const int b = (q>>1);
    for(int i=0; i<64; ++i) {
      Q[0][k-1][0][i] = FIX(q,16); // quant
      Q[0][k-1][1][i] = (b&1) ? 1 : 0;
      Q[1][k-1][0][i] = q;         // dequant
      Q[1][k-1][1][i] = (b&1) ? b : b-1;
    }
  }
}
  
static SKL_QUANT_DSP Skl_Quant_H263_Dsp_MMX =
{
  "H263-MMX", Skl_Switch_MMX,
  Init_Quantizer_H263_MMX_SSE,

  Skl_Quant_Intra_H263_MMX,     Skl_Quant_Inter_H263_MMX,
  Skl_Dequant_Intra_H263_MMX,   Skl_Dequant_Inter_H263_MMX,

  Skl_Quant_Zero_MMX, Skl_Quant_Zero16_MMX,
  Skl_Dct16_MMX, Skl_IDct16_MMX,
  Skl_IDct16_Sparse_MMX, Skl_IDct16_Put_MMX, Skl_IDct16_Add_MMX,
  Skl_IDct16_Sparse_8x4_MMX, Skl_IDct16_Put_8x4_MMX, Skl_IDct16_Add_8x4_MMX
};

static SKL_QUANT_DSP Skl_Quant_H263_Dsp_SSE =
{
  "H263-SSE", Skl_Switch_MMX,
  Init_Quantizer_H263_MMX_SSE,

  Skl_Quant_Intra_H263_MMX,     Skl_Quant_Inter_H263_MMX,
  Skl_Dequant_Intra_H263_SSE,   Skl_Dequant_Inter_H263_SSE,

  Skl_Quant_Zero_MMX, Skl_Quant_Zero16_MMX,
  Skl_Dct16_SSE, Skl_IDct16_SSE,
  Skl_IDct16_Sparse_SSE, Skl_IDct16_Put_SSE, Skl_IDct16_Add_SSE,
  Skl_IDct16_Sparse_8x4_SSE, Skl_IDct16_Put_8x4_SSE, Skl_IDct16_Add_8x4_SSE
};

static SKL_QUANT_DSP Skl_Quant_H263_Dsp_SSE2 =
{
  "H263-SSE2", Skl_Switch_MMX,
  Init_Quantizer_H263_MMX_SSE,

  Skl_Quant_Intra_H263_MMX,     Skl_Quant_Inter_H263_MMX,
  Skl_Dequant_Intra_H263_SSE2,  Skl_Dequant_Inter_H263_SSE2,

  Skl_Quant_Zero_SSE2, Skl_Quant_Zero16_SSE2,
  Skl_Dct16_SSE2, Skl_IDct16_SSE2,
  Skl_IDct16_Sparse_SSE2, Skl_IDct16_Put_SSE2, Skl_IDct16_Add_SSE2,
  Skl_IDct16_Sparse_SSE2, Skl_IDct16_Put_SSE2, Skl_IDct16_Add_SSE2
};

#else

#define Skl_Quant_MPEG4_Dsp_MMX  Skl_Quant_MPEG4_Dsp_C
#define Skl_Quant_MPEG4_Dsp_SSE  Skl_Quant_MPEG4_Dsp_C
#define Skl_Quant_MPEG4_Dsp_SSE2 Skl_Quant_MPEG4_Dsp_C
#define Skl_Quant_MPEG2_Dsp_MMX  Skl_Quant_MPEG2_Dsp_C
#define Skl_Quant_MPEG2_Dsp_SSE  Skl_Quant_MPEG2_Dsp_C
#define Skl_Quant_MPEG2_Dsp_SSE2 Skl_Quant_MPEG2_Dsp_C
#define Skl_Quant_H263_Dsp_MMX   Skl_Quant_MPEG4_Dsp_C
#define Skl_Quant_H263_Dsp_SSE   Skl_Quant_MPEG4_Dsp_C
#define Skl_Quant_H263_Dsp_SSE2  Skl_Quant_MPEG4_Dsp_C

#endif  /* SKL_USE_ASM */

//////////////////////////////////////////////////////////

int Skl_Init_Quant_DSP(SKL_QUANT_DSP *Dsp,
                       SKL_CPU_FEATURE Cpu,
                       int Quant_Type  /* 0 = H263, 1 = MPEG4, 2=MPEG1/2 */ )
{
  SKL_ASSERT(Dsp!=0);
  if (Cpu==SKL_CPU_DETECT) Cpu = Skl_Detect_CPU_Feature();

  if (Quant_Type==0) {
    if      (Cpu==SKL_CPU_C  )  *Dsp = Skl_Quant_H263_Dsp_C;
    else if (Cpu==SKL_CPU_X86)  *Dsp = Skl_Quant_H263_Dsp_C;
    else if (Cpu==SKL_CPU_MMX)  *Dsp = Skl_Quant_H263_Dsp_MMX;
    else if (Cpu==SKL_CPU_SSE)  *Dsp = Skl_Quant_H263_Dsp_SSE;
    else if (Cpu==SKL_CPU_SSE2) *Dsp = Skl_Quant_H263_Dsp_SSE2;
    else if (Cpu==SKL_CPU_REF)  *Dsp = Skl_Quant_H263_Dsp_Ref;
    else                        *Dsp = Skl_Quant_H263_Dsp_C;
  }
  else if (Quant_Type==1) {
    if      (Cpu==SKL_CPU_C  )  *Dsp = Skl_Quant_MPEG4_Dsp_C;
    else if (Cpu==SKL_CPU_X86)  *Dsp = Skl_Quant_MPEG4_Dsp_C;
    else if (Cpu==SKL_CPU_MMX)  *Dsp = Skl_Quant_MPEG4_Dsp_MMX;
    else if (Cpu==SKL_CPU_SSE)  *Dsp = Skl_Quant_MPEG4_Dsp_SSE;
    else if (Cpu==SKL_CPU_SSE2) *Dsp = Skl_Quant_MPEG4_Dsp_SSE2;
    else if (Cpu==SKL_CPU_REF)  *Dsp = Skl_Quant_MPEG4_Dsp_Ref;
    else                        *Dsp = Skl_Quant_MPEG4_Dsp_C;
  }
  else /*if (Quant_Type>=2)*/ {
    if      (Cpu==SKL_CPU_C  )  *Dsp = Skl_Quant_MPEG2_Dsp_C;
    else if (Cpu==SKL_CPU_X86)  *Dsp = Skl_Quant_MPEG2_Dsp_C;
    else if (Cpu==SKL_CPU_MMX)  *Dsp = Skl_Quant_MPEG2_Dsp_MMX;
    else if (Cpu==SKL_CPU_SSE)  *Dsp = Skl_Quant_MPEG2_Dsp_SSE;
    else if (Cpu==SKL_CPU_SSE2) *Dsp = Skl_Quant_MPEG2_Dsp_SSE2;
    else if (Cpu==SKL_CPU_REF)  *Dsp = Skl_Quant_MPEG2_Dsp_Ref;
    else                        *Dsp = Skl_Quant_MPEG2_Dsp_C;
  }
  if (Dsp->Switch_Off==0) Dsp->Switch_Off = Skl_Get_Switch(Cpu);
  return 1;
}

//////////////////////////////////////////////////////////

} /* extern "C" */
