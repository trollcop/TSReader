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
 * vc1debug.h:
 * Debugging defines and structures
 *
 */

#ifndef VC1DEBUG_H
#define VC1DEBUG_H

#ifdef cplusplus
extern "C" {
#endif

typedef struct _tagVC1Decode
{
	unsigned char * pY;
	unsigned char * pU;
	unsigned char * pV;
	int x, y;
	int interlaced;
} VC1DECODE, *PVC1DECODE;

/* Define debug zones to differentiate each program area.
 * Zones with a single bit set can be ORed togther
 * to produce compound zones.
 * Use the hex values to produce values for the options file
 * DebugMask value.
 */

typedef enum {
    /* Decoder layers */
    vc1DEBUG_API        = 0x00000001,   /* API debug   (initialization etc) */
    vc1DEBUG_FRAME      = 0x00000002,   /* Frame debug (print frame number/type)  */
    vc1DEBUG_SEQ        = 0x00000004,   /* Sequence Layer debug */
    vc1DEBUG_PIC        = 0x00000008,   /* Picture Layer debug */
    vc1DEBUG_SLICE      = 0x00000010,   /* Slice Layer debug */
    vc1DEBUG_MB         = 0x00000020,   /* Macroblock layer debug */
    vc1DEBUG_BLK        = 0x00000040,   /* Block layer debug */
    vc1DEBUG_RC         = 0x00000080,   /* Codec rate control/hypothetical ref decoder */
    /* Decoder intermediate data */
    vc1DEBUG_CMP        = 0x00000100,   /* Display run, level, mv values (run/level/mv) */
    vc1DEBUG_BIT        = 0x00000200,   /* Display low level bitstream data */
    vc1DEBUG_BITPL      = 0x00000400,   /* Display bitplane coding debug */
    vc1DEBUG_PDCAC      = 0x00000800,   /* Display DCAC predicted quantized coefficients */
    vc1DEBUG_DCAC       = 0x00001000,   /* Display DCAC prediction */
    vc1DEBUG_QUANT      = 0x00002000,   /* Display quantized coefficients */
    vc1DEBUG_TRANS      = 0x00004000,   /* Display transformed coefficients */
    vc1DEBUG_DBLK       = 0x00008000,   /* Display difference image block (not transformed) */
    vc1DEBUG_PBLK       = 0x00010000,   /* Display motion predicted image block */
    vc1DEBUG_RBLK       = 0x00020000,   /* Display reconstructed block */
    vc1DEBUG_SMOOTH     = 0x00040000,   /* Display overlap smoothing result */
    vc1DEBUG_DEBLK      = 0x00080000,   /* Display deblocking filter result */
    vc1DEBUG_IBLK       = 0x00100000,   /* Display raw input block */
    vc1DEBUG_ZZ         = 0x00200000,   /* Display zig-zagged coefficients */
    vc1DEBUG_MV         = 0x00400000,   /* Display motion vectors */
    vc1DEBUG_REFPICT    = 0x00800000,   /* Dump reference pictures to a file */
    vc1DEBUG_ME         = 0x01000000,   /* Display motion estimation process */
    vc1DEBUG_MBSUM      = 0x02000000,   /* Display block type summary for each macroblock */
    vc1DEBUG_PADIC      = 0x04000000,   /* Display padding and intensity compensation */
    vc1DEBUG_HRD        = 0x08000000,   /* Display Hypothetical Reference Decoder status */
    vc1DEBUG_ENT        = 0x10000000,   /* Display entry point layer debug */

    /* Application */
    vc1DEBUG_OPTIONS    = 0x40000000,   /* Option parsing */
    vc1DEBUG_COVERAGE   = (-1<<31),     /* Coverage output for automated testing */

    /* Unions */
    vc1DEBUG_ALL        = -1            /* NB *Not* all Fs, as enums are only */
                                        /* guaranteed 'int' range */
} vc1DEBUG_eZone;

/*
 * Zone name strings are used within vc1debug.c, but defined here
 * so they are updated with the zone bitmasks.
 */

#define DEBUGZONENAMEb0  "API"
#define DEBUGZONENAMEb1  "FRAME"
#define DEBUGZONENAMEb2  "SEQ"
#define DEBUGZONENAMEb3  "PIC"
#define DEBUGZONENAMEb4  "SLICE"
#define DEBUGZONENAMEb5  "MB"
#define DEBUGZONENAMEb6  "BLK"
#define DEBUGZONENAMEb7  "RC"
#define DEBUGZONENAMEb8  "CMP"
#define DEBUGZONENAMEb9  "BIT"
#define DEBUGZONENAMEb10 "BITPL"
#define DEBUGZONENAMEb11 "PDCAC"
#define DEBUGZONENAMEb12 "DCAC"
#define DEBUGZONENAMEb13 "QUANT"
#define DEBUGZONENAMEb14 "TRANS"
#define DEBUGZONENAMEb15 "DBLK"
#define DEBUGZONENAMEb16 "PBLK"
#define DEBUGZONENAMEb17 "RBLK"
#define DEBUGZONENAMEb18 "SMOOTH"
#define DEBUGZONENAMEb19 "DEBLK"
#define DEBUGZONENAMEb20 "IBLK"
#define DEBUGZONENAMEb21 "ZZ"
#define DEBUGZONENAMEb22 "MV"
#define DEBUGZONENAMEb23 "REFPICT"
#define DEBUGZONENAMEb24 "ME"
#define DEBUGZONENAMEb25 "MBSUM"
#define DEBUGZONENAMEb26 "PADIC"
#define DEBUGZONENAMEb27 "HRD"
#define DEBUGZONENAMEb28 "ENT"
#define DEBUGZONENAMEb29 ""
#define DEBUGZONENAMEb30 "OPTIONS"
#define DEBUGZONENAMEb31 "COVERAGE"

/* Fatal errors and warnings are always reported, even for a release build */
 
#define FATAL vc1DEBUG_Fatal
#define WARN  vc1DEBUG_Warn

/* Displays a fatal error message and continues
 *
 * Inputs:
 *      pFormat     - Pointer to a printf format string
 *      ...         - Further printf parameters
 */
    
void vc1DEBUG_Fatal(char *pFormat, ... );

/* Displays a warning
 *
 * Inputs:
 *      pFormat     - Pointer to a printf format string
 *      ...         - Further printf parameters
 */    
    
void vc1DEBUG_Warn(char *pFormat, ... );


#ifdef NDEBUG

#define DEBUG0(z,s)               ((void)(0))
#define DEBUG1(z,s,a)             ((void)(0))
#define DEBUG2(z,s,a,b)           ((void)(0))
#define DEBUG3(z,s,a,b,c)         ((void)(0))
#define DEBUG4(z,s,a,b,c,d)       ((void)(0))
#define DEBUG5(z,s,a,b,c,d,e)     ((void)(0))
#define DEBUG6(z,s,a,b,c,d,e,f)   ((void)(0))
#define DEBUG7(z,s,a,b,c,d,e,f,g) ((void)(0))
#define ASSERT (void)
#define DEBUGRECT8(a,b,c,d,e)     ((void)(0))
#define DEBUGRECT16(a,b,c,d,e)    ((void)(0))
#define DEBUGREFPICT(a, b, c)     ((void)(0))
#define DEBUGNEWREFPICT(a, b, c)  ((void)(0))
#define DEBUGMB(a, b)             ((void)(0))
#define DEBUGPICT(a, b)           ((void)(0))
#define DEBUGBLKS16(a,b,c,d)      ((void)(0))

#define COVERAGE_NONINT_INT(_n_,_i_) ((void)(0))
#define COVERAGE(_c_) ((void)(0))

#else

/* This variable defines which zones we should display debug messages from */
extern WORD32 vc1DEBUG_Zones;

/* Declare debug functions */

/*
 * Description:
 * Displays a debug message if (vc1DEBUG_Zones & zone) is non zero
 *
 * Inputs:
 * Zone     - Area the debug message applies to (declared as an int
 *            so that logical expressions can be used to combine
 *            zones without giving a compiler warning)
 * pFormat  - Pointer to a printf format string
 * ...      - Futher printf parameters
 */

void vc1DEBUG_Debug(int Zone, char *pFormat, ... );


/* Displays a fatal error message and terminates
 *
 * Inputs:
 *      pFormat     - Pointer to a printf format string
 *      ...         - Further printf parameters
 */
    
void vc1DEBUG_Fatal_Exit(char *pFormat, ... );

/* Display a rectange of 16-bit data
 *
 * Inputs:
 *      pData  - Pointer to data to display
 *      Width  - Rectangle width
 *      Height - Rectangle height
 *      Epl    - Entries per line in pData array
 */

void vc1DEBUG_PrintRectangle16(const HWD16 *pData, int Width, int Height, int Epl);

/*
 * Description:
 * Display a number of 16-bit 8x8 blocks
 *
 * Inputs:
 * pData    - Pointer to blocks to display
 * Width    - Width in blocks to display
 * N        - Number of blocks
 */

void vc1DEBUG_PrintBlocks16(const HWD16 pData[][64], int Width, int N);

/* Display a rectange of 8-bit data
 *
 * Inputs:
 *      pData  - Pointer to data to display
 *      Width  - Rectangle width
 *      Height - Rectangle height
 *      Epl    - Entries per line in pData array
 */

void vc1DEBUG_PrintRectangle8(const UBYTE8 *pData, int Width, int Height, int Epl);

/* 
 * Description:
 * Write a reference picture to a debug file
 *
 * Inputs:
 * pRef     - pointer to the reference picture
 * pName    - pointer to the file name to use
 * Append   - 0 = create a new empty file
 *            1 = append to existing file
 */

void vc1DEBUG_LogReferencePicture(vc1_sReferencePicture *pRef, char *pName, int Append);

/*
 * Description:
 * Display information on the current picture
 */

void vc1DEBUG_PrintPict(vc1_sPicture *pPict);

/*
 * Description:
 * Display the information in the current macroblock
 */

void vc1DEBUG_PrintMB(vc1_sPosition *pPos);

/* Declare debug macros */

#define DEBUG0(eZone,pFmt)       vc1DEBUG_Debug(eZone,pFmt)
#define DEBUG1(eZone,pFmt,p1)    vc1DEBUG_Debug(eZone,pFmt,p1)
#define DEBUG2(eZone,pFmt,p1,p2) vc1DEBUG_Debug(eZone,pFmt,p1,p2)
#define DEBUG3(eZone,pFmt,p1,p2,p3) \
                                 vc1DEBUG_Debug(eZone,pFmt,p1,p2,p3)
#define DEBUG4(eZone,pFmt,p1,p2,p3,p4) \
                                 vc1DEBUG_Debug(eZone,pFmt,p1,p2,p3,p4)
#define DEBUG5(eZone,pFmt,p1,p2,p3,p4,p5) \
                                 vc1DEBUG_Debug(eZone,pFmt,p1,p2,p3,p4,p5)
#define DEBUG6(eZone,pFmt,p1,p2,p3,p4,p5,p6) \
                                 vc1DEBUG_Debug(eZone,pFmt,p1,p2,p3,p4,p5,p6)
#define DEBUG7(eZone,pFmt,p1,p2,p3,p4,p5,p6,p7) \
                                 vc1DEBUG_Debug(eZone,pFmt,p1,p2,p3,p4,p5,p6, \
                                               p7)

#define ASSERT(x) if (!(x)) \
 { \
    vc1DEBUG_Fatal_Exit("Assert %s failed at line %d file %s\n", \
    #x, __LINE__, __FILE__); \
 }

#define DEBUGRECT8(eZone, pData, Width, Height, Epl) \
    if (vc1DEBUG_Zones & eZone) \
    { \
      vc1DEBUG_PrintRectangle8(pData, Width, Height, Epl); \
    }

#define DEBUGRECT16(eZone, pData, Width, Height, Epl) \
    if (vc1DEBUG_Zones & eZone) \
    { \
      vc1DEBUG_PrintRectangle16(pData, Width, Height, Epl); \
    }

#define DEBUGBLKS16(eZone, pData, Width, N) \
    if (vc1DEBUG_Zones & eZone) \
    { \
      vc1DEBUG_PrintBlocks16(pData, Width, N); \
    }

#define DEBUGREFPICT(eZone, pRef, pName) \
    if (vc1DEBUG_Zones & eZone) \
    { \
      vc1DEBUG_LogReferencePicture(pRef, pName, 1); \
    }

#define DEBUGNEWREFPICT(eZone, pRef, pName) \
    if (vc1DEBUG_Zones & eZone) \
    { \
      vc1DEBUG_LogReferencePicture(pRef, pName, 0); \
    }

#define DEBUGMB(eZone, pPos) \
    if (vc1DEBUG_Zones & eZone) \
    { \
      vc1DEBUG_PrintMB(pPos); \
    }

#define DEBUGPICT(eZone, pPict) \
    if (vc1DEBUG_Zones & eZone) \
    { \
      vc1DEBUG_PrintPict(pPict); \
    }

/* Macros to reduce the impact of coverage entries */
/* Mark coverage of an item depending on interlace mode */
#define COVERAGE_NONINT_INT(_n_,_i_) \
{ \
 static FLAG CoverageDone = FALSE; \
 if (CoverageDone == FALSE) \
 { \
  CoverageDone = TRUE; \
  if (pState->sSeqParams.Interlace == TRUE) \
  { \
    DEBUG0(vc1DEBUG_COVERAGE,_i_ "\n"); \
  } \
  else \
  { \
    DEBUG0(vc1DEBUG_COVERAGE,_n_ "\n"); \
  } \
 } \
}
/* Output coverage only once per run */
#define COVERAGE(_c_) \
{ \
 static FLAG CoverageDone = FALSE; \
 if (CoverageDone == FALSE) \
 { \
  CoverageDone = TRUE; \
  DEBUG0(vc1DEBUG_COVERAGE,_c_ "\n"); \
 } \
}

/* Debug tables */

extern const char *vc1DEBUG_PictureFormat[3];
extern const char *vc1DEBUG_PictureType[5];
extern const char *vc1DEBUG_BlkType[8];
extern const char *vc1DEBUG_SmallBlkType[8];
extern const char *vc1DEBUG_BlkNum[6];
extern const char *vc1DEBUG_BitPlaneMode[7];
extern const char *vc1DEBUG_Profile[4];
extern const char *vc1DEBUG_StartCode[0x20];
extern const char *vc1DEBUG_FieldName[4];
extern const char *vc1DEBUG_FieldPictureType[8];
extern const char *vc1DEBUG_MVRange[4];
extern const char *vc1DEBUG_MVMode[5];
extern const char *vc1DEBUG_TransType[8];
extern const char *vc1DEBUG_MBType[16];
extern const char *vc1DEBUG_BFraction[23];
extern const char *vc1DEBUG_SBPType[16];
extern const char *vc1DEBUG_QuantMode[12];
extern const char *vc1DEBUG_PictureRes[4];
extern const char *vc1DEBUG_MBModeIntField[8];
extern const char *vc1DEBUG_MBModeIntFrame[15];
extern const char *vc1DEBUG_DeBlkType[10];

#endif

#ifdef cplusplus
}
#endif

#endif
