/********************************************************
 * Some code. Copyright (C) 2003 by Pascal Massimino.   *
 * All Rights Reserved.      (http://skal.planet-d.net) *
 * For Educational/Academic use ONLY. See 'LICENSE.TXT'.*
 ********************************************************/
/*
 * skl_cvrt_map.cpp
 *
 * SKL_CONVERTER_MAP
 ********************************************************/

#include "skl.h"
#include "skl_2d/skl_btm_cvrt.h"
#include "skl_syst/skl_exception.h"

//////////////////////////////////////////////////////////
// SKL_CONVERTER_MAP
//////////////////////////////////////////////////////////

SKL_CONVERTER_MAP::SKL_CONVERTER_MAP(SKL_FORMAT In, SKL_FORMAT Out)
  : _In(In), _Out(Out), _Func(0)
{
  if (_In==_Out)
    Init_Copier();
  else if (_In==SKL_FORMAT::Colormapped())
    Init_Colormapper();
  else 
    Init_Converter();
}

void SKL_CONVERTER_MAP::Print_Infos() const {
  printf( "Format In:0x%x Out:0x%x\n", (SKL_UINT32)_In, (SKL_UINT32)_Out);
}

//////////////////////////////////////////////////////////
// 5 situations:
//   Same in/out format : needs a raw copy
//   Colormap->Truecol  : needs indexer
//   Truecol->Truecol   : needs converter
//   Truecol->Colormap  : needs a 0x10332 RGB-cubed colormap
//   Colormap->Colormap : needs color-matcher (not implemented yet)

void SKL_CONVERTER_MAP::Init_Colormapper()
{
  if ( _Out==SKL_FORMAT::Colormapped() )
    Skl_Throw( SKL_EXCEPTION( "Color matcher not implemented" ) );

  _Func = Skl_Get_Cvrt_Convert_Ops( 1, _Out.Depth() );
}

void SKL_CONVERTER_MAP::Init_Copier()
{
  SKL_ASSERT(_In==_Out);
  _Func = Skl_Get_Cvrt_Copy_Ops( _Out.Depth() );
}

void SKL_CONVERTER_MAP::Init_Converter()
{
  if (_Out==SKL_FORMAT::Colormapped()) // install RGB-cube 332...
    _Out = 0x10332;

  _Func = Skl_Get_Cvrt_Convert_Ops(_In.Depth(), _Out.Depth());
  Make_Convert_Table(_In, _Out, _Map[0], _Map[1], _Map[2], _Map[3]);
}

//////////////////////////////////////////////////////////

void SKL_CONVERTER_MAP::Make_Convert_Table(SKL_FORMAT In, SKL_FORMAT Out,
                                           SKL_UINT32 *M1, SKL_UINT32 *M2,
                                           SKL_UINT32 *M3, SKL_UINT32 *M4 )
{
  SKL_ASSERT(In!=SKL_FORMAT::Colormapped() && Out!=SKL_FORMAT::Colormapped());

  SKL_FORMAT_SHIFT in(In);
  SKL_FORMAT_SHIFT out(Out);

  for(int i=0; i<256; ++i ) {
    SKL_ARGB Tmp;
    if (M1!=0) {
      Tmp = in.Unpack(i<<0);
      M1[i] = out.Pack(Tmp);
    }
    if (M2!=0) {
      Tmp = in.Unpack(i<<8);
      M2[i] = out.Pack(Tmp);
    }
    if (M3!=0) {
      Tmp = in.Unpack(i<<16);
      M3[i] = out.Pack(Tmp);
    }
    if (M4!=0) {
      Tmp = in.Unpack(i<<24);
      M4[i] = out.Pack(Tmp);
    }
  }

//  for(i=0; i<Size; ++i ) printf( "0x%x: 0x%.8x 0x%.8x 0x%.8x  0x%.8x\n", i, M1[i], M2[i], M3[i], M4[i] );
}

void SKL_CONVERTER_MAP::Convert_And_Store_CMap(SKL_BTM &Dst, 
                                               const SKL_CMAP_X &CMap)
{
  SKL_ASSERT(!Dst.Is_Colormapped());
  SKL_FORMAT_SHIFT out(Dst.Format());
  for(int i=0; i<CMap.Get_Nb_Colors(); ++i)
  {
    SKL_ARGB Tmp = CMap[i];
    _Map[0][i] = out.Pack( Tmp );
  }
}

void SKL_CONVERTER_MAP::Store_RGB_Cube(SKL_BTM *Dst)
{
    // TODO: this is a hack.
    // best would be to retrieve Dst's cmap and adapt...
  SKL_CMAP_X Map_X;
  Map_X.RGB_Cube( Get_Out_Format() );
//  printf( "Installing 332-RGB-Cube...\n" );
  Dst->Set_CMap(Map_X);
  Dst->Store_CMap();
}

//////////////////////////////////////////////////////////
// multi-user management (->share cvrt tables for same in/out)

SKL_CONVERTER_MAP *
SKL_CONVERTER_MAP::Get_Or_Make_Map(SKL_FORMAT In, SKL_FORMAT Out)
{
  SKL_CONVERTER_MAP *New = new (SKL_MEM) SKL_CONVERTER_MAP(In, Out);
  return New;
}

void SKL_CONVERTER_MAP::Dispose() {
  SKL_MEM->Delete(this, sizeof(*this));  // wow!
}

//////////////////////////////////////////////////////////
