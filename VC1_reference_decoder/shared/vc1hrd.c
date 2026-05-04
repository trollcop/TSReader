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
 * vc1hrd.c
 *
 * Hypothetical Reference Decoder model module
 *
 *  
 */

#include "vc1types.h"
#include "vc1debug.h"
#include "vc1hrd.h"

/*
 * Description:
 * Initialize the HRD state
 *
 * Inputs:
 * pHrdState        - Pointer to Hypothetical reference decoder state
 * pHrdInitalState  - Pointer to HRD information in sequence header
 */

void vc1HRD_Init(vc1_sHrdState *pHrdState, const vc1_sHrdState *pHrdInitialState)
{
    int i, N;

    *pHrdState = *pHrdInitialState;
    N = pHrdState->NumLeakyBuckets;

    for (i=0; i<N; i++)
    {
        vc1_sLeakyBucket *pBucket = &pHrdState->sLeakyBucket[i];

        pBucket->FullFraction = 0;
    }
}


/*
 * Description:
 * Remove bits from the hypothetical reference coder buffers
 *
 * Remarks:
 * When decoding a frame remove the number of bits in the frame
 *
 * Inputs:
 * pHrd     - Pointer to Hypothetical reference decoder state
 * Bits     - Number of bits to remove
 *
 * Returns:
 * vc1_ResultOK
 * vc1_ResultHrdUnderflow 
 */

vc1_eResult vc1HRD_RemoveBits(vc1_sHrdState *pHrdState, HRDVALUE Bits)
{
    vc1_eResult eResult = vc1_ResultOK;
    int N = pHrdState->NumLeakyBuckets;
    int i;

    for (i=0; i<N; i++)
    {
        vc1_sLeakyBucket *pBucket = &pHrdState->sLeakyBucket[i];
        HRDVALUE Fullness = pBucket->Fullness;

        DEBUG3(vc1DEBUG_HRD, "Buffer[%d] = %10d/%10d\n", i, Fullness, pBucket->Buffer);

        if (Fullness < Bits)
        {
            WARN("Hypothetical Reference Decoder Underflow i=%d F=%d\n", i, Fullness);
            eResult = vc1_ResultHrdUnderflow;
            Fullness = 0;
        }
        else
        {
            Fullness -= Bits;
        }
        pBucket->Fullness = Fullness;
    }

    COVERAGE("C.1.2");
    return eResult;
}

/*
 * Description:
 * Add the number of bits that have arrived in a given time
 *
 * Inputs:
 * pHrd     - Pointer to Hypothetical reference decoder state
 * SecNum   - Seconds Numerator   (frame rate denominator)
 * SecDen   - Seconds Denominator (frame rate numerator)
 *
 * Outputs:
 * pHrd     - Updated for the extra bits arriving in this time
 */

void vc1HRD_AddBits(vc1_sHrdState *pHrdState, unsigned SecNum, unsigned SecDen)
{
    int N = pHrdState->NumLeakyBuckets;
    int i;

    if (SecDen==0 || SecNum==0)
    {
        /* Default to 15fps = 1/15 secs per frame */
        SecNum = 1;
        SecDen = 15;
    }

    DEBUG2(vc1DEBUG_HRD, "Receiving bits for %d/%d secs\n", SecNum, SecDen);

    for (i=0; i<N; i++)
    {
        vc1_sLeakyBucket *pBucket = &pHrdState->sLeakyBucket[i];
        HRDVALUE BufferSize = pBucket->Buffer;
        UWORD32 Bits;
        UWORD32 Fraction;
            
        if (pBucket->FullFraction != 0)
        {
            ASSERT(pBucket->FullDenominator == SecDen);
        }
        else
        {
            pBucket->FullDenominator = SecDen;
        }

        /* Calculate number of complete bits arrived and fraction left over
         * CompleteBits = (Rate*SecNum + FullFraction) / SecDen
         * FractionBits = (Rate*SecNum + FullFraction) % SecDen
         */
#ifdef __STRICT_ANSI__
        {   /* No 64-bit type */
            UWORD32 RateQ = pBucket->Rate / SecDen;
            UWORD32 RateR = pBucket->Rate % SecDen;
            UWORD32 SecQ = SecNum / SecDen;
            UWORD32 SecR = SecNum % SecDen;
            
            Fraction = RateR*SecR + pBucket->FullFraction;
            Bits = RateQ*SecQ*SecDen + RateQ*SecR + RateR*SecQ + (Fraction / SecDen);
            Fraction = Fraction % SecDen;
        }
#else
        {   /* Use 64-bit type */
            ULLONG64 Acc;
            Acc = (ULLONG64)pBucket->Rate * (ULLONG64)SecNum + (ULLONG64)pBucket->FullFraction;
            Bits = (UWORD32)(Acc / SecDen);
            Fraction = (UWORD32)(Acc % SecDen);
        }
#endif
        pBucket->FullFraction = Fraction;
        pBucket->Fullness += (HRDVALUE)Bits;

        DEBUG1(vc1DEBUG_HRD, "Adding %d bits\n", Bits);

        if (pBucket->Fullness >= BufferSize)
        {
            pBucket->Fullness = BufferSize;
            pBucket->FullFraction = 0;
        }

    }
}

/*
 * Description:
 * Calculate the minimum buffer fullness
 *
 * Remarks:
 * If the next frame occupies more bits than the minimum buffer fullness
 * then the decoder will suffer a buffer underflow error.
 *
 * Inputs:
 * pHrd     - Pointer to Hypothetical reference decoder state
 *
 * Returns:
 * Minimum buffer fullness
 */

HRDVALUE vc1HRD_MinFullness(vc1_sHrdState *pHrdState)
{
    int N = pHrdState->NumLeakyBuckets;
    int i;
    HRDVALUE Min = 0xFFFFFFFF;

    for (i=0; i<N; i++)
    {
        vc1_sLeakyBucket *pBucket = &pHrdState->sLeakyBucket[i];

        if (pBucket->Fullness < Min)
        {
            Min = pBucket->Fullness;
        }
    }

    return Min;
}


