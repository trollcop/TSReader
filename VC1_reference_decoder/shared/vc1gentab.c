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
 * vc1gentab.c
 *
 * General shared tables
 *
 *  
 */

#include "vc1types.h"
#include "vc1gentab.h"
#include "vc1zztab.h"
#include "vc1debug.h"

/*
 * Description:
 * Motion vector range value indexed by the extended MV range setting
 */

const vc1GENTAB_sMVRangeParams vc1GENTAB_Motion_Vector_Range_Params_Table[4] =
{
    {9,     8,      256,    128},
    {10,    9,      512,    256},
    {12,    10,     2048,   512},
    {13,    11,     4096,   1024}
};



const vc1_sAspectRatio vc1GENTAB_AspectRatios[VC1_ASPECT_RATIO_TBL_SIZE] =
{
    { 0, 0 },
    { 1, 1 },
    { 12, 11 },
    { 10, 11 },
    { 16, 11 },
    { 40, 33 },
    { 24, 11 },
    { 20, 11 },
    { 32, 11 },
    { 80, 33 },
    { 18, 11 },
    { 15, 11 },
    { 64, 33 },
    { 169, 99 },
    { 0, 0 }
};

const UWORD32 vc1GENTAB_FrameRateNumerators[VC1_FRAMERATENR_TBL_SIZE] =
{
    0, 24000, 25000, 30000, 50000, 60000, 48000, 72000
};

/* Level limits for each profile from Table D.2 of the specification */

#define ANY ((UWORD32)-1)

const vc1_sLevelLimit vc1GENTAB_LevelLimits[4][8] = 
{
    {   /* Simple Profile */
        {   1485,   99,    96,   20, vc1_MVRange64_32    }, /* Low    Level */
        {   5940,  396,   384,   77, vc1_MVRange64_32    }, /* Medium Level */
        {      0,    0,     0,    0, vc1_MVRange64_32    },
    },
    {   /* Main Profile */
        {   7200,  396,  2000,  306, vc1_MVRange128_64   }, /* Low    level */
        {  40500, 1620, 10000,  611, vc1_MVRange512_128  }, /* Medium Level */
        { 245760, 8192, 20000, 2442, vc1_MVRange1024_256 }, /* High   Level */
        {      0,    0,     0,    0, vc1_MVRange64_32    },
    },
    {   /* Reserved Profile */
        {      0,    0,     0,    0, vc1_MVRange64_32    },
    },
    {   /* Advanced Profile */
        {  11880,  396,  2000,  250, vc1_MVRange128_64   }, /* Level 0 */
        {  48600, 1620, 10000, 1250, vc1_MVRange512_128  }, /* Level 1 */
        { 110400, 3680, 20000, 2500, vc1_MVRange512_128  }, /* Level 2 */
        { 245760, 8192, 45000, 5500, vc1_MVRange1024_256 }, /* Level 3 */
        { 491520,16384,135000,16500, vc1_MVRange1024_256 }, /* Level 4 */
        {      0,    0,     0,    0, vc1_MVRange64_32    },
    }
};

/*
 * Description:
 * Table of zig-zag table pointers
 */

const vc1GENTAB_pZigZag vc1GENTAB_pZigZagTables[5][7] = 
{
    {   /* I-picture tables */
        vc1_Inv_Intra_Normal_Scan,
        vc1_Inv_Intra_Vertical_Scan,
        vc1_Inv_Intra_Horizontal_Scan
    },
    {   /* P-picture Simple/Main tables */
        vc1_Inv_Inter_8x8_Scan_Simple_Main_Profiles_Progressive_Advanced_Profile,
        vc1_Inv_Inter_8x8_Scan_Simple_Main_Profiles_Progressive_Advanced_Profile,
        vc1_Inv_Inter_8x8_Scan_Simple_Main_Profiles_Progressive_Advanced_Profile,
        vc1_Inv_Inter_8x8_Scan_Simple_Main_Profiles_Progressive_Advanced_Profile,
        vc1_Inv_Inter_8x4_Scan_Simple_Main_Profiles,
        vc1_Inv_Inter_4x8_Scan_Simple_Main_Profiles,
        vc1_Inv_Inter_4x4_Scan_Simple_Main_Profiles_Progressive_Advanced_Profile
    },
    {   /* P-picture Advanced Progressive tables */
        vc1_Inv_Inter_8x8_Scan_Simple_Main_Profiles_Progressive_Advanced_Profile,
        vc1_Inv_Inter_8x8_Scan_Simple_Main_Profiles_Progressive_Advanced_Profile,
        vc1_Inv_Inter_8x8_Scan_Simple_Main_Profiles_Progressive_Advanced_Profile,
        vc1_Inv_Inter_8x8_Scan_Simple_Main_Profiles_Progressive_Advanced_Profile,
        vc1_Inv_Progressive_Inter_8x4_Scan_Advanced_Profile,
        vc1_Inv_Progressive_Inter_4x8_Scan_Advanced_Profile,
        vc1_Inv_Inter_4x4_Scan_Simple_Main_Profiles_Progressive_Advanced_Profile
    },
    {   /* P-picture Advanced Field Interlaced tables */
        vc1_Inv_Interlace_Inter_8x8_Scan_Advanced_Profile,
        vc1_Inv_Interlace_Inter_8x8_Scan_Advanced_Profile,
        vc1_Inv_Interlace_Inter_8x8_Scan_Advanced_Profile,
        vc1_Inv_Interlace_Inter_8x8_Scan_Advanced_Profile,
        vc1_Inv_Interlace_Inter_8x4_Scan_Advanced_Profile,
        vc1_Inv_Interlace_Inter_4x8_Scan_Advanced_Profile,
        vc1_Inv_Interlace_Inter_4x4_Scan_Advanced_Profile
    },
    {   /* I/P-picture Advanced Frame Interlaced */
        vc1_Inv_Interlace_Inter_8x8_Scan_Advanced_Profile,
        vc1_Inv_Intra_Vertical_Scan,
        vc1_Inv_Intra_Horizontal_Scan,
        vc1_Inv_Interlace_Inter_8x8_Scan_Advanced_Profile,
        vc1_Inv_Interlace_Inter_8x4_Scan_Advanced_Profile,
        vc1_Inv_Interlace_Inter_4x8_Scan_Advanced_Profile,
        vc1_Inv_Interlace_Inter_4x4_Scan_Advanced_Profile
    }
};


/*
 * Description:
 * Choose a zigzag table set given profile, picture type and format
 *
 * Remarks:
 * Selects the table set from vc1GENTAB_pZigZagTables
 *
 * Inputs:
 * pPosition    - pointer to struct providing picture type information
 *
 * Outputs:
 * None
 *
 * Return Value:
 * Zigzag table index
 * 
 */
int vc1GENTAB_ChooseZigZagTableSet(vc1_sPosition * pPosition)
{
    vc1_eProfile eProfile               = pPosition->eProfile;
    vc1_ePictureType ePictureType       = pPosition->ePictureType;
    vc1_ePictureFormat ePictureFormat   = pPosition->ePictureFormat;

    if (ePictureFormat == vc1_InterlacedFrame)
    {
        return 4;
    }

    if(TRUE == vc1_PictureTypeIsIntra(ePictureType))
    {
        return(0);
    }
    else
    {
        /* Assume P frame */
        if((vc1_ProfileSimple == eProfile) || (vc1_ProfileMain == eProfile))
        {
            COVERAGE("8.3.6.1.4"); /* Table referenced encodes use of same zigzag for most types */
            return(1);
        }
        else
        {
            /* advanced profile */
            if(vc1_ProgressiveFrame == ePictureFormat)
            {
                return(2);
            }
            else /* Interlaced Field */
            {
                return(3);
            }
        }
    }
}



/* Description:
 * Table of BFraction numerator and denominator values
 */

const vc1_sBFraction vc1GENTAB_pBFraction[VC1_BFRACTION_TBL_SIZE] =
{
    /* Numerator, Denominator, ScaleFactor (proportion of 256) */
    { 1, 2, 128 },
    { 1, 3,  85 },
    { 2, 3, 170 },
    { 1, 4,  64 },
    { 3, 4, 192 },
    { 1, 5,  51 },
    { 2, 5, 102 },
    { 3, 5, 153 },
    { 4, 5, 204 },
    { 1, 6,  43 },
    { 5, 6, 215 },
    { 1, 7,  37 },
    { 2, 7,  74 },
    { 3, 7, 111 },
    { 4, 7, 148 },
    { 5, 7, 185 },
    { 6, 7, 222 },
    { 1, 8,  32 },
    { 3, 8,  96 },
    { 5, 8, 160 },
    { 7, 8, 224 },
    { 1, 0,   0 },
    { 2, 0,   0 }
};
