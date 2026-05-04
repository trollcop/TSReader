/********************************************************
 * Some code. Copyright (C) 2003 by Pascal Massimino.   *
 * All Rights Reserved.      (http://skal.planet-d.net) *
 * For Educational/Academic use ONLY. See 'LICENSE.TXT'.*
 ********************************************************/
/*
 * sku_show.cpp
 *
 *  Quick utility: shows a bitmap
 ***************************************************/

#include "skl_utils.h"
#include "skl_syst/skl_exception.h"
#include "skl_video.h"
#include "skl_syst/skl_dyn_load.h"

#ifdef SKL_SYSTEM_VIDEO_DRIVER
SKL_DECLARE_DYN_LIBRARY(SKL_VIDEO_DLL);
SKL_DECLARE_DYN_OBJECT(SKL_SYSTEM_VIDEO_DRIVER);
SKL_DYN_LIBRARY(SKL_VIDEO_DLL, SKL_SYSTEM_VIDEO_LIB, 0100);
SKL_DYN_OBJECT(SKL_SYSTEM_VIDEO_DRIVER, SKL_VIDEO_DLL);
#else

#error "no default video driver defined (SKL_SYSTEM_VIDEO_DRIVER)"

#endif

//////////////////////////////////////////////////////////

static SKL_VIDEO_I *Drv = 0;
static SKL_WINDOW *Video = 0;
static SKL_WINDOW *Video_CMap = 0;

//////////////////////////////////////////////////////////

void Sku_Show_Close()
{
  if (Video!=0) {
    delete Video;
    Video = 0;
  }
  if (Video_CMap!=0) {
    delete Video_CMap;
    Video_CMap = 0;
  }
}

SKL_WINDOW *Sku_Get_Video(int Width, int Height, SKL_FORMAT Fmt)
{
  if (Width==0) { // close all
    Sku_Show_Close();
    return 0;
  }

  if (Drv==0)
    Drv = SKL_DYN_INSTANCE(SKL_SYSTEM_VIDEO_DRIVER, SKL_VIDEO_I);
    //Sku_Init_System_Video_Driver();

  if ( Video==0 ||
       Video->Width()  != Width  ||
       Video->Height() != Height ||
       Video->Format() != Fmt )
  {
    if (Video!=0) delete Video;
    Video = Drv->Create_Window(Width, Height, Fmt);
    if (Video==0)
      Skl_Throw( SKL_MSG_EXCEPTION( "Unable to Create_Window()\n" ) );
    Video->Set_Name( "Video" );
  }
  return Video;
}

void Sku_Show_Pic(SKL_BTM *Pic)
{
  if (Pic==0) { // close all
    Sku_Show_Close();
    return;
  }
  Video = Sku_Get_Video(Pic->Width(), Pic->Height(), Pic->Format());

  if (Pic->Is_Colormapped() || Pic->Is_Alpha())
  {
    if ( Pic->Get_Nb_Colors()==0 )
    {
         // default CMap
      SKL_CMAP_X CMap;
      CMap.Ramp( SKL_COLOR_BLACK, SKL_COLOR_WHITE );
      Video->Set_CMap(CMap);
    }
    else {
      Video->Set_CMap(Pic->Get_CMap());
    }    
    Video->Store_CMap();
  }
  Pic->Copy_To( *Video );
  Video->Refresh();
}

//////////////////////////////////////////////////////////

void Sku_Dump_Pic(SKL_BTM *Pic)
{
  static int cnt = 0;
  char buf[256];
  if (Pic->Is_Colormapped())
  {
    sprintf( buf, "snap%.3d.pgm", cnt++);
    Sku_Save_PPM(buf, *Pic, 1);
  }
  else {
    sprintf( buf, "snap%.3d.ppm", cnt++);
    Sku_Save_PPM(buf, *Pic, 0);
  }
  printf("Dumped '%s'.\n", buf);
}

//////////////////////////////////////////////////////////
