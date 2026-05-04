/********************************************************
 * Some code. Copyright (C) 2003 by Pascal Massimino.   *
 * All Rights Reserved.      (http://skal.planet-d.net) *
 * For Educational/Academic use ONLY. See 'LICENSE.TXT'.*
 ********************************************************/
/*
 * skl_video.cpp
 *
 * video driver interface
 ********************************************************/

//#define SKL_DEBUG

#include "skl_video.h"

#ifndef SKL_NO_VIDEO

#include "skl_syst/skl_exception.h"

//////////////////////////////////////////////////////////
// SKL_VIDEO_I
//////////////////////////////////////////////////////////

SKL_VIDEO_I::SKL_VIDEO_I(SKL_MEM_I *Mem, SKL_CST_STRING Name)
  : SKL_DRIVER_I(Name)
  , _Mem(Mem)
  , _Modes(0)
  , _Nb_Modes(0)
  , _Max_Modes(0)
  , _Fullscreen_Mode_Nb(-1)
  , _Fullscreen_Mode(0)
  , _Windows(0)
  , _Caps(NONE)
{}

SKL_VIDEO_I::~SKL_VIDEO_I()
{
  if (!Ok()) return;
  Clear_Modes();
  Close_All_Windows();
}

void SKL_VIDEO_I::Close_All_Windows()
{
  while(_Windows!=0) {
    SKL_WINDOW *W = _Windows;
    _Windows = W->Get_Next();
    delete W;
  }
}

//////////////////////////////////////////////////////////
// fullscreen modes API
//////////////////////////////////////////////////////////

SKL_WINDOW *SKL_VIDEO_I::Set_Fullscreen_Mode_Internal(int Mode_Nb)
{
  if (!Ok()) return 0;
  if (Mode_Nb<0 || Mode_Nb>=Get_Nb_Modes()) return 0;

  SKL_WINDOW *New_Mode = Set_Fullscreen_Mode( _Modes[Mode_Nb], 1 );
  if (New_Mode==0) return 0;

  delete _Fullscreen_Mode;
  _Fullscreen_Mode = New_Mode;
  _Fullscreen_Mode_Nb = Mode_Nb;

  return _Fullscreen_Mode;
}

//////////////////////////////////////////////////////////

SKL_WINDOW *SKL_VIDEO_I::Fullscreen_Mode(int Mode_Nb)
{
  return Set_Fullscreen_Mode_Internal( Mode_Nb );
}

SKL_WINDOW *SKL_VIDEO_I::Fullscreen_Mode(int Width,  
                                         int Height, 
                                         SKL_FORMAT Fmt)
{
  return Fullscreen_Mode( Search_Best_Mode(Width, Height, Fmt) );
}

void SKL_VIDEO_I::Close_Fullscreen()
{
  if (!Ok()) return;
  if (_Fullscreen_Mode==0) return;
  Shutdown_Fullscreen();
  delete _Fullscreen_Mode;
  _Fullscreen_Mode = 0;
  _Fullscreen_Mode_Nb = -1;
}

//////////////////////////////////////////////////////////
// windows API
//////////////////////////////////////////////////////////

int SKL_VIDEO_I::Needs_Conversion(const SKL_WINDOW *Win,
                                  SKL_FORMAT Fmt) const
{
  SKL_ASSERT(Win!=0);
  return !Win->Format().Is_Compatible_With(Fmt);
}

SKL_WINDOW *SKL_VIDEO_I::Create_Window(int W, int H,
                                       SKL_FORMAT Fmt,
                                       int With_Backbuffer,
                                       int Show)
{
  SKL_ASSERT(W>0 && H>0);
  if (!Ok()) return 0;
  if (With_Backbuffer && !Has_Backbuffer()) return 0;

  SKL_WINDOW *Win = Open_Window( W, H, Fmt, With_Backbuffer, Show );
  if (Win==0) return 0;

  if (Needs_Conversion(Win,Fmt)) // TODO: || Win->Is_Direct_Video_RAM()
  {
    SKL_WINDOW *Cvrt = new SKL_WINDOW_CVRT(SKL_MEM, Fmt, *Win);
    if (Cvrt==0) { 
      delete Win;
      return 0;
    }
    Win = Cvrt; // wow! on-fly substitution
  }

  Win->Set_Next( _Windows );
  _Windows = Win;

  return Win;
}

//////////////////////////////////////////////////////////
// Managment of fullscreen modes
//////////////////////////////////////////////////////////

int SKL_VIDEO_I::Is_Better_Than(const SKL_BTM * const a,const SKL_BTM * const b) const
{
  if (a->Format()>b->Format()) return 1;
  else if (a->Format()<b->Format()) return 0;
  if (a->Width()>b->Width()) return 1;
  else if (a->Width()<b->Width()) return 0;
  if (a->Height()>b->Height()) return 1;
  else if (a->Height()<b->Height()) return 0;
  return 0;
}

int SKL_VIDEO_I::Search_Best_Mode(int w, int h, SKL_FORMAT Fmt) const
{
    // Rules: if there's some video modes with exactly the
    // same pixel format, return the widest of such modes.
    // Otherwise, return the mode with greatest format and size,
    // in this order.

  if (Get_Nb_Modes()==0) return -1;
  int Best = -1;
  SKL_BTM Cmp(0);
  Cmp.Set_Virtual(w, h, Fmt);

  int i;
  for(i=0; i<Get_Nb_Modes(); ++i ) {
    const SKL_BTM *M = _Modes[i];
    if (M->Format()==Cmp.Format())
      if (Is_Better_Than(M, &Cmp))
        Best = i;
  }
  if (Best!=-1) return Best;
  Best = 0;
  for(i=1; i<Get_Nb_Modes(); ++i ) {
    const SKL_BTM *M = _Modes[i];
    if (Is_Better_Than(M, &Cmp))
      Best = i;
  }
  return Best;
}

const SKL_BTM *SKL_VIDEO_I::Get_Mode_Infos(int Nb) const {
  if (Nb<0 || Nb>=Get_Nb_Modes()) return 0;
  return _Modes[Nb];
}

void SKL_VIDEO_I::Add_Mode(SKL_BTM *Mode)
{
    // Modes are sorted by decreasing Format/Width/Height
  int i;
  for(i=0; i<Get_Nb_Modes(); ++i)
    if ( Is_Better_Than(_Modes[i], Mode) )
      break;

  if (_Nb_Modes==_Max_Modes)    // grow ?
  {
    int New_Max = (_Max_Modes==0) ? 8 : 2*_Max_Modes;
    SKL_BTM **New_Modes = (SKL_BTM**)SKL_MEM->New( New_Max * sizeof(*_Modes));
    if (New_Modes==0)
      Skl_Throw( SKL_MEM_EXCEPTION( "Video Modes", New_Max * sizeof(*_Modes)) );
    if (_Nb_Modes>0) {
      SKL_ASSERT(_Modes!=0);
      SKL_MEMCPY( New_Modes, _Modes, _Nb_Modes*sizeof(*_Modes));
      SKL_MEM->Delete(_Modes, _Max_Modes*sizeof(*_Modes));
    }
    _Modes = New_Modes;
    _Max_Modes = New_Max;
  }
  if (i<_Nb_Modes)
    SKL_MEMMOVE(_Modes+i+1, _Modes+i, (_Nb_Modes-i)*sizeof(*_Modes));
  _Modes[i] = Mode;
  _Nb_Modes++;
}

void SKL_VIDEO_I::Clear_Modes() {
  if (_Fullscreen_Mode) Close_Fullscreen();
  for(int i=0; i<Get_Nb_Modes(); ++i )
    delete _Modes[i];
  if (_Max_Modes>0) {
    SKL_MEM->Delete(_Modes, _Max_Modes*sizeof(*_Modes));    
    _Max_Modes = 0;
    _Modes = 0;
  }
  _Nb_Modes = 0;
}

//////////////////////////////////////////////////////////
// Infos
//////////////////////////////////////////////////////////

void SKL_VIDEO_I::Print_Modes_Infos() const
{
  printf( "Nb Modes: %d\n", Get_Nb_Modes() );
  for( int i=0; i<Get_Nb_Modes(); ++i )
    _Modes[i]->Print_Infos();
  if (Is_Fullscreen()) {
    printf( "Current Fullscreen Mode (#%d):\n", _Fullscreen_Mode_Nb);
    Get_Fullscreen_Mode()->Print_Infos();
  }
}

void SKL_VIDEO_I::Print_Windows_Infos() const
{
  for(SKL_WINDOW *Win = _Windows; Win!=0; Win = Win->Get_Next())
    Win->Print_Infos();
}

void SKL_VIDEO_I::Print_Infos() const
{
  printf( "Video driver '%s':", Get_Name() );
  if (!Ok()) printf( " Uninitialized.\n" );
  else {
    printf( "\n");
    Print_Modes_Infos();
    // Print_Windows_Infos();    
  }
}

//////////////////////////////////////////////////////////
// -- wrapper for params set/get
//////////////////////////////////////////////////////////

int SKL_VIDEO_I::Set_String_Param(PARAM Opt,
                                  SKL_CST_STRING value)
{ 
  return Set_Param(Opt, value, 0);
}

int SKL_VIDEO_I::Set_Int_Param(PARAM Opt, int value)
{ 
  return Set_Param(Opt, 0, value);
}
    
SKL_CST_STRING SKL_VIDEO_I::Get_String_Param(PARAM Opt) const
{
  SKL_CST_STRING Ret;
  if (!Get_Param(Opt, &Ret, 0)) return 0;
  return Ret;
}

int SKL_VIDEO_I::Get_Int_Param(PARAM Opt) const
{
  int Ret;
  if (!Get_Param(Opt, 0, &Ret)) return 0;
  return Ret;
}

//////////////////////////////////////////////////////////

#endif  /* SKL_NO_VIDEO */
