/********************************************************
 * Some code. Copyright (C) 2003 by Pascal Massimino.   *
 * All Rights Reserved.      (http://skal.planet-d.net) *
 * For Educational/Academic use ONLY. See 'LICENSE.TXT'.*
 ********************************************************/
/*
 * two-pass log file regularizer
 *
 *********************************************************/
/*
 *   This program can be used to generate, from a 1rst-pass
 *   log file, a "roadmap" for the final encoding (2nd-pass).
 * 
 *   Usually, two-pass encoding amounts to the following three steps:
 * 
 *   a)  'tmp4 input.ppm -pass 1 -passfile pass1.log -q 2'
 * 
 *   This will generate a log file 'pass1.log' during the 1rst pass.
 *   The input source (input.ppm) is analyzed only, using a fixed 
 *   quantizer (Note: no bitstream is generated).
 * 
 *   b) 'passreg pass1.log -o pass2.log -tp 50 -v'
 * 
 *   This will generate a road map file 'pass2.log' that aims at
 *   reducing the original size by 50%. Various other options are
 *   available for 'passreg'. For instance: '-ts' to specify a
 *   target size, '-la'/'-ha/ for asymmetric bit distribution, etc...
 * 
 *   c) tmp4 input.ppm -pass 2 -passfile pass2.log -o output.mp4 -trellis -4v 60
 * 
 *   This is the final encoding, using 'pass2.log' to drive the
 *   quality and control the size of the bitstream.. 
 *   Any additional encoding option can be used (trellis, ...):
 *   they needn't be exactly the same than the first pass.
 * 
 * 
 *   Note: all of these are rather experimental, and you're encouraged
 *   to play with options/code, since it's all an external application
 *   from the core codec point of view.
 * 
 * 
 ********************************************************/

#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#ifdef _WINDOWS
#include <io.h>
#include <fcntl.h>
#endif

struct PASS_PARAM
{
  struct PASS_FRAME {
    int Pic_Type;
    float Q, FCode, mMV, dMV;
    int Bytes, Txt_Bytes, MV_Bytes;
    int Avrg;
    float Avrg_mMV, Avrg_dMV;
    int New_Bytes, New_Txt_Bytes;

    void Accumulate(int nMin, int nMax, const PASS_FRAME Frames[]);
  };

  struct PASS_SCENE {
    int Start, Len;
    int Bytes;
    float Q_Avrg;
  };

  const char *In_File, *Out_File;
  int Verbose;
  int Target_Size;
  float Target_Percent;
  float FPS, Bit_Rate;
  int Hi_Amp_Percent;
  int Low_Amp_Percent;
  int Min_Frame, Max_Frame, Nb_Frames;
  int All_Frames, All_Key_Frames;
  int All_Bytes, All_Txt_Bytes, All_MV_Bytes;
  int Win_Size;
  int Log_Type;   // 0: MPEG4, 1:H264
  int Pic_W, Pic_H, Nb_MBs;
  int Pic_Min_Sizes[3];
  int KBoost_Percent;
  int Reg_Method;       // 0: assymetric scale    1: use average  2: scene activity
  float Act_Limit;

  PASS_FRAME *Frames;
  PASS_SCENE *Scenes;

  void Reset();   // default values
  void Cleanup();

  void Compute_Windowed_Average();
  void Regularize_Frames();
  void Setup_New_Quantizers();

  void Parse_In_File();
  void Write_Out_File();

  void Help( char **argv, const char *S=0 );
  void Missing( const char *S );
  int Parse_Int_Value(const int argc, const char * const *argv, int &k,
                      const int Min=-0x7fffff, const int Max=0x7ffffff);
  float Parse_Float_Value(const int argc, const char * const *argv, int &k,
                          const float Min=-1.e30f, const float Max=1.e30f);

  void Parse_Options( int argc, char **argv );

  int Process(int argc, char *argv[]);   // main entry call
};

#define ERROR for( ; 1; throw this )

//////////////////////////////////////////////////////////

void PASS_PARAM::Reset()
{
  Out_File        = 0;
  In_File         = 0;
  Verbose         = 1;
  Target_Size     = 0;
  Target_Percent  = 0.;
  FPS             = 25.f;
  Bit_Rate        = 0;
  Hi_Amp_Percent  = 30;
  Low_Amp_Percent = 10;
  Min_Frame       = 0;
  Max_Frame       =-1;
  Nb_Frames       = 0;
  Win_Size        = 20;
  Log_Type        =-1;
  KBoost_Percent  = 5;

  Reg_Method      = 0;
  Act_Limit       = 0.5f;

  Frames          = 0;
  Scenes          = 0;
}

void PASS_PARAM::Cleanup()
{
  if (Frames) free(Frames);
  if (Scenes) free(Scenes);
  Reset();
}

//////////////////////////////////////////////////////////
// The main work

void PASS_PARAM::PASS_FRAME::Accumulate(int nMin, int nMax, const PASS_FRAME Frames[])
{
  Bytes = 0;
  mMV = 0.;
  dMV = 0.;

  for(int n=nMin; n<nMax; ++n)
  {
    Bytes += Frames[n].Bytes;
    mMV   += Frames[n].mMV;
    dMV   += Frames[n].dMV;
  }
}

void PASS_PARAM::Compute_Windowed_Average()
{
  int n, nMin, nMax;

  PASS_FRAME Avrg;

  n = Min_Frame;
  for(nMin=n; nMin>=0 && nMin>=n-Win_Size; --nMin)
    if (Frames[nMin].Pic_Type==0)
      break;

  for(nMax=n+1; nMax<=Max_Frame && nMax<=n+Win_Size; ++nMax)
    if (Frames[nMax].Pic_Type==0)
      break;

  Avrg.Accumulate(nMin, nMax, Frames);

  while(1)
  {
    const int Nb_Samples = nMax - nMin;
    Frames[n].Avrg     = Avrg.Bytes / Nb_Samples;
    Frames[n].Avrg_mMV = Avrg.mMV   / Nb_Samples;
    Frames[n].Avrg_dMV = Avrg.dMV   / Nb_Samples;
//    printf( "%d %d %d %d %d\n", n, nMin, nMax, Frames[n].Bytes, Frames[n].Avrg );

    if (n==Max_Frame) break;

    if (n-Win_Size==nMin) {
      Avrg.Bytes -= Frames[nMin].Bytes;
      Avrg.mMV   -= Frames[nMin].mMV;
      Avrg.dMV   -= Frames[nMin].dMV;
      nMin++;
    }
    n++;
    if (n+Win_Size==nMax)
    {
      if (nMax<=Max_Frame && Frames[nMax].Pic_Type!=0) {
        Avrg.Bytes += Frames[nMax].Bytes;
        Avrg.mMV   += Frames[nMax].mMV;
        Avrg.dMV   += Frames[nMax].dMV;
        nMax++;
      }
    }
    else if (n==nMax) {
      nMin = nMax;
      for(nMax=nMax+1; nMax<=Max_Frame && nMax<=n+Win_Size; ++nMax)
        if (Frames[nMax].Pic_Type==0)
          break;
      Avrg.Accumulate(nMin, nMax, Frames);
    }    
  }
}

void PASS_PARAM::Regularize_Frames()
{
  int n;

  int Locked_Size = 0, Rest_Size = 0;
  float Scale = Target_Percent/100.f;
  for(n=Min_Frame; n<=Max_Frame; ++n)
  {
    const int Type = Frames[n].Pic_Type;
    double s;
    if (Type==0) {
      s = 1. + KBoost_Percent/100.;
    }
    else {
      if (Reg_Method==0) {
        const int Delta = Frames[n].Bytes - Frames[n].Avrg;
        if (Delta>=0) s = 1. + Hi_Amp_Percent/100.;
        else          s = 1. + Low_Amp_Percent/100.;
      }
      else if (Reg_Method==1) {
        s = 1.;
      }
      else {
        double Activity;
        double mMV = Frames[n].Avrg_mMV + .01;
        double dMV = Frames[n].Avrg_dMV + .01;
        Activity = log(mMV) - Act_Limit*log(dMV);
        if (Activity>0.)
        {
          s = 1. + Activity*Low_Amp_Percent/100.;
        }
        else {
//          Activity = 1. - (Act_Limit*dMV)/mMV;
          s = 1. - Activity*Hi_Amp_Percent/100.;
        }
        if (Verbose>2)
          printf( "%d %f %f %f %f %f %f\n", n, Activity, (float)s,
            Frames[n].Avrg_mMV, Frames[n].mMV,
            Frames[n].Avrg_dMV, Frames[n].dMV );
      }
    }

    int New_Bytes = (Type==0) ? Frames[n].Bytes : Frames[n].Avrg;
    New_Bytes = (int)( s*Scale * New_Bytes );
    if (New_Bytes<Pic_Min_Sizes[Type])
    {
      New_Bytes = Pic_Min_Sizes[Type];
      Frames[n].New_Bytes = New_Bytes;
      Locked_Size += New_Bytes;
    }
    else {
      Frames[n].New_Bytes = -New_Bytes;   // negative value indicates "TODO later"
      Rest_Size += New_Bytes;
    }
  }

  Scale = 1.f * (Target_Size - Locked_Size) / Rest_Size;

  Locked_Size = 0;
  for(n=Min_Frame; n<=Max_Frame; ++n)
  {
    if (Frames[n].New_Bytes<0)
      Frames[n].New_Bytes = (int)( -Frames[n].New_Bytes*Scale );
    Frames[n].New_Txt_Bytes = (int)( Frames[n].Txt_Bytes * 1.*Frames[n].New_Bytes / Frames[n].Bytes );

    Locked_Size += Frames[n].New_Bytes;
  }

  if (Verbose>0) {
    printf( "Final size:     %d bytes  [underflow:%.4f%%]\n", Locked_Size, 100.f*Locked_Size/Target_Size-100.f );
    printf( "Final Bitrate:  %.2f kbps\n", FPS*Locked_Size*8.f/1000.f / Nb_Frames );
    printf( "\n" );
  }
}

void PASS_PARAM::Setup_New_Quantizers()
{
  int n;

  for(n=Min_Frame; n<=Max_Frame; ++n)
  {
    float New_Q = 0.;
    double Amp = 1. * Frames[n].Bytes / Frames[n].New_Bytes;
    if (Log_Type==0) {
      New_Q = (float)( Frames[n].Q * Amp );
      New_Q = (New_Q<1.1f) ? 1.1f : (New_Q>31.5f) ? 31.5f : New_Q;
    }
    else {
      New_Q = (float)( Frames[n].Q * ( 1.0 + 0.112*log( Amp ) ) );
      New_Q = (New_Q<0.1f) ? 0.1f : (New_Q>51.0f) ? 51.0f : New_Q;
    }
    Frames[n].Q = New_Q;
  }
}

//////////////////////////////////////////////////////////
// Read in-file

void PASS_PARAM::Parse_In_File()
{
  int i, n, Nb, s;
  PASS_FRAME F = {0};
  PASS_SCENE *Scene;
  FILE *f;
  char *Mem, *Start;

  f = In_File ? fopen(In_File, "r") : stdin;
  if (f==0) ERROR fprintf( stderr, "Can't open in-file %s\n", In_File );

  fseek(f, 0, SEEK_END);
  const int Size = (int)ftell(f);
  fseek(f, 0, SEEK_SET);
  
  Mem = (char*)malloc(Size+1);
  if (Mem==0) ERROR fprintf( stderr, "Malloc error for reading file (size=%d)\n", Size+1);

  if (fread(Mem, Size, 1, f)!=1) {
    free(Mem);
    ERROR fprintf( stderr, "Error reading in-file!\n" );
  }
  Mem[Size] = 0;
  if (In_File) fclose(f);


  Log_Type = -1;
  All_Frames     = 0;
  All_Key_Frames = 0;
  All_Bytes      = 0;
  All_Txt_Bytes  = 0;
  All_MV_Bytes   = 0;

  Start = Mem;
  for(i=0; i<Size; ++i)
    if (Mem[i]=='\n') {
      Mem[i] = 0;
      if (Start[0]=='#') {
        float Version;
        if (sscanf(Start, "## TMP4 v%f %d x %d\n", &Version, &Pic_W, &Pic_H)==3) Log_Type = 0;
        else if (sscanf(Start, "## TH264 v%f %d x %d\n", &Version, &Pic_W, &Pic_H)==3) Log_Type = 1;
        else if (Verbose>1) printf( "Ignoring comment [%s]\n", Start );
      }
      else {
        All_Frames++;
        int Nb, Type;
        if (sscanf( Start, "%d %d ", &Nb, &Type )!=2)
          ERROR fprintf(stderr, "Can't parse frame #%d\n", All_Frames );
        if (Type==0)
          All_Key_Frames++;
      }
      Start = Mem + i+1;
    }

  if (Log_Type==-1)
    ERROR fprintf( stderr, "Unrecognized log file format (missing '## ...' header)\n" );
  Nb_MBs = Pic_W/16 * Pic_H/16;

  if (Log_Type==0) {
      // TODO: tune.
    Pic_Min_Sizes[0] = ( (Nb_MBs*20) + 250 ) >> 3;
    Pic_Min_Sizes[1] = ( (Nb_MBs* 2) + 100 ) >> 3;
    Pic_Min_Sizes[2] = 1;
  }
  else {
    Pic_Min_Sizes[0] = ( (Nb_MBs* 7) +  13 ) >> 3;
    Pic_Min_Sizes[1] = ( (Nb_MBs* 2) + 100 ) >> 3;
    Pic_Min_Sizes[2] = 8;
  }

    // Finish setting up parameters

  assert(All_Frames>0 && All_Key_Frames>0);
  if (Max_Frame>=All_Frames || Max_Frame<0) Max_Frame = All_Frames-1;
  if (Min_Frame>Max_Frame) Min_Frame = Max_Frame;
  Nb_Frames = (Max_Frame+1 - Min_Frame);



    // allocate work memory

  Frames = (PASS_FRAME*)malloc(All_Frames * sizeof(PASS_FRAME));
  Scenes = (PASS_SCENE*)malloc(All_Key_Frames * sizeof(PASS_SCENE));
  if (Frames==0 || Scenes==0) {
    free(Mem);
    if (Frames) free( Frames );
    ERROR fprintf( stderr, "Malloc error allocating frames/scenes\n" );
  }
  memset(Frames, 0, All_Frames * sizeof(PASS_FRAME));
  memset(Scenes, 0, All_Key_Frames * sizeof(PASS_SCENE));


    // go

  Scene = 0;
  for(n=0, s=0, Start=Mem, i=0; i<Size; ++i)
  {
    if (Mem[i]!=0) continue;
    if (Start[0]!='#')
    {
      if (sscanf(Start, "%d %d %f %f %f %f %d %d %d\n", &Nb, &F.Pic_Type, &F.Q, &F.FCode, &F.mMV, &F.dMV, &F.Bytes, &F.Txt_Bytes, &F.MV_Bytes)!=9)
        ERROR fprintf( stderr, "Parse error for frame #%d\n", n );

      if (Nb<0 || Nb>=All_Frames) 
        ERROR fprintf( stderr, "Frame order is out of range (%d !in [0,%d])\n", Nb, All_Frames-1 );
      if (Frames[Nb].Pic_Type!=0)
        ERROR fprintf( stderr, "Frame #%d is already set!\n", Nb );

      F.Avrg     = F.Bytes;
      F.Avrg_mMV = F.mMV;
      F.Avrg_dMV = F.dMV;

      F.New_Bytes = F.Bytes;
      F.New_Txt_Bytes = F.Txt_Bytes;
      Frames[Nb] = F;

      if (Nb>=Min_Frame && Nb<=Max_Frame)
      {
        All_Bytes     += F.Bytes;
        All_Txt_Bytes += F.Txt_Bytes;
        All_MV_Bytes  += F.MV_Bytes;
      }

      if (F.Bytes<Pic_Min_Sizes[F.Pic_Type]) Pic_Min_Sizes[F.Pic_Type] = F.Bytes;

      if (F.Pic_Type==0)    // Key_Frame
      {
        if (Scene!=0) {
          Scene->Len = Nb - Scene->Start;
          Scene->Q_Avrg *= 1.f / Scene->Len;
          if (Nb>=Min_Frame && n<=Max_Frame)
            Frames[Scene->Start].Q = Scene->Q_Avrg;
        }
        Scene = Scenes + s;
        Scene->Start = Nb;
        Scene->Bytes = 0;
        Scene->Q_Avrg = 0.;
        s++;
      }
      assert(Scene!=0);
      Scene->Bytes += F.Bytes;
      Scene->Q_Avrg += F.Q;

      n++;
    }
    Start = Mem+i+1;
  }
  Scene->Len = n - Scene->Start;
  Scene->Q_Avrg *= 1.f / Scene->Len;
  if (Scene->Start>=Min_Frame && Scene->Start<=Max_Frame)
    Frames[Scene->Start].Q = Scene->Q_Avrg;

  free(Mem);


    // done

  if (Target_Size==0)     Target_Size = All_Bytes;
  if (Target_Percent!=0.) Target_Size = (int)( Target_Percent/100.f * Target_Size);
  if (Bit_Rate!=0.)       Target_Size = (int)( Bit_Rate*1000.f/8.f*Nb_Frames / FPS );

  Target_Percent = 100.f * Target_Size / All_Bytes;
  Bit_Rate = FPS*Target_Size*8.f/1000.f / Nb_Frames;

  if (Verbose>0) {
    printf( "\n" );
    printf( "Log File Type:  %s\n", (Log_Type==0 ? "MPEG4" : "H264") );
    printf( "Frame Size:     %dx%d  [%d frames (from #%d to #%d)]\n", Pic_W, Pic_H, Nb_Frames, Min_Frame, Max_Frame );
    printf( "Total  size:    %d bytes  [Txt:%d, MVs:%d]\n", All_Bytes, All_Txt_Bytes, All_MV_Bytes );
    printf( "Target size:    %d bytes  (%.2f %%, %.1f kbps @ %.2fHz)\n", Target_Size, Target_Percent, Bit_Rate, FPS );
  }
  if (Verbose>1) {
    printf( "Min sizes per pic type:\n  I-Pic: %d\n  P-Pic: %d\n  B-Pic: %d\n",
      Pic_Min_Sizes[0], Pic_Min_Sizes[1], Pic_Min_Sizes[2] );
    printf( "== Scenes ==\n" );
    for( s=0; s<All_Key_Frames; ++s) {
      printf( "   #%4d  [%6d -> %6d] Q_Avrg=%.2f   %d bytes    \t(~%d bytes/frame)\n", 
        s, Scenes[s].Start, Scenes[s].Start+Scenes[s].Len-1,
        Scenes[s].Q_Avrg,
        Scenes[s].Bytes, Scenes[s].Bytes/Scenes[s].Len );
    }
  }
}

//////////////////////////////////////////////////////////
// Write out-file

void PASS_PARAM::Write_Out_File()
{
  FILE *f = 0;
  int n;

  if (Out_File==0)
    return;

  f = fopen(Out_File, "w");
  if (f==0)
    ERROR fprintf( stderr, "Can't open out-file %s\n", Out_File );

  fprintf( f, "## %s v%1.1f %d x %d\n", ((Log_Type==0) ? "TMP4" : "TH264"), 1.f, Pic_W, Pic_H );

  for(n=0; n<All_Frames; ++n) {
    fprintf( f, "%d %d %.2f %.2f %.2f %.2f %d %d %d\n", 
      n, 
      Frames[n].Pic_Type,
      Frames[n].Q,
      Frames[n].FCode, 
      Frames[n].mMV, Frames[n].dMV, 
      Frames[n].New_Bytes, Frames[n].New_Txt_Bytes, Frames[n].MV_Bytes);
  }

  if (f) fclose(f);

  if (Verbose>0)
    printf( "2nd-pass file '%s' written\n", Out_File );
}

//////////////////////////////////////////////////////////
// Command line parsing

void PASS_PARAM::Missing( const char *S )
{
  ERROR fprintf( stderr, "Missing value after %s option\n", S );
}

int PASS_PARAM::Parse_Int_Value(const int argc, const char * const *argv, int &k,
                                const int Min, const int Max)
{
  if (++k==argc) Missing( argv[k-1] );
  int Val = atoi(argv[k]);
  if      (Val<Min) return Min;
  else if (Val>Max) return Max;
  else              return Val;
}

float PASS_PARAM::Parse_Float_Value(const int argc, const char * const *argv, int &k,
                               const float Min, const float Max)
{
  if (++k==argc) Missing( argv[k-1] );
  float Val = (float)atof(argv[k]);
  if      (Val<Min) return Min;
  else if (Val>Max) return Max;
  else              return Val;
}

void PASS_PARAM::Help( char **argv, const char *S )
{
  if (S!=0) printf( S );
  printf( " * Usage: %s [options] [in-file]\n", argv[0] );
  printf( "\n" );
  printf( "    -o <name> ....... 2nd-pass Out file\n" );
  printf( "    -s <int> ........ First frame to process\n" );
  printf( "    -e <int> ........ Last frame to process\n" );
  printf( "    -w <int> ........ Averaging window size        [%d past and future frames]\n", Win_Size );
  printf( "    -ta <float> ..... Low/Hi Activity threshold    [%.3f]\n", Act_Limit );
  printf( "    -ha <int> ....... Hi Amplification percent     [%d %%]\n", Hi_Amp_Percent );
  printf( "    -la <int> ....... Low Amplification percent    [%d %%]\n", Low_Amp_Percent );
  printf( "    -kb <int> ....... Key-frame boost percentage   [%d %%]\n", KBoost_Percent );
  printf( "    -ts <int> ....... Target size in bytes         [%d bytes]\n", Target_Size );
  printf( "    -tp <float> ..... Target size in percent       [%.2f %%]\n", Target_Percent );
  printf( "    -fps <float> .... Frame/sec when using bitrate [%.2f frames/sec]\n", FPS );
  printf( "    -br <float> ..... Target bitrate (0=don't use) [%.2f kbps]\n", Bit_Rate );
  printf( "    -v .............. Verbose report\n" );
  printf( "    -quiet .......... keep quiet\n" );
  printf( "\n" );
  printf( "Regularization methods:\n" );
  printf( "    -assym .......... Assymetric scaling\n" );
  printf( "    -avrg ........... replace by windowed average (default)\n" );
  printf( "    -act ............ Use scene activity  (experimental)\n" );
  printf( "\n" );
  printf( " Note: -ts, -tp, -br options are mutually exclusive.\n" );
  printf( "\n" );

  exit( 0 );
}

void PASS_PARAM::Parse_Options( int argc, char **argv )
{
  //Default_Params();
  for( int k=1; k<argc; ++k )
  {
    if ( !strcmp( argv[k], "-h" ) || !strcmp( argv[k], "-H" ) )
      Help( argv );
    else if ( !strcmp( argv[k], "-quiet" ) )   Verbose         = 0;
    else if ( !strcmp( argv[k], "-v" ) )       Verbose         = 2;
    else if ( !strcmp( argv[k], "-w" ) )       Win_Size        = Parse_Int_Value(argc, argv, k, 0);
    else if ( !strcmp( argv[k], "-s" ) )       Min_Frame       = Parse_Int_Value(argc, argv, k, 0);
    else if ( !strcmp( argv[k], "-e" ) )       Max_Frame       = Parse_Int_Value(argc, argv, k, 0);
    else if ( !strcmp( argv[k], "-ha" ) )      Hi_Amp_Percent  = Parse_Int_Value(argc, argv, k, 0);
    else if ( !strcmp( argv[k], "-la" ) )      Low_Amp_Percent = Parse_Int_Value(argc, argv, k, 0);
    else if ( !strcmp( argv[k], "-kb" ) )      KBoost_Percent  = Parse_Int_Value(argc, argv, k, 0);
    else if ( !strcmp( argv[k], "-ts" ) )    { Target_Size     = Parse_Int_Value(argc, argv, k, 0);           Target_Percent = 0; Bit_Rate = 0; }
    else if ( !strcmp( argv[k], "-tp" ) )    { Target_Percent  = Parse_Float_Value(argc, argv, k, 0., 100.f); Target_Size = 0; Bit_Rate = 0; }
    else if ( !strcmp( argv[k], "-br" ) )    { Bit_Rate        = Parse_Float_Value(argc, argv, k, 0.);        Target_Size = 0; Target_Percent = 0; }
    else if ( !strcmp( argv[k], "-fps" ) )     FPS             = Parse_Float_Value(argc, argv, k, 0.);
    else if ( !strcmp( argv[k], "-act" ) )     Reg_Method      = 2;
    else if ( !strcmp( argv[k], "-avrg" ) )    Reg_Method      = 1;
    else if ( !strcmp( argv[k], "-assym" ) )   Reg_Method      = 0;
    else if ( !strcmp( argv[k], "-ta" ) )      Act_Limit       = Parse_Float_Value(argc, argv, k, 0);
    else if ( !strcmp( argv[k], "-o" ) ) {
      if (++k==argc) Missing(argv[k-1]);
      Out_File = argv[k];
    }
    else if ( argv[k][0] == '-' )
      ERROR fprintf( stderr, "Unknown option '%s' !\n", argv[k]);
    else In_File = argv[k];
  }

  if (In_File==0 && Out_File==0)
    Help(argv, "Missing input file!\n\n" );

  if (Target_Size==0 && Target_Percent == 0. && Bit_Rate==0.)
    fprintf( stderr, "WARNING! No target size specified!\n" );
}

//////////////////////////////////////////////////////////
// Main call

int PASS_PARAM::Process(int argc, char *argv[])
{
#ifdef _WINDOWS
  _setmode(_fileno(stdin), _O_BINARY);    /* thanks to Marcos Morais <morais at dee.ufcg.edu.br> */
#endif

  int Err = 0;

  Reset();
  try {
    Parse_Options( argc, argv );
    Parse_In_File();
    if (Win_Size>0)
      Compute_Windowed_Average();
    Regularize_Frames();
    Setup_New_Quantizers();
    Write_Out_File();
  }
  catch(const PASS_PARAM * const) {
//    fprintf( stderr, "Error occured during 2nd-pass setup.\n" );
    Err = -1;
  }
  Cleanup();

  return Err;
}

//////////////////////////////////////////////////////////
// This entry point is bound to be sent to core codec, soon.

int Skl_Passreg(int argc, char *argv[])
{
  PASS_PARAM Work;
  return Work.Process(argc, argv);
}

//////////////////////////////////////////////////////////
// stand-alone app entry point

int main(int argc, char *argv[])
{
  return Skl_Passreg(argc, argv);
}
//////////////////////////////////////////////////////////
