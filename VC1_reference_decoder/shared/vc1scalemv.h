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
 * vc1scalemv.h
 *
 * Scale motion vectors for field interlaced motion vector prediction
 *
 *  
 */

#ifndef VC1SCALEMV_H
#define VC1SCALEMV_H

#ifdef cplusplus
extern "C" {
#endif

/**
 * Description:
 * Initialize the motion vector interlace scaling structures
 *
 * Inputs:
 * pPos     - pointer to position structure
 *
 * Outputs:
 * pPos->pScaleMV[0]  - forward motion vector scaling parameters (P and B pictures)
 * pPos->pScaleMV[1]  - backward motion vector scaling parameters (B pictures only)
 */

void vc1SCALEMV_InitScaleMV(vc1_sPosition *pPos);

/**
 * Description:
 * Scale an interlaced motion vector from same to opposite field.
 *
 * Inputs:
 * pX       - pointer to X of 1/4 pel motion vector
 * pY       - pointer to Y of 1/4 pel motion vector
 * pSMV     - pointer to motion vector scaling parameter
 * Opposite - 0 = Scale for same field, 
 *            1 = Scale for opposite field
 *
 * Outputs:
 * pX       - pointer to scaled X
 * pY       - pointer to scaled Y
 */

void vc1SCALEMV_ScaleMV(
    int *pX,
    int *pY,
    const vc1_sScaleMV *pSMV,
    int Opposite
);

#ifdef cplusplus
}
#endif
#endif
