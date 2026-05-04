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
 * rdopts.h
 *
 * Data types used by the parameter file parser.
 *
 *  
 */

#ifndef RDOPTS_H
#define RDOPTS_H

#ifdef cplusplus
extern "C" {
#endif

/*****************************************************************************
 * Types used to define the available options
 *****************************************************************************/

/*
 * Description: Values to use to set special behaviour for iterated values.
 */
typedef enum
{
    RDOPTS_RANDOM=-1,
    RDOPTS_ITERATE=-2
} RDOPTS_eIteratorFlag;

/*
 * Description: Available option types.
 */
typedef enum
{
    RDOPTS_STRING,
    RDOPTS_WORD32,
    RDOPTS_UWORD32,
    RDOPTS_UHWD16,
    RDOPTS_UBYTE8,
    RDOPTS_FLOAT,
    RDOPTS_FLAG,
    RDOPTS_ENUM,
    RDOPTS_ITERATED   /* a WORD32 value that can cycle through values */
} RDOPTS_eType;

/*
 * Description of permitted enumerated value
 */
typedef struct
{
    const char *Name;
    WORD32      Value;
} RDOPTS_sEnumeratorValue;

/*
 * WORD32 range specifier
 */

typedef struct
{
    WORD32 Min;     /* WORD32 values are checked against this range */
    WORD32 Max;     /* The range is inclusive. */
} RDOPTS_sWord32Range;

/*
 * UWORD32 range specifier
 */

typedef struct
{
    UWORD32 Min;     /* UWORD32 values are checked against this range */
    UWORD32 Max;     /* The range is inclusive. */
} RDOPTS_sUWord32Range;

/*
 * float range specifier
 */
typedef struct
{
    float Min;      /* Floating point values are checked against this range */
    float Max;      /* The range is inclusive. */
} RDOPTS_sFloatRange;

/**
 * Description:
 * Top-level option specifier record.
 */
typedef struct
{
    RDOPTS_eType eType; /** Type of value (byte, word, string, ...) */
    const char *Name;   /** Option name */
    void *Value;        /** Pointer to (untyped) value to update */
    WORD32 MaxIndex;    /** Maximum array index: 0 means scalar value */
    union               /** Validation information for each eType */
    {
        /**
         * The structure starts with a void pointer, because static initialisation
         * uses the first field to determine the acceptable values.
         * A (void *) means that as long as all other union elements are
         * pointers, they can be initialised.
         */
        const void *pDummy;

        const RDOPTS_sWord32Range     *pWRange; /** Range limits for WORD32 data */
        const RDOPTS_sUWord32Range    *pURange; /** Range limits for UWORD32 data */
        const RDOPTS_sFloatRange      *pFRange; /** Range limits for FLOAT data */
        const RDOPTS_sEnumeratorValue *pEnumeratorValue; /** Enumerable values */
    } uRangeCheck;

    /** String parameters are optionally validated by calling a function.
     * A NULL pointer means no validation will be done.
     * This would be just another member of the above union, but standard
     * C does not allow conversion of function pointers to void *.
     */
    vc1_eResult (*rValidateString)(char *);
} RDOPTS_sOptionDefinition;


/*****************************************************************************
 * Common option values
 *****************************************************************************/
/**
 * Description: Available bitstream format types
 */
typedef enum
{
    RDOPTS_RCVFORMAT,
    RDOPTS_ELEMENTARYFORMAT
} RDOPTS_eBitstreamFormatType;

/*****************************************************************************
 * Common option ranges
 *****************************************************************************/
extern RDOPTS_sWord32Range RDOPTS_PictureSizeLimits;
extern RDOPTS_sEnumeratorValue RDOPTS_LevelOptions [];
extern RDOPTS_sEnumeratorValue RDOPTS_BitstreamFormatOptions [];

/*****************************************************************************
 * The parsing functions
 *****************************************************************************/

/**
 * Description:
 *  Parse options from a parameter file as specified by an option list.
 *
 * Inputs:
 *  ToolName       - pointer to the name to accept in #IF processing
 *  OptionFileName - pointer to the parameter file to open and parse
 *  pOptionList    - pointer to the array of option definitions.
 *
 * Outputs:
 *  Variables referenced by the option list updated as required.
 */
vc1_eResult RDOPTS_ReadOptions(
    const char *ToolName,
    const char *OptionFileName,
    const RDOPTS_sOptionDefinition *pOptionList);

/**
 * Description:
 *  Free option storage memory allocated during RDOPTS_ReadOptions.
 *
 * Inputs:
 *  pOptionList - pointer to the array of option definitions.
 */
void RDOPTS_FreeOptions(const RDOPTS_sOptionDefinition *pOptionList);

/*****************************************************************************
 * Shared validation functions
 *****************************************************************************/

/**
 * Description:
 *  Verify value has a valid framerate description.
 *
 * Inputs:
 *  pValue       - pointer to the value string
 *  pNumerator   - pointer to numerator variable to set
 *  pDenominator - pointer to demoninator variable to set
 *
 * Outputs:
 *  *pNumerator   - updated if valid value
 *  *pDenominator - updated if valid value
 *
 * Returns:
 *  vc1_ResultOK on success 
 */
vc1_eResult RDOPTS_ValidateFrameRate(char *pValue,UWORD32 *pNumerator,UHWD16 *pDenominator);

/* Shared timing functions */

void RDOPTS_StartTiming(void);
void RDOPTS_EndTiming(void);

#ifdef cplusplus
}
#endif

#endif /* def RDOPTS_H */

