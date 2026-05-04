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
 * vc1cropmv.h
 *
 * Crop motion vectors to reference picture
 *
 *  
 */

#ifndef VC1CROPMV_H
#define VC1CROPMV_H

#ifdef cplusplus
extern "C" {
#endif

/**
 * Description:
 * Pull back motion vector after preliminary prediction.
 *
 * Remarks:
 * Simple/Main/Advanced progressive only.
 *
 * Inputs:
 * pPosition    - pointer to current position
 * pMV          - pointer to motion vector to pull back
 * Blk          - Block number for 4MV
 *
 * Outputs:
 * pMV          - pulled back to (-15,width-1)x(-15,height-1)
 */

void vc1CROPMV_PPredPullBack(
    vc1_sPosition *pPosition,
    vc1_sMV *pMV,
    int Blk
);

/**
 * Description:
 * Crop the motion vector prior to using it for B motion vector prediction.
 *
 * Remarks:
 * Main profile B pictures only.
 *
 * Inputs:
 * pPosition    - pointer to current position
 * pMV          - pointer to motion vector to pull back
 * Blk          - block number
 */

void vc1CROPMV_BPredPullBack(vc1_sPosition *pPosition, vc1_sMV *pMV, int Blk);

/**
 * Description:
 * Pull back luma motion vector prior to motion compensation.
 *
 * Remarks:
 * All levels and profiles.
 *
 * Inputs:
 * pPosition    - pointer to current position
 * pMV          - pointer to motion vector to pull back
 *
 * Outputs:
 * pMV          - pulled back
 */

void vc1CROPMV_LumaPullBack(vc1_sPosition *pPosition, vc1_sMV *pMV);

/**
 * Description:
 * Crop the integer part of a chroma motion vector.
 *
 * Remarks:
 * Crops integer part to the range [-8,WidthMB*8] x [-8, HeightMB*8].
 * The fractional part is not altered. This operation is performed
 * before interpolation of chroma motion vectors, or Direct storage
 * and also performed on Chroma motion vectors before they are used
 * for deblocking comparison.
 *
 * Remarks:
 * Simple and main profile only.
 *
 * Inputs:
 * pPosition    - pointer to current position
 * pMV          - pointer to motion vector to pull back
 */

void vc1CROPMV_ChromaPullBack(vc1_sPosition *pPosition, vc1_sMV *pMV);

#ifdef cplusplus
}
#endif
#endif
