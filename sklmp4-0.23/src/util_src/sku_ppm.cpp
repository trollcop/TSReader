/********************************************************
 * Some code. Copyright (C) 2003 by Pascal Massimino.   *
 * All Rights Reserved.      (http://skal.planet-d.net) *
 * For Educational/Academic use ONLY. See 'LICENSE.TXT'.*
 ********************************************************/
/*
 * skl_ppm.cpp
 *
 * ppm loader/saver
 ********************************************************/

#include "skl.h"
#include "skl_syst/skl_exception.h"
#include "skl_2d/skl_btm.h"
#include <stdio.h>

static const SKL_BYTE SKL_PPM_Head[] = { 'P', '6', '\n' };
static const SKL_BYTE SKL_PGM_Head[] = { 'P', '5', '\n' };

//////////////////////////////////////////////////////////

void Sku_Save_PPM(SKL_CST_STRING Name, SKL_BTM &Btm, int Use_PGM)
{
  FILE *File = fopen( Name, "wb" );
  if (File==0)
    Skl_Throw( SKL_MSG_EXCEPTION( "Save PPM: can't open out file '%s'.", Name ) );

  if (!Use_PGM) fwrite( SKL_PPM_Head, sizeof(SKL_PPM_Head), 1, File );
  else fwrite( SKL_PGM_Head, sizeof(SKL_PPM_Head), 1, File );
  SKL_BYTE Buf[32];
  sprintf( (char*)Buf, "%d %d\n255\n", Btm.Width(), Btm.Height() );
  fwrite( Buf, strlen((char*)Buf), 1, File );
  SKL_BYTE *Src = (SKL_BYTE*)Btm.Lock();
  for(int j=0; j<Btm.Height(); ++j) {
    for( int i=0; i<Btm.Width(); i++ )
    {
      int r=0, g=0, b=0;
      if (Btm.Is_Colormapped()) {
        int n = Src[i];
        r = Btm.Get_CMap()[n].R();
        g = Btm.Get_CMap()[n].G();
        b = Btm.Get_CMap()[n].B();
      }
      else if (Btm.Format()==0x20565)
      {
        SKL_UINT16 C = ((SKL_UINT16*)Src)[i];
        r = SKL_RGB_565_TO_R(C);
        g = SKL_RGB_565_TO_G(C);
        b = SKL_RGB_565_TO_B(C);
      }
      else if (Btm.Format()==0x20555)
      {
        SKL_UINT16 C = ((SKL_UINT16*)Src)[i];
        r = SKL_RGB_555_TO_R(C);
        g = SKL_RGB_555_TO_G(C);
        b = SKL_RGB_555_TO_B(C);
      }
      else if (Btm.Format()==0x30888) {
        r = Src[3*i+2];   // TODO: Endian!?!?
        g = Src[3*i+1];
        b = Src[3*i+0];
      }
      else if (Btm.Format()==0x40888) {
        r = Src[4*i+2];   // TODO: Endian!?!?
        g = Src[4*i+1];
        b = Src[4*i+0];
      }
      else
        Skl_Throw( SKL_EXCEPTION( "Save PPM: unsupported BTM format" ) );
      if (Use_PGM) {
        int v = (r+g+b) / 3;
        fputc( v, File );
      }
      else {
        fputc( r, File );
        fputc( g, File );
        fputc( b, File );
      }
    }
    Src += Btm.BpS();
  }
  fclose( File );
  Btm.Unlock();
}

//////////////////////////////////////////////////////////
