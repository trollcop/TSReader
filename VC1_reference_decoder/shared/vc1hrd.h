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
 * vc1hrd.h
 *
 * Hypothetical Reference Decoder model module
 *
 *  
 */

#ifndef VC1HRD_H
#define VC1HRD_H

#ifdef cplusplus
extern "C" {
#endif

/**
 * Description:
 * Initialize the HRD state.
 *
 * Inputs:
 * pHrdState        - Pointer to Hypothetical reference decoder state
 * pHrdInitialState - Pointer to HRD information in sequence header
 */

void vc1HRD_Init(vc1_sHrdState *pHrdState, const vc1_sHrdState *pHrdInitialState);

/**
 * Description:
 * Remove bits from the hypothetical reference coder buffers.
 *
 * Remarks:
 * When decoding a frame, call this function to remove the number of bits in the frame
 * from the HRD model.
 *
 * Inputs:
 * pHrdState - Pointer to Hypothetical reference decoder state
 * Bits      - Number of bits to remove
 *
 * Returns:
 * vc1_ResultOK           - if the call succeeded
 * vc1_ResultHrdUnderflow - if there were not enough bits to remove
 */

vc1_eResult vc1HRD_RemoveBits(vc1_sHrdState *pHrdState, HRDVALUE Bits);

/**
 * Description:
 * Add the number of bits that have arrived in a given time.
 *
 * Inputs:
 * pHrdState - Pointer to Hypothetical reference decoder state
 * SecNum    - Seconds Numerator   (frame rate denominator)
 * SecDen    - Seconds Denominator (frame rate numerator)
 *
 * Outputs:
 * pHrdState - Updated for the extra bits arriving in this time
 */

void vc1HRD_AddBits(vc1_sHrdState *pHrdState, unsigned SecNum, unsigned SecDen);

/**
 * Description:
 * Calculate the minimum buffer fullness.
 *
 * Remarks:
 * If the next frame occupies more bits than the minimum buffer fullness
 * then the decoder will suffer a buffer underflow error.
 *
 * Inputs:
 * pHrdState - Pointer to Hypothetical reference decoder state
 *
 * Returns:
 * Minimum buffer fullness
 */

HRDVALUE vc1HRD_MinFullness(vc1_sHrdState *pHrdState);

#ifdef cplusplus
}
#endif

#endif
