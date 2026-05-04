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
 * vc1decslice.c:
 * Bitstream slice layer unpack functions
 *
 */


#include "vc1types.h"
#include "vc1dec.h"
#include "vc1decbit.h"
#include "vc1debug.h"
#include "vc1decpic.h"
#include "vc1decmb.h"
#include "vc1tools.h"
#include "vc1deblock.h"
#include "vc1decslice.h"



/*
 * Description:
 * Decode a slice of picture data
 *
 * Remarks:
 * There may be no slices, in which case this will decode the entire image
 *
 * Inputs:
 * pState       - decoder state structure, into which data will be put
 * pBitstream   - structure representing position in bitstream
 *
 * Outputs:
 * pState       - updated with data from bitstream
 * pBitstream   - updated with new position in bitstream
 *
 * Return Value:
 * Standard vc1_eResult result. See enumeration for possible result codes.
 *
 */
vc1_eResult vc1DECSLICE_DecodeSlice(vc1DEC_sState * pState, vc1DEC_sBitstream * pBitstream)
{
    vc1_sPosition *pPosition = &pState->sPosition;
    vc1_eResult eResult;

    DEBUG2(vc1DEBUG_SLICE, "SliceY = %d (%08x)\n", pPosition->SliceY, pPosition->pReferenceNew);

    /* reset escape mode 3 state */
    pState->NotFirstMode3InFrame = 0;

    pPosition->Y = 0;

    if(vc1_PictureTypeSkipped != pPosition->ePictureType)
    {
        /* Iterate over all macroblocks in the slice */
        while (pPosition->SliceY + pPosition->Y < pPosition->HeightMB)
        {
            if (pState->BitplaneCodingUsed==FALSE && vc1DECBIT_BitCountGet(pBitstream)==0)
            {
                /* We need more data to complete the picture - ask for another slice */
                break;
            }

            DEBUG2(vc1DEBUG_SLICE, "Bytes/bits left = %d/%d\n",
                vc1DECBIT_ByteCountGet(pBitstream), vc1DECBIT_BitCountGet(pBitstream));

            if( (TRUE == pState->sSeqParams.SyncmarkerFlag) &&
                (TRUE == vc1_PictureTypeIsRef(pPosition->ePictureType)) &&
                (0 != pPosition->Y) )
            {
                /* Unpack the syncmarker, if one is detected */
                WORD32 Value;
                Value = vc1DECBIT_GetBits(pBitstream, 1);
                if(VC1DECBIT_EOF == Value)
                {
                    return(vc1_ResultBufferExhausted);
                }
                if(0 == Value)
                {
                    eResult = vc1DECPIC_UnpackSyncmarker(pBitstream);
                    if(vc1_ResultOK != eResult)
                    {
                        return(eResult);
                    }
                }
            }

            for(pPosition->X = 0; pPosition->X < pPosition->WidthMB; pPosition->X++)
            {
                DEBUG0(vc1DEBUG_MB, "UnpackMacroblockLayer()\n");
                eResult = vc1DECMB_UnpackMacroblockLayer(pState, pBitstream);

                if(vc1_ResultOK != eResult)
                {
                    /* Macroblock decode failed */
                    DEBUG0(vc1DEBUG_MB, "Macroblock decode failed\n");

                    /* Deblock what we have so far so we can see where the
                     * first real error lies
                     */
                    if (pState->sSeqParams.LoopFilter)
                    {
                        pPosition->PQuant = pState->sPicParams.PQuant;
                        pPosition->Y++;
                        pPosition->pCurMB += (pPosition->WidthMB - pPosition->X);
                        pPosition->X = 0;
                        vc1DEBLOCK_DeblockSlice(pPosition);
                    }

                    if (vc1_PictureTypeIsRef(pPosition->ePictureType))
                    {
                        /* Partially decoded ref picture */
                        if (pPosition->ePictureFormat==vc1_InterlacedField &&
                            pPosition->SecondField==0)
                        {
                            /* Copy first decoded field */
                            vc1TOOLS_CopyReference( pPosition->pReferenceNoIC,
                                                    pPosition->pReferenceNew);
                        }
                    }
                    else /* B or BI */
                    {
                        /* output the partially decoded B frame to the application */
                        pPosition->pReferenceB->Valid = TRUE;
                        vc1DECPIC_DisplayPicture(pState, pPosition->pReferenceB);
                    }
                    return(eResult);
                }
            }

            pPosition->Y++;
        }

        /* Deblock slice */
        if (pState->sSeqParams.LoopFilter)
        {
            pPosition->PQuant = pState->sPicParams.PQuant;
            vc1DEBLOCK_DeblockSlice(pPosition);
        }

        DEBUG3(vc1DEBUG_SLICE,
            "Decoded %d rows in this slice. %d/%d rows completed\n",
            pPosition->Y, pPosition->SliceY + pPosition->Y, pPosition->HeightMB);

        if((pPosition->SliceY + pPosition->Y) != pPosition->HeightMB)
        {
            /* the picture is incomplete - ask for another slice */
            DEBUG0(vc1DEBUG_SLICE, "Requesting next slice\n");
            return(vc1_ResultSlice);
        }
    }
    else
    {
        /* this is a skipped picture - copy the old reference into the new reference */
        vc1TOOLS_CopyReference(pPosition->pReferenceNew, pPosition->pReferenceOld);
        COVERAGE("8.4.1");

        /* update fields from skipped picture */
        pPosition->pReferenceNew->ePictureFormat = pPosition->ePictureFormat;
        pPosition->pReferenceNew->ePictureType[0] = pPosition->ePictureType;
        pPosition->pReferenceNew->Frame = pState->FrameNum;
    }

    /* At this point we have finished a field or frame */

    if (pPosition->ePictureFormat==vc1_InterlacedField && pPosition->SecondField==0)
    {
        /* We have finished the first field of an interlaced field picture */
        if (0 == vc1DECBIT_BitCountGet(pBitstream))
        {
            /* there are no more bits, and we've only decoded one field */
            DEBUG0(vc1DEBUG_PIC, "Requesting next field\n");
            return(vc1_ResultField);
        }

        FATAL (" Field Start Code is missing for frame %d Second Field is starting immediately after first field \n", pState->FrameNum);
        return vc1_ResultFatal;
        /* This code has been removed as Field start code is now mandatory.
        /* end of first field, will now start second field */
        /*
        DEBUG0(vc1DEBUG_SLICE, "Byte aligining at end of first field\n");
        vc1DECBIT_GetBits(pBitstream, 1);
        vc1DECBIT_AlignBit(pBitstream);

        return vc1_ResultOK; */
    }

    /* This Frame now complete */

    /*
     * if the previously decoded frame was not a 
     * reference (B, BI) output it to the application now
     */
    if((FALSE == vc1_PictureTypeIsRef(pPosition->ePictureType))
        && (vc1_PictureTypeSkipped != pPosition->ePictureType))
    {
        pPosition->pReferenceB->Valid = TRUE;
        vc1DECPIC_DisplayPicture(pState, pPosition->pReferenceB);
    }

    if (TRUE == pState->FirstFrameInStream)
    {
        /* the first frame in a stream is buffered for prediction purposes */
        DEBUG0(vc1DEBUG_PIC, "First frame in stream - No frame returned\n");
        pState->FirstFrameInStream = FALSE;
        return(vc1_ResultNoFrame);
    }

    return(vc1_ResultOK);
}


/*
 * Description:
 * Unpack the slice layer of the bitstream, and put the values into state
 *
 * Remarks:
 *
 * Inputs:
 * pState - decoder state structure, into which data will be put
 * pBitstream - structure representing position in bitstream
 *
 * Outputs:
 * pState - updated with data from bitstream
 * pBitstream - updated with new position in bitstream
 *
 * Return Value:
 * Standard vc1_eResult result. See enumeration for possible result codes.
 *
 */

vc1_eResult vc1DECSLICE_UnpackSliceLayer(vc1DEC_sState * pState, vc1DEC_sBitstream * pBitstream)
{
    WORD32     Value;
    vc1_sPosition *pPosition        = &pState->sPosition;
    vc1_eResult eResult;

    COVERAGE("7.1.2");
    
    /* Slice address */
    Value = vc1DECBIT_GetBits(pBitstream, VC1_BITS_SLICE_ADDR);
    if(VC1DECBIT_EOF == Value)
    {
        return(vc1_ResultBufferExhausted);
    }
    DEBUG1(vc1DEBUG_PIC, "SLICE_ADDR: %d\n", Value);

    if (pPosition->SecondField)
    {
        Value -= pPosition->HeightMB;
    }
    if (Value != pPosition->SliceY + pPosition->Y)
    {
        WARN("New slice address %d doesn't follow on from old %d\n",
            Value, pPosition->SliceY + pPosition->Y);
    }

    pPosition->SliceY = (UHWD16)Value;
    COVERAGE("7.1.2.1");

    /* Picture header present flag */
    Value = vc1DECBIT_GetBits(pBitstream, VC1_BITS_PIC_HEADER_FLAG);
    if(VC1DECBIT_EOF == Value) 
    {
        return(vc1_ResultBufferExhausted);
    }
    DEBUG1(vc1DEBUG_PIC, "PIC_HEADER_FLAG: %d\n", Value);
    COVERAGE("7.1.2.2");

    if(TRUE == Value)
    {
        /* Update picture layer parameters structure with new picture layer in the bitstream */
        vc1_eResult Result = vc1DECPIC_UnpackInSlicePictureLayer(pState, pBitstream);
        if(vc1_ResultOK != Result)
        {
            return(Result);
        }
    }

    eResult = vc1DECSLICE_DecodeSlice(pState, pBitstream);
    if(eResult != vc1_ResultOK)
    {
        return(eResult);
    }

    return(vc1_ResultOK);
}

