/********************************************************
 * Some code. Copyright (C) 2003 by Pascal Massimino.   *
 * All Rights Reserved.      (http://skal.planet-d.net) *
 * For Educational/Academic use ONLY. See 'LICENSE.TXT'.*
 ********************************************************/
/*
 * skl_cpu_specs.cpp
 *
 *  detect CPU's caps
 ********************************************************/

#include "skl.h"
#include "skl_syst/skl_cpu_specs.h"

//////////////////////////////////////////////////////////

extern "C" {

SKL_CPU_FEATURE Skl_Detect_CPU_Feature()
{
  SKL_CPU_SPECS Specs;

  SKL_CPU_FEATURE CPU = SKL_CPU_C;
#ifdef SKL_USE_ASM
  CPU = SKL_CPU_X86;
#endif

  if      (Specs.Has_SSE2()) CPU = SKL_CPU_SSE2;
  else if (Specs.Has_SSE())  CPU = SKL_CPU_SSE;
  else if (Specs.Has_MMX())  CPU = SKL_CPU_MMX;
  return CPU;
}

void Skl_Switch_None() {}
void Skl_Switch_MMX() {
  SKL_EMMS;
}

SKL_DSP_SWITCH Skl_Get_Switch(SKL_CPU_FEATURE Cpu)
{
  if (Cpu>=SKL_CPU_MMX && Cpu<=SKL_CPU_SSE)
    return Skl_Switch_MMX;
  else
    return Skl_Switch_None;
}

} /* extern "C" */

//////////////////////////////////////////////////////////

int SKL_CPU_SPECS::_SType  = SKL_CPU_SPECS::SKL_UNKNOWN;
int SKL_CPU_SPECS::_SFlags = 0;
char SKL_CPU_SPECS::_Vendor[12+1] = { "Unknown" };
char SKL_CPU_SPECS::_Model[48+1] = { "Unknown" };

//////////////////////////////////////////////////////////

SKL_CPU_SPECS::SKL_CPU_SPECS(int Ignore_Feature)
{
  if (_SType==SKL_UNKNOWN) Detect();
  _Type = _SType;
  _Flags = _SFlags & ~Ignore_Feature;
}

void SKL_CPU_SPECS::Reset() 
{
  _SType = SKL_UNKNOWN;
}

void SKL_CPU_SPECS::Detect()
{
  _SFlags = 0;

#if defined(__ALPHA__)
  _SType = SKL_ALPHA;
#elif defined(__HP__)
  _SType = SKL_HP;
#elif defined(__IRIX__)
  _SType = SKL_SGI;
#elif defined(__SUN_CC__)
  _SType = SKL_SPARC;
#elif defined(__LINUX__) || defined(_WINDOWS)
  _SType = SKL_INTEL;

#ifdef SKL_USE_ASM
  if (Skl_Detect_CPUID()!=0) {
    _SFlags |= SKL_CPUID;
    Skl_Get_CPUID(_Vendor);
    Skl_Get_ModelID(_Model);
    if (!strcmp(_Vendor, "AuthenticAMD")) _SType = SKL_AMD;  // TODO: Cyrix?
      
    if (Skl_Detect_MMX()!=0) _SFlags |= SKL_MMX;
    if (Skl_Detect_3DNOW()!=0) _SFlags |= SKL_3DNOW;

    try
    {
      if (Skl_Detect_SSE()!=0) _SFlags |= SKL_SSE;
      if (Skl_Detect_SSE2()!=0) _SFlags |= SKL_SSE2;
    } // we must check that the OS handles SSE commutation :
    catch(...) {
      _SFlags &= ~(SKL_SSE|SKL_SSE2);  // too bad!
    }
  }
#endif  // SKL_USE_ASM

#endif
}

//////////////////////////////////////////////////////////

void SKL_CPU_SPECS::Print_Infos() const
{
  static SKL_CST_STRING Names[] = {
    "INTEL", "AMD", "K6", "ATHLON", "CYRIX",
    "MIPS", "SPARC", "ALPHA", "HP", "Unknown" };
  static SKL_CST_STRING Exts[] = { 
    "CPUID", "MMX", "SSE", "SSE2",
    "3dNow!", "3dNow2!", "SSE" };

  printf( "CPU type: %s\n", Names[_Type] );
  printf( "Vendor  : %s\n", _Vendor);
  printf( "Model   : %s\n", _Model);
  printf( "Ext.    : " );
  for(int i=0; (1<<i)<SKL_LAST; i++)
    if (_Flags&(1<<i))
      printf( "%s ", Exts[i] );
  printf( "\n" );
}

//////////////////////////////////////////////////////////
