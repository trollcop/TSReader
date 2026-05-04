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
 * decoder/vc1decmbtab.c: SMPTE VC-1 decoder data tables
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
#include "vc1decmbtab.h"


const vc1DEC_sVLCCode vc1DEC_I_Picture_CBPCY_VLC [65] = /* Table 161 */
{
  {     0,    64,    13},{     1,     1,     0},{     6,     4,    32},
  {     2,     5,     8},{     3,     5,    16},{     5,     5,     3},
  {     6,     5,     4},{     9,     5,     2},{     1,     6,    20},
  {     2,     6,    12},{    20,     6,    48},{    21,     6,    56},
  {    23,     6,     1},{    28,     6,    35},{    30,     6,    24},
  {    31,     6,    40},{     1,     7,    44},{     7,     7,    52},
  {    16,     7,     7},{    18,     7,    36},{    19,     7,    19},
  {    29,     7,    11},{    30,     7,    34},{    32,     7,     6},
  {    34,     7,    28},{    44,     7,    18},{    58,     7,    10},
  {     0,     8,    15},{    13,     8,    60},{    57,     8,    42},
  {    63,     8,    23},{    66,     8,    27},{    70,     8,    22},
  {   119,     8,    14},{     2,     9,    58},{     3,     9,    33},
  {    24,     9,    51},{    25,     9,    55},{    68,     9,    38},
  {    69,     9,    47},{    70,     9,    59},{    71,     9,     5},
  {   112,     9,    39},{   124,     9,     9},{   125,     9,    50},
  {   134,     9,    54},{   135,     9,    31},{   142,     9,    43},
  {   181,     9,    26},{   182,     9,    46},{   183,     9,    17},
  {   236,     9,    13},{   286,    10,    30},{   360,    10,    21},
  {   361,    10,    63},{   474,    10,    62},{   475,    10,    57},
  {   453,    11,    29},{   454,    11,    45},{   455,    11,    53},
  {   574,    11,    41},{   575,    11,    49},{   904,    12,    37},
  {  1810,    13,    25},{  1811,    13,    61}
};

/* End of decoder/vc1decmbtab.c */

