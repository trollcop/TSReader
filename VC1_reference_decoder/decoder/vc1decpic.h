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
 * vc1decpic.h:
 * Bitstream unpack picture layer headers
 *
 */

#ifndef VC1DECPIC_H
#define VC1DECPIC_H

#ifdef cplusplus
extern "C" {
#endif

/**
 * Description
 * Copy a reference picture to the display picture buffer.
 *
 * Remark
 * This picture may be one frame or two fields.
 *
 * Inputs
 * pState       - pointer to decoder state
 * pRef         - pointer to reference picture
 *
 * Outputs
 * pState->sPicture written
 */

void vc1DECPIC_DisplayPicture(  vc1DEC_sState * pState,
                                vc1_sReferencePicture * pRef);


/**
 * Description:
 * Unpack the picture and lower layers of the bitstream.
 *
 * Remarks:
 * This function decides what profile and frame type is being decoded,
 * and selects sub functions accordingly.
 *
 * Inputs:
 * pState - decoder state structure, into which data will be put
 * pBitstream - structure representing position in bitstream
 *
 * Outputs:
 * pBitstream - updated with new position in bitstream
 * pState - filled out with information from bitstream
 *
 * Return Value:
 * Standard vc1_eResult result. See enumeration for possible result codes.
 *
 */
vc1_eResult vc1DECPIC_UnpackPictureLayer(   vc1DEC_sState * pState,
                                            vc1DEC_sBitstream * pBitstream);


/**
 * Description:
 * Unpack the field picture and lower layers of the bitstream.
 *
 * Remarks:
 * None
 *
 * Inputs:
 * pState       - decoder state structure, into which data will be put
 * pBitstream   - structure representing position in bitstream
 *
 * Outputs:
 * pBitstream   - updated with new position in bitstream
 * pState       - filled out with information from bitstream
 *
 * Return Value:
 * Standard vc1_eResult result. See enumeration for possible result codes.
 *
 */
vc1_eResult vc1DECPIC_UnpackFieldPictureLayer(  vc1DEC_sState * pState,
                                                vc1DEC_sBitstream * pBitstream);


/**
 * Description:
 * Unpack a picture layer present in a slice 
 *
 * Remarks:
 * None
 *
 * Inputs:
 * pState       - decoder state structure, into which data will be put
 * pBitstream   - structure representing position in bitstream
 *
 * Outputs:
 * pBitstream   - updated with new position in bitstream
 * pState       - filled out with information from bitstream
 *
 * Return Value:
 * Standard vc1_eResult result. See enumeration for possible result codes
 *
 */
vc1_eResult vc1DECPIC_UnpackInSlicePictureLayer(    vc1DEC_sState * pState,
                                                    vc1DEC_sBitstream * pBitstream);


/**
 * Description:
 * Unpack the syncmarker.
 *
 * Remarks:
 * Discards any payload, although it is available via debug output.
 *
 * Inputs:
 * pBitstream   - pointer to struct representing the current bitstream
 *
 * Outputs:
 * pBitstream   - updated with new position
 *
 * Return Value:
 * Standard vc1_eResult result. See enumeration for possible result codes.
 * 
 */
vc1_eResult vc1DECPIC_UnpackSyncmarker(vc1DEC_sBitstream * pBitstream);

#ifdef cplusplus
}
#endif
#endif

