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
 * decoder/vc1decbitpltab.c: SMPTE VC-1 decoder data tables
 *
 *  
 */

/* Include basic types */
#include "vc1types.h"
/* Include any other type definitions that might be needed */
#include "vc1dec.h"
#include "vc1decbit.h"
#include "vc1dec3dh.h"
/* Include the declarations for this file */
#include "vc1decbitpltab.h"


const vc1DEC_sVLCCode vc1DEC_Code_3x2_2x3_tiles [65] = /* Table 73 */
{
  {     0,    64,    13},{     1,     1,     0},{     2,     4,     1},
  {     3,     4,     2},{     4,     4,     4},{     5,     4,     8},
  {     6,     4,    16},{     7,     4,    32},{     7,     6,    63},
  {     0,     8,     3},{     1,     8,     5},{     2,     8,     6},
  {     3,     8,     9},{     4,     8,    10},{     5,     8,    12},
  {     6,     8,    17},{     7,     8,    18},{     8,     8,    20},
  {     9,     8,    24},{    10,     8,    33},{    11,     8,    34},
  {    12,     8,    36},{    13,     8,    40},{    14,     8,    48},
  {    50,     9,    62},{    51,     9,    61},{    52,     9,    59},
  {    53,     9,    55},{    54,     9,    47},{    55,     9,    31},
  {    67,    10,    35},{    69,    10,    37},{    70,    10,    38},
  {    71,    10,     7},{    73,    10,    41},{    74,    10,    42},
  {    75,    10,    11},{    76,    10,    44},{    77,    10,    13},
  {    78,    10,    14},{    81,    10,    49},{    82,    10,    50},
  {    83,    10,    19},{    84,    10,    52},{    85,    10,    21},
  {    86,    10,    22},{    88,    10,    56},{    89,    10,    25},
  {    90,    10,    26},{    92,    10,    28},{   768,    13,    60},
  {   769,    13,    58},{   770,    13,    57},{   771,    13,    54},
  {   772,    13,    53},{   773,    13,    51},{   774,    13,    46},
  {   775,    13,    45},{   776,    13,    43},{   777,    13,    39},
  {   778,    13,    30},{   779,    13,    29},{   780,    13,    27},
  {   781,    13,    23},{   782,    13,    15}
};

/* End of decoder/vc1decbitpltab.c */

