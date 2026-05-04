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
 * decopts.c
 *
 * Decoder parameter options.
 *
 *  
 */

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <math.h>

#include "vc1types.h"
#include "rdopts.h"
#include "vc1dec.h"
#include "decopts.h"
#include "vc1gentab.h"
#include "vc1debug.h"

/********************************************************************
 * Forward declaration of validation functions
 *******************************************************************/

static vc1_eResult ValidateFrameRate(char *pValue);

/*
 * Variables that store the decoded options
 */

char  *DECOPTS_InputFile;
char  *DECOPTS_OutputFile;
RDOPTS_eBitstreamFormatType DECOPTS_InputType;
vc1DEC_sDecoderConfiguration DECOPTS_sDecoderSetup;
FLAG   DECOPTS_ShowUserData;

static char  *ChainedOptionsFile;
static WORD32 ChainFrameCount=-1;
static char *FrameRate;

#ifdef NDEBUG
static UWORD32 Dummy_Zones;
#endif

/*
 * Decoder options
 */
const RDOPTS_sOptionDefinition DECOPTS_sDecoderOptions[] =
{
    /* Always accept a debug mask option. Update a dummy variable for release builds */
    {
        RDOPTS_UWORD32,
        "DebugMask",
#ifndef NDEBUG
        &vc1DEBUG_Zones,
#else
        &Dummy_Zones,
#endif
        0,
        { NULL }
    },

    /*
     * Global setup options
     */
    {
        RDOPTS_STRING,
        "BitstreamFile",
        &DECOPTS_InputFile,
        0,
        { NULL },                 /* No immediate validation */
        NULL
    },
    {
        RDOPTS_ENUM,
        "BitstreamFormat",
        &DECOPTS_InputType,
        0,
        { &RDOPTS_BitstreamFormatOptions },
        NULL
    },
    {
        RDOPTS_STRING,
        "OutputYUV",
        &DECOPTS_OutputFile,
        0,
        { NULL },
        NULL
    },
    {
        RDOPTS_FLAG,
        "ShowUserData",
        &DECOPTS_ShowUserData,
        0,
        { NULL },
        NULL
    },

    /*
     * Sequence layer options
     */
    {
        RDOPTS_ENUM,
        "Level",
        &DECOPTS_sDecoderSetup.eLevel,
        0,
        { &RDOPTS_LevelOptions },
        NULL
    },

    /* Default frame rate if not specified in bitstream */
    {
        RDOPTS_STRING,      /* Frame rate is set in a readable format, */
        "FrameRate",        /* and parsed into the numerator/denominator fields */
        &FrameRate,
        0,
        { NULL},
        ValidateFrameRate
    },

    /* Option file chaining */
    {
        RDOPTS_STRING,
        "ChainOptions",
        &ChainedOptionsFile,
        0,
        { NULL },
        NULL
    },
    {
        RDOPTS_WORD32,
        "ChainAfter",
        &ChainFrameCount,
        0,
        { NULL },
        NULL
    },

    /* Terminate the list with a record full of NULLs */
    {
        RDOPTS_STRING,          /* Type not relevant */
        NULL,                   /* No name */
        NULL,                   /* No variable to store into */
        0,                      /* Not an array */
        { NULL },               /* Not range-checked */
        NULL                    /* Not validated */
    }
};

/*
 * Definition of validation functions
 */

/*
 * Description:
 *  Verification of parameters not provided by the rdopts options.
 *
 * Returns:
 *  vc1_ResultOK on success 
 */

vc1_eResult DECOPTS_ValidateParameters(void)
{
    /*
     * See the standard for the restrictions on each value.
     */
    if ((DECOPTS_sDecoderSetup.MaxCodedWidth  & 1) ||
        (DECOPTS_sDecoderSetup.MaxCodedHeight & 1))
    {
        FATAL("Odd number of pixels in picture dimensions (%dx%d)\n",
              DECOPTS_sDecoderSetup.MaxCodedWidth,
              DECOPTS_sDecoderSetup.MaxCodedHeight);
            return vc1_ResultInvalidParameter;
    }

    return vc1_ResultOK;
}

/*
 * Description:
 *  Calculate a bitstream buffer size that will not be
 *  overflowed by any frame.
 *
 * Returns:
 *  the required buffer size in bytes,
 */
UWORD32 DECOPTS_CalculateBitstreamBufferSize(void)
{
    /*
     * Read the maximum buffer size permitted for the selected
     * profile and level.
     * Note that this means that decoding does not use the minimum
     * possible buffer size, if HRD parameters indicate a smaller
     * buffer would accomodate the bitstream. This allows decoding
     * to be done on streams that violate their HRD parameters.
     * Checking for HRD violations is done elsewhere.
     */

    ASSERT(DECOPTS_sDecoderSetup.eLevel != vc1_LevelUnknown);

    return vc1GENTAB_LevelLimits[DECOPTS_sDecoderSetup.eProfile]
                                [DECOPTS_sDecoderSetup.eLevel].Bmax * (16384/8);
}

/*
 * Description:
 *  Check for any chained option updates. This is useful for debugging:
 *  debug output can be disabled until the frame of interest.
 */
vc1_eResult DECOPTS_UpdateOptions(void)
{
    vc1_eResult eResult = vc1_ResultOK;

    while (ChainFrameCount >= 0 && --ChainFrameCount < 0 && eResult==vc1_ResultOK)
    {
        /* Chain onto next file */
        eResult = RDOPTS_ReadOptions("decoder",ChainedOptionsFile,DECOPTS_sDecoderOptions);
    }

    return eResult;
}

/*
 * Description:
 *  Verify value has a valid framerate description
 *
 * Inputs:
 *  pValue  pointer to the value string
 *
 * Returns:
 *  vc1_ResultOK on success 
 */
static vc1_eResult ValidateFrameRate(char *pValue)
{
    return RDOPTS_ValidateFrameRate(pValue,
                                    &DECOPTS_sDecoderSetup.FrameRateNumerator,
                                    &DECOPTS_sDecoderSetup.FrameRateDenominator);
}

