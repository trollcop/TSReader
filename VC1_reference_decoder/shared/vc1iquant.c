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
 * vc1iquant.c:
 * Inverse quantisation functions
 *
 */

#include "vc1types.h"
#include "vc1debug.h"
#include "vc1preddcac.h"
#include "vc1iquant.h"

static const UBYTE8 vc1IQUANT_NonUniformImplicit[32] =
{
     0,  0,  0,  0,  0,  0,  0,  0,
     0,  6,  7,  8,  9, 10, 11, 12,
    13, 14, 15, 16, 17, 18, 19, 20,
    21, 22, 23, 24, 25, 27, 29, 31
};

/*
 * Description:
 * Calculates quantizer according to quantization mode
 *
 * Remarks:
 * This does not handle the quantization modes:
 * vc1_QuantModeMBDual, vc1_QuantModeMBAny 
 *
 * Inputs:
 * pQuant       - Pointer to picture quantizer (PQUANT, HALFQP, NONUNIFORM)
 * pPos         - Pointer to position and current macroblock
 * eQuantMode   - Quantization mode
 * AltQuant     - ALTPQUANT value
 *
 * Outputs:
 * pQuant       - Updated to macroblock quantization value
 */

void vc1IQUANT_ChooseQuantizer(
    vc1_sQuant *pQuant,
    vc1_sPosition *pPos,
    vc1_eQuantMode eQuantMode,
    int AltQuant
)
{
    int X     = pPos->X;
    int Y     = pPos->Y + pPos->SliceY;
    int MaxX  = pPos->WidthMB - 1;
    int MaxY  = pPos->HeightMB - 1;
    int HalfQP = pQuant->HalfStep;
    int Quant  = pQuant->Quant;

    switch (eQuantMode)
    {
    case vc1_QuantModeAllEdges:
        if (X==0 || Y==0 || X==MaxX || Y==MaxY)
        {
            Quant = AltQuant;
            HalfQP = 0;
        }
        break;

    case vc1_QuantModeLeftTop:
        if (X==0 || Y==0)
        {
            Quant = AltQuant;
            HalfQP = 0;
        }
        break;

    case vc1_QuantModeTopRight:
        if (Y==0 || X==MaxX)
        {
            Quant = AltQuant;
            HalfQP = 0;
        }
        break;

    case vc1_QuantModeRightBottom:
        if (Y==MaxY || X==MaxX)
        {
            Quant = AltQuant;
            HalfQP = 0;
        }
        break;

    case vc1_QuantModeBottomLeft:
        if (X==0 || Y==MaxY)
        {
            Quant = AltQuant;
            HalfQP = 0;
        }
        break;

    case vc1_QuantModeLeft:
        if (X==0)
        {
            Quant = AltQuant;
            HalfQP = 0;
        }
        break;

    case vc1_QuantModeTop:
        if (Y==0)
        {
            Quant = AltQuant;
            HalfQP = 0;
        }
        break;

    case vc1_QuantModeRight:
        if (X==MaxX)
        {
            Quant = AltQuant;
            HalfQP = 0;
        }
        break;

    case vc1_QuantModeBottom:
        if (Y==MaxY)
        {
            Quant = AltQuant;
            HalfQP = 0;
        }
        break;

    default: /* vc1_QuantModeDefault */
        break;
    }

    DEBUG2(vc1DEBUG_QUANT, "ChooseQuant = %d, HalfStep = %d\n", Quant, HalfQP);
    pQuant->Quant = (UBYTE8)Quant;
    pQuant->HalfStep = (UBYTE8)HalfQP;
}

/*
 * Description:
 * Converts PQINDEX to a PQUANT and NonUniform according to current Quantizer mode
 *
 * Remarks:
 *
 * Inputs:
 * pQuant     - Pointer to vc1_sQuant structure to fill
 * PQINDEX    - PQINDEX in the range 1 to 31
 * eQuantizer - Quantizer mode, one of:
 *              vc1_QuantizerImplicit,
 *              vc1_QuantizerUniform,
 *              vc1_QuantizerNonUniform
 *
 * Outputs:
 * pQuant     - Updated with new quantisation level and quantiser
 *
 * Return Value:
 * None.
 *
 */

void vc1IQUANT_GetQuantizer(vc1_sQuant *pQuant, int PQINDEX, vc1_eQuantizer eQuantizer)
{
    int PQUANT = PQINDEX;
    int NonUniform = 0;

    switch (eQuantizer)
    {
    case vc1_QuantizerImplicit:
        if (PQINDEX>=9)
        {
            PQUANT = vc1IQUANT_NonUniformImplicit[PQINDEX];
            NonUniform = 1;
        }
        break;

    case vc1_QuantizerNonUniform:
        NonUniform = 1;
        break;

    default:      /* No action needed for other cases */
        break;
    }

    pQuant->Quant = (UBYTE8)PQUANT;
    pQuant->NonUniform = (FLAG)NonUniform;
}


/*
 * Description:
 * Inverse quantise a DC coefficient, given a quantiser and quantisation level
 *
 * Remarks:
 * Uses DCStepSize() with DC/AC prediction module
 *
 * Inputs:
 * DCCoeffQ   - quantised DC coefficient
 * pQuant     - pointer to vc1_sQuant structure
 *
 * Outputs:
 * None
 *
 * Return Value:
 * Dequantised DC coefficient
 *
 */

HWD16 vc1IQUANT_InverseDCQuantize(WORD32 DCCoeffQ, vc1_sQuant * pQuant)
{
    HWD16   DCStepSize;
    UBYTE8  PQuant = pQuant->Quant;

    if((1 > PQuant) || (31 < PQuant))
    {
        FATAL("vc1IQUANT_DCCoefIQuant: PQUANT = %d\n", PQuant);
        return(0);
    }

    COVERAGE("8.1.1.9");
    DCStepSize = (HWD16)vc1PREDDCAC_DCStepSize(PQuant);

    return((HWD16)(DCCoeffQ * DCStepSize));
}


/*
 * Description:
 * Inverse quantise a group of AC coefficients, given a quantiser, and quantisation level
 *
 * Remarks:
 *
 * Inputs:
 * TCoefs     - pointer to a 64-element array of transform coefficients
 * pQuant     - pointer to the vc1_sQuant structure for this block
 * Intra      - 0=Inter 1=Intra
 *
 * Outputs:
 * pOut       - pointer to a 64-element array of transform coefficients, with dequantised 
 *              AC coefficients
 *
 * Return Value:
 * None
 *
 */

void vc1IQUANT_InverseACQuantize(
    HWD16 * pOut,
    const HWD16 * TCoefs,
    vc1_sQuant * pQuant,
    int Intra
)
{
    int  i = Intra;
    UBYTE8  MQuant = pQuant->Quant;
    FLAG    HalfStep = pQuant->HalfStep;

    DEBUG3(vc1DEBUG_QUANT, "MQUANT=%d HalfStep=%d NonUniform=%d\n",
        MQuant, HalfStep, pQuant->NonUniform);

    COVERAGE("8.1.1.14");
    if(FALSE == pQuant->NonUniform)
    {
        for( ; i < 64; i++)
        {
            if(0 != TCoefs[i])
            {
                pOut[i] = (HWD16)(TCoefs[i] * (2 * MQuant + HalfStep));
            }
            else
            {
                pOut[i] = 0;
            }
        }
    }
    else if(TRUE == pQuant->NonUniform)
    {
        for( ; i < 64; i++)
        {
            if(0 != TCoefs[i])
            {
                pOut[i] = (HWD16)(TCoefs[i] * (2 * MQuant + HalfStep) + SIGN(TCoefs[i]) * MQuant);
            }
            else
            {
                pOut[i] = 0;
            }
        }
    }
    else
    {
        WARN("vc1DECBLK_InverseACQuantize: Unknown quantizer %d\n", pQuant->NonUniform);
    }
}

