/********************************************************
 * Some code. Copyright (C) 2003 by Pascal Massimino.   *
 * All Rights Reserved.      (http://skal.planet-d.net) *
 * For Educational/Academic use ONLY. See 'LICENSE.TXT'.*
 ********************************************************/
/*
 *  skl_img_flt_c.cpp
 *
 *   Image filtering
 *
 ********************************************************/

#include <math.h>

extern "C" {

  // so we don't even have to include skl.h
typedef unsigned char SKL_BYTE;
typedef signed char SKL_INT8;

//////////////////////////////////////////////////////////
// Downsampling 4x4 filters:
//
//        [1 3 3 1]      [-1 -3  3  1]     [-1 -3 -3 -1]
// Smooth:[3 9 9 3]   Gx:[-3 -9  9  3]  Gy:[-3 -9 -9 -3]
//        [3 9 9 3]      [-3 -9  9  3]     [ 3  9  9  3]
//        [1 3 3 1]      [-1 -3  3  1]     [ 1  3  3  1]
//
//  Input:18x18   Output:8x8
//////////////////////////////////////////////////////////

#define STORE(x)   *d = (x); d += Dst_BpS

void Skl_Smooth_18x18_To_8x8_C(SKL_BYTE *Dst, int Dst_BpS,
                              const SKL_BYTE *Src, int Src_BpS)
{
#define LOAD(x) (x) = 3*(s[1]+s[0]) +s[2]+s[-1]; s += Src_BpS

  int i;

  Src -= Src_BpS;
  for(i=0; i<8; ++i)
  {
    int mx0, mx1, tmp;
    int j;
    const SKL_BYTE *s = Src + 2*i;
    SKL_BYTE *d = Dst + i;

    LOAD(mx0);
    LOAD(tmp);
    mx0 += 3*tmp;

    for(j=4; j>0; --j) {
      LOAD(mx1); mx0 += 3*mx1;
      LOAD(tmp); mx0 += tmp; mx1 += 3*tmp;
      STORE( (32+mx0)>>6 );

      LOAD(mx0); mx1 += 3*mx0;
      LOAD(tmp); mx1 += tmp; mx0 += 3*tmp;
      STORE( (32+mx1)>>6 );
    }
  }
#undef LOAD
}


void Skl_Gradx_18x18_To_8x8_C(SKL_INT8 *Dst, int Dst_BpS,
                              const SKL_BYTE *Src, int Src_BpS)
{
#define LOAD(x) (x) =   3*(s[1]-s[0]) +s[2]-s[-1]; s += Src_BpS

  int i;

  Src -= Src_BpS;
  for(i=0; i<8; ++i)
  {
    int mx0, mx1, tmp;
    int j;
    const SKL_BYTE *s = Src + 2*i;
    SKL_INT8 *d = Dst + i;

    LOAD(mx0);
    LOAD(tmp);
    mx0 += 3*tmp;

    for(j=4; j>0; --j)
    {
      LOAD(mx1); mx0 += 3*mx1;
      LOAD(tmp); mx0 += tmp;
      mx1 += 3*tmp;
      STORE( (64+mx0)>>7 );

      LOAD(mx0); mx1 += 3*mx0;
      LOAD(tmp); mx1 += tmp;
      mx0 += 3*tmp;
      STORE( (64+mx1)>>7 );
    }
  }
#undef LOAD
}


void Skl_Grady_18x18_To_8x8_C(SKL_INT8 *Dst, int Dst_BpS,
                              const SKL_BYTE *Src, int Src_BpS)
{
#define LOAD(x) (x) = 3*(s[1]+s[0]) +s[2]+s[-1]; s += Src_BpS

  int i;

  Src -= Src_BpS;
  for(i=0; i<8; ++i)
  {
    int mx0, mx1, tmp;
    int j;
    const SKL_BYTE *s = Src + 2*i;
    SKL_INT8 *d = Dst + i;

    LOAD(mx0);
    LOAD(tmp);
    mx0 += 3*tmp;

    for(j=4; j>0; --j)
    {
      LOAD(mx1); mx0 -= 3*mx1;
      LOAD(tmp); mx0 -= tmp;
      mx1 += 3*tmp;
      STORE( (64-mx0)>>7 );

      LOAD(mx0); mx1 -= 3*mx0;
      LOAD(tmp); mx1 -= tmp;
      mx0 += 3*tmp;
      STORE( (64-mx1)>>7 );
    }
  }
#undef LOAD
}


void Skl_Grad2_18x18_To_8x8_C(SKL_BYTE *Dst, int Dst_BpS,
                             const SKL_BYTE *Src, int Src_BpS)
{
#define LOAD(x,y) (x) = s[-1] + 3*s[0]; (y) = 3*s[1] + s[2]; s += Src_BpS
#define THRESH 24

  int i;

  Src -= Src_BpS;
  for(i=0; i<8; ++i)
  {
    int mx0, mx1, my0, my1, tmpx, tmpy;
    int j;
    const SKL_BYTE *s = Src + 2*i;
    SKL_BYTE *d = Dst + i;

    LOAD(mx0,my0); 
    LOAD(tmpx,tmpy);
    mx0 += 3*tmpx; my0 += 3*tmpy;
    
    for(j=4; j>0; --j)
    {
      LOAD(mx1,my1);   mx0 -= 3*my1; my0 -= 3*mx1;
      LOAD(tmpx,tmpy); mx0 -= tmpy;  my0 -= tmpx;
      mx1 += 3*tmpx; my1 += 3*tmpy;

        // at this point: 
        //   Gx  = (64+mx0-my0)>>7
        //   Gy  = (64-mx0-my0)>>7
        //   => Gx*Gx+Gy*Gy ~= 2*( mx0*mx0 + my0*my0 )

      mx0 = (mx0+32)>>6; my0 = (my0+32)>>6;
      tmpx = mx0*mx0 + my0*my0;

      tmpx = (tmpx>255 ? 255 : tmpx);
      // tmpx = (tmpx>THRESH ? 255 : 0);
      STORE( tmpx );

      LOAD(mx0,my0);   mx1 -= 3*my0; my1 -= 3*mx0;
      LOAD(tmpx,tmpy); mx1 -= tmpy;  my1 -= tmpx;
      mx0 += 3*tmpx; my0 += 3*tmpy;
      mx1 = (mx1+32)>>6; my1 = (my1+32)>>6;
      tmpx = mx1*mx1 + my1*my1;

      tmpx = (tmpx>255 ? 255 : tmpx);
      // tmpx = (tmpx>THRESH ? 255 : 0);
      STORE( tmpx );
    }
  }
#undef THRESH
#undef LOAD
}

#undef STORE
//////////////////////////////////////////////////////////

}   // extern "C"
