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
 * vc1deczz.c
 *
 * vc1 Decoder zig zag module
 *
 *  
 */

#include "vc1types.h"
#include "vc1debug.h"
#include "vc1zztab.h"
#include "vc1gentab.h"

#include "vc1deczz.h"

/*
 * Description:
 * Inner zigzag function, converting input data to block using a de-zigzag table
 *
 * Remarks:
 * No remarks
 *
 * Inputs:
 * pOut         - pointer to output rectangle (8 pixels wide)
 * pIn          - pointer to input zigzagged data
 * pDeZigZag    - pointer to de-zigzag table
 * Width        - output rectangle width
 * Height       - output rectangle height
 *
 * Outputs:
 * pOut         - pointer to de-zigzagged data
 *
 * Return Value:
 * None
 *
 */

static void vc1DECZZ_DeZigZag(  HWD16 *pOut, 
                                const HWD16 *pIn,
                                const BYTE8 *pDeZigZag,
                                int Width,
                                int Height)
{
    int i, j;

    DEBUG5(vc1DEBUG_ZZ,
        "pOut = %08X, pIn = %08X, pDeZigZag = %08X, Width = %d, Height = %d\n",
        pOut, pIn, pDeZigZag, Width, Height);
    DEBUG0(vc1DEBUG_ZZ, "pIn =\n");
    DEBUGRECT16(vc1DEBUG_ZZ, pIn, 8, 8, 8);

    for(j = 0; j < Height; j++)
    {
        for(i = 0; i < Width; i++)
        {
            pOut[*pDeZigZag] = *pIn;

            pDeZigZag++;
            pIn++;
        }
    }

    DEBUG0(vc1DEBUG_ZZ, "pOut =\n");
    DEBUGRECT16(vc1DEBUG_ZZ, pOut, 8, 8, 8);

}


/*
 * Description:
 * Apply de-zigzag to input to produce block, depending on block type
 *
 * Remarks:
 * No remarks
 *
 * Inputs:
 * pOut - array in which to store de-zigzagged data
 * pIn  - input zigzagged data
 *
 * Outputs:
 * pOut - newly de-zigzagged data
 *
 * Return Value:
 * None
 *
 */


void vc1DECZZ_DeZigZagBlock(HWD16 *pOut, HWD16 *pIn, UBYTE8 ZZTable, vc1_eBlkType eBlkType)
{
    const vc1GENTAB_pZigZag * pTable = vc1GENTAB_pZigZagTables[ZZTable];

    COVERAGE("8.1.1.12");
    DEBUG3(vc1DEBUG_ZZ, "eBlkType = %d, pOut = %08X, pIn = %08X\n", eBlkType, pOut, pIn);


    switch(eBlkType)
    {
        case vc1_BlkIntra:      vc1DECZZ_DeZigZag(pOut, pIn, pTable[0], 8, 8);     break;
        case vc1_BlkIntraLeft:  vc1DECZZ_DeZigZag(pOut, pIn, pTable[1], 8, 8);     break;
        case vc1_BlkIntraTop:   vc1DECZZ_DeZigZag(pOut, pIn, pTable[2], 8, 8);     break;
        case vc1_BlkInter8x8:   vc1DECZZ_DeZigZag(pOut, pIn, pTable[3], 8, 8);     break;
        case vc1_BlkInter8x4:
        {
            vc1DECZZ_DeZigZag(pOut       , pIn       , pTable[4], 8, 4);
            vc1DECZZ_DeZigZag(pOut + 32  , pIn + 32  , pTable[4], 8, 4);
            COVERAGE("8.3.6.2.2");
            COVERAGE("8.3.6.2.5");
            break;
        }
        case vc1_BlkInter4x8:
        {
            vc1DECZZ_DeZigZag(pOut       , pIn       , pTable[5], 4, 8);
            vc1DECZZ_DeZigZag(pOut + 4   , pIn + 32  , pTable[5], 4, 8);
            COVERAGE("8.3.6.2.2");
            COVERAGE("8.3.6.2.5");
            break;
        }
        case vc1_BlkInter4x4:
        {
            vc1DECZZ_DeZigZag(pOut       , pIn       , pTable[6], 4, 4);
            vc1DECZZ_DeZigZag(pOut + 4   , pIn + 16  , pTable[6], 4, 4);
            vc1DECZZ_DeZigZag(pOut + 32  , pIn + 32  , pTable[6], 4, 4);
            vc1DECZZ_DeZigZag(pOut + 36  , pIn + 48  , pTable[6], 4, 4);
            COVERAGE("8.3.6.2.2");
            COVERAGE("8.3.6.2.5");
            break;
        }
    }
}


