/********************************************************
 * Some code. Copyright (C) 2003 by Pascal Massimino.   *
 * All Rights Reserved.      (http://skal.planet-d.net) *
 * For Educational/Academic use ONLY. See 'LICENSE.TXT'.*
 ********************************************************/
/*
 * skl_mpg4_tbl.cpp
 *
 * MPEG4 tables and utils
 ********************************************************/

#include "./skl_mpg4i.h"
#include "skl_syst/skl_exception.h"

#define MB_FENCE 0   // number of extraneous edges (for Valgrind/purify)

//////////////////////////////////////////////////////////

const int SKL_MP4_I::Scan_Order[4][64] = {
          /* zigzag  */
  {  0,  1,  8, 16,  9,  2,  3, 10
  , 17, 24, 32, 25, 18, 11,  4,  5
  , 12, 19, 26, 33, 40, 48, 41, 34
  , 27, 20, 13,  6,  7, 14, 21, 28
  , 35, 42, 49, 56, 57, 50, 43, 36
  , 29, 22, 15, 23, 30, 37, 44, 51
  , 58, 59, 52, 45, 38, 31, 39, 46
  , 53, 60, 61, 54, 47, 55, 62, 63 },

         /* horiz. */
  {  0,  1,  2,  3,  8,  9, 16, 17
  , 10, 11,  4,  5,  6,  7, 15, 14
  , 13, 12, 19, 18, 24, 25, 32, 33
  , 26, 27, 20, 21, 22, 23, 28, 29
  , 30, 31, 34, 35, 40, 41, 48, 49
  , 42, 43, 36, 37, 38, 39, 44, 45
  , 46, 47, 50, 51, 56, 57, 58, 59
  , 52, 53, 54, 55, 60, 61, 62, 63 },

         /* vert. */
  {  0,  8, 16, 24,  1,  9,  2, 10
  , 17, 25, 32, 40, 48, 56, 57, 49
  , 41, 33, 26, 18,  3, 11,  4, 12
  , 19, 27, 34, 42, 50, 58, 35, 43
  , 51, 59, 20, 28,  5, 13,  6, 14
  , 21, 29, 36, 44, 52, 60, 37, 45
  , 53, 61, 22, 30,  7, 15, 23, 31
  , 38, 46, 54, 62, 39, 47, 55, 63 },

        /* transposed zigzag  */
  {  0,  8,  1,  2,  9, 16, 24, 17 
  , 10,  3,  4, 11, 18, 25, 32, 40 
  , 33, 26, 19, 12,  5,  6, 13, 20 
  , 27, 34, 41, 48, 56, 49, 42, 35 
  , 28, 21, 14,  7, 15, 22, 29, 36 
  , 43, 50, 57, 58, 51, 44, 37, 30 
  , 23, 31, 38, 45, 52, 59, 60, 53
  , 46, 39, 47, 54, 61, 62, 55, 63 }
};

const SKL_BYTE SKL_MP4_I::Dflt_Mtx[3][64] = {
 {    // Dflt_Intra
   8, 17, 18, 19, 21, 23, 25, 27
, 17, 18, 19, 21, 23, 25, 27, 28
, 20, 21, 22, 23, 24, 26, 28, 30
, 21, 22, 23, 24, 26, 28, 30, 32
, 22, 23, 24, 26, 28, 30, 32, 35
, 23, 24, 26, 28, 30, 32, 35, 38
, 25, 26, 28, 30, 32, 35, 38, 41
, 27, 28, 30, 32, 35, 38, 41, 45 },

 {    // Dflt_Inter
  16, 17, 18, 19, 20, 21, 22, 23
, 17, 18, 19, 20, 21, 22, 23, 24
, 18, 19, 20, 21, 22, 23, 24, 25
, 19, 20, 21, 22, 23, 24, 26, 27
, 20, 21, 22, 23, 25, 26, 27, 28
, 21, 22, 23, 24, 26, 27, 28, 30
, 22, 23, 24, 26, 27, 28, 30, 31
, 23, 24, 25, 27, 28, 30, 31, 33 },

 {    // Dflt_Intra_MPEG2
   8, 16, 19, 22, 26, 27, 29, 34
, 16, 16, 22, 24, 27, 29, 34, 37
, 19, 22, 26, 27, 29, 34, 34, 38
, 22, 22, 26, 27, 29, 34, 37, 40
, 22, 26, 27, 29, 32, 35, 40, 48
, 26, 27, 29, 32, 35, 40, 48, 58
, 26, 27, 29, 34, 38, 46, 56, 69
, 27, 29, 35, 38, 46, 56, 69, 83 }
};

static const SKL_BYTE DC_Q_Tab_MPEG1[31] = {
        2,  4,  6,  8, 10, 12, 14
 , 16, 18, 20, 22, 24, 26, 28, 30
 , 32, 34, 36, 38, 40, 42, 44, 46
 , 48, 50, 52, 54, 56, 58, 60, 62 };

static const SKL_BYTE DC_Q_Tab_MPEG2_0[31] = {
        1,  2,  3,  4,  5,  6,  7
 ,  8,  9, 10, 11, 12, 13, 14, 15
 , 16, 17, 18, 19, 20, 21, 22, 23
 , 24, 25, 26, 27, 28, 29, 30, 31 };

static const SKL_BYTE DC_Q_Tab_MPEG2_1[31] = {
        1,  2,  3,  4,  5,   6,   7
 ,  8, 10, 12, 14, 16, 18,  20,  22
 , 24, 28, 32, 36, 40, 44,  48,  52
 , 56, 64, 72, 80, 88, 96, 104, 112 };

static const SKL_BYTE * Q_Scale_Maps[] = {
  0L, 0L,                           // MPEG4
  DC_Q_Tab_MPEG1,                   // MPEG1
  DC_Q_Tab_MPEG1, DC_Q_Tab_MPEG2_1  // MPEG2 qscale_type=0,1
};

//////////////////////////////////////////////////////////
//  Used as hint for sparse DCTs
// bit8: if set, Input is not only upper-left 4x4
// bit0-7: corresponding row has non-zero coeff

const int SKL_MP4_I::Row_From_Index[64] =
{
  0x001,0x001,0x001,0x001, 0x101,0x101,0x101,0x101,
  0x002,0x002,0x002,0x002, 0x102,0x102,0x102,0x102,
  0x004,0x004,0x004,0x004, 0x104,0x104,0x104,0x104,
  0x008,0x008,0x008,0x008, 0x108,0x108,0x108,0x108,

  0x110,0x110,0x110,0x110, 0x110,0x110,0x110,0x110,
  0x120,0x120,0x120,0x120, 0x120,0x120,0x120,0x120,
  0x140,0x140,0x140,0x140, 0x140,0x140,0x140,0x140,
  0x180,0x180,0x180,0x180, 0x180,0x180,0x180,0x180
};

//////////////////////////////////////////////////////////
// Table for converting from internal MB_Type 
// to SKL_MP4_MAP::Type public type.

const SKL_UINT8 SKL_MB::MB_To_Map_Type[SKL_MB_LAST] =
{
  SKL_MAP_16x16, SKL_MAP_16x16,  SKL_MAP_8x8,
  SKL_MAP_INTRA, SKL_MAP_INTRA,  SKL_MAP_SKIPPED
};

//////////////////////////////////////////////////////////

/*
  // TODO: test in progress...
extern const SKL_INT32 Skl_Div18[];

#define BITS 18 // minimum allowed.
#define FIX(a) ((1<<BITS)/(a) + 1)
const SKL_INT32 Skl_Div18[47] = { 
          FIX( 1),FIX( 2),FIX( 3),FIX( 4),FIX( 5),FIX( 6),FIX( 7),
  FIX( 8),FIX( 9),FIX(10),FIX(11),FIX(12),FIX(13),FIX(14),FIX(15),
  FIX(16),FIX(17),FIX(18),FIX(19),FIX(20),FIX(21),FIX(22),FIX(23),
  FIX(24),FIX(25),FIX(26),FIX(27),FIX(28),FIX(29),FIX(30),FIX(31),
  FIX(32),FIX(33),FIX(34),FIX(35),FIX(36),FIX(37),FIX(38),FIX(39),
  FIX(40),FIX(41),FIX(42),FIX(43),FIX(44),FIX(45),FIX(46),FIX(47)
};
#define FDIV(a,b) ( (((a)+((b)>>1))*Skl_Div18[(b)-1])>>BITS )
#define RDIV(a,b) ( ((a)+((b)>>1))/(b) )
#define DIV_ROUND(x,y)  (FDIV((x),(y))-((x)<0))
*/

//////////////////////////////////////////////////////////
// SKL_MP4_INFOS
//////////////////////////////////////////////////////////

SKL_MP4_INFOS::SKL_MP4_INFOS(SKL_MEM_I *Memory) : Mem(Memory) {}

SKL_MEM_I *SKL_MP4_INFOS::Set_Memory_Manager(SKL_MEM_I *New_Mem)
{
  SKL_MEM_I *Old_Mem = Mem;
  Mem = New_Mem;
  return Old_Mem;
}

//////////////////////////////////////////////////////////
// SKL_MP4_I
//////////////////////////////////////////////////////////

SKL_MP4_I::SKL_MP4_I(SKL_MEM_I *Memory)
{
  Set_Memory_Manager(Memory);
  Cpu = SKL_CPU_LAST;

  Set_Slicer(0,0);

  const int Q_Size = 4*sizeof(Q_Intra[0]) + SKL_ALIGN;
  Q_Base = (SKL_ANY)malloc(2*Q_Size + SKL_ALIGN);
  if (Q_Base==0)
    Skl_Throw( SKL_MEM_EXCEPTION("Q_Base", 2*Q_Size) );
  Q_Intra = (SKL_QUANTIZER)SKL_ALIGN_PTR( Q_Base, SKL_ALIGN );
  Q_Inter = &Q_Intra[2];

  Quant_Type = 1;
  Get_Default_Matrix( Intra_Matrix, 0 );
  Get_Default_Matrix( Inter_Matrix, 1 );
  Custom_Intra = 0;
  Custom_Inter = 0;

  Set_CPU( SKL_CPU_C ); // default guess

  MPEG_Version = 0; // unknown yet
  Debug_Level = 0;
  VOL_Id = -1;

  Frame_Number = 0;
  Time_Ref        = 0;
  Time_Last_Coded = 0;
  Time_TFrame     = 0;
  Set_Time_Frequency( 25 ); // default: 25hz clock
  Ticks_Per_VOP = 0;        // disabled

  Coding     = I_VOP;
  Rounding   = 0;
  Set_Rounding();   // sanity check
  Fwd_Code   = -1;  // sanity check..
  Bwd_Code   = -1;  // this is decided
  Quant      = -1;  // .. by the analyzer
  DC_Thresh = 0;
  Top_Field_First = 1;
  Alt_Vert_Scan   = 0;  
  Field_Dir       = 0x00;

  Pic_Base = 0;
  Preds = 0;
  Nb_Pics = 0;
  Nb_Maps = 0;
  Reset_VOL();
  Clear_Pics();
}

SKL_MP4_I::~SKL_MP4_I()
{
  Clear_Pics();
  free(Q_Base);
}

void SKL_MP4_I::Set_Slicer(SKL_MP4_SLICER S, SKL_ANY Data)
{
  Slicer      = S;
  Slicer_Data = Data;
}

//////////////////////////////////////////////////////////

void SKL_MP4_I::Set_Rounding()
{
  Add_Ops  = MB_Ops.Add;    // rounding is always 0 for B-vop
  Copy_Ops = MB_Ops.Copy[Rounding];
}

void SKL_MP4_I::Make_Edges_Dec(const SKL_MP4_PIC * const Pic) const
{
  SKL_BYTE * const YUV[3] = { Pic->Y, Pic->U, Pic->V };
  MB_Ops.Make_Edges( YUV, MB_W<<4, MB_H<<4, BpS);
}


void SKL_MP4_I::Make_Edges_Enc(const SKL_MP4_PIC * const Pic) const
{
  SKL_BYTE * const YUV[3] = { Pic->Y, Pic->U, Pic->V };
  MB_Ops.Make_Edges( YUV, Pic->Width, Pic->Height, BpS);
}

//////////////////////////////////////////////////////////

void SKL_MP4_I::Set_CPU(SKL_CPU_FEATURE New_Cpu)
{
  if (New_Cpu==SKL_CPU_LAST)
    New_Cpu = Skl_Detect_CPU_Feature();
  if (New_Cpu==Cpu)
    return;

  Cpu = New_Cpu;
  Skl_Init_Img_DSP( &Img_Ops, Cpu );
  Skl_Init_Mb_DSP ( &MB_Ops,  Cpu );
  Skl_Init_GMC_DSP( &GMC_Ops, Cpu );

  int Saved_Quant_Type = Quant_Type;
  Quant_Type = -1;      // will force re-init of _Quant_Ops...
  Set_Quant_Type( Saved_Quant_Type );

  Img_Dsp  = &Img_Ops; // in SKL_MP4_INFOS
  MB_Dsp   = &MB_Ops;  // in SKL_MP4_INFOS
  GMC_Dsp  = &GMC_Ops; // in SKL_MP4_INFOS
  Add_Ops  = 0;        // sanity check
  Copy_Ops = 0;        // ""
}

//////////////////////////////////////////////////////////

void SKL_MP4_I::Set_Time_Frequency(int Freq)
{
  Time_Frequency = Freq;
  Ticks_Bits = SKL_BMASKS::Log2( Freq-1 );
  if (Ticks_Bits<1) Ticks_Bits = 1;
}

void SKL_MP4_I::Init_VOL(int id)
{
  SKL_ASSERT(id>=0 && id<=0x1f);

     // TODO: improve id managment for multi-VOL?
  if (VOL_Id!=id) {  
    VOL_Id = id;
  }
  MPEG_Version = 4;
  Reset_VOL();
}

void SKL_MP4_I::Reset_VOL()  // this is roughly table 6-24
{
  Shape      = 0;
  Low_Delay  = 1;
  Not_8b     = 0;
  Quant_Prec = 5;
  Bpp        = 8;
  Interlace  = 0;

  Quarter     = 0;
  Resync      = 0;
  Reduced_VOP =-1; // disabled
  New_Pred    = 0;
  Sprite_Mode = SPRITE_NONE;
  Sprite_Nb_Pts = 0;
  Sprite_Accuracy = 3;
  Sprite_Transmit = SPRITE_PIECE;
  Data_Partitioned = 0;
  Rev_VLC     = 0;
  CE_Flags    = 0;
  Time_TFrame = 0; // invalided. Will be computed when first BVop is encountered.
}

//////////////////////////////////////////////////////////

void SKL_MP4_I::Set_Quant_Type(int Type)
{
  if (Type!=Quant_Type) {
    Quant_Type = Type;
    Skl_Init_Quant_DSP( &Quant_Ops, Cpu, (Quant_Type>=2) ? 2 : Quant_Type );
    Quant_Ops.Init_Quantizer(Q_Intra, Intra_Matrix, Q_Scale_Maps[Quant_Type], 1);
    Quant_Ops.Init_Quantizer(Q_Inter, Inter_Matrix, Q_Scale_Maps[Quant_Type], 0);
  }
}

void SKL_MP4_I::Get_Default_Matrix(SKL_BYTE M[64], int What)
{
  if (What<3) for(int i=0; i<64; ++i) M[i] = Dflt_Mtx[What][i];
  else /*if (What==3)*/ for(int i=0; i<64; ++i) M[i] = 16;  // Dflt_Inter_MPEG2
}

int SKL_MP4_I::Set_Matrix(int Intra, const SKL_BYTE *M)
{
  SKL_BYTE * const Dst = Intra ? Intra_Matrix : Inter_Matrix;

  int Diff = 0;
  for(int i=0; i<64; ++i) {
    Diff |= (Dst[i] != M[i]);
    Dst[i] = M[i];
  }
  if (Intra) {
    Custom_Intra = Diff;
    if (Diff)
      Quant_Ops.Init_Quantizer(Q_Intra, Intra_Matrix, Q_Scale_Maps[Quant_Type], 1);
  }
  else {
    Custom_Inter = Diff;
    if (Diff)
      Quant_Ops.Init_Quantizer(Q_Inter, Inter_Matrix, Q_Scale_Maps[Quant_Type], 0);
  }
  return Diff;
}

//////////////////////////////////////////////////////////
// Frame memory allocation

void SKL_MP4_I::Init_Pics(int W, int H, int Nb_pics, int Nb_maps)
{
  int i;
  SKL_ASSERT(Nb_Pics<=MAX_PICS && Nb_Maps<=MAX_MAPS);

  const int w = (W+15)/16;
  const int h = (H+15)/16;

  if (w==MB_W && h==MB_H && Nb_Pics==Nb_pics && Nb_Maps==Nb_maps)
    return;

  Clear_Pics();
  Width  = W;
  Height = H;
  MB_W = w;
  MB_H = h;
  EMB_W = w+2*(1+MB_FENCE);
  EMB_H = h+2*(1+MB_FENCE);
  BpS = EMB_W*16;
  MV_Stride = 2*EMB_W;

  Nb_Pics = Nb_pics;
  Nb_Maps = Nb_maps;

  const int YUV_Size = EMB_H*8*BpS * 3;
  const int MV_Size = (EMB_W*EMB_H)*4 * sizeof(SKL_MV);
  const int Map_Size = (MB_W*MB_H)*sizeof(SKL_MP4_MAP);
  const int New_Size = YUV_Size * Nb_Pics + (Map_Size+MV_Size) * Nb_Maps + SKL_ALIGN;
  Pic_Base = (Mem==0) ? (SKL_BYTE*)malloc(New_Size) 
                      : (SKL_BYTE*)Mem->New(New_Size);
  if (Pic_Base==0)
    Skl_Throw( SKL_MEM_EXCEPTION("Pic_Base", New_Size) );

  Pic_Size = New_Size;

  const int Edge_Offset = (16*BpS + 16)*(1+MB_FENCE);
  int Y_Off = Edge_Offset;
  int U_Off = (EMB_W*EMB_H)*16*16 + Edge_Offset/2;
  int V_Off = U_Off + EMB_W*8;
  SKL_BYTE *YUV_Base = (SKL_BYTE*)SKL_ALIGN_PTR(Pic_Base, SKL_ALIGN);
  SKL_BYTE *MV_Base  = YUV_Base + Nb_Pics*YUV_Size;
  SKL_BYTE *Map_Base = MV_Base + Nb_Maps*MV_Size;
  const int MV_Offset = (2+MB_FENCE)*(1 + (EMB_W*2)) * sizeof(SKL_MV);
  MV_Base += MV_Offset;   // + edge offset
  for(i=0; i<Nb_Pics; ++i)
  {
    All_Pics[i].Width  = Width;
    All_Pics[i].Height = Height;
    All_Pics[i].BpS    = BpS;
    All_Pics[i].Y = YUV_Base + Y_Off;
    All_Pics[i].U = YUV_Base + U_Off;
    All_Pics[i].V = YUV_Base + V_Off;
    All_Pics[i].MV  = 0;
    All_Pics[i].Map = 0;
    All_Pics[i].Time = (double)(SKL_INT64)Time_Ref;
    All_Pics[i].Time_Ticks = Time_Ref*Time_Frequency;
    All_Pics[i].Data = 0;
    YUV_Base += YUV_Size;
  }
  for( ; i<MAX_PICS; ++i) { // sanity check
    All_Pics[i].Y  = 0;
    All_Pics[i].U  = 0;
    All_Pics[i].V  = 0;
    All_Pics[i].MV = 0; 
    All_Pics[i].Map = 0;
    All_Pics[i].Data = 0;
  }

  for(i=0; i<Nb_Maps; ++i)
  {
    All_Maps[i].Map = (SKL_MP4_MAP*)Map_Base;
    All_Maps[i].MV  = (SKL_MV*)MV_Base; 
    Map_Base += Map_Size;
    MV_Base  += MV_Size;
  }
  for( ; i<MAX_MAPS; ++i) {
    All_Maps[i].Map = 0;  // sanity check
    All_Maps[i].MV = 0;
  }
    
    // Encoder: the boundary motion vectors are 
    // invariantly INVALID. Marks them as such
  for(i=0; i<Nb_Maps; ++i)
  {
    int x, y;
    SKL_MV *MVs  = All_Maps[i].MV;
    for(x=-1; x<=2*MB_W; ++x)    // top row
      MARK_INVALID_MV( MVs[-MV_Stride + x] );
    for(y=0; y<2*MB_H; ++y) {    // left/right columns
      MARK_INVALID_MV( MVs[-1]);
      MARK_INVALID_MV( MVs[2*MB_W]);
      MVs += MV_Stride;
    }
    for(x=-1; x<=2*MB_W; ++x)    // bottom row (for ME)
      MARK_INVALID_MV( MVs[x] );
  }

  Nb_Preds = (2*MB_W+2)*3 + (2*MB_W+4)*2;
  int Data_Size = Nb_Preds * sizeof(SKL_MB_DATA);
  Preds = (Mem==0) ? (SKL_MB_DATA*)malloc( Data_Size )
                   : (SKL_MB_DATA*)Mem->New( Data_Size );
  if (Preds==0)
    Skl_Throw( SKL_MEM_EXCEPTION("SKL_MB_DATA", Data_Size) );

  Y_Preds[0] = Preds + 1;
  Y_Preds[1] = Y_Preds[0] + (2*MB_W+2);
  Y_Preds[2] = Y_Preds[1] + (2*MB_W+2);
  C_Preds[0] = Y_Preds[2] + (2*MB_W+2);
  C_Preds[1] = C_Preds[0] + (2*MB_W+4);

    // the boundary left/right blocks are
    // invariant along every frames
    // => Initialize'em once for all.
  Y_Preds[0][      -1].Set_Not_Intra();   // Y1 (Tops[0])
  Y_Preds[0][  2*MB_W].Set_Not_Intra();
  Y_Preds[1][      -1].Set_Not_Intra();   // Y1 (Curs[0])
  Y_Preds[1][  2*MB_W].Set_Not_Intra();
  Y_Preds[2][      -1].Set_Not_Intra();   // Y2 (Curs[2])
  Y_Preds[2][  2*MB_W].Set_Not_Intra();
  C_Preds[0][      -1].Set_Not_Intra();   // U  (Tops[4])
  C_Preds[0][  MB_W+0].Set_Not_Intra();
  C_Preds[0][  MB_W+1].Set_Not_Intra();   // V  (Tops[5])
  C_Preds[0][2*MB_W+2].Set_Not_Intra();
  C_Preds[1][      -1].Set_Not_Intra();   // U  (Curs[4])
  C_Preds[1][  MB_W+0].Set_Not_Intra();
  C_Preds[1][  MB_W+1].Set_Not_Intra();   // V  (Curs[5])
  C_Preds[1][2*MB_W+2].Set_Not_Intra();
}

void SKL_MP4_I::Clear_Pics()
{
  int i;

  Width  = 0;
  Height = 0;
  BpS    = 0;
  MB_W   = 0;
  MB_H   = 0;
  EMB_W  = 0;
  EMB_H  = 0;

  for(i=0; i<MAX_PICS; ++i) {
    All_Pics[i].Y   = 0;
    All_Pics[i].U   = 0;
    All_Pics[i].V   = 0;
    All_Pics[i].MV  = 0;
    All_Pics[i].Map = 0;
  }
  for(i=0; i<MAX_MAPS; ++i) {
    All_Maps[i].Map = 0;
    All_Maps[i].MV  = 0;
  }

  if (Pic_Base!=0) {
    if (Mem==0) free( Pic_Base );
    else Mem->Delete( Pic_Base, Pic_Size );
  }
  Pic_Size = 0;
  Pic_Base = 0;
  Nb_Pics = 0;

  if (Preds) {
    if (Mem==0) free( Preds );
    else Mem->Delete( Preds, Nb_Preds * sizeof(SKL_MB_DATA) );
  }
  Preds = 0;
  Nb_Preds = 0;
  Y_Preds[0] = 0;
  Y_Preds[1] = 0;
  Y_Preds[2] = 0;
  C_Preds[0] = 0;
  C_Preds[1] = 0;

  Past    = &All_Pics[0];
  Future  = &All_Pics[1];
  Aux     = &All_Pics[2];
  Cur     = 0;
  Last    = 0;
  Cur_Map = 0;
}

int SKL_MP4_I::Sanity_Check_Preds() const
{
  if (Y_Preds[0][      -1].Q!=0 ||   // Y1 (Tops[0])
      Y_Preds[0][  2*MB_W].Q!=0 ||
      Y_Preds[1][      -1].Q!=0 ||   // Y1 (Curs[0])
      Y_Preds[1][  2*MB_W].Q!=0 ||
      Y_Preds[2][      -1].Q!=0 ||   // Y2 (Curs[2])
      Y_Preds[2][  2*MB_W].Q!=0 ||
      C_Preds[0][      -1].Q!=0 ||   // U  (Tops[4])
      C_Preds[0][  MB_W+0].Q!=0 ||
      C_Preds[0][  MB_W+1].Q!=0 ||   // V  (Tops[5])
      C_Preds[0][2*MB_W+2].Q!=0 ||
      C_Preds[1][      -1].Q!=0 ||   // U  (Curs[4])
      C_Preds[1][  MB_W+0].Q!=0 ||
      C_Preds[1][  MB_W+1].Q!=0 ||   // V  (Curs[5])
      C_Preds[1][2*MB_W+2].Q!=0)
    return 0;

  for(int i=0; i<Nb_Pics; ++i)
  {
    int x, y;
    SKL_MV *MVs  = All_Pics[i].MV;
    if (MVs==0) continue; // no map assigned to this pic yet...
    for(x=-1; x<=2*MB_W; ++x)    // top row
      if (IS_VALID_MV( MVs[-MV_Stride + x] ))
        return 0;
    for(y=0; y<2*MB_H; ++y) {    // left/right columns
      if (IS_VALID_MV( MVs[-1]) || IS_VALID_MV( MVs[2*MB_W]))
        return 0;
      MVs += MV_Stride;
    }
    for(x=-1; x<=2*MB_W; ++x)    // bottom row
      if (IS_VALID_MV( MVs[x] ))
        return 0;
  }

  return 1;
}

void SKL_MP4_I::Get_All_Frames(SKL_MP4_PIC *Pic) const
{
  SKL_ASSERT(Pic!=0);
  Pic->Y = (SKL_BYTE*)SKL_ALIGN_PTR(Pic_Base, SKL_ALIGN);
  Pic->U = 0;
  Pic->V = 0;
  Pic->Map = 0;
  Pic->MV  = 0;
  Pic->Width  = BpS;
  Pic->Height = (Height*3/2 + 3*(1+MB_FENCE)*16) * Nb_Pics;
  Pic->BpS    = BpS;
}

void SKL_MP4_I::Copy_Pic(SKL_MP4_PIC *Dst, const SKL_MP4_PIC *Src) const
{
    // this not-coded frame is not supposed to be referenced
    // for backward prediction => we don't copy MV

  const int YUV_Offset = (16*BpS + 16)*(1+MB_FENCE);
  const int YUV_Size = EMB_H*8*BpS * 3;
  SKL_MEMCPY(Dst->Y-YUV_Offset, Src->Y-YUV_Offset, YUV_Size);

  if (Dst->Map!=0 && Src->Map!=0)
    SKL_MEMCPY(Dst->Map, Src->Map, MB_W*MB_H*sizeof(SKL_MP4_MAP) );
}

//////////////////////////////////////////////////////////
// Debugging
//////////////////////////////////////////////////////////

static void Print_Pred(SKL_MB_DATA *P, int What)
{
  if (What==0) printf( "%2d", P->Q );
  else if (What==1) printf( "%c", P->Q==0 ? '.' : 'I' );
  else if (What==2) printf( "%x", P->DC/16/8 );    
}

void SKL_MP4_I::Dump_Line(int What, const SKL_MB * const MB) const
{
  int i;
  SKL_MB_DATA *Y1  = Y_Preds[1];
  SKL_MB_DATA *Top = Y_Preds[MB->y&1 ? 2 : 0];
  SKL_MB_DATA *Y2  = Y_Preds[MB->y&1 ? 0 : 1];
  SKL_MB_DATA *CC  = C_Preds[MB->y&1 ? 0 : 1];

  const int W = 2*MB_W;
  if (MB->y==0) {
    printf( "[top]" );
    for(i=-1; i<=W; i++) Print_Pred( &Top[i], What );
    printf("  Frame #%d\n", Frame_Number);
  }
  printf( "[%3d]", MB->y);
  for(i=-1; i<=W; i++) Print_Pred( &Y1[i], What );
  printf("\n");
  printf( "[---]" );
  for(i=-1; i<=W; i++) Print_Pred( &Y2[i], What );
  printf("\n");
/*
  printf( "[cc]" );
  for(i=-1; i<=W/2; i++) {
    Print_Pred( &CC[i], What );
    Print_Pred( &CC[i+W/2+2], What );
  }
  printf("\n");
*/
  (void)CC;
}

static void Draw_Line(int x0, int y0, int x1, int y1,
                      const SKL_MP4_PIC * const Pic)
{
// #define PLOT(x) (x) = ((x)>>7)-1
// #define PLOT(x) (x) ^= 0xff
#define PLOT(x) (x) = 0xc0

  // It's as ugly & slow a 16:16 Bresenham as it can. 

  const int W = Pic->Width; 
  const int H = Pic->Height;
  const int BpS = Pic->BpS;

  if (y1<y0) { int tmp = y0; y0=y1; y1=tmp; tmp=x0; x0=x1; x1=tmp; }
  if (y1<0 || y0>=H) return;

  int dx = x1-x0;
  int dy = y1-y0;

  if (dy==0) {
    if (dx==0) return;
  }
  else {
    if (y0<0) { x0 -= y0*dx/dy; y0 = 0; }
    if (y1>=H) { x1 += (H-1-y1)*dx/dy; y1 = H-1; }
  }

  if (dx>0) {
    if (x0>=W || x1<0) return;
    if (x0<0)  { y0 -= x0*dy/dx; x0 = 0; }
    if (x1>=W) { y1 += (W-1-x1)*dy/dx; x1 = W-1; }
  }
  else if (dx<0) {
    if (x1>=W || x0<0) return;
    if (x1<0)  { y1 -= x1*dy/dx; x1 = 0; }
    if (x0>=W) { y0 += (W-x0)*dy/dx; x0 = W; }
  }
  else {
    if (x0<0 || x0>=W) return;
  }

  int Err = 0x10000;
  int d1, d2, l, dErr;
  if ( dx<=dy && dx>=-dy )    // >=45'
  {
    if (dx>0) { dErr= Err*dx/dy; d2= 1; }
    else      { dErr=-Err*dx/dy; d2=-1; }
    d1 = BpS;
    l = y1-y0;
  }
  else // <45'
  {
    if (dx>=0) { dErr= Err*dy/dx; d1 = 1; l = x1-x0; }
    else       { dErr=-Err*dy/dx; d1 =-1; l = x0-x1; }
    d2 = BpS;
  }

  SKL_BYTE *Dst = Pic->Y + x0 + y0*BpS;
  while(l-->0) {
    PLOT(*Dst);
    Dst += d1;
    if ((Err-=dErr)<0) 
    {
      Err += 0x10000;
      Dst += d2;
    }
  }
#undef PLOT
}

void SKL_MP4_I::Dump_MVs(const SKL_MP4_PIC * const Pic, int Incr) const
{
#if 0           // DEBUG Draw_Line()
  static int Cnt = 0;
  for(int i=0; i<=100; ++i) {
    int dx = (int)( 300.*cos(M_PI*(i*32+(Cnt&31))/100./32.) );
    int dy = (int)( 300.*sin(M_PI*(i*32+(Cnt&31))/100./32.) );
    Draw_Line(100-dx, 100-dy, 100+dx, 100+dy, Pic);
  }
  Cnt++;
  return;
#endif

  const SKL_MV *MVs = Pic->MV;
  const int Shift = Quarter ? 2 : 1;
  for(int y=4*Incr; y<Pic->Height; y+=Incr*8) {
    int x, i;
    for(x=4*Incr, i=0; x<Pic->Width; x+=Incr*8, i++)
      Draw_Line(x,y, x+(MVs[i][0]>>Shift), y+(MVs[i][1]>>Shift), Pic);
    MVs += MV_Stride*Incr;
  }
}

void SKL_MP4_I::Draw_GMC(const SKL_MP4_PIC * const Pic) const
{
#if 0
  int x0, y0, x1, y1, x2, y2, x3, y3;
  x0 = S_Warp_Pts[0][0]/2;
  y0 = S_Warp_Pts[0][1]/2;
  x1 = x0 + S_W + S_Warp_Pts[1][0]/2;
  y1 = y0       + S_Warp_Pts[1][1]/2;
  x2 = x0       + S_Warp_Pts[2][0]/2;
  y2 = y0 + S_H + S_Warp_Pts[2][1]/2;
  x3 = x0 + S_W + S_Warp_Pts[1][0]/2 + S_Warp_Pts[2][0]/2 + S_Warp_Pts[3][0]/2;
  y3 = y0 + S_H + S_Warp_Pts[1][1]/2 + S_Warp_Pts[2][1]/2 + S_Warp_Pts[3][1]/2;

  Draw_Line(x0,y0, x1,y1, Pic);
  Draw_Line(x1,y1, x3,y3, Pic);
  Draw_Line(x3,y3, x2,y2, Pic);
  Draw_Line(x2,y2, x0,y0, Pic);
#endif
#if 0
  const int W = Pic->Width/4;
  const int H = Pic->Height/4;
  Draw_Line(  W,  H,   3*W,  H,  Pic);
  Draw_Line(  W,3*H,   3*W,3*H,  Pic);
  Draw_Line(  W,  H,     W,3*H,  Pic);
  Draw_Line(3*W,  H,   3*W,3*H,  Pic);
#endif

  int n;
  static int Cnt = 1;
  static SKL_MV P[4];
  if (!--Cnt) {
    Cnt = 150;
    const int W = Pic->Width << 2;
    const int H = Pic->Height << 2;
    P[0][0] =   W; P[0][1] =   H;
    P[1][0] = 3*W, P[1][1] =   H;
    P[2][0] = 3*W; P[2][1] = 3*H;
    P[3][0] =   W, P[3][1] = 3*H;
  }
  for(n=0; n<4; ++n) {
    int mv[2];
    GMC_Ops.Get_Average_MV(mv, P[n][0]>>8, P[n][1]>>8, Quarter);
    P[n][0] -= mv[0]<<(3-Quarter); P[n][1] -= mv[1]<<(3-Quarter);
  }
  for(n=0; n<4; ++n) {
    const int m = (n+1)%4;
    Draw_Line(  P[n][0]>>4,P[n][1]>>4,  P[m][0]>>4,P[m][1]>>4, Pic );
  }  
}

void SKL_MP4_I::Print_Infos() const
{
  if (Debug_Level==3) {
    printf( " Shape = %d ", Shape );
    printf( " Partitioned = %d ", Data_Partitioned );
    printf( " Time incr bits = %d Freq=%d\n",  Ticks_Bits, Time_Frequency );
    printf( " Size: %d x %d interlace:%d reduced:%d", Width, Height, Interlace, (Reduced_VOP>=0) );
    printf( " Quant prec %d bpp:%d QType:%d Quarter:%d\n",
      Quant_Prec, Bpp, Quant_Type, Quarter );
  }
  else if (Debug_Level==4) {    
    printf( "  Coding=[%c] Time:%lld Rnd=%d", "IPBS"[Coding], Cur->Time_Ticks, Rounding );
    if (Is_S_VOP() && Sprite_Mode==SPRITE_GMC)
      printf(" Spr_Nb:%d, Acc:%d ", Sprite_Nb_Pts, Sprite_Accuracy);
    else
      printf( " Reduced:%d NewPred:%d ", Reduced_VOP, New_Pred );
    printf( " FCode: %d/%d ", Fwd_Code, Bwd_Code );
    printf( " Q:%d DC-Trsh:%d \n", Quant, DC_Thresh );
  }
}

void SKL_MP4_FRAME::Dump_Map(SKL_MP4_PIC *Pic, int What) const
{
  printf( "---- Coding: %c-VOP -----------\n", "IPBD?"[Coding] );
  SKL_MP4_MAP *Map = Pic->Map;
  for(int y=0; y<MB_H; ++y) {
    for(int x=0; x<MB_W; ++x) {
      if (What==0) printf("%c", "sI.=4"[Map[x].Type]); // wow
      else if (What==1) printf("%.1x", Map[x].Type);   // MB_Type only
      else printf("%2d", Map[x].dQ); // dQ
    }
    Map += MB_W;
    printf("\n" );
  }
}

//////////////////////////////////////////////////////////
