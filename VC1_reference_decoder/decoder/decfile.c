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
 * decfile.c
 *
 * Decoder file handling: Writing out of frame data to file, and
 *   reading in of the input bitstream.
 *
 *  
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>

#if defined(_MSC_VER)
#include <io.h>
#include <fcntl.h>
#endif

#include "vc1types.h"
#include "vc1dec.h"
#include "rdopts.h"
#include "decopts.h"
#include "vc1decbit.h"
#include "decfile.h"
#include "rcv.h"
#include "vc1debug.h"

/********************************************************************
 * Buffer size rounding constants
 *******************************************************************/

#define BUFFER_ROUND    (32)    /*
                                 * Always round up to a multiple of 32,
                                 * to cover interlace and scaling
                                 * without having to think about it.
                                 */
#define BUFFER_ROUND_MASK (BUFFER_ROUND-1)

#define INIT_BUF_SIZE   (256)   /* Need some initial data to be able to
                                 * decode the profile and level. Size
                                 * needs to cover complete sequence header.
                                 */

/********************************************************************
 * Static variables
 *******************************************************************/

static FILE    *pInputFile;
static UBYTE8  *pBitBuffer = NULL;
static UWORD32  BitBufferSize = 0;
static FILE    *pOutputFile;
static UBYTE8   InitialBuffer[INIT_BUF_SIZE];
static FLAG     RCVIsV2Format;
static UWORD32  HRDBytesRead;
 
/********************************************************************
 * Local variable access functions
 *******************************************************************/

UWORD32 DECFILE_GetHRDBytesRead(void)
{
    return(HRDBytesRead);
}

void DECFILE_SetHRDBytesRead(UWORD32 Bytes)
{
    HRDBytesRead = Bytes;
}

//////////////////////////////////////////////////////////////////////
extern HANDLE hInputPipe;
BYTE * bSeekBuffer = NULL;
int nSeekBufferRead = 0;
int nSeekBufferWrite = 0;
int nSeekBufferMax = 0;
int nPipePseudo_ftell = 0;
BOOL fVC1PipeEOF = 0;

int FillSeekBuffer(int nBytes)
{
	int nTotalRead = 0;

	while (nBytes)
	{
		DWORD dwRead;

		ReadFile(hInputPipe, &bSeekBuffer[nSeekBufferWrite], nBytes, &dwRead, NULL);
		if (dwRead == 0)
		{
			fVC1PipeEOF = TRUE;
			return 0;
		}
		nBytes -= dwRead;
		nSeekBufferWrite += dwRead;
		return dwRead;
	}

	return TRUE;
}

size_t rh_fread(unsigned char *buffer, size_t size, size_t count, FILE *stream)
{
	int nTotalBytesNeeded = size * count;

	if (bSeekBuffer == NULL)
	{
		bSeekBuffer = LocalAlloc(LPTR, INIT_BUF_SIZE * 2);
		nSeekBufferMax = INIT_BUF_SIZE * 2;
		nSeekBufferRead = nSeekBufferWrite = 0;
	}

	do
	{
		int nFreeBytes = nSeekBufferMax - nSeekBufferWrite;
		int nActiveBytes = nSeekBufferWrite - nSeekBufferRead;

		if (nActiveBytes >= nTotalBytesNeeded)
		{
			if (buffer != NULL)
				memcpy(buffer, &bSeekBuffer[nSeekBufferRead], nTotalBytesNeeded);
			nSeekBufferRead += nTotalBytesNeeded;
			nPipePseudo_ftell += nTotalBytesNeeded;
			break;
		}

		if (nFreeBytes < nTotalBytesNeeded)
		{
			// Not enough room in the buffer, so slide the buffer
			int nBytesOver = nTotalBytesNeeded - nFreeBytes;
			memcpy(bSeekBuffer, &bSeekBuffer[nBytesOver], nSeekBufferWrite - nBytesOver);
			nSeekBufferWrite -= nBytesOver;
			nSeekBufferRead -= nBytesOver;
			continue;
		}

		if (nTotalBytesNeeded > nActiveBytes)
		{
			int nRead = FillSeekBuffer(nTotalBytesNeeded);
			if (nRead == FALSE)
				return 0;
			continue;
		}


	} while (TRUE);

	return nTotalBytesNeeded;
}

int rh_fseek(FILE *stream, long offset, int origin )
{
	{
		char szTemp[128];
		char szMoveType[16] = {"?"};

		switch(origin)
		{
		case SEEK_CUR:
			lstrcpy(szMoveType, "SEEK_CUR");
			break;
		case SEEK_END:
			lstrcpy(szMoveType, "SEEK_END");
			break;		// never used
		case SEEK_SET:
			lstrcpy(szMoveType, "SEEK_SET");
			break;
		}
		wsprintf(szTemp, "VC1: rh_fseek(%s %d)\n", szMoveType, offset);
		OutputDebugString(szTemp);
	}

	switch(origin)
	{
	case SEEK_CUR:
		if (offset > 0)
		{
			while (offset)
			{
				rh_fread(NULL, 1, 1, pInputFile);
				offset--;
				nPipePseudo_ftell++;
			}
		}
		else
		{
			nSeekBufferRead += offset;
			nPipePseudo_ftell += offset;

			if (nSeekBufferRead < 0)
			{
				nSeekBufferRead = nPipePseudo_ftell = 0;
			}
		}
		break;
	case SEEK_END:
		break;		// never used
	case SEEK_SET:
		nSeekBufferRead = 0;
		nPipePseudo_ftell = 0;
	}
	return 0;
}

long rh_ftell(FILE *stream)
{
	return nPipePseudo_ftell;
}

int rh_feof(FILE *stream)
{
	if (fVC1PipeEOF)
	{
		OutputDebugString("************************ VC1 EOF\n");
	}
	return fVC1PipeEOF;
}
//////////////////////////////////////////////////////////////////////

/********************************************************************
 * Endian-agnostic file input
 *******************************************************************/

#define READ_ITEM(_v_) ReadItem((UBYTE8 *) &(_v_),sizeof(_v_))

#ifdef __STRICT_ANSI__
  #define GET_32(__i__) (__i__.Low)
#else
  #define GET_32(__i__) ((UWORD32)__i__)
#endif

static FLAG ReadItem(UBYTE8 *pItem, int Size)
{
    UWORD32 High=0, Low=0;
    UBYTE8 tchar;
    int shift = 0;
    int numbytes = Size;
    
    /* Read a little-endian file value into a variable, regardless of host endianness */
    while (numbytes-- >0)
    {
        if (rh_fread(&tchar,1,1,pInputFile) != 1)
        {
            return FALSE;
        }

        if (shift >= 32)
        {
            High |= tchar << (shift-32);
        }
        else
        {
            Low  |= tchar << shift;
        }
        shift += 8;
    }
    
    switch (Size)
    {
    case 1:
        *pItem = (UBYTE8)Low;
        break;
        
    case 2:
        *(UHWD16 *)pItem = (UHWD16)Low;
        break;
        
    case 4:
        *(UWORD32 *)pItem = (UWORD32)Low;
        break;
        
    case 8:
#ifdef __STRICT_ANSI__
        ((ULLONG64 *)pItem)->High = High;
        ((ULLONG64 *)pItem)->Low  = Low;
#else
        *(ULLONG64 *)pItem = ((ULLONG64)High<<32)|(ULLONG64)Low;
#endif
        break;

    default:
        WARN("Unknown size (%d) item read requested",Size);
        return FALSE;
    }

    return TRUE;
}

/********************************************************************
 * Elementary file format handling
 *******************************************************************/

/*
 * Read in the next chunk. Leave the start code in the buffer for
 * the decoder to use in deciding which function to call.
 */
static WORD32 ReadElementaryChunk(vc1DEC_sBitstream *pBitstream, UBYTE8 *pStartCodeSuffix)
{
    vc1_eResult eResult;
    WORD32 Bytes;
    UBYTE8 *pBufStart, *pBufEnd;
    int NumZeroes;
    int BytesRead = (int)rh_ftell(pInputFile);

    /* Read a buffer-full of data */
    Bytes = rh_fread(pBitBuffer,1,BitBufferSize,pInputFile);
    DEBUG3(vc1DEBUG_BIT, "At %08x, Read %d bytes, BufSize %d\n",
        (int)pBitBuffer, Bytes, BitBufferSize);

    /* Skip over any zero-stuffing bytes */
    for (pBufStart = pBitBuffer;
         pBufStart-pBitBuffer < Bytes && *pBufStart == 0;
         pBufStart++);

    /* Check for a start code */
    if (pBufStart-pBitBuffer     < 2     ||
        pBufStart-pBitBuffer + 2 > Bytes ||
        *pBufStart != 0x01)
    {
        DEBUG2(vc1DEBUG_BIT, "At %08x, no start code, byte = %02x\n", (int)pBufStart, *pBufStart);
        /* vc1_ResultNoStartCode */
        return(-1);
    }

    DEBUG2(vc1DEBUG_BIT, "At %08x, start code 00 00 01 %02x\n", (int)pBufStart-2, pBufStart[1]);

    pBufEnd = pBufStart+2;
    *pStartCodeSuffix = pBufStart[1];
    pBufStart += 2;          /* Point at the first byte in the encapsulated data */
    NumZeroes = 0;

    /* scan for end of buffer or start code */
    while (pBufEnd-pBitBuffer < Bytes && !(NumZeroes >= 2 && *pBufEnd == 0x01))
    {
        if (*pBufEnd == 0)
        {
            NumZeroes += 1;
        }
        else
        {
            NumZeroes = 0;
        }
        pBufEnd++;
    }

    if (pBufEnd-pBitBuffer < Bytes)
    {
        DEBUG2(vc1DEBUG_BIT, "At %08x, start code 00 00 01 %02x\n", (int)pBufEnd-2, pBufEnd[1]);

        /* Found a start code */
        while (*--pBufEnd == 0);  /* scan back over zero bytes */

        /* pBufEnd points at the last byte we want to include in this chunk */

        /* Wind the file pointer back so it will reread the start of the next chunk */
        rh_fseek(pInputFile, (pBufEnd+1)-(pBitBuffer+Bytes), SEEK_CUR);
		{
			char szTemp[128];
			wsprintf(szTemp, "VC1: Seek %d bytes\n", (pBufEnd+1)-(pBitBuffer+Bytes));
			OutputDebugString(szTemp);
		}
    }
    else /* End of buffer */
    {
        DEBUG1(vc1DEBUG_BIT, "At %08x, end of buffer\n", (int)pBufEnd);

        /* No start code found */
        while (*--pBufEnd == 0);  /* scan back over zero bytes */
    }

    Bytes = pBufEnd-pBufStart+1;

    /* Set the bitstream to use the new chunk, with encapsulation enabled */
    vc1DECBIT_InitialiseBitstream(pBitstream, pBufStart, Bytes, TRUE);

    DEBUG1(vc1DEBUG_API, "Read %d bytes for elementary chunk (before trim)\n", Bytes);

    /* Remove the stop bit */
    eResult = vc1DECBIT_TrimBuffer(pBitstream);    
    if(vc1_ResultOK != eResult)
    {
        return(-1);
    }

    /* Calculate number of bytes read for HRD */
    BytesRead = (int)rh_ftell(pInputFile) - BytesRead;
    DEBUG1(vc1DEBUG_HRD, "Application read %d bits\n", BytesRead * 8);
	{
		char szTemp[128];
		wsprintf(szTemp, "VC1: Read %d bytes\n", BytesRead);
		OutputDebugString(szTemp);
	}

    return BytesRead;
}


/********************************************************************
 * Output Handler Functions
 *******************************************************************/

 /*
 * Description:
 *  Initialise the frame writer component, including allocation of frame buffers.
 *
 * Remarks:
 *  Opens the file named in OutputSequence
 *
 * Inputs:
 *   pPicture     a pointer to the picture structure to hold buffer pointers
 *   pState       a pointer to the state structure holding the picture size
 *
 * Outputs:
 *   pPicture is updated with pointers to the allocated buffers
 *
 * Returns:
 *   vc1_ResultOK on success
 */
vc1_eResult DECFILE_InitialiseFrameWriter(vc1_sPicture *pPicture, const vc1DEC_sState *pState)
{
    int AllocatedWidth;
    int AllocatedHeight;

    /* Start by checking that the output file is valid */
//rh     if (strcmp(DECOPTS_OutputFile, "stdout")==0)
//rh     {
//rh         /* Send the video to stdout */
//rh #if defined(_MSC_VER)
//rh         /* Set stdout to a binary stream */
//rh         if (setmode( fileno( stdout ), O_BINARY )==-1)
//rh         {
//rh             fprintf(stderr, "setmode failed\n");
//rh         }
//rh #endif
//rh         /* Increase stdout buffer size */
//rh         if (setvbuf(stdout, NULL, _IOFBF, 1024*1024)!=0)
//rh         {
//rh             fprintf(stderr, "setvbuf failed\n");
//rh         }
//rh 
//rh         /* Output to stdout */
//rh         pOutputFile = stdout;
//rh     }
//rh     else if (NULL == (pOutputFile = fopen(DECOPTS_OutputFile, "wb")))
//rh     {
//rh         WARN("Cannot open output file '%s'\n",
//rh              DECOPTS_OutputFile);
//rh         return vc1_ResultBadFile;
//rh     }

    AllocatedWidth  = pState->sSeqParams.MaxCodedWidth;
    AllocatedHeight = pState->sSeqParams.MaxCodedHeight;

    if (AllocatedWidth==0 || AllocatedHeight==0)
    {
        /* Cannot set Bpl fields in buffer setup below: fail */
        WARN("No picture size: cannot set up picture buffers\n");
        fclose(pOutputFile);
        return vc1_ResultBadImageSize;
    }

    /*
     * Allocate the YUV buffer space:
     *  Width   * Height for Y
     *  Width/2 * Height/2 for each of U,V
     */
    pPicture->sY.pData = (UBYTE8 *)malloc(AllocatedWidth * AllocatedHeight * 3/2);
    if (pPicture->sY.pData == NULL)
    {
        WARN("Cannot allocate picture buffers\n");
        fclose(pOutputFile);
        return vc1_ResultNoMemory;
    }
    pPicture->sY.Bpl = AllocatedWidth;
    pPicture->sU.pData = pPicture->sY.pData + AllocatedWidth*AllocatedHeight;
    pPicture->sU.Bpl = AllocatedWidth/2;
    pPicture->sV.pData = pPicture->sU.pData + AllocatedWidth*AllocatedHeight/4;
    pPicture->sV.Bpl = AllocatedWidth/2;

    return vc1_ResultOK;
}

/*
 * Description:
 *  Free any resources held by the frame writer component
 *
 * Inputs:
 *   pPicture     a pointer to the picture structure holding buffer pointers
 */
void DECFILE_FinaliseFrameWriter(vc1_sPicture *pPicture)
{
    free(pPicture->sY.pData);
    fclose(pOutputFile);
}

static FLAG WriteBlock(vc1_sComponent *pBlock, int Width, int Height)
{
    int y;
    UBYTE8 *pData = pBlock->pData;
    for (y=0; y<Height; y++)
    {
        if (fwrite(pData,1,Width,pOutputFile) != (size_t) Width)
        {
            return FALSE;
        }
        pData += pBlock->Bpl;
    }

    return TRUE;
}


/*
 * Description:
 *  Write the data for the next frame into the picture buffer
 *
 * Inputs:
 *  pPicture      the picture buffers to write out
 *  pState        pointer to the state holding the actual picture size
 *
 * Returns:
 *   vc1_ResultOK on success
 */
static vc1_eResult DECFILE_WriteFrame(vc1_sPicture *pPicture,
                               const vc1DEC_sState *pState,
                               UWORD32 FramesOutput)
{
    UHWD16 Width  = pState->sSeqParams.MaxCodedWidth;
    UHWD16 Height = pState->sSeqParams.MaxCodedHeight;
    size_t Size   = Width * Height *3/2;

#ifndef NDEBUG
    char pStringBuffer[250];
    char *pInfo = pStringBuffer;
    unsigned int Value;

    pInfo += sprintf(pInfo, "Display %3d Decoded %3d", FramesOutput, pPicture->Frame);

    Value = pPicture->ePictureFormat;
    if (Value < 3)
    {
        pInfo += sprintf(pInfo, " %s", vc1DEBUG_PictureFormat[Value]);
    }
    else
    {
        pInfo += sprintf(pInfo, " 0x%08x", Value);
    }

    Value = pPicture->sField[0].ePictureType;
    if (Value < 5)
    {
        pInfo += sprintf(pInfo, " %s", vc1DEBUG_PictureType[Value]);
    }
    else
    {
        pInfo += sprintf(pInfo, " 0x%08x", Value);
    }


    if (pPicture->ePictureFormat == vc1_InterlacedField)
    {
        Value = pPicture->sField[1].ePictureType;
        if (Value < 5)
        {
            pInfo += sprintf(pInfo, "/%s", vc1DEBUG_PictureType[Value]);
        }
        else
        {
            pInfo += sprintf(pInfo, "/0x%08x", Value);
        }

        pInfo += sprintf(pInfo, " TFF=%d", pPicture->TFF);
    }

    DEBUG1(vc1DEBUG_FRAME, "%s\n", pStringBuffer);
#endif
          
    /*
     * Write the sequence data. Note that malloc'ing all the buffers as one block
     * means that YUV format file data can just be written en masse - as long as
     * the size has not been rounded up.
     */
    if (Width == pPicture->sY.Bpl && (Height & BUFFER_ROUND_MASK) == 0)
    {
		DWORD dwWritten;
		HANDLE hOutputFile = CreateFile("c:\\test.yuv", GENERIC_WRITE, 0, (LPSECURITY_ATTRIBUTES) NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, (HANDLE) NULL);
		WriteFile(hOutputFile, pPicture->sY.pData, Size, &dwWritten, NULL);
		CloseHandle(hOutputFile);
//rh         if (fwrite(pPicture->sY.pData, 1, Size, pOutputFile) != Size)
//rh         {
//rh             FATAL("Failed to write picture data for output frame %d\n", FramesOutput);
//rh             return vc1_ResultNoData;
//rh         }
    }
    else
    {
        /* Need to write line-by-line */
        if (!WriteBlock(&pPicture->sY,Width,  Height) ||
            !WriteBlock(&pPicture->sU,Width/2,Height/2) ||
            !WriteBlock(&pPicture->sV,Width/2,Height/2))
        {
            FATAL("Failed to write picture data for frame %d\n", FramesOutput);
            return vc1_ResultNoData;
        }

    }

    return vc1_ResultOK;
}

/*
 * Description:
 * Write the data for the next frame to the output file
 *
 * Inputs:
 * pPicture      the picture buffers to write out
 * pState        pointer to the state holding the actual picture size
 * pFramesOutput pointer to display frame number
 * LastFrame     last frame flag
 *
 * Returns:
 *   vc1_ResultOK on success
 */
vc1_eResult DECFILE_WriteFileFrame(
    vc1_sPicture *pPicture,
    const vc1DEC_sState *pState,
    UWORD32 *pFramesOutput
)
{
    vc1_eResult Res;

    if (pState->FrameNum==0)
    {
        /* Nothing decoded yet */
        return vc1_ResultOK;
    }

    /* Write the frame */
    Res = DECFILE_WriteFrame(pPicture,pState,*pFramesOutput);
    *pFramesOutput += 1;
    if (Res != vc1_ResultOK)
    {
        FATAL("Failed to write frame %d to file (error %d)\n",
               *pFramesOutput-1,
               Res);
    }

    return Res;
}


/********************************************************************
 * Input Handler Functions
 *******************************************************************/

vc1_eResult DECFILE_InitialiseBitstreamFileReader(vc1DEC_sBitstream *pBitstream)
{
    vc1_eResult res = vc1_ResultOK;
    UWORD32 StartCode;

    /* Do temporary bitstream buffer setup. */
    BitBufferSize = INIT_BUF_SIZE;
    pBitBuffer = InitialBuffer;

    /* Open the input file */
//rh     pInputFile = fopen(DECOPTS_InputFile, "rb");
//rh     if(NULL == pInputFile)
//rh     {
//rh         WARN("Cannot open input file '%s'\n",DECOPTS_InputFile);
//rh         return vc1_ResultBadFile;
//rh     }

    /*
     * Work out the input file type.
     */

    /* READ_ITEM reads data as little-endian, and so reverses the bytes */
    if (READ_ITEM(StartCode) && StartCode == (0x00010000 | (vc1_SCSequenceHeader << 24)))
    {
        int BytesRead;
        UBYTE8 DiscardedSuffix;
        DEBUG0(vc1DEBUG_API,"File is an Elementary format file\n");
        DECOPTS_InputType = RDOPTS_ELEMENTARYFORMAT;

        rh_fseek(pInputFile,0,SEEK_SET);
        BytesRead = ReadElementaryChunk(pBitstream, &DiscardedSuffix);
        res = vc1_ResultOK;
        if(-1 == BytesRead)
        {
            res = vc1_ResultNoStartCode;
        }

        HRDBytesRead = BytesRead;
    }
    else
    {
        /* Read the RCV header to get the sequence layer */
        UBYTE8 Type;
        UWORD32 Size;

        rh_fseek(pInputFile,0,SEEK_SET);

        /* The first 3 bytes are the number of frames */
        if (rh_fread((unsigned char *)&Size, 1, 3, pInputFile) != 3)
        {
            FATAL("Failed to read RCV number of frames - StartCode = %08x\n", StartCode);
            return vc1_ResultNoData;
        }

        /* The next byte is the type and extension flag */
        if (!READ_ITEM(Type))
        {
            FATAL("Failed to read RCV type - StartCode = %08x\n", StartCode);
            return vc1_ResultNoData;
        }
        
        /*
         * There is only one basic type we deal with,
         * but it comes in two versions
         */
        if ((Type & ~RCV_V2_MASK) != RCV_VC1_TYPE)
        {
            FATAL("Not an RCV file containing VC1 data, or no sequence layer present - StartCode = %08x\n", StartCode);
            return vc1_ResultNoData;
        }
        RCVIsV2Format = (FLAG)((Type & RCV_V2_MASK) != 0);
        DEBUG1(vc1DEBUG_API,"File is a version %d RCV file\n",RCVIsV2Format ? 2 : 1);

        /* Next 4 bytes are the size of the extension data */
        if (!READ_ITEM(Size))
        {
            FATAL("Failed to read RCV frame size - StartCode = %08x\n", StartCode);
            return vc1_ResultNoData;
        }
        
        /* The extension data is the sequence layer - read in for the decoder */
        vc1DECBIT_InitialiseBitstream(pBitstream,
                                      pBitBuffer,
                                      rh_fread(pBitBuffer, 1, Size, pInputFile),
                                      FALSE);

        /* Next 8 bytes are the height and width */
        if (!READ_ITEM(Size))
        {
            FATAL("Failed to read RCV picture size - StartCode = %08x\n", StartCode);
            return vc1_ResultNoData;
        }
        DECOPTS_sDecoderSetup.MaxCodedHeight = (UHWD16) Size;

        if (!READ_ITEM(Size))
        {
            FATAL("Failed to read RCV picture size - StartCode = %08x\n", StartCode);
            return vc1_ResultNoData;
        }
        DECOPTS_sDecoderSetup.MaxCodedWidth = (UHWD16) Size;

        /* V2 format has another information block */
        if (RCVIsV2Format)
        {
            /* The block starts with its size */
            if (!READ_ITEM(Size))
            {
                FATAL("Failed to read RCV picture size - StartCode = %08x\n", StartCode);
                return vc1_ResultNoData;
            }
            /* We do not need any of the extra information */
            rh_fseek(pInputFile,Size,SEEK_CUR);
        }
                
        /* Now positioned at start of first frame chunk */
    }

    if (res != vc1_ResultOK)
    {
        if (pBitBuffer != InitialBuffer)
        {
            free(pBitBuffer);
        }
    }
    
    return res;
}

/*
 * Description:
 *  Adjust input file buffering to match the profile and level.
 */
vc1_eResult DECFILE_AdjustBitstreamBuffering(vc1DEC_sBitstream *pBitstream)
{
    /*
     * allocate and initialise the bitstream, based on the profile established,
     * or buffer size read.
     */
     if (BitBufferSize == INIT_BUF_SIZE)
     {
          BitBufferSize = DECOPTS_CalculateBitstreamBufferSize();
     }

    pBitBuffer = (UBYTE8 *)malloc(BitBufferSize);
    if (NULL == pBitBuffer)
    {
        WARN("No room for bitstream buffer\n");
        return vc1_ResultNoMemory;
    }

    /* Copy across the data set up in the initial buffer */
    {
        /* Pick up the state to refresh it */
        int Bytes = vc1DECBIT_ByteCountGet(pBitstream);

        if (Bytes > 0)
        {
            memcpy(pBitBuffer,vc1DECBIT_BufferGet(pBitstream),Bytes);
        }

        vc1DECBIT_BufferSet(pBitstream, pBitBuffer, Bytes);
        /* Bitstream uses start and end pointers internally, so this updates the end */
        /*
         * Note that if this is elementary format, we have now (probably) re-added
         * the stop bit and any padding to the buffer. We do not re-trim to reset the
         * end bit position, because if the stop bit was the only bit in the byte,
         * that byte will not be included in the current total.
         * Buffer use checks will allow a partial byte to be left.
         */
    }

	// Adjust the seek buffer too
	{
		BYTE * bNew = LocalAlloc(LPTR, BitBufferSize * 2);
		memcpy(bNew, bSeekBuffer, nSeekBufferWrite);
		LocalFree(bSeekBuffer);
		bSeekBuffer = bNew;
		nSeekBufferMax = BitBufferSize * 2;
	}

    return vc1_ResultOK;
}

/*
 * Description:
 *  Free any resources held by the bitstream file reader component
 */
void DECFILE_FinaliseBitstreamFileReader(vc1DEC_sBitstream *pBitstream)
{
    pBitstream = pBitstream; /* Parameter not used; ignore */

    if (pBitBuffer != InitialBuffer)
    {
        free(pBitBuffer);
    }

    fclose(pInputFile);
}

vc1_eResult DECFILE_ReadBitstreamFile(  vc1DEC_sBitstream *pBitstream,
                                        UWORD32 FramesOutput,
                                        UWORD32 FramesDecoded,
                                        UBYTE8 *pStartCodeSuffix)
{
    /* Check that the previous frame decode used all the bits */
    if (vc1DECBIT_BitCountGet(pBitstream) >= 8)
    {
        DEBUG5(vc1DEBUG_API,
            "Buffer not emptied: %d bits left after %d frame%s decoded, %d frame%s output\n",
             vc1DECBIT_BitCountGet(pBitstream),
             FramesDecoded,
             FramesDecoded == 1 ? "" : "s",
             FramesOutput,
             FramesOutput  == 1 ? "" : "s");

        if (vc1DECBIT_BitCountGet(pBitstream) >= 64)
        {
            WARN("Buffer not emptied: %d bits left after %d frame%s decoded, %d frame%s output\n",
             vc1DECBIT_BitCountGet(pBitstream),
             FramesDecoded,
             FramesDecoded == 1 ? "" : "s",
             FramesOutput,
             FramesOutput  == 1 ? "" : "s");
        }
    }

    /* Ensure the buffer is empty, in case there is nothing more to read */
    vc1DECBIT_InitialiseBitstream(pBitstream,pBitBuffer,0,FALSE);

    if (DECOPTS_InputType == RDOPTS_ELEMENTARYFORMAT)
    {
        int BytesRead;

        /* Is there any more data? */
        if (rh_feof(pInputFile))
        {
            DEBUG0(vc1DEBUG_BIT, "End of file\n");
            return vc1_ResultOK;        /* OK, but no more bytes */
        }
        BytesRead = ReadElementaryChunk(pBitstream,pStartCodeSuffix);

        if(-1 == BytesRead)
        {
            return(vc1_ResultNoStartCode);
        }

        /* Update HRD */
        HRDBytesRead += BytesRead;

        return(vc1_ResultOK);
    }
    else /* DECOPTS_InputType == RDOPTS_RCVFORMAT */
    {
        UWORD32 Size;

        /* Is there any more data? */
        if (rh_feof(pInputFile))
        {
            return vc1_ResultOK;        /* OK, but no more bytes */
        }

        /* Get the next frame. The block starts with the frame size. */
        if (!READ_ITEM(Size))
        {
            if (rh_feof(pInputFile))
            {
                return vc1_ResultOK;     /* Processed all the frames */
            }

            FATAL("Failed to read RCV frame size\n");
            return vc1_ResultNoData;
        }

        if (RCVIsV2Format)
        {
            /* Mask off the flag bits */
            Size = Size & ~RCV_V2_FRAMESIZE_FLAGS;
            /* Skip the timestamp */
            rh_fseek(pInputFile,4,SEEK_CUR);
        }
        
        vc1DECBIT_InitialiseBitstream(pBitstream,
                                      pBitBuffer,
                                      rh_fread(pBitBuffer, 1, Size, pInputFile),
                                      FALSE);
    }

    return vc1_ResultOK;
}

