/********************************************************
 * Some code. Copyright (C) 2003 by Pascal Massimino.   *
 * All Rights Reserved.      (http://skal.planet-d.net) *
 * For Educational/Academic use ONLY. See 'LICENSE.TXT'.*
 ********************************************************/
/*
 * skl_cmap.cpp
 *
 *  color maps
 ********************************************************/

#include "skl.h"
#include "skl_syst/skl_bits.h"
#include "skl_2d/skl_cmap.h"

//////////////////////////////////////////////////////////
// SKL_CMAP_X
//////////////////////////////////////////////////////////

SKL_CMAP_X::SKL_CMAP_X(const SKL_CMAP &In)
 :  _Nb(In.Get_Nb_Colors())
{
  const SKL_UINT32 *In_Cols = In.Get_Colors();
  SKL_FORMAT_SHIFT shft( In.Get_Format() );
  for( int i=0; i<_Nb; ++i )
    _Map[i] = shft.Unpack(In_Cols[i]);
}

SKL_CMAP_X::SKL_CMAP_X(const SKL_CMAP_X &In)
 :  _Nb(In.Get_Nb_Colors())
{
  for(int i=0; i<_Nb; ++i) _Map[i] = In[i];
}

//////////////////////////////////////////////////////////

void SKL_CMAP_X::Ramp(SKL_COLOR c1, SKL_COLOR c2, 
                      int Start, int End)
{
  SKL_ASSERT(Start>=0); 
  SKL_ASSERT(Start<End);
  SKL_ASSERT(End<=256);
  if (_Nb<End) _Nb=End;
  int N = End-Start;
  if (N<=1) {
    _Map[Start] = c1.Mix(c2, 0.5f);  // !!
    return;
  }
  float Scale = 1.0f/(N-1);
  for( int i=0; i<N; ++i )
    _Map[Start+i] = c1.Mix(c2, Scale*i);
}

void SKL_CMAP_X::RGB_Cube( SKL_FORMAT Fmt )
{
  int r_max = (1<<Fmt.Bits(2))-1;
  int g_max = (1<<Fmt.Bits(1))-1;
  int b_max = (1<<Fmt.Bits(0))-1;

  SKL_ASSERT( (r_max+1)*(g_max+1)*(b_max+1)<=256 );

  int i = 0;
  for( int r=0; r<=r_max; ++r )
    for( int g=0; g<=g_max; ++g )
      for( int b=0; b<=b_max; ++b )
        _Map[i++] = SKL_COLOR( r*255/r_max, g*255/g_max, b*255/b_max );
}   

//////////////////////////////////////////////////////////
// SKL_CMAP
//////////////////////////////////////////////////////////

SKL_CMAP::SKL_CMAP(const SKL_CMAP_X &In, SKL_FORMAT fmt )
 : _Nb(In.Get_Nb_Colors())
 , _Format(fmt) 
{
  SKL_FORMAT_SHIFT shft( fmt );
  const SKL_COLOR *In_Cols = In.Get_Colors();
  for( int i=0; i<_Nb; ++i )
    _Map[i] = shft.Pack(In_Cols[i]);
}

SKL_CMAP::SKL_CMAP(const SKL_CMAP &In, SKL_FORMAT fmt)
{
  _Nb = In.Get_Nb_Colors();
  Set_Format( In.Get_Format() );
  if (fmt==0 || fmt==In.Get_Format() ) {
    SKL_MEMCPY( _Map, In.Get_Colors(), _Nb*sizeof(_Map[0]) );
  }
  else {
    SKL_FORMAT_SHIFT in( In.Get_Format() );
    SKL_FORMAT_SHIFT out( fmt );
    for(int i=0; i<_Nb; ++i) {
      SKL_COLOR C = in.Unpack( In[i] );
      _Map[i] = out.Pack(C);
    }
  }
}

SKL_CMAP::SKL_CMAP(const SKL_FORMAT fmt, int Nb)
  : _Nb(Nb), _Format(fmt)
{
  SKL_ASSERT(Nb>=0); SKL_ASSERT(Nb<=256);
  SKL_BZERO(_Map, _Nb*sizeof(_Map[0]) );
}     

//////////////////////////////////////////////////////////

void SKL_CMAP::Ramp(SKL_COLOR c1, SKL_COLOR c2,
                    int Start, int End)
{
  SKL_CMAP_X tmp; 
  tmp.Ramp(c1,c2,Start,End);
  *this = SKL_CMAP(tmp,_Format);
}

//////////////////////////////////////////////////////////
// Formatting shifts and masks
//////////////////////////////////////////////////////////

SKL_FORMAT::SKL_FORMAT(SKL_ARGB R_msk, SKL_ARGB G_msk, 
                       SKL_ARGB B_msk, SKL_ARGB A_msk,
                       int Depth)
{
   int Pos[4] = { 0 };
   int Size[4] = { 0 };
   int Msk[4] = { B_msk, G_msk, R_msk, A_msk };

   for( int i=0; i<4; ++i ) {
     SKL_UINT32 Tmp = Msk[i];
     if (Tmp) for( ; !(Tmp&1); Tmp>>=1 ) Pos[i]++;
     for( ;(Tmp&1); Tmp>>=1 ) Size[i]++;
   }

  _Bits = (Size[3]<<12) | (Size[2]<<8) | (Size[1]<<4) | (Size[0]<<0);

  if (G_msk>R_msk) _Bits |= BGR_ORDER;

  if (Depth<=0) Depth = Compute_Depth();
  _Bits |= Depth<<16;

//  printf( "Mask RGBA: 0x%.8x 0x%.8x 0x%.8x 0x%.8x  => 0x%.8x\n", Msk[0], Msk[1], Msk[2], Msk[3], _Bits );
}
          
int SKL_FORMAT::Compute_Depth() const {
    // we assume packed pixels
  int d = Bits(0) + Bits(1) + Bits(2) + Bits(3);
  return ( (d+7) & ~7 ) / 8;    // size in byte
}

int SKL_FORMAT::Depth() const {
  int d = Raw_Depth();
  if ( !d ) return Compute_Depth();
  else return d;
}

//////////////////////////////////////////////////////////
// SKL_FORMAT_SHIFT
//////////////////////////////////////////////////////////

void SKL_FORMAT_SHIFT::Store_Mask_And_Shift(const SKL_FORMAT f)
{
         // we assume packed pixels
  int i, t;
  for(i=0, t=0; i<4; ++i) {
    _Mask[i] = SKL_BMASKS::And[f.Bits(i)]<<t;
    t += f.Bits(i);
    _Shift[i] = 32-t;
  }
  if (f.Is_BGR()) {
    SKL_INT32 S = _Shift[0]; _Shift[0] = _Shift[2]; _Shift[2] = S;
    SKL_UINT32 M = _Mask[0]; _Mask[0] = _Mask[2]; _Mask[2] = M;
  }
//  Print_Infos();
}

//////////////////////////////////////////////////////////
// -- debug
//////////////////////////////////////////////////////////

void SKL_FORMAT::Print_Infos() const {
  printf("Format:0x%.5x  (depth:%d/%d)\n",
    _Bits, Compute_Depth(), Depth() );
}

void SKL_FORMAT_SHIFT::Print_Infos() const {
  printf("Shifts: R:%d   G:%d   B:%d (A:%d) \n",
    _Shift[2], _Shift[1], _Shift[0], _Shift[3]);
  printf("Masks:  R:0x%.8x   G:0x%.8x   B:0x%.8x   (A:0x%.8x)\n",
    _Mask[2], _Mask[1], _Mask[0], _Mask[3]);
}

void SKL_FORMAT_SHIFT::Print_Col(SKL_UINT32 c) const {
  SKL_COLOR C = UnpackA(c);
  printf("C=0x%.8x -> r=0x%x  g=0x%x  b=0x%x  (a=0x%x)\n",
    c, C.R(), C.G(), C.B(), C.A() );
}

//////////////////////////////////////////////////////////
