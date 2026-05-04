/*******************************************************************

This software module is provided in full accordance with SMPTE
Administrative Procedures.  
 
This software module was developed by ARM Ltd. under contract by 
Microsoft Corp, and provided by Microsoft Corp to SMPTE.  It was 
edited by:
             
Jan 22, 2005: Radha Krishna Giduthuri (radha@vilogic.com)
  - replaced the implementation of informative Annex A.2 as in CD2r2
    with proposed normative inverse transform specification (Annex A.1)
            
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
viLOGIC, Inc.


This copyright notice must be included in all copies or derivative 
works. 
 
Copyright (c) 2004
******************************************************************/

/*
 * vc1itrans.c
 *
 * vc1 Decoder Inverse Transform module
 *
 */

#include "vc1types.h"
#include "vc1debug.h"

#include "vc1itrans.h"

/* 8x8 Inverse Transform matrix defined by the standard */

static const HWD16 T8[8][8] = 
{
    {  12,  12,  12,  12,  12,  12,  12,  12 },
    {  16,  15,   9,   4,  -4,  -9, -15, -16 },
    {  16,   6,  -6, -16, -16,  -6,   6,  16 },
    {  15,  -4, -16,  -9,   9,  16,   4, -15 },
    {  12, -12, -12,  12,  12, -12, -12,  12 },
    {   9, -16,   4,  15, -15,  -4,  16,  -9 },
    {   6, -16,  16,  -6,  -6,  16, -16,   6 },
    {   4,  -9,  15, -16,  16, -15,   9,  -4 }
};



/* 4x4 Inverse Transform matrix defined by the standard */

static const HWD16 T4[4][4] =
{
    {  17,  17,  17,  17 },
    {  22,  10, -10, -22 },
    {  17, -17, -17,  17 },
    {  10, -22,  22, -10 }
};

/* C8 vector defined by the standard */


static const HWD16 C8[8] = 
{
    0,   0,   0,   0,   1,   1,   1,   1
};


/* C4 vector defined by the standard */


static const HWD16 C4[4] = 
{
    0,   0,   0,   0
};


/* The inverse transform an 8x8, 8x4, 4x8, and 4x4 block in an 8x8 buffer
 *
 * Inputs:
 *    pR      - Pointer to output buffer (MxN block)
 *    pD      - Pointer to input buffer (MxN block)
 *    M       - Width of the block
 *    N       - Height of the block
 */


static void vc1ITRANS_InverseTransform_AnnexA1(HWD16 *pR, HWD16 *pD, int M, int N)
{
    HWD16 pE[8*8];
    const HWD16 *pT, *pC;
    UHWD16 i, j, k;
    WORD32 acc;


    /* Row transform:
     *     Equation: E = (M * T + 4) >> 3
     */
    pT = (M == 8) ? T8[0] : T4[0];
    for (j = 0; j < N; j++)
    {
        for (i = 0; i < M; i++)
        {
            for (acc = 0, k = 0; k < M; k++)
            {
                acc += pD[j*8+k]*pT[k*M+i];
            }
            pE[j*8+i] = (HWD16) ((acc + 4) >> 3);
        }
    }


    /* Column transform:
     *   Equation: R = (T' * E + C * 1 + 64) >> 7
     */
    pT = (N == 8) ? T8[0] : T4[0];
    pC = (N == 8) ? C8 : C4;
    for (i = 0; i < M; i++)
    {
        for (j = 0; j < N; j++)
        {
            for (acc = 0, k = 0; k < N; k++)
            {
                acc += pE[i+k*8]*pT[k*N+j];
            }
            pR[i+j*8] = (HWD16) ((acc + pC[j] + 64) >> 7);
        }
    }
}


/*
 * Description:
 * 2D inverse transform an 8x8 block of coefficients
 *
 * Remarks:
 *
 * Inputs:
 * pOut     - pointer to the output 64-element array
 * pIn      - pointer to the input 64-element array
 * eType    - tranform type (8x8, 8x4, 4x8, 4x4)
 *
 * Outputs:
 * pOut     - pointer to the output 64-element array
 *
 * Return Value:
 * None
 *
 */


void vc1ITRANS_InverseTransformBlock(HWD16 *pOut, HWD16 *pIn, vc1_eBlkType eType)
{
    /* COVERAGE of ("8.1.1.16") */


    if (eType == vc1_BlkInter4x4)
    {
        /* Four 4x4 blocks in an 8x8 buffer */
        vc1ITRANS_InverseTransform_AnnexA1(pOut,       pIn,       4, 4);
        vc1ITRANS_InverseTransform_AnnexA1(pOut+4,     pIn+4,     4, 4);
        vc1ITRANS_InverseTransform_AnnexA1(pOut+4*8,   pIn+4*8,   4, 4);
        vc1ITRANS_InverseTransform_AnnexA1(pOut+4*8+4, pIn+4*8+4, 4, 4);
    }
    else if (eType == vc1_BlkInter4x8)
    {
        /* Two 4x8 blocks in an 8x8 buffer */
        vc1ITRANS_InverseTransform_AnnexA1(pOut,       pIn,       4, 8);
        vc1ITRANS_InverseTransform_AnnexA1(pOut+4,     pIn+4,     4, 8);
    }
    else if (eType == vc1_BlkInter8x4)
    {
        /* Two 8x4 blocks in an 8x8 buffer */
        vc1ITRANS_InverseTransform_AnnexA1(pOut,       pIn,       8, 4);
        vc1ITRANS_InverseTransform_AnnexA1(pOut+4*8,   pIn+4*8,   8, 4);
    }
    else
    {
        /* One 8x8 block */
        vc1ITRANS_InverseTransform_AnnexA1(pOut,       pIn,       8, 8);
    }
}

/* 8x8 Inverse Transform Even Part matrix defined by the standard in Annex A.2 */

static const HWD16 T8E[8][8] = 
{
    {  6,  6,  6,  6,  6,  6,  6,  6 },
    {  8,  7,  4,  2, -2, -4, -7, -8 },
    {  8,  3, -3, -8, -8, -3,  3,  8 },
    {  7, -2, -8, -5,  5,  8,  2, -7 },
    {  6, -6, -6,  6,  6, -6, -6,  6 },
    {  4, -8,  2,  7, -7, -2,  8, -4 },
    {  3, -8,  8, -3, -3,  8, -8,  3 },
    {  2, -5,  7, -8,  8, -7,  5, -2 }
};

/* 4x4 Inverse Transform Even Part matrix defined by the standard in Annex A.2*/

static const HWD16 T4E[4][4] =
{
    {   8,   8,   8,   8 },
    {  11,   5,  -5, -11 },
    {   8,  -8,  -8,   8 },
    {   5, -11,  11,  -5 }
};

/* The 8-point 1D Inverse Transform (Row)
 *
 * Inputs:
 *    pOut    - Pointer to output row
 *    pIn     - Pointer to input row
 */

static void vc1ITRANS_InverseTransformRow8pt_AnnexA2(HWD16 *pOut, HWD16 *pIn)
{
    int i,j,acc;
    HWD16 acc16;
    
    for (i=0; i<8; i++)
    {
        acc = 4;
        for (j=0; j<8; j++)
        {
            acc += pIn[j]*T8[j][i];
        }

        /* Must reduce modulo 16-bit BEFORE right shift to match spec */
        acc16 = (HWD16)acc;
        pOut[i] = (HWD16)(acc16 >> 3);
    }
}

/* The 4-point 1D Inverse Transform (Row)
 *
 * Inputs:
 *    pOut    - Pointer to output row
 *    pIn     - Pointer to input row
 */

static void vc1ITRANS_InverseTransformRow4pt_AnnexA2(HWD16 *pOut, HWD16 *pIn)
{
    int i,j,acc;
    HWD16 acc16;
    
    for (i=0; i<4; i++)
    {
        acc = 4;
        for (j=0; j<4; j++)
        {
            acc += pIn[j]*T4[j][i];
        }

        /* Must reduce modulo 16-bit BEFORE right shift to match spec */
        acc16   = (HWD16)acc;
        pOut[i] = (HWD16)(acc16 >> 3);
    }
}

/* The 8-point 1D Inverse Transform (Column)
 *
 * Inputs:
 *    pOut    - Pointer to output column
 *    pIn     - Pointer to input column
 */

static void vc1ITRANS_InverseTransformCol8pt_AnnexA2(HWD16 *pOut, HWD16 *pIn)
{
    int i, j, acc, D2a, D2b;
    HWD16 acc16;
    
    D2a = (pIn[3*8] + pIn[5*8]) >> 1;
    D2b = (pIn[1*8] + pIn[7*8]) >> 1;
    
    for (i=0; i<8; i++)
    {
        acc = D2a;
        if ((i&3)==1 || (i&3)==2)
        {
          acc = D2b;
        }
        if (i>=4)
        {
          acc = -acc;
        }
        
        for (j=0; j<8; j++)
        {
            acc += pIn[8*j]*T8E[j][i];
        }

        /* Must reduce modulo 16-bit BEFORE right shift to match spec */
        acc16 = (HWD16)(acc + 32);
        pOut[8*i] = (HWD16)(acc16 >> 6);
    }
}

/* The 4-point 1D Inverse Transform (Column)
 *
 * Inputs:
 *    pOut    - Pointer to output column
 *    pIn     - Pointer to input column
 */

static void vc1ITRANS_InverseTransformCol4pt_AnnexA2(HWD16 *pOut, HWD16 *pIn)
{
    int i, j, acc, D2a, D2b;
    HWD16 acc16;
    
    D2a = (pIn[0*8] + pIn[2*8]) >> 1;
    D2b = (pIn[0*8] - pIn[2*8]) >> 1;
    
    for (i=0; i<4; i++)
    {
        acc = D2a;
        if (i==1 || i==2)
        {
          acc = D2b;
        }
        
        for (j=0; j<4; j++)
        {
            acc += pIn[8*j]*T4E[j][i];
        }

        /* Must reduce modulo 16-bit BEFORE right shift to match spec */
        acc16 = (HWD16)(acc + 32);
        pOut[8*i] = (HWD16)(acc16 >> 6);
    }
}


/*
 * Description:
 * 2D inverse transform an 8x8 block of coefficients
 *
 * Remarks:
 *
 * Inputs:
 * pOut     - pointer to the output 64-element array
 * pIn      - pointer to the input 64-element array
 * eType    - tranform type (8x8, 8x4, 4x8, 4x4)
 *
 * Outputs:
 * pOut     - pointer to the output 64-element array
 *
 * Return Value:
 * None
 *
 */

void vc1ITRANS_InverseTransformBlock_AnnexA2(HWD16 *pOut, HWD16 *pIn, vc1_eBlkType eType)
{
    HWD16 pTmp[8*8];
    int i;

    /* COVERAGE of ("8.1.1.16") */
    /* Row transform */
    if (eType==vc1_BlkInter4x8 || eType==vc1_BlkInter4x4)
    {
        /* Row transform is 4x1 rows */
        for (i=0; i<64; i+=4)
        {
            vc1ITRANS_InverseTransformRow4pt_AnnexA2(&pTmp[i], &pIn[i]);
        }
    }
    else
    {
        /* Row transform is 8x1 rows */
        for (i=0; i<64; i+=8)
        {
            vc1ITRANS_InverseTransformRow8pt_AnnexA2(&pTmp[i], &pIn[i]);
        }
    }

    /* Column Transform */
    if (eType==vc1_BlkInter8x4 || eType==vc1_BlkInter4x4)
    {
        /* Column transform is 1x4 columns */
        for (i=0; i<8; i++)
        {
            vc1ITRANS_InverseTransformCol4pt_AnnexA2(&pOut[i],    &pTmp[i]   );
            vc1ITRANS_InverseTransformCol4pt_AnnexA2(&pOut[i+32], &pTmp[i+32]);
        }
    }
    else
    {
        /* Column transform is 1x8 columns */
        for (i=0; i<8; i++)
        {
            vc1ITRANS_InverseTransformCol8pt_AnnexA2(&pOut[i], &pTmp[i]);
        }
    }
}
