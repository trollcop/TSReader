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
 *
 * vc1derivemv.c:
 *
 * Derive chroma motion vectors and direct motion vectors
 */

#include "vc1types.h"
#include "vc1debug.h"
#include "vc1derivemv.h"
#include "vc1tools.h"
#include "vc1gentab.h"
#include "vc1predmv.h"
#include "vc1cropmv.h"

static int vc1DERIVEMV_SignedMod(int a, int b)
{
    return(SIGN(a) * (ABS(a) % b));
}

/*
 * Description:
 * Second stage of chroma motion vector derivation.
 *
 * Remarks:
 * Shared by progressive and interlaced field derive functions.
 *
 * Inputs:
 * pMV          - pointer to a struct which will contain the new MV
 * pPosition    - pointer to current position
 *
 * Outputs:
 * pMV          - updated with new motion vector
 *
 */

static void vc1DERIVEMV_DeriveSecondStageChromaMV(vc1_sMV * pMV, vc1_sPosition * pPosition)
{
    vc1_sMB *pMB = pPosition->pCurMB;
    int IX, IY, IF, N;

    if((vc1_MVIntra == pMV->X) || (vc1_MVIntra == pMV->Y))
    {
        return;
    }
    else
    {
        const int Round[4] = {0, 0, 0, 1};
        int CMV_X, CMV_Y;

        /* Inter */
        IX = pMV->X;
        IY = pMV->Y;
        IF = pMV->BottomField;

        /* First stage Chroma rounding */
        CMV_X = (IX + Round[IX & 3]) >> 1;
        CMV_Y = (IY + Round[IY & 3]) >> 1;

        COVERAGE("8.4.5.10.2");

        /* Second stage chroma motion vector rounding */
        if (pPosition->FastUVMC)
        {
            /* Move CMV_X and CMV_Y to the nearest half pixel positions,
             * rounding towards zero
             */
            const int RndTbl[3] = {1, 0, -1};
            CMV_X = CMV_X + RndTbl[1 + vc1DERIVEMV_SignedMod(CMV_X, 2)];
            CMV_Y = CMV_Y + RndTbl[1 + vc1DERIVEMV_SignedMod(CMV_Y, 2)];
            COVERAGE("8.3.5.4.5");
        }

        N = 0;
        if (IF != pPosition->BottomField)
        {   
            /* Field conversion bias */
            N = 2-4*IF;
        }

        /* Set inter motion vectors */
        pMV->X              = (HWD16)CMV_X;
        pMV->Y              = (HWD16)CMV_Y + (HWD16)N;
		pMV->BottomField    = (FLAG)IF;

        if (pPosition->eProfile != vc1_ProfileAdvanced)
        {
            /* Pull back prior to deblock comparason */
            vc1CROPMV_ChromaPullBack(pPosition, pMV);
        }

        /* Fill them in the current macroblock so that deblocking
         * can compare motion vectors for chroma blocks
         */
        pMB->sBlk[vc1_BlkCb].u.sInter.sMotion[0].sMV = *pMV;
        pMB->sBlk[vc1_BlkCr].u.sInter.sMotion[0].sMV = *pMV;
    }
}

/*
 * Description:
 * Derive a motion vector in a progressive picture
 *
 * Remarks:
 *
 *
 * Inputs:
 * pMV          - pointer to a struct which will contain the new MV
 * pPosition    -
 * Backwards    -
 *
 * Outputs:
 * pMV          - updated with new motion vector
 *
 */

static void vc1DERIVEMV_DeriveProgMV(vc1_sMV * pMV,
                                     vc1_sPosition * pPosition,
                                     int Backwards,
                                     FLAG UseHistory)
{
    vc1_sMB *pMB = pPosition->pCurMB;
    int IX = 0, IY = 0;
    vc1_sMV *pLuMV[4];

    /* First stage chroma motion vector reconstruction */
    if((vc1_MBTypeIs1MV(pMB->eMBType)) && (FALSE == UseHistory))
    {
        IX = pMB->sBlk[0].u.sInter.sMotion[Backwards].sMV.X;
        IY = pMB->sBlk[0].u.sInter.sMotion[Backwards].sMV.Y;
        COVERAGE("8.3.5.4.3");
    }
    else /* 4MV or Use History */
    {
        int Count, MVCount = 0;

        /* count the number of luma motion vectors available */
        for (Count = 0; Count < 4; Count++)
        {
            if(TRUE == UseHistory)
            {
                if(vc1_MVIntra != pPosition->pMVHist->Y[Count].X)
                {
                    pLuMV[MVCount] = &pPosition->pMVHist->Y[Count];
                    MVCount++;
                }
            }
            else
            {
                if (TRUE == vc1_BlkTypeIsInter(pMB->sBlk[Count].eBlkType))
                {
                    pLuMV[MVCount] = &pMB->sBlk[Count].u.sInter.sMotion[Backwards].sMV;
                    MVCount++;
                }
            }
        }  

        /* derive the motion vector */
        switch(MVCount)
        {
            case 0:     /* fall through */
            case 1:
            {
                /* Chroma block is intra coded */
                pMV->X = vc1_MVIntra;
                pMV->Y = vc1_MVIntra;
                pMV->BottomField = 0;
                return;
            }
            case 2:
            {
                IX = (pLuMV[0]->X + pLuMV[1]->X) / 2;
                IY = (pLuMV[0]->Y + pLuMV[1]->Y) / 2;
                break;
            }
            case 3:
            {
                IX = vc1TOOLS_Median3(pLuMV[0]->X, pLuMV[1]->X, pLuMV[2]->X);
                IY = vc1TOOLS_Median3(pLuMV[0]->Y, pLuMV[1]->Y, pLuMV[2]->Y);
                break;
            }
            case 4:
            {
                IX = vc1TOOLS_Median4(pLuMV[0]->X, pLuMV[1]->X, pLuMV[2]->X, pLuMV[3]->X);
                IY = vc1TOOLS_Median4(pLuMV[0]->Y, pLuMV[1]->Y, pLuMV[2]->Y, pLuMV[3]->Y);
                break;
            }
        }
        COVERAGE("8.3.5.4.4");
    }

    pMV->X = (HWD16)IX;
    pMV->Y = (HWD16)IY;
    pMV->BottomField = (FLAG)0;
}

/*
 * Description:
 * Derive a motion vector in a field interlaced picture
 *
 * Remarks:
 * Derives a single motion vector from the (up to 4) luma
 * motion vectors ready for chroma motion vector construction
 * or direct motion vector construction.
 *
 * Inputs:
 * pMV          - pointer to a struct which will contain the new MV
 * pPosition    - pointer to current position
 * Backwards    - FLASE = FowardMV, TRUE = BackwardMV
 * UseHistory   - FALSE = use current MV as input (ChromaMV derivation)
 *                TRUE  = use histort MV as input (DirectMV derivation)
 *
 * Outputs:
 * pMV          - updated with new motion vector
 *
 */

static void vc1DERIVEMV_DeriveIntFieldMV(vc1_sMV * pMV,
                                         vc1_sPosition * pPosition,
                                         int Backwards,
                                         FLAG UseHistory)
{
    vc1_sMB *pMB = pPosition->pCurMB;
    int IX = 0, IY = 0, IF, N;

    IF = pMB->sBlk[0].u.sInter.sMotion[Backwards].sMV.BottomField;
    N = 0;

    if((TRUE == vc1_MBTypeIs1MV(pMB->eMBType)) && (FALSE == UseHistory))
    {
        IX = pMB->sBlk[0].u.sInter.sMotion[Backwards].sMV.X;
        IY = pMB->sBlk[0].u.sInter.sMotion[Backwards].sMV.Y;
        COVERAGE("10.3.4.5.4.2.1");
    }
    else /* 4MV or use History */
    {
        if (UseHistory && pPosition->pMVHist->Y[0].X == vc1_MVIntra)
        {
            /* Block is intra coded */
            pMV->X = vc1_MVIntra;
            pMV->Y = vc1_MVIntra;
            pMV->BottomField = pPosition->BottomField;
            return;
        }

        if( (pPosition->ePictureType == vc1_PictureTypeP) &&
            (pPosition->NumRef == 0))
        {
            /* if we have just one reference picture, take the median of the four vectors */
            IX = vc1TOOLS_Median4(  pMB->sBlk[0].u.sInter.sMotion[Backwards].sMV.X,
                                    pMB->sBlk[1].u.sInter.sMotion[Backwards].sMV.X,
                                    pMB->sBlk[2].u.sInter.sMotion[Backwards].sMV.X,
                                    pMB->sBlk[3].u.sInter.sMotion[Backwards].sMV.X);

            IY = vc1TOOLS_Median4(  pMB->sBlk[0].u.sInter.sMotion[Backwards].sMV.Y,
                                    pMB->sBlk[1].u.sInter.sMotion[Backwards].sMV.Y,
                                    pMB->sBlk[2].u.sInter.sMotion[Backwards].sMV.Y,
                                    pMB->sBlk[3].u.sInter.sMotion[Backwards].sMV.Y);
        }
        else
        {
            int Count, Fields, BFCount = 0, TFCount = 0;
            vc1_sMV pLuBFMV[4], pLuTFMV[4];
            vc1_sMV *pLuMV;

            /*
             * count the number of motion vectors in each field,
             * and separate them into two arrays
             */
            for(Count = 0; Count < 4; Count++)
            {
                if(TRUE == UseHistory)
                {
                    if(1 == pPosition->pMVHist->Y[Count].BottomField)
                    {
                        pLuBFMV[BFCount] = pPosition->pMVHist->Y[Count];
                        BFCount++;
                    }
                    else
                    {
                        pLuTFMV[TFCount] = pPosition->pMVHist->Y[Count];
                        TFCount++;
                    }
                }
                else
                {
                    if(1 == pMB->sBlk[Count].u.sInter.sMotion[Backwards].sMV.BottomField)
                    {
                        pLuBFMV[BFCount] = pMB->sBlk[Count].u.sInter.sMotion[Backwards].sMV;
                        BFCount++;
                    }
                    else
                    {
                        pLuTFMV[TFCount] = pMB->sBlk[Count].u.sInter.sMotion[Backwards].sMV;
                        TFCount++;
                    }
                }
            }

            /*
             * choose whichever is the max, and use those vectors. 
             * If equal, use vectors from current field 
             */
            if (BFCount == TFCount)
            {
                Fields = 2;
                IF = pPosition->BottomField;
                pLuMV = pLuTFMV;
                if (IF)
                {
                    /* Bottom field */
                    pLuMV = pLuBFMV;
                }
            }
            else if(BFCount > TFCount)
            {
                Fields = BFCount;
                pLuMV = pLuBFMV;
                IF = TRUE;
            }   
            else
            {
                Fields = TFCount;
                pLuMV = pLuTFMV;
                IF = FALSE;
            }

            DEBUG3(vc1DEBUG_MV,
                   "DeriveIntFieldMV: TFCount = %d, BFCount = %d, IF = %d\n",
                   TFCount, BFCount, IF);

            /* derive the motion vector */
            switch (Fields)
            {
                case 0:     
                    /* fall through */
                case 1:     
                    FATAL("DeriveIntFieldMV: TFCount = %d, BFCount = %d\n", TFCount, BFCount);
                    return;
                case 2:
                {
                    IX = (pLuMV[0].X + pLuMV[1].X) / 2;
                    IY = (pLuMV[0].Y + pLuMV[1].Y) / 2;
                    break;
                }
                case 3:
                {
                    IX = vc1TOOLS_Median3(pLuMV[0].X, pLuMV[1].X, pLuMV[2].X);
                    IY = vc1TOOLS_Median3(pLuMV[0].Y, pLuMV[1].Y, pLuMV[2].Y);
                    break;
                }
                case 4:
                {
                    IX = vc1TOOLS_Median4(pLuMV[0].X, pLuMV[1].X, pLuMV[2].X, pLuMV[3].X);
                    IY = vc1TOOLS_Median4(pLuMV[0].Y, pLuMV[1].Y, pLuMV[2].Y, pLuMV[3].Y);
                }
            }
        }

        COVERAGE("10.3.4.5.4.2.2");
    }

    pMV->X = (HWD16)IX;
    pMV->Y = (HWD16)IY;
    pMV->BottomField = (FLAG)IF;
}

/*
 * Description:
 * Derive a chroma motion vector in a frame interlaced picture
 *
 * Remarks:
 *
 *
 * Inputs:
 * pMV          - pointer to a struct which will contain the new MV
 * pPosition    - pointer to the position structure representing the current MB
 * Backwards    - if 1, motion vectors are predicting backwards, else forwards
 *
 * Outputs:
 * pMV      - updated with new motion vector
 *
 */

static void vc1DERIVEMV_DeriveIntFrameMV(   vc1_sMV * pMV, 
                                            vc1_sPosition * pPosition, 
                                            int eBlk, 
                                            int Backwards)
{
    int s_RndTbl[4]         = {0, 0, 0, 1};
    int s_RndTblField[16]   = {0, 0, 1, 2, 4, 4, 5, 6, 2, 2, 3, 8, 6, 6, 7, 12};

    vc1_sMV *pLuMV = &pPosition->pCurMB->sBlk[eBlk].u.sInter.sMotion[Backwards].sMV;
    int LMV_X = pLuMV->X;
    int LMV_Y = pLuMV->Y;
    int CMV_X, CMV_Y;

    CMV_X = (pLuMV->X + s_RndTbl[LMV_X & 3]) >> 1;
    if(TRUE == vc1_MBTypeIsFieldMV(pPosition->pCurMB->eMBType))
    {
        CMV_Y = (LMV_Y >> 4) * 8 + s_RndTblField[LMV_Y & 0xF];
    }
    else
    {
        CMV_Y = (LMV_Y + s_RndTbl[LMV_Y & 3]) >> 1;
    }

    pMV->X = (HWD16)CMV_X;
    pMV->Y = (HWD16)CMV_Y;
    pMV->BottomField = 0;
    
    COVERAGE("10.7.2.6");
}





static void vc1DERIVEMV_DeriveIntFrameDirectMV(vc1_sMV pMV[4], vc1_sPosition * pPosition)
{
    vc1_eMBType eMBType = pPosition->pCurMB->eMBType;
    vc1_sMV *pMVHist = pPosition->pMVHist->Y;
    int Count;

    if(TRUE == vc1_MBTypeIs1MV(eMBType))
    {
        /* direct vector is top left of history vectors */
        for(Count = 0; Count < 4; Count++)
        {
            pMV[Count].X = pMVHist[0].X;
            pMV[Count].Y = pMVHist[0].Y;
            pMV[Count].BottomField = 0;
        }
    }
    else if(TRUE == vc1_MBTypeIs2MV(eMBType))
    {
        /* first direct vector is top left of history, second is bottom left */
        for(Count = 0; Count < 4; Count++)
        {
            pMV[Count].X = pMVHist[Count & 2].X;
            pMV[Count].Y = pMVHist[Count & 2].Y;
            pMV[Count].BottomField = 0;
        }
    }
    else if(TRUE == vc1_MBTypeIs4MV(eMBType))
    {
        /* copy vectors from history */
        for(Count = 0; Count < 4; Count++)
        {
            pMV[Count].X = pMVHist[Count].X;
            pMV[Count].Y = pMVHist[Count].Y;
            pMV[Count].BottomField = 0;
        }
    }

    /* check for any intra mvs, and set them to zero */
    for(Count = 0; Count < 4; Count++)
    {
        if(vc1_MVIntra == pMV[Count].X)
        {
            pMV[Count].X = 0;
            pMV[Count].Y = 0;
        }
    }
}

/*
 * Description:
 * Decide if a chroma block is inter or intra, depending on the luma blocks
 * in the same macroblock.
 *
 * Remarks:
 *
 *
 * Inputs:
 * pPosition    - pointer to macroblock position structure
 *
 * Outputs:
 * Sets Cb and Cr block types for current macroblock
 *
 */

void vc1DERIVEMV_DecideChromaBlockType(vc1_sPosition * pPosition)
{
    int Count, MVCount = 0;
    vc1_sMB * pMB = pPosition->pCurMB;
    vc1_eBlkType eBlkType = vc1_BlkIntra;

    for(Count = 0; Count < 4; Count++)
    {
        if(TRUE == vc1_BlkTypeIsInter(pMB->sBlk[Count].eBlkType))
        {
            MVCount++;
            if(MVCount > 1)
            {
                eBlkType = pMB->eBlkType;
                break;
            }
        }
    }

    DEBUG1(vc1DEBUG_MB, "Setting chroma block type to %s\n", vc1DEBUG_BlkType[eBlkType]);

    pMB->sBlk[vc1_BlkCb].eBlkType = eBlkType;
    pMB->sBlk[vc1_BlkCr].eBlkType = eBlkType;
}

/*
 * Description:
 * Store the luma motion vectors to the history buffer.
 *
 * Remarks:
 * Stores the block type (intra/inter) with a defined vc1_MVIntra value.
 *
 * Inputs:
 * pPosition    - pointer to macroblock position structure
 *
 * Outputs:
 * pPosition    - motion vector history buffer updated
 *
 */

void vc1DERIVEMV_StoreMotionVectors(vc1_sPosition * pPosition)
{
    int Count;
    vc1_sBlk *pBlk  = pPosition->pCurMB->sBlk;
    vc1_sMV *pMV    = pPosition->pMVHist->Y;

    if(TRUE == vc1_PictureTypeIsRef(pPosition->ePictureType))
    {
        for(Count = 0; Count < 4; Count++)
        {
            if(TRUE == vc1_BlkTypeIsIntra(pBlk[Count].eBlkType))
            {
                pMV[Count].X            = vc1_MVIntra;
                pMV[Count].Y            = vc1_MVIntra;
                pMV[Count].BottomField  = FALSE;
            }
            else
            {
                pMV[Count].X            = pBlk[Count].u.sInter.sMotion[0].sMV.X;
                pMV[Count].Y            = pBlk[Count].u.sInter.sMotion[0].sMV.Y;
                pMV[Count].BottomField  = pBlk[Count].u.sInter.sMotion[0].sMV.BottomField;
            }
        }


    }
}

/*
 * Description:
 * Derive a chroma motion vector
 *
 * Remarks:
 * eBlk is only used for interlaced frame chroma motion vectors.
 *
 * Inputs:
 * pMV          - pointer to a struct which will contain the new MV
 * pPosition    - pointer to current position
 * eBlk         - luma block number from which chroma mv is derived
 * Backwards    - 0=forwards 1=backwards
 *
 * Outputs:
 * pMV      - updated with new motion vector
 *
 */

void vc1DERIVEMV_DeriveChromaMV(
    vc1_sMV * pMV,
    vc1_sPosition * pPosition,
    int eBlk,
    int Backwards
)
{
    switch(pPosition->ePictureFormat)
    {
        case vc1_ProgressiveFrame:
        {
            vc1DERIVEMV_DeriveProgMV(pMV, pPosition, Backwards, FALSE);
            if (pPosition->eProfile == vc1_ProfileMain && pPosition->ePictureType == vc1_PictureTypeB)
                vc1CROPMV_LumaPullBack(pPosition, pMV);
            vc1DERIVEMV_DeriveSecondStageChromaMV(pMV, pPosition);
            break;
        }
        case vc1_InterlacedField:
        {
            vc1DERIVEMV_DeriveIntFieldMV(pMV, pPosition, Backwards, FALSE);
            vc1DERIVEMV_DeriveSecondStageChromaMV(pMV, pPosition);
            break;
        }
        case vc1_InterlacedFrame:
            vc1DERIVEMV_DeriveIntFrameMV(pMV, pPosition, eBlk, Backwards);    
            break;

        default:   
            FATAL("DeriveChromaMV: Unknown picture format %d\n", pPosition->ePictureFormat);
            return;
    }
}


/*
 * Description:
 * Fill in a missing motion vector with its dominant prediction
 *
 * Inputs:
 * pPos     - pointer to position structure
 * Back     - 0=ForwardMV 1=BackwardMV
 *
 * Outputs:
 * pPos     - Luma motion vectors predicted for current block
 */

static void vc1DERIVEMV_FillInInterlaceFieldMV(vc1_sPosition *pPos, int Back)
{
    vc1_sMB *pMB = pPos->pCurMB;
    vc1_eMBType eMBType = pMB->eMBType;
    vc1_sMV sMV;
    int Blk;

    ASSERT(eMBType != vc1_MBIntra);

    /* Predictors must always be filled in using 1MV prediction */
    pMB->eMBType = vc1_MB1MV;
    vc1PREDMV_PredictInterlacedFieldMV(&sMV, pPos, 0, 0, 0, Back);
    for (Blk=0; Blk<4; Blk++)
    {
        pMB->sBlk[Blk].u.sInter.sMotion[Back].sMV = sMV;
    }
    pMB->eMBType = eMBType;
}

/*
 * Description:
 * Fill in missing motion vectors for an interlaced frame macroblock
 *
 * Remarks
 * For Interlaced Frame B pictures we always fill in the missing
 * vectors assuming that the macroblock type is 2MVF, even if the
 * sent MV's were of type 1MV. (Note that 4MV is not allowed in
 * interlaced frame B pictures).
 *
 * Inputs:
 * pPos     - pointer to position structure
 * Back     - 0=Fill in ForwardMV 1=Fill in BackwardMV
 *
 * Outputs:
 * pPos     - Luma motion vectors filled in for current block
 */

static void vc1DERIVEMV_FillInInterlaceFrameMV(vc1_sPosition *pPos, int Back)
{
    vc1_sMB *pMB = pPos->pCurMB;
    vc1_eMBType eMBType = pMB->eMBType;
    int Blk;

    ASSERT(!vc1_MBTypeIsIntra(eMBType));
    ASSERT(!vc1_MBTypeIs4MV(eMBType));
    
    for (Blk=0; Blk<4; Blk+=2)
    {
        vc1_sMV *pMV = &pMB->sBlk[Blk].u.sInter.sMotion[Back].sMV;

        if (vc1_MBTypeIsSwitchMV(eMBType))
        {
            /* We fill in by copying the MV of the other field */
            *pMV = pMB->sBlk[Blk^2].u.sInter.sMotion[Back].sMV;
        }
        else
        {
            /* Set macroblock type to 2MVF */
            pMB->eMBType = (vc1_eMBType)(vc1_MB2MV | vc1_MBFieldMV);

            /* Predict next motion vector */
            vc1PREDMV_PredictInterlacedFrameMV(pMV, pPos, Blk, Back);

            /* Restore MB Type */
            pMB->eMBType = eMBType;
        }

        /* Fill in the next block MV */
        pMB->sBlk[Blk+1].u.sInter.sMotion[Back].sMV = *pMV;

        if (vc1_MBTypeIsSwitchMV(eMBType))
        {
            /* MV Switch */
            Back = 1-Back;
        }
    }

    if (vc1_MBTypeIs1MV(eMBType))
    {
        /* Store motion vectors as 1MV */
        int X = pMB->sBlk[0].u.sInter.sMotion[Back].sMV.X;
        int Y = pMB->sBlk[0].u.sInter.sMotion[Back].sMV.Y;

        for (Blk=0; Blk<4; Blk++)
        {
            pMB->sBlk[Blk].u.sInter.sMotion[Back].sMV.X = (HWD16)X;
            pMB->sBlk[Blk].u.sInter.sMotion[Back].sMV.Y = (HWD16)Y;
        }
    }
}

/*
 * Description:
 * Calculate and apply the direct motion vector for a B macroblock
 *
 * Remarks:
 * This function is only called for B pictures.
 *
 * Inputs:
 * pPosition    - pointer to current position structure
 *
 * Outputs:
 * pPosition    - If current MB is forward, fills in backward MV
 *              - If current MB is backward, fills in forward MV
 *              - If current MB is direct, fills in both MV
 */

void vc1DERIVEMV_DirectMV(vc1_sPosition * pPosition)
{
    vc1_sMB *pMB = pPosition->pCurMB;
    vc1_eMBType eMBType = pMB->eMBType;
    int ScaleFactor;
    int DX, X;
    int DY, Y;
    int F;
    int HalfPel, Blk;
    vc1_sMV sMV[4];

    switch(pPosition->ePictureFormat)
    {
    case vc1_ProgressiveFrame:
        /* set sMV[0] to the P motion vector */
        vc1DERIVEMV_DeriveProgMV(sMV, pPosition, 0, TRUE);
        if (pPosition->eProfile != vc1_ProfileAdvanced && sMV->X != vc1_MVIntra)
        {
            vc1CROPMV_ChromaPullBack(pPosition, sMV);
        }
        break;

    case vc1_InterlacedField:
        /* Set sMV[0] to the P motion vector */
        vc1DERIVEMV_DeriveIntFieldMV(sMV, pPosition, 0, TRUE);     
        break;

    case vc1_InterlacedFrame:
        /* Set sMV[] to the four P motion vectors */
        vc1DERIVEMV_DeriveIntFrameDirectMV(sMV, pPosition);        
        break;
    
    default:
        FATAL("DeriveChromaMV: Unknown picture format %d\n", pPosition->ePictureFormat);
        return;
    }

    /* if the derived mv was intra, set the mv to 0 */
    if(vc1_MVIntra == sMV[0].X)
    {
        DEBUG0(vc1DEBUG_MV, "Direct P is Intra\n");
        sMV[0].X = 0;
        sMV[0].Y = 0;
        sMV[0].BottomField = pPosition->BottomField;
    }

    X = sMV[0].X;
    Y = sMV[0].Y;
    F = sMV[0].BottomField;

    ScaleFactor = vc1GENTAB_pBFraction[pPosition->BFraction].ScaleFactor;
    HalfPel = vc1_MVModeIsHalfPel(pPosition->eMVMode);
    COVERAGE("8.4.5.4");

    DEBUG1(vc1DEBUG_MV, "Direct Scale Factor = %d\n", ScaleFactor);

    /* Fill in forward MV when required */
    if (vc1_MBTypeIsBackward(eMBType) || vc1_MBTypeIsDirect(eMBType))
    {
        for (Blk=0; Blk<4; Blk++)
        {
            vc1_sMV *pMMV = &pMB->sBlk[Blk].u.sInter.sMotion[0].sMV;

            if(vc1_InterlacedFrame == pPosition->ePictureFormat)
            {
                X = sMV[Blk].X;
                Y = sMV[Blk].Y;
                F = sMV[Blk].BottomField;
            }
            else
            {
                X = sMV[0].X;
                Y = sMV[0].Y;
                F = sMV[0].BottomField;
            }

            DEBUG4(vc1DEBUG_MV, "Direct[%d] P-vector = X=%d Y=%d F=%d\n", Blk, X, Y, F);

            DX = X * ScaleFactor;
            DY = Y * ScaleFactor;
            if (HalfPel)
            {
                DX = 2*((DX+255)>>9);
                DY = 2*((DY+255)>>9);
            }
            else
            {
                DX = (DX+128)>>8;
                DY = (DY+128)>>8;
            }

            DEBUG4(vc1DEBUG_MV, "Direct[%d] Forward  = X=%d Y=%d F=%d\n", Blk, DX, DY, F);

            pMMV->X = (HWD16)DX;
            pMMV->Y = (HWD16)DY;
            pMMV->BottomField = (FLAG)F;
            vc1CROPMV_PPredPullBack(pPosition, pMMV, 0);
        }
    }

    /* Fill in backward MV when required */
    if (vc1_MBTypeIsForward(eMBType) || vc1_MBTypeIsDirect(eMBType))
    {
        for (Blk=0; Blk<4; Blk++)
        {
            vc1_sMV *pMMV = &pMB->sBlk[Blk].u.sInter.sMotion[1].sMV;

            if(vc1_InterlacedFrame == pPosition->ePictureFormat)
            {
                X = sMV[Blk].X;
                Y = sMV[Blk].Y;
                F = sMV[Blk].BottomField;
            }
            else
            {
                X = sMV[0].X;
                Y = sMV[0].Y;
                F = sMV[0].BottomField;
            }

            DEBUG4(vc1DEBUG_MV, "Direct[%d] P-vector = X=%d Y=%d F=%d\n", Blk, X, Y, F);

            DX = X * (ScaleFactor-256);
            DY = Y * (ScaleFactor-256);
            if (HalfPel)
            {
                DX = 2*((DX+255)>>9);
                DY = 2*((DY+255)>>9);
            }
            else
            {
                DX = (DX+128)>>8;
                DY = (DY+128)>>8;
            }

            DEBUG4(vc1DEBUG_MV,
                   "Direct[%d] Backward = X=%d Y=%d F=%d\n",
                   Blk, DX, DY, F);

            pMMV->X = (HWD16)DX;
            pMMV->Y = (HWD16)DY;
            pMMV->BottomField = (FLAG)F;
            vc1CROPMV_PPredPullBack(pPosition, pMMV, 0);
        }
    }
}

/*
 * Description:
 * Derive chorma and direct motion vectors
 *
 * Remarks:
 * Reconstructs forward and/or backward motion vectors depending on the MBType
 * This function must be called exactly once for each macroblock as it also
 * manages the motion vector history for the reference pictures.
 *
 * Inputs:
 * pPosition   - pointer to current position structure
 *
 * Outputs:
 * pPosition   - current macroblock updated with derived motion vectors
 *
 */

void vc1DERIVEMV_DeriveMV(vc1_sPosition * pPosition)
{
    vc1_sMB *pMB = pPosition->pCurMB;
    vc1_eMBType eMBType = pMB->eMBType;

    DEBUG1(vc1DEBUG_MV, "eMBType = %s\n", vc1DEBUG_MBType[eMBType & 15]);

    switch (pPosition->ePictureFormat)
    {
        case vc1_ProgressiveFrame:

            switch(eMBType & vc1_MBDirMask)
            {
                case vc1_MBForward:
                    if (pPosition->ePictureType==vc1_PictureTypeB)
                    {   /* Backward MV = Direct */
                        vc1DERIVEMV_DirectMV(pPosition);
                    }
                    break;

                case vc1_MBBackward:    /* fall through */
                case vc1_MBDirect:      
                    vc1DERIVEMV_DirectMV(pPosition); 
                    break;
            }
            break;
    
        case vc1_InterlacedField:
            switch(eMBType & vc1_MBDirMask)
            {
                case vc1_MBForward:
                    if (pPosition->ePictureType==vc1_PictureTypeB)
                    {
                        /* Fill in backward vectors */
                        vc1DERIVEMV_FillInInterlaceFieldMV(pPosition, 1);
                    }
                    break;
                case vc1_MBBackward:         
                    /* Fill in forward vectors */
                    vc1DERIVEMV_FillInInterlaceFieldMV(pPosition, 0);
                    break;
                case vc1_MBDirect:
                    /* Both MV = Direct */
                    vc1DERIVEMV_DirectMV(pPosition);
                    break;
            }
            break;

        case vc1_InterlacedFrame:
            switch(eMBType & vc1_MBDirMask)
            {
                case vc1_MBForward:
                    if (pPosition->ePictureType==vc1_PictureTypeB)
                    {
                        /* Fill in backward vectors */
                        vc1DERIVEMV_FillInInterlaceFrameMV(pPosition, 1);
                    }
                    break;

                case vc1_MBBackward:         
                    /* Fill in forward vectors */
                    vc1DERIVEMV_FillInInterlaceFrameMV(pPosition, 0);
                    break;

                case vc1_MBDirect:
                    /* Both MV = Direct */
                    vc1DERIVEMV_DirectMV(pPosition);
                    break;
            }
            break;

        case vc1_PictureFormatNone:
            WARN("DeriveMV: Unknown picture format\n");
            return;
    }
}
