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
 * rcv.h
 *
 * Raw Format information
 *
 *  
 */

#ifndef RCV_H
#define RCV_H

#ifdef cplusplus
extern "C" {
#endif

/********************************************************************
 * RCV constants
 *******************************************************************/

/*
 * The RCV format contains a type byte. The top bit indicates
 * extension data. We need VC1 type, with extension data.
 */
#define RCV_VC1_TYPE (0x85)
/* Bit 6 of the type indicates V1 if 0, V2 if 1 */
#define RCV_V2_MASK (1 << 6)
/* Top nibble bits of frame size word are flags in V2 */
#define RCV_V2_FRAMESIZE_FLAGS (0xf0000000)
#define RCV_V2_KEYFRAME_FLAG   (0x80000000)

/* V2 extra information has a VBR flag */
#define RCV_V2_VBR_FLAG (0x10000000)

#ifdef cplusplus
}
#endif

#endif /* ndef RCV_H */
