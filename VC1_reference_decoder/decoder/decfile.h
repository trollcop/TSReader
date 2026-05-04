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
 * decfile.h
 *
 * Writing out of frame data to file.
 *
 *  
 */

#ifndef DECFILE_H
#define DECFILE_H

#ifdef cplusplus
extern "C" {
#endif

/**
 * Description:
 *  Initialise the frame writer component, including allocation of frame buffers.
 *
 * Inputs:
 *   pPicture - a pointer to the picture structure to hold buffer pointers
 *   pState   - a pointer to the state structure holding the picture size
 *
 * Outputs:
 *   pPicture is updated with pointers to the allocated buffers
 *
 * Returns:
 *   vc1_ResultOK on success
 */
vc1_eResult DECFILE_InitialiseFrameWriter(vc1_sPicture *pPicture, const vc1DEC_sState *pState);

/**
 * Description:
 *  Free any resources held by the frame writer component.
 *
 * Inputs:
 *   pPicture - a pointer to the picture structure holding buffer pointers
 */
void DECFILE_FinaliseFrameWriter(vc1_sPicture *pPicture);

/**
 * Description:
 * Write the data for the next frame to the output file.
 *
 * Remarks:
 * Repeat the first frame or drop the last frame so as to match
 * other decoders.
 *
 * Inputs:
 * pPicture      the picture buffers to write out
 * pState        pointer to the state holding the actual picture size
 * pFramesOutput pointer to display frame number
 *
 * Returns:
 *   vc1_ResultOK on success
 */
vc1_eResult DECFILE_WriteFileFrame(
    vc1_sPicture *pPicture,
    const vc1DEC_sState *pState,
    UWORD32 *pFramesOutput
);

/**
 * Description:
 *  Initialise the bitstream file reader component.
 *
 * Inputs:
 *  pBitstream - the bitstream buffer to initialise
 *
 * Returns:
 *   vc1_ResultOK on success
 */
vc1_eResult DECFILE_InitialiseBitstreamFileReader(vc1DEC_sBitstream *pBitstream);

/**
 * Description:
 *  Adjust input file buffering to match the profile and level.
 *
 * Inputs:
 *  pBitstream - the bitstream buffer to adjust
 *
 * Returns:
 *   vc1_ResultOK on success
 */
vc1_eResult DECFILE_AdjustBitstreamBuffering(vc1DEC_sBitstream *pBitstream);

/**
 * Description:
 *  Free any resources held by the bitstream file reader component.
 *
 * Inputs:
 *  pBitstream - the bitstream buffer to free
 */
void DECFILE_FinaliseBitstreamFileReader(vc1DEC_sBitstream *pBitstream);

/**
 * Description:
 *  Read more data into the bitstream buffer.
 *
 * Inputs:
 *  pBitstream       - the bitstream buffer to refill
 *  FramesOutput     - the number of frames written to the output
 *  FramesDecoded    - the number of frames read from the input
 *  pStartCodeSuffix - pointer to a byte to receive the start code type for advanced profile
 *
 * Outputs:
 *  *pStartCodeSuffix - start code type if in advanced profile
 *
 * Returns:
 *   vc1_ResultOK on success
 */
vc1_eResult DECFILE_ReadBitstreamFile(  vc1DEC_sBitstream *pBitstream,
                                        UWORD32 FramesOutput,
                                        UWORD32 FramesDecoded,
                                        UBYTE8 *pStartCodeSuffix);


/**
 * Description:
 *  Get the number of bytes read from the bitstream, for HRD model
 *
 * Returns:
 *   Number of bytes read
 */
UWORD32 DECFILE_GetHRDBytesRead(void);

/**
 * Description:
 *  Set the number of bytes read from the bitstream, for HRD model
 *
 * Inputs:
 *  Bytes - Value to set HRDBytesRead. Usually 0.
 */
void DECFILE_SetHRDBytesRead(UWORD32 Bytes);


#ifdef cplusplus
}
#endif

#endif /* def DECFILE_H */

