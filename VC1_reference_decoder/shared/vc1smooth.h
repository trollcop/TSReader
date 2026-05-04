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
 * vc1smooth.h:
 * Post inverse transform overlap smooth functions
 *
 */

#ifndef VC1SMOOTH_H
#define VC1SMOOTH_H

#ifdef cplusplus
extern "C" {
#endif

/**
 * Description:
 * Horizontal overlap smooth the top edges of blocks in the current macroblock.
 *
 * Remarks:
 * This function is only called when OverlapFilter is enabled for the macroblock.
 * This function copies the bottom block rows into the macroblock state,
 * then filters over horizontal edges as required.
 * 
 *
 * Inputs:
 * pRefPic      - Pointer to output reference picture
 * pPosition    - Pointer to position of current macroblock
 *
 * Outputs:
 * pRefPic      - Updated with smoothed pixels
 */

void vc1SMOOTH_OverlapSmoothHorizMB(
    vc1_sReferencePicture * pRefPic,
    vc1_sPosition *pPosition
);

/**
 * Description:
 * Perform overlap smoothing on this and the previous macroblock.
 *
 * Remarks:
 * This function must always be called even if overlap smoothing
 * is off for the current macroblock as it may be on for the
 * previous macroblock.
 * If overlap smoothing is on for the current macroblock it smooths
 * left vertical edges.
 * If overlap smoothing is on for the previous macroblock it smooths
 * top horizontal edges.
 *
 * Inputs:
 * pRefPic      - Pointer to output reference picture
 * pPosition    - Pointer to position of current macroblock
 * pData        - Coefficients for current macroblock
 *
 * Outputs:
 * pRefPic      - Updated with smoothed pixels
 */


void vc1SMOOTH_OverlapSmoothMB(
    vc1_sReferencePicture * pRefPic,
    vc1_sPosition *pPosition,
    HWD16 pData[6][64]
);

#ifdef cplusplus
}
#endif
#endif
