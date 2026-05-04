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
 * shared/vc13dhtab.c: SMPTE VC-1 shared data tables
 *
 *  
 */

/* Include basic types */
#include "vc1types.h"
/* Include any other type definitions that might be needed */
/* Include the declarations for this file */
#include "vc13dhtab.h"


const BYTE8 vc1_High_Mot_Intra_Delta_Level_0 [31] = /* Table 173 */
{
      19,    15,    12,    11,     6,     5,     4,     4,     4,     4,
       3,     3,     3,     3,     3,     3,     2,     2,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1
};

const BYTE8 vc1_High_Mot_Intra_Delta_Level_1 [38] = /* Table 174 */
{
       6,     5,     4,     4,     3,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1
};

const BYTE8 vc1_High_Mot_Intra_Delta_Run_0 [20] = /* Table 175 */
{
      -1,    30,    17,    15,     9,     5,     4,     3,     3,     3,
       3,     3,     2,     1,     1,     1,     0,     0,     0,
       0
};

const BYTE8 vc1_High_Mot_Intra_Delta_Run_1 [7] = /* Table 176 */
{
      -1,    37,    15,     4,     3,     1,     0
};

const BYTE8 vc1_High_Mot_Inter_Delta_Level_0 [27] = /* Table 180 */
{
      23,    11,     8,     7,     5,     5,     4,     4,     3,     3,
       3,     3,     2,     2,     2,     2,     2,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1
};

const BYTE8 vc1_High_Mot_Inter_Delta_Level_1 [37] = /* Table 181 */
{
       9,     5,     4,     4,     3,     3,     3,     2,     2,     2,
       2,     2,     2,     2,     2,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1
};

const BYTE8 vc1_High_Mot_Inter_Delta_Run_0 [24] = /* Table 182 */
{
      -1,    26,    16,    11,     7,     5,     3,     3,     2,     1,
       1,     1,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0
};

const BYTE8 vc1_High_Mot_Inter_Delta_Run_1 [10] = /* Table 183 */
{
      -1,    36,    14,     6,     3,     1,     0,     0,     0,
       0
};

const BYTE8 vc1_Low_Mot_Intra_Delta_Level_0 [21] = /* Table 187 */
{
      16,    11,     8,     7,     5,     4,     4,     3,     3,     3,
       3,     3,     3,     3,     2,     2,     1,     1,     1,     1,
       1
};

const BYTE8 vc1_Low_Mot_Intra_Delta_Level_1 [27] = /* Table 188 */
{
       4,     4,     3,     3,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1
};

const BYTE8 vc1_Low_Mot_Intra_Delta_Run_0 [17] = /* Table 189 */
{
      -1,    20,    15,    13,     6,     4,     3,     3,     2,     1,
       1,     1,     0,     0,     0,     0,     0
};

const BYTE8 vc1_Low_Mot_Intra_Delta_Run_1 [5] = /* Table 190 */
{
      -1,    26,    13,     3,     1
};

const BYTE8 vc1_Low_Mot_Inter_Delta_Level_0 [30] = /* Table 194 */
{
      14,     9,     5,     4,     4,     4,     3,     3,     3,     3,
       3,     3,     3,     2,     2,     2,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,
       1
};

const BYTE8 vc1_Low_Mot_Inter_Delta_Level_1 [44] = /* Table 195 */
{
       5,     4,     3,     3,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1
};

const BYTE8 vc1_Low_Mot_Inter_Delta_Run_0 [15] = /* Table 196 */
{
      -1,    29,    15,    12,     5,     2,     1,     1,     1,     1,
       0,     0,     0,     0,     0
};

const BYTE8 vc1_Low_Mot_Inter_Delta_Run_1 [6] = /* Table 197 */
{
      -1,    43,    15,     3,     1,     0
};

const BYTE8 vc1_Mid_Rate_Intra_Delta_Level_0 [15] = /* Table 201 */
{
      27,    10,     5,     4,     3,     3,     3,     3,     2,     2,
       1,     1,     1,     1,     1
};

const BYTE8 vc1_Mid_Rate_Intra_Delta_Level_1 [21] = /* Table 202 */
{
       8,     3,     2,     2,     2,     2,     2,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1
};

const BYTE8 vc1_Mid_Rate_Intra_Delta_Run_0 [28] = /* Table 203 */
{
      -1,    14,     9,     7,     3,     2,     1,     1,     1,     1,
       1,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0
};

const BYTE8 vc1_Mid_Rate_Intra_Delta_Run_1 [9] = /* Table 204 */
{
      -1,    20,     6,     1,     0,     0,     0,     0,     0
};

const BYTE8 vc1_Mid_Rate_Inter_Delta_Level_0 [27] = /* Table 208 */
{
      12,     6,     4,     3,     3,     3,     3,     2,     2,     2,
       2,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1
};

const BYTE8 vc1_Mid_Rate_Inter_Delta_Level_1 [41] = /* Table 209 */
{
       3,     2,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1
};

const BYTE8 vc1_Mid_Rate_Inter_Delta_Run_0 [13] = /* Table 210 */
{
      -1,    26,    10,     6,     2,     1,     1,     0,     0,     0,
       0,     0,     0
};

const BYTE8 vc1_Mid_Rate_Inter_Delta_Run_1 [4] = /* Table 211 */
{
      -1,    40,     1,     0
};

const BYTE8 vc1_High_Rate_Intra_Delta_Level_0 [15] = /* Table 215 */
{
      56,    20,    10,     7,     6,     5,     4,     3,     3,     3,
       2,     2,     2,     2,     1
};

const BYTE8 vc1_High_Rate_Intra_Delta_Level_1 [17] = /* Table 216 */
{
       4,     3,     3,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     1,     1
};

const BYTE8 vc1_High_Rate_Intra_Delta_Run_0 [57] = /* Table 217 */
{
      -1,    14,    13,     9,     6,     5,     4,     3,     2,     2,
       2,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0
};

const BYTE8 vc1_High_Rate_Intra_Delta_Run_1 [5] = /* Table 218 */
{
      -1,    16,    14,     2,     0
};

const BYTE8 vc1_High_Rate_Inter_Delta_Level_0 [25] = /* Table 222 */
{
      32,    13,     8,     6,     5,     4,     4,     3,     3,     3,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     1,     1
};

const BYTE8 vc1_High_Rate_Inter_Delta_Level_1 [31] = /* Table 223 */
{
       4,     3,     3,     3,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     1,
       1
};

const BYTE8 vc1_High_Rate_Inter_Delta_Run_0 [33] = /* Table 224 */
{
      -1,    24,    22,     9,     6,     4,     3,     2,     2,     1,
       1,     1,     1,     1,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0
};

const BYTE8 vc1_High_Rate_Inter_Delta_Run_1 [5] = /* Table 225 */
{
      -1,    30,    28,     3,     0
};

/* End of shared/vc13dhtab.c */

