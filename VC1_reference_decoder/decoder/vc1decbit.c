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
 * vc1decbit.c:
 * Bitstream reading functions
 *
 */

#include "vc1types.h"
#include "vc1dec.h"
#include "vc1gentab.h"
#include "vc1decbit.h"
#include "vc1debug.h"

/*
 * Description:
 * Read bytes to fill up the input bitstream buffer
 *
 * Remarks:
 * Reads as many bytes as possible until the bistream
 * buffer is full or the end of the bitstream is reached.
 * Ensures there are at least 32 unused bits (or as many
 * bits as there are left) in the holding buffer.
 *
 * Inputs:
 * pBitstream   - pointer to bitstream state
 *
 * Outputs:
 * pBitstream   - state updated
 *
 * Returns:
 * VC1DECBIT_EOF if an illegal encapulation encountered
 */

static WORD32 vc1DECBIT_ReadBytes(vc1DEC_sBitstream * pBitstream)
{
    UBYTE8 *pBitBuffer = pBitstream->pBitBuffer;
    UBYTE8 *pEndByte   = pBitstream->pEndByte;
    int BitsValid      = pBitstream->BitsValid;
    int BitsUsed       = pBitstream->BitsUsed;
    int ZeroRun        = pBitstream->ZeroRun;
    int Encapsulated   = pBitstream->EncapsulatedIDU;
    UWORD32 Buffer0    = pBitstream->Buffer0;
    UWORD32 Buffer1    = pBitstream->Buffer1;
    int BufferByte;

    ASSERT(BitsValid >= BitsUsed);

    if (BitsUsed >= 32)
    {
        pBitstream->BitsUsed = (UBYTE8)(BitsUsed - 32);
        Buffer0 = Buffer1;
        Buffer1 = 0;
        BitsValid -= 32;
    }

    while (BitsValid <= 56 && pBitBuffer < pEndByte)
    {
        /* Try reading another byte */
        BufferByte = *pBitBuffer++;

        if (Encapsulated)
        {
            /* Annex E.3 - Extraction of RIDU from EIDU */
            if(0 == BufferByte)
            {
                ZeroRun++;
            }
            else
            {
                if(2 == ZeroRun)
                {
                    switch(BufferByte)
                    {
                        case 2:  /* Error a) - pointers not updated */
                        {
                            WARN("vc1DECBIT_ReadBytes: Error case a) - BufferByte = 0x%02x\n",
                                 BufferByte);
                            return(VC1DECBIT_EOF);
                        }
                        case 3:
                        {
                            DEBUG1(vc1DEBUG_BIT, "Escape byte = 0x%02x\n", BufferByte);

                            if (pBitBuffer < pEndByte)
                            {
                                BufferByte = *pBitBuffer++;
                                pBitstream->BitCounter += 8;
                            }
                            else
                            {
                                WARN("vc1DECBIT_ReadBytes: Error - incomplete encapulation\n");
                                return(VC1DECBIT_EOF);
                            }

                            if(4 <= BufferByte)
                            {
                                /* Error c) - pointers not updated */
                                WARN("vc1DECBIT_ReadBits: Error case c) - BufferByte = 0x%02x\n",
                                     BufferByte);
                                return(VC1DECBIT_EOF);
                            }
                            break;
                        }
                    }
                }
                else if(3 <= ZeroRun)
                {
                    if(1 != BufferByte)
                    {
                        /* Error b) - pointers not updated */
                        WARN("vc1DECBIT_ReadBits: Error case b) - BufferByte = 0x%02x\n",
                             BufferByte);
                        return(VC1DECBIT_EOF);
                    }
                }
                ZeroRun = (BufferByte == 0);
            }
        }

        DEBUG2(vc1DEBUG_BIT, "BufferByte = 0x%02x (0x%08x)\n", BufferByte, (int)pBitBuffer-1);

        /* Work out the number of valid bits read */
        if (pBitBuffer == pEndByte)
        {
            /* BufferByte is the last valid byte */
            int N = pBitstream->EndBitsValid;

            BufferByte >>= (8-N);  /* 0 to 7 */
            BitsValid += N;
        }
        else
        {
            BitsValid += 8;
        }

        /* Insert the bits into the holding buffer */
        if (BitsValid <= 32)
        {
            Buffer0 |= (BufferByte<<(32-BitsValid));
        }
        else
        {
            Buffer1 |= (BufferByte<<(64-BitsValid));
        }
    }
    
    pBitstream->pBitBuffer = pBitBuffer;
    pBitstream->Buffer0    = Buffer0;
    pBitstream->Buffer1    = Buffer1;
    pBitstream->BitsValid  = (UBYTE8)BitsValid;
    pBitstream->ZeroRun    = ZeroRun;

    return 0;
}


/*
 * Description:
 * Read bits from a memory location
 *
 * Remarks:
 * Reads tBitCount bits from pBuffer, returning them in the least significant bits of the result.
 * Can read up to 32 bits per call
 *
 * Inputs:
 * pBitstream - pointer to structure representing position in bitstream
 * tBitCount - number of bits to read from memory. Max 31.
 * UpdatePointer - update the bitstream struct to point to the bit after those read
 *
 * Outputs:
 * pBitstream - structure updated to new position in bitstream, if UpdatePointer == TRUE
 *
 * Return Value:
 * VC1DECBIT_EOF if there's not enough bytes in the buffer
 * the value read from the bitstream, otherwise
 *
 */

static WORD32 vc1DECBIT_ReadBits(
    vc1DEC_sBitstream * pBitstream,
    int BitCount,
    int UpdatePointer
)
{
    UWORD32 Buffer = pBitstream->Buffer0;
    int BitsUsed   = pBitstream->BitsUsed;

    ASSERT(BitCount!=0 && BitCount<32);

#if 0
    DEBUG5(vc1DEBUG_BIT, "Buf = %08x %08x Used = %d Valid = %d: Bits = %d\n",
        pBitstream->Buffer0, pBitstream->Buffer1,
        pBitstream->BitsUsed, pBitstream->BitsValid, BitCount);
#endif
    
    /* Get next bitcount bits */
    Buffer = (pBitstream->Buffer0 << BitsUsed);
    if (BitsUsed + BitCount > 32)
    {
        Buffer |= (pBitstream->Buffer1 >> (32-BitsUsed));
    }
    Buffer = Buffer >> (32-BitCount);

    /* Check we haven't gone past the end of the array */
    if (UpdatePointer)
    {
        BitsUsed += BitCount;
        if (BitsUsed > pBitstream->BitsValid)
        {
            return VC1DECBIT_EOF;
        }
        pBitstream->BitCounter += BitCount;
        pBitstream->BitsUsed = (UBYTE8)BitsUsed;

        /* Refill buffer if empty */
        if (BitsUsed >= 32)
        {
            if (vc1DECBIT_ReadBytes(pBitstream)==VC1DECBIT_EOF)
            {
                return VC1DECBIT_EOF;
            }
        }
    }

    DEBUG3(vc1DEBUG_BIT, "FLC %s %d Bits: %d\n", 
        UpdatePointer ? "Read" : "Look", BitCount, Buffer);

    return Buffer;
}

/*
 * Description:
 * Get bits from a memory location
 *
 * Remarks:
 * Reads tBitCount bits from pBuffer, returning them in the least significant bits of the result.
 * Can read up to 32 bits per call
 *
 * Inputs:
 * pBitstream - pointer to structure representing position in bitstream
 * tBitCount - number of bits to read from memory. Max 31.
 *
 * Outputs:
 * pBitstream - structure updated to new position in bitstream
 *
 * Return Value:
 * VC1DECBIT_EOF if there's not enough bytes in the buffer
 * the value read from the bitstream, otherwise
 *
 */

WORD32 vc1DECBIT_GetBits(vc1DEC_sBitstream * pBitstream, int BitCount)
{
    return(vc1DECBIT_ReadBits(pBitstream, BitCount, TRUE));
}


/*
 * Description:
 * Look at bits at a memory location, not updating the bitstream pointer
 *
 * Remarks:
 * Used to examine bits for a VLC code.
 * Can look at up to 32 bits per call.
 *
 * Inputs:
 * pBitstream - pointer to structure representing position in bitstream
 * tBitCount - number of bits to examine. Max 32.
 *
 * Outputs:
 * None.
 *
 * Return Value:
 * the value read from the bitstream
 *
 */


WORD32 vc1DECBIT_LookBits(vc1DEC_sBitstream * pBitstream, int BitCount)
{
    return(vc1DECBIT_ReadBits(pBitstream, BitCount, FALSE));
}


/*
 * Description:
 * Align bitstream input to byte boundary
 *
 * Remarks:
 * Aligns bitstream pointer to next byte boundary
 *
 * Inputs:
 * pBitstream - pointer to structure representing position in bitstream
 *
 * Outputs:
 * pBitstream - bitstream information updated
 *
 * Return Value:
 * VC1DECBIT_EOF, if byte aligned to is past end of buffer
 * 0, otherwise
 *
 */

WORD32 vc1DECBIT_AlignBit(vc1DEC_sBitstream * pBitstream)
{
    int BitsUsed = pBitstream->BitsUsed;

    DEBUG2(vc1DEBUG_BIT, "Byte Align: Bits used = %d (&7 = %d)\n", BitsUsed, BitsUsed&7);

    /* Byte align */
    BitsUsed = (BitsUsed + 7) &~ 7;
    pBitstream->BitsUsed = (UBYTE8)BitsUsed;

    /* Check for end of file */
    if (BitsUsed > pBitstream->BitsValid)
    {
        return(VC1DECBIT_EOF);
    }

    if (BitsUsed >= 32)
    {
        /* Fill up buffer */
        return vc1DECBIT_ReadBytes(pBitstream);
    }
    return(0);
}


/*
 * Description:
 * Read VLC bits from a memory location
 *
 * Remarks:
 * Reads variable length code bits from pBuffer, according to table pTable,
 * returning them in the least significant bits of the result.
 * Can read up to 31 bits per call
 *
 * Inputs:
 * pBitstream - pointer to structure representing position in bitstream
 * pTable - VLC table
 *
 * Outputs:
 * pBitstream - structure updated to new position in bitstream
 *
 * Return Value:
 * VC1DECBIT_EOF, if not enough bytes in the buffer, or code unmatched
 * value read from the bitstream, otherwise
 *
 */
WORD32 vc1DECBIT_GetVLC(vc1DEC_sBitstream * pBitstream, const vc1DEC_sVLCCode * pTable)
{
    UBYTE8  MaxCodeLength;
    UWORD32 TableLength;
    UWORD32 Value;
    UWORD32 Count;

    ASSERT(0 == pTable[0].Bits);
    ASSERT(1 <= pTable[0].Length);
    ASSERT(1 <= pTable[0].Value);

    MaxCodeLength = (UBYTE8)pTable[0].Value;
    TableLength = pTable[0].Length;

    ASSERT(31 >= MaxCodeLength);

    Value = vc1DECBIT_LookBits(pBitstream, MaxCodeLength);
    if(VC1DECBIT_EOF == Value)
    {
        WARN("vc1DECBIT_GetVLC: Buffer exhausted trying to look at %d bits\n", MaxCodeLength);
        return(Value);
    }

    for(Count = 1; Count < TableLength + 1; Count++)
    {
        if(pTable[Count].Bits == (Value >> (MaxCodeLength - pTable[Count].Length)))
        {
            Value = vc1DECBIT_GetBits(pBitstream, (UBYTE8)pTable[Count].Length);
            if(VC1DECBIT_EOF == Value)
            {
                WARN("vc1DECBIT_GetVLC: Buffer exhausted trying to get %d bits\n",
                     pTable[Count].Length);
                return(Value);
            }

            DEBUG1(vc1DEBUG_BIT, "VLC Code : %d\n", pTable[Count].Bits);
            DEBUG1(vc1DEBUG_BIT, "VLC Value: %d\n", pTable[Count].Value);

            return(pTable[Count].Value);
        }
    }

    WARN("vc1DECBIT_GetVLC: Code not found in VLC table\n");

    /* Code not found */
    return(VC1DECBIT_EOF);
}

/*
 * Description:
 * Initialises the bitstream structure
 *
 * Remarks:
 * The initial buffer pointer will be set to the value provided.
 *
 * Inputs:
 * pBitstream - Pointer to structure representing position in bitstream
 * pBuffer    - Initial buffer pointer
 * Length     - Bitstream length
 * Encapsulated - TRUE if the bitstream is encapsulated
 *
 * Outputs:
 * pBitstream - bitstream information updated
 *
 * Returns:
 * Result code
 */

vc1_eResult vc1DECBIT_InitialiseBitstream(
    vc1DEC_sBitstream * pBitstream,
    UBYTE8 *pBuffer,
    int Length,
    int Encapsulated
)
{
    DEBUG2(vc1DEBUG_BIT, "Initialise Bitstream Start=0x%08x Length=%d\n",
        (int)pBuffer, Length);

    pBitstream->pBitBuffer  = pBuffer;
    pBitstream->Buffer0     = 0;
    pBitstream->Buffer1     = 0;
    pBitstream->BitsUsed    = 0;
    pBitstream->BitsValid   = 0;

    pBitstream->pEndByte    = pBuffer + Length;
    pBitstream->EndBitsValid = 8;

    pBitstream->EncapsulatedIDU = (FLAG)Encapsulated;
    pBitstream->ZeroRun     = 0;
    pBitstream->BitCounter  = 0;

    if (vc1DECBIT_ReadBytes(pBitstream)==VC1DECBIT_EOF)
    {
        return vc1_ResultBufferExhausted;
    }

    return vc1_ResultOK;
}

/*
 * Description:
 * Returns the current buffer pointer for the bitstream
 *
 * Inputs:
 * pBitstream - pointer to structure representing position in bitstream
 *
 * Return Value:
 * The current bitstream buffer pointer
 *
 */
UBYTE8 *vc1DECBIT_BufferGet(vc1DEC_sBitstream * pBitstream)
{
    return pBitstream->pBitBuffer;
}

/*
 * Description:
 * Sets the current buffer pointer and length
 *
 * Inputs:
 * pBitstream - pointer to structure representing position in bitstream
 * pBuffer   - new buffer pointer
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
)
{
    DEBUG2(vc1DEBUG_BIT, "Setting buffer Start=0x%08x Length=%d\n",
        (int)pBuffer, Length);

    pBitstream->pBitBuffer   = pBuffer;
    pBitstream->pEndByte     = pBitstream->pBitBuffer + Length;
    pBitstream->EndBitsValid = 8;

    return vc1DECBIT_ReadBytes(pBitstream);
}

/*
 * Description:
 * Returns the number of whole or partial bytes remaining unread in the bitstream buffer.
 * Does not include bytes read into the internal bitstream state.
 * Use vc1DECBIT_BitCountGet for information on the amount of readable data left.
 *
 * Inputs:
 * pBitstream - pointer to structure representing position in bitstream
 *
 * Return Value:
 * The number of whole or partial bytes remaining in the bitstream buffer
 *
 */
UWORD32 vc1DECBIT_ByteCountGet(vc1DEC_sBitstream * pBitstream)
{
    return(pBitstream->pEndByte - pBitstream->pBitBuffer);
}

/*
 * Description:
 * Returns the number of bits remaining in the bitstream buffer
 *
 * Inputs:
 * pBitstream - pointer to structure representing position in bitstream
 *
 * Return Value:
 * The number of bits remaining in the bitstream buffer
 *
 */
UWORD32 vc1DECBIT_BitCountGet(vc1DEC_sBitstream * pBitstream)
{
    UBYTE8 *pBitBuffer = pBitstream->pBitBuffer;
    UBYTE8 *pEndByte   = pBitstream->pEndByte;
    int Count;

#if 0
    DEBUG7(vc1DEBUG_BIT, "%08x %08x %d %d : Curr=%08x End=%08x Last=%d\n",
        pBitstream->Buffer0, pBitstream->Buffer1,
        pBitstream->BitsUsed, pBitstream->BitsValid,
        pBitstream->pBitBuffer, pBitstream->pEndByte,
        pBitstream->EndBitsValid);
#endif
    
    /* Start with number of bits left in the holding buffer */
    Count = pBitstream->BitsValid - pBitstream->BitsUsed;

    /* Add on unread bytes if not at buffer end */
    if (pBitBuffer < pEndByte)
    {
        Count += 8*(pEndByte - pBitBuffer - 1);
        Count += pBitstream->EndBitsValid;
    }

    DEBUG1(vc1DEBUG_BIT, "Bits remaining = %d\n", Count);

    return Count;
}

/*
 * Description:
 * Set up the bit resolution buffer length in the bitstream structure
 *
 * Inputs:
 * pBitstream - pointer to structure representing position in bitstream
 *
 * Outputs:
 * pBitstream - bitstream information updated
 *
 * Return Value:
 * vc1_ResultFatal - if last byte is entirely 0,
 * vc1_ResultOK    - otherwise
 *
 */
vc1_eResult vc1DECBIT_TrimBuffer(vc1DEC_sBitstream * pBitstream)
{
    /* First check for null RBDU */
    if (pBitstream->BitsValid > 0)
    {
        int I;
        int LastByte = *(pBitstream->pEndByte - 1);

        DEBUG2(vc1DEBUG_BIT, "Trim byte %02x, (at %08x)\n",
            LastByte, (int)pBitstream->pEndByte);

        if (0 == LastByte)
        {
            FATAL("Trim byte is zero\n");
            return(vc1_ResultFatal);
        }

        /* find last set bit */
        for(I = 0; I < 7; I++)
        {
            if(1 == ((LastByte >> I) & 1))
            {
                break;
            }
        }

        if (pBitstream->pEndByte <= pBitstream->pBitBuffer)
        {
            pBitstream->BitsValid = (UBYTE8)(pBitstream->BitsValid - (I + 1));
        }

        if(7 == I)
        {
            pBitstream->pEndByte--;
            pBitstream->EndBitsValid = 8;
        }
        else
        {
            pBitstream->EndBitsValid = (UBYTE8)(7 - I);
        }
    }

    DEBUG2(vc1DEBUG_BIT, "End byte = %08x, bits valid = %d\n",
        pBitstream->pEndByte, pBitstream->EndBitsValid);

    return(vc1_ResultOK);
}


