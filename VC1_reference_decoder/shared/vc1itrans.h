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
 * vc1itrans.h
 *
 * vc1 Decoder Inverse Transform module
 *
 *  
 */

#ifndef VC1ITRANS_H
#define VC1ITRANS_H

#ifdef cplusplus
extern "C" {
#endif


/**
 * Description:
 * 2D inverse transform an 8x8 block of coefficients.
 *
 * Inputs:
 * pIn        - pointer to the input 64-element array
 * eType      - tranform type (8x8, 8x4, 4x8, 4x4)
 * pOut       - pointer to the output 64-element array
 *
 * Outputs:
 * pOut       - array filled
 *
 */
void vc1ITRANS_InverseTransformBlock(HWD16 *pOut, HWD16 *pIn, vc1_eBlkType eType);

#ifdef cplusplus
}
#endif

#endif
