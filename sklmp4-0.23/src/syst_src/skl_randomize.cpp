/********************************************************
 * Some code. Copyright (C) 2003 by Pascal Massimino.   *
 * All Rights Reserved.      (http://skal.planet-d.net) *
 * For Educational/Academic use ONLY. See 'LICENSE.TXT'.*
 ********************************************************/
/*
 * skl_randomize.cpp
 *
 * Rand utilities
 ***********************************************/

#include "skl.h"
#include "skl_syst/skl_random.h"
#include <math.h>

//////////////////////////////////////////////////////////
// SKL_RANDOM
//////////////////////////////////////////////////////////

#define UPPER_MASK 0x80000000
#define LOWER_MASK 0x7fffffff

  // Tempering parameters
#define TEMPERING_MASK_B 0x9d2c5680
#define TEMPERING_MASK_C 0xefc60000
#define TEMPERING_SHIFT_U(y)  (y >> 11)
#define TEMPERING_SHIFT_S(y)  (y << 7)
#define TEMPERING_SHIFT_T(y)  (y << 15)
#define TEMPERING_SHIFT_L(y)  (y >> 18)

void SKL_RANDOM::Set_Seed(SKL_UINT32 s)
{
    // generator used: Line 25 of Table 1 in
    // [KNUTH 1981, The Art of Computer Programming
    //    Vol. 2 (2nd Ed.), pp102]

  for (_n=N; _n>0; _n--) {
    _V[_n-1] = s;
    s = (69069 * s) & 0xffffffff;
  }
}

void SKL_RANDOM::Freshen()
{
  static const SKL_UINT32 M01[] = {0x0, 0x9908b0df};    // constant vector A

  SKL_UINT32 *V = (SKL_UINT32*)_V;
  SKL_INT32 kk;
  for ( kk=0; kk<N-M; kk++)
  {
    SKL_UINT32 y = ( V[kk] & UPPER_MASK ) | ( V[kk+1] & LOWER_MASK );
    V[kk] = V[kk+M] ^ (y >> 1) ^ M01[y & 0x1];
  }
  for ( ; kk<N-1; kk++)
  {
    SKL_UINT32 y = ( V[kk] & UPPER_MASK ) | ( V[kk+1] & LOWER_MASK );
    V[kk] = V[kk+(M-N)] ^ (y >> 1) ^ M01[y & 0x1];
  }
  SKL_UINT32 y = ( V[N-1] & UPPER_MASK ) | ( V[0] & LOWER_MASK );
  V[N-1] = V[M-1] ^ (y >> 1) ^ M01[y & 0x1];

  _n = N;
}

SKL_UINT32 SKL_RANDOM::Get()
{
  if (Is_Used_Up()) Freshen();

  SKL_UINT32 y = _V[--_n];
  y ^= TEMPERING_SHIFT_U(y);
  y ^= TEMPERING_SHIFT_S(y) & TEMPERING_MASK_B;
  y ^= TEMPERING_SHIFT_T(y) & TEMPERING_MASK_C;
  y ^= TEMPERING_SHIFT_L(y);

  return y;
}

float SKL_RANDOM::Gauss(float Amp)
{
  float x = 1.0f - Get_Float();
  SKL_ASSERT(x>.0 && x<=1.0f);
  x = (float)( Amp*sqrt(-log(x)) );
  return ( Get()&0x01 ) ? -x : x;
}

//////////////////////////////////////////////////////////
