
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
 * vc1interp.c:
 * Block interpolation functions
 *
 */

#include "vc1types.h"
#include "vc1debug.h"
#include "vc1interp.h"
#include "vc1derivemv.h"
#include "vc1cropmv.h"

static const vc1INTERP_sBicubicFilterParams vc1INTERP_Bicubic_Filter_Params_Table[4] =
{
    {-4,    53,     18,     -3}, /* 1/4 */
    {-1,    9,      9,      -1}, /* 1/2 */
    {-3,    18,     53,     -4}  /* 3/4 */
};

static const UBYTE8 vc1INTERP_Bicubic_Vert_Filter_Shift_Table[4][3] =
{
    {6, 4, 6},
    {5, 3, 5},
    {3, 1, 3},
    {5, 3, 5}
};

static const UBYTE8 vc1INTERP_Bicubic_Horiz_Filter_Shift_Table[3][4] =
{
    {6, 7, 7, 7},
    {4, 7, 7, 7},
    {6, 7, 7, 7}
};

/*
 * Description:
 * Vertical bicubic interpolation of a patch to quarter pixel position resolution
 *
 * Remarks:
 * None
 *
 * Inputs:
 * pC         - pointer to a component into which the filtered patch will be written
 * pInterp    - pointer to structure containing parameters required for operation
 * X          - X position in quarter pixel units
 * Y          - Y position in quarter pixel units
 *
 * Outputs:
 * pC         - updated with the filtered patch
 *
 * Return Value:
 * None
 *
 */

static void vc1INTERP_InterpPatchQuarterPelBicubicVert( vc1_sComponent * pC, 
                                                        vc1_sInterpolate * pInterp, 
                                                        WORD32 X, 
                                                        WORD32 Y)
{
    int I, J;
    int SizeX       = pInterp->SizeX;
    int SizeY       = pInterp->SizeY;
    int IBPL        = pInterp->sC.Bpl;
    int PBPL        = pC->Bpl;
    int XPel        = X >> 2;
    int YPel        = Y >> 2;
    int R           = pInterp->RndCtrl;
    UBYTE8 *pSource = &pInterp->sC.pData[XPel + YPel * IBPL];
    UBYTE8 *pPixels = pC->pData;
    int Pixel;
    
    const vc1INTERP_sBicubicFilterParams *FP = &vc1INTERP_Bicubic_Filter_Params_Table[(Y & 3) - 1];
    int Abs, Shift;
    int F0          = FP->F0;
    int F1          = FP->F1;
    int F2          = FP->F2;
    int F3          = FP->F3;

    Shift           = vc1INTERP_Bicubic_Vert_Filter_Shift_Table[0][(Y & 3) - 1];
    Abs             = 1 << (Shift - 1);

    for(J = 0; J < SizeY; J++)
    {
        for(I = 0; I < SizeX; I++)
        {
            Pixel = ((  pSource[I - IBPL    ] * F0 +
                        pSource[I           ] * F1 +
                        pSource[I + IBPL    ] * F2 +
                        pSource[I + 2 * IBPL] * F3 +
                        Abs - 1 + R) >> Shift);
            pPixels[I] = CLIP(Pixel);
        }
        pSource += IBPL;
        pPixels += PBPL;
    }
}


/*
 * Description:
 * Vertical and horizontal bicubic interpolation of a patch to quarter pixel position resolution
 *
 * Remarks:
 * None
 *
 * Inputs:
 * pC         - pointer to a component into which the filtered patch will be written
 * pInterp    - pointer to structure containing parameters required for operation
 * X          - X position in quarter pixel units
 * Y          - Y position in quarter pixel units
 *
 * Outputs:
 * pC         - updated with the filtered patch
 *
 * Return Value:
 * None
 *
 */

static void vc1INTERP_InterpPatchQuarterPelBicubicDiag( vc1_sComponent * pC, 
                                                        vc1_sInterpolate * pInterp, 
                                                        WORD32 X, 
                                                        WORD32 Y)
{
    /* temporary block to hold the result of the vertical filter stage
     * with one left hand and two right hand extra columns
     */
    HWD16 TempBlock[(VC1INTERP_MAX_PATCH_SIZE + 3) * VC1INTERP_MAX_PATCH_SIZE];

    int I, J;
    int SizeX         = pInterp->SizeX;
    int SizeY         = pInterp->SizeY;
    int IBPL          = pInterp->sC.Bpl;
    int PBPL          = VC1INTERP_MAX_PATCH_SIZE + 3;
    int XPel          = X >> 2;
    int YPel          = Y >> 2;
    int R             = pInterp->RndCtrl;
    UBYTE8 *pSource   = &pInterp->sC.pData[XPel + YPel * IBPL];
    HWD16  *pPixels16 = TempBlock + 1;
    HWD16  *pSource16 = TempBlock + 1;
    UBYTE8 *pPixels   = pC->pData;
    int Pixel;
    
    const vc1INTERP_sBicubicFilterParams *FP = &vc1INTERP_Bicubic_Filter_Params_Table[(Y & 3) - 1];
    int Abs, Shift;
    int F0          = FP->F0;
    int F1          = FP->F1;
    int F2          = FP->F2;
    int F3          = FP->F3;

    Shift           = vc1INTERP_Bicubic_Vert_Filter_Shift_Table[(X & 3)][(Y & 3) - 1];
    Abs             = 1 << (Shift - 1);


    /* vertical filter */
    for(J = 0; J < SizeY; J++)
    {
        for(I = -1; I < SizeX+2; I++)
        {
            Pixel = ((  pSource[I - IBPL    ] * F0 +
                        pSource[I           ] * F1 +
                        pSource[I + IBPL    ] * F2 +
                        pSource[I + 2 * IBPL] * F3 +
                        Abs - 1 + R) >> Shift);
            pPixels16[I] = (HWD16)Pixel;
        }
        pSource   += IBPL;
        pPixels16 += PBPL;
    }


    X = X & 3;

    SizeX       = pInterp->SizeX;
    IBPL        = VC1INTERP_MAX_PATCH_SIZE + 3;
    PBPL        = pC->Bpl;
    XPel        = 0;
    YPel        = 0;
    
    FP = &vc1INTERP_Bicubic_Filter_Params_Table[X - 1];
    F0 = FP->F0;
    F1 = FP->F1;
    F2 = FP->F2;
    F3 = FP->F3;

    for(J = 0; J < SizeY; J++)
    {
        for(I = 0; I < SizeX; I++)
        {
            Pixel = ((  pSource16[I - 1] * F0 +
                        pSource16[I    ] * F1 +
                        pSource16[I + 1] * F2 +
                        pSource16[I + 2] * F3 +
                        64 - R) >> 7);
            pPixels[I] = CLIP(Pixel);
        }
        pSource16 += IBPL;
        pPixels   += PBPL;
    }
}

/*
 * Description:
 * Horizontal bicubic interpolation of a patch to quarter pixel position resolution
 *
 * Remarks:
 * None
 *
 * Inputs:
 * pC         - pointer to a component into which the filtered patch will be written
 * pInterp    - pointer to structure containing parameters required for operation
 * X          - X position in quarter pixel units
 * Y          - Y position in quarter pixel units
 *
 * Outputs:
 * pC         - updated with the filtered patch
 *
 * Return Value:
 * None
 *
 */

static void vc1INTERP_InterpPatchQuarterPelBicubicHoriz(    vc1_sComponent * pC, 
                                                            vc1_sInterpolate * pInterp, 
                                                            WORD32 X, 
                                                            WORD32 Y)
{
    int I, J;
    int SizeX       = pInterp->SizeX;
    int SizeY       = pInterp->SizeY;
    int IBPL        = pInterp->sC.Bpl;
    int PBPL        = pC->Bpl;
    int XPel        = X >> 2;
    int YPel        = Y >> 2;
    int R           = pInterp->RndCtrl;
    UBYTE8 *pSource = &pInterp->sC.pData[XPel + YPel * IBPL];
    UBYTE8 *pPixels = pC->pData;
    int Pixel;
    
    const vc1INTERP_sBicubicFilterParams *FP = &vc1INTERP_Bicubic_Filter_Params_Table[(X & 3) - 1];
    int Abs, Shift;
    int F0          = FP->F0;
    int F1          = FP->F1;
    int F2          = FP->F2;
    int F3          = FP->F3;

    Shift           = vc1INTERP_Bicubic_Horiz_Filter_Shift_Table[(X & 3) - 1][0];
    Abs             = 1 << (Shift - 1);

    for(J = 0; J < SizeY; J++)
    {
        for(I = 0; I < SizeX; I++)
        {
            Pixel = ((  pSource[I - 1] * F0 +
                        pSource[I    ] * F1 +
                        pSource[I + 1] * F2 +
                        pSource[I + 2] * F3 +
                        Abs - R)      >> Shift);
            pPixels[I] = CLIP(Pixel);
        }
        pSource += IBPL;
        pPixels += PBPL;
    }
}


/*
 * Description:
 * Copy a patch to an output component.
 *
 * Remarks:
 * Used for predicted blocks at integer positions.
 *
 * Inputs:
 * pC         - pointer to a component into which the filtered patch will be written
 * pInterp    - pointer to structure containing parameters required for operation
 * X          - X position in quarter pixel units
 * Y          - Y position in quarter pixel units
 *
 * Outputs:
 * pC         - updated with the filtered patch
 *
 * Return Value:
 * None
 *
 */

static void vc1INTERP_CopyPatch(vc1_sComponent * pC, vc1_sInterpolate * pInterp, WORD32 X, WORD32 Y)
{
    int I, J;
    int SizeX = pInterp->SizeX;
    int SizeY = pInterp->SizeY;
    int IBPL  = pInterp->sC.Bpl;
    int PBPL  = pC->Bpl;
    int XPel  = X >> 2;
    int YPel  = Y >> 2;
    UBYTE8 *pSource = &pInterp->sC.pData[XPel + YPel * IBPL];
    UBYTE8 *pPixels = pC->pData;

    for(J = 0; J < SizeY; J++)
    {
        for(I = 0; I < SizeX; I++)
        {
            pPixels[I] = pSource[I];
        }
        pSource += IBPL;
        pPixels += PBPL;
    }
}


/*
 * Description:
 * Bicubic interpolation of a patch to quarter pixel position resolution
 *
 * Remarks:
 * None
 *
 * Inputs:
 * pC         - pointer to a component into which the filtered patch will be written
 * pInterp    - pointer to structure containing parameters required for operation
 * X          - X position in quarter pixel units
 * Y          - Y position in quarter pixel units
 *
 * Outputs:
 * pC         - updated with the filtered patch
 *
 * Return Value:
 * None
 *
 */

void vc1INTERP_InterpPatchQuarterPelBicubic(    vc1_sComponent * pC, 
                                                vc1_sInterpolate * pInterp, 
                                                WORD32 X, 
                                                WORD32 Y)
{
    COVERAGE("8.3.6.5.2");
    ASSERT(pInterp->SizeX <= VC1INTERP_MAX_PATCH_SIZE);
    ASSERT(pInterp->SizeY <= VC1INTERP_MAX_PATCH_SIZE);
    ASSERT(pC->Bpl >= pInterp->SizeX);

    DEBUG3(vc1DEBUG_RBLK, "InterpQPelBic: X = %d, Y = %d, Bpl = %d\n", X, Y, pInterp->sC.Bpl);

    if((0 == (X & 3)) && (0 == (Y & 3)))
    {        
        vc1INTERP_CopyPatch(pC, pInterp, X, Y);
    }
    else if(0 == (X & 3))
    {
        vc1INTERP_InterpPatchQuarterPelBicubicVert(pC, pInterp, X, Y);
    }
    else if(0 == (Y & 3))
    {
        vc1INTERP_InterpPatchQuarterPelBicubicHoriz(pC, pInterp, X, Y);
    }
    else
    {
        vc1INTERP_InterpPatchQuarterPelBicubicDiag(pC, pInterp, X, Y);
    }


}

/*
 * Description:
 * Bilinear interpolation of a patch to quarter pixel position resolution
 *
 * Remarks:
 * None
 *
 * Inputs:
 * pC         - pointer to a component into which the filtered patch will be written
 * pInterp    - pointer to structure containing parameters required for operation
 * X          - X position in half pixel units
 * Y          - Y position in half pixel units
 *
 * Outputs:
 * pC         - updated with the filtered patch
 *
 * Return Value:
 * None
 *
 */

void vc1INTERP_InterpPatchQuarterPelBilinear(
    vc1_sComponent * pC,
    vc1_sInterpolate * pInterp,
    WORD32 X,
    WORD32 Y
)
{
    int SizeX   = pInterp->SizeX;
    int SizeY   = pInterp->SizeY;
    int IBPL    = pInterp->sC.Bpl;
    int PBPL    = pC->Bpl;

    int XPel    = X >> 2;
    int YPel    = Y >> 2;
    int XOff    = X & 3;
    int YOff    = Y & 3;

    int A, B, C, D;
    int I, J;
    int R = pInterp->RndCtrl;

    UBYTE8 *pSource = &pInterp->sC.pData[XPel + YPel * IBPL];
    UBYTE8 *pPixels = pC->pData;

    const int F[4] = {4, 3, 2, 1};
    const int G[4] = {0, 1, 2, 3};

    DEBUG7( vc1DEBUG_RBLK, 
            "InterpQPelBil: IBPL=%d, PBPL=%d, XPel=%d, YPel=%d, XOff=%d, YOff=%d, R=%d\n", 
            IBPL, PBPL, XPel, YPel, XOff, YOff, R);

    COVERAGE("8.3.6.5.1");
    
    for(J = 0; J < SizeY; J++)
    {
        for(I = 0; I < SizeX; I++)
        {
            A = pSource[I];
            B = pSource[I + IBPL];
            C = pSource[I + 1];
            D = pSource[I + IBPL + 1];

            pPixels[I] = (UBYTE8)(( F[XOff] * F[YOff] * A +
                                    F[XOff] * G[YOff] * B +
                                    G[XOff] * F[YOff] * C +
                                    G[XOff] * G[YOff] * D + 8 - R) >> 4);
        }
        pSource += IBPL;
        pPixels += PBPL;
    }
}

/*
 * Description:
 * Bilinear interpolation of a patch to half pixel position resolution
 *
 * Remarks:
 * Implemented using the quarter pixel resolution routines.
 *
 * Inputs:
 * pC         - pointer to a component into which the filtered patch will be written
 * pInterp    - pointer to structure containing parameters required for operation
 * X          - X position in half pixel units
 * Y          - Y position in half pixel units
 *
 * Outputs:
 * pC         - updated with the filtered patch
 *
 * Return Value:
 * None
 *
 */

void vc1INTERP_InterpPatchHalfPelBilinear(  vc1_sComponent * pC, 
                                            vc1_sInterpolate * pInterp, 
                                            WORD32 X, 
                                            WORD32 Y)
{
    DEBUG2(vc1DEBUG_RBLK, "InterpHPelBil: X = %d, Y = %d\n", X, Y);
    vc1INTERP_InterpPatchQuarterPelBilinear(pC, pInterp, X * 2, Y * 2);
}




/*
 * Description:
 * Bicubic interpolation of a patch to half pixel position resolution
 *
 * Remarks:
 * Implemented using the quarter pixel resolution routines.
 *
 * Inputs:
 * pC         - pointer to a component into which the filtered patch will be written
 * pInterp    - pointer to structure containing parameters required for operation
 * X          - X position in half pixel units
 * Y          - Y position in half pixel units
 *
 * Outputs:
 * pC         - updated with the filtered patch
 *
 * Return Value:
 * None
 *
 */

void vc1INTERP_InterpPatchHalfPelBicubic(   vc1_sComponent * pC, 
                                            vc1_sInterpolate * pInterp, 
                                            WORD32 X,
                                            WORD32 Y)
{
    vc1INTERP_InterpPatchQuarterPelBicubic(pC, pInterp, X * 2, Y * 2);
}


/*
 * Description:
 * Interlace a predicted macroblock
 *
 * Remarks:
 * Interleaves the top blocks with the bottom blocks, and writes out four
 * interlaced blocks.
 *
 * Inputs:
 * pDest     - pointer to initialised buffer to hold the new data
 * pSourceMB - pointer to the source macroblock data
 *
 * Outputs:
 * pDest     - updated with new interlaced data
 *
 * Return Value:
 * None
 *
 */

void vc1INTERP_InterlacePredMB(UBYTE8 pDestMB[6][64], const UBYTE8 pSourceMB[6][64])
{
    int I, K, Blk;
    const UBYTE8 *pSource;
    UBYTE8 *pDest, *pDestStart;

    for(Blk = 0; Blk < VC1_BLOCKS_PER_MB; Blk++)
    {
        pSource = pSourceMB[Blk];

        if((vc1_BlkY0 == Blk) || (vc1_BlkY1 == Blk))
        {
            /*
             * write the first four lines of the top source blocks into the top dest blocks'
             * top field, then the second four lines of the top source blocks into the bottom
             * dest blocks' top field.
             *
             */
            for(K = 0; K <= 2; K += 2)
            {
                pDestStart = pDestMB[Blk + K];
                for(pDest = pDestStart; pDest < pDestStart + 4 * 16; pDest += 16)
                {
                    for(I = 0; I < 8; I++)
                    {
                        pDest[I] = *pSource++;
                    }
                }
            }
        }
        else if((vc1_BlkY2 == Blk) || (vc1_BlkY3 == Blk))
        {
            /*
             * write the first four lines of the bottom source blocks into the top dest blocks'
             * bottom field, then the second four lines of the bottom source blocks into the
             * bottom dest blocks' bottom field
             */
            for(K = 2; K >= 0; K -= 2)
            {
                pDestStart = &pDestMB[Blk - K][8];
                for(pDest = pDestStart; pDest < pDestStart + 4 * 16; pDest += 16)
                {
                    for(I = 0; I < 8; I++)
                    {
                       pDest[I] = *pSource++;
                    }
                }
            } 
        }
        else
        {
            /* copy chroma blocks */
            pSource = pSourceMB[vc1_BlkCr];
            pDest = pDestMB[vc1_BlkCr];
            for(I = 0; I < 64; I++)
            {
                *pDest++ = *pSource++;
            }

            pSource = pSourceMB[vc1_BlkCb];
            pDest = pDestMB[vc1_BlkCb];
            for(I = 0; I < 64; I++)
            {
                *pDest++ = *pSource++;
            }
        }
    }
}


/*
 * Description:
 * Interlace a differential macroblock
 *
 * Remarks:
 * Interleaves the top blocks with the bottom blocks, and writes out four
 * interlaced blocks.
 *
 * Inputs:
 * pDest     - pointer to initialised buffer to hold the new data
 * pSourceMB - pointer to the source macroblock data
 *
 * Outputs:
 * pDest     - updated with new interlaced data
 *
 * Return Value:
 * None
 *
 */

void vc1INTERP_InterlaceDiffMB(HWD16 pDestMB[6][64], const HWD16 pSourceMB[6][64])
{
    int I, K, Blk;
    const HWD16 *pSource;
    HWD16 *pDest, *pDestStart;

    for(Blk = 0; Blk < VC1_BLOCKS_PER_MB; Blk++)
    {
        pSource = pSourceMB[Blk];

        if((vc1_BlkY0 == Blk) || (vc1_BlkY1 == Blk))
        {
            /*
             * write the first four lines of the top source blocks into the top dest blocks'
             * top field, then the second four lines of the top source blocks into the bottom
             * dest blocks' top field.
             *
             */
            for(K = 0; K <= 2; K += 2)
            {
                pDestStart = pDestMB[Blk + K];
                for(pDest = pDestStart; pDest < pDestStart + 4 * 16; pDest += 16)
                {
                    for(I = 0; I < 8; I++)
                    {
                        pDest[I] = *pSource++;
                    }
                }
            }
        }
        else if((vc1_BlkY2 == Blk) || (vc1_BlkY3 == Blk))
        {
            /*
             * write the first four lines of the bottom source blocks into the top dest blocks'
             * bottom field, then the second four lines of the bottom source blocks into the
             * bottom dest blocks' bottom field
             */
            for(K = 2; K >= 0; K -= 2)
            {
                pDestStart = &pDestMB[Blk - K][8];
                for(pDest = pDestStart; pDest < pDestStart + 4 * 16; pDest += 16)
                {
                    for(I = 0; I < 8; I++)
                    {
                       pDest[I] = *pSource++;
                    }
                }
            } 
        }
        else
        {
            /* copy chroma blocks */
            pSource = pSourceMB[vc1_BlkCr];
            pDest = pDestMB[vc1_BlkCr];
            for(I = 0; I < 64; I++)
            {
                *pDest++ = *pSource++;
            }

            pSource = pSourceMB[vc1_BlkCb];
            pDest = pDestMB[vc1_BlkCb];
            for(I = 0; I < 64; I++)
            {
                *pDest++ = *pSource++;
            }
        }
    }
}

/*
 * Description:
 * Obtain an interpolated block from a reference picture, using a macroblocks MV information
 *
 * Remarks:
 * Determines which interpolation function to call given the information in the 
 * interpolation structure.
 *
 * Inputs:
 * pPredBlk   - pointer to an array into which the filtered block will be written
 * pPosition  - pointer to a position structure indicating the current macroblock
 * eBlk       - number of the block from which to obtain the motion vector
 * RndCtrl    - rounding control flag from picture layer
 * Backwards  - if 1, predict from the new reference picture, else the old
 *
 * Outputs:
 * pPredBlk   - updated with the filtered patch
 *
 * Return Value:
 * None
 *
 */

static void vc1INTERP_InterpolateBlock(
    UBYTE8 * pPredBlk,
    vc1_sPosition * pPosition,
    int eBlk,
    FLAG RndCtrl,
    int Backwards
)
{
    int X, Y, IX, IY;
    vc1_sInterpolate sInterp;
    vc1_sComponent sCout;
    vc1_sMB *pMB = pPosition->pCurMB;
    vc1_sReferencePicture *pRefPic;
    vc1_sMV sMV;
    int Count, InterpLim;
    int F = 0;
    int Bpl;
    int FieldMV = vc1_MBTypeIsFieldMV(pMB->eMBType);

    /* Find number of interpolations for this 8x8 block */
    sInterp.RndCtrl = RndCtrl;
    sInterp.SizeX = 8;
    sInterp.SizeY = 8;
    InterpLim = 1;
    if (pPosition->ePictureFormat==vc1_InterlacedFrame && vc1_BlkNumIsChroma(eBlk))
    {
        /* Split the chroma block into four subblocks */
        InterpLim = 4;
        sInterp.SizeX = 4;
        sInterp.SizeY = 4;
    }

    /* Loop over a single 8x8 block or four 4x4 subblocks */
    for (Count = 0; Count < InterpLim; Count++)
    {
        /* Find macroblock position in macroblocks */
        X = pPosition->X;
        Y = pPosition->Y + pPosition->SliceY;

        /* Find interpolation position in (X,Y) 1/4 pixel and field F */
        if (vc1_BlkNumIsLuma(eBlk))
        {
            sMV = pMB->sBlk[eBlk].u.sInter.sMotion[Backwards].sMV;

            /* Display the MV */
            DEBUG5(vc1DEBUG_RBLK, "InterpBlk[%d]: MV X = %d, Y = %d, F = %d Back = %d\n", eBlk,
                sMV.X, sMV.Y, sMV.BottomField, Backwards);

            /* Convert to coorect coordinate system for interlaced field */
            if (pPosition->ePictureFormat==vc1_InterlacedField &&
                sMV.BottomField!=pPosition->BottomField)
            {
                /* convert Y coordinate to opposite field */
                sMV.Y = (HWD16)(sMV.Y + 2-4*sMV.BottomField);
                DEBUG4(vc1DEBUG_RBLK, "InterpBlk[%d]: FDMV X = %d, Y = %d, F = %d\n", eBlk,
                    sMV.X, sMV.Y, sMV.BottomField);
            }

            /* Crop motion vector to image */
            vc1CROPMV_LumaPullBack(pPosition, &sMV);

            X = 4 * 16 * X + sMV.X;
            Y = 4 * 16 * Y + sMV.Y;
            IX = X >> 2;
            IY = Y >> 2;
            if (FieldMV)
            {
                F  = IY & 1;
                IY = IY >> 1;
            }
            else
            {
                F = sMV.BottomField;
            }

            DEBUG4(vc1DEBUG_RBLK, "InterpBlk[%d]: MBMV IX = %d, IY = %d, F = %d\n", eBlk,
                IX, IY, F);
        
            if ((vc1_BlkY1 == eBlk) || (vc1_BlkY3 == eBlk))
            {
                IX += 8;    /* add a block width */
            }

            if ((vc1_BlkY2 == eBlk) || (vc1_BlkY3 == eBlk))
            {
                if (FieldMV)
                {
                    if (F==1)
                    {
                        IY ++;
                    }
                    F ^= 1;
                }
                else
                {
                    IY += 8;    /* add a block height */
                }
            }
        }
        else /* Chroma */
        {
            /* Switch MV */
            if (vc1_MBTypeIsSwitchMV(pMB->eMBType) && Count==2)
            {
                Backwards = 1-Backwards;
            }

            /* derive the chroma motion vectors from the available luma motion vectors */
            vc1DERIVEMV_DeriveChromaMV(&sMV, pPosition, Count, Backwards);
            DEBUG6(vc1DEBUG_RBLK, "InterpBlk[%d][%d]: MV X = %d, Y = %d, F = %d, Back = %d\n",
                eBlk, Count, sMV.X, sMV.Y, sMV.BottomField, Backwards);

            /* Crop vector to image */
            vc1CROPMV_ChromaPullBack(pPosition, &sMV);

            X = 4 * 8 * X + sMV.X;
            Y = 4 * 8 * Y + sMV.Y;
            IX = X >> 2;
            IY = Y >> 2;
            if (FieldMV)
            {
                F  = IY & 1;
                IY = IY >> 1;
            }
            else
            {
                F = sMV.BottomField;
            }

            DEBUG5(vc1DEBUG_RBLK, "InterpBlk[%d][%d]: MBMV IX = %d, IY = %d, F = %d\n",
                eBlk, Count, IX, IY, F);

            if (Count & 1)
            {
                IX += 4;
            }
            if (Count & 2)
            {
                if (FieldMV)
                {
                    if (F==1)
                    {
                        IY ++;
                    }
                    F ^= 1;
                }
                else
                {
                    IY += 4;
                }
            }
        }

        DEBUG4(vc1DEBUG_RBLK, "InterpBlk[%d]: BLMV IX = %d, IY = %d, F = %d\n", eBlk, IX, IY, F);
        X = 4*IX + (X & 3);
        Y = 4*IY + (Y & 3);
        DEBUG4(vc1DEBUG_RBLK, "InterpBlk[%d]: BLMV X = %d, Y = %d, F = %d\n", eBlk, X, Y, F);

        /* Find the source reference picture */
        if (Backwards)
        {
            pRefPic  = pPosition->pReferenceNew;
        }
        else /* Forwards */
        {
            pRefPic  = pPosition->pReferenceOld;
            if (   pPosition->ePictureFormat == vc1_InterlacedField
                && pPosition->SecondField == TRUE
                && pPosition->BottomField != F)
            {
                /* Second field, other, forward comes from current frame */
                if (pPosition->ePictureType==vc1_PictureTypeB)
                {
                    pRefPic = pPosition->pReferenceB;
                }
                else
                {
                    pRefPic = pPosition->pReferenceNew;
                }
            }
        }

        /* Set the interpolation origin */
        switch (eBlk)
        {
            case vc1_BlkCb:
                sInterp.sC.pData    = pRefPic->pImageU;
                Bpl                 = pRefPic->sU.Bpl;
                break;

            case vc1_BlkCr:
                sInterp.sC.pData    = pRefPic->pImageV;
                Bpl                 = pRefPic->sV.Bpl;
                break;

            default:
                sInterp.sC.pData    = pRefPic->pImageY;
                Bpl                 = pRefPic->sY.Bpl;
        }

        /* Select field for field motion vectors */
        if (pPosition->ePictureFormat == vc1_InterlacedField || FieldMV)
        {
            if (F)
            {
                sInterp.sC.pData += Bpl;
            }
            Bpl *= 2;
        }
        sInterp.sC.Bpl = Bpl;

        /* Debug information */
        DEBUG2(vc1DEBUG_RBLK, "InterpBlk: MBX = %d, MBY = %d\n",
            pPosition->X, pPosition->Y + pPosition->SliceY);
        if(pRefPic == pPosition->pReferenceNew)
        {
            DEBUG0(vc1DEBUG_RBLK, "pRefPic == pReferenceNew\n");
        }
        else if (pRefPic == pPosition->pReferenceOld)
        {
            DEBUG0(vc1DEBUG_RBLK, "pRefPic == pReferenceOld\n");
        }
        else if (pRefPic == pPosition->pReferenceB)
        {
            DEBUG0(vc1DEBUG_RBLK, "pRefPic == pReferenceB\n");
        }
        else
        {
            DEBUG0(vc1DEBUG_RBLK, "pRefPic == Unknown\n");
        }
        DEBUG4(vc1DEBUG_RBLK, "InterpBlk: X = %d, Y = %d, F = %d, ePictureType = %s\n", 
            X, Y, F, vc1DEBUG_PictureType[pPosition->ePictureType]);

        /* Configure output 8x8 buffer */
        sCout.Bpl    = 8;
        if (vc1_BlkNumIsChroma(eBlk) && FieldMV)
        {
            sCout.Bpl = 16;
        }

        sCout.pData  = pPredBlk;
        if (Count & 1)
        {
            sCout.pData += 4;
        }
        if (Count & 2)
        {
            if (FieldMV)
            {
                sCout.pData += 8;
            }
            else
            {
                sCout.pData += 8*4;
            }
        }

        /* Interpolate */
        if (vc1_BlkNumIsChroma(eBlk))
        {
            vc1INTERP_InterpPatchQuarterPelBilinear(&sCout, &sInterp, X, Y);
        }
        else /* Luma */
        {
            switch(pPosition->eMVMode)
            {
                case vc1_MVMode1MVHalfPelBilinear:
                    vc1INTERP_InterpPatchHalfPelBilinear(&sCout, &sInterp, X/2, Y/2);
                    break;

                case vc1_MVMode1MVHalfPel:
                    vc1INTERP_InterpPatchHalfPelBicubic(&sCout, &sInterp, X/2, Y/2);
                    break;

                case vc1_MVMode1MV:
                case vc1_MVModeMixedMV:
                    vc1INTERP_InterpPatchQuarterPelBicubic(&sCout, &sInterp, X, Y);
                    break;

                default:
                    FATAL("InterpolateBlock: Unknown eMVMode - %d\n", pPosition->eMVMode);
            }
        }

    } /* Count loop */
}

/*
 * Description:
 * Merge the data from two predicted blocks into one
 *
 * Remarks:
 * Per-pixel average operation, with upwards rounding. 8x8 array assumed.
 *
 * Inputs:
 * pResult  - pointer to array into which result will be written
 * pBlkA    - pointer to array containing the first input block
 * pBlkB    - pointer to array containing the second input block
 *
 * Outputs:
 * pResult  - updated with new block
 *
 * Return Value:
 * None
 *
 */

static void vc1INTERP_AverageBlocks(UBYTE8 * pResult, UBYTE8 * pBlkA, UBYTE8 * pBlkB)
{
    int Count;

    for(Count = 0; Count < 8 * 8; Count++)
    {
        *pResult = (UBYTE8)((*pBlkA + *pBlkB + 1)>>1);
        pResult++;
        pBlkA++;
        pBlkB++;
    }

    COVERAGE("10.4.4.7");
}

/*
 * Description:
 * Predict macroblock from a reference picture
 *
 * Remarks:
 * Uses the macroblock type and motion vectors to form a prediction
 *
 * Inputs:
 * pPredBlk   - pointer to an array into which the filtered blocks will be written
 * pPosition  - pointer to a position structure indicating the current macroblock
 * RndCtrl    - rounding control flag from picture layer
 *
 * Outputs:
 * pPredBlk   - updated with the filtered blocks
 *
 * Return Value:
 * None
 *
 */
void vc1INTERP_PredictMB(
    UBYTE8 pPredBlk[6][64],
    vc1_sPosition * pPosition,
    FLAG RndCtrl
)
{
    vc1_sMB *pMB = pPosition->pCurMB;
    vc1_eMBType eMBType = pMB->eMBType;
    int Blk, Switch;

    /* store luma motion vectors to mv history buffer */
    vc1DERIVEMV_StoreMotionVectors(pPosition);

    DEBUGMB(vc1DEBUG_RBLK, pPosition);

    if(FALSE == vc1_MBTypeIsIntra(eMBType))
    {
        /* set up any motion vectors needed for B pictures */
        vc1DERIVEMV_DeriveMV(pPosition);

        for (Blk = 0; Blk < VC1_BLOCKS_PER_MB; Blk++)
        {
            if (vc1_BlkTypeIsIntra(pMB->sBlk[Blk].eBlkType))
            {
                continue;
            }

            Switch = 0;
            if (vc1_MBTypeIsSwitchMV(pMB->eMBType))
            {
                if((2 == Blk) || (3 == Blk))
                {
                    Switch = 1;
                }
            }

            switch(eMBType & vc1_MBDirMask)
            {
                case vc1_MBForward:
                    vc1INTERP_InterpolateBlock(pPredBlk[Blk], pPosition, Blk, RndCtrl, Switch - 0);
                    DEBUG0(vc1DEBUG_RBLK, "Forward block...\n");
                    DEBUGRECT8(vc1DEBUG_RBLK, pPredBlk[Blk], 8, 8, 8);
                    break;

                case vc1_MBBackward:
                    vc1INTERP_InterpolateBlock(pPredBlk[Blk], pPosition, Blk, RndCtrl, 1 - Switch);
                    DEBUG0(vc1DEBUG_RBLK, "Backward block...\n");
                    DEBUGRECT8(vc1DEBUG_RBLK, pPredBlk[Blk], 8, 8, 8);
                    break;

                default: /* Direct or interpolated */
                {
                    UBYTE8 BlkA[64];
                    UBYTE8 BlkB[64];

                    vc1INTERP_InterpolateBlock(BlkA, pPosition, Blk, RndCtrl, 0);
                    DEBUG0(vc1DEBUG_RBLK, "Forward block...\n");
                    DEBUGRECT8(vc1DEBUG_RBLK, BlkA, 8, 8, 8);
                    vc1INTERP_InterpolateBlock(BlkB, pPosition, Blk, RndCtrl, 1);
                    DEBUG0(vc1DEBUG_RBLK, "Backward block...\n");
                    DEBUGRECT8(vc1DEBUG_RBLK, BlkB, 8, 8, 8);
                    vc1INTERP_AverageBlocks(pPredBlk[Blk], BlkA, BlkB); /* average the result */
                    COVERAGE("8.4.5.3");
                    break;
                }
            }
        }
    }

    /* Increment history buffer */
    (pPosition->pMVHist)++;
}












