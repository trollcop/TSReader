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
 * vc1dec.h:
 * Decoder library functions and structs
 *
 */

#ifndef VC1DEC_H
#define VC1DEC_H

#ifdef cplusplus
extern "C" {
#endif

/**
 * Description:
 * Structure containing the fields defining a bitstream.
 */

typedef struct
{
    UBYTE8  *pBitBuffer;        /** Pointer to byte containing the next bit in the bitstream */

    UBYTE8  *pEndByte;          /** Pointer to the byte after the last used
                                 *  byte of the bitstream buffer */
    UBYTE8  EndBitsValid;       /** Number of bits valid in the last used byte 1 to 8 */

    UBYTE8  EncapsulatedIDU;    /** Non-zero if the independently decodable units 
                                    are encapsulated */
    UWORD32 ZeroRun;            /** Number of consecutive zeroes read from the bitstream */
    UWORD32 BitCounter;         /** Number of bits read from bitstream, for use with HRD */

    UWORD32 Buffer0;            /** First half of 64-bit holding buffer */
    UWORD32 Buffer1;            /** Second half of 64-bit holding buffer */
    UBYTE8  BitsUsed;           /** Number of used bits in the buffer  0 to 31 */
    UBYTE8  BitsValid;          /** Number of valid bits in the buffer 0 to 64 */ 
} vc1DEC_sBitstream;


/**
 * Description:
 * Structure for holding a variable length code.
 *
 * Remarks:
 * In an array of these, entry 0 has a special meaning:
 *  - Bits   = 0;
 *  - Length = Number of codes in the array (Length of array - 1)
 *  - Value  = Maximum code length
 */

typedef struct
{
    UWORD32     Bits;               /** Bit pattern of the code */
    UBYTE8      Length;             /** Length of the code */
    UWORD32     Value;              /** Value that the code represents */
} vc1DEC_sVLCCode;


/**
 * Description:
 * Structure to contain bitplane coding information.
 */
typedef struct
{
    FLAG    *pBitplane;     /** Pointer to bitplane table. NULL indicates RAW mode */
    FLAG    RawMode;        /** True if raw mode is used in this bitplane */
    UWORD32 Position;       /** Index of the next bit in the bitplane */
} vc1DEC_sBitplane;


/**
 * Description:
 * Structure containing the parameters read from the bitstream for the picture layer.
 * This is derived from the picture layer section of the standard.
 *
 */
typedef struct
{
    UBYTE8                  FrameCount;                     /** See the standard */
    vc1_ePictureType        ePictureType[2];                /** See the standard */
    UBYTE8                  BufferFullness;                 /** See the standard */
    UBYTE8                  PQIndex;                        /** See the standard */
    vc1_eQuantizer          eQuantizer;                     /** Per-picture quantizer */
    UBYTE8                  PQuant;                         /** PQuant value */
    FLAG                    HalfQPStep;                     /** See the standard */
    UBYTE8                  FrameTransformACCodingSetIndex; /** See the standard */
    UBYTE8                  FrameTransformACCodingSetIndex2; /** See the standard */
    FLAG                    IntraTransformDCTable;          /** See the standard */

    UBYTE8                  TemporalRefFrameCounter;        /** See the standard */
    FLAG                    TopFieldFirst;                  /** See the standard */
    FLAG                    RepeatFirstField;               /** See the standard */
    FLAG                    UVSampleMode;                   /** See the standard */
    vc1_ePostProcessing     ePostProcessing;                /** See the standard */

    vc1_eQuantMode          eQuantMode;                     /** Quantization mode */
    UBYTE8                  AltPQuant;                      /** See the standard */

    vc1_sInterpolate        sInterpolate;                   /** See the standard */
    const vc1DEC_sVLCCode   *pMotionVectorTable;            /** See the standard */
    const vc1DEC_sVLCCode   *pCodedBlockPatternTable;       /** See the standard */
    FLAG                    MBTransformTypeFlag;            /** See the standard */
    vc1_eBlkType            eFrameTransformType;            /** See the standard */

    UBYTE8                  RepeatFrameCount;               /** See the standard */
    FLAG                    FrameInterpolationHint;         /** See the standard */
    vc1_eCondOver           eConditionalOverlap;            /** See the standard */
    vc1_sPanScanParams      sPanScanParams;                 /** See the standard */

    FLAG                    DQuantFrame;                    /** Per MB quantisation mode */

    vc1DEC_sBitplane        sBPPredictAC;                   /** Bitplane for AC prediction */
    vc1DEC_sBitplane        sBPSkipMB;                      /** Bitplane for MB skip */
    vc1DEC_sBitplane        sBPMotionVectorType;            /** Bitplane for MV type */
    vc1DEC_sBitplane        sBPBFrameDirectMode;            /** Bitplane for Direct MB */
    vc1DEC_sBitplane        sBPOverflags;                   /** Bitplane for overlap flags */
    vc1DEC_sBitplane        sBPForwardMB;                   /** Bitplane for Forward MB */
    vc1DEC_sBitplane        sBPFieldTX;                     /** Bitplane for FieldTX MB */

    FLAG                    ExtendHorizDMVRange;            /** Extend horizontal differential MV */
    FLAG                    ExtendVertDMVRange;             /** Extend vertical differential MV */

    const vc1DEC_sVLCCode   *pMBModeTable;                  /** See the standard */
    const vc1DEC_sVLCCode   *pMB4MVBlockPatternTable;       /** See the standard */
    const vc1DEC_sVLCCode   *pMB2MVBlockPatternTable;       /** See the standard */

    vc1_sIntensityComp      sIC[2];   /** Intensity compensation for 0=Top 1=Bottom fields */
} vc1DEC_sPictureLayerParams;


/**
 * Description:
 * Structure containing the state of the decoder.
 */

typedef struct
{
    vc1_sPosition           sPosition;              /** Macroblock position structure */
    vc1_sPicture            *pPicture;              /** Picture information */

    int                     FrameNum;               /** Current frame number */
    
                                                    /* Pointer to other data structures */
    vc1_sMB                 *pMB;                   /** Pointer to macroblock data */
                                                    /* Limits */

    int                     NumFields;              /** Number of fields per frame */
    int                     MaxMBs;                 /** Maximum number of macroblocks */
    const vc1_sLevelLimit   *pLevelLimit;           /** Limits for current level and profile */

                                                    /* Configuration */
    vc1_sSequenceLayer      sSeqParams;             /** Data from the sequence layer */
    vc1DEC_sPictureLayerParams sPicParams;          /** Data from the current picture layer */

    UBYTE8                  NotFirstMode3InFrame;   /** 1 if not first mode 3 escape in frame */
                                                    /* (8.1.1.10 AC coef decoder) */
    UBYTE8                  LevelCodeSize;          /** Level code size for mode 3 escape,
                                                        per frame */
    UBYTE8                  RunCodeSize;            /** Run code size for mode 3 escape, 
                                                        per frame */

    UBYTE8                  ZigZagTableIndex;       /** Index to select zigzag table */
    FLAG                    FirstFrameInStream;     /** Non-zero if this is the first frame */
    FLAG                    BitplaneCodingUsed;     /** Non-zero if bitplane coding is in use */

    UBYTE8                  FirstCodedBlock;        /** First coded block in current macroblock */

    vc1_sReferencePicture   *pCurrentRef;           /** Pointer to the current reference picture
                                                        into which blocks are decoded */
    vc1_sMotionHist         *pMVHistBuffer;         /** Pointer to the motion vector
                                                        history buffer */
    UWORD32                 FieldCount;             /** Number of fields present in the current 
                                                        picture */
} vc1DEC_sState;

/**
 * Description:
 * Structure containing configuration information for the decoder. This structure
 * contains information available to the application from demultiplexing the container format.
 *
 */


typedef struct
{
    UHWD16          MaxCodedWidth;  /** Maximum coded picture width */
    UHWD16          MaxCodedHeight; /** Maximum coded picture height */
    vc1_eProfile    eProfile;       /** Highest profile supported by the decoder */
    vc1_eLevel      eLevel;         /** Highest level supported by the decoder */

    UWORD32         FrameRateNumerator;   /** Explicit default framerate numerator 
                                           *  (0 if not specified) */
    UHWD16          FrameRateDenominator; /** Explicit default framerate denominator
                                           *  - 1000, 1001, or 32 (0 if not specified) */
} vc1DEC_sDecoderConfiguration;

/* function prototypes */

/**
 * Description:
 * Obtain memory requirements for decoding a bitstream.
 *
 * Remarks:
 * Examines the profile (and if present, the level) to determine the memory requirement.
 *
 * Inputs:
 * pSize      - pointer to UWORD32 into which the memory requirement will be written
 * pConfig    - pointer to a configuration structure, used to determine memory requirement
 * pBitstream - pointer to the start of the bitstream
 *
 * Outputs:
 * pSize      - updated with memory requirement
 * pConfig    - updated with profile and level from bitstream.
 *
 * Return Value:
 * Standard vc1_eResult result. See enumeration for possible result codes.
 * 
 */
vc1_eResult vc1DEC_DecoderRequirements( UWORD32 *pSize,
                                        vc1DEC_sDecoderConfiguration *pConfig,
                                        vc1DEC_sBitstream * pBitstream);


/**
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
vc1_eResult vc1DEC_DecoderInitialise(vc1DEC_sState *pState,
                                     vc1DEC_sDecoderConfiguration *pConfig);

/**
 * Description:
 * Decode the sequence layer of the bitstream.
 *
 * Inputs:
 * pState     - pointer to the decoder state structure
 * pBitstream - pointer to the bitstream structure
 *
 * Outputs:
 * pState     - updated with new sequence layer information
 * pBitstream - updated with new bitstream position
 *
 * Return Value:
 * Standard vc1_eResult result. See enumeration for possible result codes.
 * 
 */
vc1_eResult vc1DEC_DecodeSequence(vc1DEC_sState * pState, vc1DEC_sBitstream * pBitstream);


/**
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

vc1_eResult vc1DEC_DecodeFrame(
    vc1DEC_sState * pState,
    vc1DEC_sBitstream * pBitstream,
    vc1_sPicture * pPicture
);


/**
 * Description:
 * Decode a field.
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
                                vc1_sPicture * pPicture);


/**
 * Description:
 * Decode a slice.
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
                                vc1_sPicture * pPicture);



/**
 * Description:
 * Decode the entry point layer of the bitstream.
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
vc1_eResult vc1DEC_DecodeEntryPoint(vc1DEC_sState * pState, vc1DEC_sBitstream * pBitstream);


/**
 * Description:
 * Return the final picture in a stream.
 *
 * Remarks:
 * The decoder must buffer reference pictures in order to return B frames in the
 * correct sequence, so this function is called to flush that buffer to the
 * application.
 *
 * Inputs:
 * pState   - pointer to the decoder state structure
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
vc1_eResult vc1DEC_DecodeFlush(vc1DEC_sState * pState, vc1_sPicture * pPicture);


/**
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
 * Bits     - number of bits used
 *
 * Outputs:
 * pState   - HRD model is updated
 *
 * Return Value:
 * Standard vc1_eResult result. See enumeration for possible result codes.
 * 
 */

vc1_eResult vc1DEC_UpdateBuffers(vc1DEC_sState * pState, UWORD32 Bits);




/* Some tables use an escape value */
#define ESCAPE  ((UWORD32)(-2))

#ifdef cplusplus
}
#endif
#endif
