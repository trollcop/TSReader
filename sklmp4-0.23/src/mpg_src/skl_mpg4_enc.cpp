/********************************************************
 * Some code. Copyright (C) 2003 by Pascal Massimino.   *
 * All Rights Reserved.      (http://skal.planet-d.net) *
 * For Educational/Academic use ONLY. See 'LICENSE.TXT'.*
 ********************************************************/
/*
 * skl_mpg4_enc.cpp
 *
 * MPEG4 encoder
 ********************************************************/

#include "./skl_mpg4i.h"
#include "skl_syst/skl_exception.h"

static const char * const ID_STRING = "sklmp4 004";

//////////////////////////////////////////////////////////

#define ABS(x)  (((x)<0) ? -(x) : (x))
#define DIV_ROUND(x,y)  ( (x)>=0 ? ((x)+((y)>>1))/(y) : ((x)-((y)>>1))/(y) )

//////////////////////////////////////////////////////////
// Encoding tables
//////////////////////////////////////////////////////////

struct SKL_VLC { SKL_INT16 Val, Len; };

  // Table B-12
static const SKL_VLC MV_B12_Tab[32+1+32] = {
  { 5, 13}, { 7, 13}, { 5, 12}, { 7, 12}, { 9, 12}, {11, 12}, {13, 12}, {15, 12}
, { 9, 11}, {11, 11}, {13, 11}, {15, 11}, {17, 11}, {19, 11}, {21, 11}, {23, 11}
, {25, 11}, {27, 11}, {29, 11}, {31, 11}, {33, 11}, {35, 11}, {19, 10}, {21, 10}
, {23, 10}, { 7,  8}, { 9,  8}, {11,  8}, { 7,  7}, { 3,  5}, { 3,  4}, { 3,  3}
, { 1,  1}
, { 2,  3}, { 2,  4}, { 2,  5}, { 6,  7}, {10,  8}, { 8,  8}, { 6,  8}, {22, 10}
, {20, 10}, {18, 10}, {34, 11}, {32, 11}, {30, 11}, {28, 11}, {26, 11}, {24, 11}
, {22, 11}, {20, 11}, {18, 11}, {16, 11}, {14, 11}, {12, 11}, {10, 11}, { 8, 11}
, {14, 12}, {12, 12}, {10, 12}, { 8, 12}, { 6, 12}, { 4, 12}, { 6, 13}, { 4, 13}
};

  // Table B-6
static const SKL_VLC MCBPC_Intra_B6[2][4] = {  // index: [MB_Type-3][Cbpc]
  { {1, 1}, {1, 3}, {2, 3}, {3, 3} }    // MB_Type = 3, Cbpc=00 .. 11
, { {1, 4}, {1, 6}, {2, 6}, {3, 6} }    // MB_Type = 4, Cbpc=00 .. 11
};
  // Table B-7
static const SKL_VLC MCBPC_Inter_B7[5][4] = {  // index: [MB_Type][Cbpc]
  { {1, 1}, {3, 4}, {2, 4}, {5, 6} }
, { {3, 3}, {7, 7}, {6, 7}, {5, 9} }
, { {2, 3}, {5, 7}, {4, 7}, {5, 8} }
, { {3, 5}, {4, 8}, {3, 8}, {3, 7} }
, { {4, 6}, {4, 9}, {3, 9}, {2, 9} }
};

static const SKL_VLC CBPY_B8[16] = {   // index: Cbpy
  {3, 4}, {5, 5}, {4, 5}, { 9, 4}
, {3, 5}, {7, 4}, {2, 6}, {11, 4}
, {2, 5}, {3, 6}, {5, 4}, {10, 4}
, {4, 4}, {8, 4}, {6, 4}, { 3, 2}
};

 // Table B13/B14: DC-diff code, indexed by [size-1]. 
 // Bits for |DC| are blanked. 
 // Final Marker bit included for Size>8

static const SKL_VLC DCY_Tab_B13[12] = {
  { 6,  3}, { 8,  4}, {16,  6}, {16,  7},
  {32,  9}, {64, 11}, {128, 13}, {256, 15},
  {1025, 18}, {4097, 21}, {8193, 23}, {8193, 24}
};
  
static const SKL_VLC DCC_Tab_B14[12] = {
  { 4,  3}, { 4,  4}, { 8,  6}, {16,  8}
, {32, 10}, {64, 12}, {128, 14}, {256, 16}
, {1025, 19}, {2049, 21}, {4097, 23}, {8193, 25}
};

  // Table 6-27
static const int DQuant_Tab[5] = { 1, 0, -1 /*error*/, 2, 3 };

  // Table 6-33
static const SKL_VLC Spr_Tab_B33[15] = {
  {   0, 2}, {   2, 3}, {   3, 3}, {  4, 3}
, {   5, 3}, {   6, 3}, {  14, 4}, { 30, 5}
, {  62, 6}, { 126, 7}, { 254, 8}, {510, 9}
, {1022,10}, {2046,11}, {4094,12}
};
  
//////////////////////////////////////////////////////////
// MV coding
//////////////////////////////////////////////////////////

static void Write_Vector_Comp(SKL_FBB * const Bits, SKL_INT16 Val, SKL_INT32 Fix)
{
  const int High = 16 << Fix;
  SKL_ASSERT(Val>=-2*High && Val<2*High);

  if      (Val < -High) Val += 2*High;
  else if (Val >= High) Val -= 2*High;

  if (!Val)
    Bits->Put_Bits(1, 1); // 0 case hardcoded
  else {
    if (--Fix) {
      const SKL_UINT32 Sign = (Val<0);
      if (Sign) Val ^= -1;
      else      Val -= 1;
      const int Residue = Val & SKL_BMASKS::And[Fix];
      Val >>= Fix;
      SKL_ASSERT(Val>=0 && Val<=31);
      Bits->Put_Bits( MV_B12_Tab[33+Val].Val | Sign, MV_B12_Tab[33+Val].Len );
      Bits->Put_Bits( Residue, Fix );
    }
    else {
      SKL_ASSERT(Val>=-32 && Val<=32);
      Bits->Put_Bits( MV_B12_Tab[Val+32].Val, MV_B12_Tab[Val+32].Len );
    }
  }  
}

static void Write_Vector(SKL_FBB * const Bits, const SKL_MV MV, SKL_INT32 Fix)
{
  Write_Vector_Comp(Bits, MV[0], Fix);
  Write_Vector_Comp(Bits, MV[1], Fix);
}

//////////////////////////////////////////////////////////
//   AC coeff coding. 
//
// Description of the matrices Code[Run][Level]:
//  Codable {run,level} combinaisons ('x') are packed on the
//  upper left corner. Actually, the matrix looks like:
//    +-----------------[level]
//    |xxxxxxxxxxx...... <-
//    |xxxxxxxx......... <- max_level[run]
//    |xxxx............. <-
//    |xx...............
//    |x................
//    |x................
//    |.................
//    |.................
//[run]^^^^___ max_run[level]
//
//  Uncharted coeffs ('.') need being coded with escape code.
// If you use Esc-1 coding, you will jump on the left by
// max_level[run] steps, in hope you fall on a 'x' coded
// combinaison. With Esc-2 coding, you will move up by
// max_run[level]+1 steps. If neither works, you have to
// fall back to expensive Esc-3 coding.
//////////////////////////////////////////////////////////

#include "./skl_mpg4_enc_tbl.h"

static inline void Write_DC_Coeff(SKL_FBB * const Bits,
                                  SKL_INT16 DC,
                                  const int Lum)
{
  if (DC) {
    const int Sign = (DC<0);
    if (Sign) DC = -DC;
    int Size = SKL_BMASKS::Log2(DC);
    SKL_ASSERT(Size>=1 && Size<=12);
    if (Sign) DC ^= SKL_BMASKS::And[Size];
    const SKL_VLC *VLC = Lum ? &DCY_Tab_B13[Size-1] : &DCC_Tab_B14[Size-1];
    if (Size>8) DC <<= 1;     // make room for marker bit
    Bits->Put_Bits( VLC->Val | DC, VLC->Len );
  }
  else {
    if (Lum) Bits->Put_Bits( 3, 3 );
    else     Bits->Put_Bits( 3, 2 );
  }
}

static inline void Encode_Coeff(SKL_FBB *Bits, int Level, const SKL_UINT32 * const Tab)
{
  SKL_ASSERT(Level!=0 && Level>=-2048 && Level<=2047);
  if (!((Level+64)&-128)) { // Level is in [-64,63] -> use table
    const SKL_UINT32 Code = Tab[Level];
    if (0<=(SKL_INT32)Code)  // !bit31? -> it's a regular 21bits-max code
      Bits->Put_Bits( Code&0x00ffffff, Code>>24 );
    else                     // otherwise, code is actually a 30bits-Esc3
      Bits->Put_Bits( Code&0x7fffffff, 7+2+1+6+1+12+1 );
  }
  else {
      // Esc3 encoding (30bits total) for level not in [-64,63]
      // Tab[0] contains the esc3 code base: 0x01e02001 | (last<<20) | (run<<14)
    const SKL_UINT32 Code = Tab[0] | ((Level&0xfff)<<1);
    Bits->Put_Bits( Code, 7+2+1+6+1+12+1 );
  }
}

static inline void Write_Coeffs(SKL_FBB * const Bits, 
                                const SKL_INT16 *C,
                                const int Zigzag[],
                                int Last,
                                const SKL_UINT32 Tabs[2][64][128])
{
  Zigzag += Last;
  int j = -Last;
  while(1) {
    int Level;
    do Level = C[Zigzag[j++]]; while (!Level);
    if (j<=0) {
      Encode_Coeff( Bits, Level, &Tabs[0][Last+j-1][64] );
      Last = -j;
    }
    else {
      Encode_Coeff( Bits, Level, &Tabs[1][Last][64] );
      break;
    }
  }
}

static inline int Find_Last(const SKL_INT16 C[6*64], const int *Zigzag, int i)
{
  while(i>=0)
    if (C[Zigzag[i]])
      return i;
    else i--;
  return -1;
}


//////////////////////////////////////////////////////////
//
//        Trellis-Based quantization
//
// So far I understand this paper:
//
//  "Trellis-Based R-D Optimal Quantization in H.263+"
//    J.Wen, M.Luttrell, J.Villasenor
//    IEEE Transactions on Image Processing, Vol.9, No.8, Aug. 2000.
//
// we are at stake with a simplified Bellman-Ford / Dijkstra Single
// Source Shortest Path algo. But due to the underlying graph structure
// ("Trellis"), it can be turned into a dynamic programming algo,
// partially saving the explicit graph's nodes representation. And 
// without using a heap, since the open frontier of the DAG is always
// known, and of fixed size.
//
//////////////////////////////////////////////////////////


#define DBG 0

SKL_UINT32 SKL_MP4_ENC_I::Evaluate_Cost(const SKL_INT16 *C, const int * const Zigzag, int Max, int Lambda) const
{
#if (DBG>0)
  const SKL_INT16 * const Ref = C + 6*64;
  int Last = Max;
  while(Last>=0 && C[Zigzag[Last]]==0) Last--;
  int Bits = 0;
  if (Last>=0) {
    Bits = 2;   // CBP
    int j=0, j0=0;
    int Run, Level;
    while(j<Last) {
      while(!C[Zigzag[j]]) j++;
      if (j==Last) break;
      Level=C[Zigzag[j]];
      Run = j - j0;
      j0 = ++j;
      if (Level>=-24 && Level<=24) Bits += B16_17_Code_Len[(Level<0) ? -Level-1 : Level-1][Run];
      else Bits += 30;
    }
    Level = C[Zigzag[Last]];
    Run = j - j0;
    if (Level>=-6 && Level<=6) Bits += B16_17_Code_Len_Last[(Level<0) ? -Level-1 : Level-1][Run];
    else Bits += 30;
  }

  int Dist = 0;
  for(int i=0; i<=Last; ++i) {
    const int q = Quant_Type ? ((Q*Inter_Matrix[Zigzag[i]]) >> 4) : Q;
    const int Mult = 2*q;
    const int Bias = (q-1) | 1;
    int V = C[Zigzag[i]]*Mult;
    if      (V>0) V += Bias;
    else if (V<0) V -= Bias;
    V -= Ref[Zigzag[i]];
    Dist += V*V;
  }
  SKL_UINT32 Cost = Lambda*Dist + (Bits<<16);
  if (DBG==1)
    printf( " Last:%2d/%2d Cost = [(Bits=%5.0d) + Lambda*(Dist=%6.0d) = %d ] >>12= %d ", Last,Max, Bits, Dist, Cost, Cost>>12 );
  return Cost;

#else
  return 0;
#endif
}

#define TL(q) 0xfe00/(q*q)

static const int Trellis_Lambda_Tabs[31] = {
         TL( 1),TL( 2),TL( 3),TL( 4),TL( 5),TL( 6), TL( 7),
  TL( 8),TL( 9),TL(10),TL(11),TL(12),TL(13),TL(14), TL(15),
  TL(16),TL(17),TL(18),TL(19),TL(20),TL(21),TL(22), TL(23),
  TL(24),TL(25),TL(26),TL(27),TL(28),TL(29),TL(30), TL(31)
};
#undef TL

int SKL_MP4_ENC_I::Trellis_Quantize(SKL_INT16 * const Out, const int Q, const int * const Zigzag, int Non_Zero) const
{
    // Note: We should search last non-zero coeffs on *real* DCT input coeffs (In[]), 
    // not quantized one (Out[]). However, it only improves the result *very* 
    // slightly (~0.01dB), whereas speed drops to crawling level :)
    // Well, actually, taking 1 more coeff past Non_Zero into account sometimes helps,

  Non_Zero = Find_Last(Out, Zigzag, Non_Zero);
  if (Non_Zero<0)
      return -1;  

  struct NODE { SKL_INT16 Run, Level; };
  NODE Nodes[65], Last;
  SKL_UINT32 Run_Costs0[64+1], * const Run_Costs = Run_Costs0 + 1;

  const int Lambda = Trellis_Lambda_Tabs[Q-1];    // it's 1/lambda, actually

  int Run_Start = -1;
  Run_Costs[-1] = 2<<16;                          // source (w/ CBP penalty)
  SKL_UINT32 Min_Cost = 2<<16;

  int Last_Node = -1;
  SKL_UINT32 Last_Cost = 0;

#if (DBG>0)
  Last.Level = 0; Last.Run = -1; // just initialize to smthg
#endif

  int i, j;
  for(i=0; i<=Non_Zero; i++)
  {
    const int q = Quant_Type ? ((Q*Inter_Matrix[Zigzag[i]]) >> 4) : Q;
    const int Mult = 2*q;
    const int Bias = (q-1) | 1;
    const int Lev0 = Mult + Bias;

    const SKL_INT16 * const In = Out + 6*64;
    const int AC = In[Zigzag[i]];
    const int Level1 = Out[Zigzag[i]];
    const int Dist0 = Lambda* AC*AC;
    Last_Cost += Dist0;

    SKL_UINT32 Best_Cost;
    if (3U>(SKL_UINT32)(Level1+1))                 // very specialized loop for -1,0,+1
    {
      int dQ;
      if (AC<0) {
        Nodes[i].Level = -1;
        dQ = Lev0 + AC;
      }
      else {
        Nodes[i].Level = 1;
        dQ = Lev0 - AC;
      }
      const SKL_UINT32 Cost0 = Lambda*dQ*dQ;
      Nodes[i].Run = 1;
      Best_Cost = (Code_Len20[0]<<16) + Run_Costs[i-1]+Cost0;
      for(int Run=i-Run_Start; Run>0; --Run)
      {
        const SKL_UINT32 Cost_Base = Cost0 + Run_Costs[i-Run];
        const SKL_UINT32 Cost = Cost_Base + (Code_Len20[Run-1]<<16);
          // TODO: what about tie-breaks? Should we favor short runs or
          // long runs? Although the error is the same, it would not be
          // spread the same way along high and low frequencies...
        if (Cost<Best_Cost)
        {
          Best_Cost    = Cost;
          Nodes[i].Run = Run;
        }
        const SKL_UINT32 lCost = Cost_Base + (Code_Len24[Run-1]<<16);
        if (lCost<Last_Cost)
        {
          Last_Cost  = lCost;
          Last.Run   = Run;
          Last_Node  = i;
        }
      }
      if (Last_Node==i) Last.Level = Nodes[i].Level;


      if (DBG==1) {
        Run_Costs[i] = Best_Cost;
        printf( "Costs #%2d: ", i);
        for(j=-1;j<=Non_Zero;++j) {
          if (j==Run_Start)            printf( " %3.0d|", Run_Costs[j]>>12 );
          else if (j>Run_Start && j<i) printf( " %3.0d|", Run_Costs[j]>>12 );
          else if (j==i)               printf( "(%3.0d)", Run_Costs[j]>>12 );
          else                         printf( "  - |" );
        }
        printf( "<%3.0d %2d %d>", Min_Cost>>12, Nodes[i].Level, Nodes[i].Run );
        printf( "  Last:#%2d {%3.0d %2d %d}", Last_Node, Last_Cost>>12, Last.Level, Last.Run );
        printf( " AC:%3.0d Dist0:%3d Dist(%d)=%d", AC, Dist0>>12, Nodes[i].Level, Cost0 );
        printf( "\n" );
      }
    }
    else if (51U>(SKL_UINT32)(Level1+25))             // "big" levels (not less than ESC3, though)
    {
      Best_Cost = 0xf0000000;
      const SKL_BYTE *Tbl_L1, *Tbl_L2, *Tbl_L1_Last, *Tbl_L2_Last;
      int Level2;
      int dQ1, dQ2;
      if (Level1>1) {
        dQ1 = Level1*Mult-AC + Bias;
        dQ2 = dQ1 - Mult;
        Level2 = Level1-1;
        Tbl_L1      = (Level1<=24) ? B16_17_Code_Len[Level1-1]     : Code_Len0;
        Tbl_L2      = (Level2<=24) ? B16_17_Code_Len[Level2-1]     : Code_Len0;
        Tbl_L1_Last = (Level1<=6) ? B16_17_Code_Len_Last[Level1-1] : Code_Len0;
        Tbl_L2_Last = (Level2<=6) ? B16_17_Code_Len_Last[Level2-1] : Code_Len0;
      }
      else { // Level1<-1
        dQ1 = Level1*Mult-AC - Bias;
        dQ2 = dQ1 + Mult;
        Level2 = Level1 + 1;
        Tbl_L1      = (Level1>=-24) ? B16_17_Code_Len[Level1^-1]      : Code_Len0;
        Tbl_L2      = (Level2>=-24) ? B16_17_Code_Len[Level2^-1]      : Code_Len0;
        Tbl_L1_Last = (Level1>=- 6) ? B16_17_Code_Len_Last[Level1^-1] : Code_Len0;
        Tbl_L2_Last = (Level2>=- 6) ? B16_17_Code_Len_Last[Level2^-1] : Code_Len0;
      }

      const SKL_UINT32 Dist1 = Lambda*dQ1*dQ1;
      const SKL_UINT32 Dist2 = Lambda*dQ2*dQ2;
      const int dDist21 = Dist2-Dist1;

      for(int Run=i-Run_Start; Run>0; --Run)
      {
        const SKL_UINT32 Cost_Base = Dist1 + Run_Costs[i-Run];

// for sub-optimal (but slightly worth it, speed-wise) search, uncomment the following:
//        if (Cost_Base>=Best_Cost) continue;

        SKL_UINT32 Cost1, Cost2;
        int bLevel;

        Cost1 = Cost_Base + (Tbl_L1[Run-1]<<16);
        Cost2 = Cost_Base + (Tbl_L2[Run-1]<<16) + dDist21;

        if (Cost2<Cost1) { Cost1 = Cost2; bLevel = Level2; }
        else bLevel = Level1;

        if (Cost1<Best_Cost)
        {
          Best_Cost = Cost1;
          Nodes[i].Run   = Run;
          Nodes[i].Level = bLevel;
        }

        Cost1 = Cost_Base + (Tbl_L1_Last[Run-1]<<16);
        Cost2 = Cost_Base + (Tbl_L2_Last[Run-1]<<16) + dDist21;

        if (Cost2<Cost1) { Cost1 = Cost2; bLevel = Level2; }
        else bLevel = Level1;
        if (Cost1<Last_Cost)
        {
          Last_Cost  = Cost1;
          Last.Run   = Run;
          Last.Level = bLevel;
          Last_Node  = i;
        }
      }

      if (DBG==1) {
        Run_Costs[i] = Best_Cost;
        printf( "Costs #%2d: ", i);
        for(j=-1;j<=Non_Zero;++j) {
          if (j==Run_Start)            printf( " %3.0d|", Run_Costs[j]>>12 );
          else if (j>Run_Start && j<i) printf( " %3.0d|", Run_Costs[j]>>12 );
          else if (j==i)               printf( "(%3.0d)", Run_Costs[j]>>12 );
          else                         printf( "  - |" );
        }
        printf( "<%3.0d %2d %d>", Min_Cost>>12, Nodes[i].Level, Nodes[i].Run );
        printf( "  Last:#%2d {%3.0d %2d %d}", Last_Node, Last_Cost>>12, Last.Level, Last.Run );
        printf( " AC:%3.0d Dist0:%3d Dist(%2d):%3d Dist(%2d):%3d", AC, Dist0>>12, Level1, Dist1>>12, Level2, Dist2>>12 );
        printf( "\n" );
      }
    }
    else       // Very very high levels, with no chance of being optimizable => Simply pick best Run.
    {
      Best_Cost = 0xf0000000;
      for(int Run=i-Run_Start; Run>0; --Run)
      {
        const SKL_UINT32 Cost = (30<<16) + Run_Costs[i-Run];    // 30 bits + no distortion
        if (Cost<Best_Cost)
        {
          Best_Cost = Cost;
          Nodes[i].Run   = Run;
          Nodes[i].Level = Level1;
        }

        if (Cost<Last_Cost)
        {
          Last_Cost  = Cost;
          Last.Run   = Run;
          Last.Level = Level1;
          Last_Node  = i;
        }
      }
    }


    Run_Costs[i] = Best_Cost;

    if (Best_Cost < Min_Cost + Dist0) {
      Min_Cost = Best_Cost;
      Run_Start = i;
    }
    else
    {
        // as noticed by Michael Niedermayer (michaelni at gmx.at), there's
        // a code shorter by 1 bit for a larger run (!), same level. We give
        // it a chance by not moving the left barrier too much.
      while( Run_Costs[Run_Start]>Min_Cost+(1<<16) )
        Run_Start++;

        // spread on preceding coeffs the cost incurred by skipping this one
      for(j=Run_Start; j<i; ++j) Run_Costs[j] += Dist0;
      Min_Cost += Dist0;
    }

  }
  if (DBG) {
    Last_Cost = Evaluate_Cost(Out, Zigzag, Non_Zero, Lambda);
    if (DBG==1) {
      printf( "=> " );
      for(i=0; i<=Non_Zero; ++i) printf( "[%3.0d] ", Out[Zigzag[i]] );
      printf( "\n" );
   }
  }

  if (Last_Node<0)
    return -1;

       // reconstruct optimal sequence backward with surviving paths

  SKL_BZERO(Out, 64*sizeof(*Out));
  Out[Zigzag[Last_Node]] = Last.Level;
  i = Last_Node - Last.Run;
  while(i>=0) {
    Out[Zigzag[i]] = Nodes[i].Level;
    i -= Nodes[i].Run;
  }

  if (DBG) {
    SKL_UINT32 Cost = Evaluate_Cost(Out, Zigzag, Non_Zero, Lambda);
    if (DBG==1) {
      printf( "<= " ); 
      for(i=0; i<=Last_Node; ++i) printf( "[%3.0d] ", Out[Zigzag[i]] );
      printf( "\n--------------------------------\n" );
    }
    if (Cost>Last_Cost) printf( "!!! %d > %d\n", Cost, Last_Cost );
  }
  return Last_Node;
}

#undef DBG

//////////////////////////////////////////////////////////
//
// Main decimation calls
//
//////////////////////////////////////////////////////////

void SKL_MP4_ENC_I::Decimate_Intra(SKL_INT16 * const Out,
                                    SKL_INT16 * const In,
                                    SKL_INT32 Q, SKL_INT32 DC_Q) const
{
  Quant_Ops.Dct(In);
  Quant_Ops.Quant_Intra(Out, In, Q_Intra, Q, DC_Q);

// this is WRONG 2 out of 3 times, due to DC/AC prediction
//  if (_Use_Trellis && Trellis_Quantize(Out, Q, SKL_MP4_I::Scan_Order[0]+1, 62)<0)
//    return;

  Quant_Ops.Dequant_Intra(In, Out, Q_Intra, Q, DC_Q);
}


int SKL_MP4_ENC_I::Decimate_Inter(SKL_INT16 * const Out,
                                   SKL_INT16 * const In,
                                   const SKL_INT32 Q) const
{
  Quant_Ops.Dct(In);
  int SAV = Quant_Ops.Quant_Inter(Out, In, Q_Inter, Q);
  if (SAV<_Inter_Coding_Threshold) return 0;
  if (_Use_Trellis && Trellis_Quantize(Out, Q, SKL_MP4_I::Scan_Order[0], 63)<0)
    return 0;
  Quant_Ops.Dequant_Inter(In, Out, Q_Inter, Q, 0xff);
  return SAV;
}

//////////////////////////////////////////////////////////
// Macroblock params polishing
//////////////////////////////////////////////////////////

const int SKL_MB_ENC::Map_To_Type[SKL_MAP_LAST] =
{
  SKL_MB_SKIPPED, SKL_MB_INTRA, SKL_MB_INTER /*(GMC)*/, 
  SKL_MB_INTER, SKL_MB_INTER, SKL_MB_INTER4V
};

static inline void Set_dMV(SKL_MV dMV, const SKL_MV MVo)
{
    // dMV contains predictor
  dMV[0] = MVo[0] - dMV[0];
  dMV[1] = MVo[1] - dMV[1];
}

void SKL_MB_ENC::Substract_Prediction()
{
  SKL_ASSERT(MB_Type<=SKL_MB_INTER4V && Field_Pred==0);    // INTER or INTER4V

  Predict_Motion_Vector_Blk0( dMVs[0], &MVs[0], MVs-1 );
  Set_dMV(dMVs[0], MVs[0]);
  if (MB_Type==SKL_MB_INTER4V)
  {
    Predict_Motion_Vector( dMVs[1], &MVs[1], 1 );
    Set_dMV(dMVs[1], MVs[ 1]);
    Predict_Motion_Vector( dMVs[2], &MVs2[0], 2 );
    Set_dMV(dMVs[2], MVs2[0]);
    Predict_Motion_Vector( dMVs[3], &MVs2[1], 3 );
    Set_dMV(dMVs[3], MVs2[1]);
  }
}

void SKL_MB_ENC::Substract_Field_Prediction()
{
  SKL_ASSERT(MB_Type<SKL_MB_INTER4V && Field_Pred==1);  // INTER+field only

  Predict_Motion_Vector_Blk0( dMVs[0], &MVs[0], MVs-1 );
  dMVs[0][1] /= 2;               // Both fields use the same predictor
  MVs[0][1] >>= 1;               // descale to field-MV...
  MVs[1][1] >>= 1;               // Scale will be restored after coding.
  SKL_COPY_MV(dMVs[1], dMVs[0]);
  Set_dMV(dMVs[0], MVs[0]);
  Set_dMV(dMVs[1], MVs[1]);
}

void SKL_MB_ENC::Set_Final_Params()
{
    // update dQ

  if (Map[Pos].dQ) {
    dQuant = Map[Pos].dQ;
    Prev_Quant = Quant;
    Quant += dQuant;
    SKL_ASSERT(Quant>=1 && Quant<=31);
  }
  else dQuant = 0;

    // set up field params

  if (VOL->Interlace) {
    if (VOL->Interlace&0x3) Set_Field_DCT( (Map[Pos].Flags&1)!=0 );
    else SKL_ASSERT(Field_DCT==0);
    if (VOL->Interlace&0xc)  {
      if (Map[Pos].Type==SKL_MAP_16x8) {
        Field_Pred = 1;
        Field_Dir = ((MVs[0][1]&1)<<1) | ((MVs[1][1]&1)^1);
        Substract_Field_Prediction();
      }
      else Field_Pred = 0;
    }
    else SKL_ASSERT(Field_Pred==0); 
  }
  else SKL_ASSERT(Field_DCT==-1 && Field_Pred==0);

  if (MB_Type!=SKL_MB_INTRA && !MC_Sel && !Field_Pred)
    Substract_Prediction();
}

//////////////////////////////////////////////////////////
// I-MB coding

void SKL_MB_ENC::Encode_Intra(SKL_FBB * const Bits, SKL_INT16 In[12*64], int Is_I_VOP)
{
  SKL_INT16 *C;
  int blk;
  const int * Zigzags[6];
  int Pred_Dirs[6];

  Use_AC_Pred = Select_DC_AC_Pred(In, Pred_Dirs);

  Cbp = 0;
  C = In;
  for(blk=0; blk<6; ++blk)
  {
    Zigzags[blk] = Use_AC_Pred ? SKL_MP4_I::Scan_Order[Pred_Dirs[blk]]
                               : SKL_MP4_I::Scan_Order[0];
    Last[blk] = Find_Last(C, Zigzags[blk], 63);
    Cbp <<= 1;
    if (Last[blk]>0) Cbp |= 1;
    C += 64;
  }

    // time to really set the MB_Type
  if (dQuant!=0) {
    SKL_ASSERT(MB_Type!=SKL_MB_INTER4V);
    MB_Type += 1;  // INTRA->INTRA_Q or INTER->INTER_Q
  }

  const SKL_VLC *VLC;
  if (Is_I_VOP) VLC = &MCBPC_Intra_B6[ MB_Type-3 ][ Cbp & 3 ];
  else VLC = &MCBPC_Inter_B7[ MB_Type ][ Cbp & 3 ];
  Bits->Put_Bits( VLC->Val, VLC->Len );

  Bits->Put_Bits( Use_AC_Pred, 1 );

  int CbpY = Cbp >> 2;
  if (MB_Type<=SKL_MB_INTER_Q) CbpY ^= 0x0f;
  VLC = &CBPY_B8[ CbpY ];
  Bits->Put_Bits( VLC->Val, VLC->Len );

  if (dQuant!=0)  { /* INTRA_Q or INTER_Q */ 
    SKL_ASSERT(dQuant>=-2 && dQuant<=2);
    Bits->Put_Bits( DQuant_Tab[dQuant+2], 2 );
  }

  if (Field_DCT>=0)     // interlace?
    Bits->Put_Bits( Field_DCT, 1 );

  const SKL_SAFE_INT Pos2 = Bits->Write_Bit_Pos();  
  C = In;
  for(blk=0; blk<6; ++blk)
  {
      // TODO: DC_Thresh here...
    Write_DC_Coeff(Bits, C[0], (blk<4));  // DC-coeff
    if (Last[blk]>0)
      Write_Coeffs( Bits, C, Zigzags[blk]+1, Last[blk]-1, B16_17_Tabs[1]);
    C += 64;
  }
  Texture_Bits += Bits->Write_Bit_Pos() - Pos2;

  Store_Zero_MV();
}

//////////////////////////////////////////////////////////
// P-MB coding

void SKL_MB_ENC::Encode_Inter(SKL_FBB * const Bits, SKL_INT16 In[12*64], int Fwd_Code)
{
  SKL_ASSERT(MB_Type <= SKL_MB_INTER4V);

    // time to really set the MB_Type
  if (dQuant!=0) {
    SKL_ASSERT(MB_Type<SKL_MB_INTER4V);
    MB_Type = SKL_MB_INTER_Q;
  }

  const SKL_VLC *VLC;
  VLC = &MCBPC_Inter_B7[ MB_Type ][ Cbp & 3 ];
  Bits->Put_Bits( VLC->Val, VLC->Len );

  if (MB_Type<=SKL_MB_INTER_Q && VOL->Is_S_VOP() && VOL->Sprite_Mode==SPRITE_GMC) 
    Bits->Put_Bits(MC_Sel, 1);

  VLC = &CBPY_B8[ (Cbp>>2) ^ 0xf ];
  Bits->Put_Bits( VLC->Val, VLC->Len );

  if (dQuant!=0) {  /* INTER_Q */
    SKL_ASSERT(dQuant>=-2 && dQuant<=2);
    Bits->Put_Bits( DQuant_Tab[dQuant+2], 2 );
  }

  if (Field_DCT>=0)   // interlace?  (this costs 0.1% in file size)
  {
    if (Cbp!=0) Bits->Put_Bits( Field_DCT, 1 );
    if (MB_Type<=SKL_MB_INTER_Q && !MC_Sel)  /* INTER or INTER_Q */
    {
      Bits->Put_Bits( Field_Pred, 1 );
      if (Field_Pred)
        Bits->Put_Bits( Field_Dir&3, 2 ); // Fwd Top/Bottom field select.
    }
      // Note: no field pred for INTER4V or INTRA, or GMC
  }

  if (!MC_Sel)    // no vector for GMC
  {
    const SKL_SAFE_INT Pos = Bits->Write_Bit_Pos();
    Write_Vector(Bits, dMVs[0], Fwd_Code);
    if (MB_Type==SKL_MB_INTER4V)
    {
      Write_Vector(Bits, dMVs[1], Fwd_Code);
      Write_Vector(Bits, dMVs[2], Fwd_Code);
      Write_Vector(Bits, dMVs[3], Fwd_Code);
    }
    else  /*  INTER or INTER_Q */
    {
      if (Field_Pred)
        Write_Vector(Bits, dMVs[1], Fwd_Code);
    }
    MV_Bits += Bits->Write_Bit_Pos() - Pos;
  }

  const SKL_SAFE_INT Pos2 = Bits->Write_Bit_Pos();  
  SKL_INT16 *C = In;
  for(int blk=0; blk<6; ++blk) {
    if (Last[blk]>=0)
      Write_Coeffs( Bits, C, SKL_MP4_I::Scan_Order[0], Last[blk], B16_17_Tabs[0]);
    C += 64;
  }
  Texture_Bits += Bits->Write_Bit_Pos() - Pos2;

  Set_Not_Intra();
}

//////////////////////////////////////////////////////////
// B-MB coding

void SKL_MB_ENC::Encode_Inter_B(SKL_FBB * const Bits, SKL_INT16 In[12*64],
                                int Fwd_Code, int Bwd_Code)
{
  int ModB1 = (B_Type==SKL_MB_DIRECT && dMVs[0][0]==0 && dMVs[0][1]==0);
  if (Cbp==0) {
    if (ModB1)
      Bits->Put_Bits( 1, 1 );   // ModB1
    else
      Bits->Put_Bits( 1, 2 );   // ModB2
  }
  else {
    Bits->Put_Bits( 0, 2 );     // ModB1/B2
    Bits->Put_Bits( Cbp, 6 );
    if (B_Type!=SKL_MB_DIRECT) {
      if (dQuant==-2) Bits->Put_Bits( 2, 2 );
      else if (dQuant==2) Bits->Put_Bits( 3, 2 );
      else {
        SKL_ASSERT(dQuant==0);
        Bits->Put_Bits( 0, 1 );
      }
    }
  }

  if (Field_DCT>=0)   // interlace?
  {
    if (Cbp!=0) Bits->Put_Bits( Field_DCT, 1 );
    if (B_Type!=SKL_MB_DIRECT)
    {
      Bits->Put_Bits( Field_Pred, 1 );
      if (Field_Pred) {
        if (B_Type!=SKL_MB_BWD)
          Bits->Put_Bits( Field_Dir&3, 2 ); // Fwd Top/Bottom field select.
        if (B_Type!=SKL_MB_FWD)
          Bits->Put_Bits( Field_Dir>>2, 2 ); // Bwd Top/Bottom field select.
      }
    }
  }

  const SKL_SAFE_INT Pos = Bits->Write_Bit_Pos();  
  if (B_Type == SKL_MB_DIRECT) {
    if (!ModB1)
      Write_Vector(Bits, dMVs[0], 1);   // FCode=1, always
  }
  else {
    if (B_Type!=SKL_MB_BWD) {
      Write_Vector(Bits, dMVs[0], Fwd_Code);
      if (Field_Pred)
        Write_Vector(Bits, dMVs[1], Fwd_Code);
    }
    if (B_Type!=SKL_MB_FWD) {
      Write_Vector(Bits, dMVs[2], Bwd_Code);
      if (Field_Pred)
        Write_Vector(Bits, dMVs[3], Bwd_Code);
    }
  }
  MV_Bits += Bits->Write_Bit_Pos() - Pos;

  const SKL_SAFE_INT Pos2 = Bits->Write_Bit_Pos();  
  SKL_INT16 *C = In;
  for(int blk=0; blk<6; ++blk) {
    if (Last[blk]>=0)
      Write_Coeffs( Bits, C, SKL_MP4_I::Scan_Order[0], Last[blk], B16_17_Tabs[0]);
    C += 64;
  }
  Texture_Bits += Bits->Write_Bit_Pos() - Pos2;

  Set_Not_Intra();
}

//////////////////////////////////////////////////////////
// encoder-only MB ops
//////////////////////////////////////////////////////////

void SKL_MB_ENC::Copy_8To16(SKL_INT16 Out[6*64]) const
{
  VOL->MB_Ops.Copy_16x8_8To16(Out+0*64, Y1,   YBpS);
  VOL->MB_Ops.Copy_16x8_8To16(Out+2*64, Y2,   YBpS);
  VOL->MB_Ops.Copy_8x8_8To16( Out+4*64, U,    BpS);
  VOL->MB_Ops.Copy_8x8_8To16( Out+5*64, V,    BpS);
}

void SKL_MB_ENC::Diff_8To16(SKL_INT16 Out[6*64]) const
{
  VOL->MB_Ops.Diff_16x8_8To16(Out+0*64, Y1,   YBpS);
  VOL->MB_Ops.Diff_16x8_8To16(Out+2*64, Y2,   YBpS);
  VOL->MB_Ops.Diff_8x8_8To16( Out+4*64, U,    BpS);
  VOL->MB_Ops.Diff_8x8_8To16( Out+5*64, V,    BpS);
}

//////////////////////////////////////////////////////////
// DC/AC prediction
//////////////////////////////////////////////////////////

int SKL_MB_ENC::Select_DC_AC_Pred(SKL_INT16 In[6*64],
                                  int Pred_Dirs[6])
{
  int i, blk;
  int Preds[6][7+1];  // 7 predictors/blk + 1 'worthiness' flag
  SKL_INT16 *C;
  int S_With_AC = 0;
  int S_Without_AC = 0;

  for(C=In, blk=0; blk<6; ++blk)
  {
    int Pred_DC;

    SKL_INT16 * const Left_AC = Lefts[blk];
    const SKL_MB_DATA * const Top = Tops[blk];
    SKL_MB_DATA * const Cur = Curs[blk];
    const int AC_Q = Quant;
    const int DC_Q = SKL_MP4_I::DC_Scales[(blk<4)][AC_Q-1];

    Preds[blk][7] = 1;          // a priori guess: it's worth predicting

      // first, deal with DC

    const SKL_INT16 a = Cur[-1].DC;
    const SKL_INT16 b = Top[-1].DC;
    const SKL_INT16 c = Top[ 0].DC;
    if (ABS(a-b)<ABS(c-b)) {    // vertical
      Pred_DC = c;
      if (Top[0].Q<=0)          // Potential prediction block is not intra. 
        Preds[blk][7] = 0;      // => Not worth AC predicting (dflt coeffs={0}).
      Pred_Dirs[blk] = 1;
    }
    else {                      // horizontal
      Pred_DC = a;
      if (Cur[-1].Q<=0)         // Potential prediction block is not intra. 
        Preds[blk][7] = 0;      // => Not worth AC predicting (dflt coeffs={0}).
      Pred_Dirs[blk] = 2;
    }

      // predict DC no matter what

    Cur->DC = C[0] * DC_Q;      // TODO: redundant with Dequant_XXX (except saturation?)
    Cur->Q = AC_Q;              // this will mark the block as 'Intra'
    SKL_ASSERT(C[0]>=0 && Pred_DC>=0);
    if (Pred_DC>0)
      C[0] -= ( Pred_DC + (DC_Q>>1) ) / DC_Q;

    if (!Preds[blk][7])         // done?
      goto End;


      // guess whether AC prediction is worth it or not

    if (Pred_Dirs[blk]==1) {    // vertical
      for(i=0; i<7; ++i) {
        Preds[blk][i] = C[i+1];
        if (Top->AC[i]!=0) {
          S_Without_AC += ABS(C[i+1]);
          if (Top[0].Q!=AC_Q) {
            int Corr = Top->AC[i]*Top[0].Q;
            Preds[blk][i] -= DIV_ROUND(Corr, AC_Q);
          }
          else Preds[blk][i] -= Top->AC[i];
          S_With_AC += ABS(Preds[blk][i]);
        }
        // else: if AC[i]==0, both predicted and
        // unpredicted coeffs will weight the same
        // in sums, and cancel each others. 
        // So we don't add either of them.
      }
    }
    else {                      // horizontal
      for(i=0; i<7; ++i) {
        Preds[blk][i] = C[8*(i+1)];
        if (Left_AC[i]!=0) {
          S_Without_AC += ABS(C[8*(i+1)]);
          if (Cur[-1].Q!=AC_Q) {
            int Corr = Left_AC[i]*Cur[-1].Q;
            Preds[blk][i] -= DIV_ROUND(Corr, AC_Q);
          }
          else Preds[blk][i] -= Left_AC[i];
          S_With_AC += ABS(Preds[blk][i]);
        }
      }
    }

End:
      // store for other predictions
    for(i=0; i<7; ++i) {
      Cur->AC[i] = C[i+1];
      Left_AC[i] = C[8*(i+1)];
    }

    C += 64;
  }

  if (S_Without_AC>S_With_AC)   // Ok. Go for it!
  {
    for(C=In, blk=0; blk<6; ++blk) {
      if (Preds[blk][7]) {
        if (Pred_Dirs[blk]==1) 
          for(i=0; i<7; ++i) C[i+1] = Preds[blk][i];
        else
          for(i=0; i<7; ++i) C[8*(i+1)] = Preds[blk][i];
      }
      C += 64;
    }
    return 1;
  }
  else return 0;                // was no avail...
}

//////////////////////////////////////////////////////////
//
//    Compressive part
//
// Let's explain a little bit what's going on. We're here
// to perform the following steps:
//
//   1) Import YUV 8b-data from picture to 16b DCT-coeffs C[]
//   2) FDCT( C[] )
//   3) C[] /= Q        (quantization)
//   4) C[] -= Preds[]  (remove prediction, if any)
//   5) Encode C[]      (into bitstream)
//   6) Decode C[]
//   7) C[] += Preds
//   8) C[] *= Q        (dequantization)
//   9) IDCT( C[] )     (backward transform)
//  10) Export reconstructed data to picture
//
//  Notes:
// . Using FDCT and scale-based quantization is not mandatory
// in itself. Only does the MPEG familly of hybrid encoder
// requires it. But you can instead try wavelets, cosmic rays-based
// quantization, whatever. :) The encoder (almost) doesn't care.
//
// . Also, the actual way spatio(-temporal) predictors are chosen
// pertain to the encoding format (here: MPEG4) only.
// The frame analyzer/motion estimator shouldn't care (much)
// about it. MPEG2 used to use different predictors, for instance.
//
// . Steps 6->10 are reserved to the decoder, but we
// have, as an encoder, to keep sync with what will
// be perceived by the decoder. 
//
//
// Anyway, the *only* lossy step is 3). Afterward, C[] 
// remain constant until step 8). Hence, we need not perform
// steps 6-7: just save C[] after quantization, and restore it 
// for step 8).
//
//   Now, we can reorder the job as:
//
// a) Import(C) | FDCT(C) | C/=Q | Copy C in C' | C'*=Q / IDCT(C') | Export(C')
// b) C -= Preds | Encode(C)
//
//  Step a) is function 'Decimate_Intra()', whereas step b) is 'Encode_Intra()'
//
// There are additional subtleties possible during the
//  FDCT| C/=Q | C*=Q | IDCT stage. In particular, one can save some
// transposes, at the expense of playing with zigzag orders, predicted
// directions, etc. One can also try to combine the C/=Q and C*=Q steps
// in a efficient manner, mix with DCTs, etc... Research is open.
//
// Finally, note that Inter coding is almost the same. Only does it requires
// some kind of motion estimation to Import() efficiently. But you can
// try random Motion Vectors too, no pb.
//
// That's it. Now, go!
//
//////////////////////////////////////////////////////////

void SKL_MB_ENC::Decimate_Intra(SKL_INT16 Out[12*64]) const
{
  Copy_8To16(Out+6*64);         // import

  SKL_INT16 *C = Out;           // Decimate
  const SKL_MP4_ENC_I * const Vol = (const SKL_MP4_ENC_I *)VOL;
  for(int blk=0; blk<6; ++blk) {
    const int DC_Q = SKL_MP4_I::DC_Scales[(blk<4)][Quant-1];
    Vol->Decimate_Intra(C, C+6*64, Quant, DC_Q);
    C += 64;
  }
  Copy_16To8(Out+6*64);         // transfer back decimated data
}

void SKL_MB_ENC::Decimate_Inter(SKL_INT16 Out[12*64])
{
  SKL_ASSERT(MC_Sel==0);

  if (MB_Type==SKL_MB_SKIPPED) {
    Predict_With_0MV(VOL->Copy_Ops, 1);
    Store_Zero_MV();
    return;
  }

    // Fwd predict

  int Skippable = (dQuant==0) && SKL_IS_ZERO_MV(MVs[0]) && (VOL->Coding!=S_VOP);

  Copy_8To16(Out + 6*64);              // save original data
  if (Field_Pred) {                    // form predictions
    Predict_Fields(MVs, VOL->Copy_Ops, 1, Field_Dir);
    Store_16x8_MV(&MVs2[0], &MVs[0]);
    SKL_COPY_MV(MVs[1], MVs2[0]);      // update left predictor
    if (Skippable) Skippable &= SKL_IS_ZERO_MV(MVs2[0]);
  }
  else {
    if (MB_Type!=SKL_MB_INTER4V) {
      Predict_With_1MV(MVs[0], VOL->Copy_Ops, 1);
      Store_16x16_MV();
    }
    else {
      Predict_With_4MV(&MVs[0], &MVs2[0], VOL->Copy_Ops, 1);
      if (Skippable) {
        Skippable &= SKL_IS_ZERO_MV(MVs [1]);
        Skippable &= SKL_IS_ZERO_MV(MVs2[0]);
        Skippable &= SKL_IS_ZERO_MV(MVs2[1]);
      }
    }
  }
  Diff_8To16(Out + 6*64);              // diff original/predicted data


  SKL_INT16 *C = Out;                  // Decimate and set Cbp
  Cbp = 0x00;
  const SKL_MP4_ENC_I * const Vol = (const SKL_MP4_ENC_I*)VOL;
  for(int blk=0; blk<6; ++blk)
  {
    const int SAV = Vol->Decimate_Inter(C, C+6*64, Quant);
    if (SAV>0) {
      Last[blk] = Find_Last(C, SKL_MP4_I::Scan_Order[0], 63);
      SKL_ASSERT(Last[blk]>=0);
      Cbp |= 1<<(5-blk);
    }
    else Last[blk] = -1;
    C += 64;
  }


  if (!Skippable || Cbp!=0)
    Add_16To8(Out + 6*64);             // transfer back decimated delta data  
  else
    MB_Type = SKL_MB_SKIPPED;          // won't be coded (don't transfer back data)
}

void SKL_MB_ENC::Decimate_Inter_GMC(SKL_INT16 Out[12*64])
{
  SKL_ASSERT(Field_Pred==0 && MC_Sel!=0);
  SKL_ASSERT(MB_Type==SKL_MB_INTER || MB_Type==SKL_MB_SKIPPED);

  if (MB_Type!=SKL_MB_SKIPPED) Copy_8To16(Out+6*64); // save original data
  Predict_GMC();
  if (MB_Type!=SKL_MB_SKIPPED) Diff_8To16(Out+6*64); // diff original/predicted data
  else return;                                       // Done

  SKL_INT16 *C = Out;                                // Decimate and set Cbp
  Cbp = 0x00;
  const SKL_MP4_ENC_I * const Vol = (const SKL_MP4_ENC_I *)VOL;
  for(int blk=0; blk<6; ++blk)
  {
    const int SAV = Vol->Decimate_Inter(C, C+6*64, Quant);
    if (SAV>0) {
      Last[blk] = Find_Last(C, SKL_MP4_I::Scan_Order[0], 63);
      SKL_ASSERT(Last[blk]>=0);
      Cbp |= 1<<(5-blk);
    }
    else Last[blk] = -1;
    C += 64;
  }

  const int Skippable = (dQuant==0);
  if (!Skippable || Cbp!=0) Add_16To8(Out+6*64);     // transfer back decimated delta data
  else MB_Type = SKL_MB_SKIPPED;                     // won't be coded (don't transfer back data)
}

//////////////////////////////////////////////////////////
// bitstream coding
//////////////////////////////////////////////////////////

static void Stuffed_Align(SKL_FBB * const Bits)    // hardcoded Table 6.2
{
  int Left = 8 - (Bits->Bits_Left & 7);
  Bits->Put_Bits( SKL_BMASKS::And[Left-1], Left );
  Bits->Flush_Write();
}

static void Write_User_Data(SKL_FBB * const Bits, int Len, const SKL_BYTE *Data)
{
  Bits->Flush_Write();
  Bits->Put_DWord( USER_DATA_CODE );
    // TODO: check data doesn't emulate a start code?
  while(Len-->0) Bits->Put_Bits( *Data++, 8 );
  Stuffed_Align(Bits);
}

static void Write_Matrix(SKL_FBB * const Bits, const SKL_BYTE *M)
{
  int Last = M[SKL_MP4_I::Scan_Order[0][63]];
  int i, j;
  for(j=62; j>0; --j) if (M[SKL_MP4_I::Scan_Order[0][j]]!=Last) break;
  for(i=0; i<=j; ++i) Bits->Put_Bits( M[SKL_MP4_I::Scan_Order[0][i]], 8 );
  if (j<63)
    Bits->Put_Bits(0, 8);
}

static void Write_Sprite_Comp(SKL_FBB * const Bits, int v)
{
  if (v) {
    int Size;
    const int Sign = (v<0);
    if (Sign) { 
      Size = SKL_BMASKS::Log2(-v);
      v += SKL_BMASKS::And[Size];
    }
    else {
      Size = SKL_BMASKS::Log2(v);
      v |= SKL_BMASKS::Or[Size];
    }
    SKL_ASSERT(Size>=0 && Size<15);
    Bits->Put_Bits(Spr_Tab_B33[Size].Val,Spr_Tab_B33[Size].Len);
    Bits->Put_Bits(v, Size);
  }
  else Bits->Put_Bits(Spr_Tab_B33[0].Val,Spr_Tab_B33[0].Len);
}

static void Write_Sprite_Trajectory(SKL_FBB * const Bits, const int Pts[][2], int Nb)
{
  for(int n=0; n<Nb; ++n) {
    Write_Sprite_Comp(Bits, Pts[n][0]);
    Bits->Put_Bits(1,1);    // marker bit
    Write_Sprite_Comp(Bits, Pts[n][1]);
    Bits->Put_Bits(1,1);    // marker bit
  }
}

  // section 6.2.3

void SKL_MP4_ENC_I::Write_VOL_Header(SKL_FBB * const Bits) const
{
  int Verid = (Quarter>0 || Reduced_VOP>=0 || Sprite_Mode>=SPRITE_GMC) ? 2 : 1;

  SKL_ASSERT(Bits->Is_Flushed() && _Need_VOL_Header!=0);

  Bits->Put_DWord(FIRST_CODE|0x00);    // VO start: start + id[0x00..0x1f]
  Bits->Put_DWord(VOL_CODE  |VOL_Id);  // VOL start: start code + id[0x20..0x2f]

  Bits->Put_Bits(0, 1);                // random access
  Bits->Put_Bits(1, 8);                // VO type indication (1=Simple Object)
  Bits->Put_Bits(1, 1);                // obj-layer identified.
  Bits->Put_Bits(Verid, 4);            // version id
  Bits->Put_Bits(1, 3);                // priority
  Bits->Put_Bits(1, 4);                // aspect-ratio = 1

  if (!Low_Delay) {                    // control params?
    Bits->Put_Bits(1, 1);
    Bits->Put_Bits(1, 2);              // Chroma format: 420
    Bits->Put_Bits(Low_Delay, 1);      // low delay. '1' means: no B-VOP in VOP
    Bits->Put_Bits(0, 1);              // vbv param -skip-
  }
  else Bits->Put_Bits(0, 1);           // -skip-

  Bits->Put_Bits(0, 2);                // VOL shape = rect.
  Bits->Put_Bits(1, 1);                // marker bit

  Bits->Put_Bits(Time_Frequency, 16);  // time increment
  Bits->Put_Bits(1, 1);                // marker bit

  if (Ticks_Per_VOP) {
    Bits->Put_Bits(1, 1);              // fixed rate = yes
    Bits->Put_Bits(Ticks_Per_VOP, Ticks_Bits);
  }
  else Bits->Put_Bits(0, 1);           // fixed rate = no

  Bits->Put_Bits(1, 1);                // marker bit
  Bits->Put_Bits(Width,  13);          // Width
  Bits->Put_Bits(1, 1);                // marker bit
  Bits->Put_Bits(Height, 13);          // Height
  Bits->Put_Bits(1, 1);                // marker bit

  Bits->Put_Bits(!!Interlace, 1);      // interlacing (impl note:Interlace=[0..6])
  Bits->Put_Bits(1, 1);                // overlapped motion comp.
  Bits->Put_Bits(Sprite_Mode, 
    Verid==1 ? 1 : 2);                 // sprite enable
  if (Sprite_Mode>SPRITE_NONE) {
    SKL_ASSERT(Sprite_Mode!=SPRITE_STATIC); // not supported. Only GMC is.
    Bits->Put_Bits(Sprite_Nb_Pts, 6);
    Bits->Put_Bits(Sprite_Accuracy, 2);
    Bits->Put_Bits(Sprite_Brightness, 1);
  }

  if (Shape!=0 && Verid>1)
    Bits->Put_Bits(1,1);               // sadct disable

  Bits->Put_Bits(Not_8b, 1);           // not 8 bit
  if (Not_8b) {
    Bits->Put_Bits(Quant_Prec, 4);     // quant prec
    Bits->Put_Bits(Bpp, 4);            // bits per pixel
  }

  Bits->Put_Bits(Get_Quant_Type(), 1); // quant type
  if (Get_Quant_Type()==1) {           // MPEG4 custom matrix
    Bits->Put_Bits(Custom_Intra, 1);   // intra matrix
    if (Custom_Intra)
      Write_Matrix(Bits, Intra_Matrix);
    Bits->Put_Bits(Custom_Inter, 1);   // inter matrix
    if (Custom_Inter)
      Write_Matrix(Bits, Inter_Matrix);
  }  

  if (Verid!=1)
    Bits->Put_Bits(Quarter, 1);          // QPel

  Bits->Put_Bits(1, 1);                  // complexity est.
  Bits->Put_Bits(!Resync, 1);            // resync marker disable
  Bits->Put_Bits(Data_Partitioned, 1);   // partitioned
  if (Data_Partitioned)
    Bits->Put_Bits(Rev_VLC, 1);          // reversible vlc

  if (Verid!=1) {
    Bits->Put_Bits(0, 1);                // new pred
    Bits->Put_Bits((Reduced_VOP>=0), 1); // reduced VOP
  }
  Bits->Put_Bits(0, 1);                  // scalability

  Stuffed_Align(Bits);

  if (Debug_Level==3) Print_Infos();
}

  // section 6.2.5

void SKL_MP4_ENC_I::Write_VOP_Header(SKL_FBB * const Bits,
                                     const SKL_MP4_FRAME *VOP) const
{
  SKL_ASSERT(Bits->Is_Flushed());

  if (VOP->Frame_Number==0)
    Write_User_Data(Bits, 1+strlen(ID_STRING), (const SKL_BYTE *)ID_STRING);

  Bits->Put_DWord(VOP_CODE);                   // 32b marker
  Bits->Put_Bits(VOP->Coding, 2);              // Coding type

  SKL_INT64 Elapsed = VOP->Time - Time_Ref*Time_Frequency;
  SKL_INT64 Seconds = Elapsed / Time_Frequency;
  if (Elapsed<0) Elapsed = 0;   // hoho, wrong time stamp, it seems...
  else Elapsed -= Seconds * Time_Frequency;

//  printf( "Ref:%lld Last_Coded:%lld Time:%lld Elapsed:%d Seconds:%d\n",
//    Time_Ref, Time_Last_Coded, VOP->Time, Elapsed, Seconds );
//  printf( "Tick bits:%d, Time_Freq:%d\n", Ticks_Bits, Time_Frequency);

  while(Seconds-->0) Bits->Put_Bits(1, 1);         // seconds elapsed
  Bits->Put_Bits(0, 1); 
  Bits->Put_Bits(1, 1);                            // marker bit
  Bits->Put_Bits((SKL_UINT32)Elapsed, Ticks_Bits); // time ticks increment
  Bits->Put_Bits(1, 1);                            // marker bit

  Bits->Put_Bits(1, 1);                            // coded

  if (VOP->Coding==P_VOP || VOP->Coding==S_VOP)
    Bits->Put_Bits(VOP->Rounding, 1);              // rounding type

  if ( VOP->Reduced_VOP>=0 && VOP->Coding!=B_VOP )
    Bits->Put_Bits(VOP->Reduced_VOP>0, 1);

  Bits->Put_Bits(VOP->DC_Thresh, 3);               // intra dc vlc thresh.

  if (Interlace) {                                 // interlacing mode
    Bits->Put_Bits(!!VOP->Top_Field_First, 1);     // display top field first
    Bits->Put_Bits(!!VOP->Alt_Vert_Scan, 1);       // alt. vert. scan
  }

  if (VOP->Coding==S_VOP && VOP->Sprite_Nb_Pts>0) {
    Write_Sprite_Trajectory(Bits, VOP->S_Warp_Pts, VOP->Sprite_Nb_Pts);
    SKL_ASSERT(Sprite_Brightness==0);              // no brightness change supported
  }

  Bits->Put_Bits(VOP->Quant, Quant_Prec);          // global frame quantizer

  if (VOP->Coding!=I_VOP)
    Bits->Put_Bits(VOP->Fwd_Code, 3);              // fcode
  if (VOP->Coding==B_VOP)
    Bits->Put_Bits(VOP->Bwd_Code, 3);              // bcode
}

//////////////////////////////////////////////////////////
// VOP Coding

void SKL_MP4_ENC_I::Write_I_VOP(SKL_FBB * const Bits)
{
  SKL_INT16 Base[12*64+SKL_ALIGN];
  SKL_INT16 * const In = (SKL_INT16*)SKL_ALIGN_PTR(Base, SKL_ALIGN);

  SKL_MB_ENC MB(this);
  while(MB.y<MB_H)
  {
    MB.Init_Scanline(this, 0);
    while(MB.x<MB_W)
    {
      MB.MB_Type = SKL_MB_INTRA;    // force INTRA type
      MB.Set_Final_Params();
      MB.Decimate_Intra(In);
      MB.Encode_Intra(Bits, In, 1);
      MB++;
    }
    if (Debug_Level==2) Dump_Line(0, &MB);
    if (Slicer) Slicer(Cur, MB.y*16, 16, Slicer_Data);
    MB.y++;
  }
  Texture_Bits = MB.Get_Texture_Bits();
  MV_Bits = MB.Get_MV_Bits();
}

void SKL_MP4_ENC_I::Write_P_VOP(SKL_FBB * const Bits)
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
      if (MB.MB_Type==SKL_MB_INTRA) MB.Decimate_Intra(In);
      else                          MB.Decimate_Inter(In);

      if (MB.MB_Type==SKL_MB_SKIPPED)
      {
        Bits->Put_Bits( 1, 1 );          // not_coded
        MB.Set_Not_Intra();
      }
      else {
        Bits->Put_Bits( 0, 1 );          // coded
        if (MB.MB_Type>=SKL_MB_INTRA) MB.Encode_Intra(Bits, In, 0);
        else                          MB.Encode_Inter(Bits, In, Fwd_Code);
      }
      MB.Store_Map_Infos();
      MB++;
    }
    if (Debug_Level==2) Dump_Line(0, &MB);
    if (Slicer) Slicer(Cur, MB.y*16, 16, Slicer_Data);
    MB.y++;
  }
  Texture_Bits = MB.Get_Texture_Bits();
  MV_Bits = MB.Get_MV_Bits();

  if (Debug_Level==1) Dump_MVs(Cur, 2);  // will trash the encoded frame
}

void SKL_MP4_ENC_I::Write_S_VOP(SKL_FBB * const Bits)
{
  SKL_INT16 Base[12*64+SKL_ALIGN];
  SKL_INT16 * const In = (SKL_INT16*)SKL_ALIGN_PTR(Base, SKL_ALIGN);

  SKL_ASSERT(Sprite_Nb_Pts>0);  // otherwise, it should be a P-VOP
  SKL_ASSERT(Sprite_Mode==SPRITE_GMC && Sprite_Brightness==0);
  GMC_Ops.Setup(Width, Height, S_Warp_Pts, Sprite_Nb_Pts, Sprite_Accuracy);

  SKL_MB_ENC MB(this);
  while(MB.y<MB_H)
  {
    MB.Init_Scanline(this, 0);
    while(MB.x<MB_W)
    {
      MB.Set_Type();
      SKL_ASSERT(MB.MB_Type!=SKL_MB_SKIPPED);
      if (MB.MB_Type==SKL_MB_INTRA) MB.Decimate_Intra(In);
      else if (MB.MC_Sel)           MB.Decimate_Inter_GMC(In);
      else                          MB.Decimate_Inter(In);

      if (MB.MB_Type==SKL_MB_SKIPPED)
      {
        Bits->Put_Bits( 1, 1 );       // 'GMC' not_coded (but predicted...)
        MB.MB_Type = SKL_MB_INTER;
        MB.Set_Not_Intra();
      }
      else {
        Bits->Put_Bits( 0, 1 );       // coded
        if (MB.MB_Type>=SKL_MB_INTRA) MB.Encode_Intra(Bits, In, 0);
        else                          MB.Encode_Inter(Bits, In, Fwd_Code);
      }
      MB.Store_Map_Infos();
      MB++;
    }
    if (Debug_Level==2) Dump_Line(0, &MB);
    if (Slicer) Slicer(Cur, MB.y*16, 16, Slicer_Data);
    MB.y++;
  }
  Texture_Bits = MB.Get_Texture_Bits();
  MV_Bits = MB.Get_MV_Bits();

  if (Debug_Level==1) Dump_MVs(Cur, 2); // will trash the encoded frame
}

void SKL_MP4_ENC_I::Write_B_VOP(SKL_FBB * const Bits)
{
  SKL_INT16 Base[6*64+6*64+SKL_ALIGN];
  SKL_INT16 * const In = (SKL_INT16*)SKL_ALIGN_PTR(Base, SKL_ALIGN);

  if (Time_TFrame==0)       // First B-VOP. => store Tframe of section 7.7.2.2
    Time_TFrame = Cur->Time_Ticks - Time_Last_Coded;

  SKL_MB_ENC MB(this);
  while(MB.y<MB_H)
  {
    MB.Init_Scanline(this, 0);
    while(MB.x<MB_W)
    {
      MB.Set_Type();
      if (MB.MB_Type==SKL_MB_INTRA)
      {
        // Oops!! No INTRA in B-VOP!!!
        fprintf(stderr, "INTRA requested in B-VOP!!\n");
      }
      MB.Decimate_Inter(In);

      if (MB.MB_Type==SKL_MB_SKIPPED)
      {
        Bits->Put_Bits( 1, 1 );          // not_coded
        MB.Store_Zero_MV();
        MB.Set_Not_Intra();
      }
      else {
        Bits->Put_Bits( 0, 1 );          // coded
        MB.Encode_Inter_B(Bits, In, Fwd_Code, Bwd_Code);
      }
      MB.Store_Map_Infos();
      MB++;
    }
    if (Debug_Level==2) Dump_Line(0, &MB);
    if (Slicer) Slicer(Cur, MB.y*16, 16, Slicer_Data);
    MB.y++;
  }
  Texture_Bits = MB.Get_Texture_Bits();
  MV_Bits = MB.Get_MV_Bits();

  if (Debug_Level==1)  Dump_MVs(Cur, 2); // will trash frame
}

void SKL_MP4_ENC_I::Code_Frame(SKL_BYTE * const Buf, int Max_Len,
                                       const SKL_MP4_FRAME * const Frame)
{
    // Codes 'Cur' frame into bitstream

  SKL_FBB Bits(Buf);

  if (_Need_VOL_Header) {
    if (_Emit_SEQ_Codes==1) {
      Bits.Put_DWord(SEQ_START_CODE);
      Bits.Put_Bits( 0x02, 8 );   // Simple Profile, Level 2
      Bits.Flush_Write();
      _Emit_SEQ_Codes = 2;
    }
    Write_VOL_Header(&Bits);
    _Need_VOL_Header = 0;
  }

  Write_VOP_Header(&Bits, Frame);

  Set_Rounding();

  Cur->Coding = Frame->Coding;

  if (Slicer) Slicer(Cur, 0, 0, Slicer_Data); // "start of scan" call

  if (Frame->Coding==I_VOP) {
    if (Reduced_VOP<1) Write_I_VOP(&Bits);
    else               Write_Reduced_I_VOP(&Bits);
  }
  else if (Frame->Coding==B_VOP) Write_B_VOP(&Bits);
  else {      /* P/S-VOP */             
    if (Reduced_VOP<1) {
      if (Frame->Coding==P_VOP) Write_P_VOP(&Bits);
      else                      Write_S_VOP(&Bits);
    }
    else                        Write_Reduced_P_VOP(&Bits);
  }
  Make_Edges_Enc(Cur);  // replicate edges of *decimated* frame
  Switch_Off();

  if (Slicer) Slicer(Cur, Cur->Height, 0, Slicer_Data); // "end of scan" call

  Stuffed_Align(&Bits);

  Coded_Bits = 8*(Bits.Write_Pos() - Buf);    // done coding.
}

//////////////////////////////////////////////////////////
// Input analysis and frame coding preparation
//////////////////////////////////////////////////////////

void SKL_MP4_ENC_I::Setup_VOL_Params()
{
  int IParam;

  if (Get_Analyzer()->Get_Param( "bframe", &IParam ))
    if (Low_Delay!=(!IParam)) {
      _Need_VOL_Header = 1;
      Low_Delay = (!IParam);
    }

  {
    int Mode, Acc, Nb;
    if (Get_Analyzer()->Get_Param( "gmc-mode", &Mode ) &&
        Get_Analyzer()->Get_Param( "gmc-pts", &Nb ) &&
        Get_Analyzer()->Get_Param( "gmc-accuracy", &Acc ) ) {
      Mode = (Mode>=0) ? SPRITE_GMC : SPRITE_NONE;
      if (Frame_Number==0 || Sprite_Mode!=Mode)
        _Need_VOL_Header = 1;
      Sprite_Mode = Mode;
      if (Mode==SPRITE_GMC && (Sprite_Nb_Pts!=Nb || Sprite_Accuracy!=Acc))
      {
        _Need_VOL_Header = 1;
        Sprite_Brightness = 0;
        Sprite_Accuracy   = Acc;
        Sprite_Nb_Pts     = Nb;
      }
    }
  }

  if (Get_Analyzer()->Get_Param( "subpixel", &IParam )) {
    int Sub = (IParam==2);
    if (Quarter!=Sub) {
      _Need_VOL_Header = 1;
      Quarter = Sub;
    }
  }

  if (Get_Analyzer()->Get_Param( "quant-type", &IParam ))
    if (Get_Quant_Type()!=IParam) {
      _Need_VOL_Header = 1;
      Set_Quant_Type( IParam );
    }

  int Intl = 0;
  if (Get_Analyzer()->Get_Param( "interlace-dct", &IParam ))
    Intl |= IParam;       // bits 0,1
  if (Get_Analyzer()->Get_Param( "interlace-field", &IParam ))
    Intl |= (IParam<<2);  // bits 2,3
  if (Interlace!=Intl) {
    _Need_VOL_Header = 1;
    Interlace = Intl;
  }

  if (Get_Analyzer()->Get_Param( "frequency", &IParam ))
    if (Time_Frequency!=IParam) {
      _Need_VOL_Header = 1;
      Set_Time_Frequency( IParam );
    }

  if (Get_Analyzer()->Get_Param( "reduced-frame", &IParam )) {
    if ((Reduced_VOP>=0)!=(IParam>=0)) {
      _Need_VOL_Header = 1;
      if (IParam>=0) Reduced_VOP = 0;
      else Reduced_VOP = -1;
    }
  }
}

void SKL_MP4_ENC_I::Setup_Frame_Params(SKL_MP4_FRAME *Frame)
{
  int Param;

    // we ask the analyzer for the final non-VOL params. 
    // It might have better hints than we do.

  Get_Analyzer()->Get_Param( "quant", &Frame->Quant );
  Get_Analyzer()->Get_Param( "fcode", &Frame->Fwd_Code );
  if (Frame->Coding==B_VOP)
    Get_Analyzer()->Get_Param( "fcode-bwd", &Frame->Bwd_Code );

  if (Frame->Reduced_VOP>=0) Frame->Reduced_VOP = 0;
  if (Frame->Coding != B_VOP && Frame->Coding != S_VOP) {
    int Reduc;
    if (Get_Analyzer()->Get_Param( "reduced-frame", &Reduc)) {
      if (Reduc>1) Reduc = 1;
      Frame->Reduced_VOP = Reduc;
    }
  }

  if (Frame->Coding==P_VOP || Frame->Coding==S_VOP) {
    if (!Get_Analyzer()->Get_Param( "rounding", &Frame->Rounding))
      Frame->Rounding ^= 1;
  }
  else {
    if (Frame->Coding==B_VOP)
      Frame->Rounding = 0;
  }

  if (Frame->Coding==S_VOP) {

    Get_Analyzer()->Get_Param( "gmc-mode", &Param );
    if (Param>0) {
      Frame->Coding = S_VOP;
      Sprite_Mode   = SPRITE_GMC;
      Get_Analyzer()->Get_Param( "gmc-pts", &Sprite_Nb_Pts);
      const int (*Warps)[2] = (int (*)[2])Get_Analyzer()->Get_Param( "gmc-warp-pts" );
      for(int n=0; n<Sprite_Nb_Pts; ++n) {
        S_Warp_Pts[n][0] = Warps[n][0];
        S_Warp_Pts[n][1] = Warps[n][1];
      }
    }
    else {
      Frame->Coding = P_VOP;
      Sprite_Mode = SPRITE_NONE;
    }
  }

  Get_Analyzer()->Get_Param( "inter-threshold", &_Inter_Coding_Threshold );
  
  Get_Analyzer()->Get_Param( "use-trellis", &Param );
  Set_Trellis_Usage(!!Param);

  Cur->Coding = Frame->Coding;


  /* TODO: DC_Thresh? Alt_Vert_Scan? ... */
}

//////////////////////////////////////////////////////////
// Main entry point
//////////////////////////////////////////////////////////

int SKL_MP4_ENC_I::Encode(SKL_BYTE *Buf, int Max_Len)
{
  SKL_ASSERT(Width!=0 && Height!=0 && Get_Analyzer()!=0);

  if (Last==0) {  // no user input. Check if we have a pending B-VOP in store...
    return 0;
  }
  SKL_MP4_FRAME *Frame = this;
  Cur = Last;                       // get Input frame
  Last = 0;                         // mark it consumed
  Frame->Time = Cur->Time_Ticks;    // set time
  Cur->Map = All_Maps[Cur_Map].Map; // assign analysis map
  Cur->MV  = All_Maps[Cur_Map].MV;  // assign analysis MV map
  if (++Cur_Map==Nb_Maps)
    Cur_Map = 0;

  Future->Map = 0;                  // sanity check
  Future->MV = 0;                   // ""

    // Analyze new frame -> decide: Coding, MB types, Quant, dQ, ...

      // 1rst pass: Motion analysis

  Frame->Coding = Get_Analyzer()->Analyze(this);

  if (Frame->Coding==I_VOP && _Key_VOL_Headers)
    _Need_VOL_Header = 1;

  Setup_VOL_Params();  // might trigger a _Need_VOL_Header

    // We, as encoder, have the final word about frame Coding type.

  if (_Need_VOL_Header)
  {
      // it's not a real scene change. Just a checkpoint: we must
      // insert an I-VOP after a VOL header) => Don't reset the MVs!
      // They might be useful for the next frame analysis.
    Frame->Coding = I_VOP;
  }

  if (Frame->Coding==B_VOP) {}
  else if (Frame->Coding==S_VOP) {}

    // 2nd pass for spatial analysis (dQ)

  Get_Analyzer()->Analyze_dQ(this, Frame->Coding==B_VOP);

    // Frame params are now ready => code bitstream syntax

  Setup_Frame_Params(Frame);
  Code_Frame(Buf, Max_Len, this);

    // send some stats to the analyzer

  Get_Analyzer()->Post_Coding_Update( this );
  Frame_Number++;

    // display debug infos

  if (Debug_Level) {
    if (Debug_Level==4) Dump_Map(Cur, 0);
    else if (Debug_Level==5) Dump_Map(Cur, 1);
    else if (Debug_Level==6) Dump_Map(Cur, 2);
  }

    // reorder frames if needed

  int Ticks;    // save current time first, before swapping
  Get_Analyzer()->Get_Param( "frame-ticks", &Ticks );
  SKL_UINT64 Next_Time_Ticks = Cur->Time_Ticks + Ticks;

  if (Frame->Coding!=B_VOP) {
    Aux    = Past;
    Past   = Cur;
    Cur    = Future;
    Future = Aux;
    Aux    = Cur;
    Time_Last_Coded = Past->Time_Ticks;
    Time_Ref = Past->Time_Ticks / Time_Frequency;   // sync-point

#if 0
       // secure tickers every 65k-ticks, to avoid overflows
    if (Time_Last_Coded>0x10000) {
      Time_Ref -= 0x10000;
      Time_Last_Coded -= 0x10000;
      Next_Time_Ticks -= 0x10000 * Time_Frequency;
    }
#endif
  }

    // setup time of next-to-be-encoded frame, just in
    // case the user forgets to (or doesn't care doing so).

  Aux->Time_Ticks = Next_Time_Ticks;
  Aux->Time = (double)(SKL_INT64)Next_Time_Ticks / (double)(SKL_INT64)Time_Frequency;

  Aux->Map = 0; // sanity check
  Aux->MV  = 0; // ""
  Aux->Coding = -1; // sanity check

  return Coded_Bits/8;
}

//////////////////////////////////////////////////////////
// class SKL_MP4_ENC_I
//////////////////////////////////////////////////////////

SKL_MP4_ENC_I::SKL_MP4_ENC_I(SKL_MEM_I *Mem)
  : SKL_MP4_I(Mem)
  , SKL_MP4_ENC()
  , _Dflt_Analyzer(Skl_MP4_Get_Default_Analyzer(Mem))
{
  _Buf_Size     = 0;
  _Buf          = 0;
  _Buf_Len      = 0;
  
  _In_Pic = 0;

  _Need_VOL_Header = 1;
  _Key_VOL_Headers = 0;
   _Emit_SEQ_Codes = 0;
  _Analyzer = 0;
  _Use_Trellis = -1;
  Set_Trellis_Usage(1);
  Init_VOL(0);
  Set_Analyzer( _Dflt_Analyzer );
  Clear_Aux();
}

SKL_MP4_ENC_I::~SKL_MP4_ENC_I()
{
  Clear_Aux();
  Clear_Buf();
  if (_Analyzer!=0) _Analyzer->Shut_Down();
  _Analyzer = 0;
  Skl_MP4_Destroy_Default_Analyzer( _Dflt_Analyzer );
}

SKL_MP4_ANALYZER *SKL_MP4_ENC_I::Set_Analyzer(SKL_MP4_ANALYZER *Anl)
{
  SKL_MP4_ANALYZER *Previous = _Analyzer;
  if (_Analyzer!=0) _Analyzer->Shut_Down();
  _Analyzer = (Anl==0) ? _Dflt_Analyzer : Anl;
  _Analyzer->Wake_Up(Previous);
  return Previous;
}

SKL_MP4_ANALYZER *SKL_MP4_ENC_I::Get_Analyzer() const
{
  return _Analyzer;
}

void SKL_MP4_ENC_I::Set_Slicer(SKL_MP4_SLICER Slicer, SKL_ANY Slicer_Data) {
  SKL_MP4_I::Set_Slicer(Slicer, Slicer_Data);
}

SKL_MEM_I *SKL_MP4_ENC_I::Set_Memory_Manager(SKL_MEM_I *Mem)
{
  return SKL_MP4_I::Set_Memory_Manager(Mem);
}

void SKL_MP4_ENC_I::Set_Custom_Matrix(int Intra, const SKL_BYTE *M) {
  if (Set_Matrix(Intra, M))
    _Need_VOL_Header = 1;
}

int SKL_MP4_ENC_I::Ioctl(SKL_CST_STRING Param)
{
  if (Param==0) return -1;
  if (!strcmp(Param, "emit-key-headers")) {
    _Key_VOL_Headers = 1;
    return 1;
  }
  else if (!strcmp(Param, "no-emit-key-headers")) {
    _Key_VOL_Headers = 0;
    return 1;
  }
  else if (!strcmp(Param, "emit-sequence-codes")) {
    _Emit_SEQ_Codes = 1;
    return 1;
  }
  else if (!strcmp(Param, "no-emit-sequence-codes")) {
    _Emit_SEQ_Codes = 0;
    return 1;
  }

  return 0;
}

//////////////////////////////////////////////////////////

void SKL_MP4_ENC_I::Alloc_Aux() { /* nothing more needed, for now */ }
void SKL_MP4_ENC_I::Clear_Aux() { /* nothing more needed, for now */ }

SKL_MP4_PIC *SKL_MP4_ENC_I::Prepare_For_Input(int W, int H)
{
  if (W<=0 || W>2048 || H<=0 || H>2048)
    return 0;

  if (Width!=W || Height!=H)
  {
    Init_Pics(W, H, 3, 2);  // will store new W/H
    Alloc_Aux();            // init our own rest
    _Need_VOL_Header = 1;      
  }

   // user input is always 'Aux'
   // Time_Ticks should be ok from last encoding
  Last = Aux;
  return Last;
}

void SKL_MP4_ENC_I::Set_CPU( SKL_CPU_FEATURE Cpu )
{
  SKL_MP4_I::Set_CPU(Cpu);
  Set_Trellis_Usage(_Use_Trellis);
}

void SKL_MP4_ENC_I::Set_Trellis_Usage(const int Do_It)
{
  _Use_Trellis = Do_It;
}

//////////////////////////////////////////////////////////
// Interface Wrappers

void SKL_MP4_ENC_I::Set_Debug_Level(int Level) {
  SKL_MP4_I::Set_Debug_Level( Level );
}

const SKL_MP4_PIC *
SKL_MP4_ENC_I::Prepare_Next_Frame(int Width, int Height)
{
  _In_Pic = Prepare_For_Input(Width, Height);
  return _In_Pic;
}

const SKL_MP4_PIC *SKL_MP4_ENC_I::Get_Next_Frame() const
{
  return _In_Pic;
}

const SKL_MP4_PIC *SKL_MP4_ENC_I::Get_Last_Coded_Frame() const
{
  return Past;   // TODO: this is wrong for B-vop
}

void SKL_MP4_ENC_I::Get_All_Frames(SKL_MP4_PIC *Pic) const
{
  SKL_MP4_I::Get_All_Frames(Pic);
}

int SKL_MP4_ENC_I::Encode()
{
  size_t Needed_Size = (_In_Pic->Width*_In_Pic->Height)*2; // TODO: safety factor?

  int Buf_Size_Max = 0;
  if (Get_Analyzer()->Get_Param("buffer-size", &Buf_Size_Max))
    if (Buf_Size_Max>0 && Needed_Size>(size_t)Buf_Size_Max)
      Needed_Size = Buf_Size_Max;

  if (Needed_Size<4096) Needed_Size = 4096;
  Check_Buf_Size( Needed_Size );

  _Buf_Len = Encode(_Buf, _Buf_Len);
  return _Buf_Len;
}

int SKL_MP4_ENC_I::Finish_Encoding()
{
  // TODO: Check for a pending B-VOP...
  // TODO: should we emit a SEQ_END code?
  
  if (_Emit_SEQ_Codes) {
    SKL_FBB Bits(_Buf);
    Bits.Put_DWord(SEQ_END_CODE);
    _Buf_Len = 4;
    return 4;
  }
  return 0;
}

const SKL_BYTE *SKL_MP4_ENC_I::Get_Bits() const {
  return _Buf;
}

int SKL_MP4_ENC_I::Get_Bits_Length() const {
  return _Buf_Len;
}

void SKL_MP4_ENC_I::Check_Buf_Size(const size_t Needed_Size)
{
  if (Needed_Size>_Buf_Size)
  {
    SKL_BYTE *New_Buf = (Mem!=0) ? (SKL_BYTE*)Mem->New( Needed_Size )
                                 : (SKL_BYTE*)malloc( Needed_Size );
    if (New_Buf==0)
      Skl_Throw( SKL_MEM_EXCEPTION("Byte buffer", Needed_Size) );
    if (_Buf_Len>0)
      SKL_MEMCPY(New_Buf, _Buf, _Buf_Len);
    Clear_Buf();
    _Buf      = New_Buf;
    _Buf_Size = Needed_Size;
  }
  else {
    // TODO: shrink the _Buf from time to time?
  }
}

void SKL_MP4_ENC_I::Clear_Buf() {
  if (_Buf_Size>0) {
    if (Mem) Mem->Delete( _Buf, _Buf_Size );
    else free( _Buf );
  }
  _Buf_Size = 0;
  _Buf = 0;
  /* Don't touch _Len, here. We might just be swapping buffers.. */
}


//////////////////////////////////////////////////////////
// SKL_MP4_ENC
//////////////////////////////////////////////////////////

SKL_MP4_ENC::SKL_MP4_ENC() {}
SKL_MP4_ENC::~SKL_MP4_ENC() {}

//////////////////////////////////////////////////////////
// C factory

extern "C" {

SKL_EXPORT
SKL_MP4_ENC *Skl_MP4_New_Encoder() { 
  return (SKL_MP4_ENC*)::new SKL_MP4_ENC_I((SKL_MEM_I*)0);
}


SKL_EXPORT
void Skl_MP4_Delete_Encoder(SKL_MP4_ENC *Enc) { ::delete (SKL_MP4_ENC_I*)Enc; }

} // extern "C"

//////////////////////////////////////////////////////////

