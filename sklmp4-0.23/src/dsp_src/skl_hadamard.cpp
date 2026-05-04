/********************************************************
 * Some code. Copyright (C) 2003 by Pascal Massimino.   *
 * All Rights Reserved.      (http://skal.planet-d.net) *
 * For Educational/Academic use ONLY. See 'LICENSE.TXT'.*
 ********************************************************/
/*
 *  skl_hadamard.cpp
 *
 *   Hadamard (<=> Walsh) transform
 *
 ********************************************************/

#include <math.h>

extern "C" {

typedef short SKL_INT16;
typedef unsigned char SKL_BYTE;
typedef int SKL_INT32;
typedef unsigned int SKL_UINT32;

//////////////////////////////////////////////////////////

#define ABS_V(x)  ((x)<0 ? -(x) : (x))

#define LOAD_BUTF(Src, m1, m2, a, b) \
    (m1) = (Src)[(a)] - (Src)[(b)];  \
    (m2) = (Src)[(a)] + (Src)[(b)]

#define BUTF(a, b, tmp)   \
    (tmp) = (a)-(b);      \
    (b)   = (a)+(b);      \
    (a)   = (tmp)

//////////////////////////////////////////////////////////
// Hadamard

void Skl_Hadamard_C( SKL_INT16 *C )
{
  int *Ecx, Tmp[64];
  SKL_INT16 *Eax;

  Eax = C;
  for(Ecx=Tmp; Ecx!=&Tmp[8]; Eax++, Ecx++)    // Emulates MMX 
  {
    int mm0, mm1, mm2, mm3, mm4, mm5, mm6, mm7, Spill;

    LOAD_BUTF( Eax, mm0, mm1, 0*8, 1*8 );
    LOAD_BUTF( Eax, mm2, mm3, 2*8, 3*8 );
    BUTF( mm1, mm3, mm7 );
    BUTF( mm0, mm2, mm7 );

    LOAD_BUTF( Eax, mm4, mm5, 4*8, 5*8 );
    LOAD_BUTF( Eax, mm6, mm7, 6*8, 7*8 );
    BUTF( mm4, mm6, Spill );
    BUTF( mm5, mm7, Spill );

    BUTF( mm3, mm7, Spill );
    Ecx[0*8] = mm7;
    Ecx[1*8] = mm3;

    BUTF( mm1, mm5, mm7 );
    Ecx[2*8] = mm1;
    Ecx[3*8] = mm5;

    BUTF( mm0, mm4, mm7 );
    Ecx[4*8] = mm4;
    Ecx[5*8] = mm0;

    BUTF( mm2, mm6, mm7 );
    Ecx[6*8] = mm2;
    Ecx[7*8] = mm6;
  }

  Eax = C;
  for(Ecx=Tmp; Ecx!=&Tmp[64]; Ecx+=8, Eax+=8)
  {
    int mm0, mm1, mm2, mm3, mm4, mm5, mm6, mm7, Spill;

    LOAD_BUTF( Ecx, mm0, mm1, 0, 1 );
    LOAD_BUTF( Ecx, mm2, mm3, 2, 3 );
    BUTF( mm1, mm3, mm7 );
    BUTF( mm0, mm2, mm7 );

    LOAD_BUTF( Ecx, mm4, mm5, 4, 5 );
    LOAD_BUTF( Ecx, mm6, mm7, 6, 7 );
    BUTF( mm4, mm6, Spill );
    BUTF( mm5, mm7, Spill );

    BUTF( mm3, mm7, Spill );
    Eax[0] = mm7; Eax[1] = mm3;

    BUTF( mm1, mm5, mm7 );
    Eax[2] = mm1; Eax[3] = mm5;

    BUTF( mm0, mm4, mm7 );
    Eax[4] = mm4; Eax[5] = mm0;

    BUTF( mm2, mm6, mm7 );
    Eax[6] = mm2; Eax[7] = mm6;
  }
//  for(i=0; i<64; i++) C[i] /= 8;
}

SKL_UINT32 Skl_Hadamard_Dev_8x8_C(const SKL_BYTE *Src, SKL_INT32 BpS)
{
  int *Ecx, Tmp[64];

  for(Ecx=Tmp; Ecx!=&Tmp[8]; Src+=BpS, Ecx++)    // Emulates MMX 
  {
    int mm0, mm1, mm2, mm3, mm4, mm5, mm6, mm7, Spill;

    LOAD_BUTF( Src, mm0, mm1, 0, 1 );
    LOAD_BUTF( Src, mm2, mm3, 2, 3 );
    BUTF( mm1, mm3, mm7 );
    BUTF( mm0, mm2, mm7 );

    LOAD_BUTF( Src, mm4, mm5, 4, 5 );
    LOAD_BUTF( Src, mm6, mm7, 6, 7 );
    BUTF( mm4, mm6, Spill );
    BUTF( mm5, mm7, Spill );

    BUTF( mm3, mm7, Spill );
    Ecx[0*8] = mm7;
    Ecx[1*8] = mm3;

    BUTF( mm1, mm5, mm7 );
    Ecx[2*8] = mm1;
    Ecx[3*8] = mm5;

    BUTF( mm0, mm4, mm7 );
    Ecx[4*8] = mm4;
    Ecx[5*8] = mm0;

    BUTF( mm2, mm6, mm7 );
    Ecx[6*8] = mm2;
    Ecx[7*8] = mm6;
  }

  SKL_UINT32 Sum = 0;
  for(Ecx=Tmp; Ecx!=&Tmp[64]; Ecx+=8)
  {
    int mm0, mm1, mm2, mm3, mm4, mm5, mm6, mm7, Spill;

    LOAD_BUTF( Ecx, mm0, mm1, 0, 1 );
    LOAD_BUTF( Ecx, mm2, mm3, 2, 3 );
    BUTF( mm1, mm3, mm7 );
    BUTF( mm0, mm2, mm7 );

    LOAD_BUTF( Ecx, mm4, mm5, 4, 5 );
    LOAD_BUTF( Ecx, mm6, mm7, 6, 7 );
    BUTF( mm4, mm6, Spill );
    BUTF( mm5, mm7, Spill );

    BUTF( mm3, mm7, Spill );
    if (Ecx!=Tmp) Sum += ABS_V(mm7);  // omit Mean=Coeff[0][0] from sum
    Sum += ABS_V(mm3);

    BUTF( mm1, mm5, mm7 );
    Sum += ABS_V(mm1);
    Sum += ABS_V(mm5);

    BUTF( mm0, mm4, mm7 );
    Sum += ABS_V(mm0);
    Sum += ABS_V(mm4);

    BUTF( mm2, mm6, mm7 );
    Sum += ABS_V(mm2);
    Sum += ABS_V(mm6);
  }
  return Sum;
}

SKL_UINT32 Skl_Hadamard_Dev_16x16_C(const SKL_BYTE *Src, SKL_INT32 BpS)
{
  SKL_UINT32 Sum;
  Sum  = Skl_Hadamard_Dev_8x8_C(Src,BpS) + Skl_Hadamard_Dev_8x8_C(Src+8,BpS);
  Src += 8*BpS;
  Sum += Skl_Hadamard_Dev_8x8_C(Src,BpS) + Skl_Hadamard_Dev_8x8_C(Src+8,BpS);
  return Sum;
}

//////////////////////////////////////////////////////////

#define LOAD_DIFF(m1, m2, a, b)      \
    (m1) = Src1[(a)]-Src2[(a)];      \
    (m2) = Src1[(b)]-Src2[(b)]

//////////////////////////////////////////////////////////
// 4x4 transform and SAD
//////////////////////////////////////////////////////////

void Skl_Hadamard_4x4_C( SKL_INT16 *C )
{
  int *Ecx, Tmp[16];
  SKL_INT16 *Eax;

  for(Eax=C, Ecx=Tmp; Ecx!=&Tmp[4]; Eax++, Ecx++)    // Emulates MMX 
  {
    int mm0, mm1, mm2, mm3, mm7;

    LOAD_BUTF( Eax, mm0, mm1, 0*4, 1*4 );
    LOAD_BUTF( Eax, mm2, mm3, 2*4, 3*4 );
    BUTF( mm1, mm3, mm7 );
    BUTF( mm0, mm2, mm7 );
    Ecx[0*4] = mm3;
    Ecx[1*4] = mm1;
    Ecx[2*4] = mm0;
    Ecx[3*4] = mm2;
  }

  for(Eax=C, Ecx=Tmp; Ecx!=&Tmp[16]; Eax+=4, Ecx+=4)
  {
    int mm0, mm1, mm2, mm3, mm7;

    LOAD_BUTF( Ecx, mm0, mm1, 0, 1 );
    LOAD_BUTF( Ecx, mm2, mm3, 2, 3 );
    BUTF( mm1, mm3, mm7 );
    BUTF( mm0, mm2, mm7 );
    Eax[0] = mm3;
    Eax[1] = mm1;
    Eax[2] = mm0;
    Eax[3] = mm2;
  }
}

SKL_UINT32 Skl_Hadamard_SAD_4x4_C(const SKL_BYTE *Src1, const SKL_BYTE *Src2, SKL_INT32 BpS)
{
  int *Ecx, Tmp[16];
  SKL_UINT32 Sum;

  for(Ecx=Tmp; Ecx!=&Tmp[16]; Src1+=BpS, Src2+=BpS, Ecx+=4)    // Emulates MMX 
  {
    int mm0, mm1, mm2, mm3, mm7;

    LOAD_DIFF( mm0, mm1, 0, 1 );
    LOAD_DIFF( mm2, mm3, 2, 3 );
    BUTF( mm0, mm1, mm7 );
    BUTF( mm2, mm3, mm7 );
    BUTF( mm1, mm3, mm7 );
    BUTF( mm0, mm2, mm7 );
    Ecx[0] = mm3;
    Ecx[1] = mm1;
    Ecx[2] = mm0;
    Ecx[3] = mm2;
  }

  Sum = 0;
  for(Ecx=Tmp; Ecx!=&Tmp[4]; Ecx++)
  {
    int mm0, mm1, mm2, mm3, mm7;

    LOAD_BUTF( Ecx, mm0, mm1, 0*4, 1*4 );
    LOAD_BUTF( Ecx, mm2, mm3, 2*4, 3*4 );
    BUTF( mm1, mm3, mm7 );
    BUTF( mm0, mm2, mm7 );
    Sum += ABS_V(mm3) + ABS_V(mm1);
    Sum += ABS_V(mm2) + ABS_V(mm0);
  }
  return Sum;
}

//////////////////////////////////////////////////////////

static const int H4[4*4] = {
  1, 1, 1, 1,
  1, 1,-1,-1,
  1,-1,-1, 1,
  1,-1, 1,-1
};

SKL_UINT32 Skl_Hadamard_SAD_4x4_Ref(const SKL_BYTE *Src1, const SKL_BYTE *Src2, SKL_INT32 BpS)
{
  int i, j;
  SKL_INT16 Tmp[4][4];
  SKL_UINT32 SAD = 0;
  for (i=0; i<4; i++) {
    for (j=0; j<4; j++)
    {
      int Sum = 0;
      for (int k=0; k<4; k++) Sum += H4[k*4+j]*(Src2[i*BpS+k]-Src1[i*BpS+k]);
      Tmp[i][j] = Sum;
    }
  }
  for (i=0; i<4; i++) {
    for (j=0; j<4; j++) {
      int Sum = 0;
      for (int k=0; k<4; k++) Sum += H4[k*4+i]*Tmp[k][j];
      SAD += ABS_V(Sum);
    }
  }
  return SAD;
}

void Skl_Hadamard_4x4_Ref( SKL_INT16 *C )
{
  int i, j;
  SKL_INT16 Tmp[4][4];
  for (i=0; i<4; i++) {
    for (j=0; j<4; j++)
    {
      int Sum = 0;
      for (int k=0; k<4; k++) Sum += H4[k*4+j]*C[4*i+k];
      Tmp[i][j] = Sum;
    }
  }
  for (i=0; i<4; i++) {
    for (j=0; j<4; j++) {
      int Sum = 0;
      for (int k=0; k<4; k++) Sum += H4[k*4+i]*Tmp[k][j];
      C[4*i+j] = Sum;
    }
  }
}

//////////////////////////////////////////////////////////
// 8x8 transform and SAD
//////////////////////////////////////////////////////////

SKL_UINT32 Skl_Hadamard_SAD_8x8_C(const SKL_BYTE *Src1, const SKL_BYTE *Src2, SKL_INT32 BpS)
{
  int *Ecx, Tmp[64];
  SKL_UINT32 Sum;

  for(Ecx=Tmp; Ecx!=&Tmp[64]; Src1+=BpS, Src2+=BpS, Ecx+=8)    // Emulates MMX 
  {
    int mm0, mm1, mm2, mm3, mm4, mm5, mm6, mm7, Spill;

    LOAD_DIFF( mm0, mm1, 0, 1 );
    LOAD_DIFF( mm2, mm3, 2, 3 );
    BUTF( mm0, mm1, mm7 );  BUTF( mm2, mm3, mm7 );
    BUTF( mm1, mm3, mm7 );  BUTF( mm0, mm2, mm7 );

    LOAD_DIFF( mm4, mm5, 4, 5 );
    LOAD_DIFF( mm6, mm7, 6, 7 );
    BUTF( mm4, mm5, Spill ); BUTF( mm6, mm7, Spill );
    BUTF( mm4, mm6, Spill ); BUTF( mm5, mm7, Spill );

    BUTF( mm3, mm7, Spill );
    Ecx[0] = mm7; 
    Ecx[1] = mm3;

    BUTF( mm1, mm5, mm7 );
    Ecx[2] = mm1;
    Ecx[3] = mm5;

    BUTF( mm0, mm4, mm7 );
    Ecx[4] = mm4;
    Ecx[5] = mm0;

    BUTF( mm2, mm6, mm7 );
    Ecx[6] = mm2;
    Ecx[7] = mm6;
  }

  Sum = 0;
  for(Ecx=Tmp; Ecx!=&Tmp[8]; Ecx++)
  {
    int mm0, mm1, mm2, mm3, mm4, mm5, mm6, mm7, Spill;

    LOAD_BUTF( Ecx, mm0, mm1, 0*8, 1*8 );
    LOAD_BUTF( Ecx, mm2, mm3, 2*8, 3*8 );
    BUTF( mm1, mm3, mm7 );
    BUTF( mm0, mm2, mm7 );

    LOAD_BUTF( Ecx, mm4, mm5, 4*8, 5*8 );
    LOAD_BUTF( Ecx, mm6, mm7, 6*8, 7*8 );
    BUTF( mm4, mm6, Spill );
    BUTF( mm5, mm7, Spill );

    BUTF( mm3, mm7, Spill );
    Sum += ABS_V(mm7) + ABS_V(mm3);

    BUTF( mm1, mm5, mm7 );
    Sum += ABS_V(mm5) + ABS_V(mm1);

    BUTF( mm0, mm4, mm7 );
    Sum += ABS_V(mm4) + ABS_V(mm0);

    BUTF( mm2, mm6, mm7 );
    Sum += ABS_V(mm6) + ABS_V(mm2);
  }
  return Sum;
}

//////////////////////////////////////////////////////////
// 16x8 SAD
//////////////////////////////////////////////////////////

SKL_UINT32 Skl_Hadamard_SAD_16x8_Field_C(const SKL_BYTE *Src1, const SKL_BYTE *Src2, SKL_INT32 BpS)
{
  SKL_UINT32 Sad;
  Sad  = Skl_Hadamard_SAD_8x8_C(Src1,   Src2,   2*BpS);
  Sad += Skl_Hadamard_SAD_8x8_C(Src1+8, Src2+8, 2*BpS);
  return Sad;
}

//////////////////////////////////////////////////////////
// 16x16 SAD
//////////////////////////////////////////////////////////

SKL_UINT32 Skl_Hadamard_SAD_16x16_C(const SKL_BYTE *Src1, const SKL_BYTE *Src2, SKL_INT32 BpS)
{
  SKL_UINT32 Sum;

  int *Ecx, Tmp[256];
  for(Ecx=Tmp; Ecx!=&Tmp[256]; Src1+=BpS, Src2+=BpS, Ecx+=16)    // Emulates MMX 
  {
    int mm0, mm1, mm2, mm3, mm4, mm5, mm6, mm7, Spill;

    LOAD_DIFF( mm0, mm1, 0, 1 );
    LOAD_DIFF( mm2, mm3, 2, 3 );
    BUTF( mm0, mm1, mm7 );  BUTF( mm2, mm3, mm7 );
    BUTF( mm1, mm3, mm7 );  BUTF( mm0, mm2, mm7 );

    LOAD_DIFF( mm4, mm5, 4, 5 );
    LOAD_DIFF( mm6, mm7, 6, 7 );
    BUTF( mm4, mm5, Spill ); BUTF( mm6, mm7, Spill );
    BUTF( mm4, mm6, Spill ); BUTF( mm5, mm7, Spill );

    BUTF( mm3, mm7, Spill );
    Ecx[0] = mm7; 
    Ecx[1] = mm3;

    BUTF( mm1, mm5, mm7 );
    Ecx[2] = mm1;
    Ecx[3] = mm5;

    BUTF( mm0, mm4, mm7 );
    Ecx[4] = mm4;
    Ecx[5] = mm0;

    BUTF( mm2, mm6, mm7 );
    Ecx[6] = mm2;
    Ecx[7] = mm6;

    LOAD_DIFF( mm0, mm1, 8, 9 );
    LOAD_DIFF( mm2, mm3,10,11 );
    BUTF( mm0, mm1, mm7 );  BUTF( mm2, mm3, mm7 );
    BUTF( mm1, mm3, mm7 );  BUTF( mm0, mm2, mm7 );

    LOAD_DIFF( mm4, mm5,12,13 );
    LOAD_DIFF( mm6, mm7,14,15 );
    BUTF( mm4, mm5, Spill ); BUTF( mm6, mm7, Spill );
    BUTF( mm4, mm6, Spill ); BUTF( mm5, mm7, Spill );

    BUTF( mm3, mm7, Spill );
    Ecx[ 8] = mm7; 
    Ecx[ 9] = mm3;

    BUTF( mm1, mm5, mm7 );
    Ecx[10] = mm1;
    Ecx[11] = mm5;

    BUTF( mm0, mm4, mm7 );
    Ecx[12] = mm4;
    Ecx[13] = mm0;

    BUTF( mm2, mm6, mm7 );
    Ecx[14] = mm2;
    Ecx[15] = mm6;
  }

  Sum = 0;
  for(Ecx=Tmp; Ecx!=&Tmp[16]; Ecx++)
  {
    int mm0, mm1, mm2, mm3, mm4, mm5, mm6, mm7, Spill;

    LOAD_BUTF( Ecx, mm0, mm1, 0*16, 1*16 );
    LOAD_BUTF( Ecx, mm2, mm3, 2*16, 3*16 );
    BUTF( mm1, mm3, mm7 );
    BUTF( mm0, mm2, mm7 );

    LOAD_BUTF( Ecx, mm4, mm5, 4*16, 5*16 );
    LOAD_BUTF( Ecx, mm6, mm7, 6*16, 7*16 );
    BUTF( mm4, mm6, Spill );
    BUTF( mm5, mm7, Spill );

    BUTF( mm3, mm7, Spill );
    Sum += ABS_V(mm7) + ABS_V(mm3);

    BUTF( mm1, mm5, mm7 );
    Sum += ABS_V(mm5) + ABS_V(mm1);

    BUTF( mm0, mm4, mm7 );
    Sum += ABS_V(mm4) + ABS_V(mm0);

    BUTF( mm2, mm6, mm7 );
    Sum += ABS_V(mm6) + ABS_V(mm2);


    LOAD_BUTF( Ecx, mm0, mm1, 8*16, 9*16 );
    LOAD_BUTF( Ecx, mm2, mm3,10*16,11*16 );
    BUTF( mm1, mm3, mm7 );
    BUTF( mm0, mm2, mm7 );

    LOAD_BUTF( Ecx, mm4, mm5,12*16,13*16 );
    LOAD_BUTF( Ecx, mm6, mm7,14*16,15*16 );
    BUTF( mm4, mm6, Spill );
    BUTF( mm5, mm7, Spill );

    BUTF( mm3, mm7, Spill );
    Sum += ABS_V(mm7) + ABS_V(mm3);

    BUTF( mm1, mm5, mm7 );
    Sum += ABS_V(mm5) + ABS_V(mm1);

    BUTF( mm0, mm4, mm7 );
    Sum += ABS_V(mm4) + ABS_V(mm0);

    BUTF( mm2, mm6, mm7 );
    Sum += ABS_V(mm6) + ABS_V(mm2);
  }

#if 0
  Sum  = Skl_Hadamard_SAD_8x8_C(  Src1,   Src2, BpS);
  Sum += Skl_Hadamard_SAD_8x8_C(Src1+8, Src2+8, BpS);
  Src1 += 8*BpS; Src2 += 8*BpS;
  Sum += Skl_Hadamard_SAD_8x8_C(  Src1,   Src2, BpS);
  Sum += Skl_Hadamard_SAD_8x8_C(Src1+8, Src2+8, BpS);
#endif

  return Sum;
}


//////////////////////////////////////////////////////////

}   // extern "C"
