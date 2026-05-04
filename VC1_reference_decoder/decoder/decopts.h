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
 * decopts.h
 *
 * Decoder parameter options.
 *
 *  
 */

#ifndef DECOPTS_H
#define DECOPTS_H

#ifdef cplusplus
extern "C" {
#endif

/**
 * Description: The main table of options for the decoder.
 *  This defines all the options accepted by the decoder, and defines
 *  what values will be placed in which configuration variables.
 */
extern const RDOPTS_sOptionDefinition DECOPTS_sDecoderOptions[];

/*
 * Option variables
 */
extern char  *DECOPTS_InputFile;
extern char  *DECOPTS_OutputFile;
extern RDOPTS_eBitstreamFormatType DECOPTS_InputType;
extern WORD32 DECOPTS_NumFrames;
extern vc1DEC_sDecoderConfiguration DECOPTS_sDecoderSetup;
extern FLAG   DECOPTS_ShowUserData;


/**
 * Description:
 *  Performs any required verification of parameters not
 *  done as part of option parsing.
 *
 * Returns:
 *  vc1_ResultOK on success 
 */
vc1_eResult DECOPTS_ValidateParameters(void);

/**
 * Description:
 *  Calculate a bitstream buffer size that will not be
 *  overflowed by any frame.
 *
 * Returns:
 *  the required buffer size in bytes
 */
UWORD32 DECOPTS_CalculateBitstreamBufferSize(void);

/**
 * Description:
 *  Check for any chained option updates. This is useful for debugging:
 *  debug output can be disabled until the frame of interest.
 */
vc1_eResult DECOPTS_UpdateOptions(void);

#ifdef cplusplus
}
#endif

#endif /* ndef DECOPTS_H */
