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
 * vc1cropmv.c
 *
 * Crop motion vectors to reference picture
 *
 *  
 */

#include "vc1types.h"
#include "vc1debug.h"
#include "vc1cropmv.h"

/*
 * Description:
 * Pull back motion vector after prelminary prediction
 *
 * Remarks:
 * Simple/Main/Advanced progressive only
 *
 * Inputs:
 * pPosition    - pointer to current position
 * pMV          - pointer to motion vector to pull back
 * Blk          - Block number for 4MV
 *
 * Outputs:
 * pMV          - pulled back to (-15,width-1)x(-15,height-1)
 */

void vc1CROPMV_PPredPullBack(
    vc1_sPosition *pPosition,
    vc1_sMV *pMV,
    int Blk
)
{
    int Min;
    int X = pMV->X;
    int Y = pMV->Y;
    int IX = pPosition->X * 64 + X;
    int IY = (pPosition->SliceY + pPosition->Y) * 64 + Y;
    int Width  = pPosition->WidthMB  * 64 - 4;
    int Height = pPosition->HeightMB * 64 - 4;
    vc1_eMBType eMBType=pPosition->pCurMB->eMBType;

    if (pPosition->ePictureFormat != vc1_ProgressiveFrame)
    {
        return;
    }

    Min = -15*4;
    if (vc1_MBTypeIs4MV(eMBType))
    {
        /* 8x8 blocks */
        Min = -7*4;
        if (Blk==vc1_BlkY1 || Blk==vc1_BlkY3)
        {
            IX += 8*4;
        }
        if (Blk==vc1_BlkY2 || Blk==vc1_BlkY3)
        {
            IY += 8*4;
        }
    }

    /* Limit quarter pel to -15*4 to (width-1)*4 */
    if (IX < Min)
    {
        X -= (IX - Min);
    }
    else if (IX > Width)
    {
        X -= (IX - Width);
    }

    /* Limit quarter pel to -15*4 to (width-1)*4 */
    if (IY < Min)
    {
        Y -= (IY - Min);
    }
    else if (IY > Height)
    {
        Y -= (IY - Height);
    }

    DEBUG2(vc1DEBUG_MV, "P PullBack: MV_X = %d, MV_Y = %d\n", X, Y);
    pMV->X = (HWD16)X;
    pMV->Y = (HWD16)Y;
}

/*
 * Description:
 * Crop the motion vector prior to using it for B motion vector prediction
 *
 * Remarks:
 * Main profile B pictures only
 *
 * Inputs:
 * pPosition    - pointer to current position
 * pMV          - pointer to motion vector to pull back
 * Blk          - block number
 */

void vc1CROPMV_BPredPullBack(vc1_sPosition *pPosition, vc1_sMV *pMV, int Blk)
{
    int X = pMV->X;
    int Y = pMV->Y;
    int IX = pPosition->X;
    int IY = pPosition->SliceY + pPosition->Y;
    int Width  = pPosition->WidthMB;
    int Height = pPosition->HeightMB;
    int Min;

    if (pPosition->eProfile == vc1_ProfileAdvanced)
    {
        IX = IX * 16 * 4 + X;
        IY = IY * 16 * 4 + Y;
        Width  = (Width  * 16 - 1)*4;
        Height = (Height * 16 - 1)*4;
        if (Blk & 1)
        {
            IX += 8*4;
        }
        if (Blk & 2)
        {
            IY += 8*4;
        }

        Min = -15*4;
    }
    else
    {
        IX = IX * 8 * 4 + X;
        IY = IY * 8 * 4 + Y;
        Width  = (Width  * 8 - 1)*4;
        Height = (Height * 8 - 1)*4;

        Min = -7*4;
    }

    /* Limit MV to -7*4 to 4*(Width-1) */
    if (IX < Min)
    {
        X -= (IX-Min);
    }
    else if (IX > Width)
    {
        X -= (IX-Width);
    }

    if (IY < Min)
    {
        Y -= (IY-Min);
    }
    else if (IY > Height)
    {
        Y -= (IY-Height);
    }

    DEBUG2(vc1DEBUG_MV, "B PullBack: MV_X = %d, MV_Y = %d\n", X, Y);
    pMV->X = (HWD16)X;
    pMV->Y = (HWD16)Y;
}

/*
 * Description:
 * Pull back luma motion vector prior to motion comp
 *
 * Remarks:
 * All levels and profiles
 *
 * Inputs:
 * pPosition    - pointer to current position
 * pMV          - pointer to motion vector to pull back
 *
 * Outputs:
 * pMV          - pulled back
 */

void vc1CROPMV_LumaPullBack(vc1_sPosition *pPosition, vc1_sMV *pMV)
{
    int X = pMV->X;
    int Y = pMV->Y;
    int IX = pPosition->X;
    int IY = pPosition->SliceY + pPosition->Y;

    if (pPosition->eProfile != vc1_ProfileAdvanced)
    {
        /* old style pull back - pull back integer part to (-16,SizeToMB) */
        int Width  = pPosition->WidthMB  * 16;
        int Height = pPosition->HeightMB * 16;
        IX = (IX * 16) + (X >> 2);
        IY = (IY * 16) + (Y >> 2);

        if (IX < -16)
        {
            X -= 4*(IX+16);
        }
        else if (IX > Width)
        {
            X -= 4*(IX-Width);
        }

        if (IY < -16)
        {
            Y -= 4*(IY+16);
        }
        else if (IY > Height)
        {
            Y -= 4*(IY-Height);
        }
    }
    else /* Advanced */
    {
        /* Pull back should give the same result as for infinite pad
         * Maintain field and fractional part of MV
         */

        /* Get width in pixels and height in field lines */
        int Width  = pPosition->CodedWidth;
        int Height = pPosition->CodedHeight >> 1;

        if (pPosition->ePictureFormat == vc1_InterlacedField)
        {
            IY = IY << 1;
            Y  = Y<<1;
        }

        IX = (IX * 16) + (X >> 2);
        IY = (IY * 8)  + (Y >> 3); /* number of field lines */

        /* Need left pad of 18 and right pad of 18 */
        if (IX < -17)
        {
            X -= 4*(IX+17);
        }
        else if (IX > Width)
        {
            X -= 4*(IX-Width);
        }

        /* Need top pad of 38 and bottom pad of 38 */
        if (IY < -18)
        {
            Y -= 8*(IY+18);
        }
        else if (IY > Height+1)
        {
            Y -= 8*(IY-Height-1);
        }

        if (pPosition->ePictureFormat == vc1_InterlacedField)
        {
            Y = Y>>1;
        }
    }

    DEBUG2(vc1DEBUG_RBLK, "Luma  PullBk: MV X = %d, Y = %d\n", X, Y);
    pMV->X = (HWD16)X;
    pMV->Y = (HWD16)Y;
}

/*
 * Description:
 * Pull back chroma motion vector prior to motion comp
 *
 * Remarks:
 * All levels and profiles
 *
 * Inputs:
 * pPosition    - pointer to current position
 * pMV          - pointer to motion vector to pull back
 *
 * Outputs:
 * pMV          - pulled back
 */

void vc1CROPMV_ChromaPullBack(vc1_sPosition *pPosition, vc1_sMV *pMV)
{
    int X = pMV->X;
    int Y = pMV->Y;
    int IX = pPosition->X;
    int IY = pPosition->SliceY + pPosition->Y;

    if (pPosition->eProfile != vc1_ProfileAdvanced)
    {
        /* old style pull back - pull back integer part to (-8,SizeToMB) */
        int Width  = pPosition->WidthMB  * 8;
        int Height = pPosition->HeightMB * 8;
        IX = (IX * 8) + (X >> 2);
        IY = (IY * 8) + (Y >> 2);

        if (IX < -8)
        {
            X -= 4*(IX+8);
        }
        else if (IX > Width)
        {
            X -= 4*(IX-Width);
        }

        if (IY < -8)
        {
            Y -= 4*(IY+8);
        }
        else if (IY > Height)
        {
            Y -= 4*(IY-Height);
        }
    }
    else /* Advanced */
    {
        /* Pull back should give the same result as for infinite pad
         * Maintain field and fractional part of MV
         */

        /* Get width in pixels and height in field lines */
        int Width  = pPosition->CodedWidth  >> 1;
        int Height = pPosition->CodedHeight >> 2;
        int iMinY = -8;
        int iMaxY = Height;

        if (pPosition->ePictureFormat == vc1_InterlacedField)
        {
            /* 
             * Pullback one less line for field pictures in case reference frame was progressive coded. 
             * Otherwise if we are referencing the bottom field then we could end up pulling back too far 
             */
            iMinY--;
            iMaxY++;
            IY = IY << 1;
            Y  = Y<<1;
        }

        IX = (IX * 8) + (X >> 2);
        IY = (IY * 4) + (Y >> 3); /* number of field lines */

        /* Need left pad of 8 and right pad of 7 */
        if (IX < -8)
        {
            X -= 4*(IX+8);
        }
        else if (IX > Width)
        {
            X -= 4*(IX-Width);
        }

        /* Need top pad of 16 and bottom pad of 14 */
        if (IY < iMinY)
        {
            Y -= 8*(IY-iMinY);
        }
        else if (IY > iMaxY)
        {
            Y -= 8*(IY-iMaxY);
        }

        if (pPosition->ePictureFormat == vc1_InterlacedField)
        {
            Y = Y>>1;
        }
    }    

    DEBUG2(vc1DEBUG_RBLK, "Chroma   PullBk: MV X = %d, Y = %d\n", X, Y);
    pMV->X = (HWD16)X;
    pMV->Y = (HWD16)Y;
}
