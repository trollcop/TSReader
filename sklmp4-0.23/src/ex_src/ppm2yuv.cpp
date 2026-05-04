/********************************************************
 * Some code. Copyright (C) 2003 by Pascal Massimino.   *
 * All Rights Reserved.      (http://skal.planet-d.net) *
 * For Educational/Academic use ONLY. See 'LICENSE.TXT'.*
 ********************************************************/
/*
 * small RGB -> YUV converter
 *
 ********************************************************/

#include "skl_mpg4_c.h"

#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[])
{
  if (argc<=2) {
    printf( "Usage :%s [in PPM] [out PGM]\n", argv[0] );
    return -1;
  }
  const char *In_Name  = argv[1];
  const char *Out_Name = argv[2];

  FILE *In  = fopen(In_Name, "rb");
  if (In==0) {
    printf( "Error opening In file [%s]\n", In_Name );
    return -1;
  }
  FILE *Out = fopen(Out_Name, "wb");
  if (Out==0) {
    printf( "Error opening Out file [%s]\n", Out_Name );
    fclose(In);
    return -1;
  }

  SKL_YUV_DSP Dsp;
  Skl_Init_YUV_DSP(&Dsp, SKL_CPU_DETECT);

  int W=0, H=0;
  unsigned char *RGB = 0, *YUV = 0;
  
  while(1)
  {
    int w,h;
    if (fscanf(In, "P6\n%d %d\n255\n", &w, &h)!=2)
      break;
    if (W>0 && (w!=W || h!=H)) {
      printf( "Size changed from %dx%d to %dx%d!\n", W,H, w,h );
      break;
    }
    W = w;
    H = h;
      
    if (RGB==0) {
      printf( "Input size: %dx%d\n", W, H);
      RGB = (unsigned char *)malloc(W*H*3);
      YUV = (unsigned char *)malloc(W*H*3/2);
      if (RGB==0 || YUV==0) {
        printf( "Memory allocation error!\n" );
        break;
      }
    }

    if (fread(RGB, 3*W*H, 1, In)!=1)
      break;
    
    Dsp.RGB24_TO_YUV(YUV, YUV+W*H, YUV+W*H+W/2, W, RGB,3*W, W,H);
    fprintf(Out, "P5\n%d %d\n255\n", W, H*3/2);
    fwrite(YUV, W*H*3/2, 1, Out);    
  }
  if (RGB!=0) free(RGB);
  if (YUV!=0) free(YUV);
  fclose(In);
  fclose(Out);
  return 0;
}
