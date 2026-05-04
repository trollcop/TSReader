/********************************************************
 * Some code. Copyright (C) 2003 by Pascal Massimino.   *
 * All Rights Reserved.      (http://skal.planet-d.net) *
 * For Educational/Academic use ONLY. See 'LICENSE.TXT'.*
 ********************************************************/
/*
 * skl_mpg4_rvop.cpp
 *
 * MPEG4 decoder. Reduced-Resolution related funcs.
 ********************************************************/

#include "./skl_mpg4i.h"

//////////////////////////////////////////////////////////
// Reduced-I/P-VOP
//////////////////////////////////////////////////////////

void SKL_MB::Next_Reduced()
{
  SKL_ASSERT(VOL->Reduced_VOP>0);
  Tops[0] += 2;
  Tops[1] += 2;
  Tops[2] += 2;
  Tops[3] += 2;
  Tops[4] += 1;
  Tops[5] += 1;

  Curs[0] += 2;
  Curs[1] += 2;
  Curs[2] += 2;
  Curs[3] += 2;
  Curs[4] += 1;
  Curs[5] += 1;

  MVs  += 2;
  MVs2 += 2;

  Y1  += 32;
  Y2  += 32;
  U   += 16;
  V   += 16;

  Limit_Mins[0] -= 2*MB_Pels;
  Limit_Maxs[0] -= 2*MB_Pels;
  Limit_Mins_UV[0] -= 2*16;
  Limit_Maxs_UV[0] -= 2*16;

  x += 2;
  Pos += 2;
}

//////////////////////////////////////////////////////////
// section 7.6.10.1.5

static void Post_Filter(const SKL_MB * const MB)
{
  const int BpS = MB->VOL->BpS;
  if (MB->y>0 && MB->Map[MB->Pos-MB->VOL->MB_W].Type!=SKL_MAP_SKIPPED) {
    MB->VOL->MB_Ops.HFilter_31(MB->Y1-BpS, MB->Y1, 4);
    MB->VOL->MB_Ops.HFilter_31(MB->U-BpS, MB->U, 2);
    MB->VOL->MB_Ops.HFilter_31(MB->V-BpS, MB->V, 2);
  }
  MB->VOL->MB_Ops.HFilter_31(MB->Y1+16*BpS-BpS, MB->Y1+16*BpS, 4);

  if (MB->x>0 && MB->Map[MB->Pos-1].Type!=SKL_MAP_SKIPPED) {
    MB->VOL->MB_Ops.VFilter_31(MB->Y1-1, MB->Y1, BpS, 4);
    MB->VOL->MB_Ops.VFilter_31(MB->U-1, MB->U, BpS, 2);
    MB->VOL->MB_Ops.VFilter_31(MB->V-1, MB->V, BpS, 2);
  }  
  MB->VOL->MB_Ops.VFilter_31(MB->Y1+15, MB->Y1+16, BpS, 4);
}

//////////////////////////////////////////////////////////
// Writing an 'upsampling' version of IDct_Put/IDct_Add
// is out of question (my shrink said;). So, let's split
// the work in two:

inline
void SKL_MB::Copy_16To8_Upsampled(SKL_INT16 In[6*64]) const
{
  VOL->Quant_Ops.IDct_Sparse(In+0*64);
  VOL->Quant_Ops.IDct_Sparse(In+1*64);
  VOL->Quant_Ops.IDct_Sparse(In+2*64);
  VOL->Quant_Ops.IDct_Sparse(In+3*64);
  VOL->Quant_Ops.IDct_Sparse(In+4*64);
  VOL->Quant_Ops.IDct_Sparse(In+5*64);

  VOL->MB_Ops.Copy_Upsampled_8x8_16To8(Y1,   In+0*64, YBpS);
  VOL->MB_Ops.Copy_Upsampled_8x8_16To8(Y1+16,In+1*64, YBpS);
  if (Field_DCT<=0) {
    VOL->MB_Ops.Copy_Upsampled_8x8_16To8(Y1+2*BpS8,   In+2*64, YBpS);
    VOL->MB_Ops.Copy_Upsampled_8x8_16To8(Y1+2*BpS8+16,In+3*64, YBpS);
  }
  else {
    VOL->MB_Ops.Copy_Upsampled_8x8_16To8(Y1+BpS,   In+2*64, YBpS);
    VOL->MB_Ops.Copy_Upsampled_8x8_16To8(Y1+BpS+16,In+3*64, YBpS);
  }
  VOL->MB_Ops.Copy_Upsampled_8x8_16To8(U,    In+4*64, BpS);
  VOL->MB_Ops.Copy_Upsampled_8x8_16To8(V,    In+5*64, BpS);
}

inline
void SKL_MB::Add_16To8_Upsampled(SKL_INT16 In[6*64]) const
{
  if (Cbp&0x20) VOL->Quant_Ops.IDct_Sparse(In+0*64);
  if (Cbp&0x10) VOL->Quant_Ops.IDct_Sparse(In+1*64);
  if (Cbp&0x08) VOL->Quant_Ops.IDct_Sparse(In+2*64);
  if (Cbp&0x04) VOL->Quant_Ops.IDct_Sparse(In+3*64);
  if (Cbp&0x02) VOL->Quant_Ops.IDct_Sparse(In+4*64);
  if (Cbp&0x01) VOL->Quant_Ops.IDct_Sparse(In+5*64);

  if (Cbp&0x20) VOL->MB_Ops.Add_Upsampled_8x8_16To8(Y1,   In+0*64, YBpS);
  if (Cbp&0x10) VOL->MB_Ops.Add_Upsampled_8x8_16To8(Y1+16,In+1*64, YBpS);
  if (Field_DCT<=0) {
    if (Cbp&0x08) VOL->MB_Ops.Add_Upsampled_8x8_16To8(Y1+2*BpS8,   In+2*64, YBpS);
    if (Cbp&0x04) VOL->MB_Ops.Add_Upsampled_8x8_16To8(Y1+2*BpS8+16,In+3*64, YBpS);
  }
  else {
    if (Cbp&0x08) VOL->MB_Ops.Add_Upsampled_8x8_16To8(Y1+BpS,   In+2*64, YBpS);
    if (Cbp&0x04) VOL->MB_Ops.Add_Upsampled_8x8_16To8(Y1+BpS+16,In+3*64, YBpS);
  }
  if (Cbp&0x02) VOL->MB_Ops.Add_Upsampled_8x8_16To8(U,    In+4*64, BpS);
  if (Cbp&0x01) VOL->MB_Ops.Add_Upsampled_8x8_16To8(V,    In+5*64, BpS);
}

//////////////////////////////////////////////////////////
// This functions expands the informations from a 16x16
// block to a 32x32 one.

inline void SKL_MB::Expand_Reduced() const
{
  const int Type = MB_To_Map_Type[MB_Type];
  Map[Pos            ].Type = Type;
  Map[Pos          +1].Type = Type;
  Map[Pos+VOL->MB_W  ].Type = Type;
  Map[Pos+VOL->MB_W+1].Type = Type;

  Map[Pos          +1].dQ = 0;
  Map[Pos+VOL->MB_W  ].dQ = 0;
  Map[Pos+VOL->MB_W+1].dQ = 0;

    // expand the MVs. Normally, it should only be
    // useful if a BVOP follows (Low_Delay=0)
  SKL_MV * const MVs3 = MVs2 + MV_Stride;
  SKL_MV * const MVs4 = MVs3 + MV_Stride;
  if (Map[Pos].Type!=SKL_MAP_16x8) {  // SKIPPED, INTRA, 8x8 or 16x16
/*
   12|        11|22   <-MVs []
   34|        11|22   <-MVs2[]
   --+-   =>  --+--
     |        33|44   <-MVs3[]
              33|44   <-MVs4[]
*/
      /* expand lines #1/#3 */
    SKL_COPY_MV(MVs [2], MVs [1]);
    SKL_COPY_MV(MVs [3], MVs [1]);
    SKL_COPY_MV(MVs [1], MVs [0]);
    SKL_COPY_MV(MVs3[0], MVs2[0]);
    SKL_COPY_MV(MVs3[1], MVs2[0]);
    SKL_COPY_MV(MVs3[2], MVs2[1]);
    SKL_COPY_MV(MVs3[3], MVs2[1]);

      /* copy lines #1/#3 to #2/#4 */
    SKL_COPY_MV(MVs4[0], MVs3[0]);
    SKL_COPY_MV(MVs4[1], MVs3[1]);
    SKL_COPY_MV(MVs4[2], MVs3[2]);
    SKL_COPY_MV(MVs4[3], MVs3[3]);
    SKL_COPY_MV(MVs2[0], MVs [0]);
    SKL_COPY_MV(MVs2[1], MVs [1]);
    SKL_COPY_MV(MVs2[2], MVs [2]);
    SKL_COPY_MV(MVs2[3], MVs [3]);
  }
  else {    /* ?!?! TODO: is it right ?!?! */
/*
   12|        12|12   <-MVs []
   33|        33|33   <-MVs2[]
   --+-   =>  --+--
     |        12|12   <-MVs3[]
              33|33   <-MVs4[]
*/
    SKL_COPY_MV(MVs [2], MVs [0]);
    SKL_COPY_MV(MVs [3], MVs [1]);
    SKL_COPY_MV(MVs3[0], MVs [0]);
    SKL_COPY_MV(MVs3[1], MVs [1]);
    SKL_COPY_MV(MVs3[2], MVs [0]);
    SKL_COPY_MV(MVs3[3], MVs [1]);

    SKL_COPY_MV(MVs2[2], MVs2[0]);
    SKL_COPY_MV(MVs2[3], MVs2[0]);
    SKL_COPY_MV(MVs4[0], MVs2[0]);
    SKL_COPY_MV(MVs4[1], MVs2[0]);
    SKL_COPY_MV(MVs4[2], MVs2[0]);
    SKL_COPY_MV(MVs4[3], MVs2[0]);
  }
}

//////////////////////////////////////////////////////////

static void Read_Reduced_I_VOP(SKL_FBB * const Bits, const SKL_MP4_I * const VOP)
{
  SKL_ASSERT(VOP->Reduced_VOP>0);

  SKL_MB MB(VOP);
  SKL_INT16 Base[7*64+SKL_ALIGN];
  SKL_INT16 * const In = (SKL_INT16*)SKL_ALIGN_PTR(Base, SKL_ALIGN);

  while(MB.y<VOP->MB_H)
  {
    MB.Init_Scanline(VOP, 0);
    while(MB.x<VOP->MB_W)
    {
      MB.Decode_Intra_Infos(Bits);
      MB.Decode_Intra_Blocks(Bits, In, VOP);
      MB.Copy_16To8_Upsampled(In+1*64);
      MB.Store_Zero_MV();
      MB.Store_Map_Infos();
      Post_Filter(&MB);
      MB.Expand_Reduced();

      if (!MB.Resync || !MB.Resync_Marker(Bits))
        MB.Next_Reduced();

      if (VOP->Debug_Level==5) {
        printf( "%c", ".-="[1+MB.Field_DCT] );
        if (MB.x==VOP->MB_W)    printf( "\n" );
      }
    }
    if (VOP->Debug_Level==2) VOP->Dump_Line(0, &MB);
    if (MB.y>0 && VOP->Slicer) VOP->Slicer(VOP->Cur, (MB.y-2)*16, 32, VOP->Slicer_Data);
    MB.y += 2;
  }
  if (MB.y>0 && VOP->Slicer) VOP->Slicer(VOP->Cur, (MB.y-2)*16, 32, VOP->Slicer_Data);
}

//////////////////////////////////////////////////////////

#define SCALE_UP_MV(x)  ( ((x)<<1) - ((x)>0) + ((x)<0) )

void SKL_MB::Predict_Reduced_With_0MV() const
{
  const SKL_MB_FUNCS * const Ops = VOL->Copy_Ops;
  Ops->HP_16x8[0](Y1,         Y1         + Fwd_CoLoc, BpS);
  Ops->HP_16x8[0](Y1   +BpS8, Y1+   BpS8 + Fwd_CoLoc, BpS);
  Ops->HP_16x8[0](Y1+16,      Y1+16      + Fwd_CoLoc, BpS);
  Ops->HP_16x8[0](Y1+16+BpS8, Y1+16+BpS8 + Fwd_CoLoc, BpS);
  Ops->HP_16x8[0](Y1+2*BpS8,         Y1+2*BpS8         + Fwd_CoLoc, BpS);
  Ops->HP_16x8[0](Y1+2*BpS8   +BpS8, Y1+2*BpS8+   BpS8 + Fwd_CoLoc, BpS);
  Ops->HP_16x8[0](Y1+2*BpS8+16,      Y1+2*BpS8+16      + Fwd_CoLoc, BpS);
  Ops->HP_16x8[0](Y1+2*BpS8+16+BpS8, Y1+2*BpS8+16+BpS8 + Fwd_CoLoc, BpS);
  Ops->HP_16x8[0](U,          U          + Fwd_CoLoc, BpS);
  Ops->HP_16x8[0](U    +BpS8, U    +BpS8 + Fwd_CoLoc, BpS);
  Ops->HP_16x8[0](V,          V          + Fwd_CoLoc, BpS);
  Ops->HP_16x8[0](V    +BpS8, V    +BpS8 + Fwd_CoLoc, BpS);
}

void SKL_MB::Predict_Reduced_With_1MV(const SKL_MV MV) const
{
  const SKL_MB_FUNCS * const Ops = VOL->Copy_Ops;
  SKL_MV Tmp;
  Tmp[0] = SCALE_UP_MV(MV[0]);
  Tmp[1] = SCALE_UP_MV(MV[1]);
  Clip(Tmp, Tmp);  // TODO: dunno if it's correct.
                   // should we clip on 4x16x16 basis
                   // instead of a 32x32 one??

  if (!VOL->Quarter) {
    Predict_16x16   (Y1,           Y1          +Fwd_CoLoc, Tmp, Ops);
    Predict_16x16   (Y1       +16, Y1+       16+Fwd_CoLoc, Tmp, Ops);
    Predict_16x16   (Y1+2*BpS8,    Y1+2*BpS8   +Fwd_CoLoc, Tmp, Ops);
    Predict_16x16   (Y1+2*BpS8+16, Y1+2*BpS8+16+Fwd_CoLoc, Tmp, Ops);
  }
  else {
      // mirroring problem: should be 32x32 quarter prediction?
    Predict_16x16_QP(Y1,           Y1          +Fwd_CoLoc, Tmp, Ops);
    Predict_16x16_QP(Y1       +16, Y1+       16+Fwd_CoLoc, Tmp, Ops);
    Predict_16x16_QP(Y1+2*BpS8,    Y1+2*BpS8   +Fwd_CoLoc, Tmp, Ops);
    Predict_16x16_QP(Y1+2*BpS8+16, Y1+2*BpS8+16+Fwd_CoLoc, Tmp, Ops);
  }

  SKL_MV uv_MV;
  Derive_uv_MV_From_1MV(uv_MV, Tmp);

  const int Halves = (uv_MV[0]&1) | ((uv_MV[1]&1)<<1);
  const int Off = Fwd_CoLoc + (uv_MV[1]>>1)*BpS + (uv_MV[0]>>1);
  Ops->HP_16x8[Halves](U,      U     +Off, BpS);
  Ops->HP_16x8[Halves](V,      V     +Off, BpS);
  Ops->HP_16x8[Halves](U+BpS8, U+BpS8+Off, BpS);
  Ops->HP_16x8[Halves](V+BpS8, V+BpS8+Off, BpS);
}

void SKL_MB::Predict_Reduced_Fields(const SKL_MV MV[2], const int Fld_Dirs) const
{
  const SKL_MB_FUNCS * const Ops = VOL->Copy_Ops;
  int Off, Halves;
  SKL_MV uv_MV;
  SKL_MV Tmp;

    // 1rst field
  Tmp[0] = SCALE_UP_MV(MV[0][0]);
  Tmp[1] = SCALE_UP_MV(MV[0][1]);
  Clip_Field(Tmp, Tmp);   // TODO: dunno if it's correct.

  Off = Fwd_CoLoc;
  if (Fld_Dirs&2) Off += BpS;
  if (!VOL->Quarter) {
    Predict_16x8_Field(Y1,           Y1          +Off, Tmp, Ops);
    Predict_16x8_Field(Y1+16,        Y1+16       +Off, Tmp, Ops);
    Predict_16x8_Field(Y1   +2*BpS8, Y1   +2*BpS8+Off, Tmp, Ops);
    Predict_16x8_Field(Y1+16+2*BpS8, Y1+16+2*BpS8+Off, Tmp, Ops);
  }
  else  {
    Predict_16x8_Field_QP(Y1,           Y1          +Off, Tmp, Ops);
    Predict_16x8_Field_QP(Y1+16,        Y1+16       +Off, Tmp, Ops);
    Predict_16x8_Field_QP(Y1   +2*BpS8, Y1   +2*BpS8+Off, Tmp, Ops);
    Predict_16x8_Field_QP(Y1+16+2*BpS8, Y1+16+2*BpS8+Off, Tmp, Ops);
  }

  Derive_uv_MV_From_1MV(uv_MV, Tmp);
  Halves = (uv_MV[0]&1) | ((uv_MV[1]&1)<<1);
  Off += (uv_MV[0]>>1) + (uv_MV[1]&~1)*BpS;
  Ops->HP_16x8[Halves](U, U+Off, 2*BpS);
  Ops->HP_16x8[Halves](V, V+Off, 2*BpS);

    // 2nd field
  Tmp[0] = SCALE_UP_MV(MV[1][0]);
  Tmp[1] = SCALE_UP_MV(MV[1][1]);
  Clip_Field(Tmp, Tmp);   // TODO: (still) dunno if it's correct.

  Off = Fwd_CoLoc;
  if (Fld_Dirs&1) Off += BpS;
  if (!VOL->Quarter) {
    Predict_16x8_Field(Y1+BpS,           Y1          +Off, Tmp, Ops);
    Predict_16x8_Field(Y1+BpS+16,        Y1+16       +Off, Tmp, Ops);
    Predict_16x8_Field(Y1+BpS   +2*BpS8, Y1   +2*BpS8+Off, Tmp, Ops);
    Predict_16x8_Field(Y1+BpS+16+2*BpS8, Y1+16+2*BpS8+Off, Tmp, Ops);
  }
  else  {
    Predict_16x8_Field_QP(Y1+BpS,           Y1          +Off, Tmp, Ops);
    Predict_16x8_Field_QP(Y1+BpS+16,        Y1+16       +Off, Tmp, Ops);
    Predict_16x8_Field_QP(Y1+BpS   +2*BpS8, Y1   +2*BpS8+Off, Tmp, Ops);
    Predict_16x8_Field_QP(Y1+BpS+16+2*BpS8, Y1+16+2*BpS8+Off, Tmp, Ops);
  }

  Derive_uv_MV_From_1MV(uv_MV, Tmp);
  Halves = (uv_MV[0]&1) | ((uv_MV[1]&1)<<1);
  Off += (uv_MV[0]>>1) + (uv_MV[1]&~1)*BpS;
  Ops->HP_16x8[Halves](U+BpS, U+Off, 2*BpS);
  Ops->HP_16x8[Halves](V+BpS, V+Off, 2*BpS);
}

void SKL_MB::Predict_Reduced_With_4MV(const SKL_MV MV1[2], const SKL_MV MV2[2]) const
{
  const SKL_MB_FUNCS * const Ops = VOL->Copy_Ops;
  SKL_MV Tmp[4];

  Tmp[0][0] = SCALE_UP_MV(MV1[0][0]);
  Tmp[0][1] = SCALE_UP_MV(MV1[0][1]);
  Tmp[1][0] = SCALE_UP_MV(MV1[1][0]);
  Tmp[1][1] = SCALE_UP_MV(MV1[1][1]);
  Tmp[2][0] = SCALE_UP_MV(MV2[0][0]);
  Tmp[2][1] = SCALE_UP_MV(MV2[0][1]);
  Tmp[3][0] = SCALE_UP_MV(MV2[1][0]);
  Tmp[3][1] = SCALE_UP_MV(MV2[1][1]);
  Clip_Field(Tmp[0], Tmp[0]);   // TODO: dunno if it's correct.
  Clip_Field(Tmp[1], Tmp[1]);
  Clip_Field(Tmp[2], Tmp[2]);
  Clip_Field(Tmp[3], Tmp[3]);

  if (!VOL->Quarter) {
    SKL_BYTE * Y = Y1;
    Predict_16x8 (Y,        Y        +Fwd_CoLoc, Tmp[0], Ops);
    Predict_16x8 (Y+BpS8,   Y+BpS8   +Fwd_CoLoc, Tmp[0], Ops);
    Predict_16x8 (Y+16,     Y+16     +Fwd_CoLoc, Tmp[1], Ops);
    Predict_16x8 (Y+16+BpS8,Y+16+BpS8+Fwd_CoLoc, Tmp[1], Ops);

    Y += 2*BpS8;
    Predict_16x8 (Y,        Y        +Fwd_CoLoc, Tmp[2], Ops);
    Predict_16x8 (Y+BpS8,   Y+BpS8   +Fwd_CoLoc, Tmp[2], Ops);
    Predict_16x8 (Y+16,     Y+16     +Fwd_CoLoc, Tmp[3], Ops);
    Predict_16x8 (Y+16+BpS8,Y+16+BpS8+Fwd_CoLoc, Tmp[3], Ops);
  }
  else {
    SKL_BYTE * Y = Y1;
    Predict_16x16_QP(Y,   Y   +Fwd_CoLoc, Tmp[0], Ops);
    Predict_16x16_QP(Y+16,Y+16+Fwd_CoLoc, Tmp[1], Ops);

    Y += 2*BpS8;
    Predict_16x16_QP(Y,   Y   +Fwd_CoLoc, Tmp[2], Ops);
    Predict_16x16_QP(Y+16,Y+16+Fwd_CoLoc, Tmp[3], Ops);
  }

  SKL_MV uv_MV;
  Derive_uv_MV_From_4MV(uv_MV, &Tmp[0], &Tmp[2]);

  const int Halves = (uv_MV[0]&1) | ((uv_MV[1]&1)<<1);
  const int Off = Fwd_CoLoc + (uv_MV[1]>>1)*BpS + (uv_MV[0]>>1);
  Ops->HP_16x8[Halves](U,      U     +Off, BpS);
  Ops->HP_16x8[Halves](U+BpS8, U+BpS8+Off, BpS);
  Ops->HP_16x8[Halves](V,      V     +Off, BpS);
  Ops->HP_16x8[Halves](V+BpS8, V+BpS8+Off, BpS);
}

#undef SCALE_UP_MV

//////////////////////////////////////////////////////////

static void Read_Reduced_P_VOP(SKL_FBB * const Bits, const SKL_MP4_I * const VOP)
{
  SKL_ASSERT(VOP->Reduced_VOP>0);

  SKL_MB MB(VOP);
  SKL_INT16 Base[7*64+SKL_ALIGN];
  SKL_INT16 * const In = (SKL_INT16*)SKL_ALIGN_PTR(Base, SKL_ALIGN);

  while(MB.y<VOP->MB_H)
  {
    MB.Init_Scanline(VOP, 0);
    const SKL_MV *Left_Predictor = MB.MVs - 1;

    while(MB.x<VOP->MB_W)
    {
      if (Bits->Get_Bits(1)) // not coded
      {
        SKL_ASSERT(!MB.MC_Sel);   // no Reduced_VOP for GMC
        MB.MB_Type = SKL_MB_SKIPPED;
        MB.Predict_Reduced_With_0MV();
        MB.Store_Zero_MV();          
        MB.Set_Not_Intra();
        Left_Predictor = &MB.MVs[1];        
        // no post-filter for skipped macroblock
      }
      else
      {
        MB.Decode_Inter_Infos(Bits, Left_Predictor);
        SKL_ASSERT(MB.MC_Sel==0); // no reduced VOP for GMC

        if (MB.MB_Type >= SKL_MB_INTRA)  {  /* INTRA or INTRA_Q */
          MB.Decode_Intra_Blocks(Bits, In, VOP);
          MB.Copy_16To8_Upsampled(In+1*64);
          MB.Store_Zero_MV();
          Left_Predictor = &MB.MVs[1];
        }
        else
        {
          MB.Set_Not_Intra();
          if (MB.MB_Type == SKL_MB_INTER4V)
          {
            MB.Predict_Reduced_With_4MV(MB.MVs, MB.MVs2);
            Left_Predictor = &MB.MVs[1];
          }
          else   /*  INTER or INTER_Q */
          {
            if (MB.Field_Pred)
            {                
                // Impl: we don't multiply the final field 
                // vectors by 2 (cf. Store_16x8_MV())
              MB.Predict_Reduced_Fields(MB.MVs, MB.Field_Dir);
              MB.Store_16x8_MV(&MB.MVs2[0], &MB.MVs[0]); // field MV mode
              Left_Predictor = &MB.MVs2[1]; // wow!
            }
            else
            {
              MB.Predict_Reduced_With_1MV(MB.MVs[0]);
              MB.Store_16x16_MV();          // 16x16 mode
              Left_Predictor = &MB.MVs[1];
            }
          }

          MB.Decode_Inter_Blocks(Bits, In, VOP);
          MB.Add_16To8_Upsampled(In + 1*64);
        }
        Post_Filter(&MB);
      }
      MB.Store_Map_Infos();
      MB.Expand_Reduced();

      if (!MB.Resync || !MB.Resync_Marker(Bits))
        MB.Next_Reduced();

      if (VOP->Debug_Level==5) {
        if (MB.MB_Type==SKL_MB_SKIPPED) printf( "s" ); 
        else if (MB.Field_Pred==1) printf( "=" );
        else if (MB.MB_Type==SKL_MB_INTER4V) printf( "4" );
        else printf( "%c", ".-="[1+MB.Field_DCT] );
        if (MB.x==VOP->MB_W)    printf( "\n" );
      }
    }

    if (VOP->Debug_Level==2) VOP->Dump_Line(0, &MB);
    if (MB.y>0 && VOP->Slicer) VOP->Slicer(VOP->Cur, (MB.y-2)*16, 32, VOP->Slicer_Data);
    MB.y += 2;
  }
  if (VOP->Debug_Level==1)  VOP->Dump_MVs(VOP->Cur, 2);
  if (MB.y>0 && VOP->Slicer) VOP->Slicer(VOP->Cur, (MB.y-2)*16, 32, VOP->Slicer_Data);
}



//////////////////////////////////////////////////////////
// entry point

void SKL_MP4_I::Read_Reduced_VOP(SKL_FBB * const Bits) const
{
  SKL_ASSERT(Reduced_VOP==1 && Coding!=B_VOP && Coding!=S_VOP);
  if      (Coding==I_VOP) Read_Reduced_I_VOP(Bits, this);
  else /* P/S-VOP */      Read_Reduced_P_VOP(Bits, this);
}

//////////////////////////////////////////////////////////
//
//  Encoding
//
//////////////////////////////////////////////////////////

void SKL_MB_ENC::Copy_8To16_Downsampled(SKL_INT16 Out[6*64]) const
{
  if (Field_DCT<=0) {
    VOL->MB_Ops.Filter_18x18_To_8x8(Out+0*64, Y1,           BpS);
    VOL->MB_Ops.Filter_18x18_To_8x8(Out+1*64, Y1+16,        BpS);
    VOL->MB_Ops.Filter_18x18_To_8x8(Out+2*64, Y1+16*BpS,    BpS);
    VOL->MB_Ops.Filter_18x18_To_8x8(Out+3*64, Y1+16*BpS+16, BpS);
  }
  else {
    VOL->MB_Ops.Filter_18x18_To_8x8(Out+0*64, Y1,        2*BpS);
    VOL->MB_Ops.Filter_18x18_To_8x8(Out+1*64, Y1+16,     2*BpS);
    VOL->MB_Ops.Filter_18x18_To_8x8(Out+2*64, Y1+BpS,    2*BpS);
    VOL->MB_Ops.Filter_18x18_To_8x8(Out+3*64, Y1+BpS+16, 2*BpS);
  }
  VOL->MB_Ops.Filter_18x18_To_8x8(Out+4*64, U, BpS);
  VOL->MB_Ops.Filter_18x18_To_8x8(Out+5*64, V, BpS);
}

void SKL_MB_ENC::Diff_8To16_Downsampled(SKL_INT16 Out[6*64]) const
{
  if (Field_DCT<=0) {
    VOL->MB_Ops.Filter_Diff_18x18_To_8x8(Out+0*64, Y1,           BpS);
    VOL->MB_Ops.Filter_Diff_18x18_To_8x8(Out+1*64, Y1+16,        BpS);
    VOL->MB_Ops.Filter_Diff_18x18_To_8x8(Out+2*64, Y1+16*BpS,    BpS);
    VOL->MB_Ops.Filter_Diff_18x18_To_8x8(Out+3*64, Y1+16*BpS+16, BpS);
  }
  else {
    VOL->MB_Ops.Filter_Diff_18x18_To_8x8(Out+0*64, Y1,        2*BpS);
    VOL->MB_Ops.Filter_Diff_18x18_To_8x8(Out+1*64, Y1+16,     2*BpS);
    VOL->MB_Ops.Filter_Diff_18x18_To_8x8(Out+2*64, Y1+BpS,    2*BpS);
    VOL->MB_Ops.Filter_Diff_18x18_To_8x8(Out+3*64, Y1+BpS+16, 2*BpS);
  }
  VOL->MB_Ops.Filter_Diff_18x18_To_8x8( Out+4*64, U, BpS);
  VOL->MB_Ops.Filter_Diff_18x18_To_8x8( Out+5*64, V, BpS);
}

//////////////////////////////////////////////////////////  

void SKL_MB_ENC::Decimate_Reduced_Intra(SKL_INT16 Out[12*64]) const
{
  Copy_8To16_Downsampled(Out+6*64);  // import
  SKL_INT16 *C = Out;                // Decimate
  const SKL_MP4_ENC_I * const Vol = (const SKL_MP4_ENC_I *)VOL;
  for(int blk=0; blk<6; ++blk) {
    const int DC_Q = SKL_MP4_I::DC_Scales[(blk<4)][Quant-1];
    Vol->Decimate_Intra(C, C+6*64, Quant, DC_Q);
    C += 64;
  }
  Copy_16To8_Upsampled(Out+6*64);    // transfer back decimated data
}

//////////////////////////////////////////////////////////

void SKL_MP4_ENC_I::Write_Reduced_I_VOP(SKL_FBB * const Bits)
{
  SKL_INT16 Base[12*64+SKL_ALIGN];
  SKL_INT16 * const In = (SKL_INT16*)SKL_ALIGN_PTR(Base, SKL_ALIGN);

  SKL_MB_ENC MB(this);
  while(MB.y<MB_H)
  {
    MB.Init_Scanline(this, 0);
    while(MB.x<MB_W)
    {
      MB.MB_Type = SKL_MB_INTRA;
      MB.Set_Final_Params();
      SKL_ASSERT(MB.MB_Type == SKL_MB_INTRA);
      MB.Decimate_Reduced_Intra(In);
      MB.Encode_Intra(Bits, In, 1);
      Post_Filter(&MB);
      MB.Expand_Reduced();
      MB.Next_Reduced();
    }
    if (Debug_Level==2) Dump_Line(0, &MB);
    if (MB.y>0 && Slicer) Slicer(Cur, (MB.y-2)*16, 32, Slicer_Data);
    MB.y+=2;
  }
  if (MB.y>0 && Slicer) Slicer(Cur, (MB.y-2)*16, 32, Slicer_Data);

  Texture_Bits = MB.Get_Texture_Bits();
  MV_Bits = MB.Get_MV_Bits();
}

//////////////////////////////////////////////////////////

/*
#define SCALE_DOWN(V)   \
  (V)[0][0] = ( (V)[0][0] + (V)[2][0] + (V)[2*MV_Stride][0] + (V)[2*MV_Stride+2][0] ) >> 3; \
  (V)[0][1] = ( (V)[0][1] + (V)[2][1] + (V)[2*MV_Stride][1] + (V)[2*MV_Stride+2][1] ) >> 3
*/
#define SCALE_DOWN(V)   (V)[0][0] /= 2; (V)[0][1] /= 2

static inline int Find_Last(const SKL_INT16 C[6*64], const int *Zigzag, int i)
{
  while(i>=0)
    if (C[Zigzag[i]])
      return i;
    else i--;
  return -1;
}

void SKL_MB_ENC::Decimate_Reduced_Inter(SKL_INT16 Out[12*64])
{
  if (MB_Type==SKL_MB_SKIPPED) {
    Predict_Reduced_With_0MV();
    Store_Zero_MV();
    return;
  }

    // Fwd predict

  int Skippable = (dQuant==0) && SKL_IS_ZERO_MV(MVs[0]);
  Copy_8To16_Downsampled(Out+6*64);    // save original data
  if (Field_Pred) {                    // form predictions
    Predict_Reduced_Fields(MVs, Field_Dir);
    Store_16x8_MV(&MVs2[0], &MVs[0]);
    SKL_COPY_MV(MVs[1], MVs2[0]);      // update left predictor
    if (Skippable) Skippable &= SKL_IS_ZERO_MV(MVs2[0]);
  }
  else {
    if (MB_Type!=SKL_MB_INTER4V) {
      Predict_Reduced_With_1MV(MVs[0]);
      Store_16x16_MV();
    }
    else {
      Predict_Reduced_With_4MV(&MVs[0], &MVs2[0]);
      if (Skippable) {
        Skippable &= SKL_IS_ZERO_MV(MVs [1]);
        Skippable &= SKL_IS_ZERO_MV(MVs2[0]);
        Skippable &= SKL_IS_ZERO_MV(MVs2[1]);
      }
    }
  }

  Diff_8To16_Downsampled(Out+6*64);    // diff original/predicted data

  SKL_INT16 *C = Out;                  // Decimate and set Cbp
  Cbp = 0x00;
  const SKL_MP4_ENC_I * const Vol = (const SKL_MP4_ENC_I *)VOL;
  for(int blk=0; blk<6; ++blk)
  {
    const int SAV = Vol->Decimate_Inter(C, C+6*64,Quant);
    if (SAV>0) {
      Last[blk] = Find_Last(C, SKL_MP4_I::Scan_Order[0], 63);
      SKL_ASSERT(Last[blk]>=0);
      Cbp |= 1<<(5-blk);
    }
    else Last[blk] = -1;
    C += 64;
  }


  if (Skippable && Cbp==0)
    MB_Type = SKL_MB_SKIPPED;          // won't be coded (don't transfer back data)
  else
    Add_16To8_Upsampled(Out+6*64);     // transfer back decimated delta data
}

void SKL_MP4_ENC_I::Write_Reduced_P_VOP(SKL_FBB * const Bits)
{
  SKL_INT16 Base[12*64+SKL_ALIGN];
  SKL_INT16 * const In = (SKL_INT16*)SKL_ALIGN_PTR(Base, SKL_ALIGN);

  SKL_MB_ENC MB(this);
  while(MB.y<MB_H)
  {
    MB.Init_Scanline(this, 0);
    while(MB.x<MB_W)
    {
      MB.Set_Type();
      SKL_ASSERT(MB.MB_Type!=SKL_MB_INTER4V);
      if (MB.MB_Type!=SKL_MB_SKIPPED) {
        SCALE_DOWN(MB.MVs);
        MB.Set_Final_Params();
      }
      if (MB.MB_Type==SKL_MB_INTRA) MB.Decimate_Reduced_Intra(In);
      else                          MB.Decimate_Reduced_Inter(In);

      if (MB.MB_Type==SKL_MB_SKIPPED)
      {
        Bits->Put_Bits( 1, 1 );          // not_coded
        MB.Set_Not_Intra();
      }
      else {
        Bits->Put_Bits( 0, 1 );          // coded
        if (MB.MB_Type>=SKL_MB_INTRA) MB.Encode_Intra(Bits, In, 0);
        else                          MB.Encode_Inter(Bits, In, Fwd_Code);
        Post_Filter(&MB);
      }
      MB.Store_Map_Infos();
      MB.Expand_Reduced();
      MB.Next_Reduced();
    }
    if (Debug_Level==2) Dump_Line(0, &MB);
    if (MB.y>0 && Slicer) Slicer(Cur, (MB.y-2)*16, 32, Slicer_Data);
    MB.y += 2;
  }
  if (MB.y>0 && Slicer) Slicer(Cur, (MB.y-2)*16, 32, Slicer_Data);

  Texture_Bits = MB.Get_Texture_Bits();
  MV_Bits = MB.Get_MV_Bits();

  if (Debug_Level==1)  Dump_MVs(Cur, 2); // will trash the encoded frame
}

#undef SCALE_DOWN

//////////////////////////////////////////////////////////
