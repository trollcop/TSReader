/********************************************************
 * Some code. Copyright (C) 2003 by Pascal Massimino.   *
 * All Rights Reserved.      (http://skal.planet-d.net) *
 * For Educational/Academic use ONLY. See 'LICENSE.TXT'.*
 ********************************************************/
/*
 *  skl_dct_ref.cpp
 *
 *  Reference implementation for some 2d transform
 *
 ************************************************************************/

#include "skl.h"
#include <math.h>

extern "C" {

#if defined(_WINDOWS) && !defined(M_PI)
#define M_PI 3.1415926535f
#endif

//////////////////////////////////////////////////////////
//                Reference 8x8 2d-Dct
//////////////////////////////////////////////////////////
//
//  Defining the NxN matrix: Cij = { Ai . cos(pi/2N * i*(2j+1) }
//  where Ai = 1/sqrt(8) if i==0, and Ai = 1/2 otherwise.
//  the unormalized (since tC.C = N.Id) forward and inverse Fourier 
//  transform  of M writes:
//     M' = tC.M.C, 
//     M  = C.M'.tC
//
//  Note: with MMX, e.g., working with columns is easier than rows.
//  Then forward DCT is usually implemented as 8 one-dimensional DCTs:
//    M' = tC. t( tC.tM )
//  => The matrix-matrix multiplication C.M is vectorized better than M.C.
//

//#define CLAMP(x,M) (((x)<-(M)) ? -(M) : ((Sum>(M)-1.0) ? ((M)-1.0) : Sum))
#define CLAMP(x,M) (x)
#define FDCT_RNG  2048.     // 12bits
#define IDCT_RNG  256.      //  9bits

static double Cos[8][8];
static int Ref_8x8_Ok = 0;
static void Init_Ref_DCT()
{
  for (int i=0; i<8; i++)
  {
    double scale = (i == 0) ? sqrt(0.125) : 0.5;
    for (int j=0; j<8; j++)
      Cos[i][j] = scale*cos( (M_PI/8.0)*i*(0.5 + j) );
  }
  Ref_8x8_Ok = 1;
}

void Skl_IDct16_Ref(SKL_INT16 *M)
{
  if (!Ref_8x8_Ok) Init_Ref_DCT();

  int i, j;
  double Tmp[8][8];

  for (i=0; i<8; i++) {
    for (j=0; j<8; j++)
    {
      double Sum = 0.0;
      for (int k=0; k<8; k++) Sum += Cos[k][j]*M[8*i+k];
      Tmp[i][j] = Sum;
    }
  }
  for (i=0; i<8; i++) {
    for (j=0; j<8; j++) {
      double Sum = 0.0;
      for (int k=0; k<8; k++) Sum += Cos[k][i]*Tmp[k][j];
      Sum += 0.5f;
      M[8*i+j] = (SKL_INT16)floor(CLAMP(Sum, IDCT_RNG));
    }
  }
}

void Skl_IDct16_Sparse_Ref(SKL_INT16 *M) { Skl_IDct16_Ref(M); }

void Skl_IDct16_Put_Ref(SKL_INT16 *M, SKL_BYTE *Dst, int BpS )
{
  if (!Ref_8x8_Ok) Init_Ref_DCT();

  int i, j;
  double Tmp[8][8];

  for (i=0; i<8; i++) {
    for (j=0; j<8; j++)
    {
      double Sum = 0.0;
      for (int k=0; k<8; k++) Sum += Cos[k][j]*M[8*i+k];
      Tmp[i][j] = Sum;
    }
  }
  for (i=0; i<8; i++) {
    for (j=0; j<8; j++) {
      double Sum = 0.0;
      for (int k=0; k<8; k++) Sum += Cos[k][i]*Tmp[k][j];
      Sum += 0.5f;
      int v = (int)floor(Sum);
      Dst[j] = v<0 ? 0 : v>255 ? 255 : (SKL_BYTE)v;
    }
    Dst += BpS;
  }
}

void Skl_IDct16_Add_Ref(SKL_INT16 *M, SKL_BYTE *Dst, int BpS )
{
  if (!Ref_8x8_Ok) Init_Ref_DCT();

  int i, j;
  double Tmp[8][8];

  for (i=0; i<8; i++) {
    for (j=0; j<8; j++)
    {
      double Sum = 0.0;
      for (int k=0; k<8; k++) Sum += Cos[k][j]*M[8*i+k];
      Tmp[i][j] = Sum;
    }
  }
  for (i=0; i<8; i++) {
    for (j=0; j<8; j++) {
      double Sum = 0.0;
      for (int k=0; k<8; k++) Sum += Cos[k][i]*Tmp[k][j];
      Sum += 0.5f;
      const int v = (int)floor(Sum) + (int)Dst[j];
      Dst[j] = v<0 ? 0 : v>255 ? 255 : (SKL_BYTE)v;
    }
    Dst += BpS;
  }
}

void Skl_Dct16_Ref(SKL_INT16 *M)
{
  if (!Ref_8x8_Ok) Init_Ref_DCT();

  int i, j;
  double Tmp[8][8];

  for (i=0; i<8; i++) {
    for (j=0; j<8; j++)
    {
      double Sum = 0.0;
      for (int k=0; k<8; k++) Sum += Cos[j][k]*M[8*i+k];
      Tmp[i][j] = Sum;
    }
  }
  for (i=0; i<8; i++) {
    for (j=0; j<8; j++) {
      double Sum = 0.0;
      for (int k=0; k<8; k++) Sum += Cos[i][k]*Tmp[k][j];
      Sum += 0.5 - 1.0e-6;
      M[8*i+j] = (SKL_INT16)floor(CLAMP(Sum, FDCT_RNG));
    }
  }
}


//////////////////////////////////////////////////////////
/// README ///////////////////////////////////////////////
//////////////////////////////////////////////////////////
//
// Slow general-purpose DCT
//
// [as defined in the MP3 norm (ISO/IEC 11172-3)]
//
//   Out[i] = sum{k=0..N-1} In[k]*cos(Pi/4N*(2i+1+N)*(2k+1))
//
// We have a lot of symmetries here:
//
//    Out[  i] = -Out[ N-1-i]
//    Out[N+i] =  Out[2N-1-i]
//
// for i in 0..N-1. 
//
// Roughly speaking, the first half [0..N-1] of ouput
// is anti-symmetric, whereas the second is symmetric.
// 
//////////////////////////////////////////////////////////

void Skl_Generic_IDct_Ref(int N, const float *In, float *Out)
{
  for( int i=0; i<2*N; i++ )
  {
    double Sum = 0.0;
    double Phi = M_PI/(4.0*N)*(1.0+N+2*i);
    for (int k=0; k<N; k++)
      Sum += In[k] * cos( Phi*(2.0*k+1.0) );
    Out[i] = (float)Sum;
  }
}

//////////////////////////////////////////////////////////
// Hadamard (<=> Walsh) transforms
//////////////////////////////////////////////////////////

static int H8[8*8] = {
  1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1,-1,-1,-1,-1,
  1, 1,-1,-1,-1,-1, 1, 1,
  1, 1,-1,-1, 1, 1,-1,-1,
  1,-1,-1, 1, 1,-1,-1, 1,
  1,-1,-1, 1,-1, 1, 1,-1,
  1,-1, 1,-1,-1, 1,-1, 1,
  1,-1, 1,-1, 1,-1, 1,-1
};

/*
      The matrix for the Walsh transform is:  
   
static int W8[8*8] = {
  1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1,-1,-1,-1,-1,
  1, 1,-1,-1, 1, 1,-1,-1,
  1, 1,-1,-1,-1,-1, 1, 1,
  1,-1, 1,-1, 1,-1, 1,-1,
  1,-1, 1,-1,-1, 1,-1, 1,
  1,-1,-1, 1, 1,-1,-1, 1,
  1,-1,-1, 1,-1, 1, 1,-1 
};
     but is equivalent to the Hadamard transform,
     up to a reordering: 
          [01234567] => [01327645]
     of the rows and columns of Hadamard's output
*/
  
void Skl_Hadamard_Ref( SKL_INT16 *C )
{  
  int i, j;
  SKL_INT16 Tmp[8][8];   
  for (i=0; i<8; i++) {  
    for (j=0; j<8; j++)  
    {
      int Sum = 0;
      for (int k=0; k<8; k++) Sum += H8[k*8+j]*C[8*i+k];
      Tmp[i][j] = Sum;   
    }
  }
  for (i=0; i<8; i++) {
    for (j=0; j<8; j++) {
      int Sum = 0;
      for (int k=0; k<8; k++) Sum += H8[k*8+i]*Tmp[k][j];
    C[8*i+j] = Sum;
    }
  }
} 

//////////////////////////////////////////////////////////

}   // extern "C"
