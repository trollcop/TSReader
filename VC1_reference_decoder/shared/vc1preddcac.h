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
 * vc1preddcac.h
 *
 * Predict DC and AC coefficients. Also holds shared tables
 * and functions relating to DC and AC quantization.
 *
 *  
 */

#ifndef VC1PREDDCAC_H
#define VC1PREDDCAC_H

#ifdef cplusplus
extern "C" {
#endif


/**
 * Description:
 * Calculate the default DC predictor for a block.
 *
 * Remarks:
 * In general the default is round((8*Bias)/DCStepSize).
 *
 * Inputs:
 * MQuant       - Macroblock quantizer
 * Bias         - Bias applied to DC coefficients (0 or 128)
 *
 * Return Value:
 * The calculated DC predictor.
 */

int vc1PREDDCAC_DCDefault(int MQuant, int Bias);

/**
 * Description:
 * Determine whether the ACPRED flag is present for a macroblock.
 *
 * Remarks:
 * The ACPRED flag is present if the macroblock contains an intra
 * block with an intra neighbour to the top or left.
 *
 * Inputs:
 * pPos     - Position of the current macroblock
 *
 * Return Value:
 * TRUE if ACPRED is present, otherwise FALSE.
 */

int vc1PREDDCAC_ACPREDPresent(vc1_sPosition *pPos);

/**
 * Description:
 * Predict the DC and AC values of a block from preceding blocks.
 *
 * Remarks:
 * This function is only called for an Intra 8x8 block.
 *
 * Inputs:
 * pPred        - Pointer to buffer for predicted DC and AC values
 * pPos         - Pointer to the macroblock being predicted
 * Blk          - Block number within the macroblock
 * DCDefault    - Default DC predictor
 *                - For Rule A it is 0
 *                - For Rule B it is ((1024 + (DCStepSize>>1)) / DCStepSize)
 *
 * Outputs:
 * pPred        - Pointer to the predicted output of 1 DC and then 7 AC values
 *
 * Return Value:
 * vc1_BlkIntra      - No AC values predicted
 * vc1_BlkIntraLeft  - Predicted left 7 AC values
 * vc1_BlkIntraTop   - Predicted top  7 AC values
 *
 */

vc1_eBlkType vc1PREDDCAC_PredictDCAC(
    HWD16 *pPred,
    vc1_sPosition *pPos,
    int Blk,
    int DCDefault);

/**
 * Description:
 * Calculate DCStepSize from the quantization step as
 * described in the standard.
 *
 * Remarks:
 * This function is only called for an Intra 8x8 block.
 *
 * Inputs:
 * Quant - quantisation step
 *
 * Return Value:
 * The calculated DCStepSize.
 *
 */
unsigned int vc1PREDDCAC_DCStepSize(unsigned int Quant);

/**
 * Description:
 * Copy the DC, Top and Left AC coefficients from a block to the block state structure.
 *
 * Inputs:
 * pData - Pointer to transformed quantised coefficients
 * pBlk  - Pointer to the block to update
 *
 * Outputs:
 * *pBlk - Block updated
 *
 */
void vc1PREDDCAC_CopyDCAC(vc1_sBlk *pBlk, HWD16 *pData);

extern const WORD32 vc1PREDDCAC_DQScaleTable[64];


#ifdef cplusplus
}
#endif
#endif
