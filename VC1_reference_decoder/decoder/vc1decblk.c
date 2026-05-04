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
 * vc1decblk.c:
 * Bitstream block unpack and decode functions
 *
 */

#include "vc1types.h"
#include "vc1dec.h"
#include "vc1decbit.h"
#include "vc1decblk.h"
#include "vc1decblktab.h"
#include "vc1preddcac.h"
#include "vc1debug.h"
#include "vc1dec3dh.h"
#include "vc1itrans.h"
#include "vc1deczz.h"
#include "vc1iquant.h"
#include "vc1smooth.h"
#include "vc1tools.h"
#include "vc1recon.h"
#include "vc1interp.h"


/* table 46 */
static const vc1DEC_sVLCCode vc1DECBLK_Block_Transform_Type_Index_PQUANT_1_4[9] =
{
    {0, 8, 5},
    {0, 2, vc1_SBP8x4Both},     {1, 2, vc1_SBP4x8Both},
    {3, 2, vc1_SBP8x8},         {5, 3, vc1_SBP4x4},
    {16, 5, vc1_SBP8x4Top},     {17, 5, vc1_SBP8x4Bottom},
    {18, 5, vc1_SBP4x8Right},   {19, 5, vc1_SBP4x8Left}
};

/* table 47 */
static const vc1DEC_sVLCCode vc1DECBLK_Block_Transform_Type_Index_PQUANT_5_12[9] =
{
    {0, 8, 4},
    {3, 2, vc1_SBP8x8},         {0, 3, vc1_SBP4x8Right},
    {1, 3, vc1_SBP4x8Left},     {2, 3, vc1_SBP4x4},
    {3, 3, vc1_SBP8x4Both},     {5, 3, vc1_SBP4x8Both},
    {8, 4, vc1_SBP8x4Bottom},   {9, 4, vc1_SBP8x4Top}
};

/* table 48 */
static const vc1DEC_sVLCCode vc1DECBLK_Block_Transform_Type_Index_PQUANT_13_31[9] =
{
    {0, 8, 4},
    {1, 2, vc1_SBP8x8},         {0, 3, vc1_SBP4x8Both},
    {1, 3, vc1_SBP4x4},         {4, 3, vc1_SBP8x4Bottom},
    {6, 3, vc1_SBP4x8Right},    {7, 3, vc1_SBP4x8Left},
    {10, 4, vc1_SBP8x4Both},    {11, 4, vc1_SBP8x4Top}
};

/* table 49 */
static const vc1DEC_sVLCCode vc1DECBLK_Sub_Block_Pattern_4x4_PQUANT_1_4[16] =
{
    {0, 15, 6},
    {1, 1, 15},     {0, 4, 11},     {1, 4, 13},     {2, 4, 7},
    {6, 5, 12},     {7, 5, 3},      {8, 5, 10},     {9, 5, 5},
    {10, 5, 8},     {11, 5, 4},     {12, 5, 2},     {14, 5, 1},
    {15, 5, 14},    {26, 6, 6},     {27, 6, 9}
};

/* table 50 */
static const vc1DEC_sVLCCode vc1DECBLK_Sub_Block_Pattern_4x4_PQUANT_5_12[16] =
{
    {0, 15, 5},
    {1, 2, 15},     {0, 3, 2},      {3, 4, 12},     {8, 4, 3},
    {9, 4, 10},     {10, 4, 5},     {13, 4, 8},     {14, 4, 1},
    {15, 4, 4},     {4, 5, 6},      {5, 5, 9},      {22, 5, 14},
    {23, 5, 7},     {24, 5, 13},    {25, 5, 11}
};

/* table 51 */
static const vc1DEC_sVLCCode vc1DECBLK_Sub_Block_Pattern_4x4_PQUANT_13_31[16] =
{
    {0, 15, 5},
    {2, 3, 4},      {3, 3, 8},      {5, 3, 1},      {6, 3, 2},
    {1, 4, 12},     {2, 4, 3},      {3, 4, 10},     {8, 4, 5},
    {15, 4, 15},    {0, 5, 6},      {1, 5, 9},      {18, 5, 14},
    {19, 5, 13},    {28, 5, 7},     {29, 5, 11}
};

/* table 52 */
static const vc1DEC_sVLCCode vc1DECBLK_Sub_Block_Pattern_8x4_4x8[4] =
{
    {0, 3, 2},
    {0, 1, 3},      {2, 2, 1},      {3, 2, 2}
};

/*
 * Description:
 * Unpack the DC differential value from an intra block
 *
 * Remarks:
 * None.
 *
 * Inputs:
 * pState       - pointer to the decoder's state
 * pBitstream   - pointer to a struct representing the current bitstream position
 * DCDiffResult - pointer to a location into which the DC differential will be written
 * Blk          - block number of the DC differential
 *
 * Outputs:
 * DCDiffResult - updated with DC differential value.
 * pBitstream   - updated with new bitstream position.
 *
 * Return Value:
 * Standard vc1_eResult result. See enumeration for possible result codes.
 * 
 */

static vc1_eResult vc1DECBLK_UnpackDCDifferential(vc1DEC_sState * pState,
                                                  vc1DEC_sBitstream * pBitstream,
                                                  HWD16 * DCDiffResult,
                                                  int Blk)
{
    vc1_sPosition *pPosition = &pState->sPosition;
    const vc1DEC_sVLCCode *pTable;
    UWORD32 Value;
    HWD16   DCDifferential;

    COVERAGE("8.1.1.2");
    
    if(TRUE == vc1_BlkNumIsLuma(Blk))
    {
        if(0 == pState->sPicParams.IntraTransformDCTable)
        {
            pTable = vc1DEC_Low_Mot_Luminance_DC_Diff_VLC;
        }
        else
        {
            pTable = vc1DEC_High_Mot_Luminance_DC_Diff_VLC;
        }
    }
    else                                            /* chroma block */
    {
        if(0 == pState->sPicParams.IntraTransformDCTable)
        {
            pTable = vc1DEC_Low_Mot_Chroma_DC_Diff_VLC;
        }
        else
        {
            pTable = vc1DEC_High_Mot_Chroma_DC_Diff_VLC;
        }
    }

    DCDifferential = (HWD16)vc1DECBIT_GetVLC(pBitstream, pTable);
    if(VC1DECBIT_EOF == DCDifferential)
    {
        return(vc1_ResultBufferExhausted); /* Could be code not found */
    }

    DEBUG1(vc1DEBUG_BLK, "DCDiff VLC = %d\n", DCDifferential);

    COVERAGE("7.1.4.3");
    if(0 != DCDifferential)
    {
        COVERAGE_NONINT_INT("8.1.1.7","10.1.2.1");

        if(ESCAPE == DCDifferential)
        {
            UBYTE8 BitsToGet;

            DEBUG0(vc1DEBUG_BLK, "Escaped DC differential\n");

            switch(pPosition->pCurMB->sQuant.Quant)
            {
                case 1:     BitsToGet = 10; break;
                case 2:     BitsToGet = 9; break;
                default:    BitsToGet = 8;
            }

            DEBUG1(vc1DEBUG_BLK, "Getting %d bits\n", BitsToGet);

            DCDifferential = (HWD16)vc1DECBIT_GetBits(pBitstream, BitsToGet);
            if(VC1DECBIT_EOF == DCDifferential)
            {
                return(vc1_ResultBufferExhausted);
            }
            COVERAGE("7.1.4.4");
        }
        else
        {
            if(1 == pPosition->pCurMB->sQuant.Quant)
            {
                Value = vc1DECBIT_GetBits(pBitstream, 2);
                if(VC1DECBIT_EOF == Value)
                {
                    return(vc1_ResultBufferExhausted);
                }

                COVERAGE("7.1.4.5");
                DCDifferential = (HWD16)(DCDifferential * 4 + Value - 3);
            }
            else if(2 == pPosition->pCurMB->sQuant.Quant)
            {
                Value = vc1DECBIT_GetBits(pBitstream, 1);
                if(VC1DECBIT_EOF == Value)
                {
                    return(vc1_ResultBufferExhausted);
                }
                COVERAGE("7.1.4.6");
                DCDifferential = (HWD16)(DCDifferential * 2 + Value - 1);
            }
        }
        Value = vc1DECBIT_GetBits(pBitstream, 1);
        if(VC1DECBIT_EOF == Value)
        {
            return(vc1_ResultBufferExhausted);
        }
        COVERAGE("7.1.4.7");
        if(1 == Value)
        {
            DCDifferential = (HWD16)(-DCDifferential);
        }
    }

    *DCDiffResult = DCDifferential;

    DEBUG1(vc1DEBUG_BLK, "DCDifferential = 0x%04x\n", DCDifferential);

    return(vc1_ResultOK);
}


/*
 * Description:
 * Apply predicted AC coefficients to a block.
 *
 * Remarks:
 * None.
 *
 * Inputs:
 * pCoefs       - pointer to the coefficients of the block
 * pPred        - pointer to the predicted AC coeffients
 * eBlkType     - type of the block containing the coefficents.
 *
 * Outputs:
 * pCoefs       - updated with new AC coefficients.
 *
 * Return Value:
 * Standard vc1_eResult result. See enumeration for possible result codes.
 * 
 */

static vc1_eResult vc1DECBLK_ApplyACPrediction(HWD16 * pCoefs,
                                               HWD16 * pPred,
                                               vc1_eBlkType eBlkType)
{
    int i, stride;

    COVERAGE("8.1.1.13");
    
    if(vc1_BlkIntraTop == eBlkType)
    {
        stride = 1;
    }
    else if(vc1_BlkIntraLeft == eBlkType)
    {
        stride = 8;
    }
    else
    {
        /* should be an edge block - ignore the prediction data */
        return(vc1_ResultOK);
    }

    for(i = 1; i < 8; i++)
    {
        pCoefs[i * stride] = (HWD16)(pCoefs[i * stride] + pPred[i]);
    }

    return(vc1_ResultOK);
}

/*
 * Description:
 * Unpack the block transform type.
 *
 * Remarks:
 * None.
 *
 * Inputs:
 * pState       - pointer to the decoder's state
 * pBitstream   - pointer to struct representing the current bitstream
 * eBlk         - block number of the current block
 *
 * Outputs:
 * pBitstream   - updated with new bitstream position.
 *
 * Return Value:
 * Standard vc1_eResult result. See enumeration for possible result codes.
 * 
 */

static vc1_eResult vc1DECBLK_UnpackTTBLK(vc1DEC_sState * pState,
                                         vc1DEC_sBitstream * pBitstream,
                                         int eBlk)
{
    vc1_sPosition *pPosition = &pState->sPosition;
    WORD32 Value;
    const vc1DEC_sVLCCode *pTable;
    vc1_sMB *pMB = pPosition->pCurMB;
    vc1_NumZeroCoef *pNZC = pMB->sBlk[eBlk].u.sInter.NZC;
    vc1_eBlkType *pBlkType = &pMB->sBlk[eBlk].eBlkType;

    if(pState->sPicParams.PQuant < 5)
    {
        pTable = vc1DECBLK_Block_Transform_Type_Index_PQUANT_1_4;
    }
    else if(pState->sPicParams.PQuant < 13)
    {
        pTable = vc1DECBLK_Block_Transform_Type_Index_PQUANT_5_12;
    }
    else
    {
        pTable = vc1DECBLK_Block_Transform_Type_Index_PQUANT_13_31;
    }

    Value = vc1DECBIT_GetVLC(pBitstream, pTable);
    if(VC1DECBIT_EOF == Value)
    {
        return(vc1_ResultBufferExhausted);
    }
    DEBUG2(vc1DEBUG_BLK, "TTBLK = %s (%d)\n", vc1DEBUG_SBPType[Value], Value);

    pNZC[0] = 0;
    pNZC[1] = 0;
    pNZC[2] = 0;
    pNZC[3] = 0;

    switch(Value)
    {
        case vc1_SBP8x8:
            pNZC[0] = 1;
            *pBlkType = vc1_BlkInter8x8;
            break;

        case vc1_SBP8x4Bottom:
            pNZC[1] = 1;
            *pBlkType = vc1_BlkInter8x4;
            break;

        case vc1_SBP8x4Top:
            pNZC[0] = 1;
            *pBlkType = vc1_BlkInter8x4;
            break;

        case vc1_SBP8x4Both:
            pNZC[0] = 1;
            pNZC[1] = 1;
            *pBlkType = vc1_BlkInter8x4;
            break;

        case vc1_SBP4x8Right:
            pNZC[1] = 1;
            *pBlkType = vc1_BlkInter4x8;
            break;

        case vc1_SBP4x8Left:
            pNZC[0] = 1;
            *pBlkType = vc1_BlkInter4x8;
            break;

        case vc1_SBP4x8Both:
            pNZC[0] = 1;
            pNZC[1] = 1;
            *pBlkType = vc1_BlkInter4x8;
            break;

        case vc1_SBP4x4:
            pNZC[0] = 1;
            pNZC[1] = 1;
            pNZC[2] = 1;
            pNZC[3] = 1;
            *pBlkType = vc1_BlkInter4x4;
            break;

        default:
            FATAL("UnpackTTMB: Unknown SBP - %d\n", Value);
            return(vc1_ResultFatal);
    }

    DEBUG2(vc1DEBUG_BLK, "sBlk[%d].eBlkType = %s\n", eBlk, vc1DEBUG_BlkType[*pBlkType]);

    COVERAGE("7.1.4.1");
    return(vc1_ResultOK);
}

/*
 * Description:
 * Unpack a subblock pattern and fill in the NZC values of a block.
 *
 * Remarks:
 * None.
 *
 * Inputs:
 * pState       - pointer to the decoder's state
 * pBitstream   - pointer to struct representing the current bitstream
 * eBlk         - block number of current block
 *
 * Outputs:
 * pBitstream   - updated with new bitstream position.
 *
 * Return Value:
 * Standard vc1_eResult result. See enumeration for possible result codes.
 * 
 */

static vc1_eResult vc1DECBLK_UnpackSubBlockPattern(vc1DEC_sState * pState,
                                                   vc1DEC_sBitstream * pBitstream,
                                                   int eBlk)
{
    vc1_sPosition *pPosition = &pState->sPosition;
    vc1_sBlk *pBlk          = &pPosition->pCurMB->sBlk[eBlk];
    const vc1DEC_sVLCCode *pTable;
    WORD32 Value;

    if(pState->sPicParams.PQuant < 5)
    {
        pTable = vc1DECBLK_Sub_Block_Pattern_4x4_PQUANT_1_4;
    }
    else if(pState->sPicParams.PQuant < 13)
    {
        pTable = vc1DECBLK_Sub_Block_Pattern_4x4_PQUANT_5_12;
    }
    else
    {
        pTable = vc1DECBLK_Sub_Block_Pattern_4x4_PQUANT_13_31;
    }

    switch(pBlk->eBlkType)
    {
        case vc1_BlkInter4x8:   /* fall through */
        case vc1_BlkInter8x4:
        {
            Value = vc1DECBIT_GetVLC(pBitstream, vc1DECBLK_Sub_Block_Pattern_8x4_4x8);
            if(VC1DECBIT_EOF == Value)
            {
                return(vc1_ResultBufferExhausted);
            }

            DEBUG1(vc1DEBUG_BLK, "Sub-block pattern: %d\n", Value);

            /* assume they're both coded */
            pBlk->u.sInter.NZC[0] = 1;
            pBlk->u.sInter.NZC[1] = 1;

            if(1 == Value)
            {
                pBlk->u.sInter.NZC[0] = 0;
            }
            else if(2 == Value)
            {
                pBlk->u.sInter.NZC[1] = 0;
            }

            break;
        }
        case vc1_BlkInter4x4:
        {
            int Count;

            Value = vc1DECBIT_GetVLC(pBitstream, pTable);
            if(VC1DECBIT_EOF == Value)
            {
                return(vc1_ResultBufferExhausted);
            }

            DEBUG1(vc1DEBUG_BLK, "Sub-block pattern: %d\n", Value);

            /* extract bits from sub block pattern, and put into the block's NZC flags */
            for(Count = 0; Count < 4; Count++)
            {
                if(0 != (Value & (1 << (3 - Count))))
                {
                    pBlk->u.sInter.NZC[Count] = 1;
                }
                else
                {
                    pBlk->u.sInter.NZC[Count] = 0;
                }
            }
            break;
        }
        default:
            FATAL("UnpackSubBlockPattern: Unknown block type %d (0x%x)\n",
                   pBlk->eBlkType,
                   pBlk->eBlkType);
            return(vc1_ResultFatal);
    }

    COVERAGE("7.1.4.2");
    return(vc1_ResultOK);
}

/*
 * Description:
 * Unpack an inter coded block
 *
 * Remarks:
 * None.
 *
 * Inputs:
 * pTCoefs      - pointer to area into which coefficients will be written
 * pState       - pointer to the decoder's state
 * pBitstream   - pointer to struct representing the current bitstream
 * eBlk         - block number of current block
 *
 * Outputs:.
 * pTCoefs      - updated with coefficients
 * pBitstream   - updated with new bitstream position
 *
 * Return Value:
 * Standard vc1_eResult result. See enumeration for possible result codes.
 * 
 */

static vc1_eResult vc1DECBLK_UnpackInterBlock(HWD16 * pTCoefs,
                                              vc1DEC_sState * pState,
                                              vc1DEC_sBitstream * pBitstream,
                                              int eBlk)
{
    vc1_sPosition *pPosition = &pState->sPosition;
    int Count;
    vc1_eResult eResult = vc1_ResultOK;
    vc1_sMB *pMB = pPosition->pCurMB;
    vc1_sBlk *pBlk = &pMB->sBlk[eBlk];
    vc1_NumZeroCoef *pNZC = pBlk->u.sInter.NZC;

    /* zero the coef array */
    for(Count = 0; Count < 64; Count++)
    {
        pTCoefs[Count] = 0;
    }

    if(1 == pBlk->Coded)
    {
        /* this block has some coefficients */
        HWD16 TempTCoefs[64] = {0};        

        /* Set the subblock pattern */
        switch (pBlk->eBlkType)
        {
        case vc1_BlkInter8x8:
            pNZC[0] = 1;
            break;

        case vc1_BlkInter8x4:
        case vc1_BlkInter4x8:
            if (pState->FirstCodedBlock < eBlk ||
                pState->sPicParams.eFrameTransformType == pBlk->eBlkType)
            {
                eResult = vc1DECBLK_UnpackSubBlockPattern(pState, pBitstream, eBlk);
                COVERAGE("10.3.5.2.2");
            }
            break;

        case vc1_BlkInter4x4:
            eResult = vc1DECBLK_UnpackSubBlockPattern(pState, pBitstream, eBlk);
            COVERAGE("10.3.5.2.2");
            break;

        case vc1_BlkInterAny:
            eResult = vc1DECBLK_UnpackTTBLK(pState, pBitstream, eBlk);
            if(vc1_ResultOK != eResult)
            {
                return(eResult);
            }

            if (vc1_BlkInter4x4 == pBlk->eBlkType)
            {
                eResult = vc1DECBLK_UnpackSubBlockPattern(pState, pBitstream, eBlk);
                COVERAGE("10.3.5.2.2");
            }
            break;

        default:
            FATAL("UnpackInterBlock: Bad block type %d\n", pBlk->eBlkType);
        }

        if(vc1_ResultOK != eResult)
        {
            return(eResult);
        }

        /* AC coef decode, with 3D Huffman and run level decode */
        DEBUG0(vc1DEBUG_BLK, "Decode AC run level\n");
        DEBUG2(vc1DEBUG_BLK, "BlkType[%d] = %s\n", 
            eBlk, vc1DEBUG_BlkType[pMB->sBlk[eBlk].eBlkType]);

        eResult = vc1DEC3DH_DecodeACRunLevel(pState, pBitstream, TempTCoefs, pPosition, eBlk);
        if(vc1_ResultOK != eResult)
        {
            return(eResult);
        }

        DEBUG1(vc1DEBUG_BLK, "Post AC run level - Blk = %d\n", eBlk);
        DEBUGRECT16(vc1DEBUG_BLK, TempTCoefs, 8, 8, 8);


        /* de-zig-zag AC coefs according to eBlkType */
        DEBUG0(vc1DEBUG_ZZ, "De-zig-zag coefs\n");
        vc1DECZZ_DeZigZagBlock(pTCoefs,
                               TempTCoefs,
                               pState->ZigZagTableIndex,
                               pMB->sBlk[eBlk].eBlkType);
    }
    else
    {
        /* this block has no coefficients */
    }

    return(vc1_ResultOK);
}


/*
 * Description:
 * Unpack an intra coded block
 *
 * Remarks:
 * None.
 *
 * Inputs:
 * pTCoefs      - pointer to area into which coefficients will be written
 * pState       - pointer to the decoder's state
 * pBitstream   - pointer to struct representing the current bitstream
 * eBlk         - block number of current block
 *
 * Outputs:.
 * pTCoefs      - updated with coefficients
 * pBitstream   - updated with new bitstream position
 *
 * Return Value:
 * Standard vc1_eResult result. See enumeration for possible result codes.
 * 
 */

static vc1_eResult vc1DECBLK_UnpackIntraBlock(
    HWD16 * pTCoefs,
    vc1DEC_sState * pState,
    vc1DEC_sBitstream * pBitstream,
    int eBlk)
{
    vc1_sPosition   *pPosition = &pState->sPosition;
    vc1_eBlkType    eBlkType;
    vc1_eResult     eResult;
    HWD16           DCDifferential; 
    vc1_sMB         *pMB = pPosition->pCurMB;
    HWD16           Pred[8];
    int             Count;
    int             DCDefault;

    /* zero the coef array */
    for(Count = 0; Count < 64; Count++)
    {
        pTCoefs[Count] = 0;
    }

    if( (pMB->OverlapFilter) ||
        (FALSE == vc1_PictureTypeIsIntra(pPosition->ePictureType))  ||
        (vc1_ProfileAdvanced == pState->sSeqParams.eProfile)        )
    {
        DCDefault = vc1PREDDCAC_DCDefault(pMB->sQuant.Quant, 128);
    }
    else
    {
        /* Simple/Main I/BI picture no overlap smoothing */
        DCDefault = vc1PREDDCAC_DCDefault(pMB->sQuant.Quant, 0);
    }

    /*
     * find which direction prediction was used
     *  we need to know the direction in order to choose the table
     *  for AC coef decode
     * also, get the predicted AC and DC values for this block
     *
     */

    DEBUG0(vc1DEBUG_BLK, "PredictDC/AC\n");
    eBlkType = vc1PREDDCAC_PredictDCAC(Pred, pPosition, eBlk, DCDefault);
    pMB->sBlk[eBlk].eBlkType = eBlkType;
    DEBUG2(vc1DEBUG_DCAC, "eBlkType = %s, Blk = %d, Pred = ", vc1DEBUG_BlkType[eBlkType], eBlk);
    DEBUGRECT16(vc1DEBUG_DCAC, Pred, 8, 1, 8);


    /* get the DC differential for this block */
    DEBUG0(vc1DEBUG_BLK, "Unpack DC differential\n");
    eResult = vc1DECBLK_UnpackDCDifferential(pState, pBitstream, &DCDifferential, eBlk);
    if(vc1_ResultOK != eResult)
    {
        return(eResult);
    }
    DEBUG1(vc1DEBUG_CMP, "DC = %4d\n", DCDifferential);

    if(1 == pMB->sBlk[eBlk].Coded)
    {
        HWD16 TempTCoefs[64] = {0};

        /* indicate ac coefs are present for this block */
        pMB->sBlk[eBlk].u.sIntra.NZC = 1;

        /* AC coef decode, with 3D Huffman, and run-level decode */
        DEBUG0(vc1DEBUG_BLK, "Decode AC run level\n");
        eResult = vc1DEC3DH_DecodeACRunLevel(pState, pBitstream, TempTCoefs, pPosition, eBlk);
        if(vc1_ResultOK != eResult)
        {
            return(eResult);
        }
        DEBUG1(vc1DEBUG_BLK, "Post AC run level - Blk = %d\n", eBlk);
        DEBUGRECT16(vc1DEBUG_BLK, TempTCoefs, 8, 8, 8);

        /* de-zig-zag AC coefs according to eBlkType */
        DEBUG0(vc1DEBUG_ZZ, "De-zig-zag coefs\n");
        vc1DECZZ_DeZigZagBlock(pTCoefs, TempTCoefs, pState->ZigZagTableIndex, eBlkType);
    }
    else
    {
        /* no ac coefs in this block */
        pMB->sBlk[eBlk].u.sIntra.NZC = 0;
    }

    COVERAGE_NONINT_INT("8.1.1.6","9.1.3.7");
    if(vc1_ACPredOn == pMB->eACPred)
    {
        /* AC prediction is enabled - add in predicted values at places indicated by eBlkType */
        DEBUG0(vc1DEBUG_DCAC, "Apply AC prediction\n");
        DEBUG1(vc1DEBUG_DCAC, "Pre AC prediction - Blk = %d\n", eBlk);
        DEBUGRECT16(vc1DEBUG_DCAC, pTCoefs, 8, 8, 8);
        vc1DECBLK_ApplyACPrediction(pTCoefs, Pred, eBlkType);
        DEBUG1(vc1DEBUG_DCAC, "Post AC prediction - Blk = %d\n", eBlk);
        DEBUGRECT16(vc1DEBUG_DCAC, pTCoefs, 8, 8, 8);
    }

    /* obtain quantized DC coefficient */
    DEBUG0(vc1DEBUG_BLK, "Calculate quantised DC coefficient\n");
    pTCoefs[0] = (HWD16)(Pred[0] + DCDifferential);
    DEBUG1(vc1DEBUG_QUANT, "TCoefs[0] = 0x%04x\n", pTCoefs[0]);

    /* put the data needed for later predictions into state */
    DEBUG0(vc1DEBUG_BLK, "Copy DC/AC data to state\n");
    vc1PREDDCAC_CopyDCAC(&pMB->sBlk[eBlk], pTCoefs);

    return(vc1_ResultOK);
}


/*
 * Description:
 * Decode the block layer of the bitstream.
 *
 * Remarks:
 * Unpacks and decodes the block layer.
 * Operations depending on the bitstream, and that can fail due to this, are
 *  in UnpackBlock(). Operations purely dependent on data in state or other
 *  structures are in ReconstructBlock().
 *
 * Inputs:
 * pState       - pointer to the decoder state structure
 * pBitstream   - pointer to the bitstream structure
 *
 * Outputs:
 * pState       - updated with data read from the bitstream
 * pBitstream   - updated with new bitstream position
 *
 * Return Value:
 * Standard vc1_eResult result. See enumeration for possible result codes.
 * 
 */

vc1_eResult vc1DECBLK_DecodeBlockLayer(vc1DEC_sState * pState, vc1DEC_sBitstream * pBitstream)
{
    vc1_sPosition   *pPosition = &pState->sPosition;
    vc1_eResult     eResult;
    HWD16           TCoefs[6][64] = {{0}};
    UBYTE8          PredBlk[6][64];
    int             Blk;

    for(Blk = 0; Blk < VC1_BLOCKS_PER_MB; Blk++)
    {
        DEBUG1(vc1DEBUG_BLK, "Decode block %d\n", Blk);

        if(FALSE == pPosition->pCurMB->Skipped)
        {
            if(TRUE == vc1_BlkTypeIsIntra(pPosition->pCurMB->sBlk[Blk].eBlkType))
            {
                DEBUG0(vc1DEBUG_BLK, "Decode intra block\n");
                eResult = vc1DECBLK_UnpackIntraBlock(TCoefs[Blk], pState, pBitstream, Blk);
            }
            else
            {
                DEBUG0(vc1DEBUG_BLK, "Decode inter block\n");
                eResult = vc1DECBLK_UnpackInterBlock(TCoefs[Blk], pState, pBitstream, Blk);
            }
            if(vc1_ResultOK != eResult)
            {
                return(eResult);
            }
        }
        else
        {
            DEBUG0(vc1DEBUG_BLK, "Block skipped\n");
        }
    }

    vc1INTERP_PredictMB(PredBlk, pPosition, pState->sPicParams.sInterpolate.RndCtrl);
    vc1RECON_ReconstructMB(TCoefs,
                           PredBlk,
                           pState->pCurrentRef,
                           pPosition,
                           pState->sSeqParams.eProfile);

    return(vc1_ResultOK);
}
