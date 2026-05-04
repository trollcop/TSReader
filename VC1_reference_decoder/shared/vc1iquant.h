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
 * vc1iquant.h:
 * Inverse quantisation functions
 *
 */

#ifndef VC1IQUANT_H
#define VC1IQUANT_H

#ifdef cplusplus
extern "C" {
#endif

/**
 * Description:
 * Calculates quantizer according to quantization mode.
 *
 * Remarks:
 * This does not handle the quantization modes:
 * - vc1_QuantModeMBDual
 * - vc1_QuantModeMBAny 
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
);

/**
 * Description:
 * Converts PQINDEX to a PQUANT and NonUniform according to current Quantizer mode.
 *
 * Inputs:
 * pQuant     - Pointer to vc1_sQuant structure to fill
 * PQINDEX    - PQINDEX in the range 1 to 31
 * eQuantizer - Quantizer mode, one of:
 *              - vc1_QuantizerImplicit
 *              - vc1_QuantizerUniform
 *              - vc1_QuantizerNonUniform
 *
 * Outputs:
 * pQuant     - Updated with new quantisation level and quantiser
 *
 */

void vc1IQUANT_GetQuantizer(vc1_sQuant *pQuant, int PQINDEX, vc1_eQuantizer eQuantizer);

/**
 * Description:
 * Inverse quantise a DC coefficient, given a quantiser and quantisation level.
 *
 * Remarks:
 * This function uses vc1PREDDCAC_DCStepSize() from the DC/AC prediction module.
 *
 * Inputs:
 * DCCoeffQ   - quantised DC coefficient
 * pQuant     - pointer to vc1_sQuant structure
 *
 * Return Value:
 * Dequantised DC coefficient
 *
 */
HWD16 vc1IQUANT_InverseDCQuantize(WORD32 DCCoeffQ, vc1_sQuant * pQuant);

/**
 * Description:
 * Inverse quantise a group of AC coefficients, given a quantiser, and quantisation level
 *
 * Remarks:
 *
 * Inputs:
 * pOut       - pointer to a 64-element array to fill
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
);

#ifdef cplusplus
}
#endif

#endif
