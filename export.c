#include <windows.h>
#include <commctrl.h>
#include <shlobj.h>
#include <math.h>

#include "TSReader.h"
#include "bcdmux.h"
#include "util.h"
#include "formatter.h"
#include "resource.h"

extern PVARIABLES v;
extern char gszAppName[];
extern TSIDASSIGNMENTS tsid[];
extern int nLayer1Rates[];
extern int nLayer2Rates[];
extern int nLayer3Rates[];
extern struct AC3frmsize AC3frmsizecod_tbl[];
extern int (* GetSyncLossCount) (BOOL fReset);
extern int (* GetRetuneCount) (BOOL fReset);

#ifndef LITE
void WriteAudioSampleXMLData(HANDLE hXMLFile, int nPMTIndex, int nESIndex)
{
	if (v->pat.pmt[nPMTIndex].es[nESIndex].as.nChannels)
	{
		int i;

		for (i = 0; i < v->pat.pmt[nPMTIndex].es[nESIndex].as.nChannels; i++)
		{
			int j;
			char szOpenTag[32];
			char szCloseTag[32];
			char szSampleData[4096] = {0};

			wsprintf(szOpenTag, "   <AUDIO-SAMPLES-CHANNEL-%d>", i + 1); 
			wsprintf(szCloseTag, "</AUDIO-SAMPLES-CHANNEL-%d>", i + 1); 

			for (j = 0; j < v->pat.pmt[nPMTIndex].es[nESIndex].as.nSamples; j++)
			{
				char szTemp[32];

				wsprintf(szTemp, "%d", v->pat.pmt[nPMTIndex].es[nESIndex].as.nSampleData[i][j]);
				if (j < v->pat.pmt[nPMTIndex].es[nESIndex].as.nSamples - 1)
					lstrcat(szTemp, ",");
				lstrcat(szSampleData, szTemp);
			}
			wsprintf(v->szSIFormatBuffer, "%s%s%s", szOpenTag, szSampleData, szCloseTag);
			WriteHTMLLine(hXMLFile, v->szSIFormatBuffer);
		}
	}
}

void XMLExport__WriteESParserData(HANDLE hXMLFile, int nPMTIndex, int nESIndex)
{
	if (v->pat.pmt[nPMTIndex].es[nESIndex].pParsedData != NULL)
	{
		switch(v->pat.pmt[nPMTIndex].es[nESIndex].nBlacklisted)
		{
		default:
			lstrcpy(v->szSIFormatBuffer, "   <BLACKLIST>None</BLACKLIST>"); 	WriteHTMLLine(hXMLFile, v->szSIFormatBuffer);
			break;
		case BLACKLIST_NO_TRAFFIC:
			lstrcpy(v->szSIFormatBuffer, "   <BLACKLIST>No traffic on PID</BLACKLIST>"); 	WriteHTMLLine(hXMLFile, v->szSIFormatBuffer);
			break;
		case BLACKLIST_NO_DATA:
			lstrcpy(v->szSIFormatBuffer, "   <BLACKLIST>No appreciable data</BLACKLIST>"); 	WriteHTMLLine(hXMLFile, v->szSIFormatBuffer);
			break;
		case BLACKLISTED_NO_PES_PACKETS:
			lstrcpy(v->szSIFormatBuffer, "   <BLACKLIST>No PES packets</BLACKLIST>"); 	WriteHTMLLine(hXMLFile, v->szSIFormatBuffer);
			break;
		}

		switch(v->pat.pmt[nPMTIndex].es[nESIndex].nParseType)
		{
		case PARSE_ES_TYPE_MPEG2_VIDEO:
			{
				PPARSEDMPEGVIDEO pMPEG = (PPARSEDMPEGVIDEO)v->pat.pmt[nPMTIndex].es[nESIndex].pParsedData;
				int bit_rate;
				int vbv_buffer_size;
				int horizontal_size;
				int vertical_size;
				char szProgressive[8];
				char szFrameRate[20] = {0};
				char szAspectRatio[20] = {0};
				char szChromaFormat[20] = {0};

				if (pMPEG->marker_bit2)
				{
					// we had an extension header
					bit_rate = (pMPEG->bit_rate_extension << 18 | pMPEG->bit_rate_value) * 400;
					vbv_buffer_size = (pMPEG->vbv_buffer_size_extension << 10 | pMPEG->vbv_buffer_size_value) * 2048;
					horizontal_size = pMPEG->horizontal_size_extension << 12 | pMPEG->horizontal_size_value;
					vertical_size = pMPEG->vertical_size_extension << 12 | pMPEG->vertical_size_value;
				}
				else
				{
					bit_rate = pMPEG->bit_rate_value * 400;
					vbv_buffer_size = pMPEG->vbv_buffer_size_value * 2048;
					horizontal_size = pMPEG->horizontal_size_value;
					vertical_size = pMPEG->vertical_size_value;
				}

				if (pMPEG->progressive_sequence == 1)
					lstrcpy(szProgressive, "TRUE");
				else
					lstrcpy(szProgressive, "FALSE");
				wsprintf(v->szSIFormatBuffer, "   <PROGRESSIVE>%s</PROGRESSIVE>", szProgressive);	WriteHTMLLine(hXMLFile, v->szSIFormatBuffer);
				sprintf(v->szSIFormatBuffer,  "   <BITRATE>%d</BITRATE>", bit_rate);	WriteHTMLLine(hXMLFile, v->szSIFormatBuffer);
				wsprintf(v->szSIFormatBuffer, "   <HORIZONTAL-RESOLUTION>%d</HORIZONTAL-RESOLUTION>", horizontal_size);	WriteHTMLLine(hXMLFile, v->szSIFormatBuffer);
				wsprintf(v->szSIFormatBuffer, "   <VERTICAL-RESOLUTION>%d</VERTICAL-RESOLUTION>", vertical_size);	WriteHTMLLine(hXMLFile, v->szSIFormatBuffer);
				GetMPEG2VideoAspectRation(pMPEG->aspect_ratio_information, szAspectRatio);
				wsprintf(v->szSIFormatBuffer, "   <ASPECT-RATIO>%s</ASPECT-RATIO>", szAspectRatio);	WriteHTMLLine(hXMLFile, v->szSIFormatBuffer);
				GetMPEG2FrameRate(pMPEG->frame_rate_code, szFrameRate);
				wsprintf(v->szSIFormatBuffer, "   <FRAME-RATE>%s</FRAME-RATE>", szFrameRate);	WriteHTMLLine(hXMLFile, v->szSIFormatBuffer);
				GetMPEG2ChromaFormat(pMPEG->chroma_format, szChromaFormat);
				wsprintf(v->szSIFormatBuffer, "   <CHROMA-FORMAT>%s</CHROMA-FORMAT>", szChromaFormat);	WriteHTMLLine(hXMLFile, v->szSIFormatBuffer);

				// See about AFD
				if (v->pat.pmt[nPMTIndex].es[nESIndex].nTeletextServices & VBI_SERVICE_AFD)
				{
					char szAFDFormat[128] = {0};

					GetAFDFormat(v->pat.pmt[nPMTIndex].es[nESIndex].dwAFDData, szAFDFormat);
					wsprintf(v->szSIFormatBuffer, "   <AFD>%s</AFD>", szAFDFormat);
					EscapeReplaceXML(v->szSIFormatBuffer);
					WriteHTMLLine(hXMLFile, v->szSIFormatBuffer);
				}
			}
			break;
#ifndef LITE
		case PARSE_ES_TYPE_MPEG2_AAC_AUDIO:
			//todo
			break;
		case PARSE_ES_TYPE_MPEG4_AAC_AUDIO:
			//todo
			break;
#endif LITE
		case PARSE_ES_TYPE_MPEG_AUDIO:
			{
				PPARSEDMPEGAUDIO pMPEG = (PPARSEDMPEGAUDIO)v->pat.pmt[nPMTIndex].es[nESIndex].pParsedData;
				int bitrate = -1;

				char szLayer[12];
				char szSamplingRate[12];
				char szMode[24];

				switch(pMPEG->layer)
				{
				case 3:		// layer 1
					bitrate = nLayer1Rates[pMPEG->bitrate_index];
					lstrcpy(szLayer, "I");
					break;
				case 2:		// layer 2
					bitrate = nLayer2Rates[pMPEG->bitrate_index];
					lstrcpy(szLayer, "II");
					break;
				case 1:		// layer 3
					bitrate = nLayer3Rates[pMPEG->bitrate_index];
					lstrcpy(szLayer, "III");
					break;
				default:
					lstrcpy(szLayer, "Reserved");
					break;
				}

				wsprintf(v->szSIFormatBuffer, "   <BITRATE>%d</BITRATE>", bitrate);	WriteHTMLLine(hXMLFile, v->szSIFormatBuffer);
				wsprintf(v->szSIFormatBuffer, "   <LAYER>%s</LAYER>", szLayer);	WriteHTMLLine(hXMLFile, v->szSIFormatBuffer);
				GetMPEGAudioMode(pMPEG->mode, szMode);
				wsprintf(v->szSIFormatBuffer, "   <MODE>%s</MODE>", szMode);	WriteHTMLLine(hXMLFile, v->szSIFormatBuffer);
				GetMPEGAudioSamplingFrequency(pMPEG->sampling_frequency, szSamplingRate);
				wsprintf(v->szSIFormatBuffer, "   <SAMPLE-RATE>%s</SAMPLE-RATE>", szSamplingRate);	WriteHTMLLine(hXMLFile, v->szSIFormatBuffer);
				WriteAudioSampleXMLData(hXMLFile, nPMTIndex, nESIndex);
			}
			break;
		case PARSE_ES_TYPE_AC3_AUDIO:
			{
				PPARSEDAC3AUDIO pAC3 = (PPARSEDAC3AUDIO)v->pat.pmt[nPMTIndex].es[nESIndex].pParsedData;
				int nDialNorm = 0;
				char szSamplingRate[16];
				char szBitstreamMode[32];
				char szAudioCodingMode[32];
				char szCMixLev[32] = {"n/a"};
				char szSurMixLev[32] = {"n/a"};
				char szDSurMod[32] = {"n/a"};
				char szLFEMod[16] = {"n/a"};
				char szDialNorm[32] = {"n/a"};
				char szOptionalLine[256] = {0};

				wsprintf(v->szSIFormatBuffer, "   <BITRATE>%d</BITRATE>", AC3frmsizecod_tbl[pAC3->frmsizecod].bit_rate);	WriteHTMLLine(hXMLFile, v->szSIFormatBuffer);
				GetAC3fscod(pAC3->fscod, szSamplingRate);
				wsprintf(v->szSIFormatBuffer, "   <SAMPLE-RATE>%s</SAMPLE-RATE>", szSamplingRate);	WriteHTMLLine(hXMLFile, v->szSIFormatBuffer);
				GetAC3bsmod(pAC3->bsmod, pAC3->acmod, szBitstreamMode);
				wsprintf(v->szSIFormatBuffer, "   <BITSTREAM-MODE>%s</BITSTREAM-MODE>", szBitstreamMode);	WriteHTMLLine(hXMLFile, v->szSIFormatBuffer);
				GetAC3acmod(pAC3->acmod, szAudioCodingMode);
				wsprintf(v->szSIFormatBuffer, "   <CODING-MODE>%s</CODING-MODE>", szAudioCodingMode);	WriteHTMLLine(hXMLFile, v->szSIFormatBuffer);
				GetAC3cmixlev(pAC3->cmixlev, szCMixLev);
				wsprintf(v->szSIFormatBuffer, "   <CENTER-MIX-LEVEL>%s</CENTER-MIX-LEVEL>", szCMixLev);	WriteHTMLLine(hXMLFile, v->szSIFormatBuffer);
				GetAC3surmixlev(pAC3->surmixlev, szSurMixLev);
				wsprintf(v->szSIFormatBuffer, "   <SURROUND-MIX-LEVEL>%s</SURROUND-MIX-LEVEL>", szSurMixLev);	WriteHTMLLine(hXMLFile, v->szSIFormatBuffer);
				GetAC3dsurmod(pAC3->dsurmod, szDSurMod);
				wsprintf(v->szSIFormatBuffer, "   <SURROUND-MODE>%s</SURROUND-MODE>", szDSurMod);	WriteHTMLLine(hXMLFile, v->szSIFormatBuffer);
				if (pAC3->lfeon)
					lstrcpy(szLFEMod, "TRUE");
				else
					lstrcpy(szLFEMod, "FALSE");
				wsprintf(v->szSIFormatBuffer, "   <LFE-MODE>%s</LFE-MODE>", szLFEMod);	WriteHTMLLine(hXMLFile, v->szSIFormatBuffer);				
				nDialNorm = pAC3->dialnorm;
				if (nDialNorm == 0)
					nDialNorm = 31;
				wsprintf(v->szSIFormatBuffer, "   <DIALOG-NORMALIZATION>-%d dB</DIALOG-NORMALIZATION>", nDialNorm);	WriteHTMLLine(hXMLFile, v->szSIFormatBuffer);
				WriteAudioSampleXMLData(hXMLFile, nPMTIndex, nESIndex);
			}
			break;
		case PARSE_ES_TYPE_H264_VIDEO:
			{
				PPARSEDH264VIDEO pH264 = (PPARSEDH264VIDEO)v->pat.pmt[nPMTIndex].es[nESIndex].pParsedData;
				char szProgressive[8];

				if (pH264->interlaced == 0)
					lstrcpy(szProgressive, "TRUE");
				else
					lstrcpy(szProgressive, "FALSE");
				wsprintf(v->szSIFormatBuffer, "   <PROGRESSIVE>%s</PROGRESSIVE>", szProgressive);	WriteHTMLLine(hXMLFile, v->szSIFormatBuffer);
				wsprintf(v->szSIFormatBuffer, "   <HORIZONTAL-RESOLUTION>%d</HORIZONTAL-RESOLUTION>", pH264->horizontal_size_value);	WriteHTMLLine(hXMLFile, v->szSIFormatBuffer);
				wsprintf(v->szSIFormatBuffer, "   <VERTICAL-RESOLUTION>%d</VERTICAL-RESOLUTION>", pH264->vertical_size_value);	WriteHTMLLine(hXMLFile, v->szSIFormatBuffer);
			}
			break;
		case PARSE_ES_TYPE_MPEG4_VIDEO:
			{
				PPARSEDMPEG4VIDEO pMPEG4 = (PPARSEDMPEG4VIDEO)v->pat.pmt[nPMTIndex].es[nESIndex].pParsedData;			

				wsprintf(v->szSIFormatBuffer, "   <HORIZONTAL-RESOLUTION>%d</HORIZONTAL-RESOLUTION>", pMPEG4->horizontal_size_value);	WriteHTMLLine(hXMLFile, v->szSIFormatBuffer);
				wsprintf(v->szSIFormatBuffer, "   <VERTICAL-RESOLUTION>%d</VERTICAL-RESOLUTION>", pMPEG4->vertical_size_value);	WriteHTMLLine(hXMLFile, v->szSIFormatBuffer);
			}
			break;
#ifdef PRO
		case PARSE_ES_TYPE_VC1_VIDEO:
			{
				PPARSEDVC1VIDEO pVC1 = (PPARSEDVC1VIDEO)v->pat.pmt[nPMTIndex].es[nESIndex].pParsedData;
				char szProgressive[8];
				
				if (pVC1->interlaced == 0)
					lstrcpy(szProgressive, "TRUE");
				else
					lstrcpy(szProgressive, "FALSE");
				wsprintf(v->szSIFormatBuffer, "   <PROGRESSIVE>%s</PROGRESSIVE>", szProgressive);	WriteHTMLLine(hXMLFile, v->szSIFormatBuffer);
				wsprintf(v->szSIFormatBuffer, "   <HORIZONTAL-RESOLUTION>%d</HORIZONTAL-RESOLUTION>", pVC1->horizontal_size_value);	WriteHTMLLine(hXMLFile, v->szSIFormatBuffer);
				wsprintf(v->szSIFormatBuffer, "   <VERTICAL-RESOLUTION>%d</VERTICAL-RESOLUTION>", pVC1->vertical_size_value);	WriteHTMLLine(hXMLFile, v->szSIFormatBuffer);
			}
			break;
#endif PRO
		}
	}
}

void XMLExport(HWND hDlg, HANDLE hXMLFile)
{
	int i;
	int nPMTIndex;
	int nServiceIndex;
	int nNetworkPID = -1;
	char szProgramNumbers[256] = {0};
	char szDescriptorData[10 * 1024] = {0};

#ifdef DEBUG_MESSAGES
	OutputDebugString("XML Export\n");
#endif DEBUG_MESSAGES
	UpdateMainStatusText("XML Export");

	// XML header
	WriteHTMLLine(hXMLFile, "<?xml version=\"1.0\" encoding=\"ISO-8859-1\" ?>");
	WriteHTMLLine(hXMLFile, "<!-- Created by TSReader from COOLSTF.com -->");
	WriteHTMLLine(hXMLFile, "<MPEG-TABLES>");
	WriteHTMLLine(hXMLFile, " <TSREADER>");
	wsprintf(v->szSIFormatBuffer, "  <VERSION>%s</VERSION>", GetTSRVersion(NULL));
	WriteHTMLLine(hXMLFile, v->szSIFormatBuffer);
#ifdef LITE
	WriteHTMLLine(hXMLFile, "  <EDITION>LITE</EDITION>");
#else LITE
 #ifdef PRO
	WriteHTMLLine(hXMLFile, "  <EDITION>PROFESSIONAL</EDITION>");
 #else PRO
	WriteHTMLLine(hXMLFile, "  <EDITION>STANDARD</EDITION>");
 #endif PRO
#endif LITE
	WriteHTMLLine(hXMLFile, " </TSREADER>");

	// other stuff??
	WriteHTMLLine(hXMLFile, " <TUNED-MULTIPLEX>");
	wsprintf(v->szSIFormatBuffer, "  <TONE-ENABLED>%d</TONE-ENABLED>", v->ss.n22KHz); 	WriteHTMLLine(hXMLFile, v->szSIFormatBuffer);
	wsprintf(v->szSIFormatBuffer, "  <DISEQC-INPUT>%d</DISEQC-INPUT>", v->ss.nDiSEqCInput); 	WriteHTMLLine(hXMLFile, v->szSIFormatBuffer);
	wsprintf(v->szSIFormatBuffer, "  <LNB-FREQUENCY>%d</LNB-FREQUENCY>", v->ss.nLNBFrequency); 	WriteHTMLLine(hXMLFile, v->szSIFormatBuffer);
	wsprintf(v->szSIFormatBuffer, "  <FREQUENCY>%d</FREQUENCY>", v->ss.nFrequency); 	WriteHTMLLine(hXMLFile, v->szSIFormatBuffer);
	wsprintf(v->szSIFormatBuffer, "  <SYMBOL-RATE>%d</SYMBOL-RATE>", v->ss.nSymbolRate); 	WriteHTMLLine(hXMLFile, v->szSIFormatBuffer);
	wsprintf(v->szSIFormatBuffer, "  <POLARITY>%d</POLARITY>", v->ss.nPolarity); 	WriteHTMLLine(hXMLFile, v->szSIFormatBuffer);
	WriteHTMLLine(hXMLFile, " </TUNED-MULTIPLEX>");

	// Export the PAT
	WriteHTMLLine(hXMLFile, " <PAT>");
	wsprintf(v->szSIFormatBuffer, "  <VERSION>%d</VERSION>", v->pat.nVersionNumber); 	WriteHTMLLine(hXMLFile, v->szSIFormatBuffer);
	wsprintf(v->szSIFormatBuffer, "  <CRC-ERRORS>%d</CRC-ERRORS>", v->nSIParserCRCs[SI_PARSER_STATS_PAT]); 	WriteHTMLLine(hXMLFile, v->szSIFormatBuffer);
	wsprintf(v->szSIFormatBuffer, "  <TRANSPORT-STREAM-ID>%d</TRANSPORT-STREAM-ID>", v->pat.nTransportStreamID); 	WriteHTMLLine(hXMLFile, v->szSIFormatBuffer);
	for (nPMTIndex = 0; nPMTIndex < MAX_PAT_ENTRIES; nPMTIndex++)
	{
		if (v->pat.pmt[nPMTIndex].nPMTPID != 0)
		{
			if (v->pat.pmt[nPMTIndex].nProgramNumber != 0)
			{
				char szTemp[32];

				wsprintf(szTemp, "%d,", v->pat.pmt[nPMTIndex].nProgramNumber);
				lstrcat(szProgramNumbers, szTemp);
			}
			else
				nNetworkPID = v->pat.pmt[nPMTIndex].nPMTPID;
		}
	}
	if (lstrlen(szProgramNumbers))
	{
		if (szProgramNumbers[lstrlen(szProgramNumbers) - 1] == ',')
			szProgramNumbers[lstrlen(szProgramNumbers) - 1] = '\0';
		wsprintf(v->szSIFormatBuffer, "  <SERVICE-NUMBERS>%s</SERVICE-NUMBERS>", szProgramNumbers); 	WriteHTMLLine(hXMLFile, v->szSIFormatBuffer);
	}
	if (nNetworkPID != -1)
	{
		wsprintf(v->szSIFormatBuffer, "  <NETWORK-PID>%d</NETWORK-PID>", nNetworkPID); 	WriteHTMLLine(hXMLFile, v->szSIFormatBuffer);
	}
	WriteHTMLLine(hXMLFile, " </PAT>");

	// Export channels
	WriteHTMLLine(hXMLFile, " <PMTs>");
	wsprintf(v->szSIFormatBuffer, "  <CRC-ERRORS>%d</CRC-ERRORS>", v->nSIParserCRCs[SI_PARSER_STATS_PMT]); 	WriteHTMLLine(hXMLFile, v->szSIFormatBuffer);
	wsprintf(v->szSIFormatBuffer, "  <SDT-CRC-ERRORS>%d</SDT-CRC-ERRORS>", v->nSIParserCRCs[SI_PARSER_STATS_SDT]); 	WriteHTMLLine(hXMLFile, v->szSIFormatBuffer);
	wsprintf(v->szSIFormatBuffer, "  <SDT-VERSION>%d</SDT-VERSION>", v->nSIParserVersionNumbers[SI_PARSER_STATS_SDT]); 	WriteHTMLLine(hXMLFile, v->szSIFormatBuffer);
	for (nPMTIndex = 0; nPMTIndex < MAX_PAT_ENTRIES; nPMTIndex++)
	{
		if (v->pat.pmt[nPMTIndex].nPMTPID != 0)
		{
			if (v->pat.pmt[nPMTIndex].nProgramNumber != 0)
			{
				int nLCN = GetLogicalChannelNumber(v->pat.pmt[nPMTIndex].nProgramNumber);

				WriteHTMLLine(hXMLFile, " <CHANNEL>");
				wsprintf(v->szSIFormatBuffer, "   <SERVICE-NUMBER>%d</SERVICE-NUMBER>", v->pat.pmt[nPMTIndex].nProgramNumber); 	WriteHTMLLine(hXMLFile, v->szSIFormatBuffer);
				wsprintf(v->szSIFormatBuffer, "   <VERSION>%d</VERSION>", v->pat.pmt[nPMTIndex].nVersionNumber); 	WriteHTMLLine(hXMLFile, v->szSIFormatBuffer);
				if (nLCN != 0)
				{
					wsprintf(v->szSIFormatBuffer, "   <LCN>%d</LCN>", nLCN); 	WriteHTMLLine(hXMLFile, v->szSIFormatBuffer);			
				}
				wsprintf(v->szSIFormatBuffer, "   <PMT-PID>%d</PMT-PID>", v->pat.pmt[nPMTIndex].nPMTPID); 	WriteHTMLLine(hXMLFile, v->szSIFormatBuffer);
				wsprintf(v->szSIFormatBuffer, "   <PCR-PID>%d</PCR-PID>", v->pat.pmt[nPMTIndex].nPCRPID); 	WriteHTMLLine(hXMLFile, v->szSIFormatBuffer);

				if (v->pChannelData[v->pat.pmt[nPMTIndex].nProgramNumber] != NULL)
				{
					int nSDTDescriptors;
					int j;
					char szShortName[50];
					char szLongName[200];

					lstrcpy(szShortName, v->pChannelData[v->pat.pmt[nPMTIndex].nProgramNumber]->szShortName);
					EscapeReplaceXML(szShortName);
					lstrcpy(szLongName, v->pChannelData[v->pat.pmt[nPMTIndex].nProgramNumber]->szLongName);
					EscapeReplaceXML(szLongName);

					wsprintf(v->szSIFormatBuffer, "   <SHORT-NAME>%s</SHORT-NAME>", szShortName); 	WriteHTMLLine(hXMLFile, v->szSIFormatBuffer);
					wsprintf(v->szSIFormatBuffer, "   <LONG-NAME>%s</LONG-NAME>", szLongName); 	WriteHTMLLine(hXMLFile, v->szSIFormatBuffer);
					if (v->nNetworkPID == 0x0010)
					{
						wsprintf(v->szSIFormatBuffer, "   <RUNNING-STATUS>%d</RUNNING-STATUS>", v->pChannelData[v->pat.pmt[nPMTIndex].nProgramNumber]->nRunningStatus);	WriteHTMLLine(hXMLFile, v->szSIFormatBuffer);
						wsprintf(v->szSIFormatBuffer, "   <EIT-SCHEDULE>%d</EIT-SCHEDULE>", v->pChannelData[v->pat.pmt[nPMTIndex].nProgramNumber]->fEIT_schedule_flag);	WriteHTMLLine(hXMLFile, v->szSIFormatBuffer);
						wsprintf(v->szSIFormatBuffer, "   <EIT-PRESENT-FOLLOWING>%d</EIT-PRESENT-FOLLOWING>", v->pChannelData[v->pat.pmt[nPMTIndex].nProgramNumber]->fEIT_present_following_flag);	WriteHTMLLine(hXMLFile, v->szSIFormatBuffer);
						wsprintf(v->szSIFormatBuffer, "   <FREE-CA-MODE>%d</FREE-CA-MODE>", v->pChannelData[v->pat.pmt[nPMTIndex].nProgramNumber]->fFreeCAMode);	WriteHTMLLine(hXMLFile, v->szSIFormatBuffer);
					}

					nSDTDescriptors = 0;
					for (j = 0; j < MAX_SDT_EXTRA_DESCRIPTORS; j++)
					{
						if (v->pChannelData[v->pat.pmt[nPMTIndex].nProgramNumber]->pExtraDescriptors[j] != NULL)
							nSDTDescriptors++;
					}
					if (nSDTDescriptors)
					{
						WriteHTMLLine(hXMLFile, "   <SDT-DESCRIPTORS>");
						for (j = 0; j < MAX_SDT_EXTRA_DESCRIPTORS; j++)
						{
							if (v->pChannelData[v->pat.pmt[nPMTIndex].nProgramNumber]->pExtraDescriptors[j] != NULL)
							{
								if (v->pChannelData[v->pat.pmt[nPMTIndex].nProgramNumber]->pExtraDescriptors[j][0] == 0x48)
								{
									int nServiceType = v->pChannelData[v->pat.pmt[nPMTIndex].nProgramNumber]->pExtraDescriptors[j][2];
									char szServiceType[128];

									WriteHTMLLine(hXMLFile, "    <SERVICE>");
									DecodeServiceType(szServiceType, nServiceType);
									wsprintf(v->szSIFormatBuffer, "     <TYPE>%s</TYPE>", szServiceType);	WriteHTMLLine(hXMLFile, v->szSIFormatBuffer);
									wsprintf(v->szSIFormatBuffer, "     <TYPE-RAW>%d</TYPE-RAW>", nServiceType);	WriteHTMLLine(hXMLFile, v->szSIFormatBuffer);
									WriteHTMLLine(hXMLFile, "    </SERVICE>");
								}
								//else
								{
									int n;

									szDescriptorData[0] = '\0';
									WriteHTMLLine(hXMLFile, "    <DESCRIPTOR>");
									wsprintf(v->szSIFormatBuffer, "     <TAG>0x%02x</TAG>", v->pChannelData[v->pat.pmt[nPMTIndex].nProgramNumber]->pExtraDescriptors[j][0]); WriteHTMLLine(hXMLFile, v->szSIFormatBuffer);
									wsprintf(v->szSIFormatBuffer, "     <LENGTH>%d</LENGTH>", v->pChannelData[v->pat.pmt[nPMTIndex].nProgramNumber]->pExtraDescriptors[j][1]); WriteHTMLLine(hXMLFile, v->szSIFormatBuffer);
									for (n = 0; n < v->pChannelData[v->pat.pmt[nPMTIndex].nProgramNumber]->pExtraDescriptors[j][1]; n++)
									{
										char szTemp[8];
										wsprintf(szTemp, "0x%02x ", v->pChannelData[v->pat.pmt[nPMTIndex].nProgramNumber]->pExtraDescriptors[j][n + 2]);
										lstrcat(szDescriptorData, szTemp);
									}
									if (lstrlen(szDescriptorData))
									{
										StripTrailingSpaces(szDescriptorData);
										sprintf(v->szSIFormatBuffer, "     <DATA>%s</DATA>", szDescriptorData);
										WriteHTMLLine(hXMLFile, v->szSIFormatBuffer);
									}
									WriteHTMLLine(hXMLFile, "    </DESCRIPTOR>");
								}
							}
						}
						WriteHTMLLine(hXMLFile, "   </SDT-DESCRIPTORS>");
					}
				}

				for (i = 0; i < MAX_ESLIST_ENTRIES; i++)
				{
					if (v->pat.pmt[nPMTIndex].es[i].nESPID == 0)
						break;
					
					WriteHTMLLine(hXMLFile, "  <ELEMENTARY-STREAM>");
					
					wsprintf(v->szSIFormatBuffer, "   <INDEX>%d</INDEX>", i + 1); 	WriteHTMLLine(hXMLFile, v->szSIFormatBuffer);
					wsprintf(v->szSIFormatBuffer, "   <SCRAMBLED>%d</SCRAMBLED>", v->fPIDScrambled[v->pat.pmt[nPMTIndex].es[i].nESPID]); 	WriteHTMLLine(hXMLFile, v->szSIFormatBuffer);
					wsprintf(v->szSIFormatBuffer, "   <STREAM-TYPE-RAW>%d</STREAM-TYPE-RAW>", v->pat.pmt[nPMTIndex].es[i].nStreamType); 	WriteHTMLLine(hXMLFile, v->szSIFormatBuffer);
				
					switch(v->pat.pmt[nPMTIndex].es[i].nStreamType)
					{
					case 0x01:		// MPEG-1 video
					case 0x02:		// MPEG-2 video
					case 0x10:		// MPEG-4 video
					case 0x1b:		// H264 video
					case 0x80:		// Digicipher II video
					case 0xea:		// VC1
						WriteHTMLLine(hXMLFile, "   <STREAM-TYPE>VIDEO</STREAM-TYPE>");
						break;
					case 0x03:		// MPEG-1 audio
					case 0x04:		// MPEG-2 audio
					case 0x81:		// AC-3 audio
					case 0x83:		// LPCM
					case 0x85:		// DTS
					case 0x06:		// maybe AC3/PCM
						if (v->pat.pmt[nPMTIndex].es[i].nStreamType == 0x06)
						{
							if (   IsAC3AudioStream(nPMTIndex, i) == FALSE
								&& IsPCMAudioStream(nPMTIndex, i) == FALSE
								&& IsDTSAudioStream(nPMTIndex, i) == FALSE)
							{
								WriteHTMLLine(hXMLFile, "   <STREAM-TYPE>TELETEXT</STREAM-TYPE>");
								break;
							}
						}

						WriteHTMLLine(hXMLFile, "   <STREAM-TYPE>AUDIO</STREAM-TYPE>");
						if (v->pat.pmt[nPMTIndex].es[i].pDescriptors)
						{
							BYTE * pDescriptorData = v->pat.pmt[nPMTIndex].es[i].pDescriptors;
							int nDescriptorsLength = v->pat.pmt[nPMTIndex].es[i].nDescriptorsLength;
							int nCurrentIndex = 0;

							do
							{
								if (pDescriptorData[nCurrentIndex + 0] == 10)
								{
									// Language descriptor
									char szLanguage[4];

									szLanguage[0] = pDescriptorData[nCurrentIndex + 2];
									szLanguage[1] = pDescriptorData[nCurrentIndex + 3];
									szLanguage[2] = pDescriptorData[nCurrentIndex + 4];
									szLanguage[3] = 0;

									wsprintf(v->szSIFormatBuffer, "   <AUDIO-LANGUAGE>%s</AUDIO-LANGUAGE>", szLanguage); 	WriteHTMLLine(hXMLFile, v->szSIFormatBuffer);
									break;
								}
								nCurrentIndex += (BYTE)pDescriptorData[nCurrentIndex + 1]; // descriptor length
								nCurrentIndex += 2;	// descriptor tag and length
							} while (nCurrentIndex < nDescriptorsLength);
						}
						if (IsAC3AudioStream(nPMTIndex, i))		WriteHTMLLine(hXMLFile, "   <AUDIO-TYPE>AC3</AUDIO-TYPE>");
						else if (IsDTSAudioStream(nPMTIndex, i))		WriteHTMLLine(hXMLFile, "   <AUDIO-TYPE>DTS</AUDIO-TYPE>");
						else if (IsPCMAudioStream(nPMTIndex, i))		WriteHTMLLine(hXMLFile, "   <AUDIO-TYPE>PCM</AUDIO-TYPE>");
						else if (IsMPEGAudioStream(nPMTIndex, i))		WriteHTMLLine(hXMLFile, "   <AUDIO-TYPE>MPEG</AUDIO-TYPE>");
						break;
					default:
						wsprintf(v->szSIFormatBuffer, "   <STREAM-TYPE>%d</STREAM-TYPE>", v->pat.pmt[nPMTIndex].es[i].nStreamType); 	WriteHTMLLine(hXMLFile, v->szSIFormatBuffer);
						break;
					}
					wsprintf(v->szSIFormatBuffer, "   <PID>%d</PID>", v->pat.pmt[nPMTIndex].es[i].nESPID); 	WriteHTMLLine(hXMLFile, v->szSIFormatBuffer);

					XMLExport__WriteESParserData(hXMLFile, nPMTIndex, i);

					if (v->pat.pmt[nPMTIndex].es[i].pDescriptors != NULL)
					{
						int nDescriptorsLength = v->pat.pmt[nPMTIndex].es[i].nDescriptorsLength;
						int nCurrentIndex = 0;

						do
						{
							int j;

							WriteHTMLLine(hXMLFile, "   <DESCRIPTOR>");
							wsprintf(v->szSIFormatBuffer, "     <TAG>0x%02x</TAG>", v->pat.pmt[nPMTIndex].es[i].pDescriptors[nCurrentIndex]); WriteHTMLLine(hXMLFile, v->szSIFormatBuffer);
							wsprintf(v->szSIFormatBuffer, "     <LENGTH>%d</LENGTH>", v->pat.pmt[nPMTIndex].es[i].pDescriptors[nCurrentIndex + 1]); WriteHTMLLine(hXMLFile, v->szSIFormatBuffer);
							szDescriptorData[0] = '\0';
							for (j = 0; j < v->pat.pmt[nPMTIndex].es[i].pDescriptors[nCurrentIndex + 1]; j++)
							{
								char szTemp[8];
								wsprintf(szTemp, "0x%02x ", v->pat.pmt[nPMTIndex].es[i].pDescriptors[nCurrentIndex + j + 2]);
								lstrcat(szDescriptorData, szTemp);
							}
							if (lstrlen(szDescriptorData))
							{
								StripTrailingSpaces(szDescriptorData);
								sprintf(v->szSIFormatBuffer, "     <DATA>%s</DATA>", szDescriptorData);
								WriteHTMLLine(hXMLFile, v->szSIFormatBuffer);
							}						
							nCurrentIndex += (BYTE)v->pat.pmt[nPMTIndex].es[i].pDescriptors[nCurrentIndex + 1]; // descriptor length
							nCurrentIndex += 2;	// descriptor tag and length
							WriteHTMLLine(hXMLFile, "   </DESCRIPTOR>");
						} while (nCurrentIndex < nDescriptorsLength);
					}
					WriteHTMLLine(hXMLFile, "  </ELEMENTARY-STREAM>");
				}

				if (v->pat.pmt[nPMTIndex].nProgramInfoLength)
				{
					int nOffset;

					for (nOffset = 0; nOffset < v->pat.pmt[nPMTIndex].nProgramInfoLength;)
					{
						int nDescriptor = v->pat.pmt[nPMTIndex].pProgramInfo[nOffset];
						int nDescriptorLength = v->pat.pmt[nPMTIndex].pProgramInfo[nOffset + 1];
						int j;

						szDescriptorData[0] = '\0';
						WriteHTMLLine(hXMLFile, "  <DESCRIPTOR>");
						wsprintf(v->szSIFormatBuffer, "    <TAG>0x%02x</TAG>", nDescriptor); WriteHTMLLine(hXMLFile, v->szSIFormatBuffer);
						wsprintf(v->szSIFormatBuffer, "    <LENGTH>%d</LENGTH>", nDescriptorLength); WriteHTMLLine(hXMLFile, v->szSIFormatBuffer);
						for (j = 0; j < nDescriptorLength; j++)
						{
							char szTemp[8];
							wsprintf(szTemp, "0x%02x ", v->pat.pmt[nPMTIndex].pProgramInfo[nOffset + j + 2]);
							lstrcat(szDescriptorData, szTemp);
						}
						if (lstrlen(szDescriptorData))
						{
							StripTrailingSpaces(szDescriptorData);
							sprintf(v->szSIFormatBuffer, "    <DATA>%s</DATA>", szDescriptorData);
							WriteHTMLLine(hXMLFile, v->szSIFormatBuffer);
						}						
						nOffset += nDescriptorLength + 2;
						WriteHTMLLine(hXMLFile, "  </DESCRIPTOR>");
					}
				}
				WriteHTMLLine(hXMLFile, " </CHANNEL>");
			}
		}
	}
	WriteHTMLLine(hXMLFile, " </PMTs>");

	// Export the CAT
	{
		int nCASystemNumber = 1;

		WriteHTMLLine(hXMLFile, " <CAT>");
		wsprintf(v->szSIFormatBuffer, "  <CRC-ERRORS>%d</CRC-ERRORS>", v->nSIParserCRCs[SI_PARSER_STATS_CAT]); 	WriteHTMLLine(hXMLFile, v->szSIFormatBuffer);
		wsprintf(v->szSIFormatBuffer, "  <VERSION>%d</VERSION>", v->cat.nVersionNumber); 	WriteHTMLLine(hXMLFile, v->szSIFormatBuffer);

		for (i = 0; i < MAX_CAT_DESCRIPTORS; i++)
		{
			int nCASystemID;
			char szCAName[128] = {0};

			if (v->cat.pDescriptor[i] == NULL)
				break;

			{
				int j;

				szDescriptorData[0] = '\0';
				WriteHTMLLine(hXMLFile, "  <CAT-ENTRY>");
				wsprintf(v->szSIFormatBuffer, "   <INDEX>%d</INDEX>", nCASystemNumber++); 	WriteHTMLLine(hXMLFile, v->szSIFormatBuffer);
				if (v->cat.pDescriptor[i][0] == 0x09)
				{
					nCASystemID = (v->cat.pDescriptor[i][2] << 8) + v->cat.pDescriptor[i][3];
					FormatCASystemName(nCASystemID, szCAName);
					wsprintf(v->szSIFormatBuffer, "   <NAME>%s</NAME>", szCAName); 	WriteHTMLLine(hXMLFile, v->szSIFormatBuffer);
				}
				WriteHTMLLine(hXMLFile, "   <DESCRIPTOR>");
				wsprintf(v->szSIFormatBuffer, "    <TAG>0x%02x</TAG>", v->cat.pDescriptor[i][0]); WriteHTMLLine(hXMLFile, v->szSIFormatBuffer);
				wsprintf(v->szSIFormatBuffer, "    <LENGTH>%d</LENGTH>", v->cat.pDescriptor[i][1]); WriteHTMLLine(hXMLFile, v->szSIFormatBuffer);
				for (j = 0; j < v->cat.pDescriptor[i][1]; j++)
				{
					char szTemp[8];
					wsprintf(szTemp, "0x%02x ", v->cat.pDescriptor[i][j + 2]);
					lstrcat(szDescriptorData, szTemp);
				}
				if (lstrlen(szDescriptorData))
				{
					StripTrailingSpaces(szDescriptorData);
					sprintf(v->szSIFormatBuffer, "    <DATA>%s</DATA>", szDescriptorData);
					WriteHTMLLine(hXMLFile, v->szSIFormatBuffer);
				}
				WriteHTMLLine(hXMLFile, "   </DESCRIPTOR>");
				WriteHTMLLine(hXMLFile, "  </CAT-ENTRY>");
			}
		}
		WriteHTMLLine(hXMLFile, " </CAT>");
	}

	// Export the SDT
	if (v->nNetworkPID == 0x0010)
	{
		WriteHTMLLine(hXMLFile, " <SDT>");
		wsprintf(v->szSIFormatBuffer, "  <VERSION>%d</VERSION>", v->nSIParserVersionNumbers[SI_PARSER_STATS_SDT]); 	WriteHTMLLine(hXMLFile, v->szSIFormatBuffer);
		wsprintf(v->szSIFormatBuffer, "  <CRC-ERRORS>%d</CRC-ERRORS>", v->nSIParserCRCs[SI_PARSER_STATS_SDT]); 	WriteHTMLLine(hXMLFile, v->szSIFormatBuffer);
		WriteHTMLLine(hXMLFile,       "  <SDT-PID>17</SDT-PID>");

		for (i = 0; i < MAX_EIT_CHANNEL_DATA; i++)
		{
			if (v->pChannelData[i] != NULL)
			{
				WriteHTMLLine(hXMLFile, "  <SDT-ENTRY>");
				{
					int nSDTDescriptors;
					int j;
					char szShortName[50];
					char szLongName[200];

					lstrcpy(szShortName, v->pChannelData[i]->szShortName);
					EscapeReplaceXML(szShortName);
					lstrcpy(szLongName, v->pChannelData[i]->szLongName);
					EscapeReplaceXML(szLongName);

					wsprintf(v->szSIFormatBuffer, "   <SERVICE-ID>%d</SERVICE-ID>", i);	WriteHTMLLine(hXMLFile, v->szSIFormatBuffer);
					wsprintf(v->szSIFormatBuffer, "   <SHORT-NAME>%s</SHORT-NAME>", szShortName); 	WriteHTMLLine(hXMLFile, v->szSIFormatBuffer);
					wsprintf(v->szSIFormatBuffer, "   <LONG-NAME>%s</LONG-NAME>", szLongName); 	WriteHTMLLine(hXMLFile, v->szSIFormatBuffer);
					wsprintf(v->szSIFormatBuffer, "   <RUNNING-STATUS>%d</RUNNING-STATUS>", v->pChannelData[i]->nRunningStatus);	WriteHTMLLine(hXMLFile, v->szSIFormatBuffer);
					wsprintf(v->szSIFormatBuffer, "   <EIT-SCHEDULE>%d</EIT-SCHEDULE>", v->pChannelData[i]->fEIT_schedule_flag);	WriteHTMLLine(hXMLFile, v->szSIFormatBuffer);
					wsprintf(v->szSIFormatBuffer, "   <EIT-PRESENT-FOLLOWING>%d</EIT-PRESENT-FOLLOWING>", v->pChannelData[i]->fEIT_present_following_flag);	WriteHTMLLine(hXMLFile, v->szSIFormatBuffer);
					wsprintf(v->szSIFormatBuffer, "   <FREE-CA-MODE>%d</FREE-CA-MODE>", v->pChannelData[i]->fFreeCAMode);	WriteHTMLLine(hXMLFile, v->szSIFormatBuffer);
					wsprintf(v->szSIFormatBuffer, "   <TRANSPORT-STREAM-ID>%d</TRANSPORT-STREAM-ID>", v->pChannelData[i]->nTransportStreamID);	WriteHTMLLine(hXMLFile, v->szSIFormatBuffer);
					wsprintf(v->szSIFormatBuffer, "   <ORIGINAL-NETWORK-ID>%d</ORIGINAL-NETWORK-ID>", v->pChannelData[i]->nOriginalNetworkID);	WriteHTMLLine(hXMLFile, v->szSIFormatBuffer);
					wsprintf(v->szSIFormatBuffer, "   <ACTUAL-TRANSPORT-STREAM>%d</ACTUAL-TRANSPORT-STREAM>", v->pChannelData[i]->nFromTable == 0x42);	WriteHTMLLine(hXMLFile, v->szSIFormatBuffer);					

					nSDTDescriptors = 0;
					for (j = 0; j < MAX_SDT_EXTRA_DESCRIPTORS; j++)
					{
						if (v->pChannelData[i]->pExtraDescriptors[j] != NULL)
							nSDTDescriptors++;
					}
					if (nSDTDescriptors)
					{
						WriteHTMLLine(hXMLFile, "   <SDT-DESCRIPTORS>");
						for (j = 0; j < MAX_SDT_EXTRA_DESCRIPTORS; j++)
						{
							if (v->pChannelData[i]->pExtraDescriptors[j] != NULL)
							{
								if (v->pChannelData[i]->pExtraDescriptors[j][0] == 0x48)
								{
									int nServiceType = v->pChannelData[i]->pExtraDescriptors[j][2];
									char szServiceType[128];

									WriteHTMLLine(hXMLFile, "    <SERVICE>");
									DecodeServiceType(szServiceType, nServiceType);
									wsprintf(v->szSIFormatBuffer, "     <TYPE>%s</TYPE>", szServiceType);	WriteHTMLLine(hXMLFile, v->szSIFormatBuffer);
									wsprintf(v->szSIFormatBuffer, "     <TYPE-RAW>%d</TYPE-RAW>", nServiceType);	WriteHTMLLine(hXMLFile, v->szSIFormatBuffer);
									WriteHTMLLine(hXMLFile, "    </SERVICE>");
								}
								//else
								{
									int n;

									szDescriptorData[0] = '\0';
									WriteHTMLLine(hXMLFile, "    <DESCRIPTOR>");
									wsprintf(v->szSIFormatBuffer, "     <TAG>0x%02x</TAG>", v->pChannelData[i]->pExtraDescriptors[j][0]); WriteHTMLLine(hXMLFile, v->szSIFormatBuffer);
									wsprintf(v->szSIFormatBuffer, "     <LENGTH>%d</LENGTH>", v->pChannelData[i]->pExtraDescriptors[j][1]); WriteHTMLLine(hXMLFile, v->szSIFormatBuffer);
									for (n = 0; n < v->pChannelData[i]->pExtraDescriptors[j][1]; n++)
									{
										char szTemp[8];
										wsprintf(szTemp, "0x%02x ", v->pChannelData[i]->pExtraDescriptors[j][n + 2]);
										lstrcat(szDescriptorData, szTemp);
									}
									if (lstrlen(szDescriptorData))
									{
										StripTrailingSpaces(szDescriptorData);
										sprintf(v->szSIFormatBuffer, "     <DATA>%s</DATA>", szDescriptorData);
										WriteHTMLLine(hXMLFile, v->szSIFormatBuffer);
									}
									WriteHTMLLine(hXMLFile, "    </DESCRIPTOR>");
								}
							}
						}
						WriteHTMLLine(hXMLFile, "   </SDT-DESCRIPTORS>");
					}
				}
				WriteHTMLLine(hXMLFile, "  </SDT-ENTRY>");
			}
		}
		WriteHTMLLine(hXMLFile, " </SDT>");
	}

	// Export the BAT
	if (v->nNetworkPID == 0x0010)
	{
		int nBATCount = 0;
		for (i = 0; i < MAX_BAT_ENTRIES; i++)
		{
			if (v->bat[i].bouquet_id == 0)
				break;
			nBATCount++;
		}
		if (nBATCount)
		{
			WriteHTMLLine(hXMLFile, " <BAT>");
			wsprintf(v->szSIFormatBuffer, "  <VERSION>%d</VERSION>", v->nSIParserVersionNumbers[SI_PARSER_STATS_BAT]); 	WriteHTMLLine(hXMLFile, v->szSIFormatBuffer);
			//wsprintf(v->szSIFormatBuffer, "  <CRC-ERRORS>%d</CRC-ERRORS>", v->nSIParserCRCs[SI_PARSER_STATS_BAT]); 	WriteHTMLLine(hXMLFile, v->szSIFormatBuffer);
			WriteHTMLLine(hXMLFile, "  <BAT-PID>17</BAT-PID>");
			for (i = 0; i < MAX_BAT_ENTRIES; i++)
			{
				int j;
				char szBouquetName[128];

				if (v->bat[i].bouquet_id == 0)
					break;
				WriteHTMLLine(hXMLFile, "  <BAT-ENTRY>");
				GetBouquetName(i, szBouquetName);

				wsprintf(v->szSIFormatBuffer, "   <BOUQUET-ID>%d</BOUQUET-ID>", v->bat[i].bouquet_id);	WriteHTMLLine(hXMLFile, v->szSIFormatBuffer);
				wsprintf(v->szSIFormatBuffer, "   <VERSION>%d</VERSION>", v->bat[i].version_number);	WriteHTMLLine(hXMLFile, v->szSIFormatBuffer);
				wsprintf(v->szSIFormatBuffer, "   <NAME>%s</NAME>", szBouquetName);	WriteHTMLLine(hXMLFile, v->szSIFormatBuffer);
				if (v->bat[i].bouquet_descriptors_length)
				{
					int nLength = v->bat[i].bouquet_descriptors_length;
					BYTE * bouquet_descriptors = v->bat[i].bouquet_descriptors;
					while (nLength)
					{
						int nTag = bouquet_descriptors[0];
						int nThisDescriptorLength = bouquet_descriptors[1];
						int n;

						szDescriptorData[0] = '\0';
						WriteHTMLLine(hXMLFile, "   <DESCRIPTOR>");
						wsprintf(v->szSIFormatBuffer, "    <TAG>0x%02x</TAG>", nTag); WriteHTMLLine(hXMLFile, v->szSIFormatBuffer);
						wsprintf(v->szSIFormatBuffer, "    <LENGTH>%d</LENGTH>", nThisDescriptorLength); WriteHTMLLine(hXMLFile, v->szSIFormatBuffer);
						for (n = 0; n < nThisDescriptorLength; n++)
						{
							char szTemp[8];
							wsprintf(szTemp, "0x%02x ", bouquet_descriptors[n + 2]);
							lstrcat(szDescriptorData, szTemp);
						}
						if (lstrlen(szDescriptorData))
						{
							StripTrailingSpaces(szDescriptorData);
							sprintf(v->szSIFormatBuffer, "    <DATA>%s</DATA>", szDescriptorData);
							WriteHTMLLine(hXMLFile, v->szSIFormatBuffer);
						}
						WriteHTMLLine(hXMLFile, "   </DESCRIPTOR>");

						bouquet_descriptors += nThisDescriptorLength + 2;
						nLength -= nThisDescriptorLength + 2;
					}

				}
				for (j = 0; j < MAX_BAT_TRANSPORT_ITEMS; j++)
				{
					if (v->bat[i].batts[j].transport_stream_id == 0)
						break;
					WriteHTMLLine(hXMLFile, "   <BAT-TRANSPORT-STREAM>");
					wsprintf(v->szSIFormatBuffer, "    <TRANSPORT-STREAM-ID>%d</TRANSPORT-STREAM-ID>", v->bat[i].batts[j].transport_stream_id);	WriteHTMLLine(hXMLFile, v->szSIFormatBuffer);
					wsprintf(v->szSIFormatBuffer, "    <ORIGINAL-NETWORK-ID>%d</ORIGINAL-NETWORK-ID>", v->bat[i].batts[j].original_network_id);	WriteHTMLLine(hXMLFile, v->szSIFormatBuffer);			
					if (v->bat[i].batts[j].transport_descriptors_length)
					{
						int nLength = v->bat[i].batts[j].transport_descriptors_length;
						BYTE * bouquet_descriptors = v->bat[i].batts[j].transport_descriptors;
						while (nLength)
						{
							int nTag = bouquet_descriptors[0];
							int nThisDescriptorLength = bouquet_descriptors[1];
							int n;

							if (nTag == 0x41)
							{
								// Special case for service list descriptor
								int nThisDescriptorLengthCopy = nThisDescriptorLength;
								BYTE * pServiceListDescriptor = &bouquet_descriptors[2];

								WriteHTMLLine(hXMLFile, "   <SERVICE-LIST>");
								while (nThisDescriptorLengthCopy > 0)
								{
									int nServiceID = pServiceListDescriptor[0] << 8 | pServiceListDescriptor[1];
									int nServiceType = pServiceListDescriptor[2];
									char szServiceType[64];

									WriteHTMLLine(hXMLFile, "    <SERVICE>");
									DecodeServiceType(szServiceType, nServiceType);
									wsprintf(v->szSIFormatBuffer, "     <ID>%d</ID>", nServiceID);	WriteHTMLLine(hXMLFile, v->szSIFormatBuffer);
									wsprintf(v->szSIFormatBuffer, "     <TYPE>%s</TYPE>", szServiceType);	WriteHTMLLine(hXMLFile, v->szSIFormatBuffer);
									wsprintf(v->szSIFormatBuffer, "     <TYPE-RAW>%d</TYPE-RAW>", nServiceType);	WriteHTMLLine(hXMLFile, v->szSIFormatBuffer);
									WriteHTMLLine(hXMLFile, "    </SERVICE>");
									pServiceListDescriptor += 3;
									nThisDescriptorLengthCopy -= 3;
								}
								WriteHTMLLine(hXMLFile, "   </SERVICE-LIST>");
							}
							//else
							{
								szDescriptorData[0] = '\0';
								WriteHTMLLine(hXMLFile, "   <DESCRIPTOR>");
								wsprintf(v->szSIFormatBuffer, "    <TAG>0x%02x</TAG>", nTag); WriteHTMLLine(hXMLFile, v->szSIFormatBuffer);
								wsprintf(v->szSIFormatBuffer, "    <LENGTH>%d</LENGTH>", nThisDescriptorLength); WriteHTMLLine(hXMLFile, v->szSIFormatBuffer);
								for (n = 0; n < nThisDescriptorLength; n++)
								{
									char szTemp[8];
									wsprintf(szTemp, "0x%02x ", bouquet_descriptors[n + 2]);
									lstrcat(szDescriptorData, szTemp);
								}
								if (lstrlen(szDescriptorData))
								{
									StripTrailingSpaces(szDescriptorData);
									sprintf(v->szSIFormatBuffer, "    <DATA>%s</DATA>", szDescriptorData);
									WriteHTMLLine(hXMLFile, v->szSIFormatBuffer);
								}
								WriteHTMLLine(hXMLFile, "   </DESCRIPTOR>");
							}
							bouquet_descriptors += nThisDescriptorLength + 2;
							nLength -= nThisDescriptorLength + 2;
						}
					}
					WriteHTMLLine(hXMLFile, "   </BAT-TRANSPORT-STREAM>");
				}
				WriteHTMLLine(hXMLFile, "  </BAT-ENTRY>");
			}
			WriteHTMLLine(hXMLFile, " </BAT>");
		}
	}

	// Export NIT on DVB networks
	if (v->nNetworkPID == 0x0010)
	{
		int nNITIndex;
		BOOL fWrittenNetworkIDandName = FALSE;
		int nNetworkEntry = 1;

		WriteHTMLLine(hXMLFile, " <NIT>");
		wsprintf(v->szSIFormatBuffer, "  <CRC-ERRORS>%d</CRC-ERRORS>", v->nSIParserCRCs[SI_PARSER_STATS_NIT]); 	WriteHTMLLine(hXMLFile, v->szSIFormatBuffer);
		wsprintf(v->szSIFormatBuffer, "  <VERSION>%d</VERSION>", v->nSIParserVersionNumbers[SI_PARSER_STATS_NIT]); 	WriteHTMLLine(hXMLFile, v->szSIFormatBuffer);
		WriteHTMLLine(hXMLFile, "  <NIT-PID>16</NIT-PID>");
		for (nNITIndex = 0; nNITIndex < MAX_TS_ENTRIES; nNITIndex++)
		{
			if (v->pNITData[nNITIndex] != NULL)
			{
				int nExtraDescriptorIndex;
				int nTSDescriptors;
				char szTemp[128];

				WriteHTMLLine(hXMLFile, " <NIT-ENTRY>");
				wsprintf(v->szSIFormatBuffer, "   <INDEX>%d</INDEX>", nNetworkEntry++); 	WriteHTMLLine(hXMLFile, v->szSIFormatBuffer);
				wsprintf(v->szSIFormatBuffer, "   <NETWORK-ID>%d</NETWORK-ID>", v->pNITData[nNITIndex]->nNetworkID); 	WriteHTMLLine(hXMLFile, v->szSIFormatBuffer);
				wsprintf(v->szSIFormatBuffer, "   <TRANSPORT-STREAM-ID>%d</TRANSPORT-STREAM-ID>", v->pNITData[nNITIndex]->nTransportStreamID); 	WriteHTMLLine(hXMLFile, v->szSIFormatBuffer);
				wsprintf(v->szSIFormatBuffer, "   <ORIGINAL-NETWORK-ID>%d</ORIGINAL-NETWORK-ID>", v->pNITData[nNITIndex]->nOriginalNetworkID); 	WriteHTMLLine(hXMLFile, v->szSIFormatBuffer);
				wsprintf(v->szSIFormatBuffer, "   <THIS-NETWORK-INDICATOR>%d</THIS-NETWORK-INDICATOR>", v->pNITData[nNITIndex]->fThisTS); 	WriteHTMLLine(hXMLFile, v->szSIFormatBuffer);
				wsprintf(v->szSIFormatBuffer, "   <NETWORK-NAME>%s</NETWORK-NAME>", v->pNITData[nNITIndex]->szNetworkName); 	WriteHTMLLine(hXMLFile, v->szSIFormatBuffer);

				if (v->pNITData[nNITIndex]->nNetworkDescriptorsLength)
				{
					int nLength = v->pNITData[nNITIndex]->nNetworkDescriptorsLength;
					BYTE * network_descriptors = v->pNITData[nNITIndex]->pNetworkDescriptors;

					WriteHTMLLine(hXMLFile, "  <NETWORK-DESCRIPTORS>");
					while (nLength)
					{
						int nTag = network_descriptors[0];
						int nThisDescriptorLength = network_descriptors[1];
						int n;

						szDescriptorData[0] = '\0';
						WriteHTMLLine(hXMLFile, "   <DESCRIPTOR>");
						wsprintf(v->szSIFormatBuffer, "    <TAG>0x%02x</TAG>", nTag); WriteHTMLLine(hXMLFile, v->szSIFormatBuffer);
						wsprintf(v->szSIFormatBuffer, "    <LENGTH>%d</LENGTH>", nThisDescriptorLength); WriteHTMLLine(hXMLFile, v->szSIFormatBuffer);
						for (n = 0; n < nThisDescriptorLength; n++)
						{
							char szTemp[8];
							wsprintf(szTemp, "0x%02x ", network_descriptors[n + 2]);
							lstrcat(szDescriptorData, szTemp);
						}
						if (lstrlen(szDescriptorData))
						{
							StripTrailingSpaces(szDescriptorData);
							sprintf(v->szSIFormatBuffer, "    <DATA>%s</DATA>", szDescriptorData);
							WriteHTMLLine(hXMLFile, v->szSIFormatBuffer);
						}
						WriteHTMLLine(hXMLFile, "   </DESCRIPTOR>");

						network_descriptors += nThisDescriptorLength + 2;
						nLength -= nThisDescriptorLength + 2;
					}
					WriteHTMLLine(hXMLFile, "  </NETWORK-DESCRIPTORS>");
				}

				// HANDLE DVB-C/DVB-T
				switch(v->pNITData[nNITIndex]->nType)
				{
				case NIT_DVBS:
					{
						char szPolarity[32];
						char szModulation[64] = {0};
						char szCodeRate[16];

						WriteHTMLLine(hXMLFile, "   <NETWORK-TYPE>DVB-S</NETWORK-TYPE>");

						FormatPolarity(szPolarity, v->pNITData[nNITIndex]->dvbs.nPolarization, TRUE);
						FormatDVBSModulation(szModulation, v->pNITData[nNITIndex]->dvbs.nModulation);
						FormatDVBSCodeRate(szCodeRate, v->pNITData[nNITIndex]->dvbs.nFEC);

						sprintf(szTemp, "%5.2f", (double)v->pNITData[nNITIndex]->nFrequency / 100.0);
						wsprintf(v->szSIFormatBuffer, "   <FREQUENCY>%s</FREQUENCY>", szTemp); 	WriteHTMLLine(hXMLFile, v->szSIFormatBuffer);
						wsprintf(v->szSIFormatBuffer, "   <RAW-FREQUENCY>%lu</RAW-FREQUENCY>", v->pNITData[nNITIndex]->nFrequency); 	WriteHTMLLine(hXMLFile, v->szSIFormatBuffer);
						if (v->pNITData[nNITIndex]->dvbs.fEastern)
							sprintf(szTemp, "%3.1f E", (double)v->pNITData[nNITIndex]->dvbs.nOrbitalPosition / 10.0);
						else
							sprintf(szTemp, "%3.1f W", (double)v->pNITData[nNITIndex]->dvbs.nOrbitalPosition / 10.0);
						wsprintf(v->szSIFormatBuffer, "   <EAST-WEST-FLAG>%d</EAST-WEST-FLAG>", v->pNITData[nNITIndex]->dvbs.fEastern); 	WriteHTMLLine(hXMLFile, v->szSIFormatBuffer);
						wsprintf(v->szSIFormatBuffer, "   <ORBITAL-POSITION>%s</ORBITAL-POSITION>", szTemp); 	WriteHTMLLine(hXMLFile, v->szSIFormatBuffer);
						wsprintf(v->szSIFormatBuffer, "   <ORBITAL-POSITION-RAW>%d</ORBITAL-POSITION-RAW>", v->pNITData[nNITIndex]->dvbs.nOrbitalPosition); 	WriteHTMLLine(hXMLFile, v->szSIFormatBuffer);
						wsprintf(v->szSIFormatBuffer, "   <POLARITY>%s</POLARITY>", szPolarity); 	WriteHTMLLine(hXMLFile, v->szSIFormatBuffer);
						wsprintf(v->szSIFormatBuffer, "   <POLARITY-RAW>%d</POLARITY-RAW>", v->pNITData[nNITIndex]->dvbs.nPolarization); 	WriteHTMLLine(hXMLFile, v->szSIFormatBuffer);
						wsprintf(v->szSIFormatBuffer, "   <MODULATION>%s</MODULATION>", szModulation); 	WriteHTMLLine(hXMLFile, v->szSIFormatBuffer);
						wsprintf(v->szSIFormatBuffer, "   <MODULATION-RAW>%d</MODULATION-RAW>", v->pNITData[nNITIndex]->dvbs.nModulation); 	WriteHTMLLine(hXMLFile, v->szSIFormatBuffer);
						wsprintf(v->szSIFormatBuffer, "   <SYMBOL-RATE>%d</SYMBOL-RATE>", v->pNITData[nNITIndex]->dvbs.nSymbolRate / 10); 	WriteHTMLLine(hXMLFile, v->szSIFormatBuffer);
						wsprintf(v->szSIFormatBuffer, "   <RAW-SYMBOL-RATE>%d</RAW-SYMBOL-RATE>", v->pNITData[nNITIndex]->dvbs.nSymbolRate); 	WriteHTMLLine(hXMLFile, v->szSIFormatBuffer);
						wsprintf(v->szSIFormatBuffer, "   <CODE-RATE>%s</CODE-RATE>", szCodeRate); 	WriteHTMLLine(hXMLFile, v->szSIFormatBuffer);
						wsprintf(v->szSIFormatBuffer, "   <CODE-RATE-RAW>%d</CODE-RATE-RAW>", v->pNITData[nNITIndex]->dvbs.nFEC); 	WriteHTMLLine(hXMLFile, v->szSIFormatBuffer);
					}
					break;
				case NIT_DVBT:
					{
						char szBandwidth[16];
						char szConstellation[32];
						char szHierarchyInformation[32];
						char szFECHP[16];
						char szFECLP[16];
						char szGuardInterval[32];
						char szTransmissionMode[32];

						WriteHTMLLine(hXMLFile, "   <NETWORK-TYPE>DVB-T</NETWORK-TYPE>");

						FormatDVBTBandwidth(szBandwidth, v->pNITData[nNITIndex]->dvbt.nBandwidth);
						FormatDVBTConstellation(szConstellation, v->pNITData[nNITIndex]->dvbt.nConstellation);
						FormatDVBTHierarchyInformation(szHierarchyInformation, v->pNITData[nNITIndex]->dvbt.nHierarchyInformation);
						FormatDVBTCodeRate(szFECHP, v->pNITData[nNITIndex]->dvbt.nCodeRateHPStream);
						FormatDVBTCodeRate(szFECLP, v->pNITData[nNITIndex]->dvbt.nCodeRateLPStream);
						FormatDVBTGuardInterval(szGuardInterval, v->pNITData[nNITIndex]->dvbt.nGuardInterval);
						FormatDVBTTransmissionMode(szTransmissionMode, v->pNITData[nNITIndex]->dvbt.nTransmissionMode);

						sprintf(szTemp, "%5.2f", (double)v->pNITData[nNITIndex]->nFrequency / 100000.0);
						wsprintf(v->szSIFormatBuffer, "   <FREQUENCY>%s</FREQUENCY>", szTemp); 	WriteHTMLLine(hXMLFile, v->szSIFormatBuffer);
						wsprintf(v->szSIFormatBuffer, "   <RAW-FREQUENCY>%lu</RAW-FREQUENCY>", v->pNITData[nNITIndex]->nFrequency); 	WriteHTMLLine(hXMLFile, v->szSIFormatBuffer);
						wsprintf(v->szSIFormatBuffer, "   <BANDWIDTH>%s</BANDWIDTH>", szBandwidth); 	WriteHTMLLine(hXMLFile, v->szSIFormatBuffer);
						wsprintf(v->szSIFormatBuffer, "   <BANDWIDTH-RAW>%d</BANDWIDTH-RAW>", v->pNITData[nNITIndex]->dvbt.nBandwidth); 	WriteHTMLLine(hXMLFile, v->szSIFormatBuffer);
						wsprintf(v->szSIFormatBuffer, "   <CONSTELATION>%s</CONSTELATION>", szConstellation); 	WriteHTMLLine(hXMLFile, v->szSIFormatBuffer);
						wsprintf(v->szSIFormatBuffer, "   <CONSTELATION-RAW>%d</CONSTELATION-RAW>", v->pNITData[nNITIndex]->dvbt.nConstellation); 	WriteHTMLLine(hXMLFile, v->szSIFormatBuffer);
						wsprintf(v->szSIFormatBuffer, "   <HIERARCHY-INFORMATION>%s</HIERARCHY-INFORMATION>", szHierarchyInformation); 	WriteHTMLLine(hXMLFile, v->szSIFormatBuffer);
						wsprintf(v->szSIFormatBuffer, "   <HIERARCHY-INFORMATION-RAW>%d</HIERARCHY-INFORMATION-RAW>", v->pNITData[nNITIndex]->dvbt.nHierarchyInformation); 	WriteHTMLLine(hXMLFile, v->szSIFormatBuffer);
						wsprintf(v->szSIFormatBuffer, "   <FEC-HP>%s</FEC-HP>", szFECHP); 	WriteHTMLLine(hXMLFile, v->szSIFormatBuffer);
						wsprintf(v->szSIFormatBuffer, "   <FEC-HP-RAW>%d</FEC-HP-RAW>", v->pNITData[nNITIndex]->dvbt.nCodeRateHPStream); 	WriteHTMLLine(hXMLFile, v->szSIFormatBuffer);
						wsprintf(v->szSIFormatBuffer, "   <FEC-LP>%s</FEC-LP>", szFECLP); 	WriteHTMLLine(hXMLFile, v->szSIFormatBuffer);
						wsprintf(v->szSIFormatBuffer, "   <FEC-LP-RAW>%d</FEC-LP-RAW>", v->pNITData[nNITIndex]->dvbt.nCodeRateLPStream); 	WriteHTMLLine(hXMLFile, v->szSIFormatBuffer);
						wsprintf(v->szSIFormatBuffer, "   <GUARD-INTERVAL>%s</GUARD-INTERVAL>", szGuardInterval); 	WriteHTMLLine(hXMLFile, v->szSIFormatBuffer);
						wsprintf(v->szSIFormatBuffer, "   <GUARD-INTERVAL-RAW>%d</GUARD-INTERVAL-RAW>", v->pNITData[nNITIndex]->dvbt.nGuardInterval); 	WriteHTMLLine(hXMLFile, v->szSIFormatBuffer);
						wsprintf(v->szSIFormatBuffer, "   <TRANSMISSION-MODE>%s</TRANSMISSION-MODE>", szTransmissionMode); 	WriteHTMLLine(hXMLFile, v->szSIFormatBuffer);
						wsprintf(v->szSIFormatBuffer, "   <TRANSMISSION-MODE-RAW>%d</TRANSMISSION-MODE-RAW>", v->pNITData[nNITIndex]->dvbt.nTransmissionMode); 	WriteHTMLLine(hXMLFile, v->szSIFormatBuffer);
						wsprintf(v->szSIFormatBuffer, "   <OTHER-FREQUENCY-FLAG>%d</OTHER-FREQUENCY-FLAG>", v->pNITData[nNITIndex]->dvbt.nOtherFrequencyFlag); 	WriteHTMLLine(hXMLFile, v->szSIFormatBuffer);
					}
					break;
				case NIT_DVBC:
					{
						char szModulation[32];
						char szFECOuter[32];
						char szFECInner[32];

						WriteHTMLLine(hXMLFile, "   <NETWORK-TYPE>DVB-C</NETWORK-TYPE>");

						FormatDVBCModulation(szModulation, v->pNITData[nNITIndex]->dvbc.nModulation);
						FormatDVBCOuterFEC(szFECOuter, v->pNITData[nNITIndex]->dvbc.nFECOuter);
						FormatDVBCInnerFEC(szFECInner, v->pNITData[nNITIndex]->dvbc.nFECInner);

						sprintf(szTemp, "%5.2f", (double)v->pNITData[nNITIndex]->nFrequency / 10000.0);
						wsprintf(v->szSIFormatBuffer, "   <FREQUENCY>%s</FREQUENCY>", szTemp); 	WriteHTMLLine(hXMLFile, v->szSIFormatBuffer);
						wsprintf(v->szSIFormatBuffer, "   <RAW-FREQUENCY>%lu</RAW-FREQUENCY>", v->pNITData[nNITIndex]->nFrequency); 	WriteHTMLLine(hXMLFile, v->szSIFormatBuffer);
						wsprintf(v->szSIFormatBuffer, "   <SYMBOL-RATE>%d</SYMBOL-RATE>", v->pNITData[nNITIndex]->dvbc.nSymbolRate / 10); 	WriteHTMLLine(hXMLFile, v->szSIFormatBuffer);
						wsprintf(v->szSIFormatBuffer, "   <RAW-SYMBOL-RATE>%d</RAW-SYMBOL-RATE>", v->pNITData[nNITIndex]->dvbc.nSymbolRate); 	WriteHTMLLine(hXMLFile, v->szSIFormatBuffer);
						wsprintf(v->szSIFormatBuffer, "   <MODULATION>%s</MODULATION>", szModulation); 	WriteHTMLLine(hXMLFile, v->szSIFormatBuffer);
						wsprintf(v->szSIFormatBuffer, "   <MODULATION-RAW>%d</MODULATION-RAW>", v->pNITData[nNITIndex]->dvbc.nModulation); 	WriteHTMLLine(hXMLFile, v->szSIFormatBuffer);
						wsprintf(v->szSIFormatBuffer, "   <OUTER-FEC>%s</OUTER-FEC>", szFECOuter); 	WriteHTMLLine(hXMLFile, v->szSIFormatBuffer);
						wsprintf(v->szSIFormatBuffer, "   <OUTER-FEC-RAW>%d</OUTER-FEC-RAW>", v->pNITData[nNITIndex]->dvbc.nFECOuter); 	WriteHTMLLine(hXMLFile, v->szSIFormatBuffer);
						wsprintf(v->szSIFormatBuffer, "   <INNER-FEC>%s</INNER-FEC>", szFECInner); 	WriteHTMLLine(hXMLFile, v->szSIFormatBuffer);
						wsprintf(v->szSIFormatBuffer, "   <INNER-FEC-RAW>%d</INNER-FEC-RAW>", v->pNITData[nNITIndex]->dvbc.nFECInner); 	WriteHTMLLine(hXMLFile, v->szSIFormatBuffer);
					}
					break;
				}

				nTSDescriptors = 0;
				for (nExtraDescriptorIndex = 0; nExtraDescriptorIndex < MAX_NIT_EXTRA_DESCRIPTORS; nExtraDescriptorIndex++)
				{
					if (v->pNITData[nNITIndex]->pExtraDescriptors[nExtraDescriptorIndex] != NULL)
						nTSDescriptors++;
				}
				if (nTSDescriptors)
				{
					WriteHTMLLine(hXMLFile, "  <TS-DESCRIPTORS>");
					for (nExtraDescriptorIndex = 0; nExtraDescriptorIndex < MAX_NIT_EXTRA_DESCRIPTORS; nExtraDescriptorIndex++)
					{
						if (v->pNITData[nNITIndex]->pExtraDescriptors[nExtraDescriptorIndex] != NULL)
						{
							int j;
							BYTE * pDescriptor = v->pNITData[nNITIndex]->pExtraDescriptors[nExtraDescriptorIndex];

							if (*pDescriptor == 0x41)
							{
								// Special case for service list descriptor
								int nDescriptorLength = v->pNITData[nNITIndex]->pExtraDescriptors[nExtraDescriptorIndex][1] - 2;
								BYTE * pServiceListDescriptor = &v->pNITData[nNITIndex]->pExtraDescriptors[nExtraDescriptorIndex][2];

								WriteHTMLLine(hXMLFile, "   <SERVICE-LIST>");
								while (nDescriptorLength > 0)
								{
									int nServiceID = pServiceListDescriptor[0] << 8 | pServiceListDescriptor[1];
									int nServiceType = pServiceListDescriptor[2];
									char szServiceType[64];

									WriteHTMLLine(hXMLFile, "    <SERVICE>");
									DecodeServiceType(szServiceType, nServiceType);
									wsprintf(v->szSIFormatBuffer, "     <ID>%d</ID>", nServiceID);	WriteHTMLLine(hXMLFile, v->szSIFormatBuffer);
									wsprintf(v->szSIFormatBuffer, "     <TYPE>%s</TYPE>", szServiceType);	WriteHTMLLine(hXMLFile, v->szSIFormatBuffer);
									wsprintf(v->szSIFormatBuffer, "     <TYPE-RAW>%d</TYPE-RAW>", nServiceType);	WriteHTMLLine(hXMLFile, v->szSIFormatBuffer);
									WriteHTMLLine(hXMLFile, "    </SERVICE>");
									pServiceListDescriptor += 3;
									nDescriptorLength -= 3;
								}
								WriteHTMLLine(hXMLFile, "   </SERVICE-LIST>");
							}
							//else
							{
								szDescriptorData[0] = '\0';
								WriteHTMLLine(hXMLFile, "   <DESCRIPTOR>");
								wsprintf(v->szSIFormatBuffer, "    <TAG>0x%02x</TAG>", pDescriptor[0]); WriteHTMLLine(hXMLFile, v->szSIFormatBuffer);
								wsprintf(v->szSIFormatBuffer, "    <LENGTH>%d</LENGTH>", pDescriptor[1]); WriteHTMLLine(hXMLFile, v->szSIFormatBuffer);
								for (j = 0; j < pDescriptor[1]; j++)
								{
									char szTemp[8];
									wsprintf(szTemp, "0x%02x ", pDescriptor[j + 2]);
									lstrcat(szDescriptorData, szTemp);
								}
								if (lstrlen(szDescriptorData))
								{
									StripTrailingSpaces(szDescriptorData);
									sprintf(v->szSIFormatBuffer, "    <DATA>%s</DATA>", szDescriptorData);
									WriteHTMLLine(hXMLFile, v->szSIFormatBuffer);
								}
								WriteHTMLLine(hXMLFile, "   </DESCRIPTOR>");
							}
						}
					}
					WriteHTMLLine(hXMLFile, "  </TS-DESCRIPTORS>");
				}
				WriteHTMLLine(hXMLFile, " </NIT-ENTRY>");
			}
		}
		WriteHTMLLine(hXMLFile, " </NIT>");
	}
	else if (v->nNetworkPID == 0x1ffb)
	{
		// TVCT for ATSC networks
		int nServiceIndex;
		
		WriteHTMLLine(hXMLFile, " <TVCT>");
		wsprintf(v->szSIFormatBuffer, "  <CRC-ERRORS>%d</CRC-ERRORS>", v->nSIParserCRCs[SI_PARSER_STATS_NIT]); 	WriteHTMLLine(hXMLFile, v->szSIFormatBuffer);
		wsprintf(v->szSIFormatBuffer, "  <VERSION>%d</VERSION>", v->nSIParserVersionNumbers[SI_PARSER_STATS_NIT]); 	WriteHTMLLine(hXMLFile, v->szSIFormatBuffer);
		for (nServiceIndex = 0; nServiceIndex < MAX_EIT_CHANNEL_DATA; nServiceIndex++)
		{
			if (v->pChannelData[nServiceIndex] != NULL)
			{
				int nTSIDPtr = -1;
				int nTSIDIndex;
				int nDescriptorIndex;
				char szModulationMode[48];

				WriteHTMLLine(hXMLFile, "  <CHANNEL>");
				for (nTSIDIndex = 0; tsid[nTSIDIndex].nNTSCTSID != 0; nTSIDIndex++)
				{
					if (tsid[nTSIDIndex].nNTSCTSID + 1 == v->pChannelData[nServiceIndex]->nTransportStreamID)
					{
						nTSIDPtr = nTSIDIndex;
						break;
					}
				}
				FormatATSCModulationMode(szModulationMode, v->pChannelData[nServiceIndex]->nModulationMode);

				wsprintf(v->szSIFormatBuffer, "   <PROGRAM>%d</PROGRAM>", v->pChannelData[nServiceIndex]->nChannelNumber); WriteHTMLLine(hXMLFile, v->szSIFormatBuffer);
				wsprintf(v->szSIFormatBuffer, "   <NAME>%s</NAME>", v->pChannelData[nServiceIndex]->szShortName); WriteHTMLLine(hXMLFile, v->szSIFormatBuffer);
				wsprintf(v->szSIFormatBuffer, "   <TSID>%d</TSID>", v->pChannelData[nServiceIndex]->nTransportStreamID); WriteHTMLLine(hXMLFile, v->szSIFormatBuffer);
				if (nTSIDPtr != -1)
				{
					wsprintf(v->szSIFormatBuffer, "   <NTSC-CHANNEL>%d</NTSC-CHANNEL>", tsid[nTSIDIndex].nNTSCChannel); WriteHTMLLine(hXMLFile, v->szSIFormatBuffer);
					wsprintf(v->szSIFormatBuffer, "   <ATSC-CHANNEL>%d</ATSC-CHANNEL>", tsid[nTSIDIndex].nATSCChannel); WriteHTMLLine(hXMLFile, v->szSIFormatBuffer);
					wsprintf(v->szSIFormatBuffer, "   <LOCALE>%s</LOCALE>", tsid[nTSIDIndex].szLocale); WriteHTMLLine(hXMLFile, v->szSIFormatBuffer);
				}
				wsprintf(v->szSIFormatBuffer, "   <MAJOR-CHANNEL>%d</MAJOR-CHANNEL>", v->pChannelData[nServiceIndex]->nMajorChannelNumber); WriteHTMLLine(hXMLFile, v->szSIFormatBuffer);
				wsprintf(v->szSIFormatBuffer, "   <MINOR-CHANNEL>%d</MINOR-CHANNEL>", v->pChannelData[nServiceIndex]->nMinorChannelNumber); WriteHTMLLine(hXMLFile, v->szSIFormatBuffer);
				wsprintf(v->szSIFormatBuffer, "   <SOURCE-ID>%d</SOURCE-ID>", v->pChannelData[nServiceIndex]->nSourceID); WriteHTMLLine(hXMLFile, v->szSIFormatBuffer);
				wsprintf(v->szSIFormatBuffer, "   <MODULATION>%s</MODULATION>", szModulationMode); WriteHTMLLine(hXMLFile, v->szSIFormatBuffer);
				wsprintf(v->szSIFormatBuffer, "   <CARRIER-FREQUENCY>%d</CARRIER-FREQUENCY>", v->pChannelData[nServiceIndex]->nCarrierFrequency); WriteHTMLLine(hXMLFile, v->szSIFormatBuffer);
				for (nDescriptorIndex = 0; nDescriptorIndex < MAX_SDT_EXTRA_DESCRIPTORS; nDescriptorIndex++)
				{
					if (v->pChannelData[nServiceIndex]->pExtraDescriptors[nDescriptorIndex] != NULL)
					{
						int j;
						BYTE * pDescriptor = v->pChannelData[nServiceIndex]->pExtraDescriptors[nDescriptorIndex];

						szDescriptorData[0] = '\0';
						WriteHTMLLine(hXMLFile, "   <DESCRIPTOR>");
						wsprintf(v->szSIFormatBuffer, "    <TAG>0x%02x</TAG>", pDescriptor[0]); WriteHTMLLine(hXMLFile, v->szSIFormatBuffer);
						wsprintf(v->szSIFormatBuffer, "    <LENGTH>%d</LENGTH>", pDescriptor[1]); WriteHTMLLine(hXMLFile, v->szSIFormatBuffer);
						for (j = 0; j < pDescriptor[1]; j++)
						{
							char szTemp[8];
							wsprintf(szTemp, "0x%02x ", pDescriptor[j + 2]);
							lstrcat(szDescriptorData, szTemp);
						}
						if (lstrlen(szDescriptorData))
						{
							StripTrailingSpaces(szDescriptorData);
							sprintf(v->szSIFormatBuffer, "    <DATA>%s</DATA>", szDescriptorData);
							WriteHTMLLine(hXMLFile, v->szSIFormatBuffer);
						}
						WriteHTMLLine(hXMLFile, "   </DESCRIPTOR>");
					}
				}				
				WriteHTMLLine(hXMLFile, "  </CHANNEL>");
			}
		}
		WriteHTMLLine(hXMLFile, " </TVCT>");
	}

	// Time tables
	if (v->nNetworkPID == 0x0010)
	{
		// TDT/TOT for DVB
		if (v->dvbtdt.hTreeItem != NULL)
		{
			WriteHTMLLine(hXMLFile, " <TDT>");
			wsprintf(v->szSIFormatBuffer, "  <YEAR>%d</YEAR>", v->dvbtdt.nYear); WriteHTMLLine(hXMLFile, v->szSIFormatBuffer);
			wsprintf(v->szSIFormatBuffer, "  <MONTH>%d</MONTH>", v->dvbtdt.nMonth); WriteHTMLLine(hXMLFile, v->szSIFormatBuffer);
			wsprintf(v->szSIFormatBuffer, "  <DAY>%d</DAY>", v->dvbtdt.nDay); WriteHTMLLine(hXMLFile, v->szSIFormatBuffer);
			wsprintf(v->szSIFormatBuffer, "  <HOUR>%d</HOUR>", v->dvbtdt.nHour); WriteHTMLLine(hXMLFile, v->szSIFormatBuffer);
			wsprintf(v->szSIFormatBuffer, "  <MINUTE>%d</MINUTE>", v->dvbtdt.nMinute); WriteHTMLLine(hXMLFile, v->szSIFormatBuffer);
			wsprintf(v->szSIFormatBuffer, "  <SECOND>%d</SECOND>", v->dvbtdt.nSecond); WriteHTMLLine(hXMLFile, v->szSIFormatBuffer);
			WriteHTMLLine(hXMLFile, " </TDT>");
		}
		if (v->dvbtot.hTreeItem != NULL)
		{
			WriteHTMLLine(hXMLFile, " <TOT>");
			wsprintf(v->szSIFormatBuffer, "  <YEAR>%d</YEAR>", v->dvbtot.nYear); WriteHTMLLine(hXMLFile, v->szSIFormatBuffer);
			wsprintf(v->szSIFormatBuffer, "  <MONTH>%d</MONTH>", v->dvbtot.nMonth); WriteHTMLLine(hXMLFile, v->szSIFormatBuffer);
			wsprintf(v->szSIFormatBuffer, "  <DAY>%d</DAY>", v->dvbtot.nDay); WriteHTMLLine(hXMLFile, v->szSIFormatBuffer);
			wsprintf(v->szSIFormatBuffer, "  <HOUR>%d</HOUR>", v->dvbtot.nHour); WriteHTMLLine(hXMLFile, v->szSIFormatBuffer);
			wsprintf(v->szSIFormatBuffer, "  <MINUTE>%d</MINUTE>", v->dvbtot.nMinute); WriteHTMLLine(hXMLFile, v->szSIFormatBuffer);
			wsprintf(v->szSIFormatBuffer, "  <SECOND>%d</SECOND>", v->dvbtot.nSecond); WriteHTMLLine(hXMLFile, v->szSIFormatBuffer);
			if (v->dvbtot.nDescriptorsLength)
			{
				int nOffset;

				for (nOffset = 0; nOffset < v->dvbtot.nDescriptorsLength;)
				{
					int nDescriptor = v->dvbtot.pDescriptors[nOffset];
					int nDescriptorLength = v->dvbtot.pDescriptors[nOffset + 1];
					int j;

					szDescriptorData[0] = '\0';
					WriteHTMLLine(hXMLFile, "  <DESCRIPTOR>");
					wsprintf(v->szSIFormatBuffer, "    <TAG>0x%02x</TAG>", nDescriptor); WriteHTMLLine(hXMLFile, v->szSIFormatBuffer);
					wsprintf(v->szSIFormatBuffer, "    <LENGTH>%d</LENGTH>", nDescriptorLength); WriteHTMLLine(hXMLFile, v->szSIFormatBuffer);
					for (j = 0; j < nDescriptorLength; j++)
					{
						char szTemp[8];
						wsprintf(szTemp, "0x%02x ", v->dvbtot.pDescriptors[nOffset + j + 2]);
						lstrcat(szDescriptorData, szTemp);
					}
					if (lstrlen(szDescriptorData))
					{
						StripTrailingSpaces(szDescriptorData);
						sprintf(v->szSIFormatBuffer, "    <DATA>%s</DATA>", szDescriptorData);
						WriteHTMLLine(hXMLFile, v->szSIFormatBuffer);
					}						
					nOffset += nDescriptorLength + 2;
					WriteHTMLLine(hXMLFile, "  </DESCRIPTOR>");
				}
			}
			WriteHTMLLine(hXMLFile, " </TOT>");
		}
	}
	else if (v->nNetworkPID == 0x1ffb)
	{
		if (v->dvbtdt.hTreeItem != NULL)
		{
			WriteHTMLLine(hXMLFile, " <STT>");
			wsprintf(v->szSIFormatBuffer, "  <YEAR>%d</YEAR>", v->dvbtdt.nYear); WriteHTMLLine(hXMLFile, v->szSIFormatBuffer);
			wsprintf(v->szSIFormatBuffer, "  <MONTH>%d</MONTH>", v->dvbtdt.nMonth); WriteHTMLLine(hXMLFile, v->szSIFormatBuffer);
			wsprintf(v->szSIFormatBuffer, "  <DAY>%d</DAY>", v->dvbtdt.nDay); WriteHTMLLine(hXMLFile, v->szSIFormatBuffer);
			wsprintf(v->szSIFormatBuffer, "  <HOUR>%d</HOUR>", v->dvbtdt.nHour); WriteHTMLLine(hXMLFile, v->szSIFormatBuffer);
			wsprintf(v->szSIFormatBuffer, "  <MINUTE>%d</MINUTE>", v->dvbtdt.nMinute); WriteHTMLLine(hXMLFile, v->szSIFormatBuffer);
			wsprintf(v->szSIFormatBuffer, "  <SECOND>%d</SECOND>", v->dvbtdt.nSecond); WriteHTMLLine(hXMLFile, v->szSIFormatBuffer);
			wsprintf(v->szSIFormatBuffer, "  <GPS-OFFSET>%d</GPS-OFFSET>", v->dvbtdt.nGPSOffset); WriteHTMLLine(hXMLFile, v->szSIFormatBuffer);
			wsprintf(v->szSIFormatBuffer, "  <DAYLIGHT-SAVING>%d</DAYLIGHT-SAVING>", v->dvbtdt.nDaylightSavings); WriteHTMLLine(hXMLFile, v->szSIFormatBuffer);
			WriteHTMLLine(hXMLFile, " </STT>");
		}
	}
	
	// Now the EIT data
	WriteHTMLLine(hXMLFile, " <EIT>");
	wsprintf(v->szSIFormatBuffer, "  <CRC-ERRORS>%d</CRC-ERRORS>", v->nSIParserCRCs[SI_PARSER_STATS_EIT]); 	WriteHTMLLine(hXMLFile, v->szSIFormatBuffer);
	wsprintf(v->szSIFormatBuffer, "  <VERSION>%d</VERSION>", v->nSIParserVersionNumbers[SI_PARSER_STATS_EIT]); 	WriteHTMLLine(hXMLFile, v->szSIFormatBuffer);
	for (nServiceIndex = 0; nServiceIndex < MAX_EIT_CHANNEL_DATA; nServiceIndex++)
	{
		if (v->pEvents[nServiceIndex] != NULL)
		{
			FormatEITEntry(nServiceIndex, EIT_FORMAT_XML, FALSE);
			WriteHTMLLine(hXMLFile, v->szSIFormatBuffer);
		}
	}
	WriteHTMLLine(hXMLFile, " </EIT>");


	// Now PID usage
	WriteHTMLLine(hXMLFile, " <PID-USAGE>");
	if (v->dDisplayMuxRate > 0)
	{
		EnterCriticalSection(&v->ss.csPIDCounter);
		sprintf(v->szSIFormatBuffer, "  <MUXRATE-BPS>%.0f</MUXRATE-BPS>", (v->dDisplayMuxRate / (double)v->nMuxRateCounter) * 8.0);
		WriteHTMLLine(hXMLFile, v->szSIFormatBuffer);
		LeaveCriticalSection(&v->ss.csPIDCounter);
	}

	for (i = 0; i < 8192; i++)
	{
		v->pc[i].nPID = i;
		v->pc[i].lnPackets = v->lnPIDCounter[i];
		v->pc[i].fScrambled = v->fPIDScrambled[i];
		if (v->lnPIDRateSamples[i])
			v->pc[i].dPIDRate = v->dPIDRate[i] / (double)v->lnPIDRateSamples[i];
		else
			v->pc[i].dPIDRate = 0.0;
		v->pc[i].nPIDHasContinuityErrors = v->nPIDHasContinuityErrors[i];
		v->pc[i].nPIDTEICount = v->nPIDTEICount[i];
	}
	v->lnCopyTotalTSPackets = v->lnTotalTSPackets;

	for (i = 0; i < 8192; i++)
	{
		char * szPIDDescriptionOutput;
		double dPercent;
		double dRate = 0.0;
		char szPIDDescription[128] = {0};

		if (v->pc[i].lnPackets == 0)
			continue;

		dPercent = ((double)v->pc[i].lnPackets / (double)v->lnCopyTotalTSPackets) * 100.0;
		if (v->pc[i].dPIDRate)
			dRate = v->pc[i].dPIDRate * 8.0 / 1000.0 / 1000.0;
		else
		{
			if (v->dDisplayMuxRate)
			{
				dRate = (v->dDisplayMuxRate / (double)v->nMuxRateCounter) * 8.0;
				dRate = ((dRate / 100.0) * dPercent) / 1000.0 / 1000.0;
			}
		}

		WriteHTMLLine(hXMLFile, "  <PID>");
		wsprintf(v->szSIFormatBuffer, "   <NUMBER>0x%04x</NUMBER>", i); WriteHTMLLine(hXMLFile, v->szSIFormatBuffer);
		wsprintf(v->szSIFormatBuffer, "   <NUMBER-DECIMAL>%d</NUMBER-DECIMAL>", i); WriteHTMLLine(hXMLFile, v->szSIFormatBuffer);
		sprintf(v->szSIFormatBuffer, "   <PERCENTAGE-OF-MUX>%.4f</PERCENTAGE-OF-MUX>", dPercent); WriteHTMLLine(hXMLFile, v->szSIFormatBuffer);
		if (dRate != 0.0)
		{
			sprintf(v->szSIFormatBuffer, "   <RATE-MbPS>%.4f</RATE-MbPS>", dRate); WriteHTMLLine(hXMLFile, v->szSIFormatBuffer);
		}
		GetPIDTooltipInfo(i, szPIDDescription);
		szPIDDescriptionOutput = strstr(szPIDDescription, " ");
		if (szPIDDescriptionOutput == NULL)
			szPIDDescriptionOutput = szPIDDescription;	
		else
			szPIDDescriptionOutput++;
		wsprintf(v->szSIFormatBuffer, "   <DESCRIPTION>%s</DESCRIPTION>", szPIDDescriptionOutput); WriteHTMLLine(hXMLFile, v->szSIFormatBuffer);
		wsprintf(v->szSIFormatBuffer, "   <SCRAMBLED>%d</SCRAMBLED>", v->pc[i].fScrambled); WriteHTMLLine(hXMLFile, v->szSIFormatBuffer);
		wsprintf(v->szSIFormatBuffer, "   <CONTINUITY-ERRORS>%d</CONTINUITY-ERRORS>", v->pc[i].nPIDHasContinuityErrors); WriteHTMLLine(hXMLFile, v->szSIFormatBuffer);
		wsprintf(v->szSIFormatBuffer, "   <TEI-ERRORS>%d</TEI-ERRORS>", v->pc[i].nPIDTEICount); WriteHTMLLine(hXMLFile, v->szSIFormatBuffer);
		if (lstrcmp(szPIDDescriptionOutput, "Unknown usage") == 0)
			WriteHTMLLine(hXMLFile, "   <GHOST-PID>1</GHOST-PID>");
		else			
			WriteHTMLLine(hXMLFile, "   <GHOST-PID>0</GHOST-PID>");
		WriteHTMLLine(hXMLFile, "  </PID>");
	}
	WriteHTMLLine(hXMLFile, " </PID-USAGE>");

	// That's all folks
	WriteHTMLLine(hXMLFile, "</MPEG-TABLES>");
	//UpdateMainStatusText("");
}

#ifndef LITE
void XMLTVExport(HWND hDlg, HANDLE hXMLFile)
{
	int nServiceIndex;


	// XML header
	WriteHTMLLine(hXMLFile, "<?xml version=\"1.0\" encoding=\"ISO-8859-1\" ?>");
	WriteHTMLLine(hXMLFile, "<!-- Created by TSReader from COOLSTF.com -->");
	WriteHTMLLine(hXMLFile, "<tv generator-info-name=\"TSReader\">");

	for (nServiceIndex = 0; nServiceIndex < MAX_EIT_CHANNEL_DATA; nServiceIndex++)
	{
		if (v->pEvents[nServiceIndex] != NULL)
		{
			char szTemp[128];

			wsprintf(szTemp, "XMLTV Export for ch. %d", nServiceIndex);
			UpdateMainStatusText(szTemp);
			FormatEITEntry(nServiceIndex, EIT_FORMAT_XMLTV, FALSE);
			WriteHTMLLine(hXMLFile, v->szSIFormatBuffer);
		}
	}

	WriteHTMLLine(hXMLFile, "</tv>");
	//UpdateMainStatusText("");
}
#endif LITE

void StartXMLExport(HWND hDlg, BOOL fXMLTVFormat)
{
	OPENFILENAME ofn;
	HANDLE hXMLFile;
	char szFile[MAX_PATH] = TEXT(".xml\0");

	memset( &(ofn), 0, sizeof(ofn));
	ofn.lStructSize	= sizeof(ofn);
	ofn.hwndOwner = hDlg;
    ofn.lpstrFile = szFile;
    ofn.nMaxFile = MAX_PATH;
	ofn.lpstrFilter = TEXT("XML Files (*.xml)\0*.xml\0All Files (*.*)\0*.*\0\0");	
	if (fXMLTVFormat == FALSE)
		ofn.lpstrTitle = TEXT("Export Tables in XML");
	else
		ofn.lpstrTitle = TEXT("Export XMLTV Program Guide");
	ofn.lpstrDefExt = TEXT("xml");
	ofn.lpstrInitialDir = v->szXMLInitialDir;
	ofn.Flags =  OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT | OFN_EXPLORER;
	ofn.hInstance = v->hInstance;			
	if (myGetSaveFileName(&ofn) == TRUE)
	{
		hXMLFile = CreateFile(ofn.lpstrFile,
				              GENERIC_WRITE,
				              0,
							  (LPSECURITY_ATTRIBUTES) NULL,
							  CREATE_ALWAYS,
							  FILE_ATTRIBUTE_NORMAL,
							  (HANDLE) NULL);
		if (hXMLFile == INVALID_HANDLE_VALUE)
		{
			MessageBox(hDlg, TEXT("Unable to open file for output"), gszAppName, MB_OK | MB_ICONINFORMATION);
			return;
		}
		CursorWait(hDlg);
		if (fXMLTVFormat == FALSE)
			XMLExport(hDlg, hXMLFile);
		else
			XMLTVExport(hDlg, hXMLFile);
		CloseHandle(hXMLFile);
		CursorNormal();
	}
}
#endif LITE

void WriteHTMLStatistics(HANDLE hHTMFile, int nTable, char * szTableName)
{
	char szTemp[512];
	char szPackets[128];
	char szErrors[128];

	WriteHTMLLine(hHTMFile, "  <TR>");
	if (v->nSIParserPackets[nTable] > 1000 * 1000)
		sprintf(szPackets, "%.1fm", (double)v->nSIParserPackets[nTable] / 1000.0 / 1000.0);
	else if (v->nSIParserPackets[nTable] > 1000)
		sprintf(szPackets, "%.1fk", (double)v->nSIParserPackets[nTable] / 1000.0);
	else
		wsprintf(szPackets, "%d", v->nSIParserPackets[nTable]);
	
	if (v->nSIParserCRCs[nTable] > 1000 * 1000)
		sprintf(szErrors, "%.1fm", (double)v->nSIParserCRCs[nTable] / 1000.0 / 1000.0);
	else if (v->nSIParserCRCs[nTable] > 1000)
		sprintf(szErrors, "%.1fk", (double)v->nSIParserCRCs[nTable] / 1000.0);
	else
		wsprintf(szErrors, "%d", v->nSIParserCRCs[nTable]);

	wsprintf(szTemp, "   <TD WIDTH=\"33%\">%s</TD><TD WIDTH=\"33%\">%s</TD><TD WIDTH=\"33%\">%s</TD>",
		     szTableName,
			 szPackets,
			 szErrors);
	WriteHTMLLine(hHTMFile, szTemp);	
	WriteHTMLLine(hHTMFile, "  </TR>");
}

void HTMLExport(HANDLE hHTMFile, int nExportSITables, char * szOutputFilename)
{
	// Write the header
	WriteHTMLLine(hHTMFile, "<HTML>");
	wsprintf(v->szSIFormatBuffer, "<TITLE>SI Parsing by TSReader %s</TITLE>", GetTSRVersion(NULL));
	WriteHTMLLine(hHTMFile, v->szSIFormatBuffer);
	WriteHTMLLine(hHTMFile, "<BODY>");

	if ((nExportSITables & 1) == 1)
	{
		WriteHTMLLine(hHTMFile, "<H3>Program Association Table</H3>");
		FormatPAT(TRUE, nExportSITables);
		WriteHTMLASCII(hHTMFile, v->szSIFormatBuffer);
	}

	if ((nExportSITables & 2) == 2)
	{
		int nPMTIndex;
		BOOL fNeedLine = FALSE;

		WriteHTMLLine(hHTMFile, "<H3>Program Map Table(s)</H3>");
		for (nPMTIndex = 0; nPMTIndex < MAX_PAT_ENTRIES; nPMTIndex++)
		{
			if (v->pat.pmt[nPMTIndex].nPMTPID != 0)
			{
				if (fNeedLine == TRUE)
					WriteHTMLLine(hHTMFile, "<HR ALIGN=LEFT WIDTH=\"100%\">");
				if (v->pat.pmt[nPMTIndex].nProgramNumber == 0)
				{		
					wsprintf(v->szSIFormatBuffer, "Network PMT Entry - carried on PID %d (0x%04x)", v->pat.pmt[nPMTIndex].nPMTPID, v->pat.pmt[nPMTIndex].nPMTPID);
				}
				else
				{
					int nLCN = GetLogicalChannelNumber(v->pat.pmt[nPMTIndex].nProgramNumber);
					int nESIndex;
					char szTemp[256];
					char szLogicalChannelNumber[64] = {0};

					// Program number and link to SDT possibly
					if (nLCN != 0)
						wsprintf(szLogicalChannelNumber, "%d/", nLCN);
					wsprintf(v->szSIFormatBuffer, "<A NAME=\"pmt_%d\"></A>Program Number: %s%d",
							 v->pat.pmt[nPMTIndex].nProgramNumber, szLogicalChannelNumber, v->pat.pmt[nPMTIndex].nProgramNumber);
					if (v->pChannelData[v->pat.pmt[nPMTIndex].nProgramNumber] != NULL)
					{
						if ((nExportSITables & 0x10) == 0x10)
							wsprintf(szTemp, " <A HREF=\"#sdt_%d\">%s</A>", v->pat.pmt[nPMTIndex].nProgramNumber, v->pChannelData[v->pat.pmt[nPMTIndex].nProgramNumber]->szShortName);
						else
							wsprintf(szTemp, " %s", v->pChannelData[v->pat.pmt[nPMTIndex].nProgramNumber]->szShortName);
						lstrcat(v->szSIFormatBuffer, szTemp);
					}
					lstrcat(v->szSIFormatBuffer, "\r\n");

					// PMT descriptors
					if (v->pat.pmt[nPMTIndex].nProgramInfoLength)
					{
						int nProgramInfoLength = v->pat.pmt[nPMTIndex].nProgramInfoLength;
						int nOffset = 0;

						lstrcat(v->szSIFormatBuffer, "<FONT SIZE=\"-1\">");

						while (nProgramInfoLength)
						{
							int nDescriptor = v->pat.pmt[nPMTIndex].pProgramInfo[nOffset];
							int nDescriptorLength = v->pat.pmt[nPMTIndex].pProgramInfo[nOffset + 1];
							char szDescriptor[128];

							DecodeDescriptorNames(szDescriptor, nDescriptor);
							wsprintf(szTemp, "Descriptor: %s<BR>", szDescriptor);
							lstrcat(v->szSIFormatBuffer, szTemp);

							DecodeMPEG2Descriptor(&v->pat.pmt[nPMTIndex].pProgramInfo[nOffset], FALSE);
							nOffset += nDescriptorLength + 2;
							nProgramInfoLength -= nDescriptorLength + 2;			
						}
						lstrcat(v->szSIFormatBuffer, "</FONT SIZE>");
					}

					// Thumbnails
					if ((nExportSITables & 0x100) == 0x100)
					{
						for (nESIndex = 0; nESIndex < MAX_ESLIST_ENTRIES; nESIndex++)
						{
							if (v->pat.pmt[nPMTIndex].es[nESIndex].nESPID == 0)
								break;
							if (v->pat.pmt[nPMTIndex].es[nESIndex].pRGBVideoFrame != NULL)
							{
								int i;
								char szJPGName[MAX_PATH];

								lstrcpy(szJPGName, szOutputFilename);
								for (i = lstrlen(szJPGName); i > 0; i--)
								{
									if (szJPGName[i] == '.')
									{
										char szNewExtension[128];

										szJPGName[i] = '\0';
										wsprintf(szNewExtension, "_%d_%d.jpg", v->pat.pmt[nPMTIndex].nProgramNumber, nESIndex);
										lstrcat(szJPGName, szNewExtension);
										break;
									}
								}
								if (i != 0)
								{
									HISDEST	hDestinationObject = _ISOpenFileDest(szJPGName);
									if (hDestinationObject != NULL)
									{
										int j;
										char * szHTMLJPGName = NULL;

										_ISWriteRGBToJPG(hDestinationObject,
														 v->pat.pmt[nPMTIndex].es[nESIndex].pRGBVideoFrame,
														 v->pat.pmt[nPMTIndex].es[nESIndex].nVideoWidth,
														 v->pat.pmt[nPMTIndex].es[nESIndex].nVideoHeight,
														 100,
														 0);
										_ISCloseDest(hDestinationObject);

										for (j = strlen(szJPGName); j > 0; j--)
										{
											if (szJPGName[j] == '\\' || szJPGName[j] == '/')
											{
												szHTMLJPGName = &szJPGName[j + 1];
												break;
											}
										}
										if (szHTMLJPGName != NULL)
										{
											wsprintf(szTemp, "<BR><IMG SRC=\"%s\">\r\n", szHTMLJPGName);
											lstrcat(v->szSIFormatBuffer, szTemp);
										}
									}
								}
								//break;
							}
						}
					}

					// Elementary streams
					for (nESIndex = 0; nESIndex < MAX_ESLIST_ENTRIES; nESIndex++)
					{
						char szStreamTypeEnglish[64];

						if (v->pat.pmt[nPMTIndex].es[nESIndex].nESPID == 0)
							break;

						DecodeStreamType(v->pat.pmt[nPMTIndex].es[nESIndex].nStreamType, szStreamTypeEnglish, nPMTIndex, nESIndex);
						wsprintf(szTemp, "\r\nStream Type: 0x%02x %s PID %d (0x%04x)\r\n",
								 v->pat.pmt[nPMTIndex].es[nESIndex].nStreamType,
								 szStreamTypeEnglish,
								 v->pat.pmt[nPMTIndex].es[nESIndex].nESPID,
								 v->pat.pmt[nPMTIndex].es[nESIndex].nESPID);
						lstrcat(v->szSIFormatBuffer, szTemp);

						if (v->pat.pmt[nPMTIndex].es[nESIndex].pParsedData != NULL)
						{
							char szParseDecode[200] = {0};

							switch(v->pat.pmt[nPMTIndex].es[nESIndex].nParseType)
							{
							case PARSE_ES_TYPE_MPEG2_VIDEO:
								FormatMPEGVideoParse(nPMTIndex, nESIndex, szParseDecode);
								break;
							case PARSE_ES_TYPE_MPEG_AUDIO:
								FormatMPEGAudioParse((PPARSEDMPEGAUDIO)v->pat.pmt[nPMTIndex].es[nESIndex].pParsedData, szParseDecode);
								break;
							case PARSE_ES_TYPE_AC3_AUDIO:
								FormatAC3Parse((PPARSEDAC3AUDIO)v->pat.pmt[nPMTIndex].es[nESIndex].pParsedData, szParseDecode);
								break;
#ifndef LITE
							case PARSE_ES_TYPE_MPEG2_AAC_AUDIO:
							case PARSE_ES_TYPE_MPEG4_AAC_AUDIO:
								FormatAACAudioParse((PPARSEDAACAUDIO)v->pat.pmt[nPMTIndex].es[nESIndex].pParsedData, szParseDecode);
								break;
							case PARSE_ES_TYPE_H264_VIDEO:
								FormatH264VideoParse(nPMTIndex, nESIndex, szParseDecode);
								break;
							case PARSE_ES_TYPE_MPEG4_VIDEO:
								FormatMPEG4VideoParse(nPMTIndex, nESIndex, szParseDecode);
								break;
#endif LITE
#ifdef PRO
							case PARSE_ES_TYPE_VC1_VIDEO:
								FormatVC1VideoParse(nPMTIndex, nESIndex, szParseDecode);
								break;
#endif PRO
							}
							if (lstrlen(szParseDecode))
							{
								lstrcat(v->szSIFormatBuffer, "<FONT SIZE=\"-1\">");
								lstrcat(v->szSIFormatBuffer, szParseDecode);
								lstrcat(v->szSIFormatBuffer, "</FONT SIZE>");
							}
						}

						if (v->pat.pmt[nPMTIndex].es[nESIndex].pDescriptors)
						{
							char szTemp[128];
							BYTE * pDescriptorData = v->pat.pmt[nPMTIndex].es[nESIndex].pDescriptors;
							int nDescriptorsLength = v->pat.pmt[nPMTIndex].es[nESIndex].nDescriptorsLength;
							int nCurrentIndex = 0;

							lstrcat(v->szSIFormatBuffer, "<FONT SIZE=\"-1\">");
							do
							{
								char szDescriptor[128];
								int nThisDescriptorLength = (BYTE)pDescriptorData[nCurrentIndex + 1];

								DecodeDescriptorNames(szDescriptor, pDescriptorData[nCurrentIndex]);
								wsprintf(szTemp, "Descriptor: %s\r\n", szDescriptor);
								lstrcat(v->szSIFormatBuffer, szTemp);

								DecodeMPEG2Descriptor(&pDescriptorData[nCurrentIndex], FALSE);

								nCurrentIndex += (BYTE)pDescriptorData[nCurrentIndex + 1]; // descriptor length
								nCurrentIndex += 2;	// descriptor tag and length
							} while (nCurrentIndex < nDescriptorsLength);
							lstrcat(v->szSIFormatBuffer, "</FONT SIZE>");
						}
					}
				}
				WriteHTMLASCII(hHTMFile, v->szSIFormatBuffer);
				fNeedLine = TRUE;
			}
		}
	}
	if ( ((nExportSITables & 4) == 4) && (v->fDidCAT) )
	{
		WriteHTMLLine(hHTMFile, "<H3>Conditional Access Table</H3>");
		FormatCAT(TRUE);
		WriteHTMLASCII(hHTMFile, v->szSIFormatBuffer);
	}
	if ( ((nExportSITables & 8) == 8) && (v->nNetworkPID == 0x0010) )
	{
		int nNITIndex;
		BOOL fNeedLine = FALSE;

		WriteHTMLLine(hHTMFile, "<A NAME=\"nit\">");
		WriteHTMLLine(hHTMFile, "<H3>Network Information Table</H3>");
		for (nNITIndex = 0; nNITIndex < MAX_TS_ENTRIES; nNITIndex++)
		{
			if (v->pNITData[nNITIndex] != NULL)
			{
				if (fNeedLine == TRUE)
					WriteHTMLLine(hHTMFile, "<HR ALIGN=LEFT>");
				FormatNITEntry(nNITIndex, TRUE);
				WriteHTMLASCII(hHTMFile, v->szSIFormatBuffer);
				fNeedLine = TRUE;
			}
		}
	}
	if ((nExportSITables & 0x10) == 0x10)
	{
		int nServiceIndex;
		BOOL fNeedLine = FALSE;
		if (v->nNetworkPID != 0x1ffb)
			WriteHTMLLine(hHTMFile, "<H3>Service Description Table</H3>");
		else
			WriteHTMLLine(hHTMFile, "<H3>Terrestrial Virtual Channel Table</H3>");
		
		for (nServiceIndex = 0; nServiceIndex < MAX_EIT_CHANNEL_DATA; nServiceIndex++)
		{
			if (v->pChannelData[nServiceIndex] != NULL)
			{
				if ((nExportSITables & 0x40) == 0x40)
				{
					int nPMTIndex;
					for (nPMTIndex = 0; nPMTIndex < MAX_PAT_ENTRIES; nPMTIndex++)
					{
						if (v->pat.pmt[nPMTIndex].nProgramNumber == v->pChannelData[nServiceIndex]->nChannelNumber)
							break;
					}
					if (nPMTIndex == MAX_PAT_ENTRIES)
						continue;
				}
				if (fNeedLine == TRUE)
					WriteHTMLLine(hHTMFile, "<HR ALIGN=LEFT>");
				FormatSDTEntry(nServiceIndex, TRUE);
				WriteHTMLASCII(hHTMFile, v->szSIFormatBuffer);
				fNeedLine = TRUE;
			}
		}
	}
	if ((nExportSITables & 0x20) == 0x20)
	{
		int nServiceIndex;
		BOOL fNeedLine = FALSE;

		WriteHTMLLine(hHTMFile, "<H3>Event Information Table</H3>");
		
		for (nServiceIndex = 0; nServiceIndex < MAX_EIT_CHANNEL_DATA; nServiceIndex++)
		{
			if (v->pEvents[nServiceIndex] != NULL)
			{
				char szTemp[128];

				wsprintf(szTemp, "<A NAME=\"eit_%d\"></A>", nServiceIndex);
				WriteHTMLLine(hHTMFile, szTemp);

				if (fNeedLine == TRUE)
					WriteHTMLLine(hHTMFile, "<HR ALIGN=LEFT>");
				
				FormatEITEntry(nServiceIndex, EIT_FORMAT_PLAIN, TRUE);
				WriteHTMLASCII(hHTMFile, v->szSIFormatBuffer);
				fNeedLine = TRUE;
			}
		}
	}
	if ((nExportSITables & 0x200) == 0x200)
	{
		char szNITLabel[8] = {"NIT"};
		char szSDTLabel[8] = {"SDT"};
		char szMuxRate[64] = {"n/a"};

		if (v->nNetworkPID == 0x1ffb)
		{
			lstrcpy(szNITLabel, "ETT");
			lstrcpy(szSDTLabel, "PSIP");
		}
				
		WriteHTMLLine(hHTMFile, "<H3>MPEG-2 Statistics</H3>");

		WriteHTMLLine(hHTMFile, " <TABLE BORDER=\"1\" CELLSPACING=\"2\" CELLPADDING=\"0\">");
		WriteHTMLLine(hHTMFile, "  <TR>");
		WriteHTMLLine(hHTMFile, "   <TD WIDTH=\"33%\"><B>Table</B></TD><TD WIDTH=\"33%\"><B>Sections Processed</B></TD><TD WIDTH=\"33%\"><B>CRC Errors</B></TD>");
		WriteHTMLLine(hHTMFile, "  </TR>");

		WriteHTMLStatistics(hHTMFile, SI_PARSER_STATS_PAT, "PAT");
		WriteHTMLStatistics(hHTMFile, SI_PARSER_STATS_CAT, "CAT");
		WriteHTMLStatistics(hHTMFile, SI_PARSER_STATS_PMT, "PMT");
		WriteHTMLStatistics(hHTMFile, SI_PARSER_STATS_NIT, szNITLabel);
		if (v->nNetworkPID != 0x0ffe)
		{
			WriteHTMLStatistics(hHTMFile, SI_PARSER_STATS_SDT, szSDTLabel);
			WriteHTMLStatistics(hHTMFile, SI_PARSER_STATS_EIT, "EIT");
		}
		WriteHTMLLine(hHTMFile, " </TABLE>");
		
		WriteHTMLLine(hHTMFile, " <BR>");
		wsprintf(v->szSIFormatBuffer, "Continuity errors: %d<BR>", v->nContinuityErrors); WriteHTMLLine(hHTMFile, v->szSIFormatBuffer);
		wsprintf(v->szSIFormatBuffer, "TEI errors: %d<BR>", v->nTEIErrors); WriteHTMLLine(hHTMFile, v->szSIFormatBuffer);

		if (GetSyncLossCount != NULL)
		{
			wsprintf(v->szSIFormatBuffer, "Sync Losses: %d<BR>", GetSyncLossCount(FALSE)); WriteHTMLLine(hHTMFile, v->szSIFormatBuffer);
		}
		if (GetRetuneCount != NULL)
		{
			wsprintf(v->szSIFormatBuffer, "Retunes: %d<BR>", GetRetuneCount(FALSE)); WriteHTMLLine(hHTMFile, v->szSIFormatBuffer);
		}

		if (v->dDisplayMuxRate > 0)
		{
			EnterCriticalSection(&v->ss.csPIDCounter);
			sprintf(szMuxRate, "%.0f bps", (v->dDisplayMuxRate / (double)v->nMuxRateCounter) * 8.0);
			LeaveCriticalSection(&v->ss.csPIDCounter);
		}
		wsprintf(v->szSIFormatBuffer, "Calculated multiplex rate: %s<BR>", szMuxRate);
		WriteHTMLASCII(hHTMFile, v->szSIFormatBuffer);

	}
	if ((nExportSITables & 0x400) == 0x400)
	{
		int nLine;

		WriteHTMLLine(hHTMFile, "<H3>General Information</H3>");
		for (nLine = 0; nLine < 10; nLine++)
		{
			char szTemp[128];
			GetSourceInfoLine(nLine, szTemp);
			if (lstrlen(szTemp))
			{
				wsprintf(v->szSIFormatBuffer, "%s<BR>", szTemp);
				WriteHTMLASCII(hHTMFile, v->szSIFormatBuffer);
			}
		}

	}

	if ((nExportSITables & 0x80) == 0x80)
	{
		int i;

		WriteHTMLLine(hHTMFile, "<H3>PID Usage Chart</H3>");

		for (i = 0; i < 8192; i++)
		{
			v->pc[i].nPID = i;
			v->pc[i].lnPackets = v->lnPIDCounter[i];
			v->pc[i].fScrambled = v->fPIDScrambled[i];
			if (v->lnPIDRateSamples[i])
				v->pc[i].dPIDRate = v->dPIDRate[i] / (double)v->lnPIDRateSamples[i];
			else
				v->pc[i].dPIDRate = 0.0;
			v->pc[i].nPIDTEICount = v->nPIDTEICount[i];
			v->pc[i].nPIDHasContinuityErrors = v->nPIDHasContinuityErrors[i];
		}
		v->lnCopyTotalTSPackets = v->lnTotalTSPackets;

		WriteHTMLLine(hHTMFile, "<TABLE WIDTH=\"100%\" BORDER=\"0\" CELLSPACING=\"0\" CELLPADDING=\"0\">\n");
		for (i = 0; i < 8192; i++)
		{
			double dPercent;
			char szMask[128];
			char szPID[128];
			char szTemp[128];
			char szPIDDescription[128] = {0};

			if (v->pc[i].lnPackets == 0)
				continue;
			WriteHTMLLine(hHTMFile, " <TR>");
			dPercent = ((double)v->pc[i].lnPackets / (double)v->lnCopyTotalTSPackets) * 100.0;
			if (v->pc[i].dPIDRate)
			{
				wsprintf(szMask, "%s (%%.2f%%%% - %%.2f Mbps)", v->szOutputPIDFlags);
				sprintf(szPID, szMask, v->pc[i].nPID, dPercent, v->pc[i].dPIDRate * 8.0 / 1000.0 / 1000.0);
			}
			else
			{
				if (v->dDisplayMuxRate)
				{
					double dRate = (v->dDisplayMuxRate / (double)v->nMuxRateCounter) * 8.0;
					wsprintf(szMask, "%s (%%.2f%%%% ~ %%.2f Mbps)", v->szOutputPIDFlags);
					sprintf(szPID, szMask, v->pc[i].nPID, dPercent, ((dRate / 100.0) * dPercent) / 1000.0 / 1000.0);
				}
				else
				{
					wsprintf(szMask, "%s (%%.3f%%%%)", v->szOutputPIDFlags);
					sprintf(szPID, szMask, v->pc[i].nPID, dPercent);
				}
			}
			if (v->pc[i].nPIDTEICount || v->pc[i].nPIDHasContinuityErrors)
			{
				wsprintf(szTemp, " *%d/%d", v->pc[i].nPIDHasContinuityErrors, v->pc[i].nPIDTEICount);
				lstrcat(szPID, szTemp);
			}

			wsprintf(szTemp, "  <TD WIDTH=\"20%%\" HEIGHT=\"1\"><FONT SIZE=\"-1\">%s</FONT SIZE></TD>", szPID);
			WriteHTMLLine(hHTMFile, szTemp);

			if (v->pc[i].fScrambled)
				sprintf(szTemp, "   <TD WIDTH=\"60%%\" HEIGHT=\"1\"><FONT SIZE=\"-1\"><HR ALIGN=LEFT COLOR=\"#ff0000\" SIZE=\"5\" WIDTH=\"%.1f%%\" NOSHADE></FONT SIZE></TD>", dPercent);
			else
				sprintf(szTemp, "   <TD WIDTH=\"60%%\" HEIGHT=\"1\"><FONT SIZE=\"-1\"><HR ALIGN=LEFT COLOR=\"#00ff00\" SIZE=\"5\" WIDTH=\"%.1f%%\" NOSHADE></FONT SIZE></TD>", dPercent);
			WriteHTMLLine(hHTMFile, szTemp);

			GetPIDTooltipInfo(i, szPIDDescription);
			wsprintf(szTemp, "  <TD WIDTH=\"20%%\" HEIGHT=\"1\"><FONT SIZE=\"-2\">%s</FONT SIZE></TD>", szPIDDescription);
			WriteHTMLLine(hHTMFile, szTemp);

			WriteHTMLLine(hHTMFile, " </TR>");
		}
		WriteHTMLLine(hHTMFile, "</FONT SIZE>");
		WriteHTMLLine(hHTMFile, "</TABLE>");
	}

	// All done, write the footer
	WriteHTMLLine(hHTMFile, "</BODY>");
	WriteHTMLLine(hHTMFile, "</HTML>");
}

UINT FAR PASCAL ExportSIHookProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam) 
{ 
	switch(uMsg)
	{ 
	// WM_INITDIALOG is received after commdlg 
	//is processed. 
	case WM_INITDIALOG:
		{
			if (v->nNetworkPID != 0x0010)
			{
				ShowWindow(GetDlgItem(hDlg, IDC_EXPORT_SI_NIT), SW_HIDE);
				//ShowWindow(GetDlgItem(hDlg, IDC_EXPORT_SI_SDT), SW_HIDE);
				//ShowWindow(GetDlgItem(hDlg, IDC_EXPORT_SI_EIT), SW_HIDE);
				ShowWindow(GetDlgItem(hDlg, IDC_EXPORT_SI_SDT_THIS_MUX), SW_HIDE);
			}
			if (v->nNetworkPID == 0x1ffb)
				SetDlgItemText(hDlg, IDC_EXPORT_SI_SDT, "TVCT");

			if ((v->nExportSITables & 1) == 1)
				CheckDlgButton(hDlg, IDC_EXPORT_SI_PAT, BST_CHECKED);
			if ((v->nExportSITables & 2) == 2)
				CheckDlgButton(hDlg, IDC_EXPORT_SI_PMTS, BST_CHECKED);
			if ((v->nExportSITables & 4) == 4)
				CheckDlgButton(hDlg, IDC_EXPORT_SI_CAT, BST_CHECKED);
			if ((v->nExportSITables & 8) == 8)
				CheckDlgButton(hDlg, IDC_EXPORT_SI_NIT, BST_CHECKED);
			if ((v->nExportSITables & 0x10) == 0x10)
				CheckDlgButton(hDlg, IDC_EXPORT_SI_SDT, BST_CHECKED);
			if ((v->nExportSITables & 0x20) == 0x20)
				CheckDlgButton(hDlg, IDC_EXPORT_SI_EIT, BST_CHECKED);
			if ((v->nExportSITables & 0x40) == 0x40)
				CheckDlgButton(hDlg, IDC_EXPORT_SI_SDT_THIS_MUX, BST_CHECKED);
			if ((v->nExportSITables & 0x80) == 0x80)
				CheckDlgButton(hDlg, IDC_EXPORT_SI_PID_CHART, BST_CHECKED);
			if ((v->nExportSITables & 0x100) == 0x100)
				CheckDlgButton(hDlg, IDC_EXPORT_SI_THUMBNAILS, BST_CHECKED);
			if ((v->nExportSITables & 0x200) == 0x200)
				CheckDlgButton(hDlg, IDC_EXPORT_SI_MPEG_STATS, BST_CHECKED);
			if ((v->nExportSITables & 0x400) == 0x400)
				CheckDlgButton(hDlg, IDC_EXPORT_SI_GENERAL_INFO, BST_CHECKED);		
		}
		return TRUE; 
	case WM_COMMAND:
		switch (HIWORD(wParam))
		{
		case BN_CLICKED:
			if (LOWORD(wParam) == IDC_EXPORT_SI_ALL)
			{
				CheckDlgButton(hDlg, IDC_EXPORT_SI_PAT, BST_CHECKED);
				CheckDlgButton(hDlg, IDC_EXPORT_SI_PMTS, BST_CHECKED);
				CheckDlgButton(hDlg, IDC_EXPORT_SI_CAT, BST_CHECKED);
				CheckDlgButton(hDlg, IDC_EXPORT_SI_NIT, BST_CHECKED);
				CheckDlgButton(hDlg, IDC_EXPORT_SI_SDT, BST_CHECKED);
				CheckDlgButton(hDlg, IDC_EXPORT_SI_EIT, BST_CHECKED);
				CheckDlgButton(hDlg, IDC_EXPORT_SI_SDT_THIS_MUX, BST_UNCHECKED);
				CheckDlgButton(hDlg, IDC_EXPORT_SI_PID_CHART, BST_CHECKED);
				CheckDlgButton(hDlg, IDC_EXPORT_SI_THUMBNAILS, BST_CHECKED);
				CheckDlgButton(hDlg, IDC_EXPORT_SI_MPEG_STATS, BST_CHECKED);
				CheckDlgButton(hDlg, IDC_EXPORT_SI_GENERAL_INFO, BST_CHECKED);		
				v->nExportSITables = 1 | 2| 4 | 8 | 0x10 | 0x20 | 0x80 | 0x100 | 0x200 | 0x400;
			}
			else
			{
				v->nExportSITables = 0;
				if (IsDlgButtonChecked(hDlg, IDC_EXPORT_SI_PAT)) v->nExportSITables |= 1;
				if (IsDlgButtonChecked(hDlg, IDC_EXPORT_SI_PMTS)) v->nExportSITables |= 2;
				if (IsDlgButtonChecked(hDlg, IDC_EXPORT_SI_CAT)) v->nExportSITables |= 4;
				if (IsDlgButtonChecked(hDlg, IDC_EXPORT_SI_NIT)) v->nExportSITables |= 8;
				if (IsDlgButtonChecked(hDlg, IDC_EXPORT_SI_SDT)) v->nExportSITables |= 0x10;
				if (IsDlgButtonChecked(hDlg, IDC_EXPORT_SI_EIT)) v->nExportSITables |= 0x20;
				if (IsDlgButtonChecked(hDlg, IDC_EXPORT_SI_SDT_THIS_MUX)) v->nExportSITables |= 0x40;
				if (IsDlgButtonChecked(hDlg, IDC_EXPORT_SI_PID_CHART)) v->nExportSITables |= 0x80;
				if (IsDlgButtonChecked(hDlg, IDC_EXPORT_SI_THUMBNAILS)) v->nExportSITables |= 0x100;
				if (IsDlgButtonChecked(hDlg, IDC_EXPORT_SI_MPEG_STATS)) v->nExportSITables |= 0x200;
				if (IsDlgButtonChecked(hDlg, IDC_EXPORT_SI_GENERAL_INFO)) v->nExportSITables |= 0x400;
			}
		}
		return FALSE;
	}
	return FALSE; 
} 

void SIParserExport(HWND hDlg)
{
	OPENFILENAME ofn;
	HANDLE hHTMFile;
	BOOL fStatus;
	char szFile[MAX_PATH] = TEXT(".htm\0");

	memset( &(ofn), 0, sizeof(ofn));
	ofn.lStructSize	= sizeof(ofn);
	ofn.hwndOwner = hDlg;
    ofn.lpstrFile = szFile;
    ofn.nMaxFile = MAX_PATH;
	ofn.lpstrFilter = TEXT("HTML Files (*.htm)\0*.htm\0All Files (*.*)\0*.*\0\0");	
	ofn.lpstrTitle = TEXT("Export SI");
	ofn.lpstrDefExt = TEXT("htm");
	ofn.lpstrInitialDir = v->szHTMInitialDir;
	ofn.Flags =  OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT | OFN_EXPLORER | OFN_ENABLEHOOK | OFN_ENABLETEMPLATE;
	ofn.lpfnHook = ExportSIHookProc;
	ofn.lpTemplateName = MAKEINTRESOURCE(IDD_EXPORT_SI_HOOK);
	ofn.hInstance = v->hInstance;
			
	fStatus = myGetSaveFileName(&ofn);
	if (fStatus == TRUE)
	{
		hHTMFile = CreateFile(ofn.lpstrFile,
				              GENERIC_WRITE,
				              0,
							  (LPSECURITY_ATTRIBUTES) NULL,
							  CREATE_ALWAYS,
							  FILE_ATTRIBUTE_NORMAL,
							  (HANDLE) NULL);
		if (hHTMFile == INVALID_HANDLE_VALUE)
		{
			MessageBox(hDlg, TEXT("Unable to open file for output"), gszAppName, MB_OK | MB_ICONINFORMATION);
			return;
		}
		CursorWait(hDlg);
		HTMLExport(hHTMFile, v->nExportSITables, ofn.lpstrFile);
		CloseHandle(hHTMFile);
		CursorNormal();
	}
}
