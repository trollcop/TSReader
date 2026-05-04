/**
 *  (c) Copyright:  1995  EchoSphere Corp and Houston Tracker Systems
**
**  $Workfile:   huffman.h  $
**  $Revision:   1.4  $
**  $Modtime:   15 Sep 1995 08:08:08  $
**
**  Purpose:   Provide declarations and function prototypes for the Huffman
**             data decompression routines
**
**  Entry Points: (None)
**
 *  Compiler/Assembler: MetaWare High C, R3000 Family
 *  Ext Packages:       Nucleus Plus OS
**
 *  $Author:   BURRCOR  $
**
 *  Revisions:
 *     $Log:   S:/SOFTWARE/DBS/COMMON/OSD/HUFFMAN.H_V  $
 * 
**/

//rh #include "htsdef.h"

//rh//////////////////////////////////////////////
#ifndef USHORT
#define USHORT unsigned short
#endif USHORT
#ifndef UBYTE
#define UBYTE unsigned char
#endif UBYTE
#ifndef NULL
#define NULL 0
#endif NULL
//rh//////////////////////////////////////////////

#ifndef HDCODE_H
#define HDCODE_H

/*** Huffman Table Data Structures ********************************************/

typedef struct tagHUFFMANDAT  // holds actual Huffman Table data
{
  USHORT HufCode;  // bit string for each possible char
  UBYTE  HufByte;  // actual byte represented
  UBYTE  HufBits;  // #of bits in each byte code
} HUFFMANDAT;

typedef struct tagHUFFMANTAB  // general Huffman Parameters
{
  HUFFMANDAT *pHufDt;     // pointer to current Huffman Data struct
  USHORT       HufMode;   // (0=Huffman only), (1=RLE+Huffman)
  USHORT       HufTbSz;   // #of Huffman Table Data entries
} HUFFMANTAB;


/*** Private Function Prototypes **********************************************/
UBYTE GetHufByte( void );  // decode Huffman bit string
UBYTE GetRLEByte( void );  // decode RLE data blocks

/*** Public Function Prototypes ***********************************************/
USHORT HufDecmpKernel( void*, void*, USHORT, // Huffman Decompression Kernel
                       HUFFMANTAB* );
USHORT DcpString( void*, void*,  // NULL terminated string decompression
                  USHORT, HUFFMANTAB* );

/******************************************************************************/
#ifdef HNCODE   // define only if Huffman compression capability needed

typedef union tagBITBUILDER {
  unsigned long ldata;        // four byte data (together)
  unsigned char bdata[4];     // four byte data (separate)
} BITBUILDER;

USHORT PutHufByte( UBYTE ); // encode Huffman bit string
USHORT DmpHufBufr( void );  // flush Huffman encode data buffer
USHORT PutRLEByte( UBYTE ); // encode RLE data block
USHORT DmpRLEBufr( void );  // flush RLE encode data buffer

ULONG  HufBldFoot( void*, USHORT ); // build Huffman Freq Of Occurrance Table
USHORT HufBldHTab( HUFFMANTAB* );   // build Huffman Compression Table
USHORT HufCmpKernel( void*, void*,  // Huffman Compression Kernel
                     USHORT,HUFFMANTAB* );
USHORT CmpString( void*, void*,     // NULL terminated string compression
                  HUFFMANTAB* );


#endif  // HNCODE
/******************************************************************************/
HUFFMANTAB ExtHufTab;


#endif  // HDCODE_H
