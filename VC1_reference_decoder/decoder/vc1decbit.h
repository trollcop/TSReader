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
 * vc1decbit.h:
 * Bitstream reading headers
 *
 */

#ifndef VC1DECBIT_H
#define VC1DECBIT_H

#ifdef cplusplus
extern "C" {
#endif

/************************************************************************
 * Bitstream structures
 ***********************************************************************/

/**
 * Description:
 * Type for holding a variable length coding zone.
 *
 * Remarks:
 * This is just a pointer to an array of vc1DECBIT_sVLCCode values, but
 * the typedef is needed for the automatic table generator.
 */

typedef const vc1DEC_sVLCCode *vc1DECBIT_pVLCZone;

/************************************************************************
 * Bitstream functions
 ***********************************************************************/

/**
 * Description:
 * Get bits from a memory location.
 *
 * Remarks:
 * Reads BitCount bits from pBuffer, returning them in the least significant bits of the result.
 * This function can read up to 31 bits per call.
 *
 * Inputs:
 * pBitstream - pointer to structure representing position in bitstream
 * BitCount - number of bits to read from memory. Max 31.
 *
 * Outputs:
 * pBitstream - structure updated to new position in bitstream
 *
 * Return Value:
 * - VC1DECBIT_EOF if there are not enough bytes in the buffer
 * - the value read from the bitstream otherwise
 *
 */
WORD32 vc1DECBIT_GetBits(vc1DEC_sBitstream * pBitstream, int BitCount);

/**
 * Description:
 * Look at bits at a memory location, not updating the bitstream pointer.
 *
 * Remarks:
 * Used to examine bits for a VLC code.
 * This function can look at up to 31 bits per call.
 *
 * Inputs:
 * pBitstream - pointer to structure representing position in bitstream
 * BitCount   - number of bits to examine. Max 31.
 *
 * Outputs:
 * None.
 *
 * Return Value:
 * The value read from the bitstream.
 *
 */
WORD32 vc1DECBIT_LookBits(vc1DEC_sBitstream * pBitstream, int BitCount);

/**
 * Description:
 * Read VLC bits from a memory location.
 *
 * Remarks:
 * Reads variable length code bits from pBuffer, according to table pTable,
 * returning them in the least significant bits of the result.
 * This function can read up to 31 bits per call.
 *
 * Inputs:
 * pBitstream - pointer to structure representing position in bitstream
 * pTable     - VLC table
 *
 * Outputs:
 * pBitstream - structure updated to new position in bitstream
 *
 * Return Value:
 * - VC1DECBIT_EOF if not enough bytes in the buffer, or code unmatched
 * - value read from the bitstream otherwise
 *
 */
WORD32 vc1DECBIT_GetVLC(vc1DEC_sBitstream * pBitstream, const vc1DEC_sVLCCode * pTable);

/*************************************************************************************
 * Encapsulated interface to the bitstream structure
 *************************************************************************************/

/**
 * Description:
 * Initialises the bitstream structure.
 *
 * Remarks:
 * The initial buffer pointer will be set to the value provided.
 *
 * Inputs:
 * pBitstream   - Pointer to structure representing position in bitstream
 * pBuffer      - Initial buffer pointer
 * Length       - Bitstream length
 * Encapsulated - TRUE if the bitstream is encapsulated
 *
 * Outputs:
 * pBitstream   - bitstream information updated
 *
 * Returns:
 * Result code.
 */

vc1_eResult vc1DECBIT_InitialiseBitstream(
    vc1DEC_sBitstream * pBitstream,
    UBYTE8 *pBuffer,
    int Length,
    int Encapsulated
);

/**
 * Description:
 * Returns the current buffer pointer for the bitstream.
 *
 * Inputs:
 * pBitstream - pointer to structure representing position in bitstream
 *
 * Return Value:
 * The current bitstream buffer pointer.
 *
 */
UBYTE8 *vc1DECBIT_BufferGet(vc1DEC_sBitstream * pBitstream);

/**
 * Description:
 * Sets the current buffer pointer and length.
 *
 * Inputs:
 * pBitstream - pointer to structure representing position in bitstream
 * pBuffer    - new buffer pointer
 * Length     - new buffer
 *
 * Outputs:
 * pBitstream - bitstream information updated
 *
 * Returns:
 * VC1DECBIT_EOF if illegal encapsulation encountered
 */

WORD32 vc1DECBIT_BufferSet(
    vc1DEC_sBitstream * pBitstream,
    UBYTE8 *pBuffer,
    int Length
);

/**
 * Description:
 * Returns the number of whole or partial bytes remaining unread in the bitstream buffer.
 * This does not include bytes read into the internal bitstream state.
 * Use vc1DECBIT_BitCountGet for information on the amount of readable data left.
 *
 * Inputs:
 * pBitstream - pointer to structure representing position in bitstream
 *
 * Return Value:
 * The number of whole or partial bytes remaining in the bitstream buffer.
 *
 */
UWORD32 vc1DECBIT_ByteCountGet(vc1DEC_sBitstream * pBitstream);


/**
 * Description:
 * Returns the number of bits remaining in the bitstream buffer.
 *
 * Inputs:
 * pBitstream - pointer to structure representing position in bitstream
 *
 * Return Value:
 * The number of bits remaining in the bitstream buffer.
 *
 */
UWORD32 vc1DECBIT_BitCountGet(vc1DEC_sBitstream * pBitstream);

/**
 * Description:
 * Align bitstream input to a byte boundary.
 *
 * Remarks:
 * Aligns bitstream pointer to the next byte boundary.
 *
 * Inputs:
 * pBitstream - pointer to structure representing position in bitstream
 *
 * Outputs:
 * pBitstream - bitstream information updated
 *
 * Return Value:
 * - VC1DECBIT_EOF if the byte aligned to is past the end of the buffer
 * - 0 otherwise
 *
 */
WORD32 vc1DECBIT_AlignBit(vc1DEC_sBitstream * pBitstream);


/**
 * Description:
 * Set up the bit resolution buffer length in the bitstream structure.
 *
 * Inputs:
 * pBitstream - pointer to structure representing position in bitstream
 *
 * Outputs:
 * pBitstream - bitstream information updated
 *
 * Return Value:
 * vc1_ResultFatal - if last byte is entirely 0
 * vc1_ResultOK    - otherwise
 *
 */
vc1_eResult vc1DECBIT_TrimBuffer(vc1DEC_sBitstream * pBitstream);


/* end of file indicator */
#define VC1DECBIT_EOF ((WORD32)(-1))

#ifdef cplusplus
}
#endif
#endif
