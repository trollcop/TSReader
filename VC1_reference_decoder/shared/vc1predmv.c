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
 * vc1predmv.c
 *
 * Predict motion vectors
 *
 * 
 */

#include "vc1types.h"
#include "vc1debug.h"
#include "vc1tools.h"
#include "vc1pred.h"
#include "vc1predmv.h"
#include "vc1scalemv.h"
#include "vc1cropmv.h"

/*
 * Description:
 * Predict the X and Y motion vectors for a progressive block
 *
 * Remarks:
 * For a 1MV macroblock Blk is ignored
 * For a 4MV macroblock can call to predict any of the four MVs
 *
 * Inputs:
 * pPred        - Pointer to the predicted output of X, Y
 * pPos         - Pointer to the macroblock being predicted
 * Blk          - Block number within the macroblock (for 4MV)
 * eHybridPred  - Hybrid prediction direction
 * Back         - 0=predict forward motion 1=predict backward motion
 *
 * Outputs:
 * pPred        - predicted output values written
 *
 * Return Value:
 * TRUE if Hybrid prediction applied, otherwise FALSE
 *
 */

int vc1PREDMV_PredictProgressiveMV(
    vc1_sMV *pPred,
    vc1_sPosition *pPos,
    int Blk,
    int eHybridPred,
    int Back
)
{
    vc1_sMB *pMB = pPos->pCurMB;
    vc1_sBlk *pA=NULL, *pB=NULL, *pC=NULL;
    int AX=0, AY=0;     /* predictorA */
    int BX=0, BY=0;     /* predictorB */
    int CX=0, CY=0;     /* predictorC */
    int X=0,  Y=0;      /* predictor */
    int Sum;

    /* Find predictor blocks */
    if (vc1_MBTypeIs1MV(pMB->eMBType))
    {
        pA = vc1PRED_pTopBlk(pPos, vc1_BlkY0);
        pB = vc1PRED_pB1MVBlk(pPos);
        pC = vc1PRED_pLeftBlk(pPos, vc1_BlkY0);
        COVERAGE("8.3.5.3.1");
    }
    else
    {
        pA = vc1PRED_pTopBlk(pPos, Blk);
        pB = vc1PRED_pB4MVBlk(pPos, Blk);
        pC = vc1PRED_pLeftBlk(pPos, Blk);
        COVERAGE("8.3.5.3.2");
    }

    /* Find predictors */
    if (pA && vc1_BlkTypeIsInter(pA->eBlkType))
    {
        AX = pA->u.sInter.sMotion[Back].sMV.X;
        AY = pA->u.sInter.sMotion[Back].sMV.Y;
        DEBUG2(vc1DEBUG_MV, "Predict A : MV_X = %d, MV_Y = %d\n", AX, AY);
    }

    if (pB && vc1_BlkTypeIsInter(pB->eBlkType))
    {
        BX = pB->u.sInter.sMotion[Back].sMV.X;
        BY = pB->u.sInter.sMotion[Back].sMV.Y;
        DEBUG2(vc1DEBUG_MV, "Predict B : MV_X = %d, MV_Y = %d\n", BX, BY);
    }

    if (pC && vc1_BlkTypeIsInter(pC->eBlkType))
    {
        CX = pC->u.sInter.sMotion[Back].sMV.X;
        CY = pC->u.sInter.sMotion[Back].sMV.Y;
        DEBUG2(vc1DEBUG_MV, "Predict C : MV_X = %d, MV_Y = %d\n", CX, CY);
    }

    /* Calculate motion vector predictor */
    if (pA)
    {
        if (pB==NULL)   /* This forces pC==NULL */
        {
            X = AX;
            Y = AY;
        }
        else
        {
            X = vc1TOOLS_Median3(AX, BX, CX);
            Y = vc1TOOLS_Median3(AY, BY, CY);
        }
    }
    else if (pC)
    {
        X = CX;
        Y = CY;
    }
    COVERAGE("8.3.5.3.3");

    pPred->X = (HWD16)X;
    pPred->Y = (HWD16)Y;
    pPred->BottomField = 0;

    DEBUG3(vc1DEBUG_MV, "Predict MV: MV_X = %d, MV_Y = %d Back = %d\n", X, Y, Back);

    /* Crop motion vector to reference picture */
    if (pPos->ePictureType==vc1_PictureTypeB)
    {
        vc1CROPMV_BPredPullBack(pPos, pPred, Blk);
    }
    else
    {
        vc1CROPMV_PPredPullBack(pPos, pPred, Blk);
    }

    /* See if hybrid prediction is required */
    if (pA==NULL || pC==NULL || pPos->ePictureType==vc1_PictureTypeB)
    {
        return FALSE;   /* No Hybrid prediction required */
    }

    X = pPred->X;
    Y = pPred->Y;

    Sum = ABS(X-AX) + ABS(Y-AY);
    if (Sum <= 32)
    {
        Sum = ABS(X-CX) + ABS(Y-CY);
    }
    if (Sum <= 32)
    {
        return FALSE;   /* No Hybrid prediction required */
    }

    /* Apply hybrid prediction */
    if (eHybridPred == vc1_HybridTop)
    {
        X = AX;
        Y = AY;
    }
    else
    {
        X = CX;
        Y = CY;
    }

    pPred->X = (HWD16)X;
    pPred->Y = (HWD16)Y;
    COVERAGE("8.3.5.3.4");
    DEBUG2(vc1DEBUG_MV, "Hybrid  MV: MV_X = %d, MV_Y = %d\n", X, Y);

    return TRUE;    /* Hybrid prediction used */
}

/*
 * Description:
 * Predict the X and Y motion vectors for an interlace field block
 *
 * Remarks:
 * For a 1MV macroblock Blk is ignored
 * For a 4MV macroblock can call to predict any of the four MVs
 *
 * Inputs:
 * pPred        - Pointer to the predicted output of X, Y
 * pPos         - Pointer to the macroblock being predicted
 * Blk          - Block number within the macroblock (for 4MV)
 * eHybridPred  - Hybrid prediction direction
 * PredFlag     - 0=use dominant predictor 1=use non-dominant predictor
 * Back         - 0=predict forward motion 1=predict backward motion
 *
 * Outputs:
 * pPred        - predicted output values written
 *
 * Return Value:
 * TRUE if Hybrid prediction applied, otherwise FALSE
 *
 */

int vc1PREDMV_PredictInterlacedFieldMV(
    vc1_sMV *pPred,
    vc1_sPosition *pPos,
    int Blk,
    int eHybridPred,
    int PredFlag,
    int Back
)
{
    vc1_sMB *pMB = pPos->pCurMB;
    vc1_sBlk *pA=NULL, *pB=NULL, *pC=NULL;
    vc1_sScaleMV       *pSMV = &pPos->pScaleMV[Back];
    int  X=0,  Y=0,  F=pPos->BottomField;  /* Current pos  */
    int AX=0, AY=0, AF=F; /* predictorA default */
    int BX=0, BY=0, BF=F; /* predictorB default */
    int CX=0, CY=0, CF=F; /* predictorC default */
    int SameCount=0;
    int OppositeCount=0;
    int Sum, Opposite;


    /* Find predictor blocks */
    if (vc1_MBTypeIs1MV(pMB->eMBType))
    {
        pA = vc1PRED_pTopBlk(pPos, vc1_BlkY0);
        pB = vc1PRED_pB1MVBlk(pPos);
        pC = vc1PRED_pLeftBlk(pPos, vc1_BlkY0);
    }
    else
    {
        pA = vc1PRED_pTopBlk(pPos, Blk);
        pB = vc1PRED_pB4MVBlk(pPos, Blk);
        pC = vc1PRED_pLeftBlk(pPos, Blk);
        if (pC==NULL && pPos->WidthMB==1)
        {
            /* One macroblock wide 4MV, C doesn't exist */
            pB = NULL;
        }
    }

    /* Find predictors */
    if (pA && vc1_BlkTypeIsInter(pA->eBlkType))
    {
        AX = pA->u.sInter.sMotion[Back].sMV.X;
        AY = pA->u.sInter.sMotion[Back].sMV.Y;
        AF = pA->u.sInter.sMotion[Back].sMV.BottomField;
        if (AF == F)
        {
            SameCount++;
        }
        else
        {
            OppositeCount++;
        }
        DEBUG3(vc1DEBUG_MV, "Predict A : MV_X = %d, MV_Y = %d MV_F = %d\n", AX, AY, AF);
    }

    if (pB && vc1_BlkTypeIsInter(pB->eBlkType))
    {
        BX = pB->u.sInter.sMotion[Back].sMV.X;
        BY = pB->u.sInter.sMotion[Back].sMV.Y;
        BF = pB->u.sInter.sMotion[Back].sMV.BottomField;
        if (BF == F)
        {
            SameCount++;
        }
        else
        {
            OppositeCount++;
        }
        DEBUG3(vc1DEBUG_MV, "Predict B : MV_X = %d, MV_Y = %d MV_F = %d\n", BX, BY, BF);
    }

    if (pC && vc1_BlkTypeIsInter(pC->eBlkType))
    {
        CX = pC->u.sInter.sMotion[Back].sMV.X;
        CY = pC->u.sInter.sMotion[Back].sMV.Y;
        CF = pC->u.sInter.sMotion[Back].sMV.BottomField;

        if (CF == F)
        {
            SameCount++;
        }
        else
        {
            OppositeCount++;
        }
        DEBUG3(vc1DEBUG_MV, "Predict C : MV_X = %d, MV_Y = %d MV_F = %d\n", CX, CY, CF);
    }

    /* Find the source field for prediction Same/Opposite */
    if (pPos->ePictureType==vc1_PictureTypeP && pPos->NumRef==0)
    {
        /* Single reference P picture
         * REFFIELD determines reference field
         */
        Opposite = 1 - pPos->RefField;
        Sum = SameCount + OppositeCount;
        COVERAGE("10.3.2");
        COVERAGE("10.3.4.5.3.4.1");
        DEBUG2(vc1DEBUG_MV, "Predict F : One Field F = %d, Opposite = %d\n", F, Opposite);
    }
    else
    {
        /* Two reference pictures
         * PredFlag determines reference field
         * 0 = Dominant predictor
         * 1 = Non-Dominant predictor
         */
        if (SameCount <= OppositeCount)
        {
            /* Dominant = Opposite */
            Opposite = 1-PredFlag;
        }
        else
        {
            /* Dominiant = Same */
            Opposite = PredFlag;
        }
        Sum = SameCount + OppositeCount;
        COVERAGE("10.3.4.5.3.3");
        DEBUG2(vc1DEBUG_MV, "Predict F : Two Field F = %d, Opposite = %d\n", F, Opposite);
    }


    /* Scale predictor to the correct reference field */
    if (Opposite==0)
    {
        /* Same - Scale for same */
        if (AF!=F)
        {
            vc1SCALEMV_ScaleMV(&AX, &AY, pSMV, 0);
        }
        if (BF!=F)
        {
            vc1SCALEMV_ScaleMV(&BX, &BY, pSMV, 0);
        }
        if (CF!=F)
        {
            vc1SCALEMV_ScaleMV(&CX, &CY, pSMV, 0);
        }
    }
    else
    {
        /* Opposite - Scale for opposite */
        F = 1-F;    /* motion vector is in opposite */
        if (AF!=F)
        {
            vc1SCALEMV_ScaleMV(&AX, &AY, pSMV, 1);
        }
        if (BF!=F)
        {
            vc1SCALEMV_ScaleMV(&BX, &BY, pSMV, 1);
        }
        if (CF!=F)
        {
            vc1SCALEMV_ScaleMV(&CX, &CY, pSMV, 1);
        }
    }
    COVERAGE("10.3.4.5.3.4.2");

    /* Calculate motion vector predictor */
    if (pA)
    {
        DEBUG3(vc1DEBUG_MV, "Scaled  A : MV_X = %d, MV_Y = %d MV_F = %d\n", AX, AY, F);
        if (pB==NULL)
        {
            /* Image must be one macroblock wide */
            X = AX;
            Y = AY;
        }
        else if (Sum > 1) /* At least two of A,B,C inter */
        {
            DEBUG3(vc1DEBUG_MV, "Scaled  B : MV_X = %d, MV_Y = %d MV_F = %d\n", BX, BY, F);
            DEBUG3(vc1DEBUG_MV, "Scaled  C : MV_X = %d, MV_Y = %d MV_F = %d\n", CX, CY, F);
            X = vc1TOOLS_Median3(AX, BX, CX);
            Y = vc1TOOLS_Median3(AY, BY, CY);
        }
        else /* At most one of A,B,C is Inter - choose this one */
        {
            X = AX;
            Y = AY;
            if (pB && vc1_BlkTypeIsInter(pB->eBlkType))
            {
                X = BX;
                Y = BY;
            }
            if (pC && vc1_BlkTypeIsInter(pC->eBlkType))
            {
                X = CX;
                Y = CY;
            }
        }
    }
    else /* Use C as C exists or will be the default predictor */
    {
        DEBUG3(vc1DEBUG_MV, "Scaled  C : MV_X = %d, MV_Y = %d MV_F = %d\n", CX, CY, F);
        X = CX;
        Y = CY;
    }

    pPred->X = (HWD16)X;
    pPred->Y = (HWD16)Y;
    pPred->BottomField = (FLAG)F;

    DEBUG4(vc1DEBUG_MV, "Predict MV: MV_X = %d, MV_Y = %d MV_F = %d Back = %d\n", X, Y, F, Back);

    /* See if hybrid prediction is required */
    COVERAGE("10.3.4.5.3.5");
    if (pA==NULL || pC==NULL || 
        vc1_BlkTypeIsIntra(pA->eBlkType) ||
        vc1_BlkTypeIsIntra(pC->eBlkType) || 
        pPos->ePictureType==vc1_PictureTypeB)
    {
        return FALSE;   /* No Hybrid prediction required */
    }

    X = pPred->X;
    Y = pPred->Y;
    Sum = ABS(X-AX) + ABS(Y-AY);
    if (Sum <= 32)
    {
        Sum = ABS(X-CX) + ABS(Y-CY);
    }
    if (Sum <= 32)
    {
        return FALSE;   /* No Hybrid prediction required */
    }

    /* Apply hybrid prediction */
    if (eHybridPred == vc1_HybridTop)
    {
        X = AX;
        Y = AY;
    }
    else
    {
        X = CX;
        Y = CY;
    }

    pPred->X = (HWD16)X;
    pPred->Y = (HWD16)Y;
    DEBUG3(vc1DEBUG_MV, "Hybrid  MV: MV_X = %d, MV_Y = %d MV_F = %d\n", X, Y, pPred->BottomField);

    return TRUE;    /* Hybrid prediction used */
}


/*
 * Description:
 * Interlaced frame: find a MB motion vector predictor
 *
 * Inputs:
 * pPred    - pointer to buffer for the predicted result
 * pPos     - pointer to position structure
 * pA       - pointer to adjacent macroblock (NULL if none)
 * BlkA     - block number in adjacent MB
 * Blk      - block number in MB being predicted
 * Back     - 0=foward mv 1=backward mv
 *
 * Returns:
 * 0=no predictor 1=a predictor written
 */

static int vc1PREDMV_FrameMBPred(
    vc1_sMV *pPred,
    vc1_sPosition *pPos,
    vc1_sMB *pA,
    int BlkA,
    int Blk,
    int Back
)
{
    int FieldMV, FieldMVA;
    int X,Y;

    /* Check for absent neighbour */
    if (pA==NULL)
    {
        DEBUG0(vc1DEBUG_MV, "Candidate : Absent\n");
        return 0;
    }

    /* Check for circular buffer wrap */
    if (pA < pPos->pStartMB)
    {
        pA += pPos->SizeMB;
    }

    /* Intra MB */
    if(TRUE == vc1_MBTypeIsIntra(pA->eMBType))
    {
        DEBUG0(vc1DEBUG_MV, "Candidate : Intra\n");
        return 0;
    }

    FieldMV  = vc1_MBTypeIsFieldMV(pPos->pCurMB->eMBType);
    FieldMVA = vc1_MBTypeIsFieldMV(pA->eMBType);

    if (FieldMV && FieldMVA)
    {
        /* The predictor block must be in the same field */
        BlkA = (Blk & 2) | (BlkA & 1);
    }

    /* Read motion vector */
    X = pA->sBlk[BlkA].u.sInter.sMotion[Back].sMV.X;
    Y = pA->sBlk[BlkA].u.sInter.sMotion[Back].sMV.Y;

    if (!FieldMV && FieldMVA)
    {
        /* Average the two fields */
        int SX = pA->sBlk[BlkA^2].u.sInter.sMotion[Back].sMV.X;
        int SY = pA->sBlk[BlkA^2].u.sInter.sMotion[Back].sMV.Y;
        DEBUG3(vc1DEBUG_MV, "Candidate1: MV_X = %d, MV_Y = %d (Blk = %d)\n", X, Y, BlkA);
        DEBUG3(vc1DEBUG_MV, "Candidate2: MV_X = %d, MV_Y = %d (Blk = %d)\n", SX, SY, BlkA^2);

        X = (X + SX + 1)>>1;
        Y = (Y + SY + 1)>>1;

        COVERAGE("10.7.2.4.5");
    }

    DEBUG3(vc1DEBUG_MV, "Candidate : MV_X = %d, MV_Y = %d (Blk = %d)\n", X, Y, BlkA);
    COVERAGE("10.7.2.4.1");
    pPred->X = (HWD16)X;
    pPred->Y = (HWD16)Y;

    return 1;
}

/*
 * Description:
 * Predict the X and Y motion vectors for an interlace frame block
 *
 * Remarks:
 * For a 1MV Frame MB Blk must be 0
 * For a 2MV Field MB Blk can be 0 (top field) or 2 (bottom field)
 * For a 4MV Frame MB Blk can be 0 to 3 (top left/right, bottom left/right block)
 * For a 4MV Field MB Blk can be 0 to 3 (top left/right, bottom left/right field)
 *
 * Inputs:
 * pPred        - Pointer to the predicted output of X, Y
 * pPos         - Pointer to the macroblock being predicted
 * Blk          - Block number within the macroblock
 * Back         - 0=predict forward motion 1=predict backward motion
 *
 * Outputs:
 * pPred        - predicted output values written
 *
 */

void vc1PREDMV_PredictInterlacedFrameMV(
    vc1_sMV *pPred,
    vc1_sPosition *pPos,
    int Blk,
    int Back
)
{
    vc1_sMV pCand[3];    /* predictor canidates */
    vc1_sMB *pMB = pPos->pCurMB;
    vc1_sMB *pA=NULL, *pB=NULL, *pC=NULL;
    int BlkA=0, BlkB=0, BlkC=0;
    int N, X, Y;
    int FieldMV = vc1_MBTypeIsFieldMV(pMB->eMBType);

    pPred->BottomField = 0;

    /* In this function we use the convention
     *  A=Top
     *  C=Left
     *  B=Top Right (not last MB in row)
     *  B=Top Left  (last MB in row)
     *  Picture: B A B
     *           C *
     */
    X = pPos->X;
    Y = pPos->Y;

    /* Get position of blocks A and B (top blocks) */
    if (Blk==vc1_BlkY0 || Blk==vc1_BlkY1 || FieldMV)
    {
        int WidthMB = pPos->WidthMB;

        if (Y>0)
        {
            pA   = pMB - WidthMB;
            BlkA = Blk | vc1_BlkY2;
            if (WidthMB > 1)
            {
                pB   = pA + 1;
                BlkB = vc1_BlkY2;
                if (X+1 == WidthMB)
                {
                    pB   = pA-1;
                    BlkB = vc1_BlkY3;
                }
            }
        }
    }
    else
    {
        pA   = pMB;
        BlkA = vc1_BlkY1;
        pB   = pMB;
        BlkB = vc1_BlkY0;
    }

    /* Get position of block C (left block) */
    if (Blk==vc1_BlkY0 || Blk==vc1_BlkY2)
    {
        if (X>0)
        {
            pC   = pMB-1;
            BlkC = Blk+1;
        }
    }
    else
    {
        pC   = pMB;
        BlkC = Blk-1;
    }

    /* Fill buffer with valid predictors in order left, top, top-right/left*/
    N = 0;
    N += vc1PREDMV_FrameMBPred(&pCand[N], pPos, pC, BlkC, Blk, Back);
    N += vc1PREDMV_FrameMBPred(&pCand[N], pPos, pA, BlkA, Blk, Back);
    N += vc1PREDMV_FrameMBPred(&pCand[N], pPos, pB, BlkB, Blk, Back);

    if (FieldMV)
    {
        int i, NumSameFieldMV=0, FirstSame=0, FirstOpp=0;

        for (i=N-1; i>=0; i--)
        {
            if ((pCand[i].Y & 4)==0)
            {
                NumSameFieldMV++;
                FirstSame=i;
            }
            else
            {
                FirstOpp=i;
            }
        }

        if (NumSameFieldMV < N-NumSameFieldMV)
        {
            /* Same < Opposite so use Opposite */
            FirstSame = FirstOpp;
            NumSameFieldMV = N-NumSameFieldMV;
        }

        switch (NumSameFieldMV)
        {
        case 0:
            X = 0;
            Y = 0;
            break;
        case 3:
            X = vc1TOOLS_Median3(pCand[0].X, pCand[1].X, pCand[2].X);
            Y = vc1TOOLS_Median3(pCand[0].Y, pCand[1].Y, pCand[2].Y);
            break;
        default:
            X = pCand[FirstSame].X;
            Y = pCand[FirstSame].Y;
            break;
        }
        COVERAGE("10.7.2.4.7");
    }
    else /* FrameMV */
    {
        switch (N)
        {
        case 0:
            X = 0;
            Y = 0;
            break;
        case 1:
            X = pCand->X;
            Y = pCand->Y;
            break;
        case 2:
            pCand[2].X = 0;
            pCand[2].Y = 0;
            /* Fall Through */
        case 3:
            X = vc1TOOLS_Median3(pCand[0].X, pCand[1].X, pCand[2].X);
            Y = vc1TOOLS_Median3(pCand[0].Y, pCand[1].Y, pCand[2].Y);
            break;
        }
        
        COVERAGE("10.7.2.4.6");
    }

    pPred->X = (HWD16)X;
    pPred->Y = (HWD16)Y;

    DEBUG3(vc1DEBUG_MV, "Predict MV: MV_X = %d, MV_Y = %d Back = %d\n", X, Y, Back);
}
