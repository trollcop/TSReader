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
 * vc1dec.c:
 * Decoder API
 *
 *  
 */

#include "vc1types.h"
#include "vc1debug.h"
#include "vc1dec.h"
#include "vc1gentab.h"
#include "vc1decseq.h"
#include "vc1decpic.h"
#include "vc1decmb.h"
#include "vc1decbit.h"
#include "vc1decent.h"
#include "vc1decslice.h"
#include "vc1tools.h"
#include "vc1hrd.h"

#include <string.h> /* memset */

/*
 * Description
 * Set the MaxCodedSize for the picture
 *
 * Inputs:
 * pState   - Pointer to decoder state
 * Width    - Maximum Coded Width
 * Height   - Maximum Coded Height
 *
 * Return Value:
 * Standard vc1_eResult result. See enumeration for possible result codes.
 */

static vc1_eResult vc1DEC_SetMaxSize(vc1DEC_sState *pState, UWORD32 Width, UWORD32 Height)
{
    if(((Width+15) / 16) * ((Height+15) / 16) > pState->pLevelLimit->MBf)
    {
        FATAL("Image size too big for current profile and level %d > %d\n",
              (Width * Height)/(16*16),
              pState->pLevelLimit->MBf);
        return vc1_ResultImageTooBig;
    }

    pState->sSeqParams.MaxCodedWidth  = (UHWD16)Width;
    pState->sSeqParams.MaxCodedHeight = (UHWD16)Height;
    pState->sSeqParams.CodedWidth     = (UHWD16)Width;
    pState->sSeqParams.CodedHeight    = (UHWD16)Height;
    pState->sSeqParams.DisplayWidth   = (UHWD16)Width;
    pState->sSeqParams.DisplayHeight  = (UHWD16)Height;
    pState->sPosition.CodedWidth      = Width;
    pState->sPosition.CodedHeight     = Height;
    pState->sPosition.MaxCodedWidth   = Width;
    pState->sPosition.MaxCodedHeight  = Height;
    return vc1_ResultOK;
}

/*
 * Description:
 * Obtain memory requirements for decoding a bitstream.
 *
 * Remarks:
 * Examines the profile (and if present, the level) to determine the memory requirement.
 *
 * Inputs:
 * pSize      - pointer to UWORD32 into which the memory requirement will be written
 * pConfig    - pointer to a configuration structure, used to determine memory requirement
 * pBitstream - pointer to an initialised bitstream structure,
 *              indicating the start of the bitstream.
 *
 * Outputs:
 * pSize      - updated with memory requirement
 * pConfig    - updated with profile and level from bitstream.
 *
 * Return Value:
 * Standard vc1_eResult result. See enumeration for possible result codes.
 * 
 */

vc1_eResult vc1DEC_DecoderRequirements(UWORD32 *pSize,
                                       vc1DEC_sDecoderConfiguration *pConfig,
                                       vc1DEC_sBitstream * pBitstream)
{
    UWORD32 Size = 0;
    UWORD32 Req;
    WORD32 Value;
    const vc1_sLevelLimit *pLL;
    vc1_sSequenceLayer sSL;
    vc1_eProfile eProfile;
    vc1_eLevel eLevel;
    vc1DEC_sBitstream sBitstream = *pBitstream;
    int PictureWidth, PictureHeight;
    int WidthMB, HeightMB;

    /* read the profile from the bitstream */
    Value = vc1DECBIT_GetBits(&sBitstream, VC1_BITS_PROFILE);
    if(VC1DECBIT_EOF == Value)
    {
        FATAL("DecoderRequirements: Unable to read PROFILE bits\n");
        return(vc1_ResultFatal);
    }
    DEBUG1(vc1DEBUG_API, "PROFILE: %d\n", Value);
    eProfile = (vc1_eProfile)Value;

    if(vc1_ProfileAdvanced == eProfile)
    {
        Value = vc1DECBIT_GetBits(&sBitstream, VC1_BITS_LEVEL);
        if(VC1DECBIT_EOF == Value)
        {
            FATAL("DecoderRequirements: Unable to read LEVEL bits\n");
            return(vc1_ResultFatal);
        }
        DEBUG1(vc1DEBUG_API, "LEVEL: %d\n", Value);
        eLevel = (vc1_eLevel)Value;

        if(4 < eLevel)
        {
            FATAL("DecoderRequirements: Invalid level - %d\n", eLevel);
            return(vc1_ResultFatal);
        }


        /* skip next few elements */
        Value = vc1DECBIT_GetBits(&sBitstream,  VC1_BITS_CHROMA_FORMAT +
                                                VC1_BITS_FRMRTQ_POSTPROC +
                                                VC1_BITS_BITRTQ_POSTPROC +
                                                VC1_BITS_POSTPROCFLAG);
        if(VC1DECBIT_EOF == Value)
        {
            FATAL("DecoderRequirements: Unable to read bits\n");
            return(vc1_ResultFatal);
        }


        /* get max coded width and height */
        Value = vc1DECBIT_GetBits(&sBitstream, VC1_BITS_MAX_CODED_WIDTH);
        if(VC1DECBIT_EOF == Value)
        {
            FATAL("DecoderRequirements: Unable to read MAX_CODED_WIDTH bits\n");
            return(vc1_ResultFatal);
        }
        PictureWidth = 2 * (Value + 1);

        Value = vc1DECBIT_GetBits(&sBitstream, VC1_BITS_MAX_CODED_HEIGHT);
        if(VC1DECBIT_EOF == Value)
        {
            FATAL("DecoderRequirements: Unable to read MAX_CODED_HEIGHT bits\n");
            return(vc1_ResultFatal);
        }
        PictureHeight = 2 * (Value + 1);

        pConfig->eProfile       = eProfile;
        pConfig->eLevel         = eLevel;
        pConfig->MaxCodedWidth  = (UHWD16)PictureWidth;
        pConfig->MaxCodedHeight = (UHWD16)PictureHeight;
    }
    else /* Simple Main */
    {
        pConfig->eProfile = eProfile;
        if (pConfig->eLevel == vc1_LevelUnknown)
        {
            int WidthMB  = (pConfig->MaxCodedWidth + 15) >> 4;
            int HeightMB = (pConfig->MaxCodedHeight + 15) >> 4;
            int NumMB    = WidthMB * HeightMB;
            int Level;

            for (Level = 0; ; Level++)
            {
                int MBf = vc1GENTAB_LevelLimits[pConfig->eProfile][Level].MBf;

                if (MBf==0)
                {
                    FATAL("Image too big for all levels\n");
                    return vc1_ResultFatal;
                }

                if (NumMB <= MBf)
                {
                    break;
                }
            }
            pConfig->eLevel = (vc1_eLevel)Level;

            DEBUG1(vc1DEBUG_API, "Level Unknown: Choosing level %d\n", Level);
        }
    }

    ASSERT(pConfig->eLevel != vc1_LevelUnknown);
    pLL = &vc1GENTAB_LevelLimits[pConfig->eProfile][pConfig->eLevel];

    /* size of the state structure */
    Req = ALIGN(sizeof(vc1DEC_sState));
    DEBUG1(vc1DEBUG_API, "State structure = %d bytes\n", Req);
    Size += Req;

    /* size of MB buffer */
    Req = ALIGN(pLL->MBf * sizeof(vc1_sMB));
    DEBUG1(vc1DEBUG_API, "MB buffer       = %d bytes\n", Req);
    Size += Req;

    /* size of pictures */
    sSL.eProfile       = vc1_ProfileAdvanced;
    sSL.Interlace      = 1;
    sSL.MaxCodedWidth  = pConfig->MaxCodedWidth;
    sSL.MaxCodedHeight = pConfig->MaxCodedHeight;
    Req = vc1TOOLS_InitReferencePicture(NULL, &sSL,
                                        pConfig->MaxCodedWidth,
                                        pConfig->MaxCodedHeight);

    DEBUG1(vc1DEBUG_API, "Reference Old   = %d bytes\n", Req);
    Size += Req;

    DEBUG1(vc1DEBUG_API, "Reference New   = %d bytes\n", Req);
    Size += Req;

    DEBUG1(vc1DEBUG_API, "Reference B     = %d bytes\n", Req);
    Size += Req;

    DEBUG1(vc1DEBUG_API, "Reference NoIC  = %d bytes\n", Req);
    Size += Req;

    /*
     * size of 7 bitplanes
     *  ACPRED 
     *  SKIPMB
     *  MVTYPEMB
     *  DIRECTMB
     *  OVERFLAGS
     *  FORWARDMB
     *  FIELDTX
     *  
     */
    Req = 7 * ALIGN(pLL->MBf * sizeof(FLAG));
    DEBUG1(vc1DEBUG_API, "7 Bitplanes     = %d bytes\n", Req);
    Size += Req;

    /* size of motion vector history */
    WidthMB  = (UHWD16)((pConfig->MaxCodedWidth + 15) >> 4);
    HeightMB = (UHWD16)((pConfig->MaxCodedHeight + 15) >> 4);

    /* height + 1, to handle interlaced field case */
    Req = sizeof(vc1_sMotionHist) * WidthMB * (HeightMB + 1);
    DEBUG3(vc1DEBUG_API, "MV history      = %d bytes (%dx%d mbs)\n", Req, WidthMB, HeightMB);
    Size += Req;

    *pSize = Size;

    DEBUG1(vc1DEBUG_API, "Total decoder state size is %d bytes\n", Size);

    return(vc1_ResultOK);
}

/*
 * Description:
 * Initialise the state structure of the decoder.
 *
 * Remarks:
 * Initialises the state structure, sets up picture structures, etc.
 *
 * Inputs:
 * pState   - pointer to the decoder state structure which will be initialised
 * pConfig  - pointer to a configuration structure, used to determine memory requirement
 *
 * Outputs:
 * pState   - initialised for use by the decoder
 *
 * Return Value:
 * Standard vc1_eResult result. See enumeration for possible result codes.
 * 
 */

vc1_eResult vc1DEC_DecoderInitialise(vc1DEC_sState *pState, vc1DEC_sDecoderConfiguration *pConfig)
{
    vc1_sPosition *pPosition = &pState->sPosition;
    vc1_eResult eResult = vc1_ResultOK;
    vc1_sSequenceLayer sSL;

    /*
     * Accept a profile and level in config
     * if sequence header contains higher level or profile, fail with an error
     */

    UWORD32 Size;
    const vc1_sLevelLimit *pLL = &vc1GENTAB_LevelLimits[pConfig->eProfile][pConfig->eLevel];

    if(NULL == pState)
    {
        FATAL("Decoder state is NULL\n");
        return(vc1_ResultFatal);
    }

    Size = ALIGN(sizeof(vc1DEC_sState));

    /* clear state */
    memset(pState, 0, Size);

    pState->pLevelLimit = pLL;
    pState->pMB         = (vc1_sMB *)((char *)pState + Size);

    ASSERT(pConfig->MaxCodedWidth!=0 && pConfig->MaxCodedHeight!=0);

    eResult = vc1DEC_SetMaxSize(pState, pConfig->MaxCodedWidth, pConfig->MaxCodedHeight);
    if(vc1_ResultOK != eResult)
    {
        return(eResult);
    }

    /* Reserve space for macroblock data */
    Size = (UWORD32)(Size + ALIGN(pLL->MBf * sizeof(vc1_sMB)));

    sSL.eProfile       = vc1_ProfileAdvanced;
    sSL.Interlace      = 1;
    sSL.MaxCodedWidth  = pConfig->MaxCodedWidth;
    sSL.MaxCodedHeight = pConfig->MaxCodedHeight;
    pPosition->pReferenceOld = (vc1_sReferencePicture *)((char *)pState + Size);
    Size += vc1TOOLS_InitReferencePicture(pPosition->pReferenceOld, &sSL,
                                          pConfig->MaxCodedWidth,
                                          pConfig->MaxCodedHeight);

    pPosition->pReferenceNew = (vc1_sReferencePicture *)((char *)pState + Size);
    Size += vc1TOOLS_InitReferencePicture(pPosition->pReferenceNew, &sSL,
                                          pConfig->MaxCodedWidth,
                                          pConfig->MaxCodedHeight);

    pPosition->pReferenceB   = (vc1_sReferencePicture *)((char *)pState + Size);
    Size += vc1TOOLS_InitReferencePicture(pPosition->pReferenceB, &sSL,
                                          pConfig->MaxCodedWidth,
                                          pConfig->MaxCodedHeight);

    pPosition->pReferenceNoIC = (vc1_sReferencePicture *)((char *)pState + Size);
    Size += vc1TOOLS_InitReferencePicture(pPosition->pReferenceNoIC, &sSL,
                                          pConfig->MaxCodedWidth,
                                          pConfig->MaxCodedHeight);

    DEBUGNEWREFPICT(vc1DEBUG_REFPICT, pPosition->pReferenceOld, "DecRef");

    pState->sPicParams.sBPPredictAC.pBitplane = (FLAG *)((char *)pState + Size);
    Size += ALIGN(pLL->MBf * sizeof(FLAG));

    pState->sPicParams.sBPSkipMB.pBitplane = (FLAG *)((char *)pState + Size);
    Size += ALIGN(pLL->MBf * sizeof(FLAG));

    pState->sPicParams.sBPMotionVectorType.pBitplane = (FLAG *)((char *)pState + Size);
    Size += ALIGN(pLL->MBf * sizeof(FLAG));

    pState->sPicParams.sBPBFrameDirectMode.pBitplane = (FLAG *)((char *)pState + Size);
    Size += ALIGN(pLL->MBf * sizeof(FLAG));

    pState->sPicParams.sBPOverflags.pBitplane = (FLAG *)((char *)pState + Size);
    Size += ALIGN(pLL->MBf * sizeof(FLAG));

    pState->sPicParams.sBPForwardMB.pBitplane = (FLAG *)((char *)pState + Size);
    Size += ALIGN(pLL->MBf * sizeof(FLAG));

    pState->sPicParams.sBPFieldTX.pBitplane = (FLAG *)((char *)pState + Size);
    Size += ALIGN(pLL->MBf * sizeof(FLAG));

    pState->pMVHistBuffer = (vc1_sMotionHist *)((char *)pState + Size);    

    /* Level is specified externally for simple, main profile */
    pState->sSeqParams.eLevel = pConfig->eLevel;

    /* The framerate may be specified externally if not set by the bitstream */
    pState->sSeqParams.FrameRateNumerator = pConfig->FrameRateNumerator;
    pState->sSeqParams.FrameRateDenominator = pConfig->FrameRateDenominator;

    return(vc1_ResultOK);
}


/*
 * Description:
 * Decode the sequence layer of the bitstream
 *
 * Remarks:
 *
 * Inputs:
 * pState - pointer to the decoder state structure
 * pBitstream - pointer to the bitstream structure
 *
 * Outputs:
 * pState - updated with new sequence layer information
 * pBitstream - updated with new bitstream position
 *
 * Return Value:
 * Standard vc1_eResult result. See enumeration for possible result codes.
 * 
 */

vc1_eResult vc1DEC_DecodeSequence(vc1DEC_sState * pState, vc1DEC_sBitstream * pBitstream)
{
    vc1_eResult eResult;

    eResult = vc1DECSEQ_UnpackSequenceLayer(pState, pBitstream);
    if(eResult != vc1_ResultOK)
    {
        return(eResult);
    }

    /*
     * Large block just to output the coverage information in debug builds
     */
    if (vc1_ProfileSimple == pState->sSeqParams.eProfile)
    {
        if (vc1_LevelLow == pState->sSeqParams.eLevel)
        {
            COVERAGE("D.1:SP@LL");
        }
        else if (vc1_LevelMedium == pState->sSeqParams.eLevel)
        {
            COVERAGE("D.1:SP@ML");
        }
    }
    else if (vc1_ProfileMain == pState->sSeqParams.eProfile)
    {
        if (vc1_LevelLow == pState->sSeqParams.eLevel)
        {
            COVERAGE("D.1:MP@LL");
        }
        else if (vc1_LevelMedium == pState->sSeqParams.eLevel)
        {
            COVERAGE("D.1:MP@ML");
        }
        else if (vc1_LevelHigh == pState->sSeqParams.eLevel)
        {
            COVERAGE("D.1:MP@HL");
        }
    }
    else if (vc1_ProfileAdvanced == pState->sSeqParams.eProfile)
    {
        if (vc1_LevelL1 == pState->sSeqParams.eLevel)
        {
            COVERAGE("D.1:AP@L1");
        }
        else if (vc1_LevelL2 == pState->sSeqParams.eLevel)
        {
            COVERAGE("D.1:AP@L2");
        }
        else if (vc1_LevelL3 == pState->sSeqParams.eLevel)
        {
            COVERAGE("D.1:AP@L3");
        }
        else if (vc1_LevelL4 == pState->sSeqParams.eLevel)
        {
            COVERAGE("D.1:AP@L4");
        }
    }

    /* set to true, as first frame in the stream will be buffered */
    pState->FirstFrameInStream = TRUE;

    return(vc1_ResultOK);
}


/*
 * Description:
 * Decode a frame
 *
 * Inputs:
 * pState       - pointer to the decoder state structure
 * pBitstream   - pointer to the bitstream structure
 * pPicture     - pointer to an initialised picture structure
 *
 * Outputs:
 * pState       - updated with data read from the bitstream
 * pBitstream   - updated with new bitstream position
 * pPicture     - updated with decoded picture information
 *
 * Return Value:
 * Standard vc1_eResult result. See enumeration for possible result codes.
 * 
 */

vc1_eResult vc1DEC_DecodeFrame( vc1DEC_sState * pState,
                                vc1DEC_sBitstream * pBitstream,
                                vc1_sPicture * pPicture)
{
    vc1_eResult eResult = vc1_ResultOK;

    DEBUG1(vc1DEBUG_API, "Decoding Frame %d\n", pState->FrameNum);
    pState->pPicture = pPicture;
    
    eResult = vc1DECPIC_UnpackPictureLayer(pState, pBitstream);
    DEBUG1(vc1DEBUG_API, "Decode Frame returns %d\n", eResult);

    /* FrameNum is always the number of the frame for which data is being returned */
    pState->FrameNum++;

    return(eResult);
}


/*
 * Description:
 * Decode a field
 *
 * Remarks:
 * Must be called after a call to vc1DEC_DecodeFrame() has been made, otherwise
 * most of the state will be invalid.
 *
 * Inputs:
 * pState       - pointer to the decoder state structure
 * pBitstream   - pointer to the bitstream structure
 * pPicture     - pointer to an initialised picture structure
 *
 * Outputs:
 * pState       - updated with data read from the bitstream
 * pBitstream   - updated with new bitstream position
 * pPicture     - updated with decoded picture information
 *
 * Return Value:
 * Standard vc1_eResult result. See enumeration for possible result codes.
 * 
 */

vc1_eResult vc1DEC_DecodeField( vc1DEC_sState * pState,
                                vc1DEC_sBitstream * pBitstream,
                                vc1_sPicture * pPicture)
{
    vc1_eResult eResult = vc1_ResultOK;

    DEBUG0(vc1DEBUG_API, "Decoder Field\n");
    pState->pPicture = pPicture;
    pState->sPosition.SecondField = 1;

    eResult = vc1DECPIC_UnpackFieldPictureLayer(pState, pBitstream);
    DEBUG1(vc1DEBUG_API, "Decode Field returns %d\n", eResult);

    return(eResult);
}

/*
 * Description:
 * Decode a slice
 *
 * Remarks:
 * Must be called after a call to vc1DEC_DecodeFrame() or vc1DEC_DecodeField()
 * has been made, otherwise most of the state will be invalid.
 *
 * Inputs:
 * pState       - pointer to the decoder state structure
 * pBitstream   - pointer to the bitstream structure
 * pPicture     - pointer to an initialised picture structure
 *
 * Outputs:
 * pState       - updated with data read from the bitstream
 * pBitstream   - updated with new bitstream position
 * pPicture     - updated with decoded picture information
 *
 * Return Value:
 * Standard vc1_eResult result. See enumeration for possible result codes.
 * 
 */

vc1_eResult vc1DEC_DecodeSlice( vc1DEC_sState * pState,
                                vc1DEC_sBitstream * pBitstream,
                                vc1_sPicture * pPicture)
{
    vc1_eResult eResult = vc1_ResultOK;

    DEBUG0(vc1DEBUG_API, "Decode Slice\n");
    pState->pPicture = pPicture;

    eResult = vc1DECSLICE_UnpackSliceLayer(pState, pBitstream);
    DEBUG1(vc1DEBUG_API, "Decode Slice returns %d\n", eResult);

    return(eResult);
}


/*
 * Description:
 * Decode the entry point layer of the bitstream
 *
 * Remarks:
 *
 * Inputs:
 * pState       - pointer to the decoder state structure
 * pBitstream   - pointer to the bitstream structure
 *
 * Outputs:
 * pState       - updated with new entry point layer information
 * pBitstream   - updated with new bitstream position
 *
 * Return Value:
 * Standard vc1_eResult result. See enumeration for possible result codes.
 * 
 */

vc1_eResult vc1DEC_DecodeEntryPoint(vc1DEC_sState * pState, vc1DEC_sBitstream * pBitstream)
{
    vc1_eResult eResult = vc1_ResultOK;

    DEBUG0(vc1DEBUG_API, "Decoding entry point layer\n");

    eResult = vc1DECENT_UnpackEntryPointLayer(pState, pBitstream);

    return(eResult);
}



/*
 * Description:
 * Return the final picture in a stream
 *
 * Remarks:
 * The decoder has already decoded and buffered the final (reference) picture in a stream, 
 * so this  * function should be called to flush that buffer to the application.
 *
 * Inputs:
 * pState - pointer to the decoder state structure
 * pPicture - pointer to an initialised picture structure,
 *            into which the new picture will be written
 *
 * Outputs:
 * pPicture - updated with decoded picture information
 *
 * Return Value:
 * Standard vc1_eResult result. See enumeration for possible result codes.
 * 
 */

vc1_eResult vc1DEC_DecodeFlush(vc1DEC_sState * pState, vc1_sPicture * pPicture)
{
    pState->pPicture = pPicture;

    DEBUG0(vc1DEBUG_API, "Flushing decoded reference picture\n");

    vc1DECPIC_DisplayPicture(pState, pState->sPosition.pReferenceNew);

    DEBUGREFPICT(vc1DEBUG_REFPICT, pState->sPosition.pReferenceOld, "DecRef");
    DEBUGREFPICT(vc1DEBUG_REFPICT, pState->sPosition.pReferenceNew, "DecRef");

    return(vc1_ResultOK);
}



/*
 * Description:
 * Update the internal bitstream buffers
 *
 * Remarks:
 * Updates the hypothetical reference decoder buffers, if used by the
 * bitstream, and will warn on underflow or overflow. If HRD isn't used
 * the function does nothing, and returns vc1_ResultOK.
 *
 * Inputs:
 * pState   - pointer to the decoder state structure
 * pPicture - number of bits to deduct from the internal buffer
 * Frame    - set to true if a complete frame has been decoded. Bits will
 *            be added to the buffer.
 *
 * Outputs:
 * pState   - HRD model is updated
 *
 * Return Value:
 * Standard vc1_eResult result. See enumeration for possible result codes.
 * 
 */

vc1_eResult vc1DEC_UpdateBuffers(vc1DEC_sState * pState, UWORD32 Bits)
{
    vc1_eResult eResult = vc1_ResultOK;

    /* Update HRD and check there are no more bits left */
    if(0 != pState->sSeqParams.sHrdInitialState.NumLeakyBuckets)
    {
        DEBUG1(vc1DEBUG_HRD, "Removing %d bits from HRD model\n", Bits);
        eResult = vc1HRD_RemoveBits(&pState->sSeqParams.sHrdInitialState, Bits);
        if(vc1_ResultOK != eResult)
        {
            WARN("HRD: Buffer underflow, bit count = %d\n", Bits);
        }
        COVERAGE("ANNEX C");

        /* Add the new bits that arrived in this frame duration */
        vc1HRD_AddBits( &pState->sSeqParams.sHrdInitialState, 
                        pState->sSeqParams.FrameRateDenominator,
                        pState->sSeqParams.FrameRateNumerator);
    }

    return(eResult);
}


