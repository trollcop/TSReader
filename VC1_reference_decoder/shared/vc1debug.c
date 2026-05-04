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
 * vc1debug.c
 *
 * VC1 Debug module
 *
 *  
 */

#include "vc1types.h"
#include "vc1debug.h"

#include <stdio.h>  /* vprintf */
#include <stdarg.h> /* va_start, va_end */
#include <stdlib.h> /* exit */
#include <string.h>
#include <windows.h>

/* Displays a fatal error message and continues
 *
 * Inputs:
 *      pFormat     - Pointer to a printf format string
 *      ...         - Further printf parameters
 */
    
void vc1DEBUG_Fatal(char *pFormat, ... )
{    
    va_list ap ;
	char szOutput[1024];
	char szTemp[1024];
    
//rh     fprintf(stdout, "FATAL ERROR:\n");

    /* Print error to FATAL ERROR */
    sprintf(szOutput, "VC1: FATAL ERROR: ");
    va_start( ap, pFormat ) ;
    vsprintf(szTemp, pFormat, ap ) ;
    va_end( ap ) ;
	strcat(szOutput, szTemp);
	OutputDebugString(szOutput);
}

/* Displays a warning
 *
 * Inputs:
 *      pFormat     - Pointer to a printf format string
 *      ...         - Further printf parameters
 */    
    
void vc1DEBUG_Warn(char *pFormat, ... )
{    
    va_list  ap ;
    
    fprintf(stderr, "WARNING: ");
    va_start( ap, pFormat ) ;
    vfprintf(stderr, pFormat, ap ) ;
    va_end( ap ) ;
}

#ifndef NDEBUG

/* Tables of strings for more verbose output */

const char *vc1DEBUG_PictureFormat[3]=
{
    "ProgressiveFrame",
    "Interlaced-Frame",
    "Interlaced-Field"
};

const char *vc1DEBUG_PictureType[5]=
{
    "I", "P", "B", "BI", "Skipped"
};

const char *vc1DEBUG_FieldPictureType[8]=
{
    "I I", "I P", "P I", "P P",
    "B B", "B BI", "BI B", "BI BI"
};

const char *vc1DEBUG_BitPlaneMode[7] =
{
    "Norm-2", "Norm-6", "Rowskip", "Colskip", "Diff-2", "Diff-6", "Raw"
};

const char *vc1DEBUG_Profile[4] = 
{
    "Simple", "Main", "Reserved", "Advanced"
};

const char *vc1DEBUG_MBType[16] =
{
    "Intra",   "1MVDirect", "2MVDirect", "4MVDirect",
    "Illegal", "1MVFwd",    "2MVFwd",    "4MVFwd",
    "Illegal", "1MVBack",   "2MVBack",   "4MVBack",
    "Illegal", "1MVInterp", "2MVInterp", "4MVInterp"
};

const char *vc1DEBUG_BlkType[8] =
{
    "Inter8x8", "Inter8x4", "Inter4x8", "Inter4x4", "InterAny",
    "Intra", "IntraTop", "IntraLeft",
};

const char *vc1DEBUG_SmallBlkType[8] =
{
    "8x8", "8x4", "4x8", "4x4", "Any",
    "Int", "Top", "Lef",
};

const char *vc1DEBUG_BlkNum[6] =
{
    "Y0", "Y1", "Y2", "Y3", "Cb", "Cr"
};

const char *vc1DEBUG_StartCode[0x20] =
{
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "End Of Sequence",
    "Slice",
    "Field",
    "Frame",
    "Entry Point Header",
    "Sequence Header",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Slice User Data",
    "Field User Data",
    "Frame User Data",
    "Entry Point User Data",
    "Sequence User Data"
};

const char *vc1DEBUG_MVRange[4] = 
{
    "64 x 32", "128 x 64", "512 x 128", "1024 x 256"
};

const char *vc1DEBUG_MVMode[5] =
{
    "1MVHalfPelBilinear",
    "1MVHalfPel",
    "1MV",
    "MixedMV",
    "Intensity"
};

const char *vc1DEBUG_TransType[8] =
{
    "8x8", "8x4 Bottom", "8x4 Top", "8x4 Both",
    "4x8 Right", "4x8 Left", "4x8 Both", "4x4"
};

const char *vc1DEBUG_BFraction[23] =
{
    "1/2", "1/3", "2/3", "1/4", "3/4", "1/5", "2/5",
    "3/5", "4/5", "1/6", "5/6", "1/7", "2/7", "3/7",
    "4/7", "5/7", "6/7", "1/8", "3/8", "5/8", "7/8",
    "Invalid", "BI"
};

const char *vc1DEBUG_SBPType[16] =
{
    "8x8", "8x4Bottom", "8x4Top", "8x4Both",
    "4x8Right", "4x8Left", "4x8Both", "4x4",
    "MB8x8", "MB8x4Bottom", "MB8x4Top", "MB8x4Both",
    "MB4x8Right", "MB4x8Left", "MB4x8Both", "MB4x4",
};

const char *vc1DEBUG_QuantMode[12] =
{
    "Default", "AllEdges",
    "LeftTop", "TopRight", "RightBottom", "BottomLeft",
    "Left", "Top", "Right", "Bottom",
    "MBDual", "MBAny"
};

const char *vc1DEBUG_PictureRes[4] =
{
    "1x1", "2x1", "1x2", "2x2"
};

const char *vc1DEBUG_MBModeIntField[8] =
{
    "Intra-NotCoded", "Intra-Coded",
    "1MV-Skipped",    "1MV-NotCoded",
    "1MV-NoMV",       "1MV-Coded",
    "4MV-NotCoded",   "4MV-Coded"
};

const char *vc1DEBUG_MBModeIntFrame[15] =
{
    "1MV-FrameTX", "1MV-FieldTX", "1MV-NotCoded",
    "0MV-FrameTX", "0MV-FieldTX",
    "2MVF-FrameTX", "2MVF-FieldTX", "2MVF-NotCoded",
    "Intra",
    "4MV-FrameTX", "4MV-FieldTX", "4MV-NotCoded",
    "4MVF-FrameTX", "4MVF-FieldTX", "4MVF-NotCoded"
};

const char *vc1DEBUG_DeBlkType[10] =
{
    "Progressive I",
    "Main P",
    "Main PI",
    "Advanced P",
    "Frame",
    "Frame First",
    "Frame FieldTX",
    "FieldB",
    "Field1",
    "Field2"
};

/* Table of zone names, one for each bit of the zone mask */

static const char *pZoneName[32] =
{
    DEBUGZONENAMEb0,
    DEBUGZONENAMEb1,
    DEBUGZONENAMEb2,
    DEBUGZONENAMEb3,
    DEBUGZONENAMEb4,
    DEBUGZONENAMEb5,
    DEBUGZONENAMEb6,
    DEBUGZONENAMEb7,
    DEBUGZONENAMEb8,
    DEBUGZONENAMEb9,
    DEBUGZONENAMEb10,
    DEBUGZONENAMEb11,
    DEBUGZONENAMEb12,
    DEBUGZONENAMEb13,
    DEBUGZONENAMEb14,
    DEBUGZONENAMEb15,
    DEBUGZONENAMEb16,
    DEBUGZONENAMEb17,
    DEBUGZONENAMEb18,
    DEBUGZONENAMEb19,
    DEBUGZONENAMEb20,
    DEBUGZONENAMEb21,
    DEBUGZONENAMEb22,
    DEBUGZONENAMEb23,
    DEBUGZONENAMEb24,
    DEBUGZONENAMEb25,
    DEBUGZONENAMEb26,
    DEBUGZONENAMEb27,
    DEBUGZONENAMEb28,
    DEBUGZONENAMEb29,
    DEBUGZONENAMEb30,
    DEBUGZONENAMEb31
};

/* Debug is off by default */

WORD32 vc1DEBUG_Zones = 0;

/*
 * Description:
 * Displays a debug message if (vc1DEBUG_Zones & zone) is non zero
 *
 * Inputs:
 * Zone     - Area the debug message applies to (declared as an int
 *            so that logical expressions can be used to combine
 *            zones without giving a compiler warning)
 * pFormat  - Pointer to a printf format string
 * ...      - Futher printf parameters
 */

void vc1DEBUG_Debug(int Zone, char *pFormat, ... )
{
    UWORD32 ActiveZone = vc1DEBUG_Zones & Zone;

    if (ActiveZone != 0)
    {
        va_list  ap ;
        int Bit=0;

        while (ActiveZone != 0)
        {
            if (ActiveZone & 1)
            {
                printf("%s: ", pZoneName[Bit]);
            }
            Bit += 1;
            ActiveZone = ActiveZone >> 1;
        }
    
        va_start( ap, pFormat ) ;
        vprintf( pFormat, ap ) ;
        va_end( ap ) ;
        fflush(stdout);
    }
}

/* Displays a fatal error message and terminates
 *
 * Inputs:
 *      pFormat     - Pointer to a printf format string
 *      ...         - Further printf parameters
 */
    
void vc1DEBUG_Fatal_Exit(char *pFormat, ... )
{    
    va_list  ap ;
    
    fprintf(stderr, "FATAL ERROR: ");
    va_start( ap, pFormat ) ;
    vfprintf(stderr, pFormat, ap ) ;
    va_end( ap ) ;
    exit(1);
}

/* Display a rectange of 16-bit data
 *
 * Inputs:
 *      pData  - Pointer to data to display
 *      Width  - Rectangle width
 *      Height - Rectangle height
 *      Epl    - Entries per line in pData array
 */

void vc1DEBUG_PrintRectangle16(const HWD16 *pData, int Width, int Height, int Epl)
{
    int i,j;

    for (i=0; i<Height; i++)
    {
        for (j=0; j<Width; j++)
        {
            printf(" %04x", pData[j] & 0xFFFF);
        }
        pData += Epl;
        printf("\n");
    }
}

/*
 * Description:
 * Display a number of 16-bit 8x8 blocks
 *
 * Inputs:
 * pData    - Pointer to blocks to display
 * Width    - Width in blocks to display
 * N        - Number of blocks
 */

void vc1DEBUG_PrintBlocks16(const HWD16 pData[][64], int Width, int N)
{
    int i,j,Blk,BlkX;

    for (Blk=0; Blk<N; Blk += Width)
    {
        if (Width > (N-Blk))
        {
            Width = N-Blk;
        }
        for (i=0; i<8; i++)
        {
            for (BlkX = 0; BlkX<Width; BlkX++)
            {
                if (BlkX)
                {
                    printf(" | ");
                }
                for (j=0; j<8; j++)
                {
                    if (j)
                    {
                        printf(" ");
                    }
                    printf("%03x", pData[Blk+BlkX][8*i + j] & 0xFFF);
                }
            }
            printf("\n");
        }
        printf("--\n");
    }
}


/* Display a rectange of 8-bit data
 *
 * Inputs:
 *      pData  - Pointer to data to display
 *      Width  - Rectangle width
 *      Height - Rectangle height
 *      Epl    - Entries per line in pData array
 */

void vc1DEBUG_PrintRectangle8(const UBYTE8 *pData, int Width, int Height, int Epl)
{
    int i,j;

    for (i=0; i<Height; i++)
    {
        for (j=0; j<Width; j++)
        {
            printf(" %02x", pData[j] & 0xFF);
        }
        pData += Epl;
        printf("\n");
    }
}

static void vc1DEBUG_WriteComponent(
    FILE *fp,
    UBYTE8 *pData,
    int Bpl,
    int Width,
    int Height,
    int CodedWidth,
    int CodedHeight
)
{
    int i,j;

    for (i=0; i<Height; i++)
    {
        for (j=0; j<Width; j++)
        {
            int c=0;

            if (i<CodedHeight && j<CodedWidth)
            {
                c = pData[j];
            }
            fputc(c, fp);
        }
        pData += Bpl;
    }
}

/* 
 * Description:
 * Write a reference picture to a debug file
 *
 * Inputs:
 * pRef     - pointer to the reference picture
 * pName    - pointer to the file name to use
 * Append   - 0 = create a new empty file
 *            1 = append to existing file
 */

void vc1DEBUG_LogReferencePicture(vc1_sReferencePicture *pRef, char *pName, int Append)
{
    FILE *fp;
    int CodedWidth, CodedHeight, MaxCodedWidth, MaxCodedHeight;
    char pFileName[256];

    if (Append && pRef->Valid == 0)
    {
        return;
    }

    CodedWidth     = pRef->CodedWidth;
    CodedHeight    = pRef->CodedHeight;
    MaxCodedWidth  = pRef->MaxCodedWidth;
    MaxCodedHeight = pRef->MaxCodedHeight;

    sprintf(pFileName, "%s_%dx%d.yuv", pName, MaxCodedWidth, MaxCodedHeight);

    if (Append)
    {
        DEBUG1(vc1DEBUG_REFPICT, "Saving reference picture to %s\n", pFileName);
        fp = fopen(pFileName, "ab");
    }
    else
    {
        DEBUG1(vc1DEBUG_REFPICT, "New reference picture file: %s\n", pFileName);
        fp = fopen(pFileName, "wb");
    }

    ASSERT(fp!=NULL);

    if (Append)
    {
        vc1DEBUG_WriteComponent(fp, pRef->pImageY, pRef->sY.Bpl,
            MaxCodedWidth, MaxCodedHeight, CodedWidth, CodedHeight);
        vc1DEBUG_WriteComponent(fp, pRef->pImageU, pRef->sU.Bpl,
            MaxCodedWidth/2, MaxCodedHeight/2, CodedWidth/2, CodedHeight/2);
        vc1DEBUG_WriteComponent(fp, pRef->pImageV, pRef->sV.Bpl,
            MaxCodedWidth/2, MaxCodedHeight/2, CodedWidth/2, CodedHeight/2);
    }

    fclose(fp);
}

/*
 * Description:
 * Display the information in the current macroblock
 */

void vc1DEBUG_PrintMB(vc1_sPosition *pPos)
{
    vc1_sMB *pMB = pPos->pCurMB;
    vc1_eMBType eMBType = pMB->eMBType;
    int Blk;

    printf("MB Y=%2d X=%2d Q=%2d %-10s", pPos->Y, pPos->X, pMB->sQuant.Quant,
        vc1DEBUG_MBType[eMBType & 15]);

    for (Blk=0; Blk<VC1_BLOCKS_PER_MB; Blk++)
    {
        vc1_sBlk *pBlk = &pMB->sBlk[Blk];

        printf(" %s", vc1DEBUG_SmallBlkType[pBlk->eBlkType]);
    }

    if (vc1_MBTypeIsFieldTX(eMBType))
    {
        printf(" FTX");
    }
    
    if (vc1_MBTypeIsFieldMV(eMBType))
    {
        printf(" FMV");
    }
    
    if (vc1_MBTypeIsSwitchMV(eMBType))
    {
        printf(" SMV");
    }    

    if (pMB->OverlapFilter)
    {
        printf(" OVF");
    }

    printf("\n");
}

/*
 * Description:
 * Display information on the current picture
 */

void vc1DEBUG_PrintPict(vc1_sPicture *pPict)
{
    printf("[%4d] Format = %s  Picture = %s",
           pPict->Frame,
           vc1DEBUG_PictureFormat[pPict->ePictureFormat],
           vc1DEBUG_PictureType[pPict->sField[0].ePictureType]);

    if (pPict->ePictureFormat == vc1_InterlacedField)
    {
        printf("/%s TFF=%d",
               vc1DEBUG_PictureType[pPict->sField[1].ePictureType],
               pPict->TFF);
    }

    printf("\n");
}

#endif
