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
 *
 * vc1smooth.c:
 * Post inverse transform overlap smooth functions
 *
 */

#include "vc1types.h"
#include "vc1debug.h"
#include "vc1smooth.h"
#include "vc1tools.h"
#include "vc1pred.h"

#include <string.h> /* memcpy */

/*
 * Description:
 * Horizontally overlap smooth the top edge of a block
 *
 * Inputs:
 * pC       - Position in image of smoothing
 * pA       - Pointer to block above the current block
 * pBlk     - raw 10-bit data for current block
 *
 * Outputs:
 * All updated for horizontal smoothing
 */

static void vc1SMOOTH_OverlapSmoothHorizBlk(
    vc1_sComponent *pC,
    vc1_sBlk *pA,
    HWD16 pBlk[64]
)
{
    UBYTE8 *pData = pC->pData;
    HWD16 *pTop = pA->u.sIntra.SmoothRows;
    int Bpl = pC->Bpl;
    int r0 = 4, r1 = 3;
    int X;

    for (X = 0; X < 8; X++)
    {
        int x0, x1, x2, x3;
        int y0, y1, y2, y3;
      
        x0 = pTop[X];
        x1 = pTop[X+8];
        x2 = pBlk[X];
        x3 = pBlk[X+8];

        y0 = (7*x0               +   x3 + r0) >> 3;
        y1 = ( -x0 + 7*x1 +   x2 +   x3 + r1) >> 3;
        y2 = (  x0 +   x1 + 7*x2 -   x3 + r0) >> 3;
        y3 = (  x0               + 7*x3 + r1) >> 3;

        /* clamp pixels going back into the image buffer */
        y0 = CLIP(y0 + 128);
        y1 = CLIP(y1 + 128);
        y2 = CLIP(y2 + 128);
        y3 = CLIP(y3 + 128);

        pData[-2*Bpl] = (UBYTE8)y0;
        pData[-Bpl]   = (UBYTE8)y1;
        pData[0]      = (UBYTE8)y2;
        pData[Bpl]    = (UBYTE8)y3;

        pData ++;

        /* switch values of r0 and r1 */
        r0 = 7 - r0;
        r1 = 7 - r1;
    }
}

/*
 * Description:
 * Vetically overlap smooth the left edge of a block
 *
 * Inputs:
 * pC       - Position in image of smoothing
 * pLeft    - raw 10-bit data for block to the left of current block
 * pBlk     - raw 10-bit data for current block
 *
 * Outputs:
 * All updated for vertical smoothing
 */

static void vc1SMOOTH_OverlapSmoothVertBlk(
    vc1_sComponent *pC,
    HWD16 pLeft[64],
    HWD16 pBlk[64]
)
{
    UBYTE8 *pData = pC->pData;
    int Bpl = pC->Bpl;
    int r0 = 4, r1 = 3;
    int Y;

    for (Y = 0; Y < 8; Y++)
    {
        int x0, x1, x2, x3;
        int y0, y1, y2, y3;
      
        x0 = pLeft[8*Y + 6];
        x1 = pLeft[8*Y + 7];
        x2 = pBlk [8*Y + 0];
        x3 = pBlk [8*Y + 1];

        y0 = (7*x0               +   x3 + r0) >> 3;
        y1 = ( -x0 + 7*x1 +   x2 +   x3 + r1) >> 3;
        y2 = (  x0 +   x1 + 7*x2 -   x3 + r0) >> 3;
        y3 = (  x0               + 7*x3 + r1) >> 3;

        pLeft[8*Y + 6] = (HWD16)y0;
        pLeft[8*Y + 7] = (HWD16)y1;
        pBlk [8*Y + 0] = (HWD16)y2;
        pBlk [8*Y + 1] = (HWD16)y3;

        /* clamp pixels going back into the image buffer */
        y0 = CLIP(y0 + 128);
        y1 = CLIP(y1 + 128);

        pData[-2] = (UBYTE8)y0;
        pData[-1] = (UBYTE8)y1;

        pData += Bpl;

        /* switch values of r0 and r1 */
        r0 = 7 - r0;
        r1 = 7 - r1;
    }
}
    
/* 
 * Description:
 * Horizontal overlap smooth the top edges of blocks in the current MB
 *
 * Remarks:
 * This function is only called when OverlapFilter is enabled for the macroblock
 * Copies the bottom block rows into the macroblock state
 * Then filters over horizontal edges as required
 * 
 *
 * Inputs:
 * pRefPic      - Pointer to output reference picture
 * pPosition    - Pointer to position of current macroblock
 *
 * Outputs:
 * pRefPic      - Updated with smoothed pixels
 */

void vc1SMOOTH_OverlapSmoothHorizMB(
    vc1_sReferencePicture * pRefPic,
    vc1_sPosition *pPosition
)
{
    vc1_sComponent sC;
    vc1_sMB *pMB  = pPosition->pCurMB;
    vc1_sMB *pA;
    HWD16 (*pData)[64];
    int Blk;

    if(vc1_InterlacedFrame == pPosition->ePictureFormat)
    {
        COVERAGE("10.9.2");
        return;
    }

    pData = pPosition->pSmooth;

    /* First save bottom rows in state */
    for (Blk=0; Blk<VC1_BLOCKS_PER_MB; Blk++)
    {
        vc1_sBlk *pBlk = &pMB->sBlk[Blk];

        if (vc1_BlkTypeIsIntra(pBlk->eBlkType))
        {
            memcpy(pBlk->u.sIntra.SmoothRows, &pData[Blk][6*8], 16*sizeof(HWD16));
        }
    }
    
    /* Find the macroblock above this macroblock */
    pA = vc1PRED_pTopMB(pPosition);

    if (pA && pA->OverlapFilter)
    {
        /* Filter top edges of this macroblock */
        if (vc1_BlkTypeIsIntra(pA->sBlk[2].eBlkType) && vc1_BlkTypeIsIntra(pMB->sBlk[0].eBlkType))
        {
            DEBUG0(vc1DEBUG_SMOOTH, "Block Y0 top edge\n");
            vc1TOOLS_GetPictureDestination(&sC, pRefPic, pPosition, vc1_BlkY0);
            vc1SMOOTH_OverlapSmoothHorizBlk(&sC, &pA->sBlk[2], pData[0]);
        }
        if (vc1_BlkTypeIsIntra(pA->sBlk[3].eBlkType) && vc1_BlkTypeIsIntra(pMB->sBlk[1].eBlkType))
        {
            DEBUG0(vc1DEBUG_SMOOTH, "Block Y1 top edge\n");
            vc1TOOLS_GetPictureDestination(&sC, pRefPic, pPosition, vc1_BlkY1);
            vc1SMOOTH_OverlapSmoothHorizBlk(&sC, &pA->sBlk[3], pData[1]);
        }
        if (vc1_BlkTypeIsIntra(pA->sBlk[4].eBlkType) && vc1_BlkTypeIsIntra(pMB->sBlk[4].eBlkType))
        {
            DEBUG0(vc1DEBUG_SMOOTH, "Block Cb top edge\n");
            vc1TOOLS_GetPictureDestination(&sC, pRefPic, pPosition, vc1_BlkCb);
            vc1SMOOTH_OverlapSmoothHorizBlk(&sC, &pA->sBlk[4], pData[4]);
        }
        if (vc1_BlkTypeIsIntra(pA->sBlk[5].eBlkType) && vc1_BlkTypeIsIntra(pMB->sBlk[5].eBlkType))
        {
            DEBUG0(vc1DEBUG_SMOOTH, "Block Cr top edge\n");
            vc1TOOLS_GetPictureDestination(&sC, pRefPic, pPosition, vc1_BlkCr);
            vc1SMOOTH_OverlapSmoothHorizBlk(&sC, &pA->sBlk[5], pData[5]);
        }
    }

    /* Filter internal top edges in this macroblock */

    if (vc1_BlkTypeIsIntra(pMB->sBlk[0].eBlkType) && vc1_BlkTypeIsIntra(pMB->sBlk[2].eBlkType))
    {
        DEBUG0(vc1DEBUG_SMOOTH, "Block Y2 top edge\n");
        vc1TOOLS_GetPictureDestination(&sC, pRefPic, pPosition, vc1_BlkY2);
        vc1SMOOTH_OverlapSmoothHorizBlk(&sC, &pMB->sBlk[0], pData[2]);
    }

    if (vc1_BlkTypeIsIntra(pMB->sBlk[1].eBlkType) && vc1_BlkTypeIsIntra(pMB->sBlk[3].eBlkType))
    {
        DEBUG0(vc1DEBUG_SMOOTH, "Block Y3 top edge\n");
        vc1TOOLS_GetPictureDestination(&sC, pRefPic, pPosition, vc1_BlkY3);
        vc1SMOOTH_OverlapSmoothHorizBlk(&sC, &pMB->sBlk[1], pData[3]);
    }
}

/* 
 * Description:
 * Perform overlap smoothing on this and previous macroblock
 *
 * Remarks:
 * You must always call this function even if overlap smoothing
 * is off for the current macroblock as it may be on for the
 * previous macroblock.
 * If overlap smoothing is on for the current macroblock it smooths
 * left vertical edges.
 * If overlap smoothing is on for the previous macroblock it smooths
 * top horizontal edges.
 *
 * Inputs:
 * pRefPic      - Pointer to output reference picture
 * pPosition    - Pointer to position of current macroblock
 * pData        - Coefficients for current macroblock
 *
 * Outputs:
 * pRefPic      - Updated with smoothed pixels
 */


void vc1SMOOTH_OverlapSmoothMB(
    vc1_sReferencePicture * pRefPic,
    vc1_sPosition *pPosition,
    HWD16 pData[6][64]
)
{
    vc1_sComponent sC;
    vc1_sMB *pMB  = pPosition->pCurMB;
    vc1_sMB *pA;

    COVERAGE("8.5.1");

    pA = vc1PRED_pLeftMB(pPosition);
    if (pA && pA->OverlapFilter==FALSE)
    {
        pA = NULL;
    }

    if (pA && pMB->OverlapFilter)
    {
        /* Smooth left edges between current and left macroblocks */
        if (vc1_BlkTypeIsIntra(pA->sBlk[1].eBlkType) && vc1_BlkTypeIsIntra(pMB->sBlk[0].eBlkType))
        {
            DEBUG0(vc1DEBUG_SMOOTH, "Block Y0 left edge\n");
            vc1TOOLS_GetPictureDestination(&sC, pRefPic, pPosition, vc1_BlkY0);
            vc1SMOOTH_OverlapSmoothVertBlk(&sC, pPosition->pSmooth[1], pData[0]);
        }
        if (vc1_BlkTypeIsIntra(pA->sBlk[3].eBlkType) && vc1_BlkTypeIsIntra(pMB->sBlk[2].eBlkType))
        {
            DEBUG0(vc1DEBUG_SMOOTH, "Block Y2 left edge\n");
            vc1TOOLS_GetPictureDestination(&sC, pRefPic, pPosition, vc1_BlkY2);
            vc1SMOOTH_OverlapSmoothVertBlk(&sC, pPosition->pSmooth[3], pData[2]);
        }
        if (vc1_BlkTypeIsIntra(pA->sBlk[4].eBlkType) && vc1_BlkTypeIsIntra(pMB->sBlk[4].eBlkType))
        {
            DEBUG0(vc1DEBUG_SMOOTH, "Block Cb left edge\n");
            vc1TOOLS_GetPictureDestination(&sC, pRefPic, pPosition, vc1_BlkCb);
            vc1SMOOTH_OverlapSmoothVertBlk(&sC, pPosition->pSmooth[4], pData[4]);
        }
        if (vc1_BlkTypeIsIntra(pA->sBlk[5].eBlkType) && vc1_BlkTypeIsIntra(pMB->sBlk[5].eBlkType))
        {
            DEBUG0(vc1DEBUG_SMOOTH, "Block Cr left edge\n");
            vc1TOOLS_GetPictureDestination(&sC, pRefPic, pPosition, vc1_BlkCr);
            vc1SMOOTH_OverlapSmoothVertBlk(&sC, pPosition->pSmooth[5], pData[5]);
        }
    }

    if (pMB->OverlapFilter)
    {
        /* Smooth internal left edges of current macroblock */
        if (vc1_BlkTypeIsIntra(pMB->sBlk[0].eBlkType) && vc1_BlkTypeIsIntra(pMB->sBlk[1].eBlkType))
        {
            DEBUG0(vc1DEBUG_SMOOTH, "Block Y1 left edge\n");
            vc1TOOLS_GetPictureDestination(&sC, pRefPic, pPosition, vc1_BlkY1);
            vc1SMOOTH_OverlapSmoothVertBlk(&sC, pData[0], pData[1]);
        }

        if (vc1_BlkTypeIsIntra(pMB->sBlk[2].eBlkType) && vc1_BlkTypeIsIntra(pMB->sBlk[3].eBlkType))
        {
            DEBUG0(vc1DEBUG_SMOOTH, "Block Y3 left edge\n");
            vc1TOOLS_GetPictureDestination(&sC, pRefPic, pPosition, vc1_BlkY3);
            vc1SMOOTH_OverlapSmoothVertBlk(&sC, pData[2], pData[3]);
        }
    }

    if (pA)
    {
        /* Smooth the horizontal top edges for the left macroblock */
        pPosition->X--;
        pPosition->pCurMB--;
        vc1SMOOTH_OverlapSmoothHorizMB(pRefPic, pPosition);
        pPosition->X++;
        pPosition->pCurMB++;
    }

    if (pMB->OverlapFilter)
    {
        /* Copy current coefficients into history buffer */
        memcpy(&pPosition->pSmooth[0][0], &pData[0][0], 6*64*sizeof(HWD16));
    }
}
