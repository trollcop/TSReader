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
 * vc1decbitpl.c:
 * Bitplane decoding functions
 *
 *  
 */


#include "vc1types.h"
#include "vc1debug.h"
#include "vc1dec.h"
#include "vc1decbit.h"
#include "vc1decbitpl.h"
#include "vc1decbitpltab.h"

static const vc1DEC_sVLCCode vc1DECBITPL_IMODE_Table[8] =
{
    {0, 7, 4},
    {2, 2, vc1_BitplaneCodingModeNorm2},
    {3, 2, vc1_BitplaneCodingModeNorm6},
    {2, 3, vc1_BitplaneCodingModeRowskip},
    {3, 3, vc1_BitplaneCodingModeColskip},
    {1, 3, vc1_BitplaneCodingModeDiff2},
    {1, 4, vc1_BitplaneCodingModeDiff6},
    {0, 4, vc1_BitplaneCodingModeRaw}
};

static const vc1DEC_sVLCCode vc1DECBITPL_Norm2_Table[5] =
{
    {0, 4, 3},

    {0, 1, 0},
    {3, 2, 3},
    {4, 3, 2},
    {5, 3, 1},
};


/*
 * Description:
 * Decode a Norm-2 encoded bitplane. 
 *
 * Remarks:
 * None.
 *
 * Inputs:
 * pBitplane    - pointer to initialised area into which can be written the bitplane
 * pBitstream   - pointer to struct representing the current bitstream
 * WidthMB      - width of the current picture in macroblocks, which is the width of the bitplane
 * HeightMB     - height of the current picture in macroblocks, which is the height of the bitplane
 * Invert       - the invert status, read from the bitstream
 *
 * Outputs:
 * pBitplane    - updated with decoded bitplane.
 * pBitstream   - updated with new bitstream position.
 *
 * Return Value:
 * Standard vc1_eResult result. See enumeration for possible result codes.
 * 
 */

static vc1_eResult vc1DECBITPL_DecodeNorm2Bits(vc1DEC_sBitplane * pBitplane,
                                               vc1DEC_sBitstream * pBitstream,
                                               int WidthMB,
                                               int HeightMB,
                                               int Invert)
{
    WORD32 Value;
    int I, Count, Offset = 0;

    COVERAGE("8.7.3.2");
    Count = WidthMB * HeightMB;
    DEBUG1(vc1DEBUG_BITPL, "Invert = %d\n", Invert);

    /* if odd number of bits, handle first symbol raw */
    if(1 == ((WidthMB * HeightMB) & 1))
    {
        Value = vc1DECBIT_GetBits(pBitstream, 1);
        if(VC1DECBIT_EOF == Value)
        {
            return(vc1_ResultBufferExhausted);
        }
        pBitplane->pBitplane[0] = (FLAG)(Invert ^ Value);

        Count--;
        Offset = 1;
    }

    for(I = 0; I < Count / 2; I++)
    {
        Value = vc1DECBIT_GetVLC(pBitstream, vc1DECBITPL_Norm2_Table);
        if(VC1DECBIT_EOF == Value)
        {
            return(vc1_ResultBufferExhausted);
        }
        pBitplane->pBitplane[I * 2 + Offset]     = (FLAG)(Invert ^ (Value >> 1));
        pBitplane->pBitplane[I * 2 + 1 + Offset] = (FLAG)(Invert ^ (Value & 1));
    }

    DEBUG0(vc1DEBUG_BITPL, "Norm2 bitplane:\n");
    DEBUGRECT8(vc1DEBUG_BITPL, pBitplane->pBitplane, WidthMB, HeightMB, WidthMB);

    return(vc1_ResultOK);
}


/*
 * Description:
 * Decode a Norm-6 encoded bitplane. 
 *
 * Remarks:
 *
 * Inputs:
 * pBitplane    - pointer to initialised area into which can be written the bitplane
 * pBitstream   - pointer to struct representing the current bitstream
 * WidthMB      - width of the current picture in macroblocks, which is the width of the bitplane
 * HeightMB     - height of the current picture in macroblocks, which is the height of the bitplane
 * Invert       - the invert status, read from the bitstream
 *
 * Outputs:
 * pBitplane    - updated with decoded bitplane.
 * pBitstream   - updated with new bitstream position.
 *
 * Return Value:
 * Standard vc1_eResult result. See enumeration for possible result codes.
 * 
 */

static vc1_eResult vc1DECBITPL_DecodeNorm6Bits(vc1DEC_sBitplane * pBitplane,
                                               vc1DEC_sBitstream * pBitstream,
                                               int WidthMB,
                                               int HeightMB,
                                               int Invert)
{
    UBYTE8 *pBit;
    WORD32 Value;
    int I, J, K, WidthInTiles, HeightInTiles;
    int ResidualX, ResidualY;

    COVERAGE("8.7.3.4");

    if((0 == HeightMB % 3) && (0 != WidthMB % 3))
    {
        /* use 2x3 tiles */

        WidthInTiles  = WidthMB / 2;
        HeightInTiles = HeightMB / 3;

        for(J = 0; J < HeightInTiles; J++)
        {
            pBit = &pBitplane->pBitplane[J * 3 * WidthMB];
                                   /* set pBit to point to next row of tiles */
            pBit += WidthMB & 1;   /* adjust for colskip encoded residual */

            for(I = 0; I < WidthInTiles; I++)
            {
                Value = vc1DECBIT_GetVLC(pBitstream, vc1DEC_Code_3x2_2x3_tiles);
                if(VC1DECBIT_EOF == Value)
                {
                    return(vc1_ResultBufferExhausted);
                }

                for(K = 0; K < 6; K++)
                {
                    if((2 == K) || (4 == K))            /* skip to next bitplane row
                                                         * at the end of each tile row */
                    {
                        pBit += WidthMB;
                        pBit -= 2;
                    }

                    *pBit++ = (UBYTE8)((0 != (Value & (1 << K))) ? (1 - Invert) : Invert);
                }

                pBit -= (2 * WidthMB);                  /* adjust pBit to point to next tile */
            }
        }

        ResidualX = WidthMB & 1;
        ResidualY = 0;          
    }
    else
    {
        /* use 3x2 tiles */

        WidthInTiles  = WidthMB / 3;
        HeightInTiles = HeightMB / 2;

        for(J = 0; J < HeightInTiles; J++)
        {
            DEBUG1(vc1DEBUG_BITPL, "3x2 Tile row %d\n", J);
            pBit = &pBitplane->pBitplane[J * 2 * WidthMB]; /* set pBit to point to next tile row */
            pBit += WidthMB % 3;                           /* adjust for colskip encoded residual */
            pBit += (HeightMB & 1) * WidthMB;              /* adjust for rowskip encoded residual */

            for(I = 0; I < WidthInTiles; I++)
            {
                Value = vc1DECBIT_GetVLC(pBitstream, vc1DEC_Code_3x2_2x3_tiles);
                if(VC1DECBIT_EOF == Value)
                {
                    return(vc1_ResultBufferExhausted);
                }

                for(K = 0; K < 6; K++)
                {
                    if(3 == K)
                    {
                        /* skip to next bitplane row at the end of each tile row */
                        pBit += WidthMB;
                        pBit -= 3;
                    }

                    *pBit++ = (UBYTE8)((0 != (Value & (1 << K))) ? (1 - Invert) : Invert);
                }

                pBit -= WidthMB;                            /* adjust pBit to point to next tile */
            }
        }

        ResidualX = WidthMB % 3;
        ResidualY = HeightMB & 1;
    }

    /* read the colskip encoded column, if necessary */
    for(I = 0; I < ResidualX; I++)
    {
        WORD32 ColSkip = vc1DECBIT_GetBits(pBitstream, VC1DECBITPL_BITS_BITPLANE_COLSKIP);
        if(VC1DECBIT_EOF == ColSkip)
        {
            return(vc1_ResultBufferExhausted);
        }

        DEBUG1(vc1DEBUG_BITPL, "Colskip residual %d\n", I);

        for(J = 0; J < HeightMB; J++)
        {
            Value = 0;

            if(TRUE == ColSkip)
            {
                Value = vc1DECBIT_GetBits(pBitstream, 1);
                if(VC1DECBIT_EOF == Value)
                {
                    return(vc1_ResultBufferExhausted);
                }
            }

            pBitplane->pBitplane[I + WidthMB * J] = (FLAG)(Invert ^ Value);
        }
    }

    /* read the rowskip encoded row, if necessary */
    for(J = 0; J < ResidualY; J++)
    {
        WORD32 RowSkip = vc1DECBIT_GetBits(pBitstream, VC1DECBITPL_BITS_BITPLANE_ROWSKIP);
        if(VC1DECBIT_EOF == RowSkip)
        {
            return(vc1_ResultBufferExhausted);
        }

        DEBUG1(vc1DEBUG_BITPL, "Rowskip residual %d\n", J);

        for(I = ResidualX; I < WidthMB; I++)
        {
            Value = 0;

            if(TRUE == RowSkip)
            {
                Value = vc1DECBIT_GetBits(pBitstream, 1);
                if(VC1DECBIT_EOF == Value)
                {
                    return(vc1_ResultBufferExhausted);
                }
            }
        
            pBitplane->pBitplane[I] = (FLAG)(Invert ^ Value);
        }
    }

    DEBUG0(vc1DEBUG_BITPL, "Norm-6 bitplane:\n");
    DEBUGRECT8(vc1DEBUG_BITPL, pBitplane->pBitplane, WidthMB, HeightMB, WidthMB);
    return(vc1_ResultOK);
}


/*
 * Description:
 * Decode a Rowskip encoded bitplane. 
 *
 * Remarks:
 * None.
 *
 * Inputs:
 * pBitplane    - pointer to initialised area into which can be written the bitplane
 * pBitstream   - pointer to struct representing the current bitstream
 * WidthMB      - width of the current picture in macroblocks, which is the width of the bitplane
 * HeightMB     - height of the current picture in macroblocks, which is the height of the bitplane
 * Invert       - the invert status, read from the bitstream
 *
 * Outputs:
 * pBitplane    - updated with decoded bitplane.
 * pBitstream   - updated with new bitstream position.
 *
 * Return Value:
 * Standard vc1_eResult result. See enumeration for possible result codes.
 * 
 */

static vc1_eResult vc1DECBITPL_DecodeRowskipBits(vc1DEC_sBitplane * pBitplane,
                                                 vc1DEC_sBitstream * pBitstream,
                                                 int WidthMB,
                                                 int HeightMB,
                                                 int Invert)
{
    WORD32 Value, RowSkip, ReadBits = 0;
    int I, J;

    COVERAGE("8.7.3.6");
    for(J = 0; J < HeightMB; J++)
    {
        RowSkip = vc1DECBIT_GetBits(pBitstream, VC1DECBITPL_BITS_BITPLANE_ROWSKIP);
        if(VC1DECBIT_EOF == RowSkip)
        {
            return(vc1_ResultBufferExhausted);
        }
        ReadBits++;

        for(I = 0; I < WidthMB; I++)
        {
            Value = 0;

            if(TRUE == RowSkip)
            {
                Value = vc1DECBIT_GetBits(pBitstream, 1);
                if(VC1DECBIT_EOF == Value)
                {
                    return(vc1_ResultBufferExhausted);
                }
                ReadBits++;
            }
            
            pBitplane->pBitplane[I + WidthMB * J] = (FLAG)(Invert ^ Value);
        }
    }

    DEBUG0(vc1DEBUG_BITPL, "Rowskip bitplane:\n");
    DEBUGRECT8(vc1DEBUG_BITPL, pBitplane->pBitplane, WidthMB, HeightMB, WidthMB);
    DEBUG1(vc1DEBUG_BITPL, "Bitplane bits read = %d\n", ReadBits);

    return(vc1_ResultOK);   
}


/*
 * Description:
 * Decode a Colskip encoded bitplane. 
 *
 * Remarks:
 * None.
 *
 * Inputs:
 * pBitplane    - pointer to initialised area into which can be written the bitplane
 * pBitstream   - pointer to struct representing the current bitstream
 * WidthMB      - width of the current picture in macroblocks, which is the width of the bitplane
 * HeightMB     - height of the current picture in macroblocks, which is the height of the bitplane
 * Invert       - the invert status, read from the bitstream
 *
 * Outputs:
 * pBitplane    - updated with decoded bitplane.
 * pBitstream   - updated with new bitstream position.
 *
 * Return Value:
 * Standard vc1_eResult result. See enumeration for possible result codes.
 * 
 */

static vc1_eResult vc1DECBITPL_DecodeColskipBits(vc1DEC_sBitplane * pBitplane,
                                                 vc1DEC_sBitstream * pBitstream,
                                                 int WidthMB,
                                                 int HeightMB,
                                                 int Invert)
{
    WORD32 Value, ColSkip;
    int I, J;

    COVERAGE("8.7.3.7");
    for(I = 0; I < WidthMB; I++)
    {
        ColSkip = vc1DECBIT_GetBits(pBitstream, VC1DECBITPL_BITS_BITPLANE_COLSKIP);
        if(VC1DECBIT_EOF == ColSkip)
        {
            return(vc1_ResultBufferExhausted);
        }

        for(J = 0; J < HeightMB; J++)
        {
            Value = 0;

            if(TRUE == ColSkip)
            {
                Value = vc1DECBIT_GetBits(pBitstream, 1);
                if(VC1DECBIT_EOF == Value)
                {
                    return(vc1_ResultBufferExhausted);
                }
            }

            pBitplane->pBitplane[I + WidthMB * J] = (FLAG)(Invert ^ Value);
        }
    }

    DEBUG0(vc1DEBUG_BITPL, "Colskip bitplane:\n");
    DEBUGRECT8(vc1DEBUG_BITPL, pBitplane->pBitplane, WidthMB, HeightMB, WidthMB);
    return(vc1_ResultOK);
}


/*
 * Description:
 * Bitplane Diff operation
 *
 * Remarks:
 * As described in section 8.7.
 *
 * Inputs:
 * pBits        - pointer to WidthMB x HeightMB array of flags
 * WidthMB      - width of the current picture in macroblocks, which is the width of the bitplane
 * HeightMB     - height of the current picture in macroblocks, which is the height of the bitplane
 * Invert       - the invert status, read from the bitstream
 *
 * Outputs:
 * pBits        - updated by diff operation
 *
 * Return Value:
 * Standard vc1_eResult result. See enumeration for possible result codes.
 * 
 */

static void vc1DECBITPL_BitplaneDiff(FLAG * pBits, int WidthMB, int HeightMB, int Invert)
{
    int I, J;
    int Pred = 0;

    COVERAGE("8.7.3.8");
    
    for(J = 0; J < HeightMB; J++)
    {
        for(I = 0; I < WidthMB; I++)
        {
            if((0 == I) && (0 == J))
            {
                Pred = Invert;
            }
            else if(0 == I)
            {
                Pred = pBits[0 + WidthMB * (J - 1)];
            }
            else if((0 < J) && (pBits[I + WidthMB * (J - 1)] != pBits[(I - 1) + WidthMB * J]))
            {
                Pred = Invert;
            }
            else
            {
                Pred = pBits[I - 1 + WidthMB * J];
            }

            pBits[I + WidthMB * J] = (FLAG)(pBits[I + WidthMB * J] ^ Pred);
        }
    }
}


/*
 * Description:
 * Decode a Diff-2 encoded bitplane. 
 *
 * Remarks:
 * Firstly, decodes using the Norm-2 function, then applies the Diff operation.
 *
 * Inputs:
 * pBitplane    - pointer to initialised area into which can be written the bitplane
 * pBitstream   - pointer to struct representing the current bitstream
 * WidthMB      - width of the current picture in macroblocks, which is the width of the bitplane
 * HeightMB     - height of the current picture in macroblocks, which is the height of the bitplane
 * Invert       - the invert status, read from the bitstream
 *
 * Outputs:
 * pBitplane    - updated with decoded bitplane.
 * pBitstream   - updated with new bitstream position.
 *
 * Return Value:
 * Standard vc1_eResult result. See enumeration for possible result codes.
 * 
 */

static vc1_eResult vc1DECBITPL_DecodeDiff2Bits(vc1DEC_sBitplane * pBitplane,
                                               vc1DEC_sBitstream * pBitstream,
                                               int WidthMB,
                                               int HeightMB,
                                               int Invert)
{
    FLAG *pBits = pBitplane->pBitplane;
    vc1_eResult eResult;

    COVERAGE("8.7.3.3");
    eResult = vc1DECBITPL_DecodeNorm2Bits(pBitplane, pBitstream, WidthMB, HeightMB, 0);
    if(vc1_ResultOK != eResult)
    {
        return(eResult);
    }

    /* apply difference function */
    vc1DECBITPL_BitplaneDiff(pBits, WidthMB, HeightMB, Invert);

    DEBUG0(vc1DEBUG_BITPL, "Diff2 bitplane:\n");
    DEBUGRECT8(vc1DEBUG_BITPL, pBitplane->pBitplane, WidthMB, HeightMB, WidthMB);

    return(vc1_ResultOK);
}


/*
 * Description:
 * Decode a Diff-6 encoded bitplane. 
 *
 * Remarks:
 * Firstly, decodes using the Norm-6 function, then applies the Diff operation.
 *
 * Inputs:
 * pBitplane    - pointer to initialised area into which can be written the bitplane
 * pBitstream   - pointer to struct representing the current bitstream
 * WidthMB      - width of the current picture in macroblocks, which is the width of the bitplane
 * HeightMB     - height of the current picture in macroblocks, which is the height of the bitplane
 * Invert       - the invert status, read from the bitstream
 *
 * Outputs:
 * pBitplane    - updated with decoded bitplane.
 * pBitstream   - updated with new bitstream position.
 *
 * Return Value:
 * Standard vc1_eResult result. See enumeration for possible result codes.
 * 
 */

static vc1_eResult vc1DECBITPL_DecodeDiff6Bits(vc1DEC_sBitplane * pBitplane,
                                               vc1DEC_sBitstream * pBitstream,
                                               int WidthMB,
                                               int HeightMB,
                                               int Invert)
{
    FLAG *pBits = pBitplane->pBitplane;
    vc1_eResult eResult;

    COVERAGE("8.7.3.5");
    eResult = vc1DECBITPL_DecodeNorm6Bits(pBitplane, pBitstream, WidthMB, HeightMB, 0);
    if(vc1_ResultOK != eResult)
    {
        return(eResult);
    }

    /* apply difference function */
    vc1DECBITPL_BitplaneDiff(pBits, WidthMB, HeightMB, Invert);

    DEBUG0(vc1DEBUG_BITPL, "Diff6 bitplane:\n");
    DEBUGRECT8(vc1DEBUG_BITPL, pBitplane->pBitplane, WidthMB, HeightMB, WidthMB);

    return(vc1_ResultOK);
}


/*
 * Description:
 * Decode a Raw encoded bitplane. 
 *
 * Remarks:
 * Sets a flag in the bitplane structure to indicate that bits are raw encoded
 * with each macroblock.
 *
 * Inputs:
 * pBitplane    - pointer to initialised area into which can be written the bitplane
 *
 * Outputs:
 * pBitplane    - updated with decoded bitplane.
 *
 * Return Value:
 * Standard vc1_eResult result. See enumeration for possible result codes.
 * 
 */

static vc1_eResult vc1DECBITPL_DecodeRawBits(vc1DEC_sBitplane * pBitplane)
{
    COVERAGE("8.7.3.1");
    pBitplane->RawMode = TRUE;

    return(vc1_ResultOK);
}


/*
 * Description:
 * Read a bitplane from a bitstream into a bitplane structure
 *
 * Remarks:
 * If the bitplane is raw mode, fields of the bitplane structure are set to 0,
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
 * Standard vc1_eResult result. See enumeration for possible result codes
 *
 */

vc1_eResult vc1DECBITPL_ReadBitplane(
    vc1DEC_sState *pState,
    vc1DEC_sBitplane * pBitplane,
    vc1DEC_sBitstream * pBitstream
)
{
    WORD32 Invert;
    vc1_eBitplaneCodingMode eIMode;
    vc1_eResult eResult = vc1_ResultFatal;
    int WidthMB  = pState->sPosition.WidthMB;
    int HeightMB = pState->sPosition.HeightMB;

    Invert = vc1DECBIT_GetBits(pBitstream, VC1DECBITPL_BITS_BITPLANE_INVERT);
    if(VC1DECBIT_EOF == Invert)
    {
        return(vc1_ResultBufferExhausted);
    }
    COVERAGE("7.2.1");
    COVERAGE("8.7.1");
    DEBUG1(vc1DEBUG_BITPL, "BITPLANE_INVERT: %d\n", Invert);

    eIMode = (vc1_eBitplaneCodingMode)vc1DECBIT_GetVLC(pBitstream, vc1DECBITPL_IMODE_Table);
    if(VC1DECBIT_EOF == eIMode)
    {
        return(vc1_ResultBufferExhausted);
    }
    COVERAGE("7.2.2");
    COVERAGE("8.7.2");
    DEBUG1(vc1DEBUG_BITPL, "BITPLANE_IMODE: %d\n", eIMode);

    pBitplane->Position = 0;
    pBitplane->RawMode = FALSE;

    DEBUG2(vc1DEBUG_BITPL, "WidthMB = %d, HeightMB = %d\n", WidthMB, HeightMB);

    DEBUG1(vc1DEBUG_BITPL, "Invert = %d\n", (FLAG)Invert);

    COVERAGE("7.2.3");
    switch(eIMode)
    {
        case vc1_BitplaneCodingModeNorm2:
            eResult = vc1DECBITPL_DecodeNorm2Bits(pBitplane,
                                                  pBitstream,
                                                  WidthMB,
                                                  HeightMB,
                                                  (FLAG)Invert);
            break;
        case vc1_BitplaneCodingModeNorm6:
            eResult = vc1DECBITPL_DecodeNorm6Bits(pBitplane,
                                                  pBitstream,
                                                  WidthMB,
                                                  HeightMB,
                                                  (FLAG)Invert);
            break;
        case vc1_BitplaneCodingModeRowskip:
            eResult = vc1DECBITPL_DecodeRowskipBits(pBitplane,
                                                    pBitstream,
                                                    WidthMB,
                                                    HeightMB,
                                                    (FLAG)Invert);
            break;
        case vc1_BitplaneCodingModeColskip:
            eResult = vc1DECBITPL_DecodeColskipBits(pBitplane,
                                                    pBitstream,
                                                    WidthMB,
                                                    HeightMB,
                                                    (FLAG)Invert);
            break;
        case vc1_BitplaneCodingModeDiff2:
            eResult = vc1DECBITPL_DecodeDiff2Bits(pBitplane,
                                                  pBitstream,
                                                  WidthMB,
                                                  HeightMB,
                                                  (FLAG)Invert);
            break;
        case vc1_BitplaneCodingModeDiff6:
            eResult = vc1DECBITPL_DecodeDiff6Bits(pBitplane,
                                                  pBitstream,
                                                  WidthMB,
                                                  HeightMB,
                                                  (FLAG)Invert);
            break;
        case vc1_BitplaneCodingModeRaw:
            eResult = vc1DECBITPL_DecodeRawBits(pBitplane);
            break;
    }

    if (eIMode != vc1_BitplaneCodingModeRaw)
    {
        pState->BitplaneCodingUsed = TRUE;
    }

    return(eResult);
}


/*
 * Description:
 * Read the next bit in a bitplane
 *
 * Remarks:
 * If the bitplane is raw mode, bits are read directly from the bitstream
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
 * VC1DECBIT_EOF - if buffer exhausted
 * bit value     - otherwise
 *
 */

WORD32 vc1DECBITPL_ReadBitplaneBit(vc1DEC_sBitplane * pBitplane, vc1DEC_sBitstream * pBitstream)
{
    WORD32 Value;

    if(TRUE == pBitplane->RawMode)
    {
        /* RAW mode */
        Value = vc1DECBIT_GetBits(pBitstream, 1);
    }
    else
    {
        Value = pBitplane->pBitplane[pBitplane->Position];
    }

    pBitplane->Position++;

    return(Value);
}



/*
 * Description:
 * Skip the next bit in a bitplane
 *
 * Remarks:
 * If the bitplane is raw mode, nothing happens. If in non-raw mode, the bitplane
 * pointer is updated.
 *
 * Inputs:
 * pBitplane     - pointer to a bitplane structure, containing the bitplane information 
 *
 * Outputs:
 * pBitplane     - updated with new position in bitplane
 *
 * Return Value:
 * None
 *
 */

void vc1DECBITPL_SkipBitplaneBit(vc1DEC_sBitplane * pBitplane)
{
    if(FALSE == pBitplane->RawMode)
    {
        pBitplane->Position++;
    }
}












