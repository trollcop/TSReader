/********************************************************
 * Some code. Copyright (C) 2003 by Pascal Massimino.   *
 * All Rights Reserved.      (http://skal.planet-d.net) *
 * For Educational/Academic use ONLY. See 'LICENSE.TXT'.*
 ********************************************************/
/*
 * showyuv: a simple YUV player
 *
 ********************************************************/

#include "skl_utils.h"
#include "skl_syst/skl_dsp.h"
#include "skl_syst/skl_ptimer.h"
#include "skl_2d/skl_window.h"
#include <math.h>

//////////////////////////////////////////////////////////
//
// -- global params
//
//////////////////////////////////////////////////////////

static SKL_CST_STRING In_File = 0, PSNR_File = 0;
static int YUV_W = 0, YUV_H = 0, YUV_BpS = 0;
static SKL_YUV_DSP YUV_Dsp = {0};
static SKL_UINT32 To_RGB = 0x40888;
static float FPS = 0.;
static SKL_BYTE *Yo = 0, *Uo = 0, *Vo = 0;
static int Transfer_Type = 0;
static SKL_WINDOW *Vid = 0;

//////////////////////////////////////////////////////////
//
// Read and display one frame
//
//////////////////////////////////////////////////////////

static int Show_YUV(FILE *In, SKL_PTIMER &pTimer)
{
  float Next_Tick = pTimer.Get_mSec();

  int k;
  for(k=0; k<YUV_H; ++k)
    if (fread(Yo + k*YUV_BpS, YUV_W, 1, In)!=1)
      return 0;
  for(k=0; k<YUV_H/2; ++k)
    if (fread(Uo + k*YUV_BpS, YUV_W/2, 1, In)!=1)
      return 0;
  for(k=0; k<YUV_H/2; ++k)
    if (fread(Vo + k*YUV_BpS, YUV_W/2, 1, In)!=1)
      return 0;

  SKL_BYTE * const Dst = Vid->Lock();
  const int VBpS = Vid->BpS();

  if (Vid->Format()==0x40888)
    YUV_Dsp.YUV_TO_RGB32(Dst, VBpS, Yo, Uo, Vo, YUV_BpS, YUV_W, YUV_H);
  else if (Vid->Format()==0x30888)
    YUV_Dsp.YUV_TO_RGB24(Dst, VBpS, Yo, Uo, Vo, YUV_BpS, YUV_W, YUV_H);
  else if (Vid->Format()==0x20565)
    YUV_Dsp.YUV_TO_RGB565(Dst, VBpS, Yo, Uo, Vo, YUV_BpS, YUV_W, YUV_H);

  YUV_Dsp.Switch_Off();

  Vid->Unlock();

  if (FPS>0.)
  {
    Next_Tick += 1000.f / FPS - pTimer.Get_mSec();
    if (Next_Tick>0.) pTimer.Wait(Next_Tick);
  }

  return 1;
}

static double Get_SSE(const SKL_BYTE *A, const SKL_BYTE *B, int W)
{
  double Sum = 0.;
  while(W-->0)
    Sum += ((int)A[W] - (int)B[W]) * ((int)A[W] - (int)B[W]);
  return Sum;
}

static double Get_PSNR(double SSE, int Size) {
  return SSE>0. ? -4.3429448*log( 1.*SSE / (255.*255.*Size) ) : 0.;
}

static double Compute_PSNR(FILE *PSNR)
{
  SKL_BYTE Row[2048];
  SKL_ASSERT(YUV_W<=2048);

  int k;
  double SSE_Y = 0., SSE_U = 0., SSE_V = 0.;
  for(k=0; k<YUV_H; ++k)
    if (fread(Row, YUV_W, 1, PSNR)!=1)
      return 0;
    else
      SSE_Y += Get_SSE(Row, Yo + k*YUV_BpS, YUV_W);
  for(k=0; k<YUV_H/2; ++k)
    if (fread(Row, YUV_W/2, 1, PSNR)!=1)
      return 0;
    else
      SSE_U += Get_SSE(Row, Uo + k*YUV_BpS, YUV_W/2);
  for(k=0; k<YUV_H/2; ++k)
    if (fread(Row, YUV_W/2, 1, PSNR)!=1)
      return 0;
    else
      SSE_V += Get_SSE(Row, Vo + k*YUV_BpS, YUV_W/2);

  const double All = Get_PSNR(SSE_Y+SSE_U+SSE_V, 3*YUV_W*YUV_H/2);
  printf( "PSNR:  Y:%.3f   U:%.3f    V:%.3f      All:%.3f\n",
    Get_PSNR(SSE_Y, YUV_W*YUV_H  ),
    Get_PSNR(SSE_U, YUV_W*YUV_H/4),
    Get_PSNR(SSE_V, YUV_W*YUV_H/4),
    All );


  return All;
}

//////////////////////////////////////////////////////////
//
// Options and params
//
//////////////////////////////////////////////////////////

static void Help( char **argv )
{
  printf( " * Usage: %s [options] [width] [height]\n", argv[0] );
  printf( "\n" );

  printf( " * Options:\n" );
  printf( "    -i <name> ....... Input YUV file name (if not stdin)\n" );
  printf( "    -rgb ............ yuv->rgb565 conversion\n" );
  printf( "    -rgb32 .......... yuv->rgb32 conversion\n" );
  printf( "    -tt <int> ....... yuv->rgb Transfer type\n" );
  printf( "    -fps <float> .... target frame/s) (0=free)            [%.3f]\n", FPS );
  printf( "    -h .............. This help.\n" );
  printf( "    -psnr <name> .... Compute PSNR against YUV file <name>\n" );
  printf( "\n" );
  printf( "Dimension can be omitted using the shortcut options:\n" );
  printf( "  -sqcif -qcif -cif -4cif -pal -ntsc -720p -1080p\n" );
  printf( "\n" );
  printf( "Example: ffmpeg -i film.avi -f rawvideo -img yuv pipe: | %s -ntsc\n", argv[0] );
  printf( "\n" );

  exit( 0 );
}

//////////////////////////////////////////////////////////
// Command line parsing

static void Missing( SKL_CST_STRING S )
{
  fprintf( stderr, "Missing value after %s option\n", S);
  exit(-1);
}

static const char *Parse_String_Value(const int argc, const char * const *argv, int &k)
{
  if (++k==argc) Missing( argv[k-1] );
  return argv[k];
}

static int Parse_Int_Value(const int argc, const char * const *argv, int &k,
                           const int Min=0x80000000, const int Max=0x7fffffff)
{
  if (++k==argc) Missing( argv[k-1] );
  int Val = atoi(argv[k]);
  return (Val<Min) ? Min : (Val>Max) ? Max : Val;
}

static float Parse_Float_Value(const int argc, const char * const *argv, int &k,
                               const float Min=-1.e30f, const float Max=1.e30f)
{
  if (++k==argc) Missing( argv[k-1] );
  float Val = (float)atof(argv[k]);
  return (Val<Min) ? Min : (Val>Max) ? Max : Val;
}

static void Parse_Options( int argc, char **argv )
{
  for(int k=1; k<argc; ++k)
  {
    if ( !strcmp( argv[k], "-h" ) || !strcmp( argv[k], "-H" ) )
      Help( argv );
    else if ( !strcmp( argv[k], "-rgb" ) )       To_RGB = 0x20565;
    else if ( !strcmp( argv[k], "-rgb24" ) )     To_RGB = 0x30888;
    else if ( !strcmp( argv[k], "-rgb32" ) )     To_RGB = 0x40888;
    else if ( !strcmp( argv[k], "-i" ) )         In_File       = Parse_String_Value(argc,argv,k);
    else if ( !strcmp( argv[k], "-psnr" ) )      PSNR_File      = Parse_String_Value(argc,argv,k);
    else if ( !strcmp( argv[k], "-tt" ) )        Transfer_Type = Parse_Int_Value(argc,argv, k, 0, 8);
    else if ( !strcmp( argv[k], "-fps" ) )       FPS           = Parse_Float_Value(argc, argv, k, 0.);
    else if ( !strcmp( argv[k], "-sqcif" ) )     { YUV_W = 128; YUV_H = 96; }
    else if ( !strcmp( argv[k], "-qcif" ) )      { YUV_W = 176; YUV_H = 144; }
    else if ( !strcmp( argv[k], "-cif" ) )       { YUV_W = 352; YUV_H = 288; }
    else if ( !strcmp( argv[k], "-4cif" ) )      { YUV_W = 704; YUV_H = 576; }
    else if ( !strcmp( argv[k], "-pal" ) )       { YUV_W = 720; YUV_H = 576; }
    else if ( !strcmp( argv[k], "-ntsc" ) )      { YUV_W = 720; YUV_H = 480; }
    else if ( !strcmp( argv[k], "-720p" ) )      { YUV_W = 1280; YUV_H = 720; }
    else if ( !strcmp( argv[k], "-1080p" ) )     { YUV_W = 1920; YUV_H = 1080; }
    else {
      if (YUV_W==0) YUV_W = atoi(argv[k]);
      else YUV_H = atoi(argv[k]);
    }
  }
}

//////////////////////////////////////////////////////////
//
//   main()
//
//////////////////////////////////////////////////////////

#ifdef _WINDOWS
#include <io.h>
#include <fcntl.h>
#endif

int main(int argc, char *argv[])
{
  int Err = -1;
  SKL_PTIMER pTimer;
  int Nb = 0;
  FILE *F = 0;
  FILE *PSNR_F = 0;

  Parse_Options( argc, argv );

  if (YUV_W<=0 || YUV_H<=0) {
    fprintf( stderr, "Missing input size!!\n\n" );
    Help( argv );
  }

#ifdef _WINDOWS
  _setmode(_fileno(stdin), _O_BINARY);
#endif

  if (In_File==0)
    F = stdin;
  else {
    F = fopen( In_File, "rb" );
    if (F==0) {
      fprintf( stderr, "Can't open input file %s\n", In_File);
      goto End;
    }
  }

  if (PSNR_File) {
    PSNR_F = fopen( PSNR_File, "rb" );
    if (PSNR_F==0) {
      fprintf( stderr, "Can't open PSNR reference file %s\n", PSNR_File);
      goto End;
    }
  }

  YUV_BpS = (YUV_W + 31) & ~31;
  Yo = (SKL_BYTE*)malloc(YUV_BpS*(YUV_H*3/2));
  if (Yo==0) {
    fprintf( stderr, "Cannot malloc %d bytes!\n", YUV_BpS*YUV_H*3/2 );
    goto End;
  }
  Uo = Yo + YUV_BpS*YUV_H;
  Vo = Uo + YUV_BpS/2;

  Vid = Sku_Get_Video(YUV_W, YUV_H, To_RGB);
  if (Vid==0) {
    fprintf( stderr, "Cannot open %dx%d video\n", YUV_W, YUV_H );
    goto End;
  }
  Vid->Set_Name( "YUV Player" );

  Skl_Init_YUV_DSP(&YUV_Dsp);
  YUV_Dsp.Init(Transfer_Type);

  pTimer.Reset();
  while(!feof(F)) {
    if (!Show_YUV(F, pTimer))
      break;  // error
    if (PSNR_F) Compute_PSNR(PSNR_F);
    Nb++;
  }

  printf( "Shown %d frames (%dx%d).\n", Nb, YUV_W, YUV_H );
  Err = 0;

End:
  if (Vid!=0) delete Vid;
  if (F!=stdin) fclose(F);
  if (PSNR_F!=0) fclose(PSNR_F);
  if (Yo!=0) free(Yo);

  return Err;
}

//////////////////////////////////////////////////////////
