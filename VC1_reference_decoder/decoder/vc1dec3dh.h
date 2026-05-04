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
 * vc1dec3dh.h
 *
 * vc1 Decoder 3D Huffman header
 *
 *  
 */

#ifndef VC1DEC3DH_H
#define VC1DEC3DH_H

#ifdef cplusplus
extern "C" {
#endif

/**
 * Description:
 * Structure to hold a run, level and last triple.
 *
 * Remarks:
 * This structure is used as an array element, where the index value
 * is the code for the run, level and last triple.
 */

typedef struct
{
    BYTE8 Run;
    BYTE8 Level;
    BYTE8 Last;
} vc1DEC3DH_sRunLevel;


typedef enum
{
    vc1DEC3DH_EscapeMode1 = 1,
    vc1DEC3DH_EscapeMode2,
    vc1DEC3DH_EscapeMode3
} vc1DEC3DH_eEscapeMode;


/**
 * Description:
 * Decode AC run/level information.
 *
 * Remarks:
 * Calls UnpackTransformACCoef() for each coefficient, and stores the result in the pCoefs array.
 * This code is based on the pseudo-code in figure titled "Coefficient decode pseudo-code"
 * of the standard.
 * Possible improvements include combining this function with the de-zigzag operation.
 *
 * Inputs:
 * pState     - pointer to the decoder state structure
 * pBitstream - pointer to the bitstream structure
 * pCoefs     - pointer to a 64 element array, into which coefficients will be written
 * pPosition  - pointer to position structure indicating current MB
 * eBlk       - the block number to which these AC coefficients relate
 *
 * Outputs:
 * pState     - updated with data read from the bitstream
 * pBitstream - updated with new bitstream position
 * pCoefs     - updated with coefficients unpacked from the bitstream
 *
 * Return Value:
 * Standard vc1_eResult result. See enumeration for possible result codes.
 * 
 */
vc1_eResult vc1DEC3DH_DecodeACRunLevel(vc1DEC_sState * pState,
                                       vc1DEC_sBitstream * pBitstream,
                                       HWD16 * pCoefs,
                                       vc1_sPosition * pPosition,
                                       int eBlk);

#ifdef cplusplus
}
#endif

#endif
