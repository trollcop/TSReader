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
 * vc1tools.h:
 * Utility functions
 *
 */

#ifndef VC1TOOLS_H
#define VC1TOOLS_H

#ifdef cplusplus
extern "C" {
#endif

/**
 * Description:
 * Take the median value of three inputs.
 *
 * Inputs:
 *  a   - first value
 *  b   - second value
 *  c   - third value
 *
 * Returns:
 *      Median of a, b and c.
 */

int vc1TOOLS_Median3(int a, int b, int c);

/**
 * Description:
 * Take the median value of four inputs.
 *
 * Inputs:
 *  a   - first value
 *  b   - second value
 *  c   - third value
 *  d   - fourth value
 *
 * Returns:
 *      Average of the two middle values.
 */

int vc1TOOLS_Median4(int a, int b, int c, int d);

/**
 * Description:
 * Range reduce an area.
 *
 * Inputs:
 * pData    - pointer to top left of area
 * Bpl      - bytes per line
 * Width    - area width
 * Height   - area height
 * Scale    - scale factor times 8
 *
 * Outputs:
 * pData    - area range reduced
 */

void vc1TOOLS_RangeReduce(UBYTE8 *pData, int Bpl, int Width, int Height, int Scale);

/**
 * Description:
 * Obtain a pointer into a picture at a block location.
 *
 * Inputs:
 * pC        - pointer to initialised component structure
 * pRefPic   - pointer to a reference picture structure 
 * pPosition - pointer to a position structure indicating the current MB
 * eBlk      - the block number
 *
 * Outputs:
 * pC        - pointer to filled out component structure indicating
 *             block address and bytes per line of component plane
 *
 */

void vc1TOOLS_GetPictureDestination(
    vc1_sComponent * pC,
    vc1_sReferencePicture * pRefPic,
    vc1_sPosition * pPosition,
    int eBlk
);

/**
 * Description:
 * Initialise a reference picture structure.
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
);

/**
 * Description:
 * Switches the old and new reference buffers.
 *
 * Inputs:
 * pPos     - pointer to position structure with:
 *            - pReferenceOld = reference to discard
 *            - pReferenceNew = reference to make old
 *            - RangeRed      = is the picture range reduced
 */

void vc1TOOLS_NewReference(vc1_sPosition *pPos);

/**
 * Description:
 * Copy image data from one reference picture to another.
 *
 * Remarks:
 * This function is called when dealing with a skipped frame.
 *
 * Inputs:
 * pOut     - Pointer to destination reference picture
 * pIn      - Pointer to source reference picture
 *
 * Returns:
 *      Result code.
 */

vc1_eResult vc1TOOLS_CopyReference(vc1_sReferencePicture *pOut, vc1_sReferencePicture *pIn);

/**
 * Description:
 * Intensity compensate and pad reference pictures ready for motion compensation.
 *
 * Inputs:
 * pPos             - pointer to details of current image and reference images
 * pIntensityComp   - pointer to the intensity compensation values to apply (one per field)
 */

void vc1TOOLS_ICPadReferencePicture(vc1_sPosition *pPos, vc1_sIntensityComp pIntensityComp[2]);

/**
 * Description:
 * Upsample filter.
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
);

/**
 * Description:
 * Intensity compensate a reference picture ready for prediction.
 *
 * Inputs:
 * pRef     - pointer to the reference picture to compensate
 * pIC      - pointer to intensity compensation data structure
 * Field    - field to compensate for interlace field pictures
 *
 * Outputs:
 * pRef     - picture compensated
 *
 */

void vc1TOOLS_IntensityCompensate(
    vc1_sReferencePicture *pRef, 
    vc1_sIntensityComp    *pIC,
    int Field
);

#ifdef cplusplus
}
#endif

#endif
