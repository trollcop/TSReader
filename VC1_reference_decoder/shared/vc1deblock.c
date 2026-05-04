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
 * vc1deblock.c
 *
 * In-loop deblocking functions
 *
 *  
 */

#include "vc1types.h"
#include "vc1debug.h"
#include "vc1tools.h"
#include "vc1deblock.h"


typedef enum
{
    vc1DEBLOCK_ProgressiveI,    /* Progressive I, B, BI picture */
    vc1DEBLOCK_MainP,           /* Main profile P picture - first blk not inta */
    vc1DEBLOCK_MainPI,          /* Main profile P picture - first blk intra */
    vc1DEBLOCK_AdvancedP,       /* Advanced profile progressive/field P picture */
    vc1DEBLOCK_Frame,           /* InterlacedFrame, frame transformed macroblock */
    vc1DEBLOCK_FrameFirst,      /* InterlacedFrame, frame transformed first row/column */
    vc1DEBLOCK_FrameField,      /* InterlacedFrame, field transformed macroblock */
    vc1DEBLOCK_FieldB           /* Field B picture deblock */
} vc1DEBLOCK_eDeBlkType;

/*
 * Description:
 * Apply the deblock filter to four edge pixels
 *
 * Remarks:
 * This is written with horizontal filtering in mind but it can
 * also perform vertical filtering by swapping the Skip and Bpl
 * arguments
 *
 * Inputs:
 * pData    - pointer to first pixel to deblock
 * PQuant   - quantizer
 * Skip     - skip between each pixel to deblock
 * Bpl      - bytes between each pixel in the 8 tap filter
 */

static void vc1DEBLOCK_Deblock(UBYTE8 *pData, int PQuant, int Skip, int Bpl)
{
    int FilterOther3Pixels = TRUE;
    UBYTE8 *pPixel;
    int P1, P2, P3, P4, P5, P6, P7, P8;
    int A0, A1, A2, A3;
    int Clip, D;
    int X;

    COVERAGE("8.6.4");

    pPixel = pData + 2 * Skip;

    P1 = pPixel[-3*Bpl];
    P2 = pPixel[-2*Bpl];
    P3 = pPixel[-1*Bpl];
    P4 = pPixel[ 0*Bpl];
    P5 = pPixel[ 1*Bpl];
    P6 = pPixel[ 2*Bpl];
    P7 = pPixel[ 3*Bpl];
    P8 = pPixel[ 4*Bpl];

    A0 = (2 * (P3 - P6) - 5 * (P4 - P5) + 4) >> 3;
    
    if (ABS(A0) < PQuant)
    {
        A1 = (2 * (P1 - P4) - 5 * (P2 - P3) + 4) >> 3;
        A2 = (2 * (P5 - P8) - 5 * (P6 - P7) + 4) >> 3;
        A3 = MIN(ABS(A1), ABS(A2));

        if (A3 < ABS(A0))
        {
            D = 5 * ((SIGN(A0) * A3) - A0) / 8;
            Clip = (P4 - P5) / 2;

            if (Clip == 0)
            {
                FilterOther3Pixels = FALSE;
            }
            else
            {
                if (Clip > 0)
                {
                    if (D < 0)
                    {
                        D = 0;
                    }
                    if (D > Clip)
                    {
                        D = Clip;
                    }
                }
                else
                {
                    if(D > 0)
                    {
                        D = 0;
                    }
                    if (D < Clip)
                    {
                        D = Clip;
                    }
                }
                pPixel[ 0*Bpl] = (UBYTE8)(P4 - D);
                pPixel[ 1*Bpl] = (UBYTE8)(P5 + D);
            }
        }
        else
        {
            FilterOther3Pixels = FALSE;
        }
    }
    else
    {
        FilterOther3Pixels = FALSE;
    }

    DEBUG1(vc1DEBUG_DEBLK, "FilterOther3Pixels=%d\n", FilterOther3Pixels);

    if (FilterOther3Pixels == FALSE)
    {
        return;
    }

    for (X=0; X<4; X++)
    {
        if (X==2)
        {
            /* Already filtered this pixel */
            continue;
        }

        pPixel = pData + X*Skip;
        P1 = pPixel[-3*Bpl];
        P2 = pPixel[-2*Bpl];
        P3 = pPixel[-1*Bpl];
        P4 = pPixel[ 0*Bpl];
        P5 = pPixel[ 1*Bpl];
        P6 = pPixel[ 2*Bpl];
        P7 = pPixel[ 3*Bpl];
        P8 = pPixel[ 4*Bpl];

        A0 = (2 * (P3 - P6) - 5 * (P4 - P5) + 4) >> 3;
    
        if (ABS(A0) < PQuant)
        {
            A1 = (2 * (P1 - P4) - 5 * (P2 - P3) + 4) >> 3;
            A2 = (2 * (P5 - P8) - 5 * (P6 - P7) + 4) >> 3;
            A3 = MIN(ABS(A1), ABS(A2));

            if (A3 < ABS(A0))
            {
                D = 5 * ((SIGN(A0) * A3) - A0) / 8;
                Clip = (P4 - P5) / 2;
                if (Clip > 0)
                {
                    if (D < 0)
                    {
                        D = 0;
                    }
                    if (D > Clip)
                    {
                        D = Clip;
                    }
                }
                else if (Clip < 0)
                {
                    if(D > 0)
                    {
                        D = 0;
                    }
                    if (D < Clip)
                    {
                        D = Clip;
                    }
                }
                if (Clip != 0)
                {
                    pPixel[ 0*Bpl] = (UBYTE8)(P4 - D);
                    pPixel[ 1*Bpl] = (UBYTE8)(P5 + D);
                }
            }
        }
    } /* X loop */
}

/*
 * Description:
 * Apply the horizontal deblock filter to four edge pixels
 *
 * Inputs:
 * pData    - pointer to first pixel to deblock
 * PQuant   - quantizer
 * Bpl      - bytes per line
 */

static void vc1DEBLOCK_HorizDeblock4(UBYTE8 *pData, int PQuant, int Bpl)
{
    vc1DEBLOCK_Deblock(pData, PQuant, 1, Bpl);
}

/*
 * Description:
 * Apply the vertical deblock filter to four edge pixels
 *
 * Inputs:
 * pData    - pointer to first pixel to deblock
 * PQuant   - quantizer
 * Bpl      - bytes per line
 */

static void vc1DEBLOCK_VertDeblock4(UBYTE8 *pData, int PQuant, int Bpl)
{
    vc1DEBLOCK_Deblock(pData, PQuant, Bpl, 1);
}

/*
 * Description:
 * Return the coded sublock pattern for a block
 *
 * Inputs:
 * pBlk     - Pointer to the block
 *
 * Returns:
 * bit3 = top-left    4x4 block coded
 * bit2 = top-right   4x4 block coded
 * bit1 = bottom-left 4x4 block coded
 * bit0 = bottom-right 4x4 block coded
 */

static int vc1DEBLOCK_SubBlockPattern(vc1_sBlk *pBlk)
{
    vc1_eBlkType eBlkType = pBlk->eBlkType;
    UHWD16 *pNZC = pBlk->u.sInter.NZC;
    int i, SBP = 0;

    if (pBlk->Coded==0)
    {
        return 0;
    }

    switch (eBlkType)
    {
    case vc1_BlkInter4x4:
        for (i=0; i<4; i++)
        {
            SBP = (pNZC[i]!=0) | (SBP<<1);
        }
        break;

    case vc1_BlkInter8x4:
        if (pNZC[0])
        {
            SBP |= 0xC;
        }
        if (pNZC[1])
        {
            SBP |= 0x3;
        }
        break;

    case vc1_BlkInter4x8:
        if (pNZC[0])
        {
            SBP |= 0xA;
        }
        if (pNZC[1])
        {
            SBP |= 0x5;
        }
        break;

    default:
        SBP = 0xF;
    }

    return SBP;
}

/*
 * Description:
 * Deblock the horizontal the bottom + internal edges of a block
 *
 * Remarks:
 * For progressive P pictures the block boundary must be deblocked
 * before the internal subblock boundaries. Therefore we must deblock
 * the bottom edge of a block before the internal subblock boundary.
 * This is why this routine concentrates on bottom edges rather than
 * top edges as in overlap smoothing.
 *
 * Inputs:
 * pC           - pointer to start of block component
 * pBlk         - pointer to block to deblock
 * pD           - pointer to block below/NULL if not present
 * PQuant       - quantizer
 * Blk          - block number of the block
 * eDeBlkType   - type of deblock to apply
 *
 * Outputs:
 * pC           - horizontal edges deblocked
 */

static void vc1DEBLOCK_HorizDeblockBlk(
    vc1_sComponent *pC, 
    vc1_sBlk *pBlk,
    vc1_sBlk *pD,
    int PQuant,
    int Blk,
    vc1DEBLOCK_eDeBlkType eDeBlkType)
{
    vc1_eBlkType eBlkType = pBlk->eBlkType;
    UBYTE8 *pData = pC->pData;
    int Bpl = pC->Bpl;
    int SBP;

    DEBUG4(vc1DEBUG_DEBLK, "Horizontal deblock Blk=%d BlkType=%s DeBlkType=%s (%d)\n",
        Blk, vc1DEBUG_BlkType[eBlkType], vc1DEBUG_DeBlkType[eDeBlkType], eDeBlkType);

    switch (eDeBlkType)
    {
    case vc1DEBLOCK_ProgressiveI:   /* I or B or BI */

        /* Deblock all 8x8 boundaries */
        if (pD)
        {
            DEBUG0(vc1DEBUG_DEBLK, "Horiztonal deblock I/B/BI bottom edge\n");
            vc1DEBLOCK_HorizDeblock4(pData + 7*Bpl, PQuant, Bpl);
            vc1DEBLOCK_HorizDeblock4(pData + 7*Bpl + 4, PQuant, Bpl);
        }
        break;

    case vc1DEBLOCK_MainP:
    case vc1DEBLOCK_MainPI:
    case vc1DEBLOCK_AdvancedP:
    case vc1DEBLOCK_FieldB:

        COVERAGE("8.6.2");
        /* Deblock depending on motion vectors and subblock pattern */
        SBP  = vc1DEBLOCK_SubBlockPattern(pBlk);
        DEBUG1(vc1DEBUG_DEBLK, "Horizontal deblock P SBP = %x\n", SBP);
        if (pD)
        {
            /* First deblock the block boundry */
            int Edge = 2 + 1; /* Mark bottom left and right subblocks */

            if (eDeBlkType != vc1DEBLOCK_MainPI
                && eDeBlkType != vc1DEBLOCK_FieldB
                && vc1_BlkTypeIsInter(eBlkType)
                && vc1_BlkTypeIsInter(pD->eBlkType)
                && pBlk->u.sInter.sMotion[0].sMV.X == pD->u.sInter.sMotion[0].sMV.X
                && pBlk->u.sInter.sMotion[0].sMV.Y == pD->u.sInter.sMotion[0].sMV.Y
                && pBlk->u.sInter.sMotion[0].sMV.BottomField 
                    == pD->u.sInter.sMotion[0].sMV.BottomField
               )
            {
                /* Motion vectors match between the two blocks */
                int DSBP = vc1DEBLOCK_SubBlockPattern(pD);
                int TSBP = SBP;
                DEBUG1(vc1DEBUG_DEBLK, "Horizontal deblock P DSBP = %x\n", DSBP);

                if (eDeBlkType == vc1DEBLOCK_MainP)
                {
                    /* Historical fix for 4x4 blocks */
                    if (eBlkType==vc1_BlkInter4x4 && TSBP)
                    {
                        TSBP = 0xF;
                    }
                    if (pD->eBlkType==vc1_BlkInter4x4 && DSBP)
                    {
                        DSBP = 0xF;
                    }
                }

                Edge = TSBP | (DSBP>>2);
            }

            if (Edge & 2) /* Bottom left subblock */
            {
                /* Deblock first 4 pixels of bottom edge */
                DEBUG0(vc1DEBUG_DEBLK, "Horiztonal deblock bottom left\n");
                vc1DEBLOCK_HorizDeblock4(pData + 7*Bpl, PQuant, Bpl);
            }
            if (Edge & 1) /* Bottom right subblock */
            {
                /* Deblock second 4 pixels of bottom edge */
                DEBUG0(vc1DEBUG_DEBLK, "Horiztonal deblock bottom right\n");
                vc1DEBLOCK_HorizDeblock4(pData + 7*Bpl + 4, PQuant, Bpl);
            }
        }

        /* Deblock internal borders */
        if (eBlkType==vc1_BlkInter8x4 || eBlkType==vc1_BlkInter4x4)
        {
            if ((SBP & 8) || (SBP & 2))
            {
                /* Deblock first 4 pixels of subblock edge */
                DEBUG0(vc1DEBUG_DEBLK, "Horiztonal deblock mid left\n");
                vc1DEBLOCK_HorizDeblock4(pData + 3*Bpl, PQuant, Bpl);
            }
            if ((SBP & 4) || (SBP & 1))
            {
                /* Deblock second 4 pixels of subblock edge */
                DEBUG0(vc1DEBUG_DEBLK, "Horiztonal deblock mid right\n");
                vc1DEBLOCK_HorizDeblock4(pData + 3*Bpl + 4, PQuant, Bpl);
            }
        }
        break;

    case vc1DEBLOCK_FrameFirst:
        if (Blk==vc1_BlkY2 || Blk==vc1_BlkY3)
        {
            /* Internal edges */
            if (eBlkType==vc1_BlkInter8x4 || eBlkType==vc1_BlkInter4x4)
            {
                /* Deblock rows 2 and 4 of Y */
                DEBUG0(vc1DEBUG_DEBLK, "Horiztonal deblock rows 2, 4\n");
                vc1DEBLOCK_HorizDeblock4(pData+2*Bpl, PQuant, 2*Bpl);
                vc1DEBLOCK_HorizDeblock4(pData+2*Bpl+4, PQuant, 2*Bpl);
                /* Deblock rows 3 and 5 of Y */
                DEBUG0(vc1DEBUG_DEBLK, "Horiztonal deblock rows 3, 5\n");
                vc1DEBLOCK_HorizDeblock4(pData+3*Bpl, PQuant, 2*Bpl);
                vc1DEBLOCK_HorizDeblock4(pData+3*Bpl+4, PQuant, 2*Bpl);
            }
        }
        if (pD || Blk==vc1_BlkY0 || Blk==vc1_BlkY1)
        {
            /* Deblock rows 6 and 8 of Y */
            DEBUG0(vc1DEBUG_DEBLK, "Horiztonal deblock rows 6, 8\n");
            vc1DEBLOCK_HorizDeblock4(pData+6*Bpl, PQuant, 2*Bpl);
            vc1DEBLOCK_HorizDeblock4(pData+6*Bpl+4, PQuant, 2*Bpl);
            /* Deblock rows 7 and 9 of Y */
            DEBUG0(vc1DEBUG_DEBLK, "Horiztonal deblock rows 7, 9\n");
            vc1DEBLOCK_HorizDeblock4(pData+7*Bpl, PQuant, 2*Bpl);
            vc1DEBLOCK_HorizDeblock4(pData+7*Bpl+4, PQuant, 2*Bpl);
        }
        break;

    case vc1DEBLOCK_Frame:
        if (pD || Blk==vc1_BlkY0 || Blk==vc1_BlkY1)
        {
            /* Internal edges */
            if (eBlkType==vc1_BlkInter8x4 || eBlkType==vc1_BlkInter4x4)
            {
                /* Deblock rows 2 and 4 of Y */
                DEBUG0(vc1DEBUG_DEBLK, "Horiztonal deblock rows 2, 4\n");
                vc1DEBLOCK_HorizDeblock4(pData+2*Bpl, PQuant, 2*Bpl);
                vc1DEBLOCK_HorizDeblock4(pData+2*Bpl+4, PQuant, 2*Bpl);
                /* Deblock rows 3 and 5 of Y */
                DEBUG0(vc1DEBUG_DEBLK, "Horiztonal deblock rows 3, 5\n");
                vc1DEBLOCK_HorizDeblock4(pData+3*Bpl, PQuant, 2*Bpl);
                vc1DEBLOCK_HorizDeblock4(pData+3*Bpl+4, PQuant, 2*Bpl);
            }
            /* Deblock rows 6 and 8 of Y */
            DEBUG0(vc1DEBUG_DEBLK, "Horiztonal deblock rows 6, 8\n");
            vc1DEBLOCK_HorizDeblock4(pData+6*Bpl, PQuant, 2*Bpl);
            vc1DEBLOCK_HorizDeblock4(pData+6*Bpl+4, PQuant, 2*Bpl);
            /* Deblock rows 7 and 9 of Y */
            DEBUG0(vc1DEBUG_DEBLK, "Horiztonal deblock rows 7, 9\n");
            vc1DEBLOCK_HorizDeblock4(pData+7*Bpl, PQuant, 2*Bpl);
            vc1DEBLOCK_HorizDeblock4(pData+7*Bpl+4, PQuant, 2*Bpl);
        }
        break;

    case vc1DEBLOCK_FrameField: /* Y only */
        if (Blk==vc1_BlkY2 || Blk==vc1_BlkY3)
        {
            pData -= 7*Bpl;
        }
        if (eBlkType==vc1_BlkInter8x4 || eBlkType==vc1_BlkInter4x4)
        {
            /* Deblock rows 6 and 8 of Y */
            DEBUG0(vc1DEBUG_DEBLK, "Horiztonal deblock rows 6, 8\n");
            vc1DEBLOCK_HorizDeblock4(pData+6*Bpl, PQuant, 2*Bpl);
            vc1DEBLOCK_HorizDeblock4(pData+6*Bpl+4, PQuant, 2*Bpl);
        }
        if (pD)
        {
            /* Deblock rows 14 and 16 of Y */
            DEBUG0(vc1DEBUG_DEBLK, "Horiztonal deblock rows 14, 16\n");
            vc1DEBLOCK_HorizDeblock4(pData+14*Bpl, PQuant, 2*Bpl);
            vc1DEBLOCK_HorizDeblock4(pData+14*Bpl+4, PQuant, 2*Bpl);
        }
        break;

    default:
        FATAL("Unsupported deblock type %d\n", eDeBlkType);
        break;
    }
}

/*
 * Description:
 * Vertically deblock the right and internal edges of a block
 *
 * Remarks:
 * For progressive P pictures the block boundary must be deblocked
 * before the internal subblock boundaries. Therefore we must deblock
 * the right edge of a block before the internal subblock boundary.
 * This is why this routine concentrates on right edges rather than
 * left edges as in overlap smoothing.
 *
 * Inputs:
 * pC           - pointer to start of block component
 * pBlk         - pointer to block to deblock
 * pR           - pointer to block to the right/NULL if not present
 * PQuant       - quantizer
 * Blk          - block number of the block
 * eDeBlkType   - type of deblock to apply
 *
 * Outputs:
 * pC           - vertical edges deblocked
 */

static void vc1DEBLOCK_VertDeblockBlk(
    vc1_sComponent *pC, 
    vc1_sBlk *pBlk,
    vc1_sBlk *pR,
    int PQuant,
    int Blk,
    vc1DEBLOCK_eDeBlkType eDeBlkType)
{
    vc1_eBlkType eBlkType = pBlk->eBlkType;
    UBYTE8 *pData = pC->pData;
    int Bpl = pC->Bpl;
    int SBP;

    DEBUG3(vc1DEBUG_DEBLK, "Vertical deblock Blk=%d Type=%s (%d)\n",
        Blk, vc1DEBUG_DeBlkType[eDeBlkType], eDeBlkType);

    switch (eDeBlkType)
    {
    case vc1DEBLOCK_ProgressiveI:   /* I or B or BI */

        COVERAGE("10.10.3");  /* B fields use this case */
        /* Deblock all 8x8 boundaries */
        if (pR)
        {
            DEBUG0(vc1DEBUG_DEBLK, "Vertical deblock I right edge\n");
            vc1DEBLOCK_VertDeblock4(pData + 7, PQuant, Bpl);
            vc1DEBLOCK_VertDeblock4(pData + 7 + 4*Bpl, PQuant, Bpl);
        }
        break;

    case vc1DEBLOCK_MainP:
    case vc1DEBLOCK_MainPI:
    case vc1DEBLOCK_AdvancedP:
    case vc1DEBLOCK_FieldB:

        SBP = vc1DEBLOCK_SubBlockPattern(pBlk);
        DEBUG1(vc1DEBUG_DEBLK, "Vertical deblock P SBP = %x\n", SBP);

        if (pR)
        {
            /* First deblock the block boundry */
            vc1_sBlk *pL = pBlk;
            int LSBP = SBP;
            int Edge = 4 + 1;  /* Mark top right and bottom right subblocks */

            if (eDeBlkType==vc1DEBLOCK_MainP || eDeBlkType==vc1DEBLOCK_MainPI)
            {
                if (Blk == vc1_BlkY2)
                {
                    /* In this case we look at Y1 and Y3 to decide whether
                     * to deblock the border rather than Y2 and Y3. This is
                     * due to historical reasons concerning the original VC1
                     * implementation.
                     */
                    pL   = pBlk - 1;
                    LSBP = vc1DEBLOCK_SubBlockPattern(pL);
                    DEBUG1(vc1DEBUG_DEBLK, "Vertical deblock P LSBP = %x\n", LSBP);
                }
            }

            if (eDeBlkType != vc1DEBLOCK_MainPI
                && eDeBlkType != vc1DEBLOCK_FieldB
                && vc1_BlkTypeIsInter(pBlk->eBlkType)
                && vc1_BlkTypeIsInter(pR->eBlkType)
                && pBlk->u.sInter.sMotion[0].sMV.X == pR->u.sInter.sMotion[0].sMV.X
                && pBlk->u.sInter.sMotion[0].sMV.Y == pR->u.sInter.sMotion[0].sMV.Y
                && pBlk->u.sInter.sMotion[0].sMV.BottomField 
                    == pR->u.sInter.sMotion[0].sMV.BottomField
               )
            {
                /* Motion vectors match between the two blocks */
                int RSBP = vc1DEBLOCK_SubBlockPattern(pR);
                DEBUG1(vc1DEBUG_DEBLK, "Vertical deblock P RSBP = %x\n", RSBP);

                if (eDeBlkType == vc1DEBLOCK_MainP)
                {
                    /* Historical fix for 4x4 blocks */
                    if (pL->eBlkType==vc1_BlkInter4x4 && LSBP)
                    {
                        LSBP = 0xF;
                    }
                    if (pR->eBlkType==vc1_BlkInter4x4 && RSBP)
                    {
                        RSBP = 0xF;
                    }
                }

                Edge = LSBP | (RSBP>>1);
            }

            if (Edge & 4) /* Top-right subblock */
            {
                /* Deblock first 4 pixels of right edge */
                DEBUG0(vc1DEBUG_DEBLK, "Vertical deblock top right\n");
                vc1DEBLOCK_VertDeblock4(pData + 7, PQuant, Bpl);
            }
            if (Edge & 1) /* Bottom-right subblock */
            {
                /* Deblock second 4 pixels of right edge */
                DEBUG0(vc1DEBUG_DEBLK, "Vertical deblock bottom right\n");
                vc1DEBLOCK_VertDeblock4(pData + 7 + 4*Bpl, PQuant, Bpl);
            }
        }

        /* Deblock internal borders */
        if (eBlkType==vc1_BlkInter4x8 || eBlkType==vc1_BlkInter4x4)
        {
            if ((SBP & 8) || (SBP & 4))
            {
                /* Deblock first 4 pixels of subblock edge */
                DEBUG0(vc1DEBUG_DEBLK, "Vertical deblock mid top\n");
                vc1DEBLOCK_VertDeblock4(pData + 3, PQuant, Bpl);
            }
            if ((SBP & 2) || (SBP & 1))
            {
                /* Deblock second 4 pixels of subblock edge */
                DEBUG0(vc1DEBUG_DEBLK, "Vertical deblock mid bottom\n");
                vc1DEBLOCK_VertDeblock4(pData + 3 + 4*Bpl, PQuant, Bpl);
            }
        }
        break;

    case vc1DEBLOCK_Frame:
    case vc1DEBLOCK_FrameFirst:
        COVERAGE("10.10.4.3");
        if (eBlkType==vc1_BlkInter4x8 || eBlkType==vc1_BlkInter4x4)
        {
            COVERAGE("10.10.4.4");
            /* Deblock internal edge */
            vc1DEBLOCK_VertDeblock4(pData + 3      , PQuant, 2*Bpl);
            vc1DEBLOCK_VertDeblock4(pData + 3 + Bpl, PQuant, 2*Bpl);
        }
        if (Blk==vc1_BlkY0 || Blk==vc1_BlkY2 || pR)
        {
            /* Deblock right edge */
            vc1DEBLOCK_VertDeblock4(pData + 7      , PQuant, 2*Bpl);
            vc1DEBLOCK_VertDeblock4(pData + 7 + Bpl, PQuant, 2*Bpl);
        }
        break;

    case vc1DEBLOCK_FrameField: /* Y only */
        if (Blk==vc1_BlkY2 || Blk==vc1_BlkY3)
        {
            pData -= 7*Bpl;
        }
        if (eBlkType==vc1_BlkInter4x8 || eBlkType==vc1_BlkInter4x4)
        {
            /* Deblock internal edge */
            vc1DEBLOCK_VertDeblock4(pData + 3         , PQuant, 2*Bpl);
            vc1DEBLOCK_VertDeblock4(pData + 3 + 8*Bpl , PQuant, 2*Bpl);
        }
        if (Blk==vc1_BlkY0 || Blk==vc1_BlkY2 || pR)
        {
            /* Deblock right edge */
            vc1DEBLOCK_VertDeblock4(pData + 7         , PQuant, 2*Bpl);
            vc1DEBLOCK_VertDeblock4(pData + 7 + 8*Bpl , PQuant, 2*Bpl);
        }
        break;

    default:
        FATAL("Unsupported deblock type %d\n", eDeBlkType);
        break;
    }
}

/*
 * Description:
 * Deblock horizontal bottom and internal edges in a macroblock
 *
 * Remarks:
 * For progressive P pictures the block boundary must be deblocked
 * before the internal subblock boundaries. Therefore we must deblock
 * the bottom edge of a block before the internal subblock boundary.
 * This is why this routine concentrates on bottom edges rather than
 * top edges as in overlap smoothing.
 *
 * Inputs:
 * pRefPic      - pointer to picture on which the deblocking filter operates
 * pPosition    - pointer to position structure describing the MB cyclic buffer
 * eDeBlkType   - type of deblocking to perform
 * LastRow      - True if this block is in the last row of the image
 *
 * Outputs:
 * pRefPic      - updated with deblocked image data
 *
 * Return Value:
 * None
 *
 */

static void vc1DEBLOCK_HorizDeblockMB(
    vc1_sReferencePicture * pRefPic,
    vc1_sPosition * pPosition,
    vc1DEBLOCK_eDeBlkType eDeBlkType,
    int LastRow
)
{
    vc1_sMB *pMB;           /* Macroblock to deblock */
    vc1_sMB *pD = NULL;     /* Macroblock below pMB */
    vc1_sComponent sC;      /* Component to deblock */
    vc1DEBLOCK_eDeBlkType eTypeTopLum = eDeBlkType;
    vc1DEBLOCK_eDeBlkType eTypeBotLum = eDeBlkType;
    vc1DEBLOCK_eDeBlkType eTypeChroma = eDeBlkType;
    int PQuant = pPosition->PQuant;
    int WidthMB = pPosition->WidthMB;

    DEBUG3(vc1DEBUG_DEBLK, "Horizontal deblock X = %d, Y = %d, Type = %s\n",
        pPosition->X, pPosition->Y + pPosition->SliceY, vc1DEBUG_DeBlkType[eDeBlkType]);

    /* Find currently decoded macroblock */
    pMB = pPosition->pCurMB;

    /* Find macroblock below the deblock MB */
    if (!LastRow)
    {
        pD = pMB + WidthMB;
    }

    /* Find deblock type of individual blocks */
    if (eDeBlkType == vc1DEBLOCK_Frame)
    {
        if (pPosition->Y == 0)
        {
            eTypeTopLum = vc1DEBLOCK_FrameFirst;
            eTypeChroma = vc1DEBLOCK_FrameFirst;
        }
        if (vc1_MBTypeIsFieldTX(pMB->eMBType))
        {
            eTypeTopLum = vc1DEBLOCK_FrameField;
            eTypeBotLum = vc1DEBLOCK_FrameField;
        }
    }

    if (eDeBlkType == vc1DEBLOCK_MainPI)
    {
        /* 4MV (inluding top+left) doesn't use MainPI */
        if (vc1_MBTypeIs4MV(pMB->eMBType))
        {
            eTypeTopLum = vc1DEBLOCK_MainP;
        }
        if (pD && vc1_MBTypeIs4MV(pD->eMBType))
        {
            eTypeBotLum = vc1DEBLOCK_MainP;
            eTypeChroma = vc1DEBLOCK_MainP;
        }
    }

    /* Horizontal Deblock Luma Blocks */
    vc1TOOLS_GetPictureDestination(&sC, pRefPic, pPosition, 0);

    /* Top luma blocks */
    if (pD || eTypeTopLum!=vc1DEBLOCK_FrameField)
    {
        vc1DEBLOCK_HorizDeblockBlk(&sC, &pMB->sBlk[0], &pMB->sBlk[2], PQuant, 0, eTypeTopLum);
        sC.pData += 8;
        vc1DEBLOCK_HorizDeblockBlk(&sC, &pMB->sBlk[1], &pMB->sBlk[3], PQuant, 1, eTypeTopLum);
    }
    else
    {
        vc1DEBLOCK_HorizDeblockBlk(&sC, &pMB->sBlk[0], NULL, PQuant, 0, eTypeTopLum);
        sC.pData += 8;
        vc1DEBLOCK_HorizDeblockBlk(&sC, &pMB->sBlk[1], NULL, PQuant, 1, eTypeTopLum);
    }
    sC.pData += 8*sC.Bpl - 8;

    /* Bottom luma blocks */
    if (pD)
    {
        vc1DEBLOCK_HorizDeblockBlk(&sC, &pMB->sBlk[2], &pD->sBlk[0], PQuant, 2, eTypeBotLum);
        sC.pData += 8;
        vc1DEBLOCK_HorizDeblockBlk(&sC, &pMB->sBlk[3], &pD->sBlk[1], PQuant, 3, eTypeBotLum);
    }
    else
    {
        vc1DEBLOCK_HorizDeblockBlk(&sC, &pMB->sBlk[2], NULL, PQuant, 2, eTypeBotLum);
        sC.pData += 8;
        vc1DEBLOCK_HorizDeblockBlk(&sC, &pMB->sBlk[3], NULL, PQuant, 3, eTypeBotLum);
    }

    /* Horizontal deblock Cb */
    vc1TOOLS_GetPictureDestination(&sC, pRefPic, pPosition, vc1_BlkCb);
    if (pD)
    {
        vc1DEBLOCK_HorizDeblockBlk(&sC, &pMB->sBlk[4], &pD->sBlk[4], PQuant, 4, eTypeChroma);
    }
    else
    {
        vc1DEBLOCK_HorizDeblockBlk(&sC, &pMB->sBlk[4], NULL, PQuant, 4, eTypeChroma);
    }

    /* Horizontal deblock Cr */
    vc1TOOLS_GetPictureDestination(&sC, pRefPic, pPosition, vc1_BlkCr);
    if (pD)
    {
        vc1DEBLOCK_HorizDeblockBlk(&sC, &pMB->sBlk[5], &pD->sBlk[5], PQuant, 5, eTypeChroma);
    }
    else
    {
        vc1DEBLOCK_HorizDeblockBlk(&sC, &pMB->sBlk[5], NULL, PQuant, 5, eTypeChroma);
    }
}


/*
 * Description:
 * Deblock vertical right and internal edges in a macroblock
 *
 * Remarks:
 * For progressive P pictures the block boundary must be deblocked
 * before the internal subblock boundaries. Therefore we must deblock
 * the right edge of a block before the internal subblock boundary.
 * This is why this routine concentrates on right edges rather than
 * left edges as in overlap smoothing.
 *
 * Inputs:
 * pRefPic      - pointer to picture on which the deblocking filter operates
 * pPosition    - pointer to position structure describing the MB cyclic buffer
 * eDeBlkType   - type of deblocking to perform
 *
 * Outputs:
 * pRefPic      - updated with deblocked image data
 *
 * Return Value:
 * None
 *
 */

static void vc1DEBLOCK_VertDeblockMB(
    vc1_sReferencePicture * pRefPic,
    vc1_sPosition * pPosition,
    vc1DEBLOCK_eDeBlkType eDeBlkType
)
{
    vc1_sMB *pMB;           /* Macroblock to deblock */
    vc1_sMB *pR = NULL;     /* Macroblock to the right of pMB / NULL */
    vc1_sComponent sC;      /* Component to deblock */
    vc1DEBLOCK_eDeBlkType eType = eDeBlkType;
    int PQuant  = pPosition->PQuant;
    int WidthMB = pPosition->WidthMB;

    DEBUG3(vc1DEBUG_DEBLK, "Vertical deblock X = %d, Y = %d, Type = %s\n",
        pPosition->X, pPosition->Y, vc1DEBUG_DeBlkType[eDeBlkType]);

    /* Find currently decoded macroblock */
    pMB = pPosition->pCurMB;

    /* Find macroblock right of the deblock MB */
    if (pPosition->X + 1 < WidthMB)
    {
        pR = pMB + 1;
    }

    /* Find deblock type */
    if (eType == vc1DEBLOCK_Frame)
    {
        if (vc1_MBTypeIsFieldTX(pMB->eMBType))
        {
            eType = vc1DEBLOCK_FrameField;
        }
    }

    if (eDeBlkType == vc1DEBLOCK_MainPI && vc1_MBTypeIs4MV(pMB->eMBType))
    {
        /* 4MV does not use MainPI deblock */
        eType = vc1DEBLOCK_MainP;
    }

    /* Vertical Deblock Luma Blocks */
    vc1TOOLS_GetPictureDestination(&sC, pRefPic, pPosition, 0);

    /* Left luma blocks */
    vc1DEBLOCK_VertDeblockBlk(&sC, &pMB->sBlk[0], &pMB->sBlk[1], PQuant, 0, eType);
    sC.pData += 8*sC.Bpl;
    vc1DEBLOCK_VertDeblockBlk(&sC, &pMB->sBlk[2], &pMB->sBlk[3], PQuant, 2, eType);
    sC.pData += -8*sC.Bpl + 8;

    /* Find deblock type for right hand edges */
    if (eDeBlkType == vc1DEBLOCK_MainPI)
    {
        eType = vc1DEBLOCK_MainPI;
        if (pR && vc1_MBTypeIs4MV(pR->eMBType))
        {
            eType = vc1DEBLOCK_MainP;
        }
    }

    /* Right luma blocks */
    if (pR)
    {
        vc1DEBLOCK_VertDeblockBlk(&sC, &pMB->sBlk[1], &pR->sBlk[0], PQuant, 1, eType);
        sC.pData += 8*sC.Bpl;
        vc1DEBLOCK_VertDeblockBlk(&sC, &pMB->sBlk[3], &pR->sBlk[2], PQuant, 3, eType);
    }
    else
    {
        vc1DEBLOCK_VertDeblockBlk(&sC, &pMB->sBlk[1], NULL, PQuant, 1, eType);
        sC.pData += 8*sC.Bpl;
        vc1DEBLOCK_VertDeblockBlk(&sC, &pMB->sBlk[3], NULL, PQuant, 3, eType);
    }

    if (eDeBlkType == vc1DEBLOCK_Frame)
    {
        eType = vc1DEBLOCK_Frame;
    }

    /* Vertical deblock Cb */
    vc1TOOLS_GetPictureDestination(&sC, pRefPic, pPosition, vc1_BlkCb);
    if (pR)
    {
        vc1DEBLOCK_VertDeblockBlk(&sC, &pMB->sBlk[4], &pR->sBlk[4], PQuant, 4, eType);
    }
    else
    {
        vc1DEBLOCK_VertDeblockBlk(&sC, &pMB->sBlk[4], NULL, PQuant, 4, eType);
    }

    /* Vertical deblock Cr */
    vc1TOOLS_GetPictureDestination(&sC, pRefPic, pPosition, vc1_BlkCr);
    if (pR)
    {
        vc1DEBLOCK_VertDeblockBlk(&sC, &pMB->sBlk[5], &pR->sBlk[5], PQuant, 5, eType);
    }
    else
    {
        vc1DEBLOCK_VertDeblockBlk(&sC, &pMB->sBlk[5], NULL, PQuant, 5, eType);
    }
}

/*
 * Description:
 * Deblock a picture slice
 *
 * Inputs:
 * pPosition    - Position of the END of the slice
 *
 * Outputs:
 * Picture slice deblocked
 */

void vc1DEBLOCK_DeblockSlice(vc1_sPosition *pPosition)
{
    vc1_sReferencePicture * pRefPic;
    vc1DEBLOCK_eDeBlkType eDeBlkType;
    vc1_ePictureType ePictureType = pPosition->ePictureType;
    int WidthMB = pPosition->WidthMB;
    int HeightMB = pPosition->Y;
    int X, Y;

    COVERAGE("8.6.1");

    /* Rewind to the first macroblock in the slice */
    pPosition->pCurMB -= WidthMB * HeightMB;

    /* Find deblock type */
    eDeBlkType = vc1DEBLOCK_ProgressiveI;

    switch (pPosition->ePictureFormat)
    {
        case vc1_InterlacedField:
            if (ePictureType == vc1_PictureTypeB)
            {
                eDeBlkType = vc1DEBLOCK_FieldB;
            }
            /* Fall through */

        case vc1_ProgressiveFrame:

            if (ePictureType == vc1_PictureTypeP)
            {
                vc1_sMB *pMB = pPosition->pCurMB;
                eDeBlkType = vc1DEBLOCK_MainP;

                if (pPosition->eProfile == vc1_ProfileAdvanced)
                {
                    eDeBlkType = vc1DEBLOCK_AdvancedP;
                }
                else if (vc1_BlkTypeIsIntra(pMB->sBlk[0].eBlkType))
                {
                    /* Main, first block is intra */
                    eDeBlkType = vc1DEBLOCK_MainPI;
                }
            }
            break;

        case vc1_InterlacedFrame:

            eDeBlkType = vc1DEBLOCK_Frame;
            COVERAGE("10.10.4.1");
            break;

        case vc1_PictureFormatNone:
            WARN("DeblockSlice: Unknown picture format\n");
            return;
    }

    DEBUG1(vc1DEBUG_DEBLK, "DeBlkType = %d\n", eDeBlkType);

    /* Find output picture */
    if (vc1_PictureTypeIsRef(pPosition->ePictureType))
    {
        pRefPic = pPosition->pReferenceNew;
    }
    else
    {
        pRefPic = pPosition->pReferenceB;
    }

    /* Deblock horizontal edges */
    DEBUG2(vc1DEBUG_DEBLK, "Deblock slice horizontal %dx%d\n", WidthMB, HeightMB);
    for (Y=0; Y<HeightMB; Y++)
    {
        pPosition->Y = (UHWD16)Y;

        for (X=0; X<WidthMB; X++)
        {
            pPosition->X = (UHWD16)X;
            vc1DEBLOCK_HorizDeblockMB(pRefPic, pPosition, eDeBlkType, Y+1==HeightMB);
            pPosition->pCurMB++;
        }
    }

    /* Deblock vertical edges */
    DEBUG2(vc1DEBUG_DEBLK, "Deblock slice vertical %dx%d\n", WidthMB, HeightMB);
    pPosition->pCurMB -= WidthMB * HeightMB;
    for (Y=0; Y<HeightMB; Y++)
    {
        pPosition->Y = (UHWD16)Y;

        for (X=0; X<WidthMB; X++)
        {
            pPosition->X = (UHWD16)X;
            vc1DEBLOCK_VertDeblockMB(pRefPic, pPosition, eDeBlkType);
            pPosition->pCurMB++;
        }
    }

    pPosition->Y = (UHWD16)HeightMB;
    pPosition->X = 0;
}
