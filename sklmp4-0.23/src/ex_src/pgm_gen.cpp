/********************************************************
 * Some code. Copyright (C) 2003 by Pascal Massimino.   *
 * All Rights Reserved.      (http://skal.planet-d.net) *
 * For Educational/Academic use ONLY. See 'LICENSE.TXT'.*
 ********************************************************/
//////////////////////////////////////////////////////////
// pgm_gen.cpp
//
//  (strange) video input generator (for debugging)
//////////////////////////////////////////////////////////

#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "skl.h"
#include "skl_syst/skl_random.h"

int Out_YUV = 0;
#define WIDTH  256
#define HEIGHT 256
SKL_BYTE Y_Pic[HEIGHT  ][WIDTH  ];
SKL_BYTE U_Pic[HEIGHT/2][WIDTH/2];
SKL_BYTE V_Pic[HEIGHT/2][WIDTH/2];

SKL_BYTE Y_Ref[HEIGHT  ][WIDTH  ];
SKL_BYTE U_Ref[HEIGHT/2][WIDTH/2];
SKL_BYTE V_Ref[HEIGHT/2][WIDTH/2];

typedef struct {
  int x, y;
} VECT;

//////////////////////////////////////////////////////////

static void PGM_Out()
{
  int k;

  if (Out_YUV) {
    for(k=0; k<HEIGHT; ++k)
      fwrite(Y_Pic[k], WIDTH, 1, stdout);
    for(k=0; k<HEIGHT/2; ++k)
      fwrite(U_Pic[k], WIDTH/2, 1, stdout);
    for(k=0; k<HEIGHT/2; ++k)
      fwrite(V_Pic[k], WIDTH/2, 1, stdout);
  }
  else {
    fprintf(stdout, "P5\n\n%d %d\n255\n", WIDTH, HEIGHT*3/2 );
    for(k=0; k<HEIGHT; ++k)
      fwrite(Y_Pic[k], WIDTH, 1, stdout);
    for(k=0; k<HEIGHT/2; ++k) {
      fwrite(U_Pic[k], WIDTH/2, 1, stdout);
      fwrite(V_Pic[k], WIDTH/2, 1, stdout);
    }
  }
}

//////////////////////////////////////////////////////////
// Pseudo Warp

#define FIX 8
VECT Pts[4];
VECT Po, dU, dV, dw;

#define ln_PREC 2
#define PREC  (1<<ln_PREC)        // -th pel unit


static void Build_Texture()
{
  int x,y;
  for(y=0; y<HEIGHT; ++y)
    for(x=0; x<WIDTH; ++x)
      Y_Ref[y][x] = (x^y)&0xff;
  for(y=0; y<HEIGHT/2; ++y)
    for(x=0; x<WIDTH/2; ++x) {
      U_Ref[y][x] = (4*(x^y)+ 25)&0xff;
      V_Ref[y][x] = (4*(x^y)+113)&0xff;
    }
}

static void Random_Pts(float t)
{
  static const float Amp[4] = { 16.530f, 16.123f,   38.641f, 6.245f };
  static const float Phi[4] = {  0.500f,  1.100f,    2.000f, 0.427f };
  static const float Omg[4] = {  1.5f,    1.230f,    0.650f, 0.427f };
  static const VECT  Po[4] = { {-10,-10}, {10,-10}, {-10, 10}, { 10,10 } };
  int i;
  for(i=0; i<4; ++i) {
    double x, y;
    x = Amp[i]*cos(Omg[i]*t + Phi[i]) + Po[i].x;
    y = Amp[i]*sin(Omg[i]*t + Phi[i]) + Po[i].y;
    Pts[i].x = (int)( x*PREC );
    Pts[i].y = (int)( y*PREC );
//    fprintf( stderr, "#%d (%d,%d)\n", i, Pts[i].x, Pts[i].y );
  }
}

static void Invert_Matrix(int Nb_Pts)
{
  const int SCALEX = WIDTH << FIX;
  const int SCALEY = HEIGHT << FIX;

    // convert to vectors in PREC-th pel unit
  Pts[1].x = Pts[1].x +  PREC*WIDTH;
  Pts[1].y = Pts[1].y +           0;
  Pts[2].x = Pts[2].x +           0;
  Pts[2].y = Pts[2].y + PREC*HEIGHT;
  Pts[3].x = Pts[3].x + Pts[2].x + Pts[1].x;
  Pts[3].y = Pts[3].y + Pts[2].y + Pts[1].y;

  Po.x = Pts[0].x;
  Po.y = Pts[0].y;
  switch(Nb_Pts) {
    case 1:
      dU.x = PREC*(1<<FIX); dU.y = 0;
      dV.x = 0;             dV.y = PREC*(1<<FIX);
    break;
    case 2:
    {
      int Norm = (Pts[1].x*Pts[1].x + Pts[1].y*Pts[1].y) / (PREC*PREC);
      if (Norm==0) Norm = 1;  // ahem...
      dU.x = (Pts[1].x*SCALEX)/Norm;  dU.y = (Pts[1].y*SCALEY)/Norm;
      dV.x =-(Pts[1].y*SCALEX)/Norm;  dV.y = (Pts[1].x*SCALEY)/Norm;
    }
    break;
    case 3:
    {
      int Det = (Pts[1].x*Pts[2].y - Pts[2].x*Pts[1].y) / (PREC*PREC);
      if (Det==0) Det = 1;  // ahem...
      dU.x = (Pts[2].y*SCALEX)/Det;  dU.y =-(Pts[2].x*SCALEY)/Det;
      dV.x =-(Pts[1].y*SCALEX)/Det;  dV.y = (Pts[1].x*SCALEY)/Det;
    }
    break;
    case 4:
      dU.x = 0; dU.y = 0;
      dV.x = 0; dV.y = 0;
    break;
  }
}

#define ONE (1<<(ln_PREC+FIX))
static void Warp(int Nb_Pts)
{
  int x, y;
  int uo, vo, u, v;

  if (Nb_Pts<4)
  {
    uo = -(dU.x*Po.x + dU.y*Po.y) >> ln_PREC;
    vo = -(dV.x*Po.x + dV.y*Po.y) >> ln_PREC;
    for(y=0; y<HEIGHT; ++y) {
      u = uo; v = vo;
      uo += dU.y; vo += dV.y;
      for(x=0; x<WIDTH; ++x) {
        int xx = u >> (ln_PREC+FIX);
        int yy = v >> (ln_PREC+FIX);
        if (xx<0) xx = 0;
        else if (xx>=WIDTH) xx = WIDTH-1;
        if (yy<0) yy = 0;
        else if (yy>=HEIGHT) yy = HEIGHT-1;
        //Y_Pic[y][x] = Y_Ref[yy][xx];
        const int ex = u & (ONE-1);
        const int ey = v & (ONE-1);
        Y_Pic[y][x] = ( (ONE-ex)*(ONE-ey)*Y_Ref[yy][xx] + ex*(ONE-ey)*Y_Ref[yy][xx+1]
                    + (ONE-ex)*ey*Y_Ref[yy+1][xx] + ex*ey*Y_Ref[yy+1][xx+1] ) >> (2*(ln_PREC+FIX));
        u += dU.x;
        v += dV.x;
      }
    }
    uo = -(Po.x/2*dU.x + Po.y/2*dU.y) >> ln_PREC;
    vo = -(Po.x/2*dV.x + Po.y/2*dV.y) >> ln_PREC;
    for(y=0; y<HEIGHT/2; ++y) {
      u = uo; v = vo;
      uo += dU.y; vo += dV.y;
      for(x=0; x<WIDTH/2; ++x) {
        int xx = u >> (ln_PREC+FIX);
        int yy = v >> (ln_PREC+FIX);
        if (xx<0) xx = 0;
        else if (xx>=WIDTH/2) xx = WIDTH/2-1;
        if (yy<0) yy = 0;
        else if (yy>=HEIGHT/2) yy = HEIGHT/2-1;
        //U_Pic[y][x] = U_Ref[yy][xx];
        //V_Pic[y][x] = V_Ref[yy][xx];
        const int ex = u & (ONE-1);
        const int ey = v & (ONE-1);
        U_Pic[y][x] = ( (ONE-ex)*(ONE-ey)*U_Ref[yy][xx] + ex*(ONE-ey)*U_Ref[yy][xx+1]
                    + (ONE-ex)*ey*U_Ref[yy+1][xx] + ex*ey*U_Ref[yy+1][xx+1] ) >> (2*(ln_PREC+FIX));
        V_Pic[y][x] = ( (ONE-ex)*(ONE-ey)*V_Ref[yy][xx] + ex*(ONE-ey)*V_Ref[yy][xx+1]
                    + (ONE-ex)*ey*V_Ref[yy+1][xx] + ex*ey*V_Ref[yy+1][xx+1] ) >> (2*(ln_PREC+FIX));
        u += dU.x;
        v += dV.x;
      }
    }
  }
}

//////////////////////////////////////////////////////////
// Scroll (for GMC)

void Make_Scroll(float t, int Param, int N)
{
  int x, y;

  for(y=0; y<HEIGHT; ++y)
  {
    if (Param==0)
      for(x=0; x<WIDTH;++x) Y_Pic[y][x] = Y_Ref[y][(x-N+20*WIDTH)%WIDTH];
    else
      for(x=0; x<WIDTH;++x) Y_Pic[y][x] = Y_Ref[(y-N+20*HEIGHT)%HEIGHT][x];
  }
  for(y=0; y<HEIGHT/2; ++y) {
    for(x=0; x<WIDTH/2; ++x) {
      U_Pic[y][x] = 0x80;
      V_Pic[y][x] = 0x80;
    }
  }
}

//////////////////////////////////////////////////////////
// Debug output

const int MAX_SQ = 80;
static int Sqx[MAX_SQ] = {0};
static int Sqy[MAX_SQ] = {0};
static int SqS[MAX_SQ] = {0};
static int SqC[MAX_SQ] = {0};
static int SpeedC[MAX_SQ] = {0};
void Make_Squares(float t, int Param, int N)
{
  SKL_RANDOM rnd(1+Param+N);
  int x, y, n;
  if (t==0.) {
    for(n=0; n<MAX_SQ; ++n) {
      Sqx[n] = rnd.Get_Int(WIDTH-40);
      Sqy[n] = rnd.Get_Int(HEIGHT-40);
      SqS[n] = 8+rnd.Get_Int(30);
      SqC[n] = rnd.Get_Int(0xffffff) | 0x306060;
      SpeedC[n] = rnd.Get_Int(33) - 16;
    }
  }
  for(n=0; n<MAX_SQ; ++n) {
    switch(n&3) {
      case 0: Sqx[n] = (Sqx[n]+SpeedC[n] + WIDTH-40 )%(WIDTH-40); break;
      case 1: Sqx[n] = (Sqx[n]-SpeedC[n] + WIDTH-40 )%(WIDTH-40); break;
      case 2: Sqy[n] = (Sqy[n]-SpeedC[n] + HEIGHT-40)%(HEIGHT-40); break;
      case 3: Sqy[n] = (Sqy[n]+SpeedC[n] + HEIGHT-40)%(HEIGHT-40); break;
    }
  }
  int Yo,Uo,Vo;
  int Phase = (int)(t/0.05f);
  if (Phase&1) { Yo = 0x00; Uo = 0x00; Vo = 0xff; }
  else         { Yo = 0x00; Uo = 0xff; Vo = 0x00; }
  for(y=0; y<HEIGHT; ++y)
    for(x=0; x<WIDTH; ++x)
      Y_Pic[y][x] = Yo;
  for(y=0; y<HEIGHT/2; ++y) {
      for(x=0; x<WIDTH/2; ++x) {
      U_Pic[y][x] = Uo;
      V_Pic[y][x] = Vo;
    }
  }
  for(n=0; n<MAX_SQ; ++n) {
    int xo = Sqx[n];
    int yo = Sqy[n];
    int H  = SqS[n];
    int yIncr = 1;
    if (Param==0) {
      H /=2; yIncr = 2;
      if (N&1) yo += 1;
    }
    for(y=0; y<H; y+=yIncr) 
      for(x=0; x<SqS[n]; ++x) 
        Y_Pic[yo+y][xo+x] = SqC[n]&0xff;
    for(y=0; y<SqS[n]/2; ++y) 
      for(x=0; x<SqS[n]/2; ++x) {
        U_Pic[(yo+1)/2+y][(xo+1)/2+x] = (SqC[n]>> 8)&0xff;
        V_Pic[(yo+1)/2+y][(xo+1)/2+x] = (SqC[n]>>16)&0xff;
      }
  }
}

//////////////////////////////////////////////////////////
// Zoom / Rotate

void Make_Zoom(float t, int Param, int N)
{
  SKL_ASSERT(WIDTH>=256 && HEIGHT>=256);
  int x, y;
  if (Param) {
    for(y=0; y<HEIGHT; ++y)
      for(x=0; x<WIDTH; ++x) {
        float u = 2.f*(x-WIDTH/2+.5f)/WIDTH;
        float v = 2.f*(y-HEIGHT/2+.5f)/HEIGHT;
        int uu = (int)(3.7*N+WIDTH*sqrt(u*u+v*v)) % WIDTH;
        int vv = (int)( (.5 - atan2(v,u)/2./3.14159265 )*HEIGHT );
        Y_Pic[y][x] = Y_Ref[uu][vv];
      }
  }
  else {
    for(y=0; y<HEIGHT; ++y)
      for(x=0; x<WIDTH; ++x) {
        float u = 2.f*(x-WIDTH/2+.5f)/WIDTH;
        float v = 2.f*(y-HEIGHT/2+.5f)/HEIGHT;
        int uu = (int)(HEIGHT*sqrt(u*u+v*v)) % HEIGHT;
        int vv = (int)( (.5 - atan2(v,u)/2./3.14159265 )*WIDTH + 3.7*N ) % WIDTH;
        Y_Pic[y][x] = Y_Ref[uu][vv];
      }
  }
  for(y=0; y<HEIGHT/2; ++y) {
    for(x=0; x<WIDTH/2; ++x) {
      U_Pic[y][x] = 0x80;
      V_Pic[y][x] = 0x80;
    }
  }
}

//////////////////////////////////////////////////////////
// Pattern for testing luminance masking

void Make_LM(float t, int Param, int N)
{
  SKL_RANDOM rnd(1+Param+N);
  int x, y;
  for(y=0; y<HEIGHT; ++y)
    for(x=0; x<WIDTH; ++x) {
      int C;
      if (Param==0) C = y + rnd.Get_Int(2*x+1) - x;
      else C = y + (((x+y)&1) ? (-x/2) : (x/2) );
      if (C>255) C = 255;
      else if (C<0) C = 0;
      Y_Pic[y][x] = C;
    }
  for(y=0; y<HEIGHT/2; ++y) {
    for(x=0; x<WIDTH/2; ++x) {
      U_Pic[y][x] = 0x80;
      V_Pic[y][x] = 0x80;
    }
  }
}

//////////////////////////////////////////////////////////

static void Help(const char * Exe) {
  printf( "Usage: %s [options]\n", Exe );
  printf( "options are:\n" );
  printf( " -h ............ this help\n" );
  printf( " -n <int> ...... number of frames to generate\n" );
  printf( " -t <int> ...... output type (0:warp, 1:random, etc.)\n" );
  printf( " -p <int> ...... param for output (nb warp points, ...)\n" );
  printf( " -yuv .......... output is raw YUV 4:2:0\n" );
  printf( "\n" );
  exit(0);
}

static int Check(int i, int max) {
  if (i==max) {
    fprintf( stderr, "Missing param!\n" );
    exit(1);
  }
  return i;
}

int main(int argc, const char * const argv[])
{
  int i, Type=0, Nb_Frames = 100;
  int Param = 3;
  float t = 0.;

  for(i=1; i<argc; ++i) {
    if (!strcmp(argv[i], "-h"))
      Help(argv[0]);
    if (!strcmp(argv[i], "-yuv"))
      Out_YUV = 1;
    else if (!strcmp(argv[i],"-n"))
      Nb_Frames = atoi(argv[Check(++i,argc)]);
    else if (!strcmp(argv[i],"-p"))
      Param = atoi(argv[Check(++i,argc)]);
    else if (!strcmp(argv[i],"-t"))
      Type = atoi(argv[Check(++i,argc)]);
  }

  Build_Texture();
  if (Type==0) {
    if (Param>4) Param = 4;
    else if (Param<1) Param = 1;
  }
  else if (Type==1) {}
  else if (Type==2) {}
  else if (Type==3) {}

  int n = 0;
  while(Nb_Frames-->0)
  {

    if (Type==0) {
      Random_Pts(t);
      Invert_Matrix(Param);
      Warp(Param);
    }
    else if (Type==1) {
      Make_Scroll(t,Param, n);
    }
    else if (Type==2) {
      Make_Squares(t,Param, n);
    }
    else if (Type==3) {
      Make_Zoom(t,Param, n);
    }
    else if (Type==4) {
      Make_LM(t,Param, n);
    }
    PGM_Out();
    t += .05f;
    n++;
  }
  return 0;
}

//////////////////////////////////////////////////////////
