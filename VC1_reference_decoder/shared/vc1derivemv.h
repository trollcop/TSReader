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
 * vc1derivemv.h:
 *
 * Derive chroma motion vectors and direct motion vectors
 *
 */


#ifndef VC1DERIVEMV_H
#define VC1DERIVEMV_H

#ifdef cplusplus
extern "C" {
#endif

/**
 * Description:
 * Derive chroma and direct motion vectors.
 *
 * Remarks:
 * Reconstructs forward and/or backward motion vectors depending on the MBType.
 * This function must be called exactly once for each macroblock as it also
 * manages the motion vector history for the reference pictures.
 *
 * Inputs:
 * pPosition   - pointer to current position structure
 *
 * Outputs:
 * pPosition   - current macroblock updated with derived motion vectors
 *
 */

void vc1DERIVEMV_DeriveMV(vc1_sPosition * pPosition);

/**
 * Description:
 * Calculate and apply the direct motion vector for a B macroblock.
 *
 * Remarks:
 * This function is only called for B pictures.
 *
 * Inputs:
 * pPosition    - pointer to current position structure
 *
 * Outputs:
 * pPosition is updated as follows:
 *              - If current MB is forward, fills in backward MV
 *              - If current MB is backward, fills in forward MV
 *              - If current MB is direct, fills in both MV
 */

void vc1DERIVEMV_DirectMV(vc1_sPosition * pPosition);

/**
 * Description:
 * Derive a chroma motion vector.
 *
 * Remarks:
 * eBlk is only used for interlaced frame chroma motion vectors.
 *
 * Inputs:
 * pMV          - pointer to a struct which will contain the new MV
 * pPosition    - pointer to current position
 * eBlk         - luma block number from which chroma mv is derived
 * Backwards    - 0=forwards 1=backwards
 *
 * Outputs:
 * pMV      - updated with new motion vector
 *
 */

void vc1DERIVEMV_DeriveChromaMV(
    vc1_sMV * pMV,
    vc1_sPosition * pPosition,
    int eBlk,
    int Backwards
);

/**
 * Description:
 * Decide if a chroma block is inter or intra, depending on the luma blocks
 * in the same macroblock.
 *
 * Inputs:
 * pPosition    - pointer to macroblock position structure
 *
 * Outputs:
 * Sets Cb and Cr block types for current macroblock
 *
 */
void vc1DERIVEMV_DecideChromaBlockType(vc1_sPosition * pPosition);


/**
 * Description:
 * Store the luma motion vectors to the history buffer.
 *
 * Remarks:
 * Stores the block type (intra/inter) with a defined vc1_MVIntra value.
 *
 * Inputs:
 * pPosition    - pointer to macroblock position structure
 *
 * Outputs:
 * pPosition    - motion vector history buffer updated
 *
 */
void vc1DERIVEMV_StoreMotionVectors(vc1_sPosition * pPosition);


#ifdef cplusplus
}
#endif

#endif
