/********************************************************
 * Some code. Copyright (C) 2003 by Pascal Massimino.   *
 * All Rights Reserved.      (http://skal.planet-d.net) *
 * For Educational/Academic use ONLY. See 'LICENSE.TXT'.*
 ********************************************************/
/*
 * skl_mpg4i.h
 *
 * internal MPEG4 header
 ********************************************************/

#ifndef _SKL_MPG4I_H_
#define _SKL_MPG4I_H_

#include "skl.h"
#include "skl_syst/skl_mpg4.h"
#include "skl_syst/skl_bits.h"
#include "skl_syst/skl_dsp.h"
#include <math.h>

struct SKL_MP4_I;           // internal opaque structure
class  SKL_MP4_ENC_I;       // ditto.
class SKL_MP4_DEC_I;        // ditto.

struct SKL_MB;
struct SKL_MB_ENC;  // subclass for encoder
struct SKL_MB_DATA;

//////////////////////////////////////////////////////////
// #defines

  // section 6.2.1, Table 6-3
#define FIRST_CODE          0x00000100
#define VOL_CODE            0x00000120
#define SEQ_START_CODE      0x000001b0
#define SEQ_END_CODE        0x000001b1
#define USER_DATA_CODE      0x000001b2
#define VOP_GROUP_CODE      0x000001b3
#define VSESSION_ERR_CODE   0x000001b4
#define VO_START_CODE       0x000001b5
#define VOP_CODE            0x000001b6
#define MPEG2_SEQ_END_CODE  0x000001b7
#define MPEG2_GOP_CODE      0x000001b8
#define SYSTEM_MIN_CODE     0x000001b9
#define LAST_CODE           0x000001ff

enum {  // table 6-20
  I_VOP  = 0,
  P_VOP  = 1,
  B_VOP  = 2,
  S_VOP  = 3,
  D_VOP  = 3,    // <- old MPEG1 D-mode
  NC_VOP = 4,    // not coded
  BAD_VOP = 5
};

enum {  // table V2-2
  SPRITE_NONE   = 0,
  SPRITE_STATIC = 1,
  SPRITE_GMC    = 2,
        // table 6-22
  SPRITE_STOP   = 0,
  SPRITE_PIECE  = 1,
  SPRITE_UPDATE = 2,
  SPRITE_PAUSE  = 3
};

  // Macroblock sub-types for B_VOP. 
  // WARNING! they're not the same as the norm...
#define SKL_MB_DIRECT   0
#define SKL_MB_BWD      1
#define SKL_MB_FWD      2
#define SKL_MB_INTERP   3

//  Useful 'macros'
// Motion vectors are restricted to [-2048,2047]+Prediction (~VOP size)
// So we have some bits left for coding some special case within the 32b storage.

static inline void MARK_INVALID_MV(SKL_MV MV) { *(SKL_UINT32*)MV = 0x7fff7fff; }
static inline int IS_VALID_MV(const SKL_MV MV) { return *(SKL_UINT32*)MV != 0x7fff7fff; }
static inline int IS_EQUAL_MV(const SKL_MV a, const SKL_MV b) { return *(SKL_UINT32*)a == *(SKL_UINT32*)b; }
static inline int IS_ZERO_MV(const SKL_MV MV) { return *(SKL_UINT32*)MV == 0; }

//////////////////////////////////////////////////////////
// alignment macros

#ifdef SKL_USE_ASM
#define SKL_ALIGN 15
#else
#define SKL_ALIGN 0
#endif

#define SKL_ALIGN_PTR(P,ALGN) ( ((SKL_SAFE_INT)(P) + (ALGN)) & ~(ALGN) )

//////////////////////////////////////////////////////////
// internal structs

  // coding infos

struct SKL_MP4_FRAME : public SKL_MP4_INFOS
{
  int Coding;           /**< Frame type: 0:I, 1:P, 2:B, 3:S(/D), 4:not coded */
  int Quant;
  int Fwd_Code, Bwd_Code;
  int Rounding;
  int Reduced_VOP;      /**< -1: disabled, >=0: possibly enabled (0:off, 1:on) */
  int DC_Thresh;
  int Top_Field_First;
  int Alt_Vert_Scan;
  int Sprite_Nb_Pts;    /**< max: 4 for static sprite, 3 for GMC. -1 = disable */
  int S_Warp_Pts[4][2];
  int Sprite_Accuracy;
  SKL_UINT64 Time;      /**< in ticks */

  void Dump_Map(SKL_MP4_PIC *Pic, int What=0) const;  /**< intended for debugging */
};

//////////////////////////////////////////////////////////
// struct used for DC/AC prediction

struct SKL_MB_DATA {
  SKL_INT16 Q;          /**< Q=0 means: not intra coded, Q=-1 means: not coded */
  SKL_INT16 DC;
  SKL_INT16 AC[7];

  void Set_Not_Intra() { Q =  0; DC = 1024; }  /* should be: 1<<(Bpp+2) */
  int Choose_Pred_Dir(const SKL_MB_DATA *Top);
  void Add_DC_AC_Pred(SKL_INT16 In[64],
                      int Dir,
                      SKL_INT16 Left_AC[7],
                      const SKL_MB_DATA *Top,
                      int AC_Q,
                      int DC_Q);
};

  /** @internal
      it's a cursor (only 1 instance) 
    */
struct SKL_MB
{
  int x, y;
  int Pos;               /**< absolute position in an MB_WxMB_H array */
  int MB_Count;          /**< Counter from first macroblock of slice */
  int ABits;             /**< Neighbour-availability bits. Bit0: left *not* available, Bit1:top available, Bit2:top-right available */
  int MB_W;              /**< Pic width, in macroblock units */
  int Prev_Quant;
  int Quant;             /**< Duplicated from VOL */
  int Use_AC_Pred;
  int Cbp;
  int MB_Type;  
  int Field_DCT;               /**< -1: none. 0: frame, 1:field */
  int Field_Pred;
  int Field_Dir;               /**< fwd/dir field selection => bits 3:bwd top, 2:bwd bottom */
                               /**<                                 1:fwd top, 0:fwd bottom */
  int MC_Sel;
  SKL_MV Limit_Mins;           /**< Left/Top clipping limits for Luma */
  SKL_MV Limit_Maxs;           /**< Right/Bottom clipping limits for Luma */
  SKL_MV Limit_Mins_UV;        /**< Left/Top clipping limits for Chroma */
  SKL_MV Limit_Maxs_UV;        /**< Right/Bottom clipping limits for Chroma */
  int MB_Pels;                 /**< macroblock size in mv unit */

  const SKL_MP4_I * const VOL;
  SKL_BYTE *Y1;                /**< current macroblock reference (or 1rst field) */
  SKL_BYTE *Y2;                /**< 2nd field for Field-DCT */
  SKL_BYTE *U, *V;
  const int BpS, BpS8;         /**< Pixel strides, duplicated from VOL */
  int YBpS;                    /**< strides for Field DCT */
  const int Fwd_CoLoc;         /**< Forward colocation */
  const int Bwd_CoLoc;         /**< Backward colocation */
  SKL_BYTE * const YTmp;       /**< 16x17 scratch buffer for vertical qpel interp. */

  SKL_MB_DATA *Tops[6];        /**< top blocks data for each block */
  SKL_MB_DATA *Curs[6];        /**< cur blocks data */
  SKL_INT16   *Lefts[6];       /**< pointer to left ACs of each blocks */
  SKL_INT16    Left_ACs[4][7]; /**< left ACs for 2 lum rows + (1+1) chroma row */

  SKL_MV *MVs;                 /**< motion vectors starting at block 0 */
  SKL_MV *MVs2;                /**< second row of motion vectors */
  int MV_Stride;               /**< =2*EMB_W (stride for Motion Vectors) */

  SKL_MP4_MAP * const Map;     /**< MB infos map (for B-VOP and encoder) */

  int Resync;                  /**< Resync marker len (0: disabled) */

    // constructor. Will initialize the scan.

  SKL_MB(const SKL_MP4_I * const VOL);
  ~SKL_MB();

    // stach useful infos

  static const SKL_UINT8 MB_To_Map_Type[SKL_MB_LAST];  /**< Convert from internal MB_Type to SKL_MP4_MAP::Type public type */

  inline void Store_Map_Infos() const {
    Map[Pos].Type = MB_To_Map_Type[MB_Type];
    Map[Pos].Fields = Field_Dir;
  }

    // debug

  void Dump_Line(int What, const SKL_MP4_I * const VOL) const;

    // 16b->8b block ops

  inline void Set_Field_DCT(int fDCT);
  inline void Set_No_Field();
  inline void Copy_16To8(SKL_INT16 In[6*64]) const;
  inline void Add_16To8 (SKL_INT16 In[6*64]) const;

    // spatial prediction

  int Top_Ok()       const { return MB_Count>=MB_W; }
  int Top_Right_Ok() const { return MB_Count>=MB_W-1 && x<MB_W-1; }
  int Left_Ok()      const { return MB_Count>0 && x>0; }
  void Start_Scan() { ABits = 1; MB_Count = 0; }

  void Init_Scanline(const SKL_MP4_I * const VOL, int x);
  void operator++(int);
  void Next_Reduced();
  void Next_Line_Preds();
  void Init_Offset();
  void Resync_Cursor(const SKL_MP4_I * const VOL, int New_Pos);

  void Set_Not_Intra();

    // MV prediction

  void Predict_Motion_Vector(SKL_MV MV, const SKL_MV * const Src, const int Blk) const;
  void Predict_Motion_Vector_Blk0(SKL_MV MV, const SKL_MV * const Src, const SKL_MV * const Left) const;

  inline void Store_Zero_MV() const;
  inline void Store_16x16_MV() const;
  inline void Store_16x8_MV(SKL_MV *Dst, SKL_MV MV[2]) const;

  static const int Rnd_Tab_76[16];
  static const int Rnd_Tab_78[8];
  static const int Rnd_Tab_79[4];
  inline void Derive_uv_MV_From_1MV(SKL_MV MV, const SKL_MV MVo) const;
  inline void Derive_uv_MV_From_4MV(SKL_MV MV, const SKL_MV MV1[2], const SKL_MV MV2[2]) const;
  inline void Derive_uv_MVs_Fields(SKL_MV Out, const SKL_MV MVo) const;

  inline void Clip(SKL_MV Out, const SKL_MV In) const;
  inline void Clip_Chroma(SKL_MV Out, const SKL_MV In) const;
  inline void Clip_Field(SKL_MV Out, const SKL_MV In) const;
  inline void Clip_Chroma_Field(SKL_MV Out, const SKL_MV In) const;

  inline void Predict_16x16(SKL_BYTE * const Dst, const SKL_BYTE *Src, const SKL_MV MV, const SKL_MB_FUNCS * const Ops) const;
  inline void Predict_16x8(SKL_BYTE * const Dst, const SKL_BYTE *Src, const SKL_MV MV, const SKL_MB_FUNCS * const Ops) const;
  inline void Predict_16x8_Field(SKL_BYTE * const Dst, const SKL_BYTE *Src, const SKL_MV MV, const SKL_MB_FUNCS * const Ops) const;
  inline void Predict_8x8(SKL_BYTE * const Dst, const SKL_BYTE *Src, const SKL_MV MV, const SKL_MB_FUNCS * const Ops) const;

  inline void Predict_16x16_QP(SKL_BYTE * const Dst, const SKL_BYTE *Src, const SKL_MV MV, const SKL_MB_FUNCS * const Ops) const;
  inline void Predict_16x8_Field_QP(SKL_BYTE * const Dst, const SKL_BYTE *Src, const SKL_MV MV, const SKL_MB_FUNCS * const Ops) const;
  inline void Predict_8x8_QP(SKL_BYTE * const Dst, const SKL_BYTE *Src, const SKL_MV MV, const SKL_MB_FUNCS * const Ops) const;

  inline void Predict_With_0MV(const SKL_MB_FUNCS * const Ops, const int Fwd) const;
  inline void Predict_With_1MV(const SKL_MV MV, const SKL_MB_FUNCS * const Ops, const int Fwd) const;
  inline void Predict_Fields(const SKL_MV MV[2], const SKL_MB_FUNCS * const Ops, const int Fwd, const int Fld_Dirs) const;
  inline void Predict_With_4MV(const SKL_MV MV1[2], const SKL_MV MV2[2], const SKL_MB_FUNCS * const Ops, const int Fwd) const;
  inline void Predict_GMC() const;


    // reduced predictions
  void Copy_16To8_Upsampled(SKL_INT16 In[6*64]) const;
  void Add_16To8_Upsampled (SKL_INT16 In[6*64]) const;
  void Predict_Reduced_With_0MV() const;
  void Predict_Reduced_With_1MV(const SKL_MV MV) const;
  void Predict_Reduced_Fields(const SKL_MV MV[2], const int Fld_Dirs) const;
  void Predict_Reduced_With_4MV(const SKL_MV MV1[2], const SKL_MV MV2[2]) const;
  void Expand_Reduced() const;


    // bitstream parsing
  void Decode_Intra_Infos(SKL_FBB * const Bits);
  void Decode_Inter_Infos(SKL_FBB * const Bits, const SKL_MV * const Left_Predictor);
  void Decode_Intra_Blocks(SKL_FBB * const Bits, SKL_INT16 * const In,
                           const SKL_MP4_I * const VOL) const;
  void Decode_Inter_Blocks(SKL_FBB * const Bits, SKL_INT16 * const In,
                           const SKL_MP4_I * const VOL) const;

  int Resync_Marker(SKL_FBB * const Bits);  /**< returns true if resync marker encountered */
};

//////////////////////////////////////////////////////////
// SKL_MP4_I : internal data
//////////////////////////////////////////////////////////

/** Note: we've gathered useful informations, passed to the analyzer,
    into SKL_MP4_INFOS, although most of the managment of this
    class is done by the SKL_MP4_I one.
  */
struct SKL_MP4_I : public SKL_MP4_FRAME
{
  public:

     /* index: zigzag/horizontal/vertical/transposed-zigzag */
    static const int Scan_Order[4][64];
    static const SKL_BYTE Dflt_Mtx[3][64];  // [Intra/Inter/Intra-MPEG2][]
    static const int Row_From_Index[64];    // row number from index

    static const int DC_Scales[2][31];      // index: [Lum][AC_Q-1]

      // Frame-based parameters

    int EMB_W, EMB_H; // macroblocks size (w/ edges)
    SKL_MB_DATA *Y_Preds[3], *C_Preds[2];
    SKL_MP4_PIC *Aux, *Last;
    int Cur_Map;

    int Is_I_VOP() const { return (Coding==I_VOP); }
    int Is_P_VOP() const { return (Coding==P_VOP); }
    int Is_B_VOP() const { return (Coding==B_VOP); }
    int Is_S_VOP() const { return (Coding==S_VOP); }
    int Is_D_VOP() const { return (Coding==D_VOP); }


      // VOL global params

    int MPEG_Version;
    int VOL_Id;
    int Shape;
    int Low_Delay;
    int Not_8b, Quant_Prec, Bpp;
    int Interlace;
    int Quant_Type;   // should be private. Use Get/Set()!
    int Time_Frequency;
    int Ticks_Bits;
    int Ticks_Per_VOP;                 /* Fixed VOP rate. Unused for decoding. */
    void Set_Time_Frequency(int Freq); /* sets Time_Frequency and Ticks_Bits */

    int Resync;
    int Data_Partitioned;
    int Rev_VLC;
    int Quarter;
    SKL_UINT64 Time_Ref;
    SKL_UINT64 Time_Last_Coded;
    SKL_UINT64 Time_TFrame;

    int New_Pred;
    int Sprite_Mode;
    int Sprite_Brightness;
    int Sprite_Transmit;
    int Sprite_Low_Latency;
    void Read_Sprite_Trajectory(SKL_FBB * const Bits);
    int  Read_Sprite_Params(SKL_FBB * const Bits);

    int Field_Dir;        // inter-frame persistant field selectors

    SKL_UINT32 CE_Flags;  // complexity estimation flags (22b)
    int Scalability;

    int Debug_Level;
    void Set_Debug_Level(int Level) { Debug_Level = Level; }

    int Nb_Preds;
    SKL_MB_DATA *Preds;
    int Sanity_Check_Preds() const;

    enum { MAX_PICS = 3, MAX_MAPS = 2 };
    SKL_MP4_PIC All_Pics[MAX_PICS];
    int Nb_Pics;
    struct { 
      SKL_MP4_MAP *Map;
      SKL_MV      *MV;
    } All_Maps[MAX_MAPS];  // aux frame queue
    int Nb_Maps;

    int Pic_Size;
    SKL_BYTE *Pic_Base;

    int Custom_Intra, Custom_Inter;
    SKL_BYTE Intra_Matrix[64], Inter_Matrix[64];
    SKL_ANY Q_Base;
    SKL_QUANTIZER Q_Intra, Q_Inter;

    SKL_MP4_SLICER Slicer;
    SKL_ANY        Slicer_Data;
    void Set_Slicer(SKL_MP4_SLICER S, SKL_ANY Data);

    static void Get_Default_Matrix(SKL_BYTE M[64], int What);
    void Set_Quant_Type(int Quant_Type);
    int Get_Quant_Type() const { return Quant_Type; }
    int Set_Matrix(int Intra, const SKL_BYTE *M);


    SKL_CPU_FEATURE Cpu;
    void Set_CPU( SKL_CPU_FEATURE Cpu=SKL_CPU_LAST ); // LAST=choose best
    SKL_IMG_DSP         Img_Ops;
    SKL_MB_DSP          MB_Ops;
    SKL_QUANT_DSP       Quant_Ops;
    SKL_GMC_DSP         GMC_Ops;
    const SKL_MB_FUNCS *Add_Ops;
    const SKL_MB_FUNCS *Copy_Ops;

    void Set_Rounding();                    // will set *Add_Ops and *Copy_Ops up
    void Make_Edges_Dec(const SKL_MP4_PIC * const Pic) const;  // Replicate edges for decoder (padded to MB offsets)
    void Make_Edges_Enc(const SKL_MP4_PIC * const Pic) const;  // Replicate edges for encoder (padded to input WidthxHeight only)
    void Switch_Off() const { MB_Ops.Switch_Off(); }

      // MPEG1/2 bwd compat.
    void Init_MPEG12(int MPG_Version);   // version = 1 or 2
    SKL_UINT32 MPEG12_Flags;             // various flags...
    SKL_UINT32 MPEG12_Ext_Flags;         // 14bits

    void Read_Reduced_VOP(SKL_FBB * const Bits) const;

  public:

    SKL_MP4_I(SKL_MEM_I *Mem);
    ~SKL_MP4_I();
    void Init_VOL(int id);
    void Reset_VOL();

    void Init_Pics(int W, int H, int Nb_Frames, int Nb_Maps);
    void Clear_Pics();
    void Copy_Pic(SKL_MP4_PIC *Dst, const SKL_MP4_PIC *Src) const;

       // debug
    void Get_All_Frames(SKL_MP4_PIC *Pic) const;
    void Dump_MVs(const SKL_MP4_PIC * const Pic, int Incr=1) const;
    void Draw_GMC(const SKL_MP4_PIC * const Pic) const;
    void Dump_Line(int What, const SKL_MB * const MB) const;
    void Print_Infos() const;

       // main call
    int Decode(const SKL_BYTE *Buf, int Len);
    int Decode_MPEG12(const SKL_BYTE *Buf, int Len);
};

//////////////////////////////////////////////////////////   
// SKL_MP4_DEC_I
//////////////////////////////////////////////////////////

  // Warning: Slicing problem. Inherit from SKL_MP4_I first
  // so that cast to SKL_MP4_I* is ok.
class SKL_MP4_DEC_I : public SKL_MP4_I, private SKL_MP4_DEC
{
  public:
    SKL_MP4_DEC_I(SKL_MEM_I *Mem);
    virtual ~SKL_MP4_DEC_I();

  public:
      // Top class' implementation.
    virtual int Decode(const SKL_BYTE *Buf, int Len);
    virtual int Decode_MPEG12(const SKL_BYTE *Buf, int Len);
    virtual int Get_Frame_Number() const;
    virtual int Is_Frame_Ready() const;
    virtual void Consume_Frame(SKL_MP4_PIC *Pic);
    virtual SKL_MEM_I *Set_Memory_Manager(SKL_MEM_I *Mem=0);
    virtual void Set_CPU(SKL_CPU_FEATURE Cpu = SKL_CPU_DETECT);
    virtual void Set_Slicer(SKL_MP4_SLICER Slicer, SKL_ANY Slicer_Data=0);
    virtual void Set_Debug_Level(int Level=0);
    virtual void Get_All_Frames(SKL_MP4_PIC *Pic) const;
    virtual int Ioctl(SKL_CST_STRING Param);
};

//////////////////////////////////////////////////////////
// Inlined enc/dec common functions for 8b<->8b ops
//////////////////////////////////////////////////////////

//#define DIV2RND(x) ( ( (x) + ((x)>=0) ) >> 1 )
#define DIV2RND(x) ( (x)/2 )

inline
void SKL_MB::Derive_uv_MV_From_1MV(SKL_MV Out, const SKL_MV MVo) const
{
  int x = MVo[0];
  int y = MVo[1]; 
  if (VOL->Quarter) { x = DIV2RND(x); y = DIV2RND(y); }

    // Trick! using Rnd_Tab_79[] is equivalent to the following Div2Round():
    // => This mean we can use this func for MV Field-Chroma rounding too.

  Out[0] = (x>>1) | (x&1);    // eq. to:  Out[0] = (x>>1) + Rnd_Tab_79[ x & 0x3 ];
  Out[1] = (y>>1) | (y&1);    // eq. to:  Out[1] = (y>>1) + Rnd_Tab_79[ y & 0x3 ];
}

inline
void SKL_MB::Derive_uv_MVs_Fields(SKL_MV Out, const SKL_MV MVo) const
{
  int x = MVo[0], y = MVo[1];
  if (VOL->Quarter) { x = DIV2RND(x); y = y>>1; }   // Note: WARNING! It's really "y>>1", not DIV2RND(y) ! 
  Out[0] = (x>>1) | (x&1);
  Out[1] = (y>>1) | (y&1);
  
  Clip_Chroma_Field(Out, Out);
}

inline
void SKL_MB::Derive_uv_MV_From_4MV(SKL_MV Out,
                                   const SKL_MV MV1[2],
                                   const SKL_MV MV2[2]) const
{
  int x, y;
  if (VOL->Quarter) { 
    x  = DIV2RND(MV1[0][0]) + DIV2RND(MV1[1][0]);
    x += DIV2RND(MV2[0][0]) + DIV2RND(MV2[1][0]);
    y  = DIV2RND(MV1[0][1]) + DIV2RND(MV1[1][1]);
    y += DIV2RND(MV2[0][1]) + DIV2RND(MV2[1][1]);
  }
  else {
    x = MV1[0][0] + MV1[1][0] + MV2[0][0] + MV2[1][0];
    y = MV1[0][1] + MV1[1][1] + MV2[0][1] + MV2[1][1];
  }
  Out[0] = (x>>3) + Rnd_Tab_76[ x & 0xf ];
  Out[1] = (y>>3) + Rnd_Tab_76[ y & 0xf ];
                        
  Clip_Chroma(Out, Out);
}

#undef DIV2RND

//////////////////////////////////////////////////////////
// Half-pixel interpolation
//////////////////////////////////////////////////////////

inline
void SKL_MB::Clip(SKL_MV cMV, const SKL_MV MV) const {
  cMV[0] = (MV[0]<Limit_Mins[0] ) ? Limit_Mins[0] 
         : (MV[0]>Limit_Maxs[0] ) ? Limit_Maxs[0]
         :  MV[0];
  cMV[1] = (MV[1]<Limit_Mins[1] ) ? Limit_Mins[1] 
         : (MV[1]>Limit_Maxs[1] ) ? Limit_Maxs[1]
         :  MV[1];
}

inline
void SKL_MB::Clip_Chroma(SKL_MV cMV, const SKL_MV MV) const {
  cMV[0] = (MV[0]<Limit_Mins_UV[0] ) ? Limit_Mins_UV[0] 
         : (MV[0]>Limit_Maxs_UV[0] ) ? Limit_Maxs_UV[0]
         :  MV[0];
  cMV[1] = (MV[1]<Limit_Mins_UV[1] ) ? Limit_Mins_UV[1] 
         : (MV[1]>Limit_Maxs_UV[1] ) ? Limit_Maxs_UV[1]
         :  MV[1];
}

inline
void SKL_MB::Clip_Field(SKL_MV cMV, const SKL_MV MV) const {
  cMV[0] = (MV[0]<Limit_Mins[0] ) ? Limit_Mins[0] 
         : (MV[0]>Limit_Maxs[0] ) ? Limit_Maxs[0]
         :  MV[0];
  cMV[1] = (2*MV[1]<Limit_Mins[1] ) ? (Limit_Mins[1]>>1) 
         : (2*MV[1]>Limit_Maxs[1] ) ? (Limit_Maxs[1]>>1)
         :  MV[1];

}
inline
void SKL_MB::Clip_Chroma_Field(SKL_MV cMV, const SKL_MV MV) const {
  cMV[0] = (MV[0]<Limit_Mins_UV[0] ) ? Limit_Mins_UV[0] 
         : (MV[0]>Limit_Maxs_UV[0] ) ? Limit_Maxs_UV[0]
         :  MV[0];
  cMV[1] = (2*MV[1]<Limit_Mins_UV[1] ) ? (Limit_Mins_UV[1]>>1)
         : (2*MV[1]>Limit_Maxs_UV[1] ) ? (Limit_Maxs_UV[1]>>1)
         :  MV[1];
}

//////////////////////////////////////////////////////////

inline
void SKL_MB::Predict_8x8(SKL_BYTE * const Dst, const SKL_BYTE *Src,
                         const SKL_MV MV,
                         const SKL_MB_FUNCS * const Ops) const
{
  const int Halves = (MV[0]&1) | ((MV[1]&1)<<1);
  Src += (MV[1]>>1)*BpS + (MV[0]>>1);
  Ops->HP_8x8[Halves](Dst, Src, BpS);
}

inline
void SKL_MB::Predict_16x8(SKL_BYTE * const Dst, const SKL_BYTE *Src,
                          const SKL_MV MV,
                          const SKL_MB_FUNCS * const Ops) const
{
  const int Halves = (MV[0]&1) | ((MV[1]&1)<<1);
  Src += (MV[1]>>1)*BpS + (MV[0]>>1);
  Ops->HP_16x8[Halves](Dst, Src, BpS);
}

inline
void SKL_MB::Predict_16x8_Field(SKL_BYTE * const Dst, const SKL_BYTE *Src,
                                const SKL_MV MV,
                                const SKL_MB_FUNCS * const Ops) const
{
  const int Halves = (MV[0]&1) | ((MV[1]&1)<<1);
  Src += (MV[1]&~1)*BpS + (MV[0]>>1);
  Ops->HP_16x8[Halves](Dst, Src, 2*BpS);
}

inline
void SKL_MB::Predict_16x16(SKL_BYTE * const Dst, const SKL_BYTE *Src,
                           const SKL_MV MV,
                           const SKL_MB_FUNCS * const Ops) const
{
  const int Halves = (MV[0]&1) | ((MV[1]&1)<<1);
  Src += (MV[1]>>1)*BpS + (MV[0]>>1);
  Ops->HP_16x8[Halves](Dst,      Src,      BpS);
  Ops->HP_16x8[Halves](Dst+BpS8, Src+BpS8, BpS);
}

//////////////////////////////////////////////////////////

inline
void SKL_MB::Predict_With_0MV(const SKL_MB_FUNCS * const Ops, const int Fwd) const
{
  const int CoLoc = (Fwd ? Fwd_CoLoc : Bwd_CoLoc);
  Ops->HP_16x8[0](Y1,      Y1      + CoLoc, BpS);
  Ops->HP_16x8[0](Y1+BpS8, Y1+BpS8 + CoLoc, BpS);
  Ops->HP_8x8 [0](U,       U       + CoLoc, BpS);
  Ops->HP_8x8 [0](V,       V       + CoLoc, BpS);
}

inline
void SKL_MB::Predict_With_1MV(const SKL_MV MV, 
                              const SKL_MB_FUNCS * const Ops, 
                              const int Fwd) const
{
  const int CoLoc = (Fwd ? Fwd_CoLoc : Bwd_CoLoc);
  SKL_MV cMV;

  Clip(cMV, MV);
  if (!VOL->Quarter) Predict_16x16   (Y1, Y1+CoLoc, cMV, Ops);
  else               Predict_16x16_QP(Y1, Y1+CoLoc, cMV, Ops);

  Derive_uv_MV_From_1MV(cMV, cMV);    // we re-use the *clipped* Y-vector

  const int Halves = (cMV[0]&1) | ((cMV[1]&1)<<1);
  const int Off = CoLoc + (cMV[1]>>1)*BpS + (cMV[0]>>1);
  Ops->HP_8x8[Halves](U, U+Off, BpS);
  Ops->HP_8x8[Halves](V, V+Off, BpS);
}

inline
void SKL_MB::Predict_Fields(const SKL_MV MV[2],
                            const SKL_MB_FUNCS * const Ops,
                            const int Fwd, const int Fld_Dirs) const
{
  const int CoLoc = (Fwd ? Fwd_CoLoc : Bwd_CoLoc);
  int Off, Halves;
  SKL_MV uv_MV, cMV;

    // 1rst field
  Clip_Field(cMV, MV[0]);
  Off = CoLoc;
  if (Fld_Dirs&2) Off += BpS;
  if (!VOL->Quarter) Predict_16x8_Field   (Y1, Y1+Off, cMV, Ops);
  else               Predict_16x8_Field_QP(Y1, Y1+Off, cMV, Ops);

  Derive_uv_MVs_Fields(uv_MV, MV[0]);
  Halves = (uv_MV[0]&1) | ((uv_MV[1]&1)<<1);
  Off += (uv_MV[0]>>1) + (uv_MV[1]&~1)*BpS;
  Ops->HP_8x4[Halves](U, U+Off, 2*BpS);
  Ops->HP_8x4[Halves](V, V+Off, 2*BpS);

    // 2nd field
  Clip_Field(cMV, MV[1]);
  Off = CoLoc;
  if (Fld_Dirs&1) Off += BpS;
  if (VOL->Quarter) Predict_16x8_Field_QP(Y1+BpS, Y1+Off, cMV, Ops);
  else              Predict_16x8_Field   (Y1+BpS, Y1+Off, cMV, Ops);

  Derive_uv_MVs_Fields(uv_MV, MV[1]);
  Halves = (uv_MV[0]&1) | ((uv_MV[1]&1)<<1);
  Off += (uv_MV[0]>>1) + (uv_MV[1]&~1)*BpS;
  Ops->HP_8x4[Halves](U+BpS, U+Off, 2*BpS);
  Ops->HP_8x4[Halves](V+BpS, V+Off, 2*BpS);
}

inline
void SKL_MB::Predict_With_4MV(const SKL_MV MV1[2], 
                              const SKL_MV MV2[2], 
                              const SKL_MB_FUNCS * const Ops,
                              const int Fwd) const
{
  const int CoLoc = ( Fwd ? Fwd_CoLoc : Bwd_CoLoc );

  SKL_MV cMV[2];
  if (!VOL->Quarter) {
    Clip(cMV[0], MV1[0]);
    Clip(cMV[1], MV1[1]);
    SKL_BYTE * Y = Y1;
    if (SKL_IS_SAME_MV(cMV[0], cMV[1]))
      Predict_16x8(Y, Y+CoLoc, cMV[0], Ops);
    else {
      Predict_8x8 (Y,  Y  +CoLoc, cMV[0], Ops);
      Predict_8x8 (Y+8,Y+8+CoLoc, cMV[1], Ops);
    }

    Y += BpS8;
    Clip(cMV[0], MV2[0]);
    Clip(cMV[1], MV2[1]);
    if (SKL_IS_SAME_MV(cMV[0], cMV[1]))
      Predict_16x8(Y,  Y+CoLoc, cMV[0], Ops);
    else {
      Predict_8x8 (Y,  Y  +CoLoc, cMV[0], Ops);
      Predict_8x8 (Y+8,Y+8+CoLoc, cMV[1], Ops);
    }
  }
  else {
    SKL_MV cMV[2];
    Clip(cMV[0], MV1[0]);
    Clip(cMV[1], MV1[1]);
    SKL_BYTE * Y = Y1;
    Predict_8x8_QP(Y,   Y+  CoLoc, cMV[0], Ops);
    Predict_8x8_QP(Y+8, Y+8+CoLoc, cMV[1], Ops);
    Y += BpS8;
    Clip(cMV[0], MV2[0]);
    Clip(cMV[1], MV2[1]);
    Predict_8x8_QP(Y,   Y  +CoLoc, cMV[0], Ops);
    Predict_8x8_QP(Y+8, Y+8+CoLoc, cMV[1], Ops);
  }

  SKL_MV uv_MV;
  Derive_uv_MV_From_4MV(uv_MV, MV1, MV2);

  const int Halves = (uv_MV[0]&1) | ((uv_MV[1]&1)<<1);
  const int Off = CoLoc + (uv_MV[1]>>1)*BpS + (uv_MV[0]>>1);
  Ops->HP_8x8[Halves](U, U+Off, BpS);
  Ops->HP_8x8[Halves](V, V+Off, BpS);
}

//////////////////////////////////////////////////////////
// GMC prediction

inline void SKL_MB::Predict_GMC() const
{
  const SKL_BYTE * const ySrc  = VOL->Cur->Y + Fwd_CoLoc;
  const SKL_BYTE * const uSrc  = VOL->Cur->U + Fwd_CoLoc;
  const SKL_SAFE_INT UV_Offset = VOL->Cur->V - VOL->Cur->U;

  SKL_BYTE *yDst = Y1;
  SKL_BYTE *uDst = U;

  VOL->GMC_Ops.Predict_16x16(&VOL->GMC_Ops, yDst, ySrc,            BpS, x,y, VOL->Rounding);
  VOL->GMC_Ops.Predict_8x8  (&VOL->GMC_Ops, uDst, uSrc, UV_Offset, BpS, x,y, VOL->Rounding);

  int mv[2];
  VOL->GMC_Ops.Get_Average_MV(mv, x, y, VOL->Quarter);
  const int High = 1 << (VOL->Fwd_Code+4);
  if      (mv[0]<-High) MVs[0][0] = -High;
  else if (mv[0]>=High) MVs[0][0] =  High-1;
  else                  MVs[0][0] =  mv[0];
  if      (mv[1]<-High) MVs[0][1] = -High;
  else if (mv[1]>=High) MVs[0][1] =  High-1;
  else                  MVs[0][1] =  mv[1];

  Store_16x16_MV();
}

//////////////////////////////////////////////////////////
// 16b->8b MB Prediction ops
//////////////////////////////////////////////////////////

inline void SKL_MB::Set_Field_DCT(int fDCT)
{
  Field_DCT = fDCT;
  if (fDCT>0) {
    Y2 = Y1 + BpS;
    YBpS = 2*BpS;
  }
  else { 
    Y2 = Y1 + BpS8;
    YBpS = BpS;
  }
}

inline void SKL_MB::Set_No_Field()
{
  if (Field_DCT>0) Field_DCT = 0;
  Field_Pred = 0;
}

inline
void SKL_MB::Copy_16To8(SKL_INT16 In[6*64]) const
{
  VOL->Quant_Ops.IDct_Put(In+0*64, Y1  ,YBpS);
  VOL->Quant_Ops.IDct_Put(In+1*64, Y1+8,YBpS);
  VOL->Quant_Ops.IDct_Put(In+2*64, Y2  ,YBpS);
  VOL->Quant_Ops.IDct_Put(In+3*64, Y2+8,YBpS);
  VOL->Quant_Ops.IDct_Put(In+4*64, U,    BpS);
  VOL->Quant_Ops.IDct_Put(In+5*64, V,    BpS);
}

inline
void SKL_MB::Add_16To8(SKL_INT16 In[6*64]) const
{
  if (Cbp&0x20) VOL->Quant_Ops.IDct_Add(In+0*64, Y1  ,YBpS);
  if (Cbp&0x10) VOL->Quant_Ops.IDct_Add(In+1*64, Y1+8,YBpS);
  if (Cbp&0x08) VOL->Quant_Ops.IDct_Add(In+2*64, Y2  ,YBpS);
  if (Cbp&0x04) VOL->Quant_Ops.IDct_Add(In+3*64, Y2+8,YBpS);
  if (Cbp&0x02) VOL->Quant_Ops.IDct_Add(In+4*64, U,    BpS);
  if (Cbp&0x01) VOL->Quant_Ops.IDct_Add(In+5*64, V,    BpS);
}

//////////////////////////////////////////////////////////
// SKL_MB : MV prediction
//////////////////////////////////////////////////////////

inline void SKL_MB::Store_Zero_MV() const {
  SKL_ZERO_MV(MVs[0]);
  SKL_ZERO_MV(MVs[1]);
  SKL_ZERO_MV(MVs[MV_Stride+0]);
  SKL_ZERO_MV(MVs[MV_Stride+1]);
}

inline void SKL_MB::Store_16x16_MV() const {
  SKL_COPY_MV(MVs[1]          , MVs[0]);
  SKL_COPY_MV(MVs[MV_Stride+0], MVs[0]);
  SKL_COPY_MV(MVs[MV_Stride+1], MVs[0]);
}

  // all Section 7.7.2 boils down to averaging field MVs after 
  // they've been used to form block prediction.
  // However, vertical component for field is always even,
  // and we internally don't store it multiplied by 2
  // => we don't need the DIV2RND for y-comp

#define DIV2RND(x) ( ((x)>>1) | ((x)&1) )
inline void SKL_MB::Store_16x8_MV(SKL_MV *Dst, SKL_MV MV[2]) const
{
  const int Tmp = MV[0][0] + MV[1][0];
  Dst[0][0] = DIV2RND(Tmp);
  Dst[0][1] = MV[0][1] + MV[1][1];  // no DIV2RND here
  SKL_COPY_MV(Dst[1], Dst[0]);
}
#undef DIV2RND

//////////////////////////////////////////////////////////
// Quarter-pixel interpolation
//////////////////////////////////////////////////////////
//
// case 0 (pt a):       copy
// case 2 (pt b):                h-pass
// case 1/3 (pts e/f):           h-pass + h-avrg
// case 8 (pt c):                                  v-pass
// case 10 (pt d):               h-pass          + v-pass
// case 9/11 (pts k/l):          h-pass + h-avrg + v-pass
// case 4/12 (pts g/m):                            v-pass + v-avrg
// case 6/14 (pts i/o):          h-pass          + v-pass + v-avrg
// case 5/13/7/15 (pts h/n/j/p): h-pass + h-avrg + v-pass + v-avrg
//
//////////////////////////////////////////////////////////

inline
void SKL_MB::Predict_16x16_QP(SKL_BYTE * const Dst, const SKL_BYTE *Src,
                              const SKL_MV MV,
                              const SKL_MB_FUNCS * const Ops) const
{
  const int Quads = (MV[0]&3) | ((MV[1]&3)<<2);
  Src += (MV[1]>>2)*BpS + (MV[0]>>2);

  switch(Quads) {
    default:
    case 0:  Ops->HP_16x8[0](Dst, Src, BpS);
             Ops->HP_16x8[0](Dst+BpS8, Src+BpS8, BpS); break;
    case 1:  Ops->H_Pass_Avrg(Dst, Src, 16, BpS);      break;
    case 2:  Ops->H_Pass(Dst, Src, 16, BpS);           break;
    case 3:  Ops->H_Pass_Avrg_Up(Dst, Src, 16, BpS);   break;

    case 4:  Ops->V_Pass_Avrg(Dst, Src, 16, BpS);      break;
    case 5:  Ops->H_LowPass_Avrg(YTmp, Src, 17, BpS);
             Ops->V_Pass_Avrg(Dst, YTmp, 16, BpS);     break;
    case 6:  Ops->H_LowPass(YTmp, Src,   17, BpS);
             Ops->V_Pass_Avrg(Dst, YTmp, 16, BpS);     break;
    case 7:  Ops->H_LowPass_Avrg_Up(YTmp, Src, 17, BpS);
             Ops->V_Pass_Avrg(Dst, YTmp, 16, BpS);     break;

    case 8:  Ops->V_Pass(Dst, Src, 16, BpS);           break;
    case 9:  Ops->H_LowPass_Avrg(YTmp, Src, 17, BpS);
             Ops->V_Pass(Dst, YTmp, 16, BpS);          break;
    case 10: Ops->H_LowPass(YTmp, Src, 17, BpS);
             Ops->V_Pass(Dst, YTmp, 16, BpS);          break;
    case 11: Ops->H_LowPass_Avrg_Up(YTmp, Src, 17, BpS);
             Ops->V_Pass(Dst, YTmp, 16, BpS);          break;

    case 12: Ops->V_Pass_Avrg_Up(Dst, Src, 16, BpS);   break;
    case 13: Ops->H_LowPass_Avrg(YTmp, Src, 17, BpS);
             Ops->V_Pass_Avrg_Up(Dst, YTmp, 16, BpS);  break;
    case 14: Ops->H_LowPass(YTmp, Src, 17, BpS);
             Ops->V_Pass_Avrg_Up( Dst, YTmp, 16, BpS); break;
    case 15: Ops->H_LowPass_Avrg_Up(YTmp, Src, 17, BpS);
             Ops->V_Pass_Avrg_Up(Dst, YTmp, 16, BpS);  break;
  }
}

inline
void SKL_MB::Predict_16x8_Field_QP(SKL_BYTE * const Dst, const SKL_BYTE *Src,
                                   const SKL_MV MV,
                                   const SKL_MB_FUNCS * const Ops) const
{
  const int Quads = (MV[0]&3) | ((MV[1]&3)<<2);
  Src += ((MV[1]>>1)&~1)*BpS + (MV[0]>>2);

  switch(Quads) {
    default:
    case 0: Ops->HP_16x8[0](Dst, Src, 2*BpS);               break;
    case 1: Ops->H_Pass_Avrg(Dst, Src, 8, 2*BpS);           break;
    case 2: Ops->H_Pass(Dst, Src, 8, 2*BpS);                break;
    case 3: Ops->H_Pass_Avrg_Up(Dst, Src, 8, 2*BpS);        break;

    case 4: Ops->V_Pass_Avrg_8(Dst, Src, 16, 2*BpS);        break;
    case 5: Ops->H_Pass_Avrg(YTmp, Src, 9, 2*BpS);
            Ops->V_Pass_Avrg_8(Dst, YTmp, 16, 2*BpS);       break;
    case 6: Ops->H_Pass(YTmp, Src,   9, 2*BpS);
            Ops->V_Pass_Avrg_8(Dst, YTmp, 16, 2*BpS);       break;
    case 7: Ops->H_Pass_Avrg_Up(YTmp, Src, 9, 2*BpS);
            Ops->V_Pass_Avrg_8(Dst, YTmp, 16, 2*BpS);       break;

    case 8: Ops->V_Pass_8(Dst, Src, 16, 2*BpS);             break;
    case 9: Ops->H_Pass_Avrg(YTmp, Src, 9, 2*BpS);
            Ops->V_Pass_8(Dst, YTmp, 16, 2*BpS);            break;
    case 10: Ops->H_Pass(YTmp, Src, 9, 2*BpS);
             Ops->V_Pass_8(Dst, YTmp, 16, 2*BpS);           break;
    case 11: Ops->H_Pass_Avrg_Up(YTmp, Src, 9, 2*BpS);
             Ops->V_Pass_8(Dst, YTmp, 16, 2*BpS);           break;

    case 12: Ops->V_Pass_Avrg_Up_8(Dst, Src, 16, 2*BpS);    break;
    case 13: Ops->H_Pass_Avrg(YTmp, Src, 9, 2*BpS);
             Ops->V_Pass_Avrg_Up_8(Dst, YTmp, 16, 2*BpS);   break;
    case 14: Ops->H_Pass(YTmp, Src, 9, 2*BpS);
             Ops->V_Pass_Avrg_Up_8( Dst, YTmp, 16, 2*BpS);  break;
    case 15: Ops->H_Pass_Avrg_Up(YTmp, Src, 9, 2*BpS);
             Ops->V_Pass_Avrg_Up_8(Dst, YTmp, 16, 2*BpS);   break;
  }

}

inline
void SKL_MB::Predict_8x8_QP(SKL_BYTE * const Dst, const SKL_BYTE *Src,
                            const SKL_MV MV,
                            const SKL_MB_FUNCS * const Ops) const
{
  const int Quads = (MV[0]&3) | ((MV[1]&3)<<2);
  Src += (MV[1]>>2)*BpS + (MV[0]>>2);

  switch(Quads) {
    default:
    case 0:  Ops->HP_8x8[0](Dst, Src, BpS);             break;
    case 1:  Ops->H_Pass_Avrg_8(Dst, Src, 8, BpS);      break;
    case 2:  Ops->H_Pass_8(Dst, Src, 8, BpS);           break;
    case 3:  Ops->H_Pass_Avrg_Up_8(Dst, Src, 8, BpS);   break;

    case 4:  Ops->V_Pass_Avrg_8(Dst, Src, 8, BpS);      break;
    case 5:  Ops->H_LowPass_Avrg_8(YTmp, Src, 9, BpS);   
             Ops->V_Pass_Avrg_8(Dst, YTmp, 8, BpS);     break;
    case 6:  Ops->H_LowPass_8(YTmp, Src, 9, BpS);
             Ops->V_Pass_Avrg_8(Dst, YTmp, 8, BpS);     break;
    case 7:  Ops->H_LowPass_Avrg_Up_8(YTmp, Src, 9, BpS);
             Ops->V_Pass_Avrg_8(Dst, YTmp, 8, BpS);     break;

    case 8:  Ops->V_Pass_8(Dst, Src, 8, BpS);           break;
    case 9:  Ops->H_LowPass_Avrg_8(YTmp, Src, 9, BpS);
             Ops->V_Pass_8(Dst, YTmp, 8, BpS);          break;
    case 10: Ops->H_LowPass_8(YTmp, Src, 9, BpS);
             Ops->V_Pass_8(Dst, YTmp, 8, BpS);          break;
    case 11: Ops->H_LowPass_Avrg_Up_8(YTmp, Src, 9, BpS);
             Ops->V_Pass_8(Dst, YTmp, 8, BpS);          break;

    case 12: Ops->V_Pass_Avrg_Up_8(Dst, Src, 8, BpS);   break;
    case 13: Ops->H_LowPass_Avrg_8(YTmp, Src, 9, BpS);
             Ops->V_Pass_Avrg_Up_8(Dst, YTmp, 8, BpS);  break;
    case 14: Ops->H_LowPass_8(YTmp, Src, 9, BpS);
             Ops->V_Pass_Avrg_Up_8( Dst, YTmp, 8, BpS); break;
    case 15: Ops->H_LowPass_Avrg_Up_8(YTmp, Src, 9, BpS);
             Ops->V_Pass_Avrg_Up_8(Dst, YTmp, 8, BpS);  break;
  }
}

//////////////////////////////////////////////////////////
//
// encoder specific. 
//
//////////////////////////////////////////////////////////

typedef void (*SKL_DECIMATE_INTRA_FUNC)(const SKL_QUANT_DSP *This,
                                        SKL_INT16 *Out, SKL_INT16 *In,
                                        const SKL_QUANTIZER M,
                                        SKL_INT32 Scale, SKL_INT32 DC_Scale);
typedef int (*SKL_DECIMATE_INTER_FUNC)(const SKL_QUANT_DSP *This,
                                       SKL_INT16 *Out, SKL_INT16 *In,
                                       const SKL_QUANTIZER M,
                                       SKL_INT32 Scale,
                                       int PSNR_Limit);
  /** @internal
      it's a cursor (only 1 instance) 
    */
struct SKL_MB_ENC : public SKL_MB   
{
  protected:
    static const int Map_To_Type[SKL_MAP_LAST];

    void Substract_Prediction();
    void Substract_Field_Prediction();

    int Select_DC_AC_Pred(SKL_INT16 In[6*64], int Pred_Dirs[6]);

    int Texture_Bits;
    int MV_Bits;

  public:
    SKL_MB_ENC(const SKL_MP4_I * const VOL) 
      : SKL_MB(VOL)
      , Texture_Bits(0)
      , MV_Bits(0)
    {}

    int B_Type;
    int dQuant;
    SKL_MV dMVs[4];     /**< de-predicted MVs */
    int Last[6];        /**< last non-zero DCT coeff for INTER mb only */


    void Set_Type() {
      const int Map_Type = Map[Pos].Type;
      MB_Type = Map_To_Type[ Map_Type ];
      MC_Sel  = (Map_Type==SKL_MAP_GMC);
      if (MB_Type!=SKL_MB_SKIPPED) Set_Final_Params();
    }
    void Set_Final_Params();

    void Decimate_Intra(SKL_INT16 Out[12*64]) const;
    void Decimate_Inter(SKL_INT16 Out[12*64]);
    void Decimate_Inter_GMC(SKL_INT16 Out[12*64]);
    void Decimate_Reduced_Intra(SKL_INT16 Out[12*64]) const;
    void Decimate_Reduced_Inter(SKL_INT16 Out[12*64]);

    void Encode_Intra(SKL_FBB * const Bits, SKL_INT16 In[12*64], int Is_I_VOP);
    void Encode_Inter(SKL_FBB * const Bits, SKL_INT16 In[12*64], int Fwd_Code);
    void Encode_Inter_B(SKL_FBB * const Bits, SKL_INT16 In[12*64],
                        int Fwd_Code, int Bwd_Code);

    void Copy_8To16(SKL_INT16 Out[6*64]) const;
    void Diff_8To16(SKL_INT16 Out[6*64]) const;

    void Copy_8To16_Downsampled(SKL_INT16 Out[6*64]) const;
    void Diff_8To16_Downsampled(SKL_INT16 Out[6*64]) const;

    int Get_Texture_Bits() const { return Texture_Bits; }
    int Get_MV_Bits()      const { return MV_Bits; }
};

  // Warning: Slicing problem. Inherit from SKL_MP4_I first
  // so that cast to SKL_MP4_I* is ok.

class SKL_MP4_ENC_I : public SKL_MP4_I, private SKL_MP4_ENC
{
  private:
      // bits I/O buffer
    size_t          _Buf_Size;
    SKL_BYTE       *_Buf;
    size_t          _Buf_Len;
    void Check_Buf_Size(size_t Needed_Size);  
    void Clear_Buf();

    SKL_MP4_PIC    *_In_Pic;

  private:
    int _Need_VOL_Header;
    int _Key_VOL_Headers;
    int _Emit_SEQ_Codes;

    int _Inter_Coding_Threshold;
    int _Use_Trellis;
    SKL_UINT32 Evaluate_Cost(const SKL_INT16 *C, const int * const Zigzag, int Max, int Lambda) const;
    int Trellis_Quantize(SKL_INT16 * const Out, const int Q, const int * const Zigzag, int Non_Zero) const;

    SKL_MP4_ANALYZER *_Analyzer;
    SKL_MP4_ANALYZER *_Dflt_Analyzer; // use default impl.


    void Write_I_VOP(SKL_FBB * const Bits);
    void Write_P_VOP(SKL_FBB * const Bits);
    void Write_B_VOP(SKL_FBB * const Bits);
    void Write_S_VOP(SKL_FBB * const Bits);

    void Write_Reduced_I_VOP(SKL_FBB * const Bits);
    void Write_Reduced_P_VOP(SKL_FBB * const Bits);

    void Write_VOL_Header(SKL_FBB * const Bits) const;
    void Write_VOP_Header(SKL_FBB * const Bits, const SKL_MP4_FRAME *VOP) const;  
    void Code_Frame(SKL_BYTE * const Buf, int Max_Len, const SKL_MP4_FRAME * const Frame);


    void Alloc_Aux();
    void Clear_Aux();

      /* Input analysis and frame coding preparation */
    void Setup_VOL_Params();   // init VOL-dependant param: qpel, ...
    void Setup_Frame_Params(SKL_MP4_FRAME *Frame); // init frame-specific param: Quant, FCode...

  public:
    SKL_MP4_ENC_I(SKL_MEM_I *Mem);
    ~SKL_MP4_ENC_I();

    void Set_Trellis_Usage(const int Just_Do_It);
    ;
    int Decimate_Inter(SKL_INT16 * const Out, SKL_INT16 * const In, const SKL_INT32 Q) const;
    void Decimate_Intra(SKL_INT16 * const Out, SKL_INT16 * const In, SKL_INT32 Q, SKL_INT32 DC_Q) const;

    SKL_MP4_PIC *Prepare_For_Input(int Width, int Height);

    int Encode(SKL_BYTE *Buf, int Max_Len);

  public:
      // Top class' implementation.
    virtual const SKL_MP4_PIC *Prepare_Next_Frame(int Width, int Height);
    virtual const SKL_MP4_PIC *Get_Next_Frame() const;
    virtual const SKL_MP4_PIC *Get_Last_Coded_Frame() const;
    virtual int Encode();
    virtual int Finish_Encoding();
    virtual const SKL_BYTE *Get_Bits() const;
    virtual int Get_Bits_Length() const;
    virtual SKL_MEM_I *Set_Memory_Manager(SKL_MEM_I *Mem=0);
    virtual void Set_CPU(SKL_CPU_FEATURE Cpu = SKL_CPU_DETECT);
    virtual void Set_Custom_Matrix(int Intra, const SKL_BYTE *M=0);
    virtual SKL_MP4_ANALYZER *Set_Analyzer(SKL_MP4_ANALYZER *Analyzer=0);
    virtual SKL_MP4_ANALYZER *Get_Analyzer() const;
    virtual void Set_Slicer(SKL_MP4_SLICER Slicer, SKL_ANY Slicer_Data=0);
    virtual void Get_All_Frames(SKL_MP4_PIC *Pic) const;
    virtual void Set_Debug_Level(int Level=0);
    virtual int Ioctl(SKL_CST_STRING Param);
};

//////////////////////////////////////////////////////////

#endif  /* _SKL_MPG4I_H_ */
