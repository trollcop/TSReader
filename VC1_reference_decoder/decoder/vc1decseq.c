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
 * vc1decseq.c:
 * Bitstream sequence layer unpack functions
 *
 */

#include "vc1types.h"
#include "vc1dec.h"
#include "vc1gentab.h"
#include "vc1decbit.h"
#include "vc1decseq.h"
#include "vc1debug.h"
#include "vc1hrd.h"


/*
 * Description:
 * Unpack the sequence layer of the bitstream, and put the values into a struct
 *
 * Remarks:
 * No remarks.
 *
 * Inputs:
 * pState - decoder state structure, into which data will be put
 * pBitstream - structure representing position in bitstream
 *
 * Outputs:
 * pBitstream - updated with new position in bitstream
 * pState - filled out for information from bitstream
 *
 * Return Value:
 * Communicate syntax error here
 * 
 */

vc1_eResult vc1DECSEQ_UnpackSequenceLayer(vc1DEC_sState *pState, vc1DEC_sBitstream * pBitstream)
{
    WORD32     Value;
    vc1_sSequenceLayer *pParams = &pState->sSeqParams;

    /* Defaults for sequence layer params */
    pParams->RefDistFlag = TRUE;



    /* Profile */
    Value = vc1DECBIT_GetBits(pBitstream, VC1_BITS_PROFILE);
    if(VC1DECBIT_EOF == Value)
    {
        return(vc1_ResultBufferExhausted); 
    }
    pParams->eProfile = (vc1_eProfile)Value;
    pState->sPosition.eProfile = (vc1_eProfile)Value;
    DEBUG1(vc1DEBUG_SEQ, "PROFILE: %d\n", Value);
    COVERAGE("6.1.1");
    if(vc1_ProfileReserved == Value)
    {
        FATAL("PROFILE is reserved or forbidden: %d\n", Value);
        return vc1_ResultFatal;
    }


    /* Reserved */
    if((vc1_ProfileSimple == pParams->eProfile) || (vc1_ProfileMain == pParams->eProfile))
    {
        Value = vc1DECBIT_GetBits(pBitstream, VC1_BITS_RES_SM);
        if(VC1DECBIT_EOF == Value)
        {
            return(vc1_ResultBufferExhausted);
        }
        DEBUG1(vc1DEBUG_SEQ, "RES_SM: %d\n", Value);
        COVERAGE("6.1.4");
        if(0 != Value)
        {
            WARN("RES_SM not 0: %d\n", Value);
        }
    }
   
    if(vc1_ProfileAdvanced == pParams->eProfile)
    {
        Value = vc1DECBIT_GetBits(pBitstream, VC1_BITS_LEVEL);
        if(VC1DECBIT_EOF == Value)
        {
            return(vc1_ResultBufferExhausted);
        }
        pParams->eLevel = (vc1_eLevel)Value;
        DEBUG1(vc1DEBUG_SEQ, "LEVEL: %d\n", Value);
        COVERAGE( "6.1.2");
        if(vc1_LevelL4 < Value)
        {
            WARN("LEVEL is reserved or forbidden: %d\n", Value);
        }

        COVERAGE("6.1.3");
        /* Chroma format */
        Value = vc1DECBIT_GetBits(pBitstream, VC1_BITS_CHROMA_FORMAT);
        if(VC1DECBIT_EOF == Value)
        {
            return(vc1_ResultBufferExhausted);
        }
        pParams->eChromaFormat = (vc1_eChromaFormat)Value;
        DEBUG1(vc1DEBUG_SEQ, "CHROMA_FORMAT: %d\n", Value);
        if(vc1_ChromaFormat420 != Value)
        {
            WARN("CHROMA_FORMAT is reserved or forbidden: %d\n", Value);
        }
    }

    /* Quantized frame rate for post processing indicator */
    Value = vc1DECBIT_GetBits(pBitstream, VC1_BITS_FRMRTQ_POSTPROC);
    if(VC1DECBIT_EOF == Value)
    {
        return(vc1_ResultBufferExhausted);
    }
    pParams->QFrameRateForPostProc = (UBYTE8)Value;
    DEBUG1(vc1DEBUG_SEQ, "FRMRTQ_POSTPROC: %d\n", Value);
    COVERAGE( "6.1.5");


    /* Quantised bit rate for post processing indicator */
    Value = vc1DECBIT_GetBits(pBitstream, VC1_BITS_BITRTQ_POSTPROC);
    if(VC1DECBIT_EOF == Value)
    {
        return(vc1_ResultBufferExhausted);
    }
    pParams->QBitRateForPostProc = (UBYTE8)Value;
    DEBUG1(vc1DEBUG_SEQ, "BITRTQ_POSTPROC: %d\n", Value);
    COVERAGE( "6.1.6");


    if((vc1_ProfileSimple == pParams->eProfile) || (vc1_ProfileMain == pParams->eProfile))
    {
        /* Loop filter */
        Value = vc1DECBIT_GetBits(pBitstream, VC1_BITS_LOOPFILTER);
        if(VC1DECBIT_EOF == Value)
        {
            return(vc1_ResultBufferExhausted);
        }
        pParams->LoopFilter = (FLAG)Value;
        DEBUG1(vc1DEBUG_SEQ, "LOOPFILTER: %d\n", Value);
        COVERAGE("6.1.11");


        /* Reserved coding */
        Value = vc1DECBIT_GetBits(pBitstream, VC1_BITS_RES_X8);
        if(VC1DECBIT_EOF == Value)
        {
            return(vc1_ResultBufferExhausted);
        }
        DEBUG1(vc1DEBUG_SEQ, "RES_X8: %d\n", Value);
        COVERAGE("6.1.12");
        if(FALSE != Value)
        {
            WARN("RES_X8 not 0: %d\n", Value);
        }

        /* Multiresolution coding */
        Value = vc1DECBIT_GetBits(pBitstream, VC1_BITS_MULTIRES);
        if(VC1DECBIT_EOF == Value)
        {
            return(vc1_ResultBufferExhausted);
        }
        pParams->MultiResCoding = (FLAG)Value;
        DEBUG1(vc1DEBUG_SEQ, "MULTIRES: %d\n", Value);
        COVERAGE("6.1.13");

        /* Reserved */
        Value = vc1DECBIT_GetBits(pBitstream, VC1_BITS_RES_FASTTX);
        if(VC1DECBIT_EOF == Value)
        {
            return(vc1_ResultBufferExhausted);
        }
        DEBUG1(vc1DEBUG_SEQ, "RES_FASTTX: %d\n", Value);
        COVERAGE("6.1.14");
        if(TRUE != Value)
        {
            WARN("RES_FASTTX not 1: %d\n", Value);
        }

        /* Fast UV Motion Comp */
        Value = vc1DECBIT_GetBits(pBitstream, VC1_BITS_FASTUVMC);
        if(VC1DECBIT_EOF == Value)
        {
            return(vc1_ResultBufferExhausted);
        }
        pParams->FastUVMC = (FLAG)Value;
        pState->sPosition.FastUVMC = (FLAG)Value;
        DEBUG1(vc1DEBUG_SEQ, "FASTUVMC: %d\n", Value);
        COVERAGE("6.1.15");
        if((vc1_ProfileSimple == pParams->eProfile) && (1 != Value))
        {
            WARN("FASTUVMC not 1 in simple profile: %d\n", Value);
        }

        /* Extended Motion Vectors */
        Value = vc1DECBIT_GetBits(pBitstream, VC1_BITS_EXTENDED_MV);
        if(VC1DECBIT_EOF == Value)
        {
            return(vc1_ResultBufferExhausted);
        }
        pParams->ExtendedMV = (FLAG)Value;
        DEBUG1(vc1DEBUG_SEQ, "EXTENDED_MV: %d\n", Value);
        COVERAGE("6.1.16");
        if((vc1_ProfileSimple == pParams->eProfile) && (1 == Value))
        {
            WARN("EXTENDED_MV not 0 in simple profile: %d\n", Value);
        }        

        /* Macroblock quantisation */
        Value = vc1DECBIT_GetBits(pBitstream, VC1_BITS_DQUANT);
        if(VC1DECBIT_EOF == Value)
        { 
            return(vc1_ResultBufferExhausted);
        }
        pParams->DQuant = (UBYTE8)Value;
        DEBUG1(vc1DEBUG_SEQ, "DQUANT: %d\n", Value);
        COVERAGE("6.1.17");
        if(3 == Value)
        {
            WARN("DQUANT is reserved or invalid: %d\n", Value);
        }

        /* Variable sized transform */
        Value = vc1DECBIT_GetBits(pBitstream, VC1_BITS_VSTRANSFORM);
        if(VC1DECBIT_EOF == Value)
        {
            return(vc1_ResultBufferExhausted);
        }
        pParams->VSTransform = (FLAG)Value;
        DEBUG1(vc1DEBUG_SEQ, "VSTRANSFORM: %d\n", Value);
        COVERAGE("6.1.18");

        /* Reserved */
        Value = vc1DECBIT_GetBits(pBitstream, VC1_BITS_RES_TRANSTAB);
        if(VC1DECBIT_EOF == Value)
        {
            return(vc1_ResultBufferExhausted);
        }
        DEBUG1(vc1DEBUG_SEQ, "TRANSTAB: %d\n", Value);
        COVERAGE("6.1.19");
        if(0 != Value)
        {
            WARN("TRANSTAB not 0: %d\n", Value);
        }

        /* Overlapped transform flag */
        Value = vc1DECBIT_GetBits(pBitstream, VC1_BITS_OVERLAP);
        if(VC1DECBIT_EOF == Value)
        {
            return(vc1_ResultBufferExhausted);
        }
        pParams->OverlappedTransformFlag = (FLAG)Value;
        DEBUG1(vc1DEBUG_SEQ, "OVERLAP: %d\n", Value);
        COVERAGE("6.1.20");

        /* Syncmarker flag */
        Value = vc1DECBIT_GetBits(pBitstream, VC1_BITS_SYNCMARKER);
        if(VC1DECBIT_EOF == Value)
        {
            return(vc1_ResultBufferExhausted);
        }
        pParams->SyncmarkerFlag = (FLAG)Value;
        DEBUG1(vc1DEBUG_SEQ, "SYNCMARKER: %d\n", Value);
        COVERAGE("6.1.21");


        /* Range reduction flag */
        Value = vc1DECBIT_GetBits(pBitstream, VC1_BITS_RANGE_RED);
        if(VC1DECBIT_EOF == Value)
        {
            return(vc1_ResultBufferExhausted);
        }
        pParams->RangeRedFlag = (FLAG)Value;
        DEBUG1(vc1DEBUG_SEQ, "RANGE_RED: %d\n", Value);
        COVERAGE("6.1.22");

        /* Maximum number of consecutive B frames */
        Value = vc1DECBIT_GetBits(pBitstream, VC1_BITS_MAXBFRAMES);
        if(VC1DECBIT_EOF == Value)
        {
            return(vc1_ResultBufferExhausted);
        }
        pParams->MaxBFrames = (UBYTE8)Value;
        DEBUG1(vc1DEBUG_SEQ, "MAXBFRAMES: %d\n", Value);
        COVERAGE("6.1.23");


        /* Quantizer */
        Value = vc1DECBIT_GetBits(pBitstream, VC1_BITS_QUANTIZER);
        if(VC1DECBIT_EOF == Value)
        {
            return(vc1_ResultBufferExhausted);
        }
        pParams->eQuantizer = (vc1_eQuantizer)Value;
        DEBUG1(vc1DEBUG_SEQ, "QUANTIZER: %d\n", Value);
        COVERAGE("6.1.24");
    }

    if(vc1_ProfileAdvanced == pParams->eProfile)
    {
        /* Postprocessing flag */
        Value = vc1DECBIT_GetBits(pBitstream, VC1_BITS_POSTPROCFLAG);
        if(VC1DECBIT_EOF == Value)
        {
            return(vc1_ResultBufferExhausted);
        }
        pParams->PostProcessingFlag = (FLAG)Value;
        DEBUG1(vc1DEBUG_SEQ, "POSTPROCFLAG: %d\n", Value);
        COVERAGE("6.1.25");

        /* Maximum horizontal size of picture */
        Value = vc1DECBIT_GetBits(pBitstream, VC1_BITS_MAX_CODED_WIDTH);
        if(VC1DECBIT_EOF == Value)
        {
            return(vc1_ResultBufferExhausted);
        }
        DEBUG2(vc1DEBUG_SEQ, "MAX_CODED_WIDTH: %d (Width=%d)\n", Value, 2*(Value+1));
        Value = (2 * (Value + 1));
        if (Value != pParams->MaxCodedWidth)
        {
            FATAL("Cannot change Max Coded Width to %d\n", Value);
            return vc1_ResultFatal;
        }
        COVERAGE("6.1.7");

        /* Maximum vertical size of picture */
        Value = vc1DECBIT_GetBits(pBitstream, VC1_BITS_MAX_CODED_HEIGHT);
        if(VC1DECBIT_EOF == Value)
        {
            return(vc1_ResultBufferExhausted);
        }
        DEBUG2(vc1DEBUG_SEQ, "MAX_CODED_HEIGHT: %d (Height=%d)\n", Value, 2*(Value+1));
        Value = (2 * (Value + 1));
        if (Value != pParams->MaxCodedHeight)
        {
            FATAL("Cannot change Max Coded Height to %d\n", Value);
            return vc1_ResultFatal;
        }
        COVERAGE("6.1.8");

        /* PULLDOWN */
        Value = vc1DECBIT_GetBits(pBitstream, VC1_BITS_PULLDOWN);
        if(VC1DECBIT_EOF == Value)
        {
            return(vc1_ResultBufferExhausted);
        }
        pParams->PullDownFlag = (FLAG)Value;
        DEBUG1(vc1DEBUG_SEQ, "PULLDOWN: %d\n", Value);
        COVERAGE("6.1.26");

        /* Interlace */
        Value = vc1DECBIT_GetBits(pBitstream, VC1_BITS_INTERLACE);
        if(VC1DECBIT_EOF == Value)
        {
            return(vc1_ResultBufferExhausted);
        }
        pParams->Interlace = (FLAG)Value;
        DEBUG1(vc1DEBUG_SEQ, "INTERLACE: %d\n", Value);
        COVERAGE("6.1.27");

        /* Frame counter flag */
        Value = vc1DECBIT_GetBits(pBitstream, VC1_BITS_TFCNTRFLAG);
        if(VC1DECBIT_EOF == Value)
        {
            return(vc1_ResultBufferExhausted);
        }
        pParams->FrameCounterFlag = (FLAG)Value;
        DEBUG1(vc1DEBUG_SEQ, "TFCNTRFLAG: %d\n", Value);
        COVERAGE("6.1.28");
    }

    /* Frame interpolation flag */
    Value = vc1DECBIT_GetBits(pBitstream, VC1_BITS_FINTERPFLAG);
    if(VC1DECBIT_EOF == Value)
    {
        return(vc1_ResultBufferExhausted);
    }
    pParams->FrameInterpolationFlag = (FLAG)Value;
    DEBUG1(vc1DEBUG_SEQ, "FINTERPFLAG: %d\n", Value);
    COVERAGE( "6.1.29");    

    if(vc1_ProfileAdvanced == pParams->eProfile)    
    {
        /* Reserved */
        Value = vc1DECBIT_GetBits(pBitstream, VC1_BITS_RESERVED1);
        if(VC1DECBIT_EOF == Value)
        {
            return(vc1_ResultBufferExhausted);
        }
        if (Value != 1)
        {
            WARN("Reserved Value is not 1: %d\n", Value);
        }
        DEBUG1(vc1DEBUG_SEQ, "RESERVED: %d\n", Value);

        /* Reserved */
        Value = vc1DECBIT_GetBits(pBitstream, VC1_BITS_PSF_FLAG);
        if(VC1DECBIT_EOF == Value)
        {
            return(vc1_ResultBufferExhausted);
        }
        DEBUG1(vc1DEBUG_SEQ, "PsF Flag: %d\n", Value);
        pParams->PsF = (FLAG)Value;
        COVERAGE("6.1.13");

        /* Display extension flag */
        Value = vc1DECBIT_GetBits(pBitstream, VC1_BITS_DISPLAY_EXT);
        if(VC1DECBIT_EOF == Value)
        {
            return(vc1_ResultBufferExhausted);
        }
        DEBUG1(vc1DEBUG_SEQ, "DISPLAY_EXT: %d\n", Value);
        COVERAGE("6.1.9");

        if(TRUE == Value)
        {
            /* Disp horiz size */
            Value = vc1DECBIT_GetBits(pBitstream, VC1_BITS_DISP_HORIZ_SIZE);
            if(VC1DECBIT_EOF == Value)
            {
                return(vc1_ResultBufferExhausted);
            }
            DEBUG1(vc1DEBUG_SEQ, "DISP_HORIZ_SIZE: %d\n", Value);
            Value = Value + 1;
            pParams->DisplayWidth   = (UHWD16)Value;
            COVERAGE("6.1.9.1");

            /* Disp vert size */
            Value = vc1DECBIT_GetBits(pBitstream, VC1_BITS_DISP_VERT_SIZE);
            if(VC1DECBIT_EOF == Value)
            {
                return(vc1_ResultBufferExhausted);
            }
            DEBUG1(vc1DEBUG_SEQ, "DISP_VERT_SIZE : %d\n", Value);
            Value = Value + 1;
            pParams->DisplayHeight = (UHWD16)Value;
            COVERAGE("6.1.9.2");

            /* Aspect ratio flag */
            Value = vc1DECBIT_GetBits(pBitstream, VC1_BITS_ASPECT_RATIO_FLAG);
            if(VC1DECBIT_EOF == Value)
            {
                return(vc1_ResultBufferExhausted);
            }
            DEBUG1(vc1DEBUG_SEQ, "ASPECT_RATIO_FLAG: %d\n", Value);
            if(TRUE == Value)
            {
                /* Aspect ratio */
                Value = vc1DECBIT_GetBits(pBitstream, VC1_BITS_ASPECT_RATIO);
                if(VC1DECBIT_EOF == Value)
                {
                    return(vc1_ResultBufferExhausted);
                }
                DEBUG1(vc1DEBUG_SEQ, "ASPECT_RATIO: %d\n", Value);
                COVERAGE("6.1.9.3.1");
                if(15 == Value)
                {
                    /* Aspect width */
                    Value = vc1DECBIT_GetBits(pBitstream, VC1_BITS_ASPECT_WIDTH);
                    if(VC1DECBIT_EOF == Value)
                    {
                        return(vc1_ResultBufferExhausted);
                    }
                    pParams->AspectWidth = (UHWD16)(Value + 1);
                    DEBUG1(vc1DEBUG_SEQ, "ASPECT_WIDTH: %d\n", Value);
                    COVERAGE("6.1.9.3.2");

                    /* Aspect height */
                    Value = vc1DECBIT_GetBits(pBitstream, VC1_BITS_ASPECT_HEIGHT);
                    if(VC1DECBIT_EOF == Value)
                    {
                        return(vc1_ResultBufferExhausted);
                    }
                    pParams->AspectHeight = (UHWD16)(Value + 1);
                    DEBUG1(vc1DEBUG_SEQ, "ASPECT_HEIGHT: %d\n", Value);
                    COVERAGE("6.1.9.3.3");
                }
                else
                {
                    pParams->AspectWidth = vc1GENTAB_AspectRatios[Value].Width;
                    pParams->AspectHeight = vc1GENTAB_AspectRatios[Value].Height;
                }                
            }
      
            /* Frame rate flag */
            Value = vc1DECBIT_GetBits(pBitstream, VC1_BITS_FRAMERATEFLAG);
            if(VC1DECBIT_EOF == Value)
            {
                return(vc1_ResultBufferExhausted);
            }
            DEBUG1(vc1DEBUG_SEQ, "FRAMERATEFLAG: %d\n", Value);
            if(TRUE == Value)
            {
                COVERAGE("6.1.9.4.1");
                /* Frame rate indicator */
                Value = vc1DECBIT_GetBits(pBitstream, VC1_BITS_FRAMERATEIND);
                if(VC1DECBIT_EOF == Value)
                {
                    return(vc1_ResultBufferExhausted);
                }
                DEBUG1(vc1DEBUG_SEQ, "FRAMERATEIND: %d\n", Value);
                COVERAGE("6.1.9.4.2");
                if(FALSE == Value)
                {
                    /* Frame rate numerator */
                    Value = vc1DECBIT_GetBits(pBitstream, VC1_BITS_FRAMERATENR);
                    if(VC1DECBIT_EOF == Value)
                    {
                        return(vc1_ResultBufferExhausted);
                    }
                    DEBUG1(vc1DEBUG_SEQ, "FRAMERATENR: %d\n", Value);
                    COVERAGE("6.1.9.4.3");
                    if (Value>=1 && Value<VC1_FRAMERATENR_TBL_SIZE)
                    {
                        pParams->FrameRateNumerator = vc1GENTAB_FrameRateNumerators[Value];
                    }
                    else
                    {

                        WARN("FRAMERATENR is reserved or forbidden: %d\n", Value);
                        pParams->FrameRateNumerator = 0;
                    }

                    /* Frame rate denominator */
                    Value = vc1DECBIT_GetBits(pBitstream, VC1_BITS_FRAMERATEDR);
                    if(VC1DECBIT_EOF == Value)
                    {
                        return(vc1_ResultBufferExhausted);
                    }
                    DEBUG1(vc1DEBUG_SEQ, "FRAMERATEDR: %d\n", Value);
                    COVERAGE("6.1.9.4.4");
                    switch(Value)
                    {
                        case 1:
                            pParams->FrameRateDenominator = 1000;
                            break;

                        case 2:
                            pParams->FrameRateDenominator = 1001;
                            break;
                        default:
                            WARN("FRAMERATEDR is reserved or forbidden: %d\n", Value);
                    }
                }
                else
                {
                    /* Frame rate explicit */
                    Value = vc1DECBIT_GetBits(pBitstream, VC1_BITS_FRAMERATEEXP);
                    if(VC1DECBIT_EOF == Value)
                    {
                        return(vc1_ResultBufferExhausted);
                    }
                    pParams->FrameRateNumerator = Value + 1;
                    DEBUG1(vc1DEBUG_SEQ, "FRAMERATEEXP: %d\n", Value);

                    /* always 32 for frame rate denominator */
                    pParams->FrameRateDenominator = 32;
                }
            }
            else
            {
                pParams->FrameRateNumerator = 0;
                pParams->FrameRateDenominator = 0;
            }

            /* Color format indicator */
            Value = vc1DECBIT_GetBits(pBitstream, VC1_BITS_COLOR_FORMAT_FLAG);
            if(VC1DECBIT_EOF == Value)
            {
                return(vc1_ResultBufferExhausted);
            }
            pParams->ColorFormatIndicatorFlag = (FLAG)Value;
            DEBUG1(vc1DEBUG_SEQ, "COLOR_FORMAT_FLAG: %d\n", Value);
            if(TRUE == Value)
            {
                /* Color primaries */
                Value = vc1DECBIT_GetBits(pBitstream, VC1_BITS_COLOR_PRIM);
                if(VC1DECBIT_EOF == Value)
                {
                    return(vc1_ResultBufferExhausted);
                }
                pParams->eColorPrimaries = (vc1_eColorPrimaries)Value;
                DEBUG1(vc1DEBUG_SEQ, "COLOR_PRIM: %d\n", Value);
                COVERAGE("6.1.9.5.1");
                if((vc1_ColorPrimariesForbidden == Value)
                    || (vc1_ColorPrimariesReserved1 == Value)
                    || (vc1_ColorPrimariesReserved3 <= Value))
                {
                    WARN("COLOR_PRIM is reserved or forbidden: %d\n", Value);
                }

                /* Transfer characteristics */
                Value = vc1DECBIT_GetBits(pBitstream, VC1_BITS_TRANSFER_CHAR);
                if(VC1DECBIT_EOF == Value)
                {
                    return(vc1_ResultBufferExhausted);
                }
                pParams->eTransChar = (vc1_eTransChar)Value;
                DEBUG1(vc1DEBUG_SEQ, "TRANSFER_CHAR: %d\n", Value);
                COVERAGE("6.1.9.5.2");
                if((vc1_TransCharForbidden == Value)
                    || (vc1_TransCharReserved1 == Value)
                    || (vc1_TransCharReserved2 == Value)
                    || (vc1_TransCharReserved3 == Value)
                    || (vc1_TransCharReserved4 == Value)
                    || (vc1_TransCharReserved5 <= Value))
                {
                    WARN("TRANSFER_CHAR is reserved or forbidden: %d\n", Value);
                }

                /* Matrix coefficients */
                Value = vc1DECBIT_GetBits(pBitstream, VC1_BITS_MATRIX_COEF);
                if(VC1DECBIT_EOF == Value)
                {
                    return(vc1_ResultBufferExhausted);
                }
                pParams->eMatrixCoefficients = (vc1_eMatrixCoefficients)Value;
                DEBUG1(vc1DEBUG_SEQ, "MATRIX_COEF: %d\n", Value);
                COVERAGE("6.1.9.5.3");
                if( (vc1_MatrixCoefficientsForbidden == Value)      ||
                    (vc1_MatrixCoefficientsReserved1 == Value)      ||
                    (vc1_MatrixCoefficientsReserved2 == Value)      ||
                    (vc1_MatrixCoefficientsReserved3 == Value)      ||
                    (vc1_MatrixCoefficientsReserved4 <= Value)      )
                {
                    WARN("MATRIX_COEF is reserved or forbidden: %d\n", Value);
                }
            }
        }
        else
        {
            pParams->AspectWidth = 0;
            pParams->AspectHeight = 0;
            pParams->FrameRateNumerator = 0;
            pParams->FrameRateDenominator = 0;
        }


        /* Hypothetical reference decoder indicator flag */
        Value = vc1DECBIT_GetBits(pBitstream, VC1_BITS_HRD_PARAM_FLAG);
        if(VC1DECBIT_EOF == Value)
        {
            return(vc1_ResultBufferExhausted);
        }
        DEBUG1(vc1DEBUG_SEQ, "HRD_PARAM_FLAG: %d\n", Value);

        if(TRUE == Value)
        {
            UBYTE8 Count;
            UBYTE8 BitRateExponent;
            UBYTE8 BufferSizeExponent;

            /* hrd_num_leaky_buckets */
            Value = vc1DECBIT_GetBits(pBitstream, VC1_BITS_HRD_NUM_LEAKY_BUCKETS);
            if(VC1DECBIT_EOF == Value)
            {
                return(vc1_ResultBufferExhausted);
            }
            pParams->sHrdInitialState.NumLeakyBuckets = (UBYTE8)Value;
            DEBUG1(vc1DEBUG_SEQ, "HRD_NUM_LEAKY_BUCKETS: %d\n", Value);
            COVERAGE("6.1.10.1");

            /* bit_rate_exponent */
            Value = vc1DECBIT_GetBits(pBitstream, VC1_BITS_BIT_RATE_EXPONENT);
            if(VC1DECBIT_EOF == Value)
            {
                return(vc1_ResultBufferExhausted);
            }
            BitRateExponent = (UBYTE8)(Value + 6);
            DEBUG1(vc1DEBUG_SEQ, "BIT_RATE_EXPONENT: %d\n", Value);

            /* buffer_size_exponent */
            Value = vc1DECBIT_GetBits(pBitstream, VC1_BITS_BUFFER_SIZE_EXPONENT);
            if(VC1DECBIT_EOF == Value)
            {
                return(vc1_ResultBufferExhausted);
            }
            BufferSizeExponent = (UBYTE8)(Value + 4);
            DEBUG1(vc1DEBUG_SEQ, "BUFFER_SIZE_EXPONENT: %d\n", Value);

            for(Count = 0; Count < pParams->sHrdInitialState.NumLeakyBuckets; Count++)
            {
                /* hrd_rate */
                Value = vc1DECBIT_GetBits(pBitstream, VC1_BITS_HRD_RATE);
                if(VC1DECBIT_EOF == Value)
                {
                    return(vc1_ResultBufferExhausted);
                }
                pParams->sHrdInitialState.sLeakyBucket[Count].Rate =
                  (HRDVALUE)(Value + 1) << BitRateExponent;
                DEBUG1(vc1DEBUG_SEQ, "HRD_RATE: %d\n", Value);

                /* hrd_buffer */
                Value = vc1DECBIT_GetBits(pBitstream, VC1_BITS_HRD_BUFFER);
                if(VC1DECBIT_EOF == Value)
                {
                    return(vc1_ResultBufferExhausted);
                }
                pParams->sHrdInitialState.sLeakyBucket[Count].Buffer =
                  (HRDVALUE)(Value + 1) << BufferSizeExponent;
                DEBUG1(vc1DEBUG_SEQ, "HRD_BUFFER: %d\n", Value);

                DEBUG2(vc1DEBUG_SEQ, "HrdRate  [%d] = %10d\n", 
                    Count, pParams->sHrdInitialState.sLeakyBucket[Count].Rate);
                DEBUG2(vc1DEBUG_SEQ, "HrdBuffer[%d] = %10d\n", 
                    Count, pParams->sHrdInitialState.sLeakyBucket[Count].Buffer);
            }

            COVERAGE("C.3.1");
            /* initialise the hypothetical reference decoder with this data */
            vc1HRD_Init(&pParams->sHrdInitialState, &pParams->sHrdInitialState);
        }
        else
        {
            pParams->sHrdInitialState.NumLeakyBuckets = 0;
        }
    }

    if((vc1_ProfileSimple == pParams->eProfile) || (vc1_ProfileMain == pParams->eProfile))
    {
        /* Reserved RTM flag */
        Value = vc1DECBIT_GetBits(pBitstream, VC1_BITS_RES_RTM_FLAG);
        if(VC1DECBIT_EOF == Value)
        {
            return(vc1_ResultBufferExhausted);
        }
        pParams->ReservedRTMFlag = (FLAG)Value;
        DEBUG1(vc1DEBUG_SEQ, "RES_RTM_FLAG: %d\n", Value);
        COVERAGE( "6.1.30");
        if(1 != Value)
        {
            WARN("RES_RTM_FLAG not 1: %d\n", Value);
        }
    }

    return (vc1_ResultOK);
}

