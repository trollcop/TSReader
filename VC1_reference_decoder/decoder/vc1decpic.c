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
 * vc1decpic.c:
 * Bitstream picture layer unpack functions
 *
 */

#include "vc1types.h"
#include "vc1dec.h"
#include "vc1decpic.h"
#include "vc1gentab.h"
#include "vc1decbit.h"
#include "vc1decseq.h"
#include "vc1debug.h"
#include "vc1dec.h"
#include "vc1decmb.h"
#include "vc1iquant.h"
#include "vc1decbitpl.h"
#include "vc1decpictab.h"
#include "vc1tools.h"
#include "vc1scalemv.h"
#include "vc1decslice.h"

#include <string.h> /* memset */

static const vc1DEC_sVLCCode vc1DECPIC_P_Picture_Low_Rate_Motion_Vector_Mode[6] =
{
    {0, 5, 4},
    {1, 1, vc1_MVMode1MVHalfPelBilinear},
    {1, 2, vc1_MVMode1MV},
    {1, 3, vc1_MVMode1MVHalfPel},
    {0, 4, vc1_MVModeMixedMV},
    {1, 4, vc1_MVModeIntensityCompensation}
};

static const vc1DEC_sVLCCode vc1DECPIC_P_Picture_High_Rate_Motion_Vector_Mode[6] =
{
    {0, 5, 4},
    {1, 1, vc1_MVMode1MV},
    {1, 2, vc1_MVModeMixedMV},
    {1, 3, vc1_MVMode1MVHalfPel},
    {0, 4, vc1_MVMode1MVHalfPelBilinear},
    {1, 4, vc1_MVModeIntensityCompensation}
};

static const vc1DEC_sVLCCode vc1DECPIC_B_Picture_Low_Rate_Motion_Vector_Mode[6] =
{
    {0, 4, 3},
    {1, 1, vc1_MVMode1MVHalfPelBilinear},
    {1, 2, vc1_MVMode1MV},
    {1, 3, vc1_MVMode1MVHalfPel},
    {0, 3, vc1_MVModeMixedMV},
};

static const vc1DEC_sVLCCode vc1DECPIC_B_Picture_High_Rate_Motion_Vector_Mode[6] =
{
    {0, 4, 3},
    {1, 1, vc1_MVMode1MV},
    {1, 2, vc1_MVModeMixedMV},
    {1, 3, vc1_MVMode1MVHalfPel},
    {0, 3, vc1_MVMode1MVHalfPelBilinear},
};

static const vc1DEC_sVLCCode *vc1DECPIC_P_Picture_Low_Rate_Motion_Vector_Mode_2 = 
    vc1DECPIC_B_Picture_Low_Rate_Motion_Vector_Mode;

static const vc1DEC_sVLCCode *vc1DECPIC_P_Picture_High_Rate_Motion_Vector_Mode_2 =
    vc1DECPIC_B_Picture_High_Rate_Motion_Vector_Mode;


static const vc1DEC_sVLCCode vc1DECPIC_Conditional_Overlap_Table[4] =
{
    {0, 3, 2},
    {0, 1, vc1_CondOverNone},
    {2, 2, vc1_CondOverAll},
    {3, 2, vc1_CondOverSome}
};

static const vc1DEC_sVLCCode vc1DECPIC_Picture_Type_Table[6] =
{
    {0, 5, 4},
    {0, 1, vc1_PictureTypeP},
    {2, 2, vc1_PictureTypeB},
    {6, 3, vc1_PictureTypeI},
    {14, 4, vc1_PictureTypeBI},
    {15, 4, vc1_PictureTypeSkipped}
};

static const vc1DEC_sVLCCode vc1DECPIC_Motion_Vector_Range_Table[5] =
{
    {0, 4, 3},
    {0, 1, vc1_MVRange64_32},
    {2, 2, vc1_MVRange128_64},
    {6, 3, vc1_MVRange512_128},
    {7, 3, vc1_MVRange1024_256}
};

static const vc1DEC_sVLCCode vc1DECPIC_Differential_Motion_Vector_Range_Table[5] =
{
    {0, 4, 3},
    {0, 1, 0},
    {2, 2, 2},
    {6, 3, 1},
    {7, 3, 3}
};

static const vc1DEC_sVLCCode vc1DECPIC_Intensity_Compensation_Field_Table[4] =
{
    {0, 3, 2},
    {1, 1, 0},
    {0, 2, 1},
    {1, 2, 2}
};

/* table 25 */
static const vc1DEC_sVLCCode vc1DECPIC_B_Fraction_Table[24] =
{
    {0, 23, 7},
    {0, 3, 0},      {1, 3, 1},      {2, 3, 2},      {3, 3, 3},
    {4, 3, 4},      {5, 3, 5},      {6, 3, 6},      {112, 7, 7},
    {113, 7, 8},    {114, 7, 9},    {115, 7, 10},   {116, 7, 11},
    {117, 7, 12},   {118, 7, 13},   {119, 7, 14},   {120, 7, 15},
    {121, 7, 16},   {122, 7, 17},   {123, 7, 18},   {124, 7, 19},
    {125, 7, 20},   {126, 7, 21},   {127, 7, 22}
};



/*
 * Description:
 * Set the dimensions in the macroblock position structure
 *
 * Remarks:
 * None.
 *
 * Inputs:
 * pState       - pointer to the decoder's state
 * pPosition    - pointer to struct representing current macroblock position
 *
 * Outputs:
 * pPosition    - updated with size information
 *
 * Return Value:
 * None.
 * 
 */

static void vc1DECPIC_SetDimensionsInMB(vc1DEC_sState * pState, vc1_sPosition * pPosition)
{
    int CodedWidth  = pState->sSeqParams.CodedWidth;
    int CodedHeight = pState->sSeqParams.CodedHeight;

    if( (vc1_ProfileSimple == pState->sSeqParams.eProfile)  || 
        (vc1_ProfileMain == pState->sSeqParams.eProfile)    )
    {
        if (vc1_PictureRes2x1 & pPosition->ePictureRes)
        {
            COVERAGE("8.1.1.3");
            CodedWidth = CodedWidth/2;
        }
        if (vc1_PictureRes1x2 & pPosition->ePictureRes)
        {
            COVERAGE("8.1.1.3");
            CodedHeight = CodedHeight/2;
        }
    }

    pPosition->CodedWidth  = CodedWidth;
    pPosition->CodedHeight = CodedHeight;

    if (vc1_InterlacedField == pPosition->ePictureFormat)
    {
        CodedHeight = CodedHeight/2;
    }

    pPosition->WidthMB  = (UHWD16)((CodedWidth+15)>>4);
    pPosition->HeightMB = (UHWD16)((CodedHeight+15)>>4);

    /* Set the total size field now the width and height are known */
    pPosition->SizeMB   = pPosition->WidthMB * pPosition->HeightMB;
}





/*
 * Description:
 * Unpack the motion vector mode parameters in interlaced pictures.
 *
 * Remarks:
 * None.
 *
 * Inputs:
 * pParams      - pointer to picture layer parameters structure
 * pBitstream   - pointer to struct representing the current bitstrea
 * pPosition    - pointer to struct indicating the current macroblock (for field info)
 *
 * Outputs:
 * pParams      - updated with read data
 * pBitstream   - updated with new position
 *
 * Return Value:
 * Standard vc1_eResult result. See enumeration for possible result codes.
 * 
 */

static vc1_eResult vc1DECPIC_UnpackInterlaceMVModeParams(   vc1DEC_sPictureLayerParams * pParams,
                                                            vc1DEC_sBitstream * pBitstream,
                                                            vc1_sPosition * pPosition)
{
    WORD32 Value;
    const vc1DEC_sVLCCode *pTable;

    if(vc1_PictureTypeB == pPosition->ePictureType)
    {
        if(12 < pParams->PQuant)
        {
            pTable = vc1DECPIC_B_Picture_Low_Rate_Motion_Vector_Mode;
        }
        else
        {
            pTable = vc1DECPIC_B_Picture_High_Rate_Motion_Vector_Mode;
        }
    }
    else /* P pictures */
    {
        if(12 < pParams->PQuant)
        {
            pTable = vc1DECPIC_P_Picture_Low_Rate_Motion_Vector_Mode;
        }
        else
        {
            pTable = vc1DECPIC_P_Picture_High_Rate_Motion_Vector_Mode;
        }
    }
    
    Value = vc1DECBIT_GetVLC(pBitstream, pTable);
    if(VC1DECBIT_EOF == Value)
    {
        return(vc1_ResultBufferExhausted);
    }

    DEBUG2(vc1DEBUG_PIC, "MVMODE1: %s (%d)\n", vc1DEBUG_MVMode[Value], Value);
    COVERAGE("7.1.1.22");

    if(vc1_MVModeIntensityCompensation == Value)
    {
        DEBUG0(vc1DEBUG_PIC, "Intensity Compensation Enabled\n");

        /* Motion vector mode 2*/
        if(12 < pParams->PQuant)
        {
            Value = vc1DECBIT_GetVLC(   pBitstream,
                                        vc1DECPIC_P_Picture_Low_Rate_Motion_Vector_Mode_2);
        }
        else
        {
            Value = vc1DECBIT_GetVLC(   pBitstream,
                                        vc1DECPIC_P_Picture_High_Rate_Motion_Vector_Mode_2);
        }
        if(VC1DECBIT_EOF == Value)
        {
            return(vc1_ResultBufferExhausted);
        }
        pPosition->eMVMode = (vc1_eMVMode)Value;
        DEBUG2(vc1DEBUG_PIC, "MVMODE2: %s (%d)\n", vc1DEBUG_MVMode[Value], Value);
        COVERAGE("9.1.1.35");


        /* Intensity compensation field */
        Value = vc1DECBIT_GetVLC(pBitstream, vc1DECPIC_Intensity_Compensation_Field_Table);
        if(VC1DECBIT_EOF == Value)
        {
            return(vc1_ResultBufferExhausted);
        }
        DEBUG1(vc1DEBUG_PIC, "INTCOMPFIELD: %d\n", Value);
        COVERAGE("9.1.1.37");

        switch(Value)
        {
            case 0:
                pParams->sIC[0].IntensityCompFlag = TRUE;
                pParams->sIC[1].IntensityCompFlag = TRUE;
                break;

            case 1: 
                pParams->sIC[0].IntensityCompFlag = TRUE;
                pParams->sIC[1].IntensityCompFlag = FALSE;
                break;

            case 2: 
                pParams->sIC[0].IntensityCompFlag = FALSE;
                pParams->sIC[1].IntensityCompFlag = TRUE;
                break;
        }



        /* Luminance scale */
        Value = vc1DECBIT_GetBits(pBitstream, VC1_BITS_LUMSCALE);
        if(VC1DECBIT_EOF == Value)
        {
            return(vc1_ResultBufferExhausted);
        }
        if(TRUE == pParams->sIC[0].IntensityCompFlag)
        {
            pParams->sIC[0].LuminanceScale = (UBYTE8)Value;
        }
        else
        {
            pParams->sIC[1].LuminanceScale = (UBYTE8)Value;
        }

        DEBUG1(vc1DEBUG_PIC, "LUMSCALE1: %d\n", Value);
        COVERAGE("9.1.1.40");

        /* Luminance shift */
        Value = vc1DECBIT_GetBits(pBitstream, VC1_BITS_LUMSHIFT);
        if(VC1DECBIT_EOF == Value)
        {
            return(vc1_ResultBufferExhausted);
        }
        if(TRUE == pParams->sIC[0].IntensityCompFlag)
        {
            pParams->sIC[0].LuminanceShift = (UBYTE8)Value;
        }
        else
        {
            pParams->sIC[1].LuminanceShift = (UBYTE8)Value;
        }

        DEBUG1(vc1DEBUG_PIC, "LUMSHIFT1: %d\n", Value);
        COVERAGE("9.1.1.41");

        if( TRUE == pParams->sIC[0].IntensityCompFlag &&
            TRUE == pParams->sIC[1].IntensityCompFlag)
        {
            Value = vc1DECBIT_GetBits(pBitstream, VC1_BITS_LUMSCALE);
            if(VC1DECBIT_EOF == Value)
            {
                return(vc1_ResultBufferExhausted);
            }
            pParams->sIC[1].LuminanceScale = (UBYTE8)Value;
            DEBUG1(vc1DEBUG_PIC, "LUMSCALE2: %d\n", Value);
            COVERAGE("9.1.1.42");

            Value = vc1DECBIT_GetBits(pBitstream, VC1_BITS_LUMSHIFT);
            if(VC1DECBIT_EOF == Value)
            {
                return(vc1_ResultBufferExhausted);
            }
            pParams->sIC[1].LuminanceShift = (UBYTE8)Value;
            DEBUG1(vc1DEBUG_PIC, "LUMSHIFT2: %d\n", Value);
            COVERAGE("9.1.1.43");
        }

        COVERAGE("8.3.4.3");    /* Now read all the relevant parameters */
    }
    else
    {
        pPosition->eMVMode = (vc1_eMVMode)Value;
        pParams->sIC[0].IntensityCompFlag = FALSE;
        pParams->sIC[1].IntensityCompFlag = FALSE;
        pParams->sIC[0].LuminanceScale = 0;
        pParams->sIC[1].LuminanceScale = 0;
        pParams->sIC[0].LuminanceShift = 0;
        pParams->sIC[1].LuminanceShift = 0;
    }

    return(vc1_ResultOK);
}





/*
 * Description:
 * Unpack the pan and scan parameters of a bitstream.
 *
 * Remarks:
 * None.
 *
 * Inputs:
 * pSeqParams   - pointer to the sequence layer parameters structure
 * pParams      - pointer to the picture layer parameters structure
 * pBitstream   - pointer to struct representing the current bitstream
 *
 * Outputs:
 * pSeqParams   - updated with read data
 * pParams      - updated with read data
 * pBitstream   - updated with new position
 *
 * Return Value:
 * Standard vc1_eResult result. See enumeration for possible result codes.
 * 
 */

static vc1_eResult vc1DECPIC_UnpackPanScanParams(
    vc1_sSequenceLayer * pSeqParams,
    vc1DEC_sPictureLayerParams * pParams,
    vc1DEC_sBitstream * pBitstream)
{
    int Count;
    WORD32 Value;

    /* derive number of pan scan windows */
    if(FALSE == pSeqParams->Interlace || TRUE == pSeqParams->PsF)
    {
        if(FALSE == pSeqParams->PullDownFlag)
        {
            pSeqParams->NumPanScanWin = 1;
        }
        else
        {
            pSeqParams->NumPanScanWin = (UBYTE8)(1 + pParams->RepeatFrameCount);
        }
    }
    else
    {
        if(FALSE == pSeqParams->PullDownFlag)
        {
            pSeqParams->NumPanScanWin = 2;
        }
        else
        {
            pSeqParams->NumPanScanWin = (UBYTE8)(2 + pParams->RepeatFirstField);
        }
    }
    COVERAGE("8.10.1");


    /* pan scan present flag */
    Value = vc1DECBIT_GetBits(pBitstream, VC1_BITS_PS_PRESENT);
    if(VC1DECBIT_EOF == Value)
    {
        return(vc1_ResultBufferExhausted);
    }
    DEBUG1(vc1DEBUG_PIC, "PS_PRESENT: %d\n", Value);
    COVERAGE("7.1.1.6");

    if(TRUE == Value)
    {
        for(Count = 0; Count < pSeqParams->NumPanScanWin; Count++)
        {
            /* pan scan horizontal offset */
            Value = vc1DECBIT_GetBits(pBitstream, VC1_BITS_PS_HOFFSET);
            if(VC1DECBIT_EOF == Value)
            {
                return(vc1_ResultBufferExhausted);
            }
            DEBUG1(vc1DEBUG_PIC, "PS_HOFFSET: %d\n", Value);
            pParams->sPanScanParams.sPanScanWindow[Count].HOffset = Value;
            COVERAGE("7.1.1.7");


            /* pan scan vertical offset */
            Value = vc1DECBIT_GetBits(pBitstream, VC1_BITS_PS_VOFFSET);
            if(VC1DECBIT_EOF == Value)
            {
                return(vc1_ResultBufferExhausted);
            }
            DEBUG1(vc1DEBUG_PIC, "PS_VOFFSET: %d\n", Value);
            pParams->sPanScanParams.sPanScanWindow[Count].VOffset = Value;
            COVERAGE("7.1.1.8");


            /* pan scan width */
            Value = vc1DECBIT_GetBits(pBitstream, VC1_BITS_PS_WIDTH);
            if(VC1DECBIT_EOF == Value)
            {
                return(vc1_ResultBufferExhausted);
            }
            DEBUG1(vc1DEBUG_PIC, "PS_WIDTH: %d\n", Value);
            pParams->sPanScanParams.sPanScanWindow[Count].Width = (UHWD16)Value;
            COVERAGE("7.1.1.9");


            /* pan scan height */
            Value = vc1DECBIT_GetBits(pBitstream, VC1_BITS_PS_HEIGHT);
            if(VC1DECBIT_EOF == Value)
            {
                return(vc1_ResultBufferExhausted);
            }
            DEBUG1(vc1DEBUG_PIC, "PS_HEIGHT: %d\n", Value);
            pParams->sPanScanParams.sPanScanWindow[Count].Height = (UHWD16)Value;
            COVERAGE("7.1.1.10");
        }
    }

    return(vc1_ResultOK);
}






/*
 * Description:
 * Unpack the quantisation parameters of a bitstream.
 *
 * Remarks:
 * None.
 *
 * Inputs:
 * pParams      - pointer to the picture layer parameters structure.
 * pSeqParams   - pointer to the sequence layer parameters structure.
 * pBitstream   - pointer to struct representing the current bitstream
 *
 * Outputs:
 * pParams      - updated with read data
 * pBitstream   - updated with new position
 *
 * Return Value:
 * Standard vc1_eResult result. See enumeration for possible result codes.
 * 
 */


static vc1_eResult vc1DECPIC_UnpackQuantizationParams(
    vc1DEC_sPictureLayerParams * pParams,
    vc1_sSequenceLayer      * pSeqParams,
    vc1DEC_sBitstream * pBitstream)
{
    WORD32 Value;

    /* Picture quantizer index */
    Value = vc1DECBIT_GetBits(pBitstream, VC1_BITS_PQINDEX);
    if(VC1DECBIT_EOF == Value)
    {
        return(vc1_ResultBufferExhausted);
    }
    DEBUG1(vc1DEBUG_PIC, "PQINDEX: %d\n", Value);
    COVERAGE("7.1.1.19");
    pParams->PQIndex = (UBYTE8)Value;

    if(vc1_QuantizerExplicit != pSeqParams->eQuantizer)
    {
        vc1_sQuant sQuant;

        if(0 == Value)
        {
            FATAL("PQINDEX is reserved or forbidden: %d\n", Value);
            return(vc1_ResultFatal);
        }

        vc1IQUANT_GetQuantizer(&sQuant, Value, pSeqParams->eQuantizer);

        pParams->PQuant     = sQuant.Quant;
        if (sQuant.NonUniform)
        {
            pParams->eQuantizer = vc1_QuantizerNonUniform;
        }
        else
        {
            pParams->eQuantizer = vc1_QuantizerUniform;
        }
    }
    else
    {
        /*
         * The quantizer is explicit, and signaled on a per frame basis
         * The signal is later in the bitstream
         */
    }


    pParams->HalfQPStep = 0;
    if(8 >= pParams->PQIndex)
    {
        /* Half QP step */
        Value = vc1DECBIT_GetBits(pBitstream, VC1_BITS_HALFQP);
        if(VC1DECBIT_EOF == Value)
        {
            return(vc1_ResultBufferExhausted);
        }
        DEBUG1(vc1DEBUG_PIC, "HALFQP: %d\n", Value);
        COVERAGE("7.1.1.20");
        pParams->HalfQPStep = (FLAG)Value;

        /* Copy this into each macroblock struct later */
    }


    if(vc1_QuantizerExplicit == pSeqParams->eQuantizer)
    {
        vc1_sQuant sQuant;

        /* Picture quantizer type */
        Value = vc1DECBIT_GetBits(pBitstream, VC1_BITS_PQUANTIZER);
        if(VC1DECBIT_EOF == Value)
        {
            return(vc1_ResultBufferExhausted);
        }
        DEBUG1(vc1DEBUG_PIC, "PQUANTIZER: %d\n", Value);
        COVERAGE("7.1.1.21");

        /* Now that the quantizer type is known, determine the quantizer level */
        if(FALSE == Value)
        {
            vc1IQUANT_GetQuantizer(&sQuant, pParams->PQIndex, vc1_QuantizerNonUniform);

            pParams->PQuant     = sQuant.Quant;
            pParams->eQuantizer = vc1_QuantizerNonUniform;
        }
        else
        {
            vc1IQUANT_GetQuantizer(&sQuant, pParams->PQIndex, vc1_QuantizerUniform);

            pParams->PQuant     = sQuant.Quant;
            pParams->eQuantizer = vc1_QuantizerUniform;
        }
    }

    DEBUG3(vc1DEBUG_PIC, "PQUANT=%d, HalfStep=%d, NonUniform=%d\n",
        pParams->PQuant, pParams->HalfQPStep, 3-pParams->eQuantizer);

    return(vc1_ResultOK);
}






/*
 * Description:
 * Unpack the VOPDQUANT bitstream elements.
 *
 * Remarks:
 * None.
 *
 * Inputs:
 * pParams      - pointer to the picture layer parameters structure.
 * pSeqParams   - pointer to the sequence layer parameters structure.
 * pBitstream   - pointer to struct representing the current bitstream
 *
 * Outputs:
 * pParams      - updated with read data
 * pBitstream   - updated with new position
 *
 * Return Value:
 * Standard vc1_eResult result. See enumeration for possible result codes.
 * 
 */

static vc1_eResult vc1DECPIC_UnpackVOPDQUANTParams(
    vc1DEC_sPictureLayerParams * pParams,
    vc1_sSequenceLayer      * pSeqParams,
    vc1DEC_sBitstream       * pBitstream
)
{
    WORD32 Value;

    COVERAGE("7.1.1.33");

    if(1 == pSeqParams->DQuant || 3 == pSeqParams->DQuant)
    {
        /* Macroblock quantisation frame */
        Value = vc1DECBIT_GetBits(pBitstream, VC1_BITS_DQUANTFRM);
        if(VC1DECBIT_EOF == Value)
        {
            return(vc1_ResultBufferExhausted);
        }
        DEBUG1(vc1DEBUG_PIC, "DQUANTFRM: %d\n", Value);
        COVERAGE("7.1.1.34");
        pParams->DQuantFrame = (FLAG)Value;

        if(TRUE == Value)
        {
            WORD32 DQProfile;

            /* Macroblock quantization profile */
            Value = vc1DECBIT_GetBits(pBitstream, VC1_BITS_DQPROFILE);
            if(VC1DECBIT_EOF == Value)
            {
                return(vc1_ResultBufferExhausted);
            }
            DEBUG1(vc1DEBUG_PIC, "DQPROFILE: %d\n", Value);
            DQProfile = (UBYTE8)Value;

            if(0 == DQProfile)
            {
                /* MB quantisation selection on all four edges */
                pParams->eQuantMode = vc1_QuantModeAllEdges;
            }
            else if(1 == DQProfile)
            {
                /* MB quantisation selection on pair of edges */
                Value = vc1DECBIT_GetBits(pBitstream, VC1_BITS_DQDBEDGE);
                if(VC1DECBIT_EOF == Value)
                {
                    return(vc1_ResultBufferExhausted);
                }
                DEBUG1(vc1DEBUG_PIC, "DQDBEDGE: %d\n", Value);
                pParams->eQuantMode = (vc1_eQuantMode)(Value + vc1_QuantModeLeftTop);
            }
            else if(2 == DQProfile)
            {
                /* MB quantisation selection on single edges */
                Value = vc1DECBIT_GetBits(pBitstream, VC1_BITS_DQSBEDGE);
                if(VC1DECBIT_EOF == Value)
                {
                    return(vc1_ResultBufferExhausted);
                }
                DEBUG1(vc1DEBUG_PIC, "DQSBEDGE: %d\n", Value);
                pParams->eQuantMode = (vc1_eQuantMode)(Value + vc1_QuantModeLeft);
            }
            else if(3 == DQProfile)
            {
                /* Macroblock quantization bi-level */
                Value = vc1DECBIT_GetBits(pBitstream, VC1_BITS_DQBILEVEL);
                if(VC1DECBIT_EOF == Value)
                {
                    return(vc1_ResultBufferExhausted);
                }
                DEBUG1(vc1DEBUG_PIC, "DQBILEVEL: %d\n", Value);

                if(FALSE == Value)
                {
                    pParams->eQuantMode = vc1_QuantModeMBAny;
                }
                else
                {
                    pParams->eQuantMode = vc1_QuantModeMBDual;
                }
            }
        }
        else
        {
            pParams->eQuantMode = vc1_QuantModeDefault;
            return(vc1_ResultOK);
        }             
    }
    else if (pSeqParams->DQuant==2)
    {
        pParams->eQuantMode = vc1_QuantModeAllEdges;
    }
    
    if(1 == pSeqParams->DQuant || 2 == pSeqParams->DQuant || 3 == pSeqParams->DQuant)
    {
        if( (vc1_QuantModeDefault != pParams->eQuantMode) &&
            (vc1_QuantModeMBAny   != pParams->eQuantMode))
        {
            /* PQuant differential or escape code */
            Value = vc1DECBIT_GetBits(pBitstream, VC1_BITS_PQDIFF);
            if(VC1DECBIT_EOF == Value)
            {
                return(vc1_ResultBufferExhausted);
            }
            DEBUG1(vc1DEBUG_PIC, "PQDIFF: %d\n", Value);

            if(7 != Value)
            {
                pParams->AltPQuant = (UBYTE8)(pParams->PQuant + Value + 1);
            }
            else
            {
                /* Alternative macroblock quantization level */
                Value = vc1DECBIT_GetBits(pBitstream, VC1_BITS_ABSPQ);
                if(VC1DECBIT_EOF == Value)
                {
                    return(vc1_ResultBufferExhausted);
                }
                DEBUG1(vc1DEBUG_PIC, "ABSPQ: %d\n", Value);
                pParams->AltPQuant = (UBYTE8)Value;
            }
        }
    }
    else
    {
        pParams->AltPQuant  = 0;
        pParams->eQuantMode = vc1_QuantModeDefault;
    }

    DEBUG2(vc1DEBUG_PIC, "QuantMode = %s (%d)\n",
        vc1DEBUG_QuantMode[pParams->eQuantMode],
        pParams->eQuantMode);

    return(vc1_ResultOK);
}


/*
 * Description:
 * Unpack the picture layer of the bitstream, and put the values into a struct
 *
 * Remarks:
 * Unpacks progressive I, P, B, BI picture layer for simple and main profiles
 *
 * Inputs:
 * pState     - pointer to decoder state structure, into which data will be put
 * pBitstream - pointer to structure representing position in bitstream
 *
 * Outputs:
 * pBitstream - updated with new position in bitstream
 * pState     - filled out with information from bitstream
 *
 * Return Value:
 * Standard vc1_eResult result. See enumeration for possible result codes.
 *
 */

static vc1_eResult vc1DECPIC_UnpackPictureLayerSimpleMain(vc1DEC_sState *pState,
                                                          vc1DEC_sBitstream * pBitstream)
{
    UWORD32     Value;
    vc1DEC_sPictureLayerParams *pParams    = &pState->sPicParams;
    vc1_sSequenceLayer      *pSeqParams    = &pState->sSeqParams;
    vc1_sPosition *pPosition               = &pState->sPosition;
    vc1_eResult eResult;

    /* test to see if this is a skipped frame */
    if(vc1DECBIT_BitCountGet(pBitstream) <= 8)
    {
        COVERAGE("8.3.1");
        DEBUG0(vc1DEBUG_PIC, "Picture is skipped\n");
        pParams->ePictureType[0] = vc1_PictureTypeSkipped;

        /* discard the remaining bits */
        vc1DECBIT_GetBits(pBitstream, vc1DECBIT_BitCountGet(pBitstream));

        return(vc1_ResultOK);
    }

    pState->BitplaneCodingUsed = FALSE;

    if(TRUE == pSeqParams->FrameInterpolationFlag)
    {
        /* Frame smoothness interpolate */
        Value = vc1DECBIT_GetBits(pBitstream, VC1_BITS_INTERPFRM);
        if(VC1DECBIT_EOF == Value)
        {
            return(vc1_ResultBufferExhausted);
        }
        pParams->FrameInterpolationHint = (FLAG)Value;
        DEBUG1(vc1DEBUG_PIC, "INTERPFRM: %d\n", Value);
        COVERAGE("7.1.1.11");
    }
    else
    {
        pParams->FrameInterpolationHint = FALSE;
    }

    if((vc1_ProfileSimple == pSeqParams->eProfile) || (vc1_ProfileMain == pSeqParams->eProfile))
    {
        /* Frame count (not used by the decoder) */
        Value = vc1DECBIT_GetBits(pBitstream, VC1_BITS_FRMCNT);
        if(VC1DECBIT_EOF == Value)
        {
            return(vc1_ResultBufferExhausted);
        }
        DEBUG1(vc1DEBUG_PIC, "FRMCNT: %d\n", Value);
        COVERAGE("7.1.1.12");
    }

    if(TRUE == pSeqParams->RangeRedFlag)
    {
        int Scale;

        /* Range reduction frame */
        Value = vc1DECBIT_GetBits(pBitstream, VC1_BITS_RANGE_RED_FRM);
        if(VC1DECBIT_EOF == Value)
        {
            return(vc1_ResultBufferExhausted);
        }
        Scale = 8 + 8*Value;
        DEBUG2(vc1DEBUG_PIC, "RANGE_RED_FRM: %d (Sclale up = %d/8)\n", Value, Scale);
        COVERAGE("7.1.1.13");

        pPosition->RangeYScale  = (UBYTE8)Scale;
        pPosition->RangeUVScale = (UBYTE8)Scale;
    }
    else
    {
        pPosition->RangeYScale  = 8;
        pPosition->RangeUVScale = 8;
    }

    /* Picture type */
    Value = vc1DECBIT_GetBits(pBitstream, VC1_BITS_PTYPE_1);
    if(VC1DECBIT_EOF == Value)
    {
        return(vc1_ResultBufferExhausted);
    }
    DEBUG1(vc1DEBUG_PIC, "PTYPE_1: %d\n", Value);
    COVERAGE("7.1.1.14");
    if(1 == Value)
    {
        pParams->ePictureType[0] = vc1_PictureTypeP;
    }
    else
    {
        if(0 == pSeqParams->MaxBFrames)
        {
            /* 1-bit code */
            pParams->ePictureType[0] = vc1_PictureTypeI;
        }
        else
        {
            /* 2-bit code */
            Value = vc1DECBIT_GetBits(pBitstream, VC1_BITS_PTYPE_2);
            if(VC1DECBIT_EOF == Value)
            {
                return(vc1_ResultBufferExhausted);
            }
            DEBUG1(vc1DEBUG_PIC, "PTYPE_2: %d\n", Value);
            if(0 == Value)
            {
                pParams->ePictureType[0] = vc1_PictureTypeB;
            }
            else
            {
                pParams->ePictureType[0] = vc1_PictureTypeI;
            }
        }
    }
    DEBUG1(vc1DEBUG_PIC, "Picture type = %s\n", vc1DEBUG_PictureType[pParams->ePictureType[0]]);

    if(vc1_PictureTypeB == pParams->ePictureType[0])
    {
        /* B fraction */
        Value = vc1DECBIT_GetVLC(pBitstream, vc1DECPIC_B_Fraction_Table);
        if(VC1DECBIT_EOF == Value)
        {
            return(vc1_ResultBufferExhausted);
        }

        pPosition->BFraction = (UBYTE8)Value;

        DEBUG3(vc1DEBUG_PIC, "BFRACTION: %d/%d (code %d)\n",
            vc1GENTAB_pBFraction[Value].Numerator,
            vc1GENTAB_pBFraction[Value].Denominator,
            Value);

        COVERAGE("7.1.1.15");

        if(0 == vc1GENTAB_pBFraction[Value].Denominator)    /* special cases */
        {
            if(1 == vc1GENTAB_pBFraction[Value].Numerator)
            {
                FATAL("UnpackPictureLayerSimpleMain(): Invalid BFRACTION\n");
                return(vc1_ResultFatal);
            }
            else if(2 == vc1GENTAB_pBFraction[Value].Numerator)
            {
                /* frame is type BI */
                pParams->ePictureType[0] = vc1_PictureTypeBI;
                COVERAGE("8.2.1");
            }
        }
    }

    if (vc1_PictureTypeIsIntra(pParams->ePictureType[0]))
    {
        /* I or BI have a Buffer Fullness field BF */
        Value = vc1DECBIT_GetBits(pBitstream, VC1_BITS_BF);
        if(VC1DECBIT_EOF == Value)
        {
            return(vc1_ResultBufferExhausted);
        }
        DEBUG1(vc1DEBUG_PIC, "BF: %d\n", Value);
        COVERAGE("7.1.1.16");
        pParams->BufferFullness = (UBYTE8)Value;
    }


    eResult = vc1DECPIC_UnpackQuantizationParams(pParams, pSeqParams, pBitstream);
    if(vc1_ResultOK != eResult)
    {
        return(eResult);
    }

    if (pSeqParams->ExtendedMV)
    {
        /* Extended motion vector range */
        Value = vc1DECBIT_GetVLC(pBitstream, vc1DECPIC_Motion_Vector_Range_Table);
        if(VC1DECBIT_EOF == Value)
        {
            return(vc1_ResultBufferExhausted);
        }
        DEBUG1(vc1DEBUG_PIC, "MVRANGE: %d\n", Value);
        COVERAGE("7.1.1.18");
        pPosition->eMVRange = (vc1_eMVRange)Value;
    }
    else
    {
        pPosition->eMVRange = (vc1_eMVRange)vc1_MVRange64_32;
    }


    if( (vc1_PictureTypeI == pParams->ePictureType[0])  || 
        (vc1_PictureTypeP == pParams->ePictureType[0])  )
    {
        if(TRUE == pSeqParams->MultiResCoding)
        {
            /* Progressive picture resolution index */
            Value = vc1DECBIT_GetBits(pBitstream, VC1_BITS_RESPIC);
            if(VC1DECBIT_EOF == Value)
            {
                return(vc1_ResultBufferExhausted);
            }
            DEBUG2(vc1DEBUG_PIC, "RESPIC: Upscale %s (%d)\n", vc1DEBUG_PictureRes[Value], Value);
            COVERAGE("7.1.1.23");

            if (vc1_PictureTypeP == pParams->ePictureType[0])
            {
                /* Check that the P-picture value matches the previously set value */
                if (pPosition->ePictureRes != (vc1_ePictureRes) Value)
                {
                    WARN("P picture RESPIC does not match I picture RESPIC");
                }
                COVERAGE("8.3.4.2");
            }
            pPosition->ePictureRes = (vc1_ePictureRes)Value;
        }
        else
        {
            pPosition->ePictureRes = (vc1_ePictureRes)vc1_PictureRes1x1;
        }

        if (pPosition->ePictureRes!=vc1_PictureRes1x1 && pSeqParams->DQuant!=0)
        {
            WARN("Simple/Main Profile - Resolution Scale used with DQUANT!=0 (illegal)\n");
        }
    }


    /* determine the image size in MBs now - we have enough information */
    vc1DECPIC_SetDimensionsInMB(pState, pPosition);


    if(vc1_PictureTypeP == pParams->ePictureType[0])
    {
        /* Motion vector mode 1 */
        if(12 < pParams->PQuant)
        {
            Value = vc1DECBIT_GetVLC(   pBitstream, 
                                        vc1DECPIC_P_Picture_Low_Rate_Motion_Vector_Mode);
        }
        else
        {
            Value = vc1DECBIT_GetVLC(   pBitstream, 
                                        vc1DECPIC_P_Picture_High_Rate_Motion_Vector_Mode);
        }
        if(VC1DECBIT_EOF == Value)
        {
            return(vc1_ResultBufferExhausted);
        }
        DEBUG2(vc1DEBUG_PIC, "MVMODE1: %s (%d)\n", vc1DEBUG_MVMode[Value], Value);
        COVERAGE("7.1.1.22");

        if(vc1_MVModeIntensityCompensation == Value)
        {
            pParams->sIC[0].IntensityCompFlag = TRUE;

            if (pPosition->eProfile == vc1_ProfileSimple)
            {
                FATAL("Intensity compensation enabled in simple profile\n");
                return vc1_ResultInvalidPicture;
            }

            /* Motion vector mode 2*/
            if(12 < pParams->PQuant)
            {
                Value = vc1DECBIT_GetVLC(   pBitstream,
                                            vc1DECPIC_P_Picture_Low_Rate_Motion_Vector_Mode_2);
            }
            else
            {
                Value = vc1DECBIT_GetVLC(   pBitstream,
                                            vc1DECPIC_P_Picture_High_Rate_Motion_Vector_Mode_2);
            }
            if(VC1DECBIT_EOF == Value)
            {
                return(vc1_ResultBufferExhausted);
            }
            pPosition->eMVMode = (vc1_eMVMode)Value;
            DEBUG2(vc1DEBUG_PIC, "MVMODE2: %s (%d)\n", vc1DEBUG_MVMode[Value], Value);
            COVERAGE_NONINT_INT("7.1.1.23","9.1.1.35");

            /* Luminance scale */
            Value = vc1DECBIT_GetBits(pBitstream, VC1_BITS_LUMSCALE);
            if(VC1DECBIT_EOF == Value)
            {
                return(vc1_ResultBufferExhausted);
            }
            pParams->sIC[0].LuminanceScale = (UBYTE8)Value;
            DEBUG1(vc1DEBUG_PIC, "LUMSCALE: %d\n", Value);
            COVERAGE_NONINT_INT("7.1.1.28","9.1.1.38");

            /* Luminance shift */
            Value = vc1DECBIT_GetBits(pBitstream, VC1_BITS_LUMSHIFT);
            if(VC1DECBIT_EOF == Value)
            {
                return(vc1_ResultBufferExhausted);
            }
            pParams->sIC[0].LuminanceShift = (UBYTE8)Value;
            DEBUG1(vc1DEBUG_PIC, "LUMSHIFT: %d\n", Value);
            COVERAGE_NONINT_INT("7.1.1.29","9.1.1.39");

            COVERAGE("8.3.4.3");    /* Now read all the relevant parameters */
        }
        else
        {
            pPosition->eMVMode = (vc1_eMVMode)Value;
            pParams->sIC[0].IntensityCompFlag = FALSE;
        }
    }
    else if(vc1_PictureTypeB == pParams->ePictureType[0])
    {
        /* Motion vector mode */
        Value = vc1DECBIT_GetBits(pBitstream, 1);
        if(VC1DECBIT_EOF == Value)
        {
            return(vc1_ResultBufferExhausted);
        }
        if (Value==0)
        {
            Value = vc1_MVMode1MVHalfPelBilinear;
        }
        else
        {
            Value = vc1_MVMode1MV;
        }
        pPosition->eMVMode = (vc1_eMVMode)Value;
        DEBUG2(vc1DEBUG_PIC, "MVMODE: %s (%d)\n", vc1DEBUG_MVMode[Value], Value);
        COVERAGE("7.1.1.26");

        /* turn off any intensity compensation */
        pParams->sIC[0].IntensityCompFlag = FALSE;
    }



    if( (vc1_PictureTypeB == pParams->ePictureType[0])  || 
        (vc1_PictureTypeP == pParams->ePictureType[0])  )
    {
        if(vc1_MVModeMixedMV == pPosition->eMVMode)
        {
            /* Motion vector type bitplane */
            DEBUG0(vc1DEBUG_PIC, "MVTYPEMB: Bitplane\n");
            eResult = vc1DECBITPL_ReadBitplane(pState, &pParams->sBPMotionVectorType, pBitstream);
            if(vc1_ResultOK != eResult)
            {
                return(eResult);
            }
            COVERAGE("7.1.1.30");
        }
    }


    if(vc1_PictureTypeB == pParams->ePictureType[0])
    {
        /* Direct macroblock bitplane */
        DEBUG0(vc1DEBUG_PIC, "DIRECTMB: Bitplane\n");
        eResult = vc1DECBITPL_ReadBitplane(pState, &pParams->sBPBFrameDirectMode, pBitstream);
        if(vc1_ResultOK != eResult)
        {
            return(eResult);
        }
        COVERAGE("7.1.1.25");
    }


    if( (vc1_PictureTypeB == pParams->ePictureType[0])  ||
        (vc1_PictureTypeP == pParams->ePictureType[0])  )
    {        
        /* Macroblock skip bitplane */
        DEBUG0(vc1DEBUG_PIC, "MBSKIP: Bitplane\n");
        eResult = vc1DECBITPL_ReadBitplane(pState, &pParams->sBPSkipMB, pBitstream);
        if(vc1_ResultOK != eResult)
        {
            return(eResult);
        }
        COVERAGE("7.1.1.24");

        /* Motion vector huffman table */
        Value = vc1DECBIT_GetBits(pBitstream, VC1_BITS_MVTAB);
        if(VC1DECBIT_EOF == Value)
        {
            return(vc1_ResultBufferExhausted);
        }
        switch(Value)
        {
            case 0:
                pParams->pMotionVectorTable = vc1DEC_Mot_Vector_Diff_VLC_0;
                break;

            case 1:
                pParams->pMotionVectorTable = vc1DEC_Mot_Vector_Diff_VLC_1;
                break;

            case 2:
                pParams->pMotionVectorTable = vc1DEC_Mot_Vector_Diff_VLC_2;
                break;

            case 3:
                pParams->pMotionVectorTable = vc1DEC_Mot_Vector_Diff_VLC_3; 
                break;
        }
        COVERAGE("8.3.4.5");
        DEBUG1(vc1DEBUG_PIC, "MVTAB: %d\n", Value);
        COVERAGE("7.1.1.27");


        /* Coded block pattern table */
        Value = vc1DECBIT_GetBits(pBitstream, VC1_BITS_CBPTAB);
        if(VC1DECBIT_EOF == Value)
        {
            return(vc1_ResultBufferExhausted);
        }
        switch(Value)
        {
            case 0:
                pParams->pCodedBlockPatternTable = vc1DEC_P_Picture_CBPCY_VLC_0;
                break;

            case 1:
                pParams->pCodedBlockPatternTable = vc1DEC_P_Picture_CBPCY_VLC_1;
                break;

            case 2:
                pParams->pCodedBlockPatternTable = vc1DEC_P_Picture_CBPCY_VLC_2;
                break;

            case 3:
                pParams->pCodedBlockPatternTable = vc1DEC_P_Picture_CBPCY_VLC_3;
                break;
        }
        COVERAGE("8.3.4.6");
        DEBUG1(vc1DEBUG_PIC, "CBPTAB: %d\n", Value);
        COVERAGE("7.1.1.28");

        eResult = vc1DECPIC_UnpackVOPDQUANTParams(pParams, pSeqParams, pBitstream);
        if(vc1_ResultOK != eResult)
        {
            return(eResult);
        }

        if(1 == pSeqParams->VSTransform)
        {            
            /* Macroblock level transform type flag */
            Value = vc1DECBIT_GetBits(pBitstream, VC1_BITS_TTMBF);
            if(VC1DECBIT_EOF == Value)
            {
                return(vc1_ResultBufferExhausted);
            }
            DEBUG1(vc1DEBUG_PIC, "TTMBF: %d\n", Value);
            COVERAGE_NONINT_INT("7.1.1.35","9.1.1.51");
            pParams->MBTransformTypeFlag = (FLAG)Value;

            if(1 == Value)
            {
                Value = vc1DECBIT_GetBits(pBitstream, VC1_BITS_TTFRM);
                if(VC1DECBIT_EOF == Value)
                {
                    return(vc1_ResultBufferExhausted);
                }
                DEBUG1(vc1DEBUG_PIC, "TTFRM: %d\n", Value);
                COVERAGE_NONINT_INT("7.1.1.36","9.1.1.52");
                switch(Value)
                {
                    case 0:
                        pParams->eFrameTransformType = vc1_BlkInter8x8;
                        break;

                    case 1:
                        pParams->eFrameTransformType = vc1_BlkInter8x4;
                        break;

                    case 2:
                        pParams->eFrameTransformType = vc1_BlkInter4x8;
                        break;

                    case 3:
                        pParams->eFrameTransformType = vc1_BlkInter4x4;
                        break;
                }
            }
            else
            {
                /*
                 * there's no frame level transform type - set to "any", 
                 * as it will be filled in later
                 */
                pParams->eFrameTransformType = vc1_BlkInterAny;
            }
        }
        else
        {
            /* Block transform type is fixed default */
            pParams->eFrameTransformType = vc1_BlkInter8x8;
        }
    }
    else
    {
        pParams->eQuantMode = vc1_QuantModeDefault;
    }


    /* Frame level transform AC coding set index */
    Value = vc1DECBIT_GetBits(pBitstream, VC1_BITS_TRANSACFRM_1);
    if(VC1DECBIT_EOF == Value)
    {
        return(vc1_ResultBufferExhausted);
    }
    DEBUG1(vc1DEBUG_PIC, "TRANSACFRM_1: %d\n", Value);
    if(0 == Value)
    {
        /* 1-bit code */
        pParams->FrameTransformACCodingSetIndex = 0;
    }
    else
    {
        /* 2-bit code */
        Value = vc1DECBIT_GetBits(pBitstream, VC1_BITS_TRANSACFRM_2);
        if(VC1DECBIT_EOF == Value)
        {
            return(vc1_ResultBufferExhausted);
        }
        DEBUG1(vc1DEBUG_PIC, "TRANSACFRM_2: %d\n", Value);
        if(0 == Value)
        {
            pParams->FrameTransformACCodingSetIndex = 1;
        }
        else
        {
            pParams->FrameTransformACCodingSetIndex = 2;
        }
    }
    COVERAGE("7.1.1.41");
    COVERAGE("8.1.1.1");

    if(TRUE == vc1_PictureTypeIsIntra(pParams->ePictureType[0]))
    {
        /* Frame level transform AC coding set index 2 */
        Value = vc1DECBIT_GetBits(pBitstream, VC1_BITS_TRANSACFRM2_1);
        if(VC1DECBIT_EOF == Value)
        {
            return(vc1_ResultBufferExhausted);
        }
        DEBUG1(vc1DEBUG_PIC, "TRANSACFRM2_1: %d\n", Value);
        if(0 == Value)
        {
            /* 1-bit code */
            pParams->FrameTransformACCodingSetIndex2 = 0;
        }
        else
        {
            /* 2-bit code */
            Value = vc1DECBIT_GetBits(pBitstream, VC1_BITS_TRANSACFRM2_2);
            if(VC1DECBIT_EOF == Value)
            {
                return(vc1_ResultBufferExhausted);
            }
            DEBUG1(vc1DEBUG_PIC, "TRANSACFRM2_2: %d\n", Value);
            if(0 == Value)
            {
                pParams->FrameTransformACCodingSetIndex2 = 1;
            }
            else
            {
                pParams->FrameTransformACCodingSetIndex2 = 2;
            }
        }
        COVERAGE("7.1.1.42");
    }


    /* Intra transform DC table */
    Value = vc1DECBIT_GetBits(pBitstream, VC1_BITS_TRANSDCTAB);
    if(VC1DECBIT_EOF == Value)
    {
        return(vc1_ResultBufferExhausted);
    }
    pParams->IntraTransformDCTable = (FLAG)Value;
    DEBUG1(vc1DEBUG_PIC, "TRANSDCTAB: %d\n", Value);
    COVERAGE("7.1.1.43");

    pState->sPicParams.eConditionalOverlap = vc1_CondOverAll;

    if( (vc1_PictureTypeB == pParams->ePictureType[0])    ||
        (pParams->PQuant < 9)                            ||
        (FALSE == pSeqParams->OverlappedTransformFlag) )
    {
        pState->sPicParams.eConditionalOverlap = vc1_CondOverNone;
    }
    DEBUG1(vc1DEBUG_PIC, "Overlap = %d\n", pParams->eConditionalOverlap);

    return(vc1_ResultOK);
}

typedef struct
{
    vc1_ePictureType  FirstField;
    vc1_ePictureType  SecondField;
} vc1DECPIC_sFieldTypes;

static vc1DECPIC_sFieldTypes vc1DECPIC_Field_Type_Table[8] =
{
    {vc1_PictureTypeI, vc1_PictureTypeI},
    {vc1_PictureTypeI, vc1_PictureTypeP},
    {vc1_PictureTypeP, vc1_PictureTypeI},
    {vc1_PictureTypeP, vc1_PictureTypeP},
    {vc1_PictureTypeB, vc1_PictureTypeB},
    {vc1_PictureTypeB, vc1_PictureTypeBI},
    {vc1_PictureTypeBI, vc1_PictureTypeB},
    {vc1_PictureTypeBI, vc1_PictureTypeBI}
};








/*
 * Description:
 * Unpack the advanced picture layer of the bitstream, and put the values into a struct
 *
 * Remarks:
 * Unpacks frame picture layer for advanced profile
 *
 * Inputs:
 * pState - decoder state structure, into which data will be put
 * pBitstream - structure representing position in bitstream
 *
 * Outputs:
 * pBitstream - updated with new position in bitstream
 * pState - filled out with information from bitstream
 *
 * Return Value:
 * Standard vc1_eResult result. See enumeration for possible result codes
 *
 */

static vc1_eResult vc1DECPIC_UnpackPictureLayerAdvanced(
    vc1DEC_sState * pState,
    vc1DEC_sBitstream * pBitstream
)
{
    WORD32     Value;

    vc1_sPosition *pPosition               = &pState->sPosition;
    vc1_sSequenceLayer      *pSeqParams    = &pState->sSeqParams;
    vc1DEC_sPictureLayerParams *pParams    = &pState->sPicParams;


    /* Picture type */
    if(vc1_InterlacedField == pPosition->ePictureFormat)
    {
        Value = vc1DECBIT_GetBits(pBitstream, VC1_BITS_FPTYPE);
        if(VC1DECBIT_EOF == Value)
        {
            return(vc1_ResultBufferExhausted);
        }
        pParams->ePictureType[0] = vc1DECPIC_Field_Type_Table[Value].FirstField;
        pParams->ePictureType[1] = vc1DECPIC_Field_Type_Table[Value].SecondField;
        DEBUG3(vc1DEBUG_PIC, "FPTYPE: %s/%s (%d)\n",
            vc1DEBUG_PictureType[pParams->ePictureType[0]],
            vc1DEBUG_PictureType[pParams->ePictureType[1]],
            Value);
        COVERAGE("9.1.1.2");
    }
    else
    {
        Value = vc1DECBIT_GetVLC(pBitstream, vc1DECPIC_Picture_Type_Table);
        if(VC1DECBIT_EOF == Value)
        {
            return(vc1_ResultBufferExhausted);
        }
        pParams->ePictureType[0] = (vc1_ePictureType)Value;
        pParams->ePictureType[1] = (vc1_ePictureType)Value;
        DEBUG2(vc1DEBUG_PIC, "PTYPE: %s (%d)\n",
            vc1DEBUG_PictureType[pParams->ePictureType[0]],
            Value);

        COVERAGE_NONINT_INT("7.1.1.14","9.1.1.3");
    }


    if(vc1_PictureTypeSkipped != pParams->ePictureType[0])
    {
        if(TRUE == pSeqParams->FrameCounterFlag)
        {
            /* Temporal reference frame counter */
            Value = vc1DECBIT_GetBits(pBitstream, VC1_BITS_TFCNTR);
            if(VC1DECBIT_EOF == Value)
            {
                return(vc1_ResultBufferExhausted);
            }
            DEBUG1(vc1DEBUG_PIC, "TFCNTR: %d\n", Value);
            COVERAGE("7.1.1.1");
            pParams->TemporalRefFrameCounter = (UBYTE8)Value;
        }
    }

    if(TRUE == pSeqParams->Interlace && FALSE == pSeqParams->PsF)
    {
        if(TRUE == pSeqParams->PullDownFlag)
        {
            /* Top field first */
            Value = vc1DECBIT_GetBits(pBitstream, VC1_BITS_TFF);
            if(VC1DECBIT_EOF == Value)
            {
                return(vc1_ResultBufferExhausted);
            }
            DEBUG1(vc1DEBUG_PIC, "TFF: %d\n", Value);
            COVERAGE("7.1.1.3");
            pParams->TopFieldFirst = (FLAG)Value;

            
            /* Repeat first field */
            Value = vc1DECBIT_GetBits(pBitstream, VC1_BITS_RFF);
            if(VC1DECBIT_EOF == Value)
            {
                return(vc1_ResultBufferExhausted);
            }
            DEBUG1(vc1DEBUG_PIC, "RFF: %d\n", Value);
            COVERAGE("7.1.1.4");
            pParams->RepeatFirstField = (FLAG)Value;
        }
        else
        {
            /* TFF defaults to TRUE if PULLDOWN unset */
            pParams->TopFieldFirst    = TRUE;
            pParams->RepeatFirstField = FALSE;
        }
    }
    else
    {
        if(TRUE == pSeqParams->PullDownFlag)
        {
            /* Repeat frame count */
            Value = vc1DECBIT_GetBits(pBitstream, VC1_BITS_RPTFRM);
            if(VC1DECBIT_EOF == Value)
            {
                return(vc1_ResultBufferExhausted);
            }
            DEBUG1(vc1DEBUG_PIC, "RPTFRM: %d\n", Value);
            COVERAGE("7.1.1.5");
            pParams->RepeatFrameCount = (UBYTE8)Value;
        }
        else
        {
            pParams->RepeatFrameCount = 0;
        }
    }

    if(TRUE == pSeqParams->PanScanFlag)
    {
        vc1_eResult eResult = vc1DECPIC_UnpackPanScanParams(    &pState->sSeqParams,
                                                                pParams,
                                                                pBitstream);
        if(vc1_ResultOK != eResult)
        {
            return(eResult);
        }
    }
    else
    {
        pState->sSeqParams.NumPanScanWin = 0;
    }

    if(vc1_PictureTypeSkipped == pParams->ePictureType[0])
    {
        /* The picture is skipped - no more data in the bitstream for this frame */

        /* Zero MV history buffer */
        memset((char *)(pState->pMVHistBuffer),
               0,
               sizeof(vc1_sMotionHist) * pPosition->WidthMB * pPosition->HeightMB);

        return(vc1_ResultOK);
    }

    /* Rounding control */
    Value = vc1DECBIT_GetBits(pBitstream, VC1_BITS_RNDCTRL);
    if(VC1DECBIT_EOF == Value)
    {
        return(vc1_ResultBufferExhausted);
    }
    DEBUG1(vc1DEBUG_PIC, "RNDCTRL: %d\n", Value);
    COVERAGE("7.1.1.17");
    COVERAGE("9.1.1.14");
    pParams->sInterpolate.RndCtrl = (FLAG)Value;
    if (vc1_PictureTypeIsIntra(pParams->ePictureType[0]))
    {
        /* RNDCTRL must be 0 for I or BI frames */
        if (Value!=0)
        {
            WARN("RNDCTRL should be 0 for I,BI frames in advanced profile\n");
        }
    }

    if (pSeqParams->Interlace==1)
    {
        /* UV sampling mode */
        Value = vc1DECBIT_GetBits(pBitstream, VC1_BITS_UVSAMP);
        if(VC1DECBIT_EOF == Value)
        {
            return(vc1_ResultBufferExhausted);
        }
        DEBUG1(vc1DEBUG_PIC, "UVSAMP: %d\n", Value);
        COVERAGE("7.1.1.13");
        COVERAGE("9.1.1.15");
        pParams->UVSampleMode = (FLAG)Value;
    }
    else
    {
        pParams->UVSampleMode = 0;
    }

    if( (TRUE == pSeqParams->FrameInterpolationFlag)        && 
        (vc1_ProgressiveFrame == pPosition->ePictureFormat) )
    {
        Value = vc1DECBIT_GetBits(pBitstream, VC1_BITS_INTERPFRM);
        if(VC1DECBIT_EOF == Value)
        {
            return(vc1_ResultBufferExhausted);
        }
        DEBUG1(vc1DEBUG_PIC, "INTERPFRM: %d\n", Value);
        COVERAGE("7.1.1.11");
        pParams->FrameInterpolationHint = (FLAG)Value;
    }
    else
    {
        pParams->FrameInterpolationHint = 0;
    }

    if(vc1_InterlacedFrame != pPosition->ePictureFormat)
    {
        if( (pParams->ePictureType[0] == vc1_PictureTypeB)  ||
            (pPosition->ePictureFormat==vc1_InterlacedField &&
             pParams->ePictureType[0] == vc1_PictureTypeBI)  )
        {
            /* BFRACTION */
            Value = vc1DECBIT_GetVLC(pBitstream, vc1DECPIC_B_Fraction_Table);
            if(VC1DECBIT_EOF == Value)
            {
                return(vc1_ResultBufferExhausted);
            }

            pPosition->BFraction = (UBYTE8)Value;

            DEBUG3(vc1DEBUG_PIC, "BFRACTION: %d/%d (code %d)\n",
                vc1GENTAB_pBFraction[Value].Numerator,
                vc1GENTAB_pBFraction[Value].Denominator,
                Value);

            COVERAGE("7.1.1.15");

            if(0 == vc1GENTAB_pBFraction[Value].Denominator)    /* special cases */
            {
                FATAL("UnpackPictureLayerAdvanced(): Invalid BFRACTION - %d\n", Value);
                return(vc1_ResultFatal);
            }
        }
    }

    if(vc1_InterlacedField == pPosition->ePictureFormat)
    {
        if(TRUE == pSeqParams->RefDistFlag)
        {
            if( (TRUE == vc1_PictureTypeIsRef(pParams->ePictureType[0]))    || 
                (TRUE == vc1_PictureTypeIsRef(pParams->ePictureType[1]))    )
            {
                int Count;

                /* Get first two bits, and determine how to count reference frames */
                Value = vc1DECBIT_GetBits(pBitstream, 2);
                if(VC1DECBIT_EOF == Value)
                {
                    return(vc1_ResultBufferExhausted);
                }

                if(Value < 3)
                {
                    pPosition->pReferenceOld->RefDist = (UBYTE8)Value;
                }
                else
                {
                    Count = 2;

                    /* Count frames, one bit at a time */
                    do
                    {
                        Value = vc1DECBIT_GetBits(pBitstream, 1);
                        if(VC1DECBIT_EOF == Value)
                        {
                            return(vc1_ResultBufferExhausted);
                        }
                        Count++;
                    } while(1 == Value);

                    pPosition->pReferenceOld->RefDist = (UBYTE8)Count;
                }

                DEBUG1(vc1DEBUG_PIC, "REFDIST: %d\n", pPosition->pReferenceOld->RefDist);
                COVERAGE("9.1.1.16");
            }
        }
        else
        {
            /* Default if not specified */
            pPosition->pReferenceOld->RefDist = 0;
        }
    }
    else
    {
        pPosition->pReferenceOld->RefDist = 0;
    }

    return(vc1_ResultOK);
}









/*
 * Description:
 * Unpack the picture layer of the bitstream, and put the values into a struct
 *
 * Remarks:
 * Unpacks P and B picture layer for advanced profile
 *
 * Inputs:
 * pState - decoder state structure, into which data will be put
 * pBitstream - structure representing position in bitstream
 *
 * Outputs:
 * pBitstream - updated with new position in bitstream
 * pState - filled out with information from bitstream
 *
 * Return Value:
 * Standard vc1_eResult result. See enumeration for possible result codes
 *
 */

static vc1_eResult vc1DECPIC_UnpackPictureLayerPBAdvanced(
    vc1DEC_sState * pState,
    vc1DEC_sBitstream * pBitstream
)
{
    vc1DEC_sPictureLayerParams *pParams    = &pState->sPicParams;
    vc1_sSequenceLayer      *pSeqParams    = &pState->sSeqParams;
    vc1_sPosition *pPosition               = &pState->sPosition;
    vc1_eResult eResult;
    WORD32 Value;

    /* Quantisation parameters */
    eResult = vc1DECPIC_UnpackQuantizationParams(pParams, pSeqParams, pBitstream);
    if(vc1_ResultOK != eResult)
    {
        return(eResult);
    }

    /* Post processing */
    if(TRUE == pSeqParams->PostProcessingFlag)
    {
        Value = vc1DECBIT_GetBits(pBitstream, VC1_BITS_POSTPROC);
        if(VC1DECBIT_EOF == Value)
        {
            return(vc1_ResultBufferExhausted);
        }
        DEBUG1(vc1DEBUG_PIC, "POSTPROC: %d\n", Value);
        COVERAGE("7.1.1.40");
        pParams->ePostProcessing = (vc1_ePostProcessing)Value;
    }
    else
    {
        pParams->ePostProcessing = vc1_PostProcessingNone;
    }

    if(1 == pSeqParams->ExtendedMV)
    {
        /* Extended motion vector range */
        Value = vc1DECBIT_GetVLC(pBitstream, vc1DECPIC_Motion_Vector_Range_Table);
        if(VC1DECBIT_EOF == Value)
        {
            return(vc1_ResultBufferExhausted);
        }
        DEBUG1(vc1DEBUG_PIC, "MVRANGE: %d\n", Value);
        COVERAGE("9.1.1.30");
        pPosition->eMVRange = (vc1_eMVRange)Value;
    }
    else
    {
        pPosition->eMVRange = vc1_MVRange64_32;
    }

    /* Motion vector mode 1 */
    if(vc1_PictureTypeP == pParams->ePictureType[0])
    {
        if(12 < pParams->PQuant)
        {
            Value = vc1DECBIT_GetVLC(   pBitstream,
                                        vc1DECPIC_P_Picture_Low_Rate_Motion_Vector_Mode);
        }
        else
        {
            Value = vc1DECBIT_GetVLC(   pBitstream,
                                        vc1DECPIC_P_Picture_High_Rate_Motion_Vector_Mode);
        }
        if(VC1DECBIT_EOF == Value)
        {
            return(vc1_ResultBufferExhausted);
        }
        DEBUG2(vc1DEBUG_PIC, "MVMODE1: %s (%d)\n", vc1DEBUG_MVMode[Value], Value);
    }
    else /* B picture */
    {
        Value = vc1DECBIT_GetBits(pBitstream, 1);
        if(VC1DECBIT_EOF == Value)
        {
            return(vc1_ResultBufferExhausted);
        }
        if (Value==0)
        {
            Value = vc1_MVMode1MVHalfPelBilinear;
        }
        else
        {
            Value = vc1_MVMode1MV;
        }
        DEBUG2(vc1DEBUG_PIC, "MVMODE: %s (%d)\n", vc1DEBUG_MVMode[Value], Value);
    }
    COVERAGE("7.1.1.22");


    if(vc1_MVModeIntensityCompensation == Value)
    {
        pParams->sIC[0].IntensityCompFlag = TRUE;

        /* Motion vector mode 2*/
        if(12 < pParams->PQuant)
        {
            Value = vc1DECBIT_GetVLC(   pBitstream,
                                        vc1DECPIC_P_Picture_Low_Rate_Motion_Vector_Mode_2);
        }
        else
        {
            Value = vc1DECBIT_GetVLC(   pBitstream,
                                        vc1DECPIC_P_Picture_High_Rate_Motion_Vector_Mode_2);
        }
        if(VC1DECBIT_EOF == Value)
        {
            return(vc1_ResultBufferExhausted);
        }
        pPosition->eMVMode = (vc1_eMVMode)Value;
        DEBUG2(vc1DEBUG_PIC, "MVMODE2: %s (%d)\n", vc1DEBUG_MVMode[Value], Value);
        COVERAGE_NONINT_INT("7.1.1.23","9.1.1.35");

        /* Luminance scale */
        Value = vc1DECBIT_GetBits(pBitstream, VC1_BITS_LUMSCALE);
        if(VC1DECBIT_EOF == Value)
        {
            return(vc1_ResultBufferExhausted);
        }
        pParams->sIC[0].LuminanceScale = (UBYTE8)Value;
        DEBUG1(vc1DEBUG_PIC, "LUMSCALE: %d\n", Value);
        COVERAGE_NONINT_INT("7.1.1.24","9.1.1.38");

        /* Luminance shift */
        Value = vc1DECBIT_GetBits(pBitstream, VC1_BITS_LUMSHIFT);
        if(VC1DECBIT_EOF == Value)
        {
            return(vc1_ResultBufferExhausted);
        }
        pParams->sIC[0].LuminanceShift = (UBYTE8)Value;
        DEBUG1(vc1DEBUG_PIC, "LUMSHIFT: %d\n", Value);
        COVERAGE_NONINT_INT("7.1.1.25","9.1.1.39");

        COVERAGE("8.3.4.3");    /* Now read all the relevant parameters */
    }
    else
    {
        pPosition->eMVMode = (vc1_eMVMode)Value;
        pParams->sIC[0].IntensityCompFlag = FALSE;
    }

    if(vc1_MVModeMixedMV == pPosition->eMVMode)
    {
        /* Motion vector type bitplane */
        DEBUG0(vc1DEBUG_PIC, "MVTYPEMB: Bitplane\n");
        eResult = vc1DECBITPL_ReadBitplane(pState, &pParams->sBPMotionVectorType, pBitstream);
        if(vc1_ResultOK != eResult)
        {
            return(eResult);
        }
    }

    if(vc1_PictureTypeB == pParams->ePictureType[0])
    {
        /* Direct macroblock bitplane */
        DEBUG0(vc1DEBUG_PIC, "DIRECTMB: Bitplane\n");
        eResult = vc1DECBITPL_ReadBitplane(pState, &pParams->sBPBFrameDirectMode, pBitstream);
        if(vc1_ResultOK != eResult)
        {
            return(eResult);
        }
        COVERAGE("9.1.1.44");
    }

    /* Macroblock skip bitplane */
    DEBUG0(vc1DEBUG_PIC, "MBSKIP: Bitplane\n");
    eResult = vc1DECBITPL_ReadBitplane(pState, &pParams->sBPSkipMB, pBitstream);
    if(vc1_ResultOK != eResult)
    {
        return(eResult);
    }

    /* Motion vector huffman table */
    Value = vc1DECBIT_GetBits(pBitstream, VC1_BITS_MVTAB);
    if(VC1DECBIT_EOF == Value)
    {
        return(vc1_ResultBufferExhausted);
    }
    switch(Value)
    {
        case 0:
            pParams->pMotionVectorTable = vc1DEC_Mot_Vector_Diff_VLC_0;
            break;

        case 1:
            pParams->pMotionVectorTable = vc1DEC_Mot_Vector_Diff_VLC_1;
            break;

        case 2:
            pParams->pMotionVectorTable = vc1DEC_Mot_Vector_Diff_VLC_2;
            break;

        case 3:
            pParams->pMotionVectorTable = vc1DEC_Mot_Vector_Diff_VLC_3;
            break;
    }
    DEBUG1(vc1DEBUG_PIC, "MVTAB: %d\n", Value);



    /* Coded block pattern table */
    Value = vc1DECBIT_GetBits(pBitstream, VC1_BITS_CBPTAB);
    if(VC1DECBIT_EOF == Value)
    {
        return(vc1_ResultBufferExhausted);
    }
    switch(Value)
    {
        case 0:
            pParams->pCodedBlockPatternTable = vc1DEC_P_Picture_CBPCY_VLC_0;
            break;

        case 1:
            pParams->pCodedBlockPatternTable = vc1DEC_P_Picture_CBPCY_VLC_1;
            break;

        case 2:
            pParams->pCodedBlockPatternTable = vc1DEC_P_Picture_CBPCY_VLC_2;
            break;

        case 3:
            pParams->pCodedBlockPatternTable = vc1DEC_P_Picture_CBPCY_VLC_3;
            break;
    }
    DEBUG1(vc1DEBUG_PIC, "CBPTAB: %d\n", Value);

    eResult = vc1DECPIC_UnpackVOPDQUANTParams(pParams, pSeqParams, pBitstream);
    if(vc1_ResultOK != eResult)
    {
        return(eResult);
    }


    if(1 == pSeqParams->VSTransform)
    {            
        /* Macroblock level transform type flag */
        Value = vc1DECBIT_GetBits(pBitstream, VC1_BITS_TTMBF);
        if(VC1DECBIT_EOF == Value)
        {
            return(vc1_ResultBufferExhausted);
        }
        DEBUG1(vc1DEBUG_PIC, "TTMBF: %d\n", Value);
        COVERAGE_NONINT_INT("7.1.1.31","9.1.1.51");
        pParams->MBTransformTypeFlag = (FLAG)Value;

        if(1 == Value)
        {
            Value = vc1DECBIT_GetBits(pBitstream, VC1_BITS_TTFRM);
            if(VC1DECBIT_EOF == Value)
            {
                return(vc1_ResultBufferExhausted);
            }
            DEBUG1(vc1DEBUG_PIC, "TTFRM: %d\n", Value);
            COVERAGE_NONINT_INT("7.1.1.32","9.1.1.52");
            switch(Value)
            {
                case 0:
                    pParams->eFrameTransformType = vc1_BlkInter8x8;
                    break;

                case 1:
                    pParams->eFrameTransformType = vc1_BlkInter8x4;
                    break;

                case 2:
                    pParams->eFrameTransformType = vc1_BlkInter4x8;
                    break;

                case 3:
                    pParams->eFrameTransformType = vc1_BlkInter4x4;
                    break;
            }
        }
        else
        {
            /*
             * there's no frame level transform type - set to "any", 
             * as it will be filled in later
             */
            pParams->eFrameTransformType = vc1_BlkInterAny;
        }
    }
    else
    {
        /* Block transform type is fixed default */
        pParams->eFrameTransformType = vc1_BlkInter8x8;
    }

    /* Frame level transform AC coding set index */
    Value = vc1DECBIT_GetBits(pBitstream, VC1_BITS_TRANSACFRM_1);
    if(VC1DECBIT_EOF == Value)
    {
        return(vc1_ResultBufferExhausted);
    }
    DEBUG1(vc1DEBUG_PIC, "TRANSACFRM_1: %d\n", Value);
    if(0 == Value)
    {
        /* 1-bit code */
        pParams->FrameTransformACCodingSetIndex = 0;
    }
    else
    {
        /* 2-bit code */
        Value = vc1DECBIT_GetBits(pBitstream, VC1_BITS_TRANSACFRM_2);
        if(VC1DECBIT_EOF == Value)
        {
            return(vc1_ResultBufferExhausted);
        }
        DEBUG1(vc1DEBUG_PIC, "TRANSACFRM_2: %d\n", Value);
        if(0 == Value)
        {
            pParams->FrameTransformACCodingSetIndex = 1;
        }
        else
        {
            pParams->FrameTransformACCodingSetIndex = 2;
        }
    }

    /* Intra transform DC table */
    Value = vc1DECBIT_GetBits(pBitstream, VC1_BITS_TRANSDCTAB);
    if(VC1DECBIT_EOF == Value)
    {
        return(vc1_ResultBufferExhausted);
    }
    DEBUG1(vc1DEBUG_PIC, "TRANSDCTAB: %d\n", Value);
    COVERAGE_NONINT_INT("7.1.1.43","9.1.1.26");
    pParams->IntraTransformDCTable = (FLAG)Value;


    if(pParams->PQuant >= 9)
    {
        pParams->eConditionalOverlap = (vc1_eCondOver)pSeqParams->OverlappedTransformFlag;
    }
    else
    {
        pParams->eConditionalOverlap = vc1_CondOverNone;
    }
    DEBUG1(vc1DEBUG_PIC, "Overlap = %d\n", pParams->eConditionalOverlap);

    return(vc1_ResultOK);
}













/*
 * Description:
 * Unpack the picture layer of the bitstream, and put the values into a struct
 *
 * Remarks:
 * Unpacks the I field picture layer or the second part of the I frame picture
 * layer for advanced profile
 *
 * Inputs:
 * pState - decoder state structure, into which data will be put
 * pBitstream - structure representing position in bitstream
 *
 * Outputs:
 * pBitstream - updated with new position in bitstream
 * pState - filled out with information from bitstream
 *
 * Return Value:
 * Standard vc1_eResult result. See enumeration for possible result codes
 *
 */

static vc1_eResult vc1DECPIC_UnpackFieldPictureLayerIAdvanced(
    vc1DEC_sState * pState,
    vc1DEC_sBitstream * pBitstream
)
{
    vc1DEC_sPictureLayerParams *pParams    = &pState->sPicParams;
    vc1_sSequenceLayer *pSeqParams         = &pState->sSeqParams;
    vc1_sPosition *pPosition               = &pState->sPosition;

    vc1_eResult eResult;
    WORD32 Value;

    /* Set MVRange to minimum allowed as B pictures check that the MVRange
     * they use is not less than the reference MVRange
     */
    pPosition->eMVRange = vc1_MVRange64_32;

    eResult = vc1DECPIC_UnpackQuantizationParams(pParams, pSeqParams, pBitstream);
    if(vc1_ResultOK != eResult)
    {
        return(eResult);
    }

    /* Post processing */
    if(TRUE == pSeqParams->PostProcessingFlag)
    {
        Value = vc1DECBIT_GetBits(pBitstream, VC1_BITS_POSTPROC);
        if(VC1DECBIT_EOF == Value)
        {
            return(vc1_ResultBufferExhausted);
        }
        DEBUG1(vc1DEBUG_PIC, "POSTPROC: %d\n", Value);
        COVERAGE("7.1.1.36");
        pParams->ePostProcessing = (vc1_ePostProcessing)Value;
    }
    else
    {
        pParams->ePostProcessing = vc1_PostProcessingNone;
    }

    if(vc1_InterlacedFrame == pPosition->ePictureFormat)
    {
        /* Field TX bitplane */
        DEBUG0(vc1DEBUG_PIC, "FIELDTX: Bitplane\n");
        eResult = vc1DECBITPL_ReadBitplane(pState, &pParams->sBPFieldTX, pBitstream);
        if(vc1_ResultOK != eResult)
        {
            return(eResult);
        }
    }

    if(TRUE == vc1_PictureTypeIsIntra(pPosition->ePictureType))
    {
        /* AC prediction bitplane */
        DEBUG0(vc1DEBUG_PIC, "ACPRED: Bitplane\n");
        eResult = vc1DECBITPL_ReadBitplane(pState, &pParams->sBPPredictAC, pBitstream);
        if(vc1_ResultOK != eResult)
        {
            return(eResult);
        }
        COVERAGE("7.1.1.37");
    }

    if(TRUE == pSeqParams->OverlappedTransformFlag)
    {
        if(pParams->PQuant <= 8)
        {
            /* conditional overlap flags */
            Value = vc1DECBIT_GetVLC(pBitstream, vc1DECPIC_Conditional_Overlap_Table);
            if(VC1DECBIT_EOF == Value)
            {
                return(vc1_ResultBufferExhausted);
            }
            DEBUG1(vc1DEBUG_PIC, "CONDOVER: %d\n", Value);
            COVERAGE("7.1.1.38");
            pParams->eConditionalOverlap = (vc1_eCondOver)Value;

            if(vc1_CondOverSome == Value)
            {
                /* per mb overlap */
                DEBUG0(vc1DEBUG_PIC, "OVERFLAGS: Bitplane\n");
                eResult = vc1DECBITPL_ReadBitplane(pState, &pParams->sBPOverflags, pBitstream);
                if(vc1_ResultOK != eResult)
                {
                    return(eResult);
                }
                COVERAGE("7.1.1.39");
            }            
        }
        else
        {
            pParams->eConditionalOverlap = vc1_CondOverAll;
        }
    }
    else
    {
        pParams->eConditionalOverlap = vc1_CondOverNone;
    }
    DEBUG1(vc1DEBUG_PIC, "Overlap = %d\n", pParams->eConditionalOverlap);
    
    /* Frame level transform AC coding set index */
    Value = vc1DECBIT_GetBits(pBitstream, VC1_BITS_TRANSACFRM_1);
    if(VC1DECBIT_EOF == Value)
    {
        return(vc1_ResultBufferExhausted);
    }
    DEBUG1(vc1DEBUG_PIC, "TRANSACFRM_1: %d\n", Value);
    COVERAGE_NONINT_INT("7.1.1.41","9.1.1.24");
    if(0 == Value)
    {
        /* 1-bit code */
        pParams->FrameTransformACCodingSetIndex = 0;
    }
    else
    {
        /* 2-bit code */
        Value = vc1DECBIT_GetBits(pBitstream, VC1_BITS_TRANSACFRM_2);
        if(VC1DECBIT_EOF == Value)
        {
            return(vc1_ResultBufferExhausted);
        }
        DEBUG1(vc1DEBUG_PIC, "TRANSACFRM_2: %d\n", Value);
        if(0 == Value)
        {
            pParams->FrameTransformACCodingSetIndex = 1;
        }
        else
        {
            pParams->FrameTransformACCodingSetIndex = 2;
        }
    }


    /* Frame level transform AC coding set index 2 */
    Value = vc1DECBIT_GetBits(pBitstream, VC1_BITS_TRANSACFRM2_1);
    if(VC1DECBIT_EOF == Value)
    {
        return(vc1_ResultBufferExhausted);
    }
    DEBUG1(vc1DEBUG_PIC, "TRANSACFRM2_1: %d\n", Value);
    COVERAGE_NONINT_INT("7.1.1.42","9.1.1.25");
    if(0 == Value)
    {
        /* 1-bit code */
        pParams->FrameTransformACCodingSetIndex2 = 0;
    }
    else
    {
        /* 2-bit code */
        Value = vc1DECBIT_GetBits(pBitstream, VC1_BITS_TRANSACFRM2_2);
        if(VC1DECBIT_EOF == Value)
        {
            return(vc1_ResultBufferExhausted);
        }
        DEBUG1(vc1DEBUG_PIC, "TRANSACFRM2_2: %d\n", Value);
        if(0 == Value)
        {
            pParams->FrameTransformACCodingSetIndex2 = 1;
        }
        else
        {
            pParams->FrameTransformACCodingSetIndex2 = 2;
        }
    }


    /* Intra transform DC table */
    Value = vc1DECBIT_GetBits(pBitstream, VC1_BITS_TRANSDCTAB);
    if(VC1DECBIT_EOF == Value)
    {
        return(vc1_ResultBufferExhausted);
    }
    DEBUG1(vc1DEBUG_PIC, "TRANSDCTAB: %d\n", Value);
    COVERAGE_NONINT_INT("7.1.1.43","9.1.1.26");
    pParams->IntraTransformDCTable = (FLAG)Value;


    eResult = vc1DECPIC_UnpackVOPDQUANTParams(pParams, pSeqParams, pBitstream);
    if(vc1_ResultOK != eResult)
    {
        return(eResult);
    }


    return(vc1_ResultOK);
}















/*
 * Description:
 * Unpack the picture layer of the bitstream, and put the values into a struct
 *
 * Remarks:
 * Unpacks P and B field picture layer for advanced profile
 *
 * Inputs:
 * pState - decoder state structure, into which data will be put
 * pBitstream - structure representing position in bitstream
 *
 * Outputs:
 * pBitstream - updated with new position in bitstream
 * pState - filled out with information from bitstream
 *
 * Return Value:
 * Standard vc1_eResult result. See enumeration for possible result codes
 *
 */

static vc1_eResult vc1DECPIC_UnpackFieldPictureLayerPBAdvanced(
    vc1DEC_sState * pState,
    vc1DEC_sBitstream * pBitstream
)
{
    vc1DEC_sPictureLayerParams *pParams    = &pState->sPicParams;
    vc1_sSequenceLayer      *pSeqParams    = &pState->sSeqParams;
    vc1_sPosition *pPosition               = &pState->sPosition;
    vc1_eResult eResult;
    WORD32 Value;

    /* Quantisation parameters */
    eResult = vc1DECPIC_UnpackQuantizationParams(pParams, pSeqParams, pBitstream);
    if(vc1_ResultOK != eResult)
    {
        return(eResult);
    }

    /* Post processing */
    if(TRUE == pSeqParams->PostProcessingFlag)
    {
        Value = vc1DECBIT_GetBits(pBitstream, VC1_BITS_POSTPROC);
        if(VC1DECBIT_EOF == Value)
        {
            return(vc1_ResultBufferExhausted);
        }
        DEBUG1(vc1DEBUG_PIC, "POSTPROC: %d\n", Value);
        COVERAGE("9.1.1.20");
        pParams->ePostProcessing = (vc1_ePostProcessing)Value;
    }
    else
    {
        pParams->ePostProcessing = vc1_PostProcessingNone;
    }

    if(vc1_InterlacedFrame == pPosition->ePictureFormat)
    {
        if(pParams->ePictureType[0] == vc1_PictureTypeB)
        {
            /* B fraction */
            Value = vc1DECBIT_GetVLC(pBitstream, vc1DECPIC_B_Fraction_Table);
            if(VC1DECBIT_EOF == Value)
            {
                return(vc1_ResultBufferExhausted);
            }

            pPosition->BFraction = (UBYTE8)Value;

            DEBUG3(vc1DEBUG_PIC, "BFRACTION: %d/%d (code %d)\n",
                vc1GENTAB_pBFraction[Value].Numerator,
                vc1GENTAB_pBFraction[Value].Denominator,
                Value);

            COVERAGE("7.1.1.15");

            if(0 == vc1GENTAB_pBFraction[Value].Denominator)    /* special cases */
            {
                FATAL("UnpackPictureLayerAdvanced(): Invalid BFRACTION - %d\n", Value);
                return(vc1_ResultFatal);
            }
        }
    }

    if(vc1_InterlacedField == pPosition->ePictureFormat)
    {
        if(vc1_PictureTypeP == pParams->ePictureType[pPosition->SecondField])
        {
            /* Number of reference frames flag */
            Value = vc1DECBIT_GetBits(pBitstream, VC1_BITS_NUMREF);
            if(VC1DECBIT_EOF == Value)
            {
                return(vc1_ResultBufferExhausted);
            }
            DEBUG1(vc1DEBUG_PIC, "NUMREF: %d\n", Value);
            COVERAGE("9.1.1.28");
            pPosition->NumRef = (UBYTE8)Value;

            if(0 == Value)
            {
                /* Reference field */
                Value = vc1DECBIT_GetBits(pBitstream, VC1_BITS_REFFIELD);
                if(VC1DECBIT_EOF == Value)
                {
                    return(vc1_ResultBufferExhausted);
                }
                DEBUG1(vc1DEBUG_PIC, "REFFIELD: %d\n", Value);
                COVERAGE("9.1.1.29");
                pPosition->RefField = (FLAG)Value;
            }
        }
        else if(vc1_PictureTypeB == pParams->ePictureType[pPosition->SecondField])
        {
            /* numref is assumed to be 1 */
            pPosition->NumRef = 1;
        }
        else
        {
            pPosition->NumRef = 0;
        }
    }
    else
    {
        pPosition->NumRef = 0;
        pPosition->RefField = 0;
    }

    if(1 == pSeqParams->ExtendedMV)
    {
        /* Extended motion vector range */
        Value = vc1DECBIT_GetVLC(pBitstream, vc1DECPIC_Motion_Vector_Range_Table);
        if(VC1DECBIT_EOF == Value)
        {
            return(vc1_ResultBufferExhausted);
        }
        DEBUG1(vc1DEBUG_PIC, "MVRANGE: %d\n", Value);
        COVERAGE("9.1.1.30");
        pPosition->eMVRange = (vc1_eMVRange)Value;
    }
    else
    {
        pPosition->eMVRange = (vc1_eMVRange)vc1_MVRange64_32;
    }

    pParams->ExtendHorizDMVRange = FALSE;
    pParams->ExtendVertDMVRange  = FALSE;

    if(1 == pSeqParams->ExtendedDMV)
    {
        /* Extended differential motion vector range */
        Value = vc1DECBIT_GetVLC(pBitstream, vc1DECPIC_Differential_Motion_Vector_Range_Table);
        if(VC1DECBIT_EOF == Value)
        {
            return(vc1_ResultBufferExhausted);
        }
        DEBUG1(vc1DEBUG_PIC, "DMVRANGE: %d\n", Value);
        COVERAGE("9.1.1.31");

        switch(Value)
        {
            case 0:
                /* both false */
                break;

            case 1: 
                pParams->ExtendVertDMVRange = TRUE;
                break;

            case 2:
                pParams->ExtendHorizDMVRange = TRUE;
                break;

            case 3:
                pParams->ExtendHorizDMVRange = TRUE;
                pParams->ExtendVertDMVRange = TRUE;
                break;
        }
    }

    if(vc1_InterlacedField == pPosition->ePictureFormat)
    {
        eResult = vc1DECPIC_UnpackInterlaceMVModeParams(pParams, pBitstream, pPosition);
        if(vc1_ResultOK != eResult)
        {
            return(eResult);
        }
    }
    else
    {
        if(vc1_PictureTypeP == pPosition->ePictureType)
        {
            /* 4 motion vector switch */
            Value = vc1DECBIT_GetBits(pBitstream, VC1_BITS_4MVSWITCH);
            if(VC1DECBIT_EOF == Value)
            {
                return(vc1_ResultBufferExhausted);
            }
            DEBUG1(vc1DEBUG_PIC, "4MVSWITCH: %d\n", Value);
            COVERAGE("9.1.1.33");
            if(TRUE == Value)
            {
                pPosition->eMVMode = vc1_MVModeMixedMV;
            }
            else
            {
                pPosition->eMVMode = vc1_MVMode1MV;
            }

            /* Intensity compensation flag */
            Value = vc1DECBIT_GetBits(pBitstream, VC1_BITS_INTCOMP);
            if(VC1DECBIT_EOF == Value)
            {
                return(vc1_ResultBufferExhausted);
            }
            DEBUG1(vc1DEBUG_PIC, "INTCOMP: %d\n", Value);
            COVERAGE("9.1.1.36");
            pParams->sIC[0].IntensityCompFlag = (FLAG)Value;

            if(TRUE == Value)
            {
                /* Luminance scale */
                Value = vc1DECBIT_GetBits(pBitstream, VC1_BITS_LUMSCALE);
                if(VC1DECBIT_EOF == Value)
                {
                    return(vc1_ResultBufferExhausted);
                }
                DEBUG1(vc1DEBUG_PIC, "LUMSCALE: %d\n", Value);
                COVERAGE("9.1.1.38");
                pParams->sIC[0].LuminanceScale = (UBYTE8)Value;

                /* Luminance shift */
                Value = vc1DECBIT_GetBits(pBitstream, VC1_BITS_LUMSHIFT);
                if(VC1DECBIT_EOF == Value)
                {
                    return(vc1_ResultBufferExhausted);
                }
                DEBUG1(vc1DEBUG_PIC, "LUMSHIFT: %d\n", Value);
                COVERAGE("9.1.1.39");
                pParams->sIC[0].LuminanceShift = (UBYTE8)Value;
            }
        }
        else /* B picture */
        {
            /* Intensity compensation flag */
            Value = vc1DECBIT_GetBits(pBitstream, VC1_BITS_INTCOMP);
            if(VC1DECBIT_EOF == Value)
            {
                return(vc1_ResultBufferExhausted);
            }
            DEBUG1(vc1DEBUG_PIC, "INTCOMP: %d\n", Value);
            COVERAGE("9.1.1.36");
            if(TRUE == Value)
            {
                WARN("INTCOMP is not FALSE\n");
            }

            pPosition->eMVMode = vc1_MVMode1MV;
            pParams->sIC[0].IntensityCompFlag = FALSE;
        }

        if(vc1_PictureTypeB == pPosition->ePictureType)
        {
            /* Direct macroblock bitplane */
            DEBUG0(vc1DEBUG_PIC, "DIRECTMB: Bitplane\n");
            eResult = vc1DECBITPL_ReadBitplane( pState,
                                                &pState->sPicParams.sBPBFrameDirectMode,
                                                pBitstream);
            if(vc1_ResultOK != eResult)
            {
                return(eResult);
            }
            COVERAGE("9.1.1.44");
        }

        /* Skipped macroblock bitplane */
        DEBUG0(vc1DEBUG_PIC, "SKIPMB: Bitplane\n");
        eResult = vc1DECBITPL_ReadBitplane(pState, &pState->sPicParams.sBPSkipMB, pBitstream);
        if(vc1_ResultOK != eResult)
        {
            return(eResult);
        }
        COVERAGE("9.1.1.32");
    }

    DEBUG2(vc1DEBUG_PIC,
           "Motion vector mode: %s (%d)\n",
           vc1DEBUG_MVMode[pPosition->eMVMode],
           pPosition->eMVMode);

    if(vc1_InterlacedField == pPosition->ePictureFormat)
    {
        if(vc1_PictureTypeB == pPosition->ePictureType)
        {
            /* forward macroblock */
            DEBUG0(vc1DEBUG_PIC, "FORWARDMB: Bitplane\n");
            eResult = vc1DECBITPL_ReadBitplane( pState,
                                                &pState->sPicParams.sBPForwardMB,
                                                pBitstream);

            if(vc1_ResultOK != eResult)
            {
                return(eResult);
            }
            COVERAGE("9.1.1.45");  /* Really FORWARDMB when processing interlace field */
        }
    }


    /* Macroblock mode table */

    if(vc1_InterlacedFrame != pPosition->ePictureFormat)
    {
        Value = vc1DECBIT_GetBits(pBitstream, 3);  /*-3 bit MBMODETAB */
        if(VC1DECBIT_EOF == Value)
        {
            return(vc1_ResultBufferExhausted);
        }
        if(vc1_MVModeMixedMV == pPosition->eMVMode)
        {
            const vc1DEC_sVLCCode *ModeTables[8] = {    vc1DEC_Mixed_MV_MB_0,
                                                        vc1DEC_Mixed_MV_MB_1,
                                                        vc1DEC_Mixed_MV_MB_2,
                                                        vc1DEC_Mixed_MV_MB_3,
                                                        vc1DEC_Mixed_MV_MB_4,
                                                        vc1DEC_Mixed_MV_MB_5,
                                                        vc1DEC_Mixed_MV_MB_6,
                                                        vc1DEC_Mixed_MV_MB_7 };

            pParams->pMBModeTable = ModeTables[Value];
        }
        else
        {
            const vc1DEC_sVLCCode *ModeTables[8] = {    vc1DEC_One_MV_MB_0,
                                                        vc1DEC_One_MV_MB_1,
                                                        vc1DEC_One_MV_MB_2,
                                                        vc1DEC_One_MV_MB_3,
                                                        vc1DEC_One_MV_MB_4,
                                                        vc1DEC_One_MV_MB_5,
                                                        vc1DEC_One_MV_MB_6,
                                                        vc1DEC_One_MV_MB_7 };

            pParams->pMBModeTable = ModeTables[Value];
        }
    }
    else    /* interlaced frame */
    {
        Value = vc1DECBIT_GetBits(pBitstream, 2);   /* 2-bit MBMODETAB */
        if(VC1DECBIT_EOF == Value)
        {
            return(vc1_ResultBufferExhausted);
        }
        if(vc1_MVModeMixedMV == pPosition->eMVMode)
        {
            switch(Value)
            {
                case 0:
                    pParams->pMBModeTable = vc1DEC_Interlace_4MV_MB_0;
                    break;

                case 1:
                    pParams->pMBModeTable = vc1DEC_Interlace_4MV_MB_1;
                    break;

                case 2:
                    pParams->pMBModeTable = vc1DEC_Interlace_4MV_MB_2;
                    break;

                case 3:
                    pParams->pMBModeTable = vc1DEC_Interlace_4MV_MB_3;
                    break;
            }
        }
        else
        {
            switch(Value)
            {
                case 0:
                    pParams->pMBModeTable = vc1DEC_Interlace_Non_4MV_MB_0;
                    break;

                case 1:
                    pParams->pMBModeTable = vc1DEC_Interlace_Non_4MV_MB_1;
                    break;

                case 2:
                    pParams->pMBModeTable = vc1DEC_Interlace_Non_4MV_MB_2;
                    break;

                case 3:
                    pParams->pMBModeTable = vc1DEC_Interlace_Non_4MV_MB_3;
                    break;
            }
        }
    }
    DEBUG1(vc1DEBUG_PIC, "MBMODETAB: %d\n", Value);
    COVERAGE("9.1.1.46");


    /* Motion vector table */
    if(0 == pPosition->NumRef)
    {
        /* 2-bit MVTAB code */
        Value = vc1DECBIT_GetBits(pBitstream, 2);
        if(VC1DECBIT_EOF == Value)
        {
            return(vc1_ResultBufferExhausted);
        }
        switch(Value)
        {
            case 0:
                pParams->pMotionVectorTable = vc1DEC_One_Field_Ref_Ilace_MV_0;
                break;

            case 1:
                pParams->pMotionVectorTable = vc1DEC_One_Field_Ref_Ilace_MV_1;
                break;

            case 2:
                pParams->pMotionVectorTable = vc1DEC_One_Field_Ref_Ilace_MV_2;
                break;

            case 3:
                pParams->pMotionVectorTable = vc1DEC_One_Field_Ref_Ilace_MV_3;
                break;
        }
        DEBUG1(vc1DEBUG_PIC, "MVTAB: %d (2 bits)\n", Value);
    }
    else
    {
        const vc1DEC_sVLCCode *VectorTable[8] = {   vc1DEC_Two_Field_Ref_Ilace_MV_0,
                                                    vc1DEC_Two_Field_Ref_Ilace_MV_1,
                                                    vc1DEC_Two_Field_Ref_Ilace_MV_2,
                                                    vc1DEC_Two_Field_Ref_Ilace_MV_3,
                                                    vc1DEC_Two_Field_Ref_Ilace_MV_4,
                                                    vc1DEC_Two_Field_Ref_Ilace_MV_5,
                                                    vc1DEC_Two_Field_Ref_Ilace_MV_6,
                                                    vc1DEC_Two_Field_Ref_Ilace_MV_7 };

        /* 3-bit MVTAB code */
        Value = vc1DECBIT_GetBits(pBitstream, 3);
        if(VC1DECBIT_EOF == Value)
        {
            return(vc1_ResultBufferExhausted);
        }

        pParams->pMotionVectorTable = VectorTable[Value];
        DEBUG1(vc1DEBUG_PIC, "MVTAB: %d (3 bits)\n", Value);
    }
    COVERAGE("9.1.1.47");

    /* Coded block pattern table */
    Value = vc1DECBIT_GetBits(pBitstream, VC1_BITS_CBPTAB_INTERLACE);
    if(VC1DECBIT_EOF == Value)
    {
        return(vc1_ResultBufferExhausted);
    }

    {
        const vc1DEC_sVLCCode *CBPTable[8] = {  vc1DEC_Interlaced_CBPCY_0,
                                                vc1DEC_Interlaced_CBPCY_1,
                                                vc1DEC_Interlaced_CBPCY_2,
                                                vc1DEC_Interlaced_CBPCY_3,
                                                vc1DEC_Interlaced_CBPCY_4,
                                                vc1DEC_Interlaced_CBPCY_5,
                                                vc1DEC_Interlaced_CBPCY_6,
                                                vc1DEC_Interlaced_CBPCY_7 };

        pParams->pCodedBlockPatternTable = CBPTable[Value];
    }

    DEBUG1(vc1DEBUG_PIC, "CBPTAB: %d\n", Value);
    COVERAGE("9.1.1.48");

    /* 2MV coded block pattern table */
    if(vc1_InterlacedFrame == pPosition->ePictureFormat)
    {
        Value = vc1DECBIT_GetBits(pBitstream, VC1_BITS_2MVBPTAB);
        if(VC1DECBIT_EOF == Value)
        {
            return(vc1_ResultBufferExhausted);
        }
        switch(Value)
        {
            case 0:
                pParams->pMB2MVBlockPatternTable = vc1DEC_Interlace_2_MVP_Pattern_0;
                break;

            case 1:
                pParams->pMB2MVBlockPatternTable = vc1DEC_Interlace_2_MVP_Pattern_1;
                break;

            case 2:
                pParams->pMB2MVBlockPatternTable = vc1DEC_Interlace_2_MVP_Pattern_2;
                break;

            case 3:
                pParams->pMB2MVBlockPatternTable = vc1DEC_Interlace_2_MVP_Pattern_3;
                break;
        }
        DEBUG1(vc1DEBUG_PIC, "2MVBPTAB: %d\n", Value);
        COVERAGE("9.1.1.49");   
    }
    else
    {
        pParams->pMB2MVBlockPatternTable = NULL;
    }


    /* 4MV coded block pattern table */
    if( (vc1_MVModeMixedMV == pPosition->eMVMode)
        || 
        ((vc1_PictureTypeB == pPosition->ePictureType)  && 
        (vc1_InterlacedFrame == pPosition->ePictureFormat)) )
    {
        Value = vc1DECBIT_GetBits(pBitstream, VC1_BITS_4MVBPTAB);
        if(VC1DECBIT_EOF == Value)
        {
            return(vc1_ResultBufferExhausted);
        }
        switch(Value)
        {
            case 0:
                pParams->pMB4MVBlockPatternTable = vc1DEC_FourMV_Pattern_0;
                break;

            case 1:
                pParams->pMB4MVBlockPatternTable = vc1DEC_FourMV_Pattern_1;
                break;

            case 2:
                pParams->pMB4MVBlockPatternTable = vc1DEC_FourMV_Pattern_2;
                break;

            case 3:
                pParams->pMB4MVBlockPatternTable = vc1DEC_FourMV_Pattern_3;
                break;
        }
        DEBUG1(vc1DEBUG_PIC, "4MVBPTAB: %d\n", Value);
        COVERAGE("9.1.1.50");
    }
    else
    {
        pParams->pMB4MVBlockPatternTable = NULL;
    }


    eResult = vc1DECPIC_UnpackVOPDQUANTParams(pParams, pSeqParams, pBitstream);
    if(vc1_ResultOK != eResult)
    {
        return(eResult);
    }

    if(1 == pSeqParams->VSTransform)
    {            
        /* Macroblock level transform type flag */
        Value = vc1DECBIT_GetBits(pBitstream, VC1_BITS_TTMBF);
        if(VC1DECBIT_EOF == Value)
        {
            return(vc1_ResultBufferExhausted);
        }
        DEBUG1(vc1DEBUG_PIC, "TTMBF: %d\n", Value);
        COVERAGE_NONINT_INT("7.1.1.31","9.1.1.51");
        pParams->MBTransformTypeFlag = (FLAG)Value;

        if(1 == Value)
        {
            Value = vc1DECBIT_GetBits(pBitstream, VC1_BITS_TTFRM);
            if(VC1DECBIT_EOF == Value)
            {
                return(vc1_ResultBufferExhausted);
            }
            DEBUG1(vc1DEBUG_PIC, "TTFRM: %d\n", Value);
            COVERAGE_NONINT_INT("7.1.1.32","9.1.1.52");
            switch(Value)
            {
                case 0:
                    pParams->eFrameTransformType = vc1_BlkInter8x8;
                    break;

                case 1:
                    pParams->eFrameTransformType = vc1_BlkInter8x4;
                    break;

                case 2:
                    pParams->eFrameTransformType = vc1_BlkInter4x8;
                    break;

                case 3:
                    pParams->eFrameTransformType = vc1_BlkInter4x4;
                    break;
            }
        }
        else
        {
            /*
             * there's no frame level transform type - set to "any", 
             * as it will be filled in later
             */
            pParams->eFrameTransformType = vc1_BlkInterAny;
        }
    }
    else
    {
        /* Block transform type is fixed default */
        pParams->eFrameTransformType = vc1_BlkInter8x8;
    }

    /* Frame level transform AC coding set index */
    Value = vc1DECBIT_GetBits(pBitstream, VC1_BITS_TRANSACFRM_1);
    if(VC1DECBIT_EOF == Value)
    {
        return(vc1_ResultBufferExhausted);
    }
    DEBUG1(vc1DEBUG_PIC, "TRANSACFRM_1: %d\n", Value);
    if(0 == Value)
    {
        /* 1-bit code */
        pParams->FrameTransformACCodingSetIndex = 0;
    }
    else
    {
        /* 2-bit code */
        Value = vc1DECBIT_GetBits(pBitstream, VC1_BITS_TRANSACFRM_2);
        if(VC1DECBIT_EOF == Value)
        {
            return(vc1_ResultBufferExhausted);
        }
        DEBUG1(vc1DEBUG_PIC, "TRANSACFRM_2: %d\n", Value);
        if(0 == Value)
        {
            pParams->FrameTransformACCodingSetIndex = 1;
        }
        else
        {
            pParams->FrameTransformACCodingSetIndex = 2;
        }
    }

    /* Intra transform DC table */
    Value = vc1DECBIT_GetBits(pBitstream, VC1_BITS_TRANSDCTAB);
    if(VC1DECBIT_EOF == Value)
    {
        return(vc1_ResultBufferExhausted);
    }
    DEBUG1(vc1DEBUG_PIC, "TRANSDCTAB: %d\n", Value);
    COVERAGE_NONINT_INT("7.1.1.43","9.1.1.26");
    pParams->IntraTransformDCTable = (FLAG)Value;

    if(pParams->PQuant >= 9)
    {
        pParams->eConditionalOverlap = (vc1_eCondOver)pSeqParams->OverlappedTransformFlag;
    }
    else
    {
        pParams->eConditionalOverlap = vc1_CondOverNone;
    }
    DEBUG1(vc1DEBUG_PIC, "Overlap = %d\n", pParams->eConditionalOverlap);

    return(vc1_ResultOK);
}




/*
 * Description:
 * Copy various field from the reference picture to the application's
 * picture structure.
 *
 * Remarks:
 * None
 *
 * Inputs:
 * pPic     - pointer to application's picture structure
 * pRefPic  - pointer to decoder's reference picture structure
 *
 * Outputs:
 * pPic     - updated with information from reference picture
 *
 * Return Value:
 * None.
 *
 */

static void vc1DECPIC_InitialiseAppPicture( vc1_sPicture *pPic,
                                            vc1_sReferencePicture *pRefPic)
{
    pPic->RFF                               = pRefPic->RFF;
    pPic->TFF                               = pRefPic->TFF;
    pPic->ePictureFormat                    = pRefPic->ePictureFormat;
    pPic->Frame                             = pRefPic->Frame;
    pPic->sField[0].ePictureType            = pRefPic->ePictureType[0];
    pPic->sField[1].ePictureType            = pRefPic->ePictureType[1];
    pPic->sPanScanParams                    = pRefPic->sPanScanParams;
    pPic->INTERPFRM                         = pRefPic->FrameInterpolationHint;
    pPic->UVSAMP                            = pRefPic->UVSampleMode;
    pPic->RPTFRM                            = pRefPic->RepeatFrameCount;
    pPic->ePostProcessing                   = pRefPic->ePostProcessing;
}









/*
 * Description:
 * Copy a frame or field to the output picture buffer
 *
 * Remarks:
 * Performs range reduction and resolution scale operations as part of the copy.
 *
 * Inputs:
 * pState       - pointer to the decoder's state structure
 * pRef         - pointer to the reference picture to display
 * DestField    - 0=Top 1=Bottom for InterlacedField
 * SourceField  - 0=Top 1=Bottom for InterlacedField
 *
 * Outputs:
 * pState.sPicture - updated with new picture data.
 *
 * Return Value:
 * None.
 *
 */

static void vc1DECPIC_DisplayField(
    vc1DEC_sState * pState,
    vc1_sReferencePicture * pRef,
    int DestField,
    int SrcField
)
{
    int X, Y, K;
    int step = 1;
    vc1_sComponent sD[3], sS[3];
    int CodedWidth, CodedHeight;
    int MaxCodedWidth, MaxCodedHeight;
    vc1_sPicture *pDestPic  = pState->pPicture;
    vc1_ePictureRes eRes    = pRef->ePictureRes;
    int YScale              = pRef->RangeYScale;
    int UVScale             = pRef->RangeUVScale;
    UBYTE8 *pSData, *pDData;
    int SBpl, DBpl;

    if (16 == YScale)
    {
        COVERAGE("8.1.1.4");
    }

    vc1DECPIC_InitialiseAppPicture(pDestPic, pRef);

    sD[0] = pDestPic->sY;
    sD[1] = pDestPic->sU;
    sD[2] = pDestPic->sV;

    sS[0] = pRef->sY;
    sS[1] = pRef->sU;
    sS[2] = pRef->sV;
    sS[0].pData = pRef->pImageY;
    sS[1].pData = pRef->pImageU;
    sS[2].pData = pRef->pImageV;

    if (FALSE == pRef->Valid)
    {
        /* if the source picture isn't valid, return immediately */
        DEBUG0(vc1DEBUG_PIC, "DisplayField: Source picture invalid\n");
        return;
    }

    DEBUG6(vc1DEBUG_PIC, "DisplayField: Frame = %d, Format = %s, Field=%d, "
        "Type = %d, YScale = %d, UVScale = %d\n",
        pDestPic->Frame, vc1DEBUG_PictureFormat[pDestPic->ePictureFormat], DestField,
        pDestPic->sField[DestField].ePictureType, 
        pRef->RangeYScale, pRef->RangeUVScale);

    if (vc1_InterlacedField == pRef->ePictureFormat)
    {
        step = 2;
    }

    for (K = 0; K < 3; K++)
    {
        vc1_sImagePosition *pIP;
        int Scale;
        int VertOffset = 0;
        
        pSData = sS[K].pData;
        pDData = sD[K].pData;
        SBpl = sS[K].Bpl;
        DBpl = sD[K].Bpl;

        DEBUG4(vc1DEBUG_PIC, "DisplayField: K = %d, offset = %d, step = %d, bpl = %d\n",
            K, DestField, step, SBpl);

        CodedWidth    = pRef->CodedWidth;
        CodedHeight   = pRef->CodedHeight;
        MaxCodedWidth  = pState->sSeqParams.MaxCodedWidth;
        MaxCodedHeight = pState->sSeqParams.MaxCodedHeight;

        ASSERT(CodedWidth  <= MaxCodedWidth);
        ASSERT(CodedHeight <= MaxCodedHeight);

        if (K == 0)
        {
            /* Luma */
            pIP = &pRef->sImagePosLuma;
            Scale = YScale;
        }
        else
        {
            /* Chroma */
            pIP = &pRef->sImagePosChroma;
            CodedWidth    >>= 1;
            CodedHeight   >>= 1;
            MaxCodedWidth  >>= 1;
            MaxCodedHeight >>= 1;
            Scale = UVScale;
        }

        /* Shrink according to resolution scale */
        if (eRes == vc1_PictureRes2x1 || eRes == vc1_PictureRes2x2)
        {
            pDData += CodedWidth;
            COVERAGE("ANNEX B");
        }
        if (eRes == vc1_PictureRes1x2 || eRes == vc1_PictureRes2x2)
        {
            VertOffset = CodedHeight;
            COVERAGE("ANNEX B");

            /* We run the vertical filter up to the next multiple of 8
             * (luma) or 4 (chroma) or the height+2, whichever is the
             * smaller.
             * The bottom row pixels only depend on the two following rows.
             */
            if (K==0)
            {
                CodedHeight = (CodedHeight + 7) & ~7;
            }
            else
            {
                CodedHeight = (CodedHeight + 3) & ~3;
            }
            if (VertOffset + 2 < CodedHeight)
            {
                CodedHeight = VertOffset + 2;
            }
            VertOffset = 2*VertOffset - CodedHeight;

            pDData += VertOffset*DBpl;
        }

        DEBUG5(vc1DEBUG_PIC, "DisplayField: K=%d CodedWidth=%d CodedHeight=%d MaxCodedWidth=%d"
            " MaxCodedHeight=%d\n", K, CodedWidth, CodedHeight, MaxCodedWidth, MaxCodedHeight);

        /* Copy data applying range reduction scale at the same time */
        pSData += SrcField * SBpl;
        pDData += DestField * DBpl;

        for (Y = 0; Y < CodedHeight; Y += step)
        {
            for (X = 0; X < CodedWidth; X++)
            {
                int Pixel;

                Pixel = pSData[X] - 128;
                Pixel = (Pixel*Scale + 4)>>3;
                pDData[X] = (UBYTE8)CLIP(Pixel + 128);
            }

            pSData += step * SBpl;
            pDData += step * DBpl;
        }

        /* Expand according to resolution scale (Simple/Main profiles only) */
        if (eRes == vc1_PictureRes2x1 || eRes == vc1_PictureRes2x2)
        {
            pDData -= CodedHeight * DBpl;

            /* Horizontal Upsample - Simple/Main only */
            for (Y=0; Y<CodedHeight; Y++)
            {
                vc1TOOLS_ResolutionUpsample(pDData-CodedWidth, pDData, 1, CodedWidth, CodedWidth);
                pDData += DBpl;
            }
            pDData -= CodedWidth;
            CodedWidth <<= 1;
        }

        if (eRes == vc1_PictureRes1x2 || eRes == vc1_PictureRes2x2)
        {
            pDData -= CodedHeight * DBpl;

            /* Vertical Upsample */
            for (X=0; X<CodedWidth; X++)
            {
                int Height = (VertOffset+CodedHeight)>>1;
                vc1TOOLS_ResolutionUpsample(pDData-VertOffset*DBpl,
                                            pDData,
                                            DBpl,
                                            Height,
                                            CodedHeight);
                pDData ++;
            }
        }

    } /* Next component K */
}

/*
 * Description
 * Copy a reference picture to the display picture buffer
 *
 * Remark
 * This picture may be one frame or two fields
 *
 * Inputs
 * pState       - pointer to decoder state
 * pRef         - pointer to reference picture
 *
 * Outputs
 * pState->sPicture written
 */

void vc1DECPIC_DisplayPicture(vc1DEC_sState * pState, vc1_sReferencePicture * pRef)
{
    vc1_sPicture *pDestPic  = pState->pPicture;

    /* Clear picture to aid debug */
    memset(pDestPic->sY.pData, 0, pDestPic->sY.Bpl * pState->sSeqParams.MaxCodedHeight);
    memset(pDestPic->sU.pData, 0, pDestPic->sU.Bpl * pState->sSeqParams.MaxCodedHeight/2);
    memset(pDestPic->sV.pData, 0, pDestPic->sV.Bpl * pState->sSeqParams.MaxCodedHeight/2);

    if (pRef->ePictureFormat == vc1_InterlacedField)
    {
        int TFF = pRef->TFF;

        /* Display first field. Note that in the case of an old reference
         * picture we must display the non-intensity compensated image
         */
        if (pRef != pState->sPosition.pReferenceB)
        {
            /* I or P */
            if (pRef->ePictureType[0] == vc1_PictureTypeP &&
                (!pState->sPosition.pReferenceOld->Valid ||
                  pState->sPosition.pReferenceOld->BrokenLink))
            {
                /* P/I with no reference frame */
                WARN("Broken link P/I\n");
                vc1DECPIC_DisplayField(pState, pRef, 1-TFF, TFF);
            }
            else
            {
                vc1DECPIC_DisplayField(pState, pState->sPosition.pReferenceNoIC, 1-TFF, 1-TFF);
            }
        }
        else
        {
            vc1DECPIC_DisplayField(pState, pRef, 1-TFF, 1-TFF);
        }

        /* Display the second field */
        vc1DECPIC_DisplayField(pState, pRef, TFF, TFF);
    }
    else
    {
        /* Display both fields */
        vc1DECPIC_DisplayField(pState, pRef, 0, 0);
    }
}


/*
 * Description:
 * Unpack the syncmarker
 *
 * Remarks:
 * Discards any payload, although it is available via debug output.
 *
 * Inputs:
 * pBitstream   - pointer to struct representing the current bitstream
 *
 * Outputs:
 * pBitstream   - updated with new position
 *
 * Return Value:
 * Standard vc1_eResult result. See enumeration for possible result codes.
 * 
 */

vc1_eResult vc1DECPIC_UnpackSyncmarker(vc1DEC_sBitstream * pBitstream)
{
    int i;
    UWORD32 SM, Value;

    /* Align to a byte boundary */
    if(VC1DECBIT_EOF == vc1DECBIT_AlignBit(pBitstream))
    {
        return(vc1_ResultBufferExhausted);
    }

    SM = vc1DECBIT_GetBits(pBitstream, 24); /* get 24 bits of syncmarker */
    if(VC1DECBIT_EOF == SM)
    {
        return(vc1_ResultBufferExhausted);
    }

    COVERAGE("8.8");

    if(SM == VC1_SYNCMARKER_FIVE_BYTE)
    {
        DEBUG0(vc1DEBUG_MB, "Five byte syncmarker payload:");
        for(i = 0; i < 5; i++)
        {
            Value = vc1DECBIT_GetBits(pBitstream, 8);
            if(VC1DECBIT_EOF == Value)
            {
                return(vc1_ResultBufferExhausted);
            }
            DEBUG1(vc1DEBUG_MB, "%d ", Value);
        }
        DEBUG0(vc1DEBUG_MB, "\n");
    }
    else if(SM == VC1_SYNCMARKER_ELEVEN_BYTE)
    {
        DEBUG0(vc1DEBUG_MB, "Eleven byte syncmarker payload:");
        for(i = 0; i < 11; i++)
        {
            Value = vc1DECBIT_GetBits(pBitstream, 8);
            if(VC1DECBIT_EOF == Value)
            {
                return(vc1_ResultBufferExhausted);
            }
            DEBUG1(vc1DEBUG_MB, "%d ", Value);
        }
        DEBUG0(vc1DEBUG_MB, "\n");
    }
    else
    {
        FATAL("vc1DECMB_UnpackSyncmarker: unknown syncmarker, %d\n", SM);
        return(vc1_ResultFatal);
    }

    return(vc1_ResultOK);
}







/*
 * Description:
 * Read the simple/main profile picture layer, or the first part of the advanced picture
 * layer from the bitstream
 *
 * Remarks:
 * None
 *
 * Inputs:
 * pState       - decoder state structure, into which data will be put
 * pBitstream   - structure representing position in bitstream
 *
 * Outputs:
 * pBitstream   - updated with new position in bitstream
 * pState       - filled out with information from bitstream
 *
 * Return Value:
 * Standard vc1_eResult result. See enumeration for possible result codes
 *
 */
static vc1_eResult vc1DECPIC_ReadPictureLayer(  vc1DEC_sState * pState,
                                                vc1DEC_sBitstream * pBitstream)
{
    vc1_sPosition *pPosition = &pState->sPosition;
    vc1_eResult eResult = vc1_ResultOK;

    if(vc1_ProfileAdvanced == pState->sSeqParams.eProfile)
    {
        WORD32 Value;
        pPosition->ePictureFormat = vc1_ProgressiveFrame;

        /* determine picture format */
        if(TRUE == pState->sSeqParams.Interlace)
        {
            /* Picture coding type */
            Value = vc1DECBIT_GetBits(pBitstream, VC1_BITS_FCM_1);
            if(VC1DECBIT_EOF == Value)
            {
                return(vc1_ResultBufferExhausted);
            }
            DEBUG1(vc1DEBUG_PIC, "FCM_1: %d\n", Value);
            if(0 != Value)
            {
                Value = vc1DECBIT_GetBits(pBitstream, VC1_BITS_FCM_2);
                if(VC1DECBIT_EOF == Value)
                {
                    return(vc1_ResultBufferExhausted);
                }
                DEBUG1(vc1DEBUG_PIC, "FCM_2: %d\n", Value);
                if(1 == Value)
                {
                    pPosition->ePictureFormat = vc1_InterlacedField;
                    pState->FieldCount = 2;
                }
                else
                {
                    pPosition->ePictureFormat = vc1_InterlacedFrame;
                }
            }
            
            COVERAGE("7.1.1.2");
        }

        /* set the dimensions of the picture being decoded in MB units */
        vc1DECPIC_SetDimensionsInMB(pState, pPosition);

        DEBUG2(vc1DEBUG_PIC, "WidthMB = %d, HeightMB = %d\n",
            pPosition->WidthMB, pPosition->HeightMB);
            
        /*
         * if this is an interlaced field picture, check that the 
         * number of macroblocks in the frame is still allowed by the
         * current level
         */
        if(vc1_InterlacedField == pPosition->ePictureFormat)
        {
            unsigned int MBf = pPosition->WidthMB * pPosition->HeightMB * 2;
            if(MBf > pState->pLevelLimit->MBf)
            {
                FATAL("Number of macroblocks greater than that allowed by level (%d > %d)\n", 
                    MBf, pState->pLevelLimit->MBf);
                return(vc1_ResultFatal);
            }
        }

        DEBUG0(vc1DEBUG_PIC, "UnpackPictureLayerAdvanced()\n");
        eResult = vc1DECPIC_UnpackPictureLayerAdvanced(pState, pBitstream); 
    }
    else
    {
        DEBUG0(vc1DEBUG_PIC, "UnpackPictureLayerSimpleMain()\n");
        eResult = vc1DECPIC_UnpackPictureLayerSimpleMain(pState, pBitstream);
    }

    return(eResult);
}









/*
 * Description:
 * Read the remaining fields of an advanced picture layer, or an entire field picture
 * layer from the bitstream
 *
 * Remarks:
 * None
 *
 * Inputs:
 * pState       - decoder state structure, into which data will be put
 * pBitstream   - structure representing position in bitstream
 *
 * Outputs:
 * pBitstream   - updated with new position in bitstream
 * pState       - filled out with information from bitstream
 *
 * Return Value:
 * Standard vc1_eResult result. See enumeration for possible result codes
 *
 */
static vc1_eResult vc1DECPIC_ReadAdvancedPictureLayer(  vc1DEC_sState * pState,
                                                        vc1DEC_sBitstream * pBitstream)
{
    vc1_sPosition *pPosition    = &pState->sPosition;
    vc1_eResult eResult         = vc1_ResultOK;

    if(vc1_ProfileAdvanced == pState->sSeqParams.eProfile)
    {
        if(vc1_ProgressiveFrame != pPosition->ePictureFormat)
        {
            /* Unpack the interlaced field/frame picture layer */
            switch(pPosition->ePictureType)
            {
                case vc1_PictureTypeBI: /* fall through */
                case vc1_PictureTypeI:
                    DEBUG0(vc1DEBUG_PIC, "UnpackFieldPictureLayerIAdvanced()\n");
                    eResult = vc1DECPIC_UnpackFieldPictureLayerIAdvanced(pState, pBitstream);
                    break;

                case vc1_PictureTypeP:  /* fall through */
                case vc1_PictureTypeB:
                    DEBUG0(vc1DEBUG_PIC, "UnpackFieldPictureLayerPBAdvanced()\n");
                    eResult = vc1DECPIC_UnpackFieldPictureLayerPBAdvanced(pState, pBitstream);
                    break;

                case vc1_PictureTypeSkipped:
                    DEBUG0(vc1DEBUG_PIC, "Skipped frame\n");
                    break;

                default:
                    FATAL("UnpackPictureLayer: Unsupported picture type %d\n",
                        pPosition->ePictureType);
                    return(vc1_ResultFatal);
            }
        }
        else
        {
            /* Unpack the progressive advanced picture layers */
            switch(pPosition->ePictureType)
            {
                case vc1_PictureTypeBI: /* fall through */
                case vc1_PictureTypeI:
                    DEBUG0(vc1DEBUG_PIC, "UnpackFieldPictureLayerIAdvanced()\n");
                    eResult = vc1DECPIC_UnpackFieldPictureLayerIAdvanced(pState, pBitstream);
                    break;

                case vc1_PictureTypeP:  /* fall through */
                case vc1_PictureTypeB:
                    DEBUG0(vc1DEBUG_PIC, "UnpackPictureLayerPBAdvanced()\n");
                    eResult = vc1DECPIC_UnpackPictureLayerPBAdvanced(pState, pBitstream);
                    break;

                case vc1_PictureTypeSkipped:
                    DEBUG0(vc1DEBUG_PIC, "Skipped frame\n");
                    break;

                default:
                    FATAL("UnpackPictureLayer: Unsupported picture type %d\n", 
                        pPosition->ePictureType);
                    return(vc1_ResultFatal);
            }
        }
    }

    return(eResult);
}








/*
 * Description:
 * Unpack a picture layer present in a slice 
 *
 * Remarks:
 * None
 *
 * Inputs:
 * pState       - decoder state structure, into which data will be put
 * pBitstream   - structure representing position in bitstream
 *
 * Outputs:
 * pBitstream   - updated with new position in bitstream
 * pState       - filled out with information from bitstream
 *
 * Return Value:
 * Standard vc1_eResult result. See enumeration for possible result codes
 *
 */
vc1_eResult vc1DECPIC_UnpackInSlicePictureLayer(    vc1DEC_sState * pState,
                                                    vc1DEC_sBitstream * pBitstream)
{   
    vc1_eResult     eResult;

    /* unpack the simple/main picture layer, or the first part of the advanced picture layer */
    DEBUG0(vc1DEBUG_PIC, "ReadPictureLayer()\n");
    eResult = vc1DECPIC_ReadPictureLayer(pState, pBitstream);
    if(vc1_ResultOK != eResult)
    {
        return(eResult);
    }

    /* unpack the field picture layer or second part of the advanced picture layer */
    DEBUG0(vc1DEBUG_PIC, "ReadAdvancedPictureLayer()\n");
    eResult = vc1DECPIC_ReadAdvancedPictureLayer(pState, pBitstream);

    return(eResult);
}








/*
 * Description:
 * Unpack the field picture and lower layers of the bitstream
 *
 * Remarks:
 * None
 *
 * Inputs:
 * pState       - decoder state structure, into which data will be put
 * pBitstream   - structure representing position in bitstream
 *
 * Outputs:
 * pBitstream   - updated with new position in bitstream
 * pState       - filled out with information from bitstream
 *
 * Return Value:
 * Standard vc1_eResult result. See enumeration for possible result codes
 *
 */
vc1_eResult vc1DECPIC_UnpackFieldPictureLayer(  vc1DEC_sState * pState, 
                                                vc1DEC_sBitstream * pBitstream)
{
    vc1_sPosition *pPosition    = &pState->sPosition;
    vc1_eResult eResult         = vc1_ResultOK;

    pPosition->BottomField = 0;
    if(vc1_ProgressiveFrame != pPosition->ePictureFormat)
    {
        pPosition->BottomField = pPosition->SecondField;
        if(FALSE == pState->sPicParams.TopFieldFirst)
        {
            pPosition->BottomField = (FLAG)(1 - pPosition->SecondField);
        }
    }            
    pPosition->ePictureType = pState->sPicParams.ePictureType[pPosition->SecondField];

    if(vc1_ProfileAdvanced == pState->sSeqParams.eProfile)
    {
        pState->BitplaneCodingUsed = FALSE;

        /*
         * Read the advanced parts of picture layer from the bitstream.
         * This may be a complete field picture layer, or the second part
         * of an advanced frame picture layer
         *
         */
        DEBUG0(vc1DEBUG_PIC, "ReadAdvancedPictureLayer()\n");
        eResult = vc1DECPIC_ReadAdvancedPictureLayer(pState, pBitstream);
        if(vc1_ResultOK != eResult)
        {
            return(eResult);
        }
    }


    /* set up picture level tables */
    pState->ZigZagTableIndex = (UBYTE8)vc1GENTAB_ChooseZigZagTableSet(pPosition);

    pState->sPicParams.sInterpolate.SizeX = 8;
    pState->sPicParams.sInterpolate.SizeY = 8;

    if (vc1_PictureTypeIsRef(pPosition->ePictureType)
        || (vc1_PictureTypeSkipped == pPosition->ePictureType))
    {
        /* Reference Picture (I, P or Skipped) */
        if (FALSE == pPosition->SecondField)
        {
            /* Display the last reference picture - we must do this
             * while Old is still available so we can check for broken
             * link
             */
            vc1DECPIC_DisplayPicture(pState, pPosition->pReferenceNew);

            /* switch the old and new pictures */
            DEBUGREFPICT(vc1DEBUG_REFPICT, pPosition->pReferenceOld, "DecRef");

            DEBUG0(vc1DEBUG_PIC, "Swapping New/Old references pictures\n");

            vc1TOOLS_InitReferencePicture(  pPosition->pReferenceOld,
                                            &pState->sSeqParams,
                                            pPosition->CodedWidth,
                                            pPosition->CodedHeight);

            vc1TOOLS_NewReference(pPosition);

            if (pPosition->ePictureType==vc1_PictureTypeP
                || pPosition->ePictureType==vc1_PictureTypeSkipped)
            {
                /* Check reference picture exits */
                if (pPosition->ePictureFormat==vc1_InterlacedField
                    && pState->sPicParams.ePictureType[1]==vc1_PictureTypeI)
                {
                    /* It is a P/I picture */
                    if (pState->sSeqParams.ClosedEntry
                        && !pPosition->pReferenceOld->Valid)
                    {
                        FATAL("P/I reference picture not valid\n");
                        return vc1_ResultBadReferencePicture;
                    }
                }
                else if (!pPosition->pReferenceOld->Valid)
                {
                    FATAL("P reference picture not valid\n");
                    return vc1_ResultBadReferencePicture;
                }
                /* Check reference picture details match */
                if (pPosition->ePictureFormat == vc1_InterlacedField
                    && pPosition->pReferenceOld->ePictureFormat == vc1_InterlacedField
                    && pState->sPicParams.TopFieldFirst != pPosition->pReferenceOld->TFF)
                {
                    WARN("P Picture TFF does not match reference picture\n");
                }
            }

            pState->pCurrentRef = pPosition->pReferenceNew;
            pState->pCurrentRef->Frame = pState->FrameNum;
            pState->pCurrentRef->ePictureRes = pPosition->ePictureRes;
        }
        else
        {
            /* Second Field - Keep a copy of the old picture before
             * intensity compensation is applied
             */

            vc1TOOLS_InitReferencePicture(  pPosition->pReferenceNoIC, 
                                            &pState->sSeqParams,
                                            pPosition->CodedWidth,
                                            pPosition->CodedHeight);

            vc1TOOLS_CopyReference(pPosition->pReferenceNoIC, pPosition->pReferenceNew);
            pPosition->pReferenceNew->eMVRange[1] = pPosition->eMVRange;
        }
    }
    else /* B or BI picture */
    {
        if (pPosition->SecondField==0)
        {
            /* B/BI picture first field */
            if (!pPosition->pReferenceNew->Valid || pPosition->pReferenceNew->BrokenLink)
            {
                FATAL("Future B frame anchor doesn't exist\n");
                return vc1_ResultBadReferencePicture;
            }

            if (pPosition->ePictureType == vc1_PictureTypeB)
            {
                /* B picture - check reference frames */
                if(pState->sSeqParams.ClosedEntry == 0) /* B frames might not have past anchor for ClosedEntry sequence!!! */
                {
                    if (!pPosition->pReferenceOld->Valid || pPosition->pReferenceOld->BrokenLink)
                    {
                        if (pState->sSeqParams.ClosedEntry)
                        {
                            FATAL("Past B frame anchor doesn't exist\n");
                            return vc1_ResultBadReferencePicture;
                        }
                        WARN("B picture dropped due to missing reference\n");
                        return vc1_ResultNoFrame;
                    }
                }
                if (pPosition->eMVRange < pPosition->pReferenceNew->eMVRange[0])
                {
                    WARN("B MVRange[0] smaller than the anchor MVRange[0] %d %d\n",
                        pPosition->eMVRange, pPosition->pReferenceNew->eMVRange[0]);
                }
            }

            vc1TOOLS_InitReferencePicture(  pPosition->pReferenceB,
                                            &pState->sSeqParams,
                                            pPosition->CodedWidth,
                                            pPosition->CodedHeight);

            if (pPosition->RangeYScale  != pPosition->pReferenceNew->RangeYScale ||
                pPosition->RangeUVScale != pPosition->pReferenceNew->RangeUVScale)
            {
                WARN("B Range Reduction does not match anchor frame\n");
            }
            if (pPosition->ePictureFormat != pPosition->pReferenceNew->ePictureFormat)
            {
                FATAL("B Picture Format does not match anchor frame\n");
                return vc1_ResultBadReferencePicture;
            }
            if (pPosition->ePictureFormat == vc1_InterlacedField
                && pPosition->pReferenceNew->ePictureFormat == vc1_InterlacedField
                && pState->sPicParams.TopFieldFirst != pPosition->pReferenceNew->TFF)
            {
                WARN("B Picture TFF does not match backward anchor\n");
            }
            if (pPosition->ePictureFormat == vc1_InterlacedField
                && pPosition->pReferenceOld->ePictureFormat == vc1_InterlacedField
                && pState->sPicParams.TopFieldFirst != pPosition->pReferenceOld->TFF)
            {
                WARN("B Picture TFF does not match forward anchor\n");
            }

            pPosition->pReferenceB->ePictureFormat  = pPosition->ePictureFormat;
            pPosition->pReferenceB->RangeYScale     = pPosition->RangeYScale;
            pPosition->pReferenceB->RangeUVScale    = pPosition->RangeUVScale;
            pPosition->pReferenceB->ePictureRes     = pPosition->ePictureRes;
            pPosition->pReferenceB->eMVRange[0]     = pPosition->eMVRange;

            pState->pCurrentRef = pPosition->pReferenceB;
            pState->pCurrentRef->Frame = pState->FrameNum;
            
        }
        else /* B/BI picture second field */
        {
            if (pPosition->ePictureType==vc1_PictureTypeB 
                && pPosition->eMVRange < pPosition->pReferenceNew->eMVRange[1])
            {
                WARN("B MVRange[1] smaller than the anchor MVRange[1] %d %d\n",
                    pPosition->eMVRange, pPosition->pReferenceNew->eMVRange[1]);
            }
            pPosition->pReferenceB->eMVRange[1]     = pPosition->eMVRange;
        }
    }

    if (pPosition->SecondField==0)
    {
        vc1DEC_sPictureLayerParams *pPicParams  = &pState->sPicParams;
        vc1_sReferencePicture *pRefPic          = pState->pCurrentRef;

        /* Copy picture parameters to the reference picture.
         * Be sure to copy both picture types incase there is an
         * error in decoding the first field. This will help the
         * debug view.
         */

        pRefPic->ePictureType[0]        = pPicParams->ePictureType[0];
        pRefPic->ePictureType[1]        = pPicParams->ePictureType[1];
        pRefPic->TFF                    = pPicParams->TopFieldFirst;
        pRefPic->RFF                    = pPicParams->RepeatFirstField;
        pRefPic->sPanScanParams         = pPicParams->sPanScanParams;
        pRefPic->ePostProcessing        = pPicParams->ePostProcessing;
        pRefPic->UVSampleMode           = pPicParams->UVSampleMode;
        pRefPic->FrameInterpolationHint = pPicParams->FrameInterpolationHint;
        pRefPic->RepeatFrameCount       = pPicParams->RepeatFrameCount;

    }

    if (vc1_PictureTypeIsInter(pPosition->ePictureType))
    {
        /* pad and intensity compensate old picture */
        vc1TOOLS_ICPadReferencePicture(pPosition, pState->sPicParams.sIC);
    }

    if(vc1_ProfileAdvanced != pState->sSeqParams.eProfile)
    {
        /* set interpolation rounding control depending on picture type */
        if (vc1_PictureTypeIsIntra(pPosition->ePictureType))
        {
            /* I or BI
             * Set rounding control to 1 since it is not used in
             * an I picture and we wan't it to toggle to 0 for the
             * next P picture
             */
            pState->sPicParams.sInterpolate.RndCtrl = 1;
        }
        else if(    (vc1_PictureTypeP       == pPosition->ePictureType)
            ) /* fix on 01/31/05 */
			  /* Romoved condition: '||(vc1_PictureTypeSkipped == pPosition->ePictureType)' */
        {
            /* toggle rounding control */
            pState->sPicParams.sInterpolate.RndCtrl
                = (UBYTE8)(1 - pState->sPicParams.sInterpolate.RndCtrl);
        }
        else /* B picture */
        {
            COVERAGE("8.4.4.2");  /* Rounding control same as previous for B frames */
        }
        COVERAGE("8.3.7");
    }

    if(pPosition->ePictureFormat != vc1_ProgressiveFrame)
    {
        /* set up the motion vector scaling structure */
        vc1SCALEMV_InitScaleMV(pPosition);
    }

    if(FALSE == pPosition->SecondField)
    {
        /* reset the motion vector buffer pointer */
        pPosition->pMVHist = pState->pMVHistBuffer;
    }

    /* the first slice in the picture */
    pPosition->SliceY = 0;

    eResult = vc1DECSLICE_DecodeSlice(pState, pBitstream);
    if(eResult != vc1_ResultOK)
    {
        return(eResult);
    }

    return(vc1_ResultOK);
}




/*
 * Description:
 * Unpack the picture and lower layers of the bitstream
 *
 * Remarks:
 * Decides what profile and frame type we're decoding, and select sub functions accordingly
 *
 * Inputs:
 * pState - decoder state structure, into which data will be put
 * pBitstream - structure representing position in bitstream
 *
 * Outputs:
 * pBitstream - updated with new position in bitstream
 * pState - filled out with information from bitstream
 *
 * Return Value:
 * Standard vc1_eResult result. See enumeration for possible result codes
 *
 */

vc1_eResult vc1DECPIC_UnpackPictureLayer(vc1DEC_sState * pState, vc1DEC_sBitstream * pBitstream)
{
    vc1_sPosition   *pPosition = &pState->sPosition;    
    vc1_eResult     eResult;



    pPosition->pCurMB           = pState->pMB;
    pPosition->pCurMB           = pState->pMB;
    pPosition->ePictureFormat   = vc1_ProgressiveFrame;
    pState->sPicParams.TopFieldFirst = TRUE;
    pState->FieldCount          = 1;

    DEBUG0(vc1DEBUG_PIC, "ReadPictureLayer()\n");
    eResult = vc1DECPIC_ReadPictureLayer(pState, pBitstream);
    if(vc1_ResultOK != eResult)
    {
        return(eResult);
    }


    DEBUG1(vc1DEBUG_PIC, "FieldCount = %d\n", pState->FieldCount);

    /* iterate over the number of fields in the image */
    for (   pPosition->SecondField = 0; 
            pPosition->SecondField < pState->FieldCount; 
            pPosition->SecondField++)
    {
        /* unpack one or two fields */
        eResult = vc1DECPIC_UnpackFieldPictureLayer(pState, pBitstream);
        if (vc1_ResultOK != eResult)
        {
            return(eResult);
        }
    }

    /* Note that the code will exit before here for the first frame
     * or an incomplete frame
     */

    return(eResult);
}
