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
 * vc1pred.c
 *
 * Shared routines for use by prediction functions
 *
 *  
 */

#include "vc1types.h"
#include "vc1pred.h"
#include "vc1debug.h"

/*
 * Description:
 * Find the macroblock to the left of the current macroblock
 *
 * Inputs:
 *      pPos    - pointer to position of current macroblock
 *
 * Returns:
 *      Pointer to the left macroblock or NULL if the macroblock
 *      is in the first column
 */

vc1_sMB *vc1PRED_pLeftMB(vc1_sPosition *pPos)
{
    vc1_sMB *pMB = pPos->pCurMB - 1;

    if (pPos->X == 0)
    {
        return NULL;
    }

    if (pMB < pPos->pStartMB)
    {
        pMB += pPos->SizeMB;
    }

    return pMB;
}

/*
 * Description:
 * Find the macroblock to the top of the current macroblock
 *
 * Inputs:
 *      pPos    - pointer to position of current macroblock
 *
 * Returns:
 *      Pointer to the top macroblock or NULL if the macroblock
 *      is in the first slice row
 */

vc1_sMB *vc1PRED_pTopMB(vc1_sPosition *pPos)
{
    vc1_sMB *pMB = pPos->pCurMB - pPos->WidthMB;

    if (pPos->Y == 0)
    {
        return NULL;
    }

    if (pMB < pPos->pStartMB)
    {
        pMB += pPos->SizeMB;
    }

    return pMB;
}

/*
 * Description:
 * Find the macroblock to the top left of the current macroblock
 *
 * Inputs:
 *      pPos    - pointer to position of current macroblock
 *
 * Returns:
 *      Pointer to the top left macroblock or NULL if the macroblock
 *      is in the first slice row
 */

vc1_sMB *vc1PRED_pTopLeftMB(vc1_sPosition *pPos)
{
    vc1_sMB *pMB = pPos->pCurMB - pPos->WidthMB - 1;

    if (pPos->X==0 || pPos->Y == 0)
    {
        return NULL;
    }

    if (pMB < pPos->pStartMB)
    {
        pMB += pPos->SizeMB;
    }

    return pMB;
}

/*
 * Description:
 * Find the block to the left of the current block
 *
 * Inputs:
 *      pPos    - pointer to position of current macroblock
 *      Blk     - Block within the macroblock
 *
 * Returns:
 *      Pointer to the left macroblock or NULL if the block
 *      is in the first column
 */

vc1_sBlk *vc1PRED_pLeftBlk(vc1_sPosition *pPos, int Blk)
{
    const UBYTE8 pLeftBlkNum[6] = { 1, 0, 3, 2, 4, 5 };
    vc1_sMB *pMB = pPos->pCurMB;

    Blk = pLeftBlkNum[Blk];

    if (Blk!=vc1_BlkY0 && Blk!=vc1_BlkY2)
    {
        if (pPos->X == 0)
        {
            return NULL;
        }
        pMB --;
        if (pMB < pPos->pStartMB)
        {
            pMB += pPos->SizeMB;
        }
    }

    return &pMB->sBlk[Blk];
}

/*
 * Description:
 * Find the block to the top of the current block
 *
 * Inputs:
 *      pPos    - pointer to position of current macroblock
 *      Blk     - Block within the macroblock
 *
 * Returns:
 *      Pointer to the top macroblock or NULL if the block
 *      is in the first row
 */

vc1_sBlk *vc1PRED_pTopBlk(vc1_sPosition *pPos, int Blk)
{
    const UBYTE8 pTopBlkNum[6] = { 2, 3, 0, 1, 4, 5 };
    vc1_sMB *pMB = pPos->pCurMB;

    Blk = pTopBlkNum[Blk];

    if (Blk >= vc1_BlkY2)
    {
        if (pPos->Y == 0)
        {
            return NULL;
        }
        pMB -= pPos->WidthMB;
        if (pMB < pPos->pStartMB)
        {
            pMB += pPos->SizeMB;
        }
    }

    return &pMB->sBlk[Blk];
}

/*
 * Description:
 * Find the block to the top left of the current block
 *
 * Inputs:
 *      pPos    - pointer to position of current macroblock
 *      Blk     - Block within the macroblock
 *
 * Returns:
 *      Pointer to the top left macroblock or NULL if the block
 *      is in the first row or first column
 */

vc1_sBlk *vc1PRED_pTopLeftBlk(vc1_sPosition *pPos, int Blk)
{
    const UBYTE8 pTopLeftBlkNum[6] = { 3, 2, 1, 0, 4, 5 };
    vc1_sMB *pMB = pPos->pCurMB;

    Blk = pTopLeftBlkNum[Blk];

    if (Blk >= vc1_BlkY2)
    {
        if (pPos->Y == 0)
        {
            return NULL;
        }
        pMB -= pPos->WidthMB;
    }

    if (Blk!=vc1_BlkY0 && Blk!=vc1_BlkY2)
    {
        if (pPos->X == 0)
        {
            return NULL;
        }
        pMB --;
    }

    if (pMB < pPos->pStartMB)
    {
        pMB += pPos->SizeMB;
    }

    return &pMB->sBlk[Blk];
}

/*
 * Description:
 * Find the block for predictor B in 4MV prediction
 *
 * Remarks:
 * This is often the top right block but not always.
 *
 * Inputs:
 *      pPos    - pointer to position of current macroblock
 *      Blk     - Block within the macroblock
 *
 * Returns:
 *      Pointer to the B motion predictor block or 
 *      NULL if the block is in the first row
 */

vc1_sBlk *vc1PRED_pB4MVBlk(vc1_sPosition *pPos, int Blk)
{
    vc1_sMB *pMB = pPos->pCurMB;

    switch (Blk)
    {
    case vc1_BlkY0:
        if (pPos->Y == 0)
        {
            return NULL;
        }
        pMB -= pPos->WidthMB;
        Blk  = vc1_BlkY3;
        if (pPos->X > 0)
        {
            pMB--;
        }
        if (pMB < pPos->pStartMB)
        {
            pMB += pPos->SizeMB;
        }
        break;

    case vc1_BlkY1:
        if (pPos->Y == 0)
        {
            return NULL;
        }
        pMB -= pPos->WidthMB;
        Blk  = vc1_BlkY2;
        if (pPos->X + 1 < pPos->WidthMB)
        {
            pMB++;
        }
        if (pMB < pPos->pStartMB)
        {
            pMB += pPos->SizeMB;
        }
        break;

    case vc1_BlkY2:
        Blk = vc1_BlkY1;
        break;

    case vc1_BlkY3:
        Blk = vc1_BlkY0;
        break;
    }

    return &pMB->sBlk[Blk];
}

/*
 * Description:
 * Find the block for predictor B in 1MV prediction
 *
 * Remarks:
 * This is often the top right block but not always.
 *
 * Inputs:
 *      pPos    - pointer to position of the 1MV macroblock
 *
 * Returns:
 *      Pointer to the B motion predictor block or 
 *      NULL if the block is in the first row
 */

vc1_sBlk *vc1PRED_pB1MVBlk(vc1_sPosition *pPos)
{
    vc1_sMB *pMB = pPos->pCurMB;
    int WidthMB  = pPos->WidthMB;
    int Blk;

    if (pPos->Y==0 || WidthMB<=1)
    {
        return NULL;
    }

    pMB  = pMB - WidthMB + 1;
    Blk  = vc1_BlkY2;
    if (pPos->X + 1 == WidthMB)
    {
        pMB -= 2;
        if(! (vc1_InterlacedField == pPos->ePictureFormat))
        {
            Blk = vc1_BlkY3;
        }
    }

    if (pMB < pPos->pStartMB)
    {
        pMB += pPos->SizeMB;
    }

    return &pMB->sBlk[Blk];
}
