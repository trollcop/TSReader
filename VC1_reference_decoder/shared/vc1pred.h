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
 * vc1pred.h
 *
 * Shared routines for use by prediction functions
 *
 *  
 */

#ifndef VC1PRED_H
#define VC1PRED_H

#ifdef cplusplus
extern "C" {
#endif

/**
 * Description:
 * Find the macroblock to the left of the current macroblock.
 *
 * Inputs:
 *      pPos    - pointer to position of current macroblock
 *
 * Returns:
 *      Pointer to the left macroblock or NULL if the macroblock
 *      is in the first column.
 */

vc1_sMB *vc1PRED_pLeftMB(vc1_sPosition *pPos);

/**
 * Description:
 * Find the macroblock to the top of the current macroblock.
 *
 * Inputs:
 *      pPos    - pointer to position of current macroblock
 *
 * Returns:
 *      Pointer to the top macroblock or NULL if the macroblock
 *      is in the first slice row.
 */

vc1_sMB *vc1PRED_pTopMB(vc1_sPosition *pPos);

/**
 * Description:
 * Find the macroblock to the top left of the current macroblock.
 *
 * Inputs:
 *      pPos    - pointer to position of current macroblock
 *
 * Returns:
 *      Pointer to the top left macroblock or NULL if the macroblock
 *      is in the first slice row.
 */

vc1_sMB *vc1PRED_pTopLeftMB(vc1_sPosition *pPos);

/**
 * Description:
 * Find the block to the left of the current block.
 *
 * Inputs:
 *      pPos    - pointer to position of current macroblock
 *      Blk     - Block within the macroblock
 *
 * Returns:
 *      Pointer to the left block or NULL if the block
 *      is in the first column.
 */

vc1_sBlk *vc1PRED_pLeftBlk(vc1_sPosition *pPos, int Blk);

/**
 * Description:
 * Find the block to the top of the current block.
 *
 * Inputs:
 *      pPos    - pointer to position of current macroblock
 *      Blk     - Block within the macroblock
 *
 * Returns:
 *      Pointer to the top block or NULL if the block
 *      is in the first row.
 */

vc1_sBlk *vc1PRED_pTopBlk(vc1_sPosition *pPos, int Blk);

/**
 * Description:
 * Find the block to the top left of the current block.
 *
 * Inputs:
 *      pPos    - pointer to position of current macroblock
 *      Blk     - Block within the macroblock
 *
 * Returns:
 *      Pointer to the top left block or NULL if the block
 *      is in the first row or first column.
 */

vc1_sBlk *vc1PRED_pTopLeftBlk(vc1_sPosition *pPos, int Blk);

/**
 * Description:
 * Find the block for predictor B in 4MV prediction.
 *
 * Remarks:
 * This is often the top right block but not always.
 *
 * Inputs:
 *      pPos    - pointer to position of current macroblock
 *      Blk     - Block within the macroblock
 *
 * Returns:
 *      Pointer to the B motion predictor block or 
 *      NULL if the block is in the first row
 */

vc1_sBlk *vc1PRED_pB4MVBlk(vc1_sPosition *pPos, int Blk);

/**
 * Description:
 * Find the block for predictor B in 1MV prediction.
 *
 * Remarks:
 * This is often the top right block but not always.
 *
 * Inputs:
 *      pPos    - pointer to position of the 1MV macroblock
 *
 * Returns:
 *      Pointer to the B motion predictor block or 
 *      NULL if the block is in the first row
 */

vc1_sBlk *vc1PRED_pB1MVBlk(vc1_sPosition *pPos);

#ifdef cplusplus
}
#endif
#endif
