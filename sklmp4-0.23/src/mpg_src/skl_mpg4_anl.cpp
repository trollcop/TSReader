/********************************************************
 * Some code. Copyright (C) 2003 by Pascal Massimino.   *
 * All Rights Reserved.      (http://skal.planet-d.net) *
 * For Educational/Academic use ONLY. See 'LICENSE.TXT'.*
 ********************************************************/
/*
 * skl_mpg4_anl.cpp
 *
 *  Default Built-In analyzer for MPEG4 encoder
 ********************************************************/

#ifndef SKL_AUTO_INCLUDE

#include "./skl_mpg4_anl.h"

#include <malloc.h>   // for alloca()

#if defined(__SUN__) || defined(__ALPHA__) || defined(__IRIX__) || defined(__HP__)
#include <alloca.h>   // for alloca() on Sun, Irix, HP, and Alpha
#endif

const int CORE_VERSION = 5;

//////////////////////////////////////////////////////////
// alignment macros

#ifdef SKL_USE_ASM
#define SKL_ALIGN 15
#else
#define SKL_ALIGN 0
#endif

#define SKL_ALIGN_PTR(P,ALGN) ( ((SKL_SAFE_INT)(P) + (ALGN)) & ~(ALGN) )

//////////////////////////////////////////////////////////

#define ABS(x)  (((x)<0) ? -(x) : (x))

//#define BCOST(x)  ABS(x)
//#define BCOST(x)  ( SKL_BMASKS::Log2(ABS(x)) )
#define BCOST(x)  (Code_Sizes[(x)])

#define RND(Q) ((int)((Q)+.5))

static inline int IS_VALID_MV(const SKL_MV MV) { return *(SKL_UINT32*)MV != 0x7fff7fff; }
static inline int IS_EQUAL_MV(const SKL_MV a, const SKL_MV b) { return *(SKL_UINT32*)a == *(SKL_UINT32*)b; }
static inline int IS_ZERO_MV(const SKL_MV MV) { return *(SKL_UINT32*)MV == 0; }

//////////////////////////////////////////////////////////
// Cursor to hold search parameters and results
//////////////////////////////////////////////////////////

struct ME_MAP;

  // note: for SSE2, *Src1 is supposed to be 16b-aligned!!
typedef SKL_UINT32 (*METRIC)(const SKL_BYTE *Src1, const SKL_BYTE *Src2, SKL_INT32 BpS);
typedef SKL_UINT32 (*METRIC_DEV)(const SKL_BYTE *Src, SKL_INT32 BpS);
typedef SKL_UINT32 (*METRIC_AVRG)(const SKL_BYTE *Dst, const SKL_BYTE *Src1, const SKL_BYTE *Src2, SKL_INT32 BpS);

typedef int (*SEARCH_MTHD)(ME_MAP * const Cursor);

struct ME_MAP
{
  SKL_MV Pred, IPred, Best_MV;
  SKL_UINT32 Best_Sad;
  const SKL_BYTE *Src;  // current frame pointer
  const SKL_BYTE *Ref;  // ref frame, relative to Predictor vector
  int Lambda;
  int MV_Bits_Shift;
  int Sub_Prec_Shift;
  METRIC Metric;
  METRIC_AVRG Metric_Avrg;
  int Dir;              // bits 0-7: last direction checked, bits 8-15: new direction to check

  static const SKL_BYTE MV_Bits0[32];
  static SKL_BYTE MV_Code_Sizes0[7][2*2048];
  static void Init_MV_Code_Sizes();
  void Set_FCode(const int FCode, const int Prec);
  SKL_BYTE * Code_Sizes;

  SEARCH_MTHD Search_Method;

  int xo, yo;
  int BpS;
  int xm, xM, ym, yM;                 // MV Limits in full-pel units (disregarding sub-pel units)
  int xm_Sub, xM_Sub, ym_Sub, yM_Sub; // MV Limits in sub-pel units (either 1 or 2)
  int Range;            // range in *full* pel

  // int Hit, Miss;   // debug

  const SKL_BYTE *Src0, *Ref0;
  const SKL_MP4_INFOS * const Frame;
  SKL_BYTE *Scratch1;   // 16x17 tmp buffer
  SKL_BYTE *Scratch2;   // 17x16 tmp buffer

  enum { MAX_KEY = 2048, CACHE_SIZE = 8 };      // CACHE_SIZE must be a power of 2
  SKL_UINT32 Key_Map[CACHE_SIZE*CACHE_SIZE];  // keys 
  SKL_UINT32 Sad_Map[CACHE_SIZE*CACHE_SIZE];  // cache for SAD evaluated so far
  SKL_UINT32 Min_Key;                         // offset to avoid erasing Map too often

  inline void Set_MV_Predictor(const SKL_MV * const Src, const int MV_Stride, const int Blk, const int ABits);
  inline static void Set_FP_MV_Predictor(SKL_MV Pred, const SKL_MP4_MAP * const Map, const int Map_Stride, const int ABits);

  inline SKL_UINT32 Hash(int x, int y) const { return Min_Key + x + y*MAX_KEY + 1025*2048; }
  inline int Slot(int x, int y)        const { return (x+y*CACHE_SIZE) & (CACHE_SIZE*CACHE_SIZE-1); }
  inline void New_Map() {
    if (Min_Key==0) SKL_BZERO(Key_Map, sizeof(Key_Map));
    Min_Key += MAX_KEY*MAX_KEY;
  }

  SKL_UINT32 MV_Bits_Cost(const int x, const int y) const { return Lambda*(BCOST((x<<Sub_Prec_Shift)-Pred[0])+BCOST((y<<Sub_Prec_Shift)-Pred[1])); }
//  SKL_UINT32 MV_Bits_Cost(const int x, const int y) const { return Lambda*(BCOST(x-IPred[0])+BCOST(y-IPred[1])); }
  SKL_UINT32 MV_Bits_Cost_Raw(const int x, const int y) const { return Lambda*(BCOST(x-Pred[0])+BCOST(y-Pred[1])); }
  SKL_UINT32 Eval(int x, int y) const {
    const SKL_UINT32 Sad = Metric(Src, Ref+x+y*BpS, BpS);
    return Sad + MV_Bits_Cost(x, y);
  }

  inline int Sad_Is_Tested(int x, int y) const {
    const SKL_UINT32 Key = Hash(x,y);
    const int Off = Slot(x,y);
    // if (Key_Map[Off]==Key) Hit++; else Miss++;
    return (Key_Map[Off]==Key);  // cache hit
  }
  inline SKL_UINT32 *Get_Cached_Sad(int x, int y) {
    const int Off = Slot(x,y);
    SKL_ASSERT(Key_Map[Off] == Hash(x,y));
    return &Sad_Map[Off];
  }
  inline void Cache_Sad(int x, int y, SKL_UINT32 Sad) {
    const int Off = Slot(x,y);
    Key_Map[Off] = Hash(x,y);
    Sad_Map[Off] = Sad;
    Best_Sad = Sad;
    Best_MV[0] = x;
    Best_MV[1] = y;
  }

  inline void Search() { Search_Method(this); }
  inline SKL_UINT32 Sub_Search(const SKL_MV MVo, const int dx, const int dy) {
    New_Map();
    Set_Sub_Offset(dx, dy);
    const int x = (IPred[0]<xm) ? xm : (IPred[0]>xM) ? xM : IPred[0];
    const int y = (IPred[1]<ym) ? ym : (IPred[1]>yM) ? yM : IPred[1];
    Cache_Sad( x,y, Metric(Src, Ref+x+y*BpS, BpS) );
    Eval_FP_If_Valid(MVo);
    Search();
    return Best_Sad;
  }

  void Print_Cache(int What) const {
    int x,y;
    printf("---- (Best SAD:%d at (%d,%d)---------------\n", Best_Sad, Best_MV[0], Best_MV[1]);
    for(y=0; y<CACHE_SIZE; ++y) {
      for(x=0; x<CACHE_SIZE; ++x) {
        SKL_UINT32 v = Key_Map[CACHE_SIZE*y+x];
        int Ok = (v<Min_Key + MAX_KEY*MAX_KEY) && (v>=Min_Key);
        if      (What==0) printf( "%c",  Ok ? '*' : '.');
        else if (What==1) printf( "[%6d]", Ok ? Sad_Map[CACHE_SIZE*y+x] : 0);
        else              printf( " 0x%8x ", Key_Map[CACHE_SIZE*y+x]);
      }
      printf("\n");
    }
  }

  void Start_Search() {
    Dir = 0x00; // (Best_MV[0]>0 ? 0x02 : Best_MV[0]<0 ? 0x01 : 0x00) | 
                // (Best_MV[1]>0 ? 0x04 : Best_MV[1]<0 ? 0x08 : 0x00);
  }

  void Eval_Dir(int x, int y, int Next_Dir)
  {
    if (Sad_Is_Tested(x,y)) return;
    SKL_UINT32 Sad = Eval(x,y);
    if (Sad<Best_Sad) {
      Cache_Sad(x,y,Sad);
      Dir = (Dir & 0xff) | Next_Dir; 
    }
  }

  int Eval(const SKL_MV v) {
    const int x = v[0], y = v[1];
    if (Sad_Is_Tested(x,y)) return 0;
    const SKL_UINT32 Sad = Eval(x,y);
    if (Sad<Best_Sad) {
      Cache_Sad(x,y,Sad);
      return 1;
    }
    else return 0;
  }

  inline int Is_In_Range(const SKL_MV v) { 
    return (v[0]>=xm_Sub && v[0]<=xM_Sub && 
            v[1]>=ym_Sub && v[1]<=yM_Sub);
  }

  inline int Eval_If_In_Range(const SKL_MV v) {
    if (v[0]<xm || v[0]>xM || v[1]<ym || v[1]>yM) return 0;
    return Eval(v);
  }
  int Eval_If_Valid(const SKL_MV v) {
    if (!IS_VALID_MV(v)) return 0;
    if (!Is_In_Range(v)) return 0;
    SKL_MV V;
    if (Sub_Prec_Shift==2) { V[0] = v[0]/4; V[1] = v[1]/4; }
    else                   { V[0] = v[0]/2; V[1] = v[1]/2; }
    // V[0] = v[0]>>Sub_Prec_Shift; V[1] = v[1]>>Sub_Prec_Shift;
    return Eval(V);
  }
  int Eval_FP_If_Valid(const SKL_MV v) {
    if (!IS_VALID_MV(v)) return 0;
    if (v[0]<xm || v[0]>xM || v[1]<ym || v[1]>yM) return 0;
    return Eval(v);
  }

  SKL_UINT32 Get_Sad(int x, int y) {
    SKL_ASSERT(x>=xm && x<=xM && y>=ym && y<=yM);
    const SKL_UINT32 Key = Hash(x,y);
    const int Off = Slot(x,y);
    if (Key_Map[Off]==Key) return Sad_Map[Off];
    SKL_UINT32 Sad = Eval(x,y);
    Sad_Map[Off] = Sad;
    Key_Map[Off] = Key;
    return Sad;
  }



  ME_MAP(const SKL_MP4_INFOS * const frame)
    : BpS(frame->BpS)
    , Frame(frame)
    , Min_Key(0) { Init(); }

  void Init();
  SKL_UINT32 Start(const METRIC metric, const METRIC_AVRG metric_Avrg);   // returns Metric(0,0)
  void New_Scanline(int y);
  inline void operator++(int) {
    xo  += 16;
    Src += 16;
    Ref += 16;
  }
  inline void Set_Sub_Offset(const int dx, const int dy);

  SKL_UINT32 Refine_MV_16x16(SKL_MV MV, const int Prec, const int Rounding);
  SKL_UINT32 Refine_MV_16x8 (SKL_MV MV, const int Prec, const int Rounding);
  SKL_UINT32 Refine_MV_8x8  (SKL_MV MV, const int Prec, const int Rounding);

  SKL_UINT32 Sad_Cost_QP(const SKL_BYTE * const *_HPels, int Offset, const int x, const int y) const;
                                                              
  SKL_UINT32 Refine_MV_HPels(SKL_MV MV, const int Prec, const int Rounding,
                             const SKL_BYTE * const *HPels);

  SKL_UINT32 Get_HP_Sad(SKL_BYTE *Tmp,
                        const SKL_MB_FUNCS * const mb, const SKL_BYTE *Rf,
                        int dx, int dy) const;
  SKL_UINT32 Get_HP_Sad_8x8(SKL_BYTE *Tmp,
                            const SKL_MB_FUNCS * const mb, const SKL_BYTE *Rf,
                            int dx, int dy) const;
  int Check_Limits(const SKL_MV mv, SKL_CST_STRING Label=0) const;
  int Check_Frame_Limits(const SKL_MV mv, SKL_CST_STRING Label=0) const;
};

//////////////////////////////////////////////////////////

const SKL_BYTE ME_MAP::MV_Bits0[32] = {    // Table B-12 (with sign bit counted)
   3,  4,  5,  7,  8,  8,  8, 10
, 10, 10, 11, 11, 11, 11, 11, 11
, 11, 11, 11, 11, 11, 11, 11, 11
, 12, 12, 12, 12, 12, 12, 13, 13
};

SKL_BYTE ME_MAP::MV_Code_Sizes0[7][2*2048];

void ME_MAP::Init_MV_Code_Sizes()
{
  static int MV_Code_Sizes_Ok = 0;    // not fully MT-safe, but yet ok.
  if (MV_Code_Sizes_Ok)
    return;

  for(int FCode=1; FCode<=7; FCode++)
  {
    int MV;
    const int Fix = FCode - 1;
    MV_Code_Sizes0[Fix][2048+0] = 1;
    for(MV=1; MV<=(32<<Fix); ++MV)
    {
      const int Code = (MV-1) >> Fix;
      const int Len = MV_Bits0[Code] + Fix;
      if (MV<(32<<Fix)) MV_Code_Sizes0[Fix][2048+MV] = Len;
      MV_Code_Sizes0[Fix][2048-MV] = Len;
    }
    for( ; MV<=2048; ++MV) {    // sanity check
      if (MV<2048) MV_Code_Sizes0[Fix][2048+MV] = 255;
      MV_Code_Sizes0[Fix][2048-MV] = 255;
    }
  }
  MV_Code_Sizes_Ok = 1;
}

void ME_MAP::Set_FCode(const int FCode, const int Prec)
{
  SKL_ASSERT(FCode>=1 && FCode<=7);
  Init_MV_Code_Sizes();
  Sub_Prec_Shift = 1 + (Prec==2);
  MV_Bits_Shift  = (FCode-1) + 4;
  Code_Sizes     = MV_Code_Sizes0[FCode-1] + 2048;
  Range          = ( 16 << FCode ) >> Sub_Prec_Shift; // -> Range in *full* pel unit
}

//////////////////////////////////////////////////////////
// Various search methods
//////////////////////////////////////////////////////////

static int MV_Search_Full(ME_MAP * const C)
{
  SKL_UINT32 Best_Sad = C->Best_Sad;

  const SKL_BYTE *Ref = C->Ref + (C->IPred[1]-C->ym)*C->BpS + C->IPred[0];
  int bx=0, by=0;
  for(int y=C->ym; y<=C->yM; ++y) {
    for(int x=C->xm; x<=C->xM; ++x) {
      SKL_UINT32 Sad = C->MV_Bits_Cost(x, y);
      if (Sad>=Best_Sad) continue;
      Sad += C->Metric(C->Src, Ref+x, C->BpS);
      if ( Sad<Best_Sad )
      {
        Best_Sad = Sad;
        bx = x;
        by = y;
      }
    }
    Ref += C->BpS;
  }
  if (Best_Sad<C->Best_Sad) {
    C->Best_Sad = Best_Sad;
    C->Best_MV[0] = bx;
    C->Best_MV[1] = by;
    return 1;
  }
  else return 0;
}

static int MV_Search_Log(ME_MAP * const C)
{
  int nx = C->IPred[0];
  int ny = C->IPred[1];
  int Range = C->Range >> 1;
  SKL_UINT32 Best_Sad = C->Best_Sad;
  do {
    int xm = nx - Range;
    int xM = nx + Range;
    int ym = ny - Range;
    int yM = ny + Range;
    if (xm<C->xm)  xm = C->xm;
    if (xM>C->xM)  xM = C->xM;
    if (ym<C->ym)  ym = C->ym;
    if (yM>C->yM)  yM = C->yM;

    for(int y=ym; y<=yM; y+=Range) {
      const SKL_BYTE *Ref = C->Ref + y*C->BpS;
      for(int x=xm; x<=xM; x+=Range) {
        SKL_UINT32 Sad = C->MV_Bits_Cost(x,y);
        if (Sad>=Best_Sad) continue;
        Sad += C->Metric(C->Src, Ref+x, C->BpS);
        if (Sad<Best_Sad)
        {
          Best_Sad = Sad;
          nx = x;
          ny = y;
        }
      }
    }
    Range >>= 1;
  } while(Range>0);

  if (Best_Sad<C->Best_Sad) {
    C->Best_Sad = Best_Sad;
    C->Best_MV[0] = nx;
    C->Best_MV[1] = ny;
    return 1;
  }
  else return 0;
}

    // it's not exactly a genuine PHODS, but works well...

static int MV_Search_PHODS(ME_MAP * const C)
{
  SKL_MV mv;
  SKL_COPY_MV(mv, C->IPred);
  int dx, dy;
  dx = dy = C->Range >> 1;
  SKL_UINT32 Best_Sad = C->Best_Sad;
  do {

    if (dx) {
      int xm = mv[0] - dx; if (xm< C->xm)  xm = C->xm;
      int xM = mv[0] + dx; if (xM>=C->xM)  xM = C->xM-1;

      const SKL_BYTE * const Ref = C->Ref + mv[1]*C->BpS;
      for(int x=xm; x<=xM; x+=dx) {
        SKL_UINT32 Sad = C->MV_Bits_Cost(x,mv[1]);
        if (Sad>=Best_Sad) continue;
        Sad += C->Metric(C->Src, Ref+x, C->BpS);
        if (Sad<Best_Sad)
        {
          Best_Sad = Sad;
          mv[0] = x;
        }
      }
    }
    if (dy) {
      int ym = mv[1] - dy; if (ym< C->ym)  ym = C->ym;
      int yM = mv[1] + dy; if (yM>=C->yM)  yM = C->yM-1;

      for(int y=ym; y<=yM; y+=dy) {
        SKL_UINT32 Sad = C->MV_Bits_Cost(mv[0],y);
        if (Sad>=Best_Sad) continue;
        Sad += C->Metric(C->Src, C->Ref + y*C->BpS + mv[0], C->BpS);
        if (Sad<Best_Sad)
        {
          Best_Sad = Sad;
          mv[1] = y;
        }
      }
    }
    dx >>= 1;
    dy >>= 1;
  } while(dx!=0 && dy!=0);

  return C->Eval(mv); 
}

static int MV_Search_Diamond(ME_MAP * const C)
{
  C->Start_Search();
  const SKL_UINT32 Best_Sad = C->Best_Sad;
//  fprintf( stderr, "search box: (%d,%d)->(%d,%d) around (%d,%d) [Sad=%d]\n", C->xm, C->ym, C->xM, C->yM, C->xo, C->yo, C->Best_Sad);

  SKL_ASSERT(*C->Get_Cached_Sad(C->Best_MV[0], C->Best_MV[1]) == Best_Sad);
  while(1)
  {
    int x = C->Best_MV[0];
    int y = C->Best_MV[1];
    if (x<C->xM && !(C->Dir&0x02)) C->Eval_Dir(x+1,y,  0x100);
    if (x>C->xm && !(C->Dir&0x01)) C->Eval_Dir(x-1,y,  0x200);
    if (y<C->yM && !(C->Dir&0x08)) C->Eval_Dir(x,  y+1,0x400);
    if (y>C->ym && !(C->Dir&0x04)) C->Eval_Dir(x,  y-1,0x800);

    if (C->Dir&0xf00)    // check corners now, if something was found
    {
      x = C->Best_MV[0];
      y = C->Best_MV[1];
      if (C->Dir&0x300) {
        if (y<C->yM) C->Eval_Dir(x, y+1,0x400);
        if (y>C->ym) C->Eval_Dir(x, y-1,0x800);
      }
      else {
        if (x<C->xM) C->Eval_Dir(x+1, y,0x100);
        if (x>C->xm) C->Eval_Dir(x-1, y,0x200);
      }
      C->Dir >>= 8;
    }
    else break;
  }
//  fprintf( stderr, "found:%d,%d\n", C->Best_MV[0],C->Best_MV[1]);
  return (C->Best_Sad<Best_Sad);
}

static int MV_Search_Square(ME_MAP * const C)
{
  C->Start_Search();
  const SKL_UINT32 Best_Sad = C->Best_Sad;
  SKL_ASSERT(*C->Get_Cached_Sad(C->Best_MV[0], C->Best_MV[1]) == Best_Sad);

  while(1)
  {
    int x = C->Best_MV[0];
    int y = C->Best_MV[1];

    if (!(C->Dir&0x02) && x<C->xM) C->Eval_Dir(x+1,y,  0x100);
    if (!(C->Dir&0x01) && x>C->xm) C->Eval_Dir(x-1,y,  0x200);
    if (!(C->Dir&0x08) && y<C->yM) C->Eval_Dir(x,  y+1,0x400);
    if (!(C->Dir&0x04) && y>C->ym) C->Eval_Dir(x,  y-1,0x800);
    if (x<C->xM) {
      if (y<C->yM) C->Eval_Dir(x+1, y+1,0x500);
      if (y>C->ym) C->Eval_Dir(x+1, y-1,0x900);
    }
    if (x>C->xm) {
      if (y<C->yM) C->Eval_Dir(x-1, y+1,0x600);
      if (y>C->ym) C->Eval_Dir(x-1, y-1,0xa00);
    }
    if (!(C->Dir & 0xf00)) break;
    C->Dir >>= 8;
  }
  return (C->Best_Sad<Best_Sad);
}

//////////////////////////////////////////////////////////

static int MV_Search_Zero(ME_MAP * const C)
{
  C->Best_Sad = C->Metric(C->Src,C->Ref,C->BpS);
  C->Best_MV[0] = 0;
  C->Best_MV[1] = 0;
  return 1;
}

static int MV_Search_Debug(ME_MAP * const C)
{
#if 0
  int dx = -C->Range;
  int dy = 0;
  if (dx<C->xm)      dx = C->xm;
  else if (dx>C->xM) dx = C->xM;
  if (dy<C->ym)      dy = C->ym;
  else if (dy>C->yM) dy = C->yM;
#endif
  static int P = 0;
  int dx = P ? C->Range-1 : -C->Range;
  int dy = 0;
  P ^= 1;
  C->Best_Sad = C->Metric(C->Src, C->Ref+dx+dy*C->BpS, C->BpS);
  C->Best_MV[0] = dx;
  C->Best_MV[1] = dy;
  return 1;
}

//////////////////////////////////////////////////////////

static SEARCH_MTHD MV_Searchs[] = {
  &MV_Search_Square, &MV_Search_Diamond,
  &MV_Search_PHODS,  &MV_Search_Log, &MV_Search_Full,
  &MV_Search_Zero, &MV_Search_Debug
};
enum { MAX_SEARCH = 7 };

//////////////////////////////////////////////////////////
// class ME_MAP 
//////////////////////////////////////////////////////////

void ME_MAP::Init()
{
  xo = 0;
  yo = 0;
  Src0 = Frame->Cur->Y;
  Ref0 = Frame->Past->Y;
  SKL_BZERO(Key_Map, sizeof(Key_Map));
  Scratch1 = (SKL_BYTE*)Src0 - 16 - 16*BpS;
  Scratch2 = Scratch1 + 16;
//  Hit = 0; Miss = 0;
}

inline void ME_MAP::Set_Sub_Offset(const int dx, const int dy)
{
    // Predictor should be set before getting here
  const int x = xo+dx, y = yo+dy;
  Ref = Ref0 + (x + y*BpS);
  Src = Src0 + (x + y*BpS);

  if (-Range<-16-x) xm = -15-x;
  else              xm = -Range+1;
  if (-Range<-16-y) ym = -15-y;
  else              ym = -Range+1;
  if (Range>Frame->Width-x)  xM = Frame->Width-1 -x;
  else                       xM = Range-1;
  if (Range>Frame->Height-y) yM = Frame->Height-1-y;
  else                       yM = Range-1;
//  SKL_ASSERT(xm<=0 && xM>=0 || ym<=0 || yM>=0);

  xm_Sub = ((xm-1)<<Sub_Prec_Shift) + 1;
  xM_Sub = ((xM+1)<<Sub_Prec_Shift) - 1;
  ym_Sub = ((ym-1)<<Sub_Prec_Shift) + 1;
  yM_Sub = ((yM+1)<<Sub_Prec_Shift) - 1;
}

void ME_MAP::New_Scanline(int y)
{
  yo = y;
  xo = 0;
}

SKL_UINT32 ME_MAP::Start(const METRIC metric, const METRIC_AVRG metric_Avrg)
{
  New_Map();
  Set_Sub_Offset(0,0);
  Metric      = metric;
  Metric_Avrg = metric_Avrg;
  return Metric(Src,Ref,BpS);
}


int ME_MAP::Check_Limits(const SKL_MV mv, SKL_CST_STRING Label) const {
  const int Rng = Range << Sub_Prec_Shift;
  if (mv[0]<-Rng || mv[0]>=Rng || mv[1]<-Rng || mv[1]>=Rng) {
    if (Label!=0) fprintf( stderr, Label );
    fprintf( stderr, "mv (%d,%d) has bad range (%d)!!  ", mv[0], mv[1], Rng );
    fprintf( stderr, "search box: (%d,%d)->(%d,%d) around (%d,%d)\n", xm, ym, xM, yM, xo, yo);
    return 0;
  }
  return 1;
}

int ME_MAP::Check_Frame_Limits(const SKL_MV mv, SKL_CST_STRING Label) const {
  int x = ( (xo<<Sub_Prec_Shift) + mv[0] ) >> Sub_Prec_Shift;
  int y = ( (yo<<Sub_Prec_Shift) + mv[1] ) >> Sub_Prec_Shift;
  if (x<-16 || x>=Frame->Width || y<-16 || y>=Frame->Height) {
    fprintf( stderr, "final displacement (%d,%d)=(%d,%d)+(%d,%d) is outside frame (%d,%d)! (predictor?)\n", x,y, xo,yo, mv[0], mv[1], Frame->Width, Frame->Height );
    return 0;
  }
  return 1;
}

//////////////////////////////////////////////////////////
// Field/Frame evaluation
//////////////////////////////////////////////////////////

static SKL_UINT32 Get_Frame_SAD(const SKL_BYTE * const Src,
                                const SKL_MP4_INFOS * const Info)
{
  SKL_UINT32 SAD;
  const int BpS = Info->BpS;
  SAD  = Info->Img_Dsp->SAD_16x7_Self(Src,       BpS);
  SAD += Info->Img_Dsp->SAD_16x7_Self(Src+8*BpS, BpS);
  return SAD;
}

static SKL_UINT32 Get_Field_SAD(const SKL_BYTE *Src, const SKL_MP4_INFOS * const Info)
{
  SKL_UINT32 SAD;
  const int BpS = Info->BpS;
  SAD  = Info->Img_Dsp->SAD_16x7_Self(Src,     2*BpS);
  SAD += Info->Img_Dsp->SAD_16x7_Self(Src+BpS, 2*BpS);
  return SAD;
}

static int Field_DCT_Is_Better(const SKL_BYTE *Src, const SKL_MP4_INFOS * const Info)
{
  return (Get_Field_SAD(Src,Info) < Get_Frame_SAD(Src,Info));
}

//////////////////////////////////////////////////////////
// Sub-pixel interpolation (old and experimental)
//////////////////////////////////////////////////////////

#if 0

// Find the best (assuming it exists!) SSE match position (a,b)
// between two 16x16 blocks Cur0 and Ref0.
//  This implementation uses the SGAN algorithm of mine.


static void Find_Min_SSE(int &a, int &b, 
                         const SKL_BYTE * Cur0,
                         const SKL_BYTE * Ref0,
                         const int BpS)
{
    // we use a barycentric average of every
    // optima, in hope it will lead to an
    // overall good estimate.

    // TODO: search window should be 17x17
    // since minima is located withing [-1,1[x[-1,1[
    // range (not [0,1[x[0,1[)! (check bounds, too)

  int Num;
//  int Den;
  int Nb;
  
    // horizontal pass

  Nb = 0; Num = 0; //Den = 1;
  const SKL_BYTE *Cur = Cur0, *Ref = Ref0;
  for(int y=0; y<15; ++y) {
    int N = 0, D = 0;
    for(int x=0; x<15; ++x) { // boundary pb!
        // a = U.(U-V) / |U-V|^2
      int UV = Ref[x+1] - Ref[x];
      int U  = Ref[x+1] - Cur[x];
      D += UV*UV;
      N += U*UV;
    }
    Cur += BpS; Ref += BpS;

    if (D!=0) {
//      Num = (Num*D + N*Den);
//      Den = (Den*D);
      Num += (N<<8) / D;    // 8b fixed point only?
      Nb++;
    }
  }
  if (Nb) {
    // TODO: clipping should be useless if the minimum has been well-bracketed
    a = 0xff - (Num/Nb);
    if      (a<0x00) a = 0x00;
    else if (a>0xff) a = 0xff;
  }
  else a = 0;

    // vertical pass

  Nb = 0; Num = 0; // Den = 1;
  for(int x=0; x<15; ++x) { // boundary pb!
    int N = 0, D = 0;
    const SKL_BYTE *Cur = Cur0, *Ref = Ref0;
    for(int y=0; y<15; ++y) {
        // b = U.(U-V) / |U-V|^2
      int UV = Ref[x+BpS] - Ref[x];
      int U  = Ref[x+BpS] - Cur[x];
      D += UV*UV;
      N += U*UV;
      Cur += BpS; Ref += BpS;
    }
    if (D!=0) {
      Num += (N<<8) / D;
      Nb++;
    }
  }
  if (Nb) {
    b = 0xff - (Num/Nb);
    if      (b<0x00) b = 0x00;
    else if (b>0xff) b = 0xff;
  }
  else b = 0;
}
#endif

#define SRND(x)   (((x)+0x80)>>8)

#if 0
void ME_MAP::Refine_MV(SKL_MV MV, 
                       const int Prec,
                       const int Rounding)
{
  if (Prec>0) {
    int H, V;

    const SKL_BYTE *Rf = Ref + Best_MV[0] + Best_MV[1]*BpS;   // input MV is FULL pel
    Find_Min_SSE(H, V, Rf, Src, BpS);
    if (Prec==1) {
      MV[0] = (Best_MV[0]<<1) + SRND(H);
      MV[1] = (Best_MV[1]<<1) + SRND(V);
    }
    else {  // qpel
      MV[0] = (Best_MV[0]<<2) + SRND(2*H);
      MV[1] = (Best_MV[1]<<2) + SRND(2*V);
    }
//    printf( "%d,%d -> %d %d\n", Best_MV[0]<<1, Best_MV[1]<<1, MV[0], MV[1] );
  }
  else {
    MV[0] = Best_MV[0]<<1;
    MV[1] = Best_MV[1]<<1;
  }
}
#endif

//////////////////////////////////////////////////////////
// refinement sub-searches

#define SKL_AUTO_INCLUDE

  // shameless copy-paste from skl_mpg4i.h. TODO: BAD.

static void Predict_16x16_QP(SKL_BYTE * const Dst, const SKL_BYTE *Src,
                             SKL_BYTE * const YTmp,  // <- 17x16 buffer
                             const int x, const int y, const int BpS,
                             const SKL_MB_FUNCS * const Ops)
{
  const int Quads = (x&3) | ((y&3)<<2);
  Src += (y>>2)*BpS + (x>>2);

  switch(Quads) {
    default:
    case 0:  Ops->HP_16x8[0](Dst, Src, BpS);
             Ops->HP_16x8[0](Dst+8*BpS, Src+8*BpS, BpS); break;
    case 1:  Ops->H_Pass_Avrg(Dst, Src, 16, BpS);        break;
    case 2:  Ops->H_Pass(Dst, Src, 16, BpS);             break;
    case 3:  Ops->H_Pass_Avrg_Up(Dst, Src, 16, BpS);     break;

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

static void Predict_16x8_Field_QP(SKL_BYTE * const Dst, const SKL_BYTE *Src,
                                  SKL_BYTE * const YTmp,  // <- 9x16 buffer
                                  const int x, const int y, const int BpS,
                                  const SKL_MB_FUNCS * const Ops)
{
  const int Quads = (x&3) | ((y&3)<<2);
  Src += ((y>>1)&~1)*BpS + (x>>2);

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


static void Predict_8x8_QP(SKL_BYTE * const Dst, const SKL_BYTE *Src,
                           SKL_BYTE * const YTmp,  // <- 9x16 buffer
                           const int x, const int y, const int BpS,
                           const SKL_MB_FUNCS * const Ops)
{
  const int Quads = (x&3) | ((y&3)<<2);
  Src += (y>>2)*BpS + (x>>2);

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



#define GET_HP_SAD(Ref,dx,dy)   \
  Funcs[dx+dy*2](Scratch2,       Ref,       BpS); \
  Funcs[dx+dy*2](Scratch2+8*BpS, Ref+8*BpS, BpS); \
  Sad = Metric(Src,Scratch2,BpS)

#define GET_QP_SAD(x, y)                       \
  Predict_16x16_QP(Scratch2, Ref, Scratch1, (x), (y), BpS, Funcs); \
  Sad = Metric(Src,Scratch2,BpS) + MV_Bits_Cost_Raw((x),(y))

#define REFINE_METHOD Refine_MV_16x16
#define FUNC_NAME HP_16x8
#define BPS BpS
#include "./skl_mpg4_anl.cpp"


#define GET_HP_SAD(Ref,dx,dy)    \
  Funcs[dx+dy*2](Scratch2, Ref, BpS); \
  Sad = Metric(Src,Scratch2,BpS)

#define GET_QP_SAD(x, y)                     \
  Predict_8x8_QP(Scratch2, Ref, Scratch1, (x), (y), BpS, Funcs); \
  Sad = Metric(Src,Scratch2,BpS) + MV_Bits_Cost_Raw((x),(y))

#define REFINE_METHOD Refine_MV_8x8
#define FUNC_NAME HP_8x8
#define BPS BpS
#include "./skl_mpg4_anl.cpp"


#define GET_HP_SAD(Ref,dx,dy)      \
  Funcs[dx+dy*2](Scratch2, Ref, 2*BpS); \
  Sad = Metric(Src,Scratch2,2*BpS)

#define GET_QP_SAD(x, y)                            \
  Predict_16x8_Field_QP(Scratch2, Ref, Scratch1, (x), (y), BpS, Funcs); \
  Sad = Metric(Src,Scratch2,2*BpS) + MV_Bits_Cost_Raw((x),(y))

#define REFINE_METHOD Refine_MV_16x8
#define FUNC_NAME HP_16x8
#define BPS 2*BpS
#include "./skl_mpg4_anl.cpp"


#undef SKL_AUTO_INCLUDE

#endif  /* !SKL_AUTO_INCLUDE */

//////////////////////////////////////////////////////////

#ifdef SKL_AUTO_INCLUDE

SKL_UINT32 ME_MAP::REFINE_METHOD(SKL_MV MV, const int Prec, const int Rounding)
{
  MV[0] = Best_MV[0] << 1;
  MV[1] = Best_MV[1] << 1;
  if (Prec==0)
    return Best_Sad;

  const SKL_BYTE *Rf = Ref + Best_MV[0] + Best_MV[1]*BpS;   // input MV is FULL pel
  SKL_UINT32 Sad0 = Best_Sad;
  SKL_UINT32 Sad;
  int dx = 0, dy = 0;

  if (Prec==1)
  {
    const int x = MV[0];
    const int y = MV[1];
    const SKL_MB_FUNC * const Funcs = Frame->MB_Dsp->Copy[Rounding]->FUNC_NAME;

    GET_HP_SAD( Rf-1, 1,0 );
    Sad += MV_Bits_Cost_Raw(x-1, y);
    if (Sad<Sad0) { Sad0 = Sad;  dx=-1; dy=0; }

    GET_HP_SAD( Rf, 1,0 );
    Sad += MV_Bits_Cost_Raw(x+1, y);
    if (Sad<Sad0) { Sad0 = Sad;  dx=1; dy=0; }

    GET_HP_SAD( Rf-BPS, 0,1 );
    Sad += MV_Bits_Cost_Raw(x, y-1);  
    if (Sad<Sad0) { Sad0 = Sad; dx=0; dy=-1; }

    GET_HP_SAD( Rf, 0,1 );
    Sad += MV_Bits_Cost_Raw(x, y+1);
    if (Sad<Sad0) { Sad0 = Sad; dx=0; dy=1; }

    GET_HP_SAD( Rf-1-BPS, 1,1 );
    Sad += MV_Bits_Cost_Raw(x-1, y-1);  
    if (Sad<Sad0) { Sad0 = Sad; dx=-1; dy=-1; }

    GET_HP_SAD( Rf, 1,1 );
    Sad += MV_Bits_Cost_Raw(x+1, y+1);
    if (Sad<Sad0) { Sad0 = Sad; dx= 1; dy= 1; }

    GET_HP_SAD( Rf-BPS, 1,1 );
    Sad += MV_Bits_Cost_Raw(x+1, y-1);
    if (Sad<Sad0)  { Sad0 = Sad;  dx=1; dy=-1; }

    GET_HP_SAD( Rf-1, 1,1 );
    Sad += MV_Bits_Cost_Raw(x-1, y+1);
    if (Sad<Sad0) { Sad0 = Sad; dx=-1; dy=1; }

    MV[0] += dx;
    MV[1] += dy;

  }
  else
  {
    const SKL_MB_FUNCS * const Funcs = Frame->MB_Dsp->Copy[Rounding];
    int x = Best_MV[0] << 2;
    int y = Best_MV[1] << 2;
    int dx = 0, dy = 0;

    GET_QP_SAD(x-2, y+0);
    if (Sad<Sad0) { Sad0 = Sad; dx=-2; dy= 0; }
    GET_QP_SAD(x+2, y+0);
    if (Sad<Sad0) { Sad0 = Sad; dx= 2; dy= 0; }
    GET_QP_SAD(x  , y-2);
    if (Sad<Sad0) { Sad0 = Sad; dx= 0; dy=-2; }
    GET_QP_SAD(x  , y+2);
    if (Sad<Sad0) { Sad0 = Sad; dx= 0; dy= 2; }
    GET_QP_SAD(x-2, y-2);
    if (Sad<Sad0) { Sad0 = Sad; dx=-2; dy=-2; }
    GET_QP_SAD(x-2, y+2);
    if (Sad<Sad0) { Sad0 = Sad; dx=-2; dy= 2; }
    GET_QP_SAD(x+2, y-2);
    if (Sad<Sad0) { Sad0 = Sad; dx= 2; dy=-2; }
    GET_QP_SAD(x+2, y+2);
    if (Sad<Sad0) { Sad0 = Sad; dx= 2; dy= 2; }

    x += dx;
    y += dy;
    dx = 0; dy = 0;

    GET_QP_SAD(x-1, y+0);
    if (Sad<Sad0) { Sad0 = Sad; dx=-1; dy= 0; }
    GET_QP_SAD(x+1, y+0);
    if (Sad<Sad0) { Sad0 = Sad; dx= 1; dy= 0; }
    GET_QP_SAD(x  , y-1);
    if (Sad<Sad0) { Sad0 = Sad; dx= 0; dy=-1; }
    GET_QP_SAD(x  , y+1);
    if (Sad<Sad0) { Sad0 = Sad; dx= 0; dy= 1; }
    GET_QP_SAD(x-1, y-1);
    if (Sad<Sad0) { Sad0 = Sad; dx=-1; dy=-1; }
    GET_QP_SAD(x-1, y+1);
    if (Sad<Sad0) { Sad0 = Sad; dx=-1; dy= 1; }
    GET_QP_SAD(x+1, y-1);
    if (Sad<Sad0) { Sad0 = Sad; dx= 1; dy=-1; }
    GET_QP_SAD(x+1, y+1);
    if (Sad<Sad0) { Sad0 = Sad; dx= 1; dy= 1; }

    MV[0] = x+dx;
    MV[1] = y+dy;
  }

  Best_Sad = Sad0;
  return Best_Sad;
}

#undef GET_HP_SAD
#undef GET_QP_SAD
#undef REFINE_METHOD
#undef FUNC_NAME
#undef BPS

#endif  /* SKL_AUTO_INCLUDE */

#ifndef SKL_AUTO_INCLUDE

#define TEST_HP(Ref,DX,DY)  \
  Sad = Metric(Src, (Ref), BpS) + MV_Bits_Cost_Raw(x+(DX), y+(DY)); \
  if (Sad<Sad0) { Sad0 = Sad; dx = (DX); dy = (DY); }

SKL_UINT32 ME_MAP::Sad_Cost_QP(const SKL_BYTE * const *HPels, int Offset, const int x, const int y) const
{
  SKL_UINT32 Cost;

  const int q = (x&3) | (4*(y&3));
  Offset += (x>>2) + (y>>2)*BpS;
  const SKL_BYTE * const Ref    = HPels[0] + Offset;
  const SKL_BYTE * const Ref_H  = HPels[1] + Offset;
  const SKL_BYTE * const Ref_V  = HPels[2] + Offset;
  const SKL_BYTE * const Ref_HV = HPels[3] + Offset;

  switch(q) {     // Warning ! SAD is the only supported metric  (TODO ?)
    default:
    case  0: Cost = Metric( Src, Ref, BpS);     break;
    case  1: Cost = Metric_Avrg(Src, Ref,Ref_H, BpS);   break;
    case  2: Cost = Metric( Src, Ref_H, BpS);   break;
    case  3: Cost = Metric_Avrg(Src, Ref+1,Ref_H, BpS);   break;

    case  4: Cost = Metric_Avrg(Src, Ref,Ref_V, BpS);   break;
    case  5: Cost = Metric_Avrg(Src, Ref_H, Ref_V, BpS);  break;
    case  6: Cost = Metric_Avrg(Src, Ref_HV, Ref_H, BpS); break;
    case  7: Cost = Metric_Avrg(Src, Ref_V+1, Ref_H, BpS);  break;

    case  8: Cost = Metric( Src, Ref_V, BpS);   break;
    case  9: Cost = Metric_Avrg(Src, Ref_V, Ref_HV, BpS); break;
    case 10: Cost = Metric( Src, Ref_HV, BpS);    break;
    case 11: Cost = Metric_Avrg(Src, Ref_V+1, Ref_HV, BpS); break;

    case 12: Cost = Metric_Avrg(Src, Ref+BpS,Ref_V, BpS);  break;
    case 13: Cost = Metric_Avrg(Src, Ref_V, Ref_H+BpS, BpS);   break;
    case 14: Cost = Metric_Avrg(Src, Ref_H+BpS, Ref_HV, BpS);  break;
    case 15: Cost = Metric_Avrg(Src, Ref_V+1, Ref_H+BpS, BpS); break;
  }
  Cost += MV_Bits_Cost_Raw(x,y);
  return Cost;
}

#define TEST_QP(X,Y)  \
  Sad = Sad_Cost_QP(HPels, Offset0, x+(X), y+(Y));  \
  if (Sad<Sad0) { Sad0 = Sad; dx = (X); dy = (Y); }

SKL_UINT32 ME_MAP::Refine_MV_HPels(SKL_MV MV, const int Prec, const int Rounding, 
                                   const SKL_BYTE * const *HPels)
{
  if (Prec==2) {
    MV[0] = Best_MV[0] << 2;
    MV[1] = Best_MV[1] << 2;
  }
  else {
    MV[0] = Best_MV[0] << 1;
    MV[1] = Best_MV[1] << 1;
  }
  if (Prec==0)
    return Best_Sad;

  int x = MV[0];
  int y = MV[1];
  const int Offset0 = Ref - Ref0;
  const int Offset = Offset0 + Best_MV[0] + Best_MV[1]*BpS;   // input MV is FULL pel
  SKL_UINT32 Sad0 = Best_Sad;
  SKL_UINT32 Sad = Sad0;
  int dx, dy;
  
  dx = 0;
  dy = 0;

  TEST_HP(HPels[1]+Offset-1    ,-Prec,     0);
  TEST_HP(HPels[1]+Offset      , Prec,     0);
  TEST_HP(HPels[2]+Offset-BpS  ,    0, -Prec);
  TEST_HP(HPels[2]+Offset      ,    0,  Prec);
  TEST_HP(HPels[3]+Offset-1-BpS,-Prec, -Prec);
  TEST_HP(HPels[3]+Offset      , Prec,  Prec);
  TEST_HP(HPels[3]+Offset-BpS  , Prec, -Prec);
  TEST_HP(HPels[3]+Offset-1    ,-Prec,  Prec);

  if (Prec==1)
    goto End;

#if 1

  x += dx;
  y += dy;

  dx = 0;
  dy = 0;

  TEST_QP(-1,-1);
  TEST_QP(+0,-1);
  TEST_QP(+1,-1);
  TEST_QP(-1,+0);
  TEST_QP(+1,+0);
  TEST_QP(-1,+1);
  TEST_QP(+0,+1);
  TEST_QP(+1,+1);

#else

    // Not very faster... and misses a lot of better matches
  if (dy==0) {
    const int midx = dx>>1;
    TEST_QP(  +0,+1);
    TEST_QP(  +0,-1);
    if (dx>=0) { TEST_QP(+1,0); }
    if (dx<=0) { TEST_QP(-1,0); }
    TEST_QP(midx,+1);
    TEST_QP(midx,-1);
  }
  else if (dx==0) {
    const int midy = dy>>1;
    TEST_QP(+1,   0);
    TEST_QP(-1,   0);
    if (dy>=0) { TEST_QP( 0, 1); }
    if (dy<=0) { TEST_QP( 0,-1); }
    TEST_QP(+1,midy);
    TEST_QP(-1,midy);
  }
  else {
    const int midx = dx>>1;
    const int midy = dy>>1;
    TEST_QP(midx,   0);
    TEST_QP(   0,midy);
    TEST_QP(midx,midy);
  }

#endif

End:
  MV[0] = x + dx;
  MV[1] = y + dy;
  Best_Sad = Sad0;
  return Sad0;
}

#undef TEST_HP
#undef TEST_QP

//////////////////////////////////////////////////////////
// Refinement pass
//////////////////////////////////////////////////////////

  /* TODO: Put elsewhere */


  // Map from Availability-bits to Prediction Type:
  //  Pred Type is the following, used in descending probability:
  //  . Median with 3 vectors
  //  . Median with 2 vectors (!Top/!Top_Right/!Left) + zero-vector
  //  . Copy vector (Left, Top, Top_Right)
  //  . Zero vector
  //  ABits is : Left_Ok|Top_Ok|Right_Ok|Bottom_Ok

static const SKL_BYTE Pred_Type_ABits[16][4] = {  // index: [ABits (4b)][Blk=0..3]
    {7, 1, 2, 0}
  , {1, 1, 0, 0}
  , {5, 4, 2, 0}
  , {4, 4, 0, 0}
  , {7, 1, 2, 0}
  , {1, 1, 0, 0}
  , {2, 0, 2, 0}
  , {0, 0, 0, 0}
  , {7, 1, 2, 0}
  , {1, 1, 0, 0}
  , {5, 4, 2, 0}
  , {4, 4, 0, 0}
  , {7, 1, 2, 0}
  , {1, 1, 0, 0}
  , {2, 0, 2, 0}
  , {0, 0, 0, 0}
};

static const int Top_Neighbors[4] = { 2, 1, 1, -1 };

static SKL_INT16 Median(SKL_INT16 x, SKL_INT16 y, SKL_INT16 z) {
  SKL_INT16 m = x;
  SKL_INT16 M = x;
  if (y < m) m = y;
  else M = y;
  if (z <= m) return m;
  else if (z >= M) return M;
  else return z;
}

static inline void Median(SKL_MV Dst, const SKL_MV A, const SKL_MV B, const SKL_MV C)
{
  Dst[0] = Median(A[0],B[0],C[0]);
  Dst[1] = Median(A[1],B[1],C[1]);
}
static inline void Median2(SKL_MV Dst, const SKL_MV A, const SKL_MV B)
{
  Dst[0] = Median(A[0],B[0],0);
  Dst[1] = Median(A[1],B[1],0);
}

inline void ME_MAP::Set_MV_Predictor(const SKL_MV * const Src, 
                                     const int MV_Stride, const int Blk,
                                     const int ABits)
{
  SKL_ASSERT(Blk>=0 && Blk<4);
  const SKL_MV * const MV_L  = Src - 1;
  const SKL_MV * const MV_T  = Src - MV_Stride;
  const SKL_MV * const MV_TR = MV_T + Top_Neighbors[Blk];
  const int Pred_Type = Pred_Type_ABits[ABits][Blk];
  switch(Pred_Type) {
    default:
    case 0:
      Median(Pred, MV_L[0], MV_T[0], MV_TR[0]);
    break;
    case 1:
      SKL_COPY_MV(Pred, *MV_L);
    break;
    case 2:
      Median2(Pred, MV_T[0], MV_TR[0]);
    break;
    case 3:
      Median2(Pred, MV_L[0], MV_TR[0]);
    break;
    case 4:
      Median2(Pred, MV_L[0], MV_T[0]);
    break;
    case 5:
      SKL_COPY_MV(Pred, *MV_T);
    break;
    case 6:
      SKL_COPY_MV(Pred, *MV_TR );
    break;
    case 7:
      SKL_ZERO_MV(Pred);
    break;
  }
  if (Sub_Prec_Shift==2) {
    IPred[0] = Pred[0]/4;
    IPred[1] = Pred[1]/4;
    // IPred[0] = (Pred[0]+2)>>2;
    // IPred[1] = (Pred[1]+2)>>2;
  }
  else {
    IPred[0] = Pred[0]/2;
    IPred[1] = Pred[1]/2;
    // IPred[0] = (Pred[0]+1)>>1;
    // IPred[1] = (Pred[1]+1)>>1;
  }
}

inline void ME_MAP::Set_FP_MV_Predictor(SKL_MV Pred, const SKL_MP4_MAP * const Map,
                                        const int Map_Stride, const int ABits)
{
  const SKL_MV * const MV_L  = &Map[-1].MV;
  const SKL_MV * const MV_T  = &Map[-Map_Stride].MV;
  const SKL_MV * const MV_TR = &Map[-Map_Stride+1].MV;
  const int Pred_Type = Pred_Type_ABits[ABits][0];
  switch(Pred_Type) {
    default:
    case 0:
      Median(Pred, MV_L[0], MV_T[0], MV_TR[0]);
    break;
    case 1:
      SKL_COPY_MV(Pred, *MV_L);
    break;
    case 2:
      Median2(Pred, MV_T[0], MV_TR[0]);
    break;
    case 3:
      Median2(Pred, MV_L[0], MV_TR[0]);
    break;
    case 4:
      Median2(Pred, MV_L[0], MV_T[0]);
    break;
    case 5:
      SKL_COPY_MV(Pred, *MV_T);
    break;
    case 6:
      SKL_COPY_MV(Pred, *MV_TR );
    break;
    case 7:
      SKL_ZERO_MV(Pred);
    break;
  }
}

//////////////////////////////////////////////////////////
// global pass
//////////////////////////////////////////////////////////

static inline void Update_Map_Data(SKL_MP4_MAP *const Map,
                                   SKL_MP4_MAP *const pMap,
                                   SKL_UINT32 Sad16, const SKL_MV MV)
{
  Map->Sad16 = Sad16;
  SKL_COPY_MV(Map->MV, MV);
  if (pMap) {
    Map->Acc[0] = 2*Map->MV[0] - pMap->MV[0];
    Map->Acc[1] = 2*Map->MV[1] - pMap->MV[1];
  }
  else {
    SKL_ZERO_MV(Map->Acc);
  }
}

  // Populates MVs[], Map->Type and Map->Flags

void SKL_MP4_ANALYZER_I::Global_Pass(SKL_MP4_INFOS * const Frame)
{
  const int FCode = RND(MV_Search_Window);
  int Nb_Intras   = 0;
  int Nb_Relevant = 0;

  Reset_Stats();  // will store previous Last_Avrg_MV (if any)

  ME_MAP Cursor(Frame);

  SKL_UINT32 SAD_Hi   = SAD_Hi_Limit;
  SKL_UINT32 SAD_Lo   = SAD_Low_Limit;

    // for now, disable 4v search with reduced VOP
  const int Search_4V = (Reduced_Frame<1) && (Inter4V_Probing>0);
  const int BpS = Frame->BpS;

  SKL_UINT32 Sad4_Penalty;  // penalty for 4v
  SKL_UINT32 Sad2_Penalty;  // penalty for field interlacing

  METRIC Metric_16x16, Metric_8x8, Metric_16x8;
  METRIC_AVRG Metric_16x16_Avrg, Metric_8x8_Avrg, Metric_16x8_Avrg;
  METRIC_DEV Metric_Dev;
  switch( Search_Metric ) {
    case 2:
      Metric_16x16 = Frame->Img_Dsp->Hadamard_SAD_16x16;
      Metric_16x8  = Frame->Img_Dsp->Hadamard_SAD_16x8_Field;
      Metric_8x8   = Frame->Img_Dsp->Hadamard_SAD_8x8;
      Metric_Dev   = Frame->Img_Dsp->Hadamard_Dev_16x16;
      Metric_16x16_Avrg  = Frame->Img_Dsp->SAD_Avrg_16x16;
      Metric_16x8_Avrg   = Frame->Img_Dsp->SAD_Avrg_16x8;
      Metric_8x8_Avrg    = Frame->Img_Dsp->SAD_Avrg_8x8;
      Cursor.Lambda = (int)( Q_Cur*0.9f*Lambda );
      SAD_Lo = 2*SAD_Lo;
      SAD_Hi = 2*SAD_Hi;
      Sad4_Penalty = (SKL_UINT32)( .2f * Q_Cur*Lambda );
      Sad2_Penalty = (SKL_UINT32)( .6f * Q_Cur*Lambda );
    break;
    case 1:
      Metric_16x16 = Frame->Img_Dsp->SSD_16x16;
      Metric_16x8  = Frame->Img_Dsp->SSD_16x8_Field;
      Metric_8x8   = Frame->Img_Dsp->SSD_8x8;
      Metric_Dev   = Frame->Img_Dsp->Sqr_Dev_16x16;
      Metric_16x16_Avrg  = Frame->Img_Dsp->SAD_Avrg_16x16;
      Metric_16x8_Avrg   = Frame->Img_Dsp->SAD_Avrg_16x8;
      Metric_8x8_Avrg    = Frame->Img_Dsp->SAD_Avrg_8x8;
      Cursor.Lambda = (int)( Q_Cur*Q_Cur*3.5f*Lambda );
      SAD_Lo = SAD_Lo;
      SAD_Hi = SAD_Hi;
      Sad4_Penalty = (SKL_UINT32)( 0.1f * Q_Cur * Q_Cur*Lambda );
      Sad2_Penalty = (SKL_UINT32)( 3.8f * Q_Cur * Q_Cur*Lambda );
    break;
    default:
      Metric_16x16 = Frame->Img_Dsp->SAD_16x16;
      Metric_16x8  = Frame->Img_Dsp->SAD_16x8_Field;
      Metric_8x8   = Frame->Img_Dsp->SAD_8x8;
      Metric_Dev   = Frame->Img_Dsp->Abs_Dev_16x16;
      Metric_16x16_Avrg  = Frame->Img_Dsp->SAD_Avrg_16x16;
      Metric_16x8_Avrg   = Frame->Img_Dsp->SAD_Avrg_16x8;
      Metric_8x8_Avrg    = Frame->Img_Dsp->SAD_Avrg_8x8;
      Cursor.Lambda = (int)( Q_Cur*0.8f*Lambda );
      SAD_Lo = 2*SAD_Lo;
      SAD_Hi = 2*SAD_Hi;
      Sad4_Penalty = (SKL_UINT32)( 2.4f * Q_Cur*Lambda );
      Sad2_Penalty = (SKL_UINT32)( 0.7f * Q_Cur*Lambda );
    break;
  }
  const SKL_UINT32 SKIP_Limit = SAD_Lo >> 2;   // TODO: Fine tune. Is it a good idea, btw?

  Cursor.Search_Method = MV_Searchs[Search_Method];
  Cursor.Metric        = Metric_16x16;
  Cursor.Metric_Avrg   = Metric_16x16_Avrg;
  Cursor.Set_FCode( FCode, Subpixel_Precision );

  const int MV_Stride = Frame->MV_Stride;
  const int Map_Stride = Frame->MB_W;
  SKL_MP4_MAP *Map  = Frame->Cur->Map;
  SKL_MV *MVs       = Frame->Cur->MV;
  SKL_MV *pMVs      = (Frame->Past ? Frame->Past->MV  : 0);
  SKL_MP4_MAP *pMap = (Frame->Past ? Frame->Past->Map : 0);

    // Note: Hi_Mem is only worth of QPel
  const int Use_HPels = (Hi_Mem && Frame->Past!=0) && (Subpixel_Precision==2);

  if (Use_HPels) Set_Half_Pels(Frame, Frame->Past);
  else _HPels[0] = 0;   // sanity check

  for(int y=0; y<Frame->Height; y+=16)
  {
    Cursor.New_Scanline(y);
    for(int x=0; x<Frame->Width; x+=16)
    {
//      SKL_MV Maxs[2];

      const int ABits = (x>0) | ((y>0)<<1) | ((x+16<Frame->Width)<<2) | ((y+16<Frame->Height)<<3);
      Cursor.Set_MV_Predictor(&MVs[0], MV_Stride, 0, ABits);
      SKL_UINT32 Dist0 = Cursor.Start(Metric_16x16, Metric_16x16_Avrg);      // Dist0 is distortion for mv=(0,0)
      SKL_UINT32 Sad0 = Dist0 + Cursor.MV_Bits_Cost(0,0); // Cost for mv=(0,0)
      Cursor.Cache_Sad(Cursor.xM<0 ? Cursor.xM : Cursor.xm>0 ? Cursor.xm : 0,
                       Cursor.yM<0 ? Cursor.yM : Cursor.ym>0 ? Cursor.ym : 0,
                       Sad0);
      Nb_Relevant += (Dist0>300);

      Map->Flags = 0; // a priori: no field-DCT
      Map->dQ    = 0;
      Map->Sad   = Sad0;
      Map->Sad16 = Sad0;
      Map->Type  = SKL_MAP_16x16;  // default mode: INTER, with 1MV

      SKL_ASSERT(Cursor.xo == x && Cursor.yo==y);
      SKL_ASSERT(Cursor.Src == Frame->Cur->Y  + x + y*BpS);
      SKL_ASSERT(Cursor.Ref == Frame->Past->Y + x + y*BpS);

      SKL_UINT32 Sad_Limit;


#if 1
      if (Dist0<SKIP_Limit) {
        SKL_UINT32 Chroma_SAD;
        const int Chroma_Off = (x/2) + (y/2)*BpS;
        Chroma_SAD  = Metric_8x8(Frame->Cur->U + Chroma_Off, Frame->Past->U + Chroma_Off, BpS);
        Chroma_SAD += Metric_8x8(Frame->Cur->V + Chroma_Off, Frame->Past->V + Chroma_Off, BpS);
        if (Chroma_SAD*2<SKIP_Limit) {
          Map->Type = SKL_MAP_SKIPPED;
          SKL_ZERO_MV(MVs[0]);  // <- prepare, in case it gets later promoted to INTER_16x16 (GMC)
          Update_Map_Data(Map, pMap, Dist0, MVs[0]);
          goto Done_16x16;
        }
      }

//      if (Dist0<SAD_Lo*2) Cursor.Best_Sad = SAD_Lo*2;
//      else if (Dist0>SAD_Hi*2) Cursor.Best_Sad = SAD_Hi*2;
#endif

      Cursor.Eval_If_Valid(Cursor.Pred);
#if 1
      if (ABits&7) {
        SKL_MV Med;
        Cursor.Set_FP_MV_Predictor(Med, Map, Map_Stride, ABits);
        Cursor.Eval_FP_If_Valid(Med);
      }
#endif

      if (Cursor.Best_Sad<256)
        goto Skip_Search;

      Sad_Limit = Map[0].Sad16;
      if (pMap) {
//        Cursor.Eval_FP_If_Valid(pMap[0].MV);  // past colocated
        if (ABits&4)
          Cursor.Eval_FP_If_Valid(pMap[1].MV);    // past left
      }

      if (ABits&1) {    // left
        Cursor.Eval_FP_If_Valid(Map[-1].MV);
        if (Map[-1].Sad16<Sad_Limit) Sad_Limit = Map[-1].Sad16;
      }
      if (ABits&2) {    // top
        Cursor.Eval_FP_If_Valid(Map[-Map_Stride].MV);
        if (Map[-Map_Stride].Sad16<Sad_Limit) Sad_Limit = Map[-Map_Stride].Sad16;

        if (ABits&4) {  // top-right
          Cursor.Eval_FP_If_Valid(Map[-Map_Stride+1].MV);
          if (Map[-Map_Stride+1].Sad16<Sad_Limit) Sad_Limit = Map[-Map_Stride+1].Sad16;
        }
      }

      if (Cursor.Best_Sad < Sad_Limit)
        goto Skip_Search;

       
      if (pMap && Cursor.Best_Sad>2*SAD_Lo)
      {
#if 1
        if (ABits&1) Cursor.Eval_FP_If_Valid(pMap[-1].MV);    // past left
        if (ABits&4) Cursor.Eval_FP_If_Valid(pMap[ 1].MV);    // past right
        if (ABits&2) Cursor.Eval_FP_If_Valid(pMap[-Map_Stride].MV);    // past top
        if (ABits&8) Cursor.Eval_FP_If_Valid(pMap[ Map_Stride].MV);    // past bottom
        Cursor.Eval_FP_If_Valid(pMap[0].Acc);    // accelerator
#endif
        Cursor.Eval_If_Valid(Last_Avrg_MV);      // Last avrg MV
        Cursor.Eval_FP_If_Valid(pMap[0].MV);     // past colocated
        if ((ABits&12)==12) Cursor.Eval_FP_If_Valid(pMap[Map_Stride+1].MV);    // past bottom right

        if (Cursor.Best_Sad < Sad_Limit)
          goto Skip_Search;

//        if (IS_EQUAL_MV(Cursor.Best_MV,pMap[0].MV) && Cursor.Best_Sad<pMap[0].Sad16)
//          goto Skip_Search;
      }

      Cursor.Search();

Skip_Search:

      Update_Map_Data(Map, pMap, Cursor.Best_Sad, Cursor.Best_MV);

      if (Cursor.Best_Sad > SAD_Hi && 
          Cursor.Best_Sad > Metric_Dev(Cursor.Src, BpS))
      {
        Map->Type = SKL_MAP_INTRA;
        Nb_Intras++;
        SKL_ZERO_MV(MVs[0]);
        goto Done_16x16;
      }

      if (Use_HPels) Cursor.Refine_MV_HPels(MVs[0], Subpixel_Precision, Rounding, _HPels);
      else           Cursor.Refine_MV_16x16(MVs[0], Subpixel_Precision, Rounding);

      Map->Sad = Cursor.Best_Sad;

      if (Interlace_Field>0)
      {
        if (Field_Pred_Probing<100) {
            // 50-60% is a good compromise between speed and probing
          const SKL_UINT32 Sad_Sub_Limit = Cursor.Best_Sad * (100-Field_Pred_Probing) /100;
          const SKL_BYTE * Rf = Cursor.Ref + Cursor.Best_MV[0] + Cursor.Best_MV[1]*BpS;
          SKL_UINT32 Sad2_0;
          Sad2_0  = Metric_16x8(Cursor.Src     , Rf, BpS);
          if (Sad2_0>=Sad_Sub_Limit) goto Go_Field_Pred;
          Sad2_0 = Metric_16x8(Cursor.Src + BpS, Rf, BpS);
          if (Sad2_0>=Sad_Sub_Limit) goto Go_Field_Pred;
          goto Done_Field;
        }

Go_Field_Pred:
        {
          SKL_UINT32 Sad2, Sad16, Saved_Sad = Cursor.Best_Sad;
          SKL_MV Saved_MV, Best_MV;
          SKL_COPY_MV( MVs[1], MVs[0] );
          SKL_COPY_MV( Saved_MV, MVs[0] );
          SKL_COPY_MV( Best_MV, Cursor.Best_MV );
   
          Cursor.Metric = Metric_16x8;
          Cursor.Metric_Avrg   = Metric_16x8_Avrg;

          Sad16 = Cursor.Sub_Search(Map[0].MV, 0,0);
          Sad2  = Cursor.Refine_MV_16x8(MVs[0], Subpixel_Precision, Rounding);

          Sad16 += Cursor.Sub_Search(Map[0].MV, 0,1);
          Sad2  += Cursor.Refine_MV_16x8(MVs[1], Subpixel_Precision, Rounding);


            // penalty for going field-pred is very high (we'll pay 2mv+1bit per mb)

          Sad2 = Sad2*12/10 + Sad2_Penalty;

          Cursor.Set_Sub_Offset(0,0);
          if (Sad2<Saved_Sad || Interlace_Field==2)
          {
            Map->Type = SKL_MAP_16x8; // -> use Field_Pred
            Map->Sad   = Sad2;
            MVs[1][1] += 1;   // compensate Src bottom field offset
            MVs[MV_Stride+0][0] = (MVs[0][0]+MVs[1][0])>>1;
            MVs[MV_Stride+0][1] = (MVs[0][1]+MVs[1][1])>>1;
            SKL_COPY_MV(MVs[MV_Stride+1], MVs[MV_Stride+0]);
            Update_Map_Data(Map, pMap, Sad16, MVs[MV_Stride+0]);
            
            Add_To_Stats(MVs[0]);
            Add_To_Stats(MVs[1]);

            goto Done;
          }
          else {
            SKL_COPY_MV( MVs[0], Saved_MV );  // wasn't worth...
            SKL_COPY_MV( Cursor.Best_MV, Best_MV );
            Cursor.Best_Sad = Saved_Sad;
            Cursor.Metric      = Metric_16x16;
            Cursor.Metric_Avrg = Metric_16x16_Avrg;
          }
        }
      }
Done_Field:

        // at this point, MVs[0] is in half/quarter pel unit...
      SKL_ASSERT(Cursor.Is_In_Range(MVs[0]));
      SKL_ASSERT(Map->Type==SKL_MAP_16x16);

//      Set_Max(Maxs[0], MVs[0], Cursor.Pred);

      if (Search_4V)
      {
#if 1
        if (Inter4V_Probing<100)
        {
          const SKL_UINT32 Sad_Sub_Limit = Cursor.Best_Sad * (100-Inter4V_Probing) / 100;
          const SKL_BYTE * Rf = Cursor.Ref + Cursor.Best_MV[0] + Cursor.Best_MV[1]*BpS;
          SKL_UINT32 Sad4_0;

          Sad4_0  = Metric_8x8(Cursor.Src           , Rf,   BpS);
          if (Sad4_0>=Sad_Sub_Limit) goto Go_4v;
          Sad4_0 = Metric_8x8(Cursor.Src + 8        , Rf+8, BpS);
          if (Sad4_0>=Sad_Sub_Limit) goto Go_4v;
          Rf += 8*BpS;
          Sad4_0 = Metric_8x8(Cursor.Src +     8*BpS, Rf,   BpS);
          if (Sad4_0>=Sad_Sub_Limit) goto Go_4v;
          Sad4_0 = Metric_8x8(Cursor.Src + 8 + 8*BpS, Rf+8, BpS);
          if (Sad4_0>=Sad_Sub_Limit) goto Go_4v;
          goto Done_4v;
        }
#endif
Go_4v:
        {
          SKL_UINT32 Sad4, Sad16, Saved_Sad = Cursor.Best_Sad;
          SKL_MV Saved_MV;
          SKL_COPY_MV( Saved_MV, MVs[0] );

          Cursor.Metric      = Metric_8x8;
          Cursor.Metric_Avrg = Metric_8x8_Avrg;

            // Cursor.Pred and Sub_Offset() are ok, here
//          Cursor.Set_MV_Predictor(&MVs[0], MV_Stride, 0);
          Sad16  = Cursor.Sub_Search(Map[0].MV, 0,0);
          if (Use_HPels) Sad4  = Cursor.Refine_MV_HPels(MVs[0], Subpixel_Precision, Rounding, _HPels);
          else           Sad4  = Cursor.Refine_MV_8x8(MVs[0], Subpixel_Precision, Rounding);
          SKL_ASSERT(Cursor.Is_In_Range(MVs[0]));
//          Set_Max(Maxs[1], MVs[0], Cursor.Pred);

          Cursor.Set_MV_Predictor(&MVs[1], MV_Stride, 1, ABits);
          Sad16 += Cursor.Sub_Search(Map[0].MV, 8,0);
          if (Use_HPels) Sad4 += Cursor.Refine_MV_HPels(MVs[1], Subpixel_Precision, Rounding, _HPels);
          else           Sad4 += Cursor.Refine_MV_8x8(MVs[1], Subpixel_Precision, Rounding);
          SKL_ASSERT(Cursor.Is_In_Range(MVs[1]));
//          Store_Max(Maxs[1], MVs[1], Cursor.Pred);

          Cursor.Set_MV_Predictor(&MVs[MV_Stride+0], MV_Stride, 2, ABits);
          Sad16 += Cursor.Sub_Search(Map[0].MV, 0,8);
          if (Use_HPels) Sad4 += Cursor.Refine_MV_HPels(MVs[MV_Stride+0], Subpixel_Precision, Rounding, _HPels);
          else           Sad4 += Cursor.Refine_MV_8x8(MVs[MV_Stride+0], Subpixel_Precision, Rounding);
          SKL_ASSERT(Cursor.Is_In_Range(MVs[MV_Stride+0]));
//          Store_Max(Maxs[1], MVs[MV_Stride+0], Cursor.Pred);

          Cursor.Set_MV_Predictor(&MVs[MV_Stride+1], MV_Stride, 3, ABits);
          Sad16 += Cursor.Sub_Search(Map[0].MV, 8,8);
          if (Use_HPels) Sad4 += Cursor.Refine_MV_HPels(MVs[MV_Stride+1], Subpixel_Precision, Rounding, _HPels);
          else           Sad4 += Cursor.Refine_MV_8x8(MVs[MV_Stride+1], Subpixel_Precision, Rounding);
          SKL_ASSERT(Cursor.Is_In_Range(MVs[MV_Stride+1]));
//          Store_Max(Maxs[1], MVs[MV_Stride+1], Cursor.Pred);

//          printf( "Sad4=%d Sad4_FP=%d Sad16=%d  (%d,%d)\n", Sad4, Sad16, Saved_Sad, Map[0].MV[0],Map[0].MV[1]);

          Sad4 += Sad4_Penalty;
          if (Sad4<Saved_Sad)
          {
            Map->Type = SKL_MAP_8x8;
            Map->Sad = Sad4;
            Update_Map_Data(Map, pMap, Sad16, MVs[0]);

//            Add_Max_MV_To_Stats(Maxs[1]);

            Add_To_Stats(MVs[0]);
            Add_To_Stats(MVs[1]);
            Add_To_Stats(MVs[MV_Stride+0]);
            Add_To_Stats(MVs[MV_Stride+1]);
          }
          else
            SKL_COPY_MV( MVs[0], Saved_MV );  // wasn't worth...

          Cursor.Set_Sub_Offset(0,0);
          // Warning: Cursor.Pred, .IPred, .Best_MV, .Best_Sad, Src, Ref and .Metric* are trashed here
        }
      }
Done_4v:
      if (Map->Type==SKL_MAP_16x16) {
Done_16x16:

//        Add_Max_MV_To_Stats(Maxs[0]);

        Add_To_Stats(MVs[0]);
        SKL_COPY_MV(MVs[          1], MVs[0]);
        SKL_COPY_MV(MVs[MV_Stride+0], MVs[0]);
        SKL_COPY_MV(MVs[MV_Stride+1], MVs[0]);
      }

Done:
      if (Interlace_DCT>0)
      {
          /* note: for INTER blocks, we should decide field DCT *after* the 
             predictions has been formed, but we expect it to be equally as 
             good as working directly on unpredicted frame source. */

        if (Interlace_DCT==2 || Field_DCT_Is_Better(Cursor.Src, Frame))
          Map->Flags |= 1;      // -> use Field_DCT
      }

#if 0
      Cursor.Check_Limits(MVs[0], "[post]:");
      Cursor.Check_Limits(MVs[1], "[post]:");
      Cursor.Check_Limits(MVs[MV_Stride+0], "[post]:");
      Cursor.Check_Limits(MVs[MV_Stride+1], "[post]:");
      Cursor.Check_Frame_Limits(MVs[0], "[flm]:");
#endif

      Map++;
      MVs += 2;
      if (pMVs!=0) {
        pMVs += 2;
        pMap++;
      }
      Cursor++;
    }
    MVs += 4 + MV_Stride;   // skip edge and next line
    if (pMVs!=0) pMVs += 4 + MV_Stride;
  }

  Frame->Img_Dsp->Switch_Off();
  Compile_Stats();    // Warning: this func uses floats. Call after EMMS.

    // last 30% before reaching forced key-frame?
    // => we artificially raise the perceived number of intras
  int Scale = 100;
  const int dCount = 30*Intra_Max - 100*Intra_Count;
  if (dCount>0)
    Scale += Scale*dCount/(30*Intra_Max);

  if (Nb_Intras * Scale > Intra_Limit * Nb_Relevant)
  {
    Intra_Count = -1;  // real scene change
//    printf( "Scene change! Nb:%d / %d  (/%d)  =>%.1f%%\n", Nb_Intras, Nb_Relevant, Frame->MB_W*Frame->MB_H, 100.f*Nb_Intras/Nb_Relevant);
  }
//  fprintf( stderr, "Miss:%d, Hit:%d (%.2f%%)\n", Cursor.Miss, Cursor.Hit, 100.f*Cursor.Hit/(Cursor.Miss+Cursor.Hit) );
}

//////////////////////////////////////////////////////////
//  GMC pass (experimental)
//////////////////////////////////////////////////////////

#define GMC_RND(x)  ( (x)<0. ? -(int)( -(x)+.5f ) : (int)( (x)+.5f ) )
#define GMC_DBG(S)  // printf( S )

void SKL_MP4_ANALYZER_I::Global_GMC_Pass(SKL_MP4_INFOS * const Frame)
{
  GMC_Mode &= ~1;

  if (Reduced_Frame>0)
    return;    // no GMC in reduced resolution VOP

  int Nb_Smpl, i, j;
  const int MB_W = Frame->MB_W;
  const int MB_H = Frame->MB_H;
  const int MV_Up = 2*Frame->MV_Stride;
  const int BpS   = Frame->BpS;
  int (*Smpl)[2] = (int (*)[2]) alloca(MB_W*MB_H*2*sizeof(int));
  SKL_BYTE *Scratch = (SKL_BYTE*) alloca(16*BpS);
  SKL_MP4_MAP *Map;
  SKL_MV *MVs;

  SKL_UINT32 GMC_SAD_PENALTY;  // small mv penalty (that's the point of GMC!!)
  SKL_UINT32 GMC_AUTO_SAD_LIMIT;

  METRIC Metric_16x16;
  METRIC_DEV Metric_Dev;
  switch( Search_Metric ) {
    case 2:
      Metric_16x16 = Frame->Img_Dsp->Hadamard_SAD_16x16;
      Metric_Dev   = Frame->Img_Dsp->Hadamard_Dev_16x16;
      GMC_AUTO_SAD_LIMIT = 150;
      GMC_SAD_PENALTY = 4;
    break;
    case 1:
      Metric_16x16 = Frame->Img_Dsp->SSD_16x16;
      Metric_Dev   = Frame->Img_Dsp->Sqr_Dev_16x16;
      GMC_AUTO_SAD_LIMIT = 500;
      GMC_SAD_PENALTY = 4;
    break;
    default:
      Metric_16x16 = Frame->Img_Dsp->SAD_16x16;
      Metric_Dev   = Frame->Img_Dsp->Abs_Dev_16x16;
      GMC_AUTO_SAD_LIMIT = 700;
      GMC_SAD_PENALTY = 4;
    break;
  }
  const int LAPL_LIMIT  = 14;

  float Scale = 1.f * (1<<(3-GMC_Accuracy));
  if (Subpixel_Precision==2) Scale *= 0.5f;

  Nb_Smpl = 0;
  MVs = Frame->Cur->MV;
  Map = Frame->Cur->Map;

  for(j=0; j<MB_H; ++j) {
    for(i=0; i<MB_W; ++i) {
      if (Map[i].Type!=SKL_MAP_16x16 && Map[i].Type!=SKL_MAP_8x8)   // not INTRA or SKIPPED
        { GMC_DBG( "." ); continue; }

      const int vx0 = MVs[2*i][0];
      const int vy0 = MVs[2*i][1];
      int Lapl_x = 0, Lapl_y = 0;

      if (i>0) {
        Lapl_x += MVs[2*i-1][0] - vx0;
        Lapl_y += MVs[2*i-1][1] - vy0;
      }
      if (i<MB_W-1) {
        Lapl_x += MVs[2*i+1][0] - vx0;
        Lapl_y += MVs[2*i+1][1] - vy0;
      }
      if (j>0) {
        Lapl_x += MVs[2*i-Frame->MV_Stride][0] - vx0;
        Lapl_y += MVs[2*i-Frame->MV_Stride][1] - vy0;
      }
      if (j<MB_H-1) {
        Lapl_x += MVs[2*i+Frame->MV_Stride][0] - vx0;
        Lapl_y += MVs[2*i+Frame->MV_Stride][1] - vy0;
      }

      if (ABS(Lapl_x)+ABS(Lapl_y)>LAPL_LIMIT)
        { GMC_DBG( "-" ); continue; }

      const SKL_BYTE * const Src = Frame->Cur->Y + i*16 + j*16*BpS;
      SKL_UINT32 Sad;
#if 1
      Sad  = (i<MB_W-1) ? Metric_16x16(Src, Src+1, BpS) : Metric_16x16(Src, Src-1, BpS);
      Sad += (j<MB_H-1) ? Metric_16x16(Src, Src+BpS, BpS) : Metric_16x16(Src, Src-BpS, BpS);
#else
      Sad =  Metric_Dev(Src, BpS);    // enough textured?
#endif
      if (Sad<GMC_AUTO_SAD_LIMIT) { GMC_DBG( "=" ); continue; }

      Smpl[Nb_Smpl][0] = i;
      Smpl[Nb_Smpl][1] = j;
      Nb_Smpl++;
      GMC_DBG( "*" );
    }
    GMC_DBG("\n");
    MVs += MV_Up;
    Map += MB_W;
  }
  GMC_DBG( "\n" );

  Frame->Img_Dsp->Switch_Off();

    // Note:
    //
    // We're about to least-square fit the 6-parameters model:
    //
    //    mvx(x,y) = vx0 + a.x + b.y
    //    mvy(x,y) = vy0 + c.x + d.y
    //
    // that globally interpolates the motion vector (mvx,mvy) at point x,y.
    // The unknowns are: vx0,vy0, a,b,c,d. The mapping toward the ISO warp
    // points is pretty straightforward. We always use 3 warp points, leaving
    // the task of simplifying the sprite motion to the SKL_GMC_DSP module.
    //
    // Now, minimizing the overall error E = < (vx-mvx)^2 + (vy-mvy)^2 >
    // where (vx,vy) are the result of the previous ME pass, and filtering
    // out the seemingly outliers, gives the set of equations:
    //
    //   vx0 = <vx> - a.<x> - b.<y>
    //   vy0 = <vy> - c.<x> - d.<y>
    //   {x.vx} = a.{xx} + b.{xy}
    //   {y.vx} = a.{xy} + b.{yy}
    //   {x.vy} = c.{xx} + d.{xy}
    //   {y.vy} = c.{xy} + d.{yy}
    //
    // where <f> is the sampled average of quantity 'f', and {fg} is the 
    // deviates of f*g, that is: {fg} = <fg> - <f>.<g>.
    // Inverting this system easily gives:
    //
    //   Det = {yy}.{xx} - {xy}.{xy}.
    //   a = ( {x.vx}.{yy} - {y.vx}{xy} ) / Det
    //   b = ( {y.vx}.{xx} - {x.vx}{xy} ) / Det
    //   c = ( {x.vy}.{yy} - {y.vy}{xy} ) / Det
    //   d = ( {y.vy}.{xx} - {x.vy}{xy} ) / Det
    //
    // with some special degenerate cases.
    // Of course, vx0,vy0 are deduced after a,b,c,d are known.
    //

  float a=0.,b=0.,c=0.,d=0., vx0=0., vy0=0.;    // params
  float W;
  while(Nb_Smpl>=8)
  {
    a=0.; b=0.; c=0.; d=0.;
    vx0=0.; vy0=0.;
    W = 0.;
    float x=0., y=0., Vx = 0., Vy=0.;           // stats
    float xx=0., yy=0., xy=0.;
    float xVx=0., yVx=0., xVy=0., yVy=0.;

    Map = Frame->Cur->Map; 
    MVs = Frame->Cur->MV;
    int n;
    for(n=0; n<Nb_Smpl; ++n) {
      i = Smpl[n][0]; j = Smpl[n][1];
      const float xo = 16.f*i;
      const float yo = 16.f*j;
      const float vxo = (float)MVs[2*i+j*MV_Up][0];
      const float vyo = (float)MVs[2*i+j*MV_Up][1];
      SKL_ASSERT(Map[i + j*MB_W].Sad>0);
      const float w = 1.f / Map[i + j*MB_W].Sad;
// flat weighting:      const float w = 1.f;
      W  += w;
      x  += w*xo;      y  += w*yo;
      Vx += w*vxo;     Vy += w*vyo;
      xx += w*xo*xo;   yy += w*yo*yo;   xy += w*xo*yo;
      xVx += w*xo*vxo; xVy += w*xo*vyo;
      yVx += w*yo*vxo; yVy += w*yo*vyo;

    }
// flat weighting:   W = 1.f*n;
    xx  = xx*W - x*x;   xy  = xy*W - x*y;   yy = yy*W - y*y;
    xVx = xVx*W - x*Vx; xVy = xVy*W - x*Vy;
    yVx = yVx*W - y*Vx; yVy = yVy*W - y*Vy;

    float Det = xx*yy - xy*xy;
    if (fabs(Det)<1.e-3f) { // data aligned?!?
      // printf( "{xx}:%.6f  {yy}:%.6f  {xy}:%6f\n", xx, yy, xy );
      if (fabs(xy)<1.e-3f) {
          // => either xx or yy is 0.
        if (fabs(xx)>=1.e-3f) {
          Det = Scale / xx;
          a = xVx * Scale;
          b = 0.;
          c = xVy * Scale;
          d = 0.;
        }
        else if (fabs(yy)>=1.e-6f) {
          Det = Scale / yy;
          a = 0.;
          b = yVx * Scale;
          c = 0.;
          d = yVy * Scale;
        }
        else { a = b = c = d = 0.; }
      }
      else {      // {xy}.{xy} = {xx}.{yy}. Data are aligned.
        a = b = c = d = 0.;
      }
    }
    else {
      Det = Scale / Det;
      a = ( xVx*yy - yVx*xy ) * Det;
      b = ( yVx*xx - xVx*xy ) * Det;
      c = ( xVy*yy - yVy*xy ) * Det;
      d = ( yVy*xx - xVy*xy ) * Det;
    }

    W = 1.f / W;
    vx0 = ( Scale*Vx - a*x - b*y ) * W;
    vy0 = ( Scale*Vy - c*x - d*y ) * W;

#if 0
    vx0 = 0.5f*GMC_RND(2.f*vx0);
    vy0 = 0.5f*GMC_RND(2.f*vy0);
    a = (0.5f/Frame->Width )*GMC_RND( a*2.f*Frame->Width  );
    c = (0.5f/Frame->Width )*GMC_RND( c*2.f*Frame->Width  );
    b = (0.5f/Frame->Height)*GMC_RND( b*2.f*Frame->Height );
    d = (0.5f/Frame->Height)*GMC_RND( d*2.f*Frame->Height );
#endif

//    printf( "Nb Smpl:%d/%d  Params:vx0=%.3f vy0=%.3f a=%.3f b=%.3f c=%.3f d=%.3f W=%3f\n", Nb_Smpl, MB_W*MB_H, vx0, vy0, a,b, c,d, W );

    float dVx = 0., dVy = 0.;
    for(n=0; n<Nb_Smpl; ++n) {
      i = Smpl[n][0]; j = Smpl[n][1];
      const float x = 16.f*i;
      const float y = 16.f*j;
      Vx = vx0 + a*x + b*y;
      Vy = vy0 + c*x + d*y;
//      printf( "%3f/%3f %3f/%3f\n", Vx, Scale*MVs[2*i + j*MV_Up][0], Vy, Scale*MVs[2*i + j*MV_Up][1] );
      Vx -= Scale*MVs[2*i + j*MV_Up][0];
      Vy -= Scale*MVs[2*i + j*MV_Up][1]; 
      dVx += ABS(Vx);
      dVy += ABS(Vy);
    }
    dVx /= Nb_Smpl; dVy /= Nb_Smpl;
    if (dVx<0.3f) dVx = 0.3f;
    if (dVy<0.3f) dVy = 0.3f;

    int Nb0 = Nb_Smpl;
    n = 0;
    while(n<Nb_Smpl)
    {
      i = Smpl[n][0]; j = Smpl[n][1];
      const float x = 16.f*i;
      const float y = 16.f*j;
      Vx = vx0 + a*x + b*y - Scale*MVs[2*i + j*MV_Up][0];
      Vy = vy0 + c*x + d*y - Scale*MVs[2*i + j*MV_Up][1];
      if (ABS(Vx)>dVx || ABS(Vy)>dVy) {
        --Nb_Smpl;
        Smpl[n][0] = Smpl[Nb_Smpl][0];
        Smpl[n][1] = Smpl[Nb_Smpl][1];
      }
      else n++;
    }
      
//    printf( "=> Nb_Smpl:%d/%d   Box: dVx=%f, dVy=%f\n", Nb_Smpl, Nb0, dVx, dVy );

    if (Nb_Smpl<8 || Nb_Smpl==Nb0)
      break;
  }

  if ( (Nb_Smpl<8) && ((GMC_Mode&2)==0) )
    return;  // Status quo not reached

    // ok, we seem to have some global motion params available.
    // Time to clean things up.

  GMC_Warps[0][0] = GMC_RND( vx0 );              SKL_ASSERT(GMC_Warps[0][0]>=-32768 && GMC_Warps[0][0]<=32767);
  GMC_Warps[0][1] = GMC_RND( vy0 );              SKL_ASSERT(GMC_Warps[0][0]>=-32768 && GMC_Warps[0][0]<=32767);
  GMC_Warps[1][0] = GMC_RND( a*Frame->Width  );  SKL_ASSERT(GMC_Warps[1][0]>=-32768 && GMC_Warps[1][0]<=32767);
  GMC_Warps[1][1] = GMC_RND( c*Frame->Width  );  SKL_ASSERT(GMC_Warps[1][1]>=-32768 && GMC_Warps[1][1]<=32767);
  GMC_Warps[2][0] = GMC_RND( b*Frame->Height );  SKL_ASSERT(GMC_Warps[2][0]>=-32768 && GMC_Warps[2][0]<=32767);
  GMC_Warps[2][1] = GMC_RND( d*Frame->Height );  SKL_ASSERT(GMC_Warps[2][1]>=-32768 && GMC_Warps[2][1]<=32767);
//  printf( " => (%d,%d) (%d,%d) (%d,%d)\n", GMC_Warps[0][0], GMC_Warps[0][1], GMC_Warps[1][0], GMC_Warps[1][1], GMC_Warps[2][0], GMC_Warps[2][1] );

  Frame->GMC_Dsp->Setup(Frame->Width, Frame->Height, GMC_Warps, GMC_Pts, GMC_Accuracy);
  Map = Frame->Cur->Map; 
  const SKL_BYTE * const Ref = Frame->Past->Y;
  for(j=0; j<MB_H; ++j) {
    for(i=0; i<MB_W; ++i) {
      if (Map[i].Type==SKL_MAP_SKIPPED) {
          // a SKIPPED in S-VOP is not a allowed -> We promote to INTER + zero-MV
        Map[i].Type = SKL_MAP_16x16;
        SKL_ASSERT( SKL_IS_ZERO_MV(Frame->Cur->MV[2*i + j*MV_Up]) );
        GMC_DBG( "." );
      }
      else if (Map[i].Type==SKL_MAP_16x16 || Map[i].Type==SKL_MAP_8x8)
      {
        const SKL_BYTE * const Src = Frame->Cur->Y + i*16 + j*16*BpS;

        Frame->GMC_Dsp->Predict_16x16(Frame->GMC_Dsp, Scratch, Ref, BpS, i,j, Rounding);
        SKL_UINT32 Sad = Metric_16x16(Src, Scratch, BpS) + GMC_SAD_PENALTY;

        // MVs = Frame->Cur->MV + 2*i + j*MV_Up;
        // int amv[2]; Frame->GMC_Dsp->Get_Average_MV(amv, i,j, Subpixel_Precision==2);
        // printf( "(pts:%d) %d %d [%d] (%d,%d) vs [%d] (%d,%d)\n", GMC_Pts, i,j, Sad, amv[0], amv[1], Map->Sad, MVs[0][0], MVs[0][1] );

        if (Sad<=Map->Sad) {
          Map[i].Type = SKL_MAP_GMC;
          Map[i].Sad = Sad;
          GMC_DBG( "g" );
        }
        else {
          GMC_DBG( "-" );
        }
      }
    }
    GMC_DBG( "\n" );
    Map += MB_W;
  }

  Frame->Img_Dsp->Switch_Off();
  GMC_DBG( "--------------------------------------------------\n" );

  GMC_Mode |= 1;
}

#undef GMC_RND

//////////////////////////////////////////////////////////
// Main analyzer call
//////////////////////////////////////////////////////////

  // PROTOCOL: this function should Populate MVs[] and Map[] (.Type, Flags, ...)
  // return values:
  //   0: intra coding is required (I-VOP)  
  //   1 or 2: predictive coding required (1=P or 2=B-VOP)
  //   3: GMC coding requested
  // These are NOT the definitive coding. Encoder might change
  // it just after, because of some syntax constraints.

int SKL_MP4_ANALYZER_I::Analyze(SKL_MP4_INFOS * const Frame)
{
  if (Pass.Get_Pass_Nb()==2) {
    if (Pass.Open_Read_Pass_File(Frame->Cur->Width, Frame->Cur->Height))
    {
      SKL_FRAME_STATS Stat;  
      Stat.Frame_Order = Frame->Frame_Number;
      if (Pass.Get_Next_Frame_Params(&Stat))
      {
        if (Stat.Pic_Type==0)   // Intra?
        {
          Set_Param("quant", Stat.Q);
          Set_Param("base-quant", Stat.Q);
  //        Set_Param("search-size", Stat.FCode);
        }
      }
    }
  }

  if (Reduced_Frame==0 || Reduced_Frame==1) {
    // time to take a decision about reduced frame.
    // Do it *before* the global pass
    Reduced_Frame ^= 1;   // wow. What a smart guess! :)
  }

  Rounding ^= 1;    // for P/S_VOP

  if (Intra_Count>=0) Global_Pass(Frame);

  int Coding;  
  if (Intra_Count==-1) {
    Scene_Changed();
    Reset_Frame(Frame->Cur, Frame);
    Intra_Count = Intra_Max;
    Coding = 0;                      // I_VOP
  }
  else
  {
    Coding = 1; // P_VOP

    if (GMC_Mode>=0) {               // examine GMC?
      Global_GMC_Pass(Frame);
      if (GMC_Mode>0) Coding = 3;    // S_VOP
    }
      
    if (--Intra_Count<=0) {
      Intra_Count = Intra_Max;
      Coding = 0;                    // I_VOP
    }
  }

  return Coding;
}

//////////////////////////////////////////////////////////
// Luminance masking
//////////////////////////////////////////////////////////

static void Print_dQ(const SKL_MP4_INFOS * const Infos,
                     float *Qv, float Quant, int Size)
{
  int Incr = (Size==Infos->MB_W*Infos->MB_W) ? 1 : 2;
  printf( "-----Q=%f-----\n", Quant );
  for(int y=0; y<Infos->MB_H; y+=Incr) {
    for(int x=0; x<Infos->MB_W; x+=Incr) {
      float dQ = *Qv++;
      printf( (dQ<0) ? "[%.2f]" : "[ %.2f]", dQ);
    }
    printf( "\n" );
  }
}

static float Regularize_dQ(SKL_MP4_MAP *Map, const SKL_MP4_INFOS * const Info,
                           float *Qv, int Size, int BVOP)
{
  int i, Keep_On;
//  Print_dQ(Info, Qv, Qv[0], Size);
  do {
    Keep_On = 0;
    i = Size;
    while(i-->0) {
      float dQo, dQ1;
      dQo = Qv[i  ]+.5f;
      dQ1 = Qv[i+1]+.5f;
      if (dQo<dQ1 - 2.f) {
        dQo += .5f;
        if (dQo>=31.5f) dQo = 31.4999f;
        Qv[i] = dQo;
      }
      else if (dQo>dQ1 + 2.f) {
        dQ1 += .5f;
        if (dQo>=31.5f) dQo = 31.4999f;
        Qv[i+1] = dQ1;
      }
      else if ( BVOP && (dQo>dQ1 - 0.5f && dQo<dQ1+0.5f) )
      {
        // -2,0,+2 deltas allowed only
        // TODO: Check it's correct!
        dQo += .5f;
        dQ1 -= .5f;
        if (dQo>=31.5f) dQo = 31.4999f;
        if (dQ1<=0.5f) dQ1 = 0.5f;
        Qv[i] = dQo;
        Qv[i+1] = dQ1;
      }
      else if (dQo<0.5f) Qv[i] = 0.5f;
      else if (dQo>=31.5f) Qv[i] = 31.4999f;
      else continue;
      Keep_On = 1;
    }
  }
  while(Keep_On);

  float New_Quant = Qv[0];  
  int Base_Quant = RND(Qv[0]);
  for(i=0; i<Size; ++i)
  {
    int dQ = RND(Qv[i]) - Base_Quant;
    if (BVOP) dQ = dQ/2*2;
    Base_Quant += dQ;
    SKL_ASSERT( Base_Quant>=1.f && Base_Quant<=31.f);
    SKL_ASSERT( dQ<=2 && dQ>=-2);
    Qv[i] = (float)dQ;
  }
//  Print_dQ(Info, Qv, New_Quant, Size);
  return New_Quant;
}

float SKL_MP4_ANALYZER_I::Mask_Luminance(const SKL_MP4_INFOS * const Infos,
                                       int For_BVOP) const
{
  int i, x, y;
  int Size = Infos->MB_W*Infos->MB_H;
  if (Reduced_Frame>=1) Size /= 4;
  void *Work = alloca(Size*sizeof(int)*2);  // ~9k @ 640x480
  SKL_MP4_MAP *Map;

    // we split computations in two pass, with MMX only during the first one
  SKL_BYTE *Src = Infos->Cur->Y;
  unsigned int (*AB)[2] = (unsigned int (*) [2])Work;
  Map = Infos->Cur->Map;

    // we exclude SKIPPED or 4V macroblocks from the to-be-regularized
    // array. They are incompatible with dQuant.
  if (Reduced_Frame<1)
  {
    for(y=0; y<Infos->MB_H; y++)
    {
      for(x=0; x<Infos->MB_W; x++)
      {
        if (Map[x].Type!=SKL_MAP_SKIPPED && Map[x].Type!=SKL_MAP_8x8) {
          AB[0][0] = Infos->Img_Dsp->Mean_16x16(Src + x*16, Infos->BpS);
          AB[0][1] = Infos->Img_Dsp->Sqr_16x16 (Src + x*16, Infos->BpS);
          AB++;
        }
        else Size--;  // else: don't store.
      }
      Src += 16*Infos->BpS;
      Map += Infos->MB_W;
    }
  }
  else {
    SKL_BYTE *Src2 = Src + 16*Infos->BpS;
    for(y=0; y<Infos->MB_H; y+=2)
    {
      for(x=0; x<Infos->MB_W; x+=2)
      {
        if (Map[x].Type!=SKL_MAP_SKIPPED && Map[x].Type!=SKL_MAP_8x8) {
          AB[0][0] = Infos->Img_Dsp->Mean_16x16(Src  + x*16,      Infos->BpS)
                   + Infos->Img_Dsp->Mean_16x16(Src  + x*16 + 16, Infos->BpS)
                   + Infos->Img_Dsp->Mean_16x16(Src2 + x*16,      Infos->BpS)
                   + Infos->Img_Dsp->Mean_16x16(Src2 + x*16 + 16, Infos->BpS);
          AB[0][1] = Infos->Img_Dsp->Sqr_16x16 (Src  + x*16,      Infos->BpS)
                   + Infos->Img_Dsp->Sqr_16x16 (Src  + x*16 + 16, Infos->BpS)
                   + Infos->Img_Dsp->Sqr_16x16 (Src2 + x*16,      Infos->BpS)
                   + Infos->Img_Dsp->Sqr_16x16 (Src2 + x*16 + 16, Infos->BpS);
          AB++;
        }
        else Size--;  // else: don't store.
      }
      Src  += 32*Infos->BpS;
      Src2 += 32*Infos->BpS;
      Map  += 2*Infos->MB_W;
    }
  }
  Infos->Img_Dsp->Switch_Off();

  float *Qv = (float*)Work;    // wow! HACK: we re-use the int array for floats...
  AB = (unsigned int (*) [2])Work;
  SKL_ASSERT(sizeof(float)<=2*sizeof(int));

  const float Epsilon = 0.05f;
  for(i=0; i<Size; ++i)
  {
      // simple criterion used:
      //  A = Avrg(Y)^2
      //  B = Avrg(Y^Y) - A
      //  if (B<Epsilon*A) -> dQ = dQuant_Amp.(1-sqrt(B/(Epsilon*A)))

    unsigned int Ym2 = AB[i][0];
    unsigned int Y2m = AB[i][1];
    Ym2 = Ym2*Ym2;
    float A = Epsilon*Ym2;
    float B = 1.0f*(Y2m-Ym2);

    SKL_ASSERT(B>=0. && A>=0.);
    Qv[i] = Q_Base;   // common lower value
    if (B<A) {
//        float dQ = (float)( dQuant_Amp*dQ_Tab[ (int)(255.f*B/A) ];
//        float dQ = (float)( dQuant_Amp*(exp(-1.2*sqrt(B/A)) ) );
      float dQ = (float)( dQuant_Amp*(1.-sqrt(B/A)) );
      Qv[i] += dQ;
    }
  }

  Qv = (float*)Work;
  float New_Quant = Regularize_dQ(Infos->Cur->Map, Infos, Qv, Size, For_BVOP);
  Map = Infos->Cur->Map;
  i = 0;
  y = 0;
  while(y<Infos->MB_H) {
    if (Reduced_Frame<1) {
      for(x=0; x<Infos->MB_W; x++) {
        if (Map[x].Type!=SKL_MAP_SKIPPED && Map[x].Type!=SKL_MAP_8x8)
          Map[x].dQ = (char)Qv[i++];
        else
          Map[x].dQ = 0;
      }
      y += 1;
      Map += Infos->MB_W;
    }
    else {
      for(x=0; x<Infos->MB_W; x+=2) {
        if (Map[x].Type!=SKL_MAP_SKIPPED && Map[x].Type!=SKL_MAP_8x8)
          Map[x].dQ = (char)Qv[i++];
        else
          Map[x].dQ = 0;
      }
      y += 2;
      Map += 2*Infos->MB_W;
    }
  }
  SKL_ASSERT(i==Size);
  return New_Quant;    
}

void SKL_MP4_ANALYZER_I::Analyze_dQ(SKL_MP4_INFOS * const Infos, int For_BVOP)
{
    //
    //   PROTOCOL
    //
    // Do whatever you want for luminance masking, provided that:
    //
    // * Map[].dQ is updated and in range {-2,-1,0,1,2} for
    //   non-BVOP, or {-2,0,2} for BVOP (according to For_BVOP flag).
    // * Moreover, for each block, the overall cumulated Quantizer
    //   value should be in range [1,31]. This is not necessarly
    //   checked by the encoder (only through assert()).
    // * You update the initial quantizer (Q_Cur), so that the next
    //   call to Get_Param( "quant", ...) returns its value.
    //

  if (Luminance_Masking) Q_Cur = Mask_Luminance(Infos, For_BVOP);
  else                   Q_Cur = Q_Base;
}

//////////////////////////////////////////////////////////
// SKL_MP4_ANALYZER_I
//////////////////////////////////////////////////////////

SKL_MP4_ANALYZER_I::SKL_MP4_ANALYZER_I(SKL_MEM_I *Mem)
  : SKL_MP4_ANALYZER(Mem)
  , Pass(0)
  , _Aux_Pic_Size(0)
  , _Aux_Data(0)
{
  _HPels[0] = 0;  // no ref frame selected so far.
  _HPels[1] = _HPels[2] = _HPels[3] = 0;    // sanity check

  Set_Param( "quant", 5.f );
  Set_Param( "quant-type", 1 );        // default: MPEG4  
  Set_Param( "interlace-field", 0 );   // default: none
  Set_Param( "interlace-dct", 0 );     // default: none
  Set_Param( "search-size", 2 );       // 32pxl-wide
  Set_Param( "search-size-bwd", 2 );   // TODO!!
  Set_Param( "base-quant", 2 );        // default
  Set_Param( "base-search-size", 1 );  // 0 = don't update
  Set_Param( "intra-limit", 20 );      // 20%
  Set_Param( "inter-threshold", 2 );   // inter limit on SAV
  Set_Param( "lambda", 1.f );          // Default value
  Set_Param( "sad-skip-limit", 256 );  // => SKIP
  Set_Param( "sad-intra-limit", 4000 );// => INTRA
  Set_Param( "search-metric", 0 );     // Eval = SAD
  Set_Param( "4v-probing", 0 );        // default: off
  Set_Param( "search-method", 0 );     // phods
  Set_Param( "luminance-masking", 0 ); // off
  Set_Param( "dquant-amp", 5.0f );     // default
  Set_Param( "subpixel", 1 );          // default: half-pel
  Set_Param( "rounding", 0 );          // default
  Set_Param( "bitrate", 0 );           // CBR
  Set_Param( "framerate", 29.97f );    // default
  Set_Param( "frequency", 30 );        // default
  Set_Param( "frame-ticks", 1 );       // default
  Set_Param( "intra-max-delay", 100 ); // ~every 3s at 30fps
  Set_Param( "intra-max-count", -1 );  // will start with a scene-change/I-VOP
  Set_Param( "bframe", 0 );            // no b-frame
  Set_Param( "gmc-mode", -1 );         // no GMC (-1 = disabled)
  Set_Param( "gmc-pts",  3 );          // always use 3 points
  Set_Param( "gmc-accuracy", 3 );      // 1/16th pel
  Set_Param( "reduced-frame", -1 );    // disabled
  Set_Param( "hi-mem",         1 );    // Default: Use additional memory to speed things up
  Set_Param( "buffer-size",    0 );    // disabled.
  Set_Param( "verbose", 0 );           // shut up
  Set_Param( "pass", 0 );              // no 2pass
  Set_Param( "passfile", "pass1.log" );
  Set_Param( "passrf", 0.30f );        // reaction factor

  Scene_Changed();
  Compile_Stats();  // just for init sanity
}

SKL_MP4_ANALYZER_I::~SKL_MP4_ANALYZER_I() {
  Free_Aux();
}

void SKL_MP4_ANALYZER_I::Wake_Up(SKL_MP4_ANALYZER *Previous) {}
void SKL_MP4_ANALYZER_I::Shut_Down() {}

SKL_MEM_I *SKL_MP4_ANALYZER_I::Set_Memory_Manager(SKL_MEM_I *New_Mem)
{
  SKL_MEM_I *Old_Mem = _Mem;
  _Mem = New_Mem;
  return Old_Mem;
}

//////////////////////////////////////////////////////////

void SKL_MP4_ANALYZER_I::Alloc_Aux(size_t Pic_Size) {
  Pic_Size += SKL_ALIGN;
  Free_Aux();
  if (_Mem==0) {
    _Aux_Data = (SKL_BYTE*)malloc(Pic_Size);
    if (_Aux_Data==0)
      Skl_Throw( SKL_MEM_EXCEPTION("_Aux_Data", Pic_Size) );
  }
  else _Aux_Data = (SKL_BYTE*)_Mem->New(Pic_Size);

  _Aux_Pic_Size = Pic_Size;
}

void SKL_MP4_ANALYZER_I::Free_Aux() {
  if (_Aux_Data!=0) {
    if (_Mem==0) free(_Aux_Data);
    else _Mem->Delete(_Aux_Data, _Aux_Pic_Size);
    _Aux_Data = 0;
    _Aux_Pic_Size = 0;
  }
}

void SKL_MP4_ANALYZER_I::Set_Half_Pels(const SKL_MP4_INFOS * const Infos,
                                     const SKL_MP4_PIC * const Ref)
{
  if (_HPels[0]==Ref->Y)    // ref frame already selected / filtered?
    return;

  size_t Pic_Size = (Infos->MB_H+2)*(Infos->MB_W+2) *16*16 *sizeof(SKL_BYTE);
  if (3*Pic_Size!=_Aux_Pic_Size) {
    Alloc_Aux(3*Pic_Size);
    int Offset = 16 + 16*Ref->BpS;
    _HPels[1] = (SKL_BYTE*)SKL_ALIGN_PTR(_Aux_Data + 0*Pic_Size + Offset, SKL_ALIGN);
    _HPels[2] = (SKL_BYTE*)SKL_ALIGN_PTR(_Aux_Data + 1*Pic_Size + Offset, SKL_ALIGN);
    _HPels[3] = (SKL_BYTE*)SKL_ALIGN_PTR(_Aux_Data + 2*Pic_Size + Offset, SKL_ALIGN);
  }
  _HPels[0] = Ref->Y;

  const SKL_MB_FUNCS *Funcs = Infos->MB_Dsp->Copy[Rounding];
  int x, y;
  if (Subpixel_Precision==2)
  {
    for(y=-1; y<=Infos->MB_H; ++y)
      for(x=-1; x<=Infos->MB_W; ++x) {
        const int Offset = (y*Ref->BpS + x) * 16;
        Funcs->H_Pass(_HPels[1]+Offset, _HPels[0]+Offset, 16, Ref->BpS);
        Funcs->V_Pass(_HPels[2]+Offset, _HPels[0]+Offset, 16, Ref->BpS);
      }
    for(y=-1; y<=Infos->MB_H; ++y)
      for(x=-1; x<=Infos->MB_W; ++x) {
        const int Offset = (y*Ref->BpS + x) * 16;
        Funcs->V_Pass(_HPels[3]+Offset, _HPels[1]+Offset, 16, Ref->BpS);
      }
  }
  else if (Subpixel_Precision==1)
  {
    for(y=-1; y<=Infos->MB_H; ++y)
      for(x=-1; x<=Infos->MB_W; ++x) {
        const int Offset = (y*Ref->BpS + x) * 16;
        Funcs->HP_16x8[1](_HPels[1]+Offset,       _HPels[0]+Offset,       Ref->BpS);
        Funcs->HP_16x8[1](_HPels[1]+Offset+8*Ref->BpS, _HPels[0]+Offset+8*Ref->BpS, Ref->BpS);

        Funcs->HP_16x8[2](_HPels[2]+Offset,       _HPels[0]+Offset,       Ref->BpS);
        Funcs->HP_16x8[2](_HPels[2]+Offset+8*Ref->BpS, _HPels[0]+Offset+8*Ref->BpS, Ref->BpS);

        Funcs->HP_16x8[3](_HPels[3]+Offset,       _HPels[0]+Offset,       Ref->BpS);
        Funcs->HP_16x8[3](_HPels[3]+Offset+8*Ref->BpS, _HPels[0]+Offset+8*Ref->BpS, Ref->BpS);
      }
  }
#if 0
  else
  {
    const SKL_HV_FILTER * const Filter = Infos->MB_Dsp->Filter;
    for(y=-1; y<=Infos->MB_H; ++y) {
      for(x=-1; x<=Infos->MB_W; ++x) {
        const int Offset = (y*Ref->BpS + x) * 16;
        Filter->H_Pass( _HPels[1]+Offset, _HPels[0]+Offset, Ref->BpS);
        Filter->V_Pass( _HPels[2]+Offset, _HPels[0]+Offset, Ref->BpS);
        Filter->HV_Pass(_HPels[3]+Offset, _HPels[0]+Offset, Ref->BpS);
      }
    }
  }
#endif
}

//////////////////////////////////////////////////////////

void SKL_MP4_ANALYZER_I::Reset_Stats() {
  Nb_MV_Stat    = 1;   // always include (0,0) in stats...
  MVx_Mean_Stat = 0;
  MVy_Mean_Stat = 0;
  MVx_SSE_Stat  = 0;
  MVy_SSE_Stat  = 0;
  MV_Max_Stat   = 0;
  dMV_Max_Stat  = 0;
}

void SKL_MP4_ANALYZER_I::Add_To_Stats(const SKL_MV v)
{
  if      ( v[0]>MV_Max_Stat) MV_Max_Stat =  v[0];
  else if (-v[0]>MV_Max_Stat) MV_Max_Stat =  v[0]^-1;   // equiv. to: " = -v[0]-1"
  if      ( v[1]>MV_Max_Stat) MV_Max_Stat =  v[1];
  else if (-v[1]>MV_Max_Stat) MV_Max_Stat =  v[1]^-1;
    // Note: we shift by 2 to prevent 32b overflows
  MVx_Mean_Stat += v[0]; MVx_SSE_Stat += (v[0]*v[0])>>2;
  MVy_Mean_Stat += v[1]; MVy_SSE_Stat += (v[1]*v[1])>>2;

  Nb_MV_Stat++;
}

void SKL_MP4_ANALYZER_I::Set_Max(SKL_MV Max, const SKL_MV v, const SKL_MV Pred)
{
  int V;
  V = v[0] - Pred[0];
  if (V<0) V ^= 1;
  Max[0] = V;
  V = v[1] - Pred[1];
  if (V<0) V ^= 1;
  Max[1] = V;
}
void SKL_MP4_ANALYZER_I::Store_Max(SKL_MV Max, const SKL_MV v, const SKL_MV Pred)
{
  int V;
  V = v[0] - Pred[0];
  if (V<0) V ^= 1;
  if (V>Max[0]) Max[0] = V;
  V = v[1] - Pred[1];
  if (V<0) V ^= 1;
  if (V>Max[1]) Max[1] = V;
}

void SKL_MP4_ANALYZER_I::Add_Max_MV_To_Stats(const SKL_MV v)
{
  if (v[0]>dMV_Max_Stat) dMV_Max_Stat = v[0];
  if (v[1]>dMV_Max_Stat) dMV_Max_Stat = v[1];
}

void SKL_MP4_ANALYZER_I::Compile_Stats()
{
    // Note: factor 4 is to compensate the '>>2' in Add_To_Stats()

  SKL_ASSERT(Nb_MV_Stat>0);
  Last_Avrg_MV[0] = MVx_Mean_Stat / Nb_MV_Stat;
  Last_Avrg_MV[1] = MVy_Mean_Stat / Nb_MV_Stat;

  const double N = 1. / Nb_MV_Stat;
  const double mv2 = (N*MVx_Mean_Stat)*(N*MVx_Mean_Stat) + (N*MVy_Mean_Stat)*(N*MVy_Mean_Stat);
  _mv = sqrt( mv2 );
  _dv = ( 4.*N*(MVx_SSE_Stat+MVy_SSE_Stat) - mv2 );
  if (_dv>0.) _dv = sqrt(_dv);
  else        _dv = 0; // negative values can occur, due to rounding errors
}

//////////////////////////////////////////////////////////

void SKL_MP4_ANALYZER_I::Scene_Changed() {
//  printf( "Scene change!\n" );
  if (Q0>0.)
    Q_Cur = Q0;
  if (MV_Search_Window0>0.)
    MV_Search_Window = MV_Search_Window0;
  Reset_Stats();
  SKL_ZERO_MV(Last_Avrg_MV);
}

void SKL_MP4_ANALYZER_I::Reset_Frame(const SKL_MP4_PIC *Pic,
                                   const SKL_MP4_INFOS *Infos)
{
  int x, y;

  SKL_MV *MVs = Pic->MV;
  for(y=0; y<2*Infos->MB_H; ++y) {
    for(x=0; x<2*Infos->MB_W; ++x) {
      SKL_ZERO_MV(MVs[x]);
    }
    MVs += Infos->MV_Stride;
  }

    // reset Map to full Intra
  SKL_MP4_MAP *C = Pic->Map;
  if (C!=0) {
    for(y=0; y<Infos->MB_H; ++y) {
      for(x=0; x<Infos->MB_W; ++x) {
        C[x].Type  = SKL_MAP_INTRA;
        C[x].Flags = 0;
        C[x].dQ    = 0;
        C[x].Sad   = 0xffffffff;
        SKL_ZERO_MV(C[x].MV);
        SKL_ZERO_MV(C[x].Acc);  
      }
      C += Infos->MB_W;
    }
  }
}

//////////////////////////////////////////////////////////
// params

int SKL_MP4_ANALYZER_I::Set_Param(const char * const Param, int Value)
{
  if (!strcmp(Param, "quant")) {
    if (Value<1 || Value>31) return -1;
    Q_Base = (float)Value;
    Q_Cur = Q_Base;
  }
  else if (!strcmp(Param, "search-size")) {
    if (Value<1 || Value>7) return -1;
    MV_Search_Window = (float)Value;
  }
  else if (!strcmp(Param, "intra-limit")) {
    if (Value<0 || Value>100) return -1;
    Intra_Limit = Value;
  }
  else if (!strcmp(Param, "rounding")) {
    if (Value<0 || Value>1) return -1;
    Rounding = Value&1;
  }
  else if (!strcmp(Param, "base-quant")) {
    if (Value<1 || Value>31) return -1;
    Q0 = (float)Value;
  }
  else if (!strcmp(Param, "base-search-size")) {
    if (Value<0 || Value>7) return -1;
    MV_Search_Window0 = (float)Value;
  }
  else if (!strcmp(Param, "search-method")) {
    if (Value<0 || Value>=MAX_SEARCH) return -1;
    Search_Method = Value;
  }
  else if (!strcmp(Param, "4v-probing")) {
    Inter4V_Probing = Value;
  }
  else if (!strcmp(Param, "field-pred-probing")) {
    Field_Pred_Probing = Value;
  }
  else if (!strcmp(Param, "sad-skip-limit")) {
    if (Value<0) return -1;
    SAD_Low_Limit = Value;
  }
  else if (!strcmp(Param, "sad-intra-limit")) {
    if (Value<0) return -1;
    SAD_Hi_Limit = Value;
  }
  else if (!strcmp(Param, "inter-threshold")) {
    Inter_Coding_Threshold = Value;
  }
  else if (!strcmp(Param, "use-trellis")) {
    Use_Trellis = Value;
  }
  else if (!strcmp(Param, "luminance-masking")) {
    Luminance_Masking = !!Value;
  }
  else if (!strcmp(Param, "subpixel")) {
    if (Value<0 || Value>2) return -1;  
    Subpixel_Precision = Value;
  }
  else if (!strcmp(Param, "bitrate")) {
    if (Value<0) return -1;  
    Bit_Rate = Value;
  }
  else if (!strcmp(Param, "quant-type")) {
    if (Value<0 || Value>1) return -1;
    Quant_Type = Value;
  }
  else if (!strcmp(Param, "interlace-field")) {
    if (Value<0 || Value>2) return -1;
    Interlace_Field = Value;
  }
  else if (!strcmp(Param, "interlace-dct")) {
    if (Value<0 || Value>2) return -1;
    Interlace_DCT = Value;
  }
  else if (!strcmp(Param, "reduced-frame")) {
    if (Value<-1 || Value>2) return -1;
    Reduced_Frame = Value;
  }
  else if (!strcmp(Param, "frequency")) {
    if (Value<1) return -1;
    Frequency = Value;
  }
  else if (!strcmp(Param, "frame-ticks")) {
    if (Value<1) return -1;
    Ticks_Per_Frame = Value;
  }
  else if (!strcmp(Param, "bframe")) {
    Use_BFrame = (Value!=0);
  }
  else if (!strcmp(Param, "gmc-pts")) {
    if (Value>3) return -1;
    GMC_Pts = Value;
  }
  else if (!strcmp(Param, "gmc-mode")) {
    if (Value<-1) return -1;
    GMC_Mode = Value;
  }
  else if (!strcmp(Param, "gmc-accuracy")) {
    if (Value>3 || Value<0) return -1;
    GMC_Accuracy = Value;
  }
  else if (!strcmp(Param, "intra-max-delay")) {
    if (Value<0) return -1;
    Intra_Max = Value;
  }
  else if (!strcmp(Param, "intra-max-count")) {
    Intra_Count = Value;
  }
  else if (!strcmp(Param, "search-metric")) {
    if (Value<0 || Value>2) return -1;
    Search_Metric = Value;
  }
  else if (!strcmp(Param, "hi-mem")) {
    Hi_Mem = !!Value;
  }
  else if (!strcmp(Param, "buffer-size")) {
    Buffer_Size_Max = Value;
  }
  else if (!strcmp(Param, "pass")) {
    if (Value>2 || Value<0) return -1;
    Pass.Set_Pass_Nb( Value );
  }
  else if (!strcmp(Param, "verbose")) {
    Verbose = Value;
  }
  else return 0;
  return 1;
}

int SKL_MP4_ANALYZER_I::Set_Param(const char * const Param, float Value)
{
  if (!strcmp(Param, "dquant-amp")) {
    if (Value<0.) return -1;
    dQuant_Amp = Value;
  }
  else if (!strcmp(Param, "framerate")) {
    if (Value<0) return -1;  
    Frame_Rate = Value;
  }
  else if (!strcmp(Param, "quant")) {
    if (Value<1.f || Value>31.f) return -1;
    Q_Base = Value;
    Q_Cur = Q_Base;
  }
  else if (!strcmp(Param, "search-size")) {
    if (Value<1.f || Value>7.f) return -1;
    MV_Search_Window = (float)Value;
  }
  else if (!strcmp(Param, "base-quant")) {
    if (Value<1.f || Value>31.f) return -1;
    Q0 = (float)Value;
  }
  else if (!strcmp(Param, "base-search-size")) {
    if (Value<1.f || Value>7.f) return -1;
    MV_Search_Window0 = (float)Value;
  }
  else if (!strcmp(Param, "lambda")) {
    if (Value<0.f) return -1;
    Lambda = Value;
  }
  else if (!strcmp(Param, "passrf")) {
    if (Value<0.) return -1;  
    Pass.Set_Pass_RF( Value );
  }
  else return 0;

  return 1;
}

int SKL_MP4_ANALYZER_I::Set_Param(const char * const Param, const char * const Value)
{
  if (!strcmp(Param, "passfile")) {
    if (Value==0) return -1;
    if (!Pass.Set_Pass_File_Name(Value)) return -1;
  }
  else return 0;
  return 1;
}

int SKL_MP4_ANALYZER_I::Get_Param(const char * const Param, int *Value) const
{
  if      (!strcmp(Param, "quant"))              *Value = RND( Q_Cur );
  else if (!strcmp(Param, "fcode"))              *Value = (MV_Max_Stat<=31) ? 1 : SKL_BMASKS::Bit_Sizes[MV_Max_Stat>>4];
  else if (!strcmp(Param, "fcode-bwd"))          *Value = (MV_Max_Stat<=31) ? 1 : SKL_BMASKS::Bit_Sizes[MV_Max_Stat>>4];
  else if (!strcmp(Param, "intra-limit"))        *Value = Intra_Limit;
  else if (!strcmp(Param, "4v-probing"))         *Value = Inter4V_Probing;
  else if (!strcmp(Param, "field-pred-probing")) *Value = Field_Pred_Probing;
  else if (!strcmp(Param, "sad-skip-limit"))     *Value = SAD_Low_Limit;
  else if (!strcmp(Param, "sad-intra-limit"))    *Value = SAD_Hi_Limit;
  else if (!strcmp(Param, "base-quant"))         *Value = RND( Q0 );
  else if (!strcmp(Param, "base-search-size"))   *Value = RND( MV_Search_Window0 );
  else if (!strcmp(Param, "search-method"))      *Value = Search_Method;
  else if (!strcmp(Param, "luminance-masking"))  *Value = Luminance_Masking;
  else if (!strcmp(Param, "subpixel"))           *Value = Subpixel_Precision;
  else if (!strcmp(Param, "bitrate"))            *Value = Bit_Rate;
  else if (!strcmp(Param, "verbose"))            *Value = Verbose;
  else if (!strcmp(Param, "quant-type"))         *Value = Quant_Type;
  else if (!strcmp(Param, "interlace-field"))    *Value = Interlace_Field;
  else if (!strcmp(Param, "inter-threshold"))    *Value = Inter_Coding_Threshold;
  else if (!strcmp(Param, "use-trellis"))        *Value = Use_Trellis;
  else if (!strcmp(Param, "interlace-dct"))      *Value = Interlace_DCT;
  else if (!strcmp(Param, "reduced-frame"))      *Value = Reduced_Frame;
  else if (!strcmp(Param, "frequency"))          *Value = Frequency;
  else if (!strcmp(Param, "rounding"))           *Value = Rounding;
  else if (!strcmp(Param, "frame-ticks"))        *Value = Ticks_Per_Frame;
  else if (!strcmp(Param, "intra-max-delay"))    *Value = Intra_Max;
  else if (!strcmp(Param, "intra-max-count"))    *Value = Intra_Count;
  else if (!strcmp(Param, "bframe"))             *Value = Use_BFrame;
  else if (!strcmp(Param, "gmc-pts"))            *Value = GMC_Pts;
  else if (!strcmp(Param, "gmc-mode"))           *Value = GMC_Mode;
  else if (!strcmp(Param, "gmc-accuracy"))       *Value = GMC_Accuracy;
  else if (!strcmp(Param, "search-metric"))      *Value = Search_Metric;
  else if (!strcmp(Param, "search-size"))        *Value = RND( MV_Search_Window );
  else if (!strcmp(Param, "hi-mem"))             *Value = Hi_Mem;
  else if (!strcmp(Param, "buffer-size"))        *Value = Buffer_Size_Max;
  else if (!strcmp(Param, "mean-mv-x"))          *Value = Last_Avrg_MV[0];
  else if (!strcmp(Param, "mean-mv-y"))          *Value = Last_Avrg_MV[1];
  else if (!strcmp(Param, "version"))            *Value = CORE_VERSION;
  else if (!strcmp(Param, "pass"))               *Value = Pass.Get_Pass_Nb();
  else return 0;
  return 1;
}

int SKL_MP4_ANALYZER_I::Get_Param(const char * const Param, float *Value) const
{
  if      (!strcmp(Param, "dquant-amp"))  *Value = dQuant_Amp;
  else if (!strcmp(Param, "framerate"))   *Value = Frame_Rate;
  else if (!strcmp(Param, "quant"))       *Value = Q_Cur;
  else if (!strcmp(Param, "search-size")) *Value = MV_Search_Window;
  else if (!strcmp(Param, "lambda"))      *Value = Lambda;
  else if (!strcmp(Param, "base-quant"))  *Value = Q0;
  else if (!strcmp(Param, "passrf"))      *Value = Pass.Get_Pass_RF();
  else if (!strcmp(Param, "base-search-size")) *Value = MV_Search_Window0;
  else if (!strcmp(Param, "mean-mv"))          *Value = (float)_mv;
  else if (!strcmp(Param, "dev-mv"))           *Value = (float)_dv;
  else return 0;
  return 1;
}

int SKL_MP4_ANALYZER_I::Get_Param(const char * const Param, const char ** const Value) const
{
  if (!strcmp(Param, "passfile"))  *Value = Pass.Get_Pass_File_Name();
  else return 0;
  return 1;
}

const int *SKL_MP4_ANALYZER_I::Get_Param(const char * const Param) const
{
  if (!strcmp(Param, "gmc-warp-pts")) return (const int*)GMC_Warps;
  return 0;
}

void SKL_MP4_ANALYZER_I::Print_Caps()
{
  printf( " Supported options: [quant][search-size][intra-limit]\n" );
  printf( "                    [base-quant][base-search-size][lambda]\n" );
  printf( "                    [search-method][sub-pixel][4v-probing][field-pred-probing]\n" );
  printf( "                    [luminance-masking][dquant-amp]\n" );
  printf( "                    [framerate][frequency][frame-ticks][bitrate]\n" );
  printf( "                    [inter-threshold][interlace-dct][interlace-fld]\n" );
  printf( "                    [reduced-frame][intra-max-delay][intra-max-count]\n" );
  printf( "                    [bframe][gmc][rounding][verbose]\n" );
  printf( " Searchs:    0:Square 1: Diamond 2: PHODS, 3:Log, 4:Full\n" );
  printf( "             5:Zero, 6:Debug junk\n" );
  printf( "\n" );
}
    
//////////////////////////////////////////////////////////
// And now ladies and gentlemen... (drum roll) ... 
// ... The Simplest Rate Controler Ever!!
//////////////////////////////////////////////////////////

void SKL_MP4_ANALYZER_I::Post_Coding_Update(const SKL_MP4_INFOS * const Infos)
{  
     // TODO: Examine SSE also, to check is this avrg MV is
     // a "good" one (ie.: not spread to much...)

  const double Limit = 8.*pow(2.,MV_Search_Window);
  if (_mv+3.3*_dv>Limit) {
    if (MV_Search_Window<(7.0/1.1))
      MV_Search_Window *= 1.1f;
  }
  else if (_mv+3.3*_dv<.2*Limit) {
    if (MV_Search_Window>=(1.0*1.1))
      MV_Search_Window *= (1.f/1.1f);
  }
//  printf( "%d %.3f %.3f %.3f %.3f %3f %3f\n", Infos->Frame_Number, _mv, _dv, Limit, _mv+2.*_dv, _mv+.2*_dv,.6*Limit );

  if (Pass.Get_Pass_Nb()==2)
  {
    SKL_FRAME_STATS Stat;
    Get_Param( "quant", &Stat.Q );
    Stat.Coded_Bytes = (Infos->Coded_Bits+7)>>3;
    Stat.Txt_Bytes   = (Infos->Texture_Bits+7)>>3;
    Stat.MV_Bytes    = (Infos->MV_Bits+7)>>3;
    Stat.Frame_Order = Infos->Frame_Number;
    Pass.Compute_New_Frame_Params(&Stat);
    Set_Param("quant", Stat.Q);
    Set_Param("base-quant", Stat.Q);
  }
  else {
    if (Pass.Get_Pass_Nb()==1) {
      if (Pass.Open_Write_Pass_File(Infos->Cur->Width, Infos->Cur->Height))
      {
        SKL_FRAME_STATS Stat;
        Get_Param( "quant", &Stat.Q );
        Get_Param( "search-size", &Stat.FCode );
        Get_Param( "mean-mv", &Stat.mMV );
        Get_Param( "dev-mv", &Stat.dMV );
        Stat.Frame_Order = Infos->Frame_Number;
        Stat.Pic_Type    = Infos->Cur->Coding;
        Stat.Coded_Bytes = (Infos->Coded_Bits+7)>>3;
        Stat.Txt_Bytes   = (Infos->Texture_Bits+7)>>3;
        Stat.MV_Bytes    = (Infos->MV_Bits+7)>>3;
        Pass.Save_Frame_Params( &Stat );
      }
    }

    if (Bit_Rate>0)   //   rate control activated?
    {
      const float Pass_RF = Pass.Get_Pass_RF();
      const int Coded_Bits = Infos->Coded_Bits;
      float Avrg_Per_Frame = 1000.f * (Bit_Rate-20.f) / Frame_Rate;
      float Missed = 1.0f * (Coded_Bits+100.f) / (Avrg_Per_Frame+100.f);
      float dQ = 4.f*(Missed-1.0f)*Pass_RF;
      if      (dQ> 1.5f) dQ = 1.5f;
      else if (dQ<-1.5f) dQ =-1.5f;
      Q_Cur += dQ;
      if      (Q_Cur>31.49f) Q_Cur = 31.49f;
      else if (Q_Cur< 0.50f) Q_Cur =  0.50f;
      Q_Base = Q_Cur;
    }
  }
}

//////////////////////////////////////////////////////////
// Factory

SKL_MP4_ANALYZER *Skl_MP4_Get_Default_Analyzer(SKL_MEM_I *Mem) {
  return ::new SKL_MP4_ANALYZER_I(Mem);
}

void Skl_MP4_Destroy_Default_Analyzer(SKL_MP4_ANALYZER *Anl) {
  ::delete Anl;
}

//////////////////////////////////////////////////////////

#endif  /* !SKL_AUTO_INCLUDE */
