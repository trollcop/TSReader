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
 * vc1interp.h:
 * Block interpolation functions
 *
 */

#ifndef VC1INTERP_H
#define VC1INTERP_H

#ifdef cplusplus
extern "C" {
#endif


/*
 * Structure to hold filter parameters for bicubic filtering
 */

typedef struct
{
    BYTE8   F0;
    BYTE8   F1;
    BYTE8   F2;
    BYTE8   F3;
} vc1INTERP_sBicubicFilterParams;

/*
 * Maximum patch size handled by bicubic filter functions
 */

#define VC1INTERP_MAX_PATCH_SIZE 17



/**
 * Description:
 * Bicubic interpolation of a patch to quarter pixel position resolution.
 *
 * Inputs:
 * pC         - pointer to a component into which the filtered patch will be written
 * pInterp    - pointer to structure containing parameters required for operation
 * X          - X position in quarter pixel units
 * Y          - Y position in quarter pixel units
 *
 * Outputs:
 * pC         - updated with the filtered patch
 *
 */
void vc1INTERP_InterpPatchQuarterPelBicubic(vc1_sComponent * pC,
                                            vc1_sInterpolate * pInterp,
                                            WORD32 X,
                                            WORD32 Y);


/**
 * Description:
 * Bilinear interpolation of a patch to quarter pixel position resolution
 *
 * Inputs:
 * pC         - pointer to a component into which the filtered patch will be written
 * pInterp    - pointer to structure containing parameters required for operation
 * X          - X position in half pixel units
 * Y          - Y position in half pixel units
 *
 * Outputs:
 * pC         - updated with the filtered patch
 *
 */
void vc1INTERP_InterpPatchQuarterPelBilinear(vc1_sComponent * pC,
                                             vc1_sInterpolate * pInterp,
                                             WORD32 X,
                                             WORD32 Y);


/**
 * Description:
 * Bilinear interpolation of a patch to half pixel position resolution.
 *
 * Remarks:
 * This function is implemented using the quarter pixel resolution routines.
 *
 * Inputs:
 * pC         - pointer to a component into which the filtered patch will be written
 * pInterp    - pointer to structure containing parameters required for operation
 * X          - X position in half pixel units
 * Y          - Y position in half pixel units
 *
 * Outputs:
 * pC         - updated with the filtered patch
 *
 */
void vc1INTERP_InterpPatchHalfPelBilinear(vc1_sComponent * pC,
                                          vc1_sInterpolate * pInterp,
                                          WORD32 X,
                                          WORD32 Y);



/**
 * Description:
 * Bicubic interpolation of a patch to half pixel position resolution.
 *
 * Remarks:
 * This function is implemented using the quarter pixel resolution routines.
 *
 * Inputs:
 * pC         - pointer to a component into which the filtered patch will be written
 * pInterp    - pointer to structure containing parameters required for operation
 * X          - X position in half pixel units
 * Y          - Y position in half pixel units
 *
 * Outputs:
 * pC         - updated with the filtered patch
 *
 */
void vc1INTERP_InterpPatchHalfPelBicubic(vc1_sComponent * pC,
                                         vc1_sInterpolate * pInterp,
                                         WORD32 X,
                                         WORD32 Y);



/**
 * Description:
 * Predict macroblock from a reference picture.
 *
 * Remarks:
 * Uses the macroblock type and motion vectors to form a prediction.
 *
 * Inputs:
 * pPredBlk   - pointer to an array into which the filtered blocks will be written
 * pPosition  - pointer to a position structure indicating the current macroblock
 * RndCtrl    - rounding control flag from picture layer
 *
 * Outputs:
 * pPredBlk   - updated with the filtered blocks
 *
 */
void vc1INTERP_PredictMB(
    UBYTE8 pPredBlk[6][64],
    vc1_sPosition * pPosition,
    FLAG RndCtrl
);

/**
 * Description:
 * Interlace a predicted macroblock.
 *
 * Remarks:
 * Interleaves the top blocks with the bottom blocks, and writes out four
 * interlaced blocks.
 *
 * Inputs:
 * pDestMB   - pointer to initialised buffer to hold the new data
 * pSourceMB - pointer to the source macroblock data
 *
 * Outputs:
 * pDest     - updated with new interlaced data
 *
 */
void vc1INTERP_InterlacePredMB(UBYTE8 pDestMB[6][64], const UBYTE8 pSourceMB[6][64]);



/**
 * Description:
 * Interlace a differential macroblock.
 *
 * Remarks:
 * Interleaves the top blocks with the bottom blocks, and writes out four
 * interlaced blocks.
 *
 * Inputs:
 * pDestMB   - pointer to initialised buffer to hold the new data
 * pSourceMB - pointer to the source macroblock data
 *
 * Outputs:
 * pDest     - updated with new interlaced data
 *
 */
void vc1INTERP_InterlaceDiffMB(HWD16 pDestMB[6][64], const HWD16 pSourceMB[6][64]);



#ifdef cplusplus
}
#endif
#endif
