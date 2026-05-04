/**
 *  (c) Copyright:  1995  EchoSphere Corp and Houston Tracker Systems
**
**  $Workfile:   huffman.c  $
**  $Revision:   1.21  $
**  $Modtime:   20 Oct 1998 16:19:08  $
**
**  Purpose:   Provide Huffman data (de)compression routines
**
**  Entry Points: GetHufByte      - get data byte from Huffman bit stream
**                GetRLEByte      - get data byte from RLE encoded blocks
**                HufDecmpKernel  - Huffman/RLE standard decompression routine
**                DcpString       - decompress null terminated string
**
**                PutHufByte      - put data byte into Huffman bit stream
**                DmpHufBufr      - flush Huffman bit stream buffer
**                PutRLEByte      - put data byte into RLE encoded blocks
**                DmpRLEBufr      - flush RLE encoded block buffer
**                HufBldFoot      - build Frequency Of Occurrance Table
**                HufBldHTab      - build Huffman Table from FOOT
**                HufCmpKernel    - Huffman/RLE standard compression routine
**                CmpString       - compress null terminated string
**
 *  Compiler/Assembler: MetaWare High C, R3000 Family
 *  Ext Packages:       Nucleus Plus OS
**
**/

#include "dn_huffman.h" //rh
//rh #include "config.h"

UBYTE  RLEBuff[128];  // RLE output buffer
USHORT RLERepeat;     // RLE byte counter
USHORT RLECount;      // RLE byte counter
USHORT BitsLStr;      // #of bits left in bit string
UBYTE  DBldr;         // bit string decoding area
UBYTE  *pHCode;       // pointer to decode source code
HUFFMANTAB *pHufTab;  // pointer to the current Huffman Table

/******************************************************************************/
#ifdef HNCODE   // define only if Huffman compression capability needed

USHORT IsBuilt = 1;   // Huffman table built flag
UBYTE  RLELast;       // last character analyzed
USHORT RLEMode;       // current RLE mode (0-Absolute, 1-Encoded)
UBYTE  HufBuff[4];    // Huffman buffer output area
UBYTE  BitsNStr;      // #of bits in bit string
BITBUILDER BBldr;     // bit string building area
ULONG  HEnCntr[256];  // #of chars counter (Huffman only)
ULONG  REnCntr[256];  // #of chars counter (Huffman+RLE)
ULONG  TotalBytes;    // #of chars passed through FOOT

//rh HUFFMANDAT FAR RLEHufDat[256];  // Huffman+RLE analysis data area
HUFFMANDAT RLEHufDat[256];  // Huffman+RLE analysis data area
HUFFMANTAB RLEHufTab = {        // Huffman+RLE analysis table
   RLEHufDat,
   0,
   0
};

#endif  // HNCODE
/******************************************************************************/

#ifdef EXPRESSVU
#include "huffm_ev.h"
#else
#ifndef EXTENDED_CHARACTER_SET
HUFFMANDAT DefHufDat[] = {    // Default Huffman Table Data
  {0x0000,  32,  3}, // char - (Space)
  {0x0002, 101,  4}, // char - e
  {0x0003, 116,  4}, // char - t
  {0x0004,  97,  4}, // char - a
  {0x0005, 111,  4}, // char - o
  {0x0006, 115,  4}, // char - s
  {0x0007, 110,  4}, // char - n
  {0x0020, 114,  6}, // char - r
  {0x0021, 105,  6}, // char - i
  {0x0022, 108,  6}, // char - l
  {0x0023,  99,  6}, // char - c
  {0x0024, 104,  6}, // char - h
  {0x0025, 117,  6}, // char - u
  {0x0026, 100,  6}, // char - d
  {0x0027, 112,  6}, // char - p
  {0x0028, 109,  6}, // char - m
  {0x0029, 103,  6}, // char - g
  {0x002A, 121,  6}, // char - y
  {0x002B, 118,  6}, // char - v
  {0x002C,  10,  6}, // char - (LF)
  {0x002D,  46,  6}, // char - .
  {0x002E, 119,  6}, // char - w
  {0x002F, 102,  6}, // char - f
  {0x0060,  83,  7}, // char - S
  {0x0061,  98,  7}, // char - b
  {0x0062,  84,  7}, // char - T
  {0x0063,  34,  7}, // char - "
  {0x0064, 107,  7}, // char - k
  {0x0065,  80,  7}, // char - P
  {0x0066,  65,  7}, // char - A
  {0x0067,  67,  7}, // char - C
  {0x0068,  68,  7}, // char - D
  {0x0069,  76,  7}, // char - L
  {0x006A,  77,  7}, // char - M
  {0x006B,  73,  7}, // char - I
  {0x006C,  78,  7}, // char - N
  {0x006D,  58,  7}, // char - :
  {0x006E,  82,  7}, // char - R
  {0x006F,  44,  7}, // char - ,
  {0x00E0,  69,  8}, // char - E
  {0x00E1,  85,  8}, // char - U
  {0x00E2,  70,  8}, // char - F
  {0x00E3,  72,  8}, // char - H
  {0x00E4,  89,  8}, // char - Y
  {0x00E5,  86,  8}, // char - V
  {0x00E6,  45,  8}, // char - -
  {0x00E7, 122,  8}, // char - z
  {0x00E8, 120,  8}, // char - x
  {0x00E9,  47,  8}, // char - /
  {0x00EA,  79,  8}, // char - O
  {0x00EB,  63,  8}, // char - ?
  {0x00EC,  87,  8}, // char - W
  {0x00ED,  71,  8}, // char - G
  {0x00EE,  66,  8}, // char - B
  {0x00EF,  51,  8}, // char - 3
  {0x01E0,  49,  9}, // char - 1
  {0x01E1, 113,  9}, // char - q
  {0x01E2,  48,  9}, // char - 0
  {0x01E3,  33,  9}, // char - !
  {0x01E4, 106,  9}, // char - j
  {0x01E5,  90,  9}, // char - Z
  {0x01E6,  57,  9}, // char - 9
  {0x01E7,  52,  9}, // char - 4
  {0x01E8,  75,  9}, // char - K
  {0x01E9,  42,  9}, /* char - * */
  {0x01EA,  55,  9}, // char - 7
  {0x01EB,  54,  9}, // char - 6
  {0x01EC,  53,  9}, // char - 5
  {0x01ED,  74,  9}, // char - J
  {0x01EE,  56,  9}, // char - 8
  {0x01EF,  41,  9}, // char - )
  {0x03E0,  40, 10}, // char - (
  {0x03E1,  88, 10}, // char - X
  {0x03E2,  81, 10}, // char - Q
  {0x03E3,  60, 10}, // char - <
  {0x03E4,  50, 10}, // char - 2
  {0x03E5,  39, 10}, // char - '
  {0x03E6,  38, 10}, // char - &
  {0x07CE, 127, 11}, // char - (DEL)
  {0x07CF, 126, 11}, // char - ~
  {0x07D0, 125, 11}, // char - }
  {0x07D1, 124, 11}, // char - |
  {0x07D2, 123, 11}, // char - {
  {0x07D3,  96, 11}, // char - `
  {0x07D4,  95, 11}, // char - _
  {0x07D5,  94, 11}, // char - ^
  {0x07D6,  93, 11}, // char - ]
  {0x07D7,  92, 11}, // char - (BackSlash)
  {0x07D8,  91, 11}, // char - [
  {0x07D9,  64, 11}, // char - @
  {0x07DA,  62, 11}, // char - >
  {0x07DB,  61, 11}, // char - =
  {0x07DC,  59, 11}, // char - ;
  {0x07DD,  43, 11}, // char - +
  {0x07DE,  37, 11}, // char - %
  {0x07DF,  36, 11}, // char - $
  {0x07E0,  35, 11}, // char - #
  {0x07E1,  31, 11}, // char - (US)
  {0x07E2,  30, 11}, // char - (RS)
  {0x07E3,  29, 11}, // char - (GS)
  {0x07E4,  28, 11}, // char - (FS)
  {0x07E5,  27, 11}, // char - (ESC)
  {0x07E6,  26, 11}, // char - (SUB)
  {0x07E7,  25, 11}, // char - (EM)
  {0x07E8,  24, 11}, // char - (CAN)
  {0x07E9,  23, 11}, // char - (ETB)
  {0x07EA,  22, 11}, // char - (SYN)
  {0x07EB,  21, 11}, // char - (NAK)
  {0x07EC,  20, 11}, // char - (DC4)
  {0x07ED,  19, 11}, // char - (DC3)
  {0x07EE,  18, 11}, // char - (DC2)
  {0x07EF,  17, 11}, // char - (DC1)
  {0x07F0,  16, 11}, // char - (DLE)
  {0x07F1,  15, 11}, // char - (SI)
  {0x07F2,  14, 11}, // char - (SO)
  {0x07F3,  13, 11}, // char - (CR)
  {0x07F4,  12, 11}, // char - (FF)
  {0x07F5,  11, 11}, // char - (VT)
  {0x07F6,   9, 11}, // char - (HT)
  {0x07F7,   8, 11}, // char - (BS)
  {0x07F8,   7, 11}, // char - (BEL)
  {0x07F9,   6, 11}, // char - (ACK)
  {0x07FA,   5, 11}, // char - (ENQ)
  {0x07FB,   4, 11}, // char - (EOT)
  {0x07FC,   3, 11}, // char - (ETX)
  {0x07FD,   2, 11}, // char - (STX)
  {0x07FE,   1, 11}, // char - (SOH)
  {0x07FF,   0, 11}  // char - (NUL)
};

HUFFMANTAB DefHufTab = {   // Default Huffman Table
   DefHufDat,  // pointer to default Huffman Table data
   0,          // Huffman Only Mode
   128         // 128 entries in Huffman data
};

HUFFMANDAT Table255[] = {
    0x0000, 0x20, 0x02,  // ' ' 
    0x0004, 0x65, 0x04,  // 'e' 
    0x0005, 0x72, 0x04,  // 'r' 
    0x0006, 0x6E, 0x04,  // 'n' 
    0x0007, 0x61, 0x04,  // 'a' 
    0x0010, 0x74, 0x05,  // 't' 
    0x0011, 0x6F, 0x05,  // 'o' 
    0x0012, 0x73, 0x05,  // 's' 
    0x0013, 0x69, 0x05,  // 'i' 
    0x0014, 0x6C, 0x05,  // 'l' 
    0x0015, 0x75, 0x05,  // 'u' 
    0x0016, 0x63, 0x05,  // 'c' 
    0x0017, 0x64, 0x05,  // 'd' 
    0x0060, 0x70, 0x07,  // 'p' 
    0x0061, 0x6D, 0x07,  // 'm' 
    0x0062, 0x76, 0x07,  // 'v' 
    0x0063, 0x67, 0x07,  // 'g' 
    0x0064, 0x68, 0x07,  // 'h' 
    0x0065, 0x2E, 0x07,  // '.' 
    0x0066, 0x66, 0x07,  // 'f' 
    0x0067, 0x0A, 0x07,  // '' 
    0x0068, 0x53, 0x07,  // 'S' 
    0x0069, 0x41, 0x07,  // 'A' 
    0x006A, 0x45, 0x07,  // 'E' 
    0x006B, 0x43, 0x07,  // 'C' 
    0x006C, 0x27, 0x07,  // ''' 
    0x006D, 0x7A, 0x07,  // 'z' 
    0x006E, 0x52, 0x07,  // 'R' 
    0x006F, 0x22, 0x07,  // '"' 
    0x00E0, 0x4C, 0x08,  // 'L' 
    0x00E1, 0x49, 0x08,  // 'I' 
    0x00E2, 0x4F, 0x08,  // 'O' 
    0x00E3, 0x62, 0x08,  // 'b' 
    0x00E4, 0x54, 0x08,  // 'T' 
    0x00E5, 0x4E, 0x08,  // 'N' 
    0x00E6, 0x55, 0x08,  // 'U' 
    0x00E7, 0x79, 0x08,  // 'y' 
    0x00E8, 0x44, 0x08,  // 'D' 
    0x00E9, 0x50, 0x08,  // 'P' 
    0x00EA, 0x71, 0x08,  // 'q' 
    0x00EB, 0x56, 0x08,  // 'V' 
    0x00EC, 0x2D, 0x08,  // '-' 
    0x00ED, 0x3A, 0x08,  // ':' 
    0x00EE, 0x2C, 0x08,  // ',' 
    0x00EF, 0x48, 0x08,  // 'H' 
    0x01E0, 0x4D, 0x09,  // 'M' 
    0x01E1, 0x78, 0x09,  // 'x' 
    0x01E2, 0x77, 0x09,  // 'w' 
    0x01E3, 0x42, 0x09,  // 'B' 
    0x01E4, 0x47, 0x09,  // 'G' 
    0x01E5, 0x46, 0x09,  // 'F' 
    0x01E6, 0x30, 0x09,  // '0' 
    0x01E7, 0x3F, 0x09,  // '?' 
    0x01E8, 0x33, 0x09,  // '3' 
    0x01E9, 0x2F, 0x09,  // '/' 
    0x01EA, 0x39, 0x09,  // '9' 
    0x01EB, 0x31, 0x09,  // '1' 
    0x01EC, 0x38, 0x09,  // '8' 
    0x01ED, 0x6B, 0x09,  // 'k' 
    0x01EE, 0x6A, 0x09,  // 'j' 
    0x01EF, 0x21, 0x09,  // '!' 
    0x03E0, 0x36, 0x0A,  // '6' 
    0x03E1, 0x35, 0x0A,  // '5' 
    0x03E2, 0x59, 0x0A,  // 'Y' 
    0x03E3, 0x51, 0x0A,  // 'Q' 
    0x07C8, 0x34, 0x0B,  // '4' 
    0x07C9, 0x58, 0x0B,  // 'X' 
    0x07CA, 0x32, 0x0B,  // '2' 
    0x07CB, 0x2B, 0x0B,  // '+' 
    0x07CC, 0x2A, 0x0B,  // '*' 
    0x07CD, 0x5A, 0x0B,  // 'Z' 
    0x07CE, 0x4A, 0x0B,  // 'J' 
    0x07CF, 0x29, 0x0B,  // ')' 
    0x0FA0, 0x28, 0x0C,  // '(' 
    0x0FA1, 0x23, 0x0C,  // '#' 
    0x0FA2, 0x57, 0x0C,  // 'W' 
    0x0FA3, 0x4B, 0x0C,  // 'K' 
    0x0FA4, 0x3C, 0x0C,  // '<' 
    0x0FA5, 0x37, 0x0C,  // '7' 
    0x0FA6, 0x7D, 0x0C,  // '}' 
    0x0FA7, 0x7B, 0x0C,  // '{' 
    0x0FA8, 0x60, 0x0C,  // '`' 
    0x0FA9, 0x26, 0x0C,  // '&' 
    0x1F54, 0xFE, 0x0D,  // 'þ' 
    0x1F55, 0xFD, 0x0D,  // 'ý' 
    0x1F56, 0xFC, 0x0D,  // 'ü' 
    0x1F57, 0xFB, 0x0D,  // 'û' 
    0x1F58, 0xFA, 0x0D,  // 'ú' 
    0x1F59, 0xF9, 0x0D,  // 'ù' 
    0x1F5A, 0xF8, 0x0D,  // 'ø' 
    0x1F5B, 0xF7, 0x0D,  // '÷' 
    0x1F5C, 0xF6, 0x0D,  // 'ö' 
    0x1F5D, 0xF5, 0x0D,  // 'õ' 
    0x1F5E, 0xF4, 0x0D,  // 'ô' 
    0x1F5F, 0xF3, 0x0D,  // 'ó' 
    0x1F60, 0xF2, 0x0D,  // 'ò' 
    0x1F61, 0xF1, 0x0D,  // 'ñ' 
    0x1F62, 0xF0, 0x0D,  // 'ð' 
    0x1F63, 0xEF, 0x0D,  // 'ï' 
    0x1F64, 0xEE, 0x0D,  // 'î' 
    0x1F65, 0xED, 0x0D,  // 'í' 
    0x1F66, 0xEC, 0x0D,  // 'ì' 
    0x1F67, 0xEB, 0x0D,  // 'ë' 
    0x1F68, 0xEA, 0x0D,  // 'ê' 
    0x1F69, 0xE9, 0x0D,  // 'é' 
    0x1F6A, 0xE8, 0x0D,  // 'è' 
    0x1F6B, 0xE7, 0x0D,  // 'ç' 
    0x1F6C, 0xE6, 0x0D,  // 'æ' 
    0x1F6D, 0xE5, 0x0D,  // 'å' 
    0x1F6E, 0xE4, 0x0D,  // 'ä' 
    0x1F6F, 0xE3, 0x0D,  // 'ã' 
    0x1F70, 0xE2, 0x0D,  // 'â' 
    0x1F71, 0xE1, 0x0D,  // 'á' 
    0x1F72, 0xE0, 0x0D,  // 'à' 
    0x1F73, 0xDF, 0x0D,  // 'ß' 
    0x1F74, 0xDE, 0x0D,  // 'Þ' 
    0x1F75, 0xDD, 0x0D,  // 'Ý' 
    0x1F76, 0xDC, 0x0D,  // 'Ü' 
    0x1F77, 0xDB, 0x0D,  // 'Û' 
    0x1F78, 0xDA, 0x0D,  // 'Ú' 
    0x1F79, 0xD9, 0x0D,  // 'Ù' 
    0x1F7A, 0xD8, 0x0D,  // 'Ø' 
    0x1F7B, 0xD7, 0x0D,  // '×' 
    0x1F7C, 0xD6, 0x0D,  // 'Ö' 
    0x1F7D, 0xD5, 0x0D,  // 'Õ' 
    0x1F7E, 0xD4, 0x0D,  // 'Ô' 
    0x1F7F, 0xD3, 0x0D,  // 'Ó' 
    0x1F80, 0xD2, 0x0D,  // 'Ò' 
    0x1F81, 0xD1, 0x0D,  // '' 
    0x1F82, 0xD0, 0x0D,  // '' 
    0x1F83, 0xCF, 0x0D,  // '' 
    0x1F84, 0xCE, 0x0D,  // '' 
    0x1F85, 0xCD, 0x0D,  // '' 
    0x1F86, 0xCC, 0x0D,  // '' 
    0x1F87, 0xCB, 0x0D,  // '' 
    0x1F88, 0xCA, 0x0D,  // '' 
    0x1F89, 0xC9, 0x0D,  // '' 
    0x1F8A, 0xC8, 0x0D,  // '' 
    0x1F8B, 0xC7, 0x0D,  // '' 
    0x1F8C, 0xC6, 0x0D,  // '' 
    0x1F8D, 0xC5, 0x0D,  // '' 
    0x1F8E, 0xC4, 0x0D,  // '' 
    0x1F8F, 0xC3, 0x0D,  // '' 
    0x1F90, 0xC2, 0x0D,  // '' 
    0x1F91, 0xC1, 0x0D,  // '' 
    0x1F92, 0xC0, 0x0D,  // '' 
    0x1F93, 0xBF, 0x0D,  // '' 
    0x1F94, 0xBE, 0x0D,  // '' 
    0x1F95, 0xBD, 0x0D,  // '' 
    0x1F96, 0xBC, 0x0D,  // '' 
    0x1F97, 0xBB, 0x0D,  // '' 
    0x1F98, 0xBA, 0x0D,  // '' 
    0x1F99, 0xB9, 0x0D,  // '' 
    0x1F9A, 0xB8, 0x0D,  // '' 
    0x1F9B, 0xB7, 0x0D,  // '' 
    0x1F9C, 0xB6, 0x0D,  // '' 
    0x1F9D, 0xB5, 0x0D,  // '' 
    0x1F9E, 0xB4, 0x0D,  // '' 
    0x1F9F, 0xB3, 0x0D,  // '' 
    0x1FA0, 0xB2, 0x0D,  // '' 
    0x1FA1, 0xB1, 0x0D,  // '' 
    0x1FA2, 0xB0, 0x0D,  // '' 
    0x1FA3, 0xAF, 0x0D,  // '' 
    0x1FA4, 0xAE, 0x0D,  // '' 
    0x1FA5, 0xAD, 0x0D,  // '' 
    0x1FA6, 0xAC, 0x0D,  // '' 
    0x1FA7, 0xAB, 0x0D,  // '' 
    0x1FA8, 0xAA, 0x0D,  // '' 
    0x1FA9, 0xA9, 0x0D,  // '' 
    0x1FAA, 0xA8, 0x0D,  // '' 
    0x1FAB, 0xA7, 0x0D,  // '' 
    0x1FAC, 0xA6, 0x0D,  // '' 
    0x1FAD, 0xA5, 0x0D,  // '' 
    0x1FAE, 0xA4, 0x0D,  // '' 
    0x1FAF, 0xA3, 0x0D,  // '' 
    0x1FB0, 0xA2, 0x0D,  // '' 
    0x1FB1, 0xA1, 0x0D,  // '' 
    0x1FB2, 0xA0, 0x0D,  // '' 
    0x1FB3, 0x9F, 0x0D,  // '' 
    0x1FB4, 0x9E, 0x0D,  // '' 
    0x1FB5, 0x9D, 0x0D,  // '' 
    0x1FB6, 0x9C, 0x0D,  // '' 
    0x1FB7, 0x9B, 0x0D,  // '' 
    0x1FB8, 0x9A, 0x0D,  // '' 
    0x1FB9, 0x99, 0x0D,  // '' 
    0x1FBA, 0x98, 0x0D,  // '' 
    0x1FBB, 0x97, 0x0D,  // '' 
    0x1FBC, 0x96, 0x0D,  // '' 
    0x1FBD, 0x95, 0x0D,  // '' 
    0x1FBE, 0x94, 0x0D,  // '' 
    0x1FBF, 0x93, 0x0D,  // '' 
    0x1FC0, 0x92, 0x0D,  // '' 
    0x1FC1, 0x91, 0x0D,  // '' 
    0x1FC2, 0x90, 0x0D,  // '' 
    0x1FC3, 0x8F, 0x0D,  // '' 
    0x1FC4, 0x8E, 0x0D,  // '' 
    0x1FC5, 0x8D, 0x0D,  // '' 
    0x1FC6, 0x8C, 0x0D,  // '' 
    0x1FC7, 0x8B, 0x0D,  // '' 
    0x1FC8, 0x8A, 0x0D,  // '' 
    0x1FC9, 0x89, 0x0D,  // '' 
    0x1FCA, 0x88, 0x0D,  // '' 
    0x1FCB, 0x87, 0x0D,  // '' 
    0x1FCC, 0x86, 0x0D,  // '' 
    0x1FCD, 0x85, 0x0D,  // '' 
    0x1FCE, 0x84, 0x0D,  // '' 
    0x1FCF, 0x83, 0x0D,  // '' 
    0x1FD0, 0x82, 0x0D,  // '' 
    0x1FD1, 0x81, 0x0D,  // '' 
    0x1FD2, 0x80, 0x0D,  // '' 
    0x1FD3, 0x7F, 0x0D,  // '' 
    0x1FD4, 0x7E, 0x0D,  // '' 
    0x1FD5, 0x7C, 0x0D,  // '' 
    0x1FD6, 0x5F, 0x0D,  // '' 
    0x1FD7, 0x5E, 0x0D,  // '' 
    0x1FD8, 0x5D, 0x0D,  // '' 
    0x1FD9, 0x5C, 0x0D,  // '' 
    0x1FDA, 0x5B, 0x0D,  // '' 
    0x1FDB, 0x40, 0x0D,  // '' 
    0x1FDC, 0x3E, 0x0D,  // '' 
    0x1FDD, 0x3D, 0x0D,  // '' 
    0x1FDE, 0x3B, 0x0D,  // '' 
    0x1FDF, 0x25, 0x0D,  // '' 
    0x1FE0, 0x24, 0x0D,  // '' 
    0x1FE1, 0x1F, 0x0D,  // '' 
    0x1FE2, 0x1E, 0x0D,  // '' 
    0x1FE3, 0x1D, 0x0D,  // '' 
    0x1FE4, 0x1C, 0x0D,  // '' 
    0x1FE5, 0x1B, 0x0D,  // '' 
    0x1FE6, 0x1A, 0x0D,  // '' 
    0x1FE7, 0x19, 0x0D,  // '' 
    0x1FE8, 0x18, 0x0D,  // '' 
    0x1FE9, 0x17, 0x0D,  // '' 
    0x1FEA, 0x16, 0x0D,  // '' 
    0x1FEB, 0x15, 0x0D,  // '' 
    0x1FEC, 0x14, 0x0D,  // '' 
    0x1FED, 0x13, 0x0D,  // '' 
    0x1FEE, 0x12, 0x0D,  // '' 
    0x1FEF, 0x11, 0x0D,  // '' 
    0x1FF0, 0x10, 0x0D,  // '' 
    0x1FF1, 0x0F, 0x0D,  // '' 
    0x1FF2, 0x0E, 0x0D,  // '' 
    0x1FF3, 0x0D, 0x0D,  // '' 
    0x1FF4, 0x0C, 0x0D,  // '' 
    0x1FF5, 0x0B, 0x0D,  // '' 
    0x1FF6, 0x09, 0x0D,  // '' 
    0x1FF7, 0x08, 0x0D,  // '' 
    0x1FF8, 0x07, 0x0D,  // '' 
    0x1FF9, 0x06, 0x0D,  // '' 
    0x1FFA, 0x05, 0x0D,  // '' 
    0x1FFB, 0x04, 0x0D,  // '' 
    0x1FFC, 0x03, 0x0D,  // '' 
    0x1FFD, 0x02, 0x0D,  // '' 
    0x1FFE, 0x01, 0x0D,  // '' 
    0x1FFF, 0x00, 0x0D   // '' 
};

HUFFMANTAB ExtHufTab = {   // Default Huffman Table
   Table255,  // pointer to default Huffman Table data
   0,          // Huffman Only Mode
   255         // 128 entries in Huffman data
};

#else	// use EXTENDED_CHARACTER_SET
#include "huffman.txt"
#endif // EXTENDED_CHARACTER_SET
#endif // EXPRESSVU
/******************************************************************************/

/**
 **  Name:        GetHufByte
 **
 **  Description: This routine retrieves one byte of decompressed data from
 **               the compressed data stream.  Incoming Huffman bit strings are
 **               found in the Huffman table and are then replaced by their
 **               original ASCII code.
 **
 **  Arguments:   None
 **
 **  Returns:     One byte of decompressed data
 **
 **  Comments:    This routine is called by the "HufDecmpKernel"
 **               procedure and should not be called by high level routines.
 **               Huffman encoded bit strings are decoded into their original
 **               ASCII codes.  To function correctly, the Huffman table of
 **               values must be in place as mentioned in the header file.
 **/

UBYTE GetHufByte( void )
{
  USHORT i;                 // local loop index counter
  USHORT HCode;             // Huffman bit string being extracted
  UBYTE  j;                 // local loop index counter
  UBYTE  DBitsNStr;         // #of bits in code
  UBYTE  BitsDiff;          // #of bits needed for next search
  const HUFFMANDAT *pHDat;  // pointer to current Huffman data

  HCode = 0;                // start with no bits in Huffman code
  DBitsNStr = 0;            // indicate no bits decoded
  pHDat = pHufTab->pHufDt;  // get local pointer to Huffman data

  for( i=0; i<pHufTab->HufTbSz; i++ ) // loop thu table to find incoming byte
  {
    BitsDiff = pHDat->HufBits - DBitsNStr;
    for( j=0; j<BitsDiff; j++ ) // get BitsDiff #of bits from compressed data
    {
      if ( !BitsLStr )        // make sure bits are available to extract
      {
        DBldr = *pHCode++;    // get more bits from source...
        BitsLStr += 8;        // ...and record that fact
      }
      HCode <<= 1;            // make room for incoming bit
      DBitsNStr++;            // indicate new incoming bit (0 or 1)
      if ( DBldr & 0x80 )
         HCode++; // if incoming bit is 1, put it in code
      DBldr <<= 1;            // remove extracted bit from source
      BitsLStr--;             // indicate extracted bit no longer in source
    }
    if ( HCode == pHDat->HufCode )
      return( pHDat->HufByte ); // match found
    pHDat++; // else goto next table entry
  }
  return( 0 );  // if falls through here, error in source data
}


/**
 **  Name:        GetRLEByte
 **
 **  Description: This routine retrieves one byte of decompressed data from
 **               the compressed data stream.  RLE encoded data blacks are
 **               replaced by their original ASCII codes.
 **
 **  Arguments:   None
 **
 **  Returns:     One byte of decompressed data
 **
 **  Comments:    This routine is called by the "HufDecmpKernel"
 **               procedure and should not be called by high level routines.
 **               RLE encoded data blocks are decoded into their original
 **               ASCII codes.
 **/

UBYTE GetRLEByte( void )
{
  USHORT i;       // local loop index counter
  UBYTE  CharTmp; // local one byte value

  if ( RLECount < RLERepeat ) // return any remaining buffered bytes
    return( RLEBuff[RLECount++] );

  RLECount = 0;  // no buffered bytes
  RLERepeat = (USHORT) GetHufByte(); // get RLE block type

  if ( RLERepeat & 0x80 )    // if Encoded (repeating) block
  {
    RLERepeat &= 0x7f;       // isolate repeat count
    CharTmp = GetHufByte(); // get repeating byte and...
    for( i=0; i<RLERepeat; i++ )
      RLEBuff[i] = CharTmp;  // ...put it buffer
  }
  else    // if Absolute (non-repeating) block...
  {
    for( i=0; i<RLERepeat; i++ )
      RLEBuff[i] = GetHufByte(); // ...get bytes
  }
  return( RLEBuff[RLECount++] );  // return one RLE byte
}


/**
 **  Name:        HufDecmpKernel
 **
 **  Description: This routine acts as the common Huffman decompression kernel
 **               for all public decompression routines.
 **
 **  Arguments:   pOutp  - a pointer to resulting uncompressed data
 **               pByte  - a pointer to source compressed data
 **               nbytes - #of bytes to decompress into pTarget
 **               pTable - a pointer to the Huffman Table to use
 **                 *NOTE* if this value is NULL, the internal default table
 **                        is used
 **
 **  Returns:     Actual number of decompressed bytes placed in pOutp
 **
 **  Comments:    This routine is called by the public decompression functions
 **               to actually run the decompression code.
 **/

USHORT HufDecmpKernel( void* pOutp, void* pByte, USHORT nbytes,
                     HUFFMANTAB* pTable )
{
  USHORT i;          // loop index
  UBYTE *pMemory;    // local output memory pointer

  pMemory = (UBYTE*) pOutp;   // get local copy of memory pointer
  pHCode = (UBYTE*) pByte;    // get pointer to source data

  if ( pTable == 0 )          // get the current Huffman Table
    pHufTab = &DefHufTab;
  else
    pHufTab = pTable;

  BitsLStr = 0;               // no bits in Huffman buffer
  RLECount = RLERepeat = 0;   // no bytes in RLE buffer

  if ( pHufTab->HufMode )     // if RLE+Huffman mode
  {
    for( i=0; i<nbytes; i++ )
    {
      *pMemory++ = GetRLEByte();
    }
  }
  else                                // if Huffman only mode
  {
    for( i=0; i<nbytes; i++ )
    {
      *pMemory++ = GetHufByte();
    }
  }
  return( nbytes );
}


/**
 **  Name:        DcpString
 **
 **  Description: This routine decompresses null terminated strings and is
 **               the direct inverse of the compression routine "CmpString".
 **
 **  Arguments:   pTarget - a pointer to resulting decompressed character string
 **               pData   - a pointer to source compressed data
 **               nbytes  - max byte size of pTarget
 **               pTable  - a pointer to the Huffman Table to use
 **                 *NOTE* if this value is NULL, the internal default table
 **                        is used
 **
 **  Returns:     Actual number of decompressed bytes placed in pTarget
 **               not including the string's NULL terminator byte
 **
 **  Comments:    Call this routine to decompress character strings compressed
 **               with the "CmpString" routine.  This routine is restricted
 **               to strings that are LESS THAN 8192 bytes in length (including
 **               the NULL terminator byte).  If an attempt is made to use
 **               strings >8191 bytes, they will be forced to 8192 bytes.
 **
 **               This routine will function with both compressed and non-
 **               compressed strings.  The first and/or second byte of
 **               compressed strings have their high bit turned on to identify
 **               them as compressed.  Strings without the high bit on
 **               (non-compressed) will cause a simple string copy to occur.
 **
 **/

USHORT DcpString( void* pTarget, void* pData,
                  USHORT nbytes, HUFFMANTAB* pTable )
{
  UBYTE  chr;     // test character
  UBYTE* pOutp;   // byte pointer created from pTarget
  UBYTE* pByte;   // byte pointer created from pData
  USHORT nChars;  // #of bytes of uncompressed data

  pByte = (UBYTE*) pData;    // force pointer to be byte-like
  pOutp = (UBYTE*) pTarget;  // force pointer to be byte-like

  if (*pByte == NULL)     /* handle uncompressed null string*/
  {
     *pOutp = NULL;
     return( 0 );
  }
#ifndef EXTENDED_CHARACTER_SET
  if ( (*pByte < 128) && (*(pByte+1) < 128) ) // copy string if not compressed
#else
  if(*pByte != 0x1)
#endif
  {
     nChars = 0;
     do
     {
       *(pOutp + nChars++) = chr = *pByte++;
       if ( nChars >= nbytes )
       {
         *(pOutp + (nbytes-1)) = 0;  // This line moved by rev 1.15
         break;
       }
     } while( chr );
  }
  else
  {
#ifdef EXTENDED_CHARACTER_SET
  pByte++;				// point past the appended 0x1 byte
#endif
    if ( *pByte < 128 ) // handle long strings (2 byte counter)
    {
      nChars  = (USHORT)(*pByte++ & 0x3f);  // get #of bytes of uncompressed data
      nChars |= (USHORT)(*pByte++ & 0x7f) << 6;
    }
    else                // handle short string (1 byte counter)
    {
      nChars = (USHORT)(*pByte++ & 0x7f); // get #of bytes of uncompressed data
    }
    if ( nChars >= nbytes )
      nChars = nbytes - 1;
#ifdef EXTENDED_CHARACTER_SET
// #ifndef BONES_EV  CR#1074 - 1000C now uses Genbravo.exe to produce strings
    nChars--;
// #endif
// Note:  The BONES_EV switch around this line is a temporary fix for
// bones express vu.  Floyd added this line for other express vu products
// because the string builder tools used for the baker express vu products
// appended a trailing byte to the compressed strings.  The correct fix will
// be to correct the string builder tools so they do not append any unused
// bytes.  Then this line "nChars--;" can be removed. SFM.
#endif
    HufDecmpKernel( pOutp, pByte, nChars, pTable );  // decompress the string
  }
  *(pOutp + nChars) = 0;
  return( nChars );
}


/******************************************************************************/
#ifdef HNCODE   // define only if Huffman compression capability needed

/**
 **  Name:        PutHufByte
 **
 **  Description: This routine creates Huffman encoded bit strings from
 **               incoming source data using a pre-built Huffman Table.
 **
 **  Arguments:   bval - a single incoming source data byte to be added to the
 **                      Huffman encoded bit string being created
 **
 **  Returns:     a zero value - no completed Huffman data ready yet
 **               non-zero val - number of bytes ready in Huffman buffer
 **
 **  Comments:    Call this routine to create compressed Huffman data.  If
 **               a non-zero value is returned, a finished bit string is
 **               ready in global array HufBuff and MUST be copied out before
 **               the next call to this routine.
 **/

USHORT PutHufByte( UBYTE bval )
{
  USHORT i;         // loop index counter
  USHORT count;     // byte counter for output
  ULONG  CodeWord;  // Huffman encoded bit string being built
  UBYTE  BitShift;  // local bit shift holder
  const HUFFMANDAT *pHDat; // pointer to current Huffman data

  count = 0;
  pHDat = pHufTab->pHufDt;  // get pointer to Huffman data
  for( i=0; i<pHufTab->HufTbSz; i++ )
  {
    if ( bval == (pHDat+i)->HufByte )        // find bval byte in Huffman Table
    {
      CodeWord = (ULONG)(pHDat+i)->HufCode;
      BitShift = 32 - (pHDat+i)->HufBits - BitsNStr; // compute bit shift
      BBldr.ldata |= (CodeWord << BitShift);  // add new bits to stream
      BitsNStr += (pHDat+i)->HufBits;          // update bit counter
      while( BitsNStr > 7 )                 // if finished bytes are ready...
      {
        HufBuff[count++] = BBldr.bdata[3];  // ...place them in Huffman buffer
        BBldr.ldata <<= 8;                  // shift finished data out
        BitsNStr -= 8;                      // update bit counter
      }
      break;
    }
  }
  return( count );
}

/**
 **  Name:        DmpHufBufr
 **
 **  Description: This routine flushes any Huffman encoded bit strings from
 **               previous calls to "PutHufByte" and should be called after
 **               the last source data byte has been entered in "PutHufByte".
 **
 **  Arguments:   None
 **
 **  Returns:     a zero value - no completed Huffman data ready
 **               non-zero val - number of bytes ready in Huffman buffer
 **
 **  Comments:    Call this routine to flush compressed Huffman data from
 **               the Huffman data buffer.  If a non-zero value is returned, a
 **               finished Huffman data is ready in global array HufBuff.
 **/

USHORT DmpHufBufr( void )
{
  USHORT count;   // byte counter for output

  count = 0;
  if ( BitsNStr > 0 )
    HufBuff[count++] = BBldr.bdata[3];
  BBldr.ldata = 0;
  BitsNStr = 0;
  return( count );
}

/**
 **  Name:        PutRLEByte
 **
 **  Description: This routine creates RLE data blocks from incoming source
 **               data.  These blocks can be of two types: Encoded or Absolute.
 **               Encoded blocks describe repeating source data bytes.
 **               Absolute blocks describe strings of non-repeating bytes.
 **
 **  Arguments:   bval - a single incoming source data byte to be added to the
 **                      RLE block being created
 **
 **  Returns:     a zero value - no completed RLE block ready yet
 **               non-zero val - number of bytes in the finished RLE block
 **
 **  Comments:    Call this routine to create compressed RLE data blocks.  If
 **               a non-zero value is returned, a finished RLE data block is
 **               ready in global array RLEBuff and MUST be copied out before
 **               the next call to this routine.  RLE data blocks exist in two
 **               forms as mentioned above.  Their internals are shown below:
 **
 **               Encoded Block (2bytes long) - represents repeating data
 **               [1|count=3-127] [repeating byte]
 **
 **               Absolute Block (128bytes max) - for non-repeating data
 **               [0|count=0-127] [byte#1] [byte#2]...[byte#n]
 **/

USHORT PutRLEByte( UBYTE bval )
{
  USHORT count;       // finished byte count

  count = 0;
  if ( RLECount == 0 )        // start new block (type? - default to Absolute)
  {
    RLERepeat = RLECount = 1;
    RLEBuff[RLECount] = RLELast = bval;
  }
  else
  {
    if ( RLECount == 1 )      // use 2nd byte to determine type
    {
      RLEBuff[RLECount] = RLELast;
      if ( bval == RLELast )
      {
        RLEMode = 1;
        RLERepeat = 2;
        RLEBuff[++RLECount] = RLELast = bval;
      }
      else
      {
        RLEMode = 0;
        RLEBuff[++RLECount] = RLELast = bval;
      }
    }
    else
    {
      if ( RLEMode == 1 )
      {
        if ( bval == RLELast )    // continue with Encoded block
        {
          if ( RLERepeat < 127 )
          {
            RLERepeat++;
            RLEBuff[++RLECount] = RLELast = bval;
          }
          else
          {
            RLERepeat |= 0x80;    // output Encoded block
            RLEBuff[0] = (UBYTE) RLERepeat;
            RLEBuff[1] = RLELast;
            count = 2;
            RLELast = bval;
            RLERepeat = RLECount = 1;
            RLEMode = 0;
          }
        }
        else
        {
          RLERepeat |= 0x80;      // output Encoded block
          RLEBuff[0] = (UBYTE) RLERepeat;
          RLEBuff[1] = RLELast;
          count = 2;
          RLELast = bval;
          RLERepeat = RLECount = 1;
          RLEMode = 0;
        }
      }
      else
      {
        if ( bval != RLELast )    // continue with Absolute block
        {
          RLERepeat = 1;
          if ( RLECount < 127 )
          {
            RLEBuff[++RLECount] = RLELast = bval;
          }
          else                    // output Absolute block
          {
            RLEBuff[0] = (UBYTE) (count = RLECount);
            count++;
            RLELast = bval;
            RLERepeat = RLECount = 1;
            RLEMode = 0;
          }
        }
        else
        {
          RLERepeat++;
          if ( RLERepeat > 2 )    // switch to Encoded block
          {
            RLECount -= 2;
            RLEBuff[0] = (UBYTE) (count = RLECount);
            count++;
            RLEMode = 1;
            RLELast = bval;
            RLECount = RLERepeat;
          }
          else
          {
            if ( RLECount < 127 )
            {
              RLEBuff[++RLECount] = RLELast = bval;
            }
            else
            {
              RLEBuff[0] = (UBYTE) (count = RLECount);
              count++;
              RLELast = bval;
              RLERepeat = RLECount = 1;
              RLEMode = 0;
            }
          }
        }
      }
    }
  }
  return( count );
}

/**
 **  Name:        DmpRLEBufr
 **
 **  Description: This routine flushes any RLE data blocks remaining from
 **               previous calls to "PutRLEByte" and should be called after
 **               the last source data byte has been entered in "PutRLEByte".
 **
 **  Arguments:   None
 **
 **  Returns:     a zero value - no completed RLE block in buffer
 **               non-zero val - number of bytes in the finished RLE block
 **
 **  Comments:    Call this routine to flush compressed RLE data blocks from
 **               the RLE data buffer.  If a non-zero value is returned, a
 **               finished RLE data block is ready in global array RLEBuff.
 **/

USHORT DmpRLEBufr( void )
{
  USHORT count;       // finished byte count

  if ( RLEMode == 0 )
  {
    RLEBuff[0] = (UBYTE) (count = RLECount);
    count++;
  }
  else
  {
    RLERepeat |= 0x80;
    RLEBuff[0] = (UBYTE) RLERepeat;
    RLEBuff[1] = RLELast;
    count = 2;
  }
  RLEMode = 0;
  RLECount = 0;
  RLERepeat = 0;
  return( count );
}

/**
 **  Name:        HufBldFoot
 **
 **  Description: This routine builds a table of counters called a FOOT
 **               (Frequency Of Occurrance Table).  There are 256 counters;
 **               each one associated with one of 256 possible byte values.
 **               As data passes through this routine, the counters keep
 **               track of how many times each 8-bit byte appeared.
 **
 **  Arguments:   pData - pointer to the source data (any data type)
 **               nbytes - #of bytes of source data to analyze
 **
 **  Returns:     Total number of bytes analyzed so far
 **
 **  Comments:    Call this routine several times, if necessary, to analyze
 **               all source data.  The FOOT will continue to be built with
 **               each call.  A call to the "HufBldHTab" routine causes FOOT
 **               building to terminate; a flag is set so that the next call
 **               to this routine starts building a new FOOT.
 **/

ULONG HufBldFoot( void* pData, USHORT nbytes )
{
  USHORT i, j, k; // loop index counters
  UBYTE* pByte;   // byte pointer created from pData

  pByte = (UBYTE*) pData; // force pointer to be byte like

  if ( IsBuilt )          // delete previous FOOT/Huffman if necessary
  {
    for( i=0; i<256; i++ )  // initialize all counters to zero
    {
      HEnCntr[i] = 0;
      REnCntr[i] = 0;
    }
    TotalBytes = 0; // no bytes entered yet
    IsBuilt = 0;    // flag tables as not built
    DmpRLEBufr();   // flush/initialize RLE data buffer
    DmpHufBufr();   // flush/initialize Huffman data buffer
  }
  TotalBytes += (ULONG) nbytes; // keep running byte count
  for( i=0; i<nbytes; i++ )     // loop for all bytes
  {
    HEnCntr[*pByte]++;          // build FOOT counters
    j = PutRLEByte( *pByte++ ); // build 2nd FOOT of resulting RLE data
    if ( j )
      for( k=0; k<j; k++ )
         REnCntr[RLEBuff[k]]++;
  }
  return( TotalBytes );
}

/**
 **  Name:        HufBldHTab
 **
 **  Description: This routine builds a Huffman Table of encoded bit strings.
 **               The strings (and their length) are a function of the values
 **               placed in the FOOT by routine "HufBldFoot".  Each encoded bit
 **               string acts as a direct replacement for source data bytes
 **               in the output compressed data.
 **
 **  Arguments:   pUserTab - a pointer to a user supplied Huffman table
 **                          structure that will receive the newly built table
 **
 **  Returns:     The number of Huffman Table entries created
 **
 **  Comments:    Call this routine after all source data has been passed
 **               through the "HufBldFoot" routine.  The resulting Huffman Table
 **               of information is necessary for both encoding source data
 **               during compression and decoding during data decompression.
 **               Calling this routine resets the FOOT so that another FOOT
 **               can be created later with new source data.
 **/

USHORT HufBldHTab( HUFFMANTAB* pUserTab )
{
  USHORT i, j;          // loop index counters
  USHORT again;         // quick-out sort flag
  USHORT tpos;          // current table position
  USHORT tabl_end;      // current end-of-table marker
  USHORT stkptr;        // local stack pointer
  USHORT stack_pos[32]; // local stack of table positions
  ULONG  stack_frq[32]; // local stack of table occurrance frequencies
  ULONG  LongTmp;       // scratch unsigned long value
  ULONG  nbits1, nbits2;// bit counters to establish best cmp method
  HUFFMANDAT HufTmpDat; // scratch Huffman data
  HUFFMANDAT *pHDat;    // pointer to current Huffman data
  HUFFMANDAT *pRDat;    // pointer to current Huffman+RLE data

  i = DmpRLEBufr();     // flush any remaining RLE FOOT bytes
  if ( i )
      for( j=0; j<i; j++ )
         REnCntr[RLEBuff[j]]++;

  pHDat = pUserTab->pHufDt;   // get pointer to Huffman data
  pRDat = RLEHufTab.pHufDt;   // get pointer to Huffman+RLE data
  if ( !IsBuilt )
  {
    IsBuilt = 1;
    pUserTab->HufTbSz = RLEHufTab.HufTbSz = 0;  // initialize table data to zero
    for( i=0; i<256; i++ )
    {
      (pHDat+i)->HufCode = (pRDat+i)->HufCode = 0;
      (pHDat+i)->HufByte = (pRDat+i)->HufByte = (UBYTE) i;
      (pHDat+i)->HufBits = (pRDat+i)->HufBits = 0;
    }
    for( i=0; i<255; i++ )    // sort frequency table (ascending)
    {
      again = 0;
      for( j=0; j<255; j++ )
      {
        if ( HEnCntr[j] > HEnCntr[j+1] )
        {
          again = 1;
          LongTmp = HEnCntr[j];
          HEnCntr[j] = HEnCntr[j+1];
          HEnCntr[j+1] = LongTmp;
          HufTmpDat = *(pHDat+j);
          *(pHDat+j) = *(pHDat+(j+1));
          *(pHDat+(j+1)) = HufTmpDat;
        }
      }
      if ( !again )
         break;
    }
    for( i=0; i<255; i++ )    // sort RLE frequency table (ascending)
    {
      again = 0;
      for( j=0; j<255; j++ )
      {
        if ( REnCntr[j] > REnCntr[j+1] )
        {
          again = 1;
          LongTmp = REnCntr[j];
          REnCntr[j] = REnCntr[j+1];
          REnCntr[j+1] = LongTmp;
          HufTmpDat = *(pRDat+j);
          *(pRDat+j) = *(pRDat+(j+1));
          *(pRDat+(j+1)) = HufTmpDat;
        }
      }
      if ( !again )
         break;
    }
  }
  for( i=0; i<256; i++ )      // eliminate unused table entries
  {
    if ( HEnCntr[i] ) {
      *(pHDat+pUserTab->HufTbSz) = *(pHDat+i);
      HEnCntr[pUserTab->HufTbSz++] = HEnCntr[i];
    }
    if ( REnCntr[i] ) {
      *(pRDat+RLEHufTab.HufTbSz) = *(pRDat+i);
      REnCntr[RLEHufTab.HufTbSz++] = REnCntr[i];
    }
  }

  stkptr = 0;                 // build Huffman code
  stack_pos[0] = 0;
  stack_frq[0] = HEnCntr[0];
  tabl_end = pUserTab->HufTbSz - 1;

  for( tpos=1; tpos<=tabl_end; tpos++ )
  {
    if ((HEnCntr[tpos] >= stack_frq[stkptr]) || (tpos == tabl_end))
    {
      for( i=stack_pos[stkptr]; i<tpos; i++ )
        (pHDat+i)->HufCode |= (1 << (pHDat+i)->HufBits++);
      (pHDat+tpos)->HufBits++;
      stack_frq[stkptr] += HEnCntr[tpos];
    }
    else
    {
      stkptr++;
      stack_pos[stkptr] = tpos;
      stack_frq[stkptr] = HEnCntr[tpos];
      continue;
    }
    while( stkptr )
    {
      if ((stack_frq[stkptr] >= stack_frq[stkptr-1]) || (tpos == tabl_end))
      {
        for( i=stack_pos[stkptr-1]; i<stack_pos[stkptr]; i++ )
            (pHDat+i)->HufCode |= (1 << (pHDat+i)->HufBits++);
        for( i=stack_pos[stkptr]; i<=tpos; i++ )
            (pHDat+i)->HufBits++;
        stack_frq[stkptr-1] += stack_frq[stkptr];
        stkptr--;
        continue;
      }
      break;
    }
  }
  for( i=0; i<tabl_end; i++ )    // sort frequency table (ascending)
  {
    again = 0;
    for( j=0; j<tabl_end; j++ )
    {
      if ( (pHDat+j)->HufCode > (pHDat+(j+1))->HufCode )
      {
        again = 1;
        LongTmp = HEnCntr[j];
        HEnCntr[j] = HEnCntr[j+1];
        HEnCntr[j+1] = LongTmp;
        HufTmpDat = *(pHDat+j);
        *(pHDat+j) = *(pHDat+(j+1));
        *(pHDat+(j+1)) = HufTmpDat;
      }
    }
    if ( !again )
      break;
  }

  stkptr = 0;                 // build Huffman code (+RLE)
  stack_pos[0] = 0;
  stack_frq[0] = REnCntr[0];
  tabl_end = RLEHufTab.HufTbSz - 1;

  for( tpos=1; tpos<=tabl_end; tpos++ )
  {
    if ((REnCntr[tpos] >= stack_frq[stkptr]) || (tpos == tabl_end))
    {
      for( i=stack_pos[stkptr]; i<tpos; i++ )
        (pRDat+i)->HufCode |= (1 << (pRDat+i)->HufBits++);
      (pRDat+tpos)->HufBits++;
      stack_frq[stkptr] += REnCntr[tpos];
    }
    else
    {
      stkptr++;
      stack_pos[stkptr] = tpos;
      stack_frq[stkptr] = REnCntr[tpos];
      continue;
    }
    while( stkptr )
    {
      if ((stack_frq[stkptr] >= stack_frq[stkptr-1]) || (tpos == tabl_end))
      {
        for( i=stack_pos[stkptr-1]; i<stack_pos[stkptr]; i++ )
            (pRDat+i)->HufCode |= (1 << (pRDat+i)->HufBits++);
        for( i=stack_pos[stkptr]; i<=tpos; i++ )
            (pRDat+i)->HufBits++;
        stack_frq[stkptr-1] += stack_frq[stkptr];
        stkptr--;
        continue;
      }
      break;
    }
  }
  for( i=0; i<tabl_end; i++ )    // sort frequency table (ascending)
  {
    again = 0;
    for( j=0; j<tabl_end; j++ )
    {
      if ( (pRDat+j)->HufCode > (pRDat+j+1)->HufCode )
      {
        again = 1;
        LongTmp = REnCntr[j];
        REnCntr[j] = REnCntr[j+1];
        REnCntr[j+1] = LongTmp;
        HufTmpDat = *(pRDat+j);
        *(pRDat+j) = *(pRDat+j+1);
        *(pRDat+j+1) = HufTmpDat;
      }
    }
    if ( !again )
      break;
  }

  nbits1 = pUserTab->HufTbSz * 24 + 64; // determine best compression method
  for( i=0; i<pUserTab->HufTbSz; i++ )
    nbits1 += ( HEnCntr[i] * (ULONG) (pHDat+i)->HufBits );
  nbits2 = RLEHufTab.HufTbSz * 24 + 64;
  for( i=0; i<RLEHufTab.HufTbSz; i++ )
    nbits2 += ( REnCntr[i] * (ULONG) (pRDat+i)->HufBits );
  if ( nbits1 <= nbits2 )
    pUserTab->HufMode = 0;
  else
  {
    pUserTab->HufMode = 1;
    pUserTab->HufTbSz = RLEHufTab.HufTbSz;
    for( i=0; i<pUserTab->HufTbSz; i++ )
    {
      HEnCntr[i] = REnCntr[i];
      *(pHDat+i) = *(pRDat+i);
    }
  }
  return( pUserTab->HufTbSz );
}

/**
 **  Name:        HufCmpKernel
 **
 **  Description: This routine creates compressed data based on the FOOT and
 **               Huffman encoded bit string tables already built.
 **
 **  Arguments:   pTarget - a pointer indicating the memory location that
 **                         will receive finished compressed data (any type)
 **               pData   - a pointer indicating the memory location of the
 **                         start of source data to be compressed (any type)
 **               nbytes  - #of of source data bytes to compress
 **
 **               pTable  - a pointer to the Huffman Table to use
 **                 *NOTE* if this value is NULL, the internal default table
 **                        is used
 **
 **  Returns:     The number of bytes actually placed in the pTarget memory
 **               area (compressed data size)
 **
 **  Comments:    This routine acts as the common Huffman compression kernel
 **               for all public compression routines.
 **
 **               !!NOTE!! The resulting output compressed data
 **               may actually require MORE space than the source data.  Make
 **               sure that the "pTarget" memory area is at least 1/3 larger
 **               than the size of the source data; just in case!
 **/

USHORT HufCmpKernel( void* pTarget, void* pData, USHORT nbytes,
                     HUFFMANTAB* pTable )
{
  USHORT i, j, k, l, m; // loop index counters
  USHORT nCmprs;        // number of compressed bytes
  UBYTE* pByte;         // byte pointer created from pData
  UBYTE* pOutp;         // byte pointer created from pTarget

  pByte = (UBYTE*) pData;           // force pointer to be byte like
  pOutp = (UBYTE*) pTarget;         // force pointer to be byte like

  if ( pTable == 0 )                // get the current Huffman Table
    pHufTab = &DefHufTab;
  else
    pHufTab = pTable;

  nCmprs = 0;                       // initial status vars to zero
  BitsNStr = 0;
  BBldr.ldata = 0;

  if ( pHufTab->HufMode )           // RLE+Huffman method
  {
    for( i=0; i<nbytes; i++ )       // loop for all uncompressed bytes
    {
      j = PutRLEByte(*pByte++);     // encode RLE bytes
      for( k=0; k<j; k++ )          // dump RLE buffer if necessary
      {
        l = PutHufByte(RLEBuff[k]); // encode Huffman bits
        for( m=0; m<l; m++ )        // dump Huffman buffer if needed
          *(pOutp+nCmprs++) = HufBuff[m];
      }
    }
    j = DmpRLEBufr();              // when finished, dump remaining RLE
    for( k=0; k<j; k++ )
    {
      l = PutHufByte(RLEBuff[k]);   // encode remaining Huffman bits
      for( m=0; m<l; m++ )          // dump any remaining Huffman bits
        *(pOutp+nCmprs++) = HufBuff[m];
    }
  }
  else                              // Huffman only method
  {
    for( i=0; i<nbytes; i++ )       // loop for all uncompressed bytes
    {
      l = PutHufByte(*pByte++);     // encode Huffman bits
      for( m=0; m<l; m++ )          // dump Huffman buffer if needed
        *(pOutp+nCmprs++) = HufBuff[m];
    }
  }

  l = DmpHufBufr();                 // finally, dump remaining Huffman bits
  for( m=0; m<l; m++ )
      *(pOutp+nCmprs++) = HufBuff[m];

  return( nCmprs );                 // return #of bytes placed to pTarget
}

/**
 **  Name:        CmpString
 **
 **  Description: This routine compresses null terminated strings which can be
 **               decompressed by routine "DcpString".
 **
 **  Arguments:   pTarget - a pointer indicating the memory location that
 **                         will receive finished compressed data (any type)
 **
 **               pString - a pointer indicating the null terminated character
 **                         string to compress (any type)
 **
 **               pTable  - a pointer to the Huffman Table to use
 **                 *NOTE* if this value is NULL, the internal default table
 **                        is used
 **
 **  Returns:     The number of bytes actually placed in the pTarget memory
 **               area (compressed data size)
 **
 **  Comments:    Call this routine for source strings less than 8192 bytes in
 **               length.  A one/two byte data length counter is prefixed to the
 **               resulting compressed data to allow the decompression routine
 **               to know how many bytes to decompress later.  This routine
 **               can be called several times; each call produces a "stand-
 **               alone" compressed data module that can be individually
 **               decompressed.  !!NOTE!! The resulting output compressed data
 **               may actually require MORE space than the source data.  Make
 **               sure that the "pTarget" memory area is at least 1/3 larger
 **               than the size of the source data; just in case!
 **/

USHORT CmpString( void* pTarget, void* pData, HUFFMANTAB* pTable )
{
  USHORT nbytes;        // actual source byte count
  UBYTE* pByte;         // byte pointer created from pData
  UBYTE* pOutp;         // byte pointer created from pTarget

  pByte = (UBYTE*) pData;   // force pointer to be byte like
  pOutp = (UBYTE*) pTarget; // force pointer to be byte like

  for( nbytes=0; nbytes<8191; nbytes++ ) // look for string null terminator
    if ( !*(pByte+nbytes) )
      break;

#ifdef EXTENDED_CHARACTER_SET
  *pOutp++ = 0x01;
  nbytes++;
#endif //EXTENDED_CHARACTER_SET

  if ( nbytes > 127 ) // handle long strings (2 byte counter)
  {
    *pOutp++ = (UBYTE)((nbytes & 0x003f) | 0x0040);
    *pOutp++ = (UBYTE)((nbytes >> 6) | 0x0080); // prefix counter
    nbytes = HufCmpKernel( pOutp, pByte, nbytes, pTable ) + 2;
  }
  else                // handle short strings (1 byte counter)
  {
    *pOutp++ = (UBYTE)((nbytes & 0x007f) | 0x0080); // prefix counter
    nbytes = HufCmpKernel( pOutp, pByte, nbytes, pTable ) + 1;
  }
  return( nbytes );
}


#endif  // HNCODE
/******************************************************************************/


