/********************************************************
 * Some code. Copyright (C) 2003 by Pascal Massimino.   *
 * All Rights Reserved.      (http://skal.planet-d.net) *
 * For Educational/Academic use ONLY. See 'LICENSE.TXT'.*
 ********************************************************/
/*
 * skl_mem_trc.cpp
 *
 * simple memory trace
 ********************************************************/
 
#include <stdlib.h>
#include "skl.h"
#include "skl_syst/skl_mem_trc.h"
#include "skl_syst/skl_exception.h"

#define PAD_SIZE(s) (((s)+7) & ~7)

//////////////////////////////////////////////////////////
// impl.
//////////////////////////////////////////////////////////

SKL_MEM_TRC_I::SKL_MEM_TRC_I(int Verbose) 
  : SKL_MEM_I()
  , _Max_Slots(0)
  , _Slots(0)
  , _Verbose(Verbose)
{ 
  Reset();
}
 
SKL_MEM_TRC_I::~SKL_MEM_TRC_I()
{ 
  if (_Last_Slot) Print_Infos();
  ::delete [] _Slots;
  _Last_Slot = 0;
  _Max_Slots = 0;
  _Slots = 0;
}

void SKL_MEM_TRC_I::Set_Verbose(int Level) {
  _Verbose = Level;
}

int SKL_MEM_TRC_I::Grow()
{
  const int New_Max = (_Max_Slots==0) ? 100 : _Max_Slots*2;
  SLOT *New_Slots = ::new SLOT[New_Max];
  if (New_Slots==0) {
    if (_Verbose>0)
      printf( "Malloc failure during grow (%d bytes)", New_Max*sizeof(SLOT) );
    return 0;
  }
  SKL_MEMCPY( New_Slots, _Slots, _Max_Slots*sizeof(SLOT) );
  ::delete [] _Slots;
  _Slots = New_Slots;
  _Max_Slots = New_Max;
  return 1;
}

SKL_ANY SKL_MEM_TRC_I::New(const size_t s)
{
  if (s==0) return 0;
  const int Size = PAD_SIZE(s);  // pad to 8. 
  SKL_ANY p = malloc( Size );
  if (p==0) Skl_Throw( SKL_MEM_EXCEPTION("SKL_MEM_TRC", Size) );
#ifndef NDEBUG
  memset( p, 0xde, Size ); // Warning! Risk of Heisenbug!
#endif
  _Allocated += Size;
  if ( _Allocated>_Max ) _Max = _Allocated;
  if ( _Last_Slot>=_Max_Slots ) {
    if (!Grow())  
      return p;
  }
  if (_Verbose>1)
    printf( "Malloc'd %d bytes \tat %8p. \t\tTotal:%d\n", Size, p, _Allocated );

  _Slots[ _Last_Slot ]._Ptr = p;
  _Slots[ _Last_Slot ]._Size = Size;
  _Last_Slot++;
  return p;
}

void SKL_MEM_TRC_I::Delete(const SKL_ANY p, size_t Size)
{
  if (p==0) return;
  int i;
  for( i=_Last_Slot-1; i>=0; --i )
    if ( _Slots[i]._Ptr==p ) break;
  if (i<0) {
    if (_Verbose>0)
      printf( " Ptr %8p not found in _Delete()\n", p );
  }
  else {
    Size = PAD_SIZE(Size);
    if (Size!=_Slots[i]._Size)
      printf( " Size %d doesn't match stored one %d!\n", Size, _Slots[i]._Size );
    _Allocated -= _Slots[i]._Size;
    if (_Verbose>1)
      printf( "Free'd %d bytes \tat %8p. \t\tTotal:%d\n",
                     _Slots[i]._Size, _Slots[i]._Ptr, _Allocated );
    _Last_Slot--;
    _Slots[i] = _Slots[ _Last_Slot ];
    _Slots[ _Last_Slot ]._Ptr  = 0;
    _Slots[ _Last_Slot ]._Size = 0;
  }
#ifndef NDEBUG
  memset( p, 0xad, Size); // Warning! Risk of Heisenbug!
#endif
  free( p );
}

void SKL_MEM_TRC_I::Reset() {
  for( int i=0; i<_Max_Slots; ++i ) {
    _Slots[i]._Ptr  = 0;
    _Slots[i]._Size = 0;
  }
  _Allocated = _Max = _Last_Slot = 0;
}

//////////////////////////////////////////////////////////

void SKL_MEM_TRC_I::Print_Infos() const
{
  if (_Verbose==0) return;
  printf( " Nb. Allocated: \t\t%d bytes\n", _Allocated );
  printf( " Nb. Max alloc: \t\t%d bytes\n", _Max );
  printf( " Left slots   :    \t\t%d.\n", _Last_Slot );
  printf( " Max slots    :    \t\t%d.\n", _Max_Slots );
}

//////////////////////////////////////////////////////////
