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
 * rdopts.c
 *
 * Parsing of parameter file options.
 *
 *  
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include <time.h>

#include "vc1types.h"
#include "rdopts.h"
#include "vc1debug.h"
#include "vc1gentab.h"

/*
 * Static variables.
 */

/*
 * This variable holds a pointer to a malloc'ed array, which
 * has bits set for each option defined. This allows us to:
 * - reject multiply defined options
 * - tidy up at program end, by freeing malloc'ed strings.
 */
static UWORD32 *pPresent;

/*
 * Workspace for timing functions
 */
static time_t RDOPTS_StartTime;

/*
 * Constant data tables
 */

/*
 * Common values for options to refer to.
 */

RDOPTS_sWord32Range RDOPTS_PictureSizeLimits = { 2, 8192 };

RDOPTS_sEnumeratorValue RDOPTS_LevelOptions [] =
{
    { "Low",      vc1_LevelLow },
    { "Medium",   vc1_LevelMedium },
    { "High",     vc1_LevelHigh },
    { "L0",       vc1_LevelL0 },
    { "L1",       vc1_LevelL1 },
    { "L2",       vc1_LevelL2 },
    { "L3",       vc1_LevelL3 },
    { "L4",       vc1_LevelL4 },
    { "Unknown",  vc1_LevelUnknown },
#if 0
    { "Reserved", vc1_LevelReserved },
#endif

    /* End the list with a null name pointer */
    { NULL,     0 }
};

RDOPTS_sEnumeratorValue RDOPTS_BitstreamFormatOptions [] =
{
    { "RCV",        RDOPTS_RCVFORMAT },
    { "ELEMENTARY", RDOPTS_ELEMENTARYFORMAT },

    /* End the list with a null name pointer */
    { NULL,     0 }
};

/* Allow flags to be specified as yes/no or 0/1 */
static const RDOPTS_sEnumeratorValue sFlagEnumerator [] =
{
    { "yes", 1 },
    { "no",  0 },
    { "1",   1 },
    { "0",   0 },

    { NULL, 0 }
};

/* Special values for iterated range values */
static const RDOPTS_sEnumeratorValue sIteratorOptions [] =
{
    { "random", RDOPTS_RANDOM },
    { "iterate", RDOPTS_ITERATE },

    { NULL, 0 }
};

/* Default full range limits */
static const RDOPTS_sUWord32Range sUnsigned32BitRange =
{
    0, 0xffffffff
};
static const RDOPTS_sWord32Range sSigned32BitRange =
{
    -0x7fffffff-1, 0x7fffffff
};
static const RDOPTS_sWord32Range sUnsigned16BitRange =
{
    0, 0xffff
};
static const RDOPTS_sWord32Range sUnsigned8BitRange =
{
    0, 0xff
};


/***********************************************************************
 *  Functions
 ***********************************************************************/

/*
 * Timing Functions
 */

void RDOPTS_StartTiming(void)
{
    time(&RDOPTS_StartTime);
}

void RDOPTS_EndTiming(void)
{
    time_t EndTime;
    double Seconds;

    time(&EndTime);
    Seconds = difftime(EndTime, RDOPTS_StartTime);
    fprintf(stderr, "Time Taken = %.2f Minutes\n", Seconds/60);
}


static vc1_eResult ReadWORD32(const char *pChar, WORD32 *pValue, char Terminator)
{
    int NumCharsRead;

    if (sscanf(pChar,"%d%n",pValue, &NumCharsRead) == 1 &&
        pChar[NumCharsRead] == Terminator)
    {
        return vc1_ResultOK;
    }

    /* Allow hexadecimal. Use %i as valid octal will not have made it this far */
    if (sscanf(pChar,"%i%n",pValue, &NumCharsRead) == 1 &&
        pChar[NumCharsRead] == Terminator)
    {
        return vc1_ResultOK;
    }

    return vc1_ResultInvalidParameter;
}

static vc1_eResult ReadUWORD32(const char *pChar, UWORD32 *pValue, char Terminator)
{
    int NumCharsRead;

    if (sscanf(pChar,"%u%n",pValue, &NumCharsRead) == 1 &&
        pChar[NumCharsRead] == Terminator)
    {
        return vc1_ResultOK;
    }

    /* Allow hexadecimal. Cannot use %i, so must use %x - so check for 0x */
    if (pChar[0] == '0' && tolower(pChar[1]) == 'x' &&
        sscanf(pChar,"%x%n",pValue, &NumCharsRead) == 1 &&
        pChar[NumCharsRead] == Terminator)
    {
        return vc1_ResultOK;
    }

    return vc1_ResultInvalidParameter;
}

static unsigned int UncasedMatch(const char *String1, const char *String2)
{
    int NumMatches = 0;
    while (*String1 != '\0' && *String2 != '\0' &&
           tolower(*String1) == tolower(*String2))
    {
        String1 += 1;
        String2 += 1;
        NumMatches += 1;
    }

    return NumMatches;
}

/* Scan for a matching enumeration */
static const RDOPTS_sEnumeratorValue *FindEnum(const RDOPTS_sEnumeratorValue * pEnumerator,
                                               const char *pValue)
{
    /* Scan to see if the value matches an allowed entry */
    while (pEnumerator->Name != NULL)
    {
        /* Do a case-insensitive string match on Value and Name */
        const char *pCh1, *pCh2;
        for (pCh1 = pValue, pCh2 = pEnumerator->Name;
             *pCh1 != '\0' && tolower(*pCh1) == tolower(*pCh2);
             pCh1 += 1, pCh2 += 1)
                 ;

        if (*pCh2 == '\0')
        {
            break;          /* Success: name matched */
        }
        pEnumerator += 1;
    }

    return pEnumerator;
}

/*
 * Description:
 *  Parse options from a parameter file as specified by an option list.
 *
 * Remarks:
 *  See the documentation for more details of the accepted syntax.
 *
 * Inputs:
 *  ToolName        pointer to the name to accept in #IF processing
 *  OptionFileName  pointer to the parameter file to open and parse
 *  sOptionsList    pointer to the array of option definitions.
 *
 * Outputs:
 *  Variables referenced by the option list updated as required.
 */

vc1_eResult RDOPTS_ReadOptions(
    const char *ToolName,
    const char *OptionFileName,
    const RDOPTS_sOptionDefinition *pOptionList)
{
    vc1_eResult eResult = vc1_ResultOK;
    int Line = 0;
    FILE *pOptionFile;
    int OptNum;
    int IFLine = 0;
    FLAG Skipping = FALSE;

    /*
     * Count the available options
     */
    for (OptNum = 0;
         pOptionList[OptNum].Name != NULL;
         OptNum += 1)
        ;

    /*
     * Set up the map of defined options.
     */
    pPresent = (UWORD32 *)malloc(OptNum*sizeof(WORD32));
    if (pPresent == NULL)
    {
        WARN("No room for option map\n");
        return vc1_ResultNoMemory;
    }

    /*
     * Clear, as no options yet defined
     */
    memset(pPresent, 0, OptNum*sizeof(WORD32));

    /*
     * Now we are ready to open and parse the option file
     */
    pOptionFile = fopen(OptionFileName, "r");
    if (pOptionFile == NULL)
    {
        fprintf(stderr,
                "Cannot open options file '%s' for reading\n",
                OptionFileName);
        return vc1_ResultBadFile;
    }

    while (!feof(pOptionFile))
    {
        char Buffer[16384];
        char Name[512];
        char *pChar;
        char *pValue;
        const char *pOptName;
        vc1_eResult eValid;
        int Index;

        /* Read in the next line from the configuration file */
        Line += 1;
        if (NULL == fgets(Buffer, sizeof(Buffer)-1, pOptionFile))
        {
            /* fgets always fails at the end of the file - warn if not at eof */
            if (!feof(pOptionFile))
            {
                WARN("Error reading line %d from options file '%s'\n",
                     Line, OptionFileName);
            }
            continue;
        }
        pChar = Buffer;

        /* Do 'preprocessing' */
        if (UncasedMatch(Buffer,"#if") == 3 && isspace(Buffer[3]))
        {
            /* Start of a #if section: look for tool name */
            if (IFLine != 0)
            {
                WARN("Nested #IF in options at line %d:\n  '%s\n",Line,pChar);
                eResult = vc1_ResultBadLine;
            }

            IFLine = Line;
            Skipping = TRUE; /* Default unless we find a match below */

            /*
             * Now look for the tool name in the parameters.
             * We accept TOOLNAME [ || TOOLNAME ] *
             */
            pChar = &Buffer[3];
            while (*pChar != '\0')
            {
                /* Skip space */
                while (isspace(*pChar)) pChar++;

                /* Look for match */
                if (UncasedMatch(pChar, ToolName) == strlen(ToolName) &&
                    !isalnum(pChar[strlen(ToolName)]))
                {
                    DEBUG1(vc1DEBUG_OPTIONS,"#IF at line %d matched\n", Line);
                    Skipping = FALSE;
                    break;
                }

                /* Skip over token */
                while (isalnum(*pChar)) pChar++;
                while (isspace(*pChar)) pChar++;

                /* See if more */
                if (pChar[0] != '|' || pChar[1] != '|')
                {
                    break;
                }
                pChar += 2;
            }
            continue;  /* Finished processing #IF */
        }
        if (UncasedMatch(Buffer,"#endif") == 6 && !isalnum(Buffer[6]))
        {
            if (IFLine == 0)
            {
                WARN("Unmatched #ENDIF in options at line %d:\n  '%s\n",Line,pChar);
                eResult = vc1_ResultBadLine;
            }

            IFLine = 0;
            Skipping = FALSE;
            continue;
        }

        if (Skipping)
        {
          continue;    /* Ignore this line */
        }

        /* Remove any comment section */
        for (; *pChar != '\0'; pChar++)
        {
            if (*pChar == '#' ||
                *pChar == ';')
            {
                *pChar = '\0';          /* Insert new line terminator */
                break;                  /* Comment field removed */
            }
        }

        /* Remove trailing whitespace. pChar points at the terminator. */
        while (--pChar >= Buffer &&     /* Step back while in range */
               isspace(*pChar))         /* and character is whitespace */
               ;

        pChar[1] = '\0';                /* Insert new terminator */

        /* Skip blank lines */
        if (pChar < Buffer)
        {
            continue;                   /* No characters left to process */
        }

        /* Process options */
        DEBUG2(vc1DEBUG_OPTIONS, "Processing option line %d:'%s'\n", Line, Buffer);

        /* Split into name and value parts */
        for (pChar = Name, pValue = Buffer;
             *pValue != ':' && *pValue != '\0';
             pValue += 1)
        {
            if (!isspace(*pValue))
            {
                *pChar++ = (char) tolower(*pValue);
            }
        }
        /* Is there an index specifier on the name? */
        if (pChar[-1] == ']')
        {
            while (*--pChar != '[' &&
                   pChar > Name)
                   ;

            /* max index is 31 due to WORD32 elements in pPresent */
            if (pChar == Name ||
                ReadWORD32(pChar+1, &Index, ']') != vc1_ResultOK ||
                Index < 0 || Index > 31)
            {
                WARN("Line %d of options ignored: bad index in\n '%s'\n",
                     Line, Buffer);
                eResult = vc1_ResultBadLine;
                continue;
            }

        }
        else
        {
            Index = 0;          /* No array index specified */
        }

        *pChar = '\0';          /* Terminate the name string we have constructed */

        /* Was there a valid separator? */
        if (*pValue == '\0')
        {
            WARN("Line %d of options ignored: no name:value separator in\n '%s'\n",
                 Line, Buffer);
            eResult = vc1_ResultBadLine;
            continue;
        }

        /*
         * Strip leading whitespace on the value. Pre-increment to initially
         * skip the : separator
         */
        while (*++pValue != '\0' && isspace(*pValue));

        /* Find the specified name in the option table */
        for (OptNum = 0;
             pOptionList[OptNum].Name != NULL;
             OptNum += 1)
        {
            /* See if this entry name matches */
            for (pChar = Name, pOptName = pOptionList[OptNum].Name;
                 *pChar != '\0' && *pChar == tolower(*pOptName);
                 pChar += 1, pOptName += 1)
                ;

            if (*pOptName == '\0')   /* Both strings terminated */
            {
                break;                /* Exit with pOptionEntry set */
            }
        }

        if (pOptionList[OptNum].Name == NULL)
        {
            WARN("Line %d of options ignored: no match for name '%s' in\n  '%s'\n",
                 Line, Name, Buffer);
            /*
             * Unrecognised options are just warned about, and then ignored.
             * This means that the configuration file can be reused for other tools, 
             * without each tool including the full list of all options.
             */

            continue;
        }

        DEBUG2(vc1DEBUG_OPTIONS,"Name '%s' matches '%s'\n", Name, pOptionList[OptNum].Name);

        if (pPresent[OptNum] & (1<< Index))
        {
            WARN("Line %d of options ignored: option '%s' already set\n  '%s'\n",
                 Line, Name, Buffer);
            eResult = vc1_ResultBadLine;
            continue;
        }

        /* Check that any array index specified is in range */
        if (pOptionList[OptNum].MaxIndex < Index)
        {
            WARN("Line %d of options ignored: index %d too large (max %d) in\n  '%s'\n",
                 Line, Index, pOptionList[OptNum].MaxIndex, Buffer);
            eResult = vc1_ResultBadLine;
            continue;
        }

        /* Option record found: process as appropriate */
        switch (pOptionList[OptNum].eType)
        {
        case RDOPTS_STRING:
            /* First see if we must call a validation routine */
            /* Note that the validation can update the string into a cleaned version */
            if (pOptionList[OptNum].rValidateString == NULL ||
                (eValid = pOptionList[OptNum].rValidateString(pValue))==vc1_ResultOK)
            {
                char *Storage;

                DEBUG2(vc1DEBUG_OPTIONS,
                      "Value '%s' accepted for name '%s'\n",
                      pValue,
                      pOptionList[OptNum].Name);

                /* malloc space for the value, and store the pointer in the target variable */
                Storage = (char *)malloc(strlen(pValue)+1);
                if (Storage != NULL)
                {
                    strcpy(Storage, pValue);
                    ((char **) pOptionList[OptNum].Value)[Index] = Storage;
                    pPresent[OptNum] |= (1<< Index);
                }
                else
                {
                    WARN("No room to store value '%s' for '%s'\n",
                         pValue,
                         pOptionList[OptNum].Name);
                    eResult = vc1_ResultNoMemory;
                }
            }
            else
            {
                WARN("Value '%s' rejected for '%s'\n",
                     pValue,
                     pOptionList[OptNum].Name);
                eResult = eValid;
            }
            break;

        case RDOPTS_ITERATED:
            {
                /* First look for RANDOM or ITERATED */
                const RDOPTS_sEnumeratorValue *pEnumerator = FindEnum(sIteratorOptions, pValue);

                if (pEnumerator->Name != NULL)
                {
                    ((WORD32 *) pOptionList[OptNum].Value)[Index] = pEnumerator->Value;
                    pPresent[OptNum] |= (1<< Index);
                    break;
                }

                /* Otherwise fall into the general ranged code, to be handled as WORD32 */
            }

        /* Many integer values can be handled together, with appropriate range checks */
        case RDOPTS_WORD32:
        case RDOPTS_UHWD16:
        case RDOPTS_UBYTE8:
            {
                WORD32 Value;
                const RDOPTS_sWord32Range *pRange;

                pRange = pOptionList[OptNum].uRangeCheck.pWRange;
                if (pRange == NULL)
                {
                    if (pOptionList[OptNum].eType == RDOPTS_UHWD16)
                    {
                        pRange = &sUnsigned16BitRange;
                    }
                    else if (pOptionList[OptNum].eType == RDOPTS_UBYTE8)
                    {
                        pRange = &sUnsigned8BitRange;
                    }
                    else
                    {
                        pRange = &sSigned32BitRange;
                    }
                }

                if (ReadWORD32(pValue, &Value, '\0') != vc1_ResultOK)
                {
                    WARN("Line %d of options ignored: bad integer value '%s' in\n '%s'\n",
                         Line, pValue, Buffer);
                    eResult = vc1_ResultBadLine;
                }
                else if (Value < pRange->Min || Value > pRange->Max)
                {
                    WARN("Line %d of options ignored: value %d not in %d to %d in\n '%s'\n",
                         Line,
                         Value,
                         pRange->Min,
                         pRange->Max,
                         Buffer);
                    eResult = vc1_ResultBadLine;
                }
                else
                {
                    if (pOptionList[OptNum].eType == RDOPTS_UHWD16)
                    {
                        ((UHWD16 *) pOptionList[OptNum].Value)[Index] = (UHWD16) Value;
                    }
                    else if (pOptionList[OptNum].eType == RDOPTS_UBYTE8)
                    {
                        ((UBYTE8 *) pOptionList[OptNum].Value)[Index] = (UBYTE8) Value;
                    }
                    else
                    {
                        ((WORD32 *) pOptionList[OptNum].Value)[Index] = Value;
                    }
                    pPresent[OptNum] |= (1<< Index);
                }
            }
            break;

        /* UWORD32 cannot be handled within the range of WORD32 */
        case RDOPTS_UWORD32:
            {
                UWORD32 Value;
                const RDOPTS_sUWord32Range *pRange;

                pRange = pOptionList[OptNum].uRangeCheck.pURange;
                if (pRange == NULL)
                {
                   pRange = &sUnsigned32BitRange;
                }

                if (ReadUWORD32(pValue, &Value, '\0') != vc1_ResultOK)
                {
                    WARN("Line %d of options ignored: bad unsigned integer value '%s' in\n '%s'\n",
                         Line, pValue, Buffer);
                    eResult = vc1_ResultBadLine;
                }
                else if (Value < pRange->Min || Value > pRange->Max)
                {
                    WARN("Line %d of options ignored: value %ud not in %ud to %ud in\n '%s'\n",
                         Line,
                         Value,
                         pRange->Min,
                         pRange->Max,
                         Buffer);
                    eResult = vc1_ResultBadLine;
                }
                else
                {
                    ((UWORD32 *) pOptionList[OptNum].Value)[Index] = Value;
                    pPresent[OptNum] |= (1<< Index);
                }
            }
            break;

        case RDOPTS_FLOAT:
            {
                float Value;
                int NumCharsRead;

                if (sscanf(pValue,"%f%n",&Value, &NumCharsRead) != 1 ||
                    pValue[NumCharsRead] != '\0')
                {
                    WARN("Line %d of options ignored: bad floating point value '%s' in\n '%s'\n",
                         Line, pValue, Buffer);
                    eResult = vc1_ResultBadLine;
                }
                else if (pOptionList[OptNum].uRangeCheck.pFRange != NULL &&
                         (Value < pOptionList[OptNum].uRangeCheck.pFRange->Min ||
                          Value > pOptionList[OptNum].uRangeCheck.pFRange->Max))
                {
                    WARN("Line %d of options ignored: value %f not in %f to %f in\n '%s'\n",
                         Line,
                         Value,
                         pOptionList[OptNum].uRangeCheck.pFRange->Min,
                         pOptionList[OptNum].uRangeCheck.pFRange->Max,
                         Buffer);
                    eResult = vc1_ResultBadLine;
                }
                else
                {
                    ((float *) pOptionList[OptNum].Value)[Index] = Value;
                    pPresent[OptNum] |= (1<< Index);
                }
            }
            break;

        case RDOPTS_ENUM:
        case RDOPTS_FLAG:
            {
                const RDOPTS_sEnumeratorValue *pEnumerator;
                if (pOptionList[OptNum].eType == RDOPTS_FLAG)
                {
                    pEnumerator = sFlagEnumerator;
                }
                else
                {
                    pEnumerator = pOptionList[OptNum].uRangeCheck.pEnumeratorValue;
                }

                pEnumerator = FindEnum(pEnumerator, pValue);

                if (pEnumerator->Name == NULL)
                {
                    WARN("Line %d of options ignored: value '%s' not allowed in\n '%s'\n",
                         Line,
                         pValue,
                         Buffer);
                    eResult = vc1_ResultBadLine;
                }
                else
                {
                    if (pOptionList[OptNum].eType == RDOPTS_FLAG)
                    {
                        ((FLAG *) pOptionList[OptNum].Value)[Index] = (FLAG) (pEnumerator->Value);
                    }
                    else
                    {
                        /* Enumerated types are int-sized for standard C */
                        ((int *) pOptionList[OptNum].Value)[Index] = pEnumerator->Value;
                    }
                    pPresent[OptNum] |= (1<< Index);
                }
            }
            break;

        default:
            WARN("Option '%s' has bad type (%d)\n",
                 pOptionList[OptNum].Name,
                 pOptionList[OptNum].eType);
            eResult = vc1_ResultBadType;
            break;
        }
    }

    fclose(pOptionFile);

    if (IFLine != 0)
    {
        WARN("#IF at line %d in options not closed\n",IFLine);
        eResult = vc1_ResultBadLine;
    }

    return eResult;
}


/*
 * Description:
 *  Free option values malloced during RDOPTS_ReadOptions
 *
 * Inputs:
 *  sOptionsList    pointer to the array of option definitions.
 */

void RDOPTS_FreeOptions(const RDOPTS_sOptionDefinition *pOptionList)
{
    int OptNum;

    /* Iterate over the options, freeing any string values allocated */
    for (OptNum = 0;
         pOptionList[OptNum].Name != NULL;
         OptNum ++)
    {
        if (pOptionList[OptNum].eType == RDOPTS_STRING)
        {
            /* Option is a string value, so scan for values set */
            int Index;
            for (Index = 0;
                 pPresent[OptNum] != 0;
                 Index +=1, pPresent[OptNum] = pPresent[OptNum] >> 1)
            {
                if (pPresent[OptNum] & 1)
                {
                    DEBUG2(vc1DEBUG_OPTIONS,
                          "Freeing index %d of option '%s'\n",
                          Index, pOptionList[OptNum].Name);
                    free(((char **)(pOptionList[OptNum].Value))[Index]);
                }
            }
        }
    }

    /* Free the option set map */
    free(pPresent);
}


/*
 * Description
 * Check if a frame rate numerator is one of the permitted values
 *
 * Inputs:
 * Numerator - Numerator value
 *
 * Returns:
 * TRUE if this is valid frame rate numerator
 */

static int RDOPTS_NumeratorValid(UWORD32 Numerator)
{
    int i;

    for (i=1; i<VC1_FRAMERATENR_TBL_SIZE; i++)
    {
        if (vc1GENTAB_FrameRateNumerators[i] == Numerator)
        {
            return TRUE;
        }
    }
    return FALSE;
}

/*
 * Description:
 *  Verify value has a valid framerate description
 *
 * Inputs:
 *  pValue       pointer to the value string
 *  pNumerator   pointer to numerator variable to set
 *  pDenominator pointer to demoninator variable to set
 *
 * Outputs:
 *  *pNumerator   updated if valid value
 *  *pDenominator updated if valid value
 *
 * Returns:
 *  vc1_ResultOK on success 
 */
vc1_eResult RDOPTS_ValidateFrameRate(char *pValue,UWORD32 *pNumerator,UHWD16 *pDenominator)
{
    int Numerator, Denominator, NumCharsRead;
    float FPValue;

    /* Look for a string in format a/b */
    if (sscanf(pValue,"%d/%d%n",&Numerator, &Denominator, &NumCharsRead) == 2 &&
        pValue[NumCharsRead] == '\0')
    {
        /* Denominator must be 1000, 1001, or 32 */
        if (Denominator != 1000 &&
            Denominator != 1001 &&
            Denominator != 32)
        {
            FATAL("Invalid frame rate denominator (%d != 1000, 1001 or 32)\n",
                  Denominator);
            return vc1_ResultInvalidParameter;
        }

        /* Numerator must be in the table if denominator is 1000 or 1001 */
        if (Denominator > 32 && !RDOPTS_NumeratorValid(Numerator))
        {
            FATAL("Invalid frame rate numerator (%d)\n", Numerator);
            return vc1_ResultInvalidParameter;
        }

        /* Numerator must be 1..65536 if denominator is 32 */
        if (Denominator == 32 &&
            (Numerator < 1 || Numerator > 65536))
        {
            FATAL("Invalid frame rate numerator (%d is not in the range 1 to 65536)\n",
                  Numerator);
            return vc1_ResultInvalidParameter;
        }

        /* Tests passed: set the sequence layer information */
        *pNumerator = Numerator;
        *pDenominator = (UHWD16) Denominator;
        return vc1_ResultOK;
    }

    
    /* See if there is a floating point value we can make sense of */
    if (sscanf(pValue,"%f%n",&FPValue, &NumCharsRead) == 1 ||
        pValue[NumCharsRead] == '\0')
    {
        int RoundedRate;

        /* Look for simple integer rates */
        if (RDOPTS_NumeratorValid((UWORD32)FPValue*1000))
        {
            /* Simple rate with denominator of 1000 */
            *pNumerator = ((UWORD32) FPValue)*1000;
            *pDenominator = 1000;
            DEBUG2(vc1DEBUG_OPTIONS,"Encoder frame rate = %d/%d\n",
                  *pNumerator,
                  *pDenominator);
            return vc1_ResultOK;
        }

        /* See if it is close enough to a valid /1001 value */
        RoundedRate = (int)(FPValue*1001/1000+0.5);

        /* First check that rounded version is within 1% of exact value */
        if (fabs(((float)RoundedRate*1000/1001 - FPValue)/FPValue) < 0.01 &&
            RDOPTS_NumeratorValid(RoundedRate*1000))
        {
            /* Simple rate with denominator of 1001 */
            *pNumerator = RoundedRate * 1000;
            *pDenominator = 1001;
            DEBUG2(vc1DEBUG_OPTIONS,"Encoder frame rate = %d/%d\n",
                  *pNumerator,
                  *pDenominator);
            return vc1_ResultOK;
        }

        /* Check for close match with integer/32 */
        FPValue = FPValue*32;
        RoundedRate = (int)(FPValue + 0.5);
        if (fabs((RoundedRate - FPValue)/FPValue) < 0.01 &&
            RoundedRate >= 1 &&
            RoundedRate <= 0x10000) /* 16-bit range */
        {
            /* Rate with denominator of 32 */
            *pNumerator = RoundedRate;
            *pDenominator = 32;
            DEBUG2(vc1DEBUG_OPTIONS,"Encoder frame rate = %d/%d\n",
                  *pNumerator,
                  *pDenominator);
            return vc1_ResultOK;
        }
    }

    FATAL("Frame rate not recognised in '%s'\n",pValue);
    return vc1_ResultInvalidParameter;
}
