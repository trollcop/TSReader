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
 * vc1predmv.h
 *
 * Predict motion vectors
 *
 *  
 */

#ifndef VC1PREDMV_H
#define VC1PREDMV_H

#ifdef cplusplus
extern "C" {
#endif

/**
 * Description:
 * Predict the X and Y motion vectors for a progressive block.
 *
 * Remarks:
 * For a 1MV macroblock Blk is ignored.
 * For a 4MV macroblock this function can be called to predict any of the four MVs.
 *
 * Inputs:
 * pPred        - Pointer to the predicted output of X, Y
 * pPos         - Pointer to the macroblock being predicted
 * Blk          - Block number within the macroblock (for 4MV)
 * eHybridPred  - Hybrid prediction direction
 * Back         - 0=predict forward motion 1=predict backward motion
 *
 * Outputs:
 * pPred        - predicted output values written
 *
 * Return Value:
 * TRUE if Hybrid prediction applied, otherwise FALSE.
 *
 */

int vc1PREDMV_PredictProgressiveMV(
    vc1_sMV *pPred,
    vc1_sPosition *pPos,
    int Blk,
    int eHybridPred,
    int Back
);

/**
 * Description:
 * Predict the X and Y motion vectors for an interlace field block.
 *
 * Remarks:
 * For a 1MV macroblock Blk is ignored.
 * For a 4MV macroblock this function can be called to predict any of the four MVs.
 *
 * Inputs:
 * pPred        - Pointer to the predicted output of X, Y
 * pPos         - Pointer to the macroblock being predicted
 * Blk          - Block number within the macroblock (for 4MV)
 * eHybridPred  - Hybrid prediction direction
 * PredFlag     - 0=use dominant predictor 1=use non-dominant predictor
 * Back         - 0=predict forward motion 1=predict backward motion
 *
 * Outputs:
 * pPred        - predicted output values written
 *
 * Return Value:
 * TRUE if Hybrid prediction applied, otherwise FALSE.
 *
 */

int vc1PREDMV_PredictInterlacedFieldMV(
    vc1_sMV *pPred,
    vc1_sPosition *pPos,
    int Blk,
    int eHybridPred,
    int PredFlag,
    int Back
);

/**
 * Description:
 * Predict the X and Y motion vectors for an interlace frame block.
 *
 * Remarks:
 * - For a 1MV Frame MB Blk must be 0.
 * - For a 2MV Field MB Blk can be 0 (top field) or 2 (bottom field).
 * - For a 4MV Frame MB Blk can be 0 to 3 (top left/right, bottom left/right block).
 * - For a 4MV Field MB Blk can be 0 to 3 (top left/right, bottom left/right field).
 *
 * Inputs:
 * pPred        - Pointer to the predicted output of X, Y
 * pPos         - Pointer to the macroblock being predicted
 * Blk          - Block number within the macroblock
 * Back         - 0=predict forward motion 1=predict backward motion
 *
 * Outputs:
 * pPred        - predicted output values written
 *
 */

void vc1PREDMV_PredictInterlacedFrameMV(
    vc1_sMV *pPred,
    vc1_sPosition *pPos,
    int Blk,
    int Back
);

#ifdef cplusplus
}
#endif
#endif
