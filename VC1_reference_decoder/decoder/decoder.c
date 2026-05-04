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
 * decoder.c:
 * Main console application, command line parsing, and resource allocation
 *
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <ctype.h>
#include <windows.h>

#include "vc1types.h"
#include "vc1dec.h"
#include "vc1decbit.h"
#include "rdopts.h"
#include "decopts.h"
#include "decfile.h"
#include "vc1debug.h"

static char NameTag[] = "$Name: SMPTE_VC9-r0p0-02rel0 $";

static void aborthandler(int code)
{
    FATAL("Program aborted: code %d\n",code);
    exit(code);
}

HANDLE hInputPipe;

//rh int main(int argc, char **argv)
void VC1(HANDLE hInPipe, PVC1DECODE hd)
{
    vc1DEC_sBitstream sBitstream;
    vc1DEC_sState * pState;
    vc1_sPicture sPicture;
    UWORD32 InstanceSize;
    UWORD32 FramesOutput = 0;
    vc1_eResult Res = vc1_ResultOK;
    vc1_eResult AdvancedProfRes = vc1_ResultOK;
	
	hInputPipe = hInPipe;

//rh     RDOPTS_StartTiming();

    /* If NameTag holds a source control ID: display it */
//rh     if (strlen(NameTag) > 10)
//rh     {
//rh         /* Remove trailing $ */
//rh         NameTag[strlen(NameTag)-2] = '\0';
//rh         /* Print version without preceding $Name: */
//rh         fprintf(stderr, "VC1 decoder, version %s\n",NameTag+7);
//rh     }

    /* Check the arguments */
//rh    if (argc != 2)
//rh    {
//rh        fprintf(stderr,"Usage: decoder <options file name>\n");
//rh        return 1;
//rh    }

    /* Intercept aborts to suppress pop-up dialogs during automated testing */
    signal(SIGABRT,aborthandler);
    signal(SIGSEGV,aborthandler);

    /* Make sure the options structure is set to the defaults (all 0) */
    memset(&DECOPTS_sDecoderSetup, 0, sizeof(DECOPTS_sDecoderSetup));

    /* Parse the options (and so fill in entries in DECOPTS_sDecoderSetup */
//rh    if (vc1_ResultOK != RDOPTS_ReadOptions("decoder",argv[1],DECOPTS_sDecoderOptions))
//rh    {
//rh        FATAL("Failed to parse decoder options\n");
//rh        RDOPTS_FreeOptions(DECOPTS_sDecoderOptions);
//rh        return 1;
//rh    }

    /*
     * Check for required options
     */
//rh     if (DECOPTS_InputFile == NULL)
//rh     {
//rh         FATAL("No input file specified (option BitstreamFile not present)\n");
//rh         RDOPTS_FreeOptions(DECOPTS_sDecoderOptions);
//rh         return 1;
//rh     }
//rh     if (DECOPTS_OutputFile == NULL)
//rh     {
//rh         FATAL("No output file specified (option OutputYUV not present)\n");
//rh         RDOPTS_FreeOptions(DECOPTS_sDecoderOptions);
//rh         return 1;
//rh     }

    /*
     * Check any details not validated yet.
     */
//rh     if (DECOPTS_ValidateParameters() != vc1_ResultOK)
//rh     {
//rh         RDOPTS_FreeOptions(DECOPTS_sDecoderOptions);
//rh         return 1;
//rh     }

    /*
     * Prepare the input file reader (may override options during file setup.
     * Also reads in the sequence header.
     * This is a first-stage initialisation: we do not yet know the profile and level.
     */
    if(vc1_ResultOK != DECFILE_InitialiseBitstreamFileReader(&sBitstream))
    {
        FATAL("Failed to initialise the file reader\n");
//rh         RDOPTS_FreeOptions(DECOPTS_sDecoderOptions);
//rh         return 1;
		return;
    }

    /* Obtain the workspace allocation requirements of the decoder */
    Res = vc1DEC_DecoderRequirements(&InstanceSize, &DECOPTS_sDecoderSetup, &sBitstream);
    if (Res != vc1_ResultOK)
    {
        FATAL("Failed to get decoder workspace size (error %d)\n", Res);
        DECFILE_FinaliseBitstreamFileReader(&sBitstream);
//rh         RDOPTS_FreeOptions(DECOPTS_sDecoderOptions);
    }

    /* Allocate the required workspace */
    pState = (vc1DEC_sState *)malloc(InstanceSize);
    if(NULL == pState)
    {
        FATAL("No room for decoder state\n");
        DECFILE_FinaliseBitstreamFileReader(&sBitstream);
//rh         RDOPTS_FreeOptions(DECOPTS_sDecoderOptions);
//rh         return 1;
		return;
    }

    /* Initialise the decoder */
    Res = vc1DEC_DecoderInitialise(pState, &DECOPTS_sDecoderSetup);
    if (Res != vc1_ResultOK)
    {
        FATAL("Cannot initialise decoder (error %d)\n",Res);
        DECFILE_FinaliseBitstreamFileReader(&sBitstream);
        free(pState);
//rh         RDOPTS_FreeOptions(DECOPTS_sDecoderOptions);
//rh         return 1;
		return;
    }

    /*
     * The decoder initialisation has set up the profile and level.
     * Adjust the bitstream buffering to match.
     */
    if(vc1_ResultOK != DECFILE_AdjustBitstreamBuffering(&sBitstream))
    {
        FATAL("Failed to allocate file reader buffering\n");
        DECFILE_FinaliseBitstreamFileReader(&sBitstream);
        free(pState);
//rh         RDOPTS_FreeOptions(DECOPTS_sDecoderOptions);
//rh         return 1;
		return;
    }

    /* Everything is now initialised. Loop while bits remain and no errors found */
    Res = vc1DEC_DecodeSequence(pState, &sBitstream);
    if (Res != vc1_ResultOK)
    {
        FATAL("Failed to decode header (error %d)\n",
               Res);
        free(pState);
        DECFILE_FinaliseBitstreamFileReader(&sBitstream);
//rh         RDOPTS_FreeOptions(DECOPTS_sDecoderOptions);
//rh         return 1;
		return;
    }

    /* Sanity-check the file format */
    if ((pState->sSeqParams.eProfile == vc1_ProfileAdvanced) !=
        (DECOPTS_InputType == RDOPTS_ELEMENTARYFORMAT))
    {
        WARN("Format mismatch: %sadvanced profile selected with %selementary file format\n",
            pState->sSeqParams.eProfile == vc1_ProfileAdvanced ? "" : "non-",
            DECOPTS_InputType == RDOPTS_ELEMENTARYFORMAT ? "" : "non-");
    }

    /*
     * Allocate the frame buffer now. For Simple and Main profile, the
     * header just decoded has set the actual picture size.
     */
    if(vc1_ResultOK != DECFILE_InitialiseFrameWriter(&sPicture,pState))
    {
        FATAL("Failed to initialise the frame writer\n");
        free(pState);
        DECFILE_FinaliseBitstreamFileReader(&sBitstream);
//rh         RDOPTS_FreeOptions(DECOPTS_sDecoderOptions);
//rh         return 1;
		return;
    }

    do
    {
        FLAG FrameDecoded = FALSE;

        UBYTE8 StartCodeSuffix;
        /* Read the next frame's data */
        Res = DECFILE_ReadBitstreamFile(&sBitstream,
                                        FramesOutput,
                                        pState->FrameNum,
                                        &StartCodeSuffix);

        if (Res != vc1_ResultOK)
        {
            FATAL("Failed to read bits for decode of frame %d (error %d)\n",
                  pState->FrameNum,
                  Res);
        }

        /* At the end of the file, we may have success reported with no data */
        if (vc1DECBIT_BitCountGet(&sBitstream) > 0)
        {
            /* Decode the next frame */
            if (pState->sSeqParams.eProfile == vc1_ProfileAdvanced)
            {
                /* Despatch on the start code suffix */
                COVERAGE("ANNEX E");
                COVERAGE("ANNEX G");
                switch (StartCodeSuffix)
                {
                case vc1_SCSequenceHeader:
                    AdvancedProfRes = vc1DEC_DecodeSequence(pState, &sBitstream);
                    if (AdvancedProfRes == vc1_ResultOK)
                    {
                        AdvancedProfRes = vc1_ResultNoFrame;
                    }
                    break;

                case vc1_SCEntryPointHeader:
                    AdvancedProfRes = vc1DEC_DecodeEntryPoint(pState, &sBitstream);
                    if (AdvancedProfRes == vc1_ResultOK)
                    {
                        AdvancedProfRes = vc1_ResultNoFrame;
                    }
                    break;

                case vc1_SCFrameHeader:
                    if (AdvancedProfRes == vc1_ResultSlice)
                    {
                        FATAL("Slice IDU expected\n");
                        AdvancedProfRes = vc1_ResultBadType;
                        break;
                    }
                    if (AdvancedProfRes == vc1_ResultField)
                    {
                        FATAL("Field IDU expected\n");
                        AdvancedProfRes = vc1_ResultBadType;
                        break;
                    }

                    AdvancedProfRes = DECOPTS_UpdateOptions();
                    if (AdvancedProfRes == vc1_ResultOK)
                    {
                        AdvancedProfRes = vc1DEC_DecodeFrame(pState, &sBitstream, &sPicture);
                    }

                    if(vc1_ResultOK == AdvancedProfRes || vc1_ResultNoFrame == AdvancedProfRes)
                    {
                        FrameDecoded = TRUE;
                    }
                    break;

                case vc1_SCSlice:
                    if (AdvancedProfRes != vc1_ResultSlice)
                    {
                        FATAL("Slice IDU unexpected\n");
                        AdvancedProfRes = vc1_ResultBadType;
                        break;
                    }
                    AdvancedProfRes = vc1DEC_DecodeSlice(pState, &sBitstream, &sPicture);

                    if(vc1_ResultOK == AdvancedProfRes || vc1_ResultNoFrame == AdvancedProfRes)
                    {
                        FrameDecoded = TRUE;
                    }
                    break;

                case vc1_SCField:
                    if (AdvancedProfRes != vc1_ResultField)
                    {
                        FATAL("Field IDU unexpected\n");
                        AdvancedProfRes = vc1_ResultBadType;
                        break;
                    }
                    AdvancedProfRes = vc1DEC_DecodeField(pState, &sBitstream, &sPicture);

                    if(vc1_ResultOK == AdvancedProfRes || vc1_ResultNoFrame == AdvancedProfRes)
                    {
                        FrameDecoded = TRUE;
                    }
                    break;

                case vc1_SCEndOfSequence:
                    /*
                    ** This code will not be reached unless there is spurious data
                    ** following the header - the 'no data' check above will be triggered
                    */
                    AdvancedProfRes = vc1_ResultNoFrame;
                    break;

                case vc1_SCSliceUser:
                case vc1_SCFieldUser:    
                case vc1_SCFrameUser:
                case vc1_SCEntryPointUser:
                case vc1_SCSequenceUser:
                    /*
                     * Just display the user data, assuming it's an ASCII string.
                     * This verifies the data is in the correct format
                     */
                    {
                        WORD32 Val;
                        COVERAGE("ANNEX F");

                        if (DECOPTS_ShowUserData)
                        {
                            fprintf(stderr, "User %s data: ",
                                   StartCodeSuffix == vc1_SCSliceUser ? "slice" :
                                   StartCodeSuffix == vc1_SCFieldUser ? "field" :
                                   StartCodeSuffix == vc1_SCFrameUser ? "frame" :
                                   StartCodeSuffix == vc1_SCSequenceUser ? "sequence" :
                                                                           "entry point");

                            while (VC1DECBIT_EOF != (Val = vc1DECBIT_GetBits(&sBitstream, 8)))
                            {
                                /* Print as control code if appropriate */
                                if (iscntrl(Val))
                                {
                                    fprintf(stderr, "<0x%02x>",Val);
                                }
                                else
                                {
                                    fprintf(stderr, "%c",Val);
                                }
                            }
                            fprintf(stderr, "\n");
                        }
                        else
                        {
                            /* Read the data to avoid 'unused bits' complaints */
                            while (VC1DECBIT_EOF != (Val = vc1DECBIT_GetBits(&sBitstream, 8)));
                        }
                    }

                    /* Do not update AdvancedProfRes if it is indicating an expected packet */
                    if (AdvancedProfRes == vc1_ResultOK)
                    {
                        /* ... but do update if needed to prevent the frame being re-output */
                        AdvancedProfRes = vc1_ResultNoFrame;
                    }
                    break;

                default:
                    WARN("Reserved IDU: 0x%02x\n", StartCodeSuffix);
                    /* Do not update AdvancedProfRes if it is indicating an expected packet */
                    if (AdvancedProfRes == vc1_ResultOK)
                    {
                        /* ... but do update if needed to prevent the frame being re-output */
                         AdvancedProfRes = vc1_ResultNoFrame;
                    }
                    break;
                }

                Res = AdvancedProfRes;

                if (Res != vc1_ResultOK    && Res != vc1_ResultNoFrame &&
                    Res != vc1_ResultField && Res != vc1_ResultSlice)
                {
                    FATAL("Failed to decode bitstream frame %d (error %d)\n",
                           pState->FrameNum-1,
                           Res);

                    /*
                     * Write out the current frame - this keeps the output in sync
                     * so the flushed frame is at the right offset. Note that we
                     * don't write out the first frame as this would have been
                     * a ResultNoFrame had it succeeded.
                     */
                    if (pState->FrameNum > 1)
                    {
                        DECFILE_WriteFileFrame(&sPicture,pState,&FramesOutput);
                    }

                    break;
                }
            }
            else
            {
                Res = DECOPTS_UpdateOptions();
                if (Res == vc1_ResultOK)
                {
                    Res = vc1DEC_DecodeFrame(pState, &sBitstream, &sPicture);
                }

                /* Simple/main cannot include slices or fields */
                if (Res != vc1_ResultOK && Res != vc1_ResultNoFrame)
                {
                    FATAL("Failed to decode bitstream frame %d (error %d)\n",
                           pState->FrameNum-1,
                           Res);
                    /*
                     * Write out the current frame - this keeps the output in sync
                     * so the flushed frame is at the right offset. Note that we
                     * don't write out the first frame as this would have been
                     * a ResultNoFrame had it succeeded.
                     */
                    if (pState->FrameNum > 1)
                    {
                        DECFILE_WriteFileFrame(&sPicture,pState,&FramesOutput);
                    }

                }
            }

            if (Res == vc1_ResultOK )
            {
                Res = DECFILE_WriteFileFrame(&sPicture,pState,&FramesOutput);
                if (Res != vc1_ResultOK)
                {
                    FATAL("Failed to write frame %d to file (error %d)\n",
                           FramesOutput-1,
                           Res);
                }

                /* Frame has now been output */
                AdvancedProfRes = vc1_ResultNoFrame;
            }

            if (TRUE == FrameDecoded)
            {
                /* Update HRD */
                vc1DEC_UpdateBuffers(pState, DECFILE_GetHRDBytesRead() * 8);
                DECFILE_SetHRDBytesRead(0);
            }
        }
        else /* no more data */
        {
            break;
        }

    } while (Res == vc1_ResultOK    || Res == vc1_ResultNoFrame ||
             Res == vc1_ResultField || Res == vc1_ResultSlice);

    Res = vc1DEC_DecodeFlush(pState,&sPicture);
    if (Res != vc1_ResultOK)
    {
        FATAL("Failed to flush decoder (error %d)\n",
               Res);
    }

    /*
     * There is always a frame to flush
     * - unless there were no frames in the sequence anyway.
     */
	{
		UHWD16 Width  = pState->sSeqParams.MaxCodedWidth;
		UHWD16 Height = pState->sSeqParams.MaxCodedHeight;

		memcpy(hd->pY, sPicture.sY.pData, Width * Height);
		memcpy(hd->pU, sPicture.sU.pData, (Width / 2) * (Height / 2));
		memcpy(hd->pV, sPicture.sV.pData, (Width / 2) * (Height / 2));
		hd->x = Width;
		hd->y = Height;
	}

    Res = DECFILE_WriteFileFrame(&sPicture,pState,&FramesOutput);
    if (Res != vc1_ResultOK)
    {
        FATAL("Failed to write frame %d to file (error %d)\n",
               FramesOutput-1,
               Res);
    }

    free(pState);
//rh    DECFILE_FinaliseBitstreamFileReader(&sBitstream);
//rh    DECFILE_FinaliseFrameWriter(&sPicture);
//rh    RDOPTS_FreeOptions(DECOPTS_sDecoderOptions);
//rh    RDOPTS_EndTiming();

//rh     return Res == vc1_ResultOK ? 0 : 1;
}



