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
 * vc1decbitpl.h:
 * Bitplane decoding functions
 *
 * 
 */

#ifndef VC1DECBITPL_H
#define VC1DECBITPL_H

#ifdef cplusplus
extern "C" {
#endif

/*
 * Defines for bitplanes
 *
 */

#define VC1DECBITPL_BITS_BITPLANE_INVERT  1
#define VC1DECBITPL_BITS_BITPLANE_ROWSKIP 1
#define VC1DECBITPL_BITS_BITPLANE_COLSKIP 1

/**
 * Description:
 * Read a bitplane from a bitstream into a bitplane structure.
 *
 * Remarks:
 * If the bitplane is in raw mode, fields of the bitplane structure are set to 0,
 * and bits read later from the bitplane actually come directly from the
 * bitstream.
 *
 * Inputs:
 * pState     - pointer to decoder state
 * pBitplane  - pointer to a bitplane structure, which will store the bitplane information 
 * pBitstream - pointer to a structure representing the position in a bitstream
 *
 * Outputs:
 * pBitplane  - updated with bitplane information
 * pBitstream - updated with new position in bitstream
 *
 * Return Value:
 * Standard vc1_eResult result. See enumeration for possible result codes.
 *
 */

vc1_eResult vc1DECBITPL_ReadBitplane(
    vc1DEC_sState *pState,
    vc1DEC_sBitplane * pBitplane,
    vc1DEC_sBitstream * pBitstream
);

/**
 * Description:
 * Read a bit from a bitplane at a specified co-ordinate.
 *
 * Remarks:
 * If the bitplane is in raw mode, bits are read directly from the bitstream
 * rather than the bitplane.
 *
 * Inputs:
 * pBitplane     - pointer to a bitplane structure, containing the bitplane information 
 * pBitstream    - pointer to a structure representing the position in a bitstream
 *
 * Outputs:
 * pBitstream    - updated with new position in bitstream
 *
 * Return Value:
 * - VC1DECBIT_EOF if buffer exhausted
 * - bit value     otherwise
 *
 */

WORD32 vc1DECBITPL_ReadBitplaneBit(vc1DEC_sBitplane * pBitplane, vc1DEC_sBitstream * pBitstream);

/**
 * Description:
 * Read the next bit in a bitplane.
 *
 * Remarks:
 * If the bitplane is in raw mode, bits are read directly from the bitstream
 * rather than the bitplane.
 *
 * Inputs:
 * pBitplane     - pointer to a bitplane structure, containing the bitplane information 
 * pBitstream    - pointer to a structure representing the position in a bitstream
 *
 * Outputs:
 * pBitplane     - updated with new position in bitplane
 * pBitstream    - updated with new position in bitstream
 *
 * Return Value:
 * - VC1DECBIT_EOF if buffer exhausted
 * - bit value     otherwise
 *
 */

WORD32 vc1DECBITPL_ReadBitplaneBit(vc1DEC_sBitplane * pBitplane, vc1DEC_sBitstream * pBitstream);


/**
 * Description:
 * Skip the next bit in a bitplane.
 *
 * Remarks:
 * If the bitplane is in raw mode, nothing happens. If in non-raw mode, the bitplane
 * pointer is updated.
 *
 * Inputs:
 * pBitplane     - pointer to a bitplane structure, containing the bitplane information 
 *
 * Outputs:
 * pBitplane     - updated with new position in bitplane
 */

void vc1DECBITPL_SkipBitplaneBit(vc1DEC_sBitplane * pBitplane);


#ifdef cplusplus
}
#endif
#endif
