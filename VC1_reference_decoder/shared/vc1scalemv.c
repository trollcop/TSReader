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
 * vc1scalemv.c
 *
 * Scale motion vectors for field interlaced motion vector prediction
 *
 */

#include "vc1types.h"
#include "vc1debug.h"
#include "vc1scalemv.h"
#include "vc1gentab.h"

/*
 * Description:
 * Table of motion vector scaling values indexed by mode and frame distance
 */

static const vc1_sScaleMV vc1SCALEMV_pScaleTable[3][4] =
{
    { /* Forward 1st or Backward 2nd  */
        { 128,  512, 219, 32,  8, 37, 10, FALSE },
        { 192,  341, 236, 48, 12, 20,  5, FALSE },
        { 213,  307, 242, 53, 13, 14,  4, FALSE },
        { 224,  293, 245, 56, 14, 11,  3, FALSE }
    },
    { /* Forward 2nd */
        { 128,  512, 219, 32,  8, 37, 10, FALSE },
        {  64, 1024, 204, 16,  4, 52, 13, FALSE },
        {  43, 1536, 200, 11,  3, 56, 14, FALSE },
        {  32, 2048, 198,  8,  2, 58, 15, FALSE }
    },
    { /* Backward 1st */
        { 171,  384, 230, 43, 11, 26,  7, TRUE },
        { 205,  320, 239, 51, 13, 17,  4, TRUE },
        { 219,  299, 244, 55, 14, 12,  3, TRUE },
        { 228,  288, 246, 57, 14, 10,  3, TRUE }
    }
};

/*
 * Description:
 * Initialize the motion vector interlace scaling structures
 *
 * Inputs:
 * pPos     - pointer to position structure
 *
 * Outputs:
 * pPos->pScaleMV[0]  - forward motion vector scaling parameters (P and B pictures)
 * pPos->pScaleMV[1]  - backward motion vector scaling parameters (B pictures only)
 */

void vc1SCALEMV_InitScaleMV(vc1_sPosition *pPos)
{
    int RefDist, Index;

    /* Work out forward MV Scaling */

    /* Calculate forward reference frame distance RefDist */
    RefDist = pPos->pReferenceNew->RefDist;

    DEBUG2(vc1DEBUG_MV, "InitScaleMV: P RefDist=%d MVRange=%d\n", RefDist, pPos->eMVRange);

    if (pPos->ePictureType == vc1_PictureTypeB)
    {
        int ScaleFactor = vc1GENTAB_pBFraction[pPos->BFraction].ScaleFactor;

        RefDist = (ScaleFactor * RefDist)>>8;
        COVERAGE("10.4.5.2");
        DEBUG1(vc1DEBUG_MV, "InitScaleMV: B FRFD=%d\n", RefDist);
    }

    /* Find table entry */
    Index = RefDist;
    if (Index >= 4)
    {
        Index = 3;
    }

    /* Select forward motion vector scaling table */
    pPos->pScaleMV[0] = vc1SCALEMV_pScaleTable[pPos->SecondField][Index];

    /* Fill in extra parameters needed for the MVRange clip */
    pPos->pScaleMV[0].eMVRange    = pPos->eMVRange;
    pPos->pScaleMV[0].eMVMode     = pPos->eMVMode;
    pPos->pScaleMV[0].BottomField = pPos->BottomField;

    if (pPos->ePictureType != vc1_PictureTypeB)
    {
        /* Backward scaling not required */
        return;
    }

    /* Calculate backward reference frame distance */
    RefDist = pPos->pReferenceNew->RefDist - RefDist - 1;
	if (RefDist<0) RefDist=0; /* fix 02/08/05 */
    DEBUG1(vc1DEBUG_MV, "InitScaleMV: B BRFD=%d\n", RefDist);

    /* Find table entry */
    Index = RefDist;
    if (Index >= 4)
    {
        Index = 3;
    }

    if (pPos->SecondField)
    {
        pPos->pScaleMV[1] = vc1SCALEMV_pScaleTable[0][Index];
    }
    else
    {
        pPos->pScaleMV[1] = vc1SCALEMV_pScaleTable[2][Index];
    }

    COVERAGE("10.4.5.3");

    /* Fill in extra parameters needed for the MVRange clip */
    pPos->pScaleMV[1].eMVRange    = pPos->eMVRange;
    pPos->pScaleMV[1].eMVMode     = pPos->eMVMode;
    pPos->pScaleMV[1].BottomField = pPos->BottomField;
}


/*
 * Description:
 * Scale an interlaced motion vector from same to opposite field
 *
 * Remarks:
 * All scaling takes place in the coordinate system of the same
 * field.
 *
 * Inputs:
 * pX       - pointer to X of 1/4 pel motion vector
 * pY       - pointer to Y of 1/4 pel motion vector
 * pSMV     - pointer to motion vector scaling parameter
 * Opposite - 0 = Scale for same field
 *            1 = Scale for opposite field
 *
 * Outputs:
 * pX       - pointer to scaled X
 * pY       - pointer to scaled Y
 */

void vc1SCALEMV_ScaleMV(
    int *pX,
    int *pY,
    const vc1_sScaleMV *pSMV,
    int Opposite
)
{
    int X = *pX;
    int Y = *pY;

    DEBUG5(vc1DEBUG_MV, "Scale MV: X=%d Y=%d Opposite=%d ScaleUpOpp=%d Scale=%d\n",
        X, Y, Opposite, pSMV->ScaleUpOpp, pSMV->Scale);

    if (vc1_MVModeIsHalfPel(pSMV->eMVMode))
    {
        ASSERT((X&1)==0);
        ASSERT((Y&1)==0);
        X = X>>1;
        Y = Y>>1;
    }


    if (Opposite != pSMV->ScaleUpOpp)
    {        /* Scale vectors down */
        X = ((X * pSMV->Scale) >> 8);
        Y = ((Y * pSMV->Scale) >> 8);
    }
    else /* Scale vectors up */
    {
        int A = ABS(X);
        int R = vc1GENTAB_Motion_Vector_Range_Params_Table[pSMV->eMVRange].RangeX;

        if (A > 255)
        {
            /* No change */
        }
        else if (A < pSMV->ScaleZone1X)
        {
            X = (X * pSMV->Scale1) >> 8;
        }
        else if (X < 0)
        {
            X = ((X * pSMV->Scale2) >> 8 ) - pSMV->Zone1OffsetX;
        }
        else
        {
            X = ((X * pSMV->Scale2) >> 8 ) + pSMV->Zone1OffsetX;
        }

        /* Clip to MVRange */
        if (X >= R)
        {
            X = R-1;
        }
        else if (X < -R)
        {
            X = -R;
        }

        A = ABS(Y);
        R = vc1GENTAB_Motion_Vector_Range_Params_Table[pSMV->eMVRange].RangeY >> 1;

        if (A > 63)
        {
            /* No change */
        }
        else if (A < pSMV->ScaleZone1Y)
        {
            Y = (Y * pSMV->Scale1) >> 8;
        }
        else if (Y < 0)
        {
            Y = ((Y * pSMV->Scale2) >> 8) - pSMV->Zone1OffsetY;
        }
        else
        {
            Y = ((Y * pSMV->Scale2) >> 8) + pSMV->Zone1OffsetY;
        }

        /* Clip to MVRange */
        if (pSMV->BottomField && Opposite)
        {
            Y--;
        }
        if (Y >= R)
        {
            Y = R-1;
        }
        else if (Y < -R)
        {
            Y = -R;
        }
        if (pSMV->BottomField && Opposite)
        {
            Y++;
        }
    }

    if (vc1_MVModeIsHalfPel(pSMV->eMVMode))
    {
        X = X<<1;
        Y = Y<<1;
    }

    *pX = (HWD16)X;
    *pY = (HWD16)Y;
}
