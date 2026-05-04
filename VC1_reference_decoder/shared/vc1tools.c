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
 * vc1tools.c:
 * Utility functions
 *
 */

#include <string.h>     /* memset */

#include "vc1types.h"
#include "vc1debug.h"
#include "vc1tools.h"

/*
 * Description:
 * Take the median value of three inputs
 *
 * Inputs:
 *      a   - first value
 *      b   - second value
 *      c   - third value
 *
 * Returns:
 *      Median of a, b and c
 */

int vc1TOOLS_Median3(int a, int b, int c)
{
    if (a > b)
    {
        if (b > c)
        {
            return b;   /* a > b > c */
        }
        if (a > c)
        {
            return c;   /* a > c >= b */
        }
        return a;   /* c >= a > b */
    }
    if (a > c)
    {
        return a;   /* b >= a > c */
    }
    if (b > c)
    {
        return c;   /* b > c >= a */
    }
    return b;       /* c >= b >= a */
}

/*
 * Description:
 * Take the median value of four inputs
 *
 * Inputs:
 *      a   - first value
 *      b   - second value
 *      c   - third value
 *      d   - fourth value
 *
 * Returns:
 *      Average of the two middle values
 */

int vc1TOOLS_Median4(int a, int b, int c, int d)
{
    int Max = a, Min = a;

    if (b > Max)
    {
        Max = b;
    }
    else if(b < Min)
    {
        Min = b;
    }

    if (c > Max)
    {
        Max = c;
    }
    else if(c < Min)
    {
        Min = c;
    }

    if (d > Max)
    {
        Max = d;
    }
    else if(d < Min)
    {
        Min = d;
    }

    return (a + b + c + d - Max - Min) / 2;
}

/*
 * Description:
 * Obtain a pointer into a picture at a block location
 *
 * Remarks:
 * 
 *
 * Inputs:
 * pC        - pointer to initialised component structure
 * pRefPic   - pointer to a reference picture structure 
 * pPosition - pointer to a position structure indicating the current MB
 * Blk       - the block number
 *
 * Outputs:
 * pC        - pointer to filled out component structure indicating
 *             block address and bytes per line of component plane
 *
 * Return Value:
 * None
 *
 */

void vc1TOOLS_GetPictureDestination(
    vc1_sComponent * pC,
    vc1_sReferencePicture * pRefPic,
    vc1_sPosition * pPosition,
    int eBlk
)
{
    UWORD32 X = pPosition->X;
    UWORD32 Y = pPosition->Y + pPosition->SliceY;
    int XMin, YMin;
    int ResultBpl = pRefPic->sY.Bpl;
    UBYTE8 *pResultData = NULL;
    int YScale = 1;

    if(vc1_InterlacedField == pPosition->ePictureFormat)
    {
        YScale = 2;
    }

    if(TRUE == vc1_BlkNumIsLuma(eBlk))
    {
        XMin = pRefPic->sImagePosLuma.sImageRectangle.XMin;
        YMin = pRefPic->sImagePosLuma.sImageRectangle.YMin;

        X = XMin + X * 16;
        Y = YMin + Y * 16 * YScale;

        if(vc1_InterlacedField == pPosition->ePictureFormat)
        {
            Y += pPosition->BottomField;
        }

        pResultData = (UBYTE8 *)(pRefPic->sY.pData + X + ResultBpl * Y);
    }
    else
    {
        XMin = pRefPic->sImagePosChroma.sImageRectangle.XMin;
        YMin = pRefPic->sImagePosChroma.sImageRectangle.YMin;

        X = XMin + X * 8;
        Y = YMin + Y * 8 * YScale;

        if(vc1_InterlacedField == pPosition->ePictureFormat)
        {
            Y += pPosition->BottomField;
        }
    }

    switch(eBlk)
    {
        case vc1_BlkY0:
        {
            break;
        }        
        case vc1_BlkY1:
        {
            pResultData += 8;                               /* add 8 columns */
            break;
        }
        case vc1_BlkY2:
        {
            /* add 8 or 16 lines if progressive or interlaced field, respectively */
            pResultData += YScale * ResultBpl * 8;
            break;
        }
        case vc1_BlkY3:
        {
            /* add 8 columns and 8 or 16 lines if progressive or interlaced field respectively */
            pResultData += YScale * ResultBpl * 8 + 8;
            break;
        }
        case vc1_BlkCb:
        {
            ResultBpl   = pRefPic->sU.Bpl;
            pResultData = (UBYTE8 *)(pRefPic->sU.pData + X + ResultBpl * Y);
            break;
        }
        case vc1_BlkCr:
        {
            ResultBpl   = pRefPic->sV.Bpl;
            pResultData = (UBYTE8 *)(pRefPic->sV.pData + X + ResultBpl * Y);
            break;
        }
        default:
        {
            WARN("vc1DECBLK_GetPictureDestination: Unknown block type %d\n", eBlk);
        }
    }
    
    pC->Bpl = ResultBpl * YScale;

    pC->pData = pResultData;

    return;
}

/*
 * Description:
 * Initialise the image position structure for a padded component
 *
 * Remarks:
 * For simplicity we make the image buffer large enough to hold
 * the worst case padding which is a padded interlaced field image.
 * This needs 32 lines of padding at the top and bottom.
 *
 * Inputs:
 * pIPos    - pointer to the image position structure or NULL
 * Width    - width of the component to store
 * Height   - height of the component to store
 * ePadMode - padding mode to support
 * Chroma   - 0=Luma, 1=Chroma component
 *
 * Outputs:
 * pIPos    - filled in if not NULL
 *
 * Returns:
 * Size of the buffer in bytes
 */

static int vc1TOOLS_InitImagePosition(
    vc1_sImagePosition *pIPos,
    int Width,
    int Height,
    vc1_ePadMode ePadMode,
    int Chroma)
{
    int HPad, VPad, BufWidth, BufHeight;
    int WidthToMB;  /* Width to macroblock edge */
    int HeightToMB; /* Height to macroblock edge */

    /* Calculate horizontal pad */
    if (Chroma)
    {
        HPad       = 10;
        WidthToMB  = (Width  + 7) &~ 7;
        HeightToMB = (Height + 7) &~ 7;
    }
    else
    {
        HPad       = 20;
        WidthToMB  = (Width  + 15) &~ 15;
        HeightToMB = (Height + 15) &~ 15;
    }

    /* Vertical pad can be up to twice as big (interlaced field) */
    VPad = 2*HPad;

    /* Buffer is pad + size + pad */
    BufWidth  = HPad + WidthToMB  + HPad;
    BufHeight = VPad + HeightToMB + VPad;

    if (pIPos)
    {
        /* Fill in buffer details */
        pIPos->Width  = BufWidth;
        pIPos->Height = BufHeight;
        pIPos->sImageRectangle.XMin = HPad;
        pIPos->sImageRectangle.YMin = VPad;
        pIPos->sImageRectangle.XMax = HPad + Width - 1;
        pIPos->sImageRectangle.YMax = VPad + Height - 1;
        pIPos->sPadFromRectangle.XMin = HPad;
        pIPos->sPadFromRectangle.YMin = VPad;
        COVERAGE("8.3.2");
        if (ePadMode == vc1_PadSimple)
        {
            pIPos->sPadFromRectangle.XMax = HPad + WidthToMB  - 1;
            pIPos->sPadFromRectangle.YMax = VPad + HeightToMB - 1;
        }
        else
        {
            COVERAGE("10.3.1");
            pIPos->sPadFromRectangle.XMax = HPad + Width - 1;
            pIPos->sPadFromRectangle.YMax = VPad + Height - 1;
        }
        pIPos->sPadToRectangle.XMin = 0;
        pIPos->sPadToRectangle.YMin = 0;
        pIPos->sPadToRectangle.XMax = BufWidth - 1;
        pIPos->sPadToRectangle.YMax = BufHeight - 1;
    }

    return ALIGN(BufWidth*BufHeight);
}

/*
 * Description:
 * Initialise a reference picture structure
 *
 * Remarks:
 * This function may be called purely to work out the number
 * of bytes required for the reference picture structure and
 * contents. In this case pRef will be NULL. If pRef is not NULL
 * then the structure is also initialized
 *
 * Inputs:
 * pRef       - Pointer to the reference picture structure or NULL
 * pSL        - Pointer to the sequence layer settings
 * Width      - Reference image CodedWidth
 * Height     - Reference image CodedHeight
 *
 * Returns:
 * Number of bytes required by the reference picture
 */

int vc1TOOLS_InitReferencePicture(
    vc1_sReferencePicture * pRef,
    const vc1_sSequenceLayer *pSL,
    int Width,
    int Height
)
{
    int Size = ALIGN(sizeof(vc1_sReferencePicture));
    int BufSize;
    vc1_ePadMode ePadMode = vc1_PadSimple;

    /* Work out the padding mode to use */
    if (pSL->eProfile == vc1_ProfileAdvanced)
    {
        ePadMode = vc1_PadAdvancedProgressive;

        if (pSL->Interlace)
        {
            ePadMode = vc1_PadAdvancedInterlaced;
        }
    }

    DEBUG4(vc1DEBUG_PIC, "pRef = 0x%08x, Width = %d, Height = %d, ePadMode = %d\n", 
        pRef, Width, Height, ePadMode);
    
    ASSERT(0 == (Width & 1));
    ASSERT(0 == (Height & 3));

    if (pRef==NULL)
    {
        Size += vc1TOOLS_InitImagePosition(NULL, Width, Height, ePadMode, 0);       /* Luma */
        Size += 2*vc1TOOLS_InitImagePosition(NULL, Width/2, Height/2, ePadMode, 1); /* Chroma */
        return Size;
    }

    pRef->ePadMode = ePadMode;
    pRef->Valid = 0;
    pRef->CodedWidth  = (UHWD16)Width;
    pRef->CodedHeight = (UHWD16)Height;
    pRef->MaxCodedWidth = pSL->MaxCodedWidth;
    pRef->MaxCodedHeight = pSL->MaxCodedHeight;

    /* Luma */
    BufSize = vc1TOOLS_InitImagePosition(&pRef->sImagePosLuma, Width, Height, ePadMode, 0);
    pRef->sY.pData = (UBYTE8 *)((char*)pRef + Size);
    pRef->sY.Bpl = pRef->sImagePosLuma.Width;
    Size += BufSize;
    pRef->pImageY = pRef->sY.pData
                + pRef->sImagePosLuma.sImageRectangle.YMin * pRef->sY.Bpl
                + pRef->sImagePosLuma.sImageRectangle.XMin;

    /* Chroma */
    BufSize = vc1TOOLS_InitImagePosition(&pRef->sImagePosChroma, Width/2, Height/2, ePadMode, 1);
    pRef->sU.pData = (UBYTE8 *)((char*)pRef + Size);
    pRef->sU.Bpl   = pRef->sImagePosChroma.Width;
    Size += BufSize;
    pRef->sV.pData = (UBYTE8 *)((char*)pRef + Size);
    pRef->sV.Bpl   = pRef->sImagePosChroma.Width;
    Size += BufSize;
    pRef->pImageU = pRef->sU.pData
                + pRef->sImagePosChroma.sImageRectangle.YMin * pRef->sU.Bpl
                + pRef->sImagePosChroma.sImageRectangle.XMin;

    pRef->pImageV = pRef->sV.pData
                + pRef->sImagePosChroma.sImageRectangle.YMin * pRef->sV.Bpl
                + pRef->sImagePosChroma.sImageRectangle.XMin;

    return Size;
}

/*
 * Description:
 * Switches the old and new reference buffers
 *
 * Inputs:
 * pPos     - pointer to position structure with:
 *            pReferenceOld = reference to discard
 *            pReferenceNew = reference to make old
 *            RangeRed      = is the picture range reduced
 */

void vc1TOOLS_NewReference(vc1_sPosition *pPos)
{
    vc1_sReferencePicture *pRef;

    /* Swap new and old reference picture buffers */
    pRef                    = pPos->pReferenceOld;
    pPos->pReferenceOld     = pPos->pReferenceNew;
    pPos->pReferenceNew     = pRef;
    pRef->Valid             = TRUE;
    pRef->BrokenLink        = FALSE;
    pRef->PsF               = FALSE;
    pRef->Padded            = 0;
    pRef->RangeYScale       = pPos->RangeYScale;
    pRef->RangeUVScale      = pPos->RangeUVScale;
    pRef->ePictureFormat    = pPos->ePictureFormat;
    pRef->ePadMode          = vc1_PadAdvancedProgressive;
    pRef->eMVRange[0]       = pPos->eMVRange;
    pRef->eMVRange[1]       = pPos->eMVRange;

    /* Clear new reference image to make debugging easier */
    memset(pRef->sY.pData, 0, pRef->sImagePosLuma.Width  *pRef->sImagePosLuma.Height);
    memset(pRef->sU.pData, 0, pRef->sImagePosChroma.Width*pRef->sImagePosChroma.Height);
    memset(pRef->sV.pData, 0, pRef->sImagePosChroma.Width*pRef->sImagePosChroma.Height);
}

/*
 * Descritpion
 * Copy image data from one reference picture to another
 *
 * Remarks:
 * Use when dealing with a skipped frame
 *
 * Inputs:
 * pOut     - Pointer to destination reference picture
 * pIn      - Pointer to source reference picture
 */

vc1_eResult vc1TOOLS_CopyReference(vc1_sReferencePicture *pOut, vc1_sReferencePicture *pIn)
{
    vc1_eResult eResult = vc1_ResultOK;
    
    DEBUG0(vc1DEBUG_PIC, "Copying reference picture\n");

    pOut->Valid = TRUE;
    pOut->BrokenLink = FALSE;
    pOut->PsF = FALSE;

    if (pIn->Valid)
    {
        pOut->ePictureFormat  = pIn->ePictureFormat;
        pOut->ePictureType[0] = pIn->ePictureType[0];
        pOut->ePictureType[1] = pIn->ePictureType[1];
        pOut->eMVRange[0]     = pIn->eMVRange[0];
        pOut->eMVRange[1]     = pIn->eMVRange[1];
        pOut->ePadMode        = vc1_PadAdvancedProgressive;
        pOut->ePictureRes     = pIn->ePictureRes;
        pOut->Padded          = 0;
        pOut->RangeYScale     = pIn->RangeYScale;
        pOut->RangeUVScale    = pIn->RangeUVScale;
        pOut->TFF             = pIn->TFF;
        pOut->Frame           = pIn->Frame;
        memcpy( pOut->sY.pData,
                pIn->sY.pData, 
                pIn->sImagePosLuma.Width*pIn->sImagePosLuma.Height);
        memcpy( pOut->sU.pData,
                pIn->sU.pData,
                pIn->sImagePosChroma.Width*pIn->sImagePosChroma.Height);
        memcpy( pOut->sV.pData,
                pIn->sV.pData,
                pIn->sImagePosChroma.Width*pIn->sImagePosChroma.Height);
    }
    else
    {
        DEBUG0(vc1DEBUG_PIC, "Reference picture invalid\n");
        eResult = vc1_ResultFatal;
    }

    return eResult;
}

/*
 * Description:
 * Padd a component ready for motion estimation or compensation
 *
 * Inputs:
 * pC           - Pointer to the component to padd
 * pIP          - Pointer to structure describing image position in pad buffer
 * Field        - 0=Pad top field, 1=Pad bottom field, 2=Pad frame
 *
 * Outputs:
 * pC           - points to padded image
 *
 */

static void vc1TOOLS_PadComponent(
    vc1_sComponent * pC,
    vc1_sImagePosition *pIP,
    int Field)
{
    vc1_sRectangle * pPadFromRect = &pIP->sPadFromRectangle;
    vc1_sRectangle * pPadToRect   = &pIP->sPadToRectangle;
    int X, Y;
    int IXMin = pPadFromRect->XMin;
    int IXMax = pPadFromRect->XMax;
    int IYMin = pPadFromRect->YMin;
    int IYMax = pPadFromRect->YMax;
    int PXMin = pPadToRect->XMin;
    int PXMax = pPadToRect->XMax;
    int PYMin = pPadToRect->YMin;
    int PYMax = pPadToRect->YMax;
    int Bpl   = pC->Bpl;
    UBYTE8 *pData = pC->pData;
    UBYTE8 *pPixels;
    UBYTE8  Pixel;

    if (Field<2)
    {
        if (Field==1)
        {
            /* Bottom field */
            pData += Bpl;
        }
         /* Interlace */
        IYMin = IYMin/2;
        IYMax = IYMax/2;
        PYMin = PYMin/2;
        PYMax = PYMax/2;
        Bpl = Bpl*2;
    }

    /* Pad left and right edges */
    
    pPixels = &pData[IYMin * Bpl];
    for (Y = IYMin; Y <= IYMax; Y++)
    {
        /* left edge */
        Pixel = pPixels[IXMin];
        for (X = PXMin; X < IXMin; X++)
        {
            pPixels[X] = Pixel;
        }

        /* right edge */
        Pixel = pPixels[IXMax];
        for (X = IXMax + 1; X <= PXMax; X++)
        {
            pPixels[X] = Pixel;
        }

        pPixels += Bpl;
    }

    /* Pad top and bottom edges */

    pPixels = &pData[PXMin];
    for (X = PXMin; X <= PXMax; X++)
    {
        /* pad top */
        Pixel = pPixels[IYMin * Bpl];
        for (Y = PYMin; Y < IYMin; Y++)
        {
            pPixels[Y*Bpl] = Pixel;
        }

        /* pad bottom */
        Pixel = pPixels[IYMax * Bpl];
        for (Y = IYMax+1; Y <= PYMax; Y++)
        {
            pPixels[Y*Bpl] = Pixel;
        }

        pPixels++;
    }
}


/*
 * Description:
 * Pad a reference picture's padd from rectangle out to its pad to rectangle
 *
 * Remarks:
 * Requires the padding rectangle to completely enclose the image rectangle
 *
 * Inputs:
 * pRef     - pointer to a reference picture structure
 *
 * Outputs:
 * pRef     - updated with padded components
 *
 * Return Value:
 * None
 *
 */

static void vc1TOOLS_PadReferencePicture(vc1_sReferencePicture * pRef)
{
    int Field = 0; /* Interlaced Pad */

    if (pRef->ePictureFormat == vc1_ProgressiveFrame &&
        pRef->ePadMode != vc1_PadAdvancedInterlaced)
    {
        /* Progressive Pad */
        Field = 2;
    }

    do
    {
        DEBUG2(vc1DEBUG_PADIC, "Padding Frame = %d, Field = %d\n", pRef->Frame, Field);
        DEBUG4(vc1DEBUG_PADIC, "pRef->ImagePosLuma (From) = %d %d %d %d\n",
            pRef->sImagePosLuma.sPadFromRectangle.XMin,
            pRef->sImagePosLuma.sPadFromRectangle.YMin,
            pRef->sImagePosLuma.sPadFromRectangle.XMax,
            pRef->sImagePosLuma.sPadFromRectangle.YMax);
        DEBUG4(vc1DEBUG_PADIC, "pRef->ImagePosLuma (To) = %d %d %d %d\n",
            pRef->sImagePosLuma.sPadToRectangle.XMin,
            pRef->sImagePosLuma.sPadToRectangle.YMin,
            pRef->sImagePosLuma.sPadToRectangle.XMax,
            pRef->sImagePosLuma.sPadToRectangle.YMax);
        vc1TOOLS_PadComponent(&pRef->sY, &pRef->sImagePosLuma, Field);
        vc1TOOLS_PadComponent(&pRef->sU, &pRef->sImagePosChroma, Field);
        vc1TOOLS_PadComponent(&pRef->sV, &pRef->sImagePosChroma, Field);

        Field++;
    } while (Field<=1);

    pRef->Padded = 1;
}

/*
 * Description:
 * Intensity compensate a component ready for motion estimation or compensation
 *
 * Inputs:
 * pC           - Pointer to the component to compensate
 * pImageRect   - Pointer to the image rectangle to compensate
 * pLUT         - Pointer to the compensation lookup table
 * Field        - 0=top field, 1=bottom field, 2=compensate frame
 *
 * Outputs:
 * pC           - points to compensated image
 *
 */

static void vc1TOOLS_ICComponent(
    vc1_sComponent * pC,
    vc1_sRectangle * pImageRect,
    UBYTE8 *pLUT,
    int Field)
{
    int X, Y;
    int Width  = pImageRect->XMax - pImageRect->XMin + 1;
    int Height = pImageRect->YMax - pImageRect->YMin + 1;
    int Bpl   = pC->Bpl;
    UBYTE8 *pData = pC->pData;

    pData += pImageRect->YMin * Bpl + pImageRect->XMin;

    if (Field<2)
    {
        if (Field==1)
        {
            /* Bottom field */
            pData += Bpl;
        }
        /* Interlace */
        Height = Height/2;
        Bpl = Bpl*2;
    }

    DEBUG3(vc1DEBUG_PADIC, "IC In : %02x %02x %02x\n", pData[0], pData[1], pData[2]);
    for (Y=0; Y<Height; Y++)
    {
        for (X=0; X<Width; X++)
        {
            pData[X] = pLUT[pData[X]];
        }
        pData += Bpl;
    }
    DEBUG3(vc1DEBUG_PADIC, "IC Out: %02x %02x %02x\n", pData[0], pData[1], pData[2]);
}

/*
 * Description:
 * Range reduce an area
 *
 * Inputs:
 * pData    - pointer to top left of area
 * Bpl      - bytes per line
 * Width    - area width
 * Height   - area height
 * Scale    - scaling in 1/8ths
 *
 * Outputs:
 * pData    - area range reduced
 */

void vc1TOOLS_RangeReduce(UBYTE8 *pData, int Bpl, int Width, int Height, int Scale)
{
    int X, Y;

    for (Y=0; Y<Height; Y++)
    {
        for (X=0; X<Width; X++)
        {
            int V = pData[X] - 128;

            V = (8*V + (Scale>>1) - 1) / Scale;
            pData[X] = (UBYTE8)(V + 128);
        }
        pData += Bpl;
    }
}

/*
 * Description:
 * Range reduce an area by 16/8 = 2 times
 *
 * Remarks:
 * This is used when remapping the reference picture from
 * non range reduced to range reduced as described in the spec
 *
 * Inputs:
 * pData    - pointer to top left of area
 * Bpl      - bytes per line
 * Width    - area width
 * Height   - area height
 *
 * Outputs:
 * pData    - area range reduced
 */

static void vc1TOOLS_RangeReduce16(UBYTE8 *pData, int Bpl, int Width, int Height)
{
    int X, Y;

    for (Y=0; Y<Height; Y++)
    {
        for (X=0; X<Width; X++)
        {
            int V = pData[X] - 128;

            V = V>>1;
            pData[X] = (UBYTE8)(V + 128);
        }
        pData += Bpl;
    }
}

/*
 * Description:
 * Range expand an area
 *
 * Inputs:
 * pData    - pointer to top left of area
 * Bpl      - bytes per line
 * Width    - area width
 * Height   - area height
 * Scale    - scaling in 1/8ths
 *
 * Outputs:
 * pData    - area range expanded
 */

static void vc1TOOLS_RangeExpand(UBYTE8 *pData, int Bpl, int Width, int Height, int Scale)
{
    int X, Y;

    for (Y=0; Y<Height; Y++)
    {
        for (X=0; X<Width; X++)
        {
            int V = pData[X] - 128;

            V = (Scale*V + 4) >> 3;
            pData[X] = CLIP(V + 128);
        }
        pData += Bpl;
    }
}

/*
 * Description:
 * Intensity compensate a reference picture ready for prediction
 *
 * Inputs:
 * pRef     - pointer to the reference picture to change range reduction
 * RangeRed - new range reduction value
 *
 * Outputs:
 * pRef     - picture range reduced
 */

static void vc1TOOLS_RangeReduceReference(
    vc1_sReferencePicture *pRef, 
    int RangeRed
)
{
    int Bpl, Width, Height;
    vc1_sRectangle *pRect;
    UBYTE8 *pData;

    DEBUG1(vc1DEBUG_PIC, "Change Range Reduction to %d\n", RangeRed);

    /* Luma */
    pRect  = &pRef->sImagePosLuma.sPadToRectangle;
    Bpl    = pRef->sY.Bpl;
    pData  = pRef->sY.pData + pRect->YMin * Bpl;
    pData += pRect->XMin;
    Width  = pRect->XMax - pRect->XMin + 1;
    Height = pRect->YMax - pRect->YMin + 1;

    if (RangeRed==16)
    {
        vc1TOOLS_RangeReduce16(pData, Bpl, Width, Height);
    }
    else
    {
        vc1TOOLS_RangeExpand(pData, Bpl, Width, Height, 16);
    }
    COVERAGE("8.3.4.12");

    /* Chroma */
    pRect  = &pRef->sImagePosChroma.sPadToRectangle;
    Bpl    = pRef->sU.Bpl;
    pData  = pRef->sU.pData + pRect->YMin * Bpl;
    pData += pRect->XMin;
    Width  = pRect->XMax - pRect->XMin + 1;
    Height = pRect->YMax - pRect->YMin + 1;

    if (RangeRed==16)
    {
        vc1TOOLS_RangeReduce16(pData, Bpl, Width, Height);
    }
    else
    {
        vc1TOOLS_RangeExpand(pData, Bpl, Width, Height, 16);
    }

    Bpl    = pRef->sV.Bpl;
    pData  = pRef->sV.pData + pRect->YMin * Bpl;
    pData += pRect->XMin;

    if (RangeRed==16)
    {
        vc1TOOLS_RangeReduce16(pData, Bpl, Width, Height);
    }
    else
    {
        vc1TOOLS_RangeExpand(pData, Bpl, Width, Height, 16);
    }

    pRef->RangeYScale = (UBYTE8)RangeRed;
    pRef->RangeUVScale = (UBYTE8)RangeRed;
}

/*
 * Description:
 * Upsample filter
 *
 * Remarks:
 * This is just an example upsample filter and is not specified in the spec.
 * The spec leaves the choice of resolution scale upsample filter as
 * implementation dependant
 *
 * Inputs:
 * pDest    - Pointer to destination line (upsampled)
 * pSrc     - Pointer to source line
 * Skip     - Byte skip between each line element
 * Length   - Number of output samples to write
 * Limit    - Number of input samples available
 */

void vc1TOOLS_ResolutionUpsample(
    UBYTE8 *pDest, 
    UBYTE8 *pSrc, 
    int Skip, 
    int Length,
    int Limit
)
{
    int X0, X1, X2, X3, X4, X;
    int Y0, Y1;
    int Round = 15;

    if (Skip>1)
    {
        /* Vertical Upsample */
        Round = 16;
    }

    /* Initialize delay line */
    X3 = *pSrc;
    pSrc += Skip;

    X4 = *pSrc;
    pSrc += Skip;

    /* Reflect missing pixels */
    X2 = X3;
    X1 = X4;

    for (X=2; X<Length+2; X++)
    {
        /* shift samples */
        X0 = X1;
        X1 = X2;
        X2 = X3;
        X3 = X4;
        if (X<Limit)
        {
            X4 = *pSrc;
            pSrc += Skip;
        }
        else
        {
            /* Reflect missing pixels */
            if (X==Limit)
            {
                X4 = X3;
            }
            else
            {
                X4 = X1;
            }
        }

        /* Upsample x 2 */
        Y0 = (     6*X1 + 28*X2 - 3*X3 + X4 + Round) >> 5;
        Y1 = (X0 - 3*X1 + 28*X2 + 6*X3      + Round) >> 5;

        /* Save results */
        *pDest = (UBYTE8)CLIP(Y0);
        pDest += Skip;
        *pDest = (UBYTE8)CLIP(Y1);
        pDest += Skip;
    }
}


/*
 * Description:
 * Intensity compensate a reference picture ready for prediction
 *
 * Inputs:
 * pRef     - pointer to the reference picture to compensate
 * pIC      - pointer to intensity compensation data structure
 * Field    - 0=TopField, 1=BottomField, 2=Progressive
 *
 * Outputs:
 * pRef     - picture compensated
 *
 */

void vc1TOOLS_IntensityCompensate(
    vc1_sReferencePicture *pRef, 
    vc1_sIntensityComp    *pIC,
    int Field
)
{
    UBYTE8 LUTY[256];       /* look up tables */
    UBYTE8 LUTUV[256];
    int LumScale  = pIC->LuminanceScale;
    int LumShift  = pIC->LuminanceShift;
    int i, j, Scale, Shift;

    DEBUG4(vc1DEBUG_PADIC, "Field=%d Flag=%d Scale=%d Shift=%d\n", Field,
        pIC->IntensityCompFlag, pIC->LuminanceScale, pIC->LuminanceShift);

    if (pIC->IntensityCompFlag==FALSE)
    {
        return;     /* nothing to do */
    }

    COVERAGE("8.3.8");
    if (LumScale == 0)
    {
        Scale = -64;
        Shift = 255 * 64 - LumShift * 2 * 64;
        if (LumShift > 31)
        {
            Shift += 128 * 64;
        }
    }
    else
    {
        Scale = LumScale + 32;
        if (LumShift > 31)
        {
            Shift = LumShift * 64 - 64 * 64;
        }
        else
        {
            Shift = LumShift * 64;
        }
    }

    for (i = 0; i < 256; i++)
    {
        j = (Scale * i + Shift + 32) >> 6;
        j = CLIP(j);
        LUTY[i] = (UBYTE8)j;

        j = (Scale * (i - 128) + 128 * 64 + 32) >> 6;
        j = CLIP(j);
        LUTUV[i] = (UBYTE8)j;
    }

    DEBUG3(vc1DEBUG_PADIC, "Field=%d Scale=%d/64 Shift=%d\n", Field, Scale, Shift>>6);

    /* Note that for Simple/Main profile we must intensity scale the image padded
     * out to the next macroblock boundary (the pad from rectangle)
     */
    vc1TOOLS_ICComponent(&pRef->sY, &pRef->sImagePosLuma.sPadFromRectangle, LUTY, Field);
    vc1TOOLS_ICComponent(&pRef->sU, &pRef->sImagePosChroma.sPadFromRectangle, LUTUV, Field);
    vc1TOOLS_ICComponent(&pRef->sV, &pRef->sImagePosChroma.sPadFromRectangle, LUTUV, Field);
    pRef->Padded = (BYTE8)(pRef->Padded & ~(1+Field));
}

/*
 * Description:
 * Intensity compensate and pad reference pictures ready for motion compensation
 *
 * Remarks:
 * Also corrects range reduction if enabled. Only call for P/B pictures.
 *
 * Inputs:
 * pPos             - pointer to details of current image a reference images
 * pIntensityComp   - pointer to the intensity compensation to apply (one per field)
 *                    For InterlacedField: 0 = Top Field IC 1 = Bottom Field IC
 */

void vc1TOOLS_ICPadReferencePicture(vc1_sPosition *pPos, vc1_sIntensityComp pIntensityComp[2])
{
    vc1_sReferencePicture *pRef1, *pRef2;
    vc1_ePictureType ePictureType = pPos->ePictureType;
    int Bottom;

    ASSERT(vc1_PictureTypeIsInter(ePictureType));

    switch (pPos->ePictureFormat)
    {
    case vc1_ProgressiveFrame:
        pRef1 = pPos->pReferenceOld;
        pRef2 = pPos->pReferenceNew;
        DEBUG2(vc1DEBUG_PADIC, "OldRangeY = %d, NewRangeY = %d\n",
            pRef1->RangeYScale, pRef2->RangeYScale);

        /* For both P and B pictures we check that the range reduction setting
         * for the old reference buffer matches the new reference buffer.
         * If not then we remap the old reference buffer so that they do.
         * For B pictures the range reduction setting is supposed to match the future
         * anchor frame (the new reference buffer).
         */
        if (pRef1->RangeYScale != pRef2->RangeYScale)
        {
            vc1TOOLS_RangeReduceReference(pRef1, pRef2->RangeYScale);
        }

        vc1TOOLS_IntensityCompensate(pRef1, &pIntensityComp[0], 2);
        vc1TOOLS_PadReferencePicture(pRef1);
        if (ePictureType == vc1_PictureTypeB)
        {
            vc1TOOLS_PadReferencePicture(pPos->pReferenceNew);
        }
        break;

    case vc1_InterlacedField:
        pRef1  = pPos->pReferenceOld;
        pRef2  = pRef1;
        Bottom = pPos->BottomField;

        DEBUG1(vc1DEBUG_PADIC, "pPos->SecondField = %d\n", pPos->SecondField);

        if (ePictureType == vc1_PictureTypeP)
        {
            if (pPos->SecondField)
            {
                pRef2 = pPos->pReferenceNew;
            }

            /* Pad both reference images regardless of NumRef */
            {
                vc1TOOLS_IntensityCompensate(pRef1, &pIntensityComp[  Bottom],   Bottom);
                vc1TOOLS_IntensityCompensate(pRef2, &pIntensityComp[1-Bottom], 1-Bottom);
            }

            if (pPos->NumRef == 0)
            {
                /* P one reference field for forward prediction */
                if (pPos->RefField == 0)
                {
                    vc1TOOLS_PadReferencePicture(pRef2);
                }
                else
                {
                    vc1TOOLS_PadReferencePicture(pRef1);
                }
            }
            else
            {
                /* P two reference fields for forward prediction */
                DEBUG0(vc1DEBUG_PADIC, "Padding Ref1\n");
                vc1TOOLS_PadReferencePicture(pRef1);
                if (pRef1!=pRef2)
                {
                    DEBUG0(vc1DEBUG_PADIC, "Padding Ref2\n");
                    vc1TOOLS_PadReferencePicture(pRef2);
                }
            }
        }
        else    /* B picture */
        {
            /* Pad forward and backward reference fields */
            vc1TOOLS_PadReferencePicture(pRef1);
            if (pPos->SecondField)
            {
                pRef2 = pPos->pReferenceB;
                vc1TOOLS_PadReferencePicture(pRef2);
            }
            vc1TOOLS_PadReferencePicture(pPos->pReferenceNew);
        }
        break;

    case vc1_InterlacedFrame:
        pRef1 = pPos->pReferenceOld;
        vc1TOOLS_IntensityCompensate(pRef1, &pIntensityComp[0], 2);

        /* Use field based padding */
        vc1TOOLS_PadReferencePicture(pRef1);
        if (ePictureType == vc1_PictureTypeB)
        {
            vc1TOOLS_PadReferencePicture(pPos->pReferenceNew);
        }
        break;
    
    }
}
