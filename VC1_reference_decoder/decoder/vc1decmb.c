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
 * vc1decmb.c:
 * Bitstream macroblock layer unpack functions
 *
 */

#include "vc1types.h"
#include "vc1dec.h"
#include "vc1gentab.h"
#include "vc1decbit.h"
#include "vc1decmb.h"
#include "vc1decpic.h"
#include "vc1decblk.h"
#include "vc1debug.h"
#include "vc1predcbp.h"
#include "vc1decmbtab.h"
#include "vc1tools.h"
#include "vc1decbitpl.h"
#include "vc1predmv.h"
#include "vc1derivemv.h"
#include "vc1preddcac.h"
#include "vc1decmv.h"
#include "vc1iquant.h"


/* table 42 */
static const vc1DEC_sVLCCode vc1DECMB_Macroblock_Transform_Type_Index_PQUANT_1_4[17] =
{
    {0, 16, 12},
    {0, 2, vc1_SBP8x4Both},         {3, 2, vc1_SBP8x8},
    {1, 2, vc1_SBP4x8Both},         {4, 3, vc1_SBP4x4},
    {22, 5, vc1_SBP4x8Right},       {21, 5, vc1_SBP4x8Left},
    {20, 5, vc1_SBP8x8MB},          {46, 6, vc1_SBP8x4Bottom},     
    {95, 7, vc1_SBP8x4Top},         {377, 9, vc1_SBP8x4TopMB},
    {379, 9, vc1_SBP8x4BothMB},     {378, 9, vc1_SBP4x4MB},
    {753, 10, vc1_SBP8x4BottomMB},  {1505, 11, vc1_SBP4x8BothMB},
    {3008, 12, vc1_SBP4x8RightMB},  {3009, 12, vc1_SBP4x8LeftMB},
};


/* table 43 */
static const vc1DEC_sVLCCode vc1DECMB_Macroblock_Transform_Type_Index_PQUANT_5_12[17] =
{
    {0, 16, 8},

    {2, 2, vc1_SBP8x8MB},           {0, 3, vc1_SBP4x8Both},
    {2, 3, vc1_SBP4x4},             {6, 3, vc1_SBP8x8},
    {6, 4, vc1_SBP8x4Bottom},       {3, 4, vc1_SBP8x4Top},
    {7, 4, vc1_SBP8x4Both},         {15, 4, vc1_SBP4x8Right},
    {14, 4, vc1_SBP4x8Left},        {11, 6, vc1_SBP8x4BothMB},
    {9, 6, vc1_SBP4x8RightMB},      {20, 7, vc1_SBP8x4BottomMB},
    {17, 7, vc1_SBP8x4TopMB},       {21, 7, vc1_SBP4x8BothMB},
    {33, 8, vc1_SBP4x8LeftMB},      {32, 8, vc1_SBP4x4MB}
};

/* table 44 */
static const vc1DEC_sVLCCode vc1DECMB_Macroblock_Transform_Type_Index_PQUANT_13_31[17] =
{
    {0, 16, 11},
    {2, 2, vc1_SBP8x8MB},           {6, 3, vc1_SBP8x8},
    {0, 3, vc1_SBP8x4Bottom},       {2, 3, vc1_SBP4x8Right},
    {3, 3, vc1_SBP4x8Left},         {14, 4, vc1_SBP8x4Top},
    {3, 4, vc1_SBP4x8Both},         {15, 4, vc1_SBP4x4},
    {5, 5, vc1_SBP8x4Both},         {9, 6, vc1_SBP8x4BothMB},
    {17, 7, vc1_SBP4x8BothMB},      {33, 8, vc1_SBP8x4TopMB},
    {65, 9, vc1_SBP4x8LeftMB},      {129, 10, vc1_SBP8x4BottomMB},
    {257, 11, vc1_SBP4x8RightMB},   {256, 11, vc1_SBP4x4MB}
};


#define NA ((FLAG)(99)) /* Not applicable */


static const vc1DECMB_sMBMode vc1DECMB_Macroblock_Mode_Table[8] =
{
    /* MBType, CBPPresent, MVPresent */
    {vc1_MBIntra,   FALSE,  NA},
    {vc1_MBIntra,   TRUE,   NA},
    {vc1_MB1MV,     FALSE,  FALSE},
    {vc1_MB1MV,     FALSE,  TRUE},
    {vc1_MB1MV,     TRUE,   FALSE},
    {vc1_MB1MV,     TRUE,   TRUE},
    {vc1_MB4MV,     FALSE,  NA},
    {vc1_MB4MV,     TRUE,   NA}
};

typedef struct
{
    vc1_eMBType             eMBType;
    FLAG                    MVPresent;
    FLAG                    CBPPresent;
} vc1DECMB_sFrameIntMBMode;


static const vc1DECMB_sFrameIntMBMode vc1DECMB_Frame_Interlaced_Macroblock_Mode_Table[15] =
{
    {(vc1_eMBType)(vc1_MB1MV                                ), TRUE,   TRUE},
    {(vc1_eMBType)(vc1_MB1MV                 | vc1_MBFieldTX), TRUE,   TRUE},
    {(vc1_eMBType)(vc1_MB1MV                                ), TRUE,   FALSE},
    {(vc1_eMBType)(vc1_MB1MV                                ), FALSE,  TRUE},
    {(vc1_eMBType)(vc1_MB1MV                 | vc1_MBFieldTX), FALSE,  TRUE},
    {(vc1_eMBType)(vc1_MB2MV | vc1_MBFieldMV                ), NA,     TRUE},
    {(vc1_eMBType)(vc1_MB2MV | vc1_MBFieldMV | vc1_MBFieldTX), NA,     TRUE},
    {(vc1_eMBType)(vc1_MB2MV | vc1_MBFieldMV | vc1_MBFieldTX), NA,     FALSE},
    {(vc1_eMBType)(vc1_MBIntra                              ), NA,     TRUE},
    {(vc1_eMBType)(vc1_MB4MV                                ), NA,     TRUE},
    {(vc1_eMBType)(vc1_MB4MV                 | vc1_MBFieldTX), NA,     TRUE},
    {(vc1_eMBType)(vc1_MB4MV                                ), NA,     FALSE},
    {(vc1_eMBType)(vc1_MB4MV | vc1_MBFieldMV                ), NA,     TRUE},
    {(vc1_eMBType)(vc1_MB4MV | vc1_MBFieldMV | vc1_MBFieldTX), NA,     TRUE},
    {(vc1_eMBType)(vc1_MB4MV | vc1_MBFieldMV | vc1_MBFieldTX), NA,     FALSE}
};


/* table 45 */
static const vc1DEC_sVLCCode vc1DECMB_B_Frame_Motion_Prediction_Type_BFRACTION_Less_Than_Half[4] =
{
    {0, 3, 2},
    {0, 1, vc1_MBForward},      {2, 2, vc1_MBBackward},     {3, 2, vc1_MBInterp}
};

static const vc1DEC_sVLCCode vc1DECMB_B_Frame_Motion_Prediction_Type_BFRACTION_More_Than_Half[4] =
{
    {0, 3, 2},
    {0, 1, vc1_MBBackward},     {2, 2, vc1_MBForward},      {3, 2, vc1_MBInterp}
};


static const vc1DEC_sVLCCode vc1DECMB_B_Field_Macroblock_Type[4] =
{
    {0, 3, 2},
    {0, 1, vc1_MBBackward},     {2, 2, vc1_MBDirect},       {3, 2, vc1_MBInterp}
};


/*
 * Description:
 * Assign the bits in a coded block pattern to the coded flag of each
 * block.
 *
 * Remarks:
 * None.
 *
 * Inputs:
 * pMB  - pointer to macroblock from which CBP is read, and to which flags are written
 *
 * Outputs:
 * pMB  - block's coded flags updated
 *
 * Return Value:
 * None
 * 
 */
static void vc1DECMB_AssignCodedBlockPattern(vc1_sMB * pMB)
{
    int CBPCY = pMB->CBPCY;
    int Count;

    DEBUG1(vc1DEBUG_MB, "AssignCBP: CBPCY = 0x%02x\n", CBPCY);
    for(Count = 0; Count < VC1_BLOCKS_PER_MB; Count++)
    {
        vc1_sBlk *pBlk = &pMB->sBlk[Count];
        int Coded = (0 != (CBPCY & (1 << (5 - Count))));

        pBlk->Coded = (FLAG)Coded;
        if (!Coded && vc1_BlkTypeIsInter(pBlk->eBlkType))
        {
            /* Non coded blocks must be 8x8 for deblock to work properly */
            pBlk->eBlkType = vc1_BlkInter8x8;
        }

        DEBUG3(vc1DEBUG_MB, "AssignCBP: Block=%d Coded=%d Type=%s\n",
            Count, Coded, vc1DEBUG_BlkType[pBlk->eBlkType]);
    }
}



/*
 * Description:
 * Unpack the macroblock level quantisation parameters
 *
 * Remarks:
 * None.
 *
 * Inputs:
 * pState       - pointer to the decoder's state
 * pBitstream   - pointer to struct representing the current bitstream
 *
 * Outputs:
 * pState       - updated with macroblock transform type
 * pBitstream   - updated with new position
 *
 * Return Value:
 * Standard vc1_eResult result. See enumeration for possible result codes.
 * 
 */

static vc1_eResult vc1DECMB_UnpackMBQuantParams(vc1DEC_sState * pState,
                                                vc1DEC_sBitstream * pBitstream)
{
    vc1_sMB *pMB = pState->sPosition.pCurMB;
    WORD32 Value;

    if(vc1_QuantModeMBDual == pState->sPicParams.eQuantMode)
    {
        Value = vc1DECBIT_GetBits(pBitstream, 1);   /* 1-bit MQDIFF */
        if(VC1DECBIT_EOF == Value)
        {
            return(vc1_ResultBufferExhausted);
        }
        DEBUG1(vc1DEBUG_MB, "MQDIFF: %d\n", Value);

        if(TRUE == Value)
        {
            pMB->sQuant.Quant = pState->sPicParams.AltPQuant;
            pMB->sQuant.HalfStep = 0;
        }
        else
        {
            pMB->sQuant.Quant = pState->sPicParams.PQuant;
        }
    }
    else if(vc1_QuantModeMBAny == pState->sPicParams.eQuantMode)
    {
        /* MQuant differential or escape code */
        Value = vc1DECBIT_GetBits(pBitstream, 3);   /* 3-bit MQDIFF */
        if(VC1DECBIT_EOF == Value)
        {
            return(vc1_ResultBufferExhausted);
        }
        DEBUG1(vc1DEBUG_MB, "MQDIFF: %d\n", Value);

        pMB->sQuant.HalfStep = 0;

        if(7 != Value)
        {
            pMB->sQuant.Quant = (UBYTE8)(pState->sPicParams.PQuant + Value);
        }
        else
        {
            /* Alternative macroblock quantization level */
            Value = vc1DECBIT_GetBits(pBitstream, VC1_BITS_ABSMQ);
            if(VC1DECBIT_EOF == Value)
            {
                return(vc1_ResultBufferExhausted);
            }
            DEBUG1(vc1DEBUG_MB, "ABSMQ: %d\n", Value);
            pMB->sQuant.Quant = (UBYTE8)Value;
            COVERAGE_NONINT_INT("7.1.3.7","9.1.3.9");
        }
    }

    COVERAGE_NONINT_INT("7.1.3.6","9.1.3.8");


    return(vc1_ResultOK);
}




/*
 * Description:
 * Unpack the macroblock level transform type
 *
 * Remarks:
 * None.
 *
 * Inputs:
 * pState       - pointer to the decoder's state
 * pBitstream   - pointer to struct representing the current bitstream
 *
 * Outputs:
 * pState       - updated with macroblock transform type
 * pBitstream   - updated with new position
 *
 * Return Value:
 * Standard vc1_eResult result. See enumeration for possible result codes.
 * 
 */

static vc1_eResult vc1DECMB_UnpackTTMB(vc1DEC_sState * pState, vc1DEC_sBitstream * pBitstream)
{
    vc1_sPosition *pPosition = &pState->sPosition;
    vc1_sMB *pMB = pPosition->pCurMB;
    const vc1DEC_sVLCCode           * pTable;
    int eSBP;
    vc1_NumZeroCoef *pNZC;
    int Count, Limit, FirstBlock = 0;
    vc1_eBlkType *pBlkType;

    COVERAGE("7.1.3.11");
    COVERAGE("8.3.4.8");

    /* Find the first coded inter block - if there isn't one then the
     * TTMB isn't present
     */
    while (  (FirstBlock <  VC1_BLOCKS_PER_MB)             &&
             ( (0 == ((1 << (5-FirstBlock)) & pMB->CBPCY)) ||
               (pMB->sBlk[FirstBlock].eBlkType == vc1_BlkIntra) )
          )
    {
        FirstBlock++;
    }

    if(FirstBlock == VC1_BLOCKS_PER_MB)
    {
        /* there are no coded blocks in this macroblock - nothing more to do with the TTMB */
        DEBUG0(vc1DEBUG_MB, "UnpackTTMB: No coded blocks in macroblock.\n");
        return(vc1_ResultOK);
    }

    if(5 > pState->sPicParams.PQuant)
    {
        pTable = vc1DECMB_Macroblock_Transform_Type_Index_PQUANT_1_4;
    }
    else if((5 <= pState->sPicParams.PQuant) && (13 > pState->sPicParams.PQuant))
    {
        pTable = vc1DECMB_Macroblock_Transform_Type_Index_PQUANT_5_12;
    }
    else
    {
        pTable = vc1DECMB_Macroblock_Transform_Type_Index_PQUANT_13_31;
    }

    eSBP = vc1DECBIT_GetVLC(pBitstream, pTable);
    if(VC1DECBIT_EOF == eSBP)
    {
        return(vc1_ResultBufferExhausted);
    }
    DEBUG2(vc1DEBUG_MB, "TTMB: %s (%d)\n", vc1DEBUG_SBPType[eSBP], eSBP);

    Limit = VC1_BLOCKS_PER_MB;

    if(FALSE == vc1_SBPIsMBLevel(eSBP))
    {
        DEBUG0(vc1DEBUG_MB, "TTMB applies to first coded block only\n");
        pMB->eBlkType = vc1_BlkInterAny;

        /* block level transform - decoded TTMB applies to the first block only */
        Limit = FirstBlock + 1;
    }
    else
    {
        /* adjust the SBP - we've used the block/macroblock level information */
        eSBP -= vc1_SBPMBLevel;
    }

    pState->FirstCodedBlock = (UBYTE8)FirstBlock;    
    DEBUG1(vc1DEBUG_MB, "UnpackTTMB: First coded block in macroblock is %d\n", FirstBlock);

    pNZC = pMB->sBlk[FirstBlock].u.sInter.NZC;

    pNZC[0] = 0;
    pNZC[1] = 0;
    pNZC[2] = 0;
    pNZC[3] = 0;

    /* set up the subblock coded status from the subblock pattern */
    switch(eSBP)
    {
        case vc1_SBP8x8:
            pNZC[0] = 1;
            break;

        case vc1_SBP8x4Bottom:;
            pNZC[1] = 1;
            break;

        case vc1_SBP8x4Top:
            pNZC[0] = 1;
            break;

        case vc1_SBP8x4Both:
            pNZC[0] = 1;
            pNZC[1] = 1;
            break;

        case vc1_SBP4x8Right:
            pNZC[1] = 1;
            break;

        case vc1_SBP4x8Left:
            pNZC[0] = 1;
            break;

        case vc1_SBP4x8Both:
            pNZC[0] = 1;
            pNZC[1] = 1;
            break;

        case vc1_SBP4x4:
            /* do nothing - a SUBBLKPAT will appear later in the bitstream */
            break;

        default:
            FATAL("UnpackTTMB: Unknown SBP - %d\n", eSBP);
            return(vc1_ResultFatal);
    }

    for(Count = FirstBlock; Count < Limit; Count++)
    {
        pBlkType = &pMB->sBlk[Count].eBlkType;

        if(FALSE == vc1_BlkTypeIsIntra(*pBlkType))
        {
            switch(eSBP)
            {
                case vc1_SBP8x8:
                    *pBlkType = vc1_BlkInter8x8;
                    break;

                case vc1_SBP8x4Bottom:                  
                    *pBlkType = vc1_BlkInter8x4;
                    break;

                case vc1_SBP8x4Top:
                    *pBlkType = vc1_BlkInter8x4;
                    break;

                case vc1_SBP8x4Both:
                    *pBlkType = vc1_BlkInter8x4;
                    break;

                case vc1_SBP4x8Right:
                    *pBlkType = vc1_BlkInter4x8;
                    break;

                case vc1_SBP4x8Left:
                    *pBlkType = vc1_BlkInter4x8;
                    break;

                case vc1_SBP4x8Both:
                    *pBlkType = vc1_BlkInter4x8;
                    break;

                case vc1_SBP4x4:
                    *pBlkType = vc1_BlkInter4x4;
                    break;

                default:
                    FATAL("UnpackTTMB: Unknown SBP - %d\n", eSBP);
                    return(vc1_ResultFatal);
            }
        }

        DEBUG2(vc1DEBUG_MB, "sBlk[%d].eBlkType = %s\n", Count, vc1DEBUG_BlkType[*pBlkType]);
    }

    for( ; Count < VC1_BLOCKS_PER_MB; Count++)
    {
        pBlkType = &pMB->sBlk[Count].eBlkType;

        /* block type set to any - will be filled in by a later TTBLK */
        if(FALSE == vc1_BlkTypeIsIntra(*pBlkType))
        {
            *pBlkType = vc1_BlkInterAny;
        }
        DEBUG2(vc1DEBUG_MB, "sBlk[%d].eBlkType = %s\n", Count, vc1DEBUG_BlkType[*pBlkType]);
    }

    if(vc1_BlkInterAny != pMB->eBlkType)
    {
        pMB->eBlkType = pMB->sBlk[0].eBlkType;
    }

    COVERAGE("8.3.5.6");
    COVERAGE("10.3.5.2.1");
    return(vc1_ResultOK);
}


/*
 * Description:
 * Decode the transform infomation in the bitstream
 *
 * Remarks:
 * None.
 *
 * Inputs:
 * pState       - pointer to the decoder's state
 * pBitstream   - pointer to struct representing the current bitstream
 *
 * Outputs:
 * pBitstream   - updated with new position
 * pState       - updated with transform information
 *
 * Return Value:
 * Standard vc1_eResult result. See enumeration for possible result codes.
 * 
 */

static vc1_eResult vc1DECMB_DecodeTransformInfo(vc1DEC_sState * pState,
                                                vc1DEC_sBitstream * pBitstream)
{
    vc1_sPosition *pPosition = &pState->sPosition;
    vc1_sMB *pMB = pPosition->pCurMB;
    vc1_eResult eResult;

    if(TRUE == pState->sSeqParams.VSTransform)
    {
        if((FALSE == pState->sPicParams.MBTransformTypeFlag) &&
           (FALSE == vc1_MBTypeIsIntra(pMB->eMBType)))
        {
            COVERAGE("8.3.6.2.1");
            eResult = vc1DECMB_UnpackTTMB(pState, pBitstream);
            if(vc1_ResultOK != eResult)
            {
                return(eResult);
            }
        }
        else
        {
            /* Fill out blocks with frame level transform type */
            int Count;

            pMB->eBlkType = pState->sPicParams.eFrameTransformType;

            for(Count = 0; Count < VC1_BLOCKS_PER_MB; Count++)
            {
                if(FALSE == vc1_MBTypeIsIntra(pMB->eMBType))
                {
                    ASSERT(pState->sPicParams.eFrameTransformType != vc1_BlkInterAny);

                    if(FALSE == vc1_BlkTypeIsIntra(pMB->sBlk[Count].eBlkType))
                    {
                        DEBUG2(vc1DEBUG_MB,
                              "pMB->sBlk[%d].eBlkType = %d\n",
                              Count,
                              pState->sPicParams.eFrameTransformType);
                        pMB->sBlk[Count].eBlkType = pState->sPicParams.eFrameTransformType;
                    }
                }
                else
                {
                    DEBUG2(vc1DEBUG_MB, "pMB->sBlk[%d].eBlkType = %d\n", Count, vc1_BlkIntra);
                    pMB->sBlk[Count].eBlkType = vc1_BlkIntra;
                }
            }
        }
    }
    else
    {
        /*
         * if variable sized transforms are switched off,
         * inherit the frame's (default) transform type, which should be 8x8
         */
        ASSERT(pState->sPicParams.eFrameTransformType == vc1_BlkInter8x8);
        pMB->eBlkType = pState->sPicParams.eFrameTransformType;
    }

    return(vc1_ResultOK);
}


/*
 * Description:
 * Unpack the macroblock layer for progressive 1MV macroblocks in P frames
 *
 * Remarks:
 * None.
 *
 * Inputs:
 * pState       - pointer to the decoder's state
 * pBitstream   - pointer to struct representing the current bitstream
 *
 * Outputs:
 * pState       - updated with read data
 * pBitstream   - updated with new position
 *
 * Return Value:
 * Standard vc1_eResult result. See enumeration for possible result codes.
 * 
 */

static vc1_eResult vc1DECMB_UnpackMacroblockProgP1MV(vc1DEC_sState * pState,
                                                     vc1DEC_sBitstream * pBitstream)
{
    vc1_sPosition *pPosition = &pState->sPosition;
    vc1_eResult eResult;
    vc1_sMB *pMB = pPosition->pCurMB;
    WORD32 Value;
    vc1_sMV sPredMV;
    WORD32 CBPCY;   /* CBPCY */
    int Coded;      /* Coded flag */

    COVERAGE("8.3.3.1");

    /* motion vector data */
    eResult = vc1DECMV_UnpackMVData(pMB, pState, pBitstream, 0, 0);
    if(vc1_ResultOK != eResult)
    {
        return(eResult);
    }
    COVERAGE("7.1.3.8");

    /* Find if block is coded */
    Coded = pMB->sBlk[0].Coded;

    if (vc1_BlkTypeIsIntra(pMB->sBlk[0].eBlkType))
    {
        pMB->eMBType  = vc1_MBIntra;
        pMB->eBlkType = vc1_BlkIntra;
    }

    /* Note that the macroblock type Intra/1MV depends on MVData */
    DEBUG1(vc1DEBUG_CMP | vc1DEBUG_MB, "%s\n", vc1DEBUG_MBType[pMB->eMBType]);

    if (vc1_MBTypeIsInter(pMB->eMBType))
    {
        /* sniff the hybridpred bit, but don't read it, as the prediction might not need it */
        Value = vc1DECBIT_LookBits(pBitstream, VC1_BITS_HYBRIDPRED);
        if(VC1DECBIT_EOF == Value)
        {
            return(vc1_ResultBufferExhausted);
        }

        /* Blk is ignored for 1MV */
        if(TRUE == vc1PREDMV_PredictProgressiveMV(&sPredMV,
                                                  pPosition,
                                                  0,
                                                  (vc1_eHybridPred)Value,
                                                  0))
        {
            /* the prediction used the hybrid bit - advance the bitstream */
            vc1DECBIT_GetBits(pBitstream, VC1_BITS_HYBRIDPRED);
            DEBUG1(vc1DEBUG_MB | vc1DEBUG_CMP, "HYBRIDPRED: %d\n", Value);
            COVERAGE("7.1.3.10");
        }

        vc1DECMV_ApplyMVPrediction(pState, 0, &sPredMV, 0);
    }

    /* set chroma block types depending on luma blocks */
    vc1DERIVEMV_DecideChromaBlockType(pPosition);

    if (vc1_MBTypeIsIntra(pMB->eMBType) && !Coded)
    {
        if(TRUE == pState->sPicParams.DQuantFrame)
        {
            eResult = vc1DECMB_UnpackMBQuantParams(pState, pBitstream);
            if(vc1_ResultOK != eResult)
            {
                return(eResult);
            }
        }

        /* ac prediction flag */
        Value = vc1DECBIT_GetBits(pBitstream, VC1_BITS_ACPRED_MB);
        if(VC1DECBIT_EOF == Value)
        {
            return(vc1_ResultBufferExhausted);
        }
        DEBUG1(vc1DEBUG_MB | vc1DEBUG_CMP, "ACPRED=%d\n", Value);
        pMB->eACPred = (vc1_eACPred)Value;
        pMB->CBPCY = 0;
    }
    else if (Coded)
    {
        if (vc1_MBTypeIsIntra(pMB->eMBType))
        {
            /* ac prediction flag */
            Value = vc1DECBIT_GetBits(pBitstream, VC1_BITS_ACPRED_MB);
            if(VC1DECBIT_EOF == Value)
            {
                return(vc1_ResultBufferExhausted);
            }
            DEBUG1(vc1DEBUG_MB | vc1DEBUG_CMP, "1MV ACPRED=%d\n", Value);
            pMB->eACPred = (vc1_eACPred)Value;
            COVERAGE("8.3.6.1.3");
        }


        /* coded block pattern */
        CBPCY = vc1DECBIT_GetVLC(pBitstream, pState->sPicParams.pCodedBlockPatternTable);
        if(VC1DECBIT_EOF == CBPCY)
        {
            return(vc1_ResultBufferExhausted);
        }
        DEBUG1(vc1DEBUG_MB, "Read CBPCY: 0x%02X\n", CBPCY);
        pMB->CBPCY = (UBYTE8)CBPCY;

        DEBUG1(vc1DEBUG_CMP | vc1DEBUG_MB, "1MV CBPCY=%2x\n", pMB->CBPCY);

        if(TRUE == pState->sPicParams.DQuantFrame)
        {
            eResult = vc1DECMB_UnpackMBQuantParams(pState, pBitstream);
            if(vc1_ResultOK != eResult)
            {
                return(eResult);
            }
        }

        if (CBPCY==0)
        {
            /* Coded block with CPBCY. We still need to read TTMB in this
             * case so mark one block as coded
             */
            pMB->CBPCY = 1;
        }
        eResult = vc1DECMB_DecodeTransformInfo(pState, pBitstream);
        if(vc1_ResultOK != eResult)
        {
            return(eResult);
        }
        pMB->CBPCY = (UBYTE8)CBPCY;
    }
    else /* Inter not coded */
    {
        /* no 1MV coded blocks - CBPCY = 0 */
        pMB->CBPCY = 0;
    }
    
    return(vc1_ResultOK);
}



/*
 * Description:
 * Unpack the macroblock layer for progressive 4MV macroblocks in P frames
 *
 * Remarks:
 * None.
 *
 * Inputs:
 * pState       - pointer to the decoder's state
 * pBitstream   - pointer to struct representing the current bitstream
 *
 * Outputs:
 * pState       - updated with read data
 * pBitstream   - updated with new position
 *
 * Return Value:
 * Standard vc1_eResult result. See enumeration for possible result codes.
 * 
 */

static vc1_eResult vc1DECMB_UnpackMacroblockProgP4MV(vc1DEC_sState * pState,
                                                     vc1DEC_sBitstream * pBitstream)
{
    vc1_sPosition *pPosition = &pState->sPosition;
    vc1_eResult eResult;
    vc1_sMB *pMB = pPosition->pCurMB;
    WORD32 Value;
    int Blk;
    vc1_sMV sPredMV;
    int AnyIntra = 0;

    COVERAGE("8.3.3.2");

    /* coded block pattern */
    Value = vc1DECBIT_GetVLC(pBitstream, pState->sPicParams.pCodedBlockPatternTable);
    if(VC1DECBIT_EOF == Value)
    {
        return(vc1_ResultBufferExhausted);
    }
    DEBUG1(vc1DEBUG_CMP | vc1DEBUG_MB, "4MV CBPCY=%2x\n", Value);
    pMB->CBPCY = (UBYTE8)Value;

    for(Blk = 0; Blk < 4; Blk++)
    {
        /* if the CBP indicates MV data is present for this block */
        int BlkMVDataPresent = ((0 != (    (1 << (5 - Blk)) & pMB->CBPCY) ) ? 1 : 0);

        if(1 == BlkMVDataPresent)
        {
            DEBUG1(vc1DEBUG_MB, "UnpackMVData, block %d\n", Blk);
            eResult = vc1DECMV_UnpackMVData(pMB, pState, pBitstream, Blk, 0);
            if(vc1_ResultOK != eResult)
            {
                return(eResult);
            }

            COVERAGE("7.1.3.9");
        }
        else
        {
            pMB->sBlk[Blk].eBlkType = pState->sPicParams.eFrameTransformType;
            pMB->sBlk[Blk].Coded = FALSE;
            pMB->sBlk[Blk].u.sInter.sMotion[0].sDMV.X = 0;
            pMB->sBlk[Blk].u.sInter.sMotion[0].sDMV.Y = 0;
        }

        if (vc1_BlkTypeIsInter(pMB->sBlk[Blk].eBlkType))
        {
            /* sniff the hybridpred bit, but don't read it, as the prediction might not need it */
            Value = vc1DECBIT_LookBits(pBitstream, VC1_BITS_HYBRIDPRED);
            if(VC1DECBIT_EOF == Value)
            {
                return(vc1_ResultBufferExhausted);
            }

            if(TRUE == vc1PREDMV_PredictProgressiveMV(&sPredMV,
                                                      pPosition,
                                                      Blk,
                                                      (vc1_eHybridPred)Value,
                                                      0))
            {
                /* the prediction used the hybrid bit - advance the bitstream */
                vc1DECBIT_GetBits(pBitstream, VC1_BITS_HYBRIDPRED);
                DEBUG1(vc1DEBUG_MB | vc1DEBUG_CMP, "HYBRIDPRED: %d\n", Value);
            }
            else
            {
                /* the prediction didn't use the hybrid bit - do nothing */
            }

            vc1DECMV_ApplyMVPrediction(pState, Blk, &sPredMV, 0);
        }
        else /* Intra */
        {
            AnyIntra = 1;
        }
    }

    /* set chroma block types depending on luma blocks */
    vc1DERIVEMV_DecideChromaBlockType(pPosition);

    if (TRUE == pState->sPicParams.DQuantFrame)
    {
        if (AnyIntra || pMB->CBPCY!=0)
        {
            eResult = vc1DECMB_UnpackMBQuantParams(pState, pBitstream);
            if(vc1_ResultOK != eResult)
            {
                return(eResult);
            }
        }
    }


    if (AnyIntra && vc1PREDDCAC_ACPREDPresent(pPosition))
    {
        /* ACPRED */
        Value = vc1DECBIT_GetBits(pBitstream, VC1_BITS_ACPRED_MB);
        if(VC1DECBIT_EOF == Value)
        {
            return(vc1_ResultBufferExhausted);
        }
        DEBUG1(vc1DEBUG_MB, "ACPRED: %d\n", Value);
        pMB->eACPred = (vc1_eACPred)Value;
        COVERAGE("8.3.6.1.2");
    }
    else
    {
        pMB->eACPred = vc1_ACPredOff;
    }

    eResult = vc1DECMB_DecodeTransformInfo(pState, pBitstream);
    if(vc1_ResultOK != eResult)
    {
        return(eResult);
    }

    return(vc1_ResultOK);
}


/*
 * Description:
 * Unpack the macroblock layer for progressive skipped macroblocks in P frames
 *
 * Remarks:
 * None.
 *
 * Inputs:
 * pState       - pointer to the decoder's state
 * pBitstream   - pointer to struct representing the current bitstream
 *
 * Outputs:
 * pState       - updated with read data
 * pBitstream   - updated with new position
 *
 * Return Value:
 * Standard vc1_eResult result. See enumeration for possible result codes.
 * 
 */

static vc1_eResult vc1DECMB_UnpackMacroblockProgPSkipped(vc1DEC_sState * pState,
                                                         vc1DEC_sBitstream * pBitstream)
{
    vc1_sPosition *pPosition = &pState->sPosition;
    vc1_sMV sPredMV;
    vc1_sMB *pMB = pPosition->pCurMB;
    WORD32 Value;
    int Blk;

    COVERAGE("8.3.5.3.5");

    /* set coded block pattern to zero */
    pMB->CBPCY = 0;

    /* set all differential mvs to zero */
    for(Blk = 0; Blk < 4; Blk++)
    {
        pMB->sBlk[Blk].u.sInter.sMotion[0].sDMV.X = 0;
        pMB->sBlk[Blk].u.sInter.sMotion[0].sDMV.Y = 0;
    }

    if(TRUE == vc1_MBTypeIs4MV(pMB->eMBType))
    {
        for(Blk = 0; Blk < 4; Blk++)
        {
            /* sniff the hybridpred bit, but don't read it, as the prediction might not need it */
            Value = vc1DECBIT_LookBits(pBitstream, VC1_BITS_HYBRIDPRED);
            if(VC1DECBIT_EOF == Value)
            {
                return(vc1_ResultBufferExhausted);
            }

            if(TRUE == vc1PREDMV_PredictProgressiveMV(&sPredMV,
                                                      pPosition,
                                                      Blk,
                                                      (vc1_eHybridPred)Value,
                                                      0))
            {
                /* the prediction used the hybrid bit - advance the bitstream */
                vc1DECBIT_GetBits(pBitstream, VC1_BITS_HYBRIDPRED);
                DEBUG1(vc1DEBUG_MB | vc1DEBUG_CMP, "HYBRIDPRED: %d\n", Value);
            }

            vc1DECMV_ApplyMVPrediction(pState, Blk, &sPredMV, 0);

            pMB->sBlk[Blk].eBlkType = vc1_BlkInter8x8;
        }
    }
    else
    {
        /* sniff the hybridpred bit, but don't read it, as the prediction might not need it */
        Value = vc1DECBIT_LookBits(pBitstream, VC1_BITS_HYBRIDPRED);
        if(VC1DECBIT_EOF == Value)
        {
            return(vc1_ResultBufferExhausted);
        }

        if(TRUE == vc1PREDMV_PredictProgressiveMV(&sPredMV,
                                                  pPosition,
                                                  0,
                                                  (vc1_eHybridPred)Value,
                                                  0))
        {
            /* the prediction used hybrid bit - advance the bitstream */
            vc1DECBIT_GetBits(pBitstream, VC1_BITS_HYBRIDPRED);
            DEBUG1(vc1DEBUG_MB | vc1DEBUG_CMP, "HYBRIDPRED: %d\n", Value);
        }

        vc1DECMV_ApplyMVPrediction(pState, 0, &sPredMV, 0);

        for(Blk = 0; Blk < 4; Blk++)
        {
            pMB->sBlk[Blk].eBlkType = vc1_BlkInter8x8;
        }
    }

    /* set chroma block types depending on luma blocks */
    vc1DERIVEMV_DecideChromaBlockType(pPosition);

    return(vc1_ResultOK);
}



/*
 * Description:
 * Unpack the macroblock layer for progressive P frames
 *
 * Remarks:
 * None.
 *
 * Inputs:
 * pState       - pointer to the decoder's state
 * pBitstream   - pointer to struct representing the current bitstream
 *
 * Outputs:
 * pState       - updated with read data
 * pBitstream   - updated with new position
 *
 * Return Value:
 * Standard vc1_eResult result. See enumeration for possible result codes.
 * 
 */


static vc1_eResult vc1DECMB_UnpackMacroblockProgP(vc1DEC_sState * pState,
                                                  vc1DEC_sBitstream * pBitstream)
{
    WORD32     Value;
    vc1_sPosition *pPosition = &pState->sPosition;
    vc1_sMB     *pMB = pPosition->pCurMB;
    vc1_eResult eResult;

    /* default MB type is 1MV Forwards */
    pMB->eMBType = (vc1_eMBType)(vc1_MB1MV | vc1_MBForward);

    if(vc1_MVModeMixedMV == pState->sPosition.eMVMode)
    {
        /* Motion vector mode bit */
        Value = vc1DECBITPL_ReadBitplaneBit(&pState->sPicParams.sBPMotionVectorType, pBitstream);
        if(VC1DECBIT_EOF == Value)
        {
            return(vc1_ResultBufferExhausted);
        }
        DEBUG1(vc1DEBUG_MB, "MVMODEBIT: %d\n", Value);
        COVERAGE("7.1.3.2");
        if(TRUE == Value)
        {
            pMB->eMBType = (vc1_eMBType)(vc1_MB4MV | vc1_MBForward);
        }
    }

    /* Skip macroblock */
    Value = vc1DECBITPL_ReadBitplaneBit(&pState->sPicParams.sBPSkipMB, pBitstream);
    if(VC1DECBIT_EOF == Value)
    {
        return(vc1_ResultBufferExhausted);
    }
    DEBUG1(vc1DEBUG_MB, "SKIPMBBIT: %d\n", Value);
    COVERAGE("7.1.3.3");


    if(TRUE == Value)
    {
        COVERAGE("8.3.4.4");
        DEBUG0(vc1DEBUG_MB, "Decoding Skipped macroblock\n");
        eResult = vc1DECMB_UnpackMacroblockProgPSkipped(pState, pBitstream);
        pPosition->pCurMB->Skipped = TRUE;
        DEBUG1(vc1DEBUG_CMP | vc1DEBUG_MB, "%s Skipped MB\n", vc1DEBUG_MBType[pMB->eMBType]);
    }
    else
    {
        if(TRUE == vc1_MBTypeIs1MV(pMB->eMBType))
        {
            DEBUG0(vc1DEBUG_MB, "Decoding 1MV macroblock\n");
            eResult = vc1DECMB_UnpackMacroblockProgP1MV(pState, pBitstream);
        }
        else if(TRUE == vc1_MBTypeIs4MV(pMB->eMBType))
        {
            DEBUG0(vc1DEBUG_MB, "Decoding 4MV macroblock\n");
            eResult = vc1DECMB_UnpackMacroblockProgP4MV(pState, pBitstream);
        }
        else
        {
            FATAL("UnpackMacroblockProgPSimpleMain: Unknown MB type - %d\n", pMB->eMBType);
            return(vc1_ResultFatal);
        }
    }
    if(vc1_ResultOK != eResult)
    {
        return(eResult);
    }

    return(vc1_ResultOK);
}


/*
 * Description:
 * Unpack the macroblock layer for progressive B frames
 *
 * Remarks:
 * None.
 *
 * Inputs:
 * pState       - pointer to the decoder's state
 * pBitstream   - pointer to struct representing the current bitstream
 *
 * Outputs:
 * pState       - updated with read data
 * pBitstream   - updated with new position
 *
 * Return Value:
 * Standard vc1_eResult result. See enumeration for possible result codes.
 * 
 */

static vc1_eResult vc1DECMB_UnpackMacroblockProgB(vc1DEC_sState * pState,
                                                  vc1DEC_sBitstream * pBitstream)
{
    vc1_eResult eResult;
    vc1_sMB *pMB = pState->sPosition.pCurMB;
    WORD32 Value;
    vc1_sMV sPredMV;
    vc1_sPosition *pPosition = &pState->sPosition;
    int Coded;

    /* default MB type is 1MV */
    pMB->eMBType = (vc1_eMBType)(vc1_MB1MV | vc1_MBForward);

    /* Direct B bit */
    Value = vc1DECBITPL_ReadBitplaneBit(&pState->sPicParams.sBPBFrameDirectMode, pBitstream);
    if(VC1DECBIT_EOF == Value)
    {
        return(vc1_ResultBufferExhausted);
    }
    DEBUG1(vc1DEBUG_MB, "DIRECTBBIT: %d\n", Value);
    COVERAGE("7.1.3.12");
    if(TRUE == Value)
    {
        pMB->eMBType = (vc1_eMBType)(vc1_MB1MV | vc1_MBDirect);
    }

    /* Skip macroblock bit */
    Value = vc1DECBITPL_ReadBitplaneBit(&pState->sPicParams.sBPSkipMB, pBitstream);
    if(VC1DECBIT_EOF == Value)
    {
        return(vc1_ResultBufferExhausted);
    }
    DEBUG1(vc1DEBUG_MB, "SKIPMB: %d\n", Value);
    COVERAGE("7.1.3.3");
    pMB->Skipped = (FLAG)Value;

    if(FALSE == vc1_MBTypeIsDirect(pMB->eMBType))
    {
        if(FALSE == pMB->Skipped)
        {
            /* B motion vector 1 */
            /*
             * NOTE: The coded flag written into the MB by UnpackMVData is used
             *  to determine whether a second set of MV data is present in the
             *  bitstream. If it is, those data are unpacked, and the coded flag
             *  is overwritten.
             *
             * NOTE2: The first motion vector is unpacked into the backwards mv
             *  fields. If the block type is forwards, it is copied into the
             *  forwards fields before prediction.
             *
             */
            /* BMV1 */
            eResult = vc1DECMV_UnpackMVData(pMB, pState, pBitstream, 0, 1);
            if(vc1_ResultOK != eResult)
            {
                return(eResult);
            }
            COVERAGE("7.1.3.13");
        }
        else /* skipped */
        {
            /* copy the macroblock type into the block types */
            int Count;

            for(Count = 0; Count < 4; Count++)
            {
                pMB->sBlk[Count].eBlkType = vc1_BlkInter8x8;
            }
        }

        if(TRUE == vc1_BlkTypeIsIntra(pMB->sBlk[0].eBlkType))
        {
            /* if the first block is intra, it must be an intra macroblock */
            pMB->eMBType = vc1_MBIntra;
        }

        if((TRUE == pMB->Skipped) || (TRUE == vc1_BlkTypeIsInter(pMB->sBlk[0].eBlkType)))
        {        
            /* B motion vector type */
            if(vc1GENTAB_pBFraction[pPosition->BFraction].ScaleFactor < 128)
            {
                Value =
                    vc1DECBIT_GetVLC(pBitstream,
                             vc1DECMB_B_Frame_Motion_Prediction_Type_BFRACTION_Less_Than_Half);
            }
            else
            {   
                Value =
                    vc1DECBIT_GetVLC(pBitstream,
                             vc1DECMB_B_Frame_Motion_Prediction_Type_BFRACTION_More_Than_Half);
            }
            COVERAGE("8.4.5.2");

            if(VC1DECBIT_EOF == Value)
            {
                return(vc1_ResultBufferExhausted);
            }
            COVERAGE("7.1.3.15");
            pMB->eMBType = (vc1_eMBType)(vc1_MB1MV | Value);
            DEBUG1(vc1DEBUG_MB, "BMVTYPE: %s\n", vc1DEBUG_MBType[pMB->eMBType]);
            COVERAGE("8.4.3");

            if(TRUE == vc1_MBTypeIsForward(pMB->eMBType))
            {
                /* copy the backwards differential motion vectors into the forward fields */
                int Count;
                for(Count = 0; Count < 4; Count++)
                {
                    pMB->sBlk[Count].u.sInter.sMotion[0].sDMV.X =
                        pMB->sBlk[Count].u.sInter.sMotion[1].sDMV.X;
                    pMB->sBlk[Count].u.sInter.sMotion[0].sDMV.Y =
                        pMB->sBlk[Count].u.sInter.sMotion[1].sDMV.Y;
                }
            }
            COVERAGE("8.4.5.7");
        }
    }
    else /* direct */
    {
        /* copy the macroblock type into the block types */
        int Count;

        for(Count = 0; Count < 4; Count++)
        {
            pMB->sBlk[Count].eBlkType = pState->sPicParams.eFrameTransformType;
        }

        pMB->sBlk[0].Coded = (FLAG)(1-pMB->Skipped);
    }

    DEBUG1(vc1DEBUG_MB, "MB Type = %s\n", vc1DEBUG_MBType[pMB->eMBType & 15]);

    Coded = pMB->sBlk[0].Coded;

    if (FALSE == pMB->Skipped)
    {
        if (vc1_MBTypeIsIntra(pMB->eMBType) && !Coded)
        {
            if (TRUE == pState->sPicParams.DQuantFrame)
            {
                eResult = vc1DECMB_UnpackMBQuantParams(pState, pBitstream);
                if(vc1_ResultOK != eResult)
                {
                    return(eResult);
                }
            }

            /* ACPRED */
            Value = vc1DECBIT_GetBits(pBitstream, VC1_BITS_ACPRED_MB);
            if(VC1DECBIT_EOF == Value)
            {
                return(vc1_ResultBufferExhausted);
            }
            DEBUG1(vc1DEBUG_MB, "ACPRED: %d\n", Value);
            pMB->eACPred = (vc1_eACPred)Value;

            /* no block are coded - clear the coded block pattern */
            pMB->CBPCY = 0;
        }
        else /* Inter or coded */
        {
            if (vc1_MBTypeIsInterp(pMB->eMBType) && Coded)
            {
                COVERAGE("8.4.5.8");    /* This implements the final part of this section */

                /* BMV2 motion vector 2 - unpack into forward motion vector fields */
                eResult = vc1DECMV_UnpackMVData(pMB, pState, pBitstream, 0, 0);
                if(vc1_ResultOK != eResult)
                {
                    return(eResult);
                }
                COVERAGE("7.1.3.14");
                Coded = pMB->sBlk[0].Coded;
            }

            if (vc1_MBTypeIsIntra(pMB->eMBType))
            {
                /* ac prediction */
                Value = vc1DECBIT_GetBits(pBitstream, VC1_BITS_ACPRED_MB);
                if(VC1DECBIT_EOF == Value)
                {
                    return(vc1_ResultBufferExhausted);
                }
                DEBUG1(vc1DEBUG_MB, "ACPRED: %d\n", Value);
                pMB->eACPred = (vc1_eACPred)Value;
            }

            if (Coded)
            {
                /* CBPCY */
                Value = vc1DECBIT_GetVLC(pBitstream, pState->sPicParams.pCodedBlockPatternTable);
                if(VC1DECBIT_EOF == Value)
                {
                    return(vc1_ResultBufferExhausted);
                }
                DEBUG1(vc1DEBUG_CMP | vc1DEBUG_MB, "CBPCY=%2x\n", Value);
                pMB->CBPCY = (UBYTE8)Value;

                /* MQDIFF, ABSMQ */
                if(TRUE == pState->sPicParams.DQuantFrame)
                {
                    eResult = vc1DECMB_UnpackMBQuantParams(pState, pBitstream);
                    if(vc1_ResultOK != eResult)
                    {
                        return(eResult);
                    }
                }
            }
        }
    } /* Not skipped */

    /* Predict and apply differential motion vector */
    switch (pMB->eMBType & vc1_MBDirMask)
    {
        case vc1_MBForward:
            vc1PREDMV_PredictProgressiveMV(&sPredMV, pPosition, 0, 0, 0);
            vc1DECMV_ApplyMVPrediction(pState, 0, &sPredMV, 0);
            break;

        case vc1_MBBackward:
            vc1PREDMV_PredictProgressiveMV(&sPredMV, pPosition, 0, 0, 1);
            vc1DECMV_ApplyMVPrediction(pState, 0, &sPredMV, 1);
            break;

        case vc1_MBInterp:
            vc1PREDMV_PredictProgressiveMV(&sPredMV, pPosition, 0, 0, 0);
            vc1DECMV_ApplyMVPrediction(pState, 0, &sPredMV, 0);
            vc1PREDMV_PredictProgressiveMV(&sPredMV, pPosition, 0, 0, 1);
            vc1DECMV_ApplyMVPrediction(pState, 0, &sPredMV, 1);
            break;
    }

    /* set chroma block types depending on luma blocks */
    vc1DERIVEMV_DecideChromaBlockType(pPosition);

    if (Coded)
    {
        int CBPCY = pMB->CBPCY;

        if (CBPCY==0)
        {
            /* Coded and CBPCY==0 means we still need to read the TTMB */
            pMB->CBPCY = 1;
        }
        eResult = vc1DECMB_DecodeTransformInfo(pState, pBitstream);
        if(vc1_ResultOK != eResult)
        {
            return(eResult);
        }
        pMB->CBPCY = (UBYTE8)CBPCY;
    }

    return(vc1_ResultOK);
}



/*
 * Description:
 * Unpack the macroblock layer for field interlaced B frames
 *
 * Remarks:
 * None.
 *
 * Inputs:
 * pState       - pointer to the decoder's state
 * pBitstream   - pointer to struct representing the current bitstream
 *
 * Outputs:
 * pState       - updated with read data
 * pBitstream   - updated with new position
 *
 * Return Value:
 * Standard vc1_eResult result. See enumeration for possible result codes.
 * 
 */

static vc1_eResult vc1DECMB_UnpackMacroblockFieldB(vc1DEC_sState * pState,
                                                   vc1DEC_sBitstream * pBitstream)
{
    vc1_eResult eResult;
    vc1_sMB *pMB = pState->sPosition.pCurMB;
    WORD32 Value;
    vc1_sMV sPredMV;
    const vc1DECMB_sMBMode *pMBMode;
    FLAG ForwardBit;
    FLAG PredictionFlag = FALSE;
    FLAG InterpMVPresentFlag = FALSE;
    vc1_sPosition *pPosition = &pState->sPosition;

    /* macroblock mode */
    Value = vc1DECBIT_GetVLC(pBitstream, pState->sPicParams.pMBModeTable);
    if(VC1DECBIT_EOF == Value)
    {
        return(vc1_ResultBufferExhausted);
    }
    DEBUG2(vc1DEBUG_MB, "MBMODE: %s (%d)\n", vc1DEBUG_MBModeIntField[Value], Value);
    COVERAGE("9.1.3.1");

    pMBMode = &vc1DECMB_Macroblock_Mode_Table[Value];
    DEBUG1(vc1DEBUG_CMP, "MBMode = %d\n", Value);
    COVERAGE("10.3.4.4.2");

    pMB->eMBType = pMBMode->eMBType;
    DEBUG3(vc1DEBUG_MB,
          "MBType = %d, MVPresent = %d, CBPPresent = %d\n",
          pMBMode->eMBType,
          pMBMode->MVPresent,
          pMBMode->CBPPresent);

    if(FALSE == vc1_MBTypeIsIntra(pMB->eMBType))
    {
        /* forward macroblock bit */
        Value = vc1DECBITPL_ReadBitplaneBit(&pState->sPicParams.sBPForwardMB, pBitstream);
        if(VC1DECBIT_EOF == Value)
        {
            return(vc1_ResultBufferExhausted);
        }
        ForwardBit = (FLAG)Value;
        DEBUG1(vc1DEBUG_MB, "FORWARDBIT: %d\n", ForwardBit);
        COVERAGE("9.1.3.20");
    }
    else
    {
        vc1DECBITPL_SkipBitplaneBit(&pState->sPicParams.sBPForwardMB);
        ForwardBit = FALSE;
    }


    /* B motion vector type */
    if( (FALSE == vc1_MBTypeIsIntra(pMB->eMBType))      && 
        (FALSE == ForwardBit)                           &&
        (FALSE == vc1_MBTypeIs4MV(pMB->eMBType))
      )
    {
        Value = vc1DECBIT_GetVLC(pBitstream, vc1DECMB_B_Field_Macroblock_Type);
        if(VC1DECBIT_EOF == Value)
        {
            return(vc1_ResultBufferExhausted);
        }
        DEBUG1(vc1DEBUG_MB, "BMVTYPE: %d\n", Value);
        COVERAGE("9.1.3.19");
        pMB->eMBType = (vc1_eMBType)(pMB->eMBType | Value);
    }
    else if(TRUE == ForwardBit)
    {
        pMB->eMBType = (vc1_eMBType)(pMB->eMBType | vc1_MBForward);
    }

    DEBUG1(vc1DEBUG_MB, "MB Type = %s\n", vc1DEBUG_MBType[pMB->eMBType & 15]);

    if(TRUE == vc1_MBTypeIs1MV(pMB->eMBType))
    {
        if(TRUE == vc1_MBTypeIsInterp(pMB->eMBType))
        {
            /* interpolated motion vector present */
            Value = vc1DECBIT_GetBits(pBitstream, VC1_BITS_INTERPMVP);
            if(VC1DECBIT_EOF == Value)
            {
                return(vc1_ResultBufferExhausted);
            }
            DEBUG1(vc1DEBUG_MB, "INTERPMVP: %d\n", Value);
            COVERAGE("9.1.3.21");
            InterpMVPresentFlag = (FLAG)Value;
        }

        if(TRUE == pMBMode->MVPresent)
        {
            DEBUG0(vc1DEBUG_MB, "UnpackMVDataInterlace, block 0, BMV1\n");
            ASSERT (FALSE == vc1_MBTypeIsDirect(pMB->eMBType));
            eResult = vc1DECMV_UnpackMVDataInterlace(pMB,
                                                     &PredictionFlag,
                                                     pState,
                                                     pBitstream,
                                                     0,
                                                     pPosition->NumRef,
                                                     0);
            if(vc1_ResultOK != eResult)
            {
                return(eResult);
            }
            COVERAGE("9.1.3.17");
        }
        else
        {
            int Count;
            for(Count = 0; Count < 4; Count++)
            {
                pMB->sBlk[Count].eBlkType = vc1_BlkInter8x8;
            }
        }

        if(TRUE == vc1_MBTypeIsBackward(pMB->eMBType))
        {
            /* copy the motion vectors from the forward to the backward set */
            int Count;
            for(Count = 0; Count < 4; Count++)
            {
                pMB->sBlk[Count].u.sInter.sMotion[1].sDMV.X 
                    = pMB->sBlk[Count].u.sInter.sMotion[0].sDMV.X;
                pMB->sBlk[Count].u.sInter.sMotion[1].sDMV.Y 
                    = pMB->sBlk[Count].u.sInter.sMotion[0].sDMV.Y;
            }
            vc1PREDMV_PredictInterlacedFieldMV( &sPredMV, 
                                                &pState->sPosition, 
                                                0, 
                                                0, 
                                                PredictionFlag, 
                                                1);
            vc1DECMV_ApplyMVPrediction(pState, 0, &sPredMV, 1);
        }
        else
        {
            vc1PREDMV_PredictInterlacedFieldMV( &sPredMV, 
                                                &pState->sPosition, 
                                                0, 
                                                0, 
                                                PredictionFlag, 
                                                0);
            vc1DECMV_ApplyMVPrediction(pState, 0, &sPredMV, 0);
        }

        if(TRUE == vc1_MBTypeIsInterp(pMB->eMBType))
        {
            if(TRUE == InterpMVPresentFlag)
            {
                DEBUG0(vc1DEBUG_MB, "UnpackMVDataInterlace, block 0, BMV2\n");
                eResult = vc1DECMV_UnpackMVDataInterlace(   pMB, 
                                                            &PredictionFlag, 
                                                            pState, 
                                                            pBitstream, 
                                                            0, 
                                                            pPosition->NumRef, 
                                                            1);
                if(vc1_ResultOK != eResult)
                {
                    return(eResult);
                }
                COVERAGE("9.1.3.18");
                COVERAGE("10.4.4.1");
            }
            else
            {
                pMB->sBlk[0].eBlkType = vc1_BlkInter8x8;
                PredictionFlag = FALSE;
            }

            vc1PREDMV_PredictInterlacedFieldMV(&sPredMV, pPosition, 0, 0, PredictionFlag, 1);
            vc1DECMV_ApplyMVPrediction(pState, 0, &sPredMV, 1);
        }

        
    }
    else if(TRUE == vc1_MBTypeIs4MV(pMB->eMBType))
    {
        int Backwards = 0;
        int MVBlockPattern;
        int Blk;

        if(FALSE == ForwardBit)
        {            
            pMB->eMBType = (vc1_eMBType)(pMB->eMBType | vc1_MBBackward);
            Backwards = 1;
        }

        DEBUG1(vc1DEBUG_MB, "MB Type = %s\n", vc1DEBUG_MBType[pMB->eMBType & 15]);

        /* 4MV block pattern */
        Value = vc1DECBIT_GetVLC(pBitstream, pState->sPicParams.pMB4MVBlockPatternTable);
        if(VC1DECBIT_EOF == Value)
        {
            return(vc1_ResultBufferExhausted);
        }
        DEBUG1(vc1DEBUG_MB, "4MVBP: %d\n", Value);
        COVERAGE("9.1.3.11");
        COVERAGE("10.3.4.2");

        MVBlockPattern = (UBYTE8)Value;

        for(Blk = 0; Blk < 4; Blk++)
        {
            int BlkMVDataPresent = ((0 != ( (1 << (3 - Blk)) & MVBlockPattern) ) ? 1 : 0);
            FLAG PredictionFlag = FALSE;

            if(1 == BlkMVDataPresent)
            {
                ASSERT (FALSE == vc1_MBTypeIsDirect(pMB->eMBType));
                DEBUG1(vc1DEBUG_MB, "UnpackMVDataInterlace, block %d\n", Blk);
                eResult = vc1DECMV_UnpackMVDataInterlace(   pMB, 
                                                            &PredictionFlag, 
                                                            pState, 
                                                            pBitstream, 
                                                            Blk, 
                                                            pPosition->NumRef, 
                                                            Backwards);
                if(vc1_ResultOK != eResult)
                {
                    return(eResult);
                }

                COVERAGE("7.1.3.9");
            }
            else
            {
                pMB->sBlk[Blk].eBlkType = vc1_BlkInter8x8;
            }

            vc1PREDMV_PredictInterlacedFieldMV( &sPredMV, 
                                                pPosition, 
                                                Blk, 
                                                Value, 
                                                PredictionFlag, 
                                                Backwards);
            vc1DECMV_ApplyMVPrediction(pState, Blk, &sPredMV, Backwards);
        }
    }

    if(FALSE == vc1_MBTypeIsIntra(pMB->eMBType))
    {
        /* coded block pattern */
        if(TRUE == pMBMode->CBPPresent)
        {
            Value = vc1DECBIT_GetVLC(pBitstream, pState->sPicParams.pCodedBlockPatternTable);
            if(VC1DECBIT_EOF == Value)
            {
                return(vc1_ResultBufferExhausted);
            }
            DEBUG1(vc1DEBUG_MB, "CBPCY: %d\n", Value);
            DEBUG1(vc1DEBUG_CMP, "CBPCY=%x\n", Value);
            COVERAGE("9.1.3.4");
            pMB->CBPCY = (UBYTE8)Value;
        }
        else
        {
            DEBUG0(vc1DEBUG_MB, "CBPCY: 0\n");
            pMB->CBPCY = 0;
        }
    }

    if( TRUE == pState->sPicParams.DQuantFrame  && 
        ((0 != pMB->CBPCY) || (TRUE == vc1_MBTypeIsIntra(pMBMode->eMBType))))
    {
        eResult = vc1DECMB_UnpackMBQuantParams(pState, pBitstream);
        if(vc1_ResultOK != eResult)
        {
            return(eResult);
        }
    }

    if(TRUE == vc1_MBTypeIsIntra(pMBMode->eMBType))
    {
        int Blk;

        COVERAGE("10.3.4.3");

        /* AC prediction */
        Value = vc1DECBIT_GetBits(pBitstream, VC1_BITS_ACPRED_MB);
        if(VC1DECBIT_EOF == Value)
        {
            return(vc1_ResultBufferExhausted);
        }
        DEBUG1(vc1DEBUG_MB, "ACPRED: %d\n", Value);
        COVERAGE("9.1.3.7");
        pMB->eACPred = (vc1_eACPred)Value;


        /* Coded block pattern */
        if(TRUE == pMBMode->CBPPresent)
        {
            Value = vc1DECBIT_GetVLC(pBitstream, pState->sPicParams.pCodedBlockPatternTable);
            if(VC1DECBIT_EOF == Value)
            {
                return(vc1_ResultBufferExhausted);
            }
            DEBUG1(vc1DEBUG_MB, "CBPCY: %d\n", Value);
            DEBUG1(vc1DEBUG_CMP, "CBPCY=%x\n", Value);
            COVERAGE("9.1.3.4");
            pMB->CBPCY = (UBYTE8)Value;
        }
        else
        {
            pMB->CBPCY = 0;
        }

        /* assign CBP data to blocks */
        for(Blk = 0; Blk < VC1_BLOCKS_PER_MB; Blk++)
        {
            /* Copy MB type into blocks */
            pMB->sBlk[Blk].eBlkType = vc1_BlkIntra;
        }

        /* set chroma block types depending on luma blocks */
        vc1DERIVEMV_DecideChromaBlockType(pPosition);
    }
    else
    {
        pMB->eACPred = vc1_ACPredOff;

        /* set chroma block types depending on luma blocks */
        vc1DERIVEMV_DecideChromaBlockType(pPosition);

        /* macroblock transform type */
        eResult = vc1DECMB_DecodeTransformInfo(pState, pBitstream);
        if(vc1_ResultOK != eResult)
        {
            return(eResult);
        }
    }

    return(vc1_ResultOK);
}






/*
 * Description:
 * Unpack the macroblock layer for frame interlaced B frames
 *
 * Remarks:
 * None.
 *
 * Inputs:
 * pState       - pointer to the decoder's state
 * pBitstream   - pointer to struct representing the current bitstream
 *
 * Outputs:
 * pState       - updated with read data
 * pBitstream   - updated with new position
 *
 * Return Value:
 * Standard vc1_eResult result. See enumeration for possible result codes.
 * 
 */

static vc1_eResult vc1DECMB_UnpackMacroblockFrameB( vc1DEC_sState * pState, 
                                                    vc1DEC_sBitstream * pBitstream)
{
    WORD32 Value;
    vc1_sPosition *pPosition = &pState->sPosition;
    vc1_sMB *pMB = pPosition->pCurMB;
    FLAG PredFlag = FALSE;
    vc1_sMV sPredMV;
    int MVBlockPattern = 0;
    vc1_eResult eResult;

    const vc1DECMB_sFrameIntMBMode *pMBMode = NULL;

    /* skipped macroblock flag */
    Value = vc1DECBITPL_ReadBitplaneBit(&pState->sPicParams.sBPSkipMB, pBitstream);
    if(VC1DECBIT_EOF == Value)
    {
        return(vc1_ResultBufferExhausted);
    }
    DEBUG1(vc1DEBUG_MB, "SKIPMBBIT: %d\n", Value);
    COVERAGE("9.1.3.3");
    pMB->Skipped = (FLAG)Value;

    if(FALSE == pMB->Skipped)
    {
        /* Macroblock mode flag */
        Value = vc1DECBIT_GetVLC(pBitstream, pState->sPicParams.pMBModeTable);
        if(VC1DECBIT_EOF == Value)
        {
            return(vc1_ResultBufferExhausted);
        }
        pMBMode = &vc1DECMB_Frame_Interlaced_Macroblock_Mode_Table[Value];
        DEBUG2(vc1DEBUG_MB, "MBMODE: %s (%d)\n", vc1DEBUG_MBModeIntFrame[Value], Value);
        pMB->eMBType = pMBMode->eMBType;

        if(TRUE == pMBMode->MVPresent)
        {
            /* a 1MV macroblock, with MV present */
            MVBlockPattern = 15;
        }
    }
    else
    {
        /* skipped macroblocks have 1MV */
        pMB->eMBType = vc1_MB1MV;
    }

    if(TRUE == vc1_MBTypeIsIntra(pMB->eMBType))
    {
        int Blk;

        pMB->eBlkType = vc1_BlkIntra;
        /* set all block types to intra */
        for(Blk = 0; Blk < VC1_BLOCKS_PER_MB; Blk++)
        {
            pMB->sBlk[Blk].eBlkType = pMB->eBlkType;
        }

        vc1DECBITPL_SkipBitplaneBit(&pState->sPicParams.sBPBFrameDirectMode);

        /* Field transform */
        Value = vc1DECBIT_GetBits(pBitstream, VC1_BITS_FIELDTX);
        if(VC1DECBIT_EOF == Value)
        {
            return(vc1_ResultBufferExhausted);
        }
        DEBUG1(vc1DEBUG_MB, "FIELDTX: %d\n", Value);
        COVERAGE("9.1.3.5");
        if(TRUE == Value)
        {
            pMB->eMBType = (vc1_eMBType)(pMB->eMBType | vc1_MBFieldTX);
        }


        /* CBP present flag */
        Value = vc1DECBIT_GetBits(pBitstream, VC1_BITS_CBPPRESENT);
        if(VC1DECBIT_EOF == Value)
        {
            return(vc1_ResultBufferExhausted);
        }

        if(TRUE == Value)
        {
            /* Coded block pattern */
            Value = vc1DECBIT_GetVLC(pBitstream, pState->sPicParams.pCodedBlockPatternTable);
            if(VC1DECBIT_EOF == Value)
            {
                return(vc1_ResultBufferExhausted);
            }
            DEBUG1(vc1DEBUG_MB, "CBPCY: %d\n", Value);
            COVERAGE("9.1.3.4");
            pMB->CBPCY = (UBYTE8)Value;
        }
        else
        {
            pMB->CBPCY = 0;
        }


        /* AC prediction */
        Value = vc1DECBIT_GetBits(pBitstream, VC1_BITS_ACPRED_MB);
        if(VC1DECBIT_EOF == Value)
        {
            return(vc1_ResultBufferExhausted);
        }
        DEBUG1(vc1DEBUG_MB, "ACPRED: %d\n", Value);
        COVERAGE("9.1.3.7");
        pMB->eACPred = (vc1_eACPred)Value;


        if(TRUE == pState->sPicParams.DQuantFrame)
        {
            eResult = vc1DECMB_UnpackMBQuantParams(pState, pBitstream);
            if(vc1_ResultOK != eResult)
            {
                return(eResult);
            }
        }

        /* set chroma block types depending on luma blocks */
        vc1DERIVEMV_DecideChromaBlockType(pPosition);
    }
    else    /* an inter macroblock */
    {
        int Blk, BlkLimit = 0;

        pMB->eBlkType = pState->sPicParams.eFrameTransformType;

        /* Direct bit */
        Value = vc1DECBITPL_ReadBitplaneBit(&pState->sPicParams.sBPBFrameDirectMode, pBitstream);
        if(VC1DECBIT_EOF == Value)
        {
            return(vc1_ResultBufferExhausted);
        }
        DEBUG1(vc1DEBUG_MB, "DIRECTBBIT: %d\n", Value);
        COVERAGE("9.1.3.16");
        if(TRUE == Value)
        {
            pMB->eMBType = (vc1_eMBType)((pMB->eMBType & ~vc1_MBDirMask) | vc1_MBDirect);

            /* set all block types to inter */
            for(Blk = 0; Blk < VC1_BLOCKS_PER_MB; Blk++)
            {
                pMB->sBlk[Blk].eBlkType = pMB->eBlkType;
            }
        }
        else
        {
            /* set the macroblock mv type to something other than direct */
            pMB->eMBType =  (vc1_eMBType)(pMB->eMBType | vc1_MBForward);
        }
        
        /* B motion vector type */
        if(FALSE == vc1_MBTypeIsDirect(pMB->eMBType))
        {
            if(vc1GENTAB_pBFraction[pPosition->BFraction].ScaleFactor < 128)
            {
                Value = vc1DECBIT_GetVLC(pBitstream,
                            vc1DECMB_B_Frame_Motion_Prediction_Type_BFRACTION_Less_Than_Half);
            }
            else
            {
                Value = vc1DECBIT_GetVLC(pBitstream,
                            vc1DECMB_B_Frame_Motion_Prediction_Type_BFRACTION_More_Than_Half);
            }

            if(VC1DECBIT_EOF == Value)
            {
                return(vc1_ResultBufferExhausted);
            }
            DEBUG1(vc1DEBUG_MB, "BMVTYPE: %d\n", Value);
            COVERAGE("9.1.3.19");
            pMB->eMBType = (vc1_eMBType)((pMB->eMBType & ~vc1_MBDirMask) | Value);
        }

        /* motion vector switch */
        if( TRUE  == vc1_MBTypeIsFieldMV(pMB->eMBType) &&
            FALSE == vc1_MBTypeIsInterp(pMB->eMBType)  &&
            FALSE == vc1_MBTypeIsDirect(pMB->eMBType)  )
        {
            Value = vc1DECBIT_GetBits(pBitstream, VC1_BITS_MVSW);
            if(VC1DECBIT_EOF == Value)
            {
                return(vc1_ResultBufferExhausted);
            }
            DEBUG1(vc1DEBUG_MB, "MVSW: %d\n", Value);
            COVERAGE("9.1.3.22");

            if(TRUE == Value)
            {
                pMB->eMBType = (vc1_eMBType)(pMB->eMBType | vc1_MBSwitchMV);
            }
        }

        /* coded block pattern */
        if(FALSE == pMB->Skipped)
        {
            if (pMBMode->CBPPresent)
            {
                Value = vc1DECBIT_GetVLC(pBitstream, pState->sPicParams.pCodedBlockPatternTable);
                if(VC1DECBIT_EOF == Value)
                    {
                    return(vc1_ResultBufferExhausted);
                }
                DEBUG1(vc1DEBUG_MB, "CBPCY: %d\n", Value);
                COVERAGE("9.1.3.4");
                pMB->CBPCY = (UBYTE8)Value;
            }
            else
            {
                pMB->CBPCY = 0;
            }
        }

        /* 2MV block pattern */
        if(FALSE == vc1_MBTypeIsDirect(pMB->eMBType))
        {
            BlkLimit = 1;

            /* 2MVBP exists for:    1MV Interp
             *                      2MV Forward
             *                      2MV Backward
             */
            if( ((TRUE == vc1_MBTypeIs1MV(pMB->eMBType)) && 
                (TRUE == vc1_MBTypeIsInterp(pMB->eMBType)))
                ||
                ((TRUE == vc1_MBTypeIs2MV(pMB->eMBType)) && 
                (TRUE == vc1_MBTypeIsForward(pMB->eMBType)))
                ||
                ((TRUE == vc1_MBTypeIs2MV(pMB->eMBType)) &&
                (TRUE == vc1_MBTypeIsBackward(pMB->eMBType)))   )
            {
                if(FALSE == pMB->Skipped)
                {
                    Value = vc1DECBIT_GetVLC(   pBitstream, 
                                                pState->sPicParams.pMB2MVBlockPatternTable);
                    if(VC1DECBIT_EOF == Value)
                    {
                        return(vc1_ResultBufferExhausted);
                    }
                    DEBUG1(vc1DEBUG_MB, "2MVBP: %d\n", Value);
                    COVERAGE("9.1.3.12");
                    MVBlockPattern = Value;
                }

                BlkLimit = 2;
            }

        }

        /* 4MV block pattern */
        if((TRUE == vc1_MBTypeIs2MV(pMB->eMBType)) && (TRUE == vc1_MBTypeIsInterp(pMB->eMBType)))
        {
            if(FALSE == pMB->Skipped)
            {
                /* 4MVBP exists for 2MV Interp */
                Value = vc1DECBIT_GetVLC(pBitstream, pState->sPicParams.pMB4MVBlockPatternTable);
                if(VC1DECBIT_EOF == Value)
                {
                    return(vc1_ResultBufferExhausted);
                }
                DEBUG1(vc1DEBUG_MB, "4MVBP: %d\n", Value);
                MVBlockPattern = Value;
            }

            BlkLimit = 4;
        }


        if(1 == BlkLimit)
        {
            /* Unpack a single motion vector */
            int Backwards = 0;
            vc1_sMV sPredMV;

            if(TRUE == vc1_MBTypeIsBackward(pMB->eMBType))
            {
                Backwards = 1;
            }

            if(0 != (1 & MVBlockPattern))
            {
                ASSERT (FALSE == vc1_MBTypeIsDirect(pMB->eMBType));
                eResult = vc1DECMV_UnpackMVDataInterlace(   pMB, 
                                                            &PredFlag, 
                                                            pState, 
                                                            pBitstream, 
                                                            0, 
                                                            0, 
                                                            Backwards);
                if(vc1_ResultOK != eResult)
                {
                    return(eResult);
                }
            }
            else
            {
                for(Blk = 0; Blk < 4; Blk++)
                {
                    pMB->sBlk[Blk].eBlkType = pMB->eBlkType;
                }                    
            }

            vc1PREDMV_PredictInterlacedFrameMV(&sPredMV, pPosition, 0, Backwards);
            vc1DECMV_ApplyMVPrediction(pState, 0, &sPredMV, Backwards);
        }
        else if(2 == BlkLimit)
        {
            /* Unpack two motion vectors */

            if(TRUE == vc1_MBTypeIsInterp(pMB->eMBType))
            {
                /* 1MV interpolated macroblock */
                int Backwards;
                for(Backwards = 0; Backwards < 2; Backwards++)
                {
                    if(0 != ((1 << (1 - Backwards)) & MVBlockPattern))
                    {
                        ASSERT (FALSE == vc1_MBTypeIsDirect(pMB->eMBType));
                        /* if bit 1 set, unpack the forward mv */
                        eResult = vc1DECMV_UnpackMVDataInterlace(   pMB, 
                                                                    &PredFlag, 
                                                                    pState, 
                                                                    pBitstream, 
                                                                    0, 
                                                                    0, 
                                                                    Backwards);
                        if(vc1_ResultOK != eResult)
                        {
                            return(eResult);
                        }
                    }
                    else
                    {
                        for(Blk = 0; Blk < 4; Blk++)
                        {
                            pMB->sBlk[Blk].eBlkType = pMB->eBlkType;
                        }
                        PredFlag = FALSE;
                    }

                    vc1PREDMV_PredictInterlacedFrameMV(&sPredMV, pPosition, 0, Backwards);
                    vc1DECMV_ApplyMVPrediction(pState, 0, &sPredMV, Backwards);
                }                
            }
            else if(TRUE == vc1_MBTypeIs2MV(pMB->eMBType))
            {
                /* 2MV forwards or backwards macroblock */
                int Backwards = 0, Bottom;
                if(TRUE == vc1_MBTypeIsBackward(pMB->eMBType))
                {
                    Backwards = 1;
                }

                for(Bottom = 0; Bottom < 2; Bottom++)
                {
                    int Direction = Backwards;

                    if(TRUE == vc1_MBTypeIsSwitchMV(pMB->eMBType))
                    {
                        Direction = Bottom;
                        if(TRUE == vc1_MBTypeIsBackward(pMB->eMBType))
                        {
                            Direction = 1 - Bottom;
                        }
                        COVERAGE("10.8.5.4");
                    }
                    DEBUG2(vc1DEBUG_MB, "2MVUnpack: Bottom=%d Direction=%d\n", Bottom, Direction);

                    if(0 != ((1 << (1 - Bottom)) & MVBlockPattern))
                    {
                        ASSERT (FALSE == vc1_MBTypeIsDirect(pMB->eMBType));
                        eResult = vc1DECMV_UnpackMVDataInterlace(pMB,
                                                                 &PredFlag,
                                                                 pState,
                                                                 pBitstream,
                                                                 Bottom * 2,
                                                                 0,
                                                                 Direction);
                        if(vc1_ResultOK != eResult)
                        {
                            return(eResult);
                        }
                    }
                    else
                    {
                        for(Blk = 0; Blk < 2; Blk++)
                        {
                            if(TRUE == vc1_MBTypeIsSwitchMV(pMB->eMBType))
                            {
                                int Direction = Bottom;
                                if(TRUE == vc1_MBTypeIsBackward(pMB->eMBType))
                                {
                                    Direction = 1 - Bottom;
                                }
                                COVERAGE("10.8.5.4");
                            }

                            pMB->sBlk[Blk + Bottom * 2].eBlkType = pMB->eBlkType;
                        }
                        PredFlag = FALSE;
                    }

                    vc1PREDMV_PredictInterlacedFrameMV(&sPredMV, pPosition, Bottom * 2, Direction);
                    vc1DECMV_ApplyMVPrediction(pState, Bottom * 2, &sPredMV, Direction);
                }
            }
        }
        else if(4 == BlkLimit)
        {
            int Backwards, Bottom;

            /* Unpack four motion vectors */
            /* 2MV interpolated macroblock */
            for(Blk = 0; Blk < BlkLimit; Blk++)
            {
                Backwards = 0;
                Bottom = 0;
                if((1 == Blk) || (3 == Blk))
                {
                    Backwards = 1;
                }
                if((2 == Blk) || (3 == Blk))
                {
                    Bottom = 1;
                }

                if(0 != ((1 << (3 - Blk)) & MVBlockPattern))
                {
                    ASSERT (FALSE == vc1_MBTypeIsDirect(pMB->eMBType));
                    eResult = vc1DECMV_UnpackMVDataInterlace(pMB,
                                                             &PredFlag,
                                                             pState,
                                                             pBitstream,
                                                             Bottom * 2,
                                                             0,
                                                             Backwards);
                    if(vc1_ResultOK != eResult)
                    {
                        return(eResult);
                    }
                }
                else
                {
                    int Blk2; 

                    for(Blk2 = 0; Blk2 < 2; Blk2++)
                    {
                        pMB->sBlk[Blk2 + Bottom * 2].eBlkType = pMB->eBlkType;
                    }
                }

                vc1PREDMV_PredictInterlacedFrameMV(&sPredMV, pPosition, Bottom * 2, Backwards);
                vc1DECMV_ApplyMVPrediction(pState, Bottom * 2, &sPredMV, Backwards);
            }
        }

        /* set chroma block types depending on luma blocks */
        vc1DERIVEMV_DecideChromaBlockType(pPosition);

        if(FALSE == pMB->Skipped)
        {
            if((TRUE == pState->sPicParams.DQuantFrame) && (0 != pMB->CBPCY))
            {
                eResult = vc1DECMB_UnpackMBQuantParams(pState, pBitstream);
                if(vc1_ResultOK != eResult)
                {
                    return(eResult);
                }
            }

            if(1 == pState->sSeqParams.VSTransform)
            {
                if((FALSE == pState->sPicParams.MBTransformTypeFlag) && (0 != pMB->CBPCY))
                {
                    eResult = vc1DECMB_UnpackTTMB(pState, pBitstream);
                    if(vc1_ResultOK != eResult)
                    {
                        return(eResult);
                    }
                }
            }
        }
    }

    return(vc1_ResultOK);
}
































/*
 * Description:
 * Unpack the macroblock layer for interlaced field macroblocks in P fields
 *
 * Remarks:
 * None.
 *
 * Inputs:
 * pState       - pointer to the decoder's state
 * pBitstream   - pointer to struct representing the current bitstream
 *
 * Outputs:
 * pState       - updated with read data
 * pBitstream   - updated with new position
 *
 * Return Value:
 * Standard vc1_eResult result. See enumeration for possible result codes.
 * 
 */

static vc1_eResult vc1DECMB_UnpackMacroblockFieldP( vc1DEC_sState * pState, 
                                                    vc1DEC_sBitstream * pBitstream)
{
    WORD32 Value;
    vc1_sPosition *pPosition = &pState->sPosition;
    vc1_sMB *pMB = pPosition->pCurMB;
    vc1_eResult eResult;
    vc1_sMV sPredMV;
    const vc1DECMB_sMBMode *pMBMode;
    UBYTE8 MVBlockPattern;
    int Blk, BlkLimit;

    /* Macroblock mode */
    Value = vc1DECBIT_GetVLC(pBitstream, pState->sPicParams.pMBModeTable);
    if(VC1DECBIT_EOF == Value)
    {
        return(vc1_ResultBufferExhausted);
    }
    DEBUG2(vc1DEBUG_MB, "MBMODE: %s (%d)\n", vc1DEBUG_MBModeIntField[Value], Value);
    COVERAGE("9.1.3.1");

    pMBMode = &vc1DECMB_Macroblock_Mode_Table[Value];
    COVERAGE("10.3.4.4.2");


    DEBUG1(vc1DEBUG_CMP, "MBMode = %d\n", Value);
    pMB->eMBType = pMBMode->eMBType;

    /* if the macroblock is inter, it must be forward predicted */
    if(FALSE == vc1_MBTypeIsIntra(pMB->eMBType))
    {
        pMB->eMBType = (vc1_eMBType)(pMB->eMBType | vc1_MBForward);
    }
    DEBUG1(vc1DEBUG_MB, "MB Type = %s\n", vc1DEBUG_MBType[pMB->eMBType & 15]);


    if(TRUE == vc1_MBTypeIs4MV(pMBMode->eMBType))
    {
        BlkLimit = 4;

        /* 4MV block pattern */
        Value = vc1DECBIT_GetVLC(pBitstream, pState->sPicParams.pMB4MVBlockPatternTable);
        if(VC1DECBIT_EOF == Value)
        {
            return(vc1_ResultBufferExhausted);
        }
        DEBUG1(vc1DEBUG_MB, "4MVBP: %d\n", Value);
        COVERAGE("10.3.4.2");

        MVBlockPattern = (UBYTE8)Value;
    }
    else
    {
        if(TRUE == pMBMode->MVPresent)
        {
            /* Indicate MV data for all blocks (though only 1MV represents this) */
            MVBlockPattern = 15;
        }
        else
        {
            /* No MV data present */
            MVBlockPattern = 0;
        }
        BlkLimit = 1;
        COVERAGE("10.3.4.1");
    }



    if(vc1_MBIntra != pMBMode->eMBType)
    {
        for(Blk = 0; Blk < BlkLimit; Blk++)
        {
            /* if the 4MVBP indicates MV data is present for this block */
            int BlkMVDataPresent = ((0 != ( (1 << (3 - Blk)) & MVBlockPattern) ) ? 1 : 0);
            FLAG PredictionFlag = FALSE;

            if(1 == BlkMVDataPresent)
            {
                eResult = vc1DECMV_UnpackMVDataInterlace(   pMB, 
                                                            &PredictionFlag, 
                                                            pState, 
                                                            pBitstream, 
                                                            Blk, 
                                                            pPosition->NumRef, 
                                                            0);
                if(vc1_ResultOK != eResult)
                {
                    return(eResult);
                }

                COVERAGE("7.1.3.9");
            }
            else
            {
                pMB->sBlk[Blk].eBlkType = vc1_BlkInter8x8;
                pMB->sBlk[Blk].u.sInter.sMotion[0].sDMV.X = 0;
                pMB->sBlk[Blk].u.sInter.sMotion[0].sDMV.Y = 0;
            }

            /* sniff the hybridpred bit, but don't read it, as the prediction might not need it */
            Value = vc1DECBIT_LookBits(pBitstream, VC1_BITS_HYBRIDPRED);
            if(VC1DECBIT_EOF == Value)
            {
                return(vc1_ResultBufferExhausted);
            }

            if(TRUE == vc1PREDMV_PredictInterlacedFieldMV(  &sPredMV, 
                                                            pPosition, 
                                                            Blk, 
                                                            Value, 
                                                            PredictionFlag, 
                                                            0))
            {
                /* the prediction used the hybrid bit - advance the bitstream */
                vc1DECBIT_GetBits(pBitstream, VC1_BITS_HYBRIDPRED);
                DEBUG1(vc1DEBUG_MB | vc1DEBUG_CMP, "HYBRIDPRED: %d\n", Value);
                COVERAGE("9.1.3.14");
            }
            else
            {
                /* the prediction didn't use the hybrid bit - do nothing */
            }

            vc1DECMV_ApplyMVPrediction(pState, Blk, &sPredMV, 0);
        }    

        if(vc1_MB1MV == pMBMode->eMBType)
        {
            /* copy data from the first block into other blocks */
            for(Blk = 1; Blk < VC1_BLOCKS_PER_MB; Blk++)
            {
                pMB->sBlk[Blk].eBlkType         = pMB->sBlk[0].eBlkType;
                if(TRUE == vc1_BlkNumIsLuma(Blk))
                {
                    pMB->sBlk[Blk].u.sInter.sMotion[0].sDMV.X 
                        = pMB->sBlk[0].u.sInter.sMotion[0].sDMV.X;
                    pMB->sBlk[Blk].u.sInter.sMotion[0].sDMV.Y
                        = pMB->sBlk[0].u.sInter.sMotion[0].sDMV.Y;
                }
            }
        }

        /* Coded block pattern */
        if(TRUE == pMBMode->CBPPresent)
        {
            Value = vc1DECBIT_GetVLC(pBitstream, pState->sPicParams.pCodedBlockPatternTable);
            if(VC1DECBIT_EOF == Value)
            {
                return(vc1_ResultBufferExhausted);
            }
            DEBUG1(vc1DEBUG_MB, "CBPCY: %d\n", Value);
            DEBUG1(vc1DEBUG_CMP, "CBPCY=%x\n", Value);
            COVERAGE("9.1.3.4");
            pMB->CBPCY = (UBYTE8)Value;
        }
        else
        {
            DEBUG1(vc1DEBUG_MB, "CBPCY: 0\n", Value);
            pMB->CBPCY = 0;
        }
    }

    /* set chroma block types depending on luma blocks */
    vc1DERIVEMV_DecideChromaBlockType(pPosition);

    if( (TRUE == pState->sPicParams.DQuantFrame) && 
        ((0 != pMB->CBPCY) || (TRUE == vc1_MBTypeIsIntra(pMB->eMBType))))
    {
        eResult = vc1DECMB_UnpackMBQuantParams(pState, pBitstream);
        if(vc1_ResultOK != eResult)
        {
            return(eResult);
        }
    }


    if(vc1_MBIntra == pMBMode->eMBType)
    {
        COVERAGE("10.3.4.3");


        /* AC prediction */
        Value = vc1DECBIT_GetBits(pBitstream, VC1_BITS_ACPRED_MB);
        if(VC1DECBIT_EOF == Value)
        {
            return(vc1_ResultBufferExhausted);
        }
        DEBUG1(vc1DEBUG_MB, "ACPRED: %d\n", Value);
        COVERAGE("9.1.3.7");
        pMB->eACPred = (vc1_eACPred)Value;


        /* Coded block pattern */
        if(TRUE == pMBMode->CBPPresent)
        {
            Value = vc1DECBIT_GetVLC(pBitstream, pState->sPicParams.pCodedBlockPatternTable);
            if(VC1DECBIT_EOF == Value)
            {
                return(vc1_ResultBufferExhausted);
            }
            DEBUG1(vc1DEBUG_MB, "CBPCY: %d\n", Value);
            DEBUG1(vc1DEBUG_CMP, "CBPCY=%x\n", Value);
            COVERAGE("9.1.3.4");
            pMB->CBPCY = (UBYTE8)Value;
        }
        else
        {
            pMB->CBPCY = 0;
        }

        for(Blk = 0; Blk < VC1_BLOCKS_PER_MB; Blk++)
        {
            /* Copy MB type into all blocks */
            pMB->sBlk[Blk].eBlkType = vc1_BlkIntra;
        }
    }
    else
    {
        /* macroblock transform type */
        eResult = vc1DECMB_DecodeTransformInfo(pState, pBitstream);
        if(vc1_ResultOK != eResult)
        {
            return(eResult);
        }
    }

    return(vc1_ResultOK);
}



/*
 * Description:
 * Unpack the macroblock layer for interlaced frame macroblocks in P fields
 *
 * Remarks:
 * None.
 *
 * Inputs:
 * pState       - pointer to the decoder's state
 * pBitstream   - pointer to struct representing the current bitstream
 *
 * Outputs:
 * pState       - updated with read data
 * pBitstream   - updated with new position
 *
 * Return Value:
 * Standard vc1_eResult result. See enumeration for possible result codes.
 * 
 */

static vc1_eResult vc1DECMB_UnpackMacroblockFrameP( vc1DEC_sState * pState, 
                                                    vc1DEC_sBitstream * pBitstream)
{
    WORD32 Value;
    vc1_sPosition *pPosition = &pState->sPosition;
    vc1_sMB *pMB = pPosition->pCurMB;
    vc1_eResult eResult;
    const vc1DECMB_sFrameIntMBMode *pMBMode;
    int Blk;


    /* Skipped macroblock flag */
    Value = vc1DECBITPL_ReadBitplaneBit(&pState->sPicParams.sBPSkipMB, pBitstream);
    if(VC1DECBIT_EOF == Value)
    {
        return(vc1_ResultBufferExhausted);
    }
    DEBUG1(vc1DEBUG_MB, "SKIPMBBIT: %d\n", Value);
    COVERAGE("9.1.3.3");
    pMB->Skipped = (FLAG)Value;
    if(TRUE == Value)
    {
        vc1_sMV sPredMV;

        COVERAGE("10.7.2.2");

        pMB->eMBType = (vc1_eMBType)(vc1_MB1MV | vc1_MBForward);
        for(Blk = 0; Blk < VC1_BLOCKS_PER_MB; Blk++)
        {
            pMB->sBlk[Blk].eBlkType = pState->sPicParams.eFrameTransformType;
        }

        vc1PREDMV_PredictInterlacedFrameMV(&sPredMV, pPosition, 0, 0);
        vc1DECMV_ApplyMVPrediction(pState, 0, &sPredMV, 0);
    }


    /* if this is a skipped mb, no other information in the mb layer, otherwise... */
    if(FALSE == pMB->Skipped)
    {
        /* Macroblock mode flag */
        Value = vc1DECBIT_GetVLC(pBitstream, pState->sPicParams.pMBModeTable);
        if(VC1DECBIT_EOF == Value)
        {
            return(vc1_ResultBufferExhausted);
        }
        pMBMode = &vc1DECMB_Frame_Interlaced_Macroblock_Mode_Table[Value];
        DEBUG2(vc1DEBUG_MB, "MBMODE: %s (%d)\n", vc1DEBUG_MBModeIntFrame[Value], Value);
        COVERAGE("10.7.2.3");
        pMB->eMBType = pMBMode->eMBType;

        if(TRUE == vc1_MBTypeIsIntra(pMB->eMBType))
        {
            pMB->eBlkType = vc1_BlkIntra;

            /* Field transform */
            Value = vc1DECBIT_GetBits(pBitstream, VC1_BITS_FIELDTX);
            if(VC1DECBIT_EOF == Value)
            {
                return(vc1_ResultBufferExhausted);
            }
            DEBUG1(vc1DEBUG_MB, "FIELDTX: %d\n", Value);
            COVERAGE("9.1.3.5");
            if(TRUE == Value)
            {
                pMB->eMBType = (vc1_eMBType)(pMB->eMBType | vc1_MBFieldTX);
            }
    

            /* CBP present flag */
            Value = vc1DECBIT_GetBits(pBitstream, VC1_BITS_CBPPRESENT);
            if(VC1DECBIT_EOF == Value)
            {
                return(vc1_ResultBufferExhausted);
            }

            COVERAGE("9.1.3.6");

            if(TRUE == Value)
            {
                /* Coded block pattern */
                Value = vc1DECBIT_GetVLC(pBitstream, pState->sPicParams.pCodedBlockPatternTable);
                if(VC1DECBIT_EOF == Value)
                {
                    return(vc1_ResultBufferExhausted);
                }
                DEBUG1(vc1DEBUG_MB, "CBPCY: %d\n", Value);
                COVERAGE("9.1.3.4");
                pMB->CBPCY = (UBYTE8)Value;
            }
            else
            {
                pMB->CBPCY = 0;
            }

            /* assign CBP data to blocks */
            for(Blk = 0; Blk < VC1_BLOCKS_PER_MB; Blk++)
            {
                pMB->sBlk[Blk].eBlkType = pMB->eBlkType;
            }

            /* AC prediction */
            Value = vc1DECBIT_GetBits(pBitstream, VC1_BITS_ACPRED_MB);
            if(VC1DECBIT_EOF == Value)
            {
                return(vc1_ResultBufferExhausted);
            }
            DEBUG1(vc1DEBUG_MB, "ACPRED: %d\n", Value);
            COVERAGE("9.1.3.7");
            pMB->eACPred = (vc1_eACPred)Value;

            /* set chroma block types depending on luma blocks */
            vc1DERIVEMV_DecideChromaBlockType(pPosition);

            if(TRUE == pState->sPicParams.DQuantFrame)
            {
                eResult = vc1DECMB_UnpackMBQuantParams(pState, pBitstream);
                if(vc1_ResultOK != eResult)
                {
                    return(eResult);
                }
            }
        }
        else    /* an inter macroblock */
        {
            int BlkLimit = 1;

            pMB->eBlkType = pState->sPicParams.eFrameTransformType;

            /* MB type must be forward in a p frame, if it's inter */
            pMB->eMBType = (vc1_eMBType)(pMB->eMBType | vc1_MBForward);

            if (pMBMode->CBPPresent)
            {
                /* Coded block pattern */
                Value = vc1DECBIT_GetVLC(pBitstream, pState->sPicParams.pCodedBlockPatternTable);
                if(VC1DECBIT_EOF == Value)
                {
                    return(vc1_ResultBufferExhausted);
                }
                DEBUG1(vc1DEBUG_MB, "CBPCY: %d\n", Value);
                COVERAGE("9.1.3.4");
                pMB->CBPCY = (UBYTE8)Value;
            }
            else
            {
                pMB->CBPCY = 0;
            }


            /* 2mv/4mv block pattern */
            if(TRUE == vc1_MBTypeIs2MV(pMB->eMBType))
            {
                Value = vc1DECBIT_GetVLC(pBitstream, pState->sPicParams.pMB2MVBlockPatternTable);
                if(VC1DECBIT_EOF == Value)
                {
                    return(vc1_ResultBufferExhausted);
                }
                DEBUG1(vc1DEBUG_MB, "2MVBP: %d\n", Value);

                /*
                 * shift bit 1 to bit 3, and bit 0 to bit 1, to match the other MVBPs.
                 * set bits 0 and 2 to zero
                 */
                pMB->MVBP = (UBYTE8)(((Value & 2) << 2) | ((Value & 1) << 1));

                BlkLimit = 4;
            }
            else if(TRUE == vc1_MBTypeIs4MV(pMB->eMBType))
            {
                Value = vc1DECBIT_GetVLC(pBitstream, pState->sPicParams.pMB4MVBlockPatternTable);
                if(VC1DECBIT_EOF == Value)
                {
                    return(vc1_ResultBufferExhausted);
                }
                DEBUG1(vc1DEBUG_MB, "4MVBP: %d\n", Value);
                pMB->MVBP = (UBYTE8)Value;

                BlkLimit = 4;
            }
            else /* 1MV macroblock */
            {
                pMB->MVBP = 0;
                if(TRUE == pMBMode->MVPresent)
                {
                    /* 1MV - set all bits in block pattern, though only 1MV is unpacked */
                    pMB->MVBP = 15;
                }
            }


            /* predict, unpack and apply prediction to the motion vectors */
            COVERAGE("10.7.2.5");
            for(Blk = 0; Blk < BlkLimit; Blk++)
            {
                vc1_sMV sPredMV;
                /* if the block pattern indicates MV data is present for this block */
                int BlkMVPresent = ((0 != ( (1 << (3 - Blk)) & pMB->MVBP) ) ? 1 : 0);            
                FLAG PredFlag = FALSE;

                if(1 == BlkMVPresent)
                {
                    eResult = vc1DECMV_UnpackMVDataInterlace(   pMB, 
                                                                &PredFlag, 
                                                                pState, 
                                                                pBitstream, 
                                                                Blk, 
                                                                0, 
                                                                0);
                    if(vc1_ResultOK != eResult)
                    {
                        return(eResult);
                    }
                }
                else
                {
                    pMB->sBlk[Blk].eBlkType = pMB->eBlkType;
                }

                /* if this is not blocks 1 and 3 in a 2MV macroblock */
                if(!( (TRUE == vc1_MBTypeIs2MV(pMB->eMBType)) && ((1 == Blk) || (3 == Blk))))
                {
                    vc1PREDMV_PredictInterlacedFrameMV(&sPredMV, pPosition, Blk, 0);
                    vc1DECMV_ApplyMVPrediction(pState, Blk, &sPredMV, 0);
                }
            }


            if(TRUE == vc1_MBTypeIs1MV(pMB->eMBType))
            {
                /* set the block type of the all blocks to that of the first one */
                for(Blk = 1; Blk < 4; Blk++)
                {
                    pMB->sBlk[Blk].eBlkType = pMB->sBlk[0].eBlkType;
                }
            }

            /* set chroma block types depending on luma blocks */
            vc1DERIVEMV_DecideChromaBlockType(pPosition);

            if((TRUE == pState->sPicParams.DQuantFrame) && (0 != pMB->CBPCY))
            {
                eResult = vc1DECMB_UnpackMBQuantParams(pState, pBitstream);
                if(vc1_ResultOK != eResult)
                {
                    return(eResult);
                }
            }

            /* unpack the transform type */
            if( (FALSE == pState->sPicParams.MBTransformTypeFlag)   && 
                (0 != pMB->CBPCY)                                   && 
                (TRUE == pState->sSeqParams.VSTransform)            )
            {
                eResult = vc1DECMB_UnpackTTMB(pState, pBitstream);
                if(vc1_ResultOK != eResult)
                {
                    return(eResult);
                }
            }
        }
    }

    return(vc1_ResultOK);
}

/*
 * Description:
 * Unpack the macroblock layer of the bitstream, and put the values into state
 *
 * Remarks:
 *
 * Inputs:
 * pState     - decoder state structure, into which data will be put
 * pBitstream - structure representing position in bitstream
 *
 * Outputs:
 * pState     - updated with data from bitstream
 * pBitstream - updated with new position in bitstream
 *
 * Return Value:
 * Standard vc1_eResult result. See enumeration for possible result codes.
 *
 */


static vc1_eResult vc1DECMB_UnpackMacroblockI(  vc1DEC_sState * pState, 
                                                vc1DEC_sBitstream * pBitstream)
{
    WORD32 Value;
    vc1_sPosition *pPosition = &pState->sPosition;
    vc1_sMB *pMB = pPosition->pCurMB;
    int     i;
    vc1_eResult eResult;

    pMB->eMBType = vc1_MBIntra;


    if (vc1_InterlacedFrame == pPosition->ePictureFormat)
    {
        /* FIELDTX */
        Value = vc1DECBITPL_ReadBitplaneBit(&pState->sPicParams.sBPFieldTX, pBitstream);
        if(VC1DECBIT_EOF == Value)
        {
            return(vc1_ResultBufferExhausted);
        }
        DEBUG1(vc1DEBUG_MB, "FIELDTX: %d\n", Value);
        COVERAGE("9.1.3.5");
        if(TRUE == Value)
        {
            pMB->eMBType = (vc1_eMBType)(pMB->eMBType | vc1_MBFieldTX);
        }
    }

    /* CBPCY */
    Value = vc1DECBIT_GetVLC(pBitstream, vc1DEC_I_Picture_CBPCY_VLC);
    if(VC1DECBIT_EOF == Value)
    {
        return(vc1_ResultBufferExhausted);
    }
    DEBUG1(vc1DEBUG_MB, "Read CBPCY: 0x%02X\n", Value);
    COVERAGE("7.1.3.4");

    pMB->CBPCY = (UBYTE8)vc1PREDCBP_ApplyCBPCYPred(pPosition, Value);
    DEBUG1(vc1DEBUG_MB, "Predicted CBPCY = 0x%02X\n", pMB->CBPCY);
    COVERAGE("8.1.1.5");

    DEBUG1(vc1DEBUG_MB, "Calculated CBPCY: 0x%02X\n", pMB->CBPCY);
    DEBUG1(vc1DEBUG_MB | vc1DEBUG_CMP, "CBPCY = %02x\n", pMB->CBPCY);

    /* ACPRED */
    if(vc1_ProfileAdvanced == pState->sSeqParams.eProfile)
    {
        Value = vc1DECBITPL_ReadBitplaneBit(&pState->sPicParams.sBPPredictAC, pBitstream);
    }
    else
    {
        Value = vc1DECBIT_GetBits(pBitstream, VC1_BITS_ACPRED_MB);
    }
    if(VC1DECBIT_EOF == Value)
    {
        return(vc1_ResultBufferExhausted);
    }
    DEBUG1(vc1DEBUG_MB, "ACPRED: %d\n", Value);
    COVERAGE("7.1.3.5");
    pMB->eACPred = (vc1_eACPred)Value;


    /* CONDOVER */
    if(vc1_ProfileAdvanced == pState->sSeqParams.eProfile)
    {
        if(vc1_CondOverSome == pState->sPicParams.eConditionalOverlap)
        {
            /* conditional overlap flags */

            Value = vc1DECBITPL_ReadBitplaneBit(&pState->sPicParams.sBPOverflags, pBitstream);
            if(VC1DECBIT_EOF == Value)
            {
                return(vc1_ResultBufferExhausted);
            }
            DEBUG1(vc1DEBUG_MB, "OVERFLAGMB: %d\n", Value);
            COVERAGE_NONINT_INT("7.1.3.1","9.1.3.2");
            pMB->OverlapFilter = (FLAG)Value;
        }

        COVERAGE("8.5.2");
    }

    /* MQDIFF */
    if(vc1_ProfileAdvanced == pState->sSeqParams.eProfile)
    {
        if(TRUE == pState->sPicParams.DQuantFrame)
        {
            eResult = vc1DECMB_UnpackMBQuantParams(pState, pBitstream);
            if(vc1_ResultOK != eResult)
            {
                return(eResult);
            }
        }
    }

    /* I frame - all blocks are intra */
    for(i = 0; i < VC1_BLOCKS_PER_MB; i++)
    {
        pMB->sBlk[i].eBlkType = vc1_BlkIntra;
    }

    return(vc1_ResultOK);
}






/*
 * Description:
 * Unpack the macroblock layer of the bitstream, and put the values into state
 *
 * Remarks:
 *
 * Inputs:
 * pState     - decoder state structure, into which data will be put
 * pBitstream - structure representing position in bitstream
 *
 * Outputs:
 * pState     - updated with data from bitstream
 * pBitstream - updated with new position in bitstream
 *
 * Return Value:
 * Standard vc1_eResult result. See enumeration for possible result codes.
 *
 */

vc1_eResult vc1DECMB_UnpackMacroblockLayer( vc1DEC_sState * pState, 
                                            vc1DEC_sBitstream * pBitstream)
{
    vc1_sPosition *pPosition = &pState->sPosition;
    vc1_sMB *pMB = pPosition->pCurMB;
    vc1_eResult eResult;
    int Blk;

    DEBUG2(vc1DEBUG_MB, "X: %d, Y: %d\n", pPosition->X, pPosition->SliceY + pPosition->Y);

    /*
     *  Copy higher layer data into MB, where necessary, and
     *  initialise the rest to zero.
     */

    pMB->sQuant.Quant    = pState->sPicParams.PQuant;
    pMB->sQuant.HalfStep = pState->sPicParams.HalfQPStep;
    pMB->OverlapFilter   = (FLAG)pState->sPicParams.eConditionalOverlap;
    pMB->Skipped         = FALSE;
    pMB->eBlkType        = pState->sPicParams.eFrameTransformType;
    pMB->CBPCY           = 0;
    pMB->eACPred         = vc1_ACPredOff;

    vc1IQUANT_ChooseQuantizer(  &pMB->sQuant, pPosition, 
                                pState->sPicParams.eQuantMode, 
                                pState->sPicParams.AltPQuant);

    for (Blk = 0; Blk < VC1_BLOCKS_PER_MB; Blk++)
    {
        pMB->sBlk[Blk].Coded = 0;
        pMB->sBlk[Blk].u.sInter.sMotion[0].sDMV.X = 0;
        pMB->sBlk[Blk].u.sInter.sMotion[0].sDMV.Y = 0;
        pMB->sBlk[Blk].u.sInter.sMotion[0].sDMV.BottomField = 0;
        pMB->sBlk[Blk].u.sInter.sMotion[1].sDMV.X = 0;
        pMB->sBlk[Blk].u.sInter.sMotion[1].sDMV.Y = 0;
        pMB->sBlk[Blk].u.sInter.sMotion[1].sDMV.BottomField = 0;
    }

    switch(pState->sPicParams.eQuantizer)
    {
        case vc1_QuantizerUniform:      
            pMB->sQuant.NonUniform = 0; 
            break;

        case vc1_QuantizerNonUniform:   
            pMB->sQuant.NonUniform = 1; 
            break;

        default:
        {
            FATAL("vc1DECMB_UnpackMacroblockLayer: Unknown quantizer %d\n", 
                pState->sPicParams.eQuantizer);
            return(vc1_ResultInvalidParameter);
        }
    }

    DEBUG2(vc1DEBUG_MB | vc1DEBUG_CMP, "---------- X=%03d Y=%03d\n", pPosition->X, pPosition->Y);

    switch(pPosition->ePictureType)
    {
        case vc1_PictureTypeBI: /* fall through */
        case vc1_PictureTypeI:
            eResult = vc1DECMB_UnpackMacroblockI(pState, pBitstream); break;

        case vc1_PictureTypeP:
            if(vc1_ProgressiveFrame == pPosition->ePictureFormat)
            {
                DEBUG0(vc1DEBUG_MB, "UnpackMacroblockProgP()\n");
                eResult = vc1DECMB_UnpackMacroblockProgP(pState, pBitstream);
            }
            else if(vc1_InterlacedField == pPosition->ePictureFormat)
            {
                DEBUG0(vc1DEBUG_MB, "UnpackMacroblockFieldP()\n");
                eResult = vc1DECMB_UnpackMacroblockFieldP(pState, pBitstream);
            }
            else
            {
                DEBUG0(vc1DEBUG_MB, "UnpackMacroblockFrameP()\n");
                eResult = vc1DECMB_UnpackMacroblockFrameP(pState, pBitstream);
            }
            break;

        case vc1_PictureTypeB:
            if(vc1_ProgressiveFrame == pPosition->ePictureFormat)
            {
                DEBUG0(vc1DEBUG_MB, "UnpackMacroblockProgB()\n");
                eResult = vc1DECMB_UnpackMacroblockProgB(pState, pBitstream);
            }
            else if(vc1_InterlacedField == pPosition->ePictureFormat)
            {
                DEBUG0(vc1DEBUG_MB, "UnpackMacroblockFieldB()\n");
                eResult = vc1DECMB_UnpackMacroblockFieldB(pState, pBitstream);
            }
            else
            {
                DEBUG0(vc1DEBUG_MB, "UnpackMacroblockFrameB()\n");
                eResult = vc1DECMB_UnpackMacroblockFrameB(pState, pBitstream);
            }
            break;

        default:
        {
            FATAL("vc1DECMB_UnpackMacroblockLayer: Unknown picture type - %d\n", 
                pPosition->ePictureType);
            return(vc1_ResultFatal);
        }
    }
    if(vc1_ResultOK != eResult)
    {
        return(eResult);
    }

    /* assign the macroblock CBP to the coded flag in each block */
    vc1DECMB_AssignCodedBlockPattern(pPosition->pCurMB);

    /* No overlap smoothing in B pictures */
    if(vc1_PictureTypeB == pPosition->ePictureType)
    {
        pMB->OverlapFilter = FALSE;
    }

    DEBUGMB(vc1DEBUG_MB, pPosition);

    /* Decode the block layer */
    eResult = vc1DECBLK_DecodeBlockLayer(pState, pBitstream);
    if(vc1_ResultOK != eResult)
    {
       return(eResult);
    }

    if(FALSE == pMB->Skipped)
    {
        DEBUG1(vc1DEBUG_CMP, "RndCtrl = %d\n", pState->sPicParams.sInterpolate.RndCtrl);

        for(Blk = 0; Blk < 4; Blk++)
        {
            vc1_sBlk * pBlk = &pPosition->pCurMB->sBlk[Blk];

            if(FALSE == vc1_BlkTypeIsIntra(pBlk->eBlkType))
            {
                DEBUG3( vc1DEBUG_MV | vc1DEBUG_CMP, 
                    "MV Inter Forwards  X=%5d, Y=%5d, Coded=%d\n", 
                    pBlk->u.sInter.sMotion[0].sMV.X, pBlk->u.sInter.sMotion[0].sMV.Y, pBlk->Coded);
                if(vc1_PictureTypeB == pPosition->ePictureType)
                {
                    DEBUG3(vc1DEBUG_MV | vc1DEBUG_CMP, 
                        "MV Inter Backwards X=%5d, Y=%5d, Coded=%d\n", 
                        pBlk->u.sInter.sMotion[1].sMV.X, 
                        pBlk->u.sInter.sMotion[1].sMV.Y,
                        pBlk->Coded);
                }
            }
            else
            {
                DEBUG1(vc1DEBUG_MV | vc1DEBUG_CMP,
                    "MV Intra Coded=%d\n", (pPosition->pCurMB->CBPCY >> (5-Blk))&1);
            }        
        }
    }

    pPosition->pCurMB++;

    return(vc1_ResultOK);
}
