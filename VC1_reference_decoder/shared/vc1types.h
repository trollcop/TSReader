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
* vc1types.h
 *
 * Data types used by the SMPTE VC-1 reference code.
 *
 */

#ifndef VC1TYPES_H
#define VC1TYPES_H

#ifdef cplusplus
extern "C" {
#endif

/**
 * General types
 */

typedef   signed char   BYTE8;      /** Signed 8 bit type */
typedef unsigned char  UBYTE8;      /** Unsigned 8 bit type */
typedef   signed short  HWD16;      /** Signed 16 bit type */
typedef unsigned short UHWD16;      /** Unsigned 16 bit type */
typedef   signed int    WORD32;     /** Signed 32 bit type */
typedef unsigned int   UWORD32;     /** Unsigned 32 bit type */


#ifdef __STRICT_ANSI__
    /* ANSI does not support long long so define
     * an emulated equivalent for the 64-bit type
     */
    typedef struct {
        UWORD32 Low;
        UWORD32 High;
    } ULLONG64;
#else
    /* Define a 64-bit type according to compiler */
    #if defined(_MSC_VER)
        typedef unsigned __int64 ULLONG64;      /** Unsigned 64 bit type */
        typedef   signed __int64  LLONG64;      /** Signed 64 bit type */
    #else
        typedef unsigned long long ULLONG64;    /** Unsigned 64 bit type */
        typedef   signed long long  LLONG64;    /** Signed 64 bit type */
    #endif
#endif

typedef UBYTE8  FLAG;       /** Boolean type */
typedef UWORD32 HRDVALUE;


#if defined(TRUE) && (TRUE != 1)
#error "TRUE previously defined not equal to 1"
#endif
#if defined(FALSE) && (FALSE != 0)
#error "FALSE previously defined not equal to 0"
#endif

#ifndef TRUE
#define TRUE (1)
#endif
#ifndef FALSE
#define FALSE (0)
#endif
#ifndef NULL
#define NULL (0)
#endif

/*
 * Macro to explicitly mark unused parameters
 */
//#define IGNORE(_a_) ((void)(_a_))


/*
 * Some systems require alignment of data. When manually calculating
 * the size of workspace blocks, we need to manually align to an
 * appropriate boundary. These macros just aligns to the worst-case
 * requirement.
 */
#define ALIGNMENT   (8)       /* Worst case is requiring alignment to an 8 byte boundary */
#define ALIGN(_s_) ((_s_+(ALIGNMENT-1)) & ~(ALIGNMENT-1))

/* Small utility defines */

#define SIGN(x)     (((x) < 0) ? -1 : 1)                                /** Sign of x */
#define ABS(x)      (((x) >= 0) ? (x) : -(x))                           /** Absolute value of x */
#define MIN(x, y)   (((x) < (y)) ? (x) : (y))                           /** Minimum of x and y */
#define MAX(x, y)   (((x) > (y)) ? (x) : (y))                           /** Maximum of x and y */
#define CLIP(x)     ((UBYTE8)((x) < 0 ? 0 : ((x) > 255 ? 255 : (x))))   /** x clipped to 0 - 255 */

/*
 * Common typedefs
 */

/**
 * Description: Function return codes.
 */
typedef enum
{
    /* General errors */
    vc1_ResultFatal = -2,               /** Fatal condition detected */
    vc1_ResultWarn = -1,                /** Continuable fault detected */

    /* Results that are not errors */
    vc1_ResultOK = 0,                   /** Function completed successfully */
    vc1_ResultNoFrame,                  /** No frame to display (due to buffering) */
    vc1_ResultSlice,                    /** More slice data required to complete decode */
    vc1_ResultField,                    /** Second field data required to complete decode */

    /* Specific errors */
    vc1_ResultInvalidParameter,         /** Option value not accepted */
    vc1_ResultBadFile,                  /** File will not open */
    vc1_ResultBadLine,                  /** Line in option file not parseable */
    vc1_ResultBadType,                  /** Invalid type for parameter */
    vc1_ResultNoMemory,                 /** malloc() failed */
    vc1_ResultNoData,                   /** Failed to read picture data */
    vc1_ResultBufferExhausted,          /** No more data to read from buffer */
    vc1_ResultBadImageSize,             /** Size breaks restrictions set by the standard */
    vc1_ResultImageTooBig,              /** Size bigger than profile limits */
    vc1_ResultImageSizeChanged,         /** Size changed in simple or main profile */
    vc1_ResultUnsupportedTransform,     /** Invalid transform */
    vc1_ResultACRunLevelDecodeFailed,   /** Decoder failed to read the AC coef run levels */
    vc1_ResultInvalidPicture,
    vc1_ResultInvalidConfiguration,
    vc1_ResultBadReferencePicture,
    vc1_ResultHrdUnderflow,
    vc1_ResultHrdOverflow,
    vc1_ResultNoStartCode               /** Start code not found in file input data */
} vc1_eResult;

/**
 * Packed Run Level information
 */
typedef HWD16 vc1_tRunLevel;

#define vc1_RunLevelRun     0x0000      /** Run value */
#define vc1_RunLevelPlus    0x0001      /** +ve level value */
#define vc1_RunLevelMinus   0x0002      /** -ve level value */
#define vc1_RunLevelMask    3           /** Mask of run level */
#define vc1_RunLevelShift   2           /** bits 4-15 hold the |level| or run value */

/**
 * Description: Block Types
 */
typedef enum
{
    /* Inter blocks */
    vc1_BlkInter8x8 = 0,        /** Inter coded block, 8px wide, 8px high subblocks */
    vc1_BlkInter8x4 = 1,        /** Inter coded block, 8px wide, 4px high subblocks */
    vc1_BlkInter4x8 = 2,        /** Inter coded block, 4px wide, 8px high subblocks */
    vc1_BlkInter4x4 = 3,        /** Inter coded block, 4px wide, 4px high subblocks */
    vc1_BlkInterAny,            /** Inter coded block, transform not yet chosen */
    /* Intra blocks */
    vc1_BlkIntra,               /** Intra coded block, no AC prediction */
    vc1_BlkIntraTop,            /** Intra coded block, AC prediction of TOP values */
    vc1_BlkIntraLeft            /** Intra coded block, AC prediction of LEFT values */
} vc1_eBlkType;

#define vc1_BlkTypeIsIntra(BlkType) ((BlkType) >= vc1_BlkIntra) /** Test if BlkType is Intra */
#define vc1_BlkTypeIsInter(BlkType) ((BlkType) < vc1_BlkIntra)  /** Test if BlkType is Inter */

/*
 * Block level types
 */

/* Block number */

/**
 * Description: Block number
 */

typedef enum
{
    vc1_BlkY0 = 0,          /** Top left Y block in MB */
    vc1_BlkY1 = 1,          /** Top right Y block in MB */
    vc1_BlkY2 = 2,          /** Bottom left Y block in MB */
    vc1_BlkY3 = 3,          /** Bottom right Y block in MB */
    vc1_BlkCb = 4,          /** Cb block in MB */
    vc1_BlkCr = 5,          /** Cr block in MB */
    VC1_BLOCKS_PER_MB = 6   /** Number of blocks per MB */
} vc1_eBlkNumber;

#define vc1_BlkNumIsLuma(BlkNum)   ((BlkNum) <= vc1_BlkY3 ) /** Test if BlkNum is Luma */
#define vc1_BlkNumIsChroma(BlkNum) ((BlkNum) >= vc1_BlkCb ) /** Test if BlkNum is Chroma */

/* Block data */

typedef UHWD16 vc1_NumZeroCoef;         /* (NUMZERO<<8)+(NUMCOEF) */

/**
 * Description:
 * Structure defining the state required for intra blocks.
 */

typedef struct
{
    vc1_NumZeroCoef     NZC;            /** NUMZERO and NUMCOEF (excludes DC) */
    HWD16               DC;             /** Quantized DC for prediction */
    HWD16               ACTop[7];       /** Quantized AC top row for prediction */
    HWD16               ACLeft[7];      /** Quantized AC left column for prediction */
    HWD16               SmoothRows[16]; /** Bottom two rows kept for overlap smoothing */
} vc1_sBlkIntra;

typedef enum
{
    vc1_HybridLeft = 0,                 /** Predict from Left */
    vc1_HybridTop  = 1,                 /** Predict from Top  */
    vc1_HybridNone = 2                  /** No hybrid prediction specified */
} vc1_eHybridPred;


/**
 * Description:
 * Subblock pattern enumeration for TTMB and TTBLK
 */

typedef enum
{
    /* Block level transform configurations */
    vc1_SBP8x8         = 0,     /** 8x8 transform, coded */
    vc1_SBP8x4Bottom   = 1,     /** 8x4 transform, bottom subblock coded */
    vc1_SBP8x4Top      = 2,     /** 8x4 transform, top subblock coded */
    vc1_SBP8x4Both     = 3,     /** 8x4 transform, both subblocks coded */
    vc1_SBP4x8Right    = 4,     /** 4x8 transform, right subblock coded */
    vc1_SBP4x8Left     = 5,     /** 4x8 transform, left subblock coded */
    vc1_SBP4x8Both     = 6,     /** 4x8 transform, both subblocks coded */
    vc1_SBP4x4         = 7,     /** 4x4 transform, subblock pattern separate */

    /* MB level transform configurations */
    vc1_SBP8x8MB       = 8,     /** 8x8 transform, coded, whole MB */
    vc1_SBP8x4BottomMB = 9,     /** 8x4 transform, bottom subblock coded, whole MB */
    vc1_SBP8x4TopMB    = 10,    /** 8x4 transform, top subblock coded, whole MB */
    vc1_SBP8x4BothMB   = 11,    /** 8x4 transform, both subblocks coded, whole MB */
    vc1_SBP4x8RightMB  = 12,    /** 4x8 transform, right subblock coded, whole MB */
    vc1_SBP4x8LeftMB   = 13,    /** 4x8 transform, left subblock coded, whole MB */
    vc1_SBP4x8BothMB   = 14,    /** 4x8 transform, both subblocks coded, whole MB */
    vc1_SBP4x4MB       = 15,    /** 4x4 transform, subblocks pattern separate, whole MB */

    vc1_SBPMBLevel     = 8      /** MB level threshold */
} vc1_eSBP;

#define vc1_SBPIsMBLevel(SBP) ((SBP) >= vc1_SBPMBLevel) /** Test if SBP applies to whole MB */


/**
 * Description:
 * Structure to hold a motion vector.
 */

typedef struct
{
    HWD16       X;                      /** X component of the motion vector (offset to target) */
    HWD16       Y;                      /** Y component of the motion vector (offset to target) */
    FLAG        BottomField;            /** 0=TopField 1=BottomField */
} vc1_sMV;

/**
 * Description:
 * Structure defining motion vector and associated parameters.
 */

typedef struct
{
    vc1_eHybridPred     eHybridPred;    /** Hybrid Prediction mode */
    vc1_sMV             sMV;            /** Motion vector */
    vc1_sMV             sDMV;           /** Differential motion vector (X,Y) in 1/4 pel units */
                                        /*
                                         * If Two Reference Images (NUMREF=1) then:
                                         *      Y=2*(YValue)+PredFlag
                                         *      PredFlag: 0=dominant 1=non-dominant
                                         */
} vc1_sMotion;


#define vc1_MVIntra (-0x8000)    /* Motion vector value for an Intra block */

/**
 * Description:
 * Structure defining motion vector history information to store for Direct mode.
 */
typedef struct
{
    vc1_sMV Y[4];       /** Motion vectors for the 4 Y blocks */
} vc1_sMotionHist;

/**
 * Description:
 * Structure defining the state required for inter blocks.
 */

typedef struct
{
    vc1_NumZeroCoef     NZC[4];         /** NUMZERO and NUMCOEF for sub-blocks (includes DC) */
    vc1_sMotion         sMotion[2];     /** Forward and Backward motion parameters */
} vc1_sBlkInter;

/**
 * Description:
 * Structure defining the state required for a block.
 */

typedef struct
{
    vc1_eBlkType        eBlkType;       /** Block type */
    FLAG                Coded;          /** Non zero AC coefficients for Intra,
                                            non zero AC/DC for Inter */
    union
    {
        vc1_sBlkIntra   sIntra;         /** Intra block state information */
        vc1_sBlkInter   sInter;         /** Inter block state information */
    }                   u;              /** Intra/Inter union */
} vc1_sBlk;

/*
 * Macroblock level types
 */


/**
 * Description:
 * Quantizer structure.
 */

typedef struct
{
    UBYTE8  Quant;              /** Quantizer Step in the range 1 to 31 */
    UBYTE8  HalfStep;           /** Quantizer Half Step value 0 or 1 */
    FLAG    NonUniform;         /** Selects Uniform/NonUniform Quantizer */
} vc1_sQuant;

/**
 * Description:
 * Macroblock type bitmap
 */

typedef enum
{
    /* Motion vector type bits [1:0] */
    vc1_MBIntra    = 0x00000000,    /** Intra (no motion vectors) */
    vc1_MB1MV      = 0x00000001,    /** One motion vector  */
    vc1_MB2MV      = 0x00000002,    /** Two motion vectors */
    vc1_MB4MV      = 0x00000003,    /** Four motion vectors */
    vc1_MBMVMask   = 0x00000003,    /** Mask of MV information */

    /* Motion vector direction bits [3:2] */
    vc1_MBDirect   = 0x00000000,    /** Direct macroblock */
    vc1_MBForward  = 0x00000004,    /** Forward prediction */
    vc1_MBBackward = 0x00000008,    /** Backward prediction */
    vc1_MBInterp   = 0x0000000C,    /** Forward and backward */
    vc1_MBDirMask  = 0x0000000C,    /** Mask of direction information */

    /* Flags */
    vc1_MBFieldMV  = 0x00000010,    /** MV's apply to Fields */
    vc1_MBSwitchMV = 0x00000020,    /** Bottom field different direction to Top */
    vc1_MBFieldTX  = 0x00000040     /** Field transform */

} vc1_eMBType;

/** Test if macroblock type is intra */
#define vc1_MBTypeIsIntra(eMBType)      ((eMBType & vc1_MBMVMask)==vc1_MBIntra)
/** Test if macroblock type is inter */
#define vc1_MBTypeIsInter(eMBType)      ((eMBType & vc1_MBMVMask)!=vc1_MBIntra)
/** Test if macroblock type is 1MV */
#define vc1_MBTypeIs1MV(eMBType)        ((eMBType & vc1_MBMVMask)==vc1_MB1MV)
/** Test if macroblock type is 2MV */
#define vc1_MBTypeIs2MV(eMBType)        ((eMBType & vc1_MBMVMask)==vc1_MB2MV)
/** Test if macroblock type is 4MV */
#define vc1_MBTypeIs4MV(eMBType)        ((eMBType & vc1_MBMVMask)==vc1_MB4MV)

/** Test if macroblock type is direct predicted */
#define vc1_MBTypeIsDirect(eMBType)     ((eMBType & vc1_MBDirMask)==vc1_MBDirect)
/** Test if macroblock type is forward predicted */
#define vc1_MBTypeIsForward(eMBType)    ((eMBType & vc1_MBDirMask)==vc1_MBForward)
/** Test if macroblock type is backward predicted */
#define vc1_MBTypeIsBackward(eMBType)   ((eMBType & vc1_MBDirMask)==vc1_MBBackward)
/** Test if macroblock type is interpolated */
#define vc1_MBTypeIsInterp(eMBType)     ((eMBType & vc1_MBDirMask)==vc1_MBInterp)

/** Test if macroblock type is field transform */
#define vc1_MBTypeIsFieldTX(eMBType)    ((eMBType & vc1_MBFieldTX)!=0)
/** Test if macroblock type is field motion vector */
#define vc1_MBTypeIsFieldMV(eMBType)    ((eMBType & vc1_MBFieldMV)!=0)
/** Test if macroblock type is switch motion vector */
#define vc1_MBTypeIsSwitchMV(eMBType)   ((eMBType & vc1_MBSwitchMV)!=0)


/*
 * Description: AC prediction state
 */

typedef enum
{
    vc1_ACPredOff = 0,      /** AC prediction off */
    vc1_ACPredOn  = 1,      /** AC prediction on */
    vc1_ACPredAbsent        /** No blocks can be predicted */
} vc1_eACPred;


/**
 * Description: A structure to hold macroblock data.
 */

typedef struct
{
    vc1_eMBType     eMBType;            /** Macroblock type */
    vc1_eACPred     eACPred;            /** AC prediction status */
    vc1_eBlkType    eBlkType;           /** One of 8x8,8x4,4x8,4x4, or Any
                                            (Any=block based choice) */
    FLAG            OverlapFilter;      /** Overlap filter active for this macroblock */
    FLAG            Skipped;            /** Indicated macroblock is motion predicted only */
    UBYTE8          CBPCY;              /** Coded  block pattern, where:
                                          *  - bit 5 set means Y0 is coded
                                          *  - bit 4 set means Y1 is coded
                                          *  - bit 3 set means Y2 is coded
                                          *  - bit 2 set means Y3 is coded
                                          *  - bit 1 set means Cb is coded
                                          *  - bit 0 set means Cr is coded
                                          */
    UBYTE8          MVBP;               /** Motion vector block pattern, with:
                                          *  - bit3 set if dmv!=0 for Y0
                                          *  - bit2 set if dmv!=0 for Y1
                                          *  - bit1 set if dmv!=0 for Y2
                                          *  - bit0 set if dmv!=0 for Y3
                                          */
    vc1_sQuant      sQuant;             /** Macroblock Quantizer information */
    vc1_sBlk        sBlk[VC1_BLOCKS_PER_MB];    /** Block level information */
} vc1_sMB;

/**
 * Description: Format of a Picture.
 */
typedef enum
{
    vc1_ProgressiveFrame = 0,   /** Picture is a progressive frame */
    vc1_InterlacedFrame  = 1,   /** Picture is an interlaced frame */
    vc1_InterlacedField  = 2,   /** Picture is two interlaced fields */
    vc1_PictureFormatNone       /** Picture format not yet set */
} vc1_ePictureFormat;



/*
 * Sequence Layer types
 */

/**
 * Description:
 * Bitstream profile enumeration
 */

typedef enum
{
    vc1_ProfileSimple = 0,  /** Simple profile */
    vc1_ProfileMain,        /** Main profile */
    vc1_ProfileReserved,    /** Reserved */
    vc1_ProfileAdvanced     /** Advanced profile */
} vc1_eProfile;

/**
 * Description:
 * Profile level enumeration
 */

typedef enum
{
    vc1_LevelLow    = 0,    /** Simple/Main profile low level */
    vc1_LevelMedium = 1,    /** Simple/Main profile medium level */
    vc1_LevelHigh   = 2,    /** Simple/Main profile high level */

    vc1_LevelL0     = 0,    /** Advanced profile level 0 */
    vc1_LevelL1     = 1,    /** Advanced profile level 1 */
    vc1_LevelL2     = 2,    /** Advanced profile level 2 */
    vc1_LevelL3     = 3,    /** Advanced profile level 3 */
    vc1_LevelL4     = 4,    /** Advanced profile level 4 */

    /* 5 to 7 reserved */

    vc1_LevelUnknown = 255  /** Unknown profile */
} vc1_eLevel;

typedef enum
{
    vc1_ChromaFormatReserved = 0,
    vc1_ChromaFormat420,
    vc1_ChromaFormatReserved2,
    vc1_ChromaFormatReserved3
} vc1_eChromaFormat;

typedef enum
{
    vc1_ColorPrimariesForbidden = 0,
    vc1_ColorPrimariesITU_R_BT_709,
    vc1_ColorPrimariesUnspecified,
    vc1_ColorPrimariesReserved1,
    vc1_ColorPrimariesReserved2,
    vc1_ColorPrimariesEBU_Tech_3213,
    vc1_ColorPrimariesSMPTE_C,
    vc1_ColorPrimariesReserved3
    /* values 7 - 255 reserved */
} vc1_eColorPrimaries;

/*
 * Description:
 * Transfer Characteristics types for source picture
 */

typedef enum
{
    vc1_TransCharForbidden = 0,
    vc1_TransCharITU_R_BT_709,
    vc1_TransCharUnspecified,
    vc1_TransCharReserved1,
    vc1_TransCharReserved2,
    vc1_TransCharReserved3,
    vc1_TransCharReserved4,
    vc1_TransCharSMPTE_240M,
    vc1_TransCharReserved5
    /* values 8 - 255 reserved */
} vc1_eTransChar;

typedef enum
{
    vc1_MatrixCoefficientsForbidden = 0,
    vc1_MatrixCoefficientsITU_R_BT_709,
    vc1_MatrixCoefficientsUnspecified,
    vc1_MatrixCoefficientsReserved1,
    vc1_MatrixCoefficientsReserved2,
    vc1_MatrixCoefficientsReserved3,
    vc1_MatrixCoefficientsSMPTE_170M,
    vc1_MatrixCoefficientsSMPTE_240M,
    vc1_MatrixCoefficientsReserved4
    /* values 8 - 255 reserved */
} vc1_eMatrixCoefficients;

/**
 * Description:
 * Quantizer mode enumeration
 */
typedef enum
{
    vc1_QuantizerImplicit   = 0,    /** Quantizer implied by quantizer step size */
    vc1_QuantizerExplicit   = 1,    /** Quantizer explicitly signaled */
    vc1_QuantizerNonUniform = 2,    /** Non-uniform quantizer */
    vc1_QuantizerUniform    = 3     /** Uniform quantizer */
} vc1_eQuantizer;


/**
 * Description:
 * Picture type enumeration
 */
typedef enum
{
    vc1_PictureTypeI        = 0,    /** I Picture / Field - can be used as a reference */
    vc1_PictureTypeP        = 1,    /** P Picture / Field - can be used as a reference */
    vc1_PictureTypeB        = 2,    /** B Picture / Field */
    vc1_PictureTypeBI       = 3,    /** BI Picture / Field */
    vc1_PictureTypeSkipped  = 4     /** Skipped Frame */
} vc1_ePictureType;

/** Test if picture type is a reference (I or P) */
#define vc1_PictureTypeIsRef(Type)   ((Type)<=vc1_PictureTypeP)
/** Test if picture type is intra (I or BI) */
#define vc1_PictureTypeIsIntra(Type) ((Type)==vc1_PictureTypeI || (Type)==vc1_PictureTypeBI)
/** Test if picture type is inter (P or B) */
#define vc1_PictureTypeIsInter(Type) ((Type)==vc1_PictureTypeP || (Type)==vc1_PictureTypeB )

/**
 * Description: Scaling to be applied to decoded picture before display
 */
typedef enum
{
    vc1_PictureRes1x1=0,    /** No scaling */
    vc1_PictureRes2x1,      /** Scale horizontally */
    vc1_PictureRes1x2,      /** Scale vertically */
    vc1_PictureRes2x2       /** Scale horizontally and vertically */
} vc1_ePictureRes;

/**
 * Description:
 * Motion vector range enumeration
 */

typedef enum
{
    vc1_MVRange64_32    = 0,    /* x=-64 to 63.f by y=-32 to 31.f */
    vc1_MVRange128_64   = 1,
    vc1_MVRange512_128  = 2,
    vc1_MVRange1024_256 = 3
} vc1_eMVRange;

/**
 * Description:
 * Differential motion vector range enumeration
 */

typedef enum
{
    vc1_DMVRangeNone    = 0,    /* No extended DMV */
    vc1_DMVRangeX       = 1,    /* Extended DMV horizontal/X */
    vc1_DMVRangeY       = 2,    /* Extended DMV vertical/Y */
    vc1_DMVRangeXY      = 3     /* Extended DMV horizontal+vertical */
} vc1_eDMVRange;

typedef enum
{
    vc1_PostProcessingNone = 0,
    vc1_PostProcessingDeblock,
    vc1_PostProcessingDering,
    vc1_PostProcessingDeblockDering
} vc1_ePostProcessing;

/**
 * Description:
 * Per macroblock quantizer step size enumeration
 */
typedef enum
{
    vc1_QuantModeDefault,           /** All macroblocks use PQUANT */
    vc1_QuantModeAllEdges,          /** Edge macroblocks use ALTPQUANT */
    vc1_QuantModeLeftTop,           /** Left/Top     macroblocks use ALTPQUANT */
    vc1_QuantModeTopRight,          /** Top/Right    macroblocks use ALTPQUANT */
    vc1_QuantModeRightBottom,       /** Right/Bottom macroblocks use ALTPQUANT */
    vc1_QuantModeBottomLeft,        /** Bottom/Left  macroblocks use ALTPQUANT */
    vc1_QuantModeLeft,              /** Left         macroblocks use ALTPQUANT */
    vc1_QuantModeTop,               /** Top          macroblocks use ALTPQUANT */
    vc1_QuantModeRight,             /** Right        macroblocks use ALTPQUANT */
    vc1_QuantModeBottom,            /** Bottom       macroblocks use ALTPQUANT */
    vc1_QuantModeMBDual,            /** PQUANT/ALTPQUANT selected on macroblock basis */
    vc1_QuantModeMBAny              /** Any QUANT selected on a macroblock basis */
} vc1_eQuantMode;

/**
 * Description:
 * Bitplane coding mode.
 */

typedef enum
{
    vc1_BitplaneCodingModeNorm2 = 0,    /** Normal-2 bitplane coding */
    vc1_BitplaneCodingModeNorm6,        /** Normal-6 bitplane coding */
    vc1_BitplaneCodingModeRowskip,      /** Rowskip bitplane coding */
    vc1_BitplaneCodingModeColskip,      /** Colskip bitplane coding */
    vc1_BitplaneCodingModeDiff2,        /** Diff-2 bitplane coding */ 
    vc1_BitplaneCodingModeDiff6,        /** Diff-6 bitplane coding */
    vc1_BitplaneCodingModeRaw           /** Raw (No) bitplane coding */
} vc1_eBitplaneCodingMode;

typedef enum
{
    vc1_CondOverNone = 0,       /* No macroblocks overlap smooth */
    vc1_CondOverAll  = 1,       /* All macroblocks overlap smooth */
    vc1_CondOverSome            /* Selected macroblocks overlap smooth */
} vc1_eCondOver;

/**
 * Description:
 * Motion vector mode enumeration
 */

typedef enum
{
    vc1_MVMode1MVHalfPelBilinear = 0,   /** 1MV     0.50 pel bilinear  */
    vc1_MVMode1MVHalfPel         = 1,   /** 1MV     0.50 pel bicubic   */
    vc1_MVMode1MV                = 2,   /** 1MV     0.25 pel bicubic   */
    vc1_MVModeMixedMV            = 3,   /** MixedMV 0.25 pel bicubic   */
    vc1_MVModeIntensityCompensation     /** Variable length code escape flag */
} vc1_eMVMode;

/** Test if motion vector mode is half pixel resolution */
#define vc1_MVModeIsHalfPel(Mode) ((Mode) <= vc1_MVMode1MVHalfPel)
/** Test if motion vector mode is bicubic interpolated */
#define vc1_MVModeIsBicubic(Mode) ((Mode) >= vc1_MVMode1MVHalfPel)

/**
 * Description:
 * Intensity compensation information structure.
 */

typedef struct
{
    FLAG    IntensityCompFlag;      /** Intensity Compensation Enable */
    UBYTE8  LuminanceScale;         /** Intensity Compensation Scale */
    UBYTE8  LuminanceShift;         /** Intensity Compensation Shift */
} vc1_sIntensityComp;

/**
 * Description:
 * B Fraction numerator and denominator structure.
 */

typedef struct
{
    UBYTE8  Numerator;      /** BFraction numerator */
    UBYTE8  Denominator;    /** BFraction denominator */
    UBYTE8  ScaleFactor;    /** Approximated Numerator*256/Denominator */
} vc1_sBFraction;

/**
 * Description:
 * Hypothetical reference decoder information.
 * 
 * Remarks:
 * Note that the decoder can always round up the Rate, Buffer size
 * and Fullness parameters.
 * Note that 0 <= FullFraction/FullDenominator < 1 bit.
 *
 */

typedef struct
{
    HRDVALUE    Rate;               /** Maximum bit rate in bits per second */
    HRDVALUE    Buffer;             /** Buffer size in bits */
    HRDVALUE    Fullness;           /** Buffer fullness in complete bits */
    UWORD32     FullFraction;       /** Numerator of fractional bit buffer fullness count */
    UWORD32     FullDenominator;    /** Denominator of fractional bit buffer fullness count */
} vc1_sLeakyBucket;

/** Maximum number of leaky buckets supported */
#define VC1_MAX_HRD_NUM_LEAKY_BUCKETS 32

/**
 * Description:
 * Structure defining the state of the hypothetical reference decoder.
 */

typedef struct
{
    UBYTE8           NumLeakyBuckets;                               /** Buckets
                                                                        (0 if none specified) */
    vc1_sLeakyBucket sLeakyBucket[VC1_MAX_HRD_NUM_LEAKY_BUCKETS];   /** Per-bucket information */
} vc1_sHrdState;



/**
 * Description: Structure holding individual Pan Scan Window information.
 */

typedef struct
{
    UWORD32     HOffset;        /** Horizontal offset in pixels*/
    UWORD32     VOffset;        /** Vertical offset in pixels */
    UHWD16      Width;          /** Width in pixels */
    UHWD16      Height;         /** Height in pixels */
} vc1_sPanScanWindow;

/** Maximum number of pan scan windows supported */
#define VC1_MAX_PAN_SCAN_WINDOWS 3

/**
 * Description:
 * Structure holding all the pan and scan window information.
 */

typedef struct
{
    FLAG                PanScanPresent;                             /** PS present */
    vc1_sPanScanWindow  sPanScanWindow[VC1_MAX_PAN_SCAN_WINDOWS];   /** Per-window information */
} vc1_sPanScanParams;


/**
 * Description:
 * Sequence and Layer parameters.
 *
 */

typedef struct
{
    vc1_eProfile            eProfile;                   /** See standard */
    UHWD16                  MaxCodedWidth;              /** Maximum coded width  */
    UHWD16                  MaxCodedHeight;             /** Maximum coded height */
    UHWD16                  CodedWidth;                 /** Coded width or 0 if not specified */
    UHWD16                  CodedHeight;                /** Coded height or 0 if not specified */
    UHWD16                  DisplayWidth;               /** Display width or 0 if not specified */
    UHWD16                  DisplayHeight;              /** Display height or 0 if not specified */
    UHWD16                  AspectWidth;                /** Aspect width or 0 if not specified */
    UHWD16                  AspectHeight;               /** Aspect height or 0 if not specified */
    vc1_eLevel              eLevel;                     /** See standard */
    FLAG                    Interlace;                  /** See standard */
    UWORD32                 FrameRateNumerator;         /** 0 if not specified */
    UHWD16                  FrameRateDenominator;       /** 1000, 1001, 
                                                            or 32 (0 if not specified) */
    FLAG                    ColorFormatIndicatorFlag;   /** See standard */
    vc1_eChromaFormat       eChromaFormat;              /** See standard */
    vc1_eColorPrimaries     eColorPrimaries;            /** See standard */
    vc1_eTransChar          eTransChar;                 /** See standard */
    vc1_eMatrixCoefficients eMatrixCoefficients;        /** See standard */
    vc1_sHrdState           sHrdInitialState;           /** Initial state of HRD */
    FLAG                    LoopFilter;                 /** See standard */
    FLAG                    MultiResCoding;             /** See standard */
    FLAG                    FastUVMC;                   /** See standard */
    FLAG                    ExtendedMV;                 /** See standard */
    FLAG                    ExtendedDMV;                /** See standard */
    UBYTE8                  DQuant;                     /** See standard */
    FLAG                    VSTransform;                /** See standard */
    FLAG                    OverlappedTransformFlag;    /** See standard */
    FLAG                    SyncmarkerFlag;             /** See standard */
    FLAG                    RangeRedFlag;               /** See standard */
    UBYTE8                  MaxBFrames;                 /** See standard */
    vc1_eQuantizer          eQuantizer;                 /** See standard */
    FLAG                    PostProcessingFlag;         /** See standard */
    FLAG                    FrameCounterFlag;           /** See standard */
    FLAG                    PullDownFlag;               /** See standard */
    FLAG                    PsF;                        /** See standard */
    UBYTE8                  QFrameRateForPostProc;      /** See standard */
    UBYTE8                  QBitRateForPostProc;        /** See standard */
    FLAG                    PanScanFlag;                /** See standard */
    FLAG                    ReservedRTMFlag;            /** See standard */
    FLAG                    FrameInterpolationFlag;     /** See standard */
    UBYTE8                  RangeYScale;                /** Scale value times 8 */
    UBYTE8                  RangeUVScale;               /** Scale value times 8 */
    UBYTE8                  NumPanScanWin;              /** Number of pan scan windows */
    FLAG                    BrokenLink;                 /** Broken link flag */
    FLAG                    ClosedEntry;                /** Closed Entry flag */
    FLAG                    RefDistFlag;                /** RefDist flag */
    FLAG                    FrameUserDataFlag;          /** Frame user data present flag */
    FLAG                    EndOfSequence;              /** End of sequence marker present */
} vc1_sSequenceLayer;

/*
 * Description:
 * Start code enumeration
 */

typedef enum
{
    vc1_SCEndOfSequence     = 0x0A,
    vc1_SCSlice             = 0x0B,
    vc1_SCField             = 0x0C,
    vc1_SCFrameHeader       = 0x0D,
    vc1_SCEntryPointHeader  = 0x0E,
    vc1_SCSequenceHeader    = 0x0F,
    vc1_SCSliceUser         = 0x1B,
    vc1_SCFieldUser         = 0x1C,
    vc1_SCFrameUser         = 0x1D,
    vc1_SCEntryPointUser    = 0x1E,
    vc1_SCSequenceUser      = 0x1F
} vc1_eStartCode;



/**
 * Description: Defines the format of a single image component.
 */
typedef struct
{
    UBYTE8 *pData;          /** Pointer to raster scan data */
    int    Bpl;             /** Number of bytes per line */
} vc1_sComponent;


/**
 * Description: Defines the format of a single field.
 *
 * This structure contains information that can be local to one
 * of the two Fields in an Interlaced Field Picture.
 */

typedef struct
{
    vc1_ePictureType    ePictureType;       /** I, P, B or BI */
    vc1_eCondOver       eCondOver;          /** Conditional overlap mode */
    vc1_eQuantMode      eQuantMode;         /** Quantization mode */
    vc1_eMVMode         eMVMode;            /** Motion vector mode */
    vc1_eMVRange        eMVRange;           /** Motion vector range */
    vc1_eBlkType        eBlkType;           /** Transform type;
                                                one of Inter8x8,8x4,4x8,4x4, or Any */
    FLAG                PostProcess;        /** Post processing flag */
    FLAG                DMVExtendX;         /** Extend X DMV range */
    FLAG                DMVExtendY;         /** Extend Y DMV range */
    UBYTE8              NumRef;             /** Number of reference fields 0=one 1=two */
    UBYTE8              RefField;           /** Reference field 0=last 1=last-but-one */
    UBYTE8              MVCodingTable;      /** Range 0-7, Motion vector variable length
                                                code table */
    UBYTE8              MBModeTable;        /** Range 0-7, MB mode variable length code table */
    UBYTE8              BP2MVTable;         /** Range 0-3, Block pattern 2MV table */
    UBYTE8              BP4MVTable;         /** Range 0-3, Block pattern 4MV table */
    UBYTE8              CBPCodingTable;     /** Range 0-3, Inter CBP variable length code table */
    UBYTE8              ACCodingSetIntra;   /** Range 0-2, Coding set to use for Y Intra */
    UBYTE8              ACCodingSetInter;   /** Range 0-2, Coding set to use for Inter
                                                or for Cb,Cr Intra */
    UBYTE8              DCCodingSet;        /** Range 0-1, Coding set to use for DC */
    UHWD16              SliceRows;          /** Rows per slice (0=slices not used) */
} vc1_sField;

/**
 * Description: A Picture is the basic image unit decoded by the library.
 * A Picture can be one of the following formats:
 *    - A Progressive Frame
 *    - An Interlaced Top Field
 *    - An Interlaced Bottom Field
 *    - An Interlaced Frame
 */
typedef struct
{
    UWORD32                 Frame;          /** Frame number modulo (1<<32) */
    vc1_ePictureFormat      ePictureFormat; /** ProgressiveFrame, InterlacedField/Frame */
    vc1_sComponent          sY;             /** Y luminance values */
    vc1_sComponent          sU;             /** U/Cb chrominance values */
    vc1_sComponent          sV;             /** V/Cr chrominance values */

    vc1_sField              sField[2];      /** Field information, First then Second */

    vc1_ePictureRes         ePicRes;        /** Picture resolution index */
    FLAG                    TFF;            /** Top Field First flag */
    FLAG                    RFF;            /** Repeat First Field flag */
    FLAG                    RangeReduction; /** Range reduction used */
    FLAG                    INTERPFRM;      /** Frame interpolation hint */
    FLAG                    UVSAMP;         /** UV sampling format */
    UBYTE8                  RPTFRM;         /** Repeat Frame Count field */
    vc1_sPanScanParams      sPanScanParams; /** Pan scan window coordinates */
    vc1_ePostProcessing     ePostProcessing; /** Out of loop post processing mode */
} vc1_sPicture;

/**
 * Description:
 * Structure describing how to scale interlaced motion vectors.
 */

typedef struct {
    UHWD16              Scale;          /** Down scale factor * 256 */
    UHWD16              Scale1;         /** Up scale factor * 256 if in Zone1 */
    UHWD16              Scale2;         /** Up scale factor * 256 if not in Zone1 */
    UHWD16              ScaleZone1X;    /** Zone1 X size */
    UHWD16              ScaleZone1Y;    /** Zone1 Y size */
    UHWD16              Zone1OffsetX;   /** Zone1 X offset */
    UHWD16              Zone1OffsetY;   /** Zone1 Y offset */
    FLAG                ScaleUpOpp;     /** 0=ScaleDownForOpposite 1=ScaleUpForOpposite */
    FLAG                BottomField;    /** 0=Top Field 1=Bottom Field */
    vc1_eMVRange        eMVRange;       /** Permitted range */
    vc1_eMVMode         eMVMode;        /** Motion vector mode */
} vc1_sScaleMV;

/**
 * Description: 
 * A structure holding information required by the bilinear and bicubic interpolation functions.
 *
 */

typedef struct
{
    vc1_sComponent      sC;         /** Component from which to obtain the patch to be filtered */
    UBYTE8              SizeX;      /** X size in pixels of the resulting filtered patch */
    UBYTE8              SizeY;      /** Y size in pixels of the resulting filtered patch */
    FLAG                RndCtrl;    /** Rounding control for the frame */
} vc1_sInterpolate;


/**
 * Description:
 * Enumeration defining padding modes supported.
 */

typedef enum
{
    vc1_PadSimple,              /** Simple or main profile - pad from macroblock edge */
    vc1_PadAdvancedProgressive, /** Advanced profile progressive - pad from image edge */
    vc1_PadAdvancedInterlaced   /** Advanced profile interlaced field padding */
} vc1_ePadMode;

/**
 * Description:
 * Structure defining a rectangular area.
 */

typedef struct
{
    WORD32         XMin;            /** Minimum x coordinate */
    WORD32         XMax;            /** Maximum x coordinate (inclusive) */
    WORD32         YMin;            /** Minimum y coordinate */
    WORD32         YMax;            /** Maximum y coordinate (inclusive) */
} vc1_sRectangle;

/**
 * Description:
 * A structure to hold the rectangles to control padding and cropping.
 */

typedef struct
{
    UWORD32         Width;              /** Total width of buffer */
    UWORD32         Height;             /** Total height of buffer */
    vc1_sRectangle  sImageRectangle;    /** Image rectangle in pels relative to buffer origin */
    vc1_sRectangle  sPadFromRectangle;  /** Rectangle to pad outwards from in 
                                            pels relative to buffer origin */
    vc1_sRectangle  sPadToRectangle;    /** Rectangle limits to pad outwards to in 
                                            pels relative to buffer origin */
} vc1_sImagePosition;

/**
 * Description: A structure to hold reference key frame image information.
 */

typedef struct
{
    FLAG                Valid;              /** TRUE=Buffer contains valid data */
    FLAG                BrokenLink;         /** TRUE=no longer available due to a broken link */
    UBYTE8              Padded;             /** 1=Top field padded, 2=bottom 3=all */
    UBYTE8              RangeYScale;        /** Y scaling factor times 8 */
    UBYTE8              RangeUVScale;       /** UV scaling factor times 8 */
    UBYTE8              RefDist;            /** Number of frames between this and 
                                                the last reference frame */
    UWORD32             Frame;              /** Frame number modulo (1<<32) */
    FLAG                TFF;                /** Top Field First flag */
    FLAG                RFF;                /** Repeat First Field flag */
    FLAG                PsF;
    vc1_sPanScanParams  sPanScanParams;     /** Pan and scan window coordinates */
    FLAG                FrameInterpolationHint; /** Frame interpolation hint. Not 
                                                    used in the decoding process */
    FLAG                UVSampleMode;       /** UV plane sampling mode. Only relevant 
                                                to interlaced frames */
    UBYTE8              RepeatFrameCount;   /** Repeated frame count */
    vc1_ePostProcessing ePostProcessing;    /** Out of loop post processing mode */    
    UWORD32             CodedWidth;         /** Width in pixels of coded picture */
    UWORD32             CodedHeight;        /** Height in pixels of coded picture */
    UWORD32             MaxCodedWidth;      /** Max Width in pixels of coded picture */
    UWORD32             MaxCodedHeight;     /** Max Height in pixels of coded picture */
    vc1_ePictureFormat  ePictureFormat;     /** Format of reference picture */
    vc1_eMVRange        eMVRange[2];        /** MV Range used by reference picture (each field) */
    vc1_ePictureType    ePictureType[2];    /** Picture type for each reference field */
    vc1_ePadMode        ePadMode;           /** Padding mode to use for this picture */
    vc1_ePictureRes     ePictureRes;        /** Picture resolution scale */
    vc1_sComponent      sY;                 /** Y luminance values */
    vc1_sComponent      sU;                 /** U/Cb chrominance values */
    vc1_sComponent      sV;                 /** V/Cr chrominance values */
    UBYTE8              *pImageY;           /** Image Y top left corner */
    UBYTE8              *pImageU;           /** Image U top left corner */
    UBYTE8              *pImageV;           /** Image V top left corner */
    vc1_sImagePosition  sImagePosLuma;      /** Position of luma samples in the image buffer */
    vc1_sImagePosition  sImagePosChroma;    /** Position of chroma samples in the image buffer */
   
} vc1_sReferencePicture;

/**
 * Description:
 * A structure holding the limits for a given profile and level.
 */

typedef struct
{
    UWORD32         MBs;        /** Maximum macroblocks per second */
    UWORD32         MBf;        /** Maximum macroblocks per frame */
    UWORD32         Rmax;       /** Maximum peak transmission rate in kpbs */
    UWORD32         Bmax;       /** Maximum buffer size in multiples of 16kbits */
    vc1_eMVRange    eMVRange;   /** Motion vector range allowed */
}   vc1_sLevelLimit;

/**
 * Description:
 * Current position structure describing the macroblock being
 * processed and the slice, field, and picture it lies in.
 */
typedef struct
{
    vc1_ePictureType    ePictureType;   /** Picture type: I, P, B or BI */
    vc1_ePictureFormat  ePictureFormat; /** Picture format: Progressive, Interlace Field/frame */
    vc1_eProfile        eProfile;       /** Profile Simple/Main/Advanced */
    vc1_eMVMode         eMVMode;        /** Motion vector mode for this picture */
    vc1_eMVRange        eMVRange;       /** Motion vector range setting for this picture */
    FLAG                BottomField;    /** 0=Top Field, 1=Bottom Field */
    FLAG                SecondField;    /** 0=First Field, 1=Second Field */
    vc1_sMB             *pCurMB;        /** Pointer to the current macroblock */
    vc1_sMB             *pStartMB;      /** Pointer to start of macroblock circular buffer */
    vc1_sMotionHist     *pMVHist;       /** Current position in motion vector history buffer */
    UWORD32             SizeMB;         /** Circular buffer size in macroblocks */
    UHWD16              X;              /** X macroblock offset in current slice */
    UHWD16              Y;              /** Y macroblock offset in current slice */
    UHWD16              SliceY;         /** Y macroblock offset of slice in picture */
    UHWD16              WidthMB;        /** Width in macroblocks of coded picture */
    UHWD16              HeightMB;       /** Height in macroblocks of coded picture */
    UWORD32             CodedWidth;     /** Width in pixels of coded picture */
    UWORD32             CodedHeight;    /** Height in pixels of coded picture */
    UWORD32             MaxCodedWidth;  /** Max Width in pixels of coded picture */
    UWORD32             MaxCodedHeight; /** Max Height in pixels of coded picture */
    UBYTE8              PQuant;         /** Picture quantizer */
    UBYTE8              BFraction;      /** BFRACTION syntax element */
    UBYTE8              NumRef;         /** Number of reference fields-1 */
    UBYTE8              RefField;       /** Reference field when NumRef==0 */
    UBYTE8              IntraBias;      /** Bias to add to intra blocks post transform */
    UBYTE8              RangeYScale;    /** Y scaling factor times 8 */
    UBYTE8              RangeUVScale;   /** UV scaling factor times 8 */
    FLAG                FastUVMC;       /** Fast U,V motion compensation flag */
    vc1_ePictureRes     ePictureRes;    /** Picture resolution scale mode */
    vc1_sReferencePicture   *pReferenceOld;  /** Pointer to old I/P reference picture */
    vc1_sReferencePicture   *pReferenceNew;  /** Pointer to new/current I/P */
    vc1_sReferencePicture   *pReferenceB;    /** Reconstructed B picture */
    vc1_sReferencePicture   *pReferenceNoIC; /** Backup copy of reference before intensity 
                                                 compensation applied */
    vc1_sScaleMV        pScaleMV[2];        /** MV scaling (forward, backward) */
    HWD16               pSmooth[6][64]; /** Overlap smoothing macroblock history (left) */
} vc1_sPosition;

/**** BITSTREAM HANDLING INFORMATION ****/

/*
 * Description:
 * Defines for number of bits in each element of the sequence layer
 *  From section 5 of standard
 */
#define VC1_BITS_PROFILE            2
#define VC1_BITS_RES_SM             2
#define VC1_BITS_FRMRTQ_POSTPROC    3
#define VC1_BITS_BITRTQ_POSTPROC    5

#define VC1_BITS_PIC_SIZE_FLAG      1
#define VC1_BITS_PIC_HORIZ_SIZE     12
#define VC1_BITS_PIC_VERT_SIZE      12
#define VC1_BITS_DISP_HORIZ_SIZE    14
#define VC1_BITS_DISP_VERT_SIZE     14

#define VC1_BITS_ASPECT_RATIO_FLAG  1
#define VC1_BITS_ASPECT_RATIO       4
#define VC1_BITS_ASPECT_WIDTH       8
#define VC1_BITS_ASPECT_HEIGHT      8

#define VC1_BITS_LEVEL              3

#define VC1_BITS_INTERLACE          1
#define VC1_BITS_FRAMERATEFLAG      1
#define VC1_BITS_FRAMERATEIND       1
#define VC1_BITS_FRAMERATENR        8
#define VC1_BITS_FRAMERATEDR        4
#define VC1_BITS_FRAMERATEEXP       16

#define VC1_BITS_COLOR_FORMAT_FLAG  1
#define VC1_BITS_CHROMA_FORMAT      2
#define VC1_BITS_COLOR_PRIM         8
#define VC1_BITS_TRANSFER_CHAR      8
#define VC1_BITS_MATRIX_COEF        8

#define VC1_BITS_HRD_PARAM_FLAG           1
#define VC1_BITS_HRD_NUM_LEAKY_BUCKETS    5
#define VC1_BITS_BIT_RATE_EXPONENT        4
#define VC1_BITS_BUFFER_SIZE_EXPONENT     4
#define VC1_BITS_HRD_RATE           16
#define VC1_BITS_HRD_BUFFER         16
#define VC1_BITS_HRD_FULLNESS       8

#define VC1_BITS_LOOPFILTER         1
#define VC1_BITS_RES_X8             1
#define VC1_BITS_MULTIRES           1
#define VC1_BITS_RES_FASTTX         1

#define VC1_BITS_FASTUVMC           1
#define VC1_BITS_EXTENDED_MV        1
#define VC1_BITS_EXTENDED_DMV       1
#define VC1_BITS_DQUANT             2
#define VC1_BITS_VSTRANSFORM        1
#define VC1_BITS_RES_TRANSTAB       1
#define VC1_BITS_OVERLAP            1
#define VC1_BITS_SYNCMARKER         1
#define VC1_BITS_RANGE_RED          1
#define VC1_BITS_MAXBFRAMES         3
#define VC1_BITS_QUANTIZER          2

#define VC1_BITS_POSTPROCFLAG       1
#define VC1_BITS_FRSKIPFLAG         1
#define VC1_BITS_TFCNTRFLAG         1
#define VC1_BITS_PULLDOWN           1
#define VC1_BITS_FINTERPFLAG        1
#define VC1_BITS_PANSCANFLAG        1
#define VC1_BITS_RES_RTM_FLAG       1
#define VC1_BITS_PSF_FLAG           1
#define VC1_BITS_FS_FLAG            1

#define VC1_BITS_MAX_CODED_WIDTH    12
#define VC1_BITS_MAX_CODED_HEIGHT   12
#define VC1_BITS_RESERVED1          1
#define VC1_BITS_RESERVED2          1
#define VC1_BITS_DISPLAY_EXT        1
#define VC1_BITS_BROKEN_LINK        1
#define VC1_BITS_CLOSED_ENTRY       1
#define VC1_BITS_PANSCAN_FLAG       1
#define VC1_BITS_REFDIST_FLAG       1
#define VC1_BITS_CODED_SIZE_FLAG    1
#define VC1_BITS_CODED_WIDTH        12
#define VC1_BITS_CODED_HEIGHT       12
#define VC1_BITS_RANGE_MAPY_FLAG    1
#define VC1_BITS_RANGE_MAPY         3
#define VC1_BITS_RANGE_MAPUV_FLAG   1
#define VC1_BITS_RANGE_MAPUV        3



/*
 * Description:
 * Defines for number of bits in each element of the picture layer
 *  From section 6 of standard
 */
#define VC1_BITS_INTERPFRM          1
#define VC1_BITS_FRMCNT             2
#define VC1_BITS_RANGE_RED_FRM      1
#define VC1_BITS_PTYPE_1            1       /* First bit of PTYPE */
#define VC1_BITS_PTYPE_2            1       /* Second bit of PTYPE, if present */
#define VC1_BITS_FPTYPE             3
#define VC1_BITS_BF                 7
#define VC1_BITS_PQINDEX            5
#define VC1_BITS_HALFQP             1
#define VC1_BITS_PQUANTIZER         1
#define VC1_BITS_RESPIC             2
#define VC1_BITS_INTERLCF           1       

#define VC1_BITS_TRANSACFRM_1       1       /* First bit of TRANSACFRM */
#define VC1_BITS_TRANSACFRM_2       1       /* Second bit of TRANSACFRM, if present */
#define VC1_BITS_TRANSACFRM2_1      1       /* First bit of TRANSACFRM2 */
#define VC1_BITS_TRANSACFRM2_2      1       /* Second bit of TRANSACFRM2, if present */
#define VC1_BITS_TRANSDCTAB         1

#define VC1_BITS_FRSKIP             1
#define VC1_BITS_TFCNTR             8
#define VC1_BITS_FCM_1              1
#define VC1_BITS_FCM_2              1
#define VC1_BITS_TFF                1
#define VC1_BITS_RFF                1
#define VC1_BITS_REPSEQHDR          1
#define VC1_BITS_UVSAMP             1
#define VC1_BITS_POSTPROC           2
#define VC1_BITS_DQUANTFRM          1
#define VC1_BITS_DQPROFILE          2
#define VC1_BITS_DQSBEDGE           2
#define VC1_BITS_DQDBEDGE           2
#define VC1_BITS_DQBILEVEL          1
#define VC1_BITS_PQDIFF             3
#define VC1_BITS_ABSPQ              5

#define VC1_BITS_MVMODEBIT          1
#define VC1_BITS_LUMSCALE           6
#define VC1_BITS_LUMSHIFT           6
#define VC1_BITS_MVTAB              2
#define VC1_BITS_CBPTAB             2
#define VC1_BITS_TTMBF              1
#define VC1_BITS_TTFRM              2
#define VC1_BITS_ABSMQ              5

#define VC1_BITS_RPTFRM             2
#define VC1_BITS_RNDCTRL            1

#define VC1_BITS_PS_PRESENT         1
#define VC1_BITS_PS_HOFFSET         18
#define VC1_BITS_PS_VOFFSET         18
#define VC1_BITS_PS_WIDTH           14
#define VC1_BITS_PS_HEIGHT          14

#define VC1_BITS_REFDIST            2
#define VC1_BITS_REFFIELD           1

#define VC1_BITS_NUMREF             1
#define VC1_BITS_MBMODETAB          3
#define VC1_BITS_CBPTAB_INTERLACE   3
#define VC1_BITS_4MVBPTAB           2
#define VC1_BITS_2MVBPTAB           2

#define VC1_BITS_4MVSWITCH          1
#define VC1_BITS_INTCOMP            1

/*
 * Defines for number of bits in each element of the slice layer
 *  From section 6.1.2 of standard
 */

#define VC1_BITS_SLICE_ADDR         9
#define VC1_BITS_PIC_HEADER_FLAG    1

/*
 * Defines for number of bits in each element of the macroblock layer
 *  From section 6.1.2 of standard
 */

#define VC1_BITS_ACPRED_MB          1
#define VC1_BITS_ABSMQ_MB           5
#define VC1_BITS_HYBRIDPRED         1
#define VC1_BITS_INTERPMVP          1
#define VC1_BITS_FIELDTX            1
#define VC1_BITS_CBPPRESENT         1
#define VC1_BITS_MVSW               1

/*
 * Defines for bitplane coding
 *
 */

#define VC1_BITS_BITPLANE_INVERT    1

/*
 * Defines for syncmarker
 *
 */

#define VC1_SYNCMARKER_FIVE_BYTE    0x0000AA
#define VC1_SYNCMARKER_ELEVEN_BYTE  0x0000AB


#ifdef cplusplus
}
#endif

#endif /* def VC1TYPES_H */

