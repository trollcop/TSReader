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
 * vc1decmv.c:
 * Motion vector prediction functions specific to the decoder
 *
 */


#include "vc1types.h"
#include "vc1debug.h"
#include "vc1cropmv.h"
#include "vc1gentab.h"

#include "vc1dec.h"
#include "vc1decmv.h"
#include "vc1decbit.h"

/*
 * Description:
 * Apply predicted motion vectors to the decoded differential motion vectors
 *
 * Remarks:
 * None.
 *
 * Inputs:
 * pState       - pointer to the decoder's state
 * eBlk         - block number of current block
 * pPred        - pointer to the predicted motion vectors
 * Backwards    - 0=forward mv, 1=backward mv
 *
 * Outputs:
 * pState       - updated with new motion vectors
 *
 * Return Value:
 * None
 * 
 */

void vc1DECMV_ApplyMVPrediction(    vc1DEC_sState * pState, 
                                    int eBlk, 
                                    vc1_sMV * pPred, 
                                    int Backwards)
{
    const vc1GENTAB_sMVRangeParams *pMVRange;
    int RangeX, RangeY, YBias=0;
    int MV_X, MV_Y, DMV_X, DMV_Y;
    int PredictorX, PredictorY;
    int Count;
    vc1_sPosition *pPosition = &pState->sPosition;
    vc1_sMB *pMB = pPosition->pCurMB;
    vc1_sMV sMV;

    switch(pPosition->eMVRange)
    {
        case vc1_MVRange64_32:
            pMVRange = &vc1GENTAB_Motion_Vector_Range_Params_Table[0]; 
            break;  

        case vc1_MVRange128_64:     
            pMVRange = &vc1GENTAB_Motion_Vector_Range_Params_Table[1]; 
            break;

        case vc1_MVRange512_128:    
            pMVRange = &vc1GENTAB_Motion_Vector_Range_Params_Table[2]; 
            break;

        case vc1_MVRange1024_256:   
            pMVRange = &vc1GENTAB_Motion_Vector_Range_Params_Table[3]; 
            break;

        default:                    
            FATAL("ApplyMVPrediction: Unknown MVRange - %d\n",
                pPosition->eMVRange);
            return;
    }

    RangeX = pMVRange->RangeX;
    RangeY = pMVRange->RangeY;

    if (pPosition->ePictureFormat != vc1_ProgressiveFrame &&
        vc1_MVModeIsHalfPel(pPosition->eMVMode))
    {
        RangeX <<= 1;
        RangeY <<= 1;
    }
    if (    pPosition->ePictureFormat == vc1_InterlacedField    && 
            pPosition->NumRef==1                                )
    {
        RangeY >>= 1;
    }
    if (pPosition->ePictureFormat == vc1_InterlacedField &&
        pPosition->BottomField &&
        pPred->BottomField==0)
    {
        /* Current bottom, ref is top */
        YBias = 1;
    }

    DMV_X = pMB->sBlk[eBlk].u.sInter.sMotion[Backwards].sDMV.X;
    DMV_Y = pMB->sBlk[eBlk].u.sInter.sMotion[Backwards].sDMV.Y;

    PredictorX = pPred->X;
    PredictorY = pPred->Y;

    DEBUG2(vc1DEBUG_MV, "ApplyPred : RangeX = %d, RangeY = %d\n", RangeX, RangeY);
    DEBUG3(vc1DEBUG_MV, "ApplyPred : PredX = %d, PredY = %d, PredF = %d\n", 
        PredictorX, PredictorY, pPred->BottomField);
    DEBUG3(vc1DEBUG_MV, "ApplyPred : DMV_X = %d, DMV_Y = %d, DMV_F = %d\n",
        DMV_X, DMV_Y, pMB->sBlk[eBlk].u.sInter.sMotion[Backwards].sDMV.BottomField);

    DMV_X += PredictorX;
    DMV_Y += PredictorY;

    /* (dmv_x + predictor_x) smod range_x */
    sMV.X = (HWD16)( ((DMV_X + RangeX) & (2 * RangeX - 1)) - RangeX );

    /* (dmv_y + predictor_y) smod range_y */
    sMV.Y = (HWD16)( ((DMV_Y + RangeY - YBias) & (2 * RangeY - 1)) - RangeY + YBias );
    
    COVERAGE_NONINT_INT("8.3.5.4.1","10.3.4.5.4.1");

    MV_X = sMV.X;
    MV_Y = sMV.Y;

    if(TRUE == vc1_MBTypeIs1MV(pMB->eMBType))
    { 
        for(Count = 0; Count < 4; Count++)
        {
            pMB->sBlk[Count].u.sInter.sMotion[Backwards].sMV.X = (HWD16)MV_X;
            pMB->sBlk[Count].u.sInter.sMotion[Backwards].sMV.Y = (HWD16)MV_Y;
            if(vc1_ProgressiveFrame == pPosition->ePictureFormat)
            {
                pMB->sBlk[Count].u.sInter.sMotion[Backwards].sMV.BottomField = 0;
            }
            else
            {
                pMB->sBlk[Count].u.sInter.sMotion[Backwards].sMV.BottomField = pPred->BottomField;
            }
        }
    }
    else if(TRUE == vc1_MBTypeIs2MV(pMB->eMBType))
    {
        for(Count = 0; Count < 2; Count++)
        {
            pMB->sBlk[Count + eBlk].u.sInter.sMotion[Backwards].sMV.X = (HWD16)MV_X;
            pMB->sBlk[Count + eBlk].u.sInter.sMotion[Backwards].sMV.Y = (HWD16)MV_Y;
            pMB->sBlk[Count + eBlk].u.sInter.sMotion[Backwards].sMV.BottomField =
              pPred->BottomField;
        }
    }
    else    /* 4MV */
    {
        pMB->sBlk[eBlk].u.sInter.sMotion[Backwards].sMV.X = (HWD16)MV_X;
        pMB->sBlk[eBlk].u.sInter.sMotion[Backwards].sMV.Y = (HWD16)MV_Y;
        if(vc1_ProgressiveFrame == pPosition->ePictureFormat)
        {
            pMB->sBlk[eBlk].u.sInter.sMotion[Backwards].sMV.BottomField = 0;
        }
        else
        {
            pMB->sBlk[eBlk].u.sInter.sMotion[Backwards].sMV.BottomField = pPred->BottomField;
        }
    }

    DEBUG3(vc1DEBUG_MV, "ApplyPred : MV_X  = %d, MV_Y  = %d, MV_F  = %d\n",
        MV_X, MV_Y, pMB->sBlk[eBlk].u.sInter.sMotion[Backwards].sMV.BottomField);
}





/*
 * Description:
 * Unpack differential motion vectors for progressive bitstreams.
 *
 * Remarks:
 * None.
 *
 * Inputs:
 * pMB          - pointer to current macroblock
 * pState       - pointer to the decoder's state
 * pBitstream   - pointer to struct representing the current bitstream
 * eBlk         - block number of current block
 * Backwards    - 0=forward mv, 1=backward mv
 *
 * Outputs:
 * pMB          - updated with new differential motion vectors
 * pBitstream   - updated with new position
 *
 * Return Value:
 * None
 * 
 */

vc1_eResult vc1DECMV_UnpackMVData(  vc1_sMB * pMB, 
                                    vc1DEC_sState * pState, 
                                    vc1DEC_sBitstream * pBitstream, 
                                    int eBlk, int Backwards)
{
    const UBYTE8 SizeTable[6]      = {0, 2, 3, 4, 5, 8};
    const UBYTE8 OffsetTable[6]    = {0, 1, 3, 7, 15, 31};
    int Index, Index1, DMV_X, DMV_Y, KX, KY;
    int LastFlag, HalfPelFlag = 0, IntraFlag;
    const vc1GENTAB_sMVRangeParams *pMVRange;
    int Count;
    
    if(TRUE == vc1_MVModeIsHalfPel(pState->sPosition.eMVMode))
    {
        HalfPelFlag = 1;
    }   

    switch(pState->sPosition.eMVRange)
    {
        case vc1_MVRange64_32:
            pMVRange = &vc1GENTAB_Motion_Vector_Range_Params_Table[0];
            break;

        case vc1_MVRange128_64:
            pMVRange = &vc1GENTAB_Motion_Vector_Range_Params_Table[1];
            break;
        case vc1_MVRange512_128:
            pMVRange = &vc1GENTAB_Motion_Vector_Range_Params_Table[2];
            break;

        case vc1_MVRange1024_256:
            pMVRange = &vc1GENTAB_Motion_Vector_Range_Params_Table[3];
            break;

        default:
            FATAL("UnpackMVData: Unknown MVRange - %d\n", pState->sPosition.eMVRange);
            return(vc1_ResultFatal);
    }

    KX = pMVRange->KX;
    KY = pMVRange->KY;

    Index = vc1DECBIT_GetVLC(pBitstream, pState->sPicParams.pMotionVectorTable);
    if(VC1DECBIT_EOF == Index)
    {
        return(vc1_ResultBufferExhausted);
    }

    Index++;

    DEBUG1(vc1DEBUG_MB, "index = %d\n", Index);

    if(Index >= 37)
    {
        LastFlag = 1;
        Index -= 37;
    }
    else
    {
        LastFlag = 0;
    }

    IntraFlag = 0;
    if(0 == Index)
    {
        DMV_X = 0;
        DMV_Y = 0;
    }
    else if(35 == Index)
    {
        DMV_X = vc1DECBIT_GetBits(pBitstream, (UBYTE8)(KX - HalfPelFlag));
        if(VC1DECBIT_EOF == DMV_X)
        {
            return(vc1_ResultBufferExhausted);
        }

        DMV_Y = vc1DECBIT_GetBits(pBitstream, (UBYTE8)(KY - HalfPelFlag));
        if(VC1DECBIT_EOF == DMV_Y)
        {
            return(vc1_ResultBufferExhausted);
        }
    }
    else if(36 == Index)
    {
        IntraFlag = 1;
        DMV_X = 0;
        DMV_Y = 0;
    }
    else
    {
        int Val, Hpel, Sign;

        Index1 = Index % 6;
        if((1 == HalfPelFlag) && (5 == Index1))
        {
            Hpel = 1;
        }
        else
        {
            Hpel = 0;
        }

        if(0 == Index1)
        {
            DMV_X = 0;
        }
        else
        {        
            Val = vc1DECBIT_GetBits(pBitstream, (UBYTE8)(SizeTable[Index1] - Hpel));
            if(VC1DECBIT_EOF == Val)
            {
                return(vc1_ResultBufferExhausted);
            }

            Sign = 0 - (Val & 1);
            DMV_X = Sign ^ ((Val >> 1) + OffsetTable[Index1]);
            DMV_X = DMV_X - Sign;
        }

        Index1 = Index / 6;
        if((1 == HalfPelFlag) && (5 == Index1))
        {
            Hpel = 1;
        }
        else
        {
            Hpel = 0;
        }

        if(0 == Index1)
        {
            DMV_Y = 0;
        }
        else
        {
            Val = vc1DECBIT_GetBits(pBitstream, (UBYTE8)(SizeTable[Index1] - Hpel));
            if(VC1DECBIT_EOF == Val)
            {
                return(vc1_ResultBufferExhausted);
            }

            Sign = 0 - (Val & 1);
            DMV_Y = Sign ^ ((Val >> 1) + OffsetTable[Index1]);
            DMV_Y = DMV_Y - Sign;
        }
    }
    COVERAGE("8.3.5.2.1");

    if(TRUE == vc1_MBTypeIs1MV(pMB->eMBType))
    {
        if(1 == IntraFlag)
        {
            DEBUG1(vc1DEBUG_MV | vc1DEBUG_CMP, "DMV Intra Coded=%d\n", LastFlag);
        }
        else
        {
            DEBUG3(vc1DEBUG_MV | vc1DEBUG_CMP, "DMV Inter X=%5d, Y=%5d, Coded=%d\n",
                DMV_X * (1 + HalfPelFlag), DMV_Y * (1 + HalfPelFlag), LastFlag);
        }

        pMB->sBlk[0].Coded = (FLAG)LastFlag;

        for(Count = 0; Count < 4; Count++)
        {
            /* preserve chroma bits, clear others */
            pMB->CBPCY &= 0x3;

            if(1 == HalfPelFlag)
            {
                /* store dmv_x,y in qpels */
                pMB->sBlk[Count].u.sInter.sMotion[Backwards].sDMV.X = (HWD16)(DMV_X * 2);
                pMB->sBlk[Count].u.sInter.sMotion[Backwards].sDMV.Y = (HWD16)(DMV_Y * 2);
            }
            else
            {
                pMB->sBlk[Count].u.sInter.sMotion[Backwards].sDMV.X = (HWD16)DMV_X;
                pMB->sBlk[Count].u.sInter.sMotion[Backwards].sDMV.Y = (HWD16)DMV_Y;
            }
            if(1 == IntraFlag)
            {
                pMB->sBlk[Count].eBlkType = vc1_BlkIntra;
            }
            else
            {
                pMB->sBlk[Count].eBlkType = vc1_BlkInter8x8;
            }
        }
        COVERAGE("8.3.5.5.1");
    }
    else
    {
        if(1 == HalfPelFlag)
        {
            /* store dmv_x, y in qpels */
            pMB->sBlk[eBlk].u.sInter.sMotion[Backwards].sDMV.X = (HWD16)(DMV_X * 2);
            pMB->sBlk[eBlk].u.sInter.sMotion[Backwards].sDMV.Y = (HWD16)(DMV_Y * 2);
        }
        else
        {
            pMB->sBlk[eBlk].u.sInter.sMotion[Backwards].sDMV.X = (HWD16)DMV_X;
            pMB->sBlk[eBlk].u.sInter.sMotion[Backwards].sDMV.Y = (HWD16)DMV_Y;
        }
        if(1 == IntraFlag)
        {
            DEBUG1(vc1DEBUG_MV | vc1DEBUG_CMP, "DMV Intra Coded=%d\n", LastFlag);
            pMB->sBlk[eBlk].eBlkType = vc1_BlkIntra;
        }
        else
        {
            DEBUG3(vc1DEBUG_MV | vc1DEBUG_CMP, "DMV Inter X=%5d, Y=%5d, Coded=%d\n",
                DMV_X * (1 + HalfPelFlag), DMV_Y * (1 + HalfPelFlag), LastFlag);
            pMB->sBlk[eBlk].eBlkType = vc1_BlkInter8x8;
        }

        pMB->sBlk[eBlk].Coded = (FLAG)LastFlag;
        if(FALSE == LastFlag)
        {
            /* Block not coded after all */
            pMB->CBPCY = (UBYTE8)(pMB->CBPCY & ~(1 << (5 - eBlk)));
        }

        COVERAGE("8.3.5.5.2");
    }

    return(vc1_ResultOK);
}






/*
 * Description:
 * Unpack differential motion vectors for interlaced bitstreams.
 *
 * Remarks:
 * None.
 *
 * Inputs:
 * pMB          - pointer to current macroblock
 * pPredFlag    - pointer to location into which can be written the prediction flag
 * pState       - pointer to the decoder's state
 * pBitstream   - pointer to struct representing the current bitstream
 * eBlk         - block number of current block
 * TwoField     - if true, there are two reference field, else one
 * Backwards    - 0=forward mv, 1=backward mv
 *
 * Outputs:
 * pMB          - updated with new differential motion vectors
 * pPredFlag    - updated with predicted flag status
 * pBitstream   - updated with new position
 *
 * Return Value:
 * None
 * 
 */

vc1_eResult vc1DECMV_UnpackMVDataInterlace( vc1_sMB * pMB,
                                            FLAG * pPredFlag,
                                            vc1DEC_sState * pState,
                                            vc1DEC_sBitstream * pBitstream,
                                            int eBlk,
                                            FLAG TwoField,
                                            int Backwards)
{
    const vc1GENTAB_sMVRangeParams *pMVRange;
    const UBYTE8 OffsetTable1[9] = {0, 1, 2, 4, 8, 16, 32, 64, 128};
    const UBYTE8 OffsetTable2[9] = {0, 1, 3, 7, 15, 31, 63, 127, 255};
    const UBYTE8 *OffsetTable = OffsetTable1;
    WORD32 Value;
    int KX, KY, Index, Sign, DMV_X, DMV_Y;
    int PredictorFlag = 0, IndexEscape = 71, Scale = 1;

    int ExtendX = pState->sPicParams.ExtendHorizDMVRange;
    int ExtendY = pState->sPicParams.ExtendVertDMVRange;


    if (vc1_MVModeIsHalfPel(pState->sPosition.eMVMode))
    {
        Scale = 2;
    } 

    switch(pState->sPosition.eMVRange)
    {
        case vc1_MVRange64_32:
            pMVRange = &vc1GENTAB_Motion_Vector_Range_Params_Table[0];
            break;

        case vc1_MVRange128_64:
            pMVRange = &vc1GENTAB_Motion_Vector_Range_Params_Table[1];
            break;

        case vc1_MVRange512_128:
            pMVRange = &vc1GENTAB_Motion_Vector_Range_Params_Table[2];
            break;

        case vc1_MVRange1024_256:
            pMVRange = &vc1GENTAB_Motion_Vector_Range_Params_Table[3];
            break;

        default:
            FATAL("UnpackMVData: Unknown MVRange - %d\n", pState->sPosition.eMVRange);
            return(vc1_ResultFatal);
    }

    KX = pMVRange->KX;
    KY = pMVRange->KY;

    if(TRUE == TwoField)
    {
        IndexEscape = 125;
    }

    Index = vc1DECBIT_GetVLC(pBitstream, pState->sPicParams.pMotionVectorTable);
    if(VC1DECBIT_EOF == Index)
    {
        return(vc1_ResultBufferExhausted);
    }
    DEBUG1(vc1DEBUG_MB, "UnpackMVDataInterlace: Index = %d\n", Index);

    if(IndexEscape == Index)
    {
        DMV_X = vc1DECBIT_GetBits(pBitstream, (UBYTE8)KX);
        if(VC1DECBIT_EOF == DMV_X)
        {
            return(vc1_ResultBufferExhausted);
        }
        DMV_Y = vc1DECBIT_GetBits(pBitstream, (UBYTE8)KY);
        if(VC1DECBIT_EOF == DMV_Y)
        {
            return(vc1_ResultBufferExhausted);
        }
        if(TRUE == TwoField)
        {
            PredictorFlag = DMV_Y & 1;
            DMV_Y = (DMV_Y+1) >> 1;
        }
    }
    else
    {
        int Index1;

        /* Decode DMV_X */
        if(1 == ExtendX)
        {
            OffsetTable = OffsetTable2;
        }

        Index1 = (Index + 1) % 9;
        if(0 == Index1)
        {
            Value = 0;
        }
        else
        {
            Value = vc1DECBIT_GetBits(pBitstream, (UBYTE8)(Index1 + ExtendX));
            if(VC1DECBIT_EOF == Value)
            {
                return(vc1_ResultBufferExhausted);
            }
        }

        Sign = 0 - (Value & 1);

        DMV_X = Sign ^ ((Value >> 1) + OffsetTable[Index1]);
        DMV_X = DMV_X - Sign;

        /* Decode DMV_Y */
        OffsetTable = OffsetTable1;
        if(1 == ExtendY)
        {
            OffsetTable = OffsetTable2;
        }

        if(TRUE == TwoField)
        {
            int Size;

            /* Two Field */
            Index1 = (Index + 1) / 9;

            PredictorFlag = Index1 & 1;
            Size = Index1 >> 1;

            if (Size == 0)
            {
                Value = 0;
            }
            else
            {
                Value = vc1DECBIT_GetBits(pBitstream, Size + ExtendY);
                if(VC1DECBIT_EOF == Value)
                {
                    return(vc1_ResultBufferExhausted);
                }
            }

            Sign = 0 - (Value & 1);

            DMV_Y = Sign ^ ((Value >> 1) + OffsetTable[Size]);
            DMV_Y = DMV_Y - Sign;

            COVERAGE("10.3.4.5.2.2");
        }
        else /* One Field */
        {
            Index1 = (Index + 1) / 9;
            if(0 == Index1)
            {
                Value = 0;
            }
            else
            {
                Value = vc1DECBIT_GetBits(pBitstream, (UBYTE8)(Index1 + ExtendY));
                if(VC1DECBIT_EOF == Value)
                {
                    return(vc1_ResultBufferExhausted);
                }
            }

            Sign = 0 - (Value & 1);

            DMV_Y = Sign ^ ((Value >> 1) + OffsetTable[Index1]);
            DMV_Y = DMV_Y - Sign;
            COVERAGE("10.3.4.5.2.1");
        }
    }

    DMV_X *= Scale;
    DMV_Y *= Scale;

    /* Write the results into the macroblock */
    if(TRUE == vc1_MBTypeIs1MV(pMB->eMBType))
    {
        int Count;

        for(Count = 0; Count < 4; Count++)
        {
            pMB->sBlk[Count].u.sInter.sMotion[Backwards].sDMV.X = (HWD16)DMV_X;
            pMB->sBlk[Count].u.sInter.sMotion[Backwards].sDMV.Y = (HWD16)DMV_Y;
            pMB->sBlk[Count].eBlkType = pMB->eBlkType;
        }
    }
    else if(TRUE == vc1_MBTypeIs2MV(pMB->eMBType))
    {
        int Count;

        for(Count = 0; Count < 2; Count++)
        {
            pMB->sBlk[eBlk + Count].u.sInter.sMotion[Backwards].sDMV.X = (HWD16)DMV_X;
            pMB->sBlk[eBlk + Count].u.sInter.sMotion[Backwards].sDMV.Y = (HWD16)DMV_Y;
            pMB->sBlk[eBlk + Count].eBlkType = pMB->eBlkType;
        }
    }
    else if(TRUE == vc1_MBTypeIs4MV(pMB->eMBType))
    {
        pMB->sBlk[eBlk].u.sInter.sMotion[Backwards].sDMV.X = (HWD16)DMV_X;
        pMB->sBlk[eBlk].u.sInter.sMotion[Backwards].sDMV.Y = (HWD16)DMV_Y;
        pMB->sBlk[eBlk].eBlkType = pMB->eBlkType;
    }

    if (TwoField)
    {
        DEBUG1(vc1DEBUG_MV, "PredFlag = %d\n", PredictorFlag);
        *pPredFlag = (FLAG)PredictorFlag;
    }

    return(vc1_ResultOK);
}
