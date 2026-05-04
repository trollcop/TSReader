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
 * vc1decmv.h:
 * Motion vector prediction functions specific to the decoder
 *
 */


#ifndef VC1DECMV_H
#define VC1DECMV_H

#ifdef cplusplus
extern "C" {
#endif

/**
 * Description:
 * Apply predicted motion vectors to the decoded differential motion vectors.
 *
 * Remarks:
 * None.
 *
 * Inputs:
 * pState       - pointer to the decoder's state
 * eBlk         - block number of current block
 * pPred        - pointer to the predicted motion vectors
 * Backwards    - 0=forward mv, 1=backward mv
 *
 * Outputs:
 * pState       - updated with new motion vectors
 *
 */
void vc1DECMV_ApplyMVPrediction(    vc1DEC_sState * pState, 
                                    int eBlk, 
                                    vc1_sMV * pPred, 
                                    int Backwards);



/**
 * Description:
 * Unpack differential motion vectors for progressive bitstreams.
 *
 * Remarks:
 * None.
 *
 * Inputs:
 * pMB          - pointer to current macroblock
 * pState       - pointer to the decoder's state
 * pBitstream   - pointer to struct representing the current bitstream
 * eBlk         - block number of current block
 * Backwards    - 0=forward mv, 1=backward mv
 *
 * Outputs:
 * pMB          - updated with new differential motion vectors
 * pBitstream   - updated with new position
 *
 * Return Value:
 * Standard vc1_eResult result. See the enumeration for possible result codes.
 * 
 */
vc1_eResult vc1DECMV_UnpackMVData(vc1_sMB * pMB,
                                  vc1DEC_sState * pState,
                                  vc1DEC_sBitstream * pBitstream,
                                  int eBlk,
                                  int Backwards);




/**
 * Description:
 * Unpack differential motion vectors for interlaced bitstreams.
 *
 * Remarks:
 * None.
 *
 * Inputs:
 * pMB          - pointer to current macroblock
 * pPredFlag    - pointer to location into which can be written the prediction flag
 * pState       - pointer to the decoder's state
 * pBitstream   - pointer to struct representing the current bitstream
 * eBlk         - block number of current block
 * TwoField     - if true, there are two reference field, else one
 * Backwards    - 0=forward mv, 1=backward mv
 *
 * Outputs:
 * pMB          - updated with new differential motion vectors
 * pPredFlag    - updated with predicted flag status
 * pBitstream   - updated with new position
 *
 * Return Value:
 * Standard vc1_eResult result. See the enumeration for possible result codes.
 * 
 */
vc1_eResult vc1DECMV_UnpackMVDataInterlace( vc1_sMB * pMB,
                                            FLAG * pPredFlag,
                                            vc1DEC_sState * pState,
                                            vc1DEC_sBitstream * pBitstream,
                                            int eBlk,
                                            FLAG TwoField,
                                            int Backwards);




#ifdef cplusplus
}
#endif
#endif

