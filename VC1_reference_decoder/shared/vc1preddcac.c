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
 * vc1preddcac.c
 *
 * Predict DC and AC coefficients. Also holds shared tables
 * and functions relating to DC and AC quantization.
 *
 */

#include "vc1types.h"
#include "vc1debug.h"

#include "vc1pred.h"
#include "vc1preddcac.h"

/* DQScale reciprocals. Table[k]=Round((1<<18)/k). */

const WORD32 vc1PREDDCAC_DQScaleTable[64] =
{
     -1, 262144, 131072,  87381,  65536,  52429,  43691,  37449,
  32768,  29127,  26214,  23831,  21845,  20165,  18725,  17476,
  16384,  15420,  14564,  13797,  13107,  12483,  11916,  11398,
  10923,  10486,  10082,   9709,   9362,   9039,   8738,   8456,
   8192,   7944,   7710,   7490,   7282,   7085,   6899,   6722,
   6554,   6394,   6242,   6096,   5958,   5825,   5699,   5578,
   5461,   5350,   5243,   5140,   5041,   4946,   4855,   4766,
   4681,   4599,   4520,   4443,   4369,   4297,   4228,   4161
};



/*
 * Description:
 * Copy the DC, Top and Left AC coefficients from a block to the block state structure
 *
 * Remarks:
 *
 * Inputs:
 * pData - Pointer to transformed quantised coefficients
 *
 * Outputs:
 * pBlk  - Pointer to the block to update
 *
 * Return Value:
 * None
 *
 */

void vc1PREDDCAC_CopyDCAC(vc1_sBlk *pBlk, HWD16 *pData)
{
    int i;
    pBlk->u.sIntra.DC = pData[0];

    for (i=1; i<8; i++)
    {
        pBlk->u.sIntra.ACTop[i-1]  = pData[i];
        pBlk->u.sIntra.ACLeft[i-1] = pData[8*i];
    }
}


/*
 * Description:
 * Calculate DCStepSize from the quantization step as in 8.1.1.9
 *
 * Remarks:
 * This function is only called for an Intra 8x8 block.
 *
 * Inputs:
 * Quant - quantisation step
 *
 * Outputs:
 * None
 *
 * Return Value:
 * The calculated DCStepSize
 *
 */

unsigned int vc1PREDDCAC_DCStepSize(unsigned int Quant)
{
    if (Quant < 4)
    {
        return (1<<Quant);
    }
    return (Quant/2) + 6;
}

/*
 * Description:
 * Scale a DC coefficient for a different quantizer
 *
 * Inputs:
 * DC       - DC coefficient to scale
 * QuantA   - Old Quantizer (2*Quant + HalfStep)
 * Quant    - Current Quantizer (2*Quant + HalfStep)
 *
 * Returns:
 * DC scaled to current quantizer
 */

static int vc1PREDDCAC_ScaleDC(int DC, int QuantA, int Quant)
{
    /* Remove HalfStep */
    Quant  >>= 1;
    QuantA >>= 1;

    if (Quant != QuantA)
    {
        /* Convert quantizer to step size */
        Quant  = vc1PREDDCAC_DCStepSize(Quant);
        QuantA = vc1PREDDCAC_DCStepSize(QuantA);

        DC = (DC * vc1PREDDCAC_DQScaleTable[Quant] * QuantA + 0x20000)>>18;
        COVERAGE("8.1.1.15");
    }

    return DC;
}

/*
 * Description:
 * Calculate the default DC predictor for a block
 *
 * Remarks:
 * In general the default is round(8*(128-Bias)/DCStepSize)
 *
 * Inputs:
 * MQuant       - Macroblock quantizer
 * Bias         - Bias applied to DC coefficients (0 or 128)
 */

int vc1PREDDCAC_DCDefault(int MQuant, int Bias)
{
    int DCStepSize;

    if (Bias==128)
    {
        return 0;
    }

    DCStepSize = vc1PREDDCAC_DCStepSize(MQuant);

    return ((1024 + (DCStepSize>>1)) / DCStepSize);
}

/*
 * Description:
 * Determine whether the ACPRED flag is present for a macroblock
 *
 * Remarks:
 * The ACPRED flag is present if the macroblock contains an intra
 * block with an intra neighbour to the top or left.
 *
 * Inputs:
 * pPos     - Position of the current macroblock
 *
 * Returns:
 * TRUE if ACPRED is present, otherwise FALSE
 */

int vc1PREDDCAC_ACPREDPresent(vc1_sPosition *pPos)
{
    vc1_sMB *pMB = pPos->pCurMB;
    vc1_sBlk *pBlk;
    int Blk;

    /* Intra macroblocks always have ACPRED */
    if (vc1_MBTypeIsIntra(pMB->eMBType))
    {
        return TRUE;
    }

    /* 1MV or 2MV are always inter */
    if (!vc1_MBTypeIs4MV(pMB->eMBType))
    {
        return FALSE;
    }

    /* 4MV may have Intra blocks that can be predicted */
    for (Blk=0; Blk < VC1_BLOCKS_PER_MB; Blk++)
    {
        if (vc1_BlkTypeIsIntra(pMB->sBlk[Blk].eBlkType))
        {
            pBlk = vc1PRED_pTopBlk(pPos, Blk);
            if (pBlk && vc1_BlkTypeIsIntra(pBlk->eBlkType))
            {
                return TRUE;
            }
            pBlk = vc1PRED_pLeftBlk(pPos, Blk);
            if (pBlk && vc1_BlkTypeIsIntra(pBlk->eBlkType))
            {
                return TRUE;
            }
        }
    }
    return FALSE;
}

/*
 * Description:
 * Read the macroblock quantizer in the form 2*Quant + HalfStep
 *
 * Inputs:
 * pMB      - pointer to the macroblock
 *
 * Returns:
 * Quantizer in half steps
 */

static int vc1PREDDCAC_GetQuant(vc1_sMB *pMB)
{
    return 2*pMB->sQuant.Quant + pMB->sQuant.HalfStep;
}

/*
 * Description:
 * Predict the DC and AC values of a block from preceding blocks
 *
 * Remarks:
 * This function is only called for an Intra 8x8 block.
 *
 * Inputs:
 * pPred        - Pointer to buffer for predicted DC and AC values
 * pPos         - Pointer to the macroblock being predicted
 * Blk          - Block number within the macroblock
 * DCDefault    - Default DC predictor
 *                For Rule A it is 0
 *                For Rule B it is ((1024 + (DCStepSize>>1)) / DCStepSize)
 *
 * Outputs:
 * pPred        - Pointer to the predicted output of 1 DC and then 7 AC values
 *
 * Return Value:
 * vc1_BlkIntra      - No AC values predicted     (use normal zigzag table)
 * vc1_BlkIntraLeft  - Predicted left 7 AC values (use left zigzag table)
 * vc1_BlkIntraTop   - Predicted top  7 AC values (use top zigzag table)
 *
 */

vc1_eBlkType vc1PREDDCAC_PredictDCAC(
    HWD16 *pPred,
    vc1_sPosition *pPos,
    int Blk,
    int DCDefault)
{
    vc1_sMB *pMB = pPos->pCurMB;
    vc1_sBlk *pA=NULL, *pB=NULL, *pC=NULL;
    vc1_eBlkType eDirection = vc1_BlkIntra;
    int Quant, QuantA, QuantB, QuantC;
    int i;
    int X = pPos->X;
    int Y = pPos->Y;
    int WidthMB = pPos->WidthMB;
    int DCP = DCDefault;    /* predicted DC */

    COVERAGE("8.1.1.8");

    Quant = vc1PREDDCAC_GetQuant(pMB);
    QuantA = Quant;
    QuantB = Quant;
    QuantC = Quant;

    /* This code uses the naming convention:
     * A = top block
     * B = top left block
     * C = left block
     */
    switch (Blk)
    {
    case vc1_BlkY0:
        if (Y>0)
        {
            pA = &pMB[-WidthMB].sBlk[2];
            QuantA = vc1PREDDCAC_GetQuant(pMB - WidthMB);
            if (X>0)
            {
                pB = &pMB[-WidthMB-1].sBlk[3];
                QuantB = vc1PREDDCAC_GetQuant(pMB - WidthMB -1);
            }
        }
        if (X>0)
        {
            pC = &pMB[-1].sBlk[1];
            QuantC = vc1PREDDCAC_GetQuant(pMB - 1);
        }
        break;

    case vc1_BlkY1:
        if (Y>0)
        {
            pA = &pMB[-WidthMB].sBlk[3];
            QuantA = vc1PREDDCAC_GetQuant(pMB - WidthMB);
            pB = &pMB[-WidthMB].sBlk[2];
            QuantB = QuantA;
        }
        pC = &pMB->sBlk[0];
        break;

    case vc1_BlkY2:
        pA = &pMB->sBlk[0];
        if (X>0)
        {
            pB = &pMB[-1].sBlk[1];
            QuantB = vc1PREDDCAC_GetQuant(pMB - 1);
            pC = &pMB[-1].sBlk[3];
            QuantC = QuantB;
        }
        break;

    case vc1_BlkY3:
        pA = &pMB->sBlk[1];
        pB = &pMB->sBlk[0];
        pC = &pMB->sBlk[2];
        break;

    case vc1_BlkCb:
    case vc1_BlkCr:
        if (Y>0)
        {
            pA = &pMB[-WidthMB].sBlk[Blk];
            QuantA = vc1PREDDCAC_GetQuant(pMB - WidthMB);
            if (X>0)
            {
                pB = &pMB[-WidthMB-1].sBlk[Blk];
                QuantB = vc1PREDDCAC_GetQuant(pMB - WidthMB - 1);
            }
        }
        if (X>0)
        {
            pC = &pMB[-1].sBlk[Blk];
            QuantC = vc1PREDDCAC_GetQuant(pMB - 1);
        }
        break;
    }

    /* Decide prediction direction */
    if (pA && vc1_BlkTypeIsIntra(pA->eBlkType))
    {
        int DCA = vc1PREDDCAC_ScaleDC(pA->u.sIntra.DC, QuantA, Quant);
         
        if (pC && vc1_BlkTypeIsIntra(pC->eBlkType))
        {
            /* Both top and left blocks exist and are intra */
            int D1, D2;
            int DCB = DCDefault;
            int DCC = vc1PREDDCAC_ScaleDC(pC->u.sIntra.DC, QuantC, Quant);

            if (vc1_BlkTypeIsIntra(pB->eBlkType))
            {
                DCB = vc1PREDDCAC_ScaleDC(pB->u.sIntra.DC, QuantB, Quant);
            }

            D1 = ABS(DCB - DCA);
            D2 = ABS(DCB - DCC);

            DEBUG3(vc1DEBUG_DCAC, "DCA=%04x DCB=%04x DCC=%04x\n", DCA, DCB, DCC);

            if (D1 <= D2)
            {
                eDirection = vc1_BlkIntraLeft;
                DCP = DCC;
            }
            else /* D2 < D1 */
            {
                eDirection = vc1_BlkIntraTop;
                DCP = DCA;
            }
        }
        else
        {
            /* Intra above, but no intra to the left */
            eDirection = vc1_BlkIntraTop;
            DCP = DCA;
            if (pPos->eProfile!=vc1_ProfileAdvanced && vc1_PictureTypeIsIntra(pPos->ePictureType))
            {
                /* I/BI pictures predict DC left if Top==Default
                 * Therefore there is no AC prediction from as
                 * the left block is off the image. However, we
                 * use the left zig-zag table
                 */
                if (DCA == DCDefault)
                {
                    /* No AC prediction */
                    eDirection = vc1_BlkIntra;
                }
            }
        }
    }
    else
    {
        if (pC && vc1_BlkTypeIsIntra(pC->eBlkType))
        {
            /* Intra to the left but no intra above */
            eDirection = vc1_BlkIntraLeft;
            DCP = vc1PREDDCAC_ScaleDC(pC->u.sIntra.DC, QuantC, Quant);
        }
    }

    pPred[0] = (HWD16)DCP;
    DEBUG1(vc1DEBUG_DCAC, "Pred DC direction %s\n", vc1DEBUG_BlkType[eDirection]);

    /* Perform prediction */
    switch (eDirection)
    {
    case vc1_BlkIntraLeft:
        /* Left block exists and is intra */
        if (pMB->eACPred == vc1_ACPredOn)
        {
            WORD32 Scale = vc1PREDDCAC_DQScaleTable[Quant-1] * (QuantC-1);

            /* Predict AC */
            for (i=1; i<8; i++)
            {
                pPred[i] = (HWD16)((pC->u.sIntra.ACLeft[i-1] * Scale + 0x20000)>>18);
            }
        }
        else
        {
            /* No AC Prediction */
            eDirection = vc1_BlkIntra;
        }
        break;

    case vc1_BlkIntraTop:
        /* Top block exists and is intra */
        if (pMB->eACPred == vc1_ACPredOn)
        {
            WORD32 Scale = vc1PREDDCAC_DQScaleTable[Quant-1] * (QuantA-1);

            /* Predict AC */
            for (i=1; i<8; i++)
            {
                pPred[i] = (HWD16)((pA->u.sIntra.ACTop[i-1] * Scale + 0x20000)>>18);
            }
        }
        else
        {
            /* No AC Prediction */
            eDirection = vc1_BlkIntra;
        }
        break;

    default:
        /* No intra block to AC predict from */
        if (pMB->eACPred == vc1_ACPredOn)
        {
            /* If AC prediction is zero but
             * AC prediction is on, then prediction direction
             * is left (affects the zig-zag table used)
             */
            for (i=1; i<8; i++)
            {
                pPred[i] = 0;
            }
            eDirection = vc1_BlkIntraLeft;
        }
		/* fix on 01/28/05 */
        /* change to match interlace frame zigzag scan selection */
        if (pPos->ePictureFormat == vc1_InterlacedFrame) {
            eDirection = vc1_BlkIntra;
        } 
 
        break;
    }

    return eDirection;
}
