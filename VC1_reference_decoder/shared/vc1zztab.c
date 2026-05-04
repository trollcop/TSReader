/*******************************************************************

This software module is provided in full accordance with SMPTE
Administrative Procedures.  
 
This software module was developed by ARM Ltd. under contract by 
Microsoft Corp, and provided by Microsoft Corp to SMPTE.  It was 
edited by:
             
< Placeholder for name/e-mail of other SMPTE members that contribute 
to this module in the future >
            
in the course of development of the SMPTE VC-1 test materials. This 
software module is an implementation of a part of one or more SMPTE 
VC-1 tools as specified by the SMPTE VC-1 standard. 
 
SMPTE gives users of the SMPTE VC-1 standard royalty-free copyright 
license to this software module or modifications thereof for use in 
hardware or software products claiming conformance to the SMPTE VC-1 
standard.  SMPTE does not give users a license under any patents 
relative to the SMPTE VC-1 standard or this software. 
 
Those intending to use this software module in hardware or software 
products are advised that its use may infringe existing patents. The 
companies that developed and provided the original software module, 
the subsequent editors and their companies, and SMPTE have no liability 
for use of this software module or modifications thereof in an 
implementation. 
 
Copyright is not released for non SMPTE VC-1 standard conforming 
products. 
 
The companies listed below retain full right to use their contributed 
code for their own purpose, assign or donate the code to a third party 
and to inhibit third parties from using the code for non SMPTE VC-1 
standard conforming products. 
 
Microsoft Corporation
< Placeholder for other companies whose employees contribute to this 
module in the future>

This copyright notice must be included in all copies or derivative 
works. 
 
Copyright (c) 2004
******************************************************************/

/*
 * shared/vc1zztab.c: SMPTE VC-1 shared data tables
 *
 *  
 */

/* Include basic types */
#include "vc1types.h"
/* Include any other type definitions that might be needed */
/* Include the declarations for this file */
#include "vc1zztab.h"


/* Table inverted */
const BYTE8 vc1_Inv_Intra_Normal_Scan [64] = /* Table 226 */
{
       0,     8,     1,     2,     9,    16,    24,    17,
      10,     3,     4,    11,    18,    25,    32,    40,
      33,    48,    26,    19,    12,     5,     6,    13,
      20,    27,    34,    41,    56,    49,    57,    42,
      35,    28,    21,    14,     7,    15,    22,    29,
      36,    43,    50,    58,    51,    59,    44,    37,
      30,    23,    31,    38,    45,    52,    60,    53,
      61,    46,    39,    47,    54,    62,    55,    63
};

/* Table inverted */
const BYTE8 vc1_Inv_Intra_Horizontal_Scan [64] = /* Table 227 */
{
       0,     1,     8,     2,     3,     9,    16,    24,
      17,    10,     4,     5,    11,    18,    25,    32,
      40,    48,    33,    26,    19,    12,     6,     7,
      13,    20,    27,    34,    41,    56,    49,    57,
      42,    35,    28,    21,    14,    15,    22,    29,
      36,    43,    50,    58,    51,    44,    37,    30,
      23,    31,    38,    45,    52,    59,    60,    53,
      46,    39,    47,    54,    61,    62,    55,    63
};

/* Table inverted */
const BYTE8 vc1_Inv_Intra_Vertical_Scan [64] = /* Table 228 */
{
       0,     8,    16,     1,    24,    32,    40,     9,
       2,     3,    10,    17,    25,    48,    56,    41,
      33,    26,    18,    11,     4,     5,    12,    19,
      27,    34,    49,    57,    50,    42,    35,    28,
      20,    13,     6,     7,    14,    21,    29,    36,
      43,    51,    58,    59,    52,    44,    37,    30,
      22,    15,    23,    31,    38,    45,    60,    53,
      46,    39,    47,    54,    61,    62,    55,    63
};

/* Table inverted */
const BYTE8 vc1_Inv_Inter_8x8_Scan_Simple_Main_Profiles_Progressive_Advanced_Profile [64] = 
/* Table 229 */
{
       0,     8,     1,     2,     9,    16,    24,    17,
      10,     3,     4,    11,    18,    25,    32,    40,
      48,    56,    41,    33,    26,    19,    12,     5,
       6,    13,    20,    27,    34,    49,    57,    58,
      50,    42,    35,    28,    21,    14,     7,    15,
      22,    29,    36,    43,    51,    59,    60,    52,
      44,    37,    30,    23,    31,    38,    45,    53,
      61,    62,    54,    46,    39,    47,    55,    63
};

/* Table inverted */
const BYTE8 vc1_Inv_Inter_8x4_Scan_Simple_Main_Profiles [32] = /* Table 230 */
{
       0,     1,     2,     8,     3,     9,    10,    16,
       4,    11,    17,    24,    18,    12,     5,    19,
      25,    13,    20,    26,    27,     6,    21,    28,
      14,    22,    29,     7,    30,    15,    23,    31
};

/* Table inverted and remapped to index for 8x8 array */
const BYTE8 vc1_Inv_Inter_4x8_Scan_Simple_Main_Profiles [32] = /* Table 231 */
{
       0,     8,     1,    16,
       9,    24,    17,     2,
      32,    10,    25,    40,
      18,    48,    33,    26,
      56,    41,    34,     3,
      49,    57,    11,    42,
      19,    50,    27,    58,
      35,    43,    51,    59
};

/* Table inverted and remapped to index for 8x8 array */
const BYTE8 vc1_Inv_Inter_4x4_Scan_Simple_Main_Profiles_Progressive_Advanced_Profile [16] = 
/* Table 232 */
{
       0,     8,    16,     1,
       9,    24,    17,     2,
      10,    18,    25,     3,
      11,    26,    19,    27
};

/* Table inverted */
const BYTE8 vc1_Inv_Progressive_Inter_8x4_Scan_Advanced_Profile [32] = /* Table 233 */
{
       0,     8,     1,    16,     2,     9,    10,     3,
      24,    17,     4,    11,    18,    12,     5,    19,
      25,    13,    20,    26,    27,     6,    21,    28,
      14,    22,    29,     7,    30,    15,    23,    31
};

/* Table inverted and remapped to index for 8x8 array */
const BYTE8 vc1_Inv_Progressive_Inter_4x8_Scan_Advanced_Profile [32] = /* Table 234 */
{
       0,     1,     8,     2,
       9,    16,    17,    24,
      10,    32,    25,    18,
      40,     3,    33,    26,
      48,    11,    56,    41,
      34,    49,    57,    42,
      19,    50,    27,    58,
      35,    43,    51,    59
};

/* Table inverted */
const BYTE8 vc1_Inv_Interlace_Inter_8x8_Scan_Advanced_Profile [64] = /* Table 235 */
{
       0,     8,     1,    16,    24,     9,     2,    32,
      40,    48,    56,    17,    10,     3,    25,    18,
      11,     4,    33,    41,    49,    57,    26,    34,
      42,    50,    58,    19,    12,     5,    27,    20,
      13,     6,    35,    28,    21,    14,     7,    15,
      22,    29,    36,    43,    51,    59,    60,    52,
      44,    37,    30,    23,    31,    38,    45,    53,
      61,    62,    54,    46,    39,    47,    55,    63
};

/* Table inverted */
const BYTE8 vc1_Inv_Interlace_Inter_8x4_Scan_Advanced_Profile [32] = /* Table 236 */
{
       0,     8,    16,    24,     1,     9,     2,    17,
      25,    10,     3,    18,    26,     4,    11,    19,
      12,     5,    13,    20,    27,     6,    21,    28,
      14,    22,    29,     7,    30,    15,    23,    31
};

/* Table inverted and remapped to index for 8x8 array */
const BYTE8 vc1_Inv_Interlace_Inter_4x8_Scan_Advanced_Profile [32] = /* Table 237 */
{
       0,     1,     2,     8,
      16,     9,    24,    17,
      10,     3,    32,    40,
      48,    56,    25,    18,
      33,    26,    41,    34,
      49,    57,    11,    42,
      19,    50,    27,    58,
      35,    43,    51,    59
};

/* Table inverted and remapped to index for 8x8 array */
const BYTE8 vc1_Inv_Interlace_Inter_4x4_Scan_Advanced_Profile [16] = /* Table 238 */
{
       0,     8,    16,    24,
       1,     9,    17,     2,
      25,    10,    18,     3,
      26,    11,    19,    27
};

/* End of shared/vc1zztab.c */

