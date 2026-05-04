/********************************************************
 * Some code. Copyright (C) 2003 by Pascal Massimino.   *
 * All Rights Reserved.      (http://skal.planet-d.net) *
 * For Educational/Academic use ONLY. See 'LICENSE.TXT'.*
 ********************************************************/
/*
 * skl_dyn_load.cpp
 *
 *  Dynamic lib loader
 * This file must be compiler regardless to whether
 * SKL_USE_DYN_LOAD has been defined or not.
 ********************************************************/
 
#include "skl.h"
#include "skl_syst/skl_dyn_load.h"
#include "skl_syst/skl_exception.h"

//////////////////////////////////////////////////////////
// __UNIX__ system calls
//////////////////////////////////////////////////////////

#if defined(__UNIX__)

#include <dlfcn.h>
// #define OPEN_MODE   RTLD_NOW
#define OPEN_MODE   RTLD_LAZY

const int MAX_STR = 512;
static SKL_CST_STRING Make_Lib_Name(SKL_STRING Out,
                                    SKL_CST_STRING Module)
{
  SKL_ASSERT( strlen(Module)+1+3<MAX_STR);
  sprintf(Out, "%s.so", Module);
  return Out;
}
static SKL_CST_STRING Make_Sym_Name(SKL_STRING Out,
                                    SKL_CST_STRING Name)
{
  return Name;
}
static SKL_CST_STRING Make_Sym_Name2(SKL_STRING Out,
                                     SKL_CST_STRING Name) {
  return 0;
}

static SKL_ANY Open(SKL_CST_STRING Name) {
  return (SKL_ANY)dlopen(Name, OPEN_MODE);
}
static SKL_ANY Get_Sym(SKL_ANY Handle, SKL_CST_STRING Name) {
  return dlsym((void*)Handle, (char*)Name);
}
static void Close(SKL_ANY Handle) { 
  dlclose((void*)Handle);
}

static void Print_Error(SKL_CST_STRING Module) {
  fprintf( stderr, "Error while loading '%s':\n%s\n", Module, dlerror() );
}

#endif  // UNIX

//////////////////////////////////////////////////////////
// _WINDOWS system calls
//////////////////////////////////////////////////////////

#if defined(_WINDOWS)

#include <windows.h>

const int MAX_STR = 512;
static SKL_CST_STRING Make_Lib_Name(SKL_STRING Out,
                                    SKL_CST_STRING Module)
{
  SKL_ASSERT( strlen(Module)+4+1<MAX_STR);
  sprintf(Out, "%s.dll", Module);
  return Out;
}
static SKL_CST_STRING Make_Sym_Name(SKL_STRING Out,
                                    SKL_CST_STRING Name)
{
  return Name;
}
static SKL_CST_STRING Make_Sym_Name2(SKL_STRING Out,
                                     SKL_CST_STRING Name)
{
  SKL_ASSERT( strlen(Name)+1+1<MAX_STR);
  sprintf(Out, "_%s", Name);
  return Out;
}

static SKL_ANY Open(SKL_CST_STRING Name) {
  return (SKL_ANY)LoadLibrary(Name);
}
static SKL_ANY Get_Sym(SKL_ANY Handle, SKL_CST_STRING Name) {
  return (SKL_ANY)GetProcAddress((HMODULE)Handle, (char*)Name);
}
static void Close(SKL_ANY Handle) { 
  FreeLibrary((HMODULE)Handle);
}
static void Print_Error(SKL_CST_STRING Module) {
  fprintf( stderr, "Dynload Error while loading '%s'\n", Module );
}

#endif  // _WINDOWS

//////////////////////////////////////////////////////////
// SKL_DYN_LIB
//////////////////////////////////////////////////////////

SKL_DYN_LOADER::SKL_DYN_LOADER(SKL_CST_STRING Module, 
                               const SKL_INT32 Version,
                               int Load_Now)
  : _Sym_Count(0), _Handle(0), _Module(0), _Version(0)
{
  Set_Target( Module, Version );
  if (Load_Now) Load(); 
}

void SKL_DYN_LOADER::Set_Target(SKL_CST_STRING Module, const SKL_INT32 Version)
{
  Unload();
  _Module = Module;
  _Version = Version;
}

int SKL_DYN_LOADER::Load()
{
  if (Ok()) return 1;

  char Out[MAX_STR];
  _Handle = Open(Make_Lib_Name(Out, _Module));
  if ( _Handle==0 )
    goto Error;
    // TODO: Check version number here...
  return 1;

Error:
  Print_Error(_Module);
  return 0;
}

SKL_ANY SKL_DYN_LOADER::Load_Symbol(SKL_CST_STRING Symbol)
{
  if (!Ok())
    if (!Load())
      return 0;

  char Out[MAX_STR];
  const char *Sym_Name = Make_Sym_Name(Out, Symbol);
  SKL_ANY Sym = (SKL_ANY)Get_Sym(_Handle, Sym_Name);
  if (Sym==0) // second chance
  {
    Sym_Name = Make_Sym_Name2(Out, Symbol);
    if (Sym_Name!=0)
      Sym = (SKL_ANY)Get_Sym(_Handle, Sym_Name);
  }  
  if (Sym==0) {
    Print_Error(_Module);
    return 0;
  }
  _Sym_Count++;
  return Sym;
}

void SKL_DYN_LOADER::Unload() {
  if (Ok() && --_Sym_Count==0) {
    Close(_Handle);
    _Handle = 0;
  }
}

//////////////////////////////////////////////////////////
// SKL_SYMBOL_BUILDER
//////////////////////////////////////////////////////////

void SKL_SYMBOL_MAPPER::Map()
{
  if (!Is_Mapped()) {
    if (!_Lib.Load())
      Skl_Throw( SKL_EXCEPTION( "Dynamic load aborted...\n" ) );
    _New = (void *(*)())_Lib.Load_Symbol(_Symb);
    if (_New==0)
      Skl_Throw( SKL_EXCEPTION( "Dynamic load aborted...\n" ) );
  }
}

void SKL_SYMBOL_MAPPER::Unmap()
{
  _Lib.Unload();
  _New = 0;
}

//////////////////////////////////////////////////////////
