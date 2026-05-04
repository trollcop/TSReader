/********************************************************
 * Some code. Copyright (C) 2003 by Pascal Massimino.   *
 * All Rights Reserved.      (http://skal.planet-d.net) *
 * For Educational/Academic use ONLY. See 'LICENSE.TXT'.*
 ********************************************************/
/*
 * skl_window.cpp
 *
 *  converter bitmaps & windows
 ********************************************************/

#include "skl.h"
#include "skl_2d/skl_window.h"
#include "skl_syst/skl_exception.h"
#include "skl_syst/skl_event.h"

//////////////////////////////////////////////////////////
// SKL_WINDOW
//////////////////////////////////////////////////////////

SKL_WINDOW::SKL_WINDOW(SKL_MEM_I *Mem, SKL_CST_STRING Name) 
  : SKL_BTM(Mem)
  , _Xo(0)
  , _Yo(0)
  , _Name(Name)
  , _Show(0)
  , _Next(0)
{
  _Event.Reset();
}

SKL_WINDOW::~SKL_WINDOW()
{}

const SKL_EVENT *SKL_WINDOW::Get_Event()
{
  _Event.Reset();
  Get_Event(_Event);
  if (_Event.Is_Empty()) return 0;
  else return &_Event;
}

void SKL_WINDOW::Refresh()
{
  Lock();
  Unlock();
}
    
// Virtual API

void SKL_WINDOW::Print_Infos() const
{
  printf( "Window (%s):\n", (_Name==0) ? "no name" : _Name );
  printf( "Position: %d,%d   Show=%d\n", _Xo, _Yo, _Show );
  SKL_BTM::Print_Infos();
}

//////////////////////////////////////////////////////////
// SKL_BTM_CVRT
//////////////////////////////////////////////////////////

SKL_BTM_CVRT::SKL_BTM_CVRT(SKL_MEM_I *Mem, SKL_FORMAT fmt, SKL_BTM &Dst)
  : SKL_BTM(Mem),
    _Dst(&Dst)
{
  Set(Dst.Width(), Dst.Height(), fmt);
  _Map = SKL_CONVERTER_MAP::Get_Or_Make_Map( fmt, Dst.Format() );
  if (Get_Dst()->Is_Colormapped()) _Map->Store_RGB_Cube( Get_Dst() );
}

SKL_BTM_CVRT::~SKL_BTM_CVRT() { Kill(); }

void SKL_BTM_CVRT::Kill() {
  _Map->Dispose();
  _Map = 0;
  _Dst = 0;
}

void SKL_BTM_CVRT::Store_CMap()
{
    // TODO: Finish (ditherer)
  if (Get_Dst()->Format().Raw_Depth()==1)
    Get_Dst()->Store_CMap();
  else _Map->Convert_And_Store_CMap(*Get_Dst(), Get_CMap());
}

void SKL_BTM_CVRT::Print_Infos() const
{
  printf("Converted bitmap\n");
  printf("Map:");
  _Map->Print_Infos();
  printf("Src:");
  SKL_BTM::Print_Infos();
  printf("Dst:");
  Get_Dst()->Print_Infos();
}

//////////////////////////////////////////////////////////
// SKL_WINDOW_CVRT
//////////////////////////////////////////////////////////

SKL_WINDOW_CVRT::SKL_WINDOW_CVRT(SKL_MEM_I *Mem, SKL_FORMAT fmt, SKL_WINDOW &Dst)
  : SKL_WINDOW(Mem),
    _Dst(&Dst)
{
  Set(Dst.Width(), Dst.Height(), fmt);
  _Map = SKL_CONVERTER_MAP::Get_Or_Make_Map(fmt, Dst.Format());
  if (Get_Dst()->Is_Colormapped()) _Map->Store_RGB_Cube( Get_Dst() );
}

SKL_WINDOW_CVRT::~SKL_WINDOW_CVRT() {
  _Map->Dispose();
  /* delete _Dst? */
}

void SKL_WINDOW_CVRT::Print_Infos() const
{
  printf("Converted window\n");
  printf("Map:");
  _Map->Print_Infos();
  printf("Src:");
  SKL_BTM::Print_Infos();
  printf("Dst:");
  Get_Dst()->Print_Infos();
}

//////////////////////////////////////////////////////////

void SKL_WINDOW_CVRT::Store_CMap()
{
      // TODO: Finish (ditherer)
  if (Get_Dst()->Format().Raw_Depth()==1)
    Get_Dst()->Store_CMap();
  else _Map->Convert_And_Store_CMap(*Get_Dst(), Get_CMap());
}

//////////////////////////////////////////////////////////
