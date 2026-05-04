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
 * vc1dec3dh.c
 *
 * vc1 Decoder 3D Huffman module
 *
 *  
 */

#include "vc1types.h"
#include "vc1dec.h"
#include "vc1decbit.h"
#include "vc1dec3dh.h"
#include "vc1dec3dhtab.h"
#include "vc13dhtab.h"
#include "vc1debug.h"

static const vc1DEC_sVLCCode vc1DEC3DH_Escape_Decoding_Mode[4] =
{
    {0, 3, 2},
    {1, 1, vc1DEC3DH_EscapeMode1},
    {1, 2, vc1DEC3DH_EscapeMode2},
    {0, 2, vc1DEC3DH_EscapeMode3}
};

static const vc1DEC_sVLCCode vc1DEC3DH_Block_Escape_Mode_3_PQUANT_1_To_7[12] =
{
    {0, 11, 5}, {1, 3, 1}, {2, 3, 2}, {3, 3, 3}, {4, 3, 4}, {5, 3, 5},
    {6, 3, 6}, {7, 3, 7}, {0, 5, 8}, {1, 5, 9}, {2, 5, 10}, {3, 5, 11}
};

static const vc1DEC_sVLCCode vc1DEC3DH_Block_Escape_Mode_3_PQUANT_8_To_31[8] =
{
    {0, 7, 6}, {1, 1, 2}, {1, 2, 3}, {1, 3, 4}, {1, 4, 5}, {1, 5, 6}, {1, 6, 7}, {0, 6, 8}
};



/* AC coding set tables */

typedef struct
{
    const vc1DEC_sVLCCode           *pVLCTable;
    const vc1DEC3DH_sRunLevel       *pRLLTable;
    const BYTE8                     *pDeltaLevel;
    const BYTE8                     *pDeltaRun;
    const BYTE8                     *pDeltaLevelLast;
    const BYTE8                     *pDeltaRunLast;  
} vc1DEC3DH_sACCodingSet;

static const vc1DEC3DH_sACCodingSet vc1DEC3DH_HighMotIntra =
{
    vc1DEC_High_Mot_Intra_VLC,           vc1DEC_High_Mot_Intra_Run_Level,
    vc1_High_Mot_Intra_Delta_Level_0,    vc1_High_Mot_Intra_Delta_Run_0,
    vc1_High_Mot_Intra_Delta_Level_1,    vc1_High_Mot_Intra_Delta_Run_1
};

static const vc1DEC3DH_sACCodingSet vc1DEC3DH_HighMotInter =
{
    vc1DEC_High_Mot_Inter_VLC,           vc1DEC_High_Mot_Inter_Run_Level,
    vc1_High_Mot_Inter_Delta_Level_0,    vc1_High_Mot_Inter_Delta_Run_0,
    vc1_High_Mot_Inter_Delta_Level_1,    vc1_High_Mot_Inter_Delta_Run_1
};

static const vc1DEC3DH_sACCodingSet vc1DEC3DH_LowMotIntra =
{
    vc1DEC_Low_Mot_Intra_VLC,            vc1DEC_Low_Mot_Intra_Run_Level,
    vc1_Low_Mot_Intra_Delta_Level_0,     vc1_Low_Mot_Intra_Delta_Run_0,
    vc1_Low_Mot_Intra_Delta_Level_1,     vc1_Low_Mot_Intra_Delta_Run_1
};

static const vc1DEC3DH_sACCodingSet vc1DEC3DH_LowMotInter =
{
    vc1DEC_Low_Mot_Inter_VLC,            vc1DEC_Low_Mot_Inter_Run_Level,
    vc1_Low_Mot_Inter_Delta_Level_0,     vc1_Low_Mot_Inter_Delta_Run_0,
    vc1_Low_Mot_Inter_Delta_Level_1,     vc1_Low_Mot_Inter_Delta_Run_1
};

static const vc1DEC3DH_sACCodingSet vc1DEC3DH_MidRateIntra =
{
    vc1DEC_Mid_Rate_Intra_VLC,           vc1DEC_Mid_Rate_Intra_Run_Level,
    vc1_Mid_Rate_Intra_Delta_Level_0,    vc1_Mid_Rate_Intra_Delta_Run_0,
    vc1_Mid_Rate_Intra_Delta_Level_1,    vc1_Mid_Rate_Intra_Delta_Run_1
};

static const vc1DEC3DH_sACCodingSet vc1DEC3DH_MidRateInter =
{
    vc1DEC_Mid_Rate_Inter_VLC,           vc1DEC_Mid_Rate_Inter_Run_Level,
    vc1_Mid_Rate_Inter_Delta_Level_0,    vc1_Mid_Rate_Inter_Delta_Run_0,
    vc1_Mid_Rate_Inter_Delta_Level_1,    vc1_Mid_Rate_Inter_Delta_Run_1
};

static const vc1DEC3DH_sACCodingSet vc1DEC3DH_HighRateIntra =
{
    vc1DEC_High_Rate_Intra_VLC,          vc1DEC_High_Rate_Intra_Run_Level,
    vc1_High_Rate_Intra_Delta_Level_0,   vc1_High_Rate_Intra_Delta_Run_0,
    vc1_High_Rate_Intra_Delta_Level_1,   vc1_High_Rate_Intra_Delta_Run_1
};

static const vc1DEC3DH_sACCodingSet vc1DEC3DH_HighRateInter =
{
    vc1DEC_High_Rate_Inter_VLC,          vc1DEC_High_Rate_Inter_Run_Level,
    vc1_High_Rate_Inter_Delta_Level_0,   vc1_High_Rate_Inter_Delta_Run_0,
    vc1_High_Rate_Inter_Delta_Level_1,   vc1_High_Rate_Inter_Delta_Run_1
};

static const vc1DEC3DH_sACCodingSet * const vc1DEC3DH_pACCodingSets[4][3] =
{
    /* Inter */
    { &vc1DEC3DH_HighRateInter,  &vc1DEC3DH_HighMotInter, &vc1DEC3DH_MidRateInter },
    { &vc1DEC3DH_LowMotInter,    &vc1DEC3DH_HighMotInter, &vc1DEC3DH_MidRateInter },
    /* Intra */
    { &vc1DEC3DH_HighRateIntra,  &vc1DEC3DH_HighMotIntra, &vc1DEC3DH_MidRateIntra },
    { &vc1DEC3DH_LowMotIntra,    &vc1DEC3DH_HighMotIntra, &vc1DEC3DH_MidRateIntra }
};


/*
 * Description:
 * Choose the AC run level decode escape mode 3 table.
 *
 * Remarks:
 * None.
 *
 * Inputs:
 * pState  - the decoder state
 *
 * Outputs:
 * None.
 *
 * Return Value:
 * Pointer to the VLCCode table.
 *
 */

static const vc1DEC_sVLCCode * vc1DEC3DH_ChooseEscapeMode3Table(vc1DEC_sState *pState)
{
    
    if (pState->sPicParams.eQuantMode!=vc1_QuantModeDefault || pState->sPicParams.PQuant<=7)
    {
        return(vc1DEC3DH_Block_Escape_Mode_3_PQUANT_1_To_7);
    }
    else
    {
        return(vc1DEC3DH_Block_Escape_Mode_3_PQUANT_8_To_31);
    }
}    

/*
 * Description:
 * Choose the AC coding set for run level decoding.
 *
 * Remarks:
 * None.
 *
 * Inputs:
 * pPicParams   - pointer to a structure of parameters for the current picture
 * pPosition    - pointer to a macroblock position structure
 * eBlk         - block number in the macroblock.
 * eBlkType     - block type of the eBlk
 *
 * Outputs:
 * None.
 *
 * Return Value:
 * Pointer to the coding set used for this block.
 *
 */

static const vc1DEC3DH_sACCodingSet * vc1DEC3DH_ChooseACCodingSet(
    vc1DEC_sPictureLayerParams * pPicParams,
    vc1_sPosition * pPosition,
    int eBlk,
    vc1_eBlkType eBlkType
)
{
    int codingset = 0, acindex;

    if((vc1_PictureTypeP == pPosition->ePictureType) ||
       (vc1_PictureTypeB == pPosition->ePictureType))
    {
        acindex = pPicParams->FrameTransformACCodingSetIndex;

        if(TRUE == vc1_BlkTypeIsIntra(eBlkType))
        {
            if(TRUE == vc1_BlkNumIsLuma(eBlk))
            {
                codingset = 2;
                DEBUG0(vc1DEBUG_CMP, "Luma\n");
            }
            else
            {
                DEBUG0(vc1DEBUG_CMP, "Chroma\n");
            }
        }            
    }
    else if(TRUE == vc1_PictureTypeIsIntra(pPosition->ePictureType))
    { 
        if(TRUE == vc1_BlkNumIsLuma(eBlk))
        {
            codingset = 2;
            acindex = pPicParams->FrameTransformACCodingSetIndex2;
            DEBUG0(vc1DEBUG_CMP, "Luma\n");
        }
        else
        {
            acindex = pPicParams->FrameTransformACCodingSetIndex;
            DEBUG0(vc1DEBUG_CMP, "Chroma\n");
        }
    }
    else
    {
        acindex = 0;
        FATAL("ChooseACCodingSet: Unknown picture type - %d\n", pPosition->ePictureType);
    }
    
    if(TRUE == vc1_BlkTypeIsIntra(eBlkType))
    {
        if(8 < pPicParams->PQIndex)
        {
            codingset += 1;
        }
        COVERAGE("8.3.6.1.5");
    }
    else
    {
        COVERAGE("8.3.6.2.3");
        if(8 < pPicParams->PQIndex)
        {
            codingset += 1;
        }
    }

    DEBUG2(vc1DEBUG_BLK, "codingset = %d, acindex = %d\n", codingset, acindex);
    return(vc1DEC3DH_pACCodingSets[codingset][acindex]);
}


/*
 * Description:
 * Unpack a single AC coefficient.
 *
 * Remarks:
 * None.
 *
 * Inputs:
 * pState               - pointer to the decoder's state
 * pBitstream           - pointer to the struct representing the current bitstream
 * pRun                 - pointer to a location to write the run value
 * pLevel               - pointer to a location to write the level value
 * pLast                - pointer to a location to write the last value
 * pACCodingSet         - pointer to the coding set used for this coefficient.
 * pEscapeMode3Table    - pointer to the escape mode 3 table
 *
 * Outputs:
 * pBitstream           - updated with new position
 * pRun                 - updated with run value
 * pLevel               - updated with level value
 * pLast                - updated with last value
 *
 * Return Value:
 * Standard vc1_eResult result. See enumeration for possible result codes.
 * 
 */

static vc1_eResult vc1DEC3DH_UnpackTransformACCoef( vc1DEC_sState * pState,
                                                    vc1DEC_sBitstream * pBitstream,
                                                    int * pRun,
                                                    int * pLevel,
                                                    int * pLast,
                                                    const vc1DEC3DH_sACCodingSet * pACCodingSet,
                                                    const vc1DEC_sVLCCode * pEscapeMode3Table)
{
    int                             run, last_flag = 0;
    int                             level;
    int                             sign;
    const vc1DEC_sVLCCode           *pHuffTable     = pACCodingSet->pVLCTable;
    const vc1DEC3DH_sRunLevel       *pRLLTable      = pACCodingSet->pRLLTable;
    WORD32                          index;

    COVERAGE_NONINT_INT("8.1.1.10","10.1.2.4");
    index = vc1DECBIT_GetVLC(pBitstream, pHuffTable);
    if(VC1DECBIT_EOF == index)
    {
        return(vc1_ResultBufferExhausted);
    }

    COVERAGE("7.1.4.8");
    
    if(ESCAPE != index)
    {
        run         = pRLLTable[index].Run;
        level       = pRLLTable[index].Level;
        last_flag   = pRLLTable[index].Last;

        sign = (BYTE8)vc1DECBIT_GetBits(pBitstream, 1);
        if(VC1DECBIT_EOF == sign)
        {
            return(vc1_ResultBufferExhausted);
        }
        COVERAGE("7.1.4.11");
        if(1 == sign)
        {
            level = -level;
        }

        *pRun = run;
        *pLevel = level;
        *pLast = last_flag;

        return(vc1_ResultOK);
    }
    else
    {
        const BYTE8                     *pDeltaLevel    = pACCodingSet->pDeltaLevel;
        const BYTE8                     *pDeltaRun      = pACCodingSet->pDeltaRun;
        const BYTE8                     *pDeltaLevelLast= pACCodingSet->pDeltaLevelLast;
        const BYTE8                     *pDeltaRunLast  = pACCodingSet->pDeltaRunLast;
        BYTE8 escape_mode;

        escape_mode = (BYTE8)vc1DECBIT_GetVLC(pBitstream, vc1DEC3DH_Escape_Decoding_Mode);
        if(VC1DECBIT_EOF == escape_mode)
        {
            return(vc1_ResultBufferExhausted);
        }

        DEBUG1(vc1DEBUG_BIT, "Escape = %d\n", escape_mode);
        COVERAGE("7.1.4.10");

        if(vc1DEC3DH_EscapeMode1 == escape_mode)
        {
            index = vc1DECBIT_GetVLC(pBitstream, pHuffTable);
            if(VC1DECBIT_EOF == index)
            {
                return(vc1_ResultBufferExhausted);
            }

            COVERAGE("7.1.4.9");
            run         = pRLLTable[index].Run;
            level       = pRLLTable[index].Level;
            last_flag   = pRLLTable[index].Last;

            DEBUG3(vc1DEBUG_BIT, "index = %d, run = %d, level = %d\n", index, run, level);

            if(0 == last_flag)
            {
                level = level + pDeltaLevel[run];
            }
            else
            {
                level = level + pDeltaLevelLast[run];
            }

            sign = (BYTE8)vc1DECBIT_GetBits(pBitstream, 1);
            if(VC1DECBIT_EOF == sign)
            {
                return(vc1_ResultBufferExhausted);
            }
            COVERAGE("7.1.4.11");
            if(1 == sign)
            {
                level = -level;
            }
        }
        else if(vc1DEC3DH_EscapeMode2 == escape_mode)
        {
            index = vc1DECBIT_GetVLC(pBitstream, pHuffTable);
            if(VC1DECBIT_EOF == index)
            {
                return(vc1_ResultBufferExhausted);
            }

            COVERAGE("7.1.4.9");
            run         = pRLLTable[index].Run;
            level       = pRLLTable[index].Level;
            last_flag   = pRLLTable[index].Last;

            if(0 == last_flag)
            {
                run = run + pDeltaRun[level] + 1;
            }
            else
            {
                run = run + pDeltaRunLast[level] + 1;
            }

            sign = (BYTE8)vc1DECBIT_GetBits(pBitstream, 1);
            if(VC1DECBIT_EOF == sign)
            {
                return(vc1_ResultBufferExhausted);
            }
            COVERAGE("7.1.4.11");
            if(1 == sign)
            {
                level = -level;
            }
        }
        else if(vc1DEC3DH_EscapeMode3 == escape_mode)
        {
            last_flag = (BYTE8)vc1DECBIT_GetBits(pBitstream, 1);
            if(VC1DECBIT_EOF == last_flag)
            {
                return(vc1_ResultBufferExhausted);
            }
            COVERAGE("7.1.4.12");

            /* fixed length encoding */
            if(0 == pState->NotFirstMode3InFrame)
            {
                UWORD32 Value;
                pState->NotFirstMode3InFrame = 1;

                Value = vc1DECBIT_GetVLC(pBitstream, pEscapeMode3Table);
                if(VC1DECBIT_EOF == Value)
                {
                    return(vc1_ResultBufferExhausted);
                }
                pState->LevelCodeSize = (UBYTE8)Value;
                COVERAGE("7.1.4.16");

                Value= vc1DECBIT_GetBits(pBitstream, 2);
                if(VC1DECBIT_EOF == Value)
                {
                    return(vc1_ResultBufferExhausted);
                }
                pState->RunCodeSize = (BYTE8)(3 + Value);

                COVERAGE("7.1.4.17");
                DEBUG2(vc1DEBUG_BIT,
                      "LevelCodeSize = %d, RunCodeSize = %d\n",
                      pState->LevelCodeSize,
                      pState->RunCodeSize);

            }

            run = vc1DECBIT_GetBits(pBitstream, pState->RunCodeSize);
            if(VC1DECBIT_EOF == run)
            {
                return(vc1_ResultBufferExhausted);
            }
            COVERAGE("7.1.4.13");

            sign = (BYTE8)vc1DECBIT_GetBits(pBitstream, 1);
            if(VC1DECBIT_EOF == sign)
            {
                return(vc1_ResultBufferExhausted);
            }
            COVERAGE("7.1.4.15");

            level = vc1DECBIT_GetBits(pBitstream, pState->LevelCodeSize);
            if(VC1DECBIT_EOF == level)
            {
                return(vc1_ResultBufferExhausted);
            }
            COVERAGE("7.1.4.14");

            if(1 == sign)
            {
                level = -level;
            }
        }
        else
        {
            FATAL("vc1DEC3DH_UnpackTransformACCoefs: Unknown escape mode %d\n", escape_mode);
            return(vc1_ResultInvalidParameter);
        }
    }

    *pRun = run;
    *pLevel = level;
    *pLast = last_flag;

    return(vc1_ResultOK);
}


/*
 * Description:
 * Decode AC run/level information
 *
 * Remarks:
 * Calls UnpackTransformACCoef() for each coefficient, and stores the result in the pACCoefs array.
 * Based on the pseudo-code in figure 36 of the standard.
 * Possible improvements include combining this function with the de-zigzag operation.
 * Uses the NZC values in each block to determine which subblocks are coded.
 *
 * Inputs:
 * pState     - pointer to the decoder state structure
 * pBitstream - pointer to the bitstream structure
 * pCoefs     - pointer to a 64 element array, into which coefficients will be written
 * pPosition  - pointer to position structure indicating current MB
 * eBlk       - the block number to which these coefficients relate
 *
 * Outputs:
 * pState     - updated with data read from the bitstream
 * pBitstream - updated with new bitstream position
 * pCoefs     - updated with coefficients unpacked from the bitstream
 *
 * Return Value:
 * Standard vc1_eResult result. See enumeration for possible result codes.
 * 
 */

vc1_eResult vc1DEC3DH_DecodeACRunLevel(vc1DEC_sState * pState,
                                       vc1DEC_sBitstream * pBitstream,
                                       HWD16 *pCoefs,
                                       vc1_sPosition * pPosition,
                                       int eBlk)
{
    int                             Run, Level, LastFlag = 0;
    vc1_eResult                     eResult;
    const vc1DEC3DH_sACCodingSet    *pACCodingSet;
    const vc1DEC_sVLCCode           *pEscapeMode3Table;
    int                             CoefCount = 0, CoefLoops = 0;
    vc1_sMB                         *pMB = pPosition->pCurMB;
    vc1_sBlk                        *pBlk = &pMB->sBlk[eBlk];
    int                             Loop;

    pACCodingSet        = vc1DEC3DH_ChooseACCodingSet(&pState->sPicParams,
                                                      pPosition,
                                                      eBlk,
                                                      pPosition->pCurMB->sBlk[eBlk].eBlkType);
    pEscapeMode3Table   = vc1DEC3DH_ChooseEscapeMode3Table(pState);

    if(TRUE == vc1_BlkTypeIsIntra(pPosition->pCurMB->sBlk[eBlk].eBlkType))
    {
        COVERAGE("8.1.1.11");
    }
    else
    {
        COVERAGE("8.3.6.2.4");
    }

    switch(pMB->sBlk[eBlk].eBlkType)
    {
        case vc1_BlkIntra:          /* fall through */
        case vc1_BlkIntraLeft:      /* fall through */
        case vc1_BlkIntraTop:
            /* skip the DC coefficient if this is an intra block */
            pCoefs++;
            CoefCount = 63;
            CoefLoops = 1;
            break;
        
        case vc1_BlkInter8x8:
            CoefCount = 64;
            CoefLoops = 1;
            break;
        
        case vc1_BlkInter4x8:       /* fall through */
        case vc1_BlkInter8x4:
            CoefCount = 32;
            CoefLoops = 2;
            break;

        case vc1_BlkInter4x4:
            CoefCount = 16;
            CoefLoops = 4;
            break;

        default:
            FATAL("DecodeACRunLevel: Unexpected block type %d\n",eBlk);
            return(vc1_ResultFatal);
    }
    
    for(Loop = 0; Loop < CoefLoops; Loop++)
    {
        int CurrPosition = 0;

        /* unpack AC coefs if this subblock is coded */
        if (vc1_BlkTypeIsIntra(pBlk->eBlkType) || pBlk->u.sInter.NZC[Loop])
        {
            do
            {
                eResult = vc1DEC3DH_UnpackTransformACCoef(pState,
                                                          pBitstream,
                                                          &Run,
                                                          &Level,
                                                          &LastFlag,
                                                          pACCodingSet,
                                                          pEscapeMode3Table);
                if(vc1_ResultOK != eResult)
                {
                    WARN("DecodeACRunLevel: Error in unpack transform coefs - %d\n", eResult);
                    return(vc1_ResultACRunLevelDecodeFailed);
                }

                DEBUG4(vc1DEBUG_CMP,
                      "AC Run=%2d Level=%c%3d Last=%d\n",
                      Run, 
                      (Level < 0) ? '-' : '+',
                      (Level < 0) ? -Level : Level,
                      LastFlag);

                if((CurrPosition + Run) < CoefCount)
                {
                    pCoefs[CurrPosition + Run] = (HWD16)Level;
                }
                else
                {
                    FATAL("DecodeACRunLevel: AC coef array overrun\n");
                    return(vc1_ResultACRunLevelDecodeFailed);
                }
                CurrPosition = CurrPosition + Run + 1;

            } while(1 != LastFlag);
        }
        else
        {
            DEBUG2(vc1DEBUG_BLK, "No codes in block %d Loop %d\n", eBlk, Loop);
        }

        pCoefs += CoefCount;
    }

    return(vc1_ResultOK);
}
