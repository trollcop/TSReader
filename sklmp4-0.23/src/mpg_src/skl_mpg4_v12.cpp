/********************************************************
 * Some code. Copyright (C) 2003 by Pascal Massimino.   *
 * All Rights Reserved.      (http://skal.planet-d.net) *
 * For Educational/Academic use ONLY. See 'LICENSE.TXT'.*
 ********************************************************/
/*
 * skl_mpg4_v12.cpp
 *
 * simple, approximative, MPEG1/2 decoder
 * Are incorrect: oddification (MPEG1), mismatch control of
 *  MPEG2 inter blocks. And most probably, field motion is broken, even...
 * Not supported: 
 *   Field frames (Bweurrrk) and of course scalable modes (who does??:).
 * 
 ********************************************************/

#include "./skl_mpg4i.h"

//////////////////////////////////////////////////////////

enum {

    // Start codes
  MPEG12_PICTURE_START    = 0x00000100
, MPEG12_SLICE_MIN        = 0x00000101
, MPEG12_SLICE_MAX        = 0x000001af
/*    reserved            = 0x000001b0 */
/*    reserved            = 0x000001b1 */
, MPEG12_USER_START       = 0x000001b2
, MPEG12_SEQ_START        = 0x000001b3
, MPEG12_SEQUENCE_ERROR   = 0x000001b4
, MPEG12_EXT_START        = 0x000001b5
/*    reserved            = 0x000001b6 */
, MPEG12_SEQ_END          = 0x000001b7
, MPEG12_GOP_START        = 0x000001b8
, MPEG12_LAST_CODE        = 0x000001ff

    // extension code id. (Table 6-2).
, MPEG12_SEQ_EXT_ID                   = 1
, MPEG12_SEQ_DISPLAY_EXT_ID           = 2
, MPEG12_QMATRIX_EXT_ID               = 3
, MPEG12_SEQ_SCALABLE_EXT_ID          = 5
, MPEG12_PIC_DISPLAY_EXT_ID           = 7
, MPEG12_PIC_CODING_EXT_ID            = 8
, MPEG12_PIC_SPATIAL_SCALABLE_EXT_ID  = 9
, MPEG12_PIC_TEMPORAL_SCALABLE_EXT_ID = 10

    // picture structure
, MPEG12_TOP_FIELD    = 1
, MPEG12_BOTTOM_FIELD = 2
, MPEG12_FRAME_PIC    = 3

    // MB types
, MPEG12_MBLK_ERROR   = 0x00
, MPEG12_MBLK_INTRA   = 0x01
, MPEG12_MBLK_PATTERN = 0x02
, MPEG12_MBLK_MBWD    = 0x04
, MPEG12_MBLK_MFWD    = 0x08
, MPEG12_MBLK_QUANT   = 0x10
, MPEG12_MBLK_STWCF   = 0x20  // Spatial-Temporal Weight Code Flag
, MPEG12_MBLK_PSTWC   = 0x40  // Permitted Spatial-Temporal Weight Class
, MPEG12_MBLK_MOTION = (MPEG12_MBLK_MFWD  | MPEG12_MBLK_MBWD)
, MPEG12_MBLK_QI     = (MPEG12_MBLK_INTRA | MPEG12_MBLK_QUANT)
, MPEG12_MBLK_PI     = (MPEG12_MBLK_INTRA | MPEG12_MBLK_PATTERN)

, MC_NONE  = 0x000
, MC_FIELD = 0x100
, MC_FRAME = 0x200
, MC_16x8  = 0x200
, MC_DMV   = 0x300 /* dual prime */
, MC_ALL   = 0x300

};

struct SKL_MB2 : public SKL_MB    // special cursor for MPEG1/2
{
  int DC[3];
  int DC_Q;
  int DC_Prec;
  SKL_MV PMV[2][2];   // [fwd/bwd][1rst pred/2nd pred]
  int Pic_Struct;
  int Concealment_MV;
  int Frame_Pred_DCT;

  const int *Scan_Order;
  void (*Read_Intra)(SKL_FBB*, SKL_MB2 *MB, SKL_INT16*);
  int (*Read_Inter)(SKL_FBB*, SKL_MB2 *MB, SKL_INT16*);
  SKL_FBB * const FBB;

  int Is_Intra() const         { return ((MB_Type & MPEG12_MBLK_INTRA)!=0); }
  int Is_Quant() const         { return ((MB_Type & MPEG12_MBLK_QUANT)!=0); }
  int Is_Intra_Quant() const   { return ((MB_Type & MPEG12_MBLK_QI)!=0); }
  int Is_Intra_Pattern() const { return ((MB_Type & MPEG12_MBLK_PI)!=0); }
  int Is_Pattern() const       { return ((MB_Type & MPEG12_MBLK_PATTERN)!=0); }
  int Is_Forward() const       { return ((MB_Type & MPEG12_MBLK_MFWD)!=0); }
  int Is_Backward() const      { return ((MB_Type & MPEG12_MBLK_MBWD)!=0); }
  int Has_Motion() const       { return ((MB_Type&MPEG12_MBLK_MOTION)!=0); }
  int Has_B_Motion() const     { return ((MB_Type&MPEG12_MBLK_MOTION)==MPEG12_MBLK_MOTION); }

  int Motion() const           { return (MB_Type&MC_ALL); }

  int Is_Frame() const           { return (Pic_Struct==MPEG12_FRAME_PIC); }
  int Has_Concealment_MV() const { return (Concealment_MV!=0); }
  int Is_DC_Predicted() const    { return (Frame_Pred_DCT!=0); }

  void Reset_DC() {
    DC[0] = DC[1] = DC[2] = 1<<(7+DC_Prec);
  }
  void Reset_Motion_Type();
  void Reset_Field_Motion_Type();
  void Reset_Fwd() { SKL_ZERO_MV(PMV[0][0]); SKL_ZERO_MV(PMV[0][1]); }
  void Reset_Bwd() { SKL_ZERO_MV(PMV[1][0]); SKL_ZERO_MV(PMV[1][1]); }

  inline int MV_Outside(const SKL_MV MV) const;
  inline int Field_MVs_Outside(const SKL_MV MV[2]) const;
  void Predict_MPEG1();
  void Predict_Frame();
  void Predict_Fields();
  void Predict_DMV();
  void Predict_Reuse();
  void Copy_Fwd() const { Predict_With_0MV(VOL->Copy_Ops, 1); }

  inline void Predict_MPEG12(const SKL_MV MVo,
                             const SKL_MB_FUNCS * const Ops,
                             const int CoLoc) const;
  inline void Predict_Fields_MPEG12(const SKL_MV MVo[2],
                                    const SKL_MB_FUNCS * const Ops,
                                    const int Src_Fields,
                                    const int CoLoc) const;

  int Is_MPEG1() const { return (VOL->MPEG_Version==1); }

  SKL_MB2(const SKL_MP4_I *vol, SKL_FBB *FBB);

  inline void Decode_Intra_Macroblock();
  inline void Decode_Inter_Macroblock();

  void Init_Scan(int yPos, int xPos) {
    y = yPos;
    if (xPos!=0) {
      y += xPos / VOL->MB_W;
      x = xPos % VOL->MB_W;
    }
    else x = 0;
    Init_Offset();
  }

  int Finished() const { return (y==VOL->MB_H-1 && x==VOL->MB_W-1); }
  int Next() {
    (*this)++;
    if (x>=VOL->MB_W) {
      y++;
      if (y>=VOL->MB_H) return 0;   // oh,oh
      x = 0;
      Init_Scanline(VOL, x);
    }
    return 1;
  }
  inline int Skip_Macroblock(int Skip);

  void Print_Infos() const;
};

//////////////////////////////////////////////////////////
// VLC reading helpers

struct SKL_VLC { SKL_INT16 Value, Len; };
struct SKL_DCT_VLC { SKL_INT16 Run; SKL_BYTE Level, Len; };

struct SKL_VLC_CHUNK { 
  const SKL_VLC *Tab;     // <- table pointer is pre-offset
  SKL_UINT32 Thresh;
  int Shift;
};

struct SKL_DCT_CHUNK {
  const SKL_DCT_VLC *Tab; // <- table pointer is pre-offset
  SKL_UINT32 Thresh; 
  int Shift;
};

inline static const SKL_VLC *Skl_VLC_Search(const SKL_VLC_CHUNK *Chk, SKL_UINT32 Code)
{
  while(Code<Chk->Thresh) Chk++;
  SKL_ASSERT(Chk->Tab!=0);
  return &Chk->Tab[ Code >> Chk->Shift ];
}

inline static const SKL_DCT_VLC *Skl_DCT_VLC_Search(const SKL_DCT_CHUNK *Chk, SKL_UINT32 Code)
{
  while(Code<Chk->Thresh) Chk++;
  SKL_ASSERT(Chk->Tab!=0);
  return &Chk->Tab[ Code >> Chk->Shift ];
}

//////////////////////////////////////////////////////////
//  VLC for Motion Code
// ISO/IEC 13818-2 section 6.2.5
// Tables B-10

static const SKL_VLC MV_Tab_B10_0[14] = {
  { 3, 4},{-3, 4},{ 2, 3},{ 2, 3},{-2, 3},{-2, 3}
, { 1, 2},{ 1, 2},{ 1, 2},{ 1, 2},{-1, 2},{-1, 2},{-1, 2},{-1, 2}
};
static const SKL_VLC MV_Tab_B10_1[10] = {
  { 7, 7},{-7, 7}
, { 6, 7},{-6, 7},{ 5, 7},{-5, 7},{ 4, 6},{ 4, 6},{-4, 6},{-4, 6}
};
static const SKL_VLC MV_Tab_B10_2[24] = {
  {16,10},{-16,10},{15,10},{-15,10},{14,10},{-14,10},{13,10},{-13,10}
, {12,10},{-12,10},{11,10},{-11,10},{10, 9},{10, 9},{-10, 9},{-10, 9}
, { 9, 9},{ 9, 9},{-9, 9},{-9, 9},{ 8, 9},{ 8, 9},{-8, 9},{-8, 9}
};
static const SKL_VLC MV_Tab_B11[4] = {
  {0, 1}, {0,1}, { 1,2}, {-1,2}
};

static void Read_Motion_Vector(SKL_FBB * const Bits, SKL_INT32 Fix, SKL_INT16 &MV)
{
  if (Bits->Get_Bits(1)) return;

  Bits->Check_Bits_Word(9+1);
  SKL_UINT32 Code = Bits->Bits;
  const SKL_VLC *VLC;
  if      (Code>=0x20000000)   VLC = MV_Tab_B10_0 -  2 + (Code>>(22+6));
  else if (Code>=0x0c000000)   VLC = MV_Tab_B10_1 -  6 + (Code>>(22+3));
  else  /*(Code>=0x06000000)*/ VLC = MV_Tab_B10_2 - 24 + (Code>>(22+0));
  Bits->Discard(VLC->Len);

  const int High  = 1 << (Fix+3);

  int Val = VLC->Value;
  if (--Fix) {
    const int Res = Bits->Get_Bits(Fix) + 1;
    if (Val<0) {
      MV += ((Val+1)<<Fix) - Res;
      if (MV<-High) MV += 2*High;
    }
    else {
      MV += ((Val-1)<<Fix) + Res;
      if (MV>=High) MV -= 2*High;
    }
  }
  else {
    if (Val<0) {
      MV += Val;
      if (MV<-High) MV += 2*High;
    }
    else {
      MV += Val;
      if (MV>=High) MV -= 2*High;
    }
  }
}

static inline int Read_DMV(SKL_FBB * const Bits)
{
  const SKL_VLC * const VLC = &MV_Tab_B11[Bits->See_Bits(2)];
  Bits->Discard(VLC->Len);
  return VLC->Value;
}

static inline 
void Read_Vector(SKL_FBB * const Bits, SKL_MV MV,
                 SKL_INT32 xFCode, SKL_INT32 yFCode)
{
  Read_Motion_Vector(Bits, xFCode, MV[0]);
  Read_Motion_Vector(Bits, yFCode, MV[1]);
}

static void Read_Motion_Vectors(SKL_MB2 *MB, int Bwd)
{
  SKL_ASSERT(!MB->Is_MPEG1() && MB->Motion()!=MC_DMV);
  const int FCode = (Bwd ? MB->VOL->Bwd_Code : MB->VOL->Fwd_Code);
  Read_Vector(MB->FBB, MB->PMV[Bwd][0], FCode>>4, FCode&0xf);

  SKL_COPY_MV(MB->PMV[Bwd][1], MB->PMV[Bwd][0]); // update 2nd predictor
}

static inline 
void Read_DMV_Vector(SKL_FBB * const Bits, SKL_MV MV, SKL_MV DMV, SKL_INT32 FCode)
{
  Read_Motion_Vector(Bits, FCode>>4, MV[0]);
  DMV[0] = Read_DMV(Bits);
  Read_Motion_Vector(Bits, FCode&0xf, MV[1]);
  DMV[1] = Read_DMV(Bits);
}


static int Read_Field_Motion_Vectors(SKL_MB2 *MB, int Bwd)
{
  SKL_ASSERT(!MB->Is_MPEG1() && MB->Motion()!=MC_DMV);
  const int FCode = (Bwd ? MB->VOL->Bwd_Code : MB->VOL->Fwd_Code);

  int Dirs = MB->FBB->Get_Bits(1) << 1;
  Read_Vector(MB->FBB, MB->PMV[Bwd][0], FCode>>4, FCode&0xf);

  Dirs |= MB->FBB->Get_Bits(1);
  Read_Vector(MB->FBB, MB->PMV[Bwd][1], FCode>>4, FCode&0xf);

  return Dirs;
}

static void Read_Motion_Vectors_MPEG1(SKL_MB2 *MB, int Bwd)
{
  const int FCode = (Bwd ? MB->VOL->Bwd_Code : MB->VOL->Fwd_Code);
  if (!(FCode&8))
    Read_Vector(MB->FBB, MB->PMV[Bwd][0], FCode&0x7, FCode&0x7);
  else {  // full-pix motion vector
    MB->PMV[Bwd][0][0] >>= 1; MB->PMV[Bwd][0][1] >>= 1;
    Read_Vector(MB->FBB, MB->PMV[Bwd][0], FCode&0x7, FCode&0x7);
    MB->PMV[Bwd][0][0] <<= 1; MB->PMV[Bwd][0][1] <<= 1;
  }
  SKL_COPY_MV(MB->PMV[Bwd][1], MB->PMV[Bwd][0]);  // update 2nd predictor
}

void SKL_MB2::Reset_Motion_Type()
{
    // derive motion_type (section 6.3.17.1)
  SKL_ASSERT(Is_Frame());
  Set_Field_DCT(-1);
  MB_Type &= ~MC_ALL;
  MB_Type |= MC_FRAME;
}

//////////////////////////////////////////////////////////
// MPEG1-2 specific prediction

inline
int SKL_MB2::MV_Outside(const SKL_MV MV) const
{
  return (MV[0]<Limit_Mins[0] || MV[0]>Limit_Maxs[0] ||
          MV[1]<Limit_Mins[1] || MV[1]>Limit_Maxs[1] );
}

inline
int SKL_MB2::Field_MVs_Outside(const SKL_MV MV[2]) const
{
  return (MV[0][0]<Limit_Mins[0] || MV[0][0]>Limit_Maxs[0] ||
          MV[1][0]<Limit_Mins[0] || MV[1][0]>Limit_Maxs[0] ||
          2*MV[0][1]<Limit_Mins[0] || 2*MV[0][1]>Limit_Maxs[1] ||
          2*MV[1][1]<Limit_Mins[0] || 2*MV[1][1]>Limit_Maxs[1] );
}


inline
void SKL_MB2::Predict_MPEG12(const SKL_MV MVo,
                             const SKL_MB_FUNCS * const Ops,
                             const int Fwd) const
{
  if (MV_Outside(MVo)) return;

  int Halves, Off;
  const int CoLoc = (Fwd ? Fwd_CoLoc : Bwd_CoLoc);
  SKL_BYTE * const Dst = Y1;

  Halves = (MVo[0]&1) | ((MVo[1]&1)<<1);
  Off = CoLoc + (MVo[1]>>1)*BpS + (MVo[0]>>1);
  Ops->HP_16x8[Halves](Dst,      Dst+Off,      BpS);
  Ops->HP_16x8[Halves](Dst+BpS8, Dst+BpS8+Off, BpS);

  SKL_MV uv_MV;
  uv_MV[0] = MVo[0] / 2; uv_MV[1] = MVo[1] / 2;

  Halves = (uv_MV[0]&1) | ((uv_MV[1]&1)<<1);
  Off = (uv_MV[1]>>1)*BpS + (uv_MV[0]>>1);
  Off += CoLoc;
  Ops->HP_8x8[Halves](U, U+Off, BpS);
  Ops->HP_8x8[Halves](V, V+Off, BpS);
}

inline
void SKL_MB2::Predict_Fields_MPEG12(const SKL_MV MVo[2],
                                    const SKL_MB_FUNCS * const Ops,
                                    const int Src_Fields,
                                    const int Fwd) const
{
  if (Field_MVs_Outside(MVo)) return;

  int x, y, Halves, Off;
  const int CoLoc = (Fwd ? Fwd_CoLoc : Bwd_CoLoc);
  SKL_BYTE * const Dst = Y1;

    // 1rst field

  Halves = (MVo[0][0]&1) | ((MVo[0][1]&1)<<1);
  Off = CoLoc + (MVo[0][1]&~1)*BpS + (MVo[0][0]>>1);
  if (Src_Fields&2) Off += BpS;
  Ops->HP_16x8[Halves](Dst, Dst+Off, 2*BpS);

  x = MVo[0][0] / 2; y = MVo[0][1] / 2;
  Halves = (x&1) | ((y&1)<<1);
  Off = CoLoc + (y&~1)*BpS + (x>>1);
  if (Src_Fields&2) Off += BpS;
  Ops->HP_8x4[Halves](U, U+Off, 2*BpS);
  Ops->HP_8x4[Halves](V, V+Off, 2*BpS);

    // 2nd field

  Halves = (MVo[1][0]&1) | ((MVo[1][1]&1)<<1);
  Off = CoLoc + (MVo[1][1]&~1)*BpS + (MVo[1][0]>>1);
  if (Src_Fields&1) Off += BpS;
  Ops->HP_16x8[Halves](Dst+BpS, Dst+Off, 2*BpS);

  x = MVo[1][0] / 2; y = MVo[1][1] / 2;
  Halves = (x&1) | ((y&1)<<1);
  Off = CoLoc + (y&~1)*BpS + (x>>1);
  if (Src_Fields&1) Off += BpS;
  Ops->HP_8x4[Halves](U+BpS, U+Off, 2*BpS);
  Ops->HP_8x4[Halves](V+BpS, V+Off, 2*BpS);
}

//////////////////////////////////////////////////////////

void SKL_MB2::Predict_MPEG1()
{
  SKL_ASSERT(Has_Motion());
  const SKL_MB_FUNCS *Ops = VOL->Copy_Ops;

  if (Is_Forward()) {
    Read_Motion_Vectors_MPEG1(this, 0);
    Predict_MPEG12(PMV[0][0], Ops, 1);
    Ops = VOL->Add_Ops;
  }
  if (Is_Backward()) {
    Read_Motion_Vectors_MPEG1(this, 1);
    Predict_MPEG12(PMV[1][0], Ops, 0);
  }
}

void SKL_MB2::Predict_Reuse()
{
  SKL_ASSERT(Has_Motion());
  const SKL_MB_FUNCS *Ops = VOL->Copy_Ops;

  if (Is_Forward()) {
    Predict_MPEG12(PMV[0][0], Ops, 1);
    Ops = VOL->Add_Ops;
  }
  if (Is_Backward()) {
    Predict_MPEG12(PMV[1][0], Ops, 0);
  }
}

void SKL_MB2::Predict_Frame()
{
  SKL_ASSERT(Has_Motion());
  const SKL_MB_FUNCS *Ops =  VOL->Copy_Ops;

  if (Is_Forward()) {
    Read_Motion_Vectors(this, 0);
    Predict_MPEG12(PMV[0][0], Ops, 1);
    Ops = VOL->Add_Ops;
  }
  if (Is_Backward()) {
    Read_Motion_Vectors(this, 1);
    Predict_MPEG12(PMV[1][0], Ops, 0);
  }
}

void SKL_MB2::Predict_Fields()
{
  SKL_ASSERT(Has_Motion());
  const SKL_MB_FUNCS *Ops =  VOL->Copy_Ops;
  if (Is_Forward())  {
    PMV[0][0][1]>>=1; PMV[0][1][1]>>=1;
    int Dir = Read_Field_Motion_Vectors(this, 0);
    Predict_Fields_MPEG12(PMV[0], Ops, Dir, 1);
    PMV[0][0][1]<<=1; PMV[0][1][1]<<=1;
    Ops = VOL->Add_Ops;
  }

  if (Is_Backward()) {
    PMV[1][0][1]>>=1; PMV[1][1][1]>>=1;
    int Dir = Read_Field_Motion_Vectors(this, 1);
    Predict_Fields_MPEG12(PMV[1], Ops, Dir, 0);
    PMV[1][0][1]<<=1; PMV[1][1][1]<<=1;
  }
}

#define RND2(x,P) (((x)*(P)+((x)>0))>>1)

void SKL_MB2::Predict_DMV()   // Dual prime arithmetic (section 7.6.3.6)
{
  SKL_ASSERT(Has_Motion() && Is_Forward() && Motion()==MC_DMV);
  SKL_MV dMV;
  SKL_MV cMV[2];

  PMV[0][0][1] >>= 1;
  Read_DMV_Vector(FBB, PMV[0][0], dMV, VOL->Fwd_Code);    // always forward
  int Dir, Parity;
  if (VOL->Top_Field_First) {
    Parity = 1;
    Dir = 0x02;
  }
  else {
    Parity = 3;
    Dir = 0x01;
  }
  cMV[0][0] = RND2(PMV[0][0][0],   Parity) + dMV[0];
  cMV[0][1] = RND2(PMV[0][0][1],   Parity) + dMV[1] - 1;
  cMV[1][0] = RND2(PMV[0][0][0], 4-Parity) + dMV[0];
  cMV[1][1] = RND2(PMV[0][0][1], 4-Parity) + dMV[1] + 1;

  Predict_Fields_MPEG12(cMV, VOL->Copy_Ops, Dir, 1);

  PMV[0][0][1]<<=1;
  Predict_MPEG12(PMV[0][0], VOL->Add_Ops, 1);
  SKL_COPY_MV(PMV[0][1], PMV[0][0]); // update 2nd predictor
}
#undef RND2

//////////////////////////////////////////////////////////
//  VLC for Address Increment
// ISO/IEC 13818-2 section 6.2.5

  // Table B-1

  // Note: Len is pre-offset by -1, as leading '1' bit
  // is handled separatly

  /* bLen=4 minCode=b0010xxxxxx maxCode=b1100xxxxxx (0x80/0x33f) */
static const SKL_VLC Tab_B1_Mod_0[14] = {
  { 7, 4},{ 6, 4},{ 5, 3},{ 5, 3},{ 4, 3},{ 4, 3}
, { 3, 2},{ 3, 2},{ 3, 2},{ 3, 2},{ 2, 2},{ 2, 2},{ 2, 2},{ 2, 2}
};
  /* bLen=7 minCode=b0000110xxx maxCode=b0001110xxx (0x30/0x77) */
static const SKL_VLC Tab_B1_Mod_1[10] = {
  {15, 7},{14, 7}
, {13, 7},{12, 7},{11, 7},{10, 7},{ 9, 6},{ 9, 6},{ 8, 6},{ 8, 6}
};
  /* bLen=10 minCode=b0000001000 maxCode=b0000101110 (0x8/0x2e) */
static const SKL_VLC Tab_B1_Mod_2[40] = {
  {-1,10},{ 0,-1},{ 0,-1},{ 0,-1},{ 0,-1},{ 0,-1},{ 0,-1},{ 0,-1}
, { 0,-1},{ 0,-1},{ 0,-1},{ 0,-1},{ 0,-1},{ 0,-1},{ 0,-1},{ 0,-1}
, {33,10},{32,10},{31,10},{30,10},{29,10},{28,10},{27,10},{26,10}
, {25,10},{24,10},{23,10},{22,10},{21, 9},{21, 9},{20, 9},{20, 9}
, {19, 9},{19, 9},{18, 9},{18, 9},{17, 9},{17, 9},{16, 9},{16, 9}
};
static SKL_VLC_CHUNK Incr_Tab_Chunks[] = { /* bSize=10  (total size=64)*/
  { Tab_B1_Mod_0 - 0x2, 0x80, 6 },
  { Tab_B1_Mod_1 - 0x6, 0x30, 3 },
  { Tab_B1_Mod_2 - 0x8, 0x8, 0 },
  {0,0}
};

static int Read_Addr_Incr(SKL_FBB *FBB)
{
  if (FBB->Get_Bits(1)) return 1;

  int Mb_Addr = 0;

  SKL_UINT32 Bits = FBB->See_Bits(10);
  while(Bits<24)
  {
    FBB->Discard(10);
    if (Bits==0x08) Mb_Addr += 33; // escape
    else if (Bits==0x0f) {}        // stuffing (MPEG1)
    else return 0;                 // Error. forbidden codes [0-7][9-14][16-23]
    if (FBB->Get_Bits(1))
      return Mb_Addr+1;
    Bits = FBB->See_Bits(10);
  }

  const SKL_VLC *vlc = Skl_VLC_Search( Incr_Tab_Chunks, Bits );
  SKL_ASSERT(vlc!=0);
  FBB->Discard(vlc->Len);
  return (Mb_Addr + vlc->Value);
}

//////////////////////////////////////////////////////////
//  VLC for non Spatial-Scalable macroblock types
// ISO/IEC 13818-2 section 6.2.5
// Tables B-2..B-4

  // Table B-2 (I-VOP) //

#define MBLK_ERROR  {MPEG12_MBLK_ERROR,0}

static const SKL_VLC MBlk_Type_B_2[] = {
  {MPEG12_MBLK_QUANT|MPEG12_MBLK_INTRA,2} , {MPEG12_MBLK_INTRA, 1}
};


  // Table B-3 (P-VOP) //

#define MBLK_B3_1 {MPEG12_MBLK_PATTERN|MPEG12_MBLK_MFWD, 1}
#define MBLK_B3_2 {MPEG12_MBLK_PATTERN, 2}
#define MBLK_B3_3 {MPEG12_MBLK_MFWD, 3}
#define MBLK_B3_4 {MPEG12_MBLK_INTRA, 5}
#define MBLK_B3_5 {MPEG12_MBLK_PATTERN|MPEG12_MBLK_MFWD|MPEG12_MBLK_QUANT, 5}
#define MBLK_B3_6 {MPEG12_MBLK_PATTERN|MPEG12_MBLK_QUANT, 5}
#define MBLK_B3_7 {MPEG12_MBLK_INTRA|MPEG12_MBLK_QUANT, 6}

static const SKL_VLC MBlk_Type_B_3[] = {

  MBLK_B3_7

, MBLK_B3_6, MBLK_B3_5, MBLK_B3_4

, MBLK_B3_3, MBLK_B3_3, MBLK_B3_3, MBLK_B3_3

, MBLK_B3_2, MBLK_B3_2, MBLK_B3_2, MBLK_B3_2
, MBLK_B3_2, MBLK_B3_2, MBLK_B3_2, MBLK_B3_2

, MBLK_B3_1, MBLK_B3_1, MBLK_B3_1, MBLK_B3_1, MBLK_B3_1, MBLK_B3_1, MBLK_B3_1, MBLK_B3_1
, MBLK_B3_1, MBLK_B3_1, MBLK_B3_1, MBLK_B3_1, MBLK_B3_1, MBLK_B3_1, MBLK_B3_1, MBLK_B3_1
};

  // Table B-4 (B-VOP) //

#define MBLK_B4_1  {MPEG12_MBLK_PATTERN|MPEG12_MBLK_MFWD|MPEG12_MBLK_MBWD, 2}
#define MBLK_B4_2  {MPEG12_MBLK_MFWD|MPEG12_MBLK_MBWD, 2}
#define MBLK_B4_3  {MPEG12_MBLK_PATTERN|MPEG12_MBLK_MBWD, 3}
#define MBLK_B4_4  {MPEG12_MBLK_MBWD, 3}
#define MBLK_B4_5  {MPEG12_MBLK_PATTERN|MPEG12_MBLK_MFWD, 4}
#define MBLK_B4_6  {MPEG12_MBLK_MFWD, 4}
#define MBLK_B4_7  {MPEG12_MBLK_INTRA, 5}
#define MBLK_B4_8  {MPEG12_MBLK_PATTERN|MPEG12_MBLK_MFWD|MPEG12_MBLK_MBWD|MPEG12_MBLK_QUANT, 5}
#define MBLK_B4_9  {MPEG12_MBLK_PATTERN|MPEG12_MBLK_MFWD|MPEG12_MBLK_QUANT, 6}
#define MBLK_B4_10 {MPEG12_MBLK_PATTERN|MPEG12_MBLK_MBWD|MPEG12_MBLK_QUANT, 6}
#define MBLK_B4_11 {MPEG12_MBLK_INTRA|MPEG12_MBLK_QUANT, 6}

static const SKL_VLC MBlk_Type_B_4[] = {
  MBLK_ERROR

, MBLK_B4_11
, MBLK_B4_10
, MBLK_B4_9

, MBLK_B4_8, MBLK_B4_8
, MBLK_B4_7, MBLK_B4_7

, MBLK_B4_6, MBLK_B4_6, MBLK_B4_6, MBLK_B4_6
, MBLK_B4_5, MBLK_B4_5, MBLK_B4_5, MBLK_B4_5

, MBLK_B4_4, MBLK_B4_4, MBLK_B4_4, MBLK_B4_4, MBLK_B4_4, MBLK_B4_4, MBLK_B4_4, MBLK_B4_4
, MBLK_B4_3, MBLK_B4_3, MBLK_B4_3, MBLK_B4_3, MBLK_B4_3, MBLK_B4_3, MBLK_B4_3, MBLK_B4_3

, MBLK_B4_2, MBLK_B4_2, MBLK_B4_2, MBLK_B4_2, MBLK_B4_2, MBLK_B4_2, MBLK_B4_2, MBLK_B4_2
, MBLK_B4_2, MBLK_B4_2, MBLK_B4_2, MBLK_B4_2, MBLK_B4_2, MBLK_B4_2, MBLK_B4_2, MBLK_B4_2
, MBLK_B4_1, MBLK_B4_1, MBLK_B4_1, MBLK_B4_1, MBLK_B4_1, MBLK_B4_1, MBLK_B4_1, MBLK_B4_1
, MBLK_B4_1, MBLK_B4_1, MBLK_B4_1, MBLK_B4_1, MBLK_B4_1, MBLK_B4_1, MBLK_B4_1, MBLK_B4_1

};

  // Special for old D-Frames

static const SKL_VLC MBlk_Type_B_D_Frame[] = {
  { MPEG12_MBLK_INTRA, 1 }, { MPEG12_MBLK_INTRA, 1 }
};

//////////////////////////////////////////////////////////

static const SKL_VLC * const MBlk_Type_Tabs[4] = {
  MBlk_Type_B_2,
  MBlk_Type_B_3,
  MBlk_Type_B_4,
  MBlk_Type_B_D_Frame,
};
static const int MBlk_Type_Tabs_Bitsize[4] = { 1, 5, 6, 1 };

static int Read_MBlk_Type(SKL_FBB *FBB, int Coding)
{
  SKL_ASSERT(Coding>=I_VOP && Coding<=D_VOP);
  int Bits = FBB->See_Bits( MBlk_Type_Tabs_Bitsize[Coding] );
  const SKL_VLC *vlc = MBlk_Type_Tabs[Coding];
  FBB->Discard( vlc[Bits].Len );
  return vlc[Bits].Value;
}

//////////////////////////////////////////////////////////
//  Coded block pattern
// ISO/IEC 13818-2 section 6.2.5
// Table B-9

static const SKL_VLC Tab_B9_1[24] = {
  {62, 5},{ 2, 5},{61, 5},{ 1, 5},{56, 5},{52, 5},{44, 5},{28, 5}
, {40, 5},{20, 5},{48, 5},{12, 5},{32, 4},{32, 4},{16, 4},{16, 4}
, { 8, 4},{ 8, 4},{ 4, 4},{ 4, 4},{60, 3},{60, 3},{60, 3},{60, 3}
};
static const SKL_VLC Tab_B9_2[16] = {
  {34, 7},{18, 7},{10, 7},{ 6, 7},{33, 7},{17, 7},{ 9, 7},{ 5, 7}
, {63, 6},{63, 6},{ 3, 6},{ 3, 6},{36, 6},{36, 6},{24, 6},{24, 6}
};
static const SKL_VLC Tab_B9_3[63] = {
  { 0, 9},{39, 9},{27, 9},{59, 9},{55, 9},{47, 9},{31, 9}
, {58, 8},{58, 8},{54, 8},{54, 8},{46, 8},{46, 8},{30, 8},{30, 8}
, {57, 8},{57, 8},{53, 8},{53, 8},{45, 8},{45, 8},{29, 8},{29, 8}
, {38, 8},{38, 8},{26, 8},{26, 8},{37, 8},{37, 8},{25, 8},{25, 8}
, {43, 8},{43, 8},{23, 8},{23, 8},{51, 8},{51, 8},{15, 8},{15, 8}
, {42, 8},{42, 8},{22, 8},{22, 8},{50, 8},{50, 8},{14, 8},{14, 8}
, {41, 8},{41, 8},{21, 8},{21, 8},{49, 8},{49, 8},{13, 8},{13, 8}
, {35, 8},{35, 8},{19, 8},{19, 8},{11, 8},{11, 8},{ 7, 8},{ 7, 8}
};
static const SKL_VLC_CHUNK CBP_Tab_Chunks[] = { /* bSize=9  (total size=103)*/
  { Tab_B9_1 - 8, 128, 4 }, /* bLen=5 min=0x80 Max=0x1cf */
  { Tab_B9_2 - 16, 64, 2 }, /* bLen=7 min=0x40 Max=0x7b */
  { Tab_B9_3 - 1, 1, 0 }, /* bLen=9 min=0x1 Max=0x3e */
  {0,0}
};

static int Read_Coded_Block_Pattern(SKL_FBB *FBB)
{
  int Bits = FBB->See_Bits( 9 );
  const SKL_VLC *vlc = Skl_VLC_Search( CBP_Tab_Chunks, Bits );
  FBB->Discard(vlc->Len);
  return vlc->Value;
}

//////////////////////////////////////////////////////////
// DC-Size tables (B-12 / B-13)
//////////////////////////////////////////////////////////

  // Lum-DC-Size Table B-12 (genhufftab 2 3 2)
  // Note: This table is coded for 10bits input code (instead of 9bits in the norm). 
  // With this, we have the same loop for decoding both Lum or Chroma.
  // => we shift by 1 before final look-up.

  /* bLen=5 minCode=b00000xxxx maxCode=b11110xxxx (0x0/0x1ef) */
static const SKL_VLC Tab_B12_1[31] = {
  { 1, 2},{ 1, 2},{ 1, 2},{ 1, 2},{ 1, 2},{ 1, 2},{ 1, 2},{ 1, 2}
, { 2, 2},{ 2, 2},{ 2, 2},{ 2, 2},{ 2, 2},{ 2, 2},{ 2, 2},{ 2, 2}
, { 0, 3},{ 0, 3},{ 0, 3},{ 0, 3},{ 3, 3},{ 3, 3},{ 3, 3},{ 3, 3}
, { 4, 3},{ 4, 3},{ 4, 3},{ 4, 3},{ 5, 4},{ 5, 4},{ 6, 5}
};
  /* bLen=9 minCode=b111110000 maxCode=b111111111 (0x1f0/0x1ff) */
static const SKL_VLC Tab_B12_3[16] = {
  { 7, 6},{ 7, 6},{ 7, 6},{ 7, 6},{ 7, 6},{ 7, 6},{ 7, 6},{ 7, 6}
, { 8, 7},{ 8, 7},{ 8, 7},{ 8, 7},{ 9, 8},{ 9, 8},{10, 9},{11, 9}
};

static const SKL_VLC_CHUNK DC_Size_Chunks_Lum[] = { /* bSize=9  (total size=47)*/
  { Tab_B12_3 - 0x1f0, (0x1f0<<1), 0+1 },
  { Tab_B12_1 - 0x0, 0x0, 4+1 },
  {0,0}
};


  // Chroma-DC-Size table B-13 (genhufftab 2 3 2)

  /* bLen=5 minCode=b00000xxxxx maxCode=b11110xxxxx (0x0/0x3df) */
static const SKL_VLC Tab_B13_1[31] = {
  { 0, 2},{ 0, 2},{ 0, 2},{ 0, 2},{ 0, 2},{ 0, 2},{ 0, 2},{ 0, 2}
, { 1, 2},{ 1, 2},{ 1, 2},{ 1, 2},{ 1, 2},{ 1, 2},{ 1, 2},{ 1, 2}
, { 2, 2},{ 2, 2},{ 2, 2},{ 2, 2},{ 2, 2},{ 2, 2},{ 2, 2},{ 2, 2}
, { 3, 3},{ 3, 3},{ 3, 3},{ 3, 3},{ 4, 4},{ 4, 4},{ 5, 5}
};
  /* bLen=10 minCode=b1111100000 maxCode=b1111111111 (0x3e0/0x3ff) */
static const SKL_VLC Tab_B13_3[32] = {
  { 6, 6},{ 6, 6},{ 6, 6},{ 6, 6},{ 6, 6},{ 6, 6},{ 6, 6},{ 6, 6}
, { 6, 6},{ 6, 6},{ 6, 6},{ 6, 6},{ 6, 6},{ 6, 6},{ 6, 6},{ 6, 6}
, { 7, 7},{ 7, 7},{ 7, 7},{ 7, 7},{ 7, 7},{ 7, 7},{ 7, 7},{ 7, 7}
, { 8, 8},{ 8, 8},{ 8, 8},{ 8, 8},{ 9, 9},{ 9, 9},{10,10},{11,10}
};
static const SKL_VLC_CHUNK DC_Size_Chunks_Chroma[] = { /* bSize=10  (total size=63)*/
  { Tab_B13_3 - 0x3e0, 0x3e0, 0 },
  { Tab_B13_1 - 0x0, 0x0, 5 },
  {0,0}
};

//////////////////////////////////////////////////////////
// DCT B-14/B-15 flavored tables
//////////////////////////////////////////////////////////

// B14-DC: genhufftab -s -v4 -n 26 5 8 10 13 15 16
// B14-AC: genhufftab -s -v4 -n 27 5 8 10 13 15 16
// B15-AC: genhufftab -s -v4 -n 28 5 8 10 13 15 16
// (Official table: genhufftab 4 2 2 2 1 1 1 1 1)


  // Table B-14 for AC Coeffs

#define ESC {-1,0,6}
#define EOB {64,0,2}

  /* bLen=5 minCode=b00101xxxxxxxxxxx maxCode=b11000xxxxxxxxxxx (0x2800/0xc7ff) */
static const SKL_DCT_VLC Tab_B14_AC_0[27] = {
  { 0, 3, 5},{ 4, 1, 5},{ 3, 1, 5}
, { 0, 2, 4},{ 0, 2, 4},{ 2, 1, 4},{ 2, 1, 4},{ 1, 1, 3},{ 1, 1, 3},{ 1, 1, 3},{ 1, 1, 3}
,        EOB,       EOB,       EOB,       EOB,       EOB,       EOB,       EOB,       EOB
, { 0, 1, 2},{ 0, 1, 2},{ 0, 1, 2},{ 0, 1, 2},{ 0, 1, 2},{ 0, 1, 2},{ 0, 1, 2},{ 0, 1, 2}
};
  /* bLen=8 minCode=b00000100xxxxxxxx maxCode=b00100111xxxxxxxx (0x400/0x27ff) */
static const SKL_DCT_VLC Tab_B14_AC_1[36] = {
         ESC,       ESC,       ESC,       ESC
, { 2, 2, 7},{ 2, 2, 7},{ 9, 1, 7},{ 9, 1, 7},{ 0, 4, 7},{ 0, 4, 7},{ 8, 1, 7},{ 8, 1, 7}
, { 7, 1, 6},{ 7, 1, 6},{ 7, 1, 6},{ 7, 1, 6},{ 6, 1, 6},{ 6, 1, 6},{ 6, 1, 6},{ 6, 1, 6}
, { 1, 2, 6},{ 1, 2, 6},{ 1, 2, 6},{ 1, 2, 6},{ 5, 1, 6},{ 5, 1, 6},{ 5, 1, 6},{ 5, 1, 6}
, {13, 1, 8},{ 0, 6, 8},{12, 1, 8},{11, 1, 8},{ 3, 2, 8},{ 1, 3, 8},{ 0, 5, 8},{10, 1, 8}
};
  /* bLen=10 minCode=b0000001000xxxxxx maxCode=b0000001111xxxxxx (0x200/0x3ff) */
static const SKL_DCT_VLC Tab_B14_AC_2[8] = {
  {16, 1,10},{ 5, 2,10},{ 0, 7,10},{ 2, 3,10},{ 1, 4,10},{15, 1,10},{14, 1,10},{ 4, 2,10}
};
  /* bLen=13 minCode=b0000000010000xxx maxCode=b0000000111110xxx (0x80/0x1f7) */
static const SKL_DCT_VLC Tab_B14_AC_3[48] = {
  {10, 2,13},{ 9, 2,13},{ 5, 3,13},{ 3, 4,13},{ 2, 5,13},{ 1, 7,13},{ 1, 6,13},{ 0,15,13}
, { 0,14,13},{ 0,13,13},{ 0,12,13},{26, 1,13},{25, 1,13},{24, 1,13},{23, 1,13},{22, 1,13}
, { 0,11,12},{ 0,11,12},{ 8, 2,12},{ 8, 2,12},{ 4, 3,12},{ 4, 3,12},{ 0,10,12},{ 0,10,12}
, { 2, 4,12},{ 2, 4,12},{ 7, 2,12},{ 7, 2,12},{21, 1,12},{21, 1,12},{20, 1,12},{20, 1,12}
, { 0, 9,12},{ 0, 9,12},{19, 1,12},{19, 1,12},{18, 1,12},{18, 1,12},{ 1, 5,12},{ 1, 5,12}
, { 3, 3,12},{ 3, 3,12},{ 0, 8,12},{ 0, 8,12},{ 6, 2,12},{ 6, 2,12},{17, 1,12},{17, 1,12}
};
  /* bLen=15 minCode=b000000000010000x maxCode=b000000000111110x (0x20/0x7d) */
static const SKL_DCT_VLC Tab_B14_AC_4[48] = {
  { 0,40,15},{ 0,39,15},{ 0,38,15},{ 0,37,15},{ 0,36,15},{ 0,35,15},{ 0,34,15},{ 0,33,15}
, { 0,32,15},{ 1,14,15},{ 1,13,15},{ 1,12,15},{ 1,11,15},{ 1,10,15},{ 1, 9,15},{ 1, 8,15}
, { 0,31,14},{ 0,31,14},{ 0,30,14},{ 0,30,14},{ 0,29,14},{ 0,29,14},{ 0,28,14},{ 0,28,14}
, { 0,27,14},{ 0,27,14},{ 0,26,14},{ 0,26,14},{ 0,25,14},{ 0,25,14},{ 0,24,14},{ 0,24,14}
, { 0,23,14},{ 0,23,14},{ 0,22,14},{ 0,22,14},{ 0,21,14},{ 0,21,14},{ 0,20,14},{ 0,20,14}
, { 0,19,14},{ 0,19,14},{ 0,18,14},{ 0,18,14},{ 0,17,14},{ 0,17,14},{ 0,16,14},{ 0,16,14}
};
  /* bLen=16 minCode=b0000000000010000 maxCode=b0000000000011111 (0x10/0x1f) */
static const SKL_DCT_VLC Tab_B14_AC_5[16] = {
  { 1,18,16},{ 1,17,16},{ 1,16,16},{ 1,15,16},{ 6, 3,16},{16, 2,16},{15, 2,16},{14, 2,16}
, {13, 2,16},{12, 2,16},{11, 2,16},{31, 1,16},{30, 1,16},{29, 1,16},{28, 1,16},{27, 1,16}
};



  // Modified B-14 for DC Coeff in Inter only

  /* bLen=5 minCode=b00101xxxxxxxxxxx maxCode=b10000xxxxxxxxxxx (0x2800/0x87ff) */
static const SKL_DCT_VLC Tab_B14_DC_0[27] = {
  { 0, 3, 5},{ 4, 1, 5},{ 3, 1, 5}
, { 0, 2, 4},{ 0, 2, 4},{ 2, 1, 4},{ 2, 1, 4},{ 1, 1, 3},{ 1, 1, 3},{ 1, 1, 3},{ 1, 1, 3}
, { 0, 1, 1},{ 0, 1, 1},{ 0, 1, 1},{ 0, 1, 1},{ 0, 1, 1},{ 0, 1, 1},{ 0, 1, 1},{ 0, 1, 1}
, { 0, 1, 1},{ 0, 1, 1},{ 0, 1, 1},{ 0, 1, 1},{ 0, 1, 1},{ 0, 1, 1},{ 0, 1, 1},{ 0, 1, 1}
};


  // Table B-15 for Intra_VLC_Format=1

#define EOB2 {64,0,4}   // warning! EOB code is longer (4bits)
#define NOP {0,0,64}

  // Note: The real official tables for B-15 are exactly the same
  // than B-14 (see Tab_B14_AC_3->Tab_B14_AC_5), notwithstanding 
  // some minor unreachable NOP codes. We'll then share some B-14
  // slices in B-15 search table...


  /* bLen=8 minCode=b00000100xxxxxxxx maxCode=b11111111xxxxxxxx (0x400/0xffff) */
static const SKL_DCT_VLC Tab_B15_AC_1[252] = {
         ESC,       ESC,       ESC,       ESC
, { 7, 1, 7},{ 7, 1, 7},{ 8, 1, 7},{ 8, 1, 7},{ 6, 1, 7},{ 6, 1, 7},{ 2, 2, 7},{ 2, 2, 7}
, { 0, 7, 6},{ 0, 7, 6},{ 0, 7, 6},{ 0, 7, 6},{ 0, 6, 6},{ 0, 6, 6},{ 0, 6, 6},{ 0, 6, 6}
, { 4, 1, 6},{ 4, 1, 6},{ 4, 1, 6},{ 4, 1, 6},{ 5, 1, 6},{ 5, 1, 6},{ 5, 1, 6},{ 5, 1, 6}
, { 1, 5, 8},{11, 1, 8},{ 0,11, 8},{ 0,10, 8},{13, 1, 8},{12, 1, 8},{ 3, 2, 8},{ 1, 4, 8}
, { 2, 1, 5},{ 2, 1, 5},{ 2, 1, 5},{ 2, 1, 5},{ 2, 1, 5},{ 2, 1, 5},{ 2, 1, 5},{ 2, 1, 5}
, { 1, 2, 5},{ 1, 2, 5},{ 1, 2, 5},{ 1, 2, 5},{ 1, 2, 5},{ 1, 2, 5},{ 1, 2, 5},{ 1, 2, 5}
, { 3, 1, 5},{ 3, 1, 5},{ 3, 1, 5},{ 3, 1, 5},{ 3, 1, 5},{ 3, 1, 5},{ 3, 1, 5},{ 3, 1, 5}
, { 1, 1, 3},{ 1, 1, 3},{ 1, 1, 3},{ 1, 1, 3},{ 1, 1, 3},{ 1, 1, 3},{ 1, 1, 3},{ 1, 1, 3}
, { 1, 1, 3},{ 1, 1, 3},{ 1, 1, 3},{ 1, 1, 3},{ 1, 1, 3},{ 1, 1, 3},{ 1, 1, 3},{ 1, 1, 3}
, { 1, 1, 3},{ 1, 1, 3},{ 1, 1, 3},{ 1, 1, 3},{ 1, 1, 3},{ 1, 1, 3},{ 1, 1, 3},{ 1, 1, 3}
, { 1, 1, 3},{ 1, 1, 3},{ 1, 1, 3},{ 1, 1, 3},{ 1, 1, 3},{ 1, 1, 3},{ 1, 1, 3},{ 1, 1, 3}
,       EOB2,      EOB2,      EOB2,      EOB2,      EOB2,      EOB2,      EOB2,      EOB2
,       EOB2,      EOB2,      EOB2,      EOB2,      EOB2,      EOB2,      EOB2,      EOB2
, { 0, 3, 4},{ 0, 3, 4},{ 0, 3, 4},{ 0, 3, 4},{ 0, 3, 4},{ 0, 3, 4},{ 0, 3, 4},{ 0, 3, 4}
, { 0, 3, 4},{ 0, 3, 4},{ 0, 3, 4},{ 0, 3, 4},{ 0, 3, 4},{ 0, 3, 4},{ 0, 3, 4},{ 0, 3, 4}
, { 0, 1, 2},{ 0, 1, 2},{ 0, 1, 2},{ 0, 1, 2},{ 0, 1, 2},{ 0, 1, 2},{ 0, 1, 2},{ 0, 1, 2}
, { 0, 1, 2},{ 0, 1, 2},{ 0, 1, 2},{ 0, 1, 2},{ 0, 1, 2},{ 0, 1, 2},{ 0, 1, 2},{ 0, 1, 2}
, { 0, 1, 2},{ 0, 1, 2},{ 0, 1, 2},{ 0, 1, 2},{ 0, 1, 2},{ 0, 1, 2},{ 0, 1, 2},{ 0, 1, 2}
, { 0, 1, 2},{ 0, 1, 2},{ 0, 1, 2},{ 0, 1, 2},{ 0, 1, 2},{ 0, 1, 2},{ 0, 1, 2},{ 0, 1, 2}
, { 0, 1, 2},{ 0, 1, 2},{ 0, 1, 2},{ 0, 1, 2},{ 0, 1, 2},{ 0, 1, 2},{ 0, 1, 2},{ 0, 1, 2}
, { 0, 1, 2},{ 0, 1, 2},{ 0, 1, 2},{ 0, 1, 2},{ 0, 1, 2},{ 0, 1, 2},{ 0, 1, 2},{ 0, 1, 2}
, { 0, 1, 2},{ 0, 1, 2},{ 0, 1, 2},{ 0, 1, 2},{ 0, 1, 2},{ 0, 1, 2},{ 0, 1, 2},{ 0, 1, 2}
, { 0, 1, 2},{ 0, 1, 2},{ 0, 1, 2},{ 0, 1, 2},{ 0, 1, 2},{ 0, 1, 2},{ 0, 1, 2},{ 0, 1, 2}
, { 0, 2, 3},{ 0, 2, 3},{ 0, 2, 3},{ 0, 2, 3},{ 0, 2, 3},{ 0, 2, 3},{ 0, 2, 3},{ 0, 2, 3}
, { 0, 2, 3},{ 0, 2, 3},{ 0, 2, 3},{ 0, 2, 3},{ 0, 2, 3},{ 0, 2, 3},{ 0, 2, 3},{ 0, 2, 3}
, { 0, 2, 3},{ 0, 2, 3},{ 0, 2, 3},{ 0, 2, 3},{ 0, 2, 3},{ 0, 2, 3},{ 0, 2, 3},{ 0, 2, 3}
, { 0, 2, 3},{ 0, 2, 3},{ 0, 2, 3},{ 0, 2, 3},{ 0, 2, 3},{ 0, 2, 3},{ 0, 2, 3},{ 0, 2, 3}
, { 0, 4, 5},{ 0, 4, 5},{ 0, 4, 5},{ 0, 4, 5},{ 0, 4, 5},{ 0, 4, 5},{ 0, 4, 5},{ 0, 4, 5}
, { 0, 5, 5},{ 0, 5, 5},{ 0, 5, 5},{ 0, 5, 5},{ 0, 5, 5},{ 0, 5, 5},{ 0, 5, 5},{ 0, 5, 5}
, { 9, 1, 7},{ 9, 1, 7},{ 1, 3, 7},{ 1, 3, 7},{10, 1, 7},{10, 1, 7},{ 0, 8, 7},{ 0, 8, 7}
, { 0, 9, 7},{ 0, 9, 7},{ 0,12, 8},{ 0,13, 8},{ 2, 3, 8},{ 4, 2, 8},{ 0,14, 8},{ 0,15, 8}
};
  /* bLen=10 minCode=b0000001000xxxxxx maxCode=b0000001110xxxxxx (0x200/0x3bf) */
static const SKL_DCT_VLC Tab_B15_AC_2[8] = {
  { 5, 2, 9},{ 5, 2, 9},{14, 1, 9},{14, 1, 9},{ 2, 4,10},{16, 1,10},{15, 1, 9},{15, 1, 9}
};
  /* bLen=13 minCode=b0000000010000xxx maxCode=b0000000111110xxx (0x80/0x1f7) */
static const SKL_DCT_VLC Tab_B15_AC_3[48] = {
  {10, 2,13},{ 9, 2,13},{ 5, 3,13},{ 3, 4,13},{ 2, 5,13},{ 1, 7,13},{ 1, 6,13},      NOP 
,       NOP ,      NOP ,      NOP ,{26, 1,13},{25, 1,13},{24, 1,13},{23, 1,13},{22, 1,13}
,       NOP ,      NOP ,{ 8, 2,12},{ 8, 2,12},{ 4, 3,12},{ 4, 3,12},      NOP ,      NOP 
,       NOP ,      NOP ,{ 7, 2,12},{ 7, 2,12},{21, 1,12},{21, 1,12},{20, 1,12},{20, 1,12}
,       NOP ,      NOP ,{19, 1,12},{19, 1,12},{18, 1,12},{18, 1,12},      NOP ,      NOP 
, { 3, 3,12},{ 3, 3,12},      NOP ,      NOP ,{ 6, 2,12},{ 6, 2,12},{17, 1,12},{17, 1,12}
};
  /* bLen=15 minCode=b000000000010000x maxCode=b000000000111110x (0x20/0x7d) */
static const SKL_DCT_VLC Tab_B15_AC_4[48] = {
  { 0,40,15},{ 0,39,15},{ 0,38,15},{ 0,37,15},{ 0,36,15},{ 0,35,15},{ 0,34,15},{ 0,33,15}
, { 0,32,15},{ 1,14,15},{ 1,13,15},{ 1,12,15},{ 1,11,15},{ 1,10,15},{ 1, 9,15},{ 1, 8,15}
, { 0,31,14},{ 0,31,14},{ 0,30,14},{ 0,30,14},{ 0,29,14},{ 0,29,14},{ 0,28,14},{ 0,28,14}
, { 0,27,14},{ 0,27,14},{ 0,26,14},{ 0,26,14},{ 0,25,14},{ 0,25,14},{ 0,24,14},{ 0,24,14}
, { 0,23,14},{ 0,23,14},{ 0,22,14},{ 0,22,14},{ 0,21,14},{ 0,21,14},{ 0,20,14},{ 0,20,14}
, { 0,19,14},{ 0,19,14},{ 0,18,14},{ 0,18,14},{ 0,17,14},{ 0,17,14},{ 0,16,14},{ 0,16,14}
};
  /* bLen=16 minCode=b0000000000010000 maxCode=b0000000000011111 (0x10/0x1f) */
static const SKL_DCT_VLC Tab_B15_AC_5[16] = {
  { 1,18,16},{ 1,17,16},{ 1,16,16},{ 1,15,16},{ 6, 3,16},{16, 2,16},{15, 2,16},{14, 2,16}
, {13, 2,16},{12, 2,16},{11, 2,16},{31, 1,16},{30, 1,16},{29, 1,16},{28, 1,16},{27, 1,16}
};

// -- Final tables

  // AC table for Intra_VLC_Format=0
static const SKL_DCT_CHUNK B14_AC_Chunks[] = {/* bSize=16  (total size=183)*/
  { Tab_B14_AC_0 - 0x5, 0x28000000, 27 }, /* nbBits:5 */  // <- contains EOB
  { Tab_B14_AC_1 - 0x4, 0x04000000, 24 }, /* nbBits:8 */  // <- contains ESC
  { Tab_B14_AC_2 - 0x8, 0x02000000, 22 }, /* nbBits:10 */
  { Tab_B14_AC_3 - 0x10, 0x00800000, 19 }, /* nbBits:13 */
  { Tab_B14_AC_4 - 0x10, 0x00200000, 17 }, /* nbBits:15 */
  { Tab_B14_AC_5 - 0x10, 0x00100000, 16 }, /* nbBits:16 */
  {0,0}
};

  // Modified B14 for Coeff[0][0] in Inter only
static const SKL_DCT_CHUNK B14_DC_Chunks[] = { /* bSize=16  (total size=183)*/
  { Tab_B14_DC_0 - 0x5, 0x28000000, 27 }, /* nbBits:5 */  // <- alternative codes. no EOB
  { Tab_B14_AC_1 - 0x4, 0x04000000, 24 }, /* nbBits:8 */  // <- contains ESC
  { Tab_B14_AC_2 - 0x8, 0x02000000, 22 }, /* nbBits:10 */
  { Tab_B14_AC_3 - 0x10, 0x00800000, 19 }, /* nbBits:13 */
  { Tab_B14_AC_4 - 0x10, 0x00200000, 17 }, /* nbBits:15 */
  { Tab_B14_AC_5 - 0x10, 0x00100000, 16 }, /* nbBits:16 */
  {0,0}
};

  // AC table for Intra_VLC_Format=1 (MPEG-2)
static const SKL_DCT_CHUNK B15_AC_Chunks[] = { /* bSize=16  (total size=380)*/
  { Tab_B15_AC_1 - 0x4, 0x04000000, 24 }, /* nbBits:8 */
  { Tab_B15_AC_2 - 0x8, 0x02000000, 22 }, /* nbBits:10 */
  { Tab_B15_AC_3 - 0x10, 0x00800000, 19 }, /* nbBits:13 */
  { Tab_B15_AC_4 - 0x10, 0x00200000, 17 }, /* nbBits:15 */
  { Tab_B15_AC_5 - 0x10, 0x00100000, 16 }, /* nbBits:16 */
  {0,0}
};

//////////////////////////////////////////////////////////
//  Lum-DC diff
// Chunks = DC_Size_Chunks_Lum for luma decoding
//        = DC_Size_Chunks_Chroma for chroma decoding
//

static const SKL_VLC_CHUNK *DCT_Chunks[3] = {
  DC_Size_Chunks_Lum,
  DC_Size_Chunks_Chroma,
  DC_Size_Chunks_Chroma
};

static inline int Read_DC_Diff_MPEG2(SKL_FBB *FBB, const SKL_VLC_CHUNK *Chk)
{
  int Bits = FBB->See_Bits( 10 );
  const SKL_VLC *vlc = Skl_VLC_Search( Chk, Bits );
  FBB->Discard(vlc->Len);
  int Size = vlc->Value;
  if (!Size) return 0;
  SKL_INT32 Diff = FBB->Get_Bits( Size );
  if ( !(Diff & SKL_BMASKS::Or[Size]) )
    Diff -= SKL_BMASKS::And[Size];
  return Diff;
}

//////////////////////////////////////////////////////////
// -- Escape values

inline static int Read_MPEG1_Escape(SKL_FBB *FBB) {
  int Val = FBB->Get_Bits(8);
  if (Val==0) Val = FBB->Get_Bits(8);
  else if (Val==128) Val = FBB->Get_Bits(8) - 256;
  else if (Val>128) Val-=256;
  return Val;
}

inline static int Read_MPEG2_Escape(SKL_FBB *FBB) {
  int Val = FBB->Get_Bits(12);
  if ((Val&2047)==0) return 0;  // error
  if (Val>=2048) Val -= 4096;
  return Val;
}

//////////////////////////////////////////////////////////
// -- MPEG-1 intra block

static 
void Read_Intra_Block_MPEG1(SKL_FBB *FBB, SKL_MB2 *MB, SKL_INT16 *Out)
{
  const int * const Scan = SKL_MP4_I::Scan_Order[0]+64;

    // Note: sometimes, EOB occurs *exactly* at the end, for i==0.
    // We need to parse it however. That's why we're not using
    // a loop like "while(i<0)", but "while(1)" instead.

    // Note2: We unroll the Huffman table searching, because:
    //  * EOB is only present on the 1rst chunk
    //  * ESC is only present on the second
    //  * Only the last chunk requires full 16bits
    // We re-ordered the search in a (small) logarithmic search

    // Note3: For code-cache coherency and branch prediction, we put
    // the 'Ok:' code *first*

  SKL_FBB_DECL;
  SKL_FBB_START(*FBB);
  SKL_FBB_LOAD_WORD(16);
  const SKL_DCT_VLC *Tab = 0;

  int i = -63;
  while(1)
  {
    if (Bits>=0x28000000) {
      Tab = Tab_B14_AC_0 -  5 + (Bits>>(16+11));
Ok:
      if ((i += Tab->Run)>=0) break;  // EOB
      SKL_INT32 Val = Tab->Level;
      Bits_Left -= Tab->Len+1;
      Bits <<= Tab->Len;
      if (Bits&0x80000000) Val = -Val;
      Bits <<= 1;
      SKL_FBB_LOAD_WORD(16);

      Out[Scan[i++]] = Val;
      continue;
    }
    else if (Bits>=0x08000000) {
      Tab = Tab_B14_AC_1 -  4 + (Bits>>(16+8));
      goto Ok;
    }
    else if (Bits>=0x00800000) {
      if (Bits>=0x04000000) // ESC code only
      {
          // parse away 6bits for ESC code and 6bits for 'Skip' value
        SKL_UINT32 Skip = (Bits >> (32-12)) & 0x3f;
        if ((i += Skip)>=0) break;  // error
        SKL_FBB_DISCARD_SAFE( 6+6 );

//        inlined version of 'Read_MPEG1_Escape()'
        SKL_FBB_LOAD_WORD(16);
        SKL_INT32 Val = SKL_FBB_BITS_SIGNED(8);
        SKL_FBB_DISCARD_SAFE(8);
        if ((Val&0x7f)==0)  // Val=0 or 128
        {
          Val = SKL_FBB_BITS(8) + 2*Val;
          SKL_FBB_DISCARD_SAFE(8);
        }

        Out[Scan[i++]] = Val;
        SKL_FBB_LOAD_WORD(16);
        continue;
      }
      else {
        if (Bits>=0x02000000)
          Tab = Tab_B14_AC_2 -  8 + (Bits>>(16+6));
        else /* (Bits>=0x00800000) */
          Tab = Tab_B14_AC_3 - 16 + (Bits>>(16+3));
        goto Ok;
      }
    }
    else {
      if (Bits>=0x00200000) {
        Tab = Tab_B14_AC_4 - 16 + (Bits>>(16+1));
        goto Ok;
      }
      else if (Bits>=0x00100000) {
        Tab = Tab_B14_AC_5 - 16 + (Bits>>(16+0));
        SKL_FBB_LOAD_WORD(17);
        goto Ok;
      }
      /* else -> error */
    }
    break;  /* error */
  }
  SKL_FBB_DISCARD_SAFE(Tab->Len);
  SKL_FBB_STOP(*FBB);
}

static
void Read_Intra_Block_DType_MPEG1(SKL_FBB *FBB, SKL_MB2 *MB, SKL_INT16 *Out)
{ /* nothing to do */ }

//////////////////////////////////////////////////////////
// MPEG-1 Inter block

static
int Read_Inter_Block_MPEG1(SKL_FBB *FBB, SKL_MB2 *MB, SKL_INT16 *Out)
{
  const int * const Scan = SKL_MP4_I::Scan_Order[0]+64;

  SKL_FBB Local;
  Local.Copy(FBB);

  const SKL_DCT_VLC *Tab;
  int i = -64;
  int Rows = 0x00;

    // we decode the first coeff DC the usual way. No optim worth.
  Local.Check_Bits(16 + 1);
  Tab = Skl_DCT_VLC_Search( B14_DC_Chunks, Local.Bits );
  if (Tab->Run<0) goto ESC_Code;
  else goto Ok;

  while(1)
  {
    if (Local.Bits>=0x28000000)  {
      Tab = Tab_B14_AC_0 - 5 + (Local.Bits>>(16+11));
Ok:
      if ((i += Tab->Run)>=0) break; // EOB
      int Val = Tab->Level;
      Local.Bits_Left -= Tab->Len+1;
      Local.Bits <<= Tab->Len;
      if (Local.Bits&0x80000000) Val = -Val;
      Local.Bits <<= 1;
      Local.Check_Bits_Word(16);
      const int j = Scan[i++];
      Out[j] = Val;
      Rows |= SKL_MP4_I::Row_From_Index[j];
      continue;
    }
    else if (Local.Bits>=0x08000000) {
      Tab = Tab_B14_AC_1 - 4 + (Local.Bits>>(16+8));
      goto Ok;
    }
    else if (Local.Bits>=0x00800000) {
      if (Local.Bits>=0x04000000) // ESC code only
      {
ESC_Code:
          // parse away 6bits for ESC code and 6bits for 'Skip' value
        SKL_UINT32 Skip = (Local.Bits >> (32-12)) & 0x3f;
        if ((i += Skip)>=0) break;  // error
        Local.Discard_Safe( 6+6 );
        int Val = Read_MPEG1_Escape(&Local);
    
        Out[Scan[i++]] = Val;
        Local.Check_Bits_Word(16);
        continue;
      }
      else {
        if (Local.Bits>=0x02000000)
          Tab = Tab_B14_AC_2 - 8 + (Local.Bits>>(16+6));
        else /*(Local.Bits>=0x00800000)*/
          Tab = Tab_B14_AC_3 - 16 + (Local.Bits>>(16+3));
        goto Ok;
      }
    }
    else {
      if (Local.Bits>=0x00200000) {
        Tab = Tab_B14_AC_4 - 16 + (Local.Bits>>(16+1));
        goto Ok;
      }
      else if (Local.Bits>=0x00100000) {
        Tab = Tab_B14_AC_5 - 16 + (Local.Bits>>(16+0));
        Local.Check_Bits(17);   // loads 1 more byte
        goto Ok;
      }
      /* else -> error */
    }
    break; /* error */
  }
// Error:
  Local.Discard_Safe(Tab->Len);
  FBB->Copy(&Local);

  return Rows;
}

//////////////////////////////////////////////////////////
// -- MPEG-2 intra block

static
void Read_Intra_Block_B14_MPEG2(SKL_FBB *FBB, SKL_MB2 *MB, SKL_INT16 *Out)
{
  const int * const Scan = MB->Scan_Order;

  SKL_FBB Local;
  Local.Copy(FBB);

  SKL_INT32 Mismatch = Out[0];

  int i = -63;
  while(1)
  {
    int Val;
    Local.Check_Bits(16);
    const SKL_DCT_VLC *Tab = Skl_DCT_VLC_Search( B14_AC_Chunks, Local.Bits );
    if (Tab->Run<0) // ESC
    {
        // parse away 6bits for ESC code and 6bits for 'Skip' value
      SKL_UINT32 Skip = (Local.Bits >> (32-12)) & 0x3f;
      i += Skip;
      if (i>=0) break;  // error
      Local.Discard_Safe( 6+6 );
      Val = Read_MPEG2_Escape(&Local);
    }
    else {
      Local.Discard_Word(Tab->Len);
      i += Tab->Run;
      if (i>=0) break;  // EOB2
      Val = Tab->Level;
      if (Local.Get_Bits(1)) Val = -Val;
    }
    Out[Scan[i++]] = Val;
    Mismatch ^= Val;
  }
  Out[63] ^= Mismatch & 1;

  FBB->Copy(&Local);
}

static
void Read_Intra_Block_B15_MPEG2(SKL_FBB *FBB, SKL_MB2 *MB, SKL_INT16 *Out)
{
  const int * const Scan = MB->Scan_Order;
  SKL_FBB Local;
  Local.Copy(FBB);

  SKL_INT32 Mismatch = Out[0];

  int i = -63;
  while(1)
  {
    int Val;
    Local.Check_Bits(16);
    const SKL_DCT_VLC *Tab = Skl_DCT_VLC_Search( B15_AC_Chunks, Local.Bits );
    if (Tab->Run<0) // ESC
    {
        // parse away 6bits for ESC code and 6bits for 'Skip' value
      SKL_UINT32 Skip = (Local.Bits >> (32-12)) & 0x3f;
      i += Skip;
      if (i>=0) break;  // error
      Local.Discard_Safe( 6+6 );
      Val = Read_MPEG2_Escape(&Local);
    }
    else {
      Local.Discard_Word(Tab->Len);
      i += Tab->Run;
      if (i>=0) break;  // EOB2
      Val = Tab->Level;
      if (Local.Get_Bits(1)) Val = -Val;
    }
    Out[Scan[i++]] = Val;
    Mismatch ^= Val;
  }
  Out[63] ^= Mismatch & 1;

  FBB->Copy(&Local);
}

//////////////////////////////////////////////////////////
// MPEG-2 Inter block

static
int Read_Inter_Block_MPEG2(SKL_FBB *FBB, SKL_MB2 *MB, SKL_INT16 *Out)
{
  const int * const Scan = MB->Scan_Order;
  SKL_FBB Local;
  Local.Copy(FBB);

  Local.Check_Bits(16);
  const SKL_DCT_VLC *Tab = Skl_DCT_VLC_Search( B14_DC_Chunks, Local.Bits );

  int i = -64;
  int Rows = 0x00;
  while(1)
  {
    int Val;
    SKL_ASSERT(Tab!=0);
    if (Tab->Run<0) // ESC
    {
        // parse away 6bits (=Tab->Len) for ESC code
        // and 6bits for 'Skip' value
      SKL_UINT32 Skip = (Local.Bits >> (32-12)) & 0x3f;
      i += Skip;
      if (i>=0) break;  // error
      Local.Discard_Safe( 6+6 /*Tab->Len+6*/ );
      Val = Read_MPEG2_Escape(&Local);
    }
    else {
      Local.Discard_Word(Tab->Len);
      i += Tab->Run;
      if (i>=0) break;  // EOB
      Val = Tab->Level;
      if (Local.Get_Bits(1)) Val = -Val;
    }
    const int j = Scan[i++];
    Out[j] = Val;
    Rows |= SKL_MP4_I::Row_From_Index[j];

      // next lump
    Local.Check_Bits(16);
    Tab = Skl_DCT_VLC_Search( B14_AC_Chunks, Local.Bits );
  }

  FBB->Copy(&Local);

  return Rows;
}

//////////////////////////////////////////////////////////
// ISO/IEC 13818-2 section 7.2 -> 7.5

// -- Intra blocks

inline void SKL_MB2::Decode_Intra_Macroblock()
{
  SKL_INT16 Base[7*64+SKL_ALIGN];
  SKL_INT16 * const In = (SKL_INT16*)SKL_ALIGN_PTR(Base, SKL_ALIGN);
  SKL_INT16 *Out = In + 1*64;

  SKL_ASSERT(Is_Intra());

  if (!Has_Concealment_MV())
  {
    Reset_Fwd(); // reset all PMV
    Reset_Bwd();
  }
  else // read concealment motion vectors
  {
    if (Is_MPEG1()) Read_Motion_Vectors_MPEG1(this, 0);
    else if (Motion()==MC_FIELD) Read_Field_Motion_Vectors(this, 0);
    else Read_Motion_Vectors(this, 0);
    SKL_COPY_MV(PMV[1][0], PMV[0][0]);
    SKL_COPY_MV(PMV[1][1], PMV[0][1]);
    FBB->Discard(1); // marker bit
  }

  for(int blk=0; blk<6; ++blk)
  {
    VOL->Quant_Ops.Zero(In);
    const int cc = (blk<4) ? 0 : blk-3;
    DC[cc] += Read_DC_Diff_MPEG2(FBB, DCT_Chunks[cc]);
    In[0] = DC[cc];

    Read_Intra(FBB, this, In);
    VOL->Quant_Ops.Dequant_Intra(Out, In, VOL->Q_Intra, Quant, DC_Q);
    Out += 64;
  }
  Copy_16To8(In+1*64);

      // ISO/IEC 11172-2 section 2.4.2.7 & 2.4.3.6
  if (VOL->Is_D_VOP()) FBB->Get_Bits(1);
}

//////////////////////////////////////////////////////////
// -- Inter blocks

inline void SKL_MB2::Decode_Inter_Macroblock()
{
  SKL_ASSERT(!Is_Intra());

  Reset_DC();  // note in 7.2.1

  if (VOL->Is_P_VOP() && !Is_Forward()) // section 7.6.3.5
  {
    Reset_Fwd();  // only reset fwd pmv
    Copy_Fwd();   // simply copy
    Reset_Motion_Type();
  }
  else if (Has_Motion())   // section 6.2.5
  {
    if (Is_MPEG1())  Predict_MPEG1();
    else {
      switch (Motion()) {
        case MC_NONE:  /* aieee! */
        break;
        case MC_FIELD:
          Predict_Fields();
        break;
        case MC_FRAME:
          Predict_Frame();
        break;
        case MC_DMV:
          Predict_DMV();
        break;
      }
    }
  }
  else Copy_Fwd();

  if (Is_Pattern())     // coded
  {
    SKL_INT16 Base[7*64+SKL_ALIGN];
    SKL_INT16 * const In = (SKL_INT16*)SKL_ALIGN_PTR(Base, SKL_ALIGN);
    SKL_INT16 *Out = In + 1*64;

    Cbp = Read_Coded_Block_Pattern(FBB);

    for(int blk=(1<<5); blk; blk>>=1)
    {
      if (Cbp & blk)      // coded
      {
        VOL->Quant_Ops.Zero(In);
        const int Rows = Read_Inter(FBB, this, In);
        VOL->Quant_Ops.Dequant_Inter(Out, In, VOL->Q_Inter, Quant, Rows&0xff);
      }
      Out += 64;
    }
    Add_16To8(In + 1*64);
  }
}

//////////////////////////////////////////////////////////
// section 7.6.3.4

inline int SKL_MB2::Skip_Macroblock(int Skip)
{
  if (Skip>1)
  {
    Reset_DC();   // section 7.2.1

    if (VOL->Is_B_VOP()) { // => re-use previous motion vectors
      while(Skip-->1) {
        if (!Next()) break;
        Predict_Reuse();
      }
    }
    else {
      if ( VOL->Is_P_VOP() )
        Reset_Fwd(); // section 7.6.3.4
      while(Skip-->1) {
        if (!Next()) break;
        Copy_Fwd();
      }
    }
    Reset_Motion_Type();
  }
  return !Next();
}

//////////////////////////////////////////////////////////
// section 6.2.3.6 (picture data)

SKL_MB2::SKL_MB2(const SKL_MP4_I *vol, SKL_FBB *fbb) 
  : SKL_MB(vol), FBB(fbb)
{
  DC_Prec = (vol->MPEG12_Ext_Flags>>12) & 0x03;
  DC_Q    = 1<<(3-DC_Prec);

  Pic_Struct     = (vol->MPEG12_Ext_Flags>>10) & 0x03;
  Frame_Pred_DCT = (vol->MPEG12_Ext_Flags>> 8) & 0x01;
  Concealment_MV = (vol->MPEG12_Ext_Flags>> 7) & 0x01;
  int Intra_VLC  = (vol->MPEG12_Ext_Flags>> 5) & 0x01;

  if (Is_MPEG1()) {
    Read_Intra = ( VOL->Is_D_VOP() ? Read_Intra_Block_DType_MPEG1
                                   : Read_Intra_Block_MPEG1 );
    Read_Inter = Read_Inter_Block_MPEG1;
  }
  else  {
    Read_Intra = ( (Intra_VLC==0) ? Read_Intra_Block_B14_MPEG2
                                  : Read_Intra_Block_B15_MPEG2 );
    Read_Inter = Read_Inter_Block_MPEG2;
  }

  Scan_Order = VOL->Alt_Vert_Scan ? SKL_MP4_I::Scan_Order[2]
                                  : SKL_MP4_I::Scan_Order[0];
  Scan_Order += 64;

  Reset_DC();  // reset prediction (section 7.2.1)
  Reset_Fwd(); // reset all MV prediction (section 7.6.3.4)  
  Reset_Bwd();
}

void SKL_MB2::Print_Infos() const {
  printf( "Q:%d DC-prec:%d Frm_Pred:%d IntraVLC:%d\n",
    Quant, DC_Prec, Frame_Pred_DCT, Read_Intra==Read_Intra_Block_B15_MPEG2);
}

static int Read_Frame_Slice(SKL_FBB * const Bits, int yPos, const SKL_MP4_I *VOL)
{
  SKL_MB2 MB(VOL, Bits);

    // (section 6.2.4) (Yet Another Patch)
  if (VOL->MPEG_Version==2 && VOL->Width>2800)
    yPos += Bits->Get_Bits(3) << 7;

    // partition breakpoint ignored, here

  MB.Quant = Bits->Get_Bits(5);

    // We ignore Intra_Slice_Flag (1b), Intra_Slice(1b) and
    // Reserved field (7b) and discard them all:
  while(Bits->Get_Bits(1)) Bits->Discard(8);

  MB.Init_Scan(yPos, Read_Addr_Incr(Bits) - 1);

  if (VOL->Debug_Level==3) MB.Print_Infos();

    // now, go for it. Reconstruct.

  while(1)
  {
      // Macroblock_Modes (section 6.2.5.1)

    // no scale extension, here
    MB.MB_Type = Read_MBlk_Type( Bits, VOL->Coding );
    // no STWCF here

    if (MB.Has_Motion() && !MB.Is_DC_Predicted()) // frame motion type (~Table 6-17)
      MB.MB_Type |= Bits->Get_Bits(2) << 8;
    else MB.MB_Type |= MC_FRAME;

      // set DCT_Type
    if ( !MB.Is_DC_Predicted() && MB.Is_Intra_Pattern() )
      MB.Set_Field_DCT( Bits->Get_Bits(1) );
    else
      MB.Set_Field_DCT( -1 );

    if (MB.Is_Quant())
      MB.Quant = Bits->Get_Bits(5);

    if (MB.Is_Intra())
      MB.Decode_Intra_Macroblock();
    else
      MB.Decode_Inter_Macroblock();

    if (Bits->See_Bits(23)==0)
      break;

    int Skip = Read_Addr_Incr(Bits);
    if (Skip==0) break;           // error or end-of-slice
    if (MB.Skip_Macroblock(Skip)) //section 7.6.3.4
      break;
  }
  return MB.Finished();
}

/*
static int Read_Field_Slice(SKL_FBB * const Bits, int yPos, 
                            const SKL_MP4_I *VOL, int Bottom_Field)
{
  SKL_MB2 MB(VOL, Bits);

  MB.Set_Field_DCT( 1 );
  MB.Set_Field_Pred();
  
    // (section 6.2.4)
  if (VOL->MPEG_Version==2 && VOL->Width>2800)
    yPos += Bits->Get_Bits(3) << 7;

    // partition breakpoint ignored, here

  MB.Quant = Bits->Get_Bits(5);

    // We ignore Intra_Slice_Flag (1b), Intra_Slice(1b) and
    // Reserved field (7b) and discard them all:
  while(Bits->Get_Bits(1)) Bits->Discard(8);

  MB.Init_Scan(yPos, Read_Addr_Incr(Bits) - 1);

    // now, go for it. Reconstruct.

  while(1)
  {
        // Macroblock_Modes (section 6.2.5.1)

    // no scale extension, here
    MB.MB_Type = Read_MBlk_Type( Bits, VOL->Coding );
    // no STWCF here

    if (MB.Has_Motion()) {
      MB.MB_Type |= Bits->Get_Bits(2) << 8;
      // if (MB.Motion()==MC_16x8)   =>  MV_Count = 2;
    }
    else {
      if (MB.Is_Intra() && MB.Has_Concealment_MV())
        MB.MB_Type |= MC_FIELD;
      else
        MB.MB_Type |= MC_FRAME;
    }      

    if (MB.Is_Quant())
      MB.Quant = Bits->Get_Bits(5);

    if (MB.Is_Intra())
      MB.Decode_Intra_Macroblock();
    else
      MB.Decode_Inter_Macroblock();

    if (Bits->See_Bits(23)==0)
      break;

    int Skip = Read_Addr_Incr(Bits);
    if (Skip==0) break;  // error or end-of-slice
    if (MB.Skip_Macroblock(Skip)) //section 7.6.3.4
      break;
  }
  return MB.Finished();
}
*/

//////////////////////////////////////////////////////////
// Headers decoding
//////////////////////////////////////////////////////////

void SKL_MP4_I::Init_MPEG12(int Version)
{
  SKL_ASSERT(Version==1 || Version==2);

  Init_VOL(0);
  MPEG_Version = Version;
  Low_Delay  = 0;  // force
  Quarter    = 0;
  Rounding   = 0;
  Set_Rounding();
  if (Version==1) {
    Set_Quant_Type(2);
/*
    Is_Progressive    = 1;
    Progressive_Frame = 1;
    DC_Prec           = 0;
    Pic_Struct        = MPEG12_FRAME_PIC;
    Frame_Pred_DCT    = 1;
    QScale_Type       = 0;
    Intra_VLC         = 0;
    Concealment_MV    = 0;    
    Matrix_Coeffs     = 5;
*/    
    MPEG12_Flags = 0x01;
    MPEG12_Ext_Flags = (3<<10) | (1<<8);
  }
  if (Version==2) {
    Set_Quant_Type(3);
    MPEG12_Flags = 0x01;
    MPEG12_Ext_Flags = (3<<10) | (1<<8);
  }
}

static void Init_MPEG1_Constrained_Param(SKL_MP4_I * const VOP)
{}

//////////////////////////////////////////////////////////

static int Read_Picture_Header(SKL_FBB * const Bits, SKL_MP4_I * const VOP)
{
  static const int Cvrt[8] = { 
    BAD_VOP, I_VOP, P_VOP, B_VOP, D_VOP, BAD_VOP, BAD_VOP, BAD_VOP };

  Bits->Discard(10);                       // temporal ref.
  VOP->Coding = Cvrt[Bits->Get_Bits(3)];   // coding type
  if (VOP->Coding==BAD_VOP) return 0;      // D-VOP?!
  Bits->Discard(16);                       // VBV delay


  if ( !VOP->Is_I_VOP() ) {                // MPEG-1 Fwd/Bwd code
    VOP->Fwd_Code = Bits->Get_Bits(4);     // 1 + 3bits
    if (VOP->Is_B_VOP())
      VOP->Bwd_Code = Bits->Get_Bits(4);   // 1 + 3bits
  }

  if (VOP->Debug_Level==4) {
    printf( "  Coding=[%c] ", "IPBD"[VOP->Coding] );
    printf( " Fwd/Bwd: %d/%d\n", VOP->Fwd_Code, VOP->Bwd_Code );
  }

  VOP->Last = 0;
  if (VOP->Is_B_VOP()) {  // reorder
    VOP->Cur = VOP->Aux;
  }
  else {
    VOP->Cur    = VOP->Past;
    VOP->Past   = VOP->Future;
    VOP->Future = VOP->Cur;
  }

  if (VOP->Cur==0) return 0; // no Sequence header seen yet?!?

  VOP->Cur->Time = VOP->Past->Time + 1;   // artificial clock
  return 1;
}

//////////////////////////////////////////////////////////
// Sequence (section 6.2.2.1)

/*
static const float Frame_Rate_Table[] = {
  0.,              // Forbidden
  24000.f/1001.f,  24.f, 25.f, 
  30000.f/1001.f,  30.f, 50.f, 
  60000.f/1001.f,  60.f
};

static const float MPEG1_Aspect_Ratio_Table[] = {
  0.0, // Forbidden
  1.0000f, 0.6735f, 0.7031f, 0.7615f,
  0.8055f, 0.8437f, 0.8935f, 0.9375f,
  0.9815f, 1.0255f, 1.0695f, 1.1250f,
  1.1575f, 1.2015f, 
  0.0 // reserved
};

static const float MPEG2_Aspect_Ratio_Table[] = {
  0.0, // Forbidden
  1.0f, 3.0f/4.0f, 9.0f/16.0f, 12.0f/21.0f
};
*/

static int Read_Sequence(SKL_FBB * const Bits, SKL_MP4_I * const VOP)
{
  VOP->Init_MPEG12(1);

  int Width        = Bits->Get_Bits(12);
  int Height       = Bits->Get_Bits(12);

#if 0
  int Aspect_Ratio = Bits->Get_Bits(4);
  int Frame_Rate   = (int)( 1000.f*Frame_Rate_Table[Bits->Get_Bits(4)] );
  int Bit_Rate     = Bits->Get_Bits(18);
  Bits->Discard(1);  // marker bit
  int VBV_Size     = Bits->Get_Bits(10);
#else
  Bits->Discard( 4+18 );
  Bits->Discard( 4+1+10 );
#endif

  VOP->Init_Pics( Width, Height, VOP->Low_Delay ? 2 : 3, 0 );

  if (Bits->Get_Bits(1))
    Init_MPEG1_Constrained_Param(VOP);

  int i;
  SKL_BYTE Mo[64];
  if (Bits->Get_Bits(1))
    for(i=0; i<64; i++) Mo[SKL_MP4_I::Scan_Order[0][i]] = Bits->Get_Bits(8);
  else
    VOP->Get_Default_Matrix(Mo, 2);
  VOP->Set_Matrix(1, Mo);

  if (Bits->Get_Bits(1))
    for(i=0; i<64; i++) Mo[SKL_MP4_I::Scan_Order[0][i]] = Bits->Get_Bits(8);
  else
    VOP->Get_Default_Matrix(Mo, 3);
  VOP->Set_Matrix(0, Mo);

  return 1;
}

static int Read_Sequence_Extension(SKL_FBB * const Bits, SKL_MP4_I * const VOP)
{
  SKL_UINT32 Code;

  VOP->Init_MPEG12(2);

  Code = Bits->Get_Bits(15);
  /* Profile_And_Level = (Code>>7) & 0xff; */ // 8bits
  
  int Is_Progressive = (Code>>6) & 0x01;      // 1bit
  VOP->MPEG12_Flags |= Is_Progressive;

  int Chroma_Fmt = (Code>>4) & 0x03;          // 2bits
  if (Chroma_Fmt!=1)  // 4:2:0
  {
    fprintf( stderr, "Only chroma format 4:2:0 is supported!\n" );
    return 0;
  }

  if (Code&0x0f) {
    int W  = VOP->Width  & 0xfff;
    W |= ((Code>>2)&0x03) << 12;              // 2bits
    int H  = VOP->Height & 0xfff;
    H |= ((Code>>0)&0x03) << 12;              // 2bits
    VOP->Init_Pics( W, H, 3, 0); 
  }

#if 0
    // the rest (Bit_Rate_Ext, VBVSize, Low_Delay, Frame_Rate_Ext...) is ignored
  Code = Bits->Get_Bits(21);
  Bit_Rate |= ((Code>>9) & 0x3ff) << 18;      // 12bits

  Mark = (Code>>8)&0x01;                      // 1bit marker

  int VBVS = Get_VBV_Size() & 0x3ff;
  VBVS |= ((Code>>0)&0xff) << 10;             // 8bits
  Set_VBV_Size(VBVS);
#else
  Bits->Discard(12+1+8);
#endif

  return 1;
}

static int Read_Coding_Extension(SKL_FBB * const Bits, SKL_MP4_I *VOP)
{
  SKL_UINT32 Code;

  Code = Bits->Get_Bits(16);

  VOP->Fwd_Code = (Code >> 8) & 0xff;          // 4+4 bits
  VOP->Bwd_Code = (Code >> 0) & 0xff;          // 4+4 bits

  Code = Bits->Get_Bits(14);
  VOP->MPEG12_Ext_Flags = Code;

  int QScale_Type      = (Code>> 6) & 0x01;    // 1bit
  VOP->Set_Quant_Type( 3+QScale_Type );

  VOP->Top_Field_First = (Code>> 9) & 0x01;    // 1bit
  VOP->Alt_Vert_Scan   = (Code>> 4) & 0x01;    // 1bit
/*
  Repeat_First_Field   = (Code>> 3) & 0x01;    // 1bit
  Chroma_420_Type      = (Code>> 2) & 0x01;    // 1bit
  Progressive_Frame    = (Code>> 1) & 0x01;    // 1bit
*/

  // the rest (Composite_Display,...) is ignored (20bits)
  if (Code&0x01) Bits->Discard(20);

  return 1;
}

static int Read_Extensions(SKL_FBB * const Bits, SKL_MP4_I * const VOP)
{
  switch(Bits->Get_Bits(4)) // Id
  {
    case MPEG12_SEQ_EXT_ID:
      if (!Read_Sequence_Extension(Bits,VOP))
        return 0;
    break;
    case MPEG12_QMATRIX_EXT_ID:    // Matrix Extension
    {
          // Format 4:4:4 is not supported. We discard the matrices...
      int i;
      if (Bits->Get_Bits(1)) for(i=0; i<64; i++) Bits->Get_Bits(8);
      if (Bits->Get_Bits(1)) for(i=0; i<64; i++) Bits->Get_Bits(8);
    }
    break;
    case MPEG12_PIC_CODING_EXT_ID: 
      if (!Read_Coding_Extension(Bits,VOP))
        return 0;
    break;
    default:  // ignore (scalable-ext, video-ext, etc...)
    break;
  }
  return 1;
}

static int Read_GOP_Header(SKL_FBB * const Bits, SKL_MP4_I * const VOP)
{
  SKL_UINT32 Code;

  Code = Bits->Get_Bits(19);
  /* int Drop = (Code>>18)&0x01;*/ 
  int Tc_Hours = (Code>>13)&0x1f; // 5bits
  int Tc_Mins = (Code>>7)&0x3f;   // 6bits
  /* (Code>>6)&0x01 -> marker bit */
  int Tc_Secs = (Code>>0)&0x3f;   // 6bits
  (void)Tc_Hours; (void)Tc_Mins; (void)Tc_Secs;

  Code = Bits->Get_Bits(8);
/*
  int Pic_Count = (Code>>2)&0x3f;    6bits
  int Closed    = (Code>>1)&0x01;    1bit
  int Broken    = (Code>>0)&0x01;    1bit
*/
  return 1;
}

//////////////////////////////////////////////////////////
// Main entry point for MPEG1/2 decoding
//////////////////////////////////////////////////////////

int SKL_MP4_I::Decode_MPEG12(const SKL_BYTE *Buf, int Len)
{
  SKL_UINT32 Code = 0xdeadc0de;

  SKL_ASSERT(MPEG_Version<=2); // don't mix!

  if (Len==0) { // special case: post very last residual frame
    Last = Cur;
    Cur  = 0;
    Frame_Number++;
    return 0;
  }

  const SKL_BYTE * const Buf_End = Buf + Len;
  while(1)
  {
    do {
      Code = (Code<<8) | (*Buf++);
      if (Buf>=Buf_End) return Len;
    }
    while (Code>MPEG12_LAST_CODE);
    if (Code<MPEG12_PICTURE_START) continue;

    SKL_FBB Bits(Buf);
    if (Code<=MPEG12_SLICE_MAX)
    {
      if (Code==MPEG12_PICTURE_START)    // 0x100
      {
        if (!Read_Picture_Header(&Bits, this)) break;
      }
      else /*Code<=MPEG12_SLICE_MAX*/    // 0x101-0x1af
      {
        int Pic_Struct = (MPEG12_Ext_Flags>>10) & 0x03;
        int Finished;
        if (Pic_Struct==MPEG12_FRAME_PIC) 
          Finished = Read_Frame_Slice(&Bits, Code-MPEG12_SLICE_MIN, this);
        else
//        Finished = Read_Field_Slice(&Bits, Code-MPEG12_SLICE_MIN, this, 
//                                    (Pic_Struct==MPEG12_BOTTOM_FIELD));
          continue; // FIELD picture structure not supported (it's a mess)...
        Switch_Off();    // reset (emms)
        Buf = Bits.Pos();

        if (Finished) {   // post previous frame to user
          if (Frame_Number==0) Last = 0;
          else if (Is_B_VOP()) Last = Cur;
          else                 Last = Past;
          Frame_Number++;
          break;
        }
      }
    }
    else
    {
      if (Code==MPEG12_USER_START)  // 0x1b2
      {
        continue;  // -skip-
      }
      else if (Code==MPEG12_SEQ_START)   // 0x1b3
      {
        if (!Read_Sequence(&Bits, this)) break;
      }
      else if (Code==MPEG12_EXT_START)   // 0x1b5
      {
        if (!Read_Extensions(&Bits, this)) break;
      }
      else if (Code==MPEG12_SEQ_END)     // 0x1b7
      {
        break;
      }
      else if (Code==MPEG12_GOP_START)   // 0x1b8
      {
        if (!Read_GOP_Header(&Bits,this)) break;
      }
      else if (Code>=SYSTEM_MIN_CODE) {
        fprintf( stderr, "Unexpected system code 0x%x. Is stream still multiplexed?\n", Code );
        continue;
      }
      else {
//        fprintf( stderr, "Unhandled code 0x%x.\n", Code );
      }
    }
    Buf = Bits.Pos();
  }

  return Len - (Buf_End - Buf);
}

//////////////////////////////////////////////////////////
