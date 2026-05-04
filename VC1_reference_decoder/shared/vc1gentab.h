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
 * vc1gentab.h
 *
 * General shared tables
 *
 *  
 */

#ifndef VC1GENTAB_H
#define VC1GENTAB_H

#ifdef cplusplus
extern "C" {
#endif

/*
 * Description:
 * Table of zig-zag table pointers
 */

typedef const BYTE8 * vc1GENTAB_pZigZag;

extern const vc1GENTAB_pZigZag vc1GENTAB_pZigZagTables[5][7];

extern const vc1_sLevelLimit vc1GENTAB_LevelLimits[4][8];

typedef struct
{
    UBYTE8 Width;
    UBYTE8 Height;
} vc1_sAspectRatio;

#define VC1_ASPECT_RATIO_TBL_SIZE 15
extern const vc1_sAspectRatio vc1GENTAB_AspectRatios[VC1_ASPECT_RATIO_TBL_SIZE];

#define VC1_FRAMERATENR_TBL_SIZE 8
extern const UWORD32 vc1GENTAB_FrameRateNumerators[VC1_FRAMERATENR_TBL_SIZE];

#define VC1_BFRACTION_TBL_SIZE 23
#define VC1_BFRACTION_BI    22  /* BFraction code for BI frames */
#define VC1_BFRACTION_NUM   21  /* Number of valid BFraction settings */
extern const vc1_sBFraction vc1GENTAB_pBFraction[VC1_BFRACTION_TBL_SIZE];


typedef struct
{
    UBYTE8      KX;
    UBYTE8      KY;
    UHWD16      RangeX;
    UHWD16      RangeY;
} vc1GENTAB_sMVRangeParams;

/*
 * Description:
 * Motion vector range value indexed by the extended MV range setting
 */

extern const vc1GENTAB_sMVRangeParams vc1GENTAB_Motion_Vector_Range_Params_Table[4];



/**
 * Description:
 * Choose a zigzag table set, given the profile, picture type and format.
 *
 * Remarks:
 * Selects the table set from vc1GENTAB_pZigZagTables.
 *
 * Inputs:
 * pPosition    - pointer to struct providing picture type information
 *
 * Return Value:
 * Zigzag table index.
 * 
 */
int vc1GENTAB_ChooseZigZagTableSet(vc1_sPosition * pPosition);


#ifdef cplusplus
}
#endif
#endif
