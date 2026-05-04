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
 * vc1recon.c:
 * Block reconstruction functions
 *
 */

#include "vc1types.h"
#include "vc1debug.h"
#include "vc1iquant.h"
#include "vc1itrans.h"
#include "vc1smooth.h"
#include "vc1tools.h"
#include "vc1recon.h"
#include "vc1interp.h"

/*
 * Description:
 * Combine the predicted and difference macroblock, and copy them to a reference picture
 *
 * Remarks:
 * None
 *
 * Inputs:
 * pRefPic      - pointer to initialised reference picture
 * pMBData      - pointer to differential macroblock
 * pPredMB      - pointer to predicted macroblock
 * pPosition    - pointer to struct representing current macroblock's position
 * eProfile     - profile of current bitstream
 *
 * Outputs:
 * pRefPic      - updated with new block data
 *
 * Return Value:
 * None
 *
 */

static void vc1RECON_ApplyPredictionAndCopyMB(  vc1_sReferencePicture * pRefPic,
                                                HWD16 pMBData[6][64],
                                                const UBYTE8 pPredMB[6][64],
                                                vc1_sPosition * pPosition,
                                                vc1_eProfile eProfile)
{
    int             I, J;
    vc1_sComponent  sC;
    UBYTE8          *pDest;
    int             Blk;
    const HWD16     *pPixel;
    const UBYTE8    *pPredBlk;
    HWD16           TempMB[6][64];
    UBYTE8          TempPredMB[6][64];
    vc1_eMBType     eMBType = pPosition->pCurMB->eMBType;
    vc1_sMB         *pMB = pPosition->pCurMB;
    
    ASSERT(pRefPic != NULL);

    /* if the difference macroblock is field separated, reinterlace it */
    if(TRUE == vc1_MBTypeIsFieldTX(eMBType))
    {
        DEBUG0(vc1DEBUG_MB, "Interlacing FieldTX macroblock\n");
        vc1INTERP_InterlaceDiffMB(TempMB, pMBData);
        pMBData = TempMB;
    }

    /* if the predicted macroblock is field separated, reinterlace it */
    if(TRUE == vc1_MBTypeIsFieldMV(eMBType))
    {
        COVERAGE("10.7.2.1.2");
        DEBUG0(vc1DEBUG_MB, "Interlaced FieldMV macroblock\n");
        vc1INTERP_InterlacePredMB(TempPredMB, pPredMB);
        pPredMB = TempPredMB;
    }

    /* by this point, both macroblocks should be in the same format */

    /* overlap smooth state machine - always call */
    DEBUG1(vc1DEBUG_MB, "Overlap = %d\n", pMB->OverlapFilter);
    vc1SMOOTH_OverlapSmoothMB(pRefPic, pPosition, pMBData);

    for(Blk = 0; Blk < VC1_BLOCKS_PER_MB; Blk++)
    {
        vc1_eBlkType eBlkType = pPosition->pCurMB->sBlk[Blk].eBlkType;

        pPixel = pMBData[Blk];
        pPredBlk = pPredMB[Blk];

        /* 
         * obtain a component pointer at the point in the reference picture
         * at which the new block will be written
         */
        vc1TOOLS_GetPictureDestination(&sC, pRefPic, pPosition, Blk);
        pDest = sC.pData;

        DEBUG1(  vc1DEBUG_RBLK,
                "ApplyErrorAndCopyMB: ePictureFormat = %d\n",
                pPosition->ePictureFormat);
        DEBUG4(  vc1DEBUG_RBLK,
                "ApplyErrorAndCopyMB: X = %d, Y = %d, Blk = %d, Overlap = %d\n",
                pPosition->X,
                pPosition->Y,
                Blk,
                pMB->OverlapFilter);

        if(TRUE == vc1_BlkTypeIsIntra(eBlkType))
        {
            int Offset = 128;

            if (vc1_PictureTypeIsIntra(pPosition->ePictureType) /* I or BI */
                && (eProfile==vc1_ProfileSimple || eProfile==vc1_ProfileMain) /* Simple/Main */
                && FALSE == pMB->OverlapFilter) /* No Overlap */
            {
                Offset = 0;
            }

            /* this is an intra block - no predicted block to add in */
            for(J = 0; J < 8; J++)
            {
                for(I = 0; I < 8; I++)
                {
                    int Pixel = CLIP(pPixel[I] + Offset);
#if 0
                    /* frame luma blocks */
                    if(((0 == I) || (0 == J)) && (TRUE == vc1_BlkNumIsLuma(Blk)))
                    {
                        Pixel = 255;
                    }
#endif
                    pDest[I] = (UBYTE8)Pixel;
                }
                pPixel += 8;
                pDest += sC.Bpl;
            }
            DEBUG0(vc1DEBUG_RBLK, "Result Block...\n");
            DEBUGRECT8(vc1DEBUG_RBLK, sC.pData, 8, 8, sC.Bpl);
        }
        else
        {
            DEBUG0(vc1DEBUG_RBLK, "Difference Block...\n");
            DEBUGRECT16(vc1DEBUG_RBLK, pPixel, 8, 8, 8);

            DEBUG0(vc1DEBUG_RBLK, "Interpolated Block...\n");
            DEBUGRECT8(vc1DEBUG_RBLK, pPredBlk, 8, 8, 8);

            COVERAGE("8.3.6.5.3");

            /* this is an inter block - add in predicted block */
            for(J = 0; J < 8; J++)
            {
                for(I = 0; I < 8; I++)
                {
                    int Pixel = CLIP(pPixel[I] + pPredBlk[I]);

#if 0
                    /* frame luma blocks */
                    if(((0 == I) || (0 == J)) && (TRUE == vc1_BlkNumIsLuma(Blk)))
                    {
                        Pixel = 255;
                    }
#endif

                    pDest[I] = (UBYTE8)Pixel;
                }
                pPredBlk += 8;
                pPixel += 8;
                pDest += sC.Bpl;
            }

            DEBUG0(vc1DEBUG_RBLK, "Result Block...\n");
            DEBUGRECT8(vc1DEBUG_RBLK, sC.pData, 8, 8, sC.Bpl);
        }
    } /* Block loop */

    /* Overlap smooth - last MB in a row special case */
    if (pPosition->X+1 == pPosition->WidthMB && pMB->OverlapFilter)
    {
        /* Last macroblock in row so smooth horizontal top edges now */
        DEBUG0(vc1DEBUG_SMOOTH, "Last block in row\n");
        vc1SMOOTH_OverlapSmoothHorizMB(pRefPic, pPosition);
    }
}



/*
 * Description:
 * Reconstruct a macroblock from transform coefficients, and write into a picture
 *
 * Remarks:
 * Includes dequantise, inverse transform, overlap smooth and clamping
 *
 * Inputs:
 * pTCoefs      - pointer to 6 8x8 arrays of transform coefficients
 * pPredBlk     - pointer to 6 8x8 predicted blocks
 * pRefPic      - pointer to picture structure into which blocks will be written
 * pPosition    - pointer to position structure representing current MB
 * eProfile     - profile (simple/main/advanced) of stream
 *
 * Outputs:
 * pRefPic      - updated with newly reconstructed blocks
 *
 * Return Value:
 * None.
 *
 */

void vc1RECON_ReconstructMB( const HWD16  pTCoefs[6][64],
                                const UBYTE8 pPredBlk[6][64],
                                vc1_sReferencePicture * pRefPic,
                                vc1_sPosition * pPosition, 
                                vc1_eProfile eProfile)
{
    int Blk;
    HWD16 LocalTCoefs[6][64] = {{0}};
    
    for(Blk = 0; Blk < VC1_BLOCKS_PER_MB; Blk++)
    {
        vc1_sMB         *pMB = pPosition->pCurMB;
        vc1_eBlkType    eBlkType = pMB->sBlk[Blk].eBlkType;

        if(FALSE == pMB->Skipped)
        {
            int Intra = vc1_BlkTypeIsIntra(eBlkType);

            /* inverse ac quantise */
            DEBUG0(vc1DEBUG_QUANT, "Inverse AC quantise\n");
            DEBUG1(vc1DEBUG_QUANT, "Pre-iACquant - Blk = %d\n", Blk);
            DEBUGRECT16(vc1DEBUG_QUANT, pTCoefs[Blk], 8, 8, 8);
            vc1IQUANT_InverseACQuantize(LocalTCoefs[Blk], pTCoefs[Blk], &pMB->sQuant, Intra);




            /* inverse quantise DC coefficient */
            if (Intra)
            {
                DEBUG0(vc1DEBUG_QUANT, "Inverse DC quantise\n");
                LocalTCoefs[Blk][0] = vc1IQUANT_InverseDCQuantize(pTCoefs[Blk][0], &pMB->sQuant);
            }



            /* inverse transform coefs */
            DEBUG0(vc1DEBUG_TRANS, "Inverse transform block\n");
            DEBUG1(vc1DEBUG_TRANS, "Pre-transform - Blk = %d\n", Blk);
            DEBUGRECT16(vc1DEBUG_TRANS, LocalTCoefs[Blk], 8, 8, 8);
            vc1ITRANS_InverseTransformBlock(LocalTCoefs[Blk], LocalTCoefs[Blk], eBlkType);
            DEBUG1(vc1DEBUG_TRANS, "Post-transform - Blk = %d\n", Blk);
            DEBUGRECT16(vc1DEBUG_TRANS, LocalTCoefs[Blk], 8, 8, 8);
        }          
        else
        {
            /* this macroblock is skipped - zero the block */
            int Count;

            for(Count = 0; Count < 64; Count++)
            {
                LocalTCoefs[Blk][Count] = 0;
            }
        }
    }

    /* add in predicted block, clamp results [0 255], and copy into picture*/
    DEBUG0(vc1DEBUG_RBLK, "Apply prediction and copy macroblock\n");
    vc1RECON_ApplyPredictionAndCopyMB(pRefPic, LocalTCoefs, pPredBlk, pPosition, eProfile);

    return;
}
