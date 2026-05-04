/********************************************************
 * Some code. Copyright (C) 2003 by Pascal Massimino.   *
 * All Rights Reserved.      (http://skal.planet-d.net) *
 * For Educational/Academic use ONLY. See 'LICENSE.TXT'.*
 ********************************************************/
/*
 * skl_ptimer.cpp
 *
 *  simple "profiling" timer
 ********************************************************/

#include "skl.h"
#include "skl_syst/skl_ptimer.h"

#ifndef _WINDOWS
#include <sys/time.h>
#else
#include <winsock2.h>   // for select()
#endif

//////////////////////////////////////////////////////////
// Timing utilities

void SKL_PTIMER::Elapsed_s(SKL_CST_STRING s,SKL_CST_STRING e)
{
  Stop();
  if (s) printf(s);
  printf( " - elapsed: %.3f s    \r", Get_Sec() );
  if (e) printf(e);
}

void SKL_PTIMER::Elapsed_FPS(SKL_CST_STRING s, SKL_CST_STRING e)
{
  Stop();
  if (s) printf(s);
  float Tmp = Get_Sec();
  if ( Tmp<=0.0 ) return;
  float t = 1.0f*_Count/Tmp;
  printf( " - %ld frames in %.3f sec -> %.3f FPS       \r", _Count, Tmp, t );
  if (e) printf(e);
}

void SKL_PTIMER::Wait(float ms) const
{
  if (ms<=0.) return; /* !! */
#if 0   // BWEUARK!@!
  ms = clock() + (ms/1000.0f)*TICKS;
  while( clock()<ms ) {}
#else
  struct timeval val;
  val.tv_sec = (int)( ms/1000.f );
  val.tv_usec = (int)(1.e3f*ms - 1.e6f*val.tv_sec);
  select(1, 0, 0, 0, &val);
#endif
}

//////////////////////////////////////////////////////////
// SKL_STAT / SKL_CALL_STAT
//////////////////////////////////////////////////////////

  // -- statics

static SKL_CALL_STAT *_All_Stats = 0;

struct SKL_MEGA_STATIC_CALL_STAT  // just to have a static destructor
{
  ~SKL_MEGA_STATIC_CALL_STAT()
  {
    SKL_CALL_STAT *Cur = _All_Stats;
    while(Cur!=0) {
      SKL_CALL_STAT *Next = Cur->Get_Next();
      Cur->Print_Stats();
      ::delete Cur;
      Cur = Next;
    }
    _All_Stats = 0;
  }
};
static SKL_MEGA_STATIC_CALL_STAT All_Stats;
//////////////////////////////////////////////////////////

SKL_CALL_STAT::SKL_CALL_STAT(SKL_CST_STRING Name)
  : _Run_Time(0)
  , _dTicksA(0), _dTicksD(0), _TicksA(0), _TicksD(0)
  , _Calls(0), _tCalls(0)
  , _Next(0)
{
  Set_Name(Name);
}

  // README: we're copying *Name in _Name[] instead
  // of simply storing a pointer to *Name because of Dlls:
  // *Name might not be valid any longer when 
  // time has come to destroy the static _Stats[]
  // and print some infos...

void SKL_CALL_STAT::Set_Name(SKL_CST_STRING Name)
{
  // strncpy is not standard on all libc :(
  int n;
  for(n=0; n<MAX_STR && Name[n]; ++n) _Name[n] = Name[n];
  _Name[n] = 0;
}

SKL_CALL_STAT *SKL_CALL_STAT::Search(SKL_CST_STRING Name)
{
  SKL_CALL_STAT **Prev = &_All_Stats;
  SKL_CALL_STAT *Cur = _All_Stats;
  while(Cur!=0 && strcmp(Cur->_Name, Name)) {
    Prev = &Cur->_Next;
    Cur = Cur->_Next;
  }
  if (Cur==0) { Cur = ::new SKL_CALL_STAT(Name); SKL_ASSERT(Cur!=0); }
  else *Prev = Cur->_Next;
  Cur->_Next = _All_Stats;
  _All_Stats = Cur;
  return Cur;
}

//////////////////////////////////////////////////////////    

void SKL_CALL_STAT::Print_Stats()
{
  fprintf( stderr, "Call Stats for '%s':\n", _Name);
  if (_tCalls)
  {
    fprintf( stderr, " - ticks ........... 0x%.8x%.8x (called %d times)\n",  _dTicksD, _dTicksA, _tCalls);
    float Tmp = 4294967296.0f*_dTicksD/_tCalls;
    Tmp += 1.0f*_dTicksA/_tCalls;
    fprintf( stderr, " - ticks/call ...... %.2f\n", Tmp );
  }
  else if (_Calls) {
    float t = 1000.0f*_Run_Time/TICKS;   // in ms
    fprintf( stderr, " - called .......... %d time(s) -> %4.3f ms\n", _Calls, t );
    fprintf( stderr, " - average/call .... %4.3f ms\n",  t/_Calls );
  }
}

void SKL_CALL_STAT::Record() {
  _Cur = clock();
  _Calls++;
}

void SKL_CALL_STAT::Pause() {
  _Run_Time += clock() - _Cur;
}

#ifdef __LINUX__
static SKL_UINT32 _Eax, _Edx;
inline void SKL_RDTSC_Get() {
asm(".byte 0x0f\t\n"
    ".byte 0x31\t\n"
    "mov %%eax,_Eax\t\n"
    "mov %%edx,_Edx\t\n"
     : : : "eax", "edx");
}
#endif

void SKL_CALL_STAT::Record_Ticks() {
#ifdef __LINUX__
  _tCalls++;
  SKL_RDTSC_Get();
  _TicksA = _Eax; _TicksD = _Edx;
#endif
}

void SKL_CALL_STAT::Pause_Ticks() {
#ifdef __LINUX__
  SKL_RDTSC_Get();
  SKL_UINT32 Eax = _Eax;
  SKL_UINT32 Edx = _Edx;

    // remove overhead (approximatively)
    // on a P120:  Offset ~= 0x4f
    // on a PII: Offset ~= 0xc0
  Eax -= 0x4f;
  if (Eax>_Eax) --Edx;  // underflow

  
  if (Eax<_TicksA) ++Edx; // carry
  _dTicksA += Eax - _TicksA;
  _dTicksD += Edx - _TicksD;
#endif
}

//////////////////////////////////////////////////////////
