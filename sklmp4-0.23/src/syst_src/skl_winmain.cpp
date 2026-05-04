/********************************************************
 * Some code. Copyright (C) 2003 by Pascal Massimino.   *
 * All Rights Reserved.      (http://skal.planet-d.net) *
 * For Educational/Academic use ONLY. See 'LICENSE.TXT'.*
 ********************************************************/
/*
 * skl_winmain.cpp
 *
 *  Windows entry point
 ********************************************************/

#ifdef _WINDOWS

#include "skl.h"
#include "skl_syst/skl_syswin.h"

#include <stdlib.h>

extern "C" int main(int argc, char *argv[]);

//////////////////////////////////////////////////////////
// Win $#@#@! main call
//////////////////////////////////////////////////////////

int WINAPI WinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance, 
    LPSTR lpCmdLine, int nCmdShow )
{
  if ( hPrevInstance!=0 ) return 0;
  SKL_WIN_SYSTEM::Init( hInstance );

   // thanks Nic (at everwicked dot com) for the __argc/v trick!
  int ret = main( __argc, __argv );

  SKL_WIN_SYSTEM::Finish();
  return ret;
}

//////////////////////////////////////////////////////////

#endif   // _WINDOWS
