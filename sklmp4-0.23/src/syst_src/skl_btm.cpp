/********************************************************
 * Some code. Copyright (C) 2003 by Pascal Massimino.   *
 * All Rights Reserved.      (http://skal.planet-d.net) *
 * For Educational/Academic use ONLY. See 'LICENSE.TXT'.*
 ********************************************************/
/*
 * skl_btm.cpp
 *
 *   SKL_BTM : base bitmap class
 ********************************************************/

#include "skl.h"
#include "skl_2d/skl_btm.h"
#include "skl_2d/skl_btm_cvrt.h"

//////////////////////////////////////////////////////////
//  Constructor
//////////////////////////////////////////////////////////

SKL_BTM::SKL_BTM(SKL_MEM_I *Mem) {
  _Mem = Mem;
  _W = _H = _BpS = 0;
  _Format = SKL_FORMAT::DFLT;
  _Quantum = _Format.Depth();
  _CMap = 0;
  _Owns_CMap = 0;
  _Data = 0;
  _Data_Size = 0;
}

SKL_BTM::~SKL_BTM() { Reset(); }

void SKL_BTM::Set_Data_Size(size_t Size)
{
  if (_Data_Size!=0) {
    _Mem->Delete(_Data, _Data_Size);
    _Data = 0;
    _Data_Size = 0;
  }
  _Data = (Size>0) ? (SKL_BYTE*)_Mem->New(Size) : 0;
  _Data_Size = Size;
}

void SKL_BTM::Reset()
{
  Clear_CMap();
  Set_Data_Size(0);
  _W = _H = _BpS = 0;
  _Quantum   = 0;
  _Format    = 0;
  _Owns_CMap = 0;
}

void SKL_BTM::Set(int W, int H, SKL_FORMAT Format, int BpS, SKL_CMAP_X *CMap) 
{
  SKL_ASSERT(W>=0 && H>=0);

  if (Format==0) Format  = _Format; // re-use previous format
  Reset();

  _Format    = Format;
  _Quantum   = Format.Depth();
  _CMap      = CMap;
  _Owns_CMap = 0;

  _W = W;
  _H = H;
  if (BpS<=0) _BpS = ( _W*_Quantum + 7 ) & ~7;
  else _BpS = BpS;  // -> don't pad the BpS!

  Set_Data_Size(_H*_BpS);
}

void SKL_BTM::Set_Virtual(int W, int H, SKL_FORMAT Format, 
                          SKL_BYTE *Ptr,
                          int BpS, const SKL_CMAP_X *CMap)
{
  SKL_ASSERT(W>=0 && H>=0);


  if (Format!=0) _Format = Format;
  _Quantum = Format.Depth();
  _CMap = (SKL_CMAP_X*)CMap;
  _Owns_CMap = 0;

  _W = W;
  _H = H;
  if (BpS<=0) _BpS = ( _W*_Quantum + 7 ) & ~7;
  else _BpS = BpS;  // -> don't pad the BpS!

  Set_Data_Size(0);
  _Data = (SKL_BYTE*)Ptr;
}

//////////////////////////////////////////////////////////
// -- copy constructor
//////////////////////////////////////////////////////////

void SKL_BTM::Make_Copy(const SKL_BTM *In)    // hard copy
{
  SKL_ASSERT(In!=0);
  Reset();
  SKL_BYTE *Src = ((SKL_BTM*)In)->Lock();
  if (In->Is_Virtual())  
    Set_Virtual( In->Width(), In->Height(), In->Format(), Src, In->BpS(), 0 );
  else {
    Set( In->Width(), In->Height(), In->Format(), In->BpS() );
    SKL_BYTE *dst = Lock();
    SKL_MEMCPY( dst, Src, In->BpS()*In->Height() );
  }
  Unlock();
  ((SKL_BTM*)In)->Unlock();

  if (In->Has_CMap()) 
    Set_CMap(In->Get_CMap());
}

void SKL_BTM::Make_Virtual_Copy(const SKL_BTM *In)    // virtual copy
{
  SKL_ASSERT(In!=0);
  SKL_BYTE *Ptr = ((SKL_BTM*)In)->Lock();
  ((SKL_BTM*)In)->Unlock();
  Set_Virtual(In->Width(), In->Height(), In->Format(), 
              Ptr, In->BpS(), In->Get_CMap_Ptr() );
}

//////////////////////////////////////////////////////////
// Color maps
//////////////////////////////////////////////////////////

void SKL_BTM::Clear_CMap() { 
  if (_Owns_CMap && _CMap!=0)
    _Mem->Delete( _CMap, sizeof (*_CMap) );
  _CMap = 0;
  _Owns_CMap = 0;
}

void SKL_BTM::New_CMap(int Nb) {
  SKL_ASSERT(_Format==SKL_FORMAT::Colormapped()); // TODO: or Alpha?
  Clear_CMap();
  _CMap = new (_Mem) SKL_CMAP_X(Nb);
  _Owns_CMap = 1;
}

//////////////////////////////////////////////////////////

void SKL_BTM::Set_CMap(const SKL_CMAP_X &CMap)  // hard copy
{
  Clear_CMap();
  _CMap = new (_Mem) SKL_CMAP_X(CMap);
  _Owns_CMap = 1;
  Store_CMap();
}

void SKL_BTM::Set_CMap(const SKL_CMAP_X *CMap)   // ref copy
{
  Clear_CMap();
  _CMap = (SKL_CMAP_X*)CMap;
  _Owns_CMap = 0;
  Store_CMap();
}

//////////////////////////////////////////////////////////
//  misc conversion ops
//////////////////////////////////////////////////////////

void SKL_BTM::Apply_Func(SKL_BTM &Out, const SKL_CVRT_FUNC Func, const SKL_UINT32 *Map)
{
  SKL_BYTE *Dst = Out.Lock();
  if (Dst!=0)   // sometimes, with DDraw, the surface get lost
  {
    SKL_BYTE *Src = SKL_BTM::Lock(); // TODO: <- this is a mess
    Func(Dst, Out.BpS(), Src, BpS(), Width(), Height(), Map);
    Out.Unlock();
    SKL_BTM::Unlock();
  }
}

//////////////////////////////////////////////////////////

void SKL_BTM::Remap(SKL_BTM &Out, SKL_UINT32 Map[256])
{
  SKL_ASSERT(Out.Width()==Width());
  SKL_ASSERT(Out.Height()==Height());
  SKL_CVRT_FUNC Func = Skl_Get_Cvrt_Convert_Ops(1, Quantum()); // RGB8_To_*
  Apply_Func(Out, Func, Map);
}

void SKL_BTM::Copy_To(SKL_BTM &Out)
{
  if (&Out==this) return;
  SKL_ASSERT(Out.Width()==Width());
  SKL_ASSERT(Out.Height()==Height());
  SKL_ASSERT( Out.Quantum() == Quantum() );
  SKL_CVRT_FUNC Func = Skl_Get_Cvrt_Copy_Ops( Quantum() );
  Apply_Func(Out, Func, 0);
}

void SKL_BTM::Convert_To(SKL_BTM &Out, SKL_CONVERTER_MAP *Map)
{
  SKL_CONVERTER_MAP *Real_Map = Map;
  if (Map==0)
     Real_Map = SKL_CONVERTER_MAP::Get_Or_Make_Map(Format(), Out.Format());
  Apply_Func(Out, Real_Map->Get_Func(), Real_Map->Get_Map());

  if (Map==0) Real_Map->Dispose();
}

void SKL_BTM::Clear()
{
  SKL_BYTE *Src = Lock();
  for( int j=Height(); j>0; --j ) {
    SKL_BZERO(Src, bWidth() );
    Src += BpS();
  }
  Unlock();
}

//////////////////////////////////////////////////////////
// -- Debug
//////////////////////////////////////////////////////////

void SKL_BTM::Print_Infos() const {
  printf( "size:%dx%d, format=0x%x, BpS=%d\n",
    Width(), Height(), (SKL_UINT32)Format(), BpS() );
}

//////////////////////////////////////////////////////////
