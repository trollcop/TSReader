/********************************************************
 * Some code. Copyright (C) 2003 by Pascal Massimino.   *
 * All Rights Reserved.      (http://skal.planet-d.net) *
 * For Educational/Academic use ONLY. See 'LICENSE.TXT'.*
 ********************************************************/
/*
 * skl_mpg4_dec.cpp
 *
 * MPEG4 decoder
 ********************************************************/

#include "./skl_mpg4i.h"
#include "skl_syst/skl_exception.h"

//////////////////////////////////////////////////////////
// Decoding tables & values
//////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////

struct SKL_VLC { SKL_INT16 Val, Len; };
struct SKL_DCT_VLC { SKL_INT16 Run; SKL_INT8 Level, Len; };

  // table 6-14. Shape type
#define GRAYSCALE   3
#define BINARY_ONLY 2
#define BINARY      1
#define RECTANGULAR 0

  // Table 6-21
static const int DC_Thresh_Tab[] = { 512, 13, 15, 17, 19, 21, 23, 0 };

  // Table 6-27
static const int DQuant_Tab[4] = {-1,-2, 1, 2 };
  // Table 6-28
static const int DBQuant_Tab[4] = { 0, 0,-2, 2 };

  // Table 7-1
const int SKL_MP4_I::DC_Scales[2][31] = {   // index: [Lum][AC_Q-1]
 {  /* DC_Q_chroma = (Q<5) ? 8 : (Q<24) ? (Q+13)/2 : Q-6; */
   8,  8,  8,  8,
   9,  9, 10, 10, 11, 11, 12, 12, 13, 13, 14, 14, 15, 15, 16, 16, 17, 17, 18,
  18, 19, 20, 21, 22, 23, 24, 25 },
 {  /* DC_Q_Lum = (Q<5) ? 8 : (Q<9) ? 2*Q : (Q<25) ? Q+8 : Q*2-16; */
   8,  8,  8,  8,
  10, 12, 14, 16,
  17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31,
  32, 34, 36, 38, 40, 42, 44, 46 }
};

  // Table 7-6 (K=4)  (modified)
const int SKL_MB::Rnd_Tab_76[16] = { 0, 0, 0, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1 };
  // Table 7-8 (K=2)  (modified)
const int SKL_MB::Rnd_Tab_78[ 8] = { 0,    0,    1,    1,    0,    0,    0,    1    };
  // Table 7-9 (K=1)  (modified)
const int SKL_MB::Rnd_Tab_79[ 4] = { 0,          1,          0,          0          };

  // Table B-4
static const SKL_VLC BType_Tab_B4[15] = {
                      { SKL_MB_FWD,    4},{ SKL_MB_BWD,    3},{ SKL_MB_BWD,    3}
, { SKL_MB_INTERP, 2},{ SKL_MB_INTERP, 2},{ SKL_MB_INTERP, 2},{ SKL_MB_INTERP, 2}
, { SKL_MB_DIRECT, 1},{ SKL_MB_DIRECT, 1},{ SKL_MB_DIRECT, 1},{ SKL_MB_DIRECT, 1}
, { SKL_MB_DIRECT, 1},{ SKL_MB_DIRECT, 1},{ SKL_MB_DIRECT, 1},{ SKL_MB_DIRECT, 1}
};

  // Table B-6 packed: MB_Type = bits 2,3,4 / CBPC for blocks 5,6 = bits 0,1

static const SKL_VLC MCBPC_Intra_B6_0[63] = {
  {17, 6},{18, 6},{19, 6},{16, 4},{16, 4},{16, 4},{16, 4}
, {13, 3},{13, 3},{13, 3},{13, 3},{13, 3},{13, 3},{13, 3},{13, 3}
, {14, 3},{14, 3},{14, 3},{14, 3},{14, 3},{14, 3},{14, 3},{14, 3}
, {15, 3},{15, 3},{15, 3},{15, 3},{15, 3},{15, 3},{15, 3},{15, 3}
, {12, 1},{12, 1},{12, 1},{12, 1},{12, 1},{12, 1},{12, 1},{12, 1}
, {12, 1},{12, 1},{12, 1},{12, 1},{12, 1},{12, 1},{12, 1},{12, 1}
, {12, 1},{12, 1},{12, 1},{12, 1},{12, 1},{12, 1},{12, 1},{12, 1}
, {12, 1},{12, 1},{12, 1},{12, 1},{12, 1},{12, 1},{12, 1},{12, 1}
};

  // Table B-7 packed: MB_Type = bits 2,3,4 / CBPC for blocks 5,6 = bits 0,1

static const SKL_VLC MCBPC_Inter_B7_0[13+1] = {
  { 0x00, 0 }, // special code for leading bit = 1  

  {12, 4},{ 2, 3},{ 2, 3},{ 1, 3},{ 1, 3}
, { 8, 2},{ 8, 2},{ 8, 2},{ 8, 2},{ 4, 2},{ 4, 2},{ 4, 2},{ 4, 2}
};

static const SKL_VLC MCBPC_Inter_B7_1[46] = {
  {19, 8},{18, 8},{17, 8},{ 7, 8},{14, 7},{14, 7}
, {13, 7},{13, 7},{11, 7},{11, 7},{15, 6},{15, 6},{15, 6},{15, 6}
, {10, 6},{10, 6},{10, 6},{10, 6},{ 9, 6},{ 9, 6},{ 9, 6},{ 9, 6}
, { 6, 6},{ 6, 6},{ 6, 6},{ 6, 6},{ 5, 6},{ 5, 6},{ 5, 6},{ 5, 6}
, {16, 5},{16, 5},{16, 5},{16, 5},{16, 5},{16, 5},{16, 5},{16, 5}
, { 3, 5},{ 3, 5},{ 3, 5},{ 3, 5},{ 3, 5},{ 3, 5},{ 3, 5},{ 3, 5}
};

  // Table B-8. CBPY (bits 2,3,4,5)
static const SKL_VLC CBPY_Tab[62] = {
  {24, 6},{36, 6},{32, 5},{32, 5},{16, 5},{16, 5}
, { 8, 5},{ 8, 5},{ 4, 5},{ 4, 5},{ 0, 4},{ 0, 4},{ 0, 4},{ 0, 4}
, {48, 4},{48, 4},{48, 4},{48, 4},{40, 4},{40, 4},{40, 4},{40, 4}
, {56, 4},{56, 4},{56, 4},{56, 4},{20, 4},{20, 4},{20, 4},{20, 4}
, {52, 4},{52, 4},{52, 4},{52, 4},{12, 4},{12, 4},{12, 4},{12, 4}
, {44, 4},{44, 4},{44, 4},{44, 4},{28, 4},{28, 4},{28, 4},{28, 4}
, {60, 2},{60, 2},{60, 2},{60, 2},{60, 2},{60, 2},{60, 2},{60, 2}
, {60, 2},{60, 2},{60, 2},{60, 2},{60, 2},{60, 2},{60, 2},{60, 2}
};


  // Table B-12. Motion vectors multiplied by 2, without 
  // sign (trailing bit) and leading bit (dealt with separately)
  // Length is incremented by 1.
static const SKL_VLC MV_Tab_B12_0[7] = {
  { 3, 4},{ 2, 3},{ 2, 3},{ 1, 2},{ 1, 2},{ 1, 2},{ 1, 2}
};
static const SKL_VLC MV_Tab_B12_1[60] = {
  {24,10},{23,10},{22,10},{21,10}
, {20,10},{19,10},{18,10},{17,10},{16,10},{15,10},{14,10},{13,10}
, {12,10},{11,10},{10, 9},{10, 9},{ 9, 9},{ 9, 9},{ 8, 9},{ 8, 9}
, { 7, 7},{ 7, 7},{ 7, 7},{ 7, 7},{ 7, 7},{ 7, 7},{ 7, 7},{ 7, 7}
, { 6, 7},{ 6, 7},{ 6, 7},{ 6, 7},{ 6, 7},{ 6, 7},{ 6, 7},{ 6, 7}
, { 5, 7},{ 5, 7},{ 5, 7},{ 5, 7},{ 5, 7},{ 5, 7},{ 5, 7},{ 5, 7}
, { 4, 6},{ 4, 6},{ 4, 6},{ 4, 6},{ 4, 6},{ 4, 6},{ 4, 6},{ 4, 6}
, { 4, 6},{ 4, 6},{ 4, 6},{ 4, 6},{ 4, 6},{ 4, 6},{ 4, 6},{ 4, 6}
};
static const SKL_VLC MV_Tab_B12_2[14] = {
  {32,12},{31,12},{30,11},{30,11},{29,11},{29,11}
, {28,11},{28,11},{27,11},{27,11},{26,11},{26,11},{25,11},{25,11}
};

  // Table B-13, restricted to code size <= 3
static const SKL_VLC DC_Size_Lum_B13_0[7] = {
  { 4, 3},{ 3, 3},{ 0, 3},{ 2, 2},{ 2, 2},{ 1, 2},{ 1, 2}
};

  // Table B-14, restricted to code size <= 2
  // Unused (hardcoded)
// static const SKL_VLC DC_Size_Chrom_B14_0[3] = { { 2, 2},{ 1, 2},{ 0, 2} }; 

  // Table B-16

static const SKL_DCT_VLC Intra_B16_0[52] = {
  { 0,-2, 6},{ 5, 1, 6},{ 2,-1, 6},{ 1,-1, 6}
, { 4, 1, 6},{ 3, 1, 6},{ 0, 8, 6},{ 0, 7, 6},{ 1, 2, 6},{ 0, 6, 6},{ 2, 1, 5},{ 2, 1, 5}
, { 0, 5, 5},{ 0, 5, 5},{ 0, 4, 5},{ 0, 4, 5},{ 0,-1, 4},{ 0,-1, 4},{ 0,-1, 4},{ 0,-1, 4}
, { 0, 1, 2},{ 0, 1, 2},{ 0, 1, 2},{ 0, 1, 2},{ 0, 1, 2},{ 0, 1, 2},{ 0, 1, 2},{ 0, 1, 2}
, { 0, 1, 2},{ 0, 1, 2},{ 0, 1, 2},{ 0, 1, 2},{ 0, 1, 2},{ 0, 1, 2},{ 0, 1, 2},{ 0, 1, 2}
, { 0, 2, 3},{ 0, 2, 3},{ 0, 2, 3},{ 0, 2, 3},{ 0, 2, 3},{ 0, 2, 3},{ 0, 2, 3},{ 0, 2, 3}
, { 1, 1, 4},{ 1, 1, 4},{ 1, 1, 4},{ 1, 1, 4},{ 0, 3, 4},{ 0, 3, 4},{ 0, 3, 4},{ 0, 3, 4}
};
static const SKL_DCT_VLC Intra_B16_1[29] = {
  { 8,-1, 8},{ 7,-1, 8},{ 6,-1, 8},{ 0,-3, 8},{10, 1, 8}
, { 9, 1, 8},{ 8, 1, 8},{ 9,-1, 8},{ 3, 2, 8},{ 1, 4, 8},{ 0,12, 8},{ 0,11, 8},{ 0,10, 8}
, { 4,-1, 7},{ 4,-1, 7},{ 3,-1, 7},{ 3,-1, 7},{ 6, 1, 7},{ 6, 1, 7},{ 5,-1, 7},{ 5,-1, 7}
, { 7, 1, 7},{ 7, 1, 7},{ 2, 2, 7},{ 2, 2, 7},{ 1, 3, 7},{ 1, 3, 7},{ 0, 9, 7},{ 0, 9, 7}
};
static const SKL_DCT_VLC Intra_B16_2[104] = {
    // These are the ESC codes
  {-1, 0, 8},{-1, 0, 8},{-1, 0, 8},{-1, 0, 8},{-1, 0, 8},{-1, 0, 8},{-1, 0, 8},{-1, 0, 8}
, {-2, 0, 9},{-2, 0, 9},{-2, 0, 9},{-2, 0, 9},{-3, 0, 9},{-3, 0, 9},{-3, 0, 9},{-3, 0, 9}

, { 0,18,10},{ 0,18,10},{ 0,17,10},{ 0,17,10},{14,-1, 9},{14,-1, 9},{14,-1, 9},{14,-1, 9}
, {13,-1, 9},{13,-1, 9},{13,-1, 9},{13,-1, 9},{12,-1, 9},{12,-1, 9},{12,-1, 9},{12,-1, 9}
, {11,-1, 9},{11,-1, 9},{11,-1, 9},{11,-1, 9},{10,-1, 9},{10,-1, 9},{10,-1, 9},{10,-1, 9}
, { 1,-2, 9},{ 1,-2, 9},{ 1,-2, 9},{ 1,-2, 9},{ 0,-4, 9},{ 0,-4, 9},{ 0,-4, 9},{ 0,-4, 9}
, {12, 1, 9},{12, 1, 9},{12, 1, 9},{12, 1, 9},{11, 1, 9},{11, 1, 9},{11, 1, 9},{11, 1, 9}
, { 7, 2, 9},{ 7, 2, 9},{ 7, 2, 9},{ 7, 2, 9},{ 6, 2, 9},{ 6, 2, 9},{ 6, 2, 9},{ 6, 2, 9}
, { 5, 2, 9},{ 5, 2, 9},{ 5, 2, 9},{ 5, 2, 9},{ 3, 3, 9},{ 3, 3, 9},{ 3, 3, 9},{ 3, 3, 9}
, { 2, 3, 9},{ 2, 3, 9},{ 2, 3, 9},{ 2, 3, 9},{ 1, 6, 9},{ 1, 6, 9},{ 1, 6, 9},{ 1, 6, 9}
, { 1, 5, 9},{ 1, 5, 9},{ 1, 5, 9},{ 1, 5, 9},{ 0,16, 9},{ 0,16, 9},{ 0,16, 9},{ 0,16, 9}
, { 4, 2, 9},{ 4, 2, 9},{ 4, 2, 9},{ 4, 2, 9},{ 0,15, 9},{ 0,15, 9},{ 0,15, 9},{ 0,15, 9}
, { 0,14, 9},{ 0,14, 9},{ 0,14, 9},{ 0,14, 9},{ 0,13, 9},{ 0,13, 9},{ 0,13, 9},{ 0,13, 9}
};
static const SKL_DCT_VLC Intra_B16_3[88] = {
  { 0,-7,11},{ 0,-7,11},{ 0,-6,11},{ 0,-6,11},{ 0,22,11},{ 0,22,11},{ 0,21,11},{ 0,21,11}
, { 2,-2,10},{ 2,-2,10},{ 2,-2,10},{ 2,-2,10},{ 1,-3,10},{ 1,-3,10},{ 1,-3,10},{ 1,-3,10}
, { 0,-5,10},{ 0,-5,10},{ 0,-5,10},{ 0,-5,10},{13, 1,10},{13, 1,10},{13, 1,10},{13, 1,10}
, { 5, 3,10},{ 5, 3,10},{ 5, 3,10},{ 5, 3,10},{ 8, 2,10},{ 8, 2,10},{ 8, 2,10},{ 8, 2,10}
, { 4, 3,10},{ 4, 3,10},{ 4, 3,10},{ 4, 3,10},{ 3, 4,10},{ 3, 4,10},{ 3, 4,10},{ 3, 4,10}
, { 2, 4,10},{ 2, 4,10},{ 2, 4,10},{ 2, 4,10},{ 1, 7,10},{ 1, 7,10},{ 1, 7,10},{ 1, 7,10}
, { 0,20,10},{ 0,20,10},{ 0,20,10},{ 0,20,10},{ 0,19,10},{ 0,19,10},{ 0,19,10},{ 0,19,10}
, { 0,23,11},{ 0,23,11},{ 0,24,11},{ 0,24,11},{ 1, 8,11},{ 1, 8,11},{ 9, 2,11},{ 9, 2,11}
, { 3,-2,11},{ 3,-2,11},{ 4,-2,11},{ 4,-2,11},{15,-1,11},{15,-1,11},{16,-1,11},{16,-1,11}
, { 0,25,12},{ 0,26,12},{ 0,27,12},{ 1, 9,12},{ 6, 3,12},{ 1,10,12},{ 2, 5,12},{ 7, 3,12}
, {14, 1,12},{ 0,-8,12},{ 5,-2,12},{ 6,-2,12},{17,-1,12},{18,-1,12},{19,-1,12},{20,-1,12}
};

  // Table B-17
static const SKL_DCT_VLC Inter_B17_0[52] = {
  { 4,-1, 6},{ 3,-1, 6},{ 2,-1, 6},{ 1,-1, 6}
, { 9, 1, 6},{ 8, 1, 6},{ 7, 1, 6},{ 6, 1, 6},{ 1, 2, 6},{ 0, 3, 6},{ 5, 1, 5},{ 5, 1, 5}
, { 4, 1, 5},{ 4, 1, 5},{ 3, 1, 5},{ 3, 1, 5},{ 0,-1, 4},{ 0,-1, 4},{ 0,-1, 4},{ 0,-1, 4}
, { 0, 1, 2},{ 0, 1, 2},{ 0, 1, 2},{ 0, 1, 2},{ 0, 1, 2},{ 0, 1, 2},{ 0, 1, 2},{ 0, 1, 2}
, { 0, 1, 2},{ 0, 1, 2},{ 0, 1, 2},{ 0, 1, 2},{ 0, 1, 2},{ 0, 1, 2},{ 0, 1, 2},{ 0, 1, 2}
, { 1, 1, 3},{ 1, 1, 3},{ 1, 1, 3},{ 1, 1, 3},{ 1, 1, 3},{ 1, 1, 3},{ 1, 1, 3},{ 1, 1, 3}
, { 2, 1, 4},{ 2, 1, 4},{ 2, 1, 4},{ 2, 1, 4},{ 0, 2, 4},{ 0, 2, 4},{ 0, 2, 4},{ 0, 2, 4}
};
static const SKL_DCT_VLC Inter_B17_1[29] = {
  {16,-1, 8},{15,-1, 8},{14,-1, 8},{13,-1, 8},{12,-1, 8}
, {11,-1, 8},{10,-1, 8},{ 9,-1, 8},{14, 1, 8},{13, 1, 8},{ 2, 2, 8},{ 1, 3, 8},{ 0, 5, 8}
, { 8,-1, 7},{ 8,-1, 7},{ 7,-1, 7},{ 7,-1, 7},{ 6,-1, 7},{ 6,-1, 7},{ 5,-1, 7},{ 5,-1, 7}
, {12, 1, 7},{12, 1, 7},{11, 1, 7},{11, 1, 7},{10, 1, 7},{10, 1, 7},{ 0, 4, 7},{ 0, 4, 7}
};
static const SKL_DCT_VLC Inter_B17_2[52] = {
    // These are the ESC codes
  {-1, 0, 8},{-1, 0, 8},{-1, 0, 8},{-1, 0, 8},{-2, 0, 9},{-2, 0, 9},{-3, 0, 9},{-3, 0, 9}

, { 0, 9,10},{ 0, 8,10},{24,-1, 9},{24,-1, 9},{23,-1, 9},{23,-1, 9},{22,-1, 9},{22,-1, 9}
, {21,-1, 9},{21,-1, 9},{20,-1, 9},{20,-1, 9},{19,-1, 9},{19,-1, 9},{18,-1, 9},{18,-1, 9}
, {17,-1, 9},{17,-1, 9},{ 0,-2, 9},{ 0,-2, 9},{22, 1, 9},{22, 1, 9},{21, 1, 9},{21, 1, 9}
, {20, 1, 9},{20, 1, 9},{19, 1, 9},{19, 1, 9},{18, 1, 9},{18, 1, 9},{17, 1, 9},{17, 1, 9}
, {16, 1, 9},{16, 1, 9},{15, 1, 9},{15, 1, 9},{ 4, 2, 9},{ 4, 2, 9},{ 3, 2, 9},{ 3, 2, 9}
, { 0, 7, 9},{ 0, 7, 9},{ 0, 6, 9},{ 0, 6, 9}
};
static const SKL_DCT_VLC Inter_B17_3[88] = {
  { 1,-2,11},{ 1,-2,11},{ 0,-3,11},{ 0,-3,11},{ 0,11,11},{ 0,11,11},{ 0,10,11},{ 0,10,11}
, {28,-1,10},{28,-1,10},{28,-1,10},{28,-1,10},{27,-1,10},{27,-1,10},{27,-1,10},{27,-1,10}
, {26,-1,10},{26,-1,10},{26,-1,10},{26,-1,10},{25,-1,10},{25,-1,10},{25,-1,10},{25,-1,10}
, { 9, 2,10},{ 9, 2,10},{ 9, 2,10},{ 9, 2,10},{ 8, 2,10},{ 8, 2,10},{ 8, 2,10},{ 8, 2,10}
, { 7, 2,10},{ 7, 2,10},{ 7, 2,10},{ 7, 2,10},{ 6, 2,10},{ 6, 2,10},{ 6, 2,10},{ 6, 2,10}
, { 5, 2,10},{ 5, 2,10},{ 5, 2,10},{ 5, 2,10},{ 3, 3,10},{ 3, 3,10},{ 3, 3,10},{ 3, 3,10}
, { 2, 3,10},{ 2, 3,10},{ 2, 3,10},{ 2, 3,10},{ 1, 4,10},{ 1, 4,10},{ 1, 4,10},{ 1, 4,10}
, { 0,12,11},{ 0,12,11},{ 1, 5,11},{ 1, 5,11},{23, 1,11},{23, 1,11},{24, 1,11},{24, 1,11}
, {29,-1,11},{29,-1,11},{30,-1,11},{30,-1,11},{31,-1,11},{31,-1,11},{32,-1,11},{32,-1,11}
, { 1, 6,12},{ 2, 4,12},{ 4, 3,12},{ 5, 3,12},{ 6, 3,12},{10, 2,12},{25, 1,12},{26, 1,12}
, {33,-1,12},{34,-1,12},{35,-1,12},{36,-1,12},{37,-1,12},{38,-1,12},{39,-1,12},{40,-1,12}
};

  // Table B-19
static const int Max_LEVEL_Intra_Last0[15] =      /* INTRA */
  { 27,10,5, 4, 3, 3, 3, 3, 2, 2, 1, 1, 1, 1, 1 };
static const int Max_LEVEL_Intra_Last1[21] =      /* INTRA */
  { 8, 3, 2, 2, 2, 2, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1 };

  // Table B-20
static const int Max_LEVEL_Inter_Last0[27] =      /* INTER */
  { 12,6, 4, 3, 3, 3, 3, 2, 2, 2, 2, 1, 1, 1, 1, 1,
     1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 };
static const int Max_LEVEL_Inter_Last1[41] =      /* INTER */
  { 3, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1 };

const int *(Max_LEVEL[2][2]) = {    // index: [intra][last][run/level]
  { Max_LEVEL_Intra_Last0, Max_LEVEL_Intra_Last1 },
  { Max_LEVEL_Inter_Last0, Max_LEVEL_Inter_Last1 }
};

  // Table B-21 (+1 included)
static const int Max_RUN_Intra_Last0[28-1] =
  { 15, 10, 8, 4, 3, 2, 2, 2, 2, 2, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 };
static const int Max_RUN_Intra_Last1[9-1] =
  { 21, 7, 2, 1, 1, 1, 1, 1 };

  // Table B-22
static const int Max_RUN_Inter_Last0[13-1] =
  { 27, 11, 7, 3, 2, 2, 1, 1, 1, 1, 1, 1 };
static const int Max_RUN_Inter_Last1[4-1] =
  { 41, 2, 1 };

static const int *(Max_RUN[2][2]) = {
  { Max_RUN_Intra_Last0, Max_RUN_Intra_Last1 },
  { Max_RUN_Inter_Last0, Max_RUN_Inter_Last1 }
};

  // Table B-33 (first part)
static const SKL_VLC Spr_Tab_B33[8] = {
  { 0, 2},{ 0, 2},{ 1, 3},{ 2, 3},{ 3, 3},{ 4, 3},{ 5, 3},{-1, 3}
};
  // Table B-34 for size>=6
static const SKL_VLC Spr_Tab_B34[8] = {
  { 6, 1},{ 6, 1},{ 6, 1},{ 6, 1},{ 7, 2},{ 7, 2},{ 9, 3},{10, 3}
};

//////////////////////////////////////////////////////////
// Vector decoding
//////////////////////////////////////////////////////////

static int Read_Motion_Vector(SKL_FBB *Bits, SKL_INT32 Fix)
{
  if (Bits->Get_Bits(1))
    return 0;

  Bits->Check_Bits(11);
  SKL_UINT32 Code = Bits->Bits;
  const SKL_VLC *VLC;
  if      (Code>=0x20000000)     VLC = MV_Tab_B12_0 - 1 + (Code>>(21+8));
  else if (Code>=0x02000000)     VLC = MV_Tab_B12_1 - 4 + (Code>>(21+2));
  else /*if (Code>=0x00400000)*/ VLC = MV_Tab_B12_2 - 2 + (Code>>(21+0));

  int Val;
  if (!--Fix) {   // special case for Fix==1 (residue=0)
    Val = VLC->Val;
    Code = Bits->Get_Bits(VLC->Len);  // reads 'sign bit'
    if (Code & 1) Val = -Val;
  }
  else {
    Val = ((VLC->Val-1)<<Fix) + 1;
    Code = Bits->Get_Bits(VLC->Len + Fix);  // reads 'sign bit' and 'res' too...
    const SKL_INT32 Scale = 1 << Fix;
    Val += Code & (Scale-1);        // add residue...
    if (Code & Scale) Val = -Val;   // ...and sign
  }
  return Val;
}

static void Read_Vector(SKL_FBB *Bits, SKL_INT32 Fix, SKL_MV MV)
{
  const int High  = 16 << Fix;

  int dx = MV[0] + Read_Motion_Vector(Bits, Fix);
  int dy = MV[1] + Read_Motion_Vector(Bits, Fix);

  if      (dx<-High) dx += 2*High;
  else if (dx>=High) dx -= 2*High;
  MV[0] = dx;

  if      (dy<-High) dy += 2*High;
  else if (dy>=High) dy -= 2*High;
  MV[1] = dy;
}

//////////////////////////////////////////////////////////
// Coeffs decoding
//////////////////////////////////////////////////////////

static int Read_DC_Size_Lum(SKL_FBB *Bits)
{
  int Size;
  const SKL_UINT32 Code = Bits->See_Bits_Safe(3);
  if (Code!=0) {
    const SKL_VLC *VLC = &DC_Size_Lum_B13_0[Code - 1];
    Bits->Discard_Safe( VLC->Len );
    Size = VLC->Val;
  }
  else {
    const int Val = 11 + 1 - SKL_BMASKS::Bit_Sizes[Bits->See_Bits_Safe(11)];
    Bits->Discard_Safe( Val );
    Size = Val+1;
  }
  return Size;
}

static int Read_DC_Size_Chroma(SKL_FBB *Bits)
{
  int Size;
  const SKL_UINT32 Code = Bits->See_Bits_Safe(2);
  if (Code!=0) {
    Bits->Discard_Safe( 2 );
    Size = 3-Code; // <- hardcoded version of DC_Size_Chrom_B14_0[]
  }
  else {
    const int Val = 12 + 1 - SKL_BMASKS::Log2(Bits->See_Bits_Safe(12));
    Bits->Discard_Safe( Val );
    Size = Val;
  }
  return Size;
}

static int Read_DC_Diff(SKL_FBB *Bits, int Lum)
{
  Bits->Check_Bits(12);
  const int Size = Lum ? Read_DC_Size_Lum(Bits) : Read_DC_Size_Chroma(Bits);
  if (Size==0) return 0;
  SKL_UINT32 Code;
  if (Size<=8) Code = Bits->Get_Bits(Size);
  else {
    Code = Bits->Get_Bits(Size+1);    // + marker bit
    Code >>= 1;
  }
  if (!(Code&SKL_BMASKS::Or[Size]))   // MSB not set
    Code -= SKL_BMASKS::And[Size];
  return Code;  
}

static void Read_Intra_AC(SKL_FBB *Bits, SKL_INT16 *Coeff,
                          const int * const Zigzag, int i)
{
  while(1)
  {
    const SKL_DCT_VLC *VLC;

    Bits->Check_Bits(12);
    SKL_UINT32 Code = Bits->Bits;
    SKL_ASSERT(Code>=0x00800000);

         if (Code>=0x30000000) VLC = Intra_B16_0 - 12 + (Code>>(20+6));
    else if (Code>=0x13000000) VLC = Intra_B16_1 - 19 + (Code>>(20+4));
    else if (Code>=0x06000000) VLC = Intra_B16_2 - 48 + (Code>>(20+1));
    else /*if (Code>=0x00800000)*/ VLC = Intra_B16_3 -  8 + (Code>>(20+0));
    //else break; /* error */        

    int Run = VLC->Run;
    int Level = VLC->Level;
    int Last  = (Level<0);
    if (Run>=0) {
      if (Last) Level = -Level;
      Code = Bits->Get_Bits(VLC->Len+1);
      if (Code&1) Level = -Level;
    }
    else  // Escape. Note: can only occur in Intra_B16_2[]
    {
      Bits->Discard(VLC->Len);
      if (Run==-3)    // 3rd escape
      {
        Code = Bits->Get_Bits(1+6+1+12+1);
        Last  = (Code>>20);
        Run   = (Code>>14) & 0x3f;
        Level = (Code>> 1) & 0xfff;
        if (Level&0x800) Level |= (-1)^0xfff;
      }
      else {  // 1rst and 2nd escape
        const int Esc2 = (Run==-2);
        Bits->Check_Bits(12);
        Code = Bits->Bits;
        SKL_ASSERT(Code>=0x00800000);
             if (Code>=0x30000000) VLC = Intra_B16_0 - 12 + (Code>>(20+6));
        else if (Code>=0x13000000) VLC = Intra_B16_1 - 19 + (Code>>(20+4));
        else if (Code>=0x06000000) VLC = Intra_B16_2 - 48 + (Code>>(20+1));
        else /*if (Code>=0x00800000)*/ VLC = Intra_B16_3 -  8 + (Code>>(20+0));
        // else break; /* error */
        SKL_ASSERT(VLC->Run>=0); // Hope it's not an ESCAPE code again! ;)
        Code  = Bits->Get_Bits(VLC->Len+1);
        Level = VLC->Level;
        Last  = (VLC->Level<0);
        if (Last) Level = -Level;
        Run = VLC->Run;
        if (Esc2) Run += Max_RUN[0][Last][Level-1];    // 2nd escape
        else      Level += Max_LEVEL[0][Last][Run];    // 1rst escape
        if (Code&1) Level = -Level;
      }
    }
    i += Run;
    if (i<64) { 
      const int j = Zigzag[i++];
      Coeff[j] = (SKL_INT16)Level;
      if (Last) break;
    }
    else break; /* error */
  }
}

static int Read_Inter_AC(SKL_FBB *Bits, SKL_INT16 *Coeff,
                         const int * const Zigzag)
{
  int i = 0;
  int Rows = 0x00;
  while(1)
  {
    const SKL_DCT_VLC *VLC;

    Bits->Check_Bits_Word(12);
    SKL_UINT32 Code = Bits->Bits;
    SKL_ASSERT(Code>=0x00800000);

    if      (Code>=0x30000000) VLC = Inter_B17_0 - 12 + (Code>>(20+6));
    else if (Code>=0x13000000) VLC = Inter_B17_1 - 19 + (Code>>(20+4));
    else if (Code>=0x06000000) VLC = Inter_B17_2 - 24 + (Code>>(20+2));
    else if (Code>=0x00800000) VLC = Inter_B17_3 -  8 + (Code>>(20+0));
    else break; /* error */

    int Run   = VLC->Run;
    int Level = VLC->Level;
    int Last  = (Level<0);
    if (Run>=0) {
      if (Last) Level = -Level;
      if (Bits->Get_Bits(VLC->Len+1) & 1)
        Level = -Level;
    }
    else  // Escape. Note: can only occur in Inter_B17_2[]
    {
      Bits->Discard_Safe(VLC->Len);
      if (Run==-3)    // 3rd escape
      {
        SKL_UINT32 Pack = Bits->Get_Bits(1+6+1+12+1);
        Last  = (Pack>>20);
        Run   = (Pack>>14) & 0x3f;
        Level = ((SKL_INT32)Pack<<19)>>20;
        // Level = (Pack>> 1) & 0xfff;
        // if (Level&0x800) Level |= (-1)^0xfff;
      }
      else    // 1rst and 2nd escape
      {
        const int Esc2 = (Run==-2);
        Bits->Check_Bits_Word(12);
        Code = Bits->Bits;
        SKL_ASSERT(Code>=0x00800000);
        if      (Code>=0x30000000) VLC = Inter_B17_0 - 12 + (Code>>(20+6));
        else if (Code>=0x13000000) VLC = Inter_B17_1 - 19 + (Code>>(20+4));
        else if (Code>=0x06000000) VLC = Inter_B17_2 - 24 + (Code>>(20+2));
        else if (Code>=0x00800000) VLC = Inter_B17_3 -  8 + (Code>>(20+0));
        else break; /* error */
        SKL_ASSERT(VLC->Run>=0); // Hope it's not an ESCAPE code again! ;)
        Run = VLC->Run;
        Level = VLC->Level;
        Last = (VLC->Level<0);
        if (Last) Level = -Level;
        if (Esc2) Run += Max_RUN[1][Last][Level-1];    // 2nd escape
        else      Level += Max_LEVEL[1][Last][Run];    // 1rst escape
        if (Bits->Get_Bits(VLC->Len+1) & 1)
          Level = -Level;
      }
    }
    i += Run;
    if (i<64) { 
      const int j = Zigzag[i++];
      Coeff[j] = (SKL_INT16)Level;
      Rows |= SKL_MP4_I::Row_From_Index[j];
      if (Last) break;
    }
    else break; /* error */
  }

  return Rows;
}

//////////////////////////////////////////////////////////
// Macroblock Cursor
// TODO: this could be incrementalized (with 'Off')
//////////////////////////////////////////////////////////

SKL_MB::SKL_MB(const SKL_MP4_I * const Vol)
  : VOL(Vol)
  , BpS(Vol->BpS)
  , BpS8(8*Vol->BpS)
  , Fwd_CoLoc(Vol->Past->Y - Vol->Cur->Y)
  , Bwd_CoLoc(Vol->Future->Y - Vol->Cur->Y)
  , YTmp(Vol->Cur->Y - 16 - 16*Vol->BpS)     // size: 17x16
  , Map(Vol->Cur->Map)
{

    // HACK! about YTmp: We form the qpel intermediate prediction in the 
    // top left (margin) macroblock of the current edged picture. Since 
    // the current picture is not supposed to be motion-searched for, we
    // can happily trash the edges. 
    //(Note: I'm somewhat ashamed of such a hack :)

  Quant = VOL->Quant;
  Prev_Quant = Quant;
  Field_DCT = VOL->Interlace ? 0 : -1;
  MV_Stride = VOL->MV_Stride;
  y = 0;
  MC_Sel = 0;
  Field_Dir = VOL->Field_Dir;

    // Init scan DC/AC predictors

  int i;
  MB_W = VOL->MB_W;
  if (VOL->Reduced_VOP>0) MB_W >>= 1;

  for(i=0; i<2*MB_W; ++i)
    VOL->Y_Preds[0][i].Set_Not_Intra();             // reset Top Y
  for(i=0; i<MB_W; ++i) {
    VOL->C_Preds[0][            i].Set_Not_Intra(); // reset Top U
    VOL->C_Preds[0][VOL->MB_W+2+i].Set_Not_Intra(); // reset Top V
  }

    // invariants along each scan lines

  Lefts[0] = Left_ACs[0];
  Lefts[1] = Left_ACs[0];
  Lefts[2] = Left_ACs[1];
  Lefts[3] = Left_ACs[1];
  Lefts[4] = Left_ACs[2];
  Lefts[5] = Left_ACs[3];

    // compute resync marker len

  if (VOL->Resync) {
    int Len;
    if (VOL->Coding==I_VOP) Len = 16 + 1;
    else if (VOL->Coding==B_VOP) {
      Len = 16 + ((VOL->Bwd_Code > VOL->Fwd_Code) ? VOL->Bwd_Code
                                                  : VOL->Fwd_Code);
      if (VOL->Quarter) Len += 1;
    }
    else  /* P/S-VOP */
      Len = 16 + VOL->Fwd_Code + (VOL->Quarter ? 1 : 0);
    Resync = Len;
  }
  else Resync = 0;

  Start_Scan();
}

SKL_MB::~SKL_MB() {
    // Store field parity for next frame
  ((SKL_MP4_I*)VOL)->Field_Dir = Field_Dir;
}

void SKL_MB::Set_Not_Intra()
{
  Curs[0]->Set_Not_Intra();
  Curs[1]->Set_Not_Intra();
  Curs[2]->Set_Not_Intra();
  Curs[3]->Set_Not_Intra();
  Curs[4]->Set_Not_Intra();
  Curs[5]->Set_Not_Intra();
}

void SKL_MB::Init_Offset()
{
  const SKL_MP4_PIC * const Pic = VOL->Cur;
  const int Off = (x + y*BpS) * 8;
  Y1  = Pic->Y + Off * 2;
  U   = Pic->U + Off;
  V   = Pic->V + Off;

  if (Field_DCT<=0) {
    YBpS = BpS;
    Y2 = Y1 + BpS8;
    Field_Pred = 0;
  }
  else { SKL_ASSERT( (Y2=0, 1) ); /* sanity check */ }

  const int Shift = 1 + VOL->Quarter - (VOL->Reduced_VOP>0);
  MB_Pels = 16 << (Shift);
  Limit_Mins[0] = (-x*16         -16) << Shift;
  Limit_Maxs[0] = (-x*16+VOL->Width ) << Shift;
  Limit_Mins[1] = (-y*16         -16) << Shift;
  Limit_Maxs[1] = (-y*16+VOL->Height) << Shift;
  Limit_Mins_UV[0] = (-x*8            -8) << 1;
  Limit_Maxs_UV[0] = (-x*8+VOL->Width/2 ) << 1;
  Limit_Mins_UV[1] = (-y*8            -8) << 1;
  Limit_Maxs_UV[1] = (-y*8+VOL->Height/2) << 1;
}

void SKL_MB::Resync_Cursor(const SKL_MP4_I * const VOL,
                           int New_Pos)
{
  Start_Scan();

  const int xo = New_Pos % VOL->MB_W;
  y = New_Pos / VOL->MB_W;
  Init_Scanline(VOL, xo);

    // mark left column/top row as invalid ("out-of-video-packet")
  int To_Fill = VOL->MB_W - xo;
  int Fill_Left = xo;
  if (VOL->Reduced_VOP>0) { 
    SKL_ASSERT((y&1)==0 && (xo&1)==0);
    To_Fill >>= 1;
    Fill_Left >>= 1;
  }

  int i;
  for(i=0; i<2*To_Fill; ++i)       // top rows
    Tops[0][i].Set_Not_Intra();
  for(i=0; i<To_Fill; ++i) {
    Tops[4][i].Set_Not_Intra();
    Tops[5][i].Set_Not_Intra();
  }
  for(i=-2*Fill_Left; i<0; ++i)    // left rows
    Curs[2][i].Set_Not_Intra();
  for(i=-Fill_Left; i<0; ++i) {
    Curs[4][i].Set_Not_Intra();
    Curs[5][i].Set_Not_Intra();
  }
  
  Curs[0][-1].Set_Not_Intra();

  Prev_Quant = Quant = VOL->Quant;
}

void SKL_MB::Init_Scanline(const SKL_MP4_I * const VOL, int xo)
{
    // 'y' should have been initialized prior to getting here
  x = xo;
  Pos = x + y*VOL->MB_W;

  ABits = (Top_Ok()<<1) | (Top_Right_Ok()<<2) | 1;  // left is never available.

    // reset left predictors to zero
  for(int k=0; k<4; ++k)
    for(int i=0; i<7; ++i)
      Left_ACs[k][i] = 0;

  int yo = y;
  if (VOL->Reduced_VOP>0) { yo >>= 1; xo >>= 1; }

  if (!VOL->Is_B_VOP()) {
    MVs  = VOL->Cur->MV + 2*(xo + yo*MV_Stride);
    MVs2 = MVs + MV_Stride;
  }
  else {    // sanity check
    MVs  = 0;
    MVs2 = 0;
  }

    // swap the block scan lines
  if (!(yo&1)) {
    Curs[2] = VOL->Y_Preds[2] + 2*xo;
    Tops[0] = VOL->Y_Preds[0] + 2*xo;
    Tops[4] = VOL->C_Preds[0] + xo;
    Curs[4] = VOL->C_Preds[1] + xo;
  }
  else {
    Curs[2] = VOL->Y_Preds[0] + 2*xo;
    Tops[0] = VOL->Y_Preds[2] + 2*xo;
    Tops[4] = VOL->C_Preds[1] + xo;
    Curs[4] = VOL->C_Preds[0] + xo;
  }

    // invariants along each scan lines
  const int W = VOL->MB_W;
  Curs[0] = VOL->Y_Preds[1] + 2*xo;
  Curs[1] = Curs[0] + 1;
  Tops[2] = Curs[0];
  Tops[3] = Curs[1];

  Tops[1] = Tops[0] + 1;
  Curs[3] = Curs[2] + 1;
  Tops[5] = Tops[4] + W + 2;
  Curs[5] = Curs[4] + W + 2;

  Init_Offset();

    // Sanity check: Boundary blocks are invariants
  SKL_ASSERT( VOL->Sanity_Check_Preds() );
}

void SKL_MB::operator++(int)
{
  SKL_ASSERT(VOL->Reduced_VOP<=0);

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

  Y1  += 16;
  Y2  += 16;
  U   += 8;
  V   += 8;

  Limit_Mins[0] -= MB_Pels;
  Limit_Maxs[0] -= MB_Pels;
  Limit_Mins_UV[0] -= 16;
  Limit_Maxs_UV[0] -= 16;

  x++;
  Pos++;
  MB_Count++;
  ABits = ((ABits>>1) & 0x02) | (Top_Right_Ok()<<2);
}

#if 0    /* TODO: BROKEN FOR NOW */
void SKL_MB::Next_Line_Preds()
{

  SKL_MB_DATA *Tmp;
#define SWAP(a,b) Tmp = (a); (a) = (b); (b) = Tmp
  SWAP(Tops[0], Curs[2]);
  SWAP(Tops[4], Curs[4]);
  SWAP(Tops[5], Curs[5]);
#undef SWAP
  for(int k=0; k<4; ++k)
    for(int i=0; i<7; ++i)
      Left_ACs[k][i] = 0;
  MVs += 4; // skip edges
  MVs2+= 4; // "
}
#endif

//////////////////////////////////////////////////////////
// MV prediction
//////////////////////////////////////////////////////////

  // Map from Availability-bits to Prediction Type:
  //  Pred Type is the following, used in descending probability:
  //  . Median with 3 vectors
  //  . Median with 2 vectors (!Top/!Top_Right/!Left) + zero-vector
  //  . Copy vector (Left, Top, Top_Right)
  //  . Zero vector

static const SKL_BYTE Pred_Type_ABits[8][4] = { // index: [ABits (3b)][Blk=0..3]
  {1, 1, 0, 0}
, {7, 1, 2, 0}
, {4, 4, 0, 0}
, {5, 4, 2, 0}
, {3, 3, 0, 0}
, {6, 3, 2, 0}
, {0, 0, 0, 0}
, {2, 0, 2, 0}
};

  // hardcoded Fig 7-20. There's always 2 fixed predictors exactly
  // on left and on top. Only the third is wandering on top, according to:
static const int Top_Neighbors[4] = { 2, 1, 1, -1 };

static inline SKL_INT16 Median(SKL_INT16 x, SKL_INT16 y, SKL_INT16 z) {
  SKL_INT16 m = x;
  SKL_INT16 M = x;
  if (y < m) m = y;
  else       M = y;
  if      (z <= m) return m;
  else if (z >= M) return M;
  else             return z;
}

void SKL_MB::Predict_Motion_Vector(SKL_MV MV, const SKL_MV * const Src, 
                                   const int Blk) const
{
  SKL_ASSERT(Blk>=0 && Blk<4);

  const SKL_MV * const MV_L  = Src - 1;
  const SKL_MV * const MV_T  = Src - MV_Stride;
  const SKL_MV * const MV_TR = MV_T + Top_Neighbors[Blk];
  const int Pred_Type = Pred_Type_ABits[ABits][Blk];
  switch(Pred_Type) {
    default:
    case 0:
      MV[0] = Median(MV_L[0][0], MV_T[0][0], MV_TR[0][0]);
      MV[1] = Median(MV_L[0][1], MV_T[0][1], MV_TR[0][1]);
      return;
    break;
    case 1:
      SKL_COPY_MV(MV, *MV_L);
      return;
    break;
    case 2:
      MV[0] = Median(0, MV_T[0][0], MV_TR[0][0]);
      MV[1] = Median(0, MV_T[0][1], MV_TR[0][1]);
      return;
    break;
    case 3:
      MV[0] = Median(MV_L[0][0], 0, MV_TR[0][0]);
      MV[1] = Median(MV_L[0][1], 0, MV_TR[0][1]);
      return;
    break;
    case 4:
      MV[0] = Median(MV_L[0][0], MV_T[0][0], 0);
      MV[1] = Median(MV_L[0][1], MV_T[0][1], 0);
      return;
    break;
    case 5:
      SKL_COPY_MV(MV, *MV_T);
      return;
    break;
    case 6:
      SKL_COPY_MV(MV, *MV_TR );
      return;
    break;
    case 7:
      SKL_ZERO_MV(MV);
      return;
    break;
  }
}

void SKL_MB::Predict_Motion_Vector_Blk0(SKL_MV MV, const SKL_MV *Src,
                                        const SKL_MV * const MV_L) const
{
  const SKL_MV * const MV_T = Src - MV_Stride;
  const SKL_MV * const MV_TR = MV_T + 2;
  const int Pred_Type = Pred_Type_ABits[ABits][0];
  switch(Pred_Type) {
    default:
    case 0:
      MV[0] = Median(MV_L[0][0], MV_T[0][0], MV_TR[0][0]);
      MV[1] = Median(MV_L[0][1], MV_T[0][1], MV_TR[0][1]);
      return;
    break;
    case 1:
      SKL_COPY_MV(MV, *MV_L);
      return;
    break;
    case 2:
      MV[0] = Median(0, MV_T[0][0], MV_TR[0][0]);
      MV[1] = Median(0, MV_T[0][1], MV_TR[0][1]);
      return;
    break;
    case 3:
      MV[0] = Median(MV_L[0][0], 0, MV_TR[0][0]);
      MV[1] = Median(MV_L[0][1], 0, MV_TR[0][1]);
      return;
    break;
    case 4:
      MV[0] = Median(MV_L[0][0], MV_T[0][0], 0);
      MV[1] = Median(MV_L[0][1], MV_T[0][1], 0);
      return;
    break;
    case 5:
      SKL_COPY_MV(MV, *MV_T);
      return;
    break;
    case 6:
      SKL_COPY_MV(MV, *MV_TR );
      return;
    break;
    case 7:
      SKL_ZERO_MV(MV);
      return;
    break;
  }
}

//////////////////////////////////////////////////////////
// DC / AC prediction
//////////////////////////////////////////////////////////

#define ABS(x)  (((x)<0) ? -(x) : (x))
#define DIV_ROUND(x,y)  ( (x)>=0 ? ((x)+((y)>>1))/(y) : ((x)-((y)>>1))/(y) )
#define FDIV(x,y)   ((x)+((y)>>1)) / (y)

  // section 7.4.3

int SKL_MB_DATA::Choose_Pred_Dir(const SKL_MB_DATA *Top)
{
  const SKL_INT16 A = this[-1].DC;  // wow
  const SKL_INT16 B = Top[-1].DC;
  const SKL_INT16 C = Top[ 0].DC;
  if (ABS(A-B)<ABS(C-B)) {
    DC = C;
    return 1;  // vertical
  }
  else {
    DC = A;
    return 2;  // horizontal
  }
}

#define BITS 16
#define FIX(a) ((1<<BITS)/(a) + 1)
static const int Div_AC[31] = {
          FIX( 1),FIX( 2),FIX( 3),FIX( 4),FIX( 5),FIX( 6),FIX( 7),
  FIX( 8),FIX( 9),FIX(10),FIX(11),FIX(12),FIX(13),FIX(14),FIX(15),
  FIX(16),FIX(17),FIX(18),FIX(19),FIX(20),FIX(21),FIX(22),FIX(23),
  FIX(24),FIX(25),FIX(26),FIX(27),FIX(28),FIX(29),FIX(30),FIX(31),
};
#define FDIV_SIGN(x,r,y)   ( (x)>=0 ? ((((x)+((r)>>1))*(y))>>BITS) : -(((-(x)+((r)>>1))*(y))>>BITS) )

void SKL_MB_DATA::Add_DC_AC_Pred(SKL_INT16 In[64],
                                 int Dir,
                                 SKL_INT16 Left_AC[7],
                                 const SKL_MB_DATA *Top,
                                 int AC_Q,
                                 int DC_Q)
{
  int i;  
  if (Dir==1) {              // vertical
    if (Top[0].Q>0) {        // intra block?
      if (Top[0].Q==AC_Q) {
        for(i=0; i<7; ++i) In[i+1] += Top->AC[i];
      }
      else {
        const int Div = Div_AC[AC_Q-1]; // (1<<16) / AC_Q;
        for(i=0; i<7; ++i) {
          if (Top->AC[i]!=0)
            In[i+1] += FDIV_SIGN(Top[0].Q*Top->AC[i], AC_Q, Div);
        }
      }
    }
  }
  else if (Dir==2) {         // horizontal
    if (this[-1].Q>0) {      // intra block?
      if (this[-1].Q==AC_Q) {
        for(i=0; i<7; ++i) In[8*(i+1)] += Left_AC[i];
      }
      else {
        const int Div = Div_AC[AC_Q-1]; // (1<<16) / AC_Q;
        for(i=0; i<7; ++i) {
          if (Left_AC[i]!=0)
            In[8*(i+1)] += FDIV_SIGN(this[-1].Q*Left_AC[i], AC_Q, Div);
        }
      }
    }
  }
  for(i=0; i<7; ++i) {
    AC[i] = In[i+1];
    Left_AC[i] = In[8*(i+1)];
  }
  if (DC>0)
    In[0] += FDIV(DC, DC_Q);
  DC = In[0] * DC_Q;     // TODO: redundant with Dequant? (except saturation?)
  SKL_ASSERT(In[0]>=0 && DC>=0);

  Q = AC_Q;              // this will mark the block as 'Intra'
}

#undef FDIV_SIGN
#undef BITS
#undef FIX


//////////////////////////////////////////////////////////
// I/P(/B) VOP decoding

static SKL_UINT64 Read_VOP_Time(SKL_FBB *Bits, SKL_MP4_I *VOP)
{
  SKL_UINT64 Time = VOP->Time_Ref;
  while(Bits->Get_Bits(1)) Time++;// elapsed seconds since last GOV, or I/P VOP
  Bits->Discard(1);               // marker bit

  Time *= VOP->Time_Frequency;    // switch to tick scale
  Time += Bits->Get_Bits( VOP->Ticks_Bits );

  Bits->Discard(1);               // marker bit

  return Time;
}

//////////////////////////////////////////////////////////

static int Read_Video_Packet(SKL_FBB * const Bits, SKL_MP4_I * const VOP)
{
  int Ext_Code = 0;
  if (VOP->Shape!=RECTANGULAR) {
    Ext_Code = Bits->Get_Bits(1);
    if (Ext_Code)                    // header extension code
    {
      if (!(VOP->Sprite_Mode==SPRITE_STATIC && VOP->Is_I_VOP())) {
        Bits->Discard(13+1);         // sprite width  + marker bit
        Bits->Discard(13+1);         // sprite height + marker bit
        Bits->Discard(13+1);         // sprite left coord + marker bit
        Bits->Discard(13+1);         // sprite top coord  + marker bit
      }
    }
  }
    
  const int Nb = SKL_BMASKS::Log2( VOP->MB_W*VOP->MB_H-1 );
  const int Pos = (Nb==0) ? 0 : Bits->Get_Bits(Nb);

  if (VOP->Shape!=BINARY_ONLY)
    VOP->Quant = Bits->Get_Bits(VOP->Quant_Prec);

    // now, The Question: why didn't they put the extension
    // code at the same position, whatever shape, hmm? Why? :(
  if (VOP->Shape==RECTANGULAR)
    Ext_Code = Bits->Get_Bits(1);
    
  if (Ext_Code)                     // header extension code
  {
    /* SKL_UINT64 Time = */ Read_VOP_Time(Bits, VOP);

    Bits->Discard(2);               // coding type (should match current one)

    if (VOP->Shape!=RECTANGULAR) {
      Bits->Discard(1);             // change conv. ratio disable.
      if (!VOP->Is_I_VOP())
        Bits->Discard(1);           // shape coding type
    }

    if (VOP->Shape!=BINARY_ONLY)
    {
      VOP->DC_Thresh = 
        DC_Thresh_Tab[ Bits->Get_Bits(3) ];   // intra dc vlc thresh.

      if (VOP->Sprite_Mode==SPRITE_GMC && VOP->Is_S_VOP() && VOP->Sprite_Nb_Pts>0)
        VOP->Read_Sprite_Trajectory(Bits);

      if (VOP->Shape==RECTANGULAR && VOP->Reduced_VOP>=0) {
        if (VOP->Is_I_VOP() || VOP->Is_P_VOP())
          VOP->Reduced_VOP = Bits->Get_Bits(1);
        else VOP->Reduced_VOP = 0;  // disabled for B-VOP
      }

      if (!VOP->Is_I_VOP())
        VOP->Fwd_Code = Bits->Get_Bits(3);    // fwd fixed code
      if (VOP->Is_B_VOP())
        VOP->Bwd_Code = Bits->Get_Bits(3);    // bwd fixed code
    }
  }

  if (VOP->New_Pred) {
    int Nb = VOP->Ticks_Bits + 3;
    if (Nb>15) Nb = 15;
    int Id_For_Pred = Bits->Get_Bits( Nb );
    if (Bits->Get_Bits(1)) {            // id for pred. indications
      Id_For_Pred = Bits->Get_Bits( Nb );  
    }
    Bits->Discard(1);                 // marker bit
  }

  return Pos;
}

int SKL_MB::Resync_Marker(SKL_FBB * const Bits)
{
  const int Len = Resync - 17;  // bit count expected
  SKL_ASSERT(Len>=0 && Len<=7);

  SKL_BYTE *Pos = Bits->Pos();
  int To_Align = Bits->Bits_Left & 7;
  if (To_Align==0) { To_Align = 8; Pos++; }

      // bit stuffing
  if (Bits->See_Bits(To_Align)!=SKL_BMASKS::And[To_Align-1])
    return 0;

      // marker check
  if ( Pos[0]!=0 || Pos[1]!=0 || ((Pos[2]>>(7-Len)) != 0x00000001) )
    return 0;

  Bits->Discard( To_Align );
  Bits->Discard( 16+1 + Len );

//  fprintf( stderr, " RESYNC: (%d,%d)", x, y );
  const int New_Pos = Read_Video_Packet(Bits, (SKL_MP4_I*)VOL);
  Resync_Cursor(VOL, New_Pos);
//  fprintf( stderr, " -> (%d,%d)\n", x, y );

  return 1;
}

//////////////////////////////////////////////////////////
// block DCT decoding
//////////////////////////////////////////////////////////

void SKL_MB::Decode_Intra_Blocks(SKL_FBB * const Bits,
                                 SKL_INT16 * const In,
                                 const SKL_MP4_I * const VOL) const
{
  SKL_INT16 *Out = In + 1*64;

  for(int blk=0; blk<6; ++blk)
  {
    const int DC_Q = SKL_MP4_I::DC_Scales[(blk<4)][Quant-1];
    int Pred_Dir = Curs[blk]->Choose_Pred_Dir( Tops[blk] );
    const int * const Zigzag = 
       VOL->Alt_Vert_Scan ? SKL_MP4_I::Scan_Order[2]
                          : Use_AC_Pred ? SKL_MP4_I::Scan_Order[Pred_Dir]
                                        : SKL_MP4_I::Scan_Order[0];

    VOL->Quant_Ops.Zero(In);

    int Off;
    if (Prev_Quant<VOL->DC_Thresh)
    {
      In[0] = Read_DC_Diff( Bits, (blk<4) );
      Off = 1;
    }
    else Off = 0;

    if (Cbp & (1<<(5-blk)))     // coded
      Read_Intra_AC(Bits, In, Zigzag, Off);

    Curs[blk]->Add_DC_AC_Pred(In, 
                              Use_AC_Pred ? Pred_Dir : 0,
                              Lefts[blk],
                              Tops[blk],
                              Quant,
                              DC_Q);
    VOL->Quant_Ops.Dequant_Intra(Out, In, VOL->Q_Intra, Quant, DC_Q);
    Out += 64;
  }
}

void SKL_MB::Decode_Inter_Blocks(SKL_FBB * const Bits,
                                 SKL_INT16 * const In,
                                 const SKL_MP4_I * const VOL) const
{
  SKL_INT16 *Out = In + 1*64;
  const int * const Zigzag = 
    VOL->Alt_Vert_Scan ? SKL_MP4_I::Scan_Order[2]
                       : SKL_MP4_I::Scan_Order[0];

  for(int blk=(1<<5); blk; blk>>=1)
  {
    if (Cbp & blk)      // coded
    {
      VOL->Quant_Ops.Zero(In);
      const int Rows = Read_Inter_AC(Bits, In, Zigzag);
      VOL->Quant_Ops.Dequant_Inter(Out, In, VOL->Q_Inter, Quant, Rows&0xff);
    }
    Out += 64;
  }
}

//////////////////////////////////////////////////////////
// I-frame

void SKL_MB::Decode_Intra_Infos(SKL_FBB * const Bits)
{
  const SKL_VLC *VLC;

  while(Bits->See_Bits(9)==1) Bits->Discard(9);    // MCBPC stuffing
  VLC = &MCBPC_Intra_B6_0[Bits->See_Bits(6)-1];
  Bits->Discard(VLC->Len);
  MB_Type = VLC->Val >> 2;
  Cbp = VLC->Val & 3;  // Cbpc

  Use_AC_Pred = Bits->Get_Bits(1);

  VLC = &CBPY_Tab[Bits->See_Bits(6)-2];
  Bits->Discard(VLC->Len);
  Cbp |= VLC->Val;

  if (MB_Type==SKL_MB_INTRA_Q) {
    Prev_Quant = Quant;
    Quant += DQuant_Tab[ Bits->Get_Bits(2) ];
    if      (Quant>31) Quant = 31;
    else if (Quant< 1) Quant =  1;
  }

  if (Field_DCT>=0) Set_Field_DCT( Bits->Get_Bits(1) );
  else { SKL_ASSERT(Field_DCT==-1 && Field_Pred==0); }
}

static void Read_I_VOP(SKL_FBB * const Bits,
                       const SKL_MP4_I * const VOP)
{
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
      MB.Copy_16To8(In+1*64);
      MB.Store_Map_Infos();
      MB.Store_Zero_MV();

      if (!MB.Resync || !MB.Resync_Marker(Bits))
        MB++;

      if (VOP->Debug_Level==5) {
        printf( "%c", "*-="[1+MB.Field_DCT] );
        if (MB.x==VOP->MB_W) {
          printf( "\n" );
          if (MB.y==VOP->MB_H-1) printf( "\n" );
        }
      }
    }
    if (VOP->Debug_Level==2) VOP->Dump_Line(0, &MB);

    if (VOP->Slicer) VOP->Slicer(VOP->Cur, MB.y*16, 16, VOP->Slicer_Data);

    MB.y++;
  }
}

//////////////////////////////////////////////////////////
// P-frame

void SKL_MB::Decode_Inter_Infos(SKL_FBB * const Bits,
                                const SKL_MV * const Left_Predictor)
{
  while(Bits->See_Bits(10)==1)
    Bits->Discard(10);      // MCBPC stuffing

  const SKL_VLC *VLC;       // Chroma MB type and block pattern
  if (Bits->Get_Bits(1))    // test leading bit first
    VLC = &MCBPC_Inter_B7_0[0];
  else {
    Bits->Check_Bits(8);
    const SKL_UINT32 Code = Bits->Bits;
    if (Code>=0x30000000)
      VLC = MCBPC_Inter_B7_0 - 3+1 + (Code>>(24+4));
    else /* if (Code>=0x02000000) */
      VLC = MCBPC_Inter_B7_1 - 2   + (Code>>(24+0));
    Bits->Discard(VLC->Len);
  }

  MB_Type = VLC->Val >> 2;
  Cbp = VLC->Val & 3;       // Cbpc

  if (MB_Type >= SKL_MB_INTRA)            /* INTRA or INTRA_Q */
    Use_AC_Pred = Bits->Get_Bits(1);
  else {
    Use_AC_Pred = 0;
    Cbp |= 0x0f<<2;
    if (MB_Type<=SKL_MB_INTER_Q && VOL->Is_S_VOP() && VOL->Sprite_Mode==SPRITE_GMC) 
      MC_Sel = Bits->Get_Bits(1);
    else
      MC_Sel = 0;
  }

  VLC = &CBPY_Tab[ Bits->See_Bits(6)-2 ]; // block pattern for luminance
  Bits->Discard(VLC->Len);
  Cbp ^= VLC->Val;

  if (MB_Type==SKL_MB_INTER_Q || MB_Type==SKL_MB_INTRA_Q)
  {
    Prev_Quant = Quant;
    Quant += DQuant_Tab[ Bits->Get_Bits(2) ];
    if      (Quant>31) Quant = 31;     // TODO: should be quant_prec here
    else if (Quant< 1) Quant =  1;
  }

  if (Field_DCT>=0)                       // <- equivalent to testing 'VOL->Interlace'
  {
    if (MB_Type<=SKL_MB_INTER_Q && !MC_Sel) /* INTER or INTER_Q */
    {          
      int DCT = (Cbp!=0) ? (int)Bits->Get_Bits(1) : 0;
      Set_Field_DCT( DCT );
      Field_Pred = Bits->Get_Bits(1);
      if (Field_Pred)
        Field_Dir = (Field_Dir&~0x03) | Bits->Get_Bits(2);    // top/bottom
    }
    else {
      Field_Pred = 0;
      if (MB_Type>=SKL_MB_INTRA || Cbp!=0) // for INTRA in P-VOP or INTER4V
        Set_Field_DCT( Bits->Get_Bits(1) );
      else
        Set_Field_DCT( 0 );
    }
  }
  else { SKL_ASSERT(Field_DCT==-1 && Field_Pred==0); }

  if (MB_Type<=SKL_MB_INTER4V && !MC_Sel)
  {
    Predict_Motion_Vector_Blk0( MVs[0], &MVs[0], Left_Predictor );
    if (!Field_Pred) {
      Read_Vector( Bits, VOL->Fwd_Code, MVs[0] );
      if (MB_Type==SKL_MB_INTER4V)
      {
        Predict_Motion_Vector( MVs[1], &MVs[1], 1 );
        Read_Vector( Bits, VOL->Fwd_Code, MVs[1] );
        Predict_Motion_Vector( MVs2[0], &MVs2[0], 2 );
        Read_Vector( Bits, VOL->Fwd_Code, MVs2[0] );
        Predict_Motion_Vector( MVs2[1], &MVs2[1], 3 );
        Read_Vector( Bits, VOL->Fwd_Code, MVs2[1] );
      }
    }
    else
    {
      MVs[0][1] /= 2;                 // see section 7.7.2.1
      SKL_COPY_MV(MVs[1], MVs[0]);    // Both fields use the same predictor
      Read_Vector( Bits, VOL->Fwd_Code, MVs[0] );
      Read_Vector( Bits, VOL->Fwd_Code, MVs[1] );
    }
  }
}

static void Read_P_VOP(SKL_FBB * const Bits, const SKL_MP4_I * const VOP)
{
  SKL_MB MB(VOP);
  SKL_INT16 Base[7*64+SKL_ALIGN];
  SKL_INT16 * const In = (SKL_INT16*)SKL_ALIGN_PTR(Base, SKL_ALIGN);

  while(MB.y<VOP->MB_H)
  {
    MB.Init_Scanline(VOP, 0);
    const SKL_MV *Left_Predictor = 0;

    while(MB.x<VOP->MB_W)
    {
      if (Bits->Get_Bits(1)) // not coded
      {
        MB.MC_Sel = (VOP->Is_S_VOP() && VOP->Sprite_Mode==SPRITE_GMC);
        if (!MB.MC_Sel) {
          MB.MB_Type = SKL_MB_SKIPPED;
          MB.Predict_With_0MV(VOP->Copy_Ops, 1);
          MB.Store_Zero_MV();
        }
        else {
          MB.MB_Type = SKL_MB_INTER;    // skipped GMC mb are not really 'skipped'
          MB.Predict_GMC();             // 16x16 mode
        }
        MB.Set_Not_Intra();
        Left_Predictor = &MB.MVs[1];
      }
      else
      {
        MB.Decode_Inter_Infos(Bits, Left_Predictor);

        if (MB.MB_Type >= SKL_MB_INTRA)         /* INTRA or INTRA_Q */
        {
          MB.Decode_Intra_Blocks(Bits, In, VOP);
          MB.Copy_16To8(In+1*64);
          MB.Store_Zero_MV();
          Left_Predictor = &MB.MVs[1];
        }
        else
        {
          MB.Decode_Inter_Blocks(Bits, In, VOP);
          MB.Set_Not_Intra();

          if (MB.MC_Sel) {
            MB.MB_Type = SKL_MB_INTER;
            MB.Predict_GMC();             // 16x16 mode
            Left_Predictor = &MB.MVs[1];
          }
          else {
            if (MB.MB_Type != SKL_MB_INTER4V)     /*  INTER or INTER_Q */
            {
              if (!MB.Field_Pred) {
                MB.Predict_With_1MV(MB.MVs[0], VOP->Copy_Ops, 1);
                MB.Store_16x16_MV();          // 16x16 mode
                Left_Predictor = &MB.MVs[1];
              }
              else
              {
                  // Impl: we don't multiply the final field 
                  // vectors by 2 (cf. Store_16x8_MV())
                MB.Predict_Fields(MB.MVs, VOP->Copy_Ops, 1, MB.Field_Dir);
                MB.Store_16x8_MV(&MB.MVs2[0], &MB.MVs[0]); // field MV mode
                Left_Predictor = &MB.MVs2[1]; // wow!
              } 
            }
            else                                  /* INTER4V */
            {
              MB.Predict_With_4MV(MB.MVs, MB.MVs2, VOP->Copy_Ops, 1);
              Left_Predictor = &MB.MVs[1];
            }
          }

          MB.Add_16To8(In + 1*64);
        }
      }
      MB.Store_Map_Infos();

      if (!MB.Resync || !MB.Resync_Marker(Bits))
        MB++;

      if (VOP->Debug_Level==5) {
        if (MB.MC_Sel==1) printf( MB.MB_Type==SKL_MB_SKIPPED ? "G" : "g" ); 
        else if (MB.MB_Type==SKL_MB_SKIPPED) printf( "s" );        
        else if (MB.Field_Pred==1) printf( "=" );
        else if (MB.MB_Type==SKL_MB_INTER4V) printf( "4" );
        else if (MB.MB_Type>=SKL_MB_INTRA) printf( "*" );
        else printf( "%c", ".-="[1+MB.Field_DCT] );
        if (MB.x==VOP->MB_W) {
          printf( "\n" );
          if (MB.y==VOP->MB_H-1) printf( "\n" );
        }
      }
    }

    if (VOP->Debug_Level==2) VOP->Dump_Line(0, &MB);
    if (VOP->Slicer) VOP->Slicer(VOP->Cur, MB.y*16, 16, VOP->Slicer_Data);

    MB.y++;
  }
  if (VOP->Debug_Level==1) {
    if (VOP->Is_S_VOP()) VOP->Draw_GMC(VOP->Cur);
    VOP->Dump_MVs(VOP->Cur, 2);
  }
}

//////////////////////////////////////////////////////////
// B-frame

static void Read_B_VOP(SKL_FBB * const Bits, const SKL_MP4_I * const VOP)
{
  SKL_ASSERT(VOP->Rounding==0);

    // Note: at this point, due to re-ordering, the Past frame
    // is VOP->Future and the future one be VOP->Past!
        SKL_MP4_PIC * const Cur  = VOP->Cur;
  const SKL_MP4_PIC * const Next = VOP->Future;
  const SKL_MP4_PIC * const Prev = VOP->Past;

    // Note: we don't store the Map[] values for B-VOPs,
    // but we need the future Map
  const SKL_MP4_MAP * const Map = Next->Map;
  SKL_ASSERT(Map!=0);

  const int Trb = (int)( Cur->Time_Ticks  - VOP->Time_Last_Coded );
  const int Trd = (int)( Next->Time_Ticks - VOP->Time_Last_Coded  );
  int Trbf = 0, Trdf = 0;

  SKL_MB MB(VOP);
  const int S = VOP->MV_Stride;
  while(MB.y<VOP->MB_H)
  {
      // note: MVs are not used nor stored for B-VOPs
      // 4 special predictors are used, except for MB_DIRECT
      // They are resetted at start of new row
      // Indexes: #0/1: fwd frame/field preds.  #2/3: bwd preds
    SKL_MV Preds[4] = { {0,0}, {0,0}, {0,0}, {0,0} };

    MB.Init_Scanline(VOP, 0);
    while(MB.x<VOP->MB_W)
    {
      if ( Map[MB.Pos].Type==SKL_MAP_SKIPPED ) // skipped in the future?
      {
          // we don't need no MV. Simply copy.
        MB.Cbp = 0;
        MB.Set_No_Field();
        MB.Predict_With_0MV(VOP->Copy_Ops, 1);
        if (VOP->Debug_Level==5) {
          printf( "s" );
          if (MB.x==VOP->MB_W-1) {
            printf( "\n" );
            if (MB.y==VOP->MB_H-1) printf( "\n" );
          }
        }
      }
      else
      {
        int B_Type;
        int ModB1 = Bits->Get_Bits(1);  // ModeB (hardcoded table B-3)
        if (ModB1)
        {
          MB.Cbp = 0;
          B_Type = SKL_MB_DIRECT;
          MB.Set_No_Field();
        }
        else
        {
          const int ModB2 = Bits->Get_Bits(1);
          const SKL_VLC *VLC = &BType_Tab_B4[Bits->See_Bits(4) - 1];
          B_Type = VLC->Val;
          Bits->Discard( VLC->Len );

          MB.Cbp = ModB2 ? 0 : Bits->Get_Bits(6);

          if ( B_Type!=SKL_MB_DIRECT && MB.Cbp!=0 ) {
            if (Bits->Get_Bits(1)) {
              MB.Prev_Quant = MB.Quant;
              MB.Quant += DBQuant_Tab[ Bits->Get_Bits(1) + 2 ];
              if      (MB.Quant>31) MB.Quant = 31;     // TODO: should be quant_prec here
              else if (MB.Quant< 1) MB.Quant =  1;
            }
          }          

          if (MB.Field_DCT>=0) {
            MB.Set_Field_DCT( MB.Cbp ? (int)Bits->Get_Bits(1) : 0 );
            if (B_Type!=SKL_MB_DIRECT)
            {
              MB.Field_Pred = Bits->Get_Bits(1);
              if (MB.Field_Pred) {
                if (B_Type!=SKL_MB_BWD)
                  MB.Field_Dir = (MB.Field_Dir&~0x03) | Bits->Get_Bits(2);
                if (B_Type!=SKL_MB_FWD)
                  MB.Field_Dir = (MB.Field_Dir&~0x0c) | (Bits->Get_Bits(2) << 2);
              }
            }
            else { MB.Field_Pred = 0; }
          }
          else
            SKL_ASSERT(MB.Field_DCT==-1 && MB.Field_Pred==0);
        }

            // here we go for the predictions

        if (B_Type==SKL_MB_DIRECT)         // Grrrr!
        {
          SKL_MV dMV = {0,0};

          if (!ModB1)                      // need a differential MV?
            Read_Vector( Bits, 1, dMV );   // FCode=1, always.

            // interpolate the 4 bwd/fwd vectors (section 7.6.9.5.2)

          const int Offset = 2 * (MB.x + MB.y*S);
          const SKL_MV * const Ref = Next->MV + Offset;
          SKL_MV MVs[8];  // no need to store the MVs => use local var

          if (Map[MB.Pos].Type!=SKL_MAP_16x8) 
          {
            MVs[0][0] = dMV[0] + (SKL_INT16)( Trb*Ref[0][0]/Trd );
            MVs[0][1] = dMV[1] + (SKL_INT16)( Trb*Ref[0][1]/Trd );
            if (dMV[0]==0) MVs[4][0] = (SKL_INT16)( (Trb-Trd)*Ref[0][0]/Trd );
            else           MVs[4][0] = MVs[0][0] - Ref[0][0];
            if (dMV[1]==0) MVs[4][1] = (SKL_INT16)( (Trb-Trd)*Ref[0][1]/Trd );
            else           MVs[4][1] = MVs[0][1] - Ref[0][1];

            if (Map[MB.Pos].Type==SKL_MAP_16x16) {
              if (!VOP->Quarter) {
                MB.Predict_With_1MV(MVs[0], VOP->Copy_Ops, 1);
                MB.Predict_With_1MV(MVs[4], VOP->Add_Ops,  0);
              }
              else {
                  // For QPel, because of mirroring, we must split
                  // the 16x16 block into four 8x8 ones.
                SKL_COPY_MV(MVs[1], MVs[0]);
                SKL_COPY_MV(MVs[2], MVs[0]);
                SKL_COPY_MV(MVs[3], MVs[0]);
                SKL_COPY_MV(MVs[5], MVs[4]);
                SKL_COPY_MV(MVs[6], MVs[4]);
                SKL_COPY_MV(MVs[7], MVs[4]);
                MB.Predict_With_4MV(&MVs[0], &MVs[2], VOP->Copy_Ops, 1);
                MB.Predict_With_4MV(&MVs[4], &MVs[6], VOP->Add_Ops,  0);
              }
            }
            else
            {
              MVs[1][0] = dMV[0] + (SKL_INT16)( Trb*Ref[  1][0]/Trd );
              MVs[1][1] = dMV[1] + (SKL_INT16)( Trb*Ref[  1][1]/Trd );
              MVs[2][0] = dMV[0] + (SKL_INT16)( Trb*Ref[S+0][0]/Trd );
              MVs[2][1] = dMV[1] + (SKL_INT16)( Trb*Ref[S+0][1]/Trd );
              MVs[3][0] = dMV[0] + (SKL_INT16)( Trb*Ref[S+1][0]/Trd );
              MVs[3][1] = dMV[1] + (SKL_INT16)( Trb*Ref[S+1][1]/Trd );

              if (dMV[0]==0) {
                MVs[5][0] = (SKL_INT16)( (Trb-Trd)*Ref[  1][0]/Trd );
                MVs[6][0] = (SKL_INT16)( (Trb-Trd)*Ref[S+0][0]/Trd );
                MVs[7][0] = (SKL_INT16)( (Trb-Trd)*Ref[S+1][0]/Trd );
              }
              else {
                MVs[5][0] = MVs[1][0] - Ref[  1][0];
                MVs[6][0] = MVs[2][0] - Ref[S+0][0];
                MVs[7][0] = MVs[3][0] - Ref[S+1][0];
              }
              if (dMV[1]==0) {
                MVs[5][1] = (SKL_INT16)( (Trb-Trd)*Ref[  1][1]/Trd );
                MVs[6][1] = (SKL_INT16)( (Trb-Trd)*Ref[S+0][1]/Trd );
                MVs[7][1] = (SKL_INT16)( (Trb-Trd)*Ref[S+1][1]/Trd );
              }
              else {
                MVs[5][1] = MVs[1][1] - Ref[  1][1];
                MVs[6][1] = MVs[2][1] - Ref[S+0][1];
                MVs[7][1] = MVs[3][1] - Ref[S+1][1];
              }
              MB.Predict_With_4MV(&MVs[0], &MVs[2], VOP->Copy_Ops, 1);
              MB.Predict_With_4MV(&MVs[4], &MVs[6], VOP->Add_Ops,  0);
            }
          }
          else   // colocated MB is field-predicted. This is the worst case :(
          {
            if (Trdf==0) {
              const SKL_UINT64 T = VOP->Time_TFrame;
              Trbf = 2*(int)( ( Cur->Time_Ticks+(T>>1))/T - (VOP->Time_Last_Coded+(T>>1))/T );
              Trdf = 2*(int)( (Next->Time_Ticks+(T>>1))/T - (VOP->Time_Last_Coded+(T>>1))/T );
            }

              // hardcoded Table 7-12
            const int Flds = Map[MB.Pos].Fields;    // future field directions
            int delta0 = ( (Flds>>1)&1 );      // future Top field dir
            int delta1 = ( (Flds>>0)&1 ) - 1;  // future Bottom field dir
            if (VOP->Top_Field_First) { delta0 = -delta0; delta1 = -delta1; }

            MVs[0][0] = dMV[0] + (SKL_INT16)( (Trbf+delta0)*Ref[0][0]/(Trdf+delta0) );
            MVs[0][1] = dMV[1] + (SKL_INT16)( (Trbf+delta0)*Ref[0][1]/(Trdf+delta0) );
            MVs[1][0] = dMV[0] + (SKL_INT16)( (Trbf+delta1)*Ref[1][0]/(Trdf+delta1) );
            MVs[1][1] = dMV[1] + (SKL_INT16)( (Trbf+delta1)*Ref[1][1]/(Trdf+delta1) );
            if (dMV[0]==0) {
              MVs[4][0] = (SKL_INT16)( (Trbf-Trdf)*Ref[0][0]/(Trdf+delta0) );
              MVs[5][0] = (SKL_INT16)( (Trbf-Trdf)*Ref[1][0]/(Trdf+delta1) );
            }
            else {
              MVs[4][0] = MVs[0][0] - Ref[0][0];
              MVs[5][0] = MVs[1][0] - Ref[1][0];
            }
            if (dMV[1]==0) {
              MVs[4][1] = (SKL_INT16)( (Trbf-Trdf)*Ref[0][1]/(Trdf+delta0) );
              MVs[5][1] = (SKL_INT16)( (Trbf-Trdf)*Ref[1][1]/(Trdf+delta1) );
            }
            else {
              MVs[4][1] = MVs[0][1] - Ref[0][1];
              MVs[5][1] = MVs[1][1] - Ref[1][1];
            }
/*
            printf( "x,y=%d,%d dmv=%d,%d sel=%d,%d/%d,%d Fwd: Flds=%d ref=(%d,%d) mv=(%d,%d)/(%d,%d)  Bwd: Flds=%d ref=(%d,%d) mv=(%d,%d)/(%d,%d)\n",
              MB.x, MB.y, dMV[0], dMV[1],
              (MB.Field_Dir>>1)&1,(MB.Field_Dir>>0)&1,(MB.Field_Dir>>3)&1,(MB.Field_Dir>>2)&1,
              (Flds>>1)&1, Ref[0][0], Ref[0][1], MVs[0][0],MVs[0][1], MVs[4][0],MVs[4][1],
              (Flds>>0)&1, Ref[1][0], Ref[1][1], MVs[1][0],MVs[1][1], MVs[5][0],MVs[5][1]);
*/
            MB.Predict_Fields(&MVs[0], VOP->Copy_Ops, 1, MB.Field_Dir& 3);
            MB.Predict_Fields(&MVs[4], VOP->Add_Ops,  0, MB.Field_Dir>>2);
          }
                // end of DIRECT mode
        }
        else
        {
                // INTERPOLATE/FWD/BWD. 
                // We don't use MB.MVs[], but directly Preds[] instead

          const SKL_MB_FUNCS * Ops = VOP->Copy_Ops;

          if (MB.Field_Pred)
          {
            if (B_Type!=SKL_MB_BWD)
            {
              Preds[0][1] /= 2; Preds[1][1] /= 2;       // descale predictors. See section 7.7.2.1
              Read_Vector( Bits, VOP->Fwd_Code, Preds[0] );
              Read_Vector( Bits, VOP->Fwd_Code, Preds[1] );
              MB.Predict_Fields(&Preds[0], Ops, 1, MB.Field_Dir);
              Preds[0][1] *= 2; Preds[1][1] *= 2;       // restore predictors to frame scale
              Ops = VOP->Add_Ops;
            }
            if (B_Type!=SKL_MB_FWD)
            {
              Preds[2][1] /= 2; Preds[3][1] /= 2;       // descale predictors.
              Read_Vector( Bits, VOP->Bwd_Code, Preds[2] );
              Read_Vector( Bits, VOP->Bwd_Code, Preds[3] );
              MB.Predict_Fields(&Preds[2], Ops, 0, MB.Field_Dir>>2);
              Preds[2][1] *= 2; Preds[3][1] *= 2;       // restore predictors.
            }
          }
          else {
            if (B_Type!=SKL_MB_BWD) {
              Read_Vector(Bits, VOP->Fwd_Code, Preds[0]);
              SKL_COPY_MV(Preds[1], Preds[0]);          // update 2nd predictor
              MB.Predict_With_1MV(Preds[0], Ops, 1);
              Ops = VOP->Add_Ops;
            }
            if (B_Type!=SKL_MB_FWD)
            {
              Read_Vector(Bits, VOP->Bwd_Code, Preds[2]);
              SKL_COPY_MV(Preds[3], Preds[2]);          // update 2nd predictor
              MB.Predict_With_1MV(Preds[2], Ops, 0);              
            }
          }
        }

        MB.MB_Type = SKL_MB_INTER;
        if (MB.Cbp)
        {
          SKL_INT16 Base[7*64+SKL_ALIGN];
          SKL_INT16 * const In = (SKL_INT16*)SKL_ALIGN_PTR(Base, SKL_ALIGN);
          MB.Decode_Inter_Blocks(Bits, In, VOP);
          MB.Add_16To8(In + 1*64);
        }
        // no need to MB.Set_Not_Intra() (B-VOP content cannot be used for pred)

        if (VOP->Debug_Level==5) {
          if (Map[MB.Pos].Type==SKL_MAP_SKIPPED) printf("s");
          else if (MB.Field_Pred) printf( "%c", "DBFI."[B_Type] );
          else if (B_Type==SKL_MB_DIRECT) {
            if (ModB1!=0) printf( "0" );
            else if (Map[MB.Pos].Type==SKL_MAP_16x16) printf( "d" );
            else printf("4");
          }
          else printf( "%c", "?+-=."[B_Type] );
          if (MB.x==VOP->MB_W-1) {
            printf( "\n" );
            if (MB.y==VOP->MB_H-1) printf( "\n" );
          }
        }
      }

      if (!MB.Resync || !MB.Resync_Marker(Bits))
        MB++;
      else {    // reset predictors for RESYNC
        SKL_ZERO_MV(Preds[0]);
        SKL_ZERO_MV(Preds[1]);
        SKL_ZERO_MV(Preds[2]);
        SKL_ZERO_MV(Preds[3]);
      }
    }
    if (VOP->Debug_Level==2) VOP->Dump_Line(1, &MB);
    if (VOP->Slicer) VOP->Slicer(VOP->Cur, MB.y*16, 16, VOP->Slicer_Data);
    MB.y++;
  }
  if (VOP->Debug_Level==1) // only draw past MVs (Cur are trashed)
    if (Prev->MV!=0) VOP->Dump_MVs(Prev, 2); 
}

//////////////////////////////////////////////////////////
// S-frame (GMC)

  // sprite trajectory

static int Get_Sprite_Point_Size(SKL_FBB *Bits)
{
  const SKL_VLC *VLC = &Spr_Tab_B33[ Bits->See_Bits(3) ];
  if (VLC->Val>=0) {
    Bits->Discard(VLC->Len);
    return VLC->Val;
  }
  Bits->Discard(3);
  int Len = 6;
  while(Bits->Get_Bits(1)) Len++;
  return Len;
}

void SKL_MP4_I::Read_Sprite_Trajectory(SKL_FBB * const Bits)
{
  int n;
  for(n=0; n<Sprite_Nb_Pts; ++n)
  {
    int Val, Size;
    Size = Get_Sprite_Point_Size(Bits);
    if (Size) {
      Val = Bits->Get_Bits(Size);
      if (!(Val&SKL_BMASKS::Or[Size]))   // MSB not set
        Val -= SKL_BMASKS::And[Size];
      S_Warp_Pts[n][0] = Val;
    }
    else S_Warp_Pts[n][0] = 0;
    Bits->Discard(1);                    // marker bit

    Size = Get_Sprite_Point_Size(Bits);
    if (Size) {
      Val = Bits->Get_Bits(Size);
      if (!(Val&SKL_BMASKS::Or[Size]))   // MSB not set
        Val -= SKL_BMASKS::And[Size];
      S_Warp_Pts[n][1] = Val;
    }
    else S_Warp_Pts[n][1] = 0;    
    Bits->Discard(1);                    // marker bit
  }
  for( ; n<4; ++n) S_Warp_Pts[n][0] = S_Warp_Pts[n][1] = 0;

  GMC_Ops.Setup(Width,Height, S_Warp_Pts, Sprite_Nb_Pts, Sprite_Accuracy);
}

  // misc useless stuff

static int Read_Sprite_Brightness_Change(SKL_FBB * const Bits)
{
    /* TODO: rather untested stuff... */
  int Size;
  int Code = Bits->See_Bits(4);
  if (!Bits->Get_Bits(1)) Size = 5;
  else {
    const SKL_VLC *VLC = &Spr_Tab_B34[Bits->See_Bits(3)];
    Bits->Discard(VLC->Len);
    Size = VLC->Val;
  }
  Code = Bits->Get_Bits(Size);
  if (Size<=7) {
    if (!(Code&SKL_BMASKS::Or[Size]))    // MSB not set
      Code -= SKL_BMASKS::And[Size];
    if (Code<0) Code += 15;
    else Code -= 15;
  }
  else if (Size==9) Code += 113;
  else if (Size==10) Code += 625;
  return Code;    /* -> float factor = Code/100.f + 1.f */
}

int SKL_MP4_I::Read_Sprite_Params(SKL_FBB * const Bits)
{
  if (Sprite_Brightness!=0)
    Sprite_Brightness = Read_Sprite_Brightness_Change(Bits);
  if (Sprite_Mode==SPRITE_STATIC) {
    if (Sprite_Transmit!=SPRITE_STOP && Sprite_Low_Latency)
    {
      fprintf( stderr, "Sprite transmission not supported\n" );
      return 0;
    }
  }
  return 1;
}

//////////////////////////////////////////////////////////
// higher-level syntactic element decoding
//////////////////////////////////////////////////////////

static void Read_Matrix(SKL_FBB * const Bits, SKL_BYTE * const M)
{
  int Value = 16;
  int i = 0;
  while(i<64) {
    int Tmp = Bits->Get_Bits(8);
    if (Tmp==0) break;
    Value = Tmp;
    M[SKL_MP4_I::Scan_Order[0][i++]] = Value;
  }
  while(i<64) M[SKL_MP4_I::Scan_Order[0][i++]] = Value;
}

static int Read_VOL_Header(SKL_FBB * const Bits, SKL_MP4_I * const VOL)
{
  SKL_UINT32 Code;
  int Verid = 1;

    // section 6.2.3

  Bits->Discard(1);                // random accessible
  Code = Bits->Get_Bits(8);        // VO type indication

  if (Code!=0 && Code!=1 && Code !=3 && Code!=4 && Code<=9) // support for Simple, Core and Main obj only.
    fprintf( stderr, "Warning: VO type '%d' might be unsupported.\n", Code );

  if (Bits->Get_Bits(1)) {         // is object layer identified
    Verid = Bits->Get_Bits(4);     // layer version id
    Bits->Discard(3);              // layer priority
  }

  if (Bits->Get_Bits(4)==0xf)      // aspect ratio info == extended_PAR?
    Bits->Discard(8+8);            // -> par_width + par_height

  if (Bits->Get_Bits(1)) {         // control params
    Code = Bits->Get_Bits(2+1+1);  // chroma format (01=420) + low delay + vbv param
    VOL->Low_Delay = !!(Code & 2);
    if (Code&1) {                  // vbv param
      Bits->Discard(15+1);         // first_half bit rate + marker bit
      Bits->Discard(15+1);         // latter_half bit rate + marker bit
      Bits->Discard(15+1);         // first_half vbv size + marker bit
      Bits->Discard(3);            // latter_half vbv size rate
      Bits->Discard(11+1);         // first_half vbv occupancy + marker bit
      Bits->Discard(15+1);         // latter_half vbv occupancy + marker bit
    }      
  }
  else VOL->Low_Delay = 1; 

  VOL->Shape = Bits->Get_Bits(2);  // layer shape
  if (VOL->Shape != RECTANGULAR) {
    fprintf( stderr, "WARNING: Shape (%d) is not RECTANGULAR\n", VOL->Shape );
    return 0;
  }
  if (VOL->Shape==GRAYSCALE && Verid!=1)
    Bits->Get_Bits(4);             // shape extension in version 2
  
  Bits->Discard(1);                // marker bit
  int Freq = Bits->Get_Bits(16);   // time increment resolution
  Bits->Discard(1);                // marker bit

  VOL->Set_Time_Frequency(Freq);

  if (Bits->Get_Bits(1))           // fixed vop time incr
    VOL->Ticks_Per_VOP = Bits->Get_Bits(VOL->Ticks_Bits);
  else VOL->Ticks_Per_VOP = 0;     // disabled
    /* note: Ticks_Per_VOP is not used for decoding. */

  if (VOL->Shape!=BINARY_ONLY)
  {
    if (VOL->Shape==RECTANGULAR)
    {
      Bits->Discard(1);            // marker bit
      int Width  = Bits->Get_Bits(13);
      Bits->Discard(1);            // marker bit
      int Height = Bits->Get_Bits(13);
      Bits->Discard(1);            // marker bit

        // we cannot trust (DivX) the value of Low_Delay.
        // So we always allocated 1 auxiliary frame, and Map[]s.
        // We need to allocated the maps for cur/past frames
        // just for in case some B-VOP shows up.
        // Note: If it was not for these damned DIRECT macroblocks
        // in B-VOP, we wouldn't need to store the MVs for each
        // frames, too :(
      VOL->Init_Pics( Width, Height, 3, 1 );
    }

    VOL->Interlace = Bits->Get_Bits(1);

    if (Bits->Get_Bits(1)!=1) {    // overlapped motion comp. disable
      fprintf( stderr, "WARNING: overlapped motion comp not supported (who does?)\n" );
      return 0;
    }

    VOL->Sprite_Mode = Bits->Get_Bits(Verid==1 ? 1 : 2);

    if (VOL->Sprite_Mode!=SPRITE_NONE) // sprite enable
    {
      if (VOL->Sprite_Mode==SPRITE_STATIC) {
        printf( "WARNING! Static sprite not supported!\n" );
        Bits->Discard(13+1);         // sprite width  + marker bit
        Bits->Discard(13+1);         // sprite height + marker bit
        Bits->Discard(13+1);         // sprite left coord + marker bit
        Bits->Discard(13+1);         // sprite top coord  + marker bit
      }
      VOL->Sprite_Nb_Pts     = Bits->Get_Bits(6);
      VOL->Sprite_Accuracy   = Bits->Get_Bits(2);
      VOL->Sprite_Brightness = Bits->Get_Bits(1);
      if (VOL->Sprite_Mode==SPRITE_STATIC)
        VOL->Sprite_Low_Latency = Bits->Get_Bits(1);
    }

    if (VOL->Shape!=RECTANGULAR && Verid!=1)
      Bits->Discard(1);                     // sadct disable

    VOL->Not_8b = Bits->Get_Bits(1);
    if (VOL->Not_8b) {                      // not 8 bit
      VOL->Quant_Prec = Bits->Get_Bits(4);  // quant prec
      VOL->Bpp = Bits->Get_Bits(4);         // bits per pixel
      SKL_ASSERT(VOL->Quant_Prec>=3 && VOL->Quant_Prec<=9);
      SKL_ASSERT(VOL->Bpp>=4 && VOL->Bpp<=12);
      fprintf( stderr, "WARNING: Not_8b unsupported!\n" );
    }

    if (VOL->Shape==GRAYSCALE)
      Bits->Discard(1+1+1);       // no gray quant update, composition, linear comp.

    VOL->Set_Quant_Type( Bits->Get_Bits(1) );  // quant type
    if (VOL->Get_Quant_Type()==1)              // MPEG4 custom matrix
    {
      SKL_BYTE M[64];
      if (Bits->Get_Bits(1)) {                 // custom intra
        Read_Matrix(Bits, M);
        VOL->Set_Matrix(1, M);
      }
      if (Bits->Get_Bits(1)) {                 // custom inter
        Read_Matrix(Bits, M);
        VOL->Set_Matrix(0, M);
      }

      if (VOL->Shape==GRAYSCALE)
      {
        if (Bits->Get_Bits(1)) Read_Matrix(Bits, M);  // greyscale intra
        if (Bits->Get_Bits(1)) Read_Matrix(Bits, M);  // greyscale inter
      }
    }

    if (Verid!=1)
      VOL->Quarter = Bits->Get_Bits(1); // quarter pel
    else
      VOL->Quarter = 0;

    VOL->CE_Flags = 0;
    if (!Bits->Get_Bits(1))         // define vop complexity estimation
    {
      int Version = Bits->Get_Bits(2);
      if (Version!=0 && Version!=1) {
        fprintf( stderr, "WARNING: complexity estimation version %d not supported\n", Version);
        return 0;
      }

      if (!Bits->Get_Bits(1))       // shape complexity estimation disable
        VOL->CE_Flags |= Bits->Get_Bits(6) << 16; // Opaque, transp., intraCAE, 
                                                  // interCAE, NoUpdate, UpSampling

      if (!Bits->Get_Bits(1))       // texture complexity estimation disable set #1
        VOL->CE_Flags |= Bits->Get_Bits(4) << 12;   // IntraBlks, InterBlks, 
                                                    // Inter4vBlks, NotCodedBlks

      Bits->Discard(1);             // marker bit

      if (!Bits->Get_Bits(1))       // texture complexity estimation disable set #2
        VOL->CE_Flags |= Bits->Get_Bits(4) << 8;   // DCT coeffs, DCT lines, 
                                                   // VLC Symb, VLC bits

      if (!Bits->Get_Bits(1))       // motion compensation complexity disable
        VOL->CE_Flags |= Bits->Get_Bits(6) << 2;   // APM, NPM, IMCQ, FwdBwdMCQ, 
                                                   // HalfPel2, HalfPel4

      Bits->Discard(1);             // marker bit

      if (Version == 1)
        if (!Bits->Get_Bits(1))     // motion compensation complexity disable
          VOL->CE_Flags |= Bits->Get_Bits(2) << 0; // SADCT, Quarterpel
    }

    VOL->Resync = !Bits->Get_Bits(1);   // resync marker disable
    VOL->Data_Partitioned =
                  Bits->Get_Bits(1);    // data partitioned
    if (VOL->Data_Partitioned)
      VOL->Rev_VLC = Bits->Get_Bits(1); // reversible vlc

    if (Verid!=1) {
      VOL->New_Pred = Bits->Get_Bits(1);
      if (VOL->New_Pred)  {                 // new pred enabled
        int Msg = Bits->Get_Bits(2);        // requested upstream msg + seg. types
        int Seg_Type = Bits->Get_Bits(1);
        (void)Msg; (void)Seg_Type;
      }
      if (VOL->Shape==RECTANGULAR && Bits->Get_Bits(1))
        VOL->Reduced_VOP = 0;     // reduced resolution enabled
      else VOL->Reduced_VOP = -1; // disabled
    }
    else {
      VOL->New_Pred    = 0; // disabled
      VOL->Reduced_VOP =-1; // disabled
    }

    if (Bits->Get_Bits(1))              // scalability
    {
      int Hierarchy = Bits->Get_Bits(1);
      Bits->Discard(4+1);               // ref layer, sampling dir.
      Bits->Discard(5+5+5+5);           // sampling factors
      VOL->Scalability = Bits->Get_Bits(1); // enhancement type
      if (VOL->Shape==BINARY && Hierarchy==0)
        Bits->Discard(1+1+5+5+5+5);         // use_ref + shape factors
      fprintf( stderr, "WARNING! Scalability not supported.\n" );
    }
    else VOL->Scalability = -1;         // disabled
  }
  else {                                // Shape == binary only
    if (Verid!=1) {
      if (Bits->Get_Bits(1))            // scalability
        Bits->Discard(4+5+5+5+5);       // ref_id + shape sampling factors
    }
    VOL->Resync = !Bits->Get_Bits(1);   // resync marker disable
    VOL->New_Pred = 0;                  // disabled
  }

  if (VOL->Debug_Level==3) VOL->Print_Infos();

  return 1;
}

static void ReadDCECS(SKL_FBB * const Bits, SKL_MP4_I * const VOP)
{
  // Warning! this code has been extensively, massively, ruthlessly untested!

    // section 6.2.5.1
  if (VOP->CE_Flags && (1<<21)) Bits->Discard(8);  // Opaque
  if (VOP->CE_Flags && (1<<20)) Bits->Discard(8);  // Transparent
  if (VOP->CE_Flags && (1<<19)) Bits->Discard(8);  // IntraCAE
  if (VOP->CE_Flags && (1<<18)) Bits->Discard(8);  // InterCAE
  if (VOP->CE_Flags && (1<<17)) Bits->Discard(8);  // No Update
  if (VOP->CE_Flags && (1<<16)) Bits->Discard(8);  // UpSampling

  if (VOP->CE_Flags && (1<<15)) Bits->Discard(8);  // Intra blocks
  if (VOP->CE_Flags && (1<<14)) Bits->Discard(8);  // Inter blocks
  if (VOP->CE_Flags && (1<<13)) Bits->Discard(8);  // Inter4v blocks
  if (VOP->CE_Flags && (1<<12)) Bits->Discard(8);  // Not Coded blocks

  if (VOP->CE_Flags && (1<<11)) Bits->Discard(8);  // DCT coeffs
  if (VOP->CE_Flags && (1<<10)) Bits->Discard(8);  // DCT lines
  if (VOP->CE_Flags && (1<< 9)) Bits->Discard(8);  // VLC symbols
  if (VOP->CE_Flags && (1<< 8)) Bits->Discard(4);  // VLC bits

  if (VOP->CE_Flags && (1<< 7)) Bits->Discard(8);  // APM
  if (VOP->CE_Flags && (1<< 6)) Bits->Discard(8);  // NPM
  if (VOP->CE_Flags && (1<< 5)) Bits->Discard(8);  // InterpMCQ
  if (VOP->CE_Flags && (1<< 4)) Bits->Discard(8);  // Fwd/Bwd MCQ
  if (VOP->CE_Flags && (1<< 3)) Bits->Discard(8);  // Halfpel2
  if (VOP->CE_Flags && (1<< 2)) Bits->Discard(8);  // Halfpel4

  if (VOP->CE_Flags && (1<< 1)) Bits->Discard(8);  // SADCT
  if (VOP->CE_Flags && (1<< 0)) Bits->Discard(8);  // QuarterPel
}

static int Read_VOP(SKL_FBB * const Bits, SKL_MP4_I * const VOP)
{
    // section 6.3.5

  VOP->Coding = Bits->Get_Bits(2); // Coding type

  if (VOP->Is_B_VOP()) {
    VOP->Cur = VOP->Aux;
    VOP->Cur->Map = 0;    // sanity check
    VOP->Cur->MV = 0;
  }
  else {
    VOP->Cur    = VOP->Past;
    VOP->Past   = VOP->Future;
    VOP->Future = VOP->Cur;
    VOP->Cur->Map = VOP->All_Maps[0].Map;
    VOP->Cur->MV  = VOP->All_Maps[0].MV;
      // This is a sync point in *display* order. VOP->Past is not a B-VOP.
    VOP->Time_Ref = VOP->Past->Time_Ticks / VOP->Time_Frequency;
  }

  SKL_UINT64 Time = Read_VOP_Time(Bits, VOP);

  VOP->Cur->Time = (double)(SKL_INT64)Time / (double)(SKL_INT64)VOP->Time_Frequency;
  VOP->Cur->Time_Ticks = Time;

       // First B-VOP? => store Tframe of section 7.7.2.2
  if (VOP->Coding == B_VOP && VOP->Time_TFrame==0) {
    VOP->Time_TFrame = VOP->Cur->Time_Ticks - VOP->Time_Last_Coded;
    if (VOP->Time_TFrame==0) VOP->Time_TFrame = 1; // just in case...
  }

  VOP->Cur->Coding = VOP->Coding;

//  printf( "Prev:%lld Cur:%lld Next:%lld Last_Coded:%lld TFrame:%lld Time_Ref:%lld\n",
//    VOP->Past->Time_Ticks, VOP->Cur->Time_Ticks, VOP->Future->Time_Ticks,
//    VOP->Time_Last_Coded, VOP->Time_TFrame, VOP->Time_Ref);

  const int Coded = Bits->Get_Bits(1);  // vop not coded
  if (!Coded)
    goto End;

  if (VOP->New_Pred) {
    int Nb = VOP->Ticks_Bits + 3;
    if (Nb>15) Nb = 15;
    int Id_For_Pred = Bits->Get_Bits( Nb );
    if (Bits->Get_Bits(1)) {      // id for pred. indications
      Id_For_Pred = Bits->Get_Bits( Nb );
    }
    Bits->Discard(1);             // marker bit
  }

  if (VOP->Shape!=BINARY_ONLY && 
     ( VOP->Is_P_VOP() || (VOP->Is_S_VOP() && VOP->Sprite_Mode==SPRITE_GMC) ))
    VOP->Rounding = Bits->Get_Bits(1);   // rounding type
  else VOP->Rounding = 0;                // forced (for B-VOP, in particular)

  if ( VOP->Reduced_VOP>=0 )             // only for Shape==RECTANGULAR
  {
    if ( VOP->Is_P_VOP() || VOP->Is_I_VOP() )
      VOP->Reduced_VOP = Bits->Get_Bits(1);
    else VOP->Reduced_VOP = 0;   // disabled for B-VOP
  }

  if (VOP->Shape!=RECTANGULAR) {
    if (!(VOP->Sprite_Mode==SPRITE_STATIC && VOP->Is_I_VOP()) )
    {
      Bits->Discard(13+1);        // sprite width  + marker bit
      Bits->Discard(13+1);        // sprite height + marker bit
      Bits->Discard(13+1);        // horz. mc spatial ref + marker bit
      Bits->Discard(13);          // vert. mc spatial ref
    }
    if (VOP->Scalability>0)       // Enhancement_Type!=0
      Bits->Discard(1);           // bckground composition

    Bits->Discard(1);             // change conv. ratio disable
    if (Bits->Get_Bits(1))        // vop constant alpha
      Bits->Discard(8);           // constant alpha value
  }

  if (VOP->CE_Flags) ReadDCECS(Bits, VOP);

  if (VOP->Shape!=BINARY_ONLY)
  {
    VOP->DC_Thresh = DC_Thresh_Tab[ Bits->Get_Bits(3) ]; // intra dc vlc thresh.

    if (VOP->Interlace) {                       // interlacing mode
      VOP->Top_Field_First = Bits->Get_Bits(1); // top field first
      VOP->Alt_Vert_Scan   = Bits->Get_Bits(1); // alternate vertical scan
    }
    else {
      VOP->Top_Field_First = 0;
      VOP->Alt_Vert_Scan   = 0;
    }

    if ( (VOP->Sprite_Mode>=SPRITE_STATIC) && VOP->Is_S_VOP())
    {
      if (VOP->Sprite_Nb_Pts>0)
        VOP->Read_Sprite_Trajectory(Bits);
      if (!VOP->Read_Sprite_Params(Bits))
        return 0;
    }
  }


  if (VOP->Shape!=BINARY_ONLY)
  {
    VOP->Quant = Bits->Get_Bits(VOP->Quant_Prec); // global frame quantizer
    if (VOP->Quant<1) VOP->Quant = 1;

    if (VOP->Shape==GRAYSCALE)
      Bits->Get_Bits(6);                 // vop alpha quant

    if (!VOP->Is_I_VOP())
      VOP->Fwd_Code = Bits->Get_Bits(3); // fwd fixed code
    if (VOP->Is_B_VOP())
      VOP->Bwd_Code = Bits->Get_Bits(3); // bwd fixed code

    
/*  if (!VOP->Scalability) */
    {
      if (VOP->Shape!=RECTANGULAR && !VOP->Is_I_VOP())
        Bits->Discard(1);                // vop shape coding type
    }
/*
    else {
      // if (VOP->Enhancement_Type) { load bwd shape, fwd shape... }
      Bits->Discard(2);                  // ref select code
    }
*/
  }
  else { /* BINARY_ONLY */ }

End:

  if (VOP->Debug_Level==4) VOP->Print_Infos();

  if (Coded)
  {
    VOP->Set_Rounding();
    if (VOP->Data_Partitioned) {
      if (VOP->Is_B_VOP()) VOP->Rev_VLC = 0;
      fprintf(stderr, "Data partitioning not yet supported\n" );
      return 0;
    }

    if (VOP->Coding==B_VOP)   Read_B_VOP(Bits, VOP);
    else if (VOP->Reduced_VOP<=0) {
      if (VOP->Coding==I_VOP) Read_I_VOP(Bits, VOP);
      else /* P/S-VOP */      Read_P_VOP(Bits, VOP);
    }
    else VOP->Read_Reduced_VOP(Bits);
    VOP->Switch_Off();    // reset (emms)

    if (!VOP->Is_B_VOP()) {
      VOP->Make_Edges_Dec(VOP->Cur); // replicate edges now. Only sub-optimal for a P->I sequence.
      VOP->Switch_Off();
      VOP->Time_Last_Coded = VOP->Past->Time_Ticks;
    }
  }
  else { /* not coded. Copy the full past frame. Don't update Time_Last_Coded! */ 
    VOP->Copy_Pic( VOP->Cur, VOP->Past );
  }

    // Post previous frame to user...
  if (VOP->Frame_Number==0) VOP->Last = 0;
  else {
    if (VOP->Is_B_VOP()) VOP->Last = VOP->Cur;
    else                 VOP->Last = VOP->Past;
    SKL_ASSERT(VOP->Last!=0);
    VOP->Last->Map = 0;   // sanity check
    VOP->Last->MV  = 0;   // ""
  }

  VOP->Frame_Number++;

  return 1;
}

static int Read_Visual_Object(SKL_FBB * const Bits)
{
  int Version_Id = 1;
  if (Bits->Get_Bits(1))            // identified
  {
    Version_Id = Bits->Get_Bits(4); // verid
    Bits->Discard(3);               // priority
  }

  if (Bits->Get_Bits(4)!=1) {       // Video type
//    fprintf( stderr, "WARNING: Video type not supported\n" );
    return 0;
  }

  if (Bits->Get_Bits(1))            // video signal
  {
    Bits->Discard(3+1);             // video format + video range
    if (Bits->Get_Bits(1))          // color desc.
      Bits->Discard(8+8+8);         // primary + transfer + matrix coeff
  }
  return 1;
}

static void Read_VOP_Group(SKL_FBB * const Bits, SKL_MP4_I * const VOP)
{
    // section 6.3.4, table 6-19
  int Hours = Bits->Get_Bits(5);
  int Mins  = Bits->Get_Bits(6);
  Bits->Discard(1);                 // marker bit
  int Secs  = Bits->Get_Bits(6);
    // Note: for MPEG1/2, there was 6bits for pic-count, here...
  Bits->Discard(1+1);               // closed gov, broken link
//  printf( " GOV time: %d h %d m %d s\n", Hours, Mins, Secs );
  (void)Hours; (void)Mins; (void)Secs;
}

//////////////////////////////////////////////////////////
// entry point
//////////////////////////////////////////////////////////

int SKL_MP4_I::Decode(const SKL_BYTE *Buf, int Len)
{
  SKL_ASSERT(MPEG_Version==0 || MPEG_Version==4); // don't mix!

  if (Len==0) { // special case: post very last residual frame
    Last = Cur;
    if (Last!=0) {
      Last->Map = 0;  // sanity check
      Last->MV  = 0;  // ""
    }
    Cur  = 0;
    Frame_Number++;
    return 0;
  }

  SKL_UINT32 Code = 0xdeadc0de;
  const SKL_BYTE * const Buf_End = Buf + Len;
  while(1)
  {
    do {
      Code = (Code<<8) | (*Buf++);
      if (Buf>=Buf_End) return Len;
    }
    while(Code>LAST_CODE);
    if (Code<FIRST_CODE) continue;

    SKL_FBB Bits(Buf);

    if (Code>=0x00000130)
    {
      if (Code==SEQ_START_CODE)       // 0x1b0
      {
        const int Profile_Level = *Buf++;   // profile and level (table G-1)
//        printf( "Profile/Level:0x%x\n", Profile_Level);
        if (Profile_Level>=16)
          printf( "Warning! Only Simple Profile is supported. Stream Profile is: 0x%x\n", Profile_Level );
        continue;
      }
      else if (Code==SEQ_END_CODE)    // 0x1b1
      {
        continue;
      }
      else if (Code==USER_DATA_CODE)  // 0x1b2
      {
        continue; // -skip until next code-
      }

      if (VOL_Id<0)  // already have a VOL header?
        continue;

      if (Code==VOP_GROUP_CODE)  // 0x1b3
      {
        Read_VOP_Group(&Bits, this);
        Time_Ref = 0;                // this is a sync point
      }
      else if (Code==VO_START_CODE)   // 0x1b5
      {
        if (!Read_Visual_Object(&Bits))
          continue;
      }
      else if (Code==VOP_CODE)        // 0x1b6
      {
        if (Read_VOP(&Bits, this)) {  // VOP finished?
          Buf = Bits.Pos();
          break;
        }
        else continue;
      }
      else if (Code<SYSTEM_MIN_CODE) {
        /* system code? */
        fprintf( stderr, "Unexpected MPEG2 code 0x%x.\n" , Code );
        continue;
      }
      else {
        /* system code? */
        fprintf( stderr, "Unexpected system code 0x%x. Is stream still multiplexed?\n", Code );
        continue;
      }
    }
    else if (Code>=VOL_CODE) {      // VOL
      int Id = Code - VOL_CODE;     // [0x00..0x0f]
      Init_VOL(Id);      
      if (!Read_VOL_Header(&Bits, this))
        break;
    }
    else {                            // VO
      // int Id = Code - 0x00000100;  // [0x00..0x1f]
    }

    Buf = Bits.Pos();
    Code = 0xd3adc0d3;
  }

  return Len - (Buf_End - Buf);
}

//////////////////////////////////////////////////////////
// SKL_MP4_DEC_I
//////////////////////////////////////////////////////////

SKL_MP4_DEC_I::SKL_MP4_DEC_I(SKL_MEM_I *Mem) 
  : SKL_MP4_I(Mem)
  , SKL_MP4_DEC()
{}

SKL_MP4_DEC_I::~SKL_MP4_DEC_I() 
{}

int SKL_MP4_DEC_I::Ioctl(SKL_CST_STRING Param)
{
  return 0;
}

// Interface Wrappers

SKL_MEM_I *SKL_MP4_DEC_I::Set_Memory_Manager(SKL_MEM_I *New_Mem)
{
  return SKL_MP4_I::Set_Memory_Manager(New_Mem);
}
    
void SKL_MP4_DEC_I::Set_CPU(SKL_CPU_FEATURE Cpu)
{
  SKL_MP4_I::Set_CPU(Cpu);
}
      
int SKL_MP4_DEC_I::Is_Frame_Ready() const
{
  return (Last!=0);
}

void SKL_MP4_DEC_I::Consume_Frame(SKL_MP4_PIC *Pic)
{
  SKL_ASSERT(Is_Frame_Ready() && Pic!=0);
  *Pic = *Last;
  Last = 0;  // TODO: recycle?
}

int SKL_MP4_DEC_I::Get_Frame_Number() const
{
  return Frame_Number;
}

void SKL_MP4_DEC_I::Get_All_Frames(SKL_MP4_PIC *Pic) const
{
  SKL_MP4_I::Get_All_Frames(Pic);
}
                
void SKL_MP4_DEC_I::Set_Slicer(SKL_MP4_SLICER Slicer, SKL_ANY Slicer_Data)
{
  Set_Slicer(Slicer, Slicer_Data);
}

void SKL_MP4_DEC_I::Set_Debug_Level(int Level)
{
  SKL_MP4_I::Set_Debug_Level( Level );
}

int SKL_MP4_DEC_I::Decode(const SKL_BYTE *Buf, int Len)
{
  return SKL_MP4_I::Decode(Buf, Len);
}

int SKL_MP4_DEC_I::Decode_MPEG12(const SKL_BYTE *Buf, int Len)
{
  return SKL_MP4_I::Decode_MPEG12(Buf, Len);
}

//////////////////////////////////////////////////////////
// SKL_MP4_DEC
//////////////////////////////////////////////////////////

SKL_MP4_DEC::SKL_MP4_DEC() {}
SKL_MP4_DEC::~SKL_MP4_DEC() {}

//////////////////////////////////////////////////////////
// C factory

extern "C" {

SKL_EXPORT
SKL_MP4_DEC *Skl_MP4_New_Decoder() {
  return (SKL_MP4_DEC*)::new SKL_MP4_DEC_I((SKL_MEM_I*)0);
}

SKL_EXPORT
void Skl_MP4_Delete_Decoder(SKL_MP4_DEC *Dec) { ::delete (SKL_MP4_DEC_I*)Dec; }

} // extern "C"

//////////////////////////////////////////////////////////
