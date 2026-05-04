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
 * vc1predcbp.c
 *
 * Predict the Coded Block Pattern of the current Macroblock
 * from surrouding Macroblocks.
 *
 *  
 */

#include "vc1types.h"
#include "vc1debug.h"
#include "vc1pred.h"
#include "vc1predcbp.h"

/*
 * Description:
 * Generate coded block pattern from prediction and differential
 *
 * Inputs:
 * pPos     - pointer to position of current macroblock
 * CBPCY    - coded block pattern differential bit5=Y0 bit0=Cr
 *
 * Returns:
 * The absolute CBPCY (prediction applied)
 */

int vc1PREDCBP_ApplyCBPCYPred(vc1_sPosition *pPos, int CBPCY)
{
    vc1_sMB *pA, *pB, *pC;
    int LT3=0, T2=0, T3=0, L1=0, L3=0;

    /* Get coded flag for neighboring blocks */
    pA = vc1PRED_pTopMB(pPos);
    pB = vc1PRED_pTopLeftMB(pPos);
    pC = vc1PRED_pLeftMB(pPos);

    if (pA)
    {
        T2 = pA->sBlk[vc1_BlkY2].Coded;
        T3 = pA->sBlk[vc1_BlkY3].Coded;
    }
    if (pB)
    {
        LT3 = pB->sBlk[vc1_BlkY3].Coded;
    }
    if (pC)
    {
        L1 = pC->sBlk[vc1_BlkY1].Coded;
        L3 = pC->sBlk[vc1_BlkY3].Coded;
    }   

    /* Apply Y0 prediction */
    if (LT3==T2)
    {
        CBPCY ^= (L1<<5);
    }
    else
    {
        CBPCY ^= (T2<<5);
    }

    /* Apply Y1 prediction */
    if (T2==T3)
    {
        CBPCY ^= (CBPCY & (1<<5))>>1; /* Set predicted_Y1 = predicted_Y0 */
    }
    else
    {
        CBPCY ^= (T3<<4);
    }

    /* Apply Y2 prediction */
    if (L1 == (CBPCY>>5))
    {
        CBPCY ^= (L3<<3);
    }
    else
    {
        CBPCY ^= (CBPCY & (1<<5))>>2; /* Set predicted_Y2 = predicted_Y0 */
    }

    /* Apply Y3 prediction */
    if ((CBPCY>>5) == ((CBPCY>>4)&1) )
    {
        CBPCY ^= (CBPCY & (1<<3))>>1; /* Set predicted_Y3 = predicted_Y2 */
    }
    else
    {
        CBPCY ^= (CBPCY & (1<<4))>>2; /* Set predicted_Y3 = predicted_Y1 */
    }

    COVERAGE("8.1.1.5");
    return CBPCY;
}

/*
 * Description:
 * Generates the coded block pattern differential
 *
 * Inputs:
 * pPos     - pointer to position of current macroblock
 * CBPCY    - absolute coded block patternl bit5=Y0 bit0=Cr
 *
 * Returns:
 * The differential CBPCY (prediction subtracted)
 */

int vc1PREDCBP_CBPCYDifferential(vc1_sPosition *pPos, int CBPCY)
{
    vc1_sMB *pA, *pB, *pC;
    int LT3=0, T2=0, T3=0, L1=0, L3=0;
    int DCBPCY;

    /* Get coded flag for neighboring blocks */
    pA = vc1PRED_pTopMB(pPos);
    pB = vc1PRED_pTopLeftMB(pPos);
    pC = vc1PRED_pLeftMB(pPos);

    if (pA)
    {
        T2 = pA->sBlk[vc1_BlkY2].Coded;
        T3 = pA->sBlk[vc1_BlkY3].Coded;
    }
    if (pB)
    {
        LT3 = pB->sBlk[vc1_BlkY3].Coded;
    }
    if (pC)
    {
        L1 = pC->sBlk[vc1_BlkY1].Coded;
        L3 = pC->sBlk[vc1_BlkY3].Coded;
    }

    /* Apply Y0 prediction */
    if (LT3==T2)
    {
        DCBPCY = CBPCY ^ (L1<<5);
    }
    else
    {
        DCBPCY = CBPCY ^ (T2<<5);
    }

    /* Apply Y1 prediction */
    if (T2==T3)
    {
        DCBPCY ^= (CBPCY & (1<<5))>>1;      /* XOR with Y0 */
    }
    else
    {
        DCBPCY ^= (T3<<4);
    }

    /* Apply Y2 prediction */
    if (L1 == (CBPCY>>5))
    {
        DCBPCY ^= (L3<<3);
    }
    else
    {
        DCBPCY ^= (CBPCY & (1<<5))>>2;      /* XOR with Y0 */
    }

    /* Apply Y3 prediction */
    if ((CBPCY>>5) == ((CBPCY>>4)&1) )
    {
        DCBPCY ^= (CBPCY & (1<<3))>>1;      /* XOR with Y2 */
    }
    else
    {
        DCBPCY ^= (CBPCY & (1<<4))>>2;      /* XOR with Y1 */
    }

    return DCBPCY;
}
