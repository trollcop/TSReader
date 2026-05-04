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
 * vc1decent.c:
 * Bitstream entry point layer unpack functions
 *
 */

#include "vc1types.h"
#include "vc1dec.h"
#include "vc1decbit.h"
#include "vc1decent.h"
#include "vc1debug.h"
#include "vc1hrd.h"


vc1_eResult vc1DECENT_UnpackEntryPointLayer(vc1DEC_sState *pState, vc1DEC_sBitstream * pBitstream)
{
    vc1_sSequenceLayer * pParams = &pState->sSeqParams;
    vc1_sPosition *pPos = &pState->sPosition;
    WORD32 Value;

    DEBUG1(vc1DEBUG_ENT, "Frame Number: %d\n", pState->FrameNum);

    /* broken link flag */
    Value = vc1DECBIT_GetBits(pBitstream, VC1_BITS_BROKEN_LINK);
    if(VC1DECBIT_EOF == Value)
    {
        return(vc1_ResultBufferExhausted);
    }
    pParams->BrokenLink = (FLAG)Value;
    DEBUG1(vc1DEBUG_ENT, "BROKEN_LINK: %d\n", Value);
    COVERAGE("6.2.1");
    if (pParams->BrokenLink)
    {
        /* Mark current reference frames as preceding a broken link */
        pPos->pReferenceNew->BrokenLink = TRUE;
        pPos->pReferenceOld->BrokenLink = TRUE;
    }


    /* closed entry point */
    Value = vc1DECBIT_GetBits(pBitstream, VC1_BITS_CLOSED_ENTRY);
    if(VC1DECBIT_EOF == Value)
    {
        return(vc1_ResultBufferExhausted);
    }
    pParams->ClosedEntry = (FLAG)Value;
    DEBUG1(vc1DEBUG_ENT, "CLOSED_ENTRY: %d\n", Value);
    COVERAGE("6.2.2");
    if (pParams->ClosedEntry)
    {
        /* If ClosedEntry then BrokenLink must not be set */
        ASSERT(pParams->BrokenLink==0);
    }


    /* pan scan present flag */
    Value = vc1DECBIT_GetBits(pBitstream, VC1_BITS_PANSCAN_FLAG);
    if(VC1DECBIT_EOF == Value)
    {
        return(vc1_ResultBufferExhausted);
    }
    pParams->PanScanFlag = (FLAG)Value;
    DEBUG1(vc1DEBUG_ENT, "PANSCAN_FLAG: %d\n", Value);
    COVERAGE("6.2.3");

    
    /* reference frame distance flag */
    Value = vc1DECBIT_GetBits(pBitstream, VC1_BITS_REFDIST_FLAG);
    if(VC1DECBIT_EOF == Value)
    {
        return(vc1_ResultBufferExhausted);
    }
    pParams->RefDistFlag = (FLAG)Value;
    DEBUG1(vc1DEBUG_ENT, "REFDIST_FLAG: %d\n", Value);
    COVERAGE("6.2.4");


    /* loop filter flag */
    Value = vc1DECBIT_GetBits(pBitstream, VC1_BITS_LOOPFILTER);
    if(VC1DECBIT_EOF == Value)
    {
        return(vc1_ResultBufferExhausted);
    }
    pParams->LoopFilter = (FLAG)Value;
    DEBUG1(vc1DEBUG_ENT, "LOOPFILTER: %d\n", Value);
    COVERAGE("6.2.5");


    /* fast uv motion compensation flag */
    Value = vc1DECBIT_GetBits(pBitstream, VC1_BITS_FASTUVMC);
    if(VC1DECBIT_EOF == Value)
    {
        return(vc1_ResultBufferExhausted);
    }
    pParams->FastUVMC = (FLAG)Value;
    pState->sPosition.FastUVMC = (FLAG)Value;
    DEBUG1(vc1DEBUG_ENT, "FASTUVMC: %d\n", Value);
    COVERAGE("6.2.6");


    /* extended motion vector flag */
    Value = vc1DECBIT_GetBits(pBitstream, VC1_BITS_EXTENDED_MV);
    if(VC1DECBIT_EOF == Value)
    {
        return(vc1_ResultBufferExhausted);
    }
    pParams->ExtendedMV = (FLAG)Value;
    DEBUG1(vc1DEBUG_ENT, "EXTENDED_MV: %d\n", Value);
    COVERAGE("6.2.7");


    /* macroblock quantisation flag */
    Value = vc1DECBIT_GetBits(pBitstream, VC1_BITS_DQUANT);
    if(VC1DECBIT_EOF == Value)
    {
        return(vc1_ResultBufferExhausted);
    }
    pParams->DQuant = (UBYTE8)Value;
    DEBUG1(vc1DEBUG_ENT, "DQUANT: %d\n", Value);
    COVERAGE("6.2.8");


    /* variable sized transform flag */
    Value = vc1DECBIT_GetBits(pBitstream, VC1_BITS_VSTRANSFORM);
    if(VC1DECBIT_EOF == Value)
    {
        return(vc1_ResultBufferExhausted);
    }
    pParams->VSTransform = (FLAG)Value;
    DEBUG1(vc1DEBUG_ENT, "VSTRANFORM: %d\n", Value);
    COVERAGE("6.2.9");


    /* overlapped transform flag */
    Value = vc1DECBIT_GetBits(pBitstream, VC1_BITS_OVERLAP);
    if(VC1DECBIT_EOF == Value)
    {
        return(vc1_ResultBufferExhausted);
    }
    pParams->OverlappedTransformFlag = (FLAG)Value;
    DEBUG1(vc1DEBUG_ENT, "OVERLAP: %d\n", Value);
    COVERAGE("6.2.10");


    /* quantiser specifier */
    Value = vc1DECBIT_GetBits(pBitstream, VC1_BITS_QUANTIZER);
    if(VC1DECBIT_EOF == Value)
    {
        return(vc1_ResultBufferExhausted);
    }
    pParams->eQuantizer = (vc1_eQuantizer)Value;
    DEBUG1(vc1DEBUG_ENT, "QUANTIZER: %d\n", Value);
    COVERAGE("6.2.11");

    
    if(0 != pParams->sHrdInitialState.NumLeakyBuckets)
    {
        int N;

        /* hypothetical reference decoder params */
        for(N = 0; N < pParams->sHrdInitialState.NumLeakyBuckets; N++)
        {
            Value = vc1DECBIT_GetBits(pBitstream, VC1_BITS_HRD_FULLNESS);
            if(VC1DECBIT_EOF == Value)
            {
                return(vc1_ResultBufferExhausted);
            }

            DEBUG2(vc1DEBUG_ENT, "HRD_FULLNESS[%d]: %d\n", N, Value);
            COVERAGE("6.2.12");
            pParams->sHrdInitialState.sLeakyBucket[N].Fullness = 
                (pParams->sHrdInitialState.sLeakyBucket[N].Buffer*(Value+1))/256;

            DEBUG2(vc1DEBUG_ENT, "HRD Fullness[%d] = %10d\n", 
                N, pParams->sHrdInitialState.sLeakyBucket[N].Fullness);
        }
        /* initialise the hypothetical reference decoder with these data */
        vc1HRD_Init(&pParams->sHrdInitialState, &pParams->sHrdInitialState);
    }


    /* coded size flag */
    Value = vc1DECBIT_GetBits(pBitstream, VC1_BITS_CODED_SIZE_FLAG);
    if(VC1DECBIT_EOF == Value)
    {
        return(vc1_ResultBufferExhausted);
    }
    DEBUG1(vc1DEBUG_ENT, "CODED_SIZE_FLAG: %d\n", Value);
    COVERAGE("6.2.13");


    if(TRUE == Value)
    {
        /* coded frame width */
        Value = vc1DECBIT_GetBits(pBitstream, VC1_BITS_CODED_WIDTH);
        if(VC1DECBIT_EOF == Value)
        {
            return(vc1_ResultBufferExhausted);
        }
        pParams->CodedWidth = (UHWD16)(2 * (Value + 1));
        DEBUG2(vc1DEBUG_ENT, "CODED_WIDTH: %d (Width=%d)\n", Value, 2*(Value+1));
        if (pParams->CodedWidth > pParams->MaxCodedWidth)
        {
            FATAL("Coded width bigger than max coded width\n");
            return vc1_ResultFatal;
        }
        COVERAGE("6.2.13.1");


        /* coded frame height */
        Value = vc1DECBIT_GetBits(pBitstream, VC1_BITS_CODED_HEIGHT);
        if(VC1DECBIT_EOF == Value)
        {
            return(vc1_ResultBufferExhausted);
        }
        pParams->CodedHeight = (UHWD16)(2 * (Value + 1));
        DEBUG2(vc1DEBUG_ENT, "CODED_HEIGHT: %d (Height=%d)\n", Value, 2*(Value+1));
        if (pParams->CodedHeight > pParams->MaxCodedHeight)
        {
            FATAL("Coded height bigger than max coded height\n");
            return vc1_ResultFatal;
        }
        COVERAGE("6.2.13.2");
    }


    if(TRUE == pParams->ExtendedMV)
    {
        /* extended differential motion vector range flag */
        Value = vc1DECBIT_GetBits(pBitstream, VC1_BITS_EXTENDED_DMV);
        if(VC1DECBIT_EOF == Value)
        {
            return(vc1_ResultBufferExhausted);
        }
        pParams->ExtendedDMV = (FLAG)Value;
        DEBUG1(vc1DEBUG_ENT, "EXTENDED_DMV: %d\n", Value);
        COVERAGE("6.2.14");
    }


    /* range mapping luminance flag */
    Value = vc1DECBIT_GetBits(pBitstream, VC1_BITS_RANGE_MAPY_FLAG);
    if(VC1DECBIT_EOF == Value)
    {
        return(vc1_ResultBufferExhausted);
    }
    DEBUG1(vc1DEBUG_ENT, "RANGE_MAPY_FLAG: %d\n", Value);
    COVERAGE("6.2.15");

    if (TRUE == Value)
    {
        /* RANGE_MAPY */
        Value = vc1DECBIT_GetBits(pBitstream, VC1_BITS_RANGE_MAPY);
        if(VC1DECBIT_EOF == Value)
        {
            return(vc1_ResultBufferExhausted);
        }
        pState->sPosition.RangeYScale = (UBYTE8)(Value + 9);
        DEBUG2(vc1DEBUG_ENT, "RANGE_MAPY: %d (Scale=%d/8)\n", Value, Value+9);
        COVERAGE("6.2.15.1");
    }
    else
    {
        /* equivalent to off */
        pState->sPosition.RangeYScale = 8;
    }

    
    /* range mapping chrominance flag */
    Value = vc1DECBIT_GetBits(pBitstream, VC1_BITS_RANGE_MAPUV_FLAG);
    if(VC1DECBIT_EOF == Value)
    {
        return(vc1_ResultBufferExhausted);
    }
    DEBUG1(vc1DEBUG_ENT, "RANGE_MAPUV_FLAG: %d\n", Value);
    COVERAGE("6.2.16");

    if(TRUE == Value)
    {
        /* RANGE_MAPUV */
        Value = vc1DECBIT_GetBits(pBitstream, VC1_BITS_RANGE_MAPUV);
        if(VC1DECBIT_EOF == Value)
        {
            return(vc1_ResultBufferExhausted);
        }
        pState->sPosition.RangeUVScale = (UBYTE8)(Value + 9);
        DEBUG2(vc1DEBUG_ENT, "RANGE_MAPUV: %d (Scale=%d/8)\n", Value, Value+9);
        COVERAGE("6.2.16.1");
    }
    else
    {
        /* equivalent to off */
        pState->sPosition.RangeUVScale = 8;
    }


    /* set position structure's width/height */
    pState->sPosition.WidthMB  = pParams->CodedWidth;
    pState->sPosition.HeightMB = pParams->CodedHeight;

    return(vc1_ResultOK);
}






