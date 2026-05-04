/********************************************************
 * Some code. Copyright (C) 2003 by Pascal Massimino.   *
 * All Rights Reserved.      (http://skal.planet-d.net) *
 * For Educational/Academic use ONLY. See 'LICENSE.TXT'.*
 ********************************************************/
/*
 * skl_mem_dflt.cpp
 *
 *  Default memory module
 ********************************************************/
 
#include "skl.h"
#include "skl_syst/skl_exception.h"
#include <stdlib.h>

//////////////////////////////////////////////////////////
// SKL_MEM_I
//////////////////////////////////////////////////////////

SKL_MEM_I::SKL_MEM_I() {}
SKL_MEM_I::~SKL_MEM_I() {}

SKL_ANY SKL_MEM_I::New(const size_t s)
{
  if (s==0) Skl_Throw( SKL_MEM_EXCEPTION("(SKL_DFLT_MEM)", s) );
  int Size = (s+7) & ~7; // pad to 8.
  SKL_ANY p = malloc(Size);
#ifndef NDEBUG
  memset( p, 0xde, Size ); // Warning! Risk of Heisenbug!
#endif
  return p;
}

void SKL_MEM_I::Delete(const SKL_ANY p,  size_t s)
{ 
  if (p!=0) {
#ifndef NDEBUG
    memset( p, 0xad, s ); // Warning! Risk of Heisenbug!
#endif
    free(p);
  }
}

//////////////////////////////////////////////////////////

void SKL_MEM_I::Realloc(SKL_ANY &Ptr, 
  const size_t Size, const size_t Old_Size)
{
  SKL_ANY New_Ptr = 0;
  if (Size>0) {
    New_Ptr = New(Size);
    if (Old_Size>0) 
      SKL_MEMCPY(New_Ptr, Ptr, Old_Size); // Grow
    else SKL_MEMCPY(New_Ptr, Ptr, Size);  // Shrink
  }
  Delete( Ptr, Old_Size );
  Ptr = New_Ptr;
}

//////////////////////////////////////////////////////////

static SKL_MEM_I System_Memory;
SKL_MEM_I * const SKL_MEM = &System_Memory;

//////////////////////////////////////////////////////////
