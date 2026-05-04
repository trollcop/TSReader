/********************************************************
 * Some code. Copyright (C) 2003 by Pascal Massimino.   *
 * All Rights Reserved.      (http://skal.planet-d.net) *
 * For Educational/Academic use ONLY. See 'LICENSE.TXT'.*
 ********************************************************/
/*
 * MPEG4 codec front end example
 *
 ********************************************************/

#include "skl_utils.h"
#include "skl_syst/skl_mpg4.h"
#include "skl_syst/skl_dyn_load.h"
#include "skl_syst/skl_exception.h"
#include "skl_syst/skl_ptimer.h"
#include "skl_syst/skl_mem_trc.h"
#include "skl_syst/skl_dsp.h"
#include "skl_syst/skl_cpu_specs.h"
#include "skl_2d/skl_btm.h"
#include "skl_2d/skl_window.h"
#include <math.h>

//////////////////////////////////////////////////////////
// DLL declaration for codecs
//////////////////////////////////////////////////////////

    // plug-in to use for codec

#ifndef _WINDOWS
#define SKL_MP4_DLL_NAME "libskl_mp4"
#else
#define SKL_MP4_DLL_NAME "skl_drv_mpg"
#endif

SKL_DYN_LIBRARY(SKL_MP4_DLL, SKL_MP4_DLL_NAME, 0100);

    // DLL symbols to map
SKL_MP4_NEW_DEC MP4_New_Decoder;
SKL_MP4_DELETE_DEC MP4_Delete_Decoder;
SKL_MP4_NEW_ENC MP4_New_Encoder;
SKL_MP4_DELETE_ENC MP4_Delete_Encoder;

//////////////////////////////////////////////////////////
//
// -- global params
//
//////////////////////////////////////////////////////////

#define VERSION 0.23f

int Save, Show, With_Slices, From_Mem, Trace_Mem, MPEG12;
int To_RGB;
int Nb_Frames, Skip_Frames;
int Streaming;
int Sequence_Codes;
long Jump_To;
int Global_Q;
int Interlace_DCT;     // 0=disabled, 1=decide, 2=force
int Interlace_Field;   // 0=disabled, 1=decide, 2=force
int Sub_Pixel;
float dQuant_Amp;
float Lambda;
int Inter_Thresh;
int Use_Trellis;
int Intra_Max;
int Inter4V_Probing, Field_Pred_Probing;
int SAD_Skip_Limit;
int SAD_Intra_Limit;
int Search_Metric;
int MV_Size;
int Enc_Buf_Size;
int Use_GMC;
int Hi_Mem;
int GMC_Accuracy;
int Use_Reduced;
int Quant_Type;
int Bit_Rate;
float FPS;
int System_Stream;    // 0: none, 1:AVI, 2:MPG
int Search_Method;
int Intra_Limit;
int Delay;
int VTrace, Debug, ADebug, Quiet;
int Show_PSNR;
int Transfer_Type;
int Edge_Detect;
int YUV_Input, YUV_W, YUV_H;
SKL_CST_STRING In_File, Out_File, Pass_File, Ref_YUV_File;
SKL_CPU_FEATURE Cpu;
SKL_IMG_DSP Img_Dsp = {0};
SKL_YUV_DSP YUV_Dsp = {0};
int Pass_Nb;
float Pass_RF;
const char *Custom_Matrix = 0;
SKL_MEM_I *All_Mem = 0;   //  memory pool

//////////////////////////////////////////////////////////
//
// YUV/PGM input and output
//
//////////////////////////////////////////////////////////

static void YUV_Out(SKL_MP4_PIC *Pic)
{
  SKL_BYTE *Src = Pic->Y;
  int k = Pic->Height;
  while(k-->0) {
    fwrite(Src, Pic->Width, 1, stdout);
    Src += Pic->BpS;
  }

  Src = Pic->U;
  for(k = Pic->Height/2; k>0; --k) {
    fwrite(Src, Pic->Width/2, 1, stdout);
    Src += Pic->BpS;
  }
  Src = Pic->V;
  for(k = Pic->Height/2; k>0; --k) {  
    fwrite(Src, Pic->Width/2, 1, stdout);
    Src += Pic->BpS;
  }
}

static void PGM_Out(SKL_MP4_PIC *Pic)
{
  fprintf(stdout, "P5\n\n%d %d\n255\n", Pic->Width, Pic->Height*3/2 );

  SKL_BYTE *Src = Pic->Y;
  int k = Pic->Height;
  while(k-->0) {
    fwrite(Src, Pic->Width, 1, stdout);
    Src += Pic->BpS;
  }

  k = Pic->Height/2;
  SKL_BYTE *USrc = Pic->U;
  SKL_BYTE *VSrc = Pic->V;
  while(k-->0) {
    fwrite(USrc, Pic->Width/2, 1, stdout);
    fwrite(VSrc, Pic->Width/2, 1, stdout);
    USrc += Pic->BpS;
    VSrc += Pic->BpS;
  }
}

static int PGM_In(SKL_MP4_ENC *Enc, FILE *In, int *W, int *H, double Time)
{
  const SKL_MP4_PIC *Pic;
  SKL_BYTE *Dst;
  int k;

  if (!YUV_Input)
  {
    int Tmp;
    if (fscanf(In, "P5\n%d %d\n%d", W, H, &Tmp )!=3)
      return 0;
    if (Tmp!=255 || *W<=0 || *H<=0)
      return 0;
    fgetc(In);  // skips the '\n'

    Pic = Enc->Prepare_Next_Frame(*W, (*H)*2/3);
    if (Pic==0) return 0;
    for(Dst = Pic->Y, k=0; k<Pic->Height; ++k) {
      if (fread(Dst, Pic->Width, 1, In)!=1) return 0;
      Dst += Pic->BpS;
    }

    SKL_BYTE *DstU = Pic->U;
    SKL_BYTE *DstV = Pic->V;
    for(k=0; k<Pic->Height/2; ++k) {
      if (fread(DstU, Pic->Width/2, 1, In)!=1) return 0;
      if (fread(DstV, Pic->Width/2, 1, In)!=1) return 0;
      DstU += Pic->BpS;
      DstV += Pic->BpS;
    }
  }
  else {
    *W = YUV_W;
    *H = YUV_H * 3/2;

    Pic = Enc->Prepare_Next_Frame(YUV_W, YUV_H);
    if (Pic==0) return 0;
    
    for(Dst = Pic->Y, k=0; k<Pic->Height; ++k) {
      if (fread(Dst, Pic->Width, 1, In)!=1) return 0;
      Dst += Pic->BpS;
    }
    for(Dst = Pic->U, k=0; k<Pic->Height/2; ++k) {
      if (fread(Dst, Pic->Width/2, 1, In)!=1) return 0;
      Dst += Pic->BpS;
    }
    for(Dst = Pic->V, k=0; k<Pic->Height/2; ++k) {
      if (fread(Dst, Pic->Width/2, 1, In)!=1) return 0;
      Dst += Pic->BpS;
    }
  }

  if (Edge_Detect==1) {
    if (Img_Dsp.Name==0) Skl_Init_Img_DSP(&Img_Dsp, Cpu);
    SKL_BYTE *Dst = Pic->Y;
    const int BpS = Pic->BpS;
    int x, y;
    for(y=0; y<Pic->Height/16; y++)
      for(x=0; x<Pic->Width/16; x++)
        Img_Dsp.Grad2_18x18_To_8x8(Dst+(x+y*BpS)*8, BpS, Dst+(x+y*BpS)*16, BpS);
  }

  return 1;
}

//////////////////////////////////////////////////////////
//
// YUV => RGB and display
//
//////////////////////////////////////////////////////////

static void YUV_To_RGB32(SKL_BTM &Btm, const SKL_MP4_PIC &Pic)
{
  if (YUV_Dsp.Name==0) {
    Skl_Init_YUV_DSP(&YUV_Dsp, Cpu);
    YUV_Dsp.Init(Transfer_Type);
  }
  SKL_BYTE *Dst = Btm.Lock();
  const int BpS = Btm.BpS();

  if (Btm.Format()==0x40888)
    YUV_Dsp.YUV_TO_RGB32(Dst, BpS, Pic.Y, Pic.U, Pic.V, Pic.BpS, Pic.Width, Pic.Height);
  else if (Btm.Format()==0x20565)
    YUV_Dsp.YUV_TO_RGB565(Dst, BpS, Pic.Y, Pic.U, Pic.V, Pic.BpS, Pic.Width, Pic.Height);
  YUV_Dsp.Switch_Off();
  /* else: not supported yet */
  Btm.Unlock();
}

static void Deal_With_Pic(SKL_MP4_PIC &Pic, int Nb)
{
  int W   = Pic.Width;
  int H   = Pic.Height;
  int BpS = Pic.BpS;
  SKL_BYTE *Src = Pic.Y;
  if (VTrace==2) {
    Src = Pic.U; H /= 2;
  }
  else if (VTrace==3) { 
    Src = Pic.Y - 16 - 16*BpS;
    W += 2*16;
    H = H+16+16 + H/2+8+8; 
  }
  if (Save==2) PGM_Out(&Pic);
  else if (Save==3) YUV_Out(&Pic);

  if (Show && VTrace==0) {
#ifndef SKL_NO_VIDEO
    if (To_RGB==0x40888) {
      SKL_WINDOW *Vid = Sku_Get_Video(W, H, 0x40888);
      YUV_To_RGB32(*Vid, Pic);
      if (Save==1) Sku_Dump_Pic(Vid);
    }
    else if (To_RGB==0x20565) {
      SKL_WINDOW *Vid = Sku_Get_Video(W, H, 0x20565);
      YUV_To_RGB32(*Vid, Pic);
      if (Save==1) Sku_Dump_Pic(Vid);
    }
#endif
  }
  else if (Show||Save) {
    static SKL_CMAP_X Cmap;
    static int Ok = 0;
    if (!Ok) {
      Cmap.Ramp( SKL_COLOR_BLACK, SKL_COLOR_WHITE );
      Ok = 1;
    }
    SKL_BTM Btm(All_Mem);
    Btm.Set_Virtual(W, H, 0x10000, Src, BpS, &Cmap);
#ifndef SKL_NO_VIDEO
    if (Show) Sku_Show_Pic(&Btm);
#endif
    if (Save==1) Sku_Dump_Pic(&Btm);
  }
}

//////////////////////////////////////////////////////////
//
// Decoding
//
//////////////////////////////////////////////////////////

struct PSNR_INFOS
{ 
  double     PSNR_Y,PSNR_U,PSNR_V,PSNR_A;
  double     pY,    pU,    pV,    pA;
  size_t Size;
  int Cnt;
  SKL_BYTE *Y, *U, *V;
  FILE *_File;

  void Store(SKL_BYTE *Dst, const SKL_BYTE *Src, int W, int H, int BpS) {
    W = (W+7) & ~7;
    for(int y=0; y<H; ++y, Src+=BpS, Dst+=BpS)
      SKL_MEMCPY(Dst, Src, W);
  }
  void Load(SKL_BYTE *Dst, SKL_CST_STRING YUV_In, int W, int H, int BpS) {
    if (_File==0) {
      _File = fopen( YUV_In, "rb" );
      if (_File==0)
        Skl_Throw( SKL_EXCEPTION( "Can't open Reference YUV file\n" ) );
    }
    const int W_Padded = (W+7) & ~7;
    for(int y=0; y<H; ++y, Dst+=BpS) {
      fread(Dst, W, 1, _File);
      for(int x=W; x<W_Padded; ++x) Dst[x] = 0;
    }
  }

  SKL_UINT32 Get_SSE(SKL_BYTE *Ref, const SKL_BYTE *Src, int W, int H, int BpS) {
    SKL_UINT32 SSE = 0;
    int x,y;
    for(y=0; (y+16)<=H; y+=16) {
      for(x=0; (x+16)<=W; x+=16) SSE += Img_Dsp.SSD_16x16(Ref+x,Src+x,BpS);
      if ((x+8)<=W) SSE += Img_Dsp.SSD_8x8(Ref+x,Src+x,BpS);
      Src += 16*BpS;
      Ref += 16*BpS;
    }
    if ((y+8)<=H)
      for(x=0; x<W; x+=8) SSE += Img_Dsp.SSD_8x8(Ref+x,Src+x,BpS);
    return SSE;
  }
  double Compute_PSNR(int Val, int Size) {
    return Val>0. ? -4.3429448*log( 1.*Val / (255.*255.*Size) ) : 0.;
  }
  void Check(int W, int H, int BpS) {
    const size_t Needed = BpS*H + 2*(BpS/2)*(H/2);
    if (Y==0 || Size!=Needed) {
      Clear();
      Y = (SKL_BYTE*)All_Mem->New(Needed);
      U = Y + BpS*H;
      V = U + BpS/2;
      Size = Needed;
    }
    if (Img_Dsp.Name==0) Skl_Init_Img_DSP(&Img_Dsp, Cpu);
  }
  void Clear() { if (Y!=0) { All_Mem->Delete( Y, Size ); Y=0; Size=0; } }

  void Print_Infos() {
    SKL_ASSERT(Cnt>0);
    if (Cnt==1) printf( "#frm  \tY      U      V      All   avrg:Y      U      V      All\n" );
    printf( "%d  \t%.3f %.3f %.3f %.3f    \t%.3f %.3f %.3f %.3f\n", 
      Cnt, 
      pY, pU, pV, pA,
      PSNR_Y/Cnt,
      PSNR_U/Cnt,
      PSNR_V/Cnt,
      PSNR_A/Cnt);
  }

  PSNR_INFOS() { 
    Size = 0; 
    Y=U=V=0;
    Cnt = 0; 
    PSNR_Y = PSNR_U = PSNR_V = PSNR_A = 0.;
    _File = 0;
  }
  ~PSNR_INFOS() {
    if (_File!=0) fclose(_File);
    Clear();
  }
};

//////////////////////////////////////////////////////////
//
// compare PSNR against ref file
//
//////////////////////////////////////////////////////////

static void Compare_PSNR_External(const SKL_MP4_PIC *Pic, PSNR_INFOS *I)
{
  SKL_ASSERT(Ref_YUV_File!=0);

  I->Check(Pic->Width, Pic->Height, Pic->BpS);
  I->Load(I->Y, Ref_YUV_File, Pic->Width,   Pic->Height,   Pic->BpS);
  I->Load(I->U, Ref_YUV_File, Pic->Width/2, Pic->Height/2, Pic->BpS);
  I->Load(I->V, Ref_YUV_File, Pic->Width/2, Pic->Height/2, Pic->BpS);

  SKL_UINT32 SSE_Y = I->Get_SSE(I->Y, Pic->Y, Pic->Width,   Pic->Height,   Pic->BpS);
  SKL_UINT32 SSE_U = I->Get_SSE(I->U, Pic->U, Pic->Width/2, Pic->Height/2, Pic->BpS);
  SKL_UINT32 SSE_V = I->Get_SSE(I->V, Pic->V, Pic->Width/2, Pic->Height/2, Pic->BpS);
  Img_Dsp.Switch_Off();
  I->PSNR_Y += I->pY = I->Compute_PSNR(SSE_Y, Pic->Width*Pic->Height);
  I->PSNR_U += I->pU = I->Compute_PSNR(SSE_U, Pic->Width*Pic->Height/4);
  I->PSNR_V += I->pV = I->Compute_PSNR(SSE_V, Pic->Width*Pic->Height/4);
  I->PSNR_A += I->pA = I->Compute_PSNR(SSE_Y+SSE_U+SSE_V, 3*Pic->Width*Pic->Height/2);
  I->Cnt++;
  if (I->Cnt==10000) {
      // reset every ~10000 frames, to avoid overflow...
      // (yeah, I know, we won't end up with the real PSNR :)
    I->PSNR_Y /= I->Cnt;
    I->PSNR_U /= I->Cnt;
    I->PSNR_V /= I->Cnt;
    I->PSNR_A /= I->Cnt;
    I->Cnt = 1;
  }
  I->Print_Infos();
}

//////////////////////////////////////////////////////////
//
// simple empty example of slicer hook
//
//////////////////////////////////////////////////////////

static void Display_Slice(const SKL_MP4_PIC *Pic, int y, int Height, SKL_ANY Data)
{
  printf( "Slice y=%3d/%3d (height=%d)\n", y, Pic->Height, Height );
}

//////////////////////////////////////////////////////////
//
// naive AVI parsing
//
//////////////////////////////////////////////////////////

static SKL_UINT32 Get32b_LE(FILE *f) {
  return ((SKL_UINT32)fgetc(f)<<0) | (fgetc(f)<<8) | (fgetc(f)<<24) | (fgetc(f)<<24);
}
static SKL_UINT16 Get16b_LE(FILE *f) {
  return (fgetc(f)<<0) | (fgetc(f)<<8);
}
static SKL_UINT32 TwoCC(const char * const S) {
  return (S[0]<<0) | (S[1]<<8);
}
static SKL_UINT32 FourCC(const char * const S) {
  return (S[0]<<24) | (S[1]<<16) | (S[2]<<8) | (S[3]<<0);
}

static int Locate_Marker(SKL_UINT32 Marker, FILE *f) 
{
  SKL_UINT32 Code = 0xDeadBeef;
  do Code = (Code<<8) | fgetc(f);
  while( Code!=Marker && !feof(f) );
  return feof(f);
}

static long Parse_Input_File(SKL_BYTE *Buf, size_t Size, FILE *f)
{
  int Ok = 0;
  if (System_Stream==1)      // Ultra-Simplistic AVI Parser
  {
    static SKL_UINT32 Chunk_Size = 0;
    static int Have_MOVI = 0;
    if (!Have_MOVI) {
      const SKL_UINT32 _movi_ = FourCC( "movi" );
      if (Locate_Marker(_movi_, f))
        return 0;
      Have_MOVI = 1;
    }
    while(Size>0) {
      while (Chunk_Size==0) {
        SKL_UINT16 Nb = Get16b_LE(f);
        SKL_UINT32 Id = Get16b_LE(f);
        SKL_UINT32  S = Get32b_LE(f);
        S += (S&1);
        Nb = ((Nb>>8)-'0') + ((Nb&0xff)-'0')*10;
        if ( Nb!=0 || Id!=TwoCC( "dc" ) )
          fseek(f, S, SEEK_CUR);
        else
          Chunk_Size = S;
      }
      size_t Gotten = fread(Buf+Ok, 1, (Size>Chunk_Size) ? Chunk_Size : Size, f);
      Ok         += Gotten;
      Chunk_Size -= Gotten;
      Size       -= Gotten;
    }
  }
  else if (System_Stream==1)    // Ultra-Dumb MPG Parser
  {
    // not yet done...
  }
  else {
    Ok = fread(Buf, 1, Size, f);
  }
  return Ok;
}

//////////////////////////////////////////////////////////
//
// main decoding loop (very decorated!)
//
//////////////////////////////////////////////////////////

static int Decode_MPEG4(SKL_CST_STRING Name)
{
    // create codec instance

  MP4_New_Decoder = (SKL_MP4_NEW_DEC)SKL_LOAD_DYN_SYMBOL(SKL_MP4_DLL, Skl_MP4_New_Decoder);
  MP4_Delete_Decoder = (SKL_MP4_DELETE_DEC)SKL_LOAD_DYN_SYMBOL(SKL_MP4_DLL, Skl_MP4_Delete_Decoder);

  if (MP4_New_Decoder==0 || MP4_Delete_Decoder==0)  // problem with DLL
    return -1;

  SKL_MP4_DEC *Dec = MP4_New_Decoder();
  if (Dec==0) return -2;
  if (Trace_Mem)
    Dec->Set_Memory_Manager(All_Mem);

  Dec->Set_CPU( Cpu );
  if (With_Slices) Dec->Set_Slicer(Display_Slice,0);

      // Dec debug level: 1:show MVs  2:print scanlines 3-4: print infos 
  if (Debug>=4) Dec->Set_Debug_Level(Debug-3);

    // Prepare a PSNR_INFOS for external PSNR comparison
  PSNR_INFOS PSNR_Infos;   

    // open I/O files

  FILE *f;
  long Fullness, Left_To_Read, Size, Left;
  SKL_BYTE *Buf, *mBuf = 0;
  const int MAX_BUF = 400000;   // TODO: BAD
  SKL_BYTE Buf0[MAX_BUF+4] = {0};     // +4 = sentinel (just in case)
  int Err = 0;
  SKL_PTIMER pTimer;

  f = fopen( Name, "rb" );
  if (f==0) {
    fprintf( stderr, "Can't open file '%s' for reading.\n", Name );
    return -1;
  }
  fseek(f, 0, SEEK_END);
  Size = ftell(f);
  rewind(f);

  if (From_Mem) {
    mBuf = (SKL_BYTE*)All_Mem->New(Size);
    Buf = mBuf;
    Fullness = Parse_Input_File(Buf, Size, f);
    if (Fullness==0) {
      Err = -1;
      goto End;
    }
    Left_To_Read = 0;
  }
  else {
    Buf = Buf0;
    Left_To_Read = Size;
    Fullness = 0;
    if (Streaming && Jump_To>0) {
      if (!Quiet) printf( "Jumping to offset %ld in file\n", Jump_To );
      fseek(f, Jump_To, SEEK_SET); // wowowow!!
    }
  }  

    // main decoding loop

  if (!Quiet)
    printf( "Decoding MP4 bitstream '%s' (size:%ld).\n", Name, Size);

  Left = Size;
  pTimer.Reset();
  while(Left>=0)
  {
    float Next_Tick = pTimer.Get_mSec();

    if (!From_Mem && Fullness<MAX_BUF/2) {
      if (Fullness>0)
        memmove( Buf0, Buf, Fullness);
      Buf = Buf0;
      const int Free = MAX_BUF-Fullness;
      size_t More = (Left_To_Read>Free) ? Free : Left_To_Read;
      if (More>0) {
        More = Parse_Input_File(Buf+Fullness, More, f);
        Fullness += More;
        Left_To_Read -= More;
      }
    }

    int Read = MPEG12 ? Dec->Decode_MPEG12(Buf, Fullness)
                      : Dec->Decode(Buf, Fullness);
    if (Read>Fullness)
      throw SKL_EXCEPTION( "Aieeee! Buffer read overflow" );


    if (Debug==2)
      printf( " Left:%ld consumed:%d fullness=%ld\n", Left, Read, Fullness );
    else if (Debug==3)  // for gnuplot
      printf( "%d %d %ld\n", Dec->Get_Frame_Number(), Read, Fullness );

    if (Left==0 && Read==0) Left = -1;   // very last frame
    else Left -= Read;
    Buf       += Read;
    Fullness  -= Read;

    if (!Dec->Is_Frame_Ready() || Skip_Frames-->0)
      continue;

    if (Delay>0)    
      pTimer.Wait(1.0f*Delay);
    else if (FPS>0.)
    {
      Next_Tick += 1000.f / FPS - pTimer.Get_mSec();
      if (Next_Tick>0.)
        pTimer.Wait(Next_Tick);
      /* else: Hurry up! */
    }
      
    pTimer++;

    SKL_MP4_PIC Pic;
    if (VTrace==4) Dec->Get_All_Frames(&Pic);
    else {
      Dec->Consume_Frame(&Pic);
      if (Debug==1)
        printf( "Decoded frame #%d (Time stamp=%.3f s)   \r", Dec->Get_Frame_Number(), Pic.Time);
      if (Show_PSNR)
        Compare_PSNR_External(&Pic, &PSNR_Infos);
    }
    Deal_With_Pic( Pic, Dec->Get_Frame_Number() );

    if (!Quiet && pTimer.Get_Count()==1)
      printf( " - Frames size: %dx%d\n", Pic.Width, Pic.Height );

    if (!Streaming && Jump_To>0 && !From_Mem && pTimer.Get_Count()==1) {
      if (!Quiet) printf( "Jumping to offset %ld in file\n", Jump_To );
      fseek(f, Jump_To, SEEK_SET); // wowowow!!
      Fullness = 0;
    }

    if (!--Nb_Frames)
      break;
  }

  if (Quiet!=1) pTimer.Elapsed_FPS(0,"\n");

End:
  if (f!=0) fclose(f);
  if (From_Mem)
    All_Mem->Delete(mBuf, Size);
  MP4_Delete_Decoder(Dec);
  return Err;
}

//////////////////////////////////////////////////////////
//
//   Encoding
//
//////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////
//
// example of external "Slicer" hook 
//   => computes PSNR for each slice
//
//////////////////////////////////////////////////////////


static void Sliced_PSNR(const SKL_MP4_PIC *Pic, int yo, int Height, SKL_ANY Data)
{
  PSNR_INFOS *I = (PSNR_INFOS*)Data;

  if (Height==0)    // start/end of scan?
  {
    if (yo==0) {      // start of scan
      I->Check(Pic->Width, Pic->Height, Pic->BpS);
      I->Store(I->Y, Pic->Y, Pic->Width,   Pic->Height,   Pic->BpS);
      I->Store(I->U, Pic->U, Pic->Width/2, Pic->Height/2, Pic->BpS);
      I->Store(I->V, Pic->V, Pic->Width/2, Pic->Height/2, Pic->BpS);
    }
    else {            // end of scan
      SKL_UINT32 SSE_Y = I->Get_SSE(I->Y, Pic->Y, Pic->Width,   Pic->Height,   Pic->BpS);
      SKL_UINT32 SSE_U = I->Get_SSE(I->U, Pic->U, Pic->Width/2, Pic->Height/2, Pic->BpS);
      SKL_UINT32 SSE_V = I->Get_SSE(I->V, Pic->V, Pic->Width/2, Pic->Height/2, Pic->BpS);
      Img_Dsp.Switch_Off();
      I->PSNR_Y += I->pY = I->Compute_PSNR(SSE_Y, Pic->Width*Pic->Height);
      I->PSNR_U += I->pU = I->Compute_PSNR(SSE_U, Pic->Width*Pic->Height/4);
      I->PSNR_V += I->pV = I->Compute_PSNR(SSE_V, Pic->Width*Pic->Height/4);
      I->PSNR_A += I->pA = I->Compute_PSNR(SSE_Y+SSE_U+SSE_V, 3*Pic->Width*Pic->Height/2);
      I->Cnt++;
      if (I->Cnt==10000) {
          // reset every ~10000 frames, to avoid overflow...
          // (yeah, I know, we won't end up with the real PSNR :)
        I->PSNR_Y /= I->Cnt;
        I->PSNR_U /= I->Cnt;
        I->PSNR_V /= I->Cnt;
        I->PSNR_A /= I->Cnt;
        I->Cnt = 1;
      }
    }
  }
  else { /* we're in a slice. */ }
}

//////////////////////////////////////////////////////////
//
// Main encoding loop
//
//////////////////////////////////////////////////////////

static int Parse_Custom_Matrix_File(SKL_MP4_ENC *const Enc)
{
  FILE *f = fopen(Custom_Matrix, "r");
  if (f==0) {
    fprintf( stderr, "Can't open custom matrix file [%s]\n", Custom_Matrix );
    return 0;
  }
  SKL_BYTE M[2*64];
  int i, k;
  for(i=0; i<2*64; ++i)
    if (fscanf(f, "%d", &k)!=1) return 0;
    else M[i] = k;
  fclose(f);

  Enc->Set_Custom_Matrix(0, M+ 0);
  Enc->Set_Custom_Matrix(1, M+64);
  return 1;
}

static int Encode_MPEG4(SKL_CST_STRING Out_Name, SKL_CST_STRING In_Name)
{
    // create an encoder instance
  MP4_New_Encoder = (SKL_MP4_NEW_ENC)SKL_LOAD_DYN_SYMBOL(SKL_MP4_DLL, Skl_MP4_New_Encoder);
  MP4_Delete_Encoder = (SKL_MP4_DELETE_ENC)SKL_LOAD_DYN_SYMBOL(SKL_MP4_DLL, Skl_MP4_Delete_Encoder);

  if (MP4_New_Encoder==0 || MP4_Delete_Encoder==0)  // problem with DLL 
    return -1;

  SKL_MP4_ENC *Enc = MP4_New_Encoder();
  if (Enc==0) return -2;

      /* Enc debug level: 1:show MVs  2:print scanlines 3-4: print infos  */
  if (Debug>=4) Enc->Set_Debug_Level(Debug-3);

  Enc->Set_CPU( Cpu );
  Enc->Set_Memory_Manager(All_Mem);
  Enc->Get_Analyzer()->Set_Memory_Manager(All_Mem);
  if (Streaming) Enc->Ioctl( "emit-key-headers" );
  if (Sequence_Codes) Enc->Ioctl( "emit-sequence-codes" );
  if (Custom_Matrix) 
    if (!Parse_Custom_Matrix_File(Enc)) {
      fprintf( stderr, "Custom matrix parsing failed!\n" );
      return -2;
    }

    // warm up the analyzer

  if (FPS==0) FPS = 29.970f;   // dflt value

  Enc->Get_Analyzer()->Set_Param( "bitrate", Bit_Rate );
  Enc->Get_Analyzer()->Set_Param( "framerate", FPS );
  Enc->Get_Analyzer()->Set_Param( "base-quant", Global_Q );
  Enc->Get_Analyzer()->Set_Param( "quant", Global_Q );
  Enc->Get_Analyzer()->Set_Param( "quant-type", Quant_Type );
  Enc->Get_Analyzer()->Set_Param( "intra-max-delay", Intra_Max );
  Enc->Get_Analyzer()->Set_Param( "4v-probing", Inter4V_Probing );
  Enc->Get_Analyzer()->Set_Param( "field-pred-probing", Field_Pred_Probing );
  Enc->Get_Analyzer()->Set_Param( "gmc-mode", Use_GMC );
  Enc->Get_Analyzer()->Set_Param( "gmc-pts", 3 );
  Enc->Get_Analyzer()->Set_Param( "gmc-accuracy", GMC_Accuracy );
  Enc->Get_Analyzer()->Set_Param( "reduced-frame", Use_Reduced );
  Enc->Get_Analyzer()->Set_Param( "interlace-field", Interlace_Field );
  Enc->Get_Analyzer()->Set_Param( "rounding", 0 );
  Enc->Get_Analyzer()->Set_Param( "search-metric", Search_Metric );
  Enc->Get_Analyzer()->Set_Param( "sad-skip-limit", SAD_Skip_Limit );
  Enc->Get_Analyzer()->Set_Param( "sad-intra-limit", SAD_Intra_Limit );
  Enc->Get_Analyzer()->Set_Param( "interlace-dct", Interlace_DCT );
  Enc->Get_Analyzer()->Set_Param( "base-search-size", MV_Size );
  Enc->Get_Analyzer()->Set_Param( "search-size", MV_Size );
  Enc->Get_Analyzer()->Set_Param( "search-method", Search_Method );
  Enc->Get_Analyzer()->Set_Param( "subpixel", Sub_Pixel );
  Enc->Get_Analyzer()->Set_Param( "dquant-amp", dQuant_Amp );  
  Enc->Get_Analyzer()->Set_Param( "lambda", Lambda );
  Enc->Get_Analyzer()->Set_Param( "intra-limit", Intra_Limit );
  Enc->Get_Analyzer()->Set_Param( "inter-threshold", Inter_Thresh );
  Enc->Get_Analyzer()->Set_Param( "use-trellis", Use_Trellis );
  Enc->Get_Analyzer()->Set_Param( "hi-mem", Hi_Mem );
  Enc->Get_Analyzer()->Set_Param( "buffer-size", Enc_Buf_Size );
  Enc->Get_Analyzer()->Set_Param( "verbose", ADebug );
  Enc->Get_Analyzer()->Set_Param( "pass", Pass_Nb);
  Enc->Get_Analyzer()->Set_Param( "passfile", Pass_File);
  Enc->Get_Analyzer()->Set_Param( "passrf", Pass_RF);

    // plug PNSR slicer hook

  PSNR_INFOS PSNR_Infos;
  if (Show_PSNR&(1|2))
    Enc->Set_Slicer(Sliced_PSNR, (SKL_ANY)&PSNR_Infos);

    // open I/O files

  int Err = 0;
  FILE *In = 0, *Out = 0;  
  int Cnt = 0;
  int W=0, H=0;
  int Pos = 0;
  SKL_PTIMER pTimer;
  long Out_Size = 0;

  if (In_Name) {
    In = fopen(In_Name, "rb");
    if (In==0) {
      fprintf( stderr, "Can't open file '%s'!\n", In_Name );
      Err = -1;
      goto End;
    }
  }
  else In = stdin;

  if (Out_Name!=0) {
    Out = fopen(Out_Name, "wb");
    if (Out==0) {
      fprintf( stderr, "Can't open file '%s' for writing.", Out_Name );
      Err = -1;
      goto End;
    }
    if (!Quiet)
      printf( "Encoding MP4 bitstream '%s'.\n", Out_Name);
  }
  else SKL_ASSERT(Pass_Nb==1);      // Pass #1 doesn't require to output a bitstream (just stats)

    // main loop

  pTimer.Reset();
  while( PGM_In(Enc, In, &W, &H, Cnt*1000./FPS) )
  {
    if (Skip_Frames-->0)
      continue;

    if (Use_Reduced>=0 && (((W/16)&1) || ((H*2/3/16)&1)) ) {
      printf( "Reduced frames problem: Frame size %dx%d cannot be split into 32x32 blocks!!\n", W, H );
      goto End;
    }

    ++Cnt;

    if (!Quiet)
      if (!(Cnt&0xf)) printf( "Encoding frame #%d   \r", Cnt );

    Enc->Encode();

    if (Enc->Get_Bits_Length()) 
    {
      if (Out!=0 && fwrite( Enc->Get_Bits(), Enc->Get_Bits_Length(), 1, Out )!=1) {
        fprintf( stderr, "Write error!\n" );
        Err = -1;
        goto End;
      }
      Out_Size += Enc->Get_Bits_Length();

// Example of key-frame detection:
//      const SKL_MP4_PIC *Last = Enc->Get_Last_Coded_Frame();
//      if (Last->Coding==0) printf( "Key frame! (%ld)    ", Last->Time_Ticks );


      if (Debug==2) {
        printf( " Pos:%d Frame size:%d\n", Pos, Enc->Get_Bits_Length());
        Pos += Enc->Get_Bits_Length();
      }
    }

    pTimer++;


    if (Show_PSNR&1)
      PSNR_Infos.Print_Infos();

    if (VTrace!=0) {
      SKL_MP4_PIC Pic;
      Enc->Get_All_Frames(&Pic);
      Deal_With_Pic(Pic, pTimer.Get_Count());
    }

    if (!--Nb_Frames)
      break;
  }

  while(Enc->Finish_Encoding()) {
    if (Out!=0 && fwrite( Enc->Get_Bits(), Enc->Get_Bits_Length(), 1, Out )!=1) {
      fprintf( stderr, "Write error!\n" );
      Err = -1;
      goto End;
    }
    Out_Size += Enc->Get_Bits_Length();
  
    if (Debug==2) {
      printf( "*Pos:%d Frame size:%d\n", Pos, Enc->Get_Bits_Length());
      Pos += Enc->Get_Bits_Length();
    }

  }

  if (!Quiet || Show_PSNR)
  {
    pTimer.Stop();
    const int Nb = pTimer.Get_Count();
    const float BpF = (Nb>0) ? 1.0f*Out_Size/Nb : 0;
    const float Actual_Bit_Rate = 8.f*BpF*FPS/1000.f;

    if (!Quiet) {
      pTimer.Elapsed_FPS(0,"\n\n");
      printf( " * File size       : %ld bytes\n", Out_Size );
      printf( " * Frames          : %dx%d x %d\n", W, H*2/3, Nb);
      printf( " * Avrg frame size : %.0f bytes\n", BpF );
      printf( " * Avrg kBits/sec  : %.2f (FPS=%.3f)\n", Actual_Bit_Rate, FPS );
      if (PSNR_Infos.Cnt>0) {
        if (Show_PSNR&2) printf( " * Avrg PSNR       : %.3f (Y:%.3f U:%.3f V:%.3f)\n",
          PSNR_Infos.PSNR_A/PSNR_Infos.Cnt,
          PSNR_Infos.PSNR_Y/PSNR_Infos.Cnt,
          PSNR_Infos.PSNR_U/PSNR_Infos.Cnt,
          PSNR_Infos.PSNR_V/PSNR_Infos.Cnt );
      }
      printf( "\n" );
    }
    else if ((Show_PSNR&4) && PSNR_Infos.Cnt>0)  // rate-distortion stats
      printf( "%d %.3f %ld %.4f %.4f %.4f %.4f\n",
        Bit_Rate>0 ? 0 : Global_Q,
        Actual_Bit_Rate,
        Out_Size,
        PSNR_Infos.PSNR_A/PSNR_Infos.Cnt,
        PSNR_Infos.PSNR_Y/PSNR_Infos.Cnt,
        PSNR_Infos.PSNR_U/PSNR_Infos.Cnt,
        PSNR_Infos.PSNR_V/PSNR_Infos.Cnt );

  }
End:
  MP4_Delete_Encoder(Enc);
  if (In!=stdin && In!=0) fclose(In);
  if (Out!=0) fclose(Out);
  return Err;
}

//////////////////////////////////////////////////////////
//
// Options and params
//
//////////////////////////////////////////////////////////

static void Banner( )
{
  printf( "\n    -= Toy MPEG4 plailleure & encaudeure (v%.2f) =-\n\n", VERSION );
  printf( "     (C) 2003 Pascal Massimino (skal.planet-d.net)\n" );
}

static void Help( char **argv, SKL_CST_STRING S=0 )
{
  if (S!=0) printf( S );
  printf( " * Usage: %s [options] [in-file]\n", argv[0] );
  printf( "\n" );
  printf( " While encoding (-o option), PGM [in-file] must be supplied\n" );
  printf( " or piped into stdin.\n" );
  printf( "\n" );

  printf( " * Encoder options:\n" );
  printf( "    -o <name> ........ Out file for encoding\n" );
  printf( "    -q <int> ......... Global quantizer                    [%d]\n", Global_Q );
  printf( "    -br <int> ........ target bit-rate rate (in kbits/s)   [%d]\n", Bit_Rate );
  printf( "    -fps <float> ..... target frame/s) (0=free)            [%.3f]\n", FPS );
  printf( "    -qtype <int> ..... Quant type (0:H263, 1:MPEG4)        [%d]\n", Quant_Type );
  printf( "    -imax <int> ...... Intra max delay                     [%d]\n", Intra_Max );
  printf( "    -4v <int> ........ 4MV probing percent (0=none (off), 100=full) [%d]\n", Inter4V_Probing );
  printf( "    -sad <int> ....... Search metric to use (0=SAD)        [%d]\n", Search_Metric );
  printf( "    -sad_lo <int> .... SAD lower limit for SKIP            [%d]\n", SAD_Skip_Limit );
  printf( "    -sad_hi <int> .... SAD high limit for INTRA            [%d]\n", SAD_Intra_Limit );
  printf( "    -lambda <float> .. Lambda MV weight for cost function  [%.3f]\n", Lambda );
  printf( "    -red ............. Use reduced frames\n" );
  printf( "    -redf ............ Force use of reduced frames\n" );
  printf( "    -gmc ............. Use GMC (with decision)\n" );
  printf( "    -gmcf ............ Force GMC usage for all frames\n" );
  printf( "    -gmc_acc <int> ... GMC accuracy [0..3]=[1/2,..,1/16]pel [%d]\n", GMC_Accuracy );
  printf( "    -intl ............ Enable Interlaced encoding (with decisions)\n" );
  printf( "    -intldct ......... force interlaced DCT encoding\n" );
  printf( "    -intlfld ......... force interlaced field-prediction encoding\n" );
  printf( "    -fld <int> ....... Field-Prediction worthiness probing percent (0=none (off), 100=full) [%d]\n", Field_Pred_Probing );
  printf( "    -s <int> ......... MV-search method\n" );
  printf( "    -mv <int> ........ MV-search window size               [%d]\n", MV_Size );
  printf( "    -il <int> ........ Scene-change limit, in percent      [%d]\n", Intra_Limit );
  printf( "    -ithresh <int> ... Set inter threshold (0=disable)     [%d]\n", Inter_Thresh );
  printf( "    -trellis ......... Use trellis-based quantization (slower)   [%d]\n", Use_Trellis );
  printf( "    -qpel ............ Use quarter pixel interpolation\n" );
  printf( "    -hpel ............ Use half pixel interpolation (default)\n" );
  printf( "    -fpel ............ Use full pixel only\n" );  
  printf( "    -dq <float> ...... Luminance masking strength (0:off)  [%.4f]\n", dQuant_Amp );
  printf( "    -buf_size <int> .. Encoding buffer size (0=disable)    [%d]\n", Enc_Buf_Size );
  printf( "    -seq_code ........ Emit SEQuence codes at start/end    [%d]\n", Sequence_Codes );
  printf( "    -psnr ............ Print overall PSNR, averaged\n" );
  printf( "    -psnr_all ........ Print PSNR for each frame\n" );
  printf( "    -rdist ........... Only print rate-distortion infos\n" );
  printf( "    -pass <int> ...... Pass number (0=none, 1/2)           [%d]\n", Pass_Nb);
  printf( "    -passfile <name> . Pass file to use                    [%s]\n", Pass_File );
  printf( "    -passrf <float> .. 2nd-pass reaction factor            [%.2f]\n", Pass_RF);
  printf( "    -yuv <w> <h> ..... Input is YUV420 of size w x h\n" );
  printf( "    -custom <name> ... Read custom intra+inter matrix from file\n" );
  printf( "    -sqcif -qcif -cif\n" );
  printf( "      -4cif -pal -ntsc\n" );
  printf( "      -720p -1080p ..... Predefined shortcuts for -yuv <w> <h> option \n" );
  printf( "    -detect_edges .... Don't use! :)\n" );
  printf( "\n" );

  printf( " * Decoder options:\n" );
  printf( "    -avi ............. input stream is AVI\n" );
//  printf( "    -mpg ............. input stream is MPEG-system\n" );
  printf( "    -save ............ Save ppm files snapxxx.ppm\n" );
  printf( "    -pgm ............. Send pgm-pipe files to stdout\n" );
  printf( "    -yuv_out ......... send raw yuv420 to stdout\n" );
  printf( "    -mem ............. Load file in memory\n" );
  printf( "    -jump <int> ...... Jump to position (WARNING!!! DANGEROUS)\n" );
  printf( "    -test ............ Speed test (<=> -noshow -vtrace 0 -mem)\n" );
  printf( "    -delay <msec> .... Delay between frames display        [%d]\n", Delay );
  printf( "    -rgb ............. yuv->rgb565 conversion\n" );
  printf( "    -rgb32 ........... yuv->rgb32 conversion\n" );
  printf( "    -tt .............. transfer type for yuv->rgb32        [%d]\n", Transfer_Type );
  printf( "    -slice ........... Use per-slice output (is just an example)\n" );
  printf( "    -psnr_f <file> ... Compute decoded PSNR with ref YUV file <file>\n" );
  printf( "\n" );

  printf( " * Common options:\n" );
  printf( "    -nb <int> ........ Nb frames to decode/encode\n" );
  printf( "    -skip <int> ...... Nb frames to skip before decoding/encoding\n" );
  printf( "    -mpg2 ............ Input is a MPEG1/2 bitstream\n" );
  printf( "    -stream .......... Encoding will re-emit VOL-header at key-frames\n" );
  printf( "    -noshow .......... Don't display frames\n" );
  printf( "    -cpu <type>....... CPU to use. type={x86,sse,mmx,c,ref,alt}\n" );
  printf( "    -quiet ........... Keep quiet\n" );
  printf( "    -debug <int> ..... Debug level                         [%d]\n", Debug );
  printf( "    -adebug <int> .... Analyzer verbose level              [%d]\n", ADebug );
  printf( "    -vtrace <level> .. Visual trace (debug)                [%d]\n", VTrace );
  printf( "    -trace_mem ....... Trace memory usage (not for dll)    [%d]\n", Trace_Mem );
  printf( "    -v ............... Prints version and exits.           [%.2f]\n", VERSION );
  printf( "    -h ............... This help.\n" );
  printf( "\n" );

  printf( "\n" );
  printf( "Examples:\n" );
  printf( " %s -noshow -pgm test.m4v | %s -o test2.m4v -q 3\n", argv[0], argv[0] );
  printf( "   [will transcode 'test.m4v' into 'test2.m4v']\n" );
  printf( "\n" );
  printf( " %s test2.m4v\n", argv[0] );
  printf( "   [will decode and display 'test2.m4v']\n" );
  printf( "\n" );
  printf( " ffmpeg -i austin.avi -f pgmyuv -f imagepipe pipe: | %s -o goldmember.m4v\n", argv[0] );
  printf( "   [oh, behave!]\n" );
  printf( "\n" );

  exit( 0 );
}

//////////////////////////////////////////////////////////
// Command line parsing

static void Default_Params()
{
  Save            = 0;
  Show            = 1;
  With_Slices     = 0;
  From_Mem        = 0;
  Trace_Mem       = 0;
  MPEG12          = 0;
  Nb_Frames       =-1;  // all
  Skip_Frames     = 0;  // none
  Jump_To         = 0;  // disabled
  Streaming       = 0;  // default: storage
  Sequence_Codes  = 0;  // default: no sequence start/end codes
  Global_Q        = 4;
  Quant_Type      = 1;
  Bit_Rate        = 0;
  Search_Method   = 0;
  MV_Size         = 1;
  Hi_Mem          = 0;      // default: Don't use additional memory
  Enc_Buf_Size    = 0;
  Use_GMC         =-1;      // disabled
  GMC_Accuracy    = 3;      // 1/16th pel
  Use_Reduced     =-1;      // disabled
  Intra_Limit     = 20;     // 20% of intra-coded => scene change
  Search_Metric   = 0;      // SAD
  SAD_Skip_Limit  = 256;    // Sad()<SAD_Skip_Limit*Q ? => SKIP
  SAD_Intra_Limit = 2000;   // Sad()>SAD_Intra_Limit*Q ? => INTRA
  FPS             = 0.;
  Interlace_DCT   = 0;
  Interlace_Field = 0;
  Sub_Pixel       = 1;      // default: half-pel
  dQuant_Amp      = 0.0f;   // Default: off. Recommended value: ~8.0. Values below ~5. are not worth (file size grows)
  Lambda          = 1.0f;   // Neutral default value
  Intra_Max       = 200;
  Inter4V_Probing = 80;     // usually a good compromise between speed and PSNR gain due to 4V
  Field_Pred_Probing = 40;  // ditto.
  Inter_Thresh    =-1;      // disabled
  Use_Trellis     = 0;
  Show_PSNR       = 0;
  Pass_Nb         = 0;
  Pass_RF         = 0.10f;
  Edge_Detect     = 0;
  System_Stream   = 0;
  Delay           = 0;
  To_RGB          = 0x20565;
  Transfer_Type   = 1;
  VTrace          = 0;
  Cpu             = SKL_CPU_DETECT;   // will be probed later, at DSP's init.
  Debug           = 0;
  ADebug          = 0;
  Quiet           = 0;
  Out_File        = 0;
  In_File         = 0;
  Pass_File       = "pass1.log";
  Ref_YUV_File    = 0;
  YUV_Input       = 0;
  YUV_W           = 0;
  YUV_H           = 0;
  Custom_Matrix   = 0;
}

static void Missing( SKL_CST_STRING S )
{
  Skl_Throw( SKL_MSG_EXCEPTION( "Missing value after %s option\n", S ) );
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
  if      (Val<Min) return Min;
  else if (Val>Max) return Max;
  else              return Val;
}

static float Parse_Float_Value(const int argc, const char * const *argv, int &k,
                               const float Min=-1.e30f, const float Max=1.e30f)
{
  if (++k==argc) Missing( argv[k-1] );
  float Val = (float)atof(argv[k]);
  if      (Val<Min) return Min;
  else if (Val>Max) return Max;
  else              return Val;
}

static SKL_CPU_FEATURE Get_Feature(const int argc, const char * const *argv, int &k)
{
  if (++k==argc) Missing( argv[k-1] );
  if      ( !strcmp( argv[k], "ref" ) ) return SKL_CPU_REF;
  else if ( !strcmp( argv[k], "c"   ) ) return SKL_CPU_C;
  else if ( !strcmp( argv[k], "x86" ) ) return SKL_CPU_X86;
  else if ( !strcmp( argv[k], "sse" ) ) return SKL_CPU_SSE;
  else if ( !strcmp( argv[k], "sse2") ) return SKL_CPU_SSE2;
  else if ( !strcmp( argv[k], "mmx" ) ) return SKL_CPU_MMX;
  else if ( !strcmp( argv[k], "alt" ) ) return SKL_CPU_ALT;
  else {
    printf( "Unknown cpu '%s'! Reverting to 'c'\n", argv[k] );
    return SKL_CPU_C;
  }
}

static void Parse_Options( int argc, char **argv )
{
  Default_Params();
  for( int k=1; k<argc; ++k )
  {
    if ( !strcmp( argv[k], "-h" ) || !strcmp( argv[k], "-H" ) )
      Help( argv );
    else if ( !strcmp( argv[k], "-save" ) )      { Save = 1; }
    else if ( !strcmp( argv[k], "-pgm" ) )       { Save = 2; Show = 0; }
    else if ( !strcmp( argv[k], "-yuv_out" ) )   { Save = 3; Show = 0; }
    else if ( !strcmp( argv[k], "-noshow" ) )    { Show = 0; }
    else if ( !strcmp( argv[k], "-slice" ) )     { With_Slices = 1; }
    else if ( !strcmp( argv[k], "-mem" ) )       From_Mem = 1;
    else if ( !strcmp( argv[k], "-quiet" ) )     Quiet = 1;
    else if ( !strcmp( argv[k], "-detect_edges" ) ) Edge_Detect = 1;
    else if ( !strcmp( argv[k], "-rdist" ) )     { Show_PSNR = 2|4; Quiet=1; Show = 0; }
    else if ( !strcmp( argv[k], "-psnr_all" ) )  Show_PSNR |= 1;
    else if ( !strcmp( argv[k], "-psnr" ) )      Show_PSNR |= 2;
    else if ( !strcmp( argv[k], "-trellis" ) )   Use_Trellis = 1;
    else if ( !strcmp( argv[k], "-qpel" ) )      Sub_Pixel = 2;
    else if ( !strcmp( argv[k], "-hpel" ) )      Sub_Pixel = 1;
    else if ( !strcmp( argv[k], "-fpel" ) )      Sub_Pixel = 0;
    else if ( !strcmp( argv[k], "-red" ) )       Use_Reduced = 1;
    else if ( !strcmp( argv[k], "-redf" ) )      Use_Reduced = 2;
    else if ( !strcmp( argv[k], "-gmc" ) )       Use_GMC     = 0; // enabled
    else if ( !strcmp( argv[k], "-gmcf" ) )      Use_GMC     = 2; // force
    else if ( !strcmp( argv[k], "-low_mem" ) )   Hi_Mem      = 0;
    else if ( !strcmp( argv[k], "-hi_mem" ) )    Hi_Mem      = 1;
    else if ( !strcmp( argv[k], "-trace_mem" ) ) Trace_Mem = 1;
    else if ( !strcmp( argv[k], "-rgb" ) )       { To_RGB = 0x20565; VTrace = 0; }
    else if ( !strcmp( argv[k], "-rgb32" ) )     { To_RGB = 0x40888; VTrace = 0; }
    else if ( !strcmp( argv[k], "-mpg2" ) )      MPEG12 = 1;
    else if ( !strcmp( argv[k], "-avi" ) )       System_Stream = 1;
    else if ( !strcmp( argv[k], "-mpg" ) )       System_Stream = 2;
    else if ( !strcmp( argv[k], "-stream" ) )    Streaming = 1;
    else if ( !strcmp( argv[k], "-seq_code" ) )  Sequence_Codes = 1;
    else if ( !strcmp( argv[k], "-test" ) )     { Show=0; VTrace=0; To_RGB=0; From_Mem=1; Quiet=2; }
    else if ( !strcmp( argv[k], "-cpu" ) )       Cpu                = Get_Feature(argc, argv, k);
    else if ( !strcmp( argv[k], "-nb" ) )        Nb_Frames          = Parse_Int_Value(argc, argv, k, -1);
    else if ( !strcmp( argv[k], "-skip" ) )      Skip_Frames        = Parse_Int_Value(argc, argv, k, 0);
    else if ( !strcmp( argv[k], "-jump" ) )      Jump_To            = Parse_Int_Value(argc, argv, k, 0);
    else if ( !strcmp( argv[k], "-q" ) )         Global_Q           = Parse_Int_Value(argc, argv, k, 1, 31);
    else if ( !strcmp( argv[k], "-mv" ) )        MV_Size            = Parse_Int_Value(argc, argv, k, 1, 7);
    else if ( !strcmp( argv[k], "-qtype" ) )     Quant_Type         = Parse_Int_Value(argc, argv, k, 0, 1);
    else if ( !strcmp( argv[k], "-br" ) )        Bit_Rate           = Parse_Int_Value(argc, argv, k);
    else if ( !strcmp( argv[k], "-fps" ) )       FPS                = Parse_Float_Value(argc, argv, k, 0.);
    else if ( !strcmp( argv[k], "-pass" ) )      Pass_Nb            = Parse_Int_Value(argc, argv, k, 0, 2);
    else if ( !strcmp( argv[k], "-passrf" ) )    Pass_RF            = Parse_Float_Value(argc, argv, k, 0., 10.);
    else if ( !strcmp( argv[k], "-s" ) )         Search_Method      = Parse_Int_Value(argc, argv, k, 0);
    else if ( !strcmp( argv[k], "-metric" ) )    Search_Metric      = Parse_Int_Value(argc, argv, k, 0);
    else if ( !strcmp( argv[k], "-sad_lo" ) )    SAD_Skip_Limit     = Parse_Int_Value(argc, argv, k, 0);
    else if ( !strcmp( argv[k], "-sad_hi" ) )    SAD_Intra_Limit    = Parse_Int_Value(argc, argv, k, 0);
    else if ( !strcmp( argv[k], "-lambda" ) )    Lambda             = Parse_Float_Value(argc, argv, k, 0.);
    else if ( !strcmp( argv[k], "-gmc_acc" ) )   GMC_Accuracy       = Parse_Int_Value(argc, argv, k, 0, 3);
    else if ( !strcmp( argv[k], "-il" ) )        Intra_Limit        = Parse_Int_Value(argc, argv, k);
    else if ( !strcmp( argv[k], "-imax" ) )      Intra_Max          = Parse_Int_Value(argc, argv, k, 0);
    else if ( !strcmp( argv[k], "-4v" ) )        Inter4V_Probing    = Parse_Int_Value(argc, argv, k, 0);
    else if ( !strcmp( argv[k], "-fld" ) )       Field_Pred_Probing = Parse_Int_Value(argc, argv, k, 0);
    else if ( !strcmp( argv[k], "-ithresh" ) )   Inter_Thresh       = Parse_Int_Value(argc, argv, k);
    else if ( !strcmp( argv[k], "-dq" ) )        dQuant_Amp         = Parse_Float_Value(argc, argv, k);
    else if ( !strcmp( argv[k], "-buf_size" ) )  Enc_Buf_Size       = Parse_Int_Value(argc, argv, k, 0);
    else if ( !strcmp( argv[k], "-delay" ) )     Delay              = Parse_Int_Value(argc, argv, k, 0);
    else if ( !strcmp( argv[k], "-vtrace" ) )  { VTrace             = Parse_Int_Value(argc, argv, k, 0); To_RGB = 0; /*disable conversion*/ }
    else if ( !strcmp( argv[k], "-debug" ) )     Debug              = Parse_Int_Value(argc, argv, k);
    else if ( !strcmp( argv[k], "-adebug" ) )    ADebug             = Parse_Int_Value(argc, argv, k);
    else if ( !strcmp( argv[k], "-tt" ) )        Transfer_Type      = Parse_Int_Value(argc, argv, k);
    else if ( !strcmp( argv[k], "-sqcif" ) )   { YUV_Input = 1; YUV_W =  128; YUV_H =   96; }
    else if ( !strcmp( argv[k], "-qcif" ) )    { YUV_Input = 1; YUV_W =  176; YUV_H =  144; }
    else if ( !strcmp( argv[k], "-cif" ) )     { YUV_Input = 1; YUV_W =  352; YUV_H =  288; }
    else if ( !strcmp( argv[k], "-4cif" ) )    { YUV_Input = 1; YUV_W =  704; YUV_H =  576; }
    else if ( !strcmp( argv[k], "-pal" ) )     { YUV_Input = 1; YUV_W =  720; YUV_H =  576; }
    else if ( !strcmp( argv[k], "-ntsc" ) )    { YUV_Input = 1; YUV_W =  720; YUV_H =  480; }
    else if ( !strcmp( argv[k], "-720p" ) )    { YUV_Input = 1; YUV_W = 1280; YUV_H =  720; }
    else if ( !strcmp( argv[k], "-1080p" ) )   { YUV_Input = 1; YUV_W = 1920; YUV_H = 1080; }
    else if ( !strcmp( argv[k], "-yuv" ) ) {
      YUV_W = Parse_Int_Value(argc, argv, k);
      YUV_H = Parse_Int_Value(argc, argv, k);
      YUV_Input = 1;
    }
    else if ( !strcmp( argv[k], "-v" ) ) {
      printf( "%.2f\n", VERSION );
      exit(0);
    }
    else if ( !strcmp( argv[k], "-intl" ) ) {
      Interlace_DCT   = 1;  // decide
      Interlace_Field = 1;  // decide
    }
    else if ( !strcmp( argv[k], "-intldct" ) )
      Interlace_DCT = 2; // force
    else if ( !strcmp( argv[k], "-intlfld" ) )
      Interlace_Field = 2;  // force
    else if ( !strcmp( argv[k], "-o" ) )        Out_File      = Parse_String_Value(argc, argv, k);
    else if ( !strcmp( argv[k], "-passfile" ) ) Pass_File     = Parse_String_Value(argc, argv, k);
    else if ( !strcmp( argv[k], "-custom" ) )   Custom_Matrix = Parse_String_Value(argc, argv, k);
    else if ( !strcmp( argv[k], "-psnr_f" ) ) { Ref_YUV_File  = Parse_String_Value(argc, argv, k); Show_PSNR |= 8; }
    else if ( argv[k][0] == '-' )
      Skl_Throw( SKL_MSG_EXCEPTION("Unknown option '%s' !", argv[k]) );
    else In_File = argv[k];
  }

  if (In_File==0 && Out_File==0 && Pass_Nb!=1)
    Help(argv, "Missing input file!\n\n" );

  if (!Quiet) Quiet = (Save==2 || Save==3 || Debug>=1);
}

//////////////////////////////////////////////////////////
//
//   main(), at last
//
//////////////////////////////////////////////////////////

#ifdef _WINDOWS
#include <io.h>
#include <fcntl.h>
#endif

int main(int argc, char *argv[])
{

#ifdef _WINDOWS
  _setmode(_fileno(stdin), _O_BINARY);    /* thanks to Marcos Morais <morais at dee.ufcg.edu.br> */
  _setmode(_fileno(stdout), _O_BINARY);
#endif

  try {

    Parse_Options( argc, argv );
    if (!Quiet) Banner();

    if (Trace_Mem) // won't trace the codec in DLL, since it has its own memory arena.
      All_Mem = ::new SKL_MEM_TRC_I(2);
    else
      All_Mem = ::new SKL_MEM_I();
    if (All_Mem==0) throw SKL_MEM_EXCEPTION( "memory pool", 0 );

    if (Out_File==0 && Pass_Nb!=1) Decode_MPEG4(In_File);
    else Encode_MPEG4(Out_File, In_File);

  }
  catch(const SKL_EXCEPTION &e) {
    printf( "Caught exception: %s\n", e.Get_Message() );
    e.Print();
    return 1;
  }

  if (All_Mem!=0) ::delete All_Mem;

  return 0;
}

//////////////////////////////////////////////////////////
